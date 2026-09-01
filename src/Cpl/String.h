/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
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

#include "Cpl/Defs.h"

#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>

#if _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"
#include "winsock.h"
/*! @ingroup cpl_string
* \def CPL_CURRENT_DATE_TIME_PRECISION
* \brief Default number of fractional-second digits written by CurrentDateTimeString. 3 on Windows, 6 on Linux.
*/
#define CPL_CURRENT_DATE_TIME_PRECISION 3
#elif __linux__
#include <sys/time.h>
/*! @ingroup cpl_string
* \def CPL_CURRENT_DATE_TIME_PRECISION
* \brief Default number of fractional-second digits written by CurrentDateTimeString. 3 on Windows, 6 on Linux.
*/
#define CPL_CURRENT_DATE_TIME_PRECISION 6
#endif

namespace Cpl
{
    /*! @ingroup cpl_string
    * \brief Converts a value to a string with the stream insertion operator.
    * \tparam T - Type of the value. Must be insertable into std::ostream.
    * \param [in] value - Value to convert.
    * \return Decimal (or stream-formatted) representation of value.
    */
    template<class T> CPL_INLINE  String ToStr(const T& value)
    {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

    /*! @ingroup cpl_string
    * \brief Converts a value to a zero-padded string of at least the given width.
    * \tparam T - Type of the value. Must be insertable into std::ostream.
    * \param [in] value - Value to convert.
    * \param [in] width - Minimum field width. Shorter results are padded on the left with '0'.
    * \return Zero-padded string representation of value.
    */
    template<class T> CPL_INLINE String ToStr(T value, int width)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(width) << value;
        return ss.str();
    }

    /*! @ingroup cpl_string
    * \brief Converts a size_t value to a string.
    * \param [in] value - Value to convert.
    * \return Decimal representation of value, formatted through ptrdiff_t.
    */
    template<> CPL_INLINE String ToStr<size_t>(const size_t& value)
    {
        return ToStr((ptrdiff_t)value);
    }

    /*! @ingroup cpl_string
    * \brief Converts a float to a string with extra digits for values whose magnitude is less than 1.
    * \param [in] value - Value to convert.
    * \return Decimal representation of value. Uses fixed notation when fewer than 5 extra digits are required.
    */
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

    /*! @ingroup cpl_string
    * \brief Converts a double to a string with extra digits for values whose magnitude is less than 1.
    * \param [in] value - Value to convert.
    * \return Decimal representation of value. Uses fixed notation when fewer than 8 extra digits are required.
    */
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

