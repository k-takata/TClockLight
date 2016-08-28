/*-------------------------------------------------------------
  draw.c : drawing the clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
#include "newapi.h"
#include <math.h>

/* Globals */

void LoadDrawingSetting(HWND hwnd);
void ClearDrawing(void);
void ClearClockDC(void);
HDC GetClockBackDC(void);
void OnPaint(HWND hwnd, HDC hdc, const SYSTEMTIME* pt);
LRESULT OnCalcRect(HWND hwnd);
void CreateClockDC(HWND hwnd);

BOOL g_bFitClock = FALSE; // Fit clock to tray

/* Statics */

static LRESULT CalcRect(HWND hwnd, int *textwidth, int *textheight);
static void GetClockTextSize(HDC hdc, const TEXTMETRIC* ptm,
	const wchar_t* str, int *wout, int *hout);
static void DrawClock(HWND hwnd, HDC hdc, const SYSTEMTIME* pt);
static void CopyClockBack(HWND hwnd, HDC hdcDest, HDC hdcSrc, int w, int h);
static void FillClock(HWND hwnd, HDC hdc, const RECT *prc);
static void GradientFillClock(HDC hdc, const RECT* prc,
	COLORREF col1, COLORREF col2, DWORD grad);

static HDC      m_hdcClock = NULL;      // offscreen DC
static HBITMAP  m_hbmpClock = NULL;     // BMP for offscreen DC
static HDC      m_hdcClockBack = NULL;  // offscreen DC of background
static HBITMAP  m_hbmpClockBack = NULL; // BMP for offscreen DC of background
static HFONT    m_hFont = NULL;         // font handle
static BOOL     m_fillbackcolor = TRUE; // fill background
static COLORREF m_colback, m_colback2, m_colfore; // colors
static ULONG    m_grad = GRADIENT_FILL_RECT_H; // GradientFill direction
static BOOL     m_bFillTray;            // Paint tray background
#if TC_ENABLE_CLOCKDECORATION
static int      m_nClockDecoration;     // decoration 0:none 1:shadow 2:border
static COLORREF m_colShadow;            // color of shadow or border
static int      m_nShadowRange;         //
#endif
static int m_dwidth = 0, m_dheight = 0; // to add pixels to width and height
static int      m_dvpos = 0;            // to add pixels to vertical position
static int      m_dlineheight = 0;      // to add pixels to line height
static int      m_nTextPos = 0;         // alignment
static int      m_ClockWidth = -1;      // to save clock width

// Analog Clock
#if TC_ENABLE_ANALOGCLOCK
static void LoadAnalogClockSetting(void);
static BOOL InitAnalogClock(HWND hwnd, HDC hdc);
static void ClearAnalogClock(void);
static void DrawAnalogClock(HWND hwnd, HDC hdcClock, const SYSTEMTIME *pt,
	int wclock, int hclock);

static HDC      m_hdcAclk, m_hdcBack, m_hdcMask;
static HBITMAP  m_hbmAclk, m_hbmBack, m_hbmMask;
static HPEN     m_hpenHour, m_hpenMin;
static int      m_lastHour = -1, m_lastMin = -1;
static BOOL     m_useAnalogClock;
static COLORREF m_colHourHand, m_colMinHand;
static BOOL     m_bHourHandBold, m_bMinHandBold;
static int      m_nAClockPos;
static int      m_nAClockHPos, m_nAClockVPos;
static int      m_nAClockSize;
static char     m_fname[MAX_PATH];

static const COLORREF MASK_COLOR = RGB(0xff, 0x00, 0xff);
static const int ACLOCK_DEFSIZE = 18;

#define PI_30  0.10471975511965977462
#define PI_360 0.0087266462599716478846
#endif /* TC_ENABLE_ANALOGCLOCK */


