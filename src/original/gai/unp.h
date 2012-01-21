#ifndef __UNP_H__
#define __UNP_H__
/********************************************************************
* File: libgai/unp.h                              Part of Shadow-99 *
*********************************************************************
* This file is not based on the original unp.h.  Instead I have     *
* made up my own to only include what is needed by gai library.     *
* It odes ofcourse mean that any errors in this file is my own, and *
* not Stevens.  Errors in the other source files (however unlikely) *
* belongs to Stevens, since I have not touched them.                *
********************************************************************/

#include "../config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

/*******************************************************************/

#ifndef HAVE_GETADDRINFO

struct addrinfo
{
	int    ai_flags;           /* AI_PASSIVE, AI_CANONNAME */
	int    ai_family;          /* PF_xxx */
	int    ai_socktype;        /* SOCK_xxx */
	int    ai_protocol;        /* IPPROTO_xxx for IPv4 and IPv6 */
	size_t ai_addrlen;         /* length of ai_addr */
	char  *ai_canonname;       /* canonical name for host */
	struct sockaddr *ai_addr;  /* binary address */
	struct addrinfo *ai_next;  /* next structure in linked list */
};

    /* following for getaddrinfo() */
#define AI_PASSIVE   1   /* socket is intended for bind() + listen() */
#define AI_CANONNAME 2   /* return canonical name */

#endif /* HAVE_GETADDRINFO */

#ifndef HAVE_GETNAMEINFO

    /* following for getnameinfo() */
#define NI_MAXHOST    1025  /* max hostname returned */
#define NI_MAXSERV      32  /* max service name returned */

#define NI_NOFQDN        1  /* do not return FQDN */
#define NI_NUMERICHOST   2  /* return numeric form of hostname */
#define NI_NAMEREQD      4  /* return error if hostname not found */
#define NI_NUMERICSERV   8  /* return numeric form of service name */
#define NI_DGRAM        16  /* datagram service for getservbyname() */

#endif /* HAVE_GETNAMEINFO */

/*******************************************************************/

/*******************************************************************/
#endif /* __UNP_H__ */
