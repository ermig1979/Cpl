/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2023-2023 Daniil Germanenko.
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

#include "Cpl/File.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <set>

#ifdef __linux__
#include <unistd.h>
extern "C" {
    #include <sys/stat.h>
}
#endif

#ifdef _WIN32
#include <direct.h>
#endif

/* Test file hierarchy

FOR LINUX:
----/tmp/cpl/1
            /2
f               22222.js
            /zero0
                /zero0test
f           emptyFile.js
f           notempty.txt
f           notemptyx2
*/

#define COMPARE_RESULT(definition, target) [&]()->bool{bool result = definition; if (result != target) { std::cout << "Not correct result on row " << __LINE__  << std::endl;} return result;}();
namespace Test
{
    std::string joinPath(const std::string& a, const std::string& b) {
        assert(a.size());
        assert(b.size());
#ifdef __linux__
        return a + "/" + b;
#elif _WIN32
        return a + '\\' + b;
#endif
    }
#ifdef __linux__
    static const std::string testPath = "/tmp/cpl";
#elif _WIN32
    static const std::string testPath = ".\\cpl";
#endif
    static const std::string testString = "123456789876543210";

    const static std::set<std::string> all_folders = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "1"));
        temp.insert(joinPath(testPath, "2"));
        temp.insert(joinPath(testPath, "zero0"));
        temp.insert(joinPath(testPath, joinPath("zero0", "test")));
        return temp;
    }();

    const static std::set<std::string> not_exist_folders = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "999"));
        temp.insert(joinPath(testPath, joinPath("zxcb", "999")));
        temp.insert(joinPath(testPath, "8"));
        return temp;
    }();

    const static std::set<std::string> existance_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "emptyFile.js"));
        temp.insert(joinPath(testPath, joinPath("2", "22222.js")));
        temp.insert(joinPath(testPath, "notempty.txt"));
        temp.insert(joinPath(testPath, "notemptyx2"));
        return temp;
    }();

    const static std::set<std::string> not_existance_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "bemptyFile.js"));
        temp.insert(joinPath(testPath, joinPath("2", "b22222.js")));
        temp.insert(joinPath(testPath, "bnotempty.txt"));
        temp.insert(joinPath(testPath, "bnotemptyx2"));
        return temp;
    }();

    const static std::set<std::string> empty_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "emptyFile.js"));
        temp.insert(joinPath(testPath, joinPath("2", "22222.js")));
        return temp;
    }();

    //Every file have at least test string testString
    const static std::vector<std::pair<std::string, size_t>> not_empty_files = [](){
        std::vector<std::pair<std::string, size_t>> temp;
        temp.push_back({joinPath(testPath, "notempty.txt"), testString.size()});
        temp.push_back({joinPath(testPath, "notemptyx2"), 2*testString.size()});
        return temp;
    }();

    bool initializeTree() {
#ifdef __linux__
        auto p = mkdir(testPath.c_str(), 0777);
        p |= mkdir(joinPath(testPath, "1").c_str(), 0777);
        p |= mkdir(joinPath(testPath, "2").c_str(), 0777);
        p |= mkdir(joinPath(testPath, "zero0").c_str(), 0777);
        p |= mkdir(joinPath(testPath, joinPath("zero0", "test")).c_str(), 0777);
#elif _WIN32
        auto p = _mkdir(testPath.c_str());
        p |= _mkdir(joinPath(testPath, "1").c_str());
        p |= _mkdir(joinPath(testPath, "2").c_str());
        p |= _mkdir(joinPath(testPath, "zero0").c_str());
        p |= _mkdir(joinPath(testPath, joinPath("zero0", "test")).c_str());
#endif

        if (p)
            return false;

        std::ofstream f1(joinPath(testPath, "emptyFile.js"));
        f1.close();

        std::ofstream f2(joinPath(testPath, joinPath("2", "22222.js")));
        f2.close();

        std::ofstream f3(joinPath(testPath, "notempty.txt"));
        f3 << testString;
        f3.close();

        std::ofstream f4(joinPath(testPath, "notemptyx2"));
        f4 << testString << testString;
        f4.close();

        return true;
    }

    namespace Existance{
        bool testFileExists(){
            bool ok = true;

            //Common case #1 
            int cnt1 = 0;
            for (auto& exist : existance_files){
                cnt1 += COMPARE_RESULT(Cpl::FileExists(exist), true);
            }

            ok &= COMPARE_RESULT(cnt1 == existance_files.size(), 1);

            int cnt2 = 0;
            for (auto& not_exist : not_existance_files){
                cnt2 += COMPARE_RESULT(Cpl::FileExists(not_exist), false);
            }

            ok &= COMPARE_RESULT(cnt2 == 0, 1);

            //Inversion previous
            int cnt3 = 0;
            for (auto& not_exist : not_existance_files){
                cnt3 += !COMPARE_RESULT(Cpl::FileExists(not_exist), false);
            }

            ok &= COMPARE_RESULT(cnt3 == not_existance_files.size(), 1);

            //Check folders as files #4
            int cnt4 = 0;
            for (auto& exist : all_folders){
                cnt4 += COMPARE_RESULT(Cpl::FileExists(exist), 0);
            }
            ok &= COMPARE_RESULT(cnt4 == 0, 1);

            //Check parent folders as files #5
            int cnt5 = 0;
            for (auto& exist : existance_files){
                cnt5 += COMPARE_RESULT(Cpl::FileExists(Cpl::DirectoryByPath(exist)), 0);
            }
            ok &= COMPARE_RESULT(cnt5 == 0, 1);

            //Check files via up and down path #6
            int cnt6 = 0;
            for (auto& exist : existance_files){
                auto dir = Cpl::DirectoryByPath(exist);
                auto filename = Cpl::FileNameByPath(exist);
                cnt6 += COMPARE_RESULT(Cpl::FileExists(Cpl::MakePath(dir, filename)), 1);
            }
            ok &= COMPARE_RESULT(cnt6 == existance_files.size(), 1);


            return ok;
        }

        bool testFolderExists(){
            bool ok = true;

            int cnt1 = 0;
            //Common case #1 
            for (auto& exist : all_folders){
                cnt1 += COMPARE_RESULT(Cpl::DirectoryExists(exist), 1);
            }

            ok &= COMPARE_RESULT(cnt1 == existance_files.size(), 1);

            //Common cases #2
            int cnt2 = 0;
            for (auto& not_exist : not_exist_folders){
                cnt2 += COMPARE_RESULT(Cpl::DirectoryExists(not_exist), 0);
            }

            ok &= COMPARE_RESULT(cnt2 == 0, 1);

            //Check parent folder of exist files #3
            int cnt3 = 0;
            for (auto& exist : existance_files){
                cnt3 += COMPARE_RESULT(Cpl::DirectoryExists(Cpl::DirectoryByPath(exist)), 1);
            }

            ok &= COMPARE_RESULT(cnt3 == existance_files.size(), 1);

            //Check files as folders #4
            int cnt4 = 0;
            for (auto& exist : existance_files){
                cnt3 += COMPARE_RESULT(Cpl::DirectoryExists(exist), 0);
            }

            ok &= COMPARE_RESULT(cnt4 == 0, 1);

            int cnt5 = 0;
            //Case if directory ends with symbol "/"
            for (auto& exist : all_folders){
                cnt5 += COMPARE_RESULT(Cpl::DirectoryExists(Cpl::MakePath(exist, "")), 1);
            }

            ok &= COMPARE_RESULT(cnt5 == all_folders.size(), 1);

            return ok;
        }
    }

    namespace Modify{

        bool folders(){
            bool ok = true;
            static std::string folderName1 = "4";
            std::string newFolder = joinPath(testPath, folderName1);

            if (Cpl::DirectoryExists(newFolder))
                ok &= COMPARE_RESULT(Cpl::DeleteDirectory(newFolder), 1);

            //Case 1
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 0);
            ok &= COMPARE_RESULT(Cpl::CreatePath(newFolder), 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 1);
            ok &= COMPARE_RESULT(Cpl::DeleteDirectory(newFolder), 1);
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 0);

            //Case 2
            static std::string folderName2 = "5";
            std::string newFolder2 = joinPath(newFolder, folderName2);
            // /tmp/cpl/4/5

            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder2), 0);

            ok &= COMPARE_RESULT(Cpl::CreatePath(newFolder2), 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryExists(newFolder2), 1);

            ok &= COMPARE_RESULT(Cpl::DeleteDirectory(newFolder2), 1);
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder2), 0);
            ok &= COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 1);
            ok &= COMPARE_RESULT(Cpl::DeleteDirectory(newFolder), 1);
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 0);

            //Case 2.1
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 0);
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder2), 0);

            ok &= COMPARE_RESULT(Cpl::CreatePath(newFolder2), 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryExists(newFolder2), 1);

            ok &= COMPARE_RESULT(Cpl::DeleteDirectory(newFolder), 1);
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder2), 0);
            ok &= !COMPARE_RESULT(Cpl::DirectoryExists(newFolder), 0);

