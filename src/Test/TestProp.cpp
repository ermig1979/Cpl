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

#include "Cpl/Prop.h"

namespace Test
{
    struct FirstGroup
    {
        CPL_PROP(String, name, "frame", "frame name");
        CPL_PROP_EX(int, width, 640, 16, 1920, "Image width.");
        CPL_PROP(int, height, 480, "Image height.");
        CPL_PROP(int, reserved, 0, "");
    };

    struct SecondGroup
    {
        CPL_PROP(String, path, "path", "path to model");
        CPL_PROP_EX(int, type, 3, 0, 7, "model type.");
        CPL_PROP(float, coeff, 0.0f, "");
    };

    struct PropConfig
    {
        CPL_PROP_GROUP(FirstGroup, first);
        CPL_PROP_GROUP(SecondGroup, second);
    };

    CPL_PROP_STORAGE(PropStorage, PropConfig, storage);

    bool PropTest()
    {
        PropStorage test, loaded;

        test().first().width() = 400;
        test().second().coeff() = 3.0;

        test.SetProperty("first.name", "new_name");

        test.Save("prop_short.xml", false);
        test.Save("prop_full.xml", true);

        if (!loaded.Load("prop_full.xml"))
            return false;

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded full != original");
            loaded.Save("prop_short_loaded.xml", false);
            loaded.Save("prop_gfull_loaded.xml", true);
            return false;
        }

        return true;
    }
}
