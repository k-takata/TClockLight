/*-------------------------------------------
  desktop.c
    Customize Desktop Icon View
    Two_toNe 2002

	- small icon
	- transparent
  $Id: desktop.c,v 6d6ada219c32 2008/04/08 19:42:57 slic $
---------------------------------------------*/
#include "common.h"

#ifndef LVS_SMALLICON
#define LVS_SMALLICON           0x0002
#endif

static HWND hwndDesktop = 0;

void GetDesktopIcons(void)
{
HWND hwnd;

	if (hwndDesktop == 0) {
		hwnd = FindWindow("Progman", "Program Manager");
		hwnd = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
		hwnd = FindWindowEx(hwnd, NULL, "SysListView32", NULL);
		if (hwnd != NULL)
			hwndDesktop = hwnd;
	}
}

void DesktopIconsTransparentSetReset(BOOL c)
{
	if (c){
		ListView_SetTextBkColor(hwndDesktop,CLR_NONE);
	} else {
		ListView_SetTextBkColor(hwndDesktop,GetSysColor(COLOR_DESKTOP));
	}
	InvalidateRect(hwndDesktop, NULL, TRUE);
}

void SetDesktopIcons(void)
{
	LONG s;
	BOOL c;

	GetDesktopIcons();
	if(hwndDesktop)
	{
		if (!GetMyRegLong(NULL, "DeskTopIcon", FALSE)) {
			EndDesktopIcons();
			return;
		}

	// small icon
		s = GetWindowLong(hwndDesktop, GWL_STYLE);
		if ((s & LVS_SMALLICON) == 0){
			SetWindowLong(hwndDesktop, GWL_STYLE, s | LVS_SMALLICON);
		} 

	// transparent
		c = (ListView_GetTextBkColor(hwndDesktop) == CLR_NONE);
		if (GetMyRegLong(NULL, "TransDeskTopIconBK", FALSE)) {
			if (!c)
				DesktopIconsTransparentSetReset(TRUE);
		} else {	
			if (c)
				DesktopIconsTransparentSetReset(FALSE);
		}
			

	}
}

/*--------------------------------------------------
    reset desktop icons
----------------------------------------------------*/
void EndDesktopIcons(void)
{
	LONG s;

	GetDesktopIcons();
	if (hwndDesktop)
	{
	  	// normal icon
		s = GetWindowLong(hwndDesktop, GWL_STYLE);
			SetWindowLong(hwndDesktop, GWL_STYLE, s & 0xfffffffc);

		DesktopIconsTransparentSetReset(FALSE);
	hwndDesktop = NULL;
	}
}
