/*-------------------------------------------
  common.h
---------------------------------------------*/

#define _WIN32_IE    0x0600
#define _WIN32_WINNT 0x0600
#define WINVER       0x0600

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <shellapi.h>
#include <shlobj.h>
#include "../config.h"

#define TCLOCKVERSION     "TClock Light kt160825"

#define CLASS_TCLOCKMAIN   "TClockMainClass"
#define CLASS_TCLOCKPROP   "TClockPropertyClass"
#define CLASS_TCLOCKTIMER  "TClockTimerClass"
#define CLASS_TCLOCKPLAYER "TClockPlayerClass"
#define CLASS_TCLOCKSNTP   "TClockSNTPClass"
#define TITLE_TCLOCKMAIN   "TClock"

#define REGMYKEY  "Software\\Kazubon\\TClock"

#define TCLANGTXT "tclang.txt"
#define TCMENUTXT "tcmenu.txt"
#define LANGDIR   "lang"

/* -- messages to send to tcdll.tclock ----------------- */

#define CLOCKM_REFRESHCLOCK     (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR   (WM_USER+2)
#define CLOCKM_BLINK            (WM_USER+3)
#define CLOCKM_COPY             (WM_USER+4)
#define CLOCKM_REFRESHDESKTOP   (WM_USER+5)
#define CLOCKM_VOLCHANGE        (WM_USER+6)
#define CLOCKM_NETINIT          (WM_USER+7)
#define CLOCKM_DELUSRSTR        (WM_USER+8)
#define CLOCKM_EXIT             (WM_USER+9)
#define CLOCKM_REFRESHSTARTMENU (WM_USER+10)
#define CLOCKM_REFRESHTOOLTIP   (WM_USER+11)

#define CLOCKM_VISTACALENDAR    (WM_USER+102)
#define CLOCKM_VISTATOOLTIP     (WM_USER+103)

/* -- messages to send to tclock.exe ------------------ */

#define TCM_HWNDCLOCK       WM_USER
#define TCM_CLOCKERROR      (WM_USER+1)
#define TCM_EXIT            (WM_USER+2)
#define TCM_STOPSOUND       (WM_USER+3)
#define TCM_RELOADSETTING   (WM_USER+4)
#define TCM_DESKCAL         (WM_USER+10)
#define TCM_REQUESTSNTPLOG  (WM_USER+11)

/* -- messages to send to tctimer.exe ------------------ */
#define TIMERM_SHOWDLG      (WM_USER+1)
#define TIMERM_REQUESTMENU  (WM_USER+2)
#define TIMERM_STOP         (WM_USER+3)

/* -- messages to send to tcplayer.exe ------------------ */
#define PLAYERM_SHOWDLG     (WM_USER+1)
#define PLAYERM_REQUESTMENU (WM_USER+2)
#define PLAYERM_STOP        (WM_USER+3)
#define PLAYERM_PAUSE       (WM_USER+4)
#define PLAYERM_PREV        (WM_USER+5)
#define PLAYERM_NEXT        (WM_USER+6)

/* -- dwData of COPYDATASTRUCT ----------------------- */

#define COPYDATA_USTR0      0
#define COPYDATA_USTR1      1
#define COPYDATA_USTR2      2
#define COPYDATA_USTR3      3
#define COPYDATA_USTR4      4
#define COPYDATA_USTR5      5
#define COPYDATA_USTR6      6
#define COPYDATA_USTR7      7
#define COPYDATA_USTR8      8
#define COPYDATA_USTR9      9
#define COPYDATA_DISP1      100
#define COPYDATA_DISP2      101
#define COPYDATA_DISP3      102
#define COPYDATA_CAT1       103
#define COPYDATA_CAT2       104
#define COPYDATA_CAT3       105
#define COPYDATA_PLAY       106
#define COPYDATA_SOUND      107
#define COPYDATA_COPY       108
#define COPYDATA_EXEC       109
#define COPYDATA_SNTPLOG    110
#define COPYDATA_SNTPLOGADD 111

/* -- buffer size --------------------------------------- */

