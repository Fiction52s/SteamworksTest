#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

#include "steam/steam_api.h"
#include <iostream>

struct ConnectionManager
{

	SteamNetworkingMessage_t *messages[5];

	ConnectionManager();
	void ConnectToID(CSteamID id);
	void Update();
	void CloseConnection();
	void CreateListenSocket();

	//going to just focus on 2 player for now!
	HSteamNetConnection connection;
	HSteamListenSocket listenSocket;
	bool connected;
	bool connectionOwner;


	STEAM_CALLBACK(ConnectionManager, OnMessagesSessionFailedCallback, SteamNetworkingMessagesSessionFailed_t);
	STEAM_CALLBACK(ConnectionManager, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);
};

#endif
