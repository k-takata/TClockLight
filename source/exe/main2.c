/*-------------------------------------------------------------
  main2.c : TClockExeMain, and initializing functions
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

/* Globals */

int TClockExeMain(void);

char g_mydir[MAX_PATH];     // path to tclock.exe
BOOL g_bIniSetting = FALSE; // save setting to ini file?
char g_inifile[MAX_PATH];   // ini file name
int  g_winver;              // windows version
UINT g_uTaskbarRestart;     // taskbar recreating message

/* Statics */

static void InitTClockMain(void);
static BOOL CheckTCDLL(void);
static void InitTextColor(void);
static void InitFormat(void);
static void AddMessageFilters(void);


/*-------------------------------------------
   main routine
---------------------------------------------*/
int TClockExeMain(void)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hwnd, hwndParent;
	
	// not to execute the program twice
	hwnd = GetTClockMainWindow();
	if(hwnd != NULL)
	{
		CheckCommandLine(hwnd, TRUE);
		return 1;
	}
	
	ImmDisableIME(0);
	
	if(!CheckTCDLL()) { return 1; }
	
	if(FindWindow("ObjectBar Toolbar", NULL))
	{
		MessageBox(NULL, "ObjectBar is running",
			"Error", MB_OK|MB_ICONEXCLAMATION);
		return 1;
	}
	
	InitTClockMain();
	
	// Windows Vista UIPI filter
	AddMessageFilters();
	
	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
//	wndclass.hIcon         = LoadIcon(g_hInst, "MYICON");
	wndclass.hIcon         = NULL;
//	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = CLASS_TCLOCKMAIN;
	RegisterClass(&wndclass);
	
#if 0
	// use message-only window to reduse memory consumption
	if(g_winver&WIN2000)
		hwndParent = HWND_MESSAGE;	// message-only window
	else
		hwndParent = NULL;
#else
	// message-only window can't receive WM_POWERBROADCAST
	hwndParent = NULL;
#endif
	
	// create a hidden window
	hwnd = CreateWindowEx(0,
		CLASS_TCLOCKMAIN, TITLE_TCLOCKMAIN,
		/*WS_OVERLAPPEDWINDOW*/ WS_POPUP|WS_DISABLED,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		hwndParent, NULL, g_hInst, NULL);
	ShowWindow(hwnd, SW_MINIMIZE);
	ShowWindow(hwnd, SW_HIDE);
	// ShowWindow(hwnd, SW_SHOW);
	// UpdateWindow(hwnd);
	
	CheckCommandLine(hwnd, FALSE);
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(g_hDlgAbout && IsDialogMessage(g_hDlgAbout, &msg)) ;
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	UnregisterClass(CLASS_TCLOCKMAIN, g_hInst);
	
	return (int)msg.wParam;
}

/*-------------------------------------------
  initialize variables, format, etc.
---------------------------------------------*/
void InitTClockMain(void)
{
	// get the path where .exe is positioned
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	
	g_bIniSetting = FALSE;
	SetMyRegStr(NULL, "ExePath", g_mydir);
	
	// check if 'lang' directory exists
	if(!DoesLangDirExist())
	{
		MessageBox(NULL, "language file not found", "TClock Light",
				MB_ICONWARNING);
	}
	
	// ini file name
	strcpy(g_inifile, g_mydir);
	add_title(g_inifile, "tclock.ini");
	g_bIniSetting = TRUE;
/*  g_bIniSetting = IsFile(g_inifile); */
	
	g_winver = CheckWinVersion();
	
	// Message of the taskbar recreating
	// Special thanks to Mr.Inuya
	g_uTaskbarRestart = RegisterWindowMessage("TaskbarCreated");
	
	DelMyRegKey("OnContextMenu"); // temporarily
	
	InitTextColor();
	
	InitFormat();
}

