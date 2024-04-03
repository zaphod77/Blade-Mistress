#ifndef BBO_SMONSTER_H
#define BBO_SMONSTER_H

//#include "pumamesh.h"
#include "BBO-Smob.h"
#include ".\network\NetWorldMessages.h"
#include "inventory.h"
#include "sharedSpace.h"

enum
{
	THIEF_NOT_THIEF,
	THIEF_RUNNING,
	THIEF_WAITING,
	THIEF_MAX
};

class BBOSAvatar;
class BBOSGenerator;

class BBOSMonsterGrave : public BBOSMob
{
public:

	BBOSMonsterGrave(int t, int st, int x, int y);
	virtual ~BBOSMonsterGrave();
	void Tick(SharedSpace *ss);

	int type, subType;
	int spawnX, spawnY;
	char isWandering;
	DWORD spawnTime;

	char uniqueName[32];
	unsigned char r,g,b,a;
	float sizeCoeff;
	long health, maxHealth, damageDone, toHit, defense;
	long dropAmount;
	float magicResistance;

};

class BBOSMonster : public BBOSMob
{
public:

	BBOSMonster(int t, int st, BBOSGenerator *myGenerator, bool isVag = false);
	virtual ~BBOSMonster();
	void Tick(SharedSpace *ss);

	void ReactToAdjacentPlayer(BBOSAvatar *curAvatar, SharedSpace *ss);
	char *Name(void);
	void AnnounceMyselfCustom(SharedSpace *ss);
	void RecordDamageForLootDist(int damageDone, BBOSAvatar *curAvatar);
	void ClearDamageForLootDist (BBOSAvatar *curAvatar);
	BBOSAvatar *MostDamagingPlayer(float degradeCoeff);
	int MonsterMagicEffect(int type, float timeDelta, float amount);
	void HandlePossessedQuest(int amuletsGiven, SharedSpace *ss);
	void HandleQuestDeath(void);
	void AddPossessedLoot(int count);
	void VLordAttack(SharedSpace *ss);
	void UnicornAttack(SharedSpace *ss);
	void BMVLordAttack(SharedSpace *ss);
	void BMUnicornAttack(SharedSpace *ss);
	void PortalBat(SharedSpace *ss);

	int HasTarget(void) {if (curTarget || curMonsterTarget) return TRUE; return FALSE;};

	Inventory *inventory;

	char uniqueName[32];
	unsigned char r,g,b,a;
	float sizeCoeff;

	long health, maxHealth, damageDone, toHit, defense;
	long dropAmount, dropType, lastHealTime, lastWanderlustTime, lastEffectTime;
	long lastThiefTime, thiefMode;
	char isWandering, dontRespawn, isPossessed, form;
	bool isVagabond;
	DWORD creationTime, bombOuchTime;
	int bombDamage;
	int TotalbombDamage;
	float magicResistance;
	bool MoreMeat = false;
	// AI system.  Needed for more complex monoster fights. Work in progress. :)
	// currently used by the Unicorns, adn will be used by other monsers in the future.
	bool SpecialAI = false;  // monster has complex AI if this is TRUE.
	long AICounter = 0;		// increments every second if mSpecialAI is TRUE.  allows for actions to be taken based on time past.
	long AIFlags = 0;		// can use this to implement fight phases and decision making.  These can be set by actions of players, by counter value, or by checking health. :)

	BBOSAvatar *curTarget, *bane, *bombDropper, *questowner;
	BBOSMonster *curMonsterTarget;
	BBOSGenerator *myGenerator;

	BBOSAvatar *controllingAvatar;

	int type, subType;
	int spawnX, spawnY;
	int archMageMode;
	int healAmountPerSecond;
	int tamingcounter;

	BBOSAvatar *attackerPtrList[10];
	long attackerDamageList[10];
	long attackerLifeList[10];

	LongTime creationLongTime;

};

#endif
