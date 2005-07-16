/*-------------------------------------------------------------
  battery.c
---------------------------------------------------------------*/

#include "tcdll.h"

int GetBatteryLifePercent(void)
{
	if (g_winver&(WIN95|WIN2000))
	{
		SYSTEM_POWER_STATUS sps;
		if (GetSystemPowerStatus(&sps))
			return sps.BatteryLifePercent;
	}
	return 255;
}

