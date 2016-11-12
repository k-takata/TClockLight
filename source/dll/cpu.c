/*-------------------------------------------------------------------------
  cpu.c
  get percentage of CPU usage
  Kazubon 2001
  tapetums 2016
---------------------------------------------------------------------------*/

#include "tcdll.h"

#if TC_ENABLE_CPU

#include <pdh.h>

#define MAX_PROCESSOR 8

// cpu usage
int CPUUsage[MAX_PROCESSOR] = { 0 };

HMODULE    hmodPDH = NULL;
PDH_HQUERY hQuery  = NULL;

typedef PDH_STATUS (WINAPI* pfnPdhOpenQueryW)              (LPCWSTR, DWORD_PTR, PDH_HQUERY*);
typedef PDH_STATUS (WINAPI* pfnPdhAddCounterW)             (PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
typedef PDH_STATUS (WINAPI* pfnPdhCollectQueryData)        (PDH_HQUERY);
typedef PDH_STATUS (WINAPI* pfnPdhGetFormattedCounterValue)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
typedef PDH_STATUS (WINAPI* pfnPdhCloseQuery)              (PDH_HQUERY);
typedef PDH_STATUS (WINAPI* pfnPdhRemoveCounter)           (PDH_HCOUNTER);

pfnPdhOpenQueryW               pPdhOpenQueryW               = NULL;
pfnPdhAddCounterW              pPdhAddCounterW              = NULL;
pfnPdhCollectQueryData         pPdhCollectQueryData         = NULL;
pfnPdhGetFormattedCounterValue pPdhGetFormattedCounterValue = NULL;
pfnPdhCloseQuery               pPdhCloseQuery               = NULL;
pfnPdhRemoveCounter            pPdhRemoveCounter            = NULL;

PDH_HCOUNTER hTotalCPUCounter           = NULL;
PDH_HCOUNTER hCPUCounter[MAX_PROCESSOR] = { NULL };

void CpuMoni_start(void)
{
	PDH_STATUS status;
	wchar_t counterName[64];
	int i;

	if ( hmodPDH )
	{
		CpuMoni_end();
	}

	hmodPDH = LoadLibraryExW(L"pdh.dll", NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if ( hmodPDH == NULL )
	{
		return;
	}

	pPdhOpenQueryW               = (pfnPdhOpenQueryW)              GetProcAddress(hmodPDH, "PdhOpenQueryW");
	pPdhAddCounterW              = (pfnPdhAddCounterW)             GetProcAddress(hmodPDH, "PdhAddCounterW");
	pPdhRemoveCounter            = (pfnPdhRemoveCounter)           GetProcAddress(hmodPDH, "PdhRemoveCounter");
	pPdhCollectQueryData         = (pfnPdhCollectQueryData)        GetProcAddress(hmodPDH, "PdhCollectQueryData");
	pPdhGetFormattedCounterValue = (pfnPdhGetFormattedCounterValue)GetProcAddress(hmodPDH, "PdhGetFormattedCounterValue");
	pPdhCloseQuery               = (pfnPdhCloseQuery)              GetProcAddress(hmodPDH, "PdhCloseQuery");

	if ( pPdhOpenQueryW               == NULL ||
		 pPdhAddCounterW              == NULL ||
		 pPdhCollectQueryData         == NULL ||
		 pPdhRemoveCounter            == NULL ||
		 pPdhGetFormattedCounterValue == NULL ||
		 pPdhCloseQuery               == NULL )
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}

	// initialize
	status = pPdhOpenQueryW(NULL, 0, &hQuery);
	if ( status != ERROR_SUCCESS )
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}

	// create cpu counter
	for ( i = 0; i < MAX_PROCESSOR; ++i )
	{
		wsprintfW(counterName, L"\\Processor(%d)\\%% Processor Time", i);

		status = pPdhAddCounterW(hQuery, counterName, 0, &hCPUCounter[i]);
		if ( status != ERROR_SUCCESS )
		{
			goto FAILURE_PDH_COUNTER_INITIALIZATION;
		}
	}

	// create total cpu usage counter
	status = pPdhAddCounterW(hQuery, L"\\Processor(_Total)\\% Processor Time", 0, &hTotalCPUCounter);
	if ( status != ERROR_SUCCESS )
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}

	return; /* SUCCESS */

FAILURE_PDH_COUNTER_INITIALIZATION:
	hQuery = NULL;

	FreeLibrary(hmodPDH);
	hmodPDH = NULL;

	return; /* FAILURE */
}

int CpuMoni_get(void)
{
	PDH_STATUS           status;
	PDH_FMT_COUNTERVALUE FmtValue;
	int i;

	if ( hQuery == NULL )
	{
		return 0;
	}

	// get current data
	status = pPdhCollectQueryData(hQuery);
	if ( status != ERROR_SUCCESS )
	{
		return 0;
	}

	// get cpu counter
	for ( i = 0; i < MAX_PROCESSOR; ++i )
	{
		status = pPdhGetFormattedCounterValue(hCPUCounter[i], PDH_FMT_DOUBLE, NULL, &FmtValue);
		if ( status != ERROR_SUCCESS )
		{
			CPUUsage[i] = 0;
		}
		else
		{
			CPUUsage[i] = (int)(FmtValue.doubleValue + 0.5);
		}
	}

	// get total cpu usage
	status = pPdhGetFormattedCounterValue(hTotalCPUCounter, PDH_FMT_DOUBLE, NULL, &FmtValue);
	if ( status != ERROR_SUCCESS )
	{
		return 0;
	}

	return (int)(FmtValue.doubleValue + 0.5);
}

void CpuMoni_end(void)
{
	// finalize
	if ( pPdhCloseQuery )
	{
		pPdhCloseQuery(hQuery);
		FreeLibrary(hmodPDH);
	}

	hQuery  = NULL;
	hmodPDH = NULL;
}

#endif	/* TC_ENABLE_CPU */
