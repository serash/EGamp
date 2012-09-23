// This is the main DLL file.

#include "stdafx.h"
#include "AudioLibrary.h"


/* ==========================
   ===    AUDIOLIBRARY    ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::AudioEngine::AudioEngine(UINT32 _EngineLatencyInMS) 
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
	captureShutdownEvent = NULL;
	renderShutdownEvent = NULL;
	captureSamplesReadyEvent = NULL;
	renderSamplesReadyEvent = NULL;
	audioStream = gcnew AudioStream();
	initialize(_EngineLatencyInMS);
}

void AudioLibrary::AudioEngine::initialize(UINT32 _EngineLatencyInMS) 
{
    initializeResult = S_OK;
	pin_ptr<IMMDeviceEnumerator *> pinnedEnumeratorPtr = &pEnumerator;

    initializeResult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)pinnedEnumeratorPtr);
	if(FAILED(initializeResult)) return;

	pin_ptr<IMMDeviceCollection *> pinnedRenderPtr = &pRenderDevices;
	initializeResult = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, pinnedRenderPtr);
	if(FAILED(initializeResult)) return;
	initializeResult = makeRenderDeviceList();
	if(FAILED(initializeResult)) return;
	
	pin_ptr<IMMDeviceCollection *> pinnedCapturePtr = &pCaptureDevices;
	initializeResult = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, pinnedCapturePtr);
	if(FAILED(initializeResult)) return;
	initializeResult = makeCaptureDeviceList();
	if(FAILED(initializeResult)) return;
	

	//initialize EventHandle
    captureShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    renderShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    captureSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    renderSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	
    engineLatency = _EngineLatencyInMS;
}

void AudioLibrary::AudioEngine::dispose()
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

	audioStream->dispose();

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

	pin_ptr<IMMDeviceEnumerator *> pinnedEnumeratorPtr = &pEnumerator;
    safeRelease(reinterpret_cast<IMMDeviceEnumerator **>(pinnedEnumeratorPtr));
}

HRESULT AudioLibrary::AudioEngine::engineStatus() 
{
	return initializeResult;
}

array<String^> ^AudioLibrary::AudioEngine::getRenderDevices() 
{
	return renderDevicesList;
}

array<String^> ^AudioLibrary::AudioEngine::getCaptureDevices() 
{
	return captureDevicesList;
}

HRESULT AudioLibrary::AudioEngine::makeRenderDeviceList()
{
    HRESULT hr = S_OK;
	UINT pcDevices;
	IMMDevice *ppDevice = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR ppStrId = NULL;
	UINT i = 0;
	PROPVARIANT varName;

	hr = pRenderDevices->GetCount(&pcDevices);
	if(FAILED(hr)) return hr;
	
	renderDevicesList = gcnew array<String^>(pcDevices);
    PropVariantInit(&varName);

	for each(String^% s in renderDevicesList) {
		hr = pRenderDevices->Item(i, &ppDevice);
		if(FAILED(hr)) return hr;

		hr = ppDevice->OpenPropertyStore(STGM_READ, &pProps);
		if(FAILED(hr)) return hr;

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if(!SUCCEEDED(hr)) return hr;

		std::wstring name (varName.pwszVal);
		s = gcnew String(name.c_str());
		i++;
	}

    safeRelease(&ppDevice);
	safeRelease(&pProps);
	CoTaskMemFree(ppStrId);
	return 0;
}

HRESULT AudioLibrary::AudioEngine::makeCaptureDeviceList()
{
    HRESULT hr = S_OK;
	UINT pcDevices;
	IMMDevice *ppDevice = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR ppStrId = NULL;
	UINT i = 0;
	PROPVARIANT varName;

	hr = pCaptureDevices->GetCount(&pcDevices);
	if(FAILED(hr)) return hr;
	
	captureDevicesList = gcnew array<String^>(pcDevices);
    PropVariantInit(&varName);

	for each(String^% s in captureDevicesList) {
		hr = pCaptureDevices->Item(i, &ppDevice);
		if(FAILED(hr)) return hr;

		hr = ppDevice->OpenPropertyStore(STGM_READ, &pProps);
		if(FAILED(hr)) return hr;

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if(!SUCCEEDED(hr)) return hr;

		std::wstring name (varName.pwszVal);
		s = gcnew String(name.c_str());
		i++;
	}

    safeRelease(&ppDevice);
	safeRelease(&pProps);
	CoTaskMemFree(ppStrId);
	return 0;
}

HRESULT AudioLibrary::AudioEngine::setRenderDevice(UINT num)
{
    HRESULT hr = S_OK;
	UINT pcDevices;
	pin_ptr<IMMDevice *> pinnedRenderDevicePtr = &pRenderDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedRenderDevicePtr));

	hr = pRenderDevices->GetCount(&pcDevices);
	if(FAILED(hr)) return hr;
	if(num < 0 || num >= pcDevices) return E_INVALIDARG;
	
	pin_ptr<IMMDevice *> pinnedRenderPtr = &pRenderDevice;
	hr = pRenderDevices->Item(num, pinnedRenderPtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::AudioEngine::setCaptureDevice(UINT num)
{
    HRESULT hr = S_OK;
	UINT pcDevices;
	pin_ptr<IMMDevice *> pinnedCaptureDevicePtr = &pCaptureDevice;
    safeRelease(reinterpret_cast<IMMDevice **>(pinnedCaptureDevicePtr));

	hr = pCaptureDevices->GetCount(&pcDevices);
	if(FAILED(hr)) return hr;
	if(num < 0 || num >= pcDevices) return E_INVALIDARG;
	
	pin_ptr<IMMDevice *> pinnedCapturePtr = &pCaptureDevice;
	hr = pCaptureDevices->Item(num, pinnedCapturePtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::AudioEngine::setDefaultRenderDevice()
{
    HRESULT hr = S_OK;
	
	pin_ptr<IMMDevice *> pinnedRenderDevicePtr = &pRenderDevice;
	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, pinnedRenderDevicePtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::AudioEngine::setDefaultCaptureDevice()
{
    HRESULT hr = S_OK;
	
	pin_ptr<IMMDevice *> pinnedCaptureDevicePtr = &pCaptureDevice;
	hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, pinnedCaptureDevicePtr);
	if(FAILED(hr)) return hr;

	return hr;
}

HRESULT AudioLibrary::AudioEngine::initializeCaptureDevice()
{
    HRESULT hr = S_OK;
	WAVEFORMATEX *captureFormat;

	// INIT CAPTURE DEVICE
	pin_ptr<IAudioClient *> pinnedCaptureAudioClientPtr = &pCaptureAudioClient;
	hr = pCaptureDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedCaptureAudioClientPtr);
	if(FAILED(hr)) return hr;

    hr = pCaptureAudioClient->GetMixFormat(&captureFormat);
	if(FAILED(hr)) return hr;
    frameSize = (captureFormat->wBitsPerSample / 8) * captureFormat->nChannels;

    hr = pCaptureAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, engineLatency*10000, 0, captureFormat, NULL);
	if(FAILED(hr)) return hr;

	// set EventHandle
	hr = pCaptureAudioClient->SetEventHandle(captureSamplesReadyEvent);
	if(FAILED(hr)) return hr;
	
    // Get the size of the allocated buffer.
	pin_ptr<UINT32> pinnedBufferFrameCountPtr = &captureBufferFrameCount;
    hr = pCaptureAudioClient->GetBufferSize(pinnedBufferFrameCountPtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioCaptureClient *> pinnedCaptureClientPtr = &pCaptureClient;
    hr = pCaptureAudioClient->GetService(IID_IAudioCaptureClient, (void**)pinnedCaptureClientPtr);
	if(FAILED(hr)) return hr;

    hr = audioStream->setCaptureFormat(captureFormat);
	if(FAILED(hr)) return hr;
	return hr;
}

HRESULT AudioLibrary::AudioEngine::initializeRenderDevice()
{
    HRESULT hr = S_OK;
	WAVEFORMATEX *renderFormat;

	// INIT RENDER DEVICE
	pin_ptr<IAudioClient *> pinnedRenderAudioClientPtr = &pRenderAudioClient;
	hr = pRenderDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedRenderAudioClientPtr);
	if(FAILED(hr)) return hr;

    hr = pRenderAudioClient->GetMixFormat(&renderFormat);
	if(FAILED(hr)) return hr;

    hr = pRenderAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, engineLatency*10000, 0, renderFormat, NULL);
	if(FAILED(hr)) return hr;

	// set EventHandle
	hr = pRenderAudioClient->SetEventHandle(renderSamplesReadyEvent);
	if(FAILED(hr)) return hr;

    hr = audioStream->setRenderFormat(renderFormat);
	if(FAILED(hr)) return hr;

    // Get the actual size of the allocated buffer.
	pin_ptr<UINT32> pinnedBufferFrameCountPtr = &renderBufferFrameCount;
    hr = pRenderAudioClient->GetBufferSize(pinnedBufferFrameCountPtr);
	if(FAILED(hr)) return hr;
	
	pin_ptr<IAudioRenderClient *> pinnedRenderClientPtr = &pRenderClient;
    hr = pRenderAudioClient->GetService(IID_IAudioRenderClient, (void**)pinnedRenderClientPtr);
	if(FAILED(hr)) return hr;

    renderActualDuration = (double)REFTIMES_PER_SEC *
                        renderBufferFrameCount / renderFormat->nSamplesPerSec;
	return hr;
}

void AudioLibrary::AudioEngine::captureAudioStream()
{
    HRESULT hr = S_OK;
    bool stillPlaying = true;
    HANDLE waitArray[2] = {captureShutdownEvent, captureSamplesReadyEvent };
    BYTE *pData;
    UINT32 framesAvailable;
    DWORD  flags;

    while (stillPlaying)
    {
        DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // shutdownEvent
            stillPlaying = false;   // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:		// captureSamplesReadyEvent
            hr = pCaptureClient->GetBuffer(&pData, &framesAvailable, &flags, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                if (framesAvailable != 0)
                {
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
						audioStream->storeNullData(framesAvailable*frameSize);
                    else
						hr = audioStream->storeData(pData, framesAvailable*frameSize);
                }
                hr = pCaptureClient->ReleaseBuffer(framesAvailable);
                if (FAILED(hr))
                    printf("Unable to release capture buffer: %x!\n", hr);
            }
            break;
        }
    }
}

void AudioLibrary::AudioEngine::renderAudioStream()
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

				if(audioStream->getBufferedSize() <= (framesAvailable*frameSize))
				{
                    UINT32 framesToWrite = audioStream->getBufferedSize() / frameSize;
                    hr = pRenderClient->GetBuffer(framesToWrite, &pData);
                    if (SUCCEEDED(hr))
                    {
                        //
                        //  Copy data from the render buffer to the output buffer and bump our render pointer.
                        //
						audioStream->loadData(pData, framesToWrite*frameSize, &flags);
                        hr = pRenderClient->ReleaseBuffer(framesToWrite, flags);
                        if (!SUCCEEDED(hr))
                        {
							if(hr == AUDCLNT_E_INVALID_SIZE) std::cout << "AUDCLNT_E_INVALID_SIZE\n";
							if(hr == AUDCLNT_E_BUFFER_SIZE_ERROR) std::cout << "AUDCLNT_E_BUFFER_SIZE_ERROR\n";
							if(hr == AUDCLNT_E_OUT_OF_ORDER) std::cout << "AUDCLNT_E_OUT_OF_ORDER\n";
							if(hr == AUDCLNT_E_DEVICE_INVALIDATED) std::cout << "AUDCLNT_E_DEVICE_INVALIDATED\n";
							if(hr == AUDCLNT_E_SERVICE_NOT_RUNNING) std::cout << "AUDCLNT_E_SERVICE_NOT_RUNNING\n";
							if(hr == E_INVALIDARG) std::cout << "E_INVALIDARG\n";
                            printf("Unable to release buffer: %x\n", hr);
                            stillPlaying = false;
                        }
                    }
                }
            }
            break;
        }
    }
}

HRESULT AudioLibrary::AudioEngine::startAudioStream()
{
    HRESULT hr = S_OK;
	
	audioStream->initialize(max(captureBufferFrameCount, renderBufferFrameCount));

	hr = pCaptureAudioClient->Start();  // Start recording.
	if(FAILED(hr)) return hr;
	hr = pRenderAudioClient->Start();  // Start playing.
	if(FAILED(hr)) return hr;
	audioStream->openWriter();

	ThreadStart ^pRenderThread = gcnew ThreadStart(this, &AudioLibrary::AudioEngine::renderAudioStream);
	ThreadStart ^pCaptureThread = gcnew ThreadStart(this, &AudioLibrary::AudioEngine::captureAudioStream);
	captureThread = gcnew Thread(pCaptureThread);
	renderThread = gcnew Thread(pRenderThread);
	
	captureThread->Start();
	renderThread->Start();

	audioStreamStatus = true;
	return hr;
}

HRESULT AudioLibrary::AudioEngine::stopAudioStream()
{
    HRESULT hr = S_OK;
	// make event to shut down the threads
	if(captureShutdownEvent) {
		SetEvent(captureShutdownEvent);
	}
	if(renderShutdownEvent) {
		SetEvent(renderShutdownEvent);
	}
	// Abort any thread that is still alive after shutdown
	if(captureThread->IsAlive) {
		captureThread->Abort();
	}
	if(renderThread->IsAlive) {
		renderThread->Abort();
	}
	
    hr = pCaptureAudioClient->Stop();  // Stop recording.
	if(FAILED(hr)) return hr;
    hr = pRenderAudioClient->Stop();  // Stop playing.
	if(FAILED(hr)) return hr;
	audioStream->closeWriter();

	audioStreamStatus = false;
	return true;
}


/* ==========================
   ===    AUDIOSTREAM     ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::AudioStream::AudioStream() 
{
	captureFormat = NULL;
	renderFormat = NULL;
	data = NULL;
	dataSize = 0;
	currentFlags = 0;
}

AudioLibrary::AudioStream::~AudioStream() 
{
	dispose();
}

HRESULT AudioLibrary::AudioStream::initialize(int bufferSize)
{
	dataSize = bufferSize;
	data = (BYTE*) realloc(data, sizeof(BYTE)*dataSize);
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

HRESULT AudioLibrary::AudioStream::setRenderFormat(WAVEFORMATEX *renderFormat_)
{
	if(renderFormat) {
        CoTaskMemFree(renderFormat);
        captureFormat = NULL;
	}
	renderFormat = (WAVEFORMATEX*) CoTaskMemAlloc(sizeof(WAVEFORMATEX) + renderFormat_->cbSize);
	memcpy(renderFormat, renderFormat_, sizeof(WAVEFORMATEX) + renderFormat_->cbSize);
	return S_OK;
}

HRESULT AudioLibrary::AudioStream::storeNullData(UINT32 numFramesAvailable)
{
    captureWriter->writeData(data, numFramesAvailable);
	if(numFramesAvailable > dataSize)
		return E_INVALIDARG;
	SecureZeroMemory(data, numFramesAvailable);
	bufferedSize = numFramesAvailable;
	return S_OK;
}

HRESULT AudioLibrary::AudioStream::storeData(const BYTE *pData, UINT32 numFramesAvailable)
{
    captureWriter->writeData(pData, numFramesAvailable);
	if(numFramesAvailable > dataSize)
		return E_INVALIDARG;
	if(bufferedSize > 0)
		return AUDCLNT_E_BUFFER_ERROR;
	// store from pData
	memcpy(data, pData, numFramesAvailable);
	bufferedSize = numFramesAvailable;
	return S_OK;
}

HRESULT AudioLibrary::AudioStream::loadData(BYTE *pData, UINT32 numFramesAvailable, DWORD *flags)
{
	if(numFramesAvailable > dataSize)
		return E_INVALIDARG;
	// load to pData
	memcpy(pData, data, numFramesAvailable);
	bufferedSize -= numFramesAvailable;
    renderWriter->writeData(pData, numFramesAvailable);

	// set flags
	if(numFramesAvailable == 0) currentFlags |= AUDCLNT_BUFFERFLAGS_SILENT;
	(*flags) |= currentFlags;
	return S_OK;

}

UINT32 AudioLibrary::AudioStream::getBufferedSize()
{
	return bufferedSize;
}

void AudioLibrary::AudioStream::openWriter()
{
	captureWriter = gcnew WaveFileWriter("capture.wav", captureFormat);
	captureWriter->open();
	renderWriter = gcnew WaveFileWriter("render.wav", captureFormat);
	renderWriter->open();
}

void AudioLibrary::AudioStream::closeWriter() 
{
	captureWriter->close();
	renderWriter->close();
}

void AudioLibrary::AudioStream::dispose() 
{
	if(data) {
		free(data);
		data = NULL;
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


/* ==========================
   ===   WAVEFILEWRITER   ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::WaveFileWriter::WaveFileWriter(const char *filename_, WAVEFORMATEX *waveFormat_)
{
	filename = gcnew String(filename_);
	waveFormat = (WAVEFORMATEX*) realloc(waveFormat, sizeof(WAVEFORMATEX) + waveFormat_->cbSize);
	memcpy(waveFormat, waveFormat_, sizeof(WAVEFORMATEX) + waveFormat_->cbSize);
	waveFileSize = 0;
}

void AudioLibrary::WaveFileWriter::open()
{
	fs = gcnew FileStream(filename, System::IO::FileMode::Create, System::IO::FileAccess::Write); 
	w = gcnew BinaryWriter(fs);
	headerSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + waveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD);
	waveFileSize = headerSize;

	for (int i=0; i<headerSize; i++)
		w->Write(0x00);
}

bool AudioLibrary::WaveFileWriter::writeData(const BYTE* data, UINT32 dataSize)
{
	for (int i=0; i<dataSize; i++)
		w->Write(data[i]);

	waveFileSize += dataSize;
	return true;
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
	
	fs->Position = 0;
	for (int i=0; i<headerSize; i++)
		w->Write(waveFileData[i]);
	fs->Close();
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
	}
}