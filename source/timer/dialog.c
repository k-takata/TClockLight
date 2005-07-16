/*-------------------------------------------------------------
  dialog.c : Timer dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tctimer.h"

/* Globals */

void OnShowDialog(HWND hwnd);

HWND g_hDlg = NULL;

/* Statics */

static INT_PTR CALLBACK DlgProcTimer(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnOK(HWND hDlg);
static void OnCancel(HWND hDlg);
static void OnHelp(HWND hDlg);
static void OnName(HWND hDlg);
static void OnNameDropDown(HWND hDlg);
static void OnAdd(HWND hDlg);
static void OnDelete(HWND hDlg);
static void OnBrowse(HWND hDlg);
static void OnTest(HWND hDlg);
static void OnShowTime(HWND hDlg);
static void OnUserStr(HWND hDlg);
static void GetTimerFromDlg(HWND hDlg, PTIMERSTRUCT pitem);
static void SetTimerToDlg(HWND hDlg, const TIMERSTRUCT *pitem);
static void EnableTimerDlgItems(HWND hDlg);

static PTIMERSTRUCT m_pTimer = NULL;
static int m_nCurrent = -1;
static BOOL m_bPlaying = FALSE;  // sound is plaing

/*-------------------------------------------------------
  TIMERM_SHOWDLG message
---------------------------------------------------------*/
void OnShowDialog(HWND hwnd)
{
	if(!(g_hDlg && IsWindow(g_hDlg)))
		g_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_TIMER),
			NULL, DlgProcTimer);
	SetForegroundWindow98(g_hDlg);
}

/*-------------------------------------------
  dialog procedure
---------------------------------------------*/
INT_PTR CALLBACK DlgProcTimer(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			int id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
				case IDC_TIMERNAME:
					if(code == CBN_SELCHANGE)
						OnName(hDlg);
					else if(code == CBN_DROPDOWN)
						OnNameDropDown(hDlg);
					break;
				case IDC_TIMERADD:
					OnAdd(hDlg);
					break;
				case IDC_TIMERDEL:
					OnDelete(hDlg);
					break;
				case IDC_TIMERSANSHO:
					OnBrowse(hDlg);
					break;
				case IDC_TIMERTEST:
					OnTest(hDlg);
					break;
				case IDC_SHOWTIME:
					OnShowTime(hDlg);
					break;
				case IDC_SHOWWHOLE:
				case IDC_SHOWADD:
				case IDC_SHOWUSTR:
					OnUserStr(hDlg);
					break;
				case IDOK:
					OnOK(hDlg);
					break;
				case IDCANCEL:
					OnCancel(hDlg);
					break;
				case IDC_TIMERHELP:
					OnHelp(hDlg);
					break;
			}
			return TRUE;
		}
		// playing sound ended
		case MM_MCINOTIFY:
		case MM_WOM_DONE:
			if(message == MM_MCINOTIFY)
				OnMCINotify(hDlg, wParam, (LONG)lParam);
			else
				StopFile();
			m_bPlaying = FALSE;
			SendDlgItemMessage(hDlg, IDC_TIMERTEST,
				BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
			return TRUE;
		case WM_DESTROY:
			OnDestroy(hDlg);
			break;
	}
	return FALSE;
}

