#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "BBOServer.h"
#include "BBO-Savatar.h"
#include "BBO-Snpc.h"
#include "BBO-Stree.h"
#include "BBO-Smonster.h"
#include "BBO-Sgenerator.h"
#include "BBO-Sgen-spirits.h"
#include "BBO-Sgen-dragons.h"
#include "BBO-Sgen-orcClan.h"
#include "BBO-Sgen-vamps.h"
#include "BBO-Sgen-nearvamps.h"
#include "BBO-SNewbGenerator.h"
#include "BBO-Stower.h"
#include "BBO-Schest.h"
#include "BBO-Sbomb.h"
#include "BBO-SwarpPoint.h"
#include "BBO-SgroundEffect.h"
#include "monsterData.h"
#include "Ground-Map.h"
#include "Dungeon-Map.h"
#include "Realm-Map.h"
#include "Tower-Map.h"
#include "Labyrinth-Map.h"
#include "tokenManager.h"
#include ".\helper\GeneralUtils.h"
#include ".\helper\UniqueNames.h"
#include "longtime.h"
#include ".\helper\sendMail.h"
#include "ArmySpiders.h"
#include "ArmyCustomNormal.h"
#include "ArmyHalloween.h"
#include "ArmyDead.h"
#include "ArmyDragons.h"
#include "ArmyDuskWurm.h"
#include "ArmyDragonChaplain.h"
#include "ArmyDragonArchMage.h"
#include "ArmyDragonOverlord.h"
#include "ipBanList.h"
#include "uidbanlist.h"
#include ".\helper\crypto.h"
#include ".\helper\crc.h"
#include "QuestSystem.h"
#include "BBO-SchainQuest.h"
#include "BBO-SinvadeQuest.h"
#include "BBO-SChristmasQuest.h"
#include "hotkeys.h"
#include "version.h"

BBOServer *bboServer = NULL; // better be only one instance of this server at a time!
IPBanList *banList;
UIDBanList *uidBanList;

extern char skillNameArray[20][32];

char *cryptoText1 = "Connecting...";
char *cryptoText2 = "Sending hello...";
char *cryptoText3 = "Simple avatars OFF";
int max_logins,use_infinity,run_halloween,infinite_supply,possessed_monsters_move,combat_exp_multiplier,smith_exp_multiplier,bombing_exp_multiplier,geomancy_exp_multiplier,ts_exp_multiplier,magic_exp_multiplier,mastery_exp_multiplier;
//******************************************************************
UserMessage::UserMessage(unsigned long connectionID, char *name) : DataObject(connectionID,name)
{
    password[0] = 0;
    avatar  [0] = 0;
    message [0] = 0;

    color = 0xffffffff;
    age = timeGetTime() + 1000 * 60 * 12;
}

//******************************************************************
UserMessage::~UserMessage()
{
}

//******************************************************************
//******************************************************************
BBOServer::BBOServer(int useIOCP)
{
	
	bboServer = this;

	pleaseKillMe = 10000; // kill when this gets to 0

	// Create Logs directory
	CreateDirectory("Logs", NULL);

	char tempText[1024];
	char tempText2[1024];
	LongTime lt;
	lt.value.wHour += 19;
	if (lt.value.wHour < 24)
		lt.value.wDay -= 1;
	else
		lt.value.wHour -= 24;

	sprintf_s(tempText, "--------Server Starting at %d/%02d, %d:%02d\n", lt.value.wMonth, lt.value.wDay,
		lt.value.wHour, lt.value.wMinute);
	LogOutput("gamelog.txt", tempText);
	// load configuration file
	FILE *fp = fopen("serverdata\\config.dat", "r");
	//	int max_logins;
	fscanf(fp, "MAX_LOGINS %d\n", &max_logins); 
	sprintf_s(tempText, "MAX_LOGINS is %d\n", max_logins);
	LogOutput("gamelog.txt", tempText);
	fscanf(fp, "ENABLE_TOI %d\n", &use_infinity);
	sprintf_s(tempText, "ENABLE_TOI is %d\n", use_infinity);
	LogOutput("gamelog.txt", tempText);
	fscanf(fp, "ENABLE_HALLOWEEN %d\n", &run_halloween);
	sprintf_s(tempText, "ENABLE_HALLOWEEN is %d\n", run_halloween);
	LogOutput("gamelog.txt", tempText);
	fscanf(fp, "INFINITE_SUPPLY %d\n", &infinite_supply);
	sprintf_s(tempText, "INFINITE_SUPPLY is %d\n", infinite_supply);
	LogOutput("gamelog.txt", tempText);
	fscanf(fp, "ENABLE_CHEATING %d\n", &enable_cheating);
	sprintf_s(tempText, "ENABLE_CHEATING is %d\n", enable_cheating);
	LogOutput("gamelog.txt", tempText);
	fscanf(fp, "ENABLE_WALL_BREAKING %d\n", &enable_wall_breaking);
	sprintf_s(tempText, "ENABLE_WALL_BREAKING is %d\n", enable_wall_breaking);
	LogOutput("gamelog.txt", tempText);
	fscanf(fp, "REMOVE_GEO_WALLS %d\n", &remove_geo_walls);
	sprintf_s(tempText, "REMOVE_GEO_WALLS is %d\n", remove_geo_walls);
	fscanf(fp, "POSSESSED_MONSTERS_MOVE %d\n", &possessed_monsters_move);
	sprintf_s(tempText, "POSSESSED_MONSTERS_MOVE is %d\n", possessed_monsters_move);
	fscanf(fp, "COMBAT_EXP_MULTIPLIER %d\n", &combat_exp_multiplier); // wtf
	sprintf_s(tempText, "COMBAT_EXP_MULTIPLIER is %d\n", combat_exp_multiplier);
	fscanf(fp, "SMITH_EXP_MULTIPLIER %d\n", &smith_exp_multiplier);
	sprintf_s(tempText, "SMITH_EXP_MULTIPLIER is %d\n", smith_exp_multiplier);
	fscanf(fp, "BOMBING_EXP_MULTIPLIER %d\n", &bombing_exp_multiplier);
	sprintf_s(tempText, "BOMBING_EXP_MULTIPLIER is %d\n", bombing_exp_multiplier);
	fscanf(fp, "GEOMANCY_EXP_MULTIPLIER %d\n", &geomancy_exp_multiplier);
	sprintf_s(tempText, "GEOMANCY_EXP_MULTIPLIER is %d\n", geomancy_exp_multiplier);
	fscanf(fp, "TS_EXP_MULTIPLIER %d\n", &ts_exp_multiplier);
	sprintf_s(tempText, "TS_EXP_MULTIPLIER is %d\n", ts_exp_multiplier);
	fscanf(fp, "MAGIC_EXP_MULTIPLIER %d\n", &magic_exp_multiplier);
	sprintf_s(tempText, "MAGIC_EXP_MULTIPLIER is %d\n", magic_exp_multiplier);
	fscanf(fp, "MASTERY_EXP_MULTIPLIER %d\n", &mastery_exp_multiplier);
	sprintf_s(tempText, "MASTERY_EXP_MULTIPLIER is %d\n", mastery_exp_multiplier);
	LogOutput("gamelog.txt", tempText);

	fclose(fp);

	IOCPFlag = useIOCP;
	if (useIOCP)
	{
		IOCPSocket::StartIOCP();
		lserver = new IOCPServer();
#ifdef _TEST_SERVER
		((IOCPServer *)lserver)->startServer(9178, 100, NULL);
#else
		if (enable_cheating==0)
			((IOCPServer *)lserver)->startServer(3678, 100, NULL);
		else
			((IOCPServer *)lserver)->startServer(3679, 100, NULL);
#endif
	}
	else
	{
		lserver = new Server();
#ifdef _TEST_SERVER
		((Server *)lserver)->startServer(9178, 100, NULL, NULL);
#else
		if (enable_cheating == 0)
			((IOCPServer *)lserver)->startServer(3678, 100, NULL);
		else
			((IOCPServer *)lserver)->startServer(3679, 100, NULL);
#endif
	}

	//	lserver = new Server();
	//	lserver->startServer(3678, 32, NULL, NULL);

	//	mobs       = new DoublyLinkedList();
	//	avatars    = new DoublyLinkedList();
	incoming = new DoublyLinkedList();
	spaceList = new DoublyLinkedList();
	BBOSMob *curMobbadplace; //, *tempMob;
	userMessageList = new DoublyLinkedList();
	sprintf_s(tempText, "Loading Pet Data\n");
	LogOutput("gamelog.txt", tempText);

	LoadPetData();

	sprintf_s(tempText, "Initializing main map\n");
	LogOutput("gamelog.txt", tempText);
	GroundMap *gm = new GroundMap(SPACE_GROUND, "Ground", lserver);
	gm->InitNew(0, 0, 0, 0);
	spaceList->Append(gm);
	//	dungeonList= new DoublyLinkedList();

		// add in starting mobs
		/*
		BBOSMonster *monster = new BBOSMonster(0,0);
		mobList->Add(monster);
		monster = new BBOSMonster(0,1);
		monster->cellX--;
		mobList->Add(monster);
		monster = new BBOSMonster(0,2);
		monster->cellX++;
		mobList->Add(monster);
		*/

		// add town merchants
	sprintf_s(tempText, "Adding Merchants.\n");
	LogOutput("gamelog.txt", tempText);
	for (int t = 0; t < NUM_OF_TOWNS; ++t)
	{
		for (int t2 = 0; t2 < townList[t].size + 1; ++t2)
		{
			BBOSNpc *npc = new BBOSNpc(SMOB_TRADER);
			npc->level = townList[t].level;
			npc->townIndex = t;
			do
			{
				npc->cellX = townList[t].x + (rand() % (1 + townList[t].size)) - townList[t].size / 2;
				npc->cellY = townList[t].y + (rand() % (1 + townList[t].size)) - townList[t].size / 2;
				if ((npc->cellY == (townList[t].y + 1)) && (npc->cellX == (townList[t].x))) // if it's where the town mage will be
					npc->cellY++; // move it out of town mage square
			} while (npc->cellX == townList[t].x && npc->cellY == townList[t].y);
			gm->mobList->Add(npc);
		}
	}

	// add town trainers
	sprintf_s(tempText, "Adding trainers.\n");
	LogOutput("gamelog.txt", tempText);
	for (int t = 0; t < NUM_OF_TOWNS; ++t)
	{
		for (int t2 = 0; t2 < townList[t].size / 2 + 1; ++t2)
		{
			BBOSNpc *npc = new BBOSNpc(SMOB_TRAINER);
			do
			{
				npc->cellX = townList[t].x + (rand() % (1 + townList[t].size)) - townList[t].size / 2;
				npc->cellY = townList[t].y + (rand() % (1 + townList[t].size)) - townList[t].size / 2;
			} while (npc->cellX == townList[t].x && npc->cellY == townList[t].y);
			gm->mobList->Add(npc);
		}
	}

	// add town mages
	sprintf_s(tempText, "Adding Town Mages.\n");
	LogOutput("gamelog.txt", tempText);
	for (int t = 0; t < NUM_OF_TOWNS; ++t)
	{
		BBOSNpc *npc = new BBOSNpc(SMOB_TOWNMAGE);
		npc->cellX = townList[t].x;
		npc->cellY = townList[t].y + 1;
		gm->mobList->Add(npc);
	}


	// add the new enterance to the Realm of Pain
	BBOSWarpPoint *wpRop = new BBOSWarpPoint(161, 121, SMOB_ROPENTERANCE);
	wpRop->allCanUse = true;
	gm->mobList->Add(wpRop);
	//	BBOSTower *towerInfinity = new BBOSTower(161, 121); 
	//	towerInfinity->do_id = SMOB_ROPENTERANCE; // try to use tower mesh.
	//	gm->mobList->Add(towerInfinity);

		// make infinite tower lobby
	//	DungeonMap *dmT = new DungeonMap(SPACE_DUNGEON, "Infinite Tower Lobby", lserver);
	//	dmT->InitNew(5, 5, 161, 121, 0);
	//	spaceList->Append(dmT);
	//	towerInfinity->ss = dmT;
	//	dmT->specialFlags = SPECIAL_DUNGEON_DOOMTOWER;
	//	sprintf_s(dmT->name, "Infinite Tower Lobby");
	//	// add exit warp
	//	BBOSWarpPoint *wpT = new BBOSWarpPoint(dmT->width - 1, dmT->height - 1);
	//	wpT->targetX = dmT->enterX;
	//	wpT->targetY = dmT->enterY - 1;
	//	wpT->spaceType = SPACE_GROUND;
	//	wpT->spaceSubType = 0;
	//	dmT->mobList->Add(wpT);

		// add ticket merchant if enabled in configuration file.
	if (use_infinity == 1)
	{
		BBOSNpc *ticketseller = new BBOSNpc(SMOB_TRADER);
		ticketseller->level = 25; // Tower of Infinity merchant
		ticketseller->townIndex = 0;
		ticketseller->cellX = 162;
		ticketseller->cellY = 121;
		gm->mobList->Add(ticketseller);
		char tempTextT[1028];
		sprintf(tempTextT, "Tower of Infinity Ticket.");
		InventoryObject *iObject;
		iObject = new InventoryObject(
			INVOBJ_DOOMKEY_ENTRANCE, 0, tempTextT);
		InvDoomKey *exIn = (InvDoomKey *)iObject->extra;
		exIn->power = 1;

		iObject->mass = 0.0f;
		iObject->value = 10000;
		iObject->amount = 100;
		exIn->width = 10;
		exIn->height = 10;
		ticketseller->inventory->AddItemSorted(iObject);

	}

	/*
	// add the new TEMP CASTLE MESH
	BBOSWarpPoint *wpCastle = new BBOSWarpPoint( 24, 24, SMOB_CASTLE );
	wpCastle->allCanUse = false;
	gm->mobList->Add( wpCastle );
	*/

	// add dungeons
	sprintf_s(tempText, "Adding dungeons.\n");
	LogOutput("gamelog.txt", tempText);

	fp = fopen("serverdata\\dungeons.dat", "r");

	if (!fp)
	{
		for (int t = 0; t < MAP_SIZE_WIDTH; ++t)
		{
			for (int t2 = 0; t2 < MAP_SIZE_HEIGHT; ++t2)
			{
				int col = gm->Color(t, t2) - 250;
				if (col >= 0 && col < 5)
				{
					BBOSTower *tower = new BBOSTower(t, t2);
					gm->mobList->Add(tower);

					DungeonMap *dm = new DungeonMap(SPACE_DUNGEON, "Test Dungeon", lserver);
					dm->InitNew(20 + col * 5, 20, t, t2, col);
					spaceList->Append(dm);

					tower->ss = dm;

					// add static monsters
					for (int m = 0; m < (dm->height * dm->width) / 12;)
					{
						int t = rand() % NUM_OF_MONSTERS;
						int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

						if (monsterData[t][t2].name[0] && monsterData[t][t2].dungeonType >= col - 3 &&
							monsterData[t][t2].dungeonType <= col)
						{
							int mx, my;
							do
							{
								mx = rand() % (dm->width);
								my = rand() % (dm->height);
							} while (mx < 4 && my < 4);

							BBOSMonster *monster = new BBOSMonster(t, t2, NULL);
							monster->cellX = mx;
							monster->cellY = my;
							monster->targetCellX = mx;
							monster->targetCellY = my;
							monster->spawnX = mx;
							monster->spawnY = my;
							dm->mobList->Add(monster);
							++m;
						}
					}
					// add wandering monsters
					if (0 == col)
						col = 1;

					for (int m = 0; m < (dm->height * dm->width) / 12;)
					{
						int t, t2;
						int done = FALSE;
						while (!done)
						{
							t = rand() % NUM_OF_MONSTERS;
							t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

							if (MONSTER_PLACE_DUNGEON & monsterData[t][t2].placementFlags)
								done = TRUE;
						}

						if (monsterData[t][t2].name[0] && monsterData[t][t2].dungeonType >= col - 3 &&
							monsterData[t][t2].dungeonType <= col - 1)
						{
							int mx, my;
							do
							{
								mx = rand() % (dm->width);
								my = rand() % (dm->height);
							} while (mx < 4 && my < 4);

							BBOSMonster *monster = new BBOSMonster(t, t2, NULL);
							monster->isWandering = TRUE;
							monster->cellX = mx;
							monster->cellY = my;
							monster->targetCellX = mx;
							monster->targetCellY = my;
							monster->spawnX = mx;
							monster->spawnY = my;
							dm->mobList->Add(monster);
							++m;
						}
					}
				}
			}
		}
	}
	else
	{
		srand(25);
		//		char tempText[128];
		float dunVers;
		fscanf(fp, "%f\n", &dunVers);

		LoadLineToString(fp, tempText);
		while (!strcmp(tempText, "DUNGEON"))
		{
			DungeonMap *dm = new DungeonMap(SPACE_DUNGEON, "Test Dungeon", lserver);
			if (dunVers < 2.04f)
			{
				dm->Load(fp, dunVers);
				sprintf_s(tempText, dm->name);
				int oldX = dm->enterX;
				int oldY = dm->enterY;
				delete dm;

				dm = new DungeonMap(SPACE_DUNGEON, "Test Dungeon", lserver);
				dm->InitNew(5, 10, -1, -1, rand() % 5);
				sprintf_s(dm->name, tempText);
				dm->enterX = oldX;
				dm->enterY = oldY;
			}
			else
				dm->Load(fp, dunVers);
			AddDungeonSorted(dm);

			if (dunVers < 2.02f)
			{
				dm->enterX *= 2;
				dm->enterY *= 2;
			}

			if (dunVers < 2.04f)
			{
				// add static monsters
				for (int m = 0; m < (dm->height * dm->width) / 12;)
				{
					int t = rand() % NUM_OF_MONSTERS;
					int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

					if (monsterData[t][t2].name[0])
					{
						int mx, my;
						do
						{
							mx = rand() % (dm->width);
							my = rand() % (dm->height);
							if (!(MONSTER_PLACE_DUNGEON & monsterData[t][t2].placementFlags))
							{
								mx = my = 1000; // try again
								t = rand() % NUM_OF_MONSTERS;
								t2 = rand() % NUM_OF_MONSTER_SUBTYPES;
							}
						} while (mx > dm->width - 2 && my > dm->height - 2);

						BBOSMonster *monster = new BBOSMonster(t, t2, NULL);
						monster->cellX = mx;
						monster->cellY = my;
						monster->targetCellX = mx;
						monster->targetCellY = my;
						monster->spawnX = mx;
						monster->spawnY = my;
						dm->mobList->Add(monster);
						++m;
					}
				}
			}

			int badPlace = TRUE;
			while (badPlace)
			{
				badPlace = FALSE;

				if (!gm->CanMove(dm->enterX, dm->enterY, dm->enterX, dm->enterY) ||
					!gm->CanMove(dm->enterX, dm->enterY - 1, dm->enterX, dm->enterY - 1))
					badPlace = TRUE;

				if (dm->enterX < 2 || dm->enterX >= MAP_SIZE_WIDTH - 2)
					badPlace = TRUE;

				if (dm->enterY < 2 || dm->enterY >= MAP_SIZE_HEIGHT - 2)
					badPlace = TRUE;

				for (int i = 0; i < NUM_OF_TOWNS; ++i)
				{
					if (abs(dm->enterX - townList[i].x) < townList[i].size &&
						abs(dm->enterY - townList[i].y) < townList[i].size)
						badPlace = TRUE;
				}

				if (badPlace)
				{
					dm->enterX = rand() % 256;
					dm->enterY = rand() % 256;
				}

			}
			BBOSTower *tower = new BBOSTower(dm->enterX, dm->enterY);
			gm->mobList->Add(tower);
			tower->ss = dm;

			// add the treasure chests
			BBOSChest *chest = new BBOSChest(rand() % dm->width, rand() % dm->height);
			dm->mobList->Add(chest);

			chest = new BBOSChest(rand() % dm->width, rand() % dm->height);
			dm->mobList->Add(chest);

			chest = new BBOSChest(rand() % dm->width, rand() % dm->height);
			dm->mobList->Add(chest);

			BBOSWarpPoint *wp = new BBOSWarpPoint(dm->width - 1, dm->height - 1);
			wp->targetX = dm->enterX;
			wp->targetY = dm->enterY - 1;
			wp->spaceType = SPACE_GROUND;
			wp->spaceSubType = 0;
			dm->mobList->Add(wp);

			LoadLineToString(fp, tempText);
		}

		fclose(fp);
		srand(timeGetTime());
	}

	// add mod-controlled dungeons
	BBOSTower *tower = new BBOSTower(62, 62);
	gm->mobList->Add(tower);

	// Mysterious cave will now persist if saved
	sprintf_s(tempText, "Opening cave dat file.\n");
	LogOutput("gamelog.txt", tempText);
	fp = fopen("serverdata\\cave.dat", "r");
	DungeonMap *dm = new DungeonMap(SPACE_DUNGEON, "Mysterious Cave", lserver);

	if (!fp)
	{
		sprintf_s(tempText, "Failed, creating default.\n");
		LogOutput("gamelog.txt", tempText);
		dm->InitNew(20, 20, 62, 62, 0);
		spaceList->Append(dm);
		tower->ss = dm;
		dm->specialFlags = SPECIAL_DUNGEON_MODERATED;
		sprintf_s(dm->name, "Mysterious Cave");
		BBOSWarpPoint *wp2 = new BBOSWarpPoint(dm->width - 1, dm->height - 1);
		wp2->targetX = dm->enterX;
		wp2->targetY = dm->enterY - 1;
		wp2->spaceType = SPACE_GROUND;
		wp2->spaceSubType = 0;
		dm->mobList->Add(wp2);

	}
	else
	{
		//		char tempText[128];
		sprintf_s(tempText, "Reading cave map.\n");
		LogOutput("gamelog.txt", tempText);

		// dm->Load(fp, dunVers); inline custom one instead.

		if (dm->leftWall)
			delete[] dm->leftWall;
		if (dm->topWall)
			delete[] dm->topWall;
		// load name.
		LoadLineToString(fp, dm->name);
		// load size info
		fscanf(fp, "%d %d %d %d %d", &dm->dungeonRating, &dm->width, &dm->height, &dm->enterX, &dm->enterY);
        // init
		dm->InitNew(dm->width, dm->height, dm->enterX, dm->enterY, dm->dungeonRating);
		// load layout
		int temp1, temp2,temp3;
		fscanf(fp, "%d %d", &temp1, &temp2);
		dm->floorIndex = temp1;
		dm->outerWallIndex = temp2;

		for (int i = 0; i < dm->width * dm->height; ++i)
		{
			fscanf(fp, "%d %d", &temp1, &temp2);
			dm->leftWall[i] = temp1;
			dm->topWall[i] = temp2;
		}
		// set flags
		dm->specialFlags = SPECIAL_DUNGEON_MODERATED;
        // add it to spacelist
		spaceList->Append(dm);
        //link entrance
		tower->ss = dm;
		// add exit
		BBOSWarpPoint *wp2 = new BBOSWarpPoint(dm->width - 1, dm->height - 1);
		wp2->targetX = dm->enterX;
		wp2->targetY = dm->enterY - 1;
		wp2->spaceType = SPACE_GROUND;
		wp2->spaceSubType = 0;
		dm->mobList->Add(wp2);
		fscanf(fp, "\n"); // go to next line.
		// load whatever else is in the file
		// look for player merchants
		LoadLineToString(fp, tempText);
		while (!strcmp(tempText, "PLAYERMERCHANT")) // there's a merchant, go load it :)
		{
			sprintf_s(tempText, "Loading Playemerchant.\n");
			LogOutput("gamelog.txt", tempText);

			// read owner
			fscanf(fp, "ACCOUNT %s\n", &tempText);
			fscanf(fp, "OWNER %s\n", &tempText2);
			// read coordinates
			sprintf_s(tempText, "Loading coords.\n");
			LogOutput("gamelog.txt", tempText);
			fscanf(fp, "%d %d\n", &temp2,&temp3);
			// create merchant on square.
			BBOSNpc *npc = new BBOSNpc(SMOB_PLAYERMERCHANT);
			// store playername and username so we can find them later when stuff is sold.
			sprintf(npc->username, tempText);
			sprintf(npc->playername, tempText2);
			sprintf(tempText2, "%s's Merchant", npc->playername); 
			CopyStringSafely(tempText2, 255, npc->do_name, 255);
			npc->level = 0;
			npc->townIndex = 0;
			//load inventory.
			sprintf_s(tempText, "Loading inventory.\n");
			LogOutput("gamelog.txt", tempText);
			npc->inventory->InventoryLoad(fp,VERSION_NUMBER);
			// load price table. this says what we are willing to buy, at wha tpprice, and how much.
			// TODO
			sprintf_s(tempText, "Adding to map list at %d %d.\n",temp2,temp3);
			LogOutput("gamelog.txt", tempText);

			npc->cellX = temp2;
			npc->cellY = temp3;
			dm->mobList->Add(npc);
		}
		// close file. only ever gets here if it opened. safe to have it here.
		fclose(fp);
	}

	// add monster generators
	sprintf_s(tempText, "Adding monster generators.\n");
	LogOutput("gamelog.txt", tempText);
	BBOSGenerator *curGen;
	curGen = new BBOSGenerator(0, 0);
	gm->generators->Append(curGen);
	for (int i = 0; i < NUM_OF_MONSTERS; ++i)
	{
		for (int j = 0; j < NUM_OF_MONSTER_SUBTYPES; ++j)
		{
			curGen->max[i][j] = 200;
		}
	}

	BBOSGenVamps *genVamps = new BBOSGenVamps(0, 0);
	gm->generators->Append(genVamps);

	BBOSGenNearVamps *genNearVamps = new BBOSGenNearVamps(0, 0);
	gm->generators->Append(genNearVamps);
	/*
		BBOSMonster *monster = new BBOSMonster(15,0, NULL);
		monster->cellX = 4;
		monster->cellY = 4;
		monster->targetCellX = 4;
		monster->targetCellY = 4;
		monster->spawnX = 4;
		monster->spawnY = 4;
		gm->mobList->Add(monster);



	*/

	BBOSChainQuest *curCQ;
	curCQ = new BBOSChainQuest(gm);
	gm->generators->Append(curCQ);

	BBOSInvadeQuest *curIQ;
	curIQ = new BBOSInvadeQuest(gm);
	gm->generators->Append(curIQ);

	BBOSChristmasQuest *curXQ;
	curXQ = new BBOSChristmasQuest(gm);
	gm->generators->Append(curXQ);

	BBOSNewbGenerator *curNewbGen;
	curNewbGen = new BBOSNewbGenerator(0, 0);
	gm->generators->Append(curNewbGen);

	sprintf_s(tempText, "Adding halloween if enabled.\n");
	LogOutput("gamelog.txt", tempText);

	// halloween is controlled by config file now.
	if (run_halloween)
	{
        // first aremy near fingle
		ArmyHalloween *ah = new ArmyHalloween(gm, 104, 53, 104, 43, 25);
		gm->generators->Append(ah);
		//second army spawns later
		ArmyHalloween *ah2 = new ArmyHalloween(gm, 94, 43, 94, 53, 25);
		gm->generators->Append(ah2);

	}
	sprintf_s(tempText, "Adding spider armies.\n");
	LogOutput("gamelog.txt", tempText);

	ArmySpiders *as;

    as = new ArmySpiders(gm, 120,37, 120, 27, 35);
    gm->generators->Append(as);

    as = new ArmySpiders(gm, 96,152, 90, 140, 45);
    gm->generators->Append(as);

    /*
    for (t = 0; t < 50;)
    {
        // pick a point
        int tempX = rand() % 128;
        int tempY = rand() % 128;

        // is it valid?
        if (map.CanMoveTo(tempX, tempY))
        {
            // is it too close to town?
            int toClose = FALSE;
            for (int t2 = 0; t2 < NUM_OF_TOWNS; ++t2)
            {
                if (abs(townList[t2].x - tempX) < MONSTER_SPAWN_RANGE/3 ||
                    abs(townList[t2].y - tempY) < MONSTER_SPAWN_RANGE/3)
                         toClose = TRUE;
            }

            if (!toClose)
            {
                // make a spawn point there
                curGen = new BBOSGenerator(tempX, tempY);
                generators->Append(curGen);
                ++t;
            }
        }
    }
    */

    // add the Great Trees
	sprintf_s(tempText, "Adding great trees.\n");
	LogOutput("gamelog.txt", tempText);
	for (int t = 0; t < 9; ++t)
    {
        BBOSTree *gt = new BBOSTree(0,0);
        gt->cellX = greatTreePos[t][0];
        gt->cellY = greatTreePos[t][1];
        gt->index = t;
        gm->mobList->Add(gt);
    }
	// add Unicorns. :)
	// first check for file
	{
		FILE *fp2 = fopen("serverdata\\unicornData.dat", "r");
		int tempint;
		if (fp2)
		{
			for (int i = 0; i < 6; ++i)
			{
				fscanf(fp2, "%d %d\n", &UnicornLocations[i][0], &UnicornLocations[i][1]);
				// add new unicorn in random square
				BBOSMonster *monster = new BBOSMonster(31, i, NULL);
				monster->cellX = monster->targetCellX = monster->spawnX = UnicornLocations[i][0];
				monster->cellY = monster->targetCellY = monster->spawnY = UnicornLocations[i][1];
				
				monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
				gm->mobList->Add(monster);
			}

			fclose(fp2);

		}
		else 
		{
			// pick a town omgbbq
			int town = rand() % NUM_OF_TOWNS;
			int randomX = rand() % 10 - 4;
			int randomY = rand() % 10 - 4;

			BBOSMonster *monster = new BBOSMonster(31, 0, NULL);
			monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
			monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
			monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
			gm->mobList->Add(monster);
			UnicornLocations[monster->subType][0] = monster->cellX;
			UnicornLocations[monster->subType][1] = monster->cellY;
			monster = new BBOSMonster(31, 1, NULL);
			monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
			monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
			monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
			gm->mobList->Add(monster);
			UnicornLocations[monster->subType][0] = monster->cellX;
			UnicornLocations[monster->subType][1] = monster->cellY;
			monster = new BBOSMonster(31, 2, NULL);
			monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
			monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
			monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
			gm->mobList->Add(monster);
			UnicornLocations[monster->subType][0] = monster->cellX;
			UnicornLocations[monster->subType][1] = monster->cellY;
			monster = new BBOSMonster(31, 3, NULL);
			monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
			monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
			monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
			gm->mobList->Add(monster);
			UnicornLocations[monster->subType][0] = monster->cellX;
			UnicornLocations[monster->subType][1] = monster->cellY;
			monster = new BBOSMonster(31, 4, NULL);
			monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
			monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
			monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
			gm->mobList->Add(monster);
			UnicornLocations[monster->subType][0] = monster->cellX;
			UnicornLocations[monster->subType][1] = monster->cellY;
			monster = new BBOSMonster(31, 5, NULL);
			monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
			monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
			monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
			gm->mobList->Add(monster);
			UnicornLocations[monster->subType][0] = monster->cellX;
			UnicornLocations[monster->subType][1] = monster->cellY;

		}
	}
	// pet graveyard 58 88 and 70 30
	sprintf_s(tempText, "Adding spirit realm.\n");
	LogOutput("gamelog.txt", tempText);

    RealmMap *rm = new RealmMap(SPACE_REALM,"Realm of Spirits",lserver);
    rm->InitNew("dat\\realm-spirits-server.raw",64,64,0,0);
    rm->type = REALM_ID_SPIRITS;
    spaceList->Append(rm);

//	rm->CreateSpiritStaticPositions();

    curGen = new BBOSGenerator(0,0);
    rm->generators->Append(curGen);


    // add Dokk
	BBOSMonster *monster = new BBOSMonster(16,1, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 62;
    monster->cellY = monster->targetCellY = monster->spawnY = 3;
    rm->mobList->Add(monster);

    // add Dokk's centurions
    monster = new BBOSMonster(11,1, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 62;
    monster->cellY = monster->targetCellY = monster->spawnY = 4;
    rm->mobList->Add(monster);

    monster = new BBOSMonster(11,1, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 61;
    monster->cellY = monster->targetCellY = monster->spawnY = 5;
    rm->mobList->Add(monster);

    monster = new BBOSMonster(11,1, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 62;
    monster->cellY = monster->targetCellY = monster->spawnY = 6;
    rm->mobList->Add(monster);

	sprintf_s(tempText, "Adding realm of the dead.\n");
	LogOutput("gamelog.txt", tempText);

    rm = new RealmMap(SPACE_REALM,"Realm of the Dead",lserver);
    rm->InitNew("dat\\realm-dead-server.raw",64,64,0,0);
    rm->type = REALM_ID_DEAD;
    spaceList->Append(rm);

    curGen = new BBOSGenerator(0,0);
    rm->generators->Append(curGen);

    // anubis
    ArmyDead *ad = new ArmyDead(rm, 41,5, 50, 2, 1);
    rm->generators->Append(ad);

    // other armies
    ad = new ArmyDead(rm, 32, 35, 21, 32, 2);
    rm->generators->Append(ad);
    ad = new ArmyDead(rm, 14,54, 24, 54, 3);
    rm->generators->Append(ad);
    ad = new ArmyDead(rm, 50,40, 43,25, 4);
    rm->generators->Append(ad);


    rm = new RealmMap(SPACE_REALM,"Realm of Dragons",lserver);
    rm->InitNew("dat\\realm-dragons-server.raw",64,64,0,0);
    rm->type = REALM_ID_DRAGONS;
    spaceList->Append(rm);

    curGen = new BBOSGenDragons(0,0);
    rm->generators->Append(curGen);

    curGen = new BBOSGenOrcClan(21,7);
    rm->generators->Append(curGen);

    curGen = new BBOSGenOrcClan(12,58);
    rm->generators->Append(curGen);

    curGen = new BBOSGenOrcClan(24,19);
    rm->generators->Append(curGen);

    ArmyOverlord *adr;

    adr = new ArmyOverlord(rm, 60, 9, 60, 2, 85);
    rm->generators->Append(adr);

    ArmyArchMage *aAM;

    aAM = new ArmyArchMage(rm, 61,53, 62, 62, 86);
    rm->generators->Append(aAM);

    ArmyChaplain *aChap;

    aChap = new ArmyChaplain(rm, 41,54, 41,54, 87);
    rm->generators->Append(aChap);

    ArmyDuskWurm *aDuskWurm;

    aDuskWurm = new ArmyDuskWurm(rm, 5, 42, 16, 42, 85);
    rm->generators->Append(aDuskWurm);

    aDuskWurm = new ArmyDuskWurm(rm, 36, 3, 36, 11, 86);
    rm->generators->Append(aDuskWurm);




    // add gateway skeletons
    
    monster = new BBOSMonster(19,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 67;
    monster->cellY = monster->targetCellY = monster->spawnY = 76;
	monster->tamingcounter = 1000; // taming gateway monster not allowed, it would vanish forever!
	gm->mobList->Add(monster);

    monster = new BBOSMonster(19,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 239;
    monster->cellY = monster->targetCellY = monster->spawnY = 145;
	monster->tamingcounter = 1000; // taming gateway monster not allowed, it would vanish forever!
	gm->mobList->Add(monster);

    monster = new BBOSMonster(19,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 223;
    monster->cellY = monster->targetCellY = monster->spawnY = 188;
	monster->tamingcounter = 1000; // taming gateway monster not allowed, it would vanish forever!
	gm->mobList->Add(monster);

    // add gateway spirits
    
    monster = new BBOSMonster(9,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 223;
    monster->cellY = monster->targetCellY = monster->spawnY = 8;
	monster->tamingcounter = 1000; // taming gateway monster not allowed, it would vanish forever!
	gm->mobList->Add(monster);

    monster = new BBOSMonster(9,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 45;
    monster->cellY = monster->targetCellY = monster->spawnY = 130;
	monster->tamingcounter = 1000; // taming gateway monster not allowed, it would vanish forever!
	gm->mobList->Add(monster);

    monster = new BBOSMonster(9,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 87;
    monster->cellY = monster->targetCellY = monster->spawnY = 230;
	monster->tamingcounter = 1000; // taming gateway monster not allowed, it would vanish forever!
	gm->mobList->Add(monster);

    // ********* add tricksters
    monster = new BBOSMonster(2,2, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 112;
    monster->cellY = monster->targetCellY = monster->spawnY = 9;
    gm->mobList->Add(monster);
    sprintf_s(monster->uniqueName,"Trickster Minotaur");

    int deathCount = 1;
    fp = fopen("serverdata\\tricksterMino.dat","r");
    if (fp)
    {
        fscanf(fp,"%d",&deathCount);
        fclose(fp);
    }

    if (deathCount < 1)
        deathCount = 1;
    if (deathCount > 3000)
        deathCount = 3000;

    monster->r               = 255;
    monster->g               = 0;
    monster->b               = 0;
    monster->a               = 255;
    monster->sizeCoeff       = 1.0f + 1.0f * deathCount/50;
    monster->health          = 100 * deathCount;
    monster->maxHealth       = 100 * deathCount;
    monster->damageDone      = deathCount/2;
    monster->toHit           = deathCount;
    monster->defense         = deathCount;
    monster->dropAmount      = deathCount;
    monster->magicResistance = 1.0f - 1.0f/deathCount;
	monster->tamingcounter = 1000;						// yeah. no taming Trickster  


    // ********* end of add tricksters

    // add towers/guilds
	sprintf_s(tempText, "Loading Guilds.\n");
	LogOutput("gamelog.txt", tempText);
	fp = fopen("serverdata\\guilds.dat","r");

    if (fp)
    {
        srand(25);
        float dunVers;

        fscanf(fp,"%f\n",&dunVers);
                          
        LoadLineToString(fp,tempText);
        while (!strcmp(tempText,"TOWER"))
        {
            TowerMap * tm = new TowerMap(SPACE_GUILD,"Test Tower",lserver);
            tm->Load(fp,dunVers);
            if (tm->members->ItemsInList() < 2)
                delete tm;
            else
            {
                spaceList->Append(tm);
                // TEST
                if (-1 != tm->enterX && -1 != tm->enterY)
                {
                    int badPlace = TRUE;
                    while (badPlace)
                    {
                        badPlace = FALSE;

                        if (!gm->CanMove(tm->enterX, tm->enterY, tm->enterX, tm->enterY) || 
                             !gm->CanMove(tm->enterX, tm->enterY-1, tm->enterX, tm->enterY-1))
                            badPlace = TRUE; // must be reachable

                        if (tm->enterX < 2 || tm->enterX >= MAP_SIZE_WIDTH-2)
                            badPlace = TRUE; 

                        if (tm->enterY < 2 || tm->enterY >= MAP_SIZE_HEIGHT-2)
                            badPlace = TRUE; // cant't be too close to edge of map.

						for (int i = 0; i < NUM_OF_TOWNS; ++i) // town check
						{
							if (abs(tm->enterX - townList[i].x) < 5 &&
								abs(tm->enterY - townList[i].y) < 5)
								badPlace = TRUE;  // cant be near a town
						}
//						for (int i = 0; i < 9; ++i) // great tree check but lower check should catch these.
//						{
//							if (abs(tm->enterX - greatTreePos[i][0]) < 5 &&
//								abs(tm->enterY - greatTreePos[i][1]) < 5)
//								badPlace = TRUE; // can't be near a great tree
//						}
						// ageing ring checks.
						if (abs(230 - tm->enterX) <5 && abs(53 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(20 - tm->enterX) <5 && abs(23 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(179 - tm->enterX) <5 && abs(164 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(107 - tm->enterX) <5 && abs(162 - tm->enterY) < 5)
							badPlace = TRUE; // can't be byt an ageing ring

						// lab entrance checks these aren't spawned yet so check by hand.
						if (abs(19 - tm->enterX) <5 && abs(5 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(141 - tm->enterX) <5 && abs(86 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(182 - tm->enterX) <5 && abs(26 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(32 - tm->enterX) <5 && abs(237 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(215 - tm->enterX) <5 && abs(222 - tm->enterY) < 5)
							badPlace = TRUE;
						if (abs(123 - tm->enterX) <5 && abs(150 - tm->enterY) < 5)
							badPlace = TRUE; // can't be by a lab entrance.
						// rop tower check. but shouls already be there.
//						if (abs(161 - tm->enterX) <5 && abs(121 - tm->enterY) < 5)
//							badPlace = TRUE; // can't be by the ROP tower.
						// check the spider armies
						if (abs(120 - tm->enterX) <10 && abs(37 - tm->enterY) < 10)
							badPlace = TRUE;
						if (abs(120 - tm->enterX) <10 && abs(27 - tm->enterY) < 10)
							badPlace = TRUE;
						if (abs(96 - tm->enterX) <10 && abs(152 - tm->enterY) < 10)
							badPlace = TRUE;
						if (abs(90 - tm->enterX) <10 && abs(140 - tm->enterY) < 10)
							badPlace = TRUE; // can't be anywhere NEAR the center or spawnpoint of a spider army


						// check moblist (towers/dungeon entrances/gateway monsters, etc.)
						curMobbadplace = (BBOSMob *)gm->mobList->GetFirst(0, 0, 1000);
						while (curMobbadplace)
						{
							// first check it's type
							if (curMobbadplace->WhatAmI()==SMOB_MONSTER || curMobbadplace->WhatAmI() == SMOB_TOWER || curMobbadplace->WhatAmI()==SMOB_TREE || curMobbadplace->WhatAmI()==SMOB_WARP_POINT
								|| curMobbadplace->WhatAmI()==SMOB_ROPENTERANCE) // only cetain mob types are included in this check
							if (abs(tm->enterX - curMobbadplace->cellX) < 5 && abs(tm->enterY - curMobbadplace->cellY) < 5)
							{
								badPlace = TRUE; // too close to a dungeon/tower/gateway monster
								break;           // and stop the loop if we found one
							}
							curMobbadplace=gm->mobList->GetNext(); // go onto th enext mob
						}
						
                        if (badPlace)
                        {
                            int foundValidPoint = FALSE;
                            while (!foundValidPoint)
                            {
                                tm->enterX += (rand() % 3) - 1;
                                tm->enterY += (rand() % 3) - 1;

                                while (tm->enterX < 2)
                                    tm->enterX += 3;
                                while (tm->enterY < 2)
                                    tm->enterY += 3;
                                while (tm->enterX >= MAP_SIZE_WIDTH -2)
                                    tm->enterX -= 3;
                                while (tm->enterY >= MAP_SIZE_HEIGHT-2)
                                    tm->enterY -= 3;

                                if (gm->CanMove(tm->enterX, tm->enterY, tm->enterX, tm->enterY))
                                    foundValidPoint = TRUE;
                            }
                        }
                    }

                    BBOSTower *tower = new BBOSTower(tm->enterX, tm->enterY);
                    gm->mobList->Add(tower);
                    tower->ss = tm;
                    tower->isGuildTower = TRUE;

                    BBOSChest *chest = new BBOSChest(2,2);
                    tm->mobList->Add(chest);

                    BBOSWarpPoint *wp = new BBOSWarpPoint(4,4);
                    wp->targetX      = tm->enterX;
                    wp->targetY      = tm->enterY-1;
                    wp->spaceType    = SPACE_GROUND;
                    wp->spaceSubType = 0;
                    tm->mobList->Add(wp);

                }
            }
            LoadLineToString(fp,tempText);
        }

        fclose(fp);
		srand(timeGetTime());
	}
	sprintf_s(tempText, "Loading Lab1.\n");
	LogOutput("gamelog.txt", tempText);

    LabyrinthMap *lm = new LabyrinthMap(SPACE_LABYRINTH,"Labyrinth",lserver);
    lm->InitNew("dat\\labyrinth-server.raw",64,64,1,1);
    lm->type = REALM_ID_LAB1;
    spaceList->Append(lm);

	sprintf_s(tempText, "Loading Lab2.\n");
	LogOutput("gamelog.txt", tempText);

	LabyrinthMap *lm2 = new LabyrinthMap(SPACE_LABYRINTH, "Deep Labyrinth", lserver);
	lm2->InitNew("dat\\labyrinth2-server.raw", 64, 64, 1, 1);
	lm2->type = REALM_ID_LAB2;
	spaceList->Append(lm2);

	sprintf_s(tempText, "Loading Lab3.\n");
	LogOutput("gamelog.txt", tempText);

	LabyrinthMap *lm3 = new LabyrinthMap(SPACE_LABYRINTH, "Labyrinth Basement", lserver);
	lm3->InitNew("dat\\labyrinth3-server.raw", 64, 64, 1, 1);
	lm3->type = REALM_ID_LAB3;
	spaceList->Append(lm3);

	curGen = new BBOSGenerator(0,0);
    lm->generators->Append(curGen);

	sprintf_s(tempText, "Adding the Three Spiders.\n");
	LogOutput("gamelog.txt", tempText);

    // ******** create Larath (Grey Spidren)
    monster = new BBOSMonster(24,4, NULL);  // make boss spidren
    sprintf_s(monster->uniqueName,"Larath");
    monster->sizeCoeff = 1.7f;

    monster->health = monster->maxHealth = 80000;
    monster->damageDone         = 40;
    monster->toHit              = 90;
    monster->defense            = 135;
    monster->dropAmount         = 20;

    monster->magicResistance = 0.8f;
	monster->tamingcounter = 1000;						// can't tame spider bosses.

    monster->r = 255;
    monster->g = 128;
    monster->b = 00;

    monster->targetCellX = monster->spawnX = monster->cellX = 25;
    monster->targetCellY = monster->spawnY = monster->cellY = 4;

    lm->mobList->Add(monster);

    // ******** create Moultz (Grey Spidren)
    monster = new BBOSMonster(24,4, NULL);  // make boss spidren
    sprintf_s(monster->uniqueName,"Moultz");
    monster->sizeCoeff = 1.8f;

    monster->health = monster->maxHealth = 80000;
    monster->damageDone         = 40;
    monster->toHit              = 100;
    monster->defense            = 145;
    monster->dropAmount         = 20;

    monster->magicResistance = 0.9f;
	monster->tamingcounter = 1000;						// can't tame spider bosses..

    monster->r = 00;
    monster->g = 255;
    monster->b = 128;

    monster->targetCellX = monster->spawnX = monster->cellX = 48;
    monster->targetCellY = monster->spawnY = monster->cellY = 28;

    lm->mobList->Add(monster);

    // ******** create Reorth (Grey Spidren)
    monster = new BBOSMonster(24,4, NULL);  // make boss spidren
    sprintf_s(monster->uniqueName,"Reorth");
    monster->sizeCoeff = 1.9f;

    monster->health = monster->maxHealth = 80000;
    monster->damageDone         = 40;
    monster->toHit              = 110;
    monster->defense            = 155;
    monster->dropAmount         = 20;

    monster->magicResistance = 0.99f;
	monster->tamingcounter = 1000;						// can't tame spider bosses. but you can get a base spidren from a geo.

    monster->r = 128;
    monster->g = 00;
    monster->b = 255;

    monster->targetCellX = monster->spawnX = monster->cellX = 24;
    monster->targetCellY = monster->spawnY = monster->cellY = 56;

    lm->mobList->Add(monster);

/*
    BBOSWarpPoint *wp = new BBOSWarpPoint(28,28);
    wp->targetX      = 0;
    wp->targetY      = 0;
    wp->spaceType    = 10000;
    wp->spaceSubType = 0;
    wp->allCanUse    = FALSE;
    lm->mobList->Add(wp);
    */
	sprintf_s(tempText, "Adding warp points.\n");
	LogOutput("gamelog.txt", tempText);

	AddWarpPair(lm2, 33, 10,
		lm3, 1, 0,
		TRUE);

	AddWarpPair(lm, 28, 28,
		lm2, 1, 1,
		TRUE);

    AddWarpPair(gm, 19, 5,
                    lm, 1,1, 
                    TRUE);

    AddWarpPair(gm, 141, 86,
                    lm, 51,4, 
                    TRUE);

    AddWarpPair(gm, 183, 26,
                    lm, 51,59, 
                    TRUE);

    AddWarpPair(gm, 32, 237,
                    lm, 4, 55, 
                    TRUE);

    AddWarpPair(gm, 215, 222,
                    lm, 22, 31, 
                    TRUE);

    AddWarpPair(gm, 123, 150,
                    lm, 46, 32, 
                    TRUE);
  



	curGen = new BBOSGenerator(0, 0);
	lm2->generators->Append(curGen);
	sprintf_s(tempText, "Adding lab2 bosses.\n");
	LogOutput("gamelog.txt", tempText);

    // add level-1 vlord
    monster = new BBOSMonster(27,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 5;
    monster->cellY = monster->targetCellY = monster->spawnY = 6;
    monster->magicResistance = 0.99f;

    lm2->mobList->Add(monster);

    monster = new BBOSMonster(27,0, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 52;
    monster->cellY = monster->targetCellY = monster->spawnY = 59;
    monster->magicResistance = 0.99f;
	sprintf_s(monster->uniqueName, "Biphomet");
	lm2->mobList->Add(monster);

    monster = new BBOSMonster(27,1, NULL);
    monster->cellX = monster->targetCellX = monster->spawnX = 7;
    monster->cellY = monster->targetCellY = monster->spawnY = 58;
    monster->magicResistance = 0.99f;
	lm2->mobList->Add(monster);
	sprintf_s(tempText, "Populating Lab3.\n");
	LogOutput("gamelog.txt", tempText);

	// lab3 shold have monsters too.
	curGen = new BBOSGenerator(0, 0);
	lm3->generators->Append(curGen);
	// lab3 shold have more monsters.
	curGen = new BBOSGenerator(0, 0);
	lm3->generators->Append(curGen);
	// and some bosses (but we need to decide where the y go first.)
	sprintf_s(tempText, "Adding lab3 bosses.\n");
	LogOutput("gamelog.txt", tempText);

	// add level-3 vlord
	monster = new BBOSMonster(27, 2, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 17;
	monster->cellY = monster->targetCellY = monster->spawnY = 15;
	monster->magicResistance = 0.99f;
	lm3->mobList->Add(monster);
	// add level-4 vlord
	monster = new BBOSMonster(27, 3, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 29;
	monster->cellY = monster->targetCellY = monster->spawnY = 26;
	monster->magicResistance = 0.99f;
	lm3->mobList->Add(monster);
	// add level-5 vlord
	monster = new BBOSMonster(27, 4, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 12;
	monster->cellY = monster->targetCellY = monster->spawnY = 39;
	monster->magicResistance = 0.99f;
	lm3->mobList->Add(monster);
	// add level-3 butterfly
	monster = new BBOSMonster(30, 2, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 20;
	monster->cellY = monster->targetCellY = monster->spawnY = 53;
	monster->magicResistance = 1.00f;  // need blacks to blind/slow/stun them
	lm3->mobList->Add(monster);
	// add level-4 butterfly
	monster = new BBOSMonster(30, 3, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 35;
	monster->cellY = monster->targetCellY = monster->spawnY = 43;
	monster->magicResistance = 1.0f; // need blacks to blind/slow/stun them
	lm3->mobList->Add(monster);
	// add level-5 butterfly
	monster = new BBOSMonster(30, 4, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 53;
	monster->cellY = monster->targetCellY = monster->spawnY = 22;
	monster->magicResistance = 1.00f; // need blacks to blind/slow/stun them
	lm3->mobList->Add(monster);
	// add level-6 butterfly
	monster = new BBOSMonster(30, 5, NULL);
	monster->cellX = monster->targetCellX = monster->spawnX = 42;
	monster->cellY = monster->targetCellY = monster->spawnY = 60;
	monster->magicResistance = 1.00f; // need blacks to blind/slow/stun them
	lm3->mobList->Add(monster);

    // **** done with labyrinth
	sprintf_s(tempText, "Loading testrealm.\n");
	LogOutput("gamelog.txt", tempText);

	rm = new RealmMap(SPACE_REALM, "Test Realm", lserver);
	rm->InitNew("dat\\testrealm-server.raw", 64, 64, 0, 0);
	rm->type = REALM_ID_TEST;
	spaceList->Append(rm);
	sprintf_s(tempText, "TOken init.\n");
	LogOutput("gamelog.txt", tempText);

    tokenMan.Init();
	sprintf_s(tempText, "Loading saved tokens if present.\n");
	LogOutput("gamelog.txt", tempText);
	tokenMan.Load();

    testTime          = timeGetTime();
//	dungeonUpdateTime = timeGetTime();
    lastConnectTime   = timeGetTime();
    lastGraveyardTime = 0;
    errorContactVal   = 0;
    dungeonUpdateTime = tenSecondTime = labyMapTime = 0;
    smasherCounter = 0;
	DelayedStartDone = 0;

    dayTimeCounter    = 0;
//	dayTimeCounter    = 60 * 6 * 3; // start in darkness
	sprintf_s(tempText, "Loading banlists.\n");
	LogOutput("gamelog.txt", tempText);

    banList = new IPBanList();
    uidBanList = new UIDBanList();

    weatherState = 0; // normal, no weather
	sprintf_s(tempText, "Loading quests.\n");
	LogOutput("gamelog.txt", tempText);

    questMan = new QuestManager();
    questMan->Load();
	sprintf_s(tempText, "Loading alternate main.\n");
	LogOutput("gamelog.txt", tempText);

	// add a second normal realm to test with
	GroundMap *gm2 = new GroundMap(SPACE_GROUND, "Test Large Realm", lserver);
//	gm2->InitNew(0, 0, 0, 0); // don't use alternate .raw file to determine movement.
	gm2->InitNew(1, 1, 1, 1); // use alternate .raw file to determine movement.
	spaceList->Append(gm2);
	sprintf_s(tempText, "Done loading.\n");
	LogOutput("gamelog.txt", tempText);
	// add spirit camp to second big map.
	
	ArmyCustomNormal *ac;
	ac = new ArmyCustomNormal(gm2, 120, 37, 120, 27, 8,40,16,1,16,0,17,0);
	gm2->generators->Append(ac);
}

//******************************************************************
BBOServer::~BBOServer()
{
    delete questMan;
	
    delete banList;
    delete uidBanList;

//	delete[] groundInventory;

//	delete mobs;
    delete userMessageList;
    delete incoming;
    delete spaceList;
//	delete dungeonList;

    delete lserver;
    if (IOCPFlag)
        IOCPSocket::StopIOCP();
}


//BBOSAvatar *ServerRandomAvatar;
//int currentAvatarCount = 0;
int lastAvatarCount = 0;

          
//******************************************************************
void BBOServer::Tick(void)
{
    std::vector<TagID> tempReceiptList;
    char tempText[1024];
    HandleMessages();

    DWORD now = timeGetTime();

    DWORD delta;

    if (pleaseKillMe > 0 && pleaseKillMe < 10000)
    {
        LongTime rightNow;

        DWORD age = countDownTime.MinutesDifference(&rightNow);
        if (age != 0)
        {
            --pleaseKillMe;
            countDownTime.SetToNow();

            int ageByFives = pleaseKillMe/5*5;

            if (pleaseKillMe < 10 || pleaseKillMe == ageByFives)
            {
                MessPlayerChatLine chatMess;
                sprintf(&chatMess.text[1],"AUTOMATED SERVER SHUTDOWN IN %d MINUTES.",pleaseKillMe);
                chatMess.text[0] = TEXT_COLOR_DATA;
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess);

                sprintf(&chatMess.text[1],"After maintainence, the server will restart soon.");
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess);
            }
        }
    }

    // Server is going down, time to do things!
    if (0 == pleaseKillMe)
    {
        // save everyone!
        SharedSpace *sp = (SharedSpace *) spaceList->First();
        while (sp)
        {
            BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
            while (curMob)
            {
                if (SMOB_AVATAR == curMob->WhatAmI())
                {
                    BBOSAvatar *curAvatar = (BBOSAvatar *) curMob;
                    curAvatar->SaveAccount();				
                }
                curMob = (BBOSMob *) sp->avatars->Next();
            }


            sp = (SharedSpace *) spaceList->Next();
        }

        // Save tokens
        tokenMan.Tick();
        tokenMan.Save();
		// save unicorn locations
		FILE *fp2 = fopen("serverdata\\unicornData.dat", "w");
		if (fp2)
		{
			for (int i = 0; i < 6; ++i)
			{
				fprintf(fp2, "%d %d\n",
					UnicornLocations[i][0],UnicornLocations[i][1]);
			}

			fclose(fp2);
		}
        // Save ban list
        banList->Save();
        uidBanList->Save();

        // Save out guild towers
        FILE *fp = fopen("serverdata\\guilds.dat","w");
        if (fp)
        {
            fprintf(fp,"%f\n",VERSION_NUMBER);

            SharedSpace *sp = (SharedSpace *) spaceList->First();
            while (sp)
            {
                if (SPACE_GUILD == sp->WhatAmI())
                {
                    TowerMap *dm = (TowerMap *)sp;
                    fprintf(fp,"TOWER\n");

                    dm->ProcessVotes();

                    dm->Save(fp);
                }
                sp = (SharedSpace *) spaceList->Next();
            }

            fprintf(fp,"END\n");
            fclose(fp);
        }
    }

    delta = now - lastConnectTime;

    /*  NO MORE TRYING TO EMAIL FOR ERRORS
    if (delta > 1000 * 60 * 45 && 0 == errorContactVal)	
    {
        errorContactVal = 1;
        SendMailMessage("bigthom@vtext.com","No connections for 45 minutes.");
    }
    if (delta > 1000 * 60 * 55 && 1 == errorContactVal)	
    {
        errorContactVal = 2;
        SendMailMessage("bigthom@vtext.com","No connections for 55 minutes.");
    }
    */

    if (IOCPFlag)
    {
        // ***** bytes transfered
        delta = now - ((IOCPServer *)lserver)->lastTimeUpdate;

        if (0 == ((IOCPServer *)lserver)->lastTimeUpdate  || 
             now < ((IOCPServer *)lserver)->lastTimeUpdate)
        {
            delta = 1000 * 10  + 1;
        }

        // write out every 10 seconds
        if (delta > 1000 * 10)	
        {
            MaintainIncomingAvatars();  // do this every 10 seconds

            ((IOCPServer *)lserver)->lastTimeUpdate = now;

            FILE *source = fopen("byteflow.txt","a");
            
            /* Display operating system-style date and time. */
            _strdate( tempText );
            fprintf(source, "%s, ", tempText );
            _strtime( tempText );
            fprintf(source, "%s, ", tempText );

            fprintf(source,"%d, %d, %d, %d\n", 
                      (int) ((IOCPServer *)lserver)->bytesIn, 
                      (int) ((IOCPServer *)lserver)->bytesOut,
                      (int) ((IOCPServer *)lserver)->bytesIn + 
                      (int) ((IOCPServer *)lserver)->bytesOut,
                      (int) ((IOCPServer *)lserver)->curConnections()); 


            fclose(source);

            ((IOCPServer *)lserver)->bytesIn  = 0;
            ((IOCPServer *)lserver)->bytesOut = 0;

        }


        // ***** messages transfered
        delta = now - ((IOCPServer *)lserver)->lastMessUpdate;

        if (0 == ((IOCPServer *)lserver)->lastMessUpdate  || 
             now < ((IOCPServer *)lserver)->lastMessUpdate || 
             now == ((IOCPServer *)lserver)->lastMessUpdate)
        {
            delta = 1000 * 60 * 10 + 1;
        }

        // write out every 10 minutes
        if (delta > 1000 * 60 * 10)	
        {
            ((IOCPServer *)lserver)->lastMessUpdate = now;

            FILE *source = fopen("messageAmount.txt","a");
            
            /* Display operating system-style date and time. */
            _strdate( tempText );
            fprintf(source, "%s, ", tempText );
            _strtime( tempText );
            fprintf(source, "%s, ", tempText );

            for (int i = 0; i < 256; ++i)
            {
                fprintf(source,"%d, ", 
                      (int) ((IOCPServer *)lserver)->messSent[i]);
               ((IOCPServer *)lserver)->messSent[i] = 0; 
            }

            fprintf(source,"\n");
            fclose(source);

            source = fopen("messageBytes.txt","a");
            
            /* Display operating system-style date and time. */
            _strdate( tempText );
            fprintf(source, "%s, ", tempText );
            _strtime( tempText );
            fprintf(source, "%s, ", tempText );

            for (int i = 0; i < 256; ++i)
            {
                fprintf(source,"%d, ", 
                      (int) ((IOCPServer *)lserver)->messBytes[i]);
                ((IOCPServer *)lserver)->messBytes[i] = 0;
            }

            fprintf(source,"\n");
            fclose(source);



        }

    }
    // tick all of the spaces
    BBOSMob *curMob; //, *tempMob;
	BBOSMob *curMob2;
    lastAvatarCount = 0;
//	int randomAvatarIndex = rand() % (currentAvatarCount+1);
//	currentAvatarCount = 0;

    SharedSpace *sp = (SharedSpace *) spaceList->First();
    while (sp)
    {

        if (SPACE_GROUND == sp->WhatAmI())
        {
            delta = now - lastGraveyardTime;

            if (0 == lastGraveyardTime || now < lastGraveyardTime)
            {
                delta = 1000 * 10 + 1;
            }

            if (delta > 1000 * 10)	
            {
                lastGraveyardTime = now;

                // pet graveyard 90,53 and 71 206
                CheckGraveyard(sp, 90,53);
                CheckGraveyard(sp, 71,206);
            }
        }


        // tick all of this space's Avatars

        int cont;
        cont = FALSE;

        curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
//			if (randomAvatarIndex == currentAvatarCount)
//				ServerRandomAvatar = (BBOSAvatar *)curMob;
			if (curMob->WhatAmI() != SMOB_AVATAR) // sanity check
			{
				curMob = (BBOSMob *)sp->avatars->Next();
				continue;									// it's not really an avatar, so we should give up and go to the next.
			}
            ++lastAvatarCount;
            cont = TRUE;
			if (curMob->isDead != 0)
			{
				cont = TRUE;// debug bait
			}
            curMob->Tick(sp);
            if (curMob->isDead)
            {
				((BBOSAvatar *)curMob)->QuestSpaceChange(NULL, NULL);
				if (((BBOSAvatar *)curMob)->controlledMonster) // if we have a controlled monster, yet we died somehow
				{

					MessAdminMessage adminMess;
					std::vector<TagID> tempReceiptList;
					tempReceiptList.clear();
					tempReceiptList.push_back(((BBOSAvatar *)curMob)->socketIndex); // find avatar's client
					adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
					if (!((BBOSAvatar *)curMob)->followMode)
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList); // point camera back on the poor dead sap
					if (((BBOSAvatar *)curMob)->BeastStat() > 9) // if beastmaster
					{
						// enter follow mode
						((BBOSAvatar *)curMob)->followMode = TRUE;

					}
					
					// kill the monster now
					((BBOSAvatar *)curMob)->controlledMonster->dropAmount = 0; // no loot
					((BBOSAvatar *)curMob)->controlledMonster->type = 25;
					((BBOSAvatar *)curMob)->controlledMonster->subType = 2; //prevent special drops
					((BBOSAvatar *)curMob)->controlledMonster->health = 0;
					((BBOSAvatar *)curMob)->controlledMonster->isDead = TRUE;
					MessMonsterDeath messMD;
					messMD.mobID = (unsigned long)((BBOSAvatar *)curMob)->controlledMonster;
					sp->SendToEveryoneNearBut(0, ((BBOSAvatar *)curMob)->controlledMonster->cellX, ((BBOSAvatar *)curMob)->controlledMonster->cellY,
						sizeof(messMD), (void *)&messMD);
					// remove it fropm old map
					sp->mobList->Remove(((BBOSAvatar *)curMob)->controlledMonster);
					// clean up the dead pet
					BBOSMonster *tempy = ((BBOSAvatar *)curMob)->controlledMonster;
					((BBOSAvatar *)curMob)->controlledMonster = NULL;
					delete tempy;
					tempy = NULL;
				}
				// send death message. hopefully camera will work right for death 
				MessAvatarDeath messAD;
                int fromSocket = ((BBOSAvatar *)curMob)->socketIndex;
                messAD.avatarID = fromSocket;
                sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
                                sizeof(messAD),(void *)&messAD);

                TransferAvatar(FALSE, fromSocket);

                tempReceiptList.clear();
                tempReceiptList.push_back(fromSocket);
                MessChangeMap changeMap;
                changeMap.oldType = sp->WhatAmI(); 
                changeMap.newType = SPACE_GROUND; 
				changeMap.realmID = 0; // make sure it tells the client you are in real normal realm.
                changeMap.sizeX   = MAP_SIZE_WIDTH;
                changeMap.sizeY   = MAP_SIZE_HEIGHT;
				changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
				lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                curMob->cellX = curMob->targetCellX = 
                    ((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->
                             curCharacterIndex].spawnX;
                curMob->cellY = curMob->targetCellY = 
                    ((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->
                             curCharacterIndex].spawnY;
                curMob->isDead = FALSE;
				// gotta get id of the doomkey if you die and somehow have one
				// remove all doomkeys
				InventoryObject *io;
				// destroy doomkeys from workbench
				io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].workbench->objects.First();
				while (io)
				{
					if (INVOBJ_DOOMKEY == io->type)
					{
						((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].workbench->objects.Remove(io);
						delete io;
					}
					io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].workbench->objects.Next();
				}
				// destroy doomkeys from inventory
				io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].inventory->objects.First();
				while (io)
				{
					if (INVOBJ_DOOMKEY == io->type)
					{
						((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].inventory->objects.Remove(io);
						delete io;
					}
					io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].inventory->objects.Next();
				}
				// destroy doomkeys from wielded area
				io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].wield->objects.First();
				while (io)
				{
					if (INVOBJ_DOOMKEY == io->type)
					{
						((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].wield->objects.Remove(io);
						delete io;
					}
					io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].wield->objects.Next();
				}
				// remove all earthkey resumes. if you died outside a geo you really shoudln't have done that.
				// destroy resumes from workbench
				io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].workbench->objects.First();
				while (io)
				{
					if (INVOBJ_EARTHKEY_RESUME == io->type)
					{
						((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].workbench->objects.Remove(io);
						delete io;
					}
					io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].workbench->objects.Next();
				}
				// destroy resumes from inventory
				io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].inventory->objects.First();
				while (io)
				{
					if (INVOBJ_EARTHKEY_RESUME == io->type)
					{
						((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].inventory->objects.Remove(io);
						delete io;
					}
					io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].inventory->objects.Next();
				}
				// destroy resumes from wielded area
				io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].wield->objects.First();
				while (io)
				{
					if (INVOBJ_EARTHKEY_RESUME == io->type)
					{
						((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].wield->objects.Remove(io);
						delete io;
					}
					io = (InventoryObject *)((BBOSAvatar *)curMob)->charInfoArray[((BBOSAvatar *)curMob)->curCharacterIndex].wield->objects.Next();
				}

                TransferAvatar(TRUE, fromSocket);

                for (int i = 0; i < MONSTER_EFFECT_TYPE_NUM; ++i)
                    curMob->magicEffectAmount[i] = 0;

            }

            BBOSAvatar *curAvatar = (BBOSAvatar *) curMob;

            // handle inter-system messages ment for a particular user
            tempReceiptList.clear();
            tempReceiptList.push_back(curAvatar->socketIndex);
            MessInfoText infoText;

            UserMessage *forMe = (UserMessage *) userMessageList->First();
            while (forMe)
            {
                if (!strcmp(forMe->WhoAmI(),curAvatar->name) && 
                     forMe->password[0] && 
                     !strcmp(forMe->password, curAvatar->pass))
                {
                    if (!strcmp(forMe->message, "PAYOUT"))
                    {
                        sprintf_s(infoText.text, "There are other players in me, Thank you!");
                        sp->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        sprintf_s(infoText.text, "There are %ld unopened chests.  You get %ld gold.", forMe->value2, forMe->value3);
                        sp->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        assert(
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money +=
                            forMe->value3;

                        assert(
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

                        if (forMe->value3 > 0)
                        {
                            FILE *source = fopen("chests.txt","a");
                            
                            /* Display operating system-style date and time. */
                            _strdate( tempText );
                            fprintf(source, "%s, ", tempText );
                            _strtime( tempText );
                            fprintf(source, "%s, ", tempText );

                            fprintf(source,"PAYOUT, %s, %d, %d, %d\n",
                                     curAvatar->name,
                                     forMe->value1, forMe->value2, forMe->value3);

                            fclose(source);
                        }
                        userMessageList->Remove(forMe);
                        delete forMe;
                        forMe = (UserMessage *) userMessageList->First();
                    }
                    else
                    {
                        memcpy(infoText.text, forMe->message, MESSINFOTEXTLEN-1);
                        infoText.text[MESSINFOTEXTLEN-1] = 0;
                        sp->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        userMessageList->Remove(forMe);
                        delete forMe;
                        forMe = (UserMessage *) userMessageList->First();
                    }
                }
                else if (forMe->age < timeGetTime())
                {
                    userMessageList->Remove(forMe);
                    delete forMe;

                    forMe = (UserMessage *) userMessageList->First();
                }
                else
                    forMe = (UserMessage *) userMessageList->Next();
            }

            if ((curAvatar->activeCounter >= 12 && ACCOUNT_TYPE_ADMIN != curAvatar->accountType) && (enable_cheating==0))
            {
                // boot this sloth!
                sprintf_s(infoText.text,"%s is inactive; logged off.",
                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                sp->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                    sizeof(infoText),(void *)&infoText);

                HandleKickoff(curAvatar, sp);
            }
            else if (curAvatar->kickOff)
            {
                // boot this duper!
                sprintf_s(infoText.text,"%s is logged-on twice; forced off.",
                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                sp->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                    sizeof(infoText),(void *)&infoText);

                HandleKickoff(curAvatar, sp);
            }
            else if (false && curAvatar->kickMeOffNow) // no more kicking off expired peeps
            {
                // boot this expired account!
                HandleKickoff(curAvatar, sp);
            }

            curMob = (BBOSMob *) sp->avatars->Next();
        }

        if (cont)
        {
            // tick all of this space's Mobs
            int mobTickCount = 0;

            curMob = (BBOSMob *) sp->mobList->GetFirst(0,0,1000);
            while (curMob)
            {
                if (SMOB_MAX <= curMob->WhatAmI() || curMob->WhatAmI() < 0)
                    cont = TRUE;

                curMob->Tick(sp);
                ++mobTickCount;

                int lastBranch = 0;
                if (curMob->isDead && SMOB_MONSTER == curMob->WhatAmI())
                {
                    lastBranch = 1;
					// check for AI
					if (((BBOSMonster *)curMob)->SpecialAI)
					{
						((BBOSMonster *)curMob)->SpecialAI = FALSE; // turn it off
						// then something might happen on my death
						// unicorns WITH the AI on will spawn a new one
						if (((BBOSMonster *)curMob)->type == 31)
						{
							// we spawn a new one elsewhere. not on the edge
							int town = rand() % NUM_OF_TOWNS;
							int randomX = rand() % 10 - 4;
							int randomY = rand() % 10 - 4;
							int newSub = ((BBOSMonster *)curMob)->subType;
							if (newSub > 5)
								newSub = 5;
							if (sp->WhatAmI() == SPACE_GROUND)
							{
								BBOSMonster *monster = new BBOSMonster(31, newSub, NULL);
								monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
								monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
								monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
								bboServer->UnicornLocations[monster->subType][0] = monster->cellX;
								bboServer->UnicornLocations[monster->subType][1] = monster->cellY;
								sp->mobList->Add(monster);
							}
						}
						// no other AIs, yet.
					}
					if (((BBOSMonster *)curMob)->uniqueName[0]  ||
                         (((BBOSMonster *)curMob)->type == 7)     || // if a dragon
                        (((BBOSMonster *)curMob)->type == 16)    || // if a spirit dragon
						(((BBOSMonster *)curMob)->type == 27) || // if a vlord
						(((BBOSMonster *)curMob)->type == 29) || // if a bunny
						(((BBOSMonster *)curMob)->type == 30) || // if a butterfly
						((((BBOSMonster *)curMob)->type == 11) &&
                        (((BBOSMonster *)curMob)->subType == 1))) // if a Dokk's Centurion
						
                    {

                        sprintf_s(tempText,"-----------------------------  %s killed! ",
                            ((BBOSMonster *)curMob)->Name());
                        LogOutput("gamelog.txt", tempText);

                        LongTime lt;
                        lt.value.wHour += 19;
                        if (lt.value.wHour < 24)
                        {
//							lt.value.wHour += 24;
                            lt.value.wDay -= 1;
                        }
                        else
                            lt.value.wHour -= 24;

                        sprintf_s(tempText,"%d/%02d, %d:%02d\n", (int)lt.value.wMonth, (int)lt.value.wDay, 
                                  (int)lt.value.wHour, (int)lt.value.wMinute);
                        LogOutput("gamelog.txt", tempText);


                        // who was there?
                        sprintf_s(tempText,"----- attending: ");
                        LogOutput("gamelog.txt", tempText);

                        BBOSMob *curMob2 = (BBOSMob *) sp->avatars->First();
                        while (curMob2)
                        {
                            if ((curMob2->WhatAmI()==SMOB_AVATAR) && ((abs(curMob->cellX - curMob2->cellX) < 2 &&
                                abs(curMob->cellY - curMob2->cellY) < 2 ) || ((((BBOSAvatar*)curMob2)->BeastStat()>9)&& (SPACE_DUNGEON==sp->WhatAmI()))))
                            {
                                BBOSAvatar *curAvatar2 = (BBOSAvatar *) curMob2;

                                sprintf_s(tempText,"%s, ", 
                                    curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].name);
                                LogOutput("gamelog.txt", tempText);

                                // reward for killing Dokk!
                                if (((BBOSMonster *)curMob)->type == 16 && 
                                ((BBOSMonster *)curMob)->subType == 1 && (SPACE_REALM == sp->WhatAmI() && REALM_ID_SPIRITS == ((RealmMap *)sp)->type)) // if Dokk
                                {
                                    curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].imageFlags |= 
                                      SPECIAL_LOOK_DOKK_KILLER;
                                    MessAvatarStats mStats;
                                    curAvatar2->BuildStatsMessage(&mStats);
                                    sp->SendToEveryoneNearBut(0, curAvatar2->cellX, curAvatar2->cellY,
                                              sizeof(mStats),(void *)&mStats);
                                    sp->avatars->Find(curAvatar2);
                                }
                                // reward for killing anubis!
                                if (((BBOSMonster *)curMob)->type == 20 && 
                                ((BBOSMonster *)curMob)->subType == 0 && (SPACE_REALM == sp->WhatAmI() && REALM_ID_DEAD == ((RealmMap *)sp)->type)) // if anubis
                                {
                                    curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].imageFlags |= 
                                      SPECIAL_LOOK_ANUBIS_KILLER;
                                    MessAvatarStats mStats;
                                    curAvatar2->BuildStatsMessage(&mStats);
                                    sp->SendToEveryoneNearBut(0, curAvatar2->cellX, curAvatar2->cellY,
                                              sizeof(mStats),(void *)&mStats);
                                    sp->avatars->Find(curAvatar2);
                                }
                                // reward for killing overlord!
                                if (((BBOSMonster *)curMob)->type == 7 && 
                                ((BBOSMonster *)curMob)->subType == 4 && (SPACE_REALM == sp->WhatAmI() && REALM_ID_DRAGONS == ((RealmMap *)sp)->type)) // if Dragon Overlord
                                {
                                    if (!strcmp(((BBOSMonster *)curMob)->uniqueName,"Dragon Overlord"))
                                    {
                                        curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].imageFlags |= 
                                          SPECIAL_LOOK_OVERLORD_KILLER;
                                        MessAvatarStats mStats;
                                        curAvatar2->BuildStatsMessage(&mStats);
                                        sp->SendToEveryoneNearBut(0, curAvatar2->cellX, curAvatar2->cellY,
                                                     sizeof(mStats),(void *)&mStats);
                                        sp->avatars->Find(curAvatar2);
                                    }
                                }
								// killed a vagabond? remove all the resumes!
								// remove all earthkey resumes. 
								// destroy resumes from workbench
								if (((BBOSMonster*)curMob)->isVagabond) // lolwtf
								{
									InventoryObject* io;
									io = (InventoryObject*)curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].workbench->objects.First();
									while (io)
									{
										if (INVOBJ_EARTHKEY_RESUME == io->type)
										{
											curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].workbench->objects.Remove(io);
											delete io;
										}
										io = (InventoryObject*)curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].workbench->objects.Next();
									}
									// destroy resumes from inventory
									io = (InventoryObject*)curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].inventory->objects.First();
									while (io)
									{
										if (INVOBJ_EARTHKEY_RESUME == io->type)
										{
											curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].inventory->objects.Remove(io);
											delete io;
										}
										io = (InventoryObject*)curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].inventory->objects.Next();
									}
									// destroy resumes from wielded area
									io = (InventoryObject*)curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].wield->objects.First();
									while (io)
									{
										if (INVOBJ_EARTHKEY_RESUME == io->type)
										{
											curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].wield->objects.Remove(io);
											delete io;
										}
										io = (InventoryObject*)curAvatar2->charInfoArray[curAvatar2->curCharacterIndex].wield->objects.Next();
									}

								}
                            }

                            curMob2 = (BBOSMob *) sp->avatars->Next();
                        }

                        sprintf_s(tempText,"-----\n");
                        LogOutput("gamelog.txt", tempText);

                        // special effect for killing Dokk!
						if (((BBOSMonster *)curMob)->type == 16 &&
							((BBOSMonster *)curMob)->subType == 1 && (SPACE_REALM == sp->WhatAmI() && REALM_ID_SPIRITS == ((RealmMap *)sp)->type)) // if Dokk
						{
							MessGenericEffect messGE;
							messGE.avatarID = -1;
							messGE.mobID = (long)curMob;
							messGE.x = curMob->cellX;
							messGE.y = curMob->cellY;
							messGE.r = 255;
							messGE.g = 0;
							messGE.b = 255;
							messGE.type = 0;  // type of particles
							messGE.timeLen = 20; // in seconds
							sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
								sizeof(messGE), (void *)&messGE);

							// 3 viz ingots for Dokk only in spirit realm.


							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);

						}
						else if (((BBOSMonster *)curMob)->type == 16 && ((BBOSMonster *)curMob)->subType == 1 && (enable_cheating!=0)) // if Dokk and cheating
						{
							// can get the viz anyway

							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);

						}

                        // special effect for killing Anubis!
                        if (((BBOSMonster *)curMob)->type == 20 && 
                        ((BBOSMonster *)curMob)->subType == 0 &&  (SPACE_REALM == sp->WhatAmI() && REALM_ID_DEAD == ((RealmMap *)sp)->type))
                        {
                            MessGenericEffect messGE;
                            messGE.avatarID = -1;
                            messGE.mobID    = (long)curMob;
                            messGE.x        = curMob->cellX;
                            messGE.y        = curMob->cellY;
                            messGE.r        = 255;
                            messGE.g        = 210;
                            messGE.b        = 0;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 20; // in seconds
                            sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
                                              sizeof(messGE),(void *)&messGE);
                        }

                        // special effect for killing overlord!
                        if (((BBOSMonster *)curMob)->type == 7 && 
                        ((BBOSMonster *)curMob)->subType == 4 && (SPACE_REALM == sp->WhatAmI() && REALM_ID_DRAGONS == ((RealmMap *)sp)->type))
                        {
                            MessGenericEffect messGE;
                            messGE.avatarID = -1;
                            messGE.mobID    = (long)curMob;
                            messGE.x        = curMob->cellX;
                            messGE.y        = curMob->cellY;
                            messGE.r        = 200;
                            messGE.g        = 200;
                            messGE.b        = 255;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 20; // in seconds
                            sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
                                              sizeof(messGE),(void *)&messGE);
                        }

                        // special effect for killing spidren bosses!
                        if (((BBOSMonster *)curMob)->type == 24 && 
                        ((BBOSMonster *)curMob)->subType == 4 && (SPACE_LABYRINTH == sp->WhatAmI()))
                        {
                            MessGenericEffect messGE;
                            messGE.avatarID = -1;
                            messGE.mobID    = (long)curMob;
                            messGE.x        = curMob->cellX;
                            messGE.y        = curMob->cellY;
                            messGE.r        = 100;
                            messGE.g        = 255;
                            messGE.b        = 100;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 20; // in seconds
                            sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
                                              sizeof(messGE),(void *)&messGE);
                        }

						// special effect for killing vlords
						if (((BBOSMonster *)curMob)->type == 27)
						{
							MessGenericEffect messGE;
							messGE.avatarID = -1;
							messGE.mobID = (long)curMob;
							messGE.x = curMob->cellX;
							messGE.y = curMob->cellY;
							messGE.r = 255;
							messGE.g = 100;
							messGE.b = 200;
							messGE.type = 0;  // type of particles
							messGE.timeLen = 20; // in seconds
							sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
								sizeof(messGE), (void *)&messGE);
						}
						// special effect for killing butterfly bosses
						if (((BBOSMonster *)curMob)->type == 30 && (SPACE_LABYRINTH == sp->WhatAmI()) && ((BBOSMonster *)curMob)->subType > 1)
						{
							MessGenericEffect messGE;
							messGE.avatarID = -1;
							messGE.mobID = (long)curMob;
							messGE.x = curMob->cellX;
							messGE.y = curMob->cellY;
							messGE.r = 100;
							messGE.g = 100;
							messGE.b = 255;
							messGE.type = 0;  // type of particles
							messGE.timeLen = 20; // in seconds
							sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
								sizeof(messGE), (void *)&messGE);
						}

						if (((BBOSMonster *)curMob)->type == 11 &&
							(SPACE_REALM == sp->WhatAmI() && REALM_ID_SPIRITS == ((RealmMap *)sp)->type) && // must be in spirit realm
							((BBOSMonster *)curMob)->subType == 1) // if centurion
						{
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);
						}
						else if (((BBOSMonster *)curMob)->type == 11 && (enable_cheating!=0) && // if cheating
							((BBOSMonster *)curMob)->subType == 1) // if centurion
						{
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 1);
						}

                        if (((BBOSMonster *)curMob)->type == 16 &&
							(SPACE_REALM == sp->WhatAmI() && REALM_ID_SPIRITS == ((RealmMap *)sp)->type) ) // must be in spirit realm
						 // if spirit dragon
                        {
                            // small chance of dropping viz ingot
                            if (!(rand() % 20))
                                sp->DoMonsterDropSpecial((BBOSMonster *)curMob,1);
                        }
                    }

					if (((BBOSMonster *)curMob)->type == 23 &&
						(SPACE_REALM == sp->WhatAmI() && REALM_ID_DRAGONS == ((RealmMap *)sp)->type) && //must be in dragon realm
						((BBOSMonster *)curMob)->subType == 2) // if Dusk Wurm
					{
						sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 6); // 1 black dust
					}
					else if (((BBOSMonster *)curMob)->type == 23 && (enable_cheating!=0) && // if cheating
						((BBOSMonster *)curMob)->subType == 2) // if Dusk Wurm
					{
						sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 6); // 1 black dust
					}
					// valentine hearts drop a single random dust
					if (((BBOSMonster *)curMob)->type == 33)
					{
						int DustType = rand()%7; // random dust type
						if (DustType == 0)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 0); // blue dust
						if (DustType == 1)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 3); // red
						if (DustType == 2)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 4); // white
						if (DustType == 3)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 5); // green
						if (DustType == 4)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 6); // black
						if (DustType == 5)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 34); // gold
						if (DustType == 6)
							sp->DoMonsterDropSpecial((BBOSMonster *)curMob, 23); // silver
					}
                    MessMonsterDeath messMD;
                    messMD.mobID = (unsigned long) curMob;
                    sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
                                    sizeof(messMD),(void *)&messMD);

                    sp->mobList->Remove(curMob);
					// check if i had a controlling bestmaster
					if (((BBOSMonster *)curMob)->controllingAvatar && (((BBOSMonster *)curMob)->controllingAvatar->BeastStat() > 9)) // beastmaster pet
					{
						// unlink it now before
						((BBOSMonster *)curMob)->controllingAvatar->SaveAccount();
						((BBOSMonster *)curMob)->controllingAvatar->controlledMonster = NULL;
						// update the client omg
						MessAvatarAppear mAppear;
						mAppear.avatarID = ((BBOSMonster *)curMob)->controllingAvatar->socketIndex;
						mAppear.x = ((BBOSMonster *)curMob)->controllingAvatar->cellX;
						mAppear.y = ((BBOSMonster *)curMob)->controllingAvatar->cellY;
						tempReceiptList.clear();
						tempReceiptList.push_back(((BBOSMonster *)curMob)->controllingAvatar->socketIndex);
						sp->lserver->SendMsg(sizeof(mAppear), &mAppear, 0, &tempReceiptList);
						((BBOSMonster *)curMob)->controllingAvatar->UpdateClient(sp, TRUE);
					}

    //				Inventory *inv = &(groundInventory[128*curMob->cellY + curMob->cellX]);

                    // drop goodies!
                    BBOSMonster *monster = (BBOSMonster *) curMob;

					if ((monster->TotalbombDamage >= monster->maxHealth) // if total damage by bombs >= max health.
						&& (rand() % 10 == 1)						// and 10% change passes
						&& (sp->WhatAmI() != SPACE_GROUND)			// AND not in normal realm.
						&& (sp->WhatAmI() != SPACE_DUNGEON))		// AND not in a dungeon.
						sp->DoMonsterDropSpecial(monster, 33);		// monster drops a merchant in a box.
                    if (monster->type == 25 && monster->dropAmount <= 0) // vanished vamp
                        ;
                    else
                        sp->DoMonsterDrop(monster);

                    if (monster->bane) // check for dust drops
                    {
                        SharedSpace *tempss;
                        BBOSAvatar *curAvatar = FindAvatar(monster->bane,&tempss);
                        if (curAvatar)
                        {
                            int bestSkill = 0;
                            Inventory *inv = (curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills);
                            InventoryObject *io = (InventoryObject *) inv->objects.First();
                            while(io)
                            {
                                if (!strcmp("Dodging",io->WhoAmI()))
                                {
                                    InvSkill *skillInfo = (InvSkill *) io->extra;
                                    if (bestSkill < skillInfo->skillLevel)
                                        bestSkill = skillInfo->skillLevel;
                                }
                                io = (InventoryObject *) inv->objects.Next();
                            }

                            if (bestSkill < monster->toHit + 8 && 44 == (rand() % 140))
                            {
                                sp->DoMonsterDropSpecial(monster,0);
                            }
							// now check beastmasters for gold dust drop
							bestSkill = 0;
							inv = (curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills);
							io = (InventoryObject *)inv->objects.First();
							while (io)
							{
								if (!strcmp("Pet Energy", io->WhoAmI()))
								{
									InvSkill *skillInfo = (InvSkill *)io->extra;
									if (bestSkill < skillInfo->skillLevel)
										bestSkill = skillInfo->skillLevel;
								}
								io = (InventoryObject *)inv->objects.Next();
							}

							if ((curAvatar->BeastStat()>9) && (bestSkill>0) && (bestSkill < monster->toHit + 8) && (14 == (rand() % 20))) // you have to have pet energy to get a gold dust, and you cant overlevel too much.
							{
								sp->DoMonsterDropSpecial(monster, 34);
							}

                        }
                    }

                    // if in the realm of the dead, small chance of dropping stuff
                    if (SPACE_REALM == sp->WhatAmI() && REALM_ID_DEAD == ((RealmMap *)sp)->type)
                    {
                        if (44 == (rand() % 100))
                        {
                            int dropType = (rand() % 3) +2; // undead totem, red, or white dust.
                            sp->DoMonsterDropSpecial(monster,dropType);
                        }

                        // if wraith or wraith mistress
                        if (18 == monster->type && monster->subType >= 2 && 4 == (rand() % 30))
                        {
                            sp->DoMonsterDropSpecial(monster,2);
                        }

                    }

                    // if in the realm of dragons, small chance of dropping stuff
                    if (SPACE_REALM == sp->WhatAmI() && REALM_ID_DRAGONS == ((RealmMap *)sp)->type)
                    {
                        if (((BBOSMonster *)curMob)->type == 7) // if dragon
                        {
                            if (3 == (rand() % 40))
                            {
                                int dropType = (rand() % 4);
                                switch(dropType)
                                {
                                case 0:
                                    sp->DoMonsterDropSpecial(monster, 0); // blue dust
                                    break;
                                case 1:
                                    sp->DoMonsterDropSpecial(monster, 5); // green dust 
                                    break;
                                case 2:
                                    sp->DoMonsterDropSpecial(monster, 10); // oak staff
                                    break;
                                case 3:
                                    sp->DoMonsterDropSpecial(monster, 11); // dragon totem
                                    break;
                                }
                            }
                        }
                        else if (((BBOSMonster *)curMob)->type == 22) // if orc
                        {
                            if (((BBOSMonster *)curMob)->subType == 4) // if orc champion
                            {
                                sp->DoMonsterDropSpecial(monster, 12); // azides brick
                                sp->DoMonsterDropSpecial(monster, 13); // nitrate brick
                                sp->DoMonsterDropSpecial(monster, 14); // cotton rope
                            }
                            else if (3 == (rand() % 60))
                            {
                                int dropType = (rand() % 2);
                                switch(dropType)
                                {
                                case 0:
                                    sp->DoMonsterDropSpecial(monster, 12); // azides brick
                                    break;
                                case 1:
                                    sp->DoMonsterDropSpecial(monster, 13); // nitrate brick
                                    break;
                                }
                            }
                        }
                        else if (((BBOSMonster *)curMob)->type == 23) // if wurm
                        {
                            if (3 == (rand() % 60))
                            {
                                sp->DoMonsterDropSpecial(monster, 6); // black dust
                            }
                        }
                    }
					// Dragon Overlords now have Sun totems too so people will want to kill them with help.
					if (((BBOSMonster *)curMob)->type == 7 && // dragon
						((BBOSMonster *)curMob)->subType == 4 &&  //overloard
						((BBOSMonster *)curMob)->uniqueName != "Dragon Chaplain" && // not Chappy
						((BBOSMonster *)curMob)->uniqueName != "Dragon Archmage") // not Archamge
						sp->DoMonsterDropSpecial(monster, 30); // sun totem
                    // if in the lab, small chance of dropping stuff
                    if (SPACE_LABYRINTH == sp->WhatAmI())
                    {
                        if (48 == (rand() % 150))
                        {
                            sp->DoMonsterDropSpecial(monster,15); // chitty totem
						}
						if (68 == (rand() % 150))
						{
							sp->DoMonsterDropSpecial(monster, 16); // rainbow egg
						}

						if (REALM_ID_LAB1 < ((LabyrinthMap *)sp)->type)
						{
							if (48 == (rand() % 300))
							{
								sp->DoMonsterDropSpecial(monster, 28); // rare moon totem
							}
							if (48 == (rand() % 450))
							{
								sp->DoMonsterDropSpecial(monster, 22); // rare random mal
							}
							if (68 == (rand() % 300)) // tungsten
							{
								sp->DoMonsterDropSpecial(monster, 24);
							}
							if (68 == (rand() % 600)) // ludicrously rare titanium drop.
							{
								sp->DoMonsterDropSpecial(monster, 25);
							}
						}
						if (REALM_ID_LAB2 < ((LabyrinthMap *)sp)->type) // only in lab 3 for these
						{
							if (48 == (rand() % 300))
							{
								sp->DoMonsterDropSpecial(monster, 29); // rare star totem (still faster to do quest for em)
							}
							if (38 == (rand() % 50)) // MUCH better chance to farm tungsten here.
							{
								sp->DoMonsterDropSpecial(monster, 24);
							}
							if (68 == (rand() % 100)) // better chance to farm titanium here. about equal to silvers from above.
							{
								sp->DoMonsterDropSpecial(monster, 25);
							}
							if (68 == (rand() % 1200)) // ludicrously rare random chrome drop.
							{
								sp->DoMonsterDropSpecial(monster, 27);
							}
						}

                    }
					// if in a dungeon, reduce monster counter if it's greater than 1.
					if (SPACE_DUNGEON == sp->WhatAmI() &&
						(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY)) // it's a geo
					{
						if (((DungeonMap *)sp)->MonsterCount > 0)
						{
							((DungeonMap *)sp)->MonsterCount--;
						}
					}
						// if in a dungeon, place a grave
                    if (SPACE_DUNGEON == sp->WhatAmI() && !monster->dontRespawn &&
                         !(((DungeonMap *) sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && !(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
                    {
                        if (monster->type == 11) // if Dokk's centurion
                            ;
                        else
                        {
                            BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                     monster->type, monster->subType,
                                     monster->cellX, monster->cellY);
                            mg->isWandering = monster->isWandering;
                            sp->mobList->Add(mg);

                            // if in a dungeon, possibly drop explosives
                            if (5 == (rand() % 7))
                            {
                                int dropType = monster->toHit/4 + 7;
                                if (dropType < 7)
                                    dropType = 7;
                                if (dropType > 9)
                                    dropType = 9;
                                sp->DoMonsterDropSpecial(monster,dropType);
                            }
                        }
                    }
					else if (((BBOSMonster *)curMob)->type == 25 && ((BBOSMonster *)curMob)->dropAmount > 0) // killed vamp
					{
						int baseVal = ((BBOSMonster *)curMob)->subType;

						for (int i = 0; i < baseVal + 1; ++i)
						{
							// chance of dropping a scroll
							if ((rand() % 30) > 30 - (2 + baseVal * 2))
							{
								int dropType = 19 + (rand() % 2);
								sp->DoMonsterDropSpecial(monster, dropType);
							}

							// chance of dropping a lava totem
							if ((rand() % 100) > 100 - (1 + baseVal * 2))
							{
								sp->DoMonsterDropSpecial(monster, 18);
							}
						}
					}
//					else if (((BBOSMonster *)curMob)->type == 31 && !monster->dontRespawn) // if unicorn that's not tamed
//					{
//						
//						int randomX = 1 + rand() % 254;
//						int randomY = 1 + rand() % 254;
//						bool badspawn = TRUE;
//						while (badspawn)
//						{
//							badspawn = !sp->CanMove(randomX, randomY, randomX, randomY); // turns false if spawnw location is valid
//							if (badspawn)													// if still bad
//							{
//								// generate new coords
//								randomX = 1 + rand() % 254;
//								randomY = 1 + rand() % 254;
//							}
//						}
//						// add new unicorn in random square
//						BBOSMonster *newmonster = new BBOSMonster(31, monster->subType, NULL);
//						newmonster->cellX = newmonster->targetCellX = newmonster->spawnX = randomX;
//						newmonster->cellY = newmonster->targetCellY = newmonster->spawnY = randomY;
//						newmonster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
//						sp->mobList->Add(monster);
//
//					}
					else if (((BBOSMonster *)curMob)->type == 16 && ((BBOSMonster *)curMob)->subType == 1 && !monster->dontRespawn) // if Dokk
					{
						BBOSMonsterGrave *mg = new BBOSMonsterGrave(
							monster->type, monster->subType,
							monster->spawnX, monster->spawnY);
						mg->isWandering = FALSE;
						sp->mobList->Add(mg);
					}
					else if (((BBOSMonster *)curMob)->type == 11 &&
                        ((BBOSMonster *)curMob)->subType == 1 && !monster->dontRespawn) // if Dokk's centurion
                    {
                        BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                monster->type, monster->subType,
                                 monster->spawnX, monster->spawnY);
                        mg->isWandering = FALSE;
                        sp->mobList->Add(mg);
                    }
                    else if (!strcmp(((BBOSMonster *)curMob)->uniqueName,"Trickster Minotaur"))
                    {
                        BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                monster->type, monster->subType,
                                 monster->spawnX, monster->spawnY);
                        mg->isWandering = FALSE;
                        sp->mobList->Add(mg);

                        int deathCount = monster->toHit + 1;
                        if (deathCount < 1)
                            deathCount = 1;
                        if (deathCount > 3000)
                            deathCount = 3000;

                        strncpy(mg->uniqueName, monster->uniqueName, 32);
                        mg->r              = 255;                  
                        mg->g              = 0;                    
                        mg->b              = 0;                    
                        mg->a              = 255;                  
                        mg->sizeCoeff      = 1.0f + 1.0f * deathCount/50; 
                        mg->health         = 100 * deathCount;     
                        mg->maxHealth      = 100 * deathCount;     
                        mg->damageDone     = deathCount/2;         
                        mg->toHit          = deathCount;           
                        mg->defense        = deathCount;           
                        mg->dropAmount     = deathCount;           
                        mg->magicResistance= 1.0f - 1.0f/deathCount;

                        mg->spawnTime = timeGetTime() + 1000 * deathCount;


                        FILE *fp = fopen("serverdata\\tricksterMino.dat","w");
                        if (fp)
                        {
                            fprintf(fp,"%d",monster->toHit + 1);
                            fclose(fp);
                        }

                    }
					else if (((BBOSMonster *)curMob)->type == 28 && !monster->dontRespawn && 
						(SPACE_LABYRINTH == sp->WhatAmI() && REALM_ID_LAB2 <= ((LabyrinthMap *)sp)->type) && // only in deeper lab and basement
						!(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && !(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
					{	// lizard men
						if (((BBOSMonster *)curMob)->subType < 3 && rand() % 400 == 58) 
						{
							sp->DoMonsterDropSpecial(monster, 23);
						}
						else if (rand() % 200 == 53) 
						{
							sp->DoMonsterDropSpecial(monster, 23);
						}
					}
					else if (((BBOSMonster *)curMob)->type == 28 && (enable_cheating!=0)) // in cheaters mode, they can drop silvers anywhere.
					{	// lizard men
						if (((BBOSMonster *)curMob)->subType < 3 && rand() % 400 == 58) 
						{
							sp->DoMonsterDropSpecial(monster, 23);
						}
						else if (rand() % 200 == 53) 
						{
							sp->DoMonsterDropSpecial(monster, 23);
						}
					}
					else if (((BBOSMonster *)curMob)->type == 29 && !monster->dontRespawn && // bunnies :)
					(SPACE_LABYRINTH == sp->WhatAmI() && REALM_ID_LAB3 == ((LabyrinthMap *)sp)->type) && // only in basement
					!(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && !(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
					{
						if (((BBOSMonster *)curMob)->subType < 3 && rand() % 50 == 28)
						{
							sp->DoMonsterDropSpecial(monster, 34);
						}
						else if (rand() % 20 == 10)
						{
							sp->DoMonsterDropSpecial(monster, 34);
						}
						// they can also drop random eggs
						// tougher bunnies have a bettter chance to drop eggs
						for (int i = 0; i < 1 + ((BBOSMonster *)curMob)->subType * 2; ++i)
						{
							if (rand() % 5 == 1)
							{  // 20 % chance
								sp->DoMonsterDropSpecial(monster, 35);
							}
						}
						// they can also now drop onyx staffs
						// tougher bunnies have a better chance to drop staff
						for (int i = 0; i < 1 + ((BBOSMonster *)curMob)->subType * 2; ++i)
						{
							if (rand() % 100 == 1)
							{  // 1% chance for now 
								sp->DoMonsterDropSpecial(monster, 21);
							}
						}
					}
					else if (((BBOSMonster *)curMob)->type == 29 && (enable_cheating != 0)) // in cheaters mode they can drop their stuff in geos
					{
						if (((BBOSMonster *)curMob)->subType < 3 && rand() % 50 == 28)
						{
							sp->DoMonsterDropSpecial(monster, 34);
						}
						else if (rand() % 20 == 10)
						{
							sp->DoMonsterDropSpecial(monster, 34);
						}
						// they can also drop random eggs
						// tougher bunnies have a bettter chance to drop eggs
						for (int i = 0; i < 1 + ((BBOSMonster *)curMob)->subType * 2; ++i)
						{
							if (rand() % 5 == 1)
							{  // 20 % chance
								sp->DoMonsterDropSpecial(monster, 35);
							}
						}
						// they can also now drop onyx staffs
						// tougher bunnies have a better chance to drop staff
						for (int i = 0; i < 1 + ((BBOSMonster *)curMob)->subType * 2; ++i)
						{
							if (rand() % 100 == 1)
							{  // 1% chance for now 
								sp->DoMonsterDropSpecial(monster, 21);
							}
						}
					}
					else if (((BBOSMonster *)curMob)->type == 30 && !monster->dontRespawn && // butterflies :)
						(SPACE_LABYRINTH == sp->WhatAmI() && REALM_ID_LAB3 == ((LabyrinthMap *)sp)->type) && // only in basement
						!(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && !(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
					{
						if (((BBOSMonster *)curMob)->subType > 1) // only 2 and higher are boss
						{
							// boss butterflies respawn
							BBOSMonsterGrave *mg = new BBOSMonsterGrave(
								monster->type, monster->subType,
								monster->spawnX, monster->spawnY);
							mg->isWandering = FALSE;
							sp->mobList->Add(mg);

							sp->DoMonsterDropSpecial(monster, 27); // chrome
							sp->DoMonsterDropSpecial(monster, 27); // two players to kill, and three times as long a wait
							sp->DoMonsterDropSpecial(monster, 27); // so 3 ingots per player, assuming two are used.
							sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							if (((BBOSMonster *)curMob)->subType > 2)
							{
								// drop some more probably need four players, and this one is 2 hours
								// drop 10 more
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							}
							if (((BBOSMonster *)curMob)->subType > 3)
							{
								// six players, 2.5 hours
								// drop 14 MORE
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							}
							if (((BBOSMonster *)curMob)->subType > 4)
							{
								// this one probably needs 8 players at least, and around 3 hours
								// drop 18 more
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							}
						}
					}
					else if (((BBOSMonster *)curMob)->type == 30 && (enable_cheating!=0)) // in cheaters mode they have their special drops in geos
					{
						if (((BBOSMonster *)curMob)->subType > 1) // only 2 and higher are boss
						{
							sp->DoMonsterDropSpecial(monster, 27); // chrome
							sp->DoMonsterDropSpecial(monster, 27); // two players to kill, and three times as long a wait
							sp->DoMonsterDropSpecial(monster, 27); // so 3 ingots per player, assuming two are used.
							sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							if (((BBOSMonster *)curMob)->subType > 2)
							{
								// drop some more probably need four players, and this one is 2 hours
								// drop 10 more
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							}
							if (((BBOSMonster *)curMob)->subType > 3)
							{
								// six players, 2.5 hours
								// drop 14 MORE
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							}
							if (((BBOSMonster *)curMob)->subType > 4)
							{
								// this one probably needs 8 players at least, and around 3 hours
								// drop 18 more
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 27);
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
								sp->DoMonsterDropSpecial(monster, 30); // and some sun totems
							}
						}
					}
					else if (((BBOSMonster *)curMob)->type == 24 && ((BBOSMonster *)curMob)->subType == 4 && !monster->dontRespawn) // if spidren boss (these never dropped in geos anyway, so no need to touch it)
                    {
                        BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                monster->type, monster->subType,
                                 monster->spawnX, monster->spawnY);
                        mg->isWandering = FALSE;
                        sp->mobList->Add(mg);

                        strncpy(mg->uniqueName, monster->uniqueName, 32);
                        mg->r              = monster->r;                  
                        mg->g              = monster->g;                    
                        mg->b              = monster->b;                    
                        mg->a              = monster->a;                  
                        mg->sizeCoeff      = monster->sizeCoeff; 
                        mg->health         = monster->maxHealth;     
                        mg->maxHealth      = monster->maxHealth;     
                        mg->damageDone     = monster->damageDone;         
                        mg->toHit          = monster->toHit;           
                        mg->defense        = monster->defense;           
                        mg->dropAmount     = monster->dropAmount;           
                        mg->magicResistance= monster->magicResistance;

                        mg->spawnTime = timeGetTime() + 1000 * 60 * 60 * 4;
//						mg->spawnTime = timeGetTime() + 1000 * 60;	// TEST!!!!

                        if (!strcmp(((BBOSMonster *)curMob)->uniqueName,"Larath"))
                            sp->DoMonsterDropSpecial(monster,17);
                        else if (!strcmp(((BBOSMonster *)curMob)->uniqueName,"Moultz"))
                        {
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                        }
                        else if (!strcmp(((BBOSMonster *)curMob)->uniqueName,"Reorth"))
                        {
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                            sp->DoMonsterDropSpecial(monster,17);
                        }
                    }
                    else if (((BBOSMonster *)curMob)->type == 27 && !monster->dontRespawn ) // if vlord
                    {
                        BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                monster->type, monster->subType,
                                 monster->spawnX, monster->spawnY);
                        mg->isWandering = FALSE;
                        sp->mobList->Add(mg);

                        mg->magicResistance= monster->magicResistance;

                        mg->spawnTime = timeGetTime() + 1000 * 60 * 60 * 4;
//						mg->spawnTime = timeGetTime() + 1000 * 60;	// TEST!!!!

//						if (monster->isVagabond) { // old check
						if ((sp->WhatAmI()==SPACE_DUNGEON) && (enable_cheating==0)) { // if we are in a dungeon and not cheating
								if( monster->toHit > 250 ) // and monster is high enough level.
                                sp->DoMonsterDropSpecial(monster,22); // you are allowed to get a Malignant ingot from the lord.
                        }
                        else {  // drop full ingot amount
                            for (int i = 0; i < 1 + ((BBOSMonster *)curMob)->subType * 2; ++i)
                                sp->DoMonsterDropSpecial(monster,22);
                        }
						for (int i = 0; i < 1 + ((BBOSMonster *)curMob)->subType; ++i) // drop some star totems too wherver they are.
						{
							sp->DoMonsterDropSpecial(monster, 29);
							sp->DoMonsterDropSpecial(monster, 21);  // and onyx staffs

						}
                    }
					if ((enable_cheating != 0) && (((BBOSMonster *)curMob)->type == 32)) // reindeer in cheaters mode
					{
						if (rand() % 10 == 1) // chance to drop reindeer totem
							sp->DoMonsterDropSpecial(monster, 31);
					}
                    if (SPACE_GROUND == sp->WhatAmI() && 
                         ((BBOSMonster *)curMob)->type == 19 && 
                        ((BBOSMonster *)curMob)->subType == 0) // if bone warrior in normal world
                    {
                        BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                monster->type, monster->subType,
                                 monster->spawnX, monster->spawnY);
                        mg->isWandering = FALSE;
                        sp->mobList->Add(mg);
						// locate controlled monsters in the square, and teleport their avatars there
						curMob2 = (BBOSMob *)sp->mobList->GetFirst(0, 0, 1000); // gotta check them all
						while (curMob2)
						{   
							if ((curMob2->cellX == curMob->cellX) // same x coordinate
								&& (curMob2->cellY == curMob->cellY) // and same y coordinate
								&& (SMOB_MONSTER == curMob2->WhatAmI()) // actually a monster
								&& (((BBOSMonster *)curMob2)->controllingAvatar)) // AND it has a controlling avatar. safe due to short circuit evaluation above
							{
								// THEN we set it's coords to here so the gate handler will pick it up
								((BBOSMonster *)curMob2)->controllingAvatar->cellX = curMob2->cellX;
								((BBOSMonster *)curMob2)->controllingAvatar->cellY = curMob2->cellY;
							}
							curMob2 = (BBOSMob *)sp->mobList->GetNext();				// go on to the next monster.
						}
						HandleDeadGate(curMob->cellX, curMob->cellY, sp);

                    }

                    if (SPACE_GROUND == sp->WhatAmI() && 
                         ((BBOSMonster *)curMob)->type == 9 && 
                        ((BBOSMonster *)curMob)->subType == 0 && !monster->dontRespawn) // if spirit vision in normal world
                    {
                        BBOSMonsterGrave *mg = new BBOSMonsterGrave(
                                monster->type, monster->subType,
                                 monster->spawnX, monster->spawnY);
                        mg->isWandering = FALSE;
                        sp->mobList->Add(mg);
						// locate controlled monsters in the square, and teleport their avatars there
						curMob2 = (BBOSMob *)sp->mobList->GetFirst(0, 0, 1000); // gotta check them all
						while (curMob2)
						{
							if ((curMob2->cellX == curMob->cellX) // same x coordinate
								&& (curMob2->cellY == curMob->cellY) // AND same y coordinate
								&& (SMOB_MONSTER == curMob2->WhatAmI()) // actually a monster
								&& (((BBOSMonster *)curMob2)->controllingAvatar)) // AND it has a controlling avatar. safe due to short circuit evaluation above
							{
								// THEN we set it's coords to here so the gate handler will pick it up
								((BBOSMonster *)curMob2)->controllingAvatar->cellX = curMob2->cellX;
								((BBOSMonster *)curMob2)->controllingAvatar->cellY = curMob2->cellY;
							}
							curMob2 = (BBOSMob *)sp->mobList->GetNext();				// go on to the next monster.
						}

                        HandleSpiritGate(curMob->cellX, curMob->cellY, sp);

                    }
                    delete curMob;
                }
                else if (curMob->isDead && SMOB_MONSTER_GRAVE == curMob->WhatAmI())
                {
                    lastBranch = 2;
					// debugging crap to figure out wher eit's crashing
					sprintf_s(tempText, "Grave respawn\n");
					LogOutput("gamelog.txt", tempText);
					BBOSMonsterGrave *monsterGrave = (BBOSMonsterGrave *) curMob;
					sprintf_s(tempText, "Grave data grabbed.\n");
					LogOutput("gamelog.txt", tempText);

                    BBOSMonster *monster = new BBOSMonster(
                                      monsterGrave->type, monsterGrave->subType, NULL);
					sprintf_s(tempText, "New monster created.\n");
					LogOutput("gamelog.txt", tempText);

					monster->cellX = monsterGrave->spawnX;
                    monster->cellY = monsterGrave->spawnY;
                    monster->targetCellX = monsterGrave->spawnX;
                    monster->targetCellY = monsterGrave->spawnY;
                    monster->spawnX = monsterGrave->spawnX;
                    monster->spawnY = monsterGrave->spawnY;
                    monster->isWandering = monsterGrave->isWandering;
					sprintf_s(tempText, "Monster location set.\n");
					LogOutput("gamelog.txt", tempText);

					sp->mobList->Add(monster);
					sprintf_s(tempText, "Monster added..\n");
					LogOutput("gamelog.txt", tempText);

					sp->mobList->Remove(curMob);
					sprintf_s(tempText, "Grave removed.\n");
					LogOutput("gamelog.txt", tempText);

                    if (!monsterGrave->uniqueName[0])
                    {
                        MessMobAppear mobAppear;
                        mobAppear.mobID = (unsigned long) monster;
                        mobAppear.type = monster->WhatAmI();
                        mobAppear.monsterType = monster->type;
                        mobAppear.subType = monster->subType;

                        if(SPACE_DUNGEON == sp->WhatAmI())
                        {
                            mobAppear.staticMonsterFlag = FALSE;
                            if (!monster->isWandering)
                                mobAppear.staticMonsterFlag = TRUE;
                        }

                        mobAppear.x = monster->cellX;
                        mobAppear.y = monster->cellY;
                        sp->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
                                     sizeof(mobAppear), &mobAppear);
                    }
                    else
                    {
                        strncpy(monster->uniqueName, monsterGrave->uniqueName,32);
                        monster->r              = monsterGrave->r;
                        monster->g              = monsterGrave->g;
                        monster->b              = monsterGrave->b;
                        monster->a              = monsterGrave->a;
                        monster->sizeCoeff      = monsterGrave->sizeCoeff;
                        monster->health         = monsterGrave->health;
                        monster->maxHealth      = monsterGrave->maxHealth;
                        monster->damageDone     = monsterGrave->damageDone;
                        monster->toHit          = monsterGrave->toHit;
                        monster->defense        = monsterGrave->defense;
                        monster->dropAmount     = monsterGrave->dropAmount;
                        monster->magicResistance= monsterGrave->magicResistance;
						
						if (!strcmp(monster->uniqueName, "Trickster Minotaur"))
							monster->tamingcounter = 1000; // I said NO TAMING TRICKSTER!!!!
						if (monster->type==24 && monster->subType==4)
							monster->tamingcounter = 1000; // I said NO TAMING LAB SPIDER BOSSES!!!!
						MessMobAppearCustom mAppearCustom;
                        mAppearCustom.type = SMOB_MONSTER;
                        mAppearCustom.mobID = (unsigned long) monster;
                        mAppearCustom.x = monster->cellX;
                        mAppearCustom.y = monster->cellY;
                        mAppearCustom.monsterType = monster->type;
                        mAppearCustom.subType = monster->subType;
                        CopyStringSafely(monster->Name(), 32, mAppearCustom.name, 32);
                        mAppearCustom.a = monster->a;
                        mAppearCustom.r = monster->r;
                        mAppearCustom.g = monster->g;
                        mAppearCustom.b = monster->b;
                        mAppearCustom.sizeCoeff = monster->sizeCoeff;

                        sp->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
                                   sizeof(mAppearCustom), &mAppearCustom);
                    }


                    delete curMob;
					sprintf_s(tempText, "Grave deleted.\n");
					LogOutput("gamelog.txt", tempText);

                }
                else if (curMob->isDead && SMOB_BOMB == curMob->WhatAmI())
                {
                    lastBranch = 3;
					
                    sp->mobList->Remove(curMob);
                    delete curMob;
                }
				else if (curMob->WhatAmI() == SMOB_GROUND_EFFECT && ((BBOSGroundEffect *)curMob)->killme > 0) // safe due to short circuit
				{
					// decrease the kill counter, THEN check if its' 0. 
					--((BBOSGroundEffect *)curMob)->killme; // decrease counter
					if (((BBOSGroundEffect *)curMob)->killme < 1) // if it got to 0 after a decrease
					{
						sp->mobList->Remove(curMob); // make it go away.
						MessMobDisappear messMobDisappear;
						messMobDisappear.mobID = (unsigned long)curMob;
						messMobDisappear.x = curMob->cellX;
						messMobDisappear.y = curMob->cellY;
						sp->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY,
							sizeof(messMobDisappear), (void *)&messMobDisappear);
						delete curMob;

					}

				}
                curMob = (BBOSMob *) sp->mobList->GetNext();
            }
        }

        if (cont || SPACE_REALM == sp->WhatAmI() || SPACE_LABYRINTH == sp->WhatAmI())
        {
            // tick all of this space's Generators

            BBOSGenerator *curGen = (BBOSGenerator *) sp->generators->First();
            while (curGen)
            {
                curGen->Tick(sp);
                curGen = (BBOSGenerator *) sp->generators->Next();
            }
        }
/*
        if (SPACE_LABYRINTH == sp->WhatAmI() && REALM_ID_LAB2 == ((LabyrinthMap *)sp)->type)
        {

            delta = now - labyMapTime;

            if (0 == labyMapTime || now < labyMapTime || now == labyMapTime)
            {
                delta = 1000 * 60 + 1;
            }

            if (delta > 1000 * 60)	
            {
                labyMapTime = now;

                ((LabyrinthMap *)sp)->UpdateMonsterMap();

            }
        }
        */

        sp = (SharedSpace *) spaceList->Next();
    }
	// process delayedstartup events. this way we can have armys activate at times other than immediately on server start.
	delta = now - testTime; // test time is when server was started
	if ((DelayedStartDone==0) && (delta >1000*60*15))  // 15 minutes
	{ 
		DelayedStartDone = 1; // only do this once
		sp = (SharedSpace *)spaceList->First(); // fetch the first shared space, that's where we wants the army.
		if (run_halloween)
		{
					ArmyHalloween *ah = new ArmyHalloween(sp, 94, 43, 94, 53, 25);  //second army
					sp->generators->Append(ah);
		}

	}

    // process 10 second items
    delta = now - tenSecondTime;

    if (0 == tenSecondTime || now < tenSecondTime || now == tenSecondTime)
    {
        delta = 1000 * 10 + 1;
    }

    if (delta > 1000 * 10)	
    {
        tenSecondTime = now;

        int oldWeatherState = weatherState;

        int oneHour = 60 * 6;
        if (dayTimeCounter > 2.75f * oneHour &&
             dayTimeCounter < 3.75f * oneHour)
        {
            if (1 == weatherState) // if storming
            {
//				if (7 == rand() % 10) // TEST
                if (7 == rand() % 15)
                    weatherState = 0; // clears up
            }
            else if (0 == weatherState) // if normal
            {
//				if (7 == rand() % 10) // TEST
                if (7 == rand() % 50)
                    weatherState = 1; // lightning storm
            }
        }
        else
            weatherState = 0; // clears up

        if (oldWeatherState != weatherState)
        {
            MessWeatherState state;
            state.value = weatherState;
            lserver->SendMsg(sizeof(state),(void *)&state, 0, NULL);
        }

        ++dayTimeCounter;
        if (dayTimeCounter >= 4 * 60 * 6)
        {
            dayTimeCounter = 0;

            ++smasherCounter;
            // find a tower to destroy

            TowerMap *candidate = NULL;
            GroundMap *gm = NULL;
            SharedSpace *sp = (SharedSpace *) spaceList->First();
            while (sp)
            {
                if (SPACE_GROUND == sp->WhatAmI())
                    gm = (GroundMap *) sp;
                else if (SPACE_GUILD == sp->WhatAmI())
                {
                    TowerMap *dm = (TowerMap *)sp;
                    LongTime rightNow;

                    DWORD age = dm->lastChangedTime.MinutesDifference(&rightNow);

                    DWORD ageOut = 60 * 24 * 21; // three weeks for normal guild
					if (!dm->itemBox->objects.IsListEmpty() ||
						dm->specLevel[0] > 0 || dm->specLevel[1] > 0 || dm->specLevel[2] > 0)
						ageOut = 60 * 24 * 7 * 8; // eight weeks for big guild
					if (dm->specLevel[0] > 1 || dm->specLevel[1] > 1 || dm->specLevel[2] > 1)
						ageOut = 60 * 24 * 7 * 1000; // a LOT of weeks for guild that spent 50m+ on specs
					if (dm->specLevel[0] > 0 && dm->specLevel[1] > 0 && dm->specLevel[2] > 0)
						ageOut = 60 * 24 * 7 * 1000; // a LOT of weeks for guild with all specs

                    if (age > ageOut)
                    {
                        candidate = dm;
                    }
                }
                sp = (SharedSpace *) spaceList->Next();
            }

            if (candidate && gm && 2 == (smasherCounter % 5))
            {
                // find and remove the outside tower
                BBOSTower *towerMob = NULL;
                curMob = (BBOSMob *) gm->mobList->GetFirst(0,0,1000);
                while (curMob)
                {
                    if (SMOB_TOWER == curMob->WhatAmI() && ((BBOSTower *)curMob)->isGuildTower &&
                         ((BBOSTower *)curMob)->ss == candidate)
                    {
                        towerMob = (BBOSTower *)curMob;
                        gm->mobList->SetToLast();
                    }
                    curMob = (BBOSMob *) gm->mobList->GetNext();
                }

                if (towerMob)
                {
                    // announce it's removal
                    MessMobDisappear messMobDisappear;
                    messMobDisappear.mobID = (unsigned long) towerMob;
                    messMobDisappear.x = towerMob->cellX;
                    messMobDisappear.y = towerMob->cellY;
                    gm->SendToEveryoneNearBut(0, towerMob->cellX, towerMob->cellY,
                        sizeof(messMobDisappear),(void *)&messMobDisappear);

                    // add a generic effect on that spot
                    MessGenericEffect messGE;
                    messGE.avatarID = -1;
                    messGE.mobID    = (long)towerMob;
                    messGE.x        = towerMob->cellX;
                    messGE.y        = towerMob->cellY;
                    messGE.r        = 255;
                    messGE.g        = 255;
                    messGE.b        = 255;
                    messGE.type     = 0;  // type of particles
                    messGE.timeLen  = 20; // in seconds
                    gm->SendToEveryoneNearBut(0, towerMob->cellX, towerMob->cellY,
                                         sizeof(messGE),(void *)&messGE);

                    MessExplosion explo;
                    explo.avatarID = 0;
                    explo.r = 255;
                    explo.g = 255;
                    explo.b = 000;
                    explo.type = 0;
                    explo.flags = 0;
                    explo.size = 1000;
                    explo.x = towerMob->cellX;
                    explo.y = towerMob->cellY;

                    gm->SendToEveryoneNearBut(0, towerMob->cellX, towerMob->cellY, 
                        sizeof(explo), &explo, 5 + 1000/10);

                    // create a big nasty monster on that spot

                    BBOSMonster *monster = new BBOSMonster(1,5, NULL);  // granite golem
                    monster->cellX = monster->targetCellX = monster->spawnX = towerMob->cellX;
                    monster->cellY = monster->targetCellY = monster->spawnY = towerMob->cellY;
                    gm->mobList->Add(monster);
                    sprintf_s(monster->uniqueName, "Tower Smasher");

                    //	adjusts its power
                    monster->r               = 0;
                    monster->g               = 255;
                    monster->b               = 255;
                    monster->a               = 255;
                    monster->sizeCoeff       = 3;
                    monster->health          *= 10;
                    monster->maxHealth       *= 10;
                    monster->damageDone       = 45;

                    monster->toHit           = 20 + (rand() % 80);
                    monster->defense         = 20 + (rand() % 80);
                    monster->dropAmount      = 30;
                    monster->magicResistance = 0.6f;
                    monster->healAmountPerSecond = 200 + (rand() % 300);

                    // announce it
                    MessMobAppear mobAppear;
                    mobAppear.mobID = (unsigned long) monster;
                    mobAppear.type = monster->WhatAmI();
                    mobAppear.monsterType = monster->type;
                    mobAppear.subType = monster->subType;
                    mobAppear.x = monster->cellX;
                    mobAppear.y = monster->cellY;
                    gm->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
                                 sizeof(mobAppear), &mobAppear);

                    // tell everyone what's happening
                    sprintf(&(tempText[1]),"You hear a tremendous crash, and a low, rumbling laugh!");
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    bboServer->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);

                    // delete the outside tower
                    gm->mobList->Remove(towerMob);
                    delete towerMob;

                }
                else
                    dayTimeCounter = 3000;  // instantly try to do it all again

                spaceList->Remove(candidate);
                delete candidate;
            }
        }
    }

    // process dungeons
    delta = now - dungeonUpdateTime;

    if (0 == dungeonUpdateTime || now < dungeonUpdateTime || now == dungeonUpdateTime)
    {
        delta = 1000 * 60 * 10 + 1;
    }

    if (delta > 1000 * 60 * 10)	
    {
        tokenMan.Tick();
        tokenMan.Save();
		// save unicorn locations
		FILE *fp2 = fopen("serverdata\\unicornData.dat", "w");
		if (fp2)
		{
			for (int i = 0; i < 6; ++i)
			{
				fprintf(fp2, "%d %d\n",
					UnicornLocations[i][0], UnicornLocations[i][1]);
			}

			fclose(fp2);
		}

        banList->Save();
        uidBanList->Save();

        // save out guild towers *************
        FILE *fp = fopen("serverdata\\guilds.dat","w");
        if (fp)
        {
            fprintf(fp,"%f\n",VERSION_NUMBER);

            SharedSpace *sp = (SharedSpace *) spaceList->First();
            while (sp)
            {
                if (SPACE_GUILD == sp->WhatAmI())
                {
                    TowerMap *dm = (TowerMap *)sp;
                    fprintf(fp,"TOWER\n");

                    dm->ProcessVotes();

                    dm->Save(fp);
                }
                sp = (SharedSpace *) spaceList->Next();
            }

            fprintf(fp,"END\n");
            fclose(fp);
        }

		// save out Mysterious cave, hopefully
		fp = fopen("serverdata\\cave.dat", "w");
		SharedSpace *sp = (SharedSpace *)spaceList->First();
		while (sp)
		{
			if (SPACE_DUNGEON == sp->WhatAmI())
			{
				DungeonMap *dm = (DungeonMap *)sp;
				if (dm->specialFlags == SPECIAL_DUNGEON_MODERATED)  // it's the cave
				{
					// found it, tr yto save.
					sprintf_s(tempText, "Saving cave dat file.\n");
					LogOutput("gamelog.txt", tempText);

					dm->Save(fp);                                   // save it out
				}
			}
			sp = (SharedSpace *)spaceList->Next();              // next space
		}

		fclose (fp);
        // save out dungeons *************
        dungeonUpdateTime = now;
		fp = fopen("serverdata\\dungeons.dat", "w");
		if (fp)
        {
            fprintf(fp,"%f\n",VERSION_NUMBER);

            DungeonMap *needyDungeon = NULL;

            // make absolutely sure ServerRandomAvatar is valid.
            /*
            if (ServerRandomAvatar)
            {
                SharedSpace *tempss;
                if (!FindAvatar(ServerRandomAvatar,&tempss))
                    ServerRandomAvatar = NULL;
            }
              */
            SharedSpace *sp = (SharedSpace *) spaceList->First();
            while (sp)
            {
                if (SPACE_DUNGEON == sp->WhatAmI())
                {
                    DungeonMap *dm = (DungeonMap *)sp;
                    if (0 == dm->specialFlags)
                    {
                        fprintf(fp,"DUNGEON\n");
                        dm->Save(fp);
                    }
                    else if ((dm->specialFlags & SPECIAL_DUNGEON_TEMPORARY) || (dm->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
                    {
                        if (dm->avatars->IsListEmpty())
                        {
                            spaceList->Remove(dm);
                            delete dm;
                        }
                    }
                    /*
                    if (dm->masterName[0])
                    {
                        // if ServerRandomAvatar is already the master here, invalidate it.
                        if (ServerRandomAvatar &&
                             !strcmp(ServerRandomAvatar->name,dm->masterName) &&
                             !strcmp(ServerRandomAvatar->pass,dm->masterPass))
                             ServerRandomAvatar = NULL;
                    }
                    */
                }
                sp = (SharedSpace *) spaceList->Next();
            }

            fprintf(fp,"END\n");
            fclose(fp);

            LongTime ltNow;
            /*
            sp = (SharedSpace *) spaceList->First();
            while (sp)
            {
                if (SPACE_DUNGEON == sp->WhatAmI())
                {
                    DungeonMap *dm = (DungeonMap *)sp;

                    if (dm->masterName[0])
                    {
                        if (ltNow.MinutesDifference(&(dm->masterTimeout)) <= 0)
                        {
                            // time to say goodbye to this master!
                            UserMessage *forMe = new UserMessage(0, dm->masterName);
                            userMessageList->Append(forMe);
                            sprintf_s(forMe->password, dm->masterPass);
                            sprintf_s(forMe->message, "You are no longer the mistress of a dungeon.");
                            dm->masterName[0] = 0;
                            dm->masterPass[0] = 0;
                        }
                        else
                        {
                            // tell the master how much time is left
                            UserMessage *forMe = new UserMessage(0, dm->masterName);
                            userMessageList->Append(forMe);
                            sprintf_s(forMe->password, dm->masterPass);
                            sprintf_s(forMe->message, "You are the mistress of the %s", dm->name);

                            forMe = new UserMessage(0, dm->masterName);
                            userMessageList->Append(forMe);
                            sprintf_s(forMe->password, dm->masterPass);
                            if (ltNow.MinutesDifference(&(dm->masterTimeout)) < 60)
                                sprintf_s(forMe->message, "at %dN %dE for %d more minutes.", 
                                         256-dm->enterY, 256-dm->enterX, 
                                          ltNow.MinutesDifference(&(dm->masterTimeout)));
                            else if (ltNow.MinutesDifference(&(dm->masterTimeout)) < 60 * 24)
                                sprintf_s(forMe->message, "at %dN %dE for %d more hours.", 
                                         256-dm->enterY, 256-dm->enterX, 
                                          ltNow.MinutesDifference(&(dm->masterTimeout)) / 60);
                            else 
                                sprintf_s(forMe->message, "at %dN %dE for %d more days.", 
                                         256-dm->enterY, 256-dm->enterX, 
                                          ltNow.MinutesDifference(&(dm->masterTimeout)) / 60 / 24);

                            // now, handle payout
                            forMe = new UserMessage(0, dm->masterName);
                            sprintf_s(forMe->password, dm->masterPass);
                            sprintf_s(forMe->message, "PAYOUT");

                            forMe->value1 = forMe->value2 = forMe->value3 = 0;

                            curMob = (BBOSMob *) sp->avatars->First();
                            while (curMob)
                            {
                                if (!strcmp(((BBOSAvatar *)curMob)->name,dm->masterName) &&
                                   !strcmp(((BBOSAvatar *)curMob)->pass,dm->masterPass))
                                    ;
                                else
                                    ++(forMe->value1);

                                curMob = (BBOSMob *) sp->avatars->Next();
                            }

                            curMob = (BBOSMob *) sp->mobList->GetFirst(0,0,1000);
                            while (curMob)
                            {
                                if (SMOB_CHEST == curMob->WhatAmI())
                                {
                                    if (((BBOSChest *)curMob)->isOpen)
                                        ;
                                    else
                                        ++(forMe->value2);
                                }

                                curMob = (BBOSMob *) sp->mobList->GetNext();
                            }

                            if (forMe->value1 > 0)
                            {
                                forMe->value3 = forMe->value2 * 10;
                                userMessageList->Append(forMe);
                            }
                            else
                                delete forMe;
                        }
                    }
                    else if (ServerRandomAvatar)
                    {
                        // Say hello to a new master!
                        UserMessage *forMe = new UserMessage(0, ServerRandomAvatar->name);
                        userMessageList->Append(forMe);
                        sprintf_s(forMe->password, ServerRandomAvatar->pass);
                        sprintf_s(forMe->message, "I am the spirit of the %s.", dm->name);

                        forMe = new UserMessage(0, ServerRandomAvatar->name);
                        userMessageList->Append(forMe);
                        sprintf_s(forMe->password, ServerRandomAvatar->pass);
                        sprintf_s(forMe->message, "You are my mistress now.");

                        forMe = new UserMessage(0, ServerRandomAvatar->name);
                        userMessageList->Append(forMe);
                        sprintf_s(forMe->password, ServerRandomAvatar->pass);
                        sprintf_s(forMe->message, "Come to %dN %dE and shape my halls.", 
                                 256-dm->enterY, 256-dm->enterX);

                        sprintf_s(dm->masterName, ServerRandomAvatar->name);
                        sprintf_s(dm->masterPass, ServerRandomAvatar->pass);
                        dm->masterTimeout.SetToNow();
                        dm->masterTimeout.AddMinutes(60*24*14);

                        ServerRandomAvatar = NULL;
                    }
            
                }
                sp = (SharedSpace *) spaceList->Next();
            }
            */

        }
    }

}


//***************************************************************
void BBOServer::HandleMessages(void)
{
    char messData[4000], tempText[1024];
    int  dataSize;
    FILE *source = NULL;
    int i,j, tempX, tempY;
	int uidcount = 0;
	int loginlimit;

    InventoryObject *iObject;
    InvBlade *iBlade;
    InvTotem *iTotem;
    InvIngot *iIngot;
    Inventory *curInventory;

    MessPlayerChatLine chatMess;
    MessInfoText infoText;
    MessGeneralYes messYes;
    MessGeneralNo	messNo;
    MessAvatarMove AvMove;
    MessAccept *messAcceptPtr;

//	MessAvatarDisappear aDisappear;
    MessAvatarStats messAvatarStats;
    MessAvatarNames messAvatarNames;

    MessInventoryInfo inventoryInfo;
    MessWield messWield;
    MessUnWield messUnWield;
    MessBladeDesc messBladeDesc;
    MessMobDisappear messMobDisappear;
    MessDungeonInfo messDInfo;
    MessDungeonChange messDungeonChange;
    MessTestPing messTP;
    MessExtendedInfoRequest infoReq;

    MessEnterGame *enterGamePtr;
    MessPlayerNew *mpNewPtr;
    MessPlayerReturning *mpReturningPtr;
    MessAvatarMoveRequest *AvMoveReqPtr;
    MessAvatarRequestStats *requestStatsPtr;
    MessAvatarStats *messAvatarStatsPtr;
    MessBugReport *bugReportPtr;
    MessInventoryRequestInfo *invRequestInfoPtr;
    MessInventoryTransferRequest *transferRequestPtr;
    MessWield *messWieldPtr;
    MessUnWield *messUnWieldPtr;
	MessAvatarAttack *messAAPtr;
	MessStartTame *messSTPtr;
	MessAvatarDelete *messDeletePtr;
    MessInventoryChange *inventoryChangePtr;
    MessTryCombine *messTryCombinePtr;
    MessRequestDungeonInfo *messRDInfoPtr;
    MessDungeonChange *messDungeonChangePtr;
    MessRequestAvatarInfo *messRequestAvatarInfo;
    MessRequestTownMageService *messTMSPtr;
    MessAvatarSendMoney *messMoney;
    MessChestInfo *messChestPtr;
    MessTalkToTree *messTreePtr;
    MessFeedPetRequest *messFPRPtr;
    MessPetName *messPNPtr;
    MessAdminMessage *adminMessPtr;
    MessInfoFlags *infoFlagsPtr;
    MessSellAll *sellAllPtr;
    MessSecureTrade *secureTradePtr;
    MessExtendedInfoRequest *requestExInfoPtr;
    MessSetBomb *setBombPtr;
    MessKeyCode *keyCodePtr;
    MessAvatarNewClothes *clothesPtr;
    MessChatChannel *chatChannelPtr;

    SharedSpace *ss;
	SharedSpace *curSS;
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;
    BBOSNpc *curNpc;

    DoublyLinkedList *list = NULL;

    std::vector<TagID> tempReceiptList;
    int					fromSocket = 0;

    lserver->GetNextMsg(NULL, dataSize);
    
    while (dataSize > 0)
    {
        if (dataSize > 4000)
            dataSize = 4000;
        
        lserver->GetNextMsg(messData, dataSize, &fromSocket, &tempReceiptList);
        
        MessEmpty *empty = (MessEmpty *)messData;
        switch (messData[0])
        {
			case NWMESS_ACCEPT:
			    messAcceptPtr = (MessAccept *) messData;

			    if (banList->IsBanned(messAcceptPtr->IP))
			    {
			        messNo.subType = 10;
			        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
			    }
			    else
			    {
			        // get rid of any other avatar with this id
			        curAvatar = FindAvatar(fromSocket, &ss);
			        while (curAvatar)
			        {
			            ss->avatars->Remove(curAvatar);
			            delete curAvatar;
			            curAvatar = FindAvatar(fromSocket, &ss);
			        }
						
	                curAvatar = FindIncomingAvatar(fromSocket);
		            while (curAvatar)
			        {
				        incoming->Remove(curAvatar);
					    delete curAvatar;
	                    curAvatar = FindIncomingAvatar(fromSocket);
		            }

	                curAvatar = new BBOSAvatar();
	                curAvatar->socketIndex = fromSocket;
	                curAvatar->curCharacterIndex = -1;
		             curAvatar->IP[0] = messAcceptPtr->IP[0];
	                curAvatar->IP[1] = messAcceptPtr->IP[1];
	                curAvatar->IP[2] = messAcceptPtr->IP[2];
	                curAvatar->IP[3] = messAcceptPtr->IP[3];

	                incoming->Append(curAvatar);

		            tempReceiptList.clear();
			        tempReceiptList.push_back(fromSocket);
				    infoReq.listType = 0;
					lserver->SendMsg(sizeof(infoReq),(void *)&infoReq, 0, &tempReceiptList);
				}

			break;

	        case NWMESS_CLOSE:
	            // remove any avatars that are associated with his message
	            j = 0;
	            curAvatar = FindAvatar(fromSocket, &ss);
	            while (curAvatar)
	            {
	                j = 1;
	                sprintf_s(tempText,"** closing connection: %s, %s ",curAvatar->name,
	                          curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
	                LogOutput("gamelog.txt", tempText);
	
	                LongTime lt;
	                lt.value.wHour += 19;
	                if (lt.value.wHour < 24)
	                {
	                    lt.value.wDay -= 1;
	                }
	                else
	                    lt.value.wHour -= 24;
	
	
	                sprintf_s(tempText,"%d/%02d, %d:%02d\n", (int)lt.value.wMonth, (int)lt.value.wDay, 
	                          (int)lt.value.wHour, (int)lt.value.wMinute);
	                LogOutput("gamelog.txt", tempText);
	
	                if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType || ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
	                {
	                    LogOutput("moderatorLog.txt", tempText);
	                }
	
		             curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_NOTHING);

	                sprintf_s(infoText.text,"%s left the game.",
	                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
	                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                             sizeof(infoText),(void *)&infoText);

	                curAvatar->QuestSpaceChange(ss,NULL);

	                ss->avatars->Remove(curAvatar);
					if ((SPACE_GROUND != ss->WhatAmI()) || //not in normal realm
						((SPACE_GROUND == ss->WhatAmI()) && (((GroundMap *)ss)->type>0)))	// or in the alternate big map
					{
				        curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX = 
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].spawnX;
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY = 
						    curAvatar->charInfoArray[curAvatar->curCharacterIndex].spawnY;
					}
					else
					{
	                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX = 
	                        curAvatar->cellX;
	                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY = 
	                        curAvatar->cellY;
	                }
	                curAvatar->SaveAccount();
					if (curAvatar->controlledMonster) // if there'a  a controlled monster
					{ 
						// die with no loot for anyone!
						for (int i = 0; i < 10; ++i)
							curAvatar->controlledMonster->attackerPtrList[i] = NULL;
						curAvatar->controlledMonster->dropAmount = 0; // dont' drop anything!
						curAvatar->controlledMonster->type = 25; //make it a vamp 
						curAvatar->controlledMonster->isDead = TRUE;
						curAvatar->controlledMonster->controllingAvatar = NULL;	// and unlink it
						curAvatar->controlledMonster = NULL;	// and unlink it
					}
	                delete curAvatar;
	                curAvatar = FindAvatar(fromSocket, &ss);
	            }
	
		        curAvatar = FindIncomingAvatar(fromSocket);
		        while (curAvatar)
	            {
	                j = 1;
	                incoming->Remove(curAvatar);
	                delete curAvatar;
	                curAvatar = FindIncomingAvatar(fromSocket);
	            }

	            if (!j)
	            {
	                sprintf_s(tempText,"****** orphan close message, ");
	                LogOutput("gamelog.txt", tempText);

	                LongTime lt;
	                lt.value.wHour += 19;
	                if (lt.value.wHour < 24)
	                {
	                    lt.value.wDay -= 1;
	                }
	                else
	                    lt.value.wHour -= 24;
	
	                sprintf_s(tempText,"%d/%02d, %d:%02d\n", (int)lt.value.wMonth, (int)lt.value.wDay, 
	                          (int)lt.value.wHour, (int)lt.value.wMinute);
	                LogOutput("gamelog.txt", tempText);
	            }
	
	           break;
	
	        case NWMESS_PLAYER_CHAT_LINE:
	            GuaranteeTermination((char *) &(messData[1]), dataSize-1);
	//			CleanString((char *) &(messData[1]), 0);
	            HandleChatLine(fromSocket, &(messData[1]));
	           break;
	
	        case NWMESS_PLAYER_NEW:
	            mpNewPtr = (MessPlayerNew *) messData;
	            
	            codePad = cryptoText1;
	            UnCryptoString(mpNewPtr->name);
	
	            GuaranteeTermination(mpNewPtr->name, NUM_OF_CHARS_FOR_USERNAME);
	            CorrectString(mpNewPtr->name);
	            GuaranteeTermination(mpNewPtr->pass, NUM_OF_CHARS_FOR_PASSWORD);
	            // CorrectString(mpNewPtr->pass);
	
	            if (uidBanList->IsBanned(mpNewPtr->uniqueId))
	            {
	                messNo.subType = 12;
	                lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                break;
	            }
	
	            curAvatar = FindAvatar(mpNewPtr->name, mpNewPtr->pass, &ss);
	            if (curAvatar)
	            {
	                messNo.subType = 6;
	                lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                break;
	            }
	
	            curAvatar = FindIncomingAvatar(fromSocket);
	            if (curAvatar)
	            {
	                j = curAvatar->LoadAccount(mpNewPtr->name, mpNewPtr->pass, TRUE);
	                curAvatar->SetUniqueId(mpNewPtr->uniqueId);

	                tempReceiptList.clear();
	                tempReceiptList.push_back(fromSocket);
	                if (0 == j)
	                {
	                    messYes.subType = fromSocket;
	                    lserver->SendMsg(sizeof(messYes),(void *)&messYes, 0, &tempReceiptList);
	//					TransferAvatar(TRUE,fromSocket);
	                }
	                if (1 == j)
	                {
	                    messNo.subType = 1;
	                    lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                }
	                if (2 == j)
	                {
	                    messNo.subType = 2;
	                    lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                }
					if (3 == j)
					{
						messNo.subType = 3;
						lserver->SendMsg(sizeof(messNo), (void *)&messNo, 0, &tempReceiptList);
					}
					if (10 == j)
					{
						messNo.subType = 10;
						lserver->SendMsg(sizeof(messNo), (void *)&messNo, 0, &tempReceiptList);
					}
					if (20 == j)
					{
						messNo.subType = 12;
						lserver->SendMsg(sizeof(messNo), (void *)&messNo, 0, &tempReceiptList);
						// and ban the uid.
						uidBanList->addBannedUID(mpNewPtr->uniqueId,
							"Mr. Cheater");
						uidBanList->Save();
					}
					if (-1 == j)
	                {
						messNo.subType = -1;
	                    lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                }
	            }
				break;

			case NWMESS_PLAYER_RETURNING:
	            mpReturningPtr = (MessPlayerReturning *) messData;
	            tempReceiptList.clear();
	            tempReceiptList.push_back(fromSocket);
	            codePad = cryptoText1;
	            UnCryptoString(mpReturningPtr->name);
				loginlimit = max_logins;
				if (enable_cheating != 0) // if we are allowing cheats
				loginlimit = 1000; // no one can hit that.
	            GuaranteeTermination(mpReturningPtr->name, NUM_OF_CHARS_FOR_USERNAME);
	            CorrectString(mpReturningPtr->name);
	            GuaranteeTermination(mpReturningPtr->pass, NUM_OF_CHARS_FOR_PASSWORD);
	            // CorrectString(mpReturningPtr->pass);
	
	            if (uidBanList->IsBanned(mpReturningPtr->uniqueId))
	            {
	                messNo.subType = 12;
	                lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                break;
	            }
				// compare UID with UIDs of logged on players. 
				curSS = (SharedSpace*)spaceList->First();
				while (curSS)
				{
					BBOSAvatar *curMob = (BBOSAvatar *)curSS->avatars->First();
						while (curMob)
						{
							if (mpReturningPtr->uniqueId == curMob->GetUniqueId())
								uidcount++;
							curMob = (BBOSAvatar *)curSS->avatars->Next();
						}
						curSS = (SharedSpace*)spaceList->Next();
	
				}
				if (uidcount >= loginlimit)
				{
					messNo.subType = 13;
					lserver->SendMsg(sizeof(messNo), (void *)&messNo, 0, &tempReceiptList);
					break;
				}
	            if (mpReturningPtr->version != VERSION_NUMBER)
	            {
	                messNo.subType = 5;
	                lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                break;
	            }
	            curAvatar = FindAvatar(mpReturningPtr->name, mpReturningPtr->pass, &ss);
	            if (curAvatar)
	            {
	                messNo.subType = 6;
	                lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                break;
	            }
	            curAvatar = FindIncomingAvatar(fromSocket);
	            if (curAvatar)
	            {
	                int cont = TRUE;
	                j = curAvatar->LoadAccount(mpReturningPtr->name, mpReturningPtr->pass, FALSE,FALSE, mpReturningPtr->uniqueId);
	                curAvatar->SetUniqueId(mpReturningPtr->uniqueId);
	
	                if (1 == curAvatar->restrictionType) // if tempBanned
	                {
	                    LongTime now;
	                    if (now.MinutesDifference(&curAvatar->accountRestrictionTime) <= 0)
	                    {		
	                        curAvatar->restrictionType = 0;  // you're free!
	                    }
	                    else
	                    {
	                        messNo.subType = 7;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                        cont = FALSE;
	                    }
	                }
	                if (cont)
	                {
	                    if (0 == j)
	                    {
	                        messYes.subType = fromSocket;
	                        lserver->SendMsg(sizeof(messYes),(void *)&messYes, 0, &tempReceiptList);
	   //					TransferAvatar(TRUE,fromSocket);
	                    }
						if (1 == j)
	                    {
	                        messNo.subType = 1;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
	                    if (2 == j)
	                    {
	                        messNo.subType = 2;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
	                    if (3 == j)
	                    {
	                        messNo.subType = 3;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
	                    if (4 == j)
	                    {
	                        messNo.subType = 4;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
	                    if (-1 == j)
	                    {
	                        messNo.subType = -1;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
	                    if (10 == j)
	                    {
	                        messNo.subType = 10;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
						if (20 == j)
						{
							messNo.subType = 12;
							lserver->SendMsg(sizeof(messNo), (void *)&messNo, 0, &tempReceiptList);
	                    
							uidBanList->addBannedUID(mpReturningPtr->uniqueId,
								"Mr. Cheater");
							uidBanList->Save();
						}
						if ( 11 == j )
	                    {
	                        messNo.subType = 11;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
	                }
	            }
				break;

	        case NWMESS_AVATAR_MOVE_REQUEST:
	            AvMoveReqPtr = (MessAvatarMoveRequest *) messData;
	            if (AvMoveReqPtr->avatarID != fromSocket)
	                break;
	
	            TryToMoveAvatar(fromSocket, AvMoveReqPtr);
	            break;

	        case NWMESS_AVATAR_REQUEST_STATS:
	            requestStatsPtr = (MessAvatarRequestStats *) messData;
	            tempReceiptList.clear();
	            tempReceiptList.push_back(fromSocket);

	            curAvatar = FindIncomingAvatar(fromSocket);
	            if (curAvatar)
	            {
	                requestStatsPtr->characterIndex = requestStatsPtr->characterIndex % NUM_OF_CHARS_PER_USER;
	                if (curAvatar->charInfoArray[requestStatsPtr->characterIndex].topIndex > -1)
	                {
	                    messAvatarStats.avatarID = requestStatsPtr->characterIndex;
	
	                    messAvatarStats.faceIndex = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].faceIndex;
	                    messAvatarStats.hairR = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].hairR;
	                    messAvatarStats.hairG = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].hairG;
	                    messAvatarStats.hairB = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].hairB;
	
	                    messAvatarStats.topIndex = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].topIndex;
	                    messAvatarStats.topR = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].topR;
	                    messAvatarStats.topG = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].topG;
	                    messAvatarStats.topB = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].topB;
	
	                    messAvatarStats.bottomIndex = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].bottomIndex;
	                    messAvatarStats.bottomR = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].bottomR;
	                    messAvatarStats.bottomG = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].bottomG;
	                    messAvatarStats.bottomB = 
	                          curAvatar->charInfoArray[requestStatsPtr->characterIndex].bottomB;
	
	                    memcpy(messAvatarStats.name,
	                             curAvatar->charInfoArray[requestStatsPtr->characterIndex].name,31);
	                    messAvatarStats.name[31] = 0;
	
	                    messAvatarStats.physical =
	                        curAvatar->charInfoArray[requestStatsPtr->characterIndex].physical;
	                    messAvatarStats.magical =		
	                        curAvatar->charInfoArray[requestStatsPtr->characterIndex].magical;
						messAvatarStats.creative =
							curAvatar->charInfoArray[requestStatsPtr->characterIndex].creative;
						messAvatarStats.beast =
							curAvatar->charInfoArray[requestStatsPtr->characterIndex].beast;
	
						messAvatarStats.imageFlags =		
	                        curAvatar->charInfoArray[requestStatsPtr->characterIndex].imageFlags;
	
	                    messAvatarStats.cLevel =		
	                        curAvatar->charInfoArray[requestStatsPtr->characterIndex].cLevel;
	                    messAvatarStats.cash =		
	                        curAvatar->charInfoArray[requestStatsPtr->characterIndex].inventory->money;
	
	                    lserver->SendMsg(sizeof(messAvatarStats),
	                                      (void *)&messAvatarStats, 0, &tempReceiptList);
	                }
	                else
	                {
	                    // there's no valid avatar in that slot
						messNo.subType = 1;
	                    lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
		            }

	                // also send the names of the eight avatars
		            for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
			        {
				        if (curAvatar->charInfoArray[i].topIndex > -1)
					    {
						    memcpy(messAvatarNames.name[i],
                                 curAvatar->charInfoArray[i].name,31);
							messAvatarNames.name[i][31] = 0;
	                    }
		                else
			                messAvatarNames.name[i][0] = 0;
				    }
	                lserver->SendMsg(sizeof(messAvatarNames),(void *)&messAvatarNames, 0, &tempReceiptList);

	                // also send the account time left
	                MessAccountTimeInfo timeInfo;
	                timeInfo.wYear      = curAvatar->accountExperationTime.value.wYear     ;
	                timeInfo.wMonth     = curAvatar->accountExperationTime.value.wMonth    ;
	                timeInfo.wDay       = curAvatar->accountExperationTime.value.wDay      ;
	                timeInfo.wDayOfWeek = curAvatar->accountExperationTime.value.wDayOfWeek;
	                timeInfo.wHour      = curAvatar->accountExperationTime.value.wHour     ;
	                timeInfo.wMinute    = curAvatar->accountExperationTime.value.wMinute   ;
	
					lserver->SendMsg(sizeof(timeInfo),(void *)&timeInfo, 0, &tempReceiptList);
					// save the account when you enter character select, for the benefit of any beastmasters,
	            }
	           break;

	        case NWMESS_AVATAR_STATS:
	            messAvatarStatsPtr = (MessAvatarStats *) messData;
	            tempReceiptList.clear();
	            tempReceiptList.push_back(fromSocket);

	            messAvatarStatsPtr->avatarID = messAvatarStatsPtr->avatarID % NUM_OF_CHARS_PER_USER;
	            curAvatar = FindIncomingAvatar(fromSocket);
	            if (curAvatar)
	            {
	                if (messAvatarStatsPtr->avatarID < 0 || messAvatarStatsPtr->avatarID > 7)
	                    messAvatarStatsPtr->avatarID = 0;
	
	                GuaranteeTermination(messAvatarStatsPtr->name,32);
	                CorrectString(messAvatarStatsPtr->name);
	                CleanString(messAvatarStatsPtr->name);
	                RemoveStringTrailingSpaces(messAvatarStatsPtr->name);
	
	                if (-1 == curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].topIndex)
	                {
	                    if (!UN_IsNameUnique(messAvatarStatsPtr->name))
	                    {
	                        // there's already an avatar of that name
	                        messNo.subType = 7;
	                        lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                    }
						else
						{
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].faceIndex =
								messAvatarStatsPtr->faceIndex;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].hairR =
								messAvatarStatsPtr->hairR;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].hairG =
								messAvatarStatsPtr->hairG;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].hairB =
								messAvatarStatsPtr->hairB;

							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].topIndex =
								messAvatarStatsPtr->topIndex;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].topR =
								messAvatarStatsPtr->topR;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].topG =
								messAvatarStatsPtr->topG;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].topB =
								messAvatarStatsPtr->topB;

							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].bottomIndex =
								messAvatarStatsPtr->bottomIndex;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].bottomR =
								messAvatarStatsPtr->bottomR;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].bottomG =
								messAvatarStatsPtr->bottomG;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].bottomB =
								messAvatarStatsPtr->bottomB;

							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].physical =
								messAvatarStatsPtr->physical;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].magical =
								messAvatarStatsPtr->magical;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].creative =
								messAvatarStatsPtr->creative;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].beast =
								messAvatarStatsPtr->beast;

							// major reality check!
							curAvatar->MakeCharacterValid(messAvatarStatsPtr->avatarID);

							messAvatarStatsPtr->physical =
								curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].physical;
							messAvatarStatsPtr->magical =
								curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].magical;
							messAvatarStatsPtr->creative =
								curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].creative;
							messAvatarStatsPtr->beast =
								curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].beast;

							memcpy(curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].name,
								messAvatarStatsPtr->name, 31);
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].name[31] = 0;

							UN_AddName(messAvatarStatsPtr->name);

							messAvatarStatsPtr->imageFlags = 0;
							messAvatarStatsPtr->cash = 0;
							messAvatarStatsPtr->cLevel = 0;

							lserver->SendMsg(sizeof(MessAvatarStats),
								(void *)messAvatarStatsPtr, 0, &tempReceiptList);

							curInventory = curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].inventory;
							iObject = (InventoryObject *)curInventory->objects.First();
							while (iObject)
							{
								curInventory->objects.Remove(iObject);
								delete iObject;
								iObject = (InventoryObject *)curInventory->objects.First();
							}

							curInventory = curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].workbench;
							iObject = (InventoryObject *)curInventory->objects.First();
							while (iObject)
							{
								curInventory->objects.Remove(iObject);
								delete iObject;
								iObject = (InventoryObject *)curInventory->objects.First();
							}

							curInventory = curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].skills;
							iObject = (InventoryObject *)curInventory->objects.First();
							while (iObject)
							{
								curInventory->objects.Remove(iObject);
								delete iObject;
								iObject = (InventoryObject *)curInventory->objects.First();
							}

							curInventory = curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].wield;
							iObject = (InventoryObject *)curInventory->objects.First();
							while (iObject)
							{
								curInventory->objects.Remove(iObject);
								delete iObject;
								iObject = (InventoryObject *)curInventory->objects.First();
							}

							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].petDragonInfo[0].type = 255; // no dragon
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].petDragonInfo[1].type = 255; // no dragon
							sprintf_s(curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].petDragonInfo[0].name, "NO PET");
							sprintf_s(curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].petDragonInfo[1].name, "NO PET");

							InventoryObject *iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
							iTotem = (InvTotem *)iObject->extra;
							iTotem->type = 0;
							iTotem->quality = 0;

							iObject->mass = 1.0f;
							iObject->value = 1;
							iObject->amount = 2;
							UpdateTotem(iObject);
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].inventory->objects.Append(iObject);

							iObject = new InventoryObject(INVOBJ_INGOT, 0, "Tin Ingot");
							iIngot = (InvIngot *)iObject->extra;
							iIngot->damageVal = 1;
							iIngot->challenge = 1;
							iIngot->r = iIngot->b = iIngot->g = 128;

							iObject->mass = 0.0f;
							iObject->value = 1;
							iObject->amount = 4;

							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].inventory->objects.Append(iObject);
							if (curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].beast < 10)
							{

								iObject = new InventoryObject(INVOBJ_BLADE, 0, "Pointy Stick");
								iBlade = (InvBlade *)iObject->extra;
								iBlade->damageDone = 3;
								iBlade->size = 0.5f;
								iBlade->toHit = 1;
								iBlade->r = iBlade->b = iBlade->g = 128;

								iObject->mass = 2.0f;
								iObject->value = 1;
								iObject->amount = 1;
							}
							else
							{
								iObject = new InventoryObject(INVOBJ_BLADE, 0, "My First Scythe");
								iBlade = (InvBlade *)iObject->extra;
								iBlade->damageDone = 1;
								iBlade->size = 2.062500f;
								iBlade->toHit = 1;
								iBlade->r = iBlade->b = iBlade->g = 128;
								iBlade->tame = 15;
								iBlade->numOfHits = 1000;
								iObject->mass = 2.0f;
								iObject->value = 1;
								iObject->amount = 1;
								iBlade->type = BLADE_TYPE_TAME_SCYTHE;
							}
	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].wield->objects.Append(iObject);

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].inventory->money = 100;

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].health     = 30;
	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].healthMax  = 30;

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].lifeTime   = 0;

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].imageFlags = 0;

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].age = messAvatarStatsPtr->age = 1;
	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].cLevel = messAvatarStatsPtr->cLevel = 0;
	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].oldCLevel = 0;

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].lastX = 
		                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].spawnX = 
		                        townList[2].x;

	                        curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].lastY = 
			                    curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].spawnY = 
				                townList[2].y;
							curAvatar->charInfoArray[messAvatarStatsPtr->avatarID].Monster_maxHealth = 0; // NO MONSTER
							curAvatar->SaveAccount();

	                        // also send the names of the eight avatars
	                        for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
		                    {
			                    if (curAvatar->charInfoArray[i].topIndex > -1)
				                {
					                memcpy(messAvatarNames.name[i],
					                     curAvatar->charInfoArray[i].name,31);
							        messAvatarNames.name[i][31] = 0;
								}
	                            else
		                            messAvatarNames.name[i][0] = 0;
			                }
							lserver->SendMsg(sizeof(messAvatarNames),(void *)&messAvatarNames, 0, &tempReceiptList);
						}
					}
					else
					{
	                    // there's already an avatar in that slot
		                messNo.subType = 2;
			            lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
				    }
				}
				break;

			case NWMESS_AVATAR_DELETE:
	            messDeletePtr = (MessAvatarDelete *) messData;
	            tempReceiptList.clear();
	            tempReceiptList.push_back(fromSocket);
	
	            curAvatar = FindIncomingAvatar(fromSocket);
	            if (curAvatar)
	            {
	                messDeletePtr->characterIndex = messDeletePtr->characterIndex % NUM_OF_CHARS_PER_USER;
	                if (curAvatar->charInfoArray[messDeletePtr->characterIndex].topIndex > -1)
	                {
	                    UN_RemoveName(curAvatar->charInfoArray[messDeletePtr->characterIndex].name);
	
	                    SharedSpace *guildSpace;
	                    if (FindAvatarInGuild(
	                            curAvatar->charInfoArray[messDeletePtr->characterIndex].name, 
	                            &guildSpace))
	                    {
	                        DeleteNameFromGuild(
	                            curAvatar->charInfoArray[messDeletePtr->characterIndex].name, 
								&guildSpace);
	                    }
	
						curAvatar->charInfoArray[messDeletePtr->characterIndex].topIndex = -1;
						curAvatar->charInfoArray[messDeletePtr->characterIndex].Monster_maxHealth = 0;
						curAvatar->charInfoArray[messDeletePtr->characterIndex].Monster_health = 0;
						curAvatar->SaveAccount();
	                    // there's no valid avatar in that slot
	                    messNo.subType = 1;
	                    lserver->SendMsg(sizeof(messNo),(void *)&messNo, 0, &tempReceiptList);
	                }

	                // also send the names of the eight avatars
	                for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
	                {
	                    if (curAvatar->charInfoArray[i].topIndex > -1)
		                {
			                memcpy(messAvatarNames.name[i],
                                 curAvatar->charInfoArray[i].name,31);
				            messAvatarNames.name[i][31] = 0;
					    }
						else
	                        messAvatarNames.name[i][0] = 0;
		            }
			        lserver->SendMsg(sizeof(messAvatarNames),(void *)&messAvatarNames, 0, &tempReceiptList);
				}
				break;

	        case NWMESS_AVATAR_NEW_CLOTHES:
		        clothesPtr = (MessAvatarNewClothes *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            clothesPtr->avatarID = clothesPtr->avatarID % NUM_OF_CHARS_PER_USER;
		        curAvatar = FindIncomingAvatar(fromSocket);
			    if (curAvatar)
				{
	                if (clothesPtr->avatarID < 0 || clothesPtr->avatarID > 7)
		                clothesPtr->avatarID = 0;
						
	                if (-1 != curAvatar->charInfoArray[clothesPtr->avatarID].topIndex)
		            {
			            curAvatar->charInfoArray[clothesPtr->avatarID].topIndex =
				            clothesPtr->topIndex;		
					    curAvatar->charInfoArray[clothesPtr->avatarID].topR =
                            clothesPtr->topR;		
						curAvatar->charInfoArray[clothesPtr->avatarID].topG =
	                        clothesPtr->topG;		
						curAvatar->charInfoArray[clothesPtr->avatarID].topB =
                            clothesPtr->topB;		

						curAvatar->charInfoArray[clothesPtr->avatarID].bottomIndex =
                            clothesPtr->bottomIndex;		
						curAvatar->charInfoArray[clothesPtr->avatarID].bottomR =
                            clothesPtr->bottomR;		
						curAvatar->charInfoArray[clothesPtr->avatarID].bottomG =
                            clothesPtr->bottomG;		
						curAvatar->charInfoArray[clothesPtr->avatarID].bottomB =
                            clothesPtr->bottomB;		

						messAvatarStats.avatarID = clothesPtr->avatarID;
	                    messAvatarStats.faceIndex = 
							curAvatar->charInfoArray[clothesPtr->avatarID].faceIndex;
		                messAvatarStats.hairR = 
							curAvatar->charInfoArray[clothesPtr->avatarID].hairR;
			            messAvatarStats.hairG = 
							curAvatar->charInfoArray[clothesPtr->avatarID].hairG;
				        messAvatarStats.hairB = 
							curAvatar->charInfoArray[clothesPtr->avatarID].hairB;

	                    messAvatarStats.topIndex = 
		                    curAvatar->charInfoArray[clothesPtr->avatarID].topIndex;
			            messAvatarStats.topR = 
                            curAvatar->charInfoArray[clothesPtr->avatarID].topR;
						messAvatarStats.topG = 
                            curAvatar->charInfoArray[clothesPtr->avatarID].topG;
	                    messAvatarStats.topB = 
                            curAvatar->charInfoArray[clothesPtr->avatarID].topB;

						messAvatarStats.bottomIndex = 
                            curAvatar->charInfoArray[clothesPtr->avatarID].bottomIndex;
	                    messAvatarStats.bottomR = 
                            curAvatar->charInfoArray[clothesPtr->avatarID].bottomR;
						messAvatarStats.bottomG = 
						    curAvatar->charInfoArray[clothesPtr->avatarID].bottomG;
	                    messAvatarStats.bottomB = 
                            curAvatar->charInfoArray[clothesPtr->avatarID].bottomB;

	                    memcpy(messAvatarStats.name,
		                    curAvatar->charInfoArray[clothesPtr->avatarID].name,31);
	                    messAvatarStats.name[31] = 0;

		                messAvatarStats.physical =
			                curAvatar->charInfoArray[clothesPtr->avatarID].physical;
				        messAvatarStats.magical =		
					        curAvatar->charInfoArray[clothesPtr->avatarID].magical;
						messAvatarStats.creative =
							curAvatar->charInfoArray[clothesPtr->avatarID].creative;
						messAvatarStats.beast =
							curAvatar->charInfoArray[clothesPtr->avatarID].beast;

	                    messAvatarStats.imageFlags =		
		                    curAvatar->charInfoArray[clothesPtr->avatarID].imageFlags;

	                    lserver->SendMsg(sizeof(messAvatarStats),
                                      (void *)&messAvatarStats, 0, &tempReceiptList);

		                curAvatar->SaveAccount();

			            // also send the names of the eight avatars
				        for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
					    {
						    if (curAvatar->charInfoArray[i].topIndex > -1)
							{
	                            memcpy(messAvatarNames.name[i],
		                             curAvatar->charInfoArray[i].name,31);
			                    messAvatarNames.name[i][31] = 0;
				            }
					        else
                            messAvatarNames.name[i][0] = 0;
	                    }
		                lserver->SendMsg(sizeof(messAvatarNames),(void *)&messAvatarNames, 0, &tempReceiptList);
			        }
				}
	            break;

			case NWMESS_BUG_REPORT:
	            bugReportPtr = (MessBugReport *) messData;
		        tempReceiptList.clear();
			    tempReceiptList.push_back(fromSocket);
					
	            source = fopen("bugs.txt","a");
					
            
	           /* Display operating system-style date and time. */
				_strdate( tempText );
				fprintf(source, "%s, ", tempText );
				_strtime( tempText );
				fprintf(source, "%s, ", tempText );

				if (bugReportPtr->art)
					fprintf(source,"A, ");
				else
					fprintf(source,"-, ");
	            if (bugReportPtr->crash)
		            fprintf(source,"C, ");
			    else
				    fprintf(source,"-, ");
	            if (bugReportPtr->gameplay)
		            fprintf(source,"G, ");
	            else
		            fprintf(source,"-, ");
	            if (bugReportPtr->hang)
		            fprintf(source,"H, ");
			    else
	                fprintf(source,"-, ");
		        if (bugReportPtr->other)
			        fprintf(source,"O, ");
				else
	                fprintf(source,"-, ");
					
	            fprintf(source,"%s, ", bugReportPtr->doing);
		        fprintf(source,"%s, ", bugReportPtr->playLength);
			    fprintf(source,"%s, ", bugReportPtr->repeatable);
				fprintf(source,"%s,\n", bugReportPtr->info);

	            fclose(source);

				curAvatar = FindAvatar(fromSocket, &ss);
	            if (curAvatar)
		        {
			        sprintf(&chatMess.text[1],"Thanks for your bug report, %s!",
				         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
					chatMess.text[0] = TEXT_COLOR_DATA;
	                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
		            curAvatar->activeCounter = 0;
			    }
				break;

	        case NWMESS_ENTER_GAME:
		        enterGamePtr = (MessEnterGame *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            enterGamePtr->characterIndex = enterGamePtr->characterIndex % NUM_OF_CHARS_PER_USER;
		        curAvatar = FindIncomingAvatar(fromSocket);
			    if (curAvatar)
				{
	                if (-1 != curAvatar->charInfoArray[enterGamePtr->characterIndex].topIndex)
		            {
			            messYes.subType = fromSocket;
				        lserver->SendMsg(sizeof(messYes),(void *)&messYes, 0, &tempReceiptList);
	                    curAvatar->curCharacterIndex = enterGamePtr->characterIndex;
		                curAvatar->cellX = curAvatar->targetCellX = 
			                curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
				        curAvatar->cellY = curAvatar->targetCellY = 
					        curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;
					
	                    curAvatar->QuestSpaceChange(NULL, NULL);
		                TransferAvatar(TRUE, fromSocket);
						// init drake feed time
				        curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                                 petDragonInfo[0].lastEatenTime.SetToNow();
	                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                                 petDragonInfo[1].lastEatenTime.SetToNow();
		                // OFF-LINE HEAL
			            LongTime now;
				        DWORD timeElapsed = 
                                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                              lastSavedTime.MinutesDifference(&now);
					    curAvatar->charInfoArray[curAvatar->curCharacterIndex].health += 
						    timeElapsed * 6;

	                    if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health >
		                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax)
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].health =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;

	                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = NULL;
						sprintf_s(tempText, "**** new connection from IP %d.%d.%d.%d: %s, %s, %d ",
							curAvatar->IP[0],
							curAvatar->IP[1],
							curAvatar->IP[2],
							curAvatar->IP[3],
							curAvatar->name,
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
							curAvatar->uniqueId
						);
				        LogOutput("gamelog.txt", tempText);
							
	                    if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType || ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
		                {
			                LogOutput("moderatorLog.txt", tempText);
				        }
							
	                    LongTime lt;
		                lt.value.wHour += 19;
			            if (lt.value.wHour < 24)
				            lt.value.wDay -= 1;
					    else
						    lt.value.wHour -= 24;

	                    sprintf_s(tempText,"%d/%02d, %d:%02d\n", lt.value.wMonth, lt.value.wDay, 
		                    lt.value.wHour, lt.value.wMinute);
			            LogOutput("gamelog.txt", tempText);
				        if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType || ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
	                    {	
		                    LogOutput("moderatorLog.txt", tempText);
			            }
							
	                    lastConnectTime   = timeGetTime();
		                errorContactVal = 0;
						// remove all doomkeys
						InventoryObject *io;
						// destroy doomkeys from workbench
						io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.First();
						while (io)
						{
							if (INVOBJ_DOOMKEY == io->type)
							{
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.Remove(io);
								delete io;
							}	
							io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.Next();
						}
						// destroy doomkeys from inventory
						io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.First();
						while (io)
						{
							if (INVOBJ_DOOMKEY == io->type)
							{
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next();
						}
						// destroy doomkeys from wielded area
						io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.First();
						while (io)
						{
							if (INVOBJ_DOOMKEY == io->type)
							{
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Next();
						}
							
	                    ss = (SharedSpace *) spaceList->Find(SPACE_GROUND); // 'cause avatars appear
                                                                        // on land.

	                    if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime > 1)
		                {
			                sprintf_s(infoText.text,"%s entered the game.",
				                curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
					        ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                        sizeof(infoText),(void *)&infoText);
						}
	                    else
		                {
			                sprintf_s(infoText.text,"***  %s begins life!  ***",
				               curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
					        ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                        sizeof(infoText),(void *)&infoText);
	                    }
						if (TRUE)
						{

							sprintf_s(infoText.text, "Welcome to Blade Mistress.  Please check out the website (http://bm.kicks-ass.org/index.html).");
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}

						curAvatar->activeCounter = 0;

	                    TellBuddiesImHere(curAvatar);
							
						// and add monster if one was saved if you are a beastmaster
						
						if ((curAvatar->BeastStat() > 9) &&	curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_maxHealth > 0) // max health greater than 0 means there is a monster, and we need to birth it
						{
							
							int type = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_type;
							int subType = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_subType;
							float resist = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_magicResistance;
							if (monsterData[type][subType].name[0])
							{
								BBOSMonster *monster = new BBOSMonster(type, subType, NULL);
								if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_uniqueName != "NO NAME")
								{
									CopyStringSafely(curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_uniqueName, 32, monster->uniqueName, 32);
								}
								if (curAvatar->BeastStat() > 9)
								{
									// beastmaster pets get their names fixed omgwtf
									// check to make sure players name is at the front.
									if (strncmp(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, monster->uniqueName, strlen(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name)))
									{
										// if not, reset the name.
										sprintf(tempText, "%s's %s", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
											monsterData[type][subType].name);
										CopyStringSafely(tempText, 1024,
											monster->uniqueName, 31);
									}
								}
								monster->dontRespawn = TRUE;
								monster->cellX = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
								monster->cellY = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;
								monster->targetCellX = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
								monster->targetCellY = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;
								monster->spawnX = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
								monster->spawnY = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;

								monster->damageDone = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_damageDone;
								monster->defense = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_defense;
								monster->health = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_maxHealth;  // free pet heal on login
								monster->maxHealth = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_maxHealth;
								monster->toHit = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_toHit;
								monster->a = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_a;
								monster->b = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_b;
								monster->g = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_g;
								monster->r = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_r;
								monster->sizeCoeff = curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_sizeCoeff;
								monster->magicResistance = resist;
								ss->mobList->Add(monster);
									
								monster->isMoving = TRUE;
								monster->moveStartTime = timeGetTime() - 10000;

								MessMobAppear mobAppear;
								mobAppear.mobID = (unsigned long)monster;
								mobAppear.type = monster->WhatAmI();
								mobAppear.monsterType = monster->type;
								mobAppear.subType = monster->subType;
									
								mobAppear.x = monster->cellX;
								mobAppear.y = monster->cellY;
								ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
									sizeof(mobAppear), &mobAppear);
								monster->AnnounceMyselfCustom(ss);
								MessGenericEffect messGE;
								messGE.avatarID = -1;
								messGE.mobID = -1;
								messGE.x = monster->cellX;
								messGE.y = monster->cellY;
								messGE.r = 255;
								messGE.g = 0;
								messGE.b = 255;
								messGE.type = 0;  // type of particles
								messGE.timeLen = 1; // in seconds
								ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
									sizeof(messGE), (void *)&messGE);
								// and control it, for now
								// later will make it follow unless controlled if you are a beastmaster.
								curAvatar->controlledMonster = (BBOSMonster *)monster;
								curAvatar->controlledMonster->controllingAvatar = curAvatar;
								curAvatar->followMode = TRUE;		// activate followmode
									
							}
						}
						else if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast > 9) // if it's a beastmaster but doesn't have a monster, give it a default based on taming skill.
						{
							int type = curAvatar->GetTamingLevel();
							int subType = curAvatar->GetTamingExp();
							if (type < 1)														// if skill is missing, then start with the decaying golem.
							{
								type = 1;
								subType = 0; // just in case
							}
							while ((subType > 0) && !(monsterData[type][subType].name[0])) // decrease subtype until we find a name, or get to zero
								subType--;
							if (monsterData[type][subType].name[0])						// should always be true, but just in case.... 
							{
								BBOSMonster *monster = new BBOSMonster(type, subType, NULL);
								monster->dontRespawn = TRUE;
								sprintf(tempText, "%s's %s", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
									monsterData[type][subType].name);
								CopyStringSafely(tempText, 1024,
									monster->uniqueName, 31);

								monster->cellX = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
								monster->cellY = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;
								monster->targetCellX = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
								monster->targetCellY = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;
								monster->spawnX = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX;
								monster->spawnY = curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY;
								monster->dontRespawn = TRUE;

								ss->mobList->Add(monster);
								if (monster->damageDone < 4)  // controlled monsters that weak can't kill.
									monster->damageDone = 4;  // do at least enough to break default regen of 1 per second.
								monster->healAmountPerSecond = 1; // nerf regen just in case
								monster->isMoving = TRUE;
								monster->moveStartTime = timeGetTime() - 10000;
								monster->tamingcounter = 10000; // already tamed, don't let another beastmaster steal it.
								MessMobAppear mobAppear;
								mobAppear.mobID = (unsigned long)monster;
								mobAppear.type = monster->WhatAmI();
								mobAppear.monsterType = monster->type;
								mobAppear.subType = monster->subType;

								mobAppear.x = monster->cellX;
								mobAppear.y = monster->cellY;
								ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
									sizeof(mobAppear), &mobAppear);
								monster->AnnounceMyselfCustom(ss);
								MessGenericEffect messGE;
								messGE.avatarID = -1;
								messGE.mobID = -1;
								messGE.x = monster->cellX;
								messGE.y = monster->cellY;
								messGE.r = 255;
								messGE.g = 0;
								messGE.b = 255;
								messGE.type = 0;  // type of particles
								messGE.timeLen = 1; // in seconds
								ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
									sizeof(messGE), (void *)&messGE);
								// and control it, for now 
								// later will make it follow unless controlled if you are a beastmaster.
								curAvatar->controlledMonster = (BBOSMonster *)monster;
								curAvatar->controlledMonster->controllingAvatar = curAvatar;
								curAvatar->followMode = TRUE;		// activate followmode
							}
						}
	                    MessInfoFlags infoFlags;
		                infoFlags.flags = curAvatar->infoFlags;
			            lserver->SendMsg(sizeof(infoFlags),(void *)&infoFlags, 0, &tempReceiptList);

	                    if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
						{
							MessAdminMessage adminMess;
							adminMess.messageType = MESS_ADMIN_ACTIVATE;
							lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						}
						if (curAvatar->BeastStat()>9)  // if a beastmaster
						{
							MessAdminMessage adminMess;
							adminMess.messageType = MESS_BEASTMASTER_ACTIVATE; // send beastmaster mode activation message.
							lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						}

						MessChatChannel returnCChan;
		                returnCChan.value = curAvatar->chatChannels;
			            lserver->SendMsg(sizeof(returnCChan),(void *)&returnCChan, 0, &tempReceiptList);

	                    MessWeatherState state;
		                state.value = weatherState;
			            lserver->SendMsg(sizeof(state),(void *)&state, 0, &tempReceiptList);
							
	                    if (!curAvatar->isReferralDone && curAvatar->hasPaid)
						{
							sprintf_s(infoText.text, "Thanks for paying! Remember to use the /referredby <name>");
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

							sprintf_s(infoText.text, "command to reward the player who introduced you to Blade Mistress.");
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}

		        }
				break;

	        case NWMESS_EXIT_GAME:
		        tempReceiptList.clear();
			    tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);
	            if (curAvatar)
		        {
			        TransferAvatar(FALSE, fromSocket);

	                sprintf_s(tempText,"** log-off character: %s, %s ",curAvatar->name,
		                  curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
			        LogOutput("gamelog.txt", tempText);
				    if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType || ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
					{
					    LogOutput("moderatorLog.txt", tempText);
					}


	                LongTime lt;
		            lt.value.wHour += 19;
			        if (lt.value.wHour < 24)
				    {
					    lt.value.wDay -= 1;
					}
					else
						lt.value.wHour -= 24;


	                sprintf_s(tempText,"%d/%02d, %d:%02d\n", (int)lt.value.wMonth, (int)lt.value.wDay, 
		                (int)lt.value.wHour, (int)lt.value.wMinute);
				        LogOutput("gamelog.txt", tempText);
	                if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType || ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
	                {
	                    LogOutput("moderatorLog.txt", tempText);
		            }

	                curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_NOTHING);
						
	                sprintf_s(infoText.text,"%s left the game.",
                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
					ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                    sizeof(infoText),(void *)&infoText);

					curAvatar->QuestSpaceChange(ss,NULL);

					if ((SPACE_GROUND != ss->WhatAmI()) || //not in normal realm
						((SPACE_GROUND == ss->WhatAmI()) && (((GroundMap *)ss)->type>0)))	// or in the alternate big map
					{
				        if (SPACE_GUILD == ss->WhatAmI())
					    {
						    curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX = ((TowerMap *)ss)->enterX;
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY = ((TowerMap *)ss)->enterY-1;
						}
						else
						{
	                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX = 
		                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].spawnX;
			                curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY = 
				                curAvatar->charInfoArray[curAvatar->curCharacterIndex].spawnY;
					    }
	                }
		            else
			        {
				        curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastX = 
					        curAvatar->cellX;
	                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].lastY = 
		                    curAvatar->cellY;
			        }
				    curAvatar->SaveAccount();
					if (curAvatar->controlledMonster) // if there'a  a controlled monster
					{ // remove it 
											// die with no loot for anyone!
						for (int i = 0; i < 10; ++i)
							curAvatar->controlledMonster->attackerPtrList[i] = NULL;
						curAvatar->controlledMonster->dropAmount = 0; // dont' drop anything!
						curAvatar->controlledMonster->type = 25; //make it a vamp 
						curAvatar->controlledMonster->isDead = TRUE; // it's dead
						curAvatar->controlledMonster->controllingAvatar = NULL;	// and unlink it
						curAvatar->controlledMonster = NULL;	// and unlink it
					}
	            }
				break;

	        case NWMESS_INVENTORY_REQUEST_INFO:
		        invRequestInfoPtr = (MessInventoryRequestInfo *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);
		        if (curAvatar)
			    {
				    curAvatar->activeCounter = 0;
					if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex)
					{
	                    // tell the client what it wants to know about an inventory
		                if (MESS_INVENTORY_SAME == invRequestInfoPtr->type &&
			                curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner &&
				            MESS_INVENTORY_TRADER == invRequestInfoPtr->which)
					    {
						    curAvatar->indexList[MESS_INVENTORY_TRADER] = invRequestInfoPtr->offset;
							TellClientAboutInventory(curAvatar, 
                                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                             inventory->partner->subType);
	                    }
		                else if (MESS_INVENTORY_SAME == invRequestInfoPtr->type)
			            {
				            curAvatar->indexList[curAvatar->lastPlayerInvSent] = invRequestInfoPtr->offset;
					        TellClientAboutInventory(curAvatar, invRequestInfoPtr->which);
						}
	                    else if (MESS_INVENTORY_PLAYER == invRequestInfoPtr->type)
		                {
			                TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
				        }
					    else if (MESS_WORKBENCH_PLAYER == invRequestInfoPtr->type)
						{
	                        TellClientAboutInventory(curAvatar, MESS_WORKBENCH_PLAYER);
		                }
			            else if (MESS_SKILLS_PLAYER == invRequestInfoPtr->type)
				        {
					        TellClientAboutInventory(curAvatar, MESS_SKILLS_PLAYER);
						}
	                    else if (MESS_WIELD_PLAYER == invRequestInfoPtr->type)
		                {
			                TellClientAboutInventory(curAvatar, MESS_WIELD_PLAYER);
				        }
					    else if (MESS_INVENTORY_TOWER == invRequestInfoPtr->type)
						{
	                        SharedSpace *sx;
								
							// Players now have to be in the guild AND paying to use the chest.
	                        if( ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType )
		                    {
			                    sprintf_s(infoText.text,"Trial moderators cannot open any guild chests.");
				                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
					        }
						    else if (FindAvatarInGuild(curAvatar->charInfoArray[
							                        curAvatar->curCharacterIndex].name, &sx) &&
                                                        sx == ss)
							{
	                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner =
	                                (((TowerMap *) sx)->itemBox);
		                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving = TRUE;
			                    TellClientAboutInventory(curAvatar, MESS_INVENTORY_TOWER);
				            }
					        else
						    {
							    sprintf_s(infoText.text,"You are not allowed to open this chest. You must be in the guild and a paying player.");
								lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
	                        }
		                }
			            else if (MESS_INVENTORY_YOUR_SECURE == invRequestInfoPtr->type)
				        {
					        BBOSAvatar *partnerAvatar;
						    partnerAvatar = FindAvatar((int)invRequestInfoPtr->which, &ss);
							if (MESS_INVENTORY_TRADER == invRequestInfoPtr->which)
	                            partnerAvatar = curAvatar->tradingPartner;

		                    // anti-duping check
			                if (partnerAvatar &&
				                ( strlen(partnerAvatar->name) < 1 ||
					            IsCompletelyVisiblySame(partnerAvatar->name, curAvatar->name) ))
						        partnerAvatar = NULL;

	                        if (partnerAvatar)
		                    {
			                    LongTime now;
									
	                            if (false && now.MinutesDifference(&curAvatar->accountExperationTime) <= 0) // no more trade expire restrictions
		                        {
			                        sprintf_s(infoText.text,"I'm sorry, but your account is expired.  You cannot trade.");
				                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
					            }
						        else if (false && now.MinutesDifference(&partnerAvatar->accountExperationTime) <= 0) // no more trade expire restrictions
							    {
								    sprintf_s(infoText.text,"That person's account is expired.  They cannot trade.");
									lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
								}
								else if (!curAvatar->tradingPartner && !partnerAvatar->tradingPartner)
		                        {
	                                curAvatar->tradingPartner     = partnerAvatar;
			                        partnerAvatar->tradingPartner = curAvatar;
				                    curAvatar->agreeToTrade = FALSE;
					                partnerAvatar->agreeToTrade = FALSE;
										
	                                curAvatar->indexList[MESS_INVENTORY_YOUR_SECURE] = 0;
		                            curAvatar->indexList[MESS_INVENTORY_HER_SECURE] = 0;
			                        partnerAvatar->trade->money = 0;
				                    curAvatar->trade->money = 0;

	                                MessSecurePartnerName partnerNameMess;

	                                sprintf_s(tempText,"You begin trading with %s.",
		                                 partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name);
			                        CopyStringSafely(tempText,1024,infoText.text, MESSINFOTEXTLEN);
				                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

	                                sprintf_s(partnerNameMess.name,
		                                partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name); 
			                        partnerNameMess.name[10] = 0;	 // truncate
				                    ss->lserver->SendMsg(sizeof(partnerNameMess),(void *)&partnerNameMess, 0, &tempReceiptList);

	                                tempReceiptList.clear();
		                            tempReceiptList.push_back(partnerAvatar->socketIndex);
			                        sprintf_s(tempText,"%s begins trading with you.",
				                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
									CopyStringSafely(tempText,1024,infoText.text, MESSINFOTEXTLEN);
									ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

	                                sprintf_s(partnerNameMess.name,
		                                curAvatar->charInfoArray[curAvatar->curCharacterIndex].name); 
			                        partnerNameMess.name[10] = 0;	 // truncate
				                    ss->lserver->SendMsg(sizeof(partnerNameMess),(void *)&partnerNameMess, 0, &tempReceiptList);

	                                TellClientAboutInventory(curAvatar, MESS_INVENTORY_YOUR_SECURE);
		                            TellClientAboutInventory(curAvatar, MESS_INVENTORY_HER_SECURE);
									
	                                TellClientAboutInventory(partnerAvatar, MESS_INVENTORY_YOUR_SECURE);
									TellClientAboutInventory(partnerAvatar, MESS_INVENTORY_HER_SECURE);
	                            }
		                        else
			                    {
				                    if (curAvatar->tradingPartner == partnerAvatar && 
					                     partnerAvatar->tradingPartner == curAvatar)
						            {
							            curAvatar->indexList[MESS_INVENTORY_YOUR_SECURE] = 
								            invRequestInfoPtr->offset;
									    TellClientAboutInventory(curAvatar, MESS_INVENTORY_YOUR_SECURE);
	                                }
		                            else if (partnerAvatar->tradingPartner)
			                        {
				                        sprintf_s(infoText.text,"That person is busy trading with someone else.");
					                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
						            }
							    }
							}
						}
						else if (MESS_INVENTORY_HER_SECURE == invRequestInfoPtr->type)
						{
	                        BBOSAvatar *partnerAvatar;
		                    partnerAvatar = FindAvatar((int)invRequestInfoPtr->which, &ss);
			                if (MESS_INVENTORY_TRADER == invRequestInfoPtr->which)
				                partnerAvatar = curAvatar->tradingPartner;
					        if (partnerAvatar)
						    {
							    if (curAvatar->tradingPartner == partnerAvatar && 
									partnerAvatar->tradingPartner == curAvatar)
								{
	                                curAvatar->indexList[MESS_INVENTORY_HER_SECURE] = 
		                                    invRequestInfoPtr->offset;
			                            TellClientAboutInventory(curAvatar, MESS_INVENTORY_HER_SECURE);
				                }
					            else if (partnerAvatar->tradingPartner)
						        {
							        sprintf_s(infoText.text,"That person is busy trading with someone else.");
								    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
								}
	                        }
		                }
			            else if (MESS_INVENTORY_TRADER == invRequestInfoPtr->type && invRequestInfoPtr->which)
						{
	                        curNpc = (BBOSNpc *) ss->mobList->IsInList((BBOSMob *)invRequestInfoPtr->which, TRUE);
		                    if (curNpc && curNpc->cellX == curAvatar->cellX && curNpc->cellY == curAvatar->cellY &&
					             (SMOB_TRADER == curNpc->WhatAmI() || SMOB_TRAINER == curNpc->WhatAmI() || SMOB_PLAYERMERCHANT == curNpc->WhatAmI()))
	                        {
		                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner =
                                (curNpc->inventory);
	                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving = FALSE;

	                            if (SMOB_TRADER == curNpc->WhatAmI() || SMOB_PLAYERMERCHANT == curNpc->WhatAmI())
	                                TellClientAboutInventory(curAvatar, MESS_INVENTORY_TRADER);
	                            else if (SMOB_TRAINER == curNpc->WhatAmI())
	                                TellClientAboutInventory(curAvatar, MESS_INVENTORY_TRAINER);
	                        }
						}
						else if (MESS_INVENTORY_GROUND == invRequestInfoPtr->type)
	                    {
		                    tempX = curAvatar->cellX;
			                tempY = curAvatar->cellY;
				            if (curAvatar->controlledMonster)
					        {
						        SharedSpace *sx;

								BBOSMonster * theMonster = FindMonster(
                                      curAvatar->controlledMonster, &sx);
								if (theMonster)
		                        {
	                                curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner =
			                            ss->GetGroundInventory(theMonster->cellX,theMonster->cellY);
				                }
					        }
						    else
							{
	                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner =
		                            ss->GetGroundInventory(tempX,tempY);
			                }

	                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving = TRUE;

	                        TellClientAboutInventory(curAvatar, MESS_INVENTORY_GROUND);
	                    }
	                }
	            }
	           break;

	        case NWMESS_INVENTORY_TRANSFER_REQUEST:
	            transferRequestPtr = (MessInventoryTransferRequest *) messData;
		        tempReceiptList.clear();
			    tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);
				if ((curAvatar)&&(curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0))
				{
					MessInfoText infoText;
					sprintf_s(tempText, "You are stunned, and can't do that!");
					memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
					infoText.text[MESSINFOTEXTLEN - 1] = 0;
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					break;
				}
		        if (curAvatar)
			    {
				    curAvatar->activeCounter = 0;

	                if (curAvatar->tradingPartner == NULL &&
		                transferRequestPtr->isPlayerInfo &&
			            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner &&
				        (MESS_INVENTORY_GROUND == curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner->subType) &&
					    strlen(((InventoryObject*)transferRequestPtr->ptr)->do_name) > 1 &&
						strncmp("Demon Amulet", ((InventoryObject*)transferRequestPtr->ptr)->do_name, strlen(((InventoryObject*)transferRequestPtr->ptr)->do_name)) &&
						strncmp("Dragon Orchid", ((InventoryObject*)transferRequestPtr->ptr)->do_name, strlen(((InventoryObject*)transferRequestPtr->ptr)->do_name)) &&
						strncmp("Ancient Dragonscale", ((InventoryObject*)transferRequestPtr->ptr)->do_name, strlen(((InventoryObject*)transferRequestPtr->ptr)->do_name))
                    )
					{
	                    sprintf_s(infoText.text,"You can only put Amulets, Dragonscales and Orchids onto the ground.");
		                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
			        }
				    else if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex)
					{

	                    int retVal = TransferItem(curAvatar, transferRequestPtr, transferRequestPtr->amount,
		                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving);
			            if (1 == retVal)
				        {
					        UpdateInventory(curAvatar);
	                        TellClientAboutInventory(curAvatar, curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner->subType);

	                        if (MESS_INVENTORY_GROUND == curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner->subType)
		                    {
			                    tempX = curAvatar->cellX;
				                tempY = curAvatar->cellY;
					            SharedSpace *sx = NULL;
									
	                            if (curAvatar->controlledMonster)
		                        {
			                        BBOSMonster * theMonster = FindMonster(
				                        curAvatar->controlledMonster, &sx);
	                                if (theMonster)
		                            {
			                            tempX = theMonster->cellX;
				                        tempY = theMonster->cellY;
					                }
						        }

	                            if (sx)
		                            ss = sx;

	                            if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].
	                                  inventory->partner->objects.IsListEmpty())
		                        {
			                        messMobDisappear.mobID = (unsigned long)
				                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                              inventory->partner;
					                messMobDisappear.x = tempX;
						            messMobDisappear.y = tempY;
							        ss->SendToEveryoneNearBut(0, tempX, tempY,
								        sizeof(messMobDisappear),(void *)&messMobDisappear);
								}
								else
	                            {
		                            // tell client about an item sack
			                        MessMobAppear messMA;
				                    messMA.mobID = (unsigned long) 
					                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].
						                inventory->partner;
							        messMA.type = SMOB_ITEM_SACK;
								    messMA.x = tempX;
									messMA.y = tempY;
									ss->SendToEveryoneNearBut(0, tempX, tempY,
                                        sizeof(messMA),(void *)&messMA);
	                            }
		                    }
			            }
				        else if (2 == retVal)
					    {
						    UpdateInventory(curAvatar);
						}

	                }
		        }
			   break;

	        case NWMESS_INVENTORY_CHANGE:
		        inventoryChangePtr = (MessInventoryChange *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);
		        if (curAvatar)
			    {
				    curAvatar->activeCounter = 0;
					if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex)
					{

	                    if (ShiftItem(curAvatar, inventoryChangePtr))
		                {
			                switch(inventoryChangePtr->srcListType)
				            {
					        case GTM_BUTTON_LIST_INV:
						        TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
							    break;
							case GTM_BUTTON_LIST_WRK:
								TellClientAboutInventory(curAvatar, MESS_WORKBENCH_PLAYER);
								break;
	                        case GTM_BUTTON_LIST_SKL:
		                        TellClientAboutInventory(curAvatar, MESS_SKILLS_PLAYER);
			                    break;
				            case GTM_BUTTON_LIST_WLD:
					            TellClientAboutInventory(curAvatar, MESS_WIELD_PLAYER);
						        break;
							}
						}
	                }
				}
			break;

			case NWMESS_WIELD:
				break;  // huh? this an old packet?

			case NWMESS_UNWIELD:
				break;

			case NWMESS_AVATAR_ATTACK:
				messAAPtr = (MessAvatarAttack *)messData;
				curAvatar = FindAvatar(fromSocket, &ss);
				if (curAvatar)
				{
					if (curAvatar->controlledMonster)
					{
						SharedSpace *sx, *sz;

						BBOSMonster * theMonster = FindMonster(curAvatar->controlledMonster, &sx);
						if (theMonster)
						{
							// theMonster needs to attack a target avatar
							BBOSAvatar * curAvatar2 = FindAvatar(messAAPtr->mobID, &sx);
							if (curAvatar2) 
							{
								if ((curAvatar->accountType == ACCOUNT_TYPE_ADMIN)        // if i'm an admin
									|| (curAvatar->PVPEnabled && curAvatar2->PVPEnabled)) // or PVP is on for both characters.
								{
									if (!theMonster->curTarget) // if we do NOT laready have a target
									{
										theMonster->lastAttackTime = 0; // then the attack is immediate.
									}
									theMonster->curTarget = curAvatar2;
								}
							}
							else
							{
								BBOSMonster * targetMonster =
									FindMonster((BBOSMob *)messAAPtr->mobID, &sz);
								if (targetMonster)                                         // beastmasters can attaack monsters
								{
									if ((!targetMonster->controllingAvatar) // if monster isn't controlled
										|| ((targetMonster->controllingAvatar) && (targetMonster->controllingAvatar->accountType != ACCOUNT_TYPE_PLAYER)) // or it's controlled by an admin
										|| ((targetMonster->controllingAvatar) && (ss->WhatAmI()==SPACE_REALM) && (((RealmMap*)ss)->type == REALM_ID_TEST) ) // or it's a beastmaster pet on test realm
										|| ((targetMonster->controllingAvatar && targetMonster->controllingAvatar->PVPEnabled && curAvatar->PVPEnabled))) // or it's contorllers pvp flag is on and mine is too
									{
										if (!theMonster->curMonsterTarget) // if we do not already have a monster target
										{
											theMonster->lastAttackTime = 0;
										}
										theMonster->curMonsterTarget = targetMonster;
									}
								}
							}
						}
					}
					else if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex && messAAPtr->mobID)
					{
						curMob = (BBOSMob *)ss->mobList->IsInList((BBOSMob *)messAAPtr->mobID, TRUE);
						if (curMob &&
							curMob->cellX == curAvatar->cellX &&
							curMob->cellY == curAvatar->cellY && SMOB_MONSTER == curMob->WhatAmI())
						{	// pvp/admin/controlled monster check
							if (!(((BBOSMonster *)curMob)->controllingAvatar)												// if it's not controlled
								|| ((((BBOSMonster *)curMob)->controllingAvatar)->accountType != ACCOUNT_TYPE_PLAYER)		// or is controlled by not a normal player
								|| (((((BBOSMonster *)curMob)->controllingAvatar)->PVPEnabled) && curAvatar->PVPEnabled))	// or PVP is on.
							{
								if (!curAvatar->curTarget)              // if we didn't have a target
									curAvatar->lastAttackTime = 0;		// then the attack is immediate
								curAvatar->curTarget = (BBOSMonster *)curMob;
							}
						}
						SharedSpace *sx;
						BBOSAvatar * curAvatar2 = FindAvatar(messAAPtr->mobID, &sx);
						if (curAvatar2
							&& curAvatar->PVPEnabled && curAvatar2->PVPEnabled
							) // will add pvp check
						{
								if (!curAvatar->curPlayerTarget && !curAvatar->curTarget) // if we do NOT already have a target
								{
									curAvatar->lastAttackTime = 0; // then the attack is immediate.
								}
								curAvatar->curPlayerTarget = curAvatar2;
							
						}
					}
				}
				break;
			case NWMESS_START_TAME:
				messSTPtr = (MessStartTame *)messData;
				curAvatar = FindAvatar(fromSocket, &ss);
				if (curAvatar)
				{
					if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex && messSTPtr->mobID)
					{
						curMob = (BBOSMob *)ss->mobList->IsInList((BBOSMob *)messSTPtr->mobID, TRUE);
						if (curMob &&
							curMob->cellX == curAvatar->cellX &&
							curMob->cellY == curAvatar->cellY && SMOB_MONSTER == curMob->WhatAmI())
						{	// need to abort if it's a controlled monster AND it's not an admin to stop players from murdering peoples pets.
							if (!(((BBOSMonster *)curMob)->controllingAvatar) || ((((BBOSMonster *)curMob)->controllingAvatar)->accountType != ACCOUNT_TYPE_ADMIN))
								if (!curAvatar->curTarget)
									curAvatar->lastAttackTime = 0;
							curAvatar->curTarget = (BBOSMonster *)curMob;
						}

					}
				}
				break;

	        case NWMESS_TRY_COMBINE:
		        messTryCombinePtr = (MessTryCombine *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);
		        if (curAvatar)
			    {
				    curAvatar->activeCounter = 0;
					if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex &&
						messTryCombinePtr->skillID)
	                {
		                iObject = (InventoryObject *) 
			                curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.Find(
				                (InventoryObject *) messTryCombinePtr->skillID);
					    if (iObject && INVOBJ_SKILL == iObject->type) // if found the object
						{
							HandleCombine(curAvatar, iObject->WhoAmI()); // check for stunn is in here
						}
						else
						{
	                        char *skillNamePtr = NULL;
								
							for (i = 0; i < 19; ++i)
		                    {
			                    if (skillHotKeyArray[i] == messTryCombinePtr->skillID)
				                {
					                skillNamePtr = skillNameArray[i];
						            // UsedHotkey=TRUE;
							    }
	                        }
								
	                        if (skillNamePtr)
		                    {
			                    iObject = (InventoryObject *) 
				                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                           skills->objects.Find(skillNamePtr);
					            if (iObject && INVOBJ_SKILL == iObject->type) // if found the object
						        {
							        HandleCombine(curAvatar, iObject->WhoAmI()); // check for stun is in here
								}
	                        }
		                }
			        }
				}
				break;

	        case NWMESS_REQUEST_DUNGEON_INFO:
		        messRDInfoPtr = (MessRequestDungeonInfo *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);
		        if (curAvatar && SPACE_DUNGEON == ss->WhatAmI())
			    {
				    if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex)
					{
						messDInfo.floor = 0;
	                    messDInfo.sizeX = ((DungeonMap *)ss)->width;
		                messDInfo.sizeY = ((DungeonMap *)ss)->height;
			            messDInfo.x     = messRDInfoPtr->x;
				        messDInfo.y     = messRDInfoPtr->y;
							
	                    messDInfo.floor     = ((DungeonMap *)ss)->floorIndex;
		                messDInfo.outerWall = ((DungeonMap *)ss)->outerWallIndex;
							
	                    for (i = 0; i < DUNGEON_PIECE_SIZE; ++i)
		                {
			                for (j = 0; j < DUNGEON_PIECE_SIZE; ++j)
				            {
					            messDInfo.leftWall[i * DUNGEON_PIECE_SIZE + j] = 0;
						        if (i+ messDInfo.y * DUNGEON_PIECE_SIZE < 0 ||
							        i + messDInfo.y * DUNGEON_PIECE_SIZE >= messDInfo.sizeY ||
								    j + messDInfo.x * DUNGEON_PIECE_SIZE < 0 ||
									j + messDInfo.x * DUNGEON_PIECE_SIZE >= messDInfo.sizeX)
								{
	                                messDInfo.leftWall[i * DUNGEON_PIECE_SIZE + j] = 0;
		                        }
			                    else
				                {
					                messDInfo.leftWall[i * DUNGEON_PIECE_SIZE + j] = ((DungeonMap *)ss)->leftWall[
										(i + messDInfo.y * DUNGEON_PIECE_SIZE) * ((DungeonMap *)ss)->width +
                                        j + messDInfo.x * DUNGEON_PIECE_SIZE
                                                                    ];
	                            }

	                            messDInfo.topWall[i * DUNGEON_PIECE_SIZE + j] = 0;
		                        if (i+ messDInfo.y * DUNGEON_PIECE_SIZE < 0 ||
			                        i + messDInfo.y * DUNGEON_PIECE_SIZE >= messDInfo.sizeY ||
				                    j + messDInfo.x * DUNGEON_PIECE_SIZE < 0 ||
									j + messDInfo.x * DUNGEON_PIECE_SIZE >= messDInfo.sizeX)
								{
									messDInfo.topWall[i * DUNGEON_PIECE_SIZE + j] = 0;
								}
	                            else
		                        {
			                        messDInfo.topWall[i * DUNGEON_PIECE_SIZE + j] = 
				                        ((DungeonMap *)ss)->topWall[
					                    (i + messDInfo.y * DUNGEON_PIECE_SIZE) * ((DungeonMap *)ss)->width +
                                        j + messDInfo.x * DUNGEON_PIECE_SIZE];
								}
							}
						}

					    lserver->SendMsg(sizeof(messDInfo),
                                      (void *)&messDInfo, 0, &tempReceiptList);

	                }
		        }
			    else if (curAvatar && SPACE_GUILD == ss->WhatAmI())
				{
	                if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex)
		            {
			            messDInfo.floor = 0;
				        messDInfo.sizeX = ((TowerMap *)ss)->width;
					    messDInfo.sizeY = ((TowerMap *)ss)->height;
						messDInfo.x     = messRDInfoPtr->x;
	                    messDInfo.y     = messRDInfoPtr->y;
							
						messDInfo.floor     = ((TowerMap *)ss)->floorIndex;
	                    messDInfo.outerWall = ((TowerMap *)ss)->outerWallIndex;
							
	                    for (i = 0; i < DUNGEON_PIECE_SIZE; ++i)
		                {
			                for (j = 0; j < DUNGEON_PIECE_SIZE; ++j)
				            {
								messDInfo.leftWall[i * DUNGEON_PIECE_SIZE + j] = 0;
						        if (i+ messDInfo.y * DUNGEON_PIECE_SIZE < 0 ||
									i + messDInfo.y * DUNGEON_PIECE_SIZE >= messDInfo.sizeY ||
								    j + messDInfo.x * DUNGEON_PIECE_SIZE < 0 ||
									j + messDInfo.x * DUNGEON_PIECE_SIZE >= messDInfo.sizeX)
								{
									messDInfo.leftWall[i * DUNGEON_PIECE_SIZE + j] = 0;
								}
								else
	                            {
		                            messDInfo.leftWall[i * DUNGEON_PIECE_SIZE + j] = 
										((TowerMap *)ss)->leftWall[
										(i + messDInfo.y * DUNGEON_PIECE_SIZE) * ((TowerMap *)ss)->width +
                                        j + messDInfo.x * DUNGEON_PIECE_SIZE];
								}

	                            messDInfo.topWall[i * DUNGEON_PIECE_SIZE + j] = 0;
		                        if (i+ messDInfo.y * DUNGEON_PIECE_SIZE < 0 ||
									i + messDInfo.y * DUNGEON_PIECE_SIZE >= messDInfo.sizeY ||
				                    j + messDInfo.x * DUNGEON_PIECE_SIZE < 0 ||
									j + messDInfo.x * DUNGEON_PIECE_SIZE >= messDInfo.sizeX)
								{
	                                messDInfo.topWall[i * DUNGEON_PIECE_SIZE + j] = 0;
		                        }
			                    else
				                {
					                messDInfo.topWall[i * DUNGEON_PIECE_SIZE + j] = 
										((TowerMap *)ss)->topWall[
										(i + messDInfo.y * DUNGEON_PIECE_SIZE) * ((TowerMap *)ss)->width +
                                        j + messDInfo.x * DUNGEON_PIECE_SIZE];
	                            }
		                    }
			            }
							
	                    lserver->SendMsg(sizeof(messDInfo),
		                    (void *)&messDInfo, 0, &tempReceiptList);

					}
				}

				break;

	        case NWMESS_DUNGEON_CHANGE:
				messDungeonChangePtr = (MessDungeonChange *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				curAvatar = FindAvatar(fromSocket, &ss);
				if (curAvatar && SPACE_DUNGEON == ss->WhatAmI())
				{
					int go = FALSE;
					if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex &&
						 ((DungeonMap *) ss)->CanEdit(curAvatar) )
						go = TRUE;

					if (go)
					{
						messDungeonChange.floor = messDungeonChange.left = 
							messDungeonChange.outer = messDungeonChange.top = 
							messDungeonChange.reset = 0;
						messDungeonChange.x = messDungeonChangePtr->x;
						messDungeonChange.y = messDungeonChangePtr->y;

						if (messDungeonChangePtr->floor)
						{
							((DungeonMap *)ss)->floorIndex += 1;
							if (((DungeonMap *)ss)->floorIndex >= NUM_OF_DUNGEON_FLOOR_TYPES)
								((DungeonMap *)ss)->floorIndex = 0;

							messDungeonChange.floor = ((DungeonMap *)ss)->floorIndex + 1; 
							ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
						}
						else if (messDungeonChangePtr->outer)
						{
							((DungeonMap *)ss)->outerWallIndex += 1;
							if (((DungeonMap *)ss)->outerWallIndex >= NUM_OF_DUNGEON_WALL_TYPES)
								((DungeonMap *)ss)->outerWallIndex = 0;

							messDungeonChange.outer = ((DungeonMap *)ss)->outerWallIndex + 1; 
							ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
						}
						else if (messDungeonChangePtr->left)
						{
							int result = ((DungeonMap *)ss)->ChangeWall(TRUE,
											  messDungeonChangePtr->x,
												  messDungeonChangePtr->y);

							if (result >= 0)
							{
								messDungeonChange.left = result; 
								ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
							}
							else if (-2 == result)
							{
								sprintf_s(infoText.text,"Edit outside bounds denied.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
							else if (-1 == result)
							{
								sprintf_s(infoText.text,"That would close off parts of the dungeon.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
						}
						else if (messDungeonChangePtr->top)
						{
							int result = ((DungeonMap *)ss)->ChangeWall(FALSE,
											  messDungeonChangePtr->x,
												  messDungeonChangePtr->y);

							if (result >= 0)
							{
								messDungeonChange.top = result; 
								ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
							}
							else if (-2 == result)
							{
								sprintf_s(infoText.text,"Edit outside bounds denied.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
							else if (-1 == result)
							{
								sprintf_s(infoText.text,"That would close off parts of the dungeon.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
						}
					}
				}
				else if (curAvatar && SPACE_GUILD == ss->WhatAmI())
				{
	//				if (-1 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].topIndex &&
	//					 !strcmp(((TowerMap *) ss)->masterName, curAvatar->name) &&
	//					 !strcmp(((TowerMap *) ss)->masterPass, curAvatar->pass)
	//					)
					if (TRUE)
					{
						((TowerMap *)ss)->lastChangedTime.SetToNow();

						messDungeonChange.floor = messDungeonChange.left = 
							messDungeonChange.outer = messDungeonChange.top = 0;
						messDungeonChange.x = messDungeonChangePtr->x;
						messDungeonChange.y = messDungeonChangePtr->y;

						if (messDungeonChangePtr->floor)
						{
							((TowerMap *)ss)->floorIndex += 1;
							if (((TowerMap *)ss)->floorIndex >= NUM_OF_TOWER_FLOOR_TYPES)
								((TowerMap *)ss)->floorIndex = 0;

							messDungeonChange.floor = ((TowerMap *)ss)->floorIndex + 1; 
							ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
						}
						else if (messDungeonChangePtr->outer)
						{
							((TowerMap *)ss)->outerWallIndex += 1;
							if (((TowerMap *)ss)->outerWallIndex >= NUM_OF_TOWER_WALL_TYPES)
								((TowerMap *)ss)->outerWallIndex = 0;

							messDungeonChange.outer = ((TowerMap *)ss)->outerWallIndex + 1; 
							ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
						}
						else if (messDungeonChangePtr->left)
						{
							int result = ((TowerMap *)ss)->ChangeWall(TRUE,
											  messDungeonChangePtr->x,
												  messDungeonChangePtr->y);

							if (result >= 0)
							{
								messDungeonChange.left = result; 
								ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
							}
							else if (-2 == result)
							{
								sprintf_s(infoText.text,"Edit outside bounds denied.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
							else if (-1 == result)
							{
								sprintf_s(infoText.text,"That would close off parts of the tower.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
						}
						else if (messDungeonChangePtr->top)
						{
							int result = ((TowerMap *)ss)->ChangeWall(FALSE,
											  messDungeonChangePtr->x,
												  messDungeonChangePtr->y);

							if (result >= 0)
							{
								messDungeonChange.top = result; 
								ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
											sizeof(messDungeonChange),(void *)&messDungeonChange);
							}
							else if (-2 == result)
							{
								sprintf_s(infoText.text,"Edit outside bounds denied.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
							else if (-1 == result)
							{
								sprintf_s(infoText.text,"That would close off parts of the tower.");
								ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
							}
						}
					}
				}
				break;
                                            
	        case NWMESS_REQUEST_AVATAR_INFO:
		        messRequestAvatarInfo = (MessRequestAvatarInfo *) messData;
			    tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

	            curAvatar = FindAvatar(fromSocket, &ss);

	            if (!curAvatar)
                break;

		        curMob = (BBOSMob *) ss->avatars->First();
			    while (curMob)
				{
	                BBOSAvatar *av = (BBOSAvatar *) curMob;
		            if (SMOB_AVATAR == curMob->WhatAmI() &&
			             messRequestAvatarInfo->avatarID == av->socketIndex)
				    {
					    MessAvatarStats mStats;
	                    av->BuildStatsMessage(&mStats);
		                ss->lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);
							
		                // tell people about my cool dragons!
	                    for (int index = 0; index < 2; ++index)
			            {
				            if (255 != av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].type)
					        {
						        // tell everyone about it!
							    MessPet mPet;
								mPet.avatarID = av->socketIndex;
	                            CopyStringSafely(av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].name,16, 
		                            mPet.name,16);
			                    mPet.quality = av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].quality;
				                mPet.type    = av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].type;
					            mPet.state   = av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].state;
						        mPet.size    = av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].lifeStage +
                                    av->charInfoArray[av->curCharacterIndex].petDragonInfo[index].healthModifier / 10.0f;
							    mPet.which   = index;

	                            ss->lserver->SendMsg(sizeof(mPet),(void *)&mPet, 0, &tempReceiptList);
		                    }
			            }

	                    av->AssertGuildStatus(ss, FALSE, fromSocket);

	                    InventoryObject *iObject = (InventoryObject *) 
		                    av->charInfoArray[av->curCharacterIndex].wield->objects.First();
			            while (iObject)
				        {
					        if (INVOBJ_BLADE == iObject->type)
						    {
							    FillBladeDescMessage(&messBladeDesc, iObject, av);
								ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
	                            iObject = (InventoryObject *) 
		                            av->charInfoArray[av->curCharacterIndex].wield->objects.Last();
			                }
				            else if (INVOBJ_STAFF == iObject->type)
					        {
						        InvStaff *iStaff = (InvStaff *) iObject->extra;
							    
								MessBladeDesc messBladeDesc;
	                            messBladeDesc.bladeID = (long)iObject;
		                        messBladeDesc.size    = 4;
			                    messBladeDesc.r       = staffColor[iStaff->type][0];
				                messBladeDesc.g       = staffColor[iStaff->type][1];
					            messBladeDesc.b       = staffColor[iStaff->type][2];
						        messBladeDesc.avatarID= av->socketIndex;
							    messBladeDesc.trailType  = 0;
								messBladeDesc.meshType = BLADE_TYPE_STAFF1;
	                            ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
		                        iObject = (InventoryObject *) 
			                        av->charInfoArray[av->curCharacterIndex].wield->objects.Last();
				            }
								
	                        iObject	= (InventoryObject *) 
		                        av->charInfoArray[av->curCharacterIndex].wield->objects.Next();
			            }

	                    curMob = NULL;
		            }
			        else
				        curMob = (BBOSMob *) ss->avatars->Next();
				}

	            break;

			case NWMESS_REQUEST_TOWNMAGE_SERVICE:
				messTMSPtr = (MessRequestTownMageService *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				curAvatar = FindAvatar(fromSocket, &ss);
				SharedSpace *sx;
				if (!curAvatar)
					break;
			    // town mage is ok to use even when stunned	
				curMob = (BBOSMob *) ss->mobList->GetFirst(curAvatar->cellX, curAvatar->cellY);
				while (curMob)
				{
					if (SMOB_TOWNMAGE == curMob->WhatAmI()) // handle town mage packets
					{
						if (curMob->cellX == curAvatar->cellX && curMob->cellY == curAvatar->cellY)
						{
							switch(messTMSPtr->which)
							{
							case TMSERVICE_TELEPORT:
								HandleTeleport(curAvatar, curMob, ss, TRUE); 
								break;
							case TMSERVICE_TELEPORT_BACK:
								HandleTeleport(curAvatar, curMob, ss, FALSE); 
								break;
							case TMSERVICE_HEAL:
								{
									if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money < 180)
									{
										sprintf_s(infoText.text,"You don't have 180 gold for healing.");
										ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
										break;
									}

									if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health >=
										 curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax)
									{
										sprintf_s(infoText.text,"You are already fully healed.");
										ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
										break;
									}
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money 
												   -= 180;

									assert(
										curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

									long amount = 50;
									amount -= curAvatar->GetDodgeLevel() * 3;
									if (amount < 3)
										amount = 3;
									long realAmount = amount;
									if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax -
										 curAvatar->charInfoArray[curAvatar->curCharacterIndex].health	<
										 realAmount)
										realAmount =
										curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax -
										curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;

									curAvatar->charInfoArray[curAvatar->curCharacterIndex].health += amount;
									if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health >
										 curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax)
									{
										curAvatar->charInfoArray[curAvatar->curCharacterIndex].health =
										curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
									}

									MessAvatarHealth messHealth;
									messHealth.health    = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
									messHealth.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
									messHealth.avatarID  = curAvatar->socketIndex;
									ss->lserver->SendMsg(sizeof(messHealth),(void *)&messHealth, 0, &tempReceiptList);

									sprintf_s(infoText.text,"You are healed for %d points.",amount);
									ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

									curAvatar->AnnounceSpecial(ss, SPECIAL_APP_HEAL);

									source = fopen("healing.txt","a");
                                
									/* Display operating system-style date and time. */
									_strdate( tempText );
									fprintf(source, "%s, ", tempText );
									_strtime( tempText );
									fprintf(source, "%s, ", tempText );

									fprintf(source,"%s, %ld, %ld\n",
											 curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
											 amount, realAmount);

									fclose(source);
								}
								break;
							case TMSERVICE_BANK:
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = curAvatar->bank;
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving = TRUE;
								TellClientAboutInventory(curAvatar, MESS_INVENTORY_BANK);
								break;
							}
						}
					}
					curMob = (BBOSMob *) ss->mobList->GetNext();
				}
				if ((SPACE_GUILD == ss->WhatAmI()) && (messTMSPtr->which == TMSERVICE_BANK) && ((FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &sx) && sx == ss)))
				{
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = curAvatar->bank;
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving = TRUE;
					TellClientAboutInventory(curAvatar, MESS_INVENTORY_BANK);
				}
				if ((SPACE_GUILD == ss->WhatAmI()) && (messTMSPtr->which == TMSERVICE_TELEPORT) && ((FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &sx) && sx == ss)))
				{
					HandleTeleport(curAvatar, curMob, ss, TRUE);
					break;
				}
				break;

			case NWMESS_AVATAR_SEND_MONEY:
				messMoney = (MessAvatarSendMoney *) messData;
				// honestly, this shouldn't work while stunned, but what's the harm?  if nothing else in the code blocks it this will work.
				curAvatar = FindAvatar(fromSocket, &ss);
				if (curAvatar)
				{
					if (-1 == messMoney->avatarID && -2 == messMoney->targetAvatarID)
					{
						// change money in secure trading
						if (curAvatar->tradingPartner)
						{
							SharedSpace *sx;
							BBOSAvatar *partnerAvatar = NULL;
							partnerAvatar = FindAvatar(curAvatar->tradingPartner, &sx);

							if (partnerAvatar)
							{
								assert(
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

								curAvatar->StateNoAgreement(ss);

								assert(
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

								// copy back previous amount
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money +=
									curAvatar->trade->money;

								assert(
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

								long change = messMoney->amount;
								if (messMoney->amount <= 0)
									change = 0;
								else if (messMoney->amount > 
												curAvatar->charInfoArray[curAvatar->curCharacterIndex].
												inventory->money)
								{
									assert(change >= 0);
									change = curAvatar->charInfoArray[curAvatar->curCharacterIndex].
												inventory->money;
									assert(change >= 0);
								}

								curAvatar->charInfoArray[curAvatar->curCharacterIndex].
												 inventory->money -= change;
								assert(curAvatar->trade->money >= 0);
								curAvatar->trade->money = change;
								assert(curAvatar->trade->money >= 0);

								assert(
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

								TellClientAboutInventory(curAvatar, MESS_INVENTORY_YOUR_SECURE);
								TellClientAboutInventory(partnerAvatar, MESS_INVENTORY_HER_SECURE);

								TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
							}
						}
					}
					else if (-1 == messMoney->avatarID && -3 == messMoney->targetAvatarID)
					{
						// change money in bank
						// copy back previous amount
	//					curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money +=
	//						curAvatar->bank->money;

						long change = messMoney->amount;

						if (change > 10000000)
							change = 10000000;
						if (change < -10000000)
							change = -10000000;

						if (change >= 0)
						{
							if (change > curAvatar->charInfoArray[curAvatar->curCharacterIndex].
										inventory->money)
								change = curAvatar->charInfoArray[curAvatar->curCharacterIndex].
										inventory->money;
						}
						else
						{
							if (-change > curAvatar->bank->money)
								 change = -curAvatar->bank->money;
						}

						curAvatar->charInfoArray[curAvatar->curCharacterIndex].
										 inventory->money -= change;
						curAvatar->bank->money += change;

						assert(
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money >= 0);

						TellClientAboutInventory(curAvatar, MESS_INVENTORY_BANK);
						TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
                
					}
				}

            
				break;

			case NWMESS_CHEST_INFO:
				messChestPtr = (MessChestInfo *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				curAvatar = FindAvatar(fromSocket, &ss);
				// can't open a chest while stunned. :)

				if	(curAvatar)
				{
					if (curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0)
					{
						MessInfoText infoText;
						sprintf_s(tempText, "You are stunned, and cannot open a chest!");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						break;
					}

					if	(SPACE_DUNGEON == ss->WhatAmI())
					{
						curMob = (BBOSMob *) ss->mobList->GetFirst(0,0,1000);
						while (curMob)
						{
							if (SMOB_CHEST == curMob->WhatAmI() && messChestPtr->mobID == (long) curMob)
							{
								BBOSChest *c = (BBOSChest *) curMob;
								if (!c->isOpen)
								{
									// open it
									c->isOpen = TRUE;
									c->openTime = timeGetTime();

									// tell everyone near that it's open
									MessChestInfo chestInfo;
									chestInfo.mobID = (unsigned long) curMob;
									chestInfo.type = 1;
									ss->SendToEveryoneNearBut(0, c->cellX, c->cellY, sizeof(chestInfo),(void *)&chestInfo);

									// spill the treasure
									((DungeonMap *)ss)->DoChestDrop(c);
								}
								else
								{
									sprintf_s(infoText.text,"The chest is open and empty.");
									ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
								}

								ss->mobList->SetToLast();
							}
							curMob = (BBOSMob *) ss->mobList->GetNext();
						}
					}
					else
					{
						sprintf_s(infoText.text,"You can't open it.");
						ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
					}
				}
				break;

			case NWMESS_TALK_TO_TREE:
				messTreePtr = (MessTalkToTree *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				curAvatar = FindAvatar(fromSocket, &ss);

				if	(curAvatar)
				{
					if (curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0) // this shouldn'd happen anyway, but...
					{
						MessInfoText infoText;
						sprintf_s(tempText, "You are stunned, and cannot talk to the tree!");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						break;
					}

					HandleTreeTalk(curAvatar, ss, messTreePtr);
				}
				break;

			case NWMESS_TEST_PING:
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				lserver->SendMsg(sizeof(messTP),(void *)&messTP, 0, &tempReceiptList);
				break;

			case NWMESS_PET_NAME:
				messPNPtr = (MessPetName *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				curAvatar = FindAvatar(fromSocket, &ss);
				
				GuaranteeTermination(messPNPtr->text, 16);
				CorrectString(messPNPtr->text);
				CleanString(messPNPtr->text);
				// harmless
				if	(curAvatar && messPNPtr->text[0] != 0)
				{
					BBOSCharacterInfo *charac = 
						  &curAvatar->charInfoArray[curAvatar->curCharacterIndex];

					if (charac->petDragonInfo[0].type != 255 && 
						 (0 == charac->petDragonInfo[0].name[0] ||
						  !strcmp(charac->petDragonInfo[0].name,"Pet A")))
					{
						CopyStringSafely(messPNPtr->text,16, charac->petDragonInfo[0].name,16);

						MessPet mPet;
						mPet.avatarID = curAvatar->socketIndex;
						CopyStringSafely(charac->petDragonInfo[0].name,16, mPet.name,16);
						mPet.quality = charac->petDragonInfo[0].quality;
						mPet.type    = charac->petDragonInfo[0].type;
						mPet.state   = charac->petDragonInfo[0].state;
						mPet.size    = charac->petDragonInfo[0].lifeStage +
								charac->petDragonInfo[0].healthModifier / 10.0f;
						mPet.which   = 0;

						ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
							sizeof(mPet),(void *)&mPet);
					}
					else if (charac->petDragonInfo[1].type != 255 && 
						 (0 == charac->petDragonInfo[1].name[0] ||
						  !strcmp(charac->petDragonInfo[1].name,"Pet B")))
					{
						CopyStringSafely(messPNPtr->text,16, charac->petDragonInfo[1].name,16);

						MessPet mPet;
						mPet.avatarID = curAvatar->socketIndex;
						CopyStringSafely(charac->petDragonInfo[1].name,16, mPet.name,16);
						mPet.quality = charac->petDragonInfo[1].quality;
						mPet.type    = charac->petDragonInfo[1].type;
						mPet.state   = charac->petDragonInfo[1].state;
						mPet.size    = charac->petDragonInfo[1].lifeStage +
								charac->petDragonInfo[1].healthModifier / 10.0f;
						mPet.which   = 1;

						ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
							sizeof(mPet),(void *)&mPet);
					}
				}
				break;

			case NWMESS_FEED_PET_REQUEST:
				messFPRPtr = (MessFeedPetRequest *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);

				curAvatar = FindAvatar(fromSocket, &ss);

				if	(curAvatar)
				{
					if (curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0) 
					{
						MessInfoText infoText;
						sprintf_s(tempText, "You are stunned, and cannot feed your pet!");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						break;
					}

					HandlePetFeeding(messFPRPtr, curAvatar, ss);
				}
				break;

			case NWMESS_ADMIN_MESSAGE:
				adminMessPtr = (MessAdminMessage *) messData;

				curAvatar = FindAvatar(fromSocket, &ss);

				if	(curAvatar && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					HandleAdminMessage(adminMessPtr, curAvatar, ss);
				}
				else if ((curAvatar->BeastStat()>9) && ((adminMessPtr->messageType==MESS_PLAYER_RECALL)|| (adminMessPtr->messageType == MESS_PLAYER_CONTROL)))												// beastmasters can send recall command
				{
					HandleAdminMessage(adminMessPtr, curAvatar, ss); // beastmasters can still recall their pets if they are stunned
				}
				break;

			case NWMESS_INFO_FLAGS:
				infoFlagsPtr = (MessInfoFlags *) messData;

				curAvatar = FindAvatar(fromSocket, &ss);

				if	(curAvatar)
				{
					curAvatar->infoFlags = infoFlagsPtr->flags;
					tempReceiptList.clear();
					tempReceiptList.push_back(fromSocket);
					lserver->SendMsg(sizeof(MessInfoFlags),(void *)infoFlagsPtr, 0, &tempReceiptList);
				}
				break;

			case NWMESS_SELL_ALL:
				sellAllPtr = (MessSellAll *) messData;

				curAvatar = FindAvatar(fromSocket, &ss);
				// this shouldn't work, but is harmless, so we can allow it stunned.
				if	(curAvatar)
				{
					curNpc = (BBOSNpc *) ss->mobList->IsInList((BBOSMob *)sellAllPtr->which, TRUE);
					if (curNpc &&  
						 curNpc->cellX == curAvatar->cellX &&
						 curNpc->cellY == curAvatar->cellY &&
						 SMOB_TRADER == curNpc->WhatAmI()
						)
					{
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner =
							(curNpc->inventory);
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->giving = FALSE;
	//					curNpc->inventory.subType = curNpc->WhatAmI();

	//					TellClientAboutInventory(curAvatar, MESS_INVENTORY_TRADER);

						HandleSellingAll(curAvatar, curNpc, sellAllPtr->type);
					}
				}
				break;

			case NWMESS_SECURE_TRADE:
				secureTradePtr = (MessSecureTrade *) messData;

				curAvatar = FindAvatar(fromSocket, &ss);
				if	(curAvatar)
				{
					switch (secureTradePtr->type)
					{
					case MESS_SECURE_STOP:
					default:
						curAvatar->AbortSecureTrading(ss);
						break;

					case MESS_SECURE_ACCEPT:
						if (curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0) // this is unlikely to ever happen, but....
						{
							curAvatar->AbortSecureTrading(ss);                     // can't trade if stunned.
							break;
						}

						curAvatar->agreeToTrade = TRUE;
						if (curAvatar->tradingPartner)
						{
							SharedSpace *sx;
							BBOSAvatar *partnerAvatar = NULL;
							partnerAvatar = FindAvatar(curAvatar->tradingPartner, &sx);

							if (partnerAvatar && partnerAvatar->agreeToTrade)
								curAvatar->CompleteSecureTrading(ss);
						}
						break;
					}
				}
				break;

			case NWMESS_EXTENDED_INFO_REQUEST:
				requestExInfoPtr = (MessExtendedInfoRequest *) messData;

				curAvatar = FindAvatar(fromSocket, &ss);
				if	(curAvatar)
				{
					HandleExtendedInfo(curAvatar, requestExInfoPtr);
				}
				break;

			case NWMESS_KEYCODE:
				keyCodePtr = (MessKeyCode *) messData;
				codePad = cryptoText3;
				UnCryptoString(keyCodePtr->string);

				curAvatar = FindIncomingAvatar(fromSocket);
				if	(curAvatar)
				{
					HandleKeyCode(curAvatar, keyCodePtr);
				}
				break;

			case NWMESS_SET_BOMB:
				setBombPtr = (MessSetBomb *) messData;
				tempReceiptList.clear();
				tempReceiptList.push_back(fromSocket);                          // set recipt list, because we need to send stunned message.

				curAvatar = FindAvatar(fromSocket, &ss);
				if	(curAvatar)                                                 // this covers bombs, recalls, releasing pets, and earthkeys
				{
					// this covers bombs, recalls, and earthkeys
					if (curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0)
					{
						MessInfoText infoText;
						sprintf_s(tempText, "You are stunned, and can't do that!");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						break;
					}

					InventoryObject *detonatedBomb = NULL;

					curInventory = 
						   curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;
					iObject = (InventoryObject *)curInventory->objects.First();
					while (iObject)
					{
						if (setBombPtr->ptr == (long)iObject && INVOBJ_BOMB == iObject->type) // found the bomb to set
						{
							// if you are in pvp
							if (curAvatar->PVPEnabled)
							{
								// then you need ot be a smith to set the bomb
								if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative < 10) // check non stat totem stat!
								{
									MessInfoText infoText;
									sprintf_s(tempText, "Only smiths can set bombs in PVP zones.");
									memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
									infoText.text[MESSINFOTEXTLEN - 1] = 0;
									lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									break;
								}
							}
							InvBomb *ib = (InvBomb *) iObject->extra;

							BBOSBomb *bomb = new BBOSBomb(curAvatar);
							bomb->cellX = curAvatar->cellX;
							bomb->cellY = curAvatar->cellY;
							bomb->type  = ib->type;
							bomb->flags = ib->flags;
							bomb->r     = ib->r;
							bomb->g     = ib->g;
							bomb->b     = ib->b;
							bomb->power = ib->power;
							bomb->detonateTime = timeGetTime() + ib->fuseDelay * 1000;
							if (ib->stability < rnd(0,1))
								bomb->detonateTime = timeGetTime();

							ss->mobList->Add(bomb);

							sprintf_s(tempText,"%s sets a bomb!",
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
												sizeof(infoText),(void *)&infoText);

							detonatedBomb = iObject;
							iObject = (InventoryObject *)curInventory->objects.Last();
							// send appearance message to client fixxme
							MessMobAppear mobAppear;
							mobAppear.mobID = bomb->do_id;
							mobAppear.type = SMOB_BOMB;
							mobAppear.monsterType = 0;  // normal bomb
							mobAppear.subType = 0;		// boss bomb will be different.
							mobAppear.staticMonsterFlag = FALSE; // not static
							mobAppear.x = bomb->cellX;			// in this square
							mobAppear.y = bomb->cellY;
							ss->SendToEveryoneNearBut(0, bomb->cellX, bomb->cellY, sizeof(mobAppear), (void *)&mobAppear); // inform everyone who should be able to see it.
						}
						else if (setBombPtr->ptr == (long)iObject && INVOBJ_POTION == iObject->type) // it's a potion
						{
							InvPotion *ip = (InvPotion *) iObject->extra;

							if (POTION_TYPE_RECALL      == ip->type || 
								 POTION_TYPE_DARK_RECALL == ip->type)
							{

								if (curAvatar->controlledMonster && !curAvatar->followMode) // if we have a monster
								{
									// and make my monster vanish too
									MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
									tempReceiptList.clear();
									tempReceiptList.push_back(curAvatar->socketIndex); // point server at the target client.
									adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
									lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
									if (curAvatar->BeastStat() > 9)
										curAvatar->followMode = TRUE;
								}
								TransferAvatar(FALSE, fromSocket);
								if (curAvatar->controlledMonster)
								{
									MessMonsterDeath messMD;
									messMD.mobID = (unsigned long)curAvatar->controlledMonster;
									ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
										sizeof(messMD), (void *)&messMD);
									ss->mobList->Remove(curAvatar->controlledMonster);
								}
								curAvatar->QuestSpaceChange(NULL, NULL);

								tempReceiptList.clear();
								tempReceiptList.push_back(fromSocket);
								
								MessChangeMap changeMap;

								changeMap.oldType = ss->WhatAmI(); 
								changeMap.newType = SPACE_GROUND; 
								changeMap.sizeX   = MAP_SIZE_WIDTH;
								changeMap.sizeY   = MAP_SIZE_HEIGHT;
								changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
								changeMap.realmID = 0;
								lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

								if (POTION_TYPE_RECALL == ip->type)
								{
									curAvatar->cellX = curAvatar->targetCellX = 
										townList[ip->subType].x;
									curAvatar->cellY = curAvatar->targetCellY = 
										townList[ip->subType].y;

									curAvatar->charInfoArray[curAvatar->curCharacterIndex].spawnX = 
										townList[ip->subType].x;
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].spawnY = 
										townList[ip->subType].y;
									if (curAvatar->controlledMonster)
									{
										curAvatar->controlledMonster->cellX = townList[ip->subType].x;
										curAvatar->controlledMonster->cellY = townList[ip->subType].y;

									}
								}
								else if (POTION_TYPE_DARK_RECALL == ip->type)
								{
									int theX = 220;
									int theY = 220;
									if (1 == ip->subType)
									{
										theX = 90;
										theY = 53;
									}
									curAvatar->cellX = curAvatar->targetCellX = theX;
									curAvatar->cellY = curAvatar->targetCellY = theY;
									if (curAvatar->controlledMonster)
									{
										curAvatar->controlledMonster->cellX = theX;
										curAvatar->controlledMonster->cellY = theY;

									}
								}
								SharedSpace *sp = (SharedSpace *)spaceList->Find(SPACE_GROUND); // in case we need it for someone controling a monster.
								if (curAvatar->controlledMonster)
								{
									sp->mobList->Add(curAvatar->controlledMonster);
								}

								TransferAvatar(TRUE, fromSocket);
							}
							else if (POTION_TYPE_TOWER_RECALL == ip->type)
							{
								// find the tower
								SharedSpace *sp;
								if (FindGuild(ip->subType, &sp))
								{
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = NULL;
									// gotta check for controlling monster
									// am i controlling a monster?
									if (curAvatar->controlledMonster && !curAvatar->followMode)
									{
										// and make my monster vanish too
										MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
										tempReceiptList.clear();
										tempReceiptList.push_back(curAvatar->socketIndex); // point server at the target client.
										adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
										lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
										// and resturn server to follow mode, since both will end up in the same spot.
										if (curAvatar->BeastStat() > 9)
											curAvatar->followMode = TRUE;
										// we need to dissapear it from people's screen.
									}
									// second, vanish me
									curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);

									tempReceiptList.clear();
									tempReceiptList.push_back(fromSocket);
									// third, vanish the controlled monster if present
									if (curAvatar->controlledMonster)
									{
										// we need to dissapear it from people's screen.
										MessMonsterDeath messMD;
										messMD.mobID = (unsigned long)curAvatar->controlledMonster;
										ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
											sizeof(messMD), (void *)&messMD);
										// remove it fropm old map
										ss->mobList->Remove(curAvatar->controlledMonster);

									}
									// next adjust the coordinates
									ss->avatars->Remove(curAvatar);
									curAvatar->cellX = curAvatar->targetCellX = 4;
									curAvatar->cellY = curAvatar->targetCellY = 4;
									sp->avatars->Append(curAvatar);

									if (curAvatar->controlledMonster) // if they had a monster
									{
										// then it needs ot have it's coords updated, and be added to new map
										curAvatar->controlledMonster->cellX = curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
										curAvatar->controlledMonster->cellY = curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
										sp->mobList->Add(curAvatar->controlledMonster);
									}
									MessChangeMap changeMap;
									changeMap.dungeonID = (long)sp;
									changeMap.oldType = ss->WhatAmI(); 
									changeMap.newType = sp->WhatAmI(); 
									changeMap.sizeX   = 5;
									changeMap.sizeY   = 5;
									changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!

									lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

									MessInfoText infoText;
									sprintf_s(tempText,"You enter the guild tower of %s.", ((TowerMap *) sp)->WhoAmI());
									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
									lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

									curAvatar->QuestSpaceChange(ss,sp);

									// move me to my new SharedSpace

									((TowerMap *)sp)->lastChangedTime.SetToNow();

									// (POTION_TYPE_TOWER_RECALL == ip->type);
									// tell everyone about my arrival
									curAvatar->IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
									if (curAvatar->controlledMonster)
									{
										// tell clients about my monster
										curAvatar->controlledMonster->AnnounceMyselfCustom(sp);
									}
									// tell this player about everyone else around
									curAvatar->UpdateClient(sp, TRUE);
								}
							}
							else if (POTION_TYPE_MERCHANT_SUMMON == ip->type) // it's a merchant-in-a-bra
							{
								// search square for other merchants 
								int FoundMerchant = FALSE;
								// get list of mosnters in the square
								BBOSMob *curMob = (BBOSMob *)ss->mobList->GetFirst(curAvatar->cellX, curAvatar->cellY);
								while (curMob)
								{
									if (curMob->WhatAmI() == SMOB_TRADER) // if it's a merchant
									{
										// tell the player there's already a merchant and abort
										tempReceiptList.clear();
										tempReceiptList.push_back(fromSocket);
										sprintf_s(infoText.text, "There's already a merchant in your square.");
										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										FoundMerchant = TRUE;
									}
									curMob = ss->mobList->GetNext(); // get next one in square
								}
								if (!FoundMerchant)				// if i didn't find a merchant
								{
									BBOSNpc *npc = new BBOSNpc(SMOB_TRADER);
									InventoryObject *iObject2; // fix stupid exploit
									// add recall scrolls
									int ii;
									char tempText[1024];
									for (ii = 0; ii < NUM_OF_TOWNS; ii++)
									{

										sprintf(tempText, "%s Recall", townList[ii].name);
										iObject2 = new InventoryObject(INVOBJ_POTION, 0, tempText);
										InvPotion *extra = (InvPotion *)iObject2->extra;
										extra->type = POTION_TYPE_RECALL;
										extra->subType = ii;

										iObject2->mass = 0.0f;
										iObject2->value = 1000;
										iObject2->amount = 20 + (rand() % 10);
										npc->inventory->AddItemSorted(iObject2);
									}
									// add a complete set of totems here
									for (int i = 0; i < TOTEM_SELLABLE_QUALITY_MAX; ++i)
									{
										int type = rand() % TOTEM_MAX;
										int qual = i;
										if (qual >= TOTEM_SELLABLE_QUALITY_MAX)
											qual = TOTEM_SELLABLE_QUALITY_MAX - 1;  // pumpkin and beyond; don't sell

																					//				char name[128];
																					//				sprintf(name,"%s %s Totem", totemQualityName[qual], totemTypeName[type]);

										iObject2 = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
										InvTotem *extra = (InvTotem *)iObject2->extra;
										extra->type = type;
										extra->quality = qual;

										iObject2->mass = 0.0f;
										iObject2->value = qual * qual * 14 + 1;
										if (qual > 12)
											iObject2->value = qual * qual * 14 + 1 + (qual - 12) * 1600;
										iObject2->amount = 10;
										UpdateTotem(iObject2);
										npc->inventory->AddItemSorted(iObject2);
									}

									// add bomb making supplies here

									{
										iObject2 = new InventoryObject(INVOBJ_EXPLOSIVE, 0, "Charcoal Brick");
										InvExplosive *extra = (InvExplosive *)iObject2->extra;
										extra->type = 0;
										extra->quality = 1.0f; // explosive power of this one item
										iObject2->value = 10;
										iObject2->amount = rand() % 30 + 50;
										npc->inventory->AddItemSorted(iObject2);
									}
									{
										iObject2 = new InventoryObject(INVOBJ_FUSE, 0, "Twisted Paper");
										InvFuse *extra = (InvFuse *)iObject2->extra;
										extra->type = 0;
										extra->quality = 1.0f;
										iObject2->value = 10;
										iObject2->amount = rand() % 30 + 50;
										npc->inventory->AddItemSorted(iObject2);
									}

									// add dragonscales
									{
										iObject2 = new InventoryObject(INVOBJ_SIMPLE, 0, "Ancient Dragonscale");
										iObject2->value = 100000;
										iObject2->amount = rand() % 30 + 40;
										npc->inventory->AddItemSorted(iObject2);

										iObject2 = new InventoryObject(INVOBJ_SIMPLE, 0, "Dragon Orchid");
										iObject2->value = 10000;
										iObject2->amount = rand() % 30 + 50;
										npc->inventory->AddItemSorted(iObject2);

										iObject2 = new InventoryObject(INVOBJ_SIMPLE, 0, "Demon Amulet");
										iObject2->value = 50000;
										iObject2->amount = 20;
										npc->inventory->AddItemSorted(iObject2);
									}

									// add some staffs here
									{
										for (int qual = 0; qual < 3; ++qual)
										{
											iObject2 = new InventoryObject(INVOBJ_STAFF, 0, "Unnamed Staff");
											InvStaff *extra = (InvStaff *)iObject2->extra;
											extra->type = 0;
											extra->quality = qual;

											iObject2->mass = 0.0f;
											iObject2->value = 500 * (qual + 1) * (qual + 1);
											iObject2->amount = rand() % 10 + 1;
											UpdateStaff(iObject2, 0);
											npc->inventory->AddItemSorted(iObject2);
										}
									}

									// add beads
									for (int da = 1; da < 6; ++da)
									{
										sprintf(tempText, "%s Bead", dropAdj2[da - 1]);
										iObject2 = new InventoryObject(INVOBJ_GEOPART, 0, tempText);
										InvGeoPart *exIn = (InvGeoPart *)iObject2->extra;
										exIn->type = 0;
										exIn->power = da;

										iObject2->mass = 0.0f;
										iObject2->value = 1000 * da * da;
										iObject2->amount = 400 / da;
										npc->inventory->AddItemSorted(iObject2);
									}

									// and the requested ingots. quantity fixed at 100. 

									// tin
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Tin Ingot");
									InvIngot *exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 1;
									exIn->challenge = 1;
									exIn->r = 128;
									exIn->g = 128;
									exIn->b = 128;

									iObject2->mass = 1.0f;
									iObject2->value = 1;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);

									// Aluminum
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Aluminum Ingot");
									exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 2;
									exIn->challenge = 2;
									exIn->r = 168;
									exIn->g = 168;
									exIn->b = 168;

									iObject2->mass = 1.0f;
									iObject2->value = 5;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);

									// steel
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Steel Ingot");
									exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 3;
									exIn->challenge = 3;
									exIn->r = 228;
									exIn->g = 128;
									exIn->b = 128;

									iObject2->mass = 1.0f;
									iObject2->value = 15;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);

									// carbon
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Carbon Ingot");
									exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 4;
									exIn->challenge = 4;
									exIn->r = 128;
									exIn->g = 128;
									exIn->b = 228;

									iObject2->mass = 1.0f;
									iObject2->value = 50;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);

									// zinc
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Zinc Ingot");
									exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 5;
									exIn->challenge = 5;
									exIn->r = 128;
									exIn->g = 228;
									exIn->b = 128;

									iObject2->mass = 1.0f;
									iObject2->value = 250;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);

									// Adamantium
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Adamantium Ingot");
									exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 6;
									exIn->challenge = 6;
									exIn->r = 28;
									exIn->g = 28;
									exIn->b = 48;

									iObject2->mass = 1.0f;
									iObject2->value = 1000;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);

									// Mithril
									iObject2 = new InventoryObject(INVOBJ_INGOT, 0, "Mithril Ingot");
									exIn = (InvIngot *)iObject2->extra;
									exIn->damageVal = 7;
									exIn->challenge = 7;
									exIn->r = 228;
									exIn->g = 228;
									exIn->b = 228;

									iObject2->mass = 1.0f;
									iObject2->value = 10000;
									iObject2->amount = 400;
									npc->inventory->AddItemSorted(iObject2);


									npc->level = 0;
									npc->townIndex = 0;
									npc->cellX = curAvatar->cellX;
									npc->cellY = curAvatar->cellY;
									ss->mobList->Add(npc);
									curAvatar->GiveInfoFor(curAvatar->cellX, curAvatar->cellY, ss);
								}
								else
								{
									// otherwise increment the counter so we won't lose the item
									iObject->amount++;
								}
							}
						
							detonatedBomb = iObject;
							iObject = (InventoryObject *)curInventory->objects.Last();
						}
						else if (setBombPtr->ptr == (long)iObject && ((INVOBJ_EARTHKEY == iObject->type) || (INVOBJ_EARTHKEY_RESUME == iObject->type))) // it's an earthkey or resumer
						{
							InvEarthKey *iek = (InvEarthKey *)iObject->extra;
							tempReceiptList.clear();
							tempReceiptList.push_back(fromSocket);

							if ((SPACE_GROUND == ss->WhatAmI()) && (((GroundMap*)ss)->type==0))
							{
								// inside a town?
								int isInTown = FALSE;
								for (int i = 0; i < NUM_OF_TOWNS; ++i)
								{
									if (abs(townList[i].x - curAvatar->cellX) <= 3 &&
										abs(townList[i].y - curAvatar->cellY) <= 3)
									{
										isInTown = TRUE;
									}
								}

								if (!isInTown)
								{
									// we call either HandleEarthkeyUse, or the resume version depending on if they should get a resume or not.
									// if it's a normal earthkey, always call earthkeyuse
									if (INVOBJ_EARTHKEY == iObject->type)
									HandleEarthKeyUse(curAvatar, iek, ss);
									else  // it's a resumer, do other more complicated checks
									{ 
										// check that there's only one monster type, and that it's a legit skiller monster
										if (iek->monsterType[1]>-1) // if there is a second monster type, it can't possibly be a skiller
											HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not reate a replacement.
										else if (iek->monsterType[0] < 4)  // before skeleton, it can't be a skiller
											HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not reate a replacement.
										else if ((iek->monsterType[0] > 4) && (iek->monsterType[0] < 9))  // between skeleton ad spirit vision, it can't be a skiller
											HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not create a replacement.
										else if (iek->monsterType[0] == 11)  // spirit minos are not skillers because they also have cents.
											HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not create a replacement.
										else if (iek->monsterType[0] == 16)  // spirit dragons are not skillers because they also have cents.
											HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not create a replacement.
										else if (iek->monsterType[0] > 17)  // after spirit spider, it can't be a skiller
											HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not reate a replacement.
										else // now we need to actually check the single monster's level and compare to my dodge.
										{
											int mydodge = curAvatar->GetDodgeLevel(); // my dodge
											if (mydodge < 1) // if we don't haeve dodge
											{
												// check for Pet Energy instead
												mydodge = curAvatar->GetPetEnergyLevel();
											}
											long monsterToHit = monsterData[iek->monsterType[0]][0].toHit; // monster base to hit
											float change = 1 + iek->power / 20; // get multiplication factor.
											int rottedearthkey = 0;				// check for negative size to flag for rotted
											if (iek->height < 0)
												rottedearthkey = 1;
											if (iek->width < 0)
												rottedearthkey = 1;
											if (rottedearthkey == 1)
												change += 0.25; // correct it
											// now we apply multiplication factor to base to hit
											monsterToHit *= change;
											// and now compare with mine
											if (monsterToHit<mydodge) // too low to give experience
												HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not create a replacement.
											else if (monsterToHit-mydodge>9) // why nine?  your dodge is lowered by one to determine difference. it's too high to give dodge.
												HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not create a replacement.
											else // check if anyone else is within the square. no mroe resuming if there is someone. 
											{
												int avatarcount = 0;
												BBOSMob *curMob = (BBOSMob *)ss->avatars->First();
												while (curMob)
												{
													if (SMOB_AVATAR == curMob->WhatAmI() && curMob->cellX == curAvatar->cellX &&
														curMob->cellY == curAvatar->cellY) // if it's in your cell
													{
														avatarcount++;
													}
														curMob = (BBOSMob *)ss->avatars->Next();
												}
												if (avatarcount>1) // too many people, don't give you a resume. one means just you, so one is ok.
													HandleEarthKeyResumeUse(curAvatar, iek, ss); // so use version which does not reate a replacement.
												else // if all these pass, you get a free second resume
													HandleEarthKeyUse(curAvatar, iek, ss); // treat resume item as normal earthkey, and replace it.
											}
										}
									}
									detonatedBomb = iObject;
									iObject = (InventoryObject *)curInventory->objects.Last();
								}
								else
								{
									sprintf_s(infoText.text, "The EarthKey doesn't work in a town.");
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								}
							}
							else
							{
								sprintf_s(infoText.text, "The EarthKey doesn't work here.");
								ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
						}
						else if (setBombPtr->ptr == (long)iObject && ((INVOBJ_DOOMKEY == iObject->type)|| (INVOBJ_DOOMKEY_ENTRANCE == iObject->type))) // it's a doomkey or next floor.
						{
							InvDoomKey *iek = (InvDoomKey *)iObject->extra;
							tempReceiptList.clear();
							tempReceiptList.push_back(fromSocket);

							if (SPACE_GROUND == ss->WhatAmI())
							{
								// inside a town?
								int isInTown = FALSE;
								for (int i = 0; i < NUM_OF_TOWNS; ++i)
								{
									if (abs(townList[i].x - curAvatar->cellX) <= 3 &&
										abs(townList[i].y - curAvatar->cellY) <= 3)
									{
										isInTown = TRUE;
									}
								}

								if (!isInTown)
								{
									HandleDoomKeyUse(curAvatar, iek, ss);

									detonatedBomb = iObject;
									iObject = (InventoryObject *)curInventory->objects.Last();
								}
								else
								{
									sprintf_s(infoText.text, "Please leave town first. :)");
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								}
							}
							else //usable in places not in the normal realm for now
							{
								HandleDoomKeyUse(curAvatar, iek, ss);
								detonatedBomb = iObject;
								iObject = (InventoryObject *)curInventory->objects.Last();
							}
						}
						else if ((setBombPtr->ptr == (long)iObject) && (INVOBJ_STABLED_PET == iObject->type)) // it's a stabled pet
						{
							// fetch extra data.
							InvStabledPet *iesp = (InvStabledPet *)iObject->extra;
							tempReceiptList.clear();
							tempReceiptList.push_back(fromSocket);
							//make sure i am in town first
							bool isInTown = FALSE;
							if (ss == (SharedSpace *)spaceList->First()) // need to be in the first space to be in a town
							{
								for (int i = 0; i < NUM_OF_TOWNS; ++i)
								{
									if (abs(townList[i].x - curAvatar->cellX) <= 3 &&
										abs(townList[i].y - curAvatar->cellY) <= 3)
									{
										isInTown = TRUE;
									}
								}
							}
							if (!isInTown)
							{
								MessInfoText infoText;
								CopyStringSafely("You must be in town to let out a pet.",
									200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								break;	// abort! abort!
							}
							if (curAvatar->controlledMonster)   // if you already have a pet out.
							{
								MessInfoText infoText;
								CopyStringSafely("Stable your existing pet first with the /stablepet command.",
									200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								break;	// abort! abort!
							}

							// create new monster  
							int type = iesp->mtype;
							int subType = iesp->subType;
							float resist = iesp->magicResistance;
							if (monsterData[type][subType].name[0])
							{
								BBOSMonster *monster = new BBOSMonster(type, subType, NULL);
								sscanf(iObject->do_name, "%*s %[^\n]", tempText); // skip to after 'stabled', print rest of the name into tempText
								char tempText2[1024];
								sprintf(tempText2, "%s's %s", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
									tempText);
								CopyStringSafely(tempText2, 1024,
									monster->uniqueName, 31);
								monster->dontRespawn = TRUE;
								monster->cellX = curAvatar->cellX;
								monster->cellY = curAvatar->cellY;
								monster->targetCellX = curAvatar->cellX;
								monster->targetCellY = curAvatar->cellY;
								monster->spawnX = curAvatar->cellX;
								monster->spawnY = curAvatar->cellY;

								monster->damageDone = iesp->damageDone;
								monster->defense = iesp->defense;
								monster->health = iesp->maxHealth;  // free pet heal on login
								monster->maxHealth = iesp->maxHealth;
								monster->toHit = iesp->toHit;
								monster->a = iesp->a;
								monster->b = iesp->b;
								monster->g = iesp->g;
								monster->r = iesp->r;
								monster->sizeCoeff = iesp->sizeCoeff;
								monster->magicResistance = resist;
								monster->dontRespawn = TRUE;
								ss->mobList->Add(monster);

								monster->isMoving = TRUE;
								monster->moveStartTime = timeGetTime() - 10000;

								MessMobAppear mobAppear;
								mobAppear.mobID = (unsigned long)monster;
								mobAppear.type = monster->WhatAmI();
								mobAppear.monsterType = monster->type;
								mobAppear.subType = monster->subType;

								mobAppear.x = monster->cellX;
								mobAppear.y = monster->cellY;
								ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
									sizeof(mobAppear), &mobAppear);
								monster->AnnounceMyselfCustom(ss);
								MessGenericEffect messGE;
								messGE.avatarID = -1;
								messGE.mobID = -1;
								messGE.x = monster->cellX;
								messGE.y = monster->cellY;
								messGE.r = 255;
								messGE.g = 0;
								messGE.b = 255;
								messGE.type = 0;  // type of particles
								messGE.timeLen = 1; // in seconds
								ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
									sizeof(messGE), (void *)&messGE);
								// and control it, for now
								curAvatar->controlledMonster = monster;
								curAvatar->controlledMonster->controllingAvatar = curAvatar;
								curAvatar->followMode = TRUE;		// activate followmode

							}
							// mark object for removal
							detonatedBomb = iObject;
							iObject = (InventoryObject *)curInventory->objects.Last(); // go to end, we don't need  ot search.
						}
						iObject = (InventoryObject *)curInventory->objects.Next();
					}

					if (detonatedBomb)
					{
						if (detonatedBomb->amount > 1)
							detonatedBomb->amount -= 1;
						else
						{
							curInventory->objects.Remove(detonatedBomb);
							delete detonatedBomb;
						}
						UpdateInventory(curAvatar);
					}


				}
				break;

			case NWMESS_CHAT_CHANNEL:
				chatChannelPtr = (MessChatChannel *) messData;

				curAvatar = FindAvatar(fromSocket, &ss);
	//			curAvatar = FindIncomingAvatar(fromSocket);
				if	(curAvatar)
				{
					if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType || 
						ACCOUNT_TYPE_MODERATOR == curAvatar->accountType || 
						ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType || 
							  curAvatar->charInfoArray[curAvatar->curCharacterIndex].imageFlags &
							 SPECIAL_LOOK_HELPER)
					{
						if (chatChannelPtr->value & curAvatar->chatChannels)
							curAvatar->chatChannels &= ~(chatChannelPtr->value);
						else
							curAvatar->chatChannels |= (chatChannelPtr->value);
					}
					else
					{
						if (chatChannelPtr->value & curAvatar->chatChannels)
							curAvatar->chatChannels &= ~(chatChannelPtr->value);
						else
							curAvatar->chatChannels = (chatChannelPtr->value);
					}

					MessChatChannel returnCChan;
					returnCChan.value = curAvatar->chatChannels;

					tempReceiptList.clear();
					tempReceiptList.push_back(fromSocket);
					lserver->SendMsg(sizeof(returnCChan),(void *)&returnCChan, 0, &tempReceiptList);
				}
				break;

        }
        lserver->GetNextMsg(NULL, dataSize);
    }
}
    


//*******************************************************************************
void BBOServer::TransferAvatar(int intoWorld, int handle)
{
//	BBOSMob *curMob;
    BBOSAvatar *curAvatar;
    MessAvatarAppear messAvAppear;
    MessAvatarDisappear messAvDisappear;
    MessBladeDesc messBladeDesc;
//	char tempText[1024];

    SharedSpace *ss;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(handle);

    if (intoWorld)
    {
        curAvatar = FindIncomingAvatar(handle);
        if (curAvatar)
        {
            ss = (SharedSpace *) spaceList->Find(SPACE_GROUND);
            incoming->Remove(curAvatar);
            ss->avatars->Prepend(curAvatar);

            curAvatar->IntroduceMyself(ss, SPECIAL_APP_ENTER_GAME);

            // tell this player about everyone else around
            curAvatar->UpdateClient(ss, TRUE);
            /*
            curMob = (BBOSMob *) ss->avatars->First();
            while (curMob)
            {
                if (SMOB_AVATAR == curMob->WhatAmI())
                {
                    curAvatar = (BBOSAvatar *) curMob;
                    messAvAppear.avatarID = curAvatar->socketIndex;
                    messAvAppear.x = curAvatar->cellX;
                    messAvAppear.y = curAvatar->cellY;
                    ss->lserver->SendMsg(sizeof(messAvAppear),(void *)&messAvAppear, 0, &tempReceiptList);
                    MessAvatarStats mStats;
                    curAvatar->BuildStatsMessage(&mStats);
                    ss->lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);

                    InventoryObject *iObject = (InventoryObject *) 
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.First();
                    while (iObject)
                    {
                        if (INVOBJ_BLADE == iObject->type)
                        {
                                InvBlade *iBlade = (InvBlade *) iObject->extra;
                                messBladeDesc.bladeID = (long)iObject;
                                messBladeDesc.size    = iBlade->size;
                                messBladeDesc.r       = iBlade->r;
                                messBladeDesc.g       = iBlade->g;
                                messBladeDesc.b       = iBlade->b;
                                messBladeDesc.avatarID= curAvatar->socketIndex;
                                ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
                        }
                        iObject = (InventoryObject *) 
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next();
                    }

                }
                else
                    ; // tell the player about other things, like monsters!!

                curMob = (BBOSMob *) ss->avatars->Next();
            }
            */


        }

    }
    else
    {
        curAvatar = FindAvatar(handle, &ss);
        if (curAvatar)
        {
            curAvatar->AnnounceSpecial(ss, SPECIAL_APP_LEAVE_GAME);

            ss->avatars->Remove(curAvatar);
            incoming->Append(curAvatar);
            messAvDisappear.avatarID = handle;
            ss->lserver->SendMsg(sizeof(messAvDisappear),(void *)&messAvDisappear);
        }
    }

}


//*******************************************************************************
BBOSAvatar * BBOServer::FindAvatar(int id, SharedSpace **sp)
{
   BBOSMob *curMob;
    BBOSAvatar *curAvatar;


    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        curMob = (BBOSMob *) (*sp)->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                curAvatar = (BBOSAvatar *) curMob;
                if (curAvatar->socketIndex == id)
                {
                    return curAvatar;
                }
            }
            curMob = (BBOSMob *) (*sp)->avatars->Next();
        }

        (*sp) = (SharedSpace *) spaceList->Next();
    }
    return NULL;
}

//*******************************************************************************
BBOSAvatar * BBOServer::FindAvatar(BBOSMob *mobPtr, SharedSpace **sp)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;


    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        curMob = (BBOSMob *) (*sp)->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                curAvatar = (BBOSAvatar *) curMob;
                if (curAvatar == mobPtr)
                {
                    return curAvatar;
                }
            }
            curMob = (BBOSMob *) (*sp)->avatars->Next();
        }

        (*sp) = (SharedSpace *) spaceList->Next();
    }
    return NULL;
}

//*******************************************************************************
BBOSAvatar * BBOServer::FindAvatar(char *name, char *password, SharedSpace **sp)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;
    char tempText[1024];

    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        curMob = (BBOSMob *) (*sp)->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                curAvatar = (BBOSAvatar *) curMob;
                if (IsCompletelyVisiblySame(curAvatar->name, name))
                {
                    return curAvatar;
                }
                if (IsCompletelyVisiblySame(curAvatar->name, name))
                {
                    LongTime lt;
                    lt.value.wHour += 19;
                    if (lt.value.wHour < 24)
                    {
                        lt.value.wDay -= 1;
                    }
                    else
                        lt.value.wHour -= 24;

                    sprintf_s(tempText,"%d/%02d, %d:%02d,    ", (int)lt.value.wMonth, (int)lt.value.wDay, 
                              (int)lt.value.wHour, (int)lt.value.wMinute);
                    LogOutput("dup-possibles.txt", tempText);

                    sprintf_s(tempText,">%s< (%s)    ",curAvatar->name, curAvatar->pass);
                    LogOutput("dup-possibles.txt", tempText);
                    sprintf_s(tempText,">%s< (%s)\n",name, password);
                    LogOutput("dup-possibles.txt", tempText);
                }
            }
            curMob = (BBOSMob *) (*sp)->avatars->Next();
        }

        (*sp) = (SharedSpace *) spaceList->Next();
    }

    DoublyLinkedList *list = incoming;

    *sp = NULL; 

    curMob = (BBOSMob *) list->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())
        {
            curAvatar = (BBOSAvatar *) curMob;
            if (IsCompletelyVisiblySame(curAvatar->name, name))
            {
                return curAvatar;
            }

            if (IsCompletelyVisiblySame(curAvatar->name, name))
            {
                LongTime lt;
                lt.value.wHour += 19;
                if (lt.value.wHour < 24)
                {
                    lt.value.wDay -= 1;
                }
                else
                    lt.value.wHour -= 24;

                sprintf_s(tempText,"%d/%02d, %d:%02d,    ", (int)lt.value.wMonth, (int)lt.value.wDay, 
                          (int)lt.value.wHour, (int)lt.value.wMinute);
                LogOutput("dup-possibles.txt", tempText);

                sprintf_s(tempText,">%s< (%s)    ",curAvatar->name, curAvatar->pass);
                LogOutput("dup-possibles.txt", tempText);
                sprintf_s(tempText,">%s< (%s)\n",name, password);
                LogOutput("dup-possibles.txt", tempText);
            }
        }
        curMob = (BBOSMob *) list->Next();
    }

    return NULL;
}

//*******************************************************************************
BBOSAvatar * BBOServer::FindAvatarByAvatarName(char *avatarName, SharedSpace **sp)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;


    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        curMob = (BBOSMob *) (*sp)->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                curAvatar = (BBOSAvatar *) curMob;
                char *name = curAvatar->charInfoArray[curAvatar->curCharacterIndex].name;
                if (!(stricmp(avatarName, name)))
                {
                    return curAvatar;
                }
            }
            curMob = (BBOSMob *) (*sp)->avatars->Next();
        }

        (*sp) = (SharedSpace *) spaceList->Next();
    }
    return NULL;
}

//*******************************************************************************
BBOSAvatar * BBOServer::FindAvatarByPartialName(char *avatarName, SharedSpace **sp)
{
	BBOSMob *curMob;
	BBOSAvatar *curAvatar;


	(*sp) = (SharedSpace *)spaceList->First();
	while (*sp)
	{
		curMob = (BBOSMob *)(*sp)->avatars->First();
		while (curMob)
		{
			if (SMOB_AVATAR == curMob->WhatAmI())
			{
				curAvatar = (BBOSAvatar *)curMob;
				char *name = curAvatar->charInfoArray[curAvatar->curCharacterIndex].name;
				if (!(strnicmp(avatarName, name, strlen(avatarName))))
				{
					return curAvatar;
				}
			}
			curMob = (BBOSMob *)(*sp)->avatars->Next();
		}

		(*sp) = (SharedSpace *)spaceList->Next();
	}
	return NULL;
}

//*******************************************************************************
BBOSAvatar * BBOServer::FindAvatarNotInMySquare(BBOSAvatar *mobPtr, SharedSpace **ss, SharedSpace **sp)
{
	BBOSMob *curMob;
	BBOSAvatar *curAvatar;


	(*sp) = (SharedSpace *)spaceList->First();
	while (*sp)
	{
		curMob = (BBOSMob *)(*sp)->avatars->First();
		while (curMob)
		{
			if (SMOB_AVATAR == curMob->WhatAmI())
			{
				curAvatar = (BBOSAvatar *)curMob;
				if ((curAvatar->cellX != mobPtr->cellX) || (curAvatar->cellY != mobPtr->cellY) || ((*ss)->WhatAmI() != (*sp)->WhatAmI()) || ((((RealmMap *)(*sp))->type) != (((RealmMap *)(*ss))->type)))
				{
					return curAvatar;
				}
			}
			curMob = (BBOSMob *)(*sp)->avatars->Next();
		}

		(*sp) = (SharedSpace *)spaceList->Next();
	}
	return NULL;
}


//*******************************************************************************
BBOSAvatar * BBOServer::FindIncomingAvatar(int id)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;

    DoublyLinkedList *list = incoming;

    curMob = (BBOSMob *) list->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())
        {
            curAvatar = (BBOSAvatar *) curMob;
            if (curAvatar->socketIndex == id)
            {
                return curAvatar;
            }
        }
        curMob = (BBOSMob *) list->Next();
    }

    return NULL;
}

//*******************************************************************************
BBOSMonster * BBOServer::FindMonster(BBOSMob *mobPtr, SharedSpace **sp)
{
    BBOSMob *curMob;
    BBOSMonster *curMonster;


    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        curMob = (BBOSMob *) (*sp)->mobList->IsInList(mobPtr);
        if (curMob && SMOB_MONSTER == curMob->WhatAmI())
        {
            curMonster = (BBOSMonster *) curMob;
            return curMonster;
        }

        (*sp) = (SharedSpace *) spaceList->Next();
    }
    return NULL;
}

//*******************************************************************************
void BBOServer::BuildInventoryInfoStruct(MessInventoryInfo *info, 
                                                      Inventory *inv, int index, 
                                                      int isPlayerData)
{
    /*
   char text[10][32];
   char status[10], type[10];
   int offset;
   char isPlayerInfo;
   long money, moneyDelta;
      */

    int i;

    InventoryObject *io = (InventoryObject *) inv->objects.First();
    for (i = 0; io; ++i)
        io = (InventoryObject *) inv->objects.Next();

    int numOfItems = i;
    if (index > numOfItems - 9)
        index = numOfItems - 9;
    if (index < 0)
        index = 0;

    io = (InventoryObject *) inv->objects.First();
    for (i = 0; i < index; ++i)
        io = (InventoryObject *) inv->objects.Next();

    for (int j = 0; j < 10; ++j)
    {
        if (io)
        {
            memcpy(info->text[j],io->WhoAmI(),31);
            info->text[j][31] = 0; // blank means no object in this slot.
//			info->status[j] = io->status;
            info->type[j] = io->type;
            info->ptr[j] = (long)io;
            info->amount[j] = io->amount;
            info->value[j] = io->value;
            switch(io->type)
            {
            case INVOBJ_BLADE:
                info->f1[j] = ((InvBlade *)io->extra)->toHit;
                info->f2[j] = ((InvBlade *)io->extra)->damageDone;
                break;
            case INVOBJ_SKILL:
                info->f1[j] = ((InvSkill *)io->extra)->skillLevel;
                info->f2[j] = ((InvSkill *)io->extra)->skillPoints;
                break;
            }
        }
        else
        {
            info->text[j][0] = 0; // blank means no object in this slot.
        }
        if (io)
            io = (InventoryObject *) inv->objects.Next();
    }

    info->isPlayerInfo = isPlayerData;
    info->invPtr = (long)inv;
    info->money = inv->money;
//	info->moneyDelta = inv->moneyDelta;
    info->offset = index;
}

//*******************************************************************************
void BBOServer::UpdateInventory(BBOSAvatar *avatar)
{
    TellClientAboutInventory(avatar, avatar->lastPlayerInvSent);
}

//*******************************************************************************
void BBOServer::TellClientAboutInventory(BBOSAvatar *avatar, int type)
{
//	char smlText[128];
    MessInventoryInfo infoBase;
    MessInventoryInfo *info = &infoBase;
    Inventory *inv;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(avatar->socketIndex);

    Chronos::BStream *	stream		= NULL;
    stream	= new Chronos::BStream(sizeof(MessInventoryInfo));

    *stream << (unsigned char) NWMESS_INVENTORY_INFO; 

/*
                *stream << (unsigned char) NWMESS_PLAYER_CHAT_LINE; 
                stream->write(tEdit->text, strlen(tEdit->text));
                *stream << (unsigned char) 0; 
                lclient->SendMsg(stream->used(), stream->buffer());
*/
    int *indexPtr;

    long avatarCash = 0;
    if (avatar)
        avatarCash = avatar->charInfoArray[avatar->curCharacterIndex].inventory->money;

    SharedSpace *sx;
    BBOSAvatar *partnerAvatar = NULL;
    if (avatar->tradingPartner)
        partnerAvatar = FindAvatar(avatar->tradingPartner, &sx);

    int listSize = 9, listStartOffset = 0;

    switch(type)
    {
    case MESS_INVENTORY_PLAYER:
        listSize = 9;
        inv = (avatar->charInfoArray[avatar->curCharacterIndex].inventory);
        *stream << (unsigned char) TRUE; 
//		info->isPlayerInfo = TRUE;
        *stream << (unsigned char) MESS_INVENTORY_PLAYER; 
//		info->traderType = MESS_INVENTORY_PLAYER;
        indexPtr = &avatar->indexList[MESS_INVENTORY_PLAYER];
        avatar->lastPlayerInvSent = type;

        if (avatar->controlledMonster)
        {
//			SharedSpace *sx;

            BBOSMonster * theMonster = FindMonster(
                      avatar->controlledMonster, &sx);
            if (theMonster)
            {	
				if (avatar->accountType==ACCOUNT_TYPE_ADMIN) // admins access the monster's inventory, beastmasters do not.
                inv = (theMonster->inventory);
            }
        }
        break;

    case MESS_WORKBENCH_PLAYER:
        listSize = 11;
        inv = (avatar->charInfoArray[avatar->curCharacterIndex].workbench);
        *stream << (unsigned char) TRUE; 
//		info->isPlayerInfo = TRUE;
        *stream << (unsigned char) MESS_WORKBENCH_PLAYER; 
//		info->traderType = MESS_WORKBENCH_PLAYER;
        indexPtr = &avatar->indexList[MESS_WORKBENCH_PLAYER];
        avatar->lastPlayerInvSent = type;
        break;

    case MESS_SKILLS_PLAYER:
        listSize = 11;
        inv = (avatar->charInfoArray[avatar->curCharacterIndex].skills);
        *stream << (unsigned char) TRUE; 
//		info->isPlayerInfo = TRUE;
        *stream << (unsigned char) MESS_SKILLS_PLAYER; 
//		info->traderType = MESS_SKILLS_PLAYER;
        indexPtr = &avatar->indexList[MESS_SKILLS_PLAYER];
        avatar->lastPlayerInvSent = type;
        break;

    case MESS_WIELD_PLAYER:
        listSize = 11;
        inv = (avatar->charInfoArray[avatar->curCharacterIndex].wield);
        *stream << (unsigned char) TRUE; 
//		info->isPlayerInfo = TRUE;
        *stream << (unsigned char) MESS_WIELD_PLAYER; 
//		info->traderType = MESS_WIELD_PLAYER;
        indexPtr = &avatar->indexList[MESS_WIELD_PLAYER];
        avatar->lastPlayerInvSent = type;
        break;

    default:
    case MESS_INVENTORY_TRADER:
        inv = avatar->charInfoArray[avatar->curCharacterIndex].inventory->partner;
        *stream << (unsigned char) FALSE; 
        *stream << (unsigned char) MESS_INVENTORY_TRADER; 
        indexPtr = &avatar->indexList[MESS_INVENTORY_TRADER];
        break;

    case MESS_INVENTORY_TRAINER:
        inv = avatar->charInfoArray[avatar->curCharacterIndex].inventory->partner;
        *stream << (unsigned char) FALSE; 
        *stream << (unsigned char) MESS_INVENTORY_TRAINER; 
        indexPtr = &avatar->indexList[MESS_INVENTORY_TRADER];
        break;

    case MESS_INVENTORY_BANK:
        inv = avatar->charInfoArray[avatar->curCharacterIndex].inventory->partner;
        *stream << (unsigned char) FALSE; 
        *stream << (unsigned char) MESS_INVENTORY_BANK; 
        indexPtr = &avatar->indexList[MESS_INVENTORY_TRADER];
        avatarCash = avatar->bank->money;
        break;

    case MESS_INVENTORY_GROUND:
        inv = avatar->charInfoArray[avatar->curCharacterIndex].inventory->partner;
        *stream << (unsigned char) FALSE; 
        *stream << (unsigned char) MESS_INVENTORY_GROUND; 
        indexPtr = &avatar->indexList[MESS_INVENTORY_TRADER];
        break;

    case MESS_INVENTORY_TOWER:
        if (FindAvatarInGuild(avatar->charInfoArray[avatar->curCharacterIndex].name, &sx))
        {
            inv = (((TowerMap *) sx)->itemBox);
            *stream << (unsigned char) FALSE; 
            *stream << (unsigned char) MESS_INVENTORY_TOWER; 
            indexPtr = &avatar->indexList[MESS_INVENTORY_TRADER];
        }
        break;

    case MESS_INVENTORY_HER_SECURE:
        if (!partnerAvatar)
            return;
        inv = partnerAvatar->trade;
        *stream << (unsigned char) FALSE; 
        *stream << (unsigned char) MESS_INVENTORY_HER_SECURE; 
        indexPtr = &avatar->indexList[MESS_INVENTORY_HER_SECURE];
        avatarCash = partnerAvatar->trade->money;
        break;

    case MESS_INVENTORY_YOUR_SECURE:
        inv = avatar->trade;
        *stream << (unsigned char) FALSE; 
        *stream << (unsigned char) MESS_INVENTORY_YOUR_SECURE; 
        indexPtr = &avatar->indexList[MESS_INVENTORY_YOUR_SECURE];
        avatarCash = avatar->trade->money;
        break;

    }

    if (!inv)
        inv = (avatar->charInfoArray[avatar->curCharacterIndex].inventory);

    int i;

    InventoryObject *io = (InventoryObject *) inv->objects.First();
    for (i = 0; io; ++i)
        io = (InventoryObject *) inv->objects.Next();

    int numOfItems = i;
    if (*(indexPtr) > numOfItems - listSize+1)
        *(indexPtr) = numOfItems - listSize+1;
    if (*(indexPtr) < 0)
        *(indexPtr) = 0;

    io = (InventoryObject *) inv->objects.First();
    for (i = 0; i < *(indexPtr); ++i)
        io = (InventoryObject *) inv->objects.Next();

    for (int j = 0; j < listSize; ++j)
    {
        if (io)
        {
            *stream << (char) io->type; 

            stream->write(io->WhoAmI(), strlen(io->WhoAmI()));
            *stream << (unsigned char) 0; 
//			memcpy(info->text[j],io->WhoAmI(),31);
//			info->text[j][31] = 0; // blank means no object in this slot.
//			info->type[j] = io->type;
            *stream << (long) io; 
//			info->ptr[j] = (long)io;
            *stream << (long) io->amount; 
//			info->amount[j] = io->amount;
            *stream << (long) io->value; 
//			info->value[j] = io->value;
            switch(io->type)
            {
            case INVOBJ_BLADE:
                *stream << (long) ((InvBlade *)io->extra)->toHit; 
//				info->f1[j] = ((InvBlade *)io->extra)->toHit;
                *stream << (long) ((InvBlade *)io->extra)->damageDone; 
//				info->f2[j] = ((InvBlade *)io->extra)->damageDone;

//				sprintf_s(smlText,"%d\n",((InvBlade *)io->extra)->numOfHits);
//				LogOutput("bladeAge.txt", smlText);


                break;
            case INVOBJ_SKILL:
                *stream << (unsigned long) ((InvSkill *)io->extra)->skillLevel; 
//				info->f1[j] = ((InvSkill *)io->extra)->skillLevel;
                *stream << (unsigned long) ((InvSkill *)io->extra)->skillPoints; 
//				info->f2[j] = ((InvSkill *)io->extra)->skillPoints;
                break;
            case INVOBJ_POTION:
                *stream << ((InvPotion *)io->extra)->type; 
                *stream << ((InvPotion *)io->extra)->subType; 
                break;
            case INVOBJ_TOTEM:
                if ( ((InvTotem *)io->extra)->type >= TOTEM_PHYSICAL &&
                      ((InvTotem *)io->extra)->type <= TOTEM_CREATIVE)
                    *stream << (float) (((InvTotem *)io->extra)->imbueDeviation * -1); 
                else
                    *stream << (float) ((InvTotem *)io->extra)->imbueDeviation; 
//				info->f1[j] = ((InvTotem *)io->extra)->imbueDeviation;
                if (((InvTotem *)io->extra)->isActivated)
                {
                    LongTime ltNow;
                    *stream << (long) ((InvTotem *)io->extra)->timeToDie.MinutesDifference(&ltNow);
//					info->f2[j] = ((InvTotem *)io->extra)->timeToDie.MinutesDifference(&ltNow);
                }
                else
                    *stream << (long) 0;
//					info->f2[j] = 0;
                break;
            case INVOBJ_STAFF:
                *stream << (float) ((InvStaff *)io->extra)->imbueDeviation; 
                if (((InvStaff *)io->extra)->isActivated)
                {
                    *stream << (int) ((InvStaff *)io->extra)->charges;
                }
                else
                    *stream << (int) -1000;
                break;
            }
        }
        else
        {
            *stream << (char) -1; 
//			info->text[j][0] = 0; // blank means no object in this slot.
        }
        if (io)
            io = (InventoryObject *) inv->objects.Next();
    }

    *stream << (long) inv; 
//	info->invPtr = (long)inv;
    *stream << (long) avatarCash; 
//	info->money = avatarCash;
    *stream << (int) *(indexPtr); 
//	info->offset = *(indexPtr);

    lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);

    delete stream;

}

//*******************************************************************************
int BBOServer::TransferItem(BBOSAvatar *avatar, 
                                     MessInventoryTransferRequest *transferRequestPtr,
                                     long amount, int isGiving)
{
    char tempText[1024], dateString[128], timeString[128];
    MessInfoText infoText;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(avatar->socketIndex);

    SharedSpace *ss = NULL;
    FindAvatar(avatar->socketIndex, &ss);

    avatar->StateNoAgreement(ss);

    InventoryObject *io2;
    MessUnWield messUnWield;
	int cheat = FALSE;

    _strdate( dateString );
    _strtime( timeString );

    Inventory *inv = 
            (avatar->charInfoArray[avatar->curCharacterIndex].inventory);

    if (avatar->controlledMonster)
    {
        SharedSpace *sx;

        BBOSMonster * theMonster = FindMonster(
                  avatar->controlledMonster, &sx);
        if (theMonster)
        {
			if (avatar->accountType == ACCOUNT_TYPE_ADMIN) // admins access the monster's inventory, beastmasters do not.
            inv = (theMonster->inventory);
        }
    }

    Inventory *partner = 
            avatar->charInfoArray[avatar->curCharacterIndex].inventory->partner;

    SharedSpace *sx;
    BBOSAvatar *partnerAvatar = NULL;
    if (avatar->tradingPartner)
    {
        partnerAvatar = FindAvatar(avatar->tradingPartner, &sx);
        if (transferRequestPtr->ptr && partnerAvatar && partnerAvatar->tradingPartner == avatar)
        {

            if (transferRequestPtr->isPlayerInfo)
            {
                InventoryObject *io = (InventoryObject *) inv->objects.Find(
                                        (InventoryObject *) transferRequestPtr->ptr);
                if (io) // if found the object to transfer
                {
					if (io->type == INVOBJ_STABLED_PET)
						return 0;						// no trading pets
					// 2147483647 mean transfer all
                    if (2147483647 == amount)
                        amount = io->amount;

                    if (io->amount < amount)
                        amount = io->amount;
					if (amount < 1)
						amount = 1; //tried to trade zero? You are trading one!
                    TransferAmount(io, inv, avatar->trade, amount);

                    // tell other player
                    TellClientAboutInventory(avatar, MESS_INVENTORY_YOUR_SECURE);
                    TellClientAboutInventory(partnerAvatar, MESS_INVENTORY_HER_SECURE);

                    return 2;
                }
            }
            else
            {
                InventoryObject *io = (InventoryObject *) avatar->trade->objects.Find(
                                        (InventoryObject *) transferRequestPtr->ptr);
                if (io) // if found the object to transfer
                {
                    // 2147483647 mean transfer all
                    if (2147483647 == amount)
                        amount = io->amount;

                    if (io->amount < amount)
                        amount = io->amount;
					if (amount < 1)
						amount = 1; //tried to trade zero? You are trading one!
					if (io->type == INVOBJ_STABLED_PET)
						return 0;						// no trading pets
                    TransferAmount(io, avatar->trade, inv, amount);

                    // tell other player
                    TellClientAboutInventory(avatar, MESS_INVENTORY_YOUR_SECURE);
                    TellClientAboutInventory(partnerAvatar, MESS_INVENTORY_HER_SECURE);

                    return 2;
                }
            }
        }
        return 0;
    }

    if (partner)
    {
        // from trainer to your skill inventory ************
        if (MESS_INVENTORY_TRAINER == partner->subType && 
             partner == (Inventory *) transferRequestPtr->partner &&
             transferRequestPtr->ptr)
        {
            // no longer item inv, now it's skills inv
            inv = (avatar->charInfoArray[avatar->curCharacterIndex].skills);

            // can't buy more than one instance of a skill
            amount = 1;

            InventoryObject *io = (InventoryObject *) partner->objects.Find(
                                    (InventoryObject *) transferRequestPtr->ptr);
            if (io) // if found the object to transfer
            {
                // if there's already an object by that name
                io2 = (InventoryObject *) inv->objects.Find(io->WhoAmI());
                if (io2)
                {
                    sprintf_s(tempText,"You already know that skill.");
                    memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                    infoText.text[MESSINFOTEXTLEN-1] = 0;
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    return 0;
                }
                else if (io->value * amount <= 
                          avatar->charInfoArray[avatar->curCharacterIndex].inventory->money)
                {
                    if( avatar->charInfoArray[avatar->curCharacterIndex].magical < 10 && !strcmp( io->do_name,"Totem Shatter" ) )
                    {
                        sprintf_s(tempText,"You must have a magic of at least 10 to use this skill.");
                        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                        infoText.text[MESSINFOTEXTLEN-1] = 0;

                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
					else if (avatar->charInfoArray[avatar->curCharacterIndex].beast < 10 && !strcmp(io->do_name, "Taming"))
					{
						sprintf_s(tempText, "You must be a beastmaster to use this skill.");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;

						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else if (avatar->charInfoArray[avatar->curCharacterIndex].beast < 10 && !strcmp(io->do_name, "Pet Mastery"))
					{
						sprintf_s(tempText, "You must be a beastmaster to use this skill.");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;

						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else if (avatar->charInfoArray[avatar->curCharacterIndex].beast < 10 && !strcmp(io->do_name, "Pet Energy"))
					{
						sprintf_s(tempText, "You must be a beastmaster to use this skill.");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;

						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else if (avatar->charInfoArray[avatar->curCharacterIndex].creative < 10 && !strcmp(io->do_name, "Disarming"))
					{
						sprintf_s(tempText, "You must be a smith to use this skill.");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;

						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else if (((avatar->charInfoArray[avatar->curCharacterIndex].physical < 10)&&(avatar->charInfoArray[avatar->curCharacterIndex].beast > 9 ))&& !strcmp(io->do_name, "Dodging"))
					{
						sprintf_s(tempText, "Beastmasters can't learn Dodging.");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;

						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
                    {
                        avatar->charInfoArray[avatar->curCharacterIndex].inventory->money -= io->value * amount;
                        sprintf_s(tempText,"You spent %dg to learn %4s.",
                                (int)io->value, io->WhoAmI());
                        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                        infoText.text[MESSINFOTEXTLEN-1] = 0;

						// add the CLEVEL part for the skill that was just bought at level 1.
						if (!strcmp("Dodging", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_DODGE;
						if (!strcmp("Pet Energy", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_PET_ENERGY;
						if (!strcmp("Explosives", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_BOMB;
						if (!strcmp("Swordsmith", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH;
						if (!strcmp("Weapon Dismantle", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH;

						if (!strcmp("Katana Expertise", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH_EXPERT;
						if (!strcmp("Chaos Expertise", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH_EXPERT;
						if (!strcmp("Mace Expertise", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH_EXPERT;
						if (!strcmp("Bladestaff Expertise", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH_EXPERT;
						if (!strcmp("Claw Expertise", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SMITH_EXPERT;

						if (!strcmp("Bear Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Wolf Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Eagle Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Snake Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Frog Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Sun Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Moon Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Turtle Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Evil Magic", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;
						if (!strcmp("Geomancy", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_GEOMANCY;
						if (!strcmp("Totem Shatter", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_SHATTER;
						if (!strcmp("Taming", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_TAMING;
						if (!strcmp("Pet Mastery", io->WhoAmI()))
							avatar->charInfoArray[avatar->curCharacterIndex].cLevel += CLEVEL_VAL_PET_MASTERY;


						TransferAmount(io, partner, inv, 1);

                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }

                    return 0;
                }
                else
                {
                    sprintf_s(tempText,"You don't have %dg for %4s.",
                             (int)io->value, io->WhoAmI());
                    memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                    infoText.text[MESSINFOTEXTLEN-1] = 0;
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    return 0;
                }
            }
        }
        // from your item inventory to non-trainer ************
        else if (MESS_INVENTORY_TRAINER != partner->subType && 
                  transferRequestPtr->isPlayerInfo &&
                  transferRequestPtr->ptr)
        {
            InventoryObject *io = (InventoryObject *) inv->objects.Find(
                                    (InventoryObject *) transferRequestPtr->ptr);
            if (io) // if found the object to transfer
            {
                // 2147483647 mean transfer all
                if (2147483647 == amount)
                    amount = io->amount;

                if (io->amount < amount)
                    amount = io->amount;
				if (amount < 1)
					amount = 1; //tried to trade zero? You are trading one!

                int all = FALSE;
                if (amount == io->amount)
                    all = TRUE;

                int isInFingle = FALSE;
                if (abs(townList[2].x - avatar->cellX) <= 5 && 
                     abs(townList[2].y - avatar->cellY) <= 5)
                {
                    isInFingle = TRUE;
                }
                
                if( partner->subType == MESS_INVENTORY_TRADER ) {
                    sprintf_s(tempText,"DROP-SELL, %d, %s, %s, %s, %s, %s, Merchant, %d, %d\n", 
                                amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  avatar->cellX, avatar->cellY);
                        LogOutput("tradelog.txt", tempText);
                }
                else {
                    sprintf_s(tempText,"DROP-SELL, %d, %s, %s, %s, %s, %s, %s, %d, %d\n", 
                                amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  ss->WhoAmI(), avatar->cellX, avatar->cellY);
                        LogOutput("groundtradelog.txt", tempText);
                }

                if (INVOBJ_BLADE == io->type)
                {
                    sprintf_s(tempText,"DROP-SELL, %d, %s, %s, %s, %s, %s, %s, %d, %d\n", 
                                amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  ss->WhoAmI(), avatar->cellX, avatar->cellY);
                        LogOutput("swordTransferLog.txt", tempText);
                }

                if (INVOBJ_BLADE == io->type && ((InvBlade *)io->extra)->isWielded)
                {
                    messUnWield.bladeID = (long)avatar->socketIndex;
                    ss->SendToEveryoneNearBut(0, avatar->cellX, avatar->cellY,
                        sizeof(messUnWield),(void *)&messUnWield);
                    ((InvBlade *)io->extra)->isWielded = FALSE;
                }

                if (isGiving)
                {
                    if (MESS_INVENTORY_TOWER == partner->subType)
                    {
                        sprintf_s(tempText,"INSERT, %d, %s, %s, %s, %s, %s, %s\n", amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  ss->WhoAmI());
                        LogOutput("towerChestLog.txt", tempText);
                    }
					if (INVOBJ_STABLED_PET == io->type)
					{
						sprintf_s(tempText, "This is a town, not a zoo.");
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						return 0;
					}
					TransferAmount(io, inv, partner, amount);

                    return 1;
                }
				else if (isInFingle && INVOBJ_EARTHKEY == io->type)
				{
					sprintf_s(tempText, "We don't buy those here.  Please try another town.");
					memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
					infoText.text[MESSINFOTEXTLEN - 1] = 0;
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					return 0;
				}
				else if (INVOBJ_EARTHKEY_RESUME == io->type)
				{
					sprintf_s(tempText, "We don't buy those.");
					memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
					infoText.text[MESSINFOTEXTLEN - 1] = 0;
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					return 0;
				}
				else if (INVOBJ_STABLED_PET == io->type)
				{
					sprintf_s(tempText, "This is a town, not a zoo.");
					memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
					infoText.text[MESSINFOTEXTLEN - 1] = 0;
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					return 0;
				}
				else
                {
                    float value = io->value * amount * 7 / 10;
                    if (value < 1)
                        value = 1;
                    avatar->charInfoArray[avatar->curCharacterIndex].inventory->money += value;

                    sprintf_s(tempText,"You got %dg for %ld %4s.",
                             (int)value, amount, io->WhoAmI());
                    memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);

                    infoText.text[MESSINFOTEXTLEN-1] = 0;

                    if (INVOBJ_MEAT == io->type)
                    {
                        SaltThisMeat(io);
                    }

                    TransferAmount(io, inv, partner, amount);

                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    return 1;
                }
            }

        }
        // from non-trainer to your item inventory ************
        else if (MESS_INVENTORY_TRAINER != partner->subType && 
                  partner == (Inventory *) transferRequestPtr->partner &&
                    transferRequestPtr->ptr)
        {
//			if (partner->
            InventoryObject *io = (InventoryObject *) partner->objects.Find(
                                    (InventoryObject *) transferRequestPtr->ptr);
            if (io) // if found the object to transfer
            {
				if ((infinite_supply) // if merchant cheat is on
					&& (partner->subType == MESS_INVENTORY_TRADER) // and our partner is a merchant
					&& ((io->type == INVOBJ_EXPLOSIVE) // and it's an brick
					|| (io->type == INVOBJ_FUSE) // or a fuse
					||	(io->type == INVOBJ_SIMPLE) // or simple loot
					||	((io->type == INVOBJ_GEOPART) && (((InvGeoPart *)io->extra)->type == 0)) // or a bead (not a heartgem)
					||	((io->type == INVOBJ_INGOT) && (((InvIngot *)io->extra)->damageVal < 8)) // or an ingot LESS than vizorium
					||	((io->type == INVOBJ_POTION) && (((InvPotion *)io->extra)->type == 0)) // or a town recall (but not another type)
					||	((io->type == INVOBJ_STAFF) && (((InvStaff *)io->extra)->quality < 3)) // or a staff less than Oak.
						|| ((io->type == INVOBJ_TOTEM) && (((InvTotem *)io->extra)->quality < 17)) // or a totem less than pumpkin
						)
					)
				{
					cheat = TRUE;  //  then we can buy more than there is there
				}
				else // normal limit
					// 2147483647 mean transfer all
				{
					if (2147483647 == amount)
						amount = io->amount;

					if (io->amount < amount)
						amount = io->amount;
				}

				if (amount < 1)
					amount = 1; //tried to trade zero? You are trading one!

                if( partner->subType == MESS_INVENTORY_TRADER ) {
                    sprintf_s(tempText,"BUY-GET, %d, %s, %s, %s, %s, %s, Merchant, %d, %d\n", 
                                amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  avatar->cellX, avatar->cellY);
                        LogOutput("tradelog.txt", tempText);
                }
                else {
                    sprintf_s(tempText,"BUY-GET, %d, %s, %s, %s, %s, %s, %s, %d, %d\n", 
                                amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  ss->WhoAmI(), avatar->cellX, avatar->cellY);
                        LogOutput("groundtradelog.txt", tempText);
                }

                if (INVOBJ_BLADE == io->type)
                {
                    sprintf_s(tempText,"BUY-GET, %d, %s, %s, %s, %s, %s, %s, %d, %d\n", 
                                amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  ss->WhoAmI(), avatar->cellX, avatar->cellY);
                        LogOutput("swordTransferLog.txt", tempText);
                }


                if (isGiving)
                {
                    if (MESS_INVENTORY_BANK != partner->subType)
                    {
                        if (amount < 2)
                            sprintf_s(tempText,"%s got a %s.",
                                avatar->charInfoArray[avatar->curCharacterIndex].name, io->WhoAmI());
                        else
                            sprintf_s(tempText,"%s got %d %ss.",
                                avatar->charInfoArray[avatar->curCharacterIndex].name, amount, io->WhoAmI());
                        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                        infoText.text[MESSINFOTEXTLEN-1] = 0;
                        ss->SendToEveryoneNearBut(0, avatar->cellX, avatar->cellY,
                            sizeof(infoText),(void *)&infoText,1);
                    }

                    avatar->QuestPickupItem(ss, io);

                    if (MESS_INVENTORY_TOWER == partner->subType)
                    {
                        sprintf_s(tempText,"REMOVE, %d, %s, %s, %s, %s, %s, %s\n", amount, io->WhoAmI(),
                                 avatar->charInfoArray[avatar->curCharacterIndex].name,
                                 avatar->name, 
                                  dateString, timeString,
                                  ss->WhoAmI());
                        LogOutput("towerChestLog.txt", tempText);
                    }

                    TransferAmount(io, partner, inv, amount);

                    // save in 15 seconds
                    avatar->lastSaveTime = timeGetTime() - 1000 * 60 * 5 + 1000 * 15;
                    return 1;
                }
                else if (io->value * amount <= 
                          avatar->charInfoArray[avatar->curCharacterIndex].inventory->money)
                {
                    avatar->charInfoArray[avatar->curCharacterIndex].inventory->money -= io->value * amount;
                    sprintf_s(tempText,"You spent %dg for the %ld %4s.",
                             (int)io->value * amount, amount, io->WhoAmI());
                    memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                    infoText.text[MESSINFOTEXTLEN-1] = 0;
					if (cheat)
					{
						TransferAmountCheat(io, partner, inv, amount);
					}
					else
					{
						TransferAmount(io, partner, inv, amount);
					}
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    return 1;
                }
                else
                {
                    sprintf_s(tempText,"You don't have %dg for the %ld %4s.",
                             (int)io->value * amount, amount, io->WhoAmI());
                    memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                    infoText.text[MESSINFOTEXTLEN-1] = 0;
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    return 0;
                }
            }

        }
    }
    return 0;

}


//*******************************************************************************
int BBOServer::ShiftItem(BBOSAvatar *avatar, MessInventoryChange *inventoryChangePtr)
{
    if (!inventoryChangePtr->ptr)
        return FALSE;
    char tempText[1024];
    MessInfoText infoText;
    MessUnWield messUnWield;

    int amount = inventoryChangePtr->amount;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(avatar->socketIndex);

    SharedSpace *ss = NULL;
    FindAvatar(avatar->socketIndex, &ss);
	if ((avatar)&&(avatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0))
	{
		MessInfoText infoText;
		sprintf_s(tempText, "You are stunned, and cannot transfer items!");
		memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
		infoText.text[MESSINFOTEXTLEN - 1] = 0;
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		return FALSE;
	}

    Inventory *src, *dst;
    switch(inventoryChangePtr->srcListType)
    {
    case GTM_BUTTON_LIST_INV:
    default:
        src = (avatar->charInfoArray[avatar->curCharacterIndex].inventory);
        break;
    case GTM_BUTTON_LIST_WRK:
        src = (avatar->charInfoArray[avatar->curCharacterIndex].workbench);
        break;
    case GTM_BUTTON_LIST_WLD:
        src = (avatar->charInfoArray[avatar->curCharacterIndex].wield);
        break;
    }

    switch(inventoryChangePtr->dstListType)
    {
    case GTM_BUTTON_LIST_INV:
        dst = (avatar->charInfoArray[avatar->curCharacterIndex].inventory);
        break;
    case GTM_BUTTON_LIST_WRK:
        dst = (avatar->charInfoArray[avatar->curCharacterIndex].workbench);
        break;
    case GTM_BUTTON_LIST_WLD:
    default:
        dst = (avatar->charInfoArray[avatar->curCharacterIndex].wield);
        break;
    }

    InventoryObject *io = (InventoryObject *) src->objects.Find(
                                    (InventoryObject *) inventoryChangePtr->ptr);
	if (io) // if found the object to transfer
	{
		// 2147483647 mean transfer all
		if (2147483647 == amount)
			amount = io->amount;

		if (io->amount < amount)
			amount = io->amount;
		if (amount < 1)
			amount = 1;

		if (GTM_BUTTON_LIST_WLD == inventoryChangePtr->dstListType)
		{
			sprintf_s(tempText, "You wielded %d %4s.", amount, io->WhoAmI());
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		}
		else if (GTM_BUTTON_LIST_WLD == inventoryChangePtr->srcListType)
		{
			sprintf_s(tempText, "You removed %d %4s.", amount, io->WhoAmI());
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
			/*
						if (INVOBJ_BLADE == io->type)
						{
							messUnWield.bladeID = (long)avatar->socketIndex;
							ss->SendToEveryoneNearBut(0, avatar->cellX, avatar->cellY,
								sizeof(messUnWield),(void *)&messUnWield);
							((InvBlade *)io->extra)->isWielded = FALSE;
						}
			*/
		}
		else
		{
			sprintf_s(tempText, "You transferred %d %4s.", amount, io->WhoAmI());
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		}
		// check for beastmaster trying to wield prohibited stuff.
		if ((GTM_BUTTON_LIST_WLD == inventoryChangePtr->dstListType) && // if it was a wield action
			(io->type == INVOBJ_TOTEM) &&								// if it was a totem
			(avatar->charInfoArray[avatar->curCharacterIndex].physical < 10) && (avatar->charInfoArray[avatar->curCharacterIndex].beast > 9))									// if character is a beastmaster
		{
			sprintf_s(tempText, "Beastmasters can't wield totems.", amount, io->WhoAmI());
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		}
		else if ((GTM_BUTTON_LIST_WLD == inventoryChangePtr->dstListType) && // if it was a wield action
			(io->type == INVOBJ_STAFF) &&								// if it was a totem
			(avatar->charInfoArray[avatar->curCharacterIndex].physical < 10) && (avatar->charInfoArray[avatar->curCharacterIndex].beast > 9))									// if character is a beastmaster
		{
			sprintf_s(tempText, "Beastmasters can't wield staffs.", amount, io->WhoAmI());
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		}
		else if ((GTM_BUTTON_LIST_WLD == inventoryChangePtr->dstListType) && // if it was a wield action
			(io->type == INVOBJ_BLADE) &&// if it was a blade
			(((InvBlade *)io->extra)->type!=BLADE_TYPE_TAME_SCYTHE)&&
			(avatar->charInfoArray[avatar->curCharacterIndex].physical < 10) && (avatar->charInfoArray[avatar->curCharacterIndex].beast > 9))									// if character is a beastmaster
		{
			sprintf_s(tempText, "Beastmasters can only wield Taming scythes.", amount, io->WhoAmI());
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		}
		else
		{
			TransferAmount(io, src, dst, amount);
		}
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		if (GTM_BUTTON_LIST_WLD == inventoryChangePtr->dstListType ||
             GTM_BUTTON_LIST_WLD == inventoryChangePtr->srcListType)
        {
            InventoryObject *iObject = (InventoryObject *) 
                avatar->charInfoArray[avatar->curCharacterIndex].wield->objects.First();
            int hasBlade = FALSE;
            while (iObject)
            {
                if (INVOBJ_BLADE == iObject->type)
                {
                    MessBladeDesc messBladeDesc;
                    FillBladeDescMessage(&messBladeDesc, iObject, avatar);
                    ss->SendToEveryoneNearBut(0, avatar->cellX, avatar->cellY,
                        sizeof(messBladeDesc),(void *)&messBladeDesc);
                    iObject = (InventoryObject *) 
                        avatar->charInfoArray[avatar->curCharacterIndex].wield->objects.Last();
                    hasBlade = TRUE;
                    
                }
                if (INVOBJ_STAFF == iObject->type)
                {
                    InvStaff *iStaff = (InvStaff *) iObject->extra;
                    
                    MessBladeDesc messBladeDesc;
                    messBladeDesc.bladeID = (long)iObject;
                    messBladeDesc.size    = 4;
                    messBladeDesc.r       = staffColor[iStaff->type][0];
                    messBladeDesc.g       = staffColor[iStaff->type][1];
                    messBladeDesc.b       = staffColor[iStaff->type][2];
                    messBladeDesc.avatarID= avatar->socketIndex;
                    messBladeDesc.trailType  = 0;
                    messBladeDesc.meshType = BLADE_TYPE_STAFF1;
                    ss->SendToEveryoneNearBut(0, avatar->cellX, avatar->cellY,
                        sizeof(messBladeDesc),(void *)&messBladeDesc);
                    iObject = (InventoryObject *) 
                        avatar->charInfoArray[avatar->curCharacterIndex].wield->objects.Last();
                    hasBlade = TRUE;
                }

                iObject = (InventoryObject *) 
                    avatar->charInfoArray[avatar->curCharacterIndex].wield->objects.Next();
            }

            if (!hasBlade)
            {
                messUnWield.bladeID = (long)avatar->socketIndex;
                ss->SendToEveryoneNearBut(0, avatar->cellX, avatar->cellY,
                    sizeof(messUnWield),(void *)&messUnWield);
            }
        }


        return TRUE;
    }
    return FALSE;

}


//*******************************************************************************
void BBOServer::TransferAmount(InventoryObject *io, Inventory *inv,
                                         Inventory *partner,  long amount)
{
//	char tempText[1024];
    if (io->amount < amount)
        amount = io->amount; // shrink stack siz to what's actually there
	if (amount < 1)
		return; // only a packet hack can cause this, abort  it will be deleted on relog

    InventoryObject *io2 = new InventoryObject( INVOBJ_SIMPLE, 0, "LOADED" );

    io->CopyTo( io2 );

    io->amount -= amount;
    io2->amount = amount;

    partner->AddItemSorted(io2);    
	if (io->amount < 1)
	{
		// delete the source item
		inv->objects.Remove(io);
		delete io;
	}

}

void BBOServer::TransferAmountCheat(InventoryObject *io, Inventory *inv,
	Inventory *partner, long amount)
{
	//    char tempText[1024];


	InventoryObject *io2 = new InventoryObject(INVOBJ_SIMPLE, 0, "LOADED");

	io->CopyTo(io2);

	io2->amount = amount;

	partner->AddItemSorted(io2);

}

//*******************************************************************************
void BBOServer::SetWield(int isWielding, InventoryObject *iObject, Inventory *inventory)
{
    InvBlade *ib = (InvBlade *) iObject->extra;  // better be a blade!!!

    InventoryObject *io;

    if (!isWielding)
    {
        ib->isWielded = FALSE;
    }
    else
    {
        io = (InventoryObject *) inventory->objects.First();
        while (io)
        {
            if (INVOBJ_BLADE == io->type)
            {
                ib = (InvBlade *) io->extra;
                ib->isWielded = FALSE;
            }

            io = (InventoryObject *) inventory->objects.Next();
        }

        ib = (InvBlade *) iObject->extra;  // better be a blade!!!
        ib->isWielded = TRUE;
    }

}


//*******************************************************************************
void BBOServer::HandleCombine(BBOSAvatar *avatar, char *skillName)
{
    char tempText[1024];
    MessInfoText infoText;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(avatar->socketIndex);
/*
    sprintf_s(tempText,"Combining using %s.", skillName);
    memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
    infoText.text[MESSINFOTEXTLEN-1] = 0;
    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
*/

	if (avatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0)
	{
		MessInfoText infoText;
		sprintf_s(tempText, "You are stunned, and cannot use skills!");
		memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
		infoText.text[MESSINFOTEXTLEN - 1] = 0;
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		return;
	}
    if (!strcmp(skillName,"Swordsmith"))
    {
        sprintf_s(avatar->combineSkillName,"Swordsmith");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else if (!strcmp(skillName,"Katana Expertise"))
    {
        sprintf_s(avatar->combineSkillName,"Katana Expertise");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else if (!strcmp(skillName,"Mace Expertise"))
    {
        sprintf_s(avatar->combineSkillName,"Mace Expertise");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else if (!strcmp(skillName,"Bladestaff Expertise"))
    {
        sprintf_s(avatar->combineSkillName,"Bladestaff Expertise");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else if (!strcmp(skillName,"Claw Expertise"))
    {
        sprintf_s(avatar->combineSkillName,"Claw Expertise");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
	else if (!strcmp(skillName, "Chaos Expertise"))
	{
		sprintf_s(avatar->combineSkillName, "Chaos Expertise");
		avatar->combineStartTime = timeGetTime();
		avatar->isCombining = TRUE;

		sprintf_s(tempText, "You begin using %s.", skillName);
		memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
		infoText.text[MESSINFOTEXTLEN - 1] = 0;
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
	}
	else if (!strcmp(skillName, "Scythe Expertise"))
	{
		sprintf_s(avatar->combineSkillName, "Scythe Expertise");
		avatar->combineStartTime = timeGetTime();
		avatar->isCombining = TRUE;

		sprintf_s(tempText, "You begin using %s.", skillName);
		memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
		infoText.text[MESSINFOTEXTLEN - 1] = 0;
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
	}
	else if (!strcmp(skillName,"Weapon Dismantle"))
    {
        sprintf_s(avatar->combineSkillName,"Weapon Dismantle");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else if (!strcmp(skillName,"Geomancy"))
    {
        sprintf_s(avatar->combineSkillName,"Geomancy");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else if (!strcmp(skillName,"Totem Shatter"))
    {
        sprintf_s(avatar->combineSkillName,"Totem Shatter");
        avatar->combineStartTime = timeGetTime();
        avatar->isCombining = TRUE;

        sprintf_s(tempText,"You begin using %s.", skillName);
        memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
        infoText.text[MESSINFOTEXTLEN-1] = 0;
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
	else if (!strcmp(skillName, "Explosives"))
	{
	sprintf_s(avatar->combineSkillName, "Explosives");
	avatar->combineStartTime = timeGetTime();
	avatar->isCombining = TRUE;

	CopyStringSafely("You begin making explosives.", 1000, infoText.text, MESSINFOTEXTLEN);
	lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
	}
	else if (!strcmp(skillName, "Disarming"))
	{
	sprintf_s(avatar->combineSkillName, "Disarming");
	avatar->combineStartTime = timeGetTime();
	avatar->isCombining = TRUE;

	CopyStringSafely("You begin trying to disarm a bomb.", 1000, infoText.text, MESSINFOTEXTLEN);
	lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
	}
	else
    {
        int usedMagic = -1;
        for (int i = 0; i < MAGIC_MAX; ++i)
        {
            if (!strnicmp(magicNameList[i],skillName, strlen(magicNameList[i])))
                usedMagic = i;
        }

        if (usedMagic > -1)
        {
            sprintf_s(avatar->combineSkillName,skillName);
            avatar->combineStartTime = timeGetTime();
            avatar->isCombining = TRUE;

            sprintf_s(tempText,"You begin using %s.", skillName);
            memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
            infoText.text[MESSINFOTEXTLEN-1] = 0;
            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
			// anti-bot check
			if ((avatar->SkillCellX == avatar->cellX) && (avatar->SkillCellY == avatar->cellY))
			{
				avatar->SameCellCount++; // increase cell count
				if ((avatar->SameCellCount > 900) && (!avatar->EvilBotter)) // about an hour's worh of combining
				{
					avatar->EvilBotter = true; // set evil botter flag so thievign spiirit will steal everything
					/// and send the player off to the realm of the dead! muah hah hah!
					avatar->BotTPCount++; // increment the teleport count.
					// find the shared space
					SharedSpace *ss;
					FindAvatarByAvatarName(avatar->name, &ss);
					boolean mapfound = false;
					int realmID = REALM_ID_DEAD; //  we are going there
					// check for monster
					// gotta check for controlling monster
					// am i controlling a monster?
					if (avatar->controlledMonster && !avatar->followMode)
					{
						// and make my monster vanish too
						MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
						tempReceiptList.clear();
						tempReceiptList.push_back(avatar->socketIndex); // point server at the target client.
						adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						// and resturn server to follow mode, since both will end up in the same spot.
						if (avatar->BeastStat() > 9)
							avatar->followMode = TRUE;
						// we need to dissapear it from people's screen.
					}
					RealmMap *rp = NULL;
					SharedSpace *sp = (SharedSpace *)spaceList->First();
					
					while (!mapfound && sp)
					{
						if (SPACE_REALM == sp->WhatAmI() && realmID == ((RealmMap *)sp)->type)
						{
							rp = (RealmMap *)sp;
							mapfound = true;
						}
						sp = (SharedSpace *)spaceList->Next();
					}
					if (rp)
					{
						avatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);
						// vanish the controlled monster if present
						if (avatar->controlledMonster)
						{
							// we need to dissapear it from people's screen.
							MessMonsterDeath messMD;
							messMD.mobID = (unsigned long)avatar->controlledMonster;
							ss->SendToEveryoneNearBut(0, avatar->controlledMonster->cellX, avatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(avatar->controlledMonster);

							}
							// tell my client I'm entering the realm
							MessChangeMap changeMap;
							changeMap.realmID = rp->type;
							changeMap.oldType = ss->WhatAmI();
							changeMap.newType = rp->WhatAmI();
							changeMap.sizeX = rp->sizeX;
							changeMap.sizeY = rp->sizeY;
							changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
							lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

							MessInfoText infoText;
							sprintf_s(tempText, "You enter the %s.  You have a bad feeling about this.", rp->WhoAmI());
							CopyStringSafely(tempText,
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

							avatar->QuestSpaceChange(ss, rp);

							// move me to my new SharedSpace
							ss->avatars->Remove(avatar);
							rp->avatars->Append(avatar);

							avatar->cellX = avatar->targetCellX = (rand() % 4) + 2;
							avatar->cellY = avatar->targetCellY = (rand() % 4) + 2;
							if (avatar->controlledMonster) // if they had a monster
							{
								// then it needs ot have it's coords updated, and be added to new map
								avatar->controlledMonster->cellX = avatar->controlledMonster->targetCellX = avatar->cellX;
								avatar->controlledMonster->cellY = avatar->controlledMonster->targetCellY = avatar->cellY;
								rp->mobList->Add(avatar->controlledMonster);
							}

							// tell everyone about my arrival
							avatar->IntroduceMyself(rp, SPECIAL_APP_ADMIN_TELEPORT);
							if (avatar->controlledMonster)
							{
								// tell clients about my monster
								avatar->controlledMonster->AnnounceMyselfCustom(rp);
							}
							// tell this player about everyone else around
							avatar->UpdateClient(rp, TRUE);
					}
				}
			}
			else
			{
				// update location, and reset counter, but not the bot flag. that gets fixed if you actually move after you get ported.
				avatar->SkillCellX = avatar->cellX;
				avatar->SkillCellY = avatar->cellY;
				avatar->SameCellCount = 0;

			}
        }
        else
        {
            sprintf_s(tempText,"You can't combine with %s.", skillName);
            memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
            infoText.text[MESSINFOTEXTLEN-1] = 0;
            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
    }
}


//*******************************************************************************
void BBOServer::TryToMoveAvatar(int fromSocket, MessAvatarMoveRequest *AvMoveReqPtr)
{
    SharedSpace *ss;
    BBOSAvatar *curAvatar;
    int tempX, tempY;
    MessBladeDesc messBladeDesc;
    MessAvatarMove AvMove;
    MessInfoText infoText;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(fromSocket);
    char tempText[128];

    switch(AvMoveReqPtr->x)
    {
    case 0:
        tempX = 0;
        tempY = -1;
        break;
    case 1:
        tempX = 1;
        tempY = -1;
        break;
    case 2:
        tempX = 1;
        tempY = 0;
        break;
    case 3:
        tempX = 1;
        tempY = 1;
        break;
    case 4:
        tempX = 0;
        tempY = 1;
        break;
    case 5:
        tempX = -1;
        tempY = 1;
        break;
    case 6:
        tempX = -1;
        tempY = 0;
        break;
    case 7:
        tempX = -1;
        tempY = -1;
        break;
    }
    
    curAvatar = FindAvatar(fromSocket, &ss);
    if (curAvatar)
    {

		if (curAvatar->magicEffectAmount[MONSTER_EFFECT_BIND] > 0)
		{
			MessInfoText infoText;
			sprintf_s(tempText, "You are bound, and cannot move!");
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			return;
		}
		if (curAvatar->magicEffectAmount[MONSTER_EFFECT_STUN] > 0)
		{
			MessInfoText infoText;
			sprintf_s(tempText, "You are stunned, and cannot move!");
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			return;
		}

        if (curAvatar->controlledMonster) // first check for controlled monster.
        {
            SharedSpace *sx;

            BBOSMonster * theMonster = FindMonster(
                     curAvatar->controlledMonster, &sx);
            if (theMonster && (!curAvatar->followMode)) // normal move character instead.
            {
				if (!theMonster->isMoving)
				{
					curAvatar->MoveControlledMonster(sx, tempX, tempY);
				}
				return;  // then don't also move the character
			}
        }

        // if not going out of bounds...
        if (tempX + curAvatar->cellX < MAP_SIZE_WIDTH &&
            tempX + curAvatar->cellX >= 0 &&
            tempY + curAvatar->cellY < MAP_SIZE_HEIGHT &&
            tempY + curAvatar->cellY >= 0 &&
             !curAvatar->isMoving)
        {

            // REALM OF PAIN TEMP CODE
            if( tempX + curAvatar->cellX == 161 && tempY + curAvatar->cellY == 121 ) {
                MessInfoText infoText;
                sprintf_s( tempText,"The doorway to the Realm of Pain is sealed shut. You cannot open it now." );
                memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                infoText.text[MESSINFOTEXTLEN-1] = 0;
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }

            if (SPACE_GROUND == ss->WhatAmI())
            {
                // stepping into a dungeon?
                SharedSpace *sp = (SharedSpace *) spaceList->First();
                while (sp)
                {
                    if (SPACE_DUNGEON == sp->WhatAmI() && 
						!(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && !(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
                    {
                        if (((DungeonMap *) sp)->enterX == tempX + curAvatar->cellX &&
                             ((DungeonMap *) sp)->enterY == tempY + curAvatar->cellY &&
							((GroundMap *)ss)->type==0              // only in normal normal realm
							)
                        {
                            int lck = FALSE;
                            if (((DungeonMap *)sp)->isLocked &&
                                 !(ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                                    ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                                    ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType))
                                return;
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = NULL;
							curAvatar->activeCounter = 0;
							BBOSMonster *savedmonster = NULL;
							if (curAvatar->controlledMonster) // if i got here and have a controlled monster
							{
								// i need to stash my monster
								savedmonster = curAvatar->controlledMonster;
								MessAdminMessage adminMess; // create an admin message to tell client that it's no lnoger controlling a monster so it doesn't get confused by the map change.
								adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
								lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
								MessMonsterDeath messMD;
								messMD.mobID = (unsigned long)curAvatar->controlledMonster;
								ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(curAvatar->controlledMonster);
								curAvatar->controlledMonster = NULL; // unconfuse the client.
							}
                            // tell everyone I'm dissappearing
                            curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON); 

                            // tell my client I'm entering the dungeon
                            MessChangeMap changeMap;
                            changeMap.dungeonID = (long) sp;
                            changeMap.oldType = ss->WhatAmI();
                            changeMap.newType = sp->WhatAmI();
                            changeMap.sizeX   = ((DungeonMap *) sp)->width;
                            changeMap.sizeY   = ((DungeonMap *) sp)->height;
                            changeMap.flags   = MESS_CHANGE_NOTHING;

                            if	(((DungeonMap *) sp)->CanEdit(curAvatar))
                                changeMap.flags   = MESS_CHANGE_EDITING;

                            lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                            MessInfoText infoText;
                            sprintf_s(tempText,"You enter the %s.", ((DungeonMap *) sp)->name);
                            memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                            infoText.text[MESSINFOTEXTLEN-1] = 0;
                            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            curAvatar->QuestSpaceChange(ss,sp);

                            // move me to my new SharedSpace
                            ss->avatars->Remove(curAvatar);
                            sp->avatars->Append(curAvatar);

                            curAvatar->cellX = ((DungeonMap *) sp)->width-1;
                            curAvatar->cellY = ((DungeonMap *) sp)->height-1;
							
                            // tell everyone about my arrival
                            curAvatar->IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
							curAvatar->controlledMonster = savedmonster;
							if (curAvatar->controlledMonster)  // if we had a monster, spawn it again like i logged in.
							{
								curAvatar->controlledMonster->cellX = curAvatar->cellX;
								curAvatar->controlledMonster->cellY = curAvatar->cellY;
								curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
								curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
								curAvatar->controlledMonster->isMoving = TRUE;
								curAvatar->controlledMonster->moveStartTime = timeGetTime() - 10000;
								sp->mobList->Add(curAvatar->controlledMonster);
								MessMobAppear mobAppear;

								mobAppear.mobID = (unsigned long)curAvatar->controlledMonster;
								mobAppear.type = curAvatar->controlledMonster->WhatAmI();
								mobAppear.monsterType = curAvatar->controlledMonster->type;
								mobAppear.subType = curAvatar->controlledMonster->subType;

								mobAppear.x = curAvatar->controlledMonster->cellX;
								mobAppear.y = curAvatar->controlledMonster->cellY;
								sp->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY, sizeof(mobAppear), &mobAppear);
								curAvatar->controlledMonster->AnnounceMyselfCustom(sp);
							}
							// tell this player about everyone else around
							curAvatar->UpdateClient(sp, TRUE);
							return;		// move finished.
							
                        }
                    }
                    else if (SPACE_GUILD == sp->WhatAmI())
                    {
                        if (((TowerMap *) sp)->enterX  == tempX + curAvatar->cellX &&
                             ((TowerMap *) sp)->enterY == tempY + curAvatar->cellY &&
							((GroundMap *)ss)->type == 0              // only in normal normal realm
							)
                        {
                            if (! ((TowerMap *) sp)->IsMember(
                                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name) &&
                                 ACCOUNT_TYPE_ADMIN != curAvatar->accountType)
                            {
                                MessInfoText infoText;
                                sprintf_s(tempText,"You cannot enter the guild tower.");
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                sprintf_s(tempText,"You are not in the %s.", ((TowerMap *) sp)->WhoAmI());
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                return;
                            }

                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = NULL;
							curAvatar->activeCounter = 0;
							BBOSMonster *savedmonster = NULL;
							if (curAvatar->controlledMonster) // if i got here and have a controlled monster
							{
								// i need to stash my monster
								savedmonster = curAvatar->controlledMonster;
								MessAdminMessage adminMess; // create an admin message to tell client that it's no lnoger controlling a monster so it doesn't get confused by the map change.
								adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
								lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
								MessMonsterDeath messMD;
								messMD.mobID = (unsigned long)curAvatar->controlledMonster;
								ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(curAvatar->controlledMonster);
								curAvatar->controlledMonster = NULL; // unconfuse the client.
							}
                            // tell everyone I'm dissappearing
                            curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);

                            // tell my client I'm entering the dungeon
                            MessChangeMap changeMap;
                            changeMap.dungeonID = (long) sp;
                            changeMap.oldType = ss->WhatAmI();
                            changeMap.newType = sp->WhatAmI();
                            changeMap.sizeX   = 5;
                            changeMap.sizeY   = 5;
                            changeMap.flags   = MESS_CHANGE_NOTHING;
                            lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                            MessInfoText infoText;
                            sprintf_s(tempText,"You enter the guild tower of %s.", ((TowerMap *) sp)->WhoAmI());
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            curAvatar->QuestSpaceChange(ss,sp);

                            // move me to my new SharedSpace
                            ss->avatars->Remove(curAvatar);
                            sp->avatars->Append(curAvatar);

                            ((TowerMap *)sp)->lastChangedTime.SetToNow();

                            curAvatar->cellX = 4;
                            curAvatar->cellY = 4;

                            // tell everyone about my arrival
                            curAvatar->IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
							curAvatar->controlledMonster = savedmonster;
							if (curAvatar->controlledMonster)  // if we had a monster, spawn it again like i logged in. 
							{
								curAvatar->controlledMonster->cellX = curAvatar->cellX;
								curAvatar->controlledMonster->cellY = curAvatar->cellY;
								curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
								curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
								curAvatar->controlledMonster->isMoving = TRUE;
								curAvatar->controlledMonster->moveStartTime = timeGetTime() - 10000;
								sp->mobList->Add(curAvatar->controlledMonster);
								MessMobAppear mobAppear;

								mobAppear.mobID = (unsigned long)curAvatar->controlledMonster;
								mobAppear.type = curAvatar->controlledMonster->WhatAmI();
								mobAppear.monsterType = curAvatar->controlledMonster->type;
								mobAppear.subType = curAvatar->controlledMonster->subType;

								mobAppear.x = curAvatar->controlledMonster->cellX;
								mobAppear.y = curAvatar->controlledMonster->cellY;
								sp->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY, sizeof(mobAppear), &mobAppear);
								curAvatar->controlledMonster->AnnounceMyselfCustom(sp);
							}

                            // tell this player about everyone else around
                            curAvatar->UpdateClient(sp, TRUE);
                            return;
                        }
                    }
                    sp = (SharedSpace *) spaceList->Next();
                }

            }
            
            if (SPACE_DUNGEON == ss->WhatAmI())
            {
                // stepping in a locked dungeon?
                if (((DungeonMap *)ss)->isLocked &&
                     !(ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                        ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                        ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType))
                {
					BBOSMonster *savedmonster = NULL;
					if (curAvatar->controlledMonster) // if i got here and have a controlled monster
					{
						// i need to stash my monster
						savedmonster = curAvatar->controlledMonster;
						MessAdminMessage adminMess; // create an admin message to tell client that it's no lnoger controlling a monster so it doesn't get confused by the map change.
						adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						MessMonsterDeath messMD;
						messMD.mobID = (unsigned long)curAvatar->controlledMonster;
						ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
							sizeof(messMD), (void *)&messMD);
						// remove it fropm old map
						ss->mobList->Remove(curAvatar->controlledMonster);
						curAvatar->controlledMonster = NULL; // unconfuse the client.
					}
                    // tell everyone I'm dissappearing
                    curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
					curAvatar->activeCounter = 0;

                    // tell my client I'm leaving the dungeon
                    MessChangeMap changeMap;
                    changeMap.oldType = ss->WhatAmI();
                    changeMap.newType = SPACE_GROUND;
                    changeMap.sizeX   = MAP_SIZE_WIDTH;
                    changeMap.sizeY   = MAP_SIZE_HEIGHT;
					changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
                    lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                    SharedSpace *sp = (SharedSpace *) spaceList->Find(SPACE_GROUND);

                    // move me to my new SharedSpace
                    ss->avatars->Remove(curAvatar);
                    sp->avatars->Append(curAvatar);

                    curAvatar->cellX = ((DungeonMap *) ss)->enterX;
                    curAvatar->cellY = ((DungeonMap *) ss)->enterY - 1;

                    // tell everyone about my arrival
                    curAvatar->IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
					curAvatar->controlledMonster = savedmonster;
					if (curAvatar->controlledMonster)  // if we had a monster, spawn it again like i logged in. 
					{
						curAvatar->controlledMonster->cellX = curAvatar->cellX;
						curAvatar->controlledMonster->cellY = curAvatar->cellY;
						curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
						curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
						curAvatar->controlledMonster->isMoving = TRUE;
						curAvatar->controlledMonster->moveStartTime = timeGetTime() - 10000;
						sp->mobList->Add(curAvatar->controlledMonster);
						MessMobAppear mobAppear;

						mobAppear.mobID = (unsigned long)curAvatar->controlledMonster;
						mobAppear.type = curAvatar->controlledMonster->WhatAmI();
						mobAppear.monsterType = curAvatar->controlledMonster->type;
						mobAppear.subType = curAvatar->controlledMonster->subType;

						mobAppear.x = curAvatar->controlledMonster->cellX;
						mobAppear.y = curAvatar->controlledMonster->cellY;
						sp->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY, sizeof(mobAppear), &mobAppear);
						curAvatar->controlledMonster->AnnounceMyselfCustom(sp);
					}

                    // tell this player about everyone else around
                    curAvatar->UpdateClient(sp, TRUE);
                    return;
                }
            }

            if (ss->CanMove(curAvatar->cellX,         curAvatar->cellY, 
                              tempX + curAvatar->cellX, tempY + curAvatar->cellY) &&
                 ss->CanMove(curAvatar->cellX, curAvatar->cellY, 
                              0 + curAvatar->cellX, tempY + curAvatar->cellY) &&
                 ss->CanMove(curAvatar->cellX, curAvatar->cellY, 
                              tempX + curAvatar->cellX, 0 + curAvatar->cellY)  )
            {
                // can't move from a static dungeon monster
                if (SPACE_DUNGEON == ss->WhatAmI())
                {
                    if (!ss->CanMove(tempX + curAvatar->cellX,  curAvatar->cellY, 
                              tempX + curAvatar->cellX,        tempY + curAvatar->cellY) ||
                         !ss->CanMove(curAvatar->cellX, tempY + curAvatar->cellY, 
                              tempX + curAvatar->cellX, tempY + curAvatar->cellY))
                        return;

                    if ( ! ((DungeonMap *) ss)->CanEdit(curAvatar) )
                    {
                        BBOSMob *curMob = (BBOSMob *) ss->mobList->GetFirst(curAvatar->cellX, curAvatar->cellY);
                        while (curMob)
                        {
                            if (SMOB_MONSTER == curMob->WhatAmI())
//								&&
//								 !((BBOSMonster *) curMob)->uniqueName[0])
                            {
                                if (!((BBOSMonster *)curMob)->isWandering &&
                                     !((BBOSMonster *)curMob)->isPossessed && !((BBOSMonster *)curMob)->controllingAvatar)
                                {
                                    if (curMob->cellX == curAvatar->cellX &&
                                         curMob->cellY == curAvatar->cellY)
                                    {
                                        sprintf_s(infoText.text,"You are magically held to this spot!");
                                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                        return;
                                    }
                                }
                            }

                            curMob = (BBOSMob *) ss->mobList->GetNext();
                        }
                    }
                }

                BBOSMob *curMob = (BBOSMob *) ss->mobList->GetFirst(
                                      tempX + curAvatar->cellX, tempY + curAvatar->cellY);
                while (curMob)
                {
                    if (SMOB_WARP_POINT == curMob->WhatAmI())
                    {
                        if ((((BBOSWarpPoint *)curMob)->allCanUse ||
                              ACCOUNT_TYPE_ADMIN == curAvatar->accountType) &&
                             ((BBOSWarpPoint *)curMob)->spaceType < 100)
                        {
							curAvatar->activeCounter = 0;
							// remove all doomkeys
							InventoryObject *io;
							// destroy doomkeys from workbench
							io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.First();
							while (io)
							{
								if (INVOBJ_DOOMKEY == io->type)
								{
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.Remove(io);
									delete io;
								}
								io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.Next();
							}
							// destroy doomkeys from inventory
							io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.First();
							while (io)
							{
								if (INVOBJ_DOOMKEY == io->type)
								{
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Remove(io);
									delete io;
								}
								io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next();
							}
							// destroy doomkeys from wielded area
							io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.First();
							while (io)
							{
								if (INVOBJ_DOOMKEY == io->type)
								{
									curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Remove(io);
									delete io;
								}
								io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Next();
							}
							// remove all earthkey resumes. if you died outside a geo you really shouldn't have done that.
							// but only do this if you are in an earthkey
							if ((ss->WhatAmI() == SPACE_DUNGEON) && (((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY)) // if it's a geo
							{
								// destroy resumes from workbench
								io = (InventoryObject *)((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].workbench->objects.First();
								while (io)
								{
									if (INVOBJ_EARTHKEY_RESUME == io->type)
									{
										((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].workbench->objects.Remove(io);
										delete io;
									}
									io = (InventoryObject *)((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].workbench->objects.Next();
								}
								// destroy resumes from inventory
								io = (InventoryObject *)((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].inventory->objects.First();
								while (io)
								{
									if (INVOBJ_EARTHKEY_RESUME == io->type)
									{
										((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].inventory->objects.Remove(io);
										delete io;
									}
									io = (InventoryObject *)((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].inventory->objects.Next();
								}
								// destroy resumes from wielded area
								io = (InventoryObject *)((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].wield->objects.First();
								while (io)
								{
									if (INVOBJ_EARTHKEY_RESUME == io->type)
									{
										((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].wield->objects.Remove(io);
										delete io;
									}
									io = (InventoryObject *)((BBOSAvatar *)curAvatar)->charInfoArray[((BBOSAvatar *)curAvatar)->curCharacterIndex].wield->objects.Next();
								}
							}
							BBOSMonster *savedmonster = NULL;
							if (curAvatar->controlledMonster) // if i got here and have a controlled monster
							{
								// i need to stash my monster
								savedmonster = curAvatar->controlledMonster;
								MessAdminMessage adminMess; // create an admin message to tell client that it's no lnoger controlling a monster so it doesn't get confused by the map change.
								adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
								lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
								MessMonsterDeath messMD;
								messMD.mobID = (unsigned long)curAvatar->controlledMonster;
								ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(curAvatar->controlledMonster);
								curAvatar->controlledMonster = NULL; // unconfuse the client.
							}

							// tell everyone I'm dissappearing
                            curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);

                            SharedSpace *sp = (SharedSpace *) spaceList->First();
                            while (sp)
                            {
                                if (((BBOSWarpPoint *)curMob)->spaceType == sp->WhatAmI())
                                {
                                    if ((SPACE_REALM == sp->WhatAmI() && 
                                          ((BBOSWarpPoint *)curMob)->spaceSubType == ((RealmMap *)sp)->type)
                                        || (SPACE_LABYRINTH == sp->WhatAmI() && 
                                          ((BBOSWarpPoint *)curMob)->spaceSubType == ((LabyrinthMap *)sp)->type)
                                         || (SPACE_REALM != sp->WhatAmI() && SPACE_LABYRINTH != sp->WhatAmI())
                                        )
                                    {
                                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = NULL;

                                        // tell my client I'm leaving the dungeon
                                        MessChangeMap changeMap;
                                        changeMap.oldType = ss->WhatAmI();
                                        changeMap.newType = ((BBOSWarpPoint *)curMob)->spaceType;
                                        changeMap.realmID = ((BBOSWarpPoint *)curMob)->spaceSubType;
                                        changeMap.sizeX   = sp->sizeX;
                                        changeMap.sizeY   = sp->sizeY;
										changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
                                        lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                                        curAvatar->QuestSpaceChange(ss,sp);

                                        // move me to my new SharedSpace
                                        ss->avatars->Remove(curAvatar);
                                        sp->avatars->Append(curAvatar);

                                        curAvatar->cellX = ((BBOSWarpPoint *)curMob)->targetX;
                                        curAvatar->cellY = ((BBOSWarpPoint *)curMob)->targetY;

                                        // tell everyone about my arrival
                                        curAvatar->IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
										// and handle my monster if i am a beastmaster.
										curAvatar->controlledMonster = savedmonster;
										if (curAvatar->controlledMonster)  // if we had a monster, spawn it again like i logged in. 
										{
											curAvatar->controlledMonster->cellX = curAvatar->cellX;
											curAvatar->controlledMonster->cellY = curAvatar->cellY;
											curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
											curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
											curAvatar->controlledMonster->isMoving = TRUE;
											curAvatar->controlledMonster->moveStartTime = timeGetTime() - 10000;
											sp->mobList->Add(curAvatar->controlledMonster);
											MessMobAppear mobAppear;

											mobAppear.mobID = (unsigned long)curAvatar->controlledMonster;
											mobAppear.type = curAvatar->controlledMonster->WhatAmI();
											mobAppear.monsterType = curAvatar->controlledMonster->type;
											mobAppear.subType = curAvatar->controlledMonster->subType;

											mobAppear.x = curAvatar->controlledMonster->cellX;
											mobAppear.y = curAvatar->controlledMonster->cellY;
											sp->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY, sizeof(mobAppear), &mobAppear);
											curAvatar->controlledMonster->AnnounceMyselfCustom(sp);
										}

                                        // tell this player about everyone else around
                                        curAvatar->UpdateClient(sp, TRUE);
                                        return;
                                    }
                                }
                                sp = (SharedSpace *) spaceList->Next();
                            }
                        }
                    }

                    curMob = (BBOSMob *) ss->mobList->GetNext();
                }
				// if we got this far, we are moving.
                curAvatar->isMoving = TRUE;
				curAvatar->activeCounter = 0;
				curAvatar->targetCellX = tempX + curAvatar->cellX;
                curAvatar->targetCellY = tempY + curAvatar->cellY;
                curAvatar->moveStartTime = timeGetTime();
//				curAvatar->moveTimeCost = 500 * 3; // normally a move takes 3 seconds
				curAvatar->moveTimeCost = 1000 * 3; // normally a move takes 3 seconds
				if (tempX != 0 && tempY != 0)
//					curAvatar->moveTimeCost = 2000; // diagonal moves take 4.5 seconds
   				curAvatar->moveTimeCost = 4000; // diagonal moves take 4.5 seconds
				AvMove.avatarID = fromSocket;
                AvMove.x = curAvatar->cellX;
                AvMove.y = curAvatar->cellY;
                AvMove.targetX = curAvatar->targetCellX;
                AvMove.targetY = curAvatar->targetCellY;
                if (curAvatar->isInvisible || curAvatar->isTeleporting)
                    lserver->SendMsg(sizeof(AvMove),(void *)&AvMove, 
                                          0, &tempReceiptList);
                else
                    ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                               sizeof(AvMove),(void *)&AvMove);

                if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner)
                {
                    SharedSpace *guildSpace;
                    if (FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                    {
                        if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                                inventory->partner ==
                             (((TowerMap *)guildSpace)->itemBox))
                             dungeonUpdateTime = 0; // force dungeons to save

                    }
                }
                curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->partner = NULL;
				if (curAvatar->controlledMonster) //second check
				{
					SharedSpace *sx;

					BBOSMonster * theMonster = FindMonster(
						curAvatar->controlledMonster, &sx);
					if (theMonster && (curAvatar->followMode)) // this is following movement.
					{
						curAvatar->MoveControlledMonster(sx, tempX, tempY); // make the monster follow me, but don't update the camera.
					}
				}

			}

        }
    }
}

//*******************************************************************************
void BBOServer::AddDungeonSorted(DungeonMap *dm)
{
    int dmCenterDist = abs(64 - dm->enterX) + abs(64 - dm->enterY);

    DungeonMap *curDM;
    SharedSpace *sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        if (SPACE_DUNGEON == sp->WhatAmI())
        {
            curDM = (DungeonMap *) sp;
            int curCenterDist = abs(64 - curDM->enterX) + abs(64 - curDM->enterY);
            if (curCenterDist > dmCenterDist)
            {
                spaceList->AddBefore(dm, curDM);
                return;
            }

        }
        sp = (SharedSpace *) spaceList->Next();
    }

    spaceList->Append(dm);

}

//*******************************************************************************
void BBOServer::HandleChatLine(int fromSocket, char *chatText)
{
    SharedSpace *ss, *sp;
    BBOSAvatar *curAvatar;
//	int tempX, tempY;
    MessBladeDesc messBladeDesc;
//	MessAvatarMove AvMove;
    MessPlayerChatLine chatMess;
    MessInfoText infoText;
    int linePoint, argPoint;
    FILE *source;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(fromSocket);
    char tempText[1028], tempText2[124], tempText3[124];
    char t[500], t2[100], t3[100];

    
    curAvatar = FindAvatar(fromSocket, &ss);
    if (curAvatar)
    {
        curAvatar->activeCounter = 0;
//		curAvatar = FindAvatar(fromSocket, &ss);
        if ('/' == chatText[0])
        {
            // process slash commands
            linePoint = 0;

            argPoint = NextWord(chatText,&linePoint);

            //***************************************
            if ( IsSame(&(chatText[argPoint]) , "/teleport") ||
				IsSame(&(chatText[argPoint]), "/tele"))
            {
                argPoint = NextWord(chatText,&linePoint);
                int targX,targY;
                int maxX = 0, maxY = 0;
                if (SPACE_GROUND == ss->WhatAmI())
                {
                    maxX = 256;
                    maxY = 256;
                }
                else if (SPACE_DUNGEON == ss->WhatAmI())
                {
                    maxX = ((DungeonMap *)ss)->width;
                    maxY = ((DungeonMap *)ss)->height;
                }
                else if (SPACE_GUILD == ss->WhatAmI())
                {
                    maxX = ((TowerMap *)ss)->width;
                    maxY = ((TowerMap *)ss)->height;
                }
                else if (SPACE_REALM == ss->WhatAmI())
                {
                    maxX = ((RealmMap *)ss)->width;
                    maxY = ((RealmMap *)ss)->height;
                }
                else if (SPACE_LABYRINTH == ss->WhatAmI())
                {
                    maxX = ((LabyrinthMap *)ss)->width;
                    maxY = ((LabyrinthMap *)ss)->height;
                }
                sscanf(&chatText[argPoint],"%d %d",&targY, &targX);
                targX = maxX - targX;
                targY = maxY - targY;
                if (targX >= 0 && targX < maxX && targY >= 0 && targY < maxY && 
                     (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                      ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                      ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
                    )
                {
                    curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);

                    curAvatar->cellX = curAvatar->targetCellX = targX;
                    curAvatar->cellY = curAvatar->targetCellY = targY;

                    // tell everyone about my arrival
                    curAvatar->IntroduceMyself(ss, SPECIAL_APP_ADMIN_TELEPORT);

                    // tell this player about everyone else around
                    curAvatar->UpdateClient(ss, TRUE);
                }
            }			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/mobcount"))
			{
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
					ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
					ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
				{
					BBOSMob *countMob = (BBOSMob *)ss->mobList->GetFirst(0, 0, 1000);
					int mobcount = 0;
					while (countMob)
					{
						mobcount++;
						countMob = (BBOSMob *)ss->mobList->GetNext();
					}
					sprintf(&(tempText[0]), "There are %d mobs.",
						mobcount);
					CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				}
			}
			else if (IsSame(&(chatText[argPoint]), "/bosscheck"))
			{
					BBOSMob *countMob = (BBOSMob *)ss->mobList->GetFirst(0, 0, 1000);
					if (ss->WhatAmI() == SPACE_REALM) // if it's a realm
					{
						// which realm is it wtf
						if (((RealmMap*)ss)->type == REALM_ID_SPIRITS)
						{
							int DokkCount = 0;
							int CentCount = 0;
							while (countMob)
							{
								if (countMob->WhatAmI() != SMOB_MONSTER) // ship if it's not a monster
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}
								// if it gets here, it's a monster, so monster cast is safe
								if (((BBOSMonster*)countMob)->controllingAvatar) // if it's controled
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}

								// look for dokks and cents
								if ((((BBOSMonster *)countMob)->type == 16) && (((BBOSMonster *)countMob)->subType == 1)) // it's a Dokk
								{
									DokkCount++;
								}
								if ((((BBOSMonster *)countMob)->type == 11) && (((BBOSMonster *)countMob)->subType == 1)) // it's a Cent
								{
									CentCount++;
								}
								countMob = (BBOSMob *)ss->mobList->GetNext();
							}
							sprintf(&(tempText[0]), "There are %d Dokks and %d Cents.",
								DokkCount, CentCount);
							CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
						if (((RealmMap*)ss)->type == REALM_ID_DEAD)
						{
							bool FoundAnubis = false;
							bool FoundLord = false;
							bool FoundTyrant = false;
							bool FoundImperator = false;
							while (countMob)
							{
								if (countMob->WhatAmI() != SMOB_MONSTER) // ship if it's not a monster
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}
								// if it gets here, it's a monster, so monster cast is safe
								if (((BBOSMonster*)countMob)->controllingAvatar) // if it's controled
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}

								// look for dokks and cents
								if ((((BBOSMonster *)countMob)->type == 19) && (((BBOSMonster *)countMob)->subType == 3))
								{
									FoundLord = true;
								}
								if ((((BBOSMonster *)countMob)->type == 19) && (((BBOSMonster *)countMob)->subType == 4))
								{
									FoundTyrant = true;
								}
								if ((((BBOSMonster *)countMob)->type == 19) && (((BBOSMonster *)countMob)->subType == 5))
								{
									FoundImperator = true;
								}
								if (((BBOSMonster *)countMob)->type == 20) // it's a Cent
								{
									FoundAnubis = true;
								}
								countMob = (BBOSMob *)ss->mobList->GetNext();
							}
							if (FoundLord)
							{
								sprintf(&(tempText[0]), "Bone Lord is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundTyrant)
							{
								sprintf(&(tempText[0]), "Bone Tyrant is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundImperator)
							{
								sprintf(&(tempText[0]), "Bone Imperator is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundAnubis)
							{
								sprintf(&(tempText[0]), "Anubis is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
						}
						if (((RealmMap*)ss)->type == REALM_ID_DRAGONS)
						{
							bool FoundChappy = false;
							bool FoundArchmage = false;
							bool FoundOverlord = false;
							int DuskCount = 0;
							while (countMob)
							{
								if (countMob->WhatAmI() != SMOB_MONSTER) // ship if it's not a monster
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}
								// if it gets here, it's a monster, so monster cast is safe
								if (((BBOSMonster*)countMob)->controllingAvatar) // if it's controled
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}

								if ((((BBOSMonster *)countMob)->type == 7) && (((BBOSMonster *)countMob)->subType == 4)) // red dragon
								{
									// check unique name
									if (IsSame(((BBOSMonster *)countMob)->uniqueName, "Dragon Chaplain"))
										FoundChappy = true;
									else if (IsSame(((BBOSMonster *)countMob)->uniqueName, "Dragon ArchMage"))
										FoundArchmage = true;
									else FoundOverlord = true;
								}
								if ((((BBOSMonster *)countMob)->type == 23) && (((BBOSMonster *)countMob)->subType == 2))
								{
									DuskCount++;
								}
								countMob = (BBOSMob *)ss->mobList->GetNext();
							}
							if (FoundChappy)
							{
								sprintf(&(tempText[0]), "Dragon Chaplain is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundArchmage)
							{
								sprintf(&(tempText[0]), "Dragon Archmage is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundOverlord)
							{
								sprintf(&(tempText[0]), "Dragon Overlord is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							sprintf(&(tempText[0]), "There are %d Dusk Wurms.",
								DuskCount);
							CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						}
					}
					if (ss->WhatAmI() == SPACE_LABYRINTH) // if it's a realm
					{
						// which realm is it wtf
						if (((LabyrinthMap*)ss)->type == REALM_ID_LAB1)
						{
							bool FoundLarry = false;
							bool FoundMoe = false;
							bool FoundReo = false;
							while (countMob)
							{
								if (countMob->WhatAmI() != SMOB_MONSTER) // ship if it's not a monster
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}
								// if it gets here, it's a monster, so monster cast is safe
								if (((BBOSMonster*)countMob)->controllingAvatar) // if it's controled
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}

								if ((((BBOSMonster *)countMob)->type == 24) && (((BBOSMonster *)countMob)->subType == 4)) // Spidren
								{
									// check unique name
									if (IsSame(((BBOSMonster *)countMob)->uniqueName, "Larath"))
										FoundLarry = true;
									else if (IsSame(((BBOSMonster *)countMob)->uniqueName, "Moultz"))

										FoundMoe = true;
									else FoundReo = true;
								}
								countMob = (BBOSMob *)ss->mobList->GetNext();
							}
							if (FoundLarry)
							{
								sprintf(&(tempText[0]), "Larath is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundMoe)
							{
								sprintf(&(tempText[0]), "Moultz is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundReo)
							{
								sprintf(&(tempText[0]), "Reorth is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
						}
						if (((LabyrinthMap*)ss)->type == REALM_ID_LAB2)
						{
							bool FoundBaph = false;
							bool FoundBiph = false;
							bool FoundDagon = false;
							while (countMob)
							{
								if (countMob->WhatAmI() != SMOB_MONSTER) // ship if it's not a monster
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}
								// if it gets here, it's a monster, so monster cast is safe
								if (((BBOSMonster*)countMob)->controllingAvatar) // if it's controled
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}

								if (((BBOSMonster *)countMob)->type == 27) // vlord
								{
									// check unique name
									if (IsSame(((BBOSMonster *)countMob)->uniqueName, "Biphomet"))
										FoundBiph = true;
									else if (((BBOSMonster *)countMob)->subType==1)
										FoundDagon = true;
									else FoundBaph = true;
								}
								countMob = (BBOSMob *)ss->mobList->GetNext();
							}
							if (FoundBaph)
							{
								sprintf(&(tempText[0]), "Baphomet is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundBiph)
							{
								sprintf(&(tempText[0]), "Biphomet is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundDagon)
							{
								sprintf(&(tempText[0]), "Dagon is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
						}
						if (((LabyrinthMap*)ss)->type == REALM_ID_LAB3)
						{
							bool FoundMantus = false;
							bool FoundSammael = false;
							bool FoundThoth = false;
							bool FoundApplejack = false;
							bool FoundRarity = false;
							bool FoundRainbowDash = false;
							bool FoundTwilightSparkle = false;
							while (countMob)
							{
								if (countMob->WhatAmI() != SMOB_MONSTER) // ship if it's not a monster
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}
								// if it gets here, it's a monster, so monster cast is safe
								if (((BBOSMonster*)countMob)->controllingAvatar) // if it's controled
								{
									countMob = (BBOSMob *)ss->mobList->GetNext(); // skip it
									continue;										// and restart the loop
								}

								if (((BBOSMonster *)countMob)->type == 27)
								{
									// check subtype
									if (((BBOSMonster *)countMob)->subType == 2)
										FoundMantus = true;
									else if (((BBOSMonster *)countMob)->subType == 3)
										FoundSammael = true;
									else FoundThoth = true;
								}
								if (((BBOSMonster *)countMob)->type == 30)
								{
									// check subtype
									if (((BBOSMonster *)countMob)->subType == 2)
										FoundApplejack = true;
									else if (((BBOSMonster *)countMob)->subType == 3)
										FoundRarity = true;
									else if (((BBOSMonster *)countMob)->subType == 4)
										FoundRainbowDash = true;
									else if (((BBOSMonster *)countMob)->subType == 5)
										FoundTwilightSparkle = true;
								}
								countMob = (BBOSMob *)ss->mobList->GetNext();
							}
							if (FoundMantus)
							{
								sprintf(&(tempText[0]), "Mantus is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundSammael)
							{
								sprintf(&(tempText[0]), "Sammael is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundThoth)
							{
								sprintf(&(tempText[0]), "Thoth is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundApplejack)
							{
								sprintf(&(tempText[0]), "Applejack is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundRarity)
							{
								sprintf(&(tempText[0]), "Rarity is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundRainbowDash)
							{
								sprintf(&(tempText[0]), "Rainbow Dash is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							if (FoundTwilightSparkle)
							{
								sprintf(&(tempText[0]), "Twilight Sparkle is spawned.");
								CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
						}
					}
			}

			else if (IsSame(&(chatText[argPoint]), "/geocount"))
			{
				if (ss->WhatAmI() == SPACE_DUNGEON && (((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY)) // it's a geo
				{
					sprintf(&(tempText[0]), "There are %d monsters left.",
						((DungeonMap *)ss)->MonsterCount);
					CopyStringSafely(tempText, 200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					MessInfoText infoText;

					CopyStringSafely("Only usable in a Geo.",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				}
			}

			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/goto"))
			{
				int len;
				BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

				if (targetAv &&
					(ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
						ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
						ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
					)
				{
					argPoint = linePoint = linePoint + len;

					curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);

					// tell my client I'm entering the realm
					MessChangeMap changeMap;
					changeMap.flags = 0;
					if (SPACE_REALM == sp->WhatAmI())
						changeMap.realmID = ((RealmMap *)sp)->type;
					if (SPACE_LABYRINTH == sp->WhatAmI())
						changeMap.realmID = ((LabyrinthMap *)sp)->type;
					else if (SPACE_DUNGEON == sp->WhatAmI())
					{
						changeMap.dungeonID = (long)sp;
						if ((((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) || (((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
							changeMap.flags = MESS_CHANGE_TEMP;
					}
					if (SPACE_GROUND == sp->WhatAmI())
					{
						changeMap.realmID = ((GroundMap *)sp)->type;
					}
						changeMap.oldType = ss->WhatAmI();
					changeMap.newType = sp->WhatAmI();
					changeMap.sizeX = sp->sizeX;
					changeMap.sizeY = sp->sizeY;

					lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

					MessInfoText infoText;
					sprintf_s(tempText, "You enter the %s.", sp->WhoAmI());
					CopyStringSafely(tempText,
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

					curAvatar->QuestSpaceChange(ss, sp);

					// move me to my new SharedSpace
					ss->avatars->Remove(curAvatar);
					sp->avatars->Append(curAvatar);

					curAvatar->cellX = curAvatar->targetCellX = targetAv->cellX;
					curAvatar->cellY = curAvatar->targetCellY = targetAv->cellY;

					// tell everyone about my arrival
					curAvatar->IntroduceMyself(sp, SPECIAL_APP_ADMIN_TELEPORT);

					// tell this player about everyone else around
					curAvatar->UpdateClient(sp, TRUE);
				}
				else
				{
					sprintf(&chatMess.text[1], "No one of that name is logged on.");
					chatMess.text[0] = TEXT_COLOR_DATA;
					lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/summon"))
			{
				int len;
				BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

				if (targetAv &&
					(ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
						ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
						ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
					)
				{
					argPoint = linePoint = linePoint + len;
					// FIRST< if we have a monster, and aren't in follow mode, enter it. or release the monster if an admin has it.
					if (targetAv->controlledMonster && !targetAv->followMode)
					{
						// return client to follow mode if it's a beastmaster,or pmve camera backto where player is if it's an admin.
						MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
						tempReceiptList.clear();
						tempReceiptList.push_back(targetAv->socketIndex); // point server at the target client.
						adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						// and resturn server to follow mode, since both will end up in the same spot.
						if (targetAv->BeastStat() > 9)
							targetAv->followMode = TRUE;
						// we need to dissapear it from people's screen.
					}

					targetAv->AnnounceDisappearing(sp, SPECIAL_APP_ADMIN_TELEPORT); //second, vanish the player
					// third, vanish the monster
					if (targetAv->controlledMonster)
					{
						// we need to dissapear it from people's screen.
						MessMonsterDeath messMD;
						messMD.mobID = (unsigned long)targetAv->controlledMonster;
						sp->SendToEveryoneNearBut(0, targetAv->controlledMonster->cellX, targetAv->controlledMonster->cellY,
							sizeof(messMD), (void *)&messMD); 
						// remove it fropm old map
						sp->mobList->Remove(targetAv->controlledMonster);

					}
					// NEXT, change the coordinates
					// move me to my new SharedSpace
					sp->avatars->Remove(targetAv);
					targetAv->cellX = targetAv->targetCellX = curAvatar->cellX;
					targetAv->cellY = targetAv->targetCellY = curAvatar->cellY;
					ss->avatars->Append(targetAv);

					if (targetAv->controlledMonster) // if they had a monster
					{
						// then it needs ot have it's coords updated, and be added to new map
						targetAv->controlledMonster->cellX = targetAv->controlledMonster->targetCellX = curAvatar->cellX;
						targetAv->controlledMonster->cellY = targetAv->controlledMonster->targetCellY = curAvatar->cellY;
						ss->mobList->Add(targetAv->controlledMonster);
					}

					// next, change the map.
					MessChangeMap changeMap;
					changeMap.flags = MESS_CHANGE_NOTHING;
					if (SPACE_REALM == ss->WhatAmI())
						changeMap.realmID = ((RealmMap *)ss)->type;
					if (SPACE_LABYRINTH == ss->WhatAmI())
						changeMap.realmID = ((LabyrinthMap *)ss)->type;
					else if (SPACE_DUNGEON == ss->WhatAmI())
					{
						changeMap.dungeonID = (long)ss;
						if ((((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) || (((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
							changeMap.flags = MESS_CHANGE_TEMP;
					}
					if (SPACE_GROUND == ss->WhatAmI())
					{
						changeMap.realmID = ((GroundMap *)ss)->type;
					}

					changeMap.oldType = sp->WhatAmI();
					changeMap.newType = ss->WhatAmI();
					changeMap.sizeX = ss->sizeX;
					changeMap.sizeY = ss->sizeY;
					tempReceiptList.clear();
					tempReceiptList.push_back(targetAv->socketIndex);
					lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

					targetAv->QuestSpaceChange(sp, ss);
					// next, appear.
					targetAv->IntroduceMyself(ss, SPECIAL_APP_ADMIN_TELEPORT);
					if (targetAv->controlledMonster) // if they had a monster
					{
						// announce monster
						targetAv->controlledMonster->AnnounceMyselfCustom(ss);
					}
					targetAv->UpdateClient(ss, TRUE);
					MessInfoText infoText;
					sprintf_s(tempText, "You enter the %s.", ss->WhoAmI());
					CopyStringSafely(tempText,
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					// tell everyone about my arrival

					// need to log this to catch cheating admins
					source = fopen("summonlog.txt", "a");
					/* Display operating system-style date and time. */
					_strdate(tempText2);
					fprintf(source, "%s, ", tempText2);
					_strtime(tempText2);
					fprintf(source, "%s, ", tempText2);
					fprintf(source, "%s, ", curAvatar->name);
					fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
					fprintf(source, "%d, ", curAvatar->cellX);
					fprintf(source, "%d, ", curAvatar->cellY);
					fprintf(source, "%s, ", targetAv->name);
					fprintf(source, "%s, ", targetAv->charInfoArray[targetAv->curCharacterIndex].name);
					fprintf(source, "%s\n ", ss->WhoAmI());
					fclose(source);

				}
				else
				{
					sprintf(&chatMess.text[1], "No one of that name is logged on.");
					chatMess.text[0] = TEXT_COLOR_DATA;
					lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
				}
			}
			
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/rawcheck"))
			{
				if (SPACE_GROUND == ss->WhatAmI())
				{
					// check map raw byte
					GroundMap * gm = (GroundMap *)ss;
					int gmcolor = gm->Color(curAvatar->cellX, curAvatar->cellY);
					MessInfoText infoText;
					sprintf_s(tempText, "Color is %d", gmcolor);
					CopyStringSafely(tempText,
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/translate"))
			{
                argPoint = NextWord(chatText,&linePoint);
                int targX,targY;

                int maxX = 0, maxY = 0;
                if (SPACE_GROUND == ss->WhatAmI())
                {
                    maxX = 256;
                    maxY = 256;
                }
                else if (SPACE_DUNGEON == ss->WhatAmI())
                {
                    maxX = ((DungeonMap *)ss)->width;
                    maxY = ((DungeonMap *)ss)->height;
                }
                else if (SPACE_GUILD == ss->WhatAmI())
                {
                    maxX = ((TowerMap *)ss)->width;
                    maxY = ((TowerMap *)ss)->height;
                }
                else if (SPACE_REALM == ss->WhatAmI())
                {
                    maxX = ((RealmMap *)ss)->width;
                    maxY = ((RealmMap *)ss)->height;
                }
                else if (SPACE_LABYRINTH == ss->WhatAmI())
                {
                    maxX = ((LabyrinthMap *)ss)->width;
                    maxY = ((LabyrinthMap *)ss)->height;
                }

                sscanf(&chatText[argPoint],"%d %d",&targY, &targX);
                targX = maxX - targX;
                targY = maxY - targY;

                MessInfoText infoText;
                sprintf_s(tempText,"That's really %dx %dy.", targX, targY);
                CopyStringSafely(tempText, 
                                      200, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/realm"))
			{
				argPoint = NextWord(chatText, &linePoint);

				int realmID = REALM_ID_SPIRITS;
				if ('d' == chatText[argPoint])
					realmID = REALM_ID_DEAD;
				if ('r' == chatText[argPoint])
					realmID = REALM_ID_DRAGONS;
				if ('l' == chatText[argPoint])
					realmID = REALM_ID_LAB1;
				if ('m' == chatText[argPoint])
					realmID = REALM_ID_LAB2;
				if ('b' == chatText[argPoint])
					realmID = REALM_ID_LAB3;
				if ('t' == chatText[argPoint])
					realmID = REALM_ID_TEST;
				if ('n' == chatText[argPoint])
					realmID = REALM_ID_MAX;


				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
					ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
					ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType ||
					realmID==REALM_ID_TEST)                                       // normal players can go to testrealm for now.
				{
					boolean mapfound = false;
					// check for monster
					// gotta check for controlling monster
					// am i controlling a monster?
					if (curAvatar->controlledMonster && !curAvatar->followMode)
					{
						MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
						tempReceiptList.clear();
						tempReceiptList.push_back(curAvatar->socketIndex); // point server at the target client.
						adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						// and return to follow mode, since both will end up in the same spot.
						if (curAvatar->BeastStat() > 9)
							curAvatar->followMode = TRUE;
							// we need to dissapear it from people's screen.
					}

					if (realmID < REALM_ID_MAX)
					{
						RealmMap *rp = NULL;
						SharedSpace *sp = (SharedSpace *)spaceList->First();
						while (!mapfound && sp)
						{
							if (SPACE_REALM == sp->WhatAmI() && realmID == ((RealmMap *)sp)->type)
							{
								rp = (RealmMap *)sp;
								mapfound = true;
							}
							sp = (SharedSpace *)spaceList->Next();
						}

						if (rp)
						{
							curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);
							// vanish the controlled monster if present
							if (curAvatar->controlledMonster)
							{
								// we need to dissapear it from people's screen.
								MessMonsterDeath messMD;
								messMD.mobID = (unsigned long)curAvatar->controlledMonster;
								ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(curAvatar->controlledMonster);

							}
							// tell my client I'm entering the realm
							tempReceiptList.clear();
							tempReceiptList.push_back(curAvatar->socketIndex); // point server at the target client.
							ss->avatars->Remove(curAvatar);

							curAvatar->cellX = curAvatar->targetCellX = (rand() % 4) + 2;
							curAvatar->cellY = curAvatar->targetCellY = (rand() % 4) + 2;
							rp->avatars->Append(curAvatar);
							if (curAvatar->controlledMonster) // if they had a monster
							{
								// then it needs ot have it's coords updated, and be added to new map
								curAvatar->controlledMonster->cellX = curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
								curAvatar->controlledMonster->cellY = curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
								rp->mobList->Add(curAvatar->controlledMonster);
							}

							MessChangeMap changeMap;
							changeMap.realmID = rp->type;
							changeMap.oldType = ss->WhatAmI();
							changeMap.newType = rp->WhatAmI();
							changeMap.sizeX = rp->sizeX;
							changeMap.sizeY = rp->sizeY;
							changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
							changeMap.realmID = ((RealmMap *)rp)->type;
							lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

							MessInfoText infoText;
							sprintf_s(tempText, "You enter the %s.", rp->WhoAmI());
							CopyStringSafely(tempText,
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

							curAvatar->QuestSpaceChange(ss, rp);

							// tell everyone about my arrival
							curAvatar->IntroduceMyself(rp, SPECIAL_APP_ADMIN_TELEPORT);
							if (curAvatar->controlledMonster)
							{
								// tell clients about my monster
								curAvatar->controlledMonster->AnnounceMyselfCustom(rp);
							}
							// tell this player about everyone else around
							curAvatar->UpdateClient(rp, TRUE);
						}
						else
						{
							// enter labyrinth?

							LabyrinthMap *lp;
							sp = (SharedSpace *)spaceList->First();
							while (sp)
							{
								if (SPACE_LABYRINTH == sp->WhatAmI() && realmID == ((RealmMap *)sp)->type)
								{
									lp = (LabyrinthMap *)sp;
									sp = (SharedSpace *)spaceList->Last();
								}
								sp = (SharedSpace *)spaceList->Next();
							}

							if (lp)
							{
								curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);
								// vanish the controlled monster if present
								if (curAvatar->controlledMonster)
								{
									// we need to dissapear it from people's screen.
									MessMonsterDeath messMD;
									messMD.mobID = (unsigned long)curAvatar->controlledMonster;
									ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
										sizeof(messMD), (void *)&messMD);
									// remove it fropm old map
									ss->mobList->Remove(curAvatar->controlledMonster);

								}

								// tell my client I'm entering the realm
								MessChangeMap changeMap;
								changeMap.realmID = lp->type;
								changeMap.oldType = ss->WhatAmI();
								changeMap.newType = lp->WhatAmI();
								changeMap.sizeX = lp->sizeX;
								changeMap.sizeY = lp->sizeY;
								changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
								lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

								MessInfoText infoText;
								sprintf_s(tempText, "You enter the %s.", lp->WhoAmI());
								CopyStringSafely(tempText,
									200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

								curAvatar->QuestSpaceChange(ss, lp);

								// move me to my new SharedSpace
								ss->avatars->Remove(curAvatar);
								lp->avatars->Append(curAvatar);

								curAvatar->cellX = curAvatar->targetCellX = 1;
								curAvatar->cellY = curAvatar->targetCellY = 3;
								if (REALM_ID_LAB2 == realmID)
								{
									curAvatar->cellX = curAvatar->targetCellX = 53;
									curAvatar->cellY = curAvatar->targetCellY = 41;
								}
								if (REALM_ID_LAB3 == realmID)
								{
									curAvatar->cellX = curAvatar->targetCellX = 1;
									curAvatar->cellY = curAvatar->targetCellY = 0;
								}
								curAvatar->cellX = curAvatar->targetCellX = (rand() % 4) + 2;
								curAvatar->cellY = curAvatar->targetCellY = (rand() % 4) + 2;
								if (curAvatar->controlledMonster) // if they had a monster
								{
									// then it needs ot have it's coords updated, and be added to new map
									curAvatar->controlledMonster->cellX = curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
									curAvatar->controlledMonster->cellY = curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
									lp->mobList->Add(curAvatar->controlledMonster);
								}

								// tell everyone about my arrival
								curAvatar->IntroduceMyself(lp, SPECIAL_APP_ADMIN_TELEPORT);
								if (curAvatar->controlledMonster)
								{
									// tell clients about my monster
									curAvatar->controlledMonster->AnnounceMyselfCustom(lp);
								}

								// tell this player about everyone else around
								curAvatar->UpdateClient(lp, TRUE);
							}
						}
					}
					else  // enter test normal realm copy.
					{
						GroundMap *rp = NULL;
						SharedSpace *sp = (SharedSpace *)spaceList->Last(); // go backwards to find the second copy.
						while (sp)
						{
							if (SPACE_GROUND == sp->WhatAmI())
							{
								rp = (GroundMap *)sp;
								sp = (SharedSpace *)spaceList->First();
							}
							sp = (SharedSpace *)spaceList->Prev();
						}

						if (rp)
						{
							curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);
							if (curAvatar->controlledMonster)
							{
								// we need to dissapear it from people's screen.
								MessMonsterDeath messMD;
								messMD.mobID = (unsigned long)curAvatar->controlledMonster;
								ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(curAvatar->controlledMonster);

							}

							// tell my client I'm entering the realm
							MessChangeMap changeMap;
							changeMap.realmID = rp->type;
							changeMap.oldType = ss->WhatAmI();
							changeMap.newType = rp->WhatAmI();
							changeMap.sizeX = rp->sizeX;
							changeMap.sizeY = rp->sizeY;
							changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
							lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

							MessInfoText infoText;
							sprintf_s(tempText, "You enter the %s.", rp->WhoAmI());
							CopyStringSafely(tempText,
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

							curAvatar->QuestSpaceChange(ss, rp);

							// move me to my new SharedSpace
							ss->avatars->Remove(curAvatar);
							rp->avatars->Append(curAvatar);

							curAvatar->cellX = curAvatar->targetCellX = (rand() % 4) + 98;
							curAvatar->cellY = curAvatar->targetCellY = (rand() % 4) + 44;
							if (curAvatar->controlledMonster) // if they had a monster
							{
								// then it needs ot have it's coords updated, and be added to new map
								curAvatar->controlledMonster->cellX = curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
								curAvatar->controlledMonster->cellY = curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
								rp->mobList->Add(curAvatar->controlledMonster);
							}

							// tell everyone about my arrival
							curAvatar->IntroduceMyself(rp, SPECIAL_APP_ADMIN_TELEPORT);
							if (curAvatar->controlledMonster)
							{
								// tell clients about my monster
								curAvatar->controlledMonster->AnnounceMyselfCustom(rp);
							}

							// tell this player about everyone else around
							curAvatar->UpdateClient(rp, TRUE);
						}

					}
				}
			}
			//***************************************
			else if ((IsSame(&(chatText[argPoint]), "/testmap")) && (enable_cheating == 0)) // no testmap command in cheaters edition at the moment.
			{
					
				if (ss->WhatAmI() == SPACE_GROUND) // only works in normal realm, to stop you from escaping where reto doesn't work.
				{
					GroundMap *rp = NULL;
					SharedSpace *sp = (SharedSpace *)spaceList->Last(); // go backwards to find the second copy.
					while (sp)
						{
							if (SPACE_GROUND == sp->WhatAmI())
							{
								rp = (GroundMap *)sp;
								sp = (SharedSpace *)spaceList->First();
							}
							sp = (SharedSpace *)spaceList->Prev();
						}

					if (rp)
						{
							// first part - before dissapearing
							BBOSMonster *savedmonster = NULL;
							if (curAvatar->controlledMonster) // if i got here and have a controlled monster
							{
								// i need to stash my monster
								savedmonster = curAvatar->controlledMonster;
								MessAdminMessage adminMess; // create an admin message to tell client that it's no lnoger controlling a monster so it doesn't get confused by the map change.
								adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
								lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
								MessMonsterDeath messMD;
								messMD.mobID = (unsigned long)curAvatar->controlledMonster;
								ss->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
									sizeof(messMD), (void *)&messMD);
								// remove it fropm old map
								ss->mobList->Remove(curAvatar->controlledMonster);
								curAvatar->controlledMonster = NULL; // unconfuse the client.
							}


							curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_ADMIN_TELEPORT);

							// tell my client I'm entering the realm
							MessChangeMap changeMap;
							changeMap.realmID = rp->type;
							changeMap.oldType = ss->WhatAmI();
							changeMap.newType = rp->WhatAmI();
							changeMap.sizeX = rp->sizeX;
							changeMap.sizeY = rp->sizeY;
							changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
							lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

							MessInfoText infoText;
							sprintf_s(tempText, "You enter the %s.", rp->WhoAmI());
							CopyStringSafely(tempText,
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

							curAvatar->QuestSpaceChange(ss, rp);

							// move me to my new SharedSpace
							ss->avatars->Remove(curAvatar);
							rp->avatars->Append(curAvatar);

							curAvatar->cellX = curAvatar->targetCellX = (rand() % 4) + 98;
							curAvatar->cellY = curAvatar->targetCellY = (rand() % 4) + 44;

							// tell everyone about my arrival
							curAvatar->IntroduceMyself(rp, SPECIAL_APP_ADMIN_TELEPORT);
							// second part. after introducingmyself
							curAvatar->controlledMonster = savedmonster;
							if (curAvatar->controlledMonster)  // if we had a monster, spawn it again like i logged in. 
							{
								curAvatar->controlledMonster->cellX = curAvatar->cellX;
								curAvatar->controlledMonster->cellY = curAvatar->cellY;
								curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
								curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
								curAvatar->controlledMonster->isMoving = TRUE;
								curAvatar->controlledMonster->moveStartTime = timeGetTime() - 10000;
								rp->mobList->Add(curAvatar->controlledMonster);
								MessMobAppear mobAppear;

								mobAppear.mobID = (unsigned long)curAvatar->controlledMonster;
								mobAppear.type = curAvatar->controlledMonster->WhatAmI();
								mobAppear.monsterType = curAvatar->controlledMonster->type;
								mobAppear.subType = curAvatar->controlledMonster->subType;

								mobAppear.x = curAvatar->controlledMonster->cellX;
								mobAppear.y = curAvatar->controlledMonster->cellY;
								rp->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY, sizeof(mobAppear), &mobAppear);
								curAvatar->controlledMonster->AnnounceMyselfCustom(rp);
							}

							// tell this player about everyone else around
							curAvatar->UpdateClient(rp, TRUE);
						}

				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/xmas"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					// get the main world.
					SharedSpace* ef = (SharedSpace*)spaceList->First();
					// loop through the generators. 
					BBOSGenerator* qGen = (BBOSGenerator*)ef->generators->First();
					int questsearch; //save the ID 
					int questcount = 0; // count the number of autoquests we find
					while (qGen)
					{
						// check it's do_id
						questsearch = qGen->do_id;
						if (questsearch > 1) // it's a autoquest
						{
							// then we figure out which one it is. we know what order they are in the list
							questcount = questcount++;
							switch (questcount)
							{
							case 1:  // revenant quest.
								break; // do nothing for this test
							case 2:  // invasion quest
								break; // do nothing, it can't be tweaked,
							case 3:  // it's the xmas event, start it up
							{
								BBOSChristmasQuest* cquest = (BBOSChristmasQuest*)qGen;
								cquest->questState = 0; // CHRISTMASQUEST_READY
								break;
							}
							default: // can't happen
								break;
							}
						}
						qGen = (BBOSGenerator*)ef->generators->Next();
					}
				}
			}
			else if (IsSame(&(chatText[argPoint]), "/vday"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					int heartcount;
					sscanf(&chatText[argPoint], "%d", &heartcount);

					if (heartcount < 1)
					{
						MessInfoText infoText;
						CopyStringSafely("/vday <number of hearts>",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void*)&infoText, 0, &tempReceiptList);
					}
					else
					{
						// get the main world.
						SharedSpace* ef = (SharedSpace*)spaceList->First();
						// spawn a bunch of hearts
						while (heartcount > 0)
						{
							// spawn a heart.
							// pick a town omgbbq
							int town = rand() % NUM_OF_TOWNS ;
							int randomX = rand() % 10 - 4;
							int randomY = rand() % 10 - 4;

							// add a heart in random town square
							BBOSMonster *monster = new BBOSMonster(33, 0, NULL);
							monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
							monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
							ef->mobList->Add(monster);
							heartcount--; // decrease counter, gotta make sure the loop ends.
						}
					}
				}
			}
			else if (IsSame(&(chatText[argPoint]), "/egg"))
			{
			argPoint = NextWord(chatText, &linePoint);
			if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
			{
				int targX, targY, amount;
				sscanf(&chatText[argPoint], "%d %d %d", &targX, &targY, &amount);

				if (targX < 0 || targX >= 4 || targY < 0 || targY >= 7)
				{
					MessInfoText infoText;
					CopyStringSafely("/egg <quality (0-3)> <type (0-6)> <amount>",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void*)&infoText, 0, &tempReceiptList);
				}
				else
				{
					if (amount < 1)
						amount = 1;

					Inventory* inv = NULL;
					//= &curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

					InventoryObject* iObject = new InventoryObject(
						INVOBJ_EGG, 0, dragonInfo[targX][targY].eggName);
					iObject->mass = 1.0f;
					iObject->value = 1000;
					iObject->amount = amount;
					InvEgg* im = (InvEgg*)iObject->extra;
					im->type = targY;
					im->quality = targX;

					if (curAvatar->controlledMonster)
					{
						SharedSpace* sx;

						BBOSMonster* theMonster = FindMonster(
							curAvatar->controlledMonster, &sx);
						if (theMonster)						// this is an admin command s don't need to check.
							inv = (theMonster->inventory);
					}
					if (!inv)
						inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

					inv->AddItemSorted(iObject);

					TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
				}
			}
			}
			//***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/create"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                {
                    if (argPoint == linePoint)
                    {
                        MessInfoText infoText;
                        CopyStringSafely("/create <amount> <monetary value> <text name>", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        int amount, value;
                        Inventory *inv = NULL;

                        sscanf(&chatText[argPoint],"%d",&amount);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&value);
                        argPoint = NextWord(chatText,&linePoint);

                        if (amount < 1)
                            amount = 1;
                        if (amount > 100000)
                            amount = 10000;
                        if (value < 0)
                            value = 0;
                        if (value > 1000000)
                            value = 1000000;

                        if (curAvatar->controlledMonster)
                        {
                            SharedSpace *sx;

                            BBOSMonster * theMonster = FindMonster(
                                      curAvatar->controlledMonster, &sx);
                            if (theMonster)
                                inv = (theMonster->inventory);
                        }
                        if (!inv)
                                inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

                        InventoryObject *iObject = new InventoryObject(
                                    INVOBJ_SIMPLE,0,&chatText[argPoint]);
                        iObject->amount = amount;
                        iObject->value = value;
                        inv->AddItemSorted(iObject);

                        TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
						//log it
						source = fopen("admincreatelog.txt", "a");
						/* Display operating system-style date and time. */
						_strdate(tempText2);
						fprintf(source, "%s, ", tempText2);
						_strtime(tempText2);
						fprintf(source, "%s, ", tempText2);
						fprintf(source, "%s, ", curAvatar->name);
						fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
						fprintf(source, "%s\n", chatText);
						fclose(source);


                    }
                }
            }
            //***************************************
			else if (IsSame(&(chatText[argPoint]), "/dust"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/dust <green OR blue OR black OR white OR red OR gold OR silver> <quantity>",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int type = -1;
						int amount = -1;
						if (IsSame(&chatText[argPoint], "green"))
							type = INGR_GREEN_DUST;
						if (IsSame(&chatText[argPoint], "blue"))
							type = INGR_BLUE_DUST;
						if (IsSame(&chatText[argPoint], "black"))
							type = INGR_BLACK_DUST;
						if (IsSame(&chatText[argPoint], "white"))
							type = INGR_WHITE_DUST;
						if (IsSame(&chatText[argPoint], "red"))
							type = INGR_RED_DUST;
						if (IsSame(&chatText[argPoint], "gold"))
							type = INGR_GOLD_DUST;
						if (IsSame(&chatText[argPoint], "silver"))
							type = INGR_SILVER_DUST;
						argPoint = NextWord(chatText, &linePoint);
						sscanf(&chatText[argPoint], "%d", &amount);
						if (amount < 1)
							amount = 1;
						Inventory *inv = NULL;

						if (type != -1)
						{
							if (curAvatar->controlledMonster)
							{
								SharedSpace *sx;

								BBOSMonster * theMonster = FindMonster(
									curAvatar->controlledMonster, &sx);
								if (theMonster)
									inv = (theMonster->inventory);
							}
							if (!inv)
								inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

							InventoryObject *iObject = new InventoryObject(
								INVOBJ_INGREDIENT, 0, dustNames[type]);
							InvIngredient *exIn = (InvIngredient *)iObject->extra;
							exIn->type = type;
							exIn->quality = 1;

							iObject->mass = 0.0f;
							iObject->value = 1000;
							iObject->amount = amount;
							inv->AddItemSorted(iObject);

							TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
							// log what and who did it
							source = fopen("adminDustGeneration.txt", "a");
							/* Display operating system-style date and time. */
							_strdate(tempText2);
							fprintf(source, "%s, ", tempText2);
							_strtime(tempText2);
							fprintf(source, "%s, ", tempText2);
							fprintf(source, "%s, ", curAvatar->name);
							fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							fprintf(source, "%d, ", amount);
							fprintf(source, "%s\n", dustNames[type]);
							fclose(source);
						}
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/feeddust"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (curAvatar->controlledMonster) //
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/feeddust <green OR blue OR black OR white OR red OR gold> <quantity>",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int type = -1;
						int amount = -1;
						if (IsSame(&chatText[argPoint], "green"))
							type = INGR_GREEN_DUST;
						if (IsSame(&chatText[argPoint], "blue"))
							type = INGR_BLUE_DUST;
						if (IsSame(&chatText[argPoint], "black"))
							type = INGR_BLACK_DUST;
						if (IsSame(&chatText[argPoint], "white"))
							type = INGR_WHITE_DUST;
						if (IsSame(&chatText[argPoint], "red"))
							type = INGR_RED_DUST;
						if (IsSame(&chatText[argPoint], "gold"))
							type = INGR_GOLD_DUST;
						argPoint = NextWord(chatText, &linePoint);
						sscanf(&chatText[argPoint], "%d", &amount);
						if (amount < 1)
							amount = 1;
						Inventory *inv = NULL;
							
						if (type != -1)
						{
							inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;
							// search for named dust 
							InventoryObject *iObject = (InventoryObject *) inv->objects.First();
							while (iObject)
							{
								if (INVOBJ_INGREDIENT == iObject->type)
								{
									InvIngredient * ingre = (InvIngredient *)iObject->extra;
									if (ingre->type == type) // if it's the right one.
									{
										if (iObject->amount < amount)
										{
											MessInfoText infoText;
											CopyStringSafely("Not Enough Dusts to feed.",
												200, infoText.text, MESSINFOTEXTLEN);
											lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
											return;
										}
										else
										{
											MessInfoText infoText;
											switch (type)
											{
											case INGR_RED_DUST:
												if (amount > curAvatar->controlledMonster->r)
												{
													curAvatar->controlledMonster->r = 0;
												}
												else
												{
													curAvatar->controlledMonster->r = curAvatar->controlledMonster->r - amount;
												}
												CopyStringSafely("Red decreased.",
													200, infoText.text, MESSINFOTEXTLEN);
												lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

												break;
											case INGR_GREEN_DUST:
												if (amount > curAvatar->controlledMonster->g)
												{
													curAvatar->controlledMonster->g = 0;
												}
												else
												{
													curAvatar->controlledMonster->g = curAvatar->controlledMonster->g - amount;
												}
												CopyStringSafely("Green decreased.",
													200, infoText.text, MESSINFOTEXTLEN);
												lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
												break;
											case INGR_BLUE_DUST:
												if (amount > curAvatar->controlledMonster->b)
												{
													curAvatar->controlledMonster->b = 0;
												}
												else
												{
													curAvatar->controlledMonster->b = curAvatar->controlledMonster->b - amount;
												}
												CopyStringSafely("Blue decreased.",
													200, infoText.text, MESSINFOTEXTLEN);
												lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
												break;
											case INGR_BLACK_DUST:
												if (amount > curAvatar->controlledMonster->r)
												{
													curAvatar->controlledMonster->r = 0;
												}
												else
												{
													curAvatar->controlledMonster->r = curAvatar->controlledMonster->r - amount;
												}
												if (amount > curAvatar->controlledMonster->g)
												{
													curAvatar->controlledMonster->g = 0;
												}
												else
												{
													curAvatar->controlledMonster->g = curAvatar->controlledMonster->g - amount;
												}
												if (amount > curAvatar->controlledMonster->b)
												{
													curAvatar->controlledMonster->b = 0;
												}
												else
												{
													curAvatar->controlledMonster->b = curAvatar->controlledMonster->b - amount;
												}
												CopyStringSafely("All colors decreased.",
													200, infoText.text, MESSINFOTEXTLEN);
												lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
												break;
											case INGR_WHITE_DUST:
												if (amount > (255-curAvatar->controlledMonster->r))
												{
													curAvatar->controlledMonster->r = 255;
												}
												else
												{
													curAvatar->controlledMonster->r = curAvatar->controlledMonster->r + amount;
												}
												if (amount > (255 - curAvatar->controlledMonster->g))
												{
													curAvatar->controlledMonster->g = 255;
												}
												else
												{
													curAvatar->controlledMonster->g = curAvatar->controlledMonster->g + amount;
												}
												if (amount > (255 - curAvatar->controlledMonster->b))
												{
													curAvatar->controlledMonster->b = 255;
												}
												else
												{
													curAvatar->controlledMonster->b = curAvatar->controlledMonster->b + amount;
												}
												CopyStringSafely("All colors increased.",
													200, infoText.text, MESSINFOTEXTLEN);
												lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
												break;
											case INGR_GOLD_DUST:
												if (amount > curAvatar->controlledMonster->r)
												{
													curAvatar->controlledMonster->a = 0;
												}
												else
												{
													curAvatar->controlledMonster->a = curAvatar->controlledMonster->a - amount;
												}
												CopyStringSafely("Transparency increased.",
													200, infoText.text, MESSINFOTEXTLEN);
												lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

												break;
											}
											iObject->amount = iObject->amount - amount;
											if (iObject->amount < 1)
											{
												// delete the item
												inv->objects.Remove(iObject);
												delete iObject;
											}
											curAvatar->controlledMonster->AnnounceMyselfCustom(ss); // reannounce to change the color on the clients.
											return;  // if we got here, we found the correct dust and used it. we don't need to continue.
										}
									}
								}
								// wrong dust, or not a dust,  go to the next object.  
								iObject = (InventoryObject *)inv->objects.Next();
							}

						}
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/feedheart")) // 
			{
				argPoint = NextWord(chatText, &linePoint);
				if (curAvatar->controlledMonster) //
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/feedheart <quantity>",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int amount = -1;
						bool foundheart = FALSE;
						sscanf(&chatText[argPoint], "%d", &amount);
						if (amount < 1)
							amount = 1;
						Inventory *inv = NULL;
						inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;
						// search for some heart gems 
						InventoryObject *iObject = (InventoryObject *)inv->objects.First();
						while (iObject)
						{
								if (INVOBJ_GEOPART == iObject->type)
								{
									InvGeoPart * ingre = (InvGeoPart *)iObject->extra;
									if (ingre->type > 0) // it's ah eart gem
									{
										if (iObject->amount < amount)
										{
											MessInfoText infoText;
											CopyStringSafely("Not enough heart gems in the first stack to feed.", 100, infoText.text, MESSINFOTEXTLEN);
											lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
											return;
										}
										else
										{
											foundheart = TRUE; // we found some heart gems
											// we heal
											MessInfoText infoText;
											int heal= 100 * amount*(int)ingre->power*(int)ingre->power;
											int altheal = curAvatar->controlledMonster->maxHealth / (12 - (int)ingre->power)*amount; 
											if (altheal > heal)
												heal = altheal;
											curAvatar->controlledMonster->health += 100*amount*(int)ingre->power*(int)ingre->power;
											if (curAvatar->controlledMonster->health > curAvatar->controlledMonster->maxHealth)
											{
												curAvatar->controlledMonster->health = curAvatar->controlledMonster->maxHealth; // no overhealing
											}

											CopyStringSafely("Pet Healed.", 100, infoText.text, MESSINFOTEXTLEN);
											lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
											iObject->amount = iObject->amount - amount;
											if (iObject->amount < 1)
											{
												// delete the item
												inv->objects.Remove(iObject);
												delete iObject;
											}
										}
									}
									if (foundheart)
									{
										// announce the monster to register the health change
										curAvatar->controlledMonster->AnnounceMyselfCustom(ss);
										// and return
										return;
									}
								}
								// wrong meat age, or not a dust,  go to the next object.  
								iObject = (InventoryObject *)inv->objects.Next();
							
						}
					}
				}
			}
			else if (IsSame(&(chatText[argPoint]), "/petstats"))
			{
			if (curAvatar->controlledMonster)
			{
				SharedSpace *sp;

				BBOSMonster * theMonster = FindMonster(curAvatar->controlledMonster, &sp);
				if (theMonster)
				{
					sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
						theMonster->health, theMonster->maxHealth, theMonster->damageDone,
						theMonster->toHit, theMonster->defense, theMonster->dropAmount,
						theMonster->magicResistance, theMonster->healAmountPerSecond);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						return;
					}
				}
			}
			else if (IsSame(&(chatText[argPoint]), "/feedmeat")) // 
			{
			argPoint = NextWord(chatText, &linePoint);
			if (curAvatar->controlledMonster) //
			{
				if (argPoint == linePoint)
				{
					MessInfoText infoText;
					CopyStringSafely("/feedmeat <fresh OR rotted OR salted> <quantity>",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					int age = -2;
					int amount = -1;
					bool foundmeat = FALSE;
					if (IsSame(&chatText[argPoint], "fresh"))
						age = 0;	//fresh
					if (IsSame(&chatText[argPoint], "rotted"))
						age = 24;   // rotted
					if (IsSame(&chatText[argPoint], "salted"))
						age = -1;		// salted
					argPoint = NextWord(chatText, &linePoint);
					sscanf(&chatText[argPoint], "%d", &amount);
					if (amount < 1)
						amount = 1;
					Inventory *inv = NULL;

					if (age != -2)
					{
						inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;
						// search for some meat 
						InventoryObject *iObject = (InventoryObject *)inv->objects.First();
						while (iObject)
						{
							if (INVOBJ_MEAT == iObject->type)
							{
								InvMeat * ingre = (InvMeat *)iObject->extra;
								if ((ingre->age < 0) && (age == -1)) // if it's salted and we asked for salted.
								{
									if (iObject->amount < amount)
									{
										MessInfoText infoText;
										CopyStringSafely("Not Enough meat in the first stack to feed.", 200, infoText.text, MESSINFOTEXTLEN);
										lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										return;
									}
									else
									{
										foundmeat = TRUE; // we found the right meat
										// we shrink the monster a bit.
										MessInfoText infoText;
										curAvatar->controlledMonster->sizeCoeff = curAvatar->controlledMonster->sizeCoeff / (1 + (0.01f*amount));
										CopyStringSafely("Monster Shrunk.", 200, infoText.text, MESSINFOTEXTLEN);
										lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										iObject->amount = iObject->amount - amount;
										if (iObject->amount < 1)
										{
											// delete the item
											inv->objects.Remove(iObject);
											delete iObject;
										}
									}
								}
								else if ((ingre->age < 24) && (ingre->age > -1) && (age == 0)) // if it's fresh and we asked for fresh.
								{
									if (iObject->amount < amount)
									{
										MessInfoText infoText;
										CopyStringSafely("Not Enough meat in the first stack to feed.", 200, infoText.text, MESSINFOTEXTLEN);
										lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										return;
									}
									else
									{
										foundmeat = TRUE; // we found the right meat
										// we grow the monster a bit. UNLESS it's A zombie.  then we shrink it.
										MessInfoText infoText;
										if (curAvatar->controlledMonster->type == 25)
										{
											curAvatar->controlledMonster->sizeCoeff = curAvatar->controlledMonster->sizeCoeff / (1 + (0.01f*amount));
											CopyStringSafely("Monster Shrunk.", 200, infoText.text, MESSINFOTEXTLEN);
										}
										else
										{

											curAvatar->controlledMonster->sizeCoeff = curAvatar->controlledMonster->sizeCoeff * (1 + (0.01f*amount));
											CopyStringSafely("Monster Enlarged.", 200, infoText.text, MESSINFOTEXTLEN);
										}
										lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										iObject->amount = iObject->amount - amount;
										if (iObject->amount < 1)
										{
											// delete the item
											inv->objects.Remove(iObject);
											delete iObject;
										}
									}
								}
								else if ((ingre->age > 23) && (age == 24)) // if it's rotted and we asked for rotted.
								{
									if (iObject->amount < amount)
									{
										MessInfoText infoText;
										CopyStringSafely("Not Enough meat in the first stack to feed.", 200, infoText.text, MESSINFOTEXTLEN);
										lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										return;
									}
									else
									{
										foundmeat = TRUE; // we found the right meat
										// we shrink the monster a bit. UNLESS it's A zombie.  then we grow it.
										MessInfoText infoText;
										if (curAvatar->controlledMonster->type == 25)
										{

											curAvatar->controlledMonster->sizeCoeff = curAvatar->controlledMonster->sizeCoeff * (1 + (0.01f*amount));
											CopyStringSafely("Monster Enlarged.", 200, infoText.text, MESSINFOTEXTLEN);
										}
										else
										{
											curAvatar->controlledMonster->sizeCoeff = curAvatar->controlledMonster->sizeCoeff / (1 + (0.01f*amount));
											CopyStringSafely("Monster Shrunk.", 200, infoText.text, MESSINFOTEXTLEN);
										}
										lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										iObject->amount = iObject->amount - amount;
										if (iObject->amount < 1)
										{
											// delete the item
											inv->objects.Remove(iObject);
											delete iObject;
										}
									}
								}
								if (foundmeat)
								{
									// announce the monster to register the size change
									curAvatar->controlledMonster->AnnounceMyselfCustom(ss);
									// and return
									return;
								}
							}
							// wrong meat age, or not a dust,  go to the next object.  

							iObject = (InventoryObject *)inv->objects.Next();
						}

					}
				}
			}
			}
			else if (IsSame(&(chatText[argPoint]), "/freepet"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (curAvatar->controlledMonster)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/freepet <yes>  : removes your current pet so it can be replaced by a default based on Taming skill.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
					// get next word 
						if (!IsSame(&chatText[argPoint], "yes")) // if it's not yes
						{
							MessInfoText infoText;
							CopyStringSafely("Please type '/freepet yes' if you are sure",
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
						else
						{
							// kill pet
							curAvatar->controlledMonster->dropAmount = 0;	// no loot
							curAvatar->controlledMonster->type = 25;		// vamp me
							curAvatar->controlledMonster->health = 0;		// no health
							curAvatar->controlledMonster->isDead = TRUE;    // am dead
							curAvatar->controlledMonster = NULL;			//unlink it from me
							// clear out the variables
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_a =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_r =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_g =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_b = 0;
							sprintf(curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_uniqueName, "NO MONSTER");
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_type =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_subType =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_maxHealth =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_health =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_damageDone =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_defense =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_toHit =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_healAmountPerSecond =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_magicResistance =
								curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_sizeCoeff = 0;
							// and save
							curAvatar->SaveAccount();
						}
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/stablepet"))
			{
				if (curAvatar->controlledMonster)
				{
					// must be in town to manually stash
					// inside a town?
					bool isInTown = FALSE;
					if (ss == (SharedSpace *)spaceList->First()) // need to be in the first space to be in a town
					{
						for (int i = 0; i < NUM_OF_TOWNS; ++i)
						{
							if (abs(townList[i].x - curAvatar->cellX) <= 3 &&
								abs(townList[i].y - curAvatar->cellY) <= 3)
							{
								isInTown = TRUE;
							}
						}
					}
					if (!isInTown)
					{
						MessInfoText infoText;
						CopyStringSafely("You must be in town to manually stable a pet.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						// we can stable
						// get existing pet's name
						// get name, skipping player name and whitespace
						sscanf(curAvatar->controlledMonster->uniqueName, "%*[^\']\'s %[^\n]", tempText); // skip past 's , then print rest of the name into tempText
						// place desired stabled item name into tempText2
						sprintf(tempText2, "Stabled %s", tempText);

						InventoryObject * iObject = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.First();
						bool found = FALSE;
						while (iObject)
						{
							if (iObject->type == INVOBJ_STABLED_PET) // if we've found a stabled pet
							{
								// compare with found item
								if (!strcmp(tempText2, iObject->do_name))  // same name
								{
									// fetch extra
									InvStabledPet * iStabledPet = (InvStabledPet *)iObject->extra;
									// check it's type and subtype
									if ((iStabledPet->mtype == curAvatar->controlledMonster->type) && (iStabledPet->subType == curAvatar->controlledMonster->subType))  // same type and subtype
									{											
										// compare defense stats
										if (iStabledPet->defense < curAvatar->controlledMonster->defense) // if stabled pet's defense is lower
										{
											// then update it with the current
											iStabledPet->a = curAvatar->controlledMonster->a;
											iStabledPet->r = curAvatar->controlledMonster->r;
											iStabledPet->g = curAvatar->controlledMonster->g;
											iStabledPet->b = curAvatar->controlledMonster->b;
											iStabledPet->damageDone = curAvatar->controlledMonster->damageDone;
											iStabledPet->defense = curAvatar->controlledMonster->defense;
											iStabledPet->healAmountPerSecond = curAvatar->controlledMonster->healAmountPerSecond;
											iStabledPet->health = curAvatar->controlledMonster->maxHealth;
											iStabledPet->magicResistance = curAvatar->controlledMonster->magicResistance;
											iStabledPet->maxHealth = curAvatar->controlledMonster->maxHealth;
											iStabledPet->sizeCoeff = curAvatar->controlledMonster->sizeCoeff;
											iStabledPet->toHit = curAvatar->controlledMonster->toHit;
										}
										// either way, dispose of the object to exit the while loop;
										found = TRUE;
										iObject = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Last(); // go to the end
										iObject = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next(); // and past, just in case this item was already the last.
									}
									else
									{
										// it's a different type of pet, go to the next one
										iObject = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next();
									}
								}
								else
								{
									// it's a different type of pet, go to the next one
									iObject = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next();
								}
							}
							else
							{
								// it's a renamed pet, next item instead
								iObject = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Next();
							}
						}
						if (!found) // if we didn't find an item for my pet
						{
							// then we need to make a new one
							iObject = new InventoryObject(INVOBJ_STABLED_PET, 0, tempText2); // was set above to "Stabled petname"
							InvStabledPet * exSp = (InvStabledPet *)iObject->extra;

							iObject->mass = 0.0f;
							iObject->value = 1000;
							iObject->amount = 1;
							// copy the stats into it
							exSp->a = curAvatar->controlledMonster->a;
							exSp->b = curAvatar->controlledMonster->b;
							exSp->g = curAvatar->controlledMonster->g;
							exSp->r = curAvatar->controlledMonster->r;
							exSp->damageDone = curAvatar->controlledMonster->damageDone;
							exSp->defense = curAvatar->controlledMonster->defense;
							exSp->healAmountPerSecond = curAvatar->controlledMonster->healAmountPerSecond;
							exSp->health = curAvatar->controlledMonster->maxHealth;
							exSp->magicResistance = curAvatar->controlledMonster->magicResistance;
							exSp->maxHealth = curAvatar->controlledMonster->maxHealth;
							exSp->sizeCoeff = curAvatar->controlledMonster->sizeCoeff;
							exSp->subType = curAvatar->controlledMonster->subType;
							exSp->toHit = curAvatar->controlledMonster->toHit;
							exSp->mtype = curAvatar->controlledMonster->type;

							// and add it to the inventory list
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->objects.Prepend(iObject);
						}
						// kill pet
						curAvatar->controlledMonster->dropAmount = 0;	// no loot
						curAvatar->controlledMonster->type = 25;		// vamp me
						curAvatar->controlledMonster->health = 0;		// no health
						curAvatar->controlledMonster->isDead = TRUE;    // am dead
						curAvatar->controlledMonster = NULL;			//unlink it from me
						// clear out the variables
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_a =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_r =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_g =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_b = 0;
						sprintf(curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_uniqueName, "NO MONSTER");
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_type =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_subType =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_maxHealth =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_health =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_damageDone =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_defense =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_toHit =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_healAmountPerSecond =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_magicResistance =
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].Monster_sizeCoeff = 0;
						// and save
						curAvatar->SaveAccount();

					}
				}
			}

			
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/totemset"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_PLAYER != curAvatar->accountType)
				{
					int type = 18;
					int amount = 1;

					Inventory *inv = NULL;
					argPoint = NextWord(chatText, &linePoint);
					sscanf(&chatText[argPoint], "%d", &amount);
					if (amount < 1)
						amount = 1;

					if (type != -1)
					{

						if (curAvatar->controlledMonster)
						{
							SharedSpace *sx;

							BBOSMonster * theMonster = FindMonster(
								curAvatar->controlledMonster, &sx);
							if (theMonster) 
							{
								if (curAvatar->accountType == ACCOUNT_TYPE_ADMIN) // admins access the monster's inventory, beastmasters do not.
								inv = (theMonster->inventory);
							}
						}
						if (!inv)
							inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

						// add accuracy totem and imbue it.
						InventoryObject *iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						InvTotem *extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 0;
						extra->imbue[MAGIC_WOLF] = 0;
						extra->imbue[MAGIC_EAGLE] = 2;
						extra->imbue[MAGIC_SUN] = 1;
						extra->imbue[MAGIC_SNAKE] = 0;
						extra->imbue[MAGIC_FROG] = 0;
						extra->imbue[MAGIC_MOON] = 0;
						extra->imbue[MAGIC_TURTLE] = 0;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add health totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 0;
						extra->imbue[MAGIC_WOLF] = 1;
						extra->imbue[MAGIC_EAGLE] = 0;
						extra->imbue[MAGIC_SUN] = 0;
						extra->imbue[MAGIC_SNAKE] = 0;
						extra->imbue[MAGIC_FROG] = 2;
						extra->imbue[MAGIC_MOON] = 0;
						extra->imbue[MAGIC_TURTLE] = 1;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add quickness totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 0;
						extra->imbue[MAGIC_WOLF] = 0;
						extra->imbue[MAGIC_EAGLE] = 1;
						extra->imbue[MAGIC_SUN] = 0;
						extra->imbue[MAGIC_SNAKE] = 1;
						extra->imbue[MAGIC_FROG] = 0;
						extra->imbue[MAGIC_MOON] = 1;
						extra->imbue[MAGIC_TURTLE] = 0;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add strength totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 2;
						extra->imbue[MAGIC_WOLF] = 0;
						extra->imbue[MAGIC_EAGLE] = 0;
						extra->imbue[MAGIC_SUN] = 0;
						extra->imbue[MAGIC_SNAKE] = 0;
						extra->imbue[MAGIC_FROG] = 0;
						extra->imbue[MAGIC_MOON] = 0;
						extra->imbue[MAGIC_TURTLE] = 1;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add spirit protection totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 0;
						extra->imbue[MAGIC_WOLF] = 1;
						extra->imbue[MAGIC_EAGLE] = 0;
						extra->imbue[MAGIC_SUN] = 0;
						extra->imbue[MAGIC_SNAKE] = 1;
						extra->imbue[MAGIC_FROG] = 1;
						extra->imbue[MAGIC_MOON] = 2;
						extra->imbue[MAGIC_TURTLE] = 0;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add toughness totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 1;
						extra->imbue[MAGIC_WOLF] = 0;
						extra->imbue[MAGIC_EAGLE] = 0;
						extra->imbue[MAGIC_SUN] = 2;
						extra->imbue[MAGIC_SNAKE] = 0;
						extra->imbue[MAGIC_FROG] = 0;
						extra->imbue[MAGIC_MOON] = 0;
						extra->imbue[MAGIC_TURTLE] = 0;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add web protection totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 1;
						extra->imbue[MAGIC_WOLF] = 2;
						extra->imbue[MAGIC_EAGLE] = 1;
						extra->imbue[MAGIC_SUN] = 0;
						extra->imbue[MAGIC_SNAKE] = 0;
						extra->imbue[MAGIC_FROG] = 1;
						extra->imbue[MAGIC_MOON] = 0;
						extra->imbue[MAGIC_TURTLE] = 2;
						extra->imbue[MAGIC_EVIL] = 0;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						// add lifesteal totem and imbue it.
						iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
						extra = (InvTotem *)iObject->extra;
						extra->type = 0;
						extra->quality = type; // pumpkin
						extra->imbue[MAGIC_BEAR] = 0;
						extra->imbue[MAGIC_WOLF] = 1;
						extra->imbue[MAGIC_EAGLE] = 0;
						extra->imbue[MAGIC_SUN] = 0;
						extra->imbue[MAGIC_SNAKE] = 2;
						extra->imbue[MAGIC_FROG] = 1;
						extra->imbue[MAGIC_MOON] = 1;
						extra->imbue[MAGIC_TURTLE] = 0;
						extra->imbue[MAGIC_EVIL] = 2;
						iObject->mass = 0.0f;
						iObject->value = extra->quality * extra->quality * 14 + 1;
						if (extra->quality > 12)
							iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
						iObject->amount = amount;
						UpdateTotem(iObject);
						inv->AddItemSorted(iObject);
						amount = 8;
						TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
						// log what and who did it
						source = fopen("TotemSetGeneration.txt", "a");
						/* Display operating system-style date and time. */
						_strdate(tempText2);
						fprintf(source, "%s, ", tempText2);
						_strtime(tempText2);
						fprintf(source, "%s, ", tempText2);
						fprintf(source, "%s, ", curAvatar->name);
						fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
						fprintf(source, "%d, ", amount);
						fprintf(source, "%s Totem\n", totemQualityName[extra->quality]);
						fclose(source);
						CopyStringSafely("Undead totem set created.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

					}
				}
			}

			else if (IsSame(&(chatText[argPoint]), "/totem")) {
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/totem <quality> <quantity>",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else {
						int type = -1;
						int amount = -1;
						sscanf(&chatText[argPoint], "%d", &type);
						type = Bracket(type, 0, 25);

						Inventory *inv = NULL;
						argPoint = NextWord(chatText, &linePoint);
						sscanf(&chatText[argPoint], "%d", &amount);
						if (amount < 1)
							amount = 1;

						if (type != -1)
						{

							if (curAvatar->controlledMonster)
							{
								SharedSpace *sx;

								BBOSMonster * theMonster = FindMonster(
									curAvatar->controlledMonster, &sx);
								if (theMonster)
									inv = (theMonster->inventory);
							}
							if (!inv)
								inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

							// add totem
							InventoryObject *iObject = new InventoryObject(INVOBJ_TOTEM, 0, "Unnamed Totem");
							InvTotem *extra = (InvTotem *)iObject->extra;
							extra->type = 0;
							extra->quality = type; // pumpkin

							iObject->mass = 0.0f;
							iObject->value = extra->quality * extra->quality * 14 + 1;
							if (extra->quality > 12)
								iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality - 12) * 1600;
							iObject->amount = amount;
							UpdateTotem(iObject);
							inv->AddItemSorted(iObject);

							TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
							// log what and who did it
							source = fopen("adminTotemGeneration.txt", "a");
							/* Display operating system-style date and time. */
							_strdate(tempText2);
							fprintf(source, "%s, ", tempText2);
							_strtime(tempText2);
							fprintf(source, "%s, ", tempText2);
							fprintf(source, "%s, ", curAvatar->name);
							fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							fprintf(source, "%d, ", amount);
							fprintf(source, "%s Totem\n", totemQualityName[extra->quality]);
							fclose(source);

						}
					}
				}
			}
			else if (IsSame(&(chatText[argPoint]), "/staff")) {
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/staff <quality> <quantity> (5 is elm)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else {
						int type = -1;
						int amount = -1;
						sscanf(&chatText[argPoint], "%d", &type);
						type = Bracket(type, 0, 5);

						Inventory *inv = NULL;
						argPoint = NextWord(chatText, &linePoint);
						sscanf(&chatText[argPoint], "%d", &amount);
						if (amount < 1)
							amount = 1;

						if (type != -1)
						{

							if (curAvatar->controlledMonster)
							{
								SharedSpace *sx;

								BBOSMonster * theMonster = FindMonster(
									curAvatar->controlledMonster, &sx);
								if (theMonster)
									inv = (theMonster->inventory);
							}
							if (!inv)
								inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

							// add staff
							InventoryObject *iObject = new InventoryObject(INVOBJ_STAFF, 0, "Unnamed Staff");
							InvTotem *extra = (InvTotem *)iObject->extra;
							extra->type = 0;
							extra->quality = type;

							iObject->mass = 0.0f;
							iObject->value = 500 * (type + 1) * (type + 1);
							iObject->amount = amount;
							UpdateStaff(iObject, 0);
							inv->AddItemSorted(iObject);

							TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
							// log what and who did it
							source = fopen("adminStaffGeneration.txt", "a");
							/* Display operating system-style date and time. */
							_strdate(tempText2);
							fprintf(source, "%s, ", tempText2);
							_strtime(tempText2);
							fprintf(source, "%s, ", tempText2);
							fprintf(source, "%s, ", curAvatar->name);
							fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							fprintf(source, "%d, ", amount);
							fprintf(source, "%s Staff\n", staffQualityName[extra->quality]);
							fclose(source);

						}
					}
				}
			}
			else if (IsSame(&(chatText[argPoint]), "/ingot")) {
			argPoint = NextWord(chatText, &linePoint);
			if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
			{
				if (argPoint == linePoint)
				{
					MessInfoText infoText;
					CopyStringSafely("/ingot <quality> <quantity> (14 is chrome)",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else {
					int type = -1;
					int amount = -1;
					sscanf(&chatText[argPoint], "%d", &type);
					type = Bracket(type, 0, 14);

					Inventory *inv = NULL;
					argPoint = NextWord(chatText, &linePoint);
					sscanf(&chatText[argPoint], "%d", &amount);
					if (amount < 1)
						amount = 1;

					if (type != -1)
					{

						if (curAvatar->controlledMonster)
						{
							SharedSpace *sx;

							BBOSMonster * theMonster = FindMonster(
								curAvatar->controlledMonster, &sx);
							if (theMonster)
								inv = (theMonster->inventory);
						}
						if (!inv)
							inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

						// add ingot
						sprintf(tempText, "%s Ingot", ingotNameList[type]);
						InventoryObject *iObject = new InventoryObject(INVOBJ_INGOT, 0, tempText);
						InvIngot *extra = (InvIngot *)iObject->extra;
						extra->damageVal = type + 1;
						extra->challenge = type + 1;
						extra->r = ingotRGBList[type][0];
						extra->g = ingotRGBList[type][1];
						extra->b = ingotRGBList[type][2];


						iObject->mass = 1.0f;
						iObject->value = ingotValueList[type];
						iObject->amount = amount;
						inv->AddItemSorted(iObject);

						TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
						// log what and who did it
						source = fopen("adminIngotGeneration.txt", "a");
						/* Display operating system-style date and time. */
						_strdate(tempText2);
						fprintf(source, "%s, ", tempText2);
						_strtime(tempText2);
						fprintf(source, "%s, ", tempText2);
						fprintf(source, "%s, ", curAvatar->name);
						fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
						fprintf(source, "%d, ", amount);
						fprintf(source, "%s Ingot\n", ingotNameList[type]);
						fclose(source);

					}
				}
			}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/xpmultiplier"))
			{
			argPoint = NextWord(chatText, &linePoint);
			if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
			{
				if (argPoint == linePoint)
				{
					MessInfoText infoText;
					CopyStringSafely("/xpmultiplier <integer> (default is 1, affects ALL SKILL EXP)",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					int multiplier = 1;
					sscanf(&chatText[argPoint], "%d", &multiplier);
					multiplier = Bracket(multiplier, 1, 10);
					// set all mltiplier
					combat_exp_multiplier = multiplier;
					smith_exp_multiplier = multiplier;
					bombing_exp_multiplier = multiplier;
					geomancy_exp_multiplier = multiplier;
					ts_exp_multiplier = multiplier;
					magic_exp_multiplier = multiplier;
					mastery_exp_multiplier = multiplier;
					MessInfoText infoText;
					CopyStringSafely("All experience multipliers adjusted.",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
			}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/dodgemultiplier"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/dodgemultiplier <integer> (default is 1, affects dodging and pet energy)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set combat multiplier
						combat_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Combat experience multipliers adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/smithmultiplier"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/smithmultiplier <integer> (default is 1, affects swordsmith, expertises, and dismantle)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set smith multiplier
						smith_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Smithing experience multipliers adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/bombmultiplier"))
			{
			argPoint = NextWord(chatText, &linePoint);
			if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
			{
				if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/bombmultiplier <integer> (default is 1, affects everything related to explosives except mastery)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set bombing multiplier
						bombing_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Explosives experience multiplier adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/geomultiplier"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/geomultiplier <integer> (default is 1, affects geomancy experience)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set geomancy mltiplier
						geomancy_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Geomancy experience multiplier adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/tsmultiplier"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/tsmultiplier <integer> (default is 1)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set ts mltiplier
						ts_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Totem Shatter experience multipliers adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
				}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/magicmultiplier"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/magicmultiplier <integer> (default is 1, affects totem and staff imbues)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set magic mltiplier
						magic_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Magic experience multiplier adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/masterymultiplier"))
			{
				argPoint = NextWord(chatText, &linePoint);
				if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
				{
					if (argPoint == linePoint)
					{
						MessInfoText infoText;
						CopyStringSafely("/masterymultiplier <integer> (default is 1, affects all mastery skills)",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						int multiplier = 1;
						sscanf(&chatText[argPoint], "%d", &multiplier);
						multiplier = Bracket(multiplier, 1, 10);
						// set mastery mltiplier
						mastery_exp_multiplier = multiplier;
						MessInfoText infoText;
						CopyStringSafely("Mastery experience multipliers adjusted.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if ( IsSame(&(chatText[argPoint]) , "/shard"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                {
                    if (argPoint == linePoint)
                    {
                        MessInfoText infoText;
                        CopyStringSafely("/shard <0-8>", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        int type = -1;
                        sscanf(&chatText[argPoint],"%d",&type);
                        type = Bracket(type,0,8);

                        Inventory *inv = NULL;

                        if (type != -1)
                        {
                            type += INGR_WHITE_SHARD;

                            if (curAvatar->controlledMonster)
                            {
                                SharedSpace *sx;

                                BBOSMonster * theMonster = FindMonster(
                                          curAvatar->controlledMonster, &sx);
                                if (theMonster)
                                    inv = (theMonster->inventory);
                            }
                            if (!inv)
                                    inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;

                            InventoryObject *iObject = new InventoryObject(
                                        INVOBJ_INGREDIENT,0,dustNames[type]);
                            InvIngredient *exIn = (InvIngredient *)iObject->extra;
                            exIn->type     = type;
                            exIn->quality  = 1;

                            iObject->mass = 0.0f;
                            iObject->value = 1000;
                            iObject->amount = 1;
                            inv->AddItemSorted(iObject);

                            TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
							source = fopen("adminDustGeneration.txt", "a");
							/* Display operating system-style date and time. */
							_strdate(tempText2);
							fprintf(source, "%s, ", tempText2);
							_strtime(tempText2);
							fprintf(source, "%s, ", tempText2);
							fprintf(source, "%s, ", curAvatar->name);
							fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							fprintf(source, "%d, ", 1);
							fprintf(source, "%s\n", dustNames[type]);
							fclose(source);
						}
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/testdungeon") && 
                       ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                argPoint = NextWord(chatText,&linePoint);

                // make the new space
                DungeonMap *dm = new DungeonMap(SPACE_DUNGEON,"Test Dungeon",lserver);
                dm->specialFlags = SPECIAL_DUNGEON_TEMPORARY;
                dm->InitNew(5,5,curAvatar->cellX, curAvatar->cellY, 0);
                sprintf_s(dm->name, "%s's %s %s", 
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                  rdNameAdjective[rand() % 6], rdNamePre [rand() % 7]);

                spaceList->Append(dm);
/*
                dm->topWall [dm->width+0] = 1;
                dm->leftWall[dm->width+0] = 0;
                dm->topWall [dm->width+1] = 1;
                dm->leftWall[dm->width+1] = 0;
                dm->topWall [dm->width+2] = 1;
                dm->leftWall[dm->width+2] = 0;
                dm->topWall [dm->width+3] = 0;
                dm->leftWall[dm->width+3] = 0;
  */
                int col = 0;

                // add static monsters
                for (int m = 0; m < (dm->height * dm->width) / 12;)
                {
                    int t  = rand() % NUM_OF_MONSTERS;
                    int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

                    if (monsterData[t][t2].name[0] && monsterData[t][t2].dungeonType >= col - 3 &&
                         monsterData[t][t2].dungeonType <= col)
                    {
                        int mx, my;
                        do
                        {
                            mx = rand() % (dm->width);
                            my = rand() % (dm->height);
                        } while (mx < 2 && my < 2);

                        BBOSMonster *monster = new BBOSMonster(t,t2, NULL);
                        monster->cellX = mx;
                        monster->cellY = my;
                        monster->targetCellX = mx;
                        monster->targetCellY = my;
                        monster->spawnX = mx;
                        monster->spawnY = my;
                        dm->mobList->Add(monster);
                        ++m;
                    }
                }
                // add wandering monsters
                if (0 == col)
                    col = 1;

                for (int m = 0; m < (dm->height * dm->width) / 12;)
                {
                    int t, t2;
                    int done = FALSE;
                    while (!done)
                    {
                        t  = rand() % NUM_OF_MONSTERS;
                        t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

                        if (MONSTER_PLACE_DUNGEON & monsterData[t][t2].placementFlags)
                            done = TRUE;
                    }

                    if (monsterData[t][t2].name[0] && monsterData[t][t2].dungeonType >= col - 3 &&
                         monsterData[t][t2].dungeonType <= col - 1)
                    {
                        int mx, my;
                        do
                        {
                            mx = rand() % (dm->width);
                            my = rand() % (dm->height);
                        } while (mx < 4 && my < 4);

                        BBOSMonster *monster = new BBOSMonster(t,t2, NULL);
                        monster->isWandering = TRUE;
                        monster->cellX = mx;
                        monster->cellY = my;
                        monster->targetCellX = mx;
                        monster->targetCellY = my;
                        monster->spawnX = mx;
                        monster->spawnY = my;
                        dm->mobList->Add(monster);
                        ++m;
                    }
                }

                // add a portal out of it
                BBOSWarpPoint *wp = new BBOSWarpPoint(0,0);
                wp->targetX      = dm->enterX;
                wp->targetY      = dm->enterY-1;
                wp->spaceType    = SPACE_GROUND;
                wp->spaceSubType = 0;
                dm->mobList->Add(wp);

                // teleport inside

                // tell everyone I'm dissappearing
                curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);

                // tell my client I'm entering the dungeon
                MessChangeMap changeMap;
                changeMap.dungeonID = (long) dm;
                changeMap.oldType = ss->WhatAmI();
                changeMap.newType = dm->WhatAmI();
                changeMap.sizeX   = ((DungeonMap *) dm)->width;
                changeMap.sizeY   = ((DungeonMap *) dm)->height;
                changeMap.flags   = MESS_CHANGE_TEMP;

                lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                MessInfoText infoText;
                sprintf_s(tempText,"You enter the %s.", ((DungeonMap *) dm)->name);
                memcpy(infoText.text, tempText, MESSINFOTEXTLEN-1);
                infoText.text[MESSINFOTEXTLEN-1] = 0;
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                curAvatar->QuestSpaceChange(ss,dm);

                // move me to my new SharedSpace
                ss->avatars->Remove(curAvatar);
                dm->avatars->Append(curAvatar);

                curAvatar->cellX = ((DungeonMap *) dm)->width-1;
                curAvatar->cellY = ((DungeonMap *) dm)->height-1;

                // tell everyone about my arrival
                curAvatar->IntroduceMyself(dm, SPECIAL_APP_DUNGEON);

                // tell this player about everyone else around
                curAvatar->UpdateClient(dm, TRUE);

            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/birth"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                {
                    int type, subType, resist;
                    float change;
                    Inventory *inv = NULL;

                    if (argPoint == linePoint)
                    {
                        MessInfoText infoText;
                        CopyStringSafely("/birth <type num> <subType num> <power multiplier> <resist 0-100>", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        sscanf(&chatText[argPoint],"%d",&type);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&subType);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%f",&change);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&resist);

                        if (type < 0)
                            type = 0;
                        if (type >= NUM_OF_MONSTERS)
                            type = NUM_OF_MONSTERS-1;
                        if (subType < 0)
                            subType = 0;
                        if (subType >= NUM_OF_MONSTER_SUBTYPES)
                            subType = NUM_OF_MONSTER_SUBTYPES-1;

                        if (change < 0.1f)
                            change = 1;
                        if (change > 100)
                            change = 1;

                        if (resist < 0)
                            resist = 0;
                        if (resist > 100)
                            resist = 100;

                        if (monsterData[type][subType].name[0])
                        {
                            SharedSpace *sx;
                            BBOSMonster * theMonster = NULL;
                            int tX, tY;
                            tX = curAvatar->cellX;
                            tY = curAvatar->cellY;
                            if (curAvatar->controlledMonster)
                            {
                                theMonster = FindMonster(curAvatar->controlledMonster, &sx);
                                if (theMonster)
                                {
                                    tX = theMonster->cellX;
                                    tY = theMonster->cellY;
                                }
                            }

                            if (!theMonster)
                                sx = ss;

                            BBOSMonster *monster = new BBOSMonster(type, subType, NULL);
                            monster->dontRespawn = TRUE;
                            monster->cellX       = tX;
                            monster->cellY       = tY;
                            monster->targetCellX = tX;
                            monster->targetCellY = tY;
                            monster->spawnX      = tX;
                            monster->spawnY      = tY;
                            ss->mobList->Add(monster);

                            monster->damageDone = monster->damageDone * change;
                            monster->defense    = monster->defense    * change;
                            monster->dropAmount = monster->dropAmount * change;
                            monster->health     = monster->health     * change;
                            monster->maxHealth  = monster->maxHealth  * change;
                            monster->toHit      = monster->toHit      * change;

                            monster->magicResistance = resist / 100.0f;

                            monster->isMoving = TRUE;
                            monster->moveStartTime = timeGetTime() - 10000;
							monster->tamingcounter = 1000;						// monsters summoned by admin can't be tames
                            MessMobAppear mobAppear;
                            mobAppear.mobID = (unsigned long) monster;
                            mobAppear.type = monster->WhatAmI();
                            mobAppear.monsterType = monster->type;
                            mobAppear.subType = monster->subType;
                            if(SPACE_DUNGEON == sx->WhatAmI())
                            {
                                mobAppear.staticMonsterFlag = FALSE;
                            }

                            mobAppear.x = monster->cellX;
                            mobAppear.y = monster->cellY;
                            sx->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
                                         sizeof(mobAppear), &mobAppear);

                            MessGenericEffect messGE;
                            messGE.avatarID = -1;
                            messGE.mobID    = -1;
                            messGE.x        = monster->cellX;
                            messGE.y        = monster->cellY;
                            messGE.r        = 255;
                            messGE.g        = 0;
                            messGE.b        = 255;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 1; // in seconds
                            sx->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
                                                 sizeof(messGE),(void *)&messGE);
							//log it
							source = fopen("adminBirth.txt", "a");
							/* Display operating system-style date and time. */
							_strdate(tempText2);
							fprintf(source, "%s, ", tempText2);
							_strtime(tempText2);
							fprintf(source, "%s, ", tempText2);
							fprintf(source, "%s, ", curAvatar->name);
							fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							fprintf(source, "%s\n", chatText);
							fclose(source);
							if (type == 31)
								monster->SpecialAI = TRUE;


                        }
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/modbirth"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                     ACCOUNT_TYPE_MODERATOR == curAvatar->accountType)
                {
                    int type, subType, resist;
                    float change;
                    Inventory *inv = NULL;

                    if (argPoint == linePoint)
                    {
                        MessInfoText infoText;
                        CopyStringSafely("/modbirth <type num> <subType num> <power multiplier> <resist 0-100>", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        sscanf(&chatText[argPoint],"%d",&type);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&subType);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%f",&change);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&resist);

                        if (type < 0)
                            type = 0;
                        if (type >= NUM_OF_MONSTERS)
                            type = NUM_OF_MONSTERS-1;
                        if (subType < 0)
                            subType = 0;
                        if (subType >= NUM_OF_MONSTER_SUBTYPES)
                            subType = NUM_OF_MONSTER_SUBTYPES-1;

                        if (change < 0.1f)
                            change = 1;
                        if (change > 100)
                            change = 1;

                        if (resist < 0)
                            resist = 0;
                        if (resist > 100)
                            resist = 100;

                        if (monsterData[type][subType].name[0])
                        {
                            SharedSpace *sx;
                            BBOSMonster * theMonster = NULL;
                            int tX, tY;
                            tX = curAvatar->cellX;
                            tY = curAvatar->cellY;
                            if (curAvatar->controlledMonster)
                            {
                                theMonster = FindMonster(curAvatar->controlledMonster, &sx);
                                if (theMonster)
                                {
                                    tX = theMonster->cellX;
                                    tY = theMonster->cellY;
                                }
                            }

                            if (!theMonster)
                                sx = ss;

                            BBOSMonster *monster = new BBOSMonster(type, subType, NULL);
                            monster->dontRespawn = TRUE;
                            monster->cellX       = tX;
                            monster->cellY       = tY;
                            monster->targetCellX = tX;
                            monster->targetCellY = tY;
                            monster->spawnX      = tX;
                            monster->spawnY      = tY;
                            ss->mobList->Add(monster);

                            monster->damageDone = monster->damageDone * change;
                            monster->defense    = monster->defense    * change;
                            monster->dropAmount = monster->dropAmount * change;
                            monster->health     = monster->health     * change;
                            monster->maxHealth  = monster->maxHealth  * change;
                            monster->toHit      = monster->toHit      * change;

                            monster->magicResistance = resist / 100.0f;
							monster->tamingcounter = 1000;						// monsters summoned by admin can't be tamed

                            monster->isMoving = TRUE;
                            monster->moveStartTime = timeGetTime() - 10000;

                            MessMobAppear mobAppear;
                            mobAppear.mobID = (unsigned long) monster;
                            mobAppear.type = monster->WhatAmI();
                            mobAppear.monsterType = monster->type;
                            mobAppear.subType = monster->subType;
                            if(SPACE_DUNGEON == sx->WhatAmI())
                            {
                                mobAppear.staticMonsterFlag = FALSE;
                            }

                            mobAppear.x = monster->cellX;
                            mobAppear.y = monster->cellY;
                            sx->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
                                         sizeof(mobAppear), &mobAppear);

                            MessGenericEffect messGE;
                            messGE.avatarID = -1;
                            messGE.mobID    = -1;
                            messGE.x        = monster->cellX;
                            messGE.y        = monster->cellY;
                            messGE.r        = 255;
                            messGE.g        = 0;
                            messGE.b        = 255;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 1; // in seconds
                            sx->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
                                                 sizeof(messGE),(void *)&messGE);
							source = fopen("modbirth.txt", "a");
							/* Display operating system-style date and time. */
							_strdate(tempText2);
							fprintf(source, "%s, ", tempText2);
							_strtime(tempText2);
							fprintf(source, "%s, ", tempText2);
							fprintf(source, "%s, ", curAvatar->name);
							fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
							fprintf(source, "%s\n", chatText);
							fclose(source);


                        }
                        else
                        {
                            MessInfoText infoText;
                            sprintf_s(infoText.text,"That monster (%d,%d) doesn't exist", 
                                                  type, subType);
                            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        }
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/effect"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                     ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                     ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType )
                {
//					int type, amount, r, g, b;
//					float change;
                    Inventory *inv = NULL;

                    if (argPoint == linePoint)
                    {
                        MessInfoText infoText;
                        CopyStringSafely("/effect <type> <intensity> <r> <g> <b>", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        BBOSGroundEffect *bboGE = new BBOSGroundEffect();

                        sscanf(&chatText[argPoint],"%d",&bboGE->type);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&bboGE->amount);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&bboGE->r);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&bboGE->g);
                        argPoint = NextWord(chatText,&linePoint);
                        sscanf(&chatText[argPoint],"%d",&bboGE->b);

                        bboGE->cellX = curAvatar->cellX;
                        bboGE->cellY = curAvatar->cellY;

                        ss->mobList->Add(bboGE);

                        MessGroundEffect messGE;
                        messGE.mobID  = (unsigned long) bboGE;
                        messGE.type   = bboGE->type;
                        messGE.amount = bboGE->amount;
                        messGE.x      = bboGE->cellX;
                        messGE.y      = bboGE->cellY;
                        messGE.r      = bboGE->r;
                        messGE.g      = bboGE->g;
                        messGE.b      = bboGE->b;

                        ss->SendToEveryoneNearBut(0, 
                                   curAvatar->cellX, curAvatar->cellY, 
                               sizeof(messGE), &messGE);

                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/killeffect"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                     ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                     ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType)
                {
                    BBOSMob *curMob2 = (BBOSMob *) ss->mobList->GetFirst(
                                        curAvatar->cellX, curAvatar->cellY);
                    while (curMob2)
                    {
                        if (SMOB_GROUND_EFFECT == curMob2->WhatAmI())
                        {
                            ss->mobList->Remove(curMob2);

                            MessMobDisappear messMobDisappear;
                            messMobDisappear.mobID = (unsigned long) curMob2;
                            messMobDisappear.x = curAvatar->cellX;
                            messMobDisappear.y = curAvatar->cellY;
                            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                sizeof(messMobDisappear),(void *)&messMobDisappear);

                            delete curMob2;
                        }

                        curMob2 = (BBOSMob *) ss->mobList->GetNext();
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/invon"))
            {
                if (ACCOUNT_TYPE_PLAYER != curAvatar->accountType)
                {
                    MessAvatarDisappear aDisappear;
                    aDisappear.avatarID = curAvatar->socketIndex;
                    aDisappear.x = curAvatar->cellX;
                    aDisappear.y = curAvatar->cellY;

                    ss->SendToEveryoneNearBut(curAvatar->socketIndex, 
                                   curAvatar->cellX, curAvatar->cellY, 
                               sizeof(aDisappear), &aDisappear);

                    curAvatar->isInvisible = TRUE;
                    MessInfoText infoText;
                    CopyStringSafely("You are invisible.", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/invoff"))
            {
                if (ACCOUNT_TYPE_PLAYER != curAvatar->accountType)
                {
                    curAvatar->isInvisible = FALSE;
                    MessInfoText infoText;
                    CopyStringSafely("You are NOT invisible any longer.", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
            }
            //***************************************
			else if ((IsSame(&(chatText[argPoint]), "/mystats"))
			&& ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
			{
			int found = FALSE;
			int s1, s2, s3, s4;
			int start = linePoint;

			argPoint = NextWord(chatText, &linePoint);
			s1 = atoi(&(chatText[argPoint]));
			argPoint = NextWord(chatText, &linePoint);
			s2 = atoi(&(chatText[argPoint]));
			argPoint = NextWord(chatText, &linePoint);
			s3 = atoi(&(chatText[argPoint]));
			argPoint = NextWord(chatText, &linePoint);
			s4 = atoi(&(chatText[argPoint]));

			if (s1 <= 0 || s1 > 100 || s2 <= 0 || s2 > 100 || s3 <= 0 || s3 > 100 || s4 <= 0 || s4 > 100)
			{
				sprintf(&chatMess.text[1], "Invalid values: %d %d %d %d", s1, s2, s3, s4);
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
				sprintf(&chatMess.text[1], "USAGE: /mystats <physical> <magical> <creative> <beast>");
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);

				sprintf(&chatMess.text[1], "Your current stats are: %d %d %d %d",
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].physical,
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].magical,
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative,
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast);
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
			}
			else
			{
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].physical = s1;
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].magical = s2;
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative = s3;
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast = s4;

				sprintf(&chatMess.text[1], "New admin stats: %d %d %d %d", s1, s2, s3, s4);
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
			}
			}
			else if ((IsSame(&(chatText[argPoint]), "/mystats")) && (enable_cheating!=0)) // normal players can change stats ni cheater mode
			{
			int found = FALSE;
			int s1, s2, s3, s4;
			int start = linePoint;
			int statlimit = 10;																// but not more than their age allows.
			if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].age > 5)  
				statlimit = 11;
			if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].age > 6) 
				statlimit = 12;
			if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].age > 7) 
				statlimit = 13;

			argPoint = NextWord(chatText, &linePoint);
			s1 = atoi(&(chatText[argPoint]));
			argPoint = NextWord(chatText, &linePoint);
			s2 = atoi(&(chatText[argPoint]));
			argPoint = NextWord(chatText, &linePoint);
			s3 = atoi(&(chatText[argPoint]));
			argPoint = NextWord(chatText, &linePoint);
			s4 = atoi(&(chatText[argPoint]));

			if (s1 <= 0 || s1 > statlimit || s2 <= 0 || s2 > statlimit || s3 <= 0 || s3 > statlimit || s4 <= 0 || s4 > 100)
			{
				sprintf(&chatMess.text[1], "Invalid values: %d %d %d &d", s1, s2, s3,s4);
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
				sprintf(&chatMess.text[1], "USAGE: /mystats <physical> <magical> <creative> <beast>");
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);

				sprintf(&chatMess.text[1], "Your current stats are: %d %d %d %d",
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].physical,
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].magical,
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative,
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast);
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
			}
			else
			{
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].physical = s1;
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].magical = s2;
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative = s3;
				curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast = s4;

				sprintf(&chatMess.text[1], "New cheater stats: %d %d %d %d", s1, s2, s3, s4);
				chatMess.text[0] = TEXT_COLOR_DATA;
				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
			}
			}

            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/myskills"))
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType) // but not skills, that's too silly. :)
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                InventoryObject *skill = NULL;
                BBOSAvatar *tAv = curAvatar;
                tempText[0] = 0;

                while (!found && !done)
                {
                    argPoint = NextWord(chatText,&linePoint);
                    if (argPoint == linePoint)
                        done = TRUE;
                    else
                    {
                        memcpy(tempText,&(chatText[start]), linePoint-start);
                        int back = 1;
                        tempText[linePoint-start] = 0;
                        while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                        {
                            tempText[linePoint-start-back] = 0;
                            ++back;
                        }

                            InventoryObject *io = (InventoryObject *) 
                                tAv->charInfoArray[tAv->curCharacterIndex].skills->objects.First();
                            while (io)
                            {
                                if (!stricmp(tempText,io->WhoAmI()))
                                {
                                    skill = io;
//									skillInfo = (InvSkill *) io->extra;
                                }
                                io = (InventoryObject *) 
                                    tAv->charInfoArray[tAv->curCharacterIndex].skills->objects.Next();
                            }

                        if (skill)
                            found = TRUE;
                    }
                }

                if (found)
                {

                    argPoint = NextWord(chatText,&linePoint);
                    int levelToGive = atoi(&(chatText[argPoint]));
                    if (levelToGive <= 0 || levelToGive > 30000)
                    {
                        sprintf(&chatMess.text[1],"Invalid skill level: %d", levelToGive);
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        sprintf(&chatMess.text[1],"USAGE: /myskills <skill name> <new skill level>");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else
                    {

                        InvSkill *skillInfo = (InvSkill *) skill->extra;
                        skillInfo->skillLevel = levelToGive;

                        sprintf_s(tempText,"skill adjusted to level %d.",levelToGive);
                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"You don't have such a skill (%s).", tempText);
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"USAGE: /myskills <skill name> <new skill level>");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }

            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/gettestskills"))
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                InventoryObject *skill = NULL;
                BBOSAvatar *tAv = curAvatar;
                tempText[0] = 0;

                InventoryObject *io = (InventoryObject *) 
                    tAv->charInfoArray[tAv->curCharacterIndex].skills->objects.First();
                while (io)
                {
                    if (!stricmp("Geomancy",io->WhoAmI()))
                    {
                        skill = io;
                    }
                    io = (InventoryObject *) 
                        tAv->charInfoArray[tAv->curCharacterIndex].skills->objects.Next();
                }

                if (!skill)
                {
                    skill = new InventoryObject(INVOBJ_SKILL,0,"Geomancy");
                    skill->mass = 0.0f;
                    skill->value = 1.0f;
                    skill->amount = 1;
                    tAv->charInfoArray[tAv->curCharacterIndex].skills->AddItemSorted(skill);

                    sprintf(&chatMess.text[1],"New skill added.  Thanks for testing it!");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else
                {
                    sprintf(&chatMess.text[1],"You already have the new skill.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }

                TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);


            }

            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/resetdungeon"))
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                InventoryObject *skill = NULL;
                BBOSAvatar *tAv = curAvatar;
                tempText[0] = 0;

                argPoint = NextWord(chatText,&linePoint);
                int val;
                sscanf(&chatText[argPoint],"%d",&val);

                if (SPACE_DUNGEON == ss->WhatAmI() && 
                     SPECIAL_DUNGEON_MODERATED & ((DungeonMap *)ss)->specialFlags)
                {
                    ((DungeonMap *)ss)->Randomize(Bracket(val,1,100));

                    MessDungeonChange messDungeonChange;
                    messDungeonChange.floor = messDungeonChange.left = 
                        messDungeonChange.outer = messDungeonChange.top = 0;
                    messDungeonChange.reset = 1;
                    messDungeonChange.x = curAvatar->cellX;
                    messDungeonChange.y = curAvatar->cellY;

                    lserver->SendMsg(sizeof(messDungeonChange),(void *)&messDungeonChange, 0, &tempReceiptList);
                }

            }
 
            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/lock"))
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                InventoryObject *skill = NULL;
                BBOSAvatar *tAv = curAvatar;
                tempText[0] = 0;

                if (SPACE_DUNGEON == ss->WhatAmI())
                {
                    ((DungeonMap *)ss)->isLocked = TRUE;
                    sprintf(&chatMess.text[1],"This dungeon is now locked.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
 
            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/unlock"))
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                InventoryObject *skill = NULL;
                BBOSAvatar *tAv = curAvatar;
                tempText[0] = 0;

                if (SPACE_DUNGEON == ss->WhatAmI())
                {
                    ((DungeonMap *)ss)->isLocked = FALSE;
                    sprintf(&chatMess.text[1],"This dungeon is now NOT locked.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
 
            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/killworkbench"))
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;

                InventoryObject *io = (InventoryObject *) 
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.First();
                while (io)
                {
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                    workbench->objects.Remove(io);
                    delete io;

                    io = (InventoryObject *) 
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench->objects.First();
                }

                sprintf(&chatMess.text[1],"All items in workbench destroyed.");
                chatMess.text[0] = TEXT_COLOR_DATA;
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
            }

            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/killmoney"))
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;

                argPoint = NextWord(chatText,&linePoint);
                long moneyToKill = atoi(&(chatText[argPoint]));
                if (moneyToKill <= 0)
                {
                    sprintf(&chatMess.text[1],"Invalid amount of money: %d", moneyToKill);
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else
                {
                    long amount = 
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money;

                    if (moneyToKill > amount)
                        moneyToKill = amount;

                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money -= 
                              moneyToKill;

                    sprintf(&chatMess.text[1],"%d gold destroyed.", moneyToKill);
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }

            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/m") && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                argPoint = NextWord(chatText,&linePoint);
                HandleContMonstString(&chatText[argPoint], curAvatar, ss);
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/helper") && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                int found = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv && targetAv->cellX == curAvatar->cellX &&
                                 targetAv->cellY == curAvatar->cellY)
                {
                    argPoint = linePoint = linePoint + len;

                    if (targetAv->charInfoArray[targetAv->curCharacterIndex].imageFlags &
                         SPECIAL_LOOK_HELPER)
                    {
                        targetAv->charInfoArray[targetAv->curCharacterIndex].imageFlags &= 
                          ~(SPECIAL_LOOK_HELPER);
                        MessAvatarStats mStats;
                        targetAv->BuildStatsMessage(&mStats);
                        ss->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                   sizeof(mStats),(void *)&mStats);

                        sprintf_s(infoText.text,"%s removes the belt of", 
                                 curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                        ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                               sizeof(infoText),(void *)&infoText);

                        sprintf_s(infoText.text,"Tribal Avatar from %s.", 
                                 targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                        ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                               sizeof(infoText),(void *)&infoText);
                    }
                    else
                    {
                        targetAv->charInfoArray[targetAv->curCharacterIndex].imageFlags |= 
                          SPECIAL_LOOK_HELPER;
                        MessAvatarStats mStats;
                        targetAv->BuildStatsMessage(&mStats);
                        ss->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                   sizeof(mStats),(void *)&mStats);

                        sprintf_s(infoText.text,"%s bestows the belt of", 
                                 curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                        ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                               sizeof(infoText),(void *)&infoText);

                        sprintf_s(infoText.text,"Tribal Avatar upon %s.", 
                                 targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                        ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                               sizeof(infoText),(void *)&infoText);
                    }

                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/h") && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                curAvatar->charInfoArray[curAvatar->curCharacterIndex].health =
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;

                MessAvatarHealth messHealth;
                messHealth.health    = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
                messHealth.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
                messHealth.avatarID  = curAvatar->socketIndex;
                ss->lserver->SendMsg(sizeof(messHealth),(void *)&messHealth, 0, &tempReceiptList);

                curAvatar->AnnounceSpecial(ss, SPECIAL_APP_HEAL);

            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/shutdown") && 
                       ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                if (10000 == pleaseKillMe)
                {
                    pleaseKillMe =30;
                    countDownTime.SetToNow();
                }

            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/startvote"))
            {

                argPoint = NextWord(chatText,&linePoint);

                if (argPoint == linePoint)
                {
                    sprintf(&chatMess.text[1],"USAGE: /startvote <PROMOTE, DEMOTE, KICK, CHANGESTYLE, CHANGENAME, FIGHTER, MAGE, or CRAFTER> <subject guild member>.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                if (GUILDSTYLE_NONE != ((TowerMap *)guildSpace)->guildStyle)
                {
                    sscanf(&(chatText[argPoint]),"%s", tempText);

                    argPoint = NextWord(chatText,&linePoint);

                    AttemptToStartVote(curAvatar, ((TowerMap *)guildSpace), &(chatText[argPoint]), tempText);
                }
                else
                {
                    sprintf(&chatMess.text[1],"First use the /guildtype command to set the style of your guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/stopvote"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                if (argPoint == linePoint)
                {
                    MessInfoText infoText;
                    CopyStringSafely("USAGE: /stopvote <bill number (1-4)>", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    CopyStringSafely("You must have sponsored the bill, and voting must still be happening.", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
                else
                {
                    int targX;
                    sscanf(&chatText[argPoint],"%d",&targX);

                    argPoint = NextWord(chatText,&linePoint);

                    if (targX < 1 || targX > 4) 
                    {
                        MessInfoText infoText;
                        CopyStringSafely("USAGE: /stopvote <bill number (1-4)>", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        CopyStringSafely("You must have sponsored the bill, and voting must still be happening.", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        --targX;
                        if ( ((TowerMap *)guildSpace)->bills[targX].type > GUILDBILL_INACTIVE &&
                              VOTESTATE_VOTING == ((TowerMap *)guildSpace)->bills[targX].voteState)
                        {
                            if (IsCompletelySame(
                                   ((TowerMap *)guildSpace)->bills[targX].sponsor,
                                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name
                                ))
                            {
                                ((TowerMap *)guildSpace)->bills[targX].type = GUILDBILL_INACTIVE;

                                CopyStringSafely("Your bill is killed.", 
                                                      200, infoText.text, MESSINFOTEXTLEN);
                                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }
                    }
                }
            }
            //***************************************
			else if (IsSame(&(chatText[argPoint]), "/passchange"))
			{
				argPoint = NextWord(chatText, &linePoint);

				if (argPoint == linePoint)
				{
					MessInfoText infoText;
					CopyStringSafely("USAGE: /passchange <old password> <new password>",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					sscanf(&chatText[argPoint], "%s", tempText);
					// hash the old password.
					unsigned char salt[256];
					sprintf_s((char*)&salt[0], 256, "%s-%s", "BladeMistress", curAvatar->name);
					unsigned char hashPass[HASH_BYTE_SIZE + 1] = { 0 };
					unsigned char hashPass2[HASH_BYTE_SIZE + 1] = { 0 };
					PasswordHash::CreateStandaloneHash((const unsigned char*)tempText, salt, 6969, hashPass);

					unsigned char tempPass[OUT_HASH_SIZE + 1] = { 0 };
					unsigned char tempPass2[OUT_HASH_SIZE + 1] = { 0 };
					PasswordHash::CreateSerializableHash(hashPass, (unsigned char*)&tempPass[0]);

					if (PasswordHash::ValidateSerializablePassword(hashPass, (const unsigned char*)curAvatar->pass))
					{
						argPoint = NextWord(chatText, &linePoint);
						sscanf(&chatText[argPoint], "%s", tempText);
						GuaranteeTermination(tempText, 12);
						CorrectString(tempText);
						char tempTextp[1028] = { 0 };
						strncpy(tempTextp, tempText, sizeof tempTextp - 1);
						PasswordHash::CreateStandaloneHash((const unsigned char*)tempText, salt, 6969, hashPass2);
						PasswordHash::CreateSerializableHash(hashPass2, (unsigned char*)&tempPass2[0]);
						if (strlen(tempText) > 0)
						{
							sprintf_s(curAvatar->pass, (const char*)tempPass2);
							curAvatar->passLen = strlen(curAvatar->pass);

							sprintf(&(tempText[2]), "Your password is changed to %s.",
								tempTextp);
							tempText[0] = NWMESS_PLAYER_CHAT_LINE;
							tempText[1] = TEXT_COLOR_DATA;

							lserver->SendMsg(strlen(tempText) + 1, (void *)&tempText, 0, &tempReceiptList);
						}
						else
						{
							CopyStringSafely("new password is too short.",
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
					else
					{
						CopyStringSafely("That's not the correct old password.",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/pullchar"))
			{
				argPoint = NextWord(chatText, &linePoint);
//				return;
				if (argPoint == linePoint)
				{
					MessInfoText infoText;
					CopyStringSafely("USAGE: /pullchar <account name> <account password> <character name>",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					int loadcode = -1;
					char OtherName[NUM_OF_CHARS_FOR_USERNAME];
					sscanf(&chatText[argPoint], "%s", OtherName);
					argPoint = NextWord(chatText, &linePoint);
					sscanf(&chatText[argPoint], "%s", tempText);
					// hash the old password.
					unsigned char salt[256];
					sprintf_s((char*)&salt[0], 256, "%s-%s", "BladeMistress", OtherName);
					unsigned char hashPass[HASH_BYTE_SIZE + 1] = { 0 };
					PasswordHash::CreateStandaloneHash((const unsigned char*)tempText, salt, 6969, hashPass);
					unsigned char tempPass[OUT_HASH_SIZE + 1] = { 0 };
					PasswordHash::CreateSerializableHash(hashPass, (unsigned char*)&tempPass[0]);
					BBOSAvatar*otherAvatar = FindAvatar(OtherName,(char*)tempPass,&sp);
					if (otherAvatar)
					{
						// already logged in, abort
						CopyStringSafely("Account logged in still, aborting!",
							200, infoText.text, MESSINFOTEXTLEN);
						lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					else
					{
						otherAvatar = new BBOSAvatar(); // so we can load an account
						loadcode = otherAvatar->LoadAccount(OtherName, (char*)hashPass, FALSE);

						if (loadcode == 0)
						{
							// okay the password is right, now to find the character
							argPoint = NextWord(chatText, &linePoint);
//							tempText = &chatText[argPoint];
//							sscanf(&chatText[argPoint], "%s", tempText); // grab the character name
							strncpy(tempText, &chatText[argPoint], sizeof tempText - 1);
							int findcharindex = -1;
							int findemptyindex = -1;
							int i;
							for (i = 0; i < NUM_OF_CHARS_PER_USER; i++)
							{
								if (strcmp(otherAvatar->charInfoArray[i].name , tempText)==0)
								{
									findcharindex = i; // found the character
									CopyStringSafely("Found character.",
										200, infoText.text, MESSINFOTEXTLEN);
									lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

								}
							}
							if (findcharindex < 0)
							{
								CopyStringSafely("Character not on that account.",
									200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList); // otherAvatar gets destroyed when it goes out of scope.
							}
							else
							{
								// find empty slot on this account
								for (i = 0; i < NUM_OF_CHARS_PER_USER; i++)
								{
									if (strcmp(curAvatar->charInfoArray[i].name , "EMPTY")==0)
									{
										findemptyindex = i; // found the empty slot
										break;				// stop looking
									}
								}
								if (findemptyindex < 0)
								{
									CopyStringSafely("No empty slot on this account.",
										200, infoText.text, MESSINFOTEXTLEN);
									lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									return;																					// sends otherAvatar out of scope safely.
								}
								else
								{
									// ok lets swap!
									// use a pointer to peek, so it can safely go out of scope without destroying anything.
									BBOSCharacterInfo *tempcharacter = &otherAvatar->charInfoArray[findcharindex];
									// check to see if character has quests
									for (int i = 0; i < QUEST_SLOTS; ++i)
									{
										if ((-1 != tempcharacter->quests[i].completeVal) && (enable_cheating == 0)) // allow pullchar anyway if cheat mode is on.
										{
											// we found an active quest. abort!
											CopyStringSafely("Character has an active quest, aborting.",
												200, infoText.text, MESSINFOTEXTLEN);
											lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
											return; // tempcharacter and otheravatar harmlessly go out of scope.
										}

									}
									// now that we are clear, we perform the actual shallow MOVE.
									// temporary character has inventory pointer structs now.  DANGER Will Robinson!
									BBOSCharacterInfo tempcharacter2 = otherAvatar->charInfoArray[findcharindex];
									// copy empty character over found character. safe, because temporary character is now pointing at the inventories
									otherAvatar->charInfoArray[findcharindex] = curAvatar->charInfoArray[findemptyindex];	
									// and copy saved character into empty slot;  now temp and current character are pointing at inventory structs. danger again!
									curAvatar->charInfoArray[findemptyindex] = tempcharacter2;								
									// we need to replace the temporary character with empty inventory pointers.
									tempcharacter2 = otherAvatar->charInfoArray[findcharindex];								// round robin complete so only current account has the inventory
									curAvatar->SaveAccount();																// save current account
									otherAvatar->SaveAccount();																// and other account
									// and now otherAvatar and tempcharacter2 they should safely drop out of scope.
								}

							}

						}
						else
						{
							CopyStringSafely("That's not the correct password.",
								200, infoText.text, MESSINFOTEXTLEN);
							lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							sprintf(&(tempText[2]), "Load error code  %d.",
								loadcode);
							tempText[0] = NWMESS_PLAYER_CHAT_LINE;
							tempText[1] = TEXT_COLOR_DATA;

							lserver->SendMsg(strlen(tempText) + 1, (void *)&tempText, 0, &tempReceiptList);

						}
					}
				}
			}
			//***************************************
			else if (( IsSame(&(chatText[argPoint]) , "/setguildstats")
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType) && (enable_cheating == 0))
                      )
            {
                argPoint = NextWord(chatText,&linePoint);
                if (argPoint == linePoint)
                {
                    sprintf(&chatMess.text[1],"USAGE: /setguildstats <fight #> <mage #> <craft #> <guild name>");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                int n1 = atoi(&(chatText[argPoint]));

                argPoint = NextWord(chatText,&linePoint);
                int n2 = atoi(&(chatText[argPoint]));

                argPoint = NextWord(chatText,&linePoint);
                int n3 = atoi(&(chatText[argPoint]));

                argPoint = NextWord(chatText,&linePoint);

                TowerMap *tm = (TowerMap *) spaceList->Find(&(chatText[argPoint]));
                if (tm)
                {
                  tm->specLevel[0] = n1;
                    tm->specLevel[1] = n2;
                    tm->specLevel[2] = n3;

                    sprintf_s(infoText.text, "Specializations for %s are now", tm->WhoAmI());
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    sprintf_s(infoText.text, "%d FIG  %d MAG  %d CRA", 
                              tm->specLevel[0], tm->specLevel[1], tm->specLevel[2]);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame( &(chatText[argPoint]) , "/status" ) )
            {
                argPoint = NextWord( chatText, &linePoint );

                if ( IsSame(&(chatText[argPoint]) , "afk")) {
                    argPoint = NextWord(chatText,&linePoint);
                    curAvatar->status = AVATAR_STATUS_AFK;
                    sprintf_s( curAvatar->status_text, &(chatText[argPoint]) );

                    sprintf(&chatMess.text[1],"Your status has been changed to AFK.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else if ( IsSame(&(chatText[argPoint]) , "busy")) {
                    argPoint = NextWord(chatText,&linePoint);
                    curAvatar->status = AVATAR_STATUS_BUSY;
                    sprintf_s( curAvatar->status_text, &(chatText[argPoint]) );

                    sprintf(&chatMess.text[1],"Your status has been changed to Busy.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else if ( IsSame(&(chatText[argPoint]) , "available")) {
                    argPoint = NextWord(chatText,&linePoint);
                    curAvatar->status = AVATAR_STATUS_AVAILABLE;
                    sprintf_s( curAvatar->status_text, &(chatText[argPoint]) );

                    sprintf(&chatMess.text[1],"Your status has been changed to Available.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else
                {
                    sprintf(&chatMess.text[1],"Usage: /status <afk, busy or available> <message>");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/shout") ||
                  IsSame(&(chatText[argPoint]) , "/s") )
            {
//				if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime > 1)
                {
                    sprintf(&(tempText[2]),"%s shouts, %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_SHOUT;
                    ss->IgnorableSendToEveryoneNear(curAvatar, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s\n", curAvatar->cellX, curAvatar->cellY, &(tempText[2]));
                    fclose(source);
                }
                /*
                else
                {
                    sprintf_s(&chatMess.text[1],"That feature is locked for 10 minutes.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                */
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/about"))
            {
                int found = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if( targetAv && FALSE == targetAv->isInvisible ) 
                {
                    LongTime now;

                    argPoint = linePoint = linePoint + len;

                    sprintf(&chatMess.text[1],"About %s : ",
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                    long hours, minutes, days;
                    days  = targetAv->charInfoArray[targetAv->curCharacterIndex].lifeTime / 12 / 24;
                    hours = (targetAv->charInfoArray[targetAv->curCharacterIndex].lifeTime -
                        days * 12 * 24) / 12;
                    minutes = (targetAv->charInfoArray[targetAv->curCharacterIndex].lifeTime -
                        days * 12 * 24 - hours * 12) * 5;
                    

                    sprintf(&chatMess.text[1],"     Age: %ld days, %ld hours, %ld minutes",
                        days, hours, minutes);
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                    if( ACCOUNT_TYPE_MODERATOR == targetAv->accountType )
                        sprintf( &chatMess.text[1],"     Account Type: Moderator" );
                    else if( ACCOUNT_TYPE_TRIAL_MODERATOR == targetAv->accountType )
                        sprintf( &chatMess.text[1],"     Account Type: Trial Moderator" );
                    else if( ACCOUNT_TYPE_ADMIN == targetAv->accountType )
                        sprintf( &chatMess.text[1],"     Account Type: Administrator" );
                    else if( targetAv->hasPaid )
                        sprintf( &chatMess.text[1],"     Account Type: Blade Mistress Patron" );
                    else
                        sprintf( &chatMess.text[1],"     Account Type: Regular" );
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

					if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
						ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
						ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType ||
						ACCOUNT_TYPE_HIDDEN_ADMIN == curAvatar->accountType)
					{
						sprintf(&chatMess.text[1], "     Account Name: %s", targetAv->name);
						chatMess.text[0] = TEXT_COLOR_DATA;
						lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
					}
                    sprintf( &chatMess.text[1], "     Stats: %d/%d/%d/%d",
                            targetAv->charInfoArray[targetAv->curCharacterIndex].physical,
                            targetAv->charInfoArray[targetAv->curCharacterIndex].magical,
							targetAv->charInfoArray[targetAv->curCharacterIndex].creative,
							targetAv->charInfoArray[targetAv->curCharacterIndex].beast);

                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                    if( targetAv->status == AVATAR_STATUS_AVAILABLE ) {
                        sprintf( &chatMess.text[1],"     Status: Available" );
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                        sprintf( &chatMess.text[1],"     Message: <none>" );
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if( targetAv->status == AVATAR_STATUS_AFK ) {
                        sprintf( &chatMess.text[1],"     Status: AFK" );
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                        sprintf( &chatMess.text[1],"     Message: %s", targetAv->status_text );
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if( targetAv->status == AVATAR_STATUS_BUSY ) {
                        sprintf( &chatMess.text[1],"     Status: Busy" );
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                        sprintf( &chatMess.text[1],"     Message: %s", targetAv->status_text );
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if (( (IsSame(&(chatText[argPoint]) , "/kickall") ||
                        IsSame(&(chatText[argPoint]) , "/banall")) 
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                             ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType ||
                             ACCOUNT_TYPE_HIDDEN_ADMIN == curAvatar->accountType)
                      ) && (enable_cheating==0))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                BBOSAvatar *targetAv = NULL;

                int ban = FALSE;
                if (IsSame(&(chatText[argPoint]) , "/ban") &&
                      ( ACCOUNT_TYPE_ADMIN == curAvatar->accountType || 
                      ACCOUNT_TYPE_HIDDEN_ADMIN == curAvatar->accountType ))
                    ban = TRUE;

                while (!found && !done)
                {
                    argPoint = NextWord(chatText,&linePoint);
                    if (argPoint == linePoint)
                        done = TRUE;
                    else
                    {
                        memcpy(tempText,&(chatText[start]), linePoint-start);
                        int back = 1;
                        tempText[linePoint-start] = 0;
                        while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                        {
                            tempText[linePoint-start-back] = 0;
                            ++back;
                        }
                        targetAv = FindAvatarByPartialName((char *)tempText, &sp);
                        if (targetAv)
                            found = TRUE;
                    }
                }

                if (found)
                {
                    // boot this joker!
                    sprintf_s(tempText,"%s logged off by administrator.",
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    sp->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                        sizeof(infoText),(void *)&infoText);


                    _strdate( t2 );
                    _strtime( t3 );
                    sprintf_s( t, "[%s %s] %s, %s removed %s, %s from play.\n", 
                        t2,
                        t3,
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        curAvatar->name,
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name, 
                        targetAv->name);
                    LogOutput("moderatorKickLog.txt", t);

                    if (ban)
                        targetAv->accountType = ACCOUNT_TYPE_BANNED;
                    HandleKickoff(targetAv, sp);
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if (( (IsSame(&(chatText[argPoint]) , "/tempban"))
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                             ACCOUNT_TYPE_HIDDEN_ADMIN == curAvatar->accountType)
                      )
					   && (enable_cheating == 0))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                BBOSAvatar *targetAv = NULL;
                int doAll = IsSame(&(chatText[argPoint]) , "/tempbanall");

                argPoint = NextWord(chatText,&linePoint);
                int banHours;
                sscanf(&chatText[argPoint],"%d",&banHours);

                start = linePoint;
                while (!found && !done)
                {
                    if ( doAll)
                    {
                        argPoint = NextWord(chatText,&linePoint);
                        if (argPoint == linePoint)
                            done = TRUE;
                        else
                        {
                            memcpy(tempText,&(chatText[start]), linePoint-start);
                            int back = 1;
                            tempText[linePoint-start] = 0;
                            while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                            {
                                tempText[linePoint-start-back] = 0;
                                ++back;
                            }
                            targetAv = FindAvatarByPartialName((char *)tempText, &sp);
                            if (targetAv)
                                found = TRUE;
                        }
                    }
                    else
                    {
                        argPoint = NextWord(chatText,&linePoint);
                        if (argPoint == linePoint)
                            done = TRUE;
                        else
                        {
                            memcpy(tempText,&(chatText[start]), linePoint-start);
                            int back = 1;
                            tempText[linePoint-start] = 0;
                            while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                            {
                                tempText[linePoint-start-back] = 0;
                                ++back;
                            }
                            targetAv = FindAvatarByAvatarName((char *)tempText, &sp);
                            if (targetAv)
                                found = TRUE;
                        }
                    }
                }

                if (found)
                {
                    // boot this joker!
                    sprintf_s(tempText,"%s excused from play for %d hours.",
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                              banHours);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    sp->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                        sizeof(infoText),(void *)&infoText);

                    _strdate( t2 );
                    _strtime( t3 );
                    sprintf_s( t, "[%s %s] %s, %s removed %s, %s from play for %d hours.\n", 
                        t2,
                        t3,
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        curAvatar->name,
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name, 
                        targetAv->name,
                        banHours);
                    LogOutput("moderatorKickLog.txt", t);

                    targetAv->accountRestrictionTime.SetToNow();
                    targetAv->accountRestrictionTime.AddMinutes(banHours * 60);
                    targetAv->restrictionType = 1; // tempban

                    HandleKickoff(targetAv, sp);
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( (IsSame(&(chatText[argPoint]) , "/givetime"))
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                int found = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv)
                {
                    argPoint = linePoint = linePoint + len;

                    argPoint = NextWord(chatText,&linePoint);
                    int timeToGive = atoi(&(chatText[argPoint]));
                    if (timeToGive <= 0 || timeToGive > 100)
                    {
                        sprintf(&chatMess.text[1],"Invalid amount of time: %d", timeToGive);
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else
                    {
                        targetAv->accountExperationTime.AddMinutes(60*24*timeToGive);
                        targetAv->SaveAccount();

                        tempReceiptList.push_back(targetAv->socketIndex);

                        sprintf_s(tempText,"%s is given %d more days of play time.",
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                              timeToGive);
                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        source = fopen("freeTimeGiven.txt","a");
                        /* Display operating system-style date and time. */
                        _strdate( tempText2 );
                        fprintf(source, "%s, ", tempText2 );
                        _strtime( tempText2 );
                        fprintf(source, "%s, ", tempText2 );
                        fprintf(source,"%s is given %d more days by %s.", 
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                              timeToGive,
                             curAvatar->charInfoArray[targetAv->curCharacterIndex].name);
                        fclose(source);
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }

            //***************************************
            else if (( (IsSame(&(chatText[argPoint]) , "/kick") ||
                        IsSame(&(chatText[argPoint]) , "/ban")) 
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                             ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType ||
                             ACCOUNT_TYPE_HIDDEN_ADMIN == curAvatar->accountType)
							 ) && (enable_cheating == 0))
            {
                int found = FALSE;
                int done = FALSE;

                int ban = FALSE;
                if (IsSame(&(chatText[argPoint]) , "/ban") &&
                     ( ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                     ACCOUNT_TYPE_HIDDEN_ADMIN == curAvatar->accountType ))
                    ban = TRUE;

                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv)
                {
                    argPoint = linePoint = linePoint + len;

                    // boot this joker!
                    sprintf_s(tempText,"%s logged off by administrator.",
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    sp->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                        sizeof(infoText),(void *)&infoText);

                    _strdate( t2 );
                    _strtime( t3 );
                    sprintf_s( t, "[%s %s] %s, %s removed %s, %s from play.\n", 
                        t2,
                        t3,
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        curAvatar->name,
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name, 
                        targetAv->name);
                    LogOutput("moderatorKickLog.txt", t);

                    if (ban)
                        targetAv->accountType = 10;
                    HandleKickoff(targetAv, sp);
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }

            //***************************************
            else if (( IsSame(&(chatText[argPoint]) , "/uidban") 
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType) && (enable_cheating == 0))
            {
                int found = FALSE;
                int done = FALSE;
                int showList = FALSE;
                int remove = FALSE;
                int start = linePoint;
                BBOSAvatar *targetAv = NULL;

                while (!found && !done && !showList & !remove)
                {
                    argPoint = NextWord(chatText,&linePoint);
                    if (argPoint == linePoint)
                        done = TRUE;
                    else if (!stricmp("list", &chatText[argPoint]))
                        showList = TRUE;
                    else if (!strnicmp("remove", &chatText[argPoint], 6))
                        remove = TRUE;
                    else
                    {
                        memcpy(tempText,&(chatText[start]), linePoint-start);
                        int back = 1;
                        tempText[linePoint-start] = 0;
                        while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                        {
                            tempText[linePoint-start-back] = 0;
                            ++back;
                        }
                        targetAv = FindAvatarByAvatarName((char *)tempText, &sp);
                        if (targetAv)
                            found = TRUE;
                    }
                }

                if (found)
                {
                    // boot this joker, and ban his UID!
                    sprintf_s(tempText,"%s UID banned by administrator.",
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    sp->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                        sizeof(infoText),(void *)&infoText);

                    targetAv->accountType = 20;
                    uidBanList->addBannedUID(targetAv->uniqueId,
                         targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    uidBanList->Save();
                    HandleKickoff(targetAv, sp);
                }
                else if (showList)
                {
                    sprintf(&chatMess.text[1],"Banned UID list:");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                    UidBanRecord *br = (UidBanRecord *) uidBanList->bannedUIDRecords.First();
                    while (br)
                    {
                        sprintf_s(tempText,"%d   %s %s",
                            br->uid,
                            br->dateString, br->WhoAmI());
                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        br = (UidBanRecord *) uidBanList->bannedUIDRecords.Next();
                    }
                }
                else if (remove)
                {
                    int uid = 0;
                    argPoint = NextWord(chatText,&linePoint);

                    sscanf(&chatText[argPoint],"%d", &uid);

                    UidBanRecord *br = (UidBanRecord *) uidBanList->bannedUIDRecords.First();
                    while (br)
                    {
                        if (uid == br->uid)
                        {
                            uidBanList->bannedUIDRecords.Remove(br);
                            delete br;
                            uidBanList->Save();
                            sprintf(&chatMess.text[1],"UID removed.");
                            chatMess.text[0] = TEXT_COLOR_DATA;
                            lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        }
                        br = (UidBanRecord *) uidBanList->bannedUIDRecords.Next();
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"/uidban <character name>   adds UID to list, kicks.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"/uidban list   shows list of banned UIDs.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"/uidban remove 12345  removes UID 12345 from list.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }

            //***************************************
            else if (( IsSame(&(chatText[argPoint]) , "/ipban") 
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType) && (enable_cheating == 0))
            {
                int found = FALSE;
                int done = FALSE;
                int showList = FALSE;
                int remove = FALSE;
                int start = linePoint;
                BBOSAvatar *targetAv = NULL;

                while (!found && !done && !showList & !remove)
                {
                    argPoint = NextWord(chatText,&linePoint);
                    if (argPoint == linePoint)
                        done = TRUE;
                    else if (!stricmp("list", &chatText[argPoint]))
                        showList = TRUE;
                    else if (!strnicmp("remove", &chatText[argPoint], 6))
                        remove = TRUE;
                    else
                    {
                        memcpy(tempText,&(chatText[start]), linePoint-start);
                        int back = 1;
                        tempText[linePoint-start] = 0;
                        while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                        {
                            tempText[linePoint-start-back] = 0;
                            ++back;
                        }
                        targetAv = FindAvatarByAvatarName((char *)tempText, &sp);
                        if (targetAv)
                            found = TRUE;
                    }
                }

                if (found)
                {
                    // boot this joker, and ban his IP!
                    sprintf_s(tempText,"%s IP banned by administrator.",
                             targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    sp->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
                                        sizeof(infoText),(void *)&infoText);

                    targetAv->accountType = 10;
                    banList->addBannedIP(targetAv->IP,
                         targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    HandleKickoff(targetAv, sp);
                }
                else if (showList)
                {
                    sprintf(&chatMess.text[1],"Banned IP list:");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                    BanRecord *br = (BanRecord *) banList->bannedIPRecords.First();
                    while (br)
                    {
                        sprintf_s(tempText,"%d.%d.%d.%d   %s %s",
                            (int) br->ip[0], (int) br->ip[1], (int) br->ip[2], (int) br->ip[3],
                            br->dateString, br->WhoAmI());
                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        br = (BanRecord *) banList->bannedIPRecords.Next();
                    }
                }
                else if (remove)
                {
                    int ipNums[4];
                    argPoint = NextWord(chatText,&linePoint);

                    sscanf(&chatText[argPoint],"%d.%d.%d.%d", 
                            &ipNums[0], &ipNums[1], &ipNums[2], &ipNums[3]);

                    BanRecord *br = (BanRecord *) banList->bannedIPRecords.First();
                    while (br)
                    {
                        if (ipNums[0] == br->ip[0] &&
                            ipNums[1] == br->ip[1] &&
                            ipNums[2] == br->ip[2] &&
                            ipNums[3] == br->ip[3])
                        {
                            banList->bannedIPRecords.Remove(br);
                            delete br;
                            sprintf(&chatMess.text[1],"IP removed.");
                            chatMess.text[0] = TEXT_COLOR_DATA;
                            lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        }
                        br = (BanRecord *) banList->bannedIPRecords.Next();
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"/ipban <character name>   adds IP to list, kicks.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"/ipban list   shows list of banned IPs.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    sprintf(&chatMess.text[1],"/ipban remove 128.0.0.1  removes IP 128.0.0.1 from list.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if (( IsSame(&(chatText[argPoint]) , "/ip") 
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType) && (enable_cheating == 0))
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                BBOSAvatar *targetAv = NULL;

                while (!found && !done)
                {
                    argPoint = NextWord(chatText,&linePoint);
                    if (argPoint == linePoint)
                        done = TRUE;
                    else
                    {
                        memcpy(tempText,&(chatText[start]), linePoint-start);
                        int back = 1;
                        tempText[linePoint-start] = 0;
                        while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                        {
                            tempText[linePoint-start-back] = 0;
                            ++back;
                        }
                        targetAv = FindAvatarByAvatarName((char *)tempText, &sp);
                        if (targetAv)
                            found = TRUE;
                    }
                }

                if (found)
                {
                    sprintf_s(tempText,"%d.%d.%d.%d   %s (%s)",
                        (int) targetAv->IP[0], (int) targetAv->IP[1], 
                        (int) targetAv->IP[2], (int) targetAv->IP[3],
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                        targetAv->name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/broadcast") ||
                  IsSame(&(chatText[argPoint]) , "/b") )
            {
                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                {
                    sprintf(&(tempText[2]),"%s",&(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_DATA;

                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, ADMIN %s\n", curAvatar->cellX, curAvatar->cellY, &(tempText[2]));
                    fclose(source);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/joinguild")
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType)
                      )
            {
                argPoint = NextWord(chatText,&linePoint);

                TowerMap *tm = (TowerMap *) spaceList->Find(&(chatText[argPoint]));
                if (tm)
                {
                    MemberRecord *mr = new MemberRecord(0,
                               curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                    tm->members->Append(mr);

                    tm->lastChangedTime.SetToNow();

                    sprintf(&(tempText[1]),"%s is now an honorary member of %s.",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        tm->WhoAmI());
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                       strlen(tempText) + 1,(void *)&tempText,2);

                    curAvatar->AssertGuildStatus(ss,TRUE);

                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/guildstats")
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                      )
            {
                argPoint = NextWord(chatText,&linePoint);

                TowerMap *tm = (TowerMap *) spaceList->Find(&(chatText[argPoint]));
                if (tm)
                {
                    sprintf_s(infoText.text, "Specializations for %s are", tm->WhoAmI());
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    sprintf_s(infoText.text, "%d FIG  %d MAG  %d CRA", 
                              tm->specLevel[0], tm->specLevel[1], tm->specLevel[2]);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/getquest")
                         && (ACCOUNT_TYPE_ADMIN == curAvatar->accountType ||
                             ACCOUNT_TYPE_MODERATOR == curAvatar->accountType)
                      )
            {
                argPoint = NextWord(chatText,&linePoint);
                if (argPoint == linePoint)
                {
                    CopyStringSafely("USAGE: /getquest <0-12>     gives you a quest", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
                else
                {
                    int qType;
                    sscanf(&(chatText[argPoint]), "%d", &qType);

                    BBOSTree *t = NULL;
                    BBOSMob *curMob2 = (BBOSMob *) ss->mobList->GetFirst(0,0,1000);
                    while (curMob2 && !t)
                    {
                        if (SMOB_TREE == curMob2->WhatAmI())
                            t = (BBOSTree *) curMob2;

                        curMob2 = (BBOSMob *) ss->mobList->GetNext();
                    }

                    if (!t)
                    {
                        CopyStringSafely("Use this command on the overland map.", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        return;
                    }

                    int freeSlot = -1;
                    for (int i = 0; i < QUEST_SLOTS; ++i)
                    {
                        if (-1 == curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                          quests[i].completeVal)
                        {
                            freeSlot = i;
                            i = QUEST_SLOTS;
                        }
                    }

                    if (-1 == freeSlot)
                    {
                        sprintf_s(infoText.text,"You are already burdened with enough tasks.");
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        CreateTreeQuest(&(curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                          quests[freeSlot]), curAvatar, ss, t, qType);
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/loadquests")
                         && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                questMan->Load();
            }
            //***************************************
			else if (IsSame(&(chatText[argPoint]), "/namechange")
			&& ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
			{
			int found = FALSE;
			int done = FALSE;
			int len;
			BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

			if (targetAv)
			{
				argPoint = linePoint = linePoint + len;
				// change this avatar's name
				tempReceiptList.push_back(targetAv->socketIndex); // list is for me and target now

				if (!UN_IsNameUnique(&(chatText[argPoint])))
				{
					sprintf_s(tempText, "Name change for %s failed.  Already taken.",
						targetAv->charInfoArray[targetAv->curCharacterIndex].name);
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					UN_RemoveName(targetAv->charInfoArray[targetAv->curCharacterIndex].name);

					UN_AddName(&(chatText[argPoint]));

					sprintf_s(tempText, "%s name changed by administrator.",
						targetAv->charInfoArray[targetAv->curCharacterIndex].name);
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					//					sp->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
					//										sizeof(infoText),(void *)&infoText);
					//					if (sp != ss)
					//						ss->SendToEveryoneNearBut(0, targetAv->cellX, targetAv->cellY,
					//										sizeof(infoText),(void *)&infoText);

					int res = ChangeAvatarGuildName(
						targetAv->charInfoArray[targetAv->curCharacterIndex].name,
						&chatText[argPoint]);

					if (!res)
					{
						sprintf(&chatMess.text[1], "WARNING: no guild, or guild list change unsuccessful");
						chatMess.text[0] = TEXT_COLOR_DATA;
						lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
					}

					CopyStringSafely(&chatText[argPoint],
						strlen(&chatText[argPoint]) + 1,
						targetAv->charInfoArray[targetAv->curCharacterIndex].name,
						32);
					targetAv->SaveAccount();
				}
			}
			else
			{
				sprintf(&chatMess.text[1], "No one of that name is logged on.");
				chatMess.text[0] = TEXT_COLOR_DATA;

				lserver->SendMsg(sizeof(chatMess), (void *)&chatMess, 0, &tempReceiptList);
			}
			}
			//***************************************
			else if (IsSame(&(chatText[argPoint]), "/petname")
			&& curAvatar->BeastStat()>9)
			{
				int found = FALSE;
				int done = FALSE;
				int len;
				if (!UN_IsNameUnique(&(chatText[linePoint])))  // check for player impersonation.
				{
					sprintf_s(tempText, "Can't include a player name in a pet name.");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{
					// rename the pet wtf


					sprintf(tempText, "%s's %s", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
						&(chatText[linePoint]));
					CopyStringSafely(tempText, 1024,
						curAvatar->controlledMonster->uniqueName, 32);
					sprintf_s(tempText, "Rename successful. (give it a few seconds)");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				}
			}
			//***************************************

            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/announce") ||
                  IsSame(&(chatText[argPoint]) , "/a") )
            {
//				if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime > 1)
                {
                    sprintf(&(tempText[2]),"%s tells everyone, %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_ANNOUNCE;

                    sp = (SharedSpace *) spaceList->First();
                    while (sp)
                    {
                        sp->IgnorableSendToEveryone(curAvatar, strlen(tempText) + 1,(void *)&tempText);
                        sp = (SharedSpace *) spaceList->Next();
                    }

                    //lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s\n", curAvatar->cellX, curAvatar->cellY, &(tempText[2]));
                    fclose(source);
                }
                /*
                else
                {
                    sprintf_s(&chatMess.text[1],"That feature is locked for 10 minutes.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                */
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/emote") ||
                  IsSame(&(chatText[argPoint]) , "/e") )
            {
                argPoint = NextWord(chatText,&linePoint);

                if (curAvatar->BeastStat()<10 && curAvatar->controlledMonster)  // ony non beastmasters can make their monster talk
                {
                    SharedSpace *sx;

                    BBOSMonster * theMonster = FindMonster(
                              curAvatar->controlledMonster, &sx);
                    if (theMonster)
                    {
                        sprintf(&(tempText[2]),"%s %s",
                            theMonster->Name(),
                            &(chatText[argPoint]));
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        tempText[1] = TEXT_COLOR_EMOTE;
                        sx->IgnorableSendToEveryoneNear(curAvatar, theMonster->cellX, theMonster->cellY,
                                              strlen(tempText) + 1,(void *)&tempText,2);

                        source = fopen("logs\\chatline.txt","a");
                        /* Display operating system-style date and time. */
                        _strdate( tempText2 );
                        fprintf(source, "%s, ", tempText2 );
                        _strtime( tempText2 );
                        fprintf(source, "%s, ", tempText2 );
                        fprintf(source,"%d, %d, %s\n", theMonster->cellX, theMonster->cellY, &(tempText[2]));
                        fclose(source);
                    }
                }
                else
                {
                    sprintf(&(tempText[2]),"%s %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[argPoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_EMOTE;
                    ss->IgnorableSendToEveryoneNear(curAvatar, curAvatar->cellX, curAvatar->cellY,
                                          strlen(tempText) + 1,(void *)&tempText,2);

                    curAvatar->QuestTalk(ss);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s\n", curAvatar->cellX, curAvatar->cellY, 
                            &(tempText[2]));
                    fclose(source);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/token"))
            {
                for (int i = 0; i < MAGIC_MAX; ++i)
                {
                    if (tokenMan.towerName[i][0])
                    {
                        LongTime now;
                        sprintf(&(tempText[2]),"%s token: %s: %d hours left.", magicNameList[i],
                            tokenMan.towerName[i],now.MinutesDifference(&tokenMan.tokenTimeLeft[i]) 
                                 / 60);
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        tempText[1] = TEXT_COLOR_DATA;
                        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                    }
                    else
                    {
                        LongTime now;
                        sprintf(&(tempText[2]),"%s token: unclaimed.", magicNameList[i]);
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        tempText[1] = TEXT_COLOR_DATA;
                        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/time"))
            {
                _strdate( tempText2 );
                _strtime( tempText3 );

                sprintf(&chatMess.text[1],"%s, %s (real time at server).", tempText2, tempText3);
                chatMess.text[0] = TEXT_COLOR_DATA;
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                int oneHour = 60 * 6;
                int gameTime = dayTimeCounter / (float)(oneHour * 4) * 24.0f * 60;
                gameTime += 6 * 60;
                if (gameTime > 24 * 60)
                    gameTime -= 24 * 60;

                if (dayTimeCounter < 2.5f * oneHour)
                    sprintf_s(tempText2,"DAY");
                else if (dayTimeCounter >= 2.75f * oneHour && dayTimeCounter < 3.75f * oneHour)
                    sprintf_s(tempText2,"NIGHT");
                else if (dayTimeCounter >= 2.5f * oneHour && dayTimeCounter < 2.75f * oneHour)
                    sprintf_s(tempText2,"DUSK");
                else if (dayTimeCounter >= 3.75f * oneHour)
                    sprintf_s(tempText2,"DAWN");

                sprintf(&chatMess.text[1],"Game time of day is %d:%02d (%s)", gameTime/60,
                                            gameTime - (gameTime/60*60), tempText2);
                chatMess.text[0] = TEXT_COLOR_DATA;
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
            }
            //***************************************
            else if ( false && IsSame(&(chatText[argPoint]) , "/referredby")) // no more referrals
            {
                int found = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv && !curAvatar->isReferralDone && curAvatar->hasPaid)
                {
                    argPoint = linePoint = linePoint + len;

                    sprintf(&(tempText[2]),"You reward %s for your referral!  Thank you!",
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    std::vector<TagID> targetReceiptList;
                    targetReceiptList.clear();
                    targetReceiptList.push_back(targetAv->socketIndex);

                    sprintf(&(tempText[2]),"%s acknowledges that you referred her to Blade Mistress!",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

                    curAvatar->isReferralDone = TRUE;
                    ++targetAv->patronCount;

                    // log the referral
                    source = fopen("referralLog.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%s, refers, %s, %d\n", 
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                        targetAv->patronCount);
                    fclose(source);

                    if (!targetAv->hasPaid)
                    {
                        if (targetAv->patronCount < 2)
                        {
                            sprintf(&(tempText[2]),"You only need one more referral to become a paid Blade Mistress player!");
                            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);
                        }
                        else
                        {
                            sprintf(&(tempText[2]),"You are now a paid Blade Mistress player, with one more week of play time!");
                            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

                            targetAv->hasPaid = TRUE;

                            LongTime now;
                            if (now.MinutesDifference(&targetAv->accountExperationTime) > 0)
                                targetAv->accountExperationTime.AddMinutes(60*24*7);
                            else
                            {
                                targetAv->accountExperationTime.SetToNow();
                                targetAv->accountExperationTime.AddMinutes(60*24*7);
                            }
                        }
                    }
                    else
                    {
                        sprintf(&(tempText[2]),"You get one more week of play time!");
                        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

                        LongTime now;
                        if (now.MinutesDifference(&targetAv->accountExperationTime) > 0)
                            targetAv->accountExperationTime.AddMinutes(60*24*7);
                        else
                        {
                            targetAv->accountExperationTime.SetToNow();
                            targetAv->accountExperationTime.AddMinutes(60*24*7);
                        }
                    }

                    targetAv->SaveAccount();
                    curAvatar->SaveAccount();

                }
                else
                {
                    if (!targetAv)
                        sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    else if (!curAvatar->hasPaid)
                        sprintf(&chatMess.text[1],"You may only refer once you have paid.");
                    else
                        sprintf(&chatMess.text[1],"You have already used /referredby successfully.");

                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/tell") ||
                  IsSame(&(chatText[argPoint]) , "/t") )
            {
                int ignore = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv && targetAv->IsContact(
                       curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        CONTACT_IGNORE))
                {
//				  	sprintf_s(&(tempText[1]),"%s is ignoring you.",
//						targetAv->charInfoArray[targetAv->curCharacterIndex].name,
//						&(chatText[linePoint]));
//					tempText[0] = NWMESS_PLAYER_CHAT_LINE;
//			  		lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    ignore = TRUE;
                }

                if( targetAv && !ignore && FALSE == targetAv->isInvisible )
                {
                    argPoint = linePoint = linePoint + len;

                    sprintf(&(tempText[2]),"You tell %s, %s",
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                        &(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    std::vector<TagID> targetReceiptList;
                    targetReceiptList.clear();
                    targetReceiptList.push_back(targetAv->socketIndex);

                    sprintf(&(tempText[2]),"%s tells you, %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

                    CopyStringSafely(
                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, 32, 
                         targetAv->lastTellName, 32);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s, %s\n", curAvatar->cellX, curAvatar->cellY, 
                        &(tempText[2]), 
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    fclose(source);

                    // KARMA
                    HandleKarmaText(&(chatText[linePoint]), curAvatar, targetAv);



                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/reply") ||
                  IsSame(&(chatText[argPoint]) , "/r") )
            {
                int found = FALSE;
                int done = FALSE;
                int start = linePoint;
                BBOSAvatar *targetAv = NULL;

                if (0 == curAvatar->lastTellName[0])
                {
                    sprintf(&chatMess.text[1],"You have not yet been /telled by anyone.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                targetAv = FindAvatarByAvatarName(curAvatar->lastTellName, &sp);
                if( targetAv && FALSE == targetAv->isInvisible )
                    found = TRUE;

                if (found && targetAv->IsContact(
                       curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        CONTACT_IGNORE))
                {
//				  	sprintf_s(&(tempText[1]),"%s is ignoring you.",
//						targetAv->charInfoArray[targetAv->curCharacterIndex].name,
//						&(chatText[linePoint]));
//					tempText[0] = NWMESS_PLAYER_CHAT_LINE;
//			  		lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    found = FALSE;
                }

                if (found)
                {

                    sprintf(&(tempText[2]),"You tell %s, %s",
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                        &(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    std::vector<TagID> targetReceiptList;
                    targetReceiptList.clear();
                    targetReceiptList.push_back(targetAv->socketIndex);

                    sprintf(&(tempText[2]),"%s tells you, %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[linePoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

                    CopyStringSafely(
                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, 32, 
                         targetAv->lastTellName, 32);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s, %s\n", curAvatar->cellX, curAvatar->cellY, 
                        &(tempText[2]), 
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name);
                    fclose(source);

                    // KARMA
                    HandleKarmaText(&(chatText[linePoint]), curAvatar, targetAv);

                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/guildinfo"))
            {
                int found = FALSE;
                int done = FALSE, messLen = 0;
                int start = linePoint;

                TowerMap *tMap = NULL;
                while (!found && !done)
                {
                    argPoint = NextWord(chatText,&linePoint);
                    if (argPoint == linePoint)
                        done = TRUE;
                    else
                    {
                        memcpy(tempText,&(chatText[start]), linePoint-start);
                        int back = 1;
                        tempText[linePoint-start] = 0;
                        while (' ' == tempText[linePoint-start-back] && linePoint-start-back > 0)
                        {
                            tempText[linePoint-start-back] = 0;
                            ++back;
                        }

                        SharedSpace *sp = (SharedSpace *) spaceList->First();
                        while (sp)
                        {
                            if (SPACE_GUILD == sp->WhatAmI() && 
                                 IsCompletelySame(tempText, sp->WhoAmI())) 
                                tMap = (TowerMap *) sp;

                            sp = (SharedSpace *) spaceList->Next();
                        }

                        if (tMap)
                            found = TRUE;
                    }
                }

                if (found)
                {
                    sprintf(&chatMess.text[1],"%s: %s", 
                                 guildStyleNames[tMap->guildStyle], tMap->WhoAmI());
                    messLen = strlen(chatMess.text);

                    sprintf(&(chatMess.text[messLen])," (spec %d/%d/%d)", 
                               tMap->specLevel[0], tMap->specLevel[1], tMap->specLevel[2]);
                    messLen = strlen(chatMess.text);

                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                    messLen = 0;
                    int memberNum = 0;

                    MemberRecord *mr = (MemberRecord *) tMap->members->First();
                    while (mr)
                    {
                        ++memberNum;
                        mr = (MemberRecord *) tMap->members->Next();
                    }

                    LongTime now;

                    long diff = tMap->lastChangedTime.MinutesDifference(&now) / 60 / 24;

                    sprintf(&chatMess.text[1],"%dN %dE. %d members. Last activity: %ld days", 
                                 256- tMap->enterY, 256- tMap->enterX, memberNum, diff);
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                }
                else
                {
                    sprintf(&chatMess.text[1],"No such guild could be found.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/where") ||
                      IsSame(&(chatText[argPoint]) , "/whereis"))
            {
                int found = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv && FALSE == targetAv->isInvisible)
                {
                    argPoint = linePoint = linePoint + len;

                    int maxX = 0, maxY = 0;
                    if (SPACE_GROUND == sp->WhatAmI())
                    {
                        maxX = 256;
                        maxY = 256;
                    }
                    else if (SPACE_DUNGEON == sp->WhatAmI())
                    {
                        maxX = ((DungeonMap *)sp)->width;
                        maxY = ((DungeonMap *)sp)->height;
                    }
                    else if (SPACE_GUILD == sp->WhatAmI())
                    {
                        maxX = ((TowerMap *)sp)->width;
                        maxY = ((TowerMap *)sp)->height;
                    }
                    else if (SPACE_REALM == sp->WhatAmI())
                    {
                        maxX = ((RealmMap *)sp)->width;
                        maxY = ((RealmMap *)sp)->height;
                    }
                    else if (SPACE_LABYRINTH == sp->WhatAmI())
                    {
                        maxX = ((LabyrinthMap *)sp)->width;
                        maxY = ((LabyrinthMap *)sp)->height;
                    }

                    if (SPACE_DUNGEON == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the %s (%dN %dE),", ((DungeonMap *) sp)->name,
                            256-((DungeonMap *) sp)->enterY, 256-((DungeonMap *) sp)->enterX);
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if (SPACE_GUILD == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the guild tower (%dN %dE),",
                            256-((TowerMap *) sp)->enterY, 256-((TowerMap *) sp)->enterX);
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if (SPACE_REALM == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the %s,", sp->WhoAmI());
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if (SPACE_LABYRINTH == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the Labyrinth,");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    sprintf(&chatMess.text[1],"%s is at %dN %dE.",
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                        maxY - targetAv->cellY, maxX - targetAv->cellX);
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/loc"))
            {
                int found = FALSE;
                int done = FALSE;
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);

                if (targetAv && FALSE == targetAv->isInvisible)
                {
                    argPoint = linePoint = linePoint + len;

                    if (SPACE_DUNGEON == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the %s (%dN %dE),", ((DungeonMap *) sp)->name,
                            ((DungeonMap *) sp)->enterY, ((DungeonMap *) sp)->enterX);
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if (SPACE_GUILD == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the guild tower (%dN %dE),",
                            ((TowerMap *) sp)->enterY, ((TowerMap *) sp)->enterX);
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if (SPACE_REALM == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the %s,", sp->WhoAmI());
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    else if (SPACE_LABYRINTH == sp->WhatAmI())
                    {
                        sprintf(&chatMess.text[1],"In the Labyrinth,");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    }
                    sprintf(&chatMess.text[1],"%s is at %dN %dE.",
                        targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                        targetAv->cellY, targetAv->cellX);
                    chatMess.text[0] = TEXT_COLOR_DATA;

                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
                else
                {
                    sprintf(&chatMess.text[1],"No one of that name is logged on.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/whoall"))
            {
                ProcessWho(curAvatar,NULL);
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/who"))
            {
                ProcessWho(curAvatar,ss);
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/ignore"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (argPoint != linePoint)
                    ProcessFriend(curAvatar, FRIEND_ACTION_IGNORE, ss, &(chatText[argPoint]));
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/friend") || 
                       IsSame(&(chatText[argPoint]) , "/f"))
            {
                argPoint = NextWord(chatText,&linePoint);
                if (argPoint == linePoint)
                    ProcessFriend(curAvatar, FRIEND_ACTION_LIST, ss);
                else if ( IsSame(&(chatText[argPoint]) , "list")) 
                    ProcessFriend(curAvatar, FRIEND_ACTION_LIST, ss);
                else if ( IsSame(&(chatText[argPoint]) , "add")) 
                {
                    argPoint = NextWord(chatText,&linePoint);
                    ProcessFriend(curAvatar, FRIEND_ACTION_ADD, ss, &(chatText[argPoint]));
                }
                else if ( IsSame(&(chatText[argPoint]) , "remove")) 
                {
                    argPoint = NextWord(chatText,&linePoint);
                    ProcessFriend(curAvatar, FRIEND_ACTION_REMOVE, ss, &(chatText[argPoint]));
                }
                else
                {
                    sprintf(&(tempText[1]),"%s tells friends, %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[argPoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;

                    sp = (SharedSpace *) spaceList->First();
                    while (sp)
                    {
                        sp->SendToEveryFriend(curAvatar, strlen(tempText) + 1,(void *)&tempText);
                        sp = (SharedSpace *) spaceList->Next();
                    }

                    // send to myself as well
                    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s\n", curAvatar->cellX, curAvatar->cellY, &(tempText[1]));
                    fclose(source);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/guildkick"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                {
                    TowerMap *tm = (TowerMap *)guildSpace;
                    MemberRecord *mr2 = (MemberRecord *) tm->members->Find(&(chatText[argPoint]));
                    if (!mr2)
                    {
                        sprintf(&chatMess.text[1],"%s isn't in the guild.", &(chatText[argPoint]));
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        return;
                    }

                    sprintf(&(tempText[2]),"%s is removed from the guild by ADMIN.",
                        mr2->WhoAmI());
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_DATA;

                    sp = (SharedSpace *) spaceList->First();
                    while (sp)
                    {
                        SendToEveryGuildMate("NILLNILL",	sp, tm, 
                            strlen(tempText) + 1,(void *)&tempText);
                        sp = (SharedSpace *) spaceList->Next();
                    }

                    tm->members->Remove(mr2);
                    tm->lastChangedTime.SetToNow();
                    delete mr2;

                }
                else if (GUILDSTYLE_MONARCHY == ((TowerMap *)guildSpace)->guildStyle ||
                         GUILDSTYLE_TYRANNY  == ((TowerMap *)guildSpace)->guildStyle)
                {

                    TowerMap *tm = (TowerMap *)guildSpace;
                    MemberRecord *mr = (MemberRecord *)
                        tm->members->Find(
                                 curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                    if (!mr)
                    {
                        sprintf(&chatMess.text[1],"You're not in your own guild.  Internal Error 2.");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        return;
                    }

                    MemberRecord *mr2 = (MemberRecord *)
                        tm->members->Find(&(chatText[argPoint]));
                    if (!mr2)
                    {
                        sprintf(&chatMess.text[1],"%s isn't in the guild.", &(chatText[argPoint]));
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        return;
                    }

                    if (mr->value1 > mr2->value1)
                    {

                        sprintf(&(tempText[2]),"%s is kicked out of the guild by %s.",
                            mr2->WhoAmI(), mr->WhoAmI());
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        tempText[1] = TEXT_COLOR_DATA;

                        sp = (SharedSpace *) spaceList->First();
                        while (sp)
                        {
                            SendToEveryGuildMate("NILLNILL",	sp, tm, 
                                strlen(tempText) + 1,(void *)&tempText);
                            sp = (SharedSpace *) spaceList->Next();
                        }

                        tm->members->Remove(mr2);
                        tm->lastChangedTime.SetToNow();
                        delete mr2;
                    }
                    else
                    {

                        sprintf(&(tempText[2]),"%s tries to kick out %s, but doesn't rank higher.",
                            mr->WhoAmI(), mr2->WhoAmI());
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        tempText[1] = TEXT_COLOR_DATA;

                        sp = (SharedSpace *) spaceList->First();
                        while (sp)
                        {
                            SendToEveryGuildMate("NILLNILL",	sp, tm, 
                                strlen(tempText) + 1,(void *)&tempText);
                            sp = (SharedSpace *) spaceList->Next();
                        }
                    }

                }
                else
                {
                    sprintf(&chatMess.text[1],"Your guild style doesn't allow /guildkick.  Use /startvote instead.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/guildname"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                if (IsSame("UNNAMED GUILD",guildSpace->WhoAmI()))
                {
                    if (UN_IsNameUnique(&(chatText[argPoint])))
                    {
                        UN_AddName(&(chatText[argPoint]));

                        CopyStringSafely(&(chatText[argPoint]),1024, guildSpace->do_name,64);
                        sprintf(&(tempText[2]),"The %s guild has now been christened by %s.",
                            guildSpace->WhoAmI(),
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        tempText[1] = TEXT_COLOR_DATA;
                        ss->SendToEveryoneNearBut(0,curAvatar->cellX, curAvatar->cellY,
                                              strlen(tempText) + 1,(void *)&tempText);
                    }
                    else
                    {
                        sprintf(&chatMess.text[1],"That guild name is taken.");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        return;
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"Your guild is already named.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/guildtype"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                int index = 0;
                if (GUILDSTYLE_NONE == ((TowerMap *)guildSpace)->guildStyle)
                {
                    for (index = 1; index < GUILDSTYLE_MAX; ++index)
                    {
                        if (!stricmp(&(chatText[argPoint]), guildStyleNames[index]))
                        {
                            ((TowerMap *)guildSpace)->guildStyle = index;

//							CopyStringSafely(&(chatText[argPoint]),1024, guildSpace->do_name,64);
                            sprintf(&(tempText[2]),"The %s guild is now a %s.",
                                ((TowerMap *)guildSpace)->WhoAmI(),
                                guildStyleNames[index]);
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_DATA;
                            ss->SendToEveryoneNearBut(0,curAvatar->cellX, curAvatar->cellY,
                                                  strlen(tempText) + 1,(void *)&tempText);

                            sprintf(&(tempText[2]),"This was set by by %s.",
                                curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                            ss->SendToEveryoneNearBut(0,curAvatar->cellX, curAvatar->cellY,
                                                  strlen(tempText) + 1,(void *)&tempText);
                            index = 1000;
                        }
                    }

                    if (index < 1000)
                    {
                        sprintf(&chatMess.text[1],"That is not a valid guild type.");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                        sprintf(&chatMess.text[1],"USAGE: /guildtype <CAUCUS or COUNCIL or VESTRY or MONARCHY or TYRANNY>");
                        chatMess.text[0] = TEXT_COLOR_DATA;
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

                        return;
                    }
                }
                else
                {
                    sprintf(&chatMess.text[1],"Your guild is already a %s.",
                                  guildStyleNames[((TowerMap *)guildSpace)->guildStyle]);
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/quitguild"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, 
                              &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                sprintf(&(tempText[2]),"%s leaves the %s guild.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                    guildSpace->WhoAmI());
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                tempText[1] = TEXT_COLOR_DATA;
                ss->SendToEveryoneNearBut(0,curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText);

                DeleteNameFromGuild(
                      curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, 
                      &guildSpace);

                curAvatar->AssertGuildStatus(ss,TRUE);

            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/guild") || 
                       IsSame(&(chatText[argPoint]) , "/g"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                if (argPoint == linePoint)
                    ListGuild(curAvatar, (TowerMap *)guildSpace);
                else
                {
                    sprintf(&(tempText[2]),"%s tells the guild, %s",
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                        &(chatText[argPoint]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_GUILD;

                    sp = (SharedSpace *) spaceList->First();
                    while (sp)
                    {
                        SendToEveryGuildMate(
                                curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                                sp, (TowerMap *)guildSpace, strlen(tempText) + 1,(void *)&tempText);
                        sp = (SharedSpace *) spaceList->Next();
                    }

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s\n", curAvatar->cellX, curAvatar->cellY, &(tempText[2]));
                    fclose(source);
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/vote"))
            {
                argPoint = NextWord(chatText,&linePoint);
                // find the guild I belong to
                SharedSpace *guildSpace;
                if (!FindAvatarInGuild(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, &guildSpace))
                {
                    sprintf(&chatMess.text[1],"You do not belong to a guild.");
                    chatMess.text[0] = TEXT_COLOR_DATA;
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    return;
                }

                if (argPoint == linePoint)
                    ListVotes(curAvatar, (TowerMap *)guildSpace);
                else
                {
                    int targX;
                    sscanf(&chatText[argPoint],"%d",&targX);

                    argPoint = NextWord(chatText,&linePoint);

                    if (targX < 1 || targX > 4) 
                    {
                        MessInfoText infoText;
                        CopyStringSafely("USAGE: /vote <bill number (1-4)> (yes OR no)", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else if (argPoint == linePoint)
                        DetailVote(targX-1, curAvatar, (TowerMap *)guildSpace);
                    else
                    {
                        if (!stricmp(&chatText[argPoint], "yes"))
                            VoteOnBill(1, targX-1, curAvatar, (TowerMap *)guildSpace);
                        else if (!stricmp(&chatText[argPoint], "no"))
                            VoteOnBill(0, targX-1, curAvatar, (TowerMap *)guildSpace);
                        else
                        {
                            MessInfoText infoText;

                            CopyStringSafely("Your vote was NOT recorded.  Please use 'yes' or 'no' to vote.", 
                                                  200, infoText.text, MESSINFOTEXTLEN);
                            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            CopyStringSafely("USAGE: /vote <bill number (1-4)> (yes OR no)", 
                                                  200, infoText.text, MESSINFOTEXTLEN);
                            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        }
                    }
                }
            }
			//***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/quest"))
            {
                argPoint = NextWord(chatText,&linePoint);

                if (argPoint == linePoint)
                {
                    MessInfoText infoText;
                    CopyStringSafely("USAGE: /quest LIST      shows what quests you have", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    CopyStringSafely("USAGE: /quest <number of quest>      shows more info", 
                                          200, infoText.text, MESSINFOTEXTLEN);
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }
                else
                {
                    if (!stricmp(&chatText[argPoint], "list"))
                    {
                        for (int i = 0; i < QUEST_SLOTS; ++i)
                        {
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_DATA;
                            sprintf(&(tempText[2]),"%d: ", i+1);

                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                         quests[i].ShortDesc(&(tempText[strlen(tempText)]));

                            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                        }
                    }
                    else if (IsSame(&chatText[argPoint], "delete"))
                    {
                        if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType ||
                             ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                        {
                            argPoint = NextWord(chatText,&linePoint);

                            int targX;
                            sscanf(&chatText[argPoint],"%d",&targX);
                            --targX;
                            if (targX >= 0 && targX < QUEST_SLOTS)
                            {
                                curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                         quests[targX].EmptyOut();

                                CopyStringSafely("Done.", 200, infoText.text, MESSINFOTEXTLEN);
                                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                            else
                            {
                                CopyStringSafely("Error: Bad quest number", 200, infoText.text, MESSINFOTEXTLEN);
                                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }
                    }
                    else
                    {
                        int targX;
                        sscanf(&chatText[argPoint],"%d",&targX);

                        if (targX > 0 && targX <= QUEST_SLOTS)
                        {
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_DATA;
                            sprintf(&(tempText[2]),"%d: ", targX);

                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                         quests[targX-1].LongDesc(&(tempText[strlen(tempText)]));

                            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                        }
                    }
                }
            }
            //***************************************
            else if ( IsSame(&(chatText[argPoint]) , "/uid") && ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
            {
                int len;
                BBOSAvatar *targetAv = FindAvatarByStringName(&(chatText[linePoint]), len, &sp);
                
                // Return the unique number
                if (targetAv)
                    sprintf(	&chatMess.text[1],
                                "%s's Unique ID is %d.",
                                targetAv->charInfoArray[targetAv->curCharacterIndex].name,
                                targetAv->GetUniqueId()	);
                else
                    sprintf(&chatMess.text[1], "That character is currently not online.");

                chatMess.text[0] = TEXT_COLOR_DATA;
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
            }
            //***************************************
            else
            {
                sprintf_s(infoText.text,"unknown / command");
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
//			  	sprintf(infoText.text,"/ commands are: shout (to visual range), announce (to everyone),");
//				ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
//			  	sprintf(infoText.text,"emote, who, whoall, where, tell, and about.");
                sprintf_s(infoText.text,"/ commands are: shout (to visual range), announce");
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                sprintf_s(infoText.text,"(to everyone), emote, who, whoall, where, tell, about,");
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                sprintf_s(infoText.text,"ignore, and friend (+ add, remove, list).");
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                sprintf_s(infoText.text,"Most can be abbreviated to just the first letter.");
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }

        }
        else if ( IsSame(&(chatText[0]) , "Caorael"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_REALM_SPIRITS;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.r = messChant.g = 255;
            messChant.b = 100;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Jinweise"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_REALM_DEAD;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.r = messChant.b = 255;
            messChant.g = 100;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Atronach"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_GROUP_PRAYER;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.r = messChant.g = 200;
            messChant.b = 255;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Danfratern"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_CREATE_GUILD;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.b = 255;
            messChant.g = 255;
            messChant.r = 0;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Dantoporas"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_PLANT_TOWER;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.b = 255;
            messChant.g = 255;
            messChant.r = 0;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Danviro"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_EDIT_TOWER;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.b = 255;
            messChant.g = 255;
            messChant.r = 0;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Danduco"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_INDUCT_MEMBER;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.b = 255;
            messChant.g = 255;
            messChant.r = 0;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
		else if (IsSame(&(chatText[0]), "Reto"))
		{
		if (curAvatar->BotTPCount == 2) // your second time since logging in
		{
			// tell them they need to move first. to beat bot tricks
			MessInfoText infoText;
			sprintf_s(tempText, "Please move a square first.");
			CopyStringSafely(tempText,
				200, infoText.text, MESSINFOTEXTLEN);
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		}
		else if (curAvatar->BotTPCount > 2)
		{
			// if you still haven't moved in normal realm, you are in big trouble
			// you are stunned, and reto won't work. you have to sign out
			curAvatar->MagicEffect(MONSTER_EFFECT_STUN, 10000000, 1000.0f);
			MessInfoText infoText;
			sprintf_s(tempText, "You feel a sense of impending doom.");
			CopyStringSafely(tempText,
				200, infoText.text, MESSINFOTEXTLEN);
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		}
		else
		{
			if (-1 == curAvatar->chantType)
			{
				sprintf(&(tempText[1]), "%s utters a Word of Power.",
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
				tempText[0] = NWMESS_PLAYER_CHAT_LINE;
				ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
					strlen(tempText) + 1, (void *)&tempText, 2);
			}
			curAvatar->chantType = CHANT_LEAVE_REALM;
			curAvatar->chantTime = timeGetTime();

			MessChant messChant;
			messChant.avatarID = curAvatar->socketIndex;
			messChant.b = 255;
			messChant.g = 255;
			messChant.r = 0;
			ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
					sizeof(messChant), &messChant);

				HandleWordOfPower(curAvatar, ss);
				}
        }
        else if ( IsSame(&(chatText[0]) , "Metamoros"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Word of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_ROP;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.b = 255;
            messChant.g = 0;
            messChant.r = 0;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else if ( IsSame(&(chatText[0]) , "Polyvacus"))
        {
            if (-1 == curAvatar->chantType)
            {
                sprintf(&(tempText[1]),"%s utters a Response of Power.",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);
            }
            curAvatar->chantType = CHANT_ROP_RESPONSE;
            curAvatar->chantTime = timeGetTime();

            MessChant messChant;
            messChant.avatarID = curAvatar->socketIndex;
            messChant.b = 0;
            messChant.g = 255;
            messChant.r = 0;
            ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, 
                                       sizeof(messChant), &messChant);

            HandleWordOfPower(curAvatar, ss);

        }
        else
        {
            if (curAvatar->BeastStat() < 10 && curAvatar->controlledMonster)
            {
                SharedSpace *sx;

                BBOSMonster * theMonster = FindMonster(
                          curAvatar->controlledMonster, &sx);
                if (theMonster)
                {
                    sprintf(&(tempText[1]),"%s says, %s",
                        theMonster->Name(),
                        &(chatText[0]));
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    sx->IgnorableSendToEveryoneNear(curAvatar, theMonster->cellX, theMonster->cellY,
                                          strlen(tempText) + 1,(void *)&tempText,2);

                    source = fopen("logs\\chatline.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    _strtime( tempText2 );
                    fprintf(source, "%s, ", tempText2 );
                    fprintf(source,"%d, %d, %s\n", theMonster->cellX, theMonster->cellY, &(tempText[1]));
                    fclose(source);
                }
            }
            else
            {
                sprintf(&(tempText[1]),"%s says, %s",
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                    &(chatText[0]));
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                ss->IgnorableSendToEveryoneNear(curAvatar, curAvatar->cellX, curAvatar->cellY,
                                      strlen(tempText) + 1,(void *)&tempText,2);

                curAvatar->QuestTalk(ss);

                source = fopen("logs\\chatline.txt","a");
                /* Display operating system-style date and time. */
                _strdate( tempText2 );
                fprintf(source, "%s, ", tempText2 );
                _strtime( tempText2 );
                fprintf(source, "%s, ", tempText2 );
                fprintf(source,"%d, %d, %s\n", curAvatar->cellX, curAvatar->cellY, &(tempText[1]));
                fclose(source);
            }
        }
    }
}


//*******************************************************************************
void BBOServer::ProcessWho(BBOSAvatar *ca, SharedSpace *ss)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;
    SharedSpace *sp;
    MessPlayerChatLine chatMess;
    int messLen = 0;
    char fullName[300];
    char fullName2[300];
    int startingLine = TRUE;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(ca->socketIndex);

    sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        curMob = (BBOSMob *) sp->avatars->First();
        while (curMob && (!ss || ss == sp))
        {
            if (SMOB_AVATAR == curMob->WhatAmI() && FALSE == ((BBOSAvatar *) curMob)->isInvisible)
            {
                curAvatar = (BBOSAvatar *) curMob;
                char *name = curAvatar->charInfoArray[curAvatar->curCharacterIndex].name;

                char paidText[16];
                if (ACCOUNT_TYPE_MODERATOR == curAvatar->accountType)
                    sprintf_s( paidText, "MOD" );
                else if( ACCOUNT_TYPE_TRIAL_MODERATOR == curAvatar->accountType )
                    sprintf_s( paidText, "TRIAL MOD" );
                else if (ACCOUNT_TYPE_ADMIN == curAvatar->accountType)
                    sprintf_s( paidText, "ADMIN" );
                else
                    sprintf_s(paidText,"");

                sprintf_s( fullName, "%s %d/%d/%d/%d %s", name, 
                                                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].physical,
                                                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].magical,
														curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative,
														curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast,
                                                        paidText );

                sprintf_s( fullName2,", %s %d/%d/%d/%d %s", name, 
                                                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].physical,
                                                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].magical,
														curAvatar->charInfoArray[curAvatar->curCharacterIndex].creative,
														curAvatar->charInfoArray[curAvatar->curCharacterIndex].beast,
                                                        paidText );

                for (int testVal = 0; testVal < 1; ++testVal)
                {
                    if (strlen(fullName2) + messLen > 75)
                    {
                        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                        messLen = 0;
                        startingLine = TRUE;
                    }
                    
                    if (startingLine)
                        sprintf(&(chatMess.text[messLen]),"%s", fullName);
                    else
                        sprintf(&(chatMess.text[messLen]),"%s", fullName2);
                    messLen = strlen(chatMess.text);
                    startingLine = FALSE;
                }

            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        sp = (SharedSpace *) spaceList->Next();
    }

    if (messLen > 0)
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

    return;
}

//*******************************************************************************
void BBOServer::HandleTeleport(BBOSAvatar *ca, BBOSMob *townmage, 
                                         SharedSpace *ss, int forward)
{
    MessInfoText infoText;
    char tempText[1024];
	
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(ca->socketIndex);

    FILE *source = fopen("teleporting.txt","a");
    
    /* Display operating system-style date and time. */
    _strdate( tempText );
    fprintf(source, "%s, ", tempText );
    _strtime( tempText );
    fprintf(source, "%s, ", tempText );

    fprintf(source,"%s, ", ca->charInfoArray[ca->curCharacterIndex].name);

	if (ss->WhatAmI()==SPACE_GUILD)
	{ 
		// it's a tree teleport from the token
		int tokx = ca->cellX; // fetch coords
		int toky = ca->cellY;
		int t = (toky) * 3 + (4 - tokx); // convert tower token coords back to magic type enum. as long as you are in the 3x3 token zone in the tower, t is right.
		if ((t >= MAGIC_MAX) || (t<0))  // error check t should always be less then MAGIC_MAX and greater than -1
		{
			sprintf_s(infoText.text, "Stop trying to HACK!"); // client sent a teleport request at a bad coordinate!
			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			return;
		}
		// else we get to go.  btu first, check for controlled monster
		if (ca->controlledMonster)
		{
			// make it go poof
			MessMonsterDeath messMD;
			messMD.mobID = (unsigned long)ca->controlledMonster;
			ss->SendToEveryoneNearBut(0, ca->controlledMonster->cellX, ca->controlledMonster->cellY,
				sizeof(messMD), (void *)&messMD);
			// remove it fropm old map
			ss->mobList->Remove(ca->controlledMonster);
		}

		// otherwise we assume that the token is actually there, and go off to the right tree.
		ca->AnnounceDisappearing(ss, SPECIAL_APP_TELEPORT_AWAY); // poof!
		ca->cellX = greatTreePos[t][0];  // and fetch x and y coords for matching tree
		ca->cellY = greatTreePos[t][1];
		// tell my client I'm leaving the tower
		MessChangeMap changeMap;
		changeMap.oldType = ss->WhatAmI(); // from tower
		changeMap.newType = SPACE_GROUND;  // to main world.
		changeMap.sizeX = MAP_SIZE_WIDTH;  
		changeMap.sizeY = MAP_SIZE_HEIGHT;
		changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
		lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);
		SharedSpace *sg = (SharedSpace *)spaceList->Find(SPACE_GROUND);
		ca->QuestSpaceChange(ss, sg);   // just in case

		// move me to my new SharedSpace
		ss->avatars->Remove(ca);
		sg->avatars->Append(ca);

		// if i have a monster
		if (ca->controlledMonster)
		{
			//move my monster too
			ca->controlledMonster->cellX = ca->controlledMonster->targetCellX = greatTreePos[t][0];
			ca->controlledMonster->cellY = ca->controlledMonster->targetCellY = greatTreePos[t][1];
			sg->mobList->Add(ca->controlledMonster);

		}
		// tell everyone about my arrival
		ca->IntroduceMyself(sg, SPECIAL_APP_DUNGEON);
		if (ca->controlledMonster)
		{
			// tell clients about my monster
			ca->controlledMonster->AnnounceMyselfCustom(sg);
		}
		// tell this player about everyone else around
		ca->UpdateClient(sg, TRUE);
		// log the use of the token teleport, since we already wrote to the log file.
		fprintf(source, "Token teleport");
		fclose(source);
	}
	else
	{
		int townIndex = -1;
		for (int t = 0; t < NUM_OF_TOWNS; ++t)
		{
			if (townList[t].x == townmage->cellX && townList[t].y + 1 == townmage->cellY)
				townIndex = t;
		}

		if (townIndex < 0)
		{
			sprintf_s(infoText.text, "Internal Error: bad town");
			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

			fprintf(source, "ERROR\n");
			fclose(source);
			return;
		}

		if (ca->charInfoArray[ca->curCharacterIndex].inventory->money < 400)
		{
			sprintf_s(infoText.text, "Sorry, you don't have enough money.");
			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

			fprintf(source, "NO FUNDS\n");
			fclose(source);
			return;
		}

		int oldTownIndex = townIndex;

		if (forward)
		{
			++townIndex;
			if (townIndex >= NUM_OF_TOWNS)
				townIndex = 0;
		}
		else
		{
			--townIndex;
			if (townIndex < 0)
				townIndex = NUM_OF_TOWNS - 1;
		}
		// gotta check for controlling monster
							// am i controlling a monster?
		if (ca->controlledMonster)
		{
			// and make my monster vanish too
			MessMonsterDeath messMD;
			messMD.mobID = (unsigned long)ca->controlledMonster;
			ss->SendToEveryoneNearBut(0, ca->controlledMonster->cellX, ca->controlledMonster->cellY,
				sizeof(messMD), (void *)&messMD);
			MessAdminMessage adminMess;
			adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL; // unconfuse client so it teleports properly
			lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
		}
		ca->AnnounceDisappearing(ss, SPECIAL_APP_TELEPORT_AWAY); // initiate teleport mode
		ca->cellX = ca->targetCellX = townList[townIndex].x;
		ca->cellY = ca->targetCellY = townList[townIndex].y;
		if (ca->controlledMonster)
		{
			//move my monster too
			ca->controlledMonster->cellX = ca->controlledMonster->targetCellX = townList[townIndex].x;
			ca->controlledMonster->cellY = ca->controlledMonster->targetCellY = townList[townIndex].y;

		}
		// tell everyone about my arrival
		ca->IntroduceMyself(ss, SPECIAL_APP_TELEPORT);
		ca->UpdateClient(ss, TRUE); // do BEFORE the monster is mentioned.
		if (ca->controlledMonster)
		{
			// tell clients about my monster
			ca->controlledMonster->AnnounceMyselfCustom(ss);
		}
		// that was annoying.
		ca->charInfoArray[ca->curCharacterIndex].inventory->money -= 400;

		sprintf_s(infoText.text, "You have been teleported to %s.", townList[townIndex].name);
		ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

		fprintf(source, "MOVED, %s, %s\n", townList[oldTownIndex].name, townList[townIndex].name);

		fclose(source);

		ca->charInfoArray[ca->curCharacterIndex].spawnX = townList[townIndex].x;
		ca->charInfoArray[ca->curCharacterIndex].spawnY = townList[townIndex].y;
	}
}


//*******************************************************************************
void BBOServer::HandleKickoff(BBOSAvatar *ca, SharedSpace *sp)
{
    MessInfoText infoText;
//	char tempText[1024];

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(ca->socketIndex);

    // boot this sloth!
    ca->AnnounceDisappearing(sp, SPECIAL_APP_NOTHING);

    MessBoot boot;
    sp->lserver->SendMsg(sizeof(boot),(void *)&boot, 0, &tempReceiptList);

    ca->QuestSpaceChange(sp, NULL);

    sp->avatars->Remove(ca);
    if ((SPACE_GROUND != sp->WhatAmI()) || //not in normal realm
		((SPACE_GROUND==sp->WhatAmI()) && (((GroundMap *)sp)->type>0)))	// or in the alternate big map
    {
        ca->charInfoArray[ca->curCharacterIndex].lastX = 
            ca->charInfoArray[ca->curCharacterIndex].spawnX;
        ca->charInfoArray[ca->curCharacterIndex].lastY = 
            ca->charInfoArray[ca->curCharacterIndex].spawnY;
    }
    else
    {
        ca->charInfoArray[ca->curCharacterIndex].lastX = 
            ca->cellX;
        ca->charInfoArray[ca->curCharacterIndex].lastY = 
            ca->cellY;
    }
    ca->SaveAccount();
	if (ca->controlledMonster) // if there'a  a controlled monster
	{ // remove it
							// die with no loot for anyone!
		for (int i = 0; i < 10; ++i)
			ca->controlledMonster->attackerPtrList[i] = NULL;
		ca->controlledMonster->dropAmount = 0; // dont' drop anything!
		ca->controlledMonster->health = 0;
		ca->controlledMonster->type = 25; //make it a vamp 
		ca->controlledMonster->isDead = TRUE; // it's dead
		ca->controlledMonster->controllingAvatar = NULL;	// and unlink it
		ca->controlledMonster = NULL;	// and unlink it
	}

    delete ca;
}



//*******************************************************************************
void BBOServer::HandleTreeTalk(BBOSAvatar *curAvatar, SharedSpace *sp, MessTalkToTree *mt)
{
    MessInfoText infoText;
    char tempText[1024];

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

//	curAvatar = FindAvatar(fromSocket, &ss);

//	if	(curAvatar)
    {
        BBOSMob *curMob = (BBOSMob *) sp->mobList->GetFirst(curAvatar->cellX, curAvatar->cellY);
        while (curMob)
        {
            if (SMOB_TREE == curMob->WhatAmI() && 
                 curMob->cellX == curAvatar->cellX &&
                 curMob->cellY == curAvatar->cellY)
            {
                BBOSTree *t = (BBOSTree *) curMob;

                if (ResolveTreeQuests(curAvatar, sp, t))
                    return;

                if (0 == mt->which)
                {
                    FILE *fp = fopen("serverdata\\treeText.txt","r");
                    if (fp)
                    {
                        int done = FALSE;
                        int found = FALSE;
                        while (!done)
                        {
                            LoadLineToString(fp, tempText);
                            if (!found)
                            {
                                if (IsSame(tempText, "TREE"))
                                {
                                    if (IsSame(&(tempText[5]), magicNameList[t->index]))
                                    {
                                        found = TRUE;
                                    }
                                }
                            }
                            else
                            {
                                if (IsSame(tempText, "TREE"))
                                    done = TRUE;
                                else
                                {
                                    // send out this line in pieces
                                    int pos = 0;
                                    int size = strlen(&(tempText[pos]));
                                    while (size > 0)
                                    {
                                        int back = pos + MESSINFOTEXTLEN-2;
                                        if (pos + size < back)
                                            back = pos + size;
                                        while (' ' != tempText[back] && 0 != tempText[back] && back-1 > pos)
                                            --back;
                                        memcpy(infoText.text, &(tempText[pos]), MESSINFOTEXTLEN-2);
                                        infoText.text[back-pos] = 0;
                                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                        pos = back+1;
                                        size = strlen(&(tempText[pos]));
                                    }
                                }
                            }
                        }

                        fclose(fp);
                    }
                }
                else if (1 == mt->which)
//					&&
//						    (ACCOUNT_TYPE_ADMIN     == curAvatar->accountType || 
//							  ACCOUNT_TYPE_MODERATOR == curAvatar->accountType)
//						  )
                {
                    int freeSlot = -1;
                    for (int i = 0; i < QUEST_SLOTS; ++i)
                    {
                        if (-1 == curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                          quests[i].completeVal)
                        {
                            freeSlot = i;
                            i = QUEST_SLOTS;
                        }
                    }

                    if (-1 == freeSlot)
                    {
                        sprintf_s(infoText.text,"You are already burdened with enough tasks.");
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                    else
                    {
                        CreateTreeQuest(&(curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                          quests[freeSlot]), curAvatar, sp, t);
                    }
                }
                else if (1 == mt->which)
                {
                    sprintf_s(infoText.text,"Soon, I will be able to give you tasks for my Mistress.");
                    lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                }

                sp->mobList->SetToLast();
            }
            else if (SMOB_WITCH == curMob->WhatAmI() && 
                 curMob->cellX == curAvatar->cellX &&
                 curMob->cellY == curAvatar->cellY)
            {
                BBOSNpc *npc = (BBOSNpc *) curMob;
                if (2 == mt->which)
                {
                    // witch talks
                    questMan->WitchTalk(npc, sp);
                }
                if (3 == mt->which)
                {
                    // witch gives quest
                    questMan->WitchGiveQuest(npc, curAvatar, sp);
                }

            }
            else if (SMOB_TOWNMAGE == curMob->WhatAmI() && 
                 curMob->cellX == curAvatar->cellX &&
                 curMob->cellY == curAvatar->cellY)
            {
                BBOSNpc *npc = (BBOSNpc *) curMob;
                if (5 == mt->which)
                {
                    FILE *fp = fopen("serverdata\\treeText.txt","r");
                    if (fp)
                    {
                        int done = FALSE;
                        int found = FALSE;
                        char tmText[64];
                        sprintf_s(tmText,"TOWNMAGE%d", (rand() % 4) + 1);
                        while (!done)
                        {
                            LoadLineToString(fp, tempText);
                            if (!found)
                            {
                                if (IsSame(tempText, "TREE"))
                                {
                                    if (IsSame(&(tempText[5]), tmText))
                                    {
                                        found = TRUE;
                                    }
                                }
                            }
                            else
                            {
                                if (IsSame(tempText, "TREE"))
                                    done = TRUE;
                                else
                                {
                                    // send out this line in pieces
                                    int pos = 0;
                                    int size = strlen(&(tempText[pos]));
                                    while (size > 0)
                                    {
                                        int back = pos + MESSINFOTEXTLEN-2;
                                        if (pos + size < back)
                                            back = pos + size;
                                        while (' ' != tempText[back] && 0 != tempText[back] && back-1 > pos)
                                            --back;
                                        memcpy(infoText.text, &(tempText[pos]), MESSINFOTEXTLEN-2);
                                        infoText.text[back-pos] = 0;
                                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                        pos = back+1;
                                        size = strlen(&(tempText[pos]));
                                    }
                                }
                            }
                        }

                        fclose(fp);
                    }
                }

            }
            curMob = (BBOSMob *) sp->mobList->GetNext();
        }
    }
}


//*******************************************************************************
void BBOServer::HandleWordOfPower(BBOSAvatar *ca, SharedSpace *sp) 
{
    MessInfoText infoText;
    char tempText[1024];

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(ca->socketIndex);

    int num = 0;
    int x = ca->cellX;
    int y = ca->cellY;
    int chantType = ca->chantType;
	int uidcount = 0;
	if ((SPACE_REALM == sp->WhatAmI() || // in  a realm
          (SPACE_DUNGEON == sp->WhatAmI() && 
           SPECIAL_DUNGEON_TEMPORARY &((DungeonMap *)sp)->specialFlags) //or in a geo
		|| ((SPACE_GROUND == sp->WhatAmI()) && (((GroundMap*)sp)->type >0)) // OR in alternate normal realm
         )&& CHANT_LEAVE_REALM == chantType)
    {
		SharedSpace *sg = (SharedSpace *)spaceList->Find(SPACE_GROUND);

		if (ca->controlledMonster && !ca->followMode)
		{
			MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
			tempReceiptList.clear();
			tempReceiptList.push_back(ca->socketIndex); // point server at the target client.
			adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
			lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
			// and return to follow mode, since both will end up in the same spot.
			if (ca->BeastStat() > 9)
				ca->followMode = TRUE;
		}
		// tell everyone I'm dissappearing
		ca->AnnounceDisappearing(sp, SPECIAL_APP_DUNGEON);

		if (ca->controlledMonster)
		{
			// we need to dissapear it from people's screen.
			MessMonsterDeath messMD;
			messMD.mobID = (unsigned long)ca->controlledMonster;
			sp->SendToEveryoneNearBut(0, ca->controlledMonster->cellX, ca->controlledMonster->cellY,
				sizeof(messMD), (void *)&messMD);
			// remove it fropm old map
			sp->mobList->Remove(ca->controlledMonster);

		}
		// tell my client I'm leaving the dungeon
		tempReceiptList.clear();
		tempReceiptList.push_back(ca->socketIndex); // point server at the target client.
		sp->avatars->Remove(ca);

		ca->cellX = ca->targetCellX =
			((BBOSAvatar *)ca)->charInfoArray[((BBOSAvatar *)ca)->
			curCharacterIndex].spawnX;
		ca->cellY = ca->targetCellY =
			((BBOSAvatar *)ca)->charInfoArray[((BBOSAvatar *)ca)->
			curCharacterIndex].spawnY;    
		sg->avatars->Append(ca);
		if (ca->controlledMonster) // if they had a monster
		{
			// then it needs ot have it's coords updated, and be added to new map
			ca->controlledMonster->cellX = ca->controlledMonster->targetCellX = ca->cellX;
			ca->controlledMonster->cellY = ca->controlledMonster->targetCellY = ca->cellY;
			sg->mobList->Add(ca->controlledMonster);
		}
		MessChangeMap changeMap;

        // move me to my new SharedSpace
		changeMap.oldType = sp->WhatAmI();
		changeMap.newType = SPACE_GROUND;
		changeMap.sizeX = MAP_SIZE_WIDTH;
		changeMap.sizeY = MAP_SIZE_HEIGHT;
		changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
//		if ((SPACE_GROUND == sp->WhatAmI()) && (((GroundMap*)sp)->type > 0))
		changeMap.realmID = 0; // always send realm id when returning so we get town sparkles.
		lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);
		ca->QuestSpaceChange(sp, sg);

        // tell everyone about my arrival
        ca->IntroduceMyself(sg, SPECIAL_APP_DUNGEON);
		if (ca->controlledMonster)  // if we had a monster, spawn it again like i logged in. 
		{
				ca->controlledMonster->AnnounceMyselfCustom(sg);
		}
		// tell this player about everyone else around
        ca->UpdateClient(sg, TRUE);

        MessInfoText infoText;
        CopyStringSafely("You leave the Realm.", 
                              200, infoText.text, MESSINFOTEXTLEN);
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        return;
    }

    for (int g = 0; g < NUM_OF_TOWNS && SPACE_GROUND == sp->WhatAmI(); ++g)
    {
        if (x == townList[g].x && y == townList[g].y)
            return;
    }

    BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())
        {
            BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
            if (ca2->cellX == x && ca2->cellY == y &&
                 ca2->chantType > -1 && ca2->chantType == chantType)
            {
                ++num;
            }
        }
        curMob = (BBOSMob *) sp->avatars->Next();
    }

    if (num > 2 && CHANT_REALM_SPIRITS == chantType)
    {
		HandleSpiritGate(ca->cellX, ca->cellY, sp); // we don't need the routine below at all when the other one will drag your controller mob along fine.
													// save code. 
 /*       BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
          while (curMob)
          {
              if (SMOB_AVATAR == curMob->WhatAmI())
              {
                  BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
                  if (ca2->cellX == x && ca2->cellY == y)
                  {
  
                      RealmMap *rp = NULL;
                      SharedSpace *sp2 = (SharedSpace *) spaceList->First();
                      while (sp2)
                    {
                        if (SPACE_GROUND == sp->WhatAmI() && SPACE_REALM == sp2->WhatAmI() && 
                             REALM_ID_SPIRITS == ((RealmMap *)sp2)->type)
                        {
                            rp = (RealmMap *)sp2;
                            sp2 = (SharedSpace *) spaceList->Last();
                        }
                        sp2 = (SharedSpace *) spaceList->Next();
                    }

                    if (rp)
                    {
                        ca2->AnnounceDisappearing(sp, SPECIAL_APP_DUNGEON);

                        // tell my client I'm entering the realm
                        tempReceiptList.clear();
                        tempReceiptList.push_back(ca2->socketIndex);

                        MessChangeMap changeMap;
                        changeMap.realmID = rp->type;
                        changeMap.oldType = sp->WhatAmI(); 
                        changeMap.newType = rp->WhatAmI(); 
                        changeMap.sizeX   = rp->sizeX;
                        changeMap.sizeY   = rp->sizeY;
						changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
                        lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                        MessInfoText infoText;
                        CopyStringSafely("You enter the Realm of Spirits.", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        ca2->QuestSpaceChange(sp,rp);

                        // move me to my new SharedSpace
                        sp->avatars->Remove(ca2);
                        rp->avatars->Append(ca2);

                        ca2->cellX = ca2->targetCellX = (rand() % 4) + 2;
                        ca2->cellY = ca2->targetCellY = (rand() % 4) + 2;

                        // tell everyone about my arrival
                        ca2->IntroduceMyself(rp, SPECIAL_APP_DUNGEON);

                        // tell this player about everyone else around
                        ca2->UpdateClient(rp, TRUE);
                    }

                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }
		*/
        ca = (BBOSAvatar *)sp->avatars->First();

    } 
    else if (num > 2 && CHANT_REALM_DEAD == chantType)
    {
		HandleDeadGate(ca->cellX, ca->cellY, sp); // transport the square, just as if we had killed a gateway monster in it.
												  // but monsters won't get picked up unless they are owned by a player in square.
 /*       BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
                if (ca2->cellX == x && ca2->cellY == y)
                {

                    RealmMap *rp = NULL;
                    SharedSpace *sp2 = (SharedSpace *) spaceList->First();
                    while (sp2)
                    {
                        if (SPACE_GROUND == sp->WhatAmI() && SPACE_REALM == sp2->WhatAmI() && 
                             REALM_ID_DEAD == ((RealmMap *)sp2)->type)
                        {
                            rp = (RealmMap *)sp2;
                            sp2 = (SharedSpace *) spaceList->Last();
                        }
                        sp2 = (SharedSpace *) spaceList->Next();
                    }

                    if (rp)
                    {
                        ca2->AnnounceDisappearing(sp, SPECIAL_APP_DUNGEON);

                        // tell my client I'm entering the realm
                        tempReceiptList.clear();
                        tempReceiptList.push_back(ca2->socketIndex);

                        MessChangeMap changeMap;
                        changeMap.realmID = rp->type;
                        changeMap.oldType = sp->WhatAmI(); 
                        changeMap.newType = rp->WhatAmI(); 
                        changeMap.sizeX   = rp->sizeX;
                        changeMap.sizeY   = rp->sizeY;
						changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
                        lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

                        MessInfoText infoText;
                        CopyStringSafely("You enter the Realm of the Dead.", 
                                              200, infoText.text, MESSINFOTEXTLEN);
                        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        ca2->QuestSpaceChange(sp,rp);

                        // move me to my new SharedSpace
                        sp->avatars->Remove(ca2);
                        rp->avatars->Append(ca2);

                        ca2->cellX = ca2->targetCellX = (rand() % 4) + 2;
                        ca2->cellY = ca2->targetCellY = (rand() % 4) + 2;

                        // tell everyone about my arrival
                        ca2->IntroduceMyself(rp, SPECIAL_APP_DUNGEON);

                        // tell this player about everyone else around
                        ca2->UpdateClient(rp, TRUE);
                    }
                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }
		*/
        ca = (BBOSAvatar *)sp->avatars->First();

    }
    else if (num > 2 && CHANT_GROUP_PRAYER == chantType)
    {
        BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
				
                if (ca2->cellX == x && ca2->cellY == y)
                {
                    ca2->QuestPrayer(sp);
                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        ca = (BBOSAvatar *)sp->avatars->First();

    }
    else if (num > 3 && CHANT_CREATE_GUILD == chantType)
    {
//		return;
        // does anyone already belong to a guild?
        BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

                SharedSpace *sx;
                if (ca2->cellX == x && ca2->cellY == y && 
                     FindAvatarInGuild(ca2->charInfoArray[ca2->curCharacterIndex].name, &sx))
                {
                    sprintf(&(tempText[1]),"%s is already a member of a guild.",
                        ca2->charInfoArray[ca2->curCharacterIndex].name);
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
                                       strlen(tempText) + 1,(void *)&tempText,2);
                    return;
                }
				if (ca2->cellX == x && ca2->cellY == y && (ca->GetUniqueId() == ca2->GetUniqueId()))
					uidcount++;
				if ((uidcount > 1) && (enable_cheating==0)) // allow multibox guilding if cheat mode is on.
				{
					sprintf(&(tempText[1]), "%s is on the same computer as someone else.",
						ca->charInfoArray[ca->curCharacterIndex].name);
					tempText[0] = NWMESS_PLAYER_CHAT_LINE;
					sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
						strlen(tempText) + 1, (void *)&tempText, 2);
					return;

				}
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        // make a new guild
        TowerMap *dm = new TowerMap(SPACE_GUILD,"UNNAMED GUILD",lserver);
        dm->InitNew(5,5);
        spaceList->Append(dm);

//		BBOSTower *tower = new BBOSTower(dm->enterX, dm->enterY);
//		sp->mobList->Add(tower);
//		tower->ss = dm;
//		tower->isGuildTower = TRUE;

        // and add everyone to it!
        curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

                if (ca2->cellX == x && ca2->cellY == y)
                {
                    MemberRecord *mr = new MemberRecord(0,
                               ca2->charInfoArray[ca2->curCharacterIndex].name);
                    dm->members->Append(mr);

                    sprintf(&(tempText[1]),"%s is a member of the new guild.",
                        ca2->charInfoArray[ca2->curCharacterIndex].name);
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
                                       strlen(tempText) + 1,(void *)&tempText,2);
                }
            }
            sp->avatars->Find(curMob);
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        dungeonUpdateTime = 0; // save off dungeons and guilds

        ca = (BBOSAvatar *)sp->avatars->First();

    }
    else if (num > 3 && CHANT_PLANT_TOWER == chantType)
    {
//		return;
        // does anyone NOT belong to this guild?
        SharedSpace *sxNorm = NULL;
        int adminHelp = FALSE;
        BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

                SharedSpace *sx;
                if (ca2->cellX == x && ca2->cellY == y && ca2->chantType == chantType && 
                     FindAvatarInGuild(ca2->charInfoArray[ca2->curCharacterIndex].name, &sx))
                {
                    if (sxNorm && sxNorm != sx)
                    {
                        sprintf(&(tempText[1]),"%s is not a member of the guild.",
                            ca2->charInfoArray[ca2->curCharacterIndex].name);
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
                                           strlen(tempText) + 1,(void *)&tempText,2);
                        return;
                    }
                    else
                    {
                        sxNorm = sx;
                        if (ACCOUNT_TYPE_ADMIN == ca2->accountType || 
                             ACCOUNT_TYPE_MODERATOR == ca2->accountType ||
                             ACCOUNT_TYPE_TRIAL_MODERATOR == ca2->accountType)
                            adminHelp = TRUE;
                    }
                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }
		if (enable_cheating != 0)   // on cheaters edition
			adminHelp = TRUE;		// no admin needed to place tower.
		// move the guild tower
        if (!sxNorm || !adminHelp)
            return;

        TowerMap *dm = (TowerMap *) sxNorm;
        dm->enterX = x;
        dm->enterY = y+1; // south of here
		// check for nearby other towers and stuff to make sure playyers can't coat the entire map with towers
		BBOSMob *curMobbadplace = (BBOSMob *)sp->mobList->GetFirst(0, 0, 1000);
		while (curMobbadplace)
		{
			// first check it's type
			if (curMobbadplace->WhatAmI() == SMOB_TOWER || curMobbadplace->WhatAmI() == SMOB_TREE || curMobbadplace->WhatAmI() == SMOB_WARP_POINT
				|| curMobbadplace->WhatAmI() == SMOB_ROPENTERANCE) // only cetain mob types are included in this check
				if (abs(dm->enterX - curMobbadplace->cellX) < 5 && abs(dm->enterY - curMobbadplace->cellY) < 5)
				{
					return;           // give up on making the tower
				}
			curMobbadplace = sp->mobList->GetNext(); // go onto the next mob
		}

        // find the visible tower
        BBOSTower *towerMob = NULL;
        curMob = (BBOSMob *) sp->mobList->GetFirst(0,0,1000);
        while (curMob)
        {
            if (SMOB_TOWER == curMob->WhatAmI() && ((BBOSTower *)curMob)->isGuildTower &&
                 ((BBOSTower *)curMob)->ss == dm)
            {
                towerMob = (BBOSTower *)curMob;
                sp->mobList->SetToLast();
            }
            curMob = (BBOSMob *) sp->mobList->GetNext();
        }

        if (!towerMob)
        {
            BBOSTower *tower = new BBOSTower(x,y+1);
            sp->mobList->Add(tower);
            tower->ss = dm;
            tower->isGuildTower = TRUE;
            towerMob = tower;

            BBOSChest *chest = new BBOSChest(2,2);
            dm->mobList->Add(chest);

            BBOSWarpPoint *wp = new BBOSWarpPoint(0,0);
            wp->targetX      = dm->enterX;
            wp->targetY      = dm->enterY-1;
            wp->spaceType    = SPACE_GROUND;
            wp->spaceSubType = 0;
            dm->mobList->Add(wp);

        }

        // move the visible tower
        if (towerMob)
        {
            towerMob->cellX = x;
            towerMob->cellY = y+1;
            ((TowerMap *)towerMob->ss)->enterX = x;
            ((TowerMap *)towerMob->ss)->enterY = y+1;

            BBOSWarpPoint *wp = (BBOSWarpPoint *)dm->mobList->GetFirst(0,0);
            while (wp)
            {
                if (SMOB_WARP_POINT == wp->WhatAmI())
                {
                    wp->targetX = ((TowerMap *)towerMob->ss)->enterX; 
                    wp->targetY = ((TowerMap *)towerMob->ss)->enterY-1;
                }

                wp = (BBOSWarpPoint *)dm->mobList->GetNext();
            }
            
            sp->mobList->Remove(towerMob);
            sp->mobList->Add(towerMob);

            MessMobAppear mobAppear;
            mobAppear.mobID = (unsigned long) towerMob;
            mobAppear.type  = towerMob->WhatAmI();
            mobAppear.x     = towerMob->cellX;
            mobAppear.y     = towerMob->cellY;
            sp->SendToEveryoneNearBut(0, towerMob->cellX, towerMob->cellY,
                           sizeof(mobAppear),(void *)&mobAppear);

            MessCaveInfo cInfo;
            cInfo.mobID       = (long) towerMob;
            cInfo.hasMistress = FALSE;
            cInfo.type        = -1;
            sp->SendToEveryoneNearBut(0, towerMob->cellX, towerMob->cellY,
                           sizeof(cInfo),(void *)&cInfo);

            MessGenericEffect messGE;
            messGE.avatarID = -1;
            messGE.mobID    = (long)towerMob;
            messGE.x        = towerMob->cellX;
            messGE.y        = towerMob->cellY;
            messGE.r        = 0;
            messGE.g        = 255;
            messGE.b        = 255;
            messGE.type     = 0;  // type of particles
            messGE.timeLen  = 20; // in seconds
            sp->SendToEveryoneNearBut(0, towerMob->cellX, towerMob->cellY,
                              sizeof(messGE),(void *)&messGE);

        }

        dungeonUpdateTime = 0; // save off dungeons and guilds

        ca = (BBOSAvatar *)sp->avatars->First();

    }
    else if ( num > 3 && CHANT_INDUCT_MEMBER == chantType )
    {
//		return;
        // does anyone NOT belong to this guild?
        SharedSpace *sxNorm = NULL;
        BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
        int ranking_members = 0;
        int loc_x, loc_y = 0;
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

                SharedSpace *sx;
                loc_x = ca2->cellX;
                loc_y = ca2->cellY;

                if (ca2->cellX == x && ca2->cellY == y && ca2->chantType == chantType && 
                     FindAvatarInGuild(ca2->charInfoArray[ca2->curCharacterIndex].name, &sx))
                {
                    if (sxNorm && sxNorm != sx)
                    {
                        sprintf(&(tempText[1]),"%s is not a member of the guild.",
                            ca2->charInfoArray[ca2->curCharacterIndex].name);
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
                                           strlen(tempText) + 1,(void *)&tempText,2);
                        return;
                    }
                    else
                    {
                        if( GetRecordForGuildMember( ca2->charInfoArray[ca2->curCharacterIndex].name, &sx )->value1 > 0 || 
                            ca2->accountType == ACCOUNT_TYPE_ADMIN )
                        {
                            ranking_members += 1;
                            sxNorm = sx;
                        }
                    }
                }
                else if (ca2->cellX == x && ca2->cellY == y && -1 == ca2->chantType && 
                     FindAvatarInGuild(ca2->charInfoArray[ca2->curCharacterIndex].name, &sx))
                {
                    if (sxNorm && sxNorm != sx)
                    {
                        sprintf(&(tempText[1]),"%s is already a member of another guild.",
                            ca2->charInfoArray[ca2->curCharacterIndex].name);
                        tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                        sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
                                           strlen(tempText) + 1,(void *)&tempText,2);
                        return;
                    }
                    else
                        sxNorm = sx;
                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        if( ranking_members == 0 )
        {
            sprintf( &(tempText[1] ),"You must have a ranking member of the guild present to induct new members." );
            tempText[0] = TEXT_COLOR_DATA;
            sp->SendToEveryoneNearBut(0, x, y, strlen(tempText) + 1,(void *)&tempText,2 );

            return;
        }
        if (!sxNorm)
            return;

        TowerMap *dm = (TowerMap *) sxNorm;

        if (num <= 3 && dm->members->ItemsInList() > num)
            return;

        // and add everyone new to it!
        curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

                if (ca2->cellX == x && ca2->cellY == y && 
                     !dm->IsMember(ca2->charInfoArray[ca2->curCharacterIndex].name))
                {
                    MemberRecord *mr = new MemberRecord(0,
                               ca2->charInfoArray[ca2->curCharacterIndex].name);
                    dm->members->Append(mr);

                    dm->lastChangedTime.SetToNow();

                    sprintf(&(tempText[1]),"%s is now a member of %s.",
                        ca2->charInfoArray[ca2->curCharacterIndex].name,
                        dm->WhoAmI());
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY,
                                       strlen(tempText) + 1,(void *)&tempText,2);

                    ca2->AssertGuildStatus(sp,TRUE);

                }
            }
            sp->avatars->Find(curMob);
            curMob = (BBOSMob *) sp->avatars->Next();
        }


        dungeonUpdateTime = 0; // save off dungeons and guilds

        ca = (BBOSAvatar *)sp->avatars->First();

    }
//	else if (num > 0 && CHANT_EDIT_TOWER == chantType)
    else if (num > 3 && CHANT_EDIT_TOWER == chantType)
    {
        if (SPACE_GUILD == sp->WhatAmI())
        {
            BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
            while (curMob)
            {
                if (SMOB_AVATAR == curMob->WhatAmI())
                {
                    BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

//					SharedSpace *sx;
                    if (ca2->cellX == x && ca2->cellY == y && ca2->chantType == chantType)
                    {
                        AvatarGuildEdit age;
                        age.avatarID = ca2->socketIndex;

                        tempReceiptList.clear();
                        tempReceiptList.push_back(ca2->socketIndex);

                        lserver->SendMsg(sizeof(age),(void *)&age, 0, &tempReceiptList);
                    }
                }
                curMob = (BBOSMob *) sp->avatars->Next();
            }
        }

        ca = (BBOSAvatar *)sp->avatars->First();

    }
    else if (CHANT_ROP_RESPONSE == chantType)
    {
        
        if (SPACE_GROUND == sp->WhatAmI())
        {
            BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
            while (curMob)
            {
                if (SMOB_AVATAR == curMob->WhatAmI())
                {
                    BBOSAvatar *ca2 = (BBOSAvatar *) curMob;

                    int inPlace = FALSE;
                    if (230 == ca2->cellX &&  53 == ca2->cellY)
                        inPlace = TRUE;
                    if ( 20 == ca2->cellX &&  23 == ca2->cellY)
                        inPlace = TRUE;
                    if (179 == ca2->cellX && 164 == ca2->cellY)
                        inPlace = TRUE;
                    if (107 == ca2->cellX && 162 == ca2->cellY)
                        inPlace = TRUE;

                    // *** temporary, so mods can test this first
//					if (ACCOUNT_TYPE_ADMIN != ca2->accountType &&
//					    ACCOUNT_TYPE_MODERATOR != ca2->accountType)
//						 inPlace = FALSE;

                    if (inPlace &&
                         abs(ca2->cellX-x) < 2 && abs(ca2->cellY-y) < 2 && 
                         ca2->chantType == CHANT_ROP && CanHaveROP(
                            ca2->charInfoArray[ca2->curCharacterIndex].age, 
                             ca2->charInfoArray[ca2->curCharacterIndex].cLevel))
                    {

                        // cut short the ROP's chant
                        ca2->chantType = -1;

                        MessChant messChant;
                        messChant.avatarID = ca2->socketIndex;
                        messChant.r = messChant.b = messChant.g = 0;	 // black means stop
                        sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY, 
                                                   sizeof(messChant), &messChant);

                        // change the ROP person
                        tempReceiptList.clear();
                        tempReceiptList.push_back(ca2->socketIndex);

                        ca2->charInfoArray[ca2->curCharacterIndex].age += 1;

                        ca2->charInfoArray[ca2->curCharacterIndex].healthMax = 20 + ca2->PhysicalStat() * 6 + ca2->charInfoArray[ca2->curCharacterIndex].cLevel; 


                        if( ca2->charInfoArray[ca2->curCharacterIndex].age < 6 )
                            ca2->charInfoArray[ca2->curCharacterIndex].healthMax *= ca2->charInfoArray[ca2->curCharacterIndex].age;
                        else
                        {
                            ca2->charInfoArray[ca2->curCharacterIndex].healthMax *= 5;

                            ca2->charInfoArray[ca2->curCharacterIndex].physical += 1;
                            ca2->charInfoArray[ca2->curCharacterIndex].creative += 1;
							ca2->charInfoArray[ca2->curCharacterIndex].magical += 1;
							ca2->charInfoArray[ca2->curCharacterIndex].beast += 1;
						}

                        MessAvatarStats mStats;
                        ca2->BuildStatsMessage(&mStats);
                        sp->lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);

                        sprintf_s(tempText,"You have passed into a new age, that of %s.", 
                                      ageTextArray[ca2->charInfoArray[ca2->curCharacterIndex].age-1] );
                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                        sp->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        MessAvatarHealth messHealth;
                        messHealth.avatarID  = ca2->socketIndex;
                        messHealth.health    = ca2->charInfoArray[ca2->curCharacterIndex].health;
                        messHealth.healthMax = ca2->charInfoArray[ca2->curCharacterIndex].healthMax;
                        sp->SendToEveryoneNearBut(0, ca2->cellX, ca2->cellY, sizeof(messHealth), &messHealth);

                        MessGenericEffect messGE;
                        messGE.avatarID = -1;
                        messGE.mobID    = -1;
                        messGE.r        = 40;
                        messGE.g        = 40;
                        messGE.b        = 255;
                        messGE.type     = 0;  // type of particles
                        messGE.timeLen  = 5; // in seconds

                        for (int ly = ca2->cellY-1; ly <= ca2->cellY+1; ++ly)
                        {
                            for (int lx = ca2->cellX-1; lx <= ca2->cellX+1; ++lx)
                            {
                                messGE.x        = lx;
                                messGE.y        = ly;
                                sp->SendToEveryoneNearBut(0, messGE.x, messGE.y, sizeof(messGE),(void *)&messGE);
                            }
                        }

                        // give everyone a goodie
                        BBOSMob *curMob2 = (BBOSMob *) sp->avatars->First();
                        while (curMob2)
                        {
                            if (SMOB_AVATAR == curMob2->WhatAmI())
                            {
                                BBOSAvatar *ca3 = (BBOSAvatar *) curMob2;
                                if (ca2 != ca3 && 
                                     abs(ca2->cellX - ca3->cellX) < 2 && 
                                     abs(ca2->cellY - ca3->cellY) < 2)
                                {
                                    InventoryObject *iObject;

                                    switch(rand() % 3)
                                    {
                                    case 0:
                                        iObject = new InventoryObject(INVOBJ_SIMPLE,0,"Ancient Dragonscale");
                                        iObject->value  = 100000;
                                        break;

                                    case 1:
                                        iObject = new InventoryObject(INVOBJ_SIMPLE,0,"Dragon Orchid");
                                        iObject->value  = 10000;
                                        break;

                                    case 2:
                                        iObject = new InventoryObject(INVOBJ_SIMPLE,0,"Demon Amulet");
                                        iObject->value  = 50000;
                                        break;
                                    }

                                    iObject->amount = 1;
                                    ca3->charInfoArray[ca3->curCharacterIndex].inventory->AddItemSorted(iObject);
                                }

                            }
                            curMob2 = (BBOSMob *) sp->avatars->Next();
                        }

                        // reset the list
                        sp->avatars->Find(curMob);
                    }
                }
                curMob = (BBOSMob *) sp->avatars->Next();
            }
        }
        
        ca = (BBOSAvatar *)sp->avatars->First();

    }


    if (ca)
        sp->avatars->Find(ca);
}


//*******************************************************************************
void BBOServer::HandleDeadGate(int x, int y, SharedSpace *sp) 
{
    MessInfoText infoText;
//	char tempText[1024];

    std::vector<TagID> tempReceiptList;
//	tempReceiptList.clear();
//	tempReceiptList.push_back(ca->socketIndex);
	if ((sp->WhatAmI() == SPACE_REALM) || (REALM_ID_DEAD == ((RealmMap *)sp)->type)) // if already in the realm of the dead
	{
		return;
	}

    BBOSMob *curMob = (BBOSMob *) sp->avatars->First();  // check all of the avatars.
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())  // should always be true
        {
            BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
            if (ca2->cellX == x && ca2->cellY == y)
            {

                RealmMap *rp = NULL;
                SharedSpace *sp2 = (SharedSpace *) spaceList->First();
                while (sp2)
                {
                    if (SPACE_REALM == sp2->WhatAmI() && REALM_ID_DEAD == ((RealmMap *)sp2)->type)
                    {
                        rp = (RealmMap *)sp2;
                        sp2 = (SharedSpace *) spaceList->Last();
                    }
                    sp2 = (SharedSpace *) spaceList->Next();
                }

                if (rp)
				{
					// gotta check for controlling monster
							// am i controlling a monster?
					if (ca2->controlledMonster && !ca2->followMode)
					{
						// and make my monster vanish too
						MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
						tempReceiptList.clear();
						tempReceiptList.push_back(ca2->socketIndex); // point server at the target client.
						adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						// and resturn server to follow mode, since both will end up in the same spot.
						if (ca2->BeastStat() > 9)
							ca2->followMode = TRUE;
						// we need to dissapear it from people's screen.
					}
					// second, vanish me
					ca2->AnnounceDisappearing(sp, SPECIAL_APP_DUNGEON);
					// third, vanish the controlled monster if present
					if (ca2->controlledMonster)
					{
						// we need to dissapear it from people's screen.
						MessMonsterDeath messMD;
						messMD.mobID = (unsigned long)ca2->controlledMonster;
						sp->SendToEveryoneNearBut(0, ca2->controlledMonster->cellX, ca2->controlledMonster->cellY,
							sizeof(messMD), (void *)&messMD);
						// remove it fropm old map
						sp->mobList->Remove(ca2->controlledMonster);

					}

					// tell my client I'm entering the realm
					tempReceiptList.clear();
					tempReceiptList.push_back(ca2->socketIndex);

					// next adjust the coordinates
					sp->avatars->Remove(ca2);
					ca2->cellX = ca2->targetCellX = 4;
					ca2->cellY = ca2->targetCellY = 4;
					rp->avatars->Append(ca2);

					if (ca2->controlledMonster) // if they had a monster
					{
						// then it needs ot have it's coords updated, and be added to new map
						ca2->controlledMonster->cellX = ca2->controlledMonster->targetCellX = ca2->cellX;
						ca2->controlledMonster->cellY = ca2->controlledMonster->targetCellY = ca2->cellY;
						rp->mobList->Add(ca2->controlledMonster);
					}

					MessChangeMap changeMap;
					changeMap.realmID = rp->type;
					changeMap.oldType = sp->WhatAmI();
					changeMap.newType = rp->WhatAmI();
					changeMap.sizeX = rp->sizeX;
					changeMap.sizeY = rp->sizeY;
					changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize
					// it's a realm map, so set realmid
					changeMap.realmID = ((RealmMap *)rp)->type;
					lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);
					MessInfoText infoText;
					CopyStringSafely("You enter the Realm of the Dead.",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

					ca2->QuestSpaceChange(sp, rp);

					// tell everyone about my arrival
					ca2->IntroduceMyself(rp, SPECIAL_APP_DUNGEON);
					if (ca2->controlledMonster)
					{
						// tell clients about my monster
						ca2->controlledMonster->AnnounceMyselfCustom(rp);
					}
					// tell this player about everyone else around
					ca2->UpdateClient(rp, TRUE);
				}
            }
        }
        curMob = (BBOSMob *) sp->avatars->Next();
    }

}


//*******************************************************************************
void BBOServer::HandleSpiritGate(int x, int y, SharedSpace *sp)
{
    MessInfoText infoText;
//	char tempText[1024];

    std::vector<TagID> tempReceiptList;
//	tempReceiptList.clear();
//	tempReceiptList.push_back(ca->socketIndex);
	if ((sp->WhatAmI() == SPACE_REALM)|| (REALM_ID_SPIRITS == ((RealmMap *)sp)->type)) // if already in the spirit realm
	{
		return;
	}
    BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())
        {
            BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
            if (ca2->cellX == x && ca2->cellY == y)
            {

                RealmMap *rp = NULL;
                SharedSpace *sp2 = (SharedSpace *) spaceList->First();
                while (sp2)
                {
                    if (SPACE_REALM == sp2->WhatAmI() && REALM_ID_SPIRITS == ((RealmMap *)sp2)->type)
                    {
                        rp = (RealmMap *)sp2;
                        sp2 = (SharedSpace *) spaceList->Last();
                    }
                    sp2 = (SharedSpace *) spaceList->Next();
                }

				if (rp)
				{
					// gotta check for controlling monster
							// am i controlling a monster?
					if (ca2->controlledMonster && !ca2->followMode)
					{
						// and make my monster vanish too
						MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
						tempReceiptList.clear();
						tempReceiptList.push_back(ca2->socketIndex); // point server at the target client.
						adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
						lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
						// and resturn server to follow mode, since both will end up in the same spot.
						if (ca2->BeastStat() > 9)
							ca2->followMode = TRUE;
						// we need to dissapear it from people's screen.
					}
					// second, vanish me
					ca2->AnnounceDisappearing(sp, SPECIAL_APP_DUNGEON);
					// third, vanish the controlled monster if present
					if (ca2->controlledMonster)
					{
						// we need to dissapear it from people's screen.
						MessMonsterDeath messMD;
						messMD.mobID = (unsigned long)ca2->controlledMonster;
						sp->SendToEveryoneNearBut(0, ca2->controlledMonster->cellX, ca2->controlledMonster->cellY,
							sizeof(messMD), (void *)&messMD);
						// remove it fropm old map
						sp->mobList->Remove(ca2->controlledMonster);

					}

					// tell my client I'm entering the realm
					tempReceiptList.clear();
					tempReceiptList.push_back(ca2->socketIndex);

					// next adjust the coordinates
					sp->avatars->Remove(ca2);
					ca2->cellX = ca2->targetCellX = 4;
					ca2->cellY = ca2->targetCellY = 4;
					rp->avatars->Append(ca2);

					if (ca2->controlledMonster) // if they had a monster
					{
						// then it needs ot have it's coords updated, and be added to new map
						ca2->controlledMonster->cellX = ca2->controlledMonster->targetCellX = ca2->cellX;
						ca2->controlledMonster->cellY = ca2->controlledMonster->targetCellY = ca2->cellY;
						rp->mobList->Add(ca2->controlledMonster);
					}

					MessChangeMap changeMap;
					changeMap.realmID = rp->type;
					changeMap.oldType = sp->WhatAmI();
					changeMap.newType = rp->WhatAmI();
					changeMap.sizeX = rp->sizeX;
					changeMap.sizeY = rp->sizeY;
					changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize
					// it's a realm map, so set realmid
					changeMap.realmID = ((RealmMap *)rp)->type;
					lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);
					MessInfoText infoText;
					CopyStringSafely("You enter the Realm of Spirits.",
						200, infoText.text, MESSINFOTEXTLEN);
					lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

					ca2->QuestSpaceChange(sp, rp);

					// tell everyone about my arrival
					ca2->IntroduceMyself(rp, SPECIAL_APP_DUNGEON);
					if (ca2->controlledMonster)
					{
						// tell clients about my monster
						ca2->controlledMonster->AnnounceMyselfCustom(rp);
					}
					// tell this player about everyone else around
					ca2->UpdateClient(rp, TRUE);
				}

            }
        }
        curMob = (BBOSMob *) sp->avatars->Next();
    }

}


//*******************************************************************************
void BBOServer::ProcessFriend(BBOSAvatar *srcAvatar, int type, 
                                        SharedSpace *ss, char *targetName)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;
    SharedSpace *sp;
    MessPlayerChatLine chatMess;
    DataObject *fName, *fCurName;
    MessInfoText infoText;

    DoublyLinkedList *holdingList = new DoublyLinkedList();

    int messLen = 0;
    char fullName[300], tempText[1024];
    char fullName2[300];
    int startingLine = TRUE;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(srcAvatar->socketIndex);

    if (FRIEND_ACTION_ADD == type && targetName)
    {
        fName = srcAvatar->contacts->First();
        while (fName)
        {
            if (IsCompletelySame(fName->WhoAmI(), targetName))
            {
                sprintf_s(tempText,"%s is already a friend.", fName->WhoAmI());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }

            fName = srcAvatar->contacts->Next();
        }

        fName = new DataObject(CONTACT_FRIEND, targetName);
        srcAvatar->contacts->Append(fName);

        sprintf_s(tempText,"%s is added.", fName->WhoAmI());
        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);

        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        return;
    }

    if (FRIEND_ACTION_REMOVE == type && targetName)
    {
        fName = srcAvatar->contacts->First();
        while (fName)
        {
            if (IsCompletelySame(fName->WhoAmI(), targetName))
            {
                srcAvatar->contacts->Remove(fName);
                sprintf_s(tempText,"%s is no longer a friend.", fName->WhoAmI());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                delete fName;
                return;
            }

            fName = srcAvatar->contacts->Next();
        }

        sprintf_s(tempText,"%s is not a friend.", targetName);
        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        return;
    }

    if (FRIEND_ACTION_IGNORE == type && targetName)
    {
        fName = srcAvatar->contacts->First();
        while (fName)
        {
            if (IsCompletelySame(fName->WhoAmI(), targetName) && 
                 CONTACT_IGNORE == fName->WhatAmI())
            {
                sprintf_s(tempText,"%s is no longer ignored.", fName->WhoAmI());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                srcAvatar->contacts->Remove(fName);
                delete fName;

                return;
            }

            fName = srcAvatar->contacts->Next();
        }

        fName = new DataObject(CONTACT_IGNORE, targetName);
        srcAvatar->contacts->Append(fName);

        sprintf_s(tempText,"%s will be ignored.", fName->WhoAmI());
        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);

        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        return;
    }


    if (FRIEND_ACTION_LIST == type)
    {
        sprintf(&(chatMess.text[messLen]),"Frnds: ");
        messLen = strlen(chatMess.text);
    }

    sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            curAvatar = (BBOSAvatar *) curMob;
            if (SMOB_AVATAR == curMob->WhatAmI() && 
                 curAvatar->socketIndex != srcAvatar->socketIndex)
            {
                int isContact = FALSE;
                fCurName = NULL;

                fName = srcAvatar->contacts->First();
                while (fName)
                {
                    if (FALSE == curAvatar->isInvisible && IsCompletelySame(fName->WhoAmI(),
                         curAvatar->charInfoArray[curAvatar->curCharacterIndex].name))
                    {
                        isContact = TRUE;
                        fCurName = fName;
                        fName = srcAvatar->contacts->Last();
                    }

                    fName = srcAvatar->contacts->Next();
                }

                if (isContact)
                {
                    srcAvatar->contacts->Remove(fCurName);
                    holdingList->Append(fCurName);
                }

                if (FRIEND_ACTION_LIST == type && isContact && 
                     CONTACT_FRIEND == fCurName->WhatAmI())
                {
                    char *name = curAvatar->charInfoArray[curAvatar->curCharacterIndex].name;
                    sprintf_s(fullName,"%s (on)",name);
                    sprintf_s(fullName2,", %s (on)",name);

                    for (int testVal = 0; testVal < 1; ++testVal)
                    {
                        if (strlen(fullName2) + messLen > 75)
                        {
                            lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                            messLen = 0;
                            startingLine = TRUE;
                        }
                        
                        if (startingLine)
                            sprintf(&(chatMess.text[messLen]),"%s", fullName);
                        else
                            sprintf(&(chatMess.text[messLen]),"%s", fullName2);
                        messLen = strlen(chatMess.text);
                        startingLine = FALSE;
                    }
                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        sp = (SharedSpace *) spaceList->Next();
    }

    // tell about offline friends
    fName = srcAvatar->contacts->First();
    while (fName)
    {
        if (FRIEND_ACTION_LIST == type && CONTACT_FRIEND == fName->WhatAmI())
        {
            sprintf_s(fullName,"%s",fName->WhoAmI());
            sprintf_s(fullName2,", %s",fName->WhoAmI());

            for (int testVal = 0; testVal < 1; ++testVal)
            {
                if (strlen(fullName2) + messLen > 75)
                {
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                    messLen = 0;
                    startingLine = TRUE;
                }
                
                if (startingLine)
                    sprintf(&(chatMess.text[messLen]),"%s", fullName);
                else
                    sprintf(&(chatMess.text[messLen]),"%s", fullName2);
                messLen = strlen(chatMess.text);
                startingLine = FALSE;
            }
        }
        fName = srcAvatar->contacts->Next();
    }

    // replace held names in srcAvatar's list
    fName = holdingList->First();
    while (fName)
    {
        holdingList->Remove(fName);
        srcAvatar->contacts->Append(fName);
        fName = holdingList->First();
    }

    delete holdingList;

    if (messLen > 0)
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);

}

//*******************************************************************************
void BBOServer::TellBuddiesImHere(BBOSAvatar *srcAvatar)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;
    SharedSpace *sp;
    MessPlayerChatLine chatMess;
    int messLen = 0;
//	char fullName[300];
//	char fullName2[300];
    int startingLine = TRUE;

    std::vector<TagID> tempReceiptList;
//	tempReceiptList.clear();
//	tempReceiptList.push_back(srcAfromSocket);

    sprintf_s(chatMess.text,"Your friend %s has logged on.",
              srcAvatar->charInfoArray[srcAvatar->curCharacterIndex].name);

    sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        curMob = (BBOSMob *) sp->avatars->First();
        while (curMob)
        {
            if (SMOB_AVATAR == curMob->WhatAmI())
            {
                curAvatar = (BBOSAvatar *) curMob;

                if (curAvatar->IsContact(
                         srcAvatar->charInfoArray[srcAvatar->curCharacterIndex].name, 
                          CONTACT_FRIEND))
                {
                    tempReceiptList.clear();
                    tempReceiptList.push_back(curAvatar->socketIndex);
                    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                }
            }
            curMob = (BBOSMob *) sp->avatars->Next();
        }

        sp = (SharedSpace *) spaceList->Next();
    }

}

//*******************************************************************************
void BBOServer::HandlePetFeeding(MessFeedPetRequest *mess, BBOSAvatar *curAvatar,
                                            SharedSpace *ss)
{
    if( !mess->ptr )
        return;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    char tempText[1024];
    MessInfoText infoText;
    int invType = MESS_WIELD_PLAYER;

    if (mess->which < 0)
        mess->which = 0;
    if (mess->which > 1)
        mess->which = 1;

    // find food item to feed her
    Inventory *src = (curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield);

    InventoryObject *io = (InventoryObject *) src->objects.Find(
                                    (InventoryObject *) mess->ptr);
    if (!io)
    {
        src = (curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory);

        io = (InventoryObject *) src->objects.Find(
                                    (InventoryObject *) mess->ptr);

        invType = MESS_INVENTORY_PLAYER;

    }
    if (!io) 
    {
        src = (curAvatar->charInfoArray[curAvatar->curCharacterIndex].workbench);

        io = (InventoryObject *) src->objects.Find(
                                    (InventoryObject *) mess->ptr);

        invType = MESS_WORKBENCH_PLAYER;
    }
    if (!io) // if didn't find the object at all
    {
        return;
    }

    if (io->type != INVOBJ_MEAT)
        return;

    InvMeat *im = (InvMeat *)io->extra;

    // feed it to her
    PetDragonInfo *dInfo = &(curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[mess->which]);

    LongTime ltNow;
    if (dInfo->lastEatenTime.MinutesDifference(&ltNow) < 20)
    {
        sprintf_s(tempText,"%s is still full.  She refuses the meat.", dInfo->name);
        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        return;
    }

    dInfo->lastEatenTime.SetToNow();
    dInfo->state = DRAGON_HEALTH_GREAT;

    // record it as the last item fed
    dInfo->lastEatenType    = im->type;
    dInfo->lastEatenSubType = im->quality;

    // did it make her stronger?
    if (dInfo->lifeStage < 3 && 
         dragonInfo[dInfo->quality][dInfo->type].powerMeatType[dInfo->lifeStage] == im->type)
    {
        dInfo->healthModifier += 0.3f;

        sprintf_s(tempText,"You feed %s; she loves it!", dInfo->name);
        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
    else
    {
        sprintf_s(tempText,"You feed %s.", dInfo->name);
        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }
	// is it a color change food?
	if (dragonInfo[dInfo->quality][dInfo->type].goodMeatType[dInfo->lifeStage] == dInfo->lastEatenType)
		{
			sprintf_s(tempText, "This food may cause %s to change.", dInfo->name);
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		}
		// is it an egg laying food?
	if (dragonInfo[dInfo->quality][dInfo->type].breedMeatType[dInfo->lifeStage] == dInfo->lastEatenType ||
			dragonInfo[dInfo->quality][dInfo->type].okayMeatType[dInfo->lifeStage] == dInfo->lastEatenType)
		{
			sprintf_s(tempText, "This food may cause %s to become fertile.", dInfo->name);
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

		}
	

    if (DRAGON_TYPE_BLACK != dInfo->type)
    {
        if (im->age >= 24 || -2 == im->age) // if rotted
        {
            dInfo->healthModifier -= 0.3f;

            sprintf_s(tempText,"The rotted meat makes %s gag!", dInfo->name);
            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
    }
    else
    {
        if (im->age < 24 && im->age > -2) // if NOT rotted
        {
            dInfo->healthModifier -= 0.3f;

            sprintf_s(tempText,"The fresh meat makes %s gag!", dInfo->name);
            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
    }

    MessPet mPet;
    mPet.avatarID = curAvatar->socketIndex;
    CopyStringSafely(dInfo->name,16, 
                  mPet.name,16);
    mPet.quality = dInfo->quality;
    mPet.type    = dInfo->type;
    mPet.state   = dInfo->state;
    mPet.size    = dInfo->lifeStage + dInfo->healthModifier/10.0f;
    mPet.which   = mess->which;

    ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY, sizeof(mPet),(void *)&mPet);

    // remove the eaten meat
    --(io->amount);
    if (io->amount < 1)
    {
        src->objects.Remove(io);
        delete io;
    }

    TellClientAboutInventory(curAvatar, invType);
}

//*******************************************************************************
void BBOServer::HandleAdminMessage(MessAdminMessage *mess, BBOSAvatar *curAvatar,
                                            SharedSpace *ss)
{
    MessAdminMessage adminMess;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    char tempText[1024];
    MessInfoText infoText;

    if (MESS_ADMIN_TAKE_CONTROL == mess->messageType)
    {
        if (curAvatar->controlledMonster)
        {
            SharedSpace *sp;

            BBOSMonster * theMonster = FindMonster(
                     curAvatar->controlledMonster, &sp);
            if (theMonster)
            {
                curAvatar->controlledMonster->controllingAvatar = NULL;
                curAvatar->controlledMonster = NULL;

                sprintf_s(tempText,"You release a monster.");
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
//				return;
            }
        }

        BBOSMob *curMob = (BBOSMob *) ss->mobList->GetFirst(0,0,1000);
        while (curMob)
        {
            if (SMOB_MONSTER == curMob->WhatAmI() && mess->mobID == (long) curMob)
            {
                curAvatar->controlledMonster = (BBOSMonster *) curMob;
                curAvatar->controlledMonster->controllingAvatar = curAvatar;
				// check if it has a generator
				if (curAvatar->controlledMonster->myGenerator)
				{
					if (curAvatar->controlledMonster->myGenerator->do_id == 0) // if it's an actual generator
					{
						// then we need to decrease the counter so the monster will get replaced next spawn run.
						curAvatar->controlledMonster->myGenerator->count[curAvatar->controlledMonster->type][curAvatar->controlledMonster->subType]--;
					}
					// if it was an army instead, the army code handles that
				}
                curAvatar->controlledMonster->myGenerator = NULL; // detach from armies and spawners
																  // this prevents a double decrement of the counter (foor the generator)
																  // and will cause the army IsValidMonster function to return false, so it will get replaced.
                sprintf_s(tempText,"You have controlled the %s.",
                    ((BBOSMonster *)curMob)->Name());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }

            curMob = (BBOSMob *) ss->mobList->GetNext();
        }

    }
	else if (MESS_ADMIN_RELEASE_CONTROL == mess->messageType)
	{
		if (curAvatar->controlledMonster)
		{
			curAvatar->controlledMonster->controllingAvatar = NULL;
			curAvatar->controlledMonster = NULL;

			sprintf_s(tempText, "You release the monster.");
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

			adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
			lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);

			curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_NOTHING);

			// tell everyone about my arrival
			curAvatar->IntroduceMyself(ss, SPECIAL_APP_NOTHING);

			// tell this player about everyone else around
			curAvatar->UpdateClient(ss, TRUE);

			return;
		}

		sprintf_s(tempText, "No valid monster to release.");
		CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		return;
	}
	else if (MESS_PLAYER_RECALL == mess->messageType)
	{
		if (curAvatar->controlledMonster && curAvatar->BeastStat()>9)  // make sure it's actually a beastmaster
		{
			// remove monster's target
			curAvatar->controlledMonster->curTarget = NULL;
			// bring back the monster to my square
			int oldcellx = curAvatar->controlledMonster->cellX;
			int oldcelly = curAvatar->controlledMonster->cellY;
			if (!curAvatar->followMode)
			{
				sprintf_s(tempText, "You recall the monster to your side.");
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
				lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			}
			adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
			lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
			curAvatar->followMode = TRUE;
			MessAvatarAppear mAppear;
			mAppear.avatarID = curAvatar->socketIndex;
			mAppear.x = curAvatar->cellX;
			mAppear.y = curAvatar->cellY;
			tempReceiptList.clear();
			tempReceiptList.push_back(curAvatar->socketIndex);
			ss->lserver->SendMsg(sizeof(mAppear), &mAppear, 0, &tempReceiptList);

			curAvatar->UpdateClient(ss, TRUE);

			// curAvatar->AnnounceDisappearing(ss, SPECIAL_APP_NOTHING);
			
							// announce monster dissapearance 
			MessMobDisappear messMobDisappear;
			messMobDisappear.mobID = (unsigned long)curAvatar->controlledMonster;
			messMobDisappear.x = (unsigned char)oldcellx;
			messMobDisappear.y = (unsigned char)oldcelly;
			ss->SendToEveryoneNearBut(0, oldcellx, oldcelly,
				sizeof(messMobDisappear), (void *)&messMobDisappear);
			messMobDisappear.mobID = (unsigned long)curAvatar->controlledMonster; // and tell the people in my square it's gone too.
			messMobDisappear.x = (unsigned char)oldcellx;
			messMobDisappear.y = (unsigned char)oldcelly;
			ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
				sizeof(messMobDisappear), (void *)&messMobDisappear);

			// tell everyone about my monster warping back.
			curAvatar->controlledMonster->cellX = curAvatar->cellX;
			curAvatar->controlledMonster->cellY = curAvatar->cellY;
			curAvatar->controlledMonster->AnnounceMyselfCustom(ss); // 
			curAvatar->followMode = TRUE;
			curAvatar->UpdateClient(ss, TRUE);
			// update the moblist so this doesn't break searches
			ss->mobList->Move(curAvatar->controlledMonster);
			return;
		}

	}
	else if (MESS_PLAYER_CONTROL == mess->messageType)
	{
		curAvatar->followMode = FALSE; // monster can go on ahead now.
		sprintf_s(tempText, "You have sent your tamed monster off.");
		CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
		lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
	}

}

//*******************************************************************************
void BBOServer::HandleContMonstString(char *chatText, BBOSAvatar *curAvatar,
                                            SharedSpace *ss)
{
    MessAdminMessage adminMess;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

	char tempText[1024];
	char tempText2[1024];
	MessInfoText infoText;
    int linePoint, argPoint;
    linePoint = 0;
    argPoint = NextWord(chatText,&linePoint);
	FILE *source;
    if (curAvatar->controlledMonster)
    {
        SharedSpace *sp;

        BBOSMonster * theMonster = FindMonster(
                 curAvatar->controlledMonster, &sp);
        if (theMonster)
        {
            if ( IsSame(&(chatText[argPoint]) , "stat"))
            {
                sprintf_s(tempText,"Controlling %s", theMonster->Name());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf_s(tempText,"health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
                    theMonster->health, theMonster->maxHealth, theMonster->damageDone, 
                    theMonster->toHit, theMonster->defense, theMonster->dropAmount,
                    theMonster->magicResistance, theMonster->healAmountPerSecond);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }
            else if ( IsSame(&(chatText[argPoint]) , "health"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->health = theMonster->maxHealth = atoi(&(chatText[argPoint]));

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else if ( IsSame(&(chatText[argPoint]) , "dam"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->damageDone = atoi(&(chatText[argPoint]));

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else if ( IsSame(&(chatText[argPoint]) , "tohit"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->toHit = atoi(&(chatText[argPoint]));

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
			else if (IsSame(&(chatText[argPoint]), "def"))
			{
				argPoint = NextWord(chatText, &linePoint);
				theMonster->defense = atoi(&(chatText[argPoint]));

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
				lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			}
			else if (IsSame(&(chatText[argPoint]), "regen"))
			{
				argPoint = NextWord(chatText, &linePoint);
				theMonster->healAmountPerSecond = atoi(&(chatText[argPoint]));

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
				lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			}
			else if ( IsSame(&(chatText[argPoint]) , "resist"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->magicResistance = atoi(&(chatText[argPoint])) / 100.0f;
                if (theMonster->magicResistance < 0)
                    theMonster->magicResistance = 0;
//                if (theMonster->magicResistance > 1)
//                    theMonster->magicResistance = 1;

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else if ( IsSame(&(chatText[argPoint]) , "dropval"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->dropAmount = atoi(&(chatText[argPoint]));

				sprintf_s(tempText, "health %d/%d, dam %d, toHit %d, def %d, dropVal %d resist %1.2f, regen %d",
					theMonster->health, theMonster->maxHealth, theMonster->damageDone,
					theMonster->toHit, theMonster->defense, theMonster->dropAmount,
					theMonster->magicResistance, theMonster->healAmountPerSecond);
				lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else if ( IsSame(&(chatText[argPoint]) , "name"))
            {
                argPoint = NextWord(chatText,&linePoint);
                CopyStringSafely(&(chatText[argPoint]), 32, 
                                  theMonster->uniqueName, 32);

                sprintf_s(tempText,"Controlling %s", theMonster->Name());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                theMonster->AnnounceMyselfCustom(sp);
            }
            else if ( IsSame(&(chatText[argPoint]) , "size"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->sizeCoeff = atof(&(chatText[argPoint]));

                if (0 == theMonster->uniqueName[0])
                {
                    CopyStringSafely(theMonster->Name(), 32, 
                                      theMonster->uniqueName, 32);
                }
                sprintf_s(tempText,"Size now %f", theMonster->sizeCoeff);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                theMonster->AnnounceMyselfCustom(sp);
                return;
            }
            else if ( IsSame(&(chatText[argPoint]) , "color"))
            {
                argPoint = NextWord(chatText,&linePoint);
                theMonster->r = atoi(&(chatText[argPoint]));
                if (theMonster->r < 0)
                    theMonster->r = 0;
                if (theMonster->r > 255)
                    theMonster->r = 255;

                argPoint = NextWord(chatText,&linePoint);
                theMonster->g = atoi(&(chatText[argPoint]));
                if (theMonster->g < 0)
                    theMonster->g = 0;
                if (theMonster->g > 255)
                    theMonster->g = 255;

                argPoint = NextWord(chatText,&linePoint);
                theMonster->b = atoi(&(chatText[argPoint]));
                if (theMonster->b < 0)
                    theMonster->b = 0;
                if (theMonster->b > 255)
                    theMonster->b = 255;

                argPoint = NextWord(chatText,&linePoint);
                theMonster->a = atoi(&(chatText[argPoint]));
                if (theMonster->a < 0)
                    theMonster->a = 0;
                if (theMonster->a > 255)
                    theMonster->a = 255;

                if (0 == theMonster->uniqueName[0])
                {
                    CopyStringSafely(theMonster->Name(), 32, 
                                      theMonster->uniqueName, 32);
                }
                sprintf_s(tempText,"Color changed!");
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                theMonster->AnnounceMyselfCustom(sp);
                return;
            }
            else
            {
                CopyStringSafely("/m commands:", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m stat         - shows info for controlled monster", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m health <val> - sets current and maximum health points", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m dam <val>    - sets minimum damage done to opponent", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m tohit <val>  - sets chance to hit", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m def <val>    - sets chance to dodge attack", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m dropval <val>- sets general loot value", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m name <text>- sets monster's name", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m size <val>- sets size, 1.5 means 50 percent bigger", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m color <r> <g> <b> <a> - sets color, red, green, blue, alpha (0-255 each)", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                CopyStringSafely("/m resist <val>- sets magical resistance value (0-100)", 1024, infoText.text, MESSINFOTEXTLEN);
                lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
				CopyStringSafely("/m regen <val>    - sets health regained per second", 1024, infoText.text, MESSINFOTEXTLEN);
				lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				return;
            }
        }
        else
        {
            CopyStringSafely("Controlled monster not found.  Perhaps it's dead.", 1024, infoText.text, MESSINFOTEXTLEN);
            lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }
    }
    else
    {
        CopyStringSafely("No controlled monster.", 1024, infoText.text, MESSINFOTEXTLEN);
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
		return;
    }
	// log important /m commmands
	source = fopen("adminMCommand.txt", "a");
	/* Display operating system-style date and time. */
	_strdate(tempText2);
	fprintf(source, "%s, ", tempText2);
	_strtime(tempText2);
	fprintf(source, "%s, ", tempText2);
	fprintf(source, "%s, ", curAvatar->name);
	fprintf(source, "%s, ", curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
	fprintf(source, "%d, ", curAvatar->controlledMonster->cellX);
	fprintf(source, "%d, ", curAvatar->controlledMonster->cellY);
	fprintf(source, "%d, ", curAvatar->controlledMonster->type);
	fprintf(source, "%d, ", curAvatar->controlledMonster->subType);
	fprintf(source, "%s, ", ss->WhoAmI());
	fprintf(source, "%s\n", chatText);
	fclose(source);
}

//*******************************************************************************
void BBOServer::MaintainIncomingAvatars(void)
{
    BBOSMob *curMob;
    BBOSAvatar *curAvatar;

    DoublyLinkedList *list = incoming;

    curMob = (BBOSMob *) list->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())
        {
            curAvatar = (BBOSAvatar *) curMob;
            ++(curAvatar->activeCounter);
            if (curAvatar->activeCounter > 25)
            {
                list->Remove(curAvatar);
                delete curAvatar;
            }
        }
        curMob = (BBOSMob *) list->Next();
    }

}

//*******************************************************************************
void BBOServer::HandleSellingAll(BBOSAvatar *curAvatar, BBOSNpc *curNpc, int type)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    char tempText[1024];
    MessInfoText infoText;

    if (abs(townList[2].x - curAvatar->cellX) <= 5 && 
         abs(townList[2].y - curAvatar->cellY) <= 5 && INVOBJ_EARTHKEY == type)
    {
        CopyStringSafely("We don't buy those here.  Please try another town.", 1024, 
                          infoText.text, MESSINFOTEXTLEN);
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        return;
    }
	if (INVOBJ_SIMPLE == type || INVOBJ_MEAT == type ||
         INVOBJ_BOMB   == type || INVOBJ_EARTHKEY == type)
    {
        Inventory *inv = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory;
        Inventory *partner = curNpc->inventory;
		Inventory *tmpstorage=new Inventory(MESS_INVENTORY_PLAYER); // used for mean
		int saltedsomestuff = false;
        assert(inv->money >= 0);

        long moneyMade = 0;
        long itemsSold = 0;

        InventoryObject *io;
        io = (InventoryObject *) inv->objects.First();
        while (io)
        {
            if (type == io->type && io->value <= 1000)
            {
                if (INVOBJ_MEAT == io->type)
                {
                    SaltThisMeat(io);
					saltedsomestuff = true;
					itemsSold += io->amount;
					moneyMade -= io->value * io->amount * 3 / 10;
					inv->objects.Remove(io);
					tmpstorage->AddItemSorted(io);
				}
				else
				{
					inv->objects.Remove(io);
					itemsSold += io->amount;
					moneyMade += io->value * io->amount * 7 / 10;
					partner->AddItemSorted(io);
				}
            }

            io = (InventoryObject *) inv->objects.Next();
        }
		if (saltedsomestuff)
		{
			io = (InventoryObject *)tmpstorage->objects.First();
			while (io)
			{
				tmpstorage->objects.Remove(io);
				inv->AddItemSorted(io);
				io = (InventoryObject *)tmpstorage->objects.Next();
			}
			delete tmpstorage;
		}
		if (moneyMade>=0)
			sprintf_s(tempText, "You sold %ld items for %ld gold.", itemsSold, moneyMade);
		else
			sprintf_s(tempText, "You salted %ld meats for the cost of %ld gold.", itemsSold, (moneyMade*-1));
		CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

        assert(inv->money >= 0);

        inv->money += moneyMade;

//        assert(inv->money >= 0);
		if (inv->money < 0)  // if you didn't have enough to salt it all
			inv->money = 0;

        TellClientAboutInventory(curAvatar, MESS_INVENTORY_PLAYER);
    }
    else
    {
        CopyStringSafely("Please help us by finding and reporting bugs.  Thank you!", 1024, 
                          infoText.text, MESSINFOTEXTLEN);
        lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    }

}


//*******************************************************************************
void BBOServer::HandleExtendedInfo(BBOSAvatar *avatar, 
                                     MessExtendedInfoRequest *requestPtr)
{
//	char tempText[1024];
    MessInfoText infoText;

    if (!requestPtr->itemPtr)
        return;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(avatar->socketIndex);

//	InventoryObject *io2;

    Inventory *inv = NULL;
    switch(requestPtr->listType)
    {
    case MESS_INVENTORY_PLAYER:
        inv =	(avatar->charInfoArray[avatar->curCharacterIndex].inventory);
        break;
    case MESS_WORKBENCH_PLAYER:
        inv =	(avatar->charInfoArray[avatar->curCharacterIndex].workbench);
        break;
    case MESS_SKILLS_PLAYER:
        inv =	(avatar->charInfoArray[avatar->curCharacterIndex].skills);
        break;
    case MESS_WIELD_PLAYER:
        inv =	(avatar->charInfoArray[avatar->curCharacterIndex].wield);
        break;
    case MESS_INVENTORY_TRADER:
    case MESS_INVENTORY_GROUND:
        inv =	(avatar->charInfoArray[avatar->curCharacterIndex].inventory->partner);
        break;
    case MESS_INVENTORY_HER_SECURE:
        if (avatar->tradingPartner)
        {
            SharedSpace *sx;
            BBOSAvatar *partnerAvatar = NULL;
            partnerAvatar = FindAvatar(avatar->tradingPartner, &sx);
            if (partnerAvatar)
                inv =	(partnerAvatar->trade);
        }
        break;
    case MESS_INVENTORY_YOUR_SECURE:
        inv =	(avatar->trade);
        break;
    }

    if (!inv)  // no inventory, no info.
        return;

    LongTime ltNow;

    InventoryObject *io = (InventoryObject *) inv->objects.Find(
                                (InventoryObject *) requestPtr->itemPtr);
    if (io) // if found the object to transfer
    {
        Chronos::BStream *	stream		= NULL;
        stream	= new Chronos::BStream(1024);

        *stream << (unsigned char) NWMESS_EXTENDED_INFO; 

        *stream << (unsigned char) io->type;
        
        stream->write(io->WhoAmI(), strlen(io->WhoAmI()));
        *stream << (unsigned char) 0; 

        *stream << io->value;
        *stream << io->amount;
        *stream << io->status;

        short damValue;
        int minimum;
//		char smlText[128];

        switch(io->type)
        {
        case INVOBJ_BLADE:
            *stream << (short)((InvBlade *)io->extra)->toHit;

            damValue = ((InvBlade *)io->extra)->damageDone * 
                        (1.0f + 
                         avatar->PhysicalStat() * 0.15f) + 
                        avatar->totemEffects.effect[TOTEM_STRENGTH] * 1.0f;

            *stream << damValue;

            *stream << (unsigned short)((InvBlade *)io->extra)->poison;
            *stream << (unsigned short)((InvBlade *)io->extra)->heal;
            *stream << (unsigned short)((InvBlade *)io->extra)->slow;
            *stream << (unsigned short)((InvBlade *)io->extra)->blind;
			*stream << (unsigned short)((InvBlade *)io->extra)->lightning;
			*stream << (int)(((InvBlade *)io->extra)->numOfHits);
            *stream << (int)(((InvBlade *)io->extra)->bladeGlamourType);
			*stream << (unsigned short)((InvBlade *)io->extra)->tame;

            break;
        case INVOBJ_TOTEM:
            if (((InvTotem *)io->extra)->isActivated)
                *stream << (long) ((InvTotem *)io->extra)->timeToDie.MinutesDifference(&ltNow) * -1;
            else
                *stream << (long) -1;
            if ( ((InvTotem *)io->extra)->type >= TOTEM_PHYSICAL &&
                  ((InvTotem *)io->extra)->type <= TOTEM_CREATIVE)
                *stream << (short) (((InvTotem *)io->extra)->imbueDeviation);
            else
                *stream << (short) (((InvTotem *)io->extra)->quality - ((InvTotem *)io->extra)->imbueDeviation);
            break;
        case INVOBJ_STAFF:
            if (((InvStaff *)io->extra)->isActivated)
                *stream << (short) ((InvStaff *)io->extra)->charges;
            else
                *stream << (short) -1;
            *stream << (short) ((InvStaff *)io->extra)->imbueDeviation;
            break;
        case INVOBJ_INGOT:
            *stream << ((InvIngot *)io->extra)->damageVal;
            *stream << ((InvIngot *)io->extra)->challenge;
            *stream << (unsigned char) ((InvIngot *)io->extra)->r;
            *stream << (unsigned char) ((InvIngot *)io->extra)->g;
            *stream << (unsigned char) ((InvIngot *)io->extra)->b;
            break;
        case INVOBJ_POTION:
            *stream << ((InvPotion *)io->extra)->type;
            *stream << ((InvPotion *)io->extra)->subType;
            break;
        case INVOBJ_SKILL:
            *stream << ((InvSkill *)io->extra)->skillLevel;
            *stream << ((InvSkill *)io->extra)->skillPoints;
            break;
        case INVOBJ_INGREDIENT:
            *stream << ((InvIngredient *)io->extra)->type;
            *stream << ((InvIngredient *)io->extra)->quality;
            break;
        case INVOBJ_EXPLOSIVE:
            *stream << ((InvExplosive *)io->extra)->type;
            *stream << ((InvExplosive *)io->extra)->quality;
            *stream << ((InvExplosive *)io->extra)->power;
            break;
        case INVOBJ_FUSE:
            *stream << ((InvFuse *)io->extra)->type;
            *stream << ((InvFuse *)io->extra)->quality;
            break;
        case INVOBJ_GEOPART:
            *stream << ((InvGeoPart *)io->extra)->type;
            *stream << ((InvGeoPart *)io->extra)->power;
            break;
		case INVOBJ_EARTHKEY:
			*stream << ((InvEarthKey *)io->extra)->power;
			*stream << ((InvEarthKey *)io->extra)->monsterType[0];
			*stream << ((InvEarthKey *)io->extra)->monsterType[1];
			*stream << ((InvEarthKey *)io->extra)->width;
			*stream << ((InvEarthKey *)io->extra)->height;
			break;
		case INVOBJ_EARTHKEY_RESUME:
			*stream << ((InvEarthKey *)io->extra)->power;
			*stream << ((InvEarthKey *)io->extra)->monsterType[0];
			*stream << ((InvEarthKey *)io->extra)->monsterType[1];
			*stream << ((InvEarthKey *)io->extra)->width;
			*stream << ((InvEarthKey *)io->extra)->height;
			break;
		case INVOBJ_DOOMKEY:
			*stream << ((InvDoomKey *)io->extra)->power;
			*stream << ((InvDoomKey *)io->extra)->monsterType[0];
			*stream << ((InvDoomKey *)io->extra)->monsterType[1];
			*stream << ((InvDoomKey *)io->extra)->monsterType[2];
			*stream << ((InvDoomKey *)io->extra)->monsterType[3];
			*stream << ((InvDoomKey *)io->extra)->monsterType[4];
			*stream << ((InvDoomKey *)io->extra)->monsterType[5];
			*stream << ((InvDoomKey *)io->extra)->monsterType[6];
			*stream << ((InvDoomKey *)io->extra)->width;
			*stream << ((InvDoomKey *)io->extra)->height;
			break;
		case INVOBJ_DOOMKEY_ENTRANCE:
			*stream << ((InvDoomKey *)io->extra)->power;
			*stream << ((InvDoomKey *)io->extra)->monsterType[0];
			*stream << ((InvDoomKey *)io->extra)->monsterType[1];
			*stream << ((InvDoomKey *)io->extra)->monsterType[2];
			*stream << ((InvDoomKey *)io->extra)->monsterType[3];
			*stream << ((InvDoomKey *)io->extra)->monsterType[4];
			*stream << ((InvDoomKey *)io->extra)->monsterType[5];
			*stream << ((InvDoomKey *)io->extra)->monsterType[6];
			*stream << ((InvDoomKey *)io->extra)->width;
			*stream << ((InvDoomKey *)io->extra)->height;
			break;
		case INVOBJ_BOMB:
            *stream << (unsigned char) ((InvBomb *)io->extra)->r;
            *stream << (unsigned char) ((InvBomb *)io->extra)->g;
            *stream << (unsigned char) ((InvBomb *)io->extra)->b;
//			*stream << ((InvBomb *)io->extra)->flags;
//			*stream << ((InvBomb *)io->extra)->type;
            *stream << ((InvBomb *)io->extra)->power;
            *stream << ((InvBomb *)io->extra)->stability;
            *stream << ((InvBomb *)io->extra)->fuseDelay;
            break;
        case INVOBJ_EGG:
            *stream << ((InvEgg *)io->extra)->type;

            minimum = 4 + ((InvEgg *)io->extra)->quality + ((InvEgg *)io->extra)->type;
            if (minimum > 10)
                minimum = 10;

            if (avatar->charInfoArray[avatar->curCharacterIndex].magical < minimum)
                *stream << (char) avatar->charInfoArray[avatar->curCharacterIndex].magical;
            else
                *stream << (char) 0;
            break;
        case INVOBJ_MEAT:
            *stream << ((InvMeat *)io->extra)->type;
            *stream << ((InvMeat *)io->extra)->quality;
            *stream << ((InvMeat *)io->extra)->age;
            break;
        }

        lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);

        delete stream;


    }

}


//*******************************************************************************
int BBOServer::FindAvatarInGuild(char *name, SharedSpace **sp)
{
//   BBOSMob *curMob;
//	BBOSAvatar *curAvatar;

    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        if (SPACE_GUILD == (*sp)->WhatAmI())
        {
            MemberRecord *mr = (MemberRecord *) ((TowerMap *)(*sp))->members->First();
            while (mr)
            {
                if (IsCompletelySame(mr->WhoAmI(), name))
                {
                    return TRUE;
                }
                mr = (MemberRecord *) ((TowerMap *)(*sp))->members->Next();
            }
        }
        (*sp) = (SharedSpace *) spaceList->Next();
    }

    return FALSE;
}

MemberRecord* BBOServer::GetRecordForGuildMember( char *name, SharedSpace **sp )
{
    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        if (SPACE_GUILD == (*sp)->WhatAmI())
        {
            MemberRecord *mr = (MemberRecord *) ((TowerMap *)(*sp))->members->First();
            while (mr)
            {
                if (IsCompletelySame(mr->WhoAmI(), name))
                {
                    return mr;
                }
                mr = (MemberRecord *) ((TowerMap *)(*sp))->members->Next();
            }
        }
        (*sp) = (SharedSpace *) spaceList->Next();
    }

    return NULL;
}

//*******************************************************************************
int BBOServer::FindGuild(DWORD nameCRC, SharedSpace **sp)
{
//   BBOSMob *curMob;
//	BBOSAvatar *curAvatar;

    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        if (SPACE_GUILD == (*sp)->WhatAmI() && 0 < strlen((*sp)->WhoAmI()))
        {
            DWORD c = GetCRCForString((*sp)->WhoAmI());
            if (nameCRC == c)
                return TRUE;
        }
        (*sp) = (SharedSpace *) spaceList->Next();
    }

    return FALSE;
}

//*******************************************************************************
int BBOServer::ChangeAvatarGuildName(char *name, char *newName)
{
//   BBOSMob *curMob;
//	BBOSAvatar *curAvatar;

    SharedSpace *sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        if (SPACE_GUILD == (sp)->WhatAmI())
        {
            MemberRecord *mr = (MemberRecord *) ((TowerMap *)(sp))->members->First();
            while (mr)
            {
                if (IsCompletelySame(mr->WhoAmI(), name))
                {
                    sprintf_s(mr->do_name,newName);
                    return TRUE;
                }
                mr = (MemberRecord *) ((TowerMap *)(sp))->members->Next();
            }
        }
        (sp) = (SharedSpace *) spaceList->Next();
    }

    return FALSE;
}

//*******************************************************************************
int BBOServer::DeleteNameFromGuild(char *name, SharedSpace **sp)
{
//   BBOSMob *curMob;
//	BBOSAvatar *curAvatar;

    (*sp) = (SharedSpace *) spaceList->First();
    while (*sp)
    {
        if (SPACE_GUILD == (*sp)->WhatAmI())
        {
            MemberRecord *mr = (MemberRecord *) ((TowerMap *)(*sp))->members->First();
            while (mr)
            {
                if (IsCompletelySame(mr->WhoAmI(), name))
                {
                    ((TowerMap *)(*sp))->members->Remove(mr);
                    delete mr;
                    return TRUE;
                }
                mr = (MemberRecord *) ((TowerMap *)(*sp))->members->Next();
            }
        }
        (*sp) = (SharedSpace *) spaceList->Next();
    }

    return FALSE;
}

//*******************************************************************************
void BBOServer::SendToEveryGuildMate(char *senderName, SharedSpace *sp, 
                                                 TowerMap *guild, int size, const void *dataPtr)
{
    std::vector<TagID> tempReceiptList;

    tempReceiptList.clear();

    BBOSMob *curMob = (BBOSMob *) sp->avatars->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI())
        {
            BBOSAvatar *curAvatar = (BBOSAvatar *) curMob;
            if (guild->IsMember(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name))
            {
                if (!curAvatar->IsContact(senderName, CONTACT_IGNORE))
                    tempReceiptList.push_back(curAvatar->socketIndex);
            }
        }
        curMob = (BBOSMob *) sp->avatars->Next();
    }

    lserver->SendMsg(size, dataPtr, 0, &tempReceiptList);

}

//*******************************************************************************
void BBOServer::ListGuild(BBOSAvatar *curAvatar, TowerMap *guild)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    int messLen = 0;
//	char tempText[1024];
    char fullName[300], tempName[300];
    char fullName2[300];
    int startingLine = TRUE;

    MessPlayerChatLine chatMess;
    sprintf(&(chatMess.text[messLen]),"%s: %s", 
                 guildStyleNames[guild->guildStyle], guild->WhoAmI());
    messLen = strlen(chatMess.text);

    sprintf(&(chatMess.text[messLen])," (spec %d/%d/%d)", 
               guild->specLevel[0], guild->specLevel[1], guild->specLevel[2]);
    messLen = strlen(chatMess.text);

    lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
    messLen = 0;
    startingLine = TRUE;

    MemberRecord *mr = (MemberRecord *) guild->members->First();
    while (mr)
    {

        char *name = mr->WhoAmI();
        if (mr->value1 < 1)
            sprintf_s(tempName, name);
        else
        {
            sprintf_s(tempName, "%s %s", guildLevelNames[guild->guildStyle-1][mr->value1-1],
                     name);
        }
        sprintf_s(fullName,"%s",tempName);
        sprintf_s(fullName2,", %s",tempName);

        for (int testVal = 0; testVal < 1; ++testVal)
        {
            if (strlen(fullName2) + messLen > 70)
            {
                lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
                messLen = 0;
                startingLine = TRUE;
            }
            
            if (startingLine)
                sprintf(&(chatMess.text[messLen]),"%s", fullName);
            else
                sprintf(&(chatMess.text[messLen]),"%s", fullName2);
            messLen = strlen(chatMess.text);
            startingLine = FALSE;
        }
        mr = (MemberRecord *) guild->members->Next();
    }


    if (messLen > 0)
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
}


//*******************************************************************************
void BBOServer::ListVotes(BBOSAvatar *curAvatar, TowerMap *guild)
{
    if (!curAvatar || !guild)
        return;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    int messLen = 0;
    char tempText[1024];
//	char fullName[300];
//	char fullName2[300];
    int startingLine = TRUE;

    MessPlayerChatLine chatMess;
    MessInfoText       infoText;

    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
    tempText[1] = TEXT_COLOR_DATA;

    for (int i = 0; i < 4; ++i)
    {
        if (GUILDBILL_INACTIVE == guild->bills[i].type)
        {
            sprintf(&(tempText[2]),"%d: unused.", i+1);
            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
        }
        else
        {
            int yesVotes, allVotes;
            yesVotes = allVotes = 0;

            MemberRecord *mr = (MemberRecord *) guild->bills[i].recordedVotes.First();
            while(mr)
            {
                ++allVotes;
                if (mr->WhatAmI())
                    ++yesVotes;
                mr = (MemberRecord *) guild->bills[i].recordedVotes.Next();
            }

            int percent = 0;
            if (allVotes > 0)
                percent = yesVotes * 100 / allVotes;

            sprintf(&(tempText[2]),"%d: %s %s (%d%%).", i+1,
                         guildBillNames[guild->bills[i].type], 
                            guild->bills[i].subject, 
                            percent);
            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

            LongTime now;
            long diff = now.MinutesDifference(&(guild->bills[i].expTime));
            if (diff > 0)
            {
                sprintf(&(tempText[2]),"%ld minutes left to vote (%ld hours).",
                          diff, diff/60);
                lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            }
            else
            {
                if (VOTESTATE_PASSED == guild->bills[i].voteState)
                    sprintf(&(tempText[2]),"Bill PASSED %ld hours ago.",diff/-60);
                else
                    sprintf(&(tempText[2]),"Bill was DEFEATED %ld hours ago.",diff/-60);

                lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            }
        }
    }
}


//*******************************************************************************
void BBOServer::DetailVote(int i, BBOSAvatar *curAvatar, TowerMap *guild)
{
    if (!curAvatar || !guild)
        return;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    int messLen = 0;
    char tempText[1024];
//	char fullName[300];
//	char fullName2[300];
    int startingLine = TRUE;

    MessPlayerChatLine chatMess;
    MessInfoText       infoText;

    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
    tempText[1] = TEXT_COLOR_DATA;

    sprintf(&(tempText[2]),"%d: %s", i+1, guildBillDesc[guild->bills[i].type]);
    lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

    if (GUILDBILL_INACTIVE != guild->bills[i].type)
    {
        int yesVotes, allVotes;
        yesVotes = allVotes = 0;

        MemberRecord *mr = (MemberRecord *) guild->bills[i].recordedVotes.First();
        while(mr)
        {
            ++allVotes;
            if (mr->WhatAmI())
                ++yesVotes;
            mr = (MemberRecord *) guild->bills[i].recordedVotes.Next();
        }

        sprintf(&(tempText[2]),"Sponsor: %s.  Subject: %s.", 
            guild->bills[i].sponsor, guild->bills[i].subject);
        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

        int percent = 0;
        if (allVotes > 0)
            percent = yesVotes * 100 / allVotes;

        sprintf(&(tempText[2]),"%d votes tallied, yes %d, no %d (%d%%).", 
                     allVotes, yesVotes, allVotes - yesVotes, percent);
        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

        LongTime now;
        long diff = now.MinutesDifference(&(guild->bills[i].expTime));
        if (diff > 0)
        {
            sprintf(&(tempText[2]),"%ld minutes left to vote (%ld hours).",
                      diff, diff/60);
            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
        }
        else
        {
            if (VOTESTATE_PASSED == guild->bills[i].voteState)
                sprintf(&(tempText[2]),"Bill PASSED %ld hours ago.",diff/-60);
            else
                sprintf(&(tempText[2]),"Bill was DEFEATED %ld hours ago.",diff/-60);

            lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
        }
    }
    
}

char voteTypeText[2][12] =
{
    {"No"},
    {"Yes"}
};

//*******************************************************************************
void BBOServer::VoteOnBill(int voteVal, int i, BBOSAvatar *curAvatar, TowerMap *guild)
{
    if (!curAvatar || !guild)
        return;

    if (voteVal != 0)
        voteVal = 1;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    int messLen = 0;
    char tempText[1024];
//	char fullName[300];
//	char fullName2[300];
    int startingLine = TRUE;

    MessPlayerChatLine chatMess;
    MessInfoText       infoText;

    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
    tempText[1] = TEXT_COLOR_DATA;

//	sprintf(&(tempText[2]),"%d: %s", i, guildBillDesc[guild->bills[i].type]);
//	lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

    LongTime now;
    long diff = now.MinutesDifference(&(guild->bills[i].expTime));
    if (diff > 0 && GUILDBILL_INACTIVE != guild->bills[i].type)
    {
        int yesVotes, allVotes;
        yesVotes = allVotes = 0;

        MemberRecord *myVote = NULL;
        MemberRecord *mr = (MemberRecord *) guild->bills[i].recordedVotes.First();
        while(mr)
        {
            if (IsCompletelySame(curAvatar->charInfoArray[curAvatar->curCharacterIndex].name,
                                  mr->WhoAmI()))
                 myVote = mr;
            mr = (MemberRecord *) guild->bills[i].recordedVotes.Next();
        }

        if (myVote)
        {
            sprintf(&(tempText[2]),"You've changed your vote on bill %d to %s.", 
                i+1, voteTypeText[voteVal]);
            myVote->do_id = voteVal;
        }
        else
        {
            sprintf(&(tempText[2]),"You vote %s on bill %d.", 
                voteTypeText[voteVal], i+1);

            myVote = new MemberRecord(voteVal, 
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
            myVote->do_id = voteVal;
            guild->bills[i].recordedVotes.Append(myVote);
        }

        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
    }
    else
    {
        sprintf(&(tempText[2]),"You cannot vote on this bill."); 
        lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
    }

    
}


//*******************************************************************************
void BBOServer::AttemptToStartVote(BBOSAvatar *curAvatar, TowerMap *guild, 
                                              char *subjectName, char *type)
{
    if (!curAvatar || !guild)
        return;

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    int messLen = 0;
    char tempText[1024];
//	char fullName[300];
//	char fullName2[300];
    int startingLine = TRUE;

    MessPlayerChatLine chatMess;
    MessInfoText       infoText;

    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
    tempText[1] = TEXT_COLOR_DATA;

    int highestRank = 0;
    MemberRecord *mr2 = NULL;
    MemberRecord *mr = (MemberRecord *) guild->members->First();
    while (mr)
    {
        if (IsCompletelySame(
                   curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, mr->WhoAmI()))
            mr2 = mr;

        if (mr->value1 > highestRank)
            highestRank = mr->value1;

        mr = (MemberRecord *) guild->members->Next();
    }

    if (!mr2)
    {
        sprintf(&chatMess.text[1],"You're not in your own guild.  Internal Error 1.");
        chatMess.text[0] = TEXT_COLOR_DATA;
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
        return;
    }

    // is there a valid slot for you to start a vote in?
    int votes = 0, commonVotes = 0, openIndex = -1;

    int isRoyalSponsor = mr2->value1;

    int commonTop = commonVotesAllowed[guild->guildStyle-1];

    int i = 0;
    for (; i < 4; ++i)
    {
        if (GUILDBILL_INACTIVE == guild->bills[i].type && -1 == openIndex)
        {
            if (isRoyalSponsor && i >= commonTop)
                openIndex = i;
            else if (!isRoyalSponsor && i < commonTop)
                openIndex = i;
            else if (0 == highestRank)
                openIndex = i;
        }
    }

    if (-1 == openIndex && isRoyalSponsor)
    {
        sprintf(&chatMess.text[1],"None of the %d vote slots for ranking sponsors are open.",
                        4 - commonTop);
        chatMess.text[0] = TEXT_COLOR_DATA;
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
        return;
    }

    if (-1 == openIndex && !isRoyalSponsor)
    {
        sprintf(&chatMess.text[1],"None of the %d vote slots for unranked sponsors are open.",
                        commonTop);
        chatMess.text[0] = TEXT_COLOR_DATA;
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
        return;
    }

    i = openIndex;

    // for each type
    guild->bills[i].type = GUILDBILL_INACTIVE;

    if (!stricmp(type, "PROMOTE"))
        guild->bills[i].type = GUILDBILL_PROMOTE;
    if (!stricmp(type, "DEMOTE"))
        guild->bills[i].type = GUILDBILL_DEMOTE;
    if (!stricmp(type, "KICK"))
        guild->bills[i].type = GUILDBILL_KICKOUT;
    if (!stricmp(type, "CHANGESTYLE"))
        guild->bills[i].type = GUILDBILL_CHANGESTYLE;
    if (!stricmp(type, "CHANGENAME"))
        guild->bills[i].type = GUILDBILL_CHANGENAME;
    if (!stricmp(type, "FIGHTER"))
        guild->bills[i].type = GUILDBILL_FIGHTER_SPEC;
    if (!stricmp(type, "MAGE"))
        guild->bills[i].type = GUILDBILL_MAGE_SPEC;
    if (!stricmp(type, "CRAFTER"))
        guild->bills[i].type = GUILDBILL_CRAFTER_SPEC;
        
    if (GUILDBILL_INACTIVE == guild->bills[i].type)
    {
        sprintf(&chatMess.text[1],"%s is not a valid vote type.",type);
        chatMess.text[0] = TEXT_COLOR_DATA;
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
        sprintf(&chatMess.text[1],"Try PROMOTE, DEMOTE, KICK, CHANGESTYLE, CHANGENAME, FIGHTER, MAGE, or CRAFTER.");
        chatMess.text[0] = TEXT_COLOR_DATA;
        lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
        return;
    }

    if (GUILDBILL_CHANGESTYLE != guild->bills[i].type &&
         GUILDBILL_CHANGENAME  != guild->bills[i].type)
    {
        // make sure the subjectName IS a guild member
        SharedSpace *guildSpace;
        if (!FindAvatarInGuild(subjectName, &guildSpace))
        {
            sprintf(&chatMess.text[1],"%s is not in the guild.", subjectName);
            chatMess.text[0] = TEXT_COLOR_DATA;
            lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
            guild->bills[i].type = GUILDBILL_INACTIVE;
            return;
        }
        if (guildSpace && guild != guildSpace)
        {
            sprintf(&chatMess.text[1],"%s is not in YOUR guild.", subjectName);
            chatMess.text[0] = TEXT_COLOR_DATA;
            lserver->SendMsg(sizeof(chatMess),(void *)&chatMess, 0, &tempReceiptList);
            guild->bills[i].type = GUILDBILL_INACTIVE;
            return;
        }
    }

    guild->bills[i].expTime.SetToNow();
    guild->bills[i].expTime.AddMinutes(60 * 2);
    guild->bills[i].sponsorLevel = mr2->value1;
    guild->bills[i].voteState = VOTESTATE_VOTING;
    CopyStringSafely(mr2->WhoAmI(), 64, guild->bills[i].sponsor, 64);
    CopyStringSafely(subjectName  , 64, guild->bills[i].subject, 64);

    mr = (MemberRecord *) guild->bills[i].recordedVotes.First();
    while (mr)
    {
        guild->bills[i].recordedVotes.Remove(mr);
        delete mr;

        mr = (MemberRecord *) guild->bills[i].recordedVotes.First();
    }

    sprintf(&(tempText[2]),"A new guild bill has been introduced!");

    SharedSpace *sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        SendToEveryGuildMate("NILLNILL",
                sp, guild, strlen(tempText) + 1,(void *)&tempText);
        sp = (SharedSpace *) spaceList->Next();
    }
    sprintf(&(tempText[2]),"Sponsor: %s.  Subject: %s.", 
        guild->bills[i].sponsor, guild->bills[i].subject);

    sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        SendToEveryGuildMate("NILLNILL",
                sp, guild, strlen(tempText) + 1,(void *)&tempText);
        sp = (SharedSpace *) spaceList->Next();
    }
    sprintf(&(tempText[2]),"%d: %s", i+1, guildBillDesc[guild->bills[i].type]);

    sp = (SharedSpace *) spaceList->First();
    while (sp)
    {
        bboServer->SendToEveryGuildMate("NILLNILL",
                sp, guild, strlen(tempText) + 1,(void *)&tempText);
        sp = (SharedSpace *) spaceList->Next();
    }
    
}

//*******************************************************************************
void BBOServer::SaltThisMeat(InventoryObject *io)
{
    char tempText[1024];

    InvMeat *im = (InvMeat *) io->extra;
    if (im->age >= 0)
    {
        if (im->age >= 24)
        {
            im->age = -2;
            sprintf_s(tempText,"Slt %s",io->WhoAmI());
            CopyStringSafely(tempText, 1024, io->do_name, DO_NAME_LENGTH);
            io->value *= 2;
        }
        else
        {
            im->age = -1;
            sprintf_s(tempText,"Slt %s",io->WhoAmI());
            CopyStringSafely(tempText, 1024, io->do_name, DO_NAME_LENGTH);
            io->value *= 2;
        }
    }
}

//*******************************************************************************
void BBOServer::CheckGraveyard(SharedSpace *ss, int gX, int gY)
{
    std::vector<TagID> tempReceiptList;

    for (int i = gY; i <= gY+1; ++i)
    {
        for (int j = gX; j <= gX+1; ++j)
        {
            Inventory *inv = ss->GetGroundInventory(j, i);
            int orchidCount = 0;
            InventoryObject *iObject = (InventoryObject *) inv->objects.First();
            while (iObject)
            {
                if (INVOBJ_SIMPLE == iObject->type && 
                     !strncmp("Dragon Orchid", iObject->WhoAmI(), strlen("Dragon Orchid"))
                    )
                    orchidCount += iObject->amount;

                iObject = (InventoryObject *) inv->objects.Next();
            }

            if (orchidCount > 0)
            {
                // okay, first, destroy the orchids
                iObject = (InventoryObject *) inv->objects.First();
                while (iObject)
                {
                    if (INVOBJ_SIMPLE == iObject->type && 
                         !strncmp("Dragon Orchid", iObject->WhoAmI(), strlen("Dragon Orchid"))
                        )
                    {
                        inv->objects.Remove(iObject);
                        delete iObject;
                        iObject = (InventoryObject *) inv->objects.First();
                    }
                    else
                        iObject = (InventoryObject *) inv->objects.Next();
                }

                // now, teleport anyone on this square to the dragon realm.
                BBOSMob *curMob = (BBOSMob *) ss->avatars->First();
                while (curMob)
                {
                    if (SMOB_AVATAR == curMob->WhatAmI())
                    {
                        BBOSAvatar *ca2 = (BBOSAvatar *) curMob;
                        if (ca2->cellX == j && ca2->cellY == i)
                        {

                            RealmMap *rp = NULL;
                            SharedSpace *sp2 = (SharedSpace *) spaceList->First();
                            while (sp2)
                            {
                                if (SPACE_GROUND == ss->WhatAmI() && SPACE_REALM == sp2->WhatAmI() && 
                                     REALM_ID_DRAGONS == ((RealmMap *)sp2)->type)
                                {
                                    rp = (RealmMap *)sp2;
                                    sp2 = (SharedSpace *) spaceList->Last();
                                }
                                sp2 = (SharedSpace *) spaceList->Next();
                            }

                            if (rp)
							{
								// gotta check for controlling monster
										// am i controlling a monster?
								if (ca2->controlledMonster && !ca2->followMode)
								{
									// and make my monster vanish too
									MessAdminMessage adminMess; // create an admin message to tell client that it's no longer controlling a monster so it doesn't get confused by the map change.
									tempReceiptList.clear();
									tempReceiptList.push_back(ca2->socketIndex); // point server at the target client.
									adminMess.messageType = MESS_ADMIN_RELEASE_CONTROL;
									lserver->SendMsg(sizeof(adminMess), (void *)&adminMess, 0, &tempReceiptList);
									// and resturn server to follow mode, since both will end up in the same spot.
									if (ca2->BeastStat() > 9)
										ca2->followMode = TRUE;
									// we need to dissapear it from people's screen.
								}
								// second, vanish me
								ca2->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
								// third, vanish the controlled monster if present
								if (ca2->controlledMonster)
								{
									// we need to dissapear it from people's screen.
									MessMonsterDeath messMD;
									messMD.mobID = (unsigned long)ca2->controlledMonster;
									ss->SendToEveryoneNearBut(0, ca2->controlledMonster->cellX, ca2->controlledMonster->cellY,
										sizeof(messMD), (void *)&messMD);
									// remove it fropm old map
									ss->mobList->Remove(ca2->controlledMonster);

								}

								// tell my client I'm entering the realm
								tempReceiptList.clear();
								tempReceiptList.push_back(ca2->socketIndex);

								// next adjust the coordinates
								ss->avatars->Remove(ca2);
								ca2->cellX = ca2->targetCellX = (rand() % 4) + 2;;
								ca2->cellY = ca2->targetCellY = (rand() % 4) + 2;;
								rp->avatars->Append(ca2);

								if (ca2->controlledMonster) // if they had a monster
								{
									// then it needs ot have it's coords updated, and be added to new map
									ca2->controlledMonster->cellX = ca2->controlledMonster->targetCellX = ca2->cellX;
									ca2->controlledMonster->cellY = ca2->controlledMonster->targetCellY = ca2->cellY;
									rp->mobList->Add(ca2->controlledMonster);
								}

								MessChangeMap changeMap;
								changeMap.realmID = rp->type;
								changeMap.oldType = ss->WhatAmI();
								changeMap.newType = rp->WhatAmI();
								changeMap.sizeX = rp->sizeX;
								changeMap.sizeY = rp->sizeY;
								changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize
								// it's a realm map, so set realmid
								changeMap.realmID = ((RealmMap *)rp)->type;
								lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);
								MessInfoText infoText;
								CopyStringSafely("You enter the Realm of Dragons.",
									200, infoText.text, MESSINFOTEXTLEN);
								lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

								ca2->QuestSpaceChange(ss, rp);

								// tell everyone about my arrival
								ca2->IntroduceMyself(rp, SPECIAL_APP_DUNGEON);
								if (ca2->controlledMonster)
								{
									// tell clients about my monster
									ca2->controlledMonster->AnnounceMyselfCustom(rp);
								}
								// tell this player about everyone else around
								ca2->UpdateClient(rp, TRUE);
							}
                        }
                    }
                    curMob = (BBOSMob *) ss->avatars->Next();
                }
            }
        }
    }
}


//*******************************************************************************
void BBOServer::HandleKeyCode(BBOSAvatar *avatar, MessKeyCode *keyCodePtr)
{
    // Game is free, no codes
}


//*******************************************************************************
void BBOServer::HandleKarmaText(char *string, BBOSAvatar *curAvatar, BBOSAvatar *targetAv)
{

    // KARMA
    if (StringContainsThanks(string))
    {
        int myRelationship, herRelationship;

        curAvatar->CompareWith(targetAv, myRelationship, herRelationship);

        curAvatar->charInfoArray[curAvatar->curCharacterIndex].
              karmaGiven[myRelationship][SAMARITAN_TYPE_THANKS] += 1;
        targetAv->charInfoArray[targetAv->curCharacterIndex].
              karmaReceived[herRelationship][SAMARITAN_TYPE_THANKS] += 1;

        curAvatar->LogKarmaExchange(
                     targetAv, myRelationship, herRelationship, 
                     SAMARITAN_TYPE_THANKS, string);

        if (curAvatar->IsAGuildMate(targetAv))
        {
            curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                  karmaGiven[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_THANKS] += 1;
            targetAv->charInfoArray[targetAv->curCharacterIndex].
                  karmaReceived[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_THANKS] += 1;

            curAvatar->LogKarmaExchange(
                         targetAv, SAMARITAN_REL_GUILD, SAMARITAN_REL_GUILD, 
                         SAMARITAN_TYPE_THANKS, string);
        }
    }
      
    if (StringContainsWelcome(string))
    {
        int myRelationship, herRelationship;

        curAvatar->CompareWith(targetAv, myRelationship, herRelationship);

        curAvatar->charInfoArray[curAvatar->curCharacterIndex].
              karmaGiven[myRelationship][SAMARITAN_TYPE_WELCOME] += 1;
        targetAv->charInfoArray[targetAv->curCharacterIndex].
              karmaReceived[herRelationship][SAMARITAN_TYPE_WELCOME] += 1;

        curAvatar->LogKarmaExchange(
                     targetAv, myRelationship, herRelationship, 
                     SAMARITAN_TYPE_WELCOME, string);

        if (curAvatar->IsAGuildMate(targetAv))
        {
            curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                  karmaGiven[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_WELCOME] += 1;
            targetAv->charInfoArray[targetAv->curCharacterIndex].
                  karmaReceived[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_WELCOME] += 1;

            curAvatar->LogKarmaExchange(
                         targetAv, SAMARITAN_REL_GUILD, SAMARITAN_REL_GUILD, 
                         SAMARITAN_TYPE_WELCOME, string);
        }
    }

    if (StringContainsPlease(string))
    {
        int myRelationship, herRelationship;

        curAvatar->CompareWith(targetAv, myRelationship, herRelationship);

        curAvatar->charInfoArray[curAvatar->curCharacterIndex].
              karmaGiven[myRelationship][SAMARITAN_TYPE_PLEASE] += 1;
        targetAv->charInfoArray[targetAv->curCharacterIndex].
              karmaReceived[herRelationship][SAMARITAN_TYPE_PLEASE] += 1;

        curAvatar->LogKarmaExchange(
                     targetAv, myRelationship, herRelationship, 
                     SAMARITAN_TYPE_PLEASE, string);

        if (curAvatar->IsAGuildMate(targetAv))
        {
            curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                  karmaGiven[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_PLEASE] += 1;
            targetAv->charInfoArray[targetAv->curCharacterIndex].
                  karmaReceived[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_PLEASE] += 1;

            curAvatar->LogKarmaExchange(
                         targetAv, SAMARITAN_REL_GUILD, SAMARITAN_REL_GUILD, 
                         SAMARITAN_TYPE_PLEASE, string);
        }
    }
    // END KARMA
}

//*******************************************************************************
void BBOServer::AddWarpPair(SharedSpace *s1, int x1, int y1,
                                     SharedSpace *s2, int x2, int y2, 
                                     int allUse)
{
    BBOSWarpPoint *wp = new BBOSWarpPoint(x1,y1);
    wp->targetX      = x2;
    wp->targetY      = y2;
    wp->spaceType    = s2->WhatAmI();
    wp->spaceSubType = 0;
    wp->allCanUse    = allUse;
    if (SPACE_REALM == s2->WhatAmI())
        wp->spaceSubType = ((RealmMap *) s2)->type;
    if (SPACE_LABYRINTH == s2->WhatAmI())
        wp->spaceSubType = ((LabyrinthMap *) s2)->type;
    s1->mobList->Add(wp);

    wp = new BBOSWarpPoint(x2,y2);
    wp->targetX      = x1;
    wp->targetY      = y1;
    wp->spaceType    = s1->WhatAmI();
    wp->spaceSubType = 0;
    if (SPACE_REALM == s1->WhatAmI())
        wp->spaceSubType = ((RealmMap *) s1)->type;
    if (SPACE_LABYRINTH == s1->WhatAmI())
        wp->spaceSubType = ((LabyrinthMap *) s1)->type;
    s2->mobList->Add(wp);

}

//*******************************************************************************
void BBOServer::CreateTreeQuest(Quest *quest, BBOSAvatar *curAvatar, 
                                          SharedSpace *sp,BBOSTree *t, int forcedType)
{
    char tempText[1024];
    char tempText2[128];

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    QuestPart *qp;

    int done;
    int tx, ty;
    int allDone = FALSE;
    int lowestMagic;
    InvSkill *skillInfo = NULL;
    InventoryObject *io;
    RealmMap *rp;
    DungeonMap *dp;
    SharedSpace *sp2;

    while (!allDone)
    {
        quest->EmptyOut();
        quest->completeVal = 0;  // active, but not complete
        quest->timeLeft.SetToNow();
        quest->timeLeft.AddMinutes(60*16); // add one day

        quest->questSource = t->index;  // 0-8 is the great trees

        if (-1 == forcedType)
            forcedType = rand() % 13;

        switch(forcedType)
//		switch(12)
        {
        case 0:	 // go-to location
        default:
            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_GOTO;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_LOCATION;
            
            done = FALSE;
            while (!done)
            {
                tx = (rand() % 245) + 5;
                ty = (rand() % 245) + 5;
                if (sp->CanMove(tx, ty, tx, ty))
                    done = TRUE;
            }

            qp->x = tx;
            qp->y = ty;
            qp->mapType = sp->WhatAmI();
            qp->mapSubType = 0; // needs to be correct
            qp->range = 0;

            sprintf(&(tempText[2]),"I felt a disturbance at %dN %dE.  Please go there, see if anything's happening, then report back to me.",
                                            256-ty,256-tx);
            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 1:	 // kill monster type

            if (MAGIC_MOON == t->index ||
                 MAGIC_TURTLE == t->index)
                 break;

            if ((MAGIC_SUN == t->index) && (rand() % 2))
                 break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_KILL;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_MONSTER_TYPE;
            
            done = FALSE;
            while (!done)
            {
                qp->monsterType    = rand() % NUM_OF_MONSTERS;
                qp->monsterSubType = rand() % NUM_OF_MONSTER_SUBTYPES;

                if (monsterData[qp->monsterType][qp->monsterSubType].name[0])
                    done = TRUE;
                if (7 == qp->monsterType && qp->monsterSubType > 2)
                    done = FALSE;
                if (MONSTER_PLACE_LABYRINTH == 
                     monsterData[qp->monsterType][qp->monsterSubType].placementFlags)
                    done = FALSE;
            }

            sprintf(&(tempText[2]),"As a test of your power, vanquish a %s in the name of my Mistress.",
                                      monsterData[qp->monsterType][qp->monsterSubType].name);
            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 2:	 // go up to another player and talk

            if (MAGIC_SNAKE == t->index ||
                 MAGIC_TURTLE == t->index)
                 break;

            if ((MAGIC_EAGLE == t->index) && (rand() % 2))
                 break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_VISIT;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_PLAYER;
            
            qp->playerType = rand() % QUEST_PLAYER_TYPE_MAX;

            switch(qp->playerType)
            {
            case QUEST_PLAYER_TYPE_FIGHTER:
                sprintf(&(tempText[2]),"Go to and bless one who has chosen the warrior path, in the name of my Mistress.");
                break;
            case QUEST_PLAYER_TYPE_MAGE   :
                sprintf(&(tempText[2]),"Go to and bless one who embraces the mystical powers, in the name of my Mistress.");
                break;
			case QUEST_PLAYER_TYPE_CRAFTER:
				sprintf(&(tempText[2]), "Go to and bless a creative soul in the name of my Mistress.");
				break;
			case QUEST_PLAYER_TYPE_BEASTMASTER:
				sprintf(&(tempText[2]), "Go to and bless a soul that works with the beasts in the name of my Mistress.");
				break;
			case QUEST_PLAYER_TYPE_BALANCED:
                sprintf(&(tempText[2]),"Go to and bless one who walks all paths equally, in the name of my Mistress.");
                break;
            case QUEST_PLAYER_TYPE_YOUNG  :
                sprintf(&(tempText[2]),"Go to and bless a newcomer to our world, in the name of my Mistress.");
                break;
            case QUEST_PLAYER_TYPE_POOR   :
                sprintf(&(tempText[2]),"Go to and bless a poor woman in the name of my Mistress.");
                break;
            }

            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 3:	 // craft a weapon

            if (MAGIC_MOON == t->index ||
                 MAGIC_BEAR == t->index)
                 break;

            if ((MAGIC_SNAKE == t->index) && (rand() % 2))
                 break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_CRAFT;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_WEAPON;

            // find and check the Swordsmith skill!
            skillInfo = NULL;
            io = (InventoryObject *) curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                       skills->objects.First();
            while (io)
            {
                if (!strcmp("Swordsmith",io->WhoAmI()))
                {
                    skillInfo = (InvSkill *) io->extra;
                }
                io = (InventoryObject *) curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                             skills->objects.Next();
            }
			if (!skillInfo )
				break;
			else if (skillInfo->skillLevel < 20)
                qp->playerType = QUEST_WEAPON_TYPE_SWORD;
            else
                qp->playerType = rand() % QUEST_WEAPON_TYPE_MAX;

            sprintf(&(tempText[2]),"It would please my Mistress to have you create a fine %s and sacrifice it to me.",
                questWeaponTypeDesc[qp->playerType]);

            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 4:	 // imbue a totem

            if (MAGIC_WOLF == t->index ||
                MAGIC_SUN  == t->index ||
                 MAGIC_BEAR == t->index)
                 break;

            if ((MAGIC_EAGLE == t->index) && (rand() % 2))
                 break;

            if ((MAGIC_FROG == t->index) && (rand() % 2))
                 break;
			lowestMagic = 1000;
			skillInfo = NULL;

			for (tx = 0; tx < MAGIC_MAX; ++tx)
			{
				sprintf_s(tempText, "%s Magic", magicNameList[tx]);
				io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].
					skills->objects.First();
				while (io)
				{
					if (IsSame(tempText, io->WhoAmI()))
					{
						skillInfo = (InvSkill *)io->extra;
						if (lowestMagic > skillInfo->skillLevel)
							lowestMagic = skillInfo->skillLevel;
					}
					io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].
						skills->objects.Next();
				}
			}

			if (lowestMagic < 1 || lowestMagic > 990)
				break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_IMBUE;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_TOTEM;

            qp->playerType = rand() % TOTEM_PHYSICAL;

            sprintf(&(tempText[2]),"It would please my Mistress to have you imbue a perfect %s totem and sacrifice it to me.",
                totemTypeName[qp->playerType]);

            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 5:	 // imbue a staff

            if (MAGIC_FROG == t->index ||
                MAGIC_EAGLE == t->index ||
                 MAGIC_BEAR == t->index)
                 break;

            if ((MAGIC_WOLF == t->index) && (rand() % 2))
                 break;

            // find and check the magic skills!
            lowestMagic = 1000;
            skillInfo = NULL;

            for (tx = 0; tx < MAGIC_MAX; ++tx)
            {
                sprintf_s(tempText,"%s Magic",magicNameList[tx]);
                io = (InventoryObject *) curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                                  skills->objects.First();
                while (io)
                {
                    if (IsSame(tempText,io->WhoAmI()))
                    {
                        skillInfo = (InvSkill *) io->extra;
                        if (lowestMagic > skillInfo->skillLevel)
                            lowestMagic = skillInfo->skillLevel;
                    }
                    io = (InventoryObject *) curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                                                         skills->objects.Next();
                }
            }

            if (lowestMagic < 6 || lowestMagic > 990)
                break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_IMBUE;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_STAFF;

            qp->playerType = rand() % STAFF_MAX;

            sprintf(&(tempText[2]),"It would please my Mistress to have you imbue a perfect %s staff and sacrifice it to me.",
                staffTypeName[qp->playerType]);

            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 6:	 // go up to another player and give them money

            if (MAGIC_SNAKE == t->index ||
                MAGIC_EAGLE == t->index ||
                 MAGIC_WOLF == t->index)
                 break;

            if ((MAGIC_SUN == t->index) && (rand() % 2))
                 break;

            if ((MAGIC_FROG == t->index) && (rand() % 2))
                 break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_GIVEGOLD;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_PLAYER;
            
            qp->range = curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money / 10;
            if (qp->range < 0 || qp->range > 10000)
                qp->range = 10000;

            qp->playerType = rand() % QUEST_PLAYER_TYPE_MAX;

            switch(qp->playerType)
            {
            case QUEST_PLAYER_TYPE_FIGHTER:
                sprintf(&(tempText[2]),"Go to and give alms to one who has chosen the warrior path, in the name of my Mistress.");
                break;
            case QUEST_PLAYER_TYPE_MAGE   :
                sprintf(&(tempText[2]),"Go to and give alms to one who embraces the mystical powers, in the name of my Mistress.");
                break;
			case QUEST_PLAYER_TYPE_CRAFTER:
				sprintf(&(tempText[2]), "Go to and give alms to a creative soul in the name of my Mistress.");
				break;
			case QUEST_PLAYER_TYPE_BEASTMASTER:
				sprintf(&(tempText[2]), "Go to and give alms to a soul that works with the beasts in the name of my Mistress.");
				break;
			case QUEST_PLAYER_TYPE_BALANCED:
                sprintf(&(tempText[2]),"Go to and give alms to one who walks all paths equally, in the name of my Mistress.");
                break;
            case QUEST_PLAYER_TYPE_YOUNG  :
                sprintf(&(tempText[2]),"Go to and give alms to a newcomer to our world, in the name of my Mistress.");
                break;
            case QUEST_PLAYER_TYPE_POOR   :
                sprintf(&(tempText[2]),"Go to and give alms to a poor woman in the name of my Mistress.");
                break;
            }

            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 7:	 // retrieve from location
            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_RETRIEVE;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_LOCATION;

            qp->mapSubType = rand() % (REALM_ID_DRAGONS+1);
            
            rp = NULL;
            sp2 = (SharedSpace *) spaceList->First();
            while (sp2)
            {
                if (SPACE_REALM == sp2->WhatAmI() && qp->mapSubType == ((RealmMap *)sp2)->type)
                {
                    rp = (RealmMap *)sp2;
                    sp2 = (SharedSpace *) spaceList->Last();
                }
                sp2 = (SharedSpace *) spaceList->Next();
            }

            if (rp)
            {
                done = FALSE;
                while (!done)
                {
                    tx = (rand() % (rp->width -8)) + 4;
                    ty = (rand() % (rp->height-8)) + 4;
                    if (rp->CanMove(tx, ty, tx, ty))
                        done = TRUE;
                }

                qp->x = tx;
                qp->y = ty;
                qp->mapType = SPACE_REALM;
                qp->range = 0;

                sprintf(&(tempText[2]),"Most embarrassing.  My Mistress dropped a trifle at %dN %dE in the %s. Please go there, search the ground, and bring the item back to me.",
                                                64-ty,64-tx, rp->WhoAmI());
                tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                tempText[1] = TEXT_COLOR_TELL;
                lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                allDone = TRUE;
            }
            break;

        case 8:	 // retrieve from location
            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_RETRIEVE;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_NPC;

            do
            {
                qp->monsterType = rand() % MAGIC_MAX;
            } while (qp->monsterType == quest->questSource);
            
            sprintf(&(tempText[2]),"The Great Tree of the %s has something my Mistress wants.  Please go to the Tree and bring the item back to me.",
                                         magicNameList[qp->monsterType]);
            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 9:	 // kill monster (specific)

            if (MAGIC_MOON == t->index)
                 break;
            if (MAGIC_SNAKE == t->index)
                 break;

            if ((MAGIC_TURTLE == t->index) && (rand() % 2))
                 break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_KILL;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_LOCATION;
            
            CreateDemonPrince(sp, curAvatar, qp, tempText2);

            sprintf(&(tempText[2]),"The Demon King grows bold, and his brood have invaded the darkest places under our land. I can feel one right now in the %s.  Destroy it for my Mistress.",
                                      tempText2);
            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 10:	 // collect stuff and sacrifice it

            if (MAGIC_BEAR == t->index ||
                MAGIC_WOLF == t->index ||
                MAGIC_FROG == t->index ||
                 MAGIC_TURTLE == t->index)
                 break;

            if ((MAGIC_SUN == t->index) && (rand() % 2))
                 break;

            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_COLLECT;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            if (rand() % 2)
            {
                qp->type = QUEST_TARGET_EGG;
                qp->monsterType = rand() % (DRAGON_TYPE_NUM-1);
                qp->monsterSubType = 1;
                sprintf(&(tempText[2]),"You seek a challenge?  Then find and give to me %d %s.",
                    qp->monsterSubType,
                    dragonInfo[0][qp->monsterType].eggName);
            }
            else
            {
                qp->type = QUEST_TARGET_DUST;
                qp->monsterType = rand() % INGR_WHITE_SHARD;
                if (0 == qp->monsterType || 2 == qp->monsterType)
                    qp->monsterType = 1;
                qp->monsterSubType = 1;
                sprintf(&(tempText[2]),"You seek a challenge?  Then find and give to me %d %s.",
                    qp->monsterSubType,
                    dustNames[qp->monsterType]);
            }

            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 11:	 // get together on a spot with others and say a power word

            if (MAGIC_SNAKE == t->index)
                 break;

            if ((MAGIC_EAGLE == t->index) && (rand() % 2))
                 break;
			// make sure there's actually enogh players online to pray with you
			// get count of all players
			{
				int numavatars = 0;
				BBOSAvatar *countAvatar = (BBOSAvatar *)this->incoming->First();
				while (countAvatar)
				{
					numavatars++;
					countAvatar = (BBOSAvatar *)this->incoming->Next();
				}
				if (numavatars < 3)
				{
					break;
				}
			}
			// now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_GROUP;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_LOCATION;
            qp->mapType = SPACE_GROUND;

            switch(rand() % 4)
            {
            case 0:
            default:
                qp->x = 230;
                qp->y = 53;
                break;
            case 1:
                qp->x = 20;
                qp->y = 23;
                break;
            case 2:
                qp->x = 179;
                qp->y = 164;
                break;
            case 3:
                qp->x = 107;
                qp->y = 162;
                break;
            }

            sprintf(&(tempText[2]),"Honor my Mistress by going to the Ring and praying with two others.");
            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;
            break;

        case 12: // escape dungeon

            if (MAGIC_MOON == t->index)
                 break;
            if (MAGIC_SUN == t->index)
                 break;

            if ((MAGIC_TURTLE == t->index) && (rand() % 2))
                 break;
            if ((MAGIC_BEAR == t->index) && (rand() % 2))
                 break;


            // now add questParts
            qp = new QuestPart(QUEST_PART_VERB, "VERB");
            quest->parts.Append(qp);
            qp->type = QUEST_VERB_ESCAPE;

            qp = new QuestPart(QUEST_PART_TARGET, "TARGET");
            quest->parts.Append(qp);

            qp->type = QUEST_TARGET_SPACE;
            qp->mapType = SPACE_DUNGEON;
            qp->mapSubType = SPACE_GROUND;
            
            dp = NULL;
            sp2 = NULL;
            while (!dp)
            {
                if (!sp2)
                    sp2 = (SharedSpace *) spaceList->First();

                if (SPACE_DUNGEON == sp2->WhatAmI() && !(rand() % 40))
                    dp = (DungeonMap *)sp2;

                sp2 = (SharedSpace *) spaceList->Next();
            }

            sprintf(&(tempText[2]),"You seek a challenge?  Then get out of here alive, and I'll reward you!");
            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
            tempText[1] = TEXT_COLOR_TELL;
            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
            allDone = TRUE;

            curAvatar->AnnounceDisappearing(sp, SPECIAL_APP_DUNGEON);
            // and if i have a monster, tell people it's going away too
			if (curAvatar->controlledMonster)
			{
				// we need to dissapear it from people's screen.
				MessMonsterDeath messMD;
				messMD.mobID = (unsigned long)curAvatar->controlledMonster;
				sp->SendToEveryoneNearBut(0, curAvatar->controlledMonster->cellX, curAvatar->controlledMonster->cellY,
					sizeof(messMD), (void*)&messMD); // 
				// remove it from old map
				sp->mobList->Remove(curAvatar->controlledMonster);
			}
	    
            // tell my client I'm entering the dungeon
            MessChangeMap changeMap;
            changeMap.dungeonID = (long) dp;
            changeMap.oldType = sp->WhatAmI();
            changeMap.newType = dp->WhatAmI();
            changeMap.sizeX   = dp->width;
            changeMap.sizeY   = dp->height;
            changeMap.flags   = MESS_CHANGE_NOTHING;

            // move me to my new SharedSpace
            sp->avatars->Remove(curAvatar);
            dp->avatars->Append(curAvatar);

            curAvatar->cellX = 0;
            curAvatar->cellY = 0;
			if (curAvatar->controlledMonster) // if they had a monster
			{
				// then it needs ot have it's coords updated, and be added to new map
				curAvatar->controlledMonster->cellX = curAvatar->controlledMonster->targetCellX = curAvatar->cellX;
				curAvatar->controlledMonster->cellY = curAvatar->controlledMonster->targetCellY = curAvatar->cellY;
				dp->mobList->Add(curAvatar->controlledMonster);
			}
			// NOW send the changemap packet.
            lserver->SendMsg(sizeof(changeMap),(void *)&changeMap, 0, &tempReceiptList);

            // tell everyone about my arrival
            curAvatar->IntroduceMyself(dp, SPECIAL_APP_DUNGEON);
            // and introduce the monster if i have one
			if (curAvatar->controlledMonster) // if they had a monster
			{
				// monster announces itself too
				curAvatar->controlledMonster->AnnounceMyselfCustom(dp);
			}

            // tell this player about everyone else around
            curAvatar->UpdateClient(dp, TRUE);
            allDone = TRUE;
            break;

        }

        forcedType = rand() % 13;

    }
}

//*******************************************************************************
int BBOServer::ResolveTreeQuests(BBOSAvatar *curAvatar, 
                                          SharedSpace *sp,BBOSTree *t)
{
    char tempText[1024];

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(curAvatar->socketIndex);

    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *quest = &(curAvatar->charInfoArray[curAvatar->curCharacterIndex].quests[i]);

        if (10000 == quest->completeVal)  // if quest complete
        {
            if (quest->questSource == t->index) // quest came from this tree
            {
                int giveFavor = FALSE;
                QuestPart *qp = quest->GetVerb();
                if (qp && (QUEST_VERB_IMBUE == qp->type || 
                            QUEST_VERB_CRAFT == qp->type || 
                              QUEST_VERB_RETRIEVE == qp->type))
                {
                    QuestPart *qt = quest->GetTarget();
                    if (qt)
                    {
                        if (curAvatar->SacrificeQuestItem(qt->type, qt->playerType))
                        {
                            if (QUEST_VERB_RETRIEVE == qp->type)
                                sprintf(&(tempText[2]),"My Mistress thanks you for retrieving this item.  Please accept this favor as your reward.");
                            else
                                sprintf(&(tempText[2]),"My Mistress thanks you for the gift of this item.  Please accept this favor as your reward.");
                            giveFavor = TRUE;

                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_TELL;
                            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

  //							curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money += 100;
                        }
                        else
                        {
                            if (QUEST_VERB_RETRIEVE == qp->type)
                                sprintf(&(tempText[2]),"You found the item I told you of, but you no longer have it to give to me.  I am disappointed.");
                            else
                                sprintf(&(tempText[2]),"You created the item I asked, but you no longer have it to give to me.  I am saddened.");

                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_TELL;
                            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                        }
                    }
                }
                else
                {
                    sprintf(&(tempText[2]),"Thank you for your work on behalf of my Mistress.  Please accept this favor as your reward.");
                    tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                    tempText[1] = TEXT_COLOR_TELL;
                    lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);

                    giveFavor = TRUE;
//					curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->money += 100;
                }

                if (giveFavor)
                {
                    if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                         cLevel >= 10)
                    {
                        sprintf_s(tempText,"%s Favor", magicNameList[t->index]);

                        InventoryObject *iObject = 
                             new InventoryObject(INVOBJ_FAVOR, 0, tempText);
                        iObject->mass = 1.0f;
                        iObject->value = 10;
						iObject->amount = curAvatar->charInfoArray[curAvatar->curCharacterIndex].cLevel / 20 + 1;

                        ((InvFavor *) iObject->extra)->spirit = t->index;
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->
                                          AddItemSorted(iObject);
                    }
                    else
                    {
                        InventoryObject *iObject = 
                             new InventoryObject(INVOBJ_SIMPLE, 0, "Paper Favor");
                        iObject->mass = 1.0f;
                        iObject->value = 200;
                        iObject->amount = 1;
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->
                                          AddItemSorted(iObject);
                    }

                    MessGenericEffect messGE;
                    messGE.avatarID = curAvatar->socketIndex;
                    messGE.mobID    = 0;
                    messGE.x        = curAvatar->cellX;
                    messGE.y        = curAvatar->cellY;
                    messGE.r        = 100;
                    messGE.g        = 100;
                    messGE.b        = 255;
                    messGE.type     = 0;  // type of particles
                    messGE.timeLen  = 2; // in seconds
                    sp->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      sizeof(messGE),(void *)&messGE);

                }

                quest->EmptyOut();
                curAvatar->SaveAccount();
                return TRUE;
            }
        }
        else // incomplete quest...
        {
            if (quest->questSource == t->index) // quest came from this tree
            {
                int giveFavor = FALSE;
                QuestPart *qp = quest->GetVerb();
                if (qp && (QUEST_VERB_COLLECT == qp->type))
                {
                    int amount = 0, total = 0;

                    QuestPart *qt = quest->GetTarget();
                    switch(qt->type)
                    {
                    case QUEST_TARGET_EGG:
                    default:
                        do 
                        {
                            amount = curAvatar->SacrificeQuestItem(qt->type, qt->monsterType);
                            total += amount;
                        } while(amount && quest->completeVal + total < qt->monsterSubType);

                        quest->completeVal += total;
                        if (quest->completeVal >= qt->monsterSubType)
                        {
                            sprintf(&(tempText[2]),"I now take from you all the eggs I asked for.  Please accept this favor as your reward.");
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_TELL;
                            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                            giveFavor = TRUE;
                        }
                        else if (total > 0)
                        {
                            sprintf(&(tempText[2]),"I now take from you %d of the eggs I asked for.  Your task is still to bring me %d more.",
                                         total, qt->monsterSubType - quest->completeVal);
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_TELL;
                            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                            curAvatar->SaveAccount();
                        }

                        break;

                    case QUEST_TARGET_DUST:
                        do 
                        {
                            amount = curAvatar->SacrificeQuestItem(qt->type, qt->monsterType);
                            total += amount;
                        } while(amount && quest->completeVal + total < qt->monsterSubType);

                        quest->completeVal += total;
                        if (quest->completeVal >= qt->monsterSubType)
                        {
                            sprintf(&(tempText[2]),"I now take from you all the dust I asked for.  Please accept this favor as your reward.");
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_TELL;
                            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                            giveFavor = TRUE;
                        }
                        else if (total > 0)
                        {
                            sprintf(&(tempText[2]),"I now take from you %d of the dust I asked for.  Your task is still to bring me %d more.",
                                         total, qt->monsterSubType - quest->completeVal);
                            tempText[0] = NWMESS_PLAYER_CHAT_LINE;
                            tempText[1] = TEXT_COLOR_TELL;
                            lserver->SendMsg( strlen(tempText) + 1,(void *)&tempText, 0, &tempReceiptList);
                            curAvatar->SaveAccount();
                        }

                        break;
                    }
                }

                if (giveFavor)
                {
                    if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].
                         cLevel >= 10)
                    {
                        sprintf_s(tempText,"%s Favor", magicNameList[t->index]);

                        InventoryObject *iObject = 
                             new InventoryObject(INVOBJ_FAVOR, 0, tempText);
                        iObject->mass = 1.0f;
                        iObject->value = 10;
						iObject->amount = curAvatar->charInfoArray[curAvatar->curCharacterIndex].cLevel / 20 + 1;

                        ((InvFavor *) iObject->extra)->spirit = t->index;
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->
                                          AddItemSorted(iObject);
                    }
                    else
                    {
                        InventoryObject *iObject = 
                             new InventoryObject(INVOBJ_SIMPLE, 0, "Paper Favor");
                        iObject->mass = 1.0f;
                        iObject->value = 200;
                        iObject->amount = 1;
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].inventory->
                                          AddItemSorted(iObject);
                    }

                    quest->EmptyOut();
                    curAvatar->SaveAccount();

                    MessGenericEffect messGE;
                    messGE.avatarID = curAvatar->socketIndex;
                    messGE.mobID    = 0;
                    messGE.x        = curAvatar->cellX;
                    messGE.y        = curAvatar->cellY;
                    messGE.r        = 100;
                    messGE.g        = 100;
                    messGE.b        = 255;
                    messGE.type     = 0;  // type of particles
                    messGE.timeLen  = 2; // in seconds
                    sp->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
                                      sizeof(messGE),(void *)&messGE);

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}


//******************************************************************
void BBOServer::CreateDemonPrince(SharedSpace *ss, BBOSAvatar *curAvatar, 
                                             QuestPart *qt, char *tempText2)
{

    float totalPlayerPower = 0;
    //	totals up their power
    //		power = dodge skill + age + sword damage + sword to-hit + sword magic
    totalPlayerPower += 
             curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime / 20;

    totalPlayerPower += 
        curAvatar->GetDodgeLevel();

    totalPlayerPower += curAvatar->BestSwordRating();
	totalPlayerPower /= 2;

    //	randomly picks a dungeon
    SharedSpace *tempss = (SharedSpace *) spaceList->First();
    SharedSpace *dungeonPicked = NULL;
    while (!dungeonPicked)
    {
        if (SPACE_DUNGEON == tempss->WhatAmI() && 0 == ((DungeonMap *) tempss)->specialFlags )
        {
            if (36 == rand() % 100)
            {
                DungeonMap *dm = (DungeonMap *) tempss;
//				if (!IsCompletelySame(dm->masterName, curAvatar->name))
                    dungeonPicked = tempss;
            }
        }
        tempss = (SharedSpace *) spaceList->Next();
        if (!tempss)
            tempss = (SharedSpace *) spaceList->First();
    }

    //	creates a Possessed monster in the back of the the front half of the dungeon
    int mPosX = rand() % ((DungeonMap *)dungeonPicked)->width;
    int mPosY = rand() % ((DungeonMap *)dungeonPicked)->height;

    qt->monsterType    = 0; // means a demon mino
	int totalPlayerPower2 = (int)totalPlayerPower;
	if (totalPlayerPower2 < 1) // if it overflowed
		totalPlayerPower2 = 2147483647;
	qt->monsterSubType = totalPlayerPower2;
    qt->mapType = SPACE_DUNGEON;
    qt->x = mPosX;
    qt->y = mPosY;

    DungeonMap *dm = (DungeonMap *)dungeonPicked;
    qt->mapSubType = GetCRCForString(dm->name);
    sprintf(tempText2, dm->name);
}


//******************************************************************
BBOSAvatar *BBOServer::FindAvatarByStringName(char *string, int &length, SharedSpace **retSpace) 
{
    int done = FALSE;
    int found = FALSE;
    int argPoint = 0;
    int linePoint = 0;
    SharedSpace *sp;
    char tempText[512];

    BBOSAvatar *retAvatar = NULL, *rAv;

    while (!found && !done)
    {
        argPoint = NextWord(string,&linePoint);
        if (argPoint == linePoint)
            done = TRUE;
        else
        {
            memcpy(tempText,string, linePoint);
            int back = 1;
            tempText[linePoint] = 0;
            while (' ' == tempText[linePoint-back] && linePoint-back > 0)
            {
                tempText[linePoint-back] = 0;
                ++back;
            }
            rAv = FindAvatarByAvatarName((char *)tempText, &sp);
            if (rAv)
            {
                retAvatar = rAv;
                length = linePoint;
                *retSpace = sp;
            }
        }
    }

    return retAvatar;
}

//******************************************************************
// as much as i'd like to reuse the function, ther's no parameter to tell it not to create the resume, so i need a new copy

void BBOServer::HandleEarthKeyResumeUse(BBOSAvatar *ca, InvEarthKey *iek, SharedSpace *ss)
{
	char tempText[1024];
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(ca->socketIndex);
	BBOSMonster *monster;
	int rottedearthkey = 0;

	//	if (iek->monsterType[0] == 27 || iek->monsterType[0] == 28)
	//		iek->monsterType[0] = 0;

	//	if (iek->monsterType[1] == 27 || iek->monsterType[1] == 28)
	//		iek->monsterType[1] = 0;
	if (iek->monsterType[0] == 27) // no need to remove lizardmen now, no dusts.
		iek->monsterType[0] = 0;

	if (iek->monsterType[1] == 27)
		iek->monsterType[1] = 0;
	if (iek->width<0)
	{
		rottedearthkey = 1;
		iek->width *= -1;
	}
	if (iek->height<0)
	{
		rottedearthkey = 1;
		iek->height *= -1;
	}
	// make the new space
	DungeonMap *dm = new DungeonMap(SPACE_DUNGEON, "Test Dungeon", lserver);
	dm->specialFlags = SPECIAL_DUNGEON_TEMPORARY;
	dm->InitNew(iek->width, iek->height, ca->cellX, ca->cellY, 0);
	sprintf_s(dm->name, "%s's %s %s",
		ca->charInfoArray[ca->curCharacterIndex].name,
		rdNameAdjective[rand() % 5], rdNamePre[rand() % 7]);
	dm->maxdestroyed = (dm->NumberOfWalls/6);  // bigger geos allow you to blast more wallls before they break.
	dm->tempPower = iek->power;
	int depth = iek->power + 20;
	dm->GeoDepth = depth;                      // store it so bombs cna check it.
	spaceList->Append(dm);
	int col = 0;

	float change = 1 + iek->power / 20;
	if (rottedearthkey == 1)
		change += 0.25; // 5 meter boost for rotted monsters.
	float size1 = 1 + iek->power / 50;
	float size2 = 1 + iek->power / 80;
	float resist = Bracket(iek->power / 100.0f, 0.0f, 100000.0f);

	int t = iek->monsterType[0];
	int maxmonsters = (dm->width*dm->height) / 6 + rand() % 5; //randomize so count doesn't reveal if there is a vagabond or not. :)
	int maxmonsters2 = (dm->width*dm->height) / 6 + rand() % 5; //randomize so count doesn't reveal if there is a vagabond or not. :)
																// add static monsters
	for (int m = 0; m < maxmonsters;) // first spawn sweep
	{
		int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

		if (monsterData[t][t2].name[0])
		{
			int mx, my;
			do
			{
				mx = rand() % (dm->width);
				my = rand() % (dm->height);
			} while (mx < 1);

			monster = new BBOSMonster(t, t2, NULL);
			monster->cellX = mx;
			monster->cellY = my;
			monster->targetCellX = mx;
			monster->targetCellY = my;
			monster->spawnX = mx;
			monster->spawnY = my;
			if (rottedearthkey == 1)
			{
				sprintf_s(monster->uniqueName, "%s %s", "Rotted", monster->Name());
				monster->r = 255;
				monster->g = 0;
				monster->b = 0;
			}
			monster->damageDone = monster->damageDone * change;
			monster->defense = monster->defense    * change;
			monster->dropAmount = monster->dropAmount * change;
			monster->health = monster->health     * change;
			monster->maxHealth = monster->maxHealth  * change;
			monster->toHit = monster->toHit      * change;
			monster->dontRespawn = true;
			if (depth > 22)
			{
				monster->tamingcounter = 1000; // no taming pre boosted monsters!
			}
			if (monsterData[t][t2].placementFlags != MONSTER_PLACE_DUNGEON)
			{
				monster->tamingcounter = 1000; // no taming monsters you can find outside of a dungeon!
			}
			monster->magicResistance = resist;
			if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
				monster->magicResistance = 0.99f;
			dm->mobList->Add(monster);
			++m;
			++dm->MonsterCount;
		}
	}

	if (iek->monsterType[1] > -1)
		t = iek->monsterType[1];

	for (int m = 0; m < maxmonsters2;) // second spawn sweep. same monster unless it's  double meat geto
	{
		int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

		if (monsterData[t][t2].name[0])
		{
			int mx, my;
			do
			{
				mx = rand() % (dm->width);
				my = rand() % (dm->height);
			} while (mx < 1);

			monster = new BBOSMonster(t, t2, NULL);
			monster->cellX = mx;
			monster->cellY = my;
			monster->targetCellX = mx;
			monster->targetCellY = my;
			monster->spawnX = mx;
			monster->spawnY = my;
			monster->dontRespawn = true;

			monster->damageDone = monster->damageDone * change;
			monster->defense = monster->defense    * change;
			monster->dropAmount = monster->dropAmount * change;
			monster->health = monster->health     * change;
			monster->maxHealth = monster->maxHealth  * change;
			monster->toHit = monster->toHit      * change;

			monster->magicResistance = resist;
			if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
				monster->magicResistance = 0.99f;
			if (rottedearthkey == 1)
			{
				sprintf_s(monster->uniqueName, "%s %s", "Rotted", monster->Name());
				monster->r = 255;
				monster->g = 0;
				monster->b = 0;
			}
			if (depth > 22)
			{
				monster->tamingcounter = 1000; // no taming pre boosted monsters!
			}
			if (monsterData[t][t2].placementFlags != MONSTER_PLACE_DUNGEON)
			{
				monster->tamingcounter = 1000; // no taming monsters you can find outside of a dungeon!
			}
			dm->mobList->Add(monster);
			++m;
			++dm->MonsterCount;
		}
	}

	// add boss monster
	int i = 0;
	for (i = NUM_OF_MONSTER_SUBTYPES - 1;
		i >= 0 && 0 == monsterData[t][i].name[0];
		--i)
		;

	monster = new BBOSMonster(t, i, NULL);
	if (rottedearthkey == 1)
		sprintf_s(monster->uniqueName, "Rotted Sentinel");
	else
		sprintf_s(monster->uniqueName, "Sentinel");
	monster->sizeCoeff = size1;

	monster->damageDone = monster->damageDone * change * 2;
	monster->defense = monster->defense    * change * 2;
	monster->dropAmount = monster->dropAmount * change * 2;
	monster->health = monster->health     * change * 2;
	monster->maxHealth = monster->maxHealth  * change * 2;
	monster->toHit = monster->toHit      * change * 2;
	monster->tamingcounter = 1000; // no taming pre boosted monsters!

	monster->magicResistance = Bracket(resist, 0.5f, 1000000.0f);
	if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
		monster->magicResistance = 0.99f;
	monster->r = 0;
	monster->g = 0;
	monster->b = 255;

	monster->targetCellX = monster->spawnX = monster->cellX = dm->width - 3;
	monster->targetCellY = monster->spawnY = monster->cellY = 0;
	monster->dontRespawn = true;

	dm->mobList->Add(monster);
	++dm->MonsterCount;

	// sometimes add a Vagabond monster
	if (!(rand() % 3))
	{
		int vagIndex = t + 1;

		if (21 == vagIndex || vagIndex >= NUM_OF_MONSTERS-2) // last two monster types are not currnetly valid vagabonds
			vagIndex = 0;

		for (i = NUM_OF_MONSTER_SUBTYPES - 1;
			i >= 0 && 0 == monsterData[vagIndex][i].name[0];
			--i)
			;

		monster = new BBOSMonster(vagIndex, i, NULL, true);
		if (rottedearthkey == 1)
			sprintf_s(monster->uniqueName, "Rotted Vagabond");
		else
			sprintf_s(monster->uniqueName, "Vagabond");
		monster->sizeCoeff = size2;

		monster->damageDone = monster->damageDone * change * 1.5f;
		monster->defense = monster->defense    * change * 1.5f;
		monster->dropAmount = monster->dropAmount * change * 1.5f;
		monster->health = monster->health     * change * 1.5f;
		monster->maxHealth = monster->maxHealth  * change * 1.5f;
		monster->toHit = monster->toHit      * change * 1.5f;

		monster->magicResistance = Bracket(resist, 0.8f, 100000000.0f);
		if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
			monster->magicResistance = 0.99f;

		monster->r = 0;
		monster->g = 255;
		monster->b = 255;
		monster->tamingcounter = 1000; // no taming pre boosted monsters!

		if (rand() % 2)
		{
			monster->targetCellX = monster->spawnX = monster->cellX = dm->width * 3 / 4;
			monster->targetCellY = monster->spawnY = monster->cellY = dm->height * 3 / 4;
		}
		else
		{
			monster->targetCellX = monster->spawnX = monster->cellX = dm->width / 4;
			monster->targetCellY = monster->spawnY = monster->cellY = dm->height / 4;
		}
		monster->dontRespawn = true;
		dm->mobList->Add(monster);
		++dm->MonsterCount;

		monster->AddPossessedLoot(1 + iek->power / 10);

		BBOSGroundEffect *bboGE = new BBOSGroundEffect();

		bboGE->type = 4;
		bboGE->amount = 3;
		bboGE->r = 255;
		bboGE->g = 255;
		bboGE->b = 0;
		bboGE->cellX = monster->cellX;
		bboGE->cellY = monster->cellY;

		dm->mobList->Add(bboGE);

	}

	// add a portal out of it
	BBOSWarpPoint *wp = new BBOSWarpPoint(dm->width - 1, 0);
	wp->targetX = dm->enterX;
	wp->targetY = dm->enterY;
	wp->spaceType = SPACE_GROUND;
	wp->spaceSubType = 0;
	dm->mobList->Add(wp);

	BBOSChest *chest = new BBOSChest(dm->width - 2, 0);
	dm->mobList->Add(chest);

	if (28 == iek->monsterType[0] && 29 == iek->monsterType[1])
	{
		// add a portal to the Deep Laby
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 33;
		wp->targetY = 33;
		wp->spaceType = SPACE_LABYRINTH;
		wp->spaceSubType = REALM_ID_LAB3;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if (24 == iek->monsterType[0] && 28 == iek->monsterType[1])
	{
		// add a portal to the Deep Laby
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 27;
		wp->targetY = 25;
		wp->spaceType = SPACE_LABYRINTH;
		wp->spaceSubType = REALM_ID_LAB2;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if (8 == iek->monsterType[0] && 24 == iek->monsterType[1])
	{
		// add a portal to the Laby
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 25;
		wp->targetY = 25;
		wp->spaceType = SPACE_LABYRINTH;
		wp->spaceSubType = REALM_ID_LAB1;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if (9 == iek->monsterType[0] && 22 == iek->monsterType[1])
	{
		// add a portal to the Dragon realm
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 57;
		wp->targetY = 6;
		wp->spaceType = SPACE_REALM;
		wp->spaceSubType = REALM_ID_DRAGONS;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if (5 == iek->monsterType[0] && 12 == iek->monsterType[1])
	{
		// add a portal to the spirit realm
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 61;
		wp->targetY = 8;
		wp->spaceType = SPACE_REALM;
		wp->spaceSubType = REALM_ID_SPIRITS;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if (7 == iek->monsterType[0] && 19 == iek->monsterType[1])
	{
		// add a portal to the dead realm
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 37;
		wp->targetY = 5;
		wp->spaceType = SPACE_REALM;
		wp->spaceSubType = REALM_ID_DEAD;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	// teleport everyone inside
	BBOSMob *curMob = (BBOSMob *)ss->avatars->First();
	while (curMob)
	{
		if (SMOB_AVATAR == curMob->WhatAmI() && curMob->cellX == dm->enterX &&
			curMob->cellY == dm->enterY)
		{
			BBOSAvatar *anAvatar = (BBOSAvatar *)curMob;

			tempReceiptList.clear();
			tempReceiptList.push_back(anAvatar->socketIndex);

			// tell everyone I'm dissappearing 
			anAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
			// and if i have a monster, tell people it's going away too
			if (anAvatar->controlledMonster)
			{
				// we need to dissapear it from people's screen.
				MessMonsterDeath messMD;
				messMD.mobID = (unsigned long)anAvatar->controlledMonster;
				ss->SendToEveryoneNearBut(0, anAvatar->controlledMonster->cellX, anAvatar->controlledMonster->cellY,
					sizeof(messMD), (void *)&messMD); // 
				// remove it fropm old map
				ss->mobList->Remove(anAvatar->controlledMonster);

			}

			// tell my client I'm entering the dungeon
			MessChangeMap changeMap;
			changeMap.dungeonID = (long)dm;
			changeMap.oldType = ss->WhatAmI();
			changeMap.newType = dm->WhatAmI();
			changeMap.sizeX = ((DungeonMap *)dm)->width;
			changeMap.sizeY = ((DungeonMap *)dm)->height;
			changeMap.flags = MESS_CHANGE_TEMP;

			// move me to my new SharedSpace
			ss->avatars->Remove(anAvatar);
			dm->avatars->Append(anAvatar);

			anAvatar->targetCellX = anAvatar->cellX = 0;//((DungeonMap *) dm)->width-1;
			anAvatar->targetCellY = anAvatar->cellY = ((DungeonMap *)dm)->height - 1;
			if (anAvatar->controlledMonster) // if they had a monster
			{
				// then it needs ot have it's coords updated, and be added to new map
				anAvatar->controlledMonster->cellX = anAvatar->controlledMonster->targetCellX = anAvatar->cellX;
				anAvatar->controlledMonster->cellY = anAvatar->controlledMonster->targetCellY = anAvatar->cellY;
				dm->mobList->Add(anAvatar->controlledMonster);
			}
			// NOW send the changemap packet.
			lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

			MessInfoText infoText;
			sprintf_s(tempText, "You enter the %s.", ((DungeonMap *)dm)->name);
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			sprintf_s(tempText, "It has %d monsters. Check remaining with /geocount.", ((DungeonMap *)dm)->MonsterCount);
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

			anAvatar->QuestSpaceChange(ss, dm);


			// tell everyone about my arrival
			anAvatar->IntroduceMyself(dm, SPECIAL_APP_DUNGEON);
			// and introduce the monster if i have one
			if (anAvatar->controlledMonster) // if they had a monster
			{
				// monster announces itself too
				anAvatar->controlledMonster->AnnounceMyselfCustom(dm);
			}
			// tell this player about everyone else around
			anAvatar->UpdateClient(dm, TRUE);

			curMob = (BBOSMob *)ss->avatars->First();
		}
		else
			curMob = (BBOSMob *)ss->avatars->Next();
	}


}
void BBOServer::HandleEarthKeyUse(BBOSAvatar *ca, InvEarthKey *iek, SharedSpace *ss)
{
	char tempText[1024];
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(ca->socketIndex);
	BBOSMonster *monster;
	int rottedearthkey = 0;
	// create the resume item
	// pick name
	int depth = iek->power + 20;

	if (iek->monsterType[1]<0)
		sprintf(tempText, "%dm %s Key Redo",
			depth, monsterData[iek->monsterType[0]][0].name);
	else
		sprintf(tempText, "%dm mixed Key Redo",
			depth);
	// widthXheight monsternameKey
	//		if (meatType[1]<0)
	//			sprintf(tempText, "%dX%d %sKey",
	//				ekwidth,ekheight, monsterData[meatType[0]][0].name);
	//		else
	//			sprintf(tempText, "%dX%d mixed EarthKey",
	//				ekwidth, ekheight);

	InventoryObject *io = new InventoryObject(INVOBJ_EARTHKEY_RESUME, 0, tempText);
	io->mass = 0.0f; // no mass
	io->value = 1; // worthless to sell
	io->amount = 1; //one

	InvEarthKey *exIn = (InvEarthKey *)io->extra; // create extra info
												  // and copy from the earthkey we just used.
	exIn->power = iek->power;
	exIn->monsterType[0] = iek->monsterType[0];
	exIn->monsterType[1] = iek->monsterType[1];
	exIn->width = iek->width;
	exIn->height = iek->height;
	// and silently add it to the inventory.
	ca->charInfoArray[ca->curCharacterIndex].inventory->AddItemSorted(io);


	//	if (iek->monsterType[0] == 27 || iek->monsterType[0] == 28)
	//		iek->monsterType[0] = 0;

	//	if (iek->monsterType[1] == 27 || iek->monsterType[1] == 28)
	//		iek->monsterType[1] = 0;
	if (iek->monsterType[0] == 27) // no need to remove lizardmen now, no dusts.
		iek->monsterType[0] = 0;

	if (iek->monsterType[1] == 27)
		iek->monsterType[1] = 0;
	if (iek->width<0)
	{
		rottedearthkey = 1;
		iek->width *= -1;
	}
	if (iek->height<0)
	{
		rottedearthkey = 1;
		iek->height *= -1;
	}
	// make the new space
	DungeonMap *dm = new DungeonMap(SPACE_DUNGEON, "Test Dungeon", lserver);
	dm->specialFlags = SPECIAL_DUNGEON_TEMPORARY;
	dm->InitNew(iek->width, iek->height, ca->cellX, ca->cellY, 0);
	sprintf_s(dm->name, "%s's %s %s",
		ca->charInfoArray[ca->curCharacterIndex].name,
		rdNameAdjective[rand() % 5], rdNamePre[rand() % 7]);
	dm->maxdestroyed = (dm->NumberOfWalls / 6) ;  // bigger geos allow you to blast more wallls before they break.
	dm->tempPower = iek->power;
	dm->GeoDepth = depth;                      // store depth so bombs can check it.

	spaceList->Append(dm);
	int col = 0;

	float change = 1 + iek->power / 20;
	if (rottedearthkey == 1)
		change += 0.25; // 5 meter boost for rotted monsters.
	float size1 = 1 + iek->power / 50;
	float size2 = 1 + iek->power / 80;
	float resist = Bracket(iek->power / 100.0f, 0.0f, 100000.0f);

	int t = iek->monsterType[0];
	int maxmonsters = (dm->width*dm->height) / 6 + rand() % 5; //randomize so count doesn't reveal if there is a vagabond or not. :)
	int maxmonsters2 = (dm->width*dm->height) / 6 + rand() % 5; //randomize so count doesn't reveal if there is a vagabond or not. :)
																// add static monsters
	for (int m = 0; m < maxmonsters;) // first spawn sweep
	{
		int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

		if (monsterData[t][t2].name[0])
		{
			int mx, my;
			do
			{
				mx = rand() % (dm->width);
				my = rand() % (dm->height);
			} while (mx < 1);

			monster = new BBOSMonster(t, t2, NULL);
			monster->cellX = mx;
			monster->cellY = my;
			monster->targetCellX = mx;
			monster->targetCellY = my;
			monster->spawnX = mx;
			monster->spawnY = my;
			if (rottedearthkey == 1)
			{
				sprintf_s(monster->uniqueName, "%s %s", "Rotted", monster->Name());
				monster->r = 255;
				monster->g = 0;
				monster->b = 0;
			}
			monster->damageDone = monster->damageDone * change;
			monster->defense = monster->defense    * change;
			monster->dropAmount = monster->dropAmount * change;
			monster->health = monster->health     * change;
			monster->maxHealth = monster->maxHealth  * change;
			monster->toHit = monster->toHit      * change;
			monster->dontRespawn = true;
			if (depth > 22)
			{
				monster->tamingcounter = 1000; // no taming pre boosted monsters!
			}
			if (monsterData[t][t2].placementFlags!=MONSTER_PLACE_DUNGEON)
			{
				monster->tamingcounter = 1000; // no taming monsters you can find outside of a dungeon!
			}

			monster->magicResistance = resist;
			if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
				monster->magicResistance = 0.99f;
			dm->mobList->Add(monster);
			++m;
			++dm->MonsterCount;
		}
	}

	if (iek->monsterType[1] > -1)
		t = iek->monsterType[1];

	for (int m = 0; m < maxmonsters2;) // second spawn sweep. same monster unless it's  double meat geto
	{
		int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

		if (monsterData[t][t2].name[0])
		{
			int mx, my;
			do
			{
				mx = rand() % (dm->width);
				my = rand() % (dm->height);
			} while (mx < 1);

			monster = new BBOSMonster(t, t2, NULL);
			monster->cellX = mx;
			monster->cellY = my;
			monster->targetCellX = mx;
			monster->targetCellY = my;
			monster->spawnX = mx;
			monster->spawnY = my;
			monster->dontRespawn = true;

			monster->damageDone = monster->damageDone * change;
			monster->defense = monster->defense    * change;
			monster->dropAmount = monster->dropAmount * change;
			monster->health = monster->health     * change;
			monster->maxHealth = monster->maxHealth  * change;
			monster->toHit = monster->toHit      * change;
			if (depth > 22)
			{
				monster->tamingcounter = 1000; // no taming pre boosted monsters!
			}
			if (monsterData[t][t2].placementFlags != MONSTER_PLACE_DUNGEON)
			{
				monster->tamingcounter = 1000; // no taming monsters you can find outside of a dungeon!
			}

			monster->magicResistance = resist;
			if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
				monster->magicResistance = 0.99f;
			if (rottedearthkey == 1)
			{
				sprintf_s(monster->uniqueName, "%s %s", "Rotted", monster->Name());
				monster->r = 255;
				monster->g = 0;
				monster->b = 0;
			}

			dm->mobList->Add(monster);
			++m;
			++dm->MonsterCount;
		}
	}

	// add boss monster
	int i = 0;
	for (i = NUM_OF_MONSTER_SUBTYPES - 1;
		i >= 0 && 0 == monsterData[t][i].name[0];
		--i)
		;

	monster = new BBOSMonster(t, i, NULL);
	if (rottedearthkey == 1)
		sprintf_s(monster->uniqueName, "Rotted Sentinel");
	else
		sprintf_s(monster->uniqueName, "Sentinel");
	monster->sizeCoeff = size1;

	monster->damageDone = monster->damageDone * change * 2;
	monster->defense = monster->defense    * change * 2;
	monster->dropAmount = monster->dropAmount * change * 2;
	monster->health = monster->health     * change * 2;
	monster->maxHealth = monster->maxHealth  * change * 2;
	monster->toHit = monster->toHit      * change * 2;

	monster->magicResistance = Bracket(resist, 0.5f, 1000000.0f);
	if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
		monster->magicResistance = 0.99f;
	monster->r = 0;
	monster->g = 0;
	monster->b = 255;

	monster->targetCellX = monster->spawnX = monster->cellX = dm->width - 3;
	monster->targetCellY = monster->spawnY = monster->cellY = 0;
	monster->dontRespawn = true;
	monster->tamingcounter = 1000; // no taming pre boosted monsters!

	dm->mobList->Add(monster);
	++dm->MonsterCount;

	// sometimes add a Vagabond monster
	if (!(rand() % 3))
	{
		int vagIndex = t + 1;

		if (21 == vagIndex || vagIndex >= NUM_OF_MONSTERS)
			vagIndex = 0;

		for (i = NUM_OF_MONSTER_SUBTYPES - 1;
			i >= 0 && 0 == monsterData[vagIndex][i].name[0];
			--i)
			;

		monster = new BBOSMonster(vagIndex, i, NULL, true);
		if (rottedearthkey == 1)
			sprintf_s(monster->uniqueName, "Rotted Vagabond");
		else
			sprintf_s(monster->uniqueName, "Vagabond");
		monster->sizeCoeff = size2;

		monster->damageDone = monster->damageDone * change * 1.5f;
		monster->defense = monster->defense    * change * 1.5f;
		monster->dropAmount = monster->dropAmount * change * 1.5f;
		monster->health = monster->health     * change * 1.5f;
		monster->maxHealth = monster->maxHealth  * change * 1.5f;
		monster->toHit = monster->toHit      * change * 1.5f;

		monster->magicResistance = Bracket(resist, 0.8f, 100000000.0f);
		if (monster->type > 8 && monster->type < 18 && monster->magicResistance>0.99f)
			monster->magicResistance = 0.99f;

		monster->r = 0;
		monster->g = 255;
		monster->b = 255;
		monster->tamingcounter = 1000; // no taming pre boosted monsters!

		if (rand() % 2)
		{
			monster->targetCellX = monster->spawnX = monster->cellX = dm->width * 3 / 4;
			monster->targetCellY = monster->spawnY = monster->cellY = dm->height * 3 / 4;
		}
		else
		{
			monster->targetCellX = monster->spawnX = monster->cellX = dm->width / 4;
			monster->targetCellY = monster->spawnY = monster->cellY = dm->height / 4;
		}
		monster->dontRespawn = true;
		dm->mobList->Add(monster);
		++dm->MonsterCount;

		monster->AddPossessedLoot(1 + iek->power / 10);

		BBOSGroundEffect *bboGE = new BBOSGroundEffect();

		bboGE->type = 4;
		bboGE->amount = 3;
		bboGE->r = 255;
		bboGE->g = 255;
		bboGE->b = 0;
		bboGE->cellX = monster->cellX;
		bboGE->cellY = monster->cellY;

		dm->mobList->Add(bboGE);

	}

	// add a portal out of it
	BBOSWarpPoint *wp = new BBOSWarpPoint(dm->width - 1, 0);
	wp->targetX = dm->enterX;
	wp->targetY = dm->enterY;
	wp->spaceType = SPACE_GROUND;
	wp->spaceSubType = 0;
	dm->mobList->Add(wp);

	BBOSChest *chest = new BBOSChest(dm->width - 2, 0);
	dm->mobList->Add(chest);

	if ((28 == iek->monsterType[0] && 29 == iek->monsterType[1]) || (29 == iek->monsterType[0] && 28 == iek->monsterType[1]))
	{
		// add a portal to the basement
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 33;
		wp->targetY = 33;
		wp->spaceType = SPACE_LABYRINTH;
		wp->spaceSubType = REALM_ID_LAB3;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if ((24 == iek->monsterType[0] && 28 == iek->monsterType[1])|| (28 == iek->monsterType[0] && 24 == iek->monsterType[1]))
	{
		// add a portal to the Deep Laby
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 27;
		wp->targetY = 25;
		wp->spaceType = SPACE_LABYRINTH;
		wp->spaceSubType = REALM_ID_LAB2;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if ((8 == iek->monsterType[0] && 24 == iek->monsterType[1])|| (24 == iek->monsterType[0] && 8 == iek->monsterType[1]))
	{
		// add a portal to the Laby
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 25;
		wp->targetY = 25;
		wp->spaceType = SPACE_LABYRINTH;
		wp->spaceSubType = REALM_ID_LAB1;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if ((9 == iek->monsterType[0] && 22 == iek->monsterType[1])|| (22 == iek->monsterType[0] && 9 == iek->monsterType[1]))
	{
		// add a portal to the Dragon realm
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 57;
		wp->targetY = 6;
		wp->spaceType = SPACE_REALM;
		wp->spaceSubType = REALM_ID_DRAGONS;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if ((5 == iek->monsterType[0] && 12 == iek->monsterType[1])|| (12 == iek->monsterType[0] && 5 == iek->monsterType[1]))
	{
		// add a portal to the spirit realm
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 61;
		wp->targetY = 8;
		wp->spaceType = SPACE_REALM;
		wp->spaceSubType = REALM_ID_SPIRITS;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	if ((7 == iek->monsterType[0] && 19 == iek->monsterType[1])|| (19 == iek->monsterType[0] && 7 == iek->monsterType[1]))
	{
		// add a portal to the dead realm
		wp = new BBOSWarpPoint(0, 0);
		wp->targetX = 37;
		wp->targetY = 5;
		wp->spaceType = SPACE_REALM;
		wp->spaceSubType = REALM_ID_DEAD;
		wp->allCanUse = TRUE;
		dm->mobList->Add(wp);
	}

	// teleport everyone inside
	BBOSMob *curMob = (BBOSMob *)ss->avatars->First();
	while (curMob)
	{
		if (SMOB_AVATAR == curMob->WhatAmI() && curMob->cellX == dm->enterX &&
			curMob->cellY == dm->enterY)
		{
			BBOSAvatar *anAvatar = (BBOSAvatar *)curMob;

			tempReceiptList.clear();
			tempReceiptList.push_back(anAvatar->socketIndex);

			// tell everyone I'm dissappearing 
			anAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
			// and if i have a monster, tell people it's going away too
			if (anAvatar->controlledMonster)
			{
				// we need to dissapear it from people's screen.
				MessMonsterDeath messMD;
				messMD.mobID = (unsigned long)anAvatar->controlledMonster;
				ss->SendToEveryoneNearBut(0, anAvatar->controlledMonster->cellX, anAvatar->controlledMonster->cellY,
					sizeof(messMD), (void *)&messMD); // 
				// remove it fropm old map
				ss->mobList->Remove(anAvatar->controlledMonster);

			}

			// tell my client I'm entering the dungeon
			MessChangeMap changeMap;
			changeMap.dungeonID = (long)dm;
			changeMap.oldType = ss->WhatAmI();
			changeMap.newType = dm->WhatAmI();
			changeMap.sizeX = ((DungeonMap *)dm)->width;
			changeMap.sizeY = ((DungeonMap *)dm)->height;
			changeMap.flags = MESS_CHANGE_TEMP;

			// move me to my new SharedSpace
			ss->avatars->Remove(anAvatar);
			dm->avatars->Append(anAvatar);

			anAvatar->targetCellX = anAvatar->cellX = 0;//((DungeonMap *) dm)->width-1;
			anAvatar->targetCellY = anAvatar->cellY = ((DungeonMap *)dm)->height - 1;
			if (anAvatar->controlledMonster) // if they had a monster
			{
				// then it needs ot have it's coords updated, and be added to new map
				anAvatar->controlledMonster->cellX = anAvatar->controlledMonster->targetCellX = anAvatar->cellX;
				anAvatar->controlledMonster->cellY = anAvatar->controlledMonster->targetCellY = anAvatar->cellY;
				dm->mobList->Add(anAvatar->controlledMonster);
			}
            // NOW send the changemap packet.
			lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

			MessInfoText infoText;
			sprintf_s(tempText, "You enter the %s.", ((DungeonMap *)dm)->name);
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			sprintf_s(tempText, "It has %d monsters. Check remaining with /geocount.", ((DungeonMap *)dm)->MonsterCount);
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

			anAvatar->QuestSpaceChange(ss, dm);


			// tell everyone about my arrival
			anAvatar->IntroduceMyself(dm, SPECIAL_APP_DUNGEON);
			// and introduce the monster if i have one
			if (anAvatar->controlledMonster) // if they had a monster
			{
				// monster announces itself too
				anAvatar->controlledMonster->AnnounceMyselfCustom(dm);
			}
			// tell this player about everyone else around
			anAvatar->UpdateClient(dm, TRUE);

			curMob = (BBOSMob *)ss->avatars->First();
		}
		else
			curMob = (BBOSMob *)ss->avatars->Next();
	}


}

//******************************************************************
void BBOServer::HandleDoomKeyUse(BBOSAvatar *ca, InvDoomKey *iek, SharedSpace *ss)
{
	char tempText[1024];
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(ca->socketIndex);
	BBOSMonster *monster;
	//randomize the monster types
	iek->monsterType[0] = rand() % 31; //randomly pick a monster type
	iek->monsterType[1] = rand() % 31; //randomly pick a monster type
	iek->monsterType[2] = rand() % 31; //randomly pick a monster type
	iek->monsterType[3] = rand() % 31; //randomly pick a monster type
	iek->monsterType[4] = rand() % 31; //randomly pick a monster type
	iek->monsterType[5] = rand() % 31; //randomly pick a monster type
	iek->monsterType[6] = rand() % 31; //randomly pick a monster type
    // replace thieving spirits

	if (iek->monsterType[0] == 21) // they will be lizardmen instead!
		iek->monsterType[0] = 28;
	if (iek->monsterType[1] == 21) // or vampire lords!
		iek->monsterType[1] = 27;
	if (iek->monsterType[2] == 21) // or Dragons!
		iek->monsterType[2] = 7;
	if (iek->monsterType[3] == 21) // or bone imperators!
		iek->monsterType[3] = 19;
	if (iek->monsterType[4] == 21) // or Anubis?
		iek->monsterType[4] = 20;
	if (iek->monsterType[5] == 21) //or bats!
		iek->monsterType[5] = 26;
	if (iek->monsterType[6] == 21) // or vampires
		iek->monsterType[6] = 25;
	// find a suitable sentinel among the harder monsters.
	int sentineltype = rand() % 5;
	if (sentineltype == 0)
		sentineltype = 28;
	if (sentineltype == 1)
		sentineltype = 19;
	if (sentineltype == 2)
		sentineltype = 20;
	if (sentineltype == 3)
		sentineltype = 18;
	if (sentineltype == 4)
		sentineltype = 25;


	// make the new space
	DungeonMap *dm = new DungeonMap(SPACE_DUNGEON, "Test Dungeon", lserver);
	dm->specialFlags = SPECIAL_DUNGEON_DOOMTOWER;
	dm->InitNew(iek->width, iek->height, ca->cellX, ca->cellY, 0);
	sprintf_s(dm->name, "%s's %s %s",
		ca->charInfoArray[ca->curCharacterIndex].name,
		rdNameAdjective[rand() % 5], rdNamePre[rand() % 7]);

	dm->tempPower = iek->power;

	spaceList->Append(dm);
	int col = 0;

	float change = 1 + iek->power / 20;
	float size1 = 1 + iek->power / 50;
	float size2 = 1 + iek->power / 80;
	float resist = Bracket(iek->power / 100.0f, 0.0f, 100000000.0f);
	for (int sweep = 0; sweep < 7;)
	{
		int t = iek->monsterType[sweep];
		int maxmonsters = (dm->width*dm->height) / 21 + 1;

		for (int m = 0; m < maxmonsters;) // spawn sweep
		{
			int t2 = rand() % NUM_OF_MONSTER_SUBTYPES;

			if (monsterData[t][t2].name[0])
			{
				int mx, my;
				do
				{
					mx = rand() % (dm->width);
					my = rand() % (dm->height);
				} while (mx < 1);

				monster = new BBOSMonster(t, t2, NULL);
				monster->cellX = mx;
				monster->cellY = my;
				monster->targetCellX = mx;
				monster->targetCellY = my;
				monster->spawnX = mx;
				monster->spawnY = my;

				monster->damageDone = monster->damageDone * change;
				monster->defense = monster->defense    * change;
				monster->dropAmount = monster->dropAmount * change;
				monster->health = monster->health     * change;
				monster->maxHealth = monster->maxHealth  * change;
				monster->toHit = monster->toHit      * change;
				monster->tamingcounter = 1000; // no taming pre boosted monsters!

				monster->magicResistance = resist;
				monster->dontRespawn = true;
				dm->mobList->Add(monster);
				++m;
				++dm->MonsterCount;
			}
		}
		sweep++;
	}

	// add boss monster
	int i = 0;
	for (i = NUM_OF_MONSTER_SUBTYPES - 1;
		i >= 0 && 0 == monsterData[sentineltype][i].name[0];
		--i)
		;

	monster = new BBOSMonster(sentineltype, i, NULL);
	sprintf_s(monster->uniqueName, "Sentinel");
	monster->sizeCoeff = size1;

	monster->damageDone = monster->damageDone * change * 2;
	monster->defense = monster->defense    * change * 2;
	monster->dropAmount = monster->dropAmount * change * 2;
	monster->health = monster->health     * change * 2;
	monster->maxHealth = monster->maxHealth  * change * 2;
	monster->toHit = monster->toHit      * change * 2;

	monster->magicResistance = Bracket(resist, 0.5f, 100000000.0f);
	monster->r = 0;
	monster->g = 0;
	monster->b = 255;

	monster->targetCellX = monster->spawnX = monster->cellX = dm->width - 3;
	monster->targetCellY = monster->spawnY = monster->cellY = 0;
	monster->dontRespawn = true;
	dm->mobList->Add(monster);
	++dm->MonsterCount;
	monster->AddPossessedLoot(1 + iek->power / 10); // and give it vagabond loot
	monster->tamingcounter = 1000; // no taming pre boosted monsters!


	// add a portal out of it
	BBOSWarpPoint *wp = new BBOSWarpPoint(dm->width - 1, 0);
	wp->targetX = dm->enterX;
	wp->targetY = dm->enterY;
	wp->spaceType = SPACE_GROUND;
	wp->spaceSubType = 0;
	dm->mobList->Add(wp);

	BBOSChest *chest = new BBOSChest(dm->width - 2, 0);
	dm->mobList->Add(chest);

	// teleport everyone inside
	BBOSMob *curMob = (BBOSMob *)ss->avatars->First();
	while (curMob)
	{
		if (SMOB_AVATAR == curMob->WhatAmI() && curMob->cellX == dm->enterX &&
			curMob->cellY == dm->enterY)
		{
			BBOSAvatar *anAvatar = (BBOSAvatar *)curMob;

			tempReceiptList.clear();
			tempReceiptList.push_back(anAvatar->socketIndex);

			// tell everyone I'm dissappearing
			anAvatar->AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
			// and if i have a monster, tell people it's going away too
			if (anAvatar->controlledMonster)
			{
				// we need to dissapear it from people's screen.
				MessMonsterDeath messMD;
				messMD.mobID = (unsigned long)anAvatar->controlledMonster;
				ss->SendToEveryoneNearBut(0, anAvatar->controlledMonster->cellX, anAvatar->controlledMonster->cellY,
					sizeof(messMD), (void *)&messMD); // 
				// remove it fropm old map
				ss->mobList->Remove(anAvatar->controlledMonster);

			}

			// tell my client I'm entering the dungeon
			MessChangeMap changeMap;
			changeMap.dungeonID = (long)dm;

			changeMap.oldType = ss->WhatAmI();
			changeMap.newType = dm->WhatAmI();
			changeMap.sizeX = ((DungeonMap *)dm)->width;
			changeMap.sizeY = ((DungeonMap *)dm)->height;
			changeMap.flags = MESS_CHANGE_TEMP;

			anAvatar->QuestSpaceChange(ss, dm);

			// move me to my new SharedSpace
			ss->avatars->Remove(anAvatar);
			dm->avatars->Append(anAvatar);

			anAvatar->targetCellX = anAvatar->cellX = 0;//((DungeonMap *) dm)->width-1;
			anAvatar->targetCellY = anAvatar->cellY = ((DungeonMap *)dm)->height - 1;
			if (anAvatar->controlledMonster) // if they had a monster
			{
				// then it needs ot have it's coords updated, and be added to new map
				anAvatar->controlledMonster->cellX = anAvatar->controlledMonster->targetCellX = anAvatar->cellX;
				anAvatar->controlledMonster->cellY = anAvatar->controlledMonster->targetCellY = anAvatar->cellY;
				dm->mobList->Add(anAvatar->controlledMonster);
			}

			// NOW send the map packet
			lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

			MessInfoText infoText;
			sprintf_s(tempText, "You enter level %d.", (int)iek->power);
			memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
			infoText.text[MESSINFOTEXTLEN - 1] = 0;
			lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);


			// tell everyone about my arrival
			anAvatar->IntroduceMyself(dm, SPECIAL_APP_DUNGEON);

			if (anAvatar->controlledMonster) // if they had a monster
			{
				// monster announces itself too
				anAvatar->controlledMonster->AnnounceMyselfCustom(dm);
			}

			// tell this player about everyone else around
			anAvatar->UpdateClient(dm, TRUE);

			curMob = (BBOSMob *)ss->avatars->First();
		}
		else
			curMob = (BBOSMob *)ss->avatars->Next();
	}


}



/* end of file */
