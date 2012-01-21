/********************************************************************
* File: skills.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "monster.h"
#include "skills.h"

/*******************************************************************/

const char *skill_names[N_SKILLS] = {
	NULL, /* the reserved first skill */

	/* spells */
	"spellcasting",
	"agility",
	"reading",
	"writing",
	"body knowledge",
	"basic weapon knowledge",
	"using daggers and knives",
	"using axes",
	"using swords",
	"scroll reading",
	"mounted fight",
	"unarmed fight",
	"martial arts",
	"pary",
	"dodge",
	"weapon crafting",
	"armour crafting",
	"armours",
	"guarding",
	"poisons",
	"diplomacy",
	"keys and locks",
	"stealing",
	"stealth",
	"languages",
	"world history",
	"myths and tales",
	"valuables",
	"antuiques",
	"guards",
	"plants and trees",
	"basic herb knowledge",
	"advanced herb knowledge",
	"animals",
	"tracking and hunting",
	"archery",
	"signs of nature",
	"the art of magic",
	"elements",
	"basic spells",
	"intermediate spells",
	"advanced spells",
	"dark spells",
	"power scrolls",
	"chants",
	"potion making",
	"shapeshifting",
	"sorcery",
	"light rites",
	"dark rites",
	"creature calling",
	"miracles",
	"medecine",
	"healing",
	"storytelling",
	"prayers",
	"blessings",
	"using hammers",
	"using staffs",

	"", "", "", "", "", "", "", "", "", "",  /* 60-69 */
	"", "", "", "", "", "", "", "", "", "",  /* 70-79 */
	"", "", "", "", "", "", "", "", "", "",  /* 80-89 */
	"", "", "", "", "", "", "", "", "", "",  /* 90-99 */

	/* spells */
	"", "", "", "", "", "", "", "", "", "",  /* 100-109 */
	"", "", "", "", "", "", "", "", "", "",  /* 110-119 */
	"", "", "", "", "", "", "", "", "", "",  /* 120-129 */
	"", "", "", "", "", "", "", "", "", "",  /* 130-139 */
	"", "", "", "", "", "", "", "", "", "",  /* 140-149 */
	"", "", "", "", "", "", "", "", "", "",  /* 150-159 */
	"", "", "", "", "", "", "", "", "", "",  /* 160-169 */
	"", "", "", "", "", "", "", "", "", "",  /* 170-179 */
	"", "", "", "", "", "", "", "", "", "",  /* 180-189 */
	"", "", "", "", "", "", "", "", "", "",  /* 190-199 */
};

