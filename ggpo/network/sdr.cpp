/* -----------------------------------------------------------------------
* GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
*
* Use of this software is governed by the MIT license that can be found
* in the LICENSE file.
*/

#include "types.h"
#include "sdr.h"
#include "udp.h"


Sdr::Sdr() :
	listenConnection(0),
	_callbacks(NULL)
{
}

Sdr::~Sdr(void)
{
}

void Sdr::Init(Poll *poll, Callbacks *callbacks)
{
	_callbacks = callbacks;

	_poll = poll;
	_poll->RegisterLoop(this);

	Log("initializing sdr instance\n");
}

void Sdr::SendTo(char *buffer, int len, int flags, HSteamNetConnection p_connection)
{
	EResult res = SteamNetworkingSockets()->SendMessageToConnection(p_connection, buffer, len, k_EP2PSendReliable, NULL);

	if (res != k_EResultOK)
	{
		//currently this happens when the other player leaves the lobby. fix that.
		Log("unknown error in sendto (erro: %d).\n", res);
		ASSERT(FALSE && "Unknown error in sendto");
	}

	Log("sent packet length %d to %d. res: %d\n", len, p_connection, res);
}

bool Sdr::OnLoopPoll(void *cookie)
{
	if (listenConnection > 0) //make cleaner later
	{
		SteamNetworkingMessage_t *messages[1];

		for (;;)
		{
			int numMsges = SteamNetworkingSockets()->ReceiveMessagesOnConnection(listenConnection, messages, 1);

			if (numMsges == 1)
			{
				Log("recvfrom returned (len:%d from %d).\n", messages[0]->GetSize(), listenConnection);
				UdpMsg *msg = (UdpMsg *)messages[0]->GetData();
				_callbacks->OnMsg(listenConnection, msg, messages[0]->GetSize());
			}
			else
			{
				break;
			}
		}

	}
	return true;
}

void Sdr::SetListenConnection(HSteamNetConnection p_connection)
{
	listenConnection = p_connection;
}


void Sdr::Log(const char *fmt, ...)
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
