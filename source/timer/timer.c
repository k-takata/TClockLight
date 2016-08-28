/*-------------------------------------------------------------
  timer.c : executing timers
  (C) Kazuto Sato 1997-2003
  For the license, please read readme.txt.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tctimer.h"

/* Statics */

static void DoTimer(HWND hwnd, PTIMERSTRUCT pitem);
static void DeleteRunningTimer(HWND hwnd, PTIMERSTRUCT pitem);

static PTIMERSTRUCT m_pTimerRun = NULL;
static int m_idCurrent = 1;

/*-------------------------------------------
  add a timer to m_pTimerRun and start
---------------------------------------------*/
void TimerStart(const PTIMERSTRUCT pitem)
{
	PTIMERSTRUCT pnewitem;
	
	if(!pitem) return;
	
	pnewitem = malloc(sizeof(TIMERSTRUCT));
	memcpy(pnewitem, pitem, sizeof(TIMERSTRUCT));
	
	pnewitem->interval = pnewitem->minute * 60 + pnewitem->second;
	pnewitem->tickonstart = GetTickCount();
	pnewitem->id = m_idCurrent++;
	
	m_pTimerRun = add_listitem(m_pTimerRun, pnewitem);
}

/*-------------------------------------------
  clear all timers
---------------------------------------------*/
void ClearTimer(void)
{
	clear_list(m_pTimerRun);
	m_pTimerRun = NULL;
}

/*-------------------------------------------
  timer is running ?
---------------------------------------------*/
BOOL IsTimerRunning(void)
{
	return (m_pTimerRun != NULL);
}

/*-------------------------------------------
  WM_TIMER message
---------------------------------------------*/
void OnTimerTimer(HWND hwnd)
{
	PTIMERSTRUCT pitem, pDo;
	DWORD tick;
	int nCopyData;
	char s[20];
	wchar_t ws[20];
	
	if(!m_pTimerRun) return;
	
	tick = GetTickCount();
	pDo = NULL;
	pitem = m_pTimerRun;
	while(pitem)
	{
		if(pitem->bDisp)
		{
			int remaining =
				pitem->interval - (tick - pitem->tickonstart)/1000 - 1;
			if(remaining >= 0)
			{
				if(pitem->nDispType == 0)
					nCopyData = COPYDATA_DISP1;
				else if(pitem->nDispType == 2)
					nCopyData = COPYDATA_USTR0 + pitem->nUserStr;
				else
					nCopyData = COPYDATA_CAT1;
				wsprintf(s, "[%02d:%02d]", remaining/60, remaining%60);
				
				MultiByteToWideChar(CP_ACP, 0, s, -1, ws, 19);
				SendStringToOtherW(g_hwndClock, hwnd, ws, nCopyData);
			}
		}
		
		if(pDo == NULL &&
			tick - pitem->tickonstart > pitem->interval * 1000)
			pDo = pitem;
		
		pitem = pitem->next;
	}
	
	if(pDo) DoTimer(hwnd, pDo);
}

/*-------------------------------------------
  add item to tcmenu*.txt
---------------------------------------------*/
void OnRequestMenu(HWND hwnd, BOOL bClear)
{
	char tcmenutxt[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	HFILE hf;
	int size;
	char *buf;
	const char *p, *np;
	static BOOL bWrite = FALSE;
	
	if(!bClear && m_pTimerRun == NULL) return;
	
	if(bClear && !bWrite) return;
	
	// ../common/tclang.c
	FindFileWithLangCode(tcmenutxt, GetUserDefaultLangID(), TCMENUTXT);
	
	hfind = FindFirstFile(tcmenutxt, &fd);
	if(hfind == INVALID_HANDLE_VALUE) return;
	
	FindClose(hfind);
	size = (int)fd.nFileSizeLow;
	buf = malloc(size+1);
	if(!buf) return;
	
	hf = _lopen(tcmenutxt, OF_READWRITE);
	if(hf == HFILE_ERROR) { free(buf); return; }
	_lread(hf, buf, size);
	*(buf + size) = 0;
	
	_llseek(hf, 0, 0);
	
	p = buf;
	while(*p)
	{
		if(strncmp(p, "#Timer Begin", 12) == 0)
		{
			PTIMERSTRUCT pitem;
			DWORD tick;
			char s[160];
			
			np = nextline(p);
			_lwrite(hf, p, (int)(np - p));
			p = np;
			
			tick = GetTickCount();
			
			pitem = m_pTimerRun;
			while(pitem)
			{
				int remaining =
					pitem->interval - (tick - pitem->tickonstart)/1000 - 1;
				
				wsprintf(s, "\"%s %s %02d:%02d\" post %s %d 0 %d\r\n",
					MyString(IDS_STOP, "Stop"),
					pitem->name, remaining/60, remaining%60,
					CLASS_TCLOCKTIMER, TIMERM_STOP, pitem->id);
				_lwrite(hf, s, (int)strlen(s));
				
				pitem = pitem->next;
			}
			
			while(*p)
			{
				if(strncmp(p, "#Timer End", 10) == 0)
					break;
				p = nextline(p);
			}
		}
		else
		{
			np = nextline(p);
			_lwrite(hf, p, (int)(np - p));
			p = np;
		}
	}
	
	_lwrite(hf, NULL, 0); // truncate
	
	_lclose(hf);
	free(buf);
	
	bWrite = TRUE;
}

/*-------------------------------------------
  WM_TIMER message
---------------------------------------------*/
void OnStopTimer(HWND hwnd, int id)
{
	PTIMERSTRUCT pitem;
	
	pitem = m_pTimerRun;
	while(pitem)
	{
		if(pitem->id == id)
		{
			DeleteRunningTimer(hwnd, pitem);
			break;
		}
		pitem = pitem->next;
	}
}

/*-------------------------------------------
  execute timer
---------------------------------------------*/
void DoTimer(HWND hwnd, PTIMERSTRUCT pitem)
{
	char command[MAX_PATH + 10];
	HWND hwndMain;
	
	if(!pitem) return;
	
	command[0] = 0;
	if(pitem->bRepeat) strcpy(command, "-1 ");
	strcat(command, pitem->fname);
	hwndMain = GetTClockMainWindow();
	if(hwndMain)
		SendStringToOther(hwndMain, hwnd, command, COPYDATA_SOUND);
	
	if(pitem->bBlink)
		PostMessage(g_hwndClock, CLOCKM_BLINK, 0, 0);
	
	DeleteRunningTimer(hwnd, pitem);
}

/*-------------------------------------------
  delete an executed timer from array
---------------------------------------------*/
void DeleteRunningTimer(HWND hwnd, PTIMERSTRUCT pitem)
{
	if(!pitem) return;
	
	if(pitem->bDisp)
	{
		int nCopyData;
		if(pitem->nDispType == 0)
			nCopyData = COPYDATA_DISP1;
		else if(pitem->nDispType == 2)
			nCopyData = COPYDATA_USTR0 + pitem->nUserStr;
		else
			nCopyData = COPYDATA_CAT1;
		SendStringToOtherW(g_hwndClock, hwnd, L"", nCopyData);
	}
	
	// common/list.c
	m_pTimerRun = del_listitem(m_pTimerRun, pitem);
	
	OnRequestMenu(hwnd, TRUE);
	
	// exit if there is no timer
	if(m_pTimerRun == NULL && !(g_hDlg && IsWindow(g_hDlg)))
		PostMessage(hwnd, WM_CLOSE, 0, 0);
}
