#include <Windows.h>
#include <iostream>
#include <string>

#include "main.h"

/* sockets for networking */
#include "sockets/sockets.h"

/* other helpful stuff */
#include "stream/stream.h"

#include "../SonyNetworking/Networking/StreamConverter.h"
#pragma comment(lib, "SonyNetworking.lib")



enum LoginPacketId
{
	LOGIN_REQUEST = 1,
	LOGIN_REPLY = 2,
	LOGOUT = 3,
	FORCE_DISCONNECT = 4,
	CHARACTER_CREATE_REQUEST = 5,
	CHARACTER_CREATE_REPLY = 6,
	CHARACTER_LOGIN_REQUEST = 7,
	CHARACTER_LOGIN_REPLY = 8,
	CHARACTER_DELETE_REQUEST = 9,
	CHARACTER_DELETE_REPLY = 10,
	CHARACTER_SELECT_INFO_REQUEST = 11,
	CHARACTER_SELECT_INFO_REPLY = 12,
	SERVER_LIST_REQUEST = 13,
	SERVER_LIST_REPLY = 14,
	SERVER_UPDATE = 15,
	TUNNEL_APP_PACKET_CLIENT_TO_SERVER = 16,
	TUNNEL_APP_PACKET_SERVER_TO_CLIENT = 17,
	CHARACTER_TRANSFER_REQUEST = 18,
	CHARACTER_TRANSFER_REPLY = 19
};

struct LOGIN_REPLY {
	DWORD sessionId;
	DWORD unknownDword1;
	byte unknownByte1;
	byte unknownByte2;
	byte unknownByte3;
	byte unknownByte4;
	byte unknownByte5;
	byte unknownByte6;
	DWORD unknownDword2;
	byte unknownByte7;
};

struct SERVER_LIST_REPLY {
	DWORD serverId;
	DWORD serverState;
	bool locked;
	std::string name;
	DWORD nameId;
	std::string description;
	DWORD descriptionId;
	DWORD reqFeatureId;
	std::string serverInfo;
	DWORD populationLevel;
	std::string populationData;
	bool allowedAccess;
};

void Hexdump(void *ptr, int buflen)
{
	unsigned char *buf = (unsigned char *)ptr;
	int i, j;
	for (i = 0; i < buflen; i += 16)
	{
		printf("%06x: ", i);
		for (j = 0; j < 16; j++)
			if (i + j < buflen)
				printf("%02x ", buf[i + j]);
			else
				printf("   ");
		printf(" ");
		for (j = 0; j < 16; j++)
			if (i + j < buflen)
				printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
		printf("\n");
	}
}

void UnencryptedHexdump(ByteStreamReader reader, int buflen)
{
	int size = buflen - 2; // -2 for opcode
	byte buf[1000000]; // creates fake buffer (probably too many bytes in array but fuck it)
	int i, j;
	for (i = 0; i < size; i += 16)
	{
		for (j = 0; j < 16; j++) {
			if (i + j < size) {
				buf[i + j] = reader.ReadByte();
			}
			else {
				buf[i + j] = 00;
			}
		}
	}
	Hexdump(buf, size); 
}

void SendPacket(NetSocket *server, NetReceiver &client, int packetType, void *packet)
{
	ByteStreamWriter writer = ByteStreamWriter(0, TRUE, TRUE); // create writer
	writer.WriteWord(packetType); //Packet Id
	bool send = true;
	switch(packetType)
	{
		case LoginPacketId::LOGIN_REPLY:
		{
			struct LOGIN_REPLY p = *(struct LOGIN_REPLY*)packet; // cast void pointer to struct
			// printf("LOGIN_REPLY unknownByte7: %u\n", p.unknownByte7); // sanity check
			writer.WriteDword(p.sessionId);
			writer.WriteDword(p.unknownDword1);
			writer.WriteByte(p.unknownByte1);
			writer.WriteByte(p.unknownByte2);
			writer.WriteByte(p.unknownByte3);
			writer.WriteByte(p.unknownByte4);
			writer.WriteByte(p.unknownByte5);
			writer.WriteByte(p.unknownByte6);
			writer.WriteDword(p.unknownDword2);
			writer.WriteByte(p.unknownByte7); 
			//writer.WriteValue(&p);
			break;
		}
		case LoginPacketId::SERVER_LIST_REPLY:
		{
			struct SERVER_LIST_REPLY p = *(struct SERVER_LIST_REPLY*)packet; // cast void pointer to struct
			writer.WriteDword(p.serverId);
			writer.WriteDword(p.serverState);
			writer.WriteByte(p.locked);
			writer.WriteString(p.name);
			writer.WriteDword(p.nameId);
			writer.WriteString(p.description);
			writer.WriteDword(p.descriptionId);
			writer.WriteDword(p.reqFeatureId);
			writer.WriteString(p.serverInfo);
			writer.WriteDword(p.populationLevel);
			writer.WriteString(p.populationData);
			writer.WriteByte(p.allowedAccess);
		}
		default:
		{
			send = false;
			printf("LoginServer::SendPacket invalid packetType\n");
			break;
		}
	}
	if (send) {
		if (server->SendPacket(client, (BYTE*)writer, writer.GetSize()))
			printf("[LoginServer]: Sent packet %u to Client.\n", (LoginPacketId)packetType);
		else
			printf("[LoginServer]: Unable to send packet %u to Client.\n", (LoginPacketId)packetType);
	}
	
}

