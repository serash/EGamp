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
using EGamp.Engine.Effects;

namespace EGamp.Visualization
{
    /// <summary>
    /// Interaction logic for EffectCollection.xaml
    /// </summary>
    public partial class EffectCollection : UserControl
    {
        private List<EffectVisualization> effectsList; 
        public EffectCollection()
        {
            InitializeComponent();
            effectsList = new List<EffectVisualization>();
        }

        public void AddEffect(Effect effect)
        {
            EffectVisualization newEffect = new EffectVisualization(effect);
            effectsList.Add(newEffect);
            Effects.Children.Add(newEffect);
        }
    }
}
