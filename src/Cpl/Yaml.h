/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
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

#pragma once

#include "Cpl/String.h"

namespace Cpl
{
    /*! @ingroup cpl_yaml
    * \namespace Cpl::Yaml
    * \brief YAML document tree: parse text or a file into Node objects and serialize them back.
    * \note Parse() and Serialize() throw Cpl::Yaml::Exception (or a derived type) on failure.
    *       Implementation helpers live in Cpl::Yaml::Detail and are not part of the public API.
    */
    namespace Yaml
    {
        class Node;

        namespace Detail
        {
            template<typename T> struct StringConverter
            {
                static T Get(const std::string& data)
                {
                    T type;
                    std::stringstream ss(data);
                    ss >> type;
                    return type;
                }

                static T Get(const std::string& data, const T& defaultValue)
                {
                    T type;
                    std::stringstream ss(data);
                    ss >> type;

                    if (ss.fail())
                    {
                        return defaultValue;
                    }

                    return type;
                }
            };

            template<> struct StringConverter<std::string>
            {
                static std::string Get(const std::string& data)
                {
                    return data;
                }

                static std::string Get(const std::string& data, const std::string& defaultValue)
                {
                    if (data.size() == 0)
                    {
                        return defaultValue;
                    }
                    return data;
                }
            };

            template<> struct StringConverter<bool>
            {
                static bool Get(const std::string& data)
                {
                    std::string tmpData = data;
                    std::transform(tmpData.begin(), tmpData.end(), tmpData.begin(), ::tolower);
                    if (tmpData == "true" || tmpData == "yes" || tmpData == "1")
                    {
                        return true;
                    }

                    return false;
                }

                static bool Get(const std::string& data, const bool& defaultValue)
                {
                    if (data.size() == 0)
                    {
                        return defaultValue;
                    }

                    return Get(data);
                }
            };
        }

        /*! @ingroup cpl_yaml
        * \class Exception
        * \brief Base exception for YAML parse, serialize and internal failures.
        */
        class Exception : public std::runtime_error
        {
        public:
            /*!
            * \brief Kind of YAML failure stored by Exception.
            */
            enum Type
            {
                InternalError,  //!< Unexpected internal library error.
                ParsingError,   //!< The input is not valid YAML.
                OperationError  //!< File I/O failure or invalid Serialize() configuration.
            };

            /*!
            * \fn Exception(const std::string& message, const Type type)
            * \brief Stores the error text and a Type value.
            * \param [in] message - Description passed to std::runtime_error.
            * \param [in] type - Category returned by GetType().
            */
            Exception(const std::string& message, const Type type)
                : std::runtime_error(message)
                , _type(type)
            {
            }

            /*!
            * \fn Type GetType() const
            * \brief Returns the error category given to the constructor.
            * \return InternalError, ParsingError or OperationError.
            */
            Type GetType() const
            {
                return _type;
            }

            /*!
            * \fn const char* Message() const
            * \brief Returns the error text. Same pointer as what().
            * \return Zero-terminated description string.
            */
            const char* Message() const
            {
                return what();
            }

        private:
            Type _type;
        };

        /*! @ingroup cpl_yaml
        * \class InternalException
        * \brief Exception with Type InternalError.
        */
        class InternalException : public Exception
        {
        public:
            /*!
            * \fn InternalException(const std::string& message)
            * \brief Constructs an InternalError exception.
            * \param [in] message - Description passed to Exception.
            */
            InternalException(const std::string& message)
                : Exception(message, InternalError)
            {
            }
        };

        /*! @ingroup cpl_yaml
        * \class ParsingException
        * \brief Exception thrown by Parse() when the source is not well-formed YAML.
        */
        class ParsingException : public Exception
        {
        public:
            /*!
            * \fn ParsingException(const std::string& message)
            * \brief Constructs a ParsingError exception.
            * \param [in] message - Description of the parse error, often including line text.
            */
            ParsingException(const std::string& message)
                : Exception(message, ParsingError)
            {
            }
        };

        /*! @ingroup cpl_yaml
        * \class OperationException
        * \brief Exception thrown when a file cannot be opened or Serialize() options are invalid.
        */
        class OperationException : public Exception
        {
        public:
            /*!
            * \fn OperationException(const std::string& message)
            * \brief Constructs an OperationError exception.
            * \param [in] message - Description of the failed operation.
            */
            OperationException(const std::string& message)
                : Exception(message, OperationError)
            {
            }
        };

        /*! @ingroup cpl_yaml
        * \class Iterator
        * \brief Mutable iterator over the children of a sequence or map Node.
        * \note Dereference returns a pair: the map key (empty string for a sequence item)
        *       and a reference to the child Node. operator++(int) and operator--(int)
        *       use postfix syntax but return *this after the step.
        */
        class Iterator
        {
        public:
            friend class Node;

            /*!
            * \fn Iterator()
            * \brief Constructs an empty iterator that compares unequal to any populated iterator.
            */
            Iterator();

            /*!
            * \fn Iterator(const Iterator& it)
            * \brief Copies the current position of it.
            * \param [in] it - Iterator to copy.
            */
            Iterator(const Iterator& it);

            /*!
            * \fn Iterator& operator = (const Iterator& it)
            * \brief Replaces this iterator with a copy of it.
            * \param [in] it - Iterator to copy.
            * \return *this.
            */
            Iterator& operator = (const Iterator& it);

            /*!
            * \fn ~Iterator()
            * \brief Destroys the iterator and its implementation object.
            */
            ~Iterator();

            /*!
            * \fn std::pair<const std::string&, Node&> operator *()
            * \brief Returns the current child.
            * \return Pair of map key (empty for a sequence) and a mutable reference to the child.
            *         An empty pair is returned when the iterator has no implementation.
            */
            std::pair<const std::string&, Node&> operator *();

            /*!
            * \fn Iterator& operator ++ (int dummy)
            * \brief Advances to the next child.
            * \param dummy - Unused postfix dummy argument.
            * \return *this after the step.
            */
            Iterator& operator ++ (int);

            /*!
            * \fn Iterator& operator -- (int dummy)
            * \brief Moves to the previous child.
            * \param dummy - Unused postfix dummy argument.
            * \return *this after the step.
            */
            Iterator& operator -- (int);

            /*!
            * \fn bool operator == (const Iterator& it)
            * \brief Compares two iterators of the same container kind.
            * \param [in] it - Other iterator.
            * \return True when both point at the same child, or both are empty of the same kind.
            */
            bool operator == (const Iterator& it);

            /*!
            * \fn bool operator != (const Iterator& it)
            * \brief Negation of operator==.
            * \param [in] it - Other iterator.
            * \return True when the iterators are not equal.
            */
            bool operator != (const Iterator& it);

        private:

            enum eType
            {
                None,
                SequenceType,
                MapType
            };

            eType m_Type;
            void* m_pImp;
        };

        /*! @ingroup cpl_yaml
        * \class ConstIterator
        * \brief Read-only iterator over the children of a sequence or map Node.
        * \note Same pair layout and postfix increment/decrement behaviour as Iterator.
        */
        class ConstIterator
        {
        public:
            friend class Node;

            /*!
            * \fn ConstIterator()
            * \brief Constructs an empty iterator that compares unequal to any populated iterator.
            */
            ConstIterator();

            /*!
            * \fn ConstIterator(const ConstIterator& it)
            * \brief Copies the current position of it.
            * \param [in] it - Iterator to copy.
            */
            ConstIterator(const ConstIterator& it);

            /*!
            * \fn ConstIterator& operator = (const ConstIterator& it)
            * \brief Replaces this iterator with a copy of it.
            * \param [in] it - Iterator to copy.
            * \return *this.
            */
            ConstIterator& operator = (const ConstIterator& it);

            /*!
            * \fn ~ConstIterator()
            * \brief Destroys the iterator and its implementation object.
            */
            ~ConstIterator();

            /*!
            * \fn std::pair<const std::string&, const Node&> operator *()
            * \brief Returns the current child.
            * \return Pair of map key (empty for a sequence) and a const reference to the child.
            *         An empty pair is returned when the iterator has no implementation.
            */
            std::pair<const std::string&, const Node&> operator *();

            /*!
            * \fn ConstIterator& operator ++ (int dummy)
            * \brief Advances to the next child.
            * \param dummy - Unused postfix dummy argument.
            * \return *this after the step.
            */
            ConstIterator& operator ++ (int);

            /*!
            * \fn ConstIterator& operator -- (int dummy)
            * \brief Moves to the previous child.
            * \param dummy - Unused postfix dummy argument.
            * \return *this after the step.
            */
            ConstIterator& operator -- (int);

            /*!
            * \fn bool operator == (const ConstIterator& it)
            * \brief Compares two iterators of the same container kind.
            * \param [in] it - Other iterator.
            * \return True when both point at the same child, or both are empty of the same kind.
            */
            bool operator == (const ConstIterator& it);

            /*!
            * \fn bool operator != (const ConstIterator& it)
            * \brief Negation of operator==.
            * \param [in] it - Other iterator.
            * \return True when the iterators are not equal.
            */
            bool operator != (const ConstIterator& it);

        private:
            enum eType
            {
                None,
                SequenceType,
                MapType
            };

            eType m_Type;
            void* m_pImp;
        };

        /*! @ingroup cpl_yaml
        * \class Node
        * \brief One YAML value: an empty node, a sequence, a map or a scalar string.
        * \note Sequence operations convert this node to SequenceType. Map subscript
        *       converts it to MapType and inserts a missing key. Scalar assignment
        *       converts it to ScalarType.
        */
        class Node
        {
        public:
            friend class Iterator;

            /*!
            * \brief Kind of value stored in a Node.
            */
            enum eType
            {
                None,         //!< Empty node with no children and no scalar text.
                SequenceType, //!< Ordered list of child nodes, addressed by index.
                MapType,      //!< Mapping of string keys to child nodes.
                ScalarType    //!< Single string value, converted by As().
            };

            /*!
            * \fn Node(bool none = false)
            * \brief Constructs an empty node of type None.
            * \param [in] none - When true, Clear() is also called. Used for the empty sentinel
            *                   returned by an out-of-range sequence subscript.
            */
            Node(bool none = false);

            /*!
            * \fn Node(const Node& node)
            * \brief Deep-copies node.
            * \param [in] node - Node to copy.
            */
            Node(const Node& node);

            /*!
            * \fn Node(const std::string& value)
            * \brief Constructs a ScalarType node that holds value.
            * \param [in] value - Scalar text.
            */
            Node(const std::string& value);

