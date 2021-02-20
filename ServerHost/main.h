#ifndef MAIN_H
#define MAIN_H
#include <Windows.h>
#include <string>
#include <vector>

/* server management */
#include <ServerSystem/Sockets/Sockets.h>
#include <ServerSystem/FileSystem/File.h>
#include <ServerSystem/LuaSystem/LuaManager.h>
#pragma comment(lib, "ServerSystem.lib")

/* networking helper */
#include <SonyNetworking/Networking/StreamConverter.h>
#pragma comment(lib, "SonyNetworking.lib")


static const char *ServerHostUsage = R"(
-help		displays a list of options (this)
-headless		runs the LoginServer in headless mode, where it doesn't contain
			a Console. Use -logs for further information.

-logs		Dumps the output of the server to Console.log)";


std::string g_ConfigFile = "Config.lua";
std::string g_MainFile = "Server.lua";

std::string g_ServerName = "H1Z1 Server";


//							17    BD    08    6B    1B    94    F0    2F    F0    EC    53    D7    63    58    9B    5F
BYTE g_ServerEncryptionKey[] = { 0x17, 0xBD, 0x08, 0x6b, 0x1B, 0x94, 0xF0, 0x2F, 0xF0, 0xEC, 0x53, 0xD7, 0x63, 0x58, 0x9B, 0x5F };



struct LuaServerData
{
	/* Information returned by main server file or default */
	std::string ConfigFile;

	/* Server configuration */
	std::string ServerName;
	std::string Host;
	int Port;


	LuaServerData() {}
	LuaServerData(std::string configFile, std::string serverName, std::string host, int port)
	{
		this->ConfigFile = configFile;
		this->ServerName = serverName;

		this->Host = host;
		this->Port = port;
	}

	~LuaServerData() {}
};


std::vector<LuaManager *> g_ServerEnvironments;
std::vector<LuaServerData> g_Servers;

std::string g_LocalPath = "";

bool g_Running = true;

struct
{
	/*
	Result to logs instead of the Console.
	*/
	BOOL RunHeadless;
	BOOL Logs;
} ServerSettings;
#endif // MAIN_H