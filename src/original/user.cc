/********************************************************************
* File: user.h                                    Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "user.h"
#include "player.h"
#include "location.h"

#include <crypt.h>

/*******************************************************************/

User::User()
{
	m_next   = NULL;
	m_state  = S_NONE;
	m_player = NULL;
}

User::~User()
{
	flush_queue(&m_input);
	flush_queue(&m_output);

	if (m_socket.isopen())
		m_socket.close();

	if (m_player)
		delete m_player;

	REMOVE_FROM_LIST(this, server.m_users, m_next, User);
}

/*******************************************************************/

bool User::init(void)
{
	m_socket.write(mudconfig.wellcome);
	m_state   = S_GETNAME;
	m_prevs   = S_NONE;
	m_input   = NULL;
	m_itail   = NULL;
	m_output  = NULL;
	m_otail   = NULL;
	m_page    = NULL;
	m_pagepos = NULL;
	m_prevpos = NULL;
	m_prompt  = true;
	memset(&m_current, 0, sizeof(m_current));

	return true;
}

void User::write(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vwrite(fmt, args);
	va_end(args);
}

void User::vwrite(const char *fmt, va_list args)
{
	char buf1[1024];  /* should be enough for most texts! */
	char buf2[1536];  /* should be enough for most texts! */

	vsprintf(buf1, fmt, args);
	parse_colour(buf1, buf2);
	add_textnode(buf2, strlen(buf2), &m_output, &m_otail);
}

/* write a long text, with possible page breaks, to the user */
void User::page(const char *text)
{
	if (m_state == S_EDIT || m_state == S_MORE)
		return;  /* cant show text in the editor, or when already viewing */

	m_prevs = m_state;
	m_state = S_MORE;

	m_page    = STRDUP(text);
	m_pagepos = m_page;
	m_prevpos = NULL;
	m_nlines  = 0;
	m_cline   = 0;

	for (char *p = m_page; *p; p++)
	{
		if (*p == '\n')
			m_nlines++;
	}

	//m_npages = nlines / (m_player ? m_player->m_nlines - 1 : 19);

	/* show the first page */
	page_one();
}

/*******************************************************************/

bool User::input(void)
{
	/* this method is called from within the server main loop to read
	 * input from the user */

	/*
	 * For unused C/C++ readers:
	 * This function may be hard to follow, and difficult to understand.
	 * What it does is to read a stream of bytes (possibly preprocessed
	 * by the TelnetSocket class), split the stream into different nodes
	 * at newline.  If there is any text left with no trailing newline,
	 * it is stored in the m_current text node.
	 */

	char   buf[MAX_BUFFER_LEN + 1];  /* +1 for ending '\0' character */
	char  *p, *p2;
	int    rs;  /* read size (how many byte to read) */
	int    as;  /* actual size (how many bytes were actually read?) */
	size_t l;
	textnode *tn;

	/* 1: misc. initialisation */
	if (m_current.t != NULL)
	{
		/* we allready have some text, append the new text behind the
		   existing */
		strcpy(buf, m_current.t);
		rs = MAX_BUFFER_LEN - m_current.l;
		p  = buf + m_current.l;
	}
	else
	{
		memset(buf, 0, MAX_BUFFER_LEN + 1);
		rs = MAX_BUFFER_LEN;
		p  = buf;
	}

	/* TOOD:  read text in a loop, so we can be sure to have at
	 *        least one newline?
	 */
	/* 2: get a chunk of text */
	as = m_socket.read(p, rs);
	if (as < 0)
	{
		if (errno == EAGAIN)
			return true;  /* no data available at the moment */
		perror("User::input");
		return false;
	}
	else if (as == 0)
		return false;  /* user has closed the connection */

	p[as] = 0;

	/* 3: split it into newlines */
	for (p = p2 = buf; *p; p++)
	{
		if (*p == '\n' || *p == '\r')
		{
			/* create a new node and add the text */
			if ((tn = new textnode) == NULL)
				out_of_memory();
			tn->l = p - p2;
			if ((tn->t = new char [tn->l + 1]) == NULL)
				out_of_memory();
			memcpy(tn->t, p2, tn->l);
			tn->t[tn->l] = 0;  /* terminate the string */

			/* add the node to the list */
			if (m_itail == NULL)
			{
                                /* first node in list */
				m_input = m_itail = tn;
			}
			else
			{
				m_itail->n = tn;
				m_itail    = tn;
			}

			if ((*p == '\n' && *(p + 1) == '\r') ||
				(*p == '\r' && *(p + 1) == '\n'))
			{
				p++;
			}

			p2 = (p + 1);
		}
	}

	/* 4: save remaining text */
	if (p2 < p)
	{
		/* we have text left, save it */
		l = (p - p2);// -1 to remove the '\0' character */
		if (m_current.l < l || m_current.t == NULL)
		{
			if (m_current.t != NULL)
				delete [] m_current.t;
			if ((m_current.t = new char [l + 1]) == NULL)
				out_of_memory();
		}

		memcpy(m_current.t, p2, l);
		m_current.l    = l;
		m_current.t[l] = 0;
	}
	else
	{
		if (m_current.t)
		{
			delete [] m_current.t;
			m_current.t = NULL;
			m_current.l = 0;
		}
	}

	return true;
}

