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

#if !TC_SUPPORT_WIN95
static pfnGetDiskFreeSpaceEx pGetDiskFreeSpaceEx = GetDiskFreeSpaceEx;
#else
static pfnGetDiskFreeSpaceEx pGetDiskFreeSpaceEx = GetDiskFreeSpaceExStub;
#endif

BOOL WINAPI GetDiskFreeSpaceExStub(LPCTSTR lpDirectoryName,
		PULARGE_INTEGER lpFreeBytesAvailable,
		PULARGE_INTEGER lpTotalNumberOfBytes,
		PULARGE_INTEGER lpTotalNumberOfFreeBytes)
{
	HMODULE hKernel32 = GetModuleHandle("kernel32.dll");
	pGetDiskFreeSpaceEx = (pfnGetDiskFreeSpaceEx)
			GetProcAddress(hKernel32, "GetDiskFreeSpaceExA");
	
	if (pGetDiskFreeSpaceEx == NULL) {
		pGetDiskFreeSpaceEx = GetDiskFreeSpaceEx95;
	}
	return pGetDiskFreeSpaceEx(lpDirectoryName, lpFreeBytesAvailable,
			lpTotalNumberOfBytes, lpTotalNumberOfFreeBytes);
}

BOOL WINAPI GetDiskFreeSpaceEx95(LPCTSTR lpDirectoryName,
		PULARGE_INTEGER lpFreeBytesAvailable,
		PULARGE_INTEGER lpTotalNumberOfBytes,
		PULARGE_INTEGER lpTotalNumberOfFreeBytes)
{
	DWORD spc, bps, numfree, numtotal;
	ULONGLONG freeBytes;
	
	if (!GetDiskFreeSpace(lpDirectoryName, &spc, &bps, &numfree, &numtotal)) {
		return FALSE;
	}
	freeBytes = (ULONGLONG) spc * bps * numfree;
	if (lpFreeBytesAvailable) {
		lpFreeBytesAvailable->QuadPart = freeBytes;
	}
	if (lpTotalNumberOfBytes) {
		lpTotalNumberOfBytes->QuadPart = (ULONGLONG) spc * bps * numtotal;
	}
	if (lpTotalNumberOfFreeBytes) {
		lpTotalNumberOfFreeBytes->QuadPart = freeBytes;
	}
	return TRUE;
}


void GetDiskSpace(int nDrive, ULONGLONG *all, ULONGLONG *free)
{
	char szDrive[] = "A:\\";
	ULARGE_INTEGER availableByte, allByte, freeByte;
	
	szDrive[0] += (char)nDrive;
	
	if (pGetDiskFreeSpaceEx(szDrive, &availableByte, &allByte, &freeByte)) {
		*all = allByte.QuadPart;
		*free = freeByte.QuadPart;
	}else{
		*all = 0;
		*free = 0;
	}
}

#endif	/* TC_ENABLE_HDD */