#define BUFSIZE_FORMAT  256
#define BUFSIZE_TOOLTIP 256
#define BUFSIZE_DISP     41
#define BUFSIZE_USTR     41
#define BUFSIZE_NAME     40

/* -- alarmstruct.c ---------------------------------------- */

// struct of Alarm
typedef struct _tagAlarmStruct
{
	char name[BUFSIZE_NAME];
	BOOL bEnable;
	char strHours[80];
	char strMinutes[80];
	char strWDays[80];
	char hours[24];
	char minutes[60];
	char wdays[7];
	int  second;
	char fname[MAX_PATH];
	BOOL bHour12;
	BOOL bRepeat;
	BOOL bRepeatJihou;
	BOOL bBlink;
	int  nBlinkSec;
	BOOL bBootExec;
	BOOL bInterval;
	int  nInterval;
	DWORD tickLast;
	BOOL bResumeExec;
	int  nResumeDelay;
	BOOL bResumeTimer;
} ALARMSTRUCT;
typedef ALARMSTRUCT* PALARMSTRUCT;

void LoadAlarm(PALARMSTRUCT pAS, int count);
void SaveAlarm(const PALARMSTRUCT pAS, int count);
void SetAlarmTime(PALARMSTRUCT pAS);

/* -- autoformat.c ---------------------------------------- */

#define NUM_FORMATPART 11
#define PART_YEAR4     0
#define PART_YEAR      1
#define PART_MONTH     2
#define PART_MONTHS    3
#define PART_DAY       4
#define PART_WEEKDAY   5
#define PART_HOUR      6
#define PART_MINUTE    7
#define PART_SECOND    8
#define PART_BREAK     9
#define PART_AMPM      10

void InitAutoFormat(int ilang);
void AutoFormat(char* dst, BOOL* parts);

/* -- combobox.c ---------------------------------------- */

void InitColorCombo(HWND hDlg, int idCombo,
	const COLORREF *pColAdd, int nAdd, COLORREF colDef);
void OnMeasureItemColorCombo(LPMEASUREITEMSTRUCT pmis);
void OnDrawItemColorCombo(LPDRAWITEMSTRUCT pdis, char (*pTexts)[80]);
void ChooseColorWithCombo(HINSTANCE hInst, HWND hDlg,
	int idCombo);
void InitFontNameCombo(HWND hDlg, int idCombo, const char* deffont);
void InitFontSizeCombo(HWND hDlg, int idCombo,
	const char *fontname, int charset);
void InitLocaleCombo(HWND hDlg, int idCombo, int deflang);

/* -- exec.c -------------------------------------------- */

void RelToAbs(char *dst, const char *src);
void GetFileAndOption(const char* command, char* fname, char* option);
BOOL ExecFile(HWND hwnd, const char* command);

/* -- font.c -------------------------------------------- */

HFONT CreateMyFont(const char *fontname, int size,
	LONG weight, LONG italic, int codepage);
void GetDefaultFontName(char *fontname, const char *defaultfontname);

/* -- langcode.c ---------------------------------------- */

BOOL LangIDToLangCode(char *dst, int langid, BOOL bCountry);
BOOL FindFileWithLangCode(char *dst, int langid, const char* fname);
BOOL DoesLangDirExist(void);

/* -- localeinfo.c ---------------------------------------- */

int GetCodePage(int ilang);
int MyGetLocaleInfoW(int ilang, int codepage,
	LCTYPE lctype, wchar_t* dst, int n);
int MyGetLocaleInfoA(int ilang, int codepage,
	LCTYPE lctype, char* dst, int n);
int MyGetDateFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n);
int MyGetTimeFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n);

/* -- mousestruct.c ---------------------------------------- */

// struct of Mouse
typedef struct _tagMouseStruct
{
	char name[BUFSIZE_NAME];
	int  nButton;
	int  nClick;
	BOOL bCtrl;
	BOOL bShift;
	BOOL bAlt;
	int  nCommand;
	char option[MAX_PATH];
} MOUSESTRUCT;
typedef MOUSESTRUCT* PMOUSESTRUCT;

void LoadMouseFunc(PMOUSESTRUCT pMSS, int count);
void SaveMouseFunc(PMOUSESTRUCT pMSS, int count);
void ImportOldMouseFunc(void);

