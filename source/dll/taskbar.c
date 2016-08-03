/*-------------------------------------------------------------
  taskbar.c : customize taskbar
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
//#include "newapi.h"

#if TC_ENABLE_TASKBAR

/* Globals */
void RefreshTaskbar(HWND hwndClock);
void InitTaskbar(HWND hwndClock);
void EndTaskbar(HWND hwndClock);
LRESULT CallOldTaskbarWndProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */
static void SubclassTaskbar(HWND hwndTaskbar);
static void UnSubclassTaskbar(HWND hwndTaskbar);
static LRESULT CALLBACK WndProcTaskBar(HWND, UINT, WPARAM, LPARAM);
static void SetRebarGrippers(HWND hwndRebar);
static void EndRebarGrippers(HWND hwndRebar);
static void SetFlatTray(HWND hwndTray);
static void EndFlatTray(HWND hwndTray);
static void SetFlatTaskbar(HWND hwndTaskbar);
static void EndFlatTaskbar(HWND hwndTaskbar);
static void SetLayeredTaskbar(HWND hwndTaskbar);
static void EndLayeredTaskbar(HWND hwndTaskbar);
static void RefreshRebar(HWND hwndRebar);

#if TC_ENABLE_STARTMENU
static WNDPROC m_oldWndProcTaskBar = NULL;
#endif
static BOOL m_bHiddenGrippers = FALSE;
static BOOL m_bFlatTaskbar = FALSE;
static BOOL m_bFlatTray = FALSE;
static BOOL m_bLayeredTaskbar = FALSE;

/*--------------------------------------------------
  various customization of task bar
----------------------------------------------------*/
void InitTaskbar(HWND hwndClock)
{
	HWND hwndTaskbar, hwndRebar, hwndTray;
	
	hwndTray = GetParent(hwndClock);
	hwndTaskbar = GetParent(hwndTray);
	hwndRebar = FindWindowEx(hwndTaskbar, NULL, "ReBarWindow32", NULL);
	
#if TC_ENABLE_STARTMENU
	SubclassTaskbar(hwndTaskbar);
#endif
	
	SetFlatTray(hwndTray);       // flat tray
	
	SetRebarGrippers(hwndRebar); // hide grippers
	
	SetFlatTaskbar(hwndTaskbar); // flat task bar
	
	SetLayeredTaskbar(hwndTaskbar); // transparent task bar
}

/*--------------------------------------------------
  restore customization
----------------------------------------------------*/
void EndTaskbar(HWND hwndClock)
{
	HWND hwndTaskbar, hwndRebar, hwndTray;
	
	hwndTray = GetParent(hwndClock);
	hwndTaskbar = GetParent(hwndTray);
	hwndRebar = FindWindowEx(hwndTaskbar, NULL, "ReBarWindow32", NULL);
	
#if TC_ENABLE_STARTMENU
	UnSubclassTaskbar(hwndTaskbar);
#endif
	
	EndLayeredTaskbar(hwndTaskbar);
	
	EndFlatTaskbar(hwndTaskbar);
	
	EndRebarGrippers(hwndRebar);
	
	EndFlatTray(hwndTray);
}

/*--------------------------------------------------
  redraw taskbar
----------------------------------------------------*/
void RefreshTaskbar(HWND hwndClock)
{
	HWND hwndTaskbar, hwndRebar, hwndTray, hwndStartButton;
	
	hwndTray = GetParent(hwndClock);
	hwndTaskbar = GetParent(hwndTray);
	hwndRebar = FindWindowEx(hwndTaskbar, NULL, "ReBarWindow32", NULL);
	hwndStartButton = FindWindowEx(hwndTaskbar, NULL, "Button", NULL);
	
	InvalidateRect(hwndStartButton, NULL, TRUE);
	InvalidateRect(hwndTray, NULL, TRUE);
	PostMessage(hwndTray, WM_SIZE, SIZE_RESTORED, 0);
	
	RefreshRebar(hwndRebar);
	PostMessage(hwndRebar, WM_SIZE, SIZE_RESTORED, 0);
	
	InvalidateRect(hwndTaskbar, NULL, TRUE);
	PostMessage(hwndTaskbar, WM_SIZE, SIZE_RESTORED, 0);
}

#if TC_ENABLE_STARTMENU
/*--------------------------------------------------
  call default window procedure of task bar
  this is called in startmenu.c
----------------------------------------------------*/
LRESULT CallOldTaskbarWndProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	if(m_oldWndProcTaskBar)
		return CallWindowProc(m_oldWndProcTaskBar, hwnd,
			message, wParam, lParam);
	return 0;
}

