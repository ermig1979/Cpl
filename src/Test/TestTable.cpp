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
    bool TableSimpleTest()
    {
        Cpl::Table table(3, 2, false);

        table.SetHeader(0, "name", true);
        table.SetHeader(1, "value", false, Cpl::Table::Center);
        table.SetHeader(2, "descr.", true, Cpl::Table::Center);

        table.SetRowProp(0);
        table.SetRowProp(1);

        table.SetCell(0, 0, "0-0");
        table.SetCell(1, 0, "black");
        table.SetCell(2, 0, "night");
        table.SetCell(0, 1, "google.com", Cpl::Table::Black, "http://google.com");
        table.SetCell(1, 1, "red", Cpl::Table::Red);
        table.SetCell(2, 1, "apple");

        CPL_LOG_SS(Info, std::endl << table.GenerateText());

        std::ofstream ofs("simple_table.html");
        if (ofs.is_open())
        {
            ofs << "<html><body>" << std::endl;
            ofs << "<h2>simple table</h2>" << std::endl;
            ofs << table.GenerateHtml();
            ofs << "</body></html>" << std::endl;
            ofs.close();
        }

        return true;
    }

    bool TableSortableTest()
    {
        Cpl::Table table(3, 2, true);

        table.SetHeader(0, "name", true);
        table.SetHeader(1, "value", false, Cpl::Table::Center);
        table.SetHeader(2, "descr.", true, Cpl::Table::Center);

        table.SetRowProp(0);
        table.SetRowProp(1);

        table.SetCell(0, 0, "0-0");
        table.SetCell(1, 0, "black");
        table.SetCell(2, 0, "night");
        table.SetCell(0, 1, "google.com", Cpl::Table::Black, "http://google.com");
        table.SetCell(1, 1, "red", Cpl::Table::Red);
        table.SetCell(2, 1, "apple");

        CPL_LOG_SS(Info, std::endl << table.GenerateText());

        std::ofstream ofs("sortable_table.html");
        if (ofs.is_open())
        {
            ofs << "<html><body>" << std::endl;
            ofs << "<h2>sortable table</h2>" << std::endl;
            ofs << table.GenerateHtml();
            ofs << "</body></html>" << std::endl;
            ofs.close();
        }

        return true;
    }
}
