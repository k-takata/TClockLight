/*-------------------------------------------------------------
	sysinfo.c

	$Id: sysinfo.c,v e4eaa4f77596 2008/04/08 15:44:21 slic $
---------------------------------------------------------------*/

#include "tcdll.h"

#if TC_ENABLE_SYSINFO

/* Statics */
static BOOL GetNumFormat(const wchar_t **sp, wchar_t x, wchar_t c,
	int *len, int *slen, BOOL *bComma);
static void SetNumFormat(wchar_t **dp, unsigned n,
	int len, int slen, BOOL bComma);
static void FormatNum(const wchar_t **sp, wchar_t **dp, unsigned n);
static void FormatFixedPointNum(const wchar_t **sp, wchar_t **dp,
	ULONGLONG d, int multiplier);

#if TC_ENABLE_BATTERY
static BOOL m_bBattery;
static int iBatteryLife;
static int iBatteryMode;
#endif

#if TC_ENABLE_MEMORY
static BOOL m_bMem;
static MEMORYSTATUSEX ms;
#endif	/* TC_ENABLE_MEMORY */

#if TC_ENABLE_VOLUME
static BOOL m_bVolume;
static int iVolume;
static BOOL bMuteFlg;
#endif

#if TC_ENABLE_CPU
static BOOL m_bCPU;
static int iCPUUsage;
#endif

#if TC_ENABLE_NETWORK
static BOOL m_bNet;
static ULONGLONG net[4];
#endif

#if TC_ENABLE_HDD
static BOOL m_bHDD;
static BOOL actdvl[26];
static ULONGLONG diskAll[26];
static ULONGLONG diskFree[26];
#endif

static int m_sec;


void InitSysInfo(HWND hwnd)
{
	m_sec = GetMyRegLong(NULL, "IntervalSysInfo", TC_DEFAULT_INTERVALSYSINFO);	// default 4
	if (m_sec < 1 || 60 < m_sec) m_sec = TC_DEFAULT_INTERVALSYSINFO;			// default 1<=n<=60
	SetTimer(hwnd, IDTIMER_SYSINFO, m_sec * 1000, NULL);
}

void EndSysInfo(HWND hwnd)
{
	KillTimer(hwnd, IDTIMER_SYSINFO);

#if TC_ENABLE_NETWORK
	if (m_bNet) {
		Net_end(); // net.c
	}
	m_bNet = FALSE;
#endif
#if TC_ENABLE_HDD
	if (m_bHDD) {
		int i;
		for (i = 0; i < 26; i++)
			actdvl[i] = FALSE;
	}
	m_bHDD = FALSE;
#endif
#if TC_ENABLE_CPU
	if (m_bCPU) {
		CpuMoni_end(); // cpu.c
	}
	m_bCPU = FALSE;
#endif
#if TC_ENABLE_BATTERY
	m_bBattery = FALSE;
#endif
#if TC_ENABLE_MEMORY
	m_bMem = FALSE;
#endif
#if TC_ENABLE_VOLUME
	m_bVolume = FALSE;
#endif
}

void OnTimerSysInfo(void)
{
#if TC_ENABLE_NETWORK
	if (m_bNet) {
		ULONGLONG recv, send;
		Net_get(&recv, &send); // net.c
		net[2] = (recv - net[0]) / m_sec;
		net[3] = (send - net[1]) / m_sec;
		net[0] = recv;
		net[1] = send;
	}
#endif
#if TC_ENABLE_MEMORY
	if (m_bMem) {
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatusEx(&ms);
	}
#endif
#if TC_ENABLE_HDD
	if (m_bHDD) {
		int i;
		for (i = 0; i < 26; i++) {
			if (actdvl[i])
				GetDiskSpace(i, &diskAll[i], &diskFree[i]); // hdd.c
		}
	}
#endif
#if TC_ENABLE_CPU
	if (m_bCPU) {
		iCPUUsage = CpuMoni_get(); // cpu.c
	}
#endif
#if TC_ENABLE_BATTERY
	if (m_bBattery) {
		GetBatteryLifePercent(&iBatteryLife, &iBatteryMode); // battery.c
	}
#endif
#if TC_ENABLE_VOLUME
	if (m_bVolume) {
		int vol;
		BOOL bMute;
		GetMasterVolume(&vol); // mixer.c
		GetMasterMute(&bMute);
		bMuteFlg = bMute;
		iVolume = vol;
	}
#endif
}

