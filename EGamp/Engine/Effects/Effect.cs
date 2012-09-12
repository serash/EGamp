using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using EGamp.Visualization;

namespace EGamp.Engine.Effects
{
    public abstract class Effect
    {

        private List<CustomSlider> sliders;
        private Random rnd;
        public float SampleRate { get; set; }
        public float Tempo { get; set; }
        public bool Enabled { get; set; }
        public string Name { get; set; }

        // DEFAULT CONSTRUCTOR REQUIRED
        public Effect()
        {
            rnd = new Random();
            sliders = new List<CustomSlider>();
            Enabled = true;
            Tempo = 120;
            SampleRate = 44100;
        }

        public IList<CustomSlider> Sliders { get { return sliders; } }

        public CustomSlider AddSlider(float defaultValue, float minimum, float maximum, float increment, string description)
        {
            CustomSlider slider = new CustomSlider(defaultValue, minimum, maximum, increment, description);
            slider.ValueChanged += new RoutedPropertyChangedEventHandler<double>(Slider);
            sliders.Add(slider);
            return slider;
        }

        /// <summary>
        /// Should be called on effect load, 
        /// sample rate changes, and start of playback
        /// </summary>
        public virtual void Init()
        { }

        /// <summary>
        /// will be called when a slider value has been changed
        /// </summary>
        public abstract void Slider(object o, RoutedPropertyChangedEventArgs<double> e);

        /// <summary>
        /// called before each block is processed
        /// </summary>
        public virtual void Block()
        { }

        /// <summary>
        /// called for each sample
        /// </summary>
        public abstract void Sample(ref float spl0, ref float spl1);


        protected const float PI = (float)3.14159265;
        protected const float E =  (float)2.71828182;

        protected float sin(float x)
        {
            return (float)Math.Sin((double)x);
        }

        protected float cos(float x)
        {
            return (float)Math.Cos((double)x);
        }

        protected float tan(float x)
        {
            return (float)Math.Tan((double)x);
        }

        protected float asin(float x)
        {
            return (float)Math.Asin((double)x);
        }

        protected float acos(float x)
        {
            return (float)Math.Acos((double)x);
        }

        protected float atan(float x)
        {
            return (float)Math.Atan((double)x);
        }

        protected float atan2(float x, float y)
        {
            return (float)Math.Atan((double)x/(double)y);
        }

        protected float sqr(float x)
        {
            return pow(x, 2);
        }

        protected float sqrt(float x)
        {
            return (float)Math.Sqrt((double)x);
        }

        protected float pow(float x, float y)
        {
            return (float)Math.Pow((double)x, (double)y);
        }

        protected float exp(float x)
        {
            return pow(E, x);
        }

        protected float log(float x)
        {
            return (float)Math.Log((double)x);
        }

        protected float log10(float x)
        {
            return (float)Math.Log10((double)x);
        }

        protected float abs(float x)
        {
            return Math.Abs(x);
        }

        protected float min(float x, float y)
        {
            return Math.Min(x, y);
        }

        protected float max(float x, float y)
        {
            return Math.Max(x, y);
        }

        protected float sign(float x)
        {
            return (float)Math.Sign(x);
        }

        protected float rand(float x)
        {
            return (float)rnd.Next(0, (int)x);
        }

        protected float floor(float x)
        {
            return (float)Math.Floor(x);
        }

        protected float ceil(float x)
        {
            return (float)Math.Ceiling(x);
        }

        protected float invsqrt(float x)
        {
            return sqrt(1/x);
        }

        protected float mod(float x, float y)
        {
            return x % y;
        }
    }
}
