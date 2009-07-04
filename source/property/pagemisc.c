/*-------------------------------------------------------------
  pagemisc.c : "Misc" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

INT_PTR CALLBACK PageMiscProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnBrowse(HWND hDlg);

static m_bInit = FALSE;

/*------------------------------------------------
  dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageMiscProc(HWND hDlg, UINT message,
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
			switch (id)
			{
				case IDC_NOCLOCK:
					g_bApplyClock = TRUE;
					SendPSChanged(hDlg);
					break;
				case IDC_MCIWAVE:
				case IDC_TASKBARRESTART:
#if TC_ENABLE_DESKTOPICON
				case IDC_DESKTOPICON:
				case IDC_TRANSDESKTOPICONBK:
#endif
					SendPSChanged(hDlg);
					break;
				case IDC_DELAYSTART:
					if(code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_BROWSEHELP:
					OnBrowse(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Misc"); break;
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
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[MAX_PATH];
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Misc", g_hfontDialog);
	
	CheckDlgButton(hDlg, IDC_NOCLOCK,
		GetMyRegLong(NULL, "NoClock", FALSE));
	
	CheckDlgButton(hDlg, IDC_MCIWAVE,
		GetMyRegLong(NULL, "MCIWave", FALSE));
	
	SetDlgItemInt(hDlg, IDC_DELAYSTART,
		GetMyRegLong(NULL, "DelayStart", 0), FALSE);
	
	CheckDlgButton(hDlg, IDC_TASKBARRESTART,
		GetMyRegLong(NULL, "TaskbarRestart", FALSE));
	
#if TC_ENABLE_DESKTOPICON
	CheckDlgButton(hDlg, IDC_DESKTOPICON,
		GetMyRegLong(NULL, "DeskTopIcon", FALSE));
	
	CheckDlgButton(hDlg, IDC_TRANSDESKTOPICONBK,
		GetMyRegLong(NULL, "TransDeskTopIconBK", FALSE));
#endif
	
	GetMyRegStr(NULL, "HelpURL", s, MAX_PATH, "");
	SetDlgItemText(hDlg, IDC_HELPURL, s);
	
	m_bInit = TRUE;
}

/*------------------------------------------------
  apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[MAX_PATH];
	
	SetMyRegLong(NULL, "NoClock",
		IsDlgButtonChecked(hDlg, IDC_NOCLOCK));
	
	SetMyRegLong(NULL, "MCIWave",
		IsDlgButtonChecked(hDlg, IDC_MCIWAVE));
	
	SetMyRegLong(NULL, "DelayStart",
		GetDlgItemInt(hDlg, IDC_DELAYSTART, NULL, FALSE));
	
	SetMyRegLong(NULL, "TaskbarRestart",
		IsDlgButtonChecked(hDlg, IDC_TASKBARRESTART));
	
#if TC_ENABLE_DESKTOPICON
	SetMyRegLong(NULL, "DeskTopIcon",
		IsDlgButtonChecked(hDlg, IDC_DESKTOPICON));
	
	SetMyRegLong(NULL, "TransDeskTopIconBK",
		IsDlgButtonChecked(hDlg, IDC_TRANSDESKTOPICONBK));
#endif
	
	GetDlgItemText(hDlg, IDC_HELPURL, s, MAX_PATH);
	SetMyRegStr(NULL, "HelpURL", s);
	
#if TC_ENABLE_DESKTOPICON
	SetDesktopIcons();
#endif
}

/*------------------------------------------------
  clicked "..." button
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	char *filter = "HTML\0*.html;*.htm\0\0";
	
	GetDlgItemText(hDlg, IDC_HELPURL, deffile, MAX_PATH);
	
	if(!SelectMyFile(g_hInst, hDlg, filter, 0, deffile, fname))
		return;
	
	SetDlgItemText(hDlg, IDC_HELPURL, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