/* -- nodeflib.c ---------------------------------------- */

#ifdef NODEFAULTLIB

void *r_memcpy(void *d, const void *s, size_t l);
void *r_memset(void *d, int c, size_t l);
char *r_strstr(const char *string, const char *strCharSet);
int r_strncmp(const char* d, const char* s, size_t n);
int r_strnicmp(const char* d, const char* s, size_t n);
int r_atoi(const char *p);
int r_atox(const char *p);
int r__wtoi(const WCHAR *p);
int r_wcslen(const wchar_t *p);
wchar_t *r_wcscpy(wchar_t *dp, const wchar_t *sp);
int r_wcsncmp(const wchar_t *p1, const wchar_t *p2, int count);
wchar_t *r_wcscat(wchar_t *dp, const wchar_t *sp);

__inline int r_toupper(int c)
{
	if('a' <= c && c <= 'z') c -= 'a' - 'A';
	return c;
}

DWORDLONG r_M32x32to64(DWORD a, DWORD b);

#undef toupper
#define toupper(c) r_toupper(c)

#undef strncmp
#define strncmp(d, s, n) r_strncmp(d, s, n)

#undef strstr
#define strstr(a,b) r_strstr(a,b)

#undef atoi
#define atoi(p) r_atoi(p)

#define atox(p) r_atox(p)

#undef _wtoi
#define _wtoi(p) r__wtoi(p)

#undef wcslen
#define wcslen(p) r_wcslen(p)

#undef wcscpy
#define wcscpy(d,s) r_wcscpy(d,s)

#undef wcscat
#define wcscat(d,s) r_wcscat(d,s)

#undef wcsncmp
#define wcsncmp(d,s,n) r_wcsncmp(d,s,n)

#undef malloc
#define malloc(cb) HeapAlloc(GetProcessHeap(), 0, cb)

#undef free
#define free(p) HeapFree(GetProcessHeap(), 0, p)

#define M32x32to64(a,b) r_M32x32to64(a,b)

#ifdef __BORLANDC__

#undef memcpy
#define memcpy(d,s,l) r_memcpy(d,s,l)

#undef memset
#define memset(d,c,l) r_memset(d,c,l)

#undef strlen
#define strlen(s) lstrlen(s)

#undef strcat
#define strcat(d, s) lstrcat(d, s)

#endif  // end of __BORLANDC__

#else   // #ifndef NODEFAULTLIB

#define atox(p) strtol((p),NULL,16)

#define M32x32to64(a,b) ((DWORDLONG)(a)*(DWORDLONG)(b))

#endif  // end of #ifdef NODEFAULTLIB

/* -- playfile.c ---------------------------------------- */

BOOL PlayFile(HWND hwnd, const char *fname, int loops);
BOOL PlayFileCmdLine(HWND hwnd, const char *str);
BOOL Player(HWND hwnd, const char *fname);
void StopFile(void);
void OnMCINotify(HWND hwnd, WPARAM wFlags, LONG lDevID);
BOOL IsSoundFile(const char* fname);
BOOL PauseResume(HWND hwnd);
BOOL GetPlayingPosition(char *dst);
BOOL PrevNextTrack(HWND hwnd, BOOL bNext);
BOOL IsPrevNext(BOOL bNext);
BOOL PlayMCI(HWND hwnd, const char *fname, int loops);
void GetSoundFileExts(char* dst);

/* -- reg.c ---------------------------------------- */

int GetMyRegStr(const char *section, const char *entry,
	char* val, int cbData, const char *defval);
int GetMyRegStrW(const char *section, const char *entry,
	wchar_t *dst, int cbData, const char *defval);
BOOL SetMyRegStr(const char *section, const char *entry, const char *val);
LONG GetMyRegLong(const char *section, const char *entry, LONG defval);
BOOL SetMyRegLong(const char *section, const char *entry, DWORD val);
BOOL DelMyReg(const char *section, const char *entry);
BOOL DelMyRegKey(const char *section);
int GetRegStr(HKEY rootkey, const char *subkey, const char *entry,
	char *val, int cbData, const char *defval);
