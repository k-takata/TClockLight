/*-------------------------------------------------------------------------
  cpu.c
  get percentage of CPU usage
  Kazubon 2001
---------------------------------------------------------------------------*/

/* *** Special thanks to Naoki KOBAYASHI *** */

#include "tcdll.h"

#if TC_ENABLE_CPU

#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3

typedef struct
{
#if 1
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    ULONG_PTR pLowestUserAddress;
    ULONG_PTR pMmHighestUserAddress;
    ULONG_PTR uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
#else
    BYTE Reserved1[24];
    PVOID Reserved2[4];
    CCHAR bKeNumberProcessors;
#endif
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI)(UINT, PVOID, ULONG, PULONG);
static HMODULE hmodNTDLL = NULL;
static PROCNTQSI pNtQuerySystemInformation;

static int numProcessors;

void CpuMoni_start(void)
{
	SYSTEM_BASIC_INFORMATION SysBaseInfo;
	
	if(hmodNTDLL) CpuMoni_end();
	
	hmodNTDLL = GetModuleHandle("ntdll.dll");
	if(hmodNTDLL == NULL) return;
	
	pNtQuerySystemInformation =
		(PROCNTQSI)GetProcAddress(hmodNTDLL, "NtQuerySystemInformation");
	if(pNtQuerySystemInformation == NULL)
	{
		hmodNTDLL = NULL;
		return;
	}
	
	pNtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
	numProcessors = SysBaseInfo.bKeNumberProcessors;
}

int CpuMoni_get(void)
{
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
	SYSTEM_TIME_INFORMATION SysTimeInfo;
	LONGLONG llIdleTime;
	LONGLONG llSystemTime;
	static LONGLONG llOldIdleTime = 0;
	static LONGLONG llOldSystemTime;
	int iCpuUsage;
	LONG status;
	
	if(pNtQuerySystemInformation == NULL)
		return 255;
	
	// get new system time
	status = pNtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
	if(status != NO_ERROR)
		return 255;
	
	// get new CPU's idle time
	status = pNtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
	if(status != NO_ERROR)
		return 255;
	
	// if it's a first call - skip it
	iCpuUsage = 0;
	if(llOldIdleTime != 0)
	{
		// CurrentValue = NewValue - OldValue
		llIdleTime = SysPerfInfo.liIdleTime.QuadPart - llOldIdleTime;
		llSystemTime = SysTimeInfo.liKeSystemTime.QuadPart - llOldSystemTime;
		
		// CurrentCpuIdle = IdleTime / SystemTime
		// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
		iCpuUsage = (1000 - (int)(llIdleTime * 1000
					/ llSystemTime / numProcessors) + 5) / 10;
		if(iCpuUsage < 0)   iCpuUsage = 0;
		if(iCpuUsage > 100) iCpuUsage = 100;
	}
	
	// store new CPU's idle and system time
	llOldIdleTime = SysPerfInfo.liIdleTime.QuadPart;
	llOldSystemTime = SysTimeInfo.liKeSystemTime.QuadPart;
	
	return iCpuUsage;
}

void CpuMoni_end(void)
{
	hmodNTDLL = NULL;
	pNtQuerySystemInformation = NULL;
}

#endif	/* TC_ENABLE_CPU */
