using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EGamp.Effects
{
    interface IEffect
    {
        byte[] Apply(byte[] buffer, int bytesRecorded);
    }
}
