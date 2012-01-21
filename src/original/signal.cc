/********************************************************************
* File: signal.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "server.h"

#include <signal.h>
#include <sys/wait.h>

/*******************************************************************/

static void sighup(int sig);
static void sigill(int sig);
#ifndef SIGCLD
static void sigchild(int sig);
#endif
static void checkpoint(int sig);

typedef void sigfunc(int);

static sigfunc *my_signal(int signo, sigfunc *fun);

/*******************************************************************/

bool Server::signal_setup(void)
{
	/* Note that we don't check return values from my_signal(),
	 * this is a "Bad Thing" (tm), and shouldn't be attempted
	 * by mere mortals.  ;)
	 */

	struct itimerval itime;
	struct timeval interval;

	/* signals to exit upon */
	my_signal(SIGHUP , (sigfunc *) sighup);
	my_signal(SIGINT , (sigfunc *) sighup);
	my_signal(SIGTERM, (sigfunc *) sighup);

	/* signals to ignore */
	my_signal(SIGPIPE, SIG_IGN);
	my_signal(SIGALRM, SIG_IGN);

	/* signals to log an error, then exit, upon */
	my_signal(SIGBUS , (sigfunc *) sigill);
	my_signal(SIGFPE , (sigfunc *) sigill);
	my_signal(SIGILL , (sigfunc *) sigill);
	my_signal(SIGSEGV, (sigfunc *) sigill);

	/* all other signals */
#ifdef SIGCLD  /* System V */
	my_signal(SIGCLD, SIG_IGN);  /* just ignore any child exits */
#else
	my_signal(SIGCHLD, (sigfunc *) ::sigchild);
#endif

	/*
	 * set up the deadlock-protection so that the MUD aborts itself if it gets
	 * caught in an infinite loop for more than 30 seconds.
	 */
	interval.tv_sec = 30;
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, NULL);
	my_signal(SIGVTALRM, (sigfunc *) checkpoint);

	return true;
}

/*******************************************************************/
/*
 * This function is taken from W. Richard Stevens book
 * 'Advanced Programming in the UNIX Environment'.
 *
 * Shadow-99 uses this function instead of signal(2) because
 * we don't want interrupted functions to restart.
 */
static sigfunc *my_signal(int signo, sigfunc *fun)
{
	struct sigaction act, oact;

	act.sa_handler = fun;
	act.sa_flags   = 0;
#ifdef SA_INTERRUPT  /* for SunOS */
	act.sa_flags  |= SA_INTERRUPT;
#endif
	sigemptyset(&act.sa_mask);

	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;

	return oact.sa_handler;
}


/*******************************************************************/

static void sighup(int sig)
{
	log("Caught HANGUP signal.  Shutting down.\n");

	/* this may look a bit sparse, but remember that the Server destructor
	   is called as part of the exit process, and it handles all freeing */
	exit(0);
}

static void sigill(int sig)
{
	/* TODO:  add some sort of calltrace printout? */

	switch (sig)
	{
	case SIGBUS:
		log("Bus error caught.  Shutting down.\n");
		break;
	case SIGFPE:
		log("Math exception caught.  Shutting down.\n");
		break;
	case SIGILL:
		log("Illegal instruction caught.  Shutting down.\n");
		break;
	case SIGSEGV:
		log("Segmentation fault caught.  Shutting down.\n");
		break;

	default:
		log("Unknown illegal signal caught: %d.  Shutting down.\n", sig);
		break;
	}

	abort();  /* try to generate a core */
}

#ifndef SIGCLD
static void sigchild(int sig)
{
	/* we don't want any zombie processes */
	int rc;
	waitpid(-1, &rc, WNOHANG);
}
#endif

/* check if we are stuck in an infinite loop */
static void checkpoint(int sig)
{
	if (!server.m_tics)
	{
		log("SYSERR:  Infinite loop detected, aborting.\n");
		abort();
	}
	else
		server.m_tics = 0;
}

/*******************************************************************/
