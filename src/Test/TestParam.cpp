/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2021 Yermalayeu Ihar.
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

#include "Cpl/Param.h"

namespace Test
{
    bool ParamSimpleTest()
    {
        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().value() = 9;

        test.Save("simple_short.xml", false);
        test.Save("simple_full.xml", true);

        if (!loaded.Load("simple_short.xml"))
            return false;

        return true;
    }

    //---------------------------------------------------------------------------------------------

    bool ParamStructTest()
    {
        struct ChildParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
        };

        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_STRUCT(ChildParam, child);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().child().name() = "Horse";

        test.Save("struct_short.xml", false);
        test.Save("struct_full.xml", true);

        if (!loaded.Load("struct_full.xml"))
            return false;

        return true;
    }

    //---------------------------------------------------------------------------------------------

    bool ParamVectorTest()
    {
        struct ChildParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
        };

        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_VECTOR(ChildParam, children);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().children().push_back(ChildParam());
        test().children()[0].value() = 5;

        test.Save("vector_short.xml", false);
        test.Save("vector_full.xml", true);

        if (!loaded.Load("vector_full.xml"))
            return false;

        return true;
    }
}

//-------------------------------------------------------------------------------------------------

CPL_PARAM_ENUM0(Enum, 
    Enum1,
    Enum2,
    Enum3,
    Enum4);

CPL_PARAM_ENUM1(A, Enum,
    Enum1,
    Enum2,
    Enum3,
    Enum4);

CPL_PARAM_ENUM2(A, B, Enum,
    Enum1,
    Enum2,
    Enum3,
    Enum4);

CPL_PARAM_ENUM3(A, B, C, Enum,
    Enum1,
    Enum2,
    Enum3,
    Enum4);

namespace Test
{
    bool ParamEnumTest()
    {
        struct TestParam
        {
            CPL_PARAM_VALUE(Enum, enum0, Enum1);
            CPL_PARAM_VALUE(A::Enum, enum1, A::Enum2);
            CPL_PARAM_VALUE(A::B::Enum, enum2, A::B::Enum3);
            CPL_PARAM_VALUE(A::B::C::Enum, enum3, A::B::C::Enum4);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().enum0() = Enum4;
        test().enum1() = A::Enum3;
        test().enum2() = A::B::Enum2;
        test().enum3() = A::B::C::Enum1;

        test.Save("enum_short.xml", false);
        test.Save("enum_full.xml", true);

        if (!loaded.Load("enum_full.xml"))
            return false;


        return true;
    }

    //---------------------------------------------------------------------------------------------

    bool ParamMapTest()
    {
        struct ValueParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
        };

        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_MAP(String, ValueParam, map);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded, copy;

        test().map()["new"].value() = 42;
        test().map()["old"];

        copy.Clone(test);

        test.Save("map_short.xml", false);
        copy.Save("map_copy_full.xml", true);

        if (!loaded.Load("map_copy_full.xml"))
            return false;

        return true;
    }
}

