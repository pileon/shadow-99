/********************************************************************
* File: boot.cc                                   Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "server.h"
#include "object.h"
#include "loader.h"
#include "area.h"
#include "zone.h"

/*******************************************************************/

/* this function is pretty straightforward, it allocates a chunk of memory,
 * opens a file, and reads the contents of that file into the allocated
 * memory
 */
static bool load_text(const char *fn, char **to)
{
	int len, rlen, fd;

	if (!fn || !to)
		return false;

	if ((len = file_size(fn)) < 0)
	{
		perror(fn);
		return false;
	}

	if ((*to = new char [len + 1]) == NULL)
		out_of_memory();

	if ((fd = open(fn, 0)) < 0)
	{
		perror(fn);
		return false;
	}

	rlen = read(fd, *to, len);
	if (rlen < 0)
	{
		perror(fn);
		return false;
	}
	else if (rlen < len)
	{
		/* ??? */
	}

	(*to)[rlen] = 0;  /* terminate text */

	(void) close(fd);

	return true;
}

bool Server::load_texts(void)
{
	if (!load_text(FILE_WELLCOME, &mudconfig.wellcome))
		return false;
	if (!load_text(FILE_WMOTD, &mudconfig.wmotd))
		return false;
	if (!load_text(FILE_MOTD, &mudconfig.motd))
		return false;
	if (!load_text(FILE_RULES, &mudconfig.rules))
		return false;
	if (!load_text(FILE_BGINFO, &mudconfig.bginfo))
		return false;

	return true;
}

void Server::free_texts(void)
{
	if (mudconfig.wellcome)
		delete [] mudconfig.wellcome;
	if (mudconfig.wmotd)
		delete [] mudconfig.wmotd;
	if (mudconfig.motd)
		delete [] mudconfig.motd;
	if (mudconfig.rules)
		delete [] mudconfig.rules;
	if (mudconfig.bginfo)
		delete [] mudconfig.bginfo;
}

/*******************************************************************/

bool Server::boot_world(void)
{
	bool init_factories(void);

	FILE *idxfl;
	char  path[256];
	char  line[256];
	char *p;
	int   ln = 0, nerrs = 0, nare = 0;

	log("Loading areas...\n");

	if (!init_factories())
		return false;

	if ((idxfl = fopen(mudconfig.areaindex, "r")) == NULL)
	{
		log("Could not open area index file: %s\n", strerror(errno));
		return false;
	}

	for ( ; ; )
	{
		ln++;
		if (fgets(line, sizeof(line), idxfl) == NULL)
		{
			if (feof(idxfl))
				break;
			else
			{
				log("Error reading area index file: %s\n", strerror(errno));
				fclose(idxfl);
				return false;
			}
		}

		/* end the line where apropriate */
		for (p = line; *p; p++)
		{
			if (*p == '#' || isspace(*p))
			{
				*p = 0;
				break;
			}
		}

		/* a comment or an empty line? */
		if (!*line || *line == '#' || *line == '\n')
			continue;

		snprintf(path, sizeof(path), "world/%s", line);
		if (!boot_area(path, line))
			nerrs++;
		else
			nare++;
	}

	fclose(idxfl);

	if (nare && nerrs)
	{
		log("   %d area%s loaded OK (%d failed)\n",
			nare, nare > 1 ? "s" : "", nerrs);
	}
	else if (nare && !nerrs)
	{
		if (nare > 1)
			log("   All %d areas loaded OK\n", nare);
		else
			log("   The only area loaded OK\n");
	}
	else
	{
		if (nare == 0)
			log("   No areas could be loaded!\n");
		else
			log("   Not all areas could be loaded\n");
	}

	return (nare ? true : false);
}

/* resolve references to objects in the world files */
bool Server::resolve_world(void)
{
	Object *o;
	Area   *a;
	Zone   *z;

	log("Resolving world file references\n");
	for (o = server.m_objects; o; o = o->m_nexto)
	{
		if (!o->resolve())
			return false;
	}

	log("Resolving zone reset references\n");
	for (a = server.m_areas; a; a = a->m_nexta)
	{
		for (z = a->m_zones; z; z = z->m_nextz)
		{
			if (!z->resolve_resets())
				return false;
		}
	}

	log("Resolving configurable locations\n");
	mudconfig.mstart = (Location *) find_object(mudconfig.room_mstart);
	mudconfig.wstart = (Location *) find_object(mudconfig.room_wstart);
	mudconfig.sanc   = (Location *) find_object(mudconfig.room_sanc  );

	if (!mudconfig.mstart || !mudconfig.mstart->is_location())
	{
		log("No player entry location exists!\n");
		return false;
	}
	if (!mudconfig.wstart || !mudconfig.wstart->is_location())
		mudconfig.wstart = mudconfig.mstart;
	if (!mudconfig.sanc || !mudconfig.sanc->is_location())
		mudconfig.sanc = mudconfig.mstart;

	log("Reseting zones\n");
	for (a = server.m_areas; a; a = a->m_nexta)
	{
		for (z = a->m_zones; z; z = z->m_nextz)
			z->reset_zone();
	}

	return true;
}

/* resolve internal references to objects */
bool Server::resolve_other(void)
{
	return true;
}

/*******************************************************************/

bool Server::boot_area(const char *path, const char *file)
{
	Loader *loader;
	bool    rc;

	if (file_exists(path) <= 0)
	{
		log("  %s: File does not exist or is not a file\n", file);
		return false;
	}

	if ((loader = new Loader(path, file)) == NULL)
		out_of_memory();

	rc = loader->load();

	delete loader;

	return rc;
}

/*******************************************************************/
