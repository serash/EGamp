using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NAudio.Wave;
using NAudio.Wave.SampleProviders;

namespace EGamp.AudioEngine
{
    class WaveOutAudioEngine : BasicAudioEngine
    {

        private WaveOut wavePlayer = null;

        public WaveOutAudioEngine()
            : base()
        {
            wavePlayer = new WaveOut();
        }


        public override void Initialize()
        {
            BasicInitialize();
            var inputStream = new SampleChannel(waveIn);
            var sampleStream = new NotifyingSampleProvider(inputStream);
            sampleStream.Sample += (s, e) => aggregator.Add(e.Left);
            wavePlayer.Init(new SampleToWaveProvider(sampleStream));
        }

        public override void Play()
        {
            wavePlayer.Play();
        }

        public override void Stop()
        {
            wavePlayer.Stop();
        }

        public override void Close()
        {
            Stop();
            BasicClose();
            wavePlayer.Dispose();
        }

        public override void SetVolume(float volume)
        {
            wavePlayer.Volume = volume;
        }

        public override bool IsPlaying()
        {
            return wavePlayer.PlaybackState == PlaybackState.Playing;
        }
    }
}
