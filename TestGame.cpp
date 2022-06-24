#include "TestGame.h"
#include <iostream>
#include <assert.h>
#include <winsock.h>
#include <fstream>

using namespace std;
using namespace sf;

TestGame * TestGame::currInstance = NULL;


int fletcher32_checksum(short *data, size_t len)
{
	int sum1 = 0xffff, sum2 = 0xffff;

	while (len) {
		size_t tlen = len > 360 ? 360 : len;
		len -= tlen;
		do {
			sum1 += *data++;
			sum2 += sum1;
		} while (--tlen);
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	return sum2 << 16 | sum1;
}

/*
* vw_begin_game_callback --
*
* The begin game callback.  We don't need to do anything special here,
* so just return true.
*/
bool __cdecl
begin_game_callback(const char *)
{
	return true;
}


/*
* vw_on_event_callback --
*
* Notification from GGPO that something has happened.  Update the status
* text at the bottom of the screen to notify the user.
*/
bool __cdecl
on_event_callback(GGPOEvent *info)
{

	TestGame *tg = TestGame::GetInstance();
	//Session *sess = Session::GetSession();
	int progress;
	switch (info->code) {
	case GGPO_EVENTCODE_CONNECTED_TO_PEER:
		cout << "connected to peer" << endl;
		tg->ngs.SetConnectState(info->u.connected.player, Synchronizing);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
		cout << "synchronizing to peer" << endl;
		progress = 100 * info->u.synchronizing.count / info->u.synchronizing.total;
		tg->ngs.UpdateConnectProgress(info->u.synchronizing.player, progress);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
		cout << "synchronized with peer" << endl;
		tg->ngs.UpdateConnectProgress(info->u.synchronized.player, 100);
		break;
	case GGPO_EVENTCODE_RUNNING:
		cout << "running" << endl;
		tg->ngs.SetConnectState(Running);
		//renderer->SetStatusText("");
		break;
	case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
		cout << "connection interrupted" << endl;
		tg->ngs.SetDisconnectTimeout(info->u.connection_interrupted.player,
			0,//timeGetTime(),
			info->u.connection_interrupted.disconnect_timeout);
		break;
	case GGPO_EVENTCODE_CONNECTION_RESUMED:
		cout << "connection resumed" << endl;
		tg->ngs.SetConnectState(info->u.connection_resumed.player, Running);
		break;
	case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
		cout << "disconnected from peer" << endl;
		tg->ngs.SetConnectState(info->u.disconnected.player, Disconnected);
		break;
	case GGPO_EVENTCODE_TIMESYNC:
		cout << "timesync" << endl;
		//Sleep(1000 * info->u.timesync.frames_ahead / 60 ); //buggy
		tg->timeSyncFrames = info->u.timesync.frames_ahead;
		break;
	}
	return true;
}


/*
* vw_advance_frame_callback --
*
* Notification from GGPO we should step foward exactly 1 frame
* during a rollback.
*/
bool __cdecl
advance_frame_callback(int flags)
{
	int compressedInputs[GGPO_MAX_PLAYERS] = { 0 };
	int disconnect_flags;

	TestGame *tg = TestGame::GetInstance();

	ggpo_synchronize_input(tg->ggpo, (void *)compressedInputs, sizeof(int) * GGPO_MAX_PLAYERS,
		&disconnect_flags);

	
	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		tg->currInputs[i] = compressedInputs[i];
	}

	tg->UpdatePlayerInput();
	tg->GameUpdate();
	//sess->GGPORunGameModeUpdate();
	return true;
}

/*
* vw_load_game_state_callback --
*
* Makes our current state match the state passed in by GGPO.
*/
bool __cdecl
load_game_state_callback(unsigned char *buffer, int len)
{
	TestGame *tg = TestGame::GetInstance();


	return tg->LoadState(buffer, len);
}

/*
* vw_save_game_state_callback --
*
* Save the current state to a buffer and return it to GGPO via the
* buffer and len parameters.
*/
bool __cdecl
save_game_state_callback(unsigned char **buffer, int *len, int *checksum, int frame)
{
	return TestGame::GetInstance()->SaveState(buffer, len, checksum, frame);
}

