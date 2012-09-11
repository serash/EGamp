using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EGamp.AudioEngine.Effects;
using NAudio.CoreAudioApi;
using NAudio.Wave;
using NAudio;
using NAudio.Wave.SampleProviders;

namespace EGamp.AudioEngine
{
    abstract class BasicAudioEngine
    {
        protected bool isRecording;
        protected WaveIn sourceStream = null;
        protected IWaveProvider waveIn = null;
        protected SampleAggregator aggregator = null;
        protected EffectStream waveStream = null;
        public event EventHandler<AddEffectEventArgs> effectAdded;

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

        public BasicAudioEngine(int deviceNumber = 0)
        {
            aggregator = new SampleAggregator();
            aggregator.NotificationCount = 882;
            aggregator.PerformFFT = true;

            sourceStream = new WaveIn();
            sourceStream.BufferMilliseconds = 50;
            sourceStream.DeviceNumber = deviceNumber;
            sourceStream.WaveFormat = new WaveFormat(44100, WaveIn.GetCapabilities(deviceNumber).Channels);
            waveIn = new WaveInProvider(sourceStream);
            waveStream = new EffectStream(new WaveProviderStream(waveIn));

            // add handle events
            isRecording = false;
        }

        protected ISampleProvider createSampleStream()
        {
            var inputStream = new SampleChannel(waveStream);
            var sampleStream = new NotifyingSampleProvider(inputStream);
            sampleStream.Sample += (s, e) => aggregator.Add(e.Left);
            return sampleStream;
        }

        public void SetRecordingDevice(int deviceNumber)
        {
            if (deviceNumber <WaveIn.DeviceCount)
                sourceStream.DeviceNumber = deviceNumber;
        }

        protected void BasicInitialize()
        {
            sourceStream.StartRecording();
        }

        protected void BasicClose()
        {
            waveStream.Dispose();
            sourceStream.StopRecording();
            sourceStream.Dispose();            
        }

        public void AddEffect(Effect effect)
        {
            waveStream.AddEffect(effect);
            if (effectAdded != null)
            {
                effectAdded(this, new AddEffectEventArgs(effect));
            }
        }

        public abstract void Initialize();
        public abstract void Play();
        public abstract void Stop();
        public abstract void Close();
        public abstract bool IsPlaying();
        public abstract void SetVolume(float volume);
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
