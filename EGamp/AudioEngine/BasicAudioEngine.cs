using System;
using System.Collections.Generic;
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
        protected WaveIn sourceStream = null;
        protected WaveInProvider waveIn = null;
        protected SampleAggregator aggregator = null;
        protected EffectChain effects = null;

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

        public BasicAudioEngine(int deviceNumber = 0)
        {
            aggregator = new SampleAggregator();
            aggregator.NotificationCount = 882;
            aggregator.PerformFFT = true;

            effects = new EffectChain();
            effects.AddEffect(new Amplifier());


            sourceStream = new WaveIn();
            sourceStream.BufferMilliseconds = 50;
            sourceStream.DeviceNumber = deviceNumber;
            sourceStream.WaveFormat = new WaveFormat(44100, WaveIn.GetCapabilities(deviceNumber).Channels);
            waveIn = new WaveInProvider(sourceStream);

            // add handle events
        }

        private ISampleProvider CreateInputStream()
        {
            var inputStream = new SampleChannel(waveIn);
            var sampleStream = new NotifyingSampleProvider(inputStream);
            sampleStream.Sample += (s, e) => aggregator.Add(e.Left);
            return sampleStream;
        }

        public void SetRecordingDevice(int deviceNumber)
        {
            if (deviceNumber <WaveIn.DeviceCount)
                sourceStream.DeviceNumber = deviceNumber;
        }

        protected void BasicClose()
        {
            
        }

        public abstract void Initialize();
        public abstract void Play();
        public abstract void Stop();
        public abstract void Close();
    }
}
