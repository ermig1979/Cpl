/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2026 Yermalayeu Ihar.
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
#include "Cpl/String.h"
#include "Cpl/Console.h"

#include <atomic>
#include <mutex>
#include <map>
#include <memory>
#include <thread>

#if defined(CPL_LOG_ENABLE)
namespace Cpl
{
    /*! @ingroup cpl_log
    * \class Log
    * \brief Thread-safe logger with multiple writers, severity levels and configurable message formatting.
    * \note A writer callback is invoked while the internal lock is held, so callbacks never run concurrently.
    *       A callback must not call any method of the same Log, including Write, AddWriter and RemoveWriter:
    *       the lock is not recursive and a nested call deadlocks. Writing to another Log is allowed, but the
    *       callbacks of two instances that write into each other deadlock when used from different threads,
    *       as with any pair of locks taken in opposite orders.
    * \note The Log class is compiled only when CPL_LOG_ENABLE is defined. Otherwise the logging macros are empty.
    */
    class Log
    {
    public:
        /*!
        * \enum Level
        * \brief Severity of a log message. A greater value means a more verbose message.
        *        A writer with a given level receives messages whose level is less or equal to that value.
        */
        enum Level
        {
            None = 0, //!< Logging disabled. Messages with this level are never written.
            Error,    //!< Error messages.
            Warning,  //!< Warning messages.
            Info,     //!< Informational messages.
            Verbose,  //!< Verbose diagnostic messages.
            Debug,    //!< Debug messages (most detailed).
        };

        /*!
        * \enum Flags
        * \brief Bit flags that control the prefix written in front of a formatted log message.
        *        Flags can be combined with bitwise OR.
        */
        enum Flags
        {
            WriteThreadId = 1 << 0,  //!< Write the current thread identifier.
            WritePrefix = 1 << 1,    //!< Write the severity name (Error, Warning, Info, Verbose, Debug).
            PrettyThreadId = 1 << 2, //!< Replace the native thread id with a sequential 3-digit name.
            ColorezedPrefix = 1 << 3, //!< Colorize the severity name for terminal output.
            WriteDate = 1 << 4,      //!< Write the current date.
            WriteTime = 1 << 5,      //!< Write the current time.
            DefaultFlags = WriteThreadId | WritePrefix | PrettyThreadId, //!< Default formatting: thread id, pretty thread name and severity prefix.
            BashFlags = WriteThreadId | WritePrefix | PrettyThreadId | ColorezedPrefix, //!< Default formatting plus a colorized severity prefix.
        };

        /*!
        * \typedef Callback
        * \brief Callback that receives a fully formatted log line (prefix, message and trailing newline).
        * \param msg - Formatted log line.
        * \param userData - User pointer passed to AddWriter.
        */
        typedef void(*Callback)(const char* msg, void* userData);

        /*!
        * \typedef CallbackRaw
        * \brief Callback that receives the severity and the original message without formatting.
        * \param level - Severity of the message.
        * \param msg - Original message string without prefix or trailing newline.
        * \param userData - User pointer passed to AddWriter.
        */
        typedef void(*CallbackRaw)(Level level, const char* msg, void* userData);

        /*!
        * \typedef CallbackRawFunc
        * \brief C-style writer callback. Same as CallbackRaw, but the severity is passed as int.
        * \param level - Severity of the message as int.
        * \param msg - Original message string without prefix or trailing newline.
        * \param userData - User pointer passed to AddWriter.
        */
        typedef void(*CallbackRawFunc)(int level, const char* msg, void* userData);

        /*!
        * \fn Log()
        * \brief Constructs an empty logger with no writers, level None and DefaultFlags.
        */
        Log()
            : _writerId(0)
            , _levelMax(None)
            , _flags(DefaultFlags)
            , _rawOnly(true)
        {
        }

        /*!
        * \fn int AddWriter(Level level, Callback callback, void* userData)
        * \brief Registers a formatted writer. Enables prefix generation for subsequent Write calls.
        * \param [in] level - Maximum severity this writer will accept.
        * \param [in] callback - Callback that receives a formatted log line.
        * \param [in] userData - User pointer forwarded to the callback.
        * \return Identifier of the added writer.
        */
        int AddWriter(Level level, Callback callback, void* userData)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _writers[++_writerId] = Writer(level, callback, NULL, NULL, userData);
            _levelMax = std::max<Level>(_levelMax, level);
            _rawOnly = false;
            return _writerId;
        }

