/********************************************************************
* File: object.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "object.h"
#include "zone.h"
#include "area.h"
#include "character.h"

/*******************************************************************/

const char *lex_getfn(void);
const int   lex_getln(void);

/*******************************************************************/

Object::Object()
{
	m_nexto   = NULL;
	m_next    = NULL;
	m_children= NULL;
	m_sibling = NULL;
	m_parent  = NULL;
	m_name    = NULL;
	m_short   = NULL;
	m_long    = NULL;
	m_descr   = NULL;
	m_area    = NULL;
	m_zone    = NULL;
	m_file    = NULL;
	m_line    = 0;
	m_props   = NULL;
	m_clone   = false;
	m_alias   = NULL;
	m_master  = NULL;

	add_property("alias", &m_alias);
	add_property("short", &m_short);
	add_property("long" , &m_long );
	add_property("descr", &m_descr);
}

Object::~Object()
{
	if (!m_clone)
	{
		if (m_alias)
			delete [] m_alias;
		if (m_name)
			delete [] m_name;
		if (m_short)
			delete [] m_short;
		if (m_long)
			delete [] m_long;
		if (m_descr)
			delete [] m_descr;
		if (m_file)
			delete [] m_file;
	}

	if (!server.is_shutdown())
		REMOVE_FROM_LIST(this, server.m_objects, m_nexto, Object);
	if (m_parent)
		REMOVE_FROM_LIST(this, m_parent->m_children, m_sibling, Object);

	Object *c, *s;
	for (c = m_children; c; c = s)
	{
		s = c->m_sibling;
		c->m_parent = NULL;  /* so it doesn't remove itself from the list */
		delete c;
	}

	if (!m_clone)
		free_properties();
}

void Object::init(void)
{
	m_nexto = server.m_objects;
	server.m_objects = this;
}

/*******************************************************************/

/* TODO:  combine all add_property methods into one, that handles them all?
 *        then have inline stubs that call the bon one with the right args?
 */

Property *Object::add_property(const char *name, char **to)
{
	Property *p;

	if (!name || !to)
		return NULL;

	if ((p = new Property) == NULL)
		out_of_memory();

	p->m_name = STRDUP(name);
	p->m_type = Property::PT_STRING;
	p->m_to.m_string = to;

	p->m_next = m_props;
	m_props   = p;

	return p;
}

Property *Object::add_property(const char *name, int *to)
{
	Property *p;

	if (!name || !to)
		return NULL;

	if ((p = new Property) == NULL)
		out_of_memory();

	p->m_name = STRDUP(name);
	p->m_type = Property::PT_NUMBER;
	p->m_to.m_number = to;

	p->m_next = m_props;
	m_props   = p;

	return p;
}

Property *Object::add_property(const char *name, Object **to)
{
	Property *p;

	if (!name || !to)
		return NULL;

	if ((p = new Property) == NULL)
		out_of_memory();

	p->m_name = STRDUP(name);
	p->m_type = Property::PT_REF;
	p->m_to.m_ref = to;

	p->m_next = m_props;
	m_props   = p;

	return p;
}

Property *Object::add_property(const char *name, bool *to)
{
	Property *p;

	if (!name || !to)
		return NULL;

	if ((p = new Property) == NULL)
		out_of_memory();

	p->m_name = STRDUP(name);
	p->m_type = Property::PT_BOOL;
	p->m_to.m_number = (int *) to;

	p->m_next = m_props;
	m_props   = p;

	return p;
}

Property *Object::add_property(const char *name, int *to, const int bit)
{
	Property *p;

	if (!name || !to)
		return NULL;

	if ((p = new Property) == NULL)
		out_of_memory();

	p->m_name = STRDUP(name);
	p->m_type = Property::PT_BFLAG;
	p->m_to.m_bflag.m_flags = to;
	p->m_to.m_bflag.m_bit   = bit;

	p->m_next = m_props;
	m_props   = p;

	return p;
}

Property *Object::add_property(const char *name, propfun_t *fun)
{
	Property *p;

	if (!name || !fun)
		return NULL;

	if ((p = new Property) == NULL)
		out_of_memory();

	p->m_name = STRDUP(name);
	p->m_type = Property::PT_FUN;
	p->m_to.m_fun = fun;

	p->m_next = m_props;
	m_props   = p;

	return p;
}

/* these prop_is_*() functions should be inline in the header */
const bool Object::prop_is_string(const char *name)
{
	Property *p = find_property(name);
	return (p && p->m_type == Property::PT_STRING);
}

