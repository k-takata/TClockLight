/*-------------------------------------------------------------
  newapi.c : use new APIs after Win95
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
#include "newapi.h"

/* Globals */

void EndNewAPI(void);
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
BOOL MySetLayeredWindowAttributes(HWND hwnd, COLORREF crKey,
	BYTE bAlpha, DWORD dwFlags);

/* Statics */

static void InitMsimg32(void);

static HMODULE m_hmodMSIMG32 = NULL;
static BOOL (WINAPI *m_pGradientFill)(HDC,PTRIVERTEX,ULONG,PVOID,ULONG, ULONG) = NULL;
static BOOL (WINAPI *m_pAlphaBlend)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION) = NULL;
static BOOL (WINAPI *m_pTransparentBlt)(HDC,int,int,int,int,HDC,int,int,int,int,UINT) = NULL;


/*------------------------------------------------
  free DLLs
--------------------------------------------------*/
void EndNewAPI(void)
{
	if(m_hmodMSIMG32 != NULL) FreeLibrary(m_hmodMSIMG32);
	m_hmodMSIMG32 = NULL;
	m_pGradientFill = NULL;
	m_pAlphaBlend = NULL;
	m_pTransparentBlt = NULL;
}

/*------------------------------------------------
  replacement of API GradientFill
--------------------------------------------------*/
BOOL MyGradientFill(HDC hdc, PTRIVERTEX pVertex, ULONG dwNumVertex,
	PVOID pMesh, ULONG dwNumMesh, ULONG dwMode)
{
	if(m_hmodMSIMG32 == NULL) InitMsimg32();
	
	if(m_pGradientFill)
		return m_pGradientFill(hdc, pVertex, dwNumVertex,
			pMesh, dwNumMesh, dwMode);
	else return FALSE;
}

/*------------------------------------------------
  replacement of API AlphaBlend
--------------------------------------------------*/
BOOL MyAlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest,
	int nWidthDest, int nHeightDest,
	HDC hdcSrc, int nXOriginSrc, int nYOriginSrc,
	int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction)
{
	if(m_hmodMSIMG32 == NULL) InitMsimg32();
	
	if(m_pAlphaBlend)
		return m_pAlphaBlend(hdcDest, nXOriginDest, nYOriginDest, 
			nWidthDest, nHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc,
			nWidthSrc, nHeightSrc, blendFunction);
	else return FALSE;
}

/*------------------------------------------------
  replacement of API TransparentBlt
--------------------------------------------------*/
BOOL MyTransparentBlt(HDC hdcDest, int nXOriginDest, int nYOriginDest,
	int nWidthDest, int hHeightDest,
	HDC hdcSrc, int nXOriginSrc, int nYOriginSrc,
	int nWidthSrc, int nHeightSrc, UINT crTransparent)
{
	if(m_hmodMSIMG32 == NULL) InitMsimg32();
	
	if(m_pTransparentBlt)
		return m_pTransparentBlt(hdcDest, nXOriginDest, nYOriginDest,
			nWidthDest, hHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc,
			nWidthSrc, nHeightSrc, crTransparent);
	else return FALSE;
}

/*------------------------------------------------
  load msimg32.dll
--------------------------------------------------*/
void InitMsimg32(void)
{
	if(m_hmodMSIMG32) return;
	
	m_hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(m_hmodMSIMG32 != NULL)
	{
		(FARPROC)m_pGradientFill
			= GetProcAddress(m_hmodMSIMG32, "GradientFill");
		(FARPROC)m_pAlphaBlend
			= GetProcAddress(m_hmodMSIMG32, "AlphaBlend");
		(FARPROC)m_pTransparentBlt
			= GetProcAddress(m_hmodMSIMG32, "TransparentBlt");
	}
}

