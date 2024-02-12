/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar.
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
        CPL_PERF_FUNCF(1000 * 1000 * 1000);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    static void TestFuncV3()
    {
        CPL_PERF_INIT(pm, "1 & 3");

        CPL_PERF_START(pm);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        CPL_PERF_PAUSE(pm);

        std::this_thread::sleep_for(std::chrono::milliseconds(15));

        CPL_PERF_START(pm);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        CPL_PERF_PAUSE(pm);
    }

    static void TestFuncV4()
    {
        CPL_PERF_FUNCFH(0, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    bool PerformanceSimpleTest()
    {
#if defined(CPL_PERF_ENABLE)
        Cpl::PerformanceStorage::Global().Clear();
#endif
        for (size_t i = 0; i < 5; ++i)
            TestFuncV0();

        for (size_t i = 0; i < 10; ++i)
            TestFuncV1();

        for (size_t i = 0; i < 15; ++i)
            TestFuncV2();

        for (size_t i = 0; i < 5; ++i)
            TestFuncV3();

        for (size_t i = 0; i < 50; ++i)
            TestFuncV4();

#if defined(CPL_PERF_ENABLE)
        CPL_LOG_SS(Verbose, std::endl << Cpl::PerformanceStorage::Global().Report());
#endif
        return true;
    }

    static void TestFuncV5()
    {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        CPL_PERF_BEGFH(ss.str(), 0, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    bool PerformanceStdThreadTest()
    {
#if defined(CPL_PERF_ENABLE)
        Cpl::PerformanceStorage::Global().Clear();
#endif
        const size_t n = 10, t = 10;
        typedef std::thread Thread;
        typedef std::vector<Thread> Threads;
        Threads threads;

        for (size_t j = 0; j < t; ++j)
        {
            for (size_t i = 0; i < n; ++i)
                threads.push_back(Thread(&TestFuncV5));

            for (size_t i = 0; i < threads.size(); ++i)
                if (threads[i].joinable())
                    threads[i].join();
        }
#if defined(CPL_PERF_ENABLE)
        CPL_LOG_SS(Verbose, std::endl << Cpl::PerformanceStorage::Global().Report());
#endif
        return true;
    }

#if defined(CPL_TEST_NORETURN)
    static void* TestFuncV6(void*)
    {
    }

    bool PerformanceNoReturnTest()
    {
        TestFuncV6(NULL);

        return true;
    }
#endif
}

#if defined(__linux__)
#include <pthread.h>

namespace Test
{
    static void* TestFuncV7(void*)
    {
        CPL_PERF_FUNC();
        return 0;
    }  

    bool PerformancePthreadTest()
    {
#if defined(CPL_PERF_ENABLE)
        Cpl::PerformanceStorage::Global().Clear();
#endif
        for (int i = 0; i < 1; ++i)
        {
            pthread_t thread_run;
            pthread_create(&thread_run, NULL, TestFuncV7, NULL);
            pthread_join(thread_run, NULL);
        }
#if defined(CPL_PERF_ENABLE)
        CPL_LOG_SS(Verbose, std::endl << Cpl::PerformanceStorage::Global().Report());
#endif
        return true;
    }
}
#endif
