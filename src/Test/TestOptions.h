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
#pragma once

#include "Test/Test.h"

namespace Test
{
    struct Group;

    struct Options : public Cpl::ArgsParser
    {
        bool help;
        Log::Level logLevel;
        String logFile;
        String output;
        Strings include, exclude;

        Options(int argc, char* argv[])
            : Cpl::ArgsParser(argc, argv, true)
        {
            help = HasArg("-h", "-?");
            logLevel = (Log::Level)Cpl::ToVal<Int>(GetArg2("-ll", "--logLevel", "4", false));
            logFile = GetArg2("-lf", "--logFile", "", false);
            output = GetArg("-o", "out", false);
            include = GetArgs("-i", Strings(), false);
            exclude = GetArgs("-e", Strings(), false);
        }

        bool Required(const Group& group);

        String OutputPath(const String& name) const
        {
            return Cpl::MakePath(output, name);
        }
    };
}
