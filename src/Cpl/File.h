/*
* Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2026 Yermalayeu Ihar,
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

#include "Cpl/Defs.h"
#include "Cpl/Log.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <cstring>
#include <queue>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows.h"
#endif

#ifdef __linux__
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1900
/*! @ingroup cpl_file
* \def CPL_FILE_USE_FILESYSTEM
* \brief Filesystem backend used by the file helpers. 1 is experimental/TR2 filesystem, 2 is std::filesystem.
*        Undefined when the Win32 or POSIX APIs are used instead.
*/
#define CPL_FILE_USE_FILESYSTEM 1
#include <filesystem>
namespace fs = std::tr2::sys;
#elif (__cplusplus >= 201703L)
/*! @ingroup cpl_file
* \def CPL_FILE_USE_FILESYSTEM
* \brief Filesystem backend used by the file helpers. 1 is experimental/TR2 filesystem, 2 is std::filesystem.
*        Undefined when the Win32 or POSIX APIs are used instead.
*/
#define CPL_FILE_USE_FILESYSTEM 2
#include <filesystem>
namespace fs = std::filesystem;
#elif (__cplusplus == 201402L)
/*! @ingroup cpl_file
* \def CPL_FILE_USE_FILESYSTEM
* \brief Filesystem backend used by the file helpers. 1 is experimental/TR2 filesystem, 2 is std::filesystem.
*        Undefined when the Win32 or POSIX APIs are used instead.
*/
#define CPL_FILE_USE_FILESYSTEM 1
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
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
    /*! @ingroup cpl_file
    * \brief Returns the platform directory separator.
    * \return "\\" on Windows, "/" on Unix. An empty string on unsupported platforms.
    */
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

    /*! @ingroup cpl_file
    * \brief Characters that ChangeExtension strips from a new extension: '<', '>', ':', '"', '/', '\\', '|', '?', '*' and ASCII codes 0-31.
    */
    static const std::vector<char> forbiddenSymbols = [](){
        std::vector<char> temp = {'<', '>', ':', '"', '/', '\\', '|', '?', '*'};
        for (char ch= 0; ch < 32; ch++){
            temp.push_back(ch);
        }
        return temp;
    }();

    /*! @ingroup cpl_file
    * \brief Removes trailing directory separators and spaces from a path.
    * \param [in] path - File or directory path.
    * \return path without a trailing FolderSeparator() or spaces. Unchanged if path is not longer
    *         than the separator or does not end with a separator or space.
    */
    CPL_INLINE String DirectoryPathRemoveAllLastDash(const String& path){
        auto iter = path.rbegin();
        auto separator = FolderSeparator();

        if (path.size() <= separator.size()){
            return path;
        }

        if (path.back() != separator.back() && path.back() != ' '){
            return path;
        }

        std::advance(iter, separator.size());
        while (std::memcmp(separator.c_str(), iter.operator->(), separator.size()) == 0 || *iter == ' ')
            iter++;
        return path.substr(0, std::distance(iter, path.rend()));;
    }

    /*! @ingroup cpl_file
    * \brief Identifies the compiler family used to select a filesystem backend.
    * \return 0 for GCC < 7 or Clang < 5; 1 for GCC <= 10 or C++ below 17; 2 for newer GCC/Clang
    *         or C++17 and later; 3 for MSVC 2015 or older; 4 for other compilers.
    */
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

    /*! @ingroup cpl_file
    * \brief Identifies the filesystem API used by the file helpers.
    * \return 2 for std::filesystem, 3 for experimental/TR2 filesystem, 1 for the Win32 API,
    *         0 when neither filesystem nor Win32 is available.
    */
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

    namespace PathDetail{
        /*! @ingroup cpl_file
        * \brief Checks whether a path is a Windows drive root such as "C:" or "C:\\".
        * \param [in] path - Path to test. Trailing separators are ignored.
        * \return true on Windows when the stripped path is two characters and the second is ':'.
        *         Always false on Linux.
        */
        CPL_INLINE bool DirectoryIsDrive(const Cpl::String& path) {
#ifdef __linux__
            return false;
#elif _WIN32
            //TODO: replace by system call and check disk flag
            String substring = DirectoryPathRemoveAllLastDash(path);
            if (substring.size() == 2 && substring[1] == ':') {
                return true;
            }
            return false;
#endif
        }
    }

    /*! @ingroup cpl_file
    * \brief Joins two path components with FolderSeparator() when needed.
    * \tparam T - Type of the first component. Must be insertable into std::ostream.
    * \tparam S - Type of the second component. Must be insertable into std::ostream.
    * \param [in] a - Left path component.
    * \param [in] b - Right path component.
    * \return Concatenation of a and b. A separator is inserted when a is not empty and does not
    *         already end with FolderSeparator().
    */
    template<typename T, typename S>
    CPL_INLINE String MakePath(const T& a, const S& b)
    {
        std::stringstream stream;
        stream << a;

        if (!stream.str().empty() && stream.str().back() != Cpl::FolderSeparator().back()){
            stream << Cpl::FolderSeparator();
        }

        stream << b;
        return stream.str();
    }

    /*! @ingroup cpl_file
    * \brief Joins three or more path components from left to right.
    * \tparam T - Type of the first component. Must be insertable into std::ostream.
    * \tparam S - Type of the second component. Must be insertable into std::ostream.
    * \tparam Ts - Types of the remaining components.
    * \param [in] a - First path component.
    * \param [in] b - Second path component.
    * \param [in] args - Additional path components.
    * \return Result of joining a and b, then joining that result with args.
    */
    template<typename T, typename S, typename... Ts>
    CPL_INLINE String MakePath(const T& a, const S& b, const Ts&... args)
    {
        return MakePath(MakePath(a, b), args...);
    }

    /*! @ingroup cpl_file
    * \brief Returns the parent directory of a file or directory path.
    * \param [in] path_ - File or directory path. Trailing separators are ignored.
    * \return Directory that contains path_. For a Windows drive root such as "C:", returns the
    *         drive with a trailing separator.
    */
    //TODO: maybe rename, check "." folder on linux
    CPL_INLINE String DirectoryByPath(const String& path_) {
        const String path = DirectoryPathRemoveAllLastDash(path_);
        size_t pos = path.find_last_of(FolderSeparator());
        String substr = path.substr(0, pos);
        if (PathDetail::DirectoryIsDrive(path))
            return MakePath(path, FolderSeparator());

        return substr;
    }

    /*! @ingroup cpl_file
    * \brief Returns the parent directory of a path.
    * \param [in] path - File or directory path.
    * \return Same result as DirectoryByPath(path).
    */
    CPL_INLINE String DirectoryUp(const String& path) {
        return DirectoryByPath(path);
    }

    /*! @ingroup cpl_file
    * \brief Returns the next directory along format that is one level deeper than path.
    * \param [in] format - Full path that path is a prefix of, for example "/usr/local/bin".
    * \param [in] path - Current prefix of format, for example "/usr".
    * \return The next component of format after path. For format "/usr/local/bin" and path "/usr",
    *         the result is "/usr/local".
    */
    CPL_INLINE String DirectoryDown(const String& format, const String& path) {
        size_t endPos = format.find(Cpl::FolderSeparator(), path.size() + Cpl::FolderSeparator().size());
        return format.substr(0, endPos);
    }

    /*! @ingroup cpl_file
    * \brief Checks whether a regular file or symlink exists at filePath.
    * \param [in] filePath - Path to test.
    * \return true if a file exists at filePath. false if the path is a directory or does not exist.
    */
    CPL_INLINE bool FileExists(const String& filePath)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        fs::path fspath(filePath);
        return fs::exists(filePath) && (fs::is_regular_file(filePath) || fs::is_symlink(filePath));
