#include "sockets.h"



/* NetSocket::* */
NetSocket::NetSocket() :
	initialized(false),
	bufferSize(0)
{}

NetSocket::NetSocket(std::string host, int port, size_t _bufferSize, bool udp) :
	initialized(false),
	bufferSize(_bufferSize)
{
	if (host == "localhost")
		host = "127.0.0.1";


	WSADATA wd;
	if (WSAStartup(MAKEWORD(2, 2), &wd))
		return;

	server = socket(AF_INET, SOCK_DGRAM, udp ? IPPROTO_UDP : IPPROTO_TCP);
	if (!server || server == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}

	sockaddr_in si;
	ZeroMemory(&si, sizeof(sockaddr_in));
	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	si.sin_addr.s_addr = inet_addr(host.c_str());

	if (bind(server, (sockaddr *)&si, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		WSACleanup();
		return;
	}

	initialized = true;
}

NetSocket::~NetSocket()
{
	if (!initialized)
		return;

	DWORD code;
	GetExitCodeThread(listenThread, &code);
	if (code == STILL_ACTIVE)
		TerminateThread(listenThread, 0);
	closesocket(server);
	WSACleanup();
}



void NetSocket::AddHandler(NetSocketHandler handler)
{
	socketHandlers.push_back(handler);
}

void NetSocket::RemoveHandler(NetSocketHandler handler)
{
	for (size_t i = 0; i < socketHandlers.size(); i++)
	{
		if (socketHandlers[i] == handler)
		{
			socketHandlers.erase(socketHandlers.begin() + i);
			break;
		}
	}
}


void NetSocket::RemoveHandlers()
{
	socketHandlers.clear();
}



void NetSocket::Listener()
{
	BYTE *buffer = (BYTE *)malloc(bufferSize);
	ZeroMemory(buffer, bufferSize);

	NetReceiver client;

	while (true)
	{
		int receivedBytes = recvfrom(server, (char *)buffer, bufferSize, 0, &client.sock, &client.sock_len);
		if (receivedBytes > 0)
		{
			/* new information */
			for (size_t i = 0; i < socketHandlers.size(); i++)
				socketHandlers[i](this, client, buffer, receivedBytes);
			ZeroMemory(buffer, bufferSize);
		}
	}
}


DWORD WINAPI wrapListener(LPVOID obj)
{
	((NetSocket *)obj)->Listener();
	return 0;
}


void NetSocket::Listen()
{
	if (!initialized || !bufferSize)
		return;

	listenThread = GetCurrentThread();
	this->Listener();
}

void NetSocket::ListenAsync()
{
	if (!initialized || !bufferSize)
		return;

	listenThread = CreateThread(0, 0, wrapListener, (LPVOID)this, 0, 0);
}



void NetSocket::StopListener()
{
	if (!listenThread || listenThread == INVALID_HANDLE_VALUE)
		return;

	SuspendThread(listenThread);
}

void NetSocket::StartListener()
{
	if (!listenThread || listenThread == INVALID_HANDLE_VALUE)
		return;

	ResumeThread(listenThread);
}



bool NetSocket::SendPacket(NetReceiver to, BYTE *data, int data_size)
{
	if (sendto(server, (char *)data, data_size, 0, &to.sock, to.sock_len))
		return true;
	return false;
}