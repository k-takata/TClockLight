/*-------------------------------------------------------------
  main2.c : initialize and clear up
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */

void InitClock(HWND hwnd);
void EndClock(HWND hwnd);
void OnDestroy(HWND hwnd);
void LoadSetting(HWND hwnd);

BOOL    g_bInitClock = FALSE;  // InitTClock() has been called
HANDLE  g_hInst;               // instanse handle
BOOL    g_bIniSetting;         // use tclock.ini
char    g_inifile[MAX_PATH];   // ini file name
char    g_mydir[MAX_PATH];     // path of tcdll.dll
int     g_winver;              // Windows version
BOOL    g_bVisualStyle;        // Windows XP theme is used
BOOL    g_bNoClock;            // don't customize clock
int     g_OrigClockWidth;      // original clock width
int     g_OrigClockHeight;     // original clock height
BOOL    g_bLMousePassThru;     // pass through left button messages

#define SUBCLASS_ID			1
#define SUBCLASSTRAY_ID		2


/*------------------------------------------------
  initialize the clock
--------------------------------------------------*/
void InitClock(HWND hwnd)
{
	RECT rc;
	
	if(g_bInitClock) return;
	g_bInitClock = TRUE;
	
	g_hInst = GetModuleHandle(DLLFILENAME);
	
	g_winver = CheckWinVersion();       // common/util.c
	g_bVisualStyle = IsXPVisualStyle(); // common/util.c
	
#if 0
	if(!(g_winver&WIN10RS1))
	{
		// check subclassification
		if(IsSubclassed(hwnd))
		{
			SendMessage(g_hwndTClockMain, TCM_CLOCKERROR, 0, 6);
			return;
		}
	}
#endif
	
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	
	strcpy(g_inifile, g_mydir);
	add_title(g_inifile, "tclock.ini");
	g_bIniSetting = TRUE;
/*  g_bIniSetting = FALSE;
	if(IsFile(g_inifile)) g_bIniSetting = TRUE; */
	
	// Save the original window size
	GetWindowRect(hwnd, &rc);
	g_OrigClockWidth = rc.right - rc.left;
	g_OrigClockHeight = rc.bottom - rc.top;
	
	// tell tclock.exe clock's HWND
	PostMessage(g_hwndTClockMain, TCM_HWNDCLOCK, 0, (LPARAM)hwnd);
	
	// read settings
	LoadSetting(hwnd);
	
	InitTooltip(hwnd); // tooltip.c
	InitUserStr();     // userstr.c
	
	// subclassfy the clock window !!
	SetWindowSubclass(GetParent(hwnd), SubclassTrayProc, SUBCLASSTRAY_ID,
			(DWORD_PTR)hwnd);
	SetWindowSubclass(hwnd, SubclassProc, SUBCLASS_ID, 0);
	
	// don't accept double clicks
	SetClassLong(hwnd, GCL_STYLE,
		GetClassLong(hwnd, GCL_STYLE) & ~CS_DBLCLKS);
	
#if TC_ENABLE_STARTBUTTON
	InitStartButton(hwnd); // startbtn.c
#endif
#if TC_ENABLE_STARTMENU
	InitStartMenu(hwnd);   // startmenu.c
#endif
#if TC_ENABLE_TASKBAR
	InitTaskbar(hwnd);     // taskbar.c
#endif
#if TC_ENABLE_TASKSWITCH
	InitTaskSwitch(hwnd);  // taskswitch.c
#endif
#if TC_ENABLE_TRAYNOTIFY
	InitTrayNotify(hwnd);  // traynotify.c
#endif
#if TC_ENABLE_VOLUME
	InitMixer();           // mixer.c
#endif
#if TC_ENABLE_SYSINFO
	InitSysInfo(hwnd);     // sysinfo.c
#endif
	
#if TC_ENABLE_TASKBAR
	RefreshTaskbar(hwnd);  // taskbar.c
#else
	PostMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED, 0);
	InvalidateRect(GetParent(GetParent(hwnd)), NULL, TRUE);
#endif
	
	SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);
	
#if TC_ENABLE_DESKTOPICON
	SetDesktopIcons();     // desktop.c
#endif
}

/*------------------------------------------------
  ending process
--------------------------------------------------*/
void EndClock(HWND hwnd)
{
	static BOOL bEndClock = FALSE;
	
	if(bEndClock) return; // avoid to be called twice
	bEndClock = TRUE;
	
	g_bVisualStyle = IsXPVisualStyle();
	
#if TC_ENABLE_MOUSEDROP
	DragAcceptFiles(hwnd, FALSE);
#endif
	
#if TC_ENABLE_DESKTOPICON
	EndDesktopIcons();	// desktop.c
#endif
#if TC_ENABLE_SYSINFO
	EndSysInfo(hwnd);	// sysinfo.c
#endif
#if TC_ENABLE_VOLUME
	ReleaseMixer();		// mixer.c
#endif
#if TC_ENABLE_TRAYNOTIFY
	EndTrayNotify();    // traynotify.c
#endif
#if TC_ENABLE_TASKSWITCH
	EndTaskSwitch();    // taskswitch.c
#endif
#if TC_ENABLE_TASKBAR
	EndTaskbar(hwnd);   // taskbar.c
#endif
#if TC_ENABLE_STARTMENU
	EndStartMenu();     // startmenu.c
#endif
#if TC_ENABLE_STARTBUTTON
	EndStartButton();   // startbtn.c
#endif
	ClearDrawing();     // drawing.c
	EndTooltip(hwnd);   // tooltip.c
	
	EndNewAPI();		// newapi.c
	
	// Stop timers
	KillTimer(hwnd, IDTIMER_MAIN);
	
	// restore window procedure
	RemoveWindowSubclass(hwnd, SubclassProc, SUBCLASS_ID);
	RemoveWindowSubclass(GetParent(hwnd), SubclassTrayProc, SUBCLASSTRAY_ID);
	
#if TC_ENABLE_TASKBAR
	RefreshTaskbar(hwnd);  // taskbar.c
#else
	PostMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED, 0);
	InvalidateRect(GetParent(GetParent(hwnd)), NULL, TRUE);
#endif
	
	// close window of tclock.exe
	PostMessage(g_hwndTClockMain, WM_CLOSE, 0, 0);
	
	PostMessage(hwnd, WM_TIMER, 0, 0);
}

/*-------------------------------------------------------------
  WM_DESTROY message
---------------------------------------------------------------*/
void OnDestroy(HWND hwnd)
{
#if TC_ENABLE_STARTMENU
	ClearStartMenuResource();   // startmenu.c
#endif
#if TC_ENABLE_STARTBUTTON
	ClearStartButtonResource(); // startbtn.c
#endif
	ClearDrawing();             // drawing.c
}

/*-------------------------------------------------------------
   read settings and initialize
   this function is called in InitClock() and OnRefreshClock()
   ReadData() in old version
---------------------------------------------------------------*/
void LoadSetting(HWND hwnd)
{
	g_bNoClock = GetMyRegLong(NULL, "NoClock", FALSE);
	g_bLMousePassThru = GetMyRegLong("Mouse", "LeftMousePassThrough",
			(g_winver&WIN10RS1) != 0);
	
#if TC_ENABLE_MOUSEDROP
	DragAcceptFiles(hwnd, GetMyRegLong(NULL, "DropFiles", FALSE));
#endif
	
	LoadFormatSetting(hwnd);   // format.c
	LoadDrawingSetting(hwnd);  // drawing.c
}

