/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar.
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

#pragma once

#include "Cpl/Html.h"
#include "Cpl/String.h"

namespace Cpl
{
    class Table
    {
    public:
        enum Alignment
        {
            Left,
            Center,
            Right,
        };

        enum Color
        {
            Black,
            Red,
        };

        Table(size_t width, size_t height)
            : _width(width)
            , _height(height)
        {
            Init();
        }

        size_t Height() const
        {
            return _height;
        }

        size_t Width() const
        {
            return _width;
        }

        void SetHeader(size_t col, const String& name, bool separator = false, Alignment alignment = Left)
        {
            _headers[col] = Header(name, separator, alignment);
        }

        void SetRowProp(size_t row, bool separator = false, bool bold = false)
        {
            _rows[row] = RowProp(separator, bold);
        }

        void SetCell(size_t col, size_t row, const String& value, Color color = Black, const String & link = "")
        {
            Cell& cell = _cells[row * _width + col];
            cell.value = value;
            cell.color = color;
            cell.link = link;
            _headers[col].width = std::max(_headers[col].width, value.size());
        }

        String GenerateText(size_t indent_ = 0)
        {
            std::stringstream header, separator, table, indent;
            for (size_t i = 0; i < indent_; ++i)
                indent << " ";
            header << "| ";
            for (size_t col = 0; col < _width; ++col)
            {
                header << ExpandText(_headers[col].name, _headers[col]) << " ";
                if (_headers[col].separator)
                    header << "|" << (col == _width - 1 ? "" : " ");
            }
            for (size_t i = 0; i < header.str().size(); ++i)
                separator << "-";
            table << indent.str() << separator.str() << std::endl;
            table << indent.str() << header.str() << std::endl;
            table << indent.str() << separator.str() << std::endl;
            for (size_t row = 0; row < _height; ++row)
            {
                table << indent.str() << "| ";
                for (size_t col = 0; col < _width; ++col)
                {
                    const Cell& cell = _cells[row * _width + col];
                    table << ExpandText(cell.value, _headers[col]);
                    table << (cell.color == Black ? " " : "*");
                    if (_headers[col].separator)
                        table << "|" << (col == _width - 1 ? "" : " ");
                }
                table << std::endl;
                if (_rows[row].separator)
                    table << indent.str() << separator.str() << std::endl;
            }
            table << indent.str() << separator.str() << std::endl;
            return table.str();
        }

        String GenerateHtml(size_t indent = 0)
        {
            std::stringstream stream;
            Html html(stream, indent);

            html.WriteBegin("style", Html::Attr("type", "text/css"), true, true);
            html.WriteText("th.th0 { border-left: 0px; border-top: 0px; border-right: 0px solid #0; border-bottom: 1px solid #0;}", true, true);
            html.WriteText("th.th1 { border-left: 0px; border-top: 0px; border-right: 1px solid #0; border-bottom: 1px solid #0;}", true, true);
            html.WriteText("td.td0b { border-left: 0px; border-top: 0px; border-right: 0px solid #0; border-bottom: 0px; color:#0;}", true, true);
            html.WriteText("td.td1b { border-left: 0px; border-top: 0px; border-right: 1px solid #0; border-bottom: 0px; color:#0;}", true, true);
            html.WriteText("td.td0r { border-left: 0px; border-top: 0px; border-right: 0px solid #0; border-bottom: 0px; color:red;}", true, true);
            html.WriteText("td.td1r { border-left: 0px; border-top: 0px; border-right: 1px solid #0; border-bottom: 0px; color:red;}", true, true);
            html.WriteEnd("style", true, true);

            Html::Attributes attributes;
            attributes.push_back(Html::Attribute("align", "center"));
            attributes.push_back(Html::Attribute("cellpadding", "2"));
            attributes.push_back(Html::Attribute("cellspacing", "0"));
            attributes.push_back(Html::Attribute("border", "1"));
            attributes.push_back(Html::Attribute("cellpadding", "2"));
            attributes.push_back(Html::Attribute("width", "100%"));
            attributes.push_back(Html::Attribute("style", "border-collapse:collapse"));
            html.WriteBegin("table", attributes, true, true);

            html.WriteBegin("tr", Html::Attr("style", "background-color:#e0e0e0; font-weight:bold;"), true, false);
            for (size_t col = 0; col < _width; ++col)
                html.WriteValue("th", Html::Attr("class", String("th") + Cpl::ToStr(_headers[col].separator)), _headers[col].name, false);
            html.WriteEnd("tr", true, true);

            for (size_t row = 0; row < _height; ++row)
            {
                std::stringstream style;
                if (_rows[row].bold)
                    style << "font-weight: bold; background-color:#f0f0f0";
                html.WriteBegin("tr", Html::Attr("align", "center", "style", style.str()), true, false);
                for (size_t col = 0; col < _width; ++col)
                {
                    const Cell& cell = _cells[row * _width + col];
                    html.WriteBegin("td", Html::Attr("class", String("td") + Cpl::ToStr(_headers[col].separator) + ToStr(cell.color)), false, false);
                    if(cell.link.size())
                        html.WriteBegin("a", Html::Attr("href", cell.link), false, false);
                    html.WriteText(cell.value, false, false);
                    if (cell.link.size())
                        html.WriteEnd("a", false, false);
                    html.WriteEnd("td", false, false);
                }
                html.WriteEnd("tr", true, true);
            }

            html.WriteEnd("table", true, true);

            return stream.str();
        }

    private:
        size_t _height, _width;

        struct RowProp
        {
            bool separator;
            bool bold;
            RowProp(bool s = false, bool b = false)
                : separator(s), bold(b) {}
        };
        typedef std::vector<RowProp> RowProps;
        RowProps _rows;

        struct Header
        {
            String name;
            bool separator;
            Alignment alignment;
            size_t width;
            Header(const String n = String(), bool s = false, Alignment a = Left)
                : name(n), separator(s), alignment(a), width(n.size()) {}
        };
        typedef std::vector<Header> Headers;
        Headers _headers;

        struct Cell
        {
            String value, link;
            Color color;
        };
        typedef std::vector<Cell> Cells;
        Cells _cells;

        static String ExpandText(const String& value, const Header& header)
        {
            if (header.alignment == Left)
                return ExpandRight(value, header.width);
            else if (header.alignment == Center)
                return ExpandBoth(value, header.width);
            else if(header.alignment == Right)
                return ExpandLeft(value, header.width);
            assert(0);
            return String();
        }

        void Init()
        {
            _cells.resize(_width * _height);
            _headers.resize(_width);
            _rows.resize(_height);
        }

        String ToStr(Color color) const
        {
            switch (color)
            {
            case Black: return "b";
            case Red: return "r";
            default: return "";
            }
        }
    };
}


