/*-------------------------------------------------------------
  pageanalog.c : "Analog Clock" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

#if TC_ENABLE_ANALOGCLOCK

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnAnalogClock(HWND hDlg);
static void InitColor(HWND hDlg);
static void OnDrawItem(HWND hDlg, LPDRAWITEMSTRUCT pdis);
static void OnChooseColor(HWND hDlg, int id);
static void OnBrowse(HWND hDlg);

static const char *m_section = "AnalogClock";
static BOOL m_bInit = FALSE;
static BOOL m_bChanged = FALSE;

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
INT_PTR CALLBACK PageAnalogClockProc(HWND hDlg, UINT message,
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
				case IDC_ANALOGCLOCK:
					OnAnalogClock(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_COLHOUR:
				case IDC_COLMIN:
					if (code == CBN_SELCHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_CHOOSECOLHOUR:
				case IDC_CHOOSECOLMIN:
					OnChooseColor(hDlg, id);
					break;
				case IDC_HOURHANDBOLD:
				case IDC_MINHANDBOLD:
				case IDC_ANALOGPOSLEFT:
				case IDC_ANALOGPOSRIGHT:
				case IDC_ANALOGPOSMIDDLE:
					SendPSChanged(hDlg);
					break;
				case IDC_ANALOGHPOS:
				case IDC_ANALOGVPOS:
				case IDC_ANALOGSIZE:
				case IDC_ANALOGBMP:
					if (code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_ANALOGBMPBROWSE:
					OnBrowse(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_DRAWITEM:
			OnDrawItem(hDlg, (LPDRAWITEMSTRUCT)lParam);
			return TRUE;
		case WM_MEASUREITEM:
			// common/comobox.c
			OnMeasureItemColorCombo((LPMEASUREITEMSTRUCT)lParam);
			return TRUE;
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "AnalogClock"); break;
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
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[MAX_PATH];

	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "AnalogClock", g_hfontDialog);

	CheckDlgButton(hDlg, IDC_ANALOGCLOCK,
		GetMyRegLong(m_section, "UseAnalogClock", FALSE));

	OnAnalogClock(hDlg);

	InitColor(hDlg);
	
	CheckDlgButton(hDlg, IDC_HOURHANDBOLD,
		GetMyRegLong(m_section, "HourHandBold", FALSE));
	CheckDlgButton(hDlg, IDC_MINHANDBOLD,
		GetMyRegLong(m_section, "MinHandBold", FALSE));

	CheckRadioButton(hDlg, IDC_ANALOGPOSMIDDLE, IDC_ANALOGPOSRIGHT,
		GetMyRegLong(m_section, "AnalogClockPos", 0) + IDC_ANALOGPOSMIDDLE);

	UpDown_SetBuddy(hDlg, IDC_ANALOGHPOSSPIN, IDC_ANALOGHPOS);
	UpDown_SetRange(hDlg, IDC_ANALOGHPOSSPIN, 999, -999);
	UpDown_SetPos(hDlg, IDC_ANALOGHPOSSPIN,
		GetMyRegLong(m_section, "HorizontalPos", 0));
	UpDown_SetBuddy(hDlg, IDC_ANALOGVPOSSPIN, IDC_ANALOGVPOS);
	UpDown_SetRange(hDlg, IDC_ANALOGVPOSSPIN, 999, -999);
	UpDown_SetPos(hDlg, IDC_ANALOGVPOSSPIN,
		GetMyRegLong(m_section, "VerticalPos", 0));

	UpDown_SetBuddy(hDlg, IDC_ANALOGSIZESPIN, IDC_ANALOGSIZE);
	UpDown_SetRange(hDlg, IDC_ANALOGSIZESPIN, 99, 0);
	UpDown_SetPos(hDlg, IDC_ANALOGSIZESPIN,
		GetMyRegLong(m_section, "Size", 0));

	GetMyRegStr(m_section, "Bitmap", s, MAX_PATH, "");
	SetDlgItemText(hDlg, IDC_ANALOGBMP, s);

	m_bInit = TRUE;
}

/*------------------------------------------------
  Apply changes
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	int i;
	char s[MAX_PATH];

	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong(m_section, "UseAnalogClock",
		IsDlgButtonChecked(hDlg, IDC_ANALOGCLOCK));

	SetMyRegLong(m_section, "HourHandColor",
		CBGetItemData(hDlg, IDC_COLHOUR, CBGetCurSel(hDlg, IDC_COLHOUR)));
	SetMyRegLong(m_section, "MinHandColor",
		CBGetItemData(hDlg, IDC_COLMIN, CBGetCurSel(hDlg, IDC_COLMIN)));

	SetMyRegLong(m_section, "HourHandBold",
		IsDlgButtonChecked(hDlg, IDC_HOURHANDBOLD));
	SetMyRegLong(m_section, "MinHandBold",
		IsDlgButtonChecked(hDlg, IDC_MINHANDBOLD));

	if (IsDlgButtonChecked(hDlg, IDC_ANALOGPOSLEFT)) i = 1;
	else if (IsDlgButtonChecked(hDlg, IDC_ANALOGPOSRIGHT)) i = 2;
	else i = 0;
	SetMyRegLong(m_section, "AnalogClockPos", i);

	SetMyRegLong(m_section, "HorizontalPos",
		UpDown_GetPos(hDlg,IDC_ANALOGHPOSSPIN));
	SetMyRegLong(m_section, "VerticalPos",
		UpDown_GetPos(hDlg,IDC_ANALOGVPOSSPIN));

	SetMyRegLong(m_section, "Size",
		UpDown_GetPos(hDlg,IDC_ANALOGSIZESPIN));

	GetDlgItemText(hDlg, IDC_ANALOGBMP, s, MAX_PATH);
	SetMyRegStr(m_section, "Bitmap", s);
}

/*------------------------------------------------
   initizlize "Color" comoboxes
--------------------------------------------------*/
void InitColor(HWND hDlg)
{
	InitColorCombo(hDlg, IDC_COLHOUR, NULL, 0,
		GetMyRegLong(m_section, "HourHandColor", 0));

	InitColorCombo(hDlg, IDC_COLMIN, NULL, 0,
		GetMyRegLong(m_section, "MinHandColor", 0));
}

