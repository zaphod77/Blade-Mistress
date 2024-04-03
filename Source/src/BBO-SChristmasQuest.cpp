#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "BBO-SChristmasQuest.h"
#include "BBO.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "MonsterData.h"

enum
{
	CHRISTMASQUEST_READY,
	CHRISTMASQUEST_ON,
	CHRISTMASQUEST_RECOVERING,
	CHRISTMASQUEST_MAX
};

//******************************************************************
//******************************************************************
BBOSChristmasQuest::BBOSChristmasQuest(SharedSpace *s) : BBOSAutoQuest(s)
{
	questState = CHRISTMASQUEST_RECOVERING;  // this quest starts off
}

//******************************************************************
BBOSChristmasQuest::~BBOSChristmasQuest()
{

}

//******************************************************************
void BBOSChristmasQuest::Tick(SharedSpace *unused)
{
	char tempText[1024];

	DWORD delta;
	DWORD now = timeGetTime();

	// process at intervals
	delta = now - lastSpawnTime;
	if (delta < 1000 * 10)  // 10 seconds	
		return;

	++questCounter;

	lastSpawnTime = now;

	if (CHRISTMASQUEST_READY == questState) // if it was turned onby comand
	{
				questState = CHRISTMASQUEST_ON;
				questCounter = 0;
				monsterType = 32; // fix later when we actually add the reindeer
				monsterPower = 2; // good start

				sprintf(&(tempText[2]),"Christmas cheer fills the air...");
				tempText[0] = NWMESS_PLAYER_CHAT_LINE;
				tempText[1] = TEXT_COLOR_DATA;
	  			bboServer->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);

				sprintf(&(tempText[2]),"The Reindeer are coming to visit.");
				tempText[0] = NWMESS_PLAYER_CHAT_LINE;
				tempText[1] = TEXT_COLOR_DATA;
	  			bboServer->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);
	}
	else if (CHRISTMASQUEST_RECOVERING == questState)
	{  // do nothing, it does not start on it's own
	}
	else if (CHRISTMASQUEST_ON == questState)
	{
		if ((count[monsterType][0] <= 0) && (count[monsterType][1] <= 0))
		{
			// no time limit
			// instead we count killed monsters.
			if (totalMembersBorn <9) // 8 reindeer plus Rudolph. ;)
			{
			//	monsterPower *= rnd(1.1f, 1.2f); //no increase in power for these.
				CreateMonster();
			}
			else
			{
				sprintf(&(tempText[2]),"The Reindeer have left their gifts behind.");
				tempText[0] = NWMESS_PLAYER_CHAT_LINE;
				tempText[1] = TEXT_COLOR_DATA;
	  			bboServer->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);
				questState = CHRISTMASQUEST_RECOVERING;
			}
		}
	}
}



