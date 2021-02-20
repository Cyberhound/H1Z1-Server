#include "LuaDebug.h"


int Debug_HexDump(lua_State *L);

bool LuaDebug::Load()
{
	if (!L)
		return false;

	lua_newtable(L);
	{
		/* NetSocket.New */
		lua_pushcfunction(L, Debug_HexDump);
		lua_setfield(L, -2, "HexDump");
	}
	lua_setglobal(L, "Debug");
	return true;
}

void LuaDebug::Unload()
{
	lua_pushnil(L);
	lua_setglobal(L, "Debug");
}



int Debug_HexDump(lua_State *L)
{
	if (lua_gettop(L) != 1)
		return luaL_error(L, "Invalid use of Debug.HexDump. Please refer to documentation.");

	ByteStreamReader *reader = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	BYTE *buf = reader->GetBuffer();
	int buflen = reader->GetBufferSize();

	//buf, buflen
	int i, j;
	for (i = 0; i < buflen; i += 16)
	{
		printf("%06x: ", i);
		for (j = 0; j < 16; j++)
			if (i + j < buflen)
				printf("%02x ", buf[i + j]);
			else
				printf("   ");
		printf(" ");
		for (j = 0; j < 16; j++)
			if (i + j < buflen)
				printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
		printf("\n");
	}
}