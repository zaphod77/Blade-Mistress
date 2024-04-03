
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "BBO-Savatar.h"
#include "BBO-Sarmy.h"
#include "BBO-SAutoQuest.h"
#include "BBO-SgroundEffect.h"
#include "ArmyDragonChaplain.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "dungeon-map.h"
#include "MonsterData.h"
#include "StaffData.h"
#include ".\helper\crc.h"
#include "labyrinth-Map.h" // stop those damn bats from flying where we can't get them
#include "realm-map.h"	   // keep monsters for wandering into pits



//******************************************************************
BBOSMonsterGrave::BBOSMonsterGrave(int t, int st, int x, int y) : 
                        BBOSMob(SMOB_MONSTER_GRAVE,"MONSTER_GRAVE")
{
	type = t;
	subType = st;
	cellX = spawnX = x;
	cellY = spawnY = y;
//	spawnTime = timeGetTime() + 60 * (t+1) * (st+1) * 20;
	spawnTime = timeGetTime() + 1000 * 60 * (t+1) * (st+1);
	uniqueName[0] = 0;

}

//******************************************************************
BBOSMonsterGrave::~BBOSMonsterGrave()
{
}

//******************************************************************
void BBOSMonsterGrave::Tick(SharedSpace *ss)
{
//	DWORD delta;
	DWORD now = timeGetTime();
	if (now > spawnTime)
	{
		// kill me and create a monster where I was
		isDead = TRUE;
	}
	// this check can fail if spawnTime is very close to the max of a dword.
	// so we need an additional sanity check to trigger a spawn.
	DWORD delta = spawnTime-now;
	if (delta > (1000*60*60*24*10)) // more than ten day difference means we must have overflowed between ticks.
	{
		// kill me and create a monster where I was
		isDead = TRUE;
	}

}


//******************************************************************
//******************************************************************
BBOSMonster::BBOSMonster( int t, int st, BBOSGenerator *myGen, bool isVag ) : BBOSMob(SMOB_MONSTER,"MONSTER")
{
	myGenerator	= myGen;

	cellX = cellY = 39;
	targetCellX = targetCellY = 39;
	moveStartTime = lastWanderlustTime = lastHealTime = lastEffectTime = 0;
	isMoving = FALSE;
	lastThiefTime = 0;
	thiefMode = THIEF_NOT_THIEF;
	healAmountPerSecond = 1;
	dontRespawn = FALSE;
	form = 0;
	tamingcounter = 0;  // when this reaches 4, the monster is tamed, if tamable.

	inventory = new Inventory(MESS_INVENTORY_MONSTER, this);

	magicResistance = 0;
	archMageMode = FALSE;

	curTarget = FALSE;
	curMonsterTarget = NULL;
	bane = NULL;
	controllingAvatar = NULL;
	questowner = NULL;
	bombDamage = 0;
	TotalbombDamage = 0;

	uniqueName[0] = 0; // no unique name
	r = g = b = a = 255;
	sizeCoeff = 1.0f;
//	inventory = NULL;

	health = maxHealth = monsterData[t][st].maxHealth;
	damageDone         = monsterData[t][st].damageDone;
	toHit              = monsterData[t][st].toHit;
	defense            = monsterData[t][st].defense;
	dropAmount         = monsterData[t][st].dropAmount;
	dropType           = monsterData[t][st].dropType;

	// store weather it is a vagabond or not
	isVagabond = isVag;

	type = t;
	subType = st;
	isWandering = isPossessed = FALSE;

	if (21 == type) // if thief
		thiefMode = THIEF_WAITING;

	creationTime = timeGetTime();

	for (int i = 0; i < 10; ++i)
	{
		attackerPtrList[i] = NULL;
	}

	if (myGenerator)
	{
		myGenerator->count[t][st]++;
//		if (11 == type && 1 == subType && myGenerator->count[t][st] == myGenerator->max[t][st]) // dokk's centurion
//			myGenerator->specialSpawnFlag = TRUE;
//		if (16 == type && 1 == subType && myGenerator->count[t][st] == myGenerator->max[t][st]) // dokk
//			myGenerator->specialSpawnFlag = TRUE;
	}
}

//******************************************************************
BBOSMonster::~BBOSMonster()
{
	delete inventory;

	if (myGenerator)
		myGenerator->count[type][subType]--;

//	SAFE_DELETE(inventory);
}