    /*! @ingroup cpl_string
    * \brief Converts a vector of values to a space-separated string.
    * \tparam T - Element type. Each element is converted with ToStr.
    * \param [in] values - Values to convert.
    * \return Space-separated concatenation of the converted elements. Empty if values is empty.
    */
    template<class T> CPL_INLINE String ToStr(const std::vector<T>& values)
    {
        std::stringstream ss;
        for (size_t i = 0; i < values.size(); ++i)
            ss << (i ? " " : "") << ToStr<T>(values[i]);
        return ss.str();
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Converts a double to a fixed-precision string.
    * \param [in] value - Value to convert.
    * \param [in] precision - Number of digits after the decimal point.
    * \param [in] zero - If false and value is 0, return an empty string. True by default.
    * \return Fixed-notation string, or an empty string when value is 0 and zero is false.
    */
    CPL_INLINE String ToStr(double value, int precision, bool zero = true)
    {
        std::stringstream ss;
        if (value || zero)
            ss << std::setprecision(precision) << std::fixed << value;
        return ss.str();
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Parses a string into a value of type T.
    * \tparam T - Type to parse. Must be extractable from std::istream.
    * \param [in] str - Input string.
    * \return Parsed value. Default-constructed T if parsing fails.
    */
    template <class T> CPL_INLINE T ToVal(const String& str)
    {
        std::stringstream ss(str);
        T t;
        ss >> t;
        return t;
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Parses a string into an existing value. Leaves value unchanged if the string is empty or a single space.
    * \tparam T - Type to parse. Must be extractable from std::istream.
    * \param [in] string - Input string.
    * \param [in,out] value - Destination updated on success.
    */
    template<class T> CPL_INLINE void ToVal(const String& string, T& value)
    {
        if (string != "" && string != " ")
        {
            std::stringstream ss(string);
            ss >> value;
        }
    }

    /*! @ingroup cpl_string
    * \brief Assigns a string to value unless the input is empty.
    * \param [in] string - Input string. A single space is copied, unlike the generic ToVal overload.
    * \param [in,out] value - Destination updated when string is not empty.
    */
    template<> CPL_INLINE void ToVal<String>(const String& string, String& value)
    {
        if (string != "")
            value = string;
    }

    /*! @ingroup cpl_string
    * \brief Parses a string into a size_t value through ptrdiff_t.
    * \param [in] string - Input string. Empty or a single space leaves value unchanged.
    * \param [in,out] value - Destination updated on success.
    */
    template<> CPL_INLINE void ToVal<size_t>(const String& string, size_t& value)
    {
        if (string != "" && string != " ")
        {
            ptrdiff_t tmp;
            ToVal(string, tmp);
            value = static_cast<size_t>(tmp);
        }
    }

    /*! @ingroup cpl_string
    * \brief Parses a boolean from a case-insensitive token.
    * \param [in] string - Input string. Empty or a single space leaves value unchanged.
    * \param [in,out] value - Set to false for "0", "false", "no" or "off"; true for "1", "true", "yes" or "on".
    * \note Other non-empty tokens trigger assert(0).
    */
    template<> CPL_INLINE void ToVal<bool>(const String& string, bool& value)
    {
        if (string != "" && string != " ")
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
    }

    /*! @ingroup cpl_string
    * \brief Parses a whitespace-separated list of values into a vector.
    * \tparam T - Element type. Each token is converted with ToVal.
    * \param [in] string - Input string.
    * \param [out] values - Destination. Cleared, then filled with the parsed tokens.
    */
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

    /*! @ingroup cpl_string
    * \brief Returns a copy of src with ASCII letters A-Z converted to a-z.
    * \param [in] src - Input string.
    * \return Lower-case copy of src. Characters outside A-Z are unchanged.
    */
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

    /*! @ingroup cpl_string
    * \brief Checks whether str begins with prefix.
    * \param [in] str - String to test.
    * \param [in] prefix - Expected prefix. An empty prefix always matches.
    * \return true if str is at least as long as prefix and starts with it.
    */
    CPL_INLINE bool StartsWith(const String& str, const String& prefix)
    {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

    /*! @ingroup cpl_string
    * \brief Checks whether str ends with suffix.
    * \param [in] str - String to test.
    * \param [in] suffix - Expected suffix. An empty suffix always matches.
    * \return true if str is at least as long as suffix and ends with it.
    */
    CPL_INLINE bool EndsWith(const String& str, const String& suffix)
    {
        return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
    }

    /*! @ingroup cpl_string
    * \brief Formats a string with snprintf-style placeholders.
    * \tparam Args - Types of the format arguments.
    * \param [in] format - printf format string.
    * \param [in] args - Arguments matching the format placeholders.
    * \return Formatted string without a trailing null character.
    * \note Throws std::runtime_error if snprintf reports an error.
    */
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

    /*! @ingroup cpl_string
    * \brief Parses an enumerator from its ToStr name, ignoring ASCII case.
    * \tparam Enum - Enumeration type. Values are assumed to occupy 0 .. Size-1.
    * \tparam Size - Number of enumerators to try, from Size-1 down to 0.
    * \param [in] string - Enumerator name to match.
    * \return Matching enumerator, or (Enum)(-1) if none of the Size names match.
    */
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

    /*! @ingroup cpl_string
    * \brief Splits a string on a delimiter and returns the non-empty parts.
    * \param [in] str - String to split. An empty str yields a one-element list { "" }.
    * \param [in] delimiter - Separator. An empty delimiter splits str into one-character strings.
    * \return List of non-empty tokens. Empty tokens between consecutive delimiters are dropped.
    */
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

    /*! @ingroup cpl_string
    * \brief Pads value on the left with spaces until it is count characters long.
    * \param [in] value - String to pad.
    * \param [in] count - Target width. If value is already longer, it is returned unchanged.
    * \return Left-padded string of length max(count, value.size()).
    */
    CPL_INLINE String ExpandLeft(const String& value, size_t count)
    {
        count = std::max(count, value.size());
        std::stringstream ss;
        for (size_t i = value.size(); i < count; i++)
            ss << " ";
        ss << value;
        return ss.str();
    }

    /*! @ingroup cpl_string
    * \brief Pads value on the right with spaces until it is count characters long.
    * \param [in] value - String to pad.
    * \param [in] count - Target width. If value is already longer, it is returned unchanged.
    * \return Right-padded string of length max(count, value.size()).
    */
    CPL_INLINE String ExpandRight(const String& value, size_t count)
    {
        count = std::max(count, value.size());
        std::stringstream ss;
        ss << value;
        for (size_t i = value.size(); i < count; i++)
            ss << " ";
        return ss.str();
    }

    /*! @ingroup cpl_string
    * \brief Pads value on both sides with spaces until it is count characters long.
    * \param [in] value - String to pad.
    * \param [in] count - Target width. If value is already longer, it is returned unchanged.
    * \return Centered string of length max(count, value.size()). Extra space goes to the right when the pad count is odd.
    */
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
    
    /*! @ingroup cpl_string
    * \brief Replaces every occurrence of pattern in str with repl.
    * \param [in,out] str - String to modify.
    * \param [in] pattern - Substring to search for.
    * \param [in] repl - Replacement text.
    */
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

    /*! @ingroup cpl_string
    * \brief Returns a copy of str with every occurrence of pattern replaced by repl.
    * \param [in] str - Input string.
    * \param [in] pattern - Substring to search for.
    * \param [in] repl - Replacement text.
    * \return New string with replacements applied.
    */
    CPL_INLINE String ReplaceAll(const String& str, const String& pattern, const std::string& repl)
    {
        String res = str;
        ReplaceAllInplace(res, pattern, repl);
        return res;
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Splits a string on any of the given delimiters and returns the non-empty parts.
    * \param [in] str0 - String to split.
    * \param [in] delimiters - Separator strings. An empty list returns { str0 }. If any delimiter is
    *                          empty, the other delimiters are stripped and the remainder is split into
    *                          one-character strings.
    * \return List of non-empty tokens. All non-empty delimiters are treated as equivalent separators.
    */
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

    /*! @ingroup cpl_string
    * \brief Splits a string on any of the given delimiters. Equivalent to Separate.
    * \param [in] str0 - String to split.
    * \param [in] delimiters - Separator strings.
    * \return List of non-empty tokens. Same result as Separate(str0, delimiters).
    */
    CPL_INLINE Strings Split(const String& str0, const Strings& delimiters)
    {
        return Separate(str0, delimiters);
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Splits a string on a delimiter. Equivalent to Separate.
    * \param [in] str0 - String to split.
    * \param [in] delimiter - Separator.
    * \return List of non-empty tokens. Same result as Separate(str0, delimiter).
    */
    CPL_INLINE Strings Split(const String& str0, const String& delimiter)
    {
        return Separate(str0, delimiter);
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Removes leading whitespace from str in place.
    * \param [in,out] str - String to trim. Characters for which std::isspace is true are removed from the front.
    */
    CPL_INLINE void TrimLeftInplace(String& str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Removes trailing whitespace from str in place.
    * \param [in,out] str - String to trim. Characters for which std::isspace is true are removed from the back.
    */
    CPL_INLINE void TrimRightInplace(String& str)
    {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), str.end());
    }

    //-----------------------------------------------------------------------------------

    /*! @ingroup cpl_string
    * \brief Removes leading and trailing whitespace from str in place.
    * \param [in,out] str - String to trim.
    */
    CPL_INLINE void TrimInplace(String& str)
    {
        TrimRightInplace(str);
        TrimLeftInplace(str);
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    /*! @ingroup cpl_string
    * \brief Returns the current local date and/or time as a string.
    * \param [in] date - If true, include the date as YYYY.MM.DD. True by default.
    * \param [in] time - If true, include the time as HH:MM:SS[.fraction]. True by default.
    * \param [in] msDigits - Number of fractional-second digits after the time. Values greater than
    *                        CPL_CURRENT_DATE_TIME_PRECISION are clamped. 0 omits the fraction.
    * \return Concatenation of the requested parts, separated by a space when both date and time are included.
    * \note The default of msDigits is CPL_CURRENT_DATE_TIME_PRECISION (3 on Windows, 6 on Linux).
    */
    CPL_INLINE String CurrentDateTimeString(bool date = true, bool time = true, int msDigits = CPL_CURRENT_DATE_TIME_PRECISION)
    {
        std::time_t t;
        std::time(&t);
        std::tm* tm = ::localtime(&t);
        struct timeval current_time;
#if _WIN32
        FILETIME file_time;
        SYSTEMTIME system_time;
        ULARGE_INTEGER ularge;
        const uint64_t epoch = ((uint64_t)116444736000000000ULL);
        GetSystemTime(&system_time);
        SystemTimeToFileTime(&system_time, &file_time);
        ularge.LowPart = file_time.dwLowDateTime;
        ularge.HighPart = file_time.dwHighDateTime;
        current_time.tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
        current_time.tv_usec = (long)(system_time.wMilliseconds * 1000);
#elif __linux__
        gettimeofday(&current_time, NULL);
#endif
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

    /*! @ingroup cpl_string
    * \brief Formats a duration in seconds as hh:mm:ss.zzz.
    * \param [in] time - Duration in seconds.
    * \param [in] cutTo24hours - If true, the hour field is taken modulo 24. False by default.
    * \return Time string. Hours are two digits when less than 10, otherwise the full hour count.
    *         Minutes, seconds and milliseconds are always two, two and three digits.
    */
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

    /*! @ingroup cpl_string
    * \brief Splits a URI into prefix, login, password and path.
    * \param [in] uri - URI of the form [prefix://][login[:password]@]path.
    * \return Array of four strings: scheme prefix (without "://"), login, password and path.
    * \note Missing parts are empty strings. The first ':' after the prefix separates login and password.
    */
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
