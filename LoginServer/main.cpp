#include <Windows.h>
#include <iostream>
#include <string>

#include "main.h"

/* sockets for networking */
#include <sockets/sockets.h>

/* other helpful stuff */
#include <stream/stream.h>

#include <SonyNetworking/Networking/StreamConverter.h>
#pragma comment(lib, "SonyNetworking.lib")



enum LoginPacketId
{
	LOGIN_REQUEST = 1,
	LOGIN_REPLY = 2,

	SERVER_LIST_REQUEST = 13
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


void loginHandler(NetSocket *server, NetReceiver &client, BYTE *data, int data_size)
{
	ByteStreamReader reader(data, data_size, TRUE);
	switch ((LoginPacketId)reader.ReadWord())
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

			ByteStreamWriter writer = ByteStreamWriter(0, TRUE, TRUE);
			writer.WriteWord(LoginPacketId::LOGIN_REPLY); //Packet Id
			writer.WriteDword(sessionId);
			writer.WriteDword(0);
			writer.WriteByte(2);
			writer.WriteByte(1);
			writer.WriteByte(0);
			writer.WriteByte(0);
			writer.WriteByte(0);
			writer.WriteByte(2);
			writer.WriteDword(0);
			writer.WriteByte(3);

			if (server->SendPacket(client, (BYTE *)writer, writer.GetSize()))
				printf("[LoginServer]: Sent login response to Client with Id [%08X].\n", sessionId);
			else
				printf("[LoginServer]: Unable to send login response to Client with Id [%08X].\n", sessionId);
			break;
		}
		case SERVER_LIST_REQUEST:
		{
			//Networking::StreamConverter converter(reader, g_LoginServerEncryptionKey, sizeof(g_LoginServerEncryptionKey));
			//converter.Convert(2 /* skip packet id */);
		}
		default:
		{
			printf("[LoginServer]: Recieved unknown packet:\n");
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