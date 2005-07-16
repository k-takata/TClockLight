/*-------------------------------------------------------------
  tooltip.c : tooltip of the clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

#define TTS_BALLOON    0x40

#define TTF_TRACK      0x0020

#define TTM_TRACKACTIVATE  (WM_USER + 17)
#define TTM_TRACKPOSITION  (WM_USER + 18)
#define TTM_SETMAXTIPWIDTH (WM_USER + 24)

/* Globals */
void InitTooltip(HWND hwndClock);
void EndTooltip(HWND hwndClock);
void OnTooltipMouseMsg(HWND hwndClock,
	UINT message, WPARAM wParam, LPARAM lParam);
BOOL OnTooltipNotify(HWND hwndClock, LRESULT *pres, LPNMHDR pnmh);
void OnTimerTooltip(HWND hwndClock);
void PopupTooltip(HWND hwndClock, const wchar_t *p);

/* Statics */

static void InitTooltipFormat(void);
static void ReadTooltipFormatFromFile(const char *fname, BOOL bInit);

static HWND m_hwndTip = NULL;
static BOOL m_bUpdate = FALSE;
static BOOL m_bTrackActive = FALSE;
static char m_formatfile[MAX_PATH];
static wchar_t *m_format = NULL;
static wchar_t *m_format_temp = NULL;
static wchar_t *m_textToolTip = NULL;
static int m_textlen = 0;
static HFONT m_hFont = NULL;
static const char *m_section = "Tooltip";

