/*-------------------------------------------------------------
  combobox.c : combo box for color, font, locale
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* -------------Owner-drawn Color combobox -------------------------------*/

/*------------------------------------------------
   initialize
--------------------------------------------------*/
void InitColorCombo(HWND hDlg, int idCombo,
	const COLORREF *pColAdd, int nAdd, COLORREF colDef)
{
	int i, count;
	
	// Windows 16 colors 
	const COLORREF rgb[16] = {
		RGB(0,0,0),       RGB(128,0,0),   RGB(0,128,0),   RGB(128,128,0),
		RGB(0,0,128),     RGB(128,0,128), RGB(0,128,128), RGB(192,192,192),
		RGB(128,128,128), RGB(255,0,0),   RGB(0,255,0),   RGB(255,255,0),
		RGB(0,0,255),     RGB(255,0,255), RGB(0,255,255), RGB(255,255,255),
	};
	
	for(i = 0; i < 16; i++) // 16 colors
		CBAddString(hDlg, idCombo, rgb[i]);
	
	for(i = 0; i < nAdd && pColAdd; i++)
		CBAddString(hDlg, idCombo, pColAdd[i]);
	
	count = CBGetCount(hDlg, idCombo);
	for(i = 0; i < count; i++)
	{
		if(colDef == (COLORREF)CBGetItemData(hDlg, idCombo, i))
			break;
	}
	
	if(i == count)
		CBAddString(hDlg, idCombo, colDef);
	
	CBSetCurSel(hDlg, idCombo, i);
}

/*------------------------------------------------
  WM_MEASUREITEM message
--------------------------------------------------*/
void OnMeasureItemColorCombo(LPMEASUREITEMSTRUCT pmis)
{
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
}

/*------------------------------------------------
  WM_DRAWITEM message
--------------------------------------------------*/
void OnDrawItemColorCombo(LPDRAWITEMSTRUCT pdis, char (*pTexts)[80])
{
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	int y;
	
	if(IsWindowEnabled(pdis->hwndItem))
	{
		col = (COLORREF)pdis->itemData;
		if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	}
	else col = GetSysColor(COLOR_3DFACE);
	
	switch(pdis->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
		{
			char *p = NULL;
			
			if(pTexts)
			{
				if(pdis->itemData == (0x80000000|COLOR_3DFACE))
					p = pTexts[0];
				else if(pdis->itemData == (0x80000000|COLOR_3DSHADOW))
					p = pTexts[1];
				else if(pdis->itemData == (0x80000000|COLOR_3DHILIGHT))
					p = pTexts[2];
				else if(pdis->itemData == (0x80000000|COLOR_BTNTEXT))
					p = pTexts[3];
			}
			
			hbr = CreateSolidBrush(col);
			FillRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
			
			// print color names
			if(p)
			{
				int len;
				wchar_t *ws;
				
				SetBkMode(pdis->hDC, TRANSPARENT);
				GetTextMetrics(pdis->hDC, &tm);
				if(pdis->itemID == 19)
					SetTextColor(pdis->hDC, RGB(255,255,255));
				else
					SetTextColor(pdis->hDC, RGB(0,0,0));
				y = (pdis->rcItem.bottom - pdis->rcItem.top - tm.tmHeight)/2;
				
				len = MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
				ws = malloc(sizeof(wchar_t) * len);
				if(ws)
				{
					MultiByteToWideChar(CP_ACP, 0, p, -1, ws, len);
					TextOutW(pdis->hDC,
						pdis->rcItem.left + 4, pdis->rcItem.top + y,
						ws, len - 1);
					free(ws);
				}
			}
			if(!(pdis->itemState & ODS_FOCUS)) break;
		}
		case ODA_FOCUS:
		{
			if(pdis->itemState & ODS_FOCUS)
				hbr = CreateSolidBrush(0);
			else
				hbr = CreateSolidBrush(col);
			FrameRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
			break;
		}
	}
}

/*--------------------------------------------------------
  open a choose color dialog and set color to combobox
----------------------------------------------------------*/
BOOL ChooseColorWithCombo(HWND hDlg, int idCombo)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	int i;
	
	col = CBGetItemData(hDlg, idCombo, CBGetCurSel(hDlg, idCombo));
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	
	for(i = 0; i < 16; i++) colarray[i] = RGB(255,255,255);
	
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hDlg;
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc)) return FALSE;
	
	for(i = 0; i < CBGetCount(hDlg, idCombo); i++)
	{
		if(cc.rgbResult == (COLORREF)CBGetItemData(hDlg, idCombo, i))
			break;
	}
	if(i == CBGetCount(hDlg, idCombo))
		CBAddString(hDlg, idCombo, cc.rgbResult);
	
	CBSetCurSel(hDlg, idCombo, i);
	
	return TRUE;
}

/* ---------------------- Font combobox -------------------------------*/

/* Globals */

void InitFontNameCombo(HWND hDlg, int idCombo, const char* deffont);
void InitFontSizeCombo(HWND hDlg, int idCombo,
	const char *fontname, int charset);

/* Statics */

static BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
static BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
static int m_nFontSizes[] = 
	{ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
static int m_logpixelsy;

/*------------------------------------------------
   initialize a "Font" combobox
--------------------------------------------------*/
void InitFontNameCombo(HWND hDlg, int idCombo, const char* deffont)
{
	HDC hdc;
	LOGFONT lf;
	HWND hcombo;
	int i;
	
	CBResetContent(hDlg, idCombo);
	
	hdc = GetDC(NULL);
	
	// Enumerate fonts and set in the combo box
	memset(&lf, 0, sizeof(LOGFONT));
	hcombo = GetDlgItem(hDlg, idCombo);
	lf.lfCharSet = (BYTE)GetTextCharset(hdc);  // MS UI Gothic, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = OEM_CHARSET;   // Small Fonts, Terminal...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	ReleaseDC(NULL, hdc);
	
	i = CBFindStringExact(hDlg, idCombo, deffont);
	if(i == LB_ERR) i = 0;
	CBSetCurSel(hDlg, idCombo, i);
}

/*------------------------------------------------
   initialize a "Font Size" combobox
--------------------------------------------------*/
void InitFontSizeCombo(HWND hDlg, int idCombo,
	const char *fontname, int charset)
{
	HDC hdc;
	LOGFONT lf;
	
	CBResetContent(hDlg, idCombo);
	
	hdc = GetDC(NULL);
	m_logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	// enumerate font size
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, fontname);
	lf.lfCharSet = (BYTE)charset;
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumSizeProcEx,
		(LPARAM)GetDlgItem(hDlg, idCombo), 0);
	
	ReleaseDC(NULL, hdc);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo)
{
	// if(FontType & RASTER_FONTTYPE) return 1;
	if(pelf->elfLogFont.lfFaceName[0] != '@' && 
		SendMessage((HWND)hCombo, CB_FINDSTRINGEXACT, 0, 
			(LPARAM)pelf->elfLogFont.lfFaceName) == LB_ERR)
	{
		int index;
		index = (int)SendMessage((HWND)hCombo, CB_ADDSTRING,
			0, (LPARAM)pelf->elfLogFont.lfFaceName);
		if(index >= 0)
			SendMessage((HWND)hCombo, CB_SETITEMDATA,
				index, (LPARAM)pelf->elfLogFont.lfCharSet);
	}
	return 1;
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font size in the combo box.
--------------------------------------------------*/
BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo)
{
	char s[80];
	int num, i, count;
	
	// TrueType font, or not TrueType nor Raster font
	if((FontType & TRUETYPE_FONTTYPE) || 
		!( (FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE) ))
	{
		for (i = 0; i < 16; i++)
		{
			wsprintf(s, "%d", m_nFontSizes[i]);
			SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)s);
		}
		return FALSE;
	}
	
	// Other case
	num = MulDiv((lpntm->ntmTm.tmHeight - lpntm->ntmTm.tmInternalLeading),
		72, m_logpixelsy);
	count = (int)SendMessage((HWND)hCombo, CB_GETCOUNT, 0, 0);
	for(i = 0; i < count; i++)
	{
		SendMessage((HWND)hCombo, CB_GETLBTEXT, i, (LPARAM)s);
		if(num == atoi(s)) return TRUE;
		else if(num < atoi(s))
		{
			wsprintf(s, "%d", num);
			SendMessage((HWND)hCombo, CB_INSERTSTRING, i, (LPARAM)s);
			return TRUE;
		}
	}
	wsprintf(s, "%d", num);
	SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)s);
	
	return TRUE;
}

/* ---------------------- Locale combobox ------------------------------*/

/* Globals */

void InitLocaleCombo(HWND hDlg, int idCombo, int deflang);

/* Statics */

static BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString);
static HWND m_hDlg;
static int  m_idComboLocale;
static int  m_codepage;

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void InitLocaleCombo(HWND hDlg, int idCombo, int deflang)
{
	int i, count;
	
	m_hDlg = hDlg; m_idComboLocale = idCombo;
	m_codepage = GetCodePage(GetUserDefaultLangID());
	
	EnumSystemLocales(EnumLocalesProc, LCID_INSTALLED);
	CBSetCurSel(hDlg, idCombo, 0);
	count = CBGetCount(hDlg, idCombo);
	for(i = 0; i < count; i++)
	{
		int x;
		x = CBGetItemData(hDlg, idCombo, i);
		if(x == deflang)
		{
			CBSetCurSel(hDlg, idCombo, i); break;
		}
	}
}

/*------------------------------------------------
  for EnumSystemLocales function
--------------------------------------------------*/
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	char s[81];
	int x, index;
	
	x = atox(lpLocaleString);
	if(MyGetLocaleInfoA(x, m_codepage, LOCALE_SLANGUAGE, s, 80) > 0)
		index = CBAddString(m_hDlg, m_idComboLocale, (LPARAM)s);
	else
		index = CBAddString(m_hDlg, m_idComboLocale, (LPARAM)lpLocaleString);
	CBSetItemData(m_hDlg, m_idComboLocale, index, x);
	return TRUE;
}

