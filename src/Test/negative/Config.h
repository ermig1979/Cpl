/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Configuration shared by the negative compilation checks: every case below is a source that must
* NOT compile. The checks are run by prj/cmake/CMakeLists.txt with try_compile.
*/

#pragma once

#include "Cpl/Prop.h"

namespace Negative
{
    struct NestedGroup
    {
        CPL_PROP(int, deep, 1, "A property of a group nested into a group.");
    };

    struct FirstGroup
    {
        CPL_PROP(int, width, 640, "Image width.");
        CPL_PROP_GROUP(NestedGroup, nested);
    };

    struct SecondGroup
    {
        CPL_PROP(int, height, 480, "Image height.");
    };

    struct Config
    {
        CPL_PROP(int, standalone, 1, "A property declared next to the groups.");
        CPL_PROP_GROUP(FirstGroup, first);
        CPL_PROP_GROUP(SecondGroup, second);
    };

    // A group written by hand instead of CPL_PROP_GROUP: the name it gives its node does not
    // repeat the name of the field, so the literal built out of the field names is not the key
    // the storage registers.
    struct HandWrittenConfig
    {
        struct Param_group : public Cpl::ParamStruct<SecondGroup>
        {
            typedef Cpl::ParamStruct<SecondGroup> Base;
            Param_group() : Base("renamed") {}
        } group;
    };

    // A group declared with the holder macro: its node and its StaticName() carry the name
    // "renamed" instead of the name of the field.
    struct HolderConfig
    {
        CPL_PARAM_HOLDER(Param_group, SecondGroup, renamed)
        Param_group group;
    };

    // A storage stands where a group is expected: its properties are its direct children, and the
    // storage that holds it skips it instead of walking it for properties.
    struct StorageConfig
    {
        CPL_PROP_STORAGE(Param_inner, SecondGroup, inner)
        Param_inner inner;
    };

    // A structure of properties declared as a value instead of a group: it has the fields the
    // macro looks for, but the storage does not walk a value for properties.
    struct ValueGroup
    {
        CPL_PROP(int, width, 640, "Image width.");
    };

    inline bool operator == (const ValueGroup& a, const ValueGroup& b) { return a.width() == b.width(); }
    inline bool operator != (const ValueGroup& a, const ValueGroup& b) { return !(a == b); }
    inline std::ostream& operator << (std::ostream& os, const ValueGroup& v) { return os << v.width(); }
    inline std::istream& operator >> (std::istream& is, ValueGroup& v) { int width; is >> width; v.width() = width; return is; }

    struct ValueConfig
    {
        CPL_PARAM_VALUE(ValueGroup, group, ValueGroup());
    };
}
