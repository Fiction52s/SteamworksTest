#include "steam/steam_api.h"
#include <iostream>
#include <string>
#include <vector>
#include <SFML\Graphics.hpp>
#include <fstream>
#include "Tester.h"
#include "LobbyTester.h"
#include "NetworkingTest.h"
//#include "steam/isteamremotestorage.h"

using namespace std;
using namespace sf;

#define MESSAGE_FILE_NAME "message123456.dat"

//const static int APP_ID = 1987710;


RenderWindow *window;



void OutputDebugString(const std::string &str)
{
	cout << str;
}

void Alert(const std::string &str, const std::string &detail)
{
	cout << "Message: " << str << ", Detail: " << detail;
}

struct RemoteStorageTester
{
	ISteamRemoteStorage *m_pSteamRemoteStorage;
	ISteamUser *m_pSteamUser;
	int32 m_nNumFilesInCloud;
	uint64 m_ulBytesQuota;
	uint64 m_ulAvailableBytes;

	// Greeting message
	char m_rgchGreeting[40];
	char m_rgchGreetingNext[40];

	RemoteStorageTester()
	{
		m_pSteamRemoteStorage = SteamRemoteStorage();
	}

	void GetFileStats()
	{
		m_ulBytesQuota = 0;
		m_ulAvailableBytes = 0;
		m_nNumFilesInCloud = m_pSteamRemoteStorage->GetFileCount();
		m_pSteamRemoteStorage->GetQuota(&m_ulBytesQuota, &m_ulAvailableBytes);
	}

	void Write()
	{
		string test = "hello my name is shephard";
		strcpy(m_rgchGreeting, test.c_str());
		// Note: not writing the NULL termination, so won't read it back later either.
		bool bRet = m_pSteamRemoteStorage->FileWrite(MESSAGE_FILE_NAME, m_rgchGreeting, (int)strlen(m_rgchGreeting));

		GetFileStats();

		if (!bRet)
		{
			OutputDebugString("RemoteStorage: Failed to write file!\n");
		}
	}

	void Load()
	{
		if (!m_pSteamRemoteStorage->FileExists(MESSAGE_FILE_NAME))
		{
			cout << "file does not exist" << endl;
			return;
		}


		int32 cubFile = m_pSteamRemoteStorage->GetFileSize(MESSAGE_FILE_NAME);
		if (cubFile >= sizeof(m_rgchGreeting))
		{
			// ?? too big, nuke it
			char c = 0;
			OutputDebugString("RemoteStorage: File was larger than expected. . .\n");
			m_pSteamRemoteStorage->FileWrite(MESSAGE_FILE_NAME, &c, 1);
		}
		else
		{
			int32 cubRead = m_pSteamRemoteStorage->FileRead(MESSAGE_FILE_NAME, m_rgchGreeting, sizeof(m_rgchGreeting) - 1);
			m_rgchGreeting[cubRead] = 0; // null-terminate
		}

		string blah(m_rgchGreeting);
		cout << "testing: " << blah << endl;
	}


};

//-----------------------------------------------------------------------------
// Purpose: callback hook for debug text emitted from the Steam API
//-----------------------------------------------------------------------------
extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char *pchDebugText)
{
	// if you're running in the debugger, only warnings (nSeverity >= 1) will be sent
	// if you add -debug_steamapi to the command-line, a lot of extra informational messages will also be sent
	//OutputDebugStringA(pchDebugText);
	printf(pchDebugText);

	if (nSeverity >= 1)
	{
		// place to set a breakpoint for catching API errors
		int x = 3;
		(void)x;
	}
}



void PrintFriends()
{
	int iFriendCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	if (!iFriendCount)
		return;

	for (int iFriend = 0; iFriend < iFriendCount; iFriend++)
	{
		CSteamID steamIDFriend = SteamFriends()->GetFriendByIndex(iFriend, k_EFriendFlagImmediate);
		const char *pszFriendName = SteamFriends()->GetFriendPersonaName(steamIDFriend);
		string s(pszFriendName);
		cout << s << endl;
	}
}

enum ConnectedState
{
	S_IDLE,
	S_CREATING_LOBBY,
	S_IN_LOBBY,
};

ConnectedState state;

struct ClientTest
{


	// the name of the lobby we're connected to
	CSteamID m_steamIDLobby;

