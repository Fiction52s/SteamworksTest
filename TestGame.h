#ifndef __TEST_GAME_H__
#define __TEST_GAME_H__

#include <SFML\Graphics.hpp>
#include "ggponet.h"
#include "steam\steam_api.h"
#include "LobbyManager.h"
#include "ConnectionManager.h"

#pragma comment(lib, "wsock32.lib")

#define TIMESTEP (1.0 / 60.0)

enum PlayerConnectState {
	Connecting = 0,
	Synchronizing,
	Running,
	Disconnected,
	Disconnecting,
};

struct PlayerConnectionInfo {
	GGPOPlayerType       type;
	GGPOPlayerHandle     handle;
	PlayerConnectState   state;
	int                  connect_progress;
	int                  disconnect_timeout;
	int                  disconnect_start;
};

struct GGPONonGameState
{
	struct ChecksumInfo {
		int framenumber;
		int checksum;
	};

	void SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state) {
		for (int i = 0; i < num_players; i++) {
			if (playerInfo[i].handle == handle) {
				playerInfo[i].connect_progress = 0;
				playerInfo[i].state = state;
				break;
			}
		}
	}

	void SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout) {
		for (int i = 0; i < num_players; i++) {
			if (playerInfo[i].handle == handle) {
				playerInfo[i].disconnect_start = when;
				playerInfo[i].disconnect_timeout = timeout;
				playerInfo[i].state = Disconnecting;
				break;
			}
		}
	}

	void SetConnectState(PlayerConnectState state) {
		for (int i = 0; i < num_players; i++) {
			playerInfo[i].state = state;
		}
	}

	void UpdateConnectProgress(GGPOPlayerHandle handle, int progress) {
		for (int i = 0; i < num_players; i++) {
			if (playerInfo[i].handle == handle) {
				playerInfo[i].connect_progress = progress;
				break;
			}
		}
	}

	GGPOPlayerHandle     local_player_handle;
	PlayerConnectionInfo playerInfo[4];
	int                  num_players;

	ChecksumInfo         now;
	ChecksumInfo         periodic;
};

struct PState
{
	sf::Vector2f position;
	int currInput;
	int prevInput;
};

struct SaveGameState
{
	PState states[GGPO_MAX_PLAYERS];
	int totalGameFrames;
};

struct Player
{
	PState state;

	sf::RectangleShape rs;
	int playerIndex;

	Player( int index );

	void Update();

	void Draw(sf::RenderTarget *target);
};

int fletcher32_checksum(short *data,
	size_t len);

bool __cdecl
begin_game_callback(const char *);

bool __cdecl
on_event_callback(GGPOEvent *info);

bool __cdecl
advance_frame_callback(int flags);

bool __cdecl
load_game_state_callback(unsigned char *buffer, int len);

bool __cdecl
save_game_state_callback(unsigned char **buffer, int *len, int *checksum, int frame);

bool __cdecl
log_game_state(char *filename, unsigned char *buffer, int);

void __cdecl
free_buffer(void *buffer);

struct TestGame
{
	enum Action
	{
		A_GATHER_USERS,
		A_GET_CONNECTIONS,
		A_RUN_GAME,
	};

	Action action;

	static TestGame *currInstance;
	static TestGame *GetInstance();

	HSteamNetConnection testConnection;
	LobbyManager lobbyManager;
	ConnectionManager connectionManager;

	GGPONonGameState ngs;
	GGPOSession *ggpo;
	GGPOPlayer ggpoPlayers[GGPO_MAX_PLAYERS];

	int timeSyncFrames;
	int currInputs[GGPO_MAX_PLAYERS];
	int prevInputs[GGPO_MAX_PLAYERS];

	Player *players[GGPO_MAX_PLAYERS];

	SaveGameState currSaveState;

	bool ggpoMode;


	SaveGameState currGameState;

	double currentTime;
	double accumulator;
	sf::Clock gameClock;
	sf::RenderWindow *window;

	

	bool isSyncTest;

	sf::RenderTexture *preScreenTexture;

	TestGame();
	~TestGame();

	void SetActionRunGame();
	void UpdatePlayerInput();
	bool LoadState(unsigned char *bytes, int len);
	bool SaveState(unsigned char **buffer,
		int *len, int *checksum, int frame);
	void GameUpdate();
	void GGPORunFrame();
	void RegularRunFrame();
	void InitGGPO();

	void FullFrameUpdate();
	void Run();

	void SetupWindow();


};

#endif
