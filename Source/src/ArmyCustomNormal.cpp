
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "ArmyCustomNormal.h"
#include "BBO.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "MonsterData.h"

//******************************************************************
//******************************************************************
ArmyCustomNormal::ArmyCustomNormal(SharedSpace *s, int centerX, int centerY, int sX, int sY, int numLieutenants, int numPrivates , int typeGeneral, int subtypeGeneral, int typeLieutenants, int subtypeLieutenants, int typePrivates, int subtypePrivates) : BBOSArmy(s, centerX, centerY)
{
	// create an army of spiders!

	spawnX = sX;
	spawnY = sY;

	// start with a general
	ArmyMember *am = new ArmyMember();
	am->quality = 10;
	am->type = typeGeneral;
	am->subType = subtypeGeneral; 
	am->targetX = centerX;
	am->targetY = centerY;
	atEase.Append(am);

	// add leutenants
	for (int i = 0; i < numLieutenants; ++i)
	{
		am = new ArmyMember();
		am->quality = 6;
		am->type = typeLieutenants;
		am->subType = subtypeLieutenants; 
		am->targetX = centerX + spaceOffset[i % 8][0];
		am->targetY = centerY + spaceOffset[i % 8][1];
		atEase.Append(am);
	}

	// add privates
	for (int i = 0; i < numPrivates; ++i)
	{
		am = new ArmyMember();
		am->quality = 3;
		am->type = typePrivates;
		am->subType = subtypePrivates; // hunting spiders
		am->targetX = centerX + spaceOffset[i % 8][0] * 3 + (rand() % 3) - 1;
		am->targetY = centerY + spaceOffset[i % 8][1] * 3 + (rand() % 3) - 1;
		atEase.Append(am);
	}

}

//******************************************************************
ArmyCustomNormal::~ArmyCustomNormal()
{

}


/* end of file */



