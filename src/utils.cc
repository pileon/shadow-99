/********************************************************************
* File: utils.cc                                  Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"

/*******************************************************************/

/* these two functions taken from CircleMUD 3.0 */

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
	/* error checking in case people call number() incorrectly */
	if (from > to)
	{
		int tmp = from;
		from = to;
		to = tmp;
	}

	return ((RANDOM() % (to - from + 1)) + from);
}

/* simulates dice roll */
int dice(register int num, register int size)
{
	register int sum = 0;

	if (size <= 0 || num <= 0)
		return 0;

	while (num-- > 0)
		sum += ((RANDOM() % size) + 1);

	return sum;
}



/*******************************************************************/