bool User::output(void)
{
	/* this method is called from withing the server main loop to write
	 * queued output to the user
	 */

	textnode *tn;
	size_t    l;
	char      buf[512]; /* temporary buffer */
	char     *bufptr;   /* current position in buffer */
	size_t    buflen;   /* current length of buffer */

	if (!m_output)
		return true;  /* it's ok to have no queued output */

	memset(buf, 0, sizeof(buf));
	bufptr = buf;
	buflen = 0;

	if (!m_prompt)
	{
		/* prepend a newline */
		strcat(buf, "\r\n");
		bufptr  += 2;
		buflen  += 2;
	}

	/* for an unused C/C++ reader, this loop might be hard to follow,
	   but it should be rather effective, and it works too...  :) */
	for (tn = m_output; tn; tn = m_output)
	{
		if (!tn->p)
			tn->p = tn->t;

		for ( ; tn && tn->p && *tn->p ; )
		{
			if ((buflen + tn->l) < sizeof(buf) && tn->l < sizeof(buf))
			{
				memcpy(bufptr, tn->p, tn->l);
				bufptr += tn->l;
				buflen += tn->l;
				tn->p  += tn->l;
				tn->l  -= tn->l;

				if (tn->n == NULL)
					goto flush_buffer;  /* this is the last node */
			}
			else
			{
				/* buffer is full, flush it */
flush_buffer:
				if (tn->l >= sizeof(buf))
				{
					/* the text is to big to fit in one buffer */
					int len;

					if (bufptr == buf)
						len = sizeof(buf) - 1;  /* buffer is empty */
					else
						len = bufptr - buf - 1;

					memcpy(bufptr, tn->p, len);
					tn->p  += len;
					tn->l  -= len;
					buflen += len;
				}

				l = m_socket.write(buf, buflen);
				if (l < 0)
				{
					if (errno == EAGAIN)
					{
						/* store the current buffer */
						return true;
					}
					perror("User::output");
					return false;  /* something has gone very wrong */
				}
				else if (l == 0)
					return false;
				else if (l < buflen)
				{
					/* could not write all of the text */
					memmove(buf, buf + l, buflen - l);
					bufptr -= l;
					buflen -= l;
				}
				else
				{
					bufptr = buf;
					buflen = 0;
				}
			}
		}

		/* at this point, all text in the node have been written,
		   so we can delete the node and update the list head */
		m_output = tn->n;  /* remove node from list */
		if (m_output == NULL)  /* last node removed */
			m_otail = NULL;

		delete tn;
	}

	return true;
}

void User::add_textnode(const char *s, const size_t l,
						textnode **head, textnode **tail)
{
	textnode *tn;

	if ((tn = new textnode) == NULL)
		out_of_memory();

	tn->t = STRDUP(s);
	tn->l = l;

	if (*tail == NULL)
		*head = *tail = tn;  /* first node in list */
	else
	{
		(*tail)->n = tn;
		*tail     = tn;
	}
}

void User::flush_queue(textnode **head)
{
	textnode *tn;

	if (head && *head)
	{
		for (tn = *head; tn; tn = *head)
		{
			*head = tn->n;
			delete tn;
		}
	}
}

/*******************************************************************/

inline static bool is_enter(const textnode *tn)
{
	if (tn && tn->t && !*tn->t && !tn->l)
		return true;
	else
		return false;
}

inline static bool name_is_valid(const textnode *tn)
{
	if (!tn || !*tn->t || tn->l < 2 || tn->l > 20)
		return false;

	return true;
}

