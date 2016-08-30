/*-------------------------------------------------------------
  battery.c
---------------------------------------------------------------*/

#include "tcdll.h"

#if TC_ENABLE_BATTERY

void GetBatteryLifePercent(int *batteryLife, int *batteryMode)
{
	SYSTEM_POWER_STATUS sps;
	
	if(GetSystemPowerStatus(&sps))
	{
		*batteryLife = sps.BatteryLifePercent;
		if(sps.BatteryFlag & BATTERY_FLAG_CHARGING)
		{
			*batteryMode = 2;
		}
		else
		{
			*batteryMode = (int)sps.ACLineStatus;
		}
	}
	else
	{
		*batteryLife = 255;
		*batteryMode = 3;
	}
}

#endif	/* TC_ENABLE_BATTERY */
