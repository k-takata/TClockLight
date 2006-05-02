/*-------------------------------------------------------------
  mouse.c : mouse clicks
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"
#include "../common/command.h"

/* Globals */

void InitMouseFunction(HWND hwnd);
void EndMouseFunction(HWND hwnd);
void OnMouseDown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnMouseUp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnTimerMouse(HWND hwnd);
void OnMouseWheel(HWND hwnd, WPARAM wParam, LPARAM lParam);

/* Statics */

static void ExecuteMouseFunction(HWND hwnd, const PMOUSESTRUCT pMSS);
static PMOUSESTRUCT GetMouseCommand(int button, int nclick);

static const char *m_section = "Mouse";
static DWORD m_tickLast;
static int  m_nClick = 0;       // clicking count
static int  m_nButton = -1;     // last clicked button
static BOOL m_bDown = FALSE;    // last message is DOWN
static BOOL m_bTimer = FALSE;
static UINT m_msecDoubleClick;
static BOOL m_bStartMenuFromClock;
static BOOL m_bRClickMenu = FALSE;

static PMOUSESTRUCT m_pMouseCommand = NULL;

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void InitMouseFunction(HWND hwnd)
{
	m_tickLast = GetTickCount();
	
	// Mouse double click speed
	m_msecDoubleClick = GetMyRegLong(m_section, "DoubleClickSpeed", 0);
	if (m_msecDoubleClick == 0)
		m_msecDoubleClick = GetDoubleClickTime();
	
	m_bStartMenuFromClock =
		GetMyRegLong("StartButton", "Hide", FALSE) &&
		GetMyRegLong("StartButton", "StartMenuClock", FALSE);
	
	m_bRClickMenu = GetMyRegLong(m_section, "RightClickMenu",
		GetMyRegLong(NULL, "RightClickMenu", TRUE));
	
	if(!GetMyRegLong(m_section, "ver031031", FALSE) ||
		GetMyRegLong(m_section, "ver230", FALSE))
	{
		ImportOldMouseFunc(); // common/mousestruct.c
		DelMyReg(m_section, "ver230");
		SetMyRegLong(m_section, "ver031031", TRUE);
	}
	
	// common/list.c
	clear_list(m_pMouseCommand);
	
	// common/mousestruct.c
	m_pMouseCommand = LoadMouseFunc();
}

/*------------------------------------------------
   clean up
--------------------------------------------------*/
void EndMouseFunction(HWND hwnd)
{
	if(m_bTimer) KillTimer(hwnd, IDTIMER_MOUSE);
	m_bTimer = FALSE;
	
	// common/list.c
	clear_list(m_pMouseCommand);
	m_pMouseCommand = NULL;
}

/*------------------------------------------------------------
   when mouse button is down on the clock
--------------------------------------------------------------*/
void OnMouseDown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int button;
	int i;
	PMOUSESTRUCT pMSS;
	BOOL bMoreClick;
	DWORD tick;
	
	if(m_bTimer) KillTimer(hwnd, IDTIMER_MOUSE);
	m_bTimer = FALSE;
	
	if(message == WM_RBUTTONDOWN && m_bRClickMenu)
		return;
	
	if(message == WM_LBUTTONDOWN && m_bStartMenuFromClock)
		return;
	
	if(!m_pMouseCommand) return;
	
	if(message == WM_LBUTTONDOWN)      button = 0;
	else if(message == WM_RBUTTONDOWN) button = 1;
	else if(message == WM_MBUTTONDOWN) button = 2;
	else if(message == WM_XBUTTONDOWN)
	{
		if(HIWORD(wParam) == XBUTTON1) button = 3;
		else if(HIWORD(wParam) == XBUTTON2) button = 4;
		else return;
	}
	else return;
	
	if(m_nButton != button || m_bDown)
		m_nClick = 0;
	
	m_nButton = button;
	m_bDown = TRUE;
	
	tick = GetTickCount();
	if(tick - m_tickLast > m_msecDoubleClick)
		m_nClick = 0;
	m_tickLast = tick;
	
	pMSS = GetMouseCommand(button, m_nClick + 1);
	if(!pMSS || pMSS->nCommand == IDC_SCREENSAVER
		|| pMSS->nCommand == IDC_MONOFF)
		return;
	
	bMoreClick = FALSE;
	for(i = m_nClick + 2; i <= 4; i++)
	{
		if(GetMouseCommand(button, i))
		{
			bMoreClick = TRUE; break;
		}
	}
	
	if(!bMoreClick)
	{
		m_nClick++;
		ExecuteMouseFunction(hwnd, pMSS);
	}
}

