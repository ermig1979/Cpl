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

#include "Cpl/Log.h"

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#endif

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#if defined(__GNUC__) && (__GNUC__ <= 10 || __cplusplus < 201703L)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif defined(_MSC_VER) && _MSC_VER <= 1900
#include <filesystem>
namespace fs = std::tr2::sys;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace
{
    // https://www.geeksforgeeks.org/wildcard-character-matching/
    inline bool Match(const char* first, const char* second, size_t firstLen, size_t secondLen)
    {
        // If we reach at the end of both strings, we are done
        if (firstLen == 0 && secondLen == 0)
            return true;

        // Make sure that the characters after '*' are present
        // in second string. This function assumes that the first
        // string will not contain two consecutive '*'
        if (*first == '*' && firstLen != 1 && secondLen == 0)
            return false;

        // If the first string contains '?', or current characters
        // of both strings match
        if (*first == '?' || *first == *second)
            return Match(first + 1, second + 1, firstLen - 1, secondLen - 1);

        // If there is *, then there are two possibilities
        // a) We consider current character of second string
        // b) We ignore current character of second string.
        if (*first == '*')
            return Match(first + 1, second, firstLen - 1, secondLen) || Match(first, second + 1, firstLen, secondLen - 1);
        return false;
    }

    inline bool Match(const Cpl::String& first, const Cpl::String& second)
    {
        return Match(first.c_str(), second.c_str(), first.size(), second.size());
    }
}

namespace Cpl
{
    CPL_INLINE String FolderSeparator()
    {
#ifdef WIN32
        return String("\\");
#elif defined(__unix__)
        return String("/");
#else
        std::cerr << "FolderSeparator: Is not implemented yet!\n";
        return return String("");
#endif
    }

    namespace {
        CPL_INLINE Cpl::String MakePathImpl(const Cpl::String& a, const Cpl::String& b)
        {
            if (a.empty())
                return b;
            auto sep = Cpl::FolderSeparator();
            return a + (a[a.size() - 1] == sep[0] ? "" : sep) + b;
        }
    }

    template<typename T, typename S>
    CPL_INLINE String MakePath(const T& a, const S& b)
    {
        return MakePathImpl(a, b);
    }

    template<typename T, typename S, typename... Ts>
    CPL_INLINE String MakePath(const T& a, const S& b, const Ts&... args)
    {
        return MakePath(MakePath(a, b), args...);
    }

    CPL_INLINE bool FileExists(const String& path)
    {
#ifdef _MSC_VER
        DWORD fileAttribute = ::GetFileAttributes(path.c_str());
        return (fileAttribute != INVALID_FILE_ATTRIBUTES);
#else
        return (::access(path.c_str(), F_OK) != -1);
#endif	//_MSC_VER
    }

    CPL_INLINE bool FileIsReadable(const String& path)
    {
#ifdef _MSC_VER
        return (::_access(path.c_str(), 4) != -1);
#else
        return (::access(path.c_str(), R_OK) != -1);
#endif	//_MSC_VER
    }

