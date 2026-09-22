/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
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

#include "Test/Test.h"

#include "Cpl/Prop.h"

#include <utility>

namespace Test
{
    struct FirstGroup
    {
        CPL_PROP(String, name, "frame", "frame name");
        CPL_PROP_EX(int, width, 640, 16, 1920, "Image width.");
        CPL_PROP(int, height, 480, "Image height.");
        CPL_PROP(int, reserved, 0, "");
    };

    struct SecondGroup
    {
        CPL_PROP(String, path, "path", "path to model");
        CPL_PROP_EX(int, type, 3, 0, 7, "model type.");
        CPL_PROP(float, coeff, 0.0f, "");
    };

    struct PropConfig
    {
        CPL_PROP_GROUP(FirstGroup, first);
        CPL_PROP_GROUP(SecondGroup, second);
    };

    CPL_PROP_STORAGE(PropStorage, PropConfig, storage);

    struct PlainGroup
    {
        CPL_PROP(int, width, 640, "Image width.");
        CPL_PROP(int, height, 480, "Image height.");
    };

    CPL_PARAM_HOLDER(PlainHolder, PlainGroup, plain);

    struct StringGroup
    {
        CPL_PROP(String, name, "frame", "frame name");
    };

    CPL_PARAM_HOLDER(StringHolder, StringGroup, text);

    struct LimitedGroup
    {
        CPL_PROP_EX(int, width, 640, 16, 1920, "Image width.");
    };

    CPL_PARAM_HOLDER(LimitedHolder, LimitedGroup, limited);

    struct LimitedConfig
    {
        CPL_PROP_GROUP(LimitedGroup, group);
    };

    CPL_PROP_STORAGE(LimitedStorage, LimitedConfig, storage);

    // The assert of the macro has to see each of its three arguments as one operand of the
    // comparison, so each of them in turn is written as an expression of its own: the precedence
    // of ?: takes such an argument apart unless the macro wraps it.
#if !defined(NDEBUG) && (defined(__unix__) || defined(__APPLE__))
    struct TernaryDefaultGroup
    {
        CPL_PROP_EX(int, size, true ? 100 : 0, 0, 10, "Default out of the declared range.");
    };

    struct TernaryMaxGroup
    {
        CPL_PROP_EX(int, size, 100, 0, true ? 10 : 1000, "Default above the declared maximum.");
    };

    struct TernaryMinGroup
    {
        CPL_PROP_EX(int, size, 5, true ? 0 : 1, 10, "Default inside the declared range.");
    };
#endif

