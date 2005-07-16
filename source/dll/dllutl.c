/*-------------------------------------------------------------
  dllutl.c : misc functions
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/*------------------------------------------------
  the windows is subclassified yet?
--------------------------------------------------*/
BOOL IsSubclassed(HWND hwnd)
{
	ULONG_PTR wndproc1;
	LONG_PTR wndproc2;
	
	if(g_winver&WINNT)
	{
		wndproc1 = GetClassLongPtrW(hwnd, GCLP_WNDPROC);
		wndproc2 = GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
	}
	else
	{
		wndproc1 = GetClassLongPtrA(hwnd, GCLP_WNDPROC);
		wndproc2 = GetWindowLongPtrA(hwnd, GWLP_WNDPROC);
	}
	
	return (wndproc1 != (ULONG_PTR)wndproc2) ? TRUE : FALSE;
}

/*------------------------------------------------
  create memory DC and bitmap
--------------------------------------------------*/
BOOL CreateOffScreenDC(HDC hdc, HDC *phdcMem, HBITMAP *phbmp,
	int width, int height)
{
	*phdcMem = CreateCompatibleDC(hdc);
	if(!*phdcMem) { *phbmp = NULL; return FALSE; }
	
	*phbmp = CreateCompatibleBitmap(hdc, width, height);
	if(!*phbmp)
	{
		DeleteDC(*phdcMem); *phdcMem = NULL;
		return FALSE;
	}
	
	SelectObject(*phdcMem, *phbmp);
	return TRUE;
}

/*------------------------------------------------
  width and height of HBITMAP
--------------------------------------------------*/
BOOL GetBmpSize(HBITMAP hbmp, SIZE *size)
{
	BITMAP bmp;
	if(GetObject(hbmp, sizeof(BITMAP), (LPVOID)&bmp) == 0)
		return FALSE;
	size->cx = bmp.bmWidth;
	size->cy = bmp.bmHeight;
	return TRUE;
}

/*------------------------------------------------
  copy the parent window's surface to child
--------------------------------------------------*/
void CopyParentSurface(HWND hwnd, HDC hdcDest, int xdst, int ydst,
	int w, int h, int xsrc, int ysrc)
{
	HDC hdcTemp, hdcMem;
	HBITMAP hbmp;
	RECT rcParent;
	
	GetWindowRect(GetParent(hwnd), &rcParent);
	
	hdcTemp = GetDC(NULL);
	
	if(!CreateOffScreenDC(hdcTemp, &hdcMem, &hbmp,
		rcParent.right - rcParent.left, rcParent.bottom - rcParent.top))
	{
		ReleaseDC(NULL, hdcTemp);
		return;
	}
	
	SendMessage(GetParent(hwnd), WM_PRINTCLIENT,
		(WPARAM)hdcMem, (LPARAM)PRF_CLIENT);
	
	BitBlt(hdcDest, xdst, ydst, w, h, hdcMem, xsrc, ysrc, SRCCOPY);
	
	DeleteDC(hdcMem);
	DeleteObject(hbmp);
	
	ReleaseDC(NULL, hdcTemp);
}
