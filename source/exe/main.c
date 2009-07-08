/*-------------------------------------------------------------
  main.c : entry point of tclock.exe
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

/* Globals */

HINSTANCE g_hInst;  // instance handle

/*----------------------------------------------------------
  TClock doesn't use "WinMain" for compacting the file size
-----------------------------------------------------------*/
#ifdef NODEFAULTLIB
void /*WINAPI*/ WinMainCRTStartup(void)
{
	g_hInst = GetModuleHandle(NULL);
	ExitProcess(TClockExeMain());
}
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	return TClockExeMain();
}
#endif

