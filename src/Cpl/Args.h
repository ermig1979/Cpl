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
    struct ArgsParser
    {
    public:
        ArgsParser(int argc, char* argv[], bool alt = false)
            : _argc(argc)
            , _argv(argv)
            , _alt(alt)
        {
        }

        const int Argc() const { return _argc; }
        char** Argv() const { return (char**)_argv; }

        int* ArgcPtr() { return &_argc; }
        char*** ArgvPtr() { return &_argv; }

    protected:
        String GetArg(const String& name, const String& default_ = String(), bool exit = true, const Strings& valids = Strings())
        {
            return GetArgs({ name }, { default_ }, exit, valids)[0];
        }

        String GetArg2(const String& name1, const String& name2, const String& default_ = String(), bool exit = true, const Strings& valids = Strings())
        {
            return GetArgs({ name1, name2 }, { default_ }, exit, valids)[0];
        }

        Strings GetArgs(const String& name, const Strings& defaults = Strings(), bool exit = true, const Strings& valids = Strings())
        {
            return GetArgs(Strings({ name }), defaults, exit, valids);
        }

        Strings GetArgs(const Strings& names, const Strings& defaults = Strings(), bool exit = true, const Strings & valids = Strings())
        {
            Strings values;
            for (int a = 1; a < _argc; ++a)
            {
                String arg = _argv[a];
                for (size_t n = 0; n < names.size(); ++n)
                {
                    const String& name = names[n];
                    if (arg.substr(0, name.size()) == name)
                    {
                        String value;
                        if (_alt)
                        {
                            if(arg.substr(name.size(), 1) == "=")
                                value = arg.substr(name.size() + 1);
                        }
                        else
                        {
                            a++;
                            if (a < _argc)
                                value = _argv[a];
                        }
                        if (valids.size())
                        {
                            bool found = false;
                            for (size_t v = 0; v < valids.size() && !found; ++v)
                                if (valids[v] == value)
                                    found = true;
                            if (!found)
                            {
                                std::cout << "Argument '";
                                for (size_t i = 0; i < names.size(); ++i)
                                    std::cout << (i ? " | " : "") << names[i];
                                std::cout << "' is equal to " << value << " ! Its valid values : { ";
                                for (size_t i = 0; i < valids.size(); ++i)
                                    std::cout << (i ? " | " : "") << valids[i];
                                std::cout << " }." << std::endl;
                                ::exit(1);
                            }
                        }                        
                        values.push_back(value);
                    }
                }
            }
            if (values.empty())
            {
                if (defaults.empty() && exit)
                {
                    std::cout << "Argument '";
                    for (size_t n = 0; n < names.size(); ++n)
                        std::cout << (n ? " | " : "") << names[n];
                    std::cout << "' is absent!" << std::endl;
                    ::exit(1);
                }
                else
                    return defaults;
            }

            return values;
        }

        String AppName() const
        {
            return _argv[0];
        }

        bool HasArg(const Strings& names) const
        {
            for (int a = 1; a < _argc; ++a)
            {
                String arg = _argv[a];
                for (size_t n = 0; n < names.size(); ++n)
                {
                    const String& name = names[n];
                    if (arg.substr(0, name.size()) == name)
                        return true;
                }
            }
            return false;
        }

        bool HasArg(const String& name) const
        {
            return HasArg({ name });
        }

        bool HasArg(const String& name0, const String& name1) const
        {
            return HasArg({ name0, name1 });
        }

    private:
        int _argc;
        char** _argv;
        bool _alt;
    };
}