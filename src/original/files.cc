/********************************************************************
* File: files.cc                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"

#include <sys/stat.h>
#include <limits.h>

/*******************************************************************/

/* This functions checks if a file or directory exists.
 * It returns a value bigger than 0 when it exists and is a
 * regular file, a value lower than 0 if it exists and is a
 * directory, and 0 if it doesn't exist.
 */
int file_exists(const char *file)
{
	struct stat st;

	if (file != NULL)
	{
		if (stat(file, &st) < 0)
		{
			if (errno != ENOENT)
				perror(file);
			return 0;
		}

		if (S_ISDIR(st.st_mode))
			return -1;
		else if (S_ISREG(st.st_mode))
			return 1;
		else
			return 0;
	}

	return 0;
}

/* Get the size of a specified file.
 * Returns either the size of the file, or -1 on an error.
 */
int file_size(const char *file)
{
	struct stat st;

	if (file != NULL)
	{
		if (stat(file, &st) < 0)
			return -1;

		if (S_ISREG(st.st_mode))
			return st.st_size;
		else
		{
			errno = S_ISDIR(st.st_mode) ? EISDIR : EINVAL;
			return -1;
		}
	}

	return -1;
}

char *resolve_path(const char *from)
{
	/* *2 to be on the safe side... or? */
	static char tpath[_POSIX_PATH_MAX * 2] = { 0 };
	char fpath[_POSIX_PATH_MAX * 2] = { 0 };

	/* get our current path */
	if (getcwd(fpath, _POSIX_PATH_MAX) == NULL)
	{
		perror("resolve_path: getcwd");
		return NULL;
	}

	if (fpath[strlen(fpath) - 1] == '/')
	{
		if (*from == '/')
			fpath[strlen(fpath) - 1] = 0;
	}
	else
	{
		if (*from != '/')
		{
			fpath[strlen(fpath)    ] = '/';
			fpath[strlen(fpath) + 1] = 0;
		}
	}

	/* concatenate the two paths */
	strcat(fpath, from);

	/* and resolve path */
	return realpath(fpath, tpath);
}

/*******************************************************************/
