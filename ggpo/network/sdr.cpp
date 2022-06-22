/* -----------------------------------------------------------------------
* GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
*
* Use of this software is governed by the MIT license that can be found
* in the LICENSE file.
*/

#include "types.h"
#include "sdr.h"
#include "udp.h"

//SOCKET
//CreateSocket(uint16 bind_port, int retries)
//{
//	SOCKET s;
//	sockaddr_in sin;
//	uint16 port;
//	int optval = 1;
//
//	s = socket(AF_INET, SOCK_DGRAM, 0);
//	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof optval);
//	setsockopt(s, SOL_SOCKET, SO_DONTLINGER, (const char *)&optval, sizeof optval);
//
//	// non-blocking...
//	u_long iMode = 1;
//	ioctlsocket(s, FIONBIO, &iMode);
//
//	sin.sin_family = AF_INET;
//	sin.sin_addr.s_addr = htonl(INADDR_ANY);
//	for (port = bind_port; port <= bind_port + retries; port++) {
//		sin.sin_port = htons(port);
//		if (bind(s, (sockaddr *)&sin, sizeof sin) != SOCKET_ERROR) {
//			Log("Udp bound to port: %d.\n", port);
//			return s;
//		}
//	}
//	closesocket(s);
//	return INVALID_SOCKET;
//}

Sdr::Sdr() :
	connection(0),
	_callbacks(NULL)
{
}

Sdr::~Sdr(void)
{
	//CloseConnection()?

	/*if (_socket != INVALID_SOCKET) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}*/
}

void
Sdr::Init(uint16 port, Poll *poll, Callbacks *callbacks)
{
	_callbacks = callbacks;

	_poll = poll;
	_poll->RegisterLoop(this);

	Log("binding udp socket to port %d.\n", port);
	//_socket = CreateSocket(port, 0);
}



//replace SendTo with SteamNetworkingSockets()->SendMessageToConnection()
//instead of sending to an IP address, send to our already established connection
void
Sdr::SendTo(char *buffer, int len, int flags, HSteamNetConnection p_connection)
{
	EResult res = SteamNetworkingSockets()->SendMessageToConnection(p_connection, buffer, len, k_EP2PSendReliable, NULL);

	if (res != k_EResultOK)
	{
		Log("unknown error in sendto (erro: %d).\n", res);
		ASSERT(FALSE && "Unknown error in sendto");
	}

	char dst_ip[1024];
	Log("sent packet length %d to %d. res: %d\n", len, p_connection, res);
}



//replace recvfrom with SteamNetworkingSockets()->ReceiveMessagesOnConnection()
bool
Sdr::OnLoopPoll(void *cookie)
{
	uint8          recv_buf[MAX_UDP_PACKET_SIZE];
	SteamNetworkingMessage_t *messages[1];

	for (;;) {
		int numMsges = SteamNetworkingSockets()->ReceiveMessagesOnConnection(connection, messages, 1);

		// TODO: handle len == 0... indicates a disconnect.

		if (numMsges == 1)
		{
			Log("recvfrom returned (len:%d from %d).\n", messages[0]->GetSize(), connection);
			UdpMsg *msg = (UdpMsg *)recv_buf;
			_callbacks->OnMsg(connection, msg, messages[0]->GetSize());
		}
	}
	return true;
}


void
Sdr::Log(const char *fmt, ...)
{
	char buf[1024];
	size_t offset;
	va_list args;

	strcpy_s(buf, "udp | ");
	offset = strlen(buf);
	va_start(args, fmt);
	vsnprintf(buf + offset, ARRAY_SIZE(buf) - offset - 1, fmt, args);
	buf[ARRAY_SIZE(buf) - 1] = '\0';
	::Log(buf);
	va_end(args);
}
