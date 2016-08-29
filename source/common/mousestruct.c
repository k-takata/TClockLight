/*-------------------------------------------------------------
  mousestruct.c : load and save MOUSESTRUCT
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"
#include "command.h"

/*------------------------------------------------
  read settings of Mouse
--------------------------------------------------*/
PMOUSESTRUCT LoadMouseFunc(void)
{
	PMOUSESTRUCT plist = NULL;
	MOUSESTRUCT item;
	int i, count;
	char section[20], s[20];
	
	count = GetMyRegLong("Mouse", "MouseNum", 0);
	
	for(i = 0; i < count; i++)
	{
		wsprintf(section, "Mouse%d", i + 1);
		
		memset(&item, 0, sizeof(MOUSESTRUCT));
		
		GetMyRegStr(section, "Name", item.name, BUFSIZE_NAME, section);
		
		GetMyRegStr(section, "Button", s, 20, "");
		if(strcmp(s, "left") == 0) item.nButton = 0;
		else if(strcmp(s, "right") == 0) item.nButton = 1;
		else if(strcmp(s, "middle") == 0) item.nButton = 2;
		else if(strcmp(s, "x1") == 0) item.nButton = 3;
		else if(strcmp(s, "x2") == 0) item.nButton = 4;
#if TC_ENABLE_WHEEL
		else if(strcmp(s, "wheelup") == 0) item.nButton = 5;
		else if(strcmp(s, "wheeldown") == 0) item.nButton = 6;
#endif
		
		item.nClick = GetMyRegLong(section, "Click", 1);
		item.bCtrl  = GetMyRegLong(section, "Ctrl", FALSE);
		item.bShift = GetMyRegLong(section, "Shift", FALSE);
		item.bAlt   = GetMyRegLong(section, "Alt", FALSE);
		
		item.nCommand = GetMyRegLong(section, "Command", 0);
		GetMyRegStr(section, "Option", item.option, MAX_PATH, "");
		
		plist = copy_listitem(plist, &item, sizeof(MOUSESTRUCT));
	}
	
	return plist;
}

/*------------------------------------------------
  save settings of Mouse
--------------------------------------------------*/
void SaveMouseFunc(const PMOUSESTRUCT plist)
{
	int oldcount, i;
	char section[20];
	PMOUSESTRUCT current;
	
	oldcount = GetMyRegLong("Mouse", "MouseNum", 0);
	
	current = plist;
	i = 0;
	while(current)
	{
		wsprintf(section, "Mouse%d", i + 1);
		
		SetMyRegStr(section, "Name", current->name);
		
		if(current->nButton == 0)
			SetMyRegStr(section, "Button", "left");
		else if(current->nButton == 1)
			SetMyRegStr(section, "Button", "right");
		else if(current->nButton == 2)
			SetMyRegStr(section, "Button", "middle");
		else if(current->nButton == 3)
			SetMyRegStr(section, "Button", "x1");
		else if(current->nButton == 4)
			SetMyRegStr(section, "Button", "x2");
#if TC_ENABLE_WHEEL
		else if(current->nButton == 5)
			SetMyRegStr(section, "Button", "wheelup");
		else if(current->nButton == 6)
			SetMyRegStr(section, "Button", "wheeldown");
#endif
		
		SetMyRegLong(section, "Click", current->nClick);
		SetMyRegLong(section, "Ctrl", current->bCtrl);
		SetMyRegLong(section, "Shift", current->bShift);
		SetMyRegLong(section, "Alt", current->bAlt);
		
		SetMyRegLong(section, "Command", current->nCommand);
		SetMyRegStr(section, "Option", current->option);
		
		current = current->next;
		i++;
	}
	
	SetMyRegLong("Mouse", "MouseNum", i);
	
	for(; i < oldcount; i++)
	{
		wsprintf(section, "Mouse%d", i + 1);
		DelMyRegKey(section);
	}
}

// old mouse functions
#define MOUSEFUNC_NONE       -1
#define MOUSEFUNC_DATETIME    0
#define MOUSEFUNC_EXITWIN     1
#define MOUSEFUNC_RUNAPP      2
#define MOUSEFUNC_MINALL      3
#define MOUSEFUNC_SYNCTIME    4
#define MOUSEFUNC_TIMER       5
#define MOUSEFUNC_CLIPBOARD   6
#define MOUSEFUNC_SCREENSAVER 7
#define MOUSEFUNC_KYU         8
#define MOUSEFUNC_DELRECDOCS  9
#define MOUSEFUNC_PROPERTY    10
#define MOUSEFUNC_CALENDAR    11
#define MOUSEFUNC_STARTMENU   12
#define MOUSEFUNC_TASKSW      13
#define MOUSEFUNC_SHOWDESK    14
#define MOUSEFUNC_LOCKPC      15
#define MOUSEFUNC_MENU        16
#define MOUSEFUNC_CHANGECONF  17
#define MOUSEFUNC_VOLMUTE     18
#define MOUSEFUNC_VOLSET      19
#define MOUSEFUNC_VOLUD       20
#define MOUSEFUNC_MONOFF      21
#define MOUSEFUNC_CDOPEN      22
#define MOUSEFUNC_CDCLOSE     23
#define MOUSEFUNC_NETINIT     24
#define MOUSEFUNC_DELUS       25
#define MOUSEFUNC_FILELIST    26
#define MOUSEFUNC_OPENFILE    100