            /*!
            * \fn Node(const char* value)
            * \brief Constructs a ScalarType node that holds value.
            * \param [in] value - Zero-terminated scalar text. A null pointer becomes an empty string.
            */
            Node(const char* value);

            /*!
            * \fn ~Node()
            * \brief Destroys the node and its children.
            */
            ~Node();

            /*!
            * \fn eType Type() const
            * \brief Returns the current node kind.
            * \return None, SequenceType, MapType or ScalarType.
            */
            eType Type() const;

            /*!
            * \fn bool IsNone() const
            * \brief Tests whether Type() is None.
            * \return True for an empty node.
            */
            bool IsNone() const;

            /*!
            * \fn bool IsSequence() const
            * \brief Tests whether Type() is SequenceType.
            * \return True for a sequence node.
            */
            bool IsSequence() const;

            /*!
            * \fn bool IsMap() const
            * \brief Tests whether Type() is MapType.
            * \return True for a map node.
            */
            bool IsMap() const;

            /*!
            * \fn bool IsScalar() const
            * \brief Tests whether Type() is ScalarType.
            * \return True for a scalar node.
            */
            bool IsScalar() const;

            /*!
            * \fn void Clear()
            * \brief Removes all children and scalar text and sets Type() to None.
            */
            void Clear();

            /*!
            * \fn T As() const
            * \brief Converts the scalar text to T.
            * \tparam T - Destination type. std::string returns the raw text. bool accepts
            *             "true", "yes" or "1" (case-insensitive) as true and any other text as false.
            *             Other types are parsed with std::stringstream.
            * \return Converted value. For a non-scalar or empty node the converted empty string is used.
            */
            template<typename T> T As() const
            {
                return Detail::StringConverter<T>::Get(AsString());
            }

            /*!
            * \fn T As(const T& defaultValue) const
            * \brief Converts the scalar text to T, or returns defaultValue when conversion fails.
            * \tparam T - Destination type. For std::string and bool, an empty scalar yields defaultValue.
            *             For other types, defaultValue is used when the stream extraction fails.
            * \param [in] defaultValue - Value returned when the scalar cannot be converted.
            * \return Converted value or defaultValue.
            */
            template<typename T> T As(const T& defaultValue) const
            {
                return Detail::StringConverter<T>::Get(AsString(), defaultValue);
            }

            /*!
            * \fn size_t Size() const
            * \brief Returns the number of children of a sequence or map.
            * \return Child count, or 0 for None, ScalarType or a node with no implementation.
            */
            size_t Size() const;

            /*!
            * \fn Node& Insert(const size_t index)
            * \brief Converts this node to a sequence and inserts an empty child at index.
            * \param [in] index - Insertion position. Values past the end append a child.
            * \return Reference to the new child.
            */
            Node& Insert(const size_t index);

            /*!
            * \fn Node& PushFront()
            * \brief Converts this node to a sequence and inserts an empty child at the front.
            * \return Reference to the new child.
            */
            Node& PushFront();

            /*!
            * \fn Node& PushBack()
            * \brief Converts this node to a sequence and appends an empty child.
            * \return Reference to the new child.
            */
            Node& PushBack();

            /*!
            * \fn Node& operator [] (const size_t index)
            * \brief Converts this node to a sequence and returns the child at index.
            * \param [in] index - Zero-based child index.
            * \return The child, or a static empty None node when index is out of range.
            *         A missing index is not created.
            */
            Node& operator []  (const size_t index);

            /*!
            * \fn Node& operator [] (const std::string& key)
            * \brief Converts this node to a map and returns the child named key.
            * \param [in] key - Map key. A missing key is inserted as an empty None child.
            * \return Reference to the existing or newly inserted child.
            */
            Node& operator [] (const std::string& key);

            /*!
            * \fn void Erase(const size_t index)
            * \brief Removes the sequence child at index.
            * \param [in] index - Child to remove. No effect when this node is not a sequence
            *                    or the index does not exist.
            */
            void Erase(const size_t index);

            /*!
            * \fn void Erase(const std::string& key)
            * \brief Removes the map child named key.
            * \param [in] key - Key to remove. No effect when this node is not a map
            *                  or the key does not exist.
            */
            void Erase(const std::string& key);

            /*!
            * \fn Node& operator = (const Node& node)
            * \brief Replaces this node with a deep copy of node.
            * \param [in] node - Source node.
            * \return *this.
            */
            Node& operator = (const Node& node);

            /*!
            * \fn Node& operator = (const std::string& value)
            * \brief Converts this node to a scalar that holds value.
            * \param [in] value - Scalar text.
            * \return *this.
            */
            Node& operator = (const std::string& value);

            /*!
            * \fn Node& operator = (const char* value)
            * \brief Converts this node to a scalar that holds value.
            * \param [in] value - Zero-terminated scalar text. A null pointer becomes an empty string.
            * \return *this.
            */
            Node& operator = (const char* value);

            /*!
            * \fn Iterator Begin()
            * \brief Returns an iterator to the first child of a sequence or map.
            * \return Begin iterator, or an empty iterator when this node has no children.
            */
            Iterator Begin();

            /*!
            * \fn ConstIterator Begin() const
            * \brief Returns a const iterator to the first child of a sequence or map.
            * \return Begin iterator, or an empty iterator when this node has no children.
            */
            ConstIterator Begin() const;

            /*!
            * \fn Iterator End()
            * \brief Returns a past-the-end iterator for a sequence or map.
            * \return End iterator, or an empty iterator when this node has no children.
            */
            Iterator End();

            /*!
            * \fn ConstIterator End() const
            * \brief Returns a past-the-end const iterator for a sequence or map.
            * \return End iterator, or an empty iterator when this node has no children.
            */
            ConstIterator End() const;

        private:
            const std::string& AsString() const;

            void* m_pImp; ///< Implementation of node class.
        };

        /*! @ingroup cpl_yaml
        * \brief Reads a YAML file and stores the document in root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] filename - Path opened as a binary input file.
        * \note Throws OperationException when the file cannot be opened, or ParsingException
        *       when the file is not well-formed YAML.
        */
        void Parse(Node& root, const char* filename);

        /*! @ingroup cpl_yaml
        * \brief Parses YAML text from stream into root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] stream - Input stream positioned at the start of the document.
        * \note Throws ParsingException when the text is not well-formed YAML.
        */
        void Parse(Node& root, std::istream& stream);

        /*! @ingroup cpl_yaml
        * \brief Parses YAML text from string into root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] string - Complete YAML document.
        * \note Throws ParsingException when the text is not well-formed YAML.
        */
        void Parse(Node& root, const std::string& string);

        /*! @ingroup cpl_yaml
        * \brief Parses YAML text from a memory buffer into root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] buffer - Pointer to size bytes of YAML text. Need not be zero-terminated.
        * \param [in] size - Number of bytes at buffer.
        * \note Throws ParsingException when the text is not well-formed YAML.
        */
        void Parse(Node& root, const char* buffer, const size_t size);

        /*! @ingroup cpl_yaml
        * \struct SerializeConfig
        * \brief Formatting options for Serialize().
        * \note SpaceIndentation must be at least 2 or Serialize() throws OperationException.
        */
        struct SerializeConfig
        {
            /*!
            * \fn SerializeConfig(const size_t spaceIndentation = 2, const size_t scalarMaxLength = 64, const bool sequenceMapNewline = false, const bool mapScalarNewline = false)
            * \brief Sets the four formatting fields.
            * \param [in] spaceIndentation - Spaces added per nesting level. Must be at least 2 when serializing.
            * \param [in] scalarMaxLength - Maximum plain-scalar length before folded style is used. 0 disables that limit.
            * \param [in] sequenceMapNewline - If true, a map that is a sequence item starts on a new line.
            * \param [in] mapScalarNewline - If true, a scalar that is a map value starts on a new line.
            */
            SerializeConfig(const size_t spaceIndentation = 2,
                const size_t scalarMaxLength = 64,
                const bool sequenceMapNewline = false,
                const bool mapScalarNewline = false);

            size_t SpaceIndentation;    ///< Number of spaces per indentation.
            size_t ScalarMaxLength;     ///< Maximum length of scalars. Serialized as folded scalars if exceeded.
            bool SequenceMapNewline;    ///< Put maps on a new line if parent node is a sequence.
            bool MapScalarNewline;      ///< Put scalars on a new line if parent node is a map.
        };

        /*! @ingroup cpl_yaml
        * \brief Writes root as YAML text to a file.
        * \param [in] root - Document tree to write.
        * \param [in] filename - Path opened for text output.
        * \param [in] config - Indentation and folding options.
        * \note Throws OperationException when the file cannot be opened or SpaceIndentation is less than 2.
        */
        void Serialize(const Node& root, const char* filename, const SerializeConfig& config = { 2, 64, false, false });

        /*! @ingroup cpl_yaml
        * \brief Writes root as YAML text to stream.
        * \param [in] root - Document tree to write.
        * \param [out] stream - Destination stream.
        * \param [in] config - Indentation and folding options.
        * \note Throws OperationException when SpaceIndentation is less than 2.
        */
        void Serialize(const Node& root, std::ostream& stream, const SerializeConfig& config = { 2, 64, false, false });

        /*! @ingroup cpl_yaml
        * \brief Writes root as YAML text into string, replacing its previous contents.
        * \param [in] root - Document tree to write.
        * \param [out] string - Destination string.
        * \param [in] config - Indentation and folding options.
        * \note Throws OperationException when SpaceIndentation is less than 2.
        */
        void Serialize(const Node& root, std::string& string, const SerializeConfig& config = { 2, 64, false, false });

        namespace Detail
        {
            CPL_INLINE String & EmptyString()
            {
                static String empty = String();
                return empty;
            }

            CPL_INLINE Node& EmptyNode()
            {
                static Node empty = Node(true);
                return empty;
            }

            class TypeImp
            {
            public:

                virtual ~TypeImp()
                {
                }

                virtual const std::string& GetData() const = 0;
                virtual bool SetData(const std::string& data) = 0;
                virtual size_t GetSize() const = 0;
                virtual Node* GetNode(const size_t index) = 0;
                virtual Node* GetNode(const std::string& key) = 0;
                virtual Node* Insert(const size_t index) = 0;
                virtual Node* PushFront() = 0;
                virtual Node* PushBack() = 0;
                virtual void Erase(const size_t index) = 0;
                virtual void Erase(const std::string& key) = 0;
            };

