#ifndef __TELNET_H__
#define __TELNET_H__
/********************************************************************
* File: telnet.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "socket.h"

/*******************************************************************/

class TelnetSocket : public TcpSocket
{
public:
	TelnetSocket(void);
	virtual ~TelnetSocket(void);

	bool get_echo(void) const { return m_echo; }
	bool set_echo(const bool on = true);

	virtual ssize_t read(void *buffer, const size_t maxlen,
						 const bool all = false);
	virtual ssize_t write(const void *data, const size_t length,
						  const bool all = false);
	virtual ssize_t write(const char *string);

private:
	void init_all(void);
	ssize_t handle_telnet(const char *from, char *to, const size_t len) const;
	ssize_t strip_input(const char *from, char *to, const size_t len) const;
	ssize_t check_crlf(const char *from, char *to, const size_t len) const;

	bool m_echo; /* true when echo is on */
	bool m_iac;  /* true when we sent an IAC */
};

/* write a NULL terminated string */
inline ssize_t TelnetSocket::write(const char *string)
{
	return write(string, strlen(string));
}

/*******************************************************************/
#endif /* __TELNET_H__ */
