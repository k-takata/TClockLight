/*-------------------------------------------------------------
  tooltip.c : tooltip of the clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

#define TTS_BALLOON    0x40

#define TTM_SETMAXTIPWIDTH (WM_USER + 24)

/* Globals */
void InitTooltip(HWND hwndClock);
void EndTooltip(HWND hwndClock);
void OnTooltipMouseMsg(HWND hwndClock,
	UINT message, WPARAM wParam, LPARAM lParam);
BOOL OnTooltipNotify(HWND hwndClock, LRESULT *pres, const LPNMHDR pnmh);
void TooltipCheckCursor(HWND hwndClock);
void TrackTooltip(HWND hwndClock, BOOL bShow);

/* Statics */

LRESULT CALLBACK WndProcTip(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static HWND m_hwndTip = NULL;
static WNDPROC m_oldWndProc = NULL;
static BOOL m_bTooltipShow = FALSE;
static BOOL m_bUpdate = FALSE;
static BOOL m_bTip1 = TRUE;
static wchar_t m_format[BUFSIZE_TOOLTIP];
static wchar_t m_textToolTip[BUFSIZE_TOOLTIP];
static char *m_section = "Tooltip";

/*------------------------------------------------
  create tooltip window
--------------------------------------------------*/
void InitTooltip(HWND hwndClock)
{
	TOOLINFO ti;
	LONG style;
	char s[BUFSIZE_TOOLTIP];
	int n;
	
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
	
	m_oldWndProc = SubclassWindow(m_hwndTip, WndProcTip);
	
	m_bTip1 = GetMyRegLong(m_section, "Tip1Use", TRUE);
	
	GetMyRegStr(m_section, "Tooltip", s, BUFSIZE_TOOLTIP-4,
		"\"TClock\" LDATE");
	if(strstr(s, "<%"))
		MultiByteToWideChar(CP_ACP, 0, s, -1, m_format, BUFSIZE_TOOLTIP-1);
	else
	{
		wcscpy(m_format, L"<%");
		MultiByteToWideChar(CP_ACP,
			0, s, -1, m_format + 2, BUFSIZE_TOOLTIP-5);
		wcscat(m_format, L"%>");
	}
	
	n = GetMyRegLong(NULL, "TipDispTime", 5);
	n = GetMyRegLong(m_section, "DispTime", n);
	SendMessage(m_hwndTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, n * 1000);
	
	SendMessage(m_hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(m_hwndTip, TTM_ACTIVATE, TRUE, 0);
}

/*------------------------------------------------
  clean up
--------------------------------------------------*/
void EndTooltip(HWND hwndClock)
{
	DestroyWindow(m_hwndTip);
}

/*------------------------------------------------
  window procedure of subclassified tooltip
--------------------------------------------------*/
LRESULT CALLBACK WndProcTip(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	if(message == WM_WINDOWPOSCHANGING)
	{
		HWND hwndClock = GetClockWindow();
		LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
		RECT rcClock, rcTip;
		int hscreen, h;
		
		if(!(pwp->flags & SWP_NOMOVE))
		{
			GetWindowRect(hwndClock, &rcClock);
			GetWindowRect(hwnd, &rcTip);
			hscreen = GetSystemMetrics(SM_CYSCREEN);
			h = rcTip.bottom - rcTip.top;
			
			if(rcClock.top > hscreen / 2)
				pwp->y = rcClock.top - h;
			else
			{
				if(pwp->y < rcClock.bottom)
					pwp->y = rcClock.bottom;
			}
			return 0;
		}
	}
	else if(message == WM_MOUSEMOVE) return 0;
	
	return CallWindowProc(m_oldWndProc, hwnd, message, wParam, lParam);
}

/*------------------------------------------------
  mouse message of clock window
--------------------------------------------------*/
void OnTooltipMouseMsg(HWND hwndClock,
	UINT message, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	
	if(!m_hwndTip) return;
	if(!m_bTip1) return;
	
	m_bUpdate = FALSE;
	
	msg.hwnd = hwndClock;
	msg.message = message;
	msg.wParam = wParam;
	msg.lParam = lParam;
	msg.time = GetMessageTime();
	msg.pt.x = LOWORD(GetMessagePos());
	msg.pt.y = HIWORD(GetMessagePos());
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
			wchar_t text[BUFSIZE_TOOLTIP];
			
			SendMessage(m_hwndTip, TTM_SETMAXTIPWIDTH, 0, 300);
			
			MakeFormat(text, NULL, m_format, BUFSIZE_TOOLTIP);
			
			if(code == TTN_NEEDTEXT)
			{
				WideCharToMultiByte(CP_ACP,
					0, text, -1,
					(char*)m_textToolTip, BUFSIZE_TOOLTIP-1, NULL, NULL);
				((LPTOOLTIPTEXT)pnmh)->lpszText = (char*)m_textToolTip;
			}
			else if(code == TTN_NEEDTEXTW)
			{
				wcscpy(m_textToolTip, text);
				((LPTOOLTIPTEXTW)pnmh)->lpszText = m_textToolTip;
			}
			*pres = 0;
			
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
	ti.uId = 1;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(m_hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&ti);
	m_bUpdate = TRUE;
}

