/*-------------------------------------------------------------
  vistavol.cpp : Windows Vista Master Volume Control
  
---------------------------------------------------------------*/

#define INITGUID
#include "../config.h"
#include <windows.h>

#if TC_ENABLE_VOLUME


#define SAFE_RELEASE(punk) \
		if((punk) != NULL) { (punk)->Release(); (punk) = NULL; }


DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C,
	0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35,
	0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
DEFINE_GUID(IID_IAudioEndpointVolume, 0x5CDF2C82, 0x841E, 0x4546,
	0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A);

/* mmdeviceapi.h */

typedef enum {
	eRender = 0, eCapture, eAll, EDataFlow_enum_count
} EDataFlow;

typedef enum {
	eConsole = 0, eMultimedia, eCommunications, ERole_enum_count
} ERole;

typedef void IPropertyStore;
typedef void IMMDeviceCollection;
typedef void IMMNotificationClient;
typedef void IAudioEndpointVolumeCallback;

MIDL_INTERFACE("D666063F-1587-4E43-81F1-B948E807363F")
IMMDevice : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Activate(
		REFIID iid, DWORD dwClsCtx,
		PROPVARIANT *pActivationParams,
		void **ppInterface) = 0;
	virtual HRESULT STDMETHODCALLTYPE OpenPropertyStore(
		DWORD stgmAccess, IPropertyStore **ppProperties) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetId(LPWSTR *ppstrId) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetState(DWORD *pdwState) = 0;
};

MIDL_INTERFACE("A95664D2-9614-4F35-A746-DE8DB63617E6")
IMMDeviceEnumerator : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumAudioEndpoints(
		EDataFlow dataFlow, DWORD dwStateMask,
		IMMDeviceCollection **ppDevices) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDefaultAudioEndpoint(
		EDataFlow dataFlow, ERole role, IMMDevice **ppEndpoint) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDevice(
		LPCWSTR pwstrId, IMMDevice **ppDevice) = 0;
	virtual HRESULT STDMETHODCALLTYPE RegisterEndpointNotificationCallback(
		IMMNotificationClient *pClient) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterEndpointNotificationCallback(
		IMMNotificationClient *pClient) = 0;
};

/* endpointvolume.h */

MIDL_INTERFACE("5CDF2C82-841E-4546-9722-0CF74078229A")
IAudioEndpointVolume : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE RegisterControlChangeNotify(
		IAudioEndpointVolumeCallback *pNotify) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterControlChangeNotify(
		IAudioEndpointVolumeCallback *pNotify) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetChannelCount(
		UINT *pnChannelCount) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMasterVolumeLevel(
		float fLevelDB, const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMasterVolumeLevelScalar(
		float fLevel, const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetMasterVolumeLevel(
		float *pfLevelDB) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetMasterVolumeLevelScalar(
		float *pfLevel) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetChannelVolumeLevel(
		UINT nChannel, float fLevelDB, const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetChannelVolumeLevelScalar(
		UINT nChannel, float fLevel, const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetChannelVolumeLevel(
		UINT nChannel, float *pfLevelDB) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetChannelVolumeLevelScalar(
		UINT nChannel, float *pfLevel) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMute(
		BOOL bMute, const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetMute(
		BOOL *pbMute) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetVolumeStepInfo(
		UINT *pnStep, UINT *pnStepCount) = 0;
	virtual HRESULT STDMETHODCALLTYPE VolumeStepUp(
		const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE VolumeStepDown(
		const GUID *pguidEventContext) = 0;
	virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport(
		DWORD *pdwHardwareSupportMask) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetVolumeRange(
		float *pflVolumeMindB,
		float *pflVolumeMaxdB,
		float *pflVolumeIncrementdB) = 0;
};

IAudioEndpointVolume* GetDefaultAudioEndpointVolume()
{
	HRESULT hr;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pEndpoint = NULL;
	IAudioEndpointVolume* pAudioEndVol = NULL;
	
	// オーディオデバイス一覧取得用のインターフェースを取得
	hr = CoCreateInstance(
			CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
			IID_IMMDeviceEnumerator, (void**) &pEnumerator);
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
