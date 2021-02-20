#ifndef FILE_H
#define FILE_H
#ifdef SERVERSYSTEM_EXPORTS
#define SSAPI __declspec(dllexport)
#else
#define SSAPI __declspec(dllimport)
#endif

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

SSAPI std::string local_path();
SSAPI std::string read_file(std::string path);
SSAPI void write_file(std::string path, std::string data);
SSAPI bool directory_exists(std::string dir);
SSAPI bool file_exists(std::string path);
SSAPI bool copy_file(std::string from, std::string to);
SSAPI bool delete_file(std::string path);
SSAPI void create_directory(std::string dir);
SSAPI std::vector<std::string> get_files(std::string dir, std::string extension = ".*");
SSAPI std::vector<std::string> get_directories(std::string dir, bool folderNameOnly = false);
#endif // FILE_H