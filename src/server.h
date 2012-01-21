#ifndef __SERVER_H__
#define __SERVER_H__
/********************************************************************
* File: server.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "socket.h"

/*******************************************************************/

class User;
class Character;
class Area;
class Object;

class Server
{
public:
	Server();
	~Server();

	bool init(int ac, char **av);
	void run(void);

	void shutdown(void);

	void print_version(void) const
		{ printf("%s %d.%d\n", mudconfig.mudname, get_ver(), get_rev()); }
	int get_ver(void) const
		{ return 0; }
	int get_rev(void) const
		{ return 5; }

	bool is_shutdown(void) const { return m_shutdown; }

public:
	User      *m_users;   /* a list of all connected users */
	Character *m_chars;   /* a list of all character, including monsters */
	Area      *m_areas;   /* a list of all loaded areas */
	Object    *m_objects; /* a list of _all_ loaded objects */
	long       m_tics;

private:
	bool      m_shutdown;
	TcpSocket m_master;
	fd_set    m_fdr;  /* readable fd's */
	fd_set    m_fdw;  /* writeable fd's */
	fd_set    m_fde;  /* fd's in an exception state */
	unsigned long m_beats;  /* heartbeat count */

	bool signal_setup(void);   /* in signal.cc */
	bool load_texts(void);     /* in boot.cc */
	void free_texts(void);     /* in boot.cc */
	bool boot_world(void);     /* in boot.cc */
	bool resolve_world(void);  /* in boot.cc */
	bool resolve_other(void);  /* in boot.cc */
	bool boot_area(const char *, const char *);  /* in boot.cc */

	void comm_wait(void);      /* in comm.cc */
	void comm_check(void);     /* in comm.cc */
	void comm_read(void);      /* in comm.cc */
	void comm_handle(void);    /* in comm.cc */
	void comm_write(void);     /* in comm.cc */
	void comm_kick(void);      /* in comm.cc */
	void comm_newcon(void);    /* in comm.cc */
	void comm_close(User *u);  /* in comm.cc */

	void heartbeat(void);      /* in heartbeat.cc */
	void beat_sleep(void);     /* in heartbeat.cc */
	void round_tick(void);     /* in heartbeat.cc */
	void round_monster(void);  /* in heartbeat.cc */
	void round_violence(void); /* in heartbeat.cc */
	void round_autosave(void); /* in heartbeat.cc */
};

extern Server server;

/*******************************************************************/
#endif /* __SERVER_H__ */