LONG GetRegLong(HKEY rootkey, const char *subkey, const char *entry,
	LONG defval);

/* -- selectfile.c ---------------------------------------- */

BOOL SelectMyFile(HINSTANCE hInst, HWND hDlg,
	const char *filter, DWORD nFilterIndex,
	const char *deffile, char *retfile);
BOOL SelectFolder(HWND hDlg, const char *def, char *ret);

/* -- soundselect.c ---------------------------------------- */

BOOL BrowseSoundFile(HINSTANCE hInst, HWND hDlg,
	const char *deffile, char *fname);

/* -- tclang.c ---------------------------------------- */

void CheckTCLangVersion(void);
char* MyString(UINT uID, const char *entry);
HFONT CreateDialogFont(void);
void SetDialogLanguage(HWND hDlg, const char *section, HFONT hfont);

/* ---------- desktop.c  ---------- */

void SetDesktopIcons(void);
void EndDesktopIcons(void);

/* -- utl.c ---------------------------------------- */

// Windows version flag
#define WIN95    0x0001   // 95,98,Me
#define WIN98    0x0002   // 98,Me
#define WINME    0x0004   // Me
#define WINNT    0x0008   // NT4,2000,XP,Vista,Win7, ...
#define WIN2000  0x0010   // 2000,XP,Vista,Win7, ...
#define WINXP    0x0020   // XP,Vista,Win7, ...
#define WINVISTA 0x0040   // Vista,Win7, ...
#define WIN7     0x0080   // Win7, ...
#define WIN8     0x0100   // Win8, ...
#define WIN8_1   0x0200   // Win8.1, ...
#define WIN10    0x0400   // Win10,Win10TH2,Win10RS1
#define WIN10TH2 0x0400   // Win10TH2,Win10RS1
#define WIN10RS1 0x0800   // Win10RS1

void add_title(char *path, const char* titile);
void del_title(char *path);
int ext_cmp(const char *fname, const char *ext);
void del_ext(char* ext, char *fname);
int parse(char *dst, const char *src, int n, int nMax);
void parsespace(char *dst, const char *src, int n, int nMax);
void str0cat(char* dst, const char* src);
int isdigitstr(const char *p);
const char* nextline(const char* p);

HWND GetTaskbarWindow(void);
HWND GetClockWindow(void);
HWND GetTClockMainWindow(void);
void SendStringToOther(HWND hwnd, HWND hwndFrom, const char *s, int type);
void SendStringToOtherW(HWND hwnd, HWND hwndFrom, const wchar_t *s, int type);
BOOL IsFile(const char* fname);
BOOL IsDirectory(const char* fname);
int CheckWinVersion(void);
BOOL IsXPVisualStyle(void);
BOOL IsVistaAero(void);
BOOL IsTaskbarAnimation(void);
void SetForegroundWindow98(HWND hwnd);
void SetMyDialgPos(HWND hwnd, int xLen, int yLen);
void WriteDebug(const char* s);
void WriteDebugW(const wchar_t* s);

/* ---------- mixer.c ----------------- */

BOOL GetMasterVolume(int *Val);
BOOL GetMasterMute(BOOL *Val);
BOOL SetMasterVolume(int Val);
BOOL UpDownMasterVolume(int dif);
BOOL ReverseMasterMute(void);
BOOL InitMixer(void);
void ReleaseMixer(void);

/* -- Macros ---------------------------------------- */

#define EnableDlgItem(hDlg,id,b) EnableWindow(GetDlgItem((hDlg),(id)),(b))
#define ShowDlgItem(hDlg,id,b) ShowWindow(GetDlgItem((hDlg),(id)),(b)?SW_SHOW:SW_HIDE)
#define AdjustDlgConboBoxDropDown(hDlg,id,b) AdjustConboBoxDropDown(GetDlgItem((hDlg),(id)),(b))

