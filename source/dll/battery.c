/*-------------------------------------------------------------
  battery.c
---------------------------------------------------------------*/

#include "tcdll.h"

void GetBatteryLifePercent(int *batteryLife, int *batteryMode )
{
	if (g_winver&(WIN95|WIN2000))
	{
		SYSTEM_POWER_STATUS sps;
		if (GetSystemPowerStatus(&sps))
		{
			*batteryLife = sps.BatteryLifePercent;
			if ( sps.BatteryFlag&8 )
				*batteryMode = 2;
			else
				*batteryMode = (int)sps.ACLineStatus;
		}
		else
		{
			*batteryLife = 255;
			*batteryMode = 3;
		}
	}
	else
	{
		*batteryLife = 255;
		*batteryMode = 3;
	}
}

