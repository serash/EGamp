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
	private delegate void UpdateEventHandler();
	public ref class EffectParameter {
	public:
		EffectParameter(double min_, double max_, double defaultVal_, String ^name_);
		double getValue();
		void setValue(double value_);
		String ^getName();
		double getMin();
		double getMax();
		double getDefaultVal();
		event UpdateEventHandler ^valueChanged;
	private:
		double min;
		double max;
		double defaultVal;
		double value;
		String ^name;
	};
	public interface class IEffect {
        void initialize();
		void block();
		void update();
		void Sample(double &spl0, double &spl1);
		String ^getName();
	};

	//Effects
	public ref class Amplifier : IEffect
	{
	public:
		Amplifier();
        virtual void initialize();
		virtual void block();
		virtual void update();
		virtual void Sample(double &spl0, double &spl1);
		virtual String ^getName();
		EffectParameter ^getAmplify();
	private:
		String ^name;
		EffectParameter ^amplify;
		double amp;
	};
	public ref class Tremolo : IEffect
	{
	public:
		Tremolo();
		Tremolo(DWORD sampleRate_);
        virtual void initialize();
		virtual void block();
		virtual void update();
		virtual void Sample(double &spl0, double &spl1);
		virtual String ^getName();
		EffectParameter ^getFrequency();
		EffectParameter ^getAmount();
		EffectParameter ^getStereoSeperation();
	private:
		String ^name;
		EffectParameter ^frequency;
		EffectParameter ^amount;
		EffectParameter ^stereoSeperation;
		DWORD sampleRate;
		double adv;
		double sep;
		double am;
		double sc;
		double pos;
	};

	// effects collection
	public ref class EffectsCollection : List<IEffect ^>
	{
	public:
		EffectsCollection(void);
		bool moveUp(int idx);
		bool moveDown(int idx);
	private:
		bool swap(int idx1, int idx2);
	};
}

