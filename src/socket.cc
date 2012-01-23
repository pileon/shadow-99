/********************************************************************
* File: socket.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "config.h"
#include "socket.h"

#ifndef out_of_memory
#  define out_of_memory() \
      do { fprintf(stderr, "Out of memory!\n"); abort(); } while (0)
#endif

#ifndef HAVE_GETNAMEINFO
/*
extern "C" int getnameinfo(const struct sockaddr *sa, socklen_t salen,
						   char *host, size_t hostlen,
						   char *serv, size_t servlen, int flags);
*/
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
				char *host, size_t hostlen,
				char *serv, size_t servlen, int flags);
#endif

/*******************************************************************/

#ifndef HAVE_HSTRERROR
/* Some older libc's doesn't have this function, so we use our own. */
const char hstrerror(const int herr)
{
	switch (herr)
	{
	case 0:
		return "Resolver Error 0 (no error)";
	case HOST_NOT_FOUND:
		return "Unknown host";
	case TRY_AGAIN:
		return "Host name lookup failure";
	case NO_RECOVERY:
		return "Unknown server error";
	case NO_ADDRESS:
		return "No address associated with name";
	default:
		return "Unknown resolver error";

	}
}
#endif  /* HAVE_HSTRERROR */

/*******************************************************************/

Socket::Socket(void)
{
	init_all();
}

Socket::Socket(const char *host, const int port,
			   const sa_family_t family, const int type)
{
	init_all();
	open(host, port, family, type);
}

Socket::Socket(const int port, const sa_family_t family, const int type)
{
	init_all();
	open(port, family, type);
}

Socket::Socket(const Socket &orig)
{
	init_all();
	dup(orig);
}

Socket::~Socket(void)
{
	if (isopen())
		close();

	if (m_peer && m_peer != m_local)
		delete [] (char *) m_peer;
	if (m_local)
		delete [] (char *) m_local;

	init_all();
}

void Socket::init_all(void)
{
	m_socket  = -1;
	m_local   = NULL;
	m_peer    = NULL;
	m_open    = false;
	m_type    = 0;
	m_family  = AF_UNSPEC;
	m_addrlen = 0;
}

bool Socket::open(const char *, const int,
				  const sa_family_t, const int)
{
	return false;
}

bool Socket::open(const int port, const sa_family_t family, const int type)
{
	/* Create and bind a listening socket.
	 * If family is AF_UNSPEC, this function tries to bind to the first
	 * possible protocol family (IPv6 on kernels that support it,
	 * IPv4 otherwise).
	 */

	socket_t sock;
	char     serv[32];  /* getaddrinfo() wants a string for the port number */
	addrinfo hint, *res, *save;
	int      n, on;
	bool     rc = false;

	snprintf(serv, sizeof(serv), "%d", port);

	/* get some info about localhost... */
	hint.ai_flags    = AI_PASSIVE;
	hint.ai_family   = family;
	hint.ai_socktype = type;
	if ((n = getaddrinfo(NULL, serv, &hint, &res)) != 0)
	{
		/* we should really call gai_strerror() to find out what happened */
		fprintf(stderr, "Socket::open failed: %d\n", n);
		return false;
	}
	save = res;  /* store the pointer, so we can free the list later */

	do
	{
		/* 2: try to create a socket */
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock < 0)
			continue;  /* try the next address */

		/* if possible, try to reuse addresses */
		on = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
			continue;  /* try the next address */

		/* 3: try to bind the socket to a local address */
		if (bind(sock, res->ai_addr, res->ai_addrlen) == 0)
			break;  /* ok, all done! */

		/* failure to create or bind socket, try next address */
		::close(sock);
	} while ((res = res->ai_next) != NULL);

	/* at this point, we either have a bound socket, or no socket at all */

	if (res == NULL)
	{
		/* doh!  something went wrong with the socket creation */
		perror("Socket::open");
		if (sock >= 0)
			::close(sock);
		rc = false;
	}
	else
	{
		/* good, we managed to fix a socket for us! */
		m_addrlen = res->ai_addrlen;
		m_socket  = sock;
		m_open    = true;
		m_type    = res->ai_socktype;
		m_family  = res->ai_family;

		rc = true;

		if ((m_local = (sockaddr *) new char [m_addrlen]) == NULL)
			out_of_memory();
		memcpy(m_local, res->ai_addr, m_addrlen);
		m_peer = m_local;

		if (listen(sock, 1024) < 0)
		{
			perror("Socket::open");
			::close(sock);
			rc = false;

			delete [] (char *) m_local;
			m_local = m_peer = NULL;
		}
	}

	freeaddrinfo(save);  /* free the addressinfo-list */

	return rc;
}