//******************************************************************
void BBOSMonster::Tick(SharedSpace *ss)
{
	std::vector<TagID> tempReceiptList;
	char tempText[1024];
	MessInfoText infoText;

	BBOSMob *curMob = NULL;
	BBOSAvatar *curAvatar = NULL;
	DungeonMap *dm = NULL;

	if (SPACE_DUNGEON == ss->WhatAmI())
	{
		dm = (DungeonMap *) ss;
//		assert(cellX != 0 || cellY != 0);
	}
	
	DWORD delta;
	DWORD now = timeGetTime();

	// handle bomb damage
	if (bombDamage > 0 && bombOuchTime <= now)
	{
		health -= bombDamage;
		TotalbombDamage += bombDamage;
		if (health <= 0)
		{
			isDead = TRUE;
			bane = bombDropper;

			if (myGenerator && 1 == myGenerator->WhatAmI())
			{
				BBOSArmy *army = (BBOSArmy *)	myGenerator;
				army->MonsterEvent(this, ARMY_EVENT_DIED);
//				curMob = (BBOSMob *) ss->mobs->Find(this); // reset list
			}
			else if (myGenerator && 2 == myGenerator->WhatAmI())
			{
				BBOSAutoQuest *quest = (BBOSAutoQuest *)	myGenerator;
				quest->MonsterEvent(this, AUTO_EVENT_DIED);
			}
		}
		else
		{
			MessMonsterHealth messMH;
			messMH.mobID = (unsigned long)this;
			messMH.health = health;
			messMH.healthMax = maxHealth;
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH),(void *)&messMH, 2);

			if (defense * defense <= bombDamage)
			{
				MonsterMagicEffect(MONSTER_EFFECT_STUN, 40 * 1000, 1);
			}
			if (!HasTarget())
			{
				lastAttackTime = now;
				curTarget = bombDropper;
			}

			if (myGenerator && 1 == myGenerator->WhatAmI())
			{
				BBOSArmy *army = (BBOSArmy *)	myGenerator;
				army->MonsterEvent(this, ARMY_EVENT_ATTACKED);
//				curMob = (BBOSMob *) ss->mobs->Find(this);
			}
			else if (myGenerator && 2 == myGenerator->WhatAmI())
			{
				BBOSAutoQuest *quest = (BBOSAutoQuest *)	myGenerator;
				quest->MonsterEvent(this, AUTO_EVENT_ATTACKED);
			}
		}

		bombDamage = 0;
	}

	if (isMoving)
	{
		delta = now - moveStartTime;

		if (0 == moveStartTime || now < moveStartTime) // || now == moveStartTime)
		{
//			delta = 500 * 4 + 1;
			delta = 1000 * 4 + 1;
		}

		curTarget = FALSE;
		curMonsterTarget = NULL;
		// Finish the move
		 if (delta > 1000 * 4)
//		if (delta > 500 * 4)
			{
			isMoving = FALSE;
			cellX = targetCellX;
			cellY = targetCellY;

			ss->mobList->Move(this);

//			if (ss->map.toughestMonsterPoints[cellX][cellY] < maxHealth)
//				ss->map.toughestMonsterPoints[cellX][cellY] = maxHealth;

			if (uniqueName[0])
			{
				MessMobAppearCustom mAppear;
				mAppear.mobID = (unsigned long) this;
				mAppear.x = cellX;
				mAppear.y = cellY;
				mAppear.monsterType = type;
				mAppear.subType = subType;
				mAppear.type = SMOB_MONSTER;
				CopyStringSafely(Name(), 32, mAppear.name, 32);
				mAppear.a = a;
				mAppear.r = r;
				mAppear.g = g;
				mAppear.b = b;
				mAppear.sizeCoeff = sizeCoeff;

				if(SPACE_DUNGEON == ss->WhatAmI())
				{
					mAppear.staticMonsterFlag = FALSE;
					if (!isWandering && !isPossessed)
						mAppear.staticMonsterFlag = TRUE;
				}
				mAppear.type = WhatAmI();

				// send arrival message
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mAppear), &mAppear,6);
			}
			else
			{
				MessMobAppear mAppear;
				mAppear.mobID = (unsigned long) this;
				mAppear.x = cellX;
				mAppear.y = cellY;
				mAppear.monsterType = type;
				mAppear.subType = subType;
				if(SPACE_DUNGEON == ss->WhatAmI())
				{
					mAppear.staticMonsterFlag = FALSE;
					if (!isWandering && !isPossessed)
						mAppear.staticMonsterFlag = TRUE;
				}
				mAppear.type = WhatAmI();

				// send arrival message
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mAppear), &mAppear,6);
			}

			// make players aggro on me (as I'm arriving)!
			curMob = (BBOSMob *) ss->avatars->First();
			while (curMob)
			{
				if((curMob->WhatAmI()==SMOB_AVATAR) && (curMob->cellX == cellX) && 
					 (curMob->cellY == cellY))
				{
					curAvatar = (BBOSAvatar *) curMob;

					// make her aggro
					if (!curAvatar->curTarget && !curAvatar->isInvisible && // doesn't already have a target and is not invisible
						 !controllingAvatar &&								// and i'm not controlled
						 !(dm && dm->CanEdit(curAvatar)) && !curAvatar->controlledMonster // avatar is not a dungeon mistress, and not controllign a monster
						&& type!=31) // i'm not a Unicorn
					{
						   // make the character aggro,
							curAvatar->lastAttackTime = now;
							curAvatar->curTarget = this;
					}
					if (type == 31)
					{
						curAvatar = NULL; // forget about the avatar if i'm a unicorn, since it wasn't made to attack me.
					}

				}
				curMob = (BBOSMob *) ss->avatars->Next();
			}
			// and if i saw any avatar in this square, i'm going to attack one of them!
			if (!curMonsterTarget && !curTarget && curAvatar && !curAvatar->isInvisible && // i'm not already attacking something, there's an avatar i noticed, it's not invisible
						 !controllingAvatar && !(dm && dm->CanEdit(curAvatar)))			   // i'm not being controlled, and the avatar is not a dungeon mistress 
			{
				lastAttackTime = now - (rand() % 1000);  // I will swing back.
				curTarget = curAvatar;
			}

			if (controllingAvatar)
			{
				SharedSpace *sx;
				if (bboServer->FindAvatar(controllingAvatar, &sx))
				{
					controllingAvatar->UpdateClient(sx);
				}
			}
			// look for army monsters in the square if fi'm a beast master
			if (controllingAvatar && !controllingAvatar->followMode && controllingAvatar->BeastStat() > 9)
			{
				curMob = ss->mobList->GetFirst(cellX, cellY, 0);
				while (curMob)
				{
					if (SMOB_MONSTER == curMob->WhatAmI())
					{
						BBOSMonster * curMonster;
						curMonster = (BBOSMonster *)curMob;

						// make him aggro
						if (!curMonster->curTarget &&
							!curMonster->controllingAvatar &&
							curMonster->myGenerator &&
							(curMonster->myGenerator->do_id > 0) // it's an army monster
							)
						{
							curMonster->lastAttackTime = now - (rand() % 1000);
							curMonster->curMonsterTarget = this;
						}
						else
							curMonster = NULL;
					}
					curMob = ss->mobList->GetNext();
				}

			}
			// look for beastmaster pets if i'm in an army
			if (myGenerator&& (myGenerator->do_id==1)) 
			{
				//look for monsters
				curMob = ss->mobList->GetFirst(cellX, cellY, 0);
				while (curMob)
				{
					if (SMOB_MONSTER == curMob->WhatAmI())
					{
						BBOSMonster * curMonster;
						curMonster = (BBOSMonster *)curMob;

						// aggro on the beastmaster pet
						if (!curTarget && // not already attacking something
							!curMonsterTarget && //not already attacking a monster
							curMonster->controllingAvatar && // is controlled
							curMonster->controllingAvatar->BeastStat()>9 // by a beastmaster
							)
						{
							lastAttackTime = now - (rand() % 1000);
							curMonsterTarget = curMonster;
						}
						else
							curMonster = NULL;
					}
					curMob = ss->mobList->GetNext();
				}

			}
		}
		return;
	}

	delta = now - lastAttackTime;

	if (0 == lastAttackTime || now < lastAttackTime) // || now == moveStartTime)
	{
		delta = 1000 * 2 + 1;
	}

	if (curTarget && !(ss->avatars->Find(curTarget)))
		curTarget = NULL;

	if (curMonsterTarget)
	{
		MapListState state = ss->mobList->GetState();
		int stillThere = FALSE;
		BBOSMob *tMob = NULL;
		tMob = ss->mobList->GetFirst(cellX, cellY);
		while (tMob && !stillThere)
		{
			if ((SMOB_MONSTER == tMob->WhatAmI()) && (curMonsterTarget == tMob)) // this sohuldn't happen, but we are checkign anyway.
				stillThere = TRUE;
			tMob = ss->mobList->GetNext();
		}

		ss->mobList->SetState(state);

		if (!stillThere)
			curMonsterTarget = NULL;

	}

	// ******************** attack curTarget (if it's still there)
	if (curTarget && 
		 delta > 1000 * 2 + magicEffectAmount[DRAGON_TYPE_BLUE] * 300 &&
		 magicEffectAmount[MONSTER_EFFECT_STUN] <= 0 &&
		 !(dm && dm->CanEdit(curTarget))
		)	
	{
		lastAttackTime = now - (rand() % 100);
		int didAttack = FALSE;

		if (form > 0 && type==27) // vlords don't attack in form 1
			return;
		if (curMonsterTarget && (curMonsterTarget->controllingAvatar) && (curMonsterTarget->controllingAvatar->BeastStat() > 9)) // if already attacking a beastmaster pet
		{
			didAttack = TRUE;  // skip the player target as long as the pet is tanking.
		}
		if (27 == type && 3 == (rand() % 6))
		{
			VLordAttack(ss);
			didAttack = TRUE;
			return;
		}
		if (30 == type && subType>1 && 2 == (rand() % 3))
		{
			VLordAttack(ss);
			didAttack = TRUE;
			return;
		}

		curMob = (BBOSMob *) ss->avatars->First();
		while (curMob && !didAttack && !isDead)
		{
			if ((curMob == curTarget) && (curMob->cellX == cellX) && (curMob->cellY == cellY) && (curMob->WhatAmI()==SMOB_AVATAR)) 
			{
				curAvatar = (BBOSAvatar *)curMob;
				if ((curAvatar->followMode) && (curAvatar->controlledMonster) && (curAvatar->controlledMonster->cellX == cellX) && (curAvatar->controlledMonster->cellY == cellY)) // if avatar has a controlled monster and is in my square
				{
					curMonsterTarget = curAvatar->controlledMonster; // attack the controlled monster 
					curTarget = NULL;							     // instead of the player
				}
				else
				{
					didAttack = TRUE;
					curAvatar = (BBOSAvatar *)curMob;

					InvSkill *skillInfo = NULL;
					int dodgeMod = 0;
					int found = 0;
					InventoryObject *io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.First();
					while (io && !found)
					{
						if (!strcmp("Dodging", io->WhoAmI()))
						{
							found = 1;
							skillInfo = (InvSkill *)io->extra;
							dodgeMod = skillInfo->skillLevel - 1;
							int addAmount = toHit - dodgeMod;
							if (addAmount < 0)
								addAmount = 0;
							if (addAmount > 10)
								addAmount = 0;
							skillInfo->skillPoints += addAmount*bboServer->combat_exp_multiplier;

							//						if (abs(dodgeMod - toHit) < 3)
							//							skillInfo->skillPoints += 1;
						}
						if (!strcmp("Pet Energy", io->WhoAmI()) && (curAvatar->followMode))  // beastmaster with pet energy and pet is following
						{
							found = 1;
							skillInfo = (InvSkill *)io->extra;  // get info
							dodgeMod = skillInfo->skillLevel - 1;        /// pet energy protects you as well if your pet is near
							// but this doesn't raise your pet's dodge.
						}
						io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.Next();

					}

					dodgeMod += curAvatar->totemEffects.effect[TOTEM_QUICKNESS];

					//    1. chance = 2d20 + ToHit - player.Physical
					int cVal = toHit - dodgeMod;

					if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
						cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

					int chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

					if (cVal + 40 <= 20) // if monster can't EVER hit
					{
						if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
							chance = 30;  // Hit!
					}

					//    2. if chance > 20, hit was successful
					if (chance > 20)
					{
						//    3. damage = damage
						int dDone = damageDone - curAvatar->totemEffects.effect[TOTEM_TOUGHNESS] / 3;
						if (dDone < 0)
							dDone = 0;

						if (MONSTER_PLACE_SPIRITS &
							monsterData[type][subType].placementFlags)
						{
							float fDone = (float)dDone *
								(1.0f - (curAvatar->totemEffects.effect[TOTEM_PROT_SPIRITS] / 30.0f));
							dDone = (int)fDone;
						}

						dDone += (int)rnd(0, dDone);

						if (curAvatar->totemEffects.effect[TOTEM_LIFESTEAL] > 0)
						{
							int suck = dDone * curAvatar->totemEffects.effect[TOTEM_LIFESTEAL] / 60;
							dDone += suck;
							health += suck;
						}
						if (curAvatar->totemEffects.effect[TOTEM_FOCUS] > 0 //if character has a focus totem
							&& dDone >= curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax) // and damage greater than or equal to max health
							dDone = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax - 1; // lower damage to max health-1 to not oneshot from full health
						if (ACCOUNT_TYPE_ADMIN != curAvatar->accountType)
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= dDone;

						MessMonsterAttack messAA;
						messAA.avatarID = curAvatar->socketIndex;
						messAA.mobID = (long)this;
						messAA.damage = dDone;
						ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);

						MessAvatarHealth messHealth;
						messHealth.avatarID = curAvatar->socketIndex;
						messHealth.health = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
						messHealth.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
						ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messHealth), &messHealth, 2);

						if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health <= 0)
						{
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].health = 0;
							curAvatar->isDead = TRUE;

							tempReceiptList.clear();
							tempReceiptList.push_back(curAvatar->socketIndex);
							if (SPACE_GROUND == ss->WhatAmI())
							{
								sprintf(tempText, "The %s defeats you at %dN %dE.", Name(),
									256 - curAvatar->cellY, 256 - curAvatar->cellX);
							}
							else if (SPACE_REALM == ss->WhatAmI())
							{
								sprintf(tempText, "The %s defeats you in a Mystic Realm at %dN %dE.",
									Name(),
									64 - curAvatar->cellY, 64 - curAvatar->cellX);
							}
							else if (SPACE_LABYRINTH == ss->WhatAmI())
							{
								sprintf(tempText, "The %s defeats you in the Labyrinth at %dN %dE.",
									Name(),
									64 - curAvatar->cellY, 64 - curAvatar->cellX);
							}
							else
							{
								sprintf(tempText, "The %s defeats you in the %s (%dN %dE) at %dN %dE.",
									Name(),
									((DungeonMap *)ss)->name,
									256 - ((DungeonMap *)ss)->enterY,
									256 - ((DungeonMap *)ss)->enterX,
									((DungeonMap *)ss)->height - curAvatar->cellY,
									((DungeonMap *)ss)->width - curAvatar->cellX);
							}
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						}
						else if (!(curAvatar->curTarget))
						{
							curAvatar->lastAttackTime = now;
							curAvatar->curTarget = this;
						}
						if (31 == type) // unicorn,
						{
							if ((rand() % 10) < (type + 1)) // better unicorms do it more often
							{
								UnicornAttack(ss); // pretty attack
							}
						}
						if (24 == type) // spidren group
						{
							cVal = toHit - dodgeMod;

							if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
								cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

							cVal -= curAvatar->totemEffects.effect[TOTEM_PROT_WEB];

							chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

							if (cVal + 40 <= 20) // if monster can't EVER hit
							{
								if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
									chance = 30;  // Hit!
							}

							//    2. if chance > 20, hit was successful
							if (chance > 20)
							{
								// spin a web on her ass!
								MessMagicAttack messMA;
								messMA.damage = 30;
								messMA.mobID = -1;
								messMA.avatarID = curAvatar->socketIndex;
								messMA.type = 1;
								ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
									sizeof(messMA), &messMA, 3);

								tempReceiptList.clear();
								tempReceiptList.push_back(curAvatar->socketIndex);
								sprintf(tempText, "The %s ensnares you in its web!", Name());
								CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
								ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

								int effectVal = toHit - dodgeMod / 2 - curAvatar->totemEffects.effect[TOTEM_PROT_WEB] * 2;
								if (effectVal < 2)
									effectVal = 2;

								curAvatar->MagicEffect(MONSTER_EFFECT_BIND,
									timeGetTime() + effectVal * 1000, effectVal);
								curAvatar->MagicEffect(MONSTER_EFFECT_TYPE_BLUE,
									timeGetTime() + effectVal * 1000, effectVal);

							}
						}
					}
					else
					{
						// whish!
						MessMonsterAttack messAA;
						messAA.avatarID = curAvatar->socketIndex;
						messAA.mobID = (long)this;
						messAA.damage = -1; // miss!
						ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);

						if (!(curAvatar->curTarget))
						{
							curAvatar->lastAttackTime = now;
							curAvatar->curTarget = this;
						}

					}
				}
			}
			curMob = (BBOSMob *) ss->avatars->Next();
		}

		if (!didAttack)
		{
			curTarget = NULL;
		}

	}

	// ******************** attack curMonsterTarget
	if (curMonsterTarget && curMonsterTarget != this &&
		delta > 1000 * 2 + magicEffectAmount[DRAGON_TYPE_BLUE] * 300 &&
		magicEffectAmount[MONSTER_EFFECT_STUN] <= 0)
	{
		lastAttackTime = now;
		int didAttack = FALSE;
		if (form > 0 && type == 27) // vlords don't attack in form 1
			return;
		if (controllingAvatar && controllingAvatar->BeastStat() > 9) // if i'm a beastmaster pet
		{
			//chance of spin attacks against monsters
			if (27 == type && 3 == (rand() % 6))
			{
				BMVLordAttack(ss);
				didAttack = TRUE;
				return;
			}
			if (30 == type && subType > 1 && 2 == (rand() % 3))
			{
				BMVLordAttack(ss);
				didAttack = TRUE;
				return;
			}
		}
		// trigger normal spin attacks if attacking a beastmaster pet and not being controlled.
		if (!controllingAvatar && curMonsterTarget && (curMonsterTarget->controllingAvatar) && (curMonsterTarget->controllingAvatar->BeastStat() > 9)) // if i'm not a pet, but i'm attacking one.
		{
			// do the against player AOE so i can strike the beastmaster anyway if he's inn the square :)
			if (27 == type && 3 == (rand() % 6))
			{
				VLordAttack(ss);
				didAttack = TRUE;
				return;
			}
			if (30 == type && subType > 1 && 2 == (rand() % 3))
			{
				VLordAttack(ss);
				didAttack = TRUE;
				return;
			}
		}
		// if Pet Energy is higher, use that instead of own defense
		int petenergy=0;
		if (curMonsterTarget->controllingAvatar // safe because we have a curMonstertarget
			&& curMonsterTarget->controllingAvatar->BeastStat() > 9) // beast stat > 9
		{
			// go through skills
			InventoryObject *io = (InventoryObject *)
				curMonsterTarget->controllingAvatar->charInfoArray[curMonsterTarget->controllingAvatar->curCharacterIndex].skills->objects.First();
			while (io)
			{
				if (!strcmp("Pet Energy", io->WhoAmI())) // check for pet energy (beastmaster version of dodge)
				{
					InvSkill *skillInfo = (InvSkill *)io->extra;
					petenergy = skillInfo->skillLevel;
				}
				io = (InventoryObject *)
					curMonsterTarget->controllingAvatar->charInfoArray[curMonsterTarget->controllingAvatar->curCharacterIndex].skills->objects.Next();
			}
		}
		int cVal;
		if (petenergy> curMonsterTarget->defense)
		{
			cVal = toHit -petenergy;
		}
		else
		{
			cVal = toHit - curMonsterTarget->defense;
		}

		if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
			cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

		int chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

		if (cVal + 40 <= 20) // if monster can't EVER hit
		{
			if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
				chance = 30;  // Hit!
		}
		if (controllingAvatar && (controllingAvatar->BeastStat() > 9))  // if controlled by a beastmaster
		{
			// extra chance to hit based on age
			if (rand() % 16 < controllingAvatar->charInfoArray[controllingAvatar->curCharacterIndex].age)  // wtf
			{
				chance = 30;  // Hit!
			}
		}
      //    2. if chance > 20, hit was successful
		if (chance > 20)
		{
	      //    3. damage = damage
			int dDone = damageDone;
			if (dDone < 0)
				dDone = 0;

			dDone += (int) rnd(0, dDone);
			if (controllingAvatar && (controllingAvatar->BeastStat() > 9)) // if beastmaster
			{
				// add mastery damage.
				InventoryObject *io = (InventoryObject *)controllingAvatar->charInfoArray[controllingAvatar->curCharacterIndex].skills->objects.First();
				bool found = FALSE;
				char tmp[80];
				MessInfoText infoText;

				sprintf(tmp, "Pet Mastery");
				while (io && !found) {
					if (!strcmp(tmp, io->WhoAmI())) {
						InvSkill *iSkill = (InvSkill *)io->extra;
						found = 1;
							dDone += (iSkill->skillLevel*controllingAvatar->BeastStat()); // add beast stat to damage of each pet for each level of pet mastery. Very useful at first level for starting beast master
						}


					io = (InventoryObject *)controllingAvatar->charInfoArray[controllingAvatar->curCharacterIndex].skills->objects.Next();
				}
			}
			MessMonsterAttack messAA;
			messAA.avatarID = (int) curMonsterTarget;
			messAA.mobID    = (long) this;
			messAA.damage   = dDone;
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA,2);

			curMonsterTarget->health -= dDone;
			// damage done boosts pet energy
			// if target is controlled by a beastmaster
			if (curMonsterTarget->controllingAvatar // safe because we have a curMonstertarget
				&& curMonsterTarget->controllingAvatar->BeastStat() > 9) // beast stat > 9
			{
				// go through skills
				InventoryObject *io = (InventoryObject *)
					curMonsterTarget->controllingAvatar->charInfoArray[curMonsterTarget->controllingAvatar->curCharacterIndex].skills->objects.First();
				while (io)
				{
					if (!strcmp("Pet Energy", io->WhoAmI())) // check for pet energy (beastmaster version of dodge)
					{
						InvSkill *skillInfo = (InvSkill *)io->extra;
						skillInfo->skillPoints += (1+min((dDone/2),9))*bboServer->combat_exp_multiplier; // always at least 1, but no more than 10 
					}
					io = (InventoryObject *)
						curMonsterTarget->controllingAvatar->charInfoArray[curMonsterTarget->controllingAvatar->curCharacterIndex].skills->objects.Next();
				}
			}

			if (controllingAvatar && (controllingAvatar->BeastStat() > 9)) // beastmaster controlled monster attacking
			{
				curMonsterTarget->RecordDamageForLootDist(dDone, controllingAvatar); // credit the controller for the damage.  this ensures that the controller gets loot and trips quest death flags.
				// and raise characters Pet Mastery
				InventoryObject *io = (InventoryObject *)controllingAvatar->charInfoArray[controllingAvatar->curCharacterIndex].skills->objects.First();
				bool found = FALSE;
				char tmp[80];
				char tempText[80];
				MessInfoText infoText;

				sprintf(tmp, "Pet Mastery");
				while (io && !found) {
					if (!strcmp(tmp, io->WhoAmI())) {
						InvSkill *iSkill = (InvSkill *)io->extra;
						found = 1;
						// stays slow as the other masteries, but this one will give clevels.
						if (iSkill->skillLevel < 100 && iSkill->skillLevel * 100000 <= iSkill->skillPoints) { 
							std::vector<TagID> tempReceiptList;

							tempReceiptList.clear();
							tempReceiptList.push_back(controllingAvatar->socketIndex);

							// made a skill level!!!
							iSkill->skillLevel += 1;

							sprintf(tempText, "You gained %s skill!! You feel more in tune with your pets.", tmp);
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}

						iSkill->skillPoints += 1* bboServer->mastery_exp_multiplier;
					}

					io = (InventoryObject *)controllingAvatar->charInfoArray[controllingAvatar->curCharacterIndex].skills->objects.Next();
				}
			}
			MessMonsterHealth messMH;
			messMH.mobID = (unsigned long)curMonsterTarget;
			messMH.health = curMonsterTarget->health;
			messMH.healthMax = curMonsterTarget->maxHealth;
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH),(void *)&messMH, 2);
			// send attack event to army so they will move in on me.
			if (curMonsterTarget->myGenerator && 1 == curMonsterTarget->myGenerator->WhatAmI()) // target has a genrator,a dn it's an army
			{
				BBOSArmy *army = (BBOSArmy *)curMonsterTarget->myGenerator;
				army->MonsterEvent(curMonsterTarget, ARMY_EVENT_ATTACKED); // send attack event to tell army that this member was attacked.
				//				curMob = (BBOSMob *) ss->mobs->Find(this);
			}
			if (curMonsterTarget->myGenerator && 2 == curMonsterTarget->myGenerator->WhatAmI()) // target has a genrator,a dn it's an quest
			{
				BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonsterTarget->myGenerator;
				quest->MonsterEvent(curMonsterTarget, AUTO_EVENT_ATTACKED); // send attack event to tell quest that this member was attacked (if it cares).
				//				curMob = (BBOSMob *) ss->mobs->Find(this);
			}
			// if monster dies
			if (curMonsterTarget->health <= 0)
			{
				MessMonsterAttack messAA;
				messAA.avatarID = (int) curMonsterTarget;
				messAA.mobID    = (long) this;
				messAA.damage   = -1000;
//				messAA.health   = -1000;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);
				// beastmasters pet stats improve for killing monsters.
				if (controllingAvatar && (controllingAvatar->BeastStat() > 9))
				{
					// do stat check to see if we can get a boost.
					if (defense <= curMonsterTarget->toHit) // if monsters tohit >= defense
					{
						//boost some stats
						maxHealth = maxHealth * (defense + 1) / defense; //recalculate health as if it were a higher level monster.
						defense++; // better defense to keep from gettign advancement from the same types of monsters.
						toHit++;   // so we can hit more often.
						damageDone++; // and do some more damage.
						controllingAvatar->SaveAccount(); // and save the character so the pets boosted stats get saved.
						// and, sometimes, add gold dust to the dead monster's inventory
						bool GiveGold = FALSE;  // default, will set to true later.
						int GoldEveryXLevels = 1; // change me to make them more or less common.
						if ((controllingAvatar->GetTamingExp() < 1) && (controllingAvatar->GetTamingLevel()<2)) // no tame skill, or haven't tamed the next golem.
						{
							// we haven't tamed anything else, you are allowed to reset to gather more golds for your first Scythe
							GiveGold = TRUE;
						}
						if ((controllingAvatar->GetTamingLevel() != type) || (controllingAvatar->GetTamingExp() != 1)) // if current monster is not the one that will be replaced if you release
						{
							// we will have to retame a new one to bump it's level back, using charges.
							GiveGold = TRUE;
						}
						if (type>26) // if monster is vlord or better
						{
							// go ahead and let them get gold dust anyway, even if the level is reset. tey are already hard to level withotu spending mucho gold
							GiveGold = TRUE;
						}
						if (GiveGold && ((defense-monsterData[type][subType].defense) % GoldEveryXLevels) == 0) // every five levels, if we aren't disqualified.
						{
							// create a new Glowing Gold dust
							InventoryObject * iObject = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Gold Dust");
							InvIngredient * exIn = (InvIngredient *)iObject->extra;
							exIn->type = INGR_GOLD_DUST;
							exIn->quality = 1;

							iObject->mass = 0.0f;
							iObject->value = 1000;
							iObject->amount = 1;
							// add it to the inventory of the monster that's about to die
							curMonsterTarget->inventory->AddItemSorted(iObject);

						}
					}
				}
				curMonsterTarget->isDead = TRUE;
				curMonsterTarget->bane = NULL;
				if (controllingAvatar)
				{
					curMonsterTarget->bane = controllingAvatar;  // set bane
					curMonsterTarget->HandleQuestDeath();		 // call quest death handler
				}


				if (curMonsterTarget->myGenerator && 1 == curMonsterTarget->myGenerator->WhatAmI())
				{
					MapListState oldState = ss->mobList->GetState();

					BBOSArmy *army = (BBOSArmy *)	curMonsterTarget->myGenerator;
					army->MonsterEvent(curMonsterTarget, ARMY_EVENT_DIED);

					ss->mobList->SetState(oldState);
				}
				else if (curMonsterTarget->myGenerator && 2 == curMonsterTarget->myGenerator->WhatAmI())
				{
					MapListState oldState = ss->mobList->GetState();

					BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonsterTarget->myGenerator;
					quest->MonsterEvent(curMonsterTarget, AUTO_EVENT_DIED);

					ss->mobList->SetState(oldState);
				}
			}
			else if (!curMonsterTarget->HasTarget())
			{
				curMonsterTarget->lastAttackTime = now;
				curMonsterTarget->curMonsterTarget = this;
			}
			if (31 == type) // unicorn,
			{
				if ((rand() % 10) < (subType + 1)) // better unicorms do it more often
				{
					BMUnicornAttack(ss); // pretty attack
				}
			}

			if (24 == type) // spidren group
			{
				cVal = toHit - curMonsterTarget->defense;

				if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
					cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

				chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

				if (cVal + 40 <= 20) // if monster can't EVER hit
				{
					if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
						chance = 30;  // Hit!
				}

				//    2. if chance > 20, hit was successful
				if (chance > 20)
				{
					// spin a web on her ass!
					MessMagicAttack messMA;
					messMA.damage = 30;
					messMA.mobID = (unsigned long)curMonsterTarget;
					messMA.avatarID = -1;
					messMA.type = 1;
					ss->SendToEveryoneNearBut(0, cellX, cellY, 
									sizeof(messMA), &messMA,3);

					int effectVal = toHit - curMonsterTarget->defense;
					if (effectVal < 2)
						effectVal = 2;

					curMonsterTarget->MagicEffect(MONSTER_EFFECT_BIND, 
							timeGetTime() + effectVal * 1000, effectVal);
					curMonsterTarget->MagicEffect(MONSTER_EFFECT_TYPE_BLUE, 
							timeGetTime() + effectVal * 1000, effectVal);

				}
			}
		}
		else
		{
			// whish!
			MessMonsterAttack messAA;
			messAA.avatarID = (int) curMonsterTarget;
			messAA.mobID    = (long) this;
			messAA.damage   = -1; // miss!
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA,2);

			if (!curMonsterTarget->HasTarget())
			{
				curMonsterTarget->lastAttackTime = now;
				curMonsterTarget->curMonsterTarget = this;
			}
			// if you pet's natural defese is above pet energy, then you can get boost for misses as well
			if (curMonsterTarget->controllingAvatar // safe because we have a curMonstertarget
				&& curMonsterTarget->controllingAvatar->BeastStat() > 9) // beast stat > 9
			{
				// go through skills
				InventoryObject *io = (InventoryObject *)
					curMonsterTarget->controllingAvatar->charInfoArray[curMonsterTarget->controllingAvatar->curCharacterIndex].skills->objects.First();
				while (io)
				{
					if (!strcmp("Pet Energy", io->WhoAmI())) // check for pet energy (beastmaster version of dodge)
					{
						InvSkill *skillInfo = (InvSkill *)io->extra;
						if (skillInfo->skillLevel < curMonsterTarget->defense) // our pet energy is behindpos
						{
							int energydiff = toHit - skillInfo->skillLevel+1;
							if ((energydiff > 0) && (energydiff < 11))
								skillInfo->skillPoints += energydiff*bboServer->combat_exp_multiplier;
						}
					}
					io = (InventoryObject *)
						curMonsterTarget->controllingAvatar->charInfoArray[curMonsterTarget->controllingAvatar->curCharacterIndex].skills->objects.Next();
				}
			}
		}
	}


	delta = now - lastHealTime;

	if (0 == lastHealTime || now < lastHealTime)
	{
		delta = 1000 * 1 + 1;
	}

	if (delta > 1000 * 1)	
	{
		lastHealTime = now;
		health += healAmountPerSecond;

		if ((7 == type && 4 == subType) && (!controllingAvatar)) // Dragon Overlord NOT under control. admins cna give it back it's regen.
		{
			if (!strcmp(uniqueName,"Dragon Chaplain"))
			{
				//The Dragon Chapalin have a special 
				//healing power of 200 hp +(number of Dragon Followers around * 100 hp)			
				health += 200;

				ArmyChaplain *army = (ArmyChaplain *)	myGenerator;
				int totalLivingPrelates = 0;

				// for each monster atEase
				ArmyMember *curMember = (ArmyMember *) army->atEase.First();
				while (curMember)
				{
					if (!curMember->isDead && curMember->quality < 10)
						++totalLivingPrelates;
					curMember = (ArmyMember *) army->atEase.Next();
				}
				// for each focus
				for (int f = 0; f < 3; ++f)
				{
					// for each monster in the focus
					curMember = (ArmyMember *) army->focus[f].First();
					while (curMember)
					{
						if (!curMember->isDead && curMember->quality < 10)
							++totalLivingPrelates;
						curMember = (ArmyMember *) army->focus[f].Next();
					}
				}

				health += 100 * totalLivingPrelates;
			}
			else if (!strcmp(uniqueName,"Dragon ArchMage"))
			{
				health += 200;
			}
			else
				health += 600;
		}
		else if ((7 == type && 3 == subType) && (!controllingAvatar)) // Dragon Archon NOT controlled by a player
		{
			if (!strcmp(uniqueName,"Dragon Prelate"))
			{
				;
			}
			else if (!strcmp(uniqueName,"Dragon Acolyte"))
			{
				health += 60;
			}
			else
				health += 150;
		}
		else if (24 == type && 4 == subType && (!controllingAvatar)) // lab bosses, player can get one if clever, so no regen.
		{
			healAmountPerSecond = 120 * sizeCoeff;
		}
		else if ((27 == type) && (!controllingAvatar)) // vlords
		{
			healAmountPerSecond = 200 * (1 + subType);
			if (healAmountPerSecond < 250)  // if it's baph
				healAmountPerSecond = 250;  // boost it a bit.
		}
		else if ((30 == type) && (!controllingAvatar))// butterflies
		{
			healAmountPerSecond = 600 * (subType -1);
			if (healAmountPerSecond < 1)  // for non boss butterfies that have 0 or negative regen
				healAmountPerSecond = 1;  // their regen is fixed back to 1.
		}


		if (health > maxHealth)
		{
			health = maxHealth;
			archMageMode = FALSE;
		}
		// also run AI checks.
		if (SpecialAI)
		{
			// monster has special behavior
			AICounter++; // bump the counter up. this is to allow for actions to hapen after a number of seconds.  teh script can change this later.
			// big bad boss A.I.
			if (type == 33) // doesnt' exist yet, but we need to code this
			{
				AICounter--; // undo the counter increment, this monster uses the couter for other reasons.
				// first check for players on the map
				BBOSMob *curMob2 = (BBOSMob *)ss->avatars->First();
				if (!curMob2) // if ther are no players on my map
				{
					// complete reset
					AICounter = 0;         // reset counter
					cellX = spawnX;        // go back to spawn whiel nobody is looking.
					cellY = spawnY;
					targetCellX = spawnX;  // target spawn
					targetCellY = spawnY;
					health = maxHealth;		// get back all health
					
					for (int x = 0; x < MONSTER_EFFECT_TYPE_NUM; ++x)
					{
						MonsterMagicEffect(x, 0, 0); // clear all magic effects
					}
					// despawn stuff created
				}
				else
				{
					// there is at least one player here
					// scan player list for people with stat totems
					while (curMob2)
					{
						BBOSAvatar * curCheater = (((BBOSAvatar *)curMob2));
						if ((curCheater->charInfoArray[curCheater->curCharacterIndex].creative < curCheater->CreativeStat())
							|| (curCheater->charInfoArray[curCheater->curCharacterIndex].magical < curCheater->MagicalStat())
							|| (curCheater->charInfoArray[curCheater->curCharacterIndex].physical < curCheater->PhysicalStat())) // if the character has a stat totem
						{
							// boss instakills you, he considers stat totems cheating!
							// sends you a message

							std::vector<TagID> targetReceiptList;
							targetReceiptList.clear();
							targetReceiptList.push_back(curAvatar->socketIndex);

							sprintf(&(tempText[1]), "The Boss tells you, I despise cheaters!  Return without your stat totem!");
							tempText[0] = NWMESS_PLAYER_CHAT_LINE;
							ss->lserver->SendMsg(strlen(tempText) + 1, (void *)&tempText, 0, &targetReceiptList);
							curCheater->charInfoArray[curCheater->curCharacterIndex].health = 0;    // dead
							curCheater->isDead = TRUE;												// you're dead!
						}
						curMob2 = (BBOSMob *)ss->avatars->Next();
					}
					
				}

			}
			// Unicorn AI, when enabled.
			if (type == 31) // unicorn
			{
//				if (subType > 0) //types greater than zero
//				{
//					if (AICounter > 3600)   // hour has passed
//					{
//						AICounter = 0;		// reset the counter
//						SpecialAI = FALSE;  // turn off my AI, because i'm gonna poof later
//						// summon a Unicorn of lesser type, if in normal realm
//						if (ss->WhatAmI() == SPACE_GROUND)
//						{
//							// add new unicorn in my square
//							BBOSMonster *monster = new BBOSMonster(31, subType - 1, NULL);
//							monster->cellX = monster->targetCellX = monster->spawnX = cellX; 
//							monster->cellY = monster->targetCellY = monster->spawnY = cellY;
//							monster->SpecialAI = TRUE;										  // new Unicorn has my AI.
//							ss->mobList->Add(monster);
//						}
//						// kill myself with no loot
//						type = 25; // vamp me
//						dropAmount = 0; // remove drops
//						health = 0;
//						isDead = TRUE;
//					}
//				}
				if (controllingAvatar) //if i'm controlled
				{
					SpecialAI = FALSE; // turn off my AI, i wont do anything else
					if ((ss->WhatAmI()==SPACE_GROUND) && (controllingAvatar->BeastStat() > 9)) // if controlled by a BeastMaster and on main map
					{
						// we spawn a new one elsewhere. not on the edge
						int town = rand() % NUM_OF_TOWNS;
						int randomX = rand() % 10 - 4;
						int randomY = rand() % 10 - 4;
						int newSub = subType;
						if (newSub > 5)
							newSub = 5;
						BBOSMonster *monster = new BBOSMonster(31, newSub, NULL);
						monster->cellX = monster->targetCellX = monster->spawnX = randomX + townList[town].x;
						monster->cellY = monster->targetCellY = monster->spawnY = randomY + townList[town].y;
						monster->SpecialAI = TRUE;										  // Shiny Unicorn has an AI.
						bboServer->UnicornLocations[monster->subType][0] = monster->cellX;
						bboServer->UnicornLocations[monster->subType][1] = monster->cellY;
						ss->mobList->Add(monster);

					}

				}
				if (!ss->CanMove(cellX, cellY, cellX, cellY))  // somehow ended up on a square i can't move to
				{
					SpecialAI = FALSE; // shut off to avoid possible crash
					dropAmount = 0; // drop no loot
					type = 25;      // vamp me
					health = 0;
					maxHealth = 0;     // i'm dead and will be replaced
					isDead = TRUE;

				}
			}
		}

	}
	
	// process effects
	delta = now - lastEffectTime;

	if (0 == lastEffectTime || now < lastEffectTime)
	{
		delta = 1000 * 10 + 1;
	}

	// Effect damage
	if (delta > 1000 * 10)	
	{
		lastEffectTime = now;

		if (25 == type && !uniqueName && SPACE_GROUND == ss->WhatAmI()) // if vampire
		{
			if (5 == rand() % 50)
			{
				int oneHour = 60 * 6;
				if (bboServer->dayTimeCounter < 2.0f * oneHour ||
					 bboServer->dayTimeCounter > 3.85f * oneHour)
				{
					// die with no loot for anyone!
					for (int i = 0; i < 10; ++i)
						attackerPtrList[i] = NULL; 
					isDead = TRUE;
					dropAmount = 0; // dont' drop anything!
				}
			}
		}

		if (!strncmp(uniqueName, "Revenant", 8)) // revs get cleaned up at end of the day
		{
			LongTime ltNow;
			if (creationLongTime.MinutesDifference(&ltNow) > 60 * 24)
			{
				// die with no loot for anyone!
				for (int i = 0; i < 10; ++i)
					attackerPtrList[i] = NULL;
				isDead = TRUE;
				dropAmount = 0; // dont' drop anything!
			}
		}
		if (!strncmp(uniqueName, "Storm Creature", 14)) // clean up storm creatures
		{
			LongTime ltNow;
			if (creationLongTime.MinutesDifference(&ltNow) > 10) // quest lasts ten minutes, so safe to clean them up 10 minutes after creation
			{
				// die with no loot for anyone!
				for (int i = 0; i < 10; ++i)
					attackerPtrList[i] = NULL;
				isDead = TRUE;
				dropAmount = 0; // dont' drop anything!
			}
		}

		// possessed monsters OCCASSIONALLY move
		if (bboServer->possessed_monsters_move)
		if (!strncmp(uniqueName,"Possessed", 9) && 23 == (rand() % 250))
		{
			// what direction do I want to go?
			int tX = cellX;
			int tY = cellY;

			int randDir = rand() % 4;

			if (0 == randDir)
				--tX;
			else if (1 == randDir)
				++tX;
			else if (2 == randDir)
				--tY;
			else if (3 == randDir)
				++tY;

			int willGo = TRUE;

			// Can I go there?
			if (tX == cellX && tY == cellY)
				willGo = FALSE;
			if (!ss->CanMove(cellX, cellY, tX, tY))
				willGo = FALSE;
			// code real movement check.
			/*
			if (ss->WhatAmI() == SPACE_GROUND) // if it's the main map.
			{
			if (!((GroundMap *)ss)->CanMove(cellX, cellY, tX, tY))
			willGo = FALSE;
			}
			if (ss->WhatAmI() == SPACE_REALM) // if it's a realm.
			{
			if (!((RealmMap *)ss)->CanMove(cellX, cellY, tX, tY))
			willGo = FALSE;
			}
			if (ss->WhatAmI() == SPACE_LABYRINTH) // if it's a labyrinth
			{
			if (!((LabyrinthMap *)ss)->CanMove(cellX, cellY, tX, tY))
			willGo = FALSE;
			}
			if (ss->WhatAmI() == SPACE_DUNGEON) // if it's a dungeon
			{
			if (!((DungeonMap *)ss)->CanMove(cellX, cellY, tX, tY))
			willGo = FALSE;
			}
			*/

			if (willGo)
			{
				// okay, let's go!
				isMoving = TRUE;
				targetCellX = tX;
				targetCellY = tY;
				moveStartTime = timeGetTime();

				MessMobBeginMove bMove;
				bMove.mobID = (unsigned long) this;
				bMove.x = cellX;
				bMove.y = cellY;
				bMove.targetX = tX;
				bMove.targetY = tY;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);
			}
		}

		// Reindeer move a whole lot!
		if (type==32) // it's a reindeer! 
		{
			// what direction do I want to go?
			int tX = cellX;
			int tY = cellY;

			int randDir = rand() % 4;

			if (0 == randDir)
				--tX;
			else if (1 == randDir)
				++tX;
			else if (2 == randDir)
				--tY;
			else if (3 == randDir)
				++tY;

			int willGo = TRUE;

			// Can I go there?
			if (tX == cellX && tY == cellY)
				willGo = FALSE;
			if (!ss->CanMove(cellX, cellY, tX, tY))
				willGo = FALSE;
		
			if (willGo)
			{
				// let's sparkle a little, for christmas cheer. :)

				MessGenericEffect messGE;
				messGE.avatarID = -1;
				//	messGE.mobID = (long)curMob;
				messGE.mobID = this->do_id;
				messGE.x = cellX;
				messGE.y = cellY;
				if (subType > 0)
				{
					messGE.r = 255;
					messGE.g = 25;
					messGE.b = 25;
				}
				else
				{
					messGE.r = 50;
					messGE.g = 255;
					messGE.b = 50;
				}
				messGE.type = 0;  // type of particles
				messGE.timeLen = 1; // in seconds
				ss->SendToEveryoneNearBut(0, cellX, cellY,
					sizeof(messGE), (void*)&messGE);

				// okay, let's go!
				isMoving = TRUE;
				targetCellX = tX;
				targetCellY = tY;
				moveStartTime = timeGetTime();

				MessMobBeginMove bMove;
				bMove.mobID = (unsigned long)this;
				bMove.x = cellX;
				bMove.y = cellY;
				bMove.targetX = tX;
				bMove.targetY = tY;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);
			}
		}
		// Unicorns prance a lot!
		if (type == 31) // it's a pretty unicorn! 
		{
			// what direction do I want to go?
			int tX = cellX;
			int tY = cellY;

			int randDir = rand() % 4;

			if (0 == randDir)
				--tX;
			else if (1 == randDir)
				++tX;
			else if (2 == randDir)
				--tY;
			else if (3 == randDir)
				++tY;
			
			int willGo = FALSE;
			// see if there are any nearby players
			BBOSMob *curMob2 = (BBOSMob *)ss->avatars->First();
			while (curMob2)
			{
				if ((curMob2->WhatAmI() == SMOB_AVATAR) && (((BBOSAvatar *)curMob2)->accountType==ACCOUNT_TYPE_PLAYER) && abs(cellX - curMob2->cellX) < 4 &&
					abs(cellY - curMob2->cellY) < 4)
				{
					willGo = TRUE;
				}
				curMob2 = (BBOSMob *)ss->avatars->Next();
			}
			// Can I go there?
			if (tX == cellX && tY == cellY)
				willGo = FALSE;
			if (!ss->CanMove(cellX, cellY, tX, tY))
				willGo = FALSE;
			// sentinels don't prance.
			if (r == 0 && g == 0 && b == 255)  // sentinels have this particular RGB value, we don't need to check the name.
			{
				willGo = FALSE;
			}
			if (willGo)
			{
				// let's sparkle a little
				int randr = rand() % 256; // random color. yay magic! :)
				int randg = rand() % 256;
				int randb = rand() % 256;
				if (!controllingAvatar)
				{
					MessGenericEffect messGE;
					messGE.avatarID = -1;
					//	messGE.mobID = (long)curMob;
					messGE.mobID = this->do_id;
					messGE.x = cellX;
					messGE.y = cellY;
					messGE.r = randr;
					messGE.g = randg;
					messGE.b = randb;
						messGE.type = 0;  // type of particles
					messGE.timeLen = 2; // in seconds
					ss->SendToEveryoneNearBut(0, cellX, cellY,
						sizeof(messGE), (void*)&messGE);
				}
				else
				{
					BBOSGroundEffect *bboGE = new BBOSGroundEffect();
					bboGE->type = 4;
					bboGE->amount = 200;
					bboGE->r = randr;
					bboGE->g = randg;
					bboGE->b = randb;
					bboGE->cellX = cellX;
					bboGE->cellY = cellY;
					bboGE->killme = 200; // make it go away after a small bit.
					ss->mobList->Add(bboGE);

					MessGroundEffect messGE;
					messGE.mobID = (unsigned long)bboGE;
					messGE.type = bboGE->type;
					messGE.amount = bboGE->amount;
					messGE.x = bboGE->cellX;
					messGE.y = bboGE->cellY;
					messGE.r = bboGE->r;
					messGE.g = bboGE->g;
					messGE.b = bboGE->b;

					ss->SendToEveryoneNearBut(0,
						cellX, cellY,
						sizeof(messGE), &messGE);
				}
				// okay, let's go!
				if (!controllingAvatar) // only actually move if not controlled. :)
				{
					isMoving = TRUE;
					targetCellX = tX;
					targetCellY = tY;
					moveStartTime = timeGetTime();

					MessMobBeginMove bMove;
					bMove.mobID = (unsigned long)this;
					bMove.x = cellX;
					bMove.y = cellY;
					bMove.targetX = tX;
					bMove.targetY = tY;
					ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);
					// update my location for later saving
					if (SpecialAI) // only if i'm one of the ones on the main map.
					{
						bboServer->UnicornLocations[subType][0] = tX;
						bboServer->UnicornLocations[subType][1] = tY;
					}
				}
			}
		}

		for (int i = 0; i < MONSTER_EFFECT_TYPE_NUM; ++i)
		{
			if (magicEffectAmount[i] > 0)
			{
				if (magicEffectTimer[i] < now)
					magicEffectAmount[i] = 0;
				else if (MONSTER_EFFECT_TYPE_BLACK == i)
				{
					health -= magicEffectAmount[i] / 2.0f;

					MessMonsterHealth messMH;
					messMH.mobID = (unsigned long)this;
					messMH.health = health;
					messMH.healthMax = maxHealth;
					ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH),(void *)&messMH, 2);

					if (health <= 0)
					{
						health = 0;
						isDead = TRUE;
					}
				}

			}
		}

		// decay the attacker info here
		for (int i = 0; i < 10; ++i)
		{
			if (attackerPtrList[i])
			{
				attackerDamageList[i] = attackerDamageList[i] * 9 / 10;
				if (attackerDamageList[i] < 1)
					attackerPtrList[i] = NULL; // decayed to nothingness
			}
		}

		// for vlords, change form?
		if (27 == type)
		{
			if (health < maxHealth)
			{
				if (1 == form)
					form = 0;
				else
					form = 1;
				if (controllingAvatar && controllingAvatar->BeastStat() > 9)
					form = 0; // bm pets don't change form.
				MessMonsterChangeForm messCF;
				messCF.mobID = (unsigned long)this;
				messCF.form = form;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messCF),(void *)&messCF);

				if (1 == form)  // since it's zero they wont' teleport off either.
				{
					// what direction do I want to go?
					int dirTry = 0;
					while (dirTry < 10)
					{
						++dirTry;
						int tX = cellX;
						int tY = cellY;

						int randDir = rand() % 4;

						if (0 == randDir)
							--tX;
						else if (1 == randDir)
							++tX;
						else if (2 == randDir)
							--tY;
						else if (3 == randDir)
							++tY;

						int willGo = TRUE;
						if (spawnX - tX > 2)
							willGo = FALSE;
						if (spawnY - tY > 2)
							willGo = FALSE;
						if (tX - spawnX > 2)
							willGo = FALSE;
						if	(tY - spawnY > 2)
							willGo = FALSE;

						// don't go too near the door
						if (tX < cellX && tX < 2)
							willGo = FALSE;
						if (tY < cellY && tY < 2)
							willGo = FALSE;

						// Can I go there?
						if (tX == cellX && tY == cellY)
							willGo = FALSE;
						if (!ss->CanMove(cellX, cellY, tX, tY))
							willGo = FALSE;
						// code real movement check.
						/*
						if (ss->WhatAmI() == SPACE_GROUND) // if it's the main map.
						{
						if (!((GroundMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						if (ss->WhatAmI() == SPACE_REALM) // if it's a realm.
						{
						if (!((RealmMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						if (ss->WhatAmI() == SPACE_LABYRINTH) // if it's a labyrinth
						{
						if (!((LabyrinthMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						if (ss->WhatAmI() == SPACE_DUNGEON) // if it's a dungeon
						{
						if (!((DungeonMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						*/

						if (willGo && magicEffectAmount[MONSTER_EFFECT_BIND] <= 0)
						{
							// okay, let's go!
							isMoving = TRUE;
							targetCellX = tX;
							targetCellY = tY;
							moveStartTime = timeGetTime();

							MessMobBeginMove bMove;
							bMove.mobID = (unsigned long) this;
							bMove.x = cellX;
							bMove.y = cellY;
							bMove.targetX = tX;
							bMove.targetY = tY;
							ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);

							dirTry = 100;

							// try to teleport a bat
							PortalBat(ss);

						}
					}
				}

			}

		}
		// for butterfly bosses, do fancy routine
		if ((30 == type)&& subType>1)
		{
			if (health < maxHealth)
			{
				if (1 == form)
					form = 0;
				else
					form = 1;
				// butterflies don't ACTUALLY change form, so we don't send that packet.
//				MessMonsterChangeForm messCF;
//				messCF.mobID = (unsigned long)this;
//				messCF.form = form;
//				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messCF), (void *)&messCF);
				if (controllingAvatar && controllingAvatar->BeastStat() > 9)
					form = 0; // bm pets don't change form.

				if (1 == form)
				{
					// what direction do I want to go?
					int dirTry = 0;
					while (dirTry < 10)
					{
						++dirTry;
						int tX = cellX;
						int tY = cellY;

						int randDir = rand() % 4;

						if (0 == randDir)
							--tX;
						else if (1 == randDir)
							++tX;
						else if (2 == randDir)
							--tY;
						else if (3 == randDir)
							++tY;

						int willGo = TRUE;
						if (spawnX - tX > 2)
							willGo = FALSE;
						if (spawnY - tY > 2)
							willGo = FALSE;
						if (tX - spawnX > 2)
							willGo = FALSE;
						if (tY - spawnY > 2)
							willGo = FALSE;

						// don't go too near the door
						if (tX < cellX && tX < 2)
							willGo = FALSE;
						if (tY < cellY && tY < 2)
							willGo = FALSE;

						// Can I go there?
						if (tX == cellX && tY == cellY)
							willGo = FALSE;
						if (!ss->CanMove(cellX, cellY, tX, tY))
							willGo = FALSE;
						// code real movement check.
						/*
						if (ss->WhatAmI() == SPACE_GROUND) // if it's the main map.
						{
						if (!((GroundMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						if (ss->WhatAmI() == SPACE_REALM) // if it's a realm.
						{
						if (!((RealmMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						if (ss->WhatAmI() == SPACE_LABYRINTH) // if it's a labyrinth
						{
						if (!((LabyrinthMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						if (ss->WhatAmI() == SPACE_DUNGEON) // if it's a dungeon
						{
						if (!((DungeonMap *)ss)->CanMove(cellX, cellY, tX, tY))
						willGo = FALSE;
						}
						*/

						if (willGo && magicEffectAmount[MONSTER_EFFECT_BIND] <= 0)
						{
							// okay, let's go!
							isMoving = TRUE;
							targetCellX = tX;
							targetCellY = tY;
							moveStartTime = timeGetTime();

							MessMobBeginMove bMove;
							bMove.mobID = (unsigned long)this;
							bMove.x = cellX;
							bMove.y = cellY;
							bMove.targetX = tX;
							bMove.targetY = tY;
							ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);

							dirTry = 100;

							// try to teleport a bunny. :)
							PortalBat(ss); // i KNOW it says bat...

						}
					}
				}

			}

		}

		// for dragons, check the ground for tradable items...
		if (7 == type)
		{
			Inventory *inv = ss->GetGroundInventory(cellX, cellY);
			int scaleCount = 0;
			int demonAmuletCount = 0;
			InventoryObject *iObject = (InventoryObject *) inv->objects.First();
			while (iObject)
			{
				if (INVOBJ_SIMPLE == iObject->type && 
					 !strncmp("Ancient Dragonscale", iObject->WhoAmI(), strlen("Ancient Dragonscale"))
					)
					scaleCount += iObject->amount;
				if (INVOBJ_SIMPLE == iObject->type && 
					 !strncmp("Demon Amulet", iObject->WhoAmI(), strlen("Demon Amulet"))
					)
					demonAmuletCount += iObject->amount;


				iObject = (InventoryObject *) inv->objects.Next();
			}
			 /*
			if (2 == subType)
			{
				scaleCount = scaleCount;
			}
				*/
			if (2 == subType && scaleCount > 1)
			{
				// okay, first, take my scales
				iObject = (InventoryObject *) inv->objects.First();
				while (iObject)
				{
					if (INVOBJ_SIMPLE == iObject->type && 
						 !strncmp("Ancient Dragonscale", iObject->WhoAmI(), strlen("Ancient Dragonscale"))
						)
					{
						inv->objects.Remove(iObject);
						delete iObject;
						iObject = (InventoryObject *) inv->objects.First();
					}
					else
						iObject = (InventoryObject *) inv->objects.Next();
				}

				// then drop the trade
				iObject = new InventoryObject(INVOBJ_INGOT,0,"Elatium Ingot");
				InvIngot *exIngot = (InvIngot *)iObject->extra;
				exIngot->damageVal = 9;
				exIngot->challenge = 9;
				exIngot->r = 0;
				exIngot->g = 50;
				exIngot->b = 128;

				iObject->mass = 1.0f;
				iObject->value = 75000;
				iObject->amount = scaleCount/2;
				if (iObject->amount < 1)
					iObject->amount = 1;

				inv->objects.Append(iObject);

				// and announce it
			  	sprintf(&(tempText[2]),"The dragon agrees to a trade.");
				tempText[0] = NWMESS_PLAYER_CHAT_LINE;
				tempText[1] = TEXT_COLOR_EMOTE;
				ss->SendToEveryoneNearBut(0, cellX, cellY, 
					       strlen(tempText) + 1,(void *)&tempText,1);
			}

			if (2 == subType && demonAmuletCount > 0)
			{
				// okay, first, take my amulets
				int objectsStillThere = FALSE;
				iObject = (InventoryObject *) inv->objects.First();
				while (iObject)
				{
					if (INVOBJ_SIMPLE == iObject->type && 
						 !strncmp("Demon Amulet", iObject->WhoAmI(), strlen("Demon Amulet"))
						)
					{
						inv->objects.Remove(iObject);
						delete iObject;
						iObject = (InventoryObject *) inv->objects.First();
					}
					else
					{
						objectsStillThere = TRUE;
						iObject = (InventoryObject *) inv->objects.Next();
					}
				}

				// then respond
				HandlePossessedQuest(demonAmuletCount, ss);

				if (!objectsStillThere)
				{
					MessMobDisappear messMobDisappear;
					messMobDisappear.mobID = (unsigned long) inv;
					messMobDisappear.x = cellX;
					messMobDisappear.y = cellY;
					ss->SendToEveryoneNearBut(0, cellX, cellY,
						sizeof(messMobDisappear),(void *)&messMobDisappear);
				}
			}
		}

		if (7 == type && 4 == subType) // Dragon Overlord
		{
			if (!strcmp(uniqueName,"Dragon ArchMage"))
			{
				if (health < 15000 && FALSE == archMageMode)
				{
					archMageMode = TRUE;
					// create Dragon Illusions
					for (int spawnIndex = 0; spawnIndex < 4; ++spawnIndex)
					{
						BBOSMonster *monster = new BBOSMonster(7,5, NULL);  // make Dragon Illusion
						int mx = cellX;
						int my = cellY;
//						int mx = cellX + spaceOffset[spawnIndex][0];
//						int my = cellY + spaceOffset[spawnIndex][1];
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
						mobAppear.staticMonsterFlag = FALSE;

						mobAppear.x = monster->cellX;
						mobAppear.y = monster->cellY;
						ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY, 
									 sizeof(mobAppear), &mobAppear);
					}
				}
			}
		}
	}
	
	delta = now - lastWanderlustTime;

	if (0 == lastWanderlustTime || now < lastWanderlustTime)
	{
		delta = 1000 * 50 + 1;
	}

