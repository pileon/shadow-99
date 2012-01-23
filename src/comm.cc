/********************************************************************
* File: comm.c                                    Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "server.h"
#include "user.h"
#include "player.h"

#include <stdarg.h>

/********************************************************************
 *
 * This file contains functions to handle all connections at the
 * highest level.  For every existing connection, they loop through
 * them, telling them to read commands, or write buffered text.
 *
 *******************************************************************/

void Server::comm_wait(void)
{
	if (!m_users)
	{
		/* no users are currently connected, wait for some to arrive */
		fd_set fds;

		log("No connections, going to sleep\n");

		FD_ZERO(&fds);
		FD_SET(m_master(), &fds);

		if (select(m_master() + 1, &fds, NULL, NULL, NULL) < 0)
		{
			if (errno != EINTR)
				perror("Server::comm_wait");
		}
		else
			log("New connection detected, waking up\n");
	}

	/* TODO:  init the 'last_time' variable */
}

void Server::comm_check(void)
{
	User    *u;
	socket_t m;
	timeval  tmo;

	FD_ZERO(&m_fdr);
	FD_ZERO(&m_fdw);
	FD_ZERO(&m_fde);

	FD_SET(m_master(), &m_fdr);
	m = m_master();
	for (u = m_users; u; u = u->m_next)
	{
		FD_SET(u->m_socket(), &m_fdr);
		FD_SET(u->m_socket(), &m_fdw);
		FD_SET(u->m_socket(), &m_fde);
		m = MAX(m, u->m_socket());
	}

	tmo.tv_usec = 0;
	tmo.tv_sec  = 0;
	if (select(m + 1, &m_fdr, &m_fdw, &m_fde, &tmo) < 0)
	{
		perror("Server::comm_check");
		return;
	}
}

void Server::comm_read(void)
{
	User *u, *n;

	if (FD_ISSET(m_master() , &m_fdr))
		comm_newcon();

	for (u = m_users; u; u = n)
	{
		n = u->m_next;

		if (FD_ISSET(u->m_socket(), &m_fdr))
		{
			if (!u->input())
			{
				FD_CLR(u->m_socket(), &m_fdr);
				FD_CLR(u->m_socket(), &m_fdw);
				FD_CLR(u->m_socket(), &m_fde);
				comm_close(u);
			}
		}
	}
}

void Server::comm_handle(void)
{
	/* handle all input previously read in comm_read() */

	User *u;

	for (u = m_users; u; u = u->m_next)
	{
		if (u->m_player && (u->get_state()      == User::S_PLAYING ||
							u->get_prev_state() == User::S_PLAYING))
		{
			u->m_prompt = false;
		}
		u->handle_input();
	}
}

void Server::comm_write(void)
{
	/* write queued output to all users that have it */

	User *u;

	for (u = m_users; u; u = u->m_next)
	{
		if (!u->m_prompt && u->have_output())
		{
			if (u->m_player && (u->get_state()      == User::S_PLAYING ||
								u->get_prev_state() == User::S_PLAYING))
			{
				u->m_player->write_prompt();
			}
		}
	}

	for (u = m_users; u; u = u->m_next)
	{
		if (FD_ISSET(u->m_socket(), &m_fdw))
		{
			if (!u->output())
			{
				FD_CLR(u->m_socket(), &m_fdr);
				FD_CLR(u->m_socket(), &m_fdw);
				FD_CLR(u->m_socket(), &m_fde);
				comm_close(u);
			}
			else
				u->m_prompt = true;
		}
	}
}

void Server::comm_kick(void)
{
	/* kick off users in an exception state, or that are about to quit */

	User *u, *p, *n;
	char  s[128];

	for (p = NULL, u = m_users; u; u = n)
	{
	   n = u->m_next;

	   if (FD_ISSET(u->m_socket(), &m_fde) || u->get_state() == User::S_CLOSE)
	   {
		   FD_CLR(u->m_socket(), &m_fdr);
		   FD_CLR(u->m_socket(), &m_fdw);
		   FD_CLR(u->m_socket(), &m_fde);

		   /* unlink the user from the userlist */
		   if (p == NULL)
			   m_users = u->m_next;
		   else
			   p->m_next = u->m_next;

#if 1
		   log("Closing connection to: %s:%u\n",
			   u->m_socket.get_host(s, 127), u->m_socket.get_peer_port());
#endif

		   /* delete the user */
		   delete u;
	   }
	   else
		   p = u;
	}
}

void Server::comm_newcon(void)
{
	/* called when a new user connects to the MUD */

	User *u;
	char  s[128];

	if ((u = new User) == NULL)
		out_of_memory();

	if (!m_master.accept(u->m_socket))
	{
		delete u;
		return;
	}
	u->m_socket.set_blocking(false);  /* make the socket nonblocking */

	/* let the user initialise itself */
	if (!u->init())
	{
		delete u;
		return;
	}

	u->m_next = m_users;
	m_users   = u;

#if 1
	log("New connection from: %s:%u\n",
		u->m_socket.get_host(s, 127), u->m_socket.get_peer_port());
#endif
}

void Server::comm_close(User *u)
{
	User *un, *up;

	/* first unlink the node */
	for (up = NULL, un = m_users; un; up = un, un = un->m_next)
	{
		if (u == un)
		{
			if (up == NULL)
				m_users = un->m_next;  /* first node in list */
			else
				up->m_next = un->m_next;

			break;
		}
	}

	/* if the user is connected to a player, unlink the player too */
	//if (u->m_player)
	//{
	//}

	/* delete the user */
	delete u;
}

/*******************************************************************/

// void Server::write(User *u, const char *str, ...)
// {
// 	char buf[1024];  /* should be enough for most texts! */
// 	va_list args;

// 	if (!u || !str)
// 		return;

// 	va_start(args, str);
// 	vsnprintf(buf, 1024, str, args);
// 	va_end(args);

// 	u->write(buf);
// }

// void Server::write_all(const char *str, ...)
// {
// 	char buf[1024];  /* should be enough for most texts! */
// 	va_list args;
// 	User *u;

// 	if (!m_users || !str)
// 		return;

// 	va_start(args, str);
// 	vsnprintf(buf, 1024, str, args);
// 	va_end(args);

// 	for (u = m_users; u; u = u->m_next)
// 		u->write(buf);
// }

// void Server::write_all_but(User *nu, const char *str, ...)
// {
// 	char buf[1024];  /* should be enough for most texts! */
// 	va_list args;
// 	User *u;

// 	if (!m_users || !nu || !str)
// 		return;

// 	va_start(args, str);
// 	vsnprintf(buf, 1024, str, args);
// 	va_end(args);

// 	for (u = m_users; u; u = u->m_next)
// 	{
// 		if (u != nu)
// 			u->write(buf);
// 	}
// }

/*******************************************************************/
