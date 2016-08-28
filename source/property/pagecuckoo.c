/*-------------------------------------------------------------
  pagecuckoo.c : "Cuckoo" page of properties
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Statics */

static void SendPSChanged(HWND hDlg);
static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnCuckoo(HWND hDlg);
static void OnBrowse(HWND hDlg);
static void OnFileChange(HWND hDlg);
static void OnTest(HWND hDlg);

static BOOL  m_bInit = FALSE;
static BOOL  m_bChanged = FALSE;

static BOOL m_bPlaying = FALSE;

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK PageCuckooProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
				case IDC_CUCKOO:
					OnCuckoo(hDlg);
					break;
				case IDC_CUCKOOFILE:
					if(code == EN_CHANGE)
					{
						OnFileChange(hDlg);
						SendPSChanged(hDlg);
					}
					break;
				case IDC_CUCKOOBROWSE:
					OnBrowse(hDlg);
					OnFileChange(hDlg);
					SendPSChanged(hDlg);
					break;
				case IDC_CUCKOOREPEAT:
				case IDC_CUCKOOBLINK:
					SendPSChanged(hDlg);
					break;
				case IDC_CUCKOOTEST:
					OnTest(hDlg);
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: MyHelp(GetParent(hDlg), "Cuckoo"); break;
			}
			return TRUE;
		case WM_DESTROY:
			OnDestroy(hDlg);
			break;
		// playing sound ended
		case MM_MCINOTIFY:
		case MM_WOM_DONE:
			if(message == MM_MCINOTIFY)
				OnMCINotify(hDlg, wParam, (LONG)lParam);
			else
				StopFile();
			m_bPlaying = FALSE;
			SendDlgItemMessage(hDlg, IDC_CUCKOOTEST,
				BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconPlay);
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
  notify parent window to enable "Apply" button
--------------------------------------------------*/
void SendPSChanged(HWND hDlg)
{
	if(m_bInit)
	{
		g_bApplyMain = TRUE;
		m_bChanged = TRUE;
		SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
	}
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[MAX_PATH];
	
	m_bInit = FALSE;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "Cuckoo", g_hfontDialog);
	
	CheckDlgButton(hDlg, IDC_CUCKOO,
		GetMyRegLong(NULL, "Jihou", FALSE));
	GetMyRegStr(NULL, "JihouFile", s, MAX_PATH, "");
	SetDlgItemText(hDlg, IDC_CUCKOOFILE, s);
	CheckDlgButton(hDlg, IDC_CUCKOOREPEAT,
		GetMyRegLong(NULL, "JihouRepeat", FALSE));
	CheckDlgButton(hDlg, IDC_CUCKOOBLINK,
		GetMyRegLong(NULL, "JihouBlink", FALSE));
	
	SendDlgItemMessage(hDlg, IDC_CUCKOOTEST, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconPlay);
	
	OnCuckoo(hDlg);
	
	m_bPlaying = FALSE;
	
	m_bInit = TRUE;
}

/*------------------------------------------------
   apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[MAX_PATH];
	
	if(!m_bChanged) return;
	m_bChanged = FALSE;
	
	SetMyRegLong(NULL, "Jihou",
		IsDlgButtonChecked(hDlg, IDC_CUCKOO));
	GetDlgItemText(hDlg, IDC_CUCKOOFILE, s, MAX_PATH);
	SetMyRegStr(NULL, "JihouFile", s);
	SetMyRegLong(NULL, "JihouRepeat",
		IsDlgButtonChecked(hDlg, IDC_CUCKOOREPEAT));
	SetMyRegLong(NULL, "JihouBlink",
		IsDlgButtonChecked(hDlg, IDC_CUCKOOBLINK));
}

/*------------------------------------------------
  free memories associated with combo box.
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	if(m_bPlaying) StopFile(); m_bPlaying = FALSE;
}

/*------------------------------------------------
  "Cuckoo clock" checkbox
--------------------------------------------------*/
void OnCuckoo(HWND hDlg)
{
	HWND hwnd = GetDlgItem(hDlg, IDC_CUCKOO);
	BOOL b = IsDlgButtonChecked(hDlg, IDC_CUCKOO);
	
	hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	while(hwnd)
	{
		EnableWindow(hwnd, b);
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	}
	if(b) OnFileChange(hDlg);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  browse sound file
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	char deffile[MAX_PATH], fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_CUCKOOFILE, deffile, MAX_PATH);
	
	// common/soundselect.c
	if(!BrowseSoundFile(g_hInst, hDlg, deffile, fname))
		return;
	
	SetDlgItemText(hDlg, IDC_CUCKOOFILE, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}

/*------------------------------------------------
   file name changed - enable/disable controls
--------------------------------------------------*/
void OnFileChange(HWND hDlg)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_CUCKOOFILE, fname, MAX_PATH);
	
	EnableDlgItem(hDlg, IDC_CUCKOOREPEAT, IsSoundFile(fname));
}

/*------------------------------------------------
  test sound
--------------------------------------------------*/
void OnTest(HWND hDlg)
{
	char fname[MAX_PATH];
	
	GetDlgItemText(hDlg, IDC_CUCKOOFILE, fname, MAX_PATH);
	if(fname[0] == 0) return;
	
	if((HICON)SendDlgItemMessage(hDlg, IDC_CUCKOOTEST,
		BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay)
	{
		if(PlayFile(hDlg, fname, 0))
		{
			SendDlgItemMessage(hDlg, IDC_CUCKOOTEST,
				BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
			InvalidateRect(GetDlgItem(hDlg, IDC_CUCKOOTEST), NULL, FALSE);
			m_bPlaying = TRUE;
		}
	}
	else
	{
		StopFile();
		m_bPlaying = FALSE;
	}
}

