/*-------------------------------------------------------------
  draw.c : drawing the clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
//#include "newapi.h"

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
static int m_dwidth = 0, m_dheight = 0; // to add pixels to width and height
static int      m_dvpos = 0;            // to add pixels to vertical position
static int      m_dlineheight = 0;      // to add pixels to line height
static int      m_nTextPos = 0;         // alignment
static int      m_ClockWidth = -1;      // to save clock width


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
	
	if(!(g_winver&WINXP) && m_fillbackcolor == FALSE)
	{
		m_fillbackcolor = TRUE;
		m_colback = 0x80000000 | COLOR_3DFACE;
	}
	
	m_colback2 = m_colback;
	if((g_winver&WIN98) || (g_winver&WIN2000))
	{
		if(GetMyRegLong(NULL, "UseBackColor2", TRUE))
			m_colback2 = GetMyRegLong(NULL, "BackColor2", m_colback);
		m_grad = GetMyRegLong(NULL, "GradDir", GRADIENT_FILL_RECT_H);
	}
	
	m_bFillTray = FALSE;
	if(m_fillbackcolor && ((g_winver&WINME) || (g_winver&WIN2000)))
		m_bFillTray = GetMyRegLong(NULL, "FillTray", FALSE);
	
	m_colfore = GetMyRegLong(NULL, "ForeColor", 
		0x80000000 | COLOR_BTNTEXT);
	
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
	
	g_bFitClock = FALSE;
	if(g_winver&WINXP)
		g_bFitClock = GetMyRegLong(NULL, "FitClock", TRUE);
	
	m_ClockWidth = -1;
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
	HFONT hOldFont;
	wchar_t s[BUFSIZE_FORMAT+BUFSIZE_DISP*2];
	int wclock, hclock;
	
	if(!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE))
		return 0;
	
	hdc = GetDC(hwnd);
	if(m_hFont) hOldFont = SelectObject(hdc, m_hFont);
	GetTextMetrics(hdc, &tm);
	
	if(g_sdisp2[0]) wcscpy(s, g_sdisp2);
	else if(g_sdisp1[0]) wcscpy(s, g_sdisp1);
	else MakeFormat(s, NULL, NULL, BUFSIZE_FORMAT);
	
	if(g_scat1[0]) wcscat(s, g_scat1);
	if(g_scat2[0]) wcscat(s, g_scat2);
	
	GetClockTextSize(hdc, &tm, s, &wclock, &hclock);
	if(textwidth != NULL) *textwidth = wclock;
	if(textheight != NULL) *textheight = hclock;
	
	wclock += tm.tmAveCharWidth * 2 + m_dwidth;
	hclock += (tm.tmHeight - tm.tmInternalLeading) / 2 + m_dheight;
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
	
	if(m_hFont) SelectObject(hdc, hOldFont);
	ReleaseDC(hwnd, hdc);
	
	return (hclock << 16) + wclock;
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
	wchar_t s[BUFSIZE_FORMAT+BUFSIZE_DISP*2],
		*p, *sp, *ep;
	DWORD dwRop = SRCCOPY;
	COLORREF textcolor = 0;
	BOOL aero = FALSE;
	DWORD size;
	
	if(!m_hdcClock) CreateClockDC(hwnd);
	
	if(!m_hdcClock) return;
	
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
	
	if(g_winver&WINVISTA)
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
		len = wcslen(s);
		if(len > 0 && s[len - 1] != 0x0a && s[len - 1] != ' ')
			wcscat(s, L" ");
		wcscat(s, g_scat1);
	}
	if(g_scat2[0])
	{
		len = wcslen(s);
		if(len > 0 && s[len - 1] != 0x0a && s[len - 1] != ' ')
			wcscat(s, L" ");
		wcscat(s, g_scat2);
	}
	
	GetTextMetrics(m_hdcClock, &tm);
	
	//GetClockTextSize(m_hdcClock, &tm, s, &wtext, &htext);
	
	y = (hclock - htext)/2 - tm.tmInternalLeading/2 + m_dvpos;
	
	if(m_nTextPos == 1)
		x = (tm.tmAveCharWidth * 2) / 3;
	else if(m_nTextPos == 2)
		x = wclock - (tm.tmAveCharWidth * 2) / 3;
	else
		x = wclock / 2;
	
	p = s;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		ep = p;
		if(*p == 0x0d) p += 2;
		TextOutW(m_hdcClock, x, y, sp, (int)(ep - sp));
		
		if(*p) y += tm.tmHeight - tm.tmInternalLeading
					+ 2 + m_dlineheight;
	}
	
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
	else if(m_colback == m_colback2 || !(g_winver&(WIN98|WIN2000)))
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
	GradientFill(hdc, vert, 2, &gRect, 1, grad);
}

