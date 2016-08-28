/*-------------------------------------------------------------
  dialog.c : SNTP setting dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcsntp.h"

/* Globals */

HWND g_hDlg = NULL;
HWND g_hwndLog = NULL;

/* Statics */

static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg);
static void LoadLog(HWND hDlg);
static void OnOK(HWND hDlg);
static void OnCancel(HWND hDlg);
static void OnHelp(HWND hDlg);
static void OnNTPServer(HWND hDlg);
static void OnDelServer(HWND hDlg);
static void OnBrowse(HWND hDlg);
static void OnSyncNow(HWND hDlg);
static BOOL IsUserAdmin(void);

typedef BOOL (WINAPI *pfnIsUserAnAdmin)(void);

static const char *m_section = "SNTP";

#define DEFAULT_NTPSERVER	"ntp.jst.mfeed.ad.jp"
#define LOG_BUF_SIZE	2048

/*-------------------------------------------------------
  SNTPM_SHOWDLG message
---------------------------------------------------------*/
void OnShowDialog(HWND hwnd)
{
	if(!g_hDlg || !IsWindow(g_hDlg))
		g_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG),
			NULL, DlgProc);
	SetForegroundWindow98(g_hDlg);
}

/*-------------------------------------------
  dialog procedure
---------------------------------------------*/
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			int id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
				case IDOK:
					OnOK(hDlg); // Save Settings
					// FALL-THROUGH
				case IDCANCEL:
					OnCancel(hDlg);
					break;
				case IDC_MYHELP:
					OnHelp(hDlg);
					break;
				
				case IDC_NTPSERVER:
					if(code == CBN_EDITCHANGE || code == CBN_SELCHANGE)
						OnNTPServer(hDlg);
					break;
				case IDC_DELSERVER:
					OnDelServer(hDlg);
					break;
				case IDC_SYNCNOW:
					OnSyncNow(hDlg);
					break;
				case IDC_SYNCSOUNDBROWSE:
					OnBrowse(hDlg);
					break;
			}
			return TRUE;
		}
		case SNTPM_LOADLOG:
			LoadLog(hDlg);
			return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------
  initialize main dialog
---------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[MAX_PATH], entry[20], server[BUFSIZE_SERVER];
	UDACCEL uda[2];
	int n, count, i;
	HICON hIcon;
	
	g_hwndLog = GetDlgItem(hDlg, IDC_SNTPLOGRESULT);
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "SyncTime", g_hfontDialog);
	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	
	// common/dialog.c
	SetMyDialgPos(hDlg, 32, 32);
	
	GetMyRegStr(m_section, "Server", server, BUFSIZE_SERVER, "");
	
	count = GetMyRegLong(m_section, "ServerNum", 0);
	
	if(server[0] == 0 && count == 0)
		strcpy(server, DEFAULT_NTPSERVER);
	
	for(i = 0; i < count; i++)
	{
		wsprintf(entry, "Server%d", i + 1);
		GetMyRegStr(m_section, entry, s, 80, "");
		if(s[0]) CBAddString(hDlg, IDC_NTPSERVER, (LPARAM)s);
	}
	
	if(server[0])
	{
		i = CBFindStringExact(hDlg, IDC_NTPSERVER, server);
		if(i == LB_ERR)
		{
			CBInsertString(hDlg, IDC_NTPSERVER, 0, (LPARAM)server);
			i = 0;
		}
		CBSetCurSel(hDlg, IDC_NTPSERVER, i);
	}
	
	OnNTPServer(hDlg);
	
	UpDown_SetBuddy(hDlg, IDC_TIMEOUTSPIN, IDC_TIMEOUT);
	
	n = GetMyRegLong(m_section, "Timeout", 1000);
	if(n < 1 || 30000 < n) n = 1000;
	UpDown_SetRange(hDlg, IDC_TIMEOUTSPIN, 30000, 0);
	UpDown_SetPos(hDlg, IDC_TIMEOUTSPIN, n);
	
	uda[0].nSec = 1; uda[0].nInc = 10;
	uda[1].nSec = 2; uda[1].nInc = 100;
	UpDown_SetAccel(hDlg, IDC_TIMEOUTSPIN, 2, uda);
	
	CheckDlgButton(hDlg, IDC_SNTPLOG,
		GetMyRegLong(m_section, "SaveLog", TRUE));
	
	GetMyRegStr(m_section, "Sound", s, MAX_PATH, "");
	SetDlgItemText(hDlg, IDC_SYNCSOUND, s);
	
	Button_SetElevationRequiredState(GetDlgItem(hDlg, IDC_SYNCNOW), TRUE);
	
	LoadLog(hDlg);
}

