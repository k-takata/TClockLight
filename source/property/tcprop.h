/*-------------------------------------------
  tcprop.h
---------------------------------------------*/

#define _WIN32_IE    0x0200
#define _WIN32_WINNT 0x0400
#define WINVER       0x0400

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "../common/common.h"

/* ---------- main.c --------------- */

void MyHelp(HWND hwnd, const char *title);
BOOL ExecCommandString(HWND hwnd, const char* command);

extern HINSTANCE g_hInst;
extern char  g_mydir[];
extern BOOL  g_bIniSetting;
extern char  g_inifile[];
extern char  g_langfile[];
extern HFONT g_hfontDialog;
extern int   g_winver;
extern BOOL  g_bApplyClock;
extern BOOL  g_bApplyTaskbar;
extern BOOL  g_bApplyStartMenu;
extern BOOL  g_bApplyTip;
extern BOOL  g_bApplyMain;
extern HICON g_hIconPlay, g_hIconStop;

/* ---------- pagecolor.c ---------- */
INT_PTR CALLBACK PageColorProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagesize.c ----------- */
INT_PTR CALLBACK PageSizeProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pageformat.c --------- */
INT_PTR CALLBACK PageFormatProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pageformat2.c -------- */
INT_PTR CALLBACK DlgProcFormat2(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pageanalog.c ---------- */
INT_PTR CALLBACK PageAnalogClockProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagealarm.c ---------- */
INT_PTR CALLBACK PageAlarmProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- alarmday.c ----------- */
int SetAlarmDay(HWND hDlg, PALARMSTRUCT pAS);

/* ---------- pagecuckoo.c --------- */
INT_PTR CALLBACK PageCuckooProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagemouse.c ---------- */
INT_PTR CALLBACK PageMouseProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagemouse2.c --------- */
INT_PTR CALLBACK PageMouse2Proc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagetooltip.c ----------- */
INT_PTR CALLBACK PageTooltipProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagesntp.c ----------- */
/* BOOL CALLBACK PageSNTPProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam); */

/* ---------- pagestartbtn.c ------- */
INT_PTR CALLBACK PageStartButtonProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- selecticon.c --------- */
BOOL SelectIconInDLL(HINSTANCE hInst, HWND hDlg, char* fname_index);

/* ---------- pagestartmenu.c ------- */
INT_PTR CALLBACK PageStartMenuProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagetaskbar.c -------- */
INT_PTR CALLBACK PageTaskbarProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagemisc.c ----------- */
INT_PTR CALLBACK PageMiscProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


