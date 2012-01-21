#ifndef __AREA_H__
#define __AREA_H__
/********************************************************************
* File: area.h                                    Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

/*******************************************************************/

class Object;
class Zone;

class Area : public Object
{
public:
	Area();
	virtual ~Area();

	virtual void init(void);
	virtual const bool is_area(void) const { return true; }
	virtual const bool save(FILE *fl);

	Zone *find_zone(const char *name) const;

public:
	Area *m_nexta;   /* link for the servers m_areas list */
	char *m_author;  /* author MUD name */
	char *m_rlname;  /* author "real-life" name */
	char *m_email;   /* authors email address */
	Zone *m_zones;   /* list of all zones belonging to this area */

};

Area *find_area(const char *name);

/*******************************************************************/
#endif /* __AREA_H__ */