/*------------------------------------------------
  read settings and initialize
--------------------------------------------------*/
void LoadDrawingSetting(HWND hwnd)
{
	char fontname[LF_FACESIZE];
	int size, langid, codepage;
	LONG weight, italic;
	
	/* ------- colors ------------- */
	
	m_fillbackcolor = GetMyRegLong(NULL, "UseBackColor",
		g_bVisualStyle? FALSE: TRUE);
	
	m_colback = GetMyRegLong(NULL, "BackColor",
		0x80000000 | COLOR_3DFACE);
	
	m_colback2 = m_colback;
	if(GetMyRegLong(NULL, "UseBackColor2", TRUE))
		m_colback2 = GetMyRegLong(NULL, "BackColor2", m_colback);
	m_grad = GetMyRegLong(NULL, "GradDir", GRADIENT_FILL_RECT_H);
	
	m_bFillTray = FALSE;
	if(m_fillbackcolor)
		m_bFillTray = GetMyRegLong(NULL, "FillTray", FALSE);
	
	m_colfore = GetMyRegLong(NULL, "ForeColor", 
		0x80000000 | COLOR_BTNTEXT);
	
#if TC_ENABLE_CLOCKDECORATION
	m_nClockDecoration = GetMyRegLong(NULL, "ClockDecoration", 0);
	m_colShadow = GetMyRegLong(NULL, "ShadowColor", 0);
	m_nShadowRange = GetMyRegLong(NULL, "ShadowRange", 1);
#endif
	
	/* ------- font ------------- */
	
	GetMyRegStr(NULL, "Font", fontname, LF_FACESIZE, "");
	
	if(fontname[0] == 0)
	{
		GetDefaultFontName(fontname, "System");
	}
	
	size = GetMyRegLong(NULL, "FontSize", 9);
	if(size == 0) size = 9;
	weight = GetMyRegLong(NULL, "Bold", 0);
	if(weight) weight = FW_BOLD;
	else weight = 0;
	italic = GetMyRegLong(NULL, "Italic", 0);
	
	if(m_hFont) DeleteObject(m_hFont);
	
	langid = GetMyRegLong(NULL, "Locale", (int)GetUserDefaultLangID());
	codepage = GetCodePage(langid);
	
	// font.c
	m_hFont = CreateMyFont(fontname, size, weight, italic, codepage);
	
	/* ------- size and position ------------- */
	
	m_nTextPos = GetMyRegLong(NULL, "TextPos", 0);
	m_dheight = (int)(short)GetMyRegLong(NULL, "ClockHeight", 0);
	m_dwidth = (int)(short)GetMyRegLong(NULL, "ClockWidth", 0);
	m_dvpos = (int)(short)GetMyRegLong(NULL, "VertPos", 0);
	m_dlineheight = (int)(short)GetMyRegLong(NULL, "LineHeight", 0);
	
	g_bFitClock = GetMyRegLong(NULL, "FitClock", TRUE);
	
	m_ClockWidth = -1;
	
#if TC_ENABLE_ANALOGCLOCK
	LoadAnalogClockSetting();	// Analog Clock
#endif
}

/*------------------------------------------------
  clear up resouces
--------------------------------------------------*/
void ClearDrawing(void)
{
	if(m_hFont) DeleteObject(m_hFont);
	m_hFont = NULL;
	
	ClearClockDC();
}

/*------------------------------------------------
  delete offscreen DC and bitmap
--------------------------------------------------*/
void ClearClockDC(void)
{
	m_ClockWidth = -1;
	
	if(m_hdcClock) DeleteDC(m_hdcClock); 
	m_hdcClock = NULL;
	if(m_hbmpClock) DeleteObject(m_hbmpClock);
	m_hbmpClock = NULL;
	
	if(m_hdcClockBack) DeleteDC(m_hdcClockBack); 
	m_hdcClockBack = NULL;
	if(m_hbmpClockBack) DeleteObject(m_hbmpClockBack);
	m_hbmpClockBack = NULL;
	
#if TC_ENABLE_ANALOGCLOCK
	ClearAnalogClock();	// Analog Clock
#endif
}

/*------------------------------------------------
   return m_hdcClockBack - used in traynotify.c
--------------------------------------------------*/
HDC GetClockBackDC(void)
{
	return m_hdcClockBack;
}

/*------------------------------------------------
  WM_PAINT message
--------------------------------------------------*/
void OnPaint(HWND hwnd, HDC hdc, const SYSTEMTIME* pt)
{
	DrawClock(hwnd, hdc, pt);
}

