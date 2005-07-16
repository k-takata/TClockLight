/*-------------------------------------------------------------
  main.c : TClock properties dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

void MyHelp(HWND hwnd, const char *title);
BOOL ExecCommandString(HWND hwnd, const char* command);

HINSTANCE g_hInst;                 // instance handle
char      g_mydir[MAX_PATH];       // path to this exe file
BOOL      g_bIniSetting = FALSE;   // save setting to ini file?
char      g_inifile[MAX_PATH];     // ini file name
char      g_langfile[MAX_PATH];    // tclang.txt
HFONT     g_hfontDialog = NULL;    // dialog font
int       g_winver;                // Windows version flags
BOOL      g_bApplyClock = FALSE;   // refresh clock
BOOL      g_bApplyTaskbar = FALSE; // refresh task bar
BOOL      g_bApplyStartMenu = FALSE; // refresh Start menu
BOOL      g_bApplyTip = FALSE;     // refresh tooltip
BOOL      g_bApplyMain = FALSE;    // refresh TClock main window
HICON     g_hIconPlay, g_hIconStop; // icons to use frequently

/* Statics */

static int TCPropMain(void);
static void InitTCProp(void);
static INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
static void OnInitDialog(HWND hDlg);
static void OnOK(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnHelp(HWND hDlg);
static void OnTVChanged(HWND hDlg, int nItem);
static void InitTreeView(HWND hDlg);

static int m_lastTreeItem;

/* --------- Property pages -------------- */

#define MAX_PAGE      13

enum {
	PAGE_COLOR, PAGE_SIZE, PAGE_FORMAT, PAGE_ANALOGCLOCK,
	PAGE_ALARM, PAGE_CUCKOO,
	PAGE_MOUSE, PAGE_MOUSE2,
	PAGE_TOOLTIP,
	/* PAGE_SNTP, */
	PAGE_STARTBUTTON, PAGE_STARTMENU, PAGE_TASKBAR,
	PAGE_MISC
};

static struct {
	HWND hDlg;
	int idDlg;
	DLGPROC dlgproc;
} g_dlgPage[MAX_PAGE] = {
  { NULL, IDD_PAGECOLOR,       PageColorProc },  /* PAGE_COLOR */
  { NULL, IDD_PAGESIZE,        PageSizeProc },   /* PAGE_SIZE */
  { NULL, IDD_PAGEFORMAT,      PageFormatProc }, /* PAGE_FORMAT */
  { NULL, IDD_PAGEANALOGCLOCK, PageAnalogClockProc }, /* PAGE_ANALOGCLOCK */
  { NULL, IDD_PAGEALARM,       PageAlarmProc },  /* PAGE_ALARM */
  { NULL, IDD_PAGECUCKOO,      PageCuckooProc }, /* PAGE_CUCKOO */
  { NULL, IDD_PAGEMOUSE,       PageMouseProc },  /* PAGE_MOUSE */
  { NULL, IDD_PAGEMOUSE2,      PageMouse2Proc }, /* PAGE_MOUSE2 */
  { NULL, IDD_PAGETOOLTIP,     PageTooltipProc }, /* PAGE_TOOLTIP */
/*  { NULL, IDD_PAGESNTP,        PageSNTPProc }, */  /* PAGE_SNTP */
  { NULL, IDD_PAGESTARTBUTTON, PageStartButtonProc }, /* PAGE_STARTBUTTON */
  { NULL, IDD_PAGESTARTMENU,   PageStartMenuProc }, /* PAGE_STARTMENU */
  { NULL, IDD_PAGETASKBAR,     PageTaskbarProc }, /* PAGE_TASKBAR */
  { NULL, IDD_PAGEMISC,        PageMiscProc },   /* PAGE_MISC */
};

/* --------- TreeView items -------------- */

#define MAX_TREEITEM 17

enum {
	ITEM_PARENTCLOCK, ITEM_COLOR, ITEM_SIZE, ITEM_FORMAT,ITEM_ANALOGCLOCK,
	ITEM_PARENTALARM, ITEM_ALARM, ITEM_CUCKOO,
	ITEM_PARENTMOUSE, ITEM_CLICK, ITEM_MOUSEMISC,
	ITEM_TOOLTIP,
	/* ITEM_SNTP, */
	ITEM_PARENTTASKBAR, ITEM_STARTBUTTON, ITEM_STARTMENU, ITEM_TASKBAR,
	ITEM_MISC
};

static struct {
	int   nParent;
	int   idStr;
	char *entry;
	int   nPage;
} g_treeItem[MAX_TREEITEM] = {
  
  /* ITEM_PARENTCLOCK */
  { -1, IDS_CLOCK,   "Clock",   PAGE_COLOR },
  { ITEM_PARENTCLOCK,                               /* ITEM_COLOR */
        IDS_COLOR,   "Color",   PAGE_COLOR },
  { ITEM_PARENTCLOCK,                                /* ITEM_SIZE */
        IDS_SIZEPOS, "SizePos", PAGE_SIZE  },
  { ITEM_PARENTCLOCK,                              /* ITEM_FORMAT */
        IDS_FORMAT,  "Format",  PAGE_FORMAT },
  { ITEM_PARENTCLOCK,                         /* ITEM_ANALOGCLOCK */
        IDS_ANALOGCLOCK, "AnalogClock",  PAGE_ANALOGCLOCK },
  
  /* ITEM_PARENTALARM */
  { -1, IDS_ALARM,  "Alarm",  PAGE_ALARM },
  { ITEM_PARENTALARM,                                /* ITEM_ALARM */
        IDS_ALARM,  "Alarm",  PAGE_ALARM },
  { ITEM_PARENTALARM,                               /* ITEM_CUCKOO */
        IDS_CUCKOO, "Cuckoo", PAGE_CUCKOO },
  
  /* ITEM_PARENTMOUSE */
  { -1, IDS_MOUSE, "Mouse", PAGE_MOUSE }, 
  { ITEM_PARENTMOUSE,                                /* ITEM_CLICK */
        IDS_CLICK, "Click", PAGE_MOUSE },
  { ITEM_PARENTMOUSE,                            /* ITEM_MOUSEMISC */
        IDS_MOUSE2, "Mouse2",  PAGE_MOUSE2 },
  
  /* ITEM_TOOLTIP */
  { -1, IDS_TOOLTIP, "Tooltip", PAGE_TOOLTIP },
  
  /* ITEM_SNTP */
  /* { -1, IDS_SYNC, "Sync", PAGE_SNTP }, */
  
  /* ITEM_PARENTTASKBAR */
  { -1, IDS_TASKBAR,     "Taskbar",     PAGE_STARTBUTTON },
  { ITEM_PARENTTASKBAR,                         /* ITEM_STARTBUTTON */
        IDS_STARTBUTTON, "Startbutton", PAGE_STARTBUTTON },
  { ITEM_PARENTTASKBAR,                         /* ITEM_STARTMENU */
        IDS_STARTMENU,   "Startmenu",   PAGE_STARTMENU },
  { ITEM_PARENTTASKBAR,                             /* ITEM_TASKBAR */
        IDS_TASKBAR,     "Taskbar",     PAGE_TASKBAR },
  
  /* ITEM_MISC */
  { -1, IDS_MISC, "Misc", PAGE_MISC }, 
};

/*-------------------------------------------
  WinMain
---------------------------------------------*/
#ifdef NODEFAULTLIB
void WINAPI WinMainCRTStartup(void)
{
	g_hInst = GetModuleHandle(NULL);
	ExitProcess(TCPropMain());
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	return TCPropMain();
}
#endif

/*-------------------------------------------
  main routine
---------------------------------------------*/
int TCPropMain(void)
{
	WNDCLASS wndclass;
	HWND hwnd;
	
	hwnd = FindWindow(CLASS_TCLOCKPROP, NULL);
	if(hwnd != NULL)
	{
		SetForegroundWindow98(hwnd);
		return 1;
	}
	
	InitCommonControls();
	
	InitTCProp();
	
	// register a window class of dialog
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = DefDlgProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = DLGWINDOWEXTRA;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = CLASS_TCLOCKPROP;
	RegisterClass(&wndclass);
	
	return DialogBox(g_hInst, MAKEINTRESOURCE(IDD_PROPERTY), NULL, DlgProc);
}

/*-------------------------------------------
  initialize
---------------------------------------------*/
void InitTCProp(void)
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
  dialog procedure
---------------------------------------------*/
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)

