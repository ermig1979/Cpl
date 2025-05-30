/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2023 Yermalayeu Ihar,
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

#include "Cpl/Defs.h"

#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <memory>

#if _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "windows.h"
#include "winsock.h"

#define CPL_CURRENT_DATE_TIME_PRECISION 3

namespace
{
    // PostgreSQL's implementation of gettimeofday for Windows
    /*
     * gettimeofday.c
     *    Win32 gettimeofday() replacement
     *
     * src/port/gettimeofday.c
     *
     * Copyright (c) 2003 SRA, Inc.
     * Copyright (c) 2003 SKC, Inc.
     *
     * Permission to use, copy, modify, and distribute this software and
     * its documentation for any purpose, without fee, and without a
     * written agreement is hereby granted, provided that the above
     * copyright notice and this paragraph and the following two
     * paragraphs appear in all copies.
     *
     * IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
     * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
     * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     * DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED
     * OF THE POSSIBILITY OF SUCH DAMAGE.
     *
     * THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
     * A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS
     * IS" BASIS, AND THE AUTHOR HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE,
     * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
     */

    /* FILETIME of Jan 1 1970 00:00:00. */
    static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

    /*
     * timezone information is stored outside the kernel so tzp isn't used anymore.
     *
     * Note: this function is not for Win32 high precision timing purpose. See
     * elapsed_time().
     */
    int gettimeofday(struct timeval* tp, struct timezone* tzp)
    {
        FILETIME    file_time;
        SYSTEMTIME  system_time;
        ULARGE_INTEGER ularge;

        GetSystemTime(&system_time);
        SystemTimeToFileTime(&system_time, &file_time);
        ularge.LowPart = file_time.dwLowDateTime;
        ularge.HighPart = file_time.dwHighDateTime;

        tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
        tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

        return 0;
    }
}

#elif __linux__
#include <sys/time.h>

#define CPL_CURRENT_DATE_TIME_PRECISION 6

#endif



namespace Cpl
{
    template<class T> CPL_INLINE  String ToStr(const T& value)
    {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

    template<class T> CPL_INLINE String ToStr(T value, int width)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(width) << value;
        return ss.str();
    }

    template<> CPL_INLINE String ToStr<size_t>(const size_t& value)
    {
        return ToStr((ptrdiff_t)value);
    }

    template<> CPL_INLINE String ToStr<float>(const float& value)
    {
        std::stringstream ss;
        int digits = std::numeric_limits<float>::digits10 + 1, extra = 0;
        float abs = std::abs(value);
        if (abs < 1.0f)
            extra = -(int)std::floor(std::log10(abs));
        if (extra < 5)
            ss << std::fixed;
        ss << std::setprecision(digits + extra);
        ss << value;
        return ss.str();
    }

    template<> CPL_INLINE String ToStr<double>(const double& value)
    {
        std::stringstream ss;
        int digits = std::numeric_limits<double>::digits10 + 1, extra = 0;
        double abs = std::abs(value);
        if (abs < 1.0)
            extra = -(int)std::floor(std::log10(abs));
        if (extra < 8)
            ss << std::fixed;
        ss << std::setprecision(digits + extra);
        ss << value;
        return ss.str();
    }

    template<class T> CPL_INLINE String ToStr(const std::vector<T>& values)
    {
        std::stringstream ss;
        for (size_t i = 0; i < values.size(); ++i)
            ss << (i ? " " : "") << ToStr<T>(values[i]);
        return ss.str();
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE String ToStr(double value, int precision, bool zero = true)
    {
        std::stringstream ss;
        if (value || zero)
            ss << std::setprecision(precision) << std::fixed << value;
        return ss.str();
    }

    //-----------------------------------------------------------------------------------

    template <class T> CPL_INLINE T ToVal(const String& str)
    {
        std::stringstream ss(str);
        T t;
        ss >> t;
        return t;
    }

    //-----------------------------------------------------------------------------------

    template<class T> CPL_INLINE void ToVal(const String& string, T& value)
    {
        std::stringstream ss(string);
        if(string != "" || string != " ")
            ss >> value;
    }

    template<> CPL_INLINE void ToVal<String>(const String& string, String& value)
    {
        if (string != "")
            value = string;
    }

    template<> CPL_INLINE void ToVal<size_t>(const String& string, size_t& value)
    {
        ToVal(string, (ptrdiff_t&)value);
    }

    template<> CPL_INLINE void ToVal<bool>(const String& string, bool& value)
    {
        std::string lower = string;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "0" || lower == "false" || lower == "no" || lower == "off")
            value = false;
        else if (lower == "1" || lower == "true" || lower == "yes" || lower == "on")
            value = true;
        else
            assert(0);
    }

