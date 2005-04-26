/*-------------------------------------------------------------
  alarmstruct.c : load and save ALARMSTRUCT
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Globals */

void LoadAlarm(PALARMSTRUCT pAS, int count);
void SaveAlarm(const PALARMSTRUCT pAS, int count);
void SetAlarmTime(PALARMSTRUCT pAS);

/* Statics */

static void ParseAlarmFormat(char* dst, const char* src, int first, int last);
static void ParseAlarmFormatSub(int *n1, int *n2, int *n3, BOOL *asta,
	const char* part);

/*------------------------------------------------
  read settings of Alarm
--------------------------------------------------*/
void LoadAlarm(PALARMSTRUCT pAS, int count)
{
	int i;
	char subkey[20];
	
	for(i = 0; i < count; i++)
	{
		wsprintf(subkey, "Alarm%d", i + 1);
		
		memset(pAS + i, 0, sizeof(ALARMSTRUCT));
		
		GetMyRegStr(subkey, "Name", pAS[i].name, BUFSIZE_NAME, "");
		pAS[i].bEnable = GetMyRegLong(subkey, "Alarm", FALSE);
		
		GetMyRegStr(subkey, "Hour", pAS[i].strHours, 80, "");
		GetMyRegStr(subkey, "Minute", pAS[i].strMinutes, 80, "");
		GetMyRegStr(subkey, "WDays", pAS[i].strWDays, 80, "");
		
		pAS[i].second = GetMyRegLong(subkey, "Second", 0);
		
		GetMyRegStr(subkey, "File", pAS[i].fname, MAX_PATH, "");
		pAS[i].bHour12 = GetMyRegLong(subkey, "Hour12", TRUE);
		pAS[i].bRepeat = GetMyRegLong(subkey, "Repeat", FALSE);
		pAS[i].bBlink = GetMyRegLong(subkey, "Blink", FALSE);
		pAS[i].bBootExec = GetMyRegLong(subkey, "OnBoot", FALSE);
		pAS[i].bInterval = GetMyRegLong(subkey, "Interval", FALSE);
		pAS[i].nInterval = GetMyRegLong(subkey, "IntervalMinutes", 0);
		pAS[i].bResumeExec = GetMyRegLong(subkey, "OnResume", FALSE);
		pAS[i].nResumeDelay = GetMyRegLong(subkey, "ResumeDelay", 0);
		pAS[i].bResumeTimer = FALSE;
		
		SetAlarmTime(pAS + i);
	}
}

/*------------------------------------------------
  save settings of Alarm
--------------------------------------------------*/
void SaveAlarm(const PALARMSTRUCT pAS, int count)
{
	int i;
	char subkey[20];
	
	for(i = 0; i < count; i++)
	{
		wsprintf(subkey, "Alarm%d", i + 1);
		
		SetMyRegStr(subkey, "Name", pAS[i].name);
		SetMyRegLong(subkey, "Alarm", pAS[i].bEnable);
		SetMyRegStr(subkey, "Hour", pAS[i].strHours);
		SetMyRegStr(subkey, "Minute", pAS[i].strMinutes);
		SetMyRegStr(subkey, "WDays", pAS[i].strWDays);
		SetMyRegLong(subkey, "Second", pAS[i].second);
		
		SetMyRegStr(subkey, "File", pAS[i].fname);
		SetMyRegLong(subkey, "Hour12", pAS[i].bHour12);
		SetMyRegLong(subkey, "Repeat", pAS[i].bRepeat);
		SetMyRegLong(subkey, "Blink", pAS[i].bBlink);
		SetMyRegLong(subkey, "OnBoot", pAS[i].bBootExec);
		SetMyRegLong(subkey, "Interval", pAS[i].bInterval);
		SetMyRegLong(subkey, "IntervalMinutes", pAS[i].nInterval);
		SetMyRegLong(subkey, "OnResume", pAS[i].bResumeExec);
		SetMyRegLong(subkey, "ResumeDelay", pAS[i].nResumeDelay);
	}
}

/*------------------------------------------------
  strHours -> hours, strMinutes -> minutes,
  strWDays -> wdays
--------------------------------------------------*/
void SetAlarmTime(PALARMSTRUCT pAS)
{
	int i;
	
	ParseAlarmFormat(pAS->hours, pAS->strHours, 0, 23);
	for(i = 0; i < 24; i++)
	{
		if(pAS->hours[i]) break;
	}
	if(i == 24)
	{
		for(i = 0; i < 24; i++) pAS->hours[i] = 1;
	}
	
	ParseAlarmFormat(pAS->minutes, pAS->strMinutes, 0, 59);
	
	ParseAlarmFormat(pAS->wdays, pAS->strWDays, 0, 6);
	for(i = 0; i < 7; i++)
	{
		if(pAS->wdays[i]) break;
	}
	if(i == 7)
	{
		for(i = 0; i < 7; i++) pAS->wdays[i] = 1;
	}
}

/*------------------------------------------------
   alarm format string -> integer array
--------------------------------------------------*/
void ParseAlarmFormat(char* dst, const char* src, int first, int last)
{
	char part[81];
	int i, j;
	
	for(i = 0; i < last - first + 1; i++)
		dst[i] = 0;
	
	for(i = 0; ; i++)
	{
		if(parse(part, src, i, 81)) break;
		
		if(part[0])
		{
			int n1, n2, n3;
			BOOL asta;
			
			ParseAlarmFormatSub(&n1, &n2, &n3, &asta, part);
			
			if(asta)
			{
				if(n3 < 0) n3 = 1;
				for(j = first; j <= last; j += n3)
					dst[j - first] = 1;
			}
			else if(n1 >= 0 && n2 < 0 && n3 < 0)
			{
				if(first <= n1 && n1 <= last)
					dst[n1 - first] = 1;
			}
			else if(n1 >= 0 && n2 >= 0)
			{
				if(n3 < 0) n3 = 1;
				j = n1;
				if(j < first) j = first;
				for(; j <= n2 && j <= last; j += n3)
					dst[j - first] = 1;
			}
		}
	}
}

/*------------------------------------------------
   "1-2/3" -> n1 = 1, n2 = 2, n3 = 3
   "*"     -> asta = TRUE
--------------------------------------------------*/
void ParseAlarmFormatSub(int *n1, int *n2, int *n3, BOOL *asta,
	const char* part)
{
	const char *p = part;
	
	*n1 = *n2 = *n3 = -1;
	*asta = FALSE;
	
	if(*p == '*')
	{
		*asta = TRUE; p++;
	}
	else
	{
		while(*p)
		{
			if('0' <= *p && *p <= '9')
			{
				if(*n1 < 0) *n1 = 0;
				*n1 = *n1 * 10 + *p - '0';
			}
			else break;
			p++;
		}
		
		if(*p == '-') p++;
		else return;
		
		while(*p)
		{
			if('0' <= *p && *p <= '9')
			{
				if(*n2 < 0) *n2 = 0;
				*n2 = *n2 * 10 + *p - '0';
			}
			else break;
			p++;
		}
	}
	
	if(*p == '/') p++;
	else return;
	
	while(*p)
	{
		if('0' <= *p && *p <= '9')
		{
			if(*n3 < 0) *n3 = 0;
			*n3 = *n3 * 10 + *p - '0';
		}
		else break;
		p++;
	}
}