#elif _WIN32
        DWORD fileAttribute = ::GetFileAttributesA(filePath.c_str());
        return (fileAttribute != INVALID_FILE_ATTRIBUTES) && !(fileAttribute & FILE_ATTRIBUTE_DIRECTORY);
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

    /*! @ingroup cpl_file
    * \brief Replaces the "$USER" token in a path with the value of the USER environment variable.
    * \param [in] path - Path that may contain "$USER".
    * \return path with every "$USER" replaced by getenv("USER"), or path unchanged if USER is
    *         missing or empty.
    * \note Only the $USER token is supported.
    */
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

    /*! @ingroup cpl_file
    * \brief Checks whether a directory exists at the given path.
    * \param [in] path_ - Path to test. Trailing separators are ignored.
    * \return true if a directory exists at path_. false if the path is a file or does not exist.
    */
    CPL_INLINE bool DirectoryExists(const String& path_)
    {
        const String path = DirectoryPathRemoveAllLastDash(path_);
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            return fs::is_directory(path) && fs::exists(path);
        }
        catch(...){
            return false;
        }
#elif _WIN32
        DWORD fileAttribute = GetFileAttributesA(path.c_str());
        return ((fileAttribute != INVALID_FILE_ATTRIBUTES) &&
                (fileAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0);
#elif __linux__
        struct stat buf;
        int exists = stat( path.c_str(), &buf );
        return exists == 0 && S_ISDIR(buf.st_mode);
#else
#error Not supported system
#endif
    }

    CPL_INLINE String DirectoryByPath(const String& path_);

    /*! @ingroup cpl_file
    * \brief Creates a directory path, including missing parent directories.
    * \param [in] path - Directory path to create.
    * \return true if path already exists as a directory or was created. false for a Windows drive
    *         root that does not exist, or if creation fails.
    */
    CPL_INLINE bool CreatePath(const String& path)
    {
        if (DirectoryExists(path))
            return true;
        if (PathDetail::DirectoryIsDrive(path))
            return false;

#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            return fs::create_directories(fs::path(path));
        }
        catch (...){

        }
        return false;
