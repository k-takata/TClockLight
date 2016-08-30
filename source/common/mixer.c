/*-------------------------------------------------------------
  mixer.c

  http://hp.vector.co.jp/authors/VA016117/mixer1.html
---------------------------------------------------------------*/

#include "../common/common.h"
#include <mmsystem.h>

#if TC_ENABLE_VOLUME

/* vistavol.cpp */

BOOL SetMasterVolumeVista(int iLevel);
BOOL GetMasterVolumeVista(int *piLevel);
BOOL SetMasterMuteVista(BOOL mute);
BOOL GetMasterMuteVista(BOOL *pmute);
BOOL InitVolumeVista(void);
void ReleaseVolumeVista(void);


/*-----------------------------------------------------------------------------
//   Mixer Device Open/Close
-----------------------------------------------------------------------------*/

BOOL InitMixer(void)
{
	return InitVolumeVista();
}

void ReleaseMixer(void)
{
	ReleaseVolumeVista();
}

/*-----------------------------------------------------------------------------
//   Master Volume
-----------------------------------------------------------------------------*/

/*	Val:Œ»Ý‚Ì‰¹—Ê 0-100 ‚ÉƒXƒP[ƒ‹‚³‚ê‚é */
BOOL GetMasterVolume(int *Val)
{
	return GetMasterVolumeVista(Val);
}

BOOL SetMasterVolume(int Val)
{
	return SetMasterVolumeVista(Val);
}

BOOL UpDownMasterVolume(int dif)
{
	int Val;

	if(GetMasterVolume(&Val)==FALSE) return FALSE;

	Val += dif;
	if(Val>100) Val=100;
	else if(Val<0) Val=0;

	if(SetMasterVolume(Val)==FALSE) return FALSE;

	return	TRUE;
}

/*-----------------------------------------------------------------------------
//   Master.Mute
-----------------------------------------------------------------------------*/

BOOL GetMasterMute(BOOL *Val)
{
	return GetMasterMuteVista(Val);
}

BOOL SetMasterMute(BOOL Val)
{
	return SetMasterMuteVista(Val);
}

BOOL ReverseMasterMute(void)
{
	BOOL lVal;

	if(GetMasterMute(&lVal)==FALSE) return FALSE;
	if(SetMasterMute(!lVal)==FALSE) return FALSE;
	
	return TRUE;
}

#endif	/* TC_ENABLE_VOLUME */
