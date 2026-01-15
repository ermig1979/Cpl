/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2024 Yermalayeu Ihar.
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

#include "Cpl/Table.h"

namespace Test
{
    Cpl::Table GetTestTable()
    {
        Cpl::Table table(3, 3);

        table.SetHeader(0, "name", true);
        table.SetHeader(1, "value", false, Cpl::Table::Center);
        table.SetHeader(2, "description", true, Cpl::Table::Center);

        table.SetRowProp(0);
        table.SetRowProp(1);
        table.SetRowProp(2, true, true);

        table.SetCell(0, 0, "July");
        table.SetCell(0, 1, "google.com", Cpl::Table::Black, "http://google.com");
        table.SetCell(0, 2, "August");
        table.SetCell(1, 0, "8");
        table.SetCell(1, 1, "9");
        table.SetCell(1, 2, "10", Cpl::Table::Red);
        table.SetCell(2, 0, "night");
        table.SetCell(2, 1, "sun", Cpl::Table::Red);
        table.SetCell(2, 2, "day");
        return table;
    }

    bool TableSimpleTest()
    {
        Cpl::Table table = GetTestTable();

        CPL_LOG_SS(Info, std::endl << table.GenerateText());

        std::ofstream ofsHtml("simple_table.html");
        if (ofsHtml.is_open())
        {
            ofsHtml << "<html><body>" << std::endl;
            ofsHtml << "<h2>simple table</h2>" << std::endl;
            ofsHtml << table.GenerateHtml();
            ofsHtml << "</body></html>" << std::endl;
            ofsHtml.close();
        }

        std::ofstream ofsText("simple_table.txt");
        if (ofsText.is_open())
        {
            ofsText << "simple table" << std::endl << std::endl;
            ofsText << table.GenerateText();
            ofsText.close();
        }

        return true;
    }

    bool TableSortableTest()
    {
        Cpl::Table table = GetTestTable();

        CPL_LOG_SS(Info, std::endl << table.GenerateText());

        std::ofstream ofs("sortable_table.html");
        if (ofs.is_open())
        {
            ofs << "<html><body>" << std::endl;
            ofs << "<h2>sortable table</h2>" << std::endl;
            ofs << table.GenerateHtml(0, true, true, true);
            ofs << "</body></html>" << std::endl;
            ofs.close();
        }

        return true;
    }
}
