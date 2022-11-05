# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  https://www.artsoft.org/
# -----------------------------------------------------------------------------
# src/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

ifndef PLATFORM				# unknown platform -- default to Unix
PLATFORM = unix
endif

PLATFORM_BASE = $(PLATFORM)

ifeq ($(PLATFORM),cross-win32)
PLATFORM_BASE = cross-win
endif
ifeq ($(PLATFORM),cross-win64)
PLATFORM_BASE = cross-win
endif

AR = ar
RANLIB = ranlib
ETAGS = etags
STRIP = strip
RM = rm -f

CONVERT = convert
WINDRES = windres

CONVERT_ICON_ARGS = -transparent black -background transparent

DEBUGGER = gdb -batch -ex "run" -ex "bt"

PROGBASE = rocksndiamonds
PROGNAME = ../$(PROGBASE)


# -----------------------------------------------------------------------------
# configuring platform
# -----------------------------------------------------------------------------

ifeq ($(PLATFORM),unix)			# compiling on Unix/Linux (generic)
PROFILING_FLAGS = -pg
endif

ifeq ($(PLATFORM_BASE),cross-win)	# cross-compiling to Windows
PROGNAME = ../$(PROGBASE).exe
EXTRA_LDFLAGS = -lshfolder -lwsock32
endif

ifeq ($(PLATFORM),emscripten)		# compiling with Emscripten
PROGNAME = ../$(PROGBASE).js
DATA_FILE = $(PROGBASE).data
CC = emcc
AR = emar
RANLIB = emranlib
STRIP = true
FILE_PACKAGER = file_packager
endif

ifeq ($(shell uname -s),Darwin)		# compiling on Mac OS X
DEBUGGER = lldb --batch -o "run" -k "bt" -k "quit"
SANITIZING_FLAGS = -fsanitize=undefined
ifdef BUILD_DIST			# distribution build
MAC_TARGET_VERSION_MIN = 10.7
EXTRA_FLAGS_MAC = -mmacosx-version-min=$(MAC_TARGET_VERSION_MIN)
EXTRA_CFLAGS = $(EXTRA_FLAGS_MAC)
EXTRA_LDFLAGS = $(EXTRA_FLAGS_MAC)
MACOSX_DEPLOYMENT_TARGET = $MAC_TARGET_VERSION_MIN
endif
endif

ifeq ($(shell uname -s),OS/2)		# compiling on OS/2
PROGNAME = ../$(PROGBASE).exe
EXTRA_LDFLAGS = -Zomf -Zbin-files -Zmap -lcx -Zhigh-mem
endif


# -----------------------------------------------------------------------------
# configuring target
# -----------------------------------------------------------------------------

ifndef TARGET				# auto-detect compiling for SDL2
  SDL_VERSION := $(shell sdl2-config --version 2> /dev/null)
  ifdef SDL_VERSION
    TARGET = sdl2
  else
    $(error SDL2 library not found)
  endif
endif

# $(info Using SDL version $(SDL_VERSION) [TARGET == $(TARGET)])

ifeq ($(TARGET),sdl2)			# compiling for SDL2 target
ifeq ($(PLATFORM),emscripten)
SDL_LIBS = -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_NET=2 -s USE_MODPLUG=1 -s USE_MPG123=1 -s USE_ZLIB=1
SDL_FMTS = -s SDL2_IMAGE_FORMATS='["bmp","png","pcx","xpm"]' -s SDL2_MIXER_FORMATS='["mod","mp3"]'
EXTRA_CFLAGS = $(SDL_LIBS)
EXTRA_LDFLAGS = $(SDL_FMTS) -s INITIAL_MEMORY=81920000 -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM -s NO_EXIT_RUNTIME=0 -s ASYNCIFY -O2 -lidbfs.js
DATA_DIRS = conf docs levels graphics sounds music
FILE_PACKAGER_ARGS = --preload $(DATA_DIRS) --js-output=$(DATA_FILE).js
else
SDL_LIBS = -lSDL2_image -lSDL2_mixer -lSDL2_net
endif

SYS_CFLAGS := -DTARGET_SDL2 $(shell sdl2-config --cflags)
SYS_LDFLAGS := $(SDL_LIBS) $(shell sdl2-config --libs) -lm -lz
endif


# -----------------------------------------------------------------------------
# configuring compile-time definitions
# -----------------------------------------------------------------------------

ifdef BASE_PATH					# path to read-only game data
CONFIG_BASE_PATH = -DBASE_PATH="\"$(BASE_PATH)\""
endif

CONFIG = $(CONFIG_BASE_PATH) $(JOYSTICK)

