/*-------------------------------------------------------------
  vistavol.cpp : Windows Vista Master Volume Control
  
---------------------------------------------------------------*/

#define INITGUID
#include "../config.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#if TC_ENABLE_VOLUME


#define SAFE_RELEASE(punk) \
		if((punk) != NULL) { (punk)->Release(); (punk) = NULL; }


DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C,
	0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35,
	0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546,
	0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);


IAudioEndpointVolume* GetDefaultAudioEndpointVolume()
{
	HRESULT hr;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pEndpoint = NULL;
	IAudioEndpointVolume* pAudioEndVol = NULL;
	
	// オーディオデバイス一覧取得用のインターフェースを取得
	hr = CoCreateInstance(
			CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
			IID_PPV_ARGS(&pEnumerator));
	if(FAILED(hr))
	{
		goto end;
	}
	
	// 一覧の中からデフォルトのオーディオデバイスを取得する
	hr = pEnumerator->GetDefaultAudioEndpoint(
			eRender, eConsole,
			&pEndpoint);
	if(FAILED(hr))
	{
		goto end;
	}
	
	// 取得したオーディオデバイスからメインボリューム操作用のインターフェースを取得
	hr = pEndpoint->Activate(
			IID_IAudioEndpointVolume, CLSCTX_ALL,
			NULL, (void**) &pAudioEndVol);
	if(FAILED(hr))
	{
		goto end;
	}
	
end:
	SAFE_RELEASE(pEndpoint);
	SAFE_RELEASE(pEnumerator);
	
	return pAudioEndVol;
}


extern "C"
BOOL SetMasterVolumeVista(int iLevel)
{
	HRESULT hr;
	IAudioEndpointVolume* pAudioEndVol = GetDefaultAudioEndpointVolume();
	if(pAudioEndVol == NULL)
	{
		return FALSE;
	}
	
	hr = pAudioEndVol->SetMasterVolumeLevelScalar(iLevel / 100.0f, NULL);
	SAFE_RELEASE(pAudioEndVol);
	
	return FAILED(hr) ? FALSE : TRUE;
}

extern "C"
BOOL GetMasterVolumeVista(int *piLevel)
{
	HRESULT hr;
	*piLevel = -1;
	IAudioEndpointVolume* pAudioEndVol = GetDefaultAudioEndpointVolume();
	if(pAudioEndVol == NULL)
	{
		return FALSE;
	}
	
	float fLevel;
	hr = pAudioEndVol->GetMasterVolumeLevelScalar(&fLevel);
	SAFE_RELEASE(pAudioEndVol);
	
	if(FAILED(hr))
	{
		return FALSE;
	}
	*piLevel = (int) (fLevel * 100 + 0.5);
	return TRUE;
}

extern "C"
BOOL SetMasterMuteVista(BOOL mute)
{
	HRESULT hr;
	IAudioEndpointVolume* pAudioEndVol = GetDefaultAudioEndpointVolume();
	if(pAudioEndVol == NULL)
	{
		return FALSE;
	}
	
	hr = pAudioEndVol->SetMute(mute, NULL);
	SAFE_RELEASE(pAudioEndVol);
	
	return FAILED(hr) ? FALSE : TRUE;
}

extern "C"
BOOL GetMasterMuteVista(BOOL *pmute)
{
	HRESULT hr;
	*pmute = FALSE;
	IAudioEndpointVolume* pAudioEndVol = GetDefaultAudioEndpointVolume();
	if(pAudioEndVol == NULL)
	{
		return FALSE;
	}
	
	hr = pAudioEndVol->GetMute(pmute);
	SAFE_RELEASE(pAudioEndVol);
	
	return FAILED(hr) ? FALSE : TRUE;
}

extern "C"
BOOL InitVolumeVista(void)
{
	CoInitialize(NULL);
	return TRUE;
}

extern "C"
void ReleaseVolumeVista(void)
{
	CoUninitialize();
}


#if defined(_MSC_VER) && defined(NODEFAULTLIB)
extern "C" int _fltused = 0x9875;
#endif


#endif	/* TC_ENABLE_VOLUME */
