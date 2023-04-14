/*
* Tests for Common Purpose Library (http://github.com/ermig1979/Cpl).
*
* Copyright (c) 2021-2022 Yermalayeu Ihar,
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

#include "Cpl/File.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <unistd.h>

extern "C" {
#include <sys/stat.h>
}

/*

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
        return a + "/" + b;
    }

    static inline std::string testPath = "/tmp/cpl";
    static inline std::string testString = "123456789876543210";

    const static std::set<std::string> all_folders = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "1"));
        temp.insert(joinPath(testPath, "2"));
        temp.insert(joinPath(testPath, "zero0"));
        temp.insert(joinPath(testPath, "zero0/test"));
        return temp;
    }();

    const static std::set<std::string> not_exist_folders = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "999"));
        temp.insert(joinPath(testPath, "zxcb/999"));
        temp.insert(joinPath(testPath, "8"));
        return temp;
    }();

    const static std::set<std::string> existance_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "emptyFile.js"));
        temp.insert(joinPath(testPath, "2/22222.js"));
        temp.insert(joinPath(testPath, "notempty.txt"));
        temp.insert(joinPath(testPath, "notemptyx2"));
        return temp;
    }();

    const static std::set<std::string> not_existance_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "bemptyFile.js"));
        temp.insert(joinPath(testPath, "2/b22222.js"));
        temp.insert(joinPath(testPath, "bnotempty.txt"));
        temp.insert(joinPath(testPath, "bnotemptyx2"));
        return temp;
    }();

    const static std::set<std::string> empty_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "emptyFile.js"));
        temp.insert(joinPath(testPath, "2/22222.js"));
        return temp;
    }();

    const static std::set<std::string> not_empty_files = [](){
        std::set<std::string> temp;
        temp.insert(joinPath(testPath, "notempty.txt"));
        temp.insert(joinPath(testPath, "notemptyx2"));
        return temp;
    }();

    bool initializeTree() {
#ifdef __linux__
        auto p = mkdir(joinPath(testPath, "1").c_str(), 0777);
        p |= mkdir(joinPath(testPath, "2").c_str(), 0777);
        p |= mkdir(joinPath(testPath, "zero0").c_str(), 0777);
        p |= mkdir(joinPath(testPath, "zero0/test").c_str(), 0777);

        if (p)
            return false;
        
        std::ofstream f1(joinPath(testPath, "/emptyFile.js"));
        f1.close();

        std::ofstream f2(testPath + "/2/22222.js");
        f2.close();

        std::ofstream f3(testPath + "/notempty.txt");
        f3 << testString;
        f3.close();

        std::ofstream f4(testPath + "/notemptyx2");
        f4 << testString << testString;
        f4.close();

        return true;
#endif
    }

    namespace Existance{
        bool testFileExists(){
            bool ok = true;

            //Common case #1 
            int cnt1 = 0;
            for (auto& exist : existance_files){
                cnt1 += COMPARE_RESULT(Cpl::FileExists(exist), true);

                std::cout << exist << " " << Cpl::FileExists(exist) << std::endl;
            }

            ok &= cnt1 == existance_files.size();

            int cnt2 = 0;
            for (auto& not_exist : not_existance_files){
                cnt2 += COMPARE_RESULT(Cpl::FileExists(not_exist), false);
            }

            ok &= cnt2 == 0;

            //Inversion previous
            int cnt3 = 0;
            for (auto& not_exist : not_existance_files){
                cnt3 += !COMPARE_RESULT(Cpl::FileExists(not_exist), false);
            }

            ok &= cnt3 == not_existance_files.size();

            //Check folders as files #4
            int cnt4 = 0;
            for (auto& exist : all_folders){
                cnt4 += COMPARE_RESULT(Cpl::FileExists(exist), 0);
                std::cout << exist << " " << Cpl::FileExists(exist) << std::endl;
            }
            ok &= cnt4 == 0;

            //Check parent folders as files #5
            int cnt5 = 0;
            for (auto& exist : existance_files){
                cnt5 += COMPARE_RESULT(Cpl::FileExists(Cpl::DirectoryByPath(exist)), 0);
            }
            ok &= cnt5 == 0;

            //Check files via up and down path #6
            int cnt6 = 0;
            for (auto& exist : existance_files){
                auto dir = Cpl::DirectoryByPath(exist);
                auto filename = Cpl::FileNameByPath(exist);
                cnt6 += COMPARE_RESULT(Cpl::FileExists(Cpl::MakePath(dir, filename)), 1);
            }
            ok &= cnt6 == existance_files.size();


            return ok;
        }

        bool testFolderExists(){
            bool ok = true;

            int cnt1 = 0;
            //Common case #1 
            for (auto& exist : all_folders){
                cnt1 += COMPARE_RESULT(Cpl::DirectoryExists(exist), 1);
                std::cout << exist << " " << Cpl::DirectoryExists(exist) << std::endl;
            }

            ok &= cnt1 == existance_files.size();

            //Common cases #2
            int cnt2 = 0;
            for (auto& not_exist : not_exist_folders){
                cnt2 += COMPARE_RESULT(Cpl::DirectoryExists(not_exist), 0);
                std::cout << not_exist << " " << Cpl::DirectoryExists(not_exist) << std::endl;
            }

            ok &= cnt2 == 0;

            //Check parent folder of exist files #3
            int cnt3 = 0;
            for (auto& exist : existance_files){
                cnt3 += COMPARE_RESULT(Cpl::DirectoryExists(Cpl::DirectoryByPath(exist)), 1);
                std::cout << exist << " " << Cpl::DirectoryExists(Cpl::DirectoryByPath(exist)) << std::endl;
            }

            ok &= cnt3 == existance_files.size();

            //Check files as folders #4
            int cnt4 = 0;
            for (auto& exist : existance_files){
                cnt3 += COMPARE_RESULT(Cpl::DirectoryExists(exist), 0);
                std::cout << exist << " " << Cpl::DirectoryExists(exist) << std::endl;
            }

            ok &= cnt4 == 0;


            int cnt5 = 0;
            //Case if directory ends with symbol "/"
            for (auto& exist : all_folders){
                cnt5 += COMPARE_RESULT(Cpl::DirectoryExists(Cpl::MakePath(exist, "")), 1);
            }

            ok &= cnt5 == all_folders.size();

            return ok;
        }
    }

    namespace Modify{

        bool folders(){
            bool ok = true;
            static std::string folderName1 = "4";
            std::string newFolder = joinPath(testPath, folderName1);

            //Case 1
            ok &= !Cpl::DirectoryExists(newFolder);
            ok &= Cpl::CreatePath(newFolder);
            ok &= Cpl::DirectoryExists(newFolder);
            ok &= Cpl::DeleteDirectory(newFolder);
            ok &= !Cpl::DirectoryExists(newFolder);


            //Case 2
            static std::string folderName2 = "5";
            std::string newFolder2 = joinPath(newFolder, folderName2);
            // /tmp/cpl/4/5

            ok &= !Cpl::DirectoryExists(newFolder2);
            
            ok &= Cpl::CreatePath(newFolder2);
            ok &= Cpl::DirectoryExists(newFolder);
            ok &= Cpl::DirectoryExists(newFolder2);

            ok &= Cpl::DeleteDirectory(newFolder2);
            ok &= !Cpl::DirectoryExists(newFolder2);
            ok &= Cpl::DirectoryExists(newFolder);
            ok &= Cpl::DeleteDirectory(newFolder);
            ok &= !Cpl::DirectoryExists(newFolder);

            //Case 2.1
            ok &= !Cpl::DirectoryExists(newFolder);
            ok &= !Cpl::DirectoryExists(newFolder2);
            
            ok &= Cpl::CreatePath(newFolder2);
            ok &= Cpl::DirectoryExists(newFolder);
            ok &= Cpl::DirectoryExists(newFolder2);

            ok &= Cpl::DeleteDirectory(newFolder);
            ok &= !Cpl::DirectoryExists(newFolder2);
            ok &= !Cpl::DirectoryExists(newFolder);

            ok &= !Cpl::DirectoryExists(newFolder);

            return ok;
        }


        bool createFiles(){


            
        }

    }

    namespace Info{

        bool fileList(){
            bool ok = true;

            {
                auto files = Cpl::GetFileList(testPath, "", true, false, false);
                ok &= files.size() == 3;
                for (const auto& concrete_filename_path: files){
                    ok &= !concrete_filename_path.empty();
                    auto directory = Cpl::DirectoryByPath(concrete_filename_path);
                    //TODO: fix comparison
                    ok &= COMPARE_RESULT(directory == testPath, 1);
                }


                auto filesRecursive = Cpl::GetFileList(testPath, "", true, false, true);
                ok &= filesRecursive.size() == existance_files.size();

                auto dirs = Cpl::GetFileList(testPath, "", false, true, false);
                ok &= COMPARE_RESULT(dirs.size() == 3, 1);

                auto dirsRecursive = Cpl::GetFileList(testPath, "", false, true, true);
                ok &= COMPARE_RESULT(dirsRecursive.size() == 4, 1);

            }


            return ok;
        }

        bool naming(){
            bool ok = true;
            
            auto folder = Cpl::GetNameByPath(testPath);
            ok &= folder == "cpl";

            folder = Cpl::GetNameByPath(testPath + '/');
            
            ok &= folder == "cpl";
            ok &= Cpl::LastDashFix("/tmp/cpl//") == "/tmp/cpl";

            for (const auto& file : existance_files){
                auto filename = Cpl::GetNameByPath(file);
                auto filedir = Cpl::DirectoryByPath(file);

                ok &= Cpl::MakePath(filedir, filename) == file;
                ok &= Cpl::FileExists(Cpl::MakePath(filedir, filename));
                ok &= Cpl::FileExists(Cpl::MakePath(Cpl::MakePath(filedir, ""), filename));

            }
            return ok;
        }

        bool extension(){
            bool ok = true;
            //Test get extention
            //TODO: add case for full paths, folders with dots
            ok &= Cpl::ExtensionByPath("photo.jpeg") == "jpeg";
            ok &= Cpl::ExtensionByPath("photo").size() == 0;
            ok &= Cpl::ExtensionByPath("photo.").size() == 0;
            ok &= Cpl::ExtensionByPath(".b") == "b";
            ok &= Cpl::ExtensionByPath("...b") == "b";
            ok &= Cpl::ExtensionByPath("..a.b") == "b";


            ok &= Cpl::RemoveExtension("photo.jpeg") == "photo";
            ok &= Cpl::RemoveExtension("photo.") == "photo.";
            ok &= Cpl::RemoveExtension("photo") == "photo";
            ok &= Cpl::RemoveExtension("") == "";
            ok &= Cpl::RemoveExtension(".a") == "";
            ok &= Cpl::RemoveExtension("...b") == "..";
            ok &= Cpl::RemoveExtension("..a.b") == "..a";
            ok &= Cpl::RemoveExtension("..a.b....zyx") == "..a.b...";


            ok &= Cpl::ChangeExtension("photo.jpeg", "png") == "photo.png";
            ok &= Cpl::ChangeExtension("photo.jpeg", ".png") == "photo.png";
            ok &= Cpl::ChangeExtension("photo.", ".png") == "photo..png";
            ok &= Cpl::ChangeExtension("test.photo.", ".png") == "test.photo..png";
            ok &= Cpl::ChangeExtension("test.photo.", "png") == "test.photo..png";
            ok &= Cpl::ChangeExtension("photo", ".png") == "photo.png";
            ok &= Cpl::ChangeExtension("photo", "png") == "photo.png";
            ok &= Cpl::ChangeExtension("", ".png") == "";
            ok &= Cpl::ChangeExtension("", "png") == "";
            ok &= Cpl::ChangeExtension(".a", ".png") == ".png";
            ok &= Cpl::ChangeExtension(".a", "png") == ".png";

            return ok;
        }

        bool pathing(){
            bool ok = true;

            ok &= Cpl::FileNameByPath("/usr/local/photo.png") == "photo.png";
            ok &= Cpl::FileNameByPath("photo.png") == "photo.png";
            ok &= Cpl::FileNameByPath("./photo.png") == "photo.png";
            ok &= Cpl::FileNameByPath(".././ab/../photo.png") == "photo.png";
            ok &= Cpl::FileNameByPath(".././a.b/../photo.") == "photo.";
            ok &= Cpl::FileNameByPath(".././a.b/../photo") == "photo";
            ok &= Cpl::FileNameByPath(".././a.b/../.a") == ".a";

            return ok;
        }
    }

    bool DoFileTest(){
        bool ok = true;
        try {
            std::cout << "Filesystem " << Cpl::UsedFs() << std::endl;
            initializeTree();
            ok &= Existance::testFileExists();
            ok &= Existance::testFolderExists();
            ok &= Modify::folders();
            ok &= Info::fileList();
            ok &= Info::naming();
            ok &= Info::extension();
            ok &= Info::pathing();
        }
        catch(...){
            std::cerr << "File test exception";
            return false;
        }

        return ok;
    }
}