            //-------------------------------------------------------------------------------------

            class SequenceImp : public TypeImp
            {
            public:
                ~SequenceImp()
                {
                    for (auto it = m_Sequence.begin(); it != m_Sequence.end(); it++)
                    {
                        delete it->second;
                    }
                }

                virtual const std::string& GetData() const
                {
                    return Detail::EmptyString();
                }

                virtual bool SetData(const std::string& data)
                {
                    return false;
                }

                virtual size_t GetSize() const
                {
                    return m_Sequence.size();
                }

                virtual Node* GetNode(const size_t index)
                {
                    auto it = m_Sequence.find(index);
                    if (it != m_Sequence.end())
                    {
                        return it->second;
                    }
                    return nullptr;
                }

                virtual Node* GetNode(const std::string& key)
                {
                    return nullptr;
                }

                virtual Node* Insert(const size_t index)
                {
                    if (m_Sequence.size() == 0)
                    {
                        Node* pNode = new Node;
                        m_Sequence.insert({ 0, pNode });
                        return pNode;
                    }

                    if (index >= m_Sequence.size())
                    {
                        auto it = m_Sequence.end();
                        --it;
                        Node* pNode = new Node;
                        m_Sequence.insert({ it->first, pNode });
                        return pNode;
                    }

                    auto it = m_Sequence.cbegin();
                    while (it != m_Sequence.cend())
                    {
                        m_Sequence[it->first + 1] = it->second;

                        if (it->first == index)
                        {
                            break;
                        }
                    }

                    Node* pNode = new Node;
                    m_Sequence.insert({ index, pNode });
                    return pNode;
                }

                virtual Node* PushFront()
                {
                    for (auto it = m_Sequence.cbegin(); it != m_Sequence.cend(); it++)
                    {
                        m_Sequence[it->first + 1] = it->second;
                    }

                    Node* pNode = new Node;
                    m_Sequence.insert({ 0, pNode });
                    return pNode;
                }

                virtual Node* PushBack()
                {
                    size_t index = 0;
                    if (m_Sequence.size())
                    {
                        auto it = m_Sequence.end();
                        --it;
                        index = it->first + 1;
                    }

                    Node* pNode = new Node;
                    m_Sequence.insert({ index, pNode });
                    return pNode;
                }

                virtual void Erase(const size_t index)
                {
                    auto it = m_Sequence.find(index);
                    if (it == m_Sequence.end())
                    {
                        return;
                    }
                    delete it->second;
                    m_Sequence.erase(index);
                }

                virtual void Erase(const std::string& key)
                {
                }

                std::map<size_t, Node*> m_Sequence;
            };

            //-------------------------------------------------------------------------------------

            class MapImp : public TypeImp
            {
            public:
                ~MapImp()
                {
                    for (auto it = m_Map.begin(); it != m_Map.end(); it++)
                    {
                        delete it->second;
                    }
                }

                virtual const std::string& GetData() const
                {
                    return Detail::EmptyString();
                }

                virtual bool SetData(const std::string& data)
                {
                    return false;
                }

                virtual size_t GetSize() const
                {
                    return m_Map.size();
                }

                virtual Node* GetNode(const size_t index)
                {
                    return nullptr;
                }

                virtual Node* GetNode(const std::string& key)
                {
                    auto it = m_Map.find(key);
                    if (it == m_Map.end())
                    {
                        Node* pNode = new Node;
                        m_Map.insert({ key, pNode });
                        return pNode;
                    }
                    return it->second;
                }

                virtual Node* Insert(const size_t index)
                {
                    return nullptr;
                }

                virtual Node* PushFront()
                {
                    return nullptr;
                }

                virtual Node* PushBack()
                {
                    return nullptr;
                }

                virtual void Erase(const size_t index)
                {
                }

                virtual void Erase(const std::string& key)
                {
                    auto it = m_Map.find(key);
                    if (it == m_Map.end())
                    {
                        return;
                    }
                    delete it->second;
                    m_Map.erase(key);
                }

                std::map<std::string, Node*> m_Map;

            };

            //-------------------------------------------------------------------------------------

            class ScalarImp : public TypeImp
            {

            public:
                ~ScalarImp()
                {
                }

                virtual const std::string& GetData() const
                {
                    return m_Value;
                }

                virtual bool SetData(const std::string& data)
                {
                    m_Value = data;
                    return true;
                }

                virtual size_t GetSize() const
                {
                    return 0;
                }

                virtual Node* GetNode(const size_t index)
                {
                    return nullptr;
                }

                virtual Node* GetNode(const std::string& key)
                {
                    return nullptr;
                }

                virtual Node* Insert(const size_t index)
                {
                    return nullptr;
                }

                virtual Node* PushFront()
                {
                    return nullptr;
                }

                virtual Node* PushBack()
                {
                    return nullptr;
                }

                virtual void Erase(const size_t index)
                {
                }

                virtual void Erase(const std::string& key)
                {
                }

                std::string m_Value;
            };

            //-------------------------------------------------------------------------------------

            class NodeImp
            {

            public:

                NodeImp() :
                    m_Type(Node::None),
                    m_pImp(nullptr)
                {
                }

                ~NodeImp()
                {
                    Clear();
                }

                void Clear()
                {
                    if (m_pImp != nullptr)
                    {
                        delete m_pImp;
                        m_pImp = nullptr;
                    }
                    m_Type = Node::None;
                }

                void InitSequence()
                {
                    if (m_Type != Node::SequenceType || m_pImp == nullptr)
                    {
                        if (m_pImp)
                        {
                            delete m_pImp;
                        }
                        m_pImp = new SequenceImp;
                        m_Type = Node::SequenceType;
                    }
                }

                void InitMap()
                {
                    if (m_Type != Node::MapType || m_pImp == nullptr)
                    {
                        if (m_pImp)
                        {
                            delete m_pImp;
                        }
                        m_pImp = new MapImp;
                        m_Type = Node::MapType;
                    }
                }

                void InitScalar()
                {
                    if (m_Type != Node::ScalarType || m_pImp == nullptr)
                    {
                        if (m_pImp)
                        {
                            delete m_pImp;
                        }
                        m_pImp = new ScalarImp;
                        m_Type = Node::ScalarType;
                    }

                }

                Node::eType    m_Type;  ///< Type of node.
                TypeImp* m_pImp;  ///< Imp of type.
            };

            //-------------------------------------------------------------------------------------

            class IteratorImp
            {

            public:

                virtual ~IteratorImp()
                {
                }

                virtual Node::eType GetType() const = 0;
                virtual void InitBegin(SequenceImp* pSequenceImp) = 0;
                virtual void InitEnd(SequenceImp* pSequenceImp) = 0;
                virtual void InitBegin(MapImp* pMapImp) = 0;
                virtual void InitEnd(MapImp* pMapImp) = 0;

            };

            //-------------------------------------------------------------------------------------

            class SequenceIteratorImp : public IteratorImp
            {

            public:

                virtual Node::eType GetType() const
                {
                    return Node::SequenceType;
                }

                virtual void InitBegin(SequenceImp* pSequenceImp)
                {
                    m_Iterator = pSequenceImp->m_Sequence.begin();
                }

                virtual void InitEnd(SequenceImp* pSequenceImp)
                {
                    m_Iterator = pSequenceImp->m_Sequence.end();
                }

                virtual void InitBegin(MapImp* pMapImp)
                {
                }

                virtual void InitEnd(MapImp* pMapImp)
                {
                }

                void Copy(const SequenceIteratorImp& it)
                {
                    m_Iterator = it.m_Iterator;
                }

                std::map<size_t, Node*>::iterator m_Iterator;

            };

            //-------------------------------------------------------------------------------------

            class MapIteratorImp : public IteratorImp
            {

            public:

                virtual Node::eType GetType() const
                {
                    return Node::MapType;
                }

                virtual void InitBegin(SequenceImp* pSequenceImp)
                {
                }

                virtual void InitEnd(SequenceImp* pSequenceImp)
                {
                }

                virtual void InitBegin(MapImp* pMapImp)
                {
                    m_Iterator = pMapImp->m_Map.begin();
                }

                virtual void InitEnd(MapImp* pMapImp)
                {
                    m_Iterator = pMapImp->m_Map.end();
                }

                void Copy(const MapIteratorImp& it)
                {
                    m_Iterator = it.m_Iterator;
                }

                std::map<std::string, Node*>::iterator m_Iterator;

            };

            //-------------------------------------------------------------------------------------

            class SequenceConstIteratorImp : public IteratorImp
            {

            public:

                virtual Node::eType GetType() const
                {
                    return Node::SequenceType;
                }

                virtual void InitBegin(SequenceImp* pSequenceImp)
                {
                    m_Iterator = pSequenceImp->m_Sequence.begin();
                }

                virtual void InitEnd(SequenceImp* pSequenceImp)
                {
                    m_Iterator = pSequenceImp->m_Sequence.end();
                }

                virtual void InitBegin(MapImp* pMapImp)
                {
                }

                virtual void InitEnd(MapImp* pMapImp)
                {
                }

                void Copy(const SequenceConstIteratorImp& it)
                {
                    m_Iterator = it.m_Iterator;
                }

                std::map<size_t, Node*>::const_iterator m_Iterator;

            };

            //-------------------------------------------------------------------------------------

            class MapConstIteratorImp : public IteratorImp
            {

            public:

                virtual Node::eType GetType() const
                {
                    return Node::MapType;
                }

                virtual void InitBegin(SequenceImp* pSequenceImp)
                {
                }

                virtual void InitEnd(SequenceImp* pSequenceImp)
                {
                }

                virtual void InitBegin(MapImp* pMapImp)
                {
                    m_Iterator = pMapImp->m_Map.begin();
                }

                virtual void InitEnd(MapImp* pMapImp)
                {
                    m_Iterator = pMapImp->m_Map.end();
                }

                void Copy(const MapConstIteratorImp& it)
                {
                    m_Iterator = it.m_Iterator;
                }

                std::map<std::string, Node*>::const_iterator m_Iterator;
            };

