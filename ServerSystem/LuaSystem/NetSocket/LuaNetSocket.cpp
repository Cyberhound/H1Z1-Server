#include "LuaNetSocket.h"


int NetSocket_New(lua_State *L);

bool LuaNetSocket::Load()
{
	if (!L)
		return false;

	lua_newtable(L);
	{
		/* NetSocket.New */
		lua_pushcfunction(L, NetSocket_New);
		lua_setfield(L, -2, "New");
	}
	lua_setglobal(L, "NetSocket");
	return true;
}

void LuaNetSocket::Unload()
{
	lua_pushnil(L);
	lua_setglobal(L, "NetSocket");
}



#pragma region Lua Socket Handlers
/* Methods */
bool PushNetSocket(lua_State *L, NetSocket *socket);
int NetSocket_New(lua_State *L)
{
	if (lua_gettop(L) != 4)
		return luaL_error(L, "Incorrect NetSocket.New usage. Please refer to documentation.");
	luaL_checktype(L, 4, LUA_TBOOLEAN);

	std::string host = luaL_checkstring(L, 1);
	int port = luaL_checknumber(L, 2);
	int bufferSize = luaL_checknumber(L, 3);
	BOOL udp = lua_toboolean(L, 4);

	NetSocket *socket = new NetSocket(host, port, bufferSize, udp);
	if (!PushNetSocket(L, socket))
	{
		delete socket;
		return luaL_error(L, "Unable to create socket instance.");
	}
	return 1;
}


int NetSocket_SendPacket(lua_State *L)
{
	NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");

	luaL_checktype(L, 2, LUA_TLIGHTUSERDATA);
	NetReceiver *client = (NetReceiver *)lua_touserdata(L, 2);

	ByteStreamReader *reader = (ByteStreamReader *)luaL_checkudata(L, 3, "ByteStreamReader");

	if (!socket || socket == nullptr)
		return luaL_error(L, "SendPacket got invalid socket.");
	if (!client || client == nullptr)
		return luaL_error(L, "SendPacket got invalid client.");
	if (!reader || reader == nullptr)
		return luaL_error(L, "SendPacket got invalid data.");

	lua_pushboolean(L, socket->SendPacket(*client, reader->GetBuffer(), reader->GetBufferSize()));
	return 1;
}



/* NetSocket Handlers */
struct EventUserdata
{
	NetSocket *socket;
	lua_State *L;
	int funcRef;
	int socketRef;
	int handlerIndex;
};

bool PushLuaStream(lua_State *L, BYTE *buffer, int bufferSize); // from LuaStream.cpp


void LuaNetSocket_EventHandler(NetSocket *net, NetReceiver &client, BYTE *data, int data_size, void *userdata)
{
	EventUserdata *ud = (EventUserdata *)userdata;
	if (!ud || !ud->L || !ud->funcRef || !ud->socketRef)
		return;

	lua_State *L = ud->L;

	/* Make the NetSocket userdata be marked as used */
	lua_rawgeti(L, LUA_REGISTRYINDEX, ud->socketRef);
	luaC_mark(L, -1);
	lua_pop(L, 1);


	/* Get function */
	lua_rawgeti(L, LUA_REGISTRYINDEX, ud->funcRef);

	/* Push arguments */
	lua_pushlightuserdata(L, (void *)&client);
	if (!PushLuaStream(L, data, data_size))
	{
		lua_pop(L, 1);
		return;
	}

	/* Call the function */
	if (lua_pcall(L, 2, 0, 0))
	{
		/* Error? */
		if (lua_type(L, -1) != LUA_TSTRING)
		{
			printf("[ERROR][LuaNetSocket]: Unable to call event function, no further information given.");
			return;
		}

		const char *buf = lua_tostring(L, -1);
		printf("[ERROR][LuaNetSocket]: Error calling event function:\n\t%s\n\n", buf);
		lua_pop(L, 1); //remove the error
		return;
	}
}



/* Metamethods */
#pragma region NetSocket Events
int EventHandler_Disconnect(lua_State *L)
{
	EventUserdata *ud = (EventUserdata *)luaL_checkudata(L, 1, "EventUserdata");
	if (!ud || !ud->L)
		return luaL_error(L, "Unable to disconnect invalid event");

	luaL_unref(ud->L, LUA_REGISTRYINDEX, ud->funcRef);
	luaL_unref(ud->L, LUA_REGISTRYINDEX, ud->socketRef);
	if (!ud->socket || ud->socket == nullptr)
	{
		delete ud;
		return 0;
	}

	ud->socket->RemoveHandler(ud->handlerIndex);
	delete ud;
	return 0;
}


int EventConnection_Index(lua_State *L)
{
	const char *key = lua_tostring(L, 2);
	if (!strcmp(key, "Disconnect"))
	{
		lua_pushcfunction(L, EventHandler_Disconnect);
		return 1;
	}
	return 0;
}

int EventConnection_NewIndex(lua_State *L)
{
	return luaL_error(L, "This object is not modifyable.");
}


int EventConnection_GC(lua_State *L)
{
	printf("EventConnection_GC\n");
	EventUserdata *ud = *(EventUserdata **)luaL_checkudata(L, 1, "EventUserdata");
	if (!ud || ud == nullptr)
		return 0;

	luaL_unref(ud->L, LUA_REGISTRYINDEX, ud->funcRef);
	luaL_unref(ud->L, LUA_REGISTRYINDEX, ud->socketRef);
	if (!ud->socket || ud->socket == nullptr)
	{
		delete ud;
		return 0;
	}
	
	ud->socket->RemoveHandler(ud->handlerIndex);
	delete ud;
	return 0;
}


