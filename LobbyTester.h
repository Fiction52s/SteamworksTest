#ifndef __LOBBY_TESTER_H__
#define __LOBBY_TESTER_H__

#include "steam/steam_api.h"
#include <list>
#include <string>
#include "LobbyManager.h"


#include "NetworkingTest.h"

struct LobbyTester
{
	enum Action
	{
		A_IDLE,
		A_CREATING_LOBBY,
		A_IN_LOBBY,
		A_REQUEST_LOBBY_LIST,
		A_FIND_LOBBY,
		A_ERROR,
	};

	Action action;
	CSteamID m_steamIDLobby;

	CCallResult<LobbyTester, LobbyCreated_t> m_SteamCallResultLobbyCreated;
	CCallResult<LobbyTester, LobbyMatchList_t> m_SteamCallResultLobbyMatchList;
	CCallResult<LobbyTester, LobbyEnter_t> m_SteamCallResultLobbyEnter;

	
	// Track whether we are in the middle of a refresh or not
	bool m_bRequestingLobbies;
	std::list< Lobby > m_ListLobbies;
	NetworkingTest nt;

	LobbyTester();

	//void ChooseLobby();
	//void TryToJoinLobby();
	void Update();

	void CreateLobby();
	void FindLobby();
	void OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure);

	void RefreshLobbyList();
	void OnLobbyMatchListCallback(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure);
	void OnLobbyEnter(LobbyEnter_t *pCallback, bool bIOFailure);
	void PrintLobbies();

	//STEAM_CALLBACK(LobbyTester, OnLobbyDataUpdatedCallback, LobbyDataUpdate_t, m_CallbackLobbyDataUpdated);
	STEAM_CALLBACK(LobbyTester, OnLobbyChatUpdateCallback, LobbyChatUpdate_t);
	STEAM_CALLBACK(LobbyTester, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(LobbyTester, OnLobbyEnterCallback, LobbyEnter_t);
	STEAM_CALLBACK(LobbyTester, OnLobbyDataUpdateCallback, LobbyDataUpdate_t);


};

#endif