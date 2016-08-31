/*-------------------------------------------
  tcdll.h
---------------------------------------------*/

#include "../common/common.h"

#define DLLFILENAME "tcdll.tclock"

// XButton Messages
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN                  0x020B
#endif
#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP                    0x020C
#endif

// ThemeChanged Message
#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif

// timer id
#define IDTIMER_MAIN         1001
#define IDTIMER_SYSINFO      1002
#define IDTIMER_TOOLTIP      1003

/* structure for format handler functions */

typedef struct {
	wchar_t* dp;
	const wchar_t* sp;
	SYSTEMTIME* pt;
	BOOL bZeroPad;
} FORMATHANDLERSTRUCT;

/* ---------- main.c -------------- */
extern HHOOK g_hhook;
extern HWND  g_hwndTClockMain;
extern HWND  g_hwndClock;

/* ---------- main2.c ------------- */
void InitClock(HWND hwnd);
void EndClock(HWND hwnd);
void OnDestroy(HWND hwnd);
void LoadSetting(HWND hwnd);

extern BOOL    g_bInitClock;
extern HANDLE  g_hInst;
extern BOOL    g_bIniSetting;
extern char    g_inifile[];
extern char    g_mydir[];
extern int     g_winver;
extern BOOL    g_bVisualStyle;
extern BOOL    g_bNoClock;
extern int     g_OrigClockWidth;
extern int     g_OrigClockHeight;
extern BOOL    g_bLMousePassThru;

/* ---------- wndproc.c ----------- */
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK SubclassTrayProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
extern BOOL g_bDispSecond;
extern int  g_nBlink;

/* ---------- draw.c -------------- */
void LoadDrawingSetting(HWND hwnd);
void ClearDrawing(void);
void ClearClockDC(void);
HDC GetClockBackDC(void);
void OnPaint(HWND hwnd, HDC hdc, const SYSTEMTIME* pt);
LRESULT OnCalcRect(HWND hwnd);
void CreateClockDC(HWND hwnd);

extern BOOL g_bFitClock;

/* ---------- format.c ------------ */
void LoadFormatSetting(HWND hwnd);
void MakeFormat(wchar_t* dst, const SYSTEMTIME* pt,
	const wchar_t* fmt, int nMax);
void MakeFormatEx(wchar_t* dst, const SYSTEMTIME* pt,
	const wchar_t* fmt, int nMax, BOOL bZeroPad);

/* ---------- formattime.c -------- */
void InitFormatTime(void);
void SDateHandler(FORMATHANDLERSTRUCT* pstruc);
void STimeHandler(FORMATHANDLERSTRUCT* pstruc);
void YearHandler(FORMATHANDLERSTRUCT* pstruc);
void MonthHandler(FORMATHANDLERSTRUCT* pstruc);
void DateHandler(FORMATHANDLERSTRUCT* pstruc);
void DayOfWeekHandler(FORMATHANDLERSTRUCT* pstruc);
void HourHandler(FORMATHANDLERSTRUCT* pstruc);
void MinuteHandler(FORMATHANDLERSTRUCT* pstruc);
void SecondHandler(FORMATHANDLERSTRUCT* pstruc);
void AMPMHandler(FORMATHANDLERSTRUCT* pstruc);
void CRLFHandler(FORMATHANDLERSTRUCT* pstruc);
void CharaHandler(FORMATHANDLERSTRUCT* pstruc);
void AltYearHandler(FORMATHANDLERSTRUCT* pstruc);
void EraHandler(FORMATHANDLERSTRUCT* pstruc);
void TimeDifHandler(FORMATHANDLERSTRUCT* pstruc);
void LDATEHandler(FORMATHANDLERSTRUCT* pstruc);
void DATEHandler(FORMATHANDLERSTRUCT* pstruc);
void TIMEHandler(FORMATHANDLERSTRUCT* pstruc);
void MSecondHandler(FORMATHANDLERSTRUCT* pstruc);

/* ---------- tooltip.c ----------- */
void InitTooltip(HWND hwndClock);
void EndTooltip(HWND hwndClock);
void OnTooltipMouseMsg(HWND hwndClock,
	UINT message, WPARAM wParam, LPARAM lParam);
