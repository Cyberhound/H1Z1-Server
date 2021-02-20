#ifndef LUADEBUG_H
#define LUADEBUG_H
#ifdef SERVERSYSTEM_EXPORTS
#define SSAPI __declspec(dllexport)
#else
#define SSAPI __declspec(dllimport)
#endif



#include <ServerSystem/LuaSystem/BaseLuaPlugin.h>
#include <ServerSystem/LuaSystem/Stream/LuaStream.h>


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



class SSAPI LuaDebug : public BaseLuaPlugin
{
private:
public:
	using BaseLuaPlugin::BaseLuaPlugin;
	~LuaDebug() {}


	/* returns true if library loaded successfully */
	bool Load() override;
	/* unloads library from environment */
	void Unload() override;
};
#endif // LUADEBUG_H