        /*!
        * \fn int AddWriter(Level level, CallbackRaw callbackRaw, void* userData)
        * \brief Registers a raw writer that receives the original message and its Level.
        * \param [in] level - Maximum severity this writer will accept.
        * \param [in] callbackRaw - Callback that receives level and the unformatted message.
        * \param [in] userData - User pointer forwarded to the callback.
        * \return Identifier of the added writer.
        */
        int AddWriter(Level level, CallbackRaw callbackRaw, void* userData)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _writers[++_writerId] = Writer(level, NULL, callbackRaw, NULL, userData);
            _levelMax = std::max<Level>(_levelMax, level);
            return _writerId;
        }

        /*!
        * \fn int AddWriter(Level level, CallbackRawFunc callbackRaw, void* userData)
        * \brief Registers a C-style raw writer that receives the original message and its level as int.
        * \param [in] level - Maximum severity this writer will accept.
        * \param [in] callbackRaw - Callback that receives level as int and the unformatted message.
        * \param [in] userData - User pointer forwarded to the callback.
        * \return Identifier of the added writer.
        */
        int AddWriter(Level level, CallbackRawFunc callbackRaw, void* userData)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _writers[++_writerId] = Writer(level, NULL, NULL, callbackRaw, userData);
            _levelMax = std::max<Level>(_levelMax, level);
            return _writerId;
        }

        /*!
        * \fn int AddStdWriter(Level level)
        * \brief Registers a writer that prints formatted messages to the standard output.
        * \param [in] level - Maximum severity this writer will accept.
        * \return Identifier of the added writer.
        */
        int AddStdWriter(Level level)
        {
            return AddWriter(level, StdWrite, NULL);
        }

        /*!
        * \fn int AddFileWriter(Level level, const String& fileName)
        * \brief Registers a writer that appends formatted messages to a file.
        * \param [in] level - Maximum severity this writer will accept.
        * \param [in] fileName - Path of the log file to create or open.
        * \return Identifier of the added writer, or 0 if the file could not be opened.
        */
        int AddFileWriter(Level level, const String& fileName)
        {
            std::unique_ptr<std::ofstream> file(new std::ofstream(fileName));
            if (!file->is_open())
                return 0;
            std::lock_guard<std::mutex> lock(_mutex);
            _writers[++_writerId] = Writer(level, std::move(file));
            _levelMax = std::max<Level>(_levelMax, level);
            _rawOnly = false;
            return _writerId;
        }

        /*!
        * \fn bool RemoveWriter(int id)
        * \brief Removes a previously registered writer. The file of a file writer is closed.
        * \param [in] id - Writer identifier returned by AddWriter, AddStdWriter or AddFileWriter.
        * \return true if the writer was found and removed, false otherwise.
        */
        bool RemoveWriter(int id)
        {
            // The file outlives the writer until the lock is released: closing it flushes the last messages,
            // and on a slow medium that flush would hold back every thread that logs. AddFileWriter keeps the
            // opening of a file out of the lock for the same reason.
            std::unique_ptr<std::ofstream> file;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                Writers::iterator it = _writers.find(id);
                if (it == _writers.end())
                    return false;
                file = std::move(it->second.file);
                _writers.erase(it);
                // The maximum level cannot be lowered by one step: the second highest level of the remaining
                // writers is not stored anywhere, so both summaries are recomputed from the writers themselves.
                // They are published once, at the end: Enable() reads the level without the lock and would take
                // an intermediate value for the answer, dropping a message that a writer still accepts.
                Level levelMax = None;
                bool rawOnly = true;
                for (Writers::const_iterator writer = _writers.begin(); writer != _writers.end(); ++writer)
                {
                    levelMax = std::max(levelMax, writer->second.level);
                    if (writer->second.callback || writer->second.file)
                        rawOnly = false;
                }
                _levelMax = levelMax;
                _rawOnly = rawOnly;
            }
            return true;
        }

        /*!
        * \fn void SetFlags(Flags flags)
        * \brief Sets the formatting flags used when a formatted writer is registered.
        * \param [in] flags - Combination of Flags values.
        */
        void SetFlags(Flags flags)
        {
            _flags = flags;
        }

        /*!
        * \fn Flags GetFlags() const
        * \brief Returns the current formatting flags.
        * \return Current Flags value.
        */
        Flags GetFlags() const
        {
            return _flags;
        }

        /*!
        * \fn bool Enable(Level level) const
        * \brief Checks whether a message of the given severity would be written.
        * \param [in] level - Severity to check.
        * \return true if level is not None and is less or equal to the maximum registered writer level.
        * \note The level is read without the lock, so the answer can be stale while writers are added or
        *       removed. Write() checks the level of every writer again under the lock, so a stale answer
        *       costs an unnecessary lock and never sends a message to a writer that no longer accepts it.
        */
        CPL_INLINE bool Enable(Level level) const
        {
            return level != None && _levelMax.load(std::memory_order_relaxed) >= level;
        }

        /*!
        * \fn void Write(Level level, const String& message, int id = -1) const
        * \brief Writes a message to matching writers. Formatted writers receive a prefixed line;
        *        raw writers receive the original message.
        * \param [in] level - Severity of the message.
        * \param [in] message - Message text.
        * \param [in] id - Target writer identifier, or -1 to send the message to every matching writer.
        */
        void Write(Level level, const String& message, int id = -1) const
        {
            if (!Enable(level))
                return;

            // The flags are taken once and the whole message is built from that one set: read field by field,
            // they can change between the parts of the prefix and produce a line that no set of flags describes.
            const Flags flags = _flags.load(std::memory_order_relaxed);
            const bool formatted = !_rawOnly.load(std::memory_order_relaxed);
            std::stringstream ss;

            if (formatted)
            {
                // Every part of the prefix writes a separating space before itself when the prefix is not
                // empty yet, and reports the prefix as written; the terminating ": " is written for a
                // prefix that is not empty.
                bool prefixWritten = false;
                if (flags & WriteDate)
                {
                    ss << CurrentDateTimeString(true, false);
                    prefixWritten = true;
                }
                if (flags & WriteTime)
                {
                    if (prefixWritten)
                        ss << " ";
                    ss << CurrentDateTimeString(false, true);
                    prefixWritten = true;
                }
                if (flags & WriteThreadId)
                {
                    if (prefixWritten)
                        ss << " ";
                    std::thread::id id = std::this_thread::get_id();
                    if (flags & PrettyThreadId)
                    {
                        std::lock_guard<std::mutex> lock(_mutex);
                        if (_prettyThreadNames.find(id) == _prettyThreadNames.end())
                            _prettyThreadNames[id] = ToStr((int)_prettyThreadNames.size(), 3);
                        ss << "[" << _prettyThreadNames[id] << "]";
                    }
                    else
                        ss << "[" << id << "]";
                    prefixWritten = true;
                }
                if (flags & WritePrefix)
                {
                    if (prefixWritten)
                        ss << " ";
                    level = std::min(level, Debug);
                    static const String prefixes[] = { "None", "Error", "Warning", "Info", "Verbose", "Debug" };
                    if (flags & ColorezedPrefix)
                    {
                        using namespace Console;
                        static Foreground colors[] = { ForegroundBlack, ForegroundLightRed, ForegroundYellow, ForegroundGreen, ForegroundWhite, ForegroundLightGray };
                        ss << Stylized(prefixes[level], FormatDefault, colors[level]);
                    }
                    else
                        ss << prefixes[level];
                    prefixWritten = true;
                }
                if (prefixWritten)
                    ss << ": ";

                ss << message;
                ss << std::endl;
            }

            std::lock_guard<std::mutex> lock(_mutex);
            for (Writers::const_iterator it = _writers.begin(); it != _writers.end(); ++it)
            {
                const Writer& writer = it->second;
                if (level <= writer.level && (id == -1 || id == it->first))
                {
                    // A writer that takes a formatted line while the message was built without one was
                    // registered after the decision not to format: treat it as not registered yet.
                    if ((writer.file || writer.callback) && !formatted)
                        continue;
                    if (writer.file)
                        *writer.file << ss.str() << std::flush;
                    else if (writer.callback)
                        writer.callback(ss.str().c_str(), writer.userData);
                    else if (writer.callbackRaw)
                        writer.callbackRaw(level, message.c_str(), writer.userData);
                    else if (writer.callbackRawFunc)
                        writer.callbackRawFunc(level, message.c_str(), writer.userData);
                    else
                        assert(0);
                }
            }
        }

        /*!
        * \fn Level MaxLevel() const
        * \brief Returns the highest severity registered among writers.
        * \return Maximum writer Level, or None if no writers are registered.
        */
        Level MaxLevel() const
        {
            return _levelMax.load(std::memory_order_relaxed);
        }

        /*!
        * \fn static Log& Global()
        * \brief Returns the process-wide logger singleton used by the logging macros.
        * \return Reference to the global Log instance.
        */
        static Log& Global()
        {
            static Log log;
            return log;
        }

    private:
        struct Writer
        {
            Level level;
            Callback callback;
            CallbackRaw callbackRaw;
            CallbackRawFunc callbackRawFunc;
            void* userData;
            // Set for a file writer, which owns its file and closes it when the writer is destroyed.
            std::unique_ptr<std::ofstream> file;

            Writer(Level l = None, Callback c = NULL, CallbackRaw cr = NULL, CallbackRawFunc crf = NULL, void* ud = NULL)
                : level(l)
                , callback(c)
                , callbackRaw(cr)
                , callbackRawFunc(crf)
                , userData(ud)
            {
            }

            Writer(Level l, std::unique_ptr<std::ofstream> f)
                : level(l)
                , callback(NULL)
                , callbackRaw(NULL)
                , callbackRawFunc(NULL)
                , userData(NULL)
                , file(std::move(f))
            {
            }
        };
        typedef std::map<int, Writer> Writers;
        Writers _writers;
        int _writerId;

        mutable std::mutex _mutex;
        mutable std::map<std::thread::id, String> _prettyThreadNames;
        // Written under _mutex and read by Enable() without it, hence atomic.
        std::atomic<Level> _levelMax;
        // Read by Write() without the lock, so that a message is formatted outside it.
        std::atomic<Flags> _flags;
        std::atomic<bool> _rawOnly;

        static void StdWrite(const char* msg, void*)
        {
            std::cout << msg << std::flush;
        }
    };
}