{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInitDialog(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id; // , code;
			id = LOWORD(wParam); // code = HIWORD(wParam);
			switch(id)
			{
				case IDC_APPLY:
					OnApply(hDlg);
					break;
				case IDC_MYHELP:
					OnHelp(hDlg);
					break;
				case IDOK:
					OnOK(hDlg);
				case IDCANCEL:
					if(g_hfontDialog)
						DeleteObject(g_hfontDialog);
					SetMyRegLong(NULL, "LastTreeItem", m_lastTreeItem);
					EndDialog(hDlg, id);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
		{
			NMTREEVIEW* pNMTV = (NMTREEVIEW *)lParam;
			if(pNMTV->hdr.code == TVN_SELCHANGED)
			{
				OnTVChanged(hDlg, pNMTV->itemNew.lParam);
				return TRUE;
			}
			break;
		}
		case PSM_CHANGED:
			EnableDlgItem(hDlg, IDC_APPLY, TRUE);
			return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------
  initialize main dialog
---------------------------------------------*/
void OnInitDialog(HWND hDlg)
{
	// common/tclang.c
	SetDialogLanguage(hDlg, "Property", g_hfontDialog);
	
	// hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	// SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	
	SetMyDialgPos(hDlg, 32, 32);
	
	InitTreeView(hDlg);
}

/*-------------------------------------------
  "OK" button
---------------------------------------------*/
void OnOK(HWND hDlg)
{
	OnApply(hDlg);
}

/*-------------------------------------------
  "Apply" button
---------------------------------------------*/
void OnApply(HWND hDlg)
{
	HWND hwndClock;
	NMHDR lp;
	int i;
	
	lp.code = PSN_APPLY;
	for(i = 0; i < MAX_PAGE; i++)
	{
		if(g_dlgPage[i].hDlg)
			SendMessage(g_dlgPage[i].hDlg, WM_NOTIFY, 0, (LPARAM)&lp);
	}
	
	hwndClock = GetClockWindow();
	if(hwndClock)
	{
		if(g_bApplyClock)
			PostMessage(hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
		if(g_bApplyTaskbar)
			PostMessage(hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
		if(g_bApplyStartMenu)
			PostMessage(hwndClock, CLOCKM_REFRESHSTARTMENU, 0, 0);
		if(g_bApplyTip)
			PostMessage(hwndClock, CLOCKM_REFRESHTOOLTIP, 0, 0);
	}
	g_bApplyClock = FALSE;
	g_bApplyTaskbar = FALSE;
	g_bApplyStartMenu = FALSE;
	g_bApplyTip = FALSE;
	
	if(g_bApplyMain)
	{
		HWND hwnd = GetTClockMainWindow();
		if(hwnd) PostMessage(hwnd, TCM_RELOADSETTING, 0, 0);
		g_bApplyMain = FALSE;
	}
	
	EnableDlgItem(hDlg, IDC_APPLY, FALSE);
	PostMessage(hDlg, WM_NEXTDLGCTL, 0, FALSE);
}

/*-------------------------------------------
  "Help" button
---------------------------------------------*/
void OnHelp(HWND hDlg)
{
	if(0 <= m_lastTreeItem && m_lastTreeItem < MAX_TREEITEM)
	{
		NMHDR lp;
		int nPage;
		
		nPage = g_treeItem[m_lastTreeItem].nPage;
		lp.code = PSN_HELP;
		if(g_dlgPage[nPage].hDlg)
			SendMessage(g_dlgPage[nPage].hDlg, WM_NOTIFY, 0, (LPARAM)&lp);
	}
}

/*------------------------------------------------
   when tree view item is clicked,
   create/show a child dialog.
--------------------------------------------------*/
void OnTVChanged(HWND hDlg, int nItem)
{
	int i, nPage;
	BOOL bexist = FALSE;
	HWND hDlgPage;
	RECT rc;
	POINT pt;
	
	if(nItem >= MAX_TREEITEM) return;
	
	m_lastTreeItem = nItem;
	
	nPage = g_treeItem[nItem].nPage;
	
	for(i = 0; i < MAX_PAGE; i++)
	{
		hDlgPage = g_dlgPage[i].hDlg;
		if(hDlgPage)
		{
			if(i == nPage)
			{
				ShowWindow(hDlgPage, SW_SHOW);
				bexist = TRUE;
			}
			else
				ShowWindow(hDlgPage, SW_HIDE);
		}
	}
	
	if(bexist) return;
	
	hDlgPage = CreateDialog(g_hInst,
		MAKEINTRESOURCE(g_dlgPage[nPage].idDlg),
		hDlg, g_dlgPage[nPage].dlgproc);
	g_dlgPage[nPage].hDlg = hDlgPage;
	
	GetWindowRect(GetDlgItem(hDlg, IDC_DUMMY), &rc);
	pt.x = rc.left; pt.y = rc.top;
	ScreenToClient(hDlg, &pt);
	SetWindowPos(hDlgPage, GetDlgItem(hDlg, IDC_TREE),
		pt.x, pt.y, rc.right-rc.left, rc.bottom-rc.top,
		SWP_SHOWWINDOW);
}

/*-------------------------------------------
  set items to left tree view
---------------------------------------------*/
void InitTreeView(HWND hDlg)
{
	HWND hTree;
	TVINSERTSTRUCT tv;
	HTREEITEM hTreeItem[MAX_TREEITEM];
	int i;
	
	hTree = GetDlgItem(hDlg, IDC_TREE);
	memset(&tv, 0, sizeof(TVINSERTSTRUCT));
	
	tv.hInsertAfter = TVI_LAST;
	tv.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	tv.item.state = TVIS_EXPANDED;
	tv.item.stateMask = TVIS_EXPANDED;
	
	for(i = 0; i < MAX_TREEITEM; i++)
	{
		int nParent = g_treeItem[i].nParent;
		
		tv.item.lParam = i;
		if(nParent < 0) tv.hParent = TVI_ROOT;
		else tv.hParent = hTreeItem[nParent];
		tv.item.pszText = 
			MyString(g_treeItem[i].idStr, g_treeItem[i].entry);
		hTreeItem[i] = TreeView_InsertItem(hTree, &tv);
	}
	
	m_lastTreeItem = GetMyRegLong(NULL, "LastTreeItem", 0);
	if(m_lastTreeItem >= MAX_TREEITEM) m_lastTreeItem = 0;
	TreeView_SelectItem(hTree, hTreeItem[m_lastTreeItem]);
}

/*-------------------------------------------
  Show "TClock Help"
---------------------------------------------*/
void MyHelp(HWND hwnd, const char *section)
{
	char helpurl[MAX_PATH], title[MAX_PATH];
	
	if(!g_langfile[0]) return;
	
	GetMyRegStr(NULL, "HelpURL", helpurl, MAX_PATH, "");
	if(helpurl[0] == 0)
	{
		if(GetPrivateProfileString("Main", "HelpURL", "", helpurl,
			MAX_PATH, g_langfile) == 0) return;
	}
	
	if(GetPrivateProfileString(section, "HelpURL", "", title,
			MAX_PATH, g_langfile) == 0) return;
	
	if(strlen(helpurl) > 0 && helpurl[ strlen(helpurl) - 1 ] != '/')
		del_title(helpurl);
	add_title(helpurl, title);
	
	ShellExecute(hwnd, NULL, helpurl, NULL, "", SW_SHOW);
}

/*-------------------------------------------
  called in PlayFile function
---------------------------------------------*/
BOOL ExecCommandString(HWND hwnd, const char* command)
{
	/* ExecFile(hwnd, command); */
	SendStringToOther(GetTClockMainWindow(), hwnd, command, COPYDATA_EXEC);
	
	return FALSE;
}
