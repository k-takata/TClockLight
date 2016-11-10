/*-------------------------------------------------------------------------
  cpu.c
  get percentage of CPU usage
  Kazubon 2001
---------------------------------------------------------------------------*/

#include "tcdll.h"

#if TC_ENABLE_CPU

void CpuMoni_start(void)
{
}

int CpuMoni_get(void)
{
	return -1;
}

void CpuMoni_end(void)
{
}

#endif	/* TC_ENABLE_CPU */
