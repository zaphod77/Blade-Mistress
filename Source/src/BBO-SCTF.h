#ifndef BBO_SCHAINQUEST_H
#define BBO_SCHAINQUEST_H

#include "BBO-SAutoQuest.h"

//******************************************************************
class BBOSCTF : public BBOSAutoQuest
{
public:

	BBOSCTF(SharedSpace *ss);
	virtual ~BBOSCTF();
	virtual void Tick(SharedSpace *unused);
	virtual void MonsterEvent(BBOSMonster *theMonster, int eventType, int x = -10, int y = -10);

	void CreateMonster(void);

	int monsterType;
	float monsterPower;




};

#endif
