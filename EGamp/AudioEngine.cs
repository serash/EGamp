using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EGamp.Effects;
using NAudio.CoreAudioApi;
using NAudio.Wave;
using NAudio;
using NAudio.Wave.SampleProviders;

namespace EGamp
{
    class AudioEngine
    {
        private IWavePlayer playbackDevice;
        private BufferedWaveProvider _bufferedWaveProvider;
        private DefaultWaveProvider waveProvider;
        private WaveIn _waveInStream;
        private SampleAggregator aggregator;
        private bool _isRecording;
        private List<IEffect> effects;

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

        public AudioEngine()
        {
            aggregator = new SampleAggregator();
            aggregator.NotificationCount = 882;
            aggregator.PerformFFT = true;

            effects = new List<IEffect>();
            effects.Add(new Amplifier());
            playbackDevice = new WaveOut();
            _isRecording = false;
            _waveInStream = new WaveIn();
            _waveInStream.BufferMilliseconds = 100;
            //waveProvider = new DefaultWaveProvider();
            _bufferedWaveProvider = new BufferedWaveProvider(_waveInStream.WaveFormat);
            _bufferedWaveProvider.DiscardOnBufferOverflow = true;
            _bufferedWaveProvider.BufferLength = 5 * 1024;

            // add handle events
            _waveInStream.DataAvailable += new EventHandler<WaveInEventArgs>(WaveInStreamDataAvailable);
        }

        public void Initialize()
        {
            var inputStream = CreateInputStream();
            playbackDevice.Init(new SampleToWaveProvider(inputStream));
        }

        private ISampleProvider CreateInputStream()
        {
            var inputStream = new SampleChannel(_bufferedWaveProvider);
            //var inputStream = new SampleChannel(waveProvider);
            var sampleStream = new NotifyingSampleProvider(inputStream);
            sampleStream.Sample += (s, e) => aggregator.Add(e.Left);
            return sampleStream;
        }

        public void StartPlaying()
        {
            playbackDevice.Play();
            Logger.Log("Starting playing...");
        }

        public void StopPlaying()
        {
            playbackDevice.Pause();
        }

        public void StartRecording()
        {
            if(!_isRecording)
                _waveInStream.StartRecording();
            _isRecording = true;
        }

        public void StopRecording()
        {
            if(_isRecording)
                _waveInStream.StopRecording();
            _isRecording = false;
        }

        public void Close()
        {
            if(_isRecording)
                _waveInStream.StopRecording();
            _waveInStream.Dispose();
            _waveInStream = null;

            if (playbackDevice != null)
            {
                playbackDevice.Stop();
            }
            if (playbackDevice != null)
            {
                playbackDevice.Dispose();
                playbackDevice = null;
            }
        }

        public void SetRecordingDevice(int deviceNumber)
        {
        }

        void WaveInStreamDataAvailable(object sender, WaveInEventArgs e)
        {
            //byte[] newAudio = effects[0].Apply(e.Buffer, e.BytesRecorded);  
            float[] buffer = new float[e.BytesRecorded];
            for (int index = 0; index < e.BytesRecorded; index += 2)
            {
                short sample = (short)(((short)e.Buffer[index + 1] << 8) | (short)e.Buffer[index + 0]);
                Logger.Log(Convert.ToString(sample));
                buffer[index] = sample/32768f;
            }
            //waveProvider.AddSamples(buffer, 0, e.BytesRecorded);
            _bufferedWaveProvider.AddSamples(e.Buffer, 0, e.BytesRecorded);
        }
    }
}