/*-------------------------------------------
  Check version of dll
---------------------------------------------*/
BOOL CheckTCDLL(void)
{
	char str[80];
	GetTClockVersion(str);
	if(strcmp(str, TCLOCKVERSION) != 0)
	{
		MessageBox(NULL, "Invalid file version: tcdll.tclock",
			"Error", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	return TRUE;
}

/*------------------------------------------------
  for Luna theme of Windows XP
--------------------------------------------------*/
void InitTextColor(void)
{
	int len;
	char s[80];
	char themekey[] =
		"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager";
	
	if(GetMyRegStr(NULL, "ForeColor", s, 20, "") > 0) return;
	
	len = GetRegStr(HKEY_CURRENT_USER, themekey,
		"DllName", s, 80, "");
	while((len > 0) && (s[len] != '\\'))
		len--;
	if(lstrcmpi(s + len, "\\luna.msstyles") == 0
		|| lstrcmpi(s + len, "\\Aero.msstyles") == 0)
	{
		GetRegStr(HKEY_CURRENT_USER, themekey,
			"ColorName", s, 80, "");
		if(strcmp(s, "NormalColor") == 0)
			SetMyRegLong(NULL, "ForeColor", RGB(255,255,255));
	}
}

/*------------------------------------------------
  initialize format string
--------------------------------------------------*/
void InitFormat(void)
{
	char s[BUFSIZE_FORMAT];
	BOOL parts[NUM_FORMATPART];
	RECT rc;
	HWND hwnd;
	BOOL bbreak;
	int ilang;
	int i;
	
	if(GetMyRegStr("", "Format", s, BUFSIZE_FORMAT, "") > 0)
		return;
	
	ilang = GetMyRegLong("", "Locale", (int)GetUserDefaultLangID());
	if(GetMyRegStr("", "Locale", s, 20, "") == 0)
		SetMyRegLong("", "Locale", ilang);
	
	InitAutoFormat(ilang); // common/autoformat.c
	
	for(i = 0; i < NUM_FORMATPART; i++)
		parts[i] = FALSE;
	
	parts[PART_YEAR] = TRUE;
	parts[PART_MONTH] = TRUE;
	parts[PART_DAY] = TRUE;
	parts[PART_WEEKDAY] = TRUE;
	parts[PART_HOUR] = TRUE;
	parts[PART_MINUTE] = TRUE;
	parts[PART_SECOND] = TRUE;
	
	bbreak = FALSE;
	hwnd = GetTaskbarWindow();
	if(hwnd != NULL)
	{
		GetClientRect(hwnd, &rc);
		// vertical task bar
		if(rc.right < rc.bottom) bbreak = TRUE;
		
		hwnd = FindWindowEx(hwnd, NULL, "TrayNotifyWnd", NULL);
		if(hwnd)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			if(rc.bottom - rc.top > 32) bbreak = TRUE;
		}
	}
	
	parts[PART_BREAK] = bbreak;
	
	AutoFormat(s, parts);  // common/autoformat.c
	
	if(!SetMyRegStr("", "Format", s))
	{
		MessageBox(NULL, "Can't save the settings",
			"Error", MB_OK|MB_ICONEXCLAMATION);
		ExitProcess(1);
	}
	
	SetMyRegLong("", "Kaigyo", parts[PART_BREAK]);
}

/*------------------------------------------------
  add the messages to the UIPI message filter
--------------------------------------------------*/
void AddMessageFilters(void)
{
	int i;
	const UINT messages[] = {
	//	WM_CREATE,
		WM_CLOSE,
		WM_DESTROY,
	//	WM_ENDSESSION,
	//	WM_POWERBROADCAST,
	//	WM_TIMER,
		WM_COMMAND,
		WM_CONTEXTMENU,
		WM_EXITMENULOOP,
		
		TCM_HWNDCLOCK,
		TCM_CLOCKERROR,
		TCM_EXIT,
		TCM_RELOADSETTING,
		WM_COPYDATA,
		MM_MCINOTIFY,
		MM_WOM_DONE,
		TCM_STOPSOUND,
		
		WM_DROPFILES,
		WM_MOUSEWHEEL,
		WM_LBUTTONDOWN,
		WM_RBUTTONDOWN,
		WM_MBUTTONDOWN,
		WM_XBUTTONDOWN,
		WM_LBUTTONUP,
		WM_RBUTTONUP,
		WM_MBUTTONUP,
		WM_XBUTTONUP,
	};
	
	for(i = 0; i < sizeof(messages) / sizeof(UINT); i++)
	{
		ChangeWindowMessageFilter(messages[i], MSGFLT_ADD);
	}
}