void loginHandler(NetSocket *server, NetReceiver &client, BYTE *data, int data_size)
{
	ByteStreamReader reader(data, data_size, TRUE);
	int opcode = reader.ReadWord();
	switch ((LoginPacketId)opcode)
	{
		case LOGIN_REQUEST:
		{
			int sessionKey = reader.ReadDword();
			int sessionId = reader.ReadDword();
			int sessionLocale = reader.ReadDword();

			std::string sessionProtocol = reader.ReadString(StringInterpretMode::UNTIL_NULL_TERM);

			if (sessionProtocol != "LoginUdp_9")
			{
				printf("Protocol Mismatch for Session Request with Id: %08X\n\tsessionKey: %08X\n\tsessionLocale: %08X\n\tsessionProtocol: %s\n\n", sessionId, sessionKey, sessionLocale, sessionProtocol.c_str());
				return;
			}
			printf("[LoginServer]: Received Login Request from Client [%08X].\n", sessionId);

			struct LOGIN_REPLY packet = {sessionId, 0, 2, 1, 0, 0, 0, 2, 0, 3};
			SendPacket(server, client, LoginPacketId::LOGIN_REPLY, &packet);
			
			break;
		}
		case SERVER_LIST_REQUEST:
		{
			printf("server list request\n");
			Networking::StreamConverter converter(reader, g_LoginServerEncryptionKey, sizeof(g_LoginServerEncryptionKey));
			converter.Convert(2 /* skip packet id */);
			UnencryptedHexdump(reader, data_size);
			struct SERVER_LIST_REPLY packet = 
			{
				1, 
				2, 
				0,
				"SoloServer",
				195,
				"yeah",
				1,
				0,
				"< ServerInfo Region = \"CharacterCreate.RegionUs\" Subregion=\"UI.SubregionUS\" IsRecommended=\"1\" IsRecommendedVS=\"0\" IsRecommendedNC=\"0\" IsRecommendedTR=\"0\" />",
				3,
				"<Population ServerCapacity=\"0\" PingAddress=\"127.0.0.1:1117\" Rulesets=\"Permadeath\"><factionlist IsList=\"1\"><faction Id=\"1\" Percent=\"0\" TargetPopPct=\"0\" RewardBuff=\"52\" XPBuff=\"52\" PercentAvg=\"0\"/><faction Id=\"2\" Percent=\"0\" TargetPopPct=\"1\" RewardBuff=\"0\" XPBuff=\"0\" PercentAvg=\"0\"/><faction Id=\"3\" Percent=\"0\" TargetPopPct=\"1\" RewardBuff=\"0\" XPBuff=\"0\" PercentAvg=\"1\"/></factionlist></Population>",
				1
			};
			SendPacket(server, client, LoginPacketId::SERVER_LIST_REPLY, &packet);
			// game doesn't get past this packet, probably because of missing SOEserver shit

			// Networking::StreamConverter writerconvert(writer, g_LoginServerEncryptionKey, sizeof(g_LoginServerEncryptionKey));
			// writerconvert.Convert(2 /* skip packet id */);
		}
		default:
		{
			printf("[LoginServer]: Recieved unknown packet with id [%08X]:\n", opcode);
			Hexdump(data, data_size);
			break;
		}
	}
}


int main(int argc, char **argv)
{
	SetConsoleTitle("H1Z1 Login Server");

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-headless"))
		{
			LoginServerSettings.RunHeadless = TRUE;
			continue;
		}
		else if (!strcmp(argv[i], "-help"))
		{
			printf("These are the avaliable options:\n%s\n\n", argv[i], LoginServerUsage);
			return 1;
		}
		else
		{
			printf("Unknown usage for option '%s', these are the avaliable options:\n%s\n\n", argv[i], LoginServerUsage);
			Sleep(5000);
			return 1;
		}
	}

	if (LoginServerSettings.RunHeadless)
		ShowWindow(GetConsoleWindow(), SW_HIDE);

	NetSocket loginServer(g_LoginServerHost, g_LoginServerPort, 512, true);
	if (!loginServer.isInitialized())
	{
		printf("[LoginServer]: Failed to initialize server.\n");
		Sleep(1500);
		return 1;
	}

	loginServer.AddHandler(loginHandler);
	
	printf("[LoginServer]: Server started.\n");
	loginServer.Listen();
	printf("[LoginServer]: Server terminated.\n");
	return 0;
}