/*-------------------------------------------------------------
  pagemouse.c : "Mouse" - "Click" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"
#include "../common/command.h"

/* Globals */

INT_PTR CALLBACK PageMouseProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

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
static void SetMouseCommandToDlg(HWND hDlg, PMOUSESTRUCT pMSS);
static void GetMouseCommandFromDlg(HWND hDlg, PMOUSESTRUCT pMSS);
static void EnableMousePageItems(HWND hDlg);
static void InitFunction(HWND hDlg, int id);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

static char *m_section = "Mouse";
static PMOUSESTRUCT m_pMouseCommand = NULL;
static int m_numMouseCommand = 0;
static int m_numOld = 0;
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
	PMOUSESTRUCT pMSS;
	RECT rc;
	BOOL b;
	int i;
	
	m_bInit = FALSE;
	
	if(GetMyRegLong(m_section, "ver031031", 0) == 0)
	{
		SetMyRegLong(m_section, "ver031031", 1);
		ImportOldMouseFunc(); // common/mousestruct.c
	}
	
	m_numMouseCommand = GetMyRegLong(m_section, "MouseNum", 0);
	m_numOld = m_numMouseCommand;
	
	if(m_numMouseCommand == 0) m_numMouseCommand = 1;
	
	m_pMouseCommand = malloc(sizeof(MOUSESTRUCT) * m_numMouseCommand);
	// common/mousestruct.c
	LoadMouseFunc(m_pMouseCommand, m_numMouseCommand);
	
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
	
	pMSS = m_pMouseCommand;
	for(i = 0; i < m_numMouseCommand; i++)
	{
		CBAddString(hDlg, IDC_NAMECLICK, (LPARAM)pMSS->name);
		pMSS++;
	}
	
	CBSetCurSel(hDlg, IDC_NAMECLICK, 0);
	OnName(hDlg);
	
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
	if(m_pMouseCommand) free(m_pMouseCommand);
	m_pMouseCommand = NULL;
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char section[20];
	int i;
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	OnNameDropDown(hDlg);
	
	if(m_pMouseCommand && 0 <= m_nCurrent && m_nCurrent < m_numMouseCommand)
		GetMouseCommandFromDlg(hDlg, (m_pMouseCommand + m_nCurrent));
	
	SetMyRegLong(m_section, "MouseNum", m_numMouseCommand);
	if(m_pMouseCommand)
		SaveMouseFunc(m_pMouseCommand, m_numMouseCommand);
	
	if(m_numMouseCommand < m_numOld)
	{
		for(i = m_numMouseCommand; i < m_numOld; i++)
		{
			wsprintf(section, "Mouse%d", i+1);
			DelMyRegKey(section);
		}
	}
	m_numOld = m_numMouseCommand;
	
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
	
	if(m_pMouseCommand &&
		0 <= m_nCurrent && m_nCurrent < m_numMouseCommand)
		GetMouseCommandFromDlg(hDlg, m_pMouseCommand + m_nCurrent);
	
	index = CBGetCurSel(hDlg, IDC_NAMECLICK);
	
	if(m_pMouseCommand &&
		0 <= index && index < m_numMouseCommand)
	{
		SetMouseCommandToDlg(hDlg, m_pMouseCommand + index);
		OnFunction(hDlg, TRUE);
#if TC_ENABLE_WHEEL
		OnMouseButton(hDlg);
#endif
		m_nCurrent = index;
	}
}

/*------------------------------------------------
   combo box is about to be visible
   set edited text to combo box
--------------------------------------------------*/
void OnNameDropDown(HWND hDlg)
{
	char name[BUFSIZE_NAME];
	
	if(!m_pMouseCommand ||
		!(0 <= m_nCurrent && m_nCurrent < m_numMouseCommand)) return;
	
	GetDlgItemText(hDlg, IDC_NAMECLICK, name, BUFSIZE_NAME);
	
	if(strcmp(name, m_pMouseCommand[m_nCurrent].name) != 0)
	{
		CBDeleteString(hDlg, IDC_NAMECLICK, m_nCurrent);
		CBInsertString(hDlg, IDC_NAMECLICK, m_nCurrent, name);
		strcpy(m_pMouseCommand[m_nCurrent].name, name);
	}
}

