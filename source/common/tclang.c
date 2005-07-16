/*-------------------------------------------------------------
  tclang.c : read settings of tclang*.txt
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Globals */

void CheckTCLangVersion(void);
char* MyString(UINT uID, const char *entry);
HFONT CreateDialogFont(void);
void SetDialogLanguage(HWND hDlg, const char *section, HFONT hfont);

/* Statics */
static const char *GetControlTitle(char *dst, const char *src, int nMax);

/* Externs */

extern HINSTANCE g_hInst;
extern char g_langfile[];

/*-------------------------------------------
  compare version strings
---------------------------------------------*/
void CheckTCLangVersion(void)
{
	char buf[80];
	
	if(GetPrivateProfileString("Main", "Version", "", buf,
		80, g_langfile) > 0)
	{
		if(strcmp(buf, TCLOCKVERSION) == 0) return;
	}
	g_langfile[0] = 0;
}

/*-------------------------------------------
  returns a resource string
---------------------------------------------*/
char* MyString(UINT uID, const char *entry)
{
	static char buf[80];
	
	if(g_langfile[0] == 0 ||
		GetPrivateProfileString("String", entry, "", buf,
			80, g_langfile) == 0)
	{
		LoadString(g_hInst, uID, buf, 80);
	}
	
	return buf;
}

/*-------------------------------------------
  create dialog font
---------------------------------------------*/
HFONT CreateDialogFont(void)
{
	char buf[80], name[80], nstr[10];
	int size;
	
	if(g_langfile[0] == 0) return NULL;
	
	GetPrivateProfileString("Main", "DialogFont", "", buf,
		80, g_langfile);
	parse(name, buf, 0, 80);
	if(name[0] == 0) return NULL;
	
	parse(nstr, buf, 1, 10);
	if(nstr[0] == 0) size = 9;
	else
	{
		size = atoi(nstr);
		if(size == 0) size = 9;
	}
	
	return CreateMyFont(name, size, FW_NORMAL, 0, 0);
}

/*-------------------------------------------
  change title and size of a dialog and its controls
---------------------------------------------*/
void SetDialogLanguage(HWND hDlg, const char *section, HFONT hfont)
{
	HWND hwnd;
	HDC hdc;
	SIZE sz;
	char entry[10], buf[160], title[80];
	char classname[80];
	RECT rcCtrl, rcWin, rcClient;
	POINT ptCtrl;
	int xunit;
	int x, y, w, h;
	int maxwidth, winwidth, framewidth;
	int i;
	BOOL bSize;
	const char *test = "TClock Light";
	
	if(g_langfile[0] == 0) return;
	
	if(GetPrivateProfileString("Main", "Version", "", buf,
		80, g_langfile) > 0)
	{
		if(strcmp(buf, TCLOCKVERSION) != 0) return;
	}
	
	if(hfont != NULL) ;
	//	SendMessage(hDlg, WM_SETFONT, (WPARAM)hfont, 0);
	else
		hfont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	
	hdc = GetDC(hDlg);
	SelectObject(hdc, hfont);
	GetTextExtentPoint32(hdc, test, strlen(test), &sz);
	ReleaseDC(hDlg, hdc);
	
	xunit = (sz.cx * 100) / strlen(test);
	
	GetPrivateProfileString(section, "Title", "", title,
		80, g_langfile);
	
	if(title[0])
		SetWindowText(hDlg, title);
	
	bSize = (strcmp(section, "TestSound") != 0);
	
	hwnd = GetWindow(hDlg, GW_CHILD);
	maxwidth = 0;
	for(i = 0; hwnd; i++)
	{
		const char *p, *sp;
		
		wsprintf(entry, "Line%02d", i + 1);
		GetPrivateProfileString(section, entry, "", buf,
			160, g_langfile);
		if(!buf[0]) break;
		
		p = sp = buf;
		
		while(*p)
		{
			if(*p == '[')
			{
				const char *xp = p;
				p = GetControlTitle(title, p, 80);
				
				if(hfont != NULL)
					SendMessage(hwnd, WM_SETFONT, (WPARAM)hfont, 0);
				if(title[0])
					SetWindowText(hwnd, title);
				
				GetClassName(hwnd, classname, 80);
				if(bSize && strcmp(classname, "msctls_updown32") != 0)
				{
					GetWindowRect(hwnd, &rcCtrl);
					ptCtrl.x = rcCtrl.left;
					ptCtrl.y = rcCtrl.top;
					ScreenToClient(hDlg, &ptCtrl);
					
					x = ((xp - sp) * xunit) / 100;
					y = ptCtrl.y;
					w = ((p - xp) * xunit) / 100;
					h = rcCtrl.bottom - rcCtrl.top;
					SetWindowPos(hwnd, NULL, x, y,
						w, h, SWP_NOZORDER);
				}
				
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
				if(*p) p++;
			}
			else p++;
		}
		
		w = ((p - sp) * xunit) / 100;
		if(maxwidth < w) maxwidth = w;
	}
	
	if(!bSize) return;
	
	GetWindowRect(hDlg, &rcWin);
	GetClientRect(hDlg, &rcClient);
	winwidth = rcWin.right - rcWin.left;
	framewidth = winwidth - rcClient.right;
	if(maxwidth > winwidth - (xunit/100) - framewidth)
	{
		winwidth = maxwidth + (xunit/100) + framewidth;
		SetWindowPos(hDlg, NULL, 0, 0,
			winwidth,
			rcWin.bottom - rcWin.top,
			SWP_NOMOVE|SWP_NOZORDER);
	}
}

/*-------------------------------------------
  get "AAA" in "[  AAA   ]"
---------------------------------------------*/
const char *GetControlTitle(char *dst, const char *src, int nMax)
{
	const char *sp, *ep;
	int i;
	
	if(*src != '[') return src;
	src++;
	
	while(*src == ' ') src++;
	sp = src;
	
	ep = NULL;
	while(*src && *src != ']')
	{
		if(*src == ' ')
		{
			if(!ep) ep = src;
		}
		else ep = NULL;
		src = CharNext(src);
	}
	if(!ep) ep = src;
	
	for(i = 0; i < nMax - 1 && sp != ep; i++)
		*dst++ = *sp++;
	*dst = 0;
	
	return src;
}