	void OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure)
	{
		cout << "onlobbycreated" << endl;
		//if (m_eGameState != k_EClientCreatingLobby)
		//	return;

		// record which lobby we're in
		if (pCallback->m_eResult == k_EResultOK)
		{
			// success
			m_steamIDLobby = pCallback->m_ulSteamIDLobby;
			//m_pLobby->SetLobbySteamID(m_steamIDLobby);

			// set the name of the lobby if it's ours
			char rgchLobbyName[256];
			string name = SteamFriends()->GetPersonaName() + string("'s lobby");
			//sprintf_safe(rgchLobbyName, "%s's lobby", SteamFriends()->GetPersonaName());
			SteamMatchmaking()->SetLobbyData(m_steamIDLobby, "name", name.c_str());

			cout << "DONE CREATING LOBBY: " << name << endl;
			// mark that we're in the lobby
			//SetGameState(k_EClientInLobby);
			state = S_IN_LOBBY;
		}
		else
		{
			// failed, show error
			cout << "lobby failure" << endl;
			//SetConnectionFailureText("Failed to create lobby (lost connection to Steam back-end servers.");
			//SetGameState(k_EClientGameConnectionFailure);
			state = S_IDLE;
		}
	}
};

CCallResult<ClientTest, LobbyCreated_t> m_SteamCallResultLobbyCreated;




CCallResult<WorkshopTester, CreateItemResult_t> OnCreateItemResultCallResult;
CCallResult<WorkshopTester, SubmitItemUpdateResult_t> OnSubmitItemUpdateResultCallResult;
CCallResult<WorkshopTester, SteamUGCQueryCompleted_t> OnQueryCompletedCallResult;
CCallResult<WorkshopTester, HTTPRequestCompleted_t> onHTTPRequestCompleted_t;



WorkshopTester::WorkshopTester()
{
	uploadedID = 0;
}

void WorkshopTester::OnGameOverlayActivated(GameOverlayActivated_t *callback)
{
	if (callback->m_bActive)
		printf("Steam overlay now active\n");
	else
		printf("Steam overlay now inactive\n");
}

void WorkshopTester::OnItemUpdatesSubmitted(SubmitItemUpdateResult_t *callback)
{
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "item updated successfully" << endl;
		break;
	default:
		cout << "failed to update item: " << (int)(callback->m_eResult) << endl;
		break;
	}
}

void WorkshopTester::OnCreatedItem(CreateItemResult_t *callback, bool bIOFailure)
{
	//char rgchString[256];
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "created item successfully" << endl;
		break;
	default:
		cout << "failed to create item: " << (int)(callback->m_eResult) << endl;
		break;
	}





	if (callback->m_eResult == k_EResultOK)
	{
		uploadedID = callback->m_nPublishedFileId;

		cout << "need legal agreement? " << (int)(callback->m_bUserNeedsToAcceptWorkshopLegalAgreement) << endl;

		UGCUpdateHandle_t updateHandle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(), uploadedID);

		SteamUGC()->SetItemTitle(updateHandle, "b04");
		SteamUGC()->SetItemDescription(updateHandle, "test description 4");

		SteamUGC()->SetItemContent(updateHandle, "C:\\Users\\ficti\\Documents\\Visual Studio 2015\\Projects\\SteamworksTest\\SteamworksTest\\Resources\\b04");
		SteamUGC()->SetItemPreview(updateHandle, "C:\\Users\\ficti\\Documents\\Visual Studio 2015\\Projects\\SteamworksTest\\SteamworksTest\\Resources\\b04\\b04.png");
		SteamUGC()->SetItemVisibility(updateHandle, ERemoteStoragePublishedFileVisibility::k_ERemoteStoragePublishedFileVisibilityPublic);

		SteamAPICall_t itemUpdateStatus = SteamUGC()->SubmitItemUpdate(updateHandle, NULL);

		OnSubmitItemUpdateResultCallResult.Set(itemUpdateStatus, this, &WorkshopTester::OnItemUpdated);
	}
	//sprintf_safe(rgchString, "SteamServerConnectFailure_t: %d\n", callback->m_eResult);
}

void WorkshopTester::OnItemUpdated(SubmitItemUpdateResult_t *callback, bool bIOFailure)
{
	//char rgchString[256];
	switch (callback->m_eResult)
	{
	case k_EResultOK:
		cout << "edited item successfully" << endl;
		break;
	default:
		cout << "failed to edit item: " << (int)(callback->m_eResult) << endl;
		break;
	}
}

void WorkshopTester::OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback, bool bIOFailure)
{
	if (callback->m_bRequestSuccessful)
	{
		cout << "http query successful" << endl;
	}
	else
	{
		cout << "failed to query http" << endl;
	}
}

