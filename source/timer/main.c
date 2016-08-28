/*-------------------------------------------------------------
  main.c : TClock Timer
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tctimer.h"

/* Globals */

HINSTANCE g_hInst;                 // instance handle
char      g_mydir[MAX_PATH];       // path to this exe file
BOOL      g_bIniSetting = FALSE;   // save setting to ini file?
char      g_inifile[MAX_PATH];     // ini file name
char      g_langfile[MAX_PATH];    // tclang.txt
HFONT     g_hfontDialog = NULL;    // dialog font
HWND      g_hwndClock = NULL;      // clock window handle
HWND      g_hwndTimer = NULL;      // main window
HICON     g_hIconPlay, g_hIconStop;
                                   // icons to use frequently

/* Statics */

static int TCTimerMain(void);
static void InitTCTimer(void);
static LRESULT CALLBACK WndProcTimer(HWND, UINT, WPARAM, LPARAM);
static void OnCreate(HWND hwnd);
static void OnDestroy(HWND hwnd);
static void SetOnContextMenu(void);
static void CheckCommandLine(HWND hwnd);

/*-------------------------------------------
  WinMain
---------------------------------------------*/
#ifdef NODEFAULTLIB
void WinMainCRTStartup(void)
{
	g_hInst = GetModuleHandle(NULL);
	ExitProcess(TCTimerMain());
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	return TCTimerMain();
}
#endif

/*-------------------------------------------
  main routine
---------------------------------------------*/
int TCTimerMain(void)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hwnd;
	
	// not to execute the program twice
	hwnd = FindWindow(CLASS_TCLOCKTIMER, NULL);
	if(hwnd != NULL)
	{
		CheckCommandLine(hwnd);
		return 1;
	}
	
	InitTCTimer();
	
	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProcTimer;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = CLASS_TCLOCKTIMER;
	RegisterClass(&wndclass);
	
	// create a hidden window
	g_hwndTimer = CreateWindowEx(0,
		CLASS_TCLOCKTIMER, "TClock Timer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(g_hDlg && IsWindow(g_hDlg) && IsDialogMessage(g_hDlg, &msg)) ;
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	return (int)msg.wParam;
}

/*-------------------------------------------
  initialize
---------------------------------------------*/
void InitTCTimer(void)
{
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	strcpy(g_inifile, g_mydir);
	add_title(g_inifile, "tclock.ini");
	g_bIniSetting = TRUE;
	
	// common/langcode.c
	FindFileWithLangCode(g_langfile, GetUserDefaultLangID(), TCLANGTXT);
	g_hfontDialog = CreateDialogFont();
	
	g_hIconPlay = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR|LR_SHARED);
	g_hIconStop = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR|LR_SHARED);
	
	g_hwndClock = GetClockWindow();
	
	// setting of [OnContextMenu] in tclock.ini
	SetOnContextMenu();
}

/*-------------------------------------------
  window procedure
---------------------------------------------*/
LRESULT CALLBACK WndProcTimer(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
    switch (message)
	{
		case WM_CREATE:
			OnCreate(hwnd);
			return 0;
		case WM_DESTROY:
			OnDestroy(hwnd);
			return 0;
		case WM_TIMER:
			switch (wParam)
			{
				case IDTIMER_TIMER:
					OnTimerTimer(hwnd);
					break;
			}
			return 0;
		// show dialog box
		case TIMERM_SHOWDLG:
			OnShowDialog(hwnd);
			return 0;
		// add item to tcmenu*.txt
		case TIMERM_REQUESTMENU:
			OnRequestMenu(hwnd, FALSE);
			return 0;
		// stop running timer
		case TIMERM_STOP:
			OnStopTimer(hwnd, (int)lParam);
			return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*-------------------------------------------------------
  WM_CREATE message
---------------------------------------------------------*/
void OnCreate(HWND hwnd)
{
	PostMessage(hwnd, TIMERM_SHOWDLG, 0, 0);
	SetTimer(hwnd, IDTIMER_TIMER, 1000, NULL);
}

/*-------------------------------------------------------
  WM_DESTROY message
---------------------------------------------------------*/
void OnDestroy(HWND hwnd)
{
	ClearTimer();
	
	KillTimer(hwnd, IDTIMER_TIMER);
	
	if(g_hfontDialog) DeleteObject(g_hfontDialog);
	
	PostQuitMessage(0);
}

/*-------------------------------------------
   process command line option
---------------------------------------------*/
void CheckCommandLine(HWND hwnd)
{
	char name[20], value[20];
	char *p;
	int i;
	BOOL bStop = FALSE;
	
	p = GetCommandLine();
	
	while(*p)
	{
		if(*p == '/')
		{
			p++;
			for(i = 0; *p && *p != ' ' && i < 19; i++)
			{
				name[i] = *p++;
			}
			name[i] = 0;
			while(*p == ' ') p++;
			
			value[0] = 0;
			if(*p && *p != '/')
			{
				for(i = 0; *p && i < 19; i++)
					value[i] = *p++;
				value[i] = 0;
			}
			
			if(strcmp(name, "stop") == 0)
			{
				PostMessage(hwnd, TIMERM_STOP, 0, atoi(value));
				bStop = TRUE;
			}
		}
		else p++;
	}
	
	if(!bStop)
		PostMessage(hwnd, TIMERM_SHOWDLG, 0, 0);
}

/*------------------------------------------------
  [OnContextMenu]
  AppN=TClockTimerClass,TIMERM_REQUESTMENU
-------------------------------------------------*/
void SetOnContextMenu(void)
{
	char entry[20], buf[100], cname[80], num[20];
	BOOL b;
	int i;
	
	for(i = 0; ; i++)
	{
		wsprintf(entry, "App%d", i + 1);
		GetMyRegStr("OnContextMenu", entry, buf, 100, "");
		b = FALSE;
		if(buf[0])
		{
			parse(cname, buf, 0, 80);
			parse(num, buf, 1, 20);
			if(strcmp(cname, CLASS_TCLOCKTIMER) == 0)
			{
				if(TIMERM_REQUESTMENU == atoi(num)) break;
				else b = TRUE;
			}
		}
		else b = TRUE;
		
		if(b)
		{
			wsprintf(buf, "%s,%d", CLASS_TCLOCKTIMER, TIMERM_REQUESTMENU);
			SetMyRegStr("OnContextMenu", entry, buf);
			break;
		}
	}
}

/* -------------------- Utilities ---------------------------------------*/

/*-------------------------------------------
  called in PlayFile function
---------------------------------------------*/
BOOL ExecCommandString(HWND hwnd, const char *command)
{
	SendStringToOther(GetTClockMainWindow(), hwnd, command,
		COPYDATA_EXEC);
	
	return FALSE;
}