//	if (isWandering && delta > 1000 * 50 && !isMoving && !curTarget)	
	if (delta > 1000 * 50 && !isMoving && !curTarget)	
	{
		lastWanderlustTime = now - (rand() % 10000);

		int shouldGo = isWandering;
		if (!shouldGo)
		{
			if (SPACE_GROUND == ss->WhatAmI())
			{
				BBOSMob *tMob = NULL;
				MapListState state = ss->mobList->GetState();
				tMob = ss->mobList->GetFirst(cellX, cellY);
				while (tMob)
				{
					if (SMOB_TOWER == tMob->WhatAmI())
						shouldGo = TRUE;
					tMob = ss->mobList->GetNext();
				}
				ss->mobList->SetState(state);

			}
		}

		if (26 == type && !shouldGo)  // bat wander check
		{
			// find out how many bats are in this square
			int numBats = 0;
			MapListState state = ss->mobList->GetState();
			BBOSMob *tMob = ss->mobList->GetFirst(cellX, cellY);
			while (tMob)
			{
				if (SMOB_MONSTER == tMob->WhatAmI() && 26 == ((BBOSMonster *)tMob)->type)
					++numBats;
				tMob = ss->mobList->GetNext();
			}
			ss->mobList->SetState(state);

			if (numBats < 2 || numBats > 6) // group too small or too big
				shouldGo = TRUE;            // i will wander
			else // update my spawn square to here so i can't be separated from my group (evil grin)
			{ 
				spawnX = cellX; 
				spawnY = cellY; // we shall never be apart.
			}
		}
		if (29 == type && !shouldGo) // the bunnies also group up now
		{
			// find out how many bunnies are in this square
			int numBunnies = 0;
			MapListState state = ss->mobList->GetState();
			BBOSMob *tMob = ss->mobList->GetFirst(cellX, cellY);
			while (tMob)
			{
				if (SMOB_MONSTER == tMob->WhatAmI() && 29 == ((BBOSMonster *)tMob)->type)
					++numBunnies;
				tMob = ss->mobList->GetNext();
			}
			ss->mobList->SetState(state);

			if (numBunnies < 2 || numBunnies > 6)
				shouldGo = TRUE;
			else // update my spawn square to here so i can't be separated from my group (evil grin)
			{
				spawnX = cellX;
				spawnY = cellY; // we shall never be apart.
			}
		}

		if (shouldGo)
		{
			// what direction do I want to go?
			int tX = cellX;
			int tY = cellY;

			int randDir = rand() % 4;

			if (0 == randDir)
				--tX;
			else if (1 == randDir)
				++tX;
			else if (2 == randDir)
				--tY;
			else if (3 == randDir)
				++tY;

			int willGo = TRUE;
			// don't go too near the door
			if (tX < cellX && tX < 2)
				willGo = FALSE;
			if (tY < cellY && tY < 2)
				willGo = FALSE;

			// Can I go there?
			if (tX == cellX && tY == cellY)
				willGo = FALSE;
			if (!ss->CanMove(cellX, cellY, tX, tY))  
				willGo = FALSE;
			// code real movement check.
			/*
						if (ss->WhatAmI() == SPACE_GROUND) // if it's the main map.
			{
				if (!((GroundMap *)ss)->CanMove(cellX, cellY, tX, tY))
					willGo = FALSE;
			}
			if (ss->WhatAmI() == SPACE_REALM) // if it's a realm.
			{
				if (!((RealmMap *)ss)->CanMove(cellX, cellY, tX, tY))
					willGo = FALSE;
			}
			if (ss->WhatAmI() == SPACE_LABYRINTH) // if it's a labyrinth
			{
				if (!((LabyrinthMap *)ss)->CanMove(cellX, cellY, tX, tY))
					willGo = FALSE;
			}
			if (ss->WhatAmI() == SPACE_DUNGEON) // if it's a dungeon
			{
				if (!((DungeonMap *)ss)->CanMove(cellX, cellY, tX, tY))
					willGo = FALSE;
			}
			*/

			if (!controllingAvatar && willGo && magicEffectAmount[MONSTER_EFFECT_BIND] <= 0)
			{
				// okay, let's go!
				isMoving = TRUE;
				targetCellX = tX;
				targetCellY = tY;
				moveStartTime = timeGetTime();

				MessMobBeginMove bMove;
				bMove.mobID = (unsigned long) this;
				bMove.x = cellX;
				bMove.y = cellY;
				bMove.targetX = tX;
				bMove.targetY = tY;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);
			}
		}
	}

	if (thiefMode != THIEF_NOT_THIEF)
	{
		// process effects
		delta = now - lastThiefTime;

		if (now < lastThiefTime)
		{
			delta = 1000 * 600 + 1;
		}

		if (0 == lastThiefTime)
		{
			lastThiefTime = now;
			delta = 0;
		}

		if (THIEF_WAITING == thiefMode)
		{
			if (delta > 1000 * 600)	
			{
				lastThiefTime = now;

				// find some poor sucker
				BBOSAvatar *candAv = NULL;
				BBOSAvatar *SearchAvatar= (BBOSAvatar *)ss->avatars->First();
				while (SearchAvatar && !candAv)
				{
					if (3 == rand() % 10)
					{
						candAv = SearchAvatar;
					}

					SearchAvatar = (BBOSAvatar *)ss->avatars->Next();
					if (!SearchAvatar)
						SearchAvatar = (BBOSAvatar *)ss->avatars->First();
				}

				if (candAv && magicEffectAmount[MONSTER_EFFECT_BIND] <= 0)
				{
					// teleport next to her
					MessMobDisappear messMobDisappear;
					messMobDisappear.mobID = (unsigned long) this;
					messMobDisappear.x = cellX;
					messMobDisappear.y = cellY;
					ss->SendToEveryoneNearBut(0, cellX, cellY,
						sizeof(messMobDisappear),(void *)&messMobDisappear,6);

					cellX = candAv->cellX;
					cellY = candAv->cellY;
					ss->mobList->Move(this);

					// send arrival message
					MessMobAppear mAppear;
					mAppear.mobID = (unsigned long) this;
					mAppear.x = cellX;
					mAppear.y = cellY;
					mAppear.monsterType = type;
					mAppear.subType = subType;
					mAppear.type = WhatAmI();

					ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mAppear), &mAppear,6);

					// laugh
					MessGenericEffect messGE;
					messGE.avatarID = -1;
					messGE.mobID    = (long) this;
					messGE.x        = cellX;
					messGE.y        = cellY;
					messGE.r        = 255;
					messGE.g        = 0;
					messGE.b        = 0;
					messGE.type     = 1;  // type of particles
					messGE.timeLen  = 1; // in seconds
					ss->SendToEveryoneNearBut(0, cellX, cellY,
										 sizeof(messGE),(void *)&messGE);

					// announce the stealing
			  		sprintf(&(tempText[2]),"The Thieving Spirit has arrived!");
					tempText[0] = NWMESS_PLAYER_CHAT_LINE;
					tempText[1] = TEXT_COLOR_SHOUT;
					ss->SendToEveryoneNearBut(0, cellX, cellY, 
						       strlen(tempText) + 1,(void *)&tempText);
					// set to running
					thiefMode = THIEF_RUNNING;
				}
			}
		}
		else if (THIEF_RUNNING == thiefMode)
		{
			if (delta > 1000 * 30)	
			{
				lastThiefTime = now;

				// find some poor sucker
				BBOSAvatar *candAv = NULL;
				BBOSAvatar *SearchAvatar = (BBOSAvatar *)ss->avatars->First();
				int avTries = 0;
				while (SearchAvatar && !candAv && avTries < 200)
				{
					if (SearchAvatar->cellX == cellX && SearchAvatar->cellY == cellY)
					{
						candAv = SearchAvatar;
					}

					++avTries;

					SearchAvatar = (BBOSAvatar *) ss->avatars->Next();
					if (!SearchAvatar)
						SearchAvatar = (BBOSAvatar *) ss->avatars->First();
				}

				if (candAv)
				{
					// steal some stuff
					Inventory *inv = (candAv->charInfoArray[candAv->curCharacterIndex].wield);
					InventoryObject * goodie = NULL;
					if (candAv->EvilBotter) // if it's an evil botter
					{
						// steal EVERYTHING!
						// first everything wielded, including swords!

						InventoryObject *iObject = (InventoryObject *)inv->objects.First(); // first object
						while (iObject)
						{
							goodie = iObject;
							inv->objects.Remove(iObject);
							inventory->objects.Append(iObject);
							iObject = (InventoryObject *)inv->objects.Next();
						}
						// next do workbench
						inv = (candAv->charInfoArray[candAv->curCharacterIndex].workbench);
						iObject = (InventoryObject *)inv->objects.First(); // first object
						while (iObject)
						{
							goodie = iObject;
							inv->objects.Remove(iObject);
							inventory->objects.Append(iObject);
							iObject = (InventoryObject *)inv->objects.Next();
						}
						// finally, general inventory
						inv = (candAv->charInfoArray[candAv->curCharacterIndex].inventory);
						iObject = (InventoryObject *)inv->objects.First(); // first object
						while (iObject)
						{
							goodie = iObject;
							inv->objects.Remove(iObject);
							inventory->objects.Append(iObject);
							iObject = (InventoryObject *)inv->objects.Next();
						}
						// announce the stealing 
						sprintf(&(tempText[2]), "The Thieving Spirit has robbed %s blind while she was afk crafting!",
							candAv->charInfoArray[candAv->curCharacterIndex].name);
						tempText[0] = NWMESS_PLAYER_CHAT_LINE;
						tempText[1] = TEXT_COLOR_SHOUT;
						// tell EVERYBODY about it so they know to go to realm of the dead and kill the spirit.
						bboServer->lserver->SendMsg(strlen(tempText) + 1, (void *)&tempText, 0, NULL);
						if (!goodie && (candAv->SameCellCount > 3600)) // if there was nothing to steal, and he's still trying to skill for 4 hours.
						{
							// then strip the skills too!
							inv = (candAv->charInfoArray[candAv->curCharacterIndex].skills);
							iObject = (InventoryObject *)inv->objects.First(); // first object
							while (iObject)
							{
								inv->objects.Remove(iObject);    // fuck you, cheater!
								iObject = (InventoryObject *)inv->objects.Next();
							}
							// and boot them offline
							candAv->kickOff = true;
						}

					}
					else
					{
						int tries = 0;

						// first, try to steal totems
						InventoryObject *iObject = (InventoryObject *)inv->objects.First();
						int totemCount = 0;
						while (iObject && !goodie)
						{
							if (INVOBJ_TOTEM == iObject->type)
							{
								if (3 == rand() % 10)
								{
									goodie = iObject;
								}
								++totemCount;
							}

							iObject = (InventoryObject *)inv->objects.Next();
							if (!iObject && totemCount > 0)
								iObject = (InventoryObject *)inv->objects.First();
						}

						// no?  Then just steal any old thing
						tries = 0;
						iObject = (InventoryObject *)inv->objects.First();
						while (iObject && !goodie && tries < 100)
						{
							if ((INVOBJ_BLADE != iObject->type) && (INVOBJ_STABLED_PET != iObject->type) && 3 == rand() % 5)
							{
								goodie = iObject;
							}

							++tries;

							iObject = (InventoryObject *)inv->objects.Next();
							if (!iObject)
								iObject = (InventoryObject *)inv->objects.First();
						}


						if (!goodie)
						{
							// no?  Then steal from general inventory!
							inv = (candAv->charInfoArray[candAv->curCharacterIndex].inventory);

							tries = 0;
							iObject = (InventoryObject *)inv->objects.First();
							while (iObject && !goodie && tries < 100)
							{
								if ((INVOBJ_BLADE != iObject->type) && (INVOBJ_STABLED_PET != iObject->type) && (3 == rand() % 35))
								{
									goodie = iObject;
								}

								++tries;

								iObject = (InventoryObject *)inv->objects.Next();
								if (!iObject)
									iObject = (InventoryObject *)inv->objects.First();
							}
						}

						if (goodie && !candAv->accountType)
						{
							// get the goodie
							inv->objects.Remove(goodie);
							inventory->objects.Append(goodie);

							// announce the stealing
							sprintf(&(tempText[2]), "The Thieving Spirit has stolen %s's %s!",
								candAv->charInfoArray[candAv->curCharacterIndex].name,
								goodie->WhoAmI());
							tempText[0] = NWMESS_PLAYER_CHAT_LINE;
							tempText[1] = TEXT_COLOR_SHOUT;
							ss->SendToEveryoneNearBut(0, cellX, cellY,
								strlen(tempText) + 1, (void *)&tempText);
						}
					}
					// laugh
					MessGenericEffect messGE;
					messGE.avatarID = -1;
					messGE.mobID    = (long) this;
					messGE.x        = cellX;
					messGE.y        = cellY;
					messGE.r        = 255;
					messGE.g        = 0;
					messGE.b        = 0;
					messGE.type     = 1;  // type of particles
					messGE.timeLen  = 1; // in seconds
					ss->SendToEveryoneNearBut(0, cellX, cellY,
										 sizeof(messGE),(void *)&messGE);
				}

				// move 1 squares away (to a legal spot)
				int tempX, tempY;
				do
				{
					tempX = cellX + (rand() % 3) - 1;
					tempY = cellY + (rand() % 3) - 1;
				} while (!ss->CanMove(tempX,tempY,tempX,tempY) || 
					      (tempX == cellX && tempY == cellY)
						  );


				if (magicEffectAmount[MONSTER_EFFECT_BIND] > 0)
				{
					tempX = cellX;
					tempY = cellY;
				}

				MessMobDisappear messMobDisappear;
				messMobDisappear.mobID = (unsigned long) this;
				messMobDisappear.x = cellX;
				messMobDisappear.y = cellY;
				ss->SendToEveryoneNearBut(0, cellX, cellY,
					sizeof(messMobDisappear),(void *)&messMobDisappear,6);

				cellX = tempX;
				cellY = tempY;
				ss->mobList->Move(this);

				// send arrival message
				MessMobAppear mAppear;
				mAppear.mobID = (unsigned long) this;
				mAppear.x = cellX;
				mAppear.y = cellY;
				mAppear.monsterType = type;
				mAppear.subType = subType;
				mAppear.type = WhatAmI();

				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mAppear), &mAppear,6);


				// set to waiting
				thiefMode = THIEF_WAITING;

			}
		}
	}

}


