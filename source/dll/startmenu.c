/*-------------------------------------------------------------
  startmenu.c : customize start menu
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
#include "newapi.h"

#if TC_ENABLE_STARTMENU

/* Globals */

void InitStartMenu(HWND hwndClock);
void EndStartMenu(void);
void ResetStartMenu(HWND hwndClock);
void ClearStartMenuResource(void);
void CheckStartMenu(void);
BOOL OnDrawItemStartMenu(HWND hwnd, DRAWITEMSTRUCT* pdis);

/* Statics */

static void InitStartMenuSetting(HWND hwndClock);
static BOOL IsStartMenu(HMENU hmenu);
static void SubclassBaseBar(void);
static void UnSubclassBaseBar(void);
static void SubclassUserPaneXP(void);
static void UnSubclassUserPaneXP(void);
static void TransStartMenu(void);
static void UnTransStartMenu(void);
static LRESULT CALLBACK WndProcBaseBar(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void OnPaintStartmenuIE4(HWND hwnd, HDC hdc, BOOL bPrint);
static LRESULT CALLBACK WndProcUserPaneXP(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void OnPaintUserPaneXP(HWND hwnd, HDC hdc);

// Start menu (BaseBar) of IE 4 or later
static HWND m_hwndBaseBar = NULL;           // window handle
static WNDPROC m_oldWndProcBaseBar = NULL;  // window procedure
// Start menu (DV2ControlHost) of Windows XP
static HWND m_hwndDV2ContHost = NULL;       // window handle
static HWND m_hwndUserPaneXP = NULL;        // window handle of top pane
static WNDPROC m_oldWndProcUserPaneXP = NULL; // window procedure of top pane

static BOOL m_bStartMenu = FALSE;  // customize start menu
static BOOL m_bSubclass = FALSE;   // subclassify
static HDC m_hdcMemMenu = NULL;    // offscreen DC
static HBITMAP m_hbmpMenu = NULL;  // offscreen BMP
static COLORREF m_colMenu;         // color
static BOOL m_bTile = FALSE;       // tile
static int m_alpha = 255;          // transparency
static char *m_section = "StartMenu";

/*--------------------------------------------------
  initialize
----------------------------------------------------*/
void InitStartMenu(HWND hwndClock)
{
	InitStartMenuSetting(hwndClock);
	
	// XP
	if(g_winver&WINXP) SubclassUserPaneXP();
	
	// IE 4 or later
	if(g_bIE4) SubclassBaseBar();
	
	if((g_winver&WIN2000) && m_hwndBaseBar)
		TransStartMenu();
}

/*--------------------------------------------------
  clear up
----------------------------------------------------*/
void EndStartMenu(void)
{
	if(g_bIE4) UnSubclassBaseBar();
	
	if(g_winver&WINXP) UnSubclassUserPaneXP();
	
	if((g_winver&WIN2000) && m_alpha < 255) UnTransStartMenu();
	
	ClearStartMenuResource();
}

/*--------------------------------------------------
  refresh settings
----------------------------------------------------*/
void ResetStartMenu(HWND hwndClock)
{
	BOOL bOld = m_bSubclass;
	int oldalpha = m_alpha;
	
	ClearStartMenuResource();
	InitStartMenuSetting(hwndClock);
	
	if(!bOld && m_bSubclass)
	{
		if(g_bIE4)  SubclassBaseBar();
		if(g_winver&WINXP) SubclassUserPaneXP();
	}
	else if(bOld && !m_bSubclass)
	{
		if(g_winver&WINXP) UnSubclassUserPaneXP();
		if(g_bIE4) UnSubclassBaseBar();
	}
	
	if(g_winver&WIN2000)
	{
		if(m_alpha < 255)
		{
			if(m_hwndBaseBar)
				TransStartMenu();
			else if(m_hwndDV2ContHost)
			{
				if((GetWindowLong(m_hwndDV2ContHost, GWL_EXSTYLE)
					& WS_EX_LAYERED))
						TransStartMenu();
			}
		}
		else if(oldalpha < 255 && m_alpha == 255)
			UnTransStartMenu();
	}
}

/*--------------------------------------------------
  clear up resources
----------------------------------------------------*/
void ClearStartMenuResource(void)
{
	if(m_hdcMemMenu) DeleteDC(m_hdcMemMenu);
	m_hdcMemMenu = NULL;
	if(m_hbmpMenu) DeleteObject(m_hbmpMenu);
	m_hbmpMenu = NULL;
}

/*--------------------------------------------------
  check if start menu is destroyed
  called in OnTimerMain of wndproc.c
----------------------------------------------------*/
void CheckStartMenu(void)
{
	if(m_hwndBaseBar && !IsWindow(m_hwndBaseBar))
	{
		SubclassBaseBar();
		if((g_winver&WIN2000) && m_hwndBaseBar)
			TransStartMenu();
	}
	
	if(m_hwndDV2ContHost && !IsWindow(m_hwndDV2ContHost))
	{
		SubclassUserPaneXP();
	}
}

/*------------------------------------------------
  owner-drawn start menu (Win95 without IE 4)
  called in taskbar.c
--------------------------------------------------*/
BOOL OnDrawItemStartMenu(HWND hwnd, DRAWITEMSTRUCT* pdis)
{
	HDC hdc, hdcMem;
	HBITMAP hbmpMem;
	RECT rcBox, rcItem;
	HBRUSH hbr;
	
	if(!m_bStartMenu) return FALSE;
	if(!IsStartMenu((HMENU)pdis->hwndItem)) return FALSE;
	
	hdc = pdis->hDC;
	CopyRect(&rcItem, &(pdis->rcItem));
	GetClipBox(hdc, &rcBox); // menu size
	
	// offscreen DC and BMP
	hdcMem = CreateCompatibleDC(hdc);
	hbmpMem = CreateCompatibleBitmap(hdc, rcBox.right, rcBox.bottom);
	SelectObject(hdcMem, hbmpMem);
	hbr = GetSysColorBrush(COLOR_MENU);
	FillRect(hdcMem, &rcBox, hbr);
	
	SelectObject(hdcMem, (HFONT)GetCurrentObject(hdc, OBJ_FONT));
	
	// background and text color
	if(pdis->itemState & ODS_FOCUS)
	{
		SetTextColor(hdcMem, GetSysColor(COLOR_HIGHLIGHTTEXT));
		SetBkColor(hdcMem, GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		SetTextColor(hdcMem, GetSysColor(COLOR_MENUTEXT));
		SetBkColor(hdcMem, GetSysColor(COLOR_MENU));
	}
	
	// default drawing
	pdis->hDC = hdcMem;
	CallOldTaskbarWndProc(hwnd, WM_DRAWITEM, 0, (LPARAM)pdis);
	
	// width of "Windows95"
	rcItem.right = pdis->rcItem.left;
	
	if(rcItem.right > 0)
	{
		COLORREF col;
		
		if(!m_bTile)
		{
			col = m_colMenu;
			if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
			hbr = CreateSolidBrush(col);
			FillRect(hdcMem, &rcItem, hbr);
			DeleteObject(hbr);
		}
		
		if(m_hdcMemMenu && m_hbmpMenu)
		{
			int i, j;
			int wbmp, hbmp;
			
			GetBmpSize(m_hbmpMenu, &wbmp, &hbmp);
			
			for(i = 0; ; i++)
			{
				int y, ysrc, h, x, w;
				for(j = 0; ; j++)
				{
					y = rcBox.bottom - ((i + 1) * hbmp);
					ysrc = 0;
					h = hbmp;
					if(y < 0)
					{
						y = 0;
						ysrc = ((i + 1) * hbmp) - rcBox.bottom;
						h -= ysrc;
					}
					x = j * wbmp; w = wbmp;
					if(x + w > rcItem.right)
					{
						w -= ((j + 1) * wbmp) - rcItem.right;
					}
					if(w > 0 && h > 0)
						BitBlt(hdcMem, x, y, w, h,
							m_hdcMemMenu, 0, ysrc, SRCCOPY);
					if(!m_bTile || w < wbmp) break;
				}
				if(!m_bTile || y == 0) break;
			}
		}
	}
	
	BitBlt(hdc, 0, rcItem.top,
		pdis->rcItem.right, rcItem.bottom - rcItem.top,
		hdcMem, 0, rcItem.top, SRCCOPY);
	pdis->hDC = hdc;
	
	DeleteDC(hdcMem);
	DeleteObject(hbmpMem);
	
	return TRUE;
}

/*--------------------------------------------------
  read settings
----------------------------------------------------*/
void InitStartMenuSetting(HWND hwndClock)
{
	char fname[MAX_PATH], s[MAX_PATH];
	
	m_bStartMenu = GetMyRegLong(NULL, "StartMenu", FALSE);
	m_bStartMenu = GetMyRegLong(m_section, "StartMenu", m_bStartMenu);
	
	if(g_winver&WIN2000)
	{
		m_alpha = GetMyRegLong(NULL, "AlphaStartMenu", 0);
		m_alpha = GetMyRegLong(m_section, "Alpha", m_alpha);
		m_alpha = 255 - (m_alpha * 255 / 100);
		if(m_alpha < 8) m_alpha = 8;
		else if(m_alpha > 255) m_alpha = 255;
	}
	
	if(m_bStartMenu || m_alpha < 255) m_bSubclass = TRUE;
	
	if(!m_bStartMenu) return;
	
	m_colMenu = GetMyRegLong(NULL, "StartMenuCol",
		RGB(128, 128, 128));
	m_colMenu = GetMyRegLong(m_section, "Color", m_colMenu);
	
	m_bTile = GetMyRegLong(NULL, "StartMenuTile", FALSE);
	m_bTile = GetMyRegLong(m_section, "Tile", m_bTile);
	
	GetMyRegStr(NULL, "StartMenuBmp", s, MAX_PATH, "");
	GetMyRegStr(m_section, "Bitmap", fname, MAX_PATH, s);
	
	if(fname[0]) // load bitmap
	{
		char fname2[MAX_PATH];
		RelToAbs(fname2, fname);
		m_hbmpMenu = ReadBitmap(hwndClock, fname2, FALSE);
		if(m_hbmpMenu)
		{
			HDC hdc;
			hdc = GetDC(hwndClock);
			m_hdcMemMenu = CreateCompatibleDC(hdc);
			SelectObject(m_hdcMemMenu, m_hbmpMenu);
			ReleaseDC(hwndClock, hdc);
		}
	}
}

/*--------------------------------------------------
  HMENU is start menu ?
----------------------------------------------------*/
BOOL IsStartMenu(HMENU hmenu)
{
	int i, count, id;
	
	count = GetMenuItemCount(hmenu);
	for(i = 0; i < count; i++)
	{
		id = GetMenuItemID(hmenu, i);
		// "Help" item
		if(id == 503) return TRUE;
	}
	return FALSE;
}

/*--------------------------------------------------
  subclassify "BaseBar" (Start Menu of IE 4 or later)
----------------------------------------------------*/
void SubclassBaseBar(void)
{
	HWND hwnd, hwndChild;
	HWND hwndFound;
	char classname[80];
	
	m_hwndBaseBar = NULL;
	
	if(m_hwndDV2ContHost) return;
	
	if(!m_bSubclass) return;
	
	// search
	hwnd = GetDesktopWindow();
	hwnd = GetWindow(hwnd, GW_CHILD);
	hwndFound = NULL;
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(strcmp(classname, "BaseBar") == 0)
		{
			if(GetWindowThreadProcessId(hwnd, NULL) ==
				GetCurrentThreadId())
			{
				POINT ptParent = { 0, 0 };
				RECT rcWinChild;
				
				hwndChild = GetWindow(hwnd, GW_CHILD);
				ClientToScreen(hwnd, &ptParent);
				GetWindowRect(hwndChild, &rcWinChild);
				if(rcWinChild.left - ptParent.x == 21)
				{
					hwndFound = hwnd; break;
				}
				else if(rcWinChild.right - rcWinChild.left == 0)
				{
					if(hwndFound == NULL || (int)hwndFound > (int)hwnd)
						hwndFound = hwnd;
				}
			}
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	m_hwndBaseBar = hwndFound;
	
	/*{
		char s[80];
		wsprintf(s, "%X", m_hwndBaseBar);
		WriteDebug(s);
	}*/
	
	if(m_hwndBaseBar == NULL) return;
	
	// if(IsSubclassed(m_hwndBaseBar)) return;
	
	m_oldWndProcBaseBar = SubclassWindow(m_hwndBaseBar, WndProcBaseBar);
}

/*--------------------------------------------------
  restore window procedure of "BaseBar"
----------------------------------------------------*/
void UnSubclassBaseBar(void)
{
	if(m_hwndBaseBar && IsWindow(m_hwndBaseBar))
	{
		if(m_oldWndProcBaseBar)
			SubclassWindow(m_hwndBaseBar, m_oldWndProcBaseBar);
	}
	m_oldWndProcBaseBar = NULL;
}

/*--------------------------------------------------
  Subclassify "Desktop User Pane" window of XP Start Menu
----------------------------------------------------*/
void SubclassUserPaneXP(void)
{
	if(!m_bSubclass) return;
	
	m_hwndDV2ContHost = FindWindow("DV2ControlHost", NULL);
	if(!m_hwndDV2ContHost) return;
	m_hwndUserPaneXP = FindWindowEx(m_hwndDV2ContHost, NULL,
		"Desktop User Pane", NULL);
	if(!m_hwndUserPaneXP) return;
	
	if(IsSubclassed(m_hwndUserPaneXP)) return;
	
	m_oldWndProcUserPaneXP = SubclassWindow(m_hwndUserPaneXP,
		WndProcUserPaneXP);
}

/*--------------------------------------------------
  restore window procedure of "Desktop User Pane"
----------------------------------------------------*/
void UnSubclassUserPaneXP(void)
{
	if(m_hwndUserPaneXP && IsWindow(m_hwndUserPaneXP))
	{
		if(m_oldWndProcUserPaneXP)
		{
			SubclassWindow(m_hwndUserPaneXP, m_oldWndProcUserPaneXP);
			SendMessage(GetParent(m_hwndUserPaneXP), WM_SYSCOLORCHANGE, 0, 0);
		}
	}
	m_oldWndProcUserPaneXP = NULL;
}

/*--------------------------------------------------
  set transparency of Start menu
----------------------------------------------------*/
void TransStartMenu(void)
{
	LONG style;
	
	if(m_alpha >= 255) return;
	
	if(m_hwndDV2ContHost)
	{
		if(m_hwndUserPaneXP)
			InvalidateRect(m_hwndUserPaneXP, NULL, TRUE);
		style = GetWindowLong(m_hwndDV2ContHost, GWL_EXSTYLE);
		SetWindowLong(m_hwndDV2ContHost, GWL_EXSTYLE, style|WS_EX_LAYERED);
		MySetLayeredWindowAttributes(m_hwndDV2ContHost,
			0, (BYTE)m_alpha, LWA_ALPHA);
		//InvalidateRect(m_hwndDV2ContHost, NULL, TRUE);
	}
	if(m_hwndBaseBar)
	{
		style = GetWindowLong(m_hwndBaseBar, GWL_EXSTYLE);
		SetWindowLong(m_hwndBaseBar, GWL_EXSTYLE, style|WS_EX_LAYERED);
		MySetLayeredWindowAttributes(m_hwndBaseBar,
			0, (BYTE)m_alpha, LWA_ALPHA);
	}
}

/*--------------------------------------------------
  restore transparency of Start menu
----------------------------------------------------*/
void UnTransStartMenu(void)
{
	LONG style;
	
	if(m_hwndDV2ContHost)
	{
		style = GetWindowLong(m_hwndDV2ContHost, GWL_EXSTYLE);
		style &= ~WS_EX_LAYERED;
		SetWindowLong(m_hwndDV2ContHost, GWL_EXSTYLE, style);
		MySetLayeredWindowAttributes(m_hwndDV2ContHost,
			GetSysColor(COLOR_3DFACE), 0, LWA_COLORKEY);
		InvalidateRect(m_hwndDV2ContHost, NULL, TRUE);
	}
	if(m_hwndBaseBar)
	{
		style = GetWindowLong(m_hwndBaseBar, GWL_EXSTYLE);
		style &= ~WS_EX_LAYERED;
		SetWindowLong(m_hwndBaseBar, GWL_EXSTYLE, style);
		MySetLayeredWindowAttributes(m_hwndBaseBar,
			GetSysColor(COLOR_3DFACE), 0, LWA_COLORKEY);
	}
}

/*------------------------------------------------
  window procedure of Start Menu (BaseBar)
  of IE 4 or later
--------------------------------------------------*/
LRESULT CALLBACK WndProcBaseBar(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			if(!m_bStartMenu) break;
			hdc = BeginPaint(hwnd, &ps);
			OnPaintStartmenuIE4(hwnd, hdc, FALSE);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_PRINT:
		{
			LRESULT r;
			if(!m_bStartMenu) break;
			r = CallWindowProc(m_oldWndProcBaseBar,
				hwnd, message, wParam, lParam);
			OnPaintStartmenuIE4(hwnd, (HDC)wParam, TRUE);
			return r;
		}
		case WM_STYLECHANGED:
		{
			STYLESTRUCT* pss = (STYLESTRUCT*)lParam;
			if(m_alpha < 255 && wParam == GWL_EXSTYLE &&
				!(pss->styleNew & WS_EX_LAYERED))
			{
				TransStartMenu();
				return 0;
			}
			break;
		}
	}
	return CallWindowProc(m_oldWndProcBaseBar, hwnd, message, wParam, lParam);
}

/*------------------------------------------------
  paint StartMenu (BaseBar) of IE 4 or later
--------------------------------------------------*/
void OnPaintStartmenuIE4(HWND hwnd, HDC hdc, BOOL bPrint)
{
	RECT rc, rcWin, rcChild;
	POINT pt;
	COLORREF col;
	HBRUSH hbr;
	BITMAP bmp;
	int hClient, wClient;
	
	GetWindowRect(GetWindow(hwnd, GW_CHILD), &rcChild);
	GetWindowRect(hwnd, &rcWin);
	GetClientRect(hwnd, &rc);
	pt.x = 0; pt.y = 0;
	ClientToScreen(hwnd, &pt);
	if(pt.x == rcChild.left) return;
	
	rc.right = 21;
	wClient = rc.right; hClient = rc.bottom;
	if(bPrint)
	{
		int dx, dy;
		dx = pt.x - rcWin.left; dy = pt.y - rcWin.top;
		rc.left += dx; rc.right += dx;
		rc.top += dy; rc.bottom += dy;
	}
	
	// fill with color
	col = m_colMenu;
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	hbr = CreateSolidBrush(col);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	
	// draw bitmap
	if(m_hbmpMenu)
	{
		int i, j;
		GetObject(m_hbmpMenu, sizeof(BITMAP), &bmp);
		
		for(i = 0; ; i++)
		{
			int y, ysrc, h, x, w;
			for(j = 0; ; j++)
			{
				y = hClient - ((i + 1) * bmp.bmHeight);
				ysrc = 0;
				h = bmp.bmHeight;
				if(y < 0)
				{
					y = 0;
					ysrc = ((i + 1) * bmp.bmHeight) - hClient;
					h -= ysrc;
				}
				x = j * bmp.bmWidth; w = bmp.bmWidth;
				if(x + w > wClient)
				{
					w -= ((j + 1) * bmp.bmWidth) - wClient;
				}
				if(w > 0 && h > 0)
					BitBlt(hdc, rc.left + x, rc.top + y, w, h,
						m_hdcMemMenu, 0, ysrc, SRCCOPY);
				if(!m_bTile || w < bmp.bmWidth) break;
			}
			if(!m_bTile || y == 0) break;
		}
	}
}

/*------------------------------------------------
  window procedure of
  top child window of XP Start Menu
--------------------------------------------------*/
LRESULT CALLBACK WndProcUserPaneXP(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_NOTIFY:
			if(m_alpha < 255)
			{
				LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
				if(!(style&WS_EX_LAYERED))
					PostMessage(hwnd, WM_USER+1, 0, 0);
			}
			
			if(m_bStartMenu)
			{
				HDC hdc = GetDC(hwnd);
				OnPaintUserPaneXP(hwnd, hdc);
				ReleaseDC(hwnd, hdc);
				return 0;
			}
			break;
		case (WM_USER+1):
			if(m_alpha < 255)
				TransStartMenu();
			return 0;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			if(!m_bStartMenu) break;
			hdc = BeginPaint(hwnd, &ps);
			OnPaintUserPaneXP(hwnd, hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_CTLCOLORSTATIC:
			if(m_bStartMenu) return 0;
			break;
		case WM_DRAWITEM:
			if(m_bStartMenu) return 0;
			break;
	}
	return CallWindowProc(m_oldWndProcUserPaneXP,
		hwnd, message, wParam, lParam);
}

/*----------------------------------------------------
  paint "Desktop User Pane" window of XP Start Menu
-----------------------------------------------------*/
void OnPaintUserPaneXP(HWND hwnd, HDC hdc)
{
	RECT rc;
	COLORREF col;
	HBRUSH hbr;
	int wClient, hClient;
	
	GetClientRect(hwnd, &rc);
	
	wClient = rc.right; hClient = rc.bottom;
	
	col = m_colMenu;
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	hbr = CreateSolidBrush(col);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	
	if(m_hbmpMenu)
	{
		int x, y;
		int wbmp, hbmp;
		
		GetBmpSize(m_hbmpMenu, &wbmp, &hbmp);
		
		for(y = 0; y < hClient; y += hbmp)
		{
			for(x = 0; x < wClient; x += wbmp)
			{
				BitBlt(hdc, x, y, wbmp, hbmp, m_hdcMemMenu, 0, 0, SRCCOPY);
				if(!m_bTile) break;
			}
			if(!m_bTile) break;
		}
	}
}

#endif	/* TC_ENABLE_STARTMENU */