#elif defined(_WIN32) || defined (__linux__)
        auto createDirFunctor = [](const String& path) -> bool {
#ifdef _WIN32
            return CreateDirectoryA(path.c_str(), NULL);
#elif __linux__
            return mkdir(path.c_str(), 0777) == 0;
#endif
        };

        if (createDirFunctor(path)) {
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
            if (!createDirFunctor(dir))
                return false;
        }
        while(dir != path);

        return DirectoryExists(path);
#else
#error Not supported system
#endif
    }

    /*! @ingroup cpl_file
    * \brief Lists files and/or directories in a folder that match a wildcard filter.
    * \param [in] directory - Directory to scan.
    * \param [in] filter - Wildcard mask applied to the entry name, for example "*" or "abc*".
    *                      '?' matches one character, '*' matches any sequence. An empty filter
    *                      matches every name.
    * \param [in] files - If true, include regular files.
    * \param [in] directories - If true, include directories. "." and ".." are skipped.
    * \param [in] recursive - If true, scan subdirectories. false by default.
    * \return Paths of matching entries. Empty if directory does not exist.
    */
    inline StringList GetFileList(const String& directory, String filter, bool files, bool directories, bool recursive = false) {
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
        ::WIN32_FIND_DATAA fd;
        std::queue<String> queue;

        if (filter.empty())
            filter = "*";

        queue.push(MakePath(directory, filter));

        while (!queue.empty()){
            auto dir = std::move(queue.front());
            queue.pop();

            ::HANDLE hFind = ::FindFirstFileA(dir.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    String name = fd.cFileName;
                    if (files && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                        names.push_back(MakePath(DirectoryPathRemoveAllLastDash(directory),fd.cFileName));
                    if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && name != "." && name != "..") {
                        String curDir = MakePath(DirectoryPathRemoveAllLastDash(directory), name);
                        if (directories)
                            names.push_back(curDir);
                        if (recursive)
                            queue.push(MakePath(curDir, filter));
                    }
                } while (::FindNextFileA(hFind, &fd));
                ::FindClose(hFind);
            }
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

    /*! @ingroup cpl_file
    * \brief Copies a list of strings into a sorted vector.
    * \param [in] list - Source list, typically the result of GetFileList.
    * \return Strings vector with the same elements, sorted in ascending order.
    */
    CPL_INLINE Strings ToSortedVector(const StringList& list)
    {
        Strings vector;
        for (StringList::const_iterator it = list.begin(); it != list.end(); ++it)
            vector.push_back(*it);
        std::sort(vector.begin(), vector.end());
        return vector;
    }

    /*! @ingroup cpl_file
    * \brief Returns the last component of a path, including the extension.
    * \param [in] path_ - File or directory path. Trailing separators are ignored.
    * \return File or directory name after the last FolderSeparator().
    */
    CPL_INLINE String FileNameByPath(const String & path_)
    {
        const String path = DirectoryPathRemoveAllLastDash(path_);

#if defined CPL_FILE_USE_FILESYSTEM
        return fs::path(path).filename().string();
#else
        auto pos = path.find_last_of(Cpl::FolderSeparator());
        pos = (pos == String::npos) ? 0 : pos + 1;
        return path.substr(pos);
#endif
    }

    /*! @ingroup cpl_file
    * \brief Returns the filename extension of a path, including the leading '.'.
    * \param [in] path - File path.
    * \return Substring from the last '.' of the filename, or an empty string when the filename
    *         has no extension or the only '.' is at the start (for example ".gitignore").
    */
    CPL_INLINE String ExtensionByPath(const String& path)
    {
        auto filename = FileNameByPath(path);
        assert(filename.find_first_of(FolderSeparator()) == std::string::npos);

        size_t pos = filename.find_last_of(".");
        if (pos == String::npos || pos == 0)
            return String();
        else
            return filename.substr(pos);
    }

    /*! @ingroup cpl_file
    * \brief Returns a path with the last filename extension removed.
    * \param [in] path - File path.
    * \return path without the substring from the last '.'. Unchanged when there is no '.' or the
    *         only '.' is at the start of path.
    */
    CPL_INLINE String RemoveExtension(const String& path)
    {
        size_t last_sep = (size_t) path.find_last_of(".");
        if (last_sep == String::npos || last_sep == 0)
            return path;

        return path.substr(0, last_sep);
    }

    /*! @ingroup cpl_file
    * \brief Replaces the filename extension of a path.
    * \param [in] path - File path.
    * \param [in] ext - New extension, with or without a leading '.'. Whitespace and characters
    *                   listed in forbiddenSymbols are stripped. An empty result after stripping
    *                   removes the existing extension.
    * \return path with the new extension. Unchanged when the filename contains neither a letter
    *         nor a digit (for example "." or "..").
    */
    CPL_INLINE String ChangeExtension(const String& path, const String& ext)
    {
        auto filenamePos = path.find_last_of(Cpl::FolderSeparator());
        filenamePos = (filenamePos == String::npos) ? 0 : filenamePos + 1;
        const String filename = path.substr(filenamePos);

        auto countDigit = std::count_if(filename.begin(), filename.end(), [](const char ch) {
            return isdigit(ch);
        });

        auto countLetter = std::count_if(filename.begin(), filename.end(), [](const char ch) {
            return isalpha(ch);
        });

        if (countDigit == 0 && countLetter == 0){
            return path;
        }

        String extFixed = ext;
        extFixed.erase(std::remove_if(extFixed.begin(), extFixed.end(), [](const char c){
            if (isspace(c))
                return true;

            auto iter = std::find(forbiddenSymbols.begin(), forbiddenSymbols.end(), c);
            if (iter != forbiddenSymbols.end())
                return true;

            return false;
        }), extFixed.end());

        String filenameRemovedExtesion;
        if (extFixed.empty()){
            filenameRemovedExtesion = RemoveExtension(filename);
        }
        else if (extFixed.at(0) == '.'){
            filenameRemovedExtesion = RemoveExtension(filename) + extFixed;
        }
        else {
            filenameRemovedExtesion = RemoveExtension(filename) + '.' + extFixed;
        }

        return MakePath(path.substr(0, filenamePos), filenameRemovedExtesion);
    }

    /*! @ingroup cpl_file
    * \brief Resolves a path to an absolute path.
    * \param [in] path - Path to resolve. Returned unchanged when it is already absolute and basePath is not empty.
    * \param [in] basePath - Base for a relative path. Empty by default, which resolves path against
    *                        the current working directory. When not empty, a relative path is
    *                        resolved against basePath if it is a directory, or against its parent
    *                        if it is a file.
    * \return Absolute path. On Linux, an empty string if realpath() fails when basePath is empty.
    */
    CPL_INLINE String GetAbsolutePath(const String& path, const String& basePath = "")
    {
        if (basePath.empty()) {
#ifdef _WIN32
            std::array<char, MAX_PATH> buffer;
            const char* end = _fullpath(buffer.data(), path.c_str(), MAX_PATH);
            return String(buffer.data());
#elif __linux__
            char resolved_path[PATH_MAX];
            if (!realpath(path.c_str(), resolved_path))
                return String();
            return String(resolved_path);
#else
#error Not supported system
#endif
        } else {
#ifdef CPL_FILE_USE_FILESYSTEM
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
#else
            if (path.empty())
                return path;
            if (path[0] == '/')
                return path;
            String base = DirectoryByPath(basePath);
            if (base.empty())
                base = basePath;
            return MakePath(base, path);
#endif
        }
    }

    /*! @ingroup cpl_file
    * \brief Copies a file or directory tree from src to dst, overwriting existing files.
    * \param [in] src - Source file or directory.
    * \param [in] dst - Destination path.
    * \return true if src and dst are the same or the copy succeeded, false otherwise.
    */
    CPL_INLINE bool Copy(const String& src, const String& dst)
    {
        if (src == dst) {
            return true;
        }

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
            String srcc = src;
            String dstc = dst;

            //double null termitaion
            srcc.push_back(0);
            dstc.push_back(0);

            SHFILEOPSTRUCTA s { };
            s.hwnd = 0;
            s.wFunc = FO_COPY;
            s.fFlags = FOF_SILENT;
            s.pTo = dstc.c_str();
            s.pFrom = srcc.c_str();;
            return SHFileOperationA(&s) == 0;
        }
        catch (...){
        }
        return false;
#elif __linux__
        String com = String("cp -R ") + src + " " + dst;
        return std::system(com.c_str()) == 0;
#else
#error Not supported system
#endif
    }

    /*! @ingroup cpl_file
    * \brief Deletes a regular file.
    * \param [in] filename - File path.
    * \return true if the file was deleted. false if filename does not exist, is a directory,
    *         or deletion fails.
    */
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
        return ::DeleteFileA(filename.c_str());