/*------------------------------------------------
  WM_DRAWITEM message
--------------------------------------------------*/
void OnDrawItem(HWND hDlg, LPDRAWITEMSTRUCT pdis)
{
	// common/comobox.c
	OnDrawItemColorCombo(pdis, NULL);
}

/*------------------------------------------------
  "Display analog clock" is checked
--------------------------------------------------*/
void OnAnalogClock(HWND hDlg)
{
	BOOL b = IsDlgButtonChecked(hDlg, IDC_ANALOGCLOCK);
	HWND hwnd = GetDlgItem(hDlg, IDC_ANALOGCLOCK);

	while ((hwnd = GetWindow(hwnd, GW_HWNDNEXT)) != NULL)
		EnableWindow(hwnd, b);
}

/*------------------------------------------------
  "..." button : choose color
--------------------------------------------------*/
void OnChooseColor(HWND hDlg, int id)
{
	int idCombo = id - 1;
	
	// common/combobox.c
	if(ChooseColorWithCombo(hDlg, idCombo))
	{
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		SendPSChanged(hDlg);
	}
}

/*------------------------------------------------
  browse bitmap file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	char *filter = "Bitmap (*.bmp)\0*.bmp\0\0";
	
	GetDlgItemText(hDlg, IDC_ANALOGBMP, deffile, MAX_PATH);
	
	// select file : common/selectfile.c
	if(SelectMyFile(g_hInst, hDlg, filter, 0, deffile, fname))
	{
		SetDlgItemText(hDlg, IDC_ANALOGBMP, fname);
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		SendPSChanged(hDlg);
	}
}
#endif /* TC_ENABLE_ANALOGCLOCK */