    template<class T> CPL_INLINE void ToVal(const String& string, std::vector<T>& values)
    {
        std::stringstream ss(string);
        values.clear();
        while (!ss.eof())
        {
            String item;
            ss >> item;
            if (item.size())
            {
                T value;
                ToVal(item, value);
                values.push_back(value);
            }
        }
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE String ToLowerCase(const String& src)
    {
        String dst(src);
        for (size_t i = 0; i < dst.size(); ++i)
        {
            if (dst[i] <= 'Z' && dst[i] >= 'A')
                dst[i] = dst[i] - ('Z' - 'z');
        }
        return dst;
    }

    CPL_INLINE bool StartsWith(const String& str, const String& prefix)
    {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

    CPL_INLINE bool EndsWith(const String& str, const String& suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    template<typename ... Args>
    CPL_INLINE String Format(const std::string& format, Args ... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
        if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

    template<typename Enum, int Size> CPL_INLINE Enum ToEnum(const String& string)
    {
        int type = Size - 1;
        for (; type >= 0; --type)
        {
            if (ToLowerCase(ToStr<Enum>((Enum)type)) == ToLowerCase(string))
                return (Enum)type;
        }
        return (Enum)type;
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE Strings Separate(const String& str, const String& delimiter)
    {
        size_t current = 0;
        Strings result;
        if (str.empty())
            return { "" };
        if (delimiter.empty())
        {
            result.reserve(str.size());
            for (const auto& s : str)
                result.emplace_back(&s, 1);
            return result;
        }
        while (current != String::npos)
        {
            size_t next = str.find(delimiter, current);
            String value = str.substr(current, next - current);
            if(!value.empty())
                result.push_back(value);
            current = next;
            if (current != String::npos)
                current += delimiter.size();
        }
        return result;
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE String ExpandLeft(const String& value, size_t count)
    {
        count = std::max(count, value.size());
        std::stringstream ss;
        for (size_t i = value.size(); i < count; i++)
            ss << " ";
        ss << value;
        return ss.str();
    }

    CPL_INLINE String ExpandRight(const String& value, size_t count)
    {
        count = std::max(count, value.size());
        std::stringstream ss;
        ss << value;
        for (size_t i = value.size(); i < count; i++)
            ss << " ";
        return ss.str();
    }

    CPL_INLINE String ExpandBoth(const String& value, size_t count)
    {
        count = std::max(count, value.size());
        std::stringstream ss;
        for (size_t i = 0, left = (count - value.size()) / 2; i < left; i++)
            ss << " ";
        ss << value;
        for (size_t i = ss.str().size(); i < count; i++)
            ss << " ";
        return ss.str();
    }

    //-----------------------------------------------------------------------------------
    
    CPL_INLINE void ReplaceAllInplace(String& str, const String& pattern, const std::string& repl)
    {
        size_t pos = 0;
        auto plen = pattern.length();
        auto rlen = repl.length();
        while ((pos = str.find(pattern, pos)) != std::string::npos) 
        {
            str.replace(pos, plen, repl);
            pos += rlen;
        }
    }

    CPL_INLINE String ReplaceAll(const String& str, const String& pattern, const std::string& repl)
    {
        String res = str;
        ReplaceAllInplace(res, pattern, repl);
        return res;
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE Strings Separate(const String& str0, const Strings& delimiters)
    {
        if (delimiters.empty())
            return {str0};
        String str = str0;
        String nonEmptyDelimiter;
        for (const auto& del : delimiters)
        {
            if (del.empty())
            {
                for (const auto& d : delimiters)
                {
                    if (!d.empty())
                        ReplaceAllInplace(str, d, "");
                }
                return Separate(str, "");
            }
            if (nonEmptyDelimiter.empty())
                nonEmptyDelimiter = del;
        }
        for (const auto& del : delimiters)
            ReplaceAllInplace(str, del, nonEmptyDelimiter);
        return Separate(str, nonEmptyDelimiter);
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE Strings Split(const String& str0, const Strings& delimiters)
    {
        return Separate(str0, delimiters);
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE Strings Split(const String& str0, const String& delimiter)
    {
        return Separate(str0, delimiter);
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE void TrimLeftInplace(String& str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE void TrimRightInplace(String& str)
    {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), str.end());
    }

    //-----------------------------------------------------------------------------------

    CPL_INLINE void TrimInplace(String& str)
    {
        TrimRightInplace(str);
        TrimLeftInplace(str);
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    // For Windows time precision is milliseconds
    CPL_INLINE String CurrentDateTimeString(bool date = true, bool time = true, int msDigits = CPL_CURRENT_DATE_TIME_PRECISION)
    {
        std::time_t t;
        std::time(&t);
        std::tm* tm = ::localtime(&t);
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        std::stringstream ss;
        
        if (date)
            ss << ToStr(tm->tm_year + 1900, 4) << "."  << ToStr(tm->tm_mon + 1, 2) << "." << ToStr(tm->tm_mday, 2);
        if (date && time)
            ss << " ";
        if (time)
        {
            ss << ToStr(tm->tm_hour, 2) << ":" << ToStr(tm->tm_min, 2) << ":" << ToStr(tm->tm_sec, 2);
            if (msDigits > 0)
            {
                if (msDigits > CPL_CURRENT_DATE_TIME_PRECISION)
                    msDigits = CPL_CURRENT_DATE_TIME_PRECISION;
                // 6 because we are working with microseconds
                auto sInt = static_cast<decltype(current_time.tv_usec)>((double)current_time.tv_usec * pow(10, (int)msDigits - 6));
                ss << "." << ToStr(sInt, msDigits);
            }
        }
        return ss.str();
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    //-----------------------------------------------------------------------------------

    // Prints time in seconds as 'hh:mm:ss.zzz'
    CPL_INLINE String TimeToStr(double time, bool cutTo24hours = false)
    {
        std::stringstream ss;
        double hours = time / 3600;
        (void)modf(hours, &hours);

        time -= hours * 3600;

        if (cutTo24hours)
        {
            double r = 0;
            (void)modf(hours / 24, &r);
            hours = hours - r * 24;
        }

        if (hours < 10)
            ss << ToStr((size_t)hours, 2);
        else
            ss << ToStr((size_t)hours);

        ss << ":" << ToStr(size_t(time) / 60 % 60, 2)
            << ":" << ToStr(size_t(time) % 60, 2)
            << "." << ToStr(size_t((time - (size_t)time) * 1000), 3);
        return ss.str();
    }

    //-----------------------------------------------------------------------------------

    // prefix, login, password, path
    CPL_INLINE std::array<String, 4> ParseUri(const String& uri)
    {
        String prefix, login, password, path;

        auto prefixPos = uri.find("://");
        size_t prefixSize = 0;
        if (prefixPos != String::npos)
        {
            prefix = uri.substr(0, prefixPos);
            prefixSize = prefixPos + 3;
        }

        bool hasCredentials = false;
        auto atPos = uri.find('@');
        if (atPos != std::string::npos)
        {
            if (atPos != uri.size() - 1)
                path = uri.substr(atPos + 1);
            auto dotsPos = uri.find(':', prefixSize);
            if (dotsPos != std::string::npos)
            {
                login = uri.substr(prefixSize, dotsPos - prefixSize);
                password = uri.substr(dotsPos + 1, atPos - dotsPos - 1);
            }
            else
                login = uri.substr(prefixSize, atPos - prefixSize);
        }
        else
            path = uri.substr(prefixSize);

        return std::array<String, 4> { prefix, login, password, path };
    }
}
