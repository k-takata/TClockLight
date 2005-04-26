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
static int m_numAlarm = 0;
static BOOL m_bCheckEverySeconds = FALSE;

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void InitAlarm(void)
{
	PALARMSTRUCT pAS;
	int i;
	int jihou;
	
	m_numAlarm = GetMyRegLong("", "AlarmNum", 0);
	if(m_numAlarm < 1) m_numAlarm = 0;
	
	if(m_pAlarm) free(m_pAlarm);
	m_pAlarm = NULL;
	
	jihou = 0;
	if(GetMyRegLong("", "Jihou", FALSE)) jihou = 1;
	
	if(m_numAlarm + jihou == 0) return;
	
	m_pAlarm = malloc(sizeof(ALARMSTRUCT) * (m_numAlarm + jihou));
	
	// read settings
	LoadAlarm(m_pAlarm, m_numAlarm); // common/alarmstruct.c
	
	// cuckoo clock
	if(jihou)
	{
		pAS = m_pAlarm + m_numAlarm;
		m_numAlarm++;
		
		memset(pAS, 0, sizeof(ALARMSTRUCT));
		strcpy(pAS->strHours, "*");
		strcpy(pAS->strMinutes, "0");
		strcpy(pAS->strWDays, "*");
		SetAlarmTime(pAS); // common/alarmstruct.c
		
		pAS->bEnable = TRUE;
		pAS->bHour12 = TRUE;
		GetMyRegStr("", "JihouFile", pAS->fname, MAX_PATH, "");
		if(GetMyRegLong("", "JihouRepeat", FALSE))
			pAS->bRepeatJihou = TRUE;
		if(GetMyRegLong("", "JihouBlink", FALSE))
		{
			pAS->bBlink = TRUE; pAS->nBlinkSec = 60;
		}
	}
	
	m_bCheckEverySeconds = FALSE;
	if(m_pAlarm)
	{
		for(i = 0; i < m_numAlarm; i++)
		{
			pAS = m_pAlarm + i;
			if(pAS->bEnable)
			{
				if(pAS->second)
					m_bCheckEverySeconds = TRUE;
				else if(pAS->bInterval)
				{
					if(!pAS->bBootExec)
						pAS->tickLast = GetTickCount();
					m_bCheckEverySeconds = TRUE;
				}
				if(pAS->bResumeExec)
					m_bCheckEverySeconds = TRUE;
			}
		}
	}
}

/*------------------------------------------------
  clear up
--------------------------------------------------*/
void EndAlarm(void)
{
	if(m_pAlarm) free(m_pAlarm);
	m_pAlarm = NULL;
}

/*------------------------------------------------
  execute alarms
--------------------------------------------------*/
void OnTimerAlarm(HWND hwnd, const SYSTEMTIME* st, int reason)
{
	PALARMSTRUCT pAS;
	static int hourLast = 0, minuteLast = 0;
	static DWORD resumeTick = 0;
	int i, hour, loops;
	
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
	
	for(i = 0; i < m_numAlarm; i++)
	{
		BOOL bexec = FALSE;
		
		pAS = m_pAlarm + i;
		if(!pAS->bEnable) continue;
		
		// 12 hour
		hour = 0;
		if(st)
		{
			hour = st->wHour;
			if(pAS->bHour12)
			{
				if(hour == 0) hour = 12;
				else if(hour >= 13) hour -= 12;
			}
		}
		
		// compare time
		if(reason == 0 && st)
		{
			if(pAS->hours[hour]
				&& pAS->minutes[st->wMinute]
				&& pAS->wdays[st->wDayOfWeek])
			{
				if(pAS->second)
				{
					if(pAS->second == st->wSecond) bexec = TRUE;
				}
				else bexec = TRUE;
			}
		}
		
		// Execute when TClock is started
		if(reason == 1)
		{
			if(pAS->bBootExec) bexec = TRUE;
		}
		
		// Execute on resume
		if(reason == 2 && pAS->bResumeExec)
		{
			if(pAS->nResumeDelay == 0)
			{
				bexec = TRUE;
				pAS->bResumeTimer = FALSE;
			}
			else
			{
				pAS->bResumeTimer = TRUE;
				continue;
			}
		}
		if(reason == 0 && pAS->bResumeTimer)
		{
			if(GetTickCount() - resumeTick > pAS->nResumeDelay * 1000)
			{
				bexec = TRUE;
				pAS->bResumeTimer = FALSE;
			}
		}
		
		// At regular intervals
		if(st && pAS->bInterval && pAS->nInterval > 0)
		{
			if(pAS->hours[hour] && pAS->wdays[st->wDayOfWeek])
			{
				if(GetTickCount() - pAS->tickLast >
					(DWORD)pAS->nInterval * 1000 * 60) bexec = TRUE;
			}
		}
		
		// not to execute an alarm twice within a minute
		if(bexec)
		{
			DWORD tick = GetTickCount();
			if(tick - pAS->tickLast < 60000 && reason == 0)
				bexec = FALSE;
			else pAS->tickLast = tick;
		}
		
		if(bexec)
		{
			if(pAS->bBlink && g_hwndClock)
				PostMessage(g_hwndClock, CLOCKM_BLINK, 0, pAS->nBlinkSec);
			
			if(pAS->fname[0])
			{
				if(pAS->bRepeat) loops = -1; else loops = 0;
				if(pAS->bRepeatJihou) loops = hour;
				
				// common/playfile.c
				PlayFile(hwnd, pAS->fname, loops);
			}
		}
	}
}

