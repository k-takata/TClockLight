/*-------------------------------------------------------------
  pagealarm.c : "Alarm" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

BOOL CALLBACK PageAlarmProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnName(HWND hDlg);
static void OnNameDropDown(HWND hDlg);
static void OnAdd(HWND hDlg);
static void OnDelete(HWND hDlg);
static void OnEnableAlarm(HWND hDlg);
static void OnDay(HWND hDlg);
static void OnBrowse(HWND hDlg);
static void OnFileChange(HWND hDlg);
static void OnTest(HWND hDlg, WORD id);
static void OnInterval(HWND hDlg);
static void EnableAlarmPageItems(HWND hDlg);
static void GetAlarmFromDlg(HWND hDlg, PALARMSTRUCT pAS);
static void SetAlarmToDlg(HWND hDlg, PALARMSTRUCT pAS);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

static PALARMSTRUCT m_pAlarm = NULL;
static int m_numAlarm = 0;
static int m_nCurrent = -1;
static BOOL m_bPlaying = FALSE;

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageAlarmProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
				case IDC_COMBOALARM:
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
				case IDC_ADDALARM:
					OnAdd(hDlg);
					break;
				case IDC_DELALARM:
					OnDelete(hDlg);
					break;
				case IDC_ENABLEALARM:
					OnEnableAlarm(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_HOURALARM:
				case IDC_MINUTEALARM:
				case IDC_SECONDALARM:
				case IDC_WDAYALARM:
				case IDC_ALARMINTERVALMIN:
					if(code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_SANSHOWDAY:
					OnDay(hDlg);
					break;
				case IDC_FILEALARM:
					if(code == EN_CHANGE)
					{
						OnFileChange(hDlg);
						SendPSChanged(hDlg);
					}
					break;
				case IDC_SANSHOALARM:
					OnBrowse(hDlg);
					OnFileChange(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_12HOURALARM:
				case IDC_REPEATALARM:
				case IDC_BLINKALARM:
				case IDC_ALARMBOOTEXEC:
					SendPSChanged(hDlg);
					break;
				case IDC_TESTALARM:
					OnTest(hDlg, id);
					break;
				case IDC_ALARMINTERVAL:
					OnInterval(hDlg);
					SendPSChanged(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Alarm"); break;
			}
			return TRUE;
		case WM_DESTROY:
			OnDestroy(hDlg);
			break;
		// playing sound ended
		case MM_MCINOTIFY:
		case MM_WOM_DONE:
			if(message == MM_MCINOTIFY)
				OnMCINotify(hDlg, wParam, (LONG)lParam);
			else
				StopFile();
			m_bPlaying = FALSE;
			SendDlgItemMessage(hDlg, IDC_TESTALARM,
				BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
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
	int i;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Alarm", g_hfontDialog);
	
	m_numAlarm = GetMyRegLong("", "AlarmNum", 0);
	if(m_numAlarm < 1) m_numAlarm = 0;
	
	if(m_numAlarm > 0)
	{
		m_pAlarm = malloc(sizeof(ALARMSTRUCT) * m_numAlarm);
		LoadAlarm(m_pAlarm, m_numAlarm); // common/alarmstruct.c
	}
	else // no alarm
	{
		PALARMSTRUCT pAS;
		
		m_numAlarm = 1;
		m_pAlarm = malloc(sizeof(ALARMSTRUCT));
		
		pAS = m_pAlarm;
		memset(pAS, 0, sizeof(ALARMSTRUCT));
		strcpy(pAS->name, "Alarm1");
		pAS->bEnable = TRUE;
	}
	
	for(i = 0; i < m_numAlarm; i++)
		 CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)(m_pAlarm+i)->name);
	
	CBSetCurSel(hDlg, IDC_COMBOALARM, 0);
	OnName(hDlg);
	
	SendDlgItemMessage(hDlg, IDC_TESTALARM, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconPlay);
	
	m_bPlaying = FALSE;
	
	m_bInit = TRUE;
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	int i, nOldAlarm;
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	OnNameDropDown(hDlg);
	
	if(m_pAlarm && (0 <= m_nCurrent && m_nCurrent < m_numAlarm))
		GetAlarmFromDlg(hDlg, (m_pAlarm + m_nCurrent));
	
	nOldAlarm = GetMyRegLong("", "AlarmNum", 0);
	if(nOldAlarm < 1) nOldAlarm = 0;
	
	SetMyRegLong("", "AlarmNum", m_numAlarm);
	
	if(m_pAlarm)
		SaveAlarm(m_pAlarm, m_numAlarm); // common/alarmstruct.c
	
	for(i = m_numAlarm; i < nOldAlarm; i++)
	{
		char subkey[20];
		wsprintf(subkey, "Alarm%d", i + 1);
		DelMyRegKey(subkey);
	}
}

/*------------------------------------------------
  free memories associated with combo box.
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	if(m_bPlaying) StopFile(); m_bPlaying = FALSE;
	
	if(m_pAlarm) free(m_pAlarm);
}

/*------------------------------------------------
   an alarm name is selected by combobox
--------------------------------------------------*/
void OnName(HWND hDlg)
{
	int index;
	
	if(m_pAlarm && 0 <= m_nCurrent && m_nCurrent < m_numAlarm)
		GetAlarmFromDlg(hDlg, m_pAlarm + m_nCurrent);
	
	index = CBGetCurSel(hDlg, IDC_COMBOALARM);
	
	if(m_pAlarm && 0 <= index && index < m_numAlarm)
	{
		SetAlarmToDlg(hDlg, m_pAlarm + index);
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
	
	if(!m_pAlarm || !(0 <= m_nCurrent && m_nCurrent < m_numAlarm)) return;
	
	GetDlgItemText(hDlg, IDC_COMBOALARM, name, BUFSIZE_NAME);
	
	if(strcmp(name, m_pAlarm[m_nCurrent].name) != 0)
	{
		CBDeleteString(hDlg, IDC_COMBOALARM, m_nCurrent);
		CBInsertString(hDlg, IDC_COMBOALARM, m_nCurrent, name);
		strcpy(m_pAlarm[m_nCurrent].name, name);
	}
}

/*------------------------------------------------
  add an alarm
--------------------------------------------------*/
void OnAdd(HWND hDlg)
{
	PALARMSTRUCT pASNew, pAS;
	int i;
	
	OnNameDropDown(hDlg);
	
	if(m_pAlarm && (0 <= m_nCurrent && m_nCurrent < m_numAlarm))
		GetAlarmFromDlg(hDlg, (m_pAlarm + m_nCurrent));
	
	pASNew = malloc(sizeof(ALARMSTRUCT)*(m_numAlarm+1));
	for(i = 0; i < m_numAlarm && m_pAlarm; i++)
		memcpy(pASNew + i, m_pAlarm + i, sizeof(ALARMSTRUCT));
	
	pAS = pASNew + i;
	memset(pAS, 0, sizeof(ALARMSTRUCT));
	wsprintf(pAS->name, "Alarm%d", i+1);
	pAS->bEnable = TRUE;
	
	CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)pAS->name);
	CBSetCurSel(hDlg, IDC_COMBOALARM, i);
	m_nCurrent = i;
	
	m_numAlarm++;
	if(m_pAlarm) free(m_pAlarm);
	m_pAlarm = pASNew;
	
	if(m_numAlarm == 1)
		EnableAlarmPageItems(hDlg);
	
	SetAlarmToDlg(hDlg, pAS);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  delete an alarm
