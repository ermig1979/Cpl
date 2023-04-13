/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar,
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

#include "Cpl/Xml.h"
#include <iostream>
#include <string>

namespace Test
{
    using MemPool = Cpl::Xml::MemoryPool<char>;
    bool XmlAllocateStringTest()
    {
        MemPool pool;

        constexpr int defaultSize = 8;
        {
            auto t1 = pool.AllocateString(nullptr, defaultSize);
            if (t1[defaultSize] != 0)
                return false;

        }

        const std::string testString("Some test string in 34 byte length");
        { 

            auto t1 = pool.AllocateString(testString.data(), defaultSize);

            for (size_t i = 0; i < defaultSize; i++){
                if (t1[i] != testString[i])
                    return false;
            }
            
            if (t1[defaultSize] != 0)
                return false;

            auto t2 = pool.AllocateString(testString.data(), defaultSize);

            for (size_t i = 0; i < defaultSize; i++){
                if (t1[i] != testString[i])
                    return false;
            }
            
            if (t2[defaultSize] != 0)
                return false;
            //std::cout << (void*) t1 << " "<<  (void*) t2 << "XmlAllocateStringTest" << std::endl;

            if ((void*)t1 == (void*)t2)
                return false;

        }

        {
            auto t1 = pool.AllocateString(testString.c_str(), testString.size());

            for (size_t i = 0; i < testString.size(); i++){
                if (t1[i] != testString[i])
                    return false;
            }

            if (t1[testString.size()] != 0)
                return false;

            auto t2 = pool.AllocateString(testString.c_str(), testString.size() + 1);

            for (size_t i = 0; i < testString.size(); i++){
                if (t2[i] != testString[i])
                    return false;
            }

            if (t2[testString.size()] != 0)
                return false;

        }

        {
            //source null terminated, size undefined
            auto t1 = pool.AllocateString(testString.c_str(), 0);

            for (size_t i = 0; i < testString.size(); i++){
                if (t1[i] != testString[i])
                    return false;
            }

            if (t1[testString.size()] != 0)
                return false;

        }

        {
            const int index = 6;
            auto str = testString;
            str[index] = 0;

            auto t1 = pool.AllocateString(str.c_str(), 0);

            for (size_t i = 0; i < index; i++){
                if (t1[i] != str[i])
                    return false;
            }

            if (t1[index] != 0)
                return false;

        }

        {
            const int size = 6;
            char* temp = (char*) malloc(size);
            for (size_t i = 0; i < size; i++){
                temp[i] = char(10 + i);
            }
            
            auto t1 = pool.AllocateString(temp, size);

            for (size_t i = 0; i < size; i++){
                 if (t1[i] != temp[i])
                     return false;
            }

            if (t1[size] != 0)
                return false;

            free(temp);
        }

        return true;
    }
}