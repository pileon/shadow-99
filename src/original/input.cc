/********************************************************************
* File: input.cc                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

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
	int       n;

	if (!m_input || m_state == S_CLOSE)
		return;  /* user has no input, or is about to be thrown out */

	/*
	if (!m_player)
	{
		write("An internal error have ocurred.\r\n"
			  "Please contact the MUD administatrators at: %s\r\n"
			  "(Remember to include your MUD name, and what you did "
			  "before this error.)\r\n", mudconfig.admail);
		log("User [%s] without player!\n", m_socket.get_host(b, 127));
		m_state = S_CLOSE;
	}
	*/

	for (tn = m_input; tn; tn = m_input)
	{
		switch (m_state)
		{
		case S_PLAYING:     /* the user is playing (normal state) */
			if (is_enter(tn))
				break;      /* only an empty line */
			//m_player->interpret(tn->t, tn->l);
			//m_player->write_prompt();
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
			{
				if (!name_is_valid(tn))
				{
					write("That name is not valid here.\r\n"
						  "What is your MUD name: ");
					break;
				}

				n = load_player(tn->t);
				if (n < 0)
				{
					/* playerfile could not be loaded */
					write("\r\n"
						  "Your playerfile could not be loaded\r\n"
						  "Email to %s, stating the art of your problem.\r\n"
						  "(Remember to include your MUD name.)\r\n\r\n",
						  mudconfig.admail);
					m_state = S_CLOSE;
					break;
				}
				else if (n == 0)
				{
					/* no player/wizard found, is this a new
					   one? */
					write("That name is not know here on Shadow-99.\r\n"
						  "Is '%s' really your name (Y/N)? ", tn->t);
					m_state = S_CONFNAME;
				}
				else
				{
					write("Enter your password: ");
					m_state = S_GETPWD;
				}
			}
			break;
		case S_GETPWD:      /* getting password */
			if (is_enter(tn))
				m_state = S_CLOSE;
			else
			{
				if (strncmp(m_player->m_passwd,
							crypt(tn->t, m_player->get_name()), 13))
				{
					if (++m_npasswd >= 3)
						m_state = S_CLOSE;
					else
					{
						m_player->m_npasswd++;
						write("Wrong password.  Try again.\r\nPassword: ");
					}
				}
				else
				{
					write("\r\n<Press enter to continue.>");
					m_state = S_MOTD;
				}
			}
			break;
		case S_MOTD:        /* reading the message of the day */
			if (is_enter(tn))
			{
				if (m_player->m_npasswd)
				{
					write("\r\n\r\n\007\007\007"
						  "%d LOGIN FAILURE%s SINCE LASTSUCCESSFUL "
						  "LOGIN!\r\n\r\n", m_player->m_npasswd,
						  m_player->m_npasswd > 1 ? "S" : "");
					m_player->m_npasswd = 0;
				}

				write(mudconfig.menu);
				m_state = S_MENU;
			}
			break;
		case S_MENU:        /* sitting at the login menu */
			menu_pick(tn);
			break;
		case S_CONFNAME:    /* is this really the users name? */
			if (tolower(*tn->t) == 'y' || !strcasecmp(tn->t, "yes"))
			{
				write(
					"Wellcome, new player %s!\r\n"
					"You will now be asked a series of questions, of which "
					"some can be leaved\r\nas blanks.\r\n"
					"\r\n"
					"First you need a password for your character.  "
					"It can be max 13 characters.\r\n"
					"What do you want your password to be: ",
					strcap(m_player->get_name()));
				m_state = S_GETNEWPWD;
			}
			else
			{
				write("Well then, what is your MUD name: ");
				m_state = S_GETNAME;
			}
			break;
		case S_GETNEWPWD:   /* get the new users password */
			if (is_enter(tn))
				m_state = S_CLOSE;
			else if (!pwd_is_valid(tn))
			{
				write("That password is not valid.\r\n"
					  "What do you want your password to be:");
			}
			else
			{
				/* store in plaintext only until the password is confirmed */
				strncpy(m_player->m_passwd, tn->t, 13);
				m_player->m_passwd[13] = 0;
				m_state = S_CONFNEWPWD;

				write("You must now confirm your password.  "
					  "This is to ensure that you didn't\r\n"
					  "made any typos, and to help you remember "
					  "the password better.\r\n"
					  "Confirm your password: ");
			}
			break;
		case S_CONFNEWPWD:  /* let the user retype the password */
			if (strncmp(tn->t, m_player->m_passwd, 13))
			{
				write("Passwords doesn't match!\r\n"
					  "Enter your desired password: ");
				m_state = S_GETNEWPWD;
			}
			else
			{
				strncpy(m_player->m_passwd,
						crypt(tn->t, m_player->get_name()), 13);
				m_player->m_passwd[13] = 0;
				m_state = S_GETREALNAME;

				write("You may now enter your REAL name.\r\n"
					  "(You can keep is secret by just pressing ENTER.)\r\n"
					  "Your REAL name: ");
			}
			break;
		case S_GETREALNAME: /* get the users real name */
			write("You will now have the possability to enter your email "
				  "address.\r\n(As with your real name, just press enter "
				  "to keep it secret.)\r\nYou email address: ");
			m_state = S_GETEMAIL;
			break;
		case S_GETEMAIL:    /* get the users email address */
			m_player->save();
			m_state = S_READRULES;
			write("\r\n<Press enter to continue.>");
			break;
		case S_READRULES:   /* let the user read the rules of this mud */
			if (is_enter(tn))
			{
				write("\r\n<Press enter to continue.>");
				m_state = S_MOTD;
			}
			break;
		case S_READBKG:     /* reading the background story */
			//if (is_enter(tn))
			//      m_state = S_MENU;
			break;
		case S_CHGPWD1:     /* the user want to change his password */
			break;
		case S_CHGPWD2:     /* let the user retype his new password */
			break;
		case S_NONE:        /* the user is in no specific state at all */
			log("User [%s@%s] in the NONE state\n",
				m_player->get_name(), m_socket.get_host(b, 127));
			m_state = S_CLOSE;
			break;

		default:
			log("User [%s@%s] in unknown state: %d\n",
				m_player->get_name(), m_socket.get_host(b, 127),
				m_state);
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
	COMMAND(do_look);

	if (!tn || is_enter(tn) || tn->l != 1)
	{
		write(mudconfig.menu);
		return;
	}

	/* the text for the menu is in mudconfig.cc */
	switch (*tn->t)
	{
	case '0':  /* "0:  Quit from Shadow-99!\r\n" */
		m_player->save();
		m_state = S_CLOSE;
		break;
	case '1':  /* "1:  Enter Shadow-99!\r\n" */
		m_player->save();
		m_state = S_PLAYING;
		if (server.m_locs)
		{
			m_player->m_parent = server.find_object(m_player->is_wizard() ?
													mudconfig.room_wstart :
													mudconfig.room_mstart);
		}
		write("\r\n\r\nWellcome to the world of shadows!\r\n\r\n");
		do_look(m_player, 0, NULL, 0);
		m_player->write_prompt();
		break;
	case '2':  /* "2:  Read background story\r\n" */
		page(mudconfig.bginfo);
		//write("\r\n<Press enter to continue.>");
		//m_state = S_READBKG;
		break;
	default:
		write("Illegal choice.\r\n");
		write(mudconfig.menu);
		break;
	}
}
