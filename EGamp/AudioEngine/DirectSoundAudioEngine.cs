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
    class DirectSoundAudioEngine : BasicAudioEngine
    {

        private DirectSoundOut wavePlayer = null;

        public DirectSoundAudioEngine()
            : base()
        {
            wavePlayer = new DirectSoundOut(DirectSoundOut.DSDEVID_DefaultPlayback);
        }


        public override void Initialize()
        {
            BasicInitialize();
            wavePlayer.Init(waveStream);
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