/*--------------------------------------------------
  subclassify Task bar
----------------------------------------------------*/
void SubclassTaskbar(HWND hwndTaskbar)
{
	BOOL bSubclass;
	
	bSubclass = FALSE;
	if(!g_bIE4) bSubclass = TRUE; // old Windows 95
	
	if(bSubclass)
	{
		if(m_oldWndProcTaskBar) return;
		
		if(IsSubclassed(hwndTaskbar)) return;
		
		m_oldWndProcTaskBar = SubclassWindow(hwndTaskbar, WndProcTaskBar);
	}
}

/*--------------------------------------------------
  restore window procedure of Task bar
----------------------------------------------------*/
void UnSubclassTaskbar(HWND hwndTaskbar)
{
	if(hwndTaskbar && IsWindow(hwndTaskbar))
	{
		if (m_oldWndProcTaskBar)
			SubclassWindow(hwndTaskbar, m_oldWndProcTaskBar);
	}
	m_oldWndProcTaskBar = NULL;
}

/*------------------------------------------------
  window procedure of subclassified task bar
--------------------------------------------------*/
LRESULT CALLBACK WndProcTaskBar(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		// owner-drawn start menu (Win95 without IE 4)
		case WM_DRAWITEM:
			// startmenu.c
			if(OnDrawItemStartMenu(hwnd, (DRAWITEMSTRUCT*)lParam))
				return 1;
			break;
		case WM_EXITSIZEMOVE:
			PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			break;
		
		/*
		case WM_MOUSEWHEEL:
			PostMessage(hwndTClockMain, message, wParam, lParam);
			break;
		*/
	}
	return CallWindowProc(m_oldWndProcTaskBar, hwnd, message, wParam, lParam);
}
#endif	/* TC_ENABLE_STARTMENU */

/*--------------------------------------------------
  hide grippers
----------------------------------------------------*/
void SetRebarGrippers(HWND hwndRebar)
{
	COLORSCHEME colScheme;
	
	if(!hwndRebar || g_bVisualStyle) return;
	
	EndRebarGrippers(hwndRebar);
	
	m_bHiddenGrippers  = GetMyRegLong(NULL, "RebarGripperHide", FALSE);
	
	if(!m_bHiddenGrippers) return;
	
	colScheme.dwSize = sizeof(colScheme);
	colScheme.clrBtnHighlight = GetSysColor(COLOR_3DFACE);
	colScheme.clrBtnShadow = GetSysColor(COLOR_3DFACE);
	
	SendMessage(hwndRebar, RB_SETCOLORSCHEME,
		0, (LPARAM) (LPCOLORSCHEME)&colScheme);
}

/*--------------------------------------------------
  restore "hide grippers"
----------------------------------------------------*/
void EndRebarGrippers(HWND hwndRebar)
{
	COLORSCHEME colScheme2;
	
	if(!m_bHiddenGrippers) return;
	m_bHiddenGrippers = FALSE;
	if(!hwndRebar || g_bVisualStyle) return;
	
	colScheme2.dwSize = sizeof(colScheme2);
	colScheme2.clrBtnHighlight = GetSysColor(COLOR_3DHILIGHT);
	colScheme2.clrBtnShadow = GetSysColor(COLOR_3DSHADOW);
	
	SendMessage(hwndRebar, RB_SETCOLORSCHEME,
		0, (LPARAM) (LPCOLORSCHEME)&colScheme2);
}

