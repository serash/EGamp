// EffectsLibrary.h

#pragma once
#include <winnt.h>
#include <winerror.h>
#include <Windows.h>
#include <vector>
#using <mscorlib.dll>

using System::String;
using System::Convert;
using System::TimeSpan;
using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Threading;
using namespace System::Text;

namespace EffectsLibrary {
	//const
    const double PI = 3.14159265;
    const double E =  2.71828182;

	// Helper Classes/Interfaces
	private class UpdateEventSource {
	public:
		__event void ThrowUpdateEvent();
	};

	public class EffectParameter {
	public:
		EffectParameter(double min_, double max_, double defaultVal_, std::string name_);
		double getValue();
		void setValue(double value_);
		std::string getName();
		double getMin();
		double getMax();
		double getDefaultVal();
		UpdateEventSource valueChanged;
	private:
		double min;
		double max;
		double defaultVal;
		double value;
		std::string name;
	};
	public class IEffect {
	public:
		virtual void initialize() {}
		virtual void block() {}
		virtual void update() {}
		virtual void Sample(double &spl0, double &spl1) {}
		virtual std::string getName() { return "";}

		void hookEvent(UpdateEventSource* pSource) {
			__hook(&UpdateEventSource::ThrowUpdateEvent, pSource, &IEffect::update);
		}

		void unhookEvent(UpdateEventSource* pSource) {
			__unhook(&UpdateEventSource::ThrowUpdateEvent, pSource, &IEffect::update);
		}
	private:
	};

	//Effects
	public class Amplifier : IEffect
	{
	public:
		Amplifier();
        virtual void initialize();
		virtual void block();
		virtual void update();
		virtual void Sample(double &spl0, double &spl1);
		virtual std::string getName();
	private:
		std::string name;
		EffectParameter *amplify;
		double amp;
	};
	public class Tremolo : IEffect
	{
	public:
		Tremolo(DWORD sampleRate_ = 44100);
        virtual void initialize();
		virtual void block();
		virtual void update();
		virtual void Sample(double &spl0, double &spl1);
		virtual std::string getName();
	private:
		std::string name;
		EffectParameter *frequency;
		EffectParameter *amount;
		EffectParameter *stereoSeperation;
		DWORD sampleRate;
		double adv;
		double sep;
		double am;
		double sc;
		double pos;
	};

	// effects collection
	public class EffectsCollection
	{
	public:
		EffectsCollection(void);
		bool moveUp(int idx);
		bool moveDown(int idx);
	private:
		bool swap(int idx1, int idx2);
		std::vector<IEffect *> effects;
	};
}

