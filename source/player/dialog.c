/*-------------------------------------------------------------
  dialog.c : Player setting dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcplayer.h"

/* Globals */

void OnShowDialog(HWND hwnd);

HWND g_hDlg = NULL;

/* Statics */

BOOL CALLBACK DlgProcPlayer(HWND, UINT, WPARAM, LPARAM);
static void OnInit(HWND hDlg);
static void OnOK(HWND hDlg);
static void OnCancel(HWND hDlg);
static void OnHelp(HWND hDlg);
static void OnShowTime(HWND hDlg);
static void OnUserStr(HWND hDlg);

static char *m_section = "Player";

/*-------------------------------------------------------
  PLAYERM_SHOWDLG message
---------------------------------------------------------*/
void OnShowDialog(HWND hwnd)
{
	if(g_hDlg && IsWindow(g_hDlg)) ;
	else
		g_hDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_PLAYER),
			NULL, DlgProcPlayer);
	SetForegroundWindow98(g_hDlg);
}

/*-------------------------------------------
  dialog procedure
---------------------------------------------*/
BOOL CALLBACK DlgProcPlayer(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			int id; //, code;
			id = LOWORD(wParam); // code = HIWORD(wParam);
			switch(id)
			{
				case IDC_SHOWTIME:
					OnShowTime(hDlg);
					break;
				case IDC_SHOWWHOLE:
				case IDC_SHOWADD:
				case IDC_SHOWUSTR:
					OnUserStr(hDlg);
					break;
				case IDOK:
					OnOK(hDlg);
					break;
				case IDCANCEL:
					OnCancel(hDlg);
					break;
				case IDC_PLAYERHELP:
					OnHelp(hDlg);
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*-------------------------------------------
  initialize main dialog
---------------------------------------------*/
void OnInit(HWND hDlg)
{
	HICON hIcon;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Player", g_hfontDialog);
	
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_TCLOCK));
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	
	// common/dialog.c
	SetMyDialgPos(hDlg, 32, 32);
	
	CheckDlgButton(hDlg, IDC_SHOWTIME,
		GetMyRegLong(m_section, "Disp", FALSE));
	CheckRadioButton(hDlg, IDC_SHOWWHOLE, IDC_SHOWUSTR,
		IDC_SHOWWHOLE + GetMyRegLong(m_section, "DispType", 1));
	SetDlgItemInt(hDlg, IDC_SHOWUSTRNUM,
		GetMyRegLong(m_section, "UserStr", 0), FALSE);
	
	OnShowTime(hDlg);
}

/*-------------------------------------------
  "OK" button
---------------------------------------------*/
void OnOK(HWND hDlg)
{
	int i;
	
	SetMyRegLong(m_section, "Disp",
		IsDlgButtonChecked(hDlg, IDC_SHOWTIME));
	for(i = 0; i < 3; i++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_SHOWWHOLE+i))
		{
			SetMyRegLong(m_section, "DispType", i);
			break;
		}
	}
	SetMyRegLong(m_section, "UserStr", 
		GetDlgItemInt(hDlg, IDC_SHOWUSTRNUM, NULL, FALSE));
	
	if(IsPlayerPlaying())
		InitPlayer(g_hwndPlayer);
	else
		PostMessage(g_hwndPlayer, WM_CLOSE, 0, 0);
	DestroyWindow(hDlg);
	g_hDlg = FALSE;
}

/*-------------------------------------------
  "Cancel" button
---------------------------------------------*/
void OnCancel(HWND hDlg)
{
	if(!IsPlayerPlaying())
		PostMessage(g_hwndPlayer, WM_CLOSE, 0, 0);
	DestroyWindow(hDlg);
	g_hDlg = FALSE;
}

/*-------------------------------------------
  "Help" button
---------------------------------------------*/
void OnHelp(HWND hDlg)
{
	char helpurl[MAX_PATH], title[MAX_PATH];
	
	if(!g_langfile[0]) return;
	
	GetMyRegStr(NULL, "HelpURL", helpurl, MAX_PATH, "");
	if(helpurl[0] == 0)
	{
		if(GetPrivateProfileString("Main", "HelpURL", "", helpurl,
			MAX_PATH, g_langfile) == 0) return;
	}
	
	if(GetPrivateProfileString("Player", "HelpURL", "", title,
		MAX_PATH, g_langfile) == 0) return;
	
	if(strlen(helpurl) > 0 && helpurl[ strlen(helpurl) - 1 ] != '/')
		del_title(helpurl);
	add_title(helpurl, title);
	
	ShellExecute(hDlg, NULL, helpurl, NULL, "", SW_SHOW);
}

/*------------------------------------------------
  "Show elasped time" is checked
--------------------------------------------------*/
void OnShowTime(HWND hDlg)
{
	BOOL b;
	int i;
	
	b = IsDlgButtonChecked(hDlg, IDC_SHOWTIME);
	for(i = IDC_SHOWWHOLE; i <= IDC_SHOWUSTRNUM; i++)
		EnableDlgItem(hDlg, i, b);
	
	OnUserStr(hDlg);
}

/*------------------------------------------------
  "User string" is checked
--------------------------------------------------*/
void OnUserStr(HWND hDlg)
{
	EnableDlgItem(hDlg, IDC_SHOWUSTRNUM,
		IsDlgButtonChecked(hDlg, IDC_SHOWTIME) &&
		IsDlgButtonChecked(hDlg, IDC_SHOWUSTR));
}