/*-------------------------------------------
  load SNTP log
---------------------------------------------*/
void LoadLog(HWND hDlg)
{
	HANDLE hf;
	char fname[MAX_PATH];
	char buf[LOG_BUF_SIZE];
	const char *p = buf;
	DWORD dwRead;
	DWORD dwReadLen = sizeof(buf) - 1;
	BOOL ret;
	
//	if(!IsDlgButtonChecked(hDlg, IDC_SNTPLOG)) return;
	
	strcpy(fname, g_mydir);
	add_title(fname, SNTPLOG);
	hf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hf == INVALID_HANDLE_VALUE) return;
	SetFilePointer(hf, -(LONG)dwReadLen, NULL, FILE_END);
	ret = ReadFile(hf, buf, dwReadLen, &dwRead, NULL);
	CloseHandle(hf);
	if(!ret) return;
	buf[dwRead] = '\0';
	if(dwRead == dwReadLen)
	{
		while (*p && (*p != '\n'))
			++p;
		if (*p == '\n')
			++p;
	}
	SetDlgItemText(hDlg, IDC_SNTPLOGRESULT, p);
	SendMessage(g_hwndLog, EM_LINESCROLL, 0,
		SendMessage(g_hwndLog, EM_GETLINECOUNT, 0, 0) - 1);
}

/*-------------------------------------------
  "OK" button -- Save Settings
---------------------------------------------*/
void OnOK(HWND hDlg)
{
	char s[MAX_PATH], entry[20];
	char server[BUFSIZE_SERVER];
	int i, count, index;
	
	GetDlgItemText(hDlg, IDC_NTPSERVER, server, BUFSIZE_SERVER);
	SetMyRegStr(m_section, "Server", server);
	
	if(server[0])
	{
		index = CBFindStringExact(hDlg, IDC_NTPSERVER, server);
		if(index != LB_ERR)
			CBDeleteString(hDlg, IDC_NTPSERVER, index);
		CBInsertString(hDlg, IDC_NTPSERVER, 0, server);
		CBSetCurSel(hDlg, IDC_NTPSERVER, 0);
	}
	count = CBGetCount(hDlg, IDC_NTPSERVER);
	for(i = 0; i < count; i++)
	{
		CBGetLBText(hDlg, IDC_NTPSERVER, i, s);
		wsprintf(entry, "Server%d", i+1);
		SetMyRegStr(m_section, entry, s);
	}
	SetMyRegLong(m_section, "ServerNum", count);
	
//	OnNTPServer(hDlg);
	
	SetMyRegLong(m_section, "Timeout",
		UpDown_GetPos(hDlg, IDC_TIMEOUTSPIN));
	SetMyRegLong(m_section, "SaveLog",
		IsDlgButtonChecked(hDlg, IDC_SNTPLOG));
	
	GetDlgItemText(hDlg, IDC_SYNCSOUND, s, MAX_PATH);
	SetMyRegStr(m_section, "Sound", s);
}

/*-------------------------------------------
  "Cancel" button
---------------------------------------------*/
void OnCancel(HWND hDlg)
{
	DestroyWindow(hDlg);
	g_hDlg = g_hwndLog = NULL;
	PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
}

/*-------------------------------------------
  "Help" button
---------------------------------------------*/
void OnHelp(HWND hDlg)
{
	char helpurl[MAX_PATH], title[MAX_PATH];
	
	if(!g_langfile[0]) return;
	
	GetMyRegStr(NULL, "HelpURL", helpurl, MAX_PATH, "");
	if(helpurl[0] == 0)
	{
		if(GetPrivateProfileString("Main", "HelpURL", "", helpurl,
			MAX_PATH, g_langfile) == 0) return;
	}
	
	if(GetPrivateProfileString("SyncTime", "HelpURL", "", title,
		MAX_PATH, g_langfile) == 0) return;
	
	if(strlen(helpurl) > 0 && helpurl[ strlen(helpurl) - 1 ] != '/')
		del_title(helpurl);
	add_title(helpurl, title);
	
	ShellExecute(hDlg, NULL, helpurl, NULL, "", SW_SHOW);
}