/*------------------------------------------------
  "Add"
--------------------------------------------------*/
void OnAdd(HWND hDlg)
{
	PMOUSESTRUCT pNew, pMSS;
	int i;
	
	OnNameDropDown(hDlg);
	
	if(m_pMouseCommand &&
		(0 <= m_nCurrent && m_nCurrent < m_numMouseCommand))
		GetMouseCommandFromDlg(hDlg, (m_pMouseCommand + m_nCurrent));
	
	pNew = malloc(sizeof(MOUSESTRUCT)*(m_numMouseCommand+1));
	for(i = 0; i < m_numMouseCommand && m_pMouseCommand; i++)
		memcpy(pNew + i, m_pMouseCommand + i, sizeof(MOUSESTRUCT));
	
	pMSS = pNew + i;
	memset(pMSS, 0, sizeof(MOUSESTRUCT));
	wsprintf(pMSS->name, "Mouse%d", i+1);
	pMSS->nClick = 1;
	pMSS->nCommand = 0;
	
	CBAddString(hDlg, IDC_NAMECLICK, (LPARAM)pMSS->name);
	CBSetCurSel(hDlg, IDC_NAMECLICK, i);
	m_nCurrent = i;
	
	m_numMouseCommand++;
	if(m_pMouseCommand) free(m_pMouseCommand);
	m_pMouseCommand = pNew;
	
	if(m_numMouseCommand == 1)
		EnableMousePageItems(hDlg);
	
	SetMouseCommandToDlg(hDlg, pMSS);
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
	PMOUSESTRUCT pNew;
	int i, j;
	
	if(!m_pMouseCommand || m_numMouseCommand < 1) return;
	if(!(0 <= m_nCurrent && m_nCurrent < m_numMouseCommand)) return;
	
	if(m_numMouseCommand > 1)
	{
		pNew = malloc(sizeof(MOUSESTRUCT)*(m_numMouseCommand-1));
		for(i = 0, j = 0; i < m_numMouseCommand; i++)
		{
			if(i != m_nCurrent)
			{
				memcpy(pNew + j, m_pMouseCommand + i, sizeof(MOUSESTRUCT));
				j++;
			}
		}
		
		CBDeleteString(hDlg, IDC_NAMECLICK, m_nCurrent);
		
		if(m_nCurrent == m_numMouseCommand - 1) m_nCurrent--;
		CBSetCurSel(hDlg, IDC_NAMECLICK, m_nCurrent);
		SetMouseCommandToDlg(hDlg, (pNew + m_nCurrent));
		OnFunction(hDlg, TRUE);
		
		m_numMouseCommand--;
		free(m_pMouseCommand);
		m_pMouseCommand = pNew;
	}
	else
	{
		free(m_pMouseCommand); m_pMouseCommand = NULL;
		m_numMouseCommand = 0;
		m_nCurrent = -1;
		
		CBDeleteString(hDlg, IDC_NAMECLICK, 0);
		EnableMousePageItems(hDlg);
		SetMouseCommandToDlg(hDlg, NULL);
		OnFunction(hDlg, FALSE);
#if TC_ENABLE_WHEEL
		OnMouseButton(hDlg);
#endif
	}
	
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
void SetMouseCommandToDlg(HWND hDlg, PMOUSESTRUCT pMSS)
{
	int i, count;
	
	m_prevcommand = -1;
	
	if(!pMSS)
	{
		SetDlgItemText(hDlg, IDC_NAMECLICK, "");
		CBSetCurSel(hDlg, IDC_MOUSEBUTTON, 0);
		CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADQUADRUPLE,
			IDC_RADSINGLE);
		CheckDlgButton(hDlg, IDC_MOUSECTRL,  FALSE);
		CheckDlgButton(hDlg, IDC_MOUSESHIFT, FALSE);
		CheckDlgButton(hDlg, IDC_MOUSEALT,   FALSE);
		CBSetCurSel(hDlg, IDC_MOUSEFUNC, 0);
		ShowDlgItem(hDlg, IDC_LABMOUSEOPT, FALSE);
		ShowDlgItem(hDlg, IDC_MOUSEOPT, FALSE);
		ShowDlgItem(hDlg, IDC_MOUSEOPTSANSHO, FALSE);
		ShowDlgItem(hDlg, IDC_LABMOUSEOPT2, FALSE);
		return;
	}
	
	CBSetCurSel(hDlg, IDC_MOUSEBUTTON, pMSS->nButton);
	
	CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADQUADRUPLE,
		IDC_RADSINGLE + pMSS->nClick - 1);
	
	CheckDlgButton(hDlg, IDC_MOUSECTRL,  pMSS->bCtrl);
	CheckDlgButton(hDlg, IDC_MOUSESHIFT, pMSS->bShift);
	CheckDlgButton(hDlg, IDC_MOUSEALT,   pMSS->bAlt);
	
	CBSetCurSel(hDlg, IDC_MOUSEFUNC, 0);
	count = CBGetCount(hDlg, IDC_MOUSEFUNC);
	for(i = 0; i < count; i++)
	{
		if(CBGetItemData(hDlg, IDC_MOUSEFUNC, i) == pMSS->nCommand)
		{
			CBSetCurSel(hDlg, IDC_MOUSEFUNC, i);
			m_prevcommand = pMSS->nCommand;
			break;
		}
	}
	if(i == count && pMSS->nCommand > 100)
	{
		wsprintf(pMSS->option, "%d", pMSS->nCommand);
		pMSS->nCommand = IDC_COMMAND;
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
	
	SetDlgItemText(hDlg, IDC_MOUSEOPT, pMSS->option);
}

/*------------------------------------------------
  get MOUSESTRUCT data from dialog
--------------------------------------------------*/
void GetMouseCommandFromDlg(HWND hDlg, PMOUSESTRUCT pMSS)
{
	int i;
	
	if(!pMSS) return;
	
	pMSS->nButton = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	
	for(i = 0; i < 4; i++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + i))
		{
			pMSS->nClick = i + 1;
			break;
		}
	}
	
	pMSS->bCtrl  = IsDlgButtonChecked(hDlg, IDC_MOUSECTRL);
	pMSS->bShift = IsDlgButtonChecked(hDlg, IDC_MOUSESHIFT);
	pMSS->bAlt   = IsDlgButtonChecked(hDlg, IDC_MOUSEALT);
	
	pMSS->nCommand = CBGetItemData(hDlg, IDC_MOUSEFUNC,
			CBGetCurSel(hDlg, IDC_MOUSEFUNC));
	GetDlgItemText(hDlg, IDC_MOUSEOPT, pMSS->option, MAX_PATH);
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
