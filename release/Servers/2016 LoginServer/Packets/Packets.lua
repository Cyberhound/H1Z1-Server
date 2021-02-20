package.path = './Servers/2016 LoginServer/Packets/?.lua;' .. package.path;

--[[ PACKET STRUCTURES ]]
local LoginReply = require("LoginReply");
local LoginRequest = require("LoginRequest");

local ServerListRequest = require("ServerListRequest"); --not implemented at all :/ sorry meme.


-- Put the successfully required packets here, that should be proccessed in the server.
-- Order does not matter, it is looped through specifically to automatically get the packet id's.
local ProcessedPackets = {
	"LoginReply",
	"LoginRequest",

	"ServerListRequest",
};

--[[ PACKET OP CODES ]]
local OpCodes = {};
local PacketIds = {};

--[[ AUTOMATIC PACKET OPCODE RETRIEVER ]]
--[[
	I know this looks sophisticated, but it's not.
	_LOCALS doesn't exist, so I create this table.
	It basically uses debug.getlocal to get all locals of the "parent function" (this function/script)
	which contains the required packets.
	
	I use the string name for this as an index for this, so that it can fetch the packets (and the opcode).
]]
local _LOCALS = (function()
	local locals = {};
	local idx = 1;
	
	while true do
		local name, val = debug.getlocal(2, idx);
		if (name ~= nil) then
			locals[name] = val;
		else break; end

		idx = idx + 1;
	end
	return locals;
end)();

for i, v in pairs(ProcessedPackets) do
	local success, error = pcall(function()
		--[1] = PacketId element (supposedly)
		--[1][3] = PacketId default value (the opcode)
		local field = _LOCALS[v][1];
		if (field[1] ~= "PacketId") then --[[name check]] error(string.format("[ERROR][%s]: Packet structure invalid. First field is not 'PacketId' (REQURIED).", v), 1); end

		PacketIds[field[3]] = v;
		OpCodes[v] = field[3]; --OpCodes is reverse table of PacketIds
	end);

	if (not success) then --pcall acts as try{} catch{} basically.
		print(string.format("[WARN][Packets]: Unable to obtain field 'PacketId' in packet '%s'.\nError: %s", v, error));
	end
end



--[[ PACKET HANDLING ]]
local Packets = {
	-- Returns "Unknown" if the packet opcode is not valid.
	-- If valid, returns the packet id from the opcode.
	GetPacketId = function(self, data)
		return PacketIds[data:ReadWord()] or "Unknown";
	end,
	-- Returns nil if the (packetId : string) is not a valid packet id.
	-- If valid, returns the opcode of the packet id.
	GetPacketOpCode = function(self, packetId)
		return OpCodes[packetId];
	end,

	-- Deserializes a packet from a reader, and a packet id.
	-- Returns a table if successful, otherwise, errors.
	Deserialize = function(self, packetId, data)
		local result = {};
		local packet = _LOCALS[packetId];

		if (packet == nil) then
			error("Unknown packetId, unable to deserialize.", 1);
		end
		

		for i, v in pairs(packet) do
			local name, type = unpack(v);
			if (i == 1) then
				if (name ~= "PacketId") then error("Unable to deserialize, expected PacketId for first element of structure.", 1); end
				result.PacketId = OpCodes[packetId];
			else
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
					error(string.format("Packet '%s' Error: Unknown element type (%i) at index '%i' with name '%s'.", packetId, type, i, name), 1);
				end
			end
		end
		return result;
	end,
	-- Serializes a packet from a table, and a packet id.
	-- Returns a reader if successful, otherwise, errors.
	Serialize = function(self, packetId, data)
		local buffer = ByteStream.New(0, true, true); --bufferSize, bigEndian, autoBuffer
		local packet = _LOCALS[packetId];

		if (packet == nil) then
			error("Unknown packetId, unable to deserialize.", 1);
		end
		

		for i, v in pairs(packet) do
			local name, type, default = unpack(v);
			if (i == 1) then
				if (name ~= "PacketId") then error("Unable to serialize, expected PacketId for first element of structure.", 1); end
				buffer:WriteWord(OpCodes[packetId]);
			else
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
					error(string.format("Packet '%s' Error: Unknown element type (%i) at index '%i' with name '%s'.", packetId, type, i, name), 1);
				end
			end
		end
		return buffer:GetBuffer();
	end,
};
return Packets;