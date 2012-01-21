/********************************************************************
* File: wizard.cc                                 Part of Shadow-99 *
*                                                                   *
* This file is copyright (C) 1999, 2012 by Joachim Pileborg.        *
* All rights reserved.  See COPYING for details.                    *
********************************************************************/

#include "shadow.h"
#include "player.h"

/*******************************************************************/

Wizard::Wizard()
	: Player()
{
}

Wizard::~Wizard()
{
}

Archwiz::Archwiz()
	: Wizard()
{
}

Archwiz::~Archwiz()
{
}

void Wizard::init(void)
{
	Player::init();
}

void Archwiz::init(void)
{
	Wizard::init();
}

/*******************************************************************/
