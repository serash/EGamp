// This is the main DLL file.

#include "stdafx.h"
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "AudioLibrary.h"

/* ==========================
   ===    AUDIOLIBRARY    ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::AudioEngine::AudioEngine() 
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
	audioStream = gcnew AudioStream();
	initialize();
}

void AudioLibrary::AudioEngine::initialize() 
{
	IMMDeviceEnumerator *pEnumerator = NULL;

    initializeResult = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if(checkForFailure(initializeResult)) return;
	
	pin_ptr<IMMDeviceCollection *> pinnedRenderPtr = &pRenderDevices;
	initializeResult = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, pinnedRenderPtr);
	if(checkForFailure(initializeResult)) return;
	initializeResult = makeRenderDeviceList();
	if(checkForFailure(initializeResult)) return;
	
	pin_ptr<IMMDeviceCollection *> pinnedCapturePtr = &pCaptureDevices;
	initializeResult = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, pinnedCapturePtr);
	if(checkForFailure(initializeResult)) return;
	initializeResult = makeCaptureDeviceList();
	if(checkForFailure(initializeResult)) return;
	
    SAFE_RELEASE(pEnumerator)
}

void AudioLibrary::AudioEngine::dispose()
{
    SAFE_RELEASE(pCaptureAudioClient)
    SAFE_RELEASE(pCaptureClient)
    SAFE_RELEASE(pRenderAudioClient)
    SAFE_RELEASE(pRenderClient)
    SAFE_RELEASE(pRenderDevices)
    SAFE_RELEASE(pCaptureDevices)
    SAFE_RELEASE(pRenderDevice)
    SAFE_RELEASE(pCaptureDevice)
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
    HRESULT hr;
	UINT pcDevices;
	IMMDevice *ppDevice = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR ppStrId = NULL;
	UINT i = 0;
	PROPVARIANT varName;

	hr = pRenderDevices->GetCount(&pcDevices);
	if(checkForFailure(hr)) return hr;
	
	renderDevicesList = gcnew array<String^>(pcDevices);
    PropVariantInit(&varName);

	for each(String^% s in renderDevicesList) {
		hr = pRenderDevices->Item(i, &ppDevice);
		if(checkForFailure(hr)) return hr;

		hr = ppDevice->OpenPropertyStore(STGM_READ, &pProps);
		if(checkForFailure(hr)) return hr;

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if(checkForFailure(hr)) return hr;

		std::wstring name (varName.pwszVal);
		s = gcnew String(name.c_str());
		i++;
	}

    SAFE_RELEASE(ppDevice)
	SAFE_RELEASE(pProps)
	CoTaskMemFree(ppStrId);
	return 0;
}

HRESULT AudioLibrary::AudioEngine::makeCaptureDeviceList()
{
    HRESULT hr;
	UINT pcDevices;
	IMMDevice *ppDevice = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR ppStrId = NULL;
	UINT i = 0;
	PROPVARIANT varName;

	hr = pCaptureDevices->GetCount(&pcDevices);
	if(checkForFailure(hr)) return hr;
	
	captureDevicesList = gcnew array<String^>(pcDevices);
    PropVariantInit(&varName);

	for each(String^% s in captureDevicesList) {
		hr = pCaptureDevices->Item(i, &ppDevice);
		if(checkForFailure(hr)) return hr;

		hr = ppDevice->OpenPropertyStore(STGM_READ, &pProps);
		if(checkForFailure(hr)) return hr;

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if(checkForFailure(hr)) return hr;

		std::wstring name (varName.pwszVal);
		s = gcnew String(name.c_str());
		i++;
	}

    SAFE_RELEASE(ppDevice)
	SAFE_RELEASE(pProps)
	CoTaskMemFree(ppStrId);
	return 0;
}

bool AudioLibrary::AudioEngine::setRenderDevice(UINT num)
{
    HRESULT hr;
	UINT pcDevices;
    SAFE_RELEASE(pRenderDevice)

	hr = pRenderDevices->GetCount(&pcDevices);
	if(checkForFailure(hr)) return false;
	if(num < 0 || num >= pcDevices) return false;
	
	pin_ptr<IMMDevice *> pinnedRenderPtr = &pRenderDevice;
	hr = pRenderDevices->Item(num, pinnedRenderPtr);
	if(checkForFailure(hr)) return false;

	return true;
}

bool AudioLibrary::AudioEngine::setCaptureDevice(UINT num)
{
    HRESULT hr;
	UINT pcDevices;
    SAFE_RELEASE(pCaptureDevice)

	hr = pCaptureDevices->GetCount(&pcDevices);
	if(checkForFailure(hr)) return false;
	if(num < 0 || num >= pcDevices) return false;
	
	pin_ptr<IMMDevice *> pinnedCapturePtr = &pCaptureDevice;
	hr = pCaptureDevices->Item(num, pinnedCapturePtr);
	if(checkForFailure(hr)) return false;

	return true;
}

bool AudioLibrary::AudioEngine::initializeDevices()
{
	HRESULT hr;
	WAVEFORMATEX *renderFormat;
	WAVEFORMATEX *captureFormat;
	UINT32 maxBufferSize;
	pin_ptr<UINT32> pinnedBufferFrameCountPtr = &bufferFrameCount;

	// INIT CAPTURE DEVICE
	pin_ptr<IAudioClient *> pinnedCaptureAudioClientPtr = &pCaptureAudioClient;
	hr = pCaptureDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedCaptureAudioClientPtr);
	if(checkForFailure(hr)) return false;

    hr = pCaptureAudioClient->GetMixFormat(&captureFormat);
	if(checkForFailure(hr)) return false;

    hr = pCaptureAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, captureFormat, NULL);
	if(checkForFailure(hr)) return false;

    // Get the size of the allocated buffer.
    hr = pCaptureAudioClient->GetBufferSize(pinnedBufferFrameCountPtr);
	if(checkForFailure(hr)) return false;
	maxBufferSize = bufferFrameCount;
	
	pin_ptr<IAudioCaptureClient *> pinnedCaptureClientPtr = &pCaptureClient;
    hr = pCaptureAudioClient->GetService(IID_IAudioCaptureClient, (void**)pinnedCaptureClientPtr);
	if(checkForFailure(hr)) return false;

    hr = audioStream->setCaptureFormat(captureFormat);
	if(checkForFailure(hr)) return false;

    // Calculate the actual duration of the allocated buffer.
    captureActualDuration = (double)REFTIMES_PER_SEC *
                     bufferFrameCount / captureFormat->nSamplesPerSec;

	// INIT RENDER DEVICE
	pin_ptr<IAudioClient *> pinnedRenderAudioClientPtr = &pRenderAudioClient;
	hr = pRenderDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)pinnedRenderAudioClientPtr);
	if(checkForFailure(hr)) return false;

    hr = pRenderAudioClient->GetMixFormat(&renderFormat);
	if(checkForFailure(hr)) return false;

    hr = pRenderAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, renderFormat, NULL);
	if(checkForFailure(hr)) return false;

    hr = audioStream->setRenderFormat(renderFormat);
	if(checkForFailure(hr)) return false;

    // Get the actual size of the allocated buffer.
    hr = pRenderAudioClient->GetBufferSize(pinnedBufferFrameCountPtr);
	if(checkForFailure(hr)) return false;
	if(bufferFrameCount > maxBufferSize) maxBufferSize = bufferFrameCount;
	
	pin_ptr<IAudioRenderClient *> pinnedRenderClientPtr = &pRenderClient;
    hr = pRenderAudioClient->GetService(IID_IAudioRenderClient, (void**)pinnedRenderClientPtr);
	if(checkForFailure(hr)) return false;

    renderActualDuration = (double)REFTIMES_PER_SEC *
                        bufferFrameCount / renderFormat->nSamplesPerSec;

	audioStream->initialize(maxBufferSize);
	return true;
}

bool AudioLibrary::AudioEngine::recordAudioStream()
{
	HRESULT hr;
	UINT32 numFramesAvailable;
	UINT32 numFramesPadding;
	UINT32 packetLength = 0;
    BYTE *pData;
    DWORD renderFlags = 0, captureFlags;
	BOOL bDone = FALSE;

	
	UINT32 counter = 0, MAX = 1000;

	hr = pCaptureAudioClient->Start();  // Start recording.
	hr = pRenderAudioClient->Start();  // Start playing.
	if(checkForFailure(hr)) return false;
	audioStream->openWriter();

    // Each loop fills about half of the shared buffer.
    while (bDone == FALSE && counter < MAX)
    {
        // Sleep for half the buffer duration.
        Sleep(captureActualDuration/REFTIMES_PER_MILLISEC/2);

        hr = pCaptureClient->GetNextPacketSize(&packetLength);
		if(checkForFailure(hr)) return false;

        while (packetLength != 0 && renderFlags != AUDCLNT_BUFFERFLAGS_SILENT && counter < MAX)
        {
			counter++;
			std::cout << counter << std::endl;
			// CAPTURE
            // Get the available data in the shared buffer.
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &captureFlags, NULL, NULL);
			if(checkForFailure(hr)) return false;

            if (captureFlags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = NULL;  // Tell CopyData to write silence.
            }

            // Copy the available capture data to the audio sink.
            hr = audioStream->storeData(pData, numFramesAvailable, &bDone);
			if(checkForFailure(hr)) return false;

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
			if(checkForFailure(hr)) return false;

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
			if(checkForFailure(hr)) return false;
			

			// FIXME: the stream to capture isnt fast enough to keep up with rendering...
			// TODO make 2 threads that access this stream??

			// RENDER
			// Sleep for half the buffer duration.
			//Sleep((DWORD)(renderActualDuration/REFTIMES_PER_MILLISEC/2));

			// See how much buffer space is available.
			hr = pRenderAudioClient->GetCurrentPadding(&numFramesPadding);
			if(checkForFailure(hr)) return false;

			numFramesAvailable = bufferFrameCount - numFramesPadding;

			// Grab all the available space in the shared buffer.
			hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
			if(checkForFailure(hr)) return false;

			// Get next 1/2-second of data from the audio source.
			hr = audioStream->loadData(pData, numFramesAvailable, &renderFlags);
			if(checkForFailure(hr)) return false;

			//error here!
			hr = pRenderClient->ReleaseBuffer(numFramesAvailable, renderFlags);
			if(checkForFailure(hr)) return false;
        }
    }

    hr = pCaptureAudioClient->Stop();  // Stop recording.
    hr = pRenderAudioClient->Stop();  // Stop playing.
	if(checkForFailure(hr)) return false;
	audioStream->closeWriter();

	dispose();
    return true;
}

bool AudioLibrary::AudioEngine::checkForFailure(HRESULT hres)
{
	return hres < 0;
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
	free(data);
}

bool AudioLibrary::AudioStream::initialize()
{
	return initialize(2048);
}

bool AudioLibrary::AudioStream::initialize(int bufferSize)
{
	dataSize = bufferSize;
	data = (BYTE*) realloc(data, sizeof(BYTE)*dataSize);
	return true;
}

HRESULT AudioLibrary::AudioStream::setCaptureFormat(WAVEFORMATEX *captureFormat_)
{
	captureFormat = (WAVEFORMATEX*) realloc(captureFormat, sizeof(WAVEFORMATEX));
	memcpy(captureFormat, captureFormat_, sizeof(WAVEFORMATEX));
	return 1;
}

HRESULT AudioLibrary::AudioStream::setRenderFormat(WAVEFORMATEX *renderFormat_)
{
	renderFormat = (WAVEFORMATEX*) realloc(renderFormat, sizeof(WAVEFORMATEX));
	memcpy(renderFormat, renderFormat_, sizeof(WAVEFORMATEX));
	return 1;
}

HRESULT AudioLibrary::AudioStream::storeData(const BYTE *pData, UINT32 numFramesAvailable, BOOL *bDone)
{
	if(numFramesAvailable > dataSize)
		return -1;
	memcpy(data, pData, numFramesAvailable);
	if(numFramesAvailable == 0) (*bDone) = TRUE;

    writer->writeData(data, numFramesAvailable);
	return 0;
}

HRESULT AudioLibrary::AudioStream::loadData(BYTE *pData, UINT32 numFramesAvailable, DWORD *flags)
{
	if(numFramesAvailable > dataSize)
		return -1;
	memcpy(data, pData, numFramesAvailable);
	if(numFramesAvailable == 0) currentFlags |= AUDCLNT_BUFFERFLAGS_SILENT;
	(*flags) |= currentFlags;
	return 0;

}

void AudioLibrary::AudioStream::openWriter()
{
	writer = gcnew WaveFileWriter("test.wav", captureFormat);
	writer->open();
}

void AudioLibrary::AudioStream::closeWriter() 
{
	writer->close();
}


/* ==========================
   ===   WAVEFILEWRITER   ===
   ===   IMPLEMENTATION   ===
   ========================== */

AudioLibrary::WaveFileWriter::WaveFileWriter(const char *filename_, WAVEFORMATEX *waveFormat_)
{
	filename = gcnew String(filename_);
	waveFormat = (WAVEFORMATEX*) realloc(waveFormat, sizeof(WAVEFORMATEX));
	memcpy(waveFormat, waveFormat_, sizeof(WAVEFORMATEX));
	waveFileSize = 0;
}

void AudioLibrary::WaveFileWriter::open()
{
	fs = gcnew FileStream(filename, System::IO::FileMode::Create, System::IO::FileAccess::Write); 
	w = gcnew BinaryWriter(fs);
	headerSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + waveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD);
	waveFileSize = headerSize;

	for (int i=0; i<headerSize; i++)
		w->Write(NullByte);
}

bool AudioLibrary::WaveFileWriter::writeData(BYTE* data, UINT32 dataSize)
{
	for (int i=0; i<dataSize; i++)
		w->Write(data[i]);

	waveFileSize += dataSize;
	return true;
}

void AudioLibrary::WaveFileWriter::close()
{
	fs->Position = 0;
	headerSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + waveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD);
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
	
	for (int i=0; i<headerSize; i++)
		w->Write(waveFileData[i]);
	fs->Close();
}
