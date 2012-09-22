using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using EGamp.Engine.Asio;
using NAudio.Wave;

namespace EGamp.Engine.Effects
{
    public class EffectStream : WaveStream
    {
        private EffectChain effects;
        private byte[] data = null;
        private int dataSize = 0;
        public WaveStream source;
        private object effectLock = new object();
        private object sourceLock = new object();

        public EffectStream(EffectChain effects, WaveStream sourceStream)
        {
            this.effects = effects;
            this.source = sourceStream;
            foreach (Effect effect in effects)
            {
                InitialiseEffect(effect);
            }

        }

        public EffectStream(WaveStream sourceStream)
            : this(new EffectChain(), sourceStream)
        {
        }

        public EffectStream(Effect effect, WaveStream sourceStream)
            : this(sourceStream)
        {
            AddEffect(effect);
        }

        public override WaveFormat WaveFormat
        {
            get { return source.WaveFormat; }
        }
        
        public bool MoveUp(Effect effect)
        {
            lock (effectLock)
            {
                return effects.MoveUp(effect);
            }
        }

        public bool MoveDown(Effect effect)
        {
            lock (effectLock)
            {
                return effects.MoveDown(effect);
            }
        }
        
        public override long Length
        {
            get { return source.Length; }
        }

        public override long Position
        {
            get { return source.Position; }
            set { lock (sourceLock) { source.Position = value; } }
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            //lock (sourceLock)
            //{
            //    read = source.Read(buffer, offset, count);
            //}
            int bytesToCopy = dataSize;
            if (dataSize > count)
                bytesToCopy = count;
            Buffer.BlockCopy(data, 0, buffer, offset, bytesToCopy);
            if (WaveFormat.BitsPerSample == 16)
            {
                lock (effectLock)
                {
                    Process16Bit(buffer, offset, bytesToCopy);
                }
            }
            return bytesToCopy;
        }

        private void Process16Bit(byte[] buffer, int offset, int count)
        {
            foreach (Effect effect in effects)
            {
                if (effect.Enabled)
                {
                    effect.Block();
                }
            }

            for (int sample = 0; sample < count / 2; sample++)
            {
                // get the sample(s)
                int x = offset + sample * 2;
                short sample16Left = BitConverter.ToInt16(buffer, x);
                short sample16Right = sample16Left;
                if (WaveFormat.Channels == 2)
                {
                    sample16Right = BitConverter.ToInt16(buffer, x + 2);
                    sample++;
                }

                // run these samples through the effects
                float sample64Left = sample16Left / 32768.0f;
                float sample64Right = sample16Right / 32768.0f;
                Logger.Log("before effect: " + sample64Left);
                foreach (Effect effect in effects)
                {
                    if (effect.Enabled)
                    {
                        effect.Sample(ref sample64Left, ref sample64Right);
                    }
                }
                Logger.Log("after effect: " + sample64Left);

                sample16Left = (short)(sample64Left * 32768.0f);
                sample16Right = (short)(sample64Right * 32768.0f);

                // put them back
                buffer[x] = (byte)(sample16Left & 0xFF);
                buffer[x + 1] = (byte)((sample16Left >> 8) & 0xFF);

                if (WaveFormat.Channels == 2)
                {
                    buffer[x + 2] = (byte)(sample16Right & 0xFF);
                    buffer[x + 3] = (byte)((sample16Right >> 8) & 0xFF);
                }
            }
        }

        private void Process32Bit(byte[] buffer, int offset, int count)
        {
            foreach (Effect effect in effects)
            {
                if (effect.Enabled)
                {
                    effect.Block();
                }
            }

            for (int sample = 0; sample < count / 4; sample++)
            {
                // get the sample(s)
                int x = offset + sample * 4;
                int sample32Left = BitConverter.ToInt32(buffer, x);
                int sample32Right = sample32Left;
                if (WaveFormat.Channels == 2)
                {
                    sample32Right = BitConverter.ToInt32(buffer, x + 4);
                    sample++;
                }

                // run these samples through the effects
                float sample64Left = sample32Left / 2147483648.0f;
                float sample64Right = sample32Right / 2147483648.0f;
                foreach (Effect effect in effects)
                {
                    if (effect.Enabled)
                    {
                        effect.Sample(ref sample64Left, ref sample64Right);
                    }
                }

                sample32Left = (short)(sample64Left * 2147483648.0f);
                sample32Right = (short)(sample64Right * 2147483648.0f);

                // put them back
                buffer[x] = (byte)(sample32Left & 0xFF);
                buffer[x + 1] = (byte)((sample32Left >> 8) & 0xFF);
                buffer[x + 2] = (byte)((sample32Left >> 16) & 0xFF);
                buffer[x + 3] = (byte)((sample32Left >> 24) & 0xFF);

                if (WaveFormat.Channels == 2)
                {
                    buffer[x + 4] = (byte)(sample32Right & 0xFF);
                    buffer[x + 5] = (byte)((sample32Right >> 8) & 0xFF);
                    buffer[x + 6] = (byte)((sample32Right >> 16) & 0xFF);
                    buffer[x + 7] = (byte)((sample32Right >> 24) & 0xFF);
                }
            }
        }

        public void AddEffect(Effect effect)
        {
            InitialiseEffect(effect);
            lock (effectLock)
            {
                this.effects.Add(effect);
            }
        }

        private void InitialiseEffect(Effect effect)
        {
            effect.SampleRate = WaveFormat.SampleRate;
            effect.Init();
        }

        public bool RemoveEffect(Effect effect)
        {
            lock (effectLock)
            {
                return this.effects.Remove(effect);
            }
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            source.Dispose();
        }

        protected void checkDataArray(int preferredSize)
        {
            if (data == null || dataSize != preferredSize)
            {
                data = new byte[preferredSize];
                dataSize = preferredSize;
            }
        }

        internal void writeFromAsioInput(IntPtr[] intPtr, int bufferSize, AsioSampleType asioSampleType)
        {
            checkDataArray(bufferSize);
            Marshal.Copy(intPtr[0], data, 0, bufferSize);
        }
    }

}
