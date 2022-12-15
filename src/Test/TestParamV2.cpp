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
    bool ParamVectorV2Test()
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

        test.Save("vector_v2_short.xml", false);
        test.Save("vector_v2_full.xml", true);

        if (!loaded.Load("vector_v2_short.xml"))
            return false;

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    bool ParamMapV2Test()
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

        test.Save("map_v2_short.xml", false);
        copy.Save("map_v2_copy_full.xml", true);

        if (!loaded.Load("map_v2_copy_full.xml"))
            return false;

        return loaded.Equal(test);
    }

    //---------------------------------------------------------------------------------------------

    struct GroupFirstParam
    {
        CPL_PARAM_PROP(String, name, "frame", "frame name");
        CPL_PARAM_PROP_EX(int, width, 640, 16, 1920, "Image width.");
        CPL_PARAM_PROP(int, height, 480, "Image height.");
        CPL_PARAM_PROP(int, reserved, 0, "");
    };

    struct GroupSecondParam
    {
        CPL_PARAM_PROP(String, path, "path", "path to model");
        CPL_PARAM_PROP_EX(int, type, 3, 0, 7, "model type.");
        CPL_PARAM_PROP(float, reserved, 0.0f, "");
    };

    struct PropGroupMapParam
    {
        CPL_PARAM_STRUCT(GroupFirstParam, first);
        CPL_PARAM_STRUCT(GroupSecondParam, second);
    };

    CPL_PARAM_HOLDER(PropGroupMapParamHolder, PropGroupMapParam, storage);

    bool ParamPropGroupMapTest()
    {
        PropGroupMapParamHolder test, loaded;

        test().first().width() = 400;

        test.Save("prop_group_map_short.xml", false);
        test.Save("prop_group_map_full.xml", true);

        if (!loaded.Load("prop_group_map_full.xml"))
            return false;

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded full != original");
            loaded.Save("prop_group_map_short_loaded.xml", false);
            loaded.Save("prop_group_map_full_loaded.xml", true);
            return false;
        }

        return true;
    }
}