/*------------------------------------------------
  create tooltip window
--------------------------------------------------*/
void InitTooltip(HWND hwndClock)
{
	TOOLINFO ti;
	LONG style;
	char s[LF_FACESIZE];
	int n;
	
	if(!GetMyRegLong(m_section, "Tip1Use", TRUE)) return;
	
	style = GetMyRegLong(NULL, "BalloonFlg", 0);
	style = GetMyRegLong(m_section, "Style", style);
	
	m_hwndTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, "",
		WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP | ((style == 1)? TTS_BALLOON : 0),
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_IDISHWND;
	ti.hwnd = hwndClock;
	ti.uId = (UINT_PTR)hwndClock;
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 0;
	ti.rect.bottom = 0;
	ti.hinst = NULL;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(m_hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	
	ti.uFlags = TTF_TRACK;
	ti.uId = 2;
	SendMessage(m_hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	
	n = GetMyRegLong(NULL, "TipDispTime", 5);
	n = GetMyRegLong(m_section, "DispTime", n);
	SendMessage(m_hwndTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, n * 1000);
	
	SendMessage(m_hwndTip, TTM_SETMAXTIPWIDTH, 0, 1024);
	
	GetMyRegStr(m_section, "Font", s, LF_FACESIZE, "");
	if(s[0])
	{
		int size, weight, italic;
		size = GetMyRegLong(m_section, "FontSize", 9);
		if(size == 0) size = 9;
		weight = GetMyRegLong(m_section, "Bold", 0);
		if(weight) weight = FW_BOLD;
		else weight = 0;
		italic = GetMyRegLong(m_section, "Italic", 0);
		
		m_hFont = CreateMyFont(s, size, weight, italic, 0);
		if(m_hFont)
			SendMessage(m_hwndTip, WM_SETFONT, (WPARAM)m_hFont, TRUE);
	}
	
	InitTooltipFormat();
}

/*------------------------------------------------
  read tooltip format
--------------------------------------------------*/
void InitTooltipFormat(void)
{
	char s[BUFSIZE_TOOLTIP];
	
	GetMyRegStr(m_section, "Tooltip", s, BUFSIZE_TOOLTIP-4, "\"TClock\" LDATE");
	
	m_formatfile[0] = 0;
	
	if(strncmp(s, "file:", 5) == 0)
	{
		RelToAbs(m_formatfile, s + 5);
		ReadTooltipFormatFromFile(m_formatfile, TRUE);
	}
	else if(s[0])
	{
		int len;
		len = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
		m_format = malloc(sizeof(wchar_t) * (len + 5));
		
		if(strstr(s, "<%"))
			MultiByteToWideChar(CP_ACP, 0, s, -1, m_format, len);
		else
		{
			wcscpy(m_format, L"<%");
			MultiByteToWideChar(CP_ACP, 0, s, -1, m_format + 2, len);
			wcscat(m_format, L"%>");
		}
	}
}

/*------------------------------------------------
  read tooltip format from file
--------------------------------------------------*/
void ReadTooltipFormatFromFile(const char *fname, BOOL bInit)
{
	static DWORD s_lasttime = 0;
	WIN32_FIND_DATA fd;
	HANDLE hfind, hFile;
	DWORD size1, dwRead;
	int size2;
	char *temp;
	
	hfind = FindFirstFile(fname, &fd);
	if(hfind == INVALID_HANDLE_VALUE) return;
	FindClose(hfind);
	if(!bInit)
	{
		if(s_lasttime == fd.ftLastWriteTime.dwLowDateTime) return;
	}
	
	if(m_format) free(m_format);
	m_format = NULL;
	
	s_lasttime = fd.ftLastWriteTime.dwLowDateTime;
	
	size1 = fd.nFileSizeLow;
	if(size1 == 0) return;
	
	hFile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return;

	temp = malloc(size1 + 1);
	ReadFile(hFile, temp, size1, &dwRead, NULL);
	CloseHandle(hFile);
	temp[size1] = 0;
	
	size2 = MultiByteToWideChar(CP_ACP, 0, temp, -1, NULL, 0);
	m_format = malloc(sizeof(wchar_t) * size2);
	MultiByteToWideChar(CP_ACP, 0, temp, -1, m_format, size2);
	
	free(temp);
}

/*------------------------------------------------
  clean up
--------------------------------------------------*/
void EndTooltip(HWND hwndClock)
{
	if(m_hwndTip) { DestroyWindow(m_hwndTip); m_hwndTip = NULL; }
	
	if(m_format) { free(m_format); m_format = NULL; }
	if(m_format_temp) { free(m_format_temp); m_format_temp = NULL; }
	if(m_textToolTip) { free(m_textToolTip); m_textToolTip = NULL; }
	m_textlen = 0;
	
	m_bTrackActive = FALSE;
	
	if(m_hFont) { DeleteObject(m_hFont); m_hFont = NULL; }
}

/*------------------------------------------------
  mouse message of clock window
--------------------------------------------------*/
void OnTooltipMouseMsg(HWND hwndClock,
	UINT message, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	DWORD pos;
	
	if(!m_hwndTip) return;
	
	if(m_bTrackActive)
	{
		TOOLINFO ti;
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = hwndClock;
		ti.uId = 2;
		SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
		m_bTrackActive = FALSE;
		
		if(m_format_temp) free(m_format_temp);
		m_format_temp = NULL;
		
		return;
	}
	
	m_bUpdate = FALSE;
	
	msg.hwnd = hwndClock;
	msg.message = message;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	pos = GetMessagePos();
	POINTSTOPOINT(msg.pt, pos);
	SendMessage(m_hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
}

/*------------------------------------------------
  WM_NOTIFY message
--------------------------------------------------*/
BOOL OnTooltipNotify(HWND hwndClock, LRESULT *pres, LPNMHDR pnmh)
{
	UINT code;
	
	if(pnmh->hwndFrom != m_hwndTip) return FALSE;
	code = pnmh->code;
	
	switch(code)
	{
		case TTN_GETDISPINFOA:
		case TTN_GETDISPINFOW:
		{
			wchar_t *pfmt;
			int len;
			
			*pres = 0;
			
			if(m_format_temp) pfmt = m_format_temp;
			else
			{
				if(m_formatfile[0])
					ReadTooltipFormatFromFile(m_formatfile, FALSE);
				pfmt = m_format;
			}
			
			if(*pfmt == 0)
				return TRUE;
			
			len = wcslen(pfmt) * 2;
			if(len > m_textlen || !m_textToolTip)
			{
				if(m_textToolTip) free(m_textToolTip);
				m_textToolTip = malloc(sizeof(wchar_t) * len);
				m_textlen = len;
			}
			
			MakeFormat(m_textToolTip, NULL, pfmt, m_textlen);
			
			if(code == TTN_GETDISPINFOA)
			{
				char *temp;
				
				len = WideCharToMultiByte(CP_ACP, 0, m_textToolTip, -1,
					NULL, 0, NULL, NULL);
				temp = malloc(len);
				WideCharToMultiByte(CP_ACP, 0, m_textToolTip, -1,
					temp, len, NULL, NULL);
				
				strcpy((char*)m_textToolTip, temp);
				free(temp);
				
				((LPNMTTDISPINFOA)pnmh)->lpszText = (char*)m_textToolTip;
			}
			else if(code == TTN_GETDISPINFOW)
			{
				((LPNMTTDISPINFOW)pnmh)->lpszText = m_textToolTip;
			}
			
			return TRUE;
		}
		case TTN_SHOW:
		{
			RECT rcClock, rcTip;
			int x, y;
			
			GetWindowRect(GetClockWindow(), &rcClock);
			GetWindowRect(m_hwndTip, &rcTip);
			
			if(rcClock.top > GetSystemMetrics(SM_CYSCREEN) / 2)
				y = rcClock.top - (rcTip.bottom - rcTip.top);
			else
				y = rcClock.bottom;
			if(m_bTrackActive)
			{
				if(rcClock.left > GetSystemMetrics(SM_CXSCREEN) / 2)
					x = rcClock.right - (rcTip.right - rcTip.left);
				else
					x = rcClock.left;
			} else
				x = rcTip.left;
			
			SetWindowPos(m_hwndTip, HWND_TOPMOST, x, y, 0, 0,
				SWP_NOACTIVATE | SWP_NOSIZE);
			
			*pres = TRUE;
			return TRUE;
		}
		case TTN_POP:
			m_bUpdate = FALSE;
			break;
	}
	return FALSE;
}

/*------------------------------------------------
  update tooltip
--------------------------------------------------*/
void OnTimerTooltip(HWND hwndClock)
{
	TOOLINFO ti;
	RECT rc;
	POINT pt;
	
	if(!m_hwndTip) return;
	
	if(m_bUpdate || IsWindowVisible(m_hwndTip)) return;
	
	GetWindowRect(hwndClock, &rc);
	GetCursorPos(&pt);
	
	if(PtInRect(&rc, pt)) return;
	
	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwndClock;
	ti.uId = (UINT_PTR)hwndClock;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(m_hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	m_bUpdate = TRUE;
}

/*------------------------------------------------
  force to show the tooltip
--------------------------------------------------*/
void PopupTooltip(HWND hwndClock, const wchar_t *p)
{
	TOOLINFO ti;
	
	if(!m_hwndTip) return;
	
	if(m_format_temp) { free(m_format_temp); m_format_temp = NULL; }
	
	if(p && *p)
	{
		m_format_temp = malloc(sizeof(wchar_t) * (wcslen(p) + 1));
		wcscpy(m_format_temp, p);
	}
	
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwndClock;
	ti.uId = 2;
	
	if(m_bTrackActive)
		SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
	
	if(GetWindowLong(m_hwndTip, GWL_STYLE)&TTS_BALLOON)
	{
		RECT rcClock;
		int x, y;
		
		GetWindowRect(hwndClock, &rcClock);
		
		if(rcClock.left > GetSystemMetrics(SM_CXSCREEN) / 2)
			x = rcClock.right - 18;
		else
			x = rcClock.left + 18;
		if(rcClock.top > GetSystemMetrics(SM_CYSCREEN) / 2)
			y = rcClock.bottom;
		else
			y = rcClock.top;
		
		SendMessage(m_hwndTip, TTM_TRACKPOSITION, 0, (LPARAM)(x | (y << 16)));
	}
	
	m_bTrackActive = TRUE;
	SendMessage(m_hwndTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
}
