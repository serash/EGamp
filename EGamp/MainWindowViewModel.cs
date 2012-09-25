using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Input;
using EGamp.Visualization;
using EffectsLibrary;
using AudioLibrary;

namespace EGamp
{
    class MainWindowViewModel : INotifyPropertyChanged
    {
        private int volume;
        private AudioEngine engine;
        private EffectCollection effectWindow;

        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand StartCommand { get; private set; }
        public ICommand MuteCommand { get; private set; }
        public ICommand AddEffectCommand { get; private set; }

        private IEnumerable<Type> effectsList;
        private Type selectedEffect;

        private List<String> captureDevices;
        private List<String> renderDevices;
        private String captureDevice;
        private String renderDevice;

        public MainWindowViewModel()
        {
            Configuration.LoadConfiguration();
            Logger.Initialize();
            engine = new AudioEngine(5, false);
            int result = engine.initialize();
            if (AudioEngine.Failed(result))
                Logger.Log("Error initializing AudioEngine: " + AudioEngine.getErrorCode(result));
            renderDevices = engine.getRenderDevices().ToList();
            renderDevices.Add("Default Render Device");
            captureDevices = engine.getCaptureDevices().ToList();
            captureDevices.Add("Default Capture Device");

            captureDevice = captureDevices.Last();
            renderDevice = renderDevices.Last();

            effectWindow = new EffectCollection();

            this.volume = 100;

            StartCommand = new RelayCommand(
                        () => this.Start(),
                        () => true);
            MuteCommand = new RelayCommand(
                        () => this.Mute(),
                        () => true);
            AddEffectCommand = new RelayCommand(
                        () => this.AddEffect(),
                        () => true);

        }

        public string CaptureDevice
        {
            get { return captureDevice; }
            set
            {
                if (captureDevice != value)
                {
                    captureDevice = value;
                    //uint idx = (uint)captureDevices.IndexOf(captureDevice);
                    //int result;
                    //if (idx == captureDevices.Count - 1)
                    //    result = engine.setDefaultCaptureDevice();
                    //else
                    //    result = engine.setCaptureDevice(idx);
                    //if (AudioEngine.Failed(result))
                    //    Logger.Log("Error setting capture device: " + AudioEngine.getErrorCode(result));

                    //result = engine.initializeDevices();
                    //if (AudioEngine.Failed(result))
                    //    Logger.Log("Error Initializing devices: " + AudioEngine.getErrorCode(result));
                    //RaisePropertyChanged("CaptureDevice");
                }
            }
        }

        public string RenderDevice
        {
            get { return renderDevice; }
            set
            {
                if (renderDevice != value)
                {
                    renderDevice = value;
                    //uint idx = (uint)renderDevices.IndexOf(renderDevice);
                    //int result;
                    //if (idx == renderDevices.Count - 1)
                    //    result = engine.setDefaultRenderDevice();
                    //else
                    //    result = engine.setRenderDevice(idx);
                    //if (AudioEngine.Failed(result))
                    //    Logger.Log("Error setting render device: " + AudioEngine.getErrorCode(result));

                    //result = engine.initializeDevices();
                    //if (AudioEngine.Failed(result))
                    //    Logger.Log("Error Initializing devices: " + AudioEngine.getErrorCode(result));
                    //RaisePropertyChanged("RenderDevice");
                }
            }
        }

        public object CaptureDevices
        {
            get { return this.captureDevices; }
        }

        public object RenderDevices
        {
            get { return this.renderDevices; }
        }

        public object EffectsWindow
        {
            get { return this.effectWindow.Content; }
        }

        private void Start()
        {
            int result = engine.startAudioStream();
            if (AudioEngine.Failed(result))
                Logger.Log("Error starting stream: " + AudioEngine.getErrorCode(result));
        }
        
        private void Mute()
        {
            int result = engine.toggleMute();
            if (AudioEngine.Failed(result))
                Logger.Log("Error toggling mute: " + AudioEngine.getErrorCode(result));
        }

        private void AddEffect()
        {
            Logger.Log("AddEffect not yet implemented.");
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
                        this.engine.setVolume((float)value/100);
                    }
                    RaisePropertyChanged("Volume");
                }
            }
        }

        public void Dispose()
        {
            Configuration.SaveConfiguration();
            Logger.Close();
            engine.dispose();
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
