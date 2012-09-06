﻿using System;
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
        private BasicAudioEngine engine;
        private int selectedIndex;
        private List<IVisualizationPlugin> visualizations;
        private IVisualizationPlugin selectedVisualization;

        public event PropertyChangedEventHandler PropertyChanged;

        public ICommand RecordCommand { get; private set; }
        public ICommand PlayCommand { get; private set; }
        public ICommand StopCommand { get; private set; }
        public ICommand ChangeVisualizationCommand { get; private set; }
        public ICommand TestCommand { get; private set; }

        public MainWindowViewModel()
        {
            Configuration.LoadConfiguration();
            Logger.Initialize();
            //engine = new BasicAudioEngine();
            //engine.Initialize();
            //engine.MaximumCalculated += new EventHandler<MaxSampleEventArgs>(audioGraph_MaximumCalculated);
            //engine.FftCalculated += new EventHandler<FftEventArgs>(audioGraph_FftCalculated);

            this.visualizations = new List<IVisualizationPlugin>();
            visualizations.Add(new SpectrumAnalyzerVisualization());
            visualizations.Add(new PolylineWaveFormVisualization());
            selectedIndex = 0;
            this.selectedVisualization = this.visualizations.ElementAt(selectedIndex);

            PlayCommand = new RelayCommand(
                        () => this.Play(),
                        () => true);
            RecordCommand = new RelayCommand(
                        () => this.Record(),
                        () => true);
            StopCommand = new RelayCommand(
                        () => this.Stop(),
                        () => true);
            ChangeVisualizationCommand = new RelayCommand(
                        () => this.ChangeVisualization(),
                        () => true);
            TestCommand = new RelayCommand(
                        () => this.Test(),
                        () => true);
        }

        public object InputVisualization
        {
            get
            {
                return this.visualizations.ElementAt(0).Content;
            }
        }

        public object OutputVisualization
        {
            get
            {
                return this.visualizations.ElementAt(1).Content;
            }
        }

        private void Record()
        {
            engine.StartRecording();
        }

        private void Play()
        {
            engine.Start();
        }

        private void Stop()
        {
            engine.Stop();
            engine.StopRecording();
        }

        private void ChangeVisualization()
        {
            this.selectedIndex = (this.selectedIndex + 1) % 2;
            Logger.Log("Current index: " + selectedIndex);
            this.selectedVisualization = this.visualizations[selectedIndex];
            RaisePropertyChangedEvent("Visualization");
        }

        void audioGraph_FftCalculated(object sender, FftEventArgs e)
        {
            if (selectedVisualization != null)
            {
                visualizations[0].OnFftCalculated(e.Result);
            }
        }

        void audioGraph_MaximumCalculated(object sender, MaxSampleEventArgs e)
        {
            if (this.selectedVisualization != null)
            {
                visualizations[1].OnMaxCalculated(e.MinSample, e.MaxSample);
            }
        }

        public void Dispose()
        {
            Configuration.SaveConfiguration();
            Logger.Close();
            engine.Close();
        }

        private WaveIn sourceStream = null;
        private WasapiOut waveOut = null;
        public void Test()
        {
            sourceStream = new WaveIn();
            int deviceNumber = 0;
            sourceStream.DeviceNumber = deviceNumber;
            sourceStream.WaveFormat = new WaveFormat(44100, WaveIn.GetCapabilities(deviceNumber).Channels);

            WaveInProvider waveIn = new WaveInProvider(sourceStream);

            waveOut = new WasapiOut(AudioClientShareMode.Shared, true, 100);
            waveOut.Init(waveIn);
            sourceStream.StartRecording();
            waveOut.Play();
        }



        private void RaisePropertyChangedEvent(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
