/*-------------------------------------------------------------
  memreduce.c : to call SetProcessWorkingSetSize API
  (C) 1997-2004 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

static HMODULE m_hmodKERNEL32 = NULL;
static BOOL (WINAPI *m_pSetProcessWorkingSetSize)(HANDLE, SIZE_T, SIZE_T)
	= NULL;

void InitMemReduce(void)
{
	if(g_winver&WIN2000 && GetMyRegLong("", "MemReduce", 0))
	{
		m_hmodKERNEL32 = LoadLibrary("kernel32.dll");
		if(m_hmodKERNEL32)
		{
			(FARPROC)m_pSetProcessWorkingSetSize
				= GetProcAddress(m_hmodKERNEL32,
					"SetProcessWorkingSetSize");
		}
	}
}

void MemReduce(void)
{
	if(m_pSetProcessWorkingSetSize)
		m_pSetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}

void EndMemReduce(void)
{
	if(m_hmodKERNEL32) FreeLibrary(m_hmodKERNEL32);
	m_hmodKERNEL32 = NULL;
	m_pSetProcessWorkingSetSize = NULL;
}

