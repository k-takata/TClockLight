/*-------------------------------------------------------------
  wndproc.c : subclassified window procedure of clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */
BOOL  g_bDispSecond = FALSE; // draw clock every second
int   g_nBlink = 0;          // 0: no blink
							 // 1: blink (normal) 2: blink (invert color)

/* Statics */
static void OnTimerMain(HWND hwnd);
static void OnRefreshClock(HWND hwnd);
static void OnRefreshTaskbar(HWND hwnd);
static void OnRefreshStartMenu(HWND hwnd);
static void OnRefreshTooltip(HWND hwnd);
static void OnVolumeChange(HWND hwnd);
static LRESULT OnMouseDown(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static LRESULT OnMouseUp(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void OnWindowPosChanging(HWND hwnd, LPWINDOWPOS pwp);
static void OnCopyData(HWND hwnd, HWND hwndFrom, const COPYDATASTRUCT* pcds);
static void OnCopy(HWND hwnd, const wchar_t* fmt);
static void InitDaylightTimeTransition(void);
static BOOL CheckDaylightTimeTransition(const SYSTEMTIME *plt);

int   m_nBlinkSec = 0;
DWORD m_nBlinkTick = 0;


/*------------------------------------------------
  subclass procedure of the clock
--------------------------------------------------*/
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(message) // for tooltip
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
#if TC_ENABLE_WHEEL
		case WM_MOUSEWHEEL:
#endif
			OnTooltipMouseMsg(hwnd, message, wParam, lParam);
			break;
	}
	
	switch(message)
	{
		/* -------- drawing & sizing ------------- */
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			if(g_bNoClock) break;
			hdc = BeginPaint(hwnd, &ps);
			OnPaint(hwnd, hdc, NULL);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
			break;
		
		case (WM_USER+100):        // a message requesting for clock size
			if(g_bNoClock) break;  // sent from parent window
			return OnCalcRect(hwnd);
		
		case WM_WINDOWPOSCHANGING:  // size arrangement
			if(g_bNoClock) break;
			OnWindowPosChanging(hwnd, (LPWINDOWPOS)lParam);
			break;
		
		case WM_SIZE:
			if(g_bNoClock) break;
			CreateClockDC(hwnd);    // create offscreen DC
			return 0;
		case WM_SYSCOLORCHANGE:
		case WM_THEMECHANGED:
			if(g_bNoClock) break;
			CreateClockDC(hwnd);   // create offscreen DC
			InvalidateRect(hwnd, NULL, FALSE);
#if TC_ENABLE_DESKTOPICON
			SetDesktopIcons();		// desktop.c
#endif
			return 0;
		case WM_WININICHANGE:
		case WM_TIMECHANGE:
		case (WM_USER+101):
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			if(g_bNoClock) break;
			InvalidateRect(hwnd, NULL, FALSE);
			return 0;
		
		/* -------- Timers ------------- */
		
		case WM_TIMER:
			switch (wParam)
			{
				case IDTIMER_MAIN:
					OnTimerMain(hwnd); return 0;
#if TC_ENABLE_SYSINFO
				case IDTIMER_SYSINFO:
					OnTimerSysInfo();		// sysinfo.c
#if TC_ENABLE_DESKTOPICON
					SetDesktopIcons();		// desktop.c
#endif
					return 0;
#endif
			}
			if(g_bNoClock) break;
			return 0;
		
		/* -------- Mouse messages ------------- */
		
		case WM_LBUTTONDOWN:   // mouse button is down
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			return OnMouseDown(hwnd, message, wParam, lParam);
		
		case WM_LBUTTONUP:    // mouse button is up
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			return OnMouseUp(hwnd, message, wParam, lParam);
		
		case WM_MOUSEMOVE:
			return 0;
		case WM_NCRBUTTONUP:
			return 0;
		case WM_CONTEXTMENU:
			PostMessage(g_hwndTClockMain, WM_CONTEXTMENU, wParam, lParam);
			return 0;
		case WM_NCHITTEST:     // not to pass to the original wndproc
			return DefWindowProc(hwnd, message, wParam, lParam);
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATE;
#if TC_ENABLE_MOUSEDROP
		case WM_DROPFILES:     // files are dropped
			PostMessage(g_hwndTClockMain, WM_DROPFILES, wParam, lParam);
			return 0;
#endif
#if TC_ENABLE_WHEEL
		case WM_MOUSEWHEEL:  // the mouse wheel is rotated
			PostMessage(g_hwndTClockMain, WM_MOUSEWHEEL, wParam, lParam);
			return 0;
#endif
		
		case WM_NOTIFY: // tooltip
		{
			LRESULT res;
			if(OnTooltipNotify(hwnd, &res, (LPNMHDR)lParam)) return res;
			break;
		}
		
		/* messages sent from other program */
		
		case CLOCKM_EXIT:   // clean up all
			EndClock(hwnd);
			return 0;
		case CLOCKM_REFRESHCLOCK: // refresh the clock
			OnRefreshClock(hwnd);
			return 0;
		case CLOCKM_DELUSRSTR:    // clear user strings
			InitUserStr();
			return 0;
#if TC_ENABLE_TASKBAR
		case CLOCKM_REFRESHTASKBAR: // refresh other elements than clock
			OnRefreshTaskbar(hwnd);
			return 0;
#endif
#if TC_ENABLE_STARTMENU
		case CLOCKM_REFRESHSTARTMENU: // refresh Start menu
			OnRefreshStartMenu(hwnd);
			return 0;
#endif
		case CLOCKM_REFRESHTOOLTIP: // refresh tooltip
			OnRefreshTooltip(hwnd);
			return 0;
		case CLOCKM_BLINK: // blink the clock
			g_nBlink = 2;
			m_nBlinkSec = lParam;
			if(lParam) m_nBlinkTick = GetTickCount();
			return 0;
		case CLOCKM_COPY: // copy format to clipboard
			OnCopy(hwnd, NULL);
			return 0;
#if TC_ENABLE_VOLUME
		case CLOCKM_VOLCHANGE:
			OnVolumeChange(hwnd);
			return 0;
#endif
		
		case WM_COPYDATA:
			OnCopyData(hwnd, (HWND)wParam, (COPYDATASTRUCT*)lParam);
			return 0;
		
		/* WM_DESTROY is sent only when Win95+IE4/98/Me is shut down */
		
		case WM_DESTROY:
			OnDestroy(hwnd); // main2.c
			break;
	}
	
	return DefSubclassProc(hwnd, message, wParam, lParam);
}

