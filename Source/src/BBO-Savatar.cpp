
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "BBOServer.h"
#include "BBO-SAvatar.h"
#include "BBO-SMonster.h"
#include "BBO-SNpc.h"
#include "BBO-Sbomb.h"
#include "BBO-Stower.h"
#include "BBO-Schest.h"
#include "BBO-Stree.h"
#include "BBO-Sarmy.h"
#include "BBO-SAutoQuest.h"
#include "BBO-SwarpPoint.h"
#include "BBO-SgroundEffect.h"
#include "BBO.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "TotemData.h"
#include "StaffData.h"
#include "monsterData.h"
#include "dungeon-map.h"
#include "labyrinth-map.h"
#include "realm-map.h"
#include "tower-map.h"
#include "tokenManager.h"
#include "QuestSystem.h"
#include ".\helper\crc.h"
#include "version.h"
#include ".\helper\uniquenames.h"
#include ".\helper\sendMail.h"
#include <Shlobj.h>
#include <direct.h>
#include "hotkeys.h"
extern int lastAvatarCount;

// int skillHotKeyArray[20] = {88,79, 75,72,78,66,67, 49,50,51,52, 53,54,55,56, 71, 68, 57, 48, 58}; // added hotkey for disarming
//                                                                            g   d	 

char skillNameArray[20][32] =
{
    {"Explosives"},
    {"Swordsmith"},
    {"Katana Expertise"},
    {"Chaos Expertise"},
    {"Mace Expertise"},
    {"Bladestaff Expertise"},
    {"Claw Expertise"},
    {"Bear Magic"},
    {"Wolf Magic"},
    {"Eagle Magic"},
    {"Snake Magic"},
    {"Frog Magic"},
    {"Sun Magic"},
    {"Moon Magic"},
    {"Turtle Magic"},
    {"Geomancy"},
    {"Weapon Dismantle"},
    {"Evil Magic"},
	{"Totem Shatter"},
	{"Disarming"}

};

char earthKeyArray[10][32] =
{
    {"Karaol"},
    {"Vil"},
    {"Char"},
    {"Rin"},
    {"Bool"},
    {"Teth"},
    {"Gar"},
    {"Sver"},
    {"Mite"},
    {"Haow"}
};

struct PosMonsterTypes
{
    int type, subType;
    char name[32];
};

PosMonsterTypes pmTypes[6] =
{
    {1,3,"Possessed Golem"},
    {2,4,"Possessed Minotaur"},
    {3,1,"Possessed Tiger"},
    {8,2,"Possessed Spider"},
	{ 7,0,"Possessed Dragon" }, 
	{29,5,"Possessed Rabbit"}
} ;

//******************************************************************
//******************************************************************
SkillObject::SkillObject(int doid, char *doname)	 : DataObject(doid,doname)
{
    exp = expUsed = 0;
}

//******************************************************************
SkillObject::~SkillObject()
{
    
}


//******************************************************************
//******************************************************************
BBOSCharacterInfo::BBOSCharacterInfo(void)
{
    inventory = new Inventory(MESS_INVENTORY_PLAYER);
    workbench = new Inventory(MESS_WORKBENCH_PLAYER);
    skills	 = new Inventory(MESS_SKILLS_PLAYER);
    wield		 = new Inventory(MESS_WIELD_PLAYER);

    cLevel = oldCLevel = 0;
    sprintf(witchQuestName,"NO WITCH");

    age = 1;
}

//******************************************************************
BBOSCharacterInfo::~BBOSCharacterInfo()
{
    delete inventory;
    delete workbench;
    delete skills	 ;
    delete wield	 ;	
}


//******************************************************************
//******************************************************************
BBOSAvatar::BBOSAvatar() : BBOSMob(SMOB_AVATAR,"AVATAR")
{
    SkillCellX = cellX = targetCellX = townList[2].x;  
    SkillCellY = cellY = targetCellY = townList[2].y;
    moveStartTime = 0;
    lastSaveTime = lastHealTime = combineStartTime = lastTenTime = lastMinuteTime = 0;
    isMoving = TRUE;
    moveTimeCost = 0;
    isCombining = FALSE;
    curTarget = FALSE;
    activeCounter = 0;
    chantType = -1; // means no chant
    infoFlags = 0xffffffff; // all on
    kickOff = FALSE;
	followMode = FALSE;
    trade = new Inventory(MESS_INVENTORY_YOUR_SECURE, this);
    bank  = new Inventory(MESS_INVENTORY_BANK       , this);

    agreeToTrade = FALSE;
	// anti bot stuff
	EvilBotter = FALSE; // innocent until proven guilty
	SameCellCount = 0;  // skilling up repeatedly without moving is tracked.
	CorrectAnswer = 'a';  // neither is right until you have been challenged.
	BotTPCount = 0;       // this goes up each tiem you get sent
	isInvisible = FALSE;
	isTeleporting = FALSE;
	controlledMonster = NULL;
	PVPEnabled = FALSE;

    for (int i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
    {
        charInfoArray[i].inventory->parent = this;
        charInfoArray[i].workbench->parent = this;
        charInfoArray[i].skills->parent    = this;
        charInfoArray[i].wield->parent     = this;

        charInfoArray[i].petDragonInfo[0].lastAttackTime = 0;
        charInfoArray[i].petDragonInfo[1].lastAttackTime = 0;

        for (int j = 0; j < SAMARITAN_REL_MAX; ++j)
        {
            for (int k = 0; k < SAMARITAN_TYPE_MAX; ++k)
            {
                charInfoArray[i].karmaGiven[j][k] = 0;
                charInfoArray[i].karmaReceived[j][k] = 0;
            }
        }

    }

    for (int i = 0; i < MAP_SIZE_WIDTH*MAP_SIZE_HEIGHT; ++i)
        updateMap[i] = FALSE;

    lastPlayerInvSent = MESS_INVENTORY_PLAYER;
    for (int i = 0; i < MESS_INVENTORY_MAX; ++i)
        indexList[i] = 0;

    sprintf(name,"UNINITALIZED");

    accountExperationTime.AddMinutes(60*24*14);

    contacts = new DoublyLinkedList();

    tradingPartner = NULL;

    timeOnCounter = kickMeOffNow = 0;

    lastTellName[0] = 0;

    specLevel[0] = specLevel[1] = specLevel[2] = 0;

    restrictionType = 0;

    status = AVATAR_STATUS_AVAILABLE;
    sprintf( status_text, "" );

    for (int i = 0; i < TOTEM_MAX; ++i)
        totemEffects.effect[i] = 0;

    isReferralDone = patronCount = 0;

    bHasLoaded = false;

}

//******************************************************************
BBOSAvatar::~BBOSAvatar()
{
    delete bank;
    delete trade;
    delete contacts;
}

//******************************************************************
void BBOSAvatar::Tick(SharedSpace *ss)
{
    if (!bHasLoaded)
    {
        kickOff = true;
    }

    BBOSMob *curMob = NULL;
    BBOSMonster *curMonster = NULL;
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);
    char tempText[1024];
    MessInfoText infoText;
	if (curCharacterIndex >= 0 && curCharacterIndex < NUM_OF_CHARS_PER_USER)
    {
        assert(charInfoArray[curCharacterIndex].inventory->money >= 0);
    }

    DungeonMap *dm = NULL;
    if (SPACE_DUNGEON == ss->WhatAmI())
        dm = (DungeonMap *) ss;

    DWORD delta;
    DWORD now = timeGetTime();
    if (isMoving)
    {
        agreeToTrade = FALSE;  // never trade while running

        delta = now - moveStartTime;

        if (0 == moveStartTime || now < moveStartTime) // || now == moveStartTime)
        {
            delta = moveTimeCost + 1;
        }

        curTarget = FALSE;
        // Finish the move
        if (delta > moveTimeCost)	
        {
            isMoving = FALSE;
			if (!EvilBotter)
			{
				BotTPCount = 0;
			}
			else
			{
				BotTPCount = 3;  // if you moved just to allow reto, you aren't out of the woods yet, and need to move again after the reto to clear it
			}

			EvilBotter = FALSE; // clear botter flag if you moved so the thieving spirit won't steal everything.
			if (SPACE_GROUND == ss->WhatAmI())
            {
                // entering a town?
                for (int i = 0; i < NUM_OF_TOWNS; ++i)
                {
                    if (!(abs(townList[i].x - cellX) <= 3 &&	abs(townList[i].y - cellY) <= 3) &&
                         (abs(townList[i].x - targetCellX) <= 3 && abs(townList[i].y - targetCellY) <= 3) &&
						(((GroundMap *)ss)->type==0)) // but only in actual normal realm
                    {
                        sprintf(tempText,"You are entering %s.",townList[i].name);
                        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                        
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        charInfoArray[curCharacterIndex].spawnX = townList[i].x;
                        charInfoArray[curCharacterIndex].spawnY = townList[i].y;\
                    }
                }

                // leaving a town?
                for (int i = 0; i < NUM_OF_TOWNS; ++i)
                {
                    if ((abs(townList[i].x - cellX) <= 3 &&	abs(townList[i].y - cellY) <= 3) &&
                         !(abs(townList[i].x - targetCellX) <= 3 && abs(townList[i].y - targetCellY) <= 3) &&
						(((GroundMap *)ss)->type == 0)) // but only in actual normal realm
                    {
                        sprintf(tempText,"You are leaving %s.",townList[i].name);
                        memcpy(infoText.text, tempText, 63);
                        
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        charInfoArray[curCharacterIndex].spawnX = townList[i].x;
                        charInfoArray[curCharacterIndex].spawnY = townList[i].y;
                    }
                }
            }
            cellX = targetCellX;
            cellY = targetCellY;

            // send arrival message
            MessAvatarAppear mAppear;
            mAppear.avatarID = socketIndex;
            mAppear.x = cellX;
            mAppear.y = cellY;
            if (isInvisible || isTeleporting)
                ss->lserver->SendMsg(sizeof(mAppear), &mAppear, 0, &tempReceiptList);
            else
                ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(mAppear), &mAppear);

            UpdateClient(ss);

            QuestMovement(ss);

            // make monsters aggro on me (as I'm arriving)!
            curMob = ss->mobList->GetFirst(cellX, cellY, 1);
            while (curMob)
            {
                if (SMOB_MONSTER  == curMob->WhatAmI())
                {
                    if (curMob->cellX == cellX && 
                         curMob->cellY == cellY)
                    {
                        curMonster = (BBOSMonster *) curMob;

                        // make him aggro
                        if (!curMonster->curTarget && !isInvisible &&
                             !curMonster->controllingAvatar &&
							curMonster->type != 7 &&  // not normal dragon
							curMonster->type != 31 &&  // not pretty unicorm
							!(dm && dm->CanEdit(this))
                            )
                        {
								curMonster->lastAttackTime = now - (rand() % 1000);
								curMonster->curTarget = this;
                        }
                        else
                            curMonster = NULL;
                    }
                    else if ((abs(curMob->cellX - cellX)<2) &&(abs(curMob->cellY - cellY)<2) && // new check lets monsters see you from diagonal. soem of them will be able to move diagonally in the future.
                              !isInvisible && !(((BBOSMonster *) curMob)->controllingAvatar))
                    {
                        ((BBOSMonster *) curMob)->ReactToAdjacentPlayer(this, ss);
                    }
                }
                curMob = ss->mobList->GetNext();
            }

            // and if the monster attacks me, I'm gonna attack him right back!
            if (!curTarget && curMonster && !isInvisible && !curMonster->controllingAvatar && (!controlledMonster))
            {
				
                lastAttackTime = now;
                charInfoArray[curCharacterIndex].petDragonInfo[0].lastAttackTime = now;
                charInfoArray[curCharacterIndex].petDragonInfo[1].lastAttackTime = now;

                curTarget = curMonster;
            }

            tokenMan.PlayerEntersSquare(charInfoArray[curCharacterIndex].name, ss, cellX, cellY);

        }
        return;
    }

    delta = now - lastAttackTime;

    if (0 == lastAttackTime || now < lastAttackTime) // || now == moveStartTime)
    {
        delta = 1000 * 2 + 1;
    }
	int secondsforattack = 1000;
	// fighters will attack a littel faster now
	if (charInfoArray[curCharacterIndex].physical > 9)
	{
		// secondsforattack = 750; //simple flat adjustment
		// alternatively age based adjustment wtf
		secondsforattack -= (charInfoArray[curCharacterIndex].age - 1) * 40;
	}
	// but NOT if it's a spirit monster, to not interfere with skilling.
	if (curTarget && (curTarget->subType==0) && (curTarget->type>8) && (curTarget->type<18))
		secondsforattack = 1000;
	// attack curTarget (if it's still there)
    if (delta > secondsforattack * 2 + magicEffectAmount[DRAGON_TYPE_BLUE] * 300 && curTarget &&
         !(dm && dm->CanEdit(this) && 0 == curTarget->form)
        )	
    {
        lastAttackTime = now;
        int didAttack = FALSE;
		if (magicEffectAmount[MONSTER_EFFECT_STUN] > 0)
		{
			didAttack = TRUE; // enogh to abort the attack
		}
        InvStaff usedStaff;
        int staffRange;
        usedStaff.charges = -3; // not used;

        curMob = ss->mobList->GetFirst(cellX, cellY);
        while (curMob && !didAttack)   // entire sectionskipped if you were stunned.
        {
            if (curMob == curTarget && SMOB_MONSTER == curMob->WhatAmI() && !curMob->isDead &&  
                  curMob->cellX == cellX && curMob->cellY == cellY) 
            {
                didAttack = TRUE;
                curMonster = (BBOSMonster *) curMob;
				if (curMonster->controllingAvatar && (curMonster->controllingAvatar->BeastStat() > 9) && ((!curMonster->curTarget) ||(curMonster->curTarget != this)))
				{
					curTarget=NULL; // untarget.
					break; // cancel the attack.
				}
                // find the weapon the avatar is using
                long bladeToHit = -1, bladeDamage;
                int bladePoison, bladeHeal, bladeSlow, bladeBlind, bladeShock, bladeTame;
                bladePoison = bladeHeal = bladeSlow = bladeBlind = bladeShock = bladeTame= 0;
    
                InvBlade *iBlade = NULL;
                InvStaff *iStaff = NULL;

                InventoryObject *io = (InventoryObject *) 
                    charInfoArray[curCharacterIndex].wield->objects.First();

                while (io && -1 == bladeToHit && !iStaff)
                {
                    if (INVOBJ_BLADE == io->type)
                    {
                        // if there are multiple swords
                        if( io->amount > 1 ) {
                            // create a copy of the sword being used
                            InventoryObject *tmp = new InventoryObject( INVOBJ_BLADE, 0, "tmp" );
                            io->CopyTo( tmp );
                            io->amount -= 1;  // decrement the amount

                            // tell the new sword it is one object
                            tmp->amount = 1;

                            // add it to the beginning of the list
                            (InventoryObject *) 
                                charInfoArray[curCharacterIndex].wield->objects.Prepend( tmp );

                            // set it as the attacking item
                            io = tmp;

                        }

                        iBlade = (InvBlade *)io->extra;

                        bladeToHit  = ((InvBlade *)io->extra)->toHit;
                        bladeDamage = ((InvBlade *)io->extra)->damageDone;

                        bladePoison = ((InvBlade *)io->extra)->poison;
                        bladeHeal   = ((InvBlade *)io->extra)->heal;
                        bladeSlow   = ((InvBlade *)io->extra)->slow;
                        bladeBlind  = ((InvBlade *)io->extra)->blind;
						bladeShock = ((InvBlade *)io->extra)->lightning;
						bladeTame = ((InvBlade *)io->extra)->tame;
					}

                    if (INVOBJ_STAFF == io->type)
                    {
                        iStaff = (InvStaff *) io->extra;
						if (iStaff->type == STAFF_TAMING)
							iStaff = NULL; // do not count taming staffs!
                    }
                    io = (InventoryObject *) 
                        charInfoArray[curCharacterIndex].wield->objects.Next();
                }

                // nerfing green and black dust effects on weapons
//              bladePoison = 0; //bladePoison/4;
//              bladeHeal   = 0; //bladeHeal/4;

                if (iStaff)
                {
                    if (iStaff->isActivated)
                    {
                        // swing!
                        MessAvatarAttack messAA;
                        messAA.avatarID = socketIndex;
                        messAA.mobID    = (long) curMonster;
                        messAA.damage   = -2;
                        messAA.health   = -1000;
                        ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);

                        if (!(curMonster->curTarget) || 
                                 (24 == curMonster->type && 3 == (rand() % 10))
                            )
                        {
                            curMonster->lastAttackTime = now;
                            curMonster->curTarget = this;
                        }

                        // do the effect!
                        staffRange = StaffAffectsArea(iStaff);
                        if (staffRange > 0)
                            usedStaff = *iStaff;
                        else
                            UseStaffOnMonster(ss, iStaff, curMonster);

                        // reduce the charge!
                        --(iStaff->charges);
                        if (iStaff->charges <= 0)
                        {
                            // it's gone!

                            InventoryObject *io = (InventoryObject *) 
                                charInfoArray[curCharacterIndex].wield->objects.First();
                            while (io)
                            {
                                if (INVOBJ_STAFF == io->type && iStaff == (InvStaff *) io->extra)
                                {
                                    charInfoArray[curCharacterIndex].wield->objects.Remove(io);
                                    delete io;
                                    iStaff = NULL;

                                    MessUnWield messUnWield;
                                    messUnWield.bladeID = (long)socketIndex;
                                    ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY,
                                        sizeof(messUnWield),(void *)&messUnWield);
                                }
                                io = (InventoryObject *) 
                                    charInfoArray[curCharacterIndex].wield->objects.Next();
                            }

                            // wield the next wielded weapon (if you have one)
                            MessBladeDesc messBladeDesc;

                            io = (InventoryObject *) 
                                charInfoArray[curCharacterIndex].wield->objects.First();
                            while (io)
                            {
                                if (INVOBJ_BLADE == io->type)
                                {

                                    FillBladeDescMessage(&messBladeDesc, io, this);
                                    if (isInvisible)
                                        ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
                                    else
                                        ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY,
                                                sizeof(messBladeDesc),(void *)&messBladeDesc);

                                    charInfoArray[curCharacterIndex].wield->objects.Last();
                                }
                                else if (INVOBJ_STAFF == io->type)
                                {
                                    InvStaff *iStaff = (InvStaff *) io->extra;
                                    
                                    messBladeDesc.bladeID = (long)io;
                                    messBladeDesc.size    = 4;
                                    messBladeDesc.r       = staffColor[iStaff->type][0];
                                    messBladeDesc.g       = staffColor[iStaff->type][1];
                                    messBladeDesc.b       = staffColor[iStaff->type][2]; 
                                    messBladeDesc.avatarID= socketIndex;
                                    messBladeDesc.trailType  = 0;
                                    messBladeDesc.meshType = BLADE_TYPE_STAFF1;
									if (iStaff->type == STAFF_TAMING)
										iStaff = NULL; // ignore taming staff
									else
									{
										// send the equip message
										if (isInvisible)
											ss->lserver->SendMsg(sizeof(messBladeDesc), (void *)&messBladeDesc, 0, &tempReceiptList);
										else
											ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY,
												sizeof(messBladeDesc), (void *)&messBladeDesc);

										charInfoArray[curCharacterIndex].wield->objects.Last();
									}
                                }

                                io = (InventoryObject *) 
                                    charInfoArray[curCharacterIndex].wield->objects.Next();
                            }

                        }
                    }
                    
                }
                else
                {
                    if (-1 == bladeToHit)
                    {
                        bladeToHit = 1;  // fist is minimal;
                        bladeDamage = 1;
                    }

                    // 1. chance = 2d20 + Blade * Physical + weapon.ToHit - monster.Defense
                    int cVal = bladeToHit - 
                                     curMonster->defense + PhysicalStat() +
                                     totemEffects.effect[TOTEM_ACCURACY];
					if (magicEffectAmount[MONSTER_EFFECT_TYPE_WHITE] > 0)                // if we are blinded somehow.
					{
						cVal = cVal - magicEffectAmount[MONSTER_EFFECT_TYPE_WHITE];      // blindness cancels out blues.  
					}
					int chance = (rand() % 20) + 2 + (rand() % 20) + cVal;
                    if (cVal + 40 <= 20) // if you can't EVER hit, becaue you don't have enough blues, or are blinded
                    {
                        if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
                            chance = 30;  // Hit!  even if blinded.
                    }
					if (iBlade && iBlade->type == BLADE_TYPE_TAME_SCYTHE) // taming scythes don't nedd blues
					{
						chance = 30; // hit! but golds checked later to determine if taming works.
					}
                    // 2. if chance > 20, hit was successful
                    if (chance > 20)
                    {
                        // 3. damage = Physical * weapon.Damage
                        long damValue = (long)(bladeDamage * 
                                            (1 + PhysicalStat() * 0.15f) + 
                                             totemEffects.effect[TOTEM_STRENGTH]);

                        AddMastery( ss );	// Add to mastery level

                        long additional_stun = 0;


                        if (specLevel[0] > 0)
                            damValue = (long)(damValue * (1 + 0.05f * specLevel[0]));

                        if (iBlade && BLADE_TYPE_CHAOS == iBlade->type)
                        {
                            if (3 == rand() % 20)
                            {
                                damValue *= 3;

                                if (infoFlags & INFO_FLAGS_HITS) {
                                    sprintf( tempText,"With a burst of strength, you do THREE TIMES as much damage!.");
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                            }
                            else if (2 == rand() % 5)
                            {
                                damValue *= 2;

                                if (infoFlags & INFO_FLAGS_HITS) {
                                    sprintf( tempText,"With a burst of strength, you do TWICE as much damage!.");
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                            }
                        }

                        if (iBlade && BLADE_TYPE_CLAWS == iBlade->type)
                        {
                            if (8 == rand() % 10)
                            {
                                int resisted=curMonster->MonsterMagicEffect(MONSTER_EFFECT_STUN, damValue * 250.0f, 2);
                                additional_stun = damValue * 50;

                                if (infoFlags & INFO_FLAGS_HITS) {
									if (resisted==1)
										sprintf(tempText, "The %s resists stunning by %s's Claw!", curMonster->Name(), charInfoArray[curCharacterIndex].name);
									else
										sprintf(tempText, "The %s gets stunned by %s's Claw!", curMonster->Name(), charInfoArray[curCharacterIndex].name);
									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                            }
                        }

                        if (iBlade && BLADE_TYPE_DOUBLE == iBlade->type)
                        {
                            if (1 == rand() % 3)
                            {
                                DoBladestaffExtra(ss, iBlade, damValue, curMonster);
                            }
                        }
						if (iBlade && BLADE_TYPE_DOUBLE < iBlade->type && BLADE_TYPE_STAFF1 > iBlade->type) // for scythe
						{
							if (1 == rand() % 2) //50% chance to strike the square
							{
								if (iBlade->type != BLADE_TYPE_TAME_SCYTHE)              // not a taming scythe
									DoBladestaffExtra(ss, iBlade, (damValue / 2), curMonster); // do half damage to the rest of the square
							}
						}
						if (iBlade && (iBlade->numOfHits > 0) // since the removal is after this, only do them if numofhits > 1
							&& (iBlade->type > BLADE_TYPE_DOUBLE)  // don't bother with all this if it's not a scythe.
							&& (iBlade->type < BLADE_TYPE_STAFF1))
						{
							// get most common ingot;
							int mostingots = iBlade->tinIngots;
							if (iBlade->aluminumIngots > mostingots)
								mostingots = iBlade->aluminumIngots;
							if (iBlade->steelIngots > mostingots)
								mostingots = iBlade->steelIngots;
							if (iBlade->carbonIngots > mostingots)
								mostingots = iBlade->carbonIngots;
							if (iBlade->zincIngots > mostingots)
								mostingots = iBlade->zincIngots;
							if (iBlade->adamIngots > mostingots)
								mostingots = iBlade->adamIngots;
							if (iBlade->mithIngots > mostingots)
								mostingots = iBlade->mithIngots;
							if (iBlade->vizIngots > mostingots)
								mostingots = iBlade->vizIngots;
							if (iBlade->elatIngots > mostingots)
								mostingots = iBlade->elatIngots;
							if (iBlade->chitinIngots > mostingots)
								mostingots = iBlade->chitinIngots;
							if (iBlade->maligIngots > mostingots)
								mostingots = iBlade->maligIngots;
							if (iBlade->tungstenIngots > mostingots)
								mostingots = iBlade->tungstenIngots;
							if (iBlade->titaniumIngots > mostingots)
								mostingots = iBlade->titaniumIngots;
							if (iBlade->azraelIngots > mostingots)
								mostingots = iBlade->azraelIngots;
							if (iBlade->chromeIngots > mostingots)
								mostingots = iBlade->chromeIngots;
							// wood used for staff determines extra ingots in the base, so we calculate staff power
							int effectVal = 1;  // start at 1
							if (mostingots > 65)
								effectVal++;
							if (mostingots > 67)
								effectVal++;
							if (mostingots > 71)
								effectVal++;
							if (mostingots > 79)
								effectVal++;
							if (mostingots > 95)
								effectVal++;
							// now we have the staff quality equivalent with one added to it.
							// apply staff formula to translate that to actual staff power
							effectVal = 5 * effectVal + 3; // don't worry about imbue deviation, only perfect staffs will add charges.
														   // now that we know the staff equivalent, we can apply the staff effect
							if (iBlade->type == BLADE_TYPE_BLIND_SCYTHE) // if it can blind
							{ // do staff blind effect
								curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE,
									effectVal * 1000, effectVal);
							}
							if (iBlade->type == BLADE_TYPE_POISON_SCYTHE) // if it can poison
							{ // do staff poison effect
								curMonster->MonsterMagicEffect(DRAGON_TYPE_BLACK,
									effectVal * 1000, effectVal);
							}
							if (iBlade->type == BLADE_TYPE_SLOW_SCYTHE) // if it can slow
							{ // do staff slow effect
								curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE,
									effectVal * 1000, effectVal);
							}
							if (iBlade->type == BLADE_TYPE_STUN_SCYTHE) // if it can stun
							{ // do staff stun effect
								curMonster->MonsterMagicEffect(MONSTER_EFFECT_STUN,
									effectVal * 1000, effectVal);
							}
							if (iBlade->type == BLADE_TYPE_TANGLE_SCYTHE) // if it can root
							{ // do staff root effect
								curMonster->MonsterMagicEffect(MONSTER_EFFECT_BIND,
									effectVal * 1000, effectVal);
							}
							if ((iBlade->type == BLADE_TYPE_TAME_SCYTHE) && (BeastStat()>9)) // if it can tame and i'm a beastmaster.
							{ // let's try to tame the monster
								if (
									((iBlade->tame) >= (curMonster->defense)) // enouhg gold dust
									&& (curMonster->tamingcounter < 4)			// and not already tamed
									&& ((GetTamingLevel() > curMonster->type)  // and higher taming level
										||((GetTamingLevel() == curMonster->type) && (GetTamingExp()+1 >= (curMonster->subType))) // OR same taming level AND subtype is not more than 1 greater
										||((GetTamingLevel()+1 == (curMonster->type))&& (GetTamingExp()==5)&& (curMonster->subType==0)) // or it's the next type AND we have 5 exp and zero subtype
										)
									)
								{
									// bump up the taming counter
									// but NOT if it's stunned slowed, or blinded. 
									if ((curMonster->magicEffectAmount[2] <= 0) && (curMonster->magicEffectAmount[3] <= 0) && (curMonster->magicEffectAmount[7] <= 0))
									{
										// make sure it's NOT attacking another player! (other monsters are fine)
										if (curMonster->curTarget && curMonster->curTarget != this)// has a target and it's not me
										{
											if (curMonster->tamingcounter < 4)                       // only reset tamable monsters
												curMonster->tamingcounter = 0;
												// put error message that monster isn't tamable
												sprintf(tempText, "Cannot tame a monster attacking another player.");
												CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
												ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
												iBlade->numOfHits++; // refund charge
										}
										else
										{
											curMonster->tamingcounter++;
										}
									}
									else
									{
										// put error message that monster isn't tamable
										sprintf(tempText, "Can't tame a monster that is blinded, slowed, or stunned.");
										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
										iBlade->numOfHits++; // refund charge
									}
									if (curMonster->tamingcounter > 3) // currently activate on 4th swing
									{
										if (controlledMonster)         // if there's already one under control
										{
											// get name, skipping player name and whitespace
											sscanf(controlledMonster->uniqueName, "%*[^\']\'s %[^\n]", tempText); // skip before the 's, print after into
											char tempText2[1024];
											// place desired stabled item name into tempText2
											sprintf(tempText2, "Stabled %s", tempText);
											// search for existing stabled monster of same type 
											InventoryObject * iObject = (InventoryObject *) charInfoArray[curCharacterIndex].inventory->objects.First();
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
														if ((iStabledPet->mtype == controlledMonster->type) && (iStabledPet->subType == controlledMonster->subType))  // same type and subtype
														{
															found = TRUE; // we found a pet of the same type.
															// compare defense stats
															if (iStabledPet->defense < controlledMonster->defense) // if stabled pet's defense is lower
															{
																// then update it with the current
																iStabledPet->a = controlledMonster->a;
																iStabledPet->r = controlledMonster->r;
																iStabledPet->g = controlledMonster->g;
																iStabledPet->b = controlledMonster->b;
																iStabledPet->damageDone = controlledMonster->damageDone;
																iStabledPet->defense = controlledMonster->defense;
																iStabledPet->healAmountPerSecond = controlledMonster->healAmountPerSecond;
																iStabledPet->health = controlledMonster->maxHealth;
																iStabledPet->magicResistance = controlledMonster->magicResistance;
																iStabledPet->maxHealth = controlledMonster->maxHealth;
																iStabledPet->sizeCoeff = controlledMonster->sizeCoeff;
																iStabledPet->toHit = controlledMonster->toHit;
															}
															// either way, dispose of the object to exit the whiel loop;
															iObject = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.Last(); // go to the end
															iObject = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.Next(); // and one past in case we were already at the end
														}
														else
														{
															// it's a different type of pet, go to the next one
															iObject = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.Next();
														}
												
													}
													else
													{
														// it's a different name, go to the next one
														iObject = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.Next();
													}
												}
												else
												{
													// it's not a pet, next item
													iObject = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.Next();
												}
											}
											if (!found) // if we didn't find an item for my pet
											{
												// then we need to make a new one
												sprintf(tempText, "%s", tempText2); // was set above
												iObject = new InventoryObject(INVOBJ_STABLED_PET, 0, tempText);
												InvStabledPet * exSp = (InvStabledPet *)iObject->extra;

												iObject->mass = 0.0f;
												iObject->value = 1000;
												iObject->amount = 1;
												// copy the stats into it
												exSp->a = controlledMonster->a;
												exSp->b = controlledMonster->b;
												exSp->g = controlledMonster->g;
												exSp->r = controlledMonster->r;
												exSp->damageDone = controlledMonster->damageDone;
												exSp->defense = controlledMonster->defense;
												exSp->healAmountPerSecond = controlledMonster->healAmountPerSecond;
												exSp->health = controlledMonster->maxHealth;
												exSp->magicResistance = controlledMonster->magicResistance;
												exSp->maxHealth = controlledMonster->maxHealth;
												exSp->sizeCoeff = controlledMonster->sizeCoeff;
												exSp->subType = controlledMonster->subType;
												exSp->toHit = controlledMonster->toHit;
												exSp->mtype = controlledMonster->type;

												// and add it to the inventory list
												charInfoArray[curCharacterIndex].inventory->objects.Prepend(iObject);
											}
											// make it go away like you logged out 
											// die with no loot for anyone!
											for (int i = 0; i < 10; ++i)
												controlledMonster->attackerPtrList[i] = NULL;
											controlledMonster->dropAmount = 0; // dont' drop anything!
											controlledMonster->health = 0;
											controlledMonster->type = 25; //make it a vamp 
											controlledMonster->isDead = TRUE; // it's dead
											controlledMonster->controllingAvatar = NULL;	// and unlink it
											controlledMonster = NULL;	// and unlink it
										}
										// and, if needed, level up the taming skill
										if (curMonster->type <= (GetTamingLevel()+1))   // if  this is the monster type we were on
										{											
											io = (InventoryObject *)
												charInfoArray[curCharacterIndex].skills->objects.First();
											while (io)
											{
												if (!strcmp("Taming", io->WhoAmI()))
												{
														InvSkill *skillInfo = (InvSkill *)io->extra;  // get extra data
														// up the subtype if needed
														if ((curMonster->type==GetTamingLevel()) &&((GetTamingExp()+1) == curMonster->subType)) // test for same type as level and one sub higher than exp
															skillInfo->skillPoints++;
														// next check if the new subtype actually exists
														// first, check for skipped monsters
														// normal spidren doesn't exist outside of geo, so we skip
														if (!strcmp(monsterData[GetTamingLevel()][GetTamingExp()+1].name, "Spidren"))
															skillInfo->skillPoints++;
														// albino bat doesn't exist outside of geo, so we skip
														if (!strcmp(monsterData[GetTamingLevel()][GetTamingExp()+1].name, "Albino Bat"))
															skillInfo->skillPoints++;
														// Typhon doesn't exist outside of Tower of Infinity, so we skip
														if (!strcmp(monsterData[GetTamingLevel()][GetTamingExp()+1].name, "Typhon"))
															skillInfo->skillPoints++;
														// Lizard King doesn't exist outside of Tower of Infinity, so we skip
														if (!strcmp(monsterData[GetTamingLevel()][GetTamingExp()+1].name, "Lizard King"))
															skillInfo->skillPoints++;
														// next check the next monster has a name
														if ((GetTamingExp() < 5)&&(monsterData[GetTamingLevel()][GetTamingExp()+1].name[0] == '\0')) // if we aren't already at 5 AND it doesn't have a name
														{
															skillInfo->skillPoints=5; // skip to 5 if not there already. 5 means we can attempt the NEXT one.
														}
														// and finally do a bumpup if it's the next type
														if ((curMonster->type) == (GetTamingLevel()+1))	// monster type is one higher
															skillInfo->skillPoints++;					// then increment from 5 to six.  This works because if we actually incremented the taming counter on a type
																										// one higher, then we were already at 5.
																										// what a horrible hack, but it should work.
																										
														if (skillInfo->skillPoints > 5)	// no subtype greater than 5 exists
														{
															skillInfo->skillLevel++;					//increment taming skill 
															skillInfo->skillPoints = 0;					// and reset skillpoints to start over on next subtype.
															sprintf(tempText, "You gained Taming skill!!");
															CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
															ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
															// currently no CLVL for taming but that may change later.
														}
														// check AGAIN for next monster not existing.
														if ((GetTamingExp() < 5) && (monsterData[GetTamingLevel()][GetTamingExp() + 1].name[0] == '\0')) // if we aren't already at 5 AND it doesn't have a name
														{
															skillInfo->skillPoints = 5; // skip to 5 if not there already. 5 means we can attempt the NEXT one.
														}
												}
												io = (InventoryObject *)
													charInfoArray[curCharacterIndex].skills->objects.Next();  // go to next skill regardless.
											}

										}
										// stop attacking
										this->curTarget = NULL;
										// stop monster from atacking.
										curMonster->curTarget = NULL;
										curMonster->curMonsterTarget = NULL;
										
										// now, control the monster, if it's not exempt from taming.  will add checks later 
										if (true)
										{
											controlledMonster = curMonster;					// control it
											controlledMonster->controllingAvatar = this;    // point it's record to me
											// rename it
											sprintf(tempText, "%s's %s", this->charInfoArray[this->curCharacterIndex].name,
												monsterData[controlledMonster->type][controlledMonster->subType].name);
											CopyStringSafely(tempText, 1024,
												controlledMonster->uniqueName, 31);
															// check if it has a generator 
											if (controlledMonster->myGenerator)
											{
												if (controlledMonster->myGenerator->do_id == 0) // if it's an actual generator
												{
													// then we need to decrease the counter so the monster will get replaced next spawn run.
													controlledMonster->myGenerator->count[controlledMonster->type][controlledMonster->subType]--;
												}
												else
												{
													// make adjustments and force player offline.
													if (controlledMonster->damageDone < 4)  // controlled monsters that weak can't kill.
														controlledMonster->damageDone = 4;  // do at least enough to break default regen of 1 per second.
													controlledMonster->healAmountPerSecond = 1; // nerf regen upon taming.
													controlledMonster->myGenerator = NULL; // detach from armies and spawners
													if (controlledMonster->type == 21)		// if you are taming the thieving spirit
													{
														controlledMonster->thiefMode = THIEF_NOT_THIEF; // tamed thieving spirits can no longer steal.
													}
													this->SaveAccount(); // store monster
													kickOff=TRUE;         //this will make sure it goes poof and gets properly replaced
													return;									// stop the tick, jutst in case
													controlledMonster->dontRespawn = TRUE;
												}
												// if it was an army instead, the army code handles that
											}
											controlledMonster->myGenerator = NULL; // detach from armies and spawners
																							  // this prevents a double decrement of the counter (foor the generator)
																							  // and will cause the army IsValidMonster function to return false, so it will get replaced.
											if ((controlledMonster->type == 16) && (controlledMonster->subType == 1) && !controlledMonster->dontRespawn) // if we just tamed Dokk
											{
												// create a grave so he will return.
												BBOSMonsterGrave *mg = new BBOSMonsterGrave(
													controlledMonster->type, controlledMonster->subType,
													controlledMonster->spawnX, controlledMonster->spawnY);
												mg->isWandering = FALSE;
												ss->mobList->Add(mg);
											}
											if ((controlledMonster->type == 11) && (controlledMonster->subType == 1) && !controlledMonster->dontRespawn) // if we just tamed a Cent
											{
												// create a grave so it will return
												BBOSMonsterGrave *mg = new BBOSMonsterGrave(
													controlledMonster->type, controlledMonster->subType,
													controlledMonster->spawnX, controlledMonster->spawnY);
												mg->isWandering = FALSE;
												ss->mobList->Add(mg);
											}
											if ((controlledMonster->type == 27) && !controlledMonster->dontRespawn) // if we just tamed a vampire lord
											{
												// create a grave so it will return
												BBOSMonsterGrave *mg = new BBOSMonsterGrave(
													controlledMonster->type, controlledMonster->subType,
													controlledMonster->spawnX, controlledMonster->spawnY);
												// set proper respawn time
												mg->spawnTime = timeGetTime() + 1000 * 60 * 60 * 4; // 4 hours
												mg->isWandering = FALSE;
												ss->mobList->Add(mg);
											}
											if ((controlledMonster->type == 30) && (controlledMonster->subType > 1) && !controlledMonster->dontRespawn) // if we just tamed a boss butterfly
											{
												// create a grave so it will return
												BBOSMonsterGrave *mg = new BBOSMonsterGrave(
													controlledMonster->type, controlledMonster->subType,
													controlledMonster->spawnX, controlledMonster->spawnY);
												mg->isWandering = FALSE;
												ss->mobList->Add(mg);
											}

											if (controlledMonster->damageDone < 4)  // controlled monsters that weak can't kill.
												controlledMonster->damageDone = 4;  // do at least enough to break default regen of 1 per second.
											controlledMonster->healAmountPerSecond = 1; // nerf regen upon taming.
											followMode = TRUE;						// and make it follow me.
											if (controlledMonster->type == 21)		// if you are taming the thieving spirit
											{
												controlledMonster->thiefMode = THIEF_NOT_THIEF; // tamed thieving spirits can no longer steal.
											}

											controlledMonster->healAmountPerSecond = 1; // nerf regen upon taming.
											controlledMonster->AnnounceMyselfCustom(ss);
											controlledMonster->dontRespawn = TRUE;
											this->SaveAccount();
										}
										else
										{
											// put error message that monster isn't tamable
											sprintf(tempText, "This can't happen.");
											CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
											ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
											iBlade->numOfHits++; // refund charge
										}
									}
								}
								else
								{
									// put error message that monster isn't tamable
									if (curMonster->tamingcounter > 3) // already tamed, or protected
									{
										sprintf(tempText, "This monster is untamable.");
									}
									else if (iBlade->tame < curMonster->defense)		// not enough golds
									{
										sprintf(tempText, "You have insufficient Gold Dusts to tame this monster.");
									}
									else												// if i get here, taming skill 
									{
										sprintf(tempText, "Your Taming skill is too low to tame this monster.");
									}
									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									iBlade->numOfHits++; // refund charge
								}
							}
							if ((iBlade->type == BLADE_TYPE_TAME_SCYTHE) && (BeastStat() < 10)) // if you aren't actually a beastmaster
							{

								sprintf(tempText, "You aren't a Beastmaster, don't try to use this.");

								CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
								ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

								iBlade->numOfHits++; // refund charge
							}
						}

                        if (totemEffects.effect[TOTEM_LIFESTEAL] > 0)
                        {
                            int suck = damValue * totemEffects.effect[TOTEM_LIFESTEAL] / 40;
                            if (suck > charInfoArray[curCharacterIndex].healthMax/4)
                                suck = charInfoArray[curCharacterIndex].healthMax/4;
                            damValue += suck;

                            charInfoArray[curCharacterIndex].health += suck;

                            if (charInfoArray[curCharacterIndex].health >
                                        charInfoArray[curCharacterIndex].healthMax)
                                charInfoArray[curCharacterIndex].health =
                                        charInfoArray[curCharacterIndex].healthMax;
                        }

                        // Add mastery damage
                        if( iBlade )
                            damValue += (GetMasteryForType( iBlade->type ) * charInfoArray[curCharacterIndex].physical / 2 );
						// cap actual done damage for taming scythe
						if (iBlade && (iBlade->type == BLADE_TYPE_TAME_SCYTHE)&& damValue>1)
							damValue = 1;  // needs to not kill stuff.
						
                        curMonster->health -= damValue;
                        curMonster->RecordDamageForLootDist(damValue, this);

						// get dust total to check if shard elibible
						int dustTotal = 0;
						if (iBlade) 
						{
							dustTotal += iBlade->blind;
							dustTotal += iBlade->slow;
							dustTotal += iBlade->heal;
							dustTotal += iBlade->poison;
							dustTotal += iBlade->tame;
							int blueshack = iBlade->toHit - 1;
							if (iBlade->type == BLADE_TYPE_KATANA)
								blueshack -= 20;
							dustTotal += blueshack;  // don't penalize total dust count for not using a tachi.
						}
						// age the blade
						if (iBlade)
						{
							if (iBlade->numOfHits < 259501) //
									++(iBlade->numOfHits);
						}
						// revert age if it can't have a shard and not cheating.
						if (iBlade && iBlade->numOfHits > 22000)
						{
							if ((dustTotal < 5) || (iBlade->damageDone < 72))
							{
								iBlade->numOfHits = 22000;
							}
						}
						// undo and subtract for a scythe.
						if (iBlade &&  BLADE_TYPE_DOUBLE < iBlade->type && BLADE_TYPE_STAFF1 > iBlade->type)
						{
							--(iBlade->numOfHits);
							--(iBlade->numOfHits);
							if (iBlade->numOfHits < 0) // must have been zero before the swing, and one after.
								++(iBlade->numOfHits); // so it's -1, increase to 0.
						}
						if (iBlade && iBlade->type < BLADE_TYPE_SCYTHE // it's not a scythe
							&& ((iBlade->type == BLADE_TYPE_MACE && iBlade->damageDone > 101) // it's a mace with dam 102 or greater
								|| iBlade->damageDone > 71))// or somethtign else with dam 72 or greater
						{
							// then count the dusts
							int dustTotal = 0;
							dustTotal += iBlade->blind;
							dustTotal += iBlade->slow;
							dustTotal += iBlade->heal;
							dustTotal += iBlade->poison;
							dustTotal += iBlade->tame;
							int blueshack = iBlade->toHit - 1;
							if (iBlade->type == BLADE_TYPE_KATANA)
								blueshack -= 20;
							dustTotal += blueshack;  // don't penalize total dust count for not using a tachi.
							if (dustTotal>4) // if it has enough dusts on it
							{
								// then it could ahve a shard chance, check for particle effect
								int doparticle = 0;
								if (iBlade->numOfHits > 22000 && ((iBlade->numOfHits - 22000) % 2500 == 0)) // particle for every percentage added
									doparticle = 1;
								if (iBlade->numOfHits == 22000)												// initial partical
									doparticle = 2;
								if (iBlade->numOfHits == 110000)											// old cap for normal
									doparticle = 3;
								if (iBlade->numOfHits == 259500)											// 100%
									doparticle = 10;
								if (doparticle > 0)
								{
									// pretty particle effect!
									BBOSGroundEffect *bboGE = new BBOSGroundEffect();
									bboGE->type = 4;
									bboGE->amount = 20*doparticle;
									bboGE->r = 20;
									bboGE->g = 60;
									bboGE->b = 128;
									BBOSAvatar * curAvatar = this; // reference myself
									bboGE->cellX = curAvatar->cellX;
									bboGE->cellY = curAvatar->cellY;
									bboGE->killme = 400; // make it go away after a small bit.
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
										curAvatar->cellX, curAvatar->cellY,
										sizeof(messGE), &messGE);
								} // whew!
							}
						}
                        // give experience for monster damage!

//                        if (bladeHeal > 0)
//                        {
//                            // green dust heals the wielder.. if it wasn't disabled.
//                            charInfoArray[curCharacterIndex].health += bladeHeal;
//
//                            if (charInfoArray[curCharacterIndex].health >
//                                        charInfoArray[curCharacterIndex].healthMax)
//                            {
//                                charInfoArray[curCharacterIndex].health =
//                                        charInfoArray[curCharacterIndex].healthMax;
//                            }
//                        }
						if (iBlade && (iBlade->type != BLADE_TYPE_TAME_SCYTHE)) // if not a scythe
						{
							/// check for shock.
							if (bladeShock > 0) {
								int cnt = iBlade->GetIngotCount(); // safe because bladeshock can't be greater than 0 if iblade doesn't exist.

								if (cnt > 96)
									cnt = 96;

								if (rand() % 144 < cnt && (bladeShock / 2)) {
									int resisted = curMonster->MonsterMagicEffect(MONSTER_EFFECT_STUN, bladeShock * 500.0f + additional_stun, 1.0f);

									curMonster->health -= (bladeShock / 2);

									if (infoFlags & INFO_FLAGS_HITS) {
										if (resisted == 1)
											sprintf(tempText, "The %s gets shocked by %d lightning damage but resisted the stunning effect.",
												curMonster->Name(), (bladeShock / 2));
										else
											sprintf(tempText, "The %s gets shocked by %d lightning damage and is stunned.",
												curMonster->Name(), (bladeShock / 2));

										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									}
								}
							}
						}

                        if (curMonster->health <= 0)
                        {
							// if killed by scythe, set extra meat drop chance
							if (iBlade &&  BLADE_TYPE_DOUBLE < iBlade->type && iBlade && BLADE_TYPE_STAFF1 > iBlade->type)
								curMonster->MoreMeat=true;
                            MessAvatarAttack messAA;
                            messAA.avatarID = socketIndex;
                            messAA.mobID    = (long) curMonster;
                            messAA.damage   = damValue;
                            messAA.health   = -1000;
                            ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);

                            curMonster->isDead = TRUE;
                            curMonster->bane = this;
							if (curMonster->magicEffectAmount[MONSTER_EFFECT_MORE_LOOT]>0.0f)
							{
								sprintf(tempText, "Loot falls from the sky onto the %s's corpse!",
									curMonster->Name());
								CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
								ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								curMonster->dropAmount = (int)(curMonster->dropAmount*1.1f);
							}

                            curMonster->HandleQuestDeath();

                            if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
                            {
                                MapListState oldState = ss->mobList->GetState();

                                BBOSArmy *army = (BBOSArmy *)	curMonster->myGenerator;
                                army->MonsterEvent(curMonster, ARMY_EVENT_DIED);

                                ss->mobList->SetState(oldState);
                            }
                            else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
                            {
                                MapListState oldState = ss->mobList->GetState();

                                BBOSAutoQuest *quest = (BBOSAutoQuest *) curMonster->myGenerator;
                                quest->MonsterEvent(curMonster, AUTO_EVENT_DIED);

                                ss->mobList->SetState(oldState);
                            }
                        }
                        else
                        {
                            if (!(curMonster->curTarget) || 
                                 (24 == curMonster->type && 3 == (rand() % 10))
                                )
                            {
                                curMonster->lastAttackTime = now;
                                curMonster->curTarget = this;
                            }

                            if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
                            {
                                MapListState oldState = ss->mobList->GetState();

                                BBOSArmy *army = (BBOSArmy *)	curMonster->myGenerator;
                                army->MonsterEvent(curMonster, ARMY_EVENT_ATTACKED);

                                ss->mobList->SetState(oldState);
                            }
                            else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
                            {
                                MapListState oldState = ss->mobList->GetState();

                                BBOSAutoQuest *quest = (BBOSAutoQuest *) curMonster->myGenerator;
                                quest->MonsterEvent(curMonster, AUTO_EVENT_ATTACKED);

                                ss->mobList->SetState(oldState);
                            }

                            MessAvatarAttack messAA;
                            messAA.avatarID = socketIndex;
                            messAA.mobID    = (long) curMonster;
                            messAA.damage   = damValue;
                            messAA.health   = charInfoArray[curCharacterIndex].health;
                            messAA.healthMax= charInfoArray[curCharacterIndex].healthMax;
                            ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);

                            MessMonsterHealth messMH;
                            messMH.mobID = (unsigned long)curMonster;
                            messMH.health = curMonster->health;
                            messMH.healthMax = curMonster->maxHealth;
                            ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messMH),(void *)&messMH, 2);
							
							if (iBlade && (iBlade->type != BLADE_TYPE_TAME_SCYTHE) && (bladeSlow > 0))
							{
								// blue dragons slow the target.
								int chance = bladeSlow;

								if (chance + (rand() % 20) - curMonster->defense > 10)
								{
									int resisted = curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE,
										bladeSlow * 1000.0f, bladeSlow);

									if (infoFlags & INFO_FLAGS_HITS)
									{
										if (resisted == 1)
											sprintf(tempText, "The %s resisted the slowing effect.",
												curMonster->Name());
										else
											sprintf(tempText, "The %s is Slowed.",
												curMonster->Name());
										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									}
								}
							}
							if (bladeHeal > 0)
							{
								// green dusts can cause the monster to drop more simple loot.
								int chance = bladeHeal;

								if (chance + (rand() % 20) - curMonster->defense > 10)
								{
									curMonster->MonsterMagicEffect(MONSTER_EFFECT_MORE_LOOT,
										bladeHeal * 1000.0f, bladeHeal);

								}
							}

                            
                            if (bladePoison > 0)
                            {  // chance doesn't matter. instead it works against excess resistance.
                                    curMonster->MonsterMagicEffect(MONSTER_EFFECT_RESIST_LOWER, 
                                        bladePoison * 1000.0f, bladePoison);

                                    curMonster->RecordDamageForLootDist(bladePoison * bladePoison / 40, this);

                                    if (infoFlags & INFO_FLAGS_HITS)
                                    {
                                        sprintf(tempText,"The %s is more vulnerable to other dusts.",
                                            curMonster->Name());
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                            }
							if (iBlade && (iBlade->type != BLADE_TYPE_TAME_SCYTHE) && (bladeBlind > 0))
                            {
                                int chance = bladeBlind;

                                if (chance + (rand() % 20) - curMonster->defense > 10)
                                {
                                    int resisted=curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE, 
                                        bladeBlind * 1000.0f, bladeBlind);

                                    if (infoFlags & INFO_FLAGS_HITS)
                                    {
										if (resisted==1)
											sprintf(tempText, "The %s resisted the blinding effect.",
												curMonster->Name());
										else
											sprintf(tempText, "The %s is Blinded.",
												curMonster->Name());

										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                                }
                            }
                        }

                    }
                    else
                    {
                        // whish!
                        MessAvatarAttack messAA;
                        messAA.avatarID = socketIndex;
                        messAA.mobID    = (long) curMonster;
                        messAA.damage   = -1; // miss!
                        messAA.health   = charInfoArray[curCharacterIndex].health;
                        messAA.healthMax= charInfoArray[curCharacterIndex].healthMax;
                        ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);
                    }
                }
            }
            curMob = ss->mobList->GetNext();
        }

        if (-3 != usedStaff.charges)  // if previous section was skipped, so is this one, because usedStaff.charges hasn't changed from -3;
        {
            curMob = ss->mobList->GetFirst(cellX, cellY, staffRange);
            while (curMob)
            {
                curMonster = (BBOSMonster *) curMob;

                if (SMOB_MONSTER == curMob->WhatAmI() &&
                     abs(cellX - curMob->cellX) <= staffRange &&
                     abs(cellY - curMob->cellY) <= staffRange)
                   UseStaffOnMonster(ss, &usedStaff, curMonster);

                curMob = ss->mobList->GetNext();
            }
        }

        if (!didAttack)
        {
            curTarget = NULL;
        }

    }
	// attack current PLAYER target if allowed
	if (delta > secondsforattack * 2 + magicEffectAmount[DRAGON_TYPE_BLUE] * 300 && curPlayerTarget) //
	{
		// check pvpflag of myself and target
		if (PVPEnabled && curPlayerTarget->PVPEnabled) // both should be enabled if one is, but need to make SURE.
		{
			// then we can attack 
			AttackPlayer(ss, curPlayerTarget);  // function to be written later.  will deal with staffs and weapons.
		}
	}
	// handle drake attakc routine
    for (int index = 0; index < 2; ++index)
    {
#pragma warning(push)
#pragma warning(disable:6385)		//false positive, curCharacterIndex is ALWAYS in range
		PetDragonInfo *dInfo = &charInfoArray[curCharacterIndex].petDragonInfo[index];
#pragma warning(pop)

        delta = now - dInfo->lastAttackTime;

        if (0 == dInfo->lastAttackTime || now < dInfo->lastAttackTime) // || now == moveStartTime)
        {
            delta = 1000 * 3 + 1;
        }

        // attack curTarget (if it's still there). they won't attack players, because they are friendly to them. :)
        if (255 != dInfo->type && delta > 1000 * 3 && curTarget &&
             !(dm && dm->CanEdit(this) && 0 == curTarget->form)
            )	
        {
            dInfo->lastAttackTime = now - rand() % 200;
            int didAttack = FALSE;

            curMob = ss->mobList->GetFirst(cellX, cellY);
            while (curMob && !didAttack)
            {
                if (!curMob->isDead && curMob == curTarget && 
                      curMob->cellX == cellX && curMob->cellY == cellY && 
                      SMOB_MONSTER == curMob->WhatAmI()) 
                {
                    didAttack = TRUE;

                    curMonster = (BBOSMonster *) curMob;

                    long damValue = (dInfo->lifeStage+1) * (dInfo->quality+1) * 
                        dragonInfo[dInfo->quality][dInfo->type].attackDamageBase / 5 +
                        dInfo->healthModifier * (dInfo->lifeStage+1); 

					// check for Drake Mastery
					bool dfound = FALSE;
					InventoryObject * Dio = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.First();
					while (Dio && !dfound) {
						if (!strcmp("Drake Mastery", Dio->WhoAmI())) 
						{
							InvSkill *dSkill = (InvSkill *)Dio->extra;
							dfound = TRUE;
							dSkill->skillPoints += 1* bboServer->mastery_exp_multiplier;
							if (dSkill->skillLevel < 100 && dSkill->skillLevel * 100000 <= dSkill->skillPoints) {
								std::vector<TagID> tempReceiptList;

								tempReceiptList.clear();
								tempReceiptList.push_back(socketIndex);

								// made a skill level!!!
								dSkill->skillLevel += 1;

								sprintf(tempText, "You gained Drake Mastery skill!! You feel more in tune with your drakes.");
								CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

								ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							}
							damValue += dSkill->skillLevel*MagicalStat() / 2; // add the damage.
						}

						Dio = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.Next();
					}
					curMonster->health -= damValue;
					curMonster->RecordDamageForLootDist(damValue, this);

                    if (!(curMonster->curTarget) || 
                                 (24 == curMonster->type && 3 == (rand() % 10))
                         )
                    {
                        curMonster->lastAttackTime = now;
                        curMonster->curTarget = this;
                    }

                    // give experience for monster damage!
                    int breathType = dInfo->type;
                    if (6 == breathType)
                        breathType = rand() % 6;

                    MessPetAttack messAA;
                    messAA.avatarID = socketIndex;
                    messAA.mobID    = (long) curMonster;
                    messAA.damage   = damValue;
                    messAA.which    = index;
                    messAA.type     = breathType;
                    messAA.effect   = 0;
                    ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);

                    MessMonsterHealth messMH;
                    messMH.mobID = (unsigned long)curMonster;
                    messMH.health = curMonster->health;
                    messMH.healthMax = curMonster->maxHealth;
                    ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messMH),(void *)&messMH, 2);

                    if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
                    {
                        MapListState oldState = ss->mobList->GetState();

                        BBOSArmy *army = (BBOSArmy *)	curMonster->myGenerator;
                        army->MonsterEvent(curMonster, ARMY_EVENT_ATTACKED);

                        ss->mobList->SetState(oldState);
                    }
                    else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
                    {
                        MapListState oldState = ss->mobList->GetState();

                        BBOSAutoQuest *quest = (BBOSAutoQuest *) curMonster->myGenerator;
                        quest->MonsterEvent(curMonster, AUTO_EVENT_ATTACKED);

                        ss->mobList->SetState(oldState);
                    }

                    if (curMonster->health <= 0)
                    {
                        curMonster->isDead = TRUE;
                        curMonster->bane = this;
                        curMonster->HandleQuestDeath();
						if (curMonster->magicEffectAmount[MONSTER_EFFECT_MORE_LOOT]>0.0f)
						{
							sprintf(tempText, "Loot falls from the sky onto the %s's corpse!",
								curMonster->Name());
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							curMonster->dropAmount = (int)(curMonster->dropAmount*1.1f);
						}

                        // give experience for monster death!
                        if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
                        {
                            MapListState oldState = ss->mobList->GetState();

                            BBOSArmy *army = (BBOSArmy *)	curMonster->myGenerator;
                            army->MonsterEvent(curMonster, ARMY_EVENT_DIED);

                            ss->mobList->SetState(oldState);
                        }
                        else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
                        {
                            MapListState oldState = ss->mobList->GetState();

                            BBOSAutoQuest *quest = (BBOSAutoQuest *) curMonster->myGenerator;
                            quest->MonsterEvent(curMonster, AUTO_EVENT_DIED);

                            ss->mobList->SetState(oldState);
                        }
                    }
                    else if (DRAGON_TYPE_GREEN == breathType)
                    {
                        // green dragons heal their owner.
                        charInfoArray[curCharacterIndex].health += 
                            (dInfo->lifeStage+1) * (dInfo->quality+1) / 2 +
                        dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (charInfoArray[curCharacterIndex].health >
                                    charInfoArray[curCharacterIndex].healthMax)
                        {
                            charInfoArray[curCharacterIndex].health =
                                    charInfoArray[curCharacterIndex].healthMax;
                        }
                    }
                    else if (DRAGON_TYPE_BLUE == breathType)
                    {
                        // blue dragons slow the target.
                        int chance = (dInfo->lifeStage+1) * (dInfo->quality+1) +
                            dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (chance + (rand() % 20) - curMonster->defense > 10)
                        {
                            curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE, 
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 2000.0f,
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 1.0f);

                            if (infoFlags & INFO_FLAGS_HITS)
                            {
                                sprintf(tempText,"%s slows the %s.",
                                    dInfo->name, curMonster->Name());
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }
                    }
                    else if (DRAGON_TYPE_BLACK == breathType)
                    {
                        // black dragons poison the target.
                        int chance = (dInfo->lifeStage+1) * (dInfo->quality+1) +
                            dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (chance + (rand() % 20) - curMonster->defense > 10)
                        {
                            curMonster->MonsterMagicEffect(DRAGON_TYPE_BLACK, 
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 2000.0f,
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 1.0f);

                            curMonster->RecordDamageForLootDist(
                                          (dInfo->lifeStage+1) * (dInfo->quality+1) / 20, this);

                            if (infoFlags & INFO_FLAGS_HITS)
                            {
                                sprintf(tempText,"%s poisons the %s.",
                                    dInfo->name, curMonster->Name());
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }
                    }
                    else if (DRAGON_TYPE_WHITE == breathType)
                    {
                        // white dragons blind the target.
                        int chance = (dInfo->lifeStage+1) * (dInfo->quality+1) +
                            dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (chance + (rand() % 20) - curMonster->defense > 10)
                        {
                            curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE, 
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 2000.0f,
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 1.0f);

                            if (infoFlags & INFO_FLAGS_HITS)
                            {
                                sprintf(tempText,"%s blinds the %s.",
                                    dInfo->name, curMonster->Name());
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }
                    }
                    else if (DRAGON_TYPE_GOLD == breathType)
                    {
                        // heal their owner.
                        charInfoArray[curCharacterIndex].health += 
                            (dInfo->lifeStage+1) * (dInfo->quality+1) / 2 +
                        dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (charInfoArray[curCharacterIndex].health >
                                    charInfoArray[curCharacterIndex].healthMax)
                        {
                            charInfoArray[curCharacterIndex].health =
                                    charInfoArray[curCharacterIndex].healthMax;
                        }

                        // blind the target.
                        int chance = (dInfo->lifeStage+1) * (dInfo->quality+1) / 1.0f +
                            dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (chance + (rand() % 20) - curMonster->defense > 10)
                        {
                            curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE, 
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 1000 * 2,
                                (dInfo->lifeStage+1) * (dInfo->quality+1) );

                            if (infoFlags & INFO_FLAGS_HITS)
                            {
                                sprintf(tempText,"%s blinds the %s.",
                                    dInfo->name, curMonster->Name());
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }

                        // slow the target.
                        chance = (dInfo->lifeStage+1) * (dInfo->quality+1) / 1.0f +
                            dInfo->healthModifier * (dInfo->lifeStage+1); 

                        if (chance + (rand() % 20) - curMonster->defense > 10)
                        {
                            curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE, 
                                (dInfo->lifeStage+1) * (dInfo->quality+1) * 1000 * 2,
                                (dInfo->lifeStage+1) * (dInfo->quality+1) );

                            if (infoFlags & INFO_FLAGS_HITS)
                            {
                                sprintf(tempText,"%s slows the %s.",
                                    dInfo->name, curMonster->Name());
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                        }
                    }
                }
                curMob = ss->mobList->GetNext();
            }

        }
    }

    delta = now - lastSaveTime;

    if (0 == lastSaveTime || now < lastSaveTime)
    {
        lastSaveTime = timeGetTime();
        delta = 1;
    }

    // save character every 5 minutes
    if (delta > 1000 * 60 * 5)	
    {
        lastSaveTime = now;
        if (SPACE_GROUND != ss->WhatAmI())
        {
            charInfoArray[curCharacterIndex].lastX = charInfoArray[curCharacterIndex].spawnX;
            charInfoArray[curCharacterIndex].lastY = charInfoArray[curCharacterIndex].spawnY;
        }
        else
        {
            charInfoArray[curCharacterIndex].lastX = cellX;
            charInfoArray[curCharacterIndex].lastY = cellY;
        }

        ++(charInfoArray[curCharacterIndex].lifeTime);
        ++activeCounter;

        if (!tradingPartner && bHasLoaded) // postpone saving until the transaction is finished
            SaveAccount();

        HandleMeatRot(ss);
        PetAging(ss);

        QuestTime(ss);

        ++timeOnCounter;


        /*
        NO LONGER REQUIRING ACCOUNT PAYMENTS

        LongTime rightNow;
        long timeDiff = rightNow.MinutesDifference(&accountExperationTime);
        if (timeDiff <= 0 && timeOnCounter > 1)
            kickMeOffNow = 1;
        else if (timeDiff <= 60 * 24 && 1 == timeOnCounter % 6)
        {
            sprintf(tempText,"Your account will expire in %ld minutes.",timeDiff);
            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            sprintf(tempText,"Go to www.blademistress.com to purchase a code for more time.");
            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

        }
        */
    }
    
    if (chantType > -1)
    {
        delta = now - chantTime;

        // stop chanting
        if (delta > 1000 * 10) // 10 seconds	
        {
            chantType = -1;

            MessChant messChant;
            messChant.avatarID = socketIndex;
            messChant.r = messChant.b = messChant.g = 0;	 // black means stop
            ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messChant), &messChant);
        }
    }

    // ********healing
    delta = now - lastHealTime;

    if (0 == lastHealTime || now < lastHealTime)
    {
        delta = 1000 * 5 + 1;
    }


    // heal damage every 5 seconds
    if (delta > 1000 * 5)	
    {
        float tHeal = 0.0f;
            
        if (curTarget)
        {
            tHeal = 0.3f * totemEffects.effect[TOTEM_HEALING];

            if (tHeal > charInfoArray[curCharacterIndex].healthMax / 4.0f)
                tHeal = charInfoArray[curCharacterIndex].healthMax / 4.0f;
        }
        else
            tHeal = charInfoArray[curCharacterIndex].healthMax / 6.0f;

        lastHealTime = now;
        charInfoArray[curCharacterIndex].health += 0.7f + tHeal;
        if (charInfoArray[curCharacterIndex].health >
             charInfoArray[curCharacterIndex].healthMax)
            charInfoArray[curCharacterIndex].health = charInfoArray[curCharacterIndex].healthMax;

        MessAvatarHealth messHealth;
        messHealth.health    = charInfoArray[curCharacterIndex].health;
        messHealth.healthMax = charInfoArray[curCharacterIndex].healthMax;
        messHealth.avatarID  = socketIndex;
        ss->lserver->SendMsg(sizeof(messHealth),(void *)&messHealth, 0, &tempReceiptList);
		// and also heal monster if it's out of combat.
		if (controlledMonster && !(controlledMonster->curMonsterTarget) && !(controlledMonster->curTarget) && (BeastStat() >9)) // no target and a beastmaster
		{
			// same out of combat heal rate as players         
			tHeal = controlledMonster->maxHealth / 4.0; // 1/4th the max health
			controlledMonster->health += tHeal;
			if (controlledMonster->health > controlledMonster->maxHealth)
				controlledMonster->health = controlledMonster->maxHealth;  // don't go over max.
			MessMonsterHealth messMH;
			messMH.mobID = (unsigned long)controlledMonster;
			messMH.health = controlledMonster->health;
			messMH.healthMax = controlledMonster->maxHealth;
			ss->SendToEveryoneNearBut(0, (float)controlledMonster->cellX, (float)controlledMonster->cellY, sizeof(messMH), (void *)&messMH, 2); // send to all players near it's square.

		}
		// give out all pending quest rewards.
        while (QuestReward(ss))
            ;

    }

    //********** 10 second tasks
    delta = now - lastTenTime;

    if (0 == lastTenTime || now < lastTenTime)
    {
        delta = 1000 * 10 + 1;
    }

    // time to do a ten second task
    if (delta > 1000 * 10)	
    {
        lastTenTime = now;

        for (int i = 0; i < MONSTER_EFFECT_TYPE_NUM; ++i)
        {
            if (magicEffectAmount[i] > 0)  // make magic effects expire
            {
                if (magicEffectTimer[i] < now) // if expired
                    magicEffectAmount[i] = 0;  // zero the amount field
                else if (MONSTER_EFFECT_TYPE_BLACK == i) // poison effect is triggered by time. didn't work, i had to fix it.
                {
					charInfoArray[curCharacterIndex].health -= magicEffectAmount[i] / 2.0f;

                    MessMonsterHealth messMH;
                    messMH.mobID = (unsigned long)this;
                    messMH.health = charInfoArray[curCharacterIndex].health;
                    messMH.healthMax = charInfoArray[curCharacterIndex].healthMax;
                    ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH),(void *)&messMH, 2);

                    if (charInfoArray[curCharacterIndex].health <= 0)
                    {
						charInfoArray[curCharacterIndex].health = 0;
                        isDead = TRUE;
                    }
                }

            }
        }

        // find out if there's another player on with my exact name
        BBOSAvatar *other = (BBOSAvatar *)ss->avatars->First();
        {
            if (other != this)
            {
                if (IsCompletelyVisiblySame(other->name, name))
                {
                    kickOff = TRUE;                             // no double logins!
                    other->kickOff = TRUE;						// this shouldn't be possible but just in case...
                }
            }
            other = (BBOSAvatar *)ss->avatars->Next();
        }

        ss->avatars->Find(this);

        // piggy-back the admin info message on healing
        if (ACCOUNT_TYPE_ADMIN == accountType)
        {
            MessAdminInfo aInfo;
            DWORD timeSinceConnect = (timeGetTime() - bboServer->lastConnectTime) / 1000 / 60;
            aInfo.numPlayers = lastAvatarCount;
            aInfo.lastConnectTime = timeSinceConnect;
            ss->lserver->SendMsg(sizeof(aInfo),(void *)&aInfo, 0, &tempReceiptList);
        }
    }
    // no more every ten seconds tasks for avatars.
	// handle starting of combine action.
    if (isCombining)
    {
        delta = now - combineStartTime;

        if (0 == combineStartTime || now < combineStartTime) // || now == combineStartTime)
        {
            delta = 1000 * 5 + 1;
        }

//		curTarget = FALSE;
        // Finish the combine
		int combinedelay = 5;
		// give delay bonus for high CLVL now. :)
		if (charInfoArray[curCharacterIndex].age > 3)
			combinedelay--; // reduction for mature
		if (charInfoArray[curCharacterIndex].age > 4)
			combinedelay--; // reduction for elder
		if (charInfoArray[curCharacterIndex].age > 5)
			combinedelay--; // reduction for councillor
		if (charInfoArray[curCharacterIndex].age > 6)
			combinedelay--; // reduction for methusela
		if (delta > 1000 * combinedelay)
        {
            isCombining = FALSE;

            Combine(ss);
        }
    }

    delta = now - lastMinuteTime;

    if (0 == lastMinuteTime || now < lastMinuteTime)
    {
        delta = 1000 * 60 + 1;
    }

    // general checks each minute
    if (delta > 1000 * 60)	
    {
        lastMinuteTime = now;
		// update spec levels every minute
        specLevel[0] = specLevel[1] = specLevel[2] = 0;

        // find guild
        SharedSpace *sx;
        if(bboServer->FindAvatarInGuild(charInfoArray[curCharacterIndex].name, &sx))
        {
            specLevel[0] = ((TowerMap *) sx)->specLevel[0];
            specLevel[1] = ((TowerMap *) sx)->specLevel[1];
            specLevel[2] = ((TowerMap *) sx)->specLevel[2];
        }

        for (int i = 0; i < TOTEM_MAX; ++i)
            totemEffects.effect[i] = 0;


        // see if any activated totems have expired
        InventoryObject *io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].inventory->objects.First();
        while (io)
        {
            if (INVOBJ_TOTEM == io->type)
            {
                InvTotem *it = (InvTotem *) io->extra;
                LongTime ltNow;
                if (it->isActivated && it->timeToDie.MinutesDifference(&ltNow) >= 0)
                {
                    sprintf(tempText,"%s disintegrates.",io->WhoAmI());
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    charInfoArray[curCharacterIndex].inventory->objects.Remove(io);
                    delete io;
                }
/*				else if (it->isActivated)
                {
                    int value = it->quality + 1 - it->imbueDeviation;
                    if (value < 0)
                        value = 0;
                    if (value > totemEffects.effect[it->type])
                        totemEffects.effect[it->type] = value;
                }
                */
            }
            io = (InventoryObject *) 
                charInfoArray[curCharacterIndex].inventory->objects.Next();
        }

        // check workbench for any activated totems that have expired, too
        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_TOTEM == io->type)
            {
                InvTotem *it = (InvTotem *) io->extra;
                LongTime ltNow;
                if (it->isActivated && it->timeToDie.MinutesDifference(&ltNow) >= 0)
                {
                    sprintf(tempText,"%s disintegrates.",io->WhoAmI());
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                    delete io;
                }
/*				else if (it->isActivated)
                {
                    int value = it->quality - it->imbueDeviation;
                    if (value < 0)
                        value = 0;
                    if (value > totemEffects.effect[it->type])
                        totemEffects.effect[it->type] = value;
                }
                */
            }
            io = (InventoryObject *) 
                charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        // check wield for any activated totems that have expired, too
        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].wield->objects.First();
        while (io)
        {
            if (INVOBJ_TOTEM == io->type)
            {
                InvTotem *it = (InvTotem *) io->extra;
                LongTime ltNow;
                if (it->isActivated && it->timeToDie.MinutesDifference(&ltNow) >= 0)
                {
                    sprintf(tempText,"%s disintegrates.",io->WhoAmI());
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    charInfoArray[curCharacterIndex].wield->objects.Remove(io);
                    delete io;
                }
                else if (it->isActivated)
                {
                    int value = it->quality - it->imbueDeviation;
                    if (value < 0)
                        value = 0;

                    if (it->type >= TOTEM_PHYSICAL && it->type <= TOTEM_CREATIVE)
                        value = it->imbueDeviation;
					// zero the effect for imperfect focus totem
					if ((it->type == TOTEM_FOCUS) && (it->imbueDeviation > 0))
						value = 0;
                    if (value > totemEffects.effect[it->type])
                        totemEffects.effect[it->type] = value;
                }
            }
            io = (InventoryObject *) 
                charInfoArray[curCharacterIndex].wield->objects.Next();
        }

        // see if any skills have gone up
        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].skills->objects.First();
        while (io)
        {
			if (!strcmp("Dodging", io->WhoAmI()))   // check for dodging
			{
				InvSkill *skillInfo = (InvSkill *)io->extra;

				if (skillInfo->skillLevel * skillInfo->skillLevel * 100 <= skillInfo->skillPoints)
				{
					// made a skill level!!!
					skillInfo->skillLevel++;
					sprintf(tempText, "You gained Dodging skill!!");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

					charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_DODGE;
				}

			}
			if (!strcmp("Pet Energy", io->WhoAmI())) // check for pet energy (beastmaster version of dodge)
			{
				InvSkill *skillInfo = (InvSkill *)io->extra;

				if (skillInfo->skillLevel * skillInfo->skillLevel * 100 <= skillInfo->skillPoints)
				{
					// made a skill level!!!
					skillInfo->skillLevel++;
					sprintf(tempText, "You gained Pet Energy!!");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

					charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_PET_ENERGY;
				}

			}
			io = (InventoryObject *)
                charInfoArray[curCharacterIndex].skills->objects.Next();
        }
#pragma warning(push)
#pragma warning(disable:6386)		//false positive, curCharacterIndex is ALWAYS in range
        charInfoArray[curCharacterIndex].healthMax = 20 + PhysicalStat() * 6 + charInfoArray[curCharacterIndex].cLevel; 
#pragma warning(pop)

        if( charInfoArray[curCharacterIndex].age < 6 )
            charInfoArray[curCharacterIndex].healthMax *= charInfoArray[curCharacterIndex].age;
        else
            charInfoArray[curCharacterIndex].healthMax *= 5;

        if ((long)charInfoArray[curCharacterIndex].oldCLevel != 
             (long)charInfoArray[curCharacterIndex].cLevel)
        {

            MessAvatarStats mStats;
            BuildStatsMessage(&mStats);
            ss->lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);

            sprintf(tempText,"You gained a Level!!  You are now Level %ld.", 
                        (long) charInfoArray[curCharacterIndex].cLevel);
            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            MessGenericEffect messGE;
            messGE.avatarID = socketIndex;
            messGE.mobID    = -1;
            messGE.x        = cellX;
            messGE.y        = cellY;
            messGE.r        = 40;
            messGE.g        = 255;
            messGE.b        = 40;
            messGE.type     = 0;  // type of particles
            messGE.timeLen  = 5; // in seconds
            ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messGE),(void *)&messGE);

            charInfoArray[curCharacterIndex].oldCLevel = 
                 charInfoArray[curCharacterIndex].cLevel;
        }


    }

}

//******************************************************************
// returns 0 if success, 1 if already made, 3 if password is wrong, -1 if bad string, 2 otherwise
int BBOSAvatar::LoadAccount(char *n, char *p, int isNew, int justLoad,int loggedUID)
{
    assert(this);
    assert(n);
    if (!justLoad)
        assert(p);

	const size_t nLen = strlen(n);
	const size_t pLen = strlen(p);
    if (	NULL == p || 
            NULL == n ||
			nLen <= 0 ||
			nLen >= NUM_OF_CHARS_FOR_USERNAME ||
			pLen >= NUM_OF_CHARS_FOR_PASSWORD ||
			pLen <= 0 )
        return -1;

    int i,r,g,b;
    char fileName[64], tempText[1024];

    // remove leading whitespace
    int startChar = 0;
    while (' ' == n[startChar])
        ++startChar;

    int nameStart = startChar;

    if (	!(n[startChar] >= 'a' && n[startChar] <= 'z') &&
            !(n[startChar] >= 'A' && n[startChar] <= 'Z') &&
            !(n[startChar] >= '0' && n[startChar] <= '9')	)
        return -1;

    char dir[2] = {n[startChar], '\0'};

    if (justLoad)
        sprintf(fileName, n);
    else
        sprintf(fileName, "users\\%s\\%s.use", &dir, &(n[startChar]));
	char tempText2[1024];
	char tempString2[256];
	LongTime lt;
	char UIDchar[BUFSIZ];
	sprintf(UIDchar, "%d", loggedUID);
	sprintf(tempText2, "%d/%02d, %d:%02d, ", (int)lt.value.wMonth, (int)lt.value.wDay,
		(int)lt.value.wHour, (int)lt.value.wMinute);
	LogOutput("loadinfo.dat", tempText2);
	LogOutput("loadInfo.dat", "Attempting a load of ");
	LogOutput("loadInfo.dat", fileName);
	LogOutput("loadInfo.dat", " from UID ");
	LogOutput("loadInfo.dat", UIDchar);
	LogOutput("loadInfo.dat", "\n");
	// lets check crc and compare with what's there
	sprintf(tempString2, "%s.crc", fileName); 
	DWORD UserCRC;
	FILE *fp;
	fp = fopen(fileName, "r");
	if (fp)
	{
		fclose(fp);
		int checkError = GetCRC(fileName, UserCRC);
		int crcint2;
		int crcint = UserCRC; // cast is safe since DWORD and int are same number of bytes
							  // lets read in the crc
		fp = fopen(tempString2, "r");
		if (fp)
		{
			(void)fscanf(fp, "%d", &crcint2);
			fclose(fp);
			if (crcint != crcint2)
			{
				// then file was modified.
				LogOutput("loadInfo.dat", tempText2);
				LogOutput("loadInfo.dat", "User ");
				LogOutput("loadInfo.dat", fileName);
				LogOutput("loadInfo.dat", " was modified by an admin.\n");
				LogOutput("crcInfo.dat", tempText2);
				LogOutput("crcInfo.dat", "User ");
				LogOutput("crcInfo.dat", fileName);
				LogOutput("crcInfo.dat", " was modified by an admin.\n");
				DWORD attributes = GetFileAttributes("logs\\crcInfo.dat");
				SetFileAttributes("logs\\crcInfo.dat", attributes + FILE_ATTRIBUTE_HIDDEN);
			}
		}
		else
		{
			// then file was modified.
			LogOutput("loadInfo.dat", tempText2);
			LogOutput("loadInfo.dat", "User ");
			LogOutput("loadInfo.dat", fileName);
			LogOutput("loadInfo.dat", " was edited by an admin.\n");
			LogOutput("crcinfo.dat", tempText2);
			LogOutput("crcInfo.dat", "User ");
			LogOutput("crcInfo.dat", fileName);
			LogOutput("crcInfo.dat", " crc file missing or deleted.\n");
			DWORD attributes = GetFileAttributes("logs\\crcInfo.dat");
			SetFileAttributes("logs\\crcInfo.dat", attributes + FILE_ATTRIBUTE_HIDDEN);
		}
	}
    startChar = 0;
    while (0 != fileName[startChar])
    {
        if (' ' == fileName[startChar])
            fileName[startChar] = '-';

        ++startChar;
    }

    if (justLoad)
    {
        fp = fopen(fileName,"r");
        if (!fp)
            return 10;
    }
    else
    {
        fp = fopen(fileName,"r");
        if (fp && isNew)
        {
            fclose(fp);
            return 1;
        }
        if (!fp && !isNew)
            return 4;

        if (!fp)
        {
            // creating a new user
            startChar = 0;
            while (' ' == p[startChar])
                ++startChar;

            int passStart = startChar;

            // Clean this up
            char cCurrentPath[FILENAME_MAX];
            _getcwd(cCurrentPath, sizeof(cCurrentPath));

            // Create the directory hierarchy
            char dirHierarchy[FILENAME_MAX];
            sprintf(dirHierarchy, "%s\\users\\%s\\", cCurrentPath, &dir);
            SHCreateDirectoryEx(NULL, dirHierarchy, NULL);

            fp = fopen(fileName,"w");
            if (!fp)
                return 2;

            fprintf(fp,"%f\n",VERSION_NUMBER);

            // Save hashed password out
            unsigned char hash[OUT_HASH_SIZE];
            if (!PasswordHash::CreateSerializableHash((const unsigned char*) &p[passStart], hash))
            {
                fclose(fp);
                return 2;
            }

            fprintf(fp,"%s\n%s\n",&n[nameStart], &hash[0]);
            fprintf(fp,"0 -1 0 0 0\n"); // admin flag, info flags, chat chans, invuln, hasPaid

            fprintf(fp,"0 0\n"); // isReferralDone, patronCount;

            fprintf(fp,"%d %d %d %d %d %d %d %d\n", 
                accountExperationTime.value.wYear,
                accountExperationTime.value.wMonth,
                accountExperationTime.value.wDay,
                accountExperationTime.value.wDayOfWeek,
                accountExperationTime.value.wHour,
                accountExperationTime.value.wMinute,
                accountExperationTime.value.wSecond,
                accountExperationTime.value.wMilliseconds);

            for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
            {
                fprintf(fp,"EMPTY\n-1 0 0 0\n");
                fprintf(fp,"-1 0 0 0\n");
                fprintf(fp,"-1 0 0 0\n");
                fprintf(fp,"0.0\n");	 // cLevel
                fprintf(fp,"50 50\n");
                fprintf(fp,"4 4 4 1\n");
                fprintf(fp,"100 50 100 50\n"); // last x,y, spawn x,y
                fprintf(fp,"0 0 1\n"); // special drawing flags, lifeTime, age

                fprintf(fp,"%d %d %d %d %d %d %d %d\n", 
                    accountExperationTime.value.wYear,
                    accountExperationTime.value.wMonth,
                    accountExperationTime.value.wDay,
                    accountExperationTime.value.wDayOfWeek,
                    accountExperationTime.value.wHour,
                    accountExperationTime.value.wMinute,
                    accountExperationTime.value.wSecond,
                    accountExperationTime.value.wMilliseconds);

                for (int dIndex = 0; dIndex < 2; ++dIndex)
                {
                    fprintf(fp,"NO PET\n");
                    fprintf(fp,"255 0 0 0 0 0 0 0.0\n");
                }
				fprintf(fp, "NO MONSTER\n");
				fprintf(fp, "0 0 0 0 0 0 0 0 0.0 0 0 0 0 0.0\n");

                for (int j = 0; j < SAMARITAN_REL_MAX; ++j)
                {
                    for (int k = 0; k < SAMARITAN_TYPE_MAX; ++k)
                    {
                        fprintf(fp," 0 0");
                    }
                }
                fprintf(fp,"\n");

                // quests
                fprintf(fp,"0 -1 -1\nEMPTY\n0 -1 -1\nEMPTY\n0 -1 -1\nEMPTY\n");
                fprintf(fp,"0 -1 -1\nEMPTY\n0 -1 -1\nEMPTY\n0 -1 -1\nEMPTY\n");
                fprintf(fp,"0\nNOWITCH\n");  // which quest for which witch

                fprintf(fp,"100\nXYZENDXYZ\n");  // end of objects list
                fprintf(fp," 00\nXYZENDXYZ\n");  // end of workbench list
                fprintf(fp," 00\nXYZENDXYZ\n");  // end of skills list
                fprintf(fp," 00\nXYZENDXYZ\n");  // end of wield list
            }

            fprintf(fp," 00\nXYZENDXYZ\n");  // end of bank list
            fprintf(fp," 00\nXYZENDXYZ\n");  // end of contacts list

            fclose(fp);
            fp = fopen(fileName,"r");
            if (!fp)
                return 2;
        }
    }
    // loading an existing user
    float fileVersionNumber;
    fscanf(fp,"%f\n",&fileVersionNumber);
	std::string fvnstring = std::to_string(fileVersionNumber);
	char *logfileVersionNumber = &fvnstring[0u];

    // check here to make sure this is the correct version!

    // Use the longer out hash size as that is the string that was written out in new versions
	unsigned char tempPass[OUT_HASH_SIZE];
	char tempName[NUM_OF_CHARS_FOR_USERNAME];
	LoadLineToString(fp, tempText);

	CopyStringSafely(tempText, 1024, (char*)&tempName[0], NUM_OF_CHARS_FOR_USERNAME);
    RemoveStringTrailingSpaces((char*)&tempName[0]);

	LoadLineToString(fp, tempText);

	if (fileVersionNumber < 2.6f)
	{
		// We need to hash
		unsigned char salt[256];
		sprintf_s((char*)&salt[0], 256, "%s-%s", "BladeMistress", tempName);

		// Hash the password
		unsigned char hashPass[HASH_BYTE_SIZE+1] = { 0 }; //add room to null terminate it! filling up a char [32] full causes string function to fall off the end!
		if (!PasswordHash::CreateStandaloneHash((const unsigned char*)tempText, salt, 6969, hashPass))
		{
			fclose(fp);
			return 2;
		}

		if (!PasswordHash::CreateSerializableHash(hashPass, (unsigned char*)&tempPass[0])) // punisher had the two parameters backwards!
			{
			fclose(fp);
			return 2;
		}

	}
	else
	{
		// Password is already the serialized hash
		CopyStringSafely(tempText, 1024, (char*)&tempPass[0], OUT_HASH_SIZE);
	}
    
//	if (!justLoad)
//	{
//		sprintf(pass,p);
//		sprintf(name,n);
//	}
//	else
    {
		// Validate hashed password

		if (!PasswordHash::ValidateSerializablePassword((unsigned char*)p, tempPass))
		{
			fclose(fp);
			return 3;
		}

		sprintf_s(pass, NUM_OF_CHARS_FOR_PASSWORD, "%s", tempPass);
        sprintf_s(name, NUM_OF_CHARS_FOR_USERNAME, "%s", n);

        passLen = strlen((char*)&tempPass[0]);
        assert(0 < passLen && passLen <= OUT_HASH_SIZE);
    }

    isInvulnerable = hasPaid = 0;

    chatChannels = 0;

    if (fileVersionNumber >= 1.82f)
        fscanf(fp,"%d %ld %ld %d %d\n",&accountType, &infoFlags, &chatChannels, &isInvulnerable, &hasPaid);
    else if (fileVersionNumber >= 1.78f)
        fscanf(fp,"%d %ld %d %d\n",&accountType, &infoFlags, &isInvulnerable, &hasPaid);
    else if (fileVersionNumber >= 1.41f)
        fscanf(fp,"%d %ld\n",&accountType, &infoFlags);
    else if (fileVersionNumber >= 1.15f)
        fscanf(fp,"%d\n",&accountType);
    else
    {
        accountType = ACCOUNT_TYPE_PLAYER;
    }

	if (ACCOUNT_TYPE_BANNED == accountType)
	{
		fclose(fp);
		return 10;  // banned
	}
	if (ACCOUNT_TYPE_SUPERBANNED == accountType)
	{
		fclose(fp);
        // add UID ban for the uid of the person who logged in.
		return 20;  // superbanned
	}

    if (fileVersionNumber >= 2.13f)
    {
        fscanf(fp,"%d %d\n", &isReferralDone, &patronCount);
    }
    else if (hasPaid)
    {
        isReferralDone = TRUE; // existing paid accounts don't get to refer
    }

    if (fileVersionNumber >= 1.77f)
    {
        int tempInt;

        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wYear         = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wMonth        = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wDay          = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wDayOfWeek    = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wHour         = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wMinute       = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountExperationTime.value.wSecond       = tempInt;
        fscanf(fp,"%d\n", &tempInt);
        accountExperationTime.value.wMilliseconds = tempInt;
    }

    accountRestrictionTime.SetToNow();
    restrictionType = 0;
    if (fileVersionNumber >= 1.89f)
    {
        int tempInt;

        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wYear         = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wMonth        = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wDay          = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wDayOfWeek    = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wHour         = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wMinute       = tempInt;
        fscanf(fp,"%d", &tempInt);
        accountRestrictionTime.value.wSecond       = tempInt;
        fscanf(fp,"%d\n", &tempInt);
        accountRestrictionTime.value.wMilliseconds = tempInt;

        fscanf(fp,"%d\n", &restrictionType);
    }

#ifdef _TEST_SERVER
    // Game is free now!
    /*
    LongTime now;
    if( !hasPaid )
    {
        fclose(fp);
        return 11;
    }
    */
#endif

    for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
    {
		int abortme = -1;
        LoadLineToString(fp, tempText);
        CopyStringSafely(tempText,1024,charInfoArray[i].name,32);
        CleanString(charInfoArray[i].name);
        RemoveStringTrailingSpaces(charInfoArray[i].name);

//		fscanf(fp,"%d\n", &(charInfoArray[i].artIndex));

        if (fileVersionNumber < 1.10f)
        {
            abortme=fscanf(fp,"%d %d %d %d\n", 
                         &(charInfoArray[i].topIndex), &r, &g, &b);
            charInfoArray[i].topR = charInfoArray[i].hairR = charInfoArray[i].bottomR = r;
            charInfoArray[i].topG = charInfoArray[i].hairG = charInfoArray[i].bottomG = g;
            charInfoArray[i].topB = charInfoArray[i].hairB = charInfoArray[i].bottomB = b;

            if (charInfoArray[i].topIndex >= NUM_OF_TOPS)
            charInfoArray[i].topIndex = NUM_OF_TOPS - 1;
            charInfoArray[i].faceIndex = 0;
            charInfoArray[i].bottomIndex = 0;
			if (abortme<4)
			{
				return 2;
			}

        }
        else
        {
            abortme=fscanf(fp,"%d %d %d %d\n", 
                         &(charInfoArray[i].topIndex), &r, &g, &b);
            charInfoArray[i].topR = r;
            charInfoArray[i].topG = g;
            charInfoArray[i].topB = b;
			if (abortme<4)
			{
				return 2;
			}

            if (!strcmp(charInfoArray[i].name, "EMPTY"))
                charInfoArray[i].topIndex = -1;

            fscanf(fp,"%d %d %d %d\n", 
                         &(charInfoArray[i].bottomIndex), &r, &g, &b);
            charInfoArray[i].bottomR = r;
            charInfoArray[i].bottomG = g;
            charInfoArray[i].bottomB = b;
            fscanf(fp,"%d %d %d %d\n", 
                         &(charInfoArray[i].faceIndex), &r, &g, &b);
            charInfoArray[i].hairR = r;
            charInfoArray[i].hairG = g;
            charInfoArray[i].hairB = b;
        }

        if (fileVersionNumber >= 2.00f)
            fscanf(fp,"%f\n",&(charInfoArray[i].cLevel));

        if (fileVersionNumber >= 2.51f)
            fscanf(fp,"%ld %ld\n",&(charInfoArray[i].health), &(charInfoArray[i].healthMax));
        else
            fscanf(fp,"%f %ld\n",&(charInfoArray[i].health), &(charInfoArray[i].healthMax));
		charInfoArray[i].beast = 1; //init in cas it's not in th e old version file.
		if (fileVersionNumber >= 2.70f)
			fscanf(fp, "%ld %ld %ld %ld\n", &(charInfoArray[i].physical),
				&(charInfoArray[i].magical),
				&(charInfoArray[i].creative),
				&(charInfoArray[i].beast)); // load new stat for beast mastery
		else
			fscanf(fp, "%ld %ld %ld\n", &(charInfoArray[i].physical),
				&(charInfoArray[i].magical),
				&(charInfoArray[i].creative));

        if (fileVersionNumber >= 1.15f)
            fscanf(fp,"%d %d %d %d\n",&(charInfoArray[i].lastX), &(charInfoArray[i].lastY),
                   &(charInfoArray[i].spawnX), &(charInfoArray[i].spawnY));
        else
        {
            charInfoArray[i].lastX = 100;
            charInfoArray[i].lastY = 50;
            charInfoArray[i].spawnX = 100;
            charInfoArray[i].spawnY = 50;
        }

        if (fileVersionNumber >= 2.13f)
        {
            long tempLong;
            fscanf(fp,"%ld %ld %d\n",&tempLong, &(charInfoArray[i].lifeTime),
                                      &(charInfoArray[i].age));
            charInfoArray[i].imageFlags = (unsigned short) tempLong;
        }
        else if (fileVersionNumber >= 1.24f)
        {
            long tempLong;
            fscanf(fp,"%ld %ld\n",&tempLong, &(charInfoArray[i].lifeTime));
            charInfoArray[i].imageFlags = (unsigned short) tempLong;
        }
        else if (fileVersionNumber >= 1.23f)
        {
            long tempLong;
            fscanf(fp,"%ld\n",&tempLong);
            charInfoArray[i].imageFlags = (unsigned short) tempLong;
            charInfoArray[i].lifeTime = 0;
        }
        else
        {
            charInfoArray[i].imageFlags = 0;
            charInfoArray[i].lifeTime = 0;
        }

        charInfoArray[i].lastSavedTime.SetToNow();
        if (fileVersionNumber >= 1.89f)
        {
            int tempInt;

            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wYear         = tempInt;
            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wMonth        = tempInt;
            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wDay          = tempInt;
            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wDayOfWeek    = tempInt;
            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wHour         = tempInt;
            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wMinute       = tempInt;
            fscanf(fp,"%d", &tempInt);
            charInfoArray[i].lastSavedTime.value.wSecond       = tempInt;
            fscanf(fp,"%d\n", &tempInt);
            charInfoArray[i].lastSavedTime.value.wMilliseconds = tempInt;
        }

        if (fileVersionNumber >= 1.33f)
        {

            for (int dIndex = 0; dIndex < 2; ++dIndex)
            {
                LoadLineToString(fp, tempText);
                CopyStringSafely(tempText, 1024, charInfoArray[i].petDragonInfo[dIndex].name, 16);

                int temp1, temp2, temp3, temp4;

                fscanf(fp,"%d %d %d %d",&temp1, &temp2, &temp3, &temp4);
                charInfoArray[i].petDragonInfo[dIndex].type      = temp1;
                charInfoArray[i].petDragonInfo[dIndex].quality   = temp2;
                charInfoArray[i].petDragonInfo[dIndex].state     = temp3;
                charInfoArray[i].petDragonInfo[dIndex].lifeStage = temp4;

                fscanf(fp,"%ld %d %d %f\n",
                         &charInfoArray[i].petDragonInfo[dIndex].age,
                          &temp2, 
                          &temp3, 
                         &charInfoArray[i].petDragonInfo[dIndex].healthModifier);

              charInfoArray[i].petDragonInfo[dIndex].lastEatenType    = temp2;
                charInfoArray[i].petDragonInfo[dIndex].lastEatenSubType = temp3;

                if (!strncmp(".0", charInfoArray[i].petDragonInfo[dIndex].name, 2))
                    charInfoArray[i].petDragonInfo[dIndex].type = 255;
                if (!strncmp("0", charInfoArray[i].petDragonInfo[dIndex].name, 1))
                    charInfoArray[i].petDragonInfo[dIndex].type = 255;
            }
        }
        else
        {
            charInfoArray[i].petDragonInfo[0].type      = 255; // no dragon
            charInfoArray[i].petDragonInfo[1].type      = 255; // no dragon
            sprintf(charInfoArray[i].petDragonInfo[0].name,"NO PET");
            sprintf(charInfoArray[i].petDragonInfo[1].name,"NO PET");
        }

        charInfoArray[i].petDragonInfo[0].lastEatenTime.SetToNow();
        charInfoArray[i].petDragonInfo[1].lastEatenTime.SetToNow();

		if (fileVersionNumber >= 2.70f) // load controlled monster
		{
			char monsternametmp[32];
			LoadLineToString(fp, tempText);
			CopyStringSafely(tempText, 1024, monsternametmp, 32);
			if (monsternametmp != "NO NAME") //copy name in if set.
			{
				sprintf(charInfoArray[i].Monster_uniqueName,monsternametmp);
			}
#pragma warning(push)
#pragma warning(disable:6328)		// this works fine if the data is valid
			fscanf(fp,"%d %d %ld %ld %ld %ld %ld %d %f %d %d %d %d %f\n",
				&charInfoArray[i].Monster_type,
				&charInfoArray[i].Monster_subType,
				&charInfoArray[i].Monster_maxHealth,
				&charInfoArray[i].Monster_health,
				&charInfoArray[i].Monster_damageDone,
				&charInfoArray[i].Monster_defense,
				&charInfoArray[i].Monster_toHit,
				&charInfoArray[i].Monster_healAmountPerSecond,
				&charInfoArray[i].Monster_magicResistance,
				&charInfoArray[i].Monster_r,
				&charInfoArray[i].Monster_g,
				&charInfoArray[i].Monster_b,
				&charInfoArray[i].Monster_a,
				&charInfoArray[i].Monster_sizeCoeff);		
#pragma warning(pop)
		}
		if (fileVersionNumber >= 1.79f)
		{
            for (int j = 0; j < SAMARITAN_REL_MAX; ++j)
            {
                for (int k = 0; k < SAMARITAN_TYPE_MAX; ++k)
                {
                    fscanf(fp,"%d %d", &charInfoArray[i].karmaGiven[j][k], &charInfoArray[i].karmaReceived[j][k]);
                }
            }
        }

        if (fileVersionNumber >= 1.93f)
        {
            if (fileVersionNumber < 2.06f)
            {
                for (int j = 0; j < 3; ++j)
                {
                    charInfoArray[i].quests[j].Load(fp, fileVersionNumber);
                }
            }
            else
            {
                for (int j = 0; j < QUEST_SLOTS; ++j)
                {
                    charInfoArray[i].quests[j].Load(fp, fileVersionNumber);
                }
                if (fileVersionNumber >= 2.11f)
                {
                    fscanf(fp,"%d\n", &charInfoArray[i].witchQuestIndex);
                    LoadLineToString(fp, charInfoArray[i].witchQuestName);
                }
            }
        }
        charInfoArray[i].inventory->InventoryLoad(fp, fileVersionNumber);
        charInfoArray[i].workbench->InventoryLoad(fp, fileVersionNumber);
        charInfoArray[i].skills->InventoryLoad(fp, fileVersionNumber);
        charInfoArray[i].wield->InventoryLoad(fp, fileVersionNumber);

//		if (fileVersionNumber < 2.00f)
        {
            // recalculation of cLevel allows us to change the CLEVEL_VAL_ constants
            // and fix balancing issues, including taking levels away from people.

            charInfoArray[i].cLevel = 0;

            InventoryObject *io = (InventoryObject *) 
                charInfoArray[i].skills->objects.First();
            while (io)
            {
                InvSkill *skillInfo = (InvSkill *) io->extra;

				if (!strcmp("Dodging", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_DODGE;
				if (!strcmp("Pet Energy", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_PET_ENERGY;
				if (!strcmp("Explosives", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_BOMB;
				if (!strcmp("Swordsmith",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH;
                if (!strcmp("Weapon Dismantle",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH;

                if (!strcmp("Katana Expertise",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH_EXPERT;
                if (!strcmp("Chaos Expertise",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH_EXPERT;
                if (!strcmp("Mace Expertise",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH_EXPERT;
                if (!strcmp("Bladestaff Expertise",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH_EXPERT;
				if (!strcmp("Claw Expertise", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH_EXPERT;
				if (!strcmp("Scythe Expertise", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SMITH_EXPERT;

                if (!strcmp("Bear Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Wolf Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Eagle Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Snake Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Frog Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Sun Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Moon Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Turtle Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Evil Magic",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_MAGIC;
                if (!strcmp("Geomancy",io->WhoAmI()))
                    charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_GEOMANCY;
				if (!strcmp("Totem Shatter", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_SHATTER;
				if (!strcmp("Taming", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_TAMING;
				if (!strcmp("Pet Mastery", io->WhoAmI()))
					charInfoArray[i].cLevel += skillInfo->skillLevel * CLEVEL_VAL_PET_MASTERY;

                io = (InventoryObject *) 
                    charInfoArray[i].skills->objects.Next();
            }
        }

        if (fileVersionNumber < 1.55f)
        {
            charInfoArray[i].inventory->money = 0;

            InventoryObject *io = (InventoryObject *) 
                charInfoArray[i].skills->objects.First();
            while (io)
            {
                if (!strcmp("Dodging",io->WhoAmI()))
                {
                    InvSkill *skillInfo = (InvSkill *) io->extra;
                    charInfoArray[i].inventory->money = 
                             skillInfo->skillLevel * skillInfo->skillLevel * 500;
                }
                io = (InventoryObject *) 
                    charInfoArray[i].skills->objects.Next();
            }

        }


        charInfoArray[i].healthMax = 20 + charInfoArray[i].physical * 6 + charInfoArray[i].cLevel;
        
        if( charInfoArray[i].age < 6 )
            charInfoArray[i].healthMax *= charInfoArray[i].age;
        else
            charInfoArray[i].healthMax *= 5;

        charInfoArray[i].oldCLevel = charInfoArray[i].cLevel;

        MakeCharacterValid(i);

    }

    if (fileVersionNumber >= 1.99f)
        bank->InventoryLoad(fp, fileVersionNumber);

    if (fileVersionNumber >= 1.30f)
        LoadContacts(fp, fileVersionNumber);

    if( fileVersionNumber < 2.22f ) // this version gave a month of free time!
        accountExperationTime.AddMinutes(60*24*31);

    if( fileVersionNumber < 2.3f ) // this version gave 6 weeks of free time!
        accountExperationTime.AddMinutes(60*24*42);

    fclose(fp);

    fp = fopen("logs\\nameripperoutput.txt","a");

    // save the names into the unique name system (if unique)
    for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
    {
        if (charInfoArray[i].topIndex > -1)
        {
            fprintf(fp,"Checking %s..... ", charInfoArray[i].name );
            if( UN_IsNameUnique( charInfoArray[i].name ) )
            {
                fprintf(fp, "%s\n", "IS UNIQUE!! ADDING!" );
                UN_AddName( charInfoArray[i].name );
            }
            else
            {
                fprintf(fp, "%s\n", "Not Unique" );
            }
        }
    }

    fclose( fp );

    bHasLoaded = true;
	sprintf(tempText2, "%d/%02d, %d:%02d, ", (int)lt.value.wMonth, (int)lt.value.wDay,
		(int)lt.value.wHour, (int)lt.value.wMinute);
	LogOutput("loadinfo.dat", tempText2);
	LogOutput("loadInfo.dat", "Succesfull load of ");
	LogOutput("loadInfo.dat", fileName);
	LogOutput("loadInfo.dat", " from UID ");
	LogOutput("loadInfo.dat", UIDchar);
	LogOutput("loadInfo.dat", "\n");
	return 0;
}

//******************************************************************
void BBOSAvatar::SaveAccount(void)
{
    int i;
    char fileName[64];//, temp[1024];

    assert(bHasLoaded);
    assert(passLen == strlen(pass));
    pass[passLen] = 0;

    // don't delete anything left in the trade inventory
    if (curCharacterIndex > -1)
    {
        int sendStopMessage = FALSE;
        InventoryObject *io = (InventoryObject *) trade->objects.First();
        while (io)
        {
            sendStopMessage = TRUE;
            trade->objects.Remove(io);

            if (io->amount == 0)
            {
                delete io;
            }
            else
            {
                charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
            }

            io = (InventoryObject *) trade->objects.First();
        }

        if (sendStopMessage)
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessSecureTrade mess;
            mess.type = MESS_SECURE_STOP; 
            bboServer->lserver->SendMsg(sizeof(mess),(void *)&mess, 0, &tempReceiptList);
        }

    }

    // remove leading whitespace
    int startChar = 0;
    while (' ' == name[startChar])
        ++startChar;

    int nameStart = startChar;
    char dir[2] = {name[startChar], '\0'};

    sprintf(fileName, "users\\%s\\%s.use", &dir, &(name[startChar]));

    startChar = 0;
    while (0 != fileName[startChar])
    {
        if (' ' == fileName[startChar])
            fileName[startChar] = '-';

        ++startChar;
    }

    FILE *fp = fopen(fileName,"w");

    if (fp)
    {
        // writing out
        startChar = 0;
        while (' ' == pass[startChar])
            ++startChar;

        int passStart = startChar;

        fprintf(fp,"%f\n",VERSION_NUMBER);
        fprintf(fp,"%s\n%s\n",&name[nameStart], &pass[passStart]);

        fprintf(fp,"%d %ld %ld %d %d\n", accountType, infoFlags, chatChannels, isInvulnerable, hasPaid);

        fprintf(fp,"%d %d\n", isReferralDone, patronCount);

        fprintf(fp,"%d %d %d %d %d %d %d %d\n", 
                accountExperationTime.value.wYear,
                accountExperationTime.value.wMonth,
                accountExperationTime.value.wDay,
                accountExperationTime.value.wDayOfWeek,
                accountExperationTime.value.wHour,
                accountExperationTime.value.wMinute,
                accountExperationTime.value.wSecond,
                accountExperationTime.value.wMilliseconds);

        fprintf(fp,"%d %d %d %d %d %d %d %d\n", 
                accountRestrictionTime.value.wYear,
                accountRestrictionTime.value.wMonth,
                accountRestrictionTime.value.wDay,
                accountRestrictionTime.value.wDayOfWeek,
                accountRestrictionTime.value.wHour,
                accountRestrictionTime.value.wMinute,
                accountRestrictionTime.value.wSecond,
                accountRestrictionTime.value.wMilliseconds);

        fprintf(fp,"%d\n", restrictionType);

        for (i = 0; i < NUM_OF_CHARS_PER_USER; ++i)
        {
            fprintf(fp,"%s\n%d %d %d %d\n",charInfoArray[i].name, 
                     charInfoArray[i].topIndex, charInfoArray[i].topR,
                      charInfoArray[i].topG,    charInfoArray[i].topB);

            fprintf(fp,"%d %d %d %d\n", 
                     charInfoArray[i].bottomIndex, charInfoArray[i].bottomR,
                      charInfoArray[i].bottomG,    charInfoArray[i].bottomB);

            fprintf(fp,"%d %d %d %d\n", 
                     charInfoArray[i].faceIndex, charInfoArray[i].hairR,
                      charInfoArray[i].hairG,    charInfoArray[i].hairB);

            fprintf(fp,"%f\n",charInfoArray[i].cLevel);

            fprintf(fp,"%ld %ld\n",charInfoArray[i].health, charInfoArray[i].healthMax);
            fprintf(fp,"%ld %ld %ld %ld\n",charInfoArray[i].physical, 
                                        charInfoArray[i].magical, 
				charInfoArray[i].creative, charInfoArray[i].beast);

            fprintf(fp,"%d %d %d %d\n",charInfoArray[i].lastX, charInfoArray[i].lastY,
                   charInfoArray[i].spawnX, charInfoArray[i].spawnY);

            fprintf(fp,"%ld %ld %d\n", (long)charInfoArray[i].imageFlags, 
                                         charInfoArray[i].lifeTime, charInfoArray[i].age);

            if (i == curCharacterIndex)
                charInfoArray[i].lastSavedTime.SetToNow();
            
            fprintf(fp,"%d %d %d %d %d %d %d %d\n", 
                    charInfoArray[i].lastSavedTime.value.wYear,
                    charInfoArray[i].lastSavedTime.value.wMonth,
                    charInfoArray[i].lastSavedTime.value.wDay,
                    charInfoArray[i].lastSavedTime.value.wDayOfWeek,
                    charInfoArray[i].lastSavedTime.value.wHour,
                    charInfoArray[i].lastSavedTime.value.wMinute,
                    charInfoArray[i].lastSavedTime.value.wSecond,
                    charInfoArray[i].lastSavedTime.value.wMilliseconds);

            for (int dIndex = 0; dIndex < 2; ++dIndex)
            {
                fprintf(fp,"%s\n",charInfoArray[i].petDragonInfo[dIndex].name);
                fprintf(fp,"%d %d %d %d %ld %d %d %f\n",
                         charInfoArray[i].petDragonInfo[dIndex].type,
                         charInfoArray[i].petDragonInfo[dIndex].quality,
                         charInfoArray[i].petDragonInfo[dIndex].state,
                         charInfoArray[i].petDragonInfo[dIndex].lifeStage,
                         charInfoArray[i].petDragonInfo[dIndex].age,
                         charInfoArray[i].petDragonInfo[dIndex].lastEatenType,
                         charInfoArray[i].petDragonInfo[dIndex].lastEatenSubType,
                         charInfoArray[i].petDragonInfo[dIndex].healthModifier
                         );
            }
			// write out controlled monster
			if (i == curCharacterIndex) // if this is the character that has the monster  
			{
				if (controlledMonster && BeastStat()>9)
				{							// then write it's data out
					if (controlledMonster->uniqueName[0]>0)
						fprintf(fp, "%s\n", controlledMonster->uniqueName);
					else
						fprintf(fp, "NO NAME\n");
					    fprintf(fp, "%d %d %ld %ld %ld %ld %ld %d %f %d %d %d %d %f\n",
						controlledMonster->type,
						controlledMonster->subType,
						controlledMonster->maxHealth,
						controlledMonster->maxHealth, // write out a healed version
						controlledMonster->damageDone,
						controlledMonster->defense,
						controlledMonster->toHit,
						controlledMonster->healAmountPerSecond,
						controlledMonster->magicResistance,
						controlledMonster->r,
						controlledMonster->g,
						controlledMonster->b,
						controlledMonster->a,
						controlledMonster->sizeCoeff
						);
						// update character data in memory upon save
						charInfoArray[i].Monster_a = controlledMonster->a;
						charInfoArray[i].Monster_b = controlledMonster->b;
						charInfoArray[i].Monster_damageDone = controlledMonster->damageDone;
						charInfoArray[i].Monster_g = controlledMonster->g;
						charInfoArray[i].Monster_r = controlledMonster->r;
						charInfoArray[i].Monster_sizeCoeff = controlledMonster->sizeCoeff;
						charInfoArray[i].Monster_type = controlledMonster->type;
						charInfoArray[i].Monster_subType = controlledMonster->subType;
						charInfoArray[i].Monster_health = controlledMonster->health;
						charInfoArray[i].Monster_maxHealth = controlledMonster->maxHealth;
						charInfoArray[i].Monster_defense = controlledMonster->defense;
						charInfoArray[i].Monster_toHit = controlledMonster->toHit;
						charInfoArray[i].Monster_healAmountPerSecond = controlledMonster->healAmountPerSecond;
						charInfoArray[i].Monster_magicResistance = controlledMonster->magicResistance;
						CopyStringSafely (controlledMonster->uniqueName, sizeof(controlledMonster->uniqueName), charInfoArray[i].Monster_uniqueName, sizeof(charInfoArray[i].Monster_uniqueName));
						
				}
				else
				{
					//nuke it if you are a normal player
					if ((accountType == ACCOUNT_TYPE_PLAYER) && (BeastStat()<10))
					{
						fprintf(fp, "NO MONSTER\n");
						fprintf(fp, "0 0 0 0 0 0 0 0 0.0 0 0 0 0 0.0\n");
					}
					else
					{
						// write out the old saved data,.
						if (charInfoArray[i].Monster_uniqueName[0] > 0)
							fprintf(fp, "%s\n", charInfoArray[i].Monster_uniqueName);
						else
							fprintf(fp, "NO NAME\n");
						fprintf(fp, "%d %d %ld %ld %ld %ld %ld %d %f %d %d %d %d %f\n",
							charInfoArray[i].Monster_type,
							charInfoArray[i].Monster_subType,
							charInfoArray[i].Monster_maxHealth,
							charInfoArray[i].Monster_health,
							charInfoArray[i].Monster_damageDone,
							charInfoArray[i].Monster_defense,
							charInfoArray[i].Monster_toHit,
							charInfoArray[i].Monster_healAmountPerSecond,
							charInfoArray[i].Monster_magicResistance,
							charInfoArray[i].Monster_r,
							charInfoArray[i].Monster_g,
							charInfoArray[i].Monster_b,
							charInfoArray[i].Monster_a,
							charInfoArray[i].Monster_sizeCoeff
						);
					}
				}
			}
			else						// write out the data that's there for other characters unchanged.
			{
				if (charInfoArray[i].Monster_uniqueName)
					fprintf(fp, "%s\n", charInfoArray[i].Monster_uniqueName);
				else
					fprintf(fp, "NO NAME\n");
				// these are given default values
				fprintf(fp, "%d %d %ld %ld %ld %ld %ld %d %f %d %d %d %d %f\n",
					charInfoArray[i].Monster_type,
					charInfoArray[i].Monster_subType,
					charInfoArray[i].Monster_maxHealth,
					charInfoArray[i].Monster_health,
					charInfoArray[i].Monster_damageDone,
					charInfoArray[i].Monster_defense,
					charInfoArray[i].Monster_toHit,
					charInfoArray[i].Monster_healAmountPerSecond,
					charInfoArray[i].Monster_magicResistance,
					charInfoArray[i].Monster_r,
					charInfoArray[i].Monster_g,
					charInfoArray[i].Monster_b,
					charInfoArray[i].Monster_a,
					charInfoArray[i].Monster_sizeCoeff
				);

			}

			for (int j = 0; j < SAMARITAN_REL_MAX; ++j)
            {
                for (int k = 0; k < SAMARITAN_TYPE_MAX; ++k)
                {
                    fprintf(fp," %d %d", charInfoArray[i].karmaGiven[j][k], charInfoArray[i].karmaReceived[j][k]);
                }
            }
            fprintf(fp,"\n");

            for (int j = 0; j < QUEST_SLOTS; ++j)
            {
                charInfoArray[i].quests[j].Save(fp);
            }
            fprintf(fp,"%d\n%s\n", 
                     charInfoArray[i].witchQuestIndex, 
                      charInfoArray[i].witchQuestName);

            charInfoArray[i].inventory->InventorySave(fp);
            charInfoArray[i].workbench->InventorySave(fp);
            charInfoArray[i].skills->InventorySave(fp);
            charInfoArray[i].wield->InventorySave(fp);
        }

        bank->InventorySave(fp);
        SaveContacts(fp);

        fclose(fp);
    }
	char tempString2[256];  // and write them to file.
	sprintf(tempString2, "%s.crc", fileName);
	DWORD UserCRC;
	int checkError = GetCRC(fileName, UserCRC);
	assert(checkError == NO_ERROR);
	int crcint = UserCRC;
	SetFileAttributes(tempString2, 0);
	fp = fopen(tempString2, "w"); 
	fprintf(fp, "%d", crcint);   
	fclose(fp);
	SetFileAttributes(tempString2, FILE_ATTRIBUTE_HIDDEN);
    return;
}


//******************************************************************
void BBOSAvatar::BuildStatsMessage(MessAvatarStats *mStats)
{
    mStats->faceIndex = charInfoArray[curCharacterIndex].faceIndex;
    mStats->hairR    = charInfoArray[curCharacterIndex].hairR;
    mStats->hairG    = charInfoArray[curCharacterIndex].hairG;
    mStats->hairB    = charInfoArray[curCharacterIndex].hairB;

    mStats->topIndex = charInfoArray[curCharacterIndex].topIndex;
    mStats->topR    = charInfoArray[curCharacterIndex].topR;
    mStats->topG    = charInfoArray[curCharacterIndex].topG;
    mStats->topB    = charInfoArray[curCharacterIndex].topB;

    mStats->bottomIndex = charInfoArray[curCharacterIndex].bottomIndex;
    mStats->bottomR    = charInfoArray[curCharacterIndex].bottomR;
    mStats->bottomG    = charInfoArray[curCharacterIndex].bottomG;
    mStats->bottomB    = charInfoArray[curCharacterIndex].bottomB;

    mStats->imageFlags = charInfoArray[curCharacterIndex].imageFlags;

    mStats->avatarID = socketIndex;
    memcpy(mStats->name, charInfoArray[curCharacterIndex].name,31);
    mStats->name[31] = 0;

    mStats->cash = charInfoArray[curCharacterIndex].inventory->money;
    mStats->cLevel = charInfoArray[curCharacterIndex].cLevel;
    mStats->age = charInfoArray[curCharacterIndex].age;

}

//******************************************************************
void BBOSAvatar::GiveInfoFor(int x, int y, SharedSpace *ss)
{
    BBOSAvatar *curAvatar;
    MessAvatarAppear messAvAppear;
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);
    MessBladeDesc messBladeDesc;

    int tokenVal = tokenMan.TokenTypeInSquare(ss, x,y);
    if (tokenVal > -1)
    {
        // tell client about a token
        MessMobAppear messMA;
        messMA.mobID = (unsigned long) tokenVal;
        messMA.type = SMOB_TOKEN;
        messMA.x = x;
        messMA.y = y;
        ss->lserver->SendMsg(sizeof(messMA),(void *)&messMA, 0, &tempReceiptList);
    }

    BBOSMob *curMob = (BBOSMob *) ss->avatars->First();
    while (curMob)
    {
        if (SMOB_AVATAR == curMob->WhatAmI() && curMob != this && 
             curMob->cellX == x               && curMob->cellY == y &&
             FALSE == ((BBOSAvatar *) curMob)->isInvisible || ((BBOSAvatar *) curMob)->isTeleporting )
        {
            curAvatar = (BBOSAvatar *) curMob;
            messAvAppear.avatarID = curAvatar->socketIndex;
            messAvAppear.x = curAvatar->cellX;
            messAvAppear.y = curAvatar->cellY;
            ss->lserver->SendMsg(sizeof(messAvAppear),(void *)&messAvAppear, 0, &tempReceiptList);
            MessAvatarStats mStats;
            curAvatar->BuildStatsMessage(&mStats);
            ss->lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);

            // tell people about my cool dragons!
            for (int index = 0; index < 2; ++index)
            {
                if (255 != curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].type)
                {
                    // tell everyone about it!
                    MessPet mPet;
                    mPet.avatarID = curAvatar->socketIndex;
                    CopyStringSafely(curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].name,16, 
                                          mPet.name,16);
                    mPet.quality = curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].quality;
                    mPet.type    = curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].type;
                    mPet.state   = curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].state;
                    mPet.size    = curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].lifeStage +
                            curAvatar->charInfoArray[curAvatar->curCharacterIndex].petDragonInfo[index].healthModifier / 10.0f;
                    mPet.which   = index;

                    ss->lserver->SendMsg(sizeof(mPet),(void *)&mPet, 0, &tempReceiptList);
                }
            }

            curAvatar->AssertGuildStatus(ss, FALSE, socketIndex);

            InventoryObject *iObject = (InventoryObject *) 
                curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.First();
            while (iObject)
            {
                if (INVOBJ_BLADE == iObject->type)
                {
                    FillBladeDescMessage(&messBladeDesc, iObject, curAvatar);
                    ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
                    iObject = (InventoryObject *) 
                        curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Last();
                    
                }
                if (INVOBJ_STAFF == iObject->type) 
                {
                    InvStaff *iStaff = (InvStaff *) iObject->extra;
					if (iStaff->type == STAFF_TAMING)
						iStaff = NULL;// ignore taming staff
					else
					{
						// tell clients about staff
						messBladeDesc.bladeID = (long)iObject;
						messBladeDesc.size = 4;
						messBladeDesc.r = staffColor[iStaff->type][0];
						messBladeDesc.g = staffColor[iStaff->type][1];
						messBladeDesc.b = staffColor[iStaff->type][2];
						messBladeDesc.avatarID = curAvatar->socketIndex;
						messBladeDesc.trailType = 0;
						messBladeDesc.meshType = BLADE_TYPE_STAFF1;
						ss->lserver->SendMsg(sizeof(messBladeDesc), (void *)&messBladeDesc, 0, &tempReceiptList);
						// and skip to the end
						iObject = (InventoryObject *) 
							curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Last();
					}
                }
                iObject = (InventoryObject *) 
                    curAvatar->charInfoArray[curAvatar->curCharacterIndex].wield->objects.Next();
            }

        }

        curMob = (BBOSMob *) ss->avatars->Next();
    }

    curMob = ss->mobList->GetFirst(x,y);
    while (curMob)
    {
        if (curMob->cellX == x && curMob->cellY == y)
        {
            /*
            curAvatar = (BBOSAvatar *) curMob;
            messAvAppear.avatarID = curAvatar->socketIndex;
            messAvAppear.x = curAvatar->cellX;
            messAvAppear.y = curAvatar->cellY;
            lserver->SendMsg(sizeof(messAvAppear),(void *)&messAvAppear, 0, &tempReceiptList);
            MessAvatarStats mStats;
            curAvatar->BuildStatsMessage(&mStats);
            lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);
            */

            if (curMob->cellX == 0 && curMob->cellY == 3)
            {
                if (SMOB_MONSTER == curMob->WhatAmI() && ((BBOSMonster *) curMob)->subType > 5)
                {
                    ((BBOSMonster *) curMob)->subType = 0;
                    ((BBOSMonster *) curMob)->type = 2;
                    CopyStringSafely("ILLEGAL CREATURE", 32, ((BBOSMonster *) curMob)->uniqueName, 32);
                }
            }

            int sent = FALSE;

			if (SMOB_MONSTER == curMob->WhatAmI() && ((BBOSMonster *)curMob)->uniqueName[0])
			{
				BBOSMonster *curMonster = (BBOSMonster *)curMob;

				MessMobAppearCustom mAppear;
				mAppear.type = SMOB_MONSTER;
				mAppear.mobID = (unsigned long)curMob;
				mAppear.x = curMonster->cellX;
				mAppear.y = curMonster->cellY;
				mAppear.monsterType = curMonster->type;
				mAppear.subType = curMonster->subType;
				CopyStringSafely(curMonster->Name(), 32, mAppear.name, 32);
				mAppear.a = curMonster->a;
				mAppear.r = curMonster->r;
				mAppear.g = curMonster->g;
				mAppear.b = curMonster->b;
				mAppear.sizeCoeff = curMonster->sizeCoeff;

				if (SPACE_DUNGEON == ss->WhatAmI())
				{
					mAppear.staticMonsterFlag = FALSE;
					if (!curMonster->isWandering && !curMonster->isPossessed)
						mAppear.staticMonsterFlag = TRUE;
				}

				ss->lserver->SendMsg(sizeof(mAppear), (void *)&mAppear, 0, &tempReceiptList);
				sent = TRUE;
			}
			if (!sent && SMOB_PLAYERMERCHANT == curMob->WhatAmI())
			{
				char tempText[1024];

				BBOSNpc *curNpc = (BBOSNpc *)curMob;
				sprintf_s(tempText, "Showing Playermerchant owned by %s\n", curNpc->do_name);
				LogOutput("gamelog.txt", tempText);

				MessMobAppearCustom mAppear;
				mAppear.type = SMOB_PLAYERMERCHANT;
				mAppear.mobID = (unsigned long)curMob;
				mAppear.x = curNpc->cellX;
				mAppear.y = curNpc->cellY;
				mAppear.monsterType = 0;
				mAppear.subType = 0;
				CopyStringSafely(curNpc->do_name, 32, mAppear.name, 32);
				mAppear.a = 0;
				mAppear.r = 255;
				mAppear.g = 255;
				mAppear.b = 255;
				mAppear.sizeCoeff = 1.0;

				ss->lserver->SendMsg(sizeof(mAppear), (void *)&mAppear, 0, &tempReceiptList);
				sent = TRUE;
			}

            if (!sent && SMOB_MONSTER_GRAVE != curMob->WhatAmI() && 
                 //         SMOB_BOMB != curMob->WhatAmI() &&
                          SMOB_GROUND_EFFECT != curMob->WhatAmI()
                )
            {
                MessMobAppear mobAppear;
                mobAppear.mobID = (unsigned long) curMob;
                mobAppear.type = curMob->WhatAmI();

                if (SMOB_MONSTER == mobAppear.type)
                {
                    BBOSMonster *curMonster = (BBOSMonster *) curMob;
                    mobAppear.monsterType = curMonster->type;
                    mobAppear.subType = curMonster->subType;
                    if(SPACE_DUNGEON == ss->WhatAmI())
                    {
                        mobAppear.staticMonsterFlag = FALSE;
                        if (!curMonster->isWandering && !curMonster->isPossessed)
                            mobAppear.staticMonsterFlag = TRUE;
                    }
                }
                else if (SMOB_TREE == mobAppear.type)
                {
                    BBOSTree *curTree = (BBOSTree *) curMob;
                    mobAppear.monsterType = curTree->index;
                }
                else if (SMOB_WARP_POINT == mobAppear.type)
                {
                    mobAppear.subType = 0;

                    if ((((BBOSWarpPoint *)curMob)->allCanUse ||
                          ACCOUNT_TYPE_ADMIN == accountType) &&
                         ((BBOSWarpPoint *)curMob)->spaceType < 100)
                    mobAppear.subType = 1;
                }
                else
                    mobAppear.subType = 0;

                mobAppear.x = curMob->cellX;
                mobAppear.y = curMob->cellY;
                ss->lserver->SendMsg(sizeof(mobAppear),(void *)&mobAppear, 0, &tempReceiptList);

                if (SMOB_TOWER == mobAppear.type)
                {
                    MessCaveInfo cInfo;
                    cInfo.mobID       = (long) curMob;

                    if (((BBOSTower *)curMob)->isGuildTower)
                    {
                        TowerMap *dm = (TowerMap *) ((BBOSTower *)curMob)->ss;
                        cInfo.hasMistress = FALSE;
                        cInfo.type        = -1;
                    }
                    else
                    {
                        DungeonMap *dm = (DungeonMap *) ((BBOSTower *)curMob)->ss;
                        cInfo.hasMistress = dm->masterName[0];
                        cInfo.type        = dm->dungeonRating;
                    }
                    ss->lserver->SendMsg(sizeof(cInfo),(void *)&cInfo, 0, &tempReceiptList);
                }
                else if (SMOB_CHEST == mobAppear.type)
                {
                    MessChestInfo cInfo;
                    cInfo.mobID       = (long) curMob;
                    cInfo.type = ((BBOSChest *)curMob)->isOpen;
                    ss->lserver->SendMsg(sizeof(cInfo),(void *)&cInfo, 0, &tempReceiptList);
                }
            }
            else if (!sent && SMOB_GROUND_EFFECT == curMob->WhatAmI())
            {
                BBOSGroundEffect *bboGE = (BBOSGroundEffect *) curMob;
                MessGroundEffect messGE;
                messGE.mobID  = (unsigned long) bboGE;
                messGE.type   = bboGE->type;
                messGE.amount = bboGE->amount;
                messGE.x      = bboGE->cellX;
                messGE.y      = bboGE->cellY;
                messGE.r      = bboGE->r;
                messGE.g      = bboGE->g;
                messGE.b      = bboGE->b;

                ss->lserver->SendMsg(sizeof(messGE),(void *)&messGE, 0, &tempReceiptList);
            }
        }
        curMob = ss->mobList->GetNext();
    }

}

char bombSizeNames[15][20] =
{
    {"Tiny"},
    {"Small"},
    {"Plain"},
    {"Big"},
    {"Hefty"},
    {"Huge"},
    {"Massive"},
    {"Colossal"},
    {"Enormous"},
    {"Gigantic"},
    {"Immense"},
    {"Titanic"},
    {"Nuclear"},
    {"Thermonuclear"},
    {"Doomsday"}
};


//******************************************************************
void BBOSAvatar::Combine(SharedSpace *ss)
{

    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    char tempText[1024];
    MessInfoText infoText;


    //         ***********************************
    if (!strcmp(combineSkillName,"Swordsmith"))
    {
        //1)	determine the size the sword will be from the total number of ingots in 
        // the workbench-> Extra ingots will be used in the construction of the weapon, 
        // and the actual size of the weapon will reflect how many ingots got used.  
        // This test is only to choose part of the resulting name string and base damage.
        InventoryObject *io;
        int numIngots = 0;
        int allR = 0;
        int allG = 0;
        int allB = 0;
        float allDamage = 0, allCost = 0, challenge = 0;
        int dustAmount[INGR_MAX];
        int totalDust = 0;
        bool shardIncluded = false;			// this is to make sure we don't use more than one shard

        for (int i = 0; i < INGR_MAX; ++i)
            dustAmount[i] = 0;

        char ingotNames[20][64]; // assumes no more than 20 ingot types, ever!
        int ingotAmount[20], ingotAmount2[20];;

        for (int i = 0; i < 20; ++i) {
            ingotAmount[i] = 0;
            ingotAmount2[i] = 0;
        }

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_INGREDIENT == io->type)
            {
                InvIngredient * ingre = (InvIngredient *)io->extra;

                if (ingre->type < 0 || ingre->type > INGR_MAX)
                {
                    sprintf(tempText,"Ingredient type out of bounds!  Please report this bug.");
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    return;
                }

                // make sure we only include one shard
                if( ingre->type >= INGR_WHITE_SHARD && ingre->type <= INGR_PINK_SHARD ) {
                    if( !shardIncluded && io->amount == 1 ) {
                        dustAmount[ingre->type] += io->amount;
                        totalDust += io->amount;
                        shardIncluded = true;
                    }
                    else {
                        sprintf(tempText,"You can only have one shard in your workbench when you craft.");
                        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
                        
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        return;
                    }
                }
                else {
                    dustAmount[ingre->type] += io->amount;
                    totalDust += io->amount;
					if (ingre->type == INGR_GOLD_DUST) // if it's gold dust
					{
						totalDust += io->amount;  // it counts quadruple for task difficulty purposes.
						totalDust += io->amount;  // this wont' affect how many are put on, even when they do start gettign storerd in the future.
						totalDust += io->amount;  // copy or remove these lines to make it more or less difficult.
					}
                }
                    
            }

            if (INVOBJ_INGOT == io->type)
            {
                int usableAmount = io->amount;
                if (usableAmount > 64)
                    usableAmount = 64;

                numIngots += 1 * usableAmount;
                InvIngot *ingotInfo = (InvIngot *) io->extra;

                //4)	 Get the sum total challenge level of all the ingots, as the 
                // task challenge level
                challenge += ingotInfo->challenge * usableAmount;
                allR += ingotInfo->r * usableAmount;
                allG += ingotInfo->g * usableAmount;
                allB += ingotInfo->b * usableAmount;
                allDamage +=  ingotInfo->damageVal * usableAmount;
                allCost += io->value  * usableAmount;

                for (int i = 0; i < 20; ++i)
                {
                    if (ingotAmount[i] > 0 && !strcmp(ingotNames[i],io->WhoAmI()))
                    {
                        ingotAmount[i] = 1  * usableAmount;
                        ingotAmount2[ (int)ingotInfo->challenge - 1 ] = ingotAmount[i];
                    }
                    else if (0 == ingotAmount[i])
                    {
                        ingotAmount[i] = 1 * usableAmount;
                        sprintf(ingotNames[i],io->WhoAmI());
                        ingotAmount2[ (int)ingotInfo->challenge - 1] = ingotAmount[i];
                        i = 21;
                    }
                }

            }

            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        if (numIngots > 64)
        {
            challenge += challenge * (1.0f + (numIngots-64) / 32.0f);
        }

//		for (i = 0; i < INGR_MAX; ++i)
        challenge += (challenge / 5) * totalDust; // +20% for each ingredient

        //2)	Find out the first most abundant type of ingot
        int candidate = -1, most = 0;
        for (int i = 0; i < 20; ++i)
        {
            if (most < ingotAmount[i])
            {
                most = ingotAmount[i];
                candidate = i;
            }
        }

        if (candidate < 0)
        {
            sprintf(tempText,"Use ingots to make blades.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
        else
        {
            //3)	Find out the second most abundant type of ingot
            ingotAmount[candidate] = 0;
            int candidate2 = -1, most2 = 0;
            for (int i = 0; i < 20; ++i)
            {
                if (most2 < ingotAmount[i])
                {
                    most2 = ingotAmount[i];
                    candidate2 = i;
                }
            }

            //5)	IF the task is successful,
            // B)	work = skill level * creativity * rnd(0.8,1.2)
            // C)	if Challenge <= work, success!

            InvSkill *skillInfo = NULL;
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
            while (io)
            {
                if (!strcmp(combineSkillName,io->WhoAmI()))
                {
                    skillInfo = (InvSkill *) io->extra;
                }
                io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
            }

            if (!skillInfo)
            {
                sprintf(tempText,"BUG: Cannot find skill for level.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else
            {
                float work = skillInfo->skillLevel * 
                              CreativeStat() *
                                 rnd(0.3f,1.7f);

                if (tokenMan.TokenIsInHere(MAGIC_WOLF, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }
                if (tokenMan.TokenIsInHere(MAGIC_BEAR, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }
                if (tokenMan.TokenIsInHere(MAGIC_EAGLE, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }

                if (specLevel[2] > 0)
                    work *= (1.0f + 0.05f * specLevel[2]);

                sprintf(tempText,"Task difficulty is %4.2f.", challenge);
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf(tempText,"Your work value was %4.2f.", work);
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                if (challenge <= work) // success!
                {
                    bool shardDestroyed = false;

                    // destroy everything from workbench
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {
                        if (INVOBJ_INGOT == io->type)
                        {
                            if (io->amount > 64)
                            {
                                io->amount -= 64;
                            }
                            else
                            {
                                charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                                delete io;
                            }
                        }
                        else if (INVOBJ_INGREDIENT == io->type)
                        {
                            if( io->amount > 1 && ((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type <= INGR_PINK_SHARD && !shardDestroyed ) {
                                --(io->amount);
                                shardDestroyed = true;
                            }
                            else if( !( ((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type <= INGR_PINK_SHARD ) || !shardDestroyed )
                            {
                                if( ( ((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type <= INGR_PINK_SHARD ) )
                                    shardDestroyed = true;

                                charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                                delete io;
                            }
                        }
                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                    //6)	The name of the sword is a fusion of the 
                    //    first and second ingot types, 
                    //    and the size (Carbon-Zinc Short Sword),
                    if (candidate2 > -1)
                    {
                        sscanf (ingotNames[candidate],"%s", &(tempText[0]));
                        sprintf(&(tempText[strlen(tempText)]), "-");
                        sscanf (ingotNames[candidate2],"%s", &(tempText[strlen(tempText)]));
                    }
                    else
                    {
                        sscanf (ingotNames[candidate],"%s", &(tempText[0]));
                    }
                    sprintf(&(tempText[strlen(tempText)]), " ");

                    int candidate3 = 6, most3 = 0;
                    for (int j = 0; j < 6; ++j)
                    {
                        if (bladeList[j].size > numIngots)
                        {
                            candidate3 = j-1;
                            j = 7;
                        }
                    }

                    if (candidate3 < 0)
                        candidate3 = 0;
                    if (candidate3 > 5)
                        candidate3 = 5;

                    sprintf(&(tempText[strlen(tempText)]), bladeList[candidate3].name);

                    // make the new blade!
                    io = new InventoryObject(INVOBJ_BLADE,0,tempText);
                    InvBlade *ib = (InvBlade *) io->extra; 

                    //7)	The damage of the sword is the average damage value of all ingots * the size base damage
                    ib->damageDone = allDamage * bladeList[candidate3].damage / numIngots;

                    //8)	The value of the sword is the sum of the costs of all the ingots * 1.4
                    io->value = allCost * 1.4f;

                    //9)	The color of the sword blade is the average of the color of all the ingots
                    ib->r = allR / numIngots;
                    ib->g = allG / numIngots;
                    ib->b = allB / numIngots;

                    ib->toHit = 1 + dustAmount[INGR_BLUE_DUST];
                    ib->size = 1.0 + numIngots / 16.0f;

                    ib->poison = dustAmount[INGR_BLACK_DUST];
                    ib->heal   = dustAmount[INGR_GREEN_DUST];
                    ib->slow   = dustAmount[INGR_RED_DUST];
                    ib->blind  = dustAmount[INGR_WHITE_DUST];
					ib->lightning = dustAmount[INGR_SILVER_DUST];
					ib->tame = dustAmount[INGR_GOLD_DUST];


                    ib->tinIngots = ingotAmount2[0];
                    ib->aluminumIngots = ingotAmount2[1];
                    ib->steelIngots = ingotAmount2[2];
                    ib->carbonIngots = ingotAmount2[3];
                    ib->zincIngots = ingotAmount2[4];
                    ib->adamIngots = ingotAmount2[5];
                    ib->mithIngots = ingotAmount2[6];
                    ib->vizIngots = ingotAmount2[7];
                    ib->elatIngots = ingotAmount2[8];
                    ib->chitinIngots = ingotAmount2[9];
					ib->maligIngots = ingotAmount2[10];
					ib->tungstenIngots = ingotAmount2[11];
					ib->titaniumIngots = ingotAmount2[12];
					ib->azraelIngots = ingotAmount2[13];
					ib->chromeIngots = ingotAmount2[14];

                    int mostIngr = -1, ingrCount = 0;
                    for (int i = 0; i < INGR_WHITE_SHARD; ++i)
                    {
                        if (dustAmount[i] > ingrCount)
                        {
                            ingrCount = dustAmount[i];
                            mostIngr = i;
                        }
                    }

                    if (mostIngr > -1)
                    {
                        switch(mostIngr)
                        {
                        case INGR_BLUE_DUST:
                        default:
                            ib->bladeGlamourType = BLADE_GLAMOUR_TOHIT1;
                            break;
                        case INGR_GREEN_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_HEAL1;
                            break;
                        case INGR_BLACK_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_POISON1;
                            break;
                        case INGR_WHITE_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_BLIND1;
                            break;
                        case INGR_RED_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_SLOW1;
                            break;
                        }

                        if (ingrCount > 1)
                            ++(ib->bladeGlamourType);
                        if (ingrCount > 4)
                            ++(ib->bladeGlamourType);
                        if (ingrCount > 9)
                            ++(ib->bladeGlamourType);
                    }

                    int shardIndex = -1;
                    for (int i = INGR_WHITE_SHARD; i <= INGR_PINK_SHARD; ++i)
                    {
                        if (dustAmount[i] > 0)
                        {
                            shardIndex = i-INGR_WHITE_SHARD;
                            i = INGR_MAX;
                        }
                    }

                    if (shardIndex > -1)
                    {
                        ib->bladeGlamourType = BLADE_GLAMOUR_TRAILWHITE + shardIndex;

                        ib->damageDone += ib->damageDone/10;
                    }

                    sprintf(tempText,"You have created a");
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    sprintf(tempText,"%s!",io->WhoAmI());
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    QuestCraftWeapon(ss, io, challenge, work, combineSkillName);

                    charInfoArray[curCharacterIndex].inventory->AddItemSorted( io );
                }
                else
                {
                    // D)	ELSE, for each item in the workbench, 
                    //    the chance of being destroyed = 1.0 -(work/challenge) (0.0 to 1.0)
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {
                        int randomVal = rnd(0,5);

                    //    the chance of being destroyed = 1 - (work/challenge) (0.0 to 1.0)
                        if (
                               work/challenge < rnd(0,1) &&
                             ( INVOBJ_INGOT == io->type || ( 
                               INVOBJ_INGREDIENT == io->type && ((InvIngredient *)io->extra)->type < INGR_WHITE_SHARD )  ||
                             ( INVOBJ_INGREDIENT == io->type && ((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD &&
                               INVOBJ_INGREDIENT == io->type && ((InvIngredient *)io->extra)->type < INGR_MAX && randomVal == 2 )
                             )
                             )
                        {
							// pure smiths don't lose shards anymore
							if ((charInfoArray[curCharacterIndex].creative > 9) && (INVOBJ_INGREDIENT == io->type && 
								((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type < INGR_GOLD_DUST)) // it's a shard!
							{
								io->amount++; // add 1 to the amount, so it's not really destroyed;
							}
                            if (io->amount > 1)
                                io->amount--;
                            else
                            {
                                charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                                delete io;
                            }
                        }

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                }

                // In any case, you get skill sub-points for that skill.
                if( skillInfo->skillLevel < 30000 && challenge > work / 20 )
                {
                    skillInfo->skillPoints += work* bboServer->smith_exp_multiplier;

                    if (skillInfo->skillLevel * skillInfo->skillLevel * 30 <= skillInfo->skillPoints)
                    {
                        // made a skill level!!!
                        skillInfo->skillLevel++;
                        sprintf(tempText,"You gained a skill level!!");
                        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                        
                        charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_SMITH;

                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                }
            }
        }
        return;
    }


    //         ***********************************
    if (!strcmp(combineSkillName,"Katana Expertise")     ||
         !strcmp(combineSkillName,"Claw Expertise")       ||
         !strcmp(combineSkillName,"Chaos Expertise")      ||
		!strcmp(combineSkillName, "Bladestaff Expertise") ||
		!strcmp(combineSkillName, "Scythe Expertise") ||
		!strcmp(combineSkillName,"Mace Expertise")           )
    {
        //1)	determine the size the sword will be from the total number of ingots in 
        // the workbench-> Extra ingots will be used in the construction of the weapon, 
        // and the actual size of the weapon will reflect how many ingots got used.  
        // This test is only to choose part of the resulting name string and base damage.
        InventoryObject *io;
        int numIngots = 0;
        InventoryObject *oldBlade = NULL;
        int numOldBlades = 0;
        int allR = 0;
        int allG = 0;
        int allB = 0;
        float allDamage = 0, allCost = 0, challenge = 0;
        int dustAmount[INGR_MAX];
        int totalDust = 0;
        bool shardIncluded = false;			// this is to make sure we don't use more than one shard
		int staffcount = 0;
		int chargecount = 0;
		int prevstafftype = -1;
		int prevstaffquality = -1;
		bool samestafftype = true;
		bool prevstaffimbued = false;
		int staffimbuetype = -1;
		int ingotforstaffeffect = -1;
		int mostingots = -1;
		for (int i = 0; i < INGR_MAX; ++i)
            dustAmount[i] = 0;

        char ingotNames[20][64]; // assumes no more than 20 ingot types, ever!
        int ingotAmount[20], ingotAmount2[20];

        for (int i = 0; i < 20; ++i) {
            ingotAmount[i] = 0;
            ingotAmount2[i] = 0;
        }

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
			if (strcmp(combineSkillName, "Scythe Expertise")) // dont look for dusta and shards for scythe expertise.
				if (INVOBJ_INGREDIENT == io->type)
	            {
		            InvIngredient * ingre = (InvIngredient *)io->extra;

	                if (ingre->type < 0 || ingre->type > INGR_MAX)
		            {
			            sprintf(tempText,"Ingredient type out of bounds!  Please report this bug.");
				        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
					    
						ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
						return;
					}

                // make sure we only include one shard
	                if (ingre->type >= INGR_WHITE_SHARD && ingre->type <= INGR_PINK_SHARD ) 
					{
		                if( !shardIncluded && io->amount == 1 ) 
						{
			                dustAmount[ingre->type] += io->amount;
				            totalDust += io->amount;
					        shardIncluded = true;
						}
						else {
							sprintf(tempText,"You can only have one shard in your workbench when you craft.");
	                        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
		                    
			                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
				            return;
					    }
					}
					else 
					{
						dustAmount[ingre->type] += io->amount;
						totalDust += io->amount;
						if (ingre->type == INGR_GOLD_DUST) // if it's gold dust
						{
							totalDust += io->amount;  // it counts quadruple for task difficulty purposes.
							totalDust += io->amount;  // this won't affect how many are put on, even when they do start getting storerd in the future.
							totalDust += io->amount;  // copy or remove these lines to make it more or less difficult.
						}
					}
				}
			if (!strcmp(combineSkillName, "Scythe Expertise")) // if we are doing scythe.
				if (io->type == INVOBJ_STAFF)
				{	
					InvStaff * ingre = ((InvStaff *)io->extra);
					if (prevstaffquality< 0 || ingre->quality == prevstaffquality)
					{
						if (ingre->charges > 0 && (!prevstaffimbued && (staffcount > 0))) // imbued staff after uninbued.
						{ // abort, can't mix imbue and unimbued
							sprintf(tempText, "Can't mix imbued and unimbued staffs.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							return;
						}
						if (ingre->imbueDeviation>0 && ingre->charges>0) // imperfect staff, no charges means not imbued at all.
						{ // abort, can't use imperfect staff
							sprintf(tempText, "Can't use an imperfect staff.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							return;
						}
						if (ingre->charges < 1 && (prevstaffimbued)) // unimbued after imbued.
						{ // abort, can't mix imbue and unimbued
							sprintf(tempText, "Can't mix imbued and unimbued staffs.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							return;
						}
						if ((prevstafftype > -1) && (ingre->type!=prevstafftype))
						{ // abort, can't mix staff types
							sprintf(tempText, "Can't mix different imbued staffs.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							return;
						}
						// allowed staff checks
						if (ingre->charges > 0 && ingre->type < 13) // taming staff only for now.
						{ // abort, its' a fire staff.
							sprintf(tempText, "Only taming staffs may be used inbued.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);

							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
							return;
						}
//						if (ingre->charges > 0 && ingre->type < 1) // fire staff not allowed
//						{ // abort, its' a fire staff.
//							sprintf(tempText, "Can't make a scythe with a fire staff.");
//							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
//						
//							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
//							return;
//						}
						//						if (ingre->charges > 0 && ingre->type > 5) // din staffs not allowed
//						{ // abort, its' a fire staff.
//							sprintf(tempText, "Can't make a scythe with a Din staff.");
//							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
//							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
//							return;
//						}
								
						// note	that extra unimbued staffs don't actualyl make the weapon better, they just add TD for skilling.
						prevstaffquality = ingre->quality;	// save staff quality we found
						prevstafftype = ingre->type;		// save staff type we found
						staffcount+=io->amount;						// increase the staff counter
						chargecount += ingre->charges*io->amount;		// increase the total of charges to transfer
						if (ingre->charges > 0)							// set flag to allow the next staff to also be imbued
							prevstaffimbued = true;						// don't need to set it back to false, cause it will fail before then.
						
					}
					else
					{
						sprintf(tempText, "You can only have one quality of staff when making a scythe.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						return;
					}
					
				}
            if (INVOBJ_BLADE == io->type)
            {
                numOldBlades += io->amount;
                oldBlade = io;

            }

			if (strcmp(combineSkillName, "Scythe Expertise")) // dont look for dusts and shards for scythe expertise.
				if (INVOBJ_INGOT == io->type)
				{
					int usableAmount = io->amount;
	                if (usableAmount > 32)
                    usableAmount = 32;

		            numIngots += 1 * usableAmount;
			        InvIngot *ingotInfo = (InvIngot *) io->extra;

	                //4)	 Get the sum total challenge level of all the ingots, as the 
		            // task challenge level
			        challenge += ingotInfo->challenge * usableAmount;
				    allR += ingotInfo->r * usableAmount;
	                allG += ingotInfo->g * usableAmount;
	                allB += ingotInfo->b * usableAmount;
	                allDamage +=  ingotInfo->damageVal * usableAmount;
	                allCost += io->value  * usableAmount;

	                for (int i = 0; i < 20; ++i)
		            {
			            if (ingotAmount[i] > 0 && !strcmp(ingotNames[i],io->WhoAmI()))
				        {
					        ingotAmount[i] = 1  * usableAmount;
						    ingotAmount2[ (int)ingotInfo->challenge - 1 ] = ingotAmount[i];
	                    }
		                else if (0 == ingotAmount[i])
			            {
				            ingotAmount[i] = 1 * usableAmount;
					        sprintf(ingotNames[i],io->WhoAmI());
						    ingotAmount2[ (int)ingotInfo->challenge - 1 ] = ingotAmount[i];
	                        i = 21;
						}
					}

				}

            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }
        // find and check the Swordsmith skill!
        InvSkill *skillInfo = NULL;
        io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
        while (io)
        {
            if (!strcmp("Swordsmith",io->WhoAmI()))
            {
                skillInfo = (InvSkill *) io->extra;
            }
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
        }

        if (!skillInfo)
        {
            sprintf(tempText,"You must first learn to create swords.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        if (skillInfo->skillLevel < 20)
        {
            sprintf(tempText,"You must first reach level 20 in your Swordsmith skill.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        // do you have to required sacrifical blade?
        if (!oldBlade || numOldBlades < 1 || ((InvBlade *)oldBlade->extra)->type != BLADE_TYPE_NORMAL)
        {
            sprintf(tempText,"You must sacrifice one blade to make this.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        float oldBladeSize = ((InvBlade *)oldBlade->extra)->size;
        if (oldBladeSize < 1.0 + 32 / 16.0f)
        {
            sprintf(tempText,"The sacrifical blade is not big enough.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }
		// now that we have found all the stuff do scythe specific checks
		if (!strcmp(combineSkillName, "Scythe Expertise")) // if we are making a scythe
		{													// we need ot get the best ingot
															//get most common ingot, and it's amount
															// get most common ingot;
			mostingots = ((InvBlade *)oldBlade->extra)->tinIngots;
			if (mostingots > 1)
				ingotforstaffeffect = 0;
			if (((InvBlade *)oldBlade->extra)->aluminumIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->aluminumIngots;
				ingotforstaffeffect = 1;
			}
			if (((InvBlade *)oldBlade->extra)->steelIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->steelIngots;
				ingotforstaffeffect = 2;
			}
			if (((InvBlade *)oldBlade->extra)->carbonIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->carbonIngots;
				ingotforstaffeffect = 3;
			}
			if (((InvBlade *)oldBlade->extra)->zincIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->zincIngots;
				ingotforstaffeffect = 4;
			}
			if (((InvBlade *)oldBlade->extra)->adamIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->adamIngots;
				ingotforstaffeffect = 5;
			}
			if (((InvBlade *)oldBlade->extra)->mithIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->mithIngots;
				ingotforstaffeffect = 6;
			}
			if (((InvBlade *)oldBlade->extra)->vizIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->vizIngots;
				ingotforstaffeffect = 7;
			}
			if (((InvBlade *)oldBlade->extra)->elatIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->elatIngots;
				ingotforstaffeffect = 8;
			}
			if (((InvBlade *)oldBlade->extra)->chitinIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->chitinIngots;
				ingotforstaffeffect = 9;
			}
			if (((InvBlade *)oldBlade->extra)->maligIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->maligIngots;
				ingotforstaffeffect = 10;
			}
			if (((InvBlade *)oldBlade->extra)->tungstenIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->tungstenIngots;
				ingotforstaffeffect = 11;
			}
			if (((InvBlade *)oldBlade->extra)->titaniumIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->titaniumIngots;
				ingotforstaffeffect = 12;
			}
			if (((InvBlade *)oldBlade->extra)->azraelIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->azraelIngots;
				ingotforstaffeffect = 13;
			}
			if (((InvBlade *)oldBlade->extra)->chromeIngots >= mostingots)
			{
				mostingots = ((InvBlade *)oldBlade->extra)->chromeIngots;
				ingotforstaffeffect = 14;
			}
			if (mostingots<64) // gs only for scythe
			{
				sprintf(tempText, "The sacrifical blade is not a proper greatsword.");
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				return;
			}
			if ((((InvBlade *)oldBlade->extra)->tame<1) && (chargecount > 0 ))
			{
				sprintf(tempText, "The sacrifical blade does not have any Gold Dust.");
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				return;
			}

			if (chargecount > 0) // its'an imbued scythe
			{
				chargecount *= 2; // double it because only half hit the whole square.
				staffimbuetype = prevstafftype; // set imbuetype for later
			}
			if (prevstaffquality > -1) // it's at least pine
				numIngots = 1;
			if (prevstaffquality > 0) // it's at least birch
				numIngots = 2;
			if (prevstaffquality > 1) // it's at least spruce
				numIngots = 4;
			if (prevstaffquality > 2) // it's at least oak
				numIngots = 8;
			if (prevstaffquality > 3) // it's at least onyx
				numIngots = 16;
			if (prevstaffquality > 4) // it's at least elm
				numIngots = 32;
			numIngots = numIngots * staffcount;  
		}
		if (!strcmp(combineSkillName, "Scythe Expertise")) // if we are making a scythe
		{  // then we don't have a base difficulty, and need to set it.
			challenge = (ingotforstaffeffect + 1)*numIngots;  // based on quality of greatsword and on quality and number of staffs
			if (challenge < 1) 
			{ // ABORT ABORT
				sprintf(tempText, "Please use both a staff and a greatsword.");
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);

				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				return;
			}
		}
		else if (numIngots > 32) // we already have a challenge figured for 32 ingots, and we apply multiplier if the total is greater then 32.
        {
            challenge += challenge * (1.0f + (numIngots-32) / 32.0f);
        }
		
//		for (i = 0; i < INGR_MAX; ++i)
        challenge += (challenge / 5) * totalDust; // +20% for each ingredient, but there aren't any for a scythe.

        //2)	Find out the first most abundant type of ingot
        int candidate = -1, most = 0;
		int candidate2 = -1, most2 = 0;
		if (strcmp(combineSkillName, "Scythe Expertise")) // if we aren't making a scythe
		{
			for (int i = 0; i < 20; ++i)
			{
				if (most < ingotAmount[i])
				{
					most = ingotAmount[i];
					candidate = i;
				}
			}
		}
		else candidate = ingotforstaffeffect; // we use the one we identified in our scythe check.
        if (candidate < 0)
        {
            sprintf(tempText,"Use ingots to make weapons.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
		else 
		{
			//	Find out the second most abundant type of ingot
			if (strcmp(combineSkillName, "Scythe Expertise")) // if we aren't making a scythe
			{
				ingotAmount[candidate] = 0;
				for (int i = 0; i < 20; ++i)
				{
					if (most2 < ingotAmount[i])
					{
						most2 = ingotAmount[i];
						candidate2 = i;
					}
				}
			}
			// candidate2 will not be changed from -1 if it's a scythe.
            //5)	IF the task is successful,
            // B)	work = skill level * creativity * rnd(0.8,1.2)
            // C)	if Challenge <= work, success!

            skillInfo = NULL;
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
            while (io)
            {
                if (!strcmp(combineSkillName,io->WhoAmI()))
                {
                    skillInfo = (InvSkill *) io->extra;
                }
                io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
            }

            if (!skillInfo)
            {
                sprintf(tempText,"BUG: Cannot find skill for level.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else
            {
                float work = (20 * skillInfo->skillLevel) * 
                              CreativeStat() *
                                 rnd(0.3f,1.7f);

                if (tokenMan.TokenIsInHere(MAGIC_WOLF, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }
                if (tokenMan.TokenIsInHere(MAGIC_BEAR, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }
                if (tokenMan.TokenIsInHere(MAGIC_EAGLE, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }

                if (specLevel[2] > 0)
                    work *= (1.0f + 0.05f * specLevel[2]);

                sprintf(tempText,"Task difficulty is %4.2f.", challenge);
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf(tempText,"Your work value was %4.2f.", work);
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                if (challenge <= work) // success!
                {
                    // destroy everything from workbench
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {
                        bool shardDestroyed = false;
						if (strcmp(combineSkillName, "Scythe Expertise")) // if we aren't making a scythe
						{													// then remove ingots and dusts.
							if (INVOBJ_INGOT == io->type)
							{
								if (io->amount > 32)
								{
									io->amount -= 32;
								}
								else
								{
									charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
									delete io;
								}
							}
							else if (INVOBJ_INGREDIENT == io->type)
							{
								if (io->amount > 1 && ((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type <= INGR_PINK_SHARD && !shardDestroyed) {
									--(io->amount);
									shardDestroyed = true;
								}
								else if (!(((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type <= INGR_PINK_SHARD) || !shardDestroyed)
								{
									if ((((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD && ((InvIngredient *)io->extra)->type <= INGR_PINK_SHARD))
										shardDestroyed = true;

									charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
									delete io;
								}
							}
						}
						else
						{					// we remove the staffs instead.
							if (INVOBJ_STAFF == io->type)
							{
								charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
								delete io;
							}

						}
                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                    //6)	The name of the sword is a fusion of the 
                    //    first and second ingot types, 
                    //    and the size (Carbon-Zinc Short Sword), for, no scythes
					if (strcmp(combineSkillName, "Scythe Expertise")) 
					{
						if (candidate2 > -1)
						{
							sscanf(ingotNames[candidate], "%s", &(tempText[0]));
							sprintf(&(tempText[strlen(tempText)]), "-");
							sscanf(ingotNames[candidate2], "%s", &(tempText[strlen(tempText)]));
						}
						else
						{
							sscanf(ingotNames[candidate], "%s", &(tempText[0]));
						}
					}
					else
					{
						// we instead grab the ingot name from the main ingot names area
						sscanf(ingotNameList[candidate], "%s", &(tempText[0]));
						// and then print in the staff type
						if (staffimbuetype>0) // omgwtf
						{
							sprintf(&(tempText[strlen(tempText)]), " ");
							sprintf(&(tempText[strlen(tempText)]), staffTypeName[staffimbuetype]);
						}
						if (prevstafftype > 12)
						{
							prevstafftype = prevstafftype - 6; // correct for gap left by din staffs;
						}

					}
					sprintf(&(tempText[strlen(tempText)]), " ");

                    int candidate3 = 3, most3 = 0;
					if (strcmp(combineSkillName, "Scythe Expertise")) // if we aren't making a scythe
					{                                                 // determine size from number of ingots.
						for (int j = 0; j < 3; ++j)
						{
							if (katanaList[j].size > numIngots)
							{
								candidate3 = j - 1;
								j = 4;
							}
						}

						if (candidate3 < 0)
							candidate3 = 0;
						if (candidate3 > 2)
							candidate3 = 2;
					}
					else  // it IS a scythe
					{   // we determine it from wood quality.
						candidate3 = prevstaffquality;
					}
					// and add the type of staff to the name if it's a scythe;
                   if (!strcmp(combineSkillName,"Katana Expertise")     )
                        sprintf(&(tempText[strlen(tempText)]), katanaList[candidate3].name);
                    else if (!strcmp(combineSkillName,"Claw Expertise")       )
                        sprintf(&(tempText[strlen(tempText)]), clawList[candidate3].name);
                    else if (!strcmp(combineSkillName,"Chaos Expertise")      )
                        sprintf(&(tempText[strlen(tempText)]), chaosList[candidate3].name);
                    else if (!strcmp(combineSkillName,"Bladestaff Expertise") )
						sprintf(&(tempText[strlen(tempText)]), bladestaffList[candidate3].name);
					else if (!strcmp(combineSkillName, "Mace Expertise"))
						sprintf(&(tempText[strlen(tempText)]), maceList[candidate3].name);
					else if (!strcmp(combineSkillName, "Scythe Expertise"))
						sprintf(&(tempText[strlen(tempText)]), scytheList[candidate3].name);

                    // make the new blade!
                    io = new InventoryObject(INVOBJ_BLADE,0,tempText);
                    InvBlade *ib = (InvBlade *) io->extra; 

                    //7)	The damage of the sword is the average damage value of all ingots * the size base damage
					if (strcmp(combineSkillName, "Scythe Expertise"))
					{
						ib->damageDone = allDamage * katanaList[candidate3].damage / numIngots / 2;
						ib->damageDone += ((InvBlade *)oldBlade->extra)->damageDone / 2;
					}
					else // it's a scythe, just copy the old blades over, you can't use a half of a staff.
						ib->damageDone = ((InvBlade *)oldBlade->extra)->damageDone;
					// the 20 is the katana's special enhancement
                    ib->toHit = dustAmount[INGR_BLUE_DUST] + ((InvBlade *)oldBlade->extra)->toHit;
                    ib->size = 1.0 + (numIngots + 32) / 16.0f;
					if ((ib->size > 5.0f) && (!strcmp(combineSkillName, "Scythe Expertise"))) // if the scythe would be too big
					{
						int staffcount = ib->size / 32;
						ib->size = staffcount+4.0f;
						if (ib->size > 25.0f)
							ib->size = 25.0f;
					}
					if (strcmp(combineSkillName, "Scythe Expertise")) // if not a scythe
						ib->size *= 0.7f;
                    
					if (strcmp(combineSkillName, "Scythe Expertise")) // if we aren't making a scythe
					{												  // add in the ingots we used.
						ib->tinIngots = ingotAmount2[0] + ((InvBlade *)oldBlade->extra)->tinIngots;
						ib->aluminumIngots = ingotAmount2[1] + ((InvBlade *)oldBlade->extra)->aluminumIngots;
						ib->steelIngots = ingotAmount2[2] + ((InvBlade *)oldBlade->extra)->steelIngots;
						ib->carbonIngots = ingotAmount2[3] + ((InvBlade *)oldBlade->extra)->carbonIngots;
						ib->zincIngots = ingotAmount2[4] + ((InvBlade *)oldBlade->extra)->zincIngots;
						ib->adamIngots = ingotAmount2[5] + ((InvBlade *)oldBlade->extra)->adamIngots;
						ib->mithIngots = ingotAmount2[6] + ((InvBlade *)oldBlade->extra)->mithIngots;
						ib->vizIngots = ingotAmount2[7] + ((InvBlade *)oldBlade->extra)->vizIngots;
						ib->elatIngots = ingotAmount2[8] + ((InvBlade *)oldBlade->extra)->elatIngots;
						ib->chitinIngots = ingotAmount2[9] + ((InvBlade *)oldBlade->extra)->chitinIngots;
						ib->maligIngots = ingotAmount2[10] + ((InvBlade *)oldBlade->extra)->maligIngots;
						ib->tungstenIngots = ingotAmount2[11] + ((InvBlade *)oldBlade->extra)->tungstenIngots;
						ib->titaniumIngots = ingotAmount2[12] + ((InvBlade *)oldBlade->extra)->titaniumIngots;
						ib->azraelIngots = ingotAmount2[13] + ((InvBlade *)oldBlade->extra)->azraelIngots;
						ib->chromeIngots = ingotAmount2[14] + ((InvBlade *)oldBlade->extra)->chromeIngots;
					}
					else // we instead add in the right number for the quality of teh staff
					{   // first we decide number of ingots
						int scytheingotshack;
						if (prevstaffquality == 0)
							scytheingotshack = 1;
						if (prevstaffquality == 1)
							scytheingotshack = 2;
						if (prevstaffquality == 2)
							scytheingotshack = 4;
						if (prevstaffquality == 3)
							scytheingotshack = 8;
						if (prevstaffquality == 4)
							scytheingotshack = 16;
						if (prevstaffquality == 5)
							scytheingotshack = 32;
						// then add it to the correct ingot.
						if (ingotforstaffeffect == 0)
						{
							ib->tinIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->tinIngots;
						}
						if (ingotforstaffeffect == 1)
						{
							ib->aluminumIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->aluminumIngots;
						}
						if (ingotforstaffeffect == 2)
						{
							ib->steelIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->steelIngots;
						}
						if (ingotforstaffeffect == 3)
						{
							ib->carbonIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->carbonIngots;
						}
						if (ingotforstaffeffect == 4)
						{
							ib->zincIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->zincIngots;
						}
						if (ingotforstaffeffect == 5)
						{
							ib->adamIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->adamIngots;
						}
						if (ingotforstaffeffect == 6)
						{
							ib->mithIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->mithIngots;
						}
						if (ingotforstaffeffect == 7)
						{
							ib->vizIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->vizIngots;
						}
						if (ingotforstaffeffect == 8)
						{
							ib->elatIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->elatIngots;
						}
						if (ingotforstaffeffect == 9)
						{
							ib->chitinIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->chitinIngots;
						}
						if (ingotforstaffeffect == 10)
						{
							ib->maligIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->maligIngots;
						}
						if (ingotforstaffeffect == 11)
						{
							ib->tungstenIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->tungstenIngots;
						}
						if (ingotforstaffeffect == 12)
						{
							ib->titaniumIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->titaniumIngots;
						}
						if (ingotforstaffeffect == 13)
						{
							ib->azraelIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->azraelIngots;
						}
						if (ingotforstaffeffect == 14)
						{
							ib->chromeIngots = scytheingotshack + ((InvBlade *)oldBlade->extra)->chromeIngots;
						}

					}
					int cappedNumOfHits = ((InvBlade *)oldBlade->extra)->numOfHits;
					if (cappedNumOfHits > 22000)
						cappedNumOfHits = 22000;
                    // Okay, here's the REAL reason to make a specialized weapon
					if (strcmp(combineSkillName, "Scythe Expertise")) // if we aren't making a scythe
						ib->damageDone = ib->damageDone * (1.0f +
                        (cappedNumOfHits / 22000.0 * 1.0f));

                    // weapon specializiations...
                    if (!strcmp(combineSkillName,"Katana Expertise")     )
                    {
                        ib->type = BLADE_TYPE_KATANA;
                        ib->toHit += 20;
                    }
                    else if (!strcmp(combineSkillName,"Claw Expertise")       )
                    {
                        ib->type = BLADE_TYPE_CLAWS;
                    }
                    else if (!strcmp(combineSkillName,"Chaos Expertise")      )
                    {
                        ib->type = BLADE_TYPE_CHAOS;
                    }
					else if (!strcmp(combineSkillName, "Bladestaff Expertise"))
					{
						ib->type = BLADE_TYPE_DOUBLE;
					}
					else if (!strcmp(combineSkillName, "Scythe Expertise"))
					{
						ib->type = BLADE_TYPE_SCYTHE+prevstafftype; // for uninbued the prevstafftype will be zero, which is fire, which is not allowed, so math works.
						ib->numOfHits = chargecount;                // scythes don't age, isntead age counter is used to store number of charges.
					}
					else if (!strcmp(combineSkillName,"Mace Expertise")       )
                    {
                        ib->type = BLADE_TYPE_MACE;
                        ib->damageDone += 30;
                    }

                    //8)	The value of the sword is the sum of the costs of all the ingots * 1.4
                    io->value = allCost * 1.4f;
                    io->value += oldBlade->value;

                    //9)	The color of the sword blade is the average of the color of all the ingots
                    ib->r = allR / numIngots;
                    ib->g = allG / numIngots;
                    ib->b = allB / numIngots;
					if (!strcmp(combineSkillName, "Scythe Expertise")) // but if it's a scythe
					{													// then instead copy the old blades color
						ib->r = ((InvBlade *)oldBlade->extra)->r;
						ib->g = ((InvBlade *)oldBlade->extra)->g;
						ib->b = ((InvBlade *)oldBlade->extra)->b;

					}
                    ib->poison = dustAmount[INGR_BLACK_DUST] + ((InvBlade *)oldBlade->extra)->poison;
                    ib->heal   = dustAmount[INGR_GREEN_DUST] + ((InvBlade *)oldBlade->extra)->heal;
                    ib->slow   = dustAmount[INGR_RED_DUST]   + ((InvBlade *)oldBlade->extra)->slow;
                    ib->blind  = dustAmount[INGR_WHITE_DUST] + ((InvBlade *)oldBlade->extra)->blind;
					ib->lightning = dustAmount[INGR_SILVER_DUST] + ((InvBlade *)oldBlade->extra)->lightning;
					ib->tame = dustAmount[INGR_GOLD_DUST] + ((InvBlade *)oldBlade->extra)->tame;

                    int mostIngr = -1, ingrCount = 0;
                    if (ib->poison > ingrCount)
                    {
                        ingrCount = ib->poison;
                        mostIngr = INGR_BLACK_DUST;
                    }
                    if (ib->heal > ingrCount)
                    {
                        ingrCount = ib->heal;
                        mostIngr = INGR_GREEN_DUST;
                    }
                    if (ib->slow > ingrCount)
                    {
                        ingrCount = ib->slow;
                        mostIngr = INGR_RED_DUST;
                    }
                    if (ib->blind > ingrCount)
                    {
                        ingrCount = ib->blind;
                        mostIngr = INGR_WHITE_DUST;
                    }
                    if (dustAmount[INGR_BLUE_DUST] + ((InvBlade *)oldBlade->extra)->toHit > ingrCount)
                    {
                        ingrCount = dustAmount[INGR_BLUE_DUST] + ((InvBlade *)oldBlade->extra)->toHit;
                        mostIngr = INGR_BLUE_DUST;
                    }

                    if (mostIngr > -1)
                    {
                        switch(mostIngr)
                        {
                        case INGR_BLUE_DUST:
                        default:
                            ib->bladeGlamourType = BLADE_GLAMOUR_TOHIT1;
                            break;
                        case INGR_GREEN_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_HEAL1;
                            break;
                        case INGR_BLACK_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_POISON1;
                            break;
                        case INGR_WHITE_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_BLIND1;
                            break;
                        case INGR_RED_DUST:
                            ib->bladeGlamourType = BLADE_GLAMOUR_SLOW1;
                            break;
                        }

                        if (ingrCount > 1)
                            ++(ib->bladeGlamourType);
                        if (ingrCount > 4)
                            ++(ib->bladeGlamourType);
                        if (ingrCount > 9)
                            ++(ib->bladeGlamourType);
                    }

                    int shardIndex = -1;
                    for (int i = INGR_WHITE_SHARD; i <= INGR_PINK_SHARD; ++i)
                    {
                        if (dustAmount[i] > 0)
                        {
                            shardIndex = i-INGR_WHITE_SHARD;
                            i = INGR_MAX;
                        }
                    }

                    if (shardIndex > -1)
                    {
                        ib->bladeGlamourType = BLADE_GLAMOUR_TRAILWHITE + shardIndex;
                        ib->damageDone += ib->damageDone/10;
                    }
					// for scythe, copy the existign blade glamour on if there was a shard
					if (!strcmp(combineSkillName, "Scythe Expertise")) // if it is a scythe
						if (((InvBlade *)oldBlade->extra)->bladeGlamourType>= BLADE_GLAMOUR_TRAILWHITE) // if it had a trail
							ib->bladeGlamourType = ((InvBlade *)oldBlade->extra)->bladeGlamourType;               // keep it
                    sprintf(tempText,"You have created a");
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    sprintf(tempText,"%s!",io->WhoAmI());
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    QuestCraftWeapon(ss, io, challenge, work, combineSkillName);

                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);

                    // destroy blade from workbench
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {
                        if (INVOBJ_BLADE == io->type && oldBlade == io)
                        {
                            if( io->amount == 1 ) {
                                charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                                delete io;
                            }
                            else {
                                io->amount -= 1;
                            }
                        }
                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                }
                else
                {
                    // D)	ELSE, for each item in the workbench, 
                    //    the chance of being destroyed = 1.0 -(work/challenge) (0.0 to 1.0)
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {

                    //    the chance of being destroyed = 1 - (work/challenge) (0.0 to 1.0)
                        if (
                               work/challenge < rnd(0,1) &&
                             ( INVOBJ_INGOT == io->type || ( 
                               INVOBJ_INGREDIENT == io->type && ((InvIngredient *)io->extra)->type < INGR_WHITE_SHARD )  ||
                             ( INVOBJ_INGREDIENT == io->type && ((InvIngredient *)io->extra)->type >= INGR_WHITE_SHARD &&
                               INVOBJ_INGREDIENT == io->type && ((InvIngredient *)io->extra)->type < INGR_MAX && rnd(0,5) == 2 )
                             )
                             && io->type != INVOBJ_BLADE
                        )
                        {
                            if (io->amount > 1)
                                io->amount--;
                            else
                            {
                                charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                                delete io;
                            }
                        }

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                }

                // In any case, you get skill sub-points for that skill.
                if( skillInfo->skillLevel < 1500 && challenge > work / 50 )
                {
                    skillInfo->skillPoints += work* bboServer->smith_exp_multiplier;

                    if (skillInfo->skillLevel * skillInfo->skillLevel * 3000 <= skillInfo->skillPoints)
                    {
                        // made a skill level!!!
                        skillInfo->skillLevel++;
                        sprintf(tempText,"You gained a skill level!!");
                        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                        
                        charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_SMITH_EXPERT;

                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                }
            }
        }
        return;
    }


    /*
    float r,g,b;

    for (int i = 0; i < 3; ++i)
    {
        if (enemyPen[i])
            DeleteObject(enemyPen[i]);
        if (enemyPen2[i])
            DeleteObject(enemyPen2[i]);

        int gl = gameLevel-(2-i);
        if (gl < 0)
            gl = 0;
        gl *= 111;
        while (gl > 355)
            gl -= 360;

        HSVtoRGB(gl, 1, 1, r,g,b);
        enemyPen[i] = CreatePen(PS_SOLID, 1, RGB(255*r, 255*g, 255*b));

    */


    //         ***********************************
    if (!strcmp(combineSkillName,"Explosives"))
    {
        // determine the total power and color of the bomb

        InventoryObject *io;
        int numParts = 0;
        int numFuses = 0;
        int allR = 0;
        int allG = 0;
        int allB = 0;
        float bombR, bombG, bombB;

        bombR = bombG = bombB = 1;

        float allPower = 0, allCost = 0, challenge = 0;
        float fuseQuality = 0;

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_EXPLOSIVE == io->type)
            {
                InvExplosive * ingre = (InvExplosive *)io->extra;

                allPower  += ingre->power   * io->amount;
                challenge += ingre->quality * io->amount;
                numParts  += io->amount;
            }
            else if (INVOBJ_MEAT == io->type)
            {
                InvMeat * ingre = (InvMeat *)io->extra;

                int gl = ingre->type;
                if (gl < 0)
                    gl = 0;
                gl *= 360/25;
                while (gl > 355)
                    gl -= 360;

                HSVtoRGB(gl, 1, 1, bombR, bombG, bombB);
            }


            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_FUSE == io->type)
            {
                InvFuse * ingre = (InvFuse *)io->extra;
                numFuses += io->amount;
                fuseQuality += ingre->quality * io->amount;
//				challenge -= ingre->quality;
//				charInfoArray[curCharacterIndex].workbench->objects.Last();
			//	charInfoArray[curCharacterIndex].workbench->objects.Next(); // use both ropes and twisted papers in difficulty calculation and fuse length
			}

            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        challenge *= 1.0f + fuseQuality;

        if (numParts <= 0)
        {
            sprintf(tempText,"Use explosive material to make a bomb.");
            CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        InvSkill *skillInfo = NULL;
        io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
        while (io)
        {
            if (!strcmp(combineSkillName,io->WhoAmI()))
            {
                skillInfo = (InvSkill *) io->extra;
            }
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
        }

        if (!skillInfo)
        {
            sprintf(tempText,"BUG: Cannot find skill for level.");
            CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
        else
        {
            float work = skillInfo->skillLevel * 
                          CreativeStat() *
                             rnd(0.7f,1.3f);

            if (tokenMan.TokenIsInHere(MAGIC_SNAKE, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                work *= 1.1f;
            if (tokenMan.TokenIsInHere(MAGIC_FROG, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                work *= 1.1f;
            if (tokenMan.TokenIsInHere(MAGIC_SUN, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                work *= 1.1f;

            if (specLevel[2] > 0)
                work *= (1.0f + 0.05f * specLevel[2]);

            sprintf(tempText,"Task difficulty is %4.2f.", challenge);
            CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            sprintf(tempText,"Your work value was %4.2f.", work);
            CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            if (challenge <= work) // success!
            {
                // destroy everything from workbench
                io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                while (io)
                {
                    if (INVOBJ_EXPLOSIVE == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        delete io;
                    }
                    else if (INVOBJ_FUSE == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        delete io;
                    }
                    else if (INVOBJ_MEAT == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        delete io;
                    }
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                }

                // make the new bomb!

                int tempPower, size;
                size = 0;
                tempPower = allPower/10;
                do 
                {
                    ++size;
                    tempPower /= 10;
                } while (tempPower > 0);

                if (size > 14)
                    size = 14;

                sprintf(tempText,"%s Bomb", bombSizeNames[size]);

                io = new InventoryObject(INVOBJ_BOMB,0,tempText);
				charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                InvBomb *ib = (InvBomb *) io->extra; 

                ib->power = allPower;
                ib->fuseDelay = numFuses * 3;

                ib->stability = work/(challenge*2);
                int getsEXP = TRUE;
                if (ib->stability < 0)
                    ib->stability = 0;
                if (ib->stability > 1)
                {
                    ib->stability = 1;
                }
				if (ib->fuseDelay < 1) // no fuse
					ib->stability = 1; // then stability doesn't matter
                io->value = allPower * 2.0f;

                // color
                ib->r = bombR * 255;
                ib->g = bombG * 255;
                ib->b = bombB * 255;

                sprintf(tempText,"You have created a");
                CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf(tempText,"%s!",io->WhoAmI());
                CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                // If successful, you get skill sub-points for that skill.
//				if (getsEXP)
//					skillInfo->skillPoints += 10 * (1.0f-ib->stability) + 5;
                skillInfo->skillPoints += challenge* bboServer->bombing_exp_multiplier;

                if (skillInfo->skillLevel * skillInfo->skillLevel * 100 <= skillInfo->skillPoints)
                {
                    // made a skill level!!!
                    skillInfo->skillLevel++;
                    sprintf(tempText,"You gained a skill level!!");
                    CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_BOMB;
                }
            }
            else
            {
                // D)	ELSE, for each item in the workbench, 
                //    the chance of being destroyed = 1.0 -(work/challenge) (0.0 to 1.0)
                int puffPower = 0;
                io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                while (io)
                {
                    if (INVOBJ_EXPLOSIVE == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        puffPower += io->amount;
                        delete io;
                    }
                    else if (INVOBJ_FUSE == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        puffPower += io->amount;
                        delete io;
                    }
                    else if (INVOBJ_FUSE == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        delete io;
                    }
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                }

                MessExplosion explo;
                explo.avatarID = socketIndex;
                explo.r = 0;
                explo.g = 0;
                explo.b = 0;
                explo.type = 0;
                explo.flags = 0;
                explo.size = 5 + puffPower/10;
                explo.x = cellX;
                explo.y = cellY;
                ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(explo), &explo, explo.size);

            }

        }
        
        return;
    }

    //         ***********************************
    if (!strcmp(combineSkillName,"Weapon Dismantle"))
    {
        InventoryObject *io;
//		int dustAmount[INGR_MAX];

        InventoryObject *weapon = NULL;
        int numWeapons = 0;

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_BLADE == io->type)
            {
                numWeapons += io->amount;
                weapon = io;
                io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Last();
            }
            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        if (numWeapons < 1)
        {
            sprintf(tempText,"Put a weapon in the workbench to dismantle it.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
        else
        {
            InvBlade *ib = (InvBlade *) weapon->extra;

            // If they are stacked, remove the current one so it doesn't mess up
            if( weapon->amount > 1 ) {
                InventoryObject *tmp = new InventoryObject( INVOBJ_SIMPLE, 0, "UNNAMED" );
                weapon->CopyTo( tmp );
                tmp->amount = weapon->amount - 1;
                weapon->amount = 1;

                charInfoArray[curCharacterIndex].workbench->objects.Prepend( tmp );
            }

            int type1, type2;
            DetectIngotTypes(ib, type1, type2);

            float weapSize = ib->size;

            if (BLADE_TYPE_NORMAL != ib->type)
                    weapSize /= 0.7f;

            float ingotCount = ib->GetIngotCount();
            float originalIngotCount = ib->GetIngotCount();
              
            float damDone = ib->damageDone;

            if (ib->bladeGlamourType >= BLADE_GLAMOUR_TRAILWHITE)
                 damDone = damDone * 10 / 11;

			if (BLADE_TYPE_MACE == ib->type)
				damDone -= 30;

            float challenge = (1+ingotCount) * (2+type1) * (1+ib->poison/5) * 
                               (1+ib->slow/5) * (1+ib->blind/5) * (1+ib->heal/5)* (1+ib->tame); // gold dust vastly increases complexity now.

			if (BLADE_TYPE_SCYTHE == ib->type)
				float challenge = (1 + ingotCount) * (2 + type1) * (1 + ib->poison / 5) *
				(1 + ib->slow / 5) * (1 + ib->blind / 5) * (1 + ib->heal / 5); // but not on a scythe.


            //5)	IF the task is successful,
            // B)	work = skill level * creativity * rnd(0.8,1.2)
            // C)	if Challenge <= work, success!

            InvSkill *skillInfo = NULL;
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
            while (io)
            {
                if (!strcmp(combineSkillName,io->WhoAmI()))
                {
                    skillInfo = (InvSkill *) io->extra;
                }
                io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
            }

            if (!skillInfo)
            {
                sprintf(tempText,"BUG: Cannot find skill for level.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else
            {
                float work = skillInfo->skillLevel * 
                              CreativeStat() *
                                 rnd(0.9f,1.1f) * 3;

//				float tempChall = challenge;

                if (tokenMan.TokenIsInHere(MAGIC_WOLF, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }
                if (tokenMan.TokenIsInHere(MAGIC_BEAR, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }
                if (tokenMan.TokenIsInHere(MAGIC_EAGLE, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                {
                    work *= 1.1f;
                }

                if (specLevel[2] > 0)
                    work *= (1.0f + 0.05f * specLevel[2]);

                sprintf(tempText,"This weapon's complexity is %4.2f.", challenge);
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf(tempText,"Your work value was %4.2f.", work);
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

				if ((ib->type > BLADE_TYPE_DOUBLE)) // it's a scythe
				{  // drop ingot counts to 64
					ib->chromeIngots = Bracket(ib->chromeIngots, 0, 64);
					ib->azraelIngots = Bracket(ib->azraelIngots, 0, 64);
					ib->titaniumIngots = Bracket(ib->titaniumIngots, 0, 64);
					ib->tungstenIngots = Bracket(ib->tungstenIngots, 0, 64);
					ib->maligIngots = Bracket(ib->maligIngots, 0, 64);
					ib->chitinIngots = Bracket(ib->chitinIngots, 0, 64);
					ib->elatIngots = Bracket(ib->elatIngots, 0, 64);
					ib->vizIngots = Bracket(ib->vizIngots, 0, 64);
					ib->mithIngots = Bracket(ib->mithIngots, 0, 64);
					ib->adamIngots = Bracket(ib->adamIngots, 0, 64);
					ib->zincIngots = Bracket(ib->zincIngots, 0, 64);
					ib->carbonIngots = Bracket(ib->carbonIngots, 0, 64);
					ib->steelIngots = Bracket(ib->steelIngots, 0, 64);
					ib->aluminumIngots = Bracket(ib->aluminumIngots, 0, 64);
					ib->tinIngots = Bracket(ib->tinIngots, 0, 64);
				}
				if ((ib->type > BLADE_TYPE_DOUBLE) && ((work / 15 / challenge) >= 1)) // it's a scythe and our wv was good enough to get a shard omg
				{   // we can get out GS back.
					// last check will catch my first scythe.
					// copy the blade. 
					int numIngots = 64; // was a greatsword before, which requires 64 ingots
					int mostingots = 1; // 0 ingots don't count for renaming
					int mostingots2 = -1;
					int candidate = -1;
					int candidate2 = -1;
					// find most common.
					if (ib->tinIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 0;
						mostingots = ib->tinIngots;
					}
					if (ib->aluminumIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 1;
						mostingots = ib->aluminumIngots;
					}
					if (ib->steelIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 2;
						mostingots = ib->steelIngots;
					}
					if (ib->carbonIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 3;
						mostingots = ib->carbonIngots;
					}
					if (ib->zincIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 4;
						mostingots = ib->zincIngots;
					}
					if (ib->adamIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 5;
						mostingots = ib->adamIngots;
					}
					if (ib->mithIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 6;
						mostingots = ib->mithIngots;
					}
					if (ib->vizIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 7;
						mostingots = ib->vizIngots;
					}
					if (ib->elatIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 8;
						mostingots = ib->elatIngots;
					}
					if (ib->chitinIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 9;
						mostingots = ib->chitinIngots;
					}
					if (ib->maligIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 10;
						mostingots = ib->maligIngots;
					}
					if (ib->tungstenIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 11;
						mostingots = ib->tungstenIngots;
					}
					if (ib->titaniumIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 12;
						mostingots = ib->titaniumIngots;
					}
					if (ib->azraelIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 13;
						mostingots = ib->azraelIngots;
					}
					if (ib->chromeIngots >= mostingots)
					{
						mostingots2 = mostingots;
						candidate2 = candidate;
						candidate = 14;
						mostingots = ib->chromeIngots;
					}
					// then backtrack to find second most, if it's after most.
					if ((ib->chromeIngots > mostingots2) && (candidate != 14)) // if it's not already the most
					{
						candidate2 = 14;
						mostingots2 = ib->chromeIngots;
					}
					if ((ib->azraelIngots > mostingots2) && (candidate != 13)) // if it's not already the most
					{
						candidate2 = 13;
						mostingots2 = ib->azraelIngots;
					}
					if ((ib->titaniumIngots > mostingots2) && (candidate != 12)) // if it's not already the most
					{
						candidate2 = 12;
						mostingots2 = ib->titaniumIngots;
					}
					if ((ib->tungstenIngots > mostingots2) && (candidate != 11)) // if it's not already the most
					{
						candidate2 = 11;
						mostingots2 = ib->tungstenIngots;
					}
					if ((ib->maligIngots > mostingots2) && (candidate != 10)) // if it's not already the most
					{
						candidate2 = 10;
						mostingots2 = ib->maligIngots;
					}
					if ((ib->chitinIngots > mostingots2) && (candidate != 9)) // if it's not already the most
					{
						candidate2 = 9;
						mostingots2 = ib->chitinIngots;
					}
					if ((ib->elatIngots > mostingots2) && (candidate != 8)) // if it's not already the most
					{
						candidate2 = 8;
						mostingots2 = ib->elatIngots;
					}
					if ((ib->vizIngots > mostingots2) && (candidate != 7)) // if it's not already the most
					{
						candidate2 = 7;
						mostingots2 = ib->vizIngots;
					}
					if ((ib->mithIngots > mostingots2) && (candidate != 6)) // if it's not already the most
					{
						candidate2 = 6;
						mostingots2 = ib->mithIngots;
					}
					if ((ib->adamIngots > mostingots2) && (candidate != 5)) // if it's not already the most
					{
						candidate2 = 5;
						mostingots2 = ib->adamIngots;
					}
					if ((ib->zincIngots > mostingots2) && (candidate != 4)) // if it's not already the most
					{
						candidate2 = 4;
						mostingots2 = ib->zincIngots;
					}
					if ((ib->carbonIngots > mostingots2) && (candidate != 3)) // if it's not already the most
					{
						candidate2 = 3;
						mostingots2 = ib->carbonIngots;
					}
					if ((ib->steelIngots > mostingots2) && (candidate != 2)) // if it's not already the most
					{
						candidate2 = 2;
						mostingots2 = ib->steelIngots;
					}
					if ((ib->aluminumIngots > mostingots2) && (candidate != 1)) // if it's not already the most
					{
						candidate2 = 1;
						mostingots2 = ib->aluminumIngots;
					}
					if ((ib->tinIngots > mostingots2) && (candidate != 0)) // if it's not already the most
					{
						candidate2 = 0;
						mostingots2 = ib->tinIngots;
					}
					if (candidate < 0) // this must be My First Scythe
					{
						// then fix it
						candidate = 0;
						ib->tame = 0; // bye bye dusts.
						ib->toHit = 1;
					}
					// now we can recreate the name
					if (candidate2 > -1)
					{
						sscanf(ingotNameList[candidate], "%s", &(tempText[0]));
						sprintf(&(tempText[strlen(tempText)]), "-");
						sscanf(ingotNameList[candidate2], "%s", &(tempText[strlen(tempText)]));
					}
					else
					{
						sscanf(ingotNameList[candidate], "%s", &(tempText[0]));
					}
					sprintf(&(tempText[strlen(tempText)]), " ");

					int candidate3 = 6, most3 = 0;
					for (int j = 0; j < 6; ++j)
					{
						if (bladeList[j].size > numIngots)
						{
							candidate3 = j - 1;
							j = 7;
						}
					}

					if (candidate3 < 0)
						candidate3 = 0;
					if (candidate3 > 5)
						candidate3 = 5;

					sprintf(&(tempText[strlen(tempText)]), bladeList[candidate3].name);

					// make the old blade! 
					InventoryObject *io2 = new InventoryObject(INVOBJ_BLADE, 0, tempText);
					InvBlade *ib2 = (InvBlade *)io2->extra;

					//7)	The damage of the sword is the damage value of the scythe
					ib2->damageDone = ib->damageDone;

					//8)	The value of the sword is the sme one the scythe had
					io2->value = weapon->value;

					//9)	The color of the sword blade is the same as the scythe had.
					ib2->r = ib->r;
					ib2->g = ib->g;
					ib2->b = ib->b;
					// and same tohit 
					ib2->toHit = ib->toHit;
					// size 5
					ib2->size = 5.0f; // it was a greatsword before.
					// and same dusts
					ib2->poison = ib->poison;
					ib2->heal = ib->heal;
					ib2->slow = ib->slow;
					ib2->blind = ib->blind;
					ib2->lightning = ib->lightning;
					ib2->tame = ib->tame;

					// and same ingots
					ib2->tinIngots = ib->tinIngots;
					ib2->aluminumIngots = ib->aluminumIngots;
					ib2->steelIngots = ib->steelIngots;
					ib2->carbonIngots = ib->carbonIngots;
					ib2->zincIngots = ib->zincIngots;
					ib2->adamIngots = ib->adamIngots;
					ib2->mithIngots = ib->mithIngots;
					ib2->vizIngots = ib->vizIngots;
					ib2->elatIngots = ib->elatIngots;
					ib2->chitinIngots = ib->chitinIngots;
					ib2->maligIngots = ib->maligIngots;
					ib2->tungstenIngots = ib->tungstenIngots;
					ib2->titaniumIngots = ib->titaniumIngots;
					ib2->azraelIngots = ib->azraelIngots;
					ib2->chromeIngots = ib->chromeIngots;

					// and same glamour
					ib2->bladeGlamourType = ib->bladeGlamourType;

					// and add it to your inventory
					charInfoArray[curCharacterIndex].inventory->AddItemSorted(io2);

					//finally, print the message 
					sprintf(tempText, "You have recovered the greatsword.");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}
				else
				{   // nomral dismantle, even if it is a scythe. the staff is lost eiter way.
					// generate some ingots
					int ingotsReturned = work / 5 / challenge * ingotCount;
					ingotsReturned = Bracket(ingotsReturned,
						(int)(ingotCount * 0.5f), (int)(ingotCount * 0.9f));

					int ingotsReturnedArray[25];
					for (int i = 0; i < 25; ++i)
						ingotsReturnedArray[i] = 0;

					ib->chromeIngots = ib->chromeIngots * 0.8;
					ib->azraelIngots = ib->azraelIngots * 0.8;
					ib->titaniumIngots = ib->titaniumIngots * 0.8;
					ib->tungstenIngots = ib->tungstenIngots * 0.8;
					ib->maligIngots = ib->maligIngots * 0.8;
					ib->chitinIngots = ib->chitinIngots * 0.8;
					ib->elatIngots = ib->elatIngots * 0.8;
					ib->vizIngots = ib->vizIngots * 0.8;
					ib->mithIngots = ib->mithIngots * 0.8;
					ib->adamIngots = ib->adamIngots * 0.8;
					ib->zincIngots = ib->zincIngots * 0.8;
					ib->carbonIngots = ib->carbonIngots * 0.8;
					ib->steelIngots = ib->steelIngots * 0.8;
					ib->aluminumIngots = ib->aluminumIngots * 0.8;
					ib->tinIngots = ib->tinIngots * 0.8;

					for (int i = 0; i < ingotsReturned; ++i)
					{
						int ingotVal;

						if (ib->chromeIngots)
						{
							ingotVal = 15;
							--(ib->chromeIngots);
						}
						else if (ib->azraelIngots)
						{
							ingotVal = 14;
							--(ib->azraelIngots);
						}
						else if (ib->titaniumIngots)
						{
							ingotVal = 13;
							--(ib->titaniumIngots);
						}
						else if (ib->tungstenIngots)
						{
							ingotVal = 12;
							--(ib->tungstenIngots);
						}
						else if (ib->maligIngots)
						{
							ingotVal = 11;
							--(ib->maligIngots);
						}
						else if (ib->chitinIngots)
						{
							ingotVal = 10;
							--(ib->chitinIngots);
						}
						else if (ib->elatIngots)
						{
							ingotVal = 9;
							--(ib->elatIngots);
						}
						else if (ib->vizIngots)
						{
							ingotVal = 8;
							--(ib->vizIngots);
						}
						else if (ib->mithIngots)
						{
							ingotVal = 7;
							--(ib->mithIngots);
						}
						else if (ib->adamIngots)
						{
							ingotVal = 6;
							--(ib->adamIngots);
						}
						else if (ib->zincIngots)
						{
							ingotVal = 5;
							--(ib->zincIngots);
						}
						else if (ib->carbonIngots)
						{
							ingotVal = 4;
							--(ib->carbonIngots);
						}
						else if (ib->steelIngots)
						{
							ingotVal = 3;
							--(ib->steelIngots);
						}
						else if (ib->aluminumIngots)
						{
							ingotVal = 2;
							--(ib->aluminumIngots);
						}
						else if (ib->tinIngots)
						{
							ingotVal = 1;
							--(ib->tinIngots);
						}
						else
							break;

						sprintf(tempText, "%s Ingot", ingotNameList[ingotVal - 1]);
						io = new InventoryObject(INVOBJ_INGOT, 0, tempText);
						InvIngot *exIngot = (InvIngot *)io->extra;
						exIngot->damageVal = ingotVal;
						exIngot->challenge = ingotVal;
						exIngot->r = ingotRGBList[ingotVal - 1][0];
						exIngot->g = ingotRGBList[ingotVal - 1][1];
						exIngot->b = ingotRGBList[ingotVal - 1][2];

						io->mass = 1.0f;
						io->value = ingotValueList[ingotVal - 1];
						io->amount = 1;

						++ingotsReturnedArray[ingotVal - 1];

						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
					}

					for (int i = 0; i < 25; ++i)
					{
						if (ingotsReturnedArray[i] > 0)
						{
							sprintf(tempText, "You recover %d %s Ingots.",
								ingotsReturnedArray[i], ingotNameList[i]);
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
					// generate some dust
					int gotDust = FALSE;
					int dustTotal = 0;
					float dustReturned = Bracket((int)(work / 10 / challenge * ib->blind),
						(int)(ib->blind*0.3f), (int)(ib->blind*0.9f));
					dustTotal += ib->blind;
					if (dustReturned >= 1)
					{
						io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing White Dust");
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = INGR_WHITE_DUST;
						exIn->quality = 1;
						io->mass = 0.0f;
						io->value = 1000;
						io->amount = dustReturned;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
						gotDust = TRUE;

						sprintf(tempText, "You recover %d Glowing White Dust.", (int)dustReturned);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}

					dustReturned = Bracket((int)(work / 10 / challenge * ib->slow),
						(int)(ib->slow*0.3f), (int)(ib->slow*0.9f));
					dustTotal += ib->slow;
					if (dustReturned >= 1)
					{
						io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Red Dust");
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = INGR_RED_DUST;
						exIn->quality = 1;
						io->mass = 0.0f;
						io->value = 1000;
						io->amount = dustReturned;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
						gotDust = TRUE;

						sprintf(tempText, "You recover %d Glowing Red Dust.", (int)dustReturned);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}

					dustReturned = Bracket((int)(work / 10 / challenge * ib->heal),
						(int)(ib->heal*0.3f), (int)(ib->heal*0.9f));
					dustTotal += ib->heal;
					if (dustReturned >= 1)
					{
						io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Green Dust");
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = INGR_GREEN_DUST;
						exIn->quality = 1;
						io->mass = 0.0f;
						io->value = 1000;
						io->amount = dustReturned;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
						gotDust = TRUE;

						sprintf(tempText, "You recover %d Glowing Green Dust.", (int)dustReturned);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}

					dustReturned = Bracket((int)(work / 10 / challenge * ib->poison),
						(int)(ib->poison*0.3f), (int)(ib->poison*0.9f));
					dustTotal += ib->poison;
					if (dustReturned >= 1)
					{
						io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Black Dust");
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = INGR_BLACK_DUST;
						exIn->quality = 1;
						io->mass = 0.0f;
						io->value = 1000;
						io->amount = dustReturned;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
						gotDust = TRUE;

						sprintf(tempText, "You recover %d Glowing Black Dust.", (int)dustReturned);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}

					dustReturned = Bracket((int)(work / 10 / challenge * ib->tame),
						(int)(ib->tame*0.3f), (int)(ib->tame));  // can get 100% of gold back, and ONLY gold.
					dustTotal += ib->tame;
					if (dustReturned >= 1)
					{
						io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Gold Dust");
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = INGR_GOLD_DUST;
						exIn->quality = 1;
						io->mass = 0.0f;
						io->value = 1000;
						io->amount = dustReturned;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
						gotDust = TRUE;

						sprintf(tempText, "You recover %d Glowing Gold Dust.", (int)dustReturned);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}

					if (ib->type == BLADE_TYPE_KATANA)
						dustReturned = Bracket((int)(work / 10 / challenge * (ib->toHit - 21)),
						(int)((ib->toHit - 21)*0.2f), (int)((ib->toHit - 21)*0.7f));
					else  // don't penalize total dust count for not using a tachi.
						dustReturned = Bracket((int)(work / 10 / challenge * (ib->toHit - 1)),
						(int)((ib->toHit - 1)*0.2f), (int)((ib->toHit - 1)*0.7f));
					dustTotal += (int)dustReturned;
					if (dustReturned >= 1)
					{
						io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Blue Dust");
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = INGR_BLUE_DUST;
						exIn->quality = 1;
						io->mass = 0.0f;
						io->value = 1000;
						io->amount = dustReturned;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
						gotDust = TRUE;

						sprintf(tempText, "You recover %d Glowing Blue Dust.", (int)dustReturned);
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					dustTotal += ib->lightning; // also count silver dust even though you can never get it back.


					// generate something special

					dustReturned = Bracket((int)(work / 15 / challenge), 0, 1); // work value must be at least 15 * complexity for chance at shard. 
					int chance = 0;
					int chance2 = 0;
					if (ib->numOfHits >= 22000)
						chance = ((ib->numOfHits - 22000) / 2500) + 5;
					//				if (ib->numOfHits >= 50000)
					//					chance = 15;
					if (dustTotal < 5)
						chance = 0;
					if (dustTotal > 14 && chance)
						chance += 10;
					chance2 = (rand() % 100 < chance);
					if (dustReturned > 0 && ib->numOfHits >= 22000 && gotDust && chance2  && damDone >= 72)
					{
						int type = INGR_WHITE_SHARD + (rand() % 8);
						io = new InventoryObject(
							INVOBJ_INGREDIENT, 0, dustNames[type]);
						InvIngredient *exIn = (InvIngredient *)io->extra;
						exIn->type = type;
						exIn->quality = 1;

						io->mass = 0.0f;
						io->value = 1000;
						io->amount = 1;
						charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);

						sprintf(tempText, "You find something special inside the weapon.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);


						sprintf(tempText, "%s got a shard\n", charInfoArray[curCharacterIndex].name);
						LogOutput("ShardCreationLog.txt", tempText);


					}
					else if (bboServer->enable_cheating!=0) // in cheaters mode, also give old formula a chance
					{
						chance = (rand() % 10);				// flat 10% chance if other requirements are met

						if (dustReturned > 0 && ib->numOfHits >= 22000 && gotDust && chance == 1 && originalIngotCount >= 96) //cheezable with tin tachi made from unages GS and few dusts.
						{
							int type = INGR_WHITE_SHARD + (rand() % 8);
							io = new InventoryObject(
								INVOBJ_INGREDIENT, 0, dustNames[type]);
							InvIngredient *exIn = (InvIngredient *)io->extra;
							exIn->type = type;
							exIn->quality = 1;

							io->mass = 0.0f;
							io->value = 1000;
							io->amount = 1;
							charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);

							sprintf(tempText, "You find something special inside the weapon.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);


							sprintf(tempText, "%s got a shard\n", charInfoArray[curCharacterIndex].name);
							LogOutput("ShardCreationLog.txt", tempText);


						}
					}
				}
                // destroy weapons from workbench
                io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                while (io)
                {
                    if (weapon == io)
                    {
                        if (io->amount > 1)
                            --io->amount;
                        else
                        {
                            charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                            delete io;
                        }
                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Last();
                    }
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                }


                sprintf(tempText,"You dismantle the weapon.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);


                // In any case, you get skill sub-points for that skill.
                if( skillInfo->skillLevel < 30000 && challenge > work / 10 ) 
                {
                    skillInfo->skillPoints += work* bboServer->smith_exp_multiplier;

                    if( skillInfo->skillLevel * skillInfo->skillLevel * 30 <= skillInfo->skillPoints )
                    {
                        // made a skill level!!!
						// if (skillInfo->skillLevel * skillInfo->skillLevel * 30) overflows, then you will get free elvelups until skillPoints also overflows
						// but then you have to get all the experience to make up for all the levels you skipped.
						// this cannot be easily abused because of the minimum diffiuclty requirements to gain exp.
                        skillInfo->skillLevel++;
                        sprintf(tempText,"You gained a skill level!!");
                        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                        
                        charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_SMITH;

                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    }
                }
            }
        }
        return;
    }
    //         ***********************************
	if (!strcmp(combineSkillName, "Disarming"))
	{
		InventoryObject *io;
		//		int dustAmount[INGR_MAX];

		InventoryObject *bomb = NULL;
		int numBombs = 0;

		io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
		while (io)
		{
			if (INVOBJ_BOMB == io->type)
			{
				numBombs += io->amount;
				bomb = io;
				io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Last();
			}
			io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
		}

		if (numBombs < 1)
		{
			sprintf(tempText, "Put a bomb in the workbench to attempt to disarm it.");
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
		}
		else
		{
			InvBomb *ib = (InvBomb *)bomb->extra;

			// If they are stacked, remove the current one so it doesn't mess up
			if (bomb->amount > 1) {
				InventoryObject *tmp = new InventoryObject(INVOBJ_SIMPLE, 0, "UNNAMED");
				bomb->CopyTo(tmp);
				tmp->amount = bomb->amount - 1;
				bomb->amount = 1;

				charInfoArray[curCharacterIndex].workbench->objects.Prepend(tmp);
			}


			float damDone = ib->power;
			float challenge = damDone*(1+ib->fuseDelay)*(1 / ib->stability);


//5)	IF the task is successful,
// B)	work = skill level * creativity * rnd(0.9,1.1)*3
// C)	if Challenge <= work, success!

			InvSkill *skillInfo = NULL;
			io = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.First();
			while (io)
			{
				if (!strcmp(combineSkillName, io->WhoAmI()))
				{
					skillInfo = (InvSkill *)io->extra;
				}
				io = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.Next();
			}

			if (!skillInfo)
			{
				sprintf(tempText, "BUG: Cannot find skill for level.");
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			}
			else
			{
				float work = skillInfo->skillLevel *
					CreativeStat() *
					rnd(0.9f, 1.1f) * 3;

				//				float tempChall = challenge;

				if (tokenMan.TokenIsInHere(MAGIC_WOLF, ss) && ((TowerMap*)ss)->IsMember(charInfoArray[curCharacterIndex].name))
				{
					work *= 1.1f;
				}
				if (tokenMan.TokenIsInHere(MAGIC_BEAR, ss) && ((TowerMap*)ss)->IsMember(charInfoArray[curCharacterIndex].name))
				{
					work *= 1.1f;
				}
				if (tokenMan.TokenIsInHere(MAGIC_EAGLE, ss) && ((TowerMap*)ss)->IsMember(charInfoArray[curCharacterIndex].name))
				{
					work *= 1.1f;
				}

				if (specLevel[2] > 0)
					work *= (1.0f + 0.05f * specLevel[2]);

				sprintf(tempText, "This bomb's difficulty is %4.2f.", challenge);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				sprintf(tempText, "Your work value was %4.2f.", work);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

				// destroy the bomb from workbench if you suceeded
				if (work >= challenge) // if we suceeded.
				{
					float lvalue = ib->fuseDelay * 10 + bomb->value * 2; // set value forlater so we can create a Disarmed Bomb simple loot
					io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
					while (io)
					{
						if (bomb == io)  // we found the bomb
						{
							if (io->amount > 1)
								--io->amount;
							else
							{
								charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Last();
						}
						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
					}
					// and creat the disarmed bomb
					InventoryObject *iObject =
						new InventoryObject(INVOBJ_SIMPLE, 0, "Disarmed Bomb");
					iObject->mass = 1.0f;
					iObject->value = lvalue;
					iObject->amount = 1;
					charInfoArray[curCharacterIndex].inventory->
						AddItemSorted(iObject);
					sprintf(tempText, "You disarmed the bomb.");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				}



				// If you suceeded, you get skill points equal to task difficuly.
				if (skillInfo->skillLevel < 30000 && work >= challenge) 
				{
					skillInfo->skillPoints += challenge* bboServer->bombing_exp_multiplier;

					if (skillInfo->skillLevel * skillInfo->skillLevel * 30 <= skillInfo->skillPoints)
					{
						// made a skill level!!!
						// if (skillInfo->skillLevel * skillInfo->skillLevel * 30) overflows, then you will get free elvelups until skillPoints also overflows
						// but then you have to get all the experience to make up for all the levels you skipped.
						// this cannot be easily abused because of the minimum diffiuclty requirements to gain exp.
						skillInfo->skillLevel++;
						sprintf(tempText, "You gained a skill level!!");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

						// charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_BOMB; // no clvl cuz smiths have enough. lol

						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
				}
				else
				{
					// bomb goes boom! FIXME
					sprintf(tempText, "You failed to disarm the bomb!");
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					BBOSBomb *tbomb = new BBOSBomb(this);
					tbomb->cellX = cellX;
					tbomb->cellY = cellY;
					tbomb->type = ib->type;
					tbomb->flags = ib->flags;
					tbomb->r = ib->r;
					tbomb->g = ib->g;
					tbomb->b = ib->b;
					tbomb->power = ib->power;
					tbomb->detonateTime = timeGetTime(); // we ignore stability
					// and delete the bomb object
					io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
					while (io)
					{
						if (bomb == io)  // we found the bomb
						{
							if (io->amount > 1)
								--io->amount;
							else
							{
								charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Last();
						}
						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
					}

					ss->mobList->Add(tbomb);
				}
			}
		}
		return;
	}

    //         ***********************************
    if (!strcmp(combineSkillName,"Geomancy"))
    {
        InventoryObject *io;
        int meatType[2];
        float gemPower, beadPower;
        int beadCount;
		int rottedmeat = 0;
        beadCount = 0;
        meatType[0] = meatType[1] = -1;
        gemPower = beadPower = 1;

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_MEAT == io->type)
            {
                InvMeat * ingre = (InvMeat *)io->extra;
                if (-1 != meatType[0])
                    meatType[1] = ingre->type;
                else
                    meatType[0] = ingre->type;
				if ((ingre->age > 23) || (ingre->age==-2))
					rottedmeat = 1;
            }
            else if (INVOBJ_GEOPART == io->type)
            {
                InvGeoPart * ingre = (InvGeoPart *)io->extra;
                if (0 == ingre->type) // bead
                {
                    beadCount += io->amount;
                    beadPower += ingre->power * io->amount;
                }
                else if (1 == ingre->type) // Heart Gem
                {
                    gemPower += ingre->power * io->amount;
                }
            }

            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        if (-1 == meatType[0])
        {
            sprintf(tempText,"An EarthKey requires the flesh of a monster.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
        else if (beadCount < 1)
        {
            sprintf(tempText,"An EarthKey requires at least one bead to power it.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
        else
        {
            InvSkill *skillInfo = NULL;
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
            while (io)
            {
                if (!strcmp(combineSkillName,io->WhoAmI()))
                {
                    skillInfo = (InvSkill *) io->extra;
                }
                io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
            }

            if (!skillInfo)
            {
                sprintf(tempText,"BUG: Cannot find skill for level.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else
            {
                float work = skillInfo->skillLevel * 
                    MagicalStat() / 4.0f * rnd(0.7f,1.3f);

                if (tokenMan.TokenIsInHere(MAGIC_MOON, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                    work *= 1.2f;
                if (tokenMan.TokenIsInHere(MAGIC_TURTLE, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                    work *= 1.3f;

                if (specLevel[1] > 0)
                    work *= (1.0f + 0.05f * specLevel[1]);

                float challenge = beadCount * beadCount * gemPower * 1.5f;

                sprintf(tempText,"Task difficulty is %4.2f.", challenge);
                CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf(tempText,"Your work value was %4.2f.", work);
                CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                if (challenge <= work) // success!
                {
                    // destroy everything from workbench
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {
                        if (INVOBJ_MEAT == io->type || INVOBJ_GEOPART == io->type)
                        {
                            charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                            delete io;
                        }
                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                    // make the new item, an EarthKey.

                    int ekName1 = Bracket((int)(beadPower * gemPower) % 4, 0, 4);
                    int ekName2 = 5 + Bracket((int)(beadPower * gemPower/10) % 4, 0, 4);
					// anything goes in chaeters edition
					if (bboServer->enable_cheating == 0) // if we are not cheating
					{
						// exclude a few certain types of monster
						if (21 == meatType[0] ||  // no Thieving Spirit Geos, that's messed up
							27 == meatType[0] ||   // no Vampire Lord Geos. 
							31 == meatType[0] ||   // no Unicorns, yet. they are event monsters
							32 == meatType[0]    // no Reindeer geos, only for christmas event
							)
							meatType[0] = 0;
						// spirit dragons and minos are safe, because the ingots will not drop.
						// lizard is safe cuz dust doesn't drop.
						// wurms don't drop their dusts either, so they are allowed
						if (21 == meatType[1] ||   // no Thieving Spirit Geos, that's messed up
							27 == meatType[1] ||   // no Vampire Lord Geos. 
							31 == meatType[1] ||   // no Unicorns, yet. they are event monsters
							32 == meatType[1]      // no Reindeer geos, only for christmas event
							)
							meatType[1] = 0;
					}
					int ekdepth = beadPower * gemPower + 20;
					int ekwidth = sqrt((beadPower * gemPower)) / 5;
					ekwidth = ekwidth * 5 + 10;
					int ekheight = sqrt((beadPower * gemPower*0.8f)) / 5;
					ekheight = ekheight * 5 + 10;
					// original naming code
					//					sprintf(tempText, "%s %s EarthKey",
					//						earthKeyArray[ekName1], earthKeyArray[ekName2]);
					// depth,monstername, then word earthkey (should BARELY fit)
					if (meatType[1]<0)
						sprintf(tempText, "%dm %s EarthKey",
							ekdepth, monsterData[meatType[0]][0].name);
					else
						sprintf(tempText, "%dm mixed EarthKey",
					ekdepth);
					// widthXheight monsternameKey
			//		if (meatType[1]<0)
			//			sprintf(tempText, "%dX%d %sKey",
			//				ekwidth,ekheight, monsterData[meatType[0]][0].name);
			//		else
			//			sprintf(tempText, "%dX%d mixed EarthKey",
			//				ekwidth, ekheight);

					io = new InventoryObject(INVOBJ_EARTHKEY,0,tempText);
                    io->mass = 0.0f;
                    io->value = 200 + beadPower * gemPower * 3;
                    io->amount = 1;

                    InvEarthKey *exIn = (InvEarthKey *)io->extra;
                    exIn->power = beadPower * gemPower;

                    exIn->monsterType[0] = meatType[0];
                    exIn->monsterType[1] = meatType[1];
                    exIn->width = sqrt(exIn->power)/5;
                    exIn->width = exIn->width * 5 + 10;
                    if (exIn->width < 10)
                        exIn->width = 10;

                    exIn->height = sqrt((exIn->power*0.8f))/5;
                    exIn->height = exIn->height * 5 + 10;
                    if (exIn->height < 10)
                        exIn->height = 10;
					if (rottedmeat == 1)
					{
						exIn->width = exIn->width*-1;
						exIn->height = exIn->height*-1;
					}
                    sprintf(tempText,"You have created a");
                    CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    sprintf(tempText,"%s!",io->WhoAmI());
                    CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);

                    // IF successful, you get skill sub-points for that skill.
                    if ( skillInfo->skillLevel < 30000 && challenge > work / 20 )
                    {
                        skillInfo->skillPoints += work* bboServer->geomancy_exp_multiplier;

                        if (skillInfo->skillLevel * skillInfo->skillLevel * 30 <= skillInfo->skillPoints)
                        {
                            // made a skill level!!!
                            skillInfo->skillLevel++;
                            sprintf(tempText,"You gained a skill level!!");
                            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                            
                            charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_GEOMANCY;

                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        }
                    }

                }
                else
                {
                    sprintf(tempText,"Your Eye of the Earth fails to see.");
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    // D)	ELSE, for each item in the workbench, 
                    //    the chance of being destroyed = 1.0 -(work/challenge) (0.0 to 1.0)
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                    while (io)
                    {

                    //    the chance of being destroyed = 1 - (work/challenge) (0.0 to 1.0)
                        if (work/challenge < rnd(0,1) &&
                             (INVOBJ_MEAT == io->type || INVOBJ_GEOPART == io->type))
                        {
                            if (io->amount > 1)
                                io->amount--;
                            else
                            {
                                charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                                delete io;
                            }
                        }

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                    }

                }

            }

        }
        
        return;
    }



    // *******************************************
    // TOTEM SHATTER
    // *******************************************
    if (!strcmp(combineSkillName,"Totem Shatter"))
    {
        InventoryObject *io;
        InventoryObject *tot = NULL;
        

        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_TOTEM == io->type)
            {
                tot = io;
            }

            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        if ( tot == NULL )
        {
            sprintf(tempText,"You must have a totem in your workbench to shatter.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }
        else
        {
            InvSkill *skillInfo = NULL;
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
            while (io)
            {
                if (!strcmp(combineSkillName,io->WhoAmI()))
                {
                    skillInfo = (InvSkill *) io->extra;
                }
                io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
            }

            if (!skillInfo)
            {
                sprintf(tempText,"BUG: Cannot find skill for level.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }
            else
            {
                // Calculate work value
                float work = skillInfo->skillLevel * MagicalStat() / 4.0f * rnd( 0.8f,1.5f );

                if (tokenMan.TokenIsInHere(MAGIC_EVIL, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
                    work *= 1.01f;

                if (specLevel[1] > 0)
                    work *= (1.0f + 0.05f * specLevel[1]);


                // Calculate challenge of shattering all magic from totem ( Quality + 1 ) * imbues
                float challenge = ( ((InvTotem *)tot->extra)->quality * 2 ) * ( ((InvTotem *)tot->extra)->quality * 2 ) + 
                    ((InvTotem *)tot->extra)->TotalImbues() * 10;

                if( challenge < 1 )
                    challenge = 1;

                sprintf(tempText,"Task difficulty is %4.2f.", challenge);
                CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                sprintf(tempText,"Your work value was %4.2f.", work);
                CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                if (challenge <= work) // success!
                {
                    sprintf(tempText,"You successfully shatter the totem with magic!  Its power engulfs the items in your workbench.");
                    CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);


                    //****
                    // BEGIN SHATTER ITEM CODE
                    //****

                    float * imbues;
                    imbues = ((InvTotem *)tot->extra)->imbue;

					if (imbues[MAGIC_MOON] == 7 && imbues[MAGIC_TURTLE] == 7) { // break heart gem.
						sprintf(tempText, "The released power wants to break a Heart Gem apart.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
						while (io) {
							if (INVOBJ_GEOPART == io->type && ((InvGeoPart*)io->extra)->type == 1 && imbues[MAGIC_EVIL] > 1 // one evil can't shatter anything. 
								&& ((InvGeoPart*)io->extra)->power>1) // only bigger than 1 can be shattered 
							{
								if (((InvGeoPart*)io->extra)->power <= imbues[MAGIC_EVIL]) { // 
									io->amount -= 1;
									if (io->amount<1)
										charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
									int imbues2 = ((InvGeoPart*)io->extra)->power;
									imbues[MAGIC_EVIL] -= ((InvGeoPart*)io->extra)->power;
									io = new InventoryObject(INVOBJ_GEOPART, 0, "Plain Heart Gem");
									io->extra = new InvGeoPart();
									((InvGeoPart*)io->extra)->type = 1;
									((InvGeoPart*)io->extra)->power = 1;
									io->amount = imbues2;
									io->mass = 0.0f;
									io->value = 400;
									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
							}

							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
						}

					}
                    if( imbues[ MAGIC_FROG ] == 2 && imbues[ MAGIC_BEAR ] == 2 ) { // green dust into white
                        sprintf(tempText,"The released power wants to turn Green Dust into White Dust.");
                        CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                        while (io) {
                            if ( INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_GREEN_DUST && imbues[ MAGIC_EVIL ] > 0 ) {
                                if( io->amount <= imbues[ MAGIC_EVIL ] ) {
                                    charInfoArray[curCharacterIndex].workbench->objects.Remove( io );
                                    sprintf( io->do_name, "Glowing White Dust" );
                                    ((InvIngredient*)io->extra)->type = INGR_WHITE_DUST;
                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                                else {
                                    io->amount -= imbues[ MAGIC_EVIL ];

                                    io = new InventoryObject( INVOBJ_INGREDIENT, 0, "Glowing White Dust" );
                                    io->extra = new InvIngredient();
                                    io->amount = imbues[ MAGIC_EVIL ];
                                    ((InvIngredient*)io->extra)->type = INGR_WHITE_DUST;
                                    ((InvIngredient*)io->extra)->quality = 1;
                                    io->mass = 0.0f;
                                    io->value = 1000;
                                    
                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                            }

                            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                        }
                    }
                    else if( imbues[ MAGIC_FROG ] == 2 && imbues[ MAGIC_TURTLE ] == 2 ) { // black dust into red
                        sprintf(tempText,"The released power wants to turn Black Dust into Red Dust.");
                        CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                        while (io) {
                            if ( INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_BLACK_DUST && imbues[ MAGIC_EVIL ] > 0 ) {
                                if( io->amount <= imbues[ MAGIC_EVIL ] ) {
                                    charInfoArray[curCharacterIndex].workbench->objects.Remove( io );

                                    sprintf( io->do_name, "Glowing Red Dust" );
                                    ((InvIngredient*)io->extra)->type = INGR_RED_DUST;

                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                                else {
                                    io->amount -= imbues[ MAGIC_EVIL ];

                                    io = new InventoryObject( INVOBJ_INGREDIENT, 0, "Glowing Red Dust" );
                                    io->extra = new InvIngredient();
                                    io->amount = imbues[ MAGIC_EVIL ];
                                    ((InvIngredient*)io->extra)->type = INGR_RED_DUST;
                                    ((InvIngredient*)io->extra)->quality = 1;
                                    io->mass = 0.0f;
                                    io->value = 1000;
                                    
                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                            }

                            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                        }
                    }
					if (imbues[MAGIC_SUN] == 2 && imbues[MAGIC_TURTLE] == 2) { // white into green
						sprintf(tempText, "The released power wants to turn White into Green Dust.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
						while (io) {
							if (INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_WHITE_DUST && imbues[MAGIC_EVIL] > 0) {
								if (io->amount <= imbues[MAGIC_EVIL]) {
									charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
									sprintf(io->do_name, "Glowing Green Dust");
									((InvIngredient*)io->extra)->type = INGR_GREEN_DUST;
									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
								else {
									io->amount -= imbues[MAGIC_EVIL];

									io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Green Dust");
									io->extra = new InvIngredient();
									io->amount = imbues[MAGIC_EVIL];
									((InvIngredient*)io->extra)->type = INGR_GREEN_DUST;
									((InvIngredient*)io->extra)->quality = 1;
									io->mass = 0.0f;
									io->value = 1000;

									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
							}

							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
						}
					}
					if (imbues[MAGIC_MOON] == 2 && imbues[MAGIC_SUN] == 2) { // white into Black
						sprintf(tempText, "The released power wants to turn White into Black Dust.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
						while (io) {
							if (INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_WHITE_DUST && imbues[MAGIC_EVIL] > 0) {
								if (io->amount <= imbues[MAGIC_EVIL]) {
									charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
									sprintf(io->do_name, "Glowing Black Dust");
									((InvIngredient*)io->extra)->type = INGR_BLACK_DUST;
									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
								else {
									io->amount -= imbues[MAGIC_EVIL];

									io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Black Dust");
									io->extra = new InvIngredient();
									io->amount = imbues[MAGIC_EVIL];
									((InvIngredient*)io->extra)->type = INGR_BLACK_DUST;
									((InvIngredient*)io->extra)->quality = 1;
									io->mass = 0.0f;
									io->value = 1000;

									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
							}

							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
						}
					}
					else if (imbues[MAGIC_EAGLE] == 2 && imbues[MAGIC_FROG] == 2) { // red into black
						sprintf(tempText, "The released power wants to turn Red Dust into Black Dust.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
						while (io) {
							if (INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_RED_DUST && imbues[MAGIC_EVIL] > 0) {
								if (io->amount <= imbues[MAGIC_EVIL]) {
									charInfoArray[curCharacterIndex].workbench->objects.Remove(io);

									sprintf(io->do_name, "Glowing Black Dust");
									((InvIngredient*)io->extra)->type = INGR_BLACK_DUST;

									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
								else {
									io->amount -= imbues[MAGIC_EVIL];

									io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Black Dust");
									io->extra = new InvIngredient();
									io->amount = imbues[MAGIC_EVIL];
									((InvIngredient*)io->extra)->type = INGR_BLACK_DUST;
									((InvIngredient*)io->extra)->quality = 1;
									io->mass = 0.0f;
									io->value = 1000;

									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
							}

							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
						}
					}
					else if (imbues[MAGIC_EAGLE] == 2 && imbues[MAGIC_TURTLE] == 2) { // Red dust into green dust
						sprintf(tempText, "The released power wants to turn Red Dust into Green Dust.");
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
						while (io) {
							if (INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_RED_DUST && imbues[MAGIC_EVIL] > 0) {
								if (io->amount <= imbues[MAGIC_EVIL]) {
									charInfoArray[curCharacterIndex].workbench->objects.Remove(io);

									sprintf(io->do_name, "Glowing Green Dust");
									((InvIngredient*)io->extra)->type = INGR_GREEN_DUST;

									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
								else {
									io->amount -= imbues[MAGIC_EVIL];

									io = new InventoryObject(INVOBJ_INGREDIENT, 0, "Glowing Green Dust");
									io->extra = new InvIngredient();
									io->amount = imbues[MAGIC_EVIL];
									((InvIngredient*)io->extra)->type = INGR_GREEN_DUST;
									((InvIngredient*)io->extra)->quality = 1;
									io->mass = 0.0f;
									io->value = 1000;

									charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
								}
							}

							io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
						}
					}
                    else if( imbues[ MAGIC_BEAR ] == 5 && imbues[ MAGIC_TURTLE ] == 5 ) { // green dust into blue
                        sprintf(tempText,"The released power wants to turn Green Dust into Blue Dust.");
                        CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                        while (io) {
                            if ( INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_GREEN_DUST && imbues[ MAGIC_EVIL ] > 0 ) {
                                if( io->amount <= imbues[ MAGIC_EVIL ] ) {
                                    charInfoArray[curCharacterIndex].workbench->objects.Remove( io );

                                    sprintf( io->do_name, "Glowing Blue Dust" );
                                    ((InvIngredient*)io->extra)->type = INGR_BLUE_DUST;

                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                                else {
                                    io->amount -= imbues[ MAGIC_EVIL ];

                                    io = new InventoryObject( INVOBJ_INGREDIENT, 0, "Glowing Blue Dust" );
                                    io->extra = new InvIngredient();
                                    io->amount = imbues[ MAGIC_EVIL ];
                                    ((InvIngredient*)io->extra)->type = INGR_BLUE_DUST;
                                    ((InvIngredient*)io->extra)->quality = 1;
                                    io->mass = 0.0f;
                                    io->value = 1000;
                                    
                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                            }

                            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                        }
                    }
                    else if( imbues[ MAGIC_BEAR ] == 7 && imbues[ MAGIC_TURTLE ] == 7 ) { // black dust into blue
                        sprintf(tempText,"The released power wants to turn Black Dust into Blue Dust.");
                        CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                        while (io) {
                            if ( INVOBJ_INGREDIENT == io->type && ((InvIngredient*)io->extra)->type == INGR_BLACK_DUST && imbues[ MAGIC_EVIL ] > 0 ) {
                                if( io->amount <= imbues[ MAGIC_EVIL ] ) {
                                    charInfoArray[curCharacterIndex].workbench->objects.Remove( io );

                                    sprintf( io->do_name, "Glowing Blue Dust" );
                                    ((InvIngredient*)io->extra)->type = INGR_BLUE_DUST;

                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                                else {
                                    io->amount -= imbues[ MAGIC_EVIL ];

                                    io = new InventoryObject( INVOBJ_INGREDIENT, 0, "Glowing Blue Dust" );
                                    io->extra = new InvIngredient();
                                    io->amount = imbues[ MAGIC_EVIL ];
                                    ((InvIngredient*)io->extra)->type = INGR_BLUE_DUST;
                                    ((InvIngredient*)io->extra)->quality = 1;
                                    io->mass = 0.0f;
                                    io->value = 1000;
                                    
                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
                                }
                            }

                            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                        }
                    }
                    else if( imbues[ MAGIC_WOLF ] == 6 && imbues[ MAGIC_SNAKE ] == 6 && imbues[ MAGIC_TURTLE ] == 6 ) { // rotate shard color + 1
                        sprintf(tempText,"The released power wants to change a shard's color.");
                        CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                        io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();

                        while (io) {
                            if ( INVOBJ_INGREDIENT == io->type && 
                                    ( ((InvIngredient*)io->extra)->type >= INGR_WHITE_SHARD &&
                                        ((InvIngredient*)io->extra)->type <= INGR_PINK_SHARD ) ) {
                                ((InvIngredient*)io->extra)->type += 1;

                                if( ((InvIngredient*)io->extra)->type > INGR_PINK_SHARD )
                                    ((InvIngredient*)io->extra)->type = INGR_WHITE_SHARD;

                                strcpy( io->do_name, dustNames[ ((InvIngredient*)io->extra)->type ] );

                                break;
                            }

                            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                        }
                    }
                    else if( imbues[ MAGIC_WOLF ] == 1 && imbues[ MAGIC_BEAR ] == 1 && imbues[ MAGIC_MOON ] == 1 ) { // Free both drakes
                        sprintf(tempText,"The released power wants to release your pets.");
                        CopyStringSafely(tempText,1024, infoText.text, MESSINFOTEXTLEN);
                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);		

                        PetDragonInfo *dInfo1 = &( charInfoArray[curCharacterIndex].petDragonInfo[0] );
                        PetDragonInfo *dInfo2 = &( charInfoArray[curCharacterIndex].petDragonInfo[1] );

                        MessGenericEffect messGE;
                        MessPet mPet;
                        
                        if( dInfo1->type != 255 ) {
                            messGE.avatarID = socketIndex;
                            messGE.mobID    = -1;
                            messGE.x        = cellX;
                            messGE.y        = cellY;
                            messGE.r        = 40;
                            messGE.g        = 40;
                            messGE.b        = 50;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 5; // in seconds
                            ss->SendToEveryoneNearBut(0, cellX, cellY,
                                    sizeof(messGE),(void *)&messGE);

                            sprintf(tempText,"%s is freed into the wild.", dInfo1->name);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->SendToEveryoneNearBut(0, cellX, cellY,
                                sizeof(infoText),(void *)&infoText);

                            mPet.avatarID = socketIndex;
                            CopyStringSafely(dInfo1->name,16, 
                                        mPet.name,16);
                            mPet.quality = dInfo1->quality;
                            mPet.type    = 255;
                            mPet.state   = dInfo1->state;
                            mPet.size    = dInfo1->lifeStage + dInfo1->healthModifier/10.0f;
                            mPet.which   = 0;

                            ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);

                            dInfo1->type = 255;
                        }

                        if( dInfo2->type != 255 ) {
                            messGE.avatarID = socketIndex;
                            messGE.mobID    = -1;
                            messGE.x        = cellX;
                            messGE.y        = cellY;
                            messGE.r        = 40;
                            messGE.g        = 40;
                            messGE.b        = 50;
                            messGE.type     = 0;  // type of particles
                            messGE.timeLen  = 5; // in seconds
                            ss->SendToEveryoneNearBut(0, cellX, cellY,
                                    sizeof(messGE),(void *)&messGE);

                            sprintf(tempText,"%s is freed into the wild.", dInfo2->name);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->SendToEveryoneNearBut(0, cellX, cellY,
                                sizeof(infoText),(void *)&infoText);

                            mPet.avatarID = socketIndex;
                            CopyStringSafely(dInfo2->name,16, 
                                        mPet.name,16);
                            mPet.quality = dInfo2->quality;
                            mPet.type    = 255;
                            mPet.state   = dInfo2->state;
                            mPet.size    = dInfo2->lifeStage + dInfo2->healthModifier/10.0f;
                            mPet.which   = 1;

                            ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);

                            dInfo2->type = 255;
                        }
                    }
                    
                    //****
                    // END SHATTER ITEM CODE
                    //****


                    // delete the totem
                    if (tot->amount > 1)
                        tot->amount--;
                    else
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(tot);
                        delete tot;
                    }
                    
                    // IF successful, you get skill sub-points for that skill.
                    if ( skillInfo->skillLevel < 30000 && challenge > work / 20 )
                    {
                        skillInfo->skillPoints += challenge* bboServer->ts_exp_multiplier;

                        if( skillInfo->skillLevel * skillInfo->skillLevel * 20 <= skillInfo->skillPoints )
                        {
                            // made a skill level!!!
                            skillInfo->skillLevel++;
                            sprintf(tempText,"You gained a skill level!!");
                            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                            
                            charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_SHATTER;

                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                        }
                    }

                }
                else
                {
                    sprintf(tempText, "Your attempt to magically break the totem fails. It crumbles to dust.");
                    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    if (tot->amount > 1)
                        tot->amount--;
                    else
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(tot);
                        delete tot;
                    }
                }

            }

        }
        
        return;
    }





    //         ***********************************
    int usedMagic = -1;
    for (int i = 0; i < MAGIC_MAX; ++i)
    {
        if (!strnicmp(magicNameList[i],combineSkillName, strlen(magicNameList[i])))
            usedMagic = i;
    }

    if (usedMagic > -1)
    {
        int num = 0, eggNum = 0, staffNum = 0;
        int favorType = -1, favorAmount = 0;
        InventoryObject *totem;
        InventoryObject *egg;
        InventoryObject *staff;

        InventoryObject *io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
        while (io)
        {
            if (INVOBJ_TOTEM == io->type)
            {
                num += io->amount;
                totem = io;
            }
            else if (INVOBJ_STAFF == io->type)
            {
                staffNum += io->amount;
                staff = io;
            }
            else if (INVOBJ_EGG == io->type)
            {
                eggNum += io->amount;
                egg = io;
            }
            else if (INVOBJ_FAVOR == io->type)
            {
                favorAmount += io->amount;
                if (-1 == favorType)
                    favorType = ((InvFavor *) io->extra)->spirit;
                else
                    favorType = -2;
            }
 //           else // seriously?!?! ther'es no technical need for this.
 //           {
 //               sprintf(tempText,"Use magic on a totem, staff or egg.");
 //               CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
 //               
 //               ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
 //               return;
 //           }

            io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
        }

        if (num > 1)
        {
            sprintf(tempText,"Use magic on one totem at a time.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        if (eggNum > 1)
        {
            sprintf(tempText,"Use magic on one egg at a time.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        if (staffNum > 1)
        {
            sprintf(tempText,"Use magic on one staff at a time.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

		if (num < 1 && eggNum < 1 && staffNum < 1)
		{
			sprintf(tempText, "Put a totem in your workbench.");
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			return;
		}
		if ((num + eggNum + staffNum) > 1)
		{
			sprintf(tempText, "You have too many imbuable items in the workbench.");
			CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);;

			ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			return;
		}

        InvSkill *skillInfo = NULL;
        io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
        while (io)
        {
            if (!strcmp(combineSkillName,io->WhoAmI()))
            {
                skillInfo = (InvSkill *) io->extra;
            }
            io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
        }

        if (!skillInfo)
        {
            sprintf(tempText,"BUG: Cannot find skill for magic.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        if (eggNum > 0)
        {
            InvEgg *iEgg = (InvEgg *) egg->extra;
            int minimum = 4 + iEgg->quality + iEgg->type;
            if (minimum > 10)
                minimum = 10;

            if (MagicalStat() < minimum)
            {
                sprintf(tempText,"Your magic is too weak to hatch this egg.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }

            int index;
            if (255 == charInfoArray[curCharacterIndex].petDragonInfo[0].type)
                index = 0;
            else if (255 == charInfoArray[curCharacterIndex].petDragonInfo[1].type)
                index = 1;
            else
            {
                sprintf(tempText,"You already have two pet drakes.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }

            if (0 == index)
                CopyStringSafely("Pet A", 1024,
                                 charInfoArray[curCharacterIndex].petDragonInfo[index].name,
                                      16);
            else
                CopyStringSafely("Pet B", 1024,
                                 charInfoArray[curCharacterIndex].petDragonInfo[index].name,
                                      16);

            charInfoArray[curCharacterIndex].petDragonInfo[index].age = 0;
            charInfoArray[curCharacterIndex].petDragonInfo[index].lastEatenTime.SetToNow();
            charInfoArray[curCharacterIndex].petDragonInfo[index].lifeStage = 0;
            charInfoArray[curCharacterIndex].petDragonInfo[index].type = iEgg->type;
            charInfoArray[curCharacterIndex].petDragonInfo[index].quality = iEgg->quality;
            charInfoArray[curCharacterIndex].petDragonInfo[index].state = DRAGON_HEALTH_GREAT;  // normal
            charInfoArray[curCharacterIndex].petDragonInfo[index].lastEatenType = 0;
            charInfoArray[curCharacterIndex].petDragonInfo[index].lastEatenSubType = 0;

            // if you used the right type of magic, it's born a little stronger.
            charInfoArray[curCharacterIndex].petDragonInfo[index].healthModifier = 0;
            if (usedMagic == dragonInfo[iEgg->quality][iEgg->type].powerBirthMagicType)
                charInfoArray[curCharacterIndex].petDragonInfo[index].healthModifier = 1.0;

            // that type of token is also important
            if (tokenMan.TokenIsInHere(dragonInfo[iEgg->quality][iEgg->type].powerBirthMagicType, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
            {
                charInfoArray[curCharacterIndex].petDragonInfo[index].healthModifier += 1.0;
            }

            if (specLevel[1] > 0)
                charInfoArray[curCharacterIndex].petDragonInfo[index].healthModifier += 
                          2 * specLevel[1];

            // tell everyone about it!
            MessPet mPet;
            mPet.avatarID = socketIndex;
            mPet.name[0] = 0;
            mPet.quality = charInfoArray[curCharacterIndex].petDragonInfo[index].quality;
            mPet.type    = charInfoArray[curCharacterIndex].petDragonInfo[index].type;
            mPet.state   = charInfoArray[curCharacterIndex].petDragonInfo[index].state;
            mPet.size    = charInfoArray[curCharacterIndex].petDragonInfo[index].lifeStage +
                    charInfoArray[curCharacterIndex].petDragonInfo[index].healthModifier / 10.0f;
            mPet.which   = index;

            ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);

            sprintf(tempText,"You have hatched a pet drake!");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            MessGenericEffect messGE;
            messGE.avatarID = socketIndex;
            messGE.mobID    = -1;
            messGE.x        = cellX;
            messGE.y        = cellY;
            messGE.r        = 255;
            messGE.g        = 255;
            messGE.b        = 0;
            messGE.type     = 0;  // type of particles
            messGE.timeLen  = 5; // in seconds
            ss->SendToEveryoneNearBut(0, cellX, cellY,
                              sizeof(messGE),(void *)&messGE);

            FILE *source = fopen("petInfo.txt","a");
            /* Display operating system-style date and time. */
            _strdate( tempText );
            fprintf(source, "%s, ", tempText );
            _strtime( tempText );
            fprintf(source, "%s, ", tempText );

            fprintf(source,"BIRTH, %s, %s, %d, %d\n",
                dragonInfo[mPet.quality][mPet.type].eggName,
                charInfoArray[curCharacterIndex].name,
                cellX, cellY);

            fclose(source);

            // destroy the egg
            charInfoArray[curCharacterIndex].workbench->objects.Remove(egg);
            delete egg;

            return;
        }


        float work = skillInfo->skillLevel * 
            MagicalStat() / 4.0f * rnd(0.7f,1.3f);

        if (tokenMan.TokenIsInHere(usedMagic, ss) && ( (TowerMap*) ss)->IsMember( charInfoArray[curCharacterIndex].name ) )
            work *= 1.3f;

        if (specLevel[1] > 0)
            work *= (1.0f + 0.05f * specLevel[1]);

        // ********** imbue staff
        if (staffNum > 0)
        {
            int result = ImbueStaff(staff, usedMagic, work);

            InvStaff *extra = (InvStaff *)staff->extra;

            float chall = ImbueStaffChallenge(extra);
            sprintf(tempText,"Task difficulty is %4.2f.", chall);
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            sprintf(tempText,"Your work value was %4.2f.", work);
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            unsigned long exp = StaffImbueExperience(staff, skillInfo->skillLevel);
            skillInfo->skillPoints += exp* bboServer->magic_exp_multiplier;

            switch (result)
            {
            case IMBUE_RES_SUCCESS:
                sprintf(tempText,"The magic infuses the staff!");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                UpdateStaff(staff, work);

                QuestImbue(ss, staff, chall, work, combineSkillName);
                break;

            case IMBUE_RES_FAIL:
                sprintf(tempText,"The magic fails to imbue the staff.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                break;

            case IMBUE_RES_DISINTEGRATE:
                sprintf(tempText,"The attempt destroys the staff.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                charInfoArray[curCharacterIndex].workbench->objects.Remove(staff);
                delete staff;
                staff = NULL;

                break;
            }

            if (staff)
            {
                int totalImbues = 0;
                for (int j = 0; j < MAGIC_MAX; ++j)
                {
                    totalImbues += extra->imbue[j];
                }
                if (totalImbues > extra->quality * 5 + 20 && staff)
                {
                    CopyStringSafely("The staff can't hold that much magic.  It shatters!",1024,infoText.text,MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                    charInfoArray[curCharacterIndex].workbench->objects.Remove(staff);
                    delete staff;
                }
            }

            if ( skillInfo->skillLevel < 100 && skillInfo->skillLevel * 30 <= skillInfo->skillPoints )
            {
                // made a skill level!!!
                skillInfo->skillLevel++;
                sprintf(tempText,"You gained a skill level!!");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;

                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }


            return;
        }


        // ********** imbue totem
        InvTotem *extra = (InvTotem *)totem->extra;

        if (extra->isActivated && 
             extra->type >= TOTEM_PHYSICAL &&
             extra->type <= TOTEM_CREATIVE)
        {
            CopyStringSafely("This type of totem cannot be further imbued.",1024,infoText.text,MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            return;
        }

        if (!extra->isActivated && favorType >= 0)
        {
            // special favor-imbue!
            if (usedMagic != favorType)
            {
                CopyStringSafely("The attempt feels painful and improper; it fizzles.",1024,infoText.text,MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                return;
            }

            sprintf(tempText,"Task difficulty is %d.", favorAmount + extra->quality);
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            sprintf(tempText,"Your work value was %4.2f.", work);
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            if (favorAmount + extra->quality <= work) // success!
            {
                // destroy favors from workbench
                io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.First();
                while (io)
                {
                    if (INVOBJ_FAVOR == io->type)
                    {
                        charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
                        delete io;
                    }
                    io = (InventoryObject *) charInfoArray[curCharacterIndex].workbench->objects.Next();
                }

                extra->isActivated = TRUE;
                extra->imbueDeviation = sqrt((float)favorAmount);
                extra->timeToDie.SetToNow();
                extra->timeToDie.AddMinutes(
                  (extra->quality + 1) * (extra->quality + 1) * 10);

                switch(favorType)
                {
                case MAGIC_BEAR:
                case MAGIC_EAGLE:
                case MAGIC_SUN:
                default:
                    extra->type = TOTEM_PHYSICAL;
                    break;
                
                case MAGIC_SNAKE:
                case MAGIC_MOON:
                case MAGIC_TURTLE:
                    extra->type = TOTEM_MAGICAL;
                    break;

                case MAGIC_WOLF:
                case MAGIC_FROG:
                    extra->type = TOTEM_CREATIVE;
                    break;
                }

                sprintf(totem->do_name, "%s %s Totem", totemQualityName[extra->quality],
                                                      totemTypeName[extra->type]);

                if (abs((int)(favorAmount - skillInfo->skillLevel)) < 8)
                {
                    int add = (favorAmount - skillInfo->skillLevel + 1);
                    if (add < 1)
                        add = 1;
                    skillInfo->skillPoints += add* bboServer->magic_exp_multiplier;
                }
            }
            else
            {
                sprintf(tempText,"The attempt fails.");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }

            if (skillInfo->skillLevel * 30 <= skillInfo->skillPoints)
            {
                // made a skill level!!!
                skillInfo->skillLevel++;
                sprintf(tempText,"You gained a skill level!!");
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;

                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }

            return;
        }


        int result = ImbueTotem(totem, usedMagic, work);

        sprintf(tempText,"Task difficulty is %4.2f.", (float)extra->quality);
        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
        
        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

        sprintf(tempText,"Your work value was %4.2f.", work);
        CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
        
        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

        unsigned long exp = TotemImbueExperience(totem, skillInfo->skillLevel);
        skillInfo->skillPoints += exp* bboServer->magic_exp_multiplier;

        switch (result)
        {
        case IMBUE_RES_SUCCESS:
            sprintf(tempText,"The magic infuses the totem!");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            UpdateTotem(totem);

            QuestImbue(ss, totem, (float) extra->quality, work, combineSkillName);
            break;

        case IMBUE_RES_FAIL:
            sprintf(tempText,"The magic fails to imbue the totem.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            break;

        case IMBUE_RES_DISINTEGRATE:
            sprintf(tempText,"The attempt destroys the totem.");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            charInfoArray[curCharacterIndex].workbench->objects.Remove(totem);
            delete totem;
            totem = NULL;

            break;
        }

        int totalImbues = 0;
        for (int j = 0; j < MAGIC_MAX && totem; ++j)
        {
            totalImbues += extra->imbue[j];
        }
        if (totalImbues > extra->quality + 10 && totem)
        {
            CopyStringSafely("The totem can't hold that much magic.  It shatters!",1024,infoText.text,MESSINFOTEXTLEN);
            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            charInfoArray[curCharacterIndex].workbench->objects.Remove(totem);
            delete totem;
        }
        

        if ( skillInfo->skillLevel < 100 && skillInfo->skillLevel * 30 <= skillInfo->skillPoints)
        {
            // made a skill level!!!
            skillInfo->skillLevel++;
            sprintf(tempText,"You gained a skill level!!");
            CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
            
            charInfoArray[curCharacterIndex].cLevel += CLEVEL_VAL_MAGIC;

            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
        }

        return;
    }

    sprintf(tempText,"No combining happened.");
    CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
    
    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
    
}

//******************************************************************
void BBOSAvatar::UpdateClient(SharedSpace *ss, int clearAll, int delay)
{
    int cX = cellX;
    int cY = cellY;
	Sleep(delay);
    if ((controlledMonster)&& (!followMode))
    {
        cX = controlledMonster->cellX;
        cY = controlledMonster->cellY;
    }
	
    if (clearAll)
    {
        for (int y = 0; y < MAP_SIZE_HEIGHT; ++y)
        {
            for (int x = 0; x < MAP_SIZE_WIDTH; ++x)
            {
                updateMap[y*MAP_SIZE_WIDTH+x] = FALSE;
            }
        }
    }

    // tell my client about other mobs and avatars
    for (int y = 0; y < MAP_SIZE_HEIGHT; ++y)
    {
        for (int x = 0; x < MAP_SIZE_WIDTH; ++x)
        {
            if (updateMap[y*MAP_SIZE_WIDTH+x])
            {
                if (abs(y - cY) > 4 || abs(x - cX) > 4)
                {
                    // get rid of existing static objects
                    updateMap[y*MAP_SIZE_WIDTH+x] = FALSE;
                }
            }
            else
            {
                if (abs(y - cY) <= 4 && abs(x - cX) <= 4)
                {
                    // tell my client about mobs in this square
                    updateMap[y*MAP_SIZE_WIDTH+x] = TRUE;
                    GiveInfoFor(x,y, ss);
                }
            }
        }
    }
}


//******************************************************************
int BBOSAvatar::GetDodgeLevel(void)
{

	InventoryObject *io = (InventoryObject *)
		charInfoArray[curCharacterIndex].skills->objects.First();
	while (io)
	{
		if (!strcmp("Dodging", io->WhoAmI()))
		{
			InvSkill *skillInfo = (InvSkill *)io->extra;
			return skillInfo->skillLevel;
		}
		io = (InventoryObject *)
			charInfoArray[curCharacterIndex].skills->objects.Next();
	}

	return 0;
}

int BBOSAvatar::GetPetEnergyLevel(void)
{

	InventoryObject *io = (InventoryObject *)
		charInfoArray[curCharacterIndex].skills->objects.First();
	while (io)
	{
		if (!strcmp("Pet Energy", io->WhoAmI()))
		{
			InvSkill *skillInfo = (InvSkill *)io->extra;
			return skillInfo->skillLevel;
		}
		io = (InventoryObject *)
			charInfoArray[curCharacterIndex].skills->objects.Next();
	}

	return 0;
}


//******************************************************************
void BBOSAvatar::IntroduceMyself(SharedSpace *ss, unsigned short special)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

//	if (isInvisible)
//		return;
	isTeleporting = false;
    MessBladeDesc messBladeDesc;

    // tell everyone about my arrival
    if (0 == special)
    {
        MessAvatarAppear messAvAppear;
        messAvAppear.avatarID = socketIndex;
        messAvAppear.x = cellX;
        messAvAppear.y = cellY;
        if (isInvisible)
            ss->lserver->SendMsg(sizeof(messAvAppear),(void *)&messAvAppear, 0, &tempReceiptList);
        else
            ss->SendToEveryoneNearBut(0, cellX, cellY,
                        sizeof(messAvAppear),(void *)&messAvAppear);
    }
    else
    {
        MessAvatarAppearSpecial messAvAppearSpecial;
        messAvAppearSpecial.avatarID = socketIndex;
        messAvAppearSpecial.x = cellX;
        messAvAppearSpecial.y = cellY;
        messAvAppearSpecial.typeOfAppearance = special;
        if (isInvisible)
            ss->lserver->SendMsg(sizeof(messAvAppearSpecial),(void *)&messAvAppearSpecial, 0, &tempReceiptList);
        else
            ss->SendToEveryoneNearBut(0, cellX, cellY,
                        sizeof(messAvAppearSpecial),(void *)&messAvAppearSpecial);
    }

    MessAvatarStats mStats;
    BuildStatsMessage(&mStats);
    if (isInvisible)
        ss->lserver->SendMsg(sizeof(mStats),(void *)&mStats, 0, &tempReceiptList);
    else
        ss->SendToEveryoneNearBut(0, cellX, cellY,
                    sizeof(mStats),(void *)&mStats);

    // tell people about my cool dragons!
    for (int index = 0; index < 2; ++index)
    {
        if (255 != charInfoArray[curCharacterIndex].petDragonInfo[index].type)
        {
            // tell everyone about it!
            MessPet mPet;
            mPet.avatarID = socketIndex;
            CopyStringSafely(charInfoArray[curCharacterIndex].petDragonInfo[index].name,16, 
                              mPet.name,16);
            mPet.quality = charInfoArray[curCharacterIndex].petDragonInfo[index].quality;
            mPet.type    = charInfoArray[curCharacterIndex].petDragonInfo[index].type;
            mPet.state   = charInfoArray[curCharacterIndex].petDragonInfo[index].state;
            mPet.size    = charInfoArray[curCharacterIndex].petDragonInfo[index].lifeStage +
                    charInfoArray[curCharacterIndex].petDragonInfo[index].healthModifier / 10.0f;
            mPet.which   = index;

            if (isInvisible)
                ss->lserver->SendMsg(sizeof(mPet),(void *)&mPet, 0, &tempReceiptList);
            else
                ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);
        }
    }

    // send the time to myself
    MessTimeOfDay mTime;
    mTime.value = bboServer->dayTimeCounter;
    ss->lserver->SendMsg(sizeof(mTime),(void *)&mTime, 0, &tempReceiptList);


    AssertGuildStatus(ss);

    // wield the wielded weapon (if you have one)
    InventoryObject *iObject = (InventoryObject *) 
        charInfoArray[curCharacterIndex].wield->objects.First();
    while (iObject)
    {
        if (INVOBJ_BLADE == iObject->type)
        {
            FillBladeDescMessage(&messBladeDesc, iObject, this);
            if (isInvisible)
                ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
            else
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                        sizeof(messBladeDesc),(void *)&messBladeDesc);

            return; // just the first wielded weapon
        }

        if (INVOBJ_STAFF == iObject->type)
        {
            InvStaff *iStaff = (InvStaff *) iObject->extra;
            
            messBladeDesc.bladeID = (long)iObject;
            messBladeDesc.size    = 4;
            messBladeDesc.r       = staffColor[iStaff->type][0];
            messBladeDesc.g       = staffColor[iStaff->type][1];
            messBladeDesc.b       = staffColor[iStaff->type][2]; 
            messBladeDesc.avatarID= socketIndex;
            messBladeDesc.trailType  = 0;
            messBladeDesc.meshType = BLADE_TYPE_STAFF1;
            if (isInvisible)
                ss->lserver->SendMsg(sizeof(messBladeDesc),(void *)&messBladeDesc, 0, &tempReceiptList);
            else
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                        sizeof(messBladeDesc),(void *)&messBladeDesc);
            return;
            
        }

        iObject = (InventoryObject *) 
            charInfoArray[curCharacterIndex].wield->objects.Next();
    }



}

//******************************************************************
void BBOSAvatar::AssertGuildStatus(SharedSpace *ss, int full, int socketTarget)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    // if I'm an admin
    if(ACCOUNT_TYPE_ADMIN == accountType)
    {
        Chronos::BStream *	stream		= NULL;
        stream	= new Chronos::BStream(sizeof(AvatarGuildName) + 65);

        *stream << (unsigned char) NWMESS_AVATAR_GUILD_NAME; 

        *stream << (int) socketIndex; 

        stream->write("Administrator", strlen("Administrator"));

        *stream << (unsigned char) 0; 

        if (isInvisible)
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        else if (socketTarget)
        {
            tempReceiptList.clear();
            tempReceiptList.push_back(socketTarget);
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        }
        else
            ss->SendToEveryoneNearBut(0, cellX, cellY, stream->used(), stream->buffer());

        delete stream;

        return;
    }

    // if I'm a moderator
    if( ACCOUNT_TYPE_MODERATOR == accountType || ACCOUNT_TYPE_TRIAL_MODERATOR == accountType )
    {
        Chronos::BStream *	stream		= NULL;
        stream	= new Chronos::BStream(sizeof(AvatarGuildName) + 65);

        *stream << (unsigned char) NWMESS_AVATAR_GUILD_NAME; 

        *stream << (int) socketIndex; 

        if( accountType == ACCOUNT_TYPE_MODERATOR )
            stream->write("Moderator", strlen("Moderator"));
        else
            stream->write("Trial Moderator", strlen("Trial Moderator"));

        *stream << (unsigned char) 0; 

        if (isInvisible)
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        else if (socketTarget)
        {
            tempReceiptList.clear();
            tempReceiptList.push_back(socketTarget);
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        }
        else
            ss->SendToEveryoneNearBut(0, cellX, cellY, stream->used(), stream->buffer());

        delete stream;

        return;
    }

    // if I belong to a guild, mention that
    SharedSpace *sx;
    if(bboServer->FindAvatarInGuild(charInfoArray[curCharacterIndex].name, &sx))
    {
        Chronos::BStream *	stream		= NULL;
        stream	= new Chronos::BStream(sizeof(AvatarGuildName) + 65);

        *stream << (unsigned char) NWMESS_AVATAR_GUILD_NAME; 

        *stream << (int) socketIndex; 

        stream->write(sx->WhoAmI(), strlen(sx->WhoAmI()));
        *stream << (unsigned char) 0; 

        if (isInvisible)
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        else if (socketTarget)
        {
            tempReceiptList.clear();
            tempReceiptList.push_back(socketTarget);
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        }
        else
            ss->SendToEveryoneNearBut(0, cellX, cellY, stream->used(), stream->buffer());

        delete stream;
    }
    else if (full)
    {
        Chronos::BStream *	stream		= NULL;
        stream	= new Chronos::BStream(sizeof(AvatarGuildName) + 65);

        *stream << (unsigned char) NWMESS_AVATAR_GUILD_NAME; 

        *stream << (int) socketIndex; 

        *stream << (unsigned char) 0; 

        if (isInvisible)
            ss->lserver->SendMsg(stream->used(), stream->buffer(), 0, &tempReceiptList);
        else
            ss->SendToEveryoneNearBut(0, cellX, cellY, stream->used(), stream->buffer());

        delete stream;
    }

}

//******************************************************************
void BBOSAvatar::MakeCharacterValid(int i)
{
	int total_stats = 0;

	if (charInfoArray[i].bottomIndex < 0)
		charInfoArray[i].bottomIndex = 0;

	if (charInfoArray[i].bottomIndex >= NUM_OF_BOTTOMS)
		charInfoArray[i].bottomIndex = NUM_OF_BOTTOMS - 1;

	if (charInfoArray[i].topIndex < -1)	  // -1 means empty slot
		charInfoArray[i].topIndex = -1;

	if (charInfoArray[i].topIndex >= NUM_OF_TOPS)
		charInfoArray[i].topIndex = NUM_OF_TOPS - 1;

	if (charInfoArray[i].faceIndex < 0)
		charInfoArray[i].faceIndex = 0;

	if (charInfoArray[i].faceIndex >= NUM_OF_FACES)
		charInfoArray[i].faceIndex = NUM_OF_FACES - 1;

	if (accountType)
		return;

	total_stats = charInfoArray[i].creative + charInfoArray[i].physical + charInfoArray[i].magical + charInfoArray[i].beast;

	if ((total_stats > 13 && charInfoArray[i].age < 4) || charInfoArray[i].creative < 1 || charInfoArray[i].physical < 1 || charInfoArray[i].magical < 1 || charInfoArray[i].beast < 1) 
	{
		charInfoArray[i].magical = charInfoArray[i].physical = charInfoArray[i].creative = 4;
		charInfoArray[i].beast = 1; // new stat
	}
}

//******************************************************************
void BBOSAvatar::LoadContacts(FILE *fp, float version)
{
    char tempText[1024];
    int linePoint, argPoint;
	bool done = FALSE;
    // delete old list
    delete contacts;
    contacts = new DoublyLinkedList();

    LoadLineToString(fp, tempText);

    while (strnicmp("XYZENDXYZ",tempText,9) && !done)
    {
        linePoint = 0;
        argPoint = NextWord(tempText,&linePoint);

        int val = atoi(&(tempText[argPoint]));
        argPoint = NextWord(tempText,&linePoint);

        DataObject *dat = new DataObject(val, &(tempText[argPoint]));
        contacts->Append(dat);

        LoadLineToString(fp, tempText);
		if (feof(fp))
			done=TRUE;
    }


}

//******************************************************************
void BBOSAvatar::SaveContacts(FILE *fp)
{
//	char tempText[1024];
//	int linePoint, argPoint;

    DataObject *dat = contacts->First();
    while (dat)
    {
        fprintf(fp,"%d %s\n",dat->WhatAmI(), dat->WhoAmI());
        dat = contacts->Next();
    }

    fprintf(fp,"XYZENDXYZ\n");
}

//******************************************************************
int BBOSAvatar::IsContact(char *name, int type)
{
    DataObject *dat = contacts->First();
    while (dat)
    {
        if (dat->WhatAmI() == type && IsCompletelySame(dat->WhoAmI(), name))
            return TRUE;

        dat = contacts->Next();
    }

    return FALSE;
}


//******************************************************************
void BBOSAvatar::HandleMeatRot(SharedSpace *ss)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    char tempText[1024];
    MessInfoText infoText;

    // handle for wield inventory
    InventoryObject *iObject = (InventoryObject *) 
        charInfoArray[curCharacterIndex].wield->objects.First();
    while (iObject)
    {
        if (INVOBJ_MEAT == iObject->type)
        {
            if (!HandleOneMeatRot(iObject))
            {
                sprintf(tempText,"Some %s rots away.",iObject->WhoAmI());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                charInfoArray[curCharacterIndex].wield->objects.Remove(iObject);
                delete iObject;
            }
        }
        iObject = (InventoryObject *) 
            charInfoArray[curCharacterIndex].wield->objects.Next();
    }


    // handle for normal inventory
    iObject = (InventoryObject *) 
        charInfoArray[curCharacterIndex].inventory->objects.First();
    while (iObject)
    {
        if (INVOBJ_MEAT == iObject->type)
        {
            if (!HandleOneMeatRot(iObject))
            {
                sprintf(tempText,"Some %s rots away.",iObject->WhoAmI());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                charInfoArray[curCharacterIndex].inventory->objects.Remove(iObject);
                delete iObject;
            }
        }
        iObject = (InventoryObject *) 
            charInfoArray[curCharacterIndex].inventory->objects.Next();
    }

    // handle for workbench inventory
    iObject = (InventoryObject *) 
        charInfoArray[curCharacterIndex].workbench->objects.First();
    while (iObject)
    {
        if (INVOBJ_MEAT == iObject->type)
        {
            if (!HandleOneMeatRot(iObject))
            {
                sprintf(tempText,"Some %s rots away.",iObject->WhoAmI());
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                charInfoArray[curCharacterIndex].workbench->objects.Remove(iObject);
                delete iObject;
            }
        }
        iObject = (InventoryObject *) 
            charInfoArray[curCharacterIndex].workbench->objects.Next();
    }

}

//******************************************************************
int BBOSAvatar::HandleOneMeatRot(InventoryObject *iObject)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    char tempText[1024];
    MessInfoText infoText;

    InvMeat *im = (InvMeat *) iObject->extra;

    if (im->age < 0)
        return 1;

    ++(im->age);
    if (24 == im->age) // if 2 hours have passed
    {
        sprintf(tempText,"Rotted %s",iObject->WhoAmI());
        CopyStringSafely(tempText, 1024, iObject->do_name, DO_NAME_LENGTH);
    }
    else if (48 <= im->age) // if 4 hours have passed
    {
        return 0;
    }

    return 1;
    
}


//******************************************************************
void BBOSAvatar::PetAging(SharedSpace *ss)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    char tempText[1024];
    MessInfoText infoText;

    LongTime ltNow;

    int stageTime = 48; // in 5 minute increments, so 1 hour = 12

    for (int i = 0; i < 2; ++i)
    {
        PetDragonInfo *dInfo = &(charInfoArray[curCharacterIndex].petDragonInfo[i]);

        if (255 != dInfo->type) // if a valid dragon
        {
            int oldState   = dInfo->state;

            if (dInfo->lastEatenTime.MinutesDifference(&ltNow) > 120)
            {
                // death by starvation!
                MessGenericEffect messGE;
                messGE.avatarID = socketIndex;
                messGE.mobID    = -1;
                messGE.x        = cellX;
                messGE.y        = cellY;
                messGE.r        = 40;
                messGE.g        = 40;
                messGE.b        = 50;
                messGE.type     = 0;  // type of particles
                messGE.timeLen  = 5; // in seconds
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                          sizeof(messGE),(void *)&messGE);

                sprintf(tempText,"%s dies of starvation.", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                          sizeof(infoText),(void *)&infoText);

                MessPet mPet;
                mPet.avatarID = socketIndex;
                CopyStringSafely(dInfo->name,16, 
                              mPet.name,16);
                mPet.quality = dInfo->quality;
                mPet.type    = 255;
                mPet.state   = dInfo->state;
                mPet.size    = dInfo->lifeStage + dInfo->healthModifier/10.0f;
                mPet.which   = i;

                ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);

                FILE *source = fopen("petInfo.txt","a");
                /* Display operating system-style date and time. */
                _strdate( tempText );
                fprintf(source, "%s, ", tempText );
                _strtime( tempText );
                fprintf(source, "%s, ", tempText );

                fprintf(source,"STARVATION, %s, %s, %d, %d\n",
                    dragonInfo[dInfo->quality][dInfo->type].eggName,
                    charInfoArray[curCharacterIndex].name,
                    cellX, cellY);

                fclose(source);

                dInfo->type = 255;

            }
            else if (dInfo->lastEatenTime.MinutesDifference(&ltNow) > 60)
            {
                sprintf(tempText,"%s is famished!", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                dInfo->healthModifier -= 0.1f;
                dInfo->state = DRAGON_HEALTH_SICK;
            }
            else if (dInfo->lastEatenTime.MinutesDifference(&ltNow) > 30)
            {
                sprintf(tempText,"%s is hungry.", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                dInfo->state = DRAGON_HEALTH_NORMAL;
            }
            else if (dInfo->lastEatenTime.MinutesDifference(&ltNow) > 20)
            {
                sprintf(tempText,"%s seems to be hungry.", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }

            ++(dInfo->age);
            if (dInfo->age >= 4 * stageTime)
            {
                // time... to die.
                MessGenericEffect messGE;
                messGE.avatarID = socketIndex;
                messGE.mobID    = -1;
                messGE.x        = cellX;
                messGE.y        = cellY;
                messGE.r        = 40;
                messGE.g        = 40;
                messGE.b        = 50;
                messGE.type     = 0;  // type of particles
                messGE.timeLen  = 5; // in seconds
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                          sizeof(messGE),(void *)&messGE);

                sprintf(tempText,"%s passes away.", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                          sizeof(infoText),(void *)&infoText);

                MessPet mPet;
                mPet.avatarID = socketIndex;
                CopyStringSafely(dInfo->name,16, 
                              mPet.name,16);
                mPet.quality = dInfo->quality;
                mPet.type    = 255;
                mPet.state   = dInfo->state;
                mPet.size    = dInfo->lifeStage + dInfo->healthModifier/10.0f;
                mPet.which   = i;

                ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);

                // give orchid if in graveyard
                // pet graveyard 90,53 and 71 206
                if (
                     (cellX >= 90 && cellX <= 91 && cellY >= 53 && cellY <= 54) ||
                     (cellX >= 71 && cellX <= 72 && cellY >= 206 && cellY <= 207)
                    )
                {
                    sprintf(tempText,"%s gives you a final gift for your kindness.", dInfo->name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    InventoryObject *iObject = new InventoryObject(INVOBJ_SIMPLE,0,"Dragon Orchid");
                    iObject->value  = 10000;
                    iObject->amount = 1;
                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);
                }


                FILE *source = fopen("petInfo.txt","a");
                /* Display operating system-style date and time. */
                _strdate( tempText );
                fprintf(source, "%s, ", tempText );
                _strtime( tempText );
                fprintf(source, "%s, ", tempText );

                fprintf(source,"DEATH, %s, %s, %d, %d\n",
                    dragonInfo[dInfo->quality][dInfo->type].eggName,
                    charInfoArray[curCharacterIndex].name,
                    cellX, cellY);

                fclose(source);

                // turn off the dragon
                dInfo->type = 255;

            }
            else if (dInfo->age == (dInfo->lifeStage+1) * stageTime)
            {
                // does she change?
                if (dragonInfo[dInfo->quality][dInfo->type].goodMeatType[dInfo->lifeStage] ==
                           dInfo->lastEatenType)
                {
                    dInfo->type = 
                        dragonInfo[dInfo->quality][dInfo->type].goodMeatTypeResult[dInfo->lifeStage];
                }
/*
                if (dragonInfo[dInfo->quality][dInfo->type].okayMeatType[dInfo->lifeStage] ==
                           dInfo->lastEatenType)
                {
                    dInfo->type = 
                        dragonInfo[dInfo->quality][dInfo->type].okayMeatTypeResult[dInfo->lifeStage];
                }
*/
                // does she drop an egg?
                if (dragonInfo[dInfo->quality][dInfo->type].breedMeatType[dInfo->lifeStage] ==
                           dInfo->lastEatenType ||
                    dragonInfo[dInfo->quality][dInfo->type].okayMeatType[dInfo->lifeStage] ==
                           dInfo->lastEatenType)
                {
                    sprintf(tempText,"%s lays an egg!", dInfo->name);
                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                    Inventory *inv = charInfoArray[curCharacterIndex].inventory;

                    int qual = dInfo->quality + 1;
                    if (qual > 3)
                        qual = 3;

                    InventoryObject *iObject = new InventoryObject(
                                INVOBJ_EGG,0,dragonInfo[qual][dInfo->type].eggName);
                    iObject->mass = 1.0f;
                    iObject->value = 1000;

                    InvEgg *im = (InvEgg *) iObject->extra;
                    im->type   = dInfo->type;
                    im->quality = qual;

                    inv->AddItemSorted(iObject);

                    FILE *source = fopen("petInfo.txt","a");
                    /* Display operating system-style date and time. */
                    _strdate( tempText );
                    fprintf(source, "%s, ", tempText );
                    _strtime( tempText );
                    fprintf(source, "%s, ", tempText );

                    fprintf(source,"EGG LAY, %s, %s, %d, %d\n",
                        dragonInfo[qual][dInfo->type].eggName,
                        charInfoArray[curCharacterIndex].name,
                        cellX, cellY);

                    fclose(source);

                }

                // time for a change!
                dInfo->lifeStage += 1;

                MessGenericEffect messGE;
                messGE.avatarID = socketIndex;
                messGE.mobID    = -1;
                messGE.x        = cellX;
                messGE.y        = cellY;
                messGE.r        = 255;
                messGE.g        = 255;
                messGE.b        = 0;
                messGE.type     = 0;  // type of particles
                messGE.timeLen  = 5; // in seconds
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                          sizeof(messGE),(void *)&messGE);

                MessPet mPet;
                mPet.avatarID = socketIndex;
                CopyStringSafely(dInfo->name,16, 
                              mPet.name,16);
                mPet.quality = dInfo->quality;
                mPet.type    = dInfo->type;
                mPet.state   = dInfo->state;
                mPet.size    = dInfo->lifeStage + dInfo->healthModifier/10.0f;
                mPet.which   = i;

                ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);

                sprintf(tempText,"%s has changed!", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            }
            else if (dInfo->age == (dInfo->lifeStage+1) * stageTime - 1 ||
                        dInfo->age == (dInfo->lifeStage+1) * stageTime - 2)
            {
                sprintf(tempText,"%s is acting strangely...", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            }
            else if (dInfo->age == (dInfo->lifeStage+1) * stageTime - 3)
            {
                sprintf(tempText,"%s just made a wierd noise...", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

            }

            if (dInfo->age == 4 * stageTime - 2)
            {
                sprintf(tempText,"%s looks very old and sickly...", dInfo->name);
                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                ss->SendToEveryoneNearBut(0, cellX, cellY,
                          sizeof(infoText),(void *)&infoText);

                dInfo->state = DRAGON_HEALTH_SICK;
            }

            if (oldState != dInfo->state)
            {
                MessPet mPet;
                mPet.avatarID = socketIndex;
                CopyStringSafely(dInfo->name,16, 
                              mPet.name,16);
                mPet.quality = dInfo->quality;
                mPet.type    = dInfo->type;
                mPet.state   = dInfo->state;
                mPet.size    = dInfo->lifeStage + dInfo->healthModifier/10.0f;
                mPet.which   = i;

                ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(mPet),(void *)&mPet);
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::AnnounceDisappearing(SharedSpace *sp, int type)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    // tell everyone I'm dissappearing
	isTeleporting = true;
    if (SPECIAL_APP_NOTHING != type)
    {
        MessAvatarAppearSpecial messAvAppearSpecial;
        messAvAppearSpecial.avatarID = socketIndex;
        messAvAppearSpecial.x = cellX;
        messAvAppearSpecial.y = cellY;
        messAvAppearSpecial.typeOfAppearance = type;

        if (isInvisible)
            sp->lserver->SendMsg(sizeof(messAvAppearSpecial),
                                  (void *)&messAvAppearSpecial, 
                                       0, &tempReceiptList);
        else
            sp->SendToEveryoneNearBut(0, cellX, cellY,
                    sizeof(messAvAppearSpecial),(void *)&messAvAppearSpecial);
    }

    MessAvatarDisappear aDisappear;
    aDisappear.avatarID = socketIndex;
    aDisappear.x = cellX;
    aDisappear.y = cellY;

    if (isInvisible)
        sp->lserver->SendMsg(sizeof(aDisappear),
                              (void *)&aDisappear, 
                                   0, &tempReceiptList);
    else
        sp->SendToEveryoneNearBut(0, cellX, cellY, 
                               sizeof(aDisappear), &aDisappear,5);  

    // stop any trading!
    charInfoArray[curCharacterIndex].inventory->partner = NULL;

}

//******************************************************************
void BBOSAvatar::AnnounceSpecial(SharedSpace *sp, int type)
{
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);

    MessAvatarAppearSpecial messAvAppearSpecial;
    messAvAppearSpecial.avatarID = socketIndex;
    messAvAppearSpecial.x = cellX;
    messAvAppearSpecial.y = cellY;
    messAvAppearSpecial.typeOfAppearance = type;

    if (isInvisible || isTeleporting)
        sp->lserver->SendMsg(sizeof(messAvAppearSpecial),
                              (void *)&messAvAppearSpecial, 
                                   0, &tempReceiptList);
    else
        sp->SendToEveryoneNearBut(0, cellX, cellY,
                sizeof(messAvAppearSpecial),(void *)&messAvAppearSpecial);
}

//******************************************************************
void BBOSAvatar::MoveControlledMonster(SharedSpace *ss, int deltaX, int deltaY)
{
	MessInfoText infoText;
    std::vector<TagID> tempReceiptList;
    tempReceiptList.clear();
    tempReceiptList.push_back(socketIndex);
	char tempText[1024];
    if (((!controlledMonster->isMoving)||(followMode)) && ss->CanMove(controlledMonster->cellX, controlledMonster->cellY, 
                controlledMonster->cellX + deltaX, controlledMonster->cellY + deltaY))
    {
        controlledMonster->inventory->partner = NULL;
		// check for dungeon entrance
		if (SPACE_GROUND == ss->WhatAmI())
		{
			// stepping into a dungeon?
			SharedSpace *sp = (SharedSpace *)bboServer->spaceList->First();

			while (sp)
			{
				if (SPACE_DUNGEON == sp->WhatAmI() &&
					!(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) && !(((DungeonMap *)sp)->specialFlags & SPECIAL_DUNGEON_DOOMTOWER))
				{
					if (((DungeonMap *)sp)->enterX == deltaX + controlledMonster->cellX &&
						((DungeonMap *)sp)->enterY == deltaY + controlledMonster->cellY &&
						((GroundMap *)ss)->type == 0  &&            // only in normal normal realm
						!((DungeonMap *)sp)->isLocked)              // controlled monsters can't enter locked dungeons
					{
						int lck = FALSE;
						if (((DungeonMap *)sp)->isLocked &&
							!(ACCOUNT_TYPE_ADMIN == accountType ||
								ACCOUNT_TYPE_MODERATOR == accountType ||
								ACCOUNT_TYPE_TRIAL_MODERATOR == accountType))
							return;

						charInfoArray[curCharacterIndex].inventory->partner = NULL;
						activeCounter = 0;

						// tell everyone I'm dissappearing
						AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
						// and make my monster vanish too
						MessMonsterDeath messMD;
						messMD.mobID = (unsigned long)controlledMonster;
						ss->SendToEveryoneNearBut(0, controlledMonster->cellX, controlledMonster->cellY,
							sizeof(messMD), (void *)&messMD);
						// remove it fropm old map
						ss->mobList->Remove(controlledMonster);
						

						QuestSpaceChange(ss, sp);

						// move me to my new SharedSpace
						ss->avatars->Remove(this);
						sp->avatars->Append(this);


						cellX = ((DungeonMap *)sp)->width - 1;
						cellY = ((DungeonMap *)sp)->height - 1;
                        // and the monster too
						controlledMonster->cellX = cellX;
						controlledMonster->cellY = cellY;
						// add the monster
						sp->mobList->Add(controlledMonster);
						// tell my client I'm entering the dungeon
						MessChangeMap changeMap;
						changeMap.dungeonID = (long)sp;
						changeMap.oldType = ss->WhatAmI();
						changeMap.newType = sp->WhatAmI();
						changeMap.sizeX = ((DungeonMap *)sp)->width;
						changeMap.sizeY = ((DungeonMap *)sp)->height;
						changeMap.flags = MESS_CHANGE_NOTHING;

						if (((DungeonMap *)sp)->CanEdit(this))
							changeMap.flags = MESS_CHANGE_EDITING;

						bboServer->lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

						sprintf_s(tempText, "You enter the %s.", ((DungeonMap *)sp)->name);
						memcpy(infoText.text, tempText, MESSINFOTEXTLEN - 1);
						infoText.text[MESSINFOTEXTLEN - 1] = 0;
						bboServer->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						// tell everyone about my arrival

						IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
						// tell clients about my monster
						controlledMonster->AnnounceMyselfCustom(sp);
						// tell client where ithe controlled monster is
						MessMobBeginMoveSpecial bMoveSpecial;
						bMoveSpecial.mobID = (unsigned long)controlledMonster;
						bMoveSpecial.x = controlledMonster->cellX;
						bMoveSpecial.y = controlledMonster->cellY;
						bMoveSpecial.targetX = controlledMonster->cellX;
						bMoveSpecial.targetY = controlledMonster->cellY;
						bMoveSpecial.specialType = 1;  // controlled monster
						sp->lserver->SendMsg(sizeof(bMoveSpecial),
							(void *)&bMoveSpecial,
							0, &tempReceiptList);

						// tell this player about everyone else around
						UpdateClient(sp, TRUE); 
						return;		// move finished.
					}
				}
				// no sending monsters into guild towers like this.
				sp = (SharedSpace *)bboServer->spaceList->Next();
			}

		}
		// not a cave, do next check.
		// check for dungeon based binding
		// can't move from a static dungeon monster
		if (SPACE_DUNGEON == ss->WhatAmI())
		{
			if (!((DungeonMap *)ss)->CanEdit(this))
			{
				BBOSMob *curMob = (BBOSMob *)ss->mobList->GetFirst(controlledMonster->cellX, controlledMonster->cellY);
				while (curMob)
				{
					if (SMOB_MONSTER == curMob->WhatAmI())
						//								&&
						//								 !((BBOSMonster *) curMob)->uniqueName[0])
					{
						if (!((BBOSMonster *)curMob)->isWandering && // not wandering
							!((BBOSMonster *)curMob)->isPossessed && // not possessed
							((BBOSMonster *)curMob)->controllingAvatar==NULL) // it's not being controlled
						{
							if ((curMob->cellX == controlledMonster->cellX) &&
								(curMob->cellY == controlledMonster->cellY) && accountType==ACCOUNT_TYPE_PLAYER) // admins can still move monsters
							{
								sprintf_s(infoText.text, "You are magically held to this spot!");
								bboServer->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								return;
							}
						}
					}

					curMob = (BBOSMob *)ss->mobList->GetNext();
				}
			}
		}
		// finally check for warp points
		BBOSMob *curMob = (BBOSMob *)ss->mobList->GetFirst(
			deltaX + controlledMonster->cellX
			, deltaY + controlledMonster->cellY);
		while (curMob)
		{
			if (SMOB_WARP_POINT == curMob->WhatAmI())
			{
				if ((((BBOSWarpPoint *)curMob)->allCanUse ||
					ACCOUNT_TYPE_ADMIN == accountType) &&
					((BBOSWarpPoint *)curMob)->spaceType < 100)
				{
					activeCounter = 0;
					// remove all doomkeys
					InventoryObject *io;
					// destroy doomkeys from workbench
					io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.First();
					while (io)
					{
						if (INVOBJ_DOOMKEY == io->type)
						{
							charInfoArray[curCharacterIndex].workbench->objects.Remove(io);
							delete io;
						}
						io = (InventoryObject *)charInfoArray[curCharacterIndex].workbench->objects.Next();
					}
					// destroy doomkeys from inventory
					io = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.First();
					while (io)
					{
						if (INVOBJ_DOOMKEY == io->type)
						{
							charInfoArray[curCharacterIndex].inventory->objects.Remove(io);
							delete io;
						}
						io = (InventoryObject *)charInfoArray[curCharacterIndex].inventory->objects.Next();
					}
					// destroy doomkeys from wielded area
					io = (InventoryObject *)charInfoArray[curCharacterIndex].wield->objects.First();
					while (io)
					{
						if (INVOBJ_DOOMKEY == io->type)
						{
							charInfoArray[curCharacterIndex].wield->objects.Remove(io);
							delete io;
						}
						io = (InventoryObject *)charInfoArray[curCharacterIndex].wield->objects.Next();
					}
					// remove all earthkey resumes. if you died outside a geo you really shouldn't have done that.
					// but only do this if you are in an earthkey
					if ((ss->WhatAmI() == SPACE_DUNGEON) && (((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY)) // if it's a geo
					{
						// destroy resumes from workbench
						io = (InventoryObject *)((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].workbench->objects.First();
						while (io)
						{
							if (INVOBJ_EARTHKEY_RESUME == io->type)
							{
								((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].workbench->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].workbench->objects.Next();
						}
						// destroy resumes from inventory
						io = (InventoryObject *)((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].inventory->objects.First();
						while (io)
						{
							if (INVOBJ_EARTHKEY_RESUME == io->type)
							{
								((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].inventory->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].inventory->objects.Next();
						}
						// destroy resumes from wielded area
						io = (InventoryObject *)((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].wield->objects.First();
						while (io)
						{
							if (INVOBJ_EARTHKEY_RESUME == io->type)
							{
								((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].wield->objects.Remove(io);
								delete io;
							}
							io = (InventoryObject *)((BBOSAvatar *)this)->charInfoArray[((BBOSAvatar *)this)->curCharacterIndex].wield->objects.Next();
						}
					}

					// tell everyone I'm dissappearing
					AnnounceDisappearing(ss, SPECIAL_APP_DUNGEON);
					// and make my monster vanish too
					MessMonsterDeath messMD;
					messMD.mobID = (unsigned long)controlledMonster;
					ss->SendToEveryoneNearBut(0, controlledMonster->cellX, controlledMonster->cellY,
						sizeof(messMD), (void *)&messMD);
					// remove it fropm old map
					ss->mobList->Remove(controlledMonster);
					SharedSpace *sp = (SharedSpace *)bboServer->spaceList->First();
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
								charInfoArray[curCharacterIndex].inventory->partner = NULL;

								// tell my client I'm leaving the dungeon
								MessChangeMap changeMap;
								changeMap.oldType = ss->WhatAmI();
								changeMap.newType = ((BBOSWarpPoint *)curMob)->spaceType;
								changeMap.realmID = ((BBOSWarpPoint *)curMob)->spaceSubType;
								changeMap.sizeX = sp->sizeX;
								changeMap.sizeY = sp->sizeY;
								changeMap.flags = MESS_CHANGE_NOTHING; // need to initialize!
								bboServer->lserver->SendMsg(sizeof(changeMap), (void *)&changeMap, 0, &tempReceiptList);

								QuestSpaceChange(ss, sp);

								// move me to my new SharedSpace
								ss->avatars->Remove(this);
								sp->avatars->Append(this);
								// add the monster
								sp->mobList->Add(controlledMonster);


								cellX = ((BBOSWarpPoint *)curMob)->targetX;
								cellY = ((BBOSWarpPoint *)curMob)->targetY;
								// and the monster too
								controlledMonster->cellX = cellX;
								controlledMonster->cellY = cellY;
								// tell client where ithe controlled monster is
								MessMobBeginMoveSpecial bMoveSpecial;
								bMoveSpecial.mobID = (unsigned long)controlledMonster;
								bMoveSpecial.x = controlledMonster->cellX;
								bMoveSpecial.y = controlledMonster->cellY;
								bMoveSpecial.targetX = controlledMonster->cellX;
								bMoveSpecial.targetY = controlledMonster->cellY;
								bMoveSpecial.specialType = 1;  // controlled monster
								sp->lserver->SendMsg(sizeof(bMoveSpecial),
									(void *)&bMoveSpecial,
									0, &tempReceiptList);
								// tell everyone about my arrival
								IntroduceMyself(sp, SPECIAL_APP_DUNGEON);
								// tell clients about my monster
								controlledMonster->AnnounceMyselfCustom(sp);
								// tell this player about everyone else around
								UpdateClient(sp, TRUE);
								return;		// move finished.
							}
						}
						sp = (SharedSpace *)bboServer->spaceList->Next();
					}
				}
			}

			curMob = (BBOSMob *)ss->mobList->GetNext();
		}


        // okay, let's go!
        controlledMonster->isMoving = TRUE;
		activeCounter = 0;
        controlledMonster->targetCellX = controlledMonster->cellX + deltaX;
        controlledMonster->targetCellY = controlledMonster->cellY + deltaY;
        controlledMonster->moveStartTime = timeGetTime();
		if (abs(deltaX) + abs(deltaY) < 2) // if it's straight.
		{
			// speed move up to match player
			controlledMonster->moveStartTime = controlledMonster->moveStartTime - 1500;
		}
        MessMobBeginMove bMove;
        bMove.mobID = (unsigned long) controlledMonster;
        bMove.x = controlledMonster->cellX;
        bMove.y = controlledMonster->cellY;
        bMove.targetX = controlledMonster->cellX + deltaX;
        bMove.targetY = controlledMonster->cellY + deltaY;
        ss->SendToEveryoneNearBut(socketIndex, controlledMonster->cellX, 
                       controlledMonster->cellY, sizeof(bMove), &bMove);
			MessMobBeginMoveSpecial bMoveSpecial;
			bMoveSpecial.mobID = (unsigned long)controlledMonster;
			bMoveSpecial.x = controlledMonster->cellX;
			bMoveSpecial.y = controlledMonster->cellY;
			bMoveSpecial.targetX = controlledMonster->cellX + deltaX;
			bMoveSpecial.targetY = controlledMonster->cellY + deltaY;
			if (!followMode)
				bMoveSpecial.specialType = 1;  // controlled monster
			else
				bMoveSpecial.specialType = 0;  // controlled monster
			ss->lserver->SendMsg(sizeof(bMoveSpecial),
				(void *)&bMoveSpecial,
				0, &tempReceiptList);
    }

}


//******************************************************************
void BBOSAvatar::AbortSecureTrading(SharedSpace *ss)
{
    // if the partner is valid
    if (tradingPartner)
    {
        SharedSpace *sx;
        BBOSAvatar *partnerAvatar = NULL;
        partnerAvatar = bboServer->FindAvatar(tradingPartner, &sx);

        if (partnerAvatar)
        {
            // make the partner's partner invalid
            partnerAvatar->tradingPartner = NULL;

            // call this function for the partner
            partnerAvatar->AbortSecureTrading(sx);
        }
    }

    // don't delete anything left in the trade inventory
    if (curCharacterIndex > -1)
    {
        InventoryObject *io = (InventoryObject *) trade->objects.First();
        while (io)
        {
            trade->objects.Remove(io);

            if (io->amount == 0)
            {
                delete io;
            }
            else
            {
                charInfoArray[curCharacterIndex].inventory->AddItemSorted(io);
            }

            io = (InventoryObject *) trade->objects.First();
        }

        assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

        charInfoArray[curCharacterIndex].inventory->money += trade->money;

        assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

        trade->money = 0;

        std::vector<TagID> tempReceiptList;
        tempReceiptList.clear();
        tempReceiptList.push_back(socketIndex);

        MessSecureTrade mess;
        mess.type = MESS_SECURE_STOP; 
        ss->lserver->SendMsg(sizeof(mess),(void *)&mess, 0, &tempReceiptList);
    }

    tradingPartner = NULL;

    assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

    bboServer->UpdateInventory(this);

    assert(charInfoArray[curCharacterIndex].inventory->money >= 0);
}

//******************************************************************
void BBOSAvatar::CompleteSecureTrading(SharedSpace *ss)
{
    assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

    // if the partner is valid
    if (tradingPartner)
    {
        SharedSpace *sx;
        BBOSAvatar *partnerAvatar = NULL;
        partnerAvatar = bboServer->FindAvatar(tradingPartner, &sx);

        if (partnerAvatar)
        {
            char tempText[1024];
            char dateString[128], timeString[128];

            _strdate( dateString );
            _strtime( timeString );

            FILE *source = fopen("logs\\tradeLog.txt","a");

            int iGave = FALSE;
            // give her my stuff
            InventoryObject *io = (InventoryObject *) trade->objects.First();
            while (io)
            {
                /* Display operating system-style date and time. */
                _strdate( tempText );
                fprintf(source, "%s, ", tempText );
                _strtime( tempText );
                fprintf(source, "%s, ", tempText );

                fprintf(source,"%d, %s, FROM %s, %s, %ld TO %s, %s, %ld\n",
                    io->amount, io->WhoAmI(),
                     name,
                     charInfoArray[curCharacterIndex].name,
                     charInfoArray[curCharacterIndex].inventory->money,
                     partnerAvatar->name,
                     partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name,
                     partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].inventory->money);

                if (INVOBJ_BLADE == io->type)
                {
                    sprintf(tempText,"SECURE-TRADE, %d, %s, FROM, %s, %s, %s, %s, %s, %d, %d, TO, %s, %s\n", 
                                io->amount, io->WhoAmI(),
                                 charInfoArray[curCharacterIndex].name,
                                 name, 
                                  dateString, timeString,
                                  ss->WhoAmI(), cellX, cellY,
                                 partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name,
                                 partnerAvatar->name
                                  );
                        LogOutput("swordTransferLog.txt", tempText);
                }
                else if (INVOBJ_INGREDIENT == io->type)
                {
                    sprintf(tempText,"SECURE-TRADE, %d, %s, FROM, %s, %s, %s, %s, %s, %d, %d, TO, %s, %s\n", 
                                io->amount, io->WhoAmI(),
                                 charInfoArray[curCharacterIndex].name,
                                 name, 
                                  dateString, timeString,
                                  ss->WhoAmI(), cellX, cellY,
                                 partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name,
                                 partnerAvatar->name
                                  );
                        LogOutput("dustTransferLog.txt", tempText);
                }


                trade->objects.Remove(io);

                if (io->amount == 0)
                {
                    delete io;
                }
                else
                {
                    partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].
                                         inventory->AddItemSorted(io);
                }

                iGave = TRUE;			
                io = (InventoryObject *) trade->objects.First();
            }


            QuestGiveGold(ss, partnerAvatar, trade->money);

            partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].inventory->money += 
                               trade->money;
            assert(
                partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].inventory->money >= 0);

            if (trade->money > 0)
                iGave = TRUE;
            trade->money = 0;

            int sheGave = FALSE;

            // give me her stuff
            io = (InventoryObject *) partnerAvatar->trade->objects.First();
            while (io)
            {
                /* Display operating system-style date and time. */
                _strdate( tempText );
                fprintf(source, "%s, ", tempText );
                _strtime( tempText );
                fprintf(source, "%s, ", tempText );

                fprintf(source,"%d, %s, FROM %s, %s, %ld TO %s, %s, %ld\n",
                    io->amount, io->WhoAmI(),
					partnerAvatar->name,
                    partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name,
                    partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].inventory->money,
					name,
					charInfoArray[curCharacterIndex].name,
					charInfoArray[curCharacterIndex].inventory->money);

                if (INVOBJ_BLADE == io->type)
                {
                    sprintf(tempText,"SECURE-TRADE, %d, %s, FROM, %s, %s, %s, %s, %s, %d, %d, TO, %s, %s\n", 
                                io->amount, io->WhoAmI(),
                                 partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name,
                                 partnerAvatar->name,
                                  dateString, timeString,
                                  ss->WhoAmI(), cellX, cellY,
                                 charInfoArray[curCharacterIndex].name,
                                 name
                                  );
                        LogOutput("swordTransferLog.txt", tempText);
                }
                else if (INVOBJ_INGREDIENT == io->type)
                {
                    sprintf(tempText,"SECURE-TRADE, %d, %s, FROM, %s, %s, %s, %s, %s, %d, %d, TO, %s, %s\n", 
                                io->amount, io->WhoAmI(),
                                 partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].name,
                                 partnerAvatar->name,
                                  dateString, timeString,
                                  ss->WhoAmI(), cellX, cellY,
                                 charInfoArray[curCharacterIndex].name,
                                 name 
                                  );
                        LogOutput("dustTransferLog.txt", tempText);
                }


                partnerAvatar->trade->objects.Remove(io);

                if (io->amount == 0)
                {
                    delete io;
                }
                else
                {
                    charInfoArray[curCharacterIndex].
                                         inventory->AddItemSorted(io);
                }

                sheGave = TRUE;
                io = (InventoryObject *) partnerAvatar->trade->objects.First();
            }

            fclose(source);

            assert(charInfoArray[curCharacterIndex].inventory->money >= 0);
			
            partnerAvatar->QuestGiveGold(ss, this, partnerAvatar->trade->money);

            charInfoArray[curCharacterIndex].inventory->money += 
                               partnerAvatar->trade->money;
            assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

            if (partnerAvatar->trade->money > 0)
                sheGave = TRUE;
         partnerAvatar->trade->money = 0;

            assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

            // KARMA
            int myRelationship, herRelationship;

            CompareWith(partnerAvatar, myRelationship, herRelationship);

            if (sheGave && !iGave)
            {
                partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].
                      karmaGiven[herRelationship][SAMARITAN_TYPE_GIFT] += 1;
                charInfoArray[curCharacterIndex].
                      karmaReceived[myRelationship][SAMARITAN_TYPE_GIFT] += 1;

                partnerAvatar->LogKarmaExchange(
                           this, herRelationship, myRelationship, SAMARITAN_TYPE_GIFT);

                if (IsAGuildMate(partnerAvatar))
                {
                    partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].
                          karmaGiven[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_GIFT] += 1;
                    charInfoArray[curCharacterIndex].
                          karmaReceived[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_GIFT] += 1;

                    partnerAvatar->LogKarmaExchange(
                         this, SAMARITAN_REL_GUILD, SAMARITAN_REL_GUILD, SAMARITAN_TYPE_GIFT);
                }
            }
            if (!sheGave && iGave)
            {
                partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].
                      karmaReceived[herRelationship][SAMARITAN_TYPE_GIFT] += 1;
                charInfoArray[curCharacterIndex].
                      karmaGiven[myRelationship][SAMARITAN_TYPE_GIFT] += 1;

                LogKarmaExchange(partnerAvatar,
                           myRelationship, herRelationship, SAMARITAN_TYPE_GIFT);

                if (IsAGuildMate(partnerAvatar))
                {
                    partnerAvatar->charInfoArray[partnerAvatar->curCharacterIndex].
                          karmaReceived[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_GIFT] += 1;
                    charInfoArray[curCharacterIndex].
                          karmaGiven[SAMARITAN_REL_GUILD][SAMARITAN_TYPE_GIFT] += 1;

                    LogKarmaExchange(partnerAvatar,
                         SAMARITAN_REL_GUILD, SAMARITAN_REL_GUILD, SAMARITAN_TYPE_GIFT);
                }
            }
            // END KARMA

            if (bHasLoaded)
            {
                SaveAccount();
                partnerAvatar->SaveAccount();
            }

            // make the partner's partner invalid
            partnerAvatar->tradingPartner = NULL;

            // call the abort function for the partner
            partnerAvatar->AbortSecureTrading(sx);

            assert(charInfoArray[curCharacterIndex].inventory->money >= 0);
        }
    }

    AbortSecureTrading(ss);

}


//******************************************************************
void BBOSAvatar::StateNoAgreement(SharedSpace *ss)
{
    agreeToTrade = FALSE;
    // if the partner is valid
    if (tradingPartner)
    {
        SharedSpace *sx;
        BBOSAvatar *partnerAvatar = NULL;
        partnerAvatar = bboServer->FindAvatar(tradingPartner, &sx);

        if (partnerAvatar)
        {
            partnerAvatar->agreeToTrade = FALSE;

            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessSecureTrade mess;
            mess.type = MESS_SECURE_NO_AGREEMENT; 
            ss->lserver->SendMsg(sizeof(mess),(void *)&mess, 0, &tempReceiptList);

            tempReceiptList.clear();
            tempReceiptList.push_back(partnerAvatar->socketIndex);
            sx->lserver->SendMsg(sizeof(mess),(void *)&mess, 0, &tempReceiptList);

            assert(charInfoArray[curCharacterIndex].inventory->money >= 0);

        }
    }
}

//******************************************************************
long BBOSAvatar::InventoryValue(void)
{
    float totalVal = 0;

    InventoryObject *io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].inventory->objects.First();
    while (io)
    {
        totalVal += io->amount * io->value;

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].inventory->objects.Next();
    }

    io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].wield->objects.First();
    while (io)
    {
        totalVal += io->amount * io->value;

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].wield->objects.Next();
    }

    io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].workbench->objects.First();
    while (io)
    {
        totalVal += io->amount * io->value;

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].workbench->objects.Next();
    }

    return (long) (totalVal/1000);
}


//******************************************************************
void BBOSAvatar::UseStaffOnMonster(SharedSpace *ss, InvStaff *iStaff, BBOSMonster *curMonster)
{
	char tempText[1024];
	MessInfoText infoText;
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(socketIndex);
	MessMonsterHealth messMH;

    // 1. chance = 30, staffs cant' misss
    int chance = 30;  // Hit!


	if (chance > 20)



    // always hit!
    {
		if (curMonster->controllingAvatar && (curMonster->controllingAvatar == this)) // if it's my pet
		{
			return; // ignore my pet. can't refund chanrge becase this could have been an AOE, and that allos an exploit.
		}
		if (curMonster->controllingAvatar && (curMonster->controllingAvatar->BeastStat() > 9)  // someone else's pet
			&& ((!curMonster->controllingAvatar->PVPEnabled)||(!PVPEnabled)))                  // and pvp is off for both.
		{
			return; // ignore other beastmaster pets too. 
		}
        int effectVal = 3 + iStaff->quality * 5 - iStaff->imbueDeviation;
        if (effectVal < 0)
            effectVal = 0;
        long damValue = 0;

        if (specLevel[1] > 0)
            effectVal += specLevel[1]*2;

        switch (iStaff->type)
        {
        case STAFF_DAMAGE:
        case STAFF_AREA_DAMAGE:

            // 3. damage = Physical * weapon.Damage
            damValue = (iStaff->quality + 1) * (10 - iStaff->imbueDeviation) * 
                                (1.0f + MagicalStat() * 0.5f);

            if (StaffAffectsArea(iStaff))
                damValue = (long)(damValue * 0.6f);

            if (specLevel[1] > 0)
                damValue = damValue * (1.0f + 0.05f * specLevel[1]);

            curMonster->health -= damValue;
            curMonster->RecordDamageForLootDist(damValue, this);

            messMH.mobID = (unsigned long)curMonster;
            messMH.health = curMonster->health;
            messMH.healthMax = curMonster->maxHealth;
            ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH),(void *)&messMH, 2);

            break;

        case STAFF_BLIND:
        case STAFF_AREA_BLIND:
            curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE, 
                effectVal * 1000, effectVal);

            break;

        case STAFF_SLOW:
        case STAFF_AREA_SLOW:
            curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE, 
                effectVal * 1000, effectVal);

            break;

        case STAFF_POISON:
        case STAFF_AREA_POISON:
            curMonster->MonsterMagicEffect(DRAGON_TYPE_BLACK, 
                effectVal * 1000, effectVal);

            curMonster->RecordDamageForLootDist(effectVal * effectVal / 20, this);

            break;

        case STAFF_STUN:
        case STAFF_AREA_STUN:
            curMonster->MonsterMagicEffect(MONSTER_EFFECT_STUN, 
                effectVal * 1000, effectVal);

            break;

        case STAFF_BIND:
        case STAFF_AREA_BIND:
            curMonster->MonsterMagicEffect(MONSTER_EFFECT_BIND, 
                effectVal * 1000, effectVal);

            break;
		case STAFF_AREA_TAUNT:
			DungeonMap * dm = (DungeonMap *)ss; // define the dungeon map, cast is safe because we won's use it if it't not really a dungeon
			if ((iStaff->imbueDeviation < 1) // only works if it's perfect
				&& (ss->WhatAmI()==SPACE_DUNGEON) // and when in a dungeon
				&& (dm->specialFlags & SPECIAL_DUNGEON_TEMPORARY) // and it's a geo (safe because of short circuit above)
				&& (curMonster->type > 8) // from sprit vision
				&& (curMonster->type < 18) // to spirit spider
 				&& (curMonster->subType < 1) // not a cent or dokk.
				&& ((this->cellX != curMonster->cellX)
				|| (this->cellY != curMonster->cellY))) // is not already in my square
			{
				curMonster->targetCellX = this->cellX; //tell the monster to try move to my square 
				curMonster->targetCellY = this->cellY;
				curMonster->isMoving = TRUE;  // and make it actuall move
				curMonster->moveStartTime = timeGetTime();

				MessMobBeginMove bMove;
				bMove.mobID = (unsigned long)curMonster;
				bMove.x = curMonster->cellX;
				bMove.y = curMonster->cellY;
				bMove.targetX = curMonster->targetCellX;
				bMove.targetY = curMonster->targetCellY;
				ss->SendToEveryoneNearBut(0, curMonster->cellX, curMonster->cellY, sizeof(bMove), &bMove);

			}
			break;
        }

        MessMagicAttack messMA;
        messMA.damage = damValue;
        messMA.mobID = (unsigned long) curMonster;
        messMA.avatarID = -1;
        messMA.type = iStaff->type;
        ss->SendToEveryoneNearBut(0, curMonster->cellX, curMonster->cellY, 
                     sizeof(messMA), &messMA,3);

        if (curMonster->health <= 0)
        {
            curMonster->isDead = TRUE;
            curMonster->bane = this;
			if (curMonster->magicEffectAmount[MONSTER_EFFECT_MORE_LOOT]>0.0f)
			{
				sprintf(tempText, "Loot falls from the sky onto the %s's corpse!",
					curMonster->Name());
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
				curMonster->dropAmount = (int)(curMonster->dropAmount*1.1f);
			}

            curMonster->HandleQuestDeath();

            if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
            {
                MapListState oldState = ss->mobList->GetState();

                BBOSArmy *army = (BBOSArmy *)	curMonster->myGenerator;
                army->MonsterEvent(curMonster, ARMY_EVENT_DIED, cellX, cellY);

                ss->mobList->SetState(oldState);
            }
            else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
            {
                MapListState oldState = ss->mobList->GetState();

                BBOSAutoQuest *quest = (BBOSAutoQuest *) curMonster->myGenerator;
                quest->MonsterEvent(curMonster, AUTO_EVENT_DIED, cellX, cellY);

                ss->mobList->SetState(oldState);
            }
        }
        else
        {
            if (!(curMonster->curTarget) || 
                                 (24 == curMonster->type && 3 == (rand() % 10))
             )
            {
                curMonster->lastAttackTime = timeGetTime();
                curMonster->curTarget = this;
            }

            if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
            {
                MapListState oldState = ss->mobList->GetState();

                BBOSArmy *army = (BBOSArmy *)	curMonster->myGenerator;
                army->MonsterEvent(curMonster, ARMY_EVENT_ATTACKED, cellX, cellY);

                ss->mobList->SetState(oldState);
            }
            else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
            {
                MapListState oldState = ss->mobList->GetState();

                BBOSAutoQuest *quest = (BBOSAutoQuest *) curMonster->myGenerator;
                quest->MonsterEvent(curMonster, AUTO_EVENT_ATTACKED, cellX, cellY);

                ss->mobList->SetState(oldState);
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::AttackPlayer(SharedSpace *ss, BBOSAvatar *curAvatar)
{
	bool DidAttackPlayer = FALSE; // similar to attakcking monsters, remove the player target if you fail to attack.
	DWORD delta;
	DWORD now = timeGetTime(); // need this for later
	char tempText[1024]; // these are needed to report back ti the player.
	MessInfoText infoText;
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(socketIndex);
	// we already know pvp is on at this point, but check that the player is in your square.
	if ((curAvatar->cellX == cellX) && (curAvatar->cellY == cellY))
	{
		// then we can attack
		// what is our weapon?
		// find the weapon the avatar is using
		long bladeToHit = -1, bladeDamage;
		int bladePoison, bladeHeal, bladeSlow, bladeBlind, bladeShock, bladeTame;
		bladePoison = bladeHeal = bladeSlow = bladeBlind = bladeShock = bladeTame = 0;

		InvBlade *iBlade = NULL;
		InvStaff *iStaff = NULL;

		InventoryObject *io = (InventoryObject *)
			charInfoArray[curCharacterIndex].wield->objects.First();

		while (io && -1 == bladeToHit && !iStaff)
		{
			if (INVOBJ_BLADE == io->type)
			{
				// if there are multiple swords
				if (io->amount > 1) {
					// create a copy of the sword being used
					InventoryObject *tmp = new InventoryObject(INVOBJ_BLADE, 0, "tmp");
					io->CopyTo(tmp);
					io->amount -= 1;  // decrement the amount

					// tell the new sword it is one object
					tmp->amount = 1;

					// add it to the beginning of the list
					(InventoryObject *)
						charInfoArray[curCharacterIndex].wield->objects.Prepend(tmp);

					// set it as the attacking item
					io = tmp;

				}

				iBlade = (InvBlade *)io->extra;

				bladeToHit = ((InvBlade *)io->extra)->toHit;
				bladeDamage = ((InvBlade *)io->extra)->damageDone;

				bladePoison = ((InvBlade *)io->extra)->poison;
				bladeHeal = ((InvBlade *)io->extra)->heal;
				bladeSlow = ((InvBlade *)io->extra)->slow;
				bladeBlind = ((InvBlade *)io->extra)->blind;
				bladeShock = ((InvBlade *)io->extra)->lightning;
				bladeTame = ((InvBlade *)io->extra)->tame;
			}

			if (INVOBJ_STAFF == io->type)
			{
				iStaff = (InvStaff *)io->extra;
				if (iStaff->type == STAFF_TAMING)
					iStaff = NULL; // do not count taming staffs!
			}
			io = (InventoryObject *)
				charInfoArray[curCharacterIndex].wield->objects.Next();
		}
		InvStaff usedStaff;
		int staffRange;
		usedStaff.charges = -3; // not used;

		// okay we have our weapon data now.
		// until we have the new dust in, we use golds for this too
		// first try a staff if its' the current weapon.
		if (iStaff)
		{
			if (iStaff->isActivated)
			{
				// swing!
				MessAvatarAttack messAA;
				messAA.avatarID = socketIndex;
				messAA.mobID = (long)curAvatar;
				messAA.damage = -2;
				messAA.health = -1000;
				ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);
                // make player aggro on you back.
					curAvatar->lastAttackTime = now;    // set attack time
					curAvatar->curPlayerTarget = this;  // set plaer target to me.

				// do the effect!
				staffRange = StaffAffectsArea(iStaff);
				if (staffRange > 0)  // area?
					usedStaff = *iStaff; // set variable for later code to do the area attack
				else
					UseStaffOnPlayer(ss, iStaff, curAvatar); // thwack the player with it!

				// reduce the charge!
				--(iStaff->charges);
				if (iStaff->charges <= 0)
				{
					// it's gone!

					InventoryObject *io = (InventoryObject *)
						charInfoArray[curCharacterIndex].wield->objects.First();
					while (io)
					{
						if (INVOBJ_STAFF == io->type && iStaff == (InvStaff *)io->extra)
						{
							charInfoArray[curCharacterIndex].wield->objects.Remove(io);
							delete io;
							iStaff = NULL;

							MessUnWield messUnWield;
							messUnWield.bladeID = (long)socketIndex;
							ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY,
								sizeof(messUnWield), (void *)&messUnWield);
						}
						io = (InventoryObject *)
							charInfoArray[curCharacterIndex].wield->objects.Next();
					}

					// wield the next wielded weapon (if you have one)
					MessBladeDesc messBladeDesc;

					io = (InventoryObject *)
						charInfoArray[curCharacterIndex].wield->objects.First();
					while (io)
					{
						if (INVOBJ_BLADE == io->type)
						{

							FillBladeDescMessage(&messBladeDesc, io, this);
							if (isInvisible)
								ss->lserver->SendMsg(sizeof(messBladeDesc), (void *)&messBladeDesc, 0, &tempReceiptList);
							else
								ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY,
									sizeof(messBladeDesc), (void *)&messBladeDesc);

							charInfoArray[curCharacterIndex].wield->objects.Last();
						}
						else if (INVOBJ_STAFF == io->type)
						{
							InvStaff *iStaff = (InvStaff *)io->extra;

							messBladeDesc.bladeID = (long)io;
							messBladeDesc.size = 4;
							messBladeDesc.r = staffColor[iStaff->type][0];
							messBladeDesc.g = staffColor[iStaff->type][1];
							messBladeDesc.b = staffColor[iStaff->type][2];
							messBladeDesc.avatarID = socketIndex;
							messBladeDesc.trailType = 0;
							messBladeDesc.meshType = BLADE_TYPE_STAFF1;
							if (iStaff->type == STAFF_TAMING)
								iStaff = NULL; // ignore taming staff
							else
							{
								// send the equip message
								if (isInvisible)
									ss->lserver->SendMsg(sizeof(messBladeDesc), (void *)&messBladeDesc, 0, &tempReceiptList);
								else
									ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY,
										sizeof(messBladeDesc), (void *)&messBladeDesc);

								charInfoArray[curCharacterIndex].wield->objects.Last();
							}
						}

						io = (InventoryObject *)
							charInfoArray[curCharacterIndex].wield->objects.Next();
					}

				}
			}

		}
		else
		{
			if (-1 == bladeToHit)
			{
				bladeToHit = 1;  // fist is minimal;
				bladeDamage = 1;
			}

			// 1. chance = 2d20 + BladeTame - players' clevel
			int cVal = bladeTame -
				(curAvatar->charInfoArray[curAvatar->curCharacterIndex]).cLevel;
			if (magicEffectAmount[MONSTER_EFFECT_TYPE_WHITE] > 0)                // if we are blinded somehow.
			{
				cVal = cVal - magicEffectAmount[MONSTER_EFFECT_TYPE_WHITE];      // blindness cancels out pvp power.  
			}
			int chance = (rand() % 20) + 2 + (rand() % 20) + cVal;
			if (cVal + 40 <= 20) // if you can't EVER hit, becaue you don't have enough pvp power, or are blinded
			{
				if (6 == (rand() % 400)) // 1-in-400 chance of hitting anyway
					chance = 30;  // Hit!  even if blinded.
			}
			// taming scythes don't work on players so auto hit for them is diked out
			// 2. if chance > 20, hit was successful
			if (chance > 20)
			{
				// 3. damage = Physical * weapon.Damage
				long damValue = (long)(bladeDamage *
					(1 + PhysicalStat() * 0.15f) +
				totemEffects.effect[TOTEM_STRENGTH]);
				AddMastery(ss);	// Add to mastery level
				long additional_stun = 0;


				if (specLevel[0] > 0)
					damValue = (long)(damValue * (1 + 0.05f * specLevel[0]));

				if (iBlade && BLADE_TYPE_CHAOS == iBlade->type)
				{
					if (3 == rand() % 20)
					{
						damValue *= 3;
						// wtf we need to defien tempText
						if (infoFlags & INFO_FLAGS_HITS) {
							sprintf(tempText, "With a burst of strength, you do THREE TIMES as much damage!.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
					else if (2 == rand() % 5)
					{
						damValue *= 2;

						if (infoFlags & INFO_FLAGS_HITS) {
							sprintf(tempText, "With a burst of strength, you do TWICE as much damage!.");
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
				}

				if (iBlade && BLADE_TYPE_CLAWS == iBlade->type)
				{
					if (8 == rand() % 10)
					{
						curAvatar->MagicEffect(MONSTER_EFFECT_STUN, damValue * 250.0f, 2);
						additional_stun = damValue * 50;

						if (infoFlags & INFO_FLAGS_HITS) {
								sprintf(tempText, "%s gets stunned by %s's Claw!", curAvatar->name, charInfoArray[curCharacterIndex].name);
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
				}

				if (iBlade && BLADE_TYPE_DOUBLE == iBlade->type)
				{
					if (1 == rand() % 3)
					{
						DoBladestaffExtraOnPlayer(ss, iBlade, damValue, curAvatar);
					}
				}
				if (iBlade && BLADE_TYPE_DOUBLE < iBlade->type && BLADE_TYPE_STAFF1 > iBlade->type) // for scythe
				{
					if (1 == rand() % 2) //50% chance to strike the square
					{
						if (iBlade->type != BLADE_TYPE_TAME_SCYTHE)              // not a taming scythe
							DoBladestaffExtraOnPlayer(ss, iBlade, (damValue / 2), curAvatar); // do half damage to the rest of the square
					}
				}
				if (iBlade && (iBlade->numOfHits > 0) // since the removal is after this, only do them if numofhits > 1
					&& (iBlade->type > BLADE_TYPE_DOUBLE)  // don't bother with all this if it's not a scythe.
					&& (iBlade->type < BLADE_TYPE_STAFF1))
				{
					// get most common ingot;
					int mostingots = iBlade->tinIngots;
					if (iBlade->aluminumIngots > mostingots)
						mostingots = iBlade->aluminumIngots;
					if (iBlade->steelIngots > mostingots)
						mostingots = iBlade->steelIngots;
					if (iBlade->carbonIngots > mostingots)
						mostingots = iBlade->carbonIngots;
					if (iBlade->zincIngots > mostingots)
						mostingots = iBlade->zincIngots;
					if (iBlade->adamIngots > mostingots)
						mostingots = iBlade->adamIngots;
					if (iBlade->mithIngots > mostingots)
						mostingots = iBlade->mithIngots;
					if (iBlade->vizIngots > mostingots)
						mostingots = iBlade->vizIngots;
					if (iBlade->elatIngots > mostingots)
						mostingots = iBlade->elatIngots;
					if (iBlade->chitinIngots > mostingots)
						mostingots = iBlade->chitinIngots;
					if (iBlade->maligIngots > mostingots)
						mostingots = iBlade->maligIngots;
					if (iBlade->tungstenIngots > mostingots)
						mostingots = iBlade->tungstenIngots;
					if (iBlade->titaniumIngots > mostingots)
						mostingots = iBlade->titaniumIngots;
					if (iBlade->azraelIngots > mostingots)
						mostingots = iBlade->azraelIngots;
					if (iBlade->chromeIngots > mostingots)
							mostingots = iBlade->chromeIngots;
					// wood used for staff determines extra ingots in the base, so we calculate staff power
					int effectVal = 1;  // start at 1
					if (mostingots > 65)
						effectVal++;
					if (mostingots > 67)
						effectVal++;
					if (mostingots > 71)
						effectVal++;
					if (mostingots > 79)
						effectVal++;
					if (mostingots > 95)
						effectVal++;
					// now we have the staff quality equivalent with one added to it.
					// apply staff formula to translate that to actual staff power
					effectVal = 5 * effectVal + 3; // don't worry about imbue deviation, only perfect staffs will add charges.
												   // now that we know the staff equivalent, we can apply the staff effect
					if (iBlade->type == BLADE_TYPE_BLIND_SCYTHE) // if it can blind
					{ // do staff blind effect
						curAvatar->MagicEffect(DRAGON_TYPE_WHITE,
							effectVal * 1000, effectVal);
					}
					if (iBlade->type == BLADE_TYPE_POISON_SCYTHE) // if it can poison
					{ // do staff poison effect
						curAvatar->MagicEffect(DRAGON_TYPE_BLACK,
							effectVal * 1000, effectVal);
					}
					if (iBlade->type == BLADE_TYPE_SLOW_SCYTHE) // if it can slow
					{ // do staff slow effect
						curAvatar->MagicEffect(DRAGON_TYPE_BLUE,
							effectVal * 1000, effectVal);
					}
					if (iBlade->type == BLADE_TYPE_STUN_SCYTHE) // if it can stun
					{ // do staff stun effect
						curAvatar->MagicEffect(MONSTER_EFFECT_STUN,
							effectVal * 1000, effectVal);
					}
					if (iBlade->type == BLADE_TYPE_TANGLE_SCYTHE) // if it can root
					{ // do staff root effect
						curAvatar->MagicEffect(MONSTER_EFFECT_BIND,
							effectVal * 1000, effectVal);
					}

					if ((iBlade->type == BLADE_TYPE_TAME_SCYTHE)) // if you aren't actually a beastmaster
					{
							sprintf(tempText, "This doesn't work on players.");

							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

							iBlade->numOfHits++; // refund charge
					}
				}

				if (totemEffects.effect[TOTEM_LIFESTEAL] > 0)
				{
						int suck = damValue * totemEffects.effect[TOTEM_LIFESTEAL] / 40;
						if (suck > charInfoArray[curCharacterIndex].healthMax / 4)
							suck = charInfoArray[curCharacterIndex].healthMax / 4;
						damValue += suck;

						charInfoArray[curCharacterIndex].health += suck;

						if (charInfoArray[curCharacterIndex].health >
							charInfoArray[curCharacterIndex].healthMax)
							charInfoArray[curCharacterIndex].health =
							charInfoArray[curCharacterIndex].healthMax;
					}

					// Add mastery damage
					if (iBlade)
						damValue += (GetMasteryForType(iBlade->type) * charInfoArray[curCharacterIndex].physical / 2);
					// cap actual done damage for taming scythe
					if (iBlade && (iBlade->type == BLADE_TYPE_TAME_SCYTHE) && damValue > 1)
						damValue = 1;  // needs to not kill stuff.

					curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= damValue;

					// get dust total to check if shard elibible
					int dustTotal = 0;
					if (iBlade)
					{
						dustTotal += iBlade->blind;
						dustTotal += iBlade->slow;
						dustTotal += iBlade->heal;
						dustTotal += iBlade->poison;
						dustTotal += iBlade->tame;
						int blueshack = iBlade->toHit - 1;
						if (iBlade->type == BLADE_TYPE_KATANA)
							blueshack -= 20;
						dustTotal += blueshack;  // don't penalize total dust count for not using a tachi.
					}
					// age the blade
					if (iBlade)
					{
						if (iBlade->numOfHits < 259501) //
							++(iBlade->numOfHits);
					}
					// revert age if it can't have a shard and not cheating.
					if (iBlade && iBlade->numOfHits > 22000)
					{
						if ((dustTotal < 5) || (iBlade->damageDone < 72))
						{
							iBlade->numOfHits = 22000;
						}
					}
					// undo and subtract for a scythe.
					if (iBlade &&  BLADE_TYPE_DOUBLE < iBlade->type && BLADE_TYPE_STAFF1 > iBlade->type)
					{
						--(iBlade->numOfHits);
						--(iBlade->numOfHits);
						if (iBlade->numOfHits < 0) // must have been zero before the swing, and one after.
							++(iBlade->numOfHits); // so it's -1, increase to 0.
					}
					if (iBlade && iBlade->type < BLADE_TYPE_SCYTHE // it's not a scythe
						&& ((iBlade->type == BLADE_TYPE_MACE && iBlade->damageDone > 101) // it's a mace with dam 102 or greater
							|| iBlade->damageDone > 71))// or somethtign else with dam 72 or greater
					{
						// then count the dusts
						int dustTotal = 0;
						dustTotal += iBlade->blind;
						dustTotal += iBlade->slow;
						dustTotal += iBlade->heal;
						dustTotal += iBlade->poison;
						dustTotal += iBlade->tame;
						int blueshack = iBlade->toHit - 1;
						if (iBlade->type == BLADE_TYPE_KATANA)
							blueshack -= 20;
						dustTotal += blueshack;  // don't penalize total dust count for not using a tachi.
						if (dustTotal > 4) // if it has enough dusts on it
						{
							// then it could ahve a shard chance, check for particle effect
							int doparticle = 0;
							if (iBlade->numOfHits > 22000 && ((iBlade->numOfHits - 22000) % 2500 == 0)) // particle for every percentage added
								doparticle = 1;
							if (iBlade->numOfHits == 22000)												// initial partical
								doparticle = 2;
							if (iBlade->numOfHits == 110000)											// old cap for normal
								doparticle = 3;
							if (iBlade->numOfHits == 259500)											// 100%
								doparticle = 10;
							if (doparticle > 0)
							{
								// pretty particle effect!
								BBOSGroundEffect *bboGE = new BBOSGroundEffect();
								bboGE->type = 4;
								bboGE->amount = 20 * doparticle;
								bboGE->r = 20;
								bboGE->g = 60;
								bboGE->b = 128;
								BBOSAvatar * curAvatar = this; // reference myself
								bboGE->cellX = curAvatar->cellX;
								bboGE->cellY = curAvatar->cellY;
								bboGE->killme = 400; // make it go away after a small bit.
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
									curAvatar->cellX, curAvatar->cellY,
									sizeof(messGE), &messGE);
							} // whew!
						}
					}
					// give experience for monster damage!

//                        if (bladeHeal > 0)
//                        {
//                            // green dust heals the wielder.. if it wasn't disabled.
//                            charInfoArray[curCharacterIndex].health += bladeHeal;
//
//                            if (charInfoArray[curCharacterIndex].health >
//                                        charInfoArray[curCharacterIndex].healthMax)
//                            {
//                                charInfoArray[curCharacterIndex].health =
//                                        charInfoArray[curCharacterIndex].healthMax;
//                            }
//                        }
					if (iBlade && (iBlade->type != BLADE_TYPE_TAME_SCYTHE)) // if not a scythe
					{
						/// check for shock.
						if (bladeShock > 0) {
							int cnt = iBlade->GetIngotCount(); // safe because bladeshock can't be greater than 0 if iblade doesn't exist.

							if (cnt > 96)
								cnt = 96;

							if (rand() % 144 < cnt && (bladeShock / 2)) {
								int resisted = 0; // no resistance for players yet

								curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= (bladeShock / 2);

								if (infoFlags & INFO_FLAGS_HITS) {
										sprintf(tempText, "%s gets shocked by %d lightning damage and is stunned.",
											curAvatar->charInfoArray[curAvatar->curCharacterIndex].name, (bladeShock / 2));

									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								}
							}
						}
					}

					if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health <= 0)
					{
						// no meat drop chance
						MessAvatarAttack messAA;
						messAA.avatarID = socketIndex;
						messAA.mobID = (long)curAvatar;
						messAA.damage = damValue;
						messAA.health = -1000;
						ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);

						curAvatar->isDead = TRUE;

						// no greens for loot.
						
						// now quest death for players

						// not a generator
					}
					else
					{
						if (!(curAvatar->curPlayerTarget))
						{
							curAvatar->lastAttackTime = now;
							curAvatar->curPlayerTarget = this;
						}
						// no gerneator stuff
						MessAvatarAttack messAA;
						messAA.avatarID = socketIndex;
						messAA.mobID = (long)curAvatar;
						messAA.damage = damValue;
						messAA.health = charInfoArray[curCharacterIndex].health;
						messAA.healthMax = charInfoArray[curCharacterIndex].healthMax;
						ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);

						MessMonsterHealth messMH;
						messMH.mobID = (unsigned long)curAvatar;
						messMH.health = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
						messMH.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
						ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messMH), (void *)&messMH, 2);
						// apply dusts
						if (iBlade && (iBlade->type != BLADE_TYPE_TAME_SCYTHE) && (bladeSlow > 0))
						{
							// blue dragons slow the target.
							int chance = bladeSlow;

							if (chance + (rand() % 20) - curAvatar->charInfoArray[curAvatar->curCharacterIndex].cLevel > 10)
							{
								curAvatar->MagicEffect(DRAGON_TYPE_BLUE, bladeSlow * 1000.0f, bladeSlow);

								if (infoFlags & INFO_FLAGS_HITS)
								{
										sprintf(tempText, "%s is Slowed.",
											curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);
									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								}
							}
						}
						if (bladeHeal > 0)
						{
							// green dusts can cause the monster to drop more simple loot.  no effect on players
						}


						if (bladePoison > 0)
						{  
							// no effect on players
						}
						if (iBlade && (iBlade->type != BLADE_TYPE_TAME_SCYTHE) && (bladeBlind > 0))
						{
							int chance = bladeBlind;

							if (chance + (rand() % 20) - curAvatar->charInfoArray[curAvatar->curCharacterIndex].cLevel > 10)
							{
								curAvatar->MagicEffect(DRAGON_TYPE_WHITE, bladeBlind * 1000.0f, bladeBlind);

								if (infoFlags & INFO_FLAGS_HITS)
								{
										sprintf(tempText, "%s is Blinded.",
											curAvatar->charInfoArray[curAvatar->curCharacterIndex].name);

									CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
									ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
								}
							}
						}
					}

				}
				else
				{
					// whish!
					MessAvatarAttack messAA;
					messAA.avatarID = socketIndex;
					messAA.mobID = (long)curAvatar;
					messAA.damage = -1; // miss!
					messAA.health = charInfoArray[curCharacterIndex].health;
					messAA.healthMax = charInfoArray[curCharacterIndex].healthMax;
					ss->SendToEveryoneNearBut(0, (float)cellX, (float)cellY, sizeof(messAA), &messAA, 2);
				}
		}


	}

	// lets you use your wielded weapon on a player
	// you will need new dusts equal to the players' clevel to hit for fighter weapons.
	// if you can hit, they will deal full damage, and dust effects are applied separately, with their own identical rolls to hit. if you can't miss, you will apply dust.
	// lifesteal also applies.
	// staffs don't need this, but you will HAVE to be a mage to use them in PVP (no fighters stunning players with a staff and murdering them.)
	// bombing other players is handled elsewhere, and restricted to smiths.
	// beastmasters will use their beasts instead.
	// players currently have no magic resistance stat.  this may be changed later. possibly a new totem formula?
	// to be written: Unique loot from killing players (player heads :) ) You won't be able to lose your stuff, becasue there's no death penalty in BM ever.
	// client code to allow the attack command to show depending on pvp flag, and maybe a new packet will be needed to send the attack command to the server.
}
//******************************************************************
void BBOSAvatar::UseStaffOnPlayer(SharedSpace *ss, InvStaff *iStaff, BBOSAvatar *curAvatar)
{
	char tempText[1024];
	MessInfoText infoText;
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(socketIndex);
	MessMonsterHealth messMH;

	// 1. chance = 30, staffs cant' misss
	int chance = 30;  // Hit!


	if (chance > 20)



		// always hit!
	{
		if ((curAvatar->BeastStat()>9) && curAvatar->controlledMonster  // beastmaster that has a pet out
			&& (curAvatar->controlledMonster->cellX==curAvatar->cellX) && (curAvatar->controlledMonster->cellY == curAvatar->cellY)) // and the pet is in the same square.
			
		{
			UseStaffOnMonster(ss, iStaff, curAvatar->controlledMonster); // use it on the pet instead. no need to refund the charge.
			return;
		}
		int effectVal = 3 + iStaff->quality * 5 - iStaff->imbueDeviation;
		if (effectVal < 0)
			effectVal = 0;
		long damValue = 0;

		if (specLevel[1] > 0)
			effectVal += specLevel[1] * 2;

		switch (iStaff->type)
		{
		case STAFF_DAMAGE:
		case STAFF_AREA_DAMAGE:

			// 3. damage = Physical * weapon.Damage
			damValue = (iStaff->quality + 1) * (10 - iStaff->imbueDeviation) *
				(1.0f + MagicalStat() * 0.5f);

			if (StaffAffectsArea(iStaff))
				damValue = (long)(damValue * 0.6f);

			if (specLevel[1] > 0)
				damValue = damValue * (1.0f + 0.05f * specLevel[1]);
			// this is gonna suck
			curAvatar->charInfoArray[curAvatar->curCharacterIndex].health -= damValue;
			

			messMH.mobID = (unsigned long)curAvatar;
			messMH.health = curAvatar->charInfoArray[curAvatar->curCharacterIndex].health;
			messMH.healthMax = curAvatar->charInfoArray[curAvatar->curCharacterIndex].healthMax;
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH), (void *)&messMH, 2);

			break;

		case STAFF_BLIND:
		case STAFF_AREA_BLIND:
			curAvatar->MagicEffect(DRAGON_TYPE_WHITE,
				effectVal * 1000+timeGetTime(), effectVal);

			break;

		case STAFF_SLOW:
		case STAFF_AREA_SLOW:
			curAvatar->MagicEffect(DRAGON_TYPE_BLUE,
				effectVal * 1000 + timeGetTime(), effectVal);

			break;

		case STAFF_POISON:
		case STAFF_AREA_POISON:
			curAvatar->MagicEffect(DRAGON_TYPE_BLACK,
				effectVal * 1000 + timeGetTime(), effectVal);

			break;

		case STAFF_STUN:
		case STAFF_AREA_STUN:
			curAvatar->MagicEffect(MONSTER_EFFECT_STUN,
				effectVal * 1000 + timeGetTime(), effectVal);

			break;

		case STAFF_BIND:
		case STAFF_AREA_BIND:
			curAvatar->MagicEffect(MONSTER_EFFECT_BIND,
				effectVal * 1000 + timeGetTime(), effectVal);

			break;
		case STAFF_AREA_TAUNT: // seriously?
			break;             // does nothing when used on a player
		}

		MessMagicAttack messMA;
		messMA.damage = damValue;
		messMA.mobID = (unsigned long)curAvatar;
		messMA.avatarID = -1;
		messMA.type = iStaff->type;
		ss->SendToEveryoneNearBut(0, curAvatar->cellX, curAvatar->cellY,
			sizeof(messMA), &messMA, 3);

		if (curAvatar->charInfoArray[curAvatar->curCharacterIndex].health <= 0)
		{
			curAvatar->isDead = TRUE;
		}
		else
		{
			if (!(curAvatar->curPlayerTarget) && !(curAvatar->curTarget)) // my target not attacking something already
			{
				curAvatar->lastAttackTime = timeGetTime(); // she will hit me back.
				curAvatar->curPlayerTarget = this;
			}

		}
	}
}

//******************************************************************
void BBOSAvatar::DoBladestaffExtra(SharedSpace *ss, InvBlade *ib,
	long damValue, BBOSMonster *targetMonster)
{
	char tempText[1024];
	MessInfoText infoText;
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(socketIndex);

	MessMonsterHealth messMH;
	BBOSMob *curMob = NULL;

	MapListState oldState2 = ss->mobList->GetState();

	unsigned short bladePoison, bladeSlow, bladeBlind, bladeHeal;
	bladePoison = ib->poison;
	bladeSlow = ib->slow;
	bladeBlind = ib->blind;
	bladeHeal = ib->heal;

	curMob = ss->mobList->GetFirst(cellX, cellY);
	while (curMob)
	{
		BBOSMonster *curMonster = (BBOSMonster *)curMob;

		if (SMOB_MONSTER == curMob->WhatAmI() && targetMonster != curMonster // it's a mosnter, and its' NOT the one you already hit
			&& (curMonster != controlledMonster)								// and it's NOT your pet.
			&& (!curMonster->controllingAvatar || (curMonster->controllingAvatar->BeastStat() < 10))) // and it has no controller OR its' controller is not a beastmaster (safe due to short circuit) 
		{

			curMonster->health -= damValue;
			curMonster->RecordDamageForLootDist(damValue, this);

			// give experience for monster damage!

			MessMonsterHealth messMH;
			messMH.mobID = (unsigned long)curMonster;
			messMH.health = curMonster->health;
			messMH.healthMax = curMonster->maxHealth;
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH), (void *)&messMH, 2);

			if (curMonster->health <= 0)
			{
				curMonster->isDead = TRUE;
				curMonster->bane = this;
				if (ib->type > BLADE_TYPE_DOUBLE)  // if killed by a scythe
					curMonster->MoreMeat = true;   // give more meat
				if (curMonster->magicEffectAmount[MONSTER_EFFECT_MORE_LOOT] > 0.0f)
				{
					sprintf(tempText, "Loot falls from the sky onto the %s's corpse!",
						curMonster->Name());
					CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
					ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					curMonster->dropAmount = (int)(curMonster->dropAmount*1.1f);
				}

				curMonster->HandleQuestDeath();

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
				// give experience for monster death!
			}
			else
			{
				if (bladeSlow > 0)
				{
					int chance = bladeSlow;

					if (chance + (rand() % 20) - curMonster->defense > 10)
					{
						int resisted = curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE,
							bladeSlow * 1000, bladeSlow);

						if (infoFlags & INFO_FLAGS_HITS)
						{
							if (resisted == 1)
								sprintf(tempText, "The %s resisted your slowing effect!",
									curMonster->Name());
							else
								sprintf(tempText, "The %s is Slowed.",
									curMonster->Name());
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
				}
				if (bladePoison > 0)
				{
					int chance = bladePoison;

					//                    if (chance + (rand() % 20) - curMonster->defense > 10)
					//                    {
					curMonster->MonsterMagicEffect(MONSTER_EFFECT_RESIST_LOWER,
						bladePoison * 1000, bladePoison);

					curMonster->RecordDamageForLootDist(bladePoison * bladePoison / 40, this);

					if (infoFlags & INFO_FLAGS_HITS)
					{
						sprintf(tempText, "The %s is more vulnerable to other dusts.",
							curMonster->Name());
						CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
						ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
					}
					//                   }
				}
				if (bladeBlind > 0)
				{
					int chance = bladeBlind;

					if (chance + (rand() % 20) - curMonster->defense > 10)
					{
						int resisted = curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE,
							bladeBlind * 1000, bladeBlind);

						if (infoFlags & INFO_FLAGS_HITS)
						{
							if (resisted == 1)
								sprintf(tempText, "The %s resisted your blinding effect.",
									curMonster->Name());
							else
								sprintf(tempText, "The %s is Blinded.",
									curMonster->Name());

							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
				}
				if (bladeHeal > 0)
				{
					// green dusts can cause the monster to drop more simple loot.
					int chance = bladeHeal;

					if (chance + (rand() % 20) - curMonster->defense > 10)
					{
						curMonster->MonsterMagicEffect(MONSTER_EFFECT_MORE_LOOT,
							bladeHeal * 1000.0f, bladeHeal);

					}
				}
				// staff effect checks
				if ((ib->numOfHits > 0) // since the removal is after this, only do them if numofhits > 1
					&& (ib->type > BLADE_TYPE_DOUBLE)  // don't bother with all this if it's not a scythe.
					&& (ib->type < BLADE_TYPE_STAFF1))
				{
					// get most common ingot;
					int mostingots = ib->tinIngots;
					if (ib->aluminumIngots > mostingots)
						mostingots = ib->aluminumIngots;
					if (ib->steelIngots > mostingots)
						mostingots = ib->steelIngots;
					if (ib->carbonIngots > mostingots)
						mostingots = ib->carbonIngots;
					if (ib->zincIngots > mostingots)
						mostingots = ib->zincIngots;
					if (ib->adamIngots > mostingots)
						mostingots = ib->adamIngots;
					if (ib->mithIngots > mostingots)
						mostingots = ib->mithIngots;
					if (ib->vizIngots > mostingots)
						mostingots = ib->vizIngots;
					if (ib->elatIngots > mostingots)
						mostingots = ib->elatIngots;
					if (ib->chitinIngots > mostingots)
						mostingots = ib->chitinIngots;
					if (ib->maligIngots > mostingots)
						mostingots = ib->maligIngots;
					if (ib->tungstenIngots > mostingots)
						mostingots = ib->tungstenIngots;
					if (ib->titaniumIngots > mostingots)
						mostingots = ib->titaniumIngots;
					if (ib->azraelIngots > mostingots)
						mostingots = ib->azraelIngots;
					if (ib->chromeIngots > mostingots)
						mostingots = ib->chromeIngots;
					// wood used for staff determines extra ingots in the base, so we calculate staff power
					int effectVal = 1;  // start at 1
					if (mostingots > 65)
						effectVal++;
					if (mostingots > 67)
						effectVal++;
					if (mostingots > 71)
						effectVal++;
					if (mostingots > 79)
						effectVal++;
					if (mostingots > 95)
						effectVal++;
					// now we have the staff quality equivalent with one added to it.
					// apply staff formula to translate that to actual staff power
					effectVal = 5 * effectVal + 3; // don't worry about imbue deviation, only perfect staffs will add charges.
					// now that we know the staff equivalent, we can apply the staff effect
					if (ib->type == BLADE_TYPE_BLIND_SCYTHE) // if it can blind
					{ // do staff blind effect
						curMonster->MonsterMagicEffect(DRAGON_TYPE_WHITE,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_POISON_SCYTHE) // if it can poison
					{ // do staff poison effect
						curMonster->MonsterMagicEffect(DRAGON_TYPE_BLACK,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_SLOW_SCYTHE) // if it can slow
					{ // do staff slow effect
						curMonster->MonsterMagicEffect(DRAGON_TYPE_BLUE,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_STUN_SCYTHE) // if it can stun
					{ // do staff stun effect
						curMonster->MonsterMagicEffect(MONSTER_EFFECT_STUN,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_TANGLE_SCYTHE) // if it can root
					{ // do staff root effect
						curMonster->MonsterMagicEffect(MONSTER_EFFECT_BIND,
							effectVal * 1000, effectVal);
					}
				}

				if (!(curMonster->curTarget) ||
					(24 == curMonster->type && 3 == (rand() % 10))
					)
				{
					curMonster->lastAttackTime = timeGetTime();
					curMonster->curTarget = this;
				}

				if (curMonster->myGenerator && 1 == curMonster->myGenerator->WhatAmI())
				{
					MapListState oldState = ss->mobList->GetState();

					BBOSArmy *army = (BBOSArmy *)curMonster->myGenerator;
					army->MonsterEvent(curMonster, ARMY_EVENT_ATTACKED);

					ss->mobList->SetState(oldState);
				}
				else if (curMonster->myGenerator && 2 == curMonster->myGenerator->WhatAmI())
				{
					MapListState oldState = ss->mobList->GetState();

					BBOSAutoQuest *quest = (BBOSAutoQuest *)curMonster->myGenerator;
					quest->MonsterEvent(curMonster, AUTO_EVENT_ATTACKED);

					ss->mobList->SetState(oldState);
				}
			}
		}

		curMob = ss->mobList->GetNext();
	}
	// and, if allowed to be crafted, the spirit summon effect. do thsi after goign through all of the nonsters.
	// staf effect strength doesnt' matter, so we dont need to look it up again.  we assume 
	if (ib->type == BLADE_TYPE_SUMMON_SCYTHE)
	{
		DungeonMap * dm = (DungeonMap *)ss; // define the dungeon map, 
		BBOSMob * curMob2 = ss->mobList->GetFirst(cellX, cellY, 2);
		while (curMob2)
		{
			BBOSMonster * curMonster = (BBOSMonster *)curMob2;

			if (SMOB_MONSTER == curMob2->WhatAmI() &&
				abs(cellX - curMob2->cellX) <= 2 &&
				abs(cellY - curMob2->cellY) <= 2)
				// don't need to check deviation, becasue  it was made with perfect staffs
				if ((ss->WhatAmI() == SPACE_DUNGEON) // in a dungeon
					&& (dm->specialFlags & SPECIAL_DUNGEON_TEMPORARY) // and it's a geo (safe because of short circuit)
					&& (curMonster->type > 8) // from sprit vision
					&& (curMonster->type < 18) // to spirit spider
					&& (curMonster->subType < 1) // not a cent or dokk.
					&& ((this->cellX != curMonster->cellX)
						|| (this->cellY != curMonster->cellY))) // is not already in my square
				{
					curMonster->targetCellX = this->cellX; //tell the monster to try move to my square 
					curMonster->targetCellY = this->cellY;
					curMonster->isMoving = TRUE;  // and make it actually move
					curMonster->moveStartTime = timeGetTime();

					MessMobBeginMove bMove;
					bMove.mobID = (unsigned long)curMonster;
					bMove.x = curMonster->cellX;
					bMove.y = curMonster->cellY;
					bMove.targetX = curMonster->targetCellX;
					bMove.targetY = curMonster->targetCellY;
					ss->SendToEveryoneNearBut(0, curMonster->cellX, curMonster->cellY, sizeof(bMove), &bMove);

				}

			curMob = ss->mobList->GetNext();
		}
	}

	ss->mobList->SetState(oldState2);
}
//******************************************************************
void BBOSAvatar::DoBladestaffExtraOnPlayer(SharedSpace *ss, InvBlade *ib,
	long damValue, BBOSAvatar *targetPlayer)
{
	char tempText[1024];
	MessInfoText infoText;
	std::vector<TagID> tempReceiptList;
	tempReceiptList.clear();
	tempReceiptList.push_back(socketIndex);

	MessMonsterHealth messMH;
	BBOSMob *curMob = NULL;

	MapListState oldState2 = ss->mobList->GetState();

	unsigned short bladePoison, bladeSlow, bladeBlind, bladeHeal;
	bladePoison = ib->poison;
	bladeSlow = ib->slow;
	bladeBlind = ib->blind;
	bladeHeal = ib->heal;
	curMob = (BBOSMob *)ss->avatars->First();
	while (curMob)
	{
		BBOSAvatar *curPlayer = (BBOSAvatar *)curMob; // sould be safe, because this is the avatar list.
		if ((SMOB_AVATAR == curMob->WhatAmI()) && (targetPlayer != curPlayer) // it's a player, and its' NOT the one you already hit			
			&& (cellX == curPlayer->cellX) && (cellY == curPlayer->cellY)) /// and in your square
		{

			curPlayer->charInfoArray[curPlayer->curCharacterIndex].health -= damValue;
			
			// give experience for monster damage!
			
			MessMonsterHealth messMH;
			messMH.mobID = (unsigned long)curPlayer;
			messMH.health = curPlayer->charInfoArray[curPlayer->curCharacterIndex].health;
			messMH.healthMax = curPlayer->charInfoArray[curPlayer->curCharacterIndex].healthMax;
			ss->SendToEveryoneNearBut(0, cellX, cellY, sizeof(messMH), (void *)&messMH, 2);

			if (curPlayer->charInfoArray[curPlayer->curCharacterIndex].health <= 0)
			{
				curPlayer->isDead = TRUE;
				// stuff related to monsters ignored here.
			}
			else
			{
				// not dead yet, we can apply dusts
				// but dusts aren't automatic, you still need enough to apply the effect.
				// we need to get the clvl
				int dustdefense = curPlayer->charInfoArray[curPlayer->curCharacterIndex].cLevel;
				// players can be slowed.
				if (bladeSlow > 0)
				{
					int chance = bladeSlow-dustdefense; // you need more dusts than clvl to tack on the dust effect for your cheated hit that bypassed clvl
					
					if (chance >= 0) // got enough dusts?
					{
						curPlayer->MagicEffect(DRAGON_TYPE_BLUE,	bladeSlow * 1000, bladeSlow); // apply the effect
						// player don't have dust resistance.  oh, the PAIN!
						if (infoFlags & INFO_FLAGS_HITS)
						{
								sprintf(tempText, "%s is Slowed.", curPlayer->name);
							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
				}
				// since players don't have reistance, don't bother checkig the black dust.
				// but they can be blinded.
				if (bladeBlind > 0)
				{
					int chance = bladeBlind-dustdefense;

					if (chance >= 0)  // got enough dusts?
					{
						curPlayer->MagicEffect(DRAGON_TYPE_WHITE, bladeBlind * 1000, bladeBlind);

						if (infoFlags & INFO_FLAGS_HITS)
						{
								sprintf(tempText, "%s is Blinded.",
									curPlayer->name);

							CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
							ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
						}
					}
				}
				// greens don't matter on players either. that's removed
				// and silvers don't spread to the square with monsters either.
				// now we check for the scythes that arent' even in game anymore.
				// staff effect checks
				if ((ib->numOfHits > 0) // since the removal is after this, only do them if numofhits > 1
					&& (ib->type > BLADE_TYPE_DOUBLE)  // don't bother with all this if it's not a scythe.
					&& (ib->type < BLADE_TYPE_STAFF1))
				{
					// get most common ingot;
					int mostingots = ib->tinIngots;
					if (ib->aluminumIngots > mostingots)
						mostingots = ib->aluminumIngots;
					if (ib->steelIngots > mostingots)
						mostingots = ib->steelIngots;
					if (ib->carbonIngots > mostingots)
						mostingots = ib->carbonIngots;
					if (ib->zincIngots > mostingots)
						mostingots = ib->zincIngots;
					if (ib->adamIngots > mostingots)
						mostingots = ib->adamIngots;
					if (ib->mithIngots > mostingots)
						mostingots = ib->mithIngots;
					if (ib->vizIngots > mostingots)
						mostingots = ib->vizIngots;
					if (ib->elatIngots > mostingots)
						mostingots = ib->elatIngots;
					if (ib->chitinIngots > mostingots)
						mostingots = ib->chitinIngots;
					if (ib->maligIngots > mostingots)
						mostingots = ib->maligIngots;
					if (ib->tungstenIngots > mostingots)
						mostingots = ib->tungstenIngots;
					if (ib->titaniumIngots > mostingots)
						mostingots = ib->titaniumIngots;
					if (ib->azraelIngots > mostingots)
						mostingots = ib->azraelIngots;
					if (ib->chromeIngots > mostingots)
						mostingots = ib->chromeIngots;
					// wood used for staff determines extra ingots in the base, so we calculate staff power
					int effectVal = 1;  // start at 1
					if (mostingots > 65)
						effectVal++;
					if (mostingots > 67)
						effectVal++;
					if (mostingots > 71)
						effectVal++;
					if (mostingots > 79)
						effectVal++;
					if (mostingots > 95)
						effectVal++;
					// now we have the staff quality equivalent with one added to it.
					// apply staff formula to translate that to actual staff power
					effectVal = 5 * effectVal + 3; // don't worry about imbue deviation, only perfect staffs will add charges.
					// now that we know the staff equivalent, we can apply the staff effect
					if (ib->type == BLADE_TYPE_BLIND_SCYTHE) // if it can blind
					{ // do staff blind effect
						curPlayer->MagicEffect(DRAGON_TYPE_WHITE,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_POISON_SCYTHE) // if it can poison
					{ // do staff poison effect
						curPlayer->MagicEffect(DRAGON_TYPE_BLACK,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_SLOW_SCYTHE) // if it can slow
					{ // do staff slow effect
						curPlayer->MagicEffect(DRAGON_TYPE_BLUE,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_STUN_SCYTHE) // if it can stun
					{ // do staff stun effect
						curPlayer->MagicEffect(MONSTER_EFFECT_STUN,
							effectVal * 1000, effectVal);
					}
					if (ib->type == BLADE_TYPE_TANGLE_SCYTHE) // if it can root
					{ // do staff root effect
						curPlayer->MagicEffect(MONSTER_EFFECT_BIND,
							effectVal * 1000, effectVal);
					}
				}

				if (!(curPlayer->curPlayerTarget)) // player not already attacking a player?
				{
					curPlayer->lastAttackTime = timeGetTime();
					curPlayer->curPlayerTarget = this;  // player will hit you back.
				}

			}
		}

		curMob = ss->mobList->GetNext();
	}
	// spirit summoning scythe does nothing to players, so that's removed.
}

//******************************************************************
void BBOSAvatar::CompareWith(BBOSAvatar *other, int &myRelationship, int &otherRelationship)
{

    int meVal = 0;
    int otherVal = 0;

    // lifeTime is in 5 minute increments

    if (charInfoArray[curCharacterIndex].lifeTime > 20)
        ++meVal;
    if (charInfoArray[curCharacterIndex].lifeTime > 100)
        ++meVal;
    if (charInfoArray[curCharacterIndex].lifeTime > 200)
        ++meVal;
    if (charInfoArray[curCharacterIndex].lifeTime > 1000)
        ++meVal;
    if (charInfoArray[curCharacterIndex].lifeTime > 3000)
        ++meVal;

    if (other->charInfoArray[other->curCharacterIndex].lifeTime > 20)
        ++otherVal;
    if (other->charInfoArray[other->curCharacterIndex].lifeTime > 100)
        ++otherVal;
    if (other->charInfoArray[other->curCharacterIndex].lifeTime > 200)
        ++otherVal;
    if (other->charInfoArray[other->curCharacterIndex].lifeTime > 1000)
        ++otherVal;
    if (other->charInfoArray[other->curCharacterIndex].lifeTime > 3000)
        ++otherVal;

    if (meVal == otherVal)
    {
        myRelationship    = SAMARITAN_REL_PEER;
        otherRelationship = SAMARITAN_REL_PEER;
        return;
    }
    else if (meVal > otherVal)
    {
        myRelationship    = SAMARITAN_REL_OLDER;
        otherRelationship = SAMARITAN_REL_YOUNGER;
        return;
    }

    myRelationship    = SAMARITAN_REL_YOUNGER;
    otherRelationship = SAMARITAN_REL_OLDER;

}

//******************************************************************
int BBOSAvatar::IsAGuildMate(BBOSAvatar *other)
{

    SharedSpace *sx = NULL, *sx2 = NULL;

    bboServer->FindAvatarInGuild(charInfoArray[curCharacterIndex].name, &sx);

    bboServer->FindAvatarInGuild(other->charInfoArray[other->curCharacterIndex].name, &sx2);

    if (sx != NULL && sx2 == sx)
        return TRUE;
    return FALSE;
    
    
}
    
//******************************************************************
void BBOSAvatar::LogKarmaExchange(BBOSAvatar *receiver, 
                                             int myRel, int receiverRel, int exchangeType,
                                             char *originalText)
{
    char tempText[1024];
    LongTime lt;

    sprintf(tempText,"%d/%02d, %d:%02d,    ", (int)lt.value.wMonth, (int)lt.value.wDay, 
              (int)lt.value.wHour, (int)lt.value.wMinute);
    LogOutput("karma.txt", tempText);

    sprintf(tempText,"%s, ", charInfoArray[curCharacterIndex].name);
    LogOutput("karma.txt", tempText);

    switch(myRel)
    {
    case SAMARITAN_REL_OLDER:
        LogOutput("karma.txt", "ELDER, ");
        break;

    case SAMARITAN_REL_PEER:
        LogOutput("karma.txt", "PEER, ");
        break;

    case SAMARITAN_REL_YOUNGER:
        LogOutput("karma.txt", "YOUNGER, ");
        break;

    case SAMARITAN_REL_GUILD:
        LogOutput("karma.txt", "GUILDMATE, ");
        break;

    default:
        LogOutput("karma.txt", "NO_RELATIONSHIP, ");
        break;
    }

    switch(exchangeType)
    {
    case SAMARITAN_TYPE_THANKS:
        LogOutput("karma.txt", "SAYS_THANKS_TO, ");
        break;

    case SAMARITAN_TYPE_PLEASE:
        LogOutput("karma.txt", "SAYS_PLEASE_TO, ");
        break;

    case SAMARITAN_TYPE_WELCOME:
        LogOutput("karma.txt", "SAYS_WELCOME_TO, ");
        break;

    case SAMARITAN_TYPE_GIFT:
        LogOutput("karma.txt", "GIVES_SECURE_GIFT_TO, ");
        break;

    case SAMARITAN_TYPE_CASH:
        LogOutput("karma.txt", "GIVES_CASH_TO, ");
        break;

    default:
        LogOutput("karma.txt", "NO_ACTION, ");
        break;
    }


    sprintf(tempText,"%s, ", receiver->charInfoArray[receiver->curCharacterIndex].name);
    LogOutput("karma.txt", tempText);

    switch(receiverRel)
    {
    case SAMARITAN_REL_OLDER:
        LogOutput("karma.txt", "ELDER, ");
        break;

    case SAMARITAN_REL_PEER:
        LogOutput("karma.txt", "PEER, ");
        break;

    case SAMARITAN_REL_YOUNGER:
        LogOutput("karma.txt", "YOUNGER, ");
        break;

    case SAMARITAN_REL_GUILD:
        LogOutput("karma.txt", "GUILDMATE, ");
        break;

    default:
        LogOutput("karma.txt", "NO_RELATIONSHIP, ");
        break;
    }

    if (originalText)
    {
        sprintf(tempText, originalText);
        RemoveCommasAndReturnsFromString(tempText);
        LogOutput("karma.txt", tempText);
    }
    else
        LogOutput("karma.txt", "NO_TEXT");

    LogOutput("karma.txt", "\n");
}
    
//******************************************************************
float BBOSAvatar::BestSwordRating(void)
{
    float bestVal = 0, curVal;
    InvBlade *iBlade;

    InventoryObject *io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].inventory->objects.First();
    while (io)
    {

        if (INVOBJ_BLADE == io->type)
        {
            curVal = 0;
            iBlade = (InvBlade *)io->extra;
            curVal += ((InvBlade *)io->extra)->toHit;
            curVal += ((InvBlade *)io->extra)->damageDone;

            curVal += ((InvBlade *)io->extra)->poison;
            curVal += ((InvBlade *)io->extra)->heal;
            curVal += ((InvBlade *)io->extra)->slow;
            curVal += ((InvBlade *)io->extra)->blind;
            if (curVal > bestVal)
                bestVal = curVal;
        }

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].inventory->objects.Next();
    }

    io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].wield->objects.First();
    while (io)
    {

        if (INVOBJ_BLADE == io->type)
        {
            curVal = 0;
            iBlade = (InvBlade *)io->extra;
            curVal += ((InvBlade *)io->extra)->toHit;
            curVal += ((InvBlade *)io->extra)->damageDone;

            curVal += ((InvBlade *)io->extra)->poison;
            curVal += ((InvBlade *)io->extra)->heal;
            curVal += ((InvBlade *)io->extra)->slow;
            curVal += ((InvBlade *)io->extra)->blind;
            if (curVal > bestVal)
                bestVal = curVal;
        }

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].wield->objects.Next();
    }

    io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].workbench->objects.First();
    while (io)
    {

        if (INVOBJ_BLADE == io->type)
        {
            curVal = 0;
            iBlade = (InvBlade *)io->extra;
            curVal += ((InvBlade *)io->extra)->toHit;
            curVal += ((InvBlade *)io->extra)->damageDone;

            curVal += ((InvBlade *)io->extra)->poison;
            curVal += ((InvBlade *)io->extra)->heal;
            curVal += ((InvBlade *)io->extra)->slow;
            curVal += ((InvBlade *)io->extra)->blind;
            if (curVal > bestVal)
                bestVal = curVal;
        }

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].workbench->objects.Next();
    }

    return bestVal;
}

//******************************************************************
void BBOSAvatar::QuestMovement(SharedSpace *ss)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_GOTO == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_LOCATION == qt->type)
                        {
                            if (cellX == qt->x && cellY == qt->y && 
                                 SPACE_GROUND == ss->WhatAmI())
                            {
                                q->completeVal = 10000; // finished!

                                if (q->questSource < MAGIC_MAX)
                                {
                                    sprintf(tempText,"***** Quest %d has been completed!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                    sprintf(tempText,"***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                                else
                                {
                                    questMan->ProcessWitchQuest(this, ss, q);
                                }
                            }
                        }
                    }
                }
                else if (QUEST_VERB_KILL == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_LOCATION == qt->type)
                        {
                            DungeonMap *dm = (DungeonMap *) ss;
                            if (abs(cellX - qt->x) < 4 && abs(cellY - qt->y) < 4 && 
                                 0 == q->completeVal && SPACE_DUNGEON == ss->WhatAmI() &&
                                 qt->mapSubType == GetCRCForString(dm->name))
                            {
                                q->completeVal = 1; // made monster; don't make it again.

                                BBOSMonster *monster;
                                if (0 == qt->monsterType)
                                {
                                    sprintf(tempText,"You feel the presence of the hellspawn in the distance.");
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                    monster = new BBOSMonster(2,5,false);
                                    monster->cellX = monster->targetCellX = monster->spawnX = qt->x;
                                    monster->cellY = monster->targetCellY = monster->spawnY = qt->y;
                                    ss->mobList->Add(monster);
                                    sprintf(monster->uniqueName, "Demon Prince");

                                    //	adjusts its power to match player power
                                    monster->r               = 255;
                                    monster->g               = 0;
                                    monster->b               = 0;
                                    monster->a               = 255;
                                    monster->sizeCoeff       = 1.5f + 1.0f * qt->monsterSubType /800;
                                    monster->health          = 40 * qt->monsterSubType;
                                    monster->maxHealth       = 40 * qt->monsterSubType;
                                    monster->damageDone      = qt->monsterSubType/20;
                                    if (monster->damageDone > 45)
                                        monster->damageDone = 45;

                                    monster->toHit           = qt->monsterSubType/14;
                                    monster->defense         = qt->monsterSubType/14;
                                    monster->dropAmount      = qt->monsterSubType/100;
                                    monster->magicResistance = qt->monsterSubType/20/100;
//                                    if (monster->magicResistance > 1)
//                                        monster->magicResistance = 1;
									monster->tamingcounter = 1000;						// demon princes can't be tamed either.

                                    monster->healAmountPerSecond = qt->monsterSubType/3;

                                }
                                else
                                {
                                    sprintf(tempText,"You smell the stench of the possessed beast in the distance.");
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                    monster = new BBOSMonster(
                                               pmTypes[qt->monsterType-1].type, 
                                         pmTypes[qt->monsterType-1].subType,false);
                                    monster->cellX = monster->targetCellX = monster->spawnX = qt->x;
                                    monster->cellY = monster->targetCellY = monster->spawnY = qt->y;
                                    ss->mobList->Add(monster);
                                    sprintf(tempText,"%s - %s", pmTypes[qt->monsterType-1].name, 
                                                charInfoArray[curCharacterIndex].name);
                                    CopyStringSafely(tempText, 1024, 
                                                      monster->uniqueName, 31);

                                    //	adjusts its power to match player power
                                    monster->r               = 255;
                                    monster->g               = 0;
                                    monster->b               = 0;
                                    monster->a               = 255;
                                    monster->sizeCoeff       = 1.5f + 1.0f * qt->monsterSubType /800;
                                    monster->health          = 40 * qt->monsterSubType;
                                    monster->maxHealth       = 40 * qt->monsterSubType;
                                    monster->damageDone      = qt->monsterSubType/20;
                                    if (monster->damageDone > 55)
                                        monster->damageDone = 55;

                                    monster->toHit           = qt->monsterSubType/14;
                                    monster->defense         = qt->monsterSubType/14;
                                    monster->dropAmount      = qt->monsterSubType/100;
                                    monster->magicResistance = qt->monsterSubType/20/100;
 //                                   if (monster->magicResistance > 1)
 //                                       monster->magicResistance = 1;
									monster->tamingcounter = 1000;						// no taming a possee and abusing it's high regen.

                                    monster->healAmountPerSecond = qt->monsterSubType/3;

                                    monster->isPossessed = TRUE;
									monster->dontRespawn = TRUE;
                                    monster->AddPossessedLoot(qt->monsterSubType/150+qt->range);
                                }

                                // tell everyone about me
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

                                mAppear.staticMonsterFlag = FALSE;

                                ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
                                         sizeof(mAppear),(void *)&mAppear);
                            }
                            else if (abs(cellX - qt->x) < 4 && abs(cellY - qt->y) < 4 &&
                                 0 == q->completeVal && SPACE_GROUND == ss->WhatAmI() && (qt->mapType==SPACE_GROUND))
                            {
                                q->completeVal = 1; // made monster; don't make it again.

                                qt->monsterType    = Bracket(qt->monsterType   , 0, NUM_OF_MONSTERS-1);
                                qt->monsterSubType = Bracket(qt->monsterSubType, 0, NUM_OF_MONSTER_SUBTYPES-1);
                                qt->range = Bracket(qt->range,1,100000);

                                BBOSMonster *monster;
                                sprintf(tempText,"You see a creature in the distance.");
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                monster = new BBOSMonster(qt->monsterType,qt->monsterSubType,false);
                                monster->cellX = monster->targetCellX = monster->spawnX = qt->x;
                                monster->cellY = monster->targetCellY = monster->spawnY = qt->y;
                                ss->mobList->Add(monster);
								sprintf(tempText, "%s - %s", qt->otherName,
									charInfoArray[curCharacterIndex].name);
								CopyStringSafely(tempText, 1024, monster->uniqueName, 32);
								CopyStringSafely(monster->uniqueName,32, qt->otherName,64);

                                
                                //	adjust its power
                                monster->sizeCoeff       = 1.5f + 1.0f * qt->range /800;
                                monster->health          = 40 * qt->range;
                                monster->maxHealth       = 40 * qt->range;
                                monster->damageDone      = qt->range/20;

                                monster->toHit           = qt->range/14;
                                monster->defense         = qt->range/14;
                                monster->dropAmount      = qt->range/100;
                                monster->magicResistance = qt->range/20/100;
 //                               if (monster->magicResistance > 1)
 //                                   monster->magicResistance = 1;
								monster->tamingcounter = 1000;						// no taming OP quest monsters!

                                monster->healAmountPerSecond = qt->range/3;
                                // remember who spawned it
								monster->questowner = this;
                                // tell everyone about me
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

                                mAppear.staticMonsterFlag = FALSE;

                                ss->SendToEveryoneNearBut(0, monster->cellX, monster->cellY,
                                         sizeof(mAppear),(void *)&mAppear);
                            }
                        }
                    }
                }
                else if (QUEST_VERB_RETRIEVE == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_LOCATION == qt->type)
                        {
                            if (cellX == qt->x && cellY == qt->y && 
                                 SPACE_REALM == ss->WhatAmI() && 
                                 qt->mapSubType == ((RealmMap *)ss)->type &&
                                 q->questSource < MAGIC_MAX &&
                                 0 == q->completeVal)
                            {
                                sprintf(tempText,"There's something unusual on the ground here...");
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                InventoryObject *iObject = 
                                        new InventoryObject(INVOBJ_SIMPLE,0,
                                                             questRetrieveTypeDesc[q->questSource]);
                                iObject->mass = 1.0f;
                                iObject->value = 10;
                                iObject->amount = 1;
                                iObject->status = INVSTATUS_QUEST_ITEM;

                                RealmMap *rp = (RealmMap *) ss;

                                Inventory *inv = rp->GetGroundInventory(qt->x, qt->y);
                                inv->objects.Append(iObject);

                                MessMobAppear messMA;
                                messMA.mobID = (unsigned long) inv;
                                messMA.type = SMOB_ITEM_SACK;
                                messMA.x = qt->x;
                                messMA.y = qt->y;
                                ss->SendToEveryoneNearBut(0, qt->x, qt->y,
                                                sizeof(messMA),(void *)&messMA);

                                q->completeVal += 1;

                            }
                        }
                        else if (QUEST_TARGET_NPC == qt->type)
                        {
                            if (qt->monsterType < MAGIC_MAX && SPACE_GROUND == ss->WhatAmI())
                            {
                                if (cellX == greatTreePos[qt->monsterType][0] && 
                                     cellY == greatTreePos[qt->monsterType][1])
                                {

                                    sprintf(tempText,"The Great Tree gives you a %s.", questRetrieveTypeDesc[q->questSource]);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                                    InventoryObject *iObject = 
                                             new InventoryObject(INVOBJ_SIMPLE,0,
                                                                        questRetrieveTypeDesc[q->questSource]);
                                    iObject->mass = 1.0f;
                                    iObject->value = 10;
                                    iObject->amount = 1;
                                    iObject->status = INVSTATUS_QUEST_ITEM;

                                    charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                                    q->completeVal = 10000; // finished!

                                    sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    if (q->questSource < MAGIC_MAX)
                                    {
                                        sprintf(tempText,"***** Return to the Great Tree of the %s to give the item to it.  ", magicNameList[q->questSource]);
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::QuestTime(SharedSpace *ss)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal)// &&
//			 10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            LongTime now;
			if (now.MinutesDifference(&q->timeLeft) <= 0)
			{
				q->EmptyOut();
				sprintf(tempText, "***** Quest %d has expired.", i + 1);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			}
			else if (now.MinutesDifference(&q->timeLeft) > 60*25) // no quest has more than 25 hours to complete!
			{
				q->EmptyOut();
				sprintf(tempText, "***** Quest %d has expired.", i + 1);
				CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
				ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
			}
				else
				{
                /*
                QuestPart *qp = q->GetVerb();
                if (qp)
                {
                    if (QUEST_VERB_GOTO == qp->type)
                    {
                        QuestPart *qt = q->GetTarget();
                        if (qt)
                        {
                            if (QUEST_TARGET_LOCATION == qp->type)
                            {
                                if (cellX == qp->x && cellY == qp->y)
                                {
                                    q->completeVal = 10000; // finished!

                                    CopyStringSafely("*****", 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    sprintf(tempText,"Quest %d has been completed!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    CopyStringSafely("*****", 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                            }
                        }
                        
                    }
                }
                */
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::QuestMonsterKill(SharedSpace *ss, BBOSMonster *deadMonster)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_KILL == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_MONSTER_TYPE == qt->type)
                        {
                            if (qt->monsterType == deadMonster->type &&
                                 (qt->monsterSubType == deadMonster->subType ||
                                  -1 == qt->monsterSubType)
                                )
                            {
//								if ((ss->WhatAmI()!=SPACE_DUNGEON) // if it's not a dungeon.
//									|| ((((DungeonMap *)ss)->specialFlags & SPECIAL_DUNGEON_TEMPORARY) == 0) // or it's not a geo
//									|| (qt->monsterType !=28 )												 // or it's not a lizardman.
//									)
//								{
									q->completeVal = 10000; // finished!

									if (q->questSource < MAGIC_MAX)
									{
										sprintf(tempText, "***** Quest %d has been completed!", i + 1);
										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);

										sprintf(tempText, "***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
										CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
										ss->lserver->SendMsg(sizeof(infoText), (void *)&infoText, 0, &tempReceiptList);
									}
									else
									{
										questMan->ProcessWitchQuest(this, ss, q);
									}
//								}
                            }
                        }
                        else if (QUEST_TARGET_LOCATION == qt->type)
                        {
                            DungeonMap *dm = (DungeonMap *) ss;

                            if (2 == deadMonster->type &&
                                 5 == deadMonster->subType && 0 == qt->monsterType &&
                                 cellX == qt->x && cellY == qt->y && IsSame(deadMonster->uniqueName, "Demon Prince")  // Make sure it's actually the demon prince
                                )
                            {
                                q->completeVal = 10000; // finished!

                                sprintf(tempText,"***** Quest %d has been completed!", i + 1);
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                if (q->questSource < MAGIC_MAX)
                                {
                                    sprintf(tempText,"***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                                else
                                {
                                    questMan->ProcessWitchQuest(this, ss, q);
                                }
                            }
                            else if (!strncmp(deadMonster->uniqueName,"Possessed", 9) && // it's a posse
                                 qt->monsterType > 0 &&                                  // has a proper type
                                 qt->mapSubType == GetCRCForString(dm->name) &&          // in the right dungeon
								(q->completeVal==1))									 // this posse quest has it's monster spawned
                            {
                                q->EmptyOut(); // finished!

                                sprintf(tempText,"***** Quest %d has been completed!", i + 1);
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                            else if ((IsSame(deadMonster->uniqueName,qt->otherName)) &&  // has right name
                                 (qt->monsterType == deadMonster->type) &&               // and right type
                                 (qt->monsterSubType == deadMonster->subType) &&         // and right subtype
								(deadMonster->questowner == this))                       // AND this avatar was the one wwho spawned the monster
                            {
                                questMan->ProcessWitchQuest(this, ss, q);
                            }
                        }
                    }
                }
            }
        }
    }
}


//Test possessed quest monster dies, gives loot, and finishes quest entry
//Add generator for nihgttime vampires near towns


//******************************************************************
void BBOSAvatar::QuestTalk(SharedSpace *ss)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_VISIT == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_PLAYER == qt->type)
                        {
                            BBOSAvatar *other = (BBOSAvatar *)ss->avatars->First();
                            while (other)
                            {
                                if (other != this && other->cellX == cellX && other->cellY == cellY)
                                {
                                    int found = FALSE;

                                    if (QUEST_PLAYER_TYPE_FIGHTER == qt->playerType &&
                                          other->charInfoArray[other->curCharacterIndex].physical >
                                          other->charInfoArray[other->curCharacterIndex].magical &&
										other->charInfoArray[other->curCharacterIndex].physical >
										other->charInfoArray[other->curCharacterIndex].creative &&
										other->charInfoArray[other->curCharacterIndex].physical >
										other->charInfoArray[other->curCharacterIndex].beast)
                                        found = TRUE;
                                    if (QUEST_PLAYER_TYPE_MAGE == qt->playerType &&
                                          other->charInfoArray[other->curCharacterIndex].magical >
                                          other->charInfoArray[other->curCharacterIndex].physical &&
										other->charInfoArray[other->curCharacterIndex].magical >
										other->charInfoArray[other->curCharacterIndex].creative &&
										other->charInfoArray[other->curCharacterIndex].magical >
										other->charInfoArray[other->curCharacterIndex].beast)
                                        found = TRUE;
									if (QUEST_PLAYER_TYPE_CRAFTER == qt->playerType &&
										other->charInfoArray[other->curCharacterIndex].creative >
										other->charInfoArray[other->curCharacterIndex].magical &&
										other->charInfoArray[other->curCharacterIndex].creative >
										other->charInfoArray[other->curCharacterIndex].physical &&
										other->charInfoArray[other->curCharacterIndex].creative >
										other->charInfoArray[other->curCharacterIndex].beast)
										found = TRUE;
									if (QUEST_PLAYER_TYPE_BEASTMASTER == qt->playerType &&
										other->charInfoArray[other->curCharacterIndex].beast >
										other->charInfoArray[other->curCharacterIndex].magical &&
										other->charInfoArray[other->curCharacterIndex].beast >
										other->charInfoArray[other->curCharacterIndex].physical &&
										other->charInfoArray[other->curCharacterIndex].beast >
										other->charInfoArray[other->curCharacterIndex].creative)
										found = TRUE;
									if (QUEST_PLAYER_TYPE_BALANCED == qt->playerType &&
                                          other->charInfoArray[other->curCharacterIndex].creative ==
                                          other->charInfoArray[other->curCharacterIndex].magical &&
                                          other->charInfoArray[other->curCharacterIndex].creative ==
                                          other->charInfoArray[other->curCharacterIndex].physical)
                                        found = TRUE;
                                    if (QUEST_PLAYER_TYPE_YOUNG == qt->playerType &&
                                          other->charInfoArray[other->curCharacterIndex].cLevel < 10)
                                        found = TRUE;
                                    if (QUEST_PLAYER_TYPE_POOR == qt->playerType &&
                                          other->charInfoArray[other->curCharacterIndex].
                                                     inventory->money < 100)
                                        found = TRUE;

                                    if (found)
                                    {
                                        q->completeVal = 10000; // finished!

                                        sprintf(tempText,"***** Quest %d has been completed!", i + 1);
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                        if (q->questSource < MAGIC_MAX)
                                        {
                                            sprintf(tempText,"***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
                                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                        }
                                        else
                                        {
                                            questMan->ProcessWitchQuest(this, ss, q);
                                        }

                                        return;
                                    }
                                }
                                other = (BBOSAvatar *)ss->avatars->Next();
                            }
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::QuestCraftWeapon(SharedSpace *ss, InventoryObject *io, 
                                             float challenge, float work, char *skillName)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_CRAFT == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_WEAPON == qt->type)
                        {
                            int correctSkill = FALSE;

                            if (!strcmp(skillName,"Swordsmith") && 
                                  QUEST_WEAPON_TYPE_SWORD == qt->playerType)
                                correctSkill = TRUE;

                            if (!strcmp(skillName,"Katana Expertise") && 
                                  QUEST_WEAPON_TYPE_KATANA == qt->playerType)
                                correctSkill = TRUE;
     
                            if (!strcmp(skillName,"Claw Expertise") && 
                                  QUEST_WEAPON_TYPE_CLAWS == qt->playerType)
                                correctSkill = TRUE;
       
                            if (!strcmp(skillName,"Chaos Expertise") && 
                                  QUEST_WEAPON_TYPE_CHAOS == qt->playerType)
                                correctSkill = TRUE;
      
                            if (!strcmp(skillName,"Bladestaff Expertise") && 
                                  QUEST_WEAPON_TYPE_DOUBLE == qt->playerType)
                                correctSkill = TRUE;
 
                            if (!strcmp(skillName,"Mace Expertise") && 
                                  QUEST_WEAPON_TYPE_MACE == qt->playerType)
                                correctSkill = TRUE;

                            if (correctSkill)
                            {
                                if (challenge > work / 2)
                                {
                                    q->completeVal = 10000; // finished!
                
                                    io->status = INVSTATUS_QUEST_ITEM;

                                    sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    if (q->questSource < MAGIC_MAX)
                                    {
                                        sprintf(tempText,"***** Return to the Great Tree of the %s to give the item to it.  ", magicNameList[q->questSource]);
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                                    else
                                    {
                                        questMan->ProcessWitchQuest(this, ss, q);
                                    }
            
                                    return;
                                }
                                else
                                {
                                    sprintf(tempText,"That crafting attempt was too easy to complete Quest %d.", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            
//									return;
                                }
                            }
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::QuestImbue(SharedSpace *ss, InventoryObject *io, 
                                      float challenge, float work, char *skillName)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_IMBUE == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_TOTEM == qt->type && INVOBJ_TOTEM == io->type)
                        {
                            InvTotem *extra = (InvTotem *)io->extra;

                            if (0 == extra->imbueDeviation && extra->type == qt->playerType)
                            {
                                if ((challenge > work / 2) || ( extra->quality>16))  // better than diamond always counts
                                {
                                    q->completeVal = 10000; // finished!
                
                                    io->status = INVSTATUS_QUEST_ITEM;

                                    sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    if (q->questSource < MAGIC_MAX)
                                    {
                                        sprintf(tempText,"***** Return to the Great Tree of the %s to give the item to it.  ", magicNameList[q->questSource]);
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                                    else
                                    {
                                        questMan->ProcessWitchQuest(this, ss, q);
                                    }

                                    return;
                                }
                                else
                                {
                                    sprintf(tempText,"That imbuing task was too easy to complete Quest %d.", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            
                                    return;
                                }
                            }
                        }
                        else if (QUEST_TARGET_STAFF == qt->type && INVOBJ_STAFF == io->type)
                        {
                            InvStaff *extra = (InvStaff *)io->extra;

                            if (0 == extra->imbueDeviation && extra->type == qt->playerType)
                            {
                                if ((challenge > work / 2) || (extra->quality>2)) // better than spruce always counts
                                {
                                    q->completeVal = 10000; // finished!

                                    io->status = INVSTATUS_QUEST_ITEM;
                
                                    sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    if (q->questSource < MAGIC_MAX)
                                    {
                                        sprintf(tempText,"***** Return to the Great Tree of the %s to give the item to it.  ", magicNameList[q->questSource]);
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                                    else
                                    {
                                        questMan->ProcessWitchQuest(this, ss, q);
                                    }
            
                                    return;
                                }
                                else
                                {
                                    sprintf(tempText,"That imbuing task was too easy to complete Quest %d.", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            
                                    return;
                                }
                            }
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************
int BBOSAvatar::SacrificeQuestItem(int type, int subType)
{
    int retVal;
    retVal = SacrificeQuestItemFrom(type, subType, charInfoArray[curCharacterIndex].inventory);
    if (retVal)
        return retVal;
    retVal = SacrificeQuestItemFrom(type, subType, charInfoArray[curCharacterIndex].wield);
    if (retVal)
        return retVal;
    retVal = SacrificeQuestItemFrom(type, subType, charInfoArray[curCharacterIndex].workbench);
    if (retVal)
        return retVal;

    return FALSE;
}

//******************************************************************
int BBOSAvatar::SacrificeQuestItemFrom(int type, int subType, Inventory *inv)
{
    InventoryObject *io = (InventoryObject *) inv->objects.First();
    while (io)
    {
        if (INVSTATUS_QUEST_ITEM == io->status)
        {
            if (INVOBJ_BLADE == io->type && QUEST_TARGET_WEAPON == type)
            {
                InvBlade *ib = (InvBlade *) io->extra;
                if (ib->type == subType)
                {
                    inv->objects.Remove(io);
                    delete io;
                    return TRUE;
                }
            }
            else if (INVOBJ_TOTEM == io->type && QUEST_TARGET_TOTEM == type)
            {
                InvTotem *ib = (InvTotem *) io->extra;
                if (ib->type == subType)
                {
                    inv->objects.Remove(io);
                    delete io;
                    return TRUE;
                }
            }
            else if (INVOBJ_STAFF == io->type && QUEST_TARGET_STAFF == type)
            {
                InvStaff *ib = (InvStaff *) io->extra;
                if (ib->type == subType)
                {
                    inv->objects.Remove(io);
                    delete io;
                    return TRUE;
                }
            }
            else if (INVOBJ_SIMPLE == io->type && (
                      QUEST_TARGET_LOCATION == type || QUEST_TARGET_NPC == type))
            {
                io->amount -= 1;
                if (io->amount < 1)
                {
                    inv->objects.Remove(io);
                    delete io;
                }
                return TRUE;
            }
        }
        else // items not specifically marked as quest items
        {
            if (INVOBJ_EGG == io->type && QUEST_TARGET_EGG == type)
            {
                InvEgg *ib = (InvEgg *) io->extra;
                if (ib->type == subType && 0 == ib->quality)
                {
                    io->amount -= 1;
                    if (io->amount < 1)
                    {
                        inv->objects.Remove(io);
                        delete io;
                    }
                    return 1;
                }
            }
            else if (INVOBJ_INGREDIENT == io->type && QUEST_TARGET_DUST == type)
            {
                InvIngredient *ib = (InvIngredient *) io->extra;
                if (ib->type == subType)
                {
                    io->amount -= 1;
                    if (io->amount < 1)
                    {
                        inv->objects.Remove(io);
                        delete io;
                    }
                    return 1;
                }
            }
        }

        io = (InventoryObject *) inv->objects.Next();
    }


    return FALSE;
}

//******************************************************************
void BBOSAvatar::QuestGiveGold(SharedSpace *ss, BBOSAvatar *other, long amount)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_GIVEGOLD == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_PLAYER == qt->type)
                        {
                            if (other != this && other->cellX == cellX && 
                                 other->cellY == cellY && amount >= qt->range)
                            {
                                int found = FALSE;

								if (QUEST_PLAYER_TYPE_FIGHTER == qt->playerType &&
									other->charInfoArray[other->curCharacterIndex].physical >
									other->charInfoArray[other->curCharacterIndex].magical &&
									other->charInfoArray[other->curCharacterIndex].physical >
									other->charInfoArray[other->curCharacterIndex].creative &&
									other->charInfoArray[other->curCharacterIndex].physical >
									other->charInfoArray[other->curCharacterIndex].beast)
									found = TRUE;
								if (QUEST_PLAYER_TYPE_MAGE == qt->playerType &&
									other->charInfoArray[other->curCharacterIndex].magical >
									other->charInfoArray[other->curCharacterIndex].physical &&
									other->charInfoArray[other->curCharacterIndex].magical >
									other->charInfoArray[other->curCharacterIndex].creative &&
									other->charInfoArray[other->curCharacterIndex].magical >
									other->charInfoArray[other->curCharacterIndex].beast)
									found = TRUE;
								if (QUEST_PLAYER_TYPE_CRAFTER == qt->playerType &&
									other->charInfoArray[other->curCharacterIndex].creative >
									other->charInfoArray[other->curCharacterIndex].magical &&
									other->charInfoArray[other->curCharacterIndex].creative >
									other->charInfoArray[other->curCharacterIndex].physical &&
									other->charInfoArray[other->curCharacterIndex].creative >
									other->charInfoArray[other->curCharacterIndex].beast)
									found = TRUE;
								if (QUEST_PLAYER_TYPE_BEASTMASTER == qt->playerType &&
									other->charInfoArray[other->curCharacterIndex].beast >
									other->charInfoArray[other->curCharacterIndex].magical &&
									other->charInfoArray[other->curCharacterIndex].beast >
									other->charInfoArray[other->curCharacterIndex].physical &&
									other->charInfoArray[other->curCharacterIndex].beast >
									other->charInfoArray[other->curCharacterIndex].creative)
									found = TRUE; 
								if (QUEST_PLAYER_TYPE_BALANCED == qt->playerType &&
                                      other->charInfoArray[other->curCharacterIndex].creative ==
                                      other->charInfoArray[other->curCharacterIndex].magical &&
                                      other->charInfoArray[other->curCharacterIndex].creative ==
                                      other->charInfoArray[other->curCharacterIndex].physical)
                                    found = TRUE;
                                if (QUEST_PLAYER_TYPE_YOUNG == qt->playerType &&
                                      other->charInfoArray[other->curCharacterIndex].cLevel<10)
                                    found = TRUE;
                                if (QUEST_PLAYER_TYPE_POOR == qt->playerType &&
                                      other->charInfoArray[other->curCharacterIndex].inventory->money < 100)
                                    found = TRUE;

                                if (found)
                                {
                                    q->completeVal = 10000; // finished!

                                    sprintf(tempText,"***** Quest %d has been completed!", i + 1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    if (q->questSource < MAGIC_MAX)
                                    {
                                        sprintf(tempText,"***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
                                        CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                        ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                    }
                                    else
                                    {
                                        questMan->ProcessWitchQuest(this, ss, q);
                                    }

                                    return;
                                }
                            }
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::QuestPickupItem(SharedSpace *ss, InventoryObject *io)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (1    == q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_RETRIEVE == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_LOCATION == qt->type && INVOBJ_SIMPLE == io->type &&
                             INVSTATUS_QUEST_ITEM == io->status && q->completeVal==1)
                        {
                            q->completeVal = 10000; // finished!
                
                            sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            if (q->questSource < MAGIC_MAX)
                            {
                                sprintf(tempText,"***** Return to the Great Tree of the %s to give the item to it.  ", magicNameList[q->questSource]);
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                            else
                            {
                                questMan->ProcessWitchQuest(this, ss, q);
                            }
            
                            return;
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************
void BBOSAvatar::QuestPrayer(SharedSpace *ss)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_GROUP == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_LOCATION == qt->type &&
                             qt->x == cellX && qt->y == cellY)
                        {
                            q->completeVal = 10000; // finished!
                
                            sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            if (q->questSource < MAGIC_MAX)
                            {
                                sprintf(tempText,"***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                            }
                            else
                            {
                                questMan->ProcessWitchQuest(this, ss, q);
                            }
            
                            return;
                        }
                    }
                    
                }
            }
        }
    }
}

//******************************************************************
int BBOSAvatar::QuestReward(SharedSpace *ss)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_REWARD == qp->type)
                {
                    InventoryObject *iObject;
                    InvIngredient   *exIn;
                    InvIngot        *exIngot;
//					InvFuse         *exFuse;
                    InvStaff        *exStaff;
                    InvTotem        *extra;
//					InvExplosive    *exPlosive;
                    InvEgg          *im;
//					InvPotion       *ip;
                    InvFavor        *exF;

                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_REWARD_TYPE_GOLD == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            sprintf(tempText,"***** You recieve %d gold!", qt->x);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            charInfoArray[curCharacterIndex].inventory->money += qt->x;

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                        else if (QUEST_REWARD_TYPE_INGOT == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            iObject = new InventoryObject(INVOBJ_INGOT,0, qt->WhoAmI());
                            exIngot = (InvIngot *)iObject->extra;
                            exIngot->damageVal = exIngot->challenge = qt->monsterType;
                            exIngot->r = qt->x;
                            exIngot->g = qt->y;
                            exIngot->b = qt->range;

                            iObject->mass = 1.0f;
                            iObject->value = qt->monsterSubType;
                            iObject->amount = 1;

                            charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                            sprintf(tempText,"***** You recieve %s!", qt->WhoAmI());
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                        else if (QUEST_REWARD_TYPE_FAVOR == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            sprintf(tempText,"%s Favor", magicNameList[qt->x]);
                            iObject = new InventoryObject(INVOBJ_FAVOR,0, tempText);
                            exF = (InvFavor *)iObject->extra;
                            exF->spirit = qt->x;

                            iObject->mass = 1.0f;
                            iObject->value = 10;
                            iObject->amount = 1;

                            charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                            sprintf(tempText,"***** You recieve a %s Favor!", magicNameList[qt->x]);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                        else if (QUEST_REWARD_TYPE_EGG == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            iObject = new InventoryObject(INVOBJ_EGG,0, 
                                                           dragonInfo[qt->y][qt->x].eggName);
                            im = (InvEgg *)iObject->extra;
                            im->type = qt->x;
                            im->quality = qt->y;

                            iObject->mass = 1.0f;
                            iObject->value = 1000;
                            iObject->amount = 1;

                            charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                            sprintf(tempText,"***** You recieve a %s!", dragonInfo[qt->y][qt->x].eggName);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                        else if (QUEST_REWARD_TYPE_DUST == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            iObject = new InventoryObject(INVOBJ_INGREDIENT,0, 
                                                           dustNames[qt->x]);
                            exIn = (InvIngredient *)iObject->extra;
                            exIn->type     = qt->x;
                            exIn->quality  = 1;

                            iObject->mass = 1.0f;
                            iObject->value = 1000;
                            iObject->amount = 1;

                            charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                            sprintf(tempText,"***** You recieve a %s!", dustNames[qt->x]);
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                        else if (QUEST_REWARD_TYPE_STAFF == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            iObject = new InventoryObject(INVOBJ_STAFF,0,"Unnamed Staff");
                            exStaff = (InvStaff *)iObject->extra;
                            exStaff->type     = 0;
                            exStaff->quality  = qt->x;

                            iObject->mass = 0.0f;
                            iObject->value = 500 * (qt->x + 1) * (qt->x + 1);
                            iObject->amount = 1;
                            UpdateStaff(iObject, 0);

                            sprintf(tempText,"***** You recieve a %s!", iObject->WhoAmI());
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                        else if (QUEST_REWARD_TYPE_TOTEM == qt->type)
                        {
                            q->completeVal = 10000; // finished!
                
                            iObject = new InventoryObject(INVOBJ_TOTEM,0,"Unnamed Totem");
                            extra = (InvTotem *)iObject->extra;
                            extra->type     = 0;
                            extra->quality  = qt->x;

                            iObject->mass = 0.0f;

                            iObject->value = extra->quality * extra->quality * 14 + 1;
                            if (extra->quality > 12)
                                iObject->value = extra->quality * extra->quality * 14 + 1 + (extra->quality-12) * 1600;
                            iObject->amount = 1;
                            
                            UpdateTotem(iObject);

                            sprintf(tempText,"***** You recieve a %s!", iObject->WhoAmI());
                            CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                            ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);

                            charInfoArray[curCharacterIndex].inventory->AddItemSorted(iObject);

                            questMan->ProcessWitchQuest(this, ss, q);
            
                            return TRUE;
                        }
                    }
                    
                }
            }
        }
    }

    return FALSE;
}

//******************************************************************
// call this BEFORE actually changing the space
void BBOSAvatar::QuestSpaceChange(SharedSpace *spaceFrom, SharedSpace *spaceTo)
{
    for (int i = 0; i < QUEST_SLOTS; ++i)
    {
        Quest *q = &charInfoArray[curCharacterIndex].quests[i];
        if (-1    != q->completeVal &&
             10000 != q->completeVal) // if active quest
        {
            std::vector<TagID> tempReceiptList;
            tempReceiptList.clear();
            tempReceiptList.push_back(socketIndex);

            MessInfoText infoText;
            char tempText[1024];

            QuestPart *qp = q->GetVerb();
            if (qp)
            {
                if (QUEST_VERB_ESCAPE == qp->type)
                {
                    QuestPart *qt = q->GetTarget();
                    if (qt)
                    {
                        if (QUEST_TARGET_SPACE == qt->type)
                        {
                            if (spaceFrom && spaceTo &&
                                 qt->mapType    == spaceFrom->WhatAmI() && 
                                 qt->mapSubType == spaceTo->WhatAmI())
                            {
                                q->completeVal = 10000; // finished!
                    
                                sprintf(tempText,"***** Quest %d has been accomplished!", i + 1);
                                CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                spaceFrom->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                if (q->questSource < MAGIC_MAX)
                                {
                                    sprintf(tempText,"***** Return to the Great Tree of the %s for a reward.  ", magicNameList[q->questSource]);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    spaceFrom->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                                else
                                {
                                    questMan->ProcessWitchQuest(this, spaceFrom, q);
                                }
                
                                return;
                            }
                            else
                            {
                                if (spaceFrom)
                                {
                                    sprintf(tempText,"***** Quest %d is a failure.", i+1);
                                    CopyStringSafely(tempText, 1024, infoText.text, MESSINFOTEXTLEN);
                                    spaceFrom->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
                                }
                                q->EmptyOut();
                            }
                        }
                    }
                }
            }
        }
    }
}

//******************************************************************
int BBOSAvatar::PhysicalStat(void)
{
    if (charInfoArray[curCharacterIndex].physical < totemEffects.effect[TOTEM_PHYSICAL])
        return totemEffects.effect[TOTEM_PHYSICAL];
    return charInfoArray[curCharacterIndex].physical;
}

//******************************************************************
int BBOSAvatar::MagicalStat(void)
{
    if (charInfoArray[curCharacterIndex].magical < totemEffects.effect[TOTEM_MAGICAL])
        return totemEffects.effect[TOTEM_MAGICAL];
    return charInfoArray[curCharacterIndex].magical;
}

//******************************************************************
int BBOSAvatar::CreativeStat(void)
{
	if (charInfoArray[curCharacterIndex].creative < totemEffects.effect[TOTEM_CREATIVE])
		return totemEffects.effect[TOTEM_CREATIVE];
	return charInfoArray[curCharacterIndex].creative;
}
int BBOSAvatar::BeastStat(void)
{
//	if (charInfoArray[curCharacterIndex].creative < totemEffects.effect[TOTEM_BEAST])  // no beast mastery stat totem yet.
//		return totemEffects.effect[TOTEM_CREATIVE];
	return charInfoArray[curCharacterIndex].beast;
}

//*************************************************************************
int BBOSAvatar::DetectSize(char *string, int type)
{
    char tempText[1024];

    int size = 5;

    sprintf(tempText, string);

    // find a match
    int stringSize = 0;
    while (tempText[stringSize] != 0)
    {
        for (int i = 0; i < 6 && BLADE_TYPE_NORMAL == type; ++i)
        {
            if (IsSame(&tempText[stringSize],bladeList[i].name))
            {
                size = i;
                return size;
            }
        }

        for (int i = 0; i < 3; ++i)
        {
            if (IsSame(&tempText[stringSize],katanaList[i].name) && BLADE_TYPE_KATANA == type)
                return i+3;
            if (IsSame(&tempText[stringSize],clawList[i].name) && BLADE_TYPE_CLAWS == type)
                return i+3;
            if (IsSame(&tempText[stringSize],maceList[i].name) && BLADE_TYPE_MACE == type)
                return i+3;
            if (IsSame(&tempText[stringSize],bladestaffList[i].name) && BLADE_TYPE_DOUBLE == type)
                return i+3;
            if (IsSame(&tempText[stringSize],chaosList[i].name) && BLADE_TYPE_CHAOS == type)
                return i+3;
        }

        ++stringSize;
    }

    return size;
}


//*************************************************************************
void BBOSAvatar::DetectIngotTypes(InvBlade *ib, int &type1, int &type2)
{
    type1 = -1;
    type2 = -1;

	if (ib->chromeIngots)
	{
		type1 = 14;
	}
	if (ib->azraelIngots)
	{
		if (type1 == -1)
			type1 = 13;
		else if (type2 == -1)
			type2 = 13;
	}
	if (ib->titaniumIngots)
	{
		if (type1 == -1)
			type1 = 12;
		else if (type2 == -1)
			type2 = 12;
	}
	if (ib->tungstenIngots)
	{
		if (type1 == -1)
			type1 = 11;
		else if (type2 == -1)
			type2 = 11;
	}
	if (ib->maligIngots)
	{
		if (type1 == -1)
			type1 = 10;
		else if (type2 == -1)
			type2 = 10;
	}

    if( ib->chitinIngots )
    {
        if( type1 == -1 )
            type1 = 9;
        else if( type2 == -1 )
            type2 = 9;
    }
    
    if( ib->elatIngots )
    {
        if( type1 == -1 )
            type1 = 8;
        else if( type2 == -1 )
            type2 = 8;
    }
    
    if( ib->vizIngots )
    {
        if( type1 == -1 )
            type1 = 7;
        else if( type2 == -1 )
            type2 = 7;
    }
    
    if( ib->mithIngots )
    {
        if( type1 == -1 )
            type1 = 6;
        else if( type2 == -1 )
            type2 = 6;
    }
    
    if( ib->adamIngots )
    {
        if( type1 == -1 )
            type1 = 5;
        else if( type2 == -1 )
            type2 = 5;
    }
    
    if( ib->zincIngots )
    {
        if( type1 == -1 )
            type1 = 4;
        else if( type2 == -1 )
            type2 = 4;
    }
    
    if( ib->carbonIngots )
    {
        if( type1 == -1 )
            type1 = 3;
        else if( type2 == -1 )
            type2 = 3;
    }
    
    if( ib->steelIngots )
    {
        if( type1 == -1 )
            type1 = 2;
        else if( type2 == -1 )
            type2 = 2;
    }
    
    if( ib->aluminumIngots )
    {
        if( type1 == -1 )
            type1 = 1;
        else if( type2 == -1 )
            type2 = 1;
    }
    
    if( ib->tinIngots )
    {
        if( type1 == -1 )
            type1 = 0;
        else if( type2 == -1 )
            type2 = 0;
    }
}


void BBOSAvatar::AddMastery( SharedSpace *ss ) {
    InventoryObject *io = (InventoryObject *) 
        charInfoArray[curCharacterIndex].wield->objects.First();
    int found = 0;
    char tmp[80];
    char tempText[80];
    MessInfoText infoText;
    
    sprintf( tmp, "" );

    while( io && !found ) {
        if( INVOBJ_BLADE == io->type ) {
            found = 1;

            switch( ((InvBlade *) io->extra )->type ) {
                case BLADE_TYPE_CHAOS:
                    sprintf( tmp, "Chaos Mastery" );
                    break;
                case BLADE_TYPE_NORMAL:
                    sprintf( tmp, "Sword Mastery" );
                    break;
                case BLADE_TYPE_KATANA:
                    sprintf( tmp, "Katana Mastery" );
                    break;
                case BLADE_TYPE_MACE:
                    sprintf( tmp, "Mace Mastery" );
                    break;
                case BLADE_TYPE_CLAWS:
                    sprintf( tmp, "Claw Mastery" );
                    break;
				case BLADE_TYPE_DOUBLE:
					sprintf(tmp, "Bladestaff Mastery");
					break;
			}
                
        }

        io = (InventoryObject *) 
            charInfoArray[curCharacterIndex].wield->objects.Next();
    }
    
    found = 0;
    io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
    while( io && !found ) {
        if( !strcmp( tmp, io->WhoAmI() ) ) {
            InvSkill *iSkill = (InvSkill *)io->extra;
            found = 1;
            
            if( iSkill->skillLevel < 100 && iSkill->skillLevel * 100000 <= iSkill->skillPoints ) {
                std::vector<TagID> tempReceiptList;
    
                tempReceiptList.clear();
                tempReceiptList.push_back(socketIndex);
                
                // made a skill level!!!
                iSkill->skillLevel += 1;
                
                sprintf( tempText,"You gained %s skill!! You feel more in tune with your weapon.", tmp );
                CopyStringSafely(tempText,1024,infoText.text,MESSINFOTEXTLEN);;
                
                ss->lserver->SendMsg(sizeof(infoText),(void *)&infoText, 0, &tempReceiptList);
            }

            iSkill->skillPoints += 1* bboServer->mastery_exp_multiplier;
        }

        io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
    }
}


int BBOSAvatar::GetMasteryForType( int type ) {
    char tmp[80];
    
    sprintf( tmp, "" );

    switch( type ) {
        case BLADE_TYPE_CHAOS:
            sprintf( tmp, "Chaos Mastery" );
            break;
        case BLADE_TYPE_NORMAL:
            sprintf( tmp, "Sword Mastery" );
            break;
        case BLADE_TYPE_KATANA:
            sprintf( tmp, "Katana Mastery" );
            break;
        case BLADE_TYPE_MACE:
            sprintf( tmp, "Mace Mastery" );
            break;
        case BLADE_TYPE_CLAWS:
            sprintf( tmp, "Claw Mastery" );
            break;
        case BLADE_TYPE_DOUBLE:
            sprintf( tmp, "Bladestaff Mastery" );
            break;
    }
        
    InventoryObject *io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.First();
    while( io ) {
        if( !strcmp( tmp, io->WhoAmI() ) ) {
            return ((InvSkill *)io->extra)->skillLevel;
        }

        io = (InventoryObject *) charInfoArray[curCharacterIndex].skills->objects.Next();
    }

    return 0;
}

int BBOSAvatar::GetTamingLevel(void) {
	char tmp[80];

	sprintf(tmp, "Taming");

	InventoryObject *io = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.First();
	while (io) {
		if (!strcmp(tmp, io->WhoAmI())) {
			return ((InvSkill *)io->extra)->skillLevel;
		}

		io = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.Next();
	}

	return 0;
}

int BBOSAvatar::GetTamingExp(void) {
	char tmp[80];

	sprintf(tmp, "Taming");

	InventoryObject *io = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.First();
	while (io) {
		if (!strcmp(tmp, io->WhoAmI())) {
			return ((InvSkill *)io->extra)->skillPoints;
		}

		io = (InventoryObject *)charInfoArray[curCharacterIndex].skills->objects.Next();
	}

	return 0;
}

/* end of file */



