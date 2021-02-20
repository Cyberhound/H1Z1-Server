#ifndef LUASTREAM_H
#define LUASTREAM_H
#ifdef SERVERSYSTEM_EXPORTS
#define SSAPI __declspec(dllexport)
#else
#define SSAPI __declspec(dllimport)
#endif



#include <ServerSystem/Stream/Stream.h>
#include <ServerSystem/LuaSystem/BaseLuaPlugin.h>


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



class SSAPI LuaStream : public BaseLuaPlugin
{
private:
public:
	using BaseLuaPlugin::BaseLuaPlugin;
	~LuaStream() {}


	/* returns true if library loaded successfully */
	bool Load() override;
	/* unloads library from environment */
	void Unload() override;


	bool PushBuffer(BYTE *buffer, int bufferSize);
};
#endif // LUASTREAM_H