//------------------------------------------------------------------------------
// <copyright file="stdafx.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// include file for standard system and project includes

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files
#include <windows.h>

#include <Shlobj.h>

// Direct2D Header Files
#include <d2d1.h>

// Kinect Header files
#include <Kinect.h>
#include <Kinect.Face.h>
#include <stdio.h>
#include <crtdbg.h>

#pragma comment (lib, "d2d1.lib")

using namespace std;

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

// Safe release for interfaces
template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if (pInterfaceToRelease != nullptr)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

#define ASSERT(expression) _ASSERTE(expression)

#ifdef _DEBUG
#define VERIFY(expression)	ASSERT(expression)
#define HR(expression)	ASSERT(S_OK == (expression	))


inline void TRACE(WCHAR const * const format, ...)
{
	va_list args;
	va_start(args, format);

	WCHAR output[512];
	vswprintf_s(output, format, args);

	OutputDebugString(output);

	va_end(args);
}

#else
#define VERIFY(expression) (expression)

struct ComException
{
	HRESULT const hr;
	ComException(HRESULT const value) :hr(value) {}

};

inline void HR(HRESULT const hr)
{
	if (S_OK != hr) throw ComException(hr);

}

#define TRACE __noop
#endif