            CPL_INLINE String ErrorInvalidCharacter() { return "Invalid character found."; }
            CPL_INLINE String ErrorKeyMissing() { return "Missing key."; }
            CPL_INLINE String ErrorKeyIncorrect() { return "Incorrect key."; }
            CPL_INLINE String ErrorValueIncorrect() { return "Incorrect value."; }
            CPL_INLINE String ErrorTabInOffset() { return "Tab found in offset."; }
            CPL_INLINE String ErrorBlockSequenceNotAllowed() { return "Sequence entries are not allowed in this context."; }
            CPL_INLINE String ErrorUnexpectedDocumentEnd() { return "Unexpected document end."; }
            CPL_INLINE String ErrorDiffEntryNotAllowed() { return "Different entry is not allowed in this context."; }
            CPL_INLINE String ErrorIncorrectOffset() { return "Incorrect offset."; }
            CPL_INLINE String ErrorSequenceError() { return "Error in sequence node."; }
            CPL_INLINE String ErrorCannotOpenFile() { return "Cannot open file."; }
            CPL_INLINE String ErrorIndentation() { return "Space indentation is less than 2."; }
            CPL_INLINE String ErrorInvalidBlockScalar() { return "Invalid block scalar."; }
            CPL_INLINE String ErrorInvalidQuote() { return "Invalid quote."; }
        }

        //-----------------------------------------------------------------------------------------

        class ReaderLine;

        std::string ExceptionMessage(const std::string& message, ReaderLine& line);
        std::string ExceptionMessage(const std::string& message, ReaderLine& line, const size_t errorPos);
        std::string ExceptionMessage(const std::string& message, const size_t errorLine, const size_t errorPos);
        std::string ExceptionMessage(const std::string& message, const size_t errorLine, const std::string& data);

        bool FindQuote(const std::string& input, size_t& start, size_t& end, size_t searchPos = 0);
        size_t FindNotCited(const std::string& input, char token, size_t& preQuoteCount);
        size_t FindNotCited(const std::string& input, char token);
        bool ValidateQuote(const std::string& input);
        void CopyNode(const Node& from, Node& to);
        bool ShouldBeCited(const std::string& key);
        void AddEscapeTokens(std::string& input, const std::string& tokens);
        void RemoveAllEscapeTokens(std::string& input);

        //-----------------------------------------------------------------------------------------

        inline Iterator::Iterator() :
            m_Type(None),
            m_pImp(nullptr)
        {
        }

        inline Iterator::~Iterator()
        {
            if (m_pImp)
            {
                switch (m_Type)
                {
                case SequenceType:
                    delete static_cast<Detail::SequenceIteratorImp*>(m_pImp);
                    break;
                case MapType:
                    delete static_cast<Detail::MapIteratorImp*>(m_pImp);
                    break;
                default:
                    break;
                }

            }
        }

        inline Iterator::Iterator(const Iterator& it) :
            m_Type(None),
            m_pImp(nullptr)
        {
            *this = it;
        }

        inline Iterator& Iterator::operator = (const Iterator& it)
        {
            if (m_pImp)
            {
                switch (m_Type)
                {
                case SequenceType:
                    delete static_cast<Detail::SequenceIteratorImp*>(m_pImp);
                    break;
                case MapType:
                    delete static_cast<Detail::MapIteratorImp*>(m_pImp);
                    break;
                default:
                    break;
                }
                m_pImp = nullptr;
                m_Type = None;
            }

            Detail::IteratorImp* pNewImp = nullptr;

            switch (it.m_Type)
            {
            case SequenceType:
                m_Type = SequenceType;
                pNewImp = new Detail::SequenceIteratorImp;
                static_cast<Detail::SequenceIteratorImp*>(pNewImp)->m_Iterator = static_cast<Detail::SequenceIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            case MapType:
                m_Type = MapType;
                pNewImp = new Detail::MapIteratorImp;
                static_cast<Detail::MapIteratorImp*>(pNewImp)->m_Iterator = static_cast<Detail::MapIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            default:
                break;
            }

            m_pImp = pNewImp;
            return *this;
        }

        inline std::pair<const std::string&, Node&> Iterator::operator *()
        {
            switch (m_Type)
            {
            case SequenceType:
                return { String(), *(static_cast<Detail::SequenceIteratorImp*>(m_pImp)->m_Iterator->second) };
                break;
            case MapType:
                return { static_cast<Detail::MapIteratorImp*>(m_pImp)->m_Iterator->first,
                        *(static_cast<Detail::MapIteratorImp*>(m_pImp)->m_Iterator->second) };
                break;
            default:
                break;
            }

            return { Detail::EmptyString(), Detail::EmptyNode() };
        }

        inline Iterator& Iterator::operator ++ (int dummy)
        {
            switch (m_Type)
            {
            case SequenceType:
                static_cast<Detail::SequenceIteratorImp*>(m_pImp)->m_Iterator++;
                break;
            case MapType:
                static_cast<Detail::MapIteratorImp*>(m_pImp)->m_Iterator++;
                break;
            default:
                break;
            }
            return *this;
        }

        inline Iterator& Iterator::operator -- (int dummy)
        {
            switch (m_Type)
            {
            case SequenceType:
                static_cast<Detail::SequenceIteratorImp*>(m_pImp)->m_Iterator--;
                break;
            case MapType:
                static_cast<Detail::MapIteratorImp*>(m_pImp)->m_Iterator--;
                break;
            default:
                break;
            }
            return *this;
        }

        inline bool Iterator::operator == (const Iterator& it)
        {
            if (m_Type != it.m_Type)
            {
                return false;
            }

            switch (m_Type)
            {
            case SequenceType:
                return static_cast<Detail::SequenceIteratorImp*>(m_pImp)->m_Iterator == static_cast<Detail::SequenceIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            case MapType:
                return static_cast<Detail::MapIteratorImp*>(m_pImp)->m_Iterator == static_cast<Detail::MapIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            default:
                break;
            }

            return false;
        }

        inline bool Iterator::operator != (const Iterator& it)
        {
            return !(*this == it);
        }

        //-----------------------------------------------------------------------------------------

        inline ConstIterator::ConstIterator() :
            m_Type(None),
            m_pImp(nullptr)
        {
        }

        inline ConstIterator::~ConstIterator()
        {
            if (m_pImp)
            {
                switch (m_Type)
                {
                case SequenceType:
                    delete static_cast<Detail::SequenceConstIteratorImp*>(m_pImp);
                    break;
                case MapType:
                    delete static_cast<Detail::MapConstIteratorImp*>(m_pImp);
                    break;
                default:
                    break;
                }

            }
        }

        inline ConstIterator::ConstIterator(const ConstIterator& it) :
            m_Type(None),
            m_pImp(nullptr)
        {
            *this = it;
        }

        inline ConstIterator& ConstIterator::operator = (const ConstIterator& it)
        {
            if (m_pImp)
            {
                switch (m_Type)
                {
                case SequenceType:
                    delete static_cast<Detail::SequenceConstIteratorImp*>(m_pImp);
                    break;
                case MapType:
                    delete static_cast<Detail::MapConstIteratorImp*>(m_pImp);
                    break;
                default:
                    break;
                }
                m_pImp = nullptr;
                m_Type = None;
            }

            Detail::IteratorImp* pNewImp = nullptr;

            switch (it.m_Type)
            {
            case SequenceType:
                m_Type = SequenceType;
                pNewImp = new Detail::SequenceConstIteratorImp;
                static_cast<Detail::SequenceConstIteratorImp*>(pNewImp)->m_Iterator = static_cast<Detail::SequenceConstIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            case MapType:
                m_Type = MapType;
                pNewImp = new Detail::MapConstIteratorImp;
                static_cast<Detail::MapConstIteratorImp*>(pNewImp)->m_Iterator = static_cast<Detail::MapConstIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            default:
                break;
            }

            m_pImp = pNewImp;
            return *this;
        }

        inline std::pair<const std::string&, const Node&> ConstIterator::operator *()
        {
            switch (m_Type)
            {
            case SequenceType:
                return { String(), *(static_cast<Detail::SequenceConstIteratorImp*>(m_pImp)->m_Iterator->second) };
                break;
            case MapType:
                return { static_cast<Detail::MapConstIteratorImp*>(m_pImp)->m_Iterator->first,
                        *(static_cast<Detail::MapConstIteratorImp*>(m_pImp)->m_Iterator->second) };
                break;
            default:
                break;
            }

            return { Detail::EmptyString(), Detail::EmptyNode() };
        }

        inline ConstIterator& ConstIterator::operator ++ (int dummy)
        {
            switch (m_Type)
            {
            case SequenceType:
                static_cast<Detail::SequenceConstIteratorImp*>(m_pImp)->m_Iterator++;
                break;
            case MapType:
                static_cast<Detail::MapConstIteratorImp*>(m_pImp)->m_Iterator++;
                break;
            default:
                break;
            }
            return *this;
        }

        inline ConstIterator& ConstIterator::operator -- (int dummy)
        {
            switch (m_Type)
            {
            case SequenceType:
                static_cast<Detail::SequenceConstIteratorImp*>(m_pImp)->m_Iterator--;
                break;
            case MapType:
                static_cast<Detail::MapConstIteratorImp*>(m_pImp)->m_Iterator--;
                break;
            default:
                break;
            }
            return *this;
        }

        inline bool ConstIterator::operator == (const ConstIterator& it)
        {
            if (m_Type != it.m_Type)
            {
                return false;
            }

            switch (m_Type)
            {
            case SequenceType:
                return static_cast<Detail::SequenceConstIteratorImp*>(m_pImp)->m_Iterator == static_cast<Detail::SequenceConstIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            case MapType:
                return static_cast<Detail::MapConstIteratorImp*>(m_pImp)->m_Iterator == static_cast<Detail::MapConstIteratorImp*>(it.m_pImp)->m_Iterator;
                break;
            default:
                break;
            }

            return false;
        }

        inline bool ConstIterator::operator != (const ConstIterator& it)
        {
            return !(*this == it);
        }

        //-----------------------------------------------------------------------------------------

        inline Node::Node(bool none) 
            : m_pImp(new Detail::NodeImp())
        {
            if (none)
                Clear();
        }

        inline Node::Node(const Node& node)
            : Node()
        {
            *this = node;
        }

        inline Node::Node(const std::string& value) :
            Node()
        {
            *this = value;
        }

        inline Node::Node(const char* value) :
            Node()
        {
            *this = value;
        }

        inline Node::~Node()
        {
            delete ((Detail::NodeImp*)m_pImp);
        }

        inline Node::eType Node::Type() const
        {
            return ((Detail::NodeImp*)m_pImp)->m_Type;
        }

        inline bool Node::IsNone() const
        {
            return ((Detail::NodeImp*)m_pImp)->m_Type == Node::None;
        }

