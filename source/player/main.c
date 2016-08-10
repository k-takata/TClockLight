/*-------------------------------------------------------------
  main.c : TClock Player
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcplayer.h"

/* Globals */

BOOL ExecCommandString(HWND hwnd, const char *command);

HINSTANCE g_hInst;                 // instance handle
char      g_mydir[MAX_PATH];       // path to this exe file
BOOL      g_bIniSetting = FALSE;   // save setting to ini file?
char      g_inifile[MAX_PATH];     // ini file name
char      g_langfile[MAX_PATH];    // tclang.txt
HFONT     g_hfontDialog = NULL;    // dialog font
HWND      g_hwndClock = NULL;      // clock window handle
HWND      g_hwndPlayer = NULL;     // main window

/* Statics */

static int TCPlayerMain(void);
static void InitTCPlayer(void);
static LRESULT CALLBACK WndProcPlayer(HWND, UINT, WPARAM, LPARAM);
static void OnCreate(HWND hwnd);
static void OnDestroy(HWND hwnd);
static void OnCopyData(HWND hwnd, HWND hwndFrom, COPYDATASTRUCT* pcds);
static void SetOnContextMenu(void);
static void CheckCommandLine(HWND hwnd, BOOL bPrev);

/*-------------------------------------------
  WinMain
---------------------------------------------*/
#ifdef NODEFAULTLIB
void WINAPI WinMainCRTStartup(void)
{
	g_hInst = GetModuleHandle(NULL);
	ExitProcess(TCPlayerMain());
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	return TCPlayerMain();
}
#endif

