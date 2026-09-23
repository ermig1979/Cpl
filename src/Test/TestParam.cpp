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

#include "Cpl/Param.h"

namespace Test
{
    bool ParamSimpleTest(const Options& options)
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

        test.Save(options.OutputPath("simple_short.xml"), false);
        test.Save(options.OutputPath("simple_full.xml"), true);

        if (!loaded.Load(options.OutputPath("simple_short.xml")))
            return false;

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    bool ParamSizetDefaultTest(const Options& options)
    {
        struct TestParam {
            CPL_PARAM_VALUE(size_t, value_s, 1);
            CPL_PARAM_VALUE(bool, value_b, true);
            CPL_PARAM_VALUE(float, value1_f, 2.f);
            CPL_PARAM_VALUE(float, value2_f, 3.f);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        String xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?><test><value_b></value_b><value_s></value_s><value1_f></value1_f></test>";

        if (!loaded.Load(xml.c_str(), xml.size(), Cpl::ParamFormatXml))
            return false;

        return loaded().value_b() && (loaded().value_s() == 1) && (loaded().value1_f() == 2.f) && (loaded().value2_f() == 3.f);
    }

    //---------------------------------------------------------------------------------------------

    bool ParamStructTest(const Options& options)
    {
        struct ChildParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(String, name, "Name");
            //CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
        };

        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_STRUCT(ChildParam, child);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().child().name() = "Horse";

        test.Save(options.OutputPath("struct_short.xml"), false);
        test.Save(options.OutputPath("struct_full.xml"), true);

        if (!loaded.Load(options.OutputPath("struct_full.xml")))
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

    bool ParamStructModTest(const Options& options)
    {
        struct TestParam
        {
            CPL_PARAM_STRUCT_MOD(OrigChildParam, childA, ChildParamA());
            CPL_PARAM_STRUCT_MOD(OrigChildParam, childB, ChildParamB());
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test.Save(options.OutputPath("struct_mod_short.xml"), false);
        test.Save(options.OutputPath("struct_mod_full.xml"), true);

        if (!loaded.Load(options.OutputPath("struct_mod_full.xml")))
            return false;

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    bool ParamVectorTest(const Options& options)
    {
        struct ChildParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(String, name, "Name");
            //CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
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

        test.Save(options.OutputPath("vector_short.xml"), false);
        test.Save(options.OutputPath("vector_full.xml"), true);

        if (!loaded.Load(options.OutputPath("vector_short.xml")))
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
    bool ParamEnumTest(const Options& options)
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

        test.Save(options.OutputPath("enum_short.xml"), false);
        test.Save(options.OutputPath("enum_full.xml"), true);

        if (!loaded.Load(options.OutputPath("enum_full.xml")))
            return false;

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    bool ParamMapTest(const Options& options)
    {
        struct ValueParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(String, name, "Name");
            //CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
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

        test.Save(options.OutputPath("map_short.xml"), false);
        copy.Save(options.OutputPath("map_copy_full.xml"), true);

        if (!loaded.Load(options.OutputPath("map_copy_full.xml")))
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
    bool ParamMapBugTest(const Options& options)
    {
        B::PipelineParamHolder test, loaded, copy;

        test().inference()["gender"].config() = "gender.txt";

        copy.Clone(test);

        test.Save(options.OutputPath("map_short.xml"), false);
        copy.Save(options.OutputPath("map_copy_full.xml"), true);

        if (!loaded.Load(options.OutputPath("map_copy_full.xml")))
            return false;

        return loaded.Equal(test);
    }
}

//---------------------------------------------------------------------------------------------

namespace Test
{
    bool ParamMapEqualEveryEntryTest(const Options& options)
    {
        struct ValueParam
        {
            CPL_PARAM_VALUE(Int, value, 0);
        };

        struct TestParam
        {
            CPL_PARAM_MAP(String, ValueParam, map);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        // The maps differ in their second entry only, so a comparison that stops after the first
        // one takes them for equal.
        TestParamHolder first, second;
        first().map()["a"].value() = 1;
        first().map()["b"].value() = 2;
        second().map()["a"].value() = 1;
        second().map()["b"].value() = 3;

        if (first.Equal(second))
        {
            CPL_LOG_SS(Error, "Two maps that differ in their second entry are taken for equal!");
            return false;
        }

        return true;
    }
}

//---------------------------------------------------------------------------------------------

namespace Test
{
    // A child that fails to load fails the whole load: a container that takes the failure for a
    // success leaves every child after the failed one with the value it had before.
    bool ParamXmlLoadChildFailureTest(const Options& options)
    {
        struct Leaf
        {
            CPL_PARAM_VALUE(Int, value, 0);
        };

        // A map fails its own load on an element of a foreign name, which makes it the child that
        // fails inside each of the containers below.
        struct Entry
        {
            CPL_PARAM_MAP(String, Leaf, sub);
            CPL_PARAM_VALUE(Int, x, 0);
        };

        struct StructParam
        {
            CPL_PARAM_VALUE(Int, head, 0);
            CPL_PARAM_MAP(String, Leaf, sub);
            CPL_PARAM_VALUE(Int, tail, 0);
        };

        struct MapParam
        {
            CPL_PARAM_MAP(String, Entry, map);
        };

        struct VectorParam
        {
            CPL_PARAM_VECTOR(Entry, list);
        };

        CPL_PARAM_HOLDER(StructHolder, StructParam, test);
        CPL_PARAM_HOLDER(MapHolder, MapParam, test);
        CPL_PARAM_HOLDER(VectorHolder, VectorParam, test);

        CPL_LOG_SS(Info, "The three loads below must fail.");

        if (!LoadFails<StructHolder>(options, "load_failure_struct.xml",
            "<test><head>1</head><sub><bogus/></sub><tail>9</tail></test>"))
            return false;

        if (!LoadFails<MapHolder>(options, "load_failure_map.xml",
            "<test><map>"
            "<item><first>a</first><second><sub><bogus/></sub><x>5</x></second></item>"
            "<item><first>b</first><second><x>7</x></second></item>"
            "</map></test>"))
            return false;

        if (!LoadFails<VectorHolder>(options, "load_failure_vector.xml",
            "<test><list><item><sub><bogus/></sub><x>5</x></item><item><x>7</x></item></list></test>"))
            return false;

        return true;
    }

    // The same rule for YAML, where a value that is not a scalar fails the load of a plain parameter.
    bool ParamYamlLoadChildFailureTest(const Options& options)
    {
        struct Leaf
        {
            CPL_PARAM_VALUE(Int, value, 0);
        };

        struct StructParam
        {
            CPL_PARAM_VALUE(Int, head, 0);
            CPL_PARAM_VALUE(Int, x, 0);
            CPL_PARAM_VALUE(Int, tail, 0);
        };

        struct MapParam
        {
            CPL_PARAM_MAP(String, Leaf, map);
        };

        struct VectorParam
        {
            CPL_PARAM_VECTOR(Leaf, list);
        };

        CPL_PARAM_HOLDER(StructHolder, StructParam, test);
        CPL_PARAM_HOLDER(MapHolder, MapParam, test);
        CPL_PARAM_HOLDER(VectorHolder, VectorParam, test);

        CPL_LOG_SS(Info, "The three loads below must fail.");

        if (!LoadFails<StructHolder>(options, "load_failure_struct.yml",
            "test:\n  head: 1\n  x:\n    a: 1\n  tail: 9\n"))
            return false;

        if (!LoadFails<MapHolder>(options, "load_failure_map.yml",
            "test:\n  map:\n    a:\n      value:\n        b: 1\n    c:\n      value: 7\n"))
            return false;

        if (!LoadFails<VectorHolder>(options, "load_failure_vector.yml",
            "test:\n  list:\n    - value:\n        b: 1\n    - value: 7\n"))
            return false;

        return true;
    }
}

//---------------------------------------------------------------------------------------------

namespace Test
{
    bool ParamLimitedTest(const Options& options)
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

        test.Save(options.OutputPath("limited_short.xml"), false);
        test.Save(options.OutputPath("limited_full.xml"), true);

        if (!loaded.Load(options.OutputPath("limited_full.xml")))
            return false;

        return loaded.Equal(test);
    }
}

//---------------------------------------------------------------------------------------------

namespace Test
{
    // The assert of the macro has to see each of its three arguments as one operand of the
    // comparison, so each of them in turn is written as an expression of its own: the precedence
    // of ?: takes such an argument apart unless the macro wraps it. The parameters are declared
    // where the assert is active and a child process is available, because a failing assert
    // aborts the process that runs it.
#if !defined(NDEBUG) && (defined(__unix__) || defined(__APPLE__))
    struct TernaryDefaultLimitedParam
    {
        CPL_PARAM_LIMITED(int, size, true ? 100 : 0, 0, 10);
    };

    struct TernaryMaxLimitedParam
    {
        CPL_PARAM_LIMITED(int, size, 100, 0, true ? 10 : 1000);
    };

    struct TernaryMinLimitedParam
    {
        CPL_PARAM_LIMITED(int, size, 5, true ? 0 : 1, 10);
    };
#endif

    bool ParamLimitedTernaryArgumentTest(const Options& options)
    {
#if defined(NDEBUG) || !(defined(__unix__) || defined(__APPLE__))
        CPL_LOG_SS(Info, "The test needs an active assert and a child process, it is skipped in this build.");
        return true;
#else
        CPL_LOG_SS(Info, "The first two child processes below must be terminated by the assert of CPL_PARAM_LIMITED.");
        const bool defaultConstructed = RunIsolated([]() -> bool
        {
            TernaryDefaultLimitedParam param;
            return param.size()() == 100;
        });

        if (defaultConstructed)
        {
            CPL_LOG_SS(Error, "The assert of CPL_PARAM_LIMITED accepted a default outside [Min(), Max()] "
                << "written as a ternary expression!");
            return false;
        }

        const bool maxConstructed = RunIsolated([]() -> bool
        {
            TernaryMaxLimitedParam param;
            return param.size()() == 100;
        });

        if (maxConstructed)
        {
            CPL_LOG_SS(Error, "The assert of CPL_PARAM_LIMITED accepted a default above a maximum "
                << "written as a ternary expression!");
            return false;
        }

        // A bound written as a ternary expression must not turn a default that lies inside the
        // range into a failing assert.
        const bool minConstructed = RunIsolated([]() -> bool
        {
            TernaryMinLimitedParam param;
            return param.size()() == 5;
        });

        if (!minConstructed)
        {
            CPL_LOG_SS(Error, "The assert of CPL_PARAM_LIMITED rejected a default inside [Min(), Max()] "
                << "with a minimum written as a ternary expression!");
            return false;
        }

        return true;
#endif
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
        
    bool ParamTemplateTest(const Options& options)
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


        test.Save(options.OutputPath("template_short.yml"), false);
        test.Save(options.OutputPath("template_full.yml"), true);

        test.Save(options.OutputPath("template_short.xml"), false);
        test.Save(options.OutputPath("template_full.xml"), true);

        if (!loaded.Load(options.OutputPath("template_full.yml")))
            return false;

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded full != original");
            loaded.Save(options.OutputPath("template_short_loaded.yml"), false);
            loaded.Save(options.OutputPath("template_full_loaded.yml"), true);
            return false;
        }

        return true;
    }
}



