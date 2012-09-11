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
    class FileAudioEngine : BasicAudioEngine
    {

        private WaveOut wavePlayer = null;

        public FileAudioEngine(String filename) : base()
        {
            wavePlayer = new WaveOut();
            if(System.IO.File.Exists(filename))
            {
                waveStream = new EffectStream(new Mp3FileReader(filename));
            }
            else 
                throw new Exception("File doesn't exist");
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
