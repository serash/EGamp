using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NAudio.CoreAudioApi;
using NAudio.Wave;
using NAudio;
using NAudio.Wave.SampleProviders;
namespace EGamp.AudioEngine
{
    class FileAudioEngine : BasicAudioEngine
    {

        private WaveOut wavePlayer = null;
        protected WaveStream waveStream = null;

        public FileAudioEngine(String filename) : base()
        {
            wavePlayer = new WaveOut();
            waveStream = new Mp3FileReader(filename);
        }
        
        public override void Initialize()
        {
            var inputStream = new SampleChannel(waveStream);
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
            waveStream.Dispose();
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
