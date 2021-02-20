local ServerListRequest = {
	--[[
	[IDX] = { ELEMENT_NAME : string, ELEMENT_TYPE : Types, DEFAULT_VALUE : value }
	]]
	{ "PacketId", Types.Word, 0x000D },
};
return ServerListRequest;