    bool PropTest(const Options& options)
    {
        PropStorage test, loaded;

        test().first().width() = 400;
        test().second().coeff() = 3.0;

        test.SetProperty("first.name", "new_name");

        test.Save(options.OutputPath("prop_short.xml"), false);
        test.Save(options.OutputPath("prop_full.xml"), true);

        if (!loaded.Load(options.OutputPath("prop_full.xml")))
            return false;

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "loaded full != original");
            loaded.Save(options.OutputPath("prop_short_loaded.xml"), false);
            loaded.Save(options.OutputPath("prop_full_loaded.xml"), true);
            return false;
        }

        return true;
    }

    bool PropStorageCopyTest(const Options& options)
    {
        PropStorage original;
        original.SetProperty("first.name", "original");

        PropStorage copy(original);

        String value;
        if (!copy.GetProperty("first.name", value) || value != "original")
        {
            CPL_LOG_SS(Error, "The copy holds '" << value << "' instead of 'original'!");
            return false;
        }

        copy.SetProperty("first.name", "copy");

        if (!copy.GetProperty("first.name", value) || value != "copy")
        {
            CPL_LOG_SS(Error, "The copy holds '" << value << "' instead of 'copy'!");
            return false;
        }
        if (!original.GetProperty("first.name", value) || value != "original")
        {
            CPL_LOG_SS(Error, "A property set through the copy changed the original to '" << value << "'!");
            return false;
        }

        return true;
    }

    bool PropStorageAssignTest(const Options& options)
    {
        PropStorage original, assigned;
        original.SetProperty("first.name", "original");

        assigned = original;

        String value;
        if (!assigned.GetProperty("first.name", value) || value != "original")
        {
            CPL_LOG_SS(Error, "The assigned storage holds '" << value << "' instead of 'original'!");
            return false;
        }

        assigned.SetProperty("first.name", "assigned");

        if (!assigned.GetProperty("first.name", value) || value != "assigned")
        {
            CPL_LOG_SS(Error, "The assigned storage holds '" << value << "' instead of 'assigned'!");
            return false;
        }
        if (!original.GetProperty("first.name", value) || value != "original")
        {
            CPL_LOG_SS(Error, "A property set through the assigned storage changed the original to '" << value << "'!");
            return false;
        }

        assigned = original;

        if (!assigned.GetProperty("first.name", value) || value != "original")
        {
            CPL_LOG_SS(Error, "The overwritten storage holds '" << value << "' instead of 'original'!");
            return false;
        }

        assigned.SetProperty("first.name", "overwritten");

        if (!original.GetProperty("first.name", value) || value != "original")
        {
            CPL_LOG_SS(Error, "A property set through the overwritten storage changed the original to '" << value << "'!");
            return false;
        }

        // A storage, a group and a property are assigned through their base classes as well.
        PropStorage source, target;
        source().first().width() = 800;
        source().first().height() = 600;
        source().second().type() = 5;
        Cpl::ParamStorage<PropConfig>& storageBase = target;
        storageBase = source;
        Cpl::Param<SecondGroup>& groupBase = target().second;
        groupBase = original().second;
        Cpl::ParamProp<int>& propBase = target().first().height;
        propBase = original().first().height;
        if (target().first().width() != 800 || target().first().height() != 480 || target().second().type() != 3)
        {
            CPL_LOG_SS(Error, "The storage assigned through its base classes holds width " << target().first().width()
                << ", height " << target().first().height() << " and type " << target().second().type() << "!");
            return false;
        }

        return true;
    }

    // Builds a storage out of a source that dies on return, so the result outlives the object it
    // was built from. A block cannot do it: a copy declared outside the block is built before it.
    static PropStorage CopyOfDestroyedStorage()
    {
        PropStorage source;
        source.SetProperty("first.name", "source");
        return PropStorage(source);
    }

    // The same through a move request, which binds to the copy constructor, since a storage is not
    // movable. The source is read after the request, while it is still alive.
    static PropStorage MoveOfDestroyedStorage(String& sourceValue)
    {
        PropStorage source;
        source.SetProperty("first.name", "source");
        PropStorage moved(std::move(source));
        source.GetProperty("first.name", sourceValue);
        return moved;
    }

    bool PropStorageCopyOutlivesSourceTest(const Options& options)
    {
        PropStorage copy(CopyOfDestroyedStorage());

        copy.SetProperty("first.name", "copy");

        String value;
        if (!copy.GetProperty("first.name", value) || value != "copy")
        {
            CPL_LOG_SS(Error, "The copy of a destroyed storage holds '" << value << "' instead of 'copy'!");
            return false;
        }

        return true;
    }

    bool PropStorageMoveConstructOutlivesSourceTest(const Options& options)
    {
        String sourceValue;
        PropStorage moved(MoveOfDestroyedStorage(sourceValue));

        if (sourceValue != "source")
        {
            CPL_LOG_SS(Error, "A move request left the source holding '" << sourceValue << "' instead of 'source'!");
            return false;
        }

        moved.SetProperty("first.name", "moved");

        String value;
        if (!moved.GetProperty("first.name", value) || value != "moved")
        {
            CPL_LOG_SS(Error, "The storage built out of a destroyed one holds '" << value << "' instead of 'moved'!");
            return false;
        }

        return true;
    }

    bool PropStorageMoveAssignOutlivesSourceTest(const Options& options)
    {
        PropStorage moved;
        String value;
        {
            PropStorage source;
            source.SetProperty("first.name", "source");

            // A storage is not movable, so this binds to the copy assignment.
            moved = std::move(source);

            if (!source.GetProperty("first.name", value) || value != "source")
            {
                CPL_LOG_SS(Error, "A move request left the source holding '" << value << "' instead of 'source'!");
                return false;
            }
        }

        moved.SetProperty("first.name", "moved");

        if (!moved.GetProperty("first.name", value) || value != "moved")
        {
            CPL_LOG_SS(Error, "The storage assigned from a destroyed one holds '" << value << "' instead of 'moved'!");
            return false;
        }

        return true;
    }

    bool PropInStructRoundTripTest(const Options& options)
    {
        PlainHolder test, loaded;

        test().width() = 400;
        test().height() = 300;

        test.Save(options.OutputPath("prop_in_struct_full.xml"), true);

        if (!loaded.Load(options.OutputPath("prop_in_struct_full.xml")))
        {
            CPL_LOG_SS(Error, "Can't load the saved file!");
            return false;
        }

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "The loaded holder has width " << loaded().width()() << " and height " << loaded().height()()
                << " instead of " << test().width()() << " and " << test().height()() << "!");
            return false;
        }

        return true;
    }

    bool PropStringInStructRoundTripTest(const Options& options)
    {
        StringHolder test, loaded;

        // A string property can not be assigned through operator(): the bounds of ParamValidator
        // come from std::numeric_limits<String>, where both of them are an empty string.
        test().name.ToVal("new_name");

        test.Save(options.OutputPath("prop_string_in_struct_full.xml"), true);

        if (!loaded.Load(options.OutputPath("prop_string_in_struct_full.xml")))
        {
            CPL_LOG_SS(Error, "Can't load the saved file!");
            return false;
        }

        if (!loaded.Equal(test))
        {
            CPL_LOG_SS(Error, "The loaded name is '" << loaded().name()() << "' instead of '" << test().name()() << "'!");
            return false;
        }

        return true;
    }

    // The class promises that an XML load writes the parsed value as it is, so a value outside
    // [Min(), Max()] survives the load instead of being replaced by Default() as operator() does.
    // The test pins that promise for both load paths. Inside a structure it guards the override
    // of LoadNodeXml in ParamProp: without it ParamLimited loads through the validator and 9000
    // turns into 640.
    bool PropXmlLoadKeepsOutOfRangeTest(const Options& options)
    {
        const String structPath = options.OutputPath("prop_out_of_range_struct.xml");
        const String structXml = "<limited><width>9000</width></limited>";
        // WriteToFile reports success with -1 and failure with 0.
        if (Cpl::WriteToFile(structPath, structXml.c_str(), structXml.size()) == 0)
        {
            CPL_LOG_SS(Error, "Can\'t write the file of the structure!");
            return false;
        }

        LimitedHolder holder;
        if (!holder.Load(structPath))
        {
            CPL_LOG_SS(Error, "Can\'t load the file of the structure!");
            return false;
        }
        if (holder().width()() != 9000)
        {
            CPL_LOG_SS(Error, "The width of the structure is " << holder().width()() << " instead of 9000!");
            return false;
        }

        const String storagePath = options.OutputPath("prop_out_of_range_storage.xml");
        const String storageXml = "<storage><map><item><first>group.width</first>"
            "<second><value>9000</value></second></item></map></storage>";
        if (Cpl::WriteToFile(storagePath, storageXml.c_str(), storageXml.size()) == 0)
        {
            CPL_LOG_SS(Error, "Can\'t write the file of the storage!");
            return false;
        }

        LimitedStorage storage;
        if (!storage.Load(storagePath))
        {
            CPL_LOG_SS(Error, "Can\'t load the file of the storage!");
            return false;
        }
        if (storage().group().width()() != 9000)
        {
            CPL_LOG_SS(Error, "The width of the storage is " << storage().group().width()() << " instead of 9000!");
            return false;
        }

        return true;
    }

    // An item of the map names a property and has a body, but the body has no value: the file is
    // broken the same way as an item without a body, which fails the load.
    bool PropStorageXmlLoadNoValueTest(const Options& options)
    {
        CPL_LOG_SS(Info, "The load below must fail.");
        return LoadFails<LimitedStorage>(options, "prop_storage_no_value.xml",
            "<storage><map><item><first>group.width</first>"
            "<second><desc>Image width.</desc></second></item></map></storage>");
    }

    bool PropExTernaryArgumentTest(const Options& options)
    {
#if defined(NDEBUG) || !(defined(__unix__) || defined(__APPLE__))
        CPL_LOG_SS(Info, "The test needs an active assert and a child process, it is skipped in this build.");
        return true;
#else
        CPL_LOG_SS(Info, "The first two child processes below must be terminated by the assert of CPL_PROP_EX.");
        const bool defaultConstructed = RunIsolated([]() -> bool
        {
            TernaryDefaultGroup group;
            return group.size()() == 100;
        });

        if (defaultConstructed)
        {
            CPL_LOG_SS(Error, "The assert of CPL_PROP_EX accepted a default outside [Min(), Max()] "
                << "written as a ternary expression!");
            return false;
        }

        const bool maxConstructed = RunIsolated([]() -> bool
        {
            TernaryMaxGroup group;
            return group.size()() == 100;
        });

        if (maxConstructed)
        {
            CPL_LOG_SS(Error, "The assert of CPL_PROP_EX accepted a default above a maximum "
                << "written as a ternary expression!");
            return false;
        }

        // A bound written as a ternary expression must not turn a default that lies inside the
        // range into a failing assert.
        const bool minConstructed = RunIsolated([]() -> bool
        {
            TernaryMinGroup group;
            return group.size()() == 5;
        });

        if (!minConstructed)
        {
            CPL_LOG_SS(Error, "The assert of CPL_PROP_EX rejected a default inside [Min(), Max()] "
                << "with a minimum written as a ternary expression!");
            return false;
        }

        return true;
#endif
    }
}
