/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2021 Yermalayeu Ihar.
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

#include "Test/Test.h"

#include "Cpl/Performance.h"

namespace Test
{
    static void TestFuncV0()
    {
        CPL_PERF_FUNC();
        std::this_thread::sleep_for(std::chrono::milliseconds(45));
    }

    static void TestFuncV1()
    {
        CPL_PERF_FUNC();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    static void TestFuncV2()
    {
        CPL_PERF_FUNC();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    bool PerformanceSimpleTest()
    {
        for (size_t i = 0; i < 5; ++i)
            TestFuncV0();

        for (size_t i = 0; i < 10; ++i)
            TestFuncV1();

        for (size_t i = 0; i < 15; ++i)
            TestFuncV2();

        typedef Cpl::PerformanceStorage::FunctionMap FunctionMap;
        FunctionMap merged = Cpl::PerformanceStorage::s_storage.Merged();
        for (FunctionMap::const_iterator function = merged.begin(); function != merged.end(); ++function)
        {
            const Cpl::PerformanceMeasurer& pm = *function->second;
            std::cout << function->first << ": ";
            std::cout << std::setprecision(0) << std::fixed << pm.Total() << " ms";
            std::cout << " / " << pm.Count() << " = ";
            std::cout << std::setprecision(3) << std::fixed << pm.Average() << " ms";
            std::cout << std::setprecision(3) << " {min=" << pm.Min() << "; max=" << pm.Max() << "}" << std::endl;
        }

        return true;
    }
}