/*------------------------------------------------
  return size of clock
  high-order word: height, low-order word: width
--------------------------------------------------*/
LRESULT CalcRect(HWND hwnd, int *textwidth, int *textheight)
{
	TEXTMETRIC tm;
	HDC hdc;
	HFONT hOldFont = NULL;
	wchar_t s[BUFSIZE_FORMAT+BUFSIZE_DISP*2];
	wchar_t *p;
	int wclock, hclock;
	
	if(!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE))
		return 0;
	
	if(g_sdisp2[0]) wcscpy(s, g_sdisp2);
	else if(g_sdisp1[0]) wcscpy(s, g_sdisp1);
	else MakeFormat(s, NULL, NULL, BUFSIZE_FORMAT);
	
	if(g_scat1[0]) wcscat(s, g_scat1);
	if(g_scat2[0]) wcscat(s, g_scat2);
	
	p = s;
	while(*p != L'\0')
	{
		// Replace all spaces to '0' to avoid changing the width when
		// number of digits is changed.  E.g.: ' 9:59' -> '10:00'
		if(*p == L' ')
			*p = L'0';
		++p;
	}
	
	hdc = GetDC(hwnd);
	if(m_hFont) hOldFont = SelectObject(hdc, m_hFont);
	GetTextMetrics(hdc, &tm);
	
	GetClockTextSize(hdc, &tm, s, &wclock, &hclock);
	
	if(hOldFont) SelectObject(hdc, hOldFont);
	ReleaseDC(hwnd, hdc);
	
	if(textwidth != NULL) *textwidth = wclock;
	if(textheight != NULL) *textheight = hclock;
	
	wclock += tm.tmAveCharWidth * 2 + m_dwidth;
	hclock += (tm.tmHeight - tm.tmInternalLeading) / 2 + m_dheight;
	
#if TC_ENABLE_CLOCKDECORATION
	if (m_nClockDecoration == 1)
	{
		wclock += m_nShadowRange;
		hclock += m_nShadowRange;
	}
#endif
#if TC_ENABLE_ANALOGCLOCK
	if (m_useAnalogClock && (m_nAClockPos == 1 || m_nAClockPos == 2))
	{
		wclock += m_nAClockSize;
	}
#endif
	
	if(hclock < 4) hclock = 4;
	
	if(wclock > m_ClockWidth) m_ClockWidth = wclock;
	
	if(g_bFitClock)
	{
		RECT rcTray, rcTaskbar;
		
		GetWindowRect(GetParent(hwnd), &rcTray);
		GetClientRect(GetParent(GetParent(hwnd)), &rcTaskbar);
		
		// horizontal task bar
		if(rcTaskbar.right - rcTaskbar.left >
			rcTaskbar.bottom - rcTaskbar.top)
		{
			hclock = rcTray.bottom - rcTray.top;
		}
		// vertical task bar
		else
			wclock = rcTray.right - rcTray.left;
	}
	
	return MAKELONG(wclock, hclock);
}

LRESULT OnCalcRect(HWND hwnd)
{
	return CalcRect(hwnd, NULL, NULL);
}

/*------------------------------------------------
   create offscreen buffer
--------------------------------------------------*/
void CreateClockDC(HWND hwnd)
{
	RECT rc;
	COLORREF col;
	HDC hdc;
	
	ClearClockDC();
	
	if(g_bNoClock) return;
	
	GetClientRect(hwnd, &rc);
	
	hdc = GetDC(NULL);
	
	if(!CreateOffScreenDC(hdc, &m_hdcClock, &m_hbmpClock,
		rc.right, rc.bottom)) // dllutl.c
	{
		ReleaseDC(NULL, hdc);
		return;
	}
	
	SelectObject(m_hdcClock, m_hFont);
	SetBkMode(m_hdcClock, TRANSPARENT);
	
	if(m_nTextPos == 1)
		SetTextAlign(m_hdcClock, TA_LEFT|TA_TOP);
	else if(m_nTextPos == 2)
		SetTextAlign(m_hdcClock, TA_RIGHT|TA_TOP);
	else
		SetTextAlign(m_hdcClock, TA_CENTER|TA_TOP);
	
	col = m_colfore;
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	SetTextColor(m_hdcClock, col);
	
	/* ------- background -------- */
	
	if(m_bFillTray)
	{
		hwnd = GetParent(hwnd);
		GetClientRect(hwnd, &rc);
	}
	
	if(!CreateOffScreenDC(hdc, &m_hdcClockBack, &m_hbmpClockBack,
		rc.right, rc.bottom)) // dllutl.c
	{
		ClearClockDC();
		ReleaseDC(NULL, hdc);
		return;
	}
	FillClock(hwnd, m_hdcClockBack, &rc);
	
#if TC_ENABLE_ANALOGCLOCK
	InitAnalogClock(hwnd, hdc);	// Analog Clock
#endif
	
	ReleaseDC(NULL, hdc);
}

