/*-------------------------------------------------------------
  pagetooltip.c : "Cuckoo" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

INT_PTR CALLBACK PageTooltipProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnUseTip1(HWND hDlg);
static void InitFont(HWND hDlg);
static void OnFont(HWND hDlg, BOOL bInit);
static void OnBrowse(HWND hDlg);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;
static HFONT m_hfontb, m_hfonti;

static char *m_section = "Tooltip";

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageTooltipProc(HWND hDlg, UINT message,
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
				case IDC_TOOLTIPBROWSE:
					OnBrowse(hDlg);
					break;
				case IDC_TOOLTIPSTYLE1:
				case IDC_TOOLTIPSTYLE2:
				case IDC_TOOLTIPBOLD:
				case IDC_TOOLTIPITALIC:
					SendPSChanged(hDlg);
					break;
				case IDC_TOOLTIPFONT:
				case IDC_TOOLTIPFONTSIZE:
					if(code == CBN_SELCHANGE || code == CBN_EDITCHANGE)
					{
						if(id == IDC_TOOLTIPFONT) OnFont(hDlg, FALSE);
						SendPSChanged(hDlg);
					}
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
	LOGFONT logfont;
	
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
	
	InitFont(hDlg);
	OnFont(hDlg, TRUE);
	
	b = GetMyRegLong(NULL, "TipBold", FALSE);
	b = GetMyRegLong(m_section, "Bold", b);
	CheckDlgButton(hDlg, IDC_TOOLTIPBOLD, b);
	b = GetMyRegLong(NULL, "TipItalic", FALSE);
	b = GetMyRegLong(m_section, "Italic", b);
	CheckDlgButton(hDlg, IDC_TOOLTIPITALIC, b);
	
	m_hfontb = m_hfonti = NULL;
	if(g_hfontDialog)
	{
		GetObject(g_hfontDialog, sizeof(LOGFONT), &logfont);
		logfont.lfWeight = FW_BOLD;
		m_hfontb = CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg, IDC_TOOLTIPBOLD,
			WM_SETFONT, (WPARAM)m_hfontb, 0);
		
		logfont.lfWeight = FW_NORMAL;
		logfont.lfItalic = 1;
		m_hfonti = CreateFontIndirect(&logfont);
		SendDlgItemMessage(hDlg, IDC_TOOLTIPITALIC,
			WM_SETFONT, (WPARAM)m_hfonti, 0);
		
		GetDlgItemText(hDlg, IDC_TOOLTIPITALIC, s, 77);
		strcat(s, "  ");
		SetDlgItemText(hDlg, IDC_TOOLTIPITALIC, s);
	}
	
	OnUseTip1(hDlg);
	
	m_bInit = TRUE;
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[BUFSIZE_TOOLTIP];
	
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
	
	SetMyRegLong(m_section, "DispTime",
		UpDown_GetPos(hDlg,IDC_TOOLTIPTIMESPIN));
	DelMyReg(NULL, "TipDispTime");
	
	CBGetLBText(hDlg, IDC_TOOLTIPFONT,
		CBGetCurSel(hDlg, IDC_TOOLTIPFONT), s);
	SetMyRegStr(m_section, "Font", s);
	DelMyReg(NULL, "TipFont");
	
	GetDlgItemText(hDlg, IDC_TOOLTIPFONTSIZE, s, 10);
	if(s[0]) SetMyRegStr(m_section, "FontSize", s);
	else SetMyRegStr(m_section, "FontSize", "9");
	DelMyReg(NULL, "TipFontSize");
	
	SetMyRegLong(m_section, "Bold",
		IsDlgButtonChecked(hDlg, IDC_TOOLTIPBOLD));
	DelMyReg(NULL, "TipBold");
	SetMyRegLong(m_section, "Italic",
		IsDlgButtonChecked(hDlg, IDC_TOOLTIPITALIC));
	DelMyReg(NULL, "TipItalic");
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

/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitFont(HWND hDlg)
{
	char s[80], temp[80];
	
	GetMyRegStr(NULL, "TipFont", temp, 80, "");
	GetMyRegStr(m_section, "Font", s, 80, temp);
	if(s[0] == 0)
	{
		GetDefaultFontName(s, NULL);
	}
	
	// common/combobox.c
	InitFontNameCombo(hDlg, IDC_TOOLTIPFONT, s);
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
	{
		char temp[10];
		GetMyRegStr(NULL, "TipFontSize", temp, 10, "9");
		GetMyRegStr(m_section, "FontSize", size, 10, temp);
	}
	else
		GetDlgItemText(hDlg, IDC_TOOLTIPFONTSIZE, size, 10);
	
	CBGetLBText(hDlg, IDC_TOOLTIPFONT,
		CBGetCurSel(hDlg, IDC_TOOLTIPFONT), (LPARAM)s);
	charset = CBGetItemData(hDlg, IDC_TOOLTIPFONT,
		CBGetCurSel(hDlg, IDC_TOOLTIPFONT));
	
	// common/combobox.c
	InitFontSizeCombo(hDlg, IDC_TOOLTIPFONTSIZE, s, charset);
	
	index = CBFindStringExact(hDlg, IDC_TOOLTIPFONTSIZE, size);
	if(index == CB_ERR)
		SetDlgItemText(hDlg, IDC_TOOLTIPFONTSIZE, size);
	else CBSetCurSel(hDlg, IDC_TOOLTIPFONTSIZE, index);
}

/*------------------------------------------------
  "..." button - select a text file for tooltip
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char *filter = "text file (*.txt)\0*.txt\0\0";
	char temp[MAX_PATH], deffile[MAX_PATH], fname[MAX_PATH+10];
	
	deffile[0] = 0;
	GetDlgItemText(hDlg, IDC_TOOLTIP, temp, MAX_PATH);
	if(strncmp(temp, "file:", 5) == 0)
		strcpy(deffile, temp + 5);
	
	// select file : common/selectfile.c
	if(!SelectMyFile(g_hInst, hDlg, filter, 0, deffile, fname))
		return;
	
	strcpy(temp, "file:");
	strcat(temp, fname);
	SetDlgItemText(hDlg, IDC_TOOLTIP, temp);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}


