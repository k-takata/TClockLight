/*-------------------------------------------------------------
  player.c : sound player
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcplayer.h"

/* Statics */

static BOOL PlayByNumber(HWND hwnd, int n);
static void ClearDisp(HWND hwnd);
static void ListSoundFiles(HWND hwnd, const char *src);
static void GetFilesInDir(HWND hwnd, const char *root);

static BOOL m_bDisp = FALSE;
static int m_nDispType = 0;
static int m_nUserStr = 0;
static BOOL m_bPlaying = FALSE;
static int m_numFiles = 0;
static int m_nCurrent = 0;
static const char *m_section = "Player";

/*-------------------------------------------------------------
  initialize
---------------------------------------------------------------*/
void InitPlayer(HWND hwnd)
{
	ClearDisp(hwnd);
	
	m_bDisp = GetMyRegLong(m_section, "Disp", FALSE);
	m_nDispType = GetMyRegLong(m_section, "DispType", 1);
	m_nUserStr = GetMyRegLong(m_section, "UserStr", 0);
}

/*-------------------------------------------------------------
  WM_TIMER message
---------------------------------------------------------------*/
void OnTimerPlayer(HWND hwnd)
{
	char s[31], s2[31];
	wchar_t ws[31];
	
	if(!m_bDisp || !m_bPlaying) return;
	
	if(GetPlayingPosition(s))  // common/playfile.c
	{
		if(m_numFiles > 1)
		{
			wsprintf(s2, "[%02d]", m_nCurrent+1);
			strcat(s2, s);
		}
		else strcpy(s2, s);
		
		MultiByteToWideChar(CP_ACP, 0, s2, -1, ws, 30);
		if(m_nDispType == 0)
			SendStringToOtherW(g_hwndClock, hwnd, ws, COPYDATA_DISP1);
		else if(m_nDispType == 1)
			SendStringToOtherW(g_hwndClock, hwnd, ws, COPYDATA_CAT1);
		else if(m_nDispType == 2)
			SendStringToOtherW(g_hwndClock, hwnd, ws,
				COPYDATA_USTR0 + m_nUserStr);
	}
	else StopPlayer(hwnd);
}

/*------------------------------------------------
  TClock player
--------------------------------------------------*/
BOOL Player(HWND hwnd, const char *src)
{
	char fname[MAX_PATH], fname2[MAX_PATH];
	
	StopFile();
	ClearDisp(hwnd);
	
	if(!src || *src == 0) return FALSE;
	
	ListSoundFiles(hwnd, src);
	m_numFiles = (int)SendDlgItemMessage(hwnd, IDC_LIST, LB_GETCOUNT, 0, 0);
	if(m_numFiles == 0) return FALSE;
	m_nCurrent = 0;
	
	SendDlgItemMessage(hwnd, IDC_LIST, LB_GETTEXT, m_nCurrent,
		(LPARAM)fname);
	
	if(lstrcmpi(fname, "cdaudio") != 0)
		RelToAbs(fname2, fname);
	else strcpy(fname2, fname);
	
	m_bPlaying = PlayMCI(hwnd, fname2, 0); // common/playfile.c
	
	return m_bPlaying;
}

/*------------------------------------------------
   Playing?
--------------------------------------------------*/
BOOL IsPlayerPlaying(void)
{
	return m_bPlaying;
}

/*------------------------------------------------
   MM_MCINOTIFY message
--------------------------------------------------*/
void OnMCINotifyPlayer(HWND hwnd, WPARAM wFlags, LONG lDevID)
{
	if(!m_bPlaying) return;
	
	if(wFlags != MCI_NOTIFY_SUCCESSFUL && wFlags != MCI_NOTIFY_FAILURE)
		return;
	
	if(wFlags == MCI_NOTIFY_SUCCESSFUL)
	{
		if(m_numFiles > 1)
		{
			m_nCurrent++;
			if(m_nCurrent < m_numFiles)
			{
				m_bPlaying = PlayByNumber(hwnd, m_nCurrent);
				if(m_bPlaying) return;
			}
		}
	}
	
	StopPlayer(hwnd);
}