        inline bool Node::IsSequence() const
        {
            return ((Detail::NodeImp*)m_pImp)->m_Type == Node::SequenceType;
        }

        inline bool Node::IsMap() const
        {
            return ((Detail::NodeImp*)m_pImp)->m_Type == Node::MapType;
        }

        inline bool Node::IsScalar() const
        {
            return ((Detail::NodeImp*)m_pImp)->m_Type == Node::ScalarType;
        }

        inline void Node::Clear()
        {
            ((Detail::NodeImp*)m_pImp)->Clear();
        }

        inline size_t Node::Size() const
        {
            if (((Detail::NodeImp*)m_pImp)->m_pImp == nullptr)
                return 0;
            return ((Detail::NodeImp*)m_pImp)->m_pImp->GetSize();
        }

        inline Node& Node::Insert(const size_t index)
        {
            ((Detail::NodeImp*)m_pImp)->InitSequence();
            return *((Detail::NodeImp*)m_pImp)->m_pImp->Insert(index);
        }

        inline Node& Node::PushFront()
        {
            ((Detail::NodeImp*)m_pImp)->InitSequence();
            return *((Detail::NodeImp*)m_pImp)->m_pImp->PushFront();
        }

        inline Node& Node::PushBack()
        {
            ((Detail::NodeImp*)m_pImp)->InitSequence();
            return *((Detail::NodeImp*)m_pImp)->m_pImp->PushBack();
        }

        inline Node& Node::operator[](const size_t index)
        {
            ((Detail::NodeImp*)m_pImp)->InitSequence();
            Node* pNode = ((Detail::NodeImp*)m_pImp)->m_pImp->GetNode(index);
            if (pNode == nullptr)
                return Detail::EmptyNode();
            return *pNode;
        }

        inline Node& Node::operator[](const std::string& key)
        {
            ((Detail::NodeImp*)m_pImp)->InitMap();
            return *((Detail::NodeImp*)m_pImp)->m_pImp->GetNode(key);
        }

        inline void Node::Erase(const size_t index)
        {
            if (((Detail::NodeImp*)m_pImp)->m_pImp == nullptr || ((Detail::NodeImp*)m_pImp)->m_Type != Node::SequenceType)
                return;
            return ((Detail::NodeImp*)m_pImp)->m_pImp->Erase(index);
        }

        inline void Node::Erase(const std::string& key)
        {
            if (((Detail::NodeImp*)m_pImp)->m_pImp == nullptr || ((Detail::NodeImp*)m_pImp)->m_Type != Node::MapType)
                return;
            return ((Detail::NodeImp*)m_pImp)->m_pImp->Erase(key);
        }

        inline Node& Node::operator = (const Node& node)
        {
            ((Detail::NodeImp*)m_pImp)->Clear();
            CopyNode(node, *this);
            return *this;
        }

        inline Node& Node::operator = (const std::string& value)
        {
            ((Detail::NodeImp*)m_pImp)->InitScalar();
            ((Detail::NodeImp*)m_pImp)->m_pImp->SetData(value);
            return *this;
        }

        inline Node& Node::operator = (const char* value)
        {
            ((Detail::NodeImp*)m_pImp)->InitScalar();
            ((Detail::NodeImp*)m_pImp)->m_pImp->SetData(value ? std::string(value) : "");
            return *this;
        }

        inline Iterator Node::Begin()
        {
            Iterator it;
            if (((Detail::NodeImp*)m_pImp)->m_pImp != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;
                switch (((Detail::NodeImp*)m_pImp)->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = Iterator::SequenceType;
                    pItImp = new Detail::SequenceIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::SequenceImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                case Node::MapType:
                    it.m_Type = Iterator::MapType;
                    pItImp = new Detail::MapIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::MapImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                default:
                    break;
                }
                it.m_pImp = pItImp;
            }
            return it;
        }

        inline ConstIterator Node::Begin() const
        {
            ConstIterator it;
            if (((Detail::NodeImp*)m_pImp)->m_pImp != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;
                switch (((Detail::NodeImp*)m_pImp)->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = ConstIterator::SequenceType;
                    pItImp = new Detail::SequenceConstIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::SequenceImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                case Node::MapType:
                    it.m_Type = ConstIterator::MapType;
                    pItImp = new Detail::MapConstIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::MapImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                default:
                    break;
                }
                it.m_pImp = pItImp;
            }
            return it;
        }

        inline Iterator Node::End()
        {
            Iterator it;
            if (((Detail::NodeImp*)m_pImp)->m_pImp != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;
                switch (((Detail::NodeImp*)m_pImp)->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = Iterator::SequenceType;
                    pItImp = new Detail::SequenceIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::SequenceImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                case Node::MapType:
                    it.m_Type = Iterator::MapType;
                    pItImp = new Detail::MapIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::MapImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                default:
                    break;
                }
                it.m_pImp = pItImp;
            }
            return it;
        }

        inline ConstIterator Node::End() const
        {
            ConstIterator it;
            if (((Detail::NodeImp*)m_pImp)->m_pImp != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;

                switch (((Detail::NodeImp*)m_pImp)->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = ConstIterator::SequenceType;
                    pItImp = new Detail::SequenceConstIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::SequenceImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                case Node::MapType:
                    it.m_Type = ConstIterator::MapType;
                    pItImp = new Detail::MapConstIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::MapImp*>(((Detail::NodeImp*)m_pImp)->m_pImp));
                    break;
                default:
                    break;
                }
                it.m_pImp = pItImp;
            }

            return it;
        }

        inline const std::string& Node::AsString() const
        {
            if (((Detail::NodeImp*)m_pImp)->m_pImp == nullptr)
                return Detail::EmptyString();
            return ((Detail::NodeImp*)m_pImp)->m_pImp->GetData();
        }

        //-----------------------------------------------------------------------------------------

        class ReaderLine
        {
        public:
            ReaderLine(const std::string& data = "", const size_t no = 0, 
                const size_t offset = 0, const Node::eType type = Node::None,
                const unsigned char flags = 0) 
                : Data(data)
                , No(no)
                , Offset(offset)
                , Type(type)
                , Flags(flags)
                , NextLine(nullptr)
            {
            }

            enum eFlag
            {
                LiteralScalarFlag,
                FoldedScalarFlag,
                ScalarNewlineFlag
            };

            void SetFlag(const eFlag flag)
            {
                Flags |= FlagMask(static_cast<size_t>(flag));
            }

            void SetFlags(const unsigned char flags)
            {
                Flags |= flags;
            }

            void UnsetFlag(const eFlag flag)
            {
                Flags &= ~FlagMask(static_cast<size_t>(flag));
            }

            void UnsetFlags(const unsigned char flags)
            {
                Flags &= ~flags;
            }

            bool GetFlag(const eFlag flag) const
            {
                return (Flags & FlagMask(static_cast<size_t>(flag))) != 0;
            }

            void CopyScalarFlags(ReaderLine* from)
            {
                if (from == nullptr)
                {
                    return;
                }
                unsigned char newFlags = from->Flags & (FlagMask(0) | FlagMask(1) | FlagMask(2));
                Flags |= newFlags;
            }

            static CPL_INLINE const unsigned char FlagMask(size_t index)
            {
                static const unsigned char flagMask[3] = { 0x01, 0x02, 0x04 };
                return flagMask[index];
            }

            std::string Data;
            size_t No;
            size_t Offset;
            Node::eType Type;
            unsigned char Flags;
            ReaderLine* NextLine;
        };

        //-----------------------------------------------------------------------------------------

        class ParseImp
        {
        public:
            ParseImp()
            {
            }

            ~ParseImp()
            {
                ClearLines();
            }

            void Parse(Node& root, std::istream& stream)
            {
                try
                {
                    root.Clear();
                    ReadLines(stream);
                    PostProcessLines();
                    //Print();
                    ParseRoot(root);
                }
                catch (Exception e)
                {
                    root.Clear();
                    throw;
                }
            }

        private:

            ParseImp(const ParseImp& copy)
            {
            }

