#include "LuaStream.h"


int ByteStream_New(lua_State *L);

int ByteStreamConverter_Convert(lua_State *L);


bool LuaStream::Load()
{
	if (!L) /* really? */
		return false;

	lua_newtable(L);
	{
		lua_pushinteger(L, 0);
		lua_setfield(L, -2, "Dword");

		lua_pushinteger(L, 1);
		lua_setfield(L, -2, "Qword");

		lua_pushinteger(L, 2);
		lua_setfield(L, -2, "Word");

		lua_pushinteger(L, 3);
		lua_setfield(L, -2, "Byte");

		lua_pushinteger(L, 4);
		lua_setfield(L, -2, "String");

		//lua_pushinteger(L, 5);
		//lua_setfield(L, -2, "WString"); //wip
	}
	lua_setglobal(L, "Types");

	lua_newtable(L);
	{
		/* string size then string in bytecode */
		lua_pushnumber(L, StringInterpretMode::GIVEN_SIZE);
		lua_setfield(L, -2, "GivenSize");

		/* this should be unused, if you use this one something is retarded. */
		lua_pushnumber(L, StringInterpretMode::GIVEN_SIZE_NULL_TERM);
		lua_setfield(L, -2, "GivenSizeNullTerm");

		/* string in bytecode until it reaches a '0' byte (similar to how strlen operates) */
		lua_pushnumber(L, StringInterpretMode::UNTIL_NULL_TERM);
		lua_setfield(L, -2, "UntilNullTerm");
	}
	lua_setglobal(L, "InterpretMode");



	lua_newtable(L);
	{
		lua_pushcfunction(L, ByteStream_New);
		lua_setfield(L, -2, "New");
	}
	lua_setglobal(L, "ByteStream");


	lua_newtable(L);
	{
		lua_pushcfunction(L, ByteStreamConverter_Convert);
		lua_setfield(L, -2, "Convert");
	}
	lua_setglobal(L, "ByteStreamConverter");
	return true;
}

void LuaStream::Unload()
{
	lua_pushnil(L);
	lua_setglobal(L, "Types");

	lua_pushnil(L);
	lua_setglobal(L, "InterpretMode");

	lua_pushnil(L);
	lua_setglobal(L, "ByteStream");

	lua_pushnil(L);
	lua_setglobal(L, "ByteStreamConverter");
}



#pragma region Stream Reader Handlers
/* Methods */
int BufferReader_ReadDword(lua_State *L)
{
	ByteStreamReader *buffer = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	lua_pushinteger(L, buffer->ReadDword());
	return 1;
}

int BufferReader_ReadWord(lua_State *L)
{
	ByteStreamReader *buffer = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	lua_pushinteger(L, buffer->ReadWord());
	return 1;
}

int BufferReader_ReadByte(lua_State *L)
{
	ByteStreamReader *buffer = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	lua_pushinteger(L, buffer->ReadByte());
	return 1;
}

int BufferReader_ReadQword(lua_State *L)
{
	ByteStreamReader *buffer = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	lua_pushinteger(L, buffer->ReadQword());
	return 1;
}

int BufferReader_ReadString(lua_State *L)
{
	ByteStreamReader *buffer = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	StringInterpretMode mode = (StringInterpretMode)luaL_checkinteger(L, 2);

	std::string str = buffer->ReadString(mode);
	lua_pushlstring(L, str.c_str(), str.size());
	return 1;
}


int BufferReader_GetBufferSize(lua_State *L)
{
	ByteStreamReader *buffer = (ByteStreamReader *)luaL_checkudata(L, 1, "ByteStreamReader");
	lua_pushinteger(L, buffer->GetBufferSize());
	return 1;
}



/* Metamethods */
int BufferReader_Index(lua_State *L)
{
	const char *key = lua_tostring(L, 2);
	if (!strcmp(key, "ReadDword"))
	{
		lua_pushcfunction(L, BufferReader_ReadDword);
		return 1;
	}
	else if (!strcmp(key, "ReadWord"))
	{
		lua_pushcfunction(L, BufferReader_ReadWord);
		return 1;
	}
	else if (!strcmp(key, "ReadByte"))
	{
		lua_pushcfunction(L, BufferReader_ReadByte);
		return 1;
	}
	else if (!strcmp(key, "ReadQword"))
	{
		lua_pushcfunction(L, BufferReader_ReadQword);
		return 1;
	}
	else if (!strcmp(key, "ReadString"))
	{
		lua_pushcfunction(L, BufferReader_ReadString);
		return 1;
	}
	else if (!strcmp(key, "GetBufferSize"))
	{
		lua_pushcfunction(L, BufferReader_GetBufferSize);
		return 1;
	}
	return 0;
}

int BufferReader_NewIndex(lua_State *L)
{ return luaL_error(L, "This object is not modifyable."); }

int BufferReader_ToString(lua_State *L)
{
	lua_pushstring(L, "ByteStreamReader");
	return 1;
}



