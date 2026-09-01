/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
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

#pragma once

#include "Cpl/Defs.h"

namespace Cpl
{
    /*! @ingroup cpl_html
    * \struct Html
    * \brief Incremental HTML writer that emits tags, attributes and text to an output stream.
    * \note Indentation uses two spaces per level. Text may be HTML-escaped ("shielded") so that
    *       ampersand, less-than, greater-than, quote and apostrophe become character entities.
    *       Values of attributes are written as-is.
    *       The indent flag raises or lowers the current indent level around nested tags.
    *       The line flag appends a newline after the written markup and records that the next write starts a new line.
    */
    struct Html
    {
        /*!
        * \struct Attribute
        * \brief A single HTML attribute as a name/value pair.
        */
        struct Attribute
        {
            String name;  //!< HTML attribute name, for example "class" or "href".
            String value; //!< HTML attribute value written in quotes after the name.

            /*!
            * \fn Attribute(const String& n = String(), const String& v = String())
            * \brief Constructs an attribute with the given name and value.
            * \param [in] n - HTML attribute name. Empty by default.
            * \param [in] v - HTML attribute value. Empty by default.
            */
            Attribute(const String& n = String(), const String& v = String())
                : name(n)
                , value(v)
            {
            }
        };

        /*!
        * \typedef Attributes
        * \brief Ordered list of HTML attributes passed to tag writers.
        */
        typedef std::vector<Attribute> Attributes;

        /*!
        * \fn Attributes Attr()
        * \brief Returns an empty attribute list.
        * \return Empty Attributes vector.
        */
        static Attributes Attr()
        {
            return Attributes();
        }

        /*!
        * \fn Attributes Attr(const String& name0, const String& value0)
        * \brief Returns an attribute list with one name/value pair.
        * \param [in] name0 - Name of the attribute.
        * \param [in] value0 - Value of the attribute.
        * \return Attributes vector with one element.
        */
        static Attributes Attr(
            const String& name0, const String& value0)
        {
            Attributes attrbutes;
            attrbutes.push_back(Attribute(name0, value0));
            return attrbutes;
        }

        /*!
        * \fn Attributes Attr(const String& name0, const String& value0, const String& name1, const String& value1)
        * \brief Returns an attribute list with two name/value pairs, in the given order.
        * \param [in] name0 - Name of the first attribute.
        * \param [in] value0 - Value of the first attribute.
        * \param [in] name1 - Name of the second attribute.
        * \param [in] value1 - Value of the second attribute.
        * \return Attributes vector with two elements.
        */
        static Attributes Attr(
            const String& name0, const String& value0,
            const String& name1, const String& value1)
        {
            Attributes attrbutes;
            attrbutes.push_back(Attribute(name0, value0));
            attrbutes.push_back(Attribute(name1, value1));
            return attrbutes;
        }

        /*!
        * \fn Attributes Attr(const String& name0, const String& value0, const String& name1, const String& value1, const String& name2, const String& value2)
        * \brief Returns an attribute list with three name/value pairs, in the given order.
        * \param [in] name0 - Name of the first attribute.
        * \param [in] value0 - Value of the first attribute.
        * \param [in] name1 - Name of the second attribute.
        * \param [in] value1 - Value of the second attribute.
        * \param [in] name2 - Name of the third attribute.
        * \param [in] value2 - Value of the third attribute.
        * \return Attributes vector with three elements.
        */
        static Attributes Attr(
            const String& name0, const String& value0,
            const String& name1, const String& value1,
            const String& name2, const String& value2)
        {
            Attributes attrbutes;
            attrbutes.push_back(Attribute(name0, value0));
            attrbutes.push_back(Attribute(name1, value1));
            attrbutes.push_back(Attribute(name2, value2));
            return attrbutes;
        }

        /*!
        * \fn Html(std::ostream& stream, size_t indent = 0)
        * \brief Constructs a writer that emits HTML to the given stream.
        * \param [in] stream - Destination stream. Must outlive this Html instance.
        * \param [in] indent - Initial indent level. Each level is two spaces.
        */
        Html(std::ostream& stream, size_t indent = 0)
            : _stream(stream)
            , _indent(indent)
            , _line(true)
        {
        }

        /*!
        * \fn void WriteIndent()
        * \brief Writes two spaces for each current indent level to the stream.
        */
        void WriteIndent()
        {
            static const String INDENT = "  ";
            for (size_t i = 0; i < _indent; ++i)
                _stream << INDENT;
        }

