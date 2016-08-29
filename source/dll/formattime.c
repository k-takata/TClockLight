/*-------------------------------------------------------------
  formattime.c : handle format picture strings of time
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Statics */

static BOOL m_bHour12, m_bHourZero;
static wchar_t m_DayOfWeekShort[7][11], m_DayOfWeekLong[7][31];
static wchar_t m_MonthShort[12][11], m_MonthLong[12][31];
static wchar_t *m_DayOfWeekEng[7] =
  { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
static wchar_t *m_MonthEng[12] =
  { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
    L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
static wchar_t m_AM[11], m_PM[11], m_SDate[5], m_STime[5];
static wchar_t m_EraStr[11];
static int m_AltYear;

/*------------------------------------------------
  initialize string of day, month
--------------------------------------------------*/
void InitFormatTime(void)
{
	SYSTEMTIME t;
	wchar_t s[80];
	int i, ilang, ioptcal, codepage;
	int ilangcal, codepagecal;
	
//	g_bDispSecond = FALSE;
	
	GetLocalTime(&t);
	
	m_bHour12 = GetMyRegLong(NULL, "Hour12", 0);
	m_bHourZero = GetMyRegLong(NULL, "HourZero", 0);
	
	ilang = GetMyRegLong(NULL, "Locale", (int)GetUserDefaultLangID());
	
	codepage = GetCodePage(ilang);
	
	for(i = 0; i < 7; i++)
	{
		int j;
		if(i == 0) j = 6; else j = i - 1;
		MyGetLocaleInfoW(ilang, codepage, LOCALE_SABBREVDAYNAME1 + j,
			m_DayOfWeekShort[i], 10);
		MyGetLocaleInfoW(ilang, codepage, LOCALE_SDAYNAME1 + j,
			m_DayOfWeekLong[i], 30);
	}
	for(i = 0; i < 12; i++)
	{
		MyGetLocaleInfoW(ilang, codepage, LOCALE_SABBREVMONTHNAME1 + i,
			m_MonthShort[i], 10);
		MyGetLocaleInfoW(ilang, codepage, LOCALE_SMONTHNAME1 + i,
			m_MonthLong[i], 30);
	}
	
	GetMyRegStrW(NULL, "AMsymbol", s, 80, "");
	if(s[0]) wcscpy(m_AM, s);
	else
	{
		MyGetLocaleInfoW(ilang, codepage, LOCALE_S1159, m_AM, 10);
		if(m_AM[0] == 0) wcscpy(m_AM, L"AM");
	}
	GetMyRegStrW(NULL, "PMsymbol", s, 80, "");
	if(s[0]) wcscpy(m_PM, s);
	else
	{
		MyGetLocaleInfoW(ilang, codepage, LOCALE_S2359, m_PM, 10);
		if(m_PM[0] == 0) wcscpy(m_PM, L"PM");
	}
	
	MyGetLocaleInfoW(ilang, codepage, LOCALE_SDATE, m_SDate, 4);
	MyGetLocaleInfoW(ilang, codepage, LOCALE_STIME, m_STime, 4);
	
	m_EraStr[0] = 0;
	m_AltYear = 0;
	
	ioptcal = 0;
	ilangcal = ilang;
	codepagecal = codepage;
	if(MyGetLocaleInfoW(ilang, codepage, LOCALE_IOPTIONALCALENDAR,
		s, 10) > 0)
	{
		ioptcal = _wtoi(s);
	}
	if(ioptcal == 0)
	{
		ilangcal = GetUserDefaultLangID();
		codepagecal = GetCodePage(ilangcal);
		if(MyGetLocaleInfoW(ilangcal, codepagecal,
			LOCALE_IOPTIONALCALENDAR, s, 10) > 0)
		{
			ioptcal = _wtoi(s);
		}
	}
	
	if(ioptcal > 0)
	{
		if(MyGetDateFormatW(ilangcal, codepagecal,
			DATE_USE_ALT_CALENDAR, &t, L"gg", s, 12) > 0)
			wcscpy(m_EraStr, s);
		
		if(MyGetDateFormatW(ilangcal, codepagecal,
			DATE_USE_ALT_CALENDAR, &t, L"yyyy", s, 6) > 0)
			m_AltYear = _wtoi(s);
	}
}

/*------------------------------------------------
  format handlers
--------------------------------------------------*/

/* / */
void SDateHandler(FORMATHANDLERSTRUCT* pstruc)
{
	const wchar_t *p = m_SDate;
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	pstruc->sp++;
}

/* : */
void STimeHandler(FORMATHANDLERSTRUCT* pstruc)
{
	const wchar_t *p = m_STime;
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	pstruc->sp++;
}

/* y, yy, yyy, yyyy */
void YearHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int keta = 0;
	int year = (int)pstruc->pt->wYear;
	
	while(*pstruc->sp == 'y')
	{
		pstruc->sp++;
		keta++;
		if(keta == 4) break;
	}
	
	if(keta == 4)
	{
		if(*pstruc->dp)
			*pstruc->dp++ = (wchar_t)(year / 1000 + '0');
	}
	if(keta >= 3)
	{
		if(*pstruc->dp)
			*pstruc->dp++ = (wchar_t)((year % 1000) / 100 + '0');
	}
	if(keta >= 2)
	{
		if(*pstruc->dp)
			*pstruc->dp++ = (wchar_t)((year % 100) / 10 + '0');
	}
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)(year % 10 + '0');
}