/*-------------------------------------------
  initialize main dialog
---------------------------------------------*/
void OnInit(HWND hDlg)
{
	HICON hIcon;
	char section[20];
	int i, count;
	TIMERSTRUCT item;
	PTIMERSTRUCT pitem;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Timer", g_hfontDialog);
	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	
	// ../common/dialog.c
	SetMyDialgPos(hDlg, 32, 32);
	
	SendDlgItemMessage(hDlg, IDC_TIMERTEST, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconPlay);
	
	UpDown_SetBuddy(hDlg, IDC_TIMERSPIN1, IDC_TIMERMINUTE);
	UpDown_SetBuddy(hDlg, IDC_TIMERSPIN2, IDC_TIMERSECOND);
	
	UpDown_SetRange(hDlg, IDC_TIMERSPIN1, 1440, 0);
	UpDown_SetRange(hDlg, IDC_TIMERSPIN2, 59, 0);
	
	count = GetMyRegLong(NULL, "TimerNum", 0);
	m_pTimer = NULL;
	if(count > 0)
	{
		for(i = 0; i < count; i++)
		{
			memset(&item, 0, sizeof(TIMERSTRUCT));
			wsprintf(section, "Timer%d", i + 1);
			GetMyRegStr(section, "Name", item.name, BUFSIZE_NAME, section);
			item.minute = GetMyRegLong(section, "Minute", 3);
			item.second = GetMyRegLong(section, "Second", 0);
			GetMyRegStr(section, "File", item.fname, MAX_PATH, "");
			item.bRepeat = GetMyRegLong(section, "Repeat", FALSE);
			item.bBlink = GetMyRegLong(section, "Blink", FALSE);
			item.bDisp = GetMyRegLong(section, "Disp", FALSE);
			item.nDispType = GetMyRegLong(section, "DispType", 1);
			item.nUserStr = GetMyRegLong(section, "UserStr", 0);
			
			m_pTimer = copy_listitem(m_pTimer, &item, sizeof(TIMERSTRUCT));
			// common/list.c
		}
	}
	else
	{
		memset(&item, 0, sizeof(TIMERSTRUCT));
		strcpy(item.name, "Timer1");
		item.minute = 3;
		item.nDispType = 1;
		
		m_pTimer = copy_listitem(m_pTimer, &item, sizeof(TIMERSTRUCT));
	}
	
	pitem = m_pTimer;
	while(pitem)
	{
		CBAddString(hDlg, IDC_TIMERNAME, (LPARAM)pitem->name);
		pitem = pitem->next;
	}
	
	m_nCurrent = -1;
	CBSetCurSel(hDlg, IDC_TIMERNAME, 0);
	OnName(hDlg);
}

/*------------------------------------------------
   clear up
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	g_hDlg = NULL;
	
	clear_list(m_pTimer);
	m_pTimer = NULL;
	m_nCurrent = -1;
	
	StopFile();
	m_bPlaying = FALSE;
}

/*-------------------------------------------
  "Start" button
---------------------------------------------*/
void OnOK(HWND hDlg)
{
	PTIMERSTRUCT pitem;
	char section[20];
	int i, nOldTimer;
	
	/* save settings */
	GetTimerFromDlg(hDlg, get_listitem(m_pTimer, m_nCurrent));
	
	nOldTimer = GetMyRegLong(NULL, "TimerNum", 0);
	
	pitem = m_pTimer;
	for(i = 0; pitem; i++)
	{
		wsprintf(section, "Timer%d", i + 1);
		SetMyRegStr(section, "Name", pitem->name);
		SetMyRegLong(section, "Minute", pitem->minute);
		SetMyRegLong(section, "Second", pitem->second);
		SetMyRegStr(section, "File", pitem->fname);
		SetMyRegLong(section, "Repeat", pitem->bRepeat);
		SetMyRegLong(section, "Blink", pitem->bBlink);
		SetMyRegLong(section, "Disp", pitem->bDisp);
		SetMyRegLong(section, "DispType", pitem->nDispType);
		SetMyRegLong(section, "UserStr", pitem->nUserStr);
		
		pitem = pitem->next;
	}
	
	SetMyRegLong(NULL, "TimerNum", i);
	
	for(; i < nOldTimer; i++)
	{
		wsprintf(section, "Timer%d", i + 1);
		DelMyRegKey(section);
	}
	
	/* start a timer */
	TimerStart(get_listitem(m_pTimer, m_nCurrent));
	
	DestroyWindow(hDlg);
}

/*-------------------------------------------
  "Cancel" button
---------------------------------------------*/
void OnCancel(HWND hDlg)
{
	if(!IsTimerRunning())
		PostMessage(g_hwndTimer, WM_CLOSE, 0, 0);
	DestroyWindow(hDlg);
}

/*-------------------------------------------
  "Help" button
---------------------------------------------*/
void OnHelp(HWND hDlg)
{
	char helpurl[MAX_PATH], title[MAX_PATH];
	
	if(!g_langfile[0]) return;
	
	GetMyRegStr(NULL, "HelpURL", helpurl, MAX_PATH, "");
	if(helpurl[0] == 0)
	{
		if(GetPrivateProfileString("Main", "HelpURL", "", helpurl,
			MAX_PATH, g_langfile) == 0) return;
	}
	
	if(GetPrivateProfileString("Timer", "HelpURL", "", title,
		MAX_PATH, g_langfile) == 0) return;
	
	if(strlen(helpurl) > 0 && helpurl[ strlen(helpurl) - 1 ] != '/')
		del_title(helpurl);
	add_title(helpurl, title);
	
	ShellExecute(hDlg, NULL, helpurl, NULL, "", SW_SHOW);
}

