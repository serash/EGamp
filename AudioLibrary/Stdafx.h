// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Definitions
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000
#define CUSTOM_E_FORMAT_NOT_SET 998

// including headers
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <assert.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "StdAfx.h"

// template functions
template <class T> void safeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}
// interpolation functions
template <class T> T LinearInterpolation(T sample1, T sample2, float samplePos)
{
	if(sample1 == sample2)
		return sample1;
	else if(sample2 > sample1)
		return sample1 + (sample2 - sample1)*samplePos;
	else
		return sample2 + (sample1 - sample2)*samplePos;
}

	