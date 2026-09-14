/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2026 Yermalayeu Ihar,
*               2021-2022 Andrey Drogolyub,
*               2023-2023 Daniil Germanenko.
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

#define CPL_IMPLEMENT
#include "Cpl/Log.h"
#include "Cpl/Performance.h"

#include "Test/Test.h"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>
#include <thread>
#endif

namespace Test
{
#if defined(__unix__) || defined(__APPLE__)
    bool RunIsolated(const std::function<bool()>& body, size_t timeoutMs)
    {
        std::cout << std::flush;
        std::cerr << std::flush;
        const pid_t pid = fork();
        if (pid < 0)
        {
            CPL_LOG_SS(Error, "RunIsolated: fork() failed.");
            return false;
        }
        if (pid == 0)
        {
            // ASan and TSan cannot map their shadow memory under RLIMIT_DATA, so sanitizer builds rely on the timeout alone.
#if !defined(__SANITIZE_ADDRESS__) && !defined(__SANITIZE_THREAD__)
            rlimit dataLimit;
            dataLimit.rlim_cur = size_t(1) << 30;
            dataLimit.rlim_max = size_t(1) << 30;
            setrlimit(RLIMIT_DATA, &dataLimit);
#endif
            rlimit coreLimit;
            coreLimit.rlim_cur = 0;
            coreLimit.rlim_max = 0;
            setrlimit(RLIMIT_CORE, &coreLimit);

            bool result = false;
            try
            {
                result = body();
            }
            catch (const std::exception& e)
            {
                std::cerr << "RunIsolated: uncaught exception: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "RunIsolated: uncaught non-standard exception." << std::endl;
            }
            std::cout << std::flush;
            std::cerr << std::flush;
            _exit(result ? 0 : 1);
        }
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        int status = 0;
        for (;;)
        {
            const pid_t waited = waitpid(pid, &status, WNOHANG);
            if (waited == pid)
                break;
            if (waited < 0 && errno != EINTR)
            {
                const int error = errno;
                // ECHILD means the child has already been reaped by someone else, so its identifier may
                // belong to an unrelated process by now: killing or waiting for it would hit a stranger.
                if (error == ECHILD)
                {
                    CPL_LOG_SS(Error, "RunIsolated: the child process has already been reaped, its result is unknown.");
                    return false;
                }
                CPL_LOG_SS(Error, "RunIsolated: waitpid() failed with errno " << error << ".");
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                return false;
            }
            if (std::chrono::steady_clock::now() >= deadline)
            {
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                CPL_LOG_SS(Error, "RunIsolated: body did not finish in " << timeoutMs << " ms, killed.");
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        if (WIFSIGNALED(status))
        {
            CPL_LOG_SS(Error, "RunIsolated: body terminated by signal " << WTERMSIG(status) << ".");
            return false;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            CPL_LOG_SS(Error, "RunIsolated: body failed with exit code " << (WIFEXITED(status) ? WEXITSTATUS(status) : -1) << ".");
            return false;
        }
        return true;
    }
#else
    bool RunIsolated(const std::function<bool()>& body, size_t)
    {
        try
        {
            return body();
        }
        catch (const std::exception& e)
        {
            CPL_LOG_SS(Error, "RunIsolated: uncaught exception: " << e.what() << ".");
        }
        catch (...)
        {
            CPL_LOG_SS(Error, "RunIsolated: uncaught non-standard exception.");
        }
        return false;
    }
#endif

    typedef bool(*TestPtr)(const Options& options);

    struct Group
    {
        String name;
        TestPtr test;

        Group(const String& n, const TestPtr& t)
            : name(n)
            , test(t)
        {
        }
    };
    typedef std::vector<Group> Groups;
    Groups g_groups;

#define TEST_ADD(name) \
    bool name##Test(const Options& options); \
    bool name##AddToList(){ g_groups.push_back(Group(#name, name##Test)); return true; } \
    bool name##AtList = name##AddToList();

    TEST_ADD(HasArg);
    TEST_ADD(HasArg1);
    TEST_ADD(HasArg2);

    TEST_ADD(LogCallback);
    TEST_ADD(LogCallbackRaw);
    TEST_ADD(LogDateTime);
    TEST_ADD(LogId);
    TEST_ADD(LogFileWriterDanglingUserData);
    TEST_ADD(LogRemoveWriterRecomputesLevel);
    TEST_ADD(LogPrefixSeparator);
    TEST_ADD(LogConcurrentFlags);
    // Calling the logger from a writer callback is prohibited, see the note on the Log class, so the test
    // that checks it hangs by design. It is kept for the day the logger starts to support such callbacks.
    //TEST_ADD(LogReentrantCallback);

    TEST_ADD(ParseUri);

    TEST_ADD(StartsWith);
    TEST_ADD(EndsWith);

    TEST_ADD(CurrentDateTimeString);
    TEST_ADD(SeparateString);
    TEST_ADD(SeparateStringMulti);
    TEST_ADD(TimeToStr);
    TEST_ADD(ToStr);

    TEST_ADD(PolygonHasPoint);
    TEST_ADD(PolygonOverlapsRectangle);
    TEST_ADD(PolygonOverlapsRectangleFloat);

    TEST_ADD(ParamSimple);
    TEST_ADD(ParamSizetDefault);
    TEST_ADD(ParamStruct);
    TEST_ADD(ParamStructMod);
    TEST_ADD(ParamVector);
    TEST_ADD(ParamEnum);
    TEST_ADD(ParamMap);
    TEST_ADD(ParamMapBug);
    TEST_ADD(ParamLimited);
    TEST_ADD(ParamTemplate);

    TEST_ADD(ParamVectorV2);
    TEST_ADD(ParamMapV2);

    TEST_ADD(Prop);

    TEST_ADD(PerformanceSimple);
    TEST_ADD(PerformanceStdThread);
    TEST_ADD(PerformanceClear);
#if defined(CPL_TEST_NORETURN)
    TEST_ADD(PerformanceNoReturn);
#endif
#if defined(__linux__)
    TEST_ADD(PerformancePthread);
#endif

    TEST_ADD(TableSimple);
    TEST_ADD(TableSortable);

    TEST_ADD(YamlSimple);
    TEST_ADD(YamlParam);

    TEST_ADD(XmlAllocateString);
    TEST_ADD(XmlIterator);
    TEST_ADD(ToValEmpty);
    TEST_ADD(DoFileModify);
    TEST_ADD(DoFileExistance);
    TEST_ADD(DoFileInfo);

    bool Options::Required(const Group& group)
    {
        bool required = include.empty();
        for (size_t i = 0; i < include.size() && !required; ++i)
            if (group.name.find(include[i]) != std::string::npos)
                required = true;
        for (size_t i = 0; i < exclude.size() && required; ++i)
            if (group.name.find(exclude[i]) != std::string::npos)
                required = false;
        return required;
    }

    int PrintHelp()
    {
        std::cout << "Test framework of Common Purpose Library." << std::endl << std::endl;
        std::cout << "Test application parameters:" << std::endl << std::endl;
        std::cout << " -i=test      - include test filter." << std::endl << std::endl;
        std::cout << " -e=test      - exclude test filter." << std::endl << std::endl;
        std::cout << " -ll=1        - a log level." << std::endl << std::endl;
        std::cout << " -lf=test.log - a log file name." << std::endl << std::endl;
        std::cout << " -o=out       - an output folder for test artifacts." << std::endl << std::endl;
        std::cout << " -h or -?     - to print this help message." << std::endl << std::endl;
        return 0;
    }

    int MakeTests(const Groups& groups, const Options& options)
    {
        if (!Cpl::CreatePath(options.output))
        {
            CPL_LOG_SS(Error, "Can't create output folder '" << options.output << "' !");
            return 1;
        }

        for (size_t t = 0; t < groups.size(); ++t)
        {
            const Group& group = groups[t];
            CPL_LOG_SS(Info, group.name << "Test is started :");
            bool result = group.test(options);
            if (result)
            {
                CPL_LOG_SS(Info, group.name << "Test is OK." << std::endl);
            }
            else
            {
                CPL_LOG_SS(Error, group.name << "Test has errors. TEST EXECUTION IS TERMINATED!" << std::endl);
                return 1;
            }
        }
        CPL_LOG_SS(Info, "ALL TESTS ARE FINISHED SUCCESSFULLY!" << std::endl);
        return 0;
    }
}

int main(int argc, char* argv[])
{
    Test::Options options(argc, argv);

    if (options.help)
        return Test::PrintHelp();

    Cpl::Log::Global().AddStdWriter(options.logLevel);
    if(!options.logFile.empty())
        Cpl::Log::Global().AddFileWriter(options.logLevel, options.logFile);
    Cpl::Log::Global().SetFlags(Cpl::Log::BashFlags);

    Test::Groups groups;
    for (const Test::Group& group : Test::g_groups)
        if (options.Required(group))
            groups.push_back(group);

    if (groups.empty())
    {
        std::stringstream ss;
        ss << "There are not any suitable tests for current filters! " << std::endl;
        ss << "  Include filters: " << std::endl;
        for (size_t i = 0; i < options.include.size(); ++i)
            ss << "'" << options.include[i] << "' ";
        ss << std::endl;
        ss << "  Exclude filters: " << std::endl;
        for (size_t i = 0; i < options.exclude.size(); ++i)
            ss << "'" << options.exclude[i] << "' ";
        ss << std::endl;
        CPL_LOG_SS(Error, ss.str());
        return 1;
    }

    return Test::MakeTests(groups, options);
}
