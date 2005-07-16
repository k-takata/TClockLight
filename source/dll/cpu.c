/*-------------------------------------------------------------------------
  cpu.c
  get percentage of CPU usage
  Kazubon 2001
---------------------------------------------------------------------------*/

/* *** Special thanks to Naoki KOBAYASHI *** */

#include "tcdll.h"

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

#define Li2Double(x) ((double)((x).u.HighPart) * 4.294967296E9 + (double)((x).u.LowPart))

void CpuMoni_start(void);
int CpuMoni_get(void);
void CpuMoni_end(void);

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
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
			FreeLibrary(hmodNTDLL); hmodNTDLL = NULL;
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
		double dbIdleTime;
		double dbSystemTime;
		static TC_SINT64 liOldIdleTime;
		static TC_SINT64 liOldSystemTime;
		LONG status;

		if (pNtQuerySystemInformation == NULL) return -1;

		// get new system time
		status = pNtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), 0);
		if (status != NO_ERROR) return -1;

		// get new CPU's idle time
		status = pNtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
		if (status != NO_ERROR) return -1;

		// if it's a first call - skip it
		dbIdleTime = 0;
		if (liOldIdleTime.QuadPart != 0)
		{
			// CurrentValue = NewValue - OldValue
			dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
			dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

			// CurrentCpuIdle = IdleTime / SystemTime
			dbIdleTime = dbIdleTime / dbSystemTime;

			// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
			dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
		}

		// store new CPU's idle and system time
		liOldIdleTime.QuadPart = SysPerfInfo.liIdleTime.QuadPart;
		liOldSystemTime.QuadPart = SysTimeInfo.liKeSystemTime.QuadPart;

		return (int)dbIdleTime;
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
		FreeLibrary(hmodNTDLL); hmodNTDLL = NULL;
		pNtQuerySystemInformation = NULL;
	}
	else
	{
		GetRegLong(HKEY_DYN_DATA, "PerfStats\\StopStat",
			"KERNEL\\CPUUsage", 0);
	}
}