//******************************************************************
void BBOSMonster::ReactToAdjacentPlayer(BBOSAvatar *curAvatar, SharedSpace *ss)
{
	char tempText[1024];

	// mosnter comes to attack!
	if ((MONSTER_PLACE_SPIRITS & monsterData[type][subType].placementFlags ||
		  MONSTER_PLACE_DRAGONS & monsterData[type][subType].placementFlags ||
		  26 == type // bats
		|| 29 == type //bunnies. :)
		|| 30 == type // butterflies. :)
		|| 31 == type // unicorns. :)
		)
		 && magicEffectAmount[MONSTER_EFFECT_BIND] <= 0)
	{
	
		int range = 2;
		if (SPACE_DUNGEON == ss->WhatAmI() && 
			 (((DungeonMap *) ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY))
			range = 1;
		if (type == 31)
		{
			range = 0; // unicorms run away when a player steps next to them
		}
		if(curTarget == curAvatar || NULL == curTarget)
		{
			int go = TRUE;
			if (type == 16 && subType == 1)
				go = FALSE;
			if (7 == type && 5 != subType)
				go = FALSE;
				            // exclude Dokk and normal dragons, accept for Dragon Illusions
			if (go)
			{
				int tcX = curAvatar->cellX;
				int tcY = curAvatar->cellY;
				int tc2X = curAvatar->cellX; // save original target
				int tc2Y = curAvatar->cellY;
				if (range == 0)
				{
					spawnX = cellX; // update spawn to current coordinates for always flee monsters, so they will flee the right direction.
					spawnY = cellY;
				}

				if (spawnX - tcX > range)
					tcX = cellX + 1;
				if (spawnY - tcY > range)
					tcY = cellY + 1;
				if (tc2X - spawnX > range) // check original target again instead of newly changed target
					tcX = cellX - 1;
				if (tc2Y - spawnY > range)
					tcY = cellY - 1;
				int fail = FALSE;
				if (SPACE_DUNGEON == ss->WhatAmI() &&                                      // if in a dungeon
					 !(MONSTER_PLACE_SPIRITS & monsterData[type][subType].placementFlags)) // spirits ignore walls in dugeons.
				{
					if (!ss->CanMove(cellX, cellY, tcX, tcY))
						fail = TRUE;
				}
				else if (SPACE_DUNGEON != ss->WhatAmI())			// if NOT in a dungeon
				{												// then all monsters obey can movechecks when reacting to players.
					if (!ss->CanMove(cellX, cellY, tcX, tcY))	// they will no longer cut through lab corners to flee.
						fail = TRUE;                    
				}
				// finally, check if a diagonal move is attempted
				if ((tcX != cellX) && (tcY != cellY)) // if both coords are different
				{
					if (type != 31) // for now only unicorns can react to players at a diagonal.
					{
						fail = TRUE;
					}
				}
				if (!fail)
				{
					targetCellX = tcX;
					targetCellY = tcY;

					isMoving = TRUE;
					moveStartTime = timeGetTime() - 500;

					MessMobBeginMove bMove;
					bMove.mobID = (unsigned long) this;
					bMove.x = cellX;
					bMove.y = cellY;
					bMove.targetX = targetCellX;
					bMove.targetY = targetCellY;
					ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(bMove), &bMove);
					if (SpecialAI && (type == 31)) // unicorn with ACTIVE ai
					{
							bboServer->UnicornLocations[subType][0] = tcX; 
							bboServer->UnicornLocations[subType][1] = tcY;
					}
				}
			}
		}
	}

	if (16 == type && 1 == subType) // Dokk
	{
		std::vector<TagID> targetReceiptList;
		targetReceiptList.clear();
		targetReceiptList.push_back(curAvatar->socketIndex);

		sprintf(&(tempText[1]),"Dokk tells you, Another worthless female comes to hinder my plans...  Nothing can defeat me, Human!  I will crush you!");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

	}

	if (7 == type && 4 == subType) // Dragon Overlord
	{
		std::vector<TagID> targetReceiptList;
		targetReceiptList.clear();
		targetReceiptList.push_back(curAvatar->socketIndex);

		
		if (!strcmp(uniqueName,"Dragon Chaplain"))
			sprintf(&(tempText[1]),"The Chaplain tells you, All of us are children of the Great Spirits, but still humans disgust me.");
		else if (!strcmp(uniqueName,"Dragon ArchMage"))
			sprintf(&(tempText[1]),"The ArchMage tells you, We have no time for foolish, mundane humans.");
		else
			sprintf(&(tempText[1]),"The Overlord tells you, Why do you approach, Human?  What are your intentions?");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->lserver->SendMsg(strlen(tempText) + 1,(void *)&tempText, 0, &targetReceiptList);

	}

}