/* m, mm, _m, mmm, mmmm, mme */
void MonthHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int keta = 0;
	BOOL padsp = (*pstruc->sp == '_' && !pstruc->bZeroPad);
	
	while(*pstruc->sp == 'm' || *pstruc->sp == '_')
	{
		pstruc->sp++;
		keta++;
		if(keta == 4) break;
	}
	
	if(keta == 2 && *pstruc->sp == 'e')
	{
		const wchar_t* p;
		pstruc->sp++;
		p = m_MonthEng[pstruc->pt->wMonth-1];
		while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	}
	else
	{
		if(keta >= 3)
		{
			const wchar_t* p;
			if(keta == 4) p = m_MonthLong[pstruc->pt->wMonth-1];
			else p = m_MonthShort[pstruc->pt->wMonth-1];
			while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
		}
		else
		{
			int mon = (int)pstruc->pt->wMonth;
			if(keta == 2 || mon > 9)
			{
				if(*pstruc->dp)
				{
					wchar_t c = (wchar_t)(mon / 10 + '0');
					if(padsp && c == '0') c = ' ';
					*pstruc->dp++ = c;
				}
			}
			if(*pstruc->dp)
				*pstruc->dp++ = (wchar_t)(mon % 10 + '0');
		}
	}
}

/* d, dd, _d, ddd, dddd, dde */
void DateHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int keta = 0;
	BOOL padsp = (*pstruc->sp == '_' && !pstruc->bZeroPad);
	
	while(*pstruc->sp == 'd' || *pstruc->sp == '_')
	{
		pstruc->sp++;
		keta++;
		if(keta == 4) break;
	}
	
	if(keta == 2 && *pstruc->sp == 'e')
	{
		const wchar_t* p;
		pstruc->sp++;
		p = m_DayOfWeekEng[pstruc->pt->wDayOfWeek];
		while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	}
	else
	{
		if(keta >= 3) {
			const wchar_t* p;
			if(keta == 4) p = m_DayOfWeekLong[pstruc->pt->wDayOfWeek];
			else p = m_DayOfWeekShort[pstruc->pt->wDayOfWeek];
			while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
		}
		else
		{
			int day = (int)pstruc->pt->wDay;
			if(keta == 2 || day > 9)
			{
				if(*pstruc->dp)
				{
					wchar_t c = (wchar_t)(day / 10 + '0');
					if(padsp && c == '0') c = ' ';
					*pstruc->dp++ = c;
				}
			}
			if(*pstruc->dp)
				*pstruc->dp++ = (wchar_t)(day % 10 + '0');
		}
	}
}

