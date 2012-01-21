#ifndef __FILES_H__
#define __FILES_H__
/********************************************************************
* File: files.h                                   Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

/*******************************************************************/

int   file_exists(const char *file);
int   file_size(const char *file);
char *resolve_path(const char *from);

/*******************************************************************/
#endif /* __FILES_H__ */