            void ReadLines(std::istream& stream)
            {
                std::string     line = "";
                size_t          lineNo = 0;
                bool            documentStartFound = false;
                bool            foundFirstNotEmpty = false;
                std::streampos  streamPos = 0;

                // Read all lines, as long as the stream is ok.
                while (!stream.eof() && !stream.fail())
                {
                    // Read line
                    streamPos = stream.tellg();
                    std::getline(stream, line);
                    lineNo++;

                    // Remove comment
                    const size_t commentPos = FindNotCited(line, '#');
                    if (commentPos != std::string::npos)
                    {
                        line.resize(commentPos);
                    }

                    // Start of document.
                    if (documentStartFound == false && line == "---")
                    {
                        // Erase all lines before this line.
                        ClearLines();
                        documentStartFound = true;
                        continue;
                    }

                    // End of document.
                    if (line == "...")
                    {
                        break;
                    }
                    else if (line == "---")
                    {
                        stream.seekg(streamPos);
                        break;
                    }

                    // Remove trailing return.
                    if (line.size())
                    {
                        if (line[line.size() - 1] == '\r')
                        {
                            line.resize(line.size() - 1);
                        }
                    }

                    // Validate characters.
                    for (size_t i = 0; i < line.size(); i++)
                    {
                        if (line[i] != '\t' && (line[i] < 32 || line[i] > 125))
                        {
                            throw ParsingException(ExceptionMessage(Detail::ErrorInvalidCharacter(), lineNo, i + 1));
                        }
                    }

                    // Validate tabs
                    const size_t firstTabPos = line.find_first_of('\t');
                    size_t       startOffset = line.find_first_not_of(" \t");

                    // Make sure no tabs are in the very front.
                    if (startOffset != std::string::npos)
                    {
                        if (firstTabPos < startOffset)
                        {
                            throw ParsingException(ExceptionMessage(Detail::ErrorTabInOffset(), lineNo, firstTabPos));
                        }

                        // Remove front spaces.
                        line = line.substr(startOffset);
                    }
                    else
                    {
                        startOffset = 0;
                        line = "";
                    }

                    // Add line.
                    if (foundFirstNotEmpty == false)
                    {
                        if (line.size())
                        {
                            foundFirstNotEmpty = true;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    ReaderLine* pLine = new ReaderLine(line, lineNo, startOffset);
                    m_Lines.push_back(pLine);
                }
            }

            void PostProcessLines()
            {
                for (auto it = m_Lines.begin(); it != m_Lines.end();)
                {
                    if (PostProcessSequenceLine(it))
                        continue;
                    if (PostProcessMappingLine(it))
                        continue;
                    PostProcessScalarLine(it);
                }

                if (m_Lines.size())
                {
                    if (m_Lines.back()->Type != Node::ScalarType)
                        throw ParsingException(ExceptionMessage(Detail::ErrorUnexpectedDocumentEnd(), *m_Lines.back()));
                    if (m_Lines.size() > 1)
                    {
                        auto prevEnd = m_Lines.end();
                        --prevEnd;

                        for (auto it = m_Lines.begin(); it != prevEnd; it++)
                        {
                            auto nextIt = it;
                            ++nextIt;

                            (*it)->NextLine = *nextIt;
                        }
                    }
                }
            }

            bool PostProcessSequenceLine(std::list<ReaderLine*>::iterator& it)
            {
                ReaderLine* pLine = *it;

                // Sequence split
                if (IsSequenceStart(pLine->Data) == false)
                {
                    return false;
                }

                pLine->Type = Node::SequenceType;

                ClearTrailingEmptyLines(++it);

                const size_t valueStart = pLine->Data.find_first_not_of(" \t", 1);
                if (valueStart == std::string::npos)
                {
                    return true;
                }

                // Create new line and insert
                std::string newLine = pLine->Data.substr(valueStart);
                it = m_Lines.insert(it, new ReaderLine(newLine, pLine->No, pLine->Offset + valueStart));
                pLine->Data = "";

                return false;
            }

            bool PostProcessMappingLine(std::list<ReaderLine*>::iterator& it)
            {
                ReaderLine* pLine = *it;

                // Find map key.
                size_t preKeyQuotes = 0;
                size_t tokenPos = FindNotCited(pLine->Data, ':', preKeyQuotes);
                if (tokenPos == std::string::npos)
                {
                    return false;
                }
                if (preKeyQuotes > 1)
                {
                    throw ParsingException(ExceptionMessage(Detail::ErrorKeyIncorrect(), *pLine));
                }

                pLine->Type = Node::MapType;

                // Get key
                std::string key = pLine->Data.substr(0, tokenPos);
                const size_t keyEnd = key.find_last_not_of(" \t");
                if (keyEnd == std::string::npos)
                {
                    throw ParsingException(ExceptionMessage(Detail::ErrorKeyMissing(), *pLine));
                }
                key.resize(keyEnd + 1);

                // Handle cited key.
                if (preKeyQuotes == 1)
                {
                    if (key.front() != '"' || key.back() != '"')
                    {
                        throw ParsingException(ExceptionMessage(Detail::ErrorKeyIncorrect(), *pLine));
                    }

                    key = key.substr(1, key.size() - 2);
                }
                RemoveAllEscapeTokens(key);

                // Get value
                std::string value = "";
                size_t valueStart = std::string::npos;
                if (tokenPos + 1 != pLine->Data.size())
                {
                    valueStart = pLine->Data.find_first_not_of(" \t", tokenPos + 1);
                    if (valueStart != std::string::npos)
                    {
                        value = pLine->Data.substr(valueStart);
                    }
                }

                // Make sure the value is not a sequence start.
                if (IsSequenceStart(value) == true)
                {
                    throw ParsingException(ExceptionMessage(Detail::ErrorBlockSequenceNotAllowed(), *pLine, valueStart));
                }

                pLine->Data = key;


                // Remove all empty lines after map key.
                ClearTrailingEmptyLines(++it);

                // Add new empty line?
                size_t newLineOffset = valueStart;
                if (newLineOffset == std::string::npos)
                {
                    if (it != m_Lines.end() && (*it)->Offset > pLine->Offset)
                    {
                        return true;
                    }

                    newLineOffset = tokenPos + 2;
                }
                else
                {
                    newLineOffset += pLine->Offset;
                }

                // Add new line with value.
                unsigned char dummyBlockFlags = 0;
                if (IsBlockScalar(value, pLine->No, dummyBlockFlags) == true)
                {
                    newLineOffset = pLine->Offset;
                }
                ReaderLine* pNewLine = new ReaderLine(value, pLine->No, newLineOffset, Node::ScalarType);
                it = m_Lines.insert(it, pNewLine);

                // Return false in order to handle next line(scalar value).
                return false;
            }

            void PostProcessScalarLine(std::list<ReaderLine*>::iterator& it)
            {
                ReaderLine* pLine = *it;
                pLine->Type = Node::ScalarType;

                size_t parentOffset = pLine->Offset;
                if (pLine != m_Lines.front())
                {
                    std::list<ReaderLine*>::iterator lastIt = it;
                    --lastIt;
                    parentOffset = (*lastIt)->Offset;
                }

                std::list<ReaderLine*>::iterator lastNotEmpty = it++;

                // Find last empty lines
                while (it != m_Lines.end())
                {
                    pLine = *it;
                    pLine->Type = Node::ScalarType;
                    if (pLine->Data.size())
                    {
                        if (pLine->Offset <= parentOffset)
                        {
                            break;
                        }
                        else
                        {
                            lastNotEmpty = it;
                        }
                    }
                    ++it;
                }

                ClearTrailingEmptyLines(++lastNotEmpty);
            }

            void ParseRoot(Node& root)
            {
                // Get first line and start type.
                auto it = m_Lines.begin();
                if (it == m_Lines.end())
                {
                    return;
                }
                Node::eType type = (*it)->Type;
                ReaderLine* pLine = *it;

                // Handle next line.
                switch (type)
                {
                case Node::SequenceType:
                    ParseSequence(root, it);
                    break;
                case Node::MapType:
                    ParseMap(root, it);
                    break;
                case Node::ScalarType:
                    ParseScalar(root, it);
                    break;
                default:
                    break;
                }

                if (it != m_Lines.end())
                {
                    throw InternalException(ExceptionMessage(Detail::ErrorUnexpectedDocumentEnd(), *pLine));
                }

            }

            void ParseSequence(Node& node, std::list<ReaderLine*>::iterator& it)
            {
                ReaderLine* pNextLine = nullptr;
                while (it != m_Lines.end())
                {
                    ReaderLine* pLine = *it;
                    Node& childNode = node.PushBack();

                    // Move to next line, error check.
                    ++it;
                    if (it == m_Lines.end())
                    {
                        throw InternalException(ExceptionMessage(Detail::ErrorUnexpectedDocumentEnd(), *pLine));
                    }

                    // Handle value of map
                    Node::eType valueType = (*it)->Type;
                    switch (valueType)
                    {
                    case Node::SequenceType:
                        ParseSequence(childNode, it);
                        break;
                    case Node::MapType:
                        ParseMap(childNode, it);
                        break;
                    case Node::ScalarType:
                        ParseScalar(childNode, it);
                        break;
                    default:
                        break;
                    }

                    if (it == m_Lines.end() || ((pNextLine = *it)->Offset < pLine->Offset))
                    {
                        break;
                    }
                    if (pNextLine->Offset > pLine->Offset)
                    {
                        throw ParsingException(ExceptionMessage(Detail::ErrorIncorrectOffset(), *pNextLine));
                    }
                    if (pNextLine->Type != Node::SequenceType)
                    {
                        throw InternalException(ExceptionMessage(Detail::ErrorDiffEntryNotAllowed(), *pNextLine));
                    }
                }
            }

            void ParseMap(Node& node, std::list<ReaderLine*>::iterator& it)
            {
                ReaderLine* pNextLine = nullptr;
                while (it != m_Lines.end())
                {
                    ReaderLine* pLine = *it;
                    Node& childNode = node[pLine->Data];

                    // Move to next line, error check.
                    ++it;
                    if (it == m_Lines.end())
                    {
                        throw InternalException(ExceptionMessage(Detail::ErrorUnexpectedDocumentEnd(), *pLine));
                    }

                    // Handle value of map
                    Node::eType valueType = (*it)->Type;
                    switch (valueType)
                    {
                    case Node::SequenceType:
                        ParseSequence(childNode, it);
                        break;
                    case Node::MapType:
                        ParseMap(childNode, it);
                        break;
                    case Node::ScalarType:
                        ParseScalar(childNode, it);
                        break;
                    default:
                        break;
                    }

                    if (it == m_Lines.end() || ((pNextLine = *it)->Offset < pLine->Offset))
                    {
                        break;
                    }
                    if (pNextLine->Offset > pLine->Offset)
                    {
                        throw ParsingException(ExceptionMessage(Detail::ErrorIncorrectOffset(), *pNextLine));
                    }
                    if (pNextLine->Type != pLine->Type)
                    {
                        throw InternalException(ExceptionMessage(Detail::ErrorDiffEntryNotAllowed(), *pNextLine));
                    }
                }
            }

            void ParseScalar(Node& node, std::list<ReaderLine*>::iterator& it)
            {
                std::string data = "";
                ReaderLine* pFirstLine = *it;
                ReaderLine* pLine = *it;

                // Check if current line is a block scalar.
                unsigned char blockFlags = 0;
                bool isBlockScalar = IsBlockScalar(pLine->Data, pLine->No, blockFlags);
                const bool newLineFlag = static_cast<bool>(blockFlags & ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::ScalarNewlineFlag)));
                const bool foldedFlag = static_cast<bool>(blockFlags & ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::FoldedScalarFlag)));
                const bool literalFlag = static_cast<bool>(blockFlags & ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::LiteralScalarFlag)));
                size_t parentOffset = 0;

                // Find parent offset
                if (it != m_Lines.begin())
                {
                    std::list<ReaderLine*>::iterator parentIt = it;
                    --parentIt;
                    parentOffset = (*parentIt)->Offset;
                }

                // Move to next iterator/line if current line is a block scalar.
                if (isBlockScalar)
                {
                    ++it;
                    if (it == m_Lines.end() || (pLine = *it)->Type != Node::ScalarType)
                    {
                        return;
                    }
                }

                // Not a block scalar, cut end spaces/tabs
                if (isBlockScalar == false)
                {
                    while (1)
                    {
                        pLine = *it;

                        if (parentOffset != 0 && pLine->Offset <= parentOffset)
                        {
                            throw ParsingException(ExceptionMessage(Detail::ErrorIncorrectOffset(), *pLine));
                        }

                        const size_t endOffset = pLine->Data.find_last_not_of(" \t");
                        if (endOffset == std::string::npos)
                        {
                            data += "\n";
                        }
                        else
                        {
                            data += pLine->Data.substr(0, endOffset + 1);
                        }

                        // Move to next line
                        ++it;
                        if (it == m_Lines.end() || (*it)->Type != Node::ScalarType)
                        {
                            break;
                        }

                        data += " ";
                    }

                    if (ValidateQuote(data) == false)
                    {
                        throw ParsingException(ExceptionMessage(Detail::ErrorInvalidQuote(), *pFirstLine));
                    }
                }
                // Block scalar
                else
                {
                    pLine = *it;
                    size_t blockOffset = pLine->Offset;
                    if (blockOffset <= parentOffset)
                    {
                        throw ParsingException(ExceptionMessage(Detail::ErrorIncorrectOffset(), *pLine));
                    }

                    bool addedSpace = false;
                    while (it != m_Lines.end() && (*it)->Type == Node::ScalarType)
                    {
                        pLine = *it;

                        const size_t endOffset = pLine->Data.find_last_not_of(" \t");
                        if (endOffset != std::string::npos && pLine->Offset < blockOffset)
                        {
                            throw ParsingException(ExceptionMessage(Detail::ErrorIncorrectOffset(), *pLine));
                        }

                        if (endOffset == std::string::npos)
                        {
                            if (addedSpace)
                            {
                                data[data.size() - 1] = '\n';
                                addedSpace = false;
                            }
                            else
                            {
                                data += "\n";
                            }

                            ++it;
                            continue;
                        }
                        else
                        {
                            if (blockOffset != pLine->Offset && foldedFlag)
                            {
                                if (addedSpace)
                                {
                                    data[data.size() - 1] = '\n';
                                    addedSpace = false;
                                }
                                else
                                {
                                    data += "\n";
                                }
                            }
                            data += std::string(pLine->Offset - blockOffset, ' ');
                            data += pLine->Data;
                        }

                        // Move to next line
                        ++it;
                        if (it == m_Lines.end() || (*it)->Type != Node::ScalarType)
                        {
                            if (newLineFlag)
                            {
                                data += "\n";
                            }
                            break;
                        }

                        if (foldedFlag)
                        {
                            data += " ";
                            addedSpace = true;
                        }
                        else if (literalFlag && endOffset != std::string::npos)
                        {
                            data += "\n";
                        }
                    }
                }

                if (data.size() && (data[0] == '"' || data[0] == '\''))
                {
                    data = data.substr(1, data.size() - 2);
                }

                node = data;
            }

            void Print()
            {
                for (auto it = m_Lines.begin(); it != m_Lines.end(); it++)
                {

                    ReaderLine* pLine = *it;

                    // Print type
                    if (pLine->Type == Node::SequenceType)
                    {
                        std::cout << "seq ";
                    }
                    else if (pLine->Type == Node::MapType)
                    {
                        std::cout << "map ";
                    }
                    else if (pLine->Type == Node::ScalarType)
                    {
                        std::cout << "sca ";
                    }
                    else
                    {
                        std::cout << "    ";
                    }

                    // Print flags
                    if (pLine->GetFlag(ReaderLine::FoldedScalarFlag))
                    {
                        std::cout << "f";
                    }
                    else
                    {
                        std::cout << "-";
                    }
                    if (pLine->GetFlag(ReaderLine::LiteralScalarFlag))
                    {
                        std::cout << "l";
                    }
                    else
                    {
                        std::cout << "-";
                    }
                    if (pLine->GetFlag(ReaderLine::ScalarNewlineFlag))
                    {
                        std::cout << "n";
                    }
                    else
                    {
                        std::cout << "-";
                    }
                    if (pLine->NextLine == nullptr)
                    {
                        std::cout << "e";
                    }
                    else
                    {
                        std::cout << "-";
                    }


                    std::cout << "| ";
                    std::cout << pLine->No << " ";
                    std::cout << std::string(pLine->Offset, ' ');

                    if (pLine->Type == Node::ScalarType)
                    {
                        std::string scalarValue = pLine->Data;
                        for (size_t i = 0; (i = scalarValue.find("\n", i)) != std::string::npos;)
                        {
                            scalarValue.replace(i, 1, "\\n");
                            i += 2;
                        }
                        std::cout << scalarValue << std::endl;
                    }
                    else if (pLine->Type == Node::MapType)
                    {
                        std::cout << pLine->Data + ":" << std::endl;
                    }
                    else if (pLine->Type == Node::SequenceType)
                    {
                        std::cout << "-" << std::endl;
                    }
                    else
                    {
                        std::cout << "> UNKOWN TYPE <" << std::endl;
                    }
                }
            }

            void ClearLines()
            {
                for (auto it = m_Lines.begin(); it != m_Lines.end(); it++)
                {
                    delete* it;
                }
                m_Lines.clear();
            }

            void ClearTrailingEmptyLines(std::list<ReaderLine*>::iterator& it)
            {
                while (it != m_Lines.end())
                {
                    ReaderLine* pLine = *it;
                    if (pLine->Data.size() == 0)
                    {
                        delete* it;
                        it = m_Lines.erase(it);
                    }
                    else
                    {
                        return;
                    }

                }
            }

            static bool IsSequenceStart(const std::string& data)
            {
                if (data.size() == 0 || data[0] != '-')
                {
                    return false;
                }

                if (data.size() >= 2 && data[1] != ' ')
                {
                    return false;
                }

                return true;
            }

            static bool IsBlockScalar(const std::string& data, const size_t line, unsigned char& flags)
            {
                flags = 0;
                if (data.size() == 0)
                {
                    return false;
                }

                if (data[0] == '|')
                {
                    if (data.size() >= 2)
                    {
                        if (data[1] != '-' && data[1] != ' ' && data[1] != '\t')
                        {
                            throw ParsingException(ExceptionMessage(Detail::ErrorInvalidBlockScalar(), line, data));
                        }
                    }
                    else
                    {
                        flags |= ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::ScalarNewlineFlag));
                    }
                    flags |= ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::LiteralScalarFlag));
                    return true;
                }

                if (data[0] == '>')
                {
                    if (data.size() >= 2)
                    {
                        if (data[1] != '-' && data[1] != ' ' && data[1] != '\t')
                        {
                            throw ParsingException(ExceptionMessage(Detail::ErrorInvalidBlockScalar(), line, data));
                        }
                    }
                    else
                    {
                        flags |= ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::ScalarNewlineFlag));
                    }
                    flags |= ReaderLine::FlagMask(static_cast<size_t>(ReaderLine::FoldedScalarFlag));
                    return true;
                }

                return false;
            }

            std::list<ReaderLine*> m_Lines;    ///< List of lines.

        };

        //-----------------------------------------------------------------------------------------

        /*! @ingroup cpl_yaml
        * \brief Reads a YAML file and stores the document in root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] filename - Path opened as a binary input file.
        * \note Throws OperationException when the file cannot be opened, or ParsingException
        *       when the file is not well-formed YAML.
        */
        inline void Parse(Node& root, const char* filename)
        {
            std::ifstream f(filename, std::ifstream::binary);
            if (f.is_open() == false)
            {
                throw OperationException(Detail::ErrorCannotOpenFile());
            }

            f.seekg(0, f.end);
            size_t fileSize = static_cast<size_t>(f.tellg());
            f.seekg(0, f.beg);

            std::unique_ptr<char[]> data(new char[fileSize]);
            f.read(data.get(), fileSize);
            f.close();

            Parse(root, data.get(), fileSize);
        }

        /*! @ingroup cpl_yaml
        * \brief Parses YAML text from stream into root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] stream - Input stream positioned at the start of the document.
        * \note Throws ParsingException when the text is not well-formed YAML.
        */
        inline void Parse(Node& root, std::istream& stream)
        {
            ParseImp* pImp = nullptr;

            try
            {
                pImp = new ParseImp;
                pImp->Parse(root, stream);
                delete pImp;
            }
            catch (const Exception e)
            {
                delete pImp;
                throw;
            }
        }

        /*! @ingroup cpl_yaml
        * \brief Parses YAML text from string into root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] string - Complete YAML document.
        * \note Throws ParsingException when the text is not well-formed YAML.
        */
        inline void Parse(Node& root, const std::string& string)
        {
            std::stringstream ss(string);
            Parse(root, ss);
        }

        /*! @ingroup cpl_yaml
        * \brief Parses YAML text from a memory buffer into root.
        * \param [out] root - Destination node. Existing contents are replaced.
        * \param [in] buffer - Pointer to size bytes of YAML text. Need not be zero-terminated.
        * \param [in] size - Number of bytes at buffer.
        * \note Throws ParsingException when the text is not well-formed YAML.
        */
        inline void Parse(Node& root, const char* buffer, const size_t size)
        {
            std::stringstream ss(std::string(buffer, size));
            Parse(root, ss);
        }

        //-----------------------------------------------------------------------------------------

        inline SerializeConfig::SerializeConfig(const size_t spaceIndentation, 
            const size_t scalarMaxLength,
            const bool sequenceMapNewline,
            const bool mapScalarNewline) :
            SpaceIndentation(spaceIndentation),
            ScalarMaxLength(scalarMaxLength),
            SequenceMapNewline(sequenceMapNewline),
            MapScalarNewline(mapScalarNewline)
        {
        }

        //-----------------------------------------------------------------------------------------

        /*! @ingroup cpl_yaml
        * \brief Writes root as YAML text to a file.
        * \param [in] root - Document tree to write.
        * \param [in] filename - Path opened for text output.
        * \param [in] config - Indentation and folding options.
        * \note Throws OperationException when the file cannot be opened or SpaceIndentation is less than 2.
        */
        inline void Serialize(const Node& root, const char* filename, const SerializeConfig& config)
        {
            std::stringstream stream;
            Serialize(root, stream, config);

            std::ofstream f(filename);
            if (f.is_open() == false)
            {
                throw OperationException(Detail::ErrorCannotOpenFile());
            }

            f.write(stream.str().c_str(), stream.str().size());
            f.close();
        }

        inline size_t LineFolding(const std::string& input, std::vector<std::string>& folded, const size_t maxLength)
        {
            folded.clear();
            if (input.size() == 0)
            {
                return 0;
            }

            size_t currentPos = 0;
            size_t lastPos = 0;
            size_t spacePos = std::string::npos;
            while (currentPos < input.size())
            {
                currentPos = lastPos + maxLength;

                if (currentPos < input.size())
                {
                    spacePos = input.find_first_of(' ', currentPos);
                }

                if (spacePos == std::string::npos || currentPos >= input.size())
                {
                    const std::string endLine = input.substr(lastPos);
                    if (endLine.size())
                    {
                        folded.push_back(endLine);
                    }

                    return folded.size();
                }

                folded.push_back(input.substr(lastPos, spacePos - lastPos));

                lastPos = spacePos + 1;
            }

            return folded.size();
        }

        inline void SerializeLoop(const Node& node, std::ostream& stream, bool useLevel, const size_t level, const SerializeConfig& config)
        {
            const size_t indention = config.SpaceIndentation;

            switch (node.Type())
            {
            case Node::SequenceType:
            {
                for (auto it = node.Begin(); it != node.End(); it++)
                {
                    const Node& value = (*it).second;
                    if (value.IsNone())
                    {
                        continue;
                    }
                    stream << std::string(level, ' ') << "- ";
                    useLevel = false;
                    if (value.IsSequence() || (value.IsMap() && config.SequenceMapNewline == true))
                    {
                        useLevel = true;
                        stream << "\n";
                    }

                    SerializeLoop(value, stream, useLevel, level + 2, config);
                }

            }
            break;
            case Node::MapType:
            {
                size_t count = 0;
                for (auto it = node.Begin(); it != node.End(); it++)
                {
                    const Node& value = (*it).second;
                    if (value.IsNone())
                    {
                        continue;
                    }

                    if (useLevel || count > 0)
                    {
                        stream << std::string(level, ' ');
                    }

                    std::string key = (*it).first;
                    AddEscapeTokens(key, "\\\"");
                    if (ShouldBeCited(key))
                    {
                        stream << "\"" << key << "\"" << ": ";
                    }
                    else
                    {
                        stream << key << ": ";
                    }


                    useLevel = false;
                    if (value.IsScalar() == false || (value.IsScalar() && config.MapScalarNewline))
                    {
                        useLevel = true;
                        stream << "\n";
                    }

                    SerializeLoop(value, stream, useLevel, level + indention, config);

                    useLevel = true;
                    count++;
                }

            }
            break;
            case Node::ScalarType:
            {
                const std::string value = node.As<std::string>();

                // Empty scalar
                if (value.size() == 0)
                {
                    stream << "\n";
                    break;
                }

                // Get lines of scalar.
                std::string line = "";
                std::vector<std::string> lines;
                std::istringstream iss(value);
                while (iss.eof() == false)
                {
                    std::getline(iss, line);
                    lines.push_back(line);
                }

                // Block scalar
                const std::string& lastLine = lines.back();
                const bool endNewline = lastLine.size() == 0;
                if (endNewline)
                {
                    lines.pop_back();
                }

                // Literal
                if (lines.size() > 1)
                {
                    stream << "|";
                }
                // Folded/plain
                else
                {
                    const std::string frontLine = lines.front();
                    if (config.ScalarMaxLength == 0 || lines.front().size() <= config.ScalarMaxLength ||
                        LineFolding(frontLine, lines, config.ScalarMaxLength) == 1)
                    {
                        if (useLevel)
                        {
                            stream << std::string(level, ' ');
                        }

                        if (ShouldBeCited(value))
                        {
                            stream << "\"" << value << "\"\n";
                            break;
                        }
                        stream << value << "\n";
                        break;
                    }
                    else
                    {
                        stream << ">";
                    }
                }

                if (endNewline == false)
                {
                    stream << "-";
                }
                stream << "\n";


                for (auto it = lines.begin(); it != lines.end(); it++)
                {
                    stream << std::string(level, ' ') << (*it) << "\n";
                }
            }
            break;

            default:
                break;
            }
        }

        /*! @ingroup cpl_yaml
        * \brief Writes root as YAML text to stream.
        * \param [in] root - Document tree to write.
        * \param [out] stream - Destination stream.
        * \param [in] config - Indentation and folding options.
        * \note Throws OperationException when SpaceIndentation is less than 2.
        */
        inline void Serialize(const Node& root, std::ostream& stream, const SerializeConfig& config)
        {
            if (config.SpaceIndentation < 2)
            {
                throw OperationException(Detail::ErrorIndentation());
            }

            SerializeLoop(root, stream, false, 0, config);
        }

        /*! @ingroup cpl_yaml
        * \brief Writes root as YAML text into string, replacing its previous contents.
        * \param [in] root - Document tree to write.
        * \param [out] string - Destination string.
        * \param [in] config - Indentation and folding options.
        * \note Throws OperationException when SpaceIndentation is less than 2.
        */
        inline void Serialize(const Node& root, std::string& string, const SerializeConfig& config)
        {
            std::stringstream stream;
            Serialize(root, stream, config);
            string = stream.str();
        }

        //-----------------------------------------------------------------------------------------

        inline std::string ExceptionMessage(const std::string& message, ReaderLine& line)
        {
            return message + std::string(" Line ") + std::to_string(line.No) + std::string(": ") + line.Data;
        }

        inline std::string ExceptionMessage(const std::string& message, ReaderLine& line, const size_t errorPos)
        {
            return message + std::string(" Line ") + std::to_string(line.No) + std::string(" column ") + std::to_string(errorPos + 1) + std::string(": ") + line.Data;
        }

        inline std::string ExceptionMessage(const std::string& message, const size_t errorLine, const size_t errorPos)
        {
            return message + std::string(" Line ") + std::to_string(errorLine) + std::string(" column ") + std::to_string(errorPos);
        }

        inline std::string ExceptionMessage(const std::string& message, const size_t errorLine, const std::string& data)
        {
            return message + std::string(" Line ") + std::to_string(errorLine) + std::string(": ") + data;
        }

        inline bool FindQuote(const std::string& input, size_t& start, size_t& end, size_t searchPos)
        {
            start = end = std::string::npos;
            size_t qPos = searchPos;
            bool foundStart = false;

            while (qPos != std::string::npos)
            {
                // Find first quote.
                qPos = input.find_first_of("\"'", qPos);
                if (qPos == std::string::npos)
                {
                    return false;
                }

                const char token = input[qPos];
                if (token == '"' && (qPos == 0 || input[qPos - 1] != '\\'))
                {
                    // Found start quote.
                    if (foundStart == false)
                    {
                        start = qPos;
                        foundStart = true;
                    }
                    // Found end quote
                    else
                    {
                        end = qPos;
                        return true;
                    }
                }

                // Check if it's possible for another loop.
                if (qPos + 1 == input.size())
                {
                    return false;
                }
                qPos++;
            }

            return false;
        }

        inline size_t FindNotCited(const std::string& input, char token, size_t& preQuoteCount)
        {
            preQuoteCount = 0;
            size_t tokenPos = input.find_first_of(token);
            if (tokenPos == std::string::npos)
                return std::string::npos;
            std::vector<std::pair<size_t, size_t>> quotes;
            size_t quoteStart = 0;
            size_t quoteEnd = 0;
            while (FindQuote(input, quoteStart, quoteEnd, quoteEnd))
            {
                quotes.push_back({ quoteStart, quoteEnd });
                if (quoteEnd + 1 == input.size())
                    break;
                quoteEnd++;
            }
            if (quotes.size() == 0)
                return tokenPos;
            size_t currentQuoteIndex = 0;
            std::pair<size_t, size_t> currentQuote = { 0, 0 };
            while (currentQuoteIndex < quotes.size())
            {
                currentQuote = quotes[currentQuoteIndex];
                if (tokenPos < currentQuote.first)
                    return tokenPos;
                preQuoteCount++;
                if (tokenPos <= currentQuote.second)
                {
                    if (tokenPos + 1 == input.size())
                        return std::string::npos;
                    tokenPos = input.find_first_of(token, tokenPos + 1);
                    if (tokenPos == std::string::npos)
                        return std::string::npos;
                }
                currentQuoteIndex++;
            }
            return tokenPos;
        }

        inline size_t FindNotCited(const std::string& input, char token)
        {
            size_t dummy = 0;
            return FindNotCited(input, token, dummy);
        }

        inline bool ValidateQuote(const std::string& input)
        {
            if (input.size() == 0)
                return true;
            char token = 0;
            size_t searchPos = 0;
            if (input[0] == '\"' || input[0] == '\'')
            {
                if (input.size() == 1)
                    return false;
                token = input[0];
                searchPos = 1;
            }
            while (searchPos != std::string::npos && searchPos < input.size() - 1)
            {
                searchPos = input.find_first_of("\"'", searchPos + 1);
                if (searchPos == std::string::npos)
                    break;
                const char foundToken = input[searchPos];
                if (input[searchPos] == '\"' || input[searchPos] == '\'')
                {
                    if (token == 0 && input[searchPos - 1] != '\\')
                        return false;
                    if (foundToken == token && input[searchPos - 1] != '\\')
                    {
                        if (searchPos == input.size() - 1)
                            return true;
                        return false;
                    }
                }
            }
            return token == 0;
        }

        inline void CopyNode(const Node& from, Node& to)
        {
            const Node::eType type = from.Type();
            switch (type)
            {
            case Node::SequenceType:
                for (auto it = from.Begin(); it != from.End(); it++)
                {
                    const Node& currentNode = (*it).second;
                    Node& newNode = to.PushBack();
                    CopyNode(currentNode, newNode);
                }
                break;
            case Node::MapType:
                for (auto it = from.Begin(); it != from.End(); it++)
                {
                    const Node& currentNode = (*it).second;
                    Node& newNode = to[(*it).first];
                    CopyNode(currentNode, newNode);
                }
                break;
            case Node::ScalarType:
                to = from.As<std::string>();
                break;
            case Node::None:
                break;
            }
        }

        inline bool ShouldBeCited(const std::string& key)
        {
            return key.find_first_of("\":{}[],&*#?|-<>=!%@") != std::string::npos;
        }

        inline void AddEscapeTokens(std::string& input, const std::string& tokens)
        {
            for (auto it = tokens.begin(); it != tokens.end(); it++)
            {
                const char token = *it;
                const std::string replace = std::string("\\") + std::string(1, token);
                size_t found = input.find_first_of(token);
                while (found != std::string::npos)
                {
                    input.replace(found, 1, replace);
                    found = input.find_first_of(token, found + 2);
                }
            }
        }

        inline void RemoveAllEscapeTokens(std::string& input)
        {
            size_t found = input.find_first_of("\\");
            while (found != std::string::npos)
            {
                if (found + 1 == input.size())
                    return;
                std::string replace(1, input[found + 1]);
                input.replace(found, 2, replace);
                found = input.find_first_of("\\", found + 1);
            }
        }
    }
}