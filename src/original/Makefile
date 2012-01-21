#####################################################################
# File: Makefile                                  Part of Shadow-99 #
#                                                                   #
# This file is copyright (C) 1999, 2012 by Joachim Pileborg.        #
# All rights reserved.  See COPYING for details.                    #
#####################################################################
#
#

CXX      = c++
CC       = c++
LEX      = flex -i
YACC     = bison -y -d -v
CXXFLAGS = -ggdb3 -Wall -Wshadow -Wstrict-prototypes
CFLAGS   = -ggdb3 -Wall -Wshadow -Wstrict-prototypes
LD       = c++
LDFLAGS  = -ggdb3 -Wall
LIBS     = -ldmalloc -lcrypt

#####################################################################
#
#

CXXFILES = \
	shadow.cc    \
	dmalloc.cc   \
	socket.cc    \
	telnet.cc    \
	server.cc    \
	signal.cc    \
	mudconfig.cc \
	string.cc    \
	files.cc     \
	user.cc      \
	boot.cc      \
	comm.cc      \
	object.cc    \
	area.cc      \
	zone.cc      \
	lexer.cc     \
	parser.cc    \
	character.cc \
	player.cc    \
	loader.cc    \
	factory.cc   \
	location.cc  \
	wizard.cc    \
	skills.cc    \
	guilds.cc    \
	utils.cc     \
	interp.cc    \
	monster.cc   \
	item.cc      \
	heartbeat.cc \
	cmd_info.cc  \
	cmd_move.cc  \
	cmd_spells.cc\
	cmd_skills.cc\
	cmd_comm.cc  \
	cmd_item.cc

CFILES   =

OFILES   = $(CXXFILES:.cc=.o) $(CFILES:.c=.o)
TARGET   = shadow
GAI      = libgai.a getnameinfo.o
LIBGAI   = gai/getnameinfo.o gai/libgai.a

#####################################################################

default: $(TARGET)

all: $(TARGET)

rebuild: clean all

someclean:
	cd gai; $(MAKE) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' someclean
	-rm -f *~

clean: someclean
	cd gai; $(MAKE) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' clean
	-rm -f $(OFILES)

realclean: clean
	cd gai; $(MAKE) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' realclean
	-rm -f $(TARGET)
	-rm -f .depend
	-touch .depend

#####################################################################

$(TARGET): allways $(OFILES)
	cd gai; $(MAKE) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' $(GAI)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OFILES) $(LIBGAI) $(LIBS)

allways:
	@-rm -f $(TARGET)

#####################################################################

#depend: lexer.cc parser.cc parser.h
depend:
	cd gai; $(MAKE) 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' depend
	$(CXX) -E -MM $(CXXFLAGS) $(CXXFILES) $(CFILES) > .depend

-include .depend

#####################################################################
