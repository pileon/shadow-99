#ifndef __LOADER_H__
#define __LOADER_H__
/********************************************************************
* File: loader.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

/*******************************************************************/

class Area;
class Zone;
class Object;

class Loader
{
public:
	Loader(const char *path, const char *file);
	~Loader();

	bool load(const bool plr = false);

	Object   *get_object(void) const { return m_object; }
	Object   *get_player(void) const { return m_player; }
	const int get_count (void) const { return m_count ; }

private:
	char   *m_path;
	char   *m_file;
	char   *m_dir;
	Area   *m_area;    /* current area we are loading */
	Zone   *m_zone;    /* current zone we are in */
	Object *m_object;  /* current object being loaded */
	bool    m_isplr;   /* true when loading a player object */
	Object *m_player;
	int     m_count;   /* number of objects loaded */

public:
	/* these methods are to be called by the parser functions in parser.cc */
	const bool add_object  (const char *type, const char      *name );
	const bool set_property(const char *name, const char      *value);
	const bool set_property(const char *name, const int        value);
	const bool set_propref (const char *name, const char      *value);
	const bool set_propbool(const char *name, const bool       value);
	const bool set_property(const char *name, const tupple_t  *value);
	const bool set_property(const char *name, const tripple_t *value);
};

/*******************************************************************/
#endif /* __LOADER_H__ */
