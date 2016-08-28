/*-------------------------------------------------------------
  pagemouse.c : "Mouse" - "Click" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"
#include "../common/command.h"

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnName(HWND hDlg);
static void OnNameDropDown(HWND hDlg);
static void OnAdd(HWND hDlg);
static void OnDelete(HWND hDlg);
static void OnFunction(HWND hDlg, BOOL bInit);
static void OnMouseButton(HWND hDlg);
static void OnBrowse(HWND hDlg);
static void SetMouseCommandToDlg(HWND hDlg, PMOUSESTRUCT pitem);
static void GetMouseCommandFromDlg(HWND hDlg, PMOUSESTRUCT pitem);
static void EnableMousePageItems(HWND hDlg);
static void InitFunction(HWND hDlg, int id);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

static char *m_section = "Mouse";
static PMOUSESTRUCT m_pMouseCommand = NULL;
static int m_nCurrent = -1;
static int m_widthOpt = 0;
static int m_prevcommand = -1;

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageMouseProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_DESTROY:
			OnDestroy(hDlg);
			break;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
				case IDC_NAMECLICK:
					if(code == CBN_SELCHANGE)
					{
						m_bInit = FALSE;
						OnName(hDlg);
						m_bInit = TRUE;
					}
					else if(code == CBN_DROPDOWN)
						OnNameDropDown(hDlg);
					else if(code == CBN_EDITCHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_ADDCLICK:
					OnAdd(hDlg);
					break;
				case IDC_DELCLICK:
					OnDelete(hDlg);
					break;
				case IDC_MOUSEBUTTON:
					if(code == CBN_SELCHANGE)
					{
#if TC_ENABLE_WHEEL
						OnMouseButton(hDlg);
#endif
						SendPSChanged(hDlg);
					}
					break;
				case IDC_RADSINGLE:
				case IDC_RADDOUBLE:
				case IDC_RADTRIPLE:
				case IDC_RADQUADRUPLE:
				case IDC_MOUSECTRL:
				case IDC_MOUSESHIFT:
				case IDC_MOUSEALT:
				case IDC_RCLICKMENU:
					SendPSChanged(hDlg);
					break;
				case IDC_LMOUSEPASSTHRU:
					g_bApplyClock = TRUE;
					SendPSChanged(hDlg);
					break;
				case IDC_MOUSEFUNC:
					OnFunction(hDlg, TRUE);
					break;
				case IDC_MOUSEOPT:
					if(code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_MOUSEOPTSANSHO:
					OnBrowse(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Mouse"); break;
			}
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
  notify parent window to enable "Apply" button
--------------------------------------------------*/
void SendPSChanged(HWND hDlg)
{
	if(m_bInit)
	{
		g_bApplyMain = TRUE;
		m_bChanged = TRUE;
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	}
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	PMOUSESTRUCT pitem;
	RECT rc;
	BOOL b;
	
	m_bInit = FALSE;
	
	if(GetMyRegLong(m_section, "ver031031", 0) == 0)
	{
		SetMyRegLong(m_section, "ver031031", 1);
		ImportOldMouseFunc(); // common/mousestruct.c
	}
	
	// common/mousestruct.c
	m_pMouseCommand = LoadMouseFunc();
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Mouse", g_hfontDialog);
	
	GetWindowRect(GetDlgItem(hDlg, IDC_MOUSEOPT), &rc);
	m_widthOpt = rc.right - rc.left;
	
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_LEFTBUTTON, "LButton"));
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_RIGHTBUTTONM, "RButton"));
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_MIDDLEBUTTONM, "MButton"));
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_XBUTTON1, "XButton1"));
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_XBUTTON2, "XButton2"));
#if TC_ENABLE_WHEEL
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_WHEELUP, "WheelUp"));
	CBAddString(hDlg, IDC_MOUSEBUTTON,
		(LPARAM)MyString(IDS_WHEELDOWN, "WheelDown"));
