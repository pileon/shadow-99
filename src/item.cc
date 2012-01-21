/********************************************************************
* File: item.cc                                   Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "item.h"
#include "character.h"

/*******************************************************************/

Item::Item()
	: Object()
{
	m_equiped = false;
}

Item::~Item()
{
}
/*******************************************************************/

void Item::describe(Character *to, const descr_t typ /* = DESC_SHORT */)
{
	if (typ == DESC_SHORT)
		to->write("%s\r\n", strcap(get_short()));
	else if (typ == DESC_LONG)
		to->write(get_long());
	else if (typ == DESC_DESC)
		to->write(get_descr());
}

/*******************************************************************/
