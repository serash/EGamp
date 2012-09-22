using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AudioLibrary;

namespace TestAudioLibrary
{
    class Program
    {
        static uint getNextUInt(int min, int max)
        {
            uint val = (uint)min - 1;
            val = (uint)Convert.ToUInt32(Console.ReadLine());
            while (val < (uint)min || val > (uint)max)
                val = (uint)Convert.ToUInt32(Console.ReadLine());
            return val;
        }

        static void Main(string[] args)
        {
            AudioEngine ae = new AudioEngine();
            bool result;
            long status = ae.engineStatus();
            if (status < 0)
            {
                Console.WriteLine("error initializing");
                Console.Read(); 
                return;
            }

            Console.WriteLine("Select Render Device: ");
            String[] renderDevices = ae.getRenderDevices();
            for (int i = 0; i < renderDevices.Length; i++)
                Console.WriteLine(i + ": " + renderDevices[i]);
            uint renderDevice = 1;
            result = ae.setRenderDevice(renderDevice);
            if (!result) {
                Console.WriteLine("Device " + renderDevice + " Couldn't be set");
                return;
            }


            Console.WriteLine("Select Capture Device: ");
            String[] captureDevices = ae.getCaptureDevices();
            for (int i = 0; i < captureDevices.Length; i++)
                Console.WriteLine(i + ": " + captureDevices[i]);
            uint captureDevice = 0;
            result = ae.setCaptureDevice(captureDevice);
            if (!result) {
                Console.WriteLine("Device " + captureDevice + " Couldn't be set");
                return;
            }

            ae.initializeDevices();
            result = ae.recordAudioStream();
            if (!result)
            {
                Console.WriteLine("Recording failed");
                return;
            }

            Console.WriteLine("press enter to exit");
            // pause at end of execution
            Console.Read();
        }
    }
}