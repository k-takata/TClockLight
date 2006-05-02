/*-------------------------------------------------------------------------
  hdd.c
  Get the free space on a disk
---------------------------------------------------------------------------*/

#include "tcdll.h"

void GetDiskSpace(int nDrive, double *all, double *free)
{
	char szDrive[] = "A:\\";
	ULARGE_INTEGER useByte, allByte, freeByte;
	
	szDrive[0] += (char)nDrive;
	
	if ( GetDiskFreeSpaceEx(szDrive, &useByte, &allByte, &freeByte) ){
		*all = allByte.QuadPart / 1048576.0;
		*free = freeByte.QuadPart / 1048576.0;
	}else{
		*all = 0.0;
		*free = 0.0;
	}
}
