/*-------------------------------------------------------------
  localeinfo.c : get locale infomation, date and time format
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/*------------------------------------------------
  get codepage from locale
--------------------------------------------------*/
int GetCodePage(int ilang)
{
	WCHAR bufw[10];
	int r;
	int codepage = CP_ACP;
	LCID locale;
	
	locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	
	r = GetLocaleInfoW(locale, LOCALE_IDEFAULTANSICODEPAGE, bufw, 10);
	if(r) codepage = _wtoi(bufw);
	
	if(!IsValidCodePage(codepage)) codepage = CP_ACP;
	return codepage;
}

/*------------------------------------------------
  get locale information by Unicode
--------------------------------------------------*/
int MyGetLocaleInfoW(int ilang, int codepage,
	LCTYPE lctype, wchar_t* dst, int n)
{
	LCID locale;
	
	*dst = 0;
	locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	return GetLocaleInfoW(locale, lctype, dst, n);
}

/*------------------------------------------------
  get locale information by ANSI
--------------------------------------------------*/
int MyGetLocaleInfoA(int ilang, int codepage,
	LCTYPE lctype, char* dst, int n)
{
	LCID locale;
	
	*dst = 0;
	locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	return GetLocaleInfoA(locale, lctype, dst, n);
}

/*------------------------------------------------
  GetDateFormat() for 95/NT
--------------------------------------------------*/
int MyGetDateFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n)
{
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	return GetDateFormatW(Locale, dwFlags, t, fmt, dst, n);
}

/*------------------------------------------------
  GetTimeFormat() for 95/NT
--------------------------------------------------*/
int MyGetTimeFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n)
{
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	return GetTimeFormatW(Locale, dwFlags, t, fmt, dst, n);
}

