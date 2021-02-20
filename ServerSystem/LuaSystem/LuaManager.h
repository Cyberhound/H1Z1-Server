#ifndef LUA_MANAGER_H
#define LUA_MANAGER_H
#ifdef SERVERSYSTEM_EXPORTS
#define SSAPI __declspec(dllexport)
#else
#define SSAPI __declspec(dllimport)
#endif



#include <string>


/* environments */
#include "Stream/LuaStream.h"
#include "NetSocket/LuaNetSocket.h"
#include "Debug/LuaDebug.h"


extern "C"
{
#include <ServerSystem/Lua/luaconf.h>
#include <ServerSystem/Lua/lua.h>
#include <ServerSystem/Lua/lapi.h>
#include <ServerSystem/Lua/lauxlib.h>
#include <ServerSystem/Lua/lualib.h>
#include <ServerSystem/Lua/lobject.h>
#include <ServerSystem/Lua/lstate.h>
}



enum ExecutionResult
{
	/* No valid state was used with the manager.
		see Setup() or LuaManager(lua_State *L)
	*/
	NO_STATE,
	/* No valid file was found with given file path. (ExecuteFile) */
	NO_FILE,
	
	/* Indicates an error with the lua script (unable to compile)
		see GetError() for message.
	*/
	COMPILE_ERROR,
	/* Indicates an error with the lua script (unable to run successfully)
		see GetError() for message.
	*/
	RUN_ERROR,


	/* Lua script successfully did its workings */
	SUCCESS
};


/*
Note: When reading a table inside of a table, you MUST close the table then you may continue to reading the primary table.
You must also never use Lua without closing the table.

Idea of how to loop a table:
Setup();
while (NextElement()) {}
Close();

Close is not required if the class gets deconstructed before you use Lua again.
*/
class SSAPI LuaTableReader
{
private:
	lua_State *L;
	int table_ref;
	int current_idx;

public:
	LuaTableReader();
	/* Use this for using the last table on the stack */
	LuaTableReader(lua_State *L, int idx = -1);
	~LuaTableReader();

	/* Setup the state and use last table on stack */
	void Setup(lua_State *L, int idx = -1);
	/* Free table reference and close table reader */
	void Close();


	/* Gets the current element key as a string (if string) */
	std::string GetKeyString();
	/* Gets the current element key as an int */
	int GetKeyIdx();
	/* Gets the current element key as any value */
	void *GetKeyValue();

	/* If key is set it will get the element as a string with the key.
		note: Key may only be what GetKeyValue returns. (Unless you know what you're doing)

	   Otherwise, gets the current element as a string.
	*/
	std::string GetString(void *key = nullptr);
	/* If key is set it will get the element as a bool with the key.
		note: Key may only be what GetKeyValue returns. (Unless you know what you're doing)

	   Otherwise, gets the current element as a bool.
	*/
	int GetBool(void *key = nullptr);
	/* If key is set it will get the element as a double (number) with the key.
		note: Key may only be what GetKeyValue returns. (Unless you know what you're doing)

	   Otherwise, gets the current element as a double (number).
	*/
	double GetNumber(void *key = nullptr);
	/* If key is set it will get the element as an int with the key.
		note: Key may only be what GetKeyValue returns. (Unless you know what you're doing)
	   
	   Otherwise, gets the current element as an int.
	*/
	int GetInt(void *key = nullptr);
	/* If key is set it will get the element as a table with the key.
		note: Key may only be what GetKeyValue returns. (Unless you know what you're doing)

	   Otherwise, gets the current element as a table.
	*/
	LuaTableReader GetTable(void *key = nullptr);


	/* Gets the type of the key (use LUA_T*), see lua_type */
	int GetKeyType();
	/* Gets the type of the element (use LUA_T*), see lua_type  */
	int GetType();


	/* Rolls to the next element in the table.
		If more elements, returns true
	*/
	bool NextElement();

	/* Gets the length of the table */
	int GetLength();
	
};

class SSAPI LuaManager
{
private:
	lua_State *L;
	std::string error;
	int stackCount;

public:
	LuaManager();
	/* Sets the lua state in the manager */
	LuaManager(lua_State *L);
	~LuaManager();

	/* This will setup a lua state (for this manager). Returns the lua state */
	lua_State *Setup();
	/* This will close the lua state, make it no longer usable. (Free's all memory) */
	void Close();

	
	/* This will remove the result put by if you required or have nResults > 0 in Execute* functions */
	void ClearStack() { lua_pop(L, lua_gettop(L) - stackCount); }


	/* This gives you the compiler/run-time error */
	std::string GetError() { return this->error; }
	/* This will return the state to load environments */
	lua_State *GetState() { return this->L; }


	/* Executes lua script.
		use Setup() or LuaManager(lua_State *L) first.

		See ExecutionResult for more information on script status.
	*/
	ExecutionResult Execute(std::string script, std::string scriptName = "0000", int nArgs = 0, int nResults = 0);
	/* Requires lua script, and pushes the result on the stack (singluar result by default).
		use Setup() or LuaManager(lua_State *L) first.

		See ExecutionResult for more information on script status.
	*/
	inline ExecutionResult Require(std::string script, std::string scriptName = "0000", int nResults = 1) { return Execute(script, scriptName, 0, nResults);  }
	/* Executes lua script by file (reads the file).
		use Setup() or LuaManager(lua_State *L) first.

		See ExecutionResult for more information on script status.
	*/
	ExecutionResult ExecuteFile(std::string filePath, std::string scriptName = "", int nArgs = 0, int nResults = 0);
	/* Requires lua script by file (reads the file), and pushes the result on the stack (singluar result by default).
		use Setup() or LuaManager(lua_State *L) first.

		See ExecutionResult for more information on script status.
	*/
	inline ExecutionResult RequireFile(std::string filePath, int nResults = 1) { return ExecuteFile(filePath, "", 0, nResults); }
};
#endif // LUA_MANAGER_H