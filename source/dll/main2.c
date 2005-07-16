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
HMODULE g_hInst;               // instanse handle
WNDPROC g_oldWndProc;          // clock's window procedure
BOOL    g_bIniSetting;         // use tclock.ini
char    g_inifile[MAX_PATH];   // ini file name
char    g_mydir[MAX_PATH];     // path of tcdll.dll
int     g_winver;              // Windows version
BOOL    g_bIE4;                // IE 4 or later
BOOL    g_bVisualStyle;        // Windows XP theme is used
BOOL    g_bNoClock;            // don't customize clock


/*------------------------------------------------
  initialize the clock
--------------------------------------------------*/
void InitClock(HWND hwnd)
{
	if(g_bInitClock) return;
	g_bInitClock = TRUE;
	
	g_hInst = GetModuleHandle(DLLFILENAME);
	
	g_winver = CheckWinVersion();       // common/util.c
	g_bIE4 = IsIE4();                   // common/util.c
	g_bVisualStyle = IsXPVisualStyle(); // common/util.c
	
	// check subclassification
	if(IsSubclassed(hwnd))
	{
		SendMessage(g_hwndTClockMain, TCM_CLOCKERROR, 0, 6);
		return;
	}
	
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	
	strcpy(g_inifile, g_mydir);
	add_title(g_inifile, "tclock.ini");
	g_bIniSetting = TRUE;
/*  g_bIniSetting = FALSE;
	if(IsFile(g_inifile)) g_bIniSetting = TRUE; */
	
	// tell tclock.exe clock's HWND
	PostMessage(g_hwndTClockMain, TCM_HWNDCLOCK, 0, (LPARAM)hwnd);
	
	// read settings
	LoadSetting(hwnd);
	
	InitTooltip(hwnd); // tooltip.c
	InitUserStr();     // userstr.c
	
	// subclassfy the clock window !!
	g_oldWndProc = (WNDPROC)SetWindowLongPtr(hwnd,
		GWLP_WNDPROC, (LONG_PTR)WndProc);
	
	// don't accept double clicks
	SetClassLong(hwnd, GCL_STYLE,
		GetClassLong(hwnd, GCL_STYLE) & ~CS_DBLCLKS);
	
	InitStartButton(hwnd);  // startbtn.c
	InitStartMenu(hwnd);   // startmenu.c
	InitTaskbar(hwnd);      // taskbar.c
	InitTaskSwitch(hwnd);  // taskswitch.c
	InitTrayNotify(hwnd);  // traynotify.c
	InitSysInfo(hwnd);  // sysinfo.c
	
	RefreshTaskbar(hwnd);  // taskbar.c
	
	SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);
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
	
	DragAcceptFiles(hwnd, FALSE);
	
	EndTrayNotify();    // traynotify.c
	EndTaskSwitch();    // taskswitch.c
	EndTaskbar(hwnd);   // taskbar.c
	EndStartMenu();     // startmenu.c
	EndStartButton();   // startbtn.c
	ClearDrawing();     // drawing.c
	EndTooltip(hwnd);   // tooltip.c
	
	EndNewAPI();      // newapi.c
	EndSysInfo(hwnd); // sysinfo.c
	
	// Stop timers
	KillTimer(hwnd, IDTIMER_MAIN);
	
	// restore window procedure
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)g_oldWndProc);
	g_oldWndProc = NULL;
	
	RefreshTaskbar(hwnd);  // taskbar.c
	
	// close window of tclock.exe
	PostMessage(g_hwndTClockMain, WM_CLOSE, 0, 0);
	
	PostMessage(hwnd, WM_TIMER, 0, 0);
}

/*-------------------------------------------------------------
  WM_DESTROY message
---------------------------------------------------------------*/
void OnDestroy(HWND hwnd)
{
	ClearStartMenuResource();   // startmenu.c
	ClearStartButtonResource(); // startbtn.c
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
	
	DragAcceptFiles(hwnd, GetMyRegLong(NULL, "DropFiles", FALSE));
	
	LoadFormatSetting(hwnd);   // format.c
	LoadDrawingSetting(hwnd);  // drawing.c
}

