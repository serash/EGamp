using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace EGamp.Visualization
{
    public interface IWaveFormRenderer
    {
        void AddValue(float maxValue, float minValue);
    }
}
