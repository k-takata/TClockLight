/*-------------------------------------------
  tctimer.h
---------------------------------------------*/

#define _WIN32_IE    0x0200
#define _WIN32_WINNT 0x0400
#define WINVER       0x0400

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "../common/common.h"

#define IDTIMER_TIMER 1

/* ------ structure of timer ------ */

typedef struct _tagTimerStruct
{
	struct _tagTimerStruct* next;
	char  name[BUFSIZE_NAME];
	int   id;
	int   minute;
	int   second;
	char  fname[MAX_PATH];
	BOOL  bRepeat;
	BOOL  bBlink;
	
	BOOL  bDisp;
	int   nDispType;
	int   nUserStr;
	
	DWORD interval;
	DWORD tickonstart;
} TIMERSTRUCT;
typedef TIMERSTRUCT* PTIMERSTRUCT;

/* ---------- main.c --------------- */

BOOL ExecCommandString(HWND hwnd, const char *command);

extern HINSTANCE g_hInst;
extern char  g_mydir[];
extern BOOL  g_bIniSetting;
extern char  g_inifile[];
extern char  g_langfile[];
extern HFONT g_hfontDialog;
extern HWND  g_hwndClock;
extern HWND  g_hwndTimer;
extern HICON g_hIconPlay, g_hIconStop;

/* ---------- dialog.c --------------- */

void OnShowDialog(HWND hwnd);

extern HWND g_hDlg;

/* ---------- timer.c --------------- */
void TimerStart(const PTIMERSTRUCT pTS);
void ClearTimer(void);
BOOL IsTimerRunning(void);
void OnTimerTimer(HWND hDlg);
void OnRequestMenu(HWND hDlg, BOOL bClear);
void OnStopTimer(HWND hDlg, int id);

