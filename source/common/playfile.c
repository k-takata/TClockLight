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

static const char *m_soundexts[] =
	{ "wav", "mid", "cda", "mp3", "wma", "ogg", NULL };

// MCI
static MCIDEVICEID m_wDeviceID;
static BOOL  m_bMCIPlaying = FALSE;
static int   m_countPlay = 0, m_numLoops = 0;
static DWORD m_nTrack, m_nCDATrack;
static BOOL  m_bAudioCD;
static BOOL  m_bPausing = FALSE;

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
	
	bMCIWave = GetMyRegLong(NULL, "MCIWave", FALSE);
	
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
		mciSendCommand(m_wDeviceID, MCI_STOP, MCI_WAIT, NULL);
		mciSendCommand(m_wDeviceID, MCI_CLOSE, 0, NULL);
		m_bMCIPlaying = FALSE;
		m_bPausing = FALSE;
		m_countPlay = 0;
		m_numLoops = 0;
	}
}

/*------------------------------------------------
   MM_MCINOTIFY message
--------------------------------------------------*/
void OnMCINotify(HWND hwnd, WPARAM wFlags, LONG lDevID)
{
	if(!m_bMCIPlaying) return;
	
	if (wFlags == MCI_NOTIFY_SUCCESSFUL
		&& (m_countPlay < m_numLoops || m_numLoops < 0))
	{
		mciSendCommand(m_wDeviceID, MCI_SEEK,
			MCI_SEEK_TO_START | MCI_WAIT, NULL);
		if (StartMCI(hwnd))
		{
			m_countPlay++;
			return;
		}
	}
	if (wFlags == MCI_NOTIFY_SUCCESSFUL || wFlags == MCI_NOTIFY_FAILURE)
		StopFile();
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
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
   Pause/Resume sound
--------------------------------------------------*/
BOOL PauseResume(HWND hwnd)
{
	if (m_bMCIPlaying)
	{
		MCIERROR err;
		
		if (m_bPausing)
		{
			MCI_PLAY_PARMS mpp;
			mpp.dwCallback = (DWORD_PTR)hwnd;
			err = mciSendCommand(m_wDeviceID, MCI_PLAY, MCI_NOTIFY,
				(DWORD_PTR)&mpp);
			m_bPausing = FALSE;
		}
		else
		{
			err = mciSendCommand(m_wDeviceID, MCI_PAUSE, 0, NULL);
			m_bPausing = TRUE;
		}
		if (err == 0) return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
  get elapsed time string
--------------------------------------------------*/
BOOL GetPlayingPosition(char *dst)
{
	MCI_STATUS_PARMS msp;

	if (!m_bMCIPlaying) return FALSE;

	msp.dwItem = MCI_STATUS_POSITION;
	if (mciSendCommand(m_wDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
			(DWORD_PTR)&msp) != 0)
		return FALSE;

	if (m_bAudioCD)
		wsprintf(dst, "[%02d]%02d:%02d", MCI_TMSF_TRACK(msp.dwReturn),
			MCI_TMSF_MINUTE(msp.dwReturn), MCI_TMSF_SECOND(msp.dwReturn));
	else
		wsprintf(dst, "%02d:%02d",
			msp.dwReturn / 60000, msp.dwReturn / 1000 % 60);

	return TRUE;
}

/*------------------------------------------------
   Previous/Next Track of Audio CD
--------------------------------------------------*/
BOOL PrevNextTrack(HWND hwnd, BOOL bNext)
{
	MCI_STATUS_PARMS msp;
	MCI_PLAY_PARMS mpp;
	DWORD nTrack;

	if (!m_bMCIPlaying || !m_bAudioCD || m_nCDATrack)
		return FALSE;

	msp.dwItem = MCI_STATUS_CURRENT_TRACK;
	if (mciSendCommand(m_wDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
			(DWORD_PTR)&msp) != 0)
		return FALSE;
	nTrack = msp.dwReturn;

	if (bNext)
	{
		if (nTrack >= m_nTrack) return TRUE;
		nTrack++;
	}
	else
	{
		if (nTrack <= 1) return TRUE;
		nTrack--;
	}

	mpp.dwCallback = (DWORD_PTR)hwnd;
	mpp.dwFrom = MCI_MAKE_TMSF(nTrack, 0, 0, 0);

	return (mciSendCommand(m_wDeviceID, MCI_PLAY, MCI_FROM | MCI_NOTIFY,
		(DWORD_PTR)&mpp) == 0);
}

/*------------------------------------------------
  Prev/Next can be used?
--------------------------------------------------*/
BOOL IsPrevNext(BOOL bNext)
{
	MCI_STATUS_PARMS msp;

	if (!m_bMCIPlaying || !m_bAudioCD || m_nCDATrack)
		return FALSE;

	msp.dwItem = MCI_STATUS_CURRENT_TRACK;
	if (mciSendCommand(m_wDeviceID, MCI_STATUS, MCI_STATUS_ITEM,
			(DWORD_PTR)&msp) != 0)
		return FALSE;

	if (bNext)
	{
		if (msp.dwReturn >= m_nTrack) return FALSE;
	}
	else
	{
		if (msp.dwReturn <= 1) return FALSE;
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
	MCI_OPEN_PARMS mop;
	MCI_SET_PARMS msp;
	MCIERROR err;
	BOOL bCD, bCDROM, bCDA;
	DWORD cmd = MCI_WAIT;
	
	if(ext_cmp(fname, "ogg") == 0)
		WriteProfileString("MCI Extensions", "ogg", "MPEGVideo");

	bCD = (lstrcmpi(fname, "cdaudio") == 0);
	bCDROM = IsCDROM(fname);

	if (bCD || bCDROM)
	{
		mop.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
		cmd |= MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID;
	}
	if (!bCD)
	{
		mop.lpstrElementName = fname;
		cmd |= MCI_OPEN_ELEMENT;
	}

	err = mciSendCommand(0, MCI_OPEN, cmd, (DWORD_PTR)&mop);
	if (err != 0)
	{
		char str[1024];
		if (mciGetErrorString(err, str, 1024))
			MessageBox(hwnd, str, NULL, MB_ICONERROR);
		return FALSE;
	}
	m_wDeviceID = mop.wDeviceID;

	bCDA = (ext_cmp(fname, "cda") == 0);
	m_bAudioCD = bCD || bCDROM || bCDA;

	msp.dwTimeFormat = m_bAudioCD ? MCI_FORMAT_TMSF : MCI_FORMAT_MILLISECONDS;
	mciSendCommand(m_wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&msp);
	
	m_nTrack = 1;
	if (m_bAudioCD)
	{
		MCI_STATUS_PARMS mstp;
		mstp.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
		if (mciSendCommand(m_wDeviceID,
				MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mstp) == 0)
			m_nTrack = mstp.dwReturn;
	}
	
	m_nCDATrack = 0;
	if (bCDA)
	{
		const char *p = fname;
		while (*p)
		{
			if ('0' <= *p && *p <= '9')
				m_nCDATrack = m_nCDATrack * 10 + *p - '0';
			p++;
		}
	}
	
	if (StartMCI(hwnd))
	{
		m_bMCIPlaying = TRUE;
		m_countPlay = 1;
		m_numLoops = loops;
	}
	else
	{
		mciSendCommand(m_wDeviceID, MCI_CLOSE, 0, NULL);
	}
	
	return m_bMCIPlaying;
}

/*------------------------------------------------
   play MCI file
--------------------------------------------------*/
BOOL StartMCI(HWND hwnd)
{
	MCI_PLAY_PARMS mpp;
	DWORD cmd = MCI_NOTIFY;

	mpp.dwCallback = (DWORD_PTR)hwnd;

	if (m_nCDATrack > 0)
	{
		mpp.dwFrom = MCI_MAKE_TMSF(m_nCDATrack, 0, 0, 0);
		cmd |= MCI_FROM;

		if (m_nCDATrack < m_nTrack)
		{
			mpp.dwTo = MCI_MAKE_TMSF(m_nCDATrack + 1, 0, 0, 0);
			cmd |= MCI_TO;
		}
	}

	return (mciSendCommand(m_wDeviceID, MCI_PLAY, cmd, (DWORD_PTR)&mpp) == 0);
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
	if(mmioDescend(hmmio, &mmckinfoParent, NULL, MMIO_FINDRIFF))
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
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
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
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
	
	if(waveOutWrite(m_hWaveOut, &m_wh, sizeof(WAVEHDR)))
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

