/*-------------------------------------------------------------
  menu.c : context menu of TClock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"
#include "../common/command.h"

/* Globals */

void ContextMenuCommand(HWND hwnd, int id);
void EndMenu(void);
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos);
void OnExitMenuLoop(HWND hwnd);
void SetFocusTClockMain(HWND hwnd);

/* Statics */

typedef struct _tagMenuStruct
{
	int id;
	char command[MAX_PATH];
} MENUSTRUCT;
typedef MENUSTRUCT* PMENUSTRUCT;

static void SendOnContextMenu(void);
static void CheckMenu(HMENU hmenu);
static void LoadMenuFromText(HMENU hmenu,
	const char *fname, const WIN32_FIND_DATA *pfd);
static int ReadMenuCommands(HMENU hmenu, const char *p,
	PMENUSTRUCT pmenus);

static HMENU m_hMenu = NULL;
static char m_tcmenutxt[MAX_PATH] = { 0 };
static PMENUSTRUCT m_pmenuCommands = NULL;
static int m_numCommands = 0;
static DWORD m_lasttime = 0;

/*------------------------------------------------
  open/execute a file with context menu
--------------------------------------------------*/
void ContextMenuCommand(HWND hwnd, int id)
{
	int i;
	if(!m_pmenuCommands) return;
	
	for(i = 0; i < m_numCommands; i++)
	{
		if(id == m_pmenuCommands[i].id)
		{
			if(m_pmenuCommands[i].command[0])
				ExecCommandString(hwnd, m_pmenuCommands[i].command);
			break;
		}
	}
}

/*------------------------------------------------
  clean up
--------------------------------------------------*/
void EndMenu(void)
{
	if(m_hMenu) DestroyMenu(m_hMenu);
	m_hMenu = NULL;
	if(m_pmenuCommands) free(m_pmenuCommands);
	m_pmenuCommands = NULL;
}

/*------------------------------------------------
  when the clock is right-clicked
  show pop-up menu
--------------------------------------------------*/
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos)
{
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	SendOnContextMenu();
	
	if(m_tcmenutxt[0] == 0 || !IsFile(m_tcmenutxt))
	{
		// common/tclang.c
		FindFileWithLangCode(m_tcmenutxt, GetUserDefaultLangID(), TCMENUTXT);
	}
	
	hfind = FindFirstFile(m_tcmenutxt, &fd);
	if(hfind != INVALID_HANDLE_VALUE)
	{
		FindClose(hfind);
		if(m_lasttime != fd.ftLastWriteTime.dwLowDateTime)
		{
			EndMenu();
			m_lasttime = fd.ftLastWriteTime.dwLowDateTime;
		}
	}
	else m_lasttime = 0;
	
	// create popup menu and append items from tcmenu.txt
	if(!m_hMenu)
	{
		m_hMenu = CreatePopupMenu();
		if(hfind != INVALID_HANDLE_VALUE)
			LoadMenuFromText(m_hMenu, m_tcmenutxt, &fd);
		else
			LoadMenuFromText(m_hMenu, NULL, NULL);
		
		MemReduce();
	}
	
	CheckMenu(m_hMenu);
	
	// get keyboard input
	SetFocusTClockMain(hwnd);
	
	// open popup menu
	TrackPopupMenu(m_hMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		xPos, yPos, 0, hwnd, NULL);
}

/*------------------------------------------------
  WM_EXITMENULOOP message
--------------------------------------------------*/
void OnExitMenuLoop(HWND hwnd)
{
	HWND hwndBar;
	DWORD pid, clockthread, mythread;
	
	hwndBar = GetTaskbarWindow();
	if(hwndBar)
	{
		clockthread = GetWindowThreadProcessId(hwndBar, &pid);
		mythread = GetCurrentThreadId();
		AttachThreadInput(clockthread, mythread, TRUE);
		SetFocus(hwndBar);
		AttachThreadInput(clockthread, mythread, FALSE);
	}
}

/*------------------------------------------------
  set keyboard focus to the main window forcely
--------------------------------------------------*/
void SetFocusTClockMain(HWND hwnd)
{
	DWORD pid, curthread, mythread;
	
	curthread = GetWindowThreadProcessId(GetForegroundWindow(), &pid);
	mythread = GetCurrentThreadId();
	AttachThreadInput(mythread, curthread, TRUE);
	SetFocus(hwnd);
	AttachThreadInput(mythread, curthread, FALSE);
}

