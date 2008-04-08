/*-------------------------------------------------------------
	sysinfo.c

	$Id: sysinfo.c,v e4eaa4f77596 2008/04/08 15:44:21 slic $
---------------------------------------------------------------*/

#include "tcdll.h"

/* Statics */
static BOOL GetNumFormat(const wchar_t **sp, wchar_t x, wchar_t c,
	int *len, int *slen, BOOL *bComma);
static void SetNumFormat(wchar_t **dp, unsigned n,
	int len, int slen, BOOL bComma);

static BOOL m_bBattery,m_bMem, m_bVolume;	// m_bCPU, m_bNet, m_bHDD;
static MEMORYSTATUS ms_98;
static MEMORYSTATUSEX ms_nt;
/*
static double net[4];
static BOOL actdvl[26];
static double diskAll[26];
static double diskFree[26];
static int iCPUUsage;
*/
static int iBatteryLife;
static int iBatteryMode;
static int iVolume;
static int m_sec;
static BOOL bMuteFlg;

void InitSysInfo(HWND hwnd)
{
	// Note:
	// Because I used it to be able to hold it for desktop.c and mixer.c 
	// I enlarged update space.
	m_sec = GetMyRegLong(NULL, "IntervalSysInfo", 8);	// default 4
	if (m_sec < 1 || 60 < m_sec) m_sec = 8;			// default 1<n<60
	SetTimer(hwnd, IDTIMER_SYSINFO, m_sec * 1000, NULL);
}

void EndSysInfo(HWND hwnd)
{
	KillTimer(hwnd, IDTIMER_SYSINFO);
/*
	if (m_bNet) {
		Net_end(); // net.c
	}
	if (m_bHDD) {
		int i;
		for (i = 0; i < 26; i++)
			actdvl[i] = FALSE;
	}
	if (m_bCPU) {
		CpuMoni_end(); // cpu.c
	}

	m_bNet = m_bMem = m_bHDD = m_bCPU = m_bBattery = m_bVolume = FALSE;
*/
	m_bBattery = m_bMem = m_bVolume = FALSE;
}

void GetMemoryStatus(void)
{
		if ((g_winver&WINNT)) {
			ms_nt.dwLength = sizeof(ms_nt);
			GlobalMemoryStatusEx(&ms_nt);
		} else {
			GlobalMemoryStatus(&ms_98);
		}
}

void OnTimerSysInfo(void)
{
	int vol;
	BOOL bMute;
/*	
	if (m_bNet) {
		double recv, send;
		Net_get(&recv, &send); // net.c
		net[2] = (recv - net[0]) / m_sec;
		net[3] = (send - net[1]) / m_sec;
		net[0] = recv;
		net[1] = send;
	}
*/
	if (m_bMem) {
		GetMemoryStatus();
	}
/*
	if (m_bHDD) {
		int i;
		for (i = 0; i < 26; i++) {
			if (actdvl[i])
				GetDiskSpace(i, &diskAll[i], &diskFree[i]); // hdd.c
		}
	}
	if (m_bCPU) {
		iCPUUsage = CpuMoni_get(); // cpu.c
	}
*/
	if (m_bBattery) {
		GetBatteryLifePercent(&iBatteryLife, &iBatteryMode); // battery.c
	}

	if (m_bVolume) {
		GetMasterVolume(&vol); // mixer.c
		GetMasterMute(&bMute);
		bMuteFlg = bMute;
		iVolume = vol;
	}
}