/*! @ingroup cpl_log
* \def CPL_LOG(level, msg)
* \brief Writes a message to the global logger.
* \param level - Severity enumerator name without the Log:: prefix (Error, Warning, Info, Verbose or Debug).
* \param msg - Message string.
*/
#define CPL_LOG(level, msg) \
    Cpl::Log::Global().Write(Cpl::Log::level, msg);

/*! @ingroup cpl_log
* \def CPL_LOG_ID(level, msg, id)
* \brief Writes a message to a single writer of the global logger.
* \param level - Severity enumerator name without the Log:: prefix (Error, Warning, Info, Verbose or Debug).
* \param msg - Message string.
* \param id - Writer identifier returned by AddWriter, AddStdWriter or AddFileWriter.
*/
#define CPL_LOG_ID(level, msg, id) \
    Cpl::Log::Global().Write(Cpl::Log::level, msg, id);

/*! @ingroup cpl_log
* \def CPL_LOG_SS(level, msg)
* \brief Writes a stream-style message to the global logger.
* \param level - Severity enumerator name without the Log:: prefix (Error, Warning, Info, Verbose or Debug).
* \param msg - Stream expression, for example "value = " << value.
*/
#define CPL_LOG_SS(level, msg) \
    { \
        std::stringstream cplLogMacroStream; \
        cplLogMacroStream << msg; \
        Cpl::Log::Global().Write(Cpl::Log::level, cplLogMacroStream.str()); \
    }