/*------------------------------------------------
  calculate width and height of clock text
--------------------------------------------------*/
void GetClockTextSize(HDC hdc, const TEXTMETRIC* ptm,
	const wchar_t* str, int *wout, int *hout)
{
	int w, h;
	int heightFont;
	const wchar_t *p, *sp, *ep;
	SIZE sz;
	
	p = str; w = 0; h = 0;
	
	heightFont = ptm->tmHeight - ptm->tmInternalLeading;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		ep = p;
		if(*p == 0x0d) p += 2;
		
		if(GetTextExtentPoint32W(hdc, sp, (int)(ep - sp), &sz) == 0)
			sz.cx = (int)(ep - sp) * ptm->tmAveCharWidth;
		if(w < sz.cx) w = sz.cx;
		h += heightFont;
		
		if(*p) h += 2 + m_dlineheight;
	}
	
	*wout = w; *hout = h;
}

/*------------------------------------------------
  draw the clock
--------------------------------------------------*/
void DrawClock(HWND hwnd, HDC hdc, const SYSTEMTIME* pt)
{
	RECT rcClock;
	TEXTMETRIC tm;
	int x, y, wclock, hclock, wtext, htext;
	int len;
	int sha = 0, ana = 0;
	wchar_t s[BUFSIZE_FORMAT+BUFSIZE_DISP*2],
		*p, *sp, *ep;
	DWORD dwRop = SRCCOPY;
	COLORREF textcolor = 0;
	BOOL aero = FALSE;
#if TC_ENABLE_CLOCKDECORATION
	COLORREF colText, colShadow;
#endif
	DWORD size;
	
	if(!m_hdcClock)
	{
		CreateClockDC(hwnd);
		if(!m_hdcClock) return;
	}
	
	GetClientRect(hwnd, &rcClock);
	wclock = rcClock.right;
	hclock = rcClock.bottom;
	
	size = (DWORD)CalcRect(hwnd, &wtext, &htext);
	if(wclock < LOWORD(size) || hclock < HIWORD(size))
	{
		SetWindowPos(hwnd, NULL, 0, 0, LOWORD(size), HIWORD(size),
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		InvalidateRect(hwnd, NULL, FALSE);
		return;
	}
	
	if(g_nBlink > 0 && (g_nBlink % 2) == 0) dwRop = NOTSRCCOPY;
	
	aero = IsVistaAero();
	if(!m_fillbackcolor && (dwRop == SRCCOPY) && aero)
	{
		HBRUSH hbr = GetStockBrush(BLACK_BRUSH);
		FillRect(m_hdcClock, &rcClock, hbr);
	}
	else
	{
		// copy m_hdcClockBack to m_hdcClock
		CopyClockBack(hwnd, m_hdcClock, m_hdcClockBack, wclock, hclock);
	}
	
	if(GetFocus() == hwnd)
		DrawFocusRect(m_hdcClock, &rcClock);
	
	if(g_sdisp2[0]) wcscpy(s, g_sdisp2);
	else if(g_sdisp1[0]) wcscpy(s, g_sdisp1);
	else MakeFormat(s, pt, NULL, BUFSIZE_FORMAT);  // format.c
	
	if(g_scat1[0])
	{
		len = (int)wcslen(s);
		if(len > 0 && s[len - 1] != 0x0a && s[len - 1] != ' ')
			wcscat(s, L" ");
		wcscat(s, g_scat1);
	}
	if(g_scat2[0])
	{
		len = (int)wcslen(s);
		if(len > 0 && s[len - 1] != 0x0a && s[len - 1] != ' ')
			wcscat(s, L" ");
		wcscat(s, g_scat2);
	}
	
#if TC_ENABLE_CLOCKDECORATION
	switch(m_nClockDecoration)
	{
	case 1:		// shadow
		sha = m_nShadowRange;
		// fall-through
	case 2:		// shadow, border
		colShadow = (m_colShadow & 0x80000000) ?
			GetSysColor(m_colShadow & 0x00ffffff) : m_colShadow;
		colText = GetTextColor(m_hdcClock);
		break;
	}
#endif
	
#if TC_ENABLE_ANALOGCLOCK
	if(m_useAnalogClock)
	{
		if(m_nAClockPos == 1 && m_nTextPos != 2)
			ana = m_nAClockSize;
		else if(m_nAClockPos == 2 && m_nTextPos != 1)
			ana = -m_nAClockSize;
	}
#endif
	
	GetTextMetrics(m_hdcClock, &tm);
	
	y = (hclock - htext - tm.tmInternalLeading - sha) / 2 + m_dvpos;
	
	if(m_nTextPos == 1)
		x = ana + (tm.tmAveCharWidth * 2) / 3;
	else if(m_nTextPos == 2)
		x = wclock - sha + ana - (tm.tmAveCharWidth * 2) / 3;
	else
		x = (wclock - sha + ana) / 2;
	
	p = s;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		ep = p;
		if(*p == 0x0d) p += 2;
		
#if TC_ENABLE_CLOCKDECORATION
		switch(m_nClockDecoration)
		{
		case 1:		// shadow
			SetTextColor(m_hdcClock, colShadow);
			TextOutW(m_hdcClock,
				x + m_nShadowRange, y + m_nShadowRange, sp, (int)(ep - sp));
			SetTextColor(m_hdcClock, colText);
			break;
		case 2:		// border
			SetTextColor(m_hdcClock, colShadow);
			TextOutW(m_hdcClock, x - 1, y - 1, sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x    , y - 1, sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x + 1, y - 1, sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x - 1, y    , sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x + 1, y    , sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x - 1, y + 1, sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x    , y + 1, sp, (int)(ep - sp));
			TextOutW(m_hdcClock, x + 1, y + 1, sp, (int)(ep - sp));
			SetTextColor(m_hdcClock, colText);
			break;
		}
#endif
		TextOutW(m_hdcClock, x, y, sp, (int)(ep - sp));
		
		if(*p) y += tm.tmHeight - tm.tmInternalLeading
					+ 2 + m_dlineheight;
	}
	
#if TC_ENABLE_ANALOGCLOCK
	DrawAnalogClock(hwnd, m_hdcClock, pt, wclock, hclock);	// ANALOGCLOCK
#endif
	
//	if(g_nBlink > 0 && (g_nBlink % 2) == 0) dwRop = NOTSRCCOPY;
	
	if(!m_fillbackcolor && (dwRop == SRCCOPY) && aero)
	{
		BLENDFUNCTION bf = {AC_SRC_OVER, 0, 0xff, AC_SRC_ALPHA};
		CopyClockBack(hwnd, hdc, m_hdcClockBack, wclock, hclock);
	//	MyAlphaBlend(hdc, 0, 0, wclock, hclock, m_hdcClock, 0, 0, wclock, hclock, bf);
		BitBlt(hdc, 0, 0, wclock, hclock, m_hdcClock, 0, 0, SRCPAINT);
	}
	else
	{
		BitBlt(hdc, 0, 0, wclock, hclock, m_hdcClock, 0, 0, dwRop);
	}
	
	if(wtext + tm.tmAveCharWidth * 2 + m_dwidth > m_ClockWidth)
	{
		m_ClockWidth = wtext + tm.tmAveCharWidth * 2 + m_dwidth;
		PostMessage(GetParent(GetParent(hwnd)), WM_SIZE, SIZE_RESTORED, 0);
		InvalidateRect(GetParent(GetParent(hwnd)), NULL, TRUE);
	}
}

