package.path = './Servers/2016 LoginServer/Packets/?.lua;' .. package.path;

--[[ PACKET STRUCTURES ]]
local LoginReply = require("LoginReply");
local LoginRequest = require("LoginRequest");

--local ServerListRequest = require("ServerListRequest"); --not implemented at all :/ sorry meme.

--[[ PACKET OP CODES ]]
local OpCodes = {
	[0x0001] = 'LoginRequest',
	[0x0002] = 'LoginReply',

	[0x000D] = 'ServerListRequest',
};


--[[ PACKET HANDLING ]]
local Packets = {
	GetPacketId = function(self, data)
		return OpCodes[data:ReadWord()] or "Unknown";
	end,

	Deserialize = function(self, packetId, data)
		local result = {};
		local packet = {};

		if (packetId == "LoginReply") then
			packet = LoginReply;
		elseif (packetId == "LoginRequest") then
			packet = LoginRequest;
		
		elseif (packetId == "ServerListRequest") then
			packet = ServerListRequest;
		else
			error(1, "Unknown packetId, unable to deserialize.");
		end
		

		for i, v in pairs(packet) do
			local name, type = unpack(v);
			if (type == Types.Dword) then
				result[name] = data:ReadDword();
			elseif (type == Types.Qword) then
				result[name] = data:ReadQword();
			elseif (type == Types.Word) then
				result[name] = data:ReadWord();
			elseif (type == Types.Byte) then
				result[name] = data:ReadByte();
			elseif (type == Types.String) then
				result[name] = data:ReadString(InterpretMode.UntilNullTerm);
			else
				error(1, string.format("Packet '%s' Error: Unknown element type (%i) at index '%i' with name '%s'.", packetId, type, i, name));
			end
		end
		return result;
	end,
	Serialize = function(self, packetId, data)
		local buffer = ByteStream.New(0, true, true); --bufferSize, bigEndian, autoBuffer
		local packet = {};

		if (packetId == "LoginReply") then
			packet = LoginReply;
		elseif (packetId == "LoginRequest") then
			packet = LoginRequest;
		
		elseif (packetId == "ServerListRequest") then
			packet = ServerListRequest;
		else
			error(1, "Unknown packetId, unable to serialize.");
		end
		

		for i, v in pairs(packet) do
			local name, type, default = unpack(v);
			if (type == Types.Dword) then
				buffer:WriteDword(data[name] or default);
			elseif (type == Types.Qword) then
				buffer:WriteQword(data[name] or default);
			elseif (type == Types.Word) then
				buffer:WriteWord(data[name] or default);
			elseif (type == Types.Byte) then
				buffer:WriteByte(data[name] or default);
			elseif (type == Types.String) then
				buffer:WriteString(data[name] or default, InterpretMode.UntilNullTerm);
			else
				error(1, string.format("Packet '%s' Error: Unknown element type (%i) at index '%i' with name '%s'.", packetId, type, i, name));
			end
		end
		return buffer:GetBuffer();
	end,
};
return Packets;