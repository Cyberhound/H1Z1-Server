-- This allows for 'require' to also fetch from this directory below.
package.path = './Servers/2016 LoginServer/?.lua;' .. package.path;


--[[ PACKETS CONFIG ]]
local Packets = require("Packets/Packets");


--[[ SERVER CONFIG ]]
local ServerConfig = require("Config");

--[[ SERVER CODE ]]
local LoginServer = NetSocket.New(ServerConfig.Host, ServerConfig.Port, 0x5000, true); -- Host,Port,BufferSize,UDP

if (not LoginServer:IsInitialized()) then
	error("Unable to create LoginServer socket.");
end

LoginServer.OnRecieve:Connect(function(client, data)
	local packetId = Packets:GetPacketId(data);
	if (packetId == "LoginRequest") then
		local packet = Packets:Deserialize(packetId, data);
		
		print("[LoginServer]: Client (" .. string.format("%08X", packet.SessionId) .. ") requested permission to login.");
		print("[LoginServer]: Sending client successful reply...");

		local reply = Packets:Serialize("LoginReply", { SessionId = packet.SessionId });
		
		print("Login Reply:");
		Debug.HexDump(reply);

		if (not LoginServer:SendPacket(client, reply)) then
			print("[LoginServer]: Failed to send client login reply.");
		end
	elseif (packetId == "ServerListRequest") then
		print("Yeet");
	else
		print("[LoginServer]: Unknown/unhandled packet received: '" .. packetId .. "'. Content:");
		Debug.HexDump(data);
	end
end);


LoginServer:Listen()

-- You may specify your own config file here.
-- If nothing returned, it will result to the default file: ServerConfig.lua
return {
	ConfigFile = "Config.lua",
}