void WorkshopTester::HTTPTestCallback(HTTPRequestCompleted_t *callback)
{
	cout << "CALLBACK, CALLBACK" << endl;
	if (callback->m_bRequestSuccessful)
	{
		cout << "http query successful" << endl;
	}
	else
	{
		cout << "failed to query http" << endl;
	}
}

void WorkshopTester::OnQueryCompleted(SteamUGCQueryCompleted_t *callback, bool bIOFailure)
{
	//char rgchString[256];



	switch (callback->m_eResult)
	{
	case k_EResultOK:
	{
		cout << "query success. " << callback->m_unTotalMatchingResults << " items available" << endl;

		int numResultsReturned = callback->m_unNumResultsReturned;

		//

		char urlTest[1024];
		for (int i = 0; i < numResultsReturned; ++i)
		{
			SteamUGCDetails_t details;
			bool success = SteamUGC()->GetQueryUGCResult(callback->m_handle, i, &details);
			if (success && details.m_eResult == EResult::k_EResultOK)
			{
				cout << i << "- " << details.m_rgchTitle << ": " << details.m_rgchDescription << endl;

				//bool result = SteamUGC()->GetQueryUGCPreviewURL(callback->m_handle,
				//	i, urlTest, 1024);
				//if (result)
				//{
				//	HTTPRequestHandle rh = SteamHTTP()->CreateHTTPRequest(
				//		EHTTPMethod::k_EHTTPMethodGET,
				//		urlTest);

				//	if (rh == INVALID_HTTPREQUEST_HANDLE)
				//	{
				//		cout << "http request failed" << endl;
				//	}
				//	else
				//	{
				//		//SteamHTTP()->SendHTTPRequest( rh,)
				//		SteamAPICall_t call;// = NULL;
				//		bool httpResult = SteamHTTP()->SendHTTPRequest(rh, &call);
				//		if (!httpResult)
				//		{
				//			cout << "failed send" << endl;
				//		}
				//		else
				//		{
				//			cout << "send successful" << endl;
				//			//onHTTPRequestCompleted_t.Set(call, this, &WorkshopTester::OnHTTPRequestCompleted);
				//		}

				//	}
				//}






				//uint32 itemState = SteamUGC()->GetItemState(details.m_nPublishedFileId);

				//if ((itemState & k_EItemStateSubscribed))
				//{
				//	cout << "item is already subbed to" << endl;
				//}
				//else
				//{
				//	cout << "subbing to item" << endl;
				//	SteamUGC()->SubscribeItem(details.m_nPublishedFileId);
				//}

				//if (itemState & k_EItemStateDownloading)
				//{
				//	cout << "item is downloading" << endl;
				//}
				//else if (itemState & k_EItemStateInstalled)
				//{
				//	cout << "item is already installed" << endl;
				//	//uint64 fileSize;
				//	//char path[1024];
				//	//uint32 timestamp;
				//	//cout << SteamUGC()->GetItemInstallInfo(details.m_nPublishedFileId, &fileSize, path, 1024, &timestamp);

				//	//cout << path << endl;

				//	//cout << details.

				//	//cout << "details: " << details.

				//	LoadWorkshopItem(details.m_nPublishedFileId);
				//}
			}


		}


		break;
	}
	default:
		cout << "query failed" << endl;
		break;
	}

	SteamUGC()->ReleaseQueryUGCRequest(callback->m_handle);
}

//return true on success
bool WorkshopTester::LoadWorkshopItem(PublishedFileId_t workshopItemID)
{
	uint32 unItemState = SteamUGC()->GetItemState(workshopItemID);

	if (!(unItemState & k_EItemStateInstalled))
		return false;

	uint32 unTimeStamp = 0;
	uint64 unSizeOnDisk = 0;
	char szItemFolder[1024] = { 0 };

	if (!SteamUGC()->GetItemInstallInfo(workshopItemID, &unSizeOnDisk, szItemFolder, sizeof(szItemFolder), &unTimeStamp))
		return false;


	//is.open(folder

	cout << "folder: " << szItemFolder << endl;
}


void SetupWindow()
{
	window = NULL;
	/*windowWidth = 960;
	windowHeight = 540;*/
	window = new RenderWindow(sf::VideoMode(500, 500), "Breakneck");
	window->setKeyRepeatEnabled(false);

	window->setVerticalSyncEnabled(true);
	//window->setFramerateLimit(120);
}