/*------------------------------------------------
  copy background image
  from m_hdcClockBack to m_hdcClock
--------------------------------------------------*/
void CopyClockBack(HWND hwnd, HDC hdcDest, HDC hdcSrc, int w, int h)
{
	int xsrc = 0, ysrc = 0;
	
	if(m_bFillTray)
	{
		HWND hwndTray;
		RECT rc;
		POINT pt;
		
		hwndTray = GetParent(hwnd);
		GetWindowRect(hwnd, &rc);
		pt.x = rc.left; pt.y = rc.top;
		ScreenToClient(hwndTray, &pt);
		xsrc = pt.x; ysrc = pt.y;
	}
	
	BitBlt(hdcDest, 0, 0, w, h, hdcSrc, xsrc, ysrc, SRCCOPY);
}

/*------------------------------------------------
  paint background of clock
--------------------------------------------------*/
void FillClock(HWND hwnd, HDC hdc, const RECT *prc)
{
	HBRUSH hbr;
	COLORREF col;
	
	if(!m_fillbackcolor)
	{
		RECT rc, rcTray;
		GetWindowRect(hwnd, &rc);
		GetWindowRect(GetParent(hwnd), &rcTray);
		CopyParentSurface(hwnd, hdc, 0, 0, prc->right, prc->bottom,
			rc.left - rcTray.left, rc.top - rcTray.top);
	}
	else if(m_colback == m_colback2)
	{
		col = m_colback;
		if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
		hbr = CreateSolidBrush(col);
		FillRect(hdc, prc, hbr);
		DeleteObject(hbr);
	}
	else
	{
		COLORREF col2;
		
		col = m_colback;
		if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
		col2 = m_colback2;
		if(col2 & 0x80000000) col2 = GetSysColor(col2 & 0x00ffffff);
		
		GradientFillClock(hdc, prc, col, col2, m_grad);
	}
}