#elif __linux__
        return unlink(filename.c_str()) == 0;
#else
#error Not supported system
#endif

    }

    /*! @ingroup cpl_file
    * \brief Deletes a directory and its contents recursively.
    * \param [in] dir - Directory path.
    * \return true if the directory was deleted, false otherwise.
    */
    CPL_INLINE bool DeleteDirectory(const String& dir)
    {
#ifdef CPL_FILE_USE_FILESYSTEM
        try {
            std::error_code code;
            auto ret = fs::remove_all(dir);
            return !code && ret != -1;
        }
        catch (...) {
        }
        return 0;
#elif _WIN32
        const String undashed = DirectoryPathRemoveAllLastDash(dir);

        SHFILEOPSTRUCTA operation{};
        operation.wFunc= FO_DELETE;
        operation.fFlags = FOF_NO_UI;

        auto named = GetAbsolutePath(undashed);
        //need double null termination
        named.push_back(0);

        operation.pFrom = named.c_str();

        return SHFileOperationA( &operation ) == 0;

#elif __linux__
        String com = String("rm -rf ") + dir;
        return std::system(com.c_str()) == 0;
#else
#error Not supported system
#endif
    }

    //TODO:
    //CPL_INLINE bool EqualPath(const String& first, const String& second);

    /*! @ingroup cpl_file
    * \brief Returns the directory that contains the current executable.
    * \return Parent directory of the executable path. On Windows this is GetModuleFileNameA
    *         without the file name. On Linux this is the directory of /proc/self/exe.
    */
    CPL_INLINE String GetExecutableLocation()
    {
#if _WIN32
        std::string retval;
        char buf[MAX_PATH];
        DWORD nSize = ::GetModuleFileNameA(NULL, buf, sizeof(buf));
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

    /*! @ingroup cpl_file
    * \brief Reads the size of a file.
    * \param [in] path - File path.
    * \param [out] size - Receives the file size in bytes when the call succeeds.
    * \return true if the file exists and its size was read, false otherwise.
    */
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

    /*! @ingroup cpl_file
    * \brief Sums the sizes of files in a directory.
    * \param [in] path - Directory path.
    * \param [out] size - Receives the total size in bytes when the call succeeds.
    * \return true if the directory was scanned, false otherwise.
    * \note On Windows and when std::filesystem is available the scan is recursive. On Linux
    *       without std::filesystem only the immediate directory entries are summed.
    */
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
        ::WIN32_FIND_DATAA data;
        HANDLE handle = NULL;
        std::queue<String> queue;
        const String filter = "*";
        queue.push(path);

        while (!queue.empty()) {
            auto dir = std::move(queue.front());
            queue.pop();

            handle = FindFirstFileA(MakePath(dir, filter).c_str(), &data);
            if (handle == INVALID_HANDLE_VALUE)
                return false;

            do {
                // skip current directory and parent directory
                if ((strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0)) {
                    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
                        queue.push(Cpl::MakePath(dir, data.cFileName));
                    } else
                        tsize += (size_t) (data.nFileSizeHigh * MAXDWORD + data.nFileSizeLow);
                }

            } while (FindNextFileA(handle, &data));
        }

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

    /*! @ingroup cpl_file
    * \struct FileData
    * \brief In-memory buffer that holds file contents read by ReadFile.
    */
    struct FileData {
        /*!
        * \enum Type
        * \brief Storage format of the file buffer.
        */
        enum class Type {
            Binary,              //!< Raw file bytes with no extra terminator.
            BinaryNullTerminated //!< File bytes followed by an extra 0 byte.
        };


        /*!
        * \struct Error
        * \brief Result of ReadFile. Converts to true for NoError and PartitialRead.
        */
        struct Error {
            /*!
            * \enum ReadFileError
            * \brief Status codes returned by ReadFile.
            */
            enum ReadFileError {
                NoError,         //!< The requested range was read completely.
                PartitialRead,   //!< Only the first maxSize bytes were read.
                FailedToOpen,    //!< The file could not be opened.
                FailedToRead,    //!< The file was opened but the read failed.
                FailedToGetInfo, //!< File size or seek position could not be obtained.
                CommonFail       //!< Unspecified failure, including a missing path.
            };

            /*!
            * \fn Error(Error::ReadFileError code_)
            * \brief Stores a ReadFile status code.
            * \param [in] code_ - Status to store.
            */
            Error(Error::ReadFileError code_) : code(code_) {}
            const ReadFileError code; //!< Status of the ReadFile call.

            /*!
            * \fn operator bool() const
            * \brief Checks whether the read produced usable data.
            * \return true for NoError and PartitialRead, false for the failure codes.
            */
            operator bool() const {
                if (code == NoError || code == PartitialRead) {
                    return true;
                }
                return false;
            }
        };

        /*!
        * \fn FileData(Type type = Type::Binary)
        * \brief Constructs an empty buffer with the given storage format.
        * \param [in] type - Buffer format. Binary by default.
        */
        FileData(Type type = Type::Binary)
                : _type(type)
                , _size(0)
        {};

        /*!
        * \fn const char* data() const
        * \brief Returns a pointer to the buffer, or nullptr when the buffer is empty.
        * \return Pointer to size() bytes, plus a trailing 0 when type is BinaryNullTerminated.
        */
        const char* data() const {
            if (_holder)
                return reinterpret_cast<const char*>(_holder.get());
            else
                return nullptr;
        }

        /*!
        * \fn size_t size() const
        * \brief Returns the number of file bytes stored in the buffer.
        * \return Byte count, not including the extra terminator of BinaryNullTerminated.
        */
        size_t size() const { return _size; }

        /*!
        * \fn bool empty() const
        * \brief Checks whether the buffer holds no data.
        * \return true if data() is nullptr, false otherwise.
        */
        bool empty() const { return !_holder.operator bool(); }

        /*!
        * \fn FileData& operator=(FileData&& other)
        * \brief Moves the buffer from other and leaves other empty.
        * \param [in] other - Buffer to move from.
        * \return Reference to this buffer.
        */
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
                if (_type == Type::BinaryNullTerminated) {
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

    /*! @ingroup cpl_file
    * \brief Writes a byte range to a file.
    * \param [in] filePath - Destination file path.
    * \param [in] data - Bytes to write.
    * \param [in] size - Number of bytes to write.
    * \param [in] recreate - If true, create or overwrite the file. If false, append. True by default.
    * \return -1 on success, 0 on failure.
    */
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

    /*! @ingroup cpl_file
    * \brief Reads a file into an existing FileData buffer.
    * \param [in] path - File path.
    * \param [out] out - Destination buffer. Its Type selects Binary or BinaryNullTerminated storage.
    * \param [in] startPos - Byte offset to start reading from. 0 by default.
    * \param [in] maxSize - Maximum number of bytes to read. 1 GB by default.
    * \return FileData::Error. NoError on a complete read, PartitialRead when the remaining file
    *         is larger than maxSize. Opening a directory returns FailedToRead on Linux and
    *         CommonFail on Windows. A missing path returns CommonFail.
    */
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

    /*! @ingroup cpl_file
    * \brief Loads a file as a sequence of T values.
    * \tparam T - Element type. The file size must be a multiple of sizeof(T) for a complete last element.
    * \param [in] path - File path.
    * \param [out] data - Destination resized to file_size / sizeof(T) and filled from the file.
    * \return true if the file was opened and read, false otherwise.
    */
    template<class T> CPL_INLINE bool LoadBinaryData(const String& path, std::vector<T>& data)
    {
        std::ifstream ifs(path.c_str(), std::ofstream::binary);
        if (!ifs.is_open())
            return false;
        size_t beg = ifs.tellg();
        ifs.seekg(0, std::ios::end);
        size_t end = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        size_t size = (end - beg) / sizeof(T);
        data.resize(size);
        ifs.read((char*)data.data(), size * sizeof(T));
        ifs.close();
        return true;
    }

    /*! @ingroup cpl_file
    * \brief Writes a sequence of T values to a file as raw bytes.
    * \tparam T - Element type.
    * \param [in] data - Values to write.
    * \param [in] path - Destination file path. The file is created or overwritten.
    * \return true if the file was opened and written, false otherwise.
    */
    template<class T> CPL_INLINE bool SaveBinaryData(const std::vector<T>& data, const String& path)
    {
        std::ofstream ofs(path.c_str(), std::ofstream::binary);
        if (!ofs.is_open())
            return false;
        ofs.write((const char*)data.data(), data.size() * sizeof(T));
        bool result = (bool)ofs;
        ofs.close();
        return result;
    }

    /*! @ingroup cpl_file
    * \brief Checks whether a path can be opened for reading.
    * \param [in] path - File path.
    * \return true if an input stream to path is good, false otherwise.
    */
    CPL_INLINE bool FileIsReadable(const String& path) {
        try {
            std::ifstream file(path.c_str());
            const bool state = file.good();
            file.close();
            return state;
        }
        catch (...) {}
        return false;
    }

    /*! @ingroup cpl_file
    * \brief Checks whether an existing file can be opened for writing.
    * \param [in] path - File path.
    * \return true if the file exists and can be opened for append. false if the file does not
    *         exist or is not writable.
    */
    CPL_INLINE bool FileIsWritable(const String& path) {
        try {
            if (!FileExists(path)){
                return false;
            }

            bool state = false;
            //Combination std::ios_base::in | std::ios_base::out does not create a file, try to open already exist
            std::fstream file(path.c_str(), std::ios_base::app | std::ios_base::out );
            if (file.is_open()) {
                state = file.good();
                file.close();
            }
            return state;
        }
        catch (...) {}
        return false;
    }

    /*          Deprecated block        */


    /*! @ingroup cpl_file
    * \brief Returns the last component of a path, including the extension.
    * \param [in] path_ - File or directory path.
    * \return Same result as FileNameByPath(path_).
    * \deprecated Use FileNameByPath instead.
    */
    CPL_INLINE String GetNameByPath(const String& path_) {
        static std::once_flag onceFlag;
        std::call_once ( onceFlag, [ ]{
            CPL_LOG(Warning, "Cpl::GetNameByPath is deprecated, will be removed soon, please use Cpl::FileNameByPath instead");
        } );

        return FileNameByPath(path_);
    }

    /*! @ingroup cpl_file
    * \brief Copies a file or directory tree from src to dst.
    * \param [in] src - Source file or directory.
    * \param [in] dst - Destination path.
    * \return Same result as Copy(src, dst).
    * \deprecated Use Copy instead.
    */
    CPL_INLINE bool CopyDirectory(const String& src, const String& dst) {
        static std::once_flag onceFlag;
        std::call_once ( onceFlag, [ ]{
            CPL_LOG(Warning, "Cpl::CopyDirectory is deprecated, will be removed soon, please use Cpl::Copy instead");
        } );
        return Copy(src, dst);
    }
}
