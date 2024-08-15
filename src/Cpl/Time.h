/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2024 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include "Cpl/Defs.h"

#if defined(_MSC_VER)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__GNUC__)
#include <sys/time.h>
#else
#error Platform is not supported!
#endif

namespace Cpl
{
#if defined(_MSC_VER)
    CPL_INLINE int64_t TimeCounter()
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return counter.QuadPart;
    }

    CPL_INLINE int64_t TimeFrequency()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
    }
#elif defined(__GNUC__)
    CPL_INLINE int64_t TimeCounter()
    {
        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        return int64_t(t.tv_sec) * int64_t(1000000000) + int64_t(t.tv_nsec);
    }

    CPL_INLINE int64_t TimeFrequency()
    {
        return int64_t(1000000000);
    }
#else
#error Platform is not supported!
#endif

    CPL_INLINE double Seconds(int64_t count)
    {
        return double(count) / double(TimeFrequency());
    }

    CPL_INLINE double Miliseconds(int64_t count)
    {
        return double(count) / double(TimeFrequency()) * 1000.0;
    }

    CPL_INLINE double Time()
    {
        return double(TimeCounter()) / double(TimeFrequency());
    }
}