const bool Object::prop_is_number(const char *name)
{
	Property *p = find_property(name);
	return (p && p->m_type == Property::PT_NUMBER);
}

const bool Object::prop_is_ref(const char *name)
{
	Property *p = find_property(name);
	return (p && p->m_type == Property::PT_REF);
}

const bool Object::prop_is_bool(const char *name)
{
	Property *p = find_property(name);
	return (p && p->m_type == Property::PT_BOOL);
}

const bool Object::prop_is_flag(const char *name)
{
	Property *p = find_property(name);
	return (p && p->m_type == Property::PT_FLAG);
}

const bool Object::prop_is_bflag(const char *name)
{
	Property *p = find_property(name);
	return (p && p->m_type == Property::PT_BFLAG);
}

const bool Object::set_property(Property *p, const char *v)
{
	if (!p || !v)
		return false;

	if (p->m_type == Property::PT_FUN)
	{
		value_t val;

		val.type    = value_s::T_STR;
		val.value.s = STRDUP(v);
		if (p->m_to.m_fun)
			p->m_to.m_fun(this, p, &val);
		else
			delete [] val.value.s;
	}
	else if (p->m_type == Property::PT_STRING)
		*p->m_to.m_string = STRDUP(v);
	else
		return true;

	p->m_count++;
	if (p->m_file)
		delete [] p->m_file;
	p->m_file = STRDUP(lex_getfn());
	p->m_line = lex_getln();

	return true;
}

const bool Object::set_property(Property *p, const int v)
{
	if (!p)
		return false;

	if (p->m_type == Property::PT_FUN)
	{
		value_t val;

		val.type    = value_s::T_INT;
		val.value.i = v;
		if (p->m_to.m_fun)
			p->m_to.m_fun(this, p, &val);
	}
	else if (p->m_type == Property::PT_NUMBER)
		*p->m_to.m_number = v;
	else if (p->m_type == Property::PT_BOOL)
		*p->m_to.m_number = v;
	else if (p->m_type == Property::PT_BFLAG)
	{
		if (v)
			*p->m_to.m_bflag.m_flags |= p->m_to.m_bflag.m_bit;
		else
			*p->m_to.m_bflag.m_flags &= ~p->m_to.m_bflag.m_bit;
	}
	else
		return false;

	p->m_count++;
	if (p->m_file)
		delete [] p->m_file;
	p->m_file = STRDUP(lex_getfn());
	p->m_line = lex_getln();

	return true;
}

const bool Object::set_propref(Property *p, const char *v)
{
	if (!p || !v)
		return false;

	if (p->m_type == Property::PT_FUN)
	{
		value_t val;

		val.type    = value_s::T_REF;
		val.value.s = STRDUP(v);
		if (p->m_to.m_fun)
			p->m_to.m_fun(this, p, &val);
		else
			delete [] val.value.s;
	}
	else if (p->m_type != Property::PT_REF)
		return false;
	else
		p->m_temp = STRDUP(v);

	p->m_count++;
	if (p->m_file)
		delete [] p->m_file;
	p->m_file = STRDUP(lex_getfn());
	p->m_line = lex_getln();

	return true;
}

const bool Object::set_property(Property *p, const tupple_t *v)
{
	if (!p || !v)
		return false;

	if (p->m_type == Property::PT_FUN)
	{
		value_t val;

		val.type    = value_s::T_TUP;
		val.value.t = (tupple_t *) v;
		if (p->m_to.m_fun)
			p->m_to.m_fun(this, p, &val);
	}
	else if (p->m_type != Property::PT_TUP)
		return false;
	else
		;  /* what to do here? */

	p->m_count++;
	if (p->m_file)
		delete [] p->m_file;
	p->m_file = STRDUP(lex_getfn());
	p->m_line = lex_getln();

	return true;
}

const bool Object::set_property(Property *p, const tripple_t *v)
{
	if (!p || !v)
		return false;

	if (p->m_type == Property::PT_FUN)
	{
		value_t val;

		val.type    = value_s::T_TRI;
		val.value.t = (tripple_t *) v;
		if (p->m_to.m_fun)
			p->m_to.m_fun(this, p, &val);
	}
	else if (p->m_type != Property::PT_TRI)
		return false;
	else
		;  /* what to do here? */

	p->m_count++;
	p->m_file = STRDUP(lex_getfn());
	p->m_line = lex_getln();

	return true;
}

