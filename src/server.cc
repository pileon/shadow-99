/********************************************************************
* File: server.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include <time.h>

#include "shadow.h"
#include "user.h"
#include "object.h"
#include "player.h"

/*******************************************************************/

Server server;

/*******************************************************************/

Server::Server()
{
	m_shutdown = false;
	m_users    = NULL;
	m_chars    = NULL;
	m_areas    = NULL;
	m_objects  = NULL;
	m_tics     = 0;
	m_beats    = 0;
}

Server::~Server()
{
	User   *u, *nu;
	Object *o, *no;

	/* TODO:  set m_shutdown to true, and wait for the mainloop to finish? */

	m_shutdown = true;

	if (m_master.isopen())
		m_master.close();

	for (u = m_users; u; u = nu)
	{
#if 1
		char s[128];
		log("   Closing connection to %s:%u\n", 
			u->m_socket.get_host(s, 127), u->m_socket.get_peer_port());
#endif

		if (u->m_player)
		{
			u->m_player->save();
			u->m_player = NULL;
		}
		nu = u->m_next;
		delete u;
	}

	for (o = m_objects; o; o = no)
	{
		no = o->m_nexto;
		o->m_children = NULL;
		o->m_parent   = NULL;
		delete o;
	}

	free_texts();
}

bool Server::init(int ac, char **av)
{
	bool skill_init(void);
	bool guild_init(void);

	/* if you remove or change the next line, you must state that the MUD
	   is based on Shadow-99 version x.y */
	log("Shadow-99 %d.%d booting\n", get_ver(), get_rev());

	int seed = time(0);
#if defined(HAVE_LRAND48)
	srand48(seed);
#elif defined(HAVE_RANDOM)
	srandom(seed);
#else
	srand(seed);
#endif

	/* 1: configuration */
	{
		/* 1.1: set configuration defaults */
		if (!cfg_setdef())
			return false;

		/* 1.2: load configuration file */
		if (!cfg_readcfg(ac, av))
			return false;

		/* 1.3: parse arguments */
		if (!cfg_parseargs(ac, av))
			return false;

		/* 1.4: check environment */
		if (!cfg_checkenv())
			return false;
	}

	/* 2: move into the library directory */
	if (chdir(mudconfig.libdir) < 0)
	{
		perror("Server::init (chdir)");
		return false;
	}

	/* 3: start threads and processes, if any */

	/* 4: catch some signals */
	if (!signal_setup())
		return false;

	/* 5: boot the world */
	{
		/* 5.1: load static texts */
		if (!load_texts())
			return false;

		/* 5.2: load the world */
		if (!boot_world())
			return false;

		/* 5.3: resolve all unresolved object references in the world */
		if (!resolve_world())
			return false;

		/* 5.4: resolve all other unresolved object references */
		if (!resolve_other())
			return false;

		/* 5.5: load the help index */
		//if (!help_init())
		//	return false;
	}

	/* 6: initialise all other MUD/world specific things */
	{
		/* 6.1: init the skill system */
		if (!skill_init())
			return false;

		/* 6.2 init the guild system */
		if (!guild_init())
			return false;
	}

	/* 7: create master socket */
	if (!m_master.open(mudconfig.masterport))
		return false;
	m_master.set_blocking(false);  /* make socket nonblocking */
	log("Master socket opened on port %d\n", mudconfig.masterport);

	log("Boot done.  %s is running\n", mudconfig.mudname);
	return true;
}

void Server::run(void)
{
	for (;;)
	{
		if (m_shutdown)
			break;

		comm_wait();
		comm_check();
		comm_read();
		comm_handle();
		comm_write();
		comm_kick();

		heartbeat();

		/* update tics for deadlock protection */
		m_tics++;
	}
}

/*******************************************************************/

void Server::shutdown(void)
{
	/* TODO:  do some safety checking? */
	m_shutdown = true;
}

/*******************************************************************/
