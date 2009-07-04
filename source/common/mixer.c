/*-------------------------------------------------------------
  mixer.c

  http://hp.vector.co.jp/authors/VA016117/mixer1.html

  $Id:$
---------------------------------------------------------------*/

#include "../common/common.h"
#include <mmsystem.h>

#if TC_ENABLE_VOLUME

BOOL InitMasterVolumeControl(void);
BOOL InitMasterMuteControl(void);

#define MAXCHANNEL 10

HMIXER      g_hMixer=NULL;
MIXERCAPS   g_mxcaps;
DWORD       g_dwMinimum,g_dwMaximum;
DWORD       g_cChannels;
DWORD       g_dwVolumeControlID=-1;
DWORD       g_dwMuteControlID=-1;
DWORD       g_dwVolComponentType;

typedef struct{
	MIXERCONTROLDETAILS_UNSIGNED    Volume[MAXCHANNEL];
	DWORD                           Max;
} LASTVOLINFO,*LPLASTVOLINFO;

LASTVOLINFO	g_LastVolInfo;

/*-----------------------------------------------------------------------------
//   Mixer Device Open/Close
-----------------------------------------------------------------------------*/

BOOL InitMixer(void)
{
	int volComponentType;
	
/*  SelectComponentType

Kernel Streaming Topology to Audio Mixer API Translation
http://msdn2.microsoft.com/en-us/library/ms790574.aspx
http://msdn2.microsoft.com/en-us/library/ms790537.aspx

MIXERLINE_COMPONENTTYPE_DST_UNDEFINED    0
MIXERLINE_COMPONENTTYPE_DST_DIGITAL      1
MIXERLINE_COMPONENTTYPE_DST_LINE         2
MIXERLINE_COMPONENTTYPE_DST_MONITOR      3
MIXERLINE_COMPONENTTYPE_DST_SPEAKERS     4  //メインボリューム
MIXERLINE_COMPONENTTYPE_DST_HEADPHONES   5
MIXERLINE_COMPONENTTYPE_DST_TELEPHONE    6
MIXERLINE_COMPONENTTYPE_DST_WAVEIN       7
MIXERLINE_COMPONENTTYPE_DST_VOICEIN      8
MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED    9
MIXERLINE_COMPONENTTYPE_SRC_DIGITAL     10
MIXERLINE_COMPONENTTYPE_SRC_LINE        11
MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE  12
MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER 13
MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC 14
MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE   15
MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER   16
MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT     17  //WAVE音量らしい
MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY   18
MIXERLINE_COMPONENTTYPE_SRC_ANALOG      19
*/
	volComponentType = GetMyRegLong(NULL, "VolComponentType", 4);
	if ( volComponentType >= 0 && volComponentType <= 8 )
		g_dwVolComponentType = MIXERLINE_COMPONENTTYPE_DST_FIRST + volComponentType;
	else if ( volComponentType >= 9 && volComponentType <= 19 )
		g_dwVolComponentType = MIXERLINE_COMPONENTTYPE_SRC_FIRST + volComponentType - 9;
	else
		g_dwVolComponentType = MIXERLINE_COMPONENTTYPE_DST_FIRST + 4;

	if(g_hMixer) return TRUE;
	if(mixerOpen(&g_hMixer,0,0,0,MIXER_OBJECTF_MIXER)!=MMSYSERR_NOERROR) return FALSE;
	if(mixerGetDevCaps((UINT)g_hMixer, &g_mxcaps, sizeof(MIXERCAPS))!=MMSYSERR_NOERROR) return FALSE;
	return	TRUE;
}

void ReleaseMixer(void)
{
	if(g_hMixer) mixerClose(g_hMixer);
	g_hMixer=NULL;
	g_dwVolumeControlID=-1;
	g_dwMuteControlID=-1;
}