#endif
	
	InitFunction(hDlg, IDC_MOUSEFUNC);
	
	pitem = m_pMouseCommand;
	while(pitem)
	{
		CBAddString(hDlg, IDC_NAMECLICK, (LPARAM)pitem->name);
		pitem = pitem->next;
	}
	
	m_nCurrent = -1;
	CBSetCurSel(hDlg, IDC_NAMECLICK, 0);
	OnName(hDlg);
	
	CheckDlgButton(hDlg, IDC_LMOUSEPASSTHRU,
		GetMyRegLong(m_section, "LeftMousePassThrough",
			(g_winver&WIN10RS1) != 0));
	
	b = GetMyRegLong(NULL, "RightClickMenu", TRUE);
	b = GetMyRegLong(m_section, "RightClickMenu", b);
	CheckDlgButton(hDlg, IDC_RCLICKMENU, b);
	
	m_bInit = TRUE;
}

/*------------------------------------------------
   WM_DESTROY message
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	clear_list(m_pMouseCommand);
	m_pMouseCommand = NULL;
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	OnNameDropDown(hDlg);
	
	GetMouseCommandFromDlg(hDlg, get_listitem(m_pMouseCommand, m_nCurrent));
	
	SaveMouseFunc(m_pMouseCommand); // common/mousestruct.c
	
	SetMyRegLong(m_section, "LeftMousePassThrough",
		IsDlgButtonChecked(hDlg, IDC_LMOUSEPASSTHRU));
	
	SetMyRegLong(m_section, "RightClickMenu",
		IsDlgButtonChecked(hDlg, IDC_RCLICKMENU));
	DelMyReg(NULL, "RightClickMenu");
}

/*------------------------------------------------
   "Name" is selected
--------------------------------------------------*/
void OnName(HWND hDlg)
{
	int index;
	
	index = CBGetCurSel(hDlg, IDC_NAMECLICK);
	if(index < 0) return ;
	
	GetMouseCommandFromDlg(hDlg, get_listitem(m_pMouseCommand, m_nCurrent));
	
	SetMouseCommandToDlg(hDlg, get_listitem(m_pMouseCommand, index));
	
	OnFunction(hDlg, TRUE);
#if TC_ENABLE_WHEEL
	OnMouseButton(hDlg);
#endif
	m_nCurrent = index;
}

/*------------------------------------------------
   combo box is about to be visible
   set edited text to combo box
--------------------------------------------------*/
void OnNameDropDown(HWND hDlg)
{
	char name[BUFSIZE_NAME];
	PMOUSESTRUCT pitem;
	
	pitem = get_listitem(m_pMouseCommand, m_nCurrent);
	if(pitem == NULL) return;
	
	GetDlgItemText(hDlg, IDC_NAMECLICK, name, BUFSIZE_NAME);
	
	if(strcmp(name, pitem->name) != 0)
	{
		strcpy(pitem->name, name);
		CBDeleteString(hDlg, IDC_NAMECLICK, m_nCurrent);
		CBInsertString(hDlg, IDC_NAMECLICK, m_nCurrent, name);
	}
}

