/*-------------------------------------------------------------------------
  net.c
  get network interface info
---------------------------------------------------------------------------*/

#include "tcdll.h"
#include <iprtrmib.h>

#if TC_ENABLE_NETWORK

#ifndef IF_TYPE_ETHERNET_CSMACD
#define IF_TYPE_ETHERNET_CSMACD			6
#define IF_TYPE_PPP						23
#define IF_TYPE_IEEE80211				71
#endif

#ifndef IF_MAX_STRING_SIZE
#define IF_MAX_STRING_SIZE	256
#endif

#ifndef IF_MAX_PHYS_ADDRESS_LENGTH
#define IF_MAX_PHYS_ADDRESS_LENGTH	32
#endif

typedef struct _MIB_IF_ROW2 {
	//
	// Key structure.  Sorted by preference.
	//
	union {
		ULONG64 Value;
	} InterfaceLuid;
	ULONG InterfaceIndex;
	
	//
	// Read-Only fields.
	//
	GUID InterfaceGuid;
	WCHAR Alias[IF_MAX_STRING_SIZE + 1];
	WCHAR Description[IF_MAX_STRING_SIZE + 1];
	ULONG PhysicalAddressLength;
	UCHAR PhysicalAddress[IF_MAX_PHYS_ADDRESS_LENGTH];
	UCHAR PermanentPhysicalAddress[IF_MAX_PHYS_ADDRESS_LENGTH];
	
	ULONG Mtu;
	ULONG Type;			// Interface Type.
	ULONG TunnelType;	// Tunnel Type, if Type = IF_TUNNEL.
	ULONG MediaType;
	ULONG PhysicalMediumType;
	ULONG AccessType;
	ULONG DirectionType;
	struct {
		BOOLEAN HardwareInterface : 1;
		BOOLEAN FilterInterface : 1;
		BOOLEAN ConnectorPresent : 1;
		BOOLEAN NotAuthenticated : 1;
		BOOLEAN NotMediaConnected : 1;
		BOOLEAN Paused : 1;
		BOOLEAN LowPower : 1;
		BOOLEAN EndPointInterface : 1;
	} InterfaceAndOperStatusFlags;
	
	ULONG OperStatus;
	ULONG AdminStatus;
	ULONG MediaConnectState;
	GUID NetworkGuid;
	ULONG ConnectionType;
	
	//
	// Statistics.
	//
	ULONG64 TransmitLinkSpeed;
	ULONG64 ReceiveLinkSpeed;
	
	ULONG64 InOctets;
	ULONG64 InUcastPkts;
	ULONG64 InNUcastPkts;
	ULONG64 InDiscards;
	ULONG64 InErrors;
	ULONG64 InUnknownProtos;
	ULONG64 InUcastOctets;
	ULONG64 InMulticastOctets;
	ULONG64 InBroadcastOctets;
	ULONG64 OutOctets;
	ULONG64 OutUcastPkts;
	ULONG64 OutNUcastPkts;
	ULONG64 OutDiscards;
	ULONG64 OutErrors;
	ULONG64 OutUcastOctets;
	ULONG64 OutMulticastOctets;
	ULONG64 OutBroadcastOctets;
	ULONG64 OutQLen;
} MIB_IF_ROW2, *PMIB_IF_ROW2;

typedef struct _MIB_IF_TABLE2 {
	ULONG NumEntries;
	MIB_IF_ROW2 Table[ANY_SIZE];
} MIB_IF_TABLE2, *PMIB_IF_TABLE2;


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
