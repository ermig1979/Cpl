/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar,
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

#include "Cpl/Param.h"

namespace Cpl
{
    template<class T> struct ParamVectorV2 : public Cpl::ParamVector<T>
    {
    protected:
        typedef Cpl::Param<int> Unknown;

        ParamVectorV2(const String& name)
            : ParamVector<T>(name)
        {
        }

        CPL_INLINE String CountName() const { return "count"; }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                auto countName = CountName();
                auto itemName = Cpl::ParamVector<T>::ItemName();
                size_t itemCount = Xml::CountChildren(xmlCurrent, itemName.c_str(), itemName.size());
                size_t childrenCount = Xml::CountChildren(xmlCurrent);

                auto* countNode = xmlCurrent->FirstNode(countName.c_str(), countName.size());
                // 0 or 1 'count' node allowed
                if (countNode)
                {
                    if (itemCount != childrenCount - 1)
                        return false;

                    int count = 0;
                    Cpl::ToVal(countNode->Value(), count);
                    if (count != itemCount)
                        return false;
                }
                else if (itemCount != childrenCount)
                    return false;
                Cpl::ParamVector<T>::Resize(itemCount);
                Xml::XmlNode<char>* xmlItem = xmlCurrent->FirstNode(itemName.c_str(), itemName.size());

                size_t size = Cpl::ParamVector<T>::Size();
                for (size_t i = 0; i < itemCount; ++i)
                {
                    Unknown* paramChild = Cpl::ParamVector<T>::ChildBeg(i);
                    const Unknown* paramChildEnd = Cpl::ParamVector<T>::ChildBeg(i + 1);
                    for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                    {
                        if (!paramChild->LoadNodeXml(xmlItem))
                            return true;
                    }
                    xmlItem = xmlItem->NextSibling(itemName.c_str(), itemName.size());
                }
            }
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            
            Xml::XmlNode<char>* xmlCount = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(CountName().c_str()));
            xmlCount->Value(xmlDoc.AllocateString(Cpl::ToStr(Cpl::ParamVector<T>::Size()).c_str()));
            xmlCurrent->AppendNode(xmlCount);

            for (size_t i = 0; i < Cpl::ParamVector<T>::Size(); ++i)
            {
                const Unknown* paramChild = Cpl::ParamVector<T>::ChildBeg(i);
                const Unknown* paramChildEnd = Cpl::ParamVector<T>::ChildBeg(i + 1);
                Xml::XmlNode<char>* xmlItem = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(Cpl::ParamVector<T>::ItemName().c_str()));
                for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                {
                    if (full || paramChild->Changed())
                        paramChild->SaveNodeXml(xmlDoc, xmlItem, full);
                }
                xmlCurrent->AppendNode(xmlItem);
            }
            xmlParent->AppendNode(xmlCurrent);
        }
    };

    //---------------------------------------------------------------------------------------------

    template<class K, class T> struct ParamMapV2 : public Cpl::ParamMap<K, T>
    {
    protected:
        typedef Cpl::Param<int> Unknown;

        ParamMapV2(const String& name)
            : ParamMap<K, T>(name)
        {
        }

        CPL_INLINE String CountName() const { return "count"; }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                auto countName = CountName();
                auto itemName = Cpl::ParamMap<K, T>::ItemName();
                size_t itemCount = Xml::CountChildren(xmlCurrent, itemName.c_str(), itemName.size());
                size_t childrenCount = Xml::CountChildren(xmlCurrent);

                auto* countNode = xmlCurrent->FirstNode(countName.c_str(), countName.size());
                // 0 or 1 'count' node allowed
                if (countNode)
                {
                    if (itemCount != childrenCount - 1)
                        return false;

                    int count = 0;
                    Cpl::ToVal(countNode->Value(), count);
                    if (count != itemCount)
                        return false;
                }
                else if (itemCount != childrenCount)
                    return false;

                Xml::XmlNode<char>* xmlItem = xmlCurrent->FirstNode(itemName.c_str(), itemName.size());
                for (size_t i = 0; i < itemCount; ++i)
                {
                    Xml::XmlNode<char>* xmlKey = xmlItem->FirstNode(Cpl::ParamMap<K, T>::KeyName().c_str());
                    if (xmlKey)
                    {
                        K key;
                        Cpl::ToVal(xmlKey->Value(), key);
                        T& value = this->_value[key];
                        Xml::XmlNode<char>* xmlValue = xmlItem->FirstNode(Cpl::ParamMap<K, T>::ValueName().c_str());
                        if (xmlValue)
                        {
                            Unknown* paramChild = Cpl::ParamMap<K, T>::ChildBeg(value);
                            Unknown* paramChildEnd = Cpl::ParamMap<K, T>::ChildEnd(value);
                            for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                            {
                                if (!paramChild->LoadNodeXml(xmlValue))
                                    return true;
                            }
                        }
                    }
                    xmlItem = xmlItem->NextSibling(itemName.c_str(), itemName.size());
                }
            }
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            Xml::XmlNode<char>* xmlCount = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(CountName().c_str()));
            xmlCount->Value(xmlDoc.AllocateString(Cpl::ToStr(this->_value.size()).c_str()));
            xmlCurrent->AppendNode(xmlCount);
            
            for (typename Cpl::ParamMap<K, T>::Map::const_iterator it = this->_value.begin(); it != this->_value.end(); ++it)
            {
                Xml::XmlNode<char>* xmlItem = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(Cpl::ParamMap<K, T>::ItemName().c_str()));

                Xml::XmlNode<char>* xmlKey = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(Cpl::ParamMap<K, T>::KeyName().c_str()));
                xmlKey->Value(xmlDoc.AllocateString(Cpl::ToStr(it->first).c_str()));
                xmlItem->AppendNode(xmlKey);

                Xml::XmlNode<char>* xmlValue = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(Cpl::ParamMap<K, T>::ValueName().c_str()));
                const Unknown* paramChild = this->ChildBeg(it->second);
                const Unknown* paramChildEnd = this->ChildEnd(it->second);
                for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                {
                    if (full || paramChild->Changed())
                        paramChild->SaveNodeXml(xmlDoc, xmlValue, full);
                }
                xmlItem->AppendNode(xmlValue);

                xmlCurrent->AppendNode(xmlItem);
            }
            xmlParent->AppendNode(xmlCurrent);
        }
    };
}

//-------------------------------------------------------------------------------------------------

#define CPL_PARAM_VECTOR_V2(type, name) \
struct Param_##name : public Cpl::ParamVectorV2<type> \
{ \
    typedef Cpl::ParamVectorV2<type> Base; \
    Param_##name() : Base(#name) {} \
} name;

#define CPL_PARAM_MAP_V2(key, type, name) \
struct Param_##name : public Cpl::ParamMapV2<key, type> \
{ \
    typedef Cpl::ParamMapV2<key, type> Base; \
    Param_##name() : Base(#name) {} \
} name;