bool __cdecl
log_game_state(char *filename, unsigned char *buffer, int)
{
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "w");
	if (fp) {
		fprintf(fp, "test\n");
		/*GameState *gamestate = (GameState *)buffer;
		fprintf(fp, "GameState object.\n");
		fprintf(fp, "  bounds: %d,%d x %d,%d.\n", gamestate->_bounds.left, gamestate->_bounds.top,
		gamestate->_bounds.right, gamestate->_bounds.bottom);
		fprintf(fp, "  num_ships: %d.\n", gamestate->_num_ships);
		for (int i = 0; i < gamestate->_num_ships; i++) {
		Ship *ship = gamestate->_ships + i;
		fprintf(fp, "  ship %d position:  %.4f, %.4f\n", i, ship->position.x, ship->position.y);
		fprintf(fp, "  ship %d velocity:  %.4f, %.4f\n", i, ship->velocity.dx, ship->velocity.dy);
		fprintf(fp, "  ship %d radius:    %d.\n", i, ship->radius);
		fprintf(fp, "  ship %d heading:   %d.\n", i, ship->heading);
		fprintf(fp, "  ship %d health:    %d.\n", i, ship->health);
		fprintf(fp, "  ship %d speed:     %d.\n", i, ship->speed);
		fprintf(fp, "  ship %d cooldown:  %d.\n", i, ship->cooldown);
		fprintf(fp, "  ship %d score:     %d.\n", i, ship->score);
		for (int j = 0; j < MAX_BULLETS; j++) {
		Bullet *bullet = ship->bullets + j;
		fprintf(fp, "  ship %d bullet %d: %.2f %.2f -> %.2f %.2f.\n", i, j,
		bullet->position.x, bullet->position.y,
		bullet->velocity.dx, bullet->velocity.dy);
		}
		}*/
		fclose(fp);
	}
	return true;
}

/*
* vw_free_buffer --
*
* Free a save state buffer previously returned in vw_save_game_state_callback.
*/
void __cdecl
free_buffer(void *buffer)
{
	free(buffer);
}

Player::Player( int index )
{
	playerIndex = index;

	if (playerIndex == 0)
	{
		rs.setFillColor(Color::Red);
		rs.setPosition(100, 100);

	}
	else
	{
		rs.setFillColor(Color::Green);
		rs.setPosition(200, 100);
	}

	rs.setSize(Vector2f(40, 40));

	rs.setOrigin(rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2);
	
}

void Player::Update()
{
	if (state.currInput > 0)
	{
		rs.move(Vector2f(0, .5));
	}
}

void Player::Draw(sf::RenderTarget *target)
{
	target->draw(rs);
}


TestGame::TestGame()
{
	currInstance = this;

	ggpoMode = true;

	isSyncTest = false;

	ggpo = NULL;

	preScreenTexture = new RenderTexture;
	preScreenTexture->create(1920, 1080);
	preScreenTexture->clear();

	SetupWindow();

	
}

TestGame::~TestGame()
{
	delete window;


	if (ggpo != NULL)
	{
		ggpo_close_session(ggpo);
		ggpo = NULL;

		WSACleanup();
	}

	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		if (players[i] != NULL)
		{
			delete players[i];
		}
	}

	delete preScreenTexture;
}

void TestGame::InitGGPO()
{
	timeSyncFrames = 0;
	//srand(400);
	srand(time(0));
	WSADATA wd = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wd);

	GGPOSessionCallbacks cb = { 0 };
	cb.begin_game = begin_game_callback;
	cb.advance_frame = advance_frame_callback;
	cb.load_game_state = load_game_state_callback;
	cb.save_game_state = save_game_state_callback;
	cb.free_buffer = free_buffer;
	cb.on_event = on_event_callback;
	cb.log_game_state = log_game_state;

	GGPOErrorCode result;

	//unsigned short localPort = 7000;
	//unsigned short otherPort = 7001;

	bool shift = Keyboard::isKeyPressed(Keyboard::LShift);//HoldingShift();
	/*if (shift)
	{
		localPort = 7001;
		otherPort = 7000;
	}*/

	int frameDelay = 0;
	string ipStr;// = "127.0.0.1";

	ifstream is;
	is.open("ggpotest.txt");
	is >> frameDelay;
	is >> ipStr;

	int sync;
	is >> sync;

	isSyncTest = sync > 0;


	//int offset = 1, local_player = 0;
	int num_players = 2;
	ngs.num_players = num_players;


	if (isSyncTest)
	{
		result = ggpo_start_synctest(&ggpo, &cb, "steamworkstestsync", num_players,
			sizeof(int), 1);
	}
	else
	{
		result = ggpo_start_session(&ggpo, &cb, "steamworkstest", num_players,
			sizeof(int) );
	}



	//ggpo_log(ggpo, "test\n");
	//result = ggpo_start_session(&ggpo, &cb, "vectorwar", num_players, sizeof(int), localport);
	ggpo_set_disconnect_timeout(ggpo, 0); //3000
	ggpo_set_disconnect_notify_start(ggpo, 1000);
	int myIndex = 0;
	int otherIndex = 1;
	if (shift)
	{
		myIndex = 1;
		otherIndex = 0;
	}

	ggpoPlayers[myIndex].size = sizeof(ggpoPlayers[myIndex]);
	ggpoPlayers[myIndex].player_num = myIndex + 1;
	ggpoPlayers[otherIndex].size = sizeof(ggpoPlayers[otherIndex]);
	ggpoPlayers[otherIndex].player_num = otherIndex + 1;
	ggpoPlayers[myIndex].type = GGPO_PLAYERTYPE_LOCAL;
	ggpoPlayers[otherIndex].type = GGPO_PLAYERTYPE_REMOTE;
	//	local_player = myIndex;

	/*int ipLen = ipStr.length();
	for (int i = 0; i < ipLen; ++i)
	{
		ggpoPlayers[otherIndex].u.remote.ip_address[i] = ipStr[i];
	}
	ggpoPlayers[otherIndex].u.remote.ip_address[ipLen] = '\0';
	ggpoPlayers[otherIndex].u.remote.port = otherPort;*/


	ggpoPlayers[otherIndex].u.remote.connection = testConnection;

	int i;
	for (i = 0; i < num_players; i++) {
		GGPOPlayerHandle handle;
		result = ggpo_add_player(ggpo, ggpoPlayers + i, &handle);
		ngs.playerInfo[i].handle = handle;
		ngs.playerInfo[i].type = ggpoPlayers[i].type;
		if (ggpoPlayers[i].type == GGPO_PLAYERTYPE_LOCAL) {
			ngs.playerInfo[i].connect_progress = 100;
			ngs.local_player_handle = handle;
			ngs.SetConnectState(handle, Connecting);
			ggpo_set_frame_delay(ggpo, handle, frameDelay);
		}
		else {
			ngs.playerInfo[i].connect_progress = 0;
		}
	}
}