/*-------------------------------------------------------------
  stop playing
---------------------------------------------------------------*/
void StopPlayer(HWND hwnd)
{
	StopFile();
	ClearDisp(hwnd);
	m_bPlaying = FALSE;
	OnRequestMenu(hwnd, TRUE);
	if(!g_hDlg)
		PostMessage(hwnd, WM_CLOSE, 0, 0);
}

/*-------------------------------------------------------------
  pause/resume
---------------------------------------------------------------*/
void PausePlayer(HWND hwnd)
{
	if(m_bPlaying)
	{
		if(!PauseResume(hwnd)) StopPlayer(hwnd);
	}
}

/*-------------------------------------------------------------
  previous or next
---------------------------------------------------------------*/
void PrevNextPlayer(HWND hwnd, BOOL bNext)
{
	if(!m_bPlaying) return;
	
	if(m_numFiles > 1)
	{
		if(bNext)
		{
			if(m_nCurrent < m_numFiles - 1)
			{
				m_nCurrent++;
				m_bPlaying = PlayByNumber(hwnd, m_nCurrent);
			}
		}
		else
		{
			if(0 < m_nCurrent)
			{
				m_nCurrent--;
				m_bPlaying = PlayByNumber(hwnd, m_nCurrent);
			}
		}
		if(!m_bPlaying) StopPlayer(hwnd);
	}
	else
	{
		if(!PrevNextTrack(hwnd, bNext))
			StopPlayer(hwnd);
	}
}

