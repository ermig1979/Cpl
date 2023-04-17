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

#include "Cpl/Defs.h"
#include "Cpl/Log.h"
#include <algorithm>
#include <bits/types/FILE.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#if (defined(__GNUC__) && (__GNUC__ < 7) || ( defined(__clang__) &&  __clang_major__ < 5))
//No filesystem
#elif defined(__GNUC__) && (__GNUC__ <= 10 || __cplusplus < 201703L)
#define CPL_FILE_USE_FILESYSTEM 1
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif defined(__GNUC__) && (__GNUC__ > 10 || __cplusplus >= 201703L) ||  defined(__clang__)
#define CPL_FILE_USE_FILESYSTEM 2
#include <filesystem>
namespace fs = std::filesystem;
#elif defined(_MSC_VER) && _MSC_VER <= 1900
#define CPL_FILE_USE_FILESYSTEM 1
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

    static const std::vector<char> forbiddenSymbols = [](){
        std::vector<char> temp = {'<', '>', ':', '"', '/', '\\', '|', '?', '*'};
        for (char ch= 0; ch < 32; ch++){
            temp.push_back(ch);
        }
        return temp;
    }();

    CPL_INLINE String LastDashFix(const String& path){
        for (auto iter = path.rbegin(); iter != path.rend(); iter++){
            auto forbidden_iter = std::find(std::begin(forbiddenSymbols), std::end(forbiddenSymbols), *iter);
            if (forbidden_iter == forbiddenSymbols.end() && *iter != ' '){
                return path.substr(0, std::distance(iter, path.rend()));
            }
        }
        return path;
    }

    CPL_INLINE size_t Compiler(){
        #if (defined(__GNUC__) && (__GNUC__ < 7) || ( defined(__clang__) &&  __clang_major__ < 5))
            return 0;
        #elif defined(__GNUC__) && (__GNUC__ <= 10 || __cplusplus < 201703L)
            return 1;
        #elif defined(__GNUC__) && (__GNUC__ > 10 || __cplusplus >= 201703L) ||  defined(__clang__)
            return 2;
        #elif defined(_MSC_VER) && _MSC_VER <= 1900
            return 3;
        #else
            return 4;
        #endif
    }

    CPL_INLINE size_t UsedFs(){
#if CPL_FILE_USE_FILESYSTEM == 2
        return 2;
#elif CPL_FILE_USE_FILESYSTEM == 1
        return 3;
#elif defined(_MSC_VER)
        return 1;
#else
        return 0;
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
        const String path = LastDashFix(path_);

#if CPL_FILE_USE_FILESYSTEM == 2
        try {
            fs::directory_entry dir_entry(path);
            return dir_entry.is_directory() && dir_entry.exists();
        } catch (...) {

        }
        return false;
#elif CPL_FILE_USE_FILESYSTEM == 1
        try {
            return fs::is_directory(path);
        }
        catch(...){
            return false;
        }
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
        if (DirectoryExists(path))
            return true;

#ifdef CPL_FILE_USE_FILESYSTEM
        return fs::create_directories(fs::path(path));
#elif _MSC_VER
        return fs::create_directories(fs::path(path));
#else
        return (mkdir(path.c_str(), 0777) == 0);
#endif
    }



    inline StringList GetFileList(const String& directory, const String& filter, bool files, bool directories, bool recursive) {
        std::list<String> names;
#if CPL_FILE_USE_FILESYSTEM == 2
        fs::directory_entry dir_entry(directory);
        if (!dir_entry.exists()) {
            return names;
        }
        
        if (!recursive) {
            for (auto const& entrance : fs::directory_iterator{directory}) {
                auto regular = entrance.path().filename().string();
                if (entrance.is_regular_file() && files){
                    if (filter.empty() || Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
                else if (entrance.is_directory() && directories){
                    if (filter.empty() ||Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
            }
        }
        else {
            for (auto const& entrance : fs::recursive_directory_iterator{directory}) {
                auto regular = entrance.path().filename().string();
                if (entrance.is_regular_file() && files){
                    if (filter.empty() || Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
                else if (entrance.is_directory() && directories){
                    if (filter.empty() ||Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
            }
        }
#elif CPL_FILE_USE_FILESYSTEM == 1

        if (!Cpl::DirectoryExists(directory)) {
            return names;
        }

        for (auto const& entrance : fs::directory_iterator{directory}) {
            if (fs::is_regular_file(entrance) && files){
                names.push_back(entrance.path().string());
            }
            else if (fs::is_directory(entrance) && directories){
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
        const String path = LastDashFix(path_);

#ifdef CPL_FILE_USE_FILESYSTEM
        fs::path fspath(path);
        return fspath.filename().string();
#elif  _MSC_VER
        fs::path path(path);
        return path.filename().string();
#else
        size_t pos = path.find_last_of("/");
        if (pos == String::npos)
            return path;
        else
            return path.substr(pos + 1);
#endif
    }

    CPL_INLINE String DirectoryByPath(const String& path_)
    {
        const String path = LastDashFix(path_);

        size_t pos = path.find_last_of(FolderSeparator());
        if (pos == String::npos)
            return path.find(".") == 0 ? String("") : path;
        else
            return path.substr(0, pos);
    }

    CPL_INLINE String FileNameByPath(const String & path)
    {
#if defined CPL_FILE_USE_FILESYSTEM
        return fs::path(path).filename().string();
#else
        return path.substr(path.find_last_of("/\\") + 1);
#endif
    }


    CPL_INLINE String ExtensionByPath(const String& path)
    {
        size_t pos = path.find_last_of(".");
        if (pos == String::npos)
            return String();
        else
            return path.substr(pos + 1);
    }

    CPL_INLINE String RemoveExtension(const String& path)
    {
        size_t last_sep = (size_t) path.find_last_of(".");
        if (last_sep == String::npos || last_sep == path.size() - 1) 
            return path;

        return path.substr(0, last_sep);
    }

    CPL_INLINE String ChangeExtension(const String& path, const String& ext) 
    {
        //TODO:add asserts
        if (path == "." || path.size() == 0)
            return path;

        size_t last_sep = (size_t) path.find_last_of(".");
        size_t last_sep_ext = (size_t) ext.find_last_of(".");

        //If path end with dot without extension, this dot count as file name
        if (last_sep + 1 == path.size())
            ++last_sep;

        if (last_sep_ext == String::npos)
            last_sep_ext = 0;
        else 
            ++last_sep_ext;
        
        return path.substr(0, last_sep) + "." + ext.substr(last_sep_ext, String::npos);
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

    CPL_INLINE bool Copy(const String& src, const String& dst, bool recursive = true)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            fs::copy(src, dst, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
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

    CPL_INLINE bool DeleteFile(const String& filename)
    {
        std::error_code code;
        if (!FileExists(filename)){
            return false;
        }
        bool ret = fs::remove(filename, code);
        return ret;
    }

    CPL_INLINE size_t DeleteDirectory(const String& dir)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            std::error_code code;
            size_t removeCount = (size_t) fs::remove_all(dir, code);
            return removeCount;
        }
        catch (...) {
        }
        return 0;
#elif defined(_MSC_VER)
        return false;
#elif defined (__linux__)
        String com = String("rm -rf ") + dir;
        return std::system(com.c_str()) == 0;
#else
        return false;
#endif
    }

    //TODO:
    //CPL_INLINE bool EqualPath(const String& first, const String& second);

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

    CPL_INLINE long int FileSize (const String & path) {
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

        return -1;
    }


    CPL_INLINE bool DirectorySize (const String & path, size_t& size) {
        size_t tsize = 0;
        try {
            if (!DirectoryExists(path))
                return false;

            fs::recursive_directory_iterator end_itr;

            for (fs::recursive_directory_iterator iter(path); iter != end_itr; ++iter ) {
                if (!fs::is_directory(iter->status()) )
                    tsize += fs::file_size(iter->path());
            }
            size = tsize;
            return true;
        }
        catch(...) {
        }
        return false;
    }

    struct FileData {
        enum class Type {
            Binary,
            BinaryToNullTerminatedText
        };

        FileData(Type type = Type::Binary)
        : _type(type)
        , _size(0)
        {};

        unsigned const char* data() const {
            if (_holder)
                return _holder.get();
            else
                return nullptr;
        }

        size_t size() const { return _size; }
        bool empty() const { return !_holder.operator bool(); }

        FileData& operator=(FileData&& other) {
            this->_size = other._size;
            this->_type = other._type;
            this->_holder = std::move(other._holder);

            other._size = 0;
            other._type = Type::Binary;

            return *this;
        }

    private:
        FileData(size_t size, Type type)
            : _type(type)
            , _size(size)
        {
            if (size)
                recreateHolder();
        }

        bool recreateHolder() {
            try {
                if (_type == Type::BinaryToNullTerminatedText) {
                    _holder = std::make_unique<unsigned char[]>(_size + 1);
                    _holder.get()[_size] = 0;
                }
                else
                    _holder = std::make_unique<unsigned char[]>(_size);

                return true;
            }
            catch (...) {
            }
            return false;
        }

        Type _type;
        size_t _size;
        std::unique_ptr<unsigned char[]> _holder;

        friend int ReadFile(const String & path, FileData& out, size_t byteBudget);
    };

    CPL_INLINE int WriteToFile(const String & filePath, const char* data, size_t size, bool recreate = true) {
        try {
            std::ofstream fs;
            auto fl = std::ios::out | std::ios::binary;
            if (!recreate)
                fl |= std::ios::app;

            fs.open(filePath, fl);
            if (!fs.fail()) {
                fs.write(data, size);
                fs.close();
                return -1;
            }
        }
        catch (...) {
        }

        return 0;

    }

    CPL_INLINE int ReadFile(const String & path, FileData& out, size_t byteBudget = 1 * 1024 * 1024 * 1024 /* 1 gb */ ) {
        try {
            std::ifstream ifs;
            ifs.open(path, std::ios::in | std::ios::binary);
            bool partial = false;
            if (!ifs.fail()) {
                std::ifstream::pos_type pos = 0;

                if (!ifs.seekg(0, std::ios::end)) {
                    return 1;
                }

                pos = ifs.tellg();

                if (pos > byteBudget) {
                    pos = byteBudget;
                    partial = true;
                }

                FileData readed(pos, out._type);

                if (pos && ifs.seekg(0, std::ios::beg)) {
                    ifs.read((char*) readed._holder.get(), readed.size());
                    if (ifs.fail()) {
                        return 2;
                    }
                }

                out = std::move(readed);
                return partial ? -2 : -1;
            }
        }
        catch (...) {
        }

        return 0;
    }
}