//******************************************************************
char *BBOSMonster::Name(void)
{
	if (uniqueName[0])
	{
		return uniqueName;
	}
	else
	{
		return monsterData[type][subType].name;
	}
}

//******************************************************************
void BBOSMonster::AnnounceMyselfCustom(SharedSpace *ss)
{
	MessMobAppearCustom mAppear;
	mAppear.mobID = (unsigned long) this;
	mAppear.x = cellX;
	mAppear.y = cellY;
	mAppear.monsterType = type;
	mAppear.subType = subType;
	CopyStringSafely(Name(), 32, mAppear.name, 32);
	mAppear.a = a;
	mAppear.r = r;
	mAppear.g = g;
	mAppear.b = b;
	mAppear.sizeCoeff = sizeCoeff;

	if(SPACE_DUNGEON == ss->WhatAmI())
	{
		mAppear.staticMonsterFlag = FALSE;
		if (!isWandering && !isPossessed)
			mAppear.staticMonsterFlag = TRUE;
	}
	mAppear.type = WhatAmI();

	// send arrival message
	ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mAppear), &mAppear);
}


//******************************************************************
void BBOSMonster::RecordDamageForLootDist(int damageDone, BBOSAvatar *curAvatar)
{
	int recIndex = -1;
	// find the existing record for this attacker
	for (int i = 0; i < 10; ++i)
	{
		if (attackerPtrList[i] == curAvatar &&
			attackerLifeList[i] <=
			curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime)
		{
			recIndex = i;
			break;
		}	//exit loop so first attacker is first on the list
	}
	// if found
	if (recIndex > -1)
	{
		// add damage to record, and we're done
		attackerLifeList[recIndex] =
			      curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime;
		attackerDamageList[recIndex] += damageDone;
		return;
	}
	else
	{
		// find an empty record to put this attacker into
		for (int i = 0; i < 10; ++i)
		{
			if (NULL == attackerPtrList[i])
				recIndex = i;
		}
		// if found
		if (recIndex > -1)
		{
			// change record, set initial damage
			attackerPtrList[recIndex] = curAvatar;
			attackerLifeList[recIndex] =
				      curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime;
			attackerDamageList[recIndex] = damageDone;
			return;
		}
		else
		{
			// find a record with less damage than damageDone
			for (int i = 0; i < 10; ++i)
			{
				if (attackerDamageList[i] < damageDone)
					recIndex = i;
			}
			// if found
			if (recIndex > -1)
			{
				// take over the record
				attackerPtrList[recIndex] = curAvatar;
				attackerLifeList[recIndex] =
					      curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime;
				attackerDamageList[recIndex] = damageDone;
			}
		}
	}

}


