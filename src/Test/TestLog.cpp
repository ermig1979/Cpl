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

#include <atomic>

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

    //-------------------------------------------------------------------------------------------------

    bool LogFileWriterDanglingUserDataTest(const Options& options)
    {
        return RunIsolated([&options]() -> bool
        {
            Cpl::Log log;
            int id1 = log.AddFileWriter(Cpl::Log::Debug, options.OutputPath("log_dangle_1.txt"));
            log.Write(Cpl::Log::Debug, "before second writer");

            int id2 = log.AddFileWriter(Cpl::Log::Debug, options.OutputPath("log_dangle_2.txt"));
            if (id1 == 0 || id2 == 0)
            {
                CPL_LOG_SS(Error, "LogFileWriterDanglingUserData: could not open output files.");
                return false;
            }
            log.Write(Cpl::Log::Debug, "after second writer");

            std::ifstream ifs1(options.OutputPath("log_dangle_1.txt"));
            std::stringstream content1;
            content1 << ifs1.rdbuf();

            if (content1.str().find("before second writer") == std::string::npos ||
                content1.str().find("after second writer") == std::string::npos)
            {
                CPL_LOG_SS(Error, "LogFileWriterDanglingUserData: first log file is missing a message, content: '" << content1.str() << "'.");
                return false;
            }
            return true;
        });
    }

    //-------------------------------------------------------------------------------------------------

    static void LogRemoveWriterRecomputesLevelWriter(const char*, void*)
    {
    }

    bool LogRemoveWriterRecomputesLevelTest(const Options& options)
    {
        Cpl::Log log;
        int errorId = log.AddWriter(Cpl::Log::Error, LogRemoveWriterRecomputesLevelWriter, NULL);
        int debugId = log.AddWriter(Cpl::Log::Debug, LogRemoveWriterRecomputesLevelWriter, NULL);
        if (log.MaxLevel() != Cpl::Log::Debug)
        {
            CPL_LOG_SS(Error, "LogRemoveWriterRecomputesLevel: MaxLevel() expected Debug after adding writers, got " << log.MaxLevel());
            return false;
        }
        log.RemoveWriter(debugId);
        if (log.MaxLevel() != Cpl::Log::Error || log.Enable(Cpl::Log::Debug))
        {
            CPL_LOG_SS(Error, "LogRemoveWriterRecomputesLevel: MaxLevel() expected Error after removing the Debug writer, got " << log.MaxLevel());
            return false;
        }
        log.RemoveWriter(errorId);
        if (log.MaxLevel() != Cpl::Log::None || log.Enable(Cpl::Log::Error))
        {
            CPL_LOG_SS(Error, "LogRemoveWriterRecomputesLevel: MaxLevel() expected None after removing every writer, got " << log.MaxLevel());
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------------------------------

    static void LogPrefixSeparatorWriter(const char* msg, void* userData)
    {
        String& line = *(String*)userData;
        line = msg;
    }

    bool LogPrefixSeparatorTest(const Options& options)
    {
        struct Case
        {
            Cpl::Log::Flags flags;
            const char* expected;
        };
        // The separator ": " stands between the prefix and the message, and only when a prefix was written.
        const std::vector<Case> cases =
        {
            { Cpl::Log::WritePrefix, "Debug: message\n" },
            { Cpl::Log::Flags(Cpl::Log::WriteThreadId | Cpl::Log::PrettyThreadId), "[000]: message\n" },
            { Cpl::Log::Flags(Cpl::Log::WriteThreadId | Cpl::Log::PrettyThreadId | Cpl::Log::WritePrefix), "[000] Debug: message\n" },
            { Cpl::Log::Flags(0), "message\n" }
        };
        for (size_t i = 0; i < cases.size(); ++i)
        {
            String line;
            Cpl::Log log;
            log.SetFlags(cases[i].flags);
            log.AddWriter(Cpl::Log::Debug, LogPrefixSeparatorWriter, &line);
            log.Write(Cpl::Log::Debug, "message");
            if (line != cases[i].expected)
            {
                CPL_LOG_SS(Error, "LogPrefixSeparator: flags " << (int)cases[i].flags << " expected '" << cases[i].expected << "', got '" << line << "'.");
                return false;
            }
        }
        return true;
    }

    //-------------------------------------------------------------------------------------------------

    struct LogConcurrentFlagsContext
    {
        std::atomic<size_t> received;
        std::atomic<size_t> malformed;

        LogConcurrentFlagsContext()
            : received(0)
            , malformed(0)
        {
        }
    };

    // The switching thread alternates between thread-id-only and prefix-only flags, so every line must be
    // exactly "[NNN]: log" or "Debug: log". Any other shape means Write() formatted with a mix of both.
    static void LogConcurrentFlagsWriter(const char* msg, void* userData)
    {
        LogConcurrentFlagsContext& context = *(LogConcurrentFlagsContext*)userData;
        const Cpl::String line(msg);
        const bool threadShape = line.size() == 11 && line[0] == '[' && line.compare(4, 7, "]: log\n") == 0;
        const bool prefixShape = line == "Debug: log\n";
        context.received++;
        if (!threadShape && !prefixShape)
            context.malformed++;
    }

    static void LogConcurrentFlagsRawNoop(Cpl::Log::Level, const char*, void*)
    {
    }

    bool LogConcurrentFlagsTest(const Options& options)
    {
        return RunIsolated([]() -> bool
        {
            // Static: MSVC requires a capture for a local constant used inside a lambda, gcc and clang do not.
            static const size_t messages = 20000, switches = 20000;
            const Cpl::Log::Flags threadFlags = Cpl::Log::Flags(Cpl::Log::WriteThreadId | Cpl::Log::PrettyThreadId);
            const Cpl::Log::Flags prefixFlags = Cpl::Log::WritePrefix;
            Cpl::Log log;
            LogConcurrentFlagsContext context;
            log.SetFlags(threadFlags);
            log.AddWriter(Cpl::Log::Debug, LogConcurrentFlagsWriter, &context);
            std::atomic<bool> started(false);

            std::thread writer([&log, &started]()
            {
                while (!started)
                    std::this_thread::yield();
                for (size_t message = 0; message < messages; ++message)
                    log.Write(Cpl::Log::Debug, "log");
            });
            std::thread switcher([&log, &started, &threadFlags, &prefixFlags]()
            {
                started = true;
                for (size_t i = 0; i < switches; ++i)
                {
                    log.SetFlags(i % 2 ? prefixFlags : threadFlags);
                    int id = log.AddWriter(Cpl::Log::Debug, LogConcurrentFlagsRawNoop, NULL);
                    log.RemoveWriter(id);
                }
            });
            writer.join();
            switcher.join();

            if (context.malformed != 0 || context.received != messages)
            {
                CPL_LOG_SS(Error, "LogConcurrentFlags: " << context.malformed << " malformed lines, " << context.received << " of " << messages << " received.");
                return false;
            }
            return true;
        }, 60000);
    }

    //-------------------------------------------------------------------------------------------------

    struct LogReentrantCallbackContext
    {
        Cpl::Log log;
        int selfRemovingId;
        size_t pongs;
        size_t selfRemovingReceived;

        LogReentrantCallbackContext()
            : selfRemovingId(0)
            , pongs(0)
            , selfRemovingReceived(0)
        {
        }
    };

    // Answers "ping" with a nested Write on the same logger.
    static void LogReentrantEchoWriter(Cpl::Log::Level, const char* msg, void* userData)
    {
        LogReentrantCallbackContext& context = *(LogReentrantCallbackContext*)userData;
        if (Cpl::String(msg) == "ping")
            context.log.Write(Cpl::Log::Debug, "pong");
        else
            context.pongs++;
    }

    // Removes itself on the first message it receives; it must get nothing after that, even within the same dispatch.
    static void LogReentrantSelfRemovingWriter(Cpl::Log::Level, const char*, void* userData)
    {
        LogReentrantCallbackContext& context = *(LogReentrantCallbackContext*)userData;
        context.selfRemovingReceived++;
        context.log.RemoveWriter(context.selfRemovingId);
    }

    bool LogReentrantCallbackTest(const Options& options)
    {
        return RunIsolated([]() -> bool
        {
            LogReentrantCallbackContext context;
            context.log.AddWriter(Cpl::Log::Debug, LogReentrantEchoWriter, &context);
            context.selfRemovingId = context.log.AddWriter(Cpl::Log::Debug, LogReentrantSelfRemovingWriter, &context);

            context.log.Write(Cpl::Log::Debug, "ping");
            context.log.Write(Cpl::Log::Debug, "ping");

            if (context.pongs != 2 || context.selfRemovingReceived != 1)
            {
                CPL_LOG_SS(Error, "LogReentrantCallback: expected 2 pongs and 1 message to the self-removing writer, got "
                    << context.pongs << " and " << context.selfRemovingReceived);
                return false;
            }
            return true;
        }, 5000);
    }
}