/* NetSocket.Event.Connect */
int NetSocket_Event_Connect(lua_State *L)
{
	if (lua_gettop(L) != 2)
		return luaL_error(L, "Incorrect usage of NetSocket.Event.Connect. Please refer to documentation.");

	NetSocket *socket = *(NetSocket **)lua_touserdata(L, 1);
	if (!socket || socket == nullptr)
		return luaL_error(L, "NetSocket was invalidated.");

	/* Create userdata */
	EventUserdata *ud = (EventUserdata *)lua_newuserdata(L, sizeof(EventUserdata));
	luaL_newmetatable(L, "EventUserdata");
	{
		lua_pushcfunction(L, EventConnection_Index);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, EventConnection_NewIndex);
		lua_setfield(L, -2, "__newindex");


		lua_pushcfunction(L, EventConnection_GC);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);

	ud->L = L;

	lua_pushvalue(L, 2);
	ud->funcRef = luaL_ref(L, LUA_REGISTRYINDEX);


	ud->socket = socket;

	lua_pushvalue(L, lua_upvalueindex(1));
	ud->socketRef = luaL_ref(L, LUA_REGISTRYINDEX);

	/* Add handler */
	ud->handlerIndex = socket->AddHandler(LuaNetSocket_EventHandler, (void *)ud);
	return 1;
}


int NetSocket_Event_Index(lua_State *L)
{
	const char *key = lua_tostring(L, 2);
	if (!strcmp(key, "Connect"))
	{
		lua_pushvalue(L, 1);
		lua_pushcclosure(L, NetSocket_Event_Connect, 1);
		return 1;
	}
	return 0;
}

int NetSocket_Event_NewIndex(lua_State *L)
{
	return luaL_error(L, "This object is not modifyable.");
}
#pragma endregion


int NetSocket_Index(lua_State *L)
{
	const char *key = lua_tostring(L, 2);
	if (!strcmp(key, "OnRecieve"))
	{
		lua_pushlightuserdata(L, lua_touserdata(L, 1));
		lua_newtable(L); /* metatable */
		{
			lua_pushvalue(L, 1);
			lua_pushcclosure(L, NetSocket_Event_Index, 1);
			lua_setfield(L, -2, "__index");

			lua_pushcfunction(L, NetSocket_Event_NewIndex);
			lua_setfield(L, -2, "__newindex");
		}
		lua_setmetatable(L, -2);
		return 1;
	}
	else if (!strcmp(key, "IsInitialized"))
	{
		lua_pushcfunction(L, [](lua_State *L)->int
		{
			NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");
			if (!socket || socket == nullptr)
			{
				lua_pushboolean(L, false);
				return 1;
			}
			lua_pushboolean(L, socket->isInitialized());
			return 1;
		});
		return 1;
	}
	else if (!strcmp(key, "Listen"))
	{
		lua_pushcfunction(L, [](lua_State *L)->int
		{
			NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");
			if (!socket || socket == nullptr)
				return 0;

			// start/create listener
			socket->ListenAsync();
			return 0;
		});
		return 1;
	}
	else if (!strcmp(key, "StepListener"))
	{
		lua_pushcfunction(L, [](lua_State *L)->int
		{
			NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");
			if (!socket || socket == nullptr)
				return 0;

			// step listener
			socket->Listener();

			lua_pushboolean(L, true); //if used in while loop
			return 1;
		});
		return 1;
	}
	else if (!strcmp(key, "StartListener"))
	{
		lua_pushcfunction(L, [](lua_State *L)->int
		{
			NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");
			if (!socket || socket == nullptr)
				return 0;

			// step listener
			socket->StartListener();
			return 0;
		});
		return 1;
	}
	else if (!strcmp(key, "StopListener"))
	{
		lua_pushcfunction(L, [](lua_State *L)->int
		{
			NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");
			if (!socket || socket == nullptr)
				return 0;

			// step listener
			socket->StopListener();
			return 0;
		});
		return 1;
	}

	else if (!strcmp(key, "SendPacket"))
	{
		lua_pushcfunction(L, NetSocket_SendPacket);
		return 1;
	}
	return 0;
}

int NetSocket_NewIndex(lua_State *L)
{
	return luaL_error(L, "This object is not modifyable.");
}

int NetSocket_ToString(lua_State *L)
{
	lua_pushstring(L, "NetSocket");
	return 1;
}


int NetSocket_GC(lua_State *L)
{
	printf("NetSocket_GC\n");
	NetSocket *socket = *(NetSocket **)luaL_checkudata(L, 1, "NetSocket");
	if (!socket || socket == nullptr)
		return 0;

	if (socket->isInitialized())
	{
		socket->StopListener();
		socket->RemoveHandlers();
	}

	delete socket;
	return 0;
}



/* Bridging */
bool PushNetSocket(lua_State *L, NetSocket *socket)
{
	if (!L)
		return false;

	*(NetSocket **)lua_newuserdata(L, sizeof(NetSocket *)) = socket;
	luaL_newmetatable(L, "NetSocket");
	{
		lua_pushcfunction(L, NetSocket_Index);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, NetSocket_NewIndex);
		lua_setfield(L, -2, "__newindex");

		lua_pushcfunction(L, NetSocket_ToString);
		lua_setfield(L, -2, "__tostring");


		lua_pushcfunction(L, NetSocket_GC);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
	return true;
}
#pragma endregion



bool LuaNetSocket::PushSocket(NetSocket *socket)
{
	return PushNetSocket(L, socket);
}