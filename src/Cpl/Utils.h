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

#include "Cpl/Time.h"

namespace Cpl
{
    /*! @ingroup cpl_utils
    * \brief Rounds a size up to a multiple of the given alignment.
    * \param [in] size - Value to align.
    * \param [in] align - Alignment in bytes. Must be a power of two.
    * \return Smallest multiple of align that is greater than or equal to size.
    */
    CPL_INLINE size_t AlignHi(size_t size, size_t align)
    {
        return (size + align - 1) & ~(align - 1);
    }

    /*! @ingroup cpl_utils
    * \brief Rounds a size down to a multiple of the given alignment.
    * \param [in] size - Value to align.
    * \param [in] align - Alignment in bytes. Must be a power of two.
    * \return Largest multiple of align that is less than or equal to size.
    */
    CPL_INLINE size_t AlignLo(size_t size, size_t align)
    {
        return size & ~(align - 1);
    }

    /*! @ingroup cpl_utils
    * \brief Rounds a floating-point value to the nearest integer.
    * \param [in] value - Value to round.
    * \return Nearest int. Ties (fractional part 0.5) are rounded away from zero.
    */
    CPL_INLINE int Round(double value)
    {
        return (int)(value + (value >= 0 ? 0.5 : -0.5));
    }

    /*! @ingroup cpl_utils
    * \brief Returns a pseudo-random floating-point value in the closed range [min, max].
    * \param [in] min - Lower bound of the range.
    * \param [in] max - Upper bound of the range.
    * \return A value in [min, max] computed as min + rand() / RAND_MAX * (max - min).
    * \note Uses the C library rand() generator. Call srand() to seed it. Not suitable for
    *       cryptographic use.
    */
    CPL_INLINE double Random(double min, double max)
    {
        return min + double(rand()) / double(RAND_MAX) * (max - min);
    }

    /*! @ingroup cpl_utils
    * \brief Busy-waits for approximately the given duration.
    * \param [in] seconds - Duration to wait.
    * \note Occupies the calling thread by spinning until TimeCounter() advances by
    *       seconds * TimeFrequency() ticks. Intended for tests and dummy load, not for
    *       precise timing or yielding the CPU.
    */
    CPL_INLINE void StubWork(double seconds)
    {
        uint64_t current = TimeCounter(), finish = current + uint64_t(seconds * double(TimeFrequency())), sum = 1;
        while (current < finish && sum > 0)
        {
            for (int i = 0; i < 1000; ++i)
                sum += i;
            current = TimeCounter();
        }
    }
}