inline static bool pwd_is_valid(const textnode *tn, const char *name)
{
	if (!tn || !*tn->t || tn->l < 2 || tn->l > 20)
		return false;
	if (strcasecmp(tn->t, "password") == 0 ||
		strcasecmp(tn->t, "god"     ) == 0 ||
		strcasecmp(tn->t, name      ) == 0)
	{
		return false;
	}
	return true;
}

void User::s_getname(textnode *tn)
{
	Player *load_player(const char *pname);

	if (!name_is_valid(tn))
	{
		write("That name is not a valid one.\r\nWhat is your MUD name? ");
		return;
	}

	/* load the player */
	if ((m_player = load_player(tn->t)) == (Player *) -1)
	{
		if ((m_player = new Player) == NULL)
			out_of_memory();
		write("Did I get that right, %s (y/N)? ", tn->t);
		m_player->m_name = STRDUP(tn->t);
		m_state = S_CONFNAME;
		m_player->m_user = this;
	}
	else if (m_player == NULL)
	{
		log("Error loading player %s\n", tn->t);
		write("Your playerfile could not be loaded.\r\n"
			  "Contact the staff at %s.\r\n", mudconfig.admail);
		m_state = S_CLOSE;
	}
	else
	{
		write("Your password: ");
		m_state = S_GETPWD;
		m_player->m_user = this;
		m_socket.set_echo(false);
	}

	if (m_player)
	{
		char s[128];
		m_socket.get_host(s, 127);
		snprintf(s + strlen(s), sizeof(s) - strlen(s), ":%d",
				 m_socket.get_peer_port());
		if (m_player->m_host)
			delete [] m_player->m_host;
		m_player->m_host = STRDUP(s);
	}
}

bool User::check_dups(void)
{
	User *u;

	/* see if the player is allready logged on */
	for (u = server.m_users; u; u = u->m_next)
	{
		if (u == this)
			continue;
		if (u->m_player && u->m_player != m_player)
		{
			if (*u->m_player->get_name() == *m_player->get_name() &&
				strcasecmp(u->m_player->get_name(), m_player->get_name()) == 0)
			{
				/* found a duplicate */
				u->write("Multipple login detected, disconnecting.\r\n");
				break;
			}
		}
	}

	if (u != NULL && u->m_player != NULL)
	{
		delete m_player;

		m_player    = u->m_player;
		m_state     = u->get_state();
		u->m_player = NULL;
		u->set_state(S_CLOSE);
		m_player->m_user = this;

		write("Reconnecting to existing player.\r\n\r\n");
		return true;
	}

	return false;  /* continue login sequence */
}

static const char *hints1 =
"Now you can select two \"hints\" to be used when "
"rolling your characters\r\nstats.  You can select"
"two different hints, the same hint twice,\r\nonly "
"one hit, or no hints at all.\r\n\r\n"
"Please select one of the following hints:\r\n"
"  1) Strength      2) Intelligence     3) Wisdom\r\n"
"  4) Dexterity     5) Constitution     6) Charisma\r\n"
"\r\n"
"  7) Good with weapons       8) Good at assasinations\r\n"
"  9) Goot at thievery       10) Good at hunting\r\n"
" 11) Good at spellcasting   12) Good with nature\r\n"
" 13) Goot with animals      14) Good contact with the gods\r\n"
" 15) Good at magic\r\n";

static const char *hints2 =
"Please select one of the following hints:\r\n"
"  1) Strength      2) Intelligence     3) Wisdom\r\n"
"  4) Dexterity     5) Constitution     6) Charisma\r\n"
"\r\n"
"  7) Good with weapons       8) Good at assasinations\r\n"
"  9) Goot at thievery       10) Good at hunting\r\n"
" 11) Good at spellcasting   12) Good with nature\r\n"
" 13) Goot with animals      14) Good contact with the gods\r\n"
" 15) Good at magic\r\n";

/* take (and remove) a textnode from the input queue,
 * then do something with the node
 */