BOOL OnTooltipNotify(HWND hwndClock, LRESULT *pres, const LPNMHDR pnmh);
void OnTimerTooltip(HWND hwndClock, BOOL forceFlg);
void PopupTooltip(HWND hwndClock, const wchar_t *p);

/* ---------- userstr.c ----------- */
void InitUserStr(void);
void UStrHandler(FORMATHANDLERSTRUCT* pstruc);

extern wchar_t g_userstr[10][BUFSIZE_USTR];
extern wchar_t g_sdisp1[], g_sdisp2[], g_scat1[], g_scat2[];

/* ---------- startbtn.c ---------- */
void InitStartButton(HWND hwndClock);
void ResetStartButton(HWND hwndClock);
void EndStartButton(void);
void CheckCursorOnStartButton(void);
void ClearStartButtonResource(void);
BOOL StartMenuFromClock(UINT message, WPARAM wParam, LPARAM lParam);

/* ---------- startmenu.c ---------- */
void InitStartMenu(HWND hwndClock);
void ResetStartMenu(HWND hwndClock);
void EndStartMenu(void);
void ClearStartMenuResource(void);
void CheckStartMenu(void);
BOOL OnDrawItemStartMenu(HWND hwnd, DRAWITEMSTRUCT* pdis);

/* ---------- taskbar.c ----------- */
void InitTaskbar(HWND hwndClock);
void EndTaskbar(HWND hwndClock);
void RefreshTaskbar(HWND hwndClock);
LRESULT CallOldTaskbarWndProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);

/* ---------- taskswitch.c -------- */
void InitTaskSwitch(HWND hwndClock);
void EndTaskSwitch(void);

/* ---------- traynotify.c -------- */
void InitTrayNotify(HWND hwndClock);
void EndTrayNotify(void);

/* ---------- newapi.c ------------ */
void EndNewAPI(void);

/* ---------- bmp.c --------------- */
HBITMAP ReadBitmap(HWND hwnd, const char* fname, BOOL b);

/* ---------- dllutl.c --------------- */
BOOL IsSubclassed(HWND hwnd);
BOOL CreateOffScreenDC(HDC hdc, HDC *phdcMem, HBITMAP *phbmp,
	int width, int height);
BOOL GetBmpSize(HBITMAP hbmp, int* w, int* h);
void CopyParentSurface(HWND hwnd, HDC hdcDest, int xdst, int ydst,
	int w, int h, int xsrc, int ysrc);

/* ---------- sysinfo.c --------------- */
void InitSysInfo(HWND hwnd);
void EndSysInfo(HWND hwnd);
void OnTimerSysInfo(void);
void ElapsedTimeHandler(FORMATHANDLERSTRUCT* pstruc);
void NetworkHandler(FORMATHANDLERSTRUCT* pstruc);
void MemoryHandler(FORMATHANDLERSTRUCT* pstruc);
void HDDHandler(FORMATHANDLERSTRUCT* pstruc);
void CPUHandler(FORMATHANDLERSTRUCT* pstruc);
void BatteryHandler(FORMATHANDLERSTRUCT* pstruc);
void ACStatusHandler(FORMATHANDLERSTRUCT* pstruc);
void VolumeHandler(FORMATHANDLERSTRUCT* pstruc);
void VolumeMuteHandler(FORMATHANDLERSTRUCT* pstruc);
void MuteHandler(FORMATHANDLERSTRUCT* pstruc);
void RefreshVolume(void);

/* ---------- net.c --------------- */
void Net_start(void);
void Net_get(ULONGLONG *recv, ULONGLONG *send);
void Net_end(void);

/* ---------- hdd.c --------------- */
void GetDiskSpace(int nDrive, ULONGLONG *all, ULONGLONG *free);

/* ---------- cpu.c --------------- */
void CpuMoni_start(void);
int CpuMoni_get(void);
void CpuMoni_end(void);

/* ---------- battery.c --------------- */
void GetBatteryLifePercent(int *batteryLife, int *batteryMode);

/* ---------- desktop.c  ---------- */
void SetDesktopIcons(void);
void EndDesktopIcons(void);

