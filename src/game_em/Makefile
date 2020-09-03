# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2014 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  https://www.artsoft.org/
# -----------------------------------------------------------------------------
# The native Emerald Mine game engine is based on:
# Emerald Mine for X11 (c) 2000,2001 David Tritscher
# -----------------------------------------------------------------------------
# src/game_em/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	cave.c		\
	convert.c	\
	graphics.c	\
	init.c		\
	game.c		\
	logic.c		\
	reademc.c

OBJS =	cave.o		\
	convert.o	\
	graphics.o	\
	init.o		\
	game.o		\
	logic.o		\
	reademc.o

GAME_EM = game_em.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(GAME_EM)

$(GAME_EM): $(OBJS)
	$(AR) cr $(GAME_EM) $(OBJS)
	$(RANLIB) $(GAME_EM)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(GAME_EM)


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