void Socket::close(void)
{
	if (isopen())
		::close(m_socket);

	if (m_peer && m_peer != m_local)
		delete [] (char *) m_peer;
	if (m_local)
		delete [] (char *) m_local;

	init_all();
}

bool Socket::dup(const Socket &from)
{
	if (!from.isopen())
		return true;

	if (isopen())
		close();

	if ((m_socket = ::dup(from.m_socket)) < 0)
	{
		perror("Socket::dup");
		return false;
	}

	m_type    = from.m_type;
	m_family  = from.m_family;
	m_addrlen = from.m_addrlen;

	if ((m_local = (sockaddr *) new char[m_addrlen]) == NULL)
		out_of_memory();
	if ((m_peer = (sockaddr *) new char[m_addrlen]) == NULL)
		out_of_memory();

	memcpy(m_local, from.m_local, m_addrlen);
	memcpy(m_peer , from.m_peer , m_addrlen);

	m_open = true;

	return false;
}

ssize_t Socket::write(const void *data, const size_t length, const bool all)
{
	if (isopen())
	{
		if (all)
		{
			size_t  nleft;
			ssize_t nwritten;
			const char *ptr;

			ptr   = (const char *) data;
			nleft = length;

			while (nleft > 0)
			{
				if ((nwritten = ::write(m_socket, ptr, nleft)) < 0)
				{
					if (errno == EINTR)
						nwritten = 0;  /* try again */
					else
						return -1;
				}

				nleft -= nwritten;
				ptr   += nwritten;
			}

			return length;
		}
		else
			return ::write(m_socket, data, length);
	}
	else
	{
		errno = EBADF;
		return -1;
	}
}

ssize_t Socket::read(void *data, const size_t maxlen, const bool all)
{
	if (isopen())
	{
		if (all)
		{
			size_t  nleft;
			ssize_t nread;
			char   *ptr;

			ptr   = (char *) data;
			nleft = maxlen;

			while (nleft > 0)
			{
				if ((nread = ::read(m_socket, ptr, nleft)) < 0)
				{
					if (errno == EINTR)
						nread = 0;  /* try again */
					else
						return -1;
				}
				else if (nread == 0)
					break;  /* we found an end-of-file */

				nleft -= nread;
				ptr   += nread;
			}

			return (maxlen - nleft);
		}
		else
			return ::read(m_socket, data, maxlen);
	}
	else
	{
		errno = EBADF;
		return -1;
	}
}

const char *Socket::get_ip(char *to, const size_t maxlen) const
{
	sockaddr_in  *in;
	sockaddr_un  *un;
#ifdef HAVE_IPV6
	sockaddr_in6 *in6;
#endif

	switch (m_family)
	{
	case AF_INET:
		in = (sockaddr_in *) m_peer;
		return inet_ntop(m_family, &in->sin_addr, to, maxlen);
#ifdef HAVE_IPV6
	case AF_INET6:
		in6 = (sockaddr_in6 *) m_peer;
		inet_ntop(m_family, &in6->sin6_addr, to, maxlen);
#endif
	case AF_LOCAL:
		un = (sockaddr_un *) m_peer;
		snprintf(to, maxlen, "%s", un->sun_path);
		break;
	default:
		break;
	}

	return to;
}

const char *Socket::get_host(char *to, const size_t maxlen) const
{
	if (getnameinfo(m_peer, m_addrlen, to, maxlen, NULL, 0, 0) != 0)
	{
		perror("Socket::get_host");
		return NULL;
	}
	return to;
}

