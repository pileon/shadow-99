/********************************************************************
* File: zone.cc                                   Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "object.h"
#include "zone.h"
#include "area.h"

/*******************************************************************/

void yyerror(void);
void yyerror(const char *fmt, ...);

void reset_place(Object *o, Property *p, value_t *v);
void reset_give(Object *o, Property *p, value_t *v);
void reset_equip(Object *o, Property *p, value_t *v);

/*******************************************************************/

Zone::Zone()
	: Object()
{
	m_nextz   = NULL;
	m_objects = NULL;
	m_poptime = 0;
	m_resets  = NULL;
	m_lastm   = NULL;

	Property *p;

	add_property("poptime", &m_poptime);
	p = add_property("place", reset_place);
	if (p != NULL)
		p->m_multi = true;
	p = add_property("give", reset_give);
	if (p != NULL)
		p->m_multi = true;
	p = add_property("equip", reset_equip);
	if (p != NULL)
		p->m_multi = true;
}

Zone::~Zone()
{
	zreset *r, *n;
	for (r = m_resets; r; r = n)
	{
		n = r->next;
		delete r;
	}
}

void Zone::init(void)
{
	Object::init();
}

/*******************************************************************/

Object *Zone::find_object(const char *name) const
{
	Object *o;

    for (o = m_objects; o; o = o->m_next)
    {
		if (o->m_zone == this && *name == *o->m_name &&
			strcasecmp(name, o->m_name) == 0)
		{
			return o;
		}
	}
	return NULL;
}

bool Zone::save(FILE *fl)
{
	if (!Object::save(fl))
		return false;

	return true;
}

/*******************************************************************/

bool Zone::resolve_resets(void)
{
	zreset *r;
	bool    rc = true;

	for (r = m_resets; r && rc; r = r->next)
	{
		if (r->on != NULL && (r->obj = ::find_object(r->on)) == NULL)
		{
			log("   %s: No such object in zone reset: %s\n",
				get_name(), r->on);
			rc = false;
		}
		if (r->ln != NULL && (r->loc = ::find_object(r->ln)) == NULL)
		{
			log("   %s: No such object in zone reset: %s\n",
				get_name(), r->ln);
			rc = false;
		}
	}

	m_nextres = m_poptime;

	return rc;
}

void Zone::on_tick(void)
{
	if (--m_nextres)
		return;  /*not time for zone reset yet */

	/* reseting zone */
	reset_zone();
	m_nextres = m_poptime;
}

void Zone::reset_zone(void)
{
	syslog(LOG_INFO, "Zone reset: %s", get_long());

	zreset *r;
	int     n;

	m_lastm = NULL;

	for (r = m_resets; r; r = r->next)
	{
		/* count the current number of objects in the zone */
		if ((n = count_objects(r->obj)) < 0)
			continue;

		if (n >= r->max)
		{
			/* to many objects, skip repop */
			if (r->type == zreset::ZR_PLACE && r->obj->is_monster())
				m_lastm = NULL;
			continue;
		}

		if (r->type == zreset::ZR_PLACE)
			place(r);
		else if (r->type == zreset::ZR_GIVE)
			give(r);
		else if (r->type == zreset::ZR_EQUIP)
			equip(r);
		else
			syslog(LOG_SYSERR, "Unknown reset type: %d", r->type);
	}
}

/* TODO:  place some of the checks in resolve_resets()? */

void Zone::place(zreset *r)
{
	if (!r->obj)
	{
		syslog(LOG_SYSERR, "No object to place");
		return;
	}
	if (!r->loc)
	{
		syslog(LOG_SYSERR, "Nowhere to place object: %s",
			   r->obj->get_name());
		return;
	}
	if (!r->loc->is_location())
	{
		syslog(LOG_SYSERR, "Destination not a location: %s",
			   r->obj->get_name());
		return;
	}
	if (!r->obj->is_item() && !r->obj->is_monster())
	{
		syslog(LOG_SYSERR, "Object to place not an item or a monster");
		syslog(LOG_SYSERR, "(Object is %s)", r->obj->get_name());
		return;
	}

	Object *o = clone_object(r->obj);
	if (o == NULL)
	{
		syslog(LOG_SYSERR, "Could not clone object: %s",
			   r->obj->get_name());
		return;
	}

	if (!o->move(r->loc))
	{
		syslog(LOG_SYSERR, "Could not move object to location");
		syslog(LOG_SYSERR, "(Object: %s, Location: %s)",
			   o->get_name(), r->loc->get_name());
		return;
	}

	if (o->is_monster())
		m_lastm = o;
}