//******************************************************************
void BBOSMonster::ClearDamageForLootDist(BBOSAvatar *curAvatar)
{
	int recIndex = -1;
	// find the existing record for this attacker
	for (int i = 0; i < 10; ++i)
	{
		if (attackerPtrList[i] == curAvatar && 
			 attackerLifeList[i] <= 
			      curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime)
			recIndex = i;
	}
	// if found
	if (recIndex > -1)
	{
		attackerDamageList[recIndex] = 0;
	}

}


//******************************************************************
BBOSAvatar *BBOSMonster::MostDamagingPlayer(float degradeCoeff)
{
	int recIndex = -1;
	int mostDam = 0;
	// find the valid record with the most damage
	for (int i = 0; i < 10; ++i)
	{
		if (attackerPtrList[i] && 
			 attackerDamageList[i] > mostDam)
		{
			SharedSpace *sp;
			BBOSAvatar * foundAvatar = 
				bboServer->FindAvatar(attackerPtrList[i], &sp);
			if (foundAvatar && 
				 foundAvatar->charInfoArray[foundAvatar->curCharacterIndex].lifeTime >=
				      attackerLifeList[i])
			{
				recIndex = i;
				mostDam = attackerDamageList[i];
			}
		}
	}
	// if found
	if (recIndex > -1)
	{
		float f = (float) attackerDamageList[recIndex];
		f *= degradeCoeff;
		if (f < 1)
			f = 1;
		attackerDamageList[recIndex] = (long) f;

		return attackerPtrList[recIndex];
	}

	return NULL;
}


//******************************************************************
void BBOSMonster::HandleQuestDeath(void)
{
	for (int i = 0; i < 10; ++i)
	{
		if (attackerPtrList[i])
		{
			SharedSpace *sp;
			BBOSAvatar * foundAvatar = 
				bboServer->FindAvatar(attackerPtrList[i], &sp);
			if (foundAvatar)
				foundAvatar->QuestMonsterKill(sp, this);
		}
	}
}


//******************************************************************
int BBOSMonster::MonsterMagicEffect(int type, float timeDelta, float amount)
{
	if ((type != MONSTER_EFFECT_RESIST_LOWER) && (type != MONSTER_EFFECT_MORE_LOOT)) /// greens and blacks ignore dust resistance
	{
		float loweredresistance = magicResistance - (magicEffectAmount[MONSTER_EFFECT_RESIST_LOWER] / 200.0f);
        if (type !=MONSTER_EFFECT_TYPE_WHITE) // not blind
		MagicEffect(type,
			timeGetTime() + timeDelta * (1 - Bracket(
				loweredresistance, 0.0, 1.0)),
			amount * (1 - Bracket(loweredresistance, 0.0, 1.0)));
		else // unfair to double penalize blind.
		MagicEffect(type,
			timeGetTime() + timeDelta * (1 - Bracket(
				loweredresistance, 0.0, 1.0)),
			amount); // full amount goes through.
		if (loweredresistance >= 1.0f)
			return 1;
	}
	else // resist_lower ignores magic resistance, or it would be pointless, and greens is weak enough that it's nto fair to require more dusts AND deal with resist.
		MagicEffect(type,
			timeGetTime() + timeDelta ,
			amount);
	return 0;
}


//******************************************************************
void BBOSMonster::HandlePossessedQuest(int amuletsGiven, SharedSpace *ss)
{

	char tempText[1024];

	float totalPlayerPower = 0;
	//   finds every player near
	BBOSAvatar *chosenAvatar = NULL;
	BBOSMob *curMob = (BBOSMob *) ss->avatars->First();
	while (curMob)
	{
		if ((curMob->WhatAmI()==SMOB_AVATAR) && (abs(curMob->cellX - cellX) < 3) && (abs(curMob->cellY - cellY) < 3)) 
		{
			BBOSAvatar *curAvatar = (BBOSAvatar *) curMob;

			// remember the name of someone in the same square as me
			if (curMob->cellX == cellX && curMob->cellY == cellY && !chosenAvatar)
				chosenAvatar = curAvatar;

			//	totals up their power
			//		power = dodge skill + age + sword damage + sword to-hit + sword magic
			totalPlayerPower += 
				     curAvatar->charInfoArray[curAvatar->curCharacterIndex].lifeTime / 20;

			totalPlayerPower += 
				curAvatar->GetDodgeLevel();

			totalPlayerPower += curAvatar->BestSwordRating();
		}

		curMob = (BBOSMob *) ss->avatars->Next();
	}
	if (!chosenAvatar)
		return;

	int freeSlot = -1;
	for (int i = 0; i < QUEST_SLOTS; ++i)
	{
		if (-1 == chosenAvatar->charInfoArray[chosenAvatar->curCharacterIndex].
			              quests[i].completeVal)
		{
			freeSlot = i;
			i = QUEST_SLOTS;
		}
	}

	if (-1 == freeSlot)
	{
		sprintf(&(tempText[1]),"The Dragon takes the amulets and says,");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

		sprintf(&(tempText[1]),"%s, you are too burdened with tasks already to",
			       chosenAvatar->charInfoArray[chosenAvatar->curCharacterIndex].name);
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

		sprintf(&(tempText[1]),"accept what I have to tell.  I'll keep your amulets.");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

		sprintf(&(tempText[1]),"Let this be a lesson to you, puny Human.");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);
		return;
	}

	//	randomly picks a dungeon
	SharedSpace *tempss = (SharedSpace *) bboServer->spaceList->First();
	SharedSpace *dungeonPicked = NULL;
	while (!dungeonPicked)
	{
		if (SPACE_DUNGEON == tempss->WhatAmI() && 0 == ((DungeonMap *) tempss)->specialFlags )
		{
			if (36 == rand() % 100)
			{
				DungeonMap *dm = (DungeonMap *) tempss;
//				if (!IsCompletelySame(dm->masterName, closePlayerName))
				dungeonPicked = tempss;
			}
		}
		tempss = (SharedSpace *) bboServer->spaceList->Next();
		if (!tempss)
			tempss = (SharedSpace *) bboServer->spaceList->First();
	}

	//	creates a Possessed monster in the back of the dungeon
	int mPosX = rand() % (((DungeonMap *)dungeonPicked)->width/2);
	int mPosY = rand() % (((DungeonMap *)dungeonPicked)->height/2);

	Quest *quest = &chosenAvatar->charInfoArray[chosenAvatar->curCharacterIndex].
		                  quests[freeSlot];
	quest->EmptyOut();
	quest->completeVal = 0;  // active, but not complete
	quest->timeLeft.SetToNow();
	quest->timeLeft.AddMinutes(60*24); // add one day

	quest->questSource = MAGIC_MAX;

	// now add questParts
	QuestPart *qp = new QuestPart(QUEST_PART_VERB, "VERB");
	quest->parts.Append(qp);
	qp->type = QUEST_VERB_KILL;

	QuestPart *qt = new QuestPart(QUEST_PART_TARGET, "TARGET");
	quest->parts.Append(qt);

	qt->type = QUEST_TARGET_LOCATION;
	
	qt->monsterType    = 1 + rand() % 6; // means a possessed
	int totalPlayerPower2 = (int)totalPlayerPower;
	if (totalPlayerPower2 < 1) // if it overflowed
		totalPlayerPower2 = 2147483647;
	qt->monsterSubType = totalPlayerPower2;
	qt->mapType = SPACE_DUNGEON;
	qt->x = mPosX;
	qt->y = mPosY;
	qt->range = amuletsGiven;

	DungeonMap *dm = (DungeonMap *)dungeonPicked;
	qt->mapSubType = GetCRCForString(dm->name);
//	sprintf(tempText2, dm->name);

	//tell the assembled about it
	if (amuletsGiven > 1)
	{
		sprintf(&(tempText[1]),"The Dragon takes the amulets and tells you,");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

		sprintf(&(tempText[1]),"These amulets bring to me a vision of a foul Possessed");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);
	}
	else
	{
		sprintf(&(tempText[1]),"The Dragon takes the amulet and tells you,");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

		sprintf(&(tempText[1]),"This amulet brings to me a vision of a foul Possessed");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);
	}

//	if (rand() % 2)
	if (TRUE)
	{
		sprintf(&(tempText[1]),"creature in the %s.", ((DungeonMap *)dungeonPicked)->name);
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);
	}
	else
	{
		int rX = ((DungeonMap *)dungeonPicked)->enterX + (rand() % 13) - 6;
		if (rX < 0)
			rX = 0;
		if (rX > 255)
			rX = 255;
		int rY = ((DungeonMap *)dungeonPicked)->enterY + (rand() % 13) - 6;
		if (rY < 0)
			rY = 0;
		if (rY > 255)
			rY = 255;

		sprintf(&(tempText[1]),"creature in a dungeon near %dN %dE.", 256-rY, 256-rX);
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);
	}

	sprintf(&(tempText[1]),"Be careful.  It is a terrible minion of the Demon King.");
	tempText[0] = NWMESS_PLAYER_CHAT_LINE;
	ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

	sprintf(&(tempText[1]),"But it must be destroyed, if you want your world to continue.");
	tempText[0] = NWMESS_PLAYER_CHAT_LINE;
	ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

	sprintf(&(tempText[1]),"Good luck, %s!", 
		  chosenAvatar->charInfoArray[chosenAvatar->curCharacterIndex].name);
	tempText[0] = NWMESS_PLAYER_CHAT_LINE;
	ss->SendToEveryoneNearBut(0, cellX, cellY, strlen(tempText) + 1,(void *)&tempText, 3);

}

//******************************************************************
void BBOSMonster::AddPossessedLoot(int count)
{

//	char tempText[1024];

	InventoryObject *iObject;
	InvStaff *staffExtra;
	InvIngredient *exIn;
	InvTotem *totemExtra;

	//	gives it random stuff according to its power
	for (int i = 0; i < count; ++i)
	{
		int goodie = rand() % 10;
		switch(goodie)
		{
		case 0:
		case 6:
		case 7:
			iObject = new InventoryObject(INVOBJ_STAFF,0,"Unnamed Staff");
			staffExtra = (InvStaff *)iObject->extra;
			staffExtra->type     = 0;
			staffExtra->quality  = 3;

			iObject->mass = 0.0f;
			iObject->value = 500 * (3 + 1) * (3 + 1);
			iObject->amount = 1;
			UpdateStaff(iObject, 0);
			inventory->objects.Append(iObject);
			break;

		case 8:
		case 9:
			iObject = new InventoryObject(INVOBJ_TOTEM,0,"Unnamed Totem");
			totemExtra = (InvTotem *)iObject->extra;
			totemExtra->type     = 0;
			totemExtra->quality  = 18; // undead

			iObject->mass = 0.0f;
			iObject->value = totemExtra->quality * totemExtra->quality * 14 + 1;
			if (totemExtra->quality > 12)
				iObject->value = totemExtra->quality * totemExtra->quality * 14 + 1 + (totemExtra->quality-12) * 1600;
			iObject->amount = 1;
			UpdateTotem(iObject);
			inventory->objects.Append(iObject);
			break;
		case 1:
			iObject = new InventoryObject(INVOBJ_INGREDIENT,0,"Glowing Green Dust");
			exIn = (InvIngredient *)iObject->extra;
			exIn->type     = INGR_GREEN_DUST;
			exIn->quality  = 1;

			iObject->mass = 0.0f;
			iObject->value = 1000;
			iObject->amount = 1;
			inventory->objects.Append(iObject);
			break;
		case 2: // blacks are back in with new effect.
			iObject = new InventoryObject(INVOBJ_INGREDIENT,0,"Glowing Black Dust");
			exIn = (InvIngredient *)iObject->extra;
			exIn->type     = INGR_BLACK_DUST;
			exIn->quality  = 1;

			iObject->mass = 0.0f;
			iObject->value = 1000;
			iObject->amount = 1;
			inventory->objects.Append(iObject);
			break;
  
		case 3:
			iObject = new InventoryObject(INVOBJ_INGREDIENT,0,"Glowing Red Dust");
			exIn = (InvIngredient *)iObject->extra;
			exIn->type     = INGR_RED_DUST;
			exIn->quality  = 1;

			iObject->mass = 0.0f;
			iObject->value = 1000;
			iObject->amount = 1;
			inventory->objects.Append(iObject);
			break;

		case 4:
			iObject = new InventoryObject(INVOBJ_INGREDIENT,0,"Glowing White Dust");
			exIn = (InvIngredient *)iObject->extra;
			exIn->type     = INGR_WHITE_DUST;
			exIn->quality  = 1;

			iObject->mass = 0.0f;
			iObject->value = 1000;
			iObject->amount = 1;
			inventory->objects.Append(iObject);
			break;
		case 5:
			iObject = new InventoryObject(INVOBJ_INGREDIENT,0,"Glowing Blue Dust");
			exIn = (InvIngredient *)iObject->extra;
			exIn->type     = INGR_BLUE_DUST;
			exIn->quality  = 1;

			iObject->mass = 0.0f;
			iObject->value = 1000;
			iObject->amount = 1;
			inventory->objects.Append(iObject);
			break;

		}
	}

}


