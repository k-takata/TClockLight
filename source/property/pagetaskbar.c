/*-------------------------------------------------------------
  pagetaskbar.c : "Taskbar" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

INT_PTR CALLBACK PageTaskbarProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnTaskSwitchFlat(HWND hDlg);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
INT_PTR CALLBACK PageTaskbarProc(HWND hDlg, UINT message,
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
			case IDC_TASKSWITCHFLAT:
				OnTaskSwitchFlat(hDlg);
				SendPSChanged(hDlg);
				break;
			case IDC_TASKSWITCH_SEPARAT:
			case IDC_TBBORDER:
//			case IDC_TBBORDEREX:
			case IDC_FLATTRAY:
			case IDC_RBHIDE:
			case IDC_TASKSWITCHICON:
				SendPSChanged(hDlg);
				break;
			case IDC_BARTRANS:
				if(code == EN_CHANGE)
					SendPSChanged(hDlg);
				break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Taskbar"); break;
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
		g_bApplyTaskbar = TRUE;
		m_bChanged = TRUE;
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	}
}

/*------------------------------------------------
   initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	int i, alpha;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Taskbar", g_hfontDialog);
	
	UpDown_SetBuddy(hDlg, IDC_SPINBARTRANS, IDC_BARTRANS);
	
	CheckDlgButton(hDlg, IDC_TASKSWITCHFLAT,
		GetMyRegLong(NULL, "TaskSwitchFlat", FALSE));
	CheckDlgButton(hDlg, IDC_TASKSWITCH_SEPARAT,
		GetMyRegLong(NULL, "TaskSwitchSeparators", FALSE));
	CheckDlgButton(hDlg, IDC_TASKSWITCHICON,
		GetMyRegLong(NULL, "TaskSwitchIconsOnly", FALSE));
	CheckDlgButton(hDlg, IDC_RBHIDE,
		GetMyRegLong(NULL, "RebarGripperHide", FALSE));
	CheckDlgButton(hDlg, IDC_TBBORDER,
		GetMyRegLong(NULL, "TaskBarBorder", FALSE));
/*	CheckDlgButton(hDlg, IDC_TBBORDEREX,
		GetMyRegLong(NULL, "TaskBarBorderEx", FALSE)); */
	CheckDlgButton(hDlg, IDC_FLATTRAY,
		GetMyRegLong(NULL, "FlatTray", FALSE));
	
	alpha = GetMyRegLong(NULL, "AlphaTaskbar", 0);
	if(alpha > 100) alpha = 100;
	UpDown_SetRange(hDlg, IDC_SPINBARTRANS, 100, 0);
	UpDown_SetPos(hDlg, IDC_SPINBARTRANS, alpha);
	
	OnTaskSwitchFlat(hDlg);
	
	if(!IsIE4()) // old Win95/NT4
	{
		EnableDlgItem(hDlg, IDC_TASKSWITCHFLAT, FALSE);
		EnableDlgItem(hDlg, IDC_TASKSWITCH_SEPARAT, FALSE);
		EnableDlgItem(hDlg, IDC_TASKSWITCHICON, FALSE);
		EnableDlgItem(hDlg, IDC_RBHIDE, FALSE);
	}
	
	if(IsXPVisualStyle())
	{
		for(i = IDC_TASKBARCLASSIC; i <= IDC_TBBORDER; i++)
			EnableDlgItem(hDlg, i, FALSE);
	}
	
	if(g_winver&WINXP)
	{
		EnableDlgItem(hDlg, IDC_TASKSWITCH_SEPARAT, FALSE);
		if(IsTaskbarAnimation())
			EnableDlgItem(hDlg, IDC_TASKSWITCHICON, FALSE);
	}
	
	if(!(g_winver&WIN2000))
	{
		for(i = IDC_CAPBARTRANS; i <= IDC_SPINBARTRANS; i++)
			EnableDlgItem(hDlg, i, FALSE);
	}
	
	m_bInit = TRUE;
}

/*------------------------------------------------
  apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong(NULL, "TaskSwitchFlat",
		IsDlgButtonChecked(hDlg, IDC_TASKSWITCHFLAT));
	SetMyRegLong(NULL, "TaskSwitchSeparators",
		IsDlgButtonChecked(hDlg, IDC_TASKSWITCH_SEPARAT));
	SetMyRegLong(NULL, "TaskSwitchIconsOnly",
		IsDlgButtonChecked(hDlg, IDC_TASKSWITCHICON));
	SetMyRegLong(NULL, "RebarGripperHide",
		IsDlgButtonChecked(hDlg, IDC_RBHIDE));
	SetMyRegLong(NULL, "TaskBarBorder",
		IsDlgButtonChecked(hDlg, IDC_TBBORDER));
/*	SetMyRegLong(NULL, "TaskBarBorderEx",
		IsDlgButtonChecked(hDlg, IDC_TBBORDEREX)); */
	SetMyRegLong(NULL, "FlatTray",
		IsDlgButtonChecked(hDlg, IDC_FLATTRAY));
	
	SetMyRegLong(NULL, "AlphaTaskbar",
		UpDown_GetPos(hDlg, IDC_SPINBARTRANS));
}

/*------------------------------------------------
  checked "Flat task switch"
--------------------------------------------------*/
void OnTaskSwitchFlat(HWND hDlg)
{
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, IDC_TASKSWITCHFLAT);
	if(g_winver&WINXP) b = FALSE;
	EnableDlgItem(hDlg, IDC_TASKSWITCH_SEPARAT, b);
}

