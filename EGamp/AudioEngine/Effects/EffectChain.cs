using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EGamp.AudioEngine.Effects
{
    class EffectChain : List<IEffect>
    {
        public EffectChain():base()
        {
        }

        public void AddEffect(IEffect effect)
        {
            
        }
    }
}
