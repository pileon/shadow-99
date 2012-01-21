/********************************************************************
* File: factory.cc                                Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "object.h"
#include "area.h"
#include "zone.h"
#include "character.h"
#include "player.h"
#include "location.h"
#include "monster.h"
#include "item.h"

/*******************************************************************/

class Factory
{
public:
	virtual Object *create_object(void) const = 0;
};

struct plant
{
	Factory *plant;
	char    *name;
	plant   *next;
};

static struct plant *plants = NULL;

static bool add_factory(const char *n, Factory *f)
{
	plant *p = new plant;
	if (p == NULL)
		out_of_memory();

	p->plant = f;
	p->name  = STRDUP(n);
	p->next  = plants;
	plants   = p;

	return true;
}

static Factory *find_factory(const char *name)
{
	plant *p;

	if (!name)
		return NULL;

	for (p = plants; p; p = p->next)
	{
		if (p->name && *p->name == *name && strcasecmp(p->name, name) == 0)
			return p->plant;
	}

	return NULL;
}

/*******************************************************************/

class AreaFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Area; }
};

class ZoneFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Zone; }
};

class PlayerFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Player; }
};

class WizardFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Wizard; }
};

class ArchwizFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Archwiz; }
};

class LocationFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Location; }
};

class MonsterFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Monster; }
};

class ItemFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Item; }
};

class KeyFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Key; }
};

class DoorFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Door; }
};

class LockFactory : public Factory
{
public:
	virtual Object *create_object(void) const { return new Lock; }
};

/*******************************************************************/

bool init_factories(void)
{
	add_factory("area"    , new     AreaFactory);
	add_factory("zone"    , new     ZoneFactory);
	add_factory("player"  , new   PlayerFactory);
	add_factory("wizard"  , new   WizardFactory);
	add_factory("archwiz" , new  ArchwizFactory);
	add_factory("location", new LocationFactory);
	add_factory("monster" , new  MonsterFactory);
	add_factory("item"    , new     ItemFactory);
	add_factory("key"     , new      KeyFactory);
	add_factory("door"    , new     DoorFactory);
	add_factory("lock"    , new     LockFactory);

	return true;
}

Object *new_object(const char *name)
{
	Factory *f;

	if ((f = find_factory(name)) != NULL)
		return f->create_object();
	else
		return NULL;
}

/*******************************************************************/