#ifdef _WIN32
            ok &= COMPARE_RESULT(Cpl::DirectoryExists("C://"), 1);

            if (!Cpl::DirectoryExists("X://")) {
                ok &= !COMPARE_RESULT(Cpl::CreatePath("X:\\zxv\\123\\312\\abcdefg"), 0);
            }
#endif
            return ok;
        }

        bool createFiles(){
            const Cpl::String tempfilename = Cpl::MakePath(testPath, "write_test.bin");
            bool ok = true;
            if (Cpl::FileExists(tempfilename)){
                if (!Cpl::DeleteFile(tempfilename)){
                    CPL_LOG_SS(Error, "Cant delete file " << __LINE__);
                    return false;
                }
            }
            const uint8_t d[] = {0x01, 0x02, 0x03, 0x04, 0xff};

            {
                //Case 1, create
                ok &= COMPARE_RESULT(Cpl::WriteToFile(tempfilename, (const char *) d, sizeof(d)), 1);
                size_t size = 0;
                ok &= COMPARE_RESULT(Cpl::FileSize(tempfilename, size), 1);
                if (size != sizeof(d)){
                    Cpl::DeleteFile(tempfilename);
                    return false;
                }

                Cpl::FileData fd;
                ok &= COMPARE_RESULT(Cpl::ReadFile(tempfilename, fd), 1);
                ok &= COMPARE_RESULT(fd.size() == sizeof(d), 1);

                if (!ok) {
                    Cpl::DeleteFile(tempfilename);
                    return false;
                }
                for (size_t ind = 0; ind < sizeof(d); ind++) {
                    COMPARE_RESULT(ok &= (uint8_t) fd.data()[ind] == d[ind], 1);
                }
            }
            {
                //Case 2, append data
                ok &= COMPARE_RESULT(Cpl::WriteToFile(tempfilename, (const char *) d + 1, sizeof(d) / 2, false), 1);
                size_t size = 0;
                ok &= COMPARE_RESULT(Cpl::FileSize(tempfilename, size), 1);
                if (size != sizeof(d) + (sizeof(d) / 2)) {
                    Cpl::DeleteFile(tempfilename);
                    CPL_LOG_SS(Error, "Not correct size " << __LINE__);
                    return false;
                }

                Cpl::FileData fd;
                ok &= COMPARE_RESULT(Cpl::ReadFile(tempfilename, fd), 1);
                ok &= COMPARE_RESULT(fd.size() == sizeof(d) + (sizeof(d) / 2), 1);

                if (!ok) {
                    CPL_LOG_SS(Error, "Create files test aborted " << __LINE__);
                    Cpl::DeleteFile(tempfilename);
                    return false;
                }

                for (size_t ind = 0; ind < sizeof(d); ind++) {
                    ok &= COMPARE_RESULT((uint8_t) fd.data()[ind] == d[ind], 1);
                }

                for (size_t ind = 0; ind < sizeof(d) / 2; ind++) {
                    ok &= COMPARE_RESULT((uint8_t) fd.data()[sizeof(d) + ind] == d[ind + 1], 1);
                }
            }

            {
                //Case 3, rewrite
                ok &= COMPARE_RESULT(Cpl::WriteToFile(tempfilename, (const char *) d, sizeof(d)), 1);
                size_t size = 0;
                ok &= COMPARE_RESULT(Cpl::FileSize(tempfilename, size), 1);
                if (size != sizeof(d)) {
                    CPL_LOG_SS(Error, "Create files test aborted " << __LINE__);
                    Cpl::DeleteFile(tempfilename);
                    return false;
                }

                Cpl::FileData fd;
                ok &= COMPARE_RESULT(Cpl::ReadFile(tempfilename, fd), 1);
                ok &= COMPARE_RESULT(fd.size() == sizeof(d), 1);

                if (!ok) {
                    CPL_LOG_SS(Error, "Create files test aborted " << __LINE__);
                    Cpl::DeleteFile(tempfilename);
                    return false;
                }

                for (size_t ind = 0; ind < sizeof(d); ind++) {
                    ok &= COMPARE_RESULT((uint8_t) fd.data()[ind] == d[ind], 1);
                }
            }

            ok &= COMPARE_RESULT(Cpl::DeleteFile(tempfilename), 1);

            return ok;
        }

        bool readFormatsTest(){
            bool ok = true;

            {
                //Test binary data
                for (const auto& elem : not_empty_files){
                    Cpl::FileData fd;
                    auto error = Cpl::ReadFile(elem.first, fd);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::NoError, 1);
                    ok &= !COMPARE_RESULT(fd.empty(), 0);
                    ok &= COMPARE_RESULT(fd.size() == elem.second, 1);

                    for (size_t ind = 0; ind < testString.size(); ind++) {
                        ok &= COMPARE_RESULT(fd.data()[ind] == testString.c_str()[ind], 1);
                    }
                }

                //Test null terminated data
                for (const auto& elem : not_empty_files){
                    Cpl::FileData fd(Cpl::FileData::Type::BinaryNullTerminated);
                    auto error = Cpl::ReadFile(elem.first, fd);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::NoError, 1);
                    ok &= COMPARE_RESULT(fd.size() == elem.second, 1);
                    ok &= !COMPARE_RESULT(fd.empty(), 0);
                    ok &= COMPARE_RESULT(fd.data()[fd.size()] == 0, 1);

                    for (size_t ind = 0; ind < testString.size(); ind++) {
                        ok &= COMPARE_RESULT(fd.data()[ind] == testString.c_str()[ind], 1);
                    }
                }

                //Test binary data of empty files
                for (const auto& elem : empty_files){
                    Cpl::FileData fd;
                    auto error = Cpl::ReadFile(elem, fd);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::NoError, 1);
                    ok &= COMPARE_RESULT(fd.size() == 0, 1);
                    ok &= COMPARE_RESULT(fd.empty(), 1);
                    ok &= COMPARE_RESULT(fd.data() == nullptr, 1);
                }

                //Test null terminated data of empty files
                for (const auto& elem : empty_files){
                    Cpl::FileData fd(Cpl::FileData::Type::BinaryNullTerminated);
                    auto error = Cpl::ReadFile(elem, fd);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::NoError, 1);
                    ok &= COMPARE_RESULT(fd.size() == 0, 1);
                    ok &= COMPARE_RESULT(fd.empty(), 1);
                    ok &= COMPARE_RESULT(fd.data() == nullptr, 1);
                }

                {
                    //Not existing file binary
                    Cpl::FileData fd(Cpl::FileData::Type::Binary);
                    Cpl::FileData fd_empty(Cpl::FileData::Type::Binary);
                    auto error = Cpl::ReadFile(*not_existance_files.begin(), fd);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::CommonFail, 1);
                    ok &= COMPARE_RESULT(fd.size() == 0, 1);
                    ok &= COMPARE_RESULT(fd.empty(), 1);
                    ok &= COMPARE_RESULT(fd.data() == nullptr, 1);
                    ok &= COMPARE_RESULT(fd.size() == fd_empty.size(), 1);
                    ok &= COMPARE_RESULT(fd.empty() == fd_empty.empty(), 1);
                    ok &= COMPARE_RESULT(fd.data() == fd_empty.data(), 1);
                }

                {
                    //Existing file, binary format, budget less than file size
                    Cpl::FileData fd(Cpl::FileData::Type::Binary);
                    auto error = Cpl::ReadFile(not_empty_files[0].first, fd, 0, testString.size() - 1);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::PartitialRead, 1);
                    ok &= COMPARE_RESULT(fd.size() == testString.size() - 1, 1);
                    ok &= !COMPARE_RESULT(fd.empty(), 0);
                    ok &= COMPARE_RESULT(fd.data() != nullptr, 1);

                    for (size_t ind = 0; ind < fd.size(); ind++) {
                        ok &= COMPARE_RESULT(fd.data()[ind] == testString.c_str()[ind], 1);
                    }
                }

                {
                    //Existing file, null terminated format, budget less than file size
                    Cpl::FileData fd(Cpl::FileData::Type::BinaryNullTerminated);
                    auto error = Cpl::ReadFile(not_empty_files[0].first, fd, 0, testString.size() - 1);
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::PartitialRead, 1);
                    ok &= COMPARE_RESULT(fd.size() == testString.size() - 1, 1);
                    ok &= !COMPARE_RESULT(fd.empty(), 0);
                    ok &= COMPARE_RESULT(fd.data() != nullptr, 1);

                    for (size_t ind = 0; ind < fd.size(); ind++) {
                        ok &= COMPARE_RESULT(fd.data()[ind] == testString.c_str()[ind], 1);
                    }

                    ok &= COMPARE_RESULT(fd.data()[fd.size()] == 0, 1);
                }

                {
                    //Open folder
                    Cpl::FileData fd(Cpl::FileData::Type::Binary);
                    Cpl::FileData fd_empty(Cpl::FileData::Type::Binary);
                    auto error = Cpl::ReadFile(*all_folders.begin(), fd);
#if __linux__
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::FailedToRead, 1);
#elif _WIN32
                    ok &= COMPARE_RESULT(error.code == Cpl::FileData::Error::ReadFileError::CommonFail, 1);
