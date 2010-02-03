/*-------------------------------------------------------------
  command.c : process WM_COMMAND message
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"
#include "../common/command.h"

/* Globals */

void OnTClockCommand(HWND hwnd, int id, int code);
BOOL ExecCommandString(HWND hwnd, const char *command);
void CopyToClipBoard(HWND hwnd, const char *pfmt);

/* Statics */

typedef struct{
	BYTE   key;
	DWORD  flag;
} KEYEVENT, *LPKEYEVENT;

static void ShowHelp(HWND hwnd);
static void PostMessageCommand(const char *option);
static void ExecHiddenCmdPrompt(HWND hwnd, const char *str);
static BOOL CALLBACK doKyu(HWND hwnd, LPARAM lParam);
static void PushKeybd(LPKEYEVENT lpkey);

/*------------------------------------------------
  WM_COMMAND message
--------------------------------------------------*/
void OnTClockCommand(HWND hwnd, int id, int code)
{
	switch(id)
	{
		case IDC_SHOWHELP: // Help
			ShowHelp(hwnd);
			break;
		case IDC_SHOWPROP: // Property
			ExecFile(hwnd, "tcprop.exe");
			break;
		case IDC_EXIT: // Exit
			PostMessage(g_hwndClock, CLOCKM_EXIT, 0, 0);
			break;
		case IDC_TIMER:
			ExecFile(hwnd, "tctimer.exe");
			break;
		case IDC_SYNCTIME: // Syncronize time
			ExecFile(hwnd, "tcsntp.exe /silent");
			// StartSyncTime(hwnd, NULL, 0, FALSE);
			break;
		case IDC_ABOUT:  // About TClock
			ShowAboutBox(hwnd); // about.c
			break;
		case IDC_COPYCLIP: // copy to clipboard
			PostMessage(g_hwndClock, CLOCKM_COPY, 0, 0);
			break;
		case IDC_REFRESHCLOCK: // Refresh clock
			PostMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
			break;
		case IDC_DELUS:       // clear user strings
			PostMessage(g_hwndClock, CLOCKM_DELUSRSTR, 0, 0);
			PostMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
			break;
		case IDC_VISTACALENDAR:
			if (FindWindowEx(NULL, NULL, "ClockFlyoutWindow", NULL) == NULL) {
				PostMessage(g_hwndClock, CLOCKM_VISTACALENDAR, 1, 0);
			}
			break;
		case IDC_VISTATOOLTIP:
			if (FindWindowEx(NULL, NULL, "ClockTooltipWindow", NULL) == NULL) {
				PostMessage(g_hwndClock, CLOCKM_VISTATOOLTIP, 1, 0);
			}
			break;
		case IDC_TCLOCKMENU: // context menu
		{
			POINT pt;
			GetCursorPos(&pt);
			OnContextMenu(hwnd, NULL, pt.x, pt.y);
			break;
		}
		
		case IDC_SCREENSAVER: // screen saver
			SendMessage(GetDesktopWindow(), WM_SYSCOMMAND, SC_SCREENSAVE, 0);
			break;
		case IDC_SHOWDESK:    // show desktop
		{
			KEYEVENT key[]={
				{VK_LWIN, 0},               // Windows key down
				{'D', 0},                   // D key down
				{'D', KEYEVENTF_KEYUP},     // D key up
				{VK_LWIN, KEYEVENTF_KEYUP}, // Windows key up
				{0xff, 0}, };
			PushKeybd(key);
			break;
		}
		case IDC_KYU:  // kyu!
		{
			RECT rc;
			GetWindowRect(GetTaskbarWindow(), &rc);
		//	if(rc.bottom < GetSystemMetrics(SM_CYSCREEN))
		//		EnumWindows(doKyu, rc.bottom);
			EnumWindows(doKyu, (LPARAM)&rc);
			break;
		}
		case IDC_DELRECDOCS: // delete recently used documents
			SHAddToRecentDocs(SHARD_PATH, NULL);
			break;
		case IDC_SHOWSTARTMENU: // open start menu
		{
			KEYEVENT key[] = {
				{ VK_LWIN, 0 },               // Win key down
				{ VK_LWIN, KEYEVENTF_KEYUP }, // Win key up
				{ 0xff,0 },
			};
			PushKeybd(key);
			break;
		}
		case IDC_TASKSW: // switch tasks
		{
			KEYEVENT key[] = {
				{ VK_MENU, 0 },                  // Alt down
				{ VK_SHIFT, 0 },                 // Shift down
				{ VK_TAB, 0 },                   // Tab down
				{ VK_TAB, KEYEVENTF_KEYUP },     // Tab up
				{ VK_SHIFT, KEYEVENTF_KEYUP },   // Shift up
				{ VK_MENU, KEYEVENTF_KEYUP },    // Alt up
				{ 0xff, 0},
			};
			PushKeybd(key);
			break;
		}
		case IDC_LOCKPC: // lock PC
		{
			KEYEVENT key[] = {
				{ VK_LWIN, 0 },               // Win key down
				{ 'L', 0 },                   // L key down
				{ 'L', KEYEVENTF_KEYUP },     // L key up
				{ VK_LWIN, KEYEVENTF_KEYUP }, // Win key up
				{ 0xff,0 },
			};
			PushKeybd(key);
			break;
		}
		case IDC_MONOFF:  // monitor off
			PostMessage(hwnd, WM_SYSCOMMAND, SC_MONITORPOWER, 2); // SendMessage(GetDesktopWindow(), WM_SYSCOMMAND,SC_MONITORPOWER, 2);
			break;
	}
	
	// commands of task bar
	if(400 < id && id <= 510)
	{
		HWND hwndBar = GetTaskbarWindow();
		if(hwndBar)
			PostMessage(hwndBar, WM_COMMAND, id, 0);
	}
	
	// open file written in tcmenu.txt
	else if(id < 100) ContextMenuCommand(hwnd, id); // menu.c
}

