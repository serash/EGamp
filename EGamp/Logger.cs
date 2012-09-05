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
        public static void Initialize()
        {
            _fileName = Configuration.GetString("logFile");
            tw = new StreamWriter(_fileName);
        }

        public static void Close()
        {
            tw.Close();
        }
        public static void Log(String str)
        {
            tw.WriteLine(str);
        }
    }
}