//******************************************************************
void BBOSMonster::VLordAttack(SharedSpace *ss) // and also butterfiles
{
	DWORD now = timeGetTime();
	std::vector<TagID> tempReceiptList;
	char tempText[1024];
	MessInfoText infoText;

	MessMonsterSpecialAttack messAA;
	messAA.mobID = (long)this;
	messAA.type = 1;
	ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);

	BBOSMob *curMob = (BBOSMob *)ss->avatars->First();
	while (curMob)
	{
		if ((curMob->cellX == cellX) && (curMob->cellY == cellY) && curMob->WhatAmI() == SMOB_AVATAR)
		{
			BBOSAvatar *curAvatar = (BBOSAvatar *)curMob;

			InvSkill *skillInfo = NULL;
			int dodgeMod = 0;
			InventoryObject *io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.First();
			while (io)
			{
				if (!strcmp("Dodging", io->WhoAmI()))
				{
					skillInfo = (InvSkill *)io->extra;
					dodgeMod = skillInfo->skillLevel - 1;
					int addAmount = toHit - dodgeMod;
					if (addAmount < 0)
						addAmount = 0;
					if (addAmount > 10)
						addAmount = 0;
					skillInfo->skillPoints += addAmount * bboServer->combat_exp_multiplier;

					//					if (abs(dodgeMod - toHit) < 3)
					//						skillInfo->skillPoints += 1;
				}
				if (!strcmp("Pet Energy", io->WhoAmI()) && (curAvatar->followMode))  // beastmaster with pet energy
				{
					skillInfo = (InvSkill *)io->extra;  // get info
					dodgeMod = skillInfo->skillLevel - 1;        /// pet energy protects you as well if your pet is near
					// but this doesn't raise your pet's dodge.
				}
				io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.Next();
			}

			dodgeMod += curAvatar->totemEffects.effect[TOTEM_QUICKNESS];

			//    1. chance = 2d20 + ToHit - player.Physical
			int cVal = toHit - dodgeMod;

			if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
				cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

			int chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

			if (cVal + 40 <= 20) // if monster can't EVER hit
			{
				if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
					chance = 30;  // Hit!
			}

			//    2. if chance > 20, hit was successful
			if (chance > 20)
			{
				//    3. damage = damage
				int dDone = damageDone - curAvatar->totemEffects.effect[TOTEM_TOUGHNESS] / 3;
				if (dDone < 0)
					dDone = 0;

				if (MONSTER_PLACE_SPIRITS &
					monsterData[type][subType].placementFlags)
				{
					float fDone = (float)dDone *
						(1.0f - (curAvatar->totemEffects.effect[TOTEM_PROT_SPIRITS] / 30.0f));
					dDone = (int)fDone;
				}

				dDone += (int)rnd(0, dDone);

				if (curAvatar->totemEffects.effect[TOTEM_LIFESTEAL] > 0)
				{
					int suck = dDone * curAvatar->totemEffects.effect[TOTEM_LIFESTEAL] / 60;
					dDone += suck;
					health += suck;
				}
				if (curAvatar->totemEffects.effect[TOTEM_FOCUS] > 0 //if character has a focus totem
					&& dDone >= curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax) // and damage greater than or equal to max health
					dDone = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax - 1; // lower damage to max health-1 to not oneshot from full health

				if (ACCOUNT_TYPE_ADMIN != curAvatar->accountType)
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= dDone;

				MessAvatarHealth messHealth;
				messHealth.avatarID = curAvatar->socketIndex;
				messHealth.health = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
				messHealth.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messHealth), &messHealth, 2);

				if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health <= 0)
				{
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].health = 0;
					curAvatar->isDead = TRUE;

					tempReceiptList.clear();
					tempReceiptList.push_back(curAvatar->socketIndex);
					if (SPACE_GROUND == ss->WhatAmI())
					{
						sprintf(tempText, "The %s defeats you at %dN %dE.", Name(),
							256 - curAvatar->cellY, 256 - curAvatar->cellX);
					}
					else if (SPACE_REALM == ss->WhatAmI())
					{
						sprintf(tempText, "The %s defeats you in a Mystic Realm at %dN %dE.",
							Name(),
							64 - curAvatar->cellY, 64 - curAvatar->cellX);
					}
					else if (SPACE_LABYRINTH == ss->WhatAmI())
					{
						sprintf(tempText, "The %s defeats you in the Labyrinth at %dN %dE.",
							Name(),
							64 - curAvatar->cellY, 64 - curAvatar->cellX);
					}
					else
					{
						sprintf(tempText, "The %s defeats you in the %s (%dN %dE) at %dN %dE.",
							Name(),
							((DungeonMap *)ss)->name,
							256 - ((DungeonMap *)ss)->enterY,
							256 - ((DungeonMap *)ss)->enterX,
							((DungeonMap *)ss)->height - curAvatar->cellY,
							((DungeonMap *)ss)->width - curAvatar->cellX);
					}
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				}
				else if (!(curAvatar->curTarget))
				{
					curAvatar->lastAttackTime = now;
					curAvatar->curTarget = this;
				}

				if (24 == type) // spidren group
				{
					cVal = toHit - dodgeMod;

					if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
						cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

					cVal -= curAvatar->totemEffects.effect[TOTEM_PROT_WEB];

					chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

					if (cVal + 40 <= 20) // if monster can't EVER hit
					{
						if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
							chance = 30;  // Hit!
					}

					//    2. if chance > 20, hit was successful
					if (chance > 20)
					{
						// spin a web on her ass!
						MessMagicAttack messMA;
						messMA.damage = 30;
						messMA.mobID = -1;
						messMA.avatarID = curAvatar->socketIndex;
						messMA.type = 1;
						ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
							sizeof(messMA), &messMA, 3);

						tempReceiptList.clear();
						tempReceiptList.push_back(curAvatar->socketIndex);
						sprintf(tempText, "The %s ensnares you in its web!", Name());
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						int effectVal = toHit - dodgeMod / 2 - curAvatar->totemEffects.effect[TOTEM_PROT_WEB] * 2;
						if (effectVal < 2)
							effectVal = 2;

						curAvatar->MagicEffect(MONSTER_EFFECT_BIND,
							timeGetTime() + effectVal * 1000, effectVal);
						curAvatar->MagicEffect(MONSTER_EFFECT_TYPE_BLUE,
							timeGetTime() + effectVal * 1000, effectVal);

					}
				}
			}
			else
			{
				// whish!
				if (!(curAvatar->curTarget))
				{
					curAvatar->lastAttackTime = now;
					curAvatar->curTarget = this;
				}

			}
		}
		ss->avatars->Find(curMob);
		curMob = (BBOSMob *)ss->avatars->Next();
	}

}
void BBOSMonster::UnicornAttack(SharedSpace *ss) // and also butterfiles
{
	DWORD now = timeGetTime();
	std::vector<TagID> tempReceiptList;
	char tempText[1024];
	MessInfoText infoText;

	// generic particle efect
	MessGenericEffect messGE;
	messGE.avatarID = -1;
	messGE.mobID = (long)this;
	messGE.x = cellX;
	messGE.y = cellY;
	messGE.r = rand() %256;
	messGE.g = rand() %256;
	messGE.b = rand()%256;
	messGE.type = 0;  // type of particles
	messGE.timeLen = 1; // in seconds
	ss->SendToEveryoneNearBut(0, cellX, cellY,
		sizeof(messGE), (void *)&messGE);
	BBOSMob *curMob = (BBOSMob *)ss->avatars->First();
	while (curMob)
	{
		if ((abs(curMob->cellX - cellX) < 2) && (abs(curMob->cellY = cellY)) && (curMob->WhatAmI() == SMOB_AVATAR))
		{
			BBOSAvatar *curAvatar = (BBOSAvatar *)curMob;

			InvSkill *skillInfo = NULL;
			int dodgeMod = 0;
			InventoryObject *io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.First();
			while (io)
			{
				if (!strcmp("Dodging", io->WhoAmI()))
				{
					skillInfo = (InvSkill *)io->extra;
					dodgeMod = skillInfo->skillLevel - 1;
					int addAmount = toHit - dodgeMod;
					if (addAmount < 0)
						addAmount = 0;
					if (addAmount > 10)
						addAmount = 0;
					skillInfo->skillPoints += addAmount*bboServer->combat_exp_multiplier;

					//					if (abs(dodgeMod - toHit) < 3)
					//						skillInfo->skillPoints += 1;
				}
				io = (InventoryObject *)curAvatar->charInfoArray[curAvatar->curCharacterIndex].skills->objects.Next();
			}

			dodgeMod += curAvatar->totemEffects.effect[TOTEM_QUICKNESS];

			//    1. chance = 2d20 + ToHit - player.Physical
			int cVal = toHit - dodgeMod;

			if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
				cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

			int chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

			if (cVal + 40 <= 20) // if monster can't EVER hit
			{
				if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
					chance = 30;  // Hit!
			}
			chance = 30; // auto hit!
			//    2. if chance > 20, hit was successful
			if (chance > 20)
			{
				//    3. damage = damage
				int dDone = damageDone - curAvatar->totemEffects.effect[TOTEM_TOUGHNESS] / 3;
				if (dDone < 0)
					dDone = 0;

				if (MONSTER_PLACE_SPIRITS &
					monsterData[type][subType].placementFlags)
				{
					float fDone = (float)dDone *
						(1.0f - (curAvatar->totemEffects.effect[TOTEM_PROT_SPIRITS] / 30.0f));
					dDone = (int)fDone;
				}

				dDone += (int)rnd(0, dDone);

				if (curAvatar->totemEffects.effect[TOTEM_LIFESTEAL] > 0)
				{
					int suck = dDone * curAvatar->totemEffects.effect[TOTEM_LIFESTEAL] / 60;
					dDone += suck;
					health += suck;
				}
				if (curAvatar->totemEffects.effect[TOTEM_FOCUS] > 0 //if character has a focus totem
					&& dDone >= curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax) // and damage greater than or equal to max health
					dDone = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax - 1; // lower damage to max health-1 to not oneshot from full health

				if (ACCOUNT_TYPE_ADMIN != curAvatar->accountType)
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= dDone;

				MessAvatarHealth messHealth;
				messHealth.avatarID = curAvatar->socketIndex;
				messHealth.health = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
				messHealth.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messHealth), &messHealth, 2);

				if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health <= 0)
				{
					curAvatar->charInfoArray[curAvatar->curCharacterIndex].health = 0;
					curAvatar->isDead = TRUE;

					tempReceiptList.clear();
					tempReceiptList.push_back(curAvatar->socketIndex);
					if (SPACE_GROUND == ss->WhatAmI())
					{
						sprintf(tempText, "The %s defeats you at %dN %dE.", Name(),
							256 - curAvatar->cellY, 256 - curAvatar->cellX);
					}
					else if (SPACE_REALM == ss->WhatAmI())
					{
						sprintf(tempText, "The %s defeats you in a Mystic Realm at %dN %dE.",
							Name(),
							64 - curAvatar->cellY, 64 - curAvatar->cellX);
					}
					else if (SPACE_LABYRINTH == ss->WhatAmI())
					{
						sprintf(tempText, "The %s defeats you in the Labyrinth at %dN %dE.",
							Name(),
							64 - curAvatar->cellY, 64 - curAvatar->cellX);
					}
					else
					{
						sprintf(tempText, "The %s defeats you in the %s (%dN %dE) at %dN %dE.",
							Name(),
							((DungeonMap *)ss)->name,
							256 - ((DungeonMap *)ss)->enterY,
							256 - ((DungeonMap *)ss)->enterX,
							((DungeonMap *)ss)->height - curAvatar->cellY,
							((DungeonMap *)ss)->width - curAvatar->cellX);
					}
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				}
				else if (!(curAvatar->curTarget))
				{
					curAvatar->lastAttackTime = now;
					curAvatar->curTarget = this;
				}

				if (24 == type) // spidren group
				{
					cVal = toHit - dodgeMod;

					if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
						cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

					cVal -= curAvatar->totemEffects.effect[TOTEM_PROT_WEB];

					chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

					if (cVal + 40 <= 20) // if monster can't EVER hit
					{
						if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
							chance = 30;  // Hit!
					}

					//    2. if chance > 20, hit was successful
					if (chance > 20)
					{
						// spin a web on her ass!
						MessMagicAttack messMA;
						messMA.damage = 30;
						messMA.mobID = -1;
						messMA.avatarID = curAvatar->socketIndex;
						messMA.type = 1;
						ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
							sizeof(messMA), &messMA, 3);

						tempReceiptList.clear();
						tempReceiptList.push_back(curAvatar->socketIndex);
						sprintf(tempText, "The %s ensnares you in its web!", Name());
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						int effectVal = toHit - dodgeMod / 2 - curAvatar->totemEffects.effect[TOTEM_PROT_WEB] * 2;
						if (effectVal < 2)
							effectVal = 2;

						curAvatar->MagicEffect(MONSTER_EFFECT_BIND,
							timeGetTime() + effectVal * 1000, effectVal);
						curAvatar->MagicEffect(MONSTER_EFFECT_TYPE_BLUE,
							timeGetTime() + effectVal * 1000, effectVal);

					}
				}
			}
			else
			{
				// whish!
				if (!(curAvatar->curTarget))
				{
					curAvatar->lastAttackTime = now;
					curAvatar->curTarget = this;
				}

			}
		}
		ss->avatars->Find(curMob);
		curMob = (BBOSMob *)ss->avatars->Next();
	}

}
//----
void BBOSMonster::BMVLordAttack(SharedSpace *ss) // and also butterfiles
{
	DWORD now = timeGetTime();
	std::vector<TagID> tempReceiptList;
	MessInfoText infoText;

	MessMonsterSpecialAttack messAA;
	messAA.mobID = (long)this;
	messAA.type = 1;
	ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);

	BBOSMob *curMob = (BBOSMob *)ss->mobList->GetFirst(cellX, cellY);
	while (curMob)
	{
		if (curMob->cellX == cellX && curMob->cellY == cellY && curMob->WhatAmI() == SMOB_MONSTER) // ignore ground effects, merchants,chests, etc.
		{
			BBOSMonster *curMonster = (BBOSMonster *)curMob;

			int petenergy = 0;
			if (curMonster->controllingAvatar // safe because we have a curMonstertarget
				&& curMonster->controllingAvatar->BeastStat() > 9) // beast stat > 9
			{
				// go through skills
				InventoryObject *io = (InventoryObject *)
					curMonster->controllingAvatar->charInfoArray[curMonster->controllingAvatar->curCharacterIndex].skills->objects.First();
				while (io)
				{
					if (!strcmp("Pet Energy", io->WhoAmI())) // check for pet energy (beastmaster version of dodge)
					{
						InvSkill *skillInfo = (InvSkill *)io->extra;
						petenergy = skillInfo->skillLevel;
					}
					io = (InventoryObject *)
						curMonster->controllingAvatar->charInfoArray[curMonster->controllingAvatar->curCharacterIndex].skills->objects.Next();
				}
			}
			int cVal;
			if (petenergy > curMonster->defense)
			{
				cVal = toHit - petenergy;
			}
			else
			{
				cVal = toHit - curMonster->defense;
			}
			if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
				cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

			int chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

			if (cVal + 40 <= 20) // if monster can't EVER hit
			{
				if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
					chance = 30;  // Hit!
			}
			if (curMonster == this)
			{
				chance = 0; // miss myself. :)
			}

			//    2. if chance > 20, hit was successful
			if (chance > 20)
			{
				//    3. damage = damage
				int dDone = damageDone;
				if (dDone < 0)
					dDone = 0;

				dDone += (int)rnd(0, dDone);


				MessMonsterHealth messHealth;
				messHealth.mobID = (unsigned long)curMonster;
				messHealth.health = curMonster->health - dDone;
				messHealth.healthMax = curMonster->maxHealth;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messHealth), &messHealth, 2);
				curMonster->health -= dDone;
				if (controllingAvatar)
					curMonster->RecordDamageForLootDist(dDone, controllingAvatar); // credit the controlling avatar
				// send events
				if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI()) // target has a genrator,a dn it's an army
				{
					BBOSArmy *army = (BBOSArmy *)curMonster->myGenerator;
					army->MonsterEvent(curMonster, ARMY_EVENT_ATTACKED); // send attack event to tell army that this member was attacked.
					//				curMob = (BBOSMob *) ss->mobs->Find(this);
				}
				if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI()) // target has a genrator,a dn it's an quest
				{
					BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonster->myGenerator;
					quest->MonsterEvent(curMonster, AUTO_EVENT_ATTACKED); // send attack event to tell quest that this member was attacked (if it cares).
					//				curMob = (BBOSMob *) ss->mobs->Find(this);
				}
				if (curMonster->health <= 0)
				{
					// Same check as for normal kills
					MessMonsterAttack messAA;
					messAA.avatarID = (int)curMonster;
					messAA.mobID = (long)this;
					messAA.damage = -1000;
					//				messAA.health   = -1000;
					ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);
					// beastmasters pet stats improve for killing monsters.
					if (controllingAvatar && (controllingAvatar->BeastStat() > 9))
					{
						// do stat check to see if we can get a boost.
						if (defense <= curMonster->toHit) // if monsters tohit >= defense
						{
							//boost some stats
							maxHealth = maxHealth * (defense + 1) / defense; //recalculate health as if it were a higher level monster.
							defense++; // better defense to keep from gettign advancement from the same types of monsters.
							toHit++;   // so we can hit more often.
							damageDone++; // and do some more damage.
							controllingAvatar->SaveAccount(); // and save the character so the pets boosted stats get saved.
							// and, sometimes, add gold dust to the dead monster's inventory
							bool GiveGold = FALSE;  // default, will set to true later.
							int GoldEveryXLevels = 5; // change me to make them more or less common.
							if ((controllingAvatar->GetTamingExp() < 1) && (controllingAvatar->GetTamingLevel() < 2)) // no tame skill, or haven't tamed the next golem.
							{
								// we haven't tamed anything else, you are allowed to reset to gather more golds for your first Scythe
								GiveGold = TRUE;
							}
							if ((controllingAvatar->GetTamingExp() < subType) || (controllingAvatar->GetTamingLevel() < 2)) // if current monster is not the one that will be replaced if you release
							{
								// we will have to retame a new one to bump it's level back, using charges.
								GiveGold = TRUE;
							}
							if (type > 26) // if monster is vlord or better
							{
								// go ahead and let them get gold dust anyway, even if the level is reset. tey are already hard to level withotu spending mucho gold
								GiveGold = TRUE;
							}
							if (GiveGold && ((defense - monsterData[type][subType].defense) % GoldEveryXLevels) == 0) // every five levels, if we aren't disqualified.
							{
								// create a new Glowing Gold dust
								InventoryObject * iObject = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Gold Dust");
								InvIngredient * exIn = (InvIngredient *)iObject->extra;
								exIn->type = INGR_GOLD_DUST;
								exIn->quality = 1;

								iObject->mass = 0.0f;
								iObject->value = 1000;
								iObject->amount = 1;
								// add it to the inventory of the monster that's about to die
								curMonster->inventory->AddItemSorted(iObject);

							}
						}
					}
					curMonster->isDead = TRUE;
					curMonster->bane = NULL;
					if (controllingAvatar)
					{
						curMonster->bane = controllingAvatar;  // set bane
						curMonster->HandleQuestDeath();		 // call quest death handler
					}


					if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
					{
						MapListState oldState = ss->mobList->GetState();

						BBOSArmy *army = (BBOSArmy *)curMonster->myGenerator;
						army->MonsterEvent(curMonster, ARMY_EVENT_DIED);

						ss->mobList->SetState(oldState);
					}
					else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
					{
						MapListState oldState = ss->mobList->GetState();

						BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonster->myGenerator;
						quest->MonsterEvent(curMonster, AUTO_EVENT_DIED);
						ss->mobList->SetState(oldState);
					}


				}
				else if (!(curMonster->curMonsterTarget))
				{
					curMonster->lastAttackTime = now;
					curMonster->curMonsterTarget = this;
				}


			}
			else
			{
				// whish!
				if (!(curMonster->curMonsterTarget))
				{
					curMonster->lastAttackTime = now;
					curMonster->curMonsterTarget = this;
				}

			}
		}
		// get next monster
		curMob = (BBOSMob *)ss->mobList->GetNext();
	}

}

