#ifndef __USER_H__
#define __USER_H__
/********************************************************************
* File: user.h                                    Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "telnet.h"
#include <stdarg.h>

/*******************************************************************/

struct textnode
{
	char  *t;  /* the text itself */
	size_t l;  /* length of text in the node */
	char  *p;  /* pointer to current position in the text */

	textnode *n;

	textnode(void)
		{ t = NULL; l = 0; p = NULL; n = NULL; }
	~textnode(void)
		{
			if (t)
				delete [] t;
			t = NULL;
			l = 0;
			p = NULL;
			n = NULL;
		}
};

#define MAX_BUFFER_LEN  512

class Player;

class User
{
public:
	enum state
	{
		S_NONE    = -1,
		S_PLAYING = 0, /* the user is playing (normal state) */
		S_CLOSE,       /* the user is about to be thown out */
		S_MORE,        /* the user is viewing some text */
		S_EDIT,        /* the user is editing some text */

		/* states for normal login */
		S_GETNAME,     /* get the users name */
		S_GETPWD,      /* get the users password */
		S_MOTD,        /* let the user read the "Message Of The Day" */
		S_MENU,        /* the user is sitting at the menu */

		/* state for character creation */
		S_CONFNAME,    /* get the users name confirmation */
		S_GETNEWPWD,   /* get a new password */
		S_CONFNEWPWD,  /* confirm the new password */
		S_GETREALNAME, /* get the users real name */
		S_GETEMAIL,    /* get the users email address */
		S_GETSEX,      /* get the players prefered sex */
		S_SELHINTS1,   /* selecting hints for the stat roll */
		S_SELHINTS2,   /* selecting more hints for the stat roll */
		S_ROLLSTATS1,  /* rolling stats */
		S_ROLLSTATS2,  /* rolling stats */
		S_READRULES,   /* let the user read the rules of the MUD */

		/* other states */
		S_READBKG,     /* the user is reading the background story */
		S_CHGPWD1,     /* changing password; enter old password */
		S_CHGPWD2,     /* changing password; enter new password */
		S_CHGPWD3,     /* changing password; confirm new password */
	};

public:
	User();
	~User();

	bool init(void);
	void write(const char *fmt, ...);
	void vwrite(const char *fmt, va_list args);
	void page (const char *text);

	const state get_state     (void) const { return m_state; }
	const state get_prev_state(void) const { return m_prevs; }
	const state set_state(const state newstate)
		{ state oldstate = m_state;  m_state = newstate; return oldstate; }

	/* should only be used by the Server class */
	User        *m_next;   /* link for the servers user list */
	TelnetSocket m_socket; /* all comminucation goes through here */
	Player      *m_player; /* link to the player acosiated with this user */
	bool         m_prompt; /* true if the user have prompt displayed */

	bool input(void);         /* called only by the server object */
	bool output(void);        /* called only by the server object */
	void handle_input(void);  /* called only by the server object */

	const bool have_output(void) const { return m_output; }

private:
	textnode    *m_input;   /* input queue head */
	textnode    *m_itail;   /* input queue tail */
	textnode    *m_output;  /* output queue head */
	textnode    *m_otail;   /* output queue tail */
	textnode     m_current; /* the node we are reading text into */
	char        *m_page;    /* for the pager */
	char        *m_pagepos; /* for the pager */
	char        *m_prevpos; /* for the pager */
	int          m_nlines;  /* for the pager */
	int          m_cline;   /* for the pager */
	state        m_prevs;   /* used by the editor and pager */
	state        m_state;   /* what state the user is in */

	void add_textnode(const char *s, const size_t l,
					  textnode **head, textnode **tail);
	void menu_pick(const textnode *tn);
	int  load_player(const char *name);
	void flush_queue(textnode **head);
	void page_one(void);         /* show one page to the user */
	void page_cmd(textnode *tn); /* handle a pager command */
	void parse_colour(char *from, char *to);
	void s_getname(textnode *tn);
	bool check_dups(void);
};

/*******************************************************************/
#endif /* __USER_H__ */
