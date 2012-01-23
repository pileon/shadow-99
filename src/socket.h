#ifndef __SOCKET_H__
#define __SOCKET_H__
/********************************************************************
* File: socket.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include <sys/socket.h>

/*******************************************************************/

typedef int socket_t;

/* the maximum length a string with an ip address can be */
#define INET_MAXADDRSTRLEN 128  /* a little more than needed, to be safe */


#ifndef AF_LOCAL
#  define AF_LOCAL AF_UNIX
#endif

/*******************************************************************/

class Socket
{
public:
	Socket(void);
	Socket(const char *host, const int port,
		   const sa_family_t family, const int type);
	Socket(const int port, const sa_family_t family, const int type);
	Socket(const Socket &orig);
	virtual ~Socket(void);

	/* open an active socket to a specified host */
	virtual bool open(const char *host, const int port,
					  const sa_family_t family, const int type);

	/* open a passive (listening) socket on a specified port */
	virtual bool open(const int port,
					  const sa_family_t family, const int type);

	/* close an opened socket */
	virtual void close(void);

	/* duplicate a socket */
	virtual bool dup(const Socket &from);

	/* is the socket open? */
	bool isopen(void) const;

	/* write data to the socket */
	virtual ssize_t write(const void *data, const size_t length,
						  const bool all = false);
	virtual ssize_t write(const char *string);

	/* read data from a socket */
	virtual ssize_t read(void *buffer, const size_t maxlen,
						 const bool all = false);

	/* should only be used for select(2) or poll(2) calls */
	socket_t operator() (void) const;

	/* functions to get info about the socket */
	const char *get_ip(char *to, const size_t maxlen) const;
	const char *get_host(char *to, const size_t maxlen) const;
	unsigned short get_local_port(void) const;
	unsigned short get_peer_port(void) const;

	bool is_blocking(void) const;
	bool set_blocking(const bool on = true);

protected:
	socket_t    m_socket;   /* the socket descriptor */
	bool        m_open;     /* true if socket is open */
	int         m_type;     /* SOCK_STREAM etc. */
	sa_family_t m_family;   /* AF_INET, AF_INET6, etc. */
	sockaddr   *m_local;    /* our local address */
	sockaddr   *m_peer;     /* our peer address */
	socklen_t   m_addrlen;  /* size of socket addresses */
	bool        m_blocking; /* true is socket is blocking */
private:
	void init_all(void);
};

class TcpSocket : public Socket
{
public:
	TcpSocket(void);
	TcpSocket(const char *host, const int port);  /* create a client socket */
	TcpSocket(const int port);        /* create a passive (listening) socket */
	TcpSocket(const TcpSocket &orig); /* copy constructor */
	virtual ~TcpSocket(void);

	/* open an active socket to a specified host */
	bool open(const char *host, const int port);

	/* open a passive (listening) socket on a specified port */
	bool open(const int port);

	/* accept a connection on THIS socket */
	bool accept(TcpSocket &to);

private:
	void init_all(void);
};

/* TODO:  class UdpSocket */

/*******************************************************************/

inline bool Socket::isopen(void) const
{
	return m_open;
}

/* write a NULL terminated string */
inline ssize_t Socket::write(const char *string)
{
	return write(string, strlen(string));
}

inline socket_t Socket::operator() (void) const
{
	return m_socket;
}

inline bool Socket::is_blocking(void) const
{
	return m_blocking;
}

/*******************************************************************/
#endif /* __SOCKET_H__ */
