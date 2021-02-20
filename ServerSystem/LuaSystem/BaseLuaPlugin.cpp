#include "BaseLuaPlugin.h"



BaseLuaPlugin::BaseLuaPlugin(lua_State *L)
{
	this->L = L;
}

BaseLuaPlugin::~BaseLuaPlugin()
{
	this->L = nullptr;
}



bool BaseLuaPlugin::Load()
{
	if (!L) /* really? */
		return false;
	return true;
}

void BaseLuaPlugin::Unload()
{}