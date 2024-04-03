
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "MonsterData.h"

MonsterData monsterData[NUM_OF_MONSTERS][NUM_OF_MONSTER_SUBTYPES] =
{

	// ******* normal monsters
	{
	  {"Ghost"     , 50,  6, 6,18,  6,1, 7, 2, MONSTER_PLACE_DUNGEON}, //0
	  {"Whight"    , 80,  7, 8,20,  7,1, 8, 3, MONSTER_PLACE_DUNGEON},
	  {"Spectre"   ,120,  8,10,24,  8,1, 9, 4, MONSTER_PLACE_DUNGEON},
	  {"Banshee"   ,180, 11,12,30, 11,1,10, 4, MONSTER_PLACE_DUNGEON},
	  {"", 40, 3,3,3, 3,1, 1},
	  {""  ,100, 5,5,5, 6,1, 1}
	},
	{
	  {"Wooden Golem"   ,  80, 1,1,2,  2,1,1,  0, MONSTER_PLACE_ALL}, // 1
	  {"Decaying Golem" , 120, 2,2,3,  3,1,2,  0, MONSTER_PLACE_ALL},
	  {"Dirt Golem"     , 200, 3,3,4,  4,1,3,  1, MONSTER_PLACE_SNOW},	
	  {"Iron Golem"     , 360, 6,3,5,  5,1,5,  1, MONSTER_PLACE_ALL},
	  {"Stone Golem"    , 800, 6,6,7,  8,1,8,  2, MONSTER_PLACE_DUNGEON},
	  {"Granite Golem"  ,1800, 7,7,9, 10,1,10, 3, MONSTER_PLACE_DUNGEON}
	},
	{
	  {"Slithis"         , 20, 1,1,1, 1,1,0,  0, MONSTER_PLACE_ALL}, // 2
	  {"Common Minotaur" , 30, 1,1,1, 1,1,1,  0, MONSTER_PLACE_ALL},
	  {"Tawny Minotaur"  , 60, 2,2,2, 2,1,3,  1, MONSTER_PLACE_ALL},
	  {"Jade Minotaur"   ,120, 3,4,2, 3,1,5,  1, MONSTER_PLACE_ALL},
	  {"Armored Minotaur",180, 4,4,7, 4,1,7,  2, MONSTER_PLACE_ALL},
	  {"Demon Minotaur"  ,600, 7,7,7, 7,1,9,  3, MONSTER_PLACE_ALL}
	},
	{
	  {"Sabertooth"    , 60, 3,2,1, 2,1,5, 1, MONSTER_PLACE_ALL_LAND},//3
	  {"Ice Tiger"     ,100, 4,3,3, 3,1,7, 2, MONSTER_PLACE_SNOW},
	  {"Jaguar"        ,240, 6,5,4, 5,1,9, 2, MONSTER_PLACE_ALL_LAND},
	  {""   , 10, 1,1,1, 1,1},
	  {"", 40, 3,3,3, 3,1},
	  {""  ,100, 5,5,5, 6,1}
	},
	{
	  {"Skeleton"    , 30, 2,2,4, 2,1,3, 1, MONSTER_PLACE_ALL},      //4
	  {""     , 20, 2,2,2, 2,1,7},
	  {""  , 20, 2,2,2, 2,1},
	  {""   , 10, 1,1,1, 1,1},
	  {"", 40, 3,3,3, 3,1},
	  {""  ,100, 5,5,5, 6,1}
	},
	{
	  {"Killer Plant"    ,150,   5,5,1,   4,1,5,  1, MONSTER_PLACE_GRASS +MONSTER_PLACE_SWAMP},//5
	  {"Serpent Plant"   ,1500, 15,15,15, 8,1,12, 3, MONSTER_PLACE_GRASS +MONSTER_PLACE_SWAMP +MONSTER_PLACE_DUNGEON},
	  {""     , 20, 2,2,2, 2,1,7, 1},
	  {""  , 20, 2,2,2, 2,1, 1},
	  {""   , 10, 1,1,1, 1,1, 1},
	  {""  ,100, 5,5,5, 6,1, 1}
	},
	{
	  {"Werewolf"          , 480,  9, 7, 5, 7,1, 7, 2, MONSTER_PLACE_ALL},//6
	  {"Red Werewolf"      , 560, 10, 8, 6, 8,1, 8, 2, MONSTER_PLACE_ALL},
	  {"Frost Werewolf"    , 660, 11, 9, 7, 9,1, 9, 3, MONSTER_PLACE_SNOW},
	  {"Malacite Werewolf" , 780, 13,10, 8,10,1,10, 3, MONSTER_PLACE_ALL},
	  {"Stone Werewolf"    ,1200, 14,11,14,13,1,11, 4, MONSTER_PLACE_DUNGEON},
	  {""  ,100, 5,5,5, 6,1, 1}
	},
	{
	  {"Green Dragon"    ,  1500, 15, 15, 15, 10,1,12, 3, MONSTER_PLACE_ALL},//7
	  {"Dragon Queen"    ,  3000, 18, 18, 18, 14,1,13, 4, MONSTER_PLACE_ALL + MONSTER_PLACE_DRAGONS},
	  {"Guardian Dragon" ,  5000, 23, 23, 23, 18,1,13, 4, MONSTER_PLACE_ALL + MONSTER_PLACE_DRAGONS},
	  {"Dragon Archon"   , 10000, 30, 80, 80, 30,1,13, 4, MONSTER_PLACE_DRAGONS},
	  {"Dragon Overlord" ,100000, 40,140,140, 45,1,13, 4, MONSTER_PLACE_DRAGONS},
	  {"Dragon Illusion" ,  1000, 24, 38, 30,  1,1,13, 4, MONSTER_PLACE_DRAGONS}
	},
	{
	  {"Spider"         ,  25,  1, 3, 1,  1,1,5,  1, MONSTER_PLACE_DUNGEON},//8
	  {"Dark Spider"   ,   80,  2, 4, 2,  3,1,6,  1, MONSTER_PLACE_DUNGEON},
	  {"Hunting Spider" , 160,  3, 5, 3,  2,1,8,  2, MONSTER_PLACE_ALL},
	  {"Hairy Spider",    300,  5, 7, 5,  6,3,9,  2, MONSTER_PLACE_DUNGEON},
	  {"Venomous Spider", 450,  9, 9, 7,  8,6,10, 3, MONSTER_PLACE_DUNGEON},
	  {"Demon Spider"   ,1000, 10,13,10, 12,1,12, 4, MONSTER_PLACE_DUNGEON}
	},

	// ******* spirit monsters 9 -
	{
	  {"Spirit Vision"     , 300, 14,16,24, 4,1,8, 2,MONSTER_PLACE_SPIRITS},//9
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Golem"     , 800, 12,12,26, 10,1,8,  0,MONSTER_PLACE_SPIRITS},//10
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Minotaur"      , 150, 8,8,18, 8,1,8,  0,MONSTER_PLACE_SPIRITS},//11
	  {"Dokk's Centurion"     , 2500, 28,33,38, 20,1,12, 3,MONSTER_PLACE_SPIRITS},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Tiger"    , 800, 18,18,24, 15,1,8, 1, MONSTER_PLACE_SPIRITS},//12

	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Skeleton"    ,1000, 22,22,28, 14,1,8, 1,MONSTER_PLACE_SPIRITS},//13
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Creeper"    ,1000, 24,20,18, 13,1,8, 1,MONSTER_PLACE_SPIRITS},//14
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Werewolf"      ,1200, 26,24,30, 16,1,8, 1,MONSTER_PLACE_SPIRITS},//15
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Dragon"    , 2000, 28,30,36, 16,1,12, 3,MONSTER_PLACE_SPIRITS},//16
	  {"Dokk"             , 6000, 34,38,44, 22,1,12, 3,MONSTER_PLACE_SPIRITS},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Spirit Spider"         ,750, 20,26,32,    14,1,4,  1,MONSTER_PLACE_SPIRITS + MONSTER_PLACE_LABYRINTH},//17
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},

	// ******* dead monsters 18-
	{
	  {"Shade"            , 300, 12,12,15,  8,1,8, 2, MONSTER_PLACE_DEAD},//18
	  {"Haunt"            , 600, 16,28,30, 12,1,8, 1, MONSTER_PLACE_DEAD},
	  {"Wraith"           ,1200, 20,36,45, 16,1,8, 1, MONSTER_PLACE_DEAD},
	  {"Wraith Mistress"  ,2400, 24,42,60, 20,1,8, 1, MONSTER_PLACE_DEAD},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Bone Warrior"     , 200, 12,16,12,  2,1,11, 0, MONSTER_PLACE_DEAD},//19
	  {"Bone Sergeant"    , 600, 20,31,24, 10,1,5, 1, MONSTER_PLACE_DEAD + MONSTER_PLACE_LABYRINTH},
	  {"Bone Lieutenant"  ,1800, 28,46,36, 15,1,11, 1, MONSTER_PLACE_DEAD + MONSTER_PLACE_LABY2},
	  {"Bone Lord"        ,5000, 34,50,50, 28,1,11, 1, MONSTER_PLACE_DEAD},
	  {"Bone Tyrant"      ,6000, 33,60,60, 30,1,11, 1, MONSTER_PLACE_DEAD},
	  {"Bone Imperator"   ,7000, 32,70,70, 32,1,11, 1, MONSTER_PLACE_DEAD + MONSTER_PLACE_LABY3 }
	},
	{
	  {"Anubis"      , 15000, 30,80,80, 25,1,8,  0, MONSTER_PLACE_DEAD},//20
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
		{"Thieving Spirit" , 90, 3,3,3, 0,0,0,  0, MONSTER_PLACE_DEAD},//21
		{"", 40, 3,3,3, 3,1, 1},
		{"", 40, 3,3,3, 3,1, 1},
		{"", 40, 3,3,3, 3,1, 1},
		{"", 40, 3,3,3, 3,1, 1},
		{"", 40, 3,3,3, 3,1, 1}
	},
	
	// ******* dragon realm monsters 22 -
	{
	  {"Orc"             , 300,  4, 6, 4,  5,1,5, 0,MONSTER_PLACE_DRAGONS},//22
	  {"Orc Bully"       , 600,  8,12, 8,  8,1,5, 0,MONSTER_PLACE_DRAGONS},
	  {"Orc Warrior"     ,1200, 16,20,12, 11,1,5, 0,MONSTER_PLACE_DRAGONS},
	  {"Orc Fanatic"     ,2400, 24,28,18, 14,1,3, 0,MONSTER_PLACE_DRAGONS + MONSTER_PLACE_LABYRINTH},
	  {"Orc Champion"    ,5000, 30,37,24, 16,1,5, 0,MONSTER_PLACE_DRAGONS + MONSTER_PLACE_LABY2},
	  {"Orc Assassin"    , 500, 40,48,30, 15,1,5, 0,MONSTER_PLACE_DRAGONS}
	},
	{
	  {"Wurm"      , 4000, 25,50,60, 10,1,5,  2,MONSTER_PLACE_DRAGONS},//23
	  {"Zerad Wurm", 8000, 28,55,85, 15,1,5,  1,MONSTER_PLACE_DRAGONS},
	  {"Dusk Wurm" ,10000, 30,60,60, 10,1,5,  1,MONSTER_PLACE_DRAGONS},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Hornet Spidren",   750,  9,12, 9,  9,6,8, 3, MONSTER_PLACE_ALL + MONSTER_PLACE_LABYRINTH},//24
	  {"Worker Spidren", 1000, 12,25,20,  5,6,3, 3, MONSTER_PLACE_LABYRINTH},
	  {"Drone Spidren",  2000, 15,40,35, 12,6,5, 3, MONSTER_PLACE_LABYRINTH},
	  {"Queen Spidren",  4000, 25,60,50, 16,6,7, 3, MONSTER_PLACE_LABYRINTH + MONSTER_PLACE_LABY2},
	  {"Spidren"      ,  4000, 25,60,50, 16,6,11, 3, 0 /* no random spawning */},
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Zombie",       1000, 20, 15, 20, 10,6,25, 3, MONSTER_PLACE_WASTE},//25
	  {"Ghoul",        2000, 30, 65, 80, 15,6,20, 3, MONSTER_PLACE_WASTE + MONSTER_PLACE_LABY2},
	  {"Vampire",      5000, 40,115,130, 20,6,15, 3, MONSTER_PLACE_WASTE + MONSTER_PLACE_LABY2},
	  {"Ash Vampire", 10000, 60,165,200, 24,6, 9, 3, MONSTER_PLACE_WASTE + MONSTER_PLACE_LABY3 },
	  {"Pit Vampire", 20000, 80,215,250, 26,6, 6, 3, MONSTER_PLACE_WASTE + MONSTER_PLACE_LABY3 },
	  {"", 40, 3,3,3, 3,1, 1}
	},
	{
	  {"Bat",            500, 10, 30, 70, 1,6, 3, 3, MONSTER_PLACE_LABY2 + MONSTER_PLACE_DUNGEON},//26
	  {"Violent Bat",   1500, 20, 60,100, 3,6, 5, 3, MONSTER_PLACE_LABY2 + MONSTER_PLACE_DUNGEON},
	  {"Stingtail Bat", 5000, 30, 90,130, 6,6, 7, 3, MONSTER_PLACE_LABY2 },
	  {"Moon Bat",     25000, 40,120,160, 9,6, 9, 3, MONSTER_PLACE_LABY2 },
	  {"Silver Bat",   50000, 50,150,190,12,6,12, 3, MONSTER_PLACE_LABY2 },
	  {"Albino Bat",     500, 10, 10, 10, 1,6,15, 3, 0},
	},
	{
	  {"Baphomet",   10000, 60,150,200, 10,6, 5, 3, 0},//27
	  {"Dagon",      15000, 80,200,250, 15,6,20, 3, 0},
	  {"Mantus",     20000,100,250,300, 20,6,15, 3, 0},
	  {"Sammael",    25000,120,300,350, 24,6, 9, 3, 0},
	  {"Thoth",      30000,140,350,400, 26,6, 6, 3, 0},
	  {"Typhon",     35000,160,400,450, 26,6, 6, 3, 0}
	},

		// Here lizard lizard lizard
	{
		{ "Pewny Lizard Man",	500, 50, 80, 120, 5,0, 3, 3, MONSTER_PLACE_LABY2 + MONSTER_PLACE_DUNGEON + MONSTER_PLACE_ROP },//28
	{ "Small Lizard Man",	1500, 70, 100,150, 7,0, 5, 3, MONSTER_PLACE_LABY2 + MONSTER_PLACE_DUNGEON + MONSTER_PLACE_ROP },
	{ "Lizard Man",		5000, 100, 150,200, 10,0, 7, 3, MONSTER_PLACE_LABY2 + MONSTER_PLACE_LABY3 + MONSTER_PLACE_ROP },
	{ "Lizard Warrior",	15000, 160, 200,250, 26,0, 6, 3, MONSTER_PLACE_LABY3 },
	{ "Lizard Sentinel",	30000, 200, 300,350, 26,0, 6, 3, MONSTER_PLACE_LABY3 },
	{ "Lizard King",		50000, 250, 350,400, 26,0, 6, 3, 0 }
	},

		  // Bunnys :)
	{
		{ "Baby Bunny",	600, 60, 90, 120, 7,0, 3, 3, MONSTER_PLACE_LABY3 },//29
	  { "Fluffy Bunny",	1600, 80, 110,150, 9,0, 4, 3, MONSTER_PLACE_LABY3 },
	  { "Little Bunny Foo Foo",5500, 150, 160,200, 12,0, 5, 3, MONSTER_PLACE_LABY3 },
	  { "Peter Rabbit",	16000, 170, 210,250, 28,0, 6, 3, MONSTER_PLACE_LABY3 },
	  { "Wascally Wabbit",	35000, 250, 310,350, 28,0, 7, 3, MONSTER_PLACE_LABY3 },
	  { "Vorpal Bunny",		55000, 350, 360,400, 28,0, 8, 3, MONSTER_PLACE_LABY3 }
	},
		  // Pretty Butterflies. :)
	  {
		  { "Fluttershy", 700, 70, 160, 210, 11, 6, 10, 3, MONSTER_PLACE_LABY3 },//30
	  { "Pinky Pie",      1700, 90,210,260, 16,6,10, 3, MONSTER_PLACE_LABY3 },
	  { "Applejack",     16000,110,260,310, 21,6,15, 3, 0 },
	  { "Rarity",    17000,130,310,360, 25,6, 9, 3, 0 },
	  { "Rainbow Dash",      31000,150,360,410, 27,6, 6, 3, 0 },
	  { "Twilight Sparkle",     36000,170,410,460, 28,6, 6, 3, 0 }
	  },
		  // Cute Unicorns. :)
	  {
		  {"Mr. Ed", 800, 80, 170, 220, 15, 6, 5, 4, 0 },//31    brown
		  { "Silver",      1800, 100,220,270, 20,6,20, 4, 0 }, //  white
		  { "Princess",     17000,160,270,320, 25,6,15, 4, 0 },					// pink
		  { "Trigger",    18000,180,320,370, 29,6, 9, 4, 0 },						// purple
		  { "Traveller",      33000,260,370,420, 31,6, 6, 4, 0 },				// midnight blue
		  { "Wild Fire",     37000,360,420,470, 32,6, 6, 4, 0 }				// black 
	  },
		  // Deer
	  {
		  {"Reindeer", 10, 1,1,1, 1,1, 5,3,0 },//32    brown
		  { "Rudolph",10, 1,1,1, 1,1, 5,3,0}, //  red
		  {"", 40, 3,3,3, 3,1, 5,3,0},
		  {"", 40, 3,3,3, 3,1, 5,3,0},
		  {"", 40, 3,3,3, 3,1, 5,3,0},
		  {"", 40, 3,3,3, 3,1, 5,3,0}
	  },
			// Valentine Hearts
	  {
		  {"Heart", 10, 1,1,1, 1,1, 5,3,0 },//33 red
		  { "",40, 3,3,3, 1,1, 5,3,0}, 
		  {"", 40, 3,3,3, 3,1, 5,3,0},
		  {"", 40, 3,3,3, 3,1, 5,3,0},
		  {"", 40, 3,3,3, 3,1, 5,3,0},
		  {"", 40, 3,3,3, 3,1, 5,3,0}
	  }


};
/* 
	char name[64];
	long maxHealth, damageDone, toHit, defense;
	long dropAmount, dropType(not used), townDist;
	int  dungeonType;	// 0-4, 4 is the toughest dungeon
	unsigned long placementFlags;
	*/
/* end of file */



