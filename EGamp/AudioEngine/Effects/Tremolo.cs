using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;


namespace EGamp.AudioEngine.Effects
{
    public class Tremolo : Effect
    {
        float adv, sep, amount, sc, pos;

        public Tremolo()
        {
            AddSlider(4, 0, 100, 1, "frequency (Hz)");
            AddSlider(-6, -60, 0, 1, "amount (dB)");
            AddSlider(0, 0, 1, 0.1f, "stereo separation (0..1)");
            Name = "Tremolo";
        }

        public override void Init()
        {
            base.Init();
            adv = PI * 2 * 4 / SampleRate;
            sep = -6 * PI;
            amount = pow(2, 0 / 6);
            sc = 0.5f * amount; amount = 1 - amount;
        }

        public override void Slider(object o, RoutedPropertyChangedEventArgs<double> e)
        {
            if (e.Source.Equals(Sliders[0].MainSlider))
                adv = PI * 2 * (float)e.NewValue / SampleRate;
            else if (e.Source.Equals(Sliders[1].MainSlider))
                sep = (float)e.NewValue * PI;
            else if (e.Source.Equals(Sliders[2].MainSlider))
            {
                amount = pow(2, (float)e.NewValue / 6);
                sc = 0.5f * amount; amount = 1 - amount;
            }
        }

        public override void Sample(ref float spl0, ref float spl1)
        {
            spl0 = spl0 * ((cos(pos) + 1) * sc + amount);
            spl1 = spl1 * ((cos(pos + sep) + 1) * sc + amount);
            pos += adv;
        }
    }
}
