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
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#if (defined(__GNUC__) && (__GNUC__ < 8) || ( defined(__clang__) &&  __clang_major__ < 5) || __cplusplus < 201402L)
//No filesystem
#elif (((defined(__GNUC__) && (__GNUC__ >= 8 )) || defined(_MSC_VER)) &&  __cplusplus < 201703L) 
#define CPL_FILE_USE_FILESYSTEM 1
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#elif (((defined(__GNUC__) && (__GNUC__ > 10) ) || defined(__clang__)) && __cplusplus >= 201703L || (defined(_MSC_VER)) && __cplusplus >= 201402L)
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
#ifdef _WIN32
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

    CPL_INLINE String DirectoryPathLastDashFix(const String& path){
        auto iter = path.rbegin();
        while (*iter == FolderSeparator().at(0) || *iter == ' ')
            iter++;

        return path.substr(0, std::distance(iter, path.rend()));
    }

    CPL_INLINE size_t CompilerType(){
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

    CPL_INLINE size_t FilesystemType(){
#if CPL_FILE_USE_FILESYSTEM == 2
        return 2;
#elif CPL_FILE_USE_FILESYSTEM == 1
        return 3;
#elif _WIN32
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

        CPL_INLINE bool DirectoryIsDrive(const Cpl::String& path) {
#ifdef __linux__
            return false;
#elif _WIN32
            String substring = DirectoryPathLastDashFix(path);
            if (substring.size() == 2 && substring[1] == ':') {
                return true;
            }
            return false;
#endif
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

    //TODO: maybe rename, check "." folder on linux
    CPL_INLINE String DirectoryByPath(const String& path_) {
        const String path = DirectoryPathLastDashFix(path_);
        size_t pos = path.find_last_of(FolderSeparator());
        String substr = path.substr(0, pos);
        if (DirectoryIsDrive(substr))
            return substr + FolderSeparator();

        return substr;
    }

    CPL_INLINE String DirectoryUp(const String& path) {
        return DirectoryByPath(path);
    }

    CPL_INLINE String DirectoryDown(const String& format, const String& path) {
        size_t endPos = format.find(Cpl::FolderSeparator(), path.size() + Cpl::FolderSeparator().size());
        return format.substr(0, endPos);
    }

    CPL_INLINE bool FileExists(const String& filePath)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        fs::path fspath(filePath);
        return fs::exists(filePath) && (fs::is_regular_file(filePath) || fs::is_symlink(filePath));
#elif _WIN32
        DWORD fileAttribute = ::GetFileAttributes(filePath.c_str());
        return (fileAttribute != INVALID_FILE_ATTRIBUTES);
#elif __linux__
        struct stat buf;
        int exists = stat( filePath.c_str(), &buf );
        return exists == 0 && S_ISREG(buf.st_mode);
#else
        std::ifstream ifs;
        ifs.open(filePath, std::ios::in | std::ios::binary);
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
        const String path = DirectoryPathLastDashFix(path_);
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            return fs::is_directory(path) && fs::exists(path);
        }
        catch(...){
            return false;
        }
#elif _WIN32
        DWORD fileAttribute = GetFileAttributes(path.c_str());
        return ((fileAttribute != INVALID_FILE_ATTRIBUTES) &&
                (fileAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0);
#elif __linux__
        struct stat buf;
        int exists = stat( path_.c_str(), &buf );
        return exists == 0 && S_ISDIR(buf.st_mode);
#else
#error Not supported system
#endif
    }

    CPL_INLINE String DirectoryByPath(const String& path_);

    CPL_INLINE bool CreatePath(const String& path)
    {
        if (DirectoryExists(path))
            return true;
        if (DirectoryIsDrive(path))
            return false;

#ifdef CPL_FILE_USE_FILESYSTEM
        return fs::create_directories(fs::path(path));
#elif _WIN32
        if (CreateDirectory(path.c_str(), NULL)) {
            return true;
        } 
        else 
        {
            if (GetLastError() == ERROR_PATH_NOT_FOUND) {
                String parent = Cpl::DirectoryByPath(path);
                
                if (parent == path) {
                    return false;
                }

                if (CreatePath(parent)) {
                    if (CreatePath(path))
                        return true;
                }
            }
        }

        return false;
#elif __linux__

        if (mkdir(path.c_str(), 0777) == 0) {
            return true;
        }
        const String startPath = path;
        String parent = Cpl::DirectoryUp(startPath);

        do {
            if (!Cpl::DirectoryExists(parent)) {
                String newParent = Cpl::DirectoryUp(parent);
                if (newParent == parent) {
                    break;
                }
                parent.swap(newParent);
            }
            else {
                break;
            }
        }
        while(parent != path);

        //parent folder now is last exist in path
        String dir = parent;
        do {
            dir = Cpl::DirectoryDown(startPath, dir);
            if (mkdir(dir.c_str(), 0777) != 0)
                return false;
        }
        while(dir != path);

        return DirectoryExists(path);
#else
#error Not supported system
#endif
    }

    inline StringList GetFileList(const String& directory, const String& filter, bool files, bool directories, bool recursive) {
        std::list<String> names;
#if CPL_FILE_USE_FILESYSTEM
        if (!Cpl::DirectoryExists(directory)) {
            return names;
        }

        fs::directory_entry dir_entry(directory);
        if (!fs::exists(dir_entry)) {
            return names;
        }

        if (!recursive) {
            for (auto const& entrance : fs::directory_iterator{directory}) {
                auto regular = entrance.path().filename().string();
                if (fs::is_regular_file(entrance) && files){
                    if (filter.empty() || Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
                else if (fs::is_directory(entrance) && directories){
                    if (filter.empty() ||Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
            }
        }
        else {
            for (auto const& entrance : fs::recursive_directory_iterator{directory}) {
                auto regular = entrance.path().filename().string();
                if (fs::is_regular_file(entrance) && files){
                    if (filter.empty() || Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
                else if (fs::is_directory(entrance) && directories){
                    if (filter.empty() ||Match(filter.c_str(), regular.c_str(), filter.size(), regular.size())){
                        names.push_back(entrance.path().string());
                    }
                }
            }
        }

#elif _WIN32
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
#elif __linux__
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
                if (files && drnt->d_type == DT_REG)
                    names.push_back(Cpl::MakePath(directory, String(drnt->d_name)));
                if (drnt->d_type == DT_DIR) {
                    if (directories)
                        names.push_back(Cpl::MakePath(directory, String(drnt->d_name)));
                    if (recursive) {
                        StringList list = GetFileList(Cpl::MakePath(directory, String(drnt->d_name)), filter, files, directories, recursive);
                        names.insert(names.end(), list.begin(), list.end());
                    }
                }
            }
            ::closedir(dir);
        }
        else
            std::cout << "There is an error during (" << errno << ") opening '" << directory << "' !" << std::endl;
#else
#error Not supported system
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
        const String path = DirectoryPathLastDashFix(path_);

#ifdef CPL_FILE_USE_FILESYSTEM
        fs::path fspath(path);
        return fspath.filename().string();
#elif  _WIN32
        fs::path path(path);
        return path.filename().string();
#elif __linux__
        size_t pos = path.find_last_of("/");
        if (pos == String::npos)
            return path;
        else
            return path.substr(pos + 1);
#else
#error Not supported system
#endif

    }

    CPL_INLINE String FileNameByPath(const String & path)
    {
#if defined CPL_FILE_USE_FILESYSTEM
        return fs::path(path).filename().string();
#else
        return path.substr(path.find_last_of(Cpl::FolderSeparator()) + 1);
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

    CPL_INLINE String GetAbsolutePath(const String& path)
    {
#ifdef _WIN32
        std::array<char, MAX_PATH> buffer;
        const char* end = _fullpath(buffer.data(), path.c_str(), MAX_PATH);
        return String(buffer.data());
#elif __linux__
        char resolved_path[PATH_MAX];
        realpath(path.c_str(), resolved_path);
        return String(resolved_path);
#else
#error Not supported system
#endif
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
#elif _WIN32
        try
        {
            typedef fs::copy_options opt;
            fs::copy(src, dst, opt::overwrite_existing | opt::recursive);
        }
        catch (...){
            return false;
        }
        return true;
#elif __linux__
        String com = String("cp -R ") + src + " " + dst;
        return std::system(com.c_str()) == 0;
#else
#error Not supported system
#endif
    }

    CPL_INLINE bool DeleteFile(const String& filename)
    {
        if (!FileExists(filename)) {
            return false;
        }
#ifdef CPL_FILE_USE_FILESYSTEM
        std::error_code code;
        bool ret = fs::remove(filename, code);
        return ret;
#elif _WIN32 
        return ::DeleteFile(filename.c_str());
#elif __linux__
        return unlink(filename.c_str()) == 0;
#else
#error Not supported system
#endif

    }

    CPL_INLINE bool DeleteDirectory(const String& dir)
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
#elif _WIN32
        const String undashed = DirectoryPathLastDashFix(dir);
        String dashed = MakePath(undashed, FolderSeparator());

        WIN32_FIND_DATAA data;
        HANDLE handle = NULL;

        handle = FindFirstFileA(Cpl::MakePath(DirectoryPathLastDashFix(dashed), Cpl::String("*")).c_str(), &data);
        if (handle == INVALID_HANDLE_VALUE)
            return false;

        do {
            if ((strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0))
            {
                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (!Cpl::DeleteDirectory(Cpl::MakePath(dir, data.cFileName)))
                        return false;
                }
                else {
                    if (!Cpl::DeleteFile(Cpl::MakePath(dir, data.cFileName)))
                        return false;
                }
            }

        } while (FindNextFile(handle, &data));

        if (GetLastError() != ERROR_NO_MORE_FILES)
            return false;
        FindClose(handle);

        return RemoveDirectory(dir.c_str());
#elif __linux__
        String com = String("rm -rf ") + dir;
        return std::system(com.c_str()) == 0;
#else
#error Not supported system
#endif
    }

    //TODO:
    //CPL_INLINE bool EqualPath(const String& first, const String& second);

    CPL_INLINE String GetExecutableLocation()
    {
#if _WIN32
        std::string retval;
        char buf[MAX_PATH];
        DWORD nSize = ::GetModuleFileName(NULL, buf, sizeof(buf));
        if (nSize > 0) {
            retval = std::string(buf);
        }
#elif __linux__
        char buf[512];
        size_t len = readlink("/proc/self/exe", buf, sizeof(buf));
        std::string retval(buf, len);
#else
#error Not supported system
#endif
        retval = retval.substr(0, retval.find_last_of("/\\"));
        return retval;
    }

    CPL_INLINE bool FileSize (const String & path, size_t& size) {
        if (FileExists(path)) {
            std::ifstream ifs;
            ifs.open(path, std::ios::in | std::ios::binary);
            if (!ifs.fail()) {
                std::ifstream::pos_type pos = 0;
                if (ifs.seekg(0, std::ios::end))
                    pos = ifs.tellg();

                size = pos;
                return true;
            }
        }

        return false;
    }


    CPL_INLINE bool DirectorySize (const String & path, size_t& size) {
        size_t tsize = 0;
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            if (!DirectoryExists(path))
                return false;

            fs::recursive_directory_iterator end_itr;

            for (fs::recursive_directory_iterator iter(path); iter != end_itr; ++iter ) {
                if (!fs::is_directory(iter->status()) )
                    tsize += fs::file_size(iter->path());
            }
        }
        catch(...) {
            return false;
        }
#elif _WIN32
        WIN32_FIND_DATAA data;
        HANDLE handle = NULL;

        handle = FindFirstFileA(Cpl::MakePath(DirectoryPathLastDashFix(path), Cpl::String("*")).c_str(), &data);
        if (handle == INVALID_HANDLE_VALUE)
            return false;

        do {
            // skip current directory and parent directory
            if ((strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0))
            {
                if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                {
                    size_t ttsize = 0;
                    if (!DirectorySize(Cpl::MakePath(path, data.cFileName), ttsize))
                        return false;
                    tsize += ttsize;
                }
                else
                    tsize += (size_t)(data.nFileSizeHigh * MAXDWORD + data.nFileSizeLow);
            }

        } while (FindNextFile(handle, &data));

        FindClose(handle);
#elif __linux__
        //https://stackoverflow.com/questions/1129499/how-to-get-the-size-of-a-dir-programatically-in-linux
        DIR *d = opendir( path.c_str() );
        if( d == NULL )
            return false;

        struct dirent *de;
        struct stat buf;

        for( de = readdir( d ); de != NULL; de = readdir( d ) ) {
            int exists = stat( Cpl::MakePath(path, String(de->d_name)).c_str(), &buf );

            if( exists < 0 ) {
                fprintf( stderr, "Cannot read file statistics for %s\n", de->d_name );
            } else {
                if (de->d_type == DT_REG)
                    tsize += buf.st_size;
            }
        }

#else
#error Not supported system
#endif
        size = tsize;
        return true;
    }

    struct FileData {
        enum class Type {
            Binary,
            BinaryToNullTerminatedText
        };


        struct Error {
            enum ReadFileError {
                NoError,
                PartitialRead,
                FailedToOpen,
                FailedToRead,
                FailedToGetInfo,
                CommonFail
            };

            Error(Error::ReadFileError code_) : code(code_) {}
            const ReadFileError code;

            operator bool() const {
                if (code == NoError || code == PartitialRead) {
                    return true;
                }
                return false;
            }
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
                    //_holder = std::make_unique<unsigned char[]>(_size + 1);
                    _holder = std::unique_ptr<unsigned char[]>(new unsigned char[_size + 1]);
                    _holder.get()[_size] = 0;
                }
                else
                    _holder = std::unique_ptr<unsigned char[]>(new unsigned char[_size]);

                return true;
            }
            catch (...) {
            }
            return false;
        }

        Type _type;
        size_t _size;
        std::unique_ptr<unsigned char[]> _holder;

        friend FileData::Error ReadFile(const String & path, FileData& out, size_t startPos, size_t maxSize);
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
    
    CPL_INLINE FileData::Error ReadFile(const String & path, FileData& out, size_t startPos = 0, size_t maxSize = 1 * 1024 * 1024 * 1024 /* 1 gb */) {
        try {
            std::ifstream ifs;
            ifs.open(path, std::ios::in | std::ios::binary);
            bool partial = false;
            if (!ifs.fail()) {

                if (startPos) {
                    if (!ifs.seekg(startPos)) {
                        return FileData::Error::FailedToGetInfo;
                    }
                }

                if (!ifs.seekg(0, std::ios::end)) {
                    return FileData::Error::FailedToGetInfo;
                }

                size_t end = ifs.tellg();
                size_t size = end - startPos;

                if ( size > maxSize) {
                    size = maxSize;
                    partial = true;
                }

                FileData readed(size, out._type);

                if (size && ifs.seekg(startPos)) {
                    ifs.read((char*) readed._holder.get(), readed.size());
                    if (ifs.fail()) {
                        return FileData::Error::FailedToRead;
                    }
                }

                out = std::move(readed);
                return partial ? FileData::Error::PartitialRead : FileData::Error::NoError;
            }
        }
        catch (...) {
        }

        return FileData::Error::CommonFail;
    }
}
