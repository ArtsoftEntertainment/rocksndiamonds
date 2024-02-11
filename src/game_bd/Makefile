# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2024 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  https://www.artsoft.org/
# -----------------------------------------------------------------------------
# The native Boulder Dash game engine is based on:
# - GDash by Czirkos Zoltan (2010)
# -----------------------------------------------------------------------------
# src/game_bd/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	main_bd.c	\
	bd_cave.c	\
	bd_cavedb.c	\
	bd_caveengine.c	\
	bd_caveobject.c	\
	bd_bdcff.c	\
	bd_caveset.c	\
	bd_c64import.c	\
	bd_gameplay.c	\
	bd_graphics.c	\
	bd_sound.c

OBJS =	main_bd.o	\
	bd_cave.o	\
	bd_cavedb.o	\
	bd_caveengine.o	\
	bd_caveobject.o	\
	bd_bdcff.o	\
	bd_caveset.o	\
	bd_c64import.o	\
	bd_gameplay.o	\
	bd_graphics.o	\
	bd_sound.o

GAME_BD = game_bd.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(GAME_BD)

$(GAME_BD): $(OBJS)
	$(AR) cr $(GAME_BD) $(OBJS)
	$(RANLIB) $(GAME_BD)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(GAME_BD)


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