DEBUG = -DDEBUG -g

# ANALYZE = $(PROFILING_FLAGS)
# ANALYZE = $(SANITIZING_FLAGS)

# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -O2 -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
OPTIONS = $(DEBUG) -Wall -Wstrict-prototypes -Wmissing-prototypes
# OPTIONS = $(DEBUG) -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes
# OPTIONS = $(DEBUG) -Wall -ansi -pedantic	# only for debugging purposes
# OPTIONS = -O2 -Wall -ansi -pedantic
# OPTIONS = -O2 -Wall
# OPTIONS = -O2

ifdef BUILD_TEST			# test build
OPTIONS := $(OPTIONS) -DTESTING
endif

ifdef BUILD_DIST			# distribution build
SYS_LDFLAGS := $(shell echo $(SYS_LDFLAGS) |	\
		       sed -e "s%-rpath,[^ ]*%-rpath,'\$$ORIGIN/lib'%")
OPTIONS = -O2 -Wall
endif

CFLAGS = $(OPTIONS) $(ANALYZE) $(SYS_CFLAGS)  $(EXTRA_CFLAGS) $(CONFIG)
LDFLAGS =           $(ANALYZE) $(SYS_LDFLAGS) $(EXTRA_LDFLAGS)


SRCS =	main.c		\
	conf_gfx.c	\
	conf_snd.c	\
	conf_mus.c	\
	conf_hlp.c	\
	init.c		\
	config.c	\
	events.c	\
	tools.c		\
	screens.c	\
	game.c		\
	editor.c	\
	files.c		\
	tape.c		\
	anim.c		\
	api.c		\
	network.c	\
	netserv.c

OBJS =	main.o		\
	conf_gfx.o	\
	conf_snd.o	\
	conf_mus.o	\
	conf_hlp.o	\
	init.o		\
	config.o	\
	events.o	\
	tools.o		\
	screens.o	\
	game.o		\
	editor.o	\
	files.o		\
	tape.o		\
	anim.o		\
	api.o		\
	network.o	\
	netserv.o

CNFS =	conf_gfx.h	\
	conf_snd.h	\
	conf_mus.h	\
	conf_chr.c	\
	conf_chr.h	\
	conf_cus.c	\
	conf_cus.h	\
	conf_grp.c	\
	conf_grp.h	\
	conf_emp.c	\
	conf_emp.h	\
	conf_e2g.c	\
	conf_esg.c	\
	conf_e2s.c	\
	conf_fnt.c	\
	conf_g2s.c	\
	conf_g2m.c	\
	conf_var.c	\
	conf_act.c

CNFS_CMD = ../build-scripts/create_element_defs.pl

TIMESTAMP_FILE = conftime.h
TIMESTAMP_FORMAT = %Y-%m-%d %H:%M

# use SOURCE_DATE_EPOCH, or else last Git commit date, or else current date
SOURCE_DATE_EPOCH ?= $(shell test -d ../.git && test `git ls-files -m | wc -l` -eq 0 && git show -s --format=%ct || date +%s)

# get source date string from either GNU / Linux or BSD / Mac OS X style "date"
SOURCE_DATE_STRING := $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" "+$(TIMESTAMP_FORMAT)"  2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" "+$(TIMESTAMP_FORMAT)" 2>/dev/null || date -u "+$(TIMESTAMP_FORMAT)")

COMMIT_HASH_FILE = confhash.h
COMMIT_HASH_NONE = "[changed files]"

# get last Git commit hash, if no files were changed
SOURCE_HASH_STRING ?= $(shell test -d ../.git && test `git ls-files -m | wc -l` -eq 0 && git show -s --format=%h || echo "$(COMMIT_HASH_NONE)")

LIBGAME_DIR = libgame
LIBGAME = $(LIBGAME_DIR)/libgame.a

GAME_EM_DIR = game_em
GAME_EM = $(GAME_EM_DIR)/game_em.a

GAME_SP_DIR = game_sp
GAME_SP = $(GAME_SP_DIR)/game_sp.a

GAME_MM_DIR = game_mm
GAME_MM = $(GAME_MM_DIR)/game_mm.a

RNDLIBS = $(LIBGAME) $(GAME_EM) $(GAME_SP) $(GAME_MM)
AUTOCONF = conf_gfx.h conf_snd.h conf_mus.h

ICONBASE = windows_icon
ICON_BASEPATH = ../build-projects/windows/icons

ifeq ($(PLATFORM_BASE),cross-win)
ICON_PATH = $(ICON_BASEPATH)
ICON = $(ICONBASE).o
endif

