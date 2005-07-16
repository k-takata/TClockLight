/*-------------------------------------------------------------
  bmp.c : read BMP file
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */

HBITMAP ReadBitmap(HWND hwnd, const char* fname, BOOL bTrans);

/*--------------------------------------------------
  read BMP file and return bitmap handle
----------------------------------------------------*/
HBITMAP ReadBitmap(HWND hwnd, const char* fname, BOOL bTrans)
{
	HANDLE hf;
	DWORD size, dwRead;
	BYTE *pBuf;
	const BITMAPFILEHEADER *pbmfh;
	const BITMAPINFOHEADER *pbmih;
	const BYTE *pDIBits;
	HDC hdc;
	HBITMAP hBmp;
	
	hf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hf == INVALID_HANDLE_VALUE)
		return NULL;
	
	size = GetFileSize(hf, NULL);
	
	pBuf = malloc(size);
	if(pBuf == NULL)
	{
		CloseHandle(hf); return NULL;
	}
	
	ReadFile(hf, pBuf, size, &dwRead, NULL);
	CloseHandle(hf);
	if(dwRead != size)
	{
		free(pBuf); return NULL;
	}
	
	pbmfh = (BITMAPFILEHEADER*)pBuf;
	if(pbmfh->bfType != *(WORD *)"BM")
	{
		free(pBuf); return NULL;
	}
	
	pbmih = (BITMAPINFOHEADER*)(pBuf + sizeof(BITMAPFILEHEADER));
	// don't support OS/2 format
	if(pbmih->biSize < sizeof(BITMAPINFOHEADER))
	{
		free(pBuf); return NULL;
	}
	// don't support RLE compression
	if(pbmih->biCompression != BI_RGB &&
		pbmih->biCompression != BI_BITFIELDS)
	{
		free(pBuf); return NULL;
	}
	
	pDIBits = pBuf + pbmfh->bfOffBits;
	
	if(bTrans) // pseudo transparency
	{
		if(pbmih->biBitCount <= 8)
		{
			int index;
			RGBQUAD *pColor = (RGBQUAD*)((BYTE*)pbmih + pbmih->biSize);
			COLORREF col = GetSysColor(COLOR_3DFACE);
			
			if(pbmih->biBitCount == 1)
				index = (*pDIBits & 0x80) >> 7;
			else if(pbmih->biBitCount == 4)
				index = (*pDIBits & 0xF0) >> 4;
			else if(pbmih->biBitCount == 8)
				index = *pDIBits;
			pColor[index].rgbBlue = GetBValue(col);
			pColor[index].rgbGreen = GetGValue(col);
			pColor[index].rgbRed = GetRValue(col);
		}
	}
	
	hdc = GetDC(hwnd);
	hBmp = CreateDIBitmap(hdc, pbmih,
		CBM_INIT, pDIBits, (const BITMAPINFO*)pbmih, DIB_RGB_COLORS);
	ReleaseDC(hwnd, hdc);
	free(pBuf);
	return hBmp;
}