void TestGame::UpdatePlayerInput()
{
	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		if (players[i] == NULL)
		{
			continue;
		}

		players[i]->state.prevInput = players[i]->state.currInput;
		players[i]->state.currInput = currInputs[i];
	}
}

bool TestGame::LoadState(unsigned char *bytes, int len)
{
	int saveSize = sizeof(SaveGameState);
	memcpy(&currSaveState, bytes, saveSize);

	bytes += saveSize;

	currGameState = currSaveState;

	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		if (players[i] == NULL)
		{
			continue;
		}
		else
		{
			players[i]->state = currGameState.states[i];
		}
	}

	return true;
}

bool TestGame::SaveState(unsigned char **buffer,
	int *len, int *checksum, int frame)
{
	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		if (players[i] == NULL)
		{
			continue;
		}
		else
		{
			currSaveState = currGameState;
			currSaveState.states[i] = players[i]->state;
		}
	}

	*len = sizeof(SaveGameState);
	*buffer = (unsigned char *)malloc(*len);
	memset(*buffer, 0, *len);

	if (!*buffer) {
		return false;
	}
	memcpy(*buffer, &currSaveState, sizeof(SaveGameState));


	unsigned char *tempBuf = *buffer;
	tempBuf += sizeof(SaveGameState);

	
	int pSize = sizeof(PState);
	int offset = 0;
	int fletchLen = *len;
	*checksum = fletcher32_checksum((short *)((*buffer) + offset), fletchLen / 2);
	return true;
}

void TestGame::GameUpdate()
{
	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		if (players[i] == NULL)
			continue;
		
		players[i]->Update();
	}

	

	SteamAPI_RunCallbacks();


	currGameState.totalGameFrames++;

	if (ggpoMode)
	{
		ggpo_advance_frame(ggpo);
	}
	
}

void TestGame::GGPORunFrame()
{
	int disconnect_flags;
	int compressedInputs[GGPO_MAX_PLAYERS] = { 0 };

	int localIndex = 0;
	if (!isSyncTest)
	{
		localIndex = ngs.local_player_handle - 1;
	}

	int inputs[4] = { 0 };
	if (Keyboard::isKeyPressed(Keyboard::W) && window->hasFocus())
	{
		cout << "moving: " << ngs.local_player_handle << endl;
		inputs[localIndex] += 1;
	}
	else
	{
		//cout << "local handle: " << ngs.local_player_handle << endl;
	}

	/*if (Keyboard::isKeyPressed(Keyboard::I))
	{
		inputs[1] += 1;
	}*/

	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		currInputs[i] = inputs[i];
	}


	assert(ngs.local_player_handle != GGPO_INVALID_HANDLE);

	int input = currInputs[localIndex];
	GGPOErrorCode result = ggpo_add_local_input(ggpo, ngs.local_player_handle, &input, sizeof(input));
	//cout << "local player handle: " << ngs->local_player_handle << "\n";

	if (GGPO_SUCCEEDED(result))
	{
		result = ggpo_synchronize_input(ggpo, (void*)compressedInputs, sizeof(int) * GGPO_MAX_PLAYERS, &disconnect_flags);
		if (GGPO_SUCCEEDED(result))
		{
			for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
			{
				currInputs[i] = compressedInputs[i];
			}

			UpdatePlayerInput();
			GameUpdate();
		}

	}
}

