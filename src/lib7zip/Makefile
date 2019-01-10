# =============================================================================
# Rocks'n'Diamonds Makefile (lib7zip)
# -----------------------------------------------------------------------------
# (c) 1995-2005 Holger Schemel <info@artsoft.org>
# =============================================================================
##
# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	7zAlloc.c	\
	7zBuffer.c	\
	7zCrc.c		\
	7zDecode.c	\
	7zExtract.c	\
	7zHeader.c	\
	7zIn.c		\
	7zItem.c	\
	7zMethodID.c	\
	LzmaDecode.c	\
	7zAlloc.h	\
	7zBuffer.h	\
	7zCrc.h		\
	7zDecode.h	\
	7zExtract.h	\
	7zHeader.h	\
	7zIn.h		\
	7zItem.h	\
	7zMethodID.h	\
	7zTypes.h	\
	LzmaDecode.h
	

OBJS =	7zAlloc.o	\
	7zBuffer.o	\
	7zCrc.o		\
	7zDecode.o	\
	7zExtract.o	\
	7zHeader.o	\
	7zIn.o		\
	7zItem.o	\
	7zMethodID.o	\
	LzmaDecode.o

LIB7ZIP = lib7zip.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(LIB7ZIP)

$(LIB7ZIP): $(OBJS)
	$(AR) cru $(LIB7ZIP) $(OBJS)
	$(RANLIB) $(LIB7ZIP)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(LIB7ZIP)


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