/* Bridging */
bool PushLuaStream(lua_State *L, BYTE *buffer, int bufferSize)
{
	if (!L)
		return false;

	ByteStreamReader *reader = (ByteStreamReader *)lua_newuserdata(L, sizeof(ByteStreamReader));
	reader->Initialize(buffer, bufferSize, TRUE);

	luaL_newmetatable(L, "ByteStreamReader");
	{
		lua_pushcfunction(L, BufferReader_Index);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, BufferReader_NewIndex);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, BufferReader_ToString);
		lua_setfield(L, -2, "__tostring");
	}
	lua_setmetatable(L, -2);
	return true;
}
#pragma endregion



bool LuaStream::PushBuffer(BYTE *buffer, int bufferSize)
{
	return PushLuaStream(L, buffer, bufferSize);
}



#pragma region Stream Writer Handlers
/* Methods */
int BufferWriter_WriteDword(lua_State *L)
{
	int value = luaL_checkint(L, 2);

	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	buffer->WriteDword(value);
	return 0;
}

int BufferWriter_WriteWord(lua_State *L)
{
	int value = luaL_checkint(L, 2);

	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	buffer->WriteWord(value);
	return 0;
}

int BufferWriter_WriteByte(lua_State *L)
{
	int value = luaL_checkint(L, 2);

	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	buffer->WriteByte(value);
	return 0;
}

int BufferWriter_WriteQword(lua_State *L)
{
	int value = luaL_checkint(L, 2);

	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	buffer->WriteQword(value);
	return 0;
}

int BufferWriter_WriteString(lua_State *L)
{
	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	
	std::string value = "";
	{
		size_t buflen;
		const char *buf = luaL_checklstring(L, 2, &buflen);
		value = std::string(buf, buflen);
	}
	StringInterpretMode mode = (StringInterpretMode)luaL_checkinteger(L, 3);

	buffer->WriteString(value, mode);
	return 0;
}


int BufferWriter_GetBuffer(lua_State *L)
{
	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	if (!PushLuaStream(L, buffer->GetBuffer(), buffer->GetBufferSize()))
		return 0;
	return 1;
}

int BufferWriter_GetBufferSize(lua_State *L)
{
	ByteStreamWriter *buffer = (ByteStreamWriter *)luaL_checkudata(L, 1, "ByteStreamWriter");
	lua_pushinteger(L, buffer->GetBufferSize());
	return 1;
}



/* Metamethods */
int BufferWriter_Index(lua_State *L)
{
	const char *key = lua_tostring(L, 2);
	if (!strcmp(key, "WriteDword"))
	{
		lua_pushcfunction(L, BufferWriter_WriteDword);
		return 1;
	}
	else if (!strcmp(key, "WriteWord"))
	{
		lua_pushcfunction(L, BufferWriter_WriteWord);
		return 1;
	}
	else if (!strcmp(key, "WriteByte"))
	{
		lua_pushcfunction(L, BufferWriter_WriteByte);
		return 1;
	}
	else if (!strcmp(key, "WriteQword"))
	{
		lua_pushcfunction(L, BufferWriter_WriteQword);
		return 1;
	}
	else if (!strcmp(key, "WriteString"))
	{
		lua_pushcfunction(L, BufferWriter_WriteString);
		return 1;
	}
	else if (!strcmp(key, "GetBuffer"))
	{
		lua_pushcfunction(L, BufferWriter_GetBuffer);
		return 1;
	}
	else if (!strcmp(key, "GetBufferSize"))
	{
		lua_pushcfunction(L, BufferWriter_GetBufferSize);
		return 1;
	}
	return 0;
}

int BufferWriter_NewIndex(lua_State *L)
{
	return luaL_error(L, "This object is not modifyable.");
}

int BufferWriter_ToString(lua_State *L)
{
	lua_pushstring(L, "ByteStreamWriter");
	return 1;
}
#pragma endregion


int ByteStream_New(lua_State *L)
{
	if (lua_gettop(L) > 3)
		return luaL_error(L, "Invalid use of ByteStream.New. Please refer to documentation.");

	size_t bufferSize = luaL_checkint(L, 1);
	BOOL bigEndian = luaL_opt(L, lua_toboolean, 2, 0);
	BOOL autoBuffer = luaL_opt(L, lua_toboolean, 3, 1);

	/* New writer */
	ByteStreamWriter *writer = (ByteStreamWriter *)lua_newuserdata(L, sizeof(ByteStreamWriter));
	writer->Initialize(bufferSize, bigEndian, autoBuffer);

	luaL_newmetatable(L, "ByteStreamWriter");
	{
		lua_pushcfunction(L, BufferWriter_Index);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, BufferWriter_NewIndex);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, BufferWriter_ToString);
		lua_setfield(L, -2, "__tostring");
	}
	lua_setmetatable(L, -2);
	return 1;
}


int ByteStreamConverter_Convert(lua_State *L)
{
	luaL_error(L, "not implemented yet lol fuck me");
	return 0;
}