/* aaa, aaaa */
void DayOfWeekHandler(FORMATHANDLERSTRUCT* pstruc)
{
	const wchar_t* p;
	
	pstruc->sp += 3;
	if(*pstruc->sp == 'a')
	{
		pstruc->sp++;
		p = m_DayOfWeekLong[pstruc->pt->wDayOfWeek];
	}
	else p = m_DayOfWeekShort[pstruc->pt->wDayOfWeek];
	
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
}

/* h, hh, _h */
void HourHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int keta = 1;
	int hour = (int)pstruc->pt->wHour;
	BOOL padsp = (*pstruc->sp == '_' && !pstruc->bZeroPad);
	
	if(m_bHour12)
	{
		if(hour > 12) hour -= 12;
		else if(hour == 0) hour = 12;
		if(hour == 12 && m_bHourZero) hour = 0;
	}
	
	pstruc->sp++;
	if(*pstruc->sp == 'h') { keta++; pstruc->sp++; }
	if(keta == 2 || hour > 9)
	{
		if(*pstruc->dp)
		{
			wchar_t c = (wchar_t)(hour / 10 + '0');
			if(padsp && c == '0') c = ' ';
			*pstruc->dp++ = c;
		}
	}
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)(hour % 10 + '0');
}

/* n, nn, _n */
void MinuteHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int keta = 1;
	int min = (int)pstruc->pt->wMinute;
	BOOL padsp = (*pstruc->sp == '_' && !pstruc->bZeroPad);
	
	pstruc->sp++;
	if(*pstruc->sp == 'n') { keta++; pstruc->sp++; }
	if(keta == 2 || min > 9)
	{
		if(*pstruc->dp)
		{
			wchar_t c = (wchar_t)(min / 10 + '0');
			if(padsp && c == '0') c = ' ';
			*pstruc->dp++ = c;
		}
	}
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)(min % 10 + '0');
}

/* s, ss */
void SecondHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int keta = 1;
	int sec = (int)pstruc->pt->wSecond;
	
	pstruc->sp++;
	if(*pstruc->sp == 's') { keta++; pstruc->sp++; }
	if(keta == 2 || sec > 9)
	{
		if(*pstruc->dp)
			*pstruc->dp++ = (wchar_t)(sec / 10 + '0');
	}
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)(sec % 10 + '0');
	
	g_bDispSecond = TRUE;
}

/* tt */
void AMPMHandler(FORMATHANDLERSTRUCT* pstruc)
{
	const wchar_t* p;
	
	pstruc->sp += 2;
	if(pstruc->pt->wHour < 12) p = m_AM; else p = m_PM;
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
}

/* \n */
void CRLFHandler(FORMATHANDLERSTRUCT* pstruc)
{
	pstruc->sp += 2;
	if(*pstruc->dp && *(pstruc->dp + 1))
	{
		*pstruc->dp++ = 0x0d; *pstruc->dp++ = 0x0a;
	}
}

/* \x1234; */
void CharaHandler(FORMATHANDLERSTRUCT* pstruc)
{
	unsigned int ch = 0;
	
	pstruc->sp += 2;
	while(*pstruc->sp)
	{
		if('0' <= *pstruc->sp && *pstruc->sp <= '9')
			ch = ch * 16 + *pstruc->sp - '0';
		else if('A' <= *pstruc->sp && *pstruc->sp <= 'F')
			ch = ch * 16 + *pstruc->sp - 'A' + 10;
		else if('a' <= *pstruc->sp && *pstruc->sp <= 'f')
			ch = ch * 16 + *pstruc->sp - 'a' + 10;
		else
		{
			if(*pstruc->sp == ';') pstruc->sp++;
			break;
		}
		pstruc->sp++;
	}
	
	if(ch == 0) ch = ' ';
	if(ch >= 0x10000)
	{
		// surrogate pair
		if(*pstruc->dp) *pstruc->dp++ = (wchar_t)((ch >> 10) + 0xd7c0);
		ch = (ch & 0x3ff) + 0xdc00;
	}
	if(*pstruc->dp) *pstruc->dp++ = (wchar_t)ch;
}

