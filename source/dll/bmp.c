/*-------------------------------------------------------------
  bmp.c : read BMP file
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */

HBITMAP ReadBitmap(HWND hwnd, const char* fname, BOOL b);

/* Static */

static int GetDibNumColors(const LPBITMAPINFOHEADER pbmih);
static BYTE* GetDibBitsAddr(const BYTE* pDib);

/*--------------------------------------------------
  read BMP file and return bitmap handle
----------------------------------------------------*/
HBITMAP ReadBitmap(HWND hwnd, const char* fname, BOOL bTrans)
{
	BITMAPFILEHEADER bmfh;
	BYTE* pDib;
	DWORD size;
	HFILE hf;
	BITMAPINFOHEADER* pbmih;
	BYTE* pDIBits;
	HDC hdc;
	int index;
	HBITMAP hBmp;
	
	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return NULL;
	
	size = _llseek(hf, 0, 2) - sizeof(BITMAPFILEHEADER);
	_llseek(hf, 0, 0);
	
	if(_lread(hf, (LPSTR)&bmfh, sizeof(BITMAPFILEHEADER)) !=
		sizeof(BITMAPFILEHEADER))
	{
		_lclose(hf); return NULL;
	}
	
	if(bmfh.bfType != *(WORD *)"BM")
	{
		_lclose(hf); return NULL;
	}
	
	pDib = malloc(size);
	if(pDib == NULL)
	{
		_lclose (hf); return NULL;
	}
	
	if(_lread(hf, pDib, size) != size)
	{
		_lclose(hf); free(pDib);
		return NULL;
	}
	_lclose(hf);
	
	pbmih = (BITMAPINFOHEADER*)pDib;
	// don't support OS/2 format
	if(pbmih->biSize != sizeof(BITMAPINFOHEADER))
	{
		free(pDib); return NULL;
	}
	// don't support RLE compression
	if(pbmih->biCompression != BI_RGB &&
		pbmih->biCompression != BI_BITFIELDS)
	{
		free(pDib); return NULL;
	}
	
	if(pbmih->biCompression == BI_RGB)
		pDIBits = GetDibBitsAddr(pDib);
	else
		pDIBits = pDib + sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);
	
	if(bTrans) // pseudo transparency
	{
		if(pbmih->biBitCount == 1)
			index = (*pDIBits & 0x80) >> 7;
		else if(pbmih->biBitCount == 4)
			index = (*pDIBits & 0xF0) >> 4;
		else if(pbmih->biBitCount == 8)
			index = *pDIBits;
		if(pbmih->biBitCount <= 8)
		{
			COLORREF col = GetSysColor(COLOR_3DFACE);
			((BITMAPINFO*)pDib)->bmiColors[index].rgbBlue = GetBValue(col);
			((BITMAPINFO*)pDib)->bmiColors[index].rgbGreen = GetGValue(col);
			((BITMAPINFO*)pDib)->bmiColors[index].rgbRed = GetRValue(col);
		}
	}
	
	hdc = GetDC(hwnd);
	hBmp = CreateDIBitmap(hdc,
		(LPBITMAPINFOHEADER)pDib, CBM_INIT,
		(LPSTR)pDIBits, (LPBITMAPINFO)pDib, DIB_RGB_COLORS);
	ReleaseDC(hwnd, hdc);
	free(pDib);
	return hBmp;
}

/*--------------------------------------------------
  return palette color numbers
----------------------------------------------------*/
static int GetDibNumColors(const LPBITMAPINFOHEADER pbmih)
{
	int numColors;
	int BitCount;
	
	BitCount = (int)pbmih->biBitCount;
	numColors = (int)pbmih->biClrUsed;
	if(numColors == 0)
	{
		if(BitCount <= 8) numColors = 1 << BitCount;
		else numColors = 0;
	}
	return numColors;
}

/*--------------------------------------------------
  return DIB bits address
----------------------------------------------------*/
static BYTE* GetDibBitsAddr(const BYTE* pDib)
{
	int numColors;
	
	numColors = GetDibNumColors((LPBITMAPINFOHEADER)pDib);
	return (BYTE*)(pDib + sizeof(BITMAPINFOHEADER)
		+ sizeof(RGBQUAD) * numColors);
}
