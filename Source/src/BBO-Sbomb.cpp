
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "BBO-Savatar.h"
#include "BBO-Sbomb.h"
#include "BBO-Sarmy.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "dungeon-map.h"
#include "MonsterData.h"


//******************************************************************
BBOSBomb::BBOSBomb(BBOSAvatar *d) : 
                        BBOSMob(SMOB_BOMB,"BOMB")
{
	dropper = d;

	power = 1;
	type = 0;
	flags = 0;
	r = g = b = 255;
	detonateTime = timeGetTime();
}

//******************************************************************
BBOSBomb::~BBOSBomb()
{
}

//******************************************************************
void BBOSBomb::Tick(SharedSpace *ss)
{
	std::vector<TagID> tempReceiptList;
	char tempText[1024];
	MessInfoText infoText;

//	DWORD delta;
	DWORD now = timeGetTime();
	if (now > detonateTime)
	{
		int lootAllowed = TRUE;

		// damage every monster in range and the avatar who dropped me
		if (dropper)
		{
			bool CanHitOtherPlayers; // will set to same as dropper's PVP status;
			BBOSMob *curMob = (BBOSAvatar *) ss->avatars->Find(dropper); //make sure it's really there.
			if (curMob && (curMob->WhatAmI()==SMOB_AVATAR))                // make sure it's actually an avatar, so we can safely cast a pointer to it later.
			{
				BBOSAvatar *curAvatar = (BBOSAvatar *) curMob; // safe, because we know it's really there.
				CanHitOtherPlayers = curAvatar->PVPEnabled;    // if pvp is enabled, bomb can hit other players too.

				long damValue = power * 1.0f / 
						  (1 + Distance(cellX, cellY, curMob->cellX, curMob->cellY)); 
				if (CanHitOtherPlayers)    // if you are pvping
				{
					// then you CAN outrun your bomb after all!
					float dist = 1 + Distance(cellX, cellY, curMob->cellX, curMob->cellY);
					if (dist >= 6.0f)
					{
						damValue = 0; // we escaped the blast radius! Hooray.
					}
				}

				// reduce damage if you have explosive mastery
				// find mastery skill
				
				InventoryObject *io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.First();
				bool found = FALSE;
				char tmp[80];
				char tempText[80];
				MessInfoText infoText;
				InvSkill *iSkill=NULL;
				sprintf(tmp, "Explosives Mastery");
				while (io && !found) {
					if (!strcmp(tmp, io->WhoAmI())) {
						iSkill = (InvSkill *)io->extra;
						found = 1;
						damValue = damValue * (100 - iSkill->skillLevel) / 100; // level 100 is fully immune, for now. Won't help against the evil bomber boss
					}


					io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.Next();
				}
				if (damValue > 0)
				{
					if (iSkill && (damValue < curAvatar->charInfoArray[curAvatar->curCharacterIndex].health)) // if we found Explosive Mastery before, and damage won't kill
					{
						// then add to explosives mastery.  as mastery levels, the same bobm will give less exp.
						iSkill->skillPoints += damValue* bboServer->mastery_exp_multiplier;
						// and check for levelup 
						if (iSkill->skillLevel < 100 && iSkill->skillLevel * 100000 <= iSkill->skillPoints) {
							std::vector<TagID> tempReceiptList;

							tempReceiptList.clear();
							tempReceiptList.push_back(curAvatar->socketIndex);

							// made a skill level!!!
							iSkill->skillLevel += 1;

							sprintf(tempText, "You gained %s skill!! You feel harder to blow up.", tmp);
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}

					}
					if (!curAvatar->accountType)
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= damValue;

					MessAvatarHealth messHealth;
					messHealth.avatarID  = curAvatar->socketIndex;
					messHealth.health    = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
					messHealth.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
					ss->SendToEveryoneNearBut(0, curMob->cellX, curMob->cellY, sizeof(messHealth), &messHealth);

					if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health <= 0)
					{
						curAvatar->charInfoArray[curAvatar->curCharacterIndex].health = 0;
						curAvatar->isDead = TRUE;
						lootAllowed = FALSE;

						tempReceiptList.clear();
						tempReceiptList.push_back(curAvatar->socketIndex);
						sprintf(tempText,"You were blown up at %dN %dE.", 
							256-curAvatar->cellY, 256-curAvatar->cellX);

						if (SPACE_REALM == ss->WhatAmI())
						{
							sprintf(tempText,"You were blown up in the %d at %dN %dE.",
							ss->WhoAmI(),
							64-curAvatar->cellY, 64-curAvatar->cellX);
						}
						else if (SPACE_GUILD == ss->WhatAmI())
						{
							sprintf(tempText,"You were blown up in the %d tower at %dN %dE.",
							ss->WhoAmI(),
							5-curAvatar->cellY, 5-curAvatar->cellX);
						}
						else if (SPACE_LABYRINTH == ss->WhatAmI())
						{
							sprintf(tempText,"You were blown up in the Labyrinth at %dN %dE.",
							64-curAvatar->cellY, 64-curAvatar->cellX);
						}
						else if (SPACE_DUNGEON == ss->WhatAmI())
						{
							sprintf(tempText,"You were blown up in the %s (%dN %dE) at %dN %dE.",
							((DungeonMap *) ss)->name,
							256-((DungeonMap *) ss)->enterY,
							256-((DungeonMap *) ss)->enterX, 
							((DungeonMap *) ss)->height - curAvatar->cellY, 
							((DungeonMap *) ss)->width  - curAvatar->cellX);
						}
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
					}
				}
				// look for other players, if we can hit players
				if (CanHitOtherPlayers)
				{
					// go through list of avatars in range.
					BBOSMob *curPlayer = (BBOSMob *)ss->avatars->First();
					BBOSAvatar *curVictim; // player we will be damaging
					while (curPlayer)
					{
						if ((curPlayer->WhatAmI() == SMOB_AVATAR) && (abs(curPlayer->cellX - cellX) < 6) && (abs(curPlayer->cellY - cellY) < 6))
						{
							curVictim = (BBOSAvatar *)curPlayer; // grab victim
						}
						// make sure it's not the dropper, who already got hit once.
						if (curVictim != dropper)
						{
							long damValue = power * 1.0f /
								(1 + Distance(cellX, cellY, curPlayer->cellX, curPlayer->cellY));  // distance between bomb and the player being hit
							// reduce damage if you have explosive mastery
							// find mastery skill

							InventoryObject *io = (InventoryObject *)curVictim->charInfoArray[curVictim->curCharacterIndex].skills->objects.First();
							bool found = FALSE;
							char tmp[80];
							char tempText[80];
							MessInfoText infoText;
							InvSkill *iSkill = NULL;
							sprintf(tmp, "Explosives Mastery");
							while (io && !found) {
								if (!strcmp(tmp, io->WhoAmI())) {
									iSkill = (InvSkill *)io->extra;
									found = 1;
									damValue = damValue * (100 - iSkill->skillLevel) / 100; // level 100 is fully immune, for now. Won't help against the evil bomber boss
								}


								io = (InventoryObject *)curVictim->charInfoArray[curVictim->curCharacterIndex].skills->objects.Next();
							}
							if (damValue > 0)
							{
								// if you are PVPing, you don't have to survive the other guy's bomb to get mastery experience.
								if (iSkill) // if we found Explosive Mastery before
								{
									// if it's not killing blow
									if (damValue < curVictim->charInfoArray[curVictim->curCharacterIndex].health)
									{
									// get full exp;
										iSkill->skillPoints += damValue* bboServer->mastery_exp_multiplier;
									}
									else
									{
										// you get half of your remaining health as a consolation, to ease the pain of your PVP defeat. 
										// This prevents you from cheesing the mastery in PVP, while still letting it improve.  
										iSkill->skillPoints += (curVictim->charInfoArray[curVictim->curCharacterIndex].health / 2)*bboServer->mastery_exp_multiplier;
									}
									// and check for levelup 
									if (iSkill->skillLevel < 100 && iSkill->skillLevel * 100000 <= iSkill->skillPoints) {
										std::vector<TagID> tempReceiptList;

										tempReceiptList.clear();
										tempReceiptList.push_back(curVictim->socketIndex);

										// made a skill level!!!
										iSkill->skillLevel += 1;

										sprintf(tempText, "You gained %s skill!! You feel harder to blow up.", tmp);
										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									}

								}
								if (!curVictim->accountType)  // admins and moderators are still immune to bombs
									curVictim->charInfoArray[curVictim->curCharacterIndex].health -= damValue;

								MessAvatarHealth messHealth;
								messHealth.avatarID = curVictim->socketIndex;
								messHealth.health = curVictim->charInfoArray[curVictim->curCharacterIndex].health;
								messHealth.healthMax = curVictim->charInfoArray[curVictim->curCharacterIndex].healthMax;
								ss->SendToEveryoneNearBut(0, curPlayer->cellX, curPlayer->cellY, sizeof(messHealth), &messHealth);

								if (curVictim->charInfoArray[curVictim->curCharacterIndex].health <= 0)
								{
									curVictim->charInfoArray[curVictim->curCharacterIndex].health = 0;
									curVictim->isDead = TRUE;
									// killing other players doesn't disable loot.

									tempReceiptList.clear();
									tempReceiptList.push_back(curAvatar->socketIndex);
									// just in case realm of pain is a ormla map.
									sprintf(tempText, "You were blown up in the Realm of Pain at %dN %dE.",
										256 - curVictim->cellY, 256 - curVictim->cellX);

									if (SPACE_REALM == ss->WhatAmI())
									{
										sprintf(tempText, "You were blown up in the %d at %dN %dE.",
											ss->WhoAmI(),
											64 - curVictim->cellY, 64 - curVictim->cellX);
									}
									else if (SPACE_GUILD == ss->WhatAmI())
									{
										sprintf(tempText, "You were blown up in the %d tower at %dN %dE.",
											ss->WhoAmI(),
											5 - curVictim->cellY, 5 - curVictim->cellX);
									}
									else if (SPACE_LABYRINTH == ss->WhatAmI())
									{
										sprintf(tempText, "You were blown up in the Labyrinth at %dN %dE.",
											64 - curVictim->cellY, 64 - curVictim->cellX);
									}
									else if (SPACE_DUNGEON == ss->WhatAmI())
									{
										sprintf(tempText, "You were blown up in the %s (%dN %dE) at %dN %dE.",
											((DungeonMap *)ss)->name,
											256 - ((DungeonMap *)ss)->enterY,
											256 - ((DungeonMap *)ss)->enterX,
											((DungeonMap *)ss)->height - curVictim->cellY,
											((DungeonMap *)ss)->width - curVictim->cellX);
									}
									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								}
							}
							// either way, get the next avatar to check.
							curPlayer = (BBOSMob *)ss->avatars->Next();
						}
					}
				}
				// look for monsters
				curMob = (BBOSMob *) ss->mobList->GetFirst(cellX, cellY, power);
				while (curMob)
				{
					if (SMOB_MONSTER  == curMob->WhatAmI())
					{
						if (TRUE)
						{
							BBOSMonster *curMonster = (BBOSMonster *) curMob;

							float dist =1 + Distance(cellX, cellY, curMob->cellX, curMob->cellY); 

							long damValue = power * 1.0f / dist;
							if (dist < 6 && damValue > 0)
							{
								curMonster->bombDropper = dropper;
								if ((curMonster->controllingAvatar)											// if i'm being controlled
									&& (curMonster->bombDropper != curMonster->controllingAvatar)			// by someone who is not the bomb dropper
									&& (!curMonster->controllingAvatar->PVPEnabled)							// and the beastmaster isn't pvp enabled
									&& (curMonster->controllingAvatar->accountType==ACCOUNT_TYPE_PLAYER))	// and it's a normal player (admin monsters can be bombed still)
								{
									damValue = 0;													// zero the damage and ignore the event.
								}
								else
								{
									// do the damage properly.
									curMonster->bombDamage = damValue;
									curMonster->bombOuchTime = timeGetTime() + 200 * dist;                  // damage takes time to propagate from the square.

									MapListState oldState = ss->mobList->GetState();

									if (lootAllowed)
										curMonster->RecordDamageForLootDist(damValue, dropper);
									else
										curMonster->ClearDamageForLootDist(dropper);                        // if you blew yourself up, you get NO loot from anything at all the bomb hit. fair.

									ss->mobList->SetState(oldState);
								}
							}
						}
					}
					curMob = (BBOSMob *) ss->mobList->GetNext();
				}
			}
		}


		// send a message about me to every avatar that is appropriate
		MessExplosion explo;
		explo.avatarID = -1;
//		if (dropper)
//			explo.avatarID = dropper->socketIndex;
		explo.r = r;
		explo.g = g;
		explo.b = b;
		explo.type = type;
		explo.flags = flags;
		explo.size = power;
		explo.x = cellX;
		explo.y = cellY;

		ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(explo), &explo, 5 + power/10);
		// if big eonugh, and enabled in config, try to blow up walls
		if (bboServer->enable_wall_breaking && (ss->WhatAmI() == SPACE_DUNGEON) && (((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && (power >= (((DungeonMap *)ss)->GeoDepth))) // if it's a temporary dungeon
		{
			int wallsdestroyed = 0;
			// get the walls for my square.
			if (((DungeonMap *)ss)->topWall[cellY*((DungeonMap *)ss)->width+cellX] > 0)  // if top wall is there
			{
				((DungeonMap *)ss)->topWall[cellY*((DungeonMap *)ss)->width+cellX] = 0;   // destroy it
				wallsdestroyed++;
				((DungeonMap *)ss)->NumberOfWalls--;
				// send message to clients to remove the wall
				MessDungeonChange NewWall;
				NewWall.floor = 0; // no change in floor
				NewWall.outer = 0; // no change in outer wall
				NewWall.reset = 0; // not a reset
				NewWall.x = cellX; // this cell
				NewWall.y = cellY; // this cell
				NewWall.top = 1;   // zero out the wall  yes packets are off by 1.
				NewWall.left = 0;
				ss->SendToEveryoneNearBut(0, cellX, cellY,
					sizeof(NewWall), (void *)&NewWall);
			}
			if (((DungeonMap *)ss)->leftWall[cellY*((DungeonMap *)ss)->width + cellX] > 0) // if left wall is there
			{
				((DungeonMap *)ss)->leftWall[cellY*((DungeonMap *)ss)->width + cellX] = 0; // destroy it
				wallsdestroyed++;
				((DungeonMap *)ss)->NumberOfWalls--;
				MessDungeonChange NewWall;
				NewWall.floor = 0; // no change in floor
				NewWall.outer = 0; // no change in outer wall
				NewWall.reset = 0; // not a reset
				NewWall.x = cellX; // this cell
				NewWall.y = cellY; // this cell
				NewWall.left = 1;
				NewWall.top = 0;   // zero out the wall.  yes packets are off by 1.
				ss->SendToEveryoneNearBut(0, cellX, cellY,
					sizeof(NewWall), (void *)&NewWall);
			}
            
			// and the other two walls, if their squares exist
			if (((cellY+1)<((DungeonMap *)ss)->height)&&((DungeonMap *)ss)->topWall[(cellY+1)*((DungeonMap *)ss)->width + cellX] > 0)  // if top wall is there
			{
				((DungeonMap *)ss)->topWall[(cellY+1)*((DungeonMap *)ss)->width + cellX] = 0;   // destroy it
				wallsdestroyed++;
				((DungeonMap *)ss)->NumberOfWalls--;
				MessDungeonChange NewWall;
				NewWall.floor = 0; // no change in floor
				NewWall.outer = 0; // no change in outer wall
				NewWall.reset = 0; // not a reset
				NewWall.x = cellX; // this cell
				NewWall.y = cellY+1; // one over
				NewWall.left = 0;
				NewWall.top = 1;   // zero out the wall.  yes packets are off by 1.
				ss->SendToEveryoneNearBut(0, cellX, cellY,
					sizeof(NewWall), (void *)&NewWall);

			}
			if (((cellX+1)< ((DungeonMap *)ss)->width)&&((DungeonMap *)ss)->leftWall[cellY*((DungeonMap *)ss)->width + (cellX+1)] > 0) // if left wall is there
			{
				((DungeonMap *)ss)->leftWall[cellY*((DungeonMap *)ss)->width + (cellX+1)] = 0; // destroy it
				wallsdestroyed++;
				((DungeonMap *)ss)->NumberOfWalls--;
				MessDungeonChange NewWall;
				NewWall.floor = 0; // no change in floor
				NewWall.outer = 0; // no change in outer wall
				NewWall.reset = 0; // not a reset
				NewWall.x = cellX+1; // one over
				NewWall.y = cellY; // this cell
				NewWall.left = 1;
				NewWall.top = 0;   // zero out the wall.  yes packets are off by 1.
				ss->SendToEveryoneNearBut(0, cellX, cellY,
					sizeof(NewWall), (void *)&NewWall);

			}
			// check for collapse
			// add walls destroyed to dungeon's own counter of broken walls.
			((DungeonMap *)ss)->WallsDestroyed += wallsdestroyed;
			// roll the dice. don't add 1.
			int collapse = (rand() % ((DungeonMap *)ss)->maxdestroyed); // you want to roll high here, but can never reach maxdestroyed.
			if (wallsdestroyed < 1) // if you didn't destroy a wall with this bomb.
				collapse = ((DungeonMap *)ss)->maxdestroyed;  // ignore the roll and set it to maxdestroyed so it won't collapse unless you took out too many.
			if (collapse < ((DungeonMap *)ss)->WallsDestroyed) // did you roll too low?
			{
				// kill all avatars on the map!
				BBOSMob* FindPlayer = (BBOSMob *)ss->avatars->First();
				while (FindPlayer)
				{
					if (FindPlayer->WhatAmI() == SMOB_AVATAR) // if i'm a player
					{
						// kill it!
						BBOSAvatar *DeadPlayer = (BBOSAvatar *)FindPlayer;
						DeadPlayer->charInfoArray[DeadPlayer->curCharacterIndex].health =0; // zero health
						DeadPlayer->isDead = TRUE;											// set death flag
						tempReceiptList.clear();
						tempReceiptList.push_back(DeadPlayer->socketIndex);

						sprintf(tempText, "%s became unstable and collapsed!", ((DungeonMap *)ss)->name); // oh noes!
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					// get the next player
					FindPlayer= (BBOSMob *)ss->avatars->Next();
				}

			}
			else
			{
				// update the pathmap
				((DungeonMap *)ss)->SetPathMap();
				
			}
		}
		// kill myself
		isDead = TRUE;
	}
}


/* end of file */