/* Y */
void AltYearHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int n = 1;
	while(*pstruc->sp == 'Y') { n *= 10; pstruc->sp++; }
	if(m_AltYear < 0) return;
	while(n <= m_AltYear) n *= 10;
	while(1)
	{
		if(*pstruc->dp)
			*pstruc->dp++ = (wchar_t)((m_AltYear % n) / (n / 10) + '0');
		if(n == 10) break;
		n /= 10;
	}
}

/* g */
void EraHandler(FORMATHANDLERSTRUCT* pstruc)
{
	const wchar_t *p;
	p = m_EraStr;
	while(*p && *pstruc->sp  == 'g' && *pstruc->dp)
	{
		*pstruc->dp++ = *p++;
		pstruc->sp++;
	}
	while(*pstruc->sp == 'g') pstruc->sp++;
}

/* td[+/-]hh:nn : time difference */
void TimeDifHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int dif;
	BOOL bMinus;
	FILETIME ft;
	
	pstruc->sp += 2;
	if(*pstruc->sp == '-') bMinus = TRUE;
	else if(*pstruc->sp == '+') bMinus = FALSE;
	else return;
	pstruc->sp++;
	
	if('0' <= *pstruc->sp && *pstruc->sp <= '9')
		dif = *pstruc->sp++ - '0';
	else return;
	if('0' <= *pstruc->sp && *pstruc->sp <= '9')
		dif = dif * 10 + *pstruc->sp++ - '0';
	dif *= 60;
	if(*pstruc->sp == ':')
	{
		int dif2;
		pstruc->sp++;
		if('0' <= *pstruc->sp && *pstruc->sp <= '9')
		{
			dif2 = *pstruc->sp++ - '0';
			if('0' <= *pstruc->sp && *pstruc->sp <= '9')
				dif2 = dif2 * 10 + *pstruc->sp++ - '0';
		}
		dif += dif2;
	}
	
	SystemTimeToFileTime(pstruc->pt, &ft);
	if(bMinus) *(DWORDLONG*)&ft -= M32x32to64(dif * 60, 10000000);
	else       *(DWORDLONG*)&ft += M32x32to64(dif * 60, 10000000);
	FileTimeToSystemTime(&ft, pstruc->pt);
}

/* LDATE */
void LDATEHandler(FORMATHANDLERSTRUCT* pstruc)
{
	wchar_t s[80];
	const wchar_t* p;
	MyGetDateFormatW(GetUserDefaultLangID(),
		GetCodePage(GetUserDefaultLangID()),
		DATE_LONGDATE, pstruc->pt, NULL, s, 80);
	p = s;
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	pstruc->sp += 5;
}

/* DATE */
void DATEHandler(FORMATHANDLERSTRUCT* pstruc)
{
	wchar_t s[80];
	const wchar_t* p;
	MyGetDateFormatW(GetUserDefaultLangID(),
		GetCodePage(GetUserDefaultLangID()),
		DATE_SHORTDATE, pstruc->pt, NULL, s, 80);
	p = s;
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	pstruc->sp += 4;
}

/* TIME */
void TIMEHandler(FORMATHANDLERSTRUCT* pstruc)
{
	wchar_t s[80];
	const wchar_t* p;
	MyGetTimeFormatW(GetUserDefaultLangID(),
		GetCodePage(GetUserDefaultLangID()),
		TIME_FORCE24HOURFORMAT, pstruc->pt, NULL, s, 80);
	p = s;
	while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	pstruc->sp += 4;
}

/* SSS  only for testing */
void MSecondHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int msec = (int)pstruc->pt->wMilliseconds;
	
	pstruc->sp += 3;
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)((msec % 1000) / 100 + '0');
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)((msec % 100) / 10 + '0');
	if(*pstruc->dp)
		*pstruc->dp++ = (wchar_t)(msec % 10 + '0');
}

