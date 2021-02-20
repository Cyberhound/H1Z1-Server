local LoginReply = {
	--[[
	[IDX] = { ELEMENT_NAME : string, ELEMENT_TYPE : Types, DEFAULT_VALUE : value }
	]]
    { "PacketId", Types.Word, 0x0002 },
	{ "SessionId", Types.Dword, 0 },
	{ "Unk4_0", Types.Dword, 0 },
	{ "Unk1_0", Types.Byte, 2 },
	{ "Unk1_1", Types.Byte, 1 },
	{ "Unk1_2", Types.Byte, 0 },
	{ "Unk1_3", Types.Byte, 0 },
	{ "Unk1_4", Types.Byte, 0 },
	{ "Unk1_5", Types.Byte, 2 },
	{ "Unk4_1", Types.Dword, 0 },
	{ "Unk1_6", Types.Byte, 3 },
};
return LoginReply;