#endif

                    ok &= COMPARE_RESULT(fd.size() == 0, 1);
                    ok &= COMPARE_RESULT(fd.empty(), 1);
                    ok &= COMPARE_RESULT(fd.data() == nullptr, 1);
                    ok &= COMPARE_RESULT(fd.size() == fd_empty.size(), 1);
                    ok &= COMPARE_RESULT(fd.empty() == fd_empty.empty(), 1);
                    ok &= COMPARE_RESULT(fd.data() == fd_empty.data(), 1);

                }
            }

            return ok;
        }

        bool copy(){

            //Copy file
            bool ok = true;
            if (Cpl::FileExists(not_empty_files.front().first + "1")){
                Cpl::DeleteFile(not_empty_files.front().first + "1");
            }

            ok &= !COMPARE_RESULT(Cpl::FileExists(not_empty_files.front().first + "1"), 0);
            ok &= COMPARE_RESULT(Cpl::Copy(not_empty_files.front().first, not_empty_files.front().first + "1"), 1);

            ok &= COMPARE_RESULT(Cpl::FileExists(not_empty_files.front().first + "1"), 1);

            size_t size = 0;
            ok &= COMPARE_RESULT(Cpl::FileSize(not_empty_files.front().first + "1", size), 1);
            ok &= COMPARE_RESULT((size == not_empty_files.front().second), 1);
            ok &= COMPARE_RESULT(Cpl::DeleteFile(not_empty_files.front().first + "1"), 1);
            ok &= !COMPARE_RESULT(Cpl::FileExists(not_empty_files.front().first + "1"), 0);

            //Copy folder

            auto tdir = Cpl::String(testPath + "1");
            if (Cpl::DirectoryExists(tdir)){
                Cpl::DeleteDirectory(tdir);
            }

            ok &= COMPARE_RESULT(Cpl::Copy(testPath.c_str(), tdir.c_str()), 1);

            auto list1 = Cpl::GetFileList(testPath, "", true, true, true);
            auto list2 = Cpl::GetFileList(tdir, "", true, true, true);

            ok &= COMPARE_RESULT(list1.size() == list2.size(), 1);
            if (list1.size() == list2.size()){
                auto l1 = list1.begin();
                auto l2 = list2.begin();
                while (l1 != list1.end()){
                    ok &= COMPARE_RESULT(Cpl::FileNameByPath(*l1) == Cpl::FileNameByPath(*l2), 1);
                    l1++;
                    l2++;
                }
            }

            size_t size1, size2;
            ok &= COMPARE_RESULT(Cpl::DirectorySize(testPath, size1), 1);
            ok &= COMPARE_RESULT(Cpl::DirectorySize(tdir, size2), 1);
            ok &= COMPARE_RESULT(size1 == size2, 1);

            ok &= COMPARE_RESULT(Cpl::DeleteDirectory(tdir), 1);

            return ok;
        }
    }

    namespace Info {

        bool fileList() {
            bool ok = true;

            {
                auto files = Cpl::GetFileList(testPath, "", true, false, false);
                ok &= files.size() == 3;
                for (const auto &concrete_filename_path: files) {
                    ok &= !COMPARE_RESULT(concrete_filename_path.empty(), 0);
                    auto directory = Cpl::DirectoryByPath(concrete_filename_path);
                    //TODO: fix comparison
                    ok &= COMPARE_RESULT(directory == testPath, 1);
                }


                auto filesRecursive = Cpl::GetFileList(testPath, "", true, false, true);
                ok &= COMPARE_RESULT(filesRecursive.size() == existance_files.size(), 1);

                auto dirs = Cpl::GetFileList(testPath, "", false, true, false);
                ok &= COMPARE_RESULT(dirs.size() == 3, 1);

                auto dirsRecursive = Cpl::GetFileList(testPath, "", false, true, true);
                ok &= COMPARE_RESULT(dirsRecursive.size() == 4, 1);

            }
            return ok;
        }

        bool naming() {
            bool ok = true;

            auto folder = Cpl::FileNameByPath(testPath);
            ok &= COMPARE_RESULT(folder == "cpl", 1);

            folder = Cpl::FileNameByPath(testPath + Cpl::FolderSeparator());

            ok &= COMPARE_RESULT(folder == "cpl", 1);

            ok &= COMPARE_RESULT(Cpl::DirectoryPathRemoveAllLastDash(Cpl::MakePath(testPath, Cpl::FolderSeparator())) == testPath, 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryPathRemoveAllLastDash(Cpl::MakePath(testPath, Cpl::MakePath(Cpl::FolderSeparator(), " "))) == testPath, 1);
            ok &= COMPARE_RESULT(Cpl::DirectoryPathRemoveAllLastDash(Cpl::MakePath(testPath, Cpl::MakePath(Cpl::FolderSeparator(), Cpl::FolderSeparator()))) == testPath, 1);

            for (const auto &file: existance_files) {
                auto filename = Cpl::FileNameByPath(file);
                auto filedir = Cpl::DirectoryByPath(file);

                ok &= COMPARE_RESULT(Cpl::MakePath(filedir, filename) == file, 1);
                ok &= COMPARE_RESULT(Cpl::FileExists(Cpl::MakePath(filedir, filename)), 1);
                ok &= COMPARE_RESULT(Cpl::FileExists(Cpl::MakePath(Cpl::MakePath(filedir, ""), filename)), 1);
            }
            return ok;
        }

        bool extension() {
            bool ok = true;
            //Test get extention
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("..", ""), "png") == Cpl::MakePath("..", ""), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", ""), "png") == Cpl::MakePath(".", ""), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(".......", "png") == ".......", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(".......jpeg", "png") == ".......png", 1);
                  
            ok &= COMPARE_RESULT(Cpl::ExtensionByPath("photo.jpeg") == ".jpeg", 1);
            ok &= COMPARE_RESULT(Cpl::ExtensionByPath("photo").size() == 0, 1);
            ok &= COMPARE_RESULT(Cpl::ExtensionByPath("photo.") == ".", 1);
            ok &= COMPARE_RESULT(Cpl::ExtensionByPath(".b").size() == 0, 1);
            ok &= COMPARE_RESULT(Cpl::ExtensionByPath("...b") == ".b", 1);
            ok &= COMPARE_RESULT(Cpl::ExtensionByPath("..a.b") == ".b", 1);
                  
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("photo.jpeg") == "photo", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("photo.") == "photo", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("photo") == "photo", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("") == "", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension(".a") == ".a", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("...b") == "..", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("..a.b") == "..a", 1);
            ok &= COMPARE_RESULT(Cpl::RemoveExtension("..a.b....zyx") == "..a.b...", 1);
                  
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("photo.jpeg", "png") == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("photo.jpeg", ".png") == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("photo.", ".png") == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("test.photo.", ".png") == "test.photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("test.photo.", "png") == "test.photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("photo", ".png") == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("photo", "png") == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("", ".png") == "", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension("", "png") == "", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(".a", ".png") == ".a.png", 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(".a", "png") == ".a.png", 1);
                  
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a"), "png") == Cpl::MakePath(".", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a"), ".png") == Cpl::MakePath(".", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a"), "") == Cpl::MakePath(".", "a"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a"), " ") == Cpl::MakePath(".", "a"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a"), ".") == Cpl::MakePath(".", "a."), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "ab."), ".") == Cpl::MakePath(".", "ab."), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a.z"), ".") == Cpl::MakePath(".", "a."), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a.z"), ".random") == Cpl::MakePath(".", "a.random"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a.z"), "random") == Cpl::MakePath(".", "a.random"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "abc.z"), "random") == Cpl::MakePath(".", "abc.random"), 1);
                  
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a.jpeg"), "png") ==  Cpl::MakePath(".", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", "a.jpeg"), ".png") ==  Cpl::MakePath(".", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath(".", ".jpeg"), ".png") ==  Cpl::MakePath(".", ".jpeg.png"), 1);
                  
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("..", "a.jpeg"), "png") ==  Cpl::MakePath("..", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("..", "a.jpeg"), ".png") ==  Cpl::MakePath("..", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("..", ".jpeg"), ".png") ==  Cpl::MakePath("..", ".jpeg.png"), 1);
                  
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("abc", "a.jpeg"), "png") ==  Cpl::MakePath("abc", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("abcz123", "a.jpeg"), ".png") ==  Cpl::MakePath("abcz123", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("poiabc", ".jpeg"), ".png") ==  Cpl::MakePath("poiabc", ".jpeg.png"), 1);
                  
            //Parent folder with dot
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("ab.c", "a.jpeg"), "png") ==  Cpl::MakePath("ab.c", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("abcz.123", "a.jpeg"), ".png") ==  Cpl::MakePath("abcz.123", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("poiab.c", ".jpeg"), ".png") ==  Cpl::MakePath("poiab.c", ".jpeg.png"), 1);
                  
            //Full paths
