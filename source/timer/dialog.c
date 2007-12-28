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

INT_PTR CALLBACK DlgProcTimer(HWND, UINT, WPARAM, LPARAM);
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
static void GetTimerFromDlg(HWND hDlg, PTIMERSTRUCT pTS);
static void SetTimerToDlg(HWND hDlg, const PTIMERSTRUCT pTS);
static void EnableTimerDlgItems(HWND hDlg);

static PTIMERSTRUCT m_pTimer = NULL;
static int m_numTimer = 0;
static int m_nCurrent = -1;
static BOOL m_bPlaying = FALSE;  // sound is plaing

/*-------------------------------------------------------
  TIMERM_SHOWDLG message
---------------------------------------------------------*/
void OnShowDialog(HWND hwnd)
{
	if(g_hDlg && IsWindow(g_hDlg)) ;
	else
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
	int i;
	
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
	
	m_numTimer = GetMyRegLong(NULL, "TimerNum", 0);
	if(m_numTimer > 0)
	{
		m_pTimer = (PTIMERSTRUCT)malloc(sizeof(TIMERSTRUCT) * m_numTimer);
		for(i = 0; i < m_numTimer; i++)
		{
			PTIMERSTRUCT pTS = m_pTimer + i;
			
			wsprintf(section, "Timer%d", i + 1);
			GetMyRegStr(section, "Name", pTS->name, BUFSIZE_NAME, section);
			pTS->minute = GetMyRegLong(section, "Minute", 3);
			pTS->second = GetMyRegLong(section, "Second", 0);
			GetMyRegStr(section, "File", pTS->fname, MAX_PATH, "");
			pTS->bRepeat = GetMyRegLong(section, "Repeat", FALSE);
			pTS->bBlink = GetMyRegLong(section, "Blink", FALSE);
			pTS->bDisp = GetMyRegLong(section, "Disp", FALSE);
			pTS->nDispType = GetMyRegLong(section, "DispType", 1);
			pTS->nUserStr = GetMyRegLong(section, "UserStr", 0);
		}
	}
	else
	{
		m_numTimer = 1;
		m_pTimer = (PTIMERSTRUCT)malloc(sizeof(TIMERSTRUCT));
		memset(m_pTimer, 0, sizeof(TIMERSTRUCT));
		strcpy(m_pTimer->name, "Timer1");
		m_pTimer->minute = 3;
		m_pTimer->nDispType = 1;
	}
	
	for(i = 0; i < m_numTimer; i++)
		CBAddString(hDlg, IDC_TIMERNAME, (LPARAM)(m_pTimer + i)->name);
	
	CBSetCurSel(hDlg, IDC_TIMERNAME, 0);
	
	OnName(hDlg);
}

