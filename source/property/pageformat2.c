/*-------------------------------------------------------------
  pageformat2.c : "Detail of format" dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

BOOL CALLBACK DlgProcFormat2(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void OnInit(HWND hDlg, LPARAM lParam);
static void OnOK(HWND hDlg);

/*------------------------------------------------
   dialog procedure of "Detail of format"
--------------------------------------------------*/
BOOL CALLBACK DlgProcFormat2(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg, lParam);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id; // , code;
			id = LOWORD(wParam); // code = HIWORD(wParam);
			switch (id)
			{
				case IDOK: OnOK(hDlg); // fall through
				case IDCANCEL: EndDialog(hDlg, id);
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
  initialize the dialog
--------------------------------------------------*/
void OnInit(HWND hDlg, LPARAM lParam)
{
	char s[80], s2[11];
	int ilang, codepage;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Format2", g_hfontDialog);
	
	ilang = (int)lParam;
	
	// "AM Symbol" and "PM Symbol"
	CBResetContent(hDlg, IDC_AMSYMBOL);
	GetMyRegStr(NULL, "AMsymbol", s, 80, "");
	if(s[0]) CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)s);
	
	codepage = GetCodePage(GetUserDefaultLangID());
	
	MyGetLocaleInfoA(ilang, codepage, LOCALE_S1159, s2, 10);
	if(s2[0] && strcmp(s, s2) != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)s2);
	if(strcmp(s, "AM") != 0 && strcmp(s2, "AM") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"AM");
	if(strcmp(s, "am") != 0 && strcmp(s2, "am") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"am");
	CBSetCurSel(hDlg, IDC_AMSYMBOL, 0);
	
	CBResetContent(hDlg, IDC_PMSYMBOL);
	GetMyRegStr(NULL, "PMsymbol", s, 80, "");
	if(s[0]) CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)s);
	MyGetLocaleInfoA(ilang, codepage, LOCALE_S2359, s2, 10);
	if(s2[0] && strcmp(s, s2) != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)s2);
	if(strcmp(s, "PM") != 0 && strcmp(s2, "PM") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"PM");
	if(strcmp(s, "pm") != 0 && strcmp(s2, "pm") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"pm");
	CBSetCurSel(hDlg, IDC_PMSYMBOL, 0);
	
	CheckDlgButton(hDlg, IDC_ZERO,
		GetMyRegLong("", "HourZero", FALSE));
}

/*------------------------------------------------
  "OK" button
--------------------------------------------------*/
void OnOK(HWND hDlg)
{
	char s[80];
	
	GetDlgItemText(hDlg, IDC_AMSYMBOL, s, 80);
	SetMyRegStr("", "AMsymbol", s);
	GetDlgItemText(hDlg, IDC_PMSYMBOL, s, 80);
	SetMyRegStr("", "PMsymbol", s);
	
	SetMyRegLong("", "HourZero",
		IsDlgButtonChecked(hDlg, IDC_ZERO));
}
