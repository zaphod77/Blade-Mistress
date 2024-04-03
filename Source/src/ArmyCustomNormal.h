#ifndef BBO_SARMY_CUSTOMNORMAL_H
#define BBO_SARMY_CUSTOMNORMAL_H

#include "BBO-Sarmy.h"

//******************************************************************
class ArmyCustomNormal : public BBOSArmy
{
public:

	ArmyCustomNormal(SharedSpace *s, 
		         int centerX, int centerY, 
					int spawnX , int spawnY, 
					int numLieutenants,int numPrivates,int typeGeneral,int subtypeGeneral, int typeLieutenants, int subtypeLieutenants,int typePrivates,int subtypePrivates);
	virtual ~ArmyCustomNormal();
//	virtual void Tick(SharedSpace *ss);

//	virtual void MonsterEvent(SharedSpace *ss, BBOSMonster *theMonster, int eventType);
};

#endif
