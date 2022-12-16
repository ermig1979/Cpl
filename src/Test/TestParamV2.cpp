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
}