/*------------------------------------------------
  smooth shaded background
--------------------------------------------------*/
void GradientFillClock(HDC hdc, const RECT* prc,
	COLORREF col1, COLORREF col2, DWORD grad)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT gRect;
	
	vert[0].x      = prc->left;
	vert[0].y      = prc->top;
	vert[0].Red    = (COLOR16)((int)GetRValue(col1) * 256);
	vert[0].Green  = (COLOR16)((int)GetGValue(col1) * 256);
	vert[0].Blue   = (COLOR16)((int)GetBValue(col1) * 256);
	vert[0].Alpha  = 0x0000;
	vert[1].x      = prc->right;
	vert[1].y      = prc->bottom; 
	vert[1].Red    = (COLOR16)((int)GetRValue(col2) * 256);
	vert[1].Green  = (COLOR16)((int)GetGValue(col2) * 256);
	vert[1].Blue   = (COLOR16)((int)GetBValue(col2) * 256);
	vert[1].Alpha  = 0x0000;
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	
	// newapi.c
	MyGradientFill(hdc, vert, 2, &gRect, 1, grad);
}


#if TC_ENABLE_ANALOGCLOCK
/*------------------------------------------------
  Analog Clock
--------------------------------------------------*/
void LoadAnalogClockSetting(void)
{
	const char *section = "AnalogClock";

	m_useAnalogClock = GetMyRegLong(section, "UseAnalogClock", FALSE);

	m_colHourHand = GetMyRegLong(section, "HourHandColor", 0);
	m_colMinHand = GetMyRegLong(section, "MinHandColor", 0);
	m_bHourHandBold = GetMyRegLong(section, "HourHandBold", 0);
	m_bMinHandBold = GetMyRegLong(section, "MinHandBold", 0);

	m_nAClockPos = GetMyRegLong(section, "AnalogClockPos", 0);
	m_nAClockHPos = GetMyRegLong(section, "HorizontalPos", 0);
	m_nAClockVPos = GetMyRegLong(section, "VerticalPos", 0);
	m_nAClockSize = GetMyRegLong(section, "Size", 0);

	GetMyRegStr(section, "Bitmap", m_fname, MAX_PATH, "");
}