/*! @ingroup cpl_log
* \def CPL_LOG_SS_ID(level, msg, id)
* \brief Writes a stream-style message to a single writer of the global logger.
* \param level - Severity enumerator name without the Log:: prefix (Error, Warning, Info, Verbose or Debug).
* \param msg - Stream expression, for example "value = " << value.
* \param id - Writer identifier returned by AddWriter, AddStdWriter or AddFileWriter.
*/
#define CPL_LOG_SS_ID(level, msg, id) \
    { \
        std::stringstream cplLogMacroStream; \
        cplLogMacroStream << msg; \
        Cpl::Log::Global().Write(Cpl::Log::level, cplLogMacroStream.str(), id); \
    }

/*! @ingroup cpl_log
* \def CPL_IF_LOG_SS(cond, level, msg)
* \brief Writes a stream-style message to the global logger when the condition is true.
* \param cond - Condition that must be true to write the message.
* \param level - Severity enumerator name without the Log:: prefix (Error, Warning, Info, Verbose or Debug).
* \param msg - Stream expression, for example "value = " << value.
*/
#define CPL_IF_LOG_SS(cond, level, msg) \
    if(cond) \
    { \
        std::stringstream cplLogMacroStream; \
        cplLogMacroStream << msg; \
        Cpl::Log::Global().Write(Cpl::Log::level, cplLogMacroStream.str()); \
    }

/*! @ingroup cpl_log
* \def CPL_IF_LOG_SS_ID(cond, level, msg, id)
* \brief Writes a stream-style message to a single writer of the global logger when the condition is true.
* \param cond - Condition that must be true to write the message.
* \param level - Severity enumerator name without the Log:: prefix (Error, Warning, Info, Verbose or Debug).
* \param msg - Stream expression, for example "value = " << value.
* \param id - Writer identifier returned by AddWriter, AddStdWriter or AddFileWriter.
*/
#define CPL_IF_LOG_SS_ID(cond, level, msg, id) \
    if(cond) \
    { \
        std::stringstream cplLogMacroStream; \
        cplLogMacroStream << msg; \
        Cpl::Log::Global().Write(Cpl::Log::level, cplLogMacroStream.str(), id); \
    }

#else

#define CPL_LOG(level, msg)
#define CPL_LOG_ID(level, msg, id)
#define CPL_LOG_SS(level, msg)
#define CPL_LOG_SS_ID(level, msg, id)
#define CPL_IF_LOG_SS(cond, level, msg)
#define CPL_IF_LOG_SS_ID(cond, level, msg, id)

#endif