struct skill skills[N_SKILLS] = {
	{ -1                , NULL, },  /* SKILL_ZERO           */
	{ -1                , NULL, },  /* SKILL_SPELLCASTING   */
	{ -1                , NULL, },  /* SKILL_AGILITY        */
	{ -1                , NULL, },  /* SKILL_READING        */
	{ -1                , NULL, },  /* SKILL_WRITING        */
	{ -1                , NULL, },  /* SKILL_BODY_KNOWLEDGE */
	{ -1                , NULL, },  /* SKILL_BASIC_WEAPON   */
	{ SKILL_BASIC_WEAPON, NULL, },  /* SKILL_DAGGERS_KNIVES */
	{ SKILL_BASIC_WEAPON, NULL, },  /* SKILL_AXES           */
	{ SKILL_BASIC_WEAPON, NULL, },  /* SKILL_SWORDS         */
	{ -1                , NULL, },  /* SKILL_SCROLL_READING */
	{ -1                , NULL, },  /* SKILL_MOUNTED_FIGHT  */
	{ -1                , NULL, },  /* SKILL_UNARMED_FIGHT  */
	{ SKILL_UNARMED_FIGHT,NULL, },  /* SKILL_MARTIAL_ARTS   */
	{ -1                , NULL, },  /* SKILL_PARY           */
	{ -1                , NULL, },  /* SKILL_DODGE          */
	{ SKILL_BASIC_WEAPON, NULL, },  /* SKILL_WEAPON_CRAFT   */
	{ SKILL_ARMOURS     , NULL, },  /* SKILL_ARMOUR_CRAFT   */
	{ -1                , NULL, },  /* SKILL_ARMOURS        */
	{ -1                , NULL, },  /* SKILL_GUARDING       */
	{ -1                , NULL, },  /* SKILL_POISONS        */
	{ SKILL_LANGUAGES   , NULL, },  /* SKILL_DIPLOMACY      */
	{ -1                , NULL, },  /* SKILL_KEYS_AND_LOCKS */
	{ -1                , NULL, },  /* SKILL_STEALING       */
	{ -1                , NULL, },  /* SKILL_STEALTH        */
	{ -1                , NULL, },  /* SKILL_LANGUAGES      */
	{ -1                , NULL, },  /* SKILL_WORLD_HISTORY  */
	{ SKILL_WORLD_HISTORY,NULL, },  /* SKILL_MYTHS_TALES    */
	{ -1                , NULL, },  /* SKILL_VALUABLES      */
	{ SKILL_VALUABLES   , NULL, },  /* SKILL_ANTUIQUES      */
	{ -1                , NULL, },  /* SKILL_GUARDS         */
	{ -1                , NULL, },  /* SKILL_PLANTS_TREES   */
	{ SKILL_PLANTS_TREES, NULL, },  /* SKILL_BASIC_HERB     */
	{ SKILL_BASIC_HERB  , NULL, },  /* SKILL_ADV_HERB       */
	{ -1                , NULL, },  /* SKILL_ANIMALS        */
	{ SKILL_ANIMALS     , NULL, },  /* SKILL_TRACK_AND_HUNT */
	{ SKILL_BASIC_WEAPON, NULL, },  /* SKILL_ARCHERY        */
	{ -1                , NULL, },  /* SKILL_NATURAL_SIGNS  */
	{ SKILL_SPELLCASTING, NULL, },  /* SKILL_ART_OF_MAGIC   */
	{ SKILL_ART_OF_MAGIC, NULL, },  /* SKILL_ELEMENTS       */
	{ SKILL_ART_OF_MAGIC, NULL, },  /* SKILL_BASIC_SPELLS   */
	{ SKILL_BASIC_SPELLS, NULL, },  /* SKILL_INT_SPELLS     */
	{ SKILL_INT_SPELLS  , NULL, },  /* SKILL_ADV_SPELLS     */
	{ SKILL_ADV_SPELLS  , NULL, },  /* SKILL_DARK_SPELLS    */
	{ -1                , NULL, },  /* SKILL_POWER_SCROLLS  */
	{ -1                , NULL, },  /* SKILL_CHANTS         */
	{ -1                , NULL, },  /* SKILL_POTION_MAKING  */
	{ SKILL_ANIMALS     , NULL, },  /* SKILL_SHAPESHIFTING  */
	{ SKILL_SPELLCASTING, NULL, },  /* SKILL_SORCERY        */
	{ SKILL_SORCERY     , NULL, },  /* SKILL_LIGHT_RITES    */
	{ SKILL_LIGHT_RITES , NULL, },  /* SKILL_DARK_RITES     */
	{ SKILL_ANIMALS     , NULL, },  /* SKILL_CRT_CALLING    */
	{ -1                , NULL, },  /* SKILL_MIRACLES       */
	{ SKILL_BODY_KNOWLEDGE,NULL,},  /* SKILL_MEDECINE       */
	{ SKILL_MEDECINE    , NULL, },  /* SKILL_HEALING        */
	{ -1                , NULL, },  /* SKILL_STORYTELLING   */
	{ -1                , NULL, },  /* SKILL_PRAYERS        */
	{ SKILL_PRAYERS     , NULL, },  /* SKILL_BLESSINGS      */
	{ SKILL_BASIC_WEAPON, NULL, },  /* SKILL_HAMMERS        */
	{ SKILL_SORCERY     , NULL, },  /* SKILL_STAFFS         */

	/* 60 - 99 */
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },

	/* 100 - 199 */
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
	{ -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, }, { -1, NULL, },
};

/*******************************************************************/

bool skill_init(void)
{
	int sn, g;

	for (sn = 0; sn < N_SKILLS; sn++)
	{
		for (g = 0; g < N_GUILDS; g++)
		{
			skills[sn].m_minlev [g] = -1;
			skills[sn].m_teacher[g] = NULL;
		}
	}

	return true;
}

/* TODO:  make these two functions into macros, or inline them? */
bool set_skill(const int sn, const int gld, const int level,
			   const char *teacher)
{
	skills[sn].m_minlev [gld] = level;
	skills[sn].m_teacher[gld] = (Monster *) find_object(teacher);
	return true;
}

bool set_skill(const int sn, const int gld, const int level, Monster *teacher)
{
	skills[sn].m_minlev [gld] = level;
	skills[sn].m_teacher[gld] = teacher;
	return true;
}

/*******************************************************************/
