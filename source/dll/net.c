/*-------------------------------------------------------------------------
  net.c
  get network interface info
---------------------------------------------------------------------------*/

#include "tcdll.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#if TC_ENABLE_NETWORK

static HMODULE hmodIPHLP = NULL;
static MIB_IF_TABLE2 *ift2 = NULL;
static DWORD (WINAPI *pGetIfTable2)(PMIB_IF_TABLE2 *);
static DWORD (WINAPI *pGetIfEntry2)(PMIB_IF_ROW2);
static VOID (WINAPI *pFreeMibTable)(PVOID);

void Net_start(void)
{
	if(hmodIPHLP) Net_end();

	hmodIPHLP = LoadLibrary("iphlpapi.dll");
	if(!hmodIPHLP) return;

	// Vista
	(FARPROC)pGetIfTable2 = GetProcAddress(hmodIPHLP, "GetIfTable2");
	(FARPROC)pGetIfEntry2 = GetProcAddress(hmodIPHLP, "GetIfEntry2");
	(FARPROC)pFreeMibTable = GetProcAddress(hmodIPHLP, "FreeMibTable");
	
	if(pGetIfTable2 && pGetIfEntry2 && pFreeMibTable)
	{
		if(pGetIfTable2(&ift2) == NO_ERROR)
		{
			return;
		}
	}

	Net_end();
}

void Net_get(ULONGLONG *recv, ULONGLONG *send)
{
	ULONGLONG urecv = 0;
	ULONGLONG usend = 0;

	if(hmodIPHLP == NULL)
	{
		*recv = 0;
		*send = 0;
		return;
	}

	if(ift2)		// Vista
	{
		int i;

		for(i = ift2->NumEntries - 1; i >= 0 ; i--)
		{
			MIB_IF_ROW2 *ifr2 = &ift2->Table[i];
			if(!ifr2->InterfaceAndOperStatusFlags.FilterInterface &&
				(ifr2->Type == IF_TYPE_ETHERNET_CSMACD ||
				 ifr2->Type == IF_TYPE_IEEE80211 ||
				 ifr2->Type == IF_TYPE_PPP))
			{
				pGetIfEntry2(ifr2);
				urecv += ifr2->InOctets;
				usend += ifr2->OutOctets;
			}
		}
	}
	*recv = urecv;
	*send = usend;
}

void Net_end(void)
{
	if(hmodIPHLP)
	{
		if(ift2)
		{
			pFreeMibTable(ift2);
			ift2 = NULL;
		}
		FreeLibrary(hmodIPHLP);
		hmodIPHLP = NULL;
	}
}

#endif	/* TC_ENABLE_NETWORK */
