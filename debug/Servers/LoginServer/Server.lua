package.path = './Servers/LoginServer/?.lua;' .. package.path;

local PacketOpcodes = require("Packets/Opcodes");

local LoginRequest = require("Packets/LoginRequest");

LoginServer.OnReceive:Connect(function(client, data)
	local code = data:ReadWord();
	local opcode = PacketOpcodes[code];
	if (opcode == "LoginRequest") then
		-- Respond with login reply
		local Request = LoginRequest:Deserialize(data);
		print("[LoginServer]: Received LoginRequest from client " .. tostring(client))
		print("Protocol: " .. Request.SessionProtocol);
	elseif (opcode == "ServerListRequest") then
		print("Yeet");
	else
		print("[LoginServer]: Unknown packet received.");
		Debug.HexDump(data);
	end
end);


return {
	["ConfigFile"] = "config.lua",
}