void BBOSMonster::BMUnicornAttack(SharedSpace *ss) // and also butterfiles
{
	DWORD now = timeGetTime();
	std::vector<TagID> tempReceiptList;
	MessInfoText infoText;

	// generic particle efect
	MessGenericEffect messGE;
	messGE.avatarID = -1;
	messGE.mobID = (long)this;
	messGE.x = cellX;
	messGE.y = cellY;
	messGE.r = rand() % 256;
	messGE.g = rand() % 256;
	messGE.b = rand() % 256;
	messGE.type = 0;  // type of particles
	messGE.timeLen = 1; // in seconds
	ss->SendToEveryoneNearBut(0, cellX, cellY,
		sizeof(messGE), (void *)&messGE);

	BBOSMob *curMob = (BBOSMob *)ss->mobList->GetFirst(cellX, cellY,1);
	while (curMob)
	{
		if (curMob->WhatAmI() == SMOB_MONSTER) // ignore ground effects, merchants,chests, etc.
		{
			BBOSMonster *curMonster = (BBOSMonster *)curMob;
			int petenergy = 0;
			if (curMonster->controllingAvatar // safe because we have a curMonstertarget
				&& curMonster->controllingAvatar->BeastStat() > 9) // beast stat > 9
			{
				// go through skills
				InventoryObject *io = (InventoryObject *)
					curMonster->controllingAvatar->charInfoArray[curMonster->controllingAvatar->curCharacterIndex].skills->objects.First();
				while (io)
				{
					if (!strcmp("Pet Energy", io->WhoAmI())) // check for pet energy (beastmaster version of dodge)
					{
						InvSkill *skillInfo = (InvSkill *)io->extra;
						petenergy = skillInfo->skillLevel;
					}
					io = (InventoryObject *)
						curMonster->controllingAvatar->charInfoArray[curMonster->controllingAvatar->curCharacterIndex].skills->objects.Next();
				}
			}
			int cVal;
			if (petenergy > curMonster->defense)
			{
				cVal = toHit - petenergy;
			}
			else
			{
				cVal = toHit - curMonster->defense;
			}
			if (magicEffectAmount[DRAGON_TYPE_WHITE] > 0)
				cVal -= magicEffectAmount[DRAGON_TYPE_WHITE] * 1.0f;

			int chance = 2 + (rand() % 20) + (rand() % 20) + cVal;

			if (cVal + 40 <= 20) // if monster can't EVER hit
			{
				if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
					chance = 30;  // Hit!
			}
			chance = 30; // auto hits!
			if (curMonster == this)
			{
				chance = 0; // miss myself. :)
			}
						 //    2. if chance > 20, hit was successful
			if (chance > 20)
			{
				//    3. damage = damage
				int dDone = damageDone;
				if (dDone < 0)
					dDone = 0; 

				dDone += (int)rnd(0, dDone);


				MessMonsterHealth messHealth;
				messHealth.mobID = (unsigned long)curMonster;
				messHealth.health = curMonster->health - dDone;
				messHealth.healthMax = curMonster->maxHealth;
				ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messHealth), &messHealth, 2);
				curMonster->health -= dDone;
				if (controllingAvatar)
					curMonster->RecordDamageForLootDist(dDone, controllingAvatar); // credit the controlling avatar
				// send events
				if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI()) // target has a genrator,a dn it's an army
				{
					BBOSArmy *army = (BBOSArmy *)curMonster->myGenerator;
					army->MonsterEvent(curMonster, ARMY_EVENT_ATTACKED); // send attack event to tell army that this member was attacked.
					//				curMob = (BBOSMob *) ss->mobs->Find(this);
				}
				if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI()) // target has a genrator,a dn it's an quest
				{
					BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonster->myGenerator;
					quest->MonsterEvent(curMonster, AUTO_EVENT_ATTACKED); // send attack event to tell quest that this member was attacked (if it cares).
					//				curMob = (BBOSMob *) ss->mobs->Find(this);
				}
				if (curMonster->health <= 0)
				{
					// Same check as for normal kills
					MessMonsterAttack messAA;
					messAA.avatarID = (int)curMonster;
					messAA.mobID = (long)this;
					messAA.damage = -1000;
					//				messAA.health   = -1000;
					ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messAA), &messAA, 2);
					// beastmasters pet stats improve for killing monsters.
					if (controllingAvatar && (controllingAvatar->BeastStat() > 9))
					{
						// do stat check to see if we can get a boost.
						if (defense <= curMonster->toHit) // if monsters tohit >= defense
						{
							//boost some stats
							maxHealth = maxHealth * (defense + 1) / defense; //recalculate health as if it were a higher level monster.
							defense++; // better defense to keep from gettign advancement from the same types of monsters.
							toHit++;   // so we can hit more often.
							damageDone++; // and do some more damage.
							controllingAvatar->SaveAccount(); // and save the character so the pets boosted stats get saved.
							// and, sometimes, add gold dust to the dead monster's inventory
							bool GiveGold = FALSE;  // default, will set to true later.
							int GoldEveryXLevels = 5; // change me to make them more or less common.
							if ((controllingAvatar->GetTamingExp() < 1) && (controllingAvatar->GetTamingLevel() < 2)) // no tame skill, or haven't tamed the next golem.
							{
								// we haven't tamed anything else, you are allowed to reset to gather more golds for your first Scythe
								GiveGold = TRUE;
							}
							if ((controllingAvatar->GetTamingExp() < subType) || (controllingAvatar->GetTamingLevel() < 2)) // if current monster is not the one that will be replaced if you release
							{
								// we will have to retame a new one to bump it's level back, using charges.
								GiveGold = TRUE;
							}
							if (type > 26) // if monster is vlord or better
							{
								// go ahead and let them get gold dust anyway, even if the level is reset. tey are already hard to level withotu spending mucho gold
								GiveGold = TRUE;
							}
							if (GiveGold && ((defense - monsterData[type][subType].defense) % GoldEveryXLevels) == 0) // every five levels, if we aren't disqualified.
							{
								// create a new Glowing Gold dust
								InventoryObject * iObject = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Gold Dust");
								InvIngredient * exIn = (InvIngredient *)iObject->extra;
								exIn->type = INGR_GOLD_DUST;
								exIn->quality = 1;
									
								iObject->mass = 0.0f;
								iObject->value = 1000;
								iObject->amount = 1;
								// add it to the inventory of the monster that's about to die
								curMonster->inventory->AddItemSorted(iObject);

							}
						}
					}
					curMonster->isDead = TRUE;
					curMonster->bane = NULL;
					if (controllingAvatar)
					{
						curMonster->bane = controllingAvatar;  // set bane
						curMonster->HandleQuestDeath();		 // call quest death handler
					}


					if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
					{
						MapListState oldState = ss->mobList->GetState();

						BBOSArmy *army = (BBOSArmy *)curMonster->myGenerator;
						army->MonsterEvent(curMonster, ARMY_EVENT_DIED);

						ss->mobList->SetState(oldState);
					}
					else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
					{
						MapListState oldState = ss->mobList->GetState();

						BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonster->myGenerator;
						quest->MonsterEvent(curMonster, AUTO_EVENT_DIED);
						ss->mobList->SetState(oldState);
					}
					
					
				}
				else if (!(curMonster->curMonsterTarget))
				{
					curMonster->lastAttackTime = now;
					curMonster->curMonsterTarget = this;
				}


			}
			else
			{
				// whish!
				if (!(curMonster->curMonsterTarget))
				{
					curMonster->lastAttackTime = now;
					curMonster->curMonsterTarget = this;
				}

			}
		}
		// get next monster
		curMob = (BBOSMob *)ss->mobList->GetNext();
	}

}

//******************************************************************
void BBOSMonster::PortalBat(SharedSpace *ss)
{
	char tempText[1024];
	int searchtype = 26; // bats by default
	if ((ss->WhatAmI() == SPACE_LABYRINTH) && (((LabyrinthMap *)ss)->type == REALM_ID_LAB3)) // if in level three lab
		searchtype = 29; // find bunnies instead.
	MapListState state = ss->mobList->GetState();
	int foundBat = FALSE;
	BBOSMob *tMob = NULL;
	tMob = ss->mobList->GetFirst(cellX, cellY, 1000);
	while (tMob && !foundBat)
	{
		if (SMOB_MONSTER == tMob->WhatAmI() && 
			 searchtype == ((BBOSMonster *) tMob)->type && 
			 4 == (rand() % 10))
		{
			foundBat = TRUE;
			if (((BBOSMonster *)tMob)->controllingAvatar)
				foundBat = FALSE; // don't find controlled monsters.
		}
		if (!foundBat)
			tMob = ss->mobList->GetNext();
	}

	ss->mobList->SetState(state);

	if (tMob && foundBat)
	{
		// teleport next to me
		MessMobDisappear messMobDisappear;
		messMobDisappear.mobID = (unsigned long) tMob;
		messMobDisappear.x = tMob->cellX;
		messMobDisappear.y = tMob->cellY;
		ss->SendToEveryoneNearBut(0, tMob->cellX, tMob->cellY,
			sizeof(messMobDisappear),(void *)&messMobDisappear,6);
		// also update it's spawn location so it will chase you properly
		tMob->cellX = tMob->targetCellX = ((BBOSMonster *)tMob)->spawnX = targetCellX; // cast is safe since we KNOW it's a bat
		tMob->cellY = tMob->targetCellY = ((BBOSMonster *)tMob)->spawnY = targetCellY;
//		tMob->cellX = tMob->targetCellX = targetCellX;
//		tMob->cellY = tMob->targetCellY = targetCellY;
		ss->mobList->Move(tMob);

		// send arrival message
		MessMobAppear mAppear;
		mAppear.mobID = (unsigned long) tMob;
		mAppear.x = tMob->cellX;
		mAppear.y = tMob->cellY;
		mAppear.monsterType = ((BBOSMonster *) tMob)->type;
		mAppear.subType = ((BBOSMonster *) tMob)->subType;
		mAppear.type = tMob->WhatAmI();

		ss->SendToEveryoneNearBut(0, tMob->cellX, tMob->cellY, sizeof(mAppear), &mAppear,6);

		// laugh
		MessGenericEffect messGE;
		messGE.avatarID = -1;
		messGE.mobID    = (long) tMob;
		messGE.x        = tMob->cellX;
		messGE.y        = tMob->cellY;
		messGE.r        = 255;
		messGE.g        = 0;
		messGE.b        = 255;
		messGE.type     = 0;  // type of particles
		messGE.timeLen  = 1; // in seconds
		ss->SendToEveryoneNearBut(0, tMob->cellX, tMob->cellY,
							 sizeof(messGE),(void *)&messGE);

		// announce the summoning
		if ((ss->WhatAmI() == SPACE_LABYRINTH) && (((LabyrinthMap *)ss)->type == REALM_ID_LAB3)) // if in level three lab
			sprintf(&(tempText[2]), "A bunny has been summoned!");
		else
			sprintf(&(tempText[2]), "A bat has been summoned!");
		tempText[0] = NWMESS_PLAYER_CHAT_LINE;
		tempText[1] = TEXT_COLOR_SHOUT;
		ss->SendToEveryoneNearBut(0, tMob->cellX, tMob->cellY, 
			       strlen(tempText) + 1,(void *)&tempText);

	}

}

/* end of file */



