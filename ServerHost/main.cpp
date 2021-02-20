#include "main.h"
#include <iostream>



int main(int argc, char **argv)
{
#ifdef _DEBUG
	MessageBox(NULL, "Press OK to continue program.", "Debug Await", MB_OK);
#endif
	g_LocalPath = local_path();
	SetConsoleTitle(g_ServerName.c_str());

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-headless"))
		{
			ServerSettings.RunHeadless = TRUE;
			continue;
		}
		else if (!strcmp(argv[i], "-logs"))
		{
			ServerSettings.Logs = TRUE;
			continue;
		}
		else if (!strcmp(argv[i], "-help"))
		{
			printf("These are the avaliable options:\n%s\n\n", ServerHostUsage);
			return 1;
		}
		else
		{
			printf("Unknown usage for option '%s', these are the avaliable options:\n%s\n\n", argv[i], ServerHostUsage);
			Sleep(6000);
			return 1;
		}
	}

	if (ServerSettings.RunHeadless)
		ShowWindow(GetConsoleWindow(), SW_HIDE);

	printf("[Server]: Finding servers...\n");

	/* check for servers directory */
	if (!directory_exists(g_LocalPath + "\\Servers"))
	{
		create_directory(g_LocalPath + "\\Servers");
		printf("[Server]: Unable to locate servers. Servers folder: 'Servers/*'\n\t\tMain File: Server.lua\n\n\t\tDefault Config File: ServerConfig.lua\n");
		Sleep(4000);
		return 1;
	}

	/* check for servers in the servers folder (and get all of them if exists) */
	std::vector<std::string> servers = get_directories(g_LocalPath + "\\Servers", true);
	if (servers.size() == 0)
	{
		printf("[Server]: No servers to execute. Servers folder: 'Servers/*'\n\t\tStructure: Servers/Server/*\n\t\tMain File: Server.lua\n\t\tDefault Config File: ServerConfig.lua\n");
		Sleep(6000);
		return 1;
	}

	
	printf("[Server]: %i servers found.\n", servers.size());

	/* run each server */
	for (int i = 0; i < servers.size(); i++)
	{
		/*g_LManager = LuaManager();
		g_LManager.Setup();
		LuaStream streamLib(g_LManager.GetState());
		streamLib.Load();
		LuaNetSocket socketLib(g_LManager.GetState());
		socketLib.Load();

		g_ServerEnvironments*/
		std::string folderName = servers[i];
		std::string basePath = g_LocalPath + "\\Servers\\" + folderName;
		
		if (!file_exists(basePath + "\\" + g_MainFile))
		{
			printf("[Server]: Server folder '%s' does not have a main server file. (Main File: %s)\n\n", folderName.c_str(), g_MainFile.c_str());
			continue;
		}

		printf("[LuaSystem]: Initializing for server '%s'...\n", folderName.c_str());
		
		/* Setup lua instance and append it to the instance list */
		LuaManager *lua = new LuaManager();
		lua_State *L = lua->Setup();

		/* Load libraries */
		LuaStream streamLib(L);
		LuaNetSocket socketLib(L);
		LuaDebug debugLib(L);

		if (!streamLib.Load())
			printf("[WARN][LuaSystem]: LuaStream library failed to load.\n");
		if (!socketLib.Load())
			printf("[WARN][LuaSystem]: LuaNetSocket library failed to load.\n");
		if (!debugLib.Load())
			printf("[WARN][LuaSystem]: LuaDebug library failed to load.\n");


		printf("[LuaSystem]: Running server '%s'...\n", folderName.c_str());

		/* Execute main file */
		if (lua->RequireFile(basePath + "\\" + g_MainFile) != ExecutionResult::SUCCESS)
		{
			printf("[ERROR][LuaSystem]: Execution failed for server folder '%s'. Error:\n%s\n\n", folderName.c_str(), lua->GetError().c_str());
			lua->Close();
			delete lua;
			continue;
		}

		LuaServerData serverInfo;
		if (lua_type(L, -1) == LUA_TTABLE)
		{
			/* returns table */
			LuaTableReader serverData(L);
			BYTE exit = 0;

			while (serverData.NextElement())
			{
				if (serverData.GetKeyType() != LUA_TSTRING)
				{
					printf("[ERROR][LuaSystem]: Server info expects only strings for keys, got %s instead.\n\n", lua_typename(nullptr, serverData.GetKeyType()));
					lua->Close();
					delete lua;
					exit = 1;
					break;
				}

				std::string key = serverData.GetKeyString();
				if (!key.compare("ConfigFile"))
				{
					/* path to config file */
					if (serverData.GetType() != LUA_TSTRING)
					{
						printf("[ERROR][LuaSystem]: Server info expects string for 'ConfigFile', got %s instead.\n\n", lua_typename(nullptr, serverData.GetType()));
						lua->Close();
						delete lua;
						exit = 1;
						break;
					}

					serverInfo.ConfigFile = serverData.GetString();
				}
				else
				{
					printf("[WARN][LuaSystem]: Unknown server info key: %s\n", key.c_str());
					break;
				}
			}
			serverData.Close();
			lua->ClearStack();

			if (exit)
				continue;

			if (serverInfo.ConfigFile.empty())
				serverInfo.ConfigFile = g_ConfigFile;
		}
		else
			serverInfo.ConfigFile = g_ConfigFile;


		/* Require config file */
		if (lua->RequireFile(basePath + "\\" + serverInfo.ConfigFile) != ExecutionResult::SUCCESS)
		{
			printf("[ERROR][LuaSystem]: Server config file '%s' failed to require: %s\n\n", lua->GetError().c_str());
			lua->Close();
			delete lua;
			continue;
		}

		/* Read config file */
		LuaTableReader config(L);
		BYTE exit = 0;

		while (config.NextElement())
		{
			if (config.GetKeyType() != LUA_TSTRING)
			{
				printf("[ERROR][LuaSystem]: Server config expects only strings for keys, got %s instead.\n\n", lua_typename(nullptr, config.GetKeyType()));
				lua->Close();
				delete lua;
				exit = 1;
				break;
			}

			std::string key = config.GetKeyString();
			if (!key.compare("ServerName"))
			{
				/* path to config file */
				if (config.GetType() != LUA_TSTRING)
				{
					printf("[ERROR][LuaSystem]: Server config expects string for 'ServerName', got %s instead.\n\n", lua_typename(nullptr, config.GetType()));
					lua->Close();
					delete lua;
					exit = 1;
					break;
				}

				serverInfo.ServerName = config.GetString();
				SetConsoleTitle(serverInfo.ServerName.c_str());
			}
			else if (!key.compare("Host"))
			{
				/* path to config file */
				if (config.GetType() != LUA_TSTRING)
				{
					printf("[ERROR][LuaSystem]: Server config expects string for 'Host', got %s instead.\n\n", lua_typename(nullptr, config.GetType()));
					lua->Close();
					delete lua;
					exit = 1;
					break;
				}

				serverInfo.Host = config.GetString();
			}
			else if (!key.compare("Port"))
			{
				/* path to config file */
				if (config.GetType() != LUA_TNUMBER)
				{
					printf("[ERROR][LuaSystem]: Server config expects number for 'Port', got %s instead.\n\n", lua_typename(nullptr, config.GetType()));
					lua->Close();
					delete lua;
					exit = 1;
					break;
				}

				serverInfo.Host = config.GetInt();
			}
			else
			{
				printf("[WARN][LuaSystem]: Unknown server config key: %s\n", key.c_str());
				break;
			}
		}
		config.Close();
		lua->ClearStack();

		if (exit)
			continue;

		g_Servers.push_back(serverInfo);
		g_ServerEnvironments.push_back(lua);
		printf("[LuaSystem]: Successfully ran server '%s'.\n\n", g_MainFile.c_str());
	}


	while (g_Running)
	{
		/* server processing */
		Sleep(1000);
	}
	return 0;
}