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

        class Exception : public std::runtime_error
        {
        public:
            enum Type
            {
                InternalError,
                ParsingError,
                OperationError
            };

            Exception(const std::string& message, const Type type)
                : std::runtime_error(message)
                , _type(type)
            {
            }

            Type GetType() const
            {
                return _type;
            }

            const char* Message() const
            {
                return what();
            }

        private:
            Type _type;
        };

        class InternalException : public Exception
        {
        public:
            InternalException(const std::string& message)
                : Exception(message, InternalError)
            {
            }
        };

        class ParsingException : public Exception
        {
        public:
            ParsingException(const std::string& message)
                : Exception(message, ParsingError)
            {
            }
        };

        class OperationException : public Exception
        {
        public:
            OperationException(const std::string& message)
                : Exception(message, OperationError)
            {
            }
        };

        class Iterator
        {
        public:
            friend class Node;

            Iterator();

            Iterator(const Iterator& it);

            Iterator& operator = (const Iterator& it);

            ~Iterator();

            std::pair<const std::string&, Node&> operator *();

            Iterator& operator ++ (int);

            Iterator& operator -- (int);

            bool operator == (const Iterator& it);

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

        class ConstIterator
        {
        public:
            friend class Node;

            ConstIterator();

            ConstIterator(const ConstIterator& it);

            ConstIterator& operator = (const ConstIterator& it);

            ~ConstIterator();

            std::pair<const std::string&, const Node&> operator *();

            ConstIterator& operator ++ (int);

            ConstIterator& operator -- (int);

            bool operator == (const ConstIterator& it);

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

        class Node
        {
            Node(bool)
                : m_pImp(nullptr)
            {
            }
        public:

            inline static Node & Empty()
            {
                static Node empty = Node(true);
                return empty;
            }

            friend class Iterator;

            enum eType
            {
                None,
                SequenceType,
                MapType,
                ScalarType
            };

            Node();

            Node(const Node& node);

            Node(const std::string& value);
            Node(const char* value);

            ~Node();

            eType Type() const;
            bool IsNone() const;
            bool IsSequence() const;
            bool IsMap() const;
            bool IsScalar() const;

            void Clear();

            template<typename T> T As() const
            {
                return Detail::StringConverter<T>::Get(AsString());
            }

            template<typename T> T As(const T& defaultValue) const
            {
                return Detail::StringConverter<T>::Get(AsString(), defaultValue);
            }

            size_t Size() const;

            Node& Insert(const size_t index);

            Node& PushFront();

            Node& PushBack();

            Node& operator []  (const size_t index);
            Node& operator [] (const std::string& key);

            void Erase(const size_t index);
            void Erase(const std::string& key);

            Node& operator = (const Node& node);
            Node& operator = (const std::string& value);
            Node& operator = (const char* value);

            Iterator Begin();
            ConstIterator Begin() const;

            Iterator End();
            ConstIterator End() const;

        private:
            const std::string& AsString() const;

            void* m_pImp; ///< Implementation of node class.
        };

        void Parse(Node& root, const char* filename);
        void Parse(Node& root, std::iostream& stream);
        void Parse(Node& root, const std::string& string);
        void Parse(Node& root, const char* buffer, const size_t size);

        struct SerializeConfig
        {
            SerializeConfig(const size_t spaceIndentation = 2,
                const size_t scalarMaxLength = 64,
                const bool sequenceMapNewline = false,
                const bool mapScalarNewline = false);

            size_t SpaceIndentation;    ///< Number of spaces per indentation.
            size_t ScalarMaxLength;     ///< Maximum length of scalars. Serialized as folder scalars if exceeded.
            bool SequenceMapNewline;    ///< Put maps on a new line if parent node is a sequence.
            bool MapScalarNewline;      ///< Put scalars on a new line if parent node is a map.
        };

        void Serialize(const Node& root, const char* filename, const SerializeConfig& config = { 2, 64, false, false });
        void Serialize(const Node& root, std::iostream& stream, const SerializeConfig& config = { 2, 64, false, false });
        void Serialize(const Node& root, std::string& string, const SerializeConfig& config = { 2, 64, false, false });

        namespace Detail
        {
            CPL_INLINE String & EmptyString()
            {
                static String empty = String();
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

            return { String(), Node::Empty() };
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

            return { String(), Node::Empty() };
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

#define NODE_IMP static_cast<Detail::NodeImp*>(m_pImp)
#define NODE_IMP_EXT(node) static_cast<Detail::NodeImp*>(node.m_pImp)
#define TYPE_IMP static_cast<Detail::NodeImp*>(m_pImp)->m_pImp
#define IT_IMP static_cast<Detail::IteratorImp*>(m_pImp)

        inline Node::Node() 
            : m_pImp(new Detail::NodeImp())
        {
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
            delete static_cast<Detail::NodeImp*>(m_pImp);
        }

        inline Node::eType Node::Type() const
        {
            return NODE_IMP->m_Type;
        }

        inline bool Node::IsNone() const
        {
            return NODE_IMP->m_Type == Node::None;
        }

        inline bool Node::IsSequence() const
        {
            return NODE_IMP->m_Type == Node::SequenceType;
        }

        inline bool Node::IsMap() const
        {
            return NODE_IMP->m_Type == Node::MapType;
        }

        inline bool Node::IsScalar() const
        {
            return NODE_IMP->m_Type == Node::ScalarType;
        }

        inline void Node::Clear()
        {
            NODE_IMP->Clear();
        }

        inline size_t Node::Size() const
        {
            if (TYPE_IMP == nullptr)
            {
                return 0;
            }

            return TYPE_IMP->GetSize();
        }

        inline Node& Node::Insert(const size_t index)
        {
            NODE_IMP->InitSequence();
            return *TYPE_IMP->Insert(index);
        }

        inline Node& Node::PushFront()
        {
            NODE_IMP->InitSequence();
            return *TYPE_IMP->PushFront();
        }

        inline Node& Node::PushBack()
        {
            NODE_IMP->InitSequence();
            return *TYPE_IMP->PushBack();
        }

        inline Node& Node::operator[](const size_t index)
        {
            NODE_IMP->InitSequence();
            Node* pNode = TYPE_IMP->GetNode(index);
            if (pNode == nullptr)
                return Node::Empty();
            return *pNode;
        }

        inline Node& Node::operator[](const std::string& key)
        {
            NODE_IMP->InitMap();
            return *TYPE_IMP->GetNode(key);
        }

        inline void Node::Erase(const size_t index)
        {
            if (TYPE_IMP == nullptr || NODE_IMP->m_Type != Node::SequenceType)
                return;
            return TYPE_IMP->Erase(index);
        }

        inline void Node::Erase(const std::string& key)
        {
            if (TYPE_IMP == nullptr || NODE_IMP->m_Type != Node::MapType)
            {
                return;
            }

            return TYPE_IMP->Erase(key);
        }

        inline Node& Node::operator = (const Node& node)
        {
            NODE_IMP->Clear();
            CopyNode(node, *this);
            return *this;
        }

        inline Node& Node::operator = (const std::string& value)
        {
            NODE_IMP->InitScalar();
            TYPE_IMP->SetData(value);
            return *this;
        }

        inline Node& Node::operator = (const char* value)
        {
            NODE_IMP->InitScalar();
            TYPE_IMP->SetData(value ? std::string(value) : "");
            return *this;
        }

        inline Iterator Node::Begin()
        {
            Iterator it;

            if (TYPE_IMP != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;

                switch (NODE_IMP->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = Iterator::SequenceType;
                    pItImp = new Detail::SequenceIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::SequenceImp*>(TYPE_IMP));
                    break;
                case Node::MapType:
                    it.m_Type = Iterator::MapType;
                    pItImp = new Detail::MapIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::MapImp*>(TYPE_IMP));
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

            if (TYPE_IMP != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;

                switch (NODE_IMP->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = ConstIterator::SequenceType;
                    pItImp = new Detail::SequenceConstIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::SequenceImp*>(TYPE_IMP));
                    break;
                case Node::MapType:
                    it.m_Type = ConstIterator::MapType;
                    pItImp = new Detail::MapConstIteratorImp;
                    pItImp->InitBegin(static_cast<Detail::MapImp*>(TYPE_IMP));
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

            if (TYPE_IMP != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;

                switch (NODE_IMP->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = Iterator::SequenceType;
                    pItImp = new Detail::SequenceIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::SequenceImp*>(TYPE_IMP));
                    break;
                case Node::MapType:
                    it.m_Type = Iterator::MapType;
                    pItImp = new Detail::MapIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::MapImp*>(TYPE_IMP));
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

            if (TYPE_IMP != nullptr)
            {
                Detail::IteratorImp* pItImp = nullptr;

                switch (NODE_IMP->m_Type)
                {
                case Node::SequenceType:
                    it.m_Type = ConstIterator::SequenceType;
                    pItImp = new Detail::SequenceConstIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::SequenceImp*>(TYPE_IMP));
                    break;
                case Node::MapType:
                    it.m_Type = ConstIterator::MapType;
                    pItImp = new Detail::MapConstIteratorImp;
                    pItImp->InitEnd(static_cast<Detail::MapImp*>(TYPE_IMP));
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
            if (TYPE_IMP == nullptr)
                return Detail::EmptyString();

            return TYPE_IMP->GetData();
        }

        //-----------------------------------------------------------------------------------------

        class ReaderLine
        {
        public:
            ReaderLine(const std::string& data = "",
                const size_t no = 0,
                const size_t offset = 0,
                const Node::eType type = Node::None,
                const unsigned char flags = 0) :
                Data(data),
                No(no),
                Offset(offset),
                Type(type),
                Flags(flags),
                NextLine(nullptr)
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
                return Flags & FlagMask(static_cast<size_t>(flag));
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

            std::string     Data;
            size_t          No;
            size_t          Offset;
            Node::eType     Type;
            unsigned char   Flags;
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

            void Parse(Node& root, std::iostream& stream)
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

            void ReadLines(std::iostream& stream)
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
                    if (PostProcessSequenceLine(it) == true)
                    {
                        continue;
                    }

                    if (PostProcessMappingLine(it) == true)
                    {
                        continue;
                    }

                    // Scalar.
                    PostProcessScalarLine(it);
                }

                if (m_Lines.size())
                {
                    if (m_Lines.back()->Type != Node::ScalarType)
                    {
                        throw ParsingException(ExceptionMessage(Detail::ErrorUnexpectedDocumentEnd(), *m_Lines.back()));
                    }

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
    }
}