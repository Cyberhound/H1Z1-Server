#ifndef BASEENVIRONMENTPLUGIN_H
#define BASEENVIRONMENTPLUGIN_H
#ifdef SERVERSYSTEM_EXPORTS
#define SSAPI __declspec(dllexport)
#else
#define SSAPI __declspec(dllimport)
#endif



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



class SSAPI BaseLuaPlugin
{
protected:
	lua_State *L;

public:
	/* Use this class at your dipsense, this will use state 'L' for all processes. */
	BaseLuaPlugin(lua_State *L);
	~BaseLuaPlugin();

	/* returns true if library loaded successfully */
	virtual bool Load();
	/* unloads library from environment */
	virtual void Unload();
};
#endif // BASEENVIRONMENTPLUGIN_H