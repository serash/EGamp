using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EGamp
{
    static class Logger
    {
        private static String _fileName;
        private static TextWriter tw;
        private static bool isOpen;
        public static void Initialize()
        {
            _fileName = Configuration.GetString("logFile");
            tw = new StreamWriter(_fileName);
            isOpen = true;
        }

        public static void Close()
        {
            tw.Close();
            isOpen = false;
        }
        public static void Log(String str)
        {
            if(isOpen)
                tw.WriteLine(str);
        }
    }
}
