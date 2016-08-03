/*-------------------------------------------------------------
  startbtn.c : customize start button
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
//#include "newapi.h"

#if TC_ENABLE_STARTBUTTON

/* Globals */

void InitStartButton(HWND hwndClock);
void ResetStartButton(HWND hwndClock);
void EndStartButton(void);
void ClearStartButtonResource(void);
void CheckCursorOnStartButton(void);
BOOL StartMenuFromClock(UINT message, WPARAM wParam, LPARAM lParam);

/* Statics */

static BOOL LoadStartButtonSetting(void);
static BOOL SubClassStartButton(void);
static void UnSubclassStartButton(void);
static void InitStartButtonPos(HWND hwndClock);
static void SetTaskWinPos(HWND hwndClock, HWND hwndStart, HWND hwndTask);
static LRESULT CALLBACK WndProcStart(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK WndProcTask(HWND, UINT, WPARAM, LPARAM);
static void SetStartButtonBmp(HWND hwnd);
static void ReadStartButtonIcon(HWND hwnd,
	HBITMAP* phbmp, HICON* phicon, const char* fname);
static HFONT GetStartButtonFont(void);
static void GetIconAndCaptionSize(SIZE *psz, HDC hdc,
	HBITMAP hbmp, HICON hicon, const char* caption);
static void DrawIconAndCaption(HDC hdc, HDC hdcMem, HBITMAP hbmp, HICON hicon,
	const char* caption, int width, int height);
void DrawStartButtonBack(HWND hwnd, HDC hdc, HDC hdcMem,
	HBITMAP hbmpBack, int w, int h);
static void OnDestroyStartButton(HWND hwnd);
static void OnPaintButton(HWND hwnd, HDC hdc);
static void DrawStartButtonFrame(HDC hdc, const RECT* prc,
	BOOL bOver, BOOL bPushed);

static HWND m_hwndStart = NULL, m_hwndTask = NULL;
static HWND m_hwndClock = NULL;
static WNDPROC m_oldWndProcStart = NULL, m_oldWndProcTask = NULL;
static BOOL m_bCustomize = FALSE;
static BOOL m_bUseBackBmp = FALSE;
static BOOL m_bFlat = FALSE;
static BOOL m_bHide = FALSE;
static BOOL m_bStartMenuClock = FALSE;
static LONG m_oldClassStyle = 0;
static BOOL m_bCursorOn = FALSE;
static HBITMAP m_hbmpButton = NULL;
static HDC m_hdcButton = NULL;
static int m_wButton = -1, m_hButton = -1;
static char *m_section = "StartButton";


/*--------------------------------------------------
  initialize
----------------------------------------------------*/
void InitStartButton(HWND hwndClock)
{
	HWND hwndTaskbar, hwndTray;
	
	m_hwndClock = hwndClock;
	
	// find windows
	
	hwndTray = GetParent(hwndClock); // TrayNotifyWnd
	if(hwndTray == NULL) return;
	hwndTaskbar = GetParent(hwndTray); // Shell_TrayWnd
	if(hwndTaskbar == NULL) return;
	
	// Start Button
	m_hwndStart = FindWindowEx(hwndTaskbar, NULL, "Button", NULL);
	
	// Rebar
	m_hwndTask = FindWindowEx(hwndTaskbar, NULL, "ReBarWindow32", NULL);
	// Windows 95 without IE4
	if(m_hwndTask == NULL)
		m_hwndTask = FindWindowEx(hwndTaskbar, NULL, "MSTaskSwWClass", NULL);
	
	if((m_hwndStart == NULL) && (g_winver&WINVISTA))
	{
		DWORD dwThID1, dwThID2;
		m_hwndStart = FindWindowEx(GetParent(hwndTaskbar), NULL, "Button", NULL);
		dwThID1 = GetWindowThreadProcessId(hwndTaskbar, NULL);
		dwThID2 = GetWindowThreadProcessId(m_hwndStart, NULL);
		if(dwThID1 != dwThID2)	// Created by Explorer?
			m_hwndStart = NULL;
	}
	
	if(m_hwndStart == NULL || m_hwndTask == NULL)
		return;
	
	if(!LoadStartButtonSetting())
		return;
	
	if(m_bCustomize) SetStartButtonBmp(m_hwndStart);
	
	if(!SubClassStartButton())
	{
		ClearStartButtonResource();
		return;
	}
	
	InitStartButtonPos(hwndClock);
	
	SetTaskWinPos(hwndClock, m_hwndStart, m_hwndTask);
}

/*--------------------------------------------------
  apply changes of settings
----------------------------------------------------*/
void ResetStartButton(HWND hwndClock)
{
	BOOL bOld = (m_oldWndProcStart && m_oldWndProcTask);
	
	ClearStartButtonResource();
	
	LoadStartButtonSetting();
	
	if(m_bCustomize) SetStartButtonBmp(m_hwndStart);
	
	if(!bOld && (m_bHide || m_bCustomize))
	{
		if(!SubClassStartButton())
		{
			ClearStartButtonResource();
			return;
		}
	}
	else if(bOld && !(m_bHide || m_bCustomize))
	{
		UnSubclassStartButton();
		return;
	}
	
	InitStartButtonPos(hwndClock);
	
	SetTaskWinPos(hwndClock, m_hwndStart, m_hwndTask);
}

/*--------------------------------------------------
  clear up
----------------------------------------------------*/
void EndStartButton(void)
{
	UnSubclassStartButton();
	
	ClearStartButtonResource();
}

/*--------------------------------------------------
  delete memory DC and BMP
----------------------------------------------------*/
void ClearStartButtonResource(void)
{
	if(m_hdcButton) DeleteDC(m_hdcButton); m_hdcButton = NULL;
	if(m_hbmpButton) DeleteObject(m_hbmpButton); m_hbmpButton = NULL;
}

/*--------------------------------------------------
  Cursor position checking.
  called when clock window receive WM_TIMER.
----------------------------------------------------*/
void CheckCursorOnStartButton(void)
{
	POINT pt;
	RECT rc;
	
	if(m_hwndStart == NULL || m_bCustomize == FALSE) return;
	
	GetCursorPos(&pt);
	GetWindowRect(m_hwndStart, &rc);
	if(PtInRect(&rc, pt))
	{
		if(!m_bCursorOn)
		{
			m_bCursorOn = TRUE;
			InvalidateRect(m_hwndStart, NULL, FALSE);
		}
	}
	else
	{
		if(m_bCursorOn)
		{
			m_bCursorOn = FALSE;
			InvalidateRect(m_hwndStart, NULL, FALSE);
		}
	}
}

/*--------------------------------------------------
  Start menu from clock
----------------------------------------------------*/
BOOL StartMenuFromClock(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_LBUTTONDOWN && m_bStartMenuClock &&
		m_hwndStart && IsWindow(m_hwndStart))
	{
		SetWindowPos(m_hwndStart, NULL, 0, 0, 0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		// startmenu
		PostMessage(m_hwndStart, message, wParam, lParam);
		return TRUE;
	}
	return FALSE;
}

/*--------------------------------------------------
  read settings
----------------------------------------------------*/
BOOL LoadStartButtonSetting(void)
{
	m_bCustomize = GetMyRegLong(NULL, "StartButton", FALSE);
	m_bCustomize = GetMyRegLong(m_section, "StartButton", m_bCustomize);
	
	m_bUseBackBmp = GetMyRegLong(m_section, "UseBackBmp", FALSE);
	
	m_bFlat = GetMyRegLong(NULL, "StartButtonFlat", FALSE);
	m_bFlat = GetMyRegLong(m_section, "Flat", m_bFlat);
	
	m_bHide = GetMyRegLong(NULL, "StartButtonHide", FALSE);
	m_bHide = GetMyRegLong(m_section, "Hide", m_bHide);
	
	if(m_bHide) m_bCustomize = FALSE;
	
	m_bStartMenuClock = GetMyRegLong(NULL, "StartMenuClock", FALSE);
	m_bStartMenuClock = GetMyRegLong(m_section, "StartMenuClock",
		m_bStartMenuClock);
	
	if(!m_bCustomize && !m_bHide) return FALSE;
	return TRUE;
}

/*----------------------------------------------------------
  subclassify Start button and ReBarWindow32/MSTaskSwWClass
------------------------------------------------------------*/
BOOL SubClassStartButton(void)
{
	if(m_hwndStart == NULL || m_hwndTask == NULL) return FALSE;
	
	// check subclassification
	if(IsSubclassed(m_hwndTask))
		return FALSE;
	
	if(g_winver&WINXP)
	{
		m_oldClassStyle = GetClassLong(m_hwndStart, GCL_STYLE);
		SetClassLong(m_hwndStart, GCL_STYLE,
			m_oldClassStyle & ~(CS_HREDRAW|CS_VREDRAW));
	}
	
	m_oldWndProcStart = SubclassWindow(m_hwndStart, WndProcStart);
	m_oldWndProcTask = SubclassWindow(m_hwndTask, WndProcTask);
	
	return TRUE;
}

/*----------------------------------------------------------
  restore window procedures
------------------------------------------------------------*/
void UnSubclassStartButton(void)
{
	if(m_hwndStart && IsWindow(m_hwndStart) && m_oldWndProcStart)
	{
		if(g_winver&WINXP)
			SetClassLong(m_hwndStart, GCL_STYLE, m_oldClassStyle);
		
		SubclassWindow(m_hwndStart, m_oldWndProcStart);
		
		SetWindowPos(m_hwndStart, NULL, 0, 0, 0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	}
	m_oldWndProcStart = NULL;
	
	if(m_hwndTask && IsWindow(m_hwndTask) && m_oldWndProcTask)
	{
		SubclassWindow(m_hwndTask, m_oldWndProcTask);
	}
	m_oldWndProcTask = NULL;
}

/*----------------------------------------------------------
  initialize size and position of Start button
------------------------------------------------------------*/
void InitStartButtonPos(HWND hwndClock)
{
	HWND hwndTaskbar, hwndTray;
	
	hwndTray = GetParent(hwndClock);
	hwndTaskbar = GetParent(hwndTray);
	
	if(m_bHide) // Hide Start Button
	{
		RECT rc; POINT pt;
		m_wButton = 0; m_hButton = 0;
		GetWindowRect(hwndTray, &rc);
		pt.x = rc.left; pt.y = rc.top;
		ScreenToClient(hwndTaskbar, &pt);
		SetWindowPos(m_hwndStart, NULL, pt.x, pt.y,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER|SWP_NOACTIVATE|SWP_HIDEWINDOW);
	}
	else if(m_bCustomize)
	{
		SetWindowPos(m_hwndStart, NULL, 0, 0,
			m_wButton, m_hButton,
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
	}
}

/*------------------------------------------------------------
  initalize size and position of ReBarWindow32/MSTaskSwWClass
-------------------------------------------------------------*/
void SetTaskWinPos(HWND hwndClock, HWND hwndStart, HWND hwndTask)
{
	RECT rcBar, rcTask, rcTray;
	POINT pt;
	int x, y, w, h;
	
	GetClientRect(GetParent(hwndStart), &rcBar);  // Shell_TrayWnd
	GetWindowRect(GetParent(hwndClock), &rcTray); // TrayNotifyWnd
	GetWindowRect(hwndTask, &rcTask);     // ReBarWindow32/MSTaskSwWClass
	
	pt.x = rcTask.left; pt.y = rcTask.top;
	ScreenToClient(GetParent(hwndStart), &pt);
	
	x = pt.x; y = pt.y;
	w = rcTask.right - rcTask.left;
	h = rcTask.bottom - rcTask.top;
	
	// Taskbar is horizontal
	if(rcBar.right > rcBar.bottom)
	{
		x = 2 + m_wButton;
		w = rcTray.left - 2 - m_wButton - 2;
		if(m_wButton > 0)
		{
			x += 2; w -= 2;
		}
	}
	// Taskbar is vertical
	else
	{
		y = 2 + m_hButton;
		h = rcTray.top - 2 - m_hButton - 2;
		if(m_hButton > 0)
		{
			y += 1; h -= 2;
		}
	}
	SetWindowPos(hwndTask, NULL, x, y, w, h,
		SWP_NOZORDER|SWP_NOACTIVATE);
}

/*--------------------------------------------------
  make offscreen DC and BMP
----------------------------------------------------*/
void SetStartButtonBmp(HWND hwnd)
{
	char fname[MAX_PATH], s[MAX_PATH];
	char caption[80];
	HBITMAP hbmpIcon, hbmpBack;
	HICON hIcon;
	HDC hdc;
	HFONT hfont, hfontOld;
	COLORREF col;
	
	/* ---------- read settings and BMP file -------------- */
	
	hbmpIcon = NULL; hIcon = NULL;
	
	GetMyRegStr(NULL, "StartButtonIcon", s, MAX_PATH, "");
	GetMyRegStr(m_section, "Icon", fname, MAX_PATH, s);
	ReadStartButtonIcon(hwnd, &hbmpIcon, &hIcon, fname);
	
	GetMyRegStr(NULL, "StartButtonCaption", s, 80, "Start");
	GetMyRegStr(m_section, "Caption", caption, 80, s);
	
	col = GetMyRegLong(m_section, "CaptionColor",
		0x80000000 | COLOR_BTNTEXT);
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	
	hbmpBack = NULL;
	if(m_bUseBackBmp)
	{
		GetMyRegStr(m_section, "BackBmp", s, MAX_PATH, "");
		RelToAbs(fname, s);
		hbmpBack = ReadBitmap(hwnd, fname, TRUE);
		if(!hbmpBack) m_bUseBackBmp = FALSE;
	}
	
	/* ---------- offscreen DC -------------- */
	
	hdc = GetDC(hwnd);
	m_hdcButton = CreateCompatibleDC(hdc); // offscreen DC
	
	SetBkMode(m_hdcButton, TRANSPARENT);
	SetTextColor(m_hdcButton, col);
	
	hfont = NULL;
	if(caption[0])
	{
		hfont = GetStartButtonFont();
		if(hfont)
			hfontOld = SelectObject(m_hdcButton, hfont);
	}
	
	/* ---------- width and height of button -------------- */
	
	if(hbmpBack)
	{
		int w, h;
		GetBmpSize(hbmpBack, &w, &h);
		m_wButton = w;  m_hButton = h/3;
	}
	else
	{
		RECT rc;
		SIZE sz;
		
		GetIconAndCaptionSize(&sz,
			m_hdcButton, hbmpIcon, hIcon, caption);
		
		GetClientRect(GetParent(hwnd), &rc);
		
		m_wButton = sz.cx + 8;
		if(m_wButton > 160) m_wButton = 160;
		
		if(IsXPVisualStyle()
			&& (rc.right - rc.left > rc.bottom - rc.top))
		{
			m_hButton = 30;
			if(m_hButton > rc.bottom - rc.top)
				m_hButton = rc.bottom - rc.top;
		}
		else
			m_hButton = GetSystemMetrics(SM_CYCAPTION) + 3;
		if(sz.cy + 6 > m_hButton) m_hButton = sz.cy + 6;
		if(m_hButton > 80) m_hButton = 80;
	}
	
	/* ---------- offscreen BMP -------------- */
	
	m_hbmpButton = CreateCompatibleBitmap(hdc,
		m_wButton, m_hButton*3);
	SelectObject(m_hdcButton, m_hbmpButton);
	
	/* ---------- draw background -------------- */
	
	DrawStartButtonBack(hwnd, hdc, m_hdcButton,
		hbmpBack, m_wButton, m_hButton);
	
	/* ---------- draw icon and caption -------------- */
	
	DrawIconAndCaption(hdc, m_hdcButton, hbmpIcon, hIcon, caption,
		m_wButton, m_hButton);
	
	/* ---------- clean up -------------- */
	
	if(hfont)
	{
		SelectObject(m_hdcButton, hfontOld);
		DeleteObject(hfont);
	}
	
	if(hbmpBack) DeleteObject(hbmpBack);
	if(hbmpIcon) DeleteObject(hbmpIcon);
	if(hIcon)    DestroyIcon(hIcon);
	
	ReleaseDC(hwnd, hdc);
}

/*--------------------------------------------------
  read BMP / ICO / EXE file
----------------------------------------------------*/
void ReadStartButtonIcon(HWND hwnd,
	HBITMAP* phbmp, HICON* phicon, const char* fname)
{
	char fname2[MAX_PATH], fname3[MAX_PATH], head[2];
	HFILE hf;
	
	*phbmp = NULL; *phicon = NULL;
	if(fname[0] == 0) return;
	
	parse(fname2, fname, 0, MAX_PATH);
	RelToAbs(fname3, fname2);
	
	hf = _lopen(fname3, OF_READ);
	if(hf == HFILE_ERROR) return;
	
	_lread(hf, head, 2);
	_lclose(hf);
	
	if(head[0] == 'B' && head[1] == 'M') // bitmap
		*phbmp = ReadBitmap(hwnd, fname3, TRUE);
	else if(head[0] == 'M' && head[1] == 'Z') // executable
	{
		char numstr[10];
		HICON hiconl;
		int n;
		
		parse(numstr, fname, 1, 10);
		n = atoi(numstr);
		if(ExtractIconEx(fname3, n, &hiconl, phicon, 1) < 2)
			*phicon = NULL;
		else DestroyIcon(hiconl);
	}
	else // icon
	{
		*phicon = (HICON)LoadImage(g_hInst, fname3,
			IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR|LR_LOADFROMFILE);
	}
}

/*--------------------------------------------------
  create font of Start button
----------------------------------------------------*/
HFONT GetStartButtonFont(void)
{
	char name[LF_FACESIZE];
	int size, weight, italic;
	
	GetMyRegStr(m_section, "Font", name, LF_FACESIZE, "");
	
	if(name[0] == 0)
	{
		GetDefaultFontName(name, NULL);
	}
	
	size = GetMyRegLong(m_section, "FontSize", 9);
	if(size == 0) size = 9;
	
	weight = GetMyRegLong(m_section, "Bold", 0);
	if(weight) weight = FW_BOLD;
	else weight = 0;
	italic = GetMyRegLong(m_section, "Italic", 0);
	
	return CreateMyFont(name, size, weight, italic, 0);
}

/*--------------------------------------------------
  calculate width and height of (icon + caption)
----------------------------------------------------*/
void GetIconAndCaptionSize(SIZE *psz, HDC hdc,
	HBITMAP hbmp, HICON hicon, const char* caption)
{
	TEXTMETRIC tm;
	SIZE szCap;
	int w, h;
	
	GetTextMetrics(hdc, &tm);
	
	w = 0; h = 0;
	if(hbmp)
		GetBmpSize(hbmp, &w, &h);
	else if(hicon)
	{
		w = GetSystemMetrics(SM_CXSMICON);
		h = GetSystemMetrics(SM_CYSMICON);
	}
	
	szCap.cx = 0; szCap.cy = 0;
	if(caption[0])
	{
		if(GetTextExtentPoint32(hdc,
			caption, strlen(caption), &szCap) == 0)
			szCap.cx = strlen(caption) * tm.tmAveCharWidth;
	}
	
	psz->cx = w + 2 + szCap.cx;
	psz->cy = (h > szCap.cy) ? h : szCap.cy;
}

/*--------------------------------------------------
  draw icon and caption on offscreen DC
----------------------------------------------------*/
void DrawIconAndCaption(HDC hdc, HDC hdcMem, HBITMAP hbmp, HICON hicon,
	const char* caption, int width, int height)
{
	SIZE sz;
	TEXTMETRIC tm;
	int w, h;
	int i;
	
	GetTextMetrics(hdcMem, &tm);
	
	GetIconAndCaptionSize(&sz, hdcMem, hbmp, hicon, caption);
	
	w = 0; h = 0;
	if(hbmp)
		GetBmpSize(hbmp, &w, &h);
	else if(hicon)
	{
		w = GetSystemMetrics(SM_CXSMICON);
		h = GetSystemMetrics(SM_CYSMICON);
	}
	
	for(i = 0; i < 3; i++)
	{
		int x, y, d;
		
		x = (width - sz.cx) / 2;
		y = (height - h) / 2 + i * height;
		
		d = 0;
		if(i == 2)
		{
			if(!m_bUseBackBmp) d = 1;
		}
		
		if(hbmp)
		{
			HDC hdcTemp = CreateCompatibleDC(hdc);
			SelectObject(hdcTemp, hbmp);
			
			if((g_winver&WINME)||(g_winver&WIN2000))
				TransparentBlt(hdcMem, x + d, y + d, w, h,
					hdcTemp, 0, 0, w, h, GetSysColor(COLOR_3DFACE));
			else
				BitBlt(hdcMem, x + d, y + d, w, h, hdcTemp, 0, 0, SRCCOPY);
			
			DeleteDC(hdcTemp);
		}
		else if(hicon)
		{
			DrawIconEx(hdcMem, x + d, y + d,
				hicon, w, h, 0, NULL, DI_NORMAL);
		}
		
		if(caption)
		{
			x += w + 2;
			y = (height - tm.tmHeight) / 2 + i * height;
			
			TextOut(hdcMem, x + d, y + d, caption, strlen(caption));
		}
	}
}

/*--------------------------------------------------
  draw background on offscreen DC
----------------------------------------------------*/
void DrawStartButtonBack(HWND hwnd, HDC hdc, HDC hdcMem,
	HBITMAP hbmpBack, int w, int h)
{
	RECT rc;
	
	if(IsXPVisualStyle())
	{
		int i;
		for(i = 0; i < 3; i++)
		{
			CopyParentSurface(hwnd, hdcMem, 0, h*i, w, h, 0, 0);
		}
	}
	else
	{
		HBRUSH hbr;
		
		SetRect(&rc, 0, 0, w, h*3);
		hbr = GetSysColorBrush(COLOR_3DFACE);
		FillRect(hdcMem, &rc, hbr);
		
		if(!hbmpBack)
		{
			SetRect(&rc, 0, 0, w, h);
			DrawStartButtonFrame(hdcMem, &rc, FALSE, FALSE);
			SetRect(&rc, 0, h, w, h*2);
			DrawStartButtonFrame(hdcMem, &rc, TRUE, FALSE);
			SetRect(&rc, 0, h*2, w, h*3);
			DrawStartButtonFrame(hdcMem, &rc, TRUE, TRUE);
		}
	}
	
	if(hbmpBack)
	{
		HDC hdcTemp = CreateCompatibleDC(hdc);
		SelectObject(hdcTemp, hbmpBack);
		
		if((g_winver&WINME)||(g_winver&WIN2000))
			TransparentBlt(hdcMem, 0, 0, w, h*3,
				hdcTemp, 0, 0, w, h*3, GetSysColor(COLOR_3DFACE));
		else BitBlt(hdcMem, 0, 0, w, h*3, hdcTemp, 0, 0, SRCCOPY);
		
		DeleteDC(hdcTemp);
	}
}

/*------------------------------------------------
  subclass procedure of start button
--------------------------------------------------*/
LRESULT CALLBACK WndProcStart(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_SYSCOLORCHANGE:  // system setting is changed
		case WM_WININICHANGE:
			if(m_bCustomize)
				PostMessage(hwnd, WM_USER+10, 0, 0L);
			return 0;
		case (WM_USER + 10):     // re-create offscreen DC
			if(m_bCustomize)
			{
				ClearStartButtonResource();
				SetStartButtonBmp(hwnd);
			}
			return 0;
		case WM_WINDOWPOSCHANGING:  // restrict button size
		{
			LPWINDOWPOS pwp;
			pwp = (LPWINDOWPOS)lParam;
			if(!(pwp->flags & SWP_NOSIZE))
			{
				if(m_wButton > 0) pwp->cx = m_wButton;
				if(m_hButton > 0) pwp->cy = m_hButton;
			}
			if(m_bHide)
			{
				RECT rc; POINT pt;
				GetWindowRect(GetParent(m_hwndClock), &rc); // TrayNotifyWnd
				pt.x = rc.left; pt.y = rc.top;
				ScreenToClient(GetParent(hwnd), &pt);  // Shell_TrayWnd
				pwp->x = pt.x; pwp->y = pt.y;
				pwp->cx = rc.right - rc.left;
				pwp->cy = rc.bottom - rc.top;
			}
			break;
		}
		case WM_SETTEXT:
			return 0;
		case WM_PAINT: // draw button
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hwnd, &ps);
			OnPaintButton(hwnd, hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case BM_SETSTATE: // button status is changed
		{
			HDC hdc;
			CallWindowProc(m_oldWndProcStart, hwnd, message, wParam, lParam);
			hdc = GetDC(hwnd);
			OnPaintButton(hwnd, hdc);
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		case WM_KILLFOCUS:
		case WM_SETFOCUS:
			InvalidateRect(hwnd, NULL, FALSE);
			return 0;
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
			CheckCursorOnStartButton();
			break;
	}
	
	return CallWindowProc(m_oldWndProcStart, hwnd, message, wParam, lParam);
}

/*--------------------------------------------------
  subclass procedure of MSTaskSwWClass/ReBarWindow32
----------------------------------------------------*/
LRESULT CALLBACK WndProcTask(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_WINDOWPOSCHANGING:  // restrict size and position
		{
			LPWINDOWPOS pwp;
			RECT rcBar, rcTray;
			
			if(!m_hwndStart || !m_oldWndProcStart) break;
			
			if(!(m_bCustomize || m_bHide)) break;
			
			pwp = (LPWINDOWPOS)lParam;
			
			GetClientRect(GetParent(hwnd), &rcBar); // Shell_TrayWnd
			if(!(m_hwndClock && IsWindow(m_hwndClock))) break;
			GetWindowRect(GetParent(m_hwndClock), &rcTray); // TrayNotifyWnd
			
			// Taskbar is horizontal
			if(rcBar.right > rcBar.bottom)
			{
				pwp->x = 2 + m_wButton;
				pwp->cx = rcTray.left - 2 - m_wButton - 2;
				if(m_wButton > 0)
				{
					pwp->x += 2; pwp->cx -= 2;
				}
			}
			else // Taskbar is vertical
			{
				if(rcTray.top < pwp->y)
				{
					pwp->cy = rcBar.bottom - 2 - m_hButton - 2;
				}
				else
				{
					pwp->cy = rcTray.top - 2 - m_hButton - 2;
				}
				pwp->y = 2 + m_hButton;
				if(m_hButton > 0)
				{
					pwp->y += 1; pwp->cy -= 2;
				}
			}
			break;
		}
	}
	return CallWindowProc(m_oldWndProcTask, hwnd, message, wParam, lParam);
}

/*--------------------------------------------------
  start button drawing
----------------------------------------------------*/
void OnPaintButton(HWND hwnd, HDC hdc)
{
	BOOL bPushed;
	int y;
	
	if(!m_bCustomize || !m_hdcButton) return;
	
	bPushed = (SendMessage(hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED)?1:0;
	
	if(bPushed) y = m_hButton * 2;
	else if(m_bCursorOn && (m_bFlat || m_bUseBackBmp))
		y = m_hButton;
	else if(GetFocus() == hwnd) y = m_hButton;
	else y = 0;
	
	BitBlt(hdc, 0, 0,
		m_wButton, m_hButton, m_hdcButton, 0, y, SRCCOPY);
}

/*--------------------------------------------------
  draw frame of button
----------------------------------------------------*/
void DrawStartButtonFrame(HDC hdc, const RECT* prc, BOOL bOver, BOOL bPushed)
{
	HPEN hpen, hpenold;
	int color;
	
	if(!m_bFlat)
	{
		RECT rc;
		CopyRect(&rc, prc);
		DrawFrameControl(hdc, &rc, DFC_BUTTON,
			bPushed ?
				(DFCS_BUTTONPUSH|DFCS_PUSHED) : (DFCS_BUTTONPUSH));
		if(bOver && !bPushed)
		{
			InflateRect(&rc, -2, -2);
			DrawFocusRect(hdc, &rc);
		}
		return;
	}
	
	if(!bOver) return;
	
	color = GetSysColor(bPushed?COLOR_3DSHADOW:COLOR_3DHILIGHT);
	hpen = CreatePen(PS_SOLID, 1, color);
	hpenold = SelectObject(hdc, hpen);
	MoveToEx(hdc, prc->left, prc->top, NULL);
	LineTo(hdc, prc->right, prc->top);
	MoveToEx(hdc, prc->left, prc->top, NULL);
	LineTo(hdc, prc->left, prc->bottom);
	SelectObject(hdc, hpenold);
	DeleteObject(hpen);
	
	color = GetSysColor(bPushed?COLOR_3DHILIGHT:COLOR_3DSHADOW);
	hpen = CreatePen(PS_SOLID, 1, color);
	hpenold = SelectObject(hdc, hpen);
	MoveToEx(hdc, prc->right-1, prc->top, NULL);
	LineTo(hdc, prc->right-1, prc->bottom);
	MoveToEx(hdc, prc->left, prc->bottom-1, NULL);
	LineTo(hdc, prc->right, prc->bottom-1);
	SelectObject(hdc, hpenold);
	DeleteObject(hpen);
}

#endif	/* TC_ENABLE_STARTBUTTON */