/*------------------------------------------------
   clear up
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	g_hDlg = NULL;
	
	if(m_pTimer) free(m_pTimer);
	m_pTimer = NULL;
	m_numTimer = 0;
	m_nCurrent = -1;
	
	StopFile();
	m_bPlaying = FALSE;
}

/*-------------------------------------------
  "Start" button
---------------------------------------------*/
void OnOK(HWND hDlg)
{
	char section[20];
	int i, nOldTimer;
	
	/* save settings */
	if(m_pTimer && (0 <= m_nCurrent && m_nCurrent < m_numTimer))
		GetTimerFromDlg(hDlg, (m_pTimer + m_nCurrent));
	
	nOldTimer = GetMyRegLong(NULL, "TimerNum", 0);
	if(nOldTimer < 1) nOldTimer = 0;
	
	SetMyRegLong(NULL, "TimerNum", m_numTimer);
	
	for(i = 0; i < m_numTimer && m_pTimer; i++)
	{
		PTIMERSTRUCT pTS = m_pTimer + i;
		
		wsprintf(section, "Timer%d", i + 1);
		SetMyRegStr(section, "Name", pTS->name);
		SetMyRegLong(section, "Minute", pTS->minute);
		SetMyRegLong(section, "Second", pTS->second);
		SetMyRegStr(section, "File", pTS->fname);
		SetMyRegLong(section, "Repeat", pTS->bRepeat);
		SetMyRegLong(section, "Blink", pTS->bBlink);
		SetMyRegLong(section, "Disp", pTS->bDisp);
		SetMyRegLong(section, "DispType", pTS->nDispType);
		SetMyRegLong(section, "UserStr", pTS->nUserStr);
	}
	
	for(i = m_numTimer; i < nOldTimer; i++)
	{
		wsprintf(section, "Timer%d", i + 1);
		DelMyRegKey(section);
	}
	
	/* start a timer */
	if(m_pTimer && (0 <= m_nCurrent && m_nCurrent < m_numTimer))
		TimerStart(m_pTimer + m_nCurrent);
	
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
	
	if(0 <= m_nCurrent && m_nCurrent < m_numTimer)
		GetTimerFromDlg(hDlg, m_pTimer + m_nCurrent);
	
	index = CBGetCurSel(hDlg, IDC_TIMERNAME);
	if(0 <= index && index < m_numTimer)
	{
		SetTimerToDlg(hDlg, m_pTimer + index);
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
	
	if(!m_pTimer ||
		!(0 <= m_nCurrent && m_nCurrent < m_numTimer)) return;
	
	GetDlgItemText(hDlg, IDC_TIMERNAME, name, BUFSIZE_NAME);
	
	if(strcmp(name, m_pTimer[m_nCurrent].name) != 0)
	{
		CBDeleteString(hDlg, IDC_TIMERNAME, m_nCurrent);
		CBInsertString(hDlg, IDC_TIMERNAME, m_nCurrent, name);
		strcpy(m_pTimer[m_nCurrent].name, name);
	}
}

/*------------------------------------------------
  "Add" button
--------------------------------------------------*/
void OnAdd(HWND hDlg)
{
	PTIMERSTRUCT pTSNew;
	TIMERSTRUCT ts;
	
	OnNameDropDown(hDlg);
	
	memset(&ts, 0, sizeof(TIMERSTRUCT));
	wsprintf(ts.name, "Timer%d", m_numTimer+1);
	ts.minute = 3;
	ts.nDispType = 1;
	
	pTSNew = AddTimerStruct(m_pTimer, m_numTimer, &ts);
	
	m_nCurrent = m_numTimer;
	CBAddString(hDlg, IDC_TIMERNAME, (LPARAM)ts.name);
	CBSetCurSel(hDlg, IDC_TIMERNAME, m_numTimer);
	SetTimerToDlg(hDlg, &ts);
	
	m_numTimer++;
	if(m_pTimer) free(m_pTimer);
	m_pTimer = pTSNew;
	
	if(m_numTimer == 1)
		EnableTimerDlgItems(hDlg);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  delete an alarm
--------------------------------------------------*/
void OnDelete(HWND hDlg)
{
	PTIMERSTRUCT pTSNew;
	
	if(!m_pTimer || m_numTimer < 1) return;
	
	if(!(0 <= m_nCurrent && m_nCurrent < m_numTimer)) return;
	
	if(m_numTimer > 1)
	{
		pTSNew = DelTimerStruct(m_pTimer, m_numTimer, m_nCurrent);
		
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		CBDeleteString(hDlg, IDC_TIMERNAME, m_nCurrent);
		
		if(m_nCurrent == m_numTimer - 1) m_nCurrent--;
		CBSetCurSel(hDlg, IDC_TIMERNAME, m_nCurrent);
		SetTimerToDlg(hDlg, (pTSNew + m_nCurrent));
		
		m_numTimer--;
		if(m_pTimer) free(m_pTimer);
		m_pTimer = pTSNew;
	}
	else
	{
		free(m_pTimer); m_pTimer = NULL;
		m_numTimer = 0;
		m_nCurrent = -1;
		
		CBDeleteString(hDlg, IDC_TIMERNAME, 0);
		SetTimerToDlg(hDlg, NULL);
		
		EnableTimerDlgItems(hDlg);
	}
}

/*------------------------------------------------
  "..." button - select file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_TIMERFILE, deffile, MAX_PATH);
	
	// ../common/soundselect.c
	if(!BrowseSoundFile(g_hInst, hDlg, deffile, fname))
		return;
	
	SetDlgItemText(hDlg, IDC_TIMERFILE, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  test sound
--------------------------------------------------*/
void OnTest(HWND hDlg)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_TIMERFILE, fname, MAX_PATH);
	if(fname[0] == 0) return;
	
	if((HICON)SendDlgItemMessage(hDlg, IDC_TIMERTEST,
		BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay)
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
	
	b = IsDlgButtonChecked(hDlg, IDC_SHOWTIME);
	if(!m_pTimer) b = FALSE;
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
	b = IsDlgButtonChecked(hDlg, IDC_SHOWUSTR);
	if(!IsDlgButtonChecked(hDlg, IDC_SHOWTIME)) b = FALSE;
	if(!m_pTimer) b = FALSE;
	
	EnableDlgItem(hDlg, IDC_SHOWUSTRNUM, b);
}

/*------------------------------------------------
  get settings of a timer from the dialog
--------------------------------------------------*/
void GetTimerFromDlg(HWND hDlg, PTIMERSTRUCT pTS)
{
	if(!pTS) return;
	
	GetDlgItemText(hDlg, IDC_TIMERNAME, pTS->name, BUFSIZE_NAME);
	pTS->minute = GetDlgItemInt(hDlg, IDC_TIMERMINUTE, NULL, FALSE);
	pTS->second = GetDlgItemInt(hDlg, IDC_TIMERSECOND, NULL, FALSE);
	GetDlgItemText(hDlg, IDC_TIMERFILE, pTS->fname, MAX_PATH);
	pTS->bRepeat = IsDlgButtonChecked(hDlg, IDC_TIMERREPEAT);
	pTS->bBlink = IsDlgButtonChecked(hDlg, IDC_TIMERBLINK);
	pTS->bDisp  = IsDlgButtonChecked(hDlg, IDC_SHOWTIME);
	if(IsDlgButtonChecked(hDlg, IDC_SHOWWHOLE))
		pTS->nDispType  = 0;
	else if(IsDlgButtonChecked(hDlg, IDC_SHOWUSTR))
		pTS->nDispType  = 2;
	else pTS->nDispType  = 1;
	pTS->nUserStr = GetDlgItemInt(hDlg, IDC_SHOWUSTRNUM, NULL, FALSE);
}

/*------------------------------------------------
  set settings of a timer to the dialog
--------------------------------------------------*/
void SetTimerToDlg(HWND hDlg, const PTIMERSTRUCT pTS)
{
	SetDlgItemText(hDlg, IDC_TIMERNAME, pTS ? pTS->name : "");
	SetDlgItemInt(hDlg,  IDC_TIMERMINUTE, pTS ? pTS->minute : 0, FALSE);
	SetDlgItemInt(hDlg,  IDC_TIMERSECOND, pTS ? pTS->second : 0, FALSE);
	SetDlgItemText(hDlg, IDC_TIMERFILE, pTS ? pTS->fname : "");
	CheckDlgButton(hDlg, IDC_TIMERREPEAT, pTS ? pTS->bRepeat : FALSE);
	CheckDlgButton(hDlg, IDC_TIMERBLINK, pTS ? pTS->bBlink : FALSE);
	CheckDlgButton(hDlg, IDC_SHOWTIME, pTS ? pTS->bDisp : FALSE);
	if(pTS)
	{
		if(pTS->nDispType == 0)
			CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWWHOLE);
		else if(pTS->nDispType == 2)
			CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWUSTR);
		else
			CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWADD);
	}
	else CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR, IDC_SHOWADD);
	
	SetDlgItemInt(hDlg,  IDC_SHOWUSTRNUM, pTS ? pTS->nUserStr : 1, FALSE);
	
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