/*------------------------------------------------
  processing command string
--------------------------------------------------*/
BOOL ExecCommandString(HWND hwnd, const char *command)
{
	char cmdstr[21];
	char option[MAX_PATH];
	const char *p;
	int i;
	
	if(*command == 0) return FALSE;
	
	p = command;
	for(i = 0; i < 20 && *p; i++)
	{
		if(*p == ' ') break;
		cmdstr[i] = *p++;
	}
	cmdstr[i] = 0;
	
	while(*p == ' ') p++;
	
	for(i = 0; i < MAX_PATH-1 && *p; i++)
		option[i] = *p++;
	option[i] = 0;
	
	if(strcmp(cmdstr, "clip") == 0)
	{
		if(option[0])
			CopyToClipBoard(hwnd, option);
		else if(g_hwndClock)
			PostMessage(g_hwndClock, CLOCKM_COPY, 0, 0);
		return FALSE;
	}
	else if(strcmp(cmdstr, "sntp") == 0)
	{
		strcpy(cmdstr, "tcsntp.exe /silent");
		if(strcmp(option, "ras") == 0) strcat(cmdstr, " /ras");
		ExecFile(hwnd, cmdstr);
		// SNTPCommand(hwnd, option);
		return FALSE;
	}
	else if(strcmp(cmdstr, "post") == 0)
	{
		PostMessageCommand(option);
		return FALSE;
	}
	else if(command[0] == '>') {
		const char *p = command;
		p++;
		while(*p == ' ') p++;
		ExecHiddenCmdPrompt(hwnd, p);
		return FALSE;
	}
	else
		return ExecFile(hwnd, command); // common/utl.c
}

/*------------------------------------------------
  make tcdll.tclock to copy string to clipboard
--------------------------------------------------*/
void CopyToClipBoard(HWND hwnd, const char *pfmt)
{
	wchar_t ws[MAX_PATH];
	
	if(g_hwndClock)
	{
		MultiByteToWideChar(CP_ACP, 0, pfmt, -1, ws, MAX_PATH-1);
		SendStringToOtherW(g_hwndClock, hwnd, ws, COPYDATA_COPY);
	}
}

/*------------------------------------------------
  show help page
--------------------------------------------------*/
void ShowHelp(HWND hwnd)
{
	char helpurl[MAX_PATH];
	
	GetMyRegStr(NULL, "HelpURL", helpurl, MAX_PATH, "");
	if(helpurl[0] == 0)
	{
		char langfile[MAX_PATH];
		
		FindFileWithLangCode(langfile, GetUserDefaultLangID(), TCLANGTXT);
		
		if(GetPrivateProfileString("Main", "HelpURL", "", helpurl,
			MAX_PATH, langfile) == 0) return;
	}
	
	ShellExecute(hwnd, NULL, helpurl, NULL, "", SW_SHOW);
}