BOOL InitAnalogClock(HWND hwnd, HDC hdc)
{
	if (!m_useAnalogClock) return FALSE;

	ClearAnalogClock();

	// load background
	if (m_fname[0]) {
		char fname2[MAX_PATH];

		RelToAbs(fname2, m_fname);
		m_hbmAclk = (HBITMAP)LoadImage(NULL, fname2, IMAGE_BITMAP,
			0, 0, LR_LOADFROMFILE);

		if (m_hbmAclk) {
			int x, y, size;

			if (GetBmpSize(m_hbmAclk, &x, &y)) // dllutl.c
				size = (x < y) ? x : y;
			else
				size = -1;
			if (m_nAClockSize == 0)
				m_nAClockSize = (size > 0) ? size : ACLOCK_DEFSIZE;

			CreateOffScreenDC(hdc, &m_hdcBack, &m_hbmBack,
				m_nAClockSize, m_nAClockSize);
			m_hdcAclk = CreateCompatibleDC(hdc);
			SelectObject(m_hdcAclk, m_hbmAclk);

			if (m_nAClockSize == size) {
				BitBlt(m_hdcBack, 0, 0, m_nAClockSize, m_nAClockSize,
					m_hdcAclk, 0, 0, SRCCOPY);
			} else {
				StretchBlt(m_hdcBack, 0, 0, m_nAClockSize, m_nAClockSize,
					m_hdcAclk, 0, 0, size, size, SRCCOPY);
			}

			DeleteDC(m_hdcAclk);
			DeleteObject(m_hbmAclk);
		}
	}

	if (m_nAClockSize == 0) m_nAClockSize = ACLOCK_DEFSIZE;

	if (!m_hdcBack) {
//		const int center = (m_nAClockSize - 1) / 2;
		RECT rc;
		HBRUSH hBrush;

		CreateOffScreenDC(hdc, &m_hdcBack, &m_hbmBack,
			m_nAClockSize, m_nAClockSize);
		rc.left = rc.top = 0;
		rc.right = rc.bottom = m_nAClockSize;
		hBrush = CreateSolidBrush(MASK_COLOR);
		FillRect(m_hdcBack, &rc, hBrush);
		DeleteObject(hBrush);
//		SetPixel(m_hdcBack, center, center, 0);
	}

	CreateOffScreenDC(hdc, &m_hdcAclk, &m_hbmAclk,
		m_nAClockSize, m_nAClockSize);

	m_hdcMask = CreateCompatibleDC(hdc);
	m_hbmMask = CreateBitmap(m_nAClockSize, m_nAClockSize, 1, 1, NULL);
	SelectObject(m_hdcMask, m_hbmMask);

	m_hpenHour = CreatePen(PS_SOLID, m_bHourHandBold ? 2 : 1, m_colHourHand);
	m_hpenMin = CreatePen(PS_SOLID, m_bMinHandBold ? 2 : 1, m_colMinHand);

	if (m_hdcAclk && m_hbmAclk && m_hdcBack && m_hbmBack
			&& m_hdcMask && m_hbmMask && m_hpenHour && m_hpenMin) {
		return TRUE;
	} else {
		ClearAnalogClock();
		return FALSE;
	}
}

void ClearAnalogClock(void)
{
	if (m_hdcAclk) { DeleteDC(m_hdcAclk); m_hdcAclk = NULL; }
	if (m_hbmAclk) { DeleteObject(m_hbmAclk); m_hbmAclk = NULL; }

	if (m_hdcBack) { DeleteDC(m_hdcBack); m_hdcBack = NULL; }
	if (m_hbmBack) { DeleteObject(m_hbmBack); m_hbmBack = NULL; }

	if (m_hdcMask) { DeleteDC(m_hdcMask); m_hdcMask = NULL; }
	if (m_hbmMask) { DeleteObject(m_hbmMask); m_hbmMask = NULL; }

	if (m_hpenHour) { DeleteObject(m_hpenHour); m_hpenHour = NULL; }
	if (m_hpenMin) { DeleteObject(m_hpenMin); m_hpenMin = NULL; }

	m_lastHour = -1;
	m_lastMin = -1;
}

