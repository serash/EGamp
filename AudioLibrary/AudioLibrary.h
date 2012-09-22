// AudioLibrary.h

#pragma once
#include <winnt.h>
#include <winerror.h>
#include <Windows.h>
#include <vector>
#using <mscorlib.dll>

using System::String;
using System::Convert;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;

namespace AudioLibrary {
	
	// Definitions
	#define REFTIMES_PER_SEC  10000000
	#define REFTIMES_PER_MILLISEC  10000
	#define EXIT_ON_ERROR(hres)  \
				  if (FAILED(hres)) { goto Exit; }
	#define SAFE_RELEASE(punk)  \
				  if ((punk) != NULL)  \
					{ (punk)->Release(); (punk) = NULL; }

	// Constants
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
	const REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	const BYTE WaveHeader[] = { 'R',   'I',   'F',   'F',  0x00,  0x00,  0x00,  0x00, 'W',   'A',   'V',   'E',   'f',   'm',   't',   ' ', 0x00, 0x00, 0x00, 0x00 };
	const BYTE WaveData[] = { 'd', 'a', 't', 'a'};
	const BYTE NullByte = 0;
	
	// Helper Classes
	struct WAVEHEADER
	{
		DWORD   dwRiff;                     // "RIFF"
		DWORD   dwSize;                     // Size
		DWORD   dwWave;                     // "WAVE"
		DWORD   dwFmt;                      // "fmt "
		DWORD   dwFmtSize;                  // Wave Format Size
	};

	private ref class WaveFileWriter
	{
	public:
		WaveFileWriter(const char *filename_, WAVEFORMATEX *waveFormat_);
		void open();
		bool writeData(BYTE* data, UINT32 dataSize);
		void close();
	private:
		String ^filename;
		FileStream^ fs;
		BinaryWriter^ w;
		WAVEFORMATEX *waveFormat;
		DWORD waveFileSize;
		DWORD headerSize;
	};

	private ref class AudioStream {
	public:
		AudioStream();
		~AudioStream();
		void openWriter();
		void closeWriter();
		bool initialize();
		bool initialize(int bufferSize);
		HRESULT setCaptureFormat(WAVEFORMATEX *captureFormat_);
		HRESULT setRenderFormat(WAVEFORMATEX *renderFormat_);
		HRESULT storeData(const BYTE *pData, UINT32 numFramesAvailable, BOOL *bDone);
		HRESULT loadData(BYTE *pData, UINT32 numFramesAvailable, DWORD *flags);
		// flags is 0 if at least one frame of real data is available else its AUDCLNT_BUFFERFLAGS_SILENT
	private:
		WAVEFORMATEX *renderFormat;
		WAVEFORMATEX *captureFormat;
		BYTE *data;
		UINT32 dataSize;
		DWORD currentFlags;
		WaveFileWriter ^writer;
	};

	// Class
	public ref class AudioEngine
	{
	public:
		AudioEngine();
		HRESULT engineStatus();
		array<String^> ^getRenderDevices();
		array<String^> ^getCaptureDevices();
		bool setRenderDevice(UINT num);
		bool setCaptureDevice(UINT num);
		bool initializeDevices();
		bool recordAudioStream();
	private:
		void dispose();
		void initialize();
		HRESULT makeRenderDeviceList();
		HRESULT makeCaptureDeviceList();
		bool checkForFailure(HRESULT hres);

		HRESULT initializeResult;
		int result;
		IAudioClient *pCaptureAudioClient;
		IAudioCaptureClient *pCaptureClient;
		IAudioClient *pRenderAudioClient;
		IAudioRenderClient *pRenderClient;
		IMMDeviceCollection *pRenderDevices;
		IMMDeviceCollection *pCaptureDevices;
		IMMDevice *pRenderDevice;
		IMMDevice *pCaptureDevice;
		REFERENCE_TIME captureActualDuration;
		REFERENCE_TIME renderActualDuration;
		UINT32 bufferFrameCount;


		AudioStream ^audioStream;
		array<String^> ^renderDevicesList;
		array<String^> ^captureDevicesList;
	};
}
