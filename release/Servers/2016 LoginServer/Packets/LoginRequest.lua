local LoginRequest = {
	--[[
	[IDX] = { ELEMENT_NAME : string, ELEMENT_TYPE : Types, DEFAULT_VALUE : value }
	]]
	{ "PacketId", Types.Word, 0x0001 },
	{ "SessionKey", Types.Dword, 0 },
	{ "SessionId", Types.Dword, 0 },
	{ "SessionLocale", Types.Dword, 0 },

	{ "SessionProtocol", Types.String, "" },
};
return LoginRequest;