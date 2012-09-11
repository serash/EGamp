using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EGamp.AudioEngine.Effects
{
    public class EffectChain : List<Effect>
    {
        public EffectChain():base()
        {
        }

        public bool MoveUp(Effect effect)
        {
            if (this.Contains(effect))
            {
                int oldIndex = this.IndexOf(effect);
                if(oldIndex + 1 < this.Count) 
                {
                    Swap(oldIndex, oldIndex + 1);
                }
            }
            return false;
        }

        public bool MoveDown(Effect effect)
        {
            if (this.Contains(effect))
            {
                int oldIndex = this.IndexOf(effect);
                if (oldIndex - 1 > 0)
                {
                    Swap(oldIndex - 1, oldIndex);
                }
            }
            return false;
        }

        private void Swap(int index1, int index2)
        {
            Effect temp = this[index1];
            this[index1] = this[index2];
            this[index2] = temp;
        }
    }
}
