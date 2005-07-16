/*-------------------------------------------------------------
  pagestartmenu.c : "Start menu" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

INT_PTR CALLBACK PageStartMenuProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void InitColor(HWND hDlg);
static void OnDrawItem(HWND hDlg, LPDRAWITEMSTRUCT pdis);
static void OnStartMenu(HWND hDlg);
static void OnBrowse(HWND hDlg);
static void OnChooseColor(HWND hDlg);
static void SetColorFromBmp(HWND hDlg, int idCombo, const char* fname);

static const char *m_section = "StartMenu";
static BOOL m_bInit = FALSE;
static BOOL m_bChanged = FALSE;

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
INT_PTR CALLBACK PageStartMenuProc(HWND hDlg, UINT message,
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
				case IDC_STARTMENU:
					OnStartMenu(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_STARTMENUBMP:
				case IDC_STARTMENUTRANS:
					if(code == EN_CHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_STARTMENUBROWSE:
					OnBrowse(hDlg);
					break;
				case IDC_STARTMENUCOLOR:
					if(code == CBN_SELCHANGE)
						SendPSChanged(hDlg);
					break;
				case IDC_STARTMENUCHOOSECOL:
					OnChooseColor(hDlg);
					break;
				case IDC_STARTMENUTILE:
					SendPSChanged(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "StartMenu"); break;
			}
			return TRUE;
		case WM_MEASUREITEM:
			// common/comobox.c
			OnMeasureItemColorCombo((LPMEASUREITEMSTRUCT)lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItem(hDlg, (LPDRAWITEMSTRUCT)lParam);
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
		g_bApplyStartMenu = TRUE;
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
	int alpha;
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "StartMenu", g_hfontDialog);
	
	b = GetMyRegLong(NULL, "StartMenu", FALSE);
	b = GetMyRegLong(m_section, "StartMenu", b);
	CheckDlgButton(hDlg, IDC_STARTMENU, b);
	
	GetMyRegStr(NULL, "StartMenuBmp", s, MAX_PATH, "");
	GetMyRegStr(m_section, "Bitmap", s2, MAX_PATH, s);
	SetDlgItemText(hDlg, IDC_STARTMENUBMP, s2);
	
	InitColor(hDlg);
	
	b = GetMyRegLong(NULL, "StartMenuTile", FALSE);
	b = GetMyRegLong(m_section, "Tile", b);
	CheckDlgButton(hDlg, IDC_STARTMENUTILE, b);
	
	OnStartMenu(hDlg);
	
	UpDown_SetBuddy(hDlg, IDC_STARTMENUTRANSSPIN, IDC_STARTMENUTRANS);
	UpDown_SetRange(hDlg, IDC_STARTMENUTRANSSPIN, 100, 0);
	
	alpha = GetMyRegLong(NULL, "AlphaStartMenu", 0);
	alpha = GetMyRegLong(m_section, "Alpha", alpha);
	if(alpha < 0) alpha = 0; else if(alpha > 100) alpha = 100;
	UpDown_SetPos(hDlg, IDC_STARTMENUTRANSSPIN, alpha);
	
	if(!(g_winver&WIN2000))
	{
		int i;
		for(i = IDC_STARTMENUTRANSLAB; i <= IDC_STARTMENUTRANSSPIN; i++)
			EnableDlgItem(hDlg, i, FALSE);
	}
	
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
	
	DelMyReg(NULL, "StartMenu");
	SetMyRegLong(m_section, "StartMenu",
		IsDlgButtonChecked(hDlg, IDC_STARTMENU));
	
	DelMyReg(NULL, "StartMenuBmp");
	GetDlgItemText(hDlg, IDC_STARTMENUBMP, s, MAX_PATH);
	SetMyRegStr(m_section, "Bitmap", s);
	
	DelMyReg(NULL, "StartMenuCol");
	SetMyRegLong(m_section, "Color",
		CBGetItemData(hDlg, IDC_STARTMENUCOLOR,
			CBGetCurSel(hDlg, IDC_STARTMENUCOLOR)));
	
	DelMyReg(NULL, "StartMenuTile");
	SetMyRegLong(m_section, "Tile",
		IsDlgButtonChecked(hDlg, IDC_STARTMENUTILE));
	
	DelMyReg(NULL, "AlphaStartMenu");
	SetMyRegLong(m_section, "Alpha",
		UpDown_GetPos(hDlg, IDC_STARTMENUTRANSSPIN));
}

/*------------------------------------------------
   initizlize "Color" comoboxes
--------------------------------------------------*/
void InitColor(HWND hDlg)
{
	COLORREF colDef;
	
	colDef = GetMyRegLong(NULL, "StartMenuCol",
		0x80000000 | COLOR_BTNTEXT);
	colDef = GetMyRegLong(m_section, "Color", colDef);
	InitColorCombo(hDlg, IDC_STARTMENUCOLOR, NULL, 0, colDef);
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
   "Customize Start menu" is checked
--------------------------------------------------*/
void OnStartMenu(HWND hDlg)
{
	HWND hwnd = GetDlgItem(hDlg, IDC_STARTMENU);
	BOOL b = IsDlgButtonChecked(hDlg, IDC_STARTMENU);
	
	while((hwnd = GetWindow(hwnd, GW_HWNDNEXT)) != NULL)
	{
		EnableWindow(hwnd, b);
		if(GetDlgCtrlID(hwnd) == IDC_STARTMENUCHOOSECOL) break;
	}
}

/*------------------------------------------------
  browse bitmap file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	const char *filter = "Bitmap (*.bmp)\0*.bmp\0\0";
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_STARTMENUBMP, deffile, MAX_PATH);
	
	// select file : common/selectfile.c
	if(SelectMyFile(g_hInst, hDlg, filter, 0, deffile, fname))
	{
		SetColorFromBmp(hDlg, IDC_STARTMENUCOLOR, fname);
		
		SetDlgItemText(hDlg, IDC_STARTMENUBMP, fname);
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		SendPSChanged(hDlg);
	}
}

/*------------------------------------------------
  choose color
--------------------------------------------------*/
void OnChooseColor(HWND hDlg)
{
	// common/combobox.c
	if(ChooseColorWithCombo(hDlg, IDC_STARTMENUCOLOR))
	{
		PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
		SendPSChanged(hDlg);
	}
}

/*------------------------------------------------
   Select "Color" combo box automatically.
--------------------------------------------------*/
#define WIDTHBYTES(i) ((i+31)/32*4)

void SetColorFromBmp(HWND hDlg, int idCombo, const char* fname)
{
	HANDLE hf;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	int numColors;
	BYTE pixel[3];
	COLORREF col;
	int i, index;
	DWORD dwRead;
	
	hf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hf == INVALID_HANDLE_VALUE) return;
	
	if( !ReadFile(hf, &bmfh, sizeof(bmfh), &dwRead, NULL) ||
		bmfh.bfType != *(WORD*)"BM" ||
		!ReadFile(hf, &bmih, sizeof(bmih), &dwRead, NULL) ||
		bmih.biSize < sizeof(bmih) ||
		bmih.biCompression != BI_RGB ||
		!(bmih.biBitCount <= 8 || bmih.biBitCount == 24))
	{
		CloseHandle(hf); return;
	}
	numColors = bmih.biClrUsed;
	if(numColors == 0)
	{
		if(bmih.biBitCount <= 8) numColors = 1 << bmih.biBitCount;
	}
	if(numColors > 0 &&
		SetFilePointer(hf, bmfh.bfOffBits, NULL, FILE_BEGIN)
			== INVALID_SET_FILE_POINTER)
	{
		CloseHandle(hf); return;
	}
	if(SetFilePointer(hf,
			WIDTHBYTES(bmih.biWidth*bmih.biBitCount)*(bmih.biHeight-1),
			NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER ||
		!ReadFile(hf, pixel, sizeof(pixel), &dwRead, NULL))
	{
		CloseHandle(hf); return;
	}
	if(bmih.biBitCount < 24)
	{
		index = -1;
		if(bmih.biBitCount == 8) index = pixel[0];
		else if(bmih.biBitCount == 4)
			index = (pixel[0] & 0xF0) >> 4;
		else if(bmih.biBitCount == 1)
			index = (pixel[0] & 0x80) >> 7;
		if(SetFilePointer(hf, sizeof(bmfh)+bmih.biSize+sizeof(RGBQUAD)*index,
			NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!ReadFile(hf, pixel, sizeof(pixel), &dwRead, NULL))
		{
			index = -1;
		}
	}
	CloseHandle(hf);
	if(index == -1) return;
	col = RGB(pixel[2], pixel[1], pixel[0]);
	
	for(i = 0; i < 16; i++)
	{
		if(col == (COLORREF)CBGetItemData(hDlg, idCombo, i)) break;
	}
	if(i == 16)
	{
		if(CBGetCount(hDlg, idCombo) == 16)
			CBAddString(hDlg, idCombo, col);
		else
			CBSetItemData(hDlg, idCombo, 16, col);
	}
	CBSetCurSel(hDlg, idCombo, i);
}
