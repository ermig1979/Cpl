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
            enum eType
            {
                InternalError,  ///< Internal error.
                ParsingError,   ///< Invalid parsing data.
                OperationError  ///< User operation error.
            };

            Exception(const std::string& message, const eType type);

            eType Type() const;

            const char* Message() const;

        private:
            eType m_Type;   ///< Type of exception.
        };

        class InternalException : public Exception
        {
        public:
            InternalException(const std::string& message);
        };

        class ParsingException : public Exception
        {
        public:
            ParsingException(const std::string& message);
        };

        class OperationException : public Exception
        {
        public:
            OperationException(const std::string& message);
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
    }
}