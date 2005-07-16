/*-------------------------------------------------------------
  selecticon.c : "Select Icon" dialog
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcprop.h"

/* Globals */

BOOL SelectIconInDLL(HINSTANCE hInst, HWND hDlg, char* fname_index);

/* Statics */

INT_PTR CALLBACK DlgProcSelectIcon(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);
static BOOL InitSelectIcon(HWND hDlg);
static void EndSelectIcon(HWND hDlg);
static void OnOK(HWND hDlg);
static void OnMeasureItem(HWND hDlg, LPARAM lParam);
static void OnDrawItem(LPDRAWITEMSTRUCT pDis);
static void OnBrowse(HWND hDlg);

static char* m_fname_index;
static HINSTANCE m_hInst;

/*-----------------------------------------------------------------
   open icon selecting dialog
   
   fname_index [IN/OUT]:
   executable file -> "C:\WINDOWS\SYSTEM\SHELL32.DLL,8"
   other           -> "C:\TCLOCK\NIKO.BMP"
-------------------------------------------------------------------*/
BOOL SelectIconInDLL(HINSTANCE hInst, HWND hDlg, char* fname_index)
{
	m_hInst = hInst;
	m_fname_index = fname_index;
	return (DialogBox(hInst, MAKEINTRESOURCE(IDD_SELECTICON),
		hDlg, DlgProcSelectIcon) == IDOK);
}