/*------------------------------------------------
  post message to window
  option : "WindowClass [WindowTitle] message [wParam] [lParam]"
--------------------------------------------------*/
void PostMessageCommand(const char *option)
{
	char param2[81], param3[11], param4[11], param5[11];
	char wndclass[81], title[81];
	int message = 0, wParam = 0, lParam = 0;
	HWND hwnd;
	
	parsespace(wndclass, option, 0, 81);
	parsespace(param2, option, 1, 81);
	parsespace(param3, option, 2, 11);
	parsespace(param4, option, 3, 11);
	parsespace(param5, option, 4, 11);
	
	if(isdigitstr(param2))
	{
		title[0] = 0;
		message = atoi(param2);
		if(param3[0]) wParam = atoi(param3);
		if(param4[0]) lParam = atoi(param4);
	}
	else
	{
		strcpy(title, param2);
		if(param3[0]) message = atoi(param3);
		if(param4[0]) wParam = atoi(param4);
		if(param5[0]) lParam = atoi(param5);
	}
	
	if(wndclass[0] == 0 && title[0] == 0) return;
	if(message == 0) return;
	
	hwnd = FindWindow(wndclass[0] ? wndclass : NULL,
		title[0] ? title : NULL);
	if(hwnd)
		PostMessage(hwnd, message, wParam, lParam);
}

/*------------------------------------------------
  execute cmd.exe /c ... , hiding prompt window
--------------------------------------------------*/
void ExecHiddenCmdPrompt(HWND hwnd, const char *str)
{
	SHELLEXECUTEINFO sei;
	char *param;
	
	if(!(g_winver&WINNT)) return;
	
	param = malloc(strlen(str) + 4);
	strcpy(param, "/c ");
	strcat(param, str);
	
	memset(&sei,0,sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.nShow = SW_HIDE;
	sei.lpFile = "cmd.exe";
	sei.lpDirectory = g_mydir;
	sei.lpParameters = param;
	ShellExecuteEx(&sei);
	
	free(param);
}

/*------------------------------------------------
  send key action to Windows
--------------------------------------------------*/
void PushKeybd(LPKEYEVENT lpkey)
{
	while(lpkey->key != 0xff)
	{
		keybd_event(lpkey->key,
			(BYTE)MapVirtualKey(lpkey->key,0), lpkey->flag, 0);
		lpkey++;
	}
}

/*------------------------------------------------
  Kyu!
--------------------------------------------------*/
BOOL CALLBACK doKyu(HWND hwnd, LPARAM lParam)
{
	RECT rc;
	LPRECT lprcTaskbar = (LPRECT) lParam;
	
/*
	GetWindowRect(hwnd, &rc);
	
	if(!IsZoomed(hwnd) && IsWindowVisible(hwnd) && (rc.top < height))
		SetWindowPos(hwnd, NULL, rc.left, height, 0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
*/
	if(!IsZoomed(hwnd) && IsWindowVisible(hwnd))
	{
		int cxcenter = GetSystemMetrics(SM_CXSCREEN) / 2;
		int cycenter = GetSystemMetrics(SM_CYSCREEN) / 2;
		
		GetWindowRect(hwnd, &rc);
		
		if (lprcTaskbar->bottom <= cycenter)
		{
			//上タスクバー
			if (rc.top < lprcTaskbar->bottom)
			{
				SetWindowPos(hwnd, NULL, rc.left, lprcTaskbar->bottom, 0, 0,
							 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
		else if (lprcTaskbar->right <= cxcenter)
		{
			//左タスクバー
			if (rc.left < lprcTaskbar->right)
			{
				SetWindowPos(hwnd, NULL, lprcTaskbar->right, rc.top, 0, 0,
							 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
		else if (lprcTaskbar->left >= cxcenter)
		{
			//右タスクバー
			if (rc.right > lprcTaskbar->left)
			{
				SetWindowPos(hwnd, NULL, lprcTaskbar->left-(rc.right-rc.left), rc.top, 0, 0,
							 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
		else
		{
			//下タスクバー
			if (rc.bottom > lprcTaskbar->top)
			{
				SetWindowPos(hwnd, NULL, rc.left, lprcTaskbar->top-(rc.bottom-rc.top), 0, 0,
							 SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
			}
		}
	}
	
	return TRUE;
}

