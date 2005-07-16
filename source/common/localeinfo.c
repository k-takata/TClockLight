/*-------------------------------------------------------------
  localeinfo.c : get locale infomation, date and time format
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Globals */

int GetCodePage(int ilang);
int MyGetLocaleInfoW(int ilang, int codepage,
	LCTYPE lctype, wchar_t* dst, int n);
int MyGetLocaleInfoA(int ilang, int codepage,
	LCTYPE lctype, char* dst, int n);
int MyGetDateFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n);
int MyGetTimeFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n);

/*------------------------------------------------
  get codepage from locale
--------------------------------------------------*/
int GetCodePage(int ilang)
{
	int r;
	int codepage = CP_ACP;
	LCID locale;
	
	locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	
	if(GetVersion() & 0x80000000) // 95/98/Me
	{
		char buf[10];
		r = GetLocaleInfoA(locale, LOCALE_IDEFAULTANSICODEPAGE, buf, 10);
		if(r) codepage = atoi(buf);
	}
	else  // NT4/2000/XP
	{
		WCHAR bufw[10];
		r = GetLocaleInfoW(locale, LOCALE_IDEFAULTANSICODEPAGE, bufw, 10);
		if(r) codepage = _wtoi(bufw);
	}
	
	if(!IsValidCodePage(codepage)) codepage = CP_ACP;
	return codepage;
}

/*------------------------------------------------
  get locale information by Unicode
--------------------------------------------------*/
int MyGetLocaleInfoW(int ilang, int codepage,
	LCTYPE lctype, wchar_t* dst, int n)
{
	int r;
	LCID locale;
	
	*dst = 0;
	locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	
	if(GetVersion() & 0x80000000) // 95/98/Me
	{
		char buf[80];
		r = GetLocaleInfoA(locale, lctype, buf, 80);
		if(r)
			MultiByteToWideChar(codepage, 0, buf, -1, dst, n);
	}
	else  // NT4/2000/XP
		r = GetLocaleInfoW(locale, lctype, dst, n);
	
	return r;
}

/*------------------------------------------------
  get locale information by ANSI
--------------------------------------------------*/
int MyGetLocaleInfoA(int ilang, int codepage,
	LCTYPE lctype, char* dst, int n)
{
	int r;
	LCID locale;
	
	*dst = 0;
	locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	
	if(GetVersion() & 0x80000000) // 95/98/Me
		r = GetLocaleInfoA(locale, lctype, dst, n);
	else  // NT4/2000/XP
	{
		WCHAR buf[80];
		r = GetLocaleInfoW(locale, lctype, buf, 80);
		if(r)
			r = WideCharToMultiByte(codepage, 0, buf, -1, dst, n, NULL, NULL);
	}
	
	return r;
}

/*------------------------------------------------
  GetDateFormat() for 95/NT
--------------------------------------------------*/
int MyGetDateFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n)
{
	int r;
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	if(GetVersion() & 0x80000000) // 95
	{
		char buf[80], afmt[40];
		if(fmt)
			WideCharToMultiByte(codepage, 0, fmt, -1,
				afmt, 39, NULL, NULL);
		r = GetDateFormatA(Locale, dwFlags, t, fmt ? afmt : NULL, buf, 79);
		if(r)
			r = MultiByteToWideChar(codepage, 0, buf, -1, dst, n);
	}
	else  // NT
	{
		r = GetDateFormatW(Locale, dwFlags, t, fmt, dst, n);
	}
	return r;
}

/*------------------------------------------------
  GetTimeFormat() for 95/NT
--------------------------------------------------*/
int MyGetTimeFormatW(int ilang, int codepage,
	DWORD dwFlags, CONST SYSTEMTIME *t,
	wchar_t* fmt, wchar_t* dst, int n)
{
	int r;
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	if(GetVersion() & 0x80000000) // 95
	{
		char buf[80], afmt[40];
		if(fmt)
			WideCharToMultiByte(codepage, 0, fmt, -1,
				afmt, 39, NULL, NULL);
		r = GetTimeFormatA(Locale, dwFlags, t, fmt ? afmt : NULL, buf, 79);
		if(r)
			r = MultiByteToWideChar(codepage, 0, buf, -1, dst, n);
	}
	else  // NT
	{
		r = GetTimeFormatW(Locale, dwFlags, t, fmt, dst, n);
	}
	return r;
}

