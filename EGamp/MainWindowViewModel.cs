using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using EGamp.AudioEngine;
using EGamp.Visualization;
using NAudio.CoreAudioApi;
using NAudio.Wave;

namespace EGamp
{
    class MainWindowViewModel : INotifyPropertyChanged
    {
        private int volume;
        private BasicAudioEngine engine;
        private SpectrumAnalyzerVisualization spectrumAnalyzerVisualization;
        private PolylineWaveFormVisualization polylineWaveFormVisualization;

        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand PlayCommand { get; private set; }

        public MainWindowViewModel()
        {
            Configuration.LoadConfiguration();
            Logger.Initialize();
            engine = new FileAudioEngine("C:\\Users\\dries\\Music\\TheCatalyst.mp3");
            //engine = new WaveOutAudioEngine();
            engine.Initialize();
            engine.MaximumCalculated += new EventHandler<MaxSampleEventArgs>(audioGraph_MaximumCalculated);
            engine.FftCalculated += new EventHandler<FftEventArgs>(audioGraph_FftCalculated);

            spectrumAnalyzerVisualization = new SpectrumAnalyzerVisualization();
            polylineWaveFormVisualization = new PolylineWaveFormVisualization();

            this.volume = 100;

            PlayCommand = new RelayCommand(
                        () => this.Play(),
                        () => true);
        }

        public object InputVisualization
        {
            get
            {
                return this.spectrumAnalyzerVisualization.Content;
            }
        }

        public object OutputVisualization
        {
            get
            {
                return this.polylineWaveFormVisualization.Content;
            }
        }

        private void Play()
        {
            if (engine.IsPlaying())
                engine.Stop();
            else
                engine.Play();
        }

        public int Volume
        {
            get
            {
                return volume;
            }
            set
            {
                if (volume != value)
                {
                    this.volume = value;
                    if (this.engine != null)
                    {
                        this.engine.SetVolume((float)value/100);
                    }
                    RaisePropertyChanged("Volume");
                }
            }
        }

        void audioGraph_FftCalculated(object sender, FftEventArgs e)
        {
            spectrumAnalyzerVisualization.OnFftCalculated(e.Result);
        }

        void audioGraph_MaximumCalculated(object sender, MaxSampleEventArgs e)
        {
            polylineWaveFormVisualization.OnMaxCalculated(e.MinSample, e.MaxSample);
        }

        public void Dispose()
        {
            Configuration.SaveConfiguration();
            Logger.Close();
            engine.Close();
        }

        private void RaisePropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
