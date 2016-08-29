/*-------------------------------------------------------------
  alarmstruct.c : load and save ALARMSTRUCT
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Statics */

static void ParseAlarmFormat(char* dst, const char* src, int first, int last);
static void ParseAlarmFormatSub(int *n1, int *n2, int *n3, BOOL *asta,
	const char* part);

/*------------------------------------------------
  read settings of Alarm
--------------------------------------------------*/
PALARMSTRUCT LoadAlarm(void)
{
	PALARMSTRUCT plist = NULL;
	ALARMSTRUCT item;
	int i, count;
	char subkey[20];
	
	count = GetMyRegLong("", "AlarmNum", 0);
	
	for(i = 0; i < count; i++)
	{
		wsprintf(subkey, "Alarm%d", i + 1);
		
		memset(&item, 0, sizeof(ALARMSTRUCT));
		
		GetMyRegStr(subkey, "Name", item.name, BUFSIZE_NAME, "");
		item.bEnable = GetMyRegLong(subkey, "Alarm", FALSE);
		
		GetMyRegStr(subkey, "Hour", item.strHours, 80, "");
		GetMyRegStr(subkey, "Minute", item.strMinutes, 80, "");
		GetMyRegStr(subkey, "WDays", item.strWDays, 80, "");
		
		item.second = GetMyRegLong(subkey, "Second", 0);
		
		GetMyRegStr(subkey, "File", item.fname, MAX_PATH, "");
		item.bHour12 = GetMyRegLong(subkey, "Hour12", TRUE);
		item.bRepeat = GetMyRegLong(subkey, "Repeat", FALSE);
		item.bBlink = GetMyRegLong(subkey, "Blink", FALSE);
		item.bBootExec = GetMyRegLong(subkey, "OnBoot", FALSE);
		item.bInterval = GetMyRegLong(subkey, "Interval", FALSE);
		item.nInterval = GetMyRegLong(subkey, "IntervalMinutes", 0);
		item.bResumeExec = GetMyRegLong(subkey, "OnResume", FALSE);
		item.nResumeDelay = GetMyRegLong(subkey, "ResumeDelay", 0);
		item.bResumeTimer = FALSE;
		
		SetAlarmTime(&item);
		
		plist = copy_listitem(plist, &item, sizeof(item)); // list.c
	}
	
	return plist;
}

/*------------------------------------------------
  save settings of Alarm
--------------------------------------------------*/
void SaveAlarm(const PALARMSTRUCT plist)
{
	int count, oldcount, i;
	char subkey[20];
	PALARMSTRUCT current;
	
	oldcount = GetMyRegLong("", "AlarmNum", 0);
	
	current = plist;
	count = 0;
	while(current)
	{
		wsprintf(subkey, "Alarm%d", count + 1);
		
		SetMyRegStr(subkey, "Name", current->name);
		SetMyRegLong(subkey, "Alarm", current->bEnable);
		SetMyRegStr(subkey, "Hour", current->strHours);
		SetMyRegStr(subkey, "Minute", current->strMinutes);
		SetMyRegStr(subkey, "WDays", current->strWDays);
		SetMyRegLong(subkey, "Second", current->second);
		
		SetMyRegStr(subkey, "File", current->fname);
		SetMyRegLong(subkey, "Hour12", current->bHour12);
		SetMyRegLong(subkey, "Repeat", current->bRepeat);
		SetMyRegLong(subkey, "Blink", current->bBlink);
		SetMyRegLong(subkey, "OnBoot", current->bBootExec);
		SetMyRegLong(subkey, "Interval", current->bInterval);
		SetMyRegLong(subkey, "IntervalMinutes", current->nInterval);
		SetMyRegLong(subkey, "OnResume", current->bResumeExec);
		SetMyRegLong(subkey, "ResumeDelay", current->nResumeDelay);
		
		current = current->next;
		count++;
	}
	
	SetMyRegLong("", "AlarmNum", count);
	
	for(i = count; i < oldcount; i++)
	{
		wsprintf(subkey, "Alarm%d", i + 1);
		DelMyRegKey(subkey);
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
				if(n3 <= 0) n3 = 1;
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
				if(n3 <= 0) n3 = 1;
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
