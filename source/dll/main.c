/*-------------------------------------------------------------
  main.c : entry point of tcdll.tclock,
           API functions, and hook procedure
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */

/* APIs of tcdll.tclock */
BOOL WINAPI HookStart(HWND hwnd);
void WINAPI HookEnd(void);
void WINAPI GetTClockVersion(char* dst);

/* shared data among processes */
#ifdef _MSC_VER
#pragma data_seg("MYDATA")
HHOOK g_hhook = NULL;
HWND  g_hwndTClockMain = NULL;
HWND  g_hwndClock = NULL;
#pragma data_seg()
#endif

/* Statics */
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);

static void Debug_ListChild(HWND hwndParent, int depth);

/*------------------------------------------------
  entry point of this DLL
--------------------------------------------------*/
#ifdef NODEFAULTLIB

#ifdef __BORLANDC__
#define _DllMainCRTStartup DllEntryPoint
#endif

BOOL WINAPI _DllMainCRTStartup(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
#else
int WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
#endif
{
	return TRUE;
}

/*------------------------------------------------
  API: install my hook
  this function is called in tclock.exe process
--------------------------------------------------*/
BOOL WINAPI HookStart(HWND hwndMain)
{
	HWND hwndTaskbar, hwndTray;
	DWORD idThread;
	
	g_hwndTClockMain = hwndMain;
	
	g_hInst = GetModuleHandle(DLLFILENAME);
	
	// find the taskbar
	hwndTaskbar = GetTaskbarWindow();
	if(!hwndTaskbar)
	{
		SendMessage(hwndMain, TCM_CLOCKERROR, 0, 1);
		return FALSE;
	}
	// find the clock window
	hwndTray = FindWindowEx(hwndTaskbar, NULL, "TrayNotifyWnd", "");
	if(!hwndTray)
	{
		SendMessage(hwndMain, TCM_CLOCKERROR, 0, 2);
		return FALSE;
	}
	
	g_hwndClock = FindWindowEx(hwndTray, NULL, "TrayClockWClass", NULL);
	if(!g_hwndClock)
	{
		// WriteDebug("Your Taskbar:");
		// Debug_ListChild(hwndTaskbar, 0);
		SendMessage(hwndMain, TCM_CLOCKERROR, 0, 3);
		return FALSE;
	}
	
	// get thread ID of taskbar (explorer)
	// Specal thanks to T.Iwata.
	idThread = GetWindowThreadProcessId(hwndTaskbar, NULL);
	if(!idThread)
	{
		SendMessage(hwndMain, TCM_CLOCKERROR, 0, 4);
		return FALSE;
	}
	
	// install an hook to thread of taskbar
	g_hhook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc,
		g_hInst, idThread);
	if(!g_hhook)
	{
		SendMessage(hwndMain, TCM_CLOCKERROR, 0, 5);
		return FALSE;
	}
	
	// refresh taskbar
	PostMessage(hwndTaskbar, WM_SIZE, SIZE_RESTORED, 0);
	
	return TRUE;
}

/*------------------------------------------------
  API: uninstall my hook
  this function is called in tclock.exe process
--------------------------------------------------*/
void WINAPI HookEnd(void)
{
	// EndClock() will be called
	if(g_hwndClock && IsWindow(g_hwndClock))
		SendMessage(g_hwndClock, CLOCKM_EXIT, 0, 0);
	
	// uninstall my hook
	if(g_hhook != NULL)
		UnhookWindowsHookEx(g_hhook);
	g_hhook = NULL;
}

/*------------------------------------------------
  API: return this version
--------------------------------------------------*/
void WINAPI GetTClockVersion(char* dst)
{
	if(dst) strcpy(dst, TCLOCKVERSION);
}

/*---------------------------------------------------------
  hook procedure
  this function is called in explorer.exe process
----------------------------------------------------------*/
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LPCWPSTRUCT pcwps = (LPCWPSTRUCT)lParam;
	
	if(nCode == HC_ACTION && pcwps && pcwps->hwnd)
	{
		if(!g_bInitClock && pcwps->hwnd == g_hwndClock)
		{
			InitClock(pcwps->hwnd); // main2.c
		}
	}
	return CallNextHookEx(g_hhook, nCode, wParam, lParam);
}

/*---------------------------------------------------------
  for Debug
----------------------------------------------------------*/
void Debug_ListChild(HWND hwndParent, int depth)
{
	HWND hwnd;
	char classname[80];
	int i;
	
	for(i = 0; i < depth && i < 79; i++) classname[i] = '+';
	classname[i] = 0;
	GetClassName(hwndParent, classname + i, 80 - i);
	WriteDebug(classname);
	
	hwnd = GetWindow(hwndParent, GW_CHILD);
	while(hwnd)
	{
		Debug_ListChild(hwnd, depth + 1);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

