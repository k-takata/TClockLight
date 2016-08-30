/*-------------------------------------------------------------
  pageformat.c : "Format" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLocale(HWND hDlg);
static void OnCustom(HWND hDlg, BOOL bmouse);
static void On12Hour(HWND hDlg);
static void OnDetail(HWND hDlg);
static void OnFormatCheck(HWND hDlg, WORD id);

static char m_CustomFormat[BUFSIZE_FORMAT];
static BOOL m_bInit = FALSE;
static BOOL m_bChanged = FALSE;

static const char *m_entrydate[] = { "Year4", "Year", "Month", "MonthS",
	"Day", "Weekday", "Hour", "Minute", "Second", "Kaigyo",
	"AMPM", "Hour12", "Custom",  };
#define ENTRY(id) m_entrydate[(id)-IDC_YEAR4]

/*------------------------------------------------
   Dialog Procedure for the "Format" page
--------------------------------------------------*/
INT_PTR CALLBACK PageFormatProc(HWND hDlg, UINT message,
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
				case IDC_LOCALE:
					if(code == CBN_SELCHANGE)
						OnLocale(hDlg);
					break;
				case IDC_FORMAT:
#if TC_ENABLE_SYSINFO
				case IDC_SYSII:
#endif
					if(code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_CUSTOM:
					OnCustom(hDlg, TRUE);
					break;
				case IDC_12HOUR:
					On12Hour(hDlg);
					break;
				case IDC_FORMATDETAIL:
					OnDetail(hDlg);
					break;
			}
			if(IDC_YEAR4 <= id && id <= IDC_AMPM)
				OnFormatCheck(hDlg, id);
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Format"); break;
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
		g_bApplyClock = TRUE;
		m_bChanged = TRUE;
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	}
}

/*------------------------------------------------
  Initialize the "Format" page
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HFONT hfont;
	char s[BUFSIZE_FORMAT];
	int i, ilang;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Format", g_hfontDialog);
	
	hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_FORMAT, WM_SETFONT, (WPARAM)hfont, 0);
	
	// "Locale" combobox
	ilang = GetMyRegLong("", "Locale", (int)GetUserDefaultLangID());
	InitLocaleCombo(hDlg, IDC_LOCALE, ilang); // common/combobox.c
	
	InitAutoFormat(ilang); // common/autoformat.c
	
	// "year" -- "second"
	for(i = IDC_YEAR4; i <= IDC_KAIGYO; i++)
	{
		CheckDlgButton(hDlg, i,
			GetMyRegLong("", ENTRY(i), TRUE));
	}
	
	if(IsDlgButtonChecked(hDlg, IDC_YEAR))
		CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
	if(IsDlgButtonChecked(hDlg, IDC_YEAR4))
		CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);
	
	if(IsDlgButtonChecked(hDlg, IDC_MONTH))
		CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
	if(IsDlgButtonChecked(hDlg, IDC_MONTHS))
		CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);
	
	// "Internet Time" -- "Customize format"
	for(i = IDC_AMPM; i <= IDC_CUSTOM; i++)
	{
		CheckDlgButton(hDlg, i,
			GetMyRegLong("", ENTRY(i), FALSE));
	}
	
	GetMyRegStr("", "Format", s, BUFSIZE_FORMAT, "");
	SetDlgItemText(hDlg, IDC_FORMAT, s);
	
	GetMyRegStr("", "CustomFormat", m_CustomFormat, BUFSIZE_FORMAT, "");
	
	On12Hour(hDlg);
	OnCustom(hDlg, FALSE);
	
#if TC_ENABLE_SYSINFO
	// "Update interval"
	UpDown_SetBuddy(hDlg, IDC_SYSIISPIN, IDC_SYSII);
	UpDown_SetRange(hDlg, IDC_SYSIISPIN, 60, 1);
	i = GetMyRegLong(NULL, "IntervalSysInfo", 4);
	if(i < 1 || 60 < i) i = 4;
	UpDown_SetPos(hDlg, IDC_SYSIISPIN, i);
#endif
	
	m_bInit = TRUE;
}

/*------------------------------------------------
  Apply changes
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[BUFSIZE_FORMAT];
	int i;
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong("", "Locale",
		CBGetItemData(hDlg, IDC_LOCALE, CBGetCurSel(hDlg, IDC_LOCALE)));
	
	for(i = IDC_YEAR4; i <= IDC_CUSTOM; i++)
	{
		SetMyRegLong("", ENTRY(i), IsDlgButtonChecked(hDlg, i));
	}
	
	GetDlgItemText(hDlg, IDC_FORMAT, s, BUFSIZE_FORMAT);
	SetMyRegStr("", "Format", s);
	
	if(IsDlgButtonChecked(hDlg, IDC_CUSTOM))
	{
		strcpy(m_CustomFormat, s);
		SetMyRegStr("", "CustomFormat", m_CustomFormat);
	}
	
#if TC_ENABLE_SYSINFO
	SetMyRegLong(NULL, "IntervalSysInfo",
		UpDown_GetPos(hDlg,IDC_SYSIISPIN));
#endif
}

/*------------------------------------------------
  When changed "Locale" combobox
--------------------------------------------------*/
void OnLocale(HWND hDlg)
{
	int ilang;
	
	ilang = CBGetItemData(hDlg, IDC_LOCALE, CBGetCurSel(hDlg, IDC_LOCALE));
	InitAutoFormat(ilang);
	
	OnCustom(hDlg, FALSE);
}

