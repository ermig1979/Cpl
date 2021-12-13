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

        CPL_INLINE const Type& operator () () const { return _value; }
        CPL_INLINE Type& operator () () { return _value; }

        CPL_INLINE const String& Name() const { return _name; };

        virtual CPL_INLINE Type Default() const { return T(); } 

        virtual bool Changed() const = 0;

        CPL_INLINE void Clone(const Param& other)
        {
            CloneNode((Unknown*)&other);
        }

        bool Save(std::ostream& os, bool full) const
        {
            Xml::XmlDocument<char> doc;
            Cpl::Xml::XmlNode<char>* xmlDeclaration = doc.AllocateNode(Cpl::Xml::NodeDeclaration);
            xmlDeclaration->AppendAttribute(doc.AllocateAttribute("version", "1.0"));
            xmlDeclaration->AppendAttribute(doc.AllocateAttribute("encoding", "utf-8"));
            doc.AppendNode(xmlDeclaration);
            this->SaveNode(doc, &doc, full);
            Xml::Print(os, doc);
            return true;
        }

        bool Save(const String& path, bool full) const
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

        bool Load(const char* data, size_t size)
        {
            Xml::File<char> file(data, size);
            return Load(file);
        }

        bool Load(std::istream& is)
        {
            Xml::File<char> file(is);
            return Load(file);
        }

        bool Load(const String& path)
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
        String _name;
        Type _value;

        Param(const String &name)
            : _name(name)
            , _value()
        {
        }

        bool Load(Xml::File<char>& file)
        {
            Xml::XmlDocument<char> doc;
            try
            {
                doc.Parse<0>(file.Data());
            }
            catch (std::exception& e)
            {
                std::cout << "Can't parse xml! There is an exception: " << e.what() << std::endl;
                return false;
            }
            return this->LoadNode(&doc);
        }

        typedef Param<int> Unknown;

        virtual Unknown* End() const = 0;

        virtual void CloneNode(const Unknown* other) = 0;

        virtual bool LoadNode(Xml::XmlNode<char>* xmlParent) = 0;

        virtual void SaveNode(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const = 0;

        template<typename> friend struct Param;
        template<typename> friend struct ParamValue;
        template<typename> friend struct ParamStruct;
        template<typename> friend struct ParamVector;
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamValue : public Cpl::Param<T>
    {
        bool Changed() const override 
        {
            return this->Default() != _value;
        }

    protected:
        typedef Cpl::Param<int> Unknown;

        ParamValue(const String& name)
            : Param(name)
        {
        } 

        Unknown* End() const override 
        { 
            return (Unknown*)(this + 1);
        }

        void CloneNode(const Unknown * other) override
        {
            this->_value = ((ParamValue*)other)->_value;
        }

        bool LoadNode(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
                Cpl::ToVal(xmlCurrent->Value(), this->_value);
            return true;
        }

        void SaveNode(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            xmlCurrent->Value(xmlDoc.AllocateString(Cpl::ToStr(this->_value).c_str()));
            xmlParent->AppendNode(xmlCurrent);
        }

        template<typename> friend struct ParamValue;
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamStruct : public Cpl::Param<T>
    {
        bool Changed() const override
        {
            for (const Unknown* child = this->ChildBeg(); child < this->End(); child = child->End())
            {
                if (child->Changed())
                    return true;
            }
            return false;
        }

    protected:
        typedef Cpl::Param<int> Unknown;

        ParamStruct(const String& name)
            : Param(name)
        {
        }

        Unknown* End() const override 
        {   
            return (Unknown*)(this + 1); 
        }

        CPL_INLINE Unknown* ChildBeg() const
        { 
            return (Unknown*)(&this->_value); 
        }

        void CloneNode(const Unknown * other) override
        {
            const ParamStruct* that = (ParamStruct*)other;
            for (Unknown* tc = this->ChildBeg(), *oc = that->ChildBeg(); tc < this->End(); tc = tc->End(), oc = oc->End())
                tc->CloneNode(oc);
        }

        bool LoadNode(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                for (Unknown* paramChild = this->ChildBeg(); paramChild < this->End(); paramChild = paramChild->End())
                {
                    if (!paramChild->LoadNode(xmlCurrent))
                        return true;
                }
            }
            return true;
        }

        void SaveNode(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            for (const Unknown* paramChild = this->ChildBeg(); paramChild < this->End(); paramChild = paramChild->End())
            {
                if (full || paramChild->Changed())
                    paramChild->SaveNode(xmlDoc, xmlCurrent, full);
            }
            xmlParent->AppendNode(xmlCurrent);
        }

        template<typename> friend struct ParamStruct;
        template<typename> friend struct ParamVector;
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamVector : public Cpl::Param<std::vector<T>>
    {
        bool Changed() const override
        {
            return !_value.empty();
        }

    protected:
        typedef Cpl::Param<int> Unknown;

        ParamVector(const String& name)
            : Param(name)
        {
        }

        Unknown* End() const override 
        { 
            return (Unknown*)(this + 1); 
        }

        CPL_INLINE String ItemName() const { return "item"; }

        CPL_INLINE void Resize(size_t size)
        {
            this->_value.resize(size);
        }

        CPL_INLINE size_t Size() const
        {
            return this->_value.size();
        }

        CPL_INLINE Unknown* ChildBeg(size_t index) const
        {
            return (Unknown*)(this->_value.data() + index);
        }

        void CloneNode(const Unknown * other) override
        {
            const ParamVector * that = (ParamVector*)other;
            Resize(that->Size());
            for (Unknown* tc = this->ChildBeg(0), *oc = that->ChildBeg(0), *end = this->ChildBeg(Size() + 1); tc < end; tc = tc->End(), oc = oc->End())
                tc->CloneNode(oc);
        }

        bool LoadNode(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                Resize(Xml::CountChildren(xmlCurrent));
                Xml::XmlNode<char>* xmlItem = xmlCurrent->FirstNode();
                for (size_t i = 0; i < Size(); ++i)
                {
                    if (ItemName() != xmlItem->Name())
                        return false;
                    Unknown* paramChild = this->ChildBeg(i);
                    const Unknown* paramChildEnd = this->ChildBeg(i + 1);
                    for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                    {
                        if (!paramChild->LoadNode(xmlItem))
                            return true;
                    }
                    xmlItem = xmlItem->NextSibling();
                }
            }
            return true;
        }

        void SaveNode(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            for (size_t i = 0; i < Size(); ++i)
            {
                const Unknown* paramChild = this->ChildBeg(i);
                const Unknown* paramChildEnd = this->ChildBeg(i + 1);
                Xml::XmlNode<char>* xmlItem = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(ItemName().c_str()));
                for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                {
                    if (full || paramChild->Changed())
                        paramChild->SaveNode(xmlDoc, xmlItem, full);
                }
                xmlCurrent->AppendNode(xmlItem);
            }
            xmlParent->AppendNode(xmlCurrent);
        }

        template<typename> friend struct ParamVector;
    };

    //---------------------------------------------------------------------------------------------

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
struct Param_##name : public Cpl::ParamValue<type> \
{ \
    typedef Cpl::ParamValue<type> Base; \
    Param_##name() : Base(#name) { this->_value = this->Default(); } \
    type Default() const override { return value; } \
} name;