/*------------------------------------------------
  WM_TIMER message, wParam = IDTIMER_MAIN
--------------------------------------------------*/
void OnTimerMain(HWND hwnd)
{
	static SYSTEMTIME LastTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static BOOL bTimerAdjusting = FALSE;
	SYSTEMTIME t;
	HDC hdc;
	BOOL bRedraw;
	
	GetLocalTime(&t);
	
	// adjusting milliseconds gap
	if((t.wMilliseconds > 200 ||
		((g_winver | WINNT) && t.wMilliseconds > 50)))
	{
		KillTimer(hwnd, IDTIMER_MAIN);
		SetTimer(hwnd, IDTIMER_MAIN, 1001 - t.wMilliseconds, NULL);
		bTimerAdjusting = TRUE;
	}
	else if(bTimerAdjusting)
	{
		KillTimer(hwnd, IDTIMER_MAIN);
		bTimerAdjusting = FALSE;
		SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);
	}
	
	bRedraw = FALSE;
	
	if(g_nBlink && m_nBlinkSec)
	{
		if(GetTickCount() - m_nBlinkTick > (DWORD)m_nBlinkSec*1000)
		{
			g_nBlink = 0; bRedraw = TRUE;
		}
	}
	
	if(g_nBlink > 0) bRedraw = TRUE;
	else if(g_bDispSecond) bRedraw = TRUE;
	else if(LastTime.wHour != (int)t.wHour 
		|| LastTime.wMinute != (int)t.wMinute) bRedraw = TRUE;
	
	
	if(g_bNoClock) bRedraw = FALSE;
	
	// date changed
	if(LastTime.wDay != t.wDay || LastTime.wMonth != t.wMonth ||
		LastTime.wYear != t.wYear)
	{
		InitFormatTime(); // formattime.c
		if(!(g_winver&WINNT))
			InitDaylightTimeTransition();
	}
	
	hdc = NULL;
	if(bRedraw) hdc = GetDC(hwnd);
	
	if(hdc)
	{
		OnPaint(hwnd, hdc, &t); // draw.c: draw the clock
		ReleaseDC(hwnd, hdc);
	}
	
	if(g_nBlink)
	{
		if(g_nBlink % 2) g_nBlink++; else g_nBlink--;
	}
	
	// check daylight/standard time transition
	if(!(g_winver&WINNT) && LastTime.wHour != t.wHour)
	{
		if(CheckDaylightTimeTransition(&t))
			PostMessage(hwnd, WM_USER+102, 0, 0);
	}
	
	memcpy(&LastTime, &t, sizeof(t));
	
