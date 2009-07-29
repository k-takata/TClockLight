/*-------------------------------------------------------------------------
  cpu.c
  get percentage of CPU usage
  Kazubon 2001
---------------------------------------------------------------------------*/

/* *** Special thanks to Naoki KOBAYASHI *** */

#include "tcdll.h"

#if TC_ENABLE_CPU

#ifdef  _MSC_VER
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

static SYSTEM_BASIC_INFORMATION SysBaseInfo;

void CpuMoni_start(void)
{
	if(g_winver&WINNT) // WinNT4, 2000, XP
	{
		if(hmodNTDLL) CpuMoni_end();
		
		hmodNTDLL = GetModuleHandle("ntdll.dll");
		if(hmodNTDLL == NULL) return;
		
		pNtQuerySystemInformation =
			(PROCNTQSI)GetProcAddress(hmodNTDLL,
				"NtQuerySystemInformation");
		if (pNtQuerySystemInformation == NULL)
		{
			hmodNTDLL = NULL;
			return;
		}

		pNtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
	}
	else // Win95,98,Me
	{
		GetRegLong(HKEY_DYN_DATA, "PerfStats\\StartStat",
			"KERNEL\\CPUUsage", 0);
	}
}

int CpuMoni_get(void)
{
	if(g_winver&WINNT)
	{
		SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
		SYSTEM_TIME_INFORMATION SysTimeInfo;
		LONGLONG llIdleTime;
		LONGLONG llSystemTime;
		static TC_SINT64 liOldIdleTime;
		static TC_SINT64 liOldSystemTime;
		int iCpuUsage;
		LONG status;

		if (pNtQuerySystemInformation == NULL)
			return 255;

		// get new system time
		status = pNtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
		if (status != NO_ERROR)
			return 255;

		// get new CPU's idle time
		status = pNtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
		if (status != NO_ERROR)
			return 255;

		// if it's a first call - skip it
		iCpuUsage = 0;
		if (liOldIdleTime.QuadPart != 0)
		{
			// CurrentValue = NewValue - OldValue
			llIdleTime = SysPerfInfo.liIdleTime.QuadPart - liOldIdleTime.QuadPart;
			llSystemTime = SysTimeInfo.liKeSystemTime.QuadPart - liOldSystemTime.QuadPart;

			// CurrentCpuIdle = IdleTime / SystemTime
			// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
			iCpuUsage = (1000 - (int)(llIdleTime * 1000 / llSystemTime / SysBaseInfo.bKeNumberProcessors) + 5) / 10;
			if (iCpuUsage < 0)   iCpuUsage = 0;
			if (iCpuUsage > 100) iCpuUsage = 100;
		}

		// store new CPU's idle and system time
		liOldIdleTime.QuadPart = SysPerfInfo.liIdleTime.QuadPart;
		liOldSystemTime.QuadPart = SysTimeInfo.liKeSystemTime.QuadPart;

		return iCpuUsage;
	}
	else
	{
		return GetRegLong(HKEY_DYN_DATA, "PerfStats\\StatData",
			"KERNEL\\CPUUsage", 0);
	}
}

void CpuMoni_end(void)
{
	if(g_winver&WINNT)
	{
		hmodNTDLL = NULL;
		pNtQuerySystemInformation = NULL;
	}
	else
	{
		GetRegLong(HKEY_DYN_DATA, "PerfStats\\StopStat",
			"KERNEL\\CPUUsage", 0);
	}
}

#endif	/* TC_ENABLE_CPU */
