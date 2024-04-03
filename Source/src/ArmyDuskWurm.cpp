
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "ArmyDuskWurm.h"
#include "BBO.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "MonsterData.h"
#include "TotemData.h"


//******************************************************************
//******************************************************************
ArmyDuskWurm::ArmyDuskWurm(SharedSpace *s, int centerX, int centerY, int sX, int sY, int armyIndex) : BBOSArmy(s, centerX, centerY)
{
	// create an army of the duskWurm

	spawnX = sX;
	spawnY = sY;

	// start with a general
	ArmyMember *am = new ArmyMember();
	am->quality = 10;
	am->type    = 23; // dusk Wurm
	am->subType = 2;
	am->targetX = centerX;
	am->targetY = centerY;

	atEase.Append(am);

	// add leutenants
	for (int i = 0; i < 4; ++i)
	{
		am = new ArmyMember();
		am->quality = 1;
		am->type = 22;
		am->subType = 5;   // orc assassin
		am->targetX = centerX + spaceOffset[i][0];
		am->targetY = centerY + spaceOffset[i][1];
		atEase.Append(am);
	}
}

//******************************************************************
ArmyDuskWurm::~ArmyDuskWurm()
{

}

//******************************************************************
BBOSMonster *ArmyDuskWurm::MakeSpecialMonster(ArmyMember *curMember)
{
	return NULL;
}


/* end of file */