/*
void ElapsedTimeHandler(FORMATHANDLERSTRUCT* pstruc)
{
	static int sec = -1;
	static DWORD t;
	unsigned st;
	int len, slen;
	BOOL bComma;

	if (pstruc->pt->wSecond != sec) {
		sec = pstruc->pt->wSecond;
		t = GetTickCount();
	}

	pstruc->sp++;
	if (*pstruc->sp == 'T') {
		pstruc->sp++;
		SetNumFormat(&pstruc->dp, t / 3600000, 1, 0, FALSE);
		*pstruc->dp++ = ':';
		SetNumFormat(&pstruc->dp, t / 60000 % 60, 2, 0, FALSE);
		*pstruc->dp++ = ':';
		SetNumFormat(&pstruc->dp, t / 1000 % 60, 2, 0, FALSE);
		g_bDispSecond = TRUE;
		return;
	} else if (GetNumFormat(&pstruc->sp, 'd', 'd', &len, &slen, &bComma)) {
		st = t / 86400000;
	} else if (GetNumFormat(&pstruc->sp, 'a', 'a', &len, &slen, &bComma)) {
		st = t / 3600000;
	} else if (GetNumFormat(&pstruc->sp, 'h', 'h', &len, &slen, &bComma)) {
		st = t / 3600000 % 24;
	} else if (GetNumFormat(&pstruc->sp, 'n', 'n', &len, &slen, &bComma)) {
		st = t / 60000 % 60;
	} else if (GetNumFormat(&pstruc->sp, 's', 's', &len, &slen, &bComma)) {
		st = t / 1000 % 60;
	} else {
		*pstruc->dp++ = 'S';
		return;
	}
	SetNumFormat(&pstruc->dp, st, len, slen, FALSE);

	g_bDispSecond = TRUE;
}

void NetworkHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int i;

	if (*(pstruc->sp + 1) == 'R') i = 0;
	else if (*(pstruc->sp + 1) == 'S') i = 1;
	else i = -4;

	if (*(pstruc->sp + 2) == 'A');
	else if (*(pstruc->sp + 2) == 'S') i += 2;
	else i = -4;

	if (!(*(pstruc->sp+3)=='B' || *(pstruc->sp+3)=='K' || *(pstruc->sp+3)=='M'))
		i = -4;

	if (i >= 0) {
		double ntd;
		unsigned nt;
		int len, slen;
		BOOL bComma;

		if (!m_bNet) {
			m_bNet = TRUE;
			Net_start(); // net.c
			Net_get(&net[0], &net[1]);
			net[2] = 0; net[3] = 0;
			g_bDispSecond = TRUE;
		}

		ntd = net[i];
		if (*(pstruc->sp + 3) == 'K') ntd /= 1024;
		else if (*(pstruc->sp + 3) == 'M') ntd /= 1048576;
		nt = ntd;

		pstruc->sp += 4;
		GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
		SetNumFormat(&pstruc->dp, nt, len, slen, bComma);
		if (*pstruc->sp == '.') {
			pstruc->sp++;
			if (GetNumFormat(&pstruc->sp, 'x', 'x', &len, &slen, &bComma)) {
				*pstruc->dp++ = '.';
				if (len > 3) len = 3;
				ntd -= nt;
				for (i = 0; i < len; i++) ntd *= 10;
				nt = ntd;
				SetNumFormat(&pstruc->dp, nt, len, 0, FALSE);
			}
		}
	} else
		*pstruc->dp++ = *pstruc->sp++;
}
*/
void MemoryHandler(FORMATHANDLERSTRUCT* pstruc)
{
	DWORDLONG m, t;
	BOOL bValid = TRUE;
	BOOL bNT = (g_winver&WINNT);

	if (!m_bMem) {
		if (*(pstruc->sp + 1) == 'K' || *(pstruc->sp + 1) == 'M' || (
			(*(pstruc->sp+1)=='T'||*(pstruc->sp+1)=='A'||*(pstruc->sp+1)=='U')&&
			(*(pstruc->sp+2)=='P'||*(pstruc->sp+2)=='F'||*(pstruc->sp+2)=='V')&&
			(*(pstruc->sp+3)=='K'||*(pstruc->sp+3)=='M'||*(pstruc->sp+3)=='P')))
		{
			m_bMem = TRUE;
			GetMemoryStatus();		//GlobalMemoryStatus(&ms);
			g_bDispSecond = TRUE;
		} else {
			*pstruc->dp++ = *pstruc->sp++;
			return;
		}
	}

	if (*(pstruc->sp + 1) == 'K')
	{
		m = (bNT)? ms_nt.ullAvailPhys : ms_98.dwAvailPhys;
		m >>= 10;
		pstruc->sp -= 2;
	}
	else if (*(pstruc->sp + 1) == 'M')
	{
		m = (bNT)? ms_nt.ullAvailPhys : ms_98.dwAvailPhys;
		m >>= 20;
		pstruc->sp -= 2;
	}
	else if (*(pstruc->sp + 1) == 'T')
	{
		if (*(pstruc->sp + 2) == 'P')
			m = (bNT)? ms_nt.ullTotalPhys : ms_98.dwTotalPhys;
		else if (*(pstruc->sp + 2) == 'F')
			m = (bNT)? ms_nt.ullTotalPageFile :ms_98.dwTotalPageFile;
		else if (*(pstruc->sp + 2) == 'V')
			m = (bNT)? ms_nt.ullTotalVirtual :ms_98.dwTotalVirtual;
		else
			bValid = FALSE;
		if (bValid)
		{
			if (*(pstruc->sp + 3) == 'K')
				m >>= 10;
			else if (*(pstruc->sp + 3) == 'M')
				m >>= 20;
			else
				bValid = FALSE;
		}
	}
	else if (*(pstruc->sp + 1) == 'A')
	{
		if (*(pstruc->sp + 2) == 'P')
		{
			if (bNT)
			{
				m = ms_nt.ullAvailPhys;
				t = ms_nt.ullTotalPhys;
			} else {
				m = ms_98.dwAvailPhys;
				t = ms_98.dwTotalPhys;
			}
		}
		else if (*(pstruc->sp + 2) == 'F')
		{
			if (bNT)
			{
				m = ms_nt.ullAvailPageFile;
				t = ms_nt.ullTotalPageFile;
			} else {
				m = ms_98.dwAvailPageFile;
				t = ms_98.dwTotalPageFile;
			}
		}
		else if (*(pstruc->sp + 2) == 'V')
		{
			if (bNT)
			{
				m = ms_nt.ullAvailVirtual;
				t = ms_nt.ullTotalVirtual;
			} else {
				m = ms_98.dwAvailVirtual;
				t = ms_98.dwTotalVirtual;
			}
		}
		else
			bValid = FALSE;
		if (bValid)
		{
			if (*(pstruc->sp + 3) == 'K')
				m >>= 10;
			else if (*(pstruc->sp + 3) == 'M')
				m >>= 20;
			else if (*(pstruc->sp + 3) == 'P')
				m = MulDiv((int)m, 100, (int)t);
			else
				bValid = FALSE;
		}
	}
	else if (*(pstruc->sp + 1) == 'U')
	{
		if (*(pstruc->sp + 2) == 'P')
		{
			if (bNT)
			{
				m = ms_nt.ullTotalPhys - ms_nt.ullAvailPhys;
				t = ms_nt.ullTotalPhys;
			} else {
				m = ms_98.dwTotalPhys - ms_98.dwAvailPhys;
				t = ms_98.dwTotalPhys;
			}
		}
		else if (*(pstruc->sp + 2) == 'F')
		{
			if (bNT)
			{
				m = ms_nt.ullTotalPageFile - ms_nt.ullAvailPageFile;
				t = ms_nt.ullTotalPageFile;
			} else {
				m = ms_98.dwTotalPageFile - ms_98.dwAvailPageFile;
				t = ms_98.dwTotalPageFile;
			}
		}
		else if (*(pstruc->sp + 2) == 'V')
		{
			if (bNT)
			{
				m = ms_nt.ullTotalVirtual - ms_nt.ullAvailVirtual;
				t = ms_nt.ullTotalVirtual;
			} else {
				m = ms_98.dwTotalVirtual - ms_98.dwAvailVirtual;
				t = ms_98.dwTotalVirtual;
			}
		}
		else
			bValid = FALSE;
		if (bValid)
		{
			if (*(pstruc->sp + 3) == 'K')
				m >>= 10;
			else if (*(pstruc->sp + 3) == 'M')
				m >>= 20;
			else if (*(pstruc->sp + 3) == 'P') {
				m = MulDiv((int) m, 100,(int) t);
			} else {
				bValid = FALSE;
			}
		}
	}
	else
		bValid = FALSE;

	if (bValid) {
		int len, slen;
		BOOL bComma;
		pstruc->sp += 4;
		GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
		SetNumFormat(&pstruc->dp, (unsigned)(int) m , len, slen, bComma);
	} else
		*pstruc->dp++ = *pstruc->sp++;
}

