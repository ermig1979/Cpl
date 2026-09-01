/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2026 Yermalayeu Ihar.
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
    /*! @ingroup cpl_args
    * \struct ArgsParser
    * \brief Command-line argument parser intended to be used as a base class.
    * \note Inherit ArgsParser and call GetArg, GetArg2, GetArgs or HasArg from the derived class.
    *       An argument name is matched as a prefix of an argv token starting from index 1.
    *       In the default mode the value is the next argv token (`-name value`).
    *       In the alternative mode the value follows '=' in the same token (`-name=value`).
    */
    struct ArgsParser
    {
    public:
        /*!
        * \fn ArgsParser(int argc, char* argv[], bool alt = false)
        * \brief Stores the argument list and the value syntax mode.
        * \param [in] argc - Number of arguments, typically the argc parameter of main.
        * \param [in] argv - Argument vector, typically the argv parameter of main. argv[0] is the application path.
        * \param [in] alt - Value syntax. If false, the value is the next argv token (`-name value`).
        *                   If true, the value follows '=' in the same token (`-name=value`).
        */
        ArgsParser(int argc, char* argv[], bool alt = false)
            : _argc(argc)
            , _argv(argv)
            , _alt(alt)
        {
        }

        /*!
        * \fn const int Argc() const
        * \brief Returns the stored argument count.
        * \return Number of arguments including the application path.
        */
        const int Argc() const { return _argc; }

        /*!
        * \fn char** Argv() const
        * \brief Returns the stored argument vector.
        * \return Pointer to the array of argument strings.
        */
        char** Argv() const { return (char**)_argv; }

        /*!
        * \fn int* ArgcPtr()
        * \brief Returns a pointer to the stored argument count.
        * \return Pointer to argc, for APIs that take int*.
        */
        int* ArgcPtr() { return &_argc; }

        /*!
        * \fn char*** ArgvPtr()
        * \brief Returns a pointer to the stored argument vector.
        * \return Pointer to argv, for APIs that take char***.
        */
        char*** ArgvPtr() { return &_argv; }

    protected:
        /*!
        * \fn String GetArg(const String& name, const String& default_ = String(), bool exit = true, const Strings& valids = Strings())
        * \brief Returns the value of the first argument whose token starts with name.
        * \param [in] name - Argument name prefix, for example "-i" or "--input".
        * \param [in] default_ - Value returned when the argument is absent.
        * \param [in] exit - If true and no default is available, terminate the process when the argument is absent.
        * \param [in] valids - Allowed values. If not empty, a mismatch prints an error and terminates the process.
        * \return The first matching argument value, or default_ if the argument is absent.
        */
        String GetArg(const String& name, const String& default_ = String(), bool exit = true, const Strings& valids = Strings())
        {
            return GetArgs({ name }, { default_ }, exit, valids)[0];
        }

        /*!
        * \fn String GetArg2(const String& name1, const String& name2, const String& default_ = String(), bool exit = true, const Strings& valids = Strings())
        * \brief Returns the value of the first argument whose token starts with name1 or name2.
        *        Useful for a short and a long option, for example "-ll" and "--logLevel".
        * \param [in] name1 - First argument name prefix.
        * \param [in] name2 - Second argument name prefix.
        * \param [in] default_ - Value returned when neither name is present.
        * \param [in] exit - If true and no default is available, terminate the process when the argument is absent.
        * \param [in] valids - Allowed values. If not empty, a mismatch prints an error and terminates the process.
        * \return The first matching argument value, or default_ if both names are absent.
        */
        String GetArg2(const String& name1, const String& name2, const String& default_ = String(), bool exit = true, const Strings& valids = Strings())
        {
            return GetArgs({ name1, name2 }, { default_ }, exit, valids)[0];
        }

        /*!
        * \fn Strings GetArgs(const String& name, const Strings& defaults = Strings(), bool exit = true, const Strings& valids = Strings())
        * \brief Returns every value of arguments whose token starts with name.
        * \param [in] name - Argument name prefix. The same name may appear more than once.
        * \param [in] defaults - Values returned when the argument is absent.
        * \param [in] exit - If true and defaults is empty, print an error and terminate when the argument is absent.
        * \param [in] valids - Allowed values. If not empty, a mismatch prints an error and terminates the process.
        * \return All matching values, or defaults if none were found.
        */
        Strings GetArgs(const String& name, const Strings& defaults = Strings(), bool exit = true, const Strings& valids = Strings())
        {
            return GetArgs(Strings({ name }), defaults, exit, valids);
        }

        /*!
        * \fn Strings GetArgs(const Strings& names, const Strings& defaults = Strings(), bool exit = true, const Strings& valids = Strings())
        * \brief Returns every value of arguments whose token starts with any of the given names.
        * \param [in] names - Argument name prefixes. Each name may appear more than once.
        * \param [in] defaults - Values returned when none of the names is present.
        * \param [in] exit - If true and defaults is empty, print an error and terminate when none of the names is present.
        * \param [in] valids - Allowed values. If not empty, a mismatch prints an error and terminates the process.
        * \return All matching values, or defaults if none were found.
        */
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

        /*!
        * \fn String AppName() const
        * \brief Returns the application path stored in argv[0].
        * \return The first argument, typically the executable path or name.
        */
        String AppName() const
        {
            return _argv[0];
        }

        /*!
        * \fn bool HasArg(const Strings& names) const
        * \brief Checks whether any argument token starts with one of the given names.
        * \param [in] names - Argument name prefixes to look for.
        * \return true if a matching token is found, false otherwise.
        */
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

        /*!
        * \fn bool HasArg(const String& name) const
        * \brief Checks whether any argument token starts with name.
        * \param [in] name - Argument name prefix to look for.
        * \return true if a matching token is found, false otherwise.
        */
        bool HasArg(const String& name) const
        {
            return HasArg(Strings{ name });
        }

        /*!
        * \fn bool HasArg(const String& name0, const String& name1) const
        * \brief Checks whether any argument token starts with name0 or name1.
        *        Useful for a short and a long flag, for example "-h" and "-?".
        * \param [in] name0 - First argument name prefix.
        * \param [in] name1 - Second argument name prefix.
        * \return true if a matching token is found, false otherwise.
        */
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
