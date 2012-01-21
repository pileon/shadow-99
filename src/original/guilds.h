#ifndef __GUILDS_H__
#define __GUILDS_H__
/********************************************************************
* File: guilds.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

class Monster;

/*******************************************************************/

#define GUILD_NONE    0
#define GUILD_WARRIOR 1
#define GUILD_THIEF   2
#define GUILD_RANGER  3
#define GUILD_MAGE    4
#define GUILD_DRUID   5
#define GUILD_PRIEST  6

#define N_GUILDS 7  /* all the guilds */

#define IS_NONE(ch)    \
    ((ch)->is_player() ? (ch)->m_guild == GUILD_NONE : FALSE)
#define IS_WARRIOR(ch) \
    ((ch)->is_player() ? (ch)->m_guild == GUILD_WARRIOR : FALSE)
#define IS_THIEF(ch)   \
    ((ch)->is_player() ? (ch)->m_guild == GUILD_THIEF : FALSE)
#define IS_RANGER(ch)  \
    ((ch)->is_player() ? (ch)->m_guild == GUILD_RANGER : FALSE)
#define IS_MAGE(ch)    \
    ((ch)->is_player() ? (ch)->m_guild == GUILD_MAGE : FALSE)
#define IS_DRUID(ch)   \
    (ch)->is_player() ? ((ch)->m_guild == GUILD_DRUID : FALSE)
#define IS_PRIEST(ch)  \
    ((ch)->is_player() ? (ch)->m_guild == GUILD_PRIEST : FALSE)

#define    NONE_MASTER "master_none@temple@stonehill"
#define WARRIOR_MASTER "master_war@temple@stonehill"
#define   THIEF_MASTER "master_thief@temple@stonehill"
#define  RANGER_MASTER "master_ranger@temple@stonehill"
#define    MAGE_MASTER "master_mage@temple@stonehill"
#define   DRUID_MASTER "master_druid@temple@stonehill"
#define  PRIEST_MASTER "master_priest@temple@stonehill"

struct gskills
{
	const int   skill;
	const int   level;
	const char *teacher;
};

struct guild
{
	const char    *m_name;    /* name of the guild */
	const gskills *m_skills;  /* skills thought at this guild */
	Monster       *m_master;  /* guild master */
};

extern struct guild guilds[N_GUILDS];

/*******************************************************************/
#endif /* __GUILDS_H__ */
