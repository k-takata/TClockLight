/*-------------------------------------------------------------
  main.c : Time Synchronization
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcsntp.h"

/* Globals */

BOOL ExecCommandString(HWND hwnd, const char *command);

HINSTANCE g_hInst;                 // instance handle
char      g_mydir[MAX_PATH];       // path to this exe file
BOOL      g_bIniSetting = FALSE;   // save setting to ini file?
char      g_inifile[MAX_PATH];     // ini file name
char      g_langfile[MAX_PATH];    // tclang.txt
HFONT     g_hfontDialog = NULL;    // dialog font
HWND      g_hwndMain  = NULL;      // main window
HICON     g_hIconPlay, g_hIconStop; // icons to use frequently
int       g_winver;                // Windows version flags

/* Statics */

static int TCSNTPMain(void);
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static void InitApp(void);
static void OnCreate(HWND hwnd);
static void OnDestroy(HWND hwnd);
static BOOL CheckCommandLine(HWND hwnd, HWND hwndDlg);

BOOL m_bSilent = FALSE, m_bOnlyRas = FALSE;

/*-------------------------------------------
  WinMain
---------------------------------------------*/
#ifdef NODEFAULTLIB
void /*WINAPI*/ WinMainCRTStartup(void)
{
	g_hInst = GetModuleHandle(NULL);
	ExitProcess(TCSNTPMain());
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	return TCSNTPMain();
}
#endif

/*-------------------------------------------
  main routine
---------------------------------------------*/
int TCSNTPMain(void)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hwnd, hwndDlg = NULL;
	
	// not to execute the program twice
	hwnd = FindWindowEx(NULL, NULL, CLASS_TCLOCKSNTP, NULL);
	if(hwnd != NULL)
	{
		DWORD dwTID1, dwTID2;
		dwTID1 = GetWindowThreadProcessId(hwnd, NULL);
		do {
			hwndDlg = FindWindowEx(NULL, hwndDlg, WC_DIALOG, NULL);
			if(hwndDlg == NULL)
				break;
			dwTID2 = GetWindowThreadProcessId(hwndDlg, NULL);
		} while(dwTID1 != dwTID2);
	}
	if(CheckCommandLine(hwnd, hwndDlg))
		return 1;
	
	InitApp();
	timeBeginPeriod(1);
	
	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = CLASS_TCLOCKSNTP;
	RegisterClass(&wndclass);
	
	// create a hidden window
	g_hwndMain = CreateWindowEx(0,
		CLASS_TCLOCKSNTP, "TClock SNTP", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	// ShowWindow(g_hwndMain, SW_SHOW);
	// UpdateWindow(g_hwndMain);
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!(g_hDlg && IsWindow(g_hDlg) && IsDialogMessage(g_hDlg, &msg)))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	timeEndPeriod(1);
	return (int)msg.wParam;
}

/*-------------------------------------------
  initialize
---------------------------------------------*/
void InitApp(void)
{
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	strcpy(g_inifile, g_mydir);
	add_title(g_inifile, "tclock.ini");
	g_bIniSetting = TRUE;
	
	// common/langcode.c
	FindFileWithLangCode(g_langfile, GetUserDefaultLangID(), TCLANGTXT);
	g_hfontDialog = CreateDialogFont();
	
	g_winver = CheckWinVersion();
	
	g_hIconPlay = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR|LR_SHARED);
	g_hIconStop = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR|LR_SHARED);
}

/*-------------------------------------------
  window procedure
---------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message,
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
				case IDTIMER_MAIN:
					OnTimerSNTP(hwnd);
					break;
			}
			return 0;
		// show dialog box
		case SNTPM_SHOWDLG:
			OnShowDialog(hwnd);
			return 0;
		
		case WSOCK_GETHOST: // get IP address
			OnGetHost(hwnd, wParam, lParam);
			return 0;
		case WSOCK_SELECT:  // receive SNTP data
			OnReceive(hwnd, wParam, lParam);
			return 0;
		case SNTPM_SUCCESS:
		{
			HWND hwndClock = GetClockWindow();
			if(hwndClock) InvalidateRect(hwndClock, NULL, FALSE);
		}
		case SNTPM_ERROR:
			if(!g_hDlg)
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		
		case SNTPM_LOADLOG:
			if(g_hDlg)
				PostMessage(g_hDlg, SNTPM_LOADLOG, 0, 0);
			return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*-------------------------------------------------------
  WM_CREATE message
---------------------------------------------------------*/
void OnCreate(HWND hwnd)
{
	if(InitSNTP(hwnd) == FALSE) // sntp.c
	{
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return;
	}
	
	if(m_bSilent)
	{
		if(StartSyncTime(hwnd, m_bOnlyRas) == FALSE)
			PostMessage(hwnd, WM_CLOSE, 0, 0);
	}
	else
		PostMessage(hwnd, SNTPM_SHOWDLG, 0, 0);
}

/*-------------------------------------------------------
  WM_DESTROY message
---------------------------------------------------------*/
void OnDestroy(HWND hwnd)
{
	EndSNTP(hwnd); // sntp.c
	
	if(g_hfontDialog) DeleteObject(g_hfontDialog);
	
	PostQuitMessage(0);
}

/*-------------------------------------------
   process command line option
---------------------------------------------*/
BOOL CheckCommandLine(HWND hwnd, HWND hwndDlg)
{
	char name[20], value[20];
	char *p;
	int i;
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
			
			if(strcmp(name, "silent") == 0)
			{
				m_bSilent = TRUE;
			}
			else if(strcmp(name, "ras") == 0)
			{
				m_bOnlyRas = TRUE;
			}
		}
		else p++;
	}
	
	if(m_bSilent)
	{
		if((hwnd != NULL) && (hwndDlg == NULL))
			return TRUE;
		else
			return FALSE;
	}
	else if(hwnd != NULL)
	{
		PostMessage(hwnd, SNTPM_SHOWDLG, 0, 0);
		return TRUE;
	}
	return FALSE;
}

/* -------------------- Utilities ---------------------------------------*/

/*-------------------------------------------
  called in PlayFile function
---------------------------------------------*/
BOOL ExecCommandString(HWND hwnd, const char *command)
{
	SendStringToOther(GetTClockMainWindow(), hwnd, command, COPYDATA_EXEC);
	
	return FALSE;
}

