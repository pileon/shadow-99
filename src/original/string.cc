/********************************************************************
* File: string.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
*********************************************************************
*                                                                   *
* This file contains the implementation of the String class.        *
* It also contains misc. string handlign functions.                 *
*                                                                   *
********************************************************************/

#include "shadow.h"

/*******************************************************************/

/* Strip leading spaces.  If end is true, strip ending spaces as well. */
char *strip_spaces(char **str, const bool end /* = true */)
{
	for (; **str && isspace(**str); (*str)++)
		;

	if (end)
	{
		char *p = *str + strlen(*str) - 1;/* -1 to skip the ending NULL char */
		while (isspace(*p))
			*p-- = 0;
	}

	return *str;
}

char *shadow_strdup(const char *from)
{
	char   *to;
	size_t  len;

	if (from)
	{
		len = strlen(from) + 1;
		if ((to = new char [len]) == NULL)
			out_of_memory();
		memcpy(to, from, len);
		return to;
	}
	else
	{
		errno = EINVAL;
		return NULL;
	}
}

char *strlower(const char *from, char *to, const int maxlen)
{
	register const char *f = from;
	register char *t = to;
	register int   l = 0;

	for ( ; *f && l < maxlen; l++)
		*t++ = tolower(*f++);
	*t = 0;  /* terminate the string */

	return to;
}

char *strupper(const char *from, char *to, const int maxlen)
{
	register const char *f = from;
	register char *t = to;
	register int   l = 0;

	for ( ; *f && l < maxlen; l++)
		*t++ = toupper(*f++);
	*t = 0;  /* terminate the string */

	return to;
}

char *strcap(const char *str)
{
	static char cap[1024];

	strncpy(cap, str, 1023);
	cap[0] = toupper(cap[0]);
	return cap;
}

/*******************************************************************/

/* this function is taken from CircleMUD 3.0 */
char *fname(const char *namelist)
{
	static char holder[128];
	register char *point;

	for (point = holder; isalpha(*namelist); namelist++, point++)
		*point = *namelist;

	*point = '\0';

	return holder;
}

/* this function is taken from CircleMUD 3.0 */
const bool is_name(const char *str, const char *namelist)
{
	register char *curname, *curstr;

	curname = (char *) namelist;

	for (;;)
	{
		for (curstr = (char *) str; ; curstr++, curname++)
		{
			if (!*curstr && !isalnum(*curname))
				return true;

			if (!*curname)
				return false;

			if (!*curstr || *curname == ' ')
				break;

			if (tolower(*curstr) != tolower(*curname))
				break;
		}

		/* skip to next name */

		for (; isalnum(*curname); curname++)
			;

		if (!*curname)
			return false;

		curname++;  /* first char of new name */
	}
}

const bool isname(register const char *str, register const char *namelist)
{
	bool ok = true;
	char lookword[512];
	register char *lw = lookword;

	str = one_argument(str, lookword);

	while (ok && *lookword)
	{
		if(is_name(lw, namelist))
			str = one_argument(str, lw);
		else
			ok = false;
	}

	return ok;
}

/*
 * copy the first space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 *
 * this function is taken from CircleMUD 3.0
 */
char *one_argument(const char *argument, char *first_arg)
{
	register char *arg = (char *) argument;

	strip_spaces(&arg, false);

	while (*arg && !isspace(*arg))
	{
		*(first_arg++) = tolower(*arg);
		arg++;
	}

	*first_arg = '\0';

	return arg;
}

/* check if a arg1 is an abbreviation of arg2
 *
 * taken from CircleMUD 3.0
 */
const bool is_abbrev(register const char *arg1, register const char *arg2)
{
	if (!*arg1)
		return false;

	for (; *arg1 && *arg2; arg1++, arg2++)
	{
		if (tolower(*arg1) != tolower(*arg2))
			return false;
	}

	return (!*arg1);
}