/*-------------------------------------------
  main routine
---------------------------------------------*/
int TCPlayerMain(void)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hwnd;
	
	// not to execute the program twice
	hwnd = FindWindow(CLASS_TCLOCKPLAYER, NULL);
	if(hwnd != NULL)
	{
		CheckCommandLine(hwnd, TRUE);
		return 1;
	}
	
	InitTCPlayer();
	
	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProcPlayer;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = CLASS_TCLOCKPLAYER;
	RegisterClass(&wndclass);
	
	// create a hidden window
	g_hwndPlayer = CreateWindowEx(0,
		CLASS_TCLOCKPLAYER, "TClock Player", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	// ShowWindow(g_hwndPlayer, SW_SHOW);
	// UpdateWindow(g_hwndPlayer);
	
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
void InitTCPlayer(void)
{
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	strcpy(g_inifile, g_mydir);
	add_title(g_inifile, "tclock.ini");
	g_bIniSetting = TRUE;
	
	// common/langcode.c
	FindFileWithLangCode(g_langfile, GetUserDefaultLangID(), TCLANGTXT);
	g_hfontDialog = CreateDialogFont();
	
	g_hwndClock = GetClockWindow();
	
	SetOnContextMenu();
}

/*-------------------------------------------
  window procedure
---------------------------------------------*/
LRESULT CALLBACK WndProcPlayer(HWND hwnd, UINT message,
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
				case IDTIMER_PLAYER:
					OnTimerPlayer(hwnd);
					break;
			}
			return 0;
		// show dialog box
		case PLAYERM_SHOWDLG:
			OnShowDialog(hwnd);
			return 0;
		// add item to tcmenu*.txt
		case PLAYERM_REQUESTMENU:
			OnRequestMenu(hwnd, FALSE);
			return 0;
		case PLAYERM_STOP:
			StopPlayer(hwnd);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		case PLAYERM_PAUSE:
			PausePlayer(hwnd);
			return 0;
		case PLAYERM_NEXT:
			PrevNextPlayer(hwnd, TRUE);
			return 0;
		case PLAYERM_PREV:
			PrevNextPlayer(hwnd, FALSE);
			return 0;
		
		case MM_MCINOTIFY:
			OnMCINotifyPlayer(hwnd, wParam, (LONG)lParam);
			return 0;
		
		case WM_COPYDATA:
			OnCopyData(hwnd, (HWND)wParam, (COPYDATASTRUCT*)lParam);
			return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*-------------------------------------------------------
  WM_CREATE message
---------------------------------------------------------*/
void OnCreate(HWND hwnd)
{
	CreateWindowEx(WS_EX_CLIENTEDGE, "listbox", "",
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_SORT,
		0, 0, 400, 80,
		hwnd, (HMENU)IDC_LIST, g_hInst, NULL);
	
	InitPlayer(hwnd); // player.c
	
	CheckCommandLine(hwnd, FALSE);
	
	SetTimer(hwnd, IDTIMER_PLAYER, 1000, NULL);
}

/*-------------------------------------------------------
  WM_DESTROY message
---------------------------------------------------------*/
void OnDestroy(HWND hwnd)
{
	StopPlayer(hwnd);
	
	KillTimer(hwnd, IDTIMER_PLAYER);
	
	if(g_hfontDialog) DeleteObject(g_hfontDialog);
	
	PostQuitMessage(0);
}

/*-------------------------------------------------------
  WM_COPYDATA message
---------------------------------------------------------*/
void OnCopyData(HWND hwnd, HWND hwndFrom, COPYDATASTRUCT* pcds)
{
	const char *p = (char *)pcds->lpData;
	
	switch(pcds->dwData)
	{
		case COPYDATA_PLAY:
			if(Player(hwnd, p) == FALSE)  // player.c
			{
				if(!g_hDlg)
					PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;
	}
}

/*------------------------------------------------
  [OnContextMenu]
  AppN=TClockPlayerClass,PLAYERM_REQUESTMENU
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
			if(strcmp(cname, CLASS_TCLOCKPLAYER) == 0)
			{
				if(PLAYERM_REQUESTMENU == atoi(num)) break;
				else b = TRUE;
			}
		}
		else b = TRUE;
		
		if(b)
		{
			wsprintf(buf, "%s,%d", CLASS_TCLOCKPLAYER, PLAYERM_REQUESTMENU);
			SetMyRegStr("OnContextMenu", entry, buf);
			break;
		}
	}
}

/*-------------------------------------------
   process command line option
---------------------------------------------*/
void CheckCommandLine(HWND hwnd, BOOL bPrev)
{
	char name[20], value[20];
	char *p, *sp;
	int i;
	BOOL bOption = FALSE;
	BOOL bquot = FALSE;
	
	p = GetCommandLine();
	
	while(*p == ' ') p++;
	if(*p == '\"') { bquot = TRUE; p++; }
	while(*p)
	{
		if(bquot) { if(*p == '\"') { p++; break; } }
		else if(*p == ' ') break;
		p++;
	}
	while(*p == ' ') p++;
	sp = p;
	
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
				if(bPrev) PostMessage(hwnd, PLAYERM_STOP, 0, 0);
				bOption = TRUE;
			}
			else if(strcmp(name, "pause") == 0)
			{
				if(bPrev) PostMessage(hwnd, PLAYERM_PAUSE, 0, 0);
				bOption = TRUE;
			}
			else if(strcmp(name, "next") == 0)
			{
				if(bPrev) PostMessage(hwnd, PLAYERM_NEXT, 0, 0);
				bOption = TRUE;
			}
			else if(strcmp(name, "prev") == 0)
			{
				if(bPrev) PostMessage(hwnd, PLAYERM_PREV, 0, 0);
				bOption = TRUE;
			}
		}
		else p++;
	}
	
	if(!bOption)
	{
		p = sp;
		
		if(*p)
		{
			if(bPrev)
				SendStringToOther(hwnd, NULL, p, COPYDATA_PLAY);
			else
			{
				if(Player(hwnd, p) == FALSE)  // player.c
					PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
		}
		else
			PostMessage(hwnd, PLAYERM_SHOWDLG, 0, 0);
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

