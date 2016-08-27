/*-------------------------------------------------------------
  cmdopt.c : process command line options
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

/* Globals */
void CheckCommandLine(HWND hwnd, BOOL bPrev);
void SendStringToClock(HWND hwndClock, const char *value, int type);

/*-------------------------------------------
   process command line option
---------------------------------------------*/
void CheckCommandLine(HWND hwnd, BOOL bPrev)
{
	HWND hwndClock;
	char name[20], value[MAX_PATH];
	BOOL bquot;
	char *p;
	int i;
	
	hwndClock = GetClockWindow();
	
	p = GetCommandLine();
	
	while(*p)
	{
		if(*p == '/')
		{
			p++;
			for(i = 0; *p && *p != ' ' && *p != '\"' && i < 19; i++)
			{
				name[i] = *p++;
			}
			name[i] = 0;
			while(*p == ' ') p++;
			
			value[0] = 0;
			if(*p && *p != '/')
			{
				bquot = FALSE;
				if(*p == '\"') { bquot = TRUE; p++; }
				for(i = 0; *p && i < MAX_PATH-1; i++)
				{
					if(bquot) { if(*p == '\"') break; }
					else { if(*p == ' ') break; }
					value[i] = *p++;
				}
				value[i] = 0;
				if(bquot && *p == '\"') p++;
			}
			
			if(strcmp(name, "prop") == 0)
			{
				ExecFile(hwnd, "tcprop.exe");
			}
			else if(strcmp(name, "exit") == 0)
			{
				if(bPrev && hwndClock)
					PostMessage(hwndClock, CLOCKM_EXIT, 0, 0);
			}
			else if(strcmp(name, "blink") == 0)
			{
				if(bPrev && hwndClock)
				{
					PostMessage(hwndClock, CLOCKM_BLINK, 0,
						atoi(value));
				}
			}
			else if(strncmp(name, "ustr", 4) == 0)
			{
				int n = name[4] - '0';
				
				if(0 <= n && n <= 9 && bPrev && hwndClock)
					SendStringToClock(hwndClock, value,
						COPYDATA_USTR0 + n);
			}
			else if(strcmp(name, "disp1") == 0)
			{
				if(bPrev && hwndClock)
					SendStringToClock(hwndClock, value, COPYDATA_DISP1);
			}
			else if(strcmp(name, "disp2") == 0)
			{
				if(bPrev && hwndClock)
					SendStringToClock(hwndClock, value, COPYDATA_DISP2);
			}
			else if(strcmp(name, "cat1") == 0)
			{
				if(bPrev && hwndClock)
					SendStringToClock(hwndClock, value, COPYDATA_CAT1);
			}
			else if(strcmp(name, "cat2") == 0)
			{
				if(bPrev && hwndClock)
					SendStringToClock(hwndClock, value, COPYDATA_CAT2);
			}
			else if(strcmp(name, "sound") == 0)
			{
				if(bPrev)
					SendStringToOther(hwnd, NULL, value, COPYDATA_SOUND);
				else PlayFileCmdLine(hwnd, value);
			}
			else if(strcmp(name, "tip") == 0)
			{
				if(bPrev && hwndClock)
					SendStringToClock(hwndClock, value, COPYDATA_TOOLTIP);
			}
			else if(strcmp(name, "cmd") == 0)
			{
				if(bPrev)
				{
					int n = atoi(value);
					if(n >= 100)
						PostMessage(hwnd, WM_COMMAND, n, 0);
				}
			}
			else if(strcmp(name, "nowait") == 0)
			{
				if(!bPrev && GetMyRegLong(NULL, "DelayStart", 0) > 0)
				{
					KillTimer(hwnd, IDTIMER_START);
					SetTimer(hwnd, IDTIMER_START, 100, NULL);
				}
			}
			else if(strcmp(name, "idle") == 0)
			{
				if(!bPrev)
				{
					HANDLE op = OpenProcess(PROCESS_ALL_ACCESS,
						TRUE, GetCurrentProcessId());
					SetPriorityClass(op, IDLE_PRIORITY_CLASS);
					Sleep(10);
				}
			}
		}
		else p++;
	}
}

/*------------------------------------------------
  wrapper function of SendStringToOtherW
--------------------------------------------------*/
void SendStringToClock(HWND hwndClock, const char *value, int type)
{
	wchar_t wtemp[BUFSIZE_TOOLTIP], wvalue[BUFSIZE_TOOLTIP], *sp;
	int i;
	
	MultiByteToWideChar(CP_ACP,
		0, value, -1, wtemp, BUFSIZE_DISP-1);
	sp = wtemp;
	for(i = 0; i < BUFSIZE_TOOLTIP-1 && *sp; i++)
	{
		// line break
		if(i < BUFSIZE_TOOLTIP-2 && *sp == '\\' && *(sp + 1) == 'n')
		{
			wvalue[i++] = 0x0d; wvalue[i] = 0x0a;
			sp += 2;
		}
		// unicode charactor
		else if(*sp == '\\' && *(sp + 1) == 'x')
		{
			wchar_t ch = 0;
			
			sp += 2;
			while(*sp)
			{
				if('0' <= *sp && *sp <= '9')
					ch = (wchar_t)(ch * 16 + *sp - '0');
				else if('A' <= *sp && *sp <= 'F')
					ch = (wchar_t)(ch * 16 + *sp - 'A' + 10);
				else if('a' <= *sp && *sp <= 'f')
					ch = (wchar_t)(ch * 16 + *sp - 'a' + 10);
				else
				{
					if(*sp == ';') sp++;
					break;
				}
				sp++;
			}
			if(ch == 0) ch = ' ';
			wvalue[i] = ch;
		}
		else
			wvalue[i] = *sp++;
	}
	wvalue[i] = 0;
	
	SendStringToOtherW(hwndClock, NULL, wvalue, type);
}