#define CPL_PARAM_STRUCT(type, name) \
struct Param_##name : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    Param_##name() : Base(#name) {} \
} name;

#define CPL_PARAM_VECTOR(type, name) \
struct Param_##name : public Cpl::ParamVector<type> \
{ \
    typedef Cpl::ParamVector<type> Base; \
    Param_##name() : Base(#name) {} \
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
    template<> CPL_INLINE Cpl::String ToStr<ns##type>(const ns##type& value) \
    {\
        static thread_local Cpl::Strings names; \
        Cpl::ParseEnumNames(#__VA_ARGS__, names); \
        return (value > ns##type##unknown && value < ns##type##size) ? names[value].substr(sizeof(#type) - 1) : Cpl::String(); \
    }\
    \
    template<> CPL_INLINE void ToVal<ns##type>(const Cpl::String& string, ns##type& value)\
    {\
        value = Cpl::ToEnum<ns##type, ns##type##size>(string); \
    }\
}

#define CPL_PARAM_ENUM0(type, ...) \
    CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) \
    CPL_PARAM_ENUM_CONV(::, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_ENUM1(ns1, type, ...) \
    namespace ns1 { CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } \
    CPL_PARAM_ENUM_CONV(ns1::, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_ENUM2(ns1, ns2, type, ...) \
    namespace ns1 { namespace ns2 { CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } }\
    CPL_PARAM_ENUM_CONV(ns1::ns2::, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_ENUM3(ns1, ns2, ns3, type, ...) \
    namespace ns1 { namespace ns2 { namespace ns3 {CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } } }\
    CPL_PARAM_ENUM_CONV(ns1::ns2::ns3::, type, Unknown, Size, __VA_ARGS__)

#define CPL_PARAM_HOLDER(holder, type, name) \
struct holder : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    holder() : Base(#name) {} \
};
