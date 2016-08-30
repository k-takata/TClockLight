/*-------------------------------------------------------------
  pagealarm.c : "Alarm" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

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
static void OnResumeExec(HWND hDlg);
static void EnableAlarmPageItems(HWND hDlg);
static void GetAlarmFromDlg(HWND hDlg, PALARMSTRUCT pitem);
static void SetAlarmToDlg(HWND hDlg, const ALARMSTRUCT *pitem);
static PALARMSTRUCT CBGetAlarmStruct(HWND hDlg, int idCombo, int index);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

static PALARMSTRUCT m_pListAlarm = NULL;
static int m_nCurrent = -1;
static BOOL m_bPlaying = FALSE;

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageAlarmProc(HWND hDlg, UINT message,
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
				case IDC_ALARMRESUMEDELAY:
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
				case IDC_ALARMRESUMEEXEC:
					OnResumeExec(hDlg);
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
	PALARMSTRUCT pitem;
	int count;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Alarm", g_hfontDialog);
	
	count = GetMyRegLong("", "AlarmNum", 0);
	
	if(count > 0)
	{
		m_pListAlarm = LoadAlarm(); // common/alarmstruct.c
	}
	else // alarm count is zero
	{
		ALARMSTRUCT item;
		memset(&item, 0, sizeof(ALARMSTRUCT));
		strcpy(item.name, "Alarm1");
		item.bEnable = TRUE;
		//common/list.c
		m_pListAlarm = copy_listitem(NULL, &item, sizeof(item));
	}
	
	pitem = m_pListAlarm;
	while(pitem)
	{
		CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)pitem->name);
		pitem = pitem->next;
	}
	
	m_nCurrent = -1;
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
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	OnNameDropDown(hDlg);
	
	GetAlarmFromDlg(hDlg, get_listitem(m_pListAlarm, m_nCurrent));
	
	SaveAlarm(m_pListAlarm); // common/alarmstruct.c
}

/*------------------------------------------------
  free memories associated with combo box.
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	if(m_bPlaying) StopFile(); m_bPlaying = FALSE;
	
	clear_list(m_pListAlarm); // common/list.c
	m_pListAlarm = NULL;
}

/*------------------------------------------------
   an alarm name is selected by combobox
--------------------------------------------------*/
void OnName(HWND hDlg)
{
	int index;
	
	index = CBGetCurSel(hDlg, IDC_COMBOALARM);
	if(index < 0) return;
	
	GetAlarmFromDlg(hDlg, get_listitem(m_pListAlarm, m_nCurrent));
	
	SetAlarmToDlg(hDlg, get_listitem(m_pListAlarm, index));
	m_nCurrent = index;
}

/*------------------------------------------------
   combo box is about to be visible
   set edited text to combo box
--------------------------------------------------*/
void OnNameDropDown(HWND hDlg)
{
	char name[BUFSIZE_NAME];
	PALARMSTRUCT pitem;
	
	pitem = get_listitem(m_pListAlarm, m_nCurrent);
	if(pitem == NULL) return;
	
	GetDlgItemText(hDlg, IDC_COMBOALARM, name, BUFSIZE_NAME);
	
	if(strcmp(name, pitem->name) != 0)
	{
		strcpy(pitem->name, name);
		CBDeleteString(hDlg, IDC_COMBOALARM, m_nCurrent);
		CBInsertString(hDlg, IDC_COMBOALARM, m_nCurrent, (LPARAM)name);
	}
}

