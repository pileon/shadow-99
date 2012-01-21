/********************************************************************
* File: area.cc                                   Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "object.h"
#include "area.h"
#include "zone.h"

/*******************************************************************/

Area::Area()
	: Object()
{
	m_nexta  = NULL;
	m_author = NULL;
	m_rlname = NULL;
	m_email  = NULL;
	m_zones  = NULL;

	add_property("author", &m_author);
	add_property("rlname", &m_rlname);
	add_property("email" , &m_email );
}

Area::~Area()
{
	if (m_author)
		delete [] m_author;
	if (m_rlname)
		delete [] m_rlname;
	if (m_email)
		delete [] m_email;

	REMOVE_FROM_LIST(this, server.m_areas, m_nexta, Area);
}

void Area::init(void)
{
	Object::init();
	m_nexta = server.m_areas;
	server.m_areas = this;
}

/*******************************************************************/

Zone *Area::find_zone(register const char *name) const
{
	register Zone *z;

	for (z = m_zones; z; z = z->m_nextz)
	{
		if (*name == *z->m_name && strcasecmp(name, z->m_name) == 0)
			return z;
	}

	return NULL;
}

Area *find_area(register const char *name)
{
	register Area *a;

	for (a = server.m_areas; a; a = a->m_nexta)
	{
		if (*name == *a->m_name && strcasecmp(name, a->m_name) == 0)
			return a;
	}

	return NULL;
}

/*******************************************************************/

bool Area::save(FILE *fl)
{
	if (!Object::save(fl))
		return false;

	fprintf(fl, "\t/* Area properties */\n");
	fprintf(fl, "\tauthor = \"%s\";\n", NULLSTR(m_author));
	fprintf(fl, "\trlname = \"%s\";\n", NULLSTR(m_rlname));
	fprintf(fl, "\temail  = \"%s\";\n", NULLSTR(m_email ));
	fprintf(fl, "\n");

	return true;
}

/*******************************************************************/