/*------------------------------------------------
  "Add"
--------------------------------------------------*/
void OnAdd(HWND hDlg)
{
	PMOUSESTRUCT pitem;
	int count, index;
	
	count = CBGetCount(hDlg, IDC_NAMECLICK);
	if(count < 0) return;
	
	OnNameDropDown(hDlg);
	
	GetMouseCommandFromDlg(hDlg, get_listitem(m_pMouseCommand, m_nCurrent));
	
	pitem = malloc(sizeof(MOUSESTRUCT));
	memset(pitem, 0, sizeof(MOUSESTRUCT));
	wsprintf(pitem->name, "Mouse%d", count+1);
	pitem->nClick = 1;
	pitem->nCommand = 0;
	// common/list.c
	m_pMouseCommand = add_listitem(m_pMouseCommand, pitem);
	
	index = CBAddString(hDlg, IDC_NAMECLICK, (LPARAM)pitem->name);
	CBSetCurSel(hDlg, IDC_NAMECLICK, index);
	m_nCurrent = index;
	
	if(count == 0)
		EnableMousePageItems(hDlg);
	
	SetMouseCommandToDlg(hDlg, pitem);
	OnFunction(hDlg, FALSE);
#if TC_ENABLE_WHEEL
	OnMouseButton(hDlg);
#endif
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  "Delete"
--------------------------------------------------*/
void OnDelete(HWND hDlg)
{
	int count, index;
	PMOUSESTRUCT pitem;
	
	count = CBGetCount(hDlg, IDC_NAMECLICK);
	if(count < 1) return;
	
	index = CBGetCurSel(hDlg, IDC_NAMECLICK);
	if(index < 0) return;
	
	pitem = get_listitem(m_pMouseCommand, index);
	if(pitem == NULL) return;
	// common/list.c
	m_pMouseCommand = del_listitem(m_pMouseCommand, pitem);
	
	CBDeleteString(hDlg, IDC_NAMECLICK, index);
	
	if(count > 0)
	{
		if(index == count - 1) index--;
		CBSetCurSel(hDlg, IDC_NAMECLICK, index);
		
		SetMouseCommandToDlg(hDlg, get_listitem(m_pMouseCommand, index));
		OnFunction(hDlg, TRUE);
	}
	else
	{
		index = -1;
		
		EnableMousePageItems(hDlg);
		SetMouseCommandToDlg(hDlg, NULL);
		OnFunction(hDlg, FALSE);
#if TC_ENABLE_WHEEL
		OnMouseButton(hDlg);
#endif
	}
	m_nCurrent = index;
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
   "Function" is selected
--------------------------------------------------*/
void OnFunction(HWND hDlg, BOOL bInit)
{
	RECT rc;
	int command = CBGetItemData(hDlg, IDC_MOUSEFUNC,
					CBGetCurSel(hDlg, IDC_MOUSEFUNC));
	
	if(!bInit || command != m_prevcommand)
		SetDlgItemText(hDlg, IDC_MOUSEOPT, "");
	
	m_prevcommand = command;
	
	if(command == IDC_OPENFILE || command == IDC_MOUSECOPY
		|| command == IDC_MONOFF || command == IDC_COMMAND
#if TC_ENABLE_VOLUME
		|| command == IDC_VOLUD || command == IDC_VOLSET
#endif
		)
	{
		if(command == IDC_OPENFILE)
			SetDlgItemText(hDlg, IDC_LABMOUSEOPT,
				MyString(IDS_FILE, "File"));
		else if(command == IDC_MOUSECOPY)
			SetDlgItemText(hDlg, IDC_LABMOUSEOPT,
				MyString(IDS_FORMATCOPY, "FormatCopy"));
		else if(command == IDC_MONOFF)
			SetDlgItemText(hDlg, IDC_LABMOUSEOPT,
				MyString(IDS_MONOFFSEC, "MonOffSec"));
		else if(command == IDC_COMMAND)
			SetDlgItemText(hDlg, IDC_LABMOUSEOPT,
				MyString(IDS_NUMERO, "Numero"));
#if TC_ENABLE_VOLUME
		else if(command == IDC_VOLSET)
			SetDlgItemText(hDlg, IDC_LABMOUSEOPT,
				MyString(IDS_VOLVAL, "Volume"));
		else if(command == IDC_VOLUD)
			SetDlgItemText(hDlg, IDC_LABMOUSEOPT,
				MyString(IDS_VOLDELTA, "Delta"));
#endif
		
		ShowDlgItem(hDlg, IDC_LABMOUSEOPT, TRUE);
		
		GetWindowRect(GetDlgItem(hDlg, IDC_MOUSEOPT), &rc);
		if(command == IDC_OPENFILE || command == IDC_MOUSECOPY)
			SetWindowPos(GetDlgItem(hDlg, IDC_MOUSEOPT), NULL,
				0, 0, m_widthOpt, rc.bottom - rc.top,
				SWP_NOZORDER|SWP_NOMOVE|SWP_SHOWWINDOW);
		else
		{
			SetWindowPos(GetDlgItem(hDlg, IDC_MOUSEOPT), NULL,
				0, 0, (rc.bottom - rc.top)*2, rc.bottom - rc.top,
				SWP_NOZORDER|SWP_NOMOVE|SWP_SHOWWINDOW);
		}
		
		if(command == IDC_OPENFILE)
			ShowDlgItem(hDlg, IDC_MOUSEOPTSANSHO, TRUE);
		else
			ShowDlgItem(hDlg, IDC_MOUSEOPTSANSHO, FALSE);
	}
	else
	{
		ShowDlgItem(hDlg, IDC_LABMOUSEOPT, FALSE);
		ShowDlgItem(hDlg, IDC_MOUSEOPT, FALSE);
		ShowDlgItem(hDlg, IDC_MOUSEOPTSANSHO, FALSE);
	}
}

#if TC_ENABLE_WHEEL
/*------------------------------------------------
   "MouseButton" is selected
--------------------------------------------------*/
void OnMouseButton(HWND hDlg)
{
	int button = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	int i;
	
	if(button > 4)
	{
		CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADQUADRUPLE, IDC_RADSINGLE);
		for(i = 0; i < 4; i++)
		{
			ShowDlgItem(hDlg, IDC_RADSINGLE + i, FALSE);
		}
	}
	else
	{
		for(i = 0; i < 4; i++)
		{
			ShowDlgItem(hDlg, IDC_RADSINGLE + i, TRUE);
		}
	}
}
#endif

/*------------------------------------------------
  select file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_MOUSEOPT, deffile, MAX_PATH);
	
	// common/selectfile.c
	if(!SelectMyFile(g_hInst, hDlg, "All (*.*)\0*.*\0\0",
		0, deffile, fname))
		return;
	
	SetDlgItemText(hDlg, IDC_MOUSEOPT, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  set MOUSESTRUCT data to dialog
--------------------------------------------------*/
void SetMouseCommandToDlg(HWND hDlg, PMOUSESTRUCT pitem)
{
	MOUSESTRUCT item;
	int i, count;
	
	m_prevcommand = -1;
	
	if(!pitem)
	{
		memset(&item, 0, sizeof(item));
		item.nClick = 1;
		pitem = &item;
	}
	
	CBSetCurSel(hDlg, IDC_MOUSEBUTTON, pitem->nButton);
	
	CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADQUADRUPLE,
		IDC_RADSINGLE + pitem->nClick - 1);
	
	CheckDlgButton(hDlg, IDC_MOUSECTRL,  pitem->bCtrl);
	CheckDlgButton(hDlg, IDC_MOUSESHIFT, pitem->bShift);
	CheckDlgButton(hDlg, IDC_MOUSEALT,   pitem->bAlt);
	
	CBSetCurSel(hDlg, IDC_MOUSEFUNC, 0);
	count = CBGetCount(hDlg, IDC_MOUSEFUNC);
	for(i = 0; i < count; i++)
	{
		if(CBGetItemData(hDlg, IDC_MOUSEFUNC, i) == pitem->nCommand)
		{
			CBSetCurSel(hDlg, IDC_MOUSEFUNC, i);
			m_prevcommand = pitem->nCommand;
			break;
		}
	}
	if(i == count && pitem->nCommand > 100)
	{
		wsprintf(pitem->option, "%d", pitem->nCommand);
		pitem->nCommand = IDC_COMMAND;
		for(i = 0; i < count; i++)
		{
			if(CBGetItemData(hDlg, IDC_MOUSEFUNC, i) == IDC_COMMAND)
			{
				CBSetCurSel(hDlg, IDC_MOUSEFUNC, i);
				m_prevcommand = IDC_COMMAND;
				break;
			}
		}
	}
	
	SetDlgItemText(hDlg, IDC_MOUSEOPT, pitem->option);
}

