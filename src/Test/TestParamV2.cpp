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

#include "Cpl/ParamV2.h"

namespace Test
{
    // A child that fails to load fails the whole load, the rule ParamXmlLoadChildFailure pins for
    // the containers of Param.h.
    bool ParamV2XmlLoadChildFailureTest(const Options& options)
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

        struct VectorParam
        {
            CPL_PARAM_VECTOR_V2(Entry, list);
        };

        struct MapParam
        {
            CPL_PARAM_MAP_V2(String, Entry, map);
        };

        CPL_PARAM_HOLDER(VectorHolder, VectorParam, test);
        CPL_PARAM_HOLDER(MapHolder, MapParam, test);

        CPL_LOG_SS(Info, "Both loads below must fail.");

        if (!LoadFails<VectorHolder>(options, "load_failure_vector_v2.xml",
            "<test><list><item><sub><bogus/></sub><x>5</x></item><item><x>7</x></item></list></test>"))
            return false;

        if (!LoadFails<MapHolder>(options, "load_failure_map_v2.xml",
            "<test><map>"
            "<item><first>a</first><second><sub><bogus/></sub><x>5</x></second></item>"
            "<item><first>b</first><second><x>7</x></second></item>"
            "</map></test>"))
            return false;

        return true;
    }

    bool ParamVectorV2Test(const Options& options)
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
            CPL_PARAM_VECTOR_V2(ChildParam, children);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().children().resize(2);
        test().children()[0].value() = 5;

        test.Save(options.OutputPath("vector_v2_short.xml"), false);
        test.Save(options.OutputPath("vector_v2_full.xml"), true);

        if (!loaded.Load(options.OutputPath("vector_v2_short.xml")))
            return false;

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    bool ParamMapV2Test(const Options& options)
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
            CPL_PARAM_MAP_V2(String, ValueParam, map);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded, copy;

        test().map()["new"].value() = 42;
        test().map()["old"];

        copy.Clone(test);

        test.Save(options.OutputPath("map_v2_short.xml"), false);
        copy.Save(options.OutputPath("map_v2_copy_full.xml"), true);

        if (!loaded.Load(options.OutputPath("map_v2_copy_full.xml")))
            return false;

        return loaded.Equal(test);
    }
}
