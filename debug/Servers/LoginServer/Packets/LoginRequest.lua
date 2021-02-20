local LoginRequest = {
	--[[
	[IDX] = { ELEMENT_NAME : string, ELEMENT_TYPE : Types }
	]]
	{ "SessionKey", Types.Dword },
	{ "SessionId", Types.Dword },
	{ "SessionLocale", Types.Dword },

	{ "SessionProtocol", Types.String },
};


local function LoginRequest_Deserialize(self, data)
	local Request = {};
	for i in next, self do
		local element = self[i];
		
		local name = element[1];
		local type = element[2];

		if (type == Types.Dword) then
			Request[name] = data:ReadDword();
		elseif (type == Types.String) then
			local str = data:ReadString(InterpretMode.UntilNullTerm);
			Request[name] = str;
		elseif (type == Types.Byte) then
			Request[name] = data:ReadByte();
		elseif (type == Types.Word) then
			Request[name] = data:ReadWord();
		elseif (type == Types.Qword) then
			Request[name] = data:ReadQword();
		end
	end
	return Request;
end

setmetatable(LoginRequest, {
	__index = function(self, key)
		if (key == "Deserialize") then
			return LoginRequest_Deserialize;
		--elseif (key == "Serialize") then
		--	return LoginRequest_Serialize -- unused
		end
		return rawget(self, key);
	end
});
return LoginRequest;