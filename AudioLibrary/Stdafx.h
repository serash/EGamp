// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

// Definitions
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

// including headers
#include <iostream>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
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

	