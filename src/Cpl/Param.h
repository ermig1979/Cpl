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

#include "Cpl/Xml.h"
#include "Cpl/String.h"

namespace Cpl
{
    template<class T> struct Param
    {
        typedef T Type;
        
        CPL_INLINE const Type & operator () () const { return _value; }
        CPL_INLINE Type & operator () ()  { return _value; }

        CPL_INLINE const String & Name() const { return _name; }

        virtual Type Default() const { return T(); }

        virtual bool Changed() const
        {
            for (const Unknown * child = StructBegin(); child < StructEnd(); child = StructNext(child))
            {
                if (child->Changed())
                    return true;
            }
            return false;
        }

        virtual void Clone(const Param & other)
        {
            switch (_mode)
            {
            case Value:
                this->Clone(other);
                break;
            case Struct:
                for (Unknown * tc = this->StructBegin(), *oc = other.StructBegin(); tc < this->StructEnd(); tc = this->StructNext(tc), oc = this->StructNext(oc))
                    tc->Clone(*oc);
                break;
            case Vector:
                this->Resize(other.VectorEnd() - other.VectorBegin());
                for (Unknown * tc = this->VectorBegin(), *oc = other.VectorBegin(); tc < this->VectorEnd(); tc = this->VectorNext(tc), oc = this->VectorNext(oc))
                    tc->Clone(*oc);
                break;
            }
        }

        bool Save(std::ostream & os, bool full) const
        {
            Xml::XmlDocument<char> doc;
            Cpl::Xml::XmlNode<char> * xmlDeclaration = doc.AllocateNode(Cpl::Xml::NodeDeclaration);
            xmlDeclaration->AppendAttribute(doc.AllocateAttribute("version", "1.0"));
            xmlDeclaration->AppendAttribute(doc.AllocateAttribute("encoding", "utf-8"));
            doc.AppendNode(xmlDeclaration);
            this->Save(doc, &doc, full);
            Xml::Print(os, doc);
            return true;
        }

        bool Save(const String & path, bool full) const
        {
            bool result = false;
            std::ofstream ofs(path.c_str());
            if (ofs.is_open())
            {
                result = this->Save(ofs, full);
                ofs.close();
            }
            return result;
        }

        bool Load(const char * data, size_t size)
        {
            Xml::File<char> file(data, size);
            return Load(file);
        }

        bool Load(std::istream & is)
        {
            Xml::File<char> file(is);
            return Load(file);
        }

        bool Load(const String & path)
        {
            bool result = false;
            std::ifstream ifs(path.c_str());
            if (ifs.is_open())
            {
                result = this->Load(ifs);
                ifs.close();
            }
            return result;
        }

    protected:
        enum Mode
        {
            Value,
            Struct,
            Vector,
        };

        Mode _mode;
        String _name;
        size_t _size, _item;
        Type _value;

        Param(Mode mode, const String & name, size_t size, size_t item = 0)
            : _mode(mode)
            , _name(name)
            , _size(size)
            , _item(item)
        {
        }
        
        virtual String ToStr() const { return ""; }
        virtual void ToVal(const String & string) {}
        virtual void Resize(size_t size) {}
        CPL_INLINE String ItemName() const { return "item"; }

        template<typename> friend struct Param;

        typedef Param<int> Unknown;

        CPL_INLINE Unknown * StructBegin() const { return (Unknown*)(&_value); }
        CPL_INLINE Unknown * StructNext(const Unknown * param) const { return (Unknown*)((char*)param + param->_size); }
        CPL_INLINE Unknown * StructEnd() const { return (Unknown *)((char*)this + this->_size); }

        CPL_INLINE Unknown * VectorBegin() const { return (*(std::vector<Unknown>*)&_value).data(); }
        CPL_INLINE Unknown * VectorNext(const Unknown * param) const { return (Unknown*)((char*)param + this->_item); }
        CPL_INLINE Unknown * VectorEnd() const { return (*(std::vector<Unknown>*)&_value).data() + (*(std::vector<Unknown>*)&_value).size(); }

        bool Load(Xml::File<char> & file)
        {
            Xml::XmlDocument<char> doc;
            try
            {
                doc.Parse<0>(file.Data());
            }
            catch (std::exception & e)
            {
                std::cout << "Can't parse xml! There is an exception: " << e.what() << std::endl;
                return false;
            }
            return this->Load(&doc);
        }

        bool Load(Xml::XmlNode<char> * xmlParent)
        {
            Xml::XmlNode<char> * xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                switch (_mode)
                {
                case Value:
                    this->ToVal(xmlCurrent->Value());
                    break;
                case Struct:
                    for (Unknown * paramChild = this->StructBegin(); paramChild < this->StructEnd(); paramChild = this->StructNext(paramChild))
                    {
                        if (!paramChild->Load(xmlCurrent))
                            return true;
                    }
                    break;
                case Vector:
                    this->Resize(Xml::CountChildren(xmlCurrent));
                    Xml::XmlNode<char> * xmlItem = xmlCurrent->FirstNode();
                    for (Unknown * paramItem = this->VectorBegin(); paramItem < this->VectorEnd(); paramItem = this->VectorNext(paramItem))
                    {
                        if (ItemName() != xmlItem->Name())
                            return false;
                        const Unknown * paramChildEnd = this->VectorNext(paramItem);
                        for (Unknown * paramChild = paramItem; paramChild < paramChildEnd; paramChild = this->StructNext(paramChild))
                        {
                            if (!paramChild->Load(xmlItem))
                                return true;
                        }
                        xmlItem = xmlItem->NextSibling();
                    }
                    break;
                }
            }
            return true;
        }