/*-----------------------------------------------------------------------------
//   Master Volume
//-----------------------------------------------------------------------------*/
BOOL InitMasterVolumeControl(void)
{
	int	i;
	MIXERLINE mxl;
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;

	if(g_dwVolumeControlID != (DWORD)-1) return TRUE;
	if(InitMixer() == FALSE) return FALSE;
	
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwComponentType = g_dwVolComponentType;
	if(mixerGetLineInfo((HMIXEROBJ)g_hMixer,&mxl,
				MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)!=MMSYSERR_NOERROR) return FALSE;

	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if(mixerGetLineControls((HMIXEROBJ)g_hMixer,&mxlc,
			   MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return FALSE;

	g_cChannels = (mxl.cChannels > MAXCHANNEL)? MAXCHANNEL : mxl.cChannels;
	g_dwMinimum = mxc.Bounds.dwMinimum;
	g_dwMaximum = mxc.Bounds.dwMaximum;
	g_dwVolumeControlID = mxc.dwControlID;
	
	for(i = 0; i < MAXCHANNEL; i++) g_LastVolInfo.Volume[i].dwValue = 1;
	g_LastVolInfo.Max = 1;

	return TRUE;
}

/*	Val:現在の音量 0-100 にスケールされる */
BOOL GetMasterVolume(int *Val)
{
	MIXERCONTROLDETAILS_UNSIGNED mxcdVolume;
	MIXERCONTROLDETAILS mxcd;


	if(InitMasterVolumeControl() == FALSE){
		*Val = -1;
		 return FALSE;
	}

	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = g_dwVolumeControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = &mxcdVolume;
	if(mixerGetControlDetails((HMIXEROBJ)g_hMixer,&mxcd,
				 MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)!=MMSYSERR_NOERROR) return FALSE;
	
	*Val = ((mxcdVolume.dwValue-g_dwMinimum)*100 + (g_dwMaximum-g_dwMinimum)/2)/(g_dwMaximum-g_dwMinimum);

	return TRUE;
}

BOOL SetMasterVolume(int Val)
{
	MIXERCONTROLDETAILS_UNSIGNED	mxcdVolume[MAXCHANNEL];
	MIXERCONTROLDETAILS				mxcd;
	DWORD							dwVal,dwMaxVal;
	DWORD							i;

	if(InitMasterVolumeControl() == FALSE) return FALSE;
	
	if(Val>100)
		Val = 100;
	if(Val<0)
		Val = 0;

	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = g_dwVolumeControlID;
	mxcd.cChannels = g_cChannels;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mxcd.paDetails = mxcdVolume;

	if(mixerGetControlDetails((HMIXEROBJ)g_hMixer,&mxcd,
				 MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)!=MMSYSERR_NOERROR) return FALSE;
	
	dwMaxVal = mxcdVolume[0].dwValue;
	for(i = 1; i < g_cChannels; i++){
		if(mxcdVolume[i].dwValue > dwMaxVal) dwMaxVal = mxcdVolume[i].dwValue;
	}

	if(dwMaxVal){
		memcpy(g_LastVolInfo.Volume, mxcdVolume, sizeof(g_LastVolInfo.Volume));
		g_LastVolInfo.Max = dwMaxVal;
	}

	dwVal = Val*(g_dwMaximum-g_dwMinimum)/100+g_dwMinimum;
	for(i = 0; i < g_cChannels; i++){
		mxcdVolume[i].dwValue = dwVal * g_LastVolInfo.Volume[i].dwValue / g_LastVolInfo.Max;
	}

	if (mixerSetControlDetails((HMIXEROBJ)g_hMixer,&mxcd,
				 MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE)!=MMSYSERR_NOERROR) return FALSE;
	
	return TRUE;
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
//-----------------------------------------------------------------------------*/

BOOL InitMasterMuteControl()
{
	MIXERLINE mxl;
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;

	if(g_dwMuteControlID != (DWORD)-1) return TRUE;
	if(InitMixer() == FALSE) return FALSE;
	
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwComponentType = g_dwVolComponentType;
	if(mixerGetLineInfo((HMIXEROBJ)g_hMixer,&mxl,
				MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)!=MMSYSERR_NOERROR) return FALSE;

	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if(mixerGetLineControls((HMIXEROBJ)g_hMixer,&mxlc,
			   MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return FALSE;

	g_dwMuteControlID = mxc.dwControlID;

	return TRUE;
}

BOOL GetMasterMute(BOOL *Val)
{
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
	MIXERCONTROLDETAILS mxcd;

	*Val = FALSE;
	if(InitMasterMuteControl() == FALSE) return FALSE;

	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = g_dwMuteControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = &mxcdMute;
	if (mixerGetControlDetails((HMIXEROBJ)g_hMixer,&mxcd,
				 MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)!=MMSYSERR_NOERROR) return FALSE;
	
	*Val = mxcdMute.fValue;

	return TRUE;
}

BOOL SetMasterMute(BOOL Val)
{
	MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
	MIXERCONTROLDETAILS mxcd;

	if(InitMasterMuteControl() == FALSE) return FALSE;

	mxcdMute.fValue=Val;

	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = g_dwMuteControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mxcd.paDetails = &mxcdMute;
	if (mixerSetControlDetails((HMIXEROBJ)g_hMixer,&mxcd,
				 MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE)!=MMSYSERR_NOERROR) return FALSE;
	
	return TRUE;
}

BOOL ReverseMasterMute(void)
{
	BOOL lVal;

	if(GetMasterMute(&lVal)==FALSE) return FALSE;
	if(SetMasterMute(!lVal)==FALSE) return FALSE;
	
	return TRUE;
}

#endif	/* TC_ENABLE_VOLUME */
