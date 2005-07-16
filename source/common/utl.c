/*-------------------------------------------------------------
  utl.c : misc functions
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

extern HINSTANCE g_hInst;

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
	
	while(*sp || *ext)
	{
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
void SendStringToOther(HWND hwnd, HWND hwndFrom, const char *s, int type)
{
	if(hwnd && IsWindow(hwnd))
	{
		COPYDATASTRUCT cds;
		
		cds.dwData = type;
		cds.cbData = strlen(s) + 1;
		cds.lpData = (LPVOID)s;
		
		SendMessage(hwnd, WM_COPYDATA, (WPARAM)hwndFrom, (LPARAM)&cds);
	}
}

void SendStringToOtherW(HWND hwnd, HWND hwndFrom, const wchar_t *s, int type)
{
	if(hwnd && IsWindow(hwnd))
	{
		COPYDATASTRUCT cds;
		
		cds.dwData = type;
		cds.cbData = (wcslen(s) + 1) * sizeof(wchar_t);
		cds.lpData = (LPVOID)s;
		
		SendMessage(hwnd, WM_COPYDATA, (WPARAM)hwndFrom, (LPARAM)&cds);
	}
}

/*------------------------------------------------
  the file exists ?
--------------------------------------------------*/
BOOL IsFile(const char* fname)
{
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	hfind = FindFirstFile(fname, &fd);
	if(hfind != INVALID_HANDLE_VALUE)
	{
		FindClose(hfind); return TRUE;
	}
	return FALSE;
}

/*-------------------------------------------
  check Windows 95/98/Me/NT4/2000/XP
---------------------------------------------*/
int CheckWinVersion(void)
{
	OSVERSIONINFO info;
	int ret;

	info.dwOSVersionInfoSize = sizeof (info);
	if (GetVersionEx(&info))
	{
		if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			ret = WINNT;
			if (info.dwMajorVersion == 5)
			{
				ret |= WIN2000;
				if (info.dwMinorVersion == 1)
					ret |= WINXP;
			}
		}
		else
		{
			ret = WIN95;
			if (info.dwMinorVersion == 10)
				ret |= WIN98;
			else if (info.dwMinorVersion == 90)
				ret |= WIN98|WINME;
		}
	}
	else
		ret = 0;

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
	
	hwnd = GetTaskbarWindow();
	if(hwnd)
		return (FindWindowEx(hwnd, NULL, "ReBarWindow32", NULL) != NULL);
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
  SetForegroundWindow for Windows98
---------------------------------------------*/
void SetForegroundWindow98(HWND hwnd)
{
	int ver;
	
	ver = CheckWinVersion();
	if((ver&WIN2000) || (ver&WIN98))
	{
		DWORD thread1, thread2;
		DWORD pid;
		thread1 = GetWindowThreadProcessId(GetForegroundWindow(), &pid);
		thread2 = GetCurrentThreadId();
		AttachThreadInput(thread2, thread1, TRUE);
		SetForegroundWindow(hwnd);
		AttachThreadInput(thread2, thread1, FALSE);
		// BringWindowToTop(hwnd);
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
	
	hwndTray = GetTaskbarWindow();
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
	
	MoveWindow(hwnd, x, y, wProp, hProp, FALSE);
}

/*-------------------------------------------
  for debugging
---------------------------------------------*/
void WriteDebug(const char* s)
{
	HANDLE hf;
	DWORD dwWritten;
	char fname[MAX_PATH], *title = "DEBUG.TXT";

	GetModuleFileName(g_hInst, fname, MAX_PATH);
	del_title(fname);
	add_title(fname, title);
	hf = CreateFile(fname, GENERIC_WRITE, 0,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hf == INVALID_HANDLE_VALUE)
		return;
	SetFilePointer(hf, 0, NULL, FILE_END);
	WriteFile(hf, s, strlen(s), &dwWritten, NULL);
	WriteFile(hf, "\x0d\x0a", 2, &dwWritten, NULL);
	CloseHandle(hf);
}

void WriteDebugW(const wchar_t* s)
{
	char *temp;
	int len;
	len = WideCharToMultiByte(CP_ACP, 0, s, -1, NULL, 0, NULL, NULL);
	temp = malloc(len);
	WideCharToMultiByte(CP_ACP, 0, s, -1, temp, len, NULL, NULL);
	WriteDebug(temp);
	free(temp);
}

