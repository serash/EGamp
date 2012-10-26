// This is the main DLL file.

#include "stdafx.h"
#include "AudioLibrary.h"


/* ==========================
   ===    AUDIOLIBRARY    ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::AudioEngine::AudioEngine(UINT32 engineLatency_, bool enableStreamSwitch_)
{
	innerEngine = new InnerAudioEngine(engineLatency_, enableStreamSwitch_);
}

HRESULT AudioLibrary::AudioEngine::initialize() 
{
	HRESULT hr = S_OK;
	hr = innerEngine->initialize();
	if(FAILED(hr)) return hr;
	captureDevicesList = innerEngine->makeCaptureDeviceList();
	renderDevicesList = innerEngine->makeRenderDeviceList();

	return hr;
}

void AudioLibrary::AudioEngine::dispose()
{
	innerEngine->dispose();
}

array<String^> ^AudioLibrary::AudioEngine::getRenderDevices() 
{
	return renderDevicesList;
}

array<String^> ^AudioLibrary::AudioEngine::getCaptureDevices() 
{
	return captureDevicesList;
}

HRESULT AudioLibrary::AudioEngine::setRenderDevice(UINT num)
{
	return innerEngine->setRenderDevice(num);
}

HRESULT AudioLibrary::AudioEngine::setDefaultRenderDevice()
{
	return innerEngine->setDefaultRenderDevice();
}

HRESULT AudioLibrary::AudioEngine::setCaptureDevice(UINT num)
{
	return innerEngine->setCaptureDevice(num);
}

HRESULT AudioLibrary::AudioEngine::setDefaultCaptureDevice()
{
	return innerEngine->setDefaultCaptureDevice();
}

HRESULT AudioLibrary::AudioEngine::startAudioStream()
{
	return innerEngine->startAudioStream();
}

HRESULT AudioLibrary::AudioEngine::stopAudioStream()
{
	return innerEngine->stopAudioStream();
}

HRESULT AudioLibrary::AudioEngine::volumeUp(float fLevel)
{
	return innerEngine->volumeUp(fLevel);
}

HRESULT AudioLibrary::AudioEngine::volumeDown(float fLevel)
{
	return innerEngine->volumeDown(fLevel);
}

HRESULT AudioLibrary::AudioEngine::setVolume(float fLevel)
{
	return innerEngine->setVolume(fLevel);
}

HRESULT AudioLibrary::AudioEngine::toggleMute()
{
	return innerEngine->toggleMute();
}


/* ==========================
   ===    AUDIOSTREAM     ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::AudioStream::AudioStream() 
{
	renderFormat = NULL;
	captureFormat = NULL;
	buffer = NULL;
	bufSize = 0;
	currentFlags = 0;
}

AudioLibrary::AudioStream::~AudioStream() 
{
	dispose();
}

HRESULT AudioLibrary::AudioStream::initialize(UINT32 numCaptureFrames_, UINT32 numRenderFrames_)
{
	// 2 buffers, one for storing and one for loading (keeps swicthing)
	bufSize = max(numCaptureFrames_,numRenderFrames_);
	dataIsReady = 0;
	if(!renderFormat) return CUSTOM_E_FORMAT_NOT_SET;
	if(!captureFormat) return CUSTOM_E_FORMAT_NOT_SET;
	storedDataId = -1;
	numAvailableFrames = 0;
	if(buffer) {
		free(buffer[0]);
		free(buffer[1]);
	} else {
		buffer = (BYTE **)malloc(sizeof(BYTE *)*bufSize);
	}
	buffer[0] = (BYTE*) malloc(sizeof(BYTE)*bufSize*renderFormat->nBlockAlign);
	buffer[1] = (BYTE*) malloc(sizeof(BYTE)*bufSize*captureFormat->nBlockAlign);

	DWORD LCMSampleRate = LeastCommonMultiple(renderFormat->nSamplesPerSec, captureFormat->nSamplesPerSec);
	renderSamplePos = LCMSampleRate / renderFormat->nBlockAlign;
	captureSamplePos = LCMSampleRate / captureFormat->nBlockAlign;
	
	return S_OK;
}

HRESULT AudioLibrary::AudioStream::setRenderFormat(WAVEFORMATEX *renderFormat_)
{
	if(renderFormat) {
        CoTaskMemFree(renderFormat);
        renderFormat = NULL;
	}
	renderFormat = (WAVEFORMATEX*) CoTaskMemAlloc(sizeof(WAVEFORMATEX) + renderFormat_->cbSize);
	memcpy(renderFormat, renderFormat_, sizeof(WAVEFORMATEX) + renderFormat_->cbSize);
	return S_OK;
}

HRESULT AudioLibrary::AudioStream::setCaptureFormat(WAVEFORMATEX *captureFormat_)
{
	if(captureFormat) {
        CoTaskMemFree(captureFormat);
        captureFormat = NULL;
	}
	captureFormat = (WAVEFORMATEX*) CoTaskMemAlloc(sizeof(WAVEFORMATEX) + captureFormat_->cbSize);
	memcpy(captureFormat, captureFormat_, sizeof(WAVEFORMATEX) + captureFormat_->cbSize);
	return S_OK;
}

WAVEFORMATEX *AudioLibrary::AudioStream::getRenderFormat()
{
	return renderFormat;
}

WAVEFORMATEX *AudioLibrary::AudioStream::getCaptureFormat()
{
	return captureFormat;
}

// TODO fix the store / load data to buffer data if not all is read/stored
HRESULT AudioLibrary::AudioStream::storeNullData(UINT32 numFramesAvailable)
{
	UINT32 numBytesAvailable = numFramesAvailable*captureFormat->nBlockAlign;
	if(numFramesAvailable > bufSize)
		return E_INVALIDARG;
	
	// store zeros in buffer
	
	storedDataId = (storedDataId + 1) % 2;
	SecureZeroMemory(buffer[storedDataId], numBytesAvailable);
	numAvailableFrames = numBytesAvailable;
	convertWaveFormat();

	return S_OK;
}

HRESULT AudioLibrary::AudioStream::storeData(const BYTE *pData, UINT32 numFramesAvailable)
{
	UINT32 numBytesAvailable = numFramesAvailable*captureFormat->nBlockAlign;
	if(numFramesAvailable > bufSize)
		return E_INVALIDARG;
	
	// store from pData
	storedDataId = (storedDataId + 1) % 2;
	memcpy(buffer[storedDataId], pData, numBytesAvailable); 
	numAvailableFrames = numBytesAvailable;
	convertWaveFormat();

    //captureWriter->writeData(buffer[storedDataId], numBytesAvailable);
	return S_OK;
}

HRESULT AudioLibrary::AudioStream::loadData(BYTE *pData, UINT32 numFramesAvailable, DWORD *flags)
{
	UINT32 numBytesAvailable = numFramesAvailable*renderFormat->nBlockAlign;
	if(numFramesAvailable > bufSize)
		return E_INVALIDARG;

	// load to pData
	memcpy(pData, buffer[storedDataId], numBytesAvailable); 
	dataIsReady = 0;
	storedDataId = (storedDataId + 1) % 2;
	
	// set flags
	if(numBytesAvailable == 0) currentFlags |= AUDCLNT_BUFFERFLAGS_SILENT;
	(*flags) |= currentFlags;

    //renderWriter->writeData(pData, numBytesAvailable);
	return S_OK;

}

UINT32 AudioLibrary::AudioStream::getAvailableFrames() 
{
	if (dataIsReady == 1)
		return numAvailableFrames / renderFormat->nBlockAlign;
	else 
		return 0;
}

void AudioLibrary::AudioStream::openWriter()
{
	captureWriter = new WaveFileWriter("capture.wav", captureFormat);
	captureWriter->open();
	renderWriter = new WaveFileWriter("render.wav", renderFormat);
	renderWriter->open();
}

void AudioLibrary::AudioStream::closeWriter() 
{
	captureWriter->close();
	renderWriter->close();
}

void AudioLibrary::AudioStream::dispose() 
{
	if(buffer) {
		free(buffer[0]);
		free(buffer[1]);
		free(buffer);
		buffer = NULL;
	}
	if (captureFormat)
    {
        CoTaskMemFree(captureFormat);
        captureFormat = NULL;
    }
	if (renderFormat)
    {
        CoTaskMemFree(renderFormat);
        renderFormat = NULL;
    }
}

DWORD AudioLibrary::AudioStream::convertWaveFormat()
{
	HRESULT hr = S_OK;
	if(renderSamplePos != captureSamplePos)
		hr = resample<DWORD>();
	dataIsReady = 1;
	return 0;
}

template <class SAMPLETYPE>  HRESULT AudioLibrary::AudioStream::resample()
{
	HRESULT hr = S_OK;
	UINT32 captureIdx = 1;
	UINT32 captureCount = captureSamplePos;
	UINT32 totalCaptures = numAvailableFrames*captureFormat->nBlockAlign;
	UINT32 renderCount = 0;
	// reinterpret cast here to valid size of the numbers
	SAMPLETYPE *source = reinterpret_cast<SAMPLETYPE *>(buffer[storedDataId]);
	SAMPLETYPE *destination = reinterpret_cast<SAMPLETYPE *>(buffer[storedDataId]);
	for(UINT32 i = 0; i < numAvailableFrames; i+=renderFormat->nSamplesPerSec) {
		while(captureCount < renderCount && captureIdx < totalCaptures) {
			captureCount += captureSamplePos;
			captureIdx++;
		}
		destination[i] = LinearInterpolation<DWORD>(source[captureIdx-1], source[captureIdx], ((float)(renderCount%captureSamplePos))/captureSamplePos);
		// TODO APPLY EFFECTS IN HERE FOR destination[i] AND destination[i+1]
		renderCount += 147;
	}
	return hr;
}

DWORD AudioLibrary::AudioStream::LeastCommonMultiple(DWORD rate1, DWORD rate2)
{
	DWORD gcd = GreatestCommonDivisor(rate1, rate2);
	return ( rate1 * rate2 ) / gcd;
}

DWORD AudioLibrary::AudioStream::GreatestCommonDivisor(DWORD rate1, DWORD rate2)
{
	if(rate1 == rate2)
		return rate1;
	else if (rate2 < rate1)
		return GreatestCommonDivisor(rate1 - rate2, rate2);
	else
		return GreatestCommonDivisor(rate1, rate2 - rate1);
}


/* ==========================
   ===   WAVEFILEWRITER   ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::WaveFileWriter::WaveFileWriter(const char *filename_, WAVEFORMATEX *waveFormat_)
{
	filename = std::string(filename_);
	waveFormat = (WAVEFORMATEX*) malloc(sizeof(WAVEFORMATEX) + waveFormat_->cbSize);
	memcpy(waveFormat, waveFormat_, sizeof(WAVEFORMATEX) + waveFormat_->cbSize);
	waveFileSize = 0;
}

void AudioLibrary::WaveFileWriter::open()
{
	myfile.open (filename, std::ios::out | std::ios::binary);
	headerSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + waveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD);
	waveFileSize = headerSize;
	
	if(myfile.is_open()) {
		char *temp = (char*)malloc(sizeof(char)*headerSize);
		myfile.write(temp, headerSize);
		free(temp);
	}
}

bool AudioLibrary::WaveFileWriter::writeData(const BYTE* data, UINT32 dataSize)
{
	if(myfile.is_open()) {
		myfile.write((const char*)data, dataSize);
		waveFileSize += dataSize;
		return true;
	} else 
		return false;
}

void AudioLibrary::WaveFileWriter::close()
{
    BYTE *waveFileData = new (std::nothrow) BYTE[headerSize];
    BYTE *waveFilePointer = waveFileData;
    WAVEHEADER *waveHeader = reinterpret_cast<WAVEHEADER *>(waveFileData);

    //  Copy in the wave header - we'll fix up the lengths later.
    CopyMemory(waveFilePointer, WaveHeader, sizeof(WaveHeader));
    waveFilePointer += sizeof(WaveHeader);

    //  Update the sizes in the header.
    waveHeader->dwSize = waveFileSize - (2 * sizeof(DWORD));
    waveHeader->dwFmtSize = sizeof(WAVEFORMATEX) + waveFormat->cbSize;

    //  Next copy in the WaveFormatex structure.
    CopyMemory(waveFilePointer, waveFormat, sizeof(WAVEFORMATEX) + waveFormat->cbSize);
    waveFilePointer += sizeof(WAVEFORMATEX) + waveFormat->cbSize;

    //  Then the data header.
    CopyMemory(waveFilePointer, WaveData, sizeof(WaveData));
    waveFilePointer += sizeof(WaveData);
    *(reinterpret_cast<DWORD *>(waveFilePointer)) = static_cast<DWORD>(waveFileSize - headerSize);
	
	if(myfile.is_open()) {
		myfile.seekp(0, std::ios::beg);
		myfile.write((const char*)waveFileData, headerSize);
	}
	myfile.close();
}


/* ==========================
   ===  INNERAUDIOENGINE  ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::InnerAudioEngine::InnerAudioEngine(UINT32 _EngineLatencyInMS, bool enableStreamSwitch_) 
{
	pCaptureAudioClient = NULL;
	pCaptureClient = NULL;
	pRenderAudioClient = NULL;
	pRenderClient = NULL;
	pCaptureClient = NULL;
	pRenderDevices = NULL;
	pCaptureDevices = NULL;
	pRenderDevice = NULL;
	pCaptureDevice = NULL;
	pAudioSessionControl = NULL;
	pAudioVolume = NULL;
	captureShutdownEvent = NULL;
	renderShutdownEvent = NULL;
	captureSamplesReadyEvent = NULL;
	renderSamplesReadyEvent = NULL;
	streamSwitchEvent = NULL;
	streamSwitchCompleteEvent = NULL;
	captureThread = NULL;
	renderThread = NULL;
	pAudioStream = new AudioStream();
	engineLatency = _EngineLatencyInMS;
	enableStreamSwitch = enableStreamSwitch_;
	inStreamSwitch = false;
	audioStreamStatus = false;
}

HRESULT AudioLibrary::InnerAudioEngine::initialize() 
{
    HRESULT hr = S_OK;
	pin_ptr<IMMDeviceEnumerator *> pinnedEnumeratorPtr = &pEnumerator;

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)pinnedEnumeratorPtr);
	if(FAILED(hr)) return hr;

	pin_ptr<IMMDeviceCollection *> pinnedRenderPtr = &pRenderDevices;
	hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, pinnedRenderPtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IMMDeviceCollection *> pinnedCapturePtr = &pCaptureDevices;
	hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, pinnedCapturePtr);
	if(FAILED(hr)) return hr;
	

	//initialize EventHandle
    captureShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    renderShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    captureSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    renderSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	streamSwitchEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	
    hr = setDefaultRenderDevice();
	if(FAILED(hr)) return hr;

	hr = setDefaultCaptureDevice();
	if(FAILED(hr)) return hr;

	hr = initializeDevices();
	if(FAILED(hr)) return hr;


    if (enableStreamSwitch)
    {
        hr = initializeStreamSwitch();
		if(FAILED(hr)) return hr;
    }

	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::initializeStreamSwitch() 
{
    HRESULT hr = S_OK;
	
	pin_ptr<IAudioSessionControl *> pinnedCaptureAudioClientPtr = &pAudioSessionControl;
    hr = pCaptureAudioClient->GetService(IID_PPV_ARGS(&pAudioSessionControl));
	if(FAILED(hr)) return hr;

    streamSwitchCompleteEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);

    hr = pAudioSessionControl->RegisterAudioSessionNotification(this);
	if(FAILED(hr)) return hr;

    hr = pEnumerator->RegisterEndpointNotificationCallback(this);
	if(FAILED(hr)) return hr;

    return true;
	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::terminateStreamSwitch() 
{
    HRESULT hr = S_OK;
    hr = pAudioSessionControl->UnregisterAudioSessionNotification(this);
    if (FAILED(hr)) return hr;

    pEnumerator->UnregisterEndpointNotificationCallback(this);
    if (FAILED(hr)) return hr;

    if (streamSwitchCompleteEvent)
    {
        CloseHandle(streamSwitchCompleteEvent);
        streamSwitchCompleteEvent = NULL;
    }
	
	pin_ptr<IAudioSessionControl *> pinnedCaptureAudioClientPtr = &pAudioSessionControl;
    safeRelease(reinterpret_cast<IMMDeviceEnumerator **>(pinnedCaptureAudioClientPtr));

	pin_ptr<IMMDeviceEnumerator *> pinnedEnumeratorPtr = &pEnumerator;
    safeRelease(reinterpret_cast<IMMDeviceEnumerator **>(pinnedEnumeratorPtr));

	return hr;
}

void AudioLibrary::InnerAudioEngine::dispose()
{
	if(audioStreamStatus)
		stopAudioStream();

	if(captureShutdownEvent) {
		CloseHandle(captureShutdownEvent);
		captureShutdownEvent = NULL;
	}
	if(renderShutdownEvent) {
		CloseHandle(renderShutdownEvent);
		renderShutdownEvent = NULL;
	}
	if(renderSamplesReadyEvent) {
		CloseHandle(renderSamplesReadyEvent);
		renderSamplesReadyEvent = NULL;
	}
	if(captureSamplesReadyEvent) {
		CloseHandle(captureSamplesReadyEvent);
		captureSamplesReadyEvent = NULL;
	}
	if(streamSwitchEvent) {
		CloseHandle(streamSwitchEvent);
		streamSwitchEvent = NULL;
	}

	pAudioStream->dispose();

	pin_ptr<IAudioClient *> pinnedCaptureAudioClientPtr = &pCaptureAudioClient;
    safeRelease(reinterpret_cast<IAudioClient **>(pinnedCaptureAudioClientPtr));

	pin_ptr<IAudioCaptureClient *> pinnedCaptureClientPtr = &pCaptureClient;
    safeRelease(reinterpret_cast<IAudioCaptureClient **>(pinnedCaptureClientPtr));

	pin_ptr<IAudioClient *> pinnedRenderAudioClientPtr = &pRenderAudioClient;
    safeRelease(reinterpret_cast<IAudioClient **>(pinnedRenderAudioClientPtr));

	pin_ptr<IAudioRenderClient *> pinnedRenderClientPtr = &pRenderClient;
    safeRelease(reinterpret_cast<IAudioRenderClient **>(pinnedRenderClientPtr));

	pin_ptr<IMMDeviceCollection *> pinnedRenderDevicesPtr = &pRenderDevices;
    safeRelease(reinterpret_cast<IMMDeviceCollection **>(pinnedRenderDevicesPtr));

	pin_ptr<IMMDeviceCollection *> pinnedCaptureDevicesPtr = &pCaptureDevices;
    safeRelease(reinterpret_cast<IMMDeviceCollection **>(pinnedCaptureDevicesPtr));

	pin_ptr<IMMDevice *> pinnedRenderDevicePtr = &pRenderDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedRenderDevicePtr));

	pin_ptr<IMMDevice *> pinnedCaptureDevicePtr = &pCaptureDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedCaptureDevicePtr));

    if (enableStreamSwitch)
        terminateStreamSwitch();
    else {
		pin_ptr<IMMDeviceEnumerator *> pinnedEnumeratorPtr = &pEnumerator;
		safeRelease(reinterpret_cast<IMMDeviceEnumerator **>(pinnedEnumeratorPtr));
	}
}

array<String^> ^AudioLibrary::InnerAudioEngine::makeRenderDeviceList()
{
    HRESULT hr = S_OK;
	UINT pcDevices = 0;
	IMMDevice *ppDevice = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR ppStrId = NULL;
	UINT i = 0;
	PROPVARIANT varName;

	hr = pRenderDevices->GetCount(&pcDevices);
	
	array<String^> ^renderDevicesList = gcnew array<String^>(pcDevices);
    PropVariantInit(&varName);

	for each(String^% s in renderDevicesList) {
		hr = pRenderDevices->Item(i, &ppDevice);
		if(FAILED(hr)) return renderDevicesList;

		hr = ppDevice->OpenPropertyStore(STGM_READ, &pProps);
		if(FAILED(hr)) return renderDevicesList;

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if(!SUCCEEDED(hr)) return renderDevicesList;

		std::wstring name (varName.pwszVal);
		s = gcnew String(name.c_str());
		i++;
	}

    safeRelease(&ppDevice);
	safeRelease(&pProps);
	CoTaskMemFree(ppStrId);
	return renderDevicesList;
}

array<String^> ^ AudioLibrary::InnerAudioEngine::makeCaptureDeviceList()
{
    HRESULT hr = S_OK;
	UINT pcDevices = 0;
	IMMDevice *ppDevice = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR ppStrId = NULL;
	UINT i = 0;
	PROPVARIANT varName;

	hr = pCaptureDevices->GetCount(&pcDevices);
	
	array<String^> ^captureDevicesList = gcnew array<String^>(pcDevices);
    PropVariantInit(&varName);

	for each(String^% s in captureDevicesList) {
		hr = pCaptureDevices->Item(i, &ppDevice);
		if(FAILED(hr)) return captureDevicesList;

		hr = ppDevice->OpenPropertyStore(STGM_READ, &pProps);
		if(FAILED(hr)) return captureDevicesList;

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if(!SUCCEEDED(hr)) return captureDevicesList;

		std::wstring name (varName.pwszVal);
		s = gcnew String(name.c_str());
		i++;
	}

    safeRelease(&ppDevice);
	safeRelease(&pProps);
	CoTaskMemFree(ppStrId);
	return captureDevicesList;
}

HRESULT AudioLibrary::InnerAudioEngine::setRenderDevice(UINT num)
{
    HRESULT hr = S_OK;
	UINT pcDevices;

	pin_ptr<IMMDevice *> pinnedRenderDevicePtr = &pRenderDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedRenderDevicePtr));

	hr = pRenderDevices->GetCount(&pcDevices);
	if(FAILED(hr)) return hr;
	if(num < 0 || num >= pcDevices) return E_INVALIDARG;
	
	hr = pRenderDevices->Item(num, pinnedRenderDevicePtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioClient *> pinnedRenderAudioClientPtr = &pRenderAudioClient;
    safeRelease(reinterpret_cast<IAudioClient **>(pinnedRenderAudioClientPtr));
	hr = pRenderDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedRenderAudioClientPtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::setDefaultRenderDevice()
{
    HRESULT hr = S_OK;
	WAVEFORMATEX *waveFormat;

	pin_ptr<IMMDevice *> pinnedRenderDevicePtr = &pRenderDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedRenderDevicePtr));
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, pinnedRenderDevicePtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioClient *> pinnedRenderAudioClientPtr = &pRenderAudioClient;
    safeRelease(reinterpret_cast<IAudioClient **>(pinnedRenderAudioClientPtr));
	hr = pRenderDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedRenderAudioClientPtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::setCaptureDevice(UINT num)
{
    HRESULT hr = S_OK;
	WAVEFORMATEX *waveFormat;
	UINT pcDevices;

	pin_ptr<IMMDevice *> pinnedCaptureDevicePtr = &pCaptureDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedCaptureDevicePtr));

	hr = pCaptureDevices->GetCount(&pcDevices);
	if(FAILED(hr)) return hr;
	if(num < 0 || num >= pcDevices) return E_INVALIDARG;
	
	hr = pCaptureDevices->Item(num, pinnedCaptureDevicePtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioClient *> pinnedCaptureAudioClientPtr = &pCaptureAudioClient;
    safeRelease(reinterpret_cast<IAudioClient **>(pinnedCaptureAudioClientPtr));
	hr = pCaptureDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedCaptureAudioClientPtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::setDefaultCaptureDevice()
{
    HRESULT hr = S_OK;
	WAVEFORMATEX *waveFormat;
	
	pin_ptr<IMMDevice *> pinnedCaptureDevicePtr = &pCaptureDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedCaptureDevicePtr));
	hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, pinnedCaptureDevicePtr);
	if(FAILED(hr)) return hr;

	pin_ptr<IAudioClient *> pinnedCaptureAudioClientPtr = &pCaptureAudioClient;
    safeRelease(reinterpret_cast<IAudioClient **>(pinnedCaptureAudioClientPtr));
	hr = pCaptureDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedCaptureAudioClientPtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::initializeDevices()
{
    HRESULT hr = S_OK;
	// set WaveFormat for both devices!
	hr = setWaveFormats();
	if(FAILED(hr)) return hr;

	// INIT CAPTURE DEVICE
    hr = pCaptureAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
		engineLatency*10000, 0, pAudioStream->getCaptureFormat(), NULL); // latency in 100 nano seconds
	if(FAILED(hr)) return hr;

	// set EventHandle
	hr = pCaptureAudioClient->SetEventHandle(captureSamplesReadyEvent);
	if(FAILED(hr)) return hr;
	
    // Get the size of the allocated buffer.
	pin_ptr<UINT32> pinnedCaptureBufferFrameCountPtr = &captureBufferFrameCount;
    hr = pCaptureAudioClient->GetBufferSize(pinnedCaptureBufferFrameCountPtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioCaptureClient *> pinnedCaptureClientPtr = &pCaptureClient;
    safeRelease(reinterpret_cast<IAudioCaptureClient **>(pinnedCaptureClientPtr));
    hr = pCaptureAudioClient->GetService(IID_IAudioCaptureClient, (void**)pinnedCaptureClientPtr);
	if(FAILED(hr)) return hr;

	// INIT RENDER DEVICE
    hr = pRenderAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
		engineLatency*10000, 0, pAudioStream->getRenderFormat(), NULL);
	if(FAILED(hr)) return hr;

	// set EventHandle
	hr = pRenderAudioClient->SetEventHandle(renderSamplesReadyEvent);
	if(FAILED(hr)) return hr;

    // Get the actual size of the allocated buffer.
	pin_ptr<UINT32> pinnedRenderBufferFrameCountPtr = &renderBufferFrameCount;
    hr = pRenderAudioClient->GetBufferSize(pinnedRenderBufferFrameCountPtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioRenderClient *> pinnedRenderClientPtr = &pRenderClient;
    safeRelease(reinterpret_cast<IAudioRenderClient **>(pinnedRenderClientPtr));
    hr = pRenderAudioClient->GetService(IID_IAudioRenderClient, (void**)pinnedRenderClientPtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<ISimpleAudioVolume *> pinnedAudioVolumePtr = &pAudioVolume;
	safeRelease(reinterpret_cast<ISimpleAudioVolume **>(pinnedAudioVolumePtr));
    hr = pRenderAudioClient->GetService(IID_ISimpleAudioVolume, (void**)pinnedAudioVolumePtr);
	if(FAILED(hr)) return hr;
	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::setWaveFormats()
{
    HRESULT hr = S_OK;
	WAVEFORMATEX *renderWaveFormat, *captureWaveFormat, *possibleRenderFormat, *possibleCaptureFormat;
	
    hr = pRenderAudioClient->GetMixFormat(&renderWaveFormat);
	if(FAILED(hr)) return hr;

	hr = pRenderAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, renderWaveFormat, &possibleRenderFormat);
	if(hr == S_FALSE)
		hr = pAudioStream->setRenderFormat(possibleRenderFormat);
	else if(SUCCEEDED(hr))
		hr = pAudioStream->setRenderFormat(renderWaveFormat);
	else return hr;
	renderWaveFormat = pAudioStream->getRenderFormat();
	
    hr = pCaptureAudioClient->GetMixFormat(&captureWaveFormat);
	if(FAILED(hr)) return hr;

	hr = pCaptureAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, renderWaveFormat, &possibleCaptureFormat);
	if(hr == S_FALSE) {
		hr = pCaptureAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, captureWaveFormat, &possibleCaptureFormat);
		if(hr == S_FALSE)
			hr = pAudioStream->setCaptureFormat(possibleCaptureFormat);
		else if(SUCCEEDED(hr))
			hr = pAudioStream->setCaptureFormat(captureWaveFormat);
		else return hr;
	} else if(SUCCEEDED(hr))
		hr = pAudioStream->setCaptureFormat(renderWaveFormat);
	else return hr;


	if(FAILED(hr)) return hr;
	return hr;
}

DWORD AudioLibrary::InnerAudioEngine::captureAudioStream()
{
    HRESULT hr = S_OK;
    bool stillPlaying = true;
    HANDLE waitArray[3] = {captureShutdownEvent, streamSwitchEvent, captureSamplesReadyEvent };
    BYTE *pData;
    UINT32 framesAvailable;
    DWORD  flags;

    while (stillPlaying)
    {
        DWORD waitResult = WaitForMultipleObjects(3, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // shutdownEvent
            stillPlaying = false;   // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // _StreamSwitchEvent
            //
            //  We need to stop the capturer, tear down the _AudioClient and _CaptureClient objects and re-create them on the new.
            //  endpoint if possible.  If this fails, abort the thread.
            //
            if (!handleStreamSwitchEvent())
            {
                stillPlaying = false;
            }
            break;
        case WAIT_OBJECT_0 + 2:		// captureSamplesReadyEvent
            hr = pCaptureClient->GetBuffer(&pData, &framesAvailable, &flags, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                if (framesAvailable != 0)
                {
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
						pAudioStream->storeNullData(framesAvailable);
                    else {
						hr = pAudioStream->storeData(pData, framesAvailable);
					}
                }
                hr = pCaptureClient->ReleaseBuffer(framesAvailable);
                if (FAILED(hr))
                    printf("Unable to release capture buffer: %x!\n", hr);
            }
            break;
        }
    }
	return 0;
}

DWORD AudioLibrary::InnerAudioEngine::renderAudioStream()
{
    HRESULT hr = S_OK;
    bool stillPlaying = true;
    HANDLE waitArray[2] = {renderShutdownEvent, renderSamplesReadyEvent};
    BYTE *pData;
    UINT32 framesAvailable;
    UINT32 padding;
	DWORD flags = 0;

    while (stillPlaying)
    {
        DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // shutdownEvent
            stillPlaying = false;   // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // renderSamplesReadyEvent
            hr = pRenderAudioClient->GetCurrentPadding(&padding);
            if (SUCCEEDED(hr))
            {
                framesAvailable = renderBufferFrameCount - padding;
                UINT32 framesToWrite = min(pAudioStream->getAvailableFrames() , framesAvailable);
				if(framesToWrite > 0) {
					hr = pRenderClient->GetBuffer(framesToWrite, &pData);
					if (SUCCEEDED(hr))
					{
						pAudioStream->loadData(pData, framesToWrite, &flags);
						hr = pRenderClient->ReleaseBuffer(framesToWrite, flags);
						if (!SUCCEEDED(hr))
						{
							printf("Unable to release buffer: %x\n", hr);
							stillPlaying = false;
						}
					}
				} else if (padding == 0 && pAudioStream->getAvailableFrames() == 0) { // no data available but data needed, feed 0 data
					hr = pRenderClient->GetBuffer(framesAvailable, &pData);
					if (SUCCEEDED(hr))
					{
						SecureZeroMemory(pData, framesAvailable);
						hr = pRenderClient->ReleaseBuffer(framesAvailable, flags);
						if (!SUCCEEDED(hr))
						{
							printf("Unable to release buffer: %x\n", hr);
							stillPlaying = false;
						}
					}

				}
            }
            break;
        }
    }
	return 0;
}

DWORD AudioLibrary::InnerAudioEngine::captureThreadFunction(LPVOID Context)
{
    InnerAudioEngine *engine = static_cast<InnerAudioEngine *>(Context);
	return engine->captureAudioStream();
}

DWORD AudioLibrary::InnerAudioEngine::renderThreadFunction(LPVOID Context)
{
    InnerAudioEngine *engine = static_cast<InnerAudioEngine *>(Context);
	return engine->renderAudioStream();
}

HRESULT AudioLibrary::InnerAudioEngine::startAudioStream()
{
    HRESULT hr = S_OK;
	
	pAudioStream->initialize(captureBufferFrameCount, renderBufferFrameCount);
	pAudioStream->openWriter();
	
	// start threads:
    captureThread = CreateThread(NULL, 0, captureThreadFunction, this, 0, NULL);
	renderThread = CreateThread(NULL, 0, renderThreadFunction, this, 0, NULL);

	hr = pCaptureAudioClient->Start();  // Start recording.
	if(FAILED(hr)) return hr;
	hr = pRenderAudioClient->Start();  // Start playing.
	if(FAILED(hr)) return hr;

	audioStreamStatus = true;
	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::stopAudioStream()
{
    HRESULT hr = S_OK;
	// make event to shut down the threads
	if(captureShutdownEvent) {
		SetEvent(captureShutdownEvent);
	}
	if(renderShutdownEvent) {
		SetEvent(renderShutdownEvent);
	}
	
    hr = pCaptureAudioClient->Stop();  // Stop recording.
	if(FAILED(hr)) return hr;
    hr = pRenderAudioClient->Stop();  // Stop playing.
	if(FAILED(hr)) return hr;
	pAudioStream->closeWriter();

	// Abort any thread that is still alive after shutdown
    if (captureThread)
    {
        WaitForSingleObject(captureThread, INFINITE);
        CloseHandle(captureThread);
        captureThread = NULL;
    }
    if (renderThread)
    {
        WaitForSingleObject(renderThread, INFINITE);
        CloseHandle(renderThread);
        renderThread = NULL;
    }

	audioStreamStatus = false;
	return true;
}

HRESULT AudioLibrary::InnerAudioEngine::volumeUp(float fLevel)
{
    HRESULT hr = S_OK;
	
	float res;
	hr = pAudioVolume->GetMasterVolume(&res);
	if(FAILED(hr)) return hr;

	res += fLevel;
	if(res > 1.0f) res = 1.0f;

	hr = pAudioVolume->SetMasterVolume(res, NULL);
	if(FAILED(hr)) return hr;

	return res;
}

HRESULT AudioLibrary::InnerAudioEngine::volumeDown(float fLevel)
{
    HRESULT hr = S_OK;
	
	float res;
	hr = pAudioVolume->GetMasterVolume(&res);
	if(FAILED(hr)) return hr;

	res -= fLevel;
	if(res < 0.0f) res = 0.0f;

	hr = pAudioVolume->SetMasterVolume(res, NULL);
	if(FAILED(hr)) return hr;

	return res;
}

HRESULT AudioLibrary::InnerAudioEngine::setVolume(float fLevel)
{
    HRESULT hr = S_OK;
	
	float res = fLevel;
	if(res < 0.0f) res = 0.0f;
	if(res > 1.0f) res = 1.0f;
	hr = pAudioVolume->SetMasterVolume(res, NULL);
	if(FAILED(hr)) return hr;

	return res;
}

HRESULT AudioLibrary::InnerAudioEngine::toggleMute()
{
    HRESULT hr = S_OK;
	
	BOOL mute;
	hr = pAudioVolume->GetMute(&mute);
	if(FAILED(hr)) return hr;

	hr = pAudioVolume->SetMute(!mute, NULL);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::InnerAudioEngine::handleStreamSwitchEvent()
{
//    HRESULT hr;
//
//    assert(inStreamSwitch);
//    //
//    //  Step 1.  Stop capturing.
//    //
//    hr = pCaptureAudioClient->Stop();
//    if (FAILED(hr))
//    {
//        printf("Unable to stop audio client during stream switch: %x\n", hr);
//        goto ErrorExit;
//    }
//
//    //
//    //  Step 2.  Release our resources.  Note that we don't release the mix format, we need it for step 6.
//    //
//    hr = _AudioSessionControl->UnregisterAudioSessionNotification(this);
//    if (FAILED(hr))
//    {
//        printf("Unable to stop audio client during stream switch: %x\n", hr);
//        goto ErrorExit;
//    }
//
//    SafeRelease(&_AudioSessionControl);
//    SafeRelease(&_CaptureClient);
//    SafeRelease(&_AudioClient);
//    SafeRelease(&_Endpoint);
//
//    //
//    //  Step 3.  Wait for the default device to change.
//    //
//    //  There is a race between the session disconnect arriving and the new default device 
//    //  arriving (if applicable).  Wait the shorter of 500 milliseconds or the arrival of the 
//    //  new default device, then attempt to switch to the default device.  In the case of a 
//    //  format change (i.e. the default device does not change), we artificially generate  a
//    //  new default device notification so the code will not needlessly wait 500ms before 
//    //  re-opening on the new format.  (However, note below in step 6 that in this SDK 
//    //  sample, we are unlikely to actually successfully absorb a format change, but a 
//    //  real audio application implementing stream switching would re-format their 
//    //  pipeline to deliver the new format).  
//    //
//    DWORD waitResult = WaitForSingleObject(_StreamSwitchCompleteEvent, 500);
//    if (waitResult == WAIT_TIMEOUT)
//    {
//        printf("Stream switch timeout - aborting...\n");
//        goto ErrorExit;
//    }
//
//    //
//    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
//    //          we should be able to retrieve it.
//    //
//    hr = _DeviceEnumerator->GetDefaultAudioEndpoint(eCapture, _EndpointRole, &_Endpoint);
//    if (FAILED(hr))
//    {
//        printf("Unable to retrieve new default device during stream switch: %x\n", hr);
//        goto ErrorExit;
//    }
//    //
//    //  Step 5 - Re-instantiate the audio client on the new endpoint.
//    //
//    hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
//    if (FAILED(hr))
//    {
//        printf("Unable to activate audio client on the new endpoint: %x.\n", hr);
//        goto ErrorExit;
//    }
//    //
//    //  Step 6 - Retrieve the new mix format.
//    //
//    WAVEFORMATEX *wfxNew;
//    hr = _AudioClient->GetMixFormat(&wfxNew);
//    if (FAILED(hr))
//    {
//        printf("Unable to retrieve mix format for new audio client: %x.\n", hr);
//        goto ErrorExit;
//    }
//
//    //
//    //  Note that this is an intentionally naive comparison.  A more sophisticated comparison would
//    //  compare the sample rate, channel count and format and apply the appropriate conversions into the capture pipeline.
//    //
//    if (memcmp(_MixFormat, wfxNew, sizeof(WAVEFORMATEX) + wfxNew->cbSize) != 0)
//    {
//        printf("New mix format doesn't match old mix format.  Aborting.\n");
//        CoTaskMemFree(wfxNew);
//        goto ErrorExit;
//    }
//    CoTaskMemFree(wfxNew);
//
//    //
//    //  Step 7:  Re-initialize the audio client.
//    //
//    if (!InitializeAudioEngine())
//    {
//        goto ErrorExit;
//    }
//
//    //
//    //  Step 8: Re-register for session disconnect notifications.
//    //
//    hr = _AudioClient->GetService(IID_PPV_ARGS(&_AudioSessionControl));
//    if (FAILED(hr))
//    {
//        printf("Unable to retrieve session control on new audio client: %x\n", hr);
//        goto ErrorExit;
//    }
//    hr = _AudioSessionControl->RegisterAudioSessionNotification(this);
//    if (FAILED(hr))
//    {
//        printf("Unable to retrieve session control on new audio client: %x\n", hr);
//        goto ErrorExit;
//    }
//
//    //
//    //  Reset the stream switch complete event because it's a manual reset event.
//    //
//    ResetEvent(_StreamSwitchCompleteEvent);
//    //
//    //  And we're done.  Start capturing again.
//    //
//    hr = _AudioClient->Start();
//    if (FAILED(hr))
//    {
//        printf("Unable to start the new audio client: %x\n", hr);
//        goto ErrorExit;
//    }
//
//    inStreamSwitch = false;
//    return true;
//
//ErrorExit:
//    inStreamSwitch = false;
//    return false;
	return true;
}

HRESULT AudioLibrary::InnerAudioEngine::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
    if (DisconnectReason == DisconnectReasonDeviceRemoval)
    {
        //
        //  The stream was disconnected because the device we're capturing to was removed.
        //
        //  We want to reset the stream switch complete event (so we'll block when the HandleStreamSwitchEvent function
        //  waits until the default device changed event occurs).
        //
        //  Note that we don't set the _StreamSwitchCompleteEvent - that will be set when the OnDefaultDeviceChanged event occurs.
        //
        inStreamSwitch = true;
        SetEvent(streamSwitchEvent);
    }
    if (DisconnectReason == DisconnectReasonFormatChanged)
    {
        //
        //  The stream was disconnected because the format changed on our capture device.
        //
        //  We want to flag that we're in a stream switch and then set the stream switch event (which breaks out of the capturer).  We also
        //  want to set the _StreamSwitchCompleteEvent because we're not going to see a default device changed event after this.
        //
        inStreamSwitch = true;
        SetEvent(streamSwitchEvent);
        SetEvent(streamSwitchCompleteEvent);
    }
    return S_OK;
}

HRESULT AudioLibrary::InnerAudioEngine::OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR /*NewDefaultDeviceId*/)
{
    if (Flow == eCapture || Flow == eRender)
    {
        //
        //  The default capture/render device for our was changed.  
        //
        //  If we're not in a stream switch already, we want to initiate a stream switch event.  
        //  We also we want to set the stream switch complete event.  That will signal the capture thread that it's ok to re-initialize the
        //  audio capturer.
        //
        if (!inStreamSwitch)
        {
            inStreamSwitch = true;
            SetEvent(streamSwitchEvent);
        }
        SetEvent(streamSwitchCompleteEvent);
    }
    return S_OK;
}

