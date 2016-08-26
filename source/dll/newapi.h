/*-------------------------------------------
  newapi.h
---------------------------------------------*/

// using msimg32.dll APIs of Win98/2000 and later

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

