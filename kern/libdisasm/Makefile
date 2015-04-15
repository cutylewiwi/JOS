LIBNAME	= libdisasm.so
INCLUDE 	= -I.
FLAGS 	= $(INCLUDE)  -O3 -ggdb
LFLAGS 	= $(FLAGS) -fPIC -shared -Wl,-soname,$(LIBNAME)
CC			= gcc

SRC		= 	libdis.c \
				i386.c

all: libdis quikdis

libdis:
	$(CC) $(LFLAGS) $(SRC) -o $(LIBNAME)

quikdis: libdis
	$(CC) $(FLAGS) -L. -ldisasm quikdis.c -o quikdis
