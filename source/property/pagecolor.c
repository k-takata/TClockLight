/*-------------------------------------------------------------
  pagecolor.c : "Color and Font" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void InitColor(HWND hDlg);
static void OnDrawItem(HWND hDlg, LPDRAWITEMSTRUCT pdis);
static void OnChooseColor(HWND hDlg, int id);
#if TC_ENABLE_CLOCKDECORATION
static void OnSelectDecoration(HWND hDlg);
#endif
static void OnCheckColor(HWND hDlg);
static void InitFont(HWND hDlg);
static void OnFont(HWND hDlg, int bInit);

static HFONT m_hfontb;  // for IDC_BOLD
static HFONT m_hfonti;  // for IDC_ITALIC
static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageColorProc(HWND hDlg, UINT message,
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
				case IDC_COLBACK:
				case IDC_COLBACK2:
				case IDC_COLFORE:
#if TC_ENABLE_CLOCKDECORATION
				case IDC_COLSHADOW:
#endif
				case IDC_FONT:
				case IDC_FONTSIZE:
					if(code == CBN_SELCHANGE || code == CBN_EDITCHANGE)
					{
						if(id == IDC_FONT) OnFont(hDlg, FALSE);
						SendPSChanged(hDlg);
					}
					break;
				case IDC_CHKCOLOR:
				case IDC_CHKCOLOR2:
					OnCheckColor(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_CHOOSECOLBACK:
				case IDC_CHOOSECOLBACK2:
				case IDC_CHOOSECOLFORE:
#if TC_ENABLE_CLOCKDECORATION
				case IDC_CHOOSECOLSHADOW:
#endif
					OnChooseColor(hDlg, id);
					break;
#if TC_ENABLE_CLOCKDECORATION
				case IDC_DECONONE:
				case IDC_DECOSHADOW:
				case IDC_DECOBORDER:
					OnSelectDecoration(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_SHADOWRANGE:
					if (code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
#endif
				case IDC_GRAD1:
				case IDC_GRAD2:
				case IDC_FILLTRAY:
				case IDC_BOLD:
				case IDC_ITALIC:
					SendPSChanged(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Color"); break;
			}
			return TRUE;
		case WM_MEASUREITEM:
			// common/comobox.c
			OnMeasureItemColorCombo((LPMEASUREITEMSTRUCT)lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItem(hDlg, (LPDRAWITEMSTRUCT)lParam);
			return TRUE;
		case WM_DESTROY:
			if(m_hfontb) DeleteObject(m_hfontb);
			if(m_hfonti) DeleteObject(m_hfonti);
			break;
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
  Initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	LOGFONT logfont;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Color", g_hfontDialog);
	
	// settings of "background" and "text"
	
	InitColor(hDlg);
	
	CheckDlgButton(hDlg, IDC_CHKCOLOR,
		GetMyRegLong(NULL, "UseBackColor", IsXPVisualStyle() ? FALSE : TRUE));
	CheckDlgButton(hDlg, IDC_CHKCOLOR2,
		GetMyRegLong(NULL, "UseBackColor2", FALSE));
	
	CheckRadioButton(hDlg, IDC_GRAD1, IDC_GRAD2,
		(GetMyRegLong(NULL, "GradDir", 0) == 0) ? IDC_GRAD1 : IDC_GRAD2);
	
	CheckDlgButton(hDlg, IDC_FILLTRAY,
		GetMyRegLong(NULL, "FillTray", FALSE));
	EnableDlgItem(hDlg, IDC_FILLTRAY,
		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR));
	
	OnCheckColor(hDlg);
	
#if TC_ENABLE_CLOCKDECORATION
	// settings for decoration
	CheckRadioButton(hDlg, IDC_DECONONE, IDC_DECOBORDER,
		GetMyRegLong(NULL, "ClockDecoration", 0) + IDC_DECONONE);
	OnSelectDecoration(hDlg);
	UpDown_SetBuddy(hDlg, IDC_SHADOWRANGESPIN, IDC_SHADOWRANGE);
	UpDown_SetRange(hDlg, IDC_SHADOWRANGESPIN, 10, 1);
	UpDown_SetPos(hDlg, IDC_SHADOWRANGESPIN,
		GetMyRegLong(NULL, "ShadowRange", 1));
#endif
	
	// settings of "font" and "font size"
	
	InitFont(hDlg);
	OnFont(hDlg, TRUE);
	
	CheckDlgButton(hDlg, IDC_BOLD, GetMyRegLong(NULL, "Bold", FALSE));
	CheckDlgButton(hDlg, IDC_ITALIC, GetMyRegLong(NULL, "Italic", FALSE));
	
	m_hfontb = m_hfonti = NULL;
	if(g_hfontDialog)
	{
		char s[80];
		
		GetObject(g_hfontDialog, sizeof(LOGFONT), &logfont);
		logfont.lfWeight = FW_BOLD;
		m_hfontb = CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg, IDC_BOLD, WM_SETFONT, (WPARAM)m_hfontb, 0);
		
		logfont.lfWeight = FW_NORMAL;
		logfont.lfItalic = 1;
		m_hfonti = CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg, IDC_ITALIC, WM_SETFONT, (WPARAM)m_hfonti, 0);
		
		GetDlgItemText(hDlg, IDC_ITALIC, s, 77);
		strcat(s, "  ");
		SetDlgItemText(hDlg, IDC_ITALIC, s);
	}
	
	m_bInit = TRUE;
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[80];
#if TC_ENABLE_CLOCKDECORATION
	int i;
#endif
	
	// settings of "background" and "text"
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong(NULL, "UseBackColor",
		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR));
	SetMyRegLong(NULL, "BackColor",
		CBGetItemData(hDlg, IDC_COLBACK, CBGetCurSel(hDlg, IDC_COLBACK)));
	
	SetMyRegLong(NULL, "UseBackColor2",
		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2));
	SetMyRegLong(NULL, "BackColor2",
		CBGetItemData(hDlg, IDC_COLBACK2, CBGetCurSel(hDlg, IDC_COLBACK2)));
	
	SetMyRegLong(NULL, "GradDir",
		IsDlgButtonChecked(hDlg, IDC_GRAD1) ? 0 : 1);
	
	SetMyRegLong(NULL, "FillTray",
		IsDlgButtonChecked(hDlg, IDC_FILLTRAY));
	
	SetMyRegLong(NULL, "ForeColor",
		CBGetItemData(hDlg, IDC_COLFORE, CBGetCurSel(hDlg, IDC_COLFORE)));
	
#if TC_ENABLE_CLOCKDECORATION
	// settings of decoration
	if (IsDlgButtonChecked(hDlg, IDC_DECOSHADOW)) i = 1;
	else if (IsDlgButtonChecked(hDlg, IDC_DECOBORDER)) i = 2;
	else i = 0;
	SetMyRegLong(NULL, "ClockDecoration", i);
	SetMyRegLong(NULL, "ShadowColor",
		CBGetItemData(hDlg, IDC_COLSHADOW, CBGetCurSel(hDlg, IDC_COLSHADOW)));
	SetMyRegLong(NULL, "ShadowRange",
		UpDown_GetPos(hDlg,IDC_SHADOWRANGESPIN));
#endif
	
	// settings of "font" and "font size"
	
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), s);
	SetMyRegStr(NULL, "Font", s);
	
	GetDlgItemText(hDlg, IDC_FONTSIZE, s, 10);
	if(s[0]) SetMyRegStr(NULL, "FontSize", s);
	else SetMyRegStr(NULL, "FontSize", "9");
	
	SetMyRegLong(NULL, "Bold", IsDlgButtonChecked(hDlg, IDC_BOLD));
	SetMyRegLong(NULL, "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
}

/*---------------------------------------------------
  enable/disable to use "background" / "background 2"
-----------------------------------------------------*/
void OnCheckColor(HWND hDlg)
{
	HWND hwnd;
	BOOL b1, b2;
	
	b1 = IsDlgButtonChecked(hDlg, IDC_CHKCOLOR);
	
	hwnd = GetWindow(GetDlgItem(hDlg, IDC_CHKCOLOR), GW_HWNDNEXT);
	while(hwnd)
	{
		EnableWindow(hwnd, b1);
		if(hwnd == GetDlgItem(hDlg, IDC_CHKCOLOR2)) break;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	
	b2 = IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2);
	
	hwnd = GetWindow(GetDlgItem(hDlg, IDC_CHKCOLOR2), GW_HWNDNEXT);
	while(hwnd)
	{
		EnableWindow(hwnd, b1 && b2);
		if(hwnd == GetDlgItem(hDlg, IDC_GRAD2)) break;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	
	EnableDlgItem(hDlg, IDC_FILLTRAY, b1);
}

#if TC_ENABLE_CLOCKDECORATION
/*---------------------------------------------------
  enable/disable to use
-----------------------------------------------------*/
void OnSelectDecoration(HWND hDlg)
{
	BOOL b;
	HWND hwnd;

	b = !IsDlgButtonChecked(hDlg, IDC_DECONONE);
	hwnd = GetDlgItem(hDlg, IDC_DECOBORDER);
	while ((hwnd = GetWindow(hwnd, GW_HWNDNEXT)) != NULL)
	{
		EnableWindow(hwnd, b);
		if (GetDlgCtrlID(hwnd) == IDC_CHOOSECOLSHADOW) break;
	}

	b = IsDlgButtonChecked(hDlg, IDC_DECOSHADOW);
	hwnd = GetDlgItem(hDlg, IDC_CHOOSECOLSHADOW);
	while ((hwnd = GetWindow(hwnd, GW_HWNDNEXT)) != NULL)
	{
		EnableWindow(hwnd, b);
		if (GetDlgCtrlID(hwnd) == IDC_SHADOWRANGESPIN) break;
	}
}
#endif /* TC_ENABLE_CLOCKDECORATION */

/*------------------------------------------------
   initialize "Color" comoboxes
--------------------------------------------------*/
void InitColor(HWND hDlg)
{
	COLORREF cols[4], colDef;
	
	cols[0] = 0x80000000|COLOR_3DFACE;
	cols[1] = 0x80000000|COLOR_3DSHADOW;
	cols[2] = 0x80000000|COLOR_3DHILIGHT;
	cols[3] = 0x80000000|COLOR_BTNTEXT;
	
	colDef = GetMyRegLong("", "BackColor",
		0x80000000 | COLOR_3DFACE);
	InitColorCombo(hDlg, IDC_COLBACK, cols, 4, colDef);

	colDef = GetMyRegLong("", "BackColor2", colDef);
	InitColorCombo(hDlg, IDC_COLBACK2, cols, 4, colDef);

	colDef = GetMyRegLong("", "ForeColor",
		0x80000000 | COLOR_BTNTEXT);
	InitColorCombo(hDlg, IDC_COLFORE, cols, 4, colDef);
	
	colDef = GetMyRegLong("", "ShadowColor", 0);
	InitColorCombo(hDlg, IDC_COLSHADOW, cols, 4, colDef);
}

/*------------------------------------------------
  WM_DRAWITEM message
--------------------------------------------------*/
void OnDrawItem(HWND hDlg, LPDRAWITEMSTRUCT pdis)
{
	char texts[4][80];
	strcpy(texts[0], MyString(IDS_BTNFACE, "ButtonFace"));
	strcpy(texts[1], MyString(IDS_BTNSHADOW, "ButtonShadow"));
	strcpy(texts[2], MyString(IDS_BTNLIGHT, "ButtonLight"));
	strcpy(texts[3], MyString(IDS_BTNTEXT, "ButtonText"));
	
	// common/comobox.c
	OnDrawItemColorCombo(pdis, texts);
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
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitFont(HWND hDlg)
{
	char s[LF_FACESIZE];
	
	GetMyRegStr("", "Font", s, LF_FACESIZE, "");
	if(s[0] == 0)
	{
		GetDefaultFontName(s, "System");
	}
	
	// common/combobox.c
	InitFontNameCombo(hDlg, IDC_FONT, s);
}

/*------------------------------------------------
   set sizes to "Font Size" combo box
--------------------------------------------------*/
void OnFont(HWND hDlg, BOOL bInit)
{
	char s[160], size[10];
	int charset;
	int index;
	
	if(bInit) // WM_INITDIALOG
		GetMyRegStr(NULL, "FontSize", size, 10, "9");
	else     // WM_COMMAND
		GetDlgItemText(hDlg, IDC_FONTSIZE, size, 10);
	
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), (LPARAM)s);
	charset = CBGetItemData(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT));
	
	// common/combobox.c
	InitFontSizeCombo(hDlg, IDC_FONTSIZE, s, charset);
	
	index = CBFindStringExact(hDlg, IDC_FONTSIZE, size);
	if(index == CB_ERR)
		SetDlgItemText(hDlg, IDC_FONTSIZE, size);
	else CBSetCurSel(hDlg, IDC_FONTSIZE, index);
}

