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
    /*! @ingroup cpl_param
    * \brief Serialization format used by Param::Save and Param::Load.
    */
    enum ParamFormat
    {
        ParamFormatXml,   //!< XML document with a UTF-8 declaration.
        ParamFormatYaml,  //!< YAML document.
        ParamFormatByExt, //!< Choose XML or YAML from the file extension (.xml, .yaml or .yml).
    };

    /*! @ingroup cpl_param
    * \brief Returns a display name of a ParamFormat value.
    * \param [in] format - Format enumerator.
    * \return "XML", "YAML" or "Auto detection by file extension". An empty string if format is out of range.
    */
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

    /*! @ingroup cpl_param
    * \struct Param
    * \brief Named parameter node that stores a value of type T and can be saved or loaded as XML or YAML.
    * \tparam T - Stored value type. A scalar, a user struct of Param fields, std::vector or std::map
    *             depending on the derived class.
    * \note Declare fields with CPL_PARAM_VALUE, CPL_PARAM_LIMITED, CPL_PARAM_STRUCT, CPL_PARAM_VECTOR
    *       or CPL_PARAM_MAP and a CPL_PARAM_HOLDER root. operator() returns the stored value.
    *       Save and Load write or read a tree whose node names are the field names.
    */
    template<class T> struct Param
    {
        typedef T Type; //!< Stored value type.

        /*!
        * \fn const Type& operator () () const
        * \brief Returns a const reference to the stored value.
        * \return The parameter value.
        */
        CPL_INLINE const Type& operator () () const { return _value; }

        /*!
        * \fn Type& operator () ()
        * \brief Returns a mutable reference to the stored value.
        * \return The parameter value.
        */
        CPL_INLINE Type& operator () () { return _value; }

        /*!
        * \fn const String& Name() const
        * \brief Returns the field name used as the XML element or YAML key.
        * \return Name given to the constructor, typically the identifier passed to a CPL_PARAM_* macro.
        */
        CPL_INLINE const String& Name() const { return _name; };

        /*!
        * \fn bool Changed() const
        * \brief Checks whether this node differs from its default.
        * \return true if the stored value is not the default. The exact rule is defined by the derived class.
        */
        virtual bool Changed() const = 0;

        /*!
        * \fn bool Equal(const Param& other) const
        * \brief Compares this node with another node of the same derived type.
        * \param [in] other - Node to compare. Must have the same dynamic type as this node.
        * \return true if the stored values (and children, when present) are equal.
        */
        CPL_INLINE bool Equal(const Param& other) const
        {
            return EqualNode((Unknown*)&other);
        }

        /*!
        * \fn void Clone(const Param& other)
        * \brief Copies the stored value (and children, when present) from another node of the same derived type.
        * \param [in] other - Source node. Must have the same dynamic type as this node.
        */
        CPL_INLINE void Clone(const Param& other)
        {
            CloneNode((Unknown*)&other);
        }

        /*!
        * \fn bool Save(std::ostream& os, bool full, ParamFormat format) const
        * \brief Serializes this node to an output stream.
        * \param [in,out] os - Destination stream.
        * \param [in] full - If true, write every node. If false, omit children that are not Changed().
        * \param [in] format - ParamFormatXml or ParamFormatYaml. ParamFormatByExt is not valid here.
        * \return true on success. false if format is unsupported or YAML serialization throws.
        * \note XML output starts with a declaration version="1.0" encoding="utf-8".
        */
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

        /*!
        * \fn bool Save(const String& path, bool full, ParamFormat format = ParamFormatByExt) const
        * \brief Serializes this node to a file.
        * \param [in] path - Output file path.
        * \param [in] full - If true, write every node. If false, omit children that are not Changed().
        * \param [in] format - Serialization format. ParamFormatByExt (the default) selects XML or YAML
        *                      from the extension of path (.xml, .yaml or .yml).
        * \return true on success. false if the format cannot be detected, the file cannot be opened,
        *         or stream Save fails.
        */
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

        /*!
        * \fn bool Load(const char* data, size_t size, ParamFormat format)
        * \brief Deserializes this node from a memory buffer.
        * \param [in] data - Pointer to the XML or YAML text.
        * \param [in] size - Number of bytes at data.
        * \param [in] format - ParamFormatXml or ParamFormatYaml. ParamFormatByExt is not valid here.
        * \return true on success. false if format is unsupported, the text cannot be parsed,
        *         or a node has an unexpected YAML type.
        */
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

        /*!
        * \fn bool Load(std::istream& is, ParamFormat format)
        * \brief Deserializes this node from an input stream.
        * \param [in,out] is - Source stream positioned at the start of an XML or YAML document.
        * \param [in] format - ParamFormatXml or ParamFormatYaml. ParamFormatByExt is not valid here.
        * \return true on success. false if format is unsupported, the text cannot be parsed,
        *         or a node has an unexpected YAML type.
        */
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

        /*!
        * \fn bool Load(const String& path, ParamFormat format = ParamFormatByExt)
        * \brief Deserializes this node from a file.
        * \param [in] path - Input file path.
        * \param [in] format - Serialization format. ParamFormatByExt (the default) selects XML or YAML
        *                      from the extension of path (.xml, .yaml or .yml).
        * \return true on success. false if the format cannot be detected, the file cannot be opened,
        *         or stream Load fails.
        */
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
                CPL_LOG_SS(Error, "Can't open input file: '" << path << "' !");
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
            if (ext == ".xml")
                format = ParamFormatXml;
            else if (ext == ".yaml" || ext == ".yml")
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

    /*! @ingroup cpl_param
    * \struct ParamValue
    * \brief Scalar parameter that stores a single value of type T and compares it to a default.
    * \tparam T - Stored value type. Must support Cpl::%ToStr, Cpl::%ToVal and operator==.
    * \note Declare instances with CPL_PARAM_VALUE. Changed() is true when the stored value
    *       differs from Default(). XML and YAML represent the value as a named scalar.
    */
    template<class T> struct ParamValue : public Cpl::Param<T>
    {
        typedef T Type; //!< Stored value type.

        /*!
        * \fn bool Changed() const
        * \brief Checks whether the stored value differs from Default().
        * \return true if Default() != the stored value, false otherwise.
        */
        bool Changed() const override 
        {
            return this->Default() != this->_value;
        }

        /*!
        * \fn Type Default() const
        * \brief Returns the default value of this parameter.
        * \return Type() unless a derived class (typically generated by CPL_PARAM_VALUE) overrides it.
        */
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

    /*! @ingroup cpl_param
    * \struct ParamValidator
    * \brief Assignment wrapper that accepts a value only when it lies in [min, max].
    * \tparam T - Stored value type. Must support operator<= and stream insertion for the warning log.
    * \note Returned by ParamLimited::operator(). An out-of-range assignment stores the default
    *       and logs a warning. Conversion to Type and operator() read the current value.
    */
    template<class T> struct ParamValidator
    {
        typedef T Type; //!< Stored value type.

        /*!
        * \fn ParamValidator(Type& value, const Type& def, const Type& min, const Type& max)
        * \brief Binds this validator to an existing storage location and its limits.
        * \param [in,out] value - Storage that assignment will update.
        * \param [in] def - Value written when an assignment is out of range.
        * \param [in] min - Inclusive lower bound of the valid range.
        * \param [in] max - Inclusive upper bound of the valid range.
        */
        ParamValidator(Type& value, const Type& def, const Type& min, const Type& max)
            : _value(value)
            , _def(def)
            , _min(min)
            , _max(max)
        {
        }

        /*!
        * \fn operator Type()
        * \brief Returns a copy of the current stored value.
        * \return The bound value.
        */
        operator Type()
        { 
            return _value; 
        }
        
        /*!
        * \fn const T& operator () () const
        * \brief Returns a const reference to the current stored value.
        * \return The bound value.
        */
        const T& operator () () const
        { 
            return _value; 
        }

        /*!
        * \fn ParamValidator<Type>& operator = (const Type& value)
        * \brief Assigns value when it lies in [min, max]. Otherwise stores the default and logs a warning.
        * \param [in] value - Candidate value.
        * \return Reference to this validator.
        */
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

    /*! @ingroup cpl_param
    * \struct ParamLimited
    * \brief Scalar parameter whose assignment is clamped to an inclusive [Min(), Max()] range.
    * \tparam T - Stored value type. Must be comparable with operator<=.
    * \note Declare instances with CPL_PARAM_LIMITED. operator() returns a ParamValidator, so
    *       `field() = x` rejects values outside the range and restores Default().
    *       Load from XML or YAML applies the same validation.
    */
    template<class T> struct ParamLimited : public Cpl::ParamValue<T>
    {
        typedef T Type; //!< Stored value type.

        /*!
        * \fn ParamValidator<Type> operator () () const
        * \brief Returns a validator bound to the stored value, Default(), Min() and Max().
        * \return ParamValidator that reads the current value and validates assignments.
        */
        CPL_INLINE ParamValidator<Type> operator () () const
        { 
            return ParamValidator<Type>((Type&) this->_value, this->Default(), this->Min(), this->Max()); 
        }

        /*!
        * \fn Type Min() const
        * \brief Returns the inclusive lower bound of the valid range.
        * \return std::numeric_limits<Type>::min() unless a derived class overrides it.
        */
        virtual CPL_INLINE Type Min() const
        {
            return std::numeric_limits<Type>::min();
        }

        /*!
        * \fn Type Max() const
        * \brief Returns the inclusive upper bound of the valid range.
        * \return std::numeric_limits<Type>::max() unless a derived class overrides it.
        */
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

    /*! @ingroup cpl_param
    * \struct ParamStruct
    * \brief Parameter node whose children are the consecutive Param fields of a user struct T.
    * \tparam T - User struct that contains only Param-derived members declared with the CPL_PARAM_* macros.
    * \note Declare instances with CPL_PARAM_STRUCT or CPL_PARAM_STRUCT_MOD, or a root with CPL_PARAM_HOLDER.
    *       Children are walked in memory from the first field of T up to the end of this node.
    *       XML and YAML represent the struct as a named map of those children.
    */
    template<class T> struct ParamStruct : public Cpl::Param<T>
    {
        /*!
        * \fn bool Changed() const
        * \brief Checks whether any child node is Changed().
        * \return true if at least one child reports a change, false otherwise.
        */
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

    /*! @ingroup cpl_param
    * \struct ParamVector
    * \brief Parameter node that stores a std::vector of item structs T.
    * \tparam T - Item type. Typically a user struct of Param fields.
    * \note Declare instances with CPL_PARAM_VECTOR. operator() returns the std::vector<T>.
    *       Changed() is true when the vector is not empty. XML writes each item as an
    *       "item" element; YAML writes a sequence.
    */
    template<class T> struct ParamVector : public Cpl::Param<std::vector<T>>
    {
        /*!
        * \fn bool Changed() const
        * \brief Checks whether the vector contains any items.
        * \return true if the vector is not empty, false otherwise.
        */
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

    /*! @ingroup cpl_param
    * \struct ParamMap
    * \brief Parameter node that stores a std::map from key K to value struct T.
    * \tparam K - Key type. Must support Cpl::%ToStr and Cpl::%ToVal.
    * \tparam T - Value type. Typically a user struct of Param fields.
    * \note Declare instances with CPL_PARAM_MAP. operator() returns the std::map<K, T>.
    *       Changed() is true when the map is not empty. XML writes each entry as an
    *       "item" with "first" (key) and "second" (value). YAML writes a mapping from
    *       the stringified key to the value node.
    */
    template<class K, class T> struct ParamMap : public Cpl::Param<std::map<K, T>>
    {
        /*!
        * \fn bool Changed() const
        * \brief Checks whether the map contains any entries.
        * \return true if the map is not empty, false otherwise.
        */
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

    /*! @ingroup cpl_param
    * \brief Splits a comma-separated enumerator list into names. Does nothing if names is already non-empty.
    * \param [in] data - Comma- and space-separated enumerator identifiers, typically #__VA_ARGS__
    *                    from a CPL_PARAM_ENUM* macro.
    * \param [in,out] names - Destination list. Left unchanged when it already has elements.
    * \note Used by the %ToStr specialization generated for CPL_PARAM_ENUM0, CPL_PARAM_ENUM1,
    *       CPL_PARAM_ENUM2 and CPL_PARAM_ENUM3.
    */
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

/*! @ingroup cpl_param
* \def CPL_PARAM_VALUE(type, name, value)
* \brief Declares a named scalar parameter field with a default value.
* \param type - Stored value type. Must support Cpl::%ToStr, Cpl::%ToVal and operator==.
* \param name - Field name. Used as the member identifier and as the XML/YAML node name.
* \param value - Default value. Changed() is true when the stored value differs from this default.
*/
#define CPL_PARAM_VALUE(type, name, value) \
struct Param_##name : public Cpl::ParamValue<type> \
{ \
    typedef Cpl::ParamValue<type> Base; \
    Param_##name() : Base(#name) { this->_value = this->Default(); } \
    type Default() const override { return value; } \
} name;

/*! @ingroup cpl_param
* \def CPL_PARAM_LIMITED(type, name, value, min, max)
* \brief Declares a named scalar parameter field whose assignment is limited to [min, max].
* \param type - Stored value type. Must be comparable with operator<=.
* \param name - Field name. Used as the member identifier and as the XML/YAML node name.
* \param value - Default value. Must satisfy min <= value <= max.
* \param min - Inclusive lower bound. Out-of-range assignment stores value and logs a warning.
* \param max - Inclusive upper bound.
*/
#define CPL_PARAM_LIMITED(type, name, value, min, max) \
struct Param_##name : public Cpl::ParamLimited<type> \
{ \
    typedef Cpl::ParamLimited<type> Base; \
    Param_##name() : Base(#name) { assert(min <= value && value <= max); this->_value = this->Default(); } \
    type Default() const override { return value; } \
    type Min() const override { return min; } \
    type Max() const override { return max; } \
} name;

/*! @ingroup cpl_param
* \def CPL_PARAM_STRUCT(type, name)
* \brief Declares a nested parameter structure field.
* \param type - User struct whose members are Param fields declared with the CPL_PARAM_* macros.
* \param name - Field name. Used as the member identifier and as the XML/YAML node name.
*/
#define CPL_PARAM_STRUCT(type, name) \
struct Param_##name : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    Param_##name() : Base(#name) {} \
} name;

/*! @ingroup cpl_param
* \def CPL_PARAM_STRUCT_MOD(type, name, value)
* \brief Declares a nested parameter structure field and initializes it from an instance of type.
* \param type - User struct whose members are Param fields declared with the CPL_PARAM_* macros.
* \param name - Field name. Used as the member identifier and as the XML/YAML node name.
* \param value - Initial value of type copied into the field.
*/
#define CPL_PARAM_STRUCT_MOD(type, name, value) \
struct Param_##name : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    Param_##name() : Base(#name) { this->_value = value; } \
} name;

/*! @ingroup cpl_param
* \def CPL_PARAM_VECTOR(type, name)
* \brief Declares a vector parameter field of item type.
* \param type - Item type. Typically a user struct of Param fields.
* \param name - Field name. Used as the member identifier and as the XML/YAML node name.
* \note operator() returns std::vector of type. XML items are named "item"; YAML uses a sequence.
*/
#define CPL_PARAM_VECTOR(type, name) \
struct Param_##name : public Cpl::ParamVector<type> \
{ \
    typedef Cpl::ParamVector<type> Base; \
    Param_##name() : Base(#name) {} \
} name;

/*! @ingroup cpl_param
* \def CPL_PARAM_MAP(key, type, name)
* \brief Declares a map parameter field from key to type.
* \param key - Key type. Must support Cpl::%ToStr and Cpl::%ToVal.
* \param type - Value type. Typically a user struct of Param fields.
* \param name - Field name. Used as the member identifier and as the XML/YAML node name.
* \note operator() returns std::map of key to type. XML items use "item" / "first" / "second";
*       YAML uses a mapping from the stringified key.
*/
#define CPL_PARAM_MAP(key, type, name) \
struct Param_##name : public Cpl::ParamMap<key, type> \
{ \
    typedef Cpl::ParamMap<key, type> Base; \
    Param_##name() : Base(#name) {} \
} name;

/*! @ingroup cpl_param
* \def CPL_PARAM_ENUM_DECL(type, unknown, size, ...)
* \brief Declares an enum type with unknown (= -1) and size sentinels concatenated to the type name.
* \param type - Enumeration type name. Enumerator identifiers should start with this name.
* \param unknown - Suffix of the unknown sentinel, typically Unknown.
* \param size - Suffix of the count sentinel, typically Size.
* \param ... - Comma-separated enumerator identifiers.
* \note Used by CPL_PARAM_ENUM0, CPL_PARAM_ENUM1, CPL_PARAM_ENUM2 and CPL_PARAM_ENUM3.
*/
#define CPL_PARAM_ENUM_DECL(type, unknown, size, ...) \
enum type \
{ \
    type##unknown = -1, \
    ##__VA_ARGS__, \
    type##size \
};

/*! @ingroup cpl_param
* \def CPL_PARAM_ENUM_CONV(ns, type, unknown, size, ...)
* \brief Specializes Cpl::%ToStr and Cpl::%ToVal for an enum declared by CPL_PARAM_ENUM_DECL.
* \param ns - Enclosing namespace of the enum, or CPL_NOARG for the global namespace.
* \param type - Enumeration type name.
* \param unknown - Suffix of the unknown sentinel, typically Unknown.
* \param size - Suffix of the count sentinel, typically Size.
* \param ... - Comma-separated enumerator identifiers. %ToStr strips the type name prefix from each name.
* \note %ToVal matches names case-insensitively through Cpl::%ToEnum. Used by CPL_PARAM_ENUM0 through CPL_PARAM_ENUM3.
*/
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

/*! @ingroup cpl_param
* \def CPL_NOARG
* \brief Empty placeholder passed as the namespace argument of CPL_PARAM_ENUM_CONV from CPL_PARAM_ENUM0.
*/
#define CPL_NOARG

/*! @ingroup cpl_param
* \def CPL_PARAM_ENUM0(type, ...)
* \brief Declares an enum in the current namespace and specializations of Cpl::%ToStr and Cpl::%ToVal.
* \param type - Enumeration type name. Adds typeUnknown = -1 and typeSize sentinels.
* \param ... - Comma-separated enumerator identifiers. Each name should start with type.
*/
#define CPL_PARAM_ENUM0(type, ...) \
    CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) \
    CPL_PARAM_ENUM_CONV(CPL_NOARG, type, Unknown, Size, __VA_ARGS__)

/*! @ingroup cpl_param
* \def CPL_PARAM_ENUM1(ns1, type, ...)
* \brief Declares an enum in namespace ns1 and specializations of Cpl::%ToStr and Cpl::%ToVal.
* \param ns1 - Enclosing namespace.
* \param type - Enumeration type name. Adds typeUnknown = -1 and typeSize sentinels.
* \param ... - Comma-separated enumerator identifiers. Each name should start with type.
*/
#define CPL_PARAM_ENUM1(ns1, type, ...) \
    namespace ns1 { CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } \
    CPL_PARAM_ENUM_CONV(ns1, type, Unknown, Size, __VA_ARGS__)

/*! @ingroup cpl_param
* \def CPL_PARAM_ENUM2(ns1, ns2, type, ...)
* \brief Declares an enum in namespace ns1::ns2 and specializations of Cpl::%ToStr and Cpl::%ToVal.
* \param ns1 - Outer namespace.
* \param ns2 - Inner namespace.
* \param type - Enumeration type name. Adds typeUnknown = -1 and typeSize sentinels.
* \param ... - Comma-separated enumerator identifiers. Each name should start with type.
*/
#define CPL_PARAM_ENUM2(ns1, ns2, type, ...) \
    namespace ns1 { namespace ns2 { CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } }\
    CPL_PARAM_ENUM_CONV(ns1::ns2, type, Unknown, Size, __VA_ARGS__)

/*! @ingroup cpl_param
* \def CPL_PARAM_ENUM3(ns1, ns2, ns3, type, ...)
* \brief Declares an enum in namespace ns1::ns2::ns3 and specializations of Cpl::%ToStr and Cpl::%ToVal.
* \param ns1 - Outer namespace.
* \param ns2 - Middle namespace.
* \param ns3 - Inner namespace.
* \param type - Enumeration type name. Adds typeUnknown = -1 and typeSize sentinels.
* \param ... - Comma-separated enumerator identifiers. Each name should start with type.
*/
#define CPL_PARAM_ENUM3(ns1, ns2, ns3, type, ...) \
    namespace ns1 { namespace ns2 { namespace ns3 {CPL_PARAM_ENUM_DECL(type, Unknown, Size, __VA_ARGS__) } } }\
    CPL_PARAM_ENUM_CONV(ns1::ns2::ns3, type, Unknown, Size, __VA_ARGS__)

/*! @ingroup cpl_param
* \def CPL_PARAM_HOLDER(holder, type, name)
* \brief Declares a root ParamStruct used to Save and Load a parameter tree.
* \param holder - Name of the generated holder type.
* \param type - User struct whose members are Param fields declared with the CPL_PARAM_* macros.
* \param name - Root XML element or YAML key written by Save and expected by Load.
*/
#define CPL_PARAM_HOLDER(holder, type, name) \
struct holder : public Cpl::ParamStruct<type> \
{ \
    typedef Cpl::ParamStruct<type> Base; \
    holder() : Base(#name) {} \
};
