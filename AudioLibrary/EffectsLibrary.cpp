#include "stdafx.h"
#include "EffectsLibrary.h"

/* ==========================
   ===  EFFECTSCOLLECTION ===
   ===   IMPLEMENTATION   ===
   ========================== */

EffectsLibrary::EffectsCollection::EffectsCollection() 
{
}

bool EffectsLibrary::EffectsCollection::moveUp(int idx)
{
	if(idx + 1 < effects.size()) 
        return swap(idx, idx + 1);
	else
		return false;
}

bool EffectsLibrary::EffectsCollection::moveDown(int idx)
{
    if (idx - 1 >= 0)
		return swap(idx - 1, idx);
	else 
		return false;
}

bool EffectsLibrary::EffectsCollection::swap(int idx1, int idx2)
{
    IEffect *temp = effects[idx1];
    effects[idx1] = effects[idx2];
    effects[idx2] = temp;
	return true;
}


/* ==========================
   ===      AMPLIFIER     ===
   ===   IMPLEMENTATION   ===
   ========================== */

EffectsLibrary::Amplifier::Amplifier()
{
	name = "Amplifier";
	amplify = new EffectParameter(1, 20, 1, "Amplifier");
	hookEvent(&amplify->valueChanged);
}

void EffectsLibrary::Amplifier::initialize()
{
	amp = 1;
}

void EffectsLibrary::Amplifier::block()
{
}

void EffectsLibrary::Amplifier::update()
{
    amp = sqrt(pow(10, amplify->getValue()/10));
}

std::string EffectsLibrary::Amplifier::getName()
{
	return name;
}

void EffectsLibrary::Amplifier::Sample(double &spl0, double &spl1)
{
	spl0 = spl0 * amp;
    spl1 = spl1 * amp;
}


/* ==========================
   ===       TREMOLO      ===
   ===   IMPLEMENTATION   ===
   ========================== */

EffectsLibrary::Tremolo::Tremolo(DWORD sampleRate_)
{
	sampleRate = sampleRate_;
	name = "Tremolo";
	frequency = new EffectParameter(0, 100, 4, "frequency (Hz)");
	hookEvent(&frequency->valueChanged);
	amount = new EffectParameter(-60, 0, -6, "amount (dB)");
	hookEvent(&amount->valueChanged);
	stereoSeperation = new EffectParameter(0, 1, 0, "stereo separation (0..1)");
	hookEvent(&stereoSeperation->valueChanged);
}

void EffectsLibrary::Tremolo::initialize()
{
    adv = PI * 2 * 4 / sampleRate;
    sep = -6 * PI;
    am = pow(2, 0 / 6);
    sc = 0.5 * am; 
	am = 1 - am;
}

void EffectsLibrary::Tremolo::block()
{
}

void EffectsLibrary::Tremolo::update()
{
	adv = PI * 2 * frequency->getValue() / sampleRate;
    sep = amount->getValue() * PI;
    am = pow(2, stereoSeperation->getValue() / 6);
    sc = 0.5 * am;
	am = 1 - am;
}

std::string EffectsLibrary::Tremolo::getName()
{
	return name;
}

void EffectsLibrary::Tremolo::Sample(double &spl0, double &spl1)
{
    spl0 = spl0 * ((cos(pos) + 1) * sc + am);
    spl1 = spl1 * ((cos(pos + sep) + 1) * sc + am);
    pos += adv;
}


/* ==========================
   ===  EFFECTSPARAMETER  ===
   ===   IMPLEMENTATION   ===
   ========================== */

EffectsLibrary::EffectParameter::EffectParameter(double min_, double max_, double defaultVal_, std::string name_)
{
	min = min_;
	max = max_;
	defaultVal = defaultVal_;
	value = defaultVal;
	name = name_;
}

double EffectsLibrary::EffectParameter::getValue()
{
	return value;
}

void EffectsLibrary::EffectParameter::setValue(double value_)
{
	if(value_ >= min && value_ <= max)
		value = value_;
	__raise valueChanged.ThrowUpdateEvent();
}

std::string EffectsLibrary::EffectParameter::getName()
{
	return name;
}

double EffectsLibrary::EffectParameter::getMin()
{
	return min;
}

double EffectsLibrary::EffectParameter::getMax()
{
	return max;
}

double EffectsLibrary::EffectParameter::getDefaultVal()
{
	return defaultVal;
}
