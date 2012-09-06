using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NAudio.Wave;

namespace EGamp.AudioEngine.Effects
{
    class Amplifier : IEffect
    {
        private int shift;

        public Amplifier()
        {
            shift = 2;
        }

        byte[] IEffect.Apply(byte[] buffer, int bytesRecorded)
        {
            byte[] newbuffer = new byte[bytesRecorded];
            for (int index = 0; index < bytesRecorded; index += 2)
            {
                short sample = (short)((buffer[index + 1] << 8) |
                                        buffer[index + 0]);
                newbuffer[index + 1] = (byte)((buffer[index + 1] << shift) | (buffer[index + 1] >> (8-shift)));
                newbuffer[index + 0] = (byte)(buffer[index + 0] << shift);
                short newsample = (short)(((short)newbuffer[index + 1] << 8) |
                                        (short)newbuffer[index + 0]);
                //float sample32 = sample / 32768f;
                Logger.Log("sample: " + sample + " => " + newsample);
            }
            return newbuffer;
        }
    }
}
