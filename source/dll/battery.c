/*-------------------------------------------------------------
  battery.c
---------------------------------------------------------------*/

#include "tcdll.h"

#if TC_ENABLE_BATTERY

typedef BOOL (WINAPI *pfnGetSystemPowerStatus)(LPSYSTEM_POWER_STATUS);

BOOL WINAPI GetSystemPowerStatusStub(LPSYSTEM_POWER_STATUS lpStatus);
BOOL WINAPI GetSystemPowerStatusNT4(LPSYSTEM_POWER_STATUS lpStatus);

#if !TC_SUPPORT_NT4
static pfnGetSystemPowerStatus pGetSystemPowerStatus = GetSystemPowerStatus;
#else
static pfnGetSystemPowerStatus pGetSystemPowerStatus = GetSystemPowerStatusStub;
#endif

BOOL WINAPI GetSystemPowerStatusStub(LPSYSTEM_POWER_STATUS lpStatus)
{
	HMODULE hKernel32 = GetModuleHandle("kernel32.dll");
	pGetSystemPowerStatus = (pfnGetSystemPowerStatus)
			GetProcAddress(hKernel32, "GetSystemPowerStatus");
	
	if (pGetSystemPowerStatus == NULL) {
		pGetSystemPowerStatus = GetSystemPowerStatusNT4;
	}
	return pGetSystemPowerStatus(lpStatus);
}

// NT4
BOOL WINAPI GetSystemPowerStatusNT4(LPSYSTEM_POWER_STATUS lpStatus)
{
	return FALSE;
}


void GetBatteryLifePercent(int *batteryLife, int *batteryMode)
{
	SYSTEM_POWER_STATUS sps;
	
	if (pGetSystemPowerStatus(&sps)) {
		*batteryLife = sps.BatteryLifePercent;
		if (sps.BatteryFlag & BATTERY_FLAG_CHARGING) {
			*batteryMode = 2;
		} else {
			*batteryMode = (int)sps.ACLineStatus;
		}
	} else {
		*batteryLife = 255;
		*batteryMode = 3;
	}
}

#endif	/* TC_ENABLE_BATTERY */