Property *Object::find_property(const char *name) const
{
	Property *p;

	if (!name)
		return NULL;

	for (p = m_props; p; p = p->m_next)
	{
		if (*name == *p->m_name && strcasecmp(name, p->m_name) == 0)
			return p;
	}
	return NULL;
}

void Object::free_properties(void)
{
	Property *p, *np;

	for (p = m_props; p; p = np)
	{
		np = p->m_next;
		delete p;
	}

	m_props = NULL;
}

/*******************************************************************/

const bool Object::save(FILE *fl)
{
	fprintf(fl, "\t/* Object properties */\n");
	fprintf(fl, "\talias = \"%s\";\n", NULLSTR(m_alias));
	fprintf(fl, "\tshort = \"%s\";\n", NULLSTR(m_short));
	fprintf(fl, "\tlong  = \"%s\";\n", NULLSTR(m_long ));
	fprintf(fl, "\tdescr = \"%s\";\n", NULLSTR(m_descr));
	fprintf(fl, "\n");
	return true;
}

const char *Object::fullname(const Object *o) const
{
	static char name[PATH_MAX + 1];

	if (!o)
		return NULL;

	if (m_zone == o->m_zone)
		strcpy(name, o->m_name);
	else if (m_area == o->m_area)
		snprintf(name, sizeof(name), "%s@%s", o->m_name, o->m_zone->m_name);
	else
	{
		snprintf(name, sizeof(name), "%s@%s@%s",
				 o->m_name, o->m_zone->m_name, o->m_area->m_name);
	}

	return name;
}

const char *Object::fullname(void) const
{
	static char name[PATH_MAX + 1];

	snprintf(name, sizeof(name), "%s@%s@%s",
			 m_name, m_zone->m_name, m_area->m_name);

	return name;
}

const bool Object::move(Object *to)
{
	Object *oldparent = m_parent;

	/* first unlink this object from it current parent */
	if(m_parent)
	{
		if (!m_parent->rem_child(this))
			return false;  /* movement not allowed by parent */
	}

	if (!to->add_child(this, oldparent))
	{
		/* the new parent doesn't want a new object, relink it to this again */
		m_parent  = oldparent;
		m_sibling = m_parent->m_children;
		m_parent->m_children = this;
		return false;
	}

	return true;
}

const bool Object::rem_child(Object *child)
{
	Object *o = NULL;

	/* first remove it from the sibling list */
	if (m_children == child)
		m_children = child->m_sibling;
	else
	{
		for (o = m_children; o; o = o->m_sibling)
		{
			if (o->m_sibling == child)
			{
				o->m_sibling = child->m_sibling;
				break;
			}
		}

		if (o == NULL)
		{
			syslog(LOG_SYSERR, "Child not found in parent");
			return false;
		}
	}

	/* then zero all the childs links */
	child->m_parent  = NULL;
	child->m_sibling = NULL;

	return true;
}

const bool Object::add_child(Object *child, Object *oldparen /* = NULL */)
{
	/* add a new child to this object */
	child->m_parent  = this;
	child->m_sibling = m_children;
	m_children = child;
	return true;
}

void Object::describe(Character *to, const descr_t typ /* = DESC_SHORT */)
{
	to->write("a thing\r\n");
}

/* this function is called when an object is "created", like on zone resets
 * if it returns false, the object is destroyed
 */
const bool Object::on_create(void)
{
	return true;
}

/*******************************************************************/

const bool Object::resolve(void)
{
	Property *p;
	Object   *o;

	for (p = m_props; p; p = p->m_next)
	{
		if (p->m_type == Property::PT_REF && p->m_count && p->m_temp)
		{
			if ((o = resolve(p->m_temp, p)) != NULL)
			{
				*p->m_to.m_ref = o;
				on_resolve(o, p);
			}
		}
	}

	return true;
}

// Object *Object::resolve(const char *name, Property *p) const
// {
// 	/* this function was a hell to get right! */

// 	char   *p1, *p2;
// 	Zone   *z = NULL;
// 	Area   *a = NULL;
// 	Object *o = NULL;
// 	char  zn[128], an[128], on[128];

// 	if ((p1 = strchr(name, '@')) != NULL)
// 	{
// 		if ((p2 = strchr(p1 + 1, '@')) != NULL)
// 		{
// 			strcpy(an, p2 + 1);  /* area name */
// 			memcpy(zn, p1 + 1, p2 - (p1 + 1));  /* zone name */
// 			*(zn + (p2 - (p1 + 1))) = 0;  /* terminate zone name */
// 			memcpy(on, name, p1 - name);  /* object name */
// 			*(on + (p1 - name)) = 0;  /* terminate object name */

