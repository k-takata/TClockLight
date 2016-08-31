/*-------------------------------------------
  desktop.c
    Customize Desktop Icon View
    Two_toNe 2002

	- small icon
	- transparent
---------------------------------------------*/
#include "tcdll.h"

#if TC_ENABLE_DESKTOPICON

#ifndef LVS_SMALLICON
#define LVS_SMALLICON           0x0002
#endif

/* Statics */
static void GetDesktopIcons(void);
//static void DesktopIconsTransparentSetReset(BOOL c);

static HWND hwndDesktop = NULL;

/*------------------------------------------------
  relative desktop list view handle
--------------------------------------------------*/
void GetDesktopIcons(void)
{
	HWND hwnd;

	if(hwndDesktop == NULL)
	{
		hwnd = FindWindow("Progman", "Program Manager");
		hwnd = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
		hwnd = FindWindowEx(hwnd, NULL, "SysListView32", NULL);
		if(hwnd != NULL)
			hwndDesktop = hwnd;
	}
}

#if 0
/*--------------------------------------------------
  make background of icon text transparent
----------------------------------------------------*/
void DesktopIconsTransparentSetReset(BOOL c)
{
	if(c)
	{
		ListView_SetTextBkColor(hwndDesktop, CLR_NONE);
	}
	else
	{
		ListView_SetTextBkColor(hwndDesktop, GetSysColor(COLOR_DESKTOP));
	}
	InvalidateRect(hwndDesktop, NULL, TRUE);
}
#endif

/*--------------------------------------------------
  start customizing desktop icons
----------------------------------------------------*/
void SetDesktopIcons(void)
{
	LONG s;
	//BOOL c;

	if(!GetMyRegLong(NULL, "DeskTopIcon", FALSE))
	{
		EndDesktopIcons();
		return;
	}

	GetDesktopIcons();
	if(hwndDesktop)
	{
	// small icon
		s = GetWindowLong(hwndDesktop, GWL_STYLE);
		if((s & LVS_SMALLICON) == 0)
		{
			SetWindowLong(hwndDesktop, GWL_STYLE, s | LVS_SMALLICON);
		}

#if 0
	// transparent
		c = (ListView_GetTextBkColor(hwndDesktop) == CLR_NONE);
		if(GetMyRegLong(NULL, "TransDeskTopIconBK", FALSE))
		{
			if(!c)
				DesktopIconsTransparentSetReset(TRUE);
		}
		else
		{
			if(c)
				DesktopIconsTransparentSetReset(FALSE);
		}
#endif
	}
}

/*--------------------------------------------------
  end customizing desktop icons
----------------------------------------------------*/
void EndDesktopIcons(void)
{
	LONG s;

	if(hwndDesktop)
	{
		// normal icon
		s = GetWindowLong(hwndDesktop, GWL_STYLE);
		SetWindowLong(hwndDesktop, GWL_STYLE, s & ~LVS_SMALLICON);

		//DesktopIconsTransparentSetReset(FALSE);
		hwndDesktop = NULL;
	}
}

#endif	/* TC_ENABLE_DESKTOPICON */
