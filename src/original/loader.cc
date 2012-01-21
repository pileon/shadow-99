/********************************************************************
* File: loader.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "loader.h"
#include "object.h"
#include "area.h"
#include "zone.h"

/*******************************************************************/

bool lex_pushfile(const char *fn, const char *dn, const char *dir);
void lex_popall(void);
int  yyparse(Loader *loader);
const int lex_getnerrs(void);
void lex_setnerrs(const int n);

#ifdef __GNUC__
void yyerror(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
#else
void yyerror(const char *fmt, ...);
#endif
void yyerror(void);

/*******************************************************************/

Loader::Loader(const char *path, const char *file)
{
	m_path = m_file = m_dir = NULL;
	if (path)
		m_path = STRDUP(path);
	if (file)
		m_file = STRDUP(file);
	m_area   = NULL;
	m_zone   = NULL;
	m_object = NULL;
	m_player = NULL;
	m_isplr  = false;
}

Loader::~Loader()
{
	if (m_dir)
		delete [] m_dir;
	if (m_file)
		delete [] m_file;
	if (m_file)
		delete [] m_path;
}

/*******************************************************************/

bool Loader::load(const bool isplr /* = false */)
{
	char *p;
	bool  rc;

	if (!m_path || !m_file)
		return false;

	m_isplr = isplr;

	/* try to find out the directory the file is in */
	if ((p = strrchr(m_path, '/')) == NULL)
	{
		log("   Can not find directory %s is in\n", m_file);
		return false;
	}
	if ((m_dir = new char [(p - m_path) + 1]) == NULL)
		out_of_memory();
	memcpy(m_dir, m_path, p - m_path);
	*(m_dir + (p - m_path)) = 0;  /* terminate string */

	lex_setnerrs(0);
	if (!lex_pushfile(m_path, m_file, m_dir))
		return false;

	if (yyparse(this))
		rc = false;
	else
		rc = true;

	lex_popall();

	return rc && !lex_getnerrs();
}

/*******************************************************************/

/* this function is called when the parser want's to create a new object */
const bool Loader::add_object(const char *type, const char *name)
{
	Object *new_object(const char *name);
	const char *lex_getfn(void);
	const int   lex_getln(void);

	if ((m_object = new_object(type)) == NULL)
	{
		yyerror("No such object type exists: %s", type);
		return false;
	}

	if (m_isplr)
	{
		if (m_object->is_player())
		{
			if (m_player)
			{
				yyerror("Only one player per player file allowed");
				return false;
			}

			m_player = m_object;
		}
		else
		{
			if (!m_player)
			{
				yyerror("Player objects must be first in player files");
				return false;
			}

			if (!m_object->is_item())
			{
				yyerror("Only items allowed in player files");
				return false;
			}

			/* link the item into the players inventory */
			m_object->m_sibling  = m_player->m_children;
			m_player->m_children = m_object;
		}
	}
	else
	{
		if (!m_area && !m_object->is_area())
		{
			yyerror("Objects can't be created outside an area");
			return false;
		}
		if (!m_zone && !m_object->is_zone() && !m_object->is_area())
		{
			yyerror("Objects can't be created outside a zone");
			return false;
		}

		if (m_object->is_area())
			m_area = (Area *) m_object;
		if (m_object->is_zone())
			m_zone = (Zone *) m_object;

		if (!m_object->is_area())
			m_object->m_area = m_area;
		if (!m_object->is_zone() && !m_object->is_area())
			m_object->m_zone = m_zone;

		if (m_object->is_zone())
		{
			/* insert the zone into the areas zone list */
			Zone *z = (Zone *) m_object;
			z->m_nextz       = m_area->m_zones;
			m_area->m_zones  = z;
		}

		if (!m_object->is_area() && !m_object->is_zone())
		{
			/* link the object into the zone */
			m_object->m_next  = m_zone->m_objects;
			m_zone->m_objects = m_object;
		}
	}

	/* initialise the object */
	m_object->m_name = STRDUP(name);
	m_object->m_file = STRDUP(lex_getfn());
	m_object->m_line = lex_getln();
	m_object->init();

	//printf("new %s object '%s' (%s %d)\n",
	//	   type, m_object->m_name, m_object->m_file, m_object->m_line);
	return true;
}

/* TODO:  isn't there a better way of doing this? */

/* add a string property to the current object */
const bool Loader::set_property(const char *name, const char *value)
{
	Property *p;

	if ((p = m_object->find_property(name)) == NULL)
		yyerror("No such property exists: %s", name);
	else if (p->m_type != Property::PT_STRING && p->m_type != Property::PT_FUN)
		yyerror("%s is not a string property", name);
	else if (!p->m_multi && p->m_count)
		yyerror("Property %s is allready set", name);
	else
		return m_object->set_property(p, value);
	return false;
}

/* add a number property to the current object */
const bool Loader::set_property(const char *name, const int value)
{
	Property *p;

	if ((p = m_object->find_property(name)) == NULL)
		yyerror("No such property exists: %s", name);
	else if (!p->m_multi && p->m_count)
		yyerror("Property %s is allready set", name);
	else
	{
		if (p->m_type == Property::PT_NUMBER || p->m_type == Property::PT_FUN)
			return m_object->set_property(p, value);
		else if (p->m_type == Property::PT_BOOL)
			return m_object->set_property(p, value ? true : false);
		else
			yyerror("%s is not a number property", name);
	}
	return false;
}

/* add a reference property to the current object */
const bool Loader::set_propref(const char *name, const char *value)
{
	Property *p;

	if ((p = m_object->find_property(name)) == NULL)
		yyerror("No such property exists: %s", name);
	else if (p->m_type != Property::PT_REF && p->m_type != Property::PT_FUN)
		yyerror("%s is not a reference property", name);
	else if (!p->m_multi && p->m_count)
		yyerror("Property %s is allready set", name);
	else
		return m_object->set_propref(p, value);
	return false;
}

/* add a boolean property to the current object */
const bool Loader::set_propbool(const char *name, const bool value)
{
	Property *p;

	if ((p = m_object->find_property(name)) == NULL)
		yyerror("No such property exists: %s", name);
	else if (p->m_type != Property::PT_BOOL &&
			 p->m_type != Property::PT_BFLAG &&
			 p->m_type != Property::PT_FUN)
		yyerror("%s is not a boolean property", name);
	else if (!p->m_multi && p->m_count)
		yyerror("Property %s is allready set", name);
	else
		return m_object->set_property(p, value);
	return false;
}

/* add a tupple property to the current object */
const bool Loader::set_property(const char *name, const tupple_t *value)
{
	Property *p;

	if ((p = m_object->find_property(name)) == NULL)
		yyerror("No such property exists: %s", name);
	else if (!p->m_multi && p->m_count)
		yyerror("Property %s is allready set", name);
	else if (p->m_type != Property::PT_TUP && p->m_type != Property::PT_FUN)
		yyerror("%s is not a tupple property", name);
	else
		return m_object->set_property(p, value);
	return false;
}

/* add a tripple property to the current object */
const bool Loader::set_property(const char *name, const tripple_t *value)
{
	Property *p;

	if ((p = m_object->find_property(name)) == NULL)
		yyerror("No such property exists: %s", name);
	else if (!p->m_multi && p->m_count)
		yyerror("Property %s is allready set", name);
	else if (p->m_type != Property::PT_TRI && p->m_type != Property::PT_FUN)
		yyerror("%s is not a tripple property", name);
	else
		return m_object->set_property(p, value);
	return false;
}

/*******************************************************************/
