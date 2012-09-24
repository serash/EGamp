// AudioLibrary.h

#pragma once
#include <winnt.h>
#include <winerror.h>
#include <Windows.h>
#include <vector>
#using <mscorlib.dll>
#include "EffectsLibrary.h"

using System::String;
using System::Convert;
using System::TimeSpan;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Threading;
using namespace System::Text;
using namespace EffectsLibrary;

namespace AudioLibrary {
	// Constants
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
	const IID IID_ISimpleAudioVolume = __uuidof(ISimpleAudioVolume);
	const BYTE WaveHeader[] = { 'R',   'I',   'F',   'F',  0x00,  0x00,  0x00,  0x00, 'W',   'A',   'V',   'E',   'f',   'm',   't',   ' ', 0x00, 0x00, 0x00, 0x00 };
	const BYTE WaveData[] = { 'd', 'a', 't', 'a'};

	// Helper Classes/Structs
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
		bool writeData(const BYTE* data, UINT32 dataSize);
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
		HRESULT initialize(int bufferSize);
		void dispose();
		UINT32 getAvailableData();
		HRESULT setWaveFormat(WAVEFORMATEX *captureFormat_);
		HRESULT storeNullData(UINT32 numBytesAvailable);
		HRESULT storeData(const BYTE *pData, UINT32 numBytesAvailable);
		HRESULT loadData(BYTE *pData, UINT32 numBytesAvailable, DWORD *flags);
		// flags is 0 if at least one frame of real data is available else its AUDCLNT_BUFFERFLAGS_SILENT
	private:
		WAVEFORMATEX *waveFormat;
		BYTE **data;
		UINT32 dataSize;
		INT32 lastStored;
		DWORD currentFlags;
		WaveFileWriter ^renderWriter;
		WaveFileWriter ^captureWriter;
		EffectsCollection effects;
	};

	// Class
	public ref class AudioEngine
	{
	public:
		AudioEngine(UINT32 _EngineLatencyInMS);
		HRESULT engineStatus();
		array<String^> ^getRenderDevices();
		array<String^> ^getCaptureDevices();
		HRESULT setRenderDevice(UINT num);
		HRESULT setCaptureDevice(UINT num);
		HRESULT setDefaultRenderDevice();
		HRESULT setDefaultCaptureDevice();
		HRESULT initializeDevices();
		HRESULT startAudioStream();
		HRESULT stopAudioStream();
		HRESULT volumeDown(float fLevel);
		HRESULT volumeUp(float fLevel);
		HRESULT setVolume(float fLevel);
		HRESULT toggleMute();
		void dispose();
		static String ^getErrorCode(HRESULT hres);
		static bool Failed(HRESULT hres);
	private:
		static std::string getErrorCodeString(HRESULT hres);
		void initialize(UINT32 _EngineLatencyInMS);
		HRESULT makeRenderDeviceList();
		HRESULT makeCaptureDeviceList();
		void captureAudioStream();
		void renderAudioStream();

		HRESULT initializeResult;
		IAudioClient *pCaptureAudioClient;
		IAudioCaptureClient *pCaptureClient;
		IAudioClient *pRenderAudioClient;
		IAudioRenderClient *pRenderClient;
		ISimpleAudioVolume *pAudioVolume;
		IMMDeviceCollection *pRenderDevices;
		IMMDeviceCollection *pCaptureDevices;
		IMMDevice *pRenderDevice;
		IMMDevice *pCaptureDevice;
		IMMDeviceEnumerator *pEnumerator;
		UINT32 renderBufferFrameCount;
		UINT32 captureBufferFrameCount;
		size_t frameSize;
		LONG engineLatency;
		BOOL audioStreamStatus;

		// Handles
		HANDLE renderShutdownEvent;
		HANDLE captureShutdownEvent;
		HANDLE renderSamplesReadyEvent;
		HANDLE captureSamplesReadyEvent;

		// CLI properties
		Thread ^renderThread;
		Thread ^captureThread;
		AudioStream ^audioStream;
		array<String^> ^renderDevicesList;
		array<String^> ^captureDevicesList;
	};
}