#if TC_ENABLE_STARTBUTTON
	CheckCursorOnStartButton(); // startbtn.c
#endif
	
#if TC_ENABLE_STARTMENU
	CheckStartMenu(); // startmenu.c
#endif
	
	OnTimerTooltip(hwnd, FALSE); // tooltip.c
}

/*------------------------------------------------
  CLOCKM_REFRESHCLOCK message
--------------------------------------------------*/
void OnRefreshClock(HWND hwnd)
{
	LoadSetting(hwnd); // reload settings
	
	CreateClockDC(hwnd); // draw.c
	
#if TC_ENABLE_TRAYNOTIFY
	InitTrayNotify(hwnd); // traynotify.c
#endif
	
	// InitUserStr(); // userstr.c
	
#if TC_ENABLE_SYSINFO
	EndSysInfo(hwnd);  // sysinfo.c
	InitSysInfo(hwnd);  // sysinfo.c
#endif
	
	PostMessage(GetParent(GetParent(hwnd)), WM_SIZE,
		SIZE_RESTORED, 0);
	PostMessage(GetParent(hwnd), WM_SIZE,
		SIZE_RESTORED, 0);
	
	InvalidateRect(hwnd, NULL, FALSE);
	InvalidateRect(GetParent(hwnd), NULL, TRUE);
}

#if TC_ENABLE_TASKBAR
/*------------------------------------------------
  CLOCKM_REFRESHTASKBAR message
--------------------------------------------------*/
void OnRefreshTaskbar(HWND hwnd)
{
	g_bVisualStyle = IsXPVisualStyle();
	
#if TC_ENABLE_STARTBUTTON
	ResetStartButton(hwnd); // startbtn.c
#endif
	InitTaskbar(hwnd);      // taskbar.c
#if TC_ENABLE_TASKSWITCH
	InitTaskSwitch(hwnd);   // taskswitch.c
#endif
	
	RefreshTaskbar(hwnd); // taskbar.c
}
#endif	/* TC_ENABLE_TASKBAR */

#if TC_ENABLE_STARTMENU
/*------------------------------------------------
  CLOCKM_REFRESHSTARTMENU message
--------------------------------------------------*/
void OnRefreshStartMenu(HWND hwnd)
{
	ResetStartMenu(hwnd);
	
	if(!g_bIE4) InitTaskbar(hwnd);
}
#endif

/*------------------------------------------------
  CLOCKM_REFRESHTOOLTIP message
--------------------------------------------------*/
void OnRefreshTooltip(HWND hwnd)
{
	EndTooltip(hwnd);
	InitTooltip(hwnd);
}

#if TC_ENABLE_VOLUME
/*------------------------------------------------
  CLOCKM_VOLCHANGE message
--------------------------------------------------*/
void OnVolumeChange(HWND hwnd)
{
	RefreshVolume();
	
	PostMessage(GetParent(GetParent(hwnd)), WM_SIZE,
		SIZE_RESTORED, 0);
	PostMessage(GetParent(hwnd), WM_SIZE,
		SIZE_RESTORED, 0);
	
	InvalidateRect(hwnd, NULL, FALSE);
	InvalidateRect(GetParent(hwnd), NULL, TRUE);
	
	OnTimerTooltip(hwnd, TRUE);
}
#endif

