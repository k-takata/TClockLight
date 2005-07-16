/*-------------------------------------------
  tcplayer.h
---------------------------------------------*/

#define _WIN32_IE    0x0200
#define _WIN32_WINNT 0x0400
#define WINVER       0x0400

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "../common/common.h"

#define IDTIMER_PLAYER 1

#define IDC_LIST       1

/* ---------- main.c --------------- */

BOOL ExecCommandString(HWND hwnd, const char *command);

extern HINSTANCE g_hInst;
extern char  g_mydir[];
extern BOOL  g_bIniSetting;
extern char  g_inifile[];
extern char  g_langfile[];
extern HFONT g_hfontDialog;
extern HWND  g_hwndClock;
extern HWND  g_hwndPlayer;

/* ---------- dialog.c --------------- */

void OnShowDialog(HWND hwnd);

extern HWND g_hDlg;

/* ---------- player.c --------------- */
void InitPlayer(HWND hwnd);
void OnTimerPlayer(HWND hwnd);
BOOL Player(HWND hwnd, const char *fname);
BOOL IsPlayerPlaying(void);
void OnMCINotifyPlayer(HWND hwnd, WPARAM wFlags, LONG lDevID);
void StopPlayer(HWND hwnd);
void PausePlayer(HWND hwnd);
void PrevNextPlayer(HWND hwnd, BOOL bNext);
void OnRequestMenu(HWND hwnd, BOOL bClear);
