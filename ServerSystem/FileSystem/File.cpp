#include "File.h"


std::string local_path()
{
	char buffer[MAX_PATH + 1];
	GetModuleFileName(GetModuleHandle(NULL), buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

std::string read_file(std::string path)
{
	std::ios::openmode mode = std::ios::in;
	std::stringstream buffer;

	std::ifstream file(path, mode);
	if (file.is_open())
	{
		buffer << file.rdbuf();
		file.close();
	}
	else return "";

	file.close();
	return buffer.str();
}

void write_file(std::string path, std::string data)
{
	std::ios::openmode mode = std::ios::out;

	std::ofstream file(path, mode);
	if (file.is_open())
	{
		file << data;
		file.close();
	}

	file.close();
}

bool directory_exists(std::string dir)
{
	DWORD fat = GetFileAttributes(dir.c_str());
	if (fat == INVALID_FILE_ATTRIBUTES)
		return false;
	if (fat & FILE_ATTRIBUTE_DIRECTORY)
		return true;
	return false;
}

bool file_exists(std::string path)
{
	std::fstream file;
	file.open(path.c_str(), std::ios::in);
	if (file.is_open() == true)
	{
		file.close();
		return true;
	}
	file.close();
	return false;
}

bool copy_file(std::string from, std::string to)
{
	if (!file_exists(from))
		return false;
	if (CopyFile(from.c_str(), to.c_str(), true))
		return true;
	else
		return false;
}

bool delete_file(std::string path)
{
	if (file_exists(path))
	{
		DeleteFileA((LPCSTR)path.c_str());
		if (file_exists(path.c_str()))
			return false;
		else
			return true;
	}
	else return false;
}

void create_directory(std::string dir)
{
	SECURITY_ATTRIBUTES sa;
	CreateDirectoryA(dir.c_str(), &sa);
}

std::vector<std::string> get_files(std::string dir, std::string extension)
{
	std::vector<std::string> res;
	if (directory_exists(dir))
	{
		std::string path = dir + "\\*";
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(path.c_str(), &fd);
		if (hFind == INVALID_HANDLE_VALUE)
			return (std::vector<std::string>)0;
		do
		{
			std::string file_name = fd.cFileName;
			if (file_name.substr(file_name.find_last_of("."), file_name.size()) == extension)
				res.push_back(fd.cFileName);
		} while (FindNextFile(hFind, &fd) != 0);
		FindClose(hFind);
		return res;
	}
	return (std::vector<std::string>)0;
}


std::vector<std::string> get_directories(std::string dir, bool folderNameOnly)
{
	std::vector<std::string> res;
	if (directory_exists(dir))
	{
		std::string path = dir + "\\*";
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(path.c_str(), &fd);
		do
		{
			if (fd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				if ((!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, "..")) && strlen(fd.cFileName) <= 2)
					continue; /* . and .. directories. (not really a directory) */

				if (folderNameOnly)
					res.push_back(fd.cFileName);
				else
					res.push_back(dir + "\\" + std::string(fd.cFileName));
			}
		} while (FindNextFile(hFind, &fd) != 0);
		FindClose(hFind);
		return res;
	}
	return (std::vector<std::string>)0;
}