/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2026 Yermalayeu Ihar,
*               2021-2022 Andrey Drogolyub,
*               2023-2023 Daniil Germanenko.
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

#include <cstdlib>
#include <cassert>
#include <iterator>
#include <new>
#include <exception>

namespace Cpl
{
    /*! @ingroup cpl_xml
    * \namespace Cpl::Xml
    * \brief In-place XML DOM parser, printer and file loader.
    * \note Parse() writes into the source buffer unless non-destructive flags are set.
    *       Nodes, attributes and strings are allocated from MemoryPool and are freed only
    *       when the pool (or XmlDocument) is cleared or destroyed.
    */
    namespace Xml
    {
        /*! @ingroup cpl_xml
        * \brief Size in bytes of the first MemoryPool block, allocated as a member array.
        */
        const size_t STATIC_POOL_SIZE = 64 * 1024;

        /*! @ingroup cpl_xml
        * \brief Size in bytes of each additional MemoryPool block allocated on the heap.
        */
        const size_t DYNAMIC_POOL_SIZE = 64 * 1024;

        /*! @ingroup cpl_xml
        * \brief Alignment in bytes of MemoryPool allocations. Equal to sizeof(void*).
        */
        const size_t ALIGNMENT = sizeof(void*);

        /*! @ingroup cpl_xml
        * \class ParseError
        * \brief Exception thrown by XmlDocument::Parse when the source text is not well-formed XML.
        */
        class ParseError : public std::exception
        {
        public:
            /*!
            * \fn ParseError(const char * what, void * where)
            * \brief Stores the error description and a pointer into the source text.
            * \param [in] what - Description of the parse error. Not copied; must outlive this object.
            * \param [in] where - Position in the source text where the error was detected.
            */
            ParseError(const char * what, void * where)
                : _what(what)
                , _where(where)
            {
            }

            /*!
            * \fn const char * What() const
            * \brief Returns the error description given to the constructor.
            * \return Pointer to the description string.
            */
            virtual const char * What() const throw()
            {
                return _what;
            }

            /*!
            * \fn Ch * Where() const
            * \brief Returns the source-text position of the error, cast to character type Ch.
            * \tparam Ch - Character type of the parsed document.
            * \return Pointer into the source buffer passed to XmlDocument::Parse.
            */
            template<class Ch> Ch * Where() const
            {
                return (Ch*)_where;
            }

        private:
            const char * _what;
            void * _where;
        };

        template<class Ch> class XmlNode;
        template<class Ch> class XmlAttribute;
        template<class Ch> class XmlDocument;

        /*! @ingroup cpl_xml
        * \enum NodeType
        * \brief Kind of an XmlNode in the document tree.
        */
        enum NodeType
        {
            NodeDocument,    //!< Document root. XmlDocument has this type.
            NodeElement,     //!< Element node, for example <name>...</name>.
            NodeData,        //!< Text data node (character data between tags).
            NodeCData,       //!< CDATA section (<![CDATA[...]]>).
            NodeComment,     //!< Comment (<!-- ... -->).
            NodeDeclaration, //!< XML declaration (<?xml ... ?>).
            NodeDocType,     //!< Document type declaration (<!DOCTYPE ...>).
            NodePi           //!< Processing instruction (<?target ... ?>).
        };

        /*! @ingroup cpl_xml
        * \brief Do not create NodeData children. The first text fragment is stored as the
        *        element value unless ParseNoElementValues is also set.
        */
        const int ParseNoDataNodes = 0x1;

        /*! @ingroup cpl_xml
        * \brief Do not copy the first text fragment into the parent element value.
        */
        const int ParseNoElementValues = 0x2;

        /*! @ingroup cpl_xml
        * \brief Do not write a zero terminator after parsed names and values.
        *        NameSize() and ValueSize() must be used to read those strings.
        */
        const int ParseNoStringTerminators = 0x4;

        /*! @ingroup cpl_xml
        * \brief Do not expand XML entities such as &amp;, &lt; or numeric character references.
        */
        const int ParseNoEntityTranslation = 0x8;

        /*! @ingroup cpl_xml
        * \brief Treat the source as 8-bit characters and do not decode UTF-8 sequences
        *        when expanding numeric character references.
        */
        const int ParseNoUtf8 = 0x10;

        /*! @ingroup cpl_xml
        * \brief Create a NodeDeclaration child for an <?xml ... ?> declaration.
        */
        const int ParseDeclarationNode = 0x20;

        /*! @ingroup cpl_xml
        * \brief Create NodeComment children for <!-- ... --> comments.
        */
        const int ParseCommentNodes = 0x40;

        /*! @ingroup cpl_xml
        * \brief Create a NodeDocType child for a <!DOCTYPE ...> declaration.
        */
        const int ParseDocTypeNode = 0x80;

        /*! @ingroup cpl_xml
        * \brief Create NodePi children for processing instructions other than the XML declaration.
        */
        const int ParsePiNodes = 0x100;

        /*! @ingroup cpl_xml
        * \brief Compare closing-tag names with the corresponding opening-tag names
        *        and throw ParseError when they differ.
        */
        const int ParseValidateClosingTags = 0x200;

        /*! @ingroup cpl_xml
        * \brief Strip leading and trailing whitespace from parsed text data.
        */
        const int ParseTrimWhitespace = 0x400;

        /*! @ingroup cpl_xml
        * \brief Collapse each run of whitespace in text data to a single space character.
        *        Modifies the source buffer.
        */
        const int ParseNormalizeWhitespace = 0x800;

        /*! @ingroup cpl_xml
        * \brief Default Parse() flags. All bits are off.
        */
        const int ParseDefault = 0;

        /*! @ingroup cpl_xml
        * \brief Parse flags that leave the source text unchanged:
        *        ParseNoStringTerminators | ParseNoEntityTranslation.
        */
        const int ParseNonDestructive = ParseNoStringTerminators | ParseNoEntityTranslation;

        /*! @ingroup cpl_xml
        * \brief Fastest Parse() flags: ParseNonDestructive | ParseNoDataNodes.
        */
        const int ParseFasTest = ParseNonDestructive | ParseNoDataNodes;

        /*! @ingroup cpl_xml
        * \brief Parse flags that keep declaration, comment, DOCTYPE and PI nodes
        *        and validate closing-tag names.
        */
        const int ParseFull = ParseDeclarationNode | ParseCommentNodes | ParseDocTypeNode | ParsePiNodes | ParseValidateClosingTags;

        namespace Internal
        {
            template<class Ch> inline size_t Measure(const Ch * p)
            {
                const Ch * tmp = p;
                while (*tmp)
                    ++tmp;
                return tmp - p;
            }

            template<class Ch> inline size_t MeasureLimit(const Ch * p, size_t limit = std::numeric_limits<size_t>::max())
            {
                const Ch * tmp = p;
                while ((size_t(tmp - p) < limit) && *tmp)
                    ++tmp;
                return tmp - p;
            }


            template<class Ch> inline bool Compare(const Ch *p1, size_t size1, const Ch * p2, size_t size2, bool caseSensitive)
            {
                static const unsigned char upcase[256] =
                {
                    // 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A   B   C   D   E   F
                    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,   // 0
                    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,   // 1
                    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,   // 2
                    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,   // 3
                    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   // 4
                    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,   // 5
                    96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,   // 6
                    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123,124,125,126,127,  // 7
                    128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,  // 8
                    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,  // 9
                    160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,  // A
                    176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,  // B
                    192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,  // C
                    208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,  // D
                    224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,  // E
                    240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255   // F
                };

                if (size1 != size2)
                    return false;
                if (caseSensitive)
                {
                    for (const Ch *end = p1 + size1; p1 < end; ++p1, ++p2)
                        if (*p1 != *p2)
                            return false;
                }
                else
                {
                    for (const Ch *end = p1 + size1; p1 < end; ++p1, ++p2)
                        if (upcase[static_cast<unsigned char>(*p1)] != upcase[static_cast<unsigned char>(*p2)])
                            return false;
                }
                return true;
            }
        }

