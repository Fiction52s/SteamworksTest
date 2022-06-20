#include "NetworkingTest.h"

#include <iostream>
#include <string>

using namespace std;

#define NET SteamNetworkingMessages

NetworkingTest::NetworkingTest()
{
	SteamNetworkingUtils()->InitRelayNetworkAccess();

	connectionOwner = false;

	
}

void NetworkingTest::CreateListenSocket()
{
	SteamNetworkingSockets()->CreateListenSocketP2P(0, 0, NULL);
}

void NetworkingTest::OnMessagesSessionFailedCallback(SteamNetworkingMessagesSessionFailed_t *pCallback)
{
	cout << "messages session failed callback" << endl;

	SteamNetConnectionInfo_t info = pCallback->m_info;
	
}

void NetworkingTest::OnConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pCallback)
{
	//cout << "connection status changed callback" << endl;

	//cout << "old: " << pCallback->m_eOldState << ", new: " << pCallback->m_info.m_eState << endl;

	if (pCallback->m_eOldState == k_ESteamNetworkingConnectionState_None 
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting )
	{
		if (pCallback->m_info.m_hListenSocket)
		{
			EResult result = SteamNetworkingSockets()->AcceptConnection(pCallback->m_hConn);

			if (result == k_EResultOK)
			{
				cout << "accepting connection" << endl;
			}
			else
			{
				cout << "failing to accept connection" << endl;
			}
		}
		else
		{
			cout << "connecting but I'm not the one with a listen socket" << endl;
		}
	}
	else if (pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_FindingRoute)
	{
		cout << "finding route.." << endl;
	}
	else if ( (pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		|| pCallback->m_eOldState == k_ESteamNetworkingConnectionState_FindingRoute )
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
	{
		cout << "connection is complete! attempting to send a message" << endl;

		connection = pCallback->m_hConn;

		string myName = SteamFriends()->GetPersonaName();

		string test = "hello this is a message from " + myName;


		//SteamNetworkingMessages()->SendMessageToUser( connection, )

		SteamNetworkingSockets()->SendMessageToConnection(connection, test.c_str(), test.length() + 1, k_EP2PSendReliable, NULL);
	}
	else if ((pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		|| pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer)
	{
		cout << "connection closed by peer: " << pCallback->m_info.m_eEndReason << endl;

		SteamNetworkingSockets()->CloseConnection(connection, 0, NULL, false);
	}
	else if ((pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connecting
		|| pCallback->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
		&& pCallback->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
	{
		cout << "connection state problem locally detected: " << pCallback->m_info.m_eEndReason << endl;
	}
	else
	{
		//cout << "state is confused" << endl;
		cout << "confused: " << "old: " << pCallback->m_eOldState << ", new state: " << pCallback->m_info.m_eState << endl;
	}
}

void NetworkingTest::CloseConnection()
{
	SteamNetworkingSockets()->CloseConnection(connection, 0, NULL, false);
	//SteamNetworkingSockets()->closenet
}

void NetworkingTest::Update()
{
	//int numMsges = NET()->ReceiveMessagesOnChannel(0, messages, 5);
	int numMsges = SteamNetworkingSockets()->ReceiveMessagesOnConnection(connection, messages, 5);

	if (numMsges > 0)
	{
		cout << "got the message successfully" << endl;


		for (int i = 0; i < numMsges; ++i)
		{
			string s = (char*)(messages[i]->GetData());
			cout << "message " << i << ": " << s << endl;
		}
	}
}

void NetworkingTest::Test( CSteamID id )
{
	CSteamID myId = SteamUser()->GetSteamID();

	SteamNetworkingIdentity identity;
	identity.SetSteamID(id);

	//int dataSize = 1024;
	//char data[1024];

	//ESteamNetworkingConfigValue 

	connection = SteamNetworkingSockets()->ConnectP2P(identity, 0, 0, NULL);


	cout << "testing" << endl;
	//string myName = SteamFriends()->GetPersonaName();

	//string test = "hello this is a message from " + myName;//to_string(myId.GetAccountID());

	//EResult result = NET()->SendMessageToUser(identity, test.c_str(), test.length() + 1, k_nSteamNetworkingSend_Unreliable, 0);

	/*if (result == k_EResultOK)
	{
		cout << "send success" << endl;
	}
	else
	{
		cout << "send failed" << endl;
	}*/
	
	
}