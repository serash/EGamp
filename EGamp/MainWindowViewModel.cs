using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Input;
using EGamp.Engine;
using EGamp.Engine.Effects;
using EGamp.Visualization;
using NAudio.CoreAudioApi;
using NAudio.Wave;

namespace EGamp
{
    class MainWindowViewModel : INotifyPropertyChanged
    {
        private int volume;
        private AudioEngine engine;
        private SpectrumAnalyzerVisualization spectrumAnalyzerVisualization;
        private PolylineWaveFormVisualization polylineWaveFormVisualization;
        private EffectCollection effectWindow;

        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand PlayCommand { get; private set; }
        public ICommand AddEffectCommand { get; private set; }

        private IEnumerable<Type> effectsList;
        private Type selectedEffect;

        public MainWindowViewModel()
        {
            Configuration.LoadConfiguration();
            Logger.Initialize();
            engine = new AudioEngine();
            engine.Initialize();
            engine.MaximumCalculated += new EventHandler<MaxSampleEventArgs>(audioGraph_MaximumCalculated);
            engine.EffectAdded += new EventHandler<AddEffectEventArgs>(effectWindow_AddEffect);
            engine.FftCalculated += new EventHandler<FftEventArgs>(audioGraph_FftCalculated);

            spectrumAnalyzerVisualization = new SpectrumAnalyzerVisualization();
            polylineWaveFormVisualization = new PolylineWaveFormVisualization();
            effectWindow = new EffectCollection();

            this.volume = 100;

            PlayCommand = new RelayCommand(
                        () => this.Play(),
                        () => true);
            AddEffectCommand = new RelayCommand(
                        () => this.AddEffect(),
                        () => true);
            effectsList = GetEnumerableOfType<Effect>();
            SelectedEffect = effectsList.First();
        }

        public IEnumerable<Type> Effects
        {
            get { return effectsList; }
        }

        public Type SelectedEffect
        {
            get { return selectedEffect; }
            set
            {
                if (selectedEffect != null && selectedEffect.Equals(value)) return;
                selectedEffect = value;
                RaisePropertyChanged("SelectedEffect");
            }
        }

        public IEnumerable<Type> GetEnumerableOfType<T>() where T : class
        {
            List<Type> objects = new List<Type>();
            foreach (Type type in Assembly.GetAssembly(typeof(T)).GetTypes().Where(myType => myType.IsClass && !myType.IsAbstract && myType.IsSubclassOf(typeof(T))))
            {
                objects.Add(type);
            }
            return objects;
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

        public object EffectsWindow
        {
            get
            {
                return this.effectWindow.Content;
            }
        }

        private void Play()
        {
            if (engine.IsPlaying())
                engine.Stop();
            else
                engine.Play();
        }

        private void AddEffect()
        {
            Effect effect = (Effect)Activator.CreateInstance(selectedEffect);
            engine.AddEffect(effect);
        }

        private void effectWindow_AddEffect(object sender, AddEffectEventArgs e)
        {
            this.effectWindow.AddEffect(e.NewEffect);
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
