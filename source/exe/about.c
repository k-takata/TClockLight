/*-------------------------------------------------------------
  about.c : "About" dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

#define IDC_MAILTO   100
#define IDC_HOMEPAGE 101

#define IDC_HAND MAKEINTRESOURCE(32649)

/* Globals */

void ShowAboutBox(HWND hwnd);
HWND g_hDlgAbout = NULL;      // dialog window handle, used in main2.c

/* Statics */

static INT_PTR CALLBACK DlgProcAbout(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg);
static void OnLinkClicked(HWND hDlg, UINT id);
static LRESULT CALLBACK LabLinkProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);

static HFONT   m_hfontLink = NULL;
static HCURSOR m_hCurHand = NULL;
static WNDPROC m_oldLabProc;

/*-------------------------------------------------------
  show "About" dialog
---------------------------------------------------------*/
void ShowAboutBox(HWND hwnd)
{
	if(g_hDlgAbout && IsWindow(g_hDlgAbout)) ;
	else
		g_hDlgAbout = CreateDialog(g_hInst, "ABOUTBOX",
			NULL, DlgProcAbout);
	SetForegroundWindow98(g_hDlgAbout);
}

/*-------------------------------------------
  dialog procedure
---------------------------------------------*/
INT_PTR CALLBACK DlgProcAbout(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			int id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
				case IDOK:
				case IDCANCEL:
					DestroyWindow(hDlg);
					break;
				case IDC_MAILTO:
				case IDC_HOMEPAGE:
					if(code == STN_CLICKED)
						OnLinkClicked(hDlg, id);
					break;
			}
			return TRUE;
		}
		case WM_CTLCOLORSTATIC:
		{
			int id; HDC hdc;
			hdc = (HDC)wParam;
			id = GetDlgCtrlID((HWND)lParam);
			if(id == IDC_MAILTO || id == IDC_HOMEPAGE)
			{
				SetTextColor(hdc, RGB(0,0,255));
				SetBkMode(hdc, TRANSPARENT);
				return (int)GetSysColorBrush(COLOR_3DFACE);
			}
			break;
		}
		case WM_DESTROY:
			g_hDlgAbout = NULL;
			if(m_hfontLink) DeleteObject(m_hfontLink);
			m_hfontLink = NULL;
			break;
	}
	return FALSE;
}

/*-------------------------------------------
  initialize main dialog
---------------------------------------------*/
void OnInit(HWND hDlg)
{
	LOGFONT logfont;
	HFONT hfont;
	
	SetMyDialgPos(hDlg, 32, 32);
	
	hfont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfont, sizeof(LOGFONT), &logfont);
	logfont.lfUnderline = 1;
	m_hfontLink = CreateFontIndirect(&logfont);
	
	if(m_hfontLink)
	{
		SendDlgItemMessage(hDlg, IDC_MAILTO, WM_SETFONT,
			(WPARAM)m_hfontLink, 0);
		SendDlgItemMessage(hDlg, IDC_HOMEPAGE, WM_SETFONT,
			(WPARAM)m_hfontLink, 0);
	}
	
	if(m_hCurHand == NULL)
		m_hCurHand = LoadCursor(NULL, IDC_HAND);
	
	m_oldLabProc = GetWndProc(GetDlgItem(hDlg, IDC_MAILTO));
	(void) SubclassWindow(GetDlgItem(hDlg, IDC_MAILTO), LabLinkProc);
	(void) SubclassWindow(GetDlgItem(hDlg, IDC_HOMEPAGE), LabLinkProc);
}

/*------------------------------------------------
  link label is clicked
--------------------------------------------------*/
void OnLinkClicked(HWND hDlg, UINT id)
{
	char str[1024];
	BOOL bOE = FALSE;
	
	if(id == IDC_MAILTO)
	{
		// Outlook Express is default mailer ?
		GetRegStr(HKEY_CLASSES_ROOT, "mailto\\shell\\open\\command", "",
			str, 1024, "");
		if(strstr(str, "msimn.exe")) bOE = TRUE;
		
		strcpy(str, "mailto:");
		if(bOE)
		{
			strcat(str, "Kazubon <");
			GetDlgItemText(hDlg, id, str + strlen(str), 80);
			strcat(str, ">?subject=About%20TClock");
		}
		else
			GetDlgItemText(hDlg, id, str + strlen(str), 80);
	}
	else GetDlgItemText(hDlg, id, str, 80);
	
	// open browser / mailer
	ShellExecute(hDlg, NULL, str, NULL, "", SW_SHOW);
}

/*-------------------------------------------
  suclass window procedure of link label
---------------------------------------------*/
LRESULT CALLBACK LabLinkProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_SETCURSOR:
			if(!m_hCurHand) break;
			SetCursor(m_hCurHand);
			return 1;
	}
	return CallWindowProc(m_oldLabProc, hwnd, message, wParam, lParam);
}

