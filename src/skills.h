#ifndef __SKILLS_H__
#define __SKILLS_H__
/********************************************************************
* File: skills.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "guilds.h"

/*******************************************************************/

class Monster;
class Character;
struct skill;

#define N_SKILLS      200  /* includes both spells and skills */

typedef void (skfun_t)(Character *ch, Character *to, struct skill *sk,
					   int argc, char **argv);
#define ASKILL(fun)\
void fun(Character *ch, Character *to, struct skill *sk, int argc, char **argv)

struct skill
{
	int      m_parent;           /* must have that skill before this */
	skfun_t *m_spec;             /* special skill function */
	int      m_minlev [N_GUILDS]; /* minimum level to be used */
	Monster *m_teacher[N_GUILDS]; /* teacher for this skill */
};

extern struct skill skills[N_SKILLS];
extern const char *skill_names[N_SKILLS];

#define IS_SPELL(skill) ((skill) <= TOP_SPELL && (skill) > SPELL_ZERO)
#define IS_SKILL(skill) ((skill) <= TOP_SKILL && (skill) > SKILL_ZERO)

bool set_skill(const int sn, const int gld, const int level,
			   const char *teacher);
bool set_skill(const int sn, const int gld, const int level, Monster *teacher);

/*******************************************************************/

#define SKILL_ZERO           0  /* RESERVED -- DO NOT TOUCH */
/* skill numbers */
#define SKILL_SPELLCASTING   1
#define SKILL_AGILITY        2
#define SKILL_READING        3
#define SKILL_WRITING        4
#define SKILL_BODY_KNOWLEDGE 5
#define SKILL_BASIC_WEAPON   6
#define SKILL_DAGGERS_KNIVES 7
#define SKILL_AXES           8
#define SKILL_SWORDS         9
#define SKILL_SCROLL_READING 10
#define SKILL_MOUNTED_FIGHT  11
#define SKILL_UNARMED_FIGHT  12
#define SKILL_MARTIAL_ARTS   13
#define SKILL_PARY           14
#define SKILL_DODGE          15
#define SKILL_WEAPON_CRAFT   16
#define SKILL_ARMOUR_CRAFT   17
#define SKILL_ARMOURS        18
#define SKILL_GUARDING       19
#define SKILL_POISONS        20
#define SKILL_DIPLOMACY      21
#define SKILL_KEYS_AND_LOCKS 22
#define SKILL_STEALING       23
#define SKILL_STEALTH        24
#define SKILL_LANGUAGES      25
#define SKILL_WORLD_HISTORY  26
#define SKILL_MYTHS_TALES    27
#define SKILL_VALUABLES      28
#define SKILL_ANTUIQUES      29
#define SKILL_GUARDS         30
#define SKILL_PLANTS_TREES   31
#define SKILL_BASIC_HERB     32
#define SKILL_ADV_HERB       33
#define SKILL_ANIMALS        34
#define SKILL_TRACK_AND_HUNT 35
#define SKILL_ARCHERY        36
#define SKILL_NATURAL_SIGNS  37
#define SKILL_ART_OF_MAGIC   38
#define SKILL_ELEMENTS       39
#define SKILL_BASIC_SPELLS   40
#define SKILL_INT_SPELLS     41
#define SKILL_ADV_SPELLS     42
#define SKILL_DARK_SPELLS    43
#define SKILL_POWER_SCROLLS  44
#define SKILL_CHANTS         45
#define SKILL_POTION_MAKING  46
#define SKILL_SHAPESHIFTING  47
#define SKILL_SORCERY        48
#define SKILL_LIGHT_RITES    49
#define SKILL_DARK_RITES     50
#define SKILL_CRT_CALLING    51
#define SKILL_MIRACLES       52
#define SKILL_MEDECINE       53
#define SKILL_HEALING        54
#define SKILL_STORYTELLING   55
#define SKILL_PRAYERS        56
#define SKILL_BLESSINGS      57
#define SKILL_HAMMERS        58
#define SKILL_STAFFS         59

/* put your own skills here, and remeber to update the TOP_SKILL define */

#define TOP_SKILL SKILL_STAFFS
#define MIN_SKILL 1
#define MAX_SKILL 99

#define SPELL_ZERO           100  /* RESERVED -- DO NOT TOUCH */
/* spell numbers */

/* put your own spells here, and remeber to update the TOP_SPELL define */

#define TOP_SPELL SPELL_ZERO
#define MIN_SPELL 101
#define MAX_SPELL 199

/*******************************************************************/
#endif /* __SKILLS_H__ */
