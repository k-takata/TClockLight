/*-------------------------------------------------------------
  tooltip.c : tooltip of the clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

#define TTF_TRACK               0x0020
#define TTF_ABSOLUTE            0x0080

#define TTS_BALLOON    0x40

#define TTM_TRACKACTIVATE  (WM_USER + 17)
#define TTM_TRACKPOSITION  (WM_USER + 18)
#define TTM_SETMAXTIPWIDTH (WM_USER + 24)

/* Statics */

static void InitTooltipFormat(void);
static void ReadTooltipFormatFromFile(const char *fname, BOOL bInit);
static LRESULT CALLBACK SubclassProcTip(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

static HWND m_hwndTip = NULL;
static BOOL m_bTooltipShow = FALSE;
static BOOL m_bUpdate = FALSE;
static BOOL m_bTip1 = TRUE;
static BOOL m_bTrackActive = FALSE;
static char m_formatfile[MAX_PATH];
static wchar_t *m_format = NULL;
static wchar_t *m_format_temp = NULL;
static wchar_t *m_textToolTip = NULL;
static int m_textlen = 0;
static HFONT m_hFont = NULL;
static char *m_section = "Tooltip";

/*------------------------------------------------
  create tooltip window
--------------------------------------------------*/
void InitTooltip(HWND hwndClock)
{
	TOOLINFO ti;
	LONG style;
	char s[80];
	int n;
	
	m_bTrackActive = FALSE;
	
	style = GetMyRegLong(NULL, "BalloonFlg", 0);
	style = GetMyRegLong(m_section, "Style", style);
	
	m_hwndTip = CreateWindow(TOOLTIPS_CLASS, (LPSTR)NULL,
		WS_POPUP|TTS_NOPREFIX|TTS_ALWAYSTIP |
			((style == 1)? TTS_BALLOON : 0),
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	
	SetWindowPos(m_hwndTip, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
	
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = 0;
	ti.hwnd = hwndClock;
	ti.hinst = NULL;
	ti.uId = 1;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 480; 
	ti.rect.bottom = 480;
	
	SendMessage(m_hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	
	ti.uFlags = TTF_TRACK;
	ti.uId = 2;
	SendMessage(m_hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	
	SetWindowSubclass(m_hwndTip, SubclassProcTip, 0, 0);
	
	m_bTip1 = GetMyRegLong(m_section, "Tip1Use", TRUE);
	
	GetMyRegStr(m_section, "Font", s, 80, "");
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
	
	n = GetMyRegLong(NULL, "TipDispTime", 5);
	n = GetMyRegLong(m_section, "DispTime", n);
	SendMessage(m_hwndTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, n * 1000);
	
	SendMessage(m_hwndTip, TTM_ACTIVATE, TRUE, 0);
}

/*------------------------------------------------
  read tooltip format
--------------------------------------------------*/
void InitTooltipFormat(void)
{
	char s[BUFSIZE_TOOLTIP];
	int len;
	
	GetMyRegStr(m_section, "Tooltip", s, BUFSIZE_TOOLTIP-4,
		"\"TClock\" LDATE");
	
	m_formatfile[0] = 0;
	
	if(strncmp(s, "file:", 5) == 0)
	{
		RelToAbs(m_formatfile, s + 5);
		ReadTooltipFormatFromFile(m_formatfile, TRUE);
	}
	else if(s[0])
	{
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
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	static DWORD s_lasttime = 0;
	HFILE hf;
	int size1, size2;
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
	
	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return;
	
	temp = malloc(size1 + 1);
	_lread(hf, temp, size1);
	temp[size1] = 0;
	_lclose(hf);
	
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
  window procedure of subclassified tooltip
--------------------------------------------------*/
LRESULT CALLBACK SubclassProcTip(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if(message == WM_WINDOWPOSCHANGING)
	{
		LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
		RECT rcClock;
		int wscreen, hscreen;
		
		if(!(pwp->flags & SWP_NOMOVE))
		{
			GetWindowRect(GetClockWindow(), &rcClock);
			wscreen = GetSystemMetrics(SM_CXSCREEN);
			hscreen = GetSystemMetrics(SM_CYSCREEN);
			
			if(m_bTrackActive)
			{
				if(rcClock.left > wscreen / 2)
					pwp->x = wscreen - pwp->cx - 1;
				else
					pwp->x = 0;
				if(rcClock.top > hscreen / 2)
					pwp->y = rcClock.top - pwp->cy;
				else
					pwp->y = rcClock.bottom;
			}
			else
			{
				if(rcClock.top > hscreen / 2)
					pwp->y = rcClock.top - pwp->cy;
				else
				{
					if(pwp->y < rcClock.bottom)
						pwp->y = rcClock.bottom;
				}
			}
			
			return 0;
		}
	}
	else if(message == WM_MOUSEMOVE) return 0;
	
	return DefSubclassProc(hwnd, message, wParam, lParam);
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
	if(!m_bTip1) return;
	
	if(m_bTrackActive)
	{
		TOOLINFO ti;
		memset(&ti, 0, sizeof(ti));
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_TRACK;
		ti.hwnd = hwndClock;
		ti.uId = 2;
		ti.lpszText = LPSTR_TEXTCALLBACK;
		SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
		m_bTrackActive = FALSE;
		
		if(m_format_temp) free(m_format_temp);
		m_format_temp = NULL;
		
		SendMessage(m_hwndTip, TTM_ACTIVATE, TRUE, 0);
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
BOOL OnTooltipNotify(HWND hwndClock, LRESULT *pres, const LPNMHDR pnmh)
{
	UINT code;
	
	if(pnmh->hwndFrom != m_hwndTip) return FALSE;
	code = pnmh->code;
	
	switch(code)
	{
		case TTN_NEEDTEXT:
		case TTN_NEEDTEXTW:
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
			
			if(!pfmt || *pfmt == 0)
			{
				((LPTOOLTIPTEXTW)pnmh)->lpszText = L"";
				return TRUE;
			}
			
			SendMessage(m_hwndTip, TTM_SETMAXTIPWIDTH, 0, 1024);
			
			len = (int)wcslen(pfmt) * 2;
			if(len > m_textlen || !m_textToolTip)
			{
				if(m_textToolTip) free(m_textToolTip);
				m_textToolTip = malloc(sizeof(wchar_t) * len);
				m_textlen = len;
			}
			
			MakeFormat(m_textToolTip, NULL, pfmt, m_textlen);
			
			if(code == TTN_NEEDTEXT)
			{
				char *temp;
				int len;
				
				len = WideCharToMultiByte(CP_ACP, 0, m_textToolTip, -1,
					NULL, 0, NULL, NULL);
				temp = malloc(len + 1);
				WideCharToMultiByte(CP_ACP, 0, m_textToolTip, -1,
					temp, len, NULL, NULL);
				temp[len] = 0;
				
				strcpy((char*)m_textToolTip, temp);
				((LPTOOLTIPTEXT)pnmh)->lpszText = (char*)m_textToolTip;
				
				free(temp);
			}
			else if(code == TTN_NEEDTEXTW)
			{
				((LPTOOLTIPTEXTW)pnmh)->lpszText = m_textToolTip;
			}
			
			return TRUE;
		}
		case TTN_SHOW:
			if(!m_bTooltipShow)
			{
				SetWindowPos(m_hwndTip, HWND_TOPMOST,
					0, 0, 0, 0,
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
				m_bTooltipShow = TRUE;
			}
			break;
		case TTN_POP:
			m_bTooltipShow = FALSE;
			m_bUpdate = FALSE;
			break;
	}
	return FALSE;
}

/*------------------------------------------------
  update tooltip
--------------------------------------------------*/
void OnTimerTooltip(HWND hwndClock, BOOL forceFlg)
{
	TOOLINFO ti;
	RECT rc;
	POINT pt;
	
	if(!m_hwndTip) return;
	
	if((m_bUpdate || IsWindowVisible(m_hwndTip)) && !forceFlg) return;
	
	GetWindowRect(hwndClock, &rc);
	GetCursorPos(&pt);
	
	if(PtInRect(&rc, pt) && !forceFlg) return;
	
	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwndClock;
	ti.uId = 1;
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
	RECT rcClock, rcTip;
	int wscreen, hscreen;
	
	if(!m_hwndTip) return;
	if(!m_bTip1) return;
	
	if(m_format_temp) free(m_format_temp);
	m_format_temp = NULL;
	
	if(p && *p)
	{
		m_format_temp = malloc(sizeof(wchar_t) * (wcslen(p) + 1));
		wcscpy(m_format_temp, p);
	}
	
	memset(&ti, 0, sizeof(ti));
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_TRACK;
	ti.hwnd = hwndClock;
	ti.uId = 2;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	
	if(m_bTrackActive)
	{
		SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
		m_bTrackActive = FALSE;
	}
	
	m_bTrackActive = TRUE;
	SendMessage(m_hwndTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
	
	GetWindowRect(hwndClock, &rcClock);
	GetWindowRect(m_hwndTip, &rcTip);
	
	wscreen = GetSystemMetrics(SM_CXSCREEN);
	hscreen = GetSystemMetrics(SM_CYSCREEN);
	
	if(GetWindowLong(m_hwndTip, GWL_STYLE)&TTS_BALLOON)
	{
		int x, y;
		x = rcClock.left;
		if(x > wscreen / 2) x += rcClock.right-rcClock.left - 16;
		else x += 16;
		y = rcClock.bottom;
		if(rcClock.top > hscreen / 2) y = rcClock.top;
		
		SendMessage(m_hwndTip, TTM_TRACKPOSITION, 0,
			(LPARAM)MAKELONG(x, y));
	}
}