void User::handle_input(void)
{
	/* TODO:  Try to make this function smaller?
	 *        Maybe by using one function for each state?
	 */

	textnode *tn;
	char      b[128];

	if (!m_input || m_state == S_CLOSE)
		return;  /* user has no input, or is about to be thrown out */

	for (tn = m_input; tn; tn = m_input)
	{
		switch (m_state)
		{
		case S_PLAYING:     /* the user is playing (normal state) */
			if (!is_enter(tn))
				m_player->interpret(tn->t, tn->l);
			if (m_state == S_PLAYING)
			{
				m_player->write_prompt();
				m_prompt = true;
			}
			break;
		case S_CLOSE:       /* the user is about to be thrown out */
			break;
		case S_MORE:        /* the user is viewing a long text */
			page_cmd(tn);
			break;
		case S_EDIT:        /* the user is editing a text */
			break;
		case S_GETNAME:     /* getting name */
			if (is_enter(tn))
				m_state = S_CLOSE;
			else
				s_getname(tn);
			break;
		case S_GETPWD:      /* getting password */
			if (is_enter(tn))
				m_state = S_CLOSE;
			else
			{
				if (strcmp(m_player->m_passwd,
						   crypt(tn->t, m_player->m_name)) != 0)
				{
					if ((++m_player->m_npasswd % 3) == 0)
					{
						write("\r\nTo many failures.\r\nThis event has been "
							  "logged with your ip-address.\r\n");
						log("Three password attempts from: %s (%s)\n",
							m_player->get_name(), m_socket.get_host(b, 127));
						m_state = S_CLOSE;
					}
					else
						write("\r\nWrong password.\r\nYour password: ");
					m_player->save();
				}
				else
				{
					if (!check_dups())
					{
						m_state = S_MOTD;
						write(mudconfig.motd);
						write("\r\n[Press ENTER to continue]");
					}
					m_socket.set_echo(true);
				}
			}
			break;
		case S_MOTD:        /* reading the message of the day */
			if (m_player->m_npasswd)
			{
				write("\r\n\r\n\007\007\007"
					  "%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL "
					  "LOGIN!\r\n\r\n", m_player->m_npasswd,
					  m_player->m_npasswd > 1 ? "S" : "");
				m_player->m_npasswd = 0;
			}
			m_player->save();
			m_state = S_MENU;
			write(mudconfig.menu);
			break;
		case S_MENU:        /* sitting at the login menu */
			menu_pick(tn);
			break;

			/* states for creating new players */
		case S_CONFNAME:    /* is this really the users name? */
			if (tolower(*tn->t) == 'y' || strcasecmp(tn->t, "yes") == 0)
			{
				write("\r\n"
"Ah, a new player!  Wellcome to %s!\r\n\r\n"
"You will now be asked a series of questions, of which only some are\r\n"
"required.  When this is done, you will be presented with a menu where\r\n"
"you can quit the MUD, read tha background story, and (of course), enter\r\n"
"the MUD world.\r\n\r\n"
"First you must select a password for your character to have.  This is to\r\n"
"not let any other people use your character, and is a required question\r\n"
"Password for your character: ", mudconfig.mudname);
				m_state = S_GETNEWPWD;
			}
			else
			{
				write("Well, what is your name then? ");
				delete m_player;
				m_player = NULL;
				m_state = S_GETNAME;
			}
			break;
		case S_GETNEWPWD:   /* get the new users password */
			if (is_enter(tn))
				m_state = S_CLOSE;
			else if (!pwd_is_valid(tn, m_player->m_name))
			{
				write("That is not a valid password here on %s.\r\n"
					  "Password for your character: ", mudconfig.mudname);
			}
			else
			{
				m_player->m_passwd = STRDUP(tn->t);
				m_state = S_CONFNEWPWD;
				write("\r\nConfirm your password: ");
			}
			break;
		case S_CONFNEWPWD:  /* let the user retype the password */
			if (is_enter(tn))
				m_state = S_CLOSE;
			else if (strcmp(tn->t, m_player->m_passwd) != 0)
			{
				write("Passwords does not match.\r\n"
					  "Password for your character: ");
				m_state = S_GETNEWPWD;
			}
			else
			{
				delete [] m_player->m_passwd;
				if ((m_player->m_passwd =
					 STRDUP(crypt(tn->t, m_player->m_name))) == NULL)
				{
					out_of_memory();
				}
				write("\r\n"
"Now we want to know your real name.  If you want to keep is a secret,\r\n"
"just press enter at the prompt.\r\n\r\n"
"Your real name: ");
				m_state = S_GETREALNAME;
			}
			break;
		case S_GETREALNAME: /* get the users real name */
			if (!is_enter(tn))
				m_player->m_rlname = STRDUP(tn->t);
			m_state = S_GETEMAIL;
			write("\r\n"
"Here you can enter your email address.  As with your real name, you can\r\n"
"keep your address secret by just pressing enter.\r\n\r\n"
"Your email address: ");
			break;
		case S_GETEMAIL:    /* get the users email address */
			if (!is_enter(tn))
				m_player->m_email = STRDUP(tn->t);
			m_state = S_GETSEX;
			write("\r\n"
				  "Now you must select the sex you want your "
				  "character to be.\r\n"
				  "  [M]ale\r\n"
				  "  [F]emale\r\n"
				  "  [N]eutral (no sex at all)\r\n"
				  "Pick a sex: ");
			break;
		case S_GETSEX:      /* get the players prefered sex */
			if (!is_enter(tn))
			{
				if (toupper(*tn->t) == 'M')
					m_player->m_sex = Character::SEX_MALE;
				else if (toupper(*tn->t) == 'F')
					m_player->m_sex = Character::SEX_FEMALE;
				else if (toupper(*tn->t) == 'N')
					m_player->m_sex = Character::SEX_NEUTRAL;
				else
				{
					write("\r\n"
						  "That is not a valid sex.\r\n"
						  "  [M]ale\r\n"
						  "  [F]emale\r\n"
						  "  [N]eutral (no sex at all)\r\n"
						  "Pick a sex: ");
					break;
				}
			}
			else
			{
				write("\r\n"
					  "That is not a valid sex.\r\n"
					  "  [M]ale\r\n"
					  "  [F]emale\r\n"
					  "  [N]eutral (no sex at all)\r\n"
					  "Pick a sex: ");
				break;
			}
			m_state = S_SELHINTS1;
			write("\r\n%s\r\nSelect your first hint: ", hints1);
			break;
		case S_SELHINTS1:   /* selecting hints for the stat roll */
			if (!is_enter(tn) && isdigit(*tn->t))
				m_player->m_hint1 = (hint_t) atoi(tn->t);

			m_state = S_SELHINTS2;
			write("\r\n%s\r\nSelect your second hint: ", hints2);
			break;
		case S_SELHINTS2:   /* selecting more hints for the stat roll */
			if (!is_enter(tn) && isdigit(*tn->t))
				m_player->m_hint2 = (hint_t) atoi(tn->t);

			m_state = S_ROLLSTATS1;
			write("\r\n"
"Now that you have selected two hints (or less), the MUD will roll your\r\n"
"stats and other things like age and such.\r\n"
"If you are not satisfied with the roll you may do another one.\r\n"
"\r\nRolling...\r\n");
			m_player->roll_stats();
			m_player->show_rolls();
			write("\r\nAre satisfied with the rolls? (y/N) ");
			break;
		case S_ROLLSTATS1:   /* rolling stats */
			if (is_enter(tn) || toupper(*tn->t) != 'Y')
			{
				write("Do you wish to select new hints? (y/N) ");
				m_state = S_ROLLSTATS2;
			}
			else
			{
				m_player->save();
				m_state = S_READRULES;
				write(mudconfig.rules);
				write("\r\n[Press ENTER to continue]");
			}
			break;
		case S_ROLLSTATS2:   /* rolling stats */
			if (is_enter(tn) || toupper(*tn->t) != 'Y')
			{
				m_player->roll_stats();
				m_player->show_rolls();
				write("\r\nAre satisfied with the rolls? (y/N) ");
				m_state = S_ROLLSTATS1;
			}
			else
			{
				write("\r\n%s\r\nSelect your first hint: ", hints1);
				m_state = S_SELHINTS1;
			}
			break;
		case S_READRULES:   /* let the user read the rules of this mud */
			m_state = S_MOTD;
			write(mudconfig.motd);
			write("\r\n[Press ENTER to continue]");
			syslog(LOG_LOGIN, "Player %s created", m_player->get_name());
			break;

			/* sub-menu states */
		case S_READBKG:     /* reading the background story */
			break;
		case S_CHGPWD1:     /* changing password; enter old password */
			break;
		case S_CHGPWD2:     /* changing password; enter new password */
			break;
		case S_CHGPWD3:     /* changing password; confirm new password */
			break;

			/* all other states */
		case S_NONE:        /* the user is in no specific state at all */
			if (m_player)
				log("User [%s@%s] in the NONE state\n",
					m_player->m_name, m_socket.get_host(b, 127));
			else
				log("User [@%s] in the NONE state\n",
					m_socket.get_host(b, 127));
			m_state = S_CLOSE;
			break;

		default:
			if (m_player)
				log("User [%s@%s] in unknown state: %d\n",
					m_player->m_name, m_socket.get_host(b, 127), m_state);
			else
				log("User [@%s] in unknown state: %d\n",
					m_socket.get_host(b, 127), m_state);
			m_state = S_CLOSE;
			break;
		}

		m_input = tn->n;  /* remove node from list */
		if (m_input == NULL)  /* last node removed */
			m_itail = NULL;

		delete tn;
	}
}

