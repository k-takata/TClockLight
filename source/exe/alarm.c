/*-------------------------------------------------------------
  alarm.c : process alarms
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

/* Globals */

void InitAlarm(void);
void EndAlarm(void);
void OnTimerAlarm(HWND hwnd, const SYSTEMTIME* st, int reason);

/* Statics */

static PALARMSTRUCT m_pAlarm = NULL;
static BOOL m_bCheckEverySeconds = FALSE;

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void InitAlarm(void)
{
	PALARMSTRUCT pitem;
	
	clear_list(m_pAlarm);
	m_pAlarm = NULL;
	
	// read settings
	m_pAlarm = LoadAlarm(); // common/alarmstruct.c
	
	// cuckoo clock
	if(GetMyRegLong(NULL, "Jihou", FALSE))
	{
		ALARMSTRUCT item;
		
		memset(&item, 0, sizeof(ALARMSTRUCT));
		strcpy(item.name, "cuckoo");
		strcpy(item.strHours, "*");
		strcpy(item.strMinutes, "0");
		strcpy(item.strWDays, "*");
		SetAlarmTime(&item); // common/alarmstruct.c
		
		item.bEnable = TRUE;
		item.bHour12 = TRUE;
		GetMyRegStr(NULL, "JihouFile", item.fname, MAX_PATH, "");
		item.bRepeatJihou = GetMyRegLong(NULL, "JihouRepeat", FALSE);
		if(GetMyRegLong(NULL, "JihouBlink", FALSE))
		{
			item.bBlink = TRUE; item.nBlinkSec = 60;
		}
		
		m_pAlarm = copy_listitem(m_pAlarm, &item, sizeof(item));
	}
	
	m_bCheckEverySeconds = FALSE;
	pitem = m_pAlarm;
	while(pitem)
	{
		if(pitem->bEnable)
		{
			if(pitem->second)
				m_bCheckEverySeconds = TRUE;
			else if(pitem->bInterval)
			{
				if(!pitem->bBootExec)
					pitem->tickLast = GetTickCount();
				m_bCheckEverySeconds = TRUE;
			}
		}
		
		pitem = pitem->next;
	}
}

/*------------------------------------------------
  clear up
--------------------------------------------------*/
void EndAlarm(void)
{
	clear_list(m_pAlarm);
	m_pAlarm = NULL;
}

/*------------------------------------------------
  execute alarms
--------------------------------------------------*/
void OnTimerAlarm(HWND hwnd, const SYSTEMTIME* st, int reason)
{
	static int hourLast = 0, minuteLast = 0;
	PALARMSTRUCT pitem;
	int hour, loops;
	
	if(!m_pAlarm) return;
	
	// execute once a minute
	if(!m_bCheckEverySeconds && reason == 0 && st)
	{
		if(minuteLast == st->wMinute && hourLast == st->wHour)
			return;
		hourLast = st->wHour;
		minuteLast = st->wMinute;
	}
	
	for(pitem = m_pAlarm; pitem; pitem = pitem->next)
	{
		BOOL bexec = FALSE;
		
		if(!pitem->bEnable) continue;
		
		hour = 0;
		if(st)
		{
			// 12 hour
			hour = st->wHour;
			if(pitem->bHour12)
			{
				if(hour >= 13) hour -= 12;
				else if(hour == 0) hour = 12;
			}
			
			// compare time
			if(reason == 0)
			{
				if(pitem->hours[hour]
					&& pitem->minutes[st->wMinute]
					&& pitem->wdays[st->wDayOfWeek]
					&& pitem->second == st->wSecond)
					bexec = TRUE;
			}
			
			// At regular intervals
			if(pitem->bInterval && pitem->nInterval > 0)
			{
				if(pitem->hours[hour] && pitem->wdays[st->wDayOfWeek])
				{
					if(GetTickCount() - pitem->tickLast >
						(DWORD)pitem->nInterval * 1000 * 60) bexec = TRUE;
				}
			}
		}
		
		// Execute when TClock is started
		if(reason == 1)
		{
			if(pitem->bBootExec) bexec = TRUE;
		}
		
		if(bexec)
		{
			DWORD tick = GetTickCount();
			// not to execute an alarm twice within a minute
			if(tick - pitem->tickLast > 59840 || reason != 0)
			{
				pitem->tickLast = tick;
				
				if(pitem->bBlink && g_hwndClock)
					PostMessage(g_hwndClock, CLOCKM_BLINK, 0, pitem->nBlinkSec);
				
				if(pitem->fname[0])
				{
					if(pitem->bRepeat) loops = -1; else loops = 0;
					if(pitem->bRepeatJihou) loops = hour;
					
					// common/playfile.c
					PlayFile(hwnd, pitem->fname, loops);
				}
			}
		}
	}
}