void TestGame::RegularRunFrame()
{
	int inputs[4] = { 0 };
	if (Keyboard::isKeyPressed(Keyboard::W))
	{
		inputs[0] += 1;
	}

	if (Keyboard::isKeyPressed(Keyboard::I))
	{
		inputs[1] += 1;
	}

	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		currInputs[i] = inputs[i];
	}

	UpdatePlayerInput();
	GameUpdate();
}

void TestGame::FullFrameUpdate()
{
	double newTime = gameClock.getElapsedTime().asSeconds();
	double frameTime = newTime - currentTime;

	if (ggpoMode)
	{
		ggpo_idle(ggpo, 5);
	}

	currentTime = newTime;

	accumulator += frameTime;

	


	if (accumulator >= TIMESTEP && timeSyncFrames > 0)
	{
		--timeSyncFrames;
		accumulator -= TIMESTEP;
	}
	else
	{
		while (accumulator >= TIMESTEP)
		{
			if (ggpoMode)
			{
				GGPORunFrame();
			}
			else
			{
				RegularRunFrame();
			}
			
			//UpdateNetworkStats();
			accumulator -= TIMESTEP;
		}
	}

	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		if (players[i] == NULL)
			continue;

		players[i]->Draw(preScreenTexture);
	}
}

void TestGame::SetActionRunGame()
{
	action = A_RUN_GAME;
	gameClock.restart();
	accumulator = TIMESTEP + .1;
	currentTime = 0;
	currGameState.totalGameFrames = 0;
}

void TestGame::Run()
{
	for (int i = 0; i < GGPO_MAX_PLAYERS; ++i)
	{
		players[i] = NULL;
	}

	players[0] = new Player(0);
	players[1] = new Player(1);

	SetActionRunGame();

	if (ggpoMode)
	{
		action = A_GATHER_USERS;
		lobbyManager.FindLobby();	
	}


	bool quit = false;
	while (!quit)
	{
		sf::Event ev;
		while (window->pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:
				quit = true;
				break;
			}
		}

		preScreenTexture->clear();
		
		switch (action)
		{
		case A_GATHER_USERS:
		{
			lobbyManager.Update();

			if (lobbyManager.GetNumCurrentLobbyMembers() == 2)
			{
				action = A_GET_CONNECTIONS;

				if (lobbyManager.currentLobby.createdByMe)
				{
					cout << "create listen socket" << endl;
					connectionManager.CreateListenSocket();
				}
				else
				{
					cout << "other test " << endl;
					//this is really bad/messy for 4 players. figure out how to do multiple p2p connections soon
					CSteamID myId = SteamUser()->GetSteamID();
					for (auto it = lobbyManager.currentLobby.memberList.begin(); it != lobbyManager.currentLobby.memberList.end(); ++it)
					{
						if ((*it) == myId)
						{
							continue;
						}

						cout << "try to connect" << endl;
						connectionManager.ConnectToID((*it));
					}
				}
			}

			SteamAPI_RunCallbacks();
			break;
		}
		case A_GET_CONNECTIONS:
		{
			if (connectionManager.connected)
			{
				testConnection = connectionManager.connection;

				lobbyManager.LeaveLobby(); //need to see if this causes problems or not. I don't think so.

				InitGGPO(); //call this once I have the connections ready.
				SetActionRunGame();
			}

			SteamAPI_RunCallbacks();
			break;
		}
		case A_RUN_GAME:
		{
			FullFrameUpdate();
			break;
		}
	
		}

		preScreenTexture->display();
		const Texture &preTex = preScreenTexture->getTexture();

		Sprite preTexSprite(preTex);
		//preTexSprite.setPosition(-960 / 2, -540 / 2);
		//preTexSprite.setScale(.5, .5);
		window->clear(Color::White);
		window->draw(preTexSprite);
		window->display();
	}

	window->close();

	//delete window;
}

void TestGame::SetupWindow()
{
	window = NULL;
	/*windowWidth = 960;
	windowHeight = 540;*/
	window = new RenderWindow(sf::VideoMode(500, 500), "Breakneck");
	window->setKeyRepeatEnabled(false);

	window->setVerticalSyncEnabled(true);
	//window->setFramerateLimit(120);
}

TestGame *TestGame::GetInstance()
{
	return currInstance;
}