/*-------------------------------------------------------------
  playfile.c : play sound file
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Globals */

BOOL PlayFile(HWND hwnd, const char *fname, int loops);
BOOL PlayFileCmdLine(HWND hwnd, const char *str);
void StopFile(void);
void OnMCINotify(HWND hwnd, WPARAM wFlags, LONG lDevID);
BOOL IsSoundFile(const char* fname);
BOOL PauseResume(HWND hwnd);
BOOL GetPlayingPosition(char *dst);
BOOL PrevNextTrack(HWND hwnd, BOOL bNext);
BOOL IsPrevNext(BOOL bNext);
BOOL PlayMCI(HWND hwnd, const char *fname, int loops);

/* Statics */

static BOOL StartMCI(HWND hwnd);
static BOOL PlayWave(HWND hwnd, const char *fname, int loops);
static void StopWave(void);

static char *m_soundexts[] = { "wav", "mid", "cda", "mp3", "wma", "ogg", NULL };

// MCI
static BOOL m_bMCIPlaying = FALSE;
static int  m_countPlay = 0, m_numLoops = 0;
static int  m_nTrack, m_nCDATrack;
static BOOL m_bAudioCD;
static BOOL m_bPausing = FALSE;

// WAV
static WAVEFORMATEX *m_pWaveFormat = NULL;
static HWAVEOUT m_hWaveOut = NULL;
static HPSTR    m_pWaveData = NULL;
static WAVEHDR  m_wh;

/* Externs */

/* exe/command.c , property/main.c
  This function must exist in other source file */
BOOL ExecCommandString(HWND hwnd, const char* command);

/*------------------------------------------------
  play sound or open file
--------------------------------------------------*/
BOOL PlayFile(HWND hwnd, const char *fname, int loops)
{
	BOOL bMCIWave;
	char fname2[MAX_PATH];
	BOOL r;
	
	if(!fname || *fname == 0) return FALSE;
	
	bMCIWave = GetMyRegLong("", "MCIWave", FALSE);
	
	if(!bMCIWave && ext_cmp(fname, "wav") == 0)
	{
		if(m_bMCIPlaying || m_hWaveOut) return FALSE;
		RelToAbs(fname2, fname);
		r = PlayWave(hwnd, fname2, loops);
		if(r) return r;
	}
	else if(IsSoundFile(fname))
	{
		if(m_bMCIPlaying || m_hWaveOut) return FALSE;
		RelToAbs(fname2, fname);
		r = PlayMCI(hwnd, fname2, loops);
		if(r) return r;
	}
	
	ExecCommandString(hwnd, fname);
	return FALSE;
}

/*------------------------------------------------
  str: "3 sound.wav"
--------------------------------------------------*/
BOOL PlayFileCmdLine(HWND hwnd, const char *str)
{
	char fname[MAX_PATH];
	const char *p = str;
	int loops = 0;
	int i;
	
	if(strncmp(p, "-1 ", 3) == 0)
	{
		loops = -1; p += 3;
	}
	else
	{
		while(*p)
		{
			if('0' <= *p && *p <= '9')
				loops = loops * 10 + (*p - '0');
			else break;
			p++;
		}
	}
	while(*p == ' ') p++;
	
	for(i = 0; i < MAX_PATH-1 && *p; i++)
		fname[i] = *p++;
	fname[i] = 0;
	
	return PlayFile(hwnd, fname, loops);
}

/*------------------------------------------------
   stop playing sound
--------------------------------------------------*/
void StopFile(void)
{
	StopWave();
	if(m_bMCIPlaying)
	{
		mciSendString("stop myfile", NULL, 0, NULL);
		mciSendString("close myfile", NULL, 0, NULL);
		m_bMCIPlaying = FALSE;
		m_bPausing = FALSE;
		m_countPlay = 0; m_numLoops = 0;
	}
}

/*------------------------------------------------
   MM_MCINOTIFY message
--------------------------------------------------*/
void OnMCINotify(HWND hwnd, WPARAM wFlags, LONG lDevID)
{
	if(!m_bMCIPlaying) return;
	
	if(wFlags != MCI_NOTIFY_SUCCESSFUL && wFlags != MCI_NOTIFY_FAILURE)
		return;
	
	if(wFlags == MCI_NOTIFY_SUCCESSFUL &&
		(m_countPlay < m_numLoops || m_numLoops < 0))
	{
		mciSendString("seek myfile to start wait", NULL, 0, NULL);
		if(StartMCI(hwnd))
			m_countPlay++;
		else
			StopFile();
	}
	else StopFile();
}

/*------------------------------------------------
  Is CD-ROM ?
--------------------------------------------------*/
BOOL IsCDROM(const char *p)
{
	if((('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z')) &&
		*(p + 1) == ':' && 
		(*(p + 2) == 0 || (*(p + 2) == '\\' && *(p + 3) == 0)))
	{
		char temp[10];
		
		temp[0] = p[0]; temp[1] = p[1];
		temp[2] = '\\'; temp[3] = 0;
		if(GetDriveType(temp) == DRIVE_CDROM) return TRUE;
	}
	
	return FALSE;
}

