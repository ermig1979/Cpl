/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
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

        String GenerateHtml(size_t indent = 0, bool firstTime = true, bool sortable = false, bool ignoreAlignment = false)
        {
            std::stringstream stream;
            Html html(stream, indent);

            if (firstTime)
            {
                html.WriteBegin("style", Html::Attr("type", "text/css"), true, false);
                SetSimpleStype(html);
                if (sortable)
                    SetSortableStype(html);
                html.WriteEnd("style", true, true);
                if (sortable)
                {
                    html.WriteBegin("script", Html::Attr("language", "JavaScript", "type", "text/javascript"), true, false);
                    SetSortableScript(html);
                    html.WriteEnd("script", true, true);
                }
            }

            Html::Attributes attributes;
            if(sortable)
                attributes.push_back(Html::Attribute("class", "sortable"));
            attributes.push_back(Html::Attribute("cellpadding", "2"));
            attributes.push_back(Html::Attribute("cellspacing", "0"));
            attributes.push_back(Html::Attribute("border", "1"));
            attributes.push_back(Html::Attribute("cellpadding", "2"));
            attributes.push_back(Html::Attribute("width", "100%"));
            attributes.push_back(Html::Attribute("style", "border-collapse:collapse"));
            html.WriteBegin("table", attributes, true, true);

            html.WriteBegin("thead", Html::Attr(), true, false);
            html.WriteBegin("tr", Html::Attr("style", "background-color:#e0e0e0; font-weight:bold;"), false, sortable);
            for (size_t col = 0; col < _width; ++col)
            {
                const Header& h = _headers[col];
                if (sortable)
                {
                    html.WriteBegin("th", Html::Attr("class", String(h.separator ? "sep" : "non") + (IsNum(col) ? " num" : "")), true, false);
                    html.WriteBegin("button", Html::Attr("class", AlignmentClass(h.alignment, true, ignoreAlignment)), true, false);
                    html.WriteText(h.name, false, false, true);
                    html.WriteValue("span", Html::Attr("aria-hidden", "true"), "", false);
                    html.WriteEnd("button", true, false);
                    html.WriteEnd("th", true, true);
                }
                else
                    html.WriteValue("th", Html::Attr("class", AlignmentClass(h.alignment, false, ignoreAlignment) + (h.separator ? " sep" : " non")), h.name, false);
            }
            html.WriteEnd("tr", true, false);
            html.WriteEnd("thead", false, true);

            html.WriteBegin("tbody", Html::Attr(), true, true);
            for (size_t row = 0; row < _height; ++row)
            {
                std::stringstream style;
                if (_rows[row].bold)
                    style << "font-weight: bold; background-color:#f0f0f0";
                html.WriteBegin("tr", Html::Attr("style", style.str()), true, false);
                for (size_t col = 0; col < _width; ++col)
                {
                    const Header& h = _headers[col];
                    const Cell& c = _cells[row * _width + col];
                    String classes = AlignmentClass(h.alignment, sortable, ignoreAlignment) + (h.separator ? " sep" : " non") + (c.color == Black ? " blk" : " red");
                    html.WriteBegin("td", Html::Attr("class", classes), false, false);
                    if(c.link.size())
                        html.WriteBegin("a", Html::Attr("href", c.link), false, false);
                    html.WriteText(c.value, false, false);
                    if (c.link.size())
                        html.WriteEnd("a", false, false);
                    html.WriteEnd("td", false, false);
                }
                html.WriteEnd("tr", true, true);
            }
            html.WriteEnd("tbody", true, true);

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

        String AlignmentClass(Alignment alignment, bool sortable, bool ignore) const
        {
            if (ignore)
                return "cnt";
            else if (sortable || alignment != Right)
                return alignment == Left ? "lft" : "cnt";
            else
                return "rgt";
        }

        bool IsNum(size_t col) const
        {
            for (size_t row = 0; row < _height; ++row)
            {
                std::istringstream iss(_cells[row * _width + col].value);
                double value;
                iss >> std::noskipws >> value;
                if (!iss.eof() || iss.fail())
                    return false;
            }
            return true;
        }

        static void SetSimpleStype(Html& html)
        {
            static const char* style = R"simple_style(
th { border-left: 0px; border-top: 0px; border-bottom: 1px solid #000000;}
td { border-left: 0px; border-top: 0px; border-bottom: 0px solid #000000;}
td.blk { color:black; }
td.red { color:red; }
td.lft, th.lft { text-align: left; }
td.cnt, th.cnt { text-align: center; }
td.rgt, th.rgt { text-align: right; }
td.non, th.non { border-right: 0px solid #000000; }
td.sep, th.sep { border-right: 1px solid #000000; }
)simple_style";
            html.WriteText(style, false, false, false);
        }

        static void SetSortableStype(Html& html)
        {
            static const char* style = R"sortable_style(
table.sortable th button.lft { text-align: left; }
table.sortable th button.cnt { text-align: center; }
table.sortable th { position: relative; }
table.sortable th.no-sort { padding-top: 0.35em;}
table.sortable th button { padding: 2px; font-size: 100%; font-weight: bold; background: transparent; border: none; display: inline; right: 0; left: 0; top: 0; bottom: 0; width: 100%; outline: none; cursor: pointer;}
table.sortable th button span { position: absolute; right: 4px;}
table.sortable th[aria-sort="descending"] span::after { content: '\25BC'; color: currentcolor; font-size: 100%; top: 0;}
table.sortable th[aria-sort="ascending"] span::after { content: '\25B2'; color: currentcolor; font-size: 100%; top: 0; }
table.show-unsorted-icon th:not([aria-sort]) button span::after { content: '\25AD'; color: currentcolor; font-size: 100%; position: relative; top: -3px; left: -4px;}
table.sortable th button:focus, table.sortable th button:hover { padding: 2px; border: 0px solid currentcolor; background-color: #f7f7f7;}
table.sortable th button:focus span, table.sortable th button:hover span {right: 2px;}
table.sortable th:not([aria-sort]) button:focus span::after, table.sortable th:not([aria-sort]) button:hover span::after { content: '\25BC'; color: currentcolor; font-size: 100%; top: 0;}
)sortable_style";
            html.WriteText(style, false, false, false);
        }

        static void SetSortableScript(Html& html)
        {
            static const char * script = R"sortable_script(
class SortableTable {
  constructor(tableNode) {
    this.tableNode = tableNode;
    this.columnHeaders = tableNode.querySelectorAll('thead th');
    this.sortColumns = [];
    for (var i = 0; i < this.columnHeaders.length; i++) {
      var ch = this.columnHeaders[i];
      var buttonNode = ch.querySelector('button');
      if (buttonNode) {
        this.sortColumns.push(i);
        buttonNode.setAttribute('data-column-index', i);
        buttonNode.addEventListener('click', this.handleClick.bind(this));
      }
    }
    this.optionCheckbox = document.querySelector('input[type="checkbox"][value="show-unsorted-icon"]');
    if (this.optionCheckbox) {
      this.optionCheckbox.addEventListener('change', this.handleOptionChange.bind(this));
      if (this.optionCheckbox.checked)
        this.tableNode.classList.add('show-unsorted-icon');
    }
  }

  setColumnHeaderSort(columnIndex) {
    if (typeof columnIndex === 'string')
      columnIndex = parseInt(columnIndex);
    for (var i = 0; i < this.columnHeaders.length; i++) {
      var ch = this.columnHeaders[i];
      var buttonNode = ch.querySelector('button');
      if (i === columnIndex) {
        var value = ch.getAttribute('aria-sort');
        if (value === 'descending') {
          ch.setAttribute('aria-sort', 'ascending');
          this.sortColumn(columnIndex, 'ascending', ch.classList.contains('num'));
        } else {
          ch.setAttribute('aria-sort', 'descending');
          this.sortColumn(columnIndex, 'descending', ch.classList.contains('num'));
        }
      } else {
        if (ch.hasAttribute('aria-sort') && buttonNode)
          ch.removeAttribute('aria-sort');
      }
    }
  }

  sortColumn(columnIndex, sortValue, isNumber) {
    function compareValues(a, b) {
      if (sortValue === 'ascending') {
        if (a.value === b.value)
          return 0;
        else {
          if (isNumber)
            return a.value - b.value;
          else
            return a.value < b.value ? -1 : 1;
        }
      } else {
        if (a.value === b.value)
          return 0;
        else {
          if (isNumber)
            return b.value - a.value;
          else
            return a.value > b.value ? -1 : 1;
        }
      }
    }
    if (typeof isNumber !== 'boolean') 
      isNumber = false;
    var tbodyNode = this.tableNode.querySelector('tbody');
    var rowNodes = [];
    var dataCells = [];
    var rowNode = tbodyNode.firstElementChild;
    var index = 0;
    while (rowNode) {
      rowNodes.push(rowNode);
      var rowCells = rowNode.querySelectorAll('th, td');
      var dataCell = rowCells[columnIndex];
      var data = {};
      data.index = index;
      data.value = dataCell.textContent.toLowerCase().trim();
      if (isNumber)
        data.value = parseFloat(data.value);
      dataCells.push(data);
      rowNode = rowNode.nextElementSibling;
      index += 1;
    }
    dataCells.sort(compareValues);
    while (tbodyNode.firstChild)
      tbodyNode.removeChild(tbodyNode.lastChild);
    for (var i = 0; i < dataCells.length; i += 1)
      tbodyNode.appendChild(rowNodes[dataCells[i].index]);
  }

  handleClick(event) {
    var tgt = event.currentTarget;
    this.setColumnHeaderSort(tgt.getAttribute('data-column-index'));
  }

  handleOptionChange(event) {
    var tgt = event.currentTarget;
      if (tgt.checked)
        this.tableNode.classList.add('show-unsorted-icon');
      else
        this.tableNode.classList.remove('show-unsorted-icon');
  }
}

window.addEventListener('load', function() {
  var sortableTables = document.querySelectorAll('table.sortable');
  for (var i = 0; i < sortableTables.length; i++)
    new SortableTable(sortableTables[i]);
});
)sortable_script";
            html.WriteText(script, false, false, false);
        }
    };
}


