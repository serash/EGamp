using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using EGamp.Engine.Asio;
using EGamp.Engine.Effects;

namespace EGamp.Engine
{
    interface IAudioEngine
    {
        event EventHandler<FftEventArgs> FftCalculated;
        event EventHandler<MaxSampleEventArgs> MaximumCalculated;
        event EventHandler<AddEffectEventArgs> EffectAdded;
        void AudioAvailableEvent(object sender, AsioAudioAvailableEventArgs e);
        void AddEffect(Effect effect);
        void Initialize();
        void Play();
        void Stop();
        void Close();
        void SetVolume(float volume);
        bool IsPlaying();
    }
}