/*----------------------------------------------------
   flat taskbar
------------------------------------------------------*/
void SetFlatTaskbar(HWND hwndTaskbar)
{
	STYLESTRUCT ss;
	LONG style;
	BOOL b;
	
	if(!hwndTaskbar || g_bVisualStyle) return;
	
	b = GetMyRegLong(NULL, "TaskBarBorder", FALSE);
	if(!b && m_bFlatTaskbar) EndFlatTaskbar(hwndTaskbar);
	
	m_bFlatTaskbar = b;
	if(!m_bFlatTaskbar) return;
	
	style = GetWindowLong(hwndTaskbar, GWL_STYLE);
	
	ss.styleOld = style;
	ss.styleNew = style & ~(WS_THICKFRAME|WS_BORDER);
	
	SetWindowLong(hwndTaskbar, GWL_STYLE, ss.styleNew);
	
	SendMessage(hwndTaskbar, WM_STYLECHANGED,
		GWL_STYLE|GWL_EXSTYLE, (LPARAM)&ss);
	SendMessage(hwndTaskbar, WM_SYSCOLORCHANGE, 0, 0);
	
	SetWindowPos(hwndTaskbar, NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME | SWP_FRAMECHANGED | 
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

/*--------------------------------------------------
  restore "flat taskbar"
----------------------------------------------------*/
void EndFlatTaskbar(HWND hwndTaskbar)
{
	STYLESTRUCT ss;
	LONG style;
	
	if(!hwndTaskbar || g_bVisualStyle) return;
	
	style = GetWindowLong(hwndTaskbar, GWL_STYLE);
	
	ss.styleOld = style;
	ss.styleNew = style | (WS_THICKFRAME|WS_BORDER);
	
	SetWindowLong(hwndTaskbar, GWL_STYLE, ss.styleNew);
	
	SendMessage(hwndTaskbar, WM_STYLECHANGED,
		GWL_STYLE, (LPARAM)&ss);
	SetWindowPos(hwndTaskbar, NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME | SWP_FRAMECHANGED |
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
}

/*----------------------------------------------------
  flat tray
------------------------------------------------------*/
void SetFlatTray(HWND hwndTray)
{
	LONG style;
	
	if(!hwndTray || g_bVisualStyle) return;
	
	EndFlatTray(hwndTray);
	
	m_bFlatTray = GetMyRegLong(NULL, "FlatTray", FALSE);
	if(!m_bFlatTray) return;
	
	style = GetWindowLong(hwndTray, GWL_EXSTYLE);
	
	SetWindowLong(hwndTray, GWL_EXSTYLE, style & ~WS_EX_STATICEDGE);
	SetWindowPos(hwndTray, NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
}

/*--------------------------------------------------
  restore "flat tray"
----------------------------------------------------*/
void EndFlatTray(HWND hwndTray)
{
	LONG style;
	
	if(!m_bFlatTray) return;
	m_bFlatTray = FALSE;
	if(!hwndTray || g_bVisualStyle) return;
	
	style = GetWindowLong(hwndTray, GWL_EXSTYLE);
	
	SetWindowLong(hwndTray, GWL_EXSTYLE, style|WS_EX_STATICEDGE);
	SetWindowPos(hwndTray, NULL, 0, 0, 0, 0,
		SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
}

/*--------------------------------------------------
  transparent taskbar
----------------------------------------------------*/
void SetLayeredTaskbar(HWND hwndTaskbar)
{
	LONG exstyle;
	int alpha;
	BOOL b;
	
	if(!(g_winver&WIN2000)) return;
	if(!hwndTaskbar) return;
	
	alpha = GetMyRegLong(NULL, "AlphaTaskbar", 0);
	b = (alpha > 0);
	if(!b && m_bLayeredTaskbar) EndLayeredTaskbar(hwndTaskbar);
	
	m_bLayeredTaskbar = b;
	if(!m_bLayeredTaskbar) return;
	
	alpha = 255 - (alpha * 255 / 100);
	if(alpha < 8) alpha = 8;
	else if(alpha > 255) alpha = 255;
	
	exstyle = GetWindowLong(hwndTaskbar, GWL_EXSTYLE);
	if(alpha < 255) exstyle |= WS_EX_LAYERED;
	else exstyle &= ~WS_EX_LAYERED;
	SetWindowLong(hwndTaskbar, GWL_EXSTYLE, exstyle);
	
	if(alpha == 0)
		SetLayeredWindowAttributes(hwndTaskbar,
			GetSysColor(COLOR_3DFACE), 0, LWA_COLORKEY);
	else if(alpha < 255)
		SetLayeredWindowAttributes(hwndTaskbar, 0, (BYTE)alpha, LWA_ALPHA);
}

/*--------------------------------------------------
  restore "transparent taskbar"
----------------------------------------------------*/
void EndLayeredTaskbar(HWND hwndTaskbar)
{
	LONG exstyle;
	
	if(!(g_winver&WIN2000)) return;
	if(!m_bLayeredTaskbar) return;
	m_bLayeredTaskbar = FALSE;
	if(!hwndTaskbar) return;
	
	exstyle = GetWindowLong(hwndTaskbar, GWL_EXSTYLE);
	if(exstyle & WS_EX_LAYERED)
		SetWindowLong(hwndTaskbar, GWL_EXSTYLE, exstyle&~WS_EX_LAYERED);
}

/*--------------------------------------------------
  redraw ReBarWindow32 forcely
----------------------------------------------------*/
void RefreshRebar(HWND hwndRebar)
{
	if(hwndRebar)
	{
		HWND hwnd;
		
		InvalidateRect(hwndRebar, NULL, TRUE);
		hwnd = GetWindow(hwndRebar, GW_CHILD);
		while(hwnd)
		{
			InvalidateRect(hwnd, NULL, TRUE);
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
	}
}

#endif	/* TC_ENABLE_TASKBAR */
