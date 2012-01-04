/*-------------------------------------------------------------
  utl.c : misc functions
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

extern HINSTANCE g_hInst;
extern char g_mydir[];

/*---------------------------------------------
   add a file name to a path
   C:\aaa\bbb + ccc -> C:\aaa\bbb\ccc
----------------------------------------------*/
void add_title(char *path, const char *title)
{
	char *dp = path;
	const char *sp = title;
	
	if(*dp == 0) ;
	else if(*sp == '\\' && *(sp + 1) == '\\') ;
	else if(*sp == '\\')
	{
		if(*dp && *(dp + 1) == ':') dp += 2;
	}
	else
	{
		while(*sp)
		{
			if(*sp == ':') break;
			sp++;
		}
		
		if(*sp == 0)
		{
			while(*dp)
			{
				if((*dp == '\\' || *dp == '/') && *(dp + 1) == 0)
				{
					break;
				}
				dp = CharNext(dp);
			}
			*dp++ = '\\';
		}
	}
	sp = title;
	while(*sp) *dp++ = *sp++;
	*dp = 0;
}

/*-------------------------------------------
  remove a file name from a path
---------------------------------------------*/
void del_title(char *path)
{
	char *p, *ep;

	p = ep = path;
	while(*p)
	{
		if(*p == '\\' || *p == '/')
		{
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	*ep = 0;
}

/*------------------------------------------------
  compare file extensions
--------------------------------------------------*/
int ext_cmp(const char *fname, const char *ext)
{
	const char* p, *sp;
	
	sp = NULL; p = fname;
	while(*p)
	{
		if(*p == '.') sp = p;
		else if(*p == '\\' || *p == '/') sp = NULL;
		p = CharNext(p);
	}
	
	if(sp == NULL) sp = p;
	if(*sp == '.') sp++;
	
	while(1)
	{
		if(*sp == 0 && *ext == 0) break;
		if(toupper(*sp) != toupper(*ext))
			return (toupper(*sp) - toupper(*ext));
		sp++; ext++;
	}
	return 0;
}

/*------------------------------------------------
  retrieve and delete file extension
--------------------------------------------------*/
void del_ext(char* ext, char *fname)
{
	char* p, *sp;
	
	sp = NULL; p = fname;
	while(*p)
	{
		if(*p == '.') sp = p;
		else if(*p == '\\' || *p == '/') sp = NULL;
		p = CharNext(p);
	}
	
	if(sp == NULL) sp = p;
	if(*sp == '.') { *sp = 0; sp++; }
	
	while(*sp) *ext++ = *sp++;
	*ext = 0;
}

/*------------------------------------------------
  retreive a token from a string with comma delimiter 
--------------------------------------------------*/
int parse(char *dst, const char *src, int n, int nMax)
{
	char *dp;
	int i;
	
	for(i = 0; i < n; i++)
	{
		while(*src && *src != ',') src++;
		if(*src == ',') src++;
	}
	if(*src == 0)
	{
		*dst = 0; return 1;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	for(i = 0; *src && *src != ',' && i < nMax - 1; i++)
		*dst++ = *src++;
	*dst = 0;
	
	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
	
	return 0;
}

/*------------------------------------------------
  retreive a token from a string with space delimiter 
--------------------------------------------------*/
void parsespace(char *dst, const char *src, int n, int nMax)
{
	char quot;
	int i;
	
	for(i = 0; i < n; i++)
	{
		quot = 0;
		if(*src == '\"' || *src == '\'')
		{
			quot = *src; src++;
		}
		while(*src && *src != '\r' && *src != '\n')
		{
			if(quot)
			{
				if(*src == quot) { src++; break; }
			}
			else if(*src == ' ' || *src == '\t') break;
			src++;
		}
		while(*src == ' ' || *src == '\t') src++;
	}
	
	quot = 0;
	if(*src == '\"' || *src == '\'')
	{
		quot = *src; src++;
	}
	for(i = 0; i < nMax - 1 &&
		*src && *src != '\r' && *src != '\n'; i++)
	{
		if(quot)
		{
			if(*src == quot) break;
		}
		else if(*src == ' ' || *src == '\t') break;
		*dst++ = *src++;
	}
	*dst = 0;
}

/*------------------------------------------------
   aaa\0bbb\0 + ccc\0 -> aaa\0bbb\0ccc\0\0
--------------------------------------------------*/
void str0cat(char* dst, const char* src)
{
	char* p;
	p = dst;
	while(*p) { while(*p) p++; p++; }
	strcpy(p, src);
	while(*p) p++; p++; *p = 0;
}

/*------------------------------------------------
  the string consists of digits?
--------------------------------------------------*/
int isdigitstr(const char *p)
{
	if(*p == 0) return 0;
	while(*p)
	{
		if(!('0' <= *p && *p <= '9')) return 0;
		p++;
	}
	return 1;
}

/*------------------------------------------------
  advance pointer to next line
--------------------------------------------------*/
const char* nextline(const char* p)
{
	while(*p && *p != '\r' && *p != '\n') p++;
	if(*p == '\r')
	{
		p++;
		if(*p == '\n') p++;
	}
	else if(*p) p++;
	return p;
}

/*------------------------------------------------
  return taskbar window handle
--------------------------------------------------*/
HWND GetTaskbarWindow(void)
{
	return FindWindow("Shell_TrayWnd", "");
}

/*------------------------------------------------
  return clock window handle
--------------------------------------------------*/
HWND GetClockWindow(void)
{
	HWND hwnd;
	
	// task bar
	hwnd = GetTaskbarWindow();
	if(!hwnd) return NULL;
	// tray
	hwnd = FindWindowEx(hwnd, NULL, "TrayNotifyWnd", NULL);
	if(!hwnd) return NULL;
	// clock
	hwnd = FindWindowEx(hwnd, NULL, "TrayClockWClass", NULL);
	return hwnd;
}

/*------------------------------------------------
   return TClock main window handle
--------------------------------------------------*/
HWND GetTClockMainWindow(void)
{
	return FindWindow(CLASS_TCLOCKMAIN, TITLE_TCLOCKMAIN);
}

/*------------------------------------------------
  send a string to other module
--------------------------------------------------*/
void SendStringToOther(HWND hwnd, HWND hwndFrom,
	const char *s, int type)
{
	COPYDATASTRUCT cds;
	
	cds.dwData = type;
	cds.cbData = strlen(s) + 1;
	cds.lpData = (LPVOID)s;
	
	if(hwnd && IsWindow(hwnd))
		SendMessage(hwnd, WM_COPYDATA, (WPARAM)hwndFrom,
			(LPARAM)&cds);
}

void SendStringToOtherW(HWND hwnd, HWND hwndFrom,
	const wchar_t *s, int type)
{
	COPYDATASTRUCT cds;
	
	cds.dwData = type;
	cds.cbData = (wcslen(s) + 1) * sizeof(wchar_t);
	cds.lpData = (LPVOID)s;
	
	if(hwnd && IsWindow(hwnd))
		SendMessage(hwnd, WM_COPYDATA, (WPARAM)hwndFrom,
			(LPARAM)&cds);
}

/*------------------------------------------------
  the file exists ?
--------------------------------------------------*/
BOOL IsFile(const char* fname)
{
	DWORD ret;
	
	ret = GetFileAttributes(fname);
	if(ret == (DWORD)-1)
	{
		return FALSE;
	}
	return (ret & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

/*------------------------------------------------
  the directory exists ?
--------------------------------------------------*/
BOOL IsDirectory(const char* fname)
{
	DWORD ret;
	
	ret = GetFileAttributes(fname);
	if(ret == (DWORD)-1)
	{
		return FALSE;
	}
	return (ret & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

/*-------------------------------------------
  check Windows 95/98/Me/NT4/2000/XP/Vista
---------------------------------------------*/
int CheckWinVersion(void)
{
	DWORD dw;
	WORD ver, w;
	int ret;
	
	dw = GetVersion();
	w = LOWORD(dw);
	ver = MAKEWORD(HIBYTE(w), LOBYTE(w));
	ret = 0;
	if(dw & 0x80000000)
	{
		ret |= WIN95;
		if(ver >= MAKEWORD(10, 4))	// 4.10
			ret |= WIN98;
		if(ver >= MAKEWORD(90, 4))	// 4.90
			ret |= WINME;
	}
	else
	{
		ret |= WINNT;
		if(ver >= MAKEWORD(0, 5))	// 5.0
			ret |= WIN2000;
		if(ver >= MAKEWORD(1, 5))	// 5.1
			ret |= WINXP;
		if(ver >= MAKEWORD(0, 6))	// 6.0
			ret |= WINVISTA;
	//	if(ver >= MAKEWORD(1, 6))	// 6.1
	//		ret |= WIN7;
	}
	
	return ret;
}

/*------------------------------------------------
  IE 4 or later ?
--------------------------------------------------*/
BOOL IsIE4(void)
{
	HWND hwnd;
	DWORD dw;
	
	dw = GetRegLong(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
		"ClassicShell", 0);
	if(dw) return TRUE;
	
	hwnd = FindWindow("Shell_TrayWnd", NULL);
	if(hwnd == NULL) return FALSE;
	hwnd = FindWindowEx(hwnd, NULL, "ReBarWindow32", NULL);
	if(hwnd != NULL) return TRUE;
	return FALSE;
}

/*-------------------------------------------
  using XP Theme ?
---------------------------------------------*/
BOOL IsXPVisualStyle(void)
{
	char s[10];
	
	if(GetRegStr(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager",
		"ThemeActive", s, 10, "") > 0)
	{
		if(strcmp(s, "0") != 0) return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------
  using Vista Aero ?
---------------------------------------------*/
typedef HRESULT (WINAPI *pfnDwmIsCompositionEnabled)(BOOL *);
static HRESULT WINAPI DwmIsCompositionEnabledStub(BOOL *pfEnabled);

BOOL IsVistaAero(void)
{
#if 1
	static pfnDwmIsCompositionEnabled pDwmIsCompositionEnabled = NULL;
	BOOL ret = FALSE;
	
	if (pDwmIsCompositionEnabled == NULL) {
	//	HMODULE hDwmApi = LoadLibrary("dwmapi.dll");
		HMODULE hDwmApi = GetModuleHandle("dwmapi.dll");
		if (hDwmApi != NULL) {
			pDwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)
					GetProcAddress(hDwmApi, "DwmIsCompositionEnabled");
		}
		if (pDwmIsCompositionEnabled == NULL) {
			pDwmIsCompositionEnabled = DwmIsCompositionEnabledStub;
		}
	}
	
	pDwmIsCompositionEnabled(&ret);
	return ret;
#else
	if(GetRegLong(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\DWM",
		"Composition", 0))
	{
		return TRUE;
	}
	return FALSE;
#endif
}

static HRESULT WINAPI DwmIsCompositionEnabledStub(BOOL *pfEnabled)
{
	if (pfEnabled != NULL) {
		*pfEnabled = FALSE;
	}
	return S_OK;
}


/*-------------------------------------------
  using Taskbar Animations ?
---------------------------------------------*/
BOOL IsTaskbarAnimation(void)
{
	if(GetRegLong(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
		"TaskbarAnimations", 0))
	{
		return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------
  SetForegroundWindow for Windows98
---------------------------------------------*/
void SetForegroundWindow98(HWND hwnd)
{
	DWORD dwVer;
	
	dwVer = GetVersion();
	if(((dwVer & 0x80000000) && 
	       LOBYTE(LOWORD(dwVer)) >= 4 && HIBYTE(LOWORD(dwVer)) >= 10) ||
	   (!(dwVer & 0x80000000) && LOBYTE(LOWORD(dwVer)) >= 5)) // Win98/2000
	{
		DWORD thread1, thread2;
		DWORD pid;
		thread1 = GetWindowThreadProcessId(
			GetForegroundWindow(), &pid);
		thread2 = GetCurrentThreadId();
		AttachThreadInput(thread2, thread1, TRUE);
		SetForegroundWindow(hwnd);
		AttachThreadInput(thread2, thread1, FALSE);
		BringWindowToTop(hwnd);
	}
	else  // Win95/NT
		SetForegroundWindow(hwnd);
}

/*------------------------------------------------
   adjust the window position
--------------------------------------------------*/
void SetMyDialgPos(HWND hwnd, int xLen, int yLen)
{
	HWND hwndTray;
	RECT rc, rcTray;
	int wscreen, hscreen, wProp, hProp;
	int x, y;

	GetWindowRect(hwnd, &rc);
	wProp = rc.right - rc.left;
	hProp = rc.bottom - rc.top;
	
	wscreen = GetSystemMetrics(SM_CXSCREEN);
	hscreen = GetSystemMetrics(SM_CYSCREEN);
	
	hwndTray = FindWindow("Shell_TrayWnd", NULL);
	if(hwndTray == NULL) return;
	GetWindowRect(hwndTray, &rcTray);
	if(rcTray.right - rcTray.left > 
		rcTray.bottom - rcTray.top)
	{
		x = wscreen - wProp - xLen;
		if(rcTray.top < hscreen / 2)
			y = rcTray.bottom + yLen;
		else
			y = rcTray.top - hProp - yLen;
		if(y < 0) y = 0;
	}
	else
	{
		y = hscreen - hProp - yLen;
		if(rcTray.left < wscreen / 2)
			x = rcTray.right + xLen;
		else
			x = rcTray.left - wProp - xLen;
		if(x < 0) x = 0;
	}
	
//	MoveWindow(hwnd, x, y, wProp, hProp, FALSE);
	SetWindowPos(hwnd, NULL, x, y, wProp, hProp,
		SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREDRAW);
}

/*-------------------------------------------
  for debugging
---------------------------------------------*/
void WriteDebug(const char* s)
{
	HFILE hf;
	char fname[MAX_PATH], *title = "DEBUG.TXT";

	GetModuleFileName(g_hInst, fname, MAX_PATH);
	del_title(fname);
	add_title(fname, title);
	hf = _lopen(fname, OF_WRITE);
	if(hf == HFILE_ERROR)
		hf = _lcreat(fname, 0);
	if(hf == HFILE_ERROR) return;
	_llseek(hf, 0, 2);
	_lwrite(hf, s, strlen(s));
	_lwrite(hf, "\x0d\x0a", 2);
	_lclose(hf);
}

void WriteDebugW(const wchar_t* s)
{
	HFILE hf;
	char fname[MAX_PATH], *title = "DEBUG.TXT";

	GetModuleFileName(g_hInst, fname, MAX_PATH);
	del_title(fname);
	add_title(fname, title);
	hf = _lopen(fname, OF_WRITE);
	if(hf == HFILE_ERROR)
		hf = _lcreat(fname, 0);
	if(hf == HFILE_ERROR) return;
	_llseek(hf, 0, 2);
	_lwrite(hf, (LPCSTR)s, wcslen(s)*sizeof(wchar_t));
	_lwrite(hf, (LPCSTR)L"\x0d\x0a", wcslen(L"\x0d\x0a")*sizeof(wchar_t));
	_lclose(hf);
}