/*------------------------------------------------
  get MOUSESTRUCT data from dialog
--------------------------------------------------*/
void GetMouseCommandFromDlg(HWND hDlg, PMOUSESTRUCT pitem)
{
	int i;
	
	if(!pitem) return;
	
	pitem->nButton = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	
	for(i = 0; i < 4; i++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + i))
		{
			pitem->nClick = i + 1;
			break;
		}
	}
	
	pitem->bCtrl  = IsDlgButtonChecked(hDlg, IDC_MOUSECTRL);
	pitem->bShift = IsDlgButtonChecked(hDlg, IDC_MOUSESHIFT);
	pitem->bAlt   = IsDlgButtonChecked(hDlg, IDC_MOUSEALT);
	
	pitem->nCommand = CBGetItemData(hDlg, IDC_MOUSEFUNC,
			CBGetCurSel(hDlg, IDC_MOUSEFUNC));
	GetDlgItemText(hDlg, IDC_MOUSEOPT, pitem->option, MAX_PATH);
}

/*------------------------------------------------
  enable/disable all dialog items
--------------------------------------------------*/
void EnableMousePageItems(HWND hDlg)
{
	HWND hwnd;
	BOOL b = (m_pMouseCommand != NULL);
	
	hwnd = GetWindow(hDlg, GW_CHILD);
	while(hwnd)
	{
		if(GetDlgCtrlID(hwnd) == IDC_LMOUSEPASSTHRU) continue;
		if(GetDlgCtrlID(hwnd) == IDC_RCLICKMENU) break;
		
		if(GetDlgCtrlID(hwnd) != IDC_ADDCLICK)
			EnableWindow(hwnd, b);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

/*------------------------------------------------
   initialize "Function" combobox
--------------------------------------------------*/

static struct {
	int   idStr;
	char *entry;
	int   nCommand;
} m_mousefunc[] = {
	{ IDS_NONE,        "None",       0 },
	{ IDS_OPENFILE,    "OpenFile",   IDC_OPENFILE  },
	{ IDS_MOUSECOPY,   "MouseCopy",  IDC_MOUSECOPY },
	{ IDS_SYNCTIME,    "SyncTime",   IDC_SYNCTIME },
	{ IDS_TCLOCKMENU,  "TClockMenu", IDC_TCLOCKMENU },
	{ IDS_PROPDATE,    "PropDate",   IDC_DATETIME },
	{ IDS_EXITWIN,     "ExitWin",    IDC_EXITWIN },
	{ IDS_RUN,         "Run",        IDC_RUNAPP },
	{ IDS_ALLMIN,      "AllMin",     IDC_MINALL },
	{ IDS_SHOWDESK,    "ShowDesk",   IDC_SHOWDESK },
	{ IDS_SCREENSAVER, "SSaver",     IDC_SCREENSAVER },
	{ IDS_MONOFF,      "MonOff",     IDC_MONOFF },
	{ IDS_KYU,         "Kyu",        IDC_KYU },
	{ IDS_TCCOMMAND,   "TCCmd",      IDC_COMMAND },
#if TC_ENABLE_VOLUME
	{ IDS_VOLSET,      "Volset",     IDC_VOLSET },
	{ IDS_VOLUD,       "VolUD",      IDC_VOLUD },
	{ IDS_MUTE,        "Mute",       IDC_MUTE },
#endif
};

#define MAX_MOUSEFUNC	ARRAYSIZE(m_mousefunc)

void InitFunction(HWND hDlg, int id)
{
	int index;
	int i;
	
	for(i = 0; i < MAX_MOUSEFUNC; i++)
	{
		index = CBAddString(hDlg, id,
			(LPARAM)MyString(m_mousefunc[i].idStr, m_mousefunc[i].entry));
		CBSetItemData(hDlg, id, index, m_mousefunc[i].nCommand);
	}
}