void DrawAnalogClock(HWND hwnd, HDC hdcClock, const SYSTEMTIME *pt,
	int wclock, int hclock)
{
	static int x, y;
	SYSTEMTIME st;
	COLORREF col;

	if (!m_useAnalogClock) return;

	if (!m_hdcAclk && !InitAnalogClock(hwnd, hdcClock)) return;

	if (pt) memcpy(&st, pt, sizeof (SYSTEMTIME));
	else GetLocalTime(&st);

	if (st.wMinute != m_lastMin || st.wHour != m_lastHour) {
		const int center = (m_nAClockSize - 1) / 2;
		POINT ptHour, ptMin;

		m_lastHour = st.wHour;
		m_lastMin = st.wMinute;

		// calculate hands' pos
		ptHour.x = (LONG)(0.5 + center
			+ sin((m_lastHour * 60 + m_lastMin) * PI_360) * center * 0.7);
		ptHour.y = (LONG)(0.5 + center
			- cos((m_lastHour * 60 + m_lastMin) * PI_360) * center * 0.7);
		ptMin.x = (LONG)(0.5 + center + sin(m_lastMin * PI_30) * center);
		ptMin.y = (LONG)(0.5 + center - cos(m_lastMin * PI_30) * center);

		// draw hands
		BitBlt(m_hdcAclk, 0, 0, m_nAClockSize, m_nAClockSize,
			m_hdcBack, 0, 0, SRCCOPY);

		SelectObject(m_hdcAclk, m_hpenHour);
		MoveToEx(m_hdcAclk, center, center, NULL);
		LineTo(m_hdcAclk, ptHour.x, ptHour.y);
/*		MoveToEx(m_hdcAclk, ptHour.x, ptHour.y, NULL);
		LineTo(m_hdcAclk, center, center);*/
		SelectObject(m_hdcAclk, m_hpenMin);
		MoveToEx(m_hdcAclk, center, center, NULL);
		LineTo(m_hdcAclk, ptMin.x, ptMin.y);
/*		MoveToEx(m_hdcAclk, ptMin.x, ptMin.y, NULL);
		LineTo(m_hdcAclk, center, center);*/

		// make mask
		col = SetBkColor(m_hdcAclk, MASK_COLOR);
		BitBlt(m_hdcMask, 0, 0, m_nAClockSize, m_nAClockSize,
			m_hdcAclk, 0, 0, SRCCOPY);
		SetBkColor(m_hdcAclk, col);
		BitBlt(m_hdcAclk, 0, 0, m_nAClockSize, m_nAClockSize,
			m_hdcMask, 0, 0, 0x220326); // dest = (NOT source) AND dest

/*		col = SetBkColor(m_hdcAclk, MASK_COLOR);
		BitBlt(m_hdcMask, 0, 0, m_nAClockSize, m_nAClockSize,
			m_hdcAclk, 0, 0, NOTSRCCOPY);
		SetBkColor(m_hdcAclk, col);
		BitBlt(m_hdcAclk, 0, 0, m_nAClockSize, m_nAClockSize,
			m_hdcMask, 0, 0, SRCAND);
		BitBlt(m_hdcMask, 0, 0, m_nAClockSize, m_nAClockSize,
			m_hdcAclk, 0, 0, DSTINVERT);
*/
		if (m_nAClockPos == 2)
			x = wclock - 1 - m_nAClockSize + m_nAClockHPos;
		else
			x = m_nAClockHPos;
		y = (hclock - m_nAClockSize) / 2 + m_nAClockVPos;
	}

	col = SetTextColor(hdcClock, 0);
	BitBlt(hdcClock, x, y, m_nAClockSize, m_nAClockSize,
		m_hdcMask, 0, 0, SRCAND);
	BitBlt(hdcClock, x, y, m_nAClockSize, m_nAClockSize,
		m_hdcAclk, 0, 0, SRCPAINT);
	SetTextColor(hdcClock, col);
}
#endif /* TC_ENABLE_ANALOGCLOCK */