unsigned short Socket::get_local_port(void) const
{
	if (!isopen() || !m_local)
	{
		errno = EINVAL;
		return -1;
	}
	else
	{
		sockaddr_in  *in;
#ifdef HAVE_IPV6
		sockaddr_in6 *in6;
#endif

		switch (m_family)
		{
		case AF_INET:
			in = (sockaddr_in *) m_local;
			return ntohs(in->sin_port);
#ifdef HAVE_IPV6
		case AF_INET6:
			in = (sockaddr_in *) m_local;
			return ntohs(in->sin_port);
#endif
		default:
			errno = EAFNOSUPPORT;
			return -1;
		}
	}
}

unsigned short Socket::get_peer_port(void) const
{
	if (!isopen() || !m_peer)
	{
		errno = EINVAL;
		return -1;
	}
	else
	{
		sockaddr_in  *in;
#ifdef HAVE_IPV6
		sockaddr_in6 *in6;
#endif

		switch (m_family)
		{
		case AF_INET:
			in = (sockaddr_in *) m_peer;
			return ntohs(in->sin_port);
#ifdef HAVE_IPV6
		case AF_INET6:
			in = (sockaddr_in *) m_peer;
			return ntohs(in->sin_port);
#endif
		default:
			errno = EAFNOSUPPORT;
			return -1;
		}
	}
}

bool Socket::set_blocking(const bool on /* = true */)
{
	bool old = m_blocking;
	int  flags;

	if (m_blocking && on)
		/* trying to make a blocking socket blocking... */
		return m_blocking;
	if (!m_blocking && !on)
		/* trying to make a nonblocking socket nonblocking... */
		return m_blocking;

	flags = fcntl(m_socket, F_GETFL, 0);
	if (on)
		flags &= ~O_NDELAY;
	else
		flags |= O_NDELAY;
	if (fcntl(m_socket, F_SETFL, flags) < 0)
	{
		perror("Socket::set_blocking");
		abort();
	}

	m_blocking = on;

	return old;
}

/*******************************************************************/

TcpSocket::TcpSocket(void)
	: Socket()
{
	init_all();
}

TcpSocket::TcpSocket(const char *host, const int port)
	: Socket()
{
	init_all();
	open(host, port);
}

TcpSocket::TcpSocket(const int port)
	: Socket()
{
	init_all();
	open(port);
}

TcpSocket::TcpSocket(const TcpSocket &)
	: Socket()
{
	init_all();
}

TcpSocket::~TcpSocket(void)
{
	if (isopen())
		close();
}

void TcpSocket::init_all(void)
{
}

bool TcpSocket::open(const char *host, const int port)
{
	return Socket::open(host, port, AF_UNSPEC, SOCK_STREAM);
}

bool TcpSocket::open(const int port)
{
	return Socket::open(port, AF_UNSPEC, SOCK_STREAM);
}

bool TcpSocket::accept(TcpSocket &to)
{
	socket_t  sock;
	sockaddr *addr;
	socklen_t alen;

	alen = m_addrlen;
	if ((addr = (sockaddr *) new char [alen]) == NULL)
		out_of_memory();

	if ((sock = ::accept(m_socket, addr, &alen)) < 0)
	{
		perror("Socket::accept");
		delete [] (char *) addr;
		return false;
	}

	to.m_socket  = sock;
	to.m_open    = true;
	to.m_type    = m_type;
	to.m_family  = m_family;
	to.m_peer    = addr;
	to.m_addrlen = alen;
	to.m_local   = NULL;

	/* get the local address for the new socket */
	alen = to.m_addrlen;
	if ((addr = (sockaddr *) new char [alen]) == NULL)
		out_of_memory();

	if (getsockname(to.m_socket, addr, &alen) < 0)
	{
		perror("TcpSocket::accept");
		delete [] (char *) addr;
	}
	else
		to.m_local = addr;

	return true;
}

/*******************************************************************/