/*------------------------------------------------
   "Name" is selected
--------------------------------------------------*/
void OnName(HWND hDlg)
{
	int index;
	
	if(!m_pTimer) return;
	
	GetTimerFromDlg(hDlg, get_listitem(m_pTimer, m_nCurrent));
	
	index = CBGetCurSel(hDlg, IDC_TIMERNAME);
	SetTimerToDlg(hDlg, get_listitem(m_pTimer, index));
	m_nCurrent = index;
}

/*------------------------------------------------
   combo box is about to be visible
   set edited text to combo box
--------------------------------------------------*/
void OnNameDropDown(HWND hDlg)
{
	char name[BUFSIZE_NAME];
	PTIMERSTRUCT pitem;
	
	pitem = get_listitem(m_pTimer, m_nCurrent);
	if(pitem == NULL) return;
	
	GetDlgItemText(hDlg, IDC_TIMERNAME, name, BUFSIZE_NAME);
	
	if(strcmp(name, pitem->name) != 0)
	{
		strcpy(pitem->name, name);
		CBDeleteString(hDlg, IDC_TIMERNAME, m_nCurrent);
		CBInsertString(hDlg, IDC_TIMERNAME, m_nCurrent, name);
	}
}

/*------------------------------------------------
  "Add" button
--------------------------------------------------*/
void OnAdd(HWND hDlg)
{
	PTIMERSTRUCT pitem;
	int count, index;
	
	count = CBGetCount(hDlg, IDC_TIMERNAME);
	if(count < 0) return;
	
	OnNameDropDown(hDlg);
	
	GetTimerFromDlg(hDlg, get_listitem(m_pTimer, m_nCurrent));
	
	pitem = malloc(sizeof(TIMERSTRUCT));
	memset(pitem, 0, sizeof(TIMERSTRUCT));
	wsprintf(pitem->name, "Timer%d", count + 1);
	pitem->minute = 3;
	pitem->nDispType = 1;
	// common/list.c
	m_pTimer = add_listitem(m_pTimer, pitem); 
	
	index = CBAddString(hDlg, IDC_TIMERNAME, (LPARAM)pitem->name);
	CBSetCurSel(hDlg, IDC_TIMERNAME, index);
	
	if(count == 0)
		EnableTimerDlgItems(hDlg);
	
	SetTimerToDlg(hDlg, pitem);
	m_nCurrent = index;
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  delete a timer
--------------------------------------------------*/
void OnDelete(HWND hDlg)
{
	int count, index;
	PTIMERSTRUCT pitem;
	
	count = CBGetCount(hDlg, IDC_TIMERNAME);
	if(count < 1) return;
	
	index = CBGetCurSel(hDlg, IDC_TIMERNAME);
	if(index < 0) return;
	
	pitem = get_listitem(m_pTimer, index);
	if(pitem == NULL) return;
	// common/list.c
	m_pTimer = del_listitem(m_pTimer, pitem);
	
	CBDeleteString(hDlg, IDC_TIMERNAME, index);
	
	if(count > 1)
	{
		if(index == count - 1) index--;
		CBSetCurSel(hDlg, IDC_TIMERNAME, index);
	}
	else
	{
		index = -1;
		EnableTimerDlgItems(hDlg);
	}
	
	SetTimerToDlg(hDlg, get_listitem(m_pTimer, index));
	m_nCurrent = index;
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  "..." button - select file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_TIMERFILE, deffile, MAX_PATH);
	
	// ../common/soundselect.c
	if(BrowseSoundFile(g_hInst, hDlg, deffile, fname))
	{
		SetDlgItemText(hDlg, IDC_TIMERFILE, fname);
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	}
}

/*------------------------------------------------
  test sound
--------------------------------------------------*/
void OnTest(HWND hDlg)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_TIMERFILE, fname, MAX_PATH);
	if(fname[0] == 0) return;
	
	if(!m_bPlaying)
	{
		if(PlayFile(hDlg, fname, 0))
		{
			SendDlgItemMessage(hDlg, IDC_TIMERTEST, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)g_hIconStop);
			InvalidateRect(GetDlgItem(hDlg, IDC_TIMERTEST), NULL, FALSE);
			m_bPlaying = TRUE;
		}
	}
	else
	{
		StopFile();
		m_bPlaying = FALSE;
	}
}

