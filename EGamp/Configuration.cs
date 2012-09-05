using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EGamp
{
    static class Configuration
    {
        private const String ConfigurationFile = "EGamp.ini";
        private static Dictionary<String, String> _configuration; 
        public static void LoadConfiguration()
        {
            _configuration = new Dictionary<String, String>();
            //_configuration = new Dictionary<string, string>();
            String[] config = File.ReadAllLines(ConfigurationFile); 
            foreach (String s in config)
            {
                String[] pair = s.Split('=');
                if(pair.Length == 2)
                    _configuration.Add(pair[0], pair[1]);
            }
        }

        public static String GetString(String name)
        {
            return _configuration[name];
        }

        public static void SaveConfiguration()
        {
            TextWriter tw = new StreamWriter(ConfigurationFile);
            tw.WriteLine("# EGamp Configuration File");
            tw.WriteLine("");
            foreach (KeyValuePair<String, String> pair in _configuration)
            {
                tw.WriteLine(pair.Key + "=" + pair.Value);
            }
            tw.Close();
        }
    }
}
