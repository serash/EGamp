using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NAudio.Wave;

namespace EGamp.AudioEngine
{
    class WaveOutAudioEngine : BasicAudioEngine
    {

        private WaveOut wavePlayer = null;

        public WaveOutAudioEngine()
            : base()
        {
            wavePlayer = new WaveOut();
            wavePlayer.Init(waveIn);
        }


        public override void Initialize()
        {
            wavePlayer.Init(waveIn);
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
    }
}
