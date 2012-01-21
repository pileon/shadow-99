#####################################################################
# File: libgai/Makefile                           Part of Shadow-99 #
#####################################################################
#
#

CC     = c++
CFLAGS = -ggdb3 -Wall -Wextra
RANLIB = ranlib
AR     = ar cr

#####################################################################

CFILES = ga_aistruct.c ga_clone.c ga_echeck.c ga_nsearch.c ga_port.c \
	ga_serv.c ga_unix.c gai_strerror.c gn_ipv46.c
OFILES = $(CFILES:.c=.o)
TARGET = libgai.a

CFILES2 = getaddrinfo.c freeaddrinfo.c getnameinfo.c
OFILES2 = $(CFILES2:.c=.o)

#####################################################################

default: $(TARGET)

someclean:
	-rm -f *~

clean: someclean
	-rm -f $(OFILES)
	-rm -f $(OFILES2)

realclean: clean
	-rm -f $(TARGET)
	-rm -f .depend
	-touch .depend

#####################################################################

$(TARGET): $(OFILES)
	$(AR) $(TARGET) $(OFILES)
	$(RANLIB) $(TARGET)

#####################################################################

depend:
	$(CC) -E -MM $(CFLAGS) $(CFILES) $(CFILES2) > .depend

-include .depend

#####################################################################