        /*! @ingroup cpl_xml
        * \class MemoryPool
        * \brief Aligned allocator used by XmlDocument for nodes, attributes and strings.
        * \tparam Ch - Character type of the stored names and values. Default is char.
        * \note The first STATIC_POOL_SIZE bytes are a member array. Further blocks of
        *       DYNAMIC_POOL_SIZE (or larger) are allocated on the heap. Individual objects
        *       are not freed; call Clear() or destroy the pool to release all blocks.
        */
        template<class Ch = char> class MemoryPool
        {
        public:
            /*!
            * \typedef AllocFunc
            * \brief Function that allocates size bytes and returns the block, or NULL on failure.
            */
            typedef void *(AllocFunc)(size_t);

            /*!
            * \typedef FreeFunc
            * \brief Function that releases a block previously returned by AllocFunc.
            */
            typedef void (FreeFunc)(void *);

            /*!
            * \fn MemoryPool()
            * \brief Constructs an empty pool that uses new[] and delete[] until SetAllocator is called.
            */
            MemoryPool()
                : _allocFunc(0)
                , _freeFunc(0)
            {
                Init();
            }

            /*!
            * \fn ~MemoryPool()
            * \brief Releases every heap block owned by the pool.
            */
            ~MemoryPool()
            {
                Clear();
            }

            /*!
            * \fn XmlNode<Ch> * AllocateNode(NodeType type, const Ch *name = 0, const Ch *value = 0, size_t nameSize = 0, size_t valueSize = 0)
            * \brief Allocates an XmlNode from the pool and optionally sets its name and value.
            * \param [in] type - Node kind stored in XmlNode::Type().
            * \param [in] name - Name pointer stored in the node, or NULL to leave the name empty.
            *                    The characters are not copied.
            * \param [in] value - Value pointer stored in the node, or NULL to leave the value empty.
            *                     The characters are not copied.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] valueSize - Number of characters in value. 0 measures a zero-terminated value.
            * \return Newly constructed node. Owned by this pool.
            */
            XmlNode<Ch> * AllocateNode(NodeType type, const Ch *name = 0, const Ch *value = 0, 
                size_t nameSize = 0, size_t valueSize = 0)
            {
                void * memory = AllocateAligned(sizeof(XmlNode<Ch>));
                XmlNode<Ch> *node = new(memory) XmlNode<Ch>(type);
                if (name)
                {
                    if (nameSize > 0)
                        node->Name(name, nameSize);
                    else
                        node->Name(name);
                }
                if (value)
                {
                    if (valueSize > 0)
                        node->Value(value, valueSize);
                    else
                        node->Value(value);
                }
                return node;
            }

            /*!
            * \fn XmlAttribute<Ch> * AllocateAttribute(const Ch *name = 0, const Ch *value = 0, size_t nameSize = 0, size_t valueSize = 0)
            * \brief Allocates an XmlAttribute from the pool and optionally sets its name and value.
            * \param [in] name - Name pointer stored in the attribute, or NULL to leave the name empty.
            *                    The characters are not copied.
            * \param [in] value - Value pointer stored in the attribute, or NULL to leave the value empty.
            *                     The characters are not copied.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] valueSize - Number of characters in value. 0 measures a zero-terminated value.
            * \return Newly constructed attribute. Owned by this pool.
            */
            XmlAttribute<Ch> * AllocateAttribute(const Ch *name = 0, const Ch *value = 0,
                size_t nameSize = 0, size_t valueSize = 0)
            {
                void * memory = AllocateAligned(sizeof(XmlAttribute<Ch>));
                XmlAttribute<Ch> *attribute = new(memory) XmlAttribute<Ch>;
                if (name)
                {
                    if (nameSize > 0)
                        attribute->Name(name, nameSize);
                    else
                        attribute->Name(name);
                }
                if (value)
                {
                    if (valueSize > 0)
                        attribute->Value(value, valueSize);
                    else
                        attribute->Value(value);
                }
                return attribute;
            }

            /*!
            * \fn Ch * AllocateString(const Ch *source = 0, size_t size = 0)
            * \brief Allocates a zero-terminated string from the pool.
            * \param [in] source - Characters to copy, or NULL to allocate an uninitialized buffer
            *                     of size characters plus a terminator. At least one of source or
            *                     size must be non-zero.
            * \param [in] size - Maximum number of characters to copy from source, or the length
            *                    of an uninitialized buffer when source is NULL. 0 copies source
            *                    up to its zero terminator.
            * \return Pointer to size+1 (or measured length+1) characters. The last character is 0.
            *         Owned by this pool.
            */
#if __cplusplus >= 201703L
            [[nodiscard]] 
#endif
            Ch * AllocateString(const Ch *source = 0, size_t size = 0)
            {
                assert(source || size);

                size_t data_size = 0;

                if (source == nullptr)
                    data_size = size;
                else 
                    data_size = (size == 0) ? Internal::Measure(source) : Internal::MeasureLimit(source, size);

                Ch *result = static_cast<Ch *>(AllocateAligned((data_size + 1) * sizeof(Ch)));
                
                if (source != nullptr)
                    for (size_t i = 0; i < data_size; ++i)
                        result[i] = source[i];

                result[data_size] = 0;

                return result;
            }

            /*!
            * \fn XmlNode<Ch> * CloneNode(const XmlNode<Ch> *source, XmlNode<Ch> *result = 0)
            * \brief Deep-copies source and its descendants into this pool.
            * \param [in] source - Node to clone. Name and value pointers are copied, not the characters.
            * \param [in] result - Existing node to overwrite, or NULL to allocate a new node.
            *                      When non-NULL, its children and attributes are removed first.
            * \return result, or a newly allocated node when result is NULL. Owned by this pool.
            */
            XmlNode<Ch> * CloneNode(const XmlNode<Ch> *source, XmlNode<Ch> *result = 0)
            {
                if (result)
                {
                    result->RemoveAllAttributes();
                    result->RemoveAllNodes();
                    result->Type(source->Type());
                }
                else
                    result = AllocateNode(source->Type());
                result->Name(source->Name(), source->NameSize());
                result->Value(source->Value(), source->ValueSize());
                for (XmlNode<Ch> *child = source->FirstNode(); child; child = child->NextSibling())
                    result->AppendNode(CloneNode(child));
                for (XmlAttribute<Ch> *attr = source->FirstAttribute(); attr; attr = attr->NextAttribute())
                    result->AppendAttribute(AllocateAttribute(attr->Name(), attr->Value(), attr->NameSize(), attr->ValueSize()));
                return result;
            }

            /*!
            * \fn void Clear()
            * \brief Frees every heap block and resets the pool to the static member array.
            * \note All nodes, attributes and strings previously returned by this pool become invalid.
            */
            void Clear()
            {
                while (_begin != _staticMemory)
                {
                    char *prevBegin = reinterpret_cast<Header*>(Align(_begin))->prevBegin;
                    if (_freeFunc)
                        _freeFunc(_begin);
                    else
                        delete[] _begin;
                    _begin = prevBegin;
                }
                Init();
            }

            /*!
            * \fn void SetAllocator(AllocFunc *af, FreeFunc *ff)
            * \brief Installs custom heap allocation functions used for further dynamic blocks.
            * \param [in] af - Allocator, or NULL to use new char[size].
            * \param [in] ff - Deallocator matching af, or NULL to use delete[].
            * \note Must be called while the pool still uses only the static member array.
            */
            void SetAllocator(AllocFunc *af, FreeFunc *ff)
            {
                assert(_begin == _staticMemory && _ptr == Align(_begin));
                _allocFunc = af;
                _freeFunc = ff;
            }

        private:
            struct Header
            {
                char * prevBegin;
            };

            void Init()
            {
                _begin = _staticMemory;
                _ptr = Align(_begin);
                _end = _staticMemory + sizeof(_staticMemory);
            }

            char * Align(char *ptr)
            {
                size_t alignment = ((ALIGNMENT - (size_t(ptr) & (ALIGNMENT - 1))) & (ALIGNMENT - 1));
                return ptr + alignment;
            }

            char * AllocateRaw(size_t size)
            {
                void *memory;
                if (_allocFunc)
                {
                    memory = _allocFunc(size);
                    assert(memory);
                }
                else
                {
                    memory = new char[size];
                }
                return static_cast<char *>(memory);
            }

            void * AllocateAligned(size_t size)
            {
                char * result = Align(_ptr);
                if (result + size > _end)
                {
                    size_t pool_size = DYNAMIC_POOL_SIZE;
                    if (pool_size < size)
                        pool_size = size;

                    size_t allocSize = sizeof(Header) + (2 * ALIGNMENT - 2) + pool_size;
                    char * rawMemory = AllocateRaw(allocSize);

                    char *pool = Align(rawMemory);
                    Header *newHeader = reinterpret_cast<Header *>(pool);
                    newHeader->prevBegin = _begin;
                    _begin = rawMemory;
                    _ptr = pool + sizeof(Header);
                    _end = rawMemory + allocSize;

                    result = Align(_ptr);
                }
                _ptr = result + size;
                return result;
            }

            char * _begin;
            char * _ptr; 
            char * _end;
            char _staticMemory[STATIC_POOL_SIZE];
            AllocFunc *_allocFunc; 
            FreeFunc *_freeFunc;
        };

        /*! @ingroup cpl_xml
        * \class XmlBase
        * \brief Shared name, value and parent pointer of XmlNode and XmlAttribute.
        * \tparam Ch - Character type of the name and value. Default is char.
        * \note Name() and Value() store pointers; they do not copy characters.
        *       The pointed-to data must outlive this object. When no name or value is set,
        *       Name() and Value() return a static empty string and the size accessors return 0.
        */
        template<class Ch = char> class XmlBase
        {
        public:
            /*!
            * \fn XmlBase()
            * \brief Constructs an object with an empty name, empty value and no parent.
            */
            XmlBase()
                : _name(0)
                , _value(0)
                , _parent(0)
            {
            }

            /*!
            * \fn Ch * Name() const
            * \brief Returns the name pointer, or a static empty string when no name is set.
            * \return Zero-terminated name when ParseNoStringTerminators is off. Otherwise
            *         use NameSize() to determine the length.
            */
            Ch * Name() const
            {
                return _name ? _name : NullStr();
            }

            /*!
            * \fn size_t NameSize() const
            * \brief Returns the number of characters in the name.
            * \return Stored name length, or 0 when no name is set.
            */
            size_t NameSize() const
            {
                return _name ? _nameSize : 0;
            }

            /*!
            * \fn Ch * Value() const
            * \brief Returns the value pointer, or a static empty string when no value is set.
            * \return Zero-terminated value when ParseNoStringTerminators is off. Otherwise
            *         use ValueSize() to determine the length.
            */
            Ch * Value() const
            {
                return _value ? _value : NullStr();
            }

            /*!
            * \fn size_t ValueSize() const
            * \brief Returns the number of characters in the value.
            * \return Stored value length, or 0 when no value is set.
            */
            size_t ValueSize() const
            {
                return _value ? _valueSize : 0;
            }

            /*!
            * \fn void Name(const Ch *name, size_t size)
            * \brief Stores a name pointer and length without copying characters.
            * \param [in] name - Name characters. Must outlive this object.
            * \param [in] size - Number of characters in name.
            */
            void Name(const Ch *name, size_t size)
            {
                _name = const_cast<Ch *>(name);
                _nameSize = size;
            }

            /*!
            * \fn void Name(const Ch *name)
            * \brief Stores a zero-terminated name pointer. The length is the number of characters before the terminator.
            * \param [in] name - Zero-terminated name. Must outlive this object.
            */
            void Name(const Ch *name)
            {
                this->Name(name, Internal::Measure(name));
            }

            /*!
            * \fn void Value(const Ch *value, size_t size)
            * \brief Stores a value pointer and length without copying characters.
            * \param [in] value - Value characters. Must outlive this object.
            * \param [in] size - Number of characters in value.
            */
            void Value(const Ch *value, size_t size)
            {
                _value = const_cast<Ch *>(value);
                _valueSize = size;
            }

            /*!
            * \fn void Value(const Ch *value)
            * \brief Stores a zero-terminated value pointer. The length is the number of characters before the terminator.
            * \param [in] value - Zero-terminated value. Must outlive this object.
            */
            void Value(const Ch *value)
            {
                this->Value(value, Internal::Measure(value));
            }

            /*!
            * \fn XmlNode<Ch> * Parent() const
            * \brief Returns the parent node, or NULL when this object is not attached.
            * \return Parent XmlNode, or NULL.
            */
            XmlNode<Ch> * Parent() const
            {
                return _parent;
            }

        protected:
            static Ch * NullStr()
            {
                static Ch zero = Ch('\0');
                return &zero;
            }

            Ch * _name;
            Ch * _value; 
            size_t _nameSize;
            size_t _valueSize;
            XmlNode<Ch> * _parent;
        };