/*
void HDDHandler(FORMATHANDLERSTRUCT* pstruc)
{
	double d = -1.0;

	if (!m_bHDD) {
		if ((*(pstruc->sp+1)=='T'||*(pstruc->sp+1)=='A'||*(pstruc->sp+1)=='U')&&
			(*(pstruc->sp+3)=='M'||*(pstruc->sp+3)=='G'||*(pstruc->sp+3)=='P')&&
			(*(pstruc->sp + 2) >= 'A' && *(pstruc->sp + 2) <= 'Z'))
		{
			m_bHDD = TRUE;
			g_bDispSecond = TRUE;
		} else {
			*pstruc->dp++ = *pstruc->sp++;
			return;
		}
	}

	if ((*(pstruc->sp + 2) >= 'A') && (*(pstruc->sp + 2) <= 'Z')) {
		int drv = *(pstruc->sp + 2) - 'A';

		if (!actdvl[drv]) {
			actdvl[drv] = TRUE;
			GetDiskSpace(drv, &diskAll[drv], &diskFree[drv]); // hdd.c
		}

		if (*(pstruc->sp + 1) == 'T')
		{
			if (*(pstruc->sp + 3) == 'M')
				d = diskAll[drv];
			else if (*(pstruc->sp + 3) == 'G')
				d = diskAll[drv] / 1024;
		}
		else if (*(pstruc->sp + 1) == 'A')
		{
			if (*(pstruc->sp + 3) == 'M')
				d = diskFree[drv];
			else if (*(pstruc->sp + 3) == 'G')
				d = diskFree[drv] / 1024;
			else if (*(pstruc->sp + 3) == 'P')
				d = diskAll[drv] ? diskFree[drv] * 100 / diskAll[drv] : 0.0;
		}
		else if (*(pstruc->sp + 1) == 'U')
		{
			if (*(pstruc->sp + 3) == 'M')
				d = diskAll[drv] - diskFree[drv];
			else if (*(pstruc->sp + 3) == 'G')
				d = (diskAll[drv] - diskFree[drv]) / 1024;
			else if (*(pstruc->sp + 3) == 'P')
				d = diskAll[drv] ?
					(diskAll[drv] - diskFree[drv]) * 100 / diskAll[drv] : 0.0;
		}
	}

	if (d >= 0.0) {
		unsigned n;
		int len, slen;
		BOOL bComma;

		n = d;
		pstruc->sp += 4;
		GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
		SetNumFormat(&pstruc->dp, n, len, slen, bComma);
		if (*pstruc->sp == '.') {
			pstruc->sp++;
			if (GetNumFormat(&pstruc->sp, 'x', 'x', &len, &slen, &bComma)) {
				int i;
				*pstruc->dp++ = '.';
				if (len > 3) len = 3;
				d -= n;
				for (i = 0; i < len; i++) d *= 10;
				n = d;
				SetNumFormat(&pstruc->dp, n, len, 0, FALSE);
			}
		}
	} else
		*pstruc->dp++ = *pstruc->sp++;
}

void CPUHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int len, slen;
	BOOL bComma;

	if(!m_bCPU)
	{
		m_bCPU = TRUE;
		CpuMoni_start(); // cpu.c
		iCPUUsage = CpuMoni_get(); // cpu.c
		g_bDispSecond = TRUE;
	}

	pstruc->sp += 2;
	GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
	SetNumFormat(&pstruc->dp, iCPUUsage, len, slen, bComma);
}
*/
void BatteryHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int len, slen;
	BOOL bComma;

	if(!m_bBattery)
	{
		m_bBattery = TRUE;
		GetBatteryLifePercent(&iBatteryLife, &iBatteryMode); // battery.c
		g_bDispSecond = TRUE;
	}

	pstruc->sp += 2;
	GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
	SetNumFormat(&pstruc->dp, iBatteryLife, len, slen, bComma);
}

void ACStatusHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int sl;
	if(!m_bBattery)
	{
		m_bBattery = TRUE;
		GetBatteryLifePercent(&iBatteryLife, &iBatteryMode); // battery.c
		g_bDispSecond = TRUE;
	}
	
	pstruc->sp += 2;
	if ( *pstruc->sp++ != '(' )
		return;
	sl = iBatteryMode;
	
	while ( sl > 0 )
	{
		while ( *pstruc->sp != '|' )
		{
			pstruc->sp++;
			if ( *pstruc->sp == '\0' )
				return;
		}
		pstruc->sp++;
		sl--;
	}
	
	while ( *pstruc->sp != '|' && *pstruc->sp != ')' && *pstruc->sp != '\0' )
	{
		*pstruc->dp++ = *pstruc->sp++;
	}
	
	while ( *pstruc->sp != ')' && *pstruc->sp != '\0' )
	{
		pstruc->sp++;
	}
	pstruc->sp++;
}


void VolumeMuteHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int len, slen, vol;
	BOOL bComma, bMute;

	if(!m_bVolume)
	{
		m_bVolume = TRUE;
		
		GetMasterVolume(&vol); // mixer.c
		GetMasterMute(&bMute);
		iVolume = vol;
		bMuteFlg = bMute;
		g_bDispSecond = TRUE;
	}
	vol = iVolume;
	if ( bMuteFlg )
		vol = 0;
	pstruc->sp += 2;
	GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
	SetNumFormat(&pstruc->dp, vol, len, slen, bComma);
}

void VolumeHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int len, slen, vol;
	BOOL bComma, bMute;

	if(!m_bVolume)
	{
		m_bVolume = TRUE;
		
		GetMasterVolume(&vol); // mixer.c
		GetMasterMute(&bMute);
		iVolume = vol;
		bMuteFlg = bMute;
		g_bDispSecond = TRUE;
	}

	pstruc->sp += 3;
	GetNumFormat(&pstruc->sp, 'x', ',', &len, &slen, &bComma);
	SetNumFormat(&pstruc->dp, iVolume, len, slen, bComma);
}

void MuteHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int vol;
    int sl = 0;
	BOOL bMute;
	
	if(!m_bVolume)
	{
		m_bVolume = TRUE;
		
		GetMasterVolume(&vol); // mixer.c
		GetMasterMute(&bMute);
		iVolume = vol;
		bMuteFlg = bMute;
		g_bDispSecond = TRUE;
	}
	
	pstruc->sp += 3;
	if ( *pstruc->sp++ != '(' )
		return;
	if ( bMuteFlg )
		sl = 1;
	
	while ( sl > 0 )
	{
		while ( *pstruc->sp != '|' )
		{
			pstruc->sp++;
			if ( *pstruc->sp == '\0' )
				return;
		}
		pstruc->sp++;
		sl--;
	}
	
	while ( *pstruc->sp != '|' && *pstruc->sp != ')' && *pstruc->sp != '\0' )
	{
		*pstruc->dp++ = *pstruc->sp++;
	}
	
	while ( *pstruc->sp != ')' && *pstruc->sp != '\0' )
	{
		pstruc->sp++;
	}
	pstruc->sp++;
}

void RefreshVolume(void)
{
	int vol;
	BOOL bMute;
	
	if(m_bVolume)
	{
		GetMasterVolume(&vol); // mixer.c
		GetMasterMute(&bMute);
		iVolume = vol;
		bMuteFlg = bMute;
	}
}

BOOL GetNumFormat(const wchar_t **sp, wchar_t x, wchar_t c,
	int *len, int *slen, BOOL *bComma)
{
	const wchar_t *p = *sp;
	int n = 0, ns = 0;

	*bComma = FALSE;

	while (*p == '_')
	{
		ns++;
		p++;
	}
	while (*p == x)
	{
		n++;
		p++;
	}
	while (*p == c)
	{
		n++;
		p++;
		*bComma = TRUE;
	}

	if (n > 0) {
		*len = n+ns;
		*slen = ns;
		*sp = p;
		return TRUE;
	} else {
		*len = 1;
		*slen = 0;
		return FALSE;
	}
}

void SetNumFormat(wchar_t **dp, unsigned n, int len, int slen, BOOL bComma)
{
	unsigned u;
	int minlen,i;
	wchar_t *p = *dp;

	for (u=n, minlen=1; u>=10; u/=10, minlen++);
	if (bComma) minlen += (minlen-1) / 3;

	while (minlen < len)
	{
		if (slen > 0) { *p++ = ' '; slen--; }
		else { *p++ = '0'; }
		len--;
	}
	for (i=minlen-1, u=1; i>=0; i--, u++)
	{
		*(p+i) = (wchar_t)((n%10)+'0');
		if (bComma && u%3 == 0 && i != 0)
			*(p+ --i) = ',';
		n/=10;
	}

	*dp = p + minlen;
}

