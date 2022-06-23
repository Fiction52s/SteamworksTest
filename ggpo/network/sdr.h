/* -----------------------------------------------------------------------
* GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
*
* Use of this software is governed by the MIT license that can be found
* in the LICENSE file.
*/

#ifndef _SDR_H
#define _SDR_H

#include "poll.h"
#include "udp_msg.h"
#include "ggponet.h"
#include "ring_buffer.h"
#include "steam/steam_api.h"

#define MAX_UDP_ENDPOINTS     16

//static const int MAX_UDP_PACKET_SIZE = 4096;

class Sdr : public IPollSink
{
public:
	struct Stats {
		int      bytes_sent;
		int      packets_sent;
		float    kbps_sent;
	};

	struct Callbacks {
		virtual ~Callbacks() { }
		virtual void OnMsg(HSteamNetConnection p_connection, UdpMsg *msg, int len) = 0;
	};


protected:
	void Log(const char *fmt, ...);

public:
	Sdr();

	//void Init(uint16 port, HSteamNetConnection p_connection, Poll *p, Callbacks *callbacks);
	void Init(Poll *p, Callbacks *callbacks);

	void SendTo(char *buffer, int len, int flags, HSteamNetConnection p_connection);

	virtual bool OnLoopPoll(void *cookie);

	void SetListenConnection(HSteamNetConnection p_connection);

public:
	~Sdr(void);
protected:
	// Network transmission information
	HSteamNetConnection listenConnection;
	// state management
	Callbacks      *_callbacks;
	Poll           *_poll;
};

#endif
