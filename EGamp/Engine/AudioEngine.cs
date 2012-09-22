using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using EGamp.Engine.Asio;
using EGamp.Engine.Effects;
using NAudio.CoreAudioApi;
using NAudio;
using NAudio.Wave;
using NAudio.Wave.SampleProviders;
using AsioAudioAvailableEventArgs = NAudio.Wave.AsioAudioAvailableEventArgs;

namespace EGamp.Engine
{
    public class AudioEngine : IAudioEngine
    {
        private WaveIn sourceStream = null;
        private IWaveProvider waveIn = null;
        private SampleAggregator aggregator = null;
        private EffectStream waveStream = null;
        private MyAsioOut wavePlayer = null;
        private BufferedWaveProvider bwp;

        private event EventHandler<AddEffectEventArgs> effectAdded;

        public event EventHandler<FftEventArgs> FftCalculated
        {
            add { aggregator.FftCalculated += value; }
            remove { aggregator.FftCalculated -= value; }
        }

        public event EventHandler<MaxSampleEventArgs> MaximumCalculated
        {
            add { aggregator.MaximumCalculated += value; }
            remove { aggregator.MaximumCalculated -= value; }
        }

        public event EventHandler<AddEffectEventArgs> EffectAdded
        {
            add { effectAdded += value; }
            remove { effectAdded -= value; }
        }

        private WaveFileWriter writer;
        public AudioEngine(int deviceNumber = 0)
        {
            aggregator = new SampleAggregator();
            aggregator.NotificationCount = 882;
            aggregator.PerformFFT = true;

            // wavein
            sourceStream = new WaveIn();
            sourceStream.BufferMilliseconds = 50;
            sourceStream.DeviceNumber = deviceNumber;
            sourceStream.WaveFormat = new WaveFormat(44100, WaveIn.GetCapabilities(deviceNumber).Channels);

            // waveprovider
            waveIn = new WaveInProvider(sourceStream);
            waveStream = new EffectStream(new WaveProviderStream(waveIn));

            // asioOut
            wavePlayer = new MyAsioOut();
            wavePlayer.AudioAvailable += AudioAvailableEvent;
            bwp = new BufferedWaveProvider(new WaveFormat(44100, WaveIn.GetCapabilities(deviceNumber).Channels));

            this.writer = new WaveFileWriter("C:\\test.wav", new WaveFormat(44100, 1));
        }

        private ISampleProvider createSampleStream()
        {
            var inputStream = new SampleChannel(waveStream);
            var sampleStream = new NotifyingSampleProvider(inputStream);
            sampleStream.Sample += (s, e) => aggregator.Add(e.Left);
            return sampleStream;
        }

        public void AudioAvailableEvent(object sender, Asio.AsioAudioAvailableEventArgs e)
        {
            var samples = e.GetAsInterleavedSamples();
            Logger.Log("sample: " + samples[0]);
            writer.WriteSamples(samples, 0, samples.Length);
            //byte[] data = new byte[e.SamplesPerBuffer];
            //Marshal.Copy(e.InputBuffers[0], data, 0, e.SamplesPerBuffer);
            //bwp.AddSamples(data, 0, e.SamplesPerBuffer);
            //waveStream.writeFromAsioInput(e.InputBuffers, e.SamplesPerBuffer, e.AsioSampleType);
        }

        public void AddEffect(Effect effect)
        {
            waveStream.AddEffect(effect);
            if (effectAdded != null)
            {
                effectAdded(this, new AddEffectEventArgs(effect));
            }
        }

        public void SetRecordingDevice(int deviceNumber)
        {
            if (deviceNumber <WaveIn.DeviceCount)
                sourceStream.DeviceNumber = deviceNumber;
        }

        public void Initialize()
        {
            //wavePlayer.InitRecordAndPlayback(new SampleToWaveProvider(this.createSampleStream()), 1, 44100); 
            wavePlayer.InitRecordAndPlayback(bwp, 1, 44100);
        }

        public void Play()
        {
            wavePlayer.Play();
        }

        public void Stop()
        {
            wavePlayer.Stop();
        }

        public void Close()
        {
            Stop();
            waveStream.Dispose();
            sourceStream.Dispose();  
            wavePlayer.Dispose();
        }

        public void SetVolume(float volume)
        {
        }

        public bool IsPlaying()
        {
            return wavePlayer.PlaybackState == PlaybackState.Playing;
        }
    }

    public class AddEffectEventArgs : EventArgs
    {
        [DebuggerStepThrough]
        public AddEffectEventArgs(Effect effect)
        {
            this.NewEffect = effect;
        }
        public Effect NewEffect { get; private set; }
    }
}