#define CBAddString(hDlg,id,lParam) (int)SendDlgItemMessage((hDlg),(id),CB_ADDSTRING,0,(lParam))
#define CBDeleteString(hDlg,id, i) SendDlgItemMessage((hDlg),(id),CB_DELETESTRING,(i),0)
#define CBFindString(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRING,0,(LPARAM)(s))
#define CBFindStringExact(hDlg,id,s) (int)SendDlgItemMessage((hDlg),(id),CB_FINDSTRINGEXACT,0,(LPARAM)(s))
#define CBGetCount(hDlg,id) (int)SendDlgItemMessage((hDlg),(id),CB_GETCOUNT,0,0)
#define CBGetCurSel(hDlg,id) (int)SendDlgItemMessage((hDlg),(id),CB_GETCURSEL,0,0)
#define CBGetItemData(hDlg,id,i) (int)SendDlgItemMessage((hDlg),(id),CB_GETITEMDATA,(i),0)
#define CBGetLBText(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_GETLBTEXT,(i),(LPARAM)(s))
#define CBInsertString(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_INSERTSTRING,(i),(LPARAM)(s))
#define CBResetContent(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_RESETCONTENT,0,0)
#define CBSetCurSel(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_SETCURSEL,(i),0)
#define CBSetItemData(hDlg,id,i,lParam) SendDlgItemMessage((hDlg),(id),CB_SETITEMDATA,(i),(lParam))

#define UpDown_GetPos(hDlg,id) \
	(int)SendDlgItemMessage((hDlg),(id),UDM_GETPOS,0,0)
#define UpDown_SetAccel(hDlg,id,nAccels,aAccels) \
	SendDlgItemMessage((hDlg),(id),UDM_SETACCEL,(nAccels),(LPARAM)(aAccels))
#define UpDown_SetRange(hDlg,id,nUpper,nLower) \
	SendDlgItemMessage((hDlg),(id),UDM_SETRANGE,0,MAKELONG((nUpper),(nLower)))
#define UpDown_SetBuddy(hDlg,id,idBuddy) \
	SendDlgItemMessage((hDlg),(id),UDM_SETBUDDY, \
		(WPARAM)GetDlgItem((hDlg),(idBuddy)),0)
#define UpDown_SetPos(hDlg,id,nPos) \
	SendDlgItemMessage((hDlg),(id),UDM_SETPOS,0,nPos)

#define GetWndProc(hwnd) ((WNDPROC)GetWindowLongPtr((hwnd), GWLP_WNDPROC))

#ifndef HWND_MESSAGE
#define HWND_MESSAGE	((HWND)-3)
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam)	((short)HIWORD(wParam))
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr)	(sizeof(arr) / sizeof((arr)[0]))
#endif

/* -- for VC6 compatibility ---------------------------------------- */

#ifndef _W64
typedef unsigned long DWORD_PTR;
typedef long LONG_PTR;
typedef unsigned long ULONG_PTR;
#endif

#ifndef SetWindowLongPtr
#define SetWindowLongPtr	SetWindowLong
#define SetWindowLongPtrA	SetWindowLongA
#define SetWindowLongPtrW	SetWindowLongW
#define GetWindowLongPtr	GetWindowLong
#define GetWindowLongPtrA	GetWindowLongA
#define GetWindowLongPtrW	GetWindowLongW
#define SetClassLongPtr		SetClassLong
#define SetClassLongPtrA	SetClassLongA
#define SetClassLongPtrW	SetClassLongW
#define GetClassLongPtr		GetClassLong
#define GetClassLongPtrA	GetClassLongA
#define GetClassLongPtrW	GetClassLongW

#define GWLP_WNDPROC		GWL_WNDPROC
#define GWLP_HINSTANCE		GWL_HINSTANCE
#define GWLP_ID				GWL_ID
#define GWLP_USERDATA		GWL_USERDATA

#define DWLP_DLGPROC		DWL_DLGPROC
#define DWLP_MSGRESULT		DWL_MSGRESULT
#define DWLP_USER			DWL_USER

#define GCLP_MENUNAME		GCL_MENUNAME
#define GCLP_HBRBACKGROUND	GCL_HBRBACKGROUND
#define GCLP_HCURSOR		GCL_HCURSOR
#define GCLP_HICON			GCL_HICON
#define GCLP_HMODULE		GCL_HMODULE
#define GCLP_WNDPROC		GCL_WNDPROC
#define GCLP_HICONSM		GCL_HICONSM
#endif

