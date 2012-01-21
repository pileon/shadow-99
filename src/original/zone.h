#ifndef __ZONE_H__
#define __ZONE_H__
/********************************************************************
* File: zone.h                                    Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "object.h"

/*******************************************************************/


struct zreset
{
	int     max;  /* max number of objects */
	char   *on;   /* name of object to load */
	char   *ln;   /* name of the object where the object should be loaded */
	int     pos;  /* position to equip items on monsters */
	Object *obj;  /* object to load */
	Object *loc;  /* parent to load on */
	zreset *next;
	enum
	{
		ZR_PLACE,
		ZR_GIVE,
		ZR_EQUIP,
	} type;

	zreset()
		{ on = ln = NULL; obj = loc = NULL; next = NULL; }
	~zreset()
		{
			if (on) delete [] on;
			if (ln) delete [] ln;
		}
};

class Zone : public Object
{
public:
	Zone();
	virtual ~Zone();

	virtual void init(void);
	virtual void on_tick(void);
	virtual const bool is_zone(void) const { return true; }
	virtual const bool save(FILE *fl);

	Object *find_object(const char *name) const;
	const bool resolve_resets(void);
	void reset_zone(void);

public:
	Zone   *m_nextz;    /* link for the areas m_zones list */
	Object *m_objects;  /* list of all objects belonging to this zone */

private:
	int     m_poptime;  /* how often the zone will repop things */
	int     m_nextres;  /* time (in minutes) until next reset */
	zreset *m_resets;   /* a list of all resets */
	Object *m_lastm;    /* last monster loaded */

	void place(zreset *r);
	void give (zreset *r);
	void equip(zreset *r);
	void add_reset(zreset *r);
	const int count_objects(Object *o) const;

	friend void reset_place(Object *o, Property *p, value_t *val);
	friend void reset_give (Object *o, Property *p, value_t *val);
	friend void reset_equip(Object *o, Property *p, value_t *val);
};

/*******************************************************************/
#endif /* __ZONE_H__ */
