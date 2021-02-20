#ifndef SOCKETS_H
#define SOCKETS_H
#ifdef SERVERSYSTEM_EXPORTS
#define SSAPI __declspec(dllexport)
#else
#define SSAPI __declspec(dllimport)
#endif


#include <winsock.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <vector>
#include <map>



class NetSocket;

struct SSAPI NetReceiver
{
	sockaddr sock;
	int sock_len;

	NetReceiver() :
		sock(),
		sock_len(sizeof(sockaddr_in))
	{}

	NetReceiver(sockaddr_in sock_in, int sock_in_len) :
		sock(*reinterpret_cast<sockaddr *>(&sock_in)),
		sock_len(sock_in_len)
	{}
};


typedef void(*NetSocketHandler)(NetSocket *net, NetReceiver &client, BYTE *data, int data_size, void *userdata);


class SSAPI NetSocket
{
private:
	bool initialized;
	SOCKET server;
	size_t bufferSize;

	std::vector<NetSocketHandler> socketHandlers;
	std::map<NetSocketHandler, void *> socketUserdata;


	HANDLE listenThread;

protected:


public:
	NetSocket();

	/*
	host - ip to host the server
	port - server port
	bufferSize - size of the buffer for the socket
	udp - specify use of udp or tcp
	*/
	NetSocket(std::string host, int port, size_t bufferSize, bool udp);
	~NetSocket();

	bool isInitialized() { return initialized; }


	/*
	handler - see typedef:
	typedef void(*NetSocketHandler)(SOCKET socket, sockaddr_in client_info, int client_len, BYTE *data, int data_size);

	gets called with arguments as self explanatory.
	socket - server socket (this)
	client_info - the recieving socket information
	client_len - size of "client_info"
	data - received data
	data_size - received data size

	returns handler index.
	*/
	int AddHandler(NetSocketHandler handler, void *userdata);
	/* Removes handler, only first occurance in the vector for handlers. */
	void RemoveHandler(NetSocketHandler handler);
	/* Removes handler at idx in vector of handlers */
	void RemoveHandler(int i);
	void RemoveHandlers();

	
	void Listen();
	void ListenAsync();

	void StopListener();
	void StartListener();


	/* internal function for listening for clients and such, this will step it */
	void Listener();


	/* Sends a packet to the client */
	bool SendPacket(NetReceiver to, BYTE *data, int data_size);
};
#endif // SOCKETS_H