#ifndef __NETWORKING_TEST_H__
#define __NETWORKING_TEST_H__

#include "steam/steam_api.h"
#include <iostream>

struct NetworkingTest
{

	SteamNetworkingMessage_t *messages[5];

	NetworkingTest();
	void Test( CSteamID id );
	void Update();
	void CloseConnection();
	void CreateListenSocket();

	HSteamNetConnection connection;
	bool connectionOwner;

	
	STEAM_CALLBACK(NetworkingTest, OnMessagesSessionFailedCallback, SteamNetworkingMessagesSessionFailed_t);
	STEAM_CALLBACK(NetworkingTest, OnConnectionStatusChangedCallback, SteamNetConnectionStatusChangedCallback_t);
};

#endif
