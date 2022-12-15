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

        return loaded.Equal(test);
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

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    struct OrigChildParam
    {
        CPL_PARAM_VALUE(Int, value, 0);
        CPL_PARAM_VALUE(String, name, "");
    };

    OrigChildParam ChildParamA()
    {
        OrigChildParam param;
        param.value() = 1;
        param.name() = "A";
        return param;
    }

    OrigChildParam ChildParamB()
    {
        OrigChildParam param;
        param.value() = 2;
        param.name() = "B";
        return param;
    }

    bool ParamStructModTest()
    {
        struct TestParam
        {
            CPL_PARAM_STRUCT_MOD(OrigChildParam, childA, ChildParamA());
            CPL_PARAM_STRUCT_MOD(OrigChildParam, childB, ChildParamB());
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test.Save("struct_mod_short.xml", false);
        test.Save("struct_mod_full.xml", true);

        if (!loaded.Load("struct_mod_full.xml"))
            return false;

        return loaded.Equal(test);
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

        test().children().resize(2);
        test().children()[0].value() = 5;

        test.Save("vector_short.xml", false);
        test.Save("vector_full.xml", true);

        if (!loaded.Load("vector_short.xml"))
            return false;

        return loaded.Equal(test);
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

        return loaded.Equal(test);
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

        return loaded.Equal(test);
    }
}

//---------------------------------------------------------------------------------------------

CPL_PARAM_ENUM1(A, DeviceType,
    DeviceTypeGpu,
    DeviceTypeCpu
);

CPL_PARAM_ENUM1(A, NetworkMode,
    NetworkModeFp32,
    NetworkModeInt8,
    NetworkModeFp16
);

namespace A
{
    typedef Cpl::String String;

    struct InferParam
    {
        CPL_PARAM_VALUE(String, config, "");
        CPL_PARAM_VALUE(int, batchSize, 1);
        CPL_PARAM_VALUE(NetworkMode, netMode, NetworkModeFp16);
    };
}

namespace B
{
    typedef Cpl::String String;

    struct PipelineParam
    {
        CPL_PARAM_VALUE(String, name, "");
        CPL_PARAM_VALUE(int, gpuId, 0);
        CPL_PARAM_VALUE(size_t, batchSize, 1);
        CPL_PARAM_VALUE(float, fps, 30.0f);
        CPL_PARAM_VALUE(String, srcPath, "");
        CPL_PARAM_VALUE(size_t, srcBeg, 0);
        CPL_PARAM_VALUE(size_t, srcEnd, -1);
        CPL_PARAM_VALUE(String, outPath, "");
        CPL_PARAM_STRUCT(A::InferParam, detector);
        CPL_PARAM_STRUCT(A::InferParam, classifier);
        CPL_PARAM_STRUCT(A::InferParam, descriptor);
        CPL_PARAM_MAP(String, A::InferParam, inference);
        CPL_PARAM_VALUE(size_t, muxerHeight, 1080);
        CPL_PARAM_VALUE(size_t, muxerWidth, 1920);
        CPL_PARAM_VALUE(int, saveJpegQuality, 85);
    };

    CPL_PARAM_HOLDER(PipelineParamHolder, PipelineParam, pipeline);
}

namespace Test
{
    bool ParamMapBugTest()
    {
        B::PipelineParamHolder test, loaded, copy;

        test().inference()["gender"].config() = "gender.txt";

        copy.Clone(test);

        test.Save("map_short.xml", false);
        copy.Save("map_copy_full.xml", true);

        if (!loaded.Load("map_copy_full.xml"))
            return false;

        return loaded.Equal(test);
    }
}

//---------------------------------------------------------------------------------------------

namespace Test
{
    bool ParamLimitedTest()
    {
        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_LIMITED(Int, value, 0, -5, 6);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        double val = test().value();

        test().value() = 9;

        test.Save("limited_short.xml", false);
        test.Save("limited_full.xml", true);

        if (!loaded.Load("limited_full.xml"))
            return false;

        return loaded.Equal(test);
    }
}

//---------------------------------------------------------------------------------------------

namespace Test
{
    template <class T> struct PropParam
    {
        CPL_PARAM_VALUE(T, value, T());
        CPL_PARAM_VALUE(String, desc, String());
        CPL_PARAM_VALUE(T, value_min, std::numeric_limits<T>::min());
        CPL_PARAM_VALUE(T, value_max, std::numeric_limits<T>::max());
        CPL_PARAM_VALUE(T, value_default, T());
    };
        
    bool ParamTemplateTest()
    {
        struct TestParam
        {
            CPL_PARAM_STRUCT(PropParam<int>, intProp);
            CPL_PARAM_STRUCT(PropParam<String>, strProp);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().intProp().value() = 4;
        test().strProp().value() = "string";


        test.Save("template_short.yml", false);
        test.Save("template_full.yml", true);

        test.Save("template_short.xml", false);
        test.Save("template_full.xml", true);

        if (!loaded.Load("template_full.yml"))
            return false;

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded full != original");
            loaded.Save("template_short_loaded.yml", false);
            loaded.Save("template_full_loaded.yml", true);
            return false;
        }

        return true;
    }
}



