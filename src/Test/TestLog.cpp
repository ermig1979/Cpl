/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2026 Yermalayeu Ihar,
*               2021-2022 Andrey Drogolyub.
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

#include "Cpl/Log.h"

namespace Test
{
    static void CustomFileWriter(const char* msg, void* userData)
    {
        std::ofstream& ofs = *(std::ofstream*)userData;
        if (ofs.is_open())
            ofs << " {custom logger} " << msg << std::flush;
    }

    bool LogCallbackTest(const Options& options)
    {
        std::ofstream ofs(options.OutputPath("custom_log.txt"));
        int id = Cpl::Log::Global().AddWriter(Log::Debug, CustomFileWriter, &ofs);

        CPL_LOG(Debug, "debug log message");

        Cpl::Log::Global().RemoveWriter(id);

        return true;
    }

    //-------------------------------------------------------------------------------------------------

    static void CustomRawFileWriter(Cpl::Log::Level level, const char* msg, void* userData)
    {
        std::ofstream& ofs = *(std::ofstream*)userData;
        if (ofs.is_open())
            ofs << " {raw custom logger} " << msg << std::flush;
    }

    bool LogCallbackRawTest(const Options& options)
    {
        std::ofstream ofs(options.OutputPath("custom_raw_log.txt"));
        int id = Cpl::Log::Global().AddWriter(Log::Debug, CustomRawFileWriter, &ofs);

        CPL_LOG(Debug, "raw debug log message");

        Cpl::Log::Global().RemoveWriter(id);

        return true;
    }

    //-------------------------------------------------------------------------------------------------

    bool LogDateTimeTest(const Options& options)
    {
        Cpl::Log::Flags flags = Cpl::Log::Global().GetFlags();

        Cpl::Log::Global().SetFlags(Cpl::Log::Flags(flags | Cpl::Log::WriteDate));

        CPL_LOG(Info, "Write date in message");

        Cpl::Log::Global().SetFlags(Cpl::Log::Flags(flags | Cpl::Log::WriteTime));

        CPL_LOG(Info, "Write time in message");

        Cpl::Log::Global().SetFlags(Cpl::Log::Flags(flags | Cpl::Log::WriteDate | Cpl::Log::WriteTime));

        CPL_LOG(Info, "Write date and time in message");

        Cpl::Log::Global().SetFlags(flags);

        CPL_LOG(Info, "Write no date or time in message");

        return true;
    }

    //-------------------------------------------------------------------------------------------------

    bool LogIdTest(const Options& options)
    {
        std::ofstream ofs1(options.OutputPath("log_1.txt"));
        int id1 = Cpl::Log::Global().AddWriter(Log::Debug, CustomFileWriter, &ofs1);

        std::ofstream ofs2(options.OutputPath("log_2.txt"));
        int id2 = Cpl::Log::Global().AddWriter(Log::Debug, CustomFileWriter, &ofs2);

        CPL_LOG_ID(Debug, "log 1 message", id1);

        CPL_LOG_ID(Debug, "log 2 message", id2);

        CPL_LOG(Debug, "common message");

        Cpl::Log::Global().RemoveWriter(id1);

        CPL_LOG(Debug, "after remove log 1 message");

        Cpl::Log::Global().RemoveWriter(id2);

        return true;
    }
}