/*------------------------------------------------
  WM_xxBUTTONDOWN message
--------------------------------------------------*/
LRESULT OnMouseDown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_LBUTTONDOWN)
		SetFocus(hwnd);
	
	if(g_sdisp2[0] || g_scat2[0])
	{
		g_sdisp2[0] = g_scat2[0] = 0;
		ClearClockDC();
		InvalidateRect(hwnd, NULL, FALSE);
	}
	
	if(g_nBlink)
	{
		g_nBlink = 0; InvalidateRect(hwnd, NULL, FALSE);
	}
	
#if TC_ENABLE_STARTBUTTON
	if(StartMenuFromClock(message, wParam, lParam))  // startbtn.c
		return 0;
#endif
	
	PostMessage(g_hwndTClockMain, message, wParam, lParam);
	return 0;
}

/*------------------------------------------------
  WM_xxBUTTONUP message
--------------------------------------------------*/
LRESULT OnMouseUp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PostMessage(g_hwndTClockMain, message, wParam, lParam);
	return 0;
}

/*------------------------------------------------
  WM_WINDOWPOSCHANGING message
--------------------------------------------------*/
void OnWindowPosChanging(HWND hwnd, LPWINDOWPOS pwp)
{
	DWORD dw;
	int h, w;
	
	if(g_bNoClock || g_bFitClock) return;
	
	if(!IsWindowVisible(hwnd) || (pwp->flags & SWP_NOSIZE))
		return;
	
	dw = OnCalcRect(hwnd);
	w = LOWORD(dw); h = HIWORD(dw);
	if(pwp->cx > w) pwp->cx = w;
	if(pwp->cy > h) pwp->cy = h;
}

/*------------------------------------------------
  WM_COPYDATA message
--------------------------------------------------*/
void OnCopyData(HWND hwnd, HWND hwndFrom, const COPYDATASTRUCT* pcds)
{
	const wchar_t *p = (const wchar_t *)pcds->lpData;
	wchar_t* dst;
	BOOL bRefresh = FALSE, bResize = FALSE;
	
	switch(pcds->dwData)
	{
		case COPYDATA_USTR0:
		case COPYDATA_USTR1:
		case COPYDATA_USTR2:
		case COPYDATA_USTR3:
		case COPYDATA_USTR4:
		case COPYDATA_USTR5:
		case COPYDATA_USTR6:
		case COPYDATA_USTR7:
		case COPYDATA_USTR8:
		case COPYDATA_USTR9:
		{
			if(wcslen(p) < BUFSIZE_USTR)
			{
				BOOL bEmptyLast;
				
				dst = g_userstr[pcds->dwData - COPYDATA_USTR0];
				bEmptyLast = !dst[0];
				wcscpy(dst, p);
				
				if(*p && bEmptyLast) bResize = TRUE;
				else if(*p == 0 && !bEmptyLast) bResize = TRUE;
				bRefresh = TRUE;
			}
			break;
		}
		case COPYDATA_DISP1:
		case COPYDATA_DISP2:
		case COPYDATA_CAT1:
		case COPYDATA_CAT2:
			if(wcslen(p) < BUFSIZE_DISP)
			{
				BOOL bEmptyLast;
				
				if(pcds->dwData == COPYDATA_DISP1) dst = g_sdisp1;
				else if(pcds->dwData == COPYDATA_DISP2) dst = g_sdisp2;
				else if(pcds->dwData == COPYDATA_CAT1) dst = g_scat1;
				else dst = g_scat2;
				
				bEmptyLast = !dst[0];
				wcscpy(dst, p);
				
				if(*p && bEmptyLast) bResize = TRUE;
				else if(*p == 0 && !bEmptyLast) bResize = TRUE;
				bRefresh = TRUE;
			}
			break;
		case COPYDATA_COPY:
			OnCopy(hwnd, p);
			break;
	}
	
	if(bResize) ClearClockDC();
	if(bRefresh && !g_bDispSecond) InvalidateRect(hwnd, NULL, FALSE);
}