HRESULT AudioLibrary::InnerAudioEngine::QueryInterface(REFIID Iid, void **Object)
{
    if (Object == NULL)
    {
        return E_POINTER;
    }
    *Object = NULL;

    if (Iid == IID_IUnknown)
    {
        *Object = static_cast<IUnknown *>(static_cast<IAudioSessionEvents *>(this));
        AddRef();
    }
    else if (Iid == __uuidof(IMMNotificationClient))
    {
        *Object = static_cast<IMMNotificationClient *>(this);
        AddRef();
    }
    else if (Iid == __uuidof(IAudioSessionEvents))
    {
        *Object = static_cast<IAudioSessionEvents *>(this);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }
    return S_OK;
}

ULONG AudioLibrary::InnerAudioEngine::AddRef()
{
    return InterlockedIncrement(&_RefCount);
}

ULONG AudioLibrary::InnerAudioEngine::Release()
{
    ULONG returnValue = InterlockedDecrement(&_RefCount);
    if (returnValue == 0)
    {
        delete this;
    }
    return returnValue;
}


/* ==========================
   ===        ERROR       ===
   ===        CODES       ===
   ========================== */

bool AudioLibrary::AudioEngine::Failed(HRESULT hres)
{
	return hres < 0;
}

String ^AudioLibrary::AudioEngine::getErrorCode(HRESULT hres)
{
	return gcnew String(getErrorCodeString(hres).c_str());
}

