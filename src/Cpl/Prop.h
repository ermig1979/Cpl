/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2023 Yermalayeu Ihar,
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

#include "Cpl/Param.h"
#include "Cpl/Log.h"

namespace Cpl
{
    template<class T> struct ParamProp: public Cpl::ParamLimited<T>
    {
        typedef T Type;

        virtual CPL_INLINE String Description() const
        {
            return String();
        }

        virtual CPL_INLINE bool Limited() const
        {
            return false;
        }

        virtual CPL_INLINE String ToStr() const
        {
            return Cpl::ToStr<T>(this->_value);
        }

        virtual CPL_INLINE void ToVal(const String & str)
        {
            return Cpl::ToVal<T>(str, this->_value);
        }

    protected:
        typedef Cpl::ParamLimited<T> Base;
        typedef Cpl::Param<int> Unknown;

        ParamProp(const String& name)
            : Base(name)
        {
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlValue = xmlParent->FirstNode("value");
            if(xmlValue)
                Cpl::ToVal(xmlValue->Value(), this->_value);
            return true;
        }

        static CPL_INLINE const char * NotEmpty(const String& value)
        {
            return value.empty() ? " " : value.c_str();
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlValue = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("value"));
            xmlValue->Value(xmlDoc.AllocateString(NotEmpty(Cpl::ToStr(this->_value))));
            xmlParent->AppendNode(xmlValue);

            Xml::XmlNode<char>* xmlDescr = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("desc"));
            xmlDescr->Value(xmlDoc.AllocateString(NotEmpty(this->Description())));
            xmlParent->AppendNode(xmlDescr);

            Xml::XmlNode<char>* xmlMin = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("value_min"));
            xmlMin->Value(xmlDoc.AllocateString(this->Limited() ? NotEmpty(Cpl::ToStr(this->Min())) : " "));
            xmlParent->AppendNode(xmlMin);

            Xml::XmlNode<char>* xmlMax = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("value_max"));
            xmlMax->Value(xmlDoc.AllocateString(this->Limited() ? NotEmpty(Cpl::ToStr(this->Max())) : " "));
            xmlParent->AppendNode(xmlMax);

            Xml::XmlNode<char>* xmlDefault = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("value_default"));
            xmlDefault->Value(xmlDoc.AllocateString(NotEmpty(Cpl::ToStr(this->Default()))));
            xmlParent->AppendNode(xmlDefault);
        }

        template<typename> friend struct ParamStorage;
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamStorage : public Cpl::ParamStruct<T>
    {
        typedef T Type;

        bool SetProperty(const String& name, const String& value)
        {
            Map::iterator it = _map.find(name);
            if (it == _map.end())
                return false;
            it->second->ToVal(value);
            return true;
        }

        bool GetProperty(const String& name, String& value) const
        {
            Map::const_iterator it = _map.find(name);
            if (it == _map.end())
                return false;
            value = it->second->ToStr();
            return true;
        }

    protected:
        typedef Cpl::ParamStruct<T> Base;
        typedef Cpl::Param<int> Unknown;
        typedef Cpl::ParamProp<int> UnknownProp;
        typedef Cpl::ParamStruct<int> UnknownGroup;
        typedef std::map<String, UnknownProp*> Map;

        Map _map;

        ParamStorage(const String& name)
            : Base(name)
        {
            for (Unknown* group = this->ChildBeg(); group < this->End(); group = group->End())
            {
                for (Unknown* prop = ((UnknownGroup*)group)->ChildBeg(); prop < group->End(); prop = prop->End())
                {
                    String name = group->Name() + "." + prop->Name();
                    _map[name] = (UnknownProp*)prop;
                }
            }
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlStorage = xmlParent->FirstNode("storage");
            if (xmlStorage == NULL)
                return false;
            Xml::XmlNode<char>* xmlMap = xmlStorage->FirstNode("map");
            if (xmlMap == NULL)
                return false;
            for (Xml::XmlNode<char>* xmlItem = xmlMap->FirstNode("item"); xmlItem; xmlItem = xmlItem->NextSibling())
            {
                Xml::XmlNode<char>* xmlFirst = xmlItem->FirstNode("first");
                if (xmlFirst == NULL)
                    return false;
                Map::iterator it = _map.find(xmlFirst->Value());
                if (it == _map.end())
                {
                    CPL_LOG_SS(Debug, "Load XML has unknown propery '" << xmlFirst->Value() << "'!")
                    continue;
                }
                Xml::XmlNode<char>* xmlSecond = xmlItem->FirstNode("second");
                if (xmlSecond == NULL)
                    return false;
                if (!it->second->LoadNodeXml(xmlSecond))
                    return false;
            }
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlStorage = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("storage"));
            Xml::XmlNode<char>* xmlMap = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("map"));

            Xml::XmlNode<char>* xmlCount = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("count"));
            if(full)
                xmlCount->Value(xmlDoc.AllocateString(Cpl::ToStr(_map.size()).c_str()));
            else
            {
                size_t count = 0;
                for (Map::const_iterator it = _map.begin(); it != _map.end(); ++it)
                    if (it->second->Changed())
                        count++;
                xmlCount->Value(xmlDoc.AllocateString(Cpl::ToStr(count).c_str()));
            }
            xmlMap->AppendNode(xmlCount);

            for (Map::const_iterator it = _map.begin(); it != _map.end(); ++it)
            {
                if (it->second->Changed() || full)
                {
                    Xml::XmlNode<char>* xmlItem = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("item"));

                    Xml::XmlNode<char>* xmlFirst = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("first"));
                    xmlFirst->Value(xmlDoc.AllocateString(it->first.c_str()));
                    xmlItem->AppendNode(xmlFirst);

                    Xml::XmlNode<char>* xmlSecond = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString("second"));
                    it->second->SaveNodeXml(xmlDoc, xmlSecond, true);
                    xmlItem->AppendNode(xmlSecond);

                    xmlMap->AppendNode(xmlItem);
                }
            }
            xmlStorage->AppendNode(xmlMap);
            xmlParent->AppendNode(xmlStorage);
        }
    };
}

//-------------------------------------------------------------------------------------------------

#define CPL_PROP(type, name, value, descr) \
struct Param_##name : public Cpl::ParamProp<type> \
{ \
    typedef Cpl::ParamProp<type> Base; \
    Param_##name() : Base(#name) { this->_value = this->Default(); } \
    type Default() const override { return value; } \
    Cpl::String Description() const override { return descr; } \
} name;

#define CPL_PROP_EX(type, name, value, min, max, descr) \
struct Param_##name : public Cpl::ParamProp<type> \
{ \
    typedef Cpl::ParamProp<type> Base; \
    Param_##name() : Base(#name) { assert(min <= value && value <= max); this->_value = this->Default(); } \
    type Default() const override { return value; } \
    type Min() const override { return min; } \
    type Max() const override { return max; } \
    Cpl::String Description() const override { return descr; } \
    bool Limited() const override { return true; } \
} name;

#define CPL_PROP_GROUP(type, name) CPL_PARAM_STRUCT(type, name)

#define CPL_PROP_STORAGE(storage, type, name) \
struct storage : public Cpl::ParamStorage<type> \
{ \
    typedef Cpl::ParamStorage<type> Base; \
    storage() : Base(#name) {} \
};
