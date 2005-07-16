/*-------------------------------------------------------------------------
  hdd.c
  Get the free space on a disk
---------------------------------------------------------------------------*/

#include "tcdll.h"

void GetDiskSpace(int nDrive, double *all, double *free)
{
	char szDrive[5];
	ULARGE_INTEGER useByte, allByte, freeByte;

	wsprintf(szDrive, "%c:\\", nDrive + 'A');
	GetDiskFreeSpaceEx(szDrive, &useByte, &allByte, &freeByte);

	*all = allByte.QuadPart / 1048576.0;
	*free = freeByte.QuadPart / 1048576.0;
}

