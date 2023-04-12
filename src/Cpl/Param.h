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

#include "Cpl/String.h"
#include "Cpl/Log.h"
#include "Cpl/Xml.h"
#include "Cpl/Yaml.h"
#include "Cpl/File.h"

namespace Cpl
{
    enum ParamFormat
    {
        ParamFormatXml,
        ParamFormatYaml,
        ParamFormatByExt,
    };

    CPL_INLINE String ToStr(ParamFormat format)
    {
        static const char* names[] = { "XML", "YAML", "Auto detection by file extension" };
        return format >= ParamFormatXml && format <= ParamFormatByExt ? names[format] : "";
    }

    //---------------------------------------------------------------------------------------------

    template<typename> struct ParamValue;
    template<typename> struct ParamLimited;
    template<typename> struct ParamStruct;
    template<typename> struct ParamVector;
    template<typename> struct ParamVectorV2;
    template<typename> struct ParamProp;
    template<typename> struct ParamStorage;
    template<typename, typename> struct ParamMap;
    template<typename, typename> struct ParamMapV2;

    template<class T> struct Param
    {
        typedef T Type;

        CPL_INLINE const Type& operator () () const { return _value; }
        CPL_INLINE Type& operator () () { return _value; }

        CPL_INLINE const String& Name() const { return _name; };

        virtual bool Changed() const = 0;

        CPL_INLINE bool Equal(const Param& other) const
        {
            return EqualNode((Unknown*)&other);
        }

        CPL_INLINE void Clone(const Param& other)
        {
            CloneNode((Unknown*)&other);
        }

        bool Save(std::ostream& os, bool full, ParamFormat format) const
        {
            if (format == ParamFormatXml)
            {
                Xml::XmlDocument<char> doc;
                Cpl::Xml::XmlNode<char>* xmlDeclaration = doc.AllocateNode(Cpl::Xml::NodeDeclaration);
                xmlDeclaration->AppendAttribute(doc.AllocateAttribute("version", "1.0"));
                xmlDeclaration->AppendAttribute(doc.AllocateAttribute("encoding", "utf-8"));
                doc.AppendNode(xmlDeclaration);
                this->SaveNodeXml(doc, &doc, full);
                Xml::Print(os, doc);
            }
            else if (format == ParamFormatYaml)
            {
                Yaml::Node root;
                this->SaveNodeYaml(root, full);
                try
                {
                    Yaml::Serialize(root, os);
                }
                catch (const Yaml::Exception e)
                {
                    CPL_LOG_SS(Error, "Exception " << e.GetType() << ": " << e.what());
                    return false;
                }
            }
            else
            {
                CPL_LOG_SS(Error, "Can't save Param in '" << ToStr(format) << "' format !");
                return false;
            }
            return true;
        }

        bool Save(const String& path, bool full, ParamFormat format = ParamFormatByExt) const
        {
            if (!DetectFormat(path, format))
                return false;
            bool result = false;
            std::ofstream ofs(path.c_str());
            if (ofs.is_open())
            {
                result = this->Save(ofs, full, format);
                ofs.close();
            }
            else
            {
                CPL_LOG_SS(Error, "Can't open output file: '" << path << "' !");
            }
            return result;
        }

        bool Load(const char* data, size_t size, ParamFormat format)
        {
            if (format == ParamFormatXml)
            {
                Xml::File<char> file(data, size);
                return LoadXml(file);
            }
            else if (format == ParamFormatYaml)
            {
                Yaml::Node root;
                try
                {
                    Yaml::Parse(root, data, size);
                }
                catch (const Yaml::Exception e)
                {
                    CPL_LOG_SS(Error, "Exception " << e.GetType() << ": " << e.what());
                    return false;
                }
                return LoadNodeYaml(root);
            }
            else
            {
                CPL_LOG_SS(Error, "Can't load Param in '" << ToStr(format) << "' format !");
                return false;
            }
        }

        bool Load(std::istream& is, ParamFormat format)
        {
            if (format == ParamFormatXml)
            {
                Xml::File<char> file(is);
                return LoadXml(file);
            }
            else if (format == ParamFormatYaml)
            {
                Yaml::Node root;
                try
                {
                    Yaml::Parse(root, is);
                }
                catch (const Yaml::Exception e)
                {
                    CPL_LOG_SS(Error, "Exception " << e.GetType() << ": " << e.what());
                    return false;
                }
                return LoadNodeYaml(root);
            }
            else
            {
                CPL_LOG_SS(Error, "Can't load Param in '" << ToStr(format) << "' format !");
                return false;
            }
        }

        bool Load(const String& path, ParamFormat format = ParamFormatByExt)
        {
            if (!DetectFormat(path, format))
                return false;
            bool result = false;
            std::ifstream ifs(path.c_str());
            if (ifs.is_open())
            {
                result = this->Load(ifs, format);
                ifs.close();
            }
            else
            {
                CPL_LOG_SS(Error, "Can't open intput file: '" << path << "' !");
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

        bool DetectFormat(const String & path, ParamFormat &format) const
        {
            if (format != ParamFormatByExt)
                return true;
            String ext = ToLowerCase(ExtensionByPath(path));
            if (ext == "xml")
                format = ParamFormatXml;
            else if (ext == "yaml" || ext == "yml")
                format = ParamFormatYaml;
            else
            {
                CPL_LOG_SS(Error, "This file extension '" << ext << "' is not recognized! ");
                return false;
            }
            return true;
        }

        bool LoadXml(Xml::File<char>& file)
        {
            Xml::XmlDocument<char> doc;
            try
            {
                doc.Parse<0>(file.Data(), file.Size());
            }
            catch (std::exception& e)
            {
                CPL_LOG_SS(Error, "Can't parse xml! There is an exception: " << e.what());
                return false;
            }
            return this->LoadNodeXml(&doc);
        }

        typedef Param<int> Unknown;

        virtual Unknown* End() const = 0;

        virtual bool EqualNode(const Unknown* other) const = 0;

        virtual void CloneNode(const Unknown* other) = 0;

        virtual bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) = 0;

        virtual void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const = 0;

        virtual bool LoadNodeYaml(Yaml::Node& node) = 0;

        virtual void SaveNodeYaml(Yaml::Node & node, bool full) const = 0;

        template<typename> friend struct Param;
        template<typename> friend struct ParamValue;
        template<typename> friend struct ParamLimited;
        template<typename> friend struct ParamStruct;
        template<typename> friend struct ParamVector;
        template<typename> friend struct ParamVectorV2;
        template<typename> friend struct ParamProp;
        template<typename> friend struct ParamStorage;
        template<typename, typename> friend struct ParamMap;
        template<typename, typename> friend struct ParamMapV2;
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamValue : public Cpl::Param<T>
    {
        typedef T Type;

        bool Changed() const override 
        {
            return this->Default() != this->_value;
        }

        virtual CPL_INLINE Type Default() const
        {
            return Type();
        }

    protected:
        typedef Cpl::Param<T> Base;
        typedef Cpl::Param<int> Unknown;

        ParamValue(const String& name)
            : Base(name)
        {
        } 

        Unknown* End() const override 
        { 
            return (Unknown*)(this + 1);
        }

        bool EqualNode(const Unknown* other) const override
        {
            return this->_value == ((ParamValue*)other)->_value;
        }

        void CloneNode(const Unknown * other) override
        {
            this->_value = ((ParamValue*)other)->_value;
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
                Cpl::ToVal(xmlCurrent->Value(), this->_value);
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            xmlCurrent->Value(xmlDoc.AllocateString(Cpl::ToStr(this->_value).c_str()));
            xmlParent->AppendNode(xmlCurrent);
        }

        bool LoadNodeYaml(Yaml::Node& node) override
        {
            Yaml::Node & current = node[this->Name()];
            if (current.Type() != Yaml::Node::None)
            {
                if (current.Type() != Yaml::Node::ScalarType)
                    return false;
                String string = current.As<String>();
                if(string != "\n")
                    Cpl::ToVal(current.As<String>(), this->_value);
            }
            return true;
        }

        void SaveNodeYaml(Yaml::Node& node, bool full) const override
        {
            node[this->Name()] = Cpl::ToStr(this->_value);
        }
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamValidator
    {
        typedef T Type;

        ParamValidator(Type& value, const Type& def, const Type& min, const Type& max)
            : _value(value)
            , _def(def)
            , _min(min)
            , _max(max)
        {
        }

        operator Type()
        { 
            return _value; 
        }
        
        const T& operator () () const
        { 
            return _value; 
        }

        ParamValidator<Type>& operator = (const Type& value)
        {
            if (this->_min <= value && value <= this->_max)
                this->_value = value;
            else
            {
                this->_value = this->_def;
                CPL_LOG_SS(Warning, "Value " << value << " is out of valid range [" << this->_min 
                    << " .. " << this->_max << "], default value " << this->_def << " will be used!");
            }
            return *this;
        }

    private:
        Type& _value;
        Type _def, _min, _max;
    };

    template<class T> struct ParamLimited : public Cpl::ParamValue<T>
    {
        typedef T Type;

        CPL_INLINE ParamValidator<Type> operator () () const
        { 
            return ParamValidator<Type>((Type&) this->_value, this->Default(), this->Min(), this->Max()); 
        }

        virtual CPL_INLINE Type Min() const
        {
            return std::numeric_limits<Type>::min();
        }

        virtual CPL_INLINE Type Max() const
        {
            return std::numeric_limits<Type>::max();
        }

    protected:
        typedef Cpl::ParamValue<T> Base;
        typedef Cpl::Param<int> Unknown;

        ParamLimited(const String& name)
            : Base(name)
        {
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                T value;
                Cpl::ToVal(xmlCurrent->Value(), value);
                (*this)() = value;
            }
            return true;
        }

        bool LoadNodeYaml(Yaml::Node& node) override
        {
            Yaml::Node& current = node[this->Name()];
            if (current.Type() != Yaml::Node::None)
            {
                if (current.Type() != Yaml::Node::ScalarType)
                    return false;
                T value;
                Cpl::ToVal(current.As<String>(), value);
                (*this)() = value;
            }
            return true;
        }
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
        typedef Cpl::Param<T> Base;
        typedef Cpl::Param<int> Unknown;

        ParamStruct(const String& name)
            : Base(name)
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

        bool EqualNode(const Unknown* other) const override
        {
            const ParamStruct* that = (ParamStruct*)other;
            for (Unknown* tc = this->ChildBeg(), *oc = that->ChildBeg();; tc = tc->End(), oc = oc->End())
            {
                if (tc >= this->End())
                    return oc >= that->End();
                if (oc >= that->End())
                    return tc >= this->End();
                if (!tc->EqualNode(oc))
                    return false;
            }
            return true;
        }

        void CloneNode(const Unknown * other) override
        {
            const ParamStruct* that = (ParamStruct*)other;
            for (Unknown* tc = this->ChildBeg(), *oc = that->ChildBeg(); tc < this->End(); tc = tc->End(), oc = oc->End())
                tc->CloneNode(oc);
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                for (Unknown* paramChild = this->ChildBeg(); paramChild < this->End(); paramChild = paramChild->End())
                {
                    if (!paramChild->LoadNodeXml(xmlCurrent))
                        return true;
                }
            }
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            for (const Unknown* paramChild = this->ChildBeg(); paramChild < this->End(); paramChild = paramChild->End())
            {
                if (full || paramChild->Changed())
                    paramChild->SaveNodeXml(xmlDoc, xmlCurrent, full);
            }
            xmlParent->AppendNode(xmlCurrent);
        }

        bool LoadNodeYaml(Yaml::Node& node) override
        {
            Yaml::Node & current = node[this->Name()];
            if (current.Type() != Yaml::Node::None)
            {
                if (current.Type() != Yaml::Node::MapType)
                    return false;
                for (Unknown* paramChild = this->ChildBeg(); paramChild < this->End(); paramChild = paramChild->End())
                {
                    if (!paramChild->LoadNodeYaml(current))
                        return true;
                }
            }
            return true;
        }

        void SaveNodeYaml(Yaml::Node& node, bool full) const override
        {
            Yaml::Node& current = node[this->Name()];
            for (const Unknown* paramChild = this->ChildBeg(); paramChild < this->End(); paramChild = paramChild->End())
            {
                if (full || paramChild->Changed())
                    paramChild->SaveNodeYaml(current, full);
            }
        }

        template<typename> friend struct ParamStorage;
    };

    //---------------------------------------------------------------------------------------------

    template<class T> struct ParamVector : public Cpl::Param<std::vector<T>>
    {
        bool Changed() const override
        {
            return !this->_value.empty();
        }

    protected:
        typedef Cpl::Param<std::vector<T>> Base;
        typedef Cpl::Param<int> Unknown;

        ParamVector(const String& name)
            : Base(name)
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

        bool EqualNode(const Unknown* other) const override
        {
            const ParamVector* that = (ParamVector*)other;
            if (this->Size() != that->Size())
                return false;
            for (Unknown* tc = this->ChildBeg(0), *oc = that->ChildBeg(0), *end = this->ChildBeg(Size()); tc < end; tc = tc->End(), oc = oc->End())
            {
                if (!tc->EqualNode(oc))
                    return false;
            }
            return true;
        }

        void CloneNode(const Unknown * other) override
        {
            const ParamVector * that = (ParamVector*)other;
            Resize(that->Size());
            for (Unknown* tc = this->ChildBeg(0), *oc = that->ChildBeg(0), *end = this->ChildBeg(Size()); tc < end; tc = tc->End(), oc = oc->End())
                tc->CloneNode(oc);
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
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
                        if (!paramChild->LoadNodeXml(xmlItem))
                            return true;
                    }
                    xmlItem = xmlItem->NextSibling();
                }
            }
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
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
                        paramChild->SaveNodeXml(xmlDoc, xmlItem, full);
                }
                xmlCurrent->AppendNode(xmlItem);
            }
            xmlParent->AppendNode(xmlCurrent);
        }

        bool LoadNodeYaml(Yaml::Node& node) override
        {
            Yaml::Node& current = node[this->Name()];
            if (current.Type() != Yaml::Node::None)
            {
                if (current.Type() != Yaml::Node::SequenceType)
                    return false;
                Resize(current.Size());
                for (size_t i = 0; i < Size(); ++i)
                {
                    Unknown* paramChild = this->ChildBeg(i);
                    const Unknown* paramChildEnd = this->ChildBeg(i + 1);
                    for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                    {
                        if (!paramChild->LoadNodeYaml(current[i]))
                            return true;
                    }
                }
            }
            return true;
        }

        void SaveNodeYaml(Yaml::Node& node, bool full) const override
        {
            Yaml::Node& current = node[this->Name()];
            for (size_t i = 0; i < Size(); ++i)
            {
                Yaml::Node& childNode = current.PushBack();
                const Unknown* paramChild = this->ChildBeg(i);
                const Unknown* paramChildEnd = this->ChildBeg(i + 1);
                bool saved = false;
                for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                {
                    if (full || paramChild->Changed())
                    {
                        paramChild->SaveNodeYaml(childNode, full);
                        saved = true;
                    }
                }
                if(!saved)
                    this->ChildBeg(i)->SaveNodeYaml(childNode, full);
            }
        }
    };

    //---------------------------------------------------------------------------------------------

    template<class K, class T> struct ParamMap : public Cpl::Param<std::map<K, T>>
    {
        bool Changed() const override
        {
            return !this->_value.empty();
        }

    protected:
        typedef Cpl::Param<std::map<K, T>> Base;
        typedef Cpl::Param<int> Unknown;
        typedef std::map<K, T> Map;

        ParamMap(const String& name)
            : Base(name)
        {
        }

        Unknown* End() const override
        {
            return (Unknown*)(this + 1);
        }

        CPL_INLINE String ItemName() const { return "item"; }
        CPL_INLINE String KeyName() const { return "first"; }
        CPL_INLINE String ValueName() const { return "second"; }

        CPL_INLINE Unknown* ChildBeg(const T & value) const
        {
            return (Unknown*)(&value);
        }

        CPL_INLINE Unknown* ChildEnd(const T& value) const
        {
            return (Unknown*)(&value + 1);
        }

        bool EqualNode(const Unknown* other) const override
        {
            const ParamMap* that = (ParamMap*)other;
            if (this->_value.size() != that->_value.size())
                return false;
            for (typename Map::const_iterator o = that->_value.begin(), t = this->_value.begin(); o != that->_value.end(); ++o, ++t)
            {
                if (o->first != t->first)
                    return false;
                const Unknown* oChild = that->ChildBeg(o->second);
                const Unknown* oChildEnd = that->ChildEnd(o->second);
                const Unknown* tChild = this->ChildBeg(t->second);
                const Unknown* tChildEnd = this->ChildEnd(t->second);
                for (;; oChild = oChild->End(), tChild = tChild->End())
                {
                    if (tChild >= tChildEnd)
                        return oChild >= oChildEnd;
                    if (oChild >= oChildEnd)
                        return tChild >= tChildEnd;
                    if (!oChild->EqualNode(tChild))
                        return false;
                }
            }
            return true;
        }

        void CloneNode(const Unknown* other) override
        {
            const ParamMap* that = (ParamMap*)other;
            for (typename Map::const_iterator it = that->_value.begin(); it != that->_value.end(); ++it)
            {
                T& value = this->_value[it->first];
                const Unknown* srcChild = that->ChildBeg(it->second);
                const Unknown* srcChildEnd = that->ChildEnd(it->second);
                Unknown* dstChild = this->ChildBeg(value);
                Unknown* dstChildEnd = this->ChildEnd(value);
                for (; srcChild < srcChildEnd; srcChild = srcChild->End(), dstChild = dstChild->End())
                    dstChild->CloneNode(srcChild);
            }
        }

        bool LoadNodeXml(Xml::XmlNode<char>* xmlParent) override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlParent->FirstNode(this->Name().c_str());
            if (xmlCurrent)
            {
                size_t size = Xml::CountChildren(xmlCurrent);
                Xml::XmlNode<char>* xmlItem = xmlCurrent->FirstNode();
                for (size_t i = 0; i < size; ++i)
                {
                    if (ItemName() != xmlItem->Name())
                        return false;
                    Xml::XmlNode<char>* xmlKey = xmlItem->FirstNode(KeyName().c_str());
                    if (xmlKey)
                    {
                        K key;
                        Cpl::ToVal(xmlKey->Value(), key);
                        T & value = this->_value[key];
                        Xml::XmlNode<char>* xmlValue = xmlItem->FirstNode(ValueName().c_str());
                        if (xmlValue)
                        {
                            Unknown* paramChild = ChildBeg(value);
                            Unknown* paramChildEnd = ChildEnd(value);
                            for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                            {
                                if (!paramChild->LoadNodeXml(xmlValue))
                                    return true;
                            }
                        }
                    }
                    xmlItem = xmlItem->NextSibling();
                }
            }
            return true;
        }

        void SaveNodeXml(Xml::XmlDocument<char>& xmlDoc, Xml::XmlNode<char>* xmlParent, bool full) const override
        {
            Xml::XmlNode<char>* xmlCurrent = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(this->Name().c_str()));
            for (typename Map::const_iterator it = this->_value.begin(); it != this->_value.end(); ++it)
            {
                Xml::XmlNode<char>* xmlItem = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(ItemName().c_str()));

                Xml::XmlNode<char>* xmlKey = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(KeyName().c_str()));
                xmlKey->Value(xmlDoc.AllocateString(Cpl::ToStr(it->first).c_str()));
                xmlItem->AppendNode(xmlKey);

                Xml::XmlNode<char>* xmlValue = xmlDoc.AllocateNode(Xml::NodeElement, xmlDoc.AllocateString(ValueName().c_str()));
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

        bool LoadNodeYaml(Yaml::Node& node) override
        {
            Yaml::Node& current = node[this->Name()];
            if (current.Type() != Yaml::Node::None)
            {
                if (current.Type() != Yaml::Node::MapType)
                    return false;
                for (Yaml::Iterator it = current.Begin(), end = current.End(); it != end; it++)
                {
                    K key;
                    Cpl::ToVal((*it).first, key);
                    T& value = this->_value[key];
                    if ((*it).second.Type() != Yaml::Node::None)
                    {
                        Unknown* paramChild = ChildBeg(value);
                        Unknown* paramChildEnd = ChildEnd(value);
                        for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                        {
                            if (!paramChild->LoadNodeYaml((*it).second))
                                return true;
                        }
                    }
                }
            }
            return true;
        }

        void SaveNodeYaml(Yaml::Node& node, bool full) const override
        {
            Yaml::Node& current = node[this->Name()];
            for (typename Map::const_iterator it = this->_value.begin(); it != this->_value.end(); ++it)
            {
                Yaml::Node& childNode = current[Cpl::ToStr(it->first)];
                const Unknown* paramChild = this->ChildBeg(it->second);
                const Unknown* paramChildEnd = this->ChildEnd(it->second);
                bool saved = false;
                for (; paramChild < paramChildEnd; paramChild = paramChild->End())
                {
                    if (full || paramChild->Changed())
                    {
                        paramChild->SaveNodeYaml(childNode, full);
                        saved = true;
                    }
                }
                if (!saved)
                    this->ChildBeg(it->second)->SaveNodeYaml(childNode, full);
            }
        }
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

