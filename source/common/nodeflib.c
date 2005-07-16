/*-------------------------------------------------------------
  nodeflib.c : replacements of standard library's functions
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

#ifdef NODEFAULTLIB

/*-------------------------------------------
  memcpy for BCC
---------------------------------------------*/
void r_memcpy(void *d, const void *s, size_t l)
{
	size_t i;
	for (i = 0; i < l; i++) *((char*)d)++ = *((char*)s)++;
}

/*-------------------------------------------
  memset for BCC
---------------------------------------------*/
void r_memset(void *d, int c, size_t l)
{
	size_t i;
	for (i = 0; i < l; i++) *((char*)d)++ = (char)c;
}

/*-------------------------------------------
  strstr
---------------------------------------------*/
char *r_strstr(const char *string, const char *strCharSet)
{
	const char *p = string;
	while(*p)
	{
		if(strncmp(p, strCharSet, strlen(strCharSet)) == 0)
			return (char*)p;
		p++;
	}
	return NULL;
}

/*-------------------------------------------
  strncmp
---------------------------------------------*/
int r_strncmp(const char* d, const char* s, size_t n)
{
	unsigned int i;
	for(i = 0; i < n; i++)
	{
		if(*s == 0 && *d == 0) break;
		if(*s != *d) return ((int)*d - (int)*s);
		d++; s++;
	}
	return 0;
}

/*-------------------------------------------
  atoi
---------------------------------------------*/
int r_atoi(const char *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 10 + *p - '0';
		else break;
		p++;
	}
	return r;
}

/*-------------------------------------------
  convert a string (hexadecimal) to int
---------------------------------------------*/
int r_atox(const char *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 16 + *p - '0';
		else if('A' <= *p && *p <= 'F')
			r = r * 16 + *p - 'A' + 10;
		else if('a' <= *p && *p <= 'f')
			r = r * 16 + *p - 'a' + 10;
		p++;

	}
	return r;
}

/*-------------------------------------------
  wtoi
---------------------------------------------*/
int r__wtoi(const WCHAR *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 10 + *p - '0';
		else break;
		p++;
	}
	return r;
}

/*-------------------------------------------
  wcslen
---------------------------------------------*/
size_t r_wcslen(const wchar_t *p)
{
	size_t n = 0; while(*p) { n++; p++; } return n;
}

/*-------------------------------------------
  wcscpy
---------------------------------------------*/
wchar_t *r_wcscpy(wchar_t *dp, const wchar_t *sp)
{
	wchar_t *p = dp;
	while(*sp) *p++ = *sp++; *p = 0; return dp;
}

/*-------------------------------------------
  wcsncmp
---------------------------------------------*/
int r_wcsncmp(const wchar_t *p1, const wchar_t *p2, size_t count)
{
	size_t i;
	for(i = 0; i < count; i++)
	{
		if(*p1 == 0 && *p2 == 0) break;
		if(*p1 != *p2) return ((int)*p1 - (int)*p2);
		p1++; p2++;
	}
	return 0;
}

/*-------------------------------------------
  wcscat
---------------------------------------------*/
wchar_t *r_wcscat(wchar_t *dp, const wchar_t *sp)
{
	wchar_t *p = dp;
	while(*p) p++;
	while(*sp) *p++ = *sp++; *p = 0; return dp;
}

/*-------------------------------------------
  wcsstr
---------------------------------------------*/
wchar_t *r_wcsstr(const wchar_t *string, const wchar_t *strCharSet)
{
	const wchar_t *p = string;
	while(*p)
	{
		if(wcsncmp(p, strCharSet, wcslen(strCharSet)) == 0)
			return (wchar_t *)p;
		p++;
	}
	return NULL;
}

/*-------------------------------------------
  32bit x 32bit = 64bit
---------------------------------------------*/

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
typedef LARGE_INTEGER TC_SINT64;
typedef ULARGE_INTEGER TC_UINT64;
#else
typedef union _TC_SINT64 {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} TC_SINT64;
typedef union _TC_UINT64 {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} TC_UINT64;
#endif

DWORDLONG r_M32x32to64(DWORD a, DWORD b)
{
	TC_UINT64 r;
	DWORD *p1, *p2, *p3;
	memset(&r, 0, 8);
	p1 = &r.u.LowPart;
	p2 = (DWORD*)((BYTE*)p1 + 2);
	p3 = (DWORD*)((BYTE*)p2 + 2);
	*p1 = LOWORD(a) * LOWORD(b);
	*p2 += LOWORD(a) * HIWORD(b) + HIWORD(a) * LOWORD(b);
	*p3 += HIWORD(a) * HIWORD(b);
	return *(DWORDLONG*)(&r);
}

#endif

