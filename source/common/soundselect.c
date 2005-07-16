/*-------------------------------------------------------------
  soundselect.c : select a sound file with "Open" dialog
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Globals */

BOOL BrowseSoundFile(HINSTANCE hInst, HWND hDlg,
	const char *deffile, char *fname);

/* Statics */

#define IDC_LABTESTSOUND                1
#define IDC_TESTSOUND                   2

static BOOL CALLBACK HookProcSound(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);
static void OnInitDialog(HWND hDlg);
static void OnFileNameChanged(HWND hDlg);
static void OnTestSound(HWND hDlg);
static BOOL bPlaying = FALSE;

// OPENFILENAME struct for Win Me/2000
typedef struct _tagOFNA {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCSTR       lpstrFilter;
   LPSTR        lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPSTR        lpstrFile;
   DWORD        nMaxFile;
   LPSTR        lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCSTR       lpstrInitialDir;
   LPCSTR       lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCSTR       lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
   void *       pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
} _OPENFILENAMEA, *_LPOPENFILENAMEA;

/* Externs */

extern HICON g_hIconPlay, g_hIconStop;
extern HFONT g_hfontDialog;

/*------------------------------------------------
   open dialog to browse sound files
--------------------------------------------------*/
BOOL BrowseSoundFile(HINSTANCE hInst, HWND hDlg,
	const char *deffile, char *fname)
{
	_OPENFILENAMEA ofn;
	char filter[160], mmfileexts[80];
	char ftitle[MAX_PATH], initdir[MAX_PATH];
	int winver;
	
	memset(&ofn, '\0', sizeof(_OPENFILENAMEA));
	
	filter[0] = filter[1] = 0;
	str0cat(filter, "Sound (*.wav,*.mid, ...)");
	GetSoundFileExts(mmfileexts);
	str0cat(filter, mmfileexts);
	str0cat(filter, "All (*.*)");
	str0cat(filter, "*.*");
	
	if(deffile[0] == 0 || IsSoundFile(deffile))
		ofn.nFilterIndex = 1;
	else ofn.nFilterIndex = 2;
	
	initdir[0] = 0;
	
	if(deffile[0] && IsFile(deffile))
	{
		strcpy(initdir, deffile);
		del_title(initdir);
	}
	
	fname[0] = 0;
	
	winver = CheckWinVersion();
	if((winver&WINME) || (winver&WIN2000))
		ofn.lStructSize = sizeof(_OPENFILENAMEA);
	else ofn.lStructSize = sizeof(OPENFILENAME);
	
	ofn.hwndOwner = hDlg;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile= fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;
	ofn.lpfnHook = (LPOFNHOOKPROC)HookProcSound;
	ofn.lpTemplateName = "testsoundlg";
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST|
		OFN_ENABLEHOOK| OFN_ENABLETEMPLATE;
	
	return GetOpenFileName((LPOPENFILENAME)&ofn);
}

/*------------------------------------------------
  hook procedure of common dialog
--------------------------------------------------*/
BOOL CALLBACK HookProcSound(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInitDialog(hDlg);
			break;
		case WM_DESTROY:
			if(bPlaying) StopFile(); bPlaying = FALSE;
			break;
		case WM_NOTIFY:
			switch(((LPOFNOTIFY)lParam)->hdr.code)
			{
				case CDN_SELCHANGE:
				case CDN_FOLDERCHANGE:
					OnFileNameChanged(hDlg);
				break;
			}
			return FALSE;
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_TESTSOUND)
				OnTestSound(hDlg);
			return FALSE;
		case MM_MCINOTIFY:
		case MM_WOM_DONE:
			if(message == MM_MCINOTIFY)
				OnMCINotify(hDlg, wParam, (LONG)lParam);
			else
				StopFile();
			bPlaying = FALSE;
			SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_SETIMAGE, IMAGE_ICON,
				(LPARAM)g_hIconPlay);
			InvalidateRect(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, FALSE);
			return FALSE;
		default:
			return FALSE;
	}
	return TRUE;
}

/*------------------------------------------------
  initialize hooked dialog
--------------------------------------------------*/
void OnInitDialog(HWND hDlg)
{
	HWND hwndStatic;
	RECT rc1, rc2;
	POINT pt;
	int dx;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "TestSound", g_hfontDialog);
	
	// common/dialog.c
	SetMyDialgPos(GetParent(hDlg), 64, 64);
	
	SendDlgItemMessage(hDlg, IDC_TESTSOUND, BM_SETIMAGE, IMAGE_ICON,
		(LPARAM)g_hIconPlay);
	EnableDlgItem(hDlg, IDC_TESTSOUND, FALSE);
	
	bPlaying = FALSE;
	
	// find "File Name:" Label
	hwndStatic = GetDlgItem(GetParent(hDlg), 0x442);
	if(hwndStatic == NULL) return;
	GetWindowRect(hwndStatic, &rc1);
	// move "Test:" Label
	GetWindowRect(GetDlgItem(hDlg, IDC_LABTESTSOUND), &rc2);
	dx = rc1.left - rc2.left;
	pt.x = rc2.left + dx; pt.y = rc2.top;
	ScreenToClient(hDlg, &pt);
	SetWindowPos(GetDlgItem(hDlg, IDC_LABTESTSOUND), NULL, pt.x, pt.y, 0, 0,
		SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	// move play button
	GetWindowRect(GetDlgItem(hDlg, IDC_TESTSOUND), &rc2);
	pt.x = rc2.left + dx; pt.y = rc2.top;
	ScreenToClient(hDlg, &pt);
	SetWindowPos(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, pt.x, pt.y, 0, 0,
		SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
}

/*------------------------------------------------
  when the file name is changed in hooked dialog
--------------------------------------------------*/
void OnFileNameChanged(HWND hDlg)
{
	char fname[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	BOOL b = FALSE;
	
	if (CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
		fname, sizeof(fname)) <= sizeof(fname))
	{
		hfind = FindFirstFile(fname, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				b = TRUE;
			FindClose(hfind);
		}
	}
	EnableDlgItem(hDlg, IDC_TESTSOUND, b);
}

/*------------------------------------------------
  "Test" button
--------------------------------------------------*/
void OnTestSound(HWND hDlg)
{
	char fname[MAX_PATH];
	
	if(CommDlg_OpenSave_GetFilePath(GetParent(hDlg),
			fname, sizeof(fname)) <= sizeof(fname))
	{
		if((HICON)SendDlgItemMessage(hDlg, IDC_TESTSOUND,
			BM_GETIMAGE, IMAGE_ICON, 0) == g_hIconPlay)
		{
			if(PlayFile(hDlg, fname, 0))
			{
				SendDlgItemMessage(hDlg, IDC_TESTSOUND,
					BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_hIconStop);
				InvalidateRect(GetDlgItem(hDlg, IDC_TESTSOUND), NULL, FALSE);
				bPlaying = TRUE;
			}
		}
		else
		{
			StopFile(); bPlaying = FALSE;
		}
	}
}