/*------------------------------------------------
  "Customize format" checkbox
--------------------------------------------------*/
void OnCustom(HWND hDlg, BOOL bmouse)
{
	BOOL b;
	int i;
	
	b = IsDlgButtonChecked(hDlg, IDC_CUSTOM);
	EnableDlgItem(hDlg, IDC_FORMAT, b);
	
	for(i = IDC_YEAR4; i <= IDC_AMPM; i++)
		EnableDlgItem(hDlg, i, !b);
	
	if(m_CustomFormat[0] && bmouse)
	{
		if(b) SetDlgItemText(hDlg, IDC_FORMAT, m_CustomFormat);
		else GetDlgItemText(hDlg, IDC_FORMAT, m_CustomFormat, BUFSIZE_FORMAT);
	}
	
	if(!b) OnFormatCheck(hDlg, 0);
	
	SendPSChanged(hDlg);
}

/*------------------------------------------------
   "12H" combobox
--------------------------------------------------*/
void On12Hour(HWND hDlg)
{
	BOOL b;
	b = IsDlgButtonChecked(hDlg, IDC_12HOUR);
	if(!b)
	{
		CheckDlgButton(hDlg, IDC_AMPM, 0);
		if(!IsDlgButtonChecked(hDlg, IDC_CUSTOM))
			OnFormatCheck(hDlg, 0);
	}
	
	SendPSChanged(hDlg);
}

/*------------------------------------------------
   "Detail..."
--------------------------------------------------*/
void OnDetail(HWND hDlg)
{
	int r, ilang;
	
	ilang = CBGetItemData(hDlg, IDC_LOCALE,
		CBGetCurSel(hDlg, IDC_LOCALE));
	r = (int)DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_FORMAT2),
		GetParent(hDlg), DlgProcFormat2, (LPARAM)ilang);
	if(r == IDOK) SendPSChanged(hDlg);
}

/*------------------------------------------------
  When clicked "year" -- "am/pm"
--------------------------------------------------*/
void OnFormatCheck(HWND hDlg, WORD id)
{
	char s[BUFSIZE_FORMAT];
	int parts[15];
	int i;
	
	for(i = IDC_YEAR4; i <= IDC_AMPM; i++)
	{
		parts[i-IDC_YEAR4] = IsDlgButtonChecked(hDlg, i);
	}
	
	if(id == IDC_YEAR4 || id == IDC_YEAR)
	{
		if(id == IDC_YEAR4 && parts[PART_YEAR4])
		{
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);
			parts[PART_YEAR] = FALSE;
		}
		if(id == IDC_YEAR && parts[PART_YEAR])
		{
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
			parts[PART_YEAR4] = FALSE;
		}
	}
	
	if(id == IDC_MONTH || id == IDC_MONTHS)
	{
		if(id == IDC_MONTH && parts[PART_MONTH])
		{
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
			parts[PART_MONTHS] = FALSE;
		}
		if(id == IDC_MONTHS && parts[PART_MONTHS])
		{
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);
			parts[PART_MONTH] = FALSE;
		}
	}
	
	if(id == IDC_AMPM)
	{
		CheckDlgButton(hDlg, IDC_12HOUR, 1);
		On12Hour(hDlg);
	}
	
	AutoFormat(s, parts); // common/autoformat.c
	SetDlgItemText(hDlg, IDC_FORMAT, s);
	SendPSChanged(hDlg);
}

