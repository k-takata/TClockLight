/*-------------------------------------------------------------
  taskswitch.c : customize task switcher
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"
//#include "newapi.h"

#if TC_ENABLE_TASKSWITCH

/* Globals */

void InitTaskSwitch(HWND hwndClock);
void EndTaskSwitch(void);

/* Statics */

static LRESULT CALLBACK WndProcTab(HWND, UINT, WPARAM, LPARAM);

static BOOL    m_bTaskSwitchFlat = FALSE;
static BOOL    m_bTaskSwitchIcons = FALSE;
static BOOL    m_bSeparator = FALSE;
static HWND    m_hwndTab = NULL;
static LONG    m_oldStyle;
static DWORD   m_oldExStyle;
static DWORD   m_oldTBStyle;
static LONG    m_oldTaskWidth;
static WNDPROC m_oldWndProcTab = NULL;

/*--------------------------------------------------
   initialize
----------------------------------------------------*/
void InitTaskSwitch(HWND hwndClock)
{
	HANDLE hwndTaskbar, hwndRebar, hwndSwitch;
	
	EndTaskSwitch();
	
	if(!g_bVisualStyle)
	{
		m_bTaskSwitchFlat = GetMyRegLong(NULL, "TaskSwitchFlat", FALSE);
	}
	
	// Icons Only
	if(!IsTaskbarAnimation())
		m_bTaskSwitchIcons = GetMyRegLong(NULL, "TaskSwitchIconsOnly", FALSE);
	
	if(!m_bTaskSwitchFlat && !m_bTaskSwitchIcons)
		return;
	
	// get window handle of MSTaskSwWClass
	hwndTaskbar = GetParent(GetParent(hwndClock));
	hwndRebar = FindWindowEx(hwndTaskbar, NULL, "ReBarWindow32", NULL);
	hwndSwitch = FindWindowEx(hwndRebar ? hwndRebar : hwndTaskbar,
		NULL, "MSTaskSwWClass", NULL);
	if(hwndSwitch == NULL) return;
	
	m_hwndTab = GetWindow(hwndSwitch, GW_CHILD);
	if(m_hwndTab == NULL) return;
	
	if(m_bTaskSwitchFlat)
	{
		m_oldStyle = GetWindowLong(m_hwndTab, GWL_STYLE);
		
		// m_hwndTab is Toolbar control
		SetWindowLong(m_hwndTab, GWL_STYLE, m_oldStyle|TBSTYLE_FLAT);
		
		if(m_bSeparator)
		{
			m_oldExStyle = TabCtrl_GetExtendedStyle(m_hwndTab);
			TabCtrl_SetExtendedStyle(m_hwndTab,
				m_oldExStyle|TCS_EX_FLATSEPARATORS);
		}
	}
	
	if(m_bTaskSwitchIcons)
	{
		m_oldTBStyle = (DWORD)SendMessage(m_hwndTab,
				TB_GETEXTENDEDSTYLE, 0, 0);
		SendMessage(m_hwndTab, TB_SETEXTENDEDSTYLE,
				0, m_oldTBStyle|TBSTYLE_EX_MIXEDBUTTONS);
	}
	
	m_oldWndProcTab = SubclassWindow(m_hwndTab, WndProcTab);
	
	PostMessage(m_hwndTab, WM_SIZE, SIZE_RESTORED, 0);
}

/*--------------------------------------------------
    undo
----------------------------------------------------*/
void EndTaskSwitch(void)
{
	if(!m_hwndTab || !IsWindow(m_hwndTab)) return;
	
	if(m_oldWndProcTab)
		SubclassWindow(m_hwndTab, m_oldWndProcTab);
	m_oldWndProcTab = NULL;
	
	if(m_bTaskSwitchFlat)
		SetWindowLong(m_hwndTab, GWL_STYLE, m_oldStyle);
	
	if(m_bSeparator)
		TabCtrl_SetExtendedStyle(m_hwndTab, m_oldExStyle);
	
	if(m_bTaskSwitchIcons)
	{
		SendMessage(m_hwndTab, TB_SETEXTENDEDSTYLE, 0, m_oldTBStyle);
	}
	
	PostMessage(m_hwndTab, WM_SIZE, SIZE_RESTORED, 0);
	
	m_hwndTab = NULL;
	m_bTaskSwitchFlat = m_bSeparator = m_bTaskSwitchIcons = FALSE;
}

/*---------------------------------------------------------
   subclass procedure of SysTabControl32/ToobarWindow32
-----------------------------------------------------------*/
LRESULT CALLBACK WndProcTab(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case TCM_SETITEMSIZE:
			if(m_bTaskSwitchIcons)
			{
				lParam = MAKELONG(23, HIWORD(lParam));
			}
			else
			{
				if(LOWORD(lParam)-8 >= 22)
					lParam = MAKELONG(LOWORD(lParam)-8, HIWORD(lParam));
			}
			break;
		case TCM_INSERTITEM:
			PostMessage(GetParent(hwnd), WM_SIZE, SIZE_RESTORED, 0);
			break;
		case TCM_DELETEITEM:
			PostMessage(GetParent(hwnd), WM_SIZE, SIZE_RESTORED, 0);
			break;
		/*
		case TB_SETBUTTONSIZE:
			if(m_bTaskSwitchIcons)
			{
				lParam = MAKELONG(23, HIWORD(lParam));
			}
			else
			{
				if(LOWORD(lParam)-8 >= 22)
					lParam = MAKELONG(LOWORD(lParam)-8, HIWORD(lParam));
			}
			break;
		case TB_INSERTBUTTON:
			PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			break;
		case TB_DELETEBUTTON:
			PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			break;
		*/
	}
	return CallWindowProc(m_oldWndProcTab, hwnd, message, wParam, lParam);
}

#endif	/* TC_ENABLE_TASKSWITCH */