std::string AudioLibrary::AudioEngine::getErrorCodeString(HRESULT hres)
{
	switch(hres)
	{
	case(S_OK):
		return "Operation successfully completed.";
	case(INPLACE_S_TRUNCATED):
		return "Operation successfully completed, but truncated.";
	case(REGDB_E_CLASSNOTREG):
		return "Specified class is not registered in the registration database.";
	case(CLASS_E_NOAGGREGATION):
		return "Class cannot be created as part of an aggregate.";
	case(E_NOINTERFACE):
		return "Specified class does not implement the requested interface.";
	case(E_POINTER):
		return "Pointer parameter is NULL.";
	case(E_INVALIDARG):
		return "Parameter is not valid.";
	case(E_OUTOFMEMORY):
		return "Out of memory.";
	case(AUDCLNT_E_DEVICE_INVALIDATED):
		return "Audio endpoint not available.";
	case(AUDCLNT_E_SERVICE_NOT_RUNNING):
		return "The Windows audio service is not running.";
	case(AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL):
		return "Buffer duration and period are not equal.";
	case(AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED):
		return "This endpoint does not support exclusive mode.";
	case(AUDCLNT_E_UNSUPPORTED_FORMAT):
		return "The specified format is not supported";
	case(AUDCLNT_E_INVALID_DEVICE_PERIOD):
		return "Requested device period is not valid.";
	case(AUDCLNT_E_ENDPOINT_CREATE_FAILED):
		return "Creation of endpoint failed.";
	case(AUDCLNT_E_DEVICE_IN_USE):
		return "The endpoint device is already in use.";
	case(AUDCLNT_E_CPUUSAGE_EXCEEDED):
		return "The process-pass duration exceeds the maximum CPU usage.";
	case(AUDCLNT_E_BUFFER_SIZE_ERROR):
		return "Requested buffer duration is not valid.";
	case(AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED):
		return "The requested buffer size is not aligned.";
	case(AUDCLNT_E_WRONG_ENDPOINT_TYPE):
		return "Wrong endpoint type given.";
	case(AUDCLNT_E_ALREADY_INITIALIZED):
		return "The IAudioClient object is alraedy initialized.";
	case(AUDCLNT_E_NOT_INITIALIZED):
		return "The audio stream has not been successfully initialized.";
	case(AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED):
		return "The audio stream was not initialized for event-driven buffering.";
	case(AUDCLNT_E_BUFFER_OPERATION_PENDING):
		return "Buffer cannot be accessed because a stream reset is in progress.";
	case(AUDCLNT_E_OUT_OF_ORDER):
		return "A previous GetBuffer call is still in effect.";
	case(AUDCLNT_E_BUFFER_TOO_LARGE):
		return "Requested value exceeds the available buffer.";
	case(AUDCLNT_E_BUFFER_ERROR):
		return "Failed to retrieve data buffer.";
	case(AUDCLNT_E_INVALID_SIZE):
		return "Requested size is invalid.";
	case(AUDCLNT_E_NOT_STOPPED):
		return "The audio stream was not stopped at the time of the Start call.";
	case(AUDCLNT_E_EVENTHANDLE_NOT_SET):
		return "Eventhandle is not set for the audio stream.";
		// custom errors
	case(CUSTOM_E_FORMAT_NOT_SET):
		return "Format is not set.";
	}
}

