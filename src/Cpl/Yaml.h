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

            eType m_Type; ///< Type of iterator.
            void* m_pImp; ///< Implementation of iterator class.

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

            eType m_Type; ///< Type of iterator.
            void* m_pImp; ///< Implementation of constant iterator class.

        };

        class Node
        {

        public:

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
                    return std::string();
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
                    return std::string();
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
        }

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

            return { String(), Node() };
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

            return { String(), Node() };
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

    }
}