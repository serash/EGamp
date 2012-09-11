﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using EGamp.AudioEngine.Effects;

namespace EGamp.Visualization
{
    /// <summary>
    /// Interaction logic for EffectVisualization.xaml
    /// </summary>
    public partial class EffectVisualization : UserControl
    {
        private Effect effect;

        public Effect ThisEffect {
            get { return effect; }
            set
            {
                effect = value;
                NameLabel.Content = effect.Name;
                foreach (CustomSlider slider in effect.Sliders)
                {
                    Sliders.Children.Add(slider);
                }
            }
        }

        public EffectVisualization(Effect _effect)
        {
            InitializeComponent();
            this.ThisEffect = _effect;
        }
    }
}