void User::menu_pick(const textnode *tn)
{
	Location *to = NULL;

	if (!tn || is_enter(tn) || tn->l != 1)
	{
		write(mudconfig.menu);
		return;
	}

	/* the text for the menu is in mudconfig.cc */
	switch (*tn->t)
	{
	case '0':  /* "0:  Quit from Shadow-99!\r\n" */
		write("\r\nFarewell voyager!  Come back soon.\r\n\r\n");
		m_player->save();
		m_state = S_CLOSE;
		break;
	case '1':  /* "1:  Enter Shadow-99!\r\n" */
		write("\r\n... and they entered a world of shadows...\r\n\r\n");
		if (m_player->m_level <= 0)
			//m_player->advance();
			m_player->m_level = 1;
		m_state = S_PLAYING;
		to = m_player->is_wizard() ? mudconfig.wstart : mudconfig.mstart;
		if (to)
		{
			syslog(LOG_LOGIN, "Player %s (%s) entered the game",
				   m_player->get_name(), m_player->m_host);
			to->write(m_player, "%s has entered the game.\r\n",
					  strcap(m_player->get_name()));
			m_player->move(to);
			to->describe(m_player, DESC_LONG);
		}
		m_prompt = false;
		break;
	case '2':  /* "2:  Read background story\r\n" */
		page(mudconfig.bginfo);
		break;
	default:
		write("Illegal choice.\r\n");
		write(mudconfig.menu);
		break;
	}
}

