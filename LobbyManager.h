#ifndef __LOBBY_MANAGER_H__
#define __LOBBY_MANAGER_H__

#include "steam/steam_api.h"
#include <list>
#include <string>


// an item in the list of lobbies we've found to display
struct Lobby
{
	CSteamID m_steamIDLobby;
	//char m_rgchName[256];
	std::string name;
	bool createdByMe;
	int maxMembers;
	std::list<CSteamID> memberList;
};

struct LobbyParams
{
	int maxMembers;
};

struct LobbyManager
{
	enum Action
	{
		A_IDLE,
		A_REQUEST_CREATE_LOBBY,
		A_IN_LOBBY,
		A_REQUEST_LOBBY_LIST,
		A_REQUEST_JOIN_LOBBY,
		A_ERROR,
	};

	Action action;

	// Track whether we are in the middle of a refresh or not
	bool m_bRequestingLobbies;
	std::list<Lobby> lobbyList;

	bool readyForGameStart;

	LobbyParams paramsForMakingLobby;
	Lobby currentLobby;

	CCallResult<LobbyManager, LobbyCreated_t> m_SteamCallResultLobbyCreated;
	CCallResult<LobbyManager, LobbyMatchList_t> m_SteamCallResultLobbyMatchList;
	CCallResult<LobbyManager, LobbyEnter_t> m_SteamCallResultLobbyEnter;

	LobbyManager();

	void PopulateLobbyList( CSteamID lobbyID );
	void Update();

	void TryCreatingLobby(LobbyParams &lp);
	void TryJoiningLobby();

	void OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure);
	bool IsInLobby();

	bool CurrentLobbyHasMaxMembers();
	int GetNumCurrentLobbyMembers();

	void FindLobby();
	void LeaveLobby();

	void RefreshLobbyList();
	void OnLobbyMatchListCallback(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure);
	void ProcessLobbyList();
	
	void OnLobbyEnter(LobbyEnter_t *pCallback, bool bIOFailure);
	void PrintLobbies();

	//STEAM_CALLBACK(LobbyTester, OnLobbyDataUpdatedCallback, LobbyDataUpdate_t, m_CallbackLobbyDataUpdated);
	STEAM_CALLBACK(LobbyManager, OnLobbyChatUpdateCallback, LobbyChatUpdate_t);
	STEAM_CALLBACK(LobbyManager, OnLobbyChatMessageCallback, LobbyChatMsg_t);
	STEAM_CALLBACK(LobbyManager, OnLobbyEnterCallback, LobbyEnter_t);
	STEAM_CALLBACK(LobbyManager, OnLobbyDataUpdateCallback, LobbyDataUpdate_t);


};

#endif