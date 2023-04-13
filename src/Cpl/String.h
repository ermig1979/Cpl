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
#include <memory>

namespace Cpl
{
    template<class T> CPL_INLINE  String ToStr(const T& value)
    {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

    template<> CPL_INLINE String ToStr<size_t>(const size_t& value)
    {
        return ToStr((ptrdiff_t)value);
    }

    template<> CPL_INLINE String ToStr<float>(const float& value)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(std::numeric_limits<float>::digits10);
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

    CPL_INLINE String ToStr(int value, int width)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(width) << value;
        return ss.str();
    }

    CPL_INLINE String ToStr(size_t value, int width)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(width) << value;
        return ss.str();
    }

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

    CPL_INLINE Strings Separate(const String& str, const String& delimeter)
    {
        size_t current = 0;
        Strings result;
        while (current != String::npos)
        {
            size_t next = str.find(delimeter, current);
            String value = str.substr(current, next - current);
            if(!value.empty())
                result.push_back(value);
            current = next;
            if (current != String::npos)
                current += delimeter.size();
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    CPL_INLINE String CurrentDateTimeString()
    {
        std::time_t t;
        std::time(&t);
        std::tm* tm = ::localtime(&t);
        std::stringstream ss;
        ss << ToStr(tm->tm_year + 1900, 4) << "."
            << ToStr(tm->tm_mon + 1, 2) << "."
            << ToStr(tm->tm_mday, 2) << " "
            << ToStr(tm->tm_hour, 2) << ":"
            << ToStr(tm->tm_min, 2) << ":"
            << ToStr(tm->tm_sec, 2);
        return ss.str();
    }
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    CPL_INLINE String TimeToStr(double time)
    {
        std::stringstream ss;
        ss << ToStr(int(time) / 60 / 60, 2)
            << ":" << ToStr(int(time) / 60 % 60, 2)
            << ":" << ToStr(int(time) % 60, 2)
            << "." << ToStr(int(time * 1000) % 1000, 3);
        return ss.str();
    }

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
