/*-------------------------------------------
  tcprop.h
---------------------------------------------*/

#define _WIN32_IE    0x0200
#define _WIN32_WINNT 0x0400
#define WINVER       0x0400

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <shellapi.h>
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
BOOL CALLBACK PageColorProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagesize.c ----------- */
BOOL CALLBACK PageSizeProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pageformat.c --------- */
BOOL CALLBACK PageFormatProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pageformat2.c -------- */
BOOL CALLBACK DlgProcFormat2(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagealarm.c ---------- */
BOOL CALLBACK PageAlarmProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- alarmday.c ----------- */
int SetAlarmDay(HWND hDlg, PALARMSTRUCT pAS);

/* ---------- pagecuckoo.c --------- */
BOOL CALLBACK PageCuckooProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagemouse.c ---------- */
BOOL CALLBACK PageMouseProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagemouse2.c --------- */
BOOL CALLBACK PageMouse2Proc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagetooltip.c ----------- */
BOOL CALLBACK PageTooltipProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagesntp.c ----------- */
/* BOOL CALLBACK PageSNTPProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam); */

/* ---------- pagestartbtn.c ------- */
BOOL CALLBACK PageStartButtonProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- selecticon.c --------- */
BOOL SelectIconInDLL(HINSTANCE hInst, HWND hDlg, char* fname_index);

/* ---------- pagestartmenu.c ------- */
BOOL CALLBACK PageStartMenuProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagetaskbar.c -------- */
BOOL CALLBACK PageTaskbarProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- pagemisc.c ----------- */
BOOL CALLBACK PageMiscProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);


