/*-------------------------------------------------------------
  pagetooltip.c : "Cuckoo" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

BOOL CALLBACK PageTooltipProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnUseTip1(HWND hDlg);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

static char *m_section = "Tooltip";

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageTooltipProc(HWND hDlg, UINT message,
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
				case IDC_USETOOLTIP1:
					OnUseTip1(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_TOOLTIP:
				case IDC_TOOLTIPTIME:
					if(code == EN_CHANGE) SendPSChanged(hDlg);
					break;
				case IDC_TOOLTIPSTYLE1:
				case IDC_TOOLTIPSTYLE2:
					SendPSChanged(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Tooltip"); break;
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
		g_bApplyTip = TRUE;
		m_bChanged = TRUE;
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	}
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[BUFSIZE_TOOLTIP];
	BOOL b;
	int n;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Tooltip", g_hfontDialog);
	
	UpDown_SetBuddy(hDlg, IDC_TOOLTIPTIMESPIN, IDC_TOOLTIPTIME);
	
	b = GetMyRegLong(m_section, "Tip1Use", TRUE);
	CheckDlgButton(hDlg, IDC_USETOOLTIP1, b);
	
	GetMyRegStr(m_section, "Tooltip", s, BUFSIZE_TOOLTIP,
		"\"TClock\" LDATE");
	SetDlgItemText(hDlg, IDC_TOOLTIP, s);
	
	n = GetMyRegLong(NULL, "BalloonFlg", 0);
	n = GetMyRegLong(m_section, "Style", n);
	CheckRadioButton(hDlg, IDC_TOOLTIPSTYLE1, IDC_TOOLTIPSTYLE2,
		IDC_TOOLTIPSTYLE1 + n);
	
	n = GetMyRegLong(NULL, "TipDispTime", 5);
	n = GetMyRegLong(m_section, "DispTime", n);
	UpDown_SetRange(hDlg, IDC_TOOLTIPTIMESPIN, 32, 0);
	UpDown_SetPos(hDlg, IDC_TOOLTIPTIMESPIN, n);
	
	OnUseTip1(hDlg);
	
	m_bInit = TRUE;
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[BUFSIZE_TOOLTIP];
	int n;
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong(m_section, "Tip1Use",
		IsDlgButtonChecked(hDlg, IDC_USETOOLTIP1));
	
	GetDlgItemText(hDlg, IDC_TOOLTIP, s, BUFSIZE_TOOLTIP);
	SetMyRegStr(m_section, "Tooltip", s);
	
	if(IsDlgButtonChecked(hDlg, IDC_TOOLTIPSTYLE2))
		SetMyRegLong(m_section, "Style", 1);
	else SetMyRegLong(m_section, "Style", 0);
	
	DelMyReg(NULL, "BalloonFlg");
	
	n = GetDlgItemInt(hDlg, IDC_TOOLTIPTIME, NULL, FALSE);
	SetMyRegLong(m_section, "DispTime", n);
	DelMyReg(NULL, "TipDispTime");
}

/*------------------------------------------------
   "Show tooltip"
--------------------------------------------------*/
void OnUseTip1(HWND hDlg)
{
	BOOL b = IsDlgButtonChecked(hDlg, IDC_USETOOLTIP1);
	HWND hwnd = GetWindow(GetDlgItem(hDlg, IDC_USETOOLTIP1), GW_HWNDNEXT);
	
	while(hwnd)
	{
		EnableWindow(hwnd, b);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

