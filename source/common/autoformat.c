/*-------------------------------------------------------------
  autoformat.c : assemble time format automatically
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

/* memo: used in exe/main2.c, property/pageformat.c */

#include "common.h"

/* Statics */

static int  m_idate;              // 0: mm/dd/yy 1: dd/mm/yy 2: yy/mm/dd
static wchar_t m_sMon[11];        // abbreviated name for Monday
static BOOL m_bDayOfWeekIsLast;   // yy/mm/dd ddd
static BOOL m_bTimeMarkerIsFirst; // AM/PM hh:nn:ss

/*------------------------------------------------
  initialize locale information
--------------------------------------------------*/
void InitAutoFormat(int ilang)
{
	char s[21];
	int codepage;
	int i;
	int aLangDayOfWeekIsLast[] =
		{ LANG_JAPANESE, LANG_KOREAN, 0 };
	int aTimeMarkerIsFirst[] =
		{ LANG_CHINESE, LANG_JAPANESE, LANG_KOREAN, 0 };
	
	codepage = GetCodePage(ilang);
	
	MyGetLocaleInfoA(ilang, codepage, LOCALE_IDATE, s, 20);
	m_idate = atoi(s);
	MyGetLocaleInfoW(ilang, codepage, LOCALE_SABBREVDAYNAME1, m_sMon, 10);
	
	m_bDayOfWeekIsLast = FALSE;
	for(i = 0; aLangDayOfWeekIsLast[i]; i++)
	{
		if((ilang & 0x00ff) == aLangDayOfWeekIsLast[i])
		{
			m_bDayOfWeekIsLast = TRUE; break;
		}
	}
	
	m_bTimeMarkerIsFirst = FALSE;
	for(i = 0; aTimeMarkerIsFirst[i]; i++)
	{
		if((ilang & 0x00ff) == aTimeMarkerIsFirst[i])
		{
			m_bTimeMarkerIsFirst = TRUE; break;
		}
	}
}

/*------------------------------------------------
  create a format string automatically
--------------------------------------------------*/
void AutoFormat(char* dst, BOOL* parts)
{
	BOOL bdate = FALSE, btime = FALSE;
	BOOL bymd = FALSE, bhms = FALSE;
	BOOL byear = FALSE, bmonth = FALSE, bday = FALSE;
	int i;
	
	for(i = PART_YEAR4; i <= PART_WEEKDAY; i++)
	{
		if(parts[i]) { bdate = TRUE; break; }
	}
	for(i = PART_HOUR; i <= PART_AMPM; i++)
	{
		if(parts[i]) { btime = TRUE; break; }
	}
	for(i = PART_YEAR4; i <= PART_DAY; i++)
	{
		if(parts[i]) { bymd = TRUE; break; }
	}
	for(i = PART_HOUR; i <= PART_SECOND; i++)
	{
		if(parts[i]) { bhms = TRUE; break; }
	}
	
	if(parts[PART_YEAR4] || parts[PART_YEAR])   byear = TRUE;
	if(parts[PART_MONTH] || parts[PART_MONTHS]) bmonth = TRUE;
	if(parts[PART_DAY]) bday = TRUE;
	
	dst[0] = 0;
	
	// day of week
	
	if(!m_bDayOfWeekIsLast && parts[PART_WEEKDAY])
	{
		strcat(dst, "ddd");
		if(bymd)
		{
			if(m_sMon[0] && m_sMon[ wcslen(m_sMon) - 1 ] > 0x7f)
				strcat(dst, " ");
			else if(m_sMon[0] && m_sMon[ wcslen(m_sMon) - 1 ] == '.')
				strcat(dst, " ");
			else strcat(dst, ", ");
		}
	}
	
	// year, month, date
	
	if(m_idate == 0) // mm/dd/yy
	{
		if(bmonth) // month
		{
			if(parts[PART_MONTH])  strcat(dst, "mm");
			if(parts[PART_MONTHS]) strcat(dst, "mmm");
			if(bday || byear)
			{
				if(parts[PART_MONTH]) strcat(dst, "/");
				else strcat(dst, " ");
			}
		}
		if(bday)  // date
		{
			strcat(dst, "dd");
			if(byear)
			{
				if(parts[PART_MONTH]) strcat(dst, "/");
				else strcat(dst, ", ");
			}
		}
		// year
		if(parts[PART_YEAR4]) strcat(dst, "yyyy");
		if(parts[PART_YEAR])  strcat(dst, "yy");
	}
	else if(m_idate == 1) // dd/mm/yy
	{
		if(bday)  // date
		{
			strcat(dst, "dd");
			if(bmonth)
			{
				if(parts[PART_MONTH]) strcat(dst, "/");
				else strcat(dst, " ");
			}
			else if(byear) strcat(dst, "/");
		}
		if(bmonth) // month
		{
			if(parts[PART_MONTH])  strcat(dst, "mm");
			if(parts[PART_MONTHS]) strcat(dst, "mmm");
			if(byear)
			{
				if(parts[PART_MONTH]) strcat(dst, "/");
				else strcat(dst, " ");
			}
		}
		// year
		if(parts[PART_YEAR4]) strcat(dst, "yyyy");
		if(parts[PART_YEAR])  strcat(dst, "yy");
	}
	else // yy/mm/dd
	{
		if(byear)  // year
		{
			if(parts[PART_YEAR4]) strcat(dst, "yyyy");
			if(parts[PART_YEAR])  strcat(dst, "yy");
			if(bmonth || bday)
			{
				if(parts[PART_MONTHS]) strcat(dst, " ");
				else strcat(dst, "/");
			}
		}
		if(bmonth)  // month
		{
			if(parts[PART_MONTH])  strcat(dst, "mm");
			if(parts[PART_MONTHS]) strcat(dst, "mmm");
			if(bday)
			{
				if(parts[PART_MONTHS]) strcat(dst, " ");
				else strcat(dst, "/");
			}
		}
		// date
		if(bday) strcat(dst, "dd");
	}
	
	// day of week
	
	if(m_bDayOfWeekIsLast && parts[PART_WEEKDAY])
	{
		if(bymd) strcat(dst, " ");
		strcat(dst, "ddd");
	}
	
	// space or line break between date and time
	
	if(bdate && btime)
	{
		if(parts[PART_BREAK]) strcat(dst, "\\n");
		else
		{
			if(m_idate < 2 && parts[PART_MONTHS] && byear)
				strcat(dst, " ");
			strcat(dst, " ");
		}
	}
	
	// AM/PM
	
	if(m_bTimeMarkerIsFirst && parts[PART_AMPM])
	{
		strcat(dst, "tt");
		if(bhms) strcat(dst, " ");
	}
	
	// hour
	
	if(parts[PART_HOUR])
	{
		strcat(dst, "_h");
		if(parts[PART_MINUTE] || parts[PART_SECOND])
			strcat(dst, ":");
		else if(!m_bTimeMarkerIsFirst && parts[PART_AMPM])
			strcat(dst, " ");
	}
	
	// minute
	
	if(parts[PART_MINUTE])
	{
		strcat(dst, "nn");
		if(parts[PART_SECOND]) strcat(dst, ":");
		else if(!m_bTimeMarkerIsFirst && parts[PART_AMPM])
			strcat(dst, " ");
	}
	
	// second
	
	if(parts[PART_SECOND])
	{
		strcat(dst, "ss");
		if(!m_bTimeMarkerIsFirst && parts[PART_AMPM])
			strcat(dst, " ");
	}
	
	// AM/PM
	
	if(!m_bTimeMarkerIsFirst && parts[PART_AMPM])
		strcat(dst, "tt");
}

