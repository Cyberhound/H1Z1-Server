#include "LuaManager.h"
#include <FileSystem/File.h>


/* -------------------------------- */
/*        LUA TABLE READER          */
/*             900                  */
/* -------------------------------- */
LuaTableReader::LuaTableReader()
{
	this->L = nullptr;
	this->table_ref = -1;
	this->current_idx = -1;
}

LuaTableReader::LuaTableReader(lua_State *L, int idx)
{
	Setup(L, idx);
}

LuaTableReader::~LuaTableReader()
{
	Close();
}



void LuaTableReader::Setup(lua_State *L, int idx)
{
	if (!L)
		throw std::exception("Unable to construct LuaTableReader with null state");

	if (lua_type(L, idx) != LUA_TTABLE)
		throw std::exception((std::string("LuaTableReader expected table, got ") + luaL_typename(L, idx)).c_str());

	this->L = L;

	lua_pushvalue(L, idx);
	this->table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	this->current_idx = -1;
}

void LuaTableReader::Close()
{
	if (L && table_ref)
	{
		if (current_idx)
			lua_pop(L, 2);

		luaL_unref(L, LUA_REGISTRYINDEX, table_ref);
		L = nullptr;
		table_ref = -1;
	}
}




std::string LuaTableReader::GetKeyString()
{
	if (lua_type(L, -2) != LUA_TSTRING)
		return "";

	size_t size;
	const char *buf = lua_tolstring(L, -2, &size);
	return std::string(buf, size);
}

int LuaTableReader::GetKeyIdx()
{
	return current_idx;
}

void *LuaTableReader::GetKeyValue()
{
	return (void *)index2adr(L, -2);
}




std::string LuaTableReader::GetString(void *key)
{
	size_t size;
	const char *buf;
	if (key)
	{
		/* get table and push key for indexing */
		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		luaA_pushobject(L, (TValue *)key);

		/* index key and check element type */
		lua_rawget(L, -2);
		if (lua_type(L, -1) != LUA_TSTRING)
			return "";
	}
	buf = lua_tolstring(L, -1, &size);
	
	/* cleanup */
	if (key)
		lua_pop(L, 2);
	return std::string(buf, size);
}

int LuaTableReader::GetBool(void *key)
{
	bool result;
	if (key)
	{
		/* get table and push key for indexing */
		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		luaA_pushobject(L, (TValue *)key);

		/* index key and check element type */
		lua_rawget(L, -2);
		if (lua_type(L, -1) != LUA_TBOOLEAN)
			return -1;
	}
	result = lua_toboolean(L, -1);

	/* cleanup */
	if (key)
		lua_pop(L, 2);
	return result;
}

double LuaTableReader::GetNumber(void *key)
{
	double result;
	if (key)
	{
		/* get table and push key for indexing */
		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		luaA_pushobject(L, (TValue *)key);

		/* index key and check element type */
		lua_rawget(L, -2);
		if (lua_type(L, -1) != LUA_TNUMBER)
			return -1;
	}
	result = lua_tonumber(L, -1);

	/* cleanup */
	if (key)
		lua_pop(L, 2);
	return result;
}

int LuaTableReader::GetInt(void *key)
{
	int result;
	if (key)
	{
		/* get table and push key for indexing */
		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		luaA_pushobject(L, (TValue *)key);

		/* index key and check element type */
		lua_rawget(L, -2);
		if (lua_type(L, -1) != LUA_TNUMBER)
			return -1;
	}
	result = lua_tointeger(L, -1);

	/* cleanup */
	if (key)
		lua_pop(L, 2);
	return result;
}

LuaTableReader LuaTableReader::GetTable(void *key)
{
	LuaTableReader result;
	if (key)
	{
		/* get table and push key for indexing */
		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		luaA_pushobject(L, (TValue *)key);

		/* index key and check element type */
		lua_rawget(L, -2);
		if (lua_type(L, -1) != LUA_TTABLE)
			return nullptr;
	}
	result.Setup(L);

	/* cleanup */
	if (key)
		lua_pop(L, 2);
	return result;
}



