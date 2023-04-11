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
#include <fstream>

#ifdef _WIN32
#include "windows.h"
#endif

#if (defined(__GNUC__) && (__GNUC__ < 7) || ( defined(__clang__) &&  __clang_major__ < 5))
//No filesystem

#elif defined(__GNUC__) && (__GNUC__ <= 10 || __cplusplus < 201703L)
#define CPL_FILE_USE_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif defined(__GNUC__) && (__GNUC__ > 10 || __cplusplus >= 201703L) ||  defined(__clang__)
#define CPL_FILE_USE_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#elif defined(_MSC_VER) && _MSC_VER <= 1900
#define CPL_FILE_USE_FILESYSTEM
#include <filesystem>
namespace fs = std::tr2::sys;
#else
#error Unknow system
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
        return String("");
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

    CPL_INLINE bool FileExists(const String& filePath)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        fs::path fspath(filePath);
        return fs::exists(filePath) && (fs::is_regular_file(filePath) || fs::is_symlink(filePath));
#elif _MSC_VER
        DWORD fileAttribute = ::GetFileAttributes(path.c_str());
        return (fileAttribute != INVALID_FILE_ATTRIBUTES);
#elif (__unix__)
        return (::access(path.c_str(), F_OK) != -1);
#else
        std::ifstream ifs;
        ifs.open(path, std::ios::in | std::ios::binary);
        return (!ifs.fail());
#endif
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

    CPL_INLINE bool DirectoryExists(const String& path_)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            fs::directory_entry dir_entry(path_);
            return dir_entry.is_directory() && dir_entry.exists();
        } catch (...) {

        }
        return false;

#elif defined(_MSC_VER)
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
#ifdef CPL_FILE_USE_FILESYSTEM
        return fs::create_directories(fs::path(path));
#elif _MSC_VER
        return fs::create_directories(fs::path(path));
#else
        return std::system((String("mkdir -p ") + path).c_str()) == 0;
#endif
    }

    inline StringList GetFileList(const String& directory, const String& filter, bool files, bool directories) {
        std::list<String> names;
#ifdef CPL_FILE_USE_FILESYSTEM
        fs::directory_entry dir_entry(directory);
        if (!dir_entry.exists()) {
            return names;
        }

        for (auto const& entrance : std::filesystem::directory_iterator{directory}) {
            if (entrance.is_regular_file() && files){
                names.push_back(entrance.path().string());
            }
            else if (entrance.is_directory() && directories){
                names.push_back(entrance.path().string());
            }
        }

#elif _MSC_VER
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
#ifdef CPL_FILE_USE_FILESYSTEM
        fs::path path(path_);
        return path.filename().string();
#elif  _MSC_VER
        fs::path path(path_);
        return path.filename().string();
#else
        size_t pos = path_.find_last_of("/");
        if (pos == std::string::npos)
            return path_;
        else
            return path_.substr(pos + 1);
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

    CPL_INLINE bool CopyDirectory(const String& src, const String& dst, bool recursive = true)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            std::filesystem::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            return true;
        }
        catch (...) {
            return false;
        }
#elif defined(_MSC_VER)
        try
        {
            typedef fs::copy_options opt;
            fs::copy(src, dst, opt::overwrite_existing | opt::recursive);
        }
        catch (...){
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
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            std::filesystem::remove_all(dir);
            return true;
        }
        catch (...) {
            return false;
        }
#elif defined(_MSC_VER)
        return false;
#elif defined (__linux__)
        String com = String("rm -rf ") + dir;
        return std::system(com.c_str()) == 0;
#else
        return false;
#endif
    }

    CPL_INLINE String GetExecutableLocation()
    {
#if defined(_WIN32)
        std::string retval;
        char buf[MAX_PATH];
        DWORD nSize = ::GetModuleFileName(NULL, buf, sizeof(buf));
        if (nSize > 0) {
            retval = std::string(buf);
        }
#elif defined(linux) || defined (__linux) || defined (__linux__)
        char buf[512];
        size_t len = readlink("/proc/self/exe", buf, sizeof(buf));
        std::string retval(buf, len);
#else
#error Not support system
#endif
        retval = retval.substr(0, retval.find_last_of("/\\"));
        return retval;
    }

    CPL_INLINE String FileNameWithoutExt(const String & fileName) {
        return fileName.substr(0, fileName.find_last_of('.'));
    }

    CPL_INLINE String FileNameByPath(const String & path)
    {
#if defined CPL_FILE_USE_FILESYSTEM
        return fs::path(path).filename().string();
#else
        return path.substr(path.find_last_of("/\\") + 1);
#endif
    }

    CPL_INLINE size_t FileSize (const String & path) {
        if (FileExists(path)) {
            std::ifstream ifs;
            ifs.open(path, std::ios::in | std::ios::binary);
            if (!ifs.fail()) {
                std::ifstream::pos_type size = 0;
                if (ifs.seekg(0, std::ios::end))
                    size = ifs.tellg();

                return size;
            }
        }

        return 0;
    }

    struct FileReadedData {
        enum class Type {
            Binary,
            Text
        };

        FileReadedData(Type type = Type::Binary) : _size(0) {};

        const char* data() const {
            if (_holder)
                return _holder.get();
            else
                return nullptr;
        }

        size_t size() const { return _size; }

        FileReadedData& operator=(FileReadedData&& other) {
            this->_size = other._size;
            this->_type = other._type;
            this->_holder = std::move(other._holder);

            other._size = 0;
            other._type = Type::Binary;

            return *this;
        }

    private:
        FileReadedData(size_t size, Type type)
            : _type(type)
            , _size(size)
            , _holder()
        {
            if (size)
                recreateHolder();
        }

        bool recreateHolder() {
            try {
                if (_type == Type::Text) {
                    _holder = std::make_unique<char[]>(_size + 1);
                    _holder.get()[_size] = 0;
                }
                else
                    _holder = std::make_unique<char[]>(_size);

                return true;
            }
            catch (...) {
            }
            return false;
        }

        Type _type;
        size_t _size;
        std::unique_ptr<char[]> _holder;

        friend int ReadFile(const String & path, FileReadedData& out, size_t byteBudget);
    };

    CPL_INLINE int ReadFile(const String & path, FileReadedData& out, size_t byteBudget = 1 * 1024 * 1024 * 1024 /* 1 gb */ ) {
        try {
            std::ifstream ifs;
            ifs.open(path, std::ios::in | std::ios::binary);
            if (!ifs.fail()) {
                std::ifstream::pos_type pos = 0;

                if (!ifs.seekg(0, std::ios::end)) {
                    return 2;
                }

                pos = ifs.tellg();

                if (pos > byteBudget) {
                    return 3;
                }

                FileReadedData readed(pos, out._type);

                if (pos && ifs.seekg(0, std::ios::beg)) {
                    ifs.read(readed._holder.get(), readed.size());
                    if (ifs.fail()) {
                        return false;
                    }
                }

                out = std::move(readed);
                return -1;
            }
        }
        catch (...) {
        }

        return 0;
    }
}
