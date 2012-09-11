using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EGamp.AudioEngine.Effects;
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
            wavePlayer.NumberOfBuffers = 2;
        }


        public override void Initialize()
        {
            BasicInitialize();
            wavePlayer.Init(new SampleToWaveProvider(this.createSampleStream()));
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
