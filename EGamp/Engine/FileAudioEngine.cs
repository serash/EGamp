using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EGamp.Engine.Asio;
using EGamp.Engine.Effects;
using NAudio.Wave;
using NAudio.Wave.SampleProviders;

namespace EGamp.Engine
{
    public class FileAudioEngine : IAudioEngine
    {
        private IWaveProvider waveIn = null;
        private SampleAggregator aggregator = null;
        private EffectStream waveStream = null;
        private IWavePlayer wavePlayer = null;

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

        public FileAudioEngine()
        {
            aggregator = new SampleAggregator();
            aggregator.NotificationCount = 882;
            aggregator.PerformFFT = true;

            // asioOut
            //wavePlayer = new WaveOut();
            wavePlayer = new MyAsioOut(AsioOut.GetDriverNames()[0]);
        }

        private ISampleProvider createSampleStream()
        {
            var inputStream = new SampleChannel(waveStream);
            var sampleStream = new NotifyingSampleProvider(inputStream);
            //sampleStream.Sample += (s, e) => aggregator.Add(e.Left);
            return sampleStream;
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
        }

        public void Initialize()
        {
            
        }
        
        public void Initialize(String filename)
        {
            waveIn = new Mp3FileReader("C:\\TheCatalyst.mp3");
            waveStream = new EffectStream(new WaveProviderStream(waveIn));
            wavePlayer.Init(new SampleToWaveProvider(this.createSampleStream()));
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
            wavePlayer.Dispose();
        }

        public void SetVolume(float volume)
        {
            wavePlayer.Volume = volume;
        }

        public bool IsPlaying()
        {
            return wavePlayer.PlaybackState == PlaybackState.Playing;
        }

        public void AudioAvailableEvent(object sender, Asio.AsioAudioAvailableEventArgs e)
        {
        }
    }
}