--------------------------------------------------*/
void OnDelete(HWND hDlg)
{
	PALARMSTRUCT pASNew;
	int i, j;
	
	if(!m_pAlarm || m_numAlarm < 1) return;
	if(!(0 <= m_nCurrent && m_nCurrent < m_numAlarm)) return;
	
	if(m_numAlarm > 1)
	{
		pASNew = malloc(sizeof(ALARMSTRUCT)*(m_numAlarm-1));
		for(i = 0, j = 0; i < m_numAlarm; i++)
		{
			if(i != m_nCurrent)
			{
				memcpy(pASNew + j, m_pAlarm + i, sizeof(ALARMSTRUCT));
				j++;
			}
		}
		
		CBDeleteString(hDlg, IDC_COMBOALARM, m_nCurrent);
		
		if(m_nCurrent == m_numAlarm - 1) m_nCurrent--;
		CBSetCurSel(hDlg, IDC_COMBOALARM, m_nCurrent);
		SetAlarmToDlg(hDlg, (pASNew + m_nCurrent));
		
		m_numAlarm--;
		free(m_pAlarm);
		m_pAlarm = pASNew;
	}
	else
	{
		free(m_pAlarm); m_pAlarm = NULL;
		m_numAlarm = 0;
		m_nCurrent = -1;
		
		CBDeleteString(hDlg, IDC_COMBOALARM, 0);
		EnableAlarmPageItems(hDlg);
		SetAlarmToDlg(hDlg, NULL);
	}
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  "Active" checkbox
--------------------------------------------------*/
void OnEnableAlarm(HWND hDlg)
{
	HWND hwnd = GetDlgItem(hDlg, IDC_ENABLEALARM);
	BOOL b = IsDlgButtonChecked(hDlg, IDC_ENABLEALARM);
	
	hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	while(hwnd)
	{
		EnableWindow(hwnd, b);
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}
	if(b) OnFileChange(hDlg);
}

/*------------------------------------------------
  "Day..."
--------------------------------------------------*/
void OnDay(HWND hDlg)
{
	PALARMSTRUCT pAS;
	
	if(!m_pAlarm || !(0 <= m_nCurrent && m_nCurrent < m_numAlarm)) return;
	pAS = m_pAlarm + m_nCurrent;
	GetAlarmFromDlg(hDlg, pAS);
	SetAlarmTime(pAS);
	
	// open dialog - alarmday.c
	if(SetAlarmDay(hDlg, pAS) == IDOK)
	{
		SetDlgItemText(hDlg, IDC_WDAYALARM, pAS->strWDays);
		SendPSChanged(hDlg);
	}
}

/*------------------------------------------------
  browse sound file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_FILEALARM, deffile, MAX_PATH);
	
	// common/soundselect.c
	if(!BrowseSoundFile(g_hInst, hDlg, deffile, fname))
		return;
	
	SetDlgItemText(hDlg, IDC_FILEALARM, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
   file name changed - enable/disable controls
--------------------------------------------------*/
void OnFileChange(HWND hDlg)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_FILEALARM, fname, MAX_PATH);
	
	EnableDlgItem(hDlg, IDC_REPEATALARM, IsSoundFile(fname));
}