#if TC_ENABLE_ETIME
void ElapsedTimeHandler(FORMATHANDLERSTRUCT* pstruc)
{
	static int sec = -1;
	static ULONGLONG t;
	unsigned st;
	int len, slen;
	BOOL bComma;

	if (pstruc->pt->wSecond != sec) {
		sec = pstruc->pt->wSecond;
		t = GetTickCount64();
	}

	pstruc->sp++;
	if (*pstruc->sp == 'T') {
		pstruc->sp++;
		SetNumFormat(&pstruc->dp, (unsigned)(t / 3600000), 1, 0, FALSE);
		*pstruc->dp++ = ':';
		SetNumFormat(&pstruc->dp, (unsigned)(t / 60000 % 60), 2, 0, FALSE);
		*pstruc->dp++ = ':';
		SetNumFormat(&pstruc->dp, (unsigned)(t / 1000 % 60), 2, 0, FALSE);
		g_bDispSecond = TRUE;
		return;
	} else if (GetNumFormat(&pstruc->sp, 'd', 'd', &len, &slen, &bComma)) {
		st = (unsigned)(t / 86400000);
	} else if (GetNumFormat(&pstruc->sp, 'a', 'a', &len, &slen, &bComma)) {
		st = (unsigned)(t / 3600000);
	} else if (GetNumFormat(&pstruc->sp, 'h', 'h', &len, &slen, &bComma)) {
		st = (unsigned)(t / 3600000 % 24);
	} else if (GetNumFormat(&pstruc->sp, 'n', 'n', &len, &slen, &bComma)) {
		st = (unsigned)(t / 60000 % 60);
	} else if (GetNumFormat(&pstruc->sp, 's', 's', &len, &slen, &bComma)) {
		st = (unsigned)(t / 1000 % 60);
	} else {
		*pstruc->dp++ = 'S';
		return;
	}
	SetNumFormat(&pstruc->dp, st, len, slen, FALSE);

	g_bDispSecond = TRUE;
}
#endif

#if TC_ENABLE_NETWORK
void NetworkHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int i;

	if (*(pstruc->sp + 1) == 'R') i = 0;
	else if (*(pstruc->sp + 1) == 'S') i = 1;
	else i = -4;

	if (*(pstruc->sp + 2) == 'A');
	else if (*(pstruc->sp + 2) == 'S') i += 2;
	else i = -4;

	if (!(*(pstruc->sp+3)=='B' || *(pstruc->sp+3)=='K'
				|| *(pstruc->sp+3)=='M' || *(pstruc->sp+3)=='G'))
		i = -4;

	if (i >= 0) {
		ULONGLONG ntd;

		if (!m_bNet) {
			m_bNet = TRUE;
			Net_start(); // net.c
			Net_get(&net[0], &net[1]);
			net[2] = 0; net[3] = 0;
			g_bDispSecond = TRUE;
		}

		ntd = 1000 * net[i];
		if (*(pstruc->sp + 3) == 'K') ntd /= 1024;
		else if (*(pstruc->sp + 3) == 'M') ntd /= 1048576;
		else if (*(pstruc->sp + 3) == 'G') ntd /= 1048576 * 1024;

		pstruc->sp += 4;
		FormatFixedPointNum(&pstruc->sp, &pstruc->dp, ntd, 1000);
	} else
		*pstruc->dp++ = *pstruc->sp++;
}
#endif

