#ifndef __OBJECT_H__
#define __OBJECT_H__
/********************************************************************
* File: object.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
*********************************************************************
*                                                                   *
* TODO                                                              *
* ----                                                              *
*                                                                   *
* o Make all properties dynamic?  I.e. all objects add property     *
*   information to a table, and that table is used on boot, and     *
*   after boot also contains the data ascosiated with the property? *
*   The way properties are currently handled (member variables in   *
*   the classes) is propably more intuitive and easier to understand*
*   for newbie coders.                                              *
*                                                                   *
********************************************************************/

#include <stdarg.h>

/*******************************************************************/

class Object;
class Property;

typedef struct value_s   value_t;
typedef struct tupple_s  tupple_t;
typedef struct tripple_s tripple_t;

struct value_s
{
	enum
	{
		T_BOOL,
		T_INT,
		T_FLT,
		T_STR,
		T_REF,
		T_TUP,
		T_TRI,
	} type;
	union
	{
		int   i;
		float f;
		char *s;
		void *t;
	} value;

	value_s()
		{ memset(&value, 0, sizeof(value)); }
	~value_s()
		{
			if ((type == T_REF || type == T_STR) && value.s)
				delete [] value.s;
		}
};

struct tupple_s
{
	value_t v1;
	value_t v2;
};

struct tripple_s
{
	value_t v1;
	value_t v2;
	value_t v3;
};

typedef void (propfun_t)(Object *o, Property *p, value_t *val);

class Property
{
public:
	Property ()
		{
			m_type  = PT_NONE;
			m_multi = false;
			m_count = 0;
			m_temp  = NULL;
			m_file  = NULL;
			memset(&m_to, 0, sizeof(m_to));
		}
	~Property()
		{
			if (m_name)
				delete [] m_name;
			if (m_file)
				delete [] m_file;
			if (m_temp && m_type == PT_REF)
				delete [] m_temp;
		}

	char *m_file;  /* name of file where property was assigned */
	int   m_line;  /* line where property was assigned */
	char *m_name;
	char *m_temp;  /* temporary storage for reference properties */
	bool  m_multi; /* true for proerties that can be set many times */
	int   m_count; /* the number of encounters in the object */
	enum
	{
		PT_NONE,
		PT_STRING,
		PT_NUMBER,
		PT_REF,
		PT_BOOL,
		PT_BFLAG,
		PT_FLAG,
		PT_FUN,
		PT_TUP,
		PT_TRI,
	} m_type;
	union
	{
		char   **m_string;
		int     *m_number;
		Object **m_ref;
		bool    *m_bool;
		struct
		{
			int *m_flags;  /* pointer to the flag field */
			int  m_bit;    /* flag to set */
		} m_bflag;
		propfun_t *m_fun;  /* function to call */
		tupple_t  *m_tup;
		tripple_t *m_tri;
	} m_to;
	Property *m_next;
};

/*******************************************************************/

class Area;
class Zone;
class Character;

typedef enum
{
	DESC_SHORT,
	DESC_LONG,
	DESC_DESC,
	DESC_READ,
} descr_t;

class Object
{
public:
	Object();
	virtual ~Object();

	virtual void init(void);

	virtual bool is_object  (void) const { return true ; }
	virtual bool is_char    (void) const { return false; }
	virtual bool is_player  (void) const { return false; }
	virtual bool is_wizard  (void) const { return false; }
	virtual bool is_archwiz (void) const { return false; }
	virtual bool is_monster (void) const { return false; }
	virtual bool is_area    (void) const { return false; }
	virtual bool is_zone    (void) const { return false; }
	virtual bool is_location(void) const { return false; }
	virtual bool is_lock    (void) const { return false; }
	virtual bool is_door    (void) const { return false; }
	virtual bool is_item    (void) const { return false; }
	virtual bool is_key     (void) const { return false; }
	virtual bool is_guild   (void) const { return false; }

