//
//
// Simple program showing new /await (generators/yield) support in VS2015SP1 and
// c++ filesystem std c++ class (still in tr2 namespace, expected to be
// finalized for C++17).
//
//
// compile:
// call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
// amd64
// cl /EHs /TP /await get_older_files.cpp
// get_older_files.exe
//
#include <experimental/resumable>
#include <experimental/generator>
#include <filesystem>
#include <vector>
#include <chrono>

using namespace std::tr2::sys;

void get_older_files(const path &root, file_time_type ts, std::vector<path> &out)
{
    recursive_directory_iterator it(root);
    recursive_directory_iterator end;

    while (it != end) {
        if (is_regular_file(*it) && last_write_time(it->path()) < ts) {
            out.push_back(it->path().filename());
        }
        ++it;
    }
}

auto get_older_files_generator(const path &root, file_time_type ts)
{
    recursive_directory_iterator it(root);
    recursive_directory_iterator end;

    while (it != end) {
        if (is_regular_file(*it) && last_write_time(it->path()) < ts) {
            yield it->path().filename();
        }
        ++it;
    }
}

int main()
{
    path root("c:\\windows\\system32");
    file_time_type ft = std::chrono::system_clock::now() - std::chrono::hours(48);

    for (auto i : get_older_files_generator(root, ft)) {
        printf("%ls\n", i.c_str());
    }

    return 0;
}
