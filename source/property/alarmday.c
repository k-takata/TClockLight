/*-------------------------------------------------------------
  alarmday.c : "Day of Week" of alarm
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Statics */

static INT_PTR CALLBACK AlarmDayProc(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg);
static void OnOK(HWND hDlg);
static void OnEveryDay(HWND hDlg);

static PALARMSTRUCT m_pAS;

/*------------------------------------------------
  show dialog box
--------------------------------------------------*/
int SetAlarmDay(HWND hDlg, PALARMSTRUCT pAS)
{
	m_pAS = pAS;
	return (int)DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ALARMDAY),
		hDlg, AlarmDayProc);
}

/*------------------------------------------------
  dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK AlarmDayProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id; //, code;
			id = LOWORD(wParam); // code = HIWORD(wParam);
			switch(id)
			{
				case IDC_ALARMDAY0:
					OnEveryDay(hDlg); break;
				case IDOK: OnOK(hDlg);
				case IDCANCEL: EndDialog(hDlg, id); break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	int i;
	BOOL bAll;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "AlarmDay", g_hfontDialog);
	
	bAll = TRUE;
	for(i = 0; i < 7; i++)
	{
		if(m_pAS->wdays[i])
			CheckDlgButton(hDlg, IDC_ALARMDAY1+i, TRUE);
		else
			bAll = FALSE;
	}
	
	if(bAll)
	{
		CheckDlgButton(hDlg, IDC_ALARMDAY0, TRUE);
		OnEveryDay(hDlg);
	}
}

/*------------------------------------------------
  retreive setting
--------------------------------------------------*/
void OnOK(HWND hDlg)
{
	char s[80];
	BOOL bAll;
	int i;
	
	s[0] = 0;
	bAll = TRUE;
	for(i = 0; i < 7; i++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_ALARMDAY1 + i))
			wsprintf(s + strlen(s), "%d,", i);
		else bAll = FALSE;
	}
	
	if(bAll)
		m_pAS->strWDays[0] = 0;
	else
	{
		s[strlen(s) - 1] = 0;
		strcpy(m_pAS->strWDays, s);
	}
}

/*------------------------------------------------
   Every day
--------------------------------------------------*/
void OnEveryDay(HWND hDlg)
{
	int i;
	
	if(IsDlgButtonChecked(hDlg, IDC_ALARMDAY0))
	{
		for(i = 0; i < 7; i++)
		{
			CheckDlgButton(hDlg, IDC_ALARMDAY1 + i, TRUE);
			EnableDlgItem(hDlg, IDC_ALARMDAY1+i, FALSE);
		}
	}
	else
	{
		for(i = 0; i < 7; i++)
		{
			CheckDlgButton(hDlg, IDC_ALARMDAY1 + i, FALSE);
			EnableDlgItem(hDlg, IDC_ALARMDAY1+i, TRUE);
		}
	}
}