void Zone::give(zreset *r)
{
	if (!r->obj)
	{
		syslog(LOG_SYSERR, "No object to to give");
		return;
	}
	if (!r->obj->is_item())
	{
		syslog(LOG_SYSERR, "Object is not an item: %s", r->obj->get_name());
		return;
	}
	if (!m_lastm)
		return;  /* fail silently */

	Object *o = clone_object(r->obj);
	if (o == NULL)
	{
		syslog(LOG_SYSERR, "Could not clone object: %s",
			   r->obj->get_name());
		return;
	}

	if (!o->move(m_lastm))
	{
		syslog(LOG_SYSERR, "Could not give monster item");
		syslog(LOG_SYSERR, "(Item: %s, Monster: %s)",
			   o->get_name(), m_lastm->get_name());
		return;
	}
}

void Zone::equip(zreset *r)
{
	/* TODO:  equip, not give! */
	give(r);
}

void Zone::add_reset(zreset *r)
{
	if (!r)
		return;

	if (m_resets == NULL)
	{
		m_resets = r;
		return;
	}

	/* find the end of the resets */
	zreset *zr;
	for (zr = m_resets; zr && zr->next; zr = zr->next)
		;
	if (zr && !zr->next)
		zr->next = r;
}

int Zone::count_objects(Object *o) const
{
	if (!o)
		return -1;

	int     n = 0;
	Object *t = NULL;

	for (t = server.m_objects; t; t = t->m_nexto)
	{
		if (t->m_master == o)
			n++;
	}

	return n;
}

/*******************************************************************/

static zreset *tupple(value_t *v)
{
	tupple_t *t = (tupple_t *) v->value.t;
	if (t->v1.type != value_t::T_INT ||
		t->v2.type != value_t::T_REF)
	{
		yyerror("(Illegal types in reset)");
		return NULL;
	}

	if (t->v1.value.i < 0)
	{
		yyerror("(negative max count in reset)");
		return NULL;
	}

	/*
	printf("tupple(): t->v1.value.i = %d\n"
		   "          t->v2.value.s = %s\n",
		 t->v1.value.i, t->v2.value.s);
	*/

	zreset *r = new zreset;
	if (r == NULL)
		out_of_memory();

	r->max = t->v1.value.i;
	r->on  = STRDUP(t->v2.value.s);

	return r;
}

static zreset *tripple(value_t *v)
{
	tripple_t *t = (tripple_t *) v->value.t;
	if (t->v1.type != value_t::T_INT ||
		t->v2.type != value_t::T_REF ||
		t->v3.type != value_t::T_REF)
	{
		yyerror("(Illegal types in reset)");
		return NULL;
	}

	if (t->v1.value.i < 0)
	{
		yyerror("(negative max count in reset)");
		return NULL;
	}

	/*
	printf("tripple(): t->v1.value.i = %d\n"
		   "           t->v2.value.s = %s\n"
		   "           t->v3.value.s = %s\n",
		   t->v1.value.i, t->v2.value.s, t->v3.value.s);
	*/

	zreset *r = new zreset;
	if (r == NULL)
		out_of_memory();

	r->max = t->v1.value.i;
	r->on  = STRDUP(t->v2.value.s);
	r->ln  = STRDUP(t->v3.value.s);

	return r;
}

void reset_place(Object *o, Property *p, value_t *v)
{
	if (!o || !p || !v || !o->is_zone())
		return;

	if (strcasecmp(p->m_name, "place") != 0)
		return;

	if (v->type == value_t::T_TUP)
	{
		reset_give(o, p, v);
		return;
	}

	if (v->type != value_t::T_TRI || !v->value.t)
		return;

	Zone   *z = (Zone *) o;
	zreset *r = NULL;

	if ((r = tripple(v)) == NULL)
		return;
	r->type     = zreset::ZR_PLACE;
	z->add_reset(r);
}

void reset_give(Object *o, Property *p, value_t *v)
{
	if (!o || !p || !v || !o->is_zone())
		return;

	if (strcasecmp(p->m_name, "give") != 0)
		return;

	if (v->type != value_t::T_TUP || !v->value.t)
		return;

	Zone   *z = (Zone *) o;
	zreset *r = NULL;

	if ((r = tupple(v)) == NULL)
		return;
	r->type     = zreset::ZR_GIVE;
	z->add_reset(r);
}

void reset_equip(Object *o, Property *p, value_t *v)
{
	if (!o || !p || !v || !o->is_zone())
		return;

	if (strcasecmp(p->m_name, "equip") != 0)
		return;

	if (v->type != value_t::T_TRI || !v->value.t)
		return;

	Zone      *z = (Zone *) o;
	zreset    *r = NULL;
	tripple_t *t = (tripple_t *) v->value.t;

	if (t->v3.type != value_t::T_INT)
	{
		yyerror("(Illegal types in reset)");
		return;
	}

	if ((r = tupple(v)) == NULL)
		return;
	r->pos      = t->v3.value.i;
	r->type     = zreset::ZR_EQUIP;
	z->add_reset(r);
}

/*******************************************************************/