/*-------------------------------------------
  add item to tcmenu*.txt
---------------------------------------------*/
void OnRequestMenu(HWND hwnd, BOOL bClear)
{
	char tcmenutxt[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	HFILE hf;
	int size;
	char *buf;
	const char *p, *np;
	static BOOL bWrite = FALSE;
	
	if(!bClear && !m_bPlaying) return;
	
	if(bClear && !bWrite) return;
	
	// common/tclang.c
	FindFileWithLangCode(tcmenutxt, GetUserDefaultLangID(), TCMENUTXT);
	
	hfind = FindFirstFile(tcmenutxt, &fd);
	if(hfind == INVALID_HANDLE_VALUE) return;
	
	FindClose(hfind);
	size = (int)fd.nFileSizeLow;
	buf = malloc(size+1);
	if(!buf) return;
	
	hf = _lopen(tcmenutxt, OF_READWRITE);
	if(hf == HFILE_ERROR) { free(buf); return; }
	_lread(hf, buf, size);
	*(buf + size) = 0;
	
	_llseek(hf, 0, 0);
	
	p = buf;
	while(*p)
	{
		if(strncmp(p, "#Player Begin", 13) == 0)
		{
			char s[160];
			
			np = nextline(p);
			_lwrite(hf, p, (int)(np - p));
			p = np;
			
			if(m_bPlaying)
			{
				wsprintf(s, "\"%s\" post %s %d\r\n",
					MyString(IDS_STOP, "Stop"), CLASS_TCLOCKPLAYER,
					PLAYERM_STOP);
				_lwrite(hf, s, (int)strlen(s));
				wsprintf(s, "\"%s\" post %s %d\r\n",
					MyString(IDS_PAUSE, "Pause"), CLASS_TCLOCKPLAYER,
					PLAYERM_PAUSE);
				_lwrite(hf, s, (int)strlen(s));
				if((m_numFiles > 1 && m_nCurrent > 0) || IsPrevNext(FALSE))
				{
					wsprintf(s, "\"%s\" post %s %d\r\n",
						MyString(IDS_PREV, "Prev"), CLASS_TCLOCKPLAYER,
						PLAYERM_PREV);
					_lwrite(hf, s, (int)strlen(s));
				}
				if((m_numFiles > 1 && m_nCurrent < m_numFiles - 1)
					|| IsPrevNext(TRUE))
				{
					wsprintf(s, "\"%s\" post %s %d\r\n",
						MyString(IDS_NEXT, "Next"), CLASS_TCLOCKPLAYER,
						PLAYERM_NEXT);
					_lwrite(hf, s, (int)strlen(s));
				}
			}
			
			while(*p)
			{
				if(strncmp(p, "#Player End", 11) == 0)
					break;
				p = nextline(p);
			}
		}
		else
		{
			np = nextline(p);
			_lwrite(hf, p, (int)(np - p));
			p = np;
		}
	}
	
	_lwrite(hf, NULL, 0); // truncate
	
	_lclose(hf);
	free(buf);
	
	bWrite = TRUE;
}

/*-------------------------------------------------------------
  play track
---------------------------------------------------------------*/
BOOL PlayByNumber(HWND hwnd, int n)
{
	char fname[MAX_PATH], fname2[MAX_PATH];
	
	if(!(0 <= n && n < m_numFiles)) return FALSE;
	
	SendDlgItemMessage(hwnd, IDC_LIST,
		LB_GETTEXT, m_nCurrent, (LPARAM)fname);
	RelToAbs(fname2, fname);
	StopFile();
	return PlayMCI(hwnd, fname2, 0); // common/playfile.c
}

/*-------------------------------------------------------------
  clear elapsed time
---------------------------------------------------------------*/
void ClearDisp(HWND hwnd)
{
	if(m_bPlaying && m_bDisp)
	{
		if(m_nDispType == 0)
			SendStringToOtherW(g_hwndClock, hwnd, L"", COPYDATA_DISP1);
		else if(m_nDispType == 1)
			SendStringToOtherW(g_hwndClock, hwnd, L"", COPYDATA_CAT1);
		else if(m_nDispType == 2)
			SendStringToOtherW(g_hwndClock, hwnd, L"",
				COPYDATA_USTR0 + m_nUserStr);
	}
}

/*-------------------------------------------------------------
  make sound files list
---------------------------------------------------------------*/
void ListSoundFiles(HWND hwnd, const char *src)
{
	const char *p = src;
	char fname[MAX_PATH];
	BOOL bquot;
	int i;
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	BOOL bdir;
	
	m_numFiles = 0;
	SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
	
	while(*p)
	{
		bquot = FALSE;
		if(*p == '\"') { bquot = TRUE; p++; }
		for(i = 0; *p && i < MAX_PATH-1; i++)
		{
			if(bquot)
			{
				if(*p == '\"') { p++; break; }
			}
			else if(*p == ' ') break;
			fname[i] = *p++;
		}
		fname[i] = 0;
		
		bdir = FALSE;
		if((('A' <= fname[0] && fname[0] <= 'Z') ||
				('a' <= fname[0] && fname[0] <= 'z')) &&
			fname[1] == ':' &&
			fname[2] == '\\' && fname[3] == 0)
		{
			char temp[80];
			strcpy(temp, fname);
			strcat(temp, "*.cda");
			if(!IsFile(temp)) bdir = TRUE;
		}
		else
		{
			hfind = FindFirstFile(fname, &fd);
			if(hfind != INVALID_HANDLE_VALUE)
			{
				FindClose(hfind);
				if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					bdir = TRUE;
			}
		}
		
		if(bdir)
			GetFilesInDir(hwnd, fname);
		else
		{
			if(IsSoundFile(fname))
				SendDlgItemMessage(hwnd, IDC_LIST,
					LB_ADDSTRING, 0, (LPARAM)fname);
		}
		
		while(*p == ' ') p++;
	}
}

/*-------------------------------------------------------------
  search sound files in directory
---------------------------------------------------------------*/
void GetFilesInDir(HWND hwnd, const char *root)
{
	char search[MAX_PATH], fname[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	strcpy(search, root);
	add_title(search, "*.*");
	
	hfind = FindFirstFile(search, &fd);
	if(hfind == INVALID_HANDLE_VALUE) return;
	while(1)
	{
		if(fd.cFileName[0] != '.')
		{
			strcpy(fname, root);
			add_title(fname, fd.cFileName);
			
			if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				GetFilesInDir(hwnd, fname); // recursion
			else
			{
				if(IsSoundFile(fname))
					SendDlgItemMessage(hwnd, IDC_LIST,
						LB_ADDSTRING, 0, (LPARAM)fname);
			}
		}
		
		if(FindNextFile(hfind, &fd) == 0) break;
	}
	
	FindClose(hfind);
}

