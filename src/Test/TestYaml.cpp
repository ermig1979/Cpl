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

#include "Cpl/Yaml.h"
#include "Cpl/Param.h"

namespace Test
{
    bool YamlSimpleTest()
    {
        const std::string data =
            "data1: \n"
            "  123\n"
            "data2: Hello world\n"
            "data3:\n"
            "   - key1: 123\n"
            "     key2: Test\n"
            "   - Hello world\n"
            "   - 123\n"
            "   - 123.4\n";

        Cpl::Yaml::Node root;
        try
        {
            Cpl::Yaml::Parse(root, data);
        }
        catch (const Cpl::Yaml::Exception e)
        {
            std::cout << "Exception " << e.GetType() << ": " << e.what() << std::endl;
            return false;
        }

        std::cout << root["data1"].As<int>(0) << std::endl;
        std::cout << root["data2"].As<std::string>() << std::endl;
        std::cout << root["data3"][0]["key1"].As<int>(0) << std::endl;
        std::cout << root["data3"][0]["key2"].As<std::string>() << std::endl;
        std::cout << root["data3"][1].As<std::string>() << std::endl;
        std::cout << root["data3"][2].As<int>(0) << std::endl;
        std::cout << root["data3"][3].As<float>(0.0f) << std::endl;
        return true;
    }

    //---------------------------------------------------------------------------------------------

    bool YamlParamTest()
    {
        struct SubParam
        {
            CPL_PARAM_VALUE(Int, id, 1);
            CPL_PARAM_VALUE(String, desc, "no");
        };

        struct TestParam
        {
            CPL_PARAM_VALUE(String, name, "Name");
            CPL_PARAM_VALUE(Int, value, 0);
            CPL_PARAM_VALUE(Strings, letters, Strings({ "A", "B", "C" }));
            CPL_PARAM_STRUCT(SubParam, sub);
            CPL_PARAM_STRUCT(SubParam, orig);
            CPL_PARAM_LIMITED(Int, lim, 3, 0, 5);
            CPL_PARAM_VECTOR(SubParam, subs);
            CPL_PARAM_MAP(String, SubParam, dict);
        };

        CPL_PARAM_HOLDER(TestParamHolder, TestParam, test);

        TestParamHolder test, loaded;

        test().name() = "Changed";
        test().sub().desc() = "description";
        test().lim() = 4;
        test().subs().resize(3);
        test().subs()[0].id() = 7;
        test().subs()[1].desc() = "seven";
        test().dict()["A"].desc() = "A";
        test().dict()["B"];

        test.Save("yaml_short.yml", false, Cpl::ParamFormatYaml);

        test.Save("yaml_full.yml", true, Cpl::ParamFormatYaml);

        if (!loaded.Load("yaml_short.yml", Cpl::ParamFormatYaml))
            return false;
        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded short != original");
            loaded.Save("yaml_short_loaded.yml", false, Cpl::ParamFormatYaml);
            loaded.Save("yaml_full_loaded.yml", true, Cpl::ParamFormatYaml);
            return false;
        }

        if (!loaded.Load("yaml_full.yml", Cpl::ParamFormatYaml))
            return false;
        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded full != original");
            loaded.Save("yaml_short_loaded.yml", false, Cpl::ParamFormatYaml);
            loaded.Save("yaml_full_loaded.yml", true, Cpl::ParamFormatYaml);
            return false;
        }

        return true;
    }
}
