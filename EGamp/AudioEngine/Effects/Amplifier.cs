using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using EGamp.Visualization;
using NAudio.Wave;
using System.ComponentModel.Design;

namespace EGamp.AudioEngine.Effects
{
    public class Amplifier : Effect
    {
        private float amp;
        public Amplifier()
        {
            AddSlider(1, 1, 10, 1, "amount (dB)"); 
            Name = "Amplifier";
        }

        public override void Init()
        {
            base.Init();
            amp = 1;
        }


        public override void Slider(object o, RoutedPropertyChangedEventArgs<double> e)
        {
            if (e.Source.Equals(Sliders[0].MainSlider))
                amp = sqrt(pow(10, (float)e.NewValue/10));
        }

        public override void Sample(ref float spl0, ref float spl1)
        {
            spl0 = spl0 * amp;
            spl1 = spl1 * amp;
        }
    }
}