        /*! @ingroup cpl_xml
        * \class XmlAttribute
        * \brief Name/value pair attached to an XmlNode.
        * \tparam Ch - Character type of the name and value. Default is char.
        * \note Attributes form a doubly linked list. Allocate them from the document MemoryPool
        *       and attach them with XmlNode::AppendAttribute or a related method.
        */
        template<class Ch = char> class XmlAttribute : public XmlBase<Ch>
        {
            friend class XmlNode<Ch>;
        public:
            /*!
            * \fn XmlAttribute()
            * \brief Constructs an attribute with an empty name and value and no siblings.
            */
            XmlAttribute()
            {
            }

            /*!
            * \fn XmlDocument<Ch> * document() const
            * \brief Walks parent pointers to the document root.
            * \return The ancestor of type NodeDocument, or NULL when this attribute is detached
            *         or the root is not an XmlDocument.
            */
            XmlDocument<Ch> *document() const
            {
                if (XmlNode<Ch> *node = this->parent())
                {
                    while (node->parent())
                        node = node->parent();
                    return node->Type() == NodeDocument ? static_cast<XmlDocument<Ch> *>(node) : 0;
                }
                else
                    return 0;
            }

            /*!
            * \fn XmlAttribute<Ch> * PreviousAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the previous sibling attribute, optionally filtered by name.
            * \param [in] name - Attribute name to search for, or NULL to return the immediate previous sibling.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching previous attribute, the immediate previous sibling when name is NULL
            *         and this attribute has a parent, or NULL.
            */
            XmlAttribute<Ch> *PreviousAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlAttribute<Ch> *attribute = _prevAttribute; attribute; attribute = attribute->_prevAttribute)
                        if (Internal::Compare(attribute->Name(), attribute->NameSize(), name, nameSize, caseSensitive))
                            return attribute;
                    return 0;
                }
                else
                    return this->_parent ? _prevAttribute : 0;
            }

            /*!
            * \fn XmlAttribute<Ch> * NextAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the next sibling attribute, optionally filtered by name.
            * \param [in] name - Attribute name to search for, or NULL to return the immediate next sibling.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching next attribute, the immediate next sibling when name is NULL
            *         and this attribute has a parent, or NULL.
            */
            XmlAttribute<Ch> *NextAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlAttribute<Ch> *attribute = _nextAttribute; attribute; attribute = attribute->_nextAttribute)
                        if (Internal::Compare(attribute->Name(), attribute->NameSize(), name, nameSize, caseSensitive))
                            return attribute;
                    return 0;
                }
                else
                    return this->_parent ? _nextAttribute : 0;
            }

        private:
            XmlAttribute<Ch> *_prevAttribute;
            XmlAttribute<Ch> *_nextAttribute;
        };

        /*! @ingroup cpl_xml
        * \class XmlNode
        * \brief Node in the XML tree: document, element, data, CDATA, comment, declaration, DOCTYPE or PI.
        * \tparam Ch - Character type of the name and value. Default is char.
        * \note Children and attributes form doubly linked lists. Allocate nodes from the document
        *       MemoryPool. Removing a node unlinks it but does not free its memory.
        */
        template<class Ch = char>
        class XmlNode : public XmlBase<Ch>
        {
        public:
            /*!
            * \fn XmlNode(NodeType type)
            * \brief Constructs a node of the given type with no children, attributes or siblings.
            * \param [in] type - Node kind stored in Type().
            */
            XmlNode(NodeType type)
                : _type(type)
                , _firstNode(0)
                , _lastNode(0)
                , _prevSibling(0)
                , _nextSibling(0)
                , _lastAttribute(0)
                , _firstAttribute(0)
            {
            }

            /*!
            * \fn NodeType Type() const
            * \brief Returns the node kind given to the constructor or to Type(NodeType).
            * \return Current NodeType.
            */
            NodeType Type() const
            {
                return _type;
            }

            /*!
            * \fn XmlDocument<Ch> * Document() const
            * \brief Walks parent pointers to the document root.
            * \return The ancestor of type NodeDocument, or NULL when the root is not an XmlDocument.
            */
            XmlDocument<Ch> * Document() const
            {
                XmlNode<Ch> *node = const_cast<XmlNode<Ch> *>(this);
                while (node->parent())
                    node = node->parent();
                return node->Type() == NodeDocument ? static_cast<XmlDocument<Ch> *>(node) : 0;
            }

            /*!
            * \fn XmlNode<Ch> * FirstNode(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the first child node, optionally filtered by name.
            * \param [in] name - Child name to search for, or NULL to return the first child.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching child, the first child when name is NULL, or NULL.
            */
            XmlNode<Ch> * FirstNode(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlNode<Ch> *child = _firstNode; child; child = child->NextSibling())
                        if (Internal::Compare(child->Name(), child->NameSize(), name, nameSize, caseSensitive))
                            return child;
                    return 0;
                }
                else
                    return _firstNode;
            }

            /*!
            * \fn XmlNode<Ch> * LastNode(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the last child node, optionally filtered by name.
            * \param [in] name - Child name to search for, or NULL to return the last child.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching child, the last child when name is NULL, or NULL.
            * \note Asserts that this node has at least one child.
            */
            XmlNode<Ch> * LastNode(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                assert(_firstNode);
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlNode<Ch> *child = _lastNode; child; child = child->PreviousSibling())
                        if (Internal::Compare(child->Name(), child->NameSize(), name, nameSize, caseSensitive))
                            return child;
                    return 0;
                }
                else
                    return _lastNode;
            }

            /*!
            * \fn XmlNode<Ch> * PreviousSibling(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the previous sibling node, optionally filtered by name.
            * \param [in] name - Sibling name to search for, or NULL to return the immediate previous sibling.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching previous sibling, the immediate previous sibling when name is NULL, or NULL.
            * \note Asserts that this node has a parent.
            */
            XmlNode<Ch> * PreviousSibling(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                assert(this->_parent);
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlNode<Ch> *sibling = _prevSibling; sibling; sibling = sibling->_prevSibling)
                        if (Internal::Compare(sibling->Name(), sibling->NameSize(), name, nameSize, caseSensitive))
                            return sibling;
                    return 0;
                }
                else
                    return _prevSibling;
            }

            /*!
            * \fn XmlNode<Ch> * NextSibling(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the next sibling node, optionally filtered by name.
            * \param [in] name - Sibling name to search for, or NULL to return the immediate next sibling.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching next sibling, the immediate next sibling when name is NULL, or NULL.
            * \note Asserts that this node has a parent.
            */
            XmlNode<Ch> *NextSibling(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                assert(this->_parent);
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlNode<Ch> *sibling = _nextSibling; sibling; sibling = sibling->_nextSibling)
                        if (Internal::Compare(sibling->Name(), sibling->NameSize(), name, nameSize, caseSensitive))
                            return sibling;
                    return 0;
                }
                else
                    return _nextSibling;
            }

            /*!
            * \fn XmlAttribute<Ch> * FirstAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the first attribute, optionally filtered by name.
            * \param [in] name - Attribute name to search for, or NULL to return the first attribute.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching attribute, the first attribute when name is NULL, or NULL.
            */
            XmlAttribute<Ch> *FirstAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlAttribute<Ch> *attribute = _firstAttribute; attribute; attribute = attribute->_nextAttribute)
                        if (Internal::Compare(attribute->Name(), attribute->NameSize(), name, nameSize, caseSensitive))
                            return attribute;
                    return 0;
                }
                else
                    return _firstAttribute;
            }

            /*!
            * \fn XmlAttribute<Ch> * LastAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            * \brief Returns the last attribute, optionally filtered by name.
            * \param [in] name - Attribute name to search for, or NULL to return the last attribute.
            * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
            * \param [in] caseSensitive - If true, compare names with exact character values.
            *                             If false, compare ASCII letters case-insensitively.
            * \return Matching attribute, the last attribute when name is NULL and this node has
            *         at least one attribute, or NULL.
            */
            XmlAttribute<Ch> * LastAttribute(const Ch *name = 0, size_t nameSize = 0, bool caseSensitive = true) const
            {
                if (name)
                {
                    if (nameSize == 0)
                        nameSize = Internal::Measure(name);
                    for (XmlAttribute<Ch> *attribute = _lastAttribute; attribute; attribute = attribute->_prevAttribute)
                        if (Internal::Compare(attribute->Name(), attribute->NameSize(), name, nameSize, caseSensitive))
                            return attribute;
                    return 0;
                }
                else
                    return _firstAttribute ? _lastAttribute : 0;
            }

            /*!
            * \fn void Type(NodeType type)
            * \brief Changes the node kind.
            * \param [in] type - New NodeType.
            */
            void Type(NodeType type)
            {
                _type = type;
            }

            /*!
            * \fn void PrependNode(XmlNode<Ch> *child)
            * \brief Inserts child as the first child of this node.
            * \param [in] child - Node to insert. Must have no parent and must not be a document node.
            */
            void PrependNode(XmlNode<Ch> *child)
            {
                assert(child && !child->parent() && child->Type() != NodeDocument);
                if (FirstNode())
                {
                    child->_nextSibling = _firstNode;
                    _firstNode->_prevSibling = child;
                }
                else
                {
                    child->_nextSibling = 0;
                    _lastNode = child;
                }
                _firstNode = child;
                child->_parent = this;
                child->_prevSibling = 0;
            }

            /*!
            * \fn void AppendNode(XmlNode<Ch> *child)
            * \brief Inserts child as the last child of this node.
            * \param [in] child - Node to insert. Must have no parent and must not be a document node.
            */
            void AppendNode(XmlNode<Ch> *child)
            {
                assert(child && !child->Parent() && child->Type() != NodeDocument);
                if (FirstNode())
                {
                    child->_prevSibling = _lastNode;
                    _lastNode->_nextSibling = child;
                }
                else
                {
                    child->_prevSibling = 0;
                    _firstNode = child;
                }
                _lastNode = child;
                child->_parent = this;
                child->_nextSibling = 0;
            }

            /*!
            * \fn void InsertNode(XmlNode<Ch> *where, XmlNode<Ch> *child)
            * \brief Inserts child before where, or appends it when where is NULL.
            * \param [in] where - Existing child of this node to insert before, or NULL to append.
            * \param [in] child - Node to insert. Must have no parent and must not be a document node.
            */
            void InsertNode(XmlNode<Ch> *where, XmlNode<Ch> *child)
            {
                assert(!where || where->parent() == this);
                assert(child && !child->parent() && child->Type() != NodeDocument);
                if (where == _firstNode)
                    PrependNode(child);
                else if (where == 0)
                    AppendNode(child);
                else
                {
                    child->_prevSibling = where->_prevSibling;
                    child->_nextSibling = where;
                    where->_prevSibling->_nextSibling = child;
                    where->_prevSibling = child;
                    child->_parent = this;
                }
            }

            /*!
            * \fn void RemoveFirstNode()
            * \brief Unlinks the first child. The child's memory stays in the pool.
            * \note Asserts that this node has at least one child.
            */
            void RemoveFirstNode()
            {
                assert(FirstNode());
                XmlNode<Ch> *child = _firstNode;
                _firstNode = child->_nextSibling;
                if (child->_nextSibling)
                    child->_nextSibling->_prevSibling = 0;
                else
                    _lastNode = 0;
                child->_parent = 0;
            }

            /*!
            * \fn void RemoveLastNode()
            * \brief Unlinks the last child. The child's memory stays in the pool.
            * \note Asserts that this node has at least one child.
            */
            void RemoveLastNode()
            {
                assert(FirstNode());
                XmlNode<Ch> *child = _lastNode;
                if (child->_prevSibling)
                {
                    _lastNode = child->_prevSibling;
                    child->_prevSibling->_nextSibling = 0;
                }
                else
                    _firstNode = 0;
                child->_parent = 0;
            }

            /*!
            * \fn void RemoveNode(XmlNode<Ch> *where)
            * \brief Unlinks where from this node. The child's memory stays in the pool.
            * \param [in] where - Child of this node to unlink.
            */
            void RemoveNode(XmlNode<Ch> *where)
            {
                assert(where && where->parent() == this);
                assert(FirstNode());
                if (where == _firstNode)
                    RemoveFirstNode();
                else if (where == _lastNode)
                    RemoveLastNode();
                else
                {
                    where->_prevSibling->_nextSibling = where->_nextSibling;
                    where->_nextSibling->_prevSibling = where->_prevSibling;
                    where->_parent = 0;
                }
            }

            /*!
            * \fn void RemoveAllNodes()
            * \brief Unlinks every child. Child memory stays in the pool.
            */
            void RemoveAllNodes()
            {
                for (XmlNode<Ch> *node = FirstNode(); node; node = node->_nextSibling)
                    node->_parent = 0;
                _firstNode = 0;
            }

            /*!
            * \fn void PrependAttribute(XmlAttribute<Ch> *attribute)
            * \brief Inserts attribute as the first attribute of this node.
            * \param [in] attribute - Attribute to insert. Must have no parent.
            */
            void PrependAttribute(XmlAttribute<Ch> *attribute)
            {
                assert(attribute && !attribute->parent());
                if (FirstAttribute())
                {
                    attribute->_nextAttribute = _firstAttribute;
                    _firstAttribute->_prevAttribute = attribute;
                }
                else
                {
                    attribute->_nextAttribute = 0;
                    _lastAttribute = attribute;
                }
                _firstAttribute = attribute;
                attribute->_parent = this;
                attribute->_prevAttribute = 0;
            }

            /*!
            * \fn void AppendAttribute(XmlAttribute<Ch> *attribute)
            * \brief Inserts attribute as the last attribute of this node.
            * \param [in] attribute - Attribute to insert. Must have no parent.
            */
            void AppendAttribute(XmlAttribute<Ch> *attribute)
            {
                assert(attribute && !attribute->Parent());
                if (FirstAttribute())
                {
                    attribute->_prevAttribute = _lastAttribute;
                    _lastAttribute->_nextAttribute = attribute;
                }
                else
                {
                    attribute->_prevAttribute = 0;
                    _firstAttribute = attribute;
                }
                _lastAttribute = attribute;
                attribute->_parent = this;
                attribute->_nextAttribute = 0;
            }

            /*!
            * \fn void InsertAttribute(XmlAttribute<Ch> *where, XmlAttribute<Ch> *attribute)
            * \brief Inserts attribute before where, or appends it when where is NULL.
            * \param [in] where - Existing attribute of this node to insert before, or NULL to append.
            * \param [in] attribute - Attribute to insert. Must have no parent.
            */
            void InsertAttribute(XmlAttribute<Ch> *where, XmlAttribute<Ch> *attribute)
            {
                assert(!where || where->parent() == this);
                assert(attribute && !attribute->parent());
                if (where == _firstAttribute)
                    PrependAttribute(attribute);
                else if (where == 0)
                    AppendAttribute(attribute);
                else
                {
                    attribute->_prevAttribute = where->_prevAttribute;
                    attribute->_nextAttribute = where;
                    where->_prevAttribute->_nextAttribute = attribute;
                    where->_prevAttribute = attribute;
                    attribute->_parent = this;
                }
            }

            /*!
            * \fn void RemoveFirstAttribute()
            * \brief Unlinks the first attribute. The attribute's memory stays in the pool.
            * \note Asserts that this node has at least one attribute.
            */
            void RemoveFirstAttribute()
            {
                assert(FirstAttribute());
                XmlAttribute<Ch> *attribute = _firstAttribute;
                if (attribute->_nextAttribute)
                {
                    attribute->_nextAttribute->_prevAttribute = 0;
                }
                else
                    _lastAttribute = 0;
                attribute->_parent = 0;
                _firstAttribute = attribute->_nextAttribute;
            }

            /*!
            * \fn void RemoveLastAttribute()
            * \brief Unlinks the last attribute. The attribute's memory stays in the pool.
            * \note Asserts that this node has at least one attribute.
            */
            void RemoveLastAttribute()
            {
                assert(FirstAttribute());
                XmlAttribute<Ch> *attribute = _lastAttribute;
                if (attribute->_prevAttribute)
                {
                    attribute->_prevAttribute->_nextAttribute = 0;
                    _lastAttribute = attribute->_prevAttribute;
                }
                else
                    _firstAttribute = 0;
                attribute->_parent = 0;
            }

            /*!
            * \fn void RemoveAttribute(XmlAttribute<Ch> *where)
            * \brief Unlinks where from this node. The attribute's memory stays in the pool.
            * \param [in] where - Attribute of this node to unlink.
            */
            void RemoveAttribute(XmlAttribute<Ch> *where)
            {
                assert(FirstAttribute() && where->parent() == this);
                if (where == _firstAttribute)
                    RemoveFirstAttribute();
                else if (where == _lastAttribute)
                    RemoveLastAttribute();
                else
                {
                    where->_prevAttribute->_nextAttribute = where->_nextAttribute;
                    where->_nextAttribute->_prevAttribute = where->_prevAttribute;
                    where->_parent = 0;
                }
            }

            /*!
            * \fn void RemoveAllAttributes()
            * \brief Unlinks every attribute. Attribute memory stays in the pool.
            */
            void RemoveAllAttributes()
            {
                for (XmlAttribute<Ch> *attribute = FirstAttribute(); attribute; attribute = attribute->_nextAttribute)
                    attribute->_parent = 0;
                _firstAttribute = 0;
            }

        private:
            XmlNode(const XmlNode &);
            void operator =(const XmlNode &);

            NodeType _type;
            XmlNode<Ch> *_firstNode; 
            XmlNode<Ch> *_lastNode;
            XmlAttribute<Ch> *_firstAttribute; 
            XmlAttribute<Ch> *_lastAttribute; 
            XmlNode<Ch> *_prevSibling; 
            XmlNode<Ch> *_nextSibling;
        };

        /*! @ingroup cpl_xml
        * \class XmlDocument
        * \brief XML document: a NodeDocument root plus the MemoryPool that owns the tree.
        * \tparam Ch - Character type of the parsed text. Default is char.
        * \note Parse() writes into the source buffer unless non-destructive flags are set.
        *       The source buffer must remain valid for the lifetime of the document.
        */
        template<class Ch = char> class XmlDocument : public XmlNode<Ch>, public MemoryPool<Ch>
        {
        public:
            /*!
            * \fn XmlDocument()
            * \brief Constructs an empty document of type NodeDocument.
            */
            XmlDocument()
                : XmlNode<Ch>(NodeDocument)
            {
            }

            /*!
            * \fn void Parse(Ch * text, size_t length)
            * \brief Parses zero or more root-level nodes from text and appends them to this document.
            * \tparam Flags - Combination of Parse* flag constants. Use ParseDefault for the default behaviour.
            * \param [in,out] text - Source XML. Not copied. May be overwritten unless Flags includes
            *                       ParseNonDestructive (and does not include whitespace-normalizing flags).
            *                       Must be readable for length characters and should be zero-terminated.
            * \param [in] length - Number of characters in text, not counting a required terminator.
            * \note Existing children and attributes are removed first. Throws ParseError when the
            *       text is not well-formed XML or when a required '<' is missing.
            */
            template<int Flags> void Parse(Ch * text, size_t length)
            {
                assert(text);
                const Ch * startPos = text;
                this->RemoveAllNodes();
                this->RemoveAllAttributes();
                ParseBom<Flags>(text);
                while (length - size_t(text - startPos) && *text != 0)
                {
                    Skip<Whitespace, Flags>(text);
                    if (*text == Ch('<'))
                    {
                        ++text;
                        if (XmlNode<Ch> *node = ParseNode<Flags>(text))
                            this->AppendNode(node);
                    }
                    else if (*text != 0)
                        throw ParseError("expected <", text);
                }
            }

            /*!
            * \fn void Clear()
            * \brief Unlinks all children and attributes and releases every heap block of the pool.
            */
            void Clear()
            {
                this->RemoveAllNodes();
                this->RemoveAllAttributes();
                MemoryPool<Ch>::Clear();
            }

        private:

            struct Whitespace
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  1,  0,  0,  // 0
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 1
                        1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 2
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 3
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 4
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 5
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 6
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 7
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 8
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // 9
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // A
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // B
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // C
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // D
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  // E
                        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            struct NodeName
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            struct AttributeName
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            struct Text
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            struct TextPureNoWs
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            struct TextPureWithWs
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  0,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        0,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            template<Ch Quote> struct AttributeValue
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data1[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    static const unsigned char data2[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    if (Quote == Ch('\''))
                        return data1[static_cast<unsigned char>(ch)];
                    if (Quote == Ch('\"'))
                        return data2[static_cast<unsigned char>(ch)];
                    return 0;
                }
            };

            template<Ch Quote> struct AttributeValuePure
            {
                static unsigned char Test(Ch ch)
                {
                    static const unsigned char data1[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        1,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    static const unsigned char data2[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 0
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 1
                        1,  1,  0,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 2
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 3
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 4
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 5
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 6
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 7
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 8
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // 9
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // A
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // B
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // C
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // D
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  // E
                        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1   // F
                    };
                    if (Quote == Ch('\''))
                        return data1[static_cast<unsigned char>(ch)];
                    if (Quote == Ch('\"'))
                        return data2[static_cast<unsigned char>(ch)];
                    return 0;
                }
            };

            struct Digits
            {
                static unsigned char Digit(Ch ch)
                {
                    static const unsigned char data[256] =
                    {
                        // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 0
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 1
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 2
                          0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,  // 3
                        255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,  // 4
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 5
                        255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,  // 6
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 7
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 8
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // 9
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // A
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // B
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // C
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // D
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,  // E
                        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255   // F
                    };
                    return data[static_cast<unsigned char>(ch)];
                }
            };

            template<int Flags> static void InsertCodedCharacter(Ch *&text, unsigned long code)
            {
                if (Flags & ParseNoUtf8)
                {
                    text[0] = static_cast<unsigned char>(code);
                    text += 1;
                }
                else
                {
                    if (code < 0x80)
                    {
                        text[0] = static_cast<unsigned char>(code);
                        text += 1;
                    }
                    else if (code < 0x800)
                    {
                        text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
                        text[0] = static_cast<unsigned char>(code | 0xC0);
                        text += 2;
                    }
                    else if (code < 0x10000)
                    {
                        text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
                        text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
                        text[0] = static_cast<unsigned char>(code | 0xE0);
                        text += 3;
                    }
                    else if (code < 0x110000)
                    {
                        text[3] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
                        text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
                        text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
                        text[0] = static_cast<unsigned char>(code | 0xF0);
                        text += 4;
                    }
                    else
                    {
                        throw ParseError("invalid numeric character entity", text);
                    }
                }
            }

            template<class StopPred, int Flags> static void Skip(Ch *&text)
            {
                Ch *tmp = text;
                while (StopPred::Test(*tmp))
                    ++tmp;
                text = tmp;
            }

            template<class StopPred, class StopPredPure, int Flags> static Ch * SkipAndExpandCharacterRefs(Ch *&text)
            {
                if (Flags & ParseNoEntityTranslation &&
                    !(Flags & ParseNormalizeWhitespace) &&
                    !(Flags & ParseTrimWhitespace))
                {
                    Skip<StopPred, Flags>(text);
                    return text;
                }
                Skip<StopPredPure, Flags>(text);
                Ch *src = text;
                Ch *dest = src;
                while (StopPred::Test(*src))
                {
                    if (!(Flags & ParseNoEntityTranslation))
                    {
                        if (src[0] == Ch('&'))
                        {
                            switch (src[1])
                            {
                            case Ch('a'):
                                if (src[2] == Ch('m') && src[3] == Ch('p') && src[4] == Ch(';'))
                                {
                                    *dest = Ch('&');
                                    ++dest;
                                    src += 5;
                                    continue;
                                }
                                if (src[2] == Ch('p') && src[3] == Ch('o') && src[4] == Ch('s') && src[5] == Ch(';'))
                                {
                                    *dest = Ch('\'');
                                    ++dest;
                                    src += 6;
                                    continue;
                                }
                                break;
                            case Ch('q'):
                                if (src[2] == Ch('u') && src[3] == Ch('o') && src[4] == Ch('t') && src[5] == Ch(';'))
                                {
                                    *dest = Ch('"');
                                    ++dest;
                                    src += 6;
                                    continue;
                                }
                                break;
                            case Ch('g'):
                                if (src[2] == Ch('t') && src[3] == Ch(';'))
                                {
                                    *dest = Ch('>');
                                    ++dest;
                                    src += 4;
                                    continue;
                                }
                                break;
                            case Ch('l'):
                                if (src[2] == Ch('t') && src[3] == Ch(';'))
                                {
                                    *dest = Ch('<');
                                    ++dest;
                                    src += 4;
                                    continue;
                                }
                                break;
                            case Ch('#'):
                                if (src[2] == Ch('x'))
                                {
                                    unsigned long code = 0;
                                    src += 3;
                                    while (1)
                                    {
                                        unsigned char digit = Digits::Digit(*src);
                                        if (digit == 0xFF)
                                            break;
                                        code = code * 16 + digit;
                                        ++src;
                                    }
                                    InsertCodedCharacter<Flags>(dest, code);
                                }
                                else
                                {
                                    unsigned long code = 0;
                                    src += 2;
                                    while (1)
                                    {
                                        unsigned char digit = Digits::Digit(*src);
                                        if (digit == 0xFF)
                                            break;
                                        code = code * 10 + digit;
                                        ++src;
                                    }
                                    InsertCodedCharacter<Flags>(dest, code);
                                }
                                if (*src == Ch(';'))
                                    ++src;
                                else
                                    throw ParseError("expected ;", src);
                                continue;
                            default:
                                break;
                            }
                        }
                    }

                    if (Flags & ParseNormalizeWhitespace)
                    {
                        if (Whitespace::Test(*src))
                        {
                            *dest = Ch(' '); ++dest;
                            ++src;
                            while (Whitespace::Test(*src))
                                ++src;
                            continue;
                        }
                    }
                    *dest++ = *src++;
                }
                text = src;
                return dest;
            }

            template<int Flags> void ParseBom(Ch *&text)
            {
                if (static_cast<unsigned char>(text[0]) == 0xEF &&
                    static_cast<unsigned char>(text[1]) == 0xBB &&
                    static_cast<unsigned char>(text[2]) == 0xBF)
                {
                    text += 3;
                }
            }

            template<int Flags> XmlNode<Ch> *ParseXmlDeclaration(Ch *&text)
            {
                if (!(Flags & ParseDeclarationNode))
                {
                    while (text[0] != Ch('?') || text[1] != Ch('>'))
                    {
                        if (!text[0])
                            throw ParseError("unexpected end of data", text);
                        ++text;
                    }
                    text += 2;
                    return 0;
                }
                XmlNode<Ch> *declaration = this->AllocateNode(NodeDeclaration);
                Skip<Whitespace, Flags>(text);
                ParseNodeAttributes<Flags>(text, declaration);
                if (text[0] != Ch('?') || text[1] != Ch('>'))
                    throw ParseError("expected ?>", text);
                text += 2;
                return declaration;
            }

            template<int Flags> XmlNode<Ch> * ParseComment(Ch *&text)
            {
                if (!(Flags & ParseCommentNodes))
                {
                    while (text[0] != Ch('-') || text[1] != Ch('-') || text[2] != Ch('>'))
                    {
                        if (!text[0])
                            throw ParseError("unexpected end of data", text);
                        ++text;
                    }
                    text += 3;
                    return 0; 
                }
                Ch * value = text;
                while (text[0] != Ch('-') || text[1] != Ch('-') || text[2] != Ch('>'))
                {
                    if (!text[0])
                        throw ParseError("unexpected end of data", text);
                    ++text;
                }
                XmlNode<Ch> *comment = this->AllocateNode(NodeComment);
                comment->Value(value, text - value);
                if (!(Flags & ParseNoStringTerminators))
                    *text = Ch('\0');
                text += 3;
                return comment;
            }

            template<int Flags> XmlNode<Ch> *ParseDocType(Ch *&text)
            {
                Ch *value = text;
                while (*text != Ch('>'))
                {
                    switch (*text)
                    {
                    case Ch('['):
                    {
                        ++text;
                        int depth = 1;
                        while (depth > 0)
                        {
                            switch (*text)
                            {
                            case Ch('['): ++depth; break;
                            case Ch(']'): --depth; break;
                            case 0: throw ParseError("unexpected end of data", text);
                            }
                            ++text;
                        }
                        break;
                    }
                    case Ch('\0'):
                        throw ParseError("unexpected end of data", text);
                    default:
                        ++text;
                    }
                }

                if (Flags & ParseDocTypeNode)
                {
                    XmlNode<Ch> *doctype = this->AllocateNode(NodeDocType);
                    doctype->Value(value, text - value);
                    if (!(Flags & ParseNoStringTerminators))
                        *text = Ch('\0');
                    text += 1;
                    return doctype;
                }
                else
                {
                    text += 1;
                    return 0;
                }
            }

            template<int Flags> XmlNode<Ch> * ParsePi(Ch *&text)
            {
                if (Flags & ParsePiNodes)
                {
                    XmlNode<Ch> *pi = this->AllocateNode(NodePi);

                    Ch *name = text;
                    Skip<NodeName, Flags>(text);
                    if (text == name)
                        throw ParseError("expected PI target", text);
                    pi->Name(name, text - name);

                    Skip<Whitespace, Flags>(text);

                    Ch *value = text;

                    while (text[0] != Ch('?') || text[1] != Ch('>'))
                    {
                        if (*text == Ch('\0'))
                            throw ParseError("unexpected end of data", text);
                        ++text;
                    }

                    pi->Value(value, text - value);

                    if (!(Flags & ParseNoStringTerminators))
                    {
                        pi->Name()[pi->NameSize()] = Ch('\0');
                        pi->Value()[pi->ValueSize()] = Ch('\0');
                    }
                    text += 2;
                    return pi;
                }
                else
                {
                    while (text[0] != Ch('?') || text[1] != Ch('>'))
                    {
                        if (*text == Ch('\0'))
                            throw ParseError("unexpected end of data", text);
                        ++text;
                    }
                    text += 2;
                    return 0;
                }
            }

            template<int Flags> Ch ParseAndAppendData(XmlNode<Ch> *node, Ch *&text, Ch *contents_start)
            {
                if (!(Flags & ParseTrimWhitespace))
                    text = contents_start;

                Ch *value = text, *end;
                if (Flags & ParseNormalizeWhitespace)
                    end = SkipAndExpandCharacterRefs<Text, TextPureWithWs, Flags>(text);
                else
                    end = SkipAndExpandCharacterRefs<Text, TextPureNoWs, Flags>(text);

                if (Flags & ParseTrimWhitespace)
                {
                    if (Flags & ParseNormalizeWhitespace)
                    {
                        if (*(end - 1) == Ch(' '))
                            --end;
                    }
                    else
                    {
                        while (Whitespace::Test(*(end - 1)))
                            --end;
                    }
                }

                if (!(Flags & ParseNoDataNodes))
                {
                    XmlNode<Ch> *data = this->AllocateNode(NodeData);
                    data->Value(value, end - value);
                    node->AppendNode(data);
                }

                if (!(Flags & ParseNoElementValues))
                    if (*node->Value() == Ch('\0'))
                        node->Value(value, end - value);

                if (!(Flags & ParseNoStringTerminators))
                {
                    Ch ch = *text;
                    *end = Ch('\0');
                    return ch;
                }

                return * text;
            }

            template<int Flags> XmlNode<Ch> *ParseCData(Ch *&text)
            {
                if (Flags & ParseNoDataNodes)
                {
                    while (text[0] != Ch(']') || text[1] != Ch(']') || text[2] != Ch('>'))
                    {
                        if (!text[0])
                            throw ParseError("unexpected end of data", text);
                        ++text;
                    }
                    text += 3;
                    return 0;
                }
                Ch *value = text;
                while (text[0] != Ch(']') || text[1] != Ch(']') || text[2] != Ch('>'))
                {
                    if (!text[0])
                        throw ParseError("unexpected end of data", text);
                    ++text;
                }
                XmlNode<Ch> *cdata = this->AllocateNode(NodeCData);
                cdata->Value(value, text - value);
                if (!(Flags & ParseNoStringTerminators))
                    *text = Ch('\0');
                text += 3;
                return cdata;
            }

            template<int Flags> XmlNode<Ch> * ParseElement(Ch *&text)
            {
                XmlNode<Ch> *element = this->AllocateNode(NodeElement);
                Ch *name = text;
                Skip<NodeName, Flags>(text);
                if (text == name)
                    throw ParseError("expected element name", text);
                element->Name(name, text - name);
                Skip<Whitespace, Flags>(text);
                ParseNodeAttributes<Flags>(text, element);
                if (*text == Ch('>'))
                {
                    ++text;
                    ParseNodeContents<Flags>(text, element);
                }
                else if (*text == Ch('/'))
                {
                    ++text;
                    if (*text != Ch('>'))
                        throw ParseError("expected >", text);
                    ++text;
                }
                else
                    throw ParseError("expected >", text);
                if (!(Flags & ParseNoStringTerminators))
                    element->Name()[element->NameSize()] = Ch('\0');
                return element;
            }

            template<int Flags> XmlNode<Ch> * ParseNode(Ch *&text)
            {
                switch (text[0])
                {
                default:
                    return ParseElement<Flags>(text);
                case Ch('?'):
                    ++text;
                    if ((text[0] == Ch('x') || text[0] == Ch('X')) &&
                        (text[1] == Ch('m') || text[1] == Ch('M')) &&
                        (text[2] == Ch('l') || text[2] == Ch('L')) &&
                        Whitespace::Test(text[3]))
                    {
                        text += 4; 
                        return ParseXmlDeclaration<Flags>(text);
                    }
                    else
                    {
                        return ParsePi<Flags>(text);
                    }
                case Ch('!'):
                    switch (text[1])
                    {
                    case Ch('-'):
                        if (text[2] == Ch('-'))
                        {
                            text += 3;
                            return ParseComment<Flags>(text);
                        }
                        break;
                    case Ch('['):
                        if (text[2] == Ch('C') && text[3] == Ch('D') && text[4] == Ch('A') &&
                            text[5] == Ch('T') && text[6] == Ch('A') && text[7] == Ch('['))
                        {
                            return ParseCData<Flags>(text);
                        }
                        break;
                    case Ch('D'):
                        if (text[2] == Ch('O') && text[3] == Ch('C') && text[4] == Ch('T') &&
                            text[5] == Ch('Y') && text[6] == Ch('P') && text[7] == Ch('E') &&
                            Whitespace::Test(text[8]))
                        {
                            text += 9;
                            return ParseDocType<Flags>(text);
                        }
                    } 
                    ++text;
                    while (*text != Ch('>'))
                    {
                        if (*text == 0)
                            throw ParseError("unexpected end of data", text);
                        ++text;
                    }
                    ++text;
                    return 0;
                }
            }

            template<int Flags> void ParseNodeContents(Ch *&text, XmlNode<Ch> * node)
            {
                while (1)
                {
                    Ch *contents_start = text; 
                    Skip<Whitespace, Flags>(text);
                    Ch next_char = *text;
                after_data_node:
                    switch (next_char)
                    {
                    case Ch('<'):
                        if (text[1] == Ch('/'))
                        {
                            text += 2; 
                            if (Flags & ParseValidateClosingTags)
                            {
                                Ch *closing_name = text;
                                Skip<NodeName, Flags>(text);
                                if (!Internal::Compare(node->Name(), node->NameSize(), closing_name, text - closing_name, true))
                                    throw ParseError("invalid closing tag name", text);
                            }
                            else
                            {
                                Skip<NodeName, Flags>(text);
                            }
                            Skip<Whitespace, Flags>(text);
                            if (*text != Ch('>'))
                                throw ParseError("expected >", text);
                            ++text;  
                            return; 
                        }
                        else
                        {
                            ++text;
                            if (XmlNode<Ch> *child = ParseNode<Flags>(text))
                                node->AppendNode(child);
                        }
                        break;
                    case Ch('\0'):
                        throw ParseError("unexpected end of data", text);
                    default:
                        next_char = ParseAndAppendData<Flags>(node, text, contents_start);
                        goto after_data_node;
                    }
                }
            }

            template<int Flags> void ParseNodeAttributes(Ch *&text, XmlNode<Ch> *node)
            {
                while (AttributeName::Test(*text))
                {
                    Ch *name = text;
                    ++text;
                    Skip<AttributeName, Flags>(text);
                    if (text == name)
                        throw ParseError("expected attribute name", name);
                    XmlAttribute<Ch> *attribute = this->AllocateAttribute();
                    attribute->Name(name, text - name);
                    node->AppendAttribute(attribute);
                    Skip<Whitespace, Flags>(text);
                    if (*text != Ch('='))
                        throw ParseError("expected =", text);
                    ++text;
                    if (!(Flags & ParseNoStringTerminators))
                        attribute->Name()[attribute->NameSize()] = 0;
                    Skip<Whitespace, Flags>(text);
                    Ch quote = *text;
                    if (quote != Ch('\'') && quote != Ch('"'))
                        throw ParseError("expected ' or \"", text);
                    ++text;
                    Ch *value = text, *end;
                    const int AttFlags = Flags & ~ParseNormalizeWhitespace;
                    if (quote == Ch('\''))
                        end = SkipAndExpandCharacterRefs<AttributeValue<Ch('\'')>, AttributeValuePure<Ch('\'')>, AttFlags>(text);
                    else
                        end = SkipAndExpandCharacterRefs<AttributeValue<Ch('"')>, AttributeValuePure<Ch('"')>, AttFlags>(text);
                    attribute->Value(value, end - value);
                    if (*text != quote)
                        throw ParseError("expected ' or \"", text);
                    ++text;
                    if (!(Flags & ParseNoStringTerminators))
                        attribute->Value()[attribute->ValueSize()] = 0;
                    Skip<Whitespace, Flags>(text);
                }
            }
        };

        /*! @ingroup cpl_xml
        * \class NodeIterator
        * \brief Bidirectional iterator over the children of an XmlNode.
        * \tparam Ch - Character type of the iterated nodes.
        */
        template<class Ch> class NodeIterator
        {
        public:
            /*!
            * \typedef value_type
            * \brief Type of the iterated node.
            */
            typedef XmlNode<Ch> value_type;

            /*!
            * \typedef reference
            * \brief Reference to the iterated node.
            */
            typedef XmlNode<Ch> & reference;

            /*!
            * \typedef pointer
            * \brief Pointer to the iterated node.
            */
            typedef XmlNode<Ch> * pointer;

            /*!
            * \typedef difference_type
            * \brief Distance type required by iterator traits.
            */
            typedef std::ptrdiff_t difference_type;

            /*!
            * \typedef iterator_category
            * \brief Bidirectional iterator tag.
            */
            typedef std::bidirectional_iterator_tag iterator_category;

            /*!
            * \fn NodeIterator()
            * \brief Constructs a singular iterator that compares equal to a past-the-end iterator.
            */
            NodeIterator()
                : _node(0)
            {
            }

            /*!
            * \fn NodeIterator(XmlNode<Ch> *node)
            * \brief Constructs an iterator at the first child of node.
            * \param [in] node - Parent whose children are iterated.
            */
            NodeIterator(XmlNode<Ch> *node)
                : _node(node->FirstNode())
            {
            }

            /*!
            * \fn reference operator *() const
            * \brief Returns the current child node.
            * \return Reference to the current XmlNode. Asserts that the iterator is not past-the-end.
            */
            reference operator *() const
            {
                assert(_node);
                return *_node;
            }

            /*!
            * \fn pointer operator->() const
            * \brief Returns a pointer to the current child node.
            * \return Pointer to the current XmlNode. Asserts that the iterator is not past-the-end.
            */
            pointer operator->() const
            {
                assert(_node);
                return _node;
            }

            /*!
            * \fn NodeIterator& operator++()
            * \brief Advances to the next sibling.
            * \return *this. Asserts that the iterator is not past-the-end.
            */
            NodeIterator& operator++()
            {
                assert(_node);
                _node = _node->NextSibling();
                return *this;
            }

            /*!
            * \fn NodeIterator operator++(int)
            * \brief Advances to the next sibling and returns the previous iterator value.
            * \return Copy of the iterator before it was advanced.
            */
            NodeIterator operator++(int)
            {
                NodeIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            /*!
            * \fn NodeIterator& operator--()
            * \brief Moves to the previous sibling.
            * \return *this. Asserts that a previous sibling exists.
            */
            NodeIterator& operator--()
            {
                assert(_node && _node->PreviousSibling());
                _node = _node->PreviousSibling();
                return *this;
            }

            /*!
            * \fn NodeIterator operator--(int)
            * \brief Moves to the previous sibling and returns the previous iterator value.
            * \return Copy of the iterator before it was moved.
            */
            NodeIterator operator--(int)
            {
                NodeIterator tmp = *this;
                --(*this);
                return tmp;
            }

            /*!
            * \fn bool operator ==(const NodeIterator<Ch> &rhs)
            * \brief Compares two iterators for equality.
            * \param [in] rhs - Other iterator.
            * \return true when both iterators point to the same node (including both past-the-end).
            */
            bool operator ==(const NodeIterator<Ch> &rhs)
            {
                return _node == rhs._node;
            }

            /*!
            * \fn bool operator !=(const NodeIterator<Ch> &rhs)
            * \brief Compares two iterators for inequality.
            * \param [in] rhs - Other iterator.
            * \return true when the iterators point to different nodes.
            */
            bool operator !=(const NodeIterator<Ch> &rhs)
            {
                return _node != rhs._node;
            }

        private:

            XmlNode<Ch> *_node;

        };

        /*! @ingroup cpl_xml
        * \class AttributeIterator
        * \brief Bidirectional iterator over the attributes of an XmlNode.
        * \tparam Ch - Character type of the iterated attributes.
        */
        template<class Ch> class AttributeIterator
        {
        public:
            /*!
            * \typedef value_type
            * \brief Type of the iterated attribute.
            */
            typedef XmlAttribute<Ch> value_type;

            /*!
            * \typedef reference
            * \brief Reference to the iterated attribute.
            */
            typedef XmlAttribute<Ch> &reference;

            /*!
            * \typedef pointer
            * \brief Pointer to the iterated attribute.
            */
            typedef XmlAttribute<Ch> *pointer;

            /*!
            * \typedef difference_type
            * \brief Distance type required by iterator traits.
            */
            typedef std::ptrdiff_t difference_type;

            /*!
            * \typedef iterator_category
            * \brief Bidirectional iterator tag.
            */
            typedef std::bidirectional_iterator_tag iterator_category;

            /*!
            * \fn AttributeIterator()
            * \brief Constructs a singular iterator that compares equal to a past-the-end iterator.
            */
            AttributeIterator()
                : _attribute(0)
            {
            }

            /*!
            * \fn AttributeIterator(XmlNode<Ch> *node)
            * \brief Constructs an iterator at the first attribute of node.
            * \param [in] node - Node whose attributes are iterated.
            */
            AttributeIterator(XmlNode<Ch> *node)
                : _attribute(node->FirstAttribute())
            {
            }

            /*!
            * \fn reference operator *() const
            * \brief Returns the current attribute.
            * \return Reference to the current XmlAttribute. Asserts that the iterator is not past-the-end.
            */
            reference operator *() const
            {
                assert(_attribute);
                return *_attribute;
            }

            /*!
            * \fn pointer operator->() const
            * \brief Returns a pointer to the current attribute.
            * \return Pointer to the current XmlAttribute. Asserts that the iterator is not past-the-end.
            */
            pointer operator->() const
            {
                assert(_attribute);
                return _attribute;
            }

            /*!
            * \fn AttributeIterator& operator++()
            * \brief Advances to the next attribute.
            * \return *this. Asserts that the iterator is not past-the-end.
            */
            AttributeIterator& operator++()
            {
                assert(_attribute);
                _attribute = _attribute->NextAttribute();
                return *this;
            }

            /*!
            * \fn AttributeIterator operator++(int)
            * \brief Advances to the next attribute and returns the previous iterator value.
            * \return Copy of the iterator before it was advanced.
            */
            AttributeIterator operator++(int)
            {
                AttributeIterator tmp = *this;
                ++(*this);
                return tmp;
            }

            /*!
            * \fn AttributeIterator& operator--()
            * \brief Moves to the previous attribute.
            * \return *this. Asserts that a previous attribute exists.
            */
            AttributeIterator& operator--()
            {
                assert(_attribute && _attribute->PreviousAttribute());
                _attribute = _attribute->PreviousAttribute();
                return *this;
            }

            /*!
            * \fn AttributeIterator operator--(int)
            * \brief Moves to the previous attribute and returns the previous iterator value.
            * \return Copy of the iterator before it was moved.
            */
            AttributeIterator operator--(int)
            {
                AttributeIterator tmp = *this;
                --(*this);
                return tmp;
            }

            /*!
            * \fn bool operator ==(const AttributeIterator<Ch> &rhs)
            * \brief Compares two iterators for equality.
            * \param [in] rhs - Other iterator.
            * \return true when both iterators point to the same attribute (including both past-the-end).
            */
            bool operator ==(const AttributeIterator<Ch> &rhs)
            {
                return _attribute == rhs._attribute;
            }

            /*!
            * \fn bool operator !=(const AttributeIterator<Ch> &rhs)
            * \brief Compares two iterators for inequality.
            * \param [in] rhs - Other iterator.
            * \return true when the iterators point to different attributes.
            */
            bool operator !=(const AttributeIterator<Ch> &rhs)
            {
                return _attribute != rhs._attribute;
            }

        private:

            XmlAttribute<Ch> *_attribute;

        };

        /*! @ingroup cpl_xml
        * \brief Print() flag that writes markup without tab indentation or trailing newlines.
        */
        const int PrintNoIndenting = 0x1;

        namespace Internal
        {
            template<class OutIt, class Ch> inline OutIt CopyChars(const Ch *begin, const Ch *end, OutIt out)
            {
                while (begin != end)
                    *out++ = *begin++;
                return out;
            }

            template<class OutIt, class Ch> inline OutIt CopyAndExpandChars(const Ch *begin, const Ch *end, Ch noexpand, OutIt out)
            {
                while (begin != end)
                {
                    if (*begin == noexpand)
                    {
                        *out++ = *begin;
                    }
                    else
                    {
                        switch (*begin)
                        {
                        case Ch('<'):
                            *out++ = Ch('&'); *out++ = Ch('l'); *out++ = Ch('t'); *out++ = Ch(';');
                            break;
                        case Ch('>'):
                            *out++ = Ch('&'); *out++ = Ch('g'); *out++ = Ch('t'); *out++ = Ch(';');
                            break;
                        case Ch('\''):
                            *out++ = Ch('&'); *out++ = Ch('a'); *out++ = Ch('p'); *out++ = Ch('o'); *out++ = Ch('s'); *out++ = Ch(';');
                            break;
                        case Ch('"'):
                            *out++ = Ch('&'); *out++ = Ch('q'); *out++ = Ch('u'); *out++ = Ch('o'); *out++ = Ch('t'); *out++ = Ch(';');
                            break;
                        case Ch('&'):
                            *out++ = Ch('&'); *out++ = Ch('a'); *out++ = Ch('m'); *out++ = Ch('p'); *out++ = Ch(';');
                            break;
                        default:
                            *out++ = *begin;
                        }
                    }
                    ++begin;
                }
                return out;
            }

            template<class OutIt, class Ch> inline OutIt FillChars(OutIt out, int n, Ch ch)
            {
                for (int i = 0; i < n; ++i)
                    *out++ = ch;
                return out;
            }

            template<class Ch, Ch ch> inline bool FindChar(const Ch *begin, const Ch *end)
            {
                while (begin != end)
                    if (*begin++ == ch)
                        return true;
                return false;
            }

            template<class OutIt, class Ch> inline OutIt PrintNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent);

            template<class OutIt, class Ch> inline OutIt PrintChildren(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                for (XmlNode<Ch> *child = node->FirstNode(); child; child = child->NextSibling())
                    out = PrintNode(out, child, flags, indent);
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintAttributes(OutIt out, const XmlNode<Ch> *node, int flags)
            {
                for (XmlAttribute<Ch> *attribute = node->FirstAttribute(); attribute; attribute = attribute->NextAttribute())
                {
                    if (attribute->Name() && attribute->Value())
                    {
                        *out = Ch(' '), ++out;
                        out = CopyChars(attribute->Name(), attribute->Name() + attribute->NameSize(), out);
                        *out = Ch('='), ++out;
                        if (FindChar<Ch, Ch('"')>(attribute->Value(), attribute->Value() + attribute->ValueSize()))
                        {
                            *out = Ch('\''), ++out;
                            out = CopyAndExpandChars(attribute->Value(), attribute->Value() + attribute->ValueSize(), Ch('"'), out);
                            *out = Ch('\''), ++out;
                        }
                        else
                        {
                            *out = Ch('"'), ++out;
                            out = CopyAndExpandChars(attribute->Value(), attribute->Value() + attribute->ValueSize(), Ch('\''), out);
                            *out = Ch('"'), ++out;
                        }
                    }
                }
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintDataNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                assert(node->Type() == NodeData);
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                out = CopyAndExpandChars(node->Value(), node->Value() + node->ValueSize(), Ch(0), out);
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintCDataNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                assert(node->Type() == NodeCData);
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                *out = Ch('<'); ++out;
                *out = Ch('!'); ++out;
                *out = Ch('['); ++out;
                *out = Ch('C'); ++out;
                *out = Ch('D'); ++out;
                *out = Ch('A'); ++out;
                *out = Ch('T'); ++out;
                *out = Ch('A'); ++out;
                *out = Ch('['); ++out;
                out = CopyChars(node->Value(), node->Value() + node->ValueSize(), out);
                *out = Ch(']'); ++out;
                *out = Ch(']'); ++out;
                *out = Ch('>'); ++out;
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintElementNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                assert(node->Type() == NodeElement);
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                *out = Ch('<'), ++out;
                out = CopyChars(node->Name(), node->Name() + node->NameSize(), out);
                out = PrintAttributes(out, node, flags);
                if (node->ValueSize() == 0 && !node->FirstNode())
                {
                    *out = Ch('/'), ++out;
                    *out = Ch('>'), ++out;
                }
                else
                {
                    *out = Ch('>'), ++out;
                    XmlNode<Ch> *child = node->FirstNode();
                    if (!child)
                    {
                        out = CopyAndExpandChars(node->Value(), node->Value() + node->ValueSize(), Ch(0), out);
                    }
                    else if (child->NextSibling() == 0 && child->Type() == NodeData)
                    {
                        out = CopyAndExpandChars(child->Value(), child->Value() + child->ValueSize(), Ch(0), out);
                    }
                    else
                    {
                        if (!(flags & PrintNoIndenting))
                            *out = Ch('\n'), ++out;
                        out = PrintChildren(out, node, flags, indent + 1);
                        if (!(flags & PrintNoIndenting))
                            out = FillChars(out, indent, Ch('\t'));
                    }
                    *out = Ch('<'), ++out;
                    *out = Ch('/'), ++out;
                    out = CopyChars(node->Name(), node->Name() + node->NameSize(), out);
                    *out = Ch('>'), ++out;
                }
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintDeclarationNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                *out = Ch('<'), ++out;
                *out = Ch('?'), ++out;
                *out = Ch('x'), ++out;
                *out = Ch('m'), ++out;
                *out = Ch('l'), ++out;
                out = PrintAttributes(out, node, flags);
                *out = Ch('?'), ++out;
                *out = Ch('>'), ++out;
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintCommentNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                assert(node->Type() == NodeComment);
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                *out = Ch('<'), ++out;
                *out = Ch('!'), ++out;
                *out = Ch('-'), ++out;
                *out = Ch('-'), ++out;
                out = CopyChars(node->Value(), node->Value() + node->ValueSize(), out);
                *out = Ch('-'), ++out;
                *out = Ch('-'), ++out;
                *out = Ch('>'), ++out;
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintDocTypeNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                assert(node->Type() == NodeDocType);
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                *out = Ch('<'), ++out;
                *out = Ch('!'), ++out;
                *out = Ch('D'), ++out;
                *out = Ch('O'), ++out;
                *out = Ch('C'), ++out;
                *out = Ch('T'), ++out;
                *out = Ch('Y'), ++out;
                *out = Ch('P'), ++out;
                *out = Ch('E'), ++out;
                *out = Ch(' '), ++out;
                out = CopyChars(node->Value(), node->Value() + node->ValueSize(), out);
                *out = Ch('>'), ++out;
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintPiNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                assert(node->Type() == NodePi);
                if (!(flags & PrintNoIndenting))
                    out = FillChars(out, indent, Ch('\t'));
                *out = Ch('<'), ++out;
                *out = Ch('?'), ++out;
                out = CopyChars(node->Name(), node->Name() + node->NameSize(), out);
                *out = Ch(' '), ++out;
                out = CopyChars(node->Value(), node->Value() + node->ValueSize(), out);
                *out = Ch('?'), ++out;
                *out = Ch('>'), ++out;
                return out;
            }

            template<class OutIt, class Ch> inline OutIt PrintNode(OutIt out, const XmlNode<Ch> *node, int flags, int indent)
            {
                switch (node->Type())
                {
                case NodeDocument:
                    out = PrintChildren(out, node, flags, indent);
                    break;
                case NodeElement:
                    out = PrintElementNode(out, node, flags, indent);
                    break;
                case NodeData:
                    out = PrintDataNode(out, node, flags, indent);
                    break;
                case NodeCData:
                    out = PrintCDataNode(out, node, flags, indent);
                    break;
                case NodeDeclaration:
                    out = PrintDeclarationNode(out, node, flags, indent);
                    break;
                case NodeComment:
                    out = PrintCommentNode(out, node, flags, indent);
                    break;
                case NodeDocType:
                    out = PrintDocTypeNode(out, node, flags, indent);
                    break;
                case NodePi:
                    out = PrintPiNode(out, node, flags, indent);
                    break;
                default:
                    assert(0);
                    break;
                }
                if (!(flags & PrintNoIndenting))
                    *out = Ch('\n'), ++out;
                return out;
            }
        }

        /*! @ingroup cpl_xml
        * \brief Writes node and its descendants to an output iterator as XML text.
        * \tparam OutIt - Output iterator whose value type accepts Ch.
        * \tparam Ch - Character type of the node tree.
        * \param [in,out] out - Destination iterator. Advanced as characters are written.
        * \param [in] node - Root of the tree to print. A document node prints only its children.
        * \param [in] flags - 0 for indented output, or PrintNoIndenting to omit tabs and newlines.
        * \return out after the last written character.
        * \note Special characters in text and attribute values are written as XML entities
        *       (&lt;, &gt;, &amp;, &apos;, &quot;) except inside CDATA, comments, DOCTYPE and PI nodes.
        */
        template<class OutIt, class Ch> inline OutIt Print(OutIt out, const XmlNode<Ch> &node, int flags = 0)
        {
            return Internal::PrintNode(out, &node, flags, 0);
        }

        /*! @ingroup cpl_xml
        * \brief Writes node and its descendants to an output stream as XML text.
        * \tparam Ch - Character type of the stream and of the node tree.
        * \param [in,out] out - Destination stream.
        * \param [in] node - Root of the tree to print.
        * \param [in] flags - 0 for indented output, or PrintNoIndenting to omit tabs and newlines.
        * \return out.
        */
        template<class Ch> inline std::basic_ostream<Ch> & Print(std::basic_ostream<Ch> &out, const XmlNode<Ch> &node, int flags = 0)
        {
            Print(std::ostream_iterator<Ch>(out), node, flags);
            return out;
        }

        /*! @ingroup cpl_xml
        * \brief Writes node to out using Print() with default flags (indented).
        * \tparam Ch - Character type of the stream and of the node tree.
        * \param [in,out] out - Destination stream.
        * \param [in] node - Root of the tree to print.
        * \return out.
        */
        template<class Ch> inline std::basic_ostream<Ch> & operator <<(std::basic_ostream<Ch> & out, const XmlNode<Ch> & node)
        {
            return Print(out, node);
        }

        /*! @ingroup cpl_xml
        * \class File
        * \brief Loads a zero-terminated XML source buffer from a file, memory or input stream.
        * \tparam Ch - Character type of the buffer. Default is char.
        * \note Data() is suitable as the text argument of XmlDocument::Parse. Size() includes
        *       the terminating zero, so the character count passed to Parse() is Size() - 1
        *       when the buffer is non-empty.
        */
        template<class Ch = char> class File
        {
        public:
            /*!
            * \fn File()
            * \brief Constructs an empty buffer. Data() is empty until Open or an initializing constructor is used.
            */
            File()
            {
            }

            /*!
            * \fn File(const char * fileName)
            * \brief Loads the contents of fileName. Throws std::runtime_error if the file cannot be opened.
            * \param [in] fileName - Path passed to Open().
            */
            File(const char * fileName)
            {
                if (!Open(fileName))
                    throw std::runtime_error(std::string("Can't open file ") + fileName);
            }

            /*!
            * \fn File(const Ch * data, size_t size)
            * \brief Copies size characters from data and appends a zero terminator.
            * \param [in] data - Source characters. size characters are copied.
            * \param [in] size - Number of characters to copy.
            */
            File(const Ch * data, size_t size)
            {
                _data.resize(size + 1);
                std::copy(data, data + size, std::begin(_data));
                _data[size] = 0;
            }

            /*!
            * \fn File(std::basic_istream<Ch> & is)
            * \brief Reads the entire seekable stream into the buffer and appends a zero terminator.
            * \param [in,out] is - Input stream. skipws is cleared. Throws std::runtime_error on a read error.
            */
            File(std::basic_istream<Ch> & is)
            {
                decltype(_data) tdata;
                is.unsetf(std::ios::skipws);
                is.seekg(0, std::ios::end);
                size_t size = is.tellg();
                tdata.resize(size + 1);
                is.seekg(0, std::ios::beg);
                std::copy(std::istreambuf_iterator<Ch>(is), std::istreambuf_iterator<Ch>(), std::begin(tdata));
                if (is.fail() || is.bad())
                    throw std::runtime_error("error reading stream");
                tdata[size] = 0;
                _data = std::move(tdata);
            }

            /*!
            * \fn bool Open(const char * fileName)
            * \brief Replaces the buffer with the contents of fileName plus a zero terminator.
            * \param [in] fileName - Path opened as a binary ifstream.
            * \return true when the file was opened and read. false when the file cannot be opened.
            */
            bool Open(const char * fileName)
            {
                std::basic_ifstream<Ch> ifs(fileName, std::ios::binary);
                if (!ifs)
                    return false;
                ifs.unsetf(std::ios::skipws);
                ifs.seekg(0, std::ios::end);
                size_t size = ifs.tellg();
                ifs.seekg(0);
                _data.resize(size + 1);
                ifs.read(_data.data(), (std::streamsize)size);
                _data[size] = 0;
                return true;
            }

            /*!
            * \fn Ch * Data()
            * \brief Returns a pointer to the mutable zero-terminated buffer.
            * \return Pointer to Size() characters. The last character is 0 when the buffer is non-empty.
            */
            Ch * Data()
            {
                return _data.data();
            }

            /*!
            * \fn const Ch * Data() const
            * \brief Returns a pointer to the zero-terminated buffer.
            * \return Pointer to Size() characters. The last character is 0 when the buffer is non-empty.
            */
            const Ch * Data() const
            {
                return _data.data();
            }

            /*!
            * \fn size_t Size() const
            * \brief Returns the buffer length including the terminating zero.
            * \return Number of characters in the buffer, including the terminator. 0 for a default-constructed File.
            */
            size_t Size() const
            {
                return _data.size();
            }

        private:
            std::vector<Ch> _data;
        };

        /*! @ingroup cpl_xml
        * \brief Counts child nodes of node, optionally filtered by name.
        * \tparam Ch - Character type of the node tree.
        * \param [in] node - Parent whose children are counted.
        * \param [in] name - Child name to match, or NULL to count every child.
        * \param [in] nameSize - Number of characters in name. 0 measures a zero-terminated name.
        * \param [in] caseSensitive - If true, compare names with exact character values.
        *                             If false, compare ASCII letters case-insensitively.
        * \return Number of matching children.
        */
        template<class Ch> inline size_t CountChildren(XmlNode<Ch>* node, const Ch* name = 0, size_t nameSize = 0, bool caseSensitive = true)
        {
            XmlNode<Ch>* child = node->FirstNode(name, nameSize, caseSensitive);
            size_t count = 0;
            while (child)
            {
                ++count;
                child = child->NextSibling(name, nameSize, caseSensitive);
            }
            return count;
        }

        /*! @ingroup cpl_xml
        * \brief Counts the attributes attached to node.
        * \tparam Ch - Character type of the node tree.
        * \param [in] node - Node whose attributes are counted.
        * \return Number of attributes.
        */
        template<class Ch> inline size_t CountAttributes(XmlNode<Ch> * node)
        {
            XmlAttribute<Ch> *attr = node->FirstAttribute();
            size_t count = 0;
            while (attr)
            {
                ++count;
                attr = attr->NextAttribute();
            }
            return count;
        }
    }
}