// 			if ((a = find_area(an)) != NULL)
// 			{
// 				if ((z = a->find_zone(zn)) != NULL)
// 				{
// 					if ((o = z->find_object(on)) != NULL)
// 						return o;
// 					else
// 					{
// 						log("   Error resolving object\n\t\t(%s:%d)\n",
// 							p->m_file, p->m_line);
// 						log("   No such object found: %s\n", on);
// 					}
// 				}
// 				else
// 				{
// 					log("   Error resolving object\n\t\t(%s:%d)\n",
// 						p->m_file, p->m_line);
// 					log("   No such zone found: %s\n", zn);
// 				}
// 			}
// 			else
// 			{
// 				log("   Error resolving object\n\t\t(%s:%d)\n",
// 					p->m_file, p->m_line);
// 				log("   No such area found: %s\n", an);
// 			}
// 		}
// 		else
// 		{
// 			strcpy(zn, p1 + 1);  /* zone name */
// 			memcpy(on, name, p1 - name);  /* object name */
// 			*(on + (p1 - name)) = 0;  /* terminate object name */

// 			if (!m_area)
// 			{
// 				/* this must be an area object */
// 				if (!is_area())
// 				{
// 					log("   FATAL ERROR (%s:%d)\n", m_file, m_line);
// 					log("   Object have no area: %s\n", m_name);
// 					server.shutdown();
// 					return NULL;
// 				}
// 				/* what should I do here... *ponder* */
// 			}
// 			else
// 			{
// 				if ((z = m_area->find_zone(zn)) != NULL)
// 				{
// 					if ((o = z->find_object(on)) != NULL)
// 						return o;
// 					else
// 					{
// 						log("   Error resolving object\n\t\t(%s:%d)\n",
// 							p->m_file, p->m_line);
// 						log("   No such object found: %s\n", on);
// 					}
// 				}
// 				else
// 				{
// 					log("   Error resolving object\n\t\t(%s:%d)\n",
// 						p->m_file, p->m_line);
// 					log("   No such zone found: %s\n", zn);
// 				}
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if (!m_zone)
// 		{
// 			/* this must be either a zone or an area */
// 			if (is_zone())
// 			{
// 				/* it is a zone */
// 				/* what should I do here... *ponder* */
// 			}
// 			else if (is_area())
// 			{
// 				/* it is an area */
// 				/* what should I do here... *ponder* */
// 			}
// 			else
// 			{
// 				log("   FATAL ERROR (%s:%d)\n", m_file, m_line);
// 				log("   Object have no zone: %s\n", m_name);
// 				server.shutdown();
// 				return NULL;
// 			}
// 		}
// 		else
// 		{
// 			if ((o = m_zone->find_object(name)) != NULL)
// 				return o;
// 			else
// 			{
// 				log("   Error resolving object\n\t\t(%s:%d)\n",
// 					p->m_file, p->m_line);
// 				log("   No such object found: %s\n", name);
// 			}
// 		}
// 	}

// 	return NULL;
// }

Object *Object::resolve(const char *name, Property *p) const
{
	/* this function was a hell to get right!
	 * (the outcommented function above was the second try, this is the third)
	 */

	char *p1, *p2;
	char  zn[128], an[128], on[128];

	if ((p1 = strchr(name, '@')) != NULL)
	{
		if ((p2 = strchr(p1 + 1, '@')) != NULL)
		{
			strcpy(an, p2 + 1);  /* area name */
			memcpy(zn, p1 + 1, p2 - (p1 + 1));  /* zone name */
			*(zn + (p2 - (p1 + 1))) = 0;  /* terminate zone name */
			if (p1 == name)
				*on = 0;
			else
			{
				memcpy(on, name, p1 - name);  /* object name */
				*(on + (p1 - name)) = 0;  /* terminate object name */
			}
			return resolve(an, zn, on, p);
		}
		else
		{
			strcpy(zn, p1 + 1);  /* zone name */
			if (p1 == name)
				*on = 0;
			else
			{
				memcpy(on, name, p1 - name);  /* object name */
				*(on + (p1 - name)) = 0;  /* terminate object name */
			}
			return resolve(NULL, zn, on, p);
		}
	}
	else
		return resolve(NULL, NULL, name, p);
}