/*------------------------------------------------
    When server name changed
--------------------------------------------------*/
void OnNTPServer(HWND hDlg)
{
	EnableDlgItem(hDlg, IDC_SYNCNOW,
		GetWindowTextLength(GetDlgItem(hDlg,IDC_NTPSERVER))>0);
	EnableDlgItem(hDlg, IDC_DELSERVER,
		CBGetCount(hDlg, IDC_NTPSERVER)>0);
}

/*------------------------------------------------
    Delete Server Name
--------------------------------------------------*/
void OnDelServer(HWND hDlg)
{
	char server[BUFSIZE_SERVER];
	int index, count;
	
	GetDlgItemText(hDlg, IDC_NTPSERVER, server, BUFSIZE_SERVER);
	count = CBGetCount(hDlg, IDC_NTPSERVER);
	index = CBFindStringExact(hDlg, IDC_NTPSERVER, server);
	if(index != LB_ERR)
	{
		CBDeleteString(hDlg, IDC_NTPSERVER, index);
		if(count > 1)
		{
			if(index == count - 1) index--;
			CBSetCurSel(hDlg, IDC_NTPSERVER, index);
		}
		else SetDlgItemText(hDlg, IDC_NTPSERVER, "");
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		OnNTPServer(hDlg);
	}
}

/*------------------------------------------------
  browse sound file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_SYNCSOUND, deffile, MAX_PATH);
	
	if(BrowseSoundFile(g_hInst, hDlg, deffile, fname)) // soundselect.c
	{
		SetDlgItemText(hDlg, IDC_SYNCSOUND, fname);
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	}
}

/*------------------------------------------------
  "Test"
--------------------------------------------------*/
void OnSyncNow(HWND hDlg)
{
	char server[BUFSIZE_SERVER], soundfile[MAX_PATH];
	int nTimeOut;
	BOOL bLog;
	int index;
	
	GetDlgItemText(hDlg, IDC_NTPSERVER, server, BUFSIZE_SERVER);
	if(server[0] == 0) return;
	
	nTimeOut = UpDown_GetPos(hDlg, IDC_TIMEOUTSPIN);
	if(nTimeOut & 0xffff0000) nTimeOut = 1000;
	
	if(!IsUserAdmin()) // Elevation Required
	{
		char buf[MAX_PATH];
		OnOK(hDlg); // Save Settings
		GetModuleFileName(NULL, buf, MAX_PATH);
		ShellExecute(hDlg, "runas", buf, "/silent", NULL, SW_SHOWNORMAL);
		return;
	}
	
	bLog = IsDlgButtonChecked(hDlg, IDC_SNTPLOG);
	
	GetDlgItemText(hDlg, IDC_SYNCSOUND, soundfile, MAX_PATH);
	
	SetSNTPParam(server, nTimeOut, bLog, soundfile);
	StartSyncTime(g_hwndMain, FALSE);
	
	index = CBFindStringExact(hDlg, IDC_NTPSERVER, server);
	if(index != LB_ERR)
		CBDeleteString(hDlg, IDC_NTPSERVER, index);
	CBInsertString(hDlg, IDC_NTPSERVER, 0, server);
	CBSetCurSel(hDlg, IDC_NTPSERVER, 0);
}

/*------------------------------------------------
  Does the user have admin privileges?
--------------------------------------------------*/
BOOL IsUserAdmin(void)
{
	BOOL fIsAdmin = TRUE;
	pfnIsUserAnAdmin pIsUserAnAdmin;
//	HMODULE hShell32 = LoadLibrary("shell32.dll");
	HMODULE hShell32 = GetModuleHandle("shell32.dll");
	if (hShell32 == NULL) {
		return TRUE;
	}
	pIsUserAnAdmin = (pfnIsUserAnAdmin)
			GetProcAddress(hShell32, (LPCSTR) 680);
	if (pIsUserAnAdmin != NULL) {
		fIsAdmin = pIsUserAnAdmin();
	}
//	FreeLibrary(hShell32);
	return fIsAdmin;
}