/*------------------------------------------------
  "Show remaining time" is checked
--------------------------------------------------*/
void OnShowTime(HWND hDlg)
{
	int i;
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, IDC_SHOWTIME) && m_pTimer;
	
	for(i = IDC_SHOWWHOLE; i <= IDC_SHOWUSTRNUM; i++)
		EnableDlgItem(hDlg, i, b);
	
	OnUserStr(hDlg);
}

/*------------------------------------------------
  "User string" is checked
--------------------------------------------------*/
void OnUserStr(HWND hDlg)
{
	BOOL b;
	b = IsDlgButtonChecked(hDlg, IDC_SHOWUSTR) &&
		IsDlgButtonChecked(hDlg, IDC_SHOWTIME) &&
		m_pTimer;
	
	EnableDlgItem(hDlg, IDC_SHOWUSTRNUM, b);
}

/*------------------------------------------------
  get settings of a timer from the dialog
--------------------------------------------------*/
void GetTimerFromDlg(HWND hDlg, PTIMERSTRUCT pitem)
{
	if(!pitem) return;
	
	GetDlgItemText(hDlg, IDC_TIMERNAME, pitem->name, BUFSIZE_NAME);
	pitem->minute = UpDown_GetPos(hDlg, IDC_TIMERSPIN1);
	pitem->second = UpDown_GetPos(hDlg, IDC_TIMERSPIN2);
	GetDlgItemText(hDlg, IDC_TIMERFILE, pitem->fname, MAX_PATH);
	pitem->bRepeat = IsDlgButtonChecked(hDlg, IDC_TIMERREPEAT);
	pitem->bBlink = IsDlgButtonChecked(hDlg, IDC_TIMERBLINK);
	pitem->bDisp  = IsDlgButtonChecked(hDlg, IDC_SHOWTIME);
	if(IsDlgButtonChecked(hDlg, IDC_SHOWWHOLE))
		pitem->nDispType  = 0;
	else if(IsDlgButtonChecked(hDlg, IDC_SHOWUSTR))
		pitem->nDispType  = 2;
	else pitem->nDispType  = 1;
	pitem->nUserStr = GetDlgItemInt(hDlg, IDC_SHOWUSTRNUM, NULL, FALSE);
}

/*------------------------------------------------
  set settings of a timer to the dialog
--------------------------------------------------*/
void SetTimerToDlg(HWND hDlg, const TIMERSTRUCT *pitem)
{
	TIMERSTRUCT item;
	
	if(!pitem)
	{
		memset(&item, 0, sizeof(TIMERSTRUCT));
		item.nDispType = 1;
		pitem = &item;
	}
	
	SetDlgItemText(hDlg, IDC_TIMERNAME, pitem->name);
	UpDown_SetPos(hDlg, IDC_TIMERSPIN1, pitem->minute);
	UpDown_SetPos(hDlg, IDC_TIMERSPIN2, pitem->second);
	SetDlgItemText(hDlg, IDC_TIMERFILE, pitem->fname);
	CheckDlgButton(hDlg, IDC_TIMERREPEAT, pitem->bRepeat);
	CheckDlgButton(hDlg, IDC_TIMERBLINK, pitem->bBlink);
	CheckDlgButton(hDlg, IDC_SHOWTIME, pitem->bDisp);
	if(pitem->nDispType == 0)
		CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWWHOLE);
	else if(pitem->nDispType == 2)
		CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWUSTR);
	else
		CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWADD);
	
	SetDlgItemInt(hDlg,  IDC_SHOWUSTRNUM, pitem->nUserStr, FALSE);
	
	OnShowTime(hDlg);
}

/*------------------------------------------------
  enable/disable all dialog items
--------------------------------------------------*/
void EnableTimerDlgItems(HWND hDlg)
{
	HWND hwnd;
	BOOL b = (m_pTimer != NULL);
	
	hwnd = GetWindow(hDlg, GW_CHILD);
	while(hwnd)
	{
		if(GetDlgCtrlID(hwnd) == IDOK) break;
		
		if(GetDlgCtrlID(hwnd) != IDC_TIMERADD)
			EnableWindow(hwnd, b);
		
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