//-------------------------------------------------------------------------------------------------

#define CPL_PARAM_VALUE(type, name, value) \
struct Param_##name : public Cpl::ParamValue<type> \
{ \
    typedef Cpl::ParamValue<type> Base; \
    Param_##name() : Base(#name) { this->_value = this->Default(); } \
    type Default() const override { return value; } \
} name;

#define CPL_PARAM_LIMITED(type, name, value, min, max) \
struct Param_##name : public Cpl::ParamLimited<type> \
{ \
    typedef Cpl::ParamLimited<type> Base; \
    Param_##name() : Base(#name) { assert(min <= value && value <= max); this->_value = this->Default(); } \
    type Default() const override { return value; } \
    type Min() const override { return min; } \
    type Max() const override { return max; } \
} name;

#define CPL_PARAM_STRUCT(type, name) \
struct Param_##name : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    Param_##name() : Base(#name) {} \
} name;

#define CPL_PARAM_STRUCT_MOD(type, name, value) \
struct Param_##name : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    Param_##name() : Base(#name) { this->_value = value; } \
} name;

#define CPL_PARAM_VECTOR(type, name) \
struct Param_##name : public Cpl::ParamVector<type> \
{ \
    typedef Cpl::ParamVector<type> Base; \
    Param_##name() : Base(#name) {} \
} name;

#define CPL_PARAM_MAP(key, type, name) \
struct Param_##name : public Cpl::ParamMap<key, type> \
{ \
    typedef Cpl::ParamMap<key, type> Base; \
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
    template<> CPL_INLINE Cpl::String ToStr<ns::type>(const ns::type& value) \
    {\
        static thread_local Cpl::Strings names; \
        Cpl::ParseEnumNames(#__VA_ARGS__, names); \
        return (value > ns::type##unknown && value < ns::type##size) ? names[value].substr(sizeof(#type) - 1) : Cpl::String(); \
    }\
    \
    template<> CPL_INLINE void ToVal<ns::type>(const Cpl::String& string, ns::type& value)\
    {\
        value = Cpl::ToEnum<ns::type, ns::type##size>(string); \
    }\
}

#define CPL_NOARG

#define CPL_PARAM_ENUM0(type, ...) \
    CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) \
    CPL_PARAM_ENUM_CONV(CPL_NOARG, type, Unknown, Size, __VA_ARGS__)

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
struct holder : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    holder() : Base(#name) {} \
};