    // only $USER is currently supported
    CPL_INLINE String SubstituteEnv(const String& path)
    {
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4996)
#endif
        static char* userPtr = getenv("USER"); // see https://community.hpe.com/t5/HP-UX-General/Impact-on-performance-by-excessive-getenv/m-p/4600627/highlight/true#M144885
#ifdef _MSC_VER
# pragma warning(pop)
#endif
        if (userPtr)
        {
            String user = userPtr;
            if (!user.empty())
            {
                String newPath = path;
                Cpl::ReplaceAllInplace(newPath, "$USER", user);
                return newPath;
            }
        }
        return path;
    }

    CPL_INLINE bool DirectoryExists(const String& path)
    {
#ifdef _MSC_VER
        DWORD fileAttribute = GetFileAttributes(path.c_str());
        return ((fileAttribute != INVALID_FILE_ATTRIBUTES) &&
            (fileAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
        DIR* dir = opendir(path.c_str());
        if (dir != NULL)
        {
            ::closedir(dir);
            return true;
        }
        else
            return false;
#endif
    }

    CPL_INLINE bool CreatePath(const String& path)
    {
#if defined(_MSC_VER)
        return fs::create_directories(fs::path(path));
#else
        return std::system((String("mkdir -p ") + path).c_str()) == 0;
#endif
    }

    inline StringList GetFileList(const String& directory, const String& filter, bool files, bool directories)
    {
        std::list<String> names;
#ifdef _MSC_VER
        ::WIN32_FIND_DATA fd;
        ::HANDLE hFind = ::FindFirstFile(MakePath(directory, filter).c_str(), &fd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                String name = fd.cFileName;
                if (files && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    names.push_back(fd.cFileName);
                if (directories && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && name != "." && name != "..")
                    names.push_back(name);
            } while (::FindNextFile(hFind, &fd));
            ::FindClose(hFind);
        }
#else
        DIR* dir = ::opendir(directory.c_str());
        if (dir != NULL)
        {
            struct dirent* drnt;
            while ((drnt = ::readdir(dir)) != NULL)
            {
                String name = drnt->d_name;
                if (name == "." || name == "..")
                    continue;
                if (!filter.empty() && !Match(filter, name))
                    continue;
                if (files && drnt->d_type != DT_DIR)
                    names.push_back(String(drnt->d_name));
                if (directories && drnt->d_type == DT_DIR)
                    names.push_back(String(drnt->d_name));
            }
            ::closedir(dir);
        }
        else
            std::cout << "There is an error during (" << errno << ") opening '" << directory << "' !" << std::endl;
#endif
        return names;
    }

    CPL_INLINE Strings ToSortedVector(const StringList& list)
    {
        Strings vector;
        for (StringList::const_iterator it = list.begin(); it != list.end(); ++it)
            vector.push_back(*it);
        std::sort(vector.begin(), vector.end());
        return vector;
    }

    CPL_INLINE String GetNameByPath(const String& path_)
    {
#ifdef _MSC_VER
        fs::path path(path_);
        return path.filename().string();
#elif defined(__unix__)
        size_t pos = path_.find_last_of("/");
        if (pos == std::string::npos)
            return path_;
        else
            return path_.substr(pos + 1);
#else
        std::cerr << "GetNameByPath: Is not implemented yet!\n";
        return "";
#endif
    }

    CPL_INLINE String DirectoryByPath(const String& path)
    {
        size_t pos = path.find_last_of(FolderSeparator());
        if (pos == std::string::npos)
            return path.find(".") == 0 ? String("") : path;
        else
            return path.substr(0, pos);
    }

    CPL_INLINE String ExtensionByPath(const String& path)
    {
        size_t pos = path.find_last_of(".");
        if (pos == std::string::npos)
            return String();
        else
            return path.substr(pos + 1);
    }

    // path is absolute or relative to basePath, path and basePath must exist
    // returns empty string if path is empty
    CPL_INLINE String GetAbsolutePath(const String& path, const String& basePath)
    {
        if (path.empty())
            return path;
        if (fs::path(path).is_absolute())
            return path;
        fs::path p;
        if (fs::is_directory(basePath))
            p = fs::path(basePath) / path;
        else
            p = fs::path(basePath).parent_path() / path;
        p = fs::canonical(p);
        return fs::absolute(p).string();
    }

    CPL_INLINE bool CopyDirectory(const String& src, const String& dst)
    {
#if defined(_MSC_VER)
        try
        {
            typedef fs::copy_options opt;
            fs::copy(src, dst, opt::overwrite_existing | opt::recursive);
        }
        catch (...)
        {
            return false;
        }
        return true;
#elif defined (__linux__)
        String com = String("cp -R ") + src + " " + dst;
        return std::system(com.c_str()) == 0;
#else
        return false;
#endif
    }

    CPL_INLINE bool DeleteDirectory(const String& dir)
    {
#if defined(_MSC_VER)
        return false;
#elif defined (__linux__)
        String com = String("rm -rf ") + dir;
        return std::system(com.c_str()) == 0;
#else
        return false;
#endif
    }
}
