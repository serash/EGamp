using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NAudio.Wave;

namespace EGamp.Effects
{
    public class DefaultWaveProvider : WaveProvider32
    {
        float[] buffer = new float[1024];
        private int sampleCount = 0;
        private int offset = 0;

        public DefaultWaveProvider()
            : base(44100, 2)
        {
        }

        public void AddSamples(float[] buffer, int offSet, int BytesRecorded)
        {
            this.sampleCount = BytesRecorded;
            this.buffer = buffer;
            this.offset = offset;
        }

        public override int Read(float[] buffer, int offset, int sampleCount)
        {
            buffer = this.buffer;
            offset = this.offset;
            sampleCount = this.sampleCount;
            return sampleCount;


            //int sampleRate = WaveFormat.SampleRate;

            //double freqCoef1 = 2 * Math.PI * Frequency1;
            //double freqCoef2 = 2 * Math.PI * Frequency2;

            //for (int n = 0; n < sampleCount; n += 2)
            //{
            //    buffer[offset++] = (float)(Amplitude1 * Math.Sin((freqCoef1 * sample) / sampleRate));
            //    buffer[offset++] = (float)(Amplitude2 * Math.Sin((freqCoef2 * sample) / sampleRate));
            //    if (++sample >= sampleRate) sample = 0;
            //}

            //return sampleCount;
        }
    }
}
