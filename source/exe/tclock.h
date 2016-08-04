/*-------------------------------------------
  tclock.h
---------------------------------------------*/

#define _WIN32_IE    0x0200
#define _WIN32_WINNT 0x0400
#define WINVER       0x0400

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <shlobj.h>
#include "../common/common.h"

// IDs for timer
#define IDTIMER_START       2
#define IDTIMER_MAIN        3
#define IDTIMER_MOUSE       4
#define IDTIMER_DEKSTOPICON 5
#define IDTIMER_MONOFF      6

// wheel message
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL    0x020A
#endif

// XButton Messages
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP   0x020C
#define XBUTTON1       0x0001
#define XBUTTON2       0x0002
#endif

// Windows Vista UIPI definitions
#ifndef MSGFLT_ADD
#define MSGFLT_ADD    1
#define MSGFLT_REMOVE 2
#endif

// SNTP window
#define IDC_SNTP 1

/* --- API of tcdll.tclock ---------- */
BOOL WINAPI HookStart(HWND hwnd);
void WINAPI HookEnd(void);
void WINAPI GetTClockVersion(char *dst);

/* ---------- main.c ---------------- */
extern HINSTANCE g_hInst;

/* ---------- main2.c --------------- */
int TClockExeMain(void);
void MyHelp(HWND hwnd, int id);

extern char  g_mydir[MAX_PATH];
extern BOOL  g_bIniSetting;
extern char  g_inifile[MAX_PATH];
extern int   g_winver;
extern UINT  g_uTaskbarRestart;

/* ---------- wndproc.c ------------- */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

extern HWND  g_hwndClock;

/* ---------- cmdopt.c ------------- */
void CheckCommandLine(HWND hwnd, BOOL bPrev);

/* ---------- command.c ---------------- */
void OnTClockCommand(HWND hwnd, int id, int code);
BOOL ExecCommandString(HWND hwnd, const char *command);
void CopyToClipBoard(HWND hwnd, const char *pfmt);

/* ---------- menu.c ---------------- */
void ContextMenuCommand(HWND hwnd, int id);
void EndContextMenu(void);
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos);
void OnExitMenuLoop(HWND hwnd);
void SetFocusTClockMain(HWND hwnd);

/* ---------- mouse.c --------------- */
void InitMouseFunction(HWND hwnd);
void EndMouseFunction(HWND hwnd);
void OnMouseDown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnMouseUp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnMouseWheel(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnTimerMouse(HWND hwnd);

/* ---------- mouse2.c -------------- */
void OnDropFiles(HWND hwnd, HDROP hdrop);

/* ---------- alarm.c --------------- */
void InitAlarm(void);
void EndAlarm(void);
void OnTimerAlarm(HWND hwnd, const SYSTEMTIME* st, int reason);

/* ---------- sntp.c -------------- */
/* BOOL InitSNTP(HWND hwndParent);
void EndSNTP(HWND hwndParent);
void SNTPCommand(HWND hwndMain, const char *pCommand);
void StartSyncTime(HWND hwndMain, const char *pServer,
	int nTimeOut, BOOL bRAS);
void OnTimerSNTP(HWND hwndMain);
void OnTCMRequestSNTPLog(HWND hwndMain, HWND hDlg); */

/* ---------- about.c -------------- */
void ShowAboutBox(HWND hwnd);
extern HWND g_hDlgAbout;