/*------------------------------------------------------------
   when mouse button is up on the clock
--------------------------------------------------------------*/
void OnMouseUp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int button;
	int i;
	PMOUSESTRUCT pMSS;
	BOOL bMoreClick;
	DWORD tick;
	
	if(m_bTimer) KillTimer(hwnd, IDTIMER_MOUSE);
	m_bTimer = FALSE;
	
	if(message == WM_RBUTTONUP && (m_bRClickMenu || !m_pMouseCommand))
	{
		POINT pt;
		pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		ClientToScreen(GetClockWindow(), &pt);
		OnContextMenu(hwnd, NULL, pt.x, pt.y);
		return;
	}
	
	if(message == WM_LBUTTONUP && m_bStartMenuFromClock)
		return;
	
	if(!m_pMouseCommand) return;
	
	if(message == WM_LBUTTONUP)      button = 0;
	else if(message == WM_RBUTTONUP) button = 1;
	else if(message == WM_MBUTTONUP) button = 2;
	else if(message == WM_XBUTTONUP)
	{
		if(HIWORD(wParam) == XBUTTON1) button = 3;
		else if(HIWORD(wParam) == XBUTTON2) button = 4;
		else return;
	}
	else return;
	
	if((m_nClick > 0 && m_nButton != button) || !m_bDown)
	{
		m_nClick = 0; m_nButton = -1; m_bDown = FALSE;
		return;
	}
	
	m_nButton = button;
	m_bDown = FALSE;
	
	tick = GetTickCount();
	if(tick - m_tickLast > m_msecDoubleClick)
		m_nClick = 0;
	m_tickLast = tick;
	
	m_nClick++;
	
	pMSS = GetMouseCommand(button, m_nClick);
	if(!pMSS) return;
	
	bMoreClick = FALSE;
	for(i = m_nClick + 1; i <= 4; i++)
	{
		if(GetMouseCommand(button, i))
		{
			bMoreClick = TRUE; break;
		}
	}
	
	if(bMoreClick)
	{
		m_bTimer = TRUE;
		SetTimer(hwnd, IDTIMER_MOUSE, m_msecDoubleClick, 0);
	}
	else
		ExecuteMouseFunction(hwnd, pMSS);
}

/*------------------------------------------------
   when the mouse wheel is rotated
--------------------------------------------------*/
void OnMouseWheel(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	int zDelta, xPos, yPos, button;
	RECT rcClock;
	PMOUSESTRUCT pMSS;
	
	GetWindowRect(g_hwndClock, &rcClock);
	xPos = GET_X_LPARAM(lParam); 
	yPos = GET_Y_LPARAM(lParam);
	if (!( xPos >= rcClock.left && xPos <= rcClock.right && yPos >= rcClock.top && yPos <= rcClock.bottom ))
		return;
	
	zDelta = (short) HIWORD(wParam);
	if (zDelta > 0)
		button = 5;
	else
		button = 6;
	
	pMSS = GetMouseCommand(button, 1);
	if(!pMSS) return;
	
	ExecuteMouseFunction(hwnd, pMSS);
}

/*------------------------------------------------
   execute mouse function
--------------------------------------------------*/
void OnTimerMouse(HWND hwnd)
{
	PMOUSESTRUCT pMSS;
	
	if(m_bTimer) KillTimer(hwnd, IDTIMER_MOUSE);
	m_bTimer = FALSE;
	
	pMSS = GetMouseCommand(m_nButton, m_nClick);
	if(pMSS) ExecuteMouseFunction(hwnd, pMSS);
}

/*------------------------------------------------
   execute mouse function
--------------------------------------------------*/
void ExecuteMouseFunction(HWND hwnd, const PMOUSESTRUCT pMSS)
{
	m_nClick = 0; m_nButton = -1; m_bDown = FALSE;
	
	switch (pMSS->nCommand)
	{
		case IDC_OPENFILE:
			if(pMSS->option[0])
			{
				DWORD pid, curthread, mythread;
				
				curthread = GetWindowThreadProcessId(
					GetForegroundWindow(), &pid);
				mythread = GetCurrentThreadId();
				AttachThreadInput(mythread, curthread, TRUE);
				SetFocus(hwnd);
				AttachThreadInput(mythread, curthread, FALSE);
				
				ExecCommandString(hwnd, pMSS->option);
			}
			break;
		case IDC_MOUSECOPY:
			if(pMSS->option[0]) CopyToClipBoard(hwnd, pMSS->option);
			break;
		case IDC_MONOFF:
		{
			int delay = atoi(pMSS->option);
			if (delay == 0)
				PostMessage(hwnd, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
			else
				SetTimer(hwnd, IDTIMER_MONOFF, delay * 1000, NULL);
			break;
		}
		case IDC_COMMAND:
		{
			int nCmd = atoi(pMSS->option);
			if(nCmd >= 100) PostMessage(hwnd, WM_COMMAND, nCmd, 0);
			break;
		}
		case IDC_VOLSET:
		{
			int vol = atoi(pMSS->option);
			SetMasterVolume(vol);
			PostMessage(g_hwndClock, CLOCKM_VOLCHANGE, TRUE, TRUE);
			break;
		}
		case IDC_VOLUD:
		{
			int vol = atoi(pMSS->option);
			UpDownMasterVolume(vol);
			PostMessage(g_hwndClock, CLOCKM_VOLCHANGE, TRUE, TRUE);
			break;
		}
		case IDC_MUTE:
		{
			ReverseMasterMute();
			PostMessage(g_hwndClock, CLOCKM_VOLCHANGE, TRUE, TRUE);
			break;
		}
		case IDC_FILELIST:
			break;
		default:
			PostMessage(hwnd, WM_COMMAND, pMSS->nCommand, 0);
			break;
	}
}

/*------------------------------------------------
  retrieve mouse command
--------------------------------------------------*/
PMOUSESTRUCT GetMouseCommand(int button, int nclick)
{
	PMOUSESTRUCT pMSS;
	BOOL bCtrl, bShift, bAlt;
	
	if(!m_pMouseCommand) return NULL;
	
	bCtrl  = GetAsyncKeyState(VK_CONTROL) ? TRUE : FALSE;
	bShift = GetAsyncKeyState(VK_SHIFT)   ? TRUE : FALSE;
	bAlt   = GetAsyncKeyState(VK_MENU)    ? TRUE : FALSE;
	
	pMSS = m_pMouseCommand;
	while(pMSS)
	{
		if(pMSS->nButton == button && pMSS->nClick == nclick &&
			pMSS->bCtrl == bCtrl && pMSS->bShift == bShift &&
			pMSS->bAlt == bAlt)
			return pMSS;
		
		pMSS = pMSS->next;
	}
	return NULL;
}

