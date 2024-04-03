
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "BBO-Savatar.h"
#include "BBO-Sgen-orcClan.h"
#include "BBO.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "MonsterData.h"
#include "Realm-Map.h"


//******************************************************************
BBOSGenOrcClan::BBOSGenOrcClan(int xa, int ya) : BBOSGenerator(xa,ya)
{

	for (int i = 0; i < NUM_OF_MONSTERS; ++i)
	{
		for (int j = 0; j < NUM_OF_MONSTER_SUBTYPES; ++j)
		{
		   max[i][j]   = 0;
			if (22 == i)
			   max[i][j]   = (5-j) * (5-j) * 2;

//			if (11 == i && 1 == j) // dokk's centurion
//			   max[i][j]   = 3;
//			if (16 == i && 1 == j) // dokk
//			   max[i][j]   = 1;


		}
	}
}

//******************************************************************
BBOSGenOrcClan::~BBOSGenOrcClan()
{
}

//******************************************************************
void BBOSGenOrcClan::Tick(SharedSpace *ss)
{
	BBOSMob *curMob = NULL;
	BBOSAvatar *curAvatar = NULL;

	DWORD delta;
	DWORD now = timeGetTime();

	delta = now - lastSpawnTime;

	if (0 == lastSpawnTime || now < lastSpawnTime)
	{
		lastSpawnTime = now - rand() % 300;
	}

	// spawn
	if (delta > 300)	
	{
		lastSpawnTime = now;

		for (int i = 0; i < NUM_OF_MONSTERS; ++i)
		{
			for (int j = 0; j < NUM_OF_MONSTER_SUBTYPES; ++j)
			{
				if (count[i][j] < max[i][j] && monsterData[i][j].name[0])
				{
					if ((MONSTER_PLACE_DRAGONS & monsterData[i][j].placementFlags) &&
						 SPACE_REALM == ss->WhatAmI() &&
						 REALM_ID_DRAGONS == ((RealmMap *)ss)->type)
					{																 
						int mx, my, good;
						RealmMap *rm = (RealmMap *) ss;  // ASSUMPTION!!!

						if (4 == j)
						{
							mx = x;
							my = y;
						}
						else
						{
							do
							{
								mx = x + (rand() % ((4-j)*2+1)) - (4-j);
								my = y + (rand() % ((4-j)*2+1)) - (4-j);
								good = TRUE;

								if (!rm->CanMove(mx,my,mx,my))
									good = FALSE;

								if (abs(mx-x) != 4-j && abs(my-y) != 4-j)
									good = FALSE;

							} while (!good);
						}
						
						BBOSMonster *monster = new BBOSMonster(i,j, this);
						monster->cellX = mx;
						monster->cellY = my;
						monster->targetCellX = mx;
						monster->targetCellY = my;
						monster->spawnX = mx;
						monster->spawnY = my;
						ss->mobList->Add(monster);

						MessMobAppear mobAppear;
						mobAppear.mobID = (unsigned long) monster;
						mobAppear.type = monster->WhatAmI();
						mobAppear.monsterType = monster->type;
						mobAppear.subType = monster->subType;
						if(SPACE_DUNGEON == ss->WhatAmI())
						{
							mobAppear.staticMonsterFlag = FALSE;
							if (!monster->isWandering)
								mobAppear.staticMonsterFlag = TRUE;
						}

						mobAppear.x = monster->cellX;
						mobAppear.y = monster->cellY;
						ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
									 sizeof(mobAppear), &mobAppear);
						// if we are an Orc Champion, we may have insta-respawned, so we should scan the square for avatars, and attack if we find one
						if (monster->subType == 4) // orc Champion just spawned
						{
							// get the first Avatar
							BBOSAvatar * curAvatar = (BBOSAvatar *) ss->avatars->First();
							bool found = FALSE;
							while (curAvatar && (!found))
							{
								if ((curAvatar->cellX == monster->cellX) && (curAvatar->cellX == monster->cellX)) // if in my cell
								{
									// if it's attackable
									if (!curAvatar->isInvisible)      // if not invisible
									{
										monster->curTarget = curAvatar;		// target the first non invisible avatar I see
										found = TRUE;						// stop looking
																			// if it's a beastmaster, we will retarget when we get ticked.
									}
								}
								curAvatar = (BBOSAvatar *)ss->avatars->Next();  // go to the next Avatar in the shared space regardless. we will stop if we found one.
							}
						}
						return;

					}
				}
			}
		}


	}
	

}



/* end of file */