int main()
{

	SetupWindow();

	state = S_IDLE;
	//if (SteamAPI_RestartAppIfNecessary(1987710))
	//if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
	//{
	//	// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
	//	// local Steam client and also launches this game again.

	//	// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
	//	// removed steam_appid.txt from the game depot.

	//	return EXIT_FAILURE;
	//}

	if (!SteamAPI_Init())
	{
		OutputDebugString("SteamAPI_Init() failed\n");
		Alert("Fatal Error", "Steam must be running to play this game (SteamAPI_Init() failed).\n");
		return EXIT_FAILURE;
	}

	SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

	if (!SteamUser()->BLoggedOn())
	{
		OutputDebugString("Steam user is not logged in\n");
		Alert("Fatal Error", "Steam user must be logged in to play this game (SteamUser()->BLoggedOn() returned false).\n");
		return EXIT_FAILURE;
	}

	//PrintFriends();

	/*RemoteStorageTester tester;
	tester.Load();
	tester.Write();
	tester.Load();*/

	/*ISteamUserStats *m_pSteamUserStats = SteamUserStats();
	m_pSteamUserStats->SetStat("NumGames", 23);

	bool bSuccess = m_pSteamUserStats->StoreStats();

	int test;
	m_pSteamUserStats->GetStat("NumGames", &test);*/

	//cout << "test: " << test << endl;

	// start creating the lobby

	// ask steam to create a lobby
	//ClientTest cTest;

	//SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic /* public lobby, anyone can find it */, 4);
	// set the function to call when this completes
	//m_SteamCallResultLobbyCreated.Set(hSteamAPICall, &cTest, &ClientTest::OnLobbyCreated);
	//state = S_CREATING_LOBBY;

	//SteamFriends()->SetRichPresence("status", "Creating a lobby");
	//pchSteamRichPresenceDisplay = "WaitingForMatch";


	//WorkshopTester tester;

	//cout << "num subbed items: " << SteamUGC()->GetNumSubscribedItems();
	//cout << "num subbed items: " << SteamUGC()->getn();

	//SteamAPICall_t hSteamAPICall = SteamUGC()->CreateItem(SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity);
	//OnCreateItemResultCallResult.Set(hSteamAPICall, &tester, &WorkshopTester::OnCreatedItem);// = CCallResult<CreateItemResult_t>().


	//SteamFriends()->ActivateGameOverlayToWebPage( "http://steamgames.com/" );

	//cout << "subbed items: " << SteamUGC()->GetNumSubscribedItems() << endl;

	/*auto queryHandle = SteamUGC()->CreateQueryUserUGCRequest(SteamUser()->GetSteamID().GetAccountID(), EUserUGCList::k_EUserUGCList_Published, k_EUGCMatchingUGCType_All,
	k_EUserUGCListSortOrder_CreationOrderAsc, SteamUtils()->GetAppID(), SteamUtils()->GetAppID(), 1);*/


	//NetworkingTest nt;

	//SteamNetworkingMessages()->SendMessageToUser( SteamNetworkingIdentity().)

	/*auto queryHandle = SteamUGC()->CreateQueryAllUGCRequest(EUGCQuery::k_EUGCQuery_RankedByLastUpdatedDate,
	EUGCMatchingUGCType::k_EUGCMatchingUGCType_Items, SteamUtils()->GetAppID(),
	SteamUtils()->GetAppID(), 1);
	SteamUGC()->SetMatchAnyTag(queryHandle, true);
	auto sendQuestAPICall = SteamUGC()->SendQueryUGCRequest(queryHandle);

	OnQueryCompletedCallResult.Set(sendQuestAPICall, &tester, &WorkshopTester::OnQueryCompleted);*/

	LobbyTester lobbyTester;

	CSteamID myId = SteamUser()->GetSteamID();

	//nt.CreateListenSocket();
	//nt.Test(myId);

	lobbyTester.FindLobby();
	//lobbyTester.FindLobby();
	//SteamFriends()->ActivateGameOverlay("friends");

	bool quit = false;
	Event ev;

	int frame = 0;

	while (!quit)
	{
		while (window->pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:
				quit = true;
				break;
			}
		}

		lobbyTester.Update();

		//SteamFriends()->ActivateGameOverlay("friends");

		//nt.Update();

		SteamAPI_RunCallbacks();
		window->clear(Color::Green);
		window->display();

		++frame;
	}


	//int x;
	//cin >> x;

	window->close();

	//SteamMatchmaking()->LeaveLobby(lobbyTester.m_steamIDLobby);
	//lobbyTester.nt.CloseConnection();

	//lobbyTester.nt.CloseConnection();
	//nt.CloseConnection


	delete window;

	SteamAPI_Shutdown();
}