/*------------------------------------------------
  test sound
--------------------------------------------------*/
void OnTest(HWND hDlg, WORD id)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_FILEALARM, fname, MAX_PATH);
	if(fname[0] == 0) return;
	
	if((HICON)SendDlgItemMessage(hDlg, id, BM_GETIMAGE, IMAGE_ICON, 0)
		== g_hIconPlay)
	{
		if(PlayFile(hDlg, fname, 0))
		{
			SendDlgItemMessage(hDlg, id, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)g_hIconStop);
			InvalidateRect(GetDlgItem(hDlg, id), NULL, FALSE);
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
  "At regular intervals" checkbox
--------------------------------------------------*/
void OnInterval(HWND hDlg)
{
	HWND hwnd = GetDlgItem(hDlg, IDC_ALARMINTERVAL);
	BOOL b = IsDlgButtonChecked(hDlg, IDC_ALARMINTERVAL);
	
	if(!IsDlgButtonChecked(hDlg, IDC_ENABLEALARM)) b = FALSE;
	
	hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	EnableWindow(hwnd, b);
	hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	EnableWindow(hwnd, b);
}

/*------------------------------------------------
  enable/disable all dialog items
--------------------------------------------------*/
void EnableAlarmPageItems(HWND hDlg)
{
	HWND hwnd;
	BOOL b = (m_pAlarm != NULL);
	
	hwnd = GetWindow(hDlg, GW_CHILD);
	while(hwnd)
	{
		if(GetDlgCtrlID(hwnd) != IDC_ADDALARM)
			EnableWindow(hwnd, b);
		
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

/*------------------------------------------------
  get settings of an alarm from the page
--------------------------------------------------*/
void GetAlarmFromDlg(HWND hDlg, PALARMSTRUCT pAS)
{
	GetDlgItemText(hDlg, IDC_COMBOALARM, pAS->name, BUFSIZE_NAME);
	pAS->bEnable = IsDlgButtonChecked(hDlg, IDC_ENABLEALARM);
	
	GetDlgItemText(hDlg, IDC_HOURALARM, pAS->strHours, 80);
	GetDlgItemText(hDlg, IDC_MINUTEALARM, pAS->strMinutes, 80);
	pAS->second = GetDlgItemInt(hDlg, IDC_SECONDALARM, NULL, FALSE);
	GetDlgItemText(hDlg, IDC_WDAYALARM, pAS->strWDays, 80);
	
	GetDlgItemText(hDlg, IDC_FILEALARM, pAS->fname, MAX_PATH);
	pAS->bHour12 = IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
	pAS->bRepeat = IsDlgButtonChecked(hDlg, IDC_REPEATALARM);
	pAS->bBlink = IsDlgButtonChecked(hDlg, IDC_BLINKALARM);
	pAS->bBootExec = IsDlgButtonChecked(hDlg, IDC_ALARMBOOTEXEC);
	pAS->bInterval = IsDlgButtonChecked(hDlg, IDC_ALARMINTERVAL);
	pAS->nInterval = GetDlgItemInt(hDlg, IDC_ALARMINTERVALMIN, NULL, FALSE);
}

/*------------------------------------------------
  set settings of an alarm to the page
--------------------------------------------------*/
void SetAlarmToDlg(HWND hDlg, PALARMSTRUCT pAS)
{
	SetDlgItemText(hDlg, IDC_COMBOALARM, pAS ? pAS->name : "");
	CheckDlgButton(hDlg, IDC_ENABLEALARM, pAS ? pAS->bEnable : FALSE);
	
	SetDlgItemText(hDlg, IDC_HOURALARM, pAS ? pAS->strHours : "");
	SetDlgItemText(hDlg, IDC_MINUTEALARM, pAS ? pAS->strMinutes : "");
	if(pAS && pAS->second)
		SetDlgItemInt(hDlg, IDC_SECONDALARM, pAS->second, FALSE);
	else SetDlgItemText(hDlg, IDC_SECONDALARM, "");
	SetDlgItemText(hDlg, IDC_WDAYALARM, pAS ? pAS->strWDays : "");
	SetDlgItemText(hDlg, IDC_FILEALARM, pAS ? pAS->fname : "");
	CheckDlgButton(hDlg, IDC_12HOURALARM, pAS ? pAS->bHour12 : FALSE);
	CheckDlgButton(hDlg, IDC_REPEATALARM, pAS ? pAS->bRepeat : FALSE);
	CheckDlgButton(hDlg, IDC_BLINKALARM,  pAS ? pAS->bBlink  : FALSE);
	CheckDlgButton(hDlg, IDC_ALARMBOOTEXEC, pAS ? pAS->bBootExec : FALSE);
	CheckDlgButton(hDlg, IDC_ALARMINTERVAL, pAS ? pAS->bInterval : FALSE);
	SetDlgItemInt(hDlg, IDC_ALARMINTERVALMIN, pAS ? pAS->nInterval : 0, FALSE);
	
	OnEnableAlarm(hDlg);
	OnInterval(hDlg);
}
