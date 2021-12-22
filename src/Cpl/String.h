/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2021 Yermalayeu Ihar,
*               2021-2021 Andrey Drogolyub.
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
            result.push_back(str.substr(current, next - current));
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
}
