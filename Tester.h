#pragma once

#ifndef __DONE_H
#define __DONE_H

#include "steam/steam_api.h"

struct WorkshopTester
{
	PublishedFileId_t uploadedID;
	WorkshopTester();
	void OnCreatedItem(CreateItemResult_t *pCallback, bool bIOFailure);
	void OnItemUpdated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
	void OnQueryCompleted(SteamUGCQueryCompleted_t *pCallback, bool bIOFailure);
	bool LoadWorkshopItem(PublishedFileId_t workshopItemID);
	void OnHTTPRequestCompleted(HTTPRequestCompleted_t *callback, bool bIOFailure);
private:
	//STEAM_CALLBACK(WorkshopTester, OnCreatedItem, CreateItemResult_t);
	STEAM_CALLBACK(WorkshopTester, OnItemUpdatesSubmitted, SubmitItemUpdateResult_t);
	STEAM_CALLBACK(WorkshopTester, OnGameOverlayActivated, GameOverlayActivated_t);
	STEAM_CALLBACK(WorkshopTester, HTTPTestCallback, HTTPRequestCompleted_t);
};

#endif