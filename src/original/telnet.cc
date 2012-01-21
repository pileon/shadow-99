/********************************************************************
* File: telnet.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
*********************************************************************
*
* CHECKOUT
* -------
*
* RFC854, RFC855 - Telnet Protocol
* RFC856 - Binary Transmission Telnet Option
* RFC857 - Echo Telnet Option
* RFC858 - Suppress Go Ahead Telnet Option
* RFC859 - Status Telnet Option
* RFC860 - Timing Mark Telnet Option
* RFC861 - Extended Options List Telnet Option
*
* (Also std8.txt, std27.txt-std32.txt.)
*
********************************************************************/

#include <sys/types.h>
#include <arpa/telnet.h>
#include <ctype.h>
#include <errno.h>

#include "socket.h"
#include "telnet.h"
#include <stdio.h>

/*******************************************************************/

TelnetSocket::TelnetSocket(void)
{
	init_all();
}

TelnetSocket::~TelnetSocket(void)
{
	if (isopen())
		close();
}

void TelnetSocket::init_all(void)
{
}

/*******************************************************************/

const bool TelnetSocket::set_echo(const bool on /* = true */)
{
	static const char off_string[] =
	{
		(char) IAC,
		(char) WILL,
		(char) TELOPT_ECHO,
	};
	static const char on_string[] =
	{
		(char) IAC,
		(char) WONT,
		(char) TELOPT_ECHO,
		(char) TELOPT_NAOFFD,
		(char) TELOPT_NAOCRD,
	};
	int   len;
	const bool  prev = m_echo;
	const char *str;

	m_echo = on;
	len    = m_echo ? sizeof(on_string) : sizeof(off_string);
	str    = m_echo ? on_string : off_string;

	write(str, len);

	m_iac = true;

	return prev;
}

ssize_t TelnetSocket::read(void *buffer, const size_t maxlen,
						   const bool all /* = false */)
{
	char    tbuf1[maxlen + 1];
	char    tbuf2[maxlen + 1];
	ssize_t res;

	if ((res = TcpSocket::read(tbuf1, maxlen, all)) <= 0)
		return res;

	/* handle telnet command sequences */
	if ((res = handle_telnet((char *) tbuf1, (char *) tbuf2, res)) == 0)
	{
		errno = EWOULDBLOCK;
		return -1;
	}

	/* strip all unprintable chars */
	if ((res = strip_input((char *) tbuf2, (char *) buffer, res)) == 0)
	{
		errno = EWOULDBLOCK;
		return -1;
	}

	return res;
}

ssize_t TelnetSocket::write(const void *data, const size_t length,
							const bool all /* = false */)
{
	char    buffer[length * 2];
	ssize_t res;

	res = check_crlf((char *) data, (char *) buffer, length);

	return TcpSocket::write(data, res, all);
}

/*******************************************************************/

/*
static const char *codes[] = {
	"IAC",
	"DONT",
	"DO",
	"WONT",
	"WILL",
	"SB",
	"GA",
	"EL",
	"EC",
	"AYT",
	"AO",
	"IP",
	"BREAK",
	"DM",
	"NOP",
	"SE",
	"EOR",
	"ABORT",
	"SUSP",
	"xEOF",
};

static const char *telopts[] = {
	"TELOPT_BINARY",
	"TELOPT_ECHO",
	"TELOPT_RCP",
	"TELOPT_SGA",
	"TELOPT_NAME",
	"TELOPT_STATUS",
	"TELOPT_TM",
	"TELOPT_RCTE",
	"TELOPT_NAOL",
	"TELOPT_NAOP",
	"TELOPT_NAOCRD",
	"TELOPT_NAOHTS",
	"TELOPT_NAOHTD", 
	"TELOPT_NAOFFD",
	"TELOPT_NAOVTS",
	"TELOPT_NAOVTD",
	"TELOPT_NAOLFD",
	"TELOPT_XASCII",
	"TELOPT_LOGOUT",
	"TELOPT_BM",
	"TELOPT_DET",
	"TELOPT_SUPDUP", 
	"TELOPT_SUPDUPOUTPUT",
	"TELOPT_SNDLOC",
	"TELOPT_TTYPE",
	"TELOPT_EOR",
	"TELOPT_TUID",
	"TELOPT_OUTMRK", 
	"TELOPT_TTYLOC",
	"TELOPT_3270REGIME",
	"TELOPT_X3PAD",
	"TELOPT_NAWS",
	"TELOPT_TSPEED",
	"TELOPT_LFLOW",
	"TELOPT_LINEMODE",
	"TELOPT_XDISPLOC",
	"TELOPT_OLD_ENVIRON",
	"TELOPT_AUTHENTICATION",
	"TELOPT_ENCRYPT",
	"TELOPT_NEW_ENVIRON",
};
*/

ssize_t TelnetSocket::handle_telnet(const char *from, char *to,
									const size_t len) const
{
	register const char *f = from;
	register       char *t = to;

	//static const char *aytrep = "Shadow-99 is here...\r\n";

	for (; *f && f < (from + len); f++)
	{
		if ((unsigned char) *f == IAC)
		{
			//fprintf(stderr, "  <- IAC\n");
			switch ((unsigned char) *++f)
			{
			case WILL:
			case WONT:
			case DO:
			case DONT:	
				//fprintf(stderr, "  <- %s\n",
				//		codes[IAC - *(unsigned char *) f]);
				f++;
				//fprintf(stderr, "  <- %s\n", telopts[*(unsigned char *) f]);
				break;
			case AYT:
				//Socket::write(aytrep);
				break;
			default:
				break;
			}
		}
		else
			*t++ = *f;
	}
	*t = 0;  /* terminate string */

	return (t - to);
}

ssize_t TelnetSocket::strip_input(const char *from, char *to,
								  const size_t len) const
{
	register const char *f = from;
	register       char *t = to;

	for (; *f && f < (from + len); f++)
	{
		if (*f > 0 && (isprint(*f) || *f == '\n'))
			*t++ = *f;
		else if (*f == '\b' || *f == 127)
		{
			/* backspace or delete */
			if (t > to)
				t--;
		}
	}
	*t = 0;  /* terminate string */

	return (t - to);
}

/*******************************************************************/

ssize_t TelnetSocket::check_crlf(const char *from, char *to,
								 const size_t len) const
{
	register const char *f = from;
	register       char *t = to;

	for (; *f && f < (from + len); f++)
	{
		if ((*f == '\r' && *(f + 1) != '\n') ||
			(*f == '\n' && *(f + 1) != '\r'))
		{
			f++;
			*t++ = '\r';
			*t++ = '\n';
		}
		else if (*f == '\n' && *(f + 1) == '\r')
		{
			f++;
			*t++ = '\r';
			*t++ = '\n';
		}
		else
			*t++ = *f;
	}
	return (t - to);
}

/*******************************************************************/