GRAPHICS_DIR = ../graphics


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(AUTOCONF) libgame_dir game_em_dir game_sp_dir game_mm_dir $(PROGNAME) graphics_dir

$(PROGNAME): $(RNDLIBS) $(TIMESTAMP_FILE) $(COMMIT_HASH_FILE) $(OBJS) $(ICON)
	$(CC) $(OBJS) $(ICON) $(RNDLIBS) $(LDFLAGS) -o $(PROGNAME)
ifdef BUILD_DIST
	$(STRIP) $(PROGNAME)
endif
ifeq ($(PLATFORM),emscripten)
	(cd .. ; $(FILE_PACKAGER) $(DATA_FILE) $(FILE_PACKAGER_ARGS))
endif

libgame_dir:
	@$(MAKE) -C $(LIBGAME_DIR)
$(LIBGAME):
	@$(MAKE) -C $(LIBGAME_DIR)

game_em_dir:
	@$(MAKE) -C $(GAME_EM_DIR)
$(GAME_EM):
	@$(MAKE) -C $(GAME_EM_DIR)

game_sp_dir:
	@$(MAKE) -C $(GAME_SP_DIR)
$(GAME_SP):
	@$(MAKE) -C $(GAME_SP_DIR)

game_mm_dir:
	@$(MAKE) -C $(GAME_MM_DIR)
$(GAME_MM):
	@$(MAKE) -C $(GAME_MM_DIR)

auto-conf:
	@for i in $(CNFS); do			\
		echo "$(CNFS_CMD) $$i > $$i";	\
		$(CNFS_CMD) $$i > $$i;		\
	done

auto-conf-clean:
	@for i in $(CNFS); do			\
		echo "$(RM) $$i";		\
		$(RM) $$i;			\
	done

conf_gfx.h: conf_gfx.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf_snd.h: conf_snd.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf_mus.h: conf_mus.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf-time:
	@echo '#define SOURCE_DATE_STRING "$(SOURCE_DATE_STRING)"' \
	> $(TIMESTAMP_FILE)

conf-hash:
	@echo '#define SOURCE_HASH_STRING "$(SOURCE_HASH_STRING)"' \
	> $(COMMIT_HASH_FILE)

config.o: config.c $(TIMESTAMP_FILE)

$(TIMESTAMP_FILE): $(SRCS) $(RNDLIBS)
	@$(MAKE) conf-time

$(COMMIT_HASH_FILE): $(SRCS) $(RNDLIBS)
	@$(MAKE) conf-hash

$(ICON):
	$(CONVERT) $(ICON_PATH)/*.png $(CONVERT_ICON_ARGS) $(ICONBASE).ico
	echo "$(ICONBASE) ICON $(ICONBASE).ico" | $(WINDRES) -o $(ICON)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

graphics_dir:
	@test -f $(GRAPHICS_DIR)/Makefile && $(MAKE) -C $(GRAPHICS_DIR) || true

clean-obj:
	$(MAKE) -C $(LIBGAME_DIR) clean
	$(MAKE) -C $(GAME_EM_DIR) clean
	$(MAKE) -C $(GAME_SP_DIR) clean
	$(MAKE) -C $(GAME_MM_DIR) clean
	$(RM) $(OBJS)
	$(RM) $(RNDLIBS)

clean-ico:
	$(RM) $(ICONBASE).ico
	$(RM) $(ICONBASE).o

clean-bin:
	$(RM) $(PROGNAME)
	$(RM) $(PROGNAME).exe
	$(RM) $(PROGNAME).js
	$(RM) $(PROGNAME).wasm
	$(RM) $(PROGNAME).data

clean: clean-obj clean-ico clean-bin

clean-git: clean auto-conf-clean
	@$(MAKE) -C $(GRAPHICS_DIR) clean

dist-clean: clean-obj clean-ico


# -----------------------------------------------------------------------------
# run and test targets
# -----------------------------------------------------------------------------

run:
	cd .. && ./$(PROGBASE) --verbose

gdb:
	cd .. && $(DEBUGGER) $(PROGBASE)

valgrind:
	cd .. && valgrind -v --leak-check=yes ./$(PROGBASE) 2> valgrind.out


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

tags:
	$(ETAGS) *.[ch] $(LIBGAME_DIR)/*.[ch] $(GAME_EM_DIR)/*.[ch] $(GAME_SP_DIR)/*.[ch] $(GAME_MM_DIR)/*.[ch]

depend:
	$(MAKE) -C $(LIBGAME_DIR) depend
	$(MAKE) -C $(GAME_EM_DIR) depend
	$(MAKE) -C $(GAME_SP_DIR) depend
	$(MAKE) -C $(GAME_MM_DIR) depend
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
