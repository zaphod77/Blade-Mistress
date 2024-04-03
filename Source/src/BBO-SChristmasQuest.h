#ifndef BBO_SCHRISTMASQUEST_H
#define BBO_SCHRISTMASQUEST_H

#include "BBO-SAutoQuest.h"

//******************************************************************
class BBOSChristmasQuest : public BBOSAutoQuest
{
public:
	
	BBOSChristmasQuest(SharedSpace *ss);
	virtual ~BBOSChristmasQuest();
	virtual void Tick(SharedSpace *unused);
	virtual void MonsterEvent(BBOSMonster *theMonster, int eventType, int x = -10, int y = -10);

	void CreateMonster(void);

	int monsterType;
	float monsterPower;
};

#endif
