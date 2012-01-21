#ifndef __ITEM_H__
#define __ITEM_H__
/********************************************************************
* File: item.h                                    Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "object.h"

/*******************************************************************/

const bool isname(const char *str, const char *namelist);

class Item : public Object
{
public:
	Item();
	virtual ~Item();
	virtual void init(void) { Object::init(); }

	virtual const bool is_item   (void) const { return true; }
	virtual const bool is_equiped(void) const { return m_equiped; }
	virtual const bool is_name(const char *names)
		{ return ::isname(names, get_alias()); }
	virtual void describe(Character *to, const descr_t typ = DESC_SHORT);

protected:
	bool m_equiped;

	virtual void on_clone(Object *clone) const { *(Item *) clone = *this; }
};

class Key : public Item
{
public:
	Key() : Item() { }
	virtual ~Key() {}
	virtual void init(void) { Item::init(); }

	virtual const bool is_key(void) const { return true; }
};

/*******************************************************************/
#endif /* __ITEM_H__ */
