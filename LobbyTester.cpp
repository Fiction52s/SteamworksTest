#include "LobbyTester.h"
#include "NetworkingTest.h"
#include <string>
#include <iostream>

using namespace std;

LobbyTester::LobbyTester()
{
	action = A_IDLE;
	m_bRequestingLobbies = false;
}

void LobbyTester::OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure)
{
	if (action != A_CREATING_LOBBY)
		return;

	// record which lobby we're in
	if (pCallback->m_eResult == k_EResultOK)
	{
		// success
		m_steamIDLobby = pCallback->m_ulSteamIDLobby;
		//m_pLobby->SetLobbySteamID(m_steamIDLobby);

		// set the name of the lobby if it's ours
		string lobbyName = SteamFriends()->GetPersonaName();
		lobbyName +="'s lobby";

		SteamMatchmaking()->SetLobbyData(m_steamIDLobby, "name", lobbyName.c_str());

		cout << "created: " << lobbyName << " successfully." << endl;

		action = A_IN_LOBBY;

		string test = "test messageeeee";
		SteamMatchmaking()->SendLobbyChatMsg(m_steamIDLobby, test.c_str(), test.length()+1);
	}
	else
	{
		// failed, show error
		cout << "failed to create a lobby (lost connection to steam back-end servers" << endl;
		action = A_ERROR;
	}
}

void LobbyTester::CreateLobby()
{
	if (!m_SteamCallResultLobbyCreated.IsActive())
	{
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic /* public lobby, anyone can find it */, 2);
		// set the function to call when this completes
		m_SteamCallResultLobbyCreated.Set(hSteamAPICall, this, &LobbyTester::OnLobbyCreated);
		action = A_CREATING_LOBBY;
		cout << "attempting to create lobby" << endl;
	}
}

void LobbyTester::RefreshLobbyList()
{
	if (!m_bRequestingLobbies)
	{
		m_bRequestingLobbies = true;
		// request all lobbies for this game
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
		// set the function to call when this API call has completed
		m_SteamCallResultLobbyMatchList.Set(hSteamAPICall, this, &LobbyTester::OnLobbyMatchListCallback);

		action = A_REQUEST_LOBBY_LIST;
		//m_pMenu->ShowSearching();
	}
}

void LobbyTester::PrintLobbies()
{
	int index = 0;
	cout << "printing lobbies" << endl;
	for (auto it = m_ListLobbies.begin(); it != m_ListLobbies.end(); ++it)
	{
		cout << index << ": " << (*it).name << endl;
		++index;
	}
}

void LobbyTester::FindLobby()
{
	RefreshLobbyList();
}

//void LobbyTester::TryToJoinLobby()
//{
//	
//	//RefreshLobbyList();
//}

void LobbyTester::OnLobbyChatUpdateCallback(LobbyChatUpdate_t *pCallback)
{
	cout << "lobby chat update callback" << endl;

	int numMembers = SteamMatchmaking()->GetNumLobbyMembers(pCallback->m_ulSteamIDLobby);
	CSteamID currUser;
	CSteamID myId = SteamUser()->GetSteamID();

	for (int i = 0; i < numMembers; ++i)
	{
		currUser = SteamMatchmaking()->GetLobbyMemberByIndex(pCallback->m_ulSteamIDLobby, i);

		if (currUser == myId)
		{
			//nt.Test(currUser);
			//break;
			continue;
		}

		//test if im the owner
		nt.Test(currUser);
		//SteamFriends()->GetPersonaName()
	}
}

void LobbyTester::OnLobbyChatMessageCallback(LobbyChatMsg_t *pCallback)
{
	cout << "lobby chat message callback" << endl;

	void *pvData[1024 * 4];
	int bufSize = 4 * 1024;

	SteamMatchmaking()->GetLobbyChatEntry(pCallback->m_ulSteamIDLobby, pCallback->m_iChatID, NULL, pvData, bufSize, NULL);

	cout << "message: " << (char*)pvData << endl;
}

void LobbyTester::OnLobbyEnterCallback(LobbyEnter_t *pCallback)
{
	cout << "lobby enter callback" << endl;
}

void LobbyTester::OnLobbyDataUpdateCallback(LobbyDataUpdate_t *pCallback)
{
	cout << "lobby data update callback" << endl;
}

void LobbyTester::Update()
{
	switch (action)
	{
	case A_CREATING_LOBBY:
		break;
	case A_IN_LOBBY:
		break;
	case A_FIND_LOBBY:
		break;
	case A_ERROR:
		break;
	}

	
	nt.Update();
}

void LobbyTester::OnLobbyEnter(LobbyEnter_t *pCallback, bool bIOFailure)
{
	if (action != A_FIND_LOBBY)
		return;

	if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess)
	{
		cout << "failed to enter lobby" << endl;
		// failed, show error
		//SetConnectionFailureText("Failed to enter lobby");
		//SetGameState(k_EClientGameConnectionFailure);
		return;
	}

	// success

	// move forward the state
	m_steamIDLobby = pCallback->m_ulSteamIDLobby;

	
	for (auto it = m_ListLobbies.begin(); it != m_ListLobbies.end(); ++it)
	{
		if ((*it).m_steamIDLobby == m_steamIDLobby)
		{
			cout << "joined lobby: " << (*it).name << " successfully" << endl;
			nt.CreateListenSocket();
			break;
		}
	}

	action = A_IN_LOBBY;
	//m_pLobby->SetLobbySteamID(m_steamIDLobby);
	//SetGameState(k_EClientInLobby);
}

void LobbyTester::OnLobbyMatchListCallback(LobbyMatchList_t *pCallback, bool bIOFailure)
{
	m_ListLobbies.clear();
	m_bRequestingLobbies = false;

	if (bIOFailure)
	{
		// we had a Steam I/O failure - we probably timed out talking to the Steam back-end servers
		// doesn't matter in this case, we can just act if no lobbies were received
	}


	if (pCallback->m_nLobbiesMatching == 0)
	{
		cout << "no lobbies found!" << endl;
		CreateLobby();
		return;
	}

	cout << "retrieved lobby list: " << endl;

	action = A_FIND_LOBBY;
	// lobbies are returned in order of closeness to the user, so add them to the list in that order
	for (uint32 iLobby = 0; iLobby < pCallback->m_nLobbiesMatching; iLobby++)
	{
		CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(iLobby);

		// add the lobby to the list
		Lobby lobby;
		lobby.m_steamIDLobby = steamIDLobby;
		// pull the name from the lobby metadata
		const char *pchLobbyName = SteamMatchmaking()->GetLobbyData(steamIDLobby, "name");
		if (pchLobbyName && pchLobbyName[0])
		{
			lobby.name = pchLobbyName;
		}
		else
		{
			// we don't have info about the lobby yet, request it
			SteamMatchmaking()->RequestLobbyData(steamIDLobby);

			lobby.name = "Lobby " + to_string(steamIDLobby.GetAccountID());
			// results will be returned via LobbyDataUpdate_t callback
		}

		m_ListLobbies.push_back(lobby);
	}

	PrintLobbies();

	if (m_ListLobbies.size() > 0)
	{
		cout << "found a lobby. Attempting to join: " << m_ListLobbies.front().name << endl;
		auto apiCall = SteamMatchmaking()->JoinLobby(m_ListLobbies.front().m_steamIDLobby);
		m_SteamCallResultLobbyEnter.Set(apiCall, this, &LobbyTester::OnLobbyEnter);
	}

	//m_pMenu->Rebuild(m_ListLobbies);
}