/*------------------------------------------------
  copy date/time text to clipboard
--------------------------------------------------*/
void OnCopy(HWND hwnd, const wchar_t* pfmt)
{
	wchar_t format[BUFSIZE_FORMAT];
	wchar_t ws[BUFSIZE_FORMAT];
	HGLOBAL hg;
	
	if(pfmt)
	{
		if(wcslen(pfmt) < BUFSIZE_FORMAT - 5)
		{
			wcscpy(format, L"<%");
			wcscat(format, pfmt);
			wcscat(format, L"%>");
			pfmt = format;
		}
		else return;
	}
	
	MakeFormat(ws, NULL, pfmt, BUFSIZE_FORMAT);
	
	if(!OpenClipboard(hwnd)) return;
	EmptyClipboard();
	
	if(g_winver&WINNT)
	{
		wchar_t *p;
		
		hg = GlobalAlloc(GMEM_DDESHARE, (wcslen(ws) + 1) * sizeof(wchar_t));
		p = (wchar_t*)GlobalLock(hg);
		wcscpy(p, ws);
		GlobalUnlock(hg);
		SetClipboardData(CF_UNICODETEXT, hg);
	}
	else
	{
		char s[BUFSIZE_FORMAT], *p;
		
		WideCharToMultiByte(CP_ACP, 0, ws, -1, s, BUFSIZE_FORMAT-1,
			NULL, NULL);
		
		hg = GlobalAlloc(GMEM_DDESHARE, strlen(s) + 1);
		p = (char*)GlobalLock(hg);
		strcpy(p, s);
		GlobalUnlock(hg);
		SetClipboardData(CF_TEXT, hg);
	}
	
	CloseClipboard();
}

static int m_iHourTransition = -1, m_iMinuteTransition = -1;

/*------------------------------------------------
  initialize time-zone information
--------------------------------------------------*/
void InitDaylightTimeTransition(void)
{
	SYSTEMTIME lt, *plt;
	TIME_ZONE_INFORMATION tzi;
	DWORD dw;
	BOOL b;
	
	m_iHourTransition = m_iMinuteTransition = -1;
	
	GetLocalTime(&lt);
	
	b = FALSE;
	memset(&tzi, 0, sizeof(tzi));
	dw = GetTimeZoneInformation(&tzi);
	if(dw == TIME_ZONE_ID_STANDARD
	  && tzi.DaylightDate.wMonth == lt.wMonth
	  && tzi.DaylightDate.wDayOfWeek == lt.wDayOfWeek)
	{
		b = TRUE; plt = &(tzi.DaylightDate);
	}
	if(dw == TIME_ZONE_ID_DAYLIGHT
	  && tzi.StandardDate.wMonth == lt.wMonth
	  && tzi.StandardDate.wDayOfWeek == lt.wDayOfWeek)
	{
		b = TRUE; plt = &(tzi.StandardDate);
	}
	
	if(b && plt->wDay < 5)
	{
		if(((lt.wDay - 1) / 7 + 1) == plt->wDay)
		{
			m_iHourTransition = plt->wHour;
			m_iMinuteTransition = plt->wMinute;
		}
	}
	else if(b && plt->wDay == 5)
	{
		FILETIME ft;
		SystemTimeToFileTime(&lt, &ft);
		*(DWORDLONG*)&ft += 6048000000000i64;
		FileTimeToSystemTime(&ft, &lt);
		if(lt.wDay < 8)
		{
			m_iHourTransition = plt->wHour;
			m_iMinuteTransition = plt->wMinute;
		}
	}
}

/*------------------------------------------------
  check standard/daylight saving time transition
--------------------------------------------------*/
BOOL CheckDaylightTimeTransition(const SYSTEMTIME *plt)
{
	if((int)plt->wHour == m_iHourTransition &&
	   (int)plt->wMinute >= m_iMinuteTransition)
	{
		m_iHourTransition = m_iMinuteTransition = -1;
		return TRUE;
	}
	else return FALSE;
}

