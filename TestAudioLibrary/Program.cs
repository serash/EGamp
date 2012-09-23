using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using AudioLibrary;
using EffectsLibrary;

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

        static void waitForStopCapturing()
        {
            ConsoleKeyInfo c = Console.ReadKey(true);
            while(c.Key != ConsoleKey.Q)
                c = Console.ReadKey(true);
        }

        static void waitforEnter()
        {
            Console.WriteLine("press enter to exit");
            // pause at end of execution
            Console.Read();
        }

        static void Main(string[] args)
        {
            AudioEngine ae = new AudioEngine(100);
            int result = ae.engineStatus();
            if (AudioEngine.Failed(result))
            {
                Console.WriteLine("Error initializing AudioEngine: " + AudioEngine.getErrorCode(result));
                waitforEnter();
                return;
            }

            Console.WriteLine("Select Render Device: ");
            String[] renderDevices = ae.getRenderDevices();
            for (int i = 0; i < renderDevices.Length; i++)
                Console.WriteLine(i + ": " + renderDevices[i]);
            result = ae.setDefaultRenderDevice();
            if (AudioEngine.Failed(result))
            {
                Console.WriteLine("Error setting render device: " + AudioEngine.getErrorCode(result));
                waitforEnter();
                return;
            }
            ae.initializeRenderDevice();


            Console.WriteLine("Select Capture Device: ");
            String[] captureDevices = ae.getCaptureDevices();
            for (int i = 0; i < captureDevices.Length; i++)
                Console.WriteLine(i + ": " + captureDevices[i]);
            result = ae.setDefaultCaptureDevice();
            if (AudioEngine.Failed(result))
            {
                Console.WriteLine("Error setting capture device: " + AudioEngine.getErrorCode(result));
                waitforEnter();
                return;
            }
            ae.initializeCaptureDevice();


            result = ae.startAudioStream();
            if (AudioEngine.Failed(result))
            {
                Console.WriteLine("Error starting stream: " + AudioEngine.getErrorCode(result));
                waitforEnter();
                return;
            }


            Console.WriteLine("press Q to stop capturing");
            waitForStopCapturing();
            result = ae.stopAudioStream();
            if (result < 0)
            {
                Console.WriteLine("Error stopping stream: " + AudioEngine.getErrorCode(result));
                waitforEnter();
                return;
            }
            ae.dispose();

            Amplifier amp = new Amplifier();
            amp.getAmplify().setValue(20);

            Tremolo tremolo = new Tremolo();
            tremolo.getFrequency().setValue(3.5);
            tremolo.getStereoSeperation().setValue(0.5);
            waitforEnter();
        }
    }
}