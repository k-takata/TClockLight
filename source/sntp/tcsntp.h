/*-------------------------------------------
  tcsntp.h
---------------------------------------------*/

#define _WIN32_IE    0x0200
#define _WIN32_WINNT 0x0400
#define WINVER       0x0400

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include "resource.h"
#include "../common/common.h"

// notification of event to get host
#define WSOCK_GETHOST (WM_USER+1)
// notification of socket event
#define WSOCK_SELECT  (WM_USER+2)

#define IDTIMER_MAIN  1

#define SNTPM_SHOWDLG  (WM_USER+10)
#define SNTPM_ERROR    (WM_USER+11)
#define SNTPM_SUCCESS  (WM_USER+12)

#define BUFSIZE_SERVER 81

#define SNTPLOG		"SNTP.txt"

/* ---------- main.c --------------- */

BOOL ExecCommandString(HWND hwnd, const char *command);

extern HINSTANCE g_hInst;
extern char  g_mydir[];
extern BOOL  g_bIniSetting;
extern char  g_inifile[];
extern char  g_langfile[];
extern HFONT g_hfontDialog;
extern HWND  g_hwndMain;

/* ---------- dialog.c --------------- */

void OnShowDialog(HWND hwnd);

extern HWND g_hDlg;
extern HWND g_hwndLog;

/* ---------- sntp.c --------------- */
BOOL InitSNTP(HWND hwndParent);
void EndSNTP(HWND hwndParent);
void SetSNTPParam(const char *servername, int nTimeout, BOOL bLog,
	const char *soundfile);
BOOL StartSyncTime(HWND hwnd, const char *pServer, BOOL bRAS);
void OnTimerSNTP(HWND hwndMain);
void OnGetHost(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnReceive(HWND hwnd, WPARAM wParam, LPARAM lParam);
