/*-------------------------------------------------------------------------
  hdd.c
  Get the free space on a disk
---------------------------------------------------------------------------*/

#include "tcdll.h"

#if TC_ENABLE_HDD

typedef BOOL (WINAPI *pfnGetDiskFreeSpaceEx)(LPCTSTR,
		PULARGE_INTEGER, PULARGE_INTEGER,PULARGE_INTEGER);

BOOL WINAPI GetDiskFreeSpaceExStub(LPCTSTR lpDirectoryName,
		PULARGE_INTEGER lpFreeBytesAvailable,
		PULARGE_INTEGER lpTotalNumberOfBytes,
		PULARGE_INTEGER lpTotalNumberOfFreeBytes);
BOOL WINAPI GetDiskFreeSpaceEx95(LPCTSTR lpDirectoryName,
		PULARGE_INTEGER lpFreeBytesAvailable,
		PULARGE_INTEGER lpTotalNumberOfBytes,
		PULARGE_INTEGER lpTotalNumberOfFreeBytes);


void GetDiskSpace(int nDrive, ULONGLONG *all, ULONGLONG *free)
{
	char szDrive[] = "A:\\";
	ULARGE_INTEGER availableByte, allByte, freeByte;
	
	szDrive[0] += (char)nDrive;
	
	if (GetDiskFreeSpaceEx(szDrive, &availableByte, &allByte, &freeByte)) {
		*all = allByte.QuadPart;
		*free = freeByte.QuadPart;
	}else{
		*all = 0;
		*free = 0;
	}
}

#endif	/* TC_ENABLE_HDD */