#if TC_ENABLE_MEMORY
void MemoryHandler(FORMATHANDLERSTRUCT* pstruc)
{
	DWORDLONG m, t;
	BOOL bValid = TRUE;

	if (!m_bMem) {
		if (*(pstruc->sp+1) == 'K' || *(pstruc->sp+1) == 'M' || *(pstruc->sp+1) == 'G' || (
			(*(pstruc->sp+1)=='T'||*(pstruc->sp+1)=='A'||*(pstruc->sp+1)=='U')&&
			(*(pstruc->sp+2)=='P'||*(pstruc->sp+2)=='F'||*(pstruc->sp+2)=='V')&&
			(*(pstruc->sp+3)=='K'||*(pstruc->sp+3)=='M'||*(pstruc->sp+3)=='G'||*(pstruc->sp+3)=='P')))
		{
			m_bMem = TRUE;
			ms.dwLength = sizeof(ms);
			GlobalMemoryStatusEx(&ms);
			g_bDispSecond = TRUE;
		} else {
			*pstruc->dp++ = *pstruc->sp++;
			return;
		}
	}

	if (*(pstruc->sp + 1) == 'K')				// MK
	{
		m = (1000 * ms.ullAvailPhys) >> 10;
		pstruc->sp -= 2;
	}
	else if (*(pstruc->sp + 1) == 'M')			// MM
	{
		m = (1000 * ms.ullAvailPhys) >> 20;
		pstruc->sp -= 2;
	}
	else if (*(pstruc->sp + 1) == 'G')			// MG
	{
		m = (1000 * ms.ullAvailPhys) >> 30;
		pstruc->sp -= 2;
	}
	else
	{
		if (*(pstruc->sp + 1) == 'T')				// MT**
		{
			t = 0;
			if (*(pstruc->sp + 2) == 'P')			// MTP*
				m = ms.ullTotalPhys;
			else if (*(pstruc->sp + 2) == 'F')		// MTF*
				m = ms.ullTotalPageFile;
			else if (*(pstruc->sp + 2) == 'V')		// MTV*
				m = ms.ullTotalVirtual;
			else
				bValid = FALSE;
		}
		else if (*(pstruc->sp + 1) == 'A')			// MA**
		{
			if (*(pstruc->sp + 2) == 'P')			// MAP*
			{
				m = ms.ullAvailPhys;
				t = ms.ullTotalPhys;
			}
			else if (*(pstruc->sp + 2) == 'F')		// MAF*
			{
				m = ms.ullAvailPageFile;
				t = ms.ullTotalPageFile;
			}
			else if (*(pstruc->sp + 2) == 'V')		// MAV*
			{
				m = ms.ullAvailVirtual;
				t = ms.ullTotalVirtual;
			}
			else
				bValid = FALSE;
		}
		else if (*(pstruc->sp + 1) == 'U')			// MU**
		{
			if (*(pstruc->sp + 2) == 'P')			// MUP*
			{
				m = ms.ullTotalPhys - ms.ullAvailPhys;
				t = ms.ullTotalPhys;
			}
			else if (*(pstruc->sp + 2) == 'F')		// MUF*
			{
				m = ms.ullTotalPageFile - ms.ullAvailPageFile;
				t = ms.ullTotalPageFile;
			}
			else if (*(pstruc->sp + 2) == 'V')		// MUV*
			{
				m = ms.ullTotalVirtual - ms.ullAvailVirtual;
				t = ms.ullTotalVirtual;
			}
			else
				bValid = FALSE;
		}
		else
		{
			bValid = FALSE;
		}
		if (bValid)
		{
			if (*(pstruc->sp + 3) == 'K')		// M**K
				m = (1000 * m) >> 10;
			else if (*(pstruc->sp + 3) == 'M')	// M**M
				m = (1000 * m) >> 20;
			else if (*(pstruc->sp + 3) == 'G')	// M**G
				m = (1000 * m) >> 30;
			else if ((*(pstruc->sp + 3) == 'P') && (t != 0))	// M**P
				m = 1000 * m * 100 / t;
			else
				bValid = FALSE;
		}
	}

	if (bValid) {
		pstruc->sp += 4;
	//	FormatNum(&pstruc->sp, &pstruc->dp, (unsigned)(int) m);
		FormatFixedPointNum(&pstruc->sp, &pstruc->dp, m, 1000);
	} else
		*pstruc->dp++ = *pstruc->sp++;
}
#endif