Object *Object::resolve(const char *an, const char *zn,
						const char *on, Property *p) const
{
	Area   *a = NULL;
	Zone   *z = NULL;
	Object *o = NULL;

	if (an != NULL)
	{
		if ((a = find_area(an)) == NULL)
		{
			log("   Error resolving object\n\t\t(%s:%d)\n",
				p->m_file, p->m_line);
			log("   No such area found: %s\n", an);
			return NULL;
		}
	}
	else
		a = m_area;

	if (zn != NULL && a)
	{
		if ((z = a->find_zone(zn)) == NULL)
		{
			log("   Error resolving object\n\t\t(%s:%d)\n",
				p->m_file, p->m_line);
			log("   No such zone found: %s\n", zn);
			return NULL;
		}
	}
	else
		z = m_zone;

	if (on != NULL && z)
	{
		if ((o = z->find_object(on)) == NULL)
		{
			log("   Error resolving object\n%28s(%s:%d)\n",
				"", p->m_file, p->m_line);
			log("   No such object found: %s\n", on);
			return NULL;
		}
	}
	else
	{
		/* we were either trying to find a zone or an area */
		o = z ? z : a;
	}

	return o;
}

/*******************************************************************/

/*******************************************************************/

static Object *find_object(const char *an, const char *zn, const char *on)
{
	Area   *a = NULL;
	Zone   *z = NULL;
	Object *o = NULL;

	if (an != NULL)
	{
		if ((a = find_area(an)) == NULL)
			return NULL;
	}

	if (zn != NULL)
	{
		if (a != NULL)
		{
			if ((z = a->find_zone(zn)) == NULL)
				return NULL;
		}
		else
		{
			/* take first zone with the same name */
			for (a = server.m_areas; a; a = a->m_nexta)
			{
				if ((z = a->find_zone(zn)) != NULL)
					break;
			}
		}
	}

	if (on != NULL)
	{
		if (z != NULL)
			o = z->find_object(on);
		else
		{
			/* take first object with same name */
			for (o = server.m_objects; o; o = o->m_nexto)
			{
				if (*o->m_name == *on && strcasecmp(o->m_name, on) == 0)
					break;
			}
		}
	}
	else
	{
		/* we were either trying to find a zone or an area */
		o = z ? z : a;
	}

	return o;
}

Object *find_object(const char *name)
{
	char *p1, *p2;
	char  zn[128], an[128], on[128];

	if (name == NULL || *name == 0)
		return NULL;

	if ((p1 = strchr(name, '@')) != NULL)
	{
		if ((p2 = strchr(p1 + 1, '@')) != NULL)
		{
			strcpy(an, p2 + 1);  /* area name */
			memcpy(zn, p1 + 1, p2 - (p1 + 1));  /* zone name */
			*(zn + (p2 - (p1 + 1))) = 0;  /* terminate zone name */
			if (p1 == name)
				*on = 0;
			else
			{
				memcpy(on, name, p1 - name);  /* object name */
				*(on + (p1 - name)) = 0;  /* terminate object name */
			}
			return find_object(an, zn, *on ? on : NULL);
		}
		else
		{
			strcpy(zn, p1 + 1);  /* zone name */
			if (p1 == name)
				*on = 0;
			else
			{
				memcpy(on, name, p1 - name);  /* object name */
				*(on + (p1 - name)) = 0;  /* terminate object name */
			}
			return find_object(NULL, zn, *on ? on : NULL);
		}
	}
	else
		return find_object(NULL, NULL, name);
}

/*******************************************************************/

Object *clone_object(const char *name)
{
	return clone_object(find_object(name));
}

Object *clone_object(const Object *master)
{
	Object *new_object(const char *name);

	if (master == NULL)
		return NULL;

	Object *clone = NULL;
	if (master->is_monster())
		clone = new_object("monster");
	else if (master->is_key())
		clone = new_object("key");
	else if (master->is_item())
		clone = new_object("item");
	else
	{
		syslog(LOG_SYSERR, "Cloning a not-cloneable object: %s",
			   master->get_name());
		return NULL;
	}

	if (clone != NULL)
	{
		master->on_clone(clone);

		clone->m_props    = NULL;
		clone->m_children = NULL;
		clone->m_sibling  = NULL;
		clone->m_parent   = NULL;
		clone->m_clone    = true;
		clone->m_master   = master;

		/* add the object to the servers list */
		clone->m_nexto   = server.m_objects;
		server.m_objects = clone;

		if (clone->is_char())
		{
			Character *ch = (Character *) clone;
			ch->m_nextc    = server.m_chars;
			server.m_chars = ch;
		}
	}

	return clone;
}

/*******************************************************************/
