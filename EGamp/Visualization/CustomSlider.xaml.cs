using System;
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

namespace EGamp.Visualization
{
    /// <summary>
    /// Interaction logic for CustomSlider.xaml
    /// </summary>
    public partial class CustomSlider : UserControl
    {
        public CustomSlider()
            : this(100, 0, 100, 1, "")
        {
            InitializeComponent();
        }

        public float DefaultValue { get; set; }
        public float Minimum { get; set; }
        public float Maximum { get; set; }
        public float Increment { get; set; }
        public float Value { get; set; }
        public string Description { get; set; }

        private event RoutedPropertyChangedEventHandler<double> valueChangedHandler; 

        public CustomSlider(float _defaultValue, float _minimum, float _maximum, float _increment, string _description)
        {
            InitializeComponent();
            this.DefaultValue = _defaultValue;
            this.Minimum = _minimum;
            this.Maximum = _maximum;
            this.Increment = _increment;
            this.Description = _description;
            MainSlider.Maximum = this.Maximum;
            MainSlider.Minimum = this.Minimum;
            MainSlider.Value = this.DefaultValue;
            MainSlider.SmallChange = this.Increment;
            Name.Content = Description;
        }

        public event RoutedPropertyChangedEventHandler<double> ValueChanged
        {
            add { valueChangedHandler += value; }
            remove { valueChangedHandler -= value; }
        }

        private void MainSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            this.Value = (float) (e.NewValue);
            Val.Content = Math.Round(this.Value, 2);
            if(sender != null && valueChangedHandler != null)
                    valueChangedHandler(sender, e);
        }
    }
}