        void Save(Xml::XmlDocument<char> & xmlDoc, Xml::XmlNode<char> * xmlParent, bool full) const
        {
            Xml::XmlNode<char> * xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            switch (_mode)
            {
            case Value:
                xmlCurrent->Value(xmlDoc.AllocateString(this->ToStr().c_str()));
                break;
            case Struct:
                for (const Unknown * paramChild = this->StructBegin(); paramChild < this->StructEnd(); paramChild = this->StructNext(paramChild))
                {
                    if (full || paramChild->Changed())
                        paramChild->Save(xmlDoc, xmlCurrent, full);
                }
                break;
            case Vector:
                for (const Unknown * paramItem = this->VectorBegin(); paramItem < this->VectorEnd(); paramItem = this->VectorNext(paramItem))
                {
                    Xml::XmlNode<char> * xmlItem = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(ItemName().c_str()));
                    const Unknown * paramChildEnd = this->VectorNext(paramItem);
                    for (const Unknown * paramChild = paramItem; paramChild < paramChildEnd; paramChild = this->StructNext(paramChild))
                    {
                        if (full || paramChild->Changed())
                            paramChild->Save(xmlDoc, xmlItem, full);
                    }
                    xmlCurrent->AppendNode(xmlItem);
                }
                break;
            }
            xmlParent->AppendNode(xmlCurrent);
        }
    };

    CPL_INLINE void ParseEnumNames(const char * data, Strings & names)
    {
        if (names.size())
            return;
        while (*data)
        {
            const char * beg = data;
            while (*beg == ' ' || *beg == ',') beg++;
            const char * end = beg;
            while (*end && *end != ' ' && *end != ',') end++;
            if (beg == end)
                break;
            names.push_back(String(beg, end));
            data = end;
        }
    }
}

#define CPL_PARAM_VALUE(type, name, value) \
struct Param_##name : public Cpl::Param<type> \
{ \
typedef Cpl::Param<type> Base; \
Param_##name() : Base(Base::Value, #name, sizeof(Param_##name)) { this->_value = this->Default(); } \
virtual type Default() const { return value; } \
virtual Cpl::String ToStr() const { return Cpl::ToStr((*this)()); } \
virtual void ToVal(const Cpl::String & string) { Cpl::ToVal(string, this->_value); } \
virtual bool Changed() const { return this->Default() != this->_value; } \
virtual void Clone(const Param_##name & other) { this->_value = other._value; } \
} name;

#define CPL_PARAM_STRUCT(type, name) \
struct Param_##name : public Cpl::Param<type> \
{ \
typedef Cpl::Param<type> Base; \
Param_##name() : Base(Base::Struct, #name, sizeof(Param_##name)) {} \
} name;

#define CPL_PARAM_VECTOR(type, name) \
struct Param_##name : public Cpl::Param<std::vector<type>> \
{ \
typedef Cpl::Param<std::vector<type>> Base; \
Param_##name() : Base(Base::Vector, #name, sizeof(Param_##name), sizeof(type)) {} \
virtual void Resize(size_t size) { this->_value.resize(size); } \
virtual bool Changed() const { return !this->_value.empty(); } \
} name;

#define CPL_PARAM_ENUM_DECL(type, unknown, size, ...) \
enum type \
{ \
    type##unknown = -1, \
    ##__VA_ARGS__, \
    type##size \
};

#define CPL_PARAM_ENUM_CONV(ns, type, unknown, size, ...) \
namespace Cpl \
{\
    using namespace ns; \
    template<> CPL_INLINE Cpl::String ToStr<type>(const type& value) \
    {\
        static thread_local Cpl::Strings names; \
        Cpl::ParseEnumNames(#__VA_ARGS__, names); \
        return (value > type##unknown && value < type##size) ? names[value].substr(sizeof(#type) - 1) : Cpl::String(); \
    }\
    \
    template<> CPL_INLINE void ToVal<type>(const Cpl::String& string, type& value)\
    {\
        value = Cpl::ToEnum<type, type##size>(string); \
    }\
}

#define CPL_PARAM_ENUM0(type, ...) \
    CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) \
    CPL_PARAM_ENUM_CONV(Cpl, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_ENUM1(ns1, type, ...) \
    namespace ns1 { CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } \
    CPL_PARAM_ENUM_CONV(ns1, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_ENUM2(ns1, ns2, type, ...) \
    namespace ns1 { namespace ns2 { CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } }\
    CPL_PARAM_ENUM_CONV(ns1::ns2, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_ENUM3(ns1, ns2, ns3, type, ...) \
    namespace ns1 { namespace ns2 { namespace ns3 {CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } } }\
    CPL_PARAM_ENUM_CONV(ns1::ns2::ns3, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_HOLDER(holder, type, name) \
struct holder : public Cpl::Param<type> \
{ \
typedef Cpl::Param<type> Base; \
holder() : Base(Base::Struct, #name, sizeof(holder)) {} \
};