static int oldToNew[] = {
	IDC_DATETIME,
	IDC_EXITWIN,
	IDC_RUNAPP,
	IDC_MINALL,
	IDC_SYNCTIME,
	IDC_TIMER,
	IDC_MOUSECOPY,
	IDC_SCREENSAVER,
	IDC_KYU,
	IDC_DELRECDOCS,
	IDC_SHOWPROP,
	IDC_SHOWCALENDER,
	IDC_SHOWSTARTMENU,
	IDC_TASKSW,
	IDC_SHOWDESK,
	IDC_LOCKPC,
	IDC_TCLOCKMENU,
	IDC_CHANGECONF,
	IDC_VOLMUTE,
	IDC_VOLSET,
	IDC_VOLUD,
	IDC_MONOFF,
	IDC_CDOPEN,
	IDC_CDCLOSE,
	IDC_NETINIT,
	IDC_DELUS,
	IDC_FILELIST
};

/*-----------------------------------------------------------
  save old data (TClock 2.3.0 and TClock2ch) with new format
------------------------------------------------------------*/
void ImportOldMouseFunc(void)
{
	char oldentry[20], newsection[20], s[MAX_PATH];
	int i, j, k, count, nfunc;
	char *oldoptions[] = { "%d%dFile", "%d%dClip",
		"%d%dVol", "%d%dDrv" , "%d%dDelay", NULL, };
	
	count = 0;
	for(i = 0; i <= 21; i++)
	{
		if(i == 15) continue;
		
		for(j = 1; j <= 4; j++)
		{
			wsprintf(oldentry, "%d%d", i, j);
			nfunc = GetMyRegLong("Mouse", oldentry, -1);
			if((0 <= nfunc && nfunc <= MOUSEFUNC_FILELIST) ||
				nfunc == MOUSEFUNC_OPENFILE)
			{
				DelMyReg("Mouse", oldentry);
				
				wsprintf(newsection, "Mouse%d", count + 1);
				
				if(i == 0 || i == 5 || i == 10)
					SetMyRegStr(newsection, "Button", "left");
				else if(i == 1 || i == 6 || i == 11)
					SetMyRegStr(newsection, "Button", "right");
				else if(i == 2 || i == 7 || i == 12)
					SetMyRegStr(newsection, "Button", "middle");
				else if(i == 3 || i == 8 || i == 13)
					SetMyRegStr(newsection, "Button", "x1");
				else if(i == 4 || i == 9 || i == 14)
					SetMyRegStr(newsection, "Button", "x2");
				else
					strcpy(newsection, "Wheel");
				
				if(i <= 14)
					SetMyRegLong(newsection, "Click", j);
				
				if((5 <= i && i <= 9) || i == 18 || i == 19)
					SetMyRegLong(newsection, "Ctrl", TRUE);
				if((10 <= i && i <= 14) || i == 20 || i == 21)
					SetMyRegLong(newsection, "Shift", TRUE);
				
				if(nfunc == MOUSEFUNC_OPENFILE)
					SetMyRegLong(newsection, "Command", IDC_OPENFILE);
				else if(nfunc == MOUSEFUNC_TIMER)
				{
					SetMyRegLong(newsection, "Command", IDC_OPENFILE);
					SetMyRegStr(newsection, "Option", "tctimer.exe");
				}
				else if(nfunc == MOUSEFUNC_PROPERTY)
				{
					SetMyRegLong(newsection, "Command", IDC_OPENFILE);
					SetMyRegStr(newsection, "Option", "tcprop.exe");
				}
				else
					SetMyRegLong(newsection, "Command", oldToNew[nfunc]);
				
				for(k = 0; oldoptions[k]; k++)
				{
					wsprintf(oldentry, oldoptions[k], i, j);
					if(GetMyRegStr("Mouse", oldentry, s, MAX_PATH, "") > 0)
					{
						SetMyRegStr(newsection, "Option", s);
						DelMyReg("Mouse", oldentry);
					}
				}
				
				count++;
			}
			else
			{
				if(i == 0 && j == 1) // left single click
				{
					wsprintf(newsection, "Mouse%d", count + 1);
					SetMyRegStr(newsection, "Button", "left");
					SetMyRegLong(newsection, "Click", 1);
					SetMyRegLong(newsection, "Command", IDC_VISTACALENDAR);
					count++;
				}
				if(i == 1 && j == 1) // right single click
				{
					wsprintf(newsection, "Mouse%d", count + 1);
					SetMyRegStr(newsection, "Button", "right");
					SetMyRegLong(newsection, "Click", 1);
					SetMyRegLong(newsection, "Command", IDC_TCLOCKMENU);
					count++;
				}
			}
			if(i >= 16) break;
		}
	}
	
	SetMyRegLong("Mouse", "MouseNum", count);
}
