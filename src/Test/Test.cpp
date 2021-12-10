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

#define CPL_IMPLEMENT

#include "Test/Test.h"

namespace Test
{
    typedef bool(*TestPtr)();

    struct Test
    {
        String name;
        TestPtr test;

        Test(const String& n, const TestPtr& t)
            : name(n)
            , test(t)
        {
        }
    };
    typedef std::vector<Test> Tests;
    Tests g_tests;

#define TEST_ADD(name) \
    bool name##Test(); \
    bool name##AddToList(){ g_tests.push_back(Test::Test(#name, name##Test)); return true; } \
    bool name##AtList = name##AddToList();

    TEST_ADD(ParamSimple);
    TEST_ADD(ParamStruct);
    TEST_ADD(ParamVector);
    TEST_ADD(ParamEnum);
    TEST_ADD(ParamMap);

    struct Options : public Cpl::ArgsParser
    {
        bool help;
        Log::Level logLevel;
        Strings include, exclude;

        Options(int argc, char* argv[])
            : Cpl::ArgsParser(argc, argv, true)
        {
            help = HasArg("-h", "-?");
            logLevel = (Log::Level)Cpl::ToVal<Int>(GetArg2("-ll", "--logLevel", "3", false));
            include = GetArgs("-i", Strings(), false);
            exclude = GetArgs("-e", Strings(), false);
        }

        bool Required(const Test& test)
        {
            bool required = include.empty();
            for (size_t i = 0; i < include.size() && !required; ++i)
                if (test.name.find(include[i]) != std::string::npos)
                    required = true;
            for (size_t i = 0; i < exclude.size() && required; ++i)
                if (test.name.find(exclude[i]) != std::string::npos)
                    required = false;
            return required;
        }
    };

    int PrintHelp()
    {
        std::cout << "Test framework of Common Purpose Library." << std::endl << std::endl;
        std::cout << "Test application parameters:" << std::endl << std::endl;
        std::cout << " -i=test  - include test filter." << std::endl << std::endl;
        std::cout << " -e=test  - exclude test filter." << std::endl << std::endl;
        std::cout << " -ll=1    - a log level." << std::endl << std::endl;
        std::cout << " -h or -? - to print this help message." << std::endl << std::endl;
        return 0;
    }

    int MakeTests(const Tests& tests, const Options& options)
    {
        for (size_t t = 0; t < tests.size(); ++t)
        {
            const Test& test = tests[t];
            CPL_LOG_SS(Info, test.name << "Test is started :");
            bool result = test.test();
            if (result)
            {
                CPL_LOG_SS(Info, test.name << "Test is OK." << std::endl);
            }
            else
            {
                CPL_LOG_SS(Error, test.name << "Test has errors. TEST EXECUTION IS TERMINATED!" << std::endl);
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

    Cpl::Log::s_log.SetStd();
    Cpl::Log::s_log.SetLevel(options.logLevel);

    Test::Tests tests;
    for (const Test::Test& test : Test::g_tests)
        if (options.Required(test))
            tests.push_back(test);

    if (tests.empty())
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

    return Test::MakeTests(tests, options);
}