        /*!
        * \fn void WriteAtribute(const Attribute& attribute)
        * \brief Writes a single attribute as a leading space, the name, '=', and the quoted value.
        * \param [in] attribute - Name and value to write. The value is not HTML-escaped.
        */
        void WriteAtribute(const Attribute& attribute)
        {
            _stream << " " << attribute.name << "=\"" << attribute.value << "\"";
        }

        /*!
        * \fn void WriteBegin(const String& name, const Attributes& attributes, bool indent, bool line)
        * \brief Writes an opening HTML tag for the given name, including attributes.
        * \param [in] name - Tag name, for example "table" or "div".
        * \param [in] attributes - Attributes written inside the opening tag.
        * \param [in] indent - If true, increase the indent level after writing the tag.
        * \param [in] line - If true, append a newline after the tag and treat the next write as starting a new line.
        */
        void WriteBegin(const String& name, const Attributes& attributes, bool indent, bool line)
        {
            if (_line)
                WriteIndent();
            _stream << "<" << name;
            for (size_t i = 0; i < attributes.size(); ++i)
                WriteAtribute(attributes[i]);
            _stream << ">";
            if (line)
                _stream << std::endl;
            if (indent)
                _indent++;
            _line = line;
        }

        /*!
        * \fn void WriteEnd(const String& name, bool indent, bool line)
        * \brief Writes a closing HTML tag for the given name.
        * \param [in] name - Tag name that matches the corresponding WriteBegin call.
        * \param [in] indent - If true, decrease the indent level before writing the tag.
        *                     If the previous write ended a line, the closing tag is indented.
        * \param [in] line - If true, append a newline after the tag and treat the next write as starting a new line.
        */
        void WriteEnd(const String& name, bool indent, bool line)
        {
            if (indent)
            {
                _indent--;
                if (_line)
                    WriteIndent();
            }
            _stream << "</" << name << ">";
            if (line)
                _stream << std::endl;
            _line = line;
        }

        /*!
        * \fn void WriteValue(const String& name, const Attributes& attributes, const String& value, bool line, bool shielding = true)
        * \brief Writes a complete HTML element (opening tag, content and closing tag) on the current line.
        * \param [in] name - Tag name.
        * \param [in] attributes - Attributes written in the opening tag.
        * \param [in] value - Element content written between the opening and closing tags.
        * \param [in] line - If true, append a newline after the closing tag.
        * \param [in] shielding - If true, HTML-escape ampersand, less-than, greater-than, quote and apostrophe in value.
        *                        If false, write value unchanged.
        */
        void WriteValue(const String& name, const Attributes& attributes, const String& value, bool line, bool shielding = true)
        {
            WriteBegin(name, attributes, false, false);
            if (shielding)
                WriteWithShielding(value);
            else
                _stream << value;
            WriteEnd(name, false, line);
        }

        /*!
        * \fn void WriteText(const String& text, bool indent, bool line, bool shielding = true)
        * \brief Writes text content without wrapping it in a tag.
        * \param [in] text - Text to write.
        * \param [in] indent - If true and the previous write ended a line, write the current indent before the text.
        * \param [in] line - If true, append a newline after the text and treat the next write as starting a new line.
        * \param [in] shielding - If true, HTML-escape ampersand, less-than, greater-than, quote and apostrophe in text.
        *                        If false, write text unchanged (for example CSS or JavaScript).
        */
        void WriteText(const String& text, bool indent, bool line, bool shielding = true)
        {
            if (indent && _line)
                WriteIndent();
            if (shielding)
                WriteWithShielding(text);
            else
                _stream << text;
            if (line)
                _stream << std::endl;
            _line = line;
        }

        /*!
        * \fn size_t Indent() const
        * \brief Returns the current indent level.
        * \return Number of indent levels. Each level is two spaces.
        */
        size_t Indent() const
        {
            return _indent;
        }

    private:
        std::ostream& _stream;
        size_t _indent;
        bool _line;

        void WriteWithShielding(const String& text, char ignore = 0)
        {
            for (size_t i = 0; i < text.size(); ++i)
            {
                if (text[i] == ignore)
                    _stream << text[i];
                else
                {
                    switch (text[i])
                    {
                    case '<': _stream << "&lt;"; break;
                    case '>': _stream << "&gt;"; break;
                    case '\'': _stream << "&apos;"; break;
                    case '"': _stream << "&quot;"; break;
                    case '&': _stream << "&amp;"; break;
                    default:
                        _stream << text[i];
                    }
                }
            }
        }
    };
}