/*------------------------------------------------
  dialog procedure
--------------------------------------------------*/
INT_PTR CALLBACK DlgProcSelectIcon(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			if(!InitSelectIcon(hDlg))
				EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case WM_MEASUREITEM:
			OnMeasureItem(hDlg, lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItem((LPDRAWITEMSTRUCT)lParam);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id;
			id = LOWORD(wParam);
			switch (id)
			{
				case IDC_SANSHOICON:
					OnBrowse(hDlg);
					break;
				case IDOK:
					OnOK(hDlg);
				case IDCANCEL:
					EndSelectIcon(hDlg);
					EndDialog(hDlg, id);
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
BOOL InitSelectIcon(HWND hDlg)
{
	char fname[MAX_PATH], num[10];
	int i, count, index;
	HICON hicon, hiconl;
	
	// common/tclang.c
	SetDialogLanguage(hDlg, "SelectIcon", g_hfontDialog);
	
	parse(fname, m_fname_index, 0, MAX_PATH);
	parse(num, m_fname_index, 1, 10);
	if(num[0] == 0) index = 0;
	else index = atoi(num);
	
	count = (int)ExtractIcon(m_hInst, fname, (UINT)-1);
	if(count == 0)
	{
		char msg[MAX_PATH];
		strcpy(msg, MyString(IDS_NOICON, "NoIcon"));
		strcat(msg, "\n");
		strcat(msg, fname);
		MessageBox(hDlg, msg, "TClock", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	EndSelectIcon(hDlg);
	SendDlgItemMessage(hDlg, IDC_LISTICON, LB_RESETCONTENT, 0, 0);
	
	for(i = 0; i < count; i++)
	{
		hiconl = NULL; hicon = NULL;
		ExtractIconEx(fname, i, &hiconl, &hicon, 1);
		if(hiconl) DestroyIcon(hiconl);
		SendDlgItemMessage(hDlg, IDC_LISTICON, LB_ADDSTRING, 0, (LPARAM)hicon);
	}
	SetDlgItemText(hDlg, IDC_FNAMEICON, fname);
	SendDlgItemMessage(hDlg, IDC_LISTICON, LB_SETCURSEL, index, 0);
	strcpy(m_fname_index, fname);
	return TRUE;
}

/*------------------------------------------------
  clean up
--------------------------------------------------*/
void EndSelectIcon(HWND hDlg)
{
	int i, count;
	HICON hicon;
	count = SendDlgItemMessage(hDlg, IDC_LISTICON, LB_GETCOUNT, 0, 0);
	for(i = 0; i < count; i++)
	{
		hicon = (HICON)SendDlgItemMessage(hDlg, IDC_LISTICON,
			LB_GETITEMDATA, i, 0);
		if(hicon) DestroyIcon(hicon);
	}
}

/*------------------------------------------------
  "OK" button
--------------------------------------------------*/
void OnOK(HWND hDlg)
{
	char num[10];
	int index;
	
	GetDlgItemText(hDlg, IDC_FNAMEICON, m_fname_index, MAX_PATH);
	index = SendDlgItemMessage(hDlg, IDC_LISTICON, LB_GETCURSEL, 0, 0);
	wsprintf(num, ",%d", index);
	strcat(m_fname_index, num);
}

/*------------------------------------------------
  WM_MEASUREITEM message
--------------------------------------------------*/
void OnMeasureItem(HWND hDlg, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pMis;
	RECT rc;
	
	pMis = (LPMEASUREITEMSTRUCT)lParam;
	GetClientRect(GetDlgItem(hDlg, pMis->CtlID), &rc);
	pMis->itemHeight = rc.bottom;
	pMis->itemWidth = 32;
}

/*------------------------------------------------
  WM_DRAWITEM message
--------------------------------------------------*/
void OnDrawItem(LPDRAWITEMSTRUCT pDis)
{
	HBRUSH hbr;
	
	switch(pDis->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
		{
			if(pDis->itemState & ODS_SELECTED)
				hbr = GetSysColorBrush(COLOR_HIGHLIGHT);
			else
				hbr = GetSysColorBrush(COLOR_WINDOW);
			FillRect(pDis->hDC, &pDis->rcItem, hbr);
			if(!(pDis->itemState & ODS_FOCUS)) break;
		}
		case ODA_FOCUS:
		{
			if(pDis->itemState & ODS_FOCUS)
				hbr = GetSysColorBrush(COLOR_WINDOWTEXT);
			else
				hbr = GetSysColorBrush(COLOR_WINDOW);
			FrameRect(pDis->hDC, &pDis->rcItem, hbr);
			break;
		}
	}
	
	if(pDis->itemData != 0)
	{
		RECT rc;
		int cxicon = GetSystemMetrics(SM_CXSMICON);
		int cyicon = GetSystemMetrics(SM_CYSMICON);
		
		CopyRect(&rc, &(pDis->rcItem));
		DrawIconEx(pDis->hDC,
			rc.left + (rc.right - rc.left - cxicon)/2,
			rc.top + (rc.bottom - rc.top - cyicon)/2,
			(HICON)pDis->itemData,
			cxicon, cyicon, 0, NULL, DI_NORMAL);
	}
}

/*------------------------------------------------
  "Browse" button
--------------------------------------------------*/
void OnBrowse(HWND hDlg)
{
	const char *filter = "Bitmap, Icon (*.bmp, *.ico)\0*.bmp;*.ico\0"
		"Executable (*.exe, *.dll)\0*.exe;*.dll\0"
		"All (*.*)\0*.*\0\0";
	char deffile[MAX_PATH], fname[MAX_PATH];
	HANDLE hf;
	char head[2];
	DWORD dwRead;
	
	GetDlgItemText(hDlg, IDC_FNAMEICON, deffile, MAX_PATH);
	
	if(!SelectMyFile(m_hInst, hDlg, filter, 2, deffile, fname))
		return;
	
	hf = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hf == INVALID_HANDLE_VALUE) return;
	ReadFile(hf, head, 2, &dwRead, NULL);
	CloseHandle(hf);
	strcpy(m_fname_index, fname);
	
	if(head[0] == 'M' && head[1] == 'Z') // Executable
	{
		if(InitSelectIcon(hDlg))
			PostMessage(hDlg, WM_NEXTDLGCTL,
				(WPARAM)GetDlgItem(hDlg, IDC_LISTICON), TRUE);
	}
	else
	{
		EndSelectIcon(hDlg);
		EndDialog(hDlg, IDOK);
	}
}