	const char *get_alias(void) const { return m_alias; }
	const char *get_name (void) const { return m_name ; }
	const char *get_short(void) const { return m_short; }
	const char *get_long (void) const { return m_long ; }
	const char *get_descr(void) const { return m_descr; }

	virtual bool save(FILE *fl);

	virtual bool move(Object *to);
	virtual bool rem_child(Object *child);
	virtual bool add_child(Object *child, Object *oldparen = NULL);
	virtual bool on_create(void);

	virtual void describe(Character *to, const descr_t typ = DESC_SHORT);

	/* the before and after handlers, they are called before and after
	 * a player command, respectivly
	 * if a handler returns false, all other command execution is inhibited
	 */
	virtual bool before(Character *, int, char **, const int, const int)
		{ return true; }
	virtual bool after(Character *, int, char **, const int, const int)
		{ return true; }

	virtual bool is_name(char *) { return false; }

	virtual void on_tick (void) {}
	virtual void on_fight(void) {}

	bool is_clone(void) const { return m_clone; }

public:
	Object *m_nexto;    /* link for the servers m_objects list */
	Object *m_next;     /* link for the zone object list */
	Object *m_children; /* pointer to this objects first child */
	Object *m_sibling;  /* pointer to this objects next sibling */
	Object *m_parent;   /* pointer to this objects parent object */
	char   *m_alias;    /* a list of aliases for this object */
	char   *m_name;     /* all objects have a name,
						   it must be unique in each zone */
	char   *m_short;    /* short (display) name of the object */
	char   *m_long;     /* long (display) name of the object */
	char   *m_descr;    /* description of the object */
	Area   *m_area;     /* pointer to area this object belongs to */
	Zone   *m_zone;     /* pointer to zone this object belongs to */
	char   *m_file;     /* name of file where loaded from */
	int     m_line;     /* line number where this object was defined */

	const Object *m_master; /* master object this is cloned from */

public:
	/* these functions are used for the loading of world files */
	bool set_property (const char *name, const char *value)
		{ return set_property(find_property(name), value); }
	bool set_property (const char *name, const int   value)
		{ return set_property(find_property(name), value); }
	bool set_propref  (const char *name, const char *value)
		{ return set_property(find_property(name), value); }

	bool set_property (Property *prop, const char      *value);
	bool set_property (Property *prop, const int        value);
	bool set_propref  (Property *prop, const char      *value);
	bool set_property (Property *prop, const tupple_t  *value);
	bool set_property (Property *prop, const tripple_t *value);

	bool prop_is_string(const char *name);
	bool prop_is_number(const char *name);
	bool prop_is_ref   (const char *name);
	bool prop_is_bool  (const char *name);
	bool prop_is_flag  (const char *name);
	bool prop_is_bflag (const char *name);

	Property *find_property(const char *name) const;

	bool resolve(void);

	const char *fullname(const Object *o) const;
	const char *fullname(void) const;

	virtual void write(const char *, ...)
		{}
	virtual void vwrite(const char *, va_list)
		{}

protected:
	Property *add_property(const char *name, char   **to);
	Property *add_property(const char *name, int     *to);
	Property *add_property(const char *name, Object **to);
	Property *add_property(const char *name, bool    *to);
	Property *add_property(const char *name, int     *to, const int bit);
	Property *add_property(const char *name, propfun_t *fun);

	virtual void on_resolve(Object *, Property *)
		{}
	virtual void on_clone(Object *clone) const
		{ *clone = *this; }

private:
	void free_properties(void);

	Object *resolve(const char *name, Property *p) const;
	Object *resolve(const char *an, const char *zn,
					const char *on, Property *p) const;
	void initial(void);

	Property *m_props;
	bool      m_clone;

	friend Object *clone_object(const Object *master);
};

/*******************************************************************/

Object *find_object (const char   *name);
Object *clone_object(const char   *name);
Object *clone_object(const Object *master);

/*******************************************************************/
#endif /* __OBJECT_H__ */
