/*-------------------------------------------------------------
  sntp.c : SNTP client
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
  Special thanks to Tomoaki Nakashima
---------------------------------------------------------------*/

#include "tcsntp.h"

#include <winsock.h>
#include <ras.h>

/* Globals */

BOOL InitSNTP(HWND hwndParent);
void EndSNTP(HWND hwndParent);
void SNTPCommand(HWND hwndMain, const char *pCommand);
void SetSNTPParam(const char *servername, int nTimeout, BOOL bLog,
	const char *soundfile);
BOOL StartSyncTime(HWND hwnd, const char *pServer, BOOL bRAS);
void OnTimerSNTP(HWND hwndMain);
void OnGetHost(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnReceive(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Statics */

static BOOL IsRASConnection(void);
static BOOL SNTPStart(HWND hwndSNTP, const char *pServer);
static void SNTPSend(HWND hwndSNTP, unsigned long serveraddr);
static void SynchronizeSystemTime(HWND hwndSNTP,
	DWORD seconds, DWORD fractions);
static void SocketClose(HWND hwndSNTP, const char *msgbuf);
static int GetServerPort(const char *buf, char *server);
static void Log(HWND hwndSNTP, const char *msg);
static void time2int(int *ph, int *pm, const char *src);

static char *m_section = "SNTP";

static char m_servername[BUFSIZE_SERVER] = { 0 }; // SNTP server's host name
static int  m_nTimeOut = 1000;              // msec of time out
static BOOL m_bSaveLog = FALSE;             // save log ?
static char m_soundfile[MAX_PATH] = { 0 };  // sound file 
static int  m_nMinuteDif = 0;               // forcely time difference

static BOOL  m_bSendingData = FALSE;     // now processing?
static DWORD m_dwTickCountOnGetHost = 0; // starting time of getting IP address
static DWORD m_dwTickCountOnSend = 0;    // starting time of sending data

static char  *m_pGetHost = NULL; // buffer of host entry
static HANDLE m_hGetHost;        // task handle of WSAAsyncGetHostByName()
static int    m_socket;   // socket
static int    m_port;     // port

// RASAPI32.dll
static HMODULE m_hRASAPI = NULL;
DWORD (WINAPI *m_pRasEnumConnections)(LPRASCONN, LPDWORD, LPDWORD);
DWORD (WINAPI *m_pRasGetConnectStatus)(HRASCONN, LPRASCONNSTATUS);

struct NTP_Packet {          // NTP packet
	unsigned int Control_Word;
	int root_delay;
	int root_dispersion;
	int reference_identifier;
	__int64 reference_timestamp;
	__int64 originate_timestamp;
	__int64 receive_timestamp;
	int transmit_timestamp_seconds;
	int transmit_timestamp_fractions;
};

/*---------------------------------------------------
  initialize WinSock and read settings
---------------------------------------------------*/
BOOL InitSNTP(HWND hwndMain)
{
	WORD ver;
	WSADATA wsaData;
	char s[80];
	
	m_socket = -1;
	m_hGetHost = NULL;
	
	// initialize WinSock
	ver = 0x0101; // MAKEWORD(1, 1);
	if(WSAStartup(ver, &wsaData) != 0)
	{
		Log(NULL, "failed to initialize");
		return FALSE;
	}
	
	GetMyRegStr(m_section, "Server", m_servername, 80, "");
	m_nTimeOut = GetMyRegLong(m_section, "Timeout", 1000);
	if(!(0 < m_nTimeOut && m_nTimeOut < 30000))
		m_nTimeOut = 1000;
	m_bSaveLog = GetMyRegLong(m_section, "SaveLog", TRUE);
	GetMyRegStr(m_section, "Sound", m_soundfile, MAX_PATH, "");
	
	m_nMinuteDif = 0;
	GetMyRegStr(m_section, "Dif", s, 80, "");
	if(s[0])
	{
		int h, m;
		time2int(&h, &m, s);
		m_nMinuteDif = h * 60 + m;
	}
	
	return TRUE;
}

/*-------------------------------------------
  clean up
---------------------------------------------*/
void EndSNTP(HWND hwndMain)
{
	SocketClose(hwndMain, NULL);
	
	if(m_hRASAPI) FreeLibrary(m_hRASAPI);
	m_hRASAPI = NULL;
	
	WSACleanup();
}

/*-------------------------------------------
  set options
  called in dialog.c
---------------------------------------------*/
void SetSNTPParam(const char *servername, int nTimeOut, BOOL bLog,
	const char *soundfile)
{
	if(servername) strcpy(m_servername, servername);
	m_nTimeOut = nTimeOut;
	if(0 < nTimeOut && nTimeOut < 30000)
		m_nTimeOut = nTimeOut;
	m_bSaveLog = bLog;
	if(soundfile) strcpy(m_soundfile, soundfile);
}

/*-------------------------------------------
  start SNTP session
  check RAS connection and call SNTPStart()
---------------------------------------------*/
BOOL StartSyncTime(HWND hwnd, const char *servername, BOOL bRAS)
{
	if(m_socket != -1 || m_hGetHost != NULL) return FALSE;
	
	if(servername) strcpy(m_servername, servername);
	
	if(bRAS && !IsRASConnection()) return FALSE;
	
	return SNTPStart(hwnd, m_servername);
}

/*------------------------------------------------
  called when main window received WM_TIMER
--------------------------------------------------*/
void OnTimerSNTP(HWND hwnd)
{
	if(m_bSendingData) // while sending/receiving
	{
		char msg[80];
		DWORD dif;
		dif = GetTickCount() - m_dwTickCountOnSend;
		if(dif >= (DWORD)m_nTimeOut)  // check timeout
		{
			wsprintf(msg, "timeout (%04d)", dif);
			SocketClose(hwnd, msg);
			PostMessage(hwnd, SNTPM_ERROR, 0, 0);
			return;
		}
	}
}

/*---------------------------------------------------
  check RAS connection
---------------------------------------------------*/
BOOL IsRASConnection(void)
{
	RASCONN rc;
	RASCONNSTATUS rcs;
	DWORD cb, cConnections;
	
	// load DLL of RAS API
	
	if(!m_hRASAPI && !GetMyRegLong(m_section, "NoRASAPI", FALSE))
	{
		m_hRASAPI = LoadLibrary("RASAPI32.dll");
		if(m_hRASAPI)
		{
			(FARPROC)m_pRasEnumConnections =
				GetProcAddress(m_hRASAPI, "RasEnumConnectionsA");
			(FARPROC)m_pRasGetConnectStatus =
				GetProcAddress(m_hRASAPI, "RasGetConnectStatusA");
			if(m_pRasEnumConnections == NULL || m_pRasGetConnectStatus == NULL)
			{
				FreeLibrary(m_hRASAPI); m_hRASAPI = NULL;
			}
		}
	}
	
	if(!m_hRASAPI) return FALSE;
	
	memset(&rc, 0, sizeof(rc));
	rc.dwSize = sizeof(rc);
	cb = sizeof(rc);
	if(m_pRasEnumConnections(&rc, &cb, &cConnections) == 0 &&
		cConnections > 0)
	{
		memset(&rcs, 0, sizeof(rcs));
		rcs.dwSize = sizeof(rcs);
		if(m_pRasGetConnectStatus(rc.hrasconn, &rcs) == 0 &&
			rcs.rasconnstate == RASCS_Connected) return TRUE;
	}
	return FALSE;
}

/*---------------------------------------------------
  start SNTP session
---------------------------------------------------*/
BOOL SNTPStart(HWND hwndSNTP, const char *pServer)
{
	char servername[BUFSIZE_SERVER];
	unsigned long serveraddr;
	
	if(m_socket != -1 || m_hGetHost != NULL) return FALSE;
	
	// get server name and port
	m_port = GetServerPort(pServer, servername);
	if(m_port == -1)
	{
		Log(hwndSNTP, "invalid server name"); return FALSE;
	}
	
	// make a socket
	m_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(m_socket == INVALID_SOCKET)
	{
		Log(hwndSNTP, "socket() failed");
		m_socket = -1; return FALSE;
	}
	
	serveraddr = inet_addr(servername);
	// if server name is not "XXX.XXX.XXX.XXX"
	if(serveraddr == (unsigned long)-1)
	{
		// request IP address
		m_pGetHost = malloc(MAXGETHOSTSTRUCT);
		m_hGetHost = WSAAsyncGetHostByName(hwndSNTP, WSOCK_GETHOST,
			servername, m_pGetHost, MAXGETHOSTSTRUCT);
		if(m_hGetHost == NULL)
		{
			free(m_pGetHost); m_pGetHost = NULL;
			SocketClose(hwndSNTP, "WSAAsyncGetHostByName() failed");
			return FALSE;
		}
		m_dwTickCountOnGetHost = GetTickCount();
		return TRUE;
	}
	
	// send data
	SNTPSend(hwndSNTP, serveraddr);
	return TRUE;
}

/*---------------------------------------------------
  called when the window received WSOCK_GETHOST.
  get IP address and send data.
---------------------------------------------------*/
void OnGetHost(HWND hwndSNTP, WPARAM wParam, LPARAM lParam)
{
	struct hostent *pHostEnt;
	unsigned long serveraddr;
	
	if(m_hGetHost == NULL || m_pGetHost == NULL) return;
	
	// valid handle ?
	if(m_hGetHost != (HANDLE)wParam) return;
	
	// success ?
	if(WSAGETASYNCERROR(lParam) != 0)
	{
		SocketClose(hwndSNTP, "failed to get IP address");
		PostMessage(hwndSNTP, SNTPM_ERROR, 0, 0);
		return;
	}
	
	// get IP address
	pHostEnt = (struct hostent *)m_pGetHost;
	serveraddr =  *((unsigned long *)((pHostEnt->h_addr_list)[0]));
	free(m_pGetHost); m_pGetHost = NULL;
	m_hGetHost = NULL;
	
	// send data
	SNTPSend(hwndSNTP, serveraddr);
}

/*---------------------------------------------------
  send SNTP data
---------------------------------------------------*/
void SNTPSend(HWND hwndSNTP, unsigned long serveraddr)
{
	struct sockaddr_in serversockaddr;
	struct NTP_Packet NTP_Send;
	unsigned int sntpver;
	unsigned int Control_Word;
	
	// request notification of events
	if(WSAAsyncSelect(m_socket, hwndSNTP, WSOCK_SELECT, FD_READ)
		== SOCKET_ERROR)
	{
		SocketClose(hwndSNTP, "WSAAsyncSelect() failed");
		PostMessage(hwndSNTP, SNTPM_ERROR, 0, 0);
		return;
	}
	
	// set IP address and port
	serversockaddr.sin_family = AF_INET;
	serversockaddr.sin_addr.s_addr = serveraddr;
	serversockaddr.sin_port = htons((unsigned short)m_port);
	memset(serversockaddr.sin_zero,(int)0,sizeof(serversockaddr.sin_zero));
	
	
	// init a packet
	memset(&NTP_Send, 0, sizeof(struct NTP_Packet));
	// NTP/SNTP version number = 4
	// Mode = 3 (client)
	// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	// |LI | VN  |Mode |    Stratum    |     Poll      |   Precision   |
	// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	sntpver = GetMyRegLong(m_section, "SNTPVer", 4);
	Control_Word = (sntpver << 27) | (3 << 24);
	NTP_Send.Control_Word = htonl(Control_Word);
	
	// send a packet
	if(sendto(m_socket, (const char *)&NTP_Send, sizeof(NTP_Send), 0, 
		(struct sockaddr *)&serversockaddr,
		sizeof(serversockaddr)) == SOCKET_ERROR)
	{
		SocketClose(hwndSNTP, "sendto() failed");
		PostMessage(hwndSNTP, SNTPM_ERROR, 0, 0);
		return;
	}
	
	// save tickcount
	m_dwTickCountOnSend = GetTickCount();
	m_bSendingData = TRUE;
}

/*---------------------------------------------------
  called when the window received WSOCK_SELECT.
  receive SNTP data and set time.
---------------------------------------------------*/
void OnReceive(HWND hwndSNTP, WPARAM wParam, LPARAM lParam)
{
	struct sockaddr_in serversockaddr;
	struct NTP_Packet NTP_Recv;
	int sockaddr_Size;
	
	if(m_socket == -1) return;
	if(WSAGETSELECTERROR(lParam))
	{
		SocketClose(hwndSNTP, "failed to receive");
		return;
	}
	if(m_socket != (int)wParam ||
		WSAGETSELECTEVENT(lParam) != FD_READ) return;
	
	// receive data
	sockaddr_Size = sizeof(serversockaddr);
	if(recvfrom(m_socket, (char *)&NTP_Recv, sizeof(NTP_Recv), 0,
		(struct sockaddr *)&serversockaddr, &sockaddr_Size) == SOCKET_ERROR)
	{
		SocketClose(hwndSNTP, "recvfrom() failed");
		return;
	}
	
	// if Leap Indicator is 3
	/*
	if(ntohl(NTP_Recv.Control_Word) >> 30 == 3)
	{
		SocketClose(hwndSNTP, "server is unhealthy");
		return;
	}
	*/
	
	// set system time
	SynchronizeSystemTime(hwndSNTP,
		ntohl(NTP_Recv.transmit_timestamp_seconds),
		ntohl(NTP_Recv.transmit_timestamp_fractions));
	
	// close socket
	SocketClose(hwndSNTP, NULL);
}

/*---------------------------------------------------
  set system time to received data
---------------------------------------------------*/
void SynchronizeSystemTime(HWND hwndSNTP, DWORD seconds, DWORD fractions)
{
	FILETIME ft, ftold;
	SYSTEMTIME st, st_dif;
	char s[MAX_PATH];
	DWORD sr_time;
	DWORDLONG dif;
	BOOL b;
	
	// timeout ?
	sr_time = GetTickCount() - m_dwTickCountOnSend;
	if(sr_time >= (DWORD)m_nTimeOut)
	{
		wsprintf(s, "timeout (%04d)", sr_time);
		Log(hwndSNTP, s);
		PostMessage(hwndSNTP, SNTPM_ERROR, 0, 0);
		return;
	}
	
	// current time
	GetSystemTimeAsFileTime(&ftold);
	
	// NTP data -> FILETIME
	*(DWORDLONG*)&ft =
		// seconds from 1900/01/01 ¨ 100 nano-seconds from 1601/01/01
		M32x32to64(seconds, 10000000) + 94354848000000000i64;
	
	// difference
	if(m_nMinuteDif > 0)
		*(DWORDLONG*)&ft += M32x32to64(m_nMinuteDif * 60, 10000000);
	else if(m_nMinuteDif < 0)
		*(DWORDLONG*)&ft -= M32x32to64(-m_nMinuteDif * 60, 10000000);
	
	// set system time
	b = FileTimeToSystemTime(&ft, &st);
	if(b)
	{
		/* fractions: (2 ** 32 / 1000) */
		st.wMilliseconds = (WORD)(fractions / 4294967);
		b = SetSystemTime(&st);
	}
	if(!b)
	{
		Log(hwndSNTP, "failed to set time");
		PostMessage(hwndSNTP, SNTPM_ERROR, 0, 0);
		return;
	}
	
	SystemTimeToFileTime(&st, &ft);
	// delayed or advanced
	b = (*(DWORDLONG*)&ft > *(DWORDLONG*)&ftold);
	// get difference
	if(b) dif = *(DWORDLONG*)&ft - *(DWORDLONG*)&ftold;
	else  dif = *(DWORDLONG*)&ftold - *(DWORDLONG*)&ft;
	FileTimeToSystemTime((FILETIME*)&dif, &st_dif);
	
	// save log
	strcpy(s, "synchronized ");
	if(st_dif.wYear == 1601 && st_dif.wMonth == 1 &&
		st_dif.wDay == 1 && st_dif.wHour == 0)
	{
		strcat(s, b?"+":"-");
		wsprintf(s + strlen(s), "%02d:%02d.%03d ",
			st_dif.wMinute, st_dif.wSecond, st_dif.wMilliseconds);
	}
	wsprintf(s + strlen(s), "(%04d)", sr_time);
	Log(hwndSNTP, s);
	
	if(m_soundfile[0])
	{
		HWND hwndTClockMain = GetTClockMainWindow();
		
		if(hwndTClockMain)
			SendStringToOther(hwndTClockMain, hwndSNTP,
				m_soundfile, COPYDATA_SOUND);
	}
	
	PostMessage(hwndSNTP, SNTPM_SUCCESS, 0, 0);
}

/*---------------------------------------------------
  close the socket
---------------------------------------------------*/
void SocketClose(HWND hwndSNTP, const char *msgbuf)
{
	if(!hwndSNTP) return;
	
	// cancel task handle of WSAAsyncGetHostByName()
	if(m_hGetHost != NULL) WSACancelAsyncRequest(m_hGetHost);
	m_hGetHost = NULL;
	// free memory
	if(m_pGetHost) free(m_pGetHost);
	m_pGetHost = NULL;
	
	if(m_socket != -1)
	{
		// cancel request of notification
		WSAAsyncSelect(m_socket, hwndSNTP, 0, 0);
		// close socket
		closesocket(m_socket);
	}
	m_socket = -1;
	m_bSendingData = FALSE;
	
	if(msgbuf) Log(hwndSNTP, msgbuf);
}

/*---------------------------------------------------
	get server name and port number from string
		buf: "ntp.xxxxx.ac.jp:123"
---------------------------------------------------*/
int GetServerPort(const char *buf, char *server)
{
	char *p;
	int port = 123;
	
	if(strcmp(buf, "") == 0) return -1;
	strcpy(server, buf);
	
	for(p = server; *p != ':' && *p != '\0'; p++);
	if(*p == ':')
	{
		*p = 0; p++; port = 0;
		while(*p)
		{
			if('0' <= *p && *p <= '9')
				port = port * 10 + *p - '0';
			else
			{
				port = -1; break;
			}
			p++;
		}
	}
	return port;
}

/*-------------------------------------------
	save log data
---------------------------------------------*/
void Log(HWND hwndSNTP, const char *msg)
{
	SYSTEMTIME st;
	char s[160];
	int pos;
	
	GetLocalTime(&st);
	wsprintf(s, "%02d/%02d %02d:%02d:%02d ",
		st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	strcat(s, msg);
	strcat(s, "\r\n");
	
	// save to edit control
	if(g_hwndLog)
	{
		pos = SendMessage(g_hwndLog, WM_GETTEXTLENGTH, 0, 0);
		SendMessage(g_hwndLog, EM_SETSEL, pos, pos);
		SendMessage(g_hwndLog, EM_REPLACESEL, 0, (LPARAM)s);
	}
	
	// save to file
	if(m_bSaveLog)
	{
		HFILE hf;
		char fname[MAX_PATH];
		
		strcpy(fname, g_mydir);
		add_title(fname, "SNTP.txt");
		hf = _lopen(fname, OF_WRITE);
		if(hf == HFILE_ERROR)
			hf = _lcreat(fname, 0);
		if(hf == HFILE_ERROR) return;
		_llseek(hf, 0, 2);
		_lwrite(hf, s, strlen(s));
		_lclose(hf);
	}
}

/*-------------------------------------------
	"XX:XX" -> two integers
---------------------------------------------*/
void time2int(int *ph, int *pm, const char *src)
{
	const char *p;
	BOOL bminus;
	
	p = src;
	*ph = 0; *pm = 0;
	bminus = FALSE;
	if(*p == '-') { p++; bminus = TRUE; }
	while('0' <= *p && *p <='9')
		*ph = *ph * 10 + *p++ - '0';
	if(bminus) *ph *= -1;
	if(*p == ':') p++; else return;
	while('0' <= *p && *p <='9')
		*pm = *pm * 10 + *p++ - '0';
	if(bminus) *pm *= -1;
}
