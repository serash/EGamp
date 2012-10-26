// AudioLibrary.h

#pragma once
#include <winnt.h>
#include <winerror.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <fstream>
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
	private class SampleEventSource {
	public:
		__event void ThrowSampleEvent(UINT32 left, UINT32 right);
	};
	struct WAVEHEADER
	{
		DWORD   dwRiff;                     // "RIFF"
		DWORD   dwSize;                     // Size
		DWORD   dwWave;                     // "WAVE"
		DWORD   dwFmt;                      // "fmt "
		DWORD   dwFmtSize;                  // Wave Format Size
	};
	private class WaveFileWriter
	{
	public:
		WaveFileWriter(const char *filename_, WAVEFORMATEX *waveFormat_);
		void open();
		bool writeData(const BYTE* data, UINT32 dataSize);
		void close();
	private:
		std::string filename;
		std::ofstream myfile;
		WAVEFORMATEX *waveFormat;
		DWORD waveFileSize;
		DWORD headerSize;
	};
	private class AudioStream {
	public:
		AudioStream();
		~AudioStream();
		void openWriter();
		void closeWriter();
		HRESULT initialize(UINT32 numCaptureFrames_, UINT32 numRenderFrames_);
		void dispose();
		UINT32 getAvailableFrames();
		HRESULT setRenderFormat(WAVEFORMATEX *renderFormat_);
		HRESULT setCaptureFormat(WAVEFORMATEX *captureFormat_);
		WAVEFORMATEX *getRenderFormat();
		WAVEFORMATEX *getCaptureFormat();
		HRESULT storeNullData(UINT32 numFramesAvailable);
		HRESULT storeData(const BYTE *pData, UINT32 numFramesAvailable);
		HRESULT loadData(BYTE *pData, UINT32 numFramesAvailable, DWORD *flags);
		static DWORD __stdcall dataConversionFunction(LPVOID Context);
	private:
		template <class SAMPLETYPE> HRESULT resample();
		DWORD LeastCommonMultiple(DWORD rate1, DWORD rate2); 
		DWORD GreatestCommonDivisor(DWORD rate1, DWORD rate2);

		WAVEFORMATEX *renderFormat;
		WAVEFORMATEX *captureFormat;
		DWORD renderSamplePos;
		DWORD captureSamplePos;
		//BYTE *capturedData;
		//BYTE *renderData;
		UINT32 numAvailableFrames;
		UINT32 bufSize;
		BYTE **buffer;
		INT32 storedDataId;
		INT32 dataIsReady;
		DWORD currentFlags;
		DWORD convertWaveFormat();

		WaveFileWriter *renderWriter;
		WaveFileWriter *captureWriter;
		//EffectsCollection effects;
	};
	private class InnerAudioEngine : public IAudioSessionEvents, IMMNotificationClient
	{
	public:
		// Public Functions
		InnerAudioEngine(UINT32 _EngineLatencyInMS, bool enableStreamSwitch_);
		array<String^> ^makeRenderDeviceList();
		array<String^> ^makeCaptureDeviceList();
		HRESULT setRenderDevice(UINT num);
		HRESULT setCaptureDevice(UINT num);
		HRESULT setDefaultRenderDevice();
		HRESULT setDefaultCaptureDevice();
		HRESULT startAudioStream();
		HRESULT stopAudioStream();
		HRESULT volumeDown(float fLevel);
		HRESULT volumeUp(float fLevel);
		HRESULT setVolume(float fLevel);
		HRESULT toggleMute();
		void dispose();
		HRESULT initialize();

		// Interface related
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();
		HRESULT handleStreamSwitchEvent();
	private:
		// Private Functions
		HRESULT initializeDevices();
		HRESULT setWaveFormats();
		HRESULT initializeStreamSwitch();
		HRESULT terminateStreamSwitch();
		static DWORD __stdcall captureThreadFunction(LPVOID Context);
		static DWORD __stdcall renderThreadFunction(LPVOID Context);
		DWORD captureAudioStream();
		DWORD renderAudioStream();

		// Properties
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
		IAudioSessionControl *pAudioSessionControl;
		AudioStream *pAudioStream;
		UINT32 renderBufferFrameCount;
		UINT32 captureBufferFrameCount;
		LONG engineLatency;
		bool enableStreamSwitch;
		bool audioStreamStatus;
		bool inStreamSwitch;

		// Handles
		HANDLE captureThread;
		HANDLE renderThread;
		HANDLE renderShutdownEvent;
		HANDLE captureShutdownEvent;
		HANDLE renderSamplesReadyEvent;
		HANDLE captureSamplesReadyEvent;
		HANDLE streamSwitchEvent;          // Set when the current session is disconnected or the default device changes.
		HANDLE streamSwitchCompleteEvent;  // Set when the default device changed.
		SampleEventSource *sampleEvent;

		// Interface related
		LONG _RefCount;
		STDMETHOD(OnDisplayNameChanged) (LPCWSTR /*NewDisplayName*/, LPCGUID /*EventContext*/) { return S_OK; };
		STDMETHOD(OnIconPathChanged) (LPCWSTR /*NewIconPath*/, LPCGUID /*EventContext*/) { return S_OK; };
		STDMETHOD(OnSimpleVolumeChanged) (float /*NewSimpleVolume*/, BOOL /*NewMute*/, LPCGUID /*EventContext*/) { return S_OK; }
		STDMETHOD(OnChannelVolumeChanged) (DWORD /*ChannelCount*/, float /*NewChannelVolumes*/[], DWORD /*ChangedChannel*/, LPCGUID /*EventContext*/) { return S_OK; };
		STDMETHOD(OnGroupingParamChanged) (LPCGUID /*NewGroupingParam*/, LPCGUID /*EventContext*/) {return S_OK; };
		STDMETHOD(OnStateChanged) (AudioSessionState /*NewState*/) { return S_OK; };
		STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason);
		STDMETHOD(OnDeviceStateChanged) (LPCWSTR /*DeviceId*/, DWORD /*NewState*/) { return S_OK; }
		STDMETHOD(OnDeviceAdded) (LPCWSTR /*DeviceId*/) { return S_OK; };
		STDMETHOD(OnDeviceRemoved) (LPCWSTR /*DeviceId(*/) { return S_OK; };
		STDMETHOD(OnDefaultDeviceChanged) (EDataFlow Flow, ERole Role, LPCWSTR NewDefaultDeviceId);
		STDMETHOD(OnPropertyValueChanged) (LPCWSTR /*DeviceId*/, const PROPERTYKEY /*Key*/){return S_OK; };
		STDMETHOD(QueryInterface)(REFIID iid, void **pvObject);
	};

	// Wrapper class needed for managed c++
	public ref class AudioEngine
	{
	public:
		AudioEngine(UINT32 _EngineLatencyInMS, bool enableStreamSwitch_);
		array<String^> ^getRenderDevices();
		array<String^> ^getCaptureDevices();
		HRESULT setRenderDevice(UINT num);
		HRESULT setCaptureDevice(UINT num);
		HRESULT setDefaultRenderDevice();
		HRESULT setDefaultCaptureDevice();
		HRESULT startAudioStream();
		HRESULT stopAudioStream();
		HRESULT volumeDown(float fLevel);
		HRESULT volumeUp(float fLevel);
		HRESULT setVolume(float fLevel);
		HRESULT toggleMute();
		void dispose();
		HRESULT initialize();
		static String ^getErrorCode(HRESULT hres);
		static bool Failed(HRESULT hres);
	private:
		static std::string getErrorCodeString(HRESULT hres);
		InnerAudioEngine *innerEngine;

		// CLI properties
		array<String^> ^renderDevicesList;
		array<String^> ^captureDevicesList;
	};
}