#if TC_ENABLE_HDD
void HDDHandler(FORMATHANDLERSTRUCT* pstruc)
{
	LONGLONG d = -1;

	if (!m_bHDD) {
		if ((*(pstruc->sp+1)=='T'||*(pstruc->sp+1)=='A'||*(pstruc->sp+1)=='U')&&
			(*(pstruc->sp+3)=='M'||*(pstruc->sp+3)=='G'||
				*(pstruc->sp+3)=='T'||*(pstruc->sp+3)=='P')&&
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
				d = 1000 * diskAll[drv] / 1048576;
			else if (*(pstruc->sp + 3) == 'G')
				d = 1000 * diskAll[drv] / 1048576 / 1024;
			else if (*(pstruc->sp + 3) == 'T')
				d = 1000 * diskAll[drv] / 1048576 / 1048576;
		}
		else if (*(pstruc->sp + 1) == 'A')
		{
			if (*(pstruc->sp + 3) == 'M')
				d = 1000 * diskFree[drv] / 1048576;
			else if (*(pstruc->sp + 3) == 'G')
				d = 1000 * diskFree[drv] / 1048576 / 1024;
			else if (*(pstruc->sp + 3) == 'T')
				d = 1000 * diskFree[drv] / 1048576 / 1048576;
			else if (*(pstruc->sp + 3) == 'P')
				d = diskAll[drv] ? 1000 * diskFree[drv] * 100 / diskAll[drv] : 0;
		}
		else if (*(pstruc->sp + 1) == 'U')
		{
			if (*(pstruc->sp + 3) == 'M')
				d = 1000 * (diskAll[drv] - diskFree[drv]) / 1048576;
			else if (*(pstruc->sp + 3) == 'G')
				d = 1000 * (diskAll[drv] - diskFree[drv]) / 1048576 / 1024;
			else if (*(pstruc->sp + 3) == 'T')
				d = 1000 * (diskAll[drv] - diskFree[drv]) / 1048576 / 1048576;
			else if (*(pstruc->sp + 3) == 'P')
				d = diskAll[drv] ?
					1000 * (diskAll[drv] - diskFree[drv]) * 100 / diskAll[drv] : 0;
		}
	}

	if (d >= 0) {
		pstruc->sp += 4;
		FormatFixedPointNum(&pstruc->sp, &pstruc->dp, d, 1000);
	} else
		*pstruc->dp++ = *pstruc->sp++;
}
#endif

#if TC_ENABLE_CPU
void CPUHandler(FORMATHANDLERSTRUCT* pstruc)
{
	if(!m_bCPU)
	{
		m_bCPU = TRUE;
		CpuMoni_start(); // cpu.c
		iCPUUsage = CpuMoni_get(); // cpu.c
		g_bDispSecond = TRUE;
	}

	pstruc->sp += 2;
	FormatNum(&pstruc->sp, &pstruc->dp, iCPUUsage);
}
#endif

#if TC_ENABLE_BATTERY
void BatteryHandler(FORMATHANDLERSTRUCT* pstruc)
{
	if(!m_bBattery)
	{
		m_bBattery = TRUE;
		GetBatteryLifePercent(&iBatteryLife, &iBatteryMode); // battery.c
		g_bDispSecond = TRUE;
	}

	pstruc->sp += 2;
	FormatNum(&pstruc->sp, &pstruc->dp, iBatteryLife);
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
#endif

#if TC_ENABLE_VOLUME
void VolumeMuteHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int vol;
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
	vol = iVolume;
	if ( bMuteFlg )
		vol = 0;
	pstruc->sp += 2;
	FormatNum(&pstruc->sp, &pstruc->dp, vol);
}

void VolumeHandler(FORMATHANDLERSTRUCT* pstruc)
{
	int vol;
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
	FormatNum(&pstruc->sp, &pstruc->dp, iVolume);
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
#endif

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

void FormatNum(const wchar_t **sp, wchar_t **dp, unsigned n)
{
	int len, slen;
	BOOL bComma;
	
	GetNumFormat(sp, 'x', ',', &len, &slen, &bComma);
	SetNumFormat(dp, n, len, slen, bComma);
}

void FormatFixedPointNum(const wchar_t **sp, wchar_t **dp,
	ULONGLONG d, int multiplier)
{
	int len, slen, i;
	BOOL bComma;
	ULONGLONG n = d / multiplier;
	
	FormatNum(sp, dp, (unsigned) n);
	if (**sp == '.') {
		(*sp)++;
		if (GetNumFormat(sp, 'x', 'x', &len, &slen, &bComma)) {
			**dp = '.';
			(*dp)++;
			if (len > 3) len = 3;
			d -= n * multiplier;
			for (i = 0; i < len; i++) d *= 10;
			n = d / multiplier;
			SetNumFormat(dp, (unsigned) n, len, 0, FALSE);
		}
	}
}

#endif	/* TC_ENABLE_SYSINFO */