#ifdef _WIN32 
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("C:", "a.jpeg"), "png") ==  Cpl::MakePath("C:", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("C:", Cpl::MakePath("folder1", "a.jpeg")), "png") ==  Cpl::MakePath("C:", Cpl::MakePath("folder1", "a.png")), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("C:", Cpl::MakePath("fol.der1", "a.jpeg")), "png") ==  Cpl::MakePath("C:", Cpl::MakePath("fol.der1", "a.png")), 1);
#endif            
                  
#ifdef __linux__  
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("/tmp", "a.jpeg"), "png") ==  Cpl::MakePath("/tmp", "a.png"), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("/tmp", Cpl::MakePath("folder1", "a.jpeg")), "png") ==  Cpl::MakePath("/tmp", Cpl::MakePath("folder1", "a.png")), 1);
            ok &= COMPARE_RESULT(Cpl::ChangeExtension(Cpl::MakePath("/tmp", Cpl::MakePath("fol.der1", "a.jpeg")), "png") ==  Cpl::MakePath("/tmp", Cpl::MakePath("fol.der1", "a.png")), 1);
#endif

            return ok;
        }

        bool pathing() {
            bool ok = true;

            ok &= COMPARE_RESULT(Cpl::FileNameByPath(Cpl::MakePath("", Cpl::MakePath("usr", Cpl::MakePath("local", "photo.png")))) == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::FileNameByPath("photo.png") == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::FileNameByPath(Cpl::MakePath(".", "photo.png")) == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::FileNameByPath(Cpl::MakePath("..", Cpl::MakePath(".", Cpl::MakePath("ab", Cpl::MakePath("..", "photo.png"))))) == "photo.png", 1);
            ok &= COMPARE_RESULT(Cpl::FileNameByPath(Cpl::MakePath("..", Cpl::MakePath(".", Cpl::MakePath("a.b", Cpl::MakePath("..", "photo."))))) == "photo.", 1);
            ok &= COMPARE_RESULT(Cpl::FileNameByPath(Cpl::MakePath("..", Cpl::MakePath(".", Cpl::MakePath("a.b", Cpl::MakePath("..", "photo"))))) == "photo", 1);
            ok &= COMPARE_RESULT(Cpl::FileNameByPath(Cpl::MakePath("..", Cpl::MakePath(".", Cpl::MakePath("a.b", Cpl::MakePath("..", ".a"))))) == ".a", 1);

            return ok;
        }

        bool fileSizing() {
            bool ok = true;
            try {
                if (existance_files.size() == 0) return false;
                if (empty_files.size() == 0) return false;

                for (const auto &path: existance_files) {
                    if (!Cpl::FileExists(path))
                        return false;

                    {  //Search in not empty list
                        decltype(not_empty_files)::const_iterator iter = std::find_if(not_empty_files.begin(), not_empty_files.end(), [path](const decltype(not_empty_files)::value_type &pair) {
                            if (pair.first == path) {
                                return true;
                            }
                            return false;
                        });

                        if (iter != not_empty_files.end()) {
                            size_t size = 0;
                            ok &= COMPARE_RESULT(Cpl::FileSize(iter->first,size), 1);
                            ok &= COMPARE_RESULT(size == iter->second, 1);
                            continue;
                        }
                    }

                    {  //Search in empty list
                        auto empty_iter = empty_files.find(path);
                        if (empty_iter != empty_files.end()) {
                            size_t size = 0;
                            ok &= COMPARE_RESULT(Cpl::FileSize(*empty_iter, size), 1);
                            ok &= COMPARE_RESULT(size == 0, 1);
                            continue;
                        }
                    }
                }

                for (const auto &path: not_existance_files) {
                    if (Cpl::FileExists(path)) {
                        return false;
                    }

                    size_t size = 0;
                    ok &= !COMPARE_RESULT(Cpl::FileSize(path, size), 0);
                    ok &= COMPARE_RESULT(size == 0, 1);
                }

                return ok;
            }
            catch (...) {
            }

            return false;
        }

        bool directorySizing() {
            bool ok = true;
            if (existance_files.size() == 0) return false;
            size_t size = 0;
            size_t calc_size = 0;

            for (const auto &path: not_empty_files) {
                calc_size += path.second;
            }

            ok &= COMPARE_RESULT(Cpl::DirectorySize(testPath, size), 1);
            CPL_LOG_SS(Info, "Dir size " << size);

            ok &= COMPARE_RESULT(size == calc_size, 1);

            return ok;
        }

        bool system() {
            //TODO: add test of GetAbsolutePath with usage environment variable for check path
            bool ok = true;
            ok &= COMPARE_RESULT(Cpl::GetAbsolutePath(testPath) != testPath, 1);
            ok &= COMPARE_RESULT(Cpl::GetAbsolutePath(testPath).empty(), 0);
            return ok;
        }
    }


    bool DoFileModifyTest() {
        bool ok = true;
        try {
            CPL_LOG_SS(Info, "Filesystem " << Cpl::FilesystemType());
            CPL_LOG_SS(Info, "Compiler type " << Cpl::CompilerType());

            initializeTree();
            ok &= COMPARE_RESULT(Modify::folders(), 1);
            ok &= COMPARE_RESULT(Modify::createFiles(), 1);
            ok &= COMPARE_RESULT(Modify::readFormatsTest(), 1);
            ok &= COMPARE_RESULT(Modify::copy(), 1);

            return ok;
        }
        catch (...) {
        }
        return false;
    };

    bool DoFileExistanceTest() {
        bool ok = true;
        try {
            CPL_LOG_SS(Info, "Filesystem " << Cpl::FilesystemType());
            CPL_LOG_SS(Info, "Compiler type " << Cpl::CompilerType());

            initializeTree();
            ok &= COMPARE_RESULT(Existance::testFileExists(), 1);
            ok &= COMPARE_RESULT(Existance::testFolderExists(), 1);

            return ok;
        }
        catch (...) {
        }
        return false;
    };


    bool DoFileInfoTest() {
        bool ok = true;
        try {
            CPL_LOG_SS(Info, "Filesystem " << Cpl::FilesystemType());
            CPL_LOG_SS(Info, "Compiler type " << Cpl::CompilerType());

            initializeTree();
            ok &= COMPARE_RESULT(Info::fileList(), 1);
            ok &= COMPARE_RESULT(Info::naming(), 1);
            ok &= COMPARE_RESULT(Info::extension(), 1);
            ok &= COMPARE_RESULT(Info::pathing(), 1);
            ok &= COMPARE_RESULT(Info::fileSizing(), 1);
            ok &= COMPARE_RESULT(Info::directorySizing(), 1);

            return ok;
        }
        catch (...) {
        }
        return false;
    };
}