/*------------------------------------------------
  add an alarm
--------------------------------------------------*/
void OnAdd(HWND hDlg)
{
	PALARMSTRUCT pitem;
	int count, index;
	
	count = CBGetCount(hDlg, IDC_COMBOALARM);
	if(count < 0) return;
	
	OnNameDropDown(hDlg);
	
	GetAlarmFromDlg(hDlg, get_listitem(m_pListAlarm, m_nCurrent));
	
	pitem = malloc(sizeof(ALARMSTRUCT));
	memset(pitem, 0, sizeof(ALARMSTRUCT));
	wsprintf(pitem->name, "Alarm%d", count + 1);
	pitem->bEnable = TRUE;
	// common/list.c
	m_pListAlarm = add_listitem(m_pListAlarm, pitem);
	
	index = CBAddString(hDlg, IDC_COMBOALARM, (LPARAM)pitem->name);
	CBSetCurSel(hDlg, IDC_COMBOALARM, index);
	
	if(count == 0)
		EnableAlarmPageItems(hDlg);
	
	SetAlarmToDlg(hDlg, pitem);
	m_nCurrent = index;
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
  delete an alarm
--------------------------------------------------*/
void OnDelete(HWND hDlg)
{
	int count, index;
	PALARMSTRUCT pitem;
	
	count = CBGetCount(hDlg, IDC_COMBOALARM);
	if(count < 1) return;
	
	index = CBGetCurSel(hDlg, IDC_COMBOALARM);
	if(index < 0) return;
	
	pitem = get_listitem(m_pListAlarm, index);
	if(pitem == NULL) return;
	// common/list.c
	m_pListAlarm = del_listitem(m_pListAlarm, pitem);
	
	CBDeleteString(hDlg, IDC_COMBOALARM, index);
	
	if(count > 1)
	{
		if(index == count - 1) index--;
		CBSetCurSel(hDlg, IDC_COMBOALARM, index);
	}
	else
	{
		index = -1;
		EnableAlarmPageItems(hDlg);
	}
	
	SetAlarmToDlg(hDlg, get_listitem(m_pListAlarm, index));
	m_nCurrent = index;
	
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
	PALARMSTRUCT pitem;
	
	pitem = get_listitem(m_pListAlarm, m_nCurrent);
	if(pitem == NULL) return;
	
	GetAlarmFromDlg(hDlg, pitem);
	SetAlarmTime(pitem);
	
	// open dialog - alarmday.c
	if(SetAlarmDay(hDlg, pitem) == IDOK)
	{
		SetDlgItemText(hDlg, IDC_WDAYALARM, pitem->strWDays);
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
  "Execute when resumed" checkbox
--------------------------------------------------*/
void OnResumeExec(HWND hDlg)
{
	HWND hwnd = GetDlgItem(hDlg, IDC_ALARMRESUMEEXEC);
	BOOL b = IsDlgButtonChecked(hDlg, IDC_ALARMRESUMEEXEC);
	
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
	BOOL b = (m_pListAlarm != NULL);
	
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
void GetAlarmFromDlg(HWND hDlg, PALARMSTRUCT pitem)
{
	if(!pitem) return;
	
	GetDlgItemText(hDlg, IDC_COMBOALARM, pitem->name, BUFSIZE_NAME);
	pitem->bEnable = IsDlgButtonChecked(hDlg, IDC_ENABLEALARM);
	
	GetDlgItemText(hDlg, IDC_HOURALARM, pitem->strHours, 80);
	GetDlgItemText(hDlg, IDC_MINUTEALARM, pitem->strMinutes, 80);
	pitem->second = GetDlgItemInt(hDlg, IDC_SECONDALARM, NULL, FALSE);
	GetDlgItemText(hDlg, IDC_WDAYALARM, pitem->strWDays, 80);
	
	GetDlgItemText(hDlg, IDC_FILEALARM, pitem->fname, MAX_PATH);
	pitem->bHour12 = IsDlgButtonChecked(hDlg, IDC_12HOURALARM);
	pitem->bRepeat = IsDlgButtonChecked(hDlg, IDC_REPEATALARM);
	pitem->bBlink = IsDlgButtonChecked(hDlg, IDC_BLINKALARM);
	pitem->bBootExec = IsDlgButtonChecked(hDlg, IDC_ALARMBOOTEXEC);
	pitem->bInterval = IsDlgButtonChecked(hDlg, IDC_ALARMINTERVAL);
	pitem->nInterval = GetDlgItemInt(hDlg, IDC_ALARMINTERVALMIN, NULL, FALSE);
	pitem->bResumeExec = IsDlgButtonChecked(hDlg, IDC_ALARMRESUMEEXEC);
	pitem->nResumeDelay = GetDlgItemInt(hDlg, IDC_ALARMRESUMEDELAY, NULL, FALSE);
}

/*------------------------------------------------
  set settings of an alarm to the page
--------------------------------------------------*/
void SetAlarmToDlg(HWND hDlg, const ALARMSTRUCT *pitem)
{
	ALARMSTRUCT item;
	
	if(!pitem)
	{
		memset(&item, 0, sizeof(item));
		pitem = &item;
	}
	
	SetDlgItemText(hDlg, IDC_COMBOALARM, pitem->name);
	CheckDlgButton(hDlg, IDC_ENABLEALARM, pitem->bEnable);
	
	SetDlgItemText(hDlg, IDC_HOURALARM, pitem->strHours);
	SetDlgItemText(hDlg, IDC_MINUTEALARM, pitem->strMinutes);
	if(pitem->second)
		SetDlgItemInt(hDlg, IDC_SECONDALARM, pitem->second, FALSE);
	else SetDlgItemText(hDlg, IDC_SECONDALARM, "");
	SetDlgItemText(hDlg, IDC_WDAYALARM, pitem->strWDays);
	SetDlgItemText(hDlg, IDC_FILEALARM, pitem->fname);
	CheckDlgButton(hDlg, IDC_12HOURALARM, pitem->bHour12);
	CheckDlgButton(hDlg, IDC_REPEATALARM, pitem->bRepeat);
	CheckDlgButton(hDlg, IDC_BLINKALARM,  pitem->bBlink);
	CheckDlgButton(hDlg, IDC_ALARMBOOTEXEC, pitem->bBootExec);
	CheckDlgButton(hDlg, IDC_ALARMINTERVAL, pitem->bInterval);
	SetDlgItemInt(hDlg, IDC_ALARMINTERVALMIN, pitem->nInterval, FALSE);
	CheckDlgButton(hDlg, IDC_ALARMRESUMEEXEC, pitem->bResumeExec);
	SetDlgItemInt(hDlg, IDC_ALARMRESUMEDELAY, pitem->nResumeDelay, FALSE);
	
	OnEnableAlarm(hDlg);
	OnInterval(hDlg);
	OnResumeExec(hDlg);
}
