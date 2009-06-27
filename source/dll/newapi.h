/*-------------------------------------------
  newapi.h
---------------------------------------------*/

// using msimg32.dll APIs of Win98/2000 and later

#if (defined(_MSC_VER) && (_MSC_VER < 1200)) || (defined(__BORLANDC__) && (__BORLANDC__ < 0x550))

typedef USHORT COLOR16;

typedef struct _TRIVERTEX
{
    LONG    x;
    LONG    y;
    COLOR16 Red;
    COLOR16 Green;
    COLOR16 Blue;
    COLOR16 Alpha;
}TRIVERTEX,*PTRIVERTEX,*LPTRIVERTEX;

typedef struct _GRADIENT_RECT
{
    ULONG UpperLeft;
    ULONG LowerRight;
}GRADIENT_RECT,*PGRADIENT_RECT,*LPGRADIENT_RECT;

typedef struct _BLENDFUNCTION
{
    BYTE   BlendOp;
    BYTE   BlendFlags;
    BYTE   SourceConstantAlpha;
    BYTE   AlphaFormat;
} BLENDFUNCTION,*PBLENDFUNCTION;

#define AC_SRC_OVER                 0x00

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001
#define GRADIENT_FILL_TRIANGLE  0x00000002
#define GRADIENT_FILL_OP_FLAG   0x000000ff

#endif

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA                0x01
#endif

BOOL MyGradientFill(HDC hdc, PTRIVERTEX pVertex, ULONG dwNumVertex,
	PVOID pMesh, ULONG dwNumMesh, ULONG dwMode);

BOOL MyAlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest,
	int nWidthDest, int nHeightDest,
	HDC hdcSrc, int nXOriginSrc, int nYOriginSrc,
	int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction);

BOOL MyTransparentBlt(HDC hdcDest, int nXOriginDest, int nYOriginDest,
	int nWidthDest, int hHeightDest,
	HDC hdcSrc, int nXOriginSrc, int nYOriginSrc,
	int nWidthSrc, int nHeightSrc, UINT crTransparent);

// using SetLayeredWindowAttributes of Win2000 and later

#define WS_EX_LAYERED 0x80000
#define LWA_COLORKEY  1
#define LWA_ALPHA     2

BOOL MySetLayeredWindowAttributes(HWND hwnd, COLORREF crKey,
	BYTE bAlpha, DWORD dwFlags);

// for traynotify.c

typedef struct tagNMCUSTOMDRAWINFO
{
    NMHDR hdr;
    DWORD dwDrawStage;
    HDC hdc;
    RECT rc;
    DWORD dwItemSpec;
    UINT  uItemState;
    LPARAM lItemlParam;
} NMCUSTOMDRAW, FAR * LPNMCUSTOMDRAW;

#define CDDS_PREPAINT           0x00000001
#define CDDS_ITEM               0x00010000
#define CDDS_ITEMPREPAINT       (CDDS_ITEM | CDDS_PREPAINT)

#define NM_CUSTOMDRAW           (NM_FIRST-12)

// hide grippers

#define CCM_FIRST               0x2000
#define CCM_SETCOLORSCHEME      (CCM_FIRST + 2)
#define RB_SETCOLORSCHEME   CCM_SETCOLORSCHEME
typedef struct tagCOLORSCHEME {
   DWORD            dwSize;
   COLORREF         clrBtnHighlight;
   COLORREF         clrBtnShadow;
} COLORSCHEME, *LPCOLORSCHEME;

// flat task switch / icon only task switch

#define TCS_FLATBUTTONS         0x0008
#define TCS_HOTTRACK            0x0040
#define TCS_EX_FLATSEPARATORS	0x0001

#define TCM_SETEXTENDEDSTYLE	(4916)
#define TabCtrl_SetExtendedStyle(hwnd, dw)	\
SendMessage((hwnd), TCM_SETEXTENDEDSTYLE, 0, dw)

#define TCM_GETEXTENDEDSTYLE	(4917)
#define TabCtrl_GetExtendedStyle(hwnd)		\
SendMessage((hwnd), TCM_GETEXTENDEDSTYLE, 0, 0)

#define TCM_SETITEMSIZE         (TCM_FIRST + 41)

#define TB_SETBUTTONSIZE        (WM_USER + 31)

#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)  // For TBSTYLE_EX_*
#define TB_GETEXTENDEDSTYLE     (WM_USER + 85)  // For TBSTYLE_EX_*
#define BTNS_SHOWTEXT   0x0040              // ignored unless TBSTYLE_EX_MIXEDBUTTONS is set

#define TBSTYLE_FLAT            0x0800

#define TBSTYLE_EX_MIXEDBUTTONS             0x00000008