/*******************************************************************/

void User::page_one(void)
{
	/* this pager is pretty slow, as it only writes one line at a time */
	char  line[128];  /* no line should ever be over 80 characters */
	char *p1, *p2;
	int   l, n;

	//n = m_player ? m_player->m_nlines - 1 : 19;
	n = 19;

	for (l = 0; l < n && (m_cline + l) < m_nlines; l++)
	{
		for (p1 = m_pagepos, p2 = line; *p1 && p2 < (line + 127); )
		{
			if (*p1 == '\n')
				break;
			else if (!isprint(*p1))
				p1++;
			else
				*p2++ = *p1++;
		}

		m_pagepos = ++p1;
		*p2 = 0;

		write("%s\r\n", line);
	}
	m_cline += l;
	if (m_pagepos < (m_page + strlen(m_page)) && m_cline < m_nlines)
		write("[ENTER = next page; Q = quit pager]");
	else
	{
		delete [] m_page;
		m_state = m_prevs;

		if (m_state == S_MENU)
			write(mudconfig.menu);
	}
}

void User::page_cmd(textnode *tn)
{
	if(is_enter(tn))
		page_one();
	else if (tn->t && tolower(*tn->t) == 'q')
	{
		delete [] m_page;
		m_state = m_prevs;

		if (m_state == S_MENU)
			write(mudconfig.menu);
	}
}

/*******************************************************************/

static void to_colour(char **from, char **to)
{
	int  len;
	char tmp[16];

	/* we are guarantueed that there is at least one digit in f */

	if (**from == '0')
		sprintf(tmp, "\x1B[0m");
	else
		sprintf(tmp, "\x1B[3%cm", **from);
	len = strlen(tmp);
	memcpy(*to, tmp, len);
	*to   += len;
	*from += 1;
}

void User::parse_colour(char *from, char *to)
{
	register char *f = from;
	register char *t = to;

	if (!m_player || !(m_state == S_PLAYING || m_prevs == S_PLAYING))
	{
		strcpy(to, from);
		return;
	}

	while (*f)
	{
		if (*f == '&')
		{
			f++;

			switch (toupper(*f))
			{
			case '&':
				*t++ = *f++;
				break;
			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
				if (!m_player->m_colour)
					f++;
				else
					to_colour(&f, &t);
				break;

			default:
				*t++ = *f++;
				break;
			}
		}
		else
			*t++ = *f++;
	}
	*t = 0;
}

/*******************************************************************/
