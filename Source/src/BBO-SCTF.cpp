#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "BBOServer.h"
#include "BBO-Smonster.h"
#include "BBO-SCTF.h"
#include "BBO.h"
#include ".\helper\GeneralUtils.h"
#include ".\network\NetWorldMessages.h"
#include "MonsterData.h"

enum
{ 
	CTF_READY,
	CTF_ON,
	CTF_RECOVERING,
	CTF_MAX
};

//******************************************************************
//******************************************************************
BBOSCTF::BBOSCTF(SharedSpace *s) : BBOSAutoQuest(s)
{
	questState = CTF_READY;
}

//******************************************************************
BBOSCTF::~BBOSCTF()
{

}

//******************************************************************
void BBOSCTF::Tick(SharedSpace *unused)
{

}



//******************************************************************
void BBOSCTF::CreateMonster(void)
{

}

//******************************************************************
void BBOSCTF::MonsterEvent(BBOSMonster *theMonster, int eventType, int x, int y)
{

}


/* end of file */


