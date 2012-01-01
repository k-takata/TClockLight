/*-------------------------------------------------------------
  format.c : make a string to display in the clock
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */

void LoadFormatSetting(HWND hwnd);
void MakeFormat(wchar_t* dst, const SYSTEMTIME* pt,
	const wchar_t* fmt, int nMax);

/* Statics */

static wchar_t m_format[BUFSIZE_FORMAT];

/*------------------------------------------------
   format handler functions
--------------------------------------------------*/
typedef void (*HANDLERFUNC)(FORMATHANDLERSTRUCT* pstruc);

struct {
	wchar_t ch;
	wchar_t* prefix;
	HANDLERFUNC func;
} format_handers[] =
{
	{ '/', NULL, SDateHandler },
	{ ':', NULL, STimeHandler },
	{ 'y', NULL, YearHandler },
	{ 'm', NULL, MonthHandler },
	{ 'd', NULL, DateHandler },
	{ 0, L"aaa", DayOfWeekHandler },
	{ 'h', NULL, HourHandler },
	{ 'n', NULL, MinuteHandler },
	{ 's', NULL, SecondHandler },
	{ 0, L"tt", AMPMHandler },
	{ 0, L"\\n", CRLFHandler },
	{ 0, L"\\x", CharaHandler },
	{ 'Y', NULL, AltYearHandler },
	{ 'g', NULL, EraHandler },
	{ 0, L"td", TimeDifHandler },
	{ 0, L"LDATE", LDATEHandler },
	{ 0, L"DATE", DATEHandler },
	{ 0, L"TIME", TIMEHandler },
	{ 0, L"SSS", MSecondHandler }, // only for testing
	
	{ 0, L"USTR", UStrHandler },
	// add your functions
#if TC_ENABLE_ETIME
	{ 'S', NULL, ElapsedTimeHandler },
#endif
#if TC_ENABLE_NETWORK
	{ 'N', NULL, NetworkHandler },
#endif
#if TC_ENABLE_MEMORY
	{ 'M', NULL, MemoryHandler },
#endif
#if TC_ENABLE_HDD
	{ 'H', NULL, HDDHandler },
#endif
#if TC_ENABLE_CPU
	{ 0, L"CU", CPUHandler },
#endif
#if TC_ENABLE_BATTERY
	{ 0, L"BL", BatteryHandler },
	{ 0, L"AD", ACStatusHandler },
#endif
#if TC_ENABLE_VOLUME
	{ 0, L"VL", VolumeMuteHandler },
	{ 0, L"VOL", VolumeHandler },
	{ 0, L"VMT", MuteHandler },
#endif
};

#define NUM_HANDLERS (sizeof(format_handers) / sizeof(format_handers[0])) 

/*------------------------------------------------
   read settings and initialize
--------------------------------------------------*/
void LoadFormatSetting(HWND hwnd)
{
	wchar_t fmt_tmp[BUFSIZE_FORMAT-5];
	
	GetMyRegStrW(NULL, "Format", fmt_tmp, BUFSIZE_FORMAT-5, "");
	if(fmt_tmp[0] == 0) g_bNoClock = TRUE;
	
	// add <% - %> to the clock format string
	wcscpy(m_format, L"<%");
	wcscat(m_format, fmt_tmp);
	wcscat(m_format, L"%>");
	
	g_bDispSecond = FALSE;
	InitFormatTime();      // formattime.c
	
	// add your InitFormatXXX() here
}

/*------------------------------------------------
   make a string from date and time format
--------------------------------------------------*/
void MakeFormat(wchar_t* dst, const SYSTEMTIME* pt,
	const wchar_t* pfmt, int nMax)
{
	SYSTEMTIME st;
	FORMATHANDLERSTRUCT struc;
	int i;
	
	if(pt == NULL) GetLocalTime(&st);
	else memcpy(&st, pt, sizeof(SYSTEMTIME));
	
	if(pfmt == NULL) pfmt = m_format;
	
	struc.dp = dst;
	struc.sp = pfmt;
	struc.pt = &st;
	
	for(i = 0; i < nMax-1; i++) dst[i] = ' ';
	dst[i] = 0;
	
	while(*struc.sp && *struc.dp)
	{
		if(*struc.sp == '<' && *(struc.sp + 1) == '%')
		{
			struc.sp += 2;
			while(*struc.sp && *struc.dp)
			{
				if(*struc.sp == '%' && *(struc.sp + 1) == '>')
				{
					struc.sp += 2;
					break;
				}
				if(*struc.sp == '\"')
				{
					struc.sp++;
					while(*struc.sp != '\"' && *struc.sp && *struc.dp)
						*struc.dp++ = *struc.sp++;
					if(*struc.sp == '\"') struc.sp++;
				}
				else
				{
					for(i = 0; i < NUM_HANDLERS; i++)
					{
						if(*struc.sp == format_handers[i].ch ||
							(format_handers[i].prefix &&
							  wcsncmp(struc.sp, format_handers[i].prefix,
							     wcslen(format_handers[i].prefix)) == 0))
						{
							format_handers[i].func(&struc);
							break;
						}
					}
					if(i == NUM_HANDLERS)
						*struc.dp++ = *struc.sp++;
				}
			}
		}
		else
		{
			*struc.dp++ = *struc.sp++;
		}
	}
	*struc.dp = 0;
}