/*------------------------------------------------
  Is sound file ?
--------------------------------------------------*/
BOOL IsSoundFile(const char* fname)
{
	int i;
	
	if(lstrcmpi(fname, "cdaudio") == 0) return TRUE;
	
	if(IsCDROM(fname)) return TRUE;
	
	for(i = 0; m_soundexts[i]; i++)
	{
		if(ext_cmp(fname, m_soundexts[i]) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
   Pause/Resume sound
--------------------------------------------------*/
BOOL PauseResume(HWND hwnd)
{
	if(m_bMCIPlaying)
	{
		int r;
		
		if(m_bPausing)
		{
			r = mciSendString("play myfile notify", NULL, 0, hwnd);
			m_bPausing = FALSE;
		}
		else
		{
			r = mciSendString("pause myfile", NULL, 0, NULL);
			m_bPausing = TRUE;
		}
		if(r == 0) return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
  get elapsed time string
--------------------------------------------------*/
BOOL GetPlayingPosition(char *dst)
{
	char retr[21], *p;
	int sec;
	
	if(!m_bMCIPlaying) return FALSE;
	
	if(mciSendString("status myfile position", retr, 20, NULL) != 0)
		return FALSE;
	
	if(m_bAudioCD) // 01:02:03:04 -> [01]:02:03
	{
		int c = 0;
		
		p = retr;
		*dst++ = '[';
		while(*p)
		{
			if(!('0' <= *p && *p <= '9'))
			{
				if(c == 0) { *dst++ = ']'; p++; }
				else if(c == 1) { *dst++ = *p++; }
				else break;
				c++;
			}
			else *dst++ = *p++;
		}
		*dst = 0;
	}
	else
	{
		sec = atoi(retr) / 1000;
		wsprintf(dst, "%02d:%02d", sec/60, sec%60);
	}
	return TRUE;
}

/*------------------------------------------------
   Previous/Next Track of Audio CD
--------------------------------------------------*/
BOOL PrevNextTrack(HWND hwnd, BOOL bNext)
{
	char retr[20], command[80];
	int nTrack;
	
	if(! (m_bMCIPlaying && m_bAudioCD && !m_nCDATrack))
		return FALSE;
	
	if(mciSendString("status myfile current track", retr, 20, NULL) != 0)
		return FALSE;
	
	nTrack = atoi(retr);
	if(bNext)
	{
		if(nTrack >= m_nTrack) return TRUE;
		nTrack++;
	}
	else
	{
		if(nTrack < 2) return TRUE;
		nTrack--;
	}
	wsprintf(command,
		"play myfile from %02d:00:00:00 notify", nTrack);
	if(mciSendString(command, NULL, 0, hwnd) == 0)
		return TRUE;
	return FALSE;
}

/*------------------------------------------------
  Prev/Next can be used?
--------------------------------------------------*/
BOOL IsPrevNext(BOOL bNext)
{
	char retr[20];
	int nTrack;
	
	if(! (m_bMCIPlaying && m_bAudioCD && !m_nCDATrack))
		return FALSE;
	
	if(mciSendString("status myfile current track", retr, 20, NULL) != 0)
		return FALSE;
	
	nTrack = atoi(retr);
	if(bNext)
	{
		if(nTrack >= m_nTrack) return FALSE;
	}
	else
	{
		if(nTrack < 2) return FALSE;
	}
	return TRUE;
}

/*------------------------------------------------
  create a string:  "*.wav;*.mid; ...."
--------------------------------------------------*/
void GetSoundFileExts(char* dst)
{
	char *dp;
	int i;
	
	dp = dst;
	for(i = 0; m_soundexts[i]; i++)
	{
		if(dp != dst) *dp++ = ';';
		*dp++ = '*'; *dp++ = '.';
		strcpy(dp, m_soundexts[i]);
		dp += strlen(dp);
	}
	*dp = 0;
}

/*------------------------------------------------
  open MCI file and play
--------------------------------------------------*/
BOOL PlayMCI(HWND hwnd, const char *fname, int loops)
{
	char command[MAX_PATH+30], retr[80];
	BOOL bCDROM;
	
	if(ext_cmp(fname, "ogg") == 0)
		WriteProfileString("MCI Extensions", "ogg", "MPEGVideo");
	
	bCDROM = IsCDROM(fname);
	
	strcpy(command, "open \"");
	strcat(command, fname);
	if(bCDROM && command[strlen(command)-1] == '\\')
		 command[strlen(command)-1] = 0;
	strcat(command, "\"");
	if(bCDROM) strcat(command, " type cdaudio");
	strcat(command, " alias myfile");
	
	if(mciSendString(command, NULL, 0, NULL) != 0) return FALSE;
	
	m_bAudioCD = FALSE;
	if(lstrcmpi(fname, "cdaudio") == 0 || bCDROM ||
		ext_cmp(fname, "cda") == 0)
		m_bAudioCD = TRUE;
	
	strcpy(command, "set myfile time format ");
	if(m_bAudioCD)
		strcat(command, "tmsf");
	else
		strcat(command, "milliseconds");
	mciSendString(command, NULL, 0, NULL);
	
	m_nTrack = 1;
	if(m_bAudioCD)
	{
		if(mciSendString("status myfile number of tracks",
			retr, 80, NULL) == 0)
				m_nTrack = atoi(retr);
	}
	
	m_nCDATrack = 0;
	if(ext_cmp(fname, "cda") == 0)
	{
		const char* p = fname;
		while(*p)
		{
			if('0' <= *p && *p <= '9')
				m_nCDATrack = m_nCDATrack * 10 + *p - '0';
			p++;
		}
	}
	
	if(StartMCI(hwnd))
	{
		m_bMCIPlaying = TRUE;
		m_countPlay = 1; m_numLoops = loops;
	}
	else
	{
		mciSendString("close myfile", NULL, 0, NULL);
	}
	
	return m_bMCIPlaying;
}

/*------------------------------------------------
   play MCI file
--------------------------------------------------*/
BOOL StartMCI(HWND hwnd)
{
	char command[80];
	
	strcpy(command, "play myfile");
	if(m_nCDATrack > 0)
	{
		wsprintf(command + strlen(command), " from %02d:00:00:00",
			m_nCDATrack);
		if(m_nCDATrack < m_nTrack)
			wsprintf(command + strlen(command), " to %02d:00:00:00",
				m_nCDATrack+1);
	}
	strcat(command, " notify");
	
	if(mciSendString(command, NULL, 0, hwnd) == 0) return TRUE;
	return FALSE;
}

/*------------------------------------------------
  Play WAV
--------------------------------------------------*/
BOOL PlayWave(HWND hwnd, const char *fname, int loops)
{
	HMMIO hmmio;
	MMCKINFO mmckinfoParent;
	MMCKINFO mmckinfoSubchunk;
	LONG lFmtSize;
	LONG lDataSize;
	
	if(m_hWaveOut != NULL) return FALSE;
	
	hmmio = mmioOpen((LPSTR)fname, NULL, MMIO_READ | MMIO_ALLOCBUF);
	if(!hmmio) return FALSE;
	
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if(mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF))
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
		MMIO_FINDCHUNK))
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	lFmtSize = mmckinfoSubchunk.cksize;
	m_pWaveFormat = (WAVEFORMATEX*)malloc(lFmtSize);
	if(m_pWaveFormat == NULL)
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, (HPSTR)m_pWaveFormat, lFmtSize) != lFmtSize)
	{
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	/*
	if(m_pWaveFormat->wFormatTag != WAVE_FORMAT_PCM)
	{
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	*/

	if(waveOutOpen(&m_hWaveOut, WAVE_MAPPER,
		m_pWaveFormat, 0, 0, WAVE_FORMAT_QUERY))
	{
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmioAscend(hmmio, &mmckinfoSubchunk, 0);
    
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
		MMIO_FINDCHUNK))
    {
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	lDataSize = mmckinfoSubchunk.cksize;
	if(lDataSize == 0)
    {
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	m_pWaveData = (HPSTR)malloc(lDataSize);
	if(m_pWaveData == NULL)
	{
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, m_pWaveData, lDataSize) != lDataSize)
	{
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		free(m_pWaveData); m_pWaveData = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	mmioClose(hmmio, 0);
	
	if(waveOutOpen(&m_hWaveOut, WAVE_MAPPER,
		m_pWaveFormat, (DWORD_PTR)hwnd, 0, CALLBACK_WINDOW))
    {
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		free(m_pWaveData); m_pWaveData = NULL;
		return FALSE;
	}
	
	memset(&m_wh, 0, sizeof(WAVEHDR));
	m_wh.lpData = m_pWaveData;
	m_wh.dwBufferLength = lDataSize;
	if(loops != 0)
	{
		m_wh.dwFlags = WHDR_BEGINLOOP|WHDR_ENDLOOP;
		m_wh.dwLoops = (DWORD)loops;
	}
	if(waveOutPrepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR)))
	{
		waveOutClose(m_hWaveOut); m_hWaveOut = NULL;
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		free(m_pWaveData); m_pWaveData = NULL;
		return FALSE;
	}
	
	if(waveOutWrite(m_hWaveOut, &m_wh, sizeof(WAVEHDR)) != 0)
	{
		waveOutUnprepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));
		waveOutClose(m_hWaveOut); m_hWaveOut = NULL;
		free(m_pWaveFormat); m_pWaveFormat = NULL;
		free(m_pWaveData); m_pWaveData = NULL;
		return FALSE;
	}
	
	return TRUE;
}

/*------------------------------------------------
  stop playing WAV
--------------------------------------------------*/
void StopWave(void)
{
	if(m_hWaveOut == NULL) return;
	
	waveOutReset(m_hWaveOut);
	waveOutUnprepareHeader(m_hWaveOut, &m_wh, sizeof(WAVEHDR));
	waveOutClose(m_hWaveOut);
	m_hWaveOut = NULL;
	free(m_pWaveFormat); m_pWaveFormat = NULL;
	free(m_pWaveData); m_pWaveData = NULL;
}