/*------------------------------------------------
  tell other programs
  that right-click menu is to open
--------------------------------------------------*/
void SendOnContextMenu(void)
{
	HWND hwndProg;
	char entry[20], buf[100], cname[80], num[20];
	int i, msg;
	
	for(i = 0; ; i++)
	{
		wsprintf(entry, "App%d", i + 1);
		GetMyRegStr("OnContextMenu", entry, buf, 100, "");
		if(!buf[0]) break;
		
		parse(cname, buf, 0, 80);
		if(!cname[0]) continue;
		parse(num, buf, 1, 20);
		if(!num[0]) continue;
		msg = atoi(num);
		
		hwndProg = FindWindow(cname, NULL);
		if(hwndProg)
			SendMessage(hwndProg, msg, 0, 0);
	}
}

/*------------------------------------------------
  enable/disable menu item
--------------------------------------------------*/
void CheckMenu(HMENU hmenu)
{
	char s[80];
	int winver = CheckWinVersion();
	
	EnableMenuItem(hmenu, IDC_TASKMAN, MF_BYCOMMAND|
		((winver&WINNT) ? MF_ENABLED:MF_GRAYED) );
	
	EnableMenuItem(hmenu, IDC_SYNCTIME, MF_BYCOMMAND|
		(GetMyRegStr("SNTP", "Server", s, 80, "") > 0 ?
			MF_ENABLED:MF_GRAYED));
}

/*------------------------------------------------
  read menu commands from tcmenu*.txt
--------------------------------------------------*/
void LoadMenuFromText(HMENU hmenu,
	const char *fname, const WIN32_FIND_DATA *pfd)
{
	HFILE hf = HFILE_ERROR;
	int size;
	char *pbuf;
	
	if(fname && pfd)
		hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR)
	{
		AppendMenu(hmenu, MF_STRING, IDC_EXIT, "Exit TClock");
		return;
	}
	
	size = (int)pfd->nFileSizeLow;
	pbuf = malloc(size + 1);
	_lread(hf, pbuf, size);
	pbuf[size] = 0;
	_lclose(hf);
	
	m_numCommands = ReadMenuCommands(NULL, pbuf, NULL);
	
	if(m_pmenuCommands) free(m_pmenuCommands);
	m_pmenuCommands = NULL;
	
	if(m_numCommands > 0)
		m_pmenuCommands = malloc(sizeof(MENUSTRUCT) * m_numCommands);
	ReadMenuCommands(hmenu, pbuf, m_pmenuCommands);
	
	free(pbuf);
}

/*------------------------------------------------
  add commands to MENUSTRUCT and HMENU
--------------------------------------------------*/
int ReadMenuCommands(HMENU hmenu, const char *p,
	MENUSTRUCT* pmenus)
{
	int count = 0;
	int idCommand = 1;
	char name[81], command[MAX_PATH];
	
	while(*p)
	{
		if(*p == '#' ||
			*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
		{
			p = nextline(p);
		}
		else if(*p == '-' && *(p + 1) == '-')
		{
			if(hmenu)
				AppendMenu(hmenu, MF_SEPARATOR, 0, 0);
			p = nextline(p);
		}
		else
		{
			BOOL bdigit;
			int id, i;
			char quot;
			
			quot = 0;
			if(*p == '\"' || *p == '\'') { quot = *p; p++; }
			for(i = 0; i < 80 &&
				*p && *p != '\r' && *p != '\n'; i++)
			{
				if(quot) { if(*p == quot) { p++; break; } }
				else if(*p == ' ' || *p == '\t') break;
				name[i] = *p++;
			}
			name[i] = 0;
			while(*p == ' ' || *p == '\t') p++;
			
			quot = 0;
			if(*p == '\"' || *p == '\'') { quot = *p; p++; }
			for(i = 0; i < MAX_PATH-1 &&
				*p && *p != '\r' && *p != '\n'; i++)
			{
				if(quot && *p == quot) break;
				command[i] = *p++;
			}
			command[i] = 0;
			
			bdigit = isdigitstr(command);
			if(bdigit) id = atoi(command);
			else id = idCommand;
			
			if(hmenu)
				AppendMenu(hmenu, MF_STRING, id, name);
			
			if(!bdigit)
			{
				if(pmenus)
				{
					pmenus[count].id = id;
					strcpy(pmenus[count].command, command);
				}
				count++;
				if(count >= 50) return count;
				idCommand++;
			}
			
			p = nextline(p);
		}
	}
	
	return count;
}

