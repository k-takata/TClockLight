/*-------------------------------------------------------------------------
  net.c
  get network interface info
---------------------------------------------------------------------------*/

#include "tcdll.h"
#include <iprtrmib.h>

void Net_start(void);
void Net_get(double *recv, double *send);
void Net_end(void);

static HMODULE hmodIPHLP;
static MIB_IFTABLE *ift;
static DWORD (WINAPI *pGetIfTable)(PMIB_IFTABLE, PULONG, BOOL);
static DWORD (WINAPI *pGetIfEntry)(PMIB_IFROW);


void Net_start(void)
{
	if(hmodIPHLP) Net_end();

	hmodIPHLP = LoadLibrary("iphlpapi.dll");
	if(!hmodIPHLP) return;
	
	(FARPROC)pGetIfTable = GetProcAddress(hmodIPHLP, "GetIfTable");
	(FARPROC)pGetIfEntry = GetProcAddress(hmodIPHLP, "GetIfEntry");
	
	if(pGetIfTable && pGetIfEntry)
	{
		ULONG bufsize = 0;
		if(pGetIfTable(ift, &bufsize, TRUE) == ERROR_INSUFFICIENT_BUFFER)
		{
			if((ift = (MIB_IFTABLE*)malloc(bufsize)) != NULL)
			{
				if(pGetIfTable(ift, &bufsize, TRUE) == NO_ERROR)
				{
					return;
				}
			}
		}
	}

	Net_end();
}

void Net_get(double *recv, double *send)
{
	*recv = *send = 0;

	if(ift && hmodIPHLP)
	{
		int i;

		for(i = ift->dwNumEntries - 1; i >= 0 ; i--)
		{
			MIB_IFROW *ifr = &ift->table[i];
			if(ifr->dwType == MIB_IF_TYPE_ETHERNET ||
				ifr->dwType == MIB_IF_TYPE_PPP)
			{
				pGetIfEntry(ifr);
				*recv += ifr->dwInOctets;
				*send += ifr->dwOutOctets;
			}
		}
	}
}

void Net_end(void)
{
	if (hmodIPHLP) { FreeLibrary(hmodIPHLP); hmodIPHLP = NULL; }
	if (ift) { free(ift); ift = NULL; }
}

