/*-------------------------------------------------------------
  alarm.c : process alarms
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

/* Statics */

static PALARMSTRUCT m_pAlarm = NULL;
static BOOL m_bCheckEverySeconds = FALSE;

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void InitAlarm(void)
{
	ALARMSTRUCT item;
	PALARMSTRUCT pitem;
	int jihou;
	
	clear_list(m_pAlarm);
	m_pAlarm = NULL;
	
	jihou = 0;
	if(GetMyRegLong("", "Jihou", FALSE)) jihou = 1;
	
	// read settings
	m_pAlarm = LoadAlarm(); // common/alarmstruct.c
	
	// cuckoo clock
	if(jihou)
	{
		memset(&item, 0, sizeof(ALARMSTRUCT));
		strcpy(item.name, "cuckoo");
		strcpy(item.strHours, "*");
		strcpy(item.strMinutes, "0");
		strcpy(item.strWDays, "*");
		SetAlarmTime(&item); // common/alarmstruct.c
		
		item.bEnable = TRUE;
		item.bHour12 = TRUE;
		GetMyRegStr("", "JihouFile", item.fname, MAX_PATH, "");
		if(GetMyRegLong("", "JihouRepeat", FALSE))
			item.bRepeatJihou = TRUE;
		if(GetMyRegLong("", "JihouBlink", FALSE))
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
			if(pitem->bResumeExec)
				m_bCheckEverySeconds = TRUE;
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
	PALARMSTRUCT pitem;
	static int hourLast = 0, minuteLast = 0;
	static DWORD resumeTick = 0;
	int hour, loops;
	
	if(!m_pAlarm) return;
	
	// execute once a minute
	if(reason == 0 && st && !m_bCheckEverySeconds)
	{
		if(hourLast == (int)st->wHour &&
			minuteLast == (int)st->wMinute) return;
		hourLast = st->wHour;
		minuteLast = st->wMinute;
	}
	// save resume time
	if(reason == 2)
		resumeTick = GetTickCount();
	
	for(pitem = m_pAlarm; pitem; pitem = pitem->next)
	{
		BOOL bexec = FALSE;
		
		if(!pitem->bEnable) continue;
		
		// 12 hour
		hour = 0;
		if(st)
		{
			hour = st->wHour;
			if(pitem->bHour12)
			{
				if(hour == 0) hour = 12;
				else if(hour >= 13) hour -= 12;
			}
		}
		
		// compare time
		if(reason == 0 && st)
		{
			if(pitem->hours[hour]
				&& pitem->minutes[st->wMinute]
				&& pitem->wdays[st->wDayOfWeek])
			{
				if(pitem->second)
				{
					if(pitem->second == st->wSecond) bexec = TRUE;
				}
				else bexec = TRUE;
			}
		}
		
		// Execute when TClock is started
		if(reason == 1)
		{
			if(pitem->bBootExec) bexec = TRUE;
		}
		
		// Execute on resume
		if(reason == 2 && pitem->bResumeExec)
		{
			if(pitem->nResumeDelay == 0)
			{
				bexec = TRUE;
				pitem->bResumeTimer = FALSE;
			}
			else
			{
				pitem->bResumeTimer = TRUE;
				continue;
			}
		}
		if(reason == 0 && pitem->bResumeTimer)
		{
			if(GetTickCount() - resumeTick > (DWORD)(pitem->nResumeDelay * 1000))
			{
				bexec = TRUE;
				pitem->bResumeTimer = FALSE;
			}
		}
		
		// At regular intervals
		if(st && pitem->bInterval && pitem->nInterval > 0)
		{
			if(pitem->hours[hour] && pitem->wdays[st->wDayOfWeek])
			{
				if(GetTickCount() - pitem->tickLast >
					(DWORD)pitem->nInterval * 1000 * 60) bexec = TRUE;
			}
		}
		
		// not to execute an alarm twice within a minute
		if(bexec)
		{
			DWORD tick = GetTickCount();
			if(tick - pitem->tickLast < 60000 && reason == 0)
				bexec = FALSE;
			else pitem->tickLast = tick;
		}
		
		if(bexec)
		{
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

