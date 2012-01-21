#ifndef __STRING_H__
#define __STRING_H__
/********************************************************************
* File: string.h                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

/*******************************************************************/

char *strip_spaces(char **str, const bool end = true);
char *shadow_strdup(const char *from);
char *strlower(const char *from, char *to, const int maxlen);
char *strupper(const char *from, char *to, const int maxlen);
char *strcap  (const char *str);
char *fname  (const char *namelist);
const bool is_name(const char *str, const char *namelist);
const bool isname (const char *str, const char *namelist);
char *one_argument(const char *argument, char *first_arg);
const bool is_abbrev(const char *arg1, const char *arg2);

#define STRDUP(s) shadow_strdup(s)
#define UPPER(c) ((c) >= 'a' && (c) <= 'z' ? (c) + ('A' - 'a') : (c))
#define LOWER(c) ((c) >= 'A' && (c) <= 'Z' ? (c) + ('a' - 'A') : (c))
#define CAP(s)   (*(s) = UPPER(*(s)), s)
#define ANA(s)   (strchr("aeiouAEIOU", *s) ? "an" : "a")

/*******************************************************************/
#endif /* __STRING_H__ */
