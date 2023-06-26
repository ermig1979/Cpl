/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar,
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
#include "Cpl/String.h"

namespace
{
    template<size_t n>
    bool equals(const std::array<Cpl::String, n>& a, const std::array<Cpl::String, n>& b)
    {
        for (size_t i = 0; i < n; ++i)
            if (a[i] != b[i])
                return false;
        return true;
    }
}

namespace Test
{
    bool ParseUriTest()
    {
        std::vector<std::pair<Cpl::String, std::array<Cpl::String, 4>>> testCases =
        {
            {"http://user:pwd@url.com/1", {"http", "user", "pwd", "url.com/1"}},
            {"http://user:p%40wd@url.com/1", {"http", "user", "p%40wd", "url.com/1"}},
            {"http://user:@url.com/1", {"http", "user", "", "url.com/1"}},
            {"http://user@url.com/1", {"http", "user", "", "url.com/1"}},
            {"http://url.com/1", {"http", "", "", "url.com/1"}},
            {"user:pwd@url.com/1", {"", "user", "pwd", "url.com/1"}},
            {"user:@url.com/1", {"", "user", "", "url.com/1"}},
            {"user@url.com/1", {"", "user", "", "url.com/1"}},
            {"url.com/1", {"", "", "", "url.com/1"}},
        };

        for (const auto& testCase : testCases)
        {
            auto test = Cpl::ParseUri(testCase.first);
            if (!equals(test, testCase.second))
            {
                CPL_LOG_SS(Error, testCase.first << " -> " << test[0] << ", "
                    << test[1] << ", " << test[2] << ", " << test[3]);
                return false;
            }
        }

        return true;
    }

    bool SeparateStringTest()
    {
        std::vector<std::tuple<Cpl::String, Cpl::String, std::vector<Cpl::String>>> testCases =
        {
            {"abcd", "", {"a", "b", "c", "d"}},
            {"++a+++bb++", "+", {"a", "bb"}},
            {"", "", {""}},
            {"", "+", {""}},
            {"a aa aaa aaaa", " ", {"a", "aa", "aaa", "aaaa"}},
            {" a aa", " ", {"a", "aa"}},
            {" a a  af f f  ", "  ", {" a a", "af f f"}},
            {" a a ", " " {"a", "a"}},
            {"bababaab", "b", {"a", "a", "aa"}},
            {"bbabbabbaabb", "bb", {"a", "a", "aa"}},
            {" ba bc bdd b", " b", {"a", "c", "dd"}}
        };

        for (const auto& testCase : testCases)
        {
            auto test = Cpl::Separate(std::get<0>(testCase), std::get<1>(testCase));
            if (!equals(test, std::get<2>(testCase)))
            {
                CPL_LOG_SS(Error, std::get<0>(testCase) << " with delimiter " << std::get<1>(testCase));
                return false;
            }
        }

        return true;
    }

    bool SeparateStringMultuTest()
    {
        std::vector<std::tuple<Cpl::String, std::vector<Cpl::String>, std::vector<Cpl::String>>> testCases =
        {
            {"a aa aaa aaaa", {" ", "+"}, {"a", "aa", "aaa", "aaaa"}},
            {"a aa aaa aaaa", {" ", " "}, {"a", "aa", "aaa", "aaaa"}},
            {"a aa aaa aaaa", {}, {"a", " ", "aa", " ", "aaa", " ", "aaaa"}},
            {"a aa aaa aaaa", {""}, {"a", " ", "aa", " ", "aaa", " ", "aaaa"}},
            {"a aa aaa ", {"", " "}, {"a", "a", "a", "a", "a", "a"}},
            {"a aa aaa ", {" ", ""}, {"a", "a", "a", "a", "a", "a"}},
            {"a  b+c  d++ee  ffff", {"  ", "+"}, {"a", "b", "c", "d", "ee", "ffff"}},
            {"a  a+a  a+,+aa  aaaa", {"  ", "+", ","}, {"a", "a","a","a", "aa", "aaaa"}},
        };

        for (const auto& testCase : testCases)
        {
            auto test = Cpl::Separate(std::get<0>(testCase), std::get<1>(testCase));
            if (!equals(test, std::get<2>(testCase)))
            {
                CPL_LOG_SS(Error, std::get<0>(testCase) << " with delimiter " << std::get<1>(testCase));
                return false;
            }
        }

        return true;
    }
}



