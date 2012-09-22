using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EGamp.Engine.Asio
{
    internal class AsioDriverCapability
    {
        public String DriverName;

        public int NbInputChannels;
        public int NbOutputChannels;

        public int InputLatency;
        public int OutputLatency;

        public int BufferMinSize;
        public int BufferMaxSize;
        public int BufferPreferredSize;
        public int BufferGranularity;

        public double SampleRate;

        public ASIOChannelInfo[] InputChannelInfos;
        public ASIOChannelInfo[] OutputChannelInfos;
    }
}
