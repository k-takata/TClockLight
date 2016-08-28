/*-------------------------------------------------------------
  userstr.c : user strings in format
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tcdll.h"

/* Globals */

// user string
wchar_t g_userstr[10][BUFSIZE_USTR];

// strings to be replaced / appended
wchar_t g_sdisp1[BUFSIZE_DISP] = { 0 }, g_sdisp2[BUFSIZE_DISP] = { 0 };
wchar_t g_scat1[BUFSIZE_DISP] = { 0 }, g_scat2[BUFSIZE_DISP] = { 0 };

/*------------------------------------------------
  initialize
--------------------------------------------------*/
void InitUserStr(void)
{
	int i;
	
	for(i = 0; i < 10; i++) g_userstr[i][0] = 0;
	
	g_sdisp1[0] = g_sdisp2[0] = g_scat1[0] = g_scat2[0] = 0;
}

/*------------------------------------------------
  format handler
--------------------------------------------------*/
void UStrHandler(FORMATHANDLERSTRUCT* pstruc)
{
	const wchar_t *p;
	int n;
	
	pstruc->sp += 4;
	n = *pstruc->sp - '0';
	if(0 <= n && n <= 9)
	{
		pstruc->sp++;
		
		p = g_userstr[n];
		while(*p && *pstruc->dp) *pstruc->dp++ = *p++;
	}
}