int LuaTableReader::GetKeyType()
{
	return lua_type(L, -2);
}

int LuaTableReader::GetType()
{
	return lua_type(L, -1);
}



bool LuaTableReader::NextElement()
{
	if (current_idx == -1)
	{
		/* prep */
		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		lua_pushnil(L); /* key */
		current_idx++;
	}
	else
	{
		lua_pop(L, 1);
		current_idx++;
	}
	return lua_next(L, -2) != 0;
}

int LuaTableReader::GetLength()
{
	/* check if the table is already prepped */
	if (current_idx > 0)
		return lua_objlen(L, -2);

	/* no, so just snag it the master way (objlen will give the length of the object)
		which for tables is the length of the table.
	*/
	lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
	int len = lua_objlen(L, -1);
	lua_pop(L, 1);
	return len;
}




/* -------------------------------- */
/*         LUA MANAGER              */
/* -------------------------------- */
LuaManager::LuaManager()
{
	error = "No error occured.";
}

LuaManager::LuaManager(lua_State *L)
{
	error = "No error occured.";
	this->L = L;
}

LuaManager::~LuaManager()
{}



lua_State *LuaManager::Setup()
{
	/* Create a lua environment (with base libs) */
	this->L = lua_open();
	luaL_openlibs(L);
	error = "No error occured.";
	return L;
}


void LuaManager::Close()
{
	if (L)
		lua_close(L);
	error = "Lua state closed.";
}



ExecutionResult LuaManager::Execute(std::string script, std::string scriptName, int nArgs, int nResults)
{
	if (!L)
		return ExecutionResult::NO_STATE;
	stackCount = lua_gettop(L);

	/* Compile the script */
	int status = luaL_loadbuffer(L, script.c_str(), script.size(), ("=LuaManager#" + scriptName).c_str());
	if (status)
	{
		if (lua_type(L, -1) != LUA_TSTRING)
		{
			error = "No information given.";
			return ExecutionResult::COMPILE_ERROR;
		}

		/* Get the error */
		size_t size;
		const char *buf = lua_tolstring(L, -1, &size);
		error = std::string(buf, size);

		/* Remove the error */
		lua_pop(L, 1);
		return ExecutionResult::COMPILE_ERROR;
	}

	/* Execute the script */
	status = lua_pcall(L, nArgs, nResults, 0);
	if (status)
	{
		if (lua_type(L, -1) != LUA_TSTRING)
		{
			error = "No information given.";
			return ExecutionResult::RUN_ERROR;
		}

		/* Get the error */
		size_t size;
		const char *buf = lua_tolstring(L, -1, &size);
		error = std::string(buf, size);

		/* Remove the error */
		lua_pop(L, 1);
		return ExecutionResult::RUN_ERROR;
	}

	return ExecutionResult::SUCCESS;
}

ExecutionResult LuaManager::ExecuteFile(std::string filePath, std::string scriptName, int nArgs, int nResults)
{
	/* Check for filePath being file name || actual file path and fix accordingly */
	if (filePath.find(":/") == std::string::npos && filePath.find(":\\") == std::string::npos)
		filePath = local_path() + "\\" + filePath;

	/* File exists? */
	if (!file_exists(filePath))
	{
		error = "File does not exist.";
		return ExecutionResult::NO_FILE;
	}

	
	/* read script file */
	std::string script = read_file(filePath);

	/* get file name of script (Script.lua -> Script) */
	std::string filename = filePath;
	size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
		filename.erase(0, last_slash_idx + 1);

	size_t period_idx = filename.rfind('.');
	if (std::string::npos != period_idx)
		filename.erase(period_idx);

	/* Execute */
	return Execute(script, scriptName == "" ? filename : scriptName, nArgs, nResults);
}