//******************************************************************
void BBOSChristmasQuest::CreateMonster(void)
{
	char tempText[1024];
	char tempText2[1024];
	int mx, my, good;
	GroundMap *gm = (GroundMap *) ss;  // ASSUMPTION!!!

	// find the spot
	do
	{
		mx = rand() % 256;
		my = rand() % 256;

		good = TRUE;

		int color = gm->Color(mx,my);
		if (color > 5 || color < 1)
			good = FALSE;

		if (! ss->CanMove(mx,my,mx,my))
			good = FALSE;

		BBOSMob *tMob = NULL;
		tMob = ss->mobList->GetFirst(mx, my);
		while (tMob)
		{
			if (SMOB_TOWER == tMob->WhatAmI())
				good = FALSE;
			tMob = ss->mobList->GetNext();
		}

	} while (!good);

	// create the monster on the spot
	BBOSMonster *monster;
    if (totalMembersBorn<8)
		monster = new BBOSMonster(monsterType, 0, this);
	else
	    monster = new BBOSMonster(monsterType, 1, this);
	monster->cellX = monster->targetCellX = monster->spawnX = mx;
	monster->cellY = monster->targetCellY = monster->spawnY = my;
	ss->mobList->Add(monster);
	switch (totalMembersBorn) 
	{
		case 0:
			sprintf(monster->uniqueName, "Dasher");
			break;
		case 1:
			sprintf(monster->uniqueName, "Dancer");
			break;
		case 2:
			sprintf(monster->uniqueName, "Prancer");
			break;
		case 3:
			sprintf(monster->uniqueName, "Vixen");
			break;
		case 4:
			sprintf(monster->uniqueName, "Comet");
			break;
		case 5:
			sprintf(monster->uniqueName, "Cupid");
			break;
		case 6:
			sprintf(monster->uniqueName, "Donner");
			break;
		case 7:
			sprintf(monster->uniqueName, "Blitzen");
			break;
		default:
			break;
	}
	//	these don't need adjustment they are supposed to be easy.
	monster->a               = 255;
//	monster->sizeCoeff       = 1.5f + 1.0f * monsterPower /800;
//	monster->health          = 40 * monsterPower;
//	monster->maxHealth       = 40 * monsterPower;
//	monster->damageDone      = monsterPower/20;
	if (monster->damageDone > 50)
		monster->damageDone = 50;

//	monster->toHit           = monsterPower/14;
//	monster->defense         = monsterPower/14;
//	monster->dropAmount      = monsterPower/100;
//	monster->magicResistance = monsterPower/20/100;
	if (monster->magicResistance > 0.99)
		monster->magicResistance = 0.99;

//	monster->healAmountPerSecond = (int)monsterPower/3;
	monster->tamingcounter = 1000;					// this should be impossible anyway due to their incredibly low HP, but....
	// announce it
	MessMobAppearCustom mAppear;
	mAppear.type = SMOB_MONSTER;
	mAppear.mobID = (unsigned long) monster;
	mAppear.x = monster->cellX;
	mAppear.y = monster->cellY;
	mAppear.monsterType = monster->type;
	mAppear.subType = monster->subType;
	CopyStringSafely(monster->Name(), 32, mAppear.name, 32);
	mAppear.a = monster->a;
	mAppear.r = monster->r;
	mAppear.g = monster->g;
	mAppear.b = monster->b;
	mAppear.sizeCoeff = monster->sizeCoeff;
	ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
				 sizeof(mAppear), &mAppear);

	sprintf(&(tempText[2]), "A Reindeer prances down at %dN %dE!",
		256 - my, 256 - mx);
	sprintf(tempText2, "A Reindeer prances down at %dN %dE!",
		256 - my, 256 - mx);
	FILE *source = fopen("stormlocations.txt", "a");
	fprintf(source, "%s\n", tempText2);
	fclose(source);
	tempText[0] = NWMESS_PLAYER_CHAT_LINE;
	tempText[1] = TEXT_COLOR_DATA;
	bboServer->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, NULL);
	/* Display operating system-style date and time. */


	MessGenericEffect messGE;
	messGE.avatarID = -1;
	messGE.mobID    = 0;
	messGE.x        = mx;
	messGE.y        = my;
	messGE.r        = 50;
	messGE.g        = 255;
	messGE.b        = 50;
	messGE.type     = 0;  // type of particles
	messGE.timeLen  = 2; // in seconds
	ss->SendToEveryoneNearBut(0, mx, my,
			          sizeof(messGE),(void *)&messGE);

	// track it
	++totalMembersBorn;
	questCounter = 0;
}

//******************************************************************
void BBOSChristmasQuest::MonsterEvent(BBOSMonster *theMonster, int eventType, int x, int y)
{
	if (theMonster->controllingAvatar) // forget about controlled avatars.
		return;

	if (AUTO_EVENT_ATTACKED == eventType)
	{
		//
	}
	else if (AUTO_EVENT_DIED == eventType)
	{
		// drop a special totem.
			ss->DoMonsterDropSpecial(theMonster,31); // best totem in the game
	}
}


/* end of file */


