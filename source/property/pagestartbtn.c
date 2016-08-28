/*-------------------------------------------------------------
  pagestartbtn.c : "Start button" page of properties
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
static void InitFont(HWND hDlg);
static void OnFont(HWND hDlg, BOOL bInit);
static void OnStartBtn(HWND hDlg);
static void OnUseBack(HWND hDlg);
static void OnSansho(HWND hDlg);
static void OnChooseColor(HWND hDlg);
static void OnSanshoBmp(HWND hDlg);

static char *m_section = "StartButton";
static BOOL m_bInit = FALSE;
static BOOL m_bChanged = FALSE;
static HFONT m_hfontb;
static HFONT m_hfonti;

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
INT_PTR CALLBACK PageStartButtonProc(HWND hDlg, UINT message,
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
				case IDC_STARTBTN:
					OnStartBtn(hDlg);
					break;
				case IDC_FILESTART:
				case IDC_CAPTIONSTART:
				case IDC_STARTBTNBACKBMP:
					if(code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_SANSHOSTART:
					OnSansho(hDlg);
					break;
				case IDC_STARTBTNCOL:
				case IDC_STARTBTNFONT:
				case IDC_STARTBTNFONTSIZE:
					if(code == CBN_SELCHANGE || code == CBN_EDITCHANGE)
					{
						if(id == IDC_STARTBTNFONT) OnFont(hDlg, FALSE);
						SendPSChanged(hDlg);
					}
					break;
				case IDC_STARTBTNCHOOSECOL:
					OnChooseColor(hDlg);
					break;
				case IDC_SANSHOSTARTBACKBMP:
					OnSanshoBmp(hDlg);
					break;
				case IDC_STARTBTNUSEBACK:
					OnUseBack(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_STARTBTNBOLD:
				case IDC_STARTBTNITALIC:
				case IDC_STARTBTNFLAT:
				case IDC_STARTBTNHIDE:
				case IDC_STARTMENUCLOCK:
					SendPSChanged(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "StartButton"); break;
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
			DeleteObject(m_hfontb);
			DeleteObject(m_hfonti);
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
	BOOL b;
	char s[MAX_PATH], s2[MAX_PATH];
	LOGFONT logfont;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "StartButton", g_hfontDialog);
	
	b = GetMyRegLong(NULL, "StartButton", FALSE);
	b = GetMyRegLong(m_section, "StartButton", b);
	CheckDlgButton(hDlg, IDC_STARTBTN, b);
	
	GetMyRegStr(NULL, "StartButtonIcon", s, MAX_PATH, "");
	GetMyRegStr(m_section, "Icon", s2, MAX_PATH, s);
	SetDlgItemText(hDlg, IDC_FILESTART, s2);
	
	GetMyRegStr(NULL, "StartButtonCaption", s, 80, "Start");
	GetMyRegStr(m_section, "Caption", s2, 80, s);
	SetDlgItemText(hDlg, IDC_CAPTIONSTART, s2);
	
	InitColor(hDlg);
	
	InitFont(hDlg);
	OnFont(hDlg, TRUE);
	
	CheckDlgButton(hDlg, IDC_STARTBTNBOLD,
		GetMyRegLong(m_section, "Bold", TRUE));
	CheckDlgButton(hDlg, IDC_STARTBTNITALIC,
		GetMyRegLong(m_section, "Italic", FALSE));	
	
	m_hfontb = m_hfonti = NULL;
	if(g_hfontDialog)
	{
		GetObject(g_hfontDialog, sizeof(LOGFONT), &logfont);
		logfont.lfWeight = FW_BOLD;
		m_hfontb = CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg, IDC_STARTBTNBOLD,
			WM_SETFONT, (WPARAM)m_hfontb, 0);
		
		logfont.lfWeight = FW_NORMAL;
		logfont.lfItalic = 1;
		m_hfonti = CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg, IDC_STARTBTNITALIC,
			WM_SETFONT, (WPARAM)m_hfonti, 0);
		
		GetDlgItemText(hDlg, IDC_STARTBTNITALIC, s, 77);
		strcat(s, "  ");
		SetDlgItemText(hDlg, IDC_STARTBTNITALIC, s);
	}
	
	b = GetMyRegLong(m_section, "UseBackBmp", FALSE);
	CheckDlgButton(hDlg, IDC_STARTBTNUSEBACK, b);
	
	GetMyRegStr(m_section, "BackBmp", s, 80, "");
	SetDlgItemText(hDlg, IDC_STARTBTNBACKBMP, s);
	
	if(!IsXPVisualStyle())
	{
		b = GetMyRegLong(NULL, "StartButtonFlat", FALSE);
		b = GetMyRegLong(m_section, "Flat", b);
		CheckDlgButton(hDlg, IDC_STARTBTNFLAT, b);
	}
	
	b = GetMyRegLong(NULL, "StartButtonHide", FALSE);
	b = GetMyRegLong(m_section, "Hide", b);
	CheckDlgButton(hDlg, IDC_STARTBTNHIDE, b);
	
	b = GetMyRegLong(NULL, "StartMenuClock", FALSE);
	b = GetMyRegLong(m_section, "StartMenuClock", b);
	CheckDlgButton(hDlg, IDC_STARTMENUCLOCK, b);
	
	OnStartBtn(hDlg);
	
	m_bInit = TRUE;
}

/*------------------------------------------------
  Apply changes
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[MAX_PATH];
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong(m_section, "StartButton",
		IsDlgButtonChecked(hDlg, IDC_STARTBTN));
	
	GetDlgItemText(hDlg, IDC_FILESTART, s, MAX_PATH);
	SetMyRegStr(m_section, "Icon", s);
	
	GetDlgItemText(hDlg, IDC_CAPTIONSTART, s, 80);
	SetMyRegStr(m_section, "Caption", s);
	
	SetMyRegLong(m_section, "CaptionColor",
		CBGetItemData(hDlg, IDC_STARTBTNCOL,
			CBGetCurSel(hDlg, IDC_STARTBTNCOL)));
	
	CBGetLBText(hDlg, IDC_STARTBTNFONT,
		CBGetCurSel(hDlg, IDC_STARTBTNFONT), s);
	SetMyRegStr(m_section, "Font", s);
	
	GetDlgItemText(hDlg, IDC_STARTBTNFONTSIZE, s, 10);
	if(s[0]) SetMyRegStr(m_section, "FontSize", s);
	else SetMyRegStr(m_section, "FontSize", "9");
	
	SetMyRegLong(m_section, "Bold",
		IsDlgButtonChecked(hDlg, IDC_STARTBTNBOLD));
	SetMyRegLong(m_section, "Italic",
		IsDlgButtonChecked(hDlg, IDC_STARTBTNITALIC));
	
	
	SetMyRegLong(m_section, "UseBackBmp",
		IsDlgButtonChecked(hDlg, IDC_STARTBTNUSEBACK));
	
	GetDlgItemText(hDlg, IDC_STARTBTNBACKBMP, s, MAX_PATH);
	SetMyRegStr(m_section, "BackBmp", s);
	
	SetMyRegLong(m_section, "Flat",
		IsDlgButtonChecked(hDlg, IDC_STARTBTNFLAT));
	
	SetMyRegLong(m_section, "Hide",
		IsDlgButtonChecked(hDlg, IDC_STARTBTNHIDE));
	SetMyRegLong(m_section, "StartMenuClock",
		IsDlgButtonChecked(hDlg, IDC_STARTMENUCLOCK));
}

/*------------------------------------------------
   initizlize "Color" comoboxes
--------------------------------------------------*/
void InitColor(HWND hDlg)
{
	COLORREF cols[4], colDef;
	
	cols[0] = 0x80000000|COLOR_3DFACE;
	cols[1] = 0x80000000|COLOR_3DSHADOW;
	cols[2] = 0x80000000|COLOR_3DHILIGHT;
	cols[3] = 0x80000000|COLOR_BTNTEXT;
	
	colDef = GetMyRegLong(m_section, "CaptionColor",
		0x80000000 | COLOR_BTNTEXT);
	// common/combobox.c
	InitColorCombo(hDlg, IDC_STARTBTNCOL, cols, 4, colDef);
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
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitFont(HWND hDlg)
{
	char s[LF_FACESIZE];
	
	GetMyRegStr(m_section, "Font", s, LF_FACESIZE, "");
	if(s[0] == 0)
	{
		GetDefaultFontName(s, NULL);
	}
	
	// common/combobox.c
	InitFontNameCombo(hDlg, IDC_STARTBTNFONT, s);
}

/*------------------------------------------------
   set sizes to "Font Size" combo box
--------------------------------------------------*/
void OnFont(HWND hDlg, BOOL bInit)
{
	char s[160], size[10];
	int charset;
	int index;
	
	if(bInit)
		GetMyRegStr(m_section, "FontSize", size, 10, "9");
	else
		GetDlgItemText(hDlg, IDC_STARTBTNFONTSIZE, size, 10);
	
	CBGetLBText(hDlg, IDC_STARTBTNFONT,
		CBGetCurSel(hDlg, IDC_STARTBTNFONT), (LPARAM)s);
	charset = CBGetItemData(hDlg, IDC_STARTBTNFONT,
		CBGetCurSel(hDlg, IDC_STARTBTNFONT));
	
	// common/combobox.c
	InitFontSizeCombo(hDlg, IDC_STARTBTNFONTSIZE, s, charset);
	
	index = CBFindStringExact(hDlg, IDC_STARTBTNFONTSIZE, size);
	if(index == CB_ERR)
		SetDlgItemText(hDlg, IDC_STARTBTNFONTSIZE, size);
	else CBSetCurSel(hDlg, IDC_STARTBTNFONTSIZE, index);
}

/*------------------------------------------------
  "Customize Start button" is checked
--------------------------------------------------*/
void OnStartBtn(HWND hDlg)
{
	HWND hwnd;
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, IDC_STARTBTN);
	
	if(!b) OnUseBack(hDlg);
	
	hwnd = GetDlgItem(hDlg, IDC_STARTBTN);
	hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	while(hwnd)
	{
		EnableWindow(hwnd, b);
		if(GetDlgCtrlID(hwnd) == IDC_STARTBTNFLAT) break;
		
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	
	if(b) OnUseBack(hDlg);
	
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  "Background" is checked
--------------------------------------------------*/
void OnUseBack(HWND hDlg)
{
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, IDC_STARTBTNUSEBACK);
	EnableDlgItem(hDlg, IDC_STARTBTNBACKBMP, b);
	EnableDlgItem(hDlg, IDC_SANSHOSTARTBACKBMP, b);
	
	if(IsXPVisualStyle())
		EnableDlgItem(hDlg, IDC_STARTBTNFLAT, FALSE);
	else EnableDlgItem(hDlg, IDC_STARTBTNFLAT, !b);
}

/*------------------------------------------------
   "..." button
--------------------------------------------------*/
void OnSansho(HWND hDlg)
{
	char *filter = "Bitmap, Icon (*.bmp, *.ico)\0*.bmp;*.ico\0"
		"Executable (*.exe, *.dll)\0*.exe;*.dll\0"
		"All (*.*)\0*.*\0\0";
	char deffile[MAX_PATH], fname[MAX_PATH+10];
	char s[MAX_PATH+10], num[10];
	HFILE hf = HFILE_ERROR;
	char head[2];
	
	deffile[0] = 0;
	GetDlgItemText(hDlg, IDC_FILESTART, s, MAX_PATH);
	if(s[0])
	{
		parse(deffile, s, 0, MAX_PATH);
		parse(num, s, 1, 10);
		hf = _lopen(deffile, OF_READ);
	}
	if(hf != HFILE_ERROR)
	{
		_lread(hf, head, 2);
		_lclose(hf);
		
		if(head[0] == 'M' && head[1] == 'Z') // executable
		{
			// select icon : selecticon.c
			if(SelectIconInDLL(g_hInst, hDlg, deffile))
			{
				SetDlgItemText(hDlg, IDC_FILESTART, deffile);
				PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
				SendPSChanged(hDlg);
			}
			return;
		}
	}
	
	// select file : common/selectfile.c
	if(!SelectMyFile(g_hInst, hDlg, filter, 0, deffile, fname))
		return;
	
	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return;
	_lread(hf, head, 2);
	_lclose(hf);
	if(head[0] == 'M' && head[1] == 'Z') // executable
	{
		// select icon : selecticon.c
		if(!SelectIconInDLL(g_hInst, hDlg, fname)) return;
	}
	
	SetDlgItemText(hDlg, IDC_FILESTART, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  "..." button (choose color)
--------------------------------------------------*/
void OnChooseColor(HWND hDlg)
{
	// common/combobox.c
	if(ChooseColorWithCombo(hDlg, IDC_STARTBTNCOL))
	{
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		SendPSChanged(hDlg);
	}
}

/*------------------------------------------------
   "..." button (background image)
--------------------------------------------------*/
void OnSanshoBmp(HWND hDlg)
{
	char *filter = "Bitmap (*.bmp)\0*.bmp\0\0";
	char deffile[MAX_PATH], fname[MAX_PATH+10];
	
	deffile[0] = 0;
	GetDlgItemText(hDlg, IDC_STARTBTNBACKBMP, deffile, MAX_PATH);
	
	// select file : common/selectfile.c
	if(!SelectMyFile(g_hInst, hDlg, filter, 0, deffile, fname))
		return;
	
	SetDlgItemText(hDlg, IDC_STARTBTNBACKBMP, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

