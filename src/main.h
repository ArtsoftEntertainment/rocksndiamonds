/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* main.h                                                   *
***********************************************************/

#ifndef MAIN_H
#define MAIN_H

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "libgame/libgame.h"
#include "game_em/game_em.h"

#include "conf_gfx.h"	/* include auto-generated data structure definitions */
#include "conf_snd.h"	/* include auto-generated data structure definitions */
#include "conf_mus.h"	/* include auto-generated data structure definitions */

#define IMG_UNDEFINED			(-1)
#define IMG_EMPTY			IMG_EMPTY_SPACE
#define IMG_SP_EMPTY			IMG_SP_EMPTY_SPACE
#define IMG_EXPLOSION			IMG_DEFAULT_EXPLODING
#define IMG_CHAR_START			IMG_CHAR_SPACE
#define IMG_CUSTOM_START		IMG_CUSTOM_1

#define SND_UNDEFINED			(-1)
#define MUS_UNDEFINED			(-1)

#define WIN_XSIZE			672
#define WIN_YSIZE			560

#define SCR_FIELDX			17
#define SCR_FIELDY			17
#define MAX_BUF_XSIZE			(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE			(SCR_FIELDY + 2)
#define MIN_LEV_FIELDX			3
#define MIN_LEV_FIELDY			3
#define STD_LEV_FIELDX			64
#define STD_LEV_FIELDY			32
#define MAX_LEV_FIELDX			MAX_PLAYFIELD_WIDTH
#define MAX_LEV_FIELDY			MAX_PLAYFIELD_HEIGHT

#define SCREENX(a)			((a) - scroll_x)
#define SCREENY(a)			((a) - scroll_y)
#define LEVELX(a)			((a) + scroll_x)
#define LEVELY(a)			((a) + scroll_y)
#define IN_VIS_FIELD(x,y) ((x)>=0 && (x)<SCR_FIELDX && (y)>=0 &&(y)<SCR_FIELDY)
#define IN_SCR_FIELD(x,y) ((x)>=BX1 && (x)<=BX2 && (y)>=BY1 &&(y)<=BY2)
#define IN_LEV_FIELD(x,y) ((x)>=0 && (x)<lev_fieldx && (y)>=0 &&(y)<lev_fieldy)

/* values for configurable properties (custom elem's only, else pre-defined) */
#define EP_DIGGABLE			0
#define EP_COLLECTIBLE_ONLY		1
#define EP_DONT_RUN_INTO		2
#define EP_DONT_COLLIDE_WITH		3
#define EP_DONT_TOUCH			4
#define EP_INDESTRUCTIBLE		5
#define EP_SLIPPERY			6
#define EP_CAN_CHANGE			7
#define EP_CAN_MOVE			8
#define EP_CAN_FALL			9
#define EP_CAN_SMASH_PLAYER		10
#define EP_CAN_SMASH_ENEMIES		11
#define EP_CAN_SMASH_EVERYTHING		12
#define EP_EXPLODES_BY_FIRE		13
#define EP_EXPLODES_SMASHED		14
#define EP_EXPLODES_IMPACT		15
#define EP_WALKABLE_OVER		16
#define EP_WALKABLE_INSIDE		17
#define EP_WALKABLE_UNDER		18
#define EP_PASSABLE_OVER		19
#define EP_PASSABLE_INSIDE		20
#define EP_PASSABLE_UNDER		21
#define EP_DROPPABLE			22
#define EP_EXPLODES_1X1_OLD		23
#define EP_PUSHABLE			24
#define EP_EXPLODES_CROSS_OLD		25
#define EP_PROTECTED			26
#define EP_CAN_MOVE_INTO_ACID		27
#define EP_THROWABLE			28
#define EP_CAN_EXPLODE			29
#define EP_GRAVITY_REACHABLE		30

/* values for pre-defined properties */
#define EP_PLAYER			32
#define EP_CAN_PASS_MAGIC_WALL		33
#define EP_SWITCHABLE			34
#define EP_BD_ELEMENT			35
#define EP_SP_ELEMENT			36
#define EP_SB_ELEMENT			37
#define EP_GEM				38
#define EP_FOOD_DARK_YAMYAM		39
#define EP_FOOD_PENGUIN			40
#define EP_FOOD_PIG			41
#define EP_HISTORIC_WALL		42
#define EP_HISTORIC_SOLID		43
#define EP_CLASSIC_ENEMY		44
#define EP_BELT				45
#define EP_BELT_ACTIVE			46
#define EP_BELT_SWITCH			47
#define EP_TUBE				48
#define EP_KEYGATE			49
#define EP_AMOEBOID			50
#define EP_AMOEBALIVE			51
#define EP_HAS_EDITOR_CONTENT		52
#define EP_CAN_TURN_EACH_MOVE		53
#define EP_CAN_GROW			54
#define EP_ACTIVE_BOMB			55
#define EP_INACTIVE			56

/* values for special configurable properties (depending on level settings) */
#define EP_EM_SLIPPERY_WALL		57

/* values for special graphics properties (no effect on game engine) */
#define EP_GFX_CRUMBLED			58

/* values for derived properties (determined from properties above) */
#define EP_ACCESSIBLE_OVER		59
#define EP_ACCESSIBLE_INSIDE		60
#define EP_ACCESSIBLE_UNDER		61
#define EP_WALKABLE			62
#define EP_PASSABLE			63
#define EP_ACCESSIBLE			64
#define EP_COLLECTIBLE			65
#define EP_SNAPPABLE			66
#define EP_WALL				67
#define EP_SOLID_FOR_PUSHING		68
#define EP_DRAGONFIRE_PROOF		69
#define EP_EXPLOSION_PROOF		70
#define EP_CAN_SMASH			71
#define EP_EXPLODES_3X3_OLD		72
#define EP_CAN_EXPLODE_BY_FIRE		73
#define EP_CAN_EXPLODE_SMASHED		74
#define EP_CAN_EXPLODE_IMPACT		75
#define EP_SP_PORT			76
#define EP_CAN_EXPLODE_BY_DRAGONFIRE	77
#define EP_CAN_EXPLODE_BY_EXPLOSION	78
#define EP_COULD_MOVE_INTO_ACID		79
#define EP_MAYBE_DONT_COLLIDE_WITH	80
#define EP_CAN_BE_CLONED_BY_ANDROID	81

/* values for internal purpose only (level editor) */
#define EP_WALK_TO_OBJECT		82
#define EP_DEADLY			83
#define EP_EDITOR_CASCADE		84
#define EP_EDITOR_CASCADE_ACTIVE	85
#define EP_EDITOR_CASCADE_INACTIVE	86

/* values for internal purpose only (game engine) */
#define EP_HAS_ACTION			87
#define EP_CAN_CHANGE_OR_HAS_ACTION	88

/* values for internal purpose only (other) */
#define EP_OBSOLETE			89

#define NUM_ELEMENT_PROPERTIES		90

#define NUM_EP_BITFIELDS		((NUM_ELEMENT_PROPERTIES + 31) / 32)
#define EP_BITFIELD_BASE_NR		0

#define EP_BITMASK_BASE_DEFAULT		(1 << EP_CAN_MOVE_INTO_ACID)
#define EP_BITMASK_DEFAULT		0

#define PROPERTY_BIT(p)			(1 << ((p) % 32))
#if 1
#define PROPERTY_VAR(e,p)		(element_info[e].properties[(p) / 32])
#else
#define PROPERTY_VAR(e,p)		(Properties[e][(p) / 32])
#endif
#define HAS_PROPERTY(e,p)	((PROPERTY_VAR(e, p) & PROPERTY_BIT(p)) != 0)
#define SET_PROPERTY(e,p,v)	((v) ?					   \
				 (PROPERTY_VAR(e,p) |=  PROPERTY_BIT(p)) : \
				 (PROPERTY_VAR(e,p) &= ~PROPERTY_BIT(p)))


/* values for change events for custom elements (stored in level file) */
#define CE_DELAY			0
#define CE_TOUCHED_BY_PLAYER		1
#define CE_PRESSED_BY_PLAYER		2
#define CE_PUSHED_BY_PLAYER		3
#define CE_DROPPED_BY_PLAYER		4
#define CE_HITTING_SOMETHING		5
#define CE_IMPACT			6
#define CE_SMASHED			7
#define CE_TOUCHING_X			8
#define CE_CHANGE_OF_X			9
#define CE_EXPLOSION_OF_X		10
#define CE_PLAYER_TOUCHES_X		11
#define CE_PLAYER_PRESSES_X		12
#define CE_PLAYER_PUSHES_X		13
#define CE_PLAYER_COLLECTS_X		14
#define CE_PLAYER_DROPS_X		15
#define CE_VALUE_GETS_ZERO		16
#define CE_VALUE_GETS_ZERO_OF_X		17
#define CE_BY_OTHER_ACTION		18
#define CE_BY_DIRECT_ACTION		19
#define CE_PLAYER_DIGS_X		20
#define CE_ENTERED_BY_PLAYER		21
#define CE_LEFT_BY_PLAYER		22
#define CE_PLAYER_ENTERS_X		23
#define CE_PLAYER_LEAVES_X		24
#define CE_SWITCHED			25
#define CE_SWITCH_OF_X			26
#define CE_HIT_BY_SOMETHING		27
#define CE_HITTING_X			28
#define CE_HIT_BY_X			29
#define CE_BLOCKED			30
#define CE_SWITCHED_BY_PLAYER		31
#define CE_PLAYER_SWITCHES_X		32
#define CE_SNAPPED_BY_PLAYER		33
#define CE_PLAYER_SNAPS_X		34
#define CE_MOVE_OF_X			35
#define CE_DIGGING_X			36
#define CE_CREATION_OF_X		37
#define CE_SCORE_GETS_ZERO		38
#define CE_SCORE_GETS_ZERO_OF_X		39
#define CE_VALUE_CHANGES		40
#define CE_VALUE_CHANGES_OF_X		41
#define CE_SCORE_CHANGES		42
#define CE_SCORE_CHANGES_OF_X		43

#define NUM_CHANGE_EVENTS		44

#define NUM_CE_BITFIELDS		((NUM_CHANGE_EVENTS + 31) / 32)

#define CE_BITMASK_DEFAULT		0

#define CH_EVENT_BITFIELD_NR(e)		(e / 32)
#define CH_EVENT_BIT(e)			(1 << ((e) % 32))

#define CH_EVENT_VAR(e,c)		(element_info[e].change->has_event[c])
#define CH_ANY_EVENT_VAR(e,c)		(element_info[e].has_change_event[c])

#define PAGE_HAS_CHANGE_EVENT(p,c)	((p)->has_event[c])
#define HAS_CHANGE_EVENT(e,c)		(IS_CUSTOM_ELEMENT(e) &&	\
					 CH_EVENT_VAR(e,c))
#define HAS_ANY_CHANGE_EVENT(e,c)	(IS_CUSTOM_ELEMENT(e) &&	\
					 CH_ANY_EVENT_VAR(e,c))

#define SET_CHANGE_EVENT(e,c,v)		(IS_CUSTOM_ELEMENT(e) ?		\
					 CH_EVENT_VAR(e,c) = (v) : 0)
#define SET_ANY_CHANGE_EVENT(e,c,v)	(IS_CUSTOM_ELEMENT(e) ?		\
					 CH_ANY_EVENT_VAR(e,c) = (v) : 0)

/* values for player bitmasks */
#define PLAYER_BITS_NONE		0
#define PLAYER_BITS_1			(1 << 0)
#define PLAYER_BITS_2			(1 << 1)
#define PLAYER_BITS_3			(1 << 2)
#define PLAYER_BITS_4			(1 << 3)
#define PLAYER_BITS_ANY			(PLAYER_BITS_1 | \
					 PLAYER_BITS_2 | \
					 PLAYER_BITS_3 | \
					 PLAYER_BITS_4)
#define PLAYER_BITS_TRIGGER		(1 << 4)

/* values for move directions (bits 0 - 3: basic move directions) */
#define MV_BIT_PREVIOUS			4
#define MV_BIT_TRIGGER			5
#define MV_BIT_TRIGGER_BACK		6
#define MV_BIT_NORMAL			MV_BIT_TRIGGER
#define MV_BIT_REVERSE			MV_BIT_TRIGGER_BACK

#define MV_PREVIOUS			(1 << MV_BIT_PREVIOUS)
#define MV_TRIGGER			(1 << MV_BIT_TRIGGER)
#define MV_TRIGGER_BACK			(1 << MV_BIT_TRIGGER_BACK)
#define MV_NORMAL			(1 << MV_BIT_NORMAL)
#define MV_REVERSE			(1 << MV_BIT_REVERSE)

/* values for move stepsize */
#define STEPSIZE_NOT_MOVING		0
#define STEPSIZE_VERY_SLOW		1
#define STEPSIZE_SLOW			2
#define STEPSIZE_NORMAL			4
#define STEPSIZE_FAST			8
#define STEPSIZE_VERY_FAST		16
#define STEPSIZE_EVEN_FASTER		32
#define STEPSIZE_SLOWER			50	/* (symbolic value only) */
#define STEPSIZE_FASTER			200	/* (symbolic value only) */
#define STEPSIZE_RESET			100	/* (symbolic value only) */

/* values for change side for custom elements */
#define CH_SIDE_NONE			MV_NONE
#define CH_SIDE_LEFT			MV_LEFT
#define CH_SIDE_RIGHT			MV_RIGHT
#define CH_SIDE_TOP			MV_UP
#define CH_SIDE_BOTTOM			MV_DOWN
#define CH_SIDE_LEFT_RIGHT		MV_HORIZONTAL
#define CH_SIDE_TOP_BOTTOM		MV_VERTICAL
#define CH_SIDE_ANY			MV_ANY_DIRECTION

/* values for change player for custom elements */
#define CH_PLAYER_NONE			PLAYER_BITS_NONE
#define CH_PLAYER_1			PLAYER_BITS_1
#define CH_PLAYER_2			PLAYER_BITS_2
#define CH_PLAYER_3			PLAYER_BITS_3
#define CH_PLAYER_4			PLAYER_BITS_4
#define CH_PLAYER_ANY			PLAYER_BITS_ANY

/* values for change page for custom elements */
#define CH_PAGE_ANY_FILE		(0xff)
#define CH_PAGE_ANY			(0xffffffff)

/* values for change power for custom elements */
#define CP_WHEN_EMPTY			0
#define CP_WHEN_DIGGABLE		1
#define CP_WHEN_DESTRUCTIBLE		2
#define CP_WHEN_COLLECTIBLE		3
#define CP_WHEN_REMOVABLE		4
#define CP_WHEN_WALKABLE		5

/* values for change actions for custom elements */
#define CA_NO_ACTION			0
#define CA_EXIT_PLAYER			1
#define CA_KILL_PLAYER			2
#define CA_MOVE_PLAYER			3
#define CA_RESTART_LEVEL		4
#define CA_SHOW_ENVELOPE		5
#define CA_SET_LEVEL_TIME		6
#define CA_SET_LEVEL_GEMS		7
#define CA_SET_LEVEL_SCORE		8
#define CA_SET_LEVEL_WIND		9
#define CA_SET_PLAYER_GRAVITY		10
#define CA_SET_PLAYER_KEYS		11
#define CA_SET_PLAYER_SPEED		12
#define CA_SET_PLAYER_SHIELD		13
#define CA_SET_PLAYER_ARTWORK		14
#define CA_SET_CE_SCORE			15
#define CA_SET_CE_VALUE			16
#define CA_SET_ENGINE_SCAN_MODE		17

#define CA_HEADLINE_LEVEL_ACTIONS	250
#define CA_HEADLINE_PLAYER_ACTIONS	251
#define CA_HEADLINE_CE_ACTIONS		252
#define CA_HEADLINE_ENGINE_ACTIONS	253
#define CA_UNDEFINED			255

/* values for change action mode for custom elements */
#define CA_MODE_UNDEFINED		0
#define CA_MODE_SET			1
#define CA_MODE_ADD			2
#define CA_MODE_SUBTRACT		3
#define CA_MODE_MULTIPLY		4
#define CA_MODE_DIVIDE			5
#define CA_MODE_MODULO			6

/* values for change action parameters for custom elements */
#define CA_ARG_MIN			0
#define CA_ARG_0			0
#define CA_ARG_1			1
#define CA_ARG_2			2
#define CA_ARG_3			3
#define CA_ARG_4			4
#define CA_ARG_5			5
#define CA_ARG_6			6
#define CA_ARG_7			7
#define CA_ARG_8			8
#define CA_ARG_9			9
#define CA_ARG_10			10
#define CA_ARG_100			100
#define CA_ARG_1000			1000
#define CA_ARG_MAX			9999
#define CA_ARG_PLAYER			10000
#define CA_ARG_PLAYER_1			(CA_ARG_PLAYER + PLAYER_BITS_1)
#define CA_ARG_PLAYER_2			(CA_ARG_PLAYER + PLAYER_BITS_2)
#define CA_ARG_PLAYER_3			(CA_ARG_PLAYER + PLAYER_BITS_3)
#define CA_ARG_PLAYER_4			(CA_ARG_PLAYER + PLAYER_BITS_4)
#define CA_ARG_PLAYER_ANY		(CA_ARG_PLAYER + PLAYER_BITS_ANY)
#define CA_ARG_PLAYER_TRIGGER		(CA_ARG_PLAYER + PLAYER_BITS_TRIGGER)
#define CA_ARG_PLAYER_HEADLINE		(CA_ARG_PLAYER + 999)
#define CA_ARG_NUMBER			11000
#define CA_ARG_NUMBER_MIN		(CA_ARG_NUMBER + 0)
#define CA_ARG_NUMBER_MAX		(CA_ARG_NUMBER + 1)
#define CA_ARG_NUMBER_RESET		(CA_ARG_NUMBER + 2)
#define CA_ARG_NUMBER_CE_SCORE		(CA_ARG_NUMBER + 3)
#define CA_ARG_NUMBER_CE_VALUE		(CA_ARG_NUMBER + 4)
#define CA_ARG_NUMBER_CE_DELAY		(CA_ARG_NUMBER + 5)
#define CA_ARG_NUMBER_LEVEL_TIME	(CA_ARG_NUMBER + 6)
#define CA_ARG_NUMBER_LEVEL_GEMS	(CA_ARG_NUMBER + 7)
#define CA_ARG_NUMBER_LEVEL_SCORE	(CA_ARG_NUMBER + 8)
#define CA_ARG_NUMBER_HEADLINE		(CA_ARG_NUMBER + 999)
#define CA_ARG_ELEMENT			12000
#define CA_ARG_ELEMENT_RESET		(CA_ARG_ELEMENT + 0)
#define CA_ARG_ELEMENT_TARGET		(CA_ARG_ELEMENT + 1)
#define CA_ARG_ELEMENT_TRIGGER		(CA_ARG_ELEMENT + 2)
#define CA_ARG_ELEMENT_HEADLINE		(CA_ARG_ELEMENT + 997)
#define CA_ARG_ELEMENT_CV_TARGET	(CA_ARG_ELEMENT_TARGET)
#define CA_ARG_ELEMENT_CV_TRIGGER	(CA_ARG_ELEMENT_TRIGGER)
#define CA_ARG_ELEMENT_CV_HEADLINE	(CA_ARG_ELEMENT_HEADLINE)
#define CA_ARG_ELEMENT_NR_TARGET	(CA_ARG_ELEMENT + 3)
#define CA_ARG_ELEMENT_NR_TRIGGER	(CA_ARG_ELEMENT + 4)
#define CA_ARG_ELEMENT_NR_HEADLINE	(CA_ARG_ELEMENT + 998)
#define CA_ARG_ELEMENT_CS_TARGET	(CA_ARG_ELEMENT + 5)
#define CA_ARG_ELEMENT_CS_TRIGGER	(CA_ARG_ELEMENT + 6)
#define CA_ARG_ELEMENT_CS_HEADLINE	(CA_ARG_ELEMENT + 999)
#define CA_ARG_SPEED			13000
#define CA_ARG_SPEED_NOT_MOVING		(CA_ARG_SPEED + STEPSIZE_NOT_MOVING)
#define CA_ARG_SPEED_VERY_SLOW		(CA_ARG_SPEED + STEPSIZE_VERY_SLOW)
#define CA_ARG_SPEED_SLOW		(CA_ARG_SPEED + STEPSIZE_SLOW)
#define CA_ARG_SPEED_NORMAL		(CA_ARG_SPEED + STEPSIZE_NORMAL)
#define CA_ARG_SPEED_FAST		(CA_ARG_SPEED + STEPSIZE_FAST)
#define CA_ARG_SPEED_VERY_FAST		(CA_ARG_SPEED + STEPSIZE_VERY_FAST)
#define CA_ARG_SPEED_EVEN_FASTER	(CA_ARG_SPEED + STEPSIZE_EVEN_FASTER)
#define CA_ARG_SPEED_SLOWER		(CA_ARG_SPEED + STEPSIZE_SLOWER)
#define CA_ARG_SPEED_FASTER		(CA_ARG_SPEED + STEPSIZE_FASTER)
#define CA_ARG_SPEED_RESET		(CA_ARG_SPEED + STEPSIZE_RESET)
#define CA_ARG_SPEED_HEADLINE		(CA_ARG_SPEED + 999)
#define CA_ARG_GRAVITY			14000
#define CA_ARG_GRAVITY_OFF		(CA_ARG_GRAVITY + 0)
#define CA_ARG_GRAVITY_ON		(CA_ARG_GRAVITY + 1)
#define CA_ARG_GRAVITY_TOGGLE		(CA_ARG_GRAVITY + 2)
#define CA_ARG_GRAVITY_HEADLINE		(CA_ARG_GRAVITY + 999)
#define CA_ARG_DIRECTION		15000
#define CA_ARG_DIRECTION_NONE		(CA_ARG_DIRECTION + MV_NONE)
#define CA_ARG_DIRECTION_LEFT		(CA_ARG_DIRECTION + MV_LEFT)
#define CA_ARG_DIRECTION_RIGHT		(CA_ARG_DIRECTION + MV_RIGHT)
#define CA_ARG_DIRECTION_UP		(CA_ARG_DIRECTION + MV_UP)
#define CA_ARG_DIRECTION_DOWN		(CA_ARG_DIRECTION + MV_DOWN)
#define CA_ARG_DIRECTION_TRIGGER	(CA_ARG_DIRECTION + MV_TRIGGER)
#define CA_ARG_DIRECTION_TRIGGER_BACK	(CA_ARG_DIRECTION + MV_TRIGGER_BACK)
#define CA_ARG_DIRECTION_HEADLINE	(CA_ARG_DIRECTION + 999)
#define CA_ARG_SHIELD			16000
#define CA_ARG_SHIELD_OFF		(CA_ARG_SHIELD + 0)
#define CA_ARG_SHIELD_NORMAL		(CA_ARG_SHIELD + 1)
#define CA_ARG_SHIELD_DEADLY		(CA_ARG_SHIELD + 2)
#define CA_ARG_SHIELD_HEADLINE		(CA_ARG_SHIELD + 999)
#define CA_ARG_SCAN_MODE		17000
#define CA_ARG_SCAN_MODE_NORMAL		(CA_ARG_SCAN_MODE + MV_NORMAL)
#define CA_ARG_SCAN_MODE_REVERSE	(CA_ARG_SCAN_MODE + MV_REVERSE)
#define CA_ARG_SCAN_MODE_HEADLINE	(CA_ARG_SCAN_MODE + 999)
#define CA_ARG_UNDEFINED		65535

/* values for custom move patterns (bits 0 - 3: basic move directions) */
#define MV_BIT_TOWARDS_PLAYER		4
#define MV_BIT_AWAY_FROM_PLAYER		5
#define MV_BIT_ALONG_LEFT_SIDE		6
#define MV_BIT_ALONG_RIGHT_SIDE		7
#define MV_BIT_TURNING_LEFT		8
#define MV_BIT_TURNING_RIGHT		9
#define MV_BIT_WHEN_PUSHED		10
#define MV_BIT_MAZE_RUNNER		11
#define MV_BIT_MAZE_HUNTER		12
#define MV_BIT_WHEN_DROPPED		13
#define MV_BIT_TURNING_LEFT_RIGHT	14
#define MV_BIT_TURNING_RIGHT_LEFT	15
#define MV_BIT_TURNING_RANDOM		16
#define MV_BIT_WIND_DIRECTION		17

/* values for custom move patterns */
#define MV_TOWARDS_PLAYER		(1 << MV_BIT_TOWARDS_PLAYER)
#define MV_AWAY_FROM_PLAYER		(1 << MV_BIT_AWAY_FROM_PLAYER)
#define MV_ALONG_LEFT_SIDE		(1 << MV_BIT_ALONG_LEFT_SIDE)
#define MV_ALONG_RIGHT_SIDE		(1 << MV_BIT_ALONG_RIGHT_SIDE)
#define MV_TURNING_LEFT			(1 << MV_BIT_TURNING_LEFT)
#define MV_TURNING_RIGHT		(1 << MV_BIT_TURNING_RIGHT)
#define MV_WHEN_PUSHED			(1 << MV_BIT_WHEN_PUSHED)
#define MV_MAZE_RUNNER			(1 << MV_BIT_MAZE_RUNNER)
#define MV_MAZE_HUNTER			(1 << MV_BIT_MAZE_HUNTER)
#define MV_MAZE_RUNNER_STYLE		(MV_MAZE_RUNNER | MV_MAZE_HUNTER)
#define MV_WHEN_DROPPED			(1 << MV_BIT_WHEN_DROPPED)
#define MV_TURNING_LEFT_RIGHT		(1 << MV_BIT_TURNING_LEFT_RIGHT)
#define MV_TURNING_RIGHT_LEFT		(1 << MV_BIT_TURNING_RIGHT_LEFT)
#define MV_TURNING_RANDOM		(1 << MV_BIT_TURNING_RANDOM)
#define MV_WIND_DIRECTION		(1 << MV_BIT_WIND_DIRECTION)

/* values for initial move direction */
#define MV_START_NONE			(MV_NONE)
#define MV_START_AUTOMATIC		(MV_NONE)
#define MV_START_LEFT			(MV_LEFT)
#define MV_START_RIGHT			(MV_RIGHT)
#define MV_START_UP			(MV_UP)
#define MV_START_DOWN			(MV_DOWN)
#define MV_START_RANDOM			(MV_ALL_DIRECTIONS)
#define MV_START_PREVIOUS		(MV_PREVIOUS)

/* values for elements left behind by custom elements */
#define LEAVE_TYPE_UNLIMITED		0
#define LEAVE_TYPE_LIMITED		1

/* values for slippery property for custom elements */
#define SLIPPERY_ANY_RANDOM		0
#define SLIPPERY_ANY_LEFT_RIGHT		1
#define SLIPPERY_ANY_RIGHT_LEFT		2
#define SLIPPERY_ONLY_LEFT		3
#define SLIPPERY_ONLY_RIGHT		4

/* values for explosion type for custom elements */
#define EXPLODES_3X3			0
#define EXPLODES_1X1			1
#define EXPLODES_CROSS			2

/* macros for configurable properties */
#define IS_DIGGABLE(e)		HAS_PROPERTY(e, EP_DIGGABLE)
#define IS_COLLECTIBLE_ONLY(e)	HAS_PROPERTY(e, EP_COLLECTIBLE_ONLY)
#define DONT_RUN_INTO(e)	HAS_PROPERTY(e, EP_DONT_RUN_INTO)
#define DONT_COLLIDE_WITH(e)	HAS_PROPERTY(e, EP_DONT_COLLIDE_WITH)
#define DONT_TOUCH(e)		HAS_PROPERTY(e, EP_DONT_TOUCH)
#define IS_INDESTRUCTIBLE(e)	HAS_PROPERTY(e, EP_INDESTRUCTIBLE)
#define IS_SLIPPERY(e)		HAS_PROPERTY(e, EP_SLIPPERY)
#define CAN_CHANGE(e)		HAS_PROPERTY(e, EP_CAN_CHANGE)
#define CAN_MOVE(e)		HAS_PROPERTY(e, EP_CAN_MOVE)
#define CAN_FALL(e)		HAS_PROPERTY(e, EP_CAN_FALL)
#define CAN_SMASH_PLAYER(e)	HAS_PROPERTY(e, EP_CAN_SMASH_PLAYER)
#define CAN_SMASH_ENEMIES(e)	HAS_PROPERTY(e, EP_CAN_SMASH_ENEMIES)
#define CAN_SMASH_EVERYTHING(e)	HAS_PROPERTY(e, EP_CAN_SMASH_EVERYTHING)
#define EXPLODES_BY_FIRE(e)	HAS_PROPERTY(e, EP_EXPLODES_BY_FIRE)
#define EXPLODES_SMASHED(e)	HAS_PROPERTY(e, EP_EXPLODES_SMASHED)
#define EXPLODES_IMPACT(e)	HAS_PROPERTY(e, EP_EXPLODES_IMPACT)
#define IS_WALKABLE_OVER(e)	HAS_PROPERTY(e, EP_WALKABLE_OVER)
#define IS_WALKABLE_INSIDE(e)	HAS_PROPERTY(e, EP_WALKABLE_INSIDE)
#define IS_WALKABLE_UNDER(e)	HAS_PROPERTY(e, EP_WALKABLE_UNDER)
#define IS_PASSABLE_OVER(e)	HAS_PROPERTY(e, EP_PASSABLE_OVER)
#define IS_PASSABLE_INSIDE(e)	HAS_PROPERTY(e, EP_PASSABLE_INSIDE)
#define IS_PASSABLE_UNDER(e)	HAS_PROPERTY(e, EP_PASSABLE_UNDER)
#define IS_DROPPABLE(e)		HAS_PROPERTY(e, EP_DROPPABLE)
#define EXPLODES_1X1_OLD(e)	HAS_PROPERTY(e, EP_EXPLODES_1X1_OLD)
#define IS_PUSHABLE(e)		HAS_PROPERTY(e, EP_PUSHABLE)
#define EXPLODES_CROSS_OLD(e)	HAS_PROPERTY(e, EP_EXPLODES_CROSS_OLD)
#define IS_PROTECTED(e)		HAS_PROPERTY(e, EP_PROTECTED)
#define CAN_MOVE_INTO_ACID(e)	HAS_PROPERTY(e, EP_CAN_MOVE_INTO_ACID)
#define IS_THROWABLE(e)		HAS_PROPERTY(e, EP_THROWABLE)
#define CAN_EXPLODE(e)		HAS_PROPERTY(e, EP_CAN_EXPLODE)
#define IS_GRAVITY_REACHABLE(e)	HAS_PROPERTY(e, EP_GRAVITY_REACHABLE)

/* macros for special configurable properties */
#define IS_EM_SLIPPERY_WALL(e)	HAS_PROPERTY(e, EP_EM_SLIPPERY_WALL)

/* macros for special graphics properties */
#define GFX_CRUMBLED(e)		HAS_PROPERTY(GFX_ELEMENT(e), EP_GFX_CRUMBLED)

/* macros for pre-defined properties */
#define ELEM_IS_PLAYER(e)	HAS_PROPERTY(e, EP_PLAYER)
#define CAN_PASS_MAGIC_WALL(e)	HAS_PROPERTY(e, EP_CAN_PASS_MAGIC_WALL)
#define IS_SWITCHABLE(e)	HAS_PROPERTY(e, EP_SWITCHABLE)
#define IS_BD_ELEMENT(e)	HAS_PROPERTY(e, EP_BD_ELEMENT)
#define IS_SP_ELEMENT(e)	HAS_PROPERTY(e, EP_SP_ELEMENT)
#define IS_SB_ELEMENT(e)	HAS_PROPERTY(e, EP_SB_ELEMENT)
#define IS_GEM(e)		HAS_PROPERTY(e, EP_GEM)
#define IS_FOOD_DARK_YAMYAM(e)	HAS_PROPERTY(e, EP_FOOD_DARK_YAMYAM)
#define IS_FOOD_PENGUIN(e)	HAS_PROPERTY(e, EP_FOOD_PENGUIN)
#define IS_FOOD_PIG(e)		HAS_PROPERTY(e, EP_FOOD_PIG)
#define IS_HISTORIC_WALL(e)	HAS_PROPERTY(e, EP_HISTORIC_WALL)
#define IS_HISTORIC_SOLID(e)	HAS_PROPERTY(e, EP_HISTORIC_SOLID)
#define IS_CLASSIC_ENEMY(e)	HAS_PROPERTY(e, EP_CLASSIC_ENEMY)
#define IS_BELT(e)		HAS_PROPERTY(e, EP_BELT)
#define IS_BELT_ACTIVE(e)	HAS_PROPERTY(e, EP_BELT_ACTIVE)
#define IS_BELT_SWITCH(e)	HAS_PROPERTY(e, EP_BELT_SWITCH)
#define IS_TUBE(e)		HAS_PROPERTY(e, EP_TUBE)
#define IS_KEYGATE(e)		HAS_PROPERTY(e, EP_KEYGATE)
#define IS_AMOEBOID(e)		HAS_PROPERTY(e, EP_AMOEBOID)
#define IS_AMOEBALIVE(e)	HAS_PROPERTY(e, EP_AMOEBALIVE)
#define HAS_EDITOR_CONTENT(e)	HAS_PROPERTY(e, EP_HAS_EDITOR_CONTENT)
#define CAN_TURN_EACH_MOVE(e)	HAS_PROPERTY(e, EP_CAN_TURN_EACH_MOVE)
#define CAN_GROW(e)		HAS_PROPERTY(e, EP_CAN_GROW)
#define IS_ACTIVE_BOMB(e)	HAS_PROPERTY(e, EP_ACTIVE_BOMB)
#define IS_INACTIVE(e)		HAS_PROPERTY(e, EP_INACTIVE)

/* macros for derived properties */
#define IS_ACCESSIBLE_OVER(e)	HAS_PROPERTY(e, EP_ACCESSIBLE_OVER)
#define IS_ACCESSIBLE_INSIDE(e)	HAS_PROPERTY(e, EP_ACCESSIBLE_INSIDE)
#define IS_ACCESSIBLE_UNDER(e)	HAS_PROPERTY(e, EP_ACCESSIBLE_UNDER)
#define IS_WALKABLE(e)		HAS_PROPERTY(e, EP_WALKABLE)
#define IS_PASSABLE(e)		HAS_PROPERTY(e, EP_PASSABLE)
#define IS_ACCESSIBLE(e)	HAS_PROPERTY(e, EP_ACCESSIBLE)
#define IS_COLLECTIBLE(e)	HAS_PROPERTY(e, EP_COLLECTIBLE)
#define IS_SNAPPABLE(e)		HAS_PROPERTY(e, EP_SNAPPABLE)
#define IS_WALL(e)		HAS_PROPERTY(e, EP_WALL)
#define IS_SOLID_FOR_PUSHING(e)	HAS_PROPERTY(e, EP_SOLID_FOR_PUSHING)
#define IS_DRAGONFIRE_PROOF(e)	HAS_PROPERTY(e, EP_DRAGONFIRE_PROOF)
#define IS_EXPLOSION_PROOF(e)	HAS_PROPERTY(e, EP_EXPLOSION_PROOF)
#define CAN_SMASH(e)		HAS_PROPERTY(e, EP_CAN_SMASH)
#define EXPLODES_3X3_OLD(e)	HAS_PROPERTY(e, EP_EXPLODES_3X3_OLD)
#define CAN_EXPLODE_BY_FIRE(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_FIRE)
#define CAN_EXPLODE_SMASHED(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_SMASHED)
#define CAN_EXPLODE_IMPACT(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_IMPACT)
#define IS_SP_PORT(e)		HAS_PROPERTY(e, EP_SP_PORT)
#define CAN_EXPLODE_BY_DRAGONFIRE(e)	\
				HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_DRAGONFIRE)
#define CAN_EXPLODE_BY_EXPLOSION(e)	\
				HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_EXPLOSION)
#define COULD_MOVE_INTO_ACID(e)	HAS_PROPERTY(e, EP_COULD_MOVE_INTO_ACID)
#define MAYBE_DONT_COLLIDE_WITH(e) HAS_PROPERTY(e, EP_MAYBE_DONT_COLLIDE_WITH)
#define CAN_BE_CLONED_BY_ANDROID(e)	\
				HAS_PROPERTY(e, EP_CAN_BE_CLONED_BY_ANDROID)

#define IS_EDITOR_CASCADE(e)	HAS_PROPERTY(e, EP_EDITOR_CASCADE)
#define IS_EDITOR_CASCADE_ACTIVE(e)	\
				HAS_PROPERTY(e, EP_EDITOR_CASCADE_ACTIVE)
#define IS_EDITOR_CASCADE_INACTIVE(e)	\
				HAS_PROPERTY(e, EP_EDITOR_CASCADE_INACTIVE)

#define HAS_ACTION(e)		HAS_PROPERTY(e, EP_HAS_ACTION)
#define CAN_CHANGE_OR_HAS_ACTION(e)	\
				HAS_PROPERTY(e, EP_CAN_CHANGE_OR_HAS_ACTION)

#define IS_OBSOLETE(e)		HAS_PROPERTY(e, EP_OBSOLETE)

/* special macros used in game engine */
#define IS_CUSTOM_ELEMENT(e)	((e) >= EL_CUSTOM_START &&		\
	 			 (e) <= EL_CUSTOM_END)

#define IS_GROUP_ELEMENT(e)	((e) >= EL_GROUP_START &&		\
	 			 (e) <= EL_GROUP_END)

#define IS_CLIPBOARD_ELEMENT(e)	((e) >= EL_INTERNAL_CLIPBOARD_START &&	\
	 			 (e) <= EL_INTERNAL_CLIPBOARD_END)

#define IS_INTERNAL_ELEMENT(e)	((e) >= EL_INTERNAL_START &&		\
	 			 (e) <= EL_INTERNAL_END)

#define IS_ENVELOPE(e)		((e) >= EL_ENVELOPE_1 &&		\
	 			 (e) <= EL_ENVELOPE_4)

#define IS_RND_KEY(e)		((e) >= EL_KEY_1 &&			\
	 			 (e) <= EL_KEY_4)
#define IS_EM_KEY(e)		((e) >= EL_EM_KEY_1 &&			\
	 			 (e) <= EL_EM_KEY_4)
#define IS_EMC_KEY(e)		((e) >= EL_EMC_KEY_5 &&			\
	 			 (e) <= EL_EMC_KEY_8)
#define IS_KEY(e)		(IS_RND_KEY(e) ||			\
				 IS_EM_KEY(e) ||			\
				 IS_EMC_KEY(e))
#define RND_KEY_NR(e)		((e) - EL_KEY_1)
#define EM_KEY_NR(e)		((e) - EL_EM_KEY_1)
#define EMC_KEY_NR(e)		((e) - EL_EMC_KEY_5 + 4)
#define KEY_NR(e)		(IS_RND_KEY(e) ? RND_KEY_NR(e) :	\
				 IS_EM_KEY(e) ?  EM_KEY_NR(e) :		\
				 IS_EMC_KEY(e) ? EMC_KEY_NR(e) : 0)

#define IS_RND_GATE(e)		((e) >= EL_GATE_1 &&			\
	 			 (e) <= EL_GATE_4)
#define IS_EM_GATE(e)		((e) >= EL_EM_GATE_1 &&			\
	 			 (e) <= EL_EM_GATE_4)
#define IS_EMC_GATE(e)		((e) >= EL_EMC_GATE_5 &&		\
	 			 (e) <= EL_EMC_GATE_8)
#define IS_GATE(e)		(IS_RND_GATE(e) ||			\
				 IS_EM_GATE(e) ||			\
				 IS_EMC_GATE(e))
#define RND_GATE_NR(e)		((e) - EL_GATE_1)
#define EM_GATE_NR(e)		((e) - EL_EM_GATE_1)
#define EMC_GATE_NR(e)		((e) - EL_EMC_GATE_5 + 4)
#define GATE_NR(e)		(IS_RND_GATE(e) ? RND_GATE_NR(e) :	\
				 IS_EM_GATE(e) ?  EM_GATE_NR(e) :	\
				 IS_EMC_GATE(e) ? EMC_GATE_NR(e) : 0)

#define IS_RND_GATE_GRAY(e)	((e) >= EL_GATE_1_GRAY &&		\
	 			 (e) <= EL_GATE_4_GRAY)
#define IS_RND_GATE_GRAY_ACTIVE(e) ((e) >= EL_GATE_1_GRAY_ACTIVE &&	\
	 			 (e) <= EL_GATE_4_GRAY_ACTIVE)
#define IS_EM_GATE_GRAY(e)	((e) >= EL_EM_GATE_1_GRAY &&		\
	 			 (e) <= EL_EM_GATE_4_GRAY)
#define IS_EM_GATE_GRAY_ACTIVE(e) ((e) >= EL_EM_GATE_1_GRAY_ACTIVE &&	\
	 			 (e) <= EL_EM_GATE_4_GRAY_ACTIVE)
#define IS_EMC_GATE_GRAY(e)	((e) >= EL_EMC_GATE_5_GRAY &&		\
	 			 (e) <= EL_EMC_GATE_8_GRAY)
#define IS_EMC_GATE_GRAY_ACTIVE(e) ((e) >= EL_EMC_GATE_5_GRAY_ACTIVE &&	\
	 			 (e) <= EL_EMC_GATE_8_GRAY_ACTIVE)
#define IS_GATE_GRAY(e)		(IS_RND_GATE_GRAY(e) ||			\
				 IS_EM_GATE_GRAY(e) ||			\
				 IS_EMC_GATE_GRAY(e))
#define IS_GATE_GRAY_ACTIVE(e)	(IS_RND_GATE_GRAY_ACTIVE(e) ||		\
				 IS_EM_GATE_GRAY_ACTIVE(e) ||		\
				 IS_EMC_GATE_GRAY_ACTIVE(e))
#define RND_GATE_GRAY_NR(e)	((e) - EL_GATE_1_GRAY)
#define RND_GATE_GRAY_ACTIVE_NR(e) ((e) - EL_GATE_1_GRAY_ACTIVE)
#define EM_GATE_GRAY_NR(e)	((e) - EL_EM_GATE_1_GRAY)
#define EM_GATE_GRAY_ACTIVE_NR(e) ((e) - EL_EM_GATE_1_GRAY_ACTIVE)
#define EMC_GATE_GRAY_NR(e)	((e) - EL_EMC_GATE_5_GRAY + 4)
#define EMC_GATE_GRAY_ACTIVE_NR(e) ((e) - EL_EMC_GATE_5_GRAY_ACTIVE + 4)
#define GATE_GRAY_NR(e)		(IS_RND_GATE_GRAY(e) ? RND_GATE_GRAY_NR(e) :  \
				 IS_EM_GATE_GRAY(e) ?  EM_GATE_GRAY_NR(e) :   \
				 IS_EMC_GATE_GRAY(e) ? EMC_GATE_GRAY_NR(e) : 0)

#define GFX_ELEMENT(e)		(element_info[e].use_gfx_element ?	\
				 element_info[e].gfx_element : e)

/* !!! CHECK THIS !!! */
#if 1
#define TILE_GFX_ELEMENT(x, y)						\
		   (GfxElement[x][y] != EL_UNDEFINED &&			\
		    Feld[x][y] != EL_EXPLOSION ?			\
		    GfxElement[x][y] : Feld[x][y])
#else
#define TILE_GFX_ELEMENT(x, y)						\
	GFX_ELEMENT(GfxElement[x][y] != EL_UNDEFINED &&			\
		    Feld[x][y] != EL_EXPLOSION ?			\
		    GfxElement[x][y] : Feld[x][y])
#endif

/* !!! "use sound" deactivated due to problems with level "bug machine" !!! */
/* (solution: add separate "use sound of element" to level file and editor) */
#if 0
#define SND_ELEMENT(e)		GFX_ELEMENT(e)
#else
#define SND_ELEMENT(e)		(e)
#endif

#if 1
#define GROUP_NR(e)		((e) - EL_GROUP_START)
#define IS_IN_GROUP(e, nr)	(element_info[e].in_group[nr] == TRUE)
#define IS_IN_GROUP_EL(e, ge)	(IS_IN_GROUP(e, (ge) - EL_GROUP_START))

#define IS_EQUAL_OR_IN_GROUP(e, ge)					\
	(IS_GROUP_ELEMENT(ge) ? IS_IN_GROUP(e, GROUP_NR(ge)) : (e) == (ge))
#endif

#define IS_PLAYER(x, y)		(ELEM_IS_PLAYER(StorePlayer[x][y]))

#define IS_FREE(x, y)		(Feld[x][y] == EL_EMPTY && !IS_PLAYER(x, y))
#define IS_FREE_OR_PLAYER(x, y)	(Feld[x][y] == EL_EMPTY)

#define IS_MOVING(x,y)		(MovPos[x][y] != 0)
#define IS_FALLING(x,y)		(MovPos[x][y] != 0 && MovDir[x][y] == MV_DOWN)
#define IS_BLOCKED(x,y)		(Feld[x][y] == EL_BLOCKED)

#define IS_MV_DIAGONAL(x)	((x) & MV_HORIZONTAL && (x) & MV_VERTICAL)

#define EL_CHANGED(e)		((e) == EL_ROCK           ? EL_EMERALD :    \
				 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
				 (e) == EL_EMERALD        ? EL_DIAMOND :    \
				 (e) == EL_EMERALD_YELLOW ? EL_DIAMOND :    \
				 (e) == EL_EMERALD_RED    ? EL_DIAMOND :    \
				 (e) == EL_EMERALD_PURPLE ? EL_DIAMOND :    \
				 EL_ROCK)
#define EL_CHANGED2(e)		((e) == EL_ROCK           ? EL_BD_DIAMOND : \
				 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
				 EL_BD_ROCK)
#define IS_DRAWABLE(e)		((e) < EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)	((e) >= EL_BLOCKED)
#define TAPE_IS_EMPTY(x)	((x).length == 0)
#define TAPE_IS_STOPPED(x)	(!(x).recording && !(x).playing)

#define PLAYERINFO(x,y)		(&stored_player[StorePlayer[x][y]-EL_PLAYER_1])
#define SHIELD_ON(p)		((p)->shield_normal_time_left > 0)

#define ENEMY_PROTECTED_FIELD(x,y)	(IS_PROTECTED(Feld[x][y]) ||       \
					 IS_PROTECTED(Back[x][y]))
#define EXPLOSION_PROTECTED_FIELD(x,y)  (IS_EXPLOSION_PROOF(Feld[x][y]))
#define PLAYER_ENEMY_PROTECTED(x,y)     (SHIELD_ON(PLAYERINFO(x, y)) ||	   \
					 ENEMY_PROTECTED_FIELD(x, y))
#define PLAYER_EXPLOSION_PROTECTED(x,y) (SHIELD_ON(PLAYERINFO(x, y)) ||	   \
					 EXPLOSION_PROTECTED_FIELD(x, y))

#define PLAYER_SWITCHING(p,x,y)	((p)->is_switching &&			\
				 (p)->switch_x == (x) && (p)->switch_y == (y))

#define PLAYER_DROPPING(p,x,y)	((p)->is_dropping &&			\
				 (p)->drop_x == (x) && (p)->drop_y == (y))

#define PLAYER_NR_GFX(g,i)	((g) + i * (IMG_PLAYER_2 - IMG_PLAYER_1))

#define GET_PLAYER_ELEMENT(e)	((e) >= EL_PLAYER_1 && (e) <= EL_PLAYER_4 ? \
				 (e) : EL_PLAYER_1)

#define GET_PLAYER_NR(e)	(GET_PLAYER_ELEMENT(e) - EL_PLAYER_1)

#define ANIM_FRAMES(g)		(graphic_info[g].anim_frames)
#define ANIM_DELAY(g)		(graphic_info[g].anim_delay)
#define ANIM_MODE(g)		(graphic_info[g].anim_mode)

#define IS_ANIMATED(g)		(ANIM_FRAMES(g) > 1)
#define IS_NEW_DELAY(f, g)	((f) % ANIM_DELAY(g) == 0)
#define IS_NEW_FRAME(f, g)	(IS_ANIMATED(g) && IS_NEW_DELAY(f, g))
#define IS_NEXT_FRAME(f, g)	(IS_NEW_FRAME(f, g) && (f) > 0)

#define IS_LOOP_SOUND(s)	(sound_info[s].loop)

#define EL_CASCADE_ACTIVE(e)	(IS_EDITOR_CASCADE_INACTIVE(e) ? (e) + 1 : (e))
#define EL_CASCADE_INACTIVE(e)	(IS_EDITOR_CASCADE_ACTIVE(e)   ? (e) - 1 : (e))
#define EL_CASCADE_TOGGLE(e)	(IS_EDITOR_CASCADE_INACTIVE(e) ? (e) + 1 :    \
				 IS_EDITOR_CASCADE_ACTIVE(e)   ? (e) - 1 : (e))

#define EL_NAME(e)		((e) >= 0 ? element_info[e].token_name : "(?)")
#define MV_TEXT(d)		((d) == MV_NONE  ? "MV_NONE"  :		\
				 (d) == MV_LEFT  ? "MV_LEFT"  :		\
				 (d) == MV_RIGHT ? "MV_RIGHT" :		\
				 (d) == MV_UP    ? "MV_UP"    :		\
				 (d) == MV_DOWN  ? "MV_DOWN"  : "(various)")

/* fundamental game speed values */
#define MICROLEVEL_SCROLL_DELAY	50	/* delay for scrolling micro level */
#define MICROLEVEL_LABEL_DELAY	250	/* delay for micro level label */

/* boundaries of arrays etc. */
#define MAX_LEVEL_NAME_LEN	32
#define MAX_LEVEL_AUTHOR_LEN	32
#define MAX_ELEMENT_NAME_LEN	32
#define MAX_TAPES_PER_SET	1024
#define MAX_SCORE_ENTRIES	100
#define MAX_NUM_TITLE_SCREENS	5

#define MAX_NUM_AMOEBA		100

#if 0	/* game.h */
#define MAX_INVENTORY_SIZE	1000
#define STD_NUM_KEYS		4
#define MAX_NUM_KEYS		8
#endif

#define NUM_BELTS		4
#define NUM_BELT_PARTS		3
#define NUM_ENVELOPES		4
#define MIN_ENVELOPE_XSIZE	1
#define MIN_ENVELOPE_YSIZE	1
#define MAX_ENVELOPE_XSIZE	30
#define MAX_ENVELOPE_YSIZE	20
#define MAX_ENVELOPE_TEXT_LEN	(MAX_ENVELOPE_XSIZE * MAX_ENVELOPE_YSIZE)
#define MIN_CHANGE_PAGES	1
#define MAX_CHANGE_PAGES	32
#define MIN_ELEMENTS_IN_GROUP	1
#define MAX_ELEMENTS_IN_GROUP	16
#define MIN_ANDROID_ELEMENTS	1
#define MAX_ANDROID_ELEMENTS	16

/* values for elements with content */
#define MIN_ELEMENT_CONTENTS	1
#define STD_ELEMENT_CONTENTS	4
#define MAX_ELEMENT_CONTENTS	8

/* often used screen positions */
#define SX			8
#define SY			8
#define REAL_SX			(SX - 2)
#define REAL_SY			(SY - 2)
#define DX			566
#define DY			60
#define VX			DX
#define VY			400
#define EX			DX
#define EY			(VY - 44)
#define TILESIZE		32
#define TILEX			TILESIZE
#define TILEY			TILESIZE
#define MINI_TILESIZE		(TILESIZE / 2)
#define MINI_TILEX		MINI_TILESIZE
#define MINI_TILEY		MINI_TILESIZE
#define MICRO_TILESIZE		(TILESIZE / 8)
#define MICRO_TILEX		MICRO_TILESIZE
#define MICRO_TILEY		MICRO_TILESIZE
#define MIDPOSX			(SCR_FIELDX / 2)
#define MIDPOSY			(SCR_FIELDY / 2)
#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)
#define FXSIZE			((SCR_FIELDX + 2) * TILEX)
#define FYSIZE			((SCR_FIELDY + 2) * TILEY)
#define DXSIZE			100
#define DYSIZE			280
#define VXSIZE			DXSIZE
#define VYSIZE			100
#define EXSIZE			DXSIZE
#define EYSIZE			(VYSIZE + 44)
#define FULL_SXSIZE		(2 + SXSIZE + 2)
#define FULL_SYSIZE		(2 + SYSIZE + 2)
#define MICROLEVEL_XSIZE	((STD_LEV_FIELDX + 2) * MICRO_TILEX)
#define MICROLEVEL_YSIZE	((STD_LEV_FIELDY + 2) * MICRO_TILEY)
#define MICROLEVEL_XPOS		(SX + (SXSIZE - MICROLEVEL_XSIZE) / 2)
#define MICROLEVEL_YPOS		(SX + 12 * TILEY - MICRO_TILEY)
#define MICROLABEL1_YPOS	(MICROLEVEL_YPOS - 36)
#define MICROLABEL2_YPOS	(MICROLEVEL_YPOS + MICROLEVEL_YSIZE + 7)


/* score for elements */
#define SC_EMERALD		0
#define SC_DIAMOND		1
#define SC_BUG			2
#define SC_SPACESHIP		3
#define SC_YAMYAM		4
#define SC_ROBOT		5
#define SC_PACMAN		6
#define SC_NUT			7
#define SC_DYNAMITE		8
#define SC_KEY			9
#define SC_TIME_BONUS		10
#define SC_CRYSTAL		11
#define SC_PEARL		12
#define SC_SHIELD		13
#define SC_UNKNOWN_14		14
#define SC_UNKNOWN_15		15

#define LEVEL_SCORE_ELEMENTS	16	/* level elements with score */


/* "real" level file elements */
#define EL_UNDEFINED			-1

#define EL_EMPTY_SPACE			0
#define EL_EMPTY			EL_EMPTY_SPACE
#define EL_SAND				1
#define EL_WALL				2
#define EL_WALL_SLIPPERY		3
#define EL_ROCK				4
#define EL_KEY_OBSOLETE			5	/* obsolete; now EL_KEY_1 */
#define EL_EMERALD			6
#define EL_EXIT_CLOSED			7
#define EL_PLAYER_OBSOLETE		8	/* obsolete; now EL_PLAYER_1 */
#define EL_BUG				9
#define EL_SPACESHIP			10
#define EL_YAMYAM			11
#define EL_ROBOT			12
#define EL_STEELWALL			13
#define EL_DIAMOND			14
#define EL_AMOEBA_DEAD			15
#define EL_QUICKSAND_EMPTY		16
#define EL_QUICKSAND_FULL		17
#define EL_AMOEBA_DROP			18
#define EL_BOMB				19
#define EL_MAGIC_WALL			20
#define EL_SPEED_PILL			21
#define EL_ACID				22
#define EL_AMOEBA_WET			23
#define EL_AMOEBA_DRY			24
#define EL_NUT				25
#define EL_GAME_OF_LIFE			26
#define EL_BIOMAZE			27
#define EL_DYNAMITE_ACTIVE		28
#define EL_STONEBLOCK			29
#define EL_ROBOT_WHEEL			30
#define EL_ROBOT_WHEEL_ACTIVE		31
#define EL_KEY_1			32
#define EL_KEY_2			33
#define EL_KEY_3			34
#define EL_KEY_4			35
#define EL_GATE_1			36
#define EL_GATE_2			37
#define EL_GATE_3			38
#define EL_GATE_4			39
#define EL_GATE_1_GRAY			40
#define EL_GATE_2_GRAY			41
#define EL_GATE_3_GRAY			42
#define EL_GATE_4_GRAY			43
#define EL_DYNAMITE			44
#define EL_PACMAN			45
#define EL_INVISIBLE_WALL		46
#define EL_LAMP				47
#define EL_LAMP_ACTIVE			48
#define EL_WALL_EMERALD			49
#define EL_WALL_DIAMOND			50
#define EL_AMOEBA_FULL			51
#define EL_BD_AMOEBA			52
#define EL_TIME_ORB_FULL		53
#define EL_TIME_ORB_EMPTY		54
#define EL_EXPANDABLE_WALL		55
#define EL_BD_DIAMOND			56
#define EL_EMERALD_YELLOW		57
#define EL_WALL_BD_DIAMOND		58
#define EL_WALL_EMERALD_YELLOW		59
#define EL_DARK_YAMYAM			60
#define EL_BD_MAGIC_WALL		61
#define EL_INVISIBLE_STEELWALL		62
#define EL_SOKOBAN_FIELD_PLAYER		63
#define EL_DYNABOMB_INCREASE_NUMBER	64
#define EL_DYNABOMB_INCREASE_SIZE	65
#define EL_DYNABOMB_INCREASE_POWER	66
#define EL_SOKOBAN_OBJECT		67
#define EL_SOKOBAN_FIELD_EMPTY		68
#define EL_SOKOBAN_FIELD_FULL		69
#define EL_BD_BUTTERFLY_RIGHT		70
#define EL_BD_BUTTERFLY_UP		71
#define EL_BD_BUTTERFLY_LEFT		72
#define EL_BD_BUTTERFLY_DOWN		73
#define EL_BD_FIREFLY_RIGHT		74
#define EL_BD_FIREFLY_UP		75
#define EL_BD_FIREFLY_LEFT		76
#define EL_BD_FIREFLY_DOWN		77
#define EL_BD_BUTTERFLY_1		EL_BD_BUTTERFLY_DOWN
#define EL_BD_BUTTERFLY_2		EL_BD_BUTTERFLY_LEFT
#define EL_BD_BUTTERFLY_3		EL_BD_BUTTERFLY_UP
#define EL_BD_BUTTERFLY_4		EL_BD_BUTTERFLY_RIGHT
#define EL_BD_FIREFLY_1			EL_BD_FIREFLY_LEFT
#define EL_BD_FIREFLY_2			EL_BD_FIREFLY_DOWN
#define EL_BD_FIREFLY_3			EL_BD_FIREFLY_RIGHT
#define EL_BD_FIREFLY_4			EL_BD_FIREFLY_UP
#define EL_BD_BUTTERFLY			78
#define EL_BD_FIREFLY			79
#define EL_PLAYER_1			80
#define EL_PLAYER_2			81
#define EL_PLAYER_3			82
#define EL_PLAYER_4			83
#define EL_BUG_RIGHT			84
#define EL_BUG_UP			85
#define EL_BUG_LEFT			86
#define EL_BUG_DOWN			87
#define EL_SPACESHIP_RIGHT		88
#define EL_SPACESHIP_UP			89
#define EL_SPACESHIP_LEFT		90
#define EL_SPACESHIP_DOWN		91
#define EL_PACMAN_RIGHT			92
#define EL_PACMAN_UP			93
#define EL_PACMAN_LEFT			94
#define EL_PACMAN_DOWN			95
#define EL_EMERALD_RED			96
#define EL_EMERALD_PURPLE		97
#define EL_WALL_EMERALD_RED		98
#define EL_WALL_EMERALD_PURPLE		99
#define EL_ACID_POOL_TOPLEFT		100
#define EL_ACID_POOL_TOPRIGHT		101
#define EL_ACID_POOL_BOTTOMLEFT		102
#define EL_ACID_POOL_BOTTOM		103
#define EL_ACID_POOL_BOTTOMRIGHT	104
#define EL_BD_WALL			105
#define EL_BD_ROCK			106
#define EL_EXIT_OPEN			107
#define EL_BLACK_ORB			108
#define EL_AMOEBA_TO_DIAMOND		109
#define EL_MOLE				110
#define EL_PENGUIN			111
#define EL_SATELLITE			112
#define EL_ARROW_LEFT			113
#define EL_ARROW_RIGHT			114
#define EL_ARROW_UP			115
#define EL_ARROW_DOWN			116
#define EL_PIG				117
#define EL_DRAGON			118

#define EL_EM_KEY_1_FILE_OBSOLETE	119	/* obsolete; now EL_EM_KEY_1 */

#define EL_CHAR_START			120
#define EL_CHAR_ASCII0			(EL_CHAR_START  - 32)
#define EL_CHAR_ASCII0_START		(EL_CHAR_ASCII0 + 32)

#include "conf_chr.h"	/* include auto-generated data structure definitions */

#define EL_CHAR_ASCII0_END		(EL_CHAR_ASCII0 + 111)
#define EL_CHAR_END			(EL_CHAR_START  + 79)

#define EL_CHAR(c)			(EL_CHAR_ASCII0 + MAP_FONT_ASCII(c))

#define EL_EXPANDABLE_WALL_HORIZONTAL	200
#define EL_EXPANDABLE_WALL_VERTICAL	201
#define EL_EXPANDABLE_WALL_ANY		202

#define EL_EM_GATE_1			203
#define EL_EM_GATE_2			204
#define EL_EM_GATE_3			205
#define EL_EM_GATE_4			206

#define EL_EM_KEY_2_FILE_OBSOLETE	207	/* obsolete; now EL_EM_KEY_2 */
#define EL_EM_KEY_3_FILE_OBSOLETE	208	/* obsolete; now EL_EM_KEY_3 */
#define EL_EM_KEY_4_FILE_OBSOLETE	209	/* obsolete; now EL_EM_KEY_4 */

#define EL_SP_START			210
#define EL_SP_EMPTY_SPACE		(EL_SP_START + 0)
#define EL_SP_EMPTY			EL_SP_EMPTY_SPACE
#define EL_SP_ZONK			(EL_SP_START + 1)
#define EL_SP_BASE			(EL_SP_START + 2)
#define EL_SP_MURPHY			(EL_SP_START + 3)
#define EL_SP_INFOTRON			(EL_SP_START + 4)
#define EL_SP_CHIP_SINGLE		(EL_SP_START + 5)
#define EL_SP_HARDWARE_GRAY		(EL_SP_START + 6)
#define EL_SP_EXIT_CLOSED		(EL_SP_START + 7)
#define EL_SP_DISK_ORANGE		(EL_SP_START + 8)
#define EL_SP_PORT_RIGHT		(EL_SP_START + 9)
#define EL_SP_PORT_DOWN			(EL_SP_START + 10)
#define EL_SP_PORT_LEFT			(EL_SP_START + 11)
#define EL_SP_PORT_UP			(EL_SP_START + 12)
#define EL_SP_GRAVITY_PORT_RIGHT	(EL_SP_START + 13)
#define EL_SP_GRAVITY_PORT_DOWN		(EL_SP_START + 14)
#define EL_SP_GRAVITY_PORT_LEFT		(EL_SP_START + 15)
#define EL_SP_GRAVITY_PORT_UP		(EL_SP_START + 16)
#define EL_SP_SNIKSNAK			(EL_SP_START + 17)
#define EL_SP_DISK_YELLOW		(EL_SP_START + 18)
#define EL_SP_TERMINAL			(EL_SP_START + 19)
#define EL_SP_DISK_RED			(EL_SP_START + 20)
#define EL_SP_PORT_VERTICAL		(EL_SP_START + 21)
#define EL_SP_PORT_HORIZONTAL		(EL_SP_START + 22)
#define EL_SP_PORT_ANY			(EL_SP_START + 23)
#define EL_SP_ELECTRON			(EL_SP_START + 24)
#define EL_SP_BUGGY_BASE		(EL_SP_START + 25)
#define EL_SP_CHIP_LEFT			(EL_SP_START + 26)
#define EL_SP_CHIP_RIGHT		(EL_SP_START + 27)
#define EL_SP_HARDWARE_BASE_1		(EL_SP_START + 28)
#define EL_SP_HARDWARE_GREEN		(EL_SP_START + 29)
#define EL_SP_HARDWARE_BLUE		(EL_SP_START + 30)
#define EL_SP_HARDWARE_RED		(EL_SP_START + 31)
#define EL_SP_HARDWARE_YELLOW		(EL_SP_START + 32)
#define EL_SP_HARDWARE_BASE_2		(EL_SP_START + 33)
#define EL_SP_HARDWARE_BASE_3		(EL_SP_START + 34)
#define EL_SP_HARDWARE_BASE_4		(EL_SP_START + 35)
#define EL_SP_HARDWARE_BASE_5		(EL_SP_START + 36)
#define EL_SP_HARDWARE_BASE_6		(EL_SP_START + 37)
#define EL_SP_CHIP_TOP			(EL_SP_START + 38)
#define EL_SP_CHIP_BOTTOM		(EL_SP_START + 39)
#define EL_SP_END			(EL_SP_START + 39)

#define EL_EM_GATE_1_GRAY		250
#define EL_EM_GATE_2_GRAY		251
#define EL_EM_GATE_3_GRAY		252
#define EL_EM_GATE_4_GRAY		253

#define EL_EM_DYNAMITE			254
#define EL_EM_DYNAMITE_ACTIVE		255

#define EL_PEARL			256
#define EL_CRYSTAL			257
#define EL_WALL_PEARL			258
#define EL_WALL_CRYSTAL			259
#define EL_DOOR_WHITE			260
#define EL_DOOR_WHITE_GRAY		261
#define EL_KEY_WHITE			262
#define EL_SHIELD_NORMAL		263
#define EL_EXTRA_TIME			264
#define EL_SWITCHGATE_OPEN		265
#define EL_SWITCHGATE_CLOSED		266
#define EL_SWITCHGATE_SWITCH_UP		267
#define EL_SWITCHGATE_SWITCH_DOWN	268

#define EL_UNUSED_269			269
#define EL_UNUSED_270			270

#define EL_CONVEYOR_BELT_1_LEFT		 271
#define EL_CONVEYOR_BELT_1_MIDDLE	 272
#define EL_CONVEYOR_BELT_1_RIGHT	 273
#define EL_CONVEYOR_BELT_1_SWITCH_LEFT	 274
#define EL_CONVEYOR_BELT_1_SWITCH_MIDDLE 275
#define EL_CONVEYOR_BELT_1_SWITCH_RIGHT	 276
#define EL_CONVEYOR_BELT_2_LEFT		 277
#define EL_CONVEYOR_BELT_2_MIDDLE	 278
#define EL_CONVEYOR_BELT_2_RIGHT	 279
#define EL_CONVEYOR_BELT_2_SWITCH_LEFT	 280
#define EL_CONVEYOR_BELT_2_SWITCH_MIDDLE 281
#define EL_CONVEYOR_BELT_2_SWITCH_RIGHT	 282
#define EL_CONVEYOR_BELT_3_LEFT		 283
#define EL_CONVEYOR_BELT_3_MIDDLE	 284
#define EL_CONVEYOR_BELT_3_RIGHT	 285
#define EL_CONVEYOR_BELT_3_SWITCH_LEFT	 286
#define EL_CONVEYOR_BELT_3_SWITCH_MIDDLE 287
#define EL_CONVEYOR_BELT_3_SWITCH_RIGHT	 288
#define EL_CONVEYOR_BELT_4_LEFT		 289
#define EL_CONVEYOR_BELT_4_MIDDLE	 290
#define EL_CONVEYOR_BELT_4_RIGHT	 291
#define EL_CONVEYOR_BELT_4_SWITCH_LEFT	 292
#define EL_CONVEYOR_BELT_4_SWITCH_MIDDLE 293
#define EL_CONVEYOR_BELT_4_SWITCH_RIGHT	 294
#define EL_LANDMINE			295
#define EL_ENVELOPE_OBSOLETE		296   /* obsolete; now EL_ENVELOPE_1 */
#define EL_LIGHT_SWITCH			297
#define EL_LIGHT_SWITCH_ACTIVE		298
#define EL_SIGN_EXCLAMATION		299
#define EL_SIGN_RADIOACTIVITY		300
#define EL_SIGN_STOP			301
#define EL_SIGN_WHEELCHAIR		302
#define EL_SIGN_PARKING			303
#define EL_SIGN_ONEWAY			304
#define EL_SIGN_HEART			305
#define EL_SIGN_TRIANGLE		306
#define EL_SIGN_ROUND			307
#define EL_SIGN_EXIT			308
#define EL_SIGN_YINYANG			309
#define EL_SIGN_OTHER			310
#define EL_MOLE_LEFT			311
#define EL_MOLE_RIGHT			312
#define EL_MOLE_UP			313
#define EL_MOLE_DOWN			314
#define EL_STEELWALL_SLIPPERY		315
#define EL_INVISIBLE_SAND		316
#define EL_DX_UNKNOWN_15		317
#define EL_DX_UNKNOWN_42		318

#define EL_UNUSED_319			319
#define EL_UNUSED_320			320

#define EL_SHIELD_DEADLY		321
#define EL_TIMEGATE_OPEN		322
#define EL_TIMEGATE_CLOSED		323
#define EL_TIMEGATE_SWITCH_ACTIVE	324
#define EL_TIMEGATE_SWITCH		325

#define EL_BALLOON			326
#define EL_BALLOON_SWITCH_LEFT		327
#define EL_BALLOON_SWITCH_RIGHT		328
#define EL_BALLOON_SWITCH_UP		329
#define EL_BALLOON_SWITCH_DOWN		330
#define EL_BALLOON_SWITCH_ANY		331

#define EL_EMC_STEELWALL_1		332
#define EL_EMC_STEELWALL_2		333
#define EL_EMC_STEELWALL_3 		334
#define EL_EMC_STEELWALL_4		335
#define EL_EMC_WALL_1			336
#define EL_EMC_WALL_2			337
#define EL_EMC_WALL_3			338
#define EL_EMC_WALL_4			339
#define EL_EMC_WALL_5			340
#define EL_EMC_WALL_6			341
#define EL_EMC_WALL_7			342
#define EL_EMC_WALL_8			343

#define EL_TUBE_ANY			344
#define EL_TUBE_VERTICAL		345
#define EL_TUBE_HORIZONTAL		346
#define EL_TUBE_VERTICAL_LEFT		347
#define EL_TUBE_VERTICAL_RIGHT		348
#define EL_TUBE_HORIZONTAL_UP		349
#define EL_TUBE_HORIZONTAL_DOWN		350
#define EL_TUBE_LEFT_UP			351
#define EL_TUBE_LEFT_DOWN		352
#define EL_TUBE_RIGHT_UP		353
#define EL_TUBE_RIGHT_DOWN		354
#define EL_SPRING			355
#define EL_TRAP				356
#define EL_DX_SUPABOMB			357

#define EL_UNUSED_358			358
#define EL_UNUSED_359			359

/* ---------- begin of custom elements section ----------------------------- */
#define EL_CUSTOM_START			360

#include "conf_cus.h"	/* include auto-generated data structure definitions */

#define NUM_CUSTOM_ELEMENTS		256
#define EL_CUSTOM_END			615
/* ---------- end of custom elements section ------------------------------- */

#define EL_EM_KEY_1			616
#define EL_EM_KEY_2			617
#define EL_EM_KEY_3			618
#define EL_EM_KEY_4			619
#define EL_ENVELOPE_1			620
#define EL_ENVELOPE_2			621
#define EL_ENVELOPE_3			622
#define EL_ENVELOPE_4			623

/* ---------- begin of group elements section ------------------------------ */
#define EL_GROUP_START			624

#include "conf_grp.h"	/* include auto-generated data structure definitions */

#define NUM_GROUP_ELEMENTS		32
#define EL_GROUP_END			655
/* ---------- end of custom elements section ------------------------------- */

#define EL_UNKNOWN			656
#define EL_TRIGGER_ELEMENT		657
#define EL_TRIGGER_PLAYER		658

/* SP style elements */
#define EL_SP_GRAVITY_ON_PORT_RIGHT	659
#define EL_SP_GRAVITY_ON_PORT_DOWN	660
#define EL_SP_GRAVITY_ON_PORT_LEFT	661
#define EL_SP_GRAVITY_ON_PORT_UP	662
#define EL_SP_GRAVITY_OFF_PORT_RIGHT	663
#define EL_SP_GRAVITY_OFF_PORT_DOWN	664
#define EL_SP_GRAVITY_OFF_PORT_LEFT	665
#define EL_SP_GRAVITY_OFF_PORT_UP	666

/* EMC style elements */
#define EL_BALLOON_SWITCH_NONE		667
#define EL_EMC_GATE_5			668
#define EL_EMC_GATE_6			669
#define EL_EMC_GATE_7			670
#define EL_EMC_GATE_8			671
#define EL_EMC_GATE_5_GRAY		672
#define EL_EMC_GATE_6_GRAY		673
#define EL_EMC_GATE_7_GRAY		674
#define EL_EMC_GATE_8_GRAY		675
#define EL_EMC_KEY_5			676
#define EL_EMC_KEY_6			677
#define EL_EMC_KEY_7			678
#define EL_EMC_KEY_8			679
#define EL_EMC_ANDROID			680
#define EL_EMC_GRASS			681
#define EL_EMC_MAGIC_BALL		682
#define EL_EMC_MAGIC_BALL_ACTIVE	683
#define EL_EMC_MAGIC_BALL_SWITCH	684
#define EL_EMC_MAGIC_BALL_SWITCH_ACTIVE	685
#define EL_EMC_SPRING_BUMPER		686
#define EL_EMC_PLANT			687
#define EL_EMC_LENSES			688
#define EL_EMC_MAGNIFIER		689
#define EL_EMC_WALL_9			690
#define EL_EMC_WALL_10			691
#define EL_EMC_WALL_11			692
#define EL_EMC_WALL_12			693
#define EL_EMC_WALL_13			694
#define EL_EMC_WALL_14			695
#define EL_EMC_WALL_15			696
#define EL_EMC_WALL_16			697
#define EL_EMC_WALL_SLIPPERY_1		698
#define EL_EMC_WALL_SLIPPERY_2		699
#define EL_EMC_WALL_SLIPPERY_3		700
#define EL_EMC_WALL_SLIPPERY_4		701
#define EL_EMC_FAKE_GRASS		702
#define EL_EMC_FAKE_ACID		703
#define EL_EMC_DRIPPER			704

#define EL_TRIGGER_CE_VALUE		705
#define EL_TRIGGER_CE_SCORE		706
#define EL_CURRENT_CE_VALUE		707
#define EL_CURRENT_CE_SCORE		708

#define EL_YAMYAM_LEFT			709
#define EL_YAMYAM_RIGHT			710
#define EL_YAMYAM_UP			711
#define EL_YAMYAM_DOWN			712

#define EL_BD_EXPANDABLE_WALL		713

#define NUM_FILE_ELEMENTS		714


/* "real" (and therefore drawable) runtime elements */
#define EL_FIRST_RUNTIME_REAL		NUM_FILE_ELEMENTS

#define EL_DYNABOMB_PLAYER_1_ACTIVE	(EL_FIRST_RUNTIME_REAL + 0)
#define EL_DYNABOMB_PLAYER_2_ACTIVE	(EL_FIRST_RUNTIME_REAL + 1)
#define EL_DYNABOMB_PLAYER_3_ACTIVE	(EL_FIRST_RUNTIME_REAL + 2)
#define EL_DYNABOMB_PLAYER_4_ACTIVE	(EL_FIRST_RUNTIME_REAL + 3)
#define EL_SP_DISK_RED_ACTIVE		(EL_FIRST_RUNTIME_REAL + 4)
#define EL_SWITCHGATE_OPENING		(EL_FIRST_RUNTIME_REAL + 5)
#define EL_SWITCHGATE_CLOSING		(EL_FIRST_RUNTIME_REAL + 6)
#define EL_TIMEGATE_OPENING		(EL_FIRST_RUNTIME_REAL + 7)
#define EL_TIMEGATE_CLOSING		(EL_FIRST_RUNTIME_REAL + 8)
#define EL_PEARL_BREAKING		(EL_FIRST_RUNTIME_REAL + 9)
#define EL_TRAP_ACTIVE			(EL_FIRST_RUNTIME_REAL + 10)
#define EL_INVISIBLE_STEELWALL_ACTIVE	(EL_FIRST_RUNTIME_REAL + 11)
#define EL_INVISIBLE_WALL_ACTIVE	(EL_FIRST_RUNTIME_REAL + 12)
#define EL_INVISIBLE_SAND_ACTIVE	(EL_FIRST_RUNTIME_REAL + 13)
#define EL_CONVEYOR_BELT_1_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 14)
#define EL_CONVEYOR_BELT_1_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 15)
#define EL_CONVEYOR_BELT_1_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 16)
#define EL_CONVEYOR_BELT_2_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 17)
#define EL_CONVEYOR_BELT_2_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 18)
#define EL_CONVEYOR_BELT_2_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 19)
#define EL_CONVEYOR_BELT_3_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 20)
#define EL_CONVEYOR_BELT_3_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 21)
#define EL_CONVEYOR_BELT_3_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 22)
#define EL_CONVEYOR_BELT_4_LEFT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 23)
#define EL_CONVEYOR_BELT_4_MIDDLE_ACTIVE (EL_FIRST_RUNTIME_REAL + 24)
#define EL_CONVEYOR_BELT_4_RIGHT_ACTIVE	 (EL_FIRST_RUNTIME_REAL + 25)
#define EL_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 26)
#define EL_EXIT_CLOSING			(EL_FIRST_RUNTIME_REAL + 27)
#define EL_SP_EXIT_OPENING		(EL_FIRST_RUNTIME_REAL + 28)
#define EL_SP_EXIT_CLOSING		(EL_FIRST_RUNTIME_REAL + 29)
#define EL_SP_EXIT_OPEN			(EL_FIRST_RUNTIME_REAL + 30)
#define EL_SP_TERMINAL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 31)
#define EL_SP_BUGGY_BASE_ACTIVATING	(EL_FIRST_RUNTIME_REAL + 32)
#define EL_SP_BUGGY_BASE_ACTIVE		(EL_FIRST_RUNTIME_REAL + 33)
#define EL_SP_MURPHY_CLONE		(EL_FIRST_RUNTIME_REAL + 34)
#define EL_AMOEBA_DROPPING		(EL_FIRST_RUNTIME_REAL + 35)
#define EL_QUICKSAND_EMPTYING		(EL_FIRST_RUNTIME_REAL + 36)
#define EL_MAGIC_WALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 37)
#define EL_BD_MAGIC_WALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 38)
#define EL_MAGIC_WALL_FULL		(EL_FIRST_RUNTIME_REAL + 39)
#define EL_BD_MAGIC_WALL_FULL		(EL_FIRST_RUNTIME_REAL + 40)
#define EL_MAGIC_WALL_EMPTYING		(EL_FIRST_RUNTIME_REAL + 41)
#define EL_BD_MAGIC_WALL_EMPTYING	(EL_FIRST_RUNTIME_REAL + 42)
#define EL_MAGIC_WALL_DEAD		(EL_FIRST_RUNTIME_REAL + 43)
#define EL_BD_MAGIC_WALL_DEAD		(EL_FIRST_RUNTIME_REAL + 44)
#define EL_EMC_FAKE_GRASS_ACTIVE	(EL_FIRST_RUNTIME_REAL + 45)
#define EL_GATE_1_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 46)
#define EL_GATE_2_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 47)
#define EL_GATE_3_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 48)
#define EL_GATE_4_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 49)
#define EL_EM_GATE_1_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 50)
#define EL_EM_GATE_2_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 51)
#define EL_EM_GATE_3_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 52)
#define EL_EM_GATE_4_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 53)
#define EL_EMC_GATE_5_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 54)
#define EL_EMC_GATE_6_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 55)
#define EL_EMC_GATE_7_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 56)
#define EL_EMC_GATE_8_GRAY_ACTIVE	(EL_FIRST_RUNTIME_REAL + 57)
#define EL_EMC_DRIPPER_ACTIVE		(EL_FIRST_RUNTIME_REAL + 58)
#define EL_EMC_SPRING_BUMPER_ACTIVE	(EL_FIRST_RUNTIME_REAL + 59)

/* "unreal" (and therefore not drawable) runtime elements */
#define EL_FIRST_RUNTIME_UNREAL		(EL_FIRST_RUNTIME_REAL + 60)

#define EL_BLOCKED			(EL_FIRST_RUNTIME_UNREAL + 0)
#define EL_EXPLOSION			(EL_FIRST_RUNTIME_UNREAL + 1)
#define EL_NUT_BREAKING			(EL_FIRST_RUNTIME_UNREAL + 2)
#define EL_DIAMOND_BREAKING		(EL_FIRST_RUNTIME_UNREAL + 3)
#define EL_ACID_SPLASH_LEFT		(EL_FIRST_RUNTIME_UNREAL + 4)
#define EL_ACID_SPLASH_RIGHT		(EL_FIRST_RUNTIME_UNREAL + 5)
#define EL_AMOEBA_GROWING		(EL_FIRST_RUNTIME_UNREAL + 6)
#define EL_AMOEBA_SHRINKING		(EL_FIRST_RUNTIME_UNREAL + 7)
#define EL_EXPANDABLE_WALL_GROWING	(EL_FIRST_RUNTIME_UNREAL + 8)
#define EL_FLAMES			(EL_FIRST_RUNTIME_UNREAL + 9)
#define EL_PLAYER_IS_LEAVING		(EL_FIRST_RUNTIME_UNREAL + 10)
#define EL_PLAYER_IS_EXPLODING_1	(EL_FIRST_RUNTIME_UNREAL + 11)
#define EL_PLAYER_IS_EXPLODING_2	(EL_FIRST_RUNTIME_UNREAL + 12)
#define EL_PLAYER_IS_EXPLODING_3	(EL_FIRST_RUNTIME_UNREAL + 13)
#define EL_PLAYER_IS_EXPLODING_4	(EL_FIRST_RUNTIME_UNREAL + 14)
#define EL_QUICKSAND_FILLING		(EL_FIRST_RUNTIME_UNREAL + 15)
#define EL_MAGIC_WALL_FILLING		(EL_FIRST_RUNTIME_UNREAL + 16)
#define EL_BD_MAGIC_WALL_FILLING	(EL_FIRST_RUNTIME_UNREAL + 17)
#define EL_ELEMENT_SNAPPING		(EL_FIRST_RUNTIME_UNREAL + 18)
#define EL_DIAGONAL_SHRINKING		(EL_FIRST_RUNTIME_UNREAL + 19)
#define EL_DIAGONAL_GROWING		(EL_FIRST_RUNTIME_UNREAL + 20)

#define NUM_RUNTIME_ELEMENTS		(EL_FIRST_RUNTIME_UNREAL + 21)

/* dummy elements (never used as game elements, only used as graphics) */
#define EL_FIRST_DUMMY			NUM_RUNTIME_ELEMENTS

#define EL_STEELWALL_TOPLEFT		(EL_FIRST_DUMMY + 0)
#define EL_STEELWALL_TOPRIGHT		(EL_FIRST_DUMMY + 1)
#define EL_STEELWALL_BOTTOMLEFT		(EL_FIRST_DUMMY + 2)
#define EL_STEELWALL_BOTTOMRIGHT	(EL_FIRST_DUMMY + 3)
#define EL_STEELWALL_HORIZONTAL		(EL_FIRST_DUMMY + 4)
#define EL_STEELWALL_VERTICAL		(EL_FIRST_DUMMY + 5)
#define EL_INVISIBLE_STEELWALL_TOPLEFT	   (EL_FIRST_DUMMY + 6)
#define EL_INVISIBLE_STEELWALL_TOPRIGHT	   (EL_FIRST_DUMMY + 7)
#define EL_INVISIBLE_STEELWALL_BOTTOMLEFT  (EL_FIRST_DUMMY + 8)
#define EL_INVISIBLE_STEELWALL_BOTTOMRIGHT (EL_FIRST_DUMMY + 9)
#define EL_INVISIBLE_STEELWALL_HORIZONTAL  (EL_FIRST_DUMMY + 10)
#define EL_INVISIBLE_STEELWALL_VERTICAL	   (EL_FIRST_DUMMY + 11)
#define EL_DYNABOMB			(EL_FIRST_DUMMY + 12)
#define EL_DYNABOMB_ACTIVE		(EL_FIRST_DUMMY + 13)
#define EL_DYNABOMB_PLAYER_1		(EL_FIRST_DUMMY + 14)
#define EL_DYNABOMB_PLAYER_2		(EL_FIRST_DUMMY + 15)
#define EL_DYNABOMB_PLAYER_3		(EL_FIRST_DUMMY + 16)
#define EL_DYNABOMB_PLAYER_4		(EL_FIRST_DUMMY + 17)
#define EL_SHIELD_NORMAL_ACTIVE		(EL_FIRST_DUMMY + 18)
#define EL_SHIELD_DEADLY_ACTIVE		(EL_FIRST_DUMMY + 19)
#define EL_AMOEBA			(EL_FIRST_DUMMY + 20)
#define EL_DEFAULT			(EL_FIRST_DUMMY + 21)
#define EL_BD_DEFAULT			(EL_FIRST_DUMMY + 22)
#define EL_SP_DEFAULT			(EL_FIRST_DUMMY + 23)
#define EL_SB_DEFAULT			(EL_FIRST_DUMMY + 24)

/* internal elements (only used for internal purposes like copying) */
#define EL_FIRST_INTERNAL		(EL_FIRST_DUMMY + 25)

#define EL_INTERNAL_CLIPBOARD_CUSTOM	(EL_FIRST_INTERNAL + 0)
#define EL_INTERNAL_CLIPBOARD_CHANGE	(EL_FIRST_INTERNAL + 1)
#define EL_INTERNAL_CLIPBOARD_GROUP	(EL_FIRST_INTERNAL + 2)
#define EL_INTERNAL_DUMMY		(EL_FIRST_INTERNAL + 3)

#define EL_INTERNAL_CASCADE_BD			(EL_FIRST_INTERNAL + 4)
#define EL_INTERNAL_CASCADE_BD_ACTIVE		(EL_FIRST_INTERNAL + 5)
#define EL_INTERNAL_CASCADE_EM			(EL_FIRST_INTERNAL + 6)
#define EL_INTERNAL_CASCADE_EM_ACTIVE		(EL_FIRST_INTERNAL + 7)
#define EL_INTERNAL_CASCADE_EMC			(EL_FIRST_INTERNAL + 8)
#define EL_INTERNAL_CASCADE_EMC_ACTIVE		(EL_FIRST_INTERNAL + 9)
#define EL_INTERNAL_CASCADE_RND			(EL_FIRST_INTERNAL + 10)
#define EL_INTERNAL_CASCADE_RND_ACTIVE		(EL_FIRST_INTERNAL + 11)
#define EL_INTERNAL_CASCADE_SB			(EL_FIRST_INTERNAL + 12)
#define EL_INTERNAL_CASCADE_SB_ACTIVE		(EL_FIRST_INTERNAL + 13)
#define EL_INTERNAL_CASCADE_SP			(EL_FIRST_INTERNAL + 14)
#define EL_INTERNAL_CASCADE_SP_ACTIVE		(EL_FIRST_INTERNAL + 15)
#define EL_INTERNAL_CASCADE_DC			(EL_FIRST_INTERNAL + 16)
#define EL_INTERNAL_CASCADE_DC_ACTIVE		(EL_FIRST_INTERNAL + 17)
#define EL_INTERNAL_CASCADE_DX			(EL_FIRST_INTERNAL + 18)
#define EL_INTERNAL_CASCADE_DX_ACTIVE		(EL_FIRST_INTERNAL + 19)
#define EL_INTERNAL_CASCADE_CHARS		(EL_FIRST_INTERNAL + 20)
#define EL_INTERNAL_CASCADE_CHARS_ACTIVE	(EL_FIRST_INTERNAL + 21)
#define EL_INTERNAL_CASCADE_CE			(EL_FIRST_INTERNAL + 22)
#define EL_INTERNAL_CASCADE_CE_ACTIVE		(EL_FIRST_INTERNAL + 23)
#define EL_INTERNAL_CASCADE_GE			(EL_FIRST_INTERNAL + 24)
#define EL_INTERNAL_CASCADE_GE_ACTIVE		(EL_FIRST_INTERNAL + 25)
#define EL_INTERNAL_CASCADE_REF			(EL_FIRST_INTERNAL + 26)
#define EL_INTERNAL_CASCADE_REF_ACTIVE		(EL_FIRST_INTERNAL + 27)
#define EL_INTERNAL_CASCADE_USER		(EL_FIRST_INTERNAL + 28)
#define EL_INTERNAL_CASCADE_USER_ACTIVE		(EL_FIRST_INTERNAL + 29)
#define EL_INTERNAL_CASCADE_DYNAMIC		(EL_FIRST_INTERNAL + 30)
#define EL_INTERNAL_CASCADE_DYNAMIC_ACTIVE	(EL_FIRST_INTERNAL + 31)

#define EL_INTERNAL_CLIPBOARD_START	(EL_FIRST_INTERNAL + 0)
#define EL_INTERNAL_CLIPBOARD_END	(EL_FIRST_INTERNAL + 2)
#define EL_INTERNAL_START		(EL_FIRST_INTERNAL + 0)
#define EL_INTERNAL_END			(EL_FIRST_INTERNAL + 31)

#define MAX_NUM_ELEMENTS		(EL_FIRST_INTERNAL + 32)


/* values for graphics/sounds action types */
#define ACTION_DEFAULT			0
#define ACTION_WAITING			1
#define ACTION_FALLING			2
#define ACTION_MOVING			3
#define ACTION_DIGGING			4
#define ACTION_SNAPPING			5
#define ACTION_COLLECTING		6
#define ACTION_DROPPING			7
#define ACTION_PUSHING			8
#define ACTION_WALKING			9
#define ACTION_PASSING			10
#define ACTION_IMPACT			11
#define ACTION_BREAKING			12
#define ACTION_ACTIVATING		13
#define ACTION_DEACTIVATING		14
#define ACTION_OPENING			15
#define ACTION_CLOSING			16
#define ACTION_ATTACKING		17
#define ACTION_GROWING			18
#define ACTION_SHRINKING		19
#define ACTION_ACTIVE			20
#define ACTION_FILLING			21
#define ACTION_EMPTYING			22
#define ACTION_CHANGING			23
#define ACTION_EXPLODING		24
#define ACTION_BORING			25
#define ACTION_BORING_1			26
#define ACTION_BORING_2			27
#define ACTION_BORING_3			28
#define ACTION_BORING_4			29
#define ACTION_BORING_5			30
#define ACTION_BORING_6			31
#define ACTION_BORING_7			32
#define ACTION_BORING_8			33
#define ACTION_BORING_9			34
#define ACTION_BORING_10		35
#define ACTION_SLEEPING			36
#define ACTION_SLEEPING_1		37
#define ACTION_SLEEPING_2		38
#define ACTION_SLEEPING_3		39
#define ACTION_AWAKENING		40
#define ACTION_DYING			41
#define ACTION_TURNING			42
#define ACTION_TURNING_FROM_LEFT	43
#define ACTION_TURNING_FROM_RIGHT	44
#define ACTION_TURNING_FROM_UP		45
#define ACTION_TURNING_FROM_DOWN	46
#define ACTION_SMASHED_BY_ROCK		47
#define ACTION_SMASHED_BY_SPRING	48
#define ACTION_EATING			49
#define ACTION_TWINKLING		50
#define ACTION_SPLASHING		51
#define ACTION_PAGE_1			52
#define ACTION_PAGE_2			53
#define ACTION_PAGE_3			54
#define ACTION_PAGE_4			55
#define ACTION_PAGE_5			56
#define ACTION_PAGE_6			57
#define ACTION_PAGE_7			58
#define ACTION_PAGE_8			59
#define ACTION_PAGE_9			60
#define ACTION_PAGE_10			61
#define ACTION_PAGE_11			62
#define ACTION_PAGE_12			63
#define ACTION_PAGE_13			64
#define ACTION_PAGE_14			65
#define ACTION_PAGE_15			66
#define ACTION_PAGE_16			67
#define ACTION_PAGE_17			68
#define ACTION_PAGE_18			69
#define ACTION_PAGE_19			70
#define ACTION_PAGE_20			71
#define ACTION_PAGE_21			72
#define ACTION_PAGE_22			73
#define ACTION_PAGE_23			74
#define ACTION_PAGE_24			75
#define ACTION_PAGE_25			76
#define ACTION_PAGE_26			77
#define ACTION_PAGE_27			78
#define ACTION_PAGE_28			79
#define ACTION_PAGE_29			80
#define ACTION_PAGE_30			81
#define ACTION_PAGE_31			82
#define ACTION_PAGE_32			83
#define ACTION_OTHER			84

#define NUM_ACTIONS			85

#define ACTION_BORING_LAST		ACTION_BORING_10
#define ACTION_SLEEPING_LAST		ACTION_SLEEPING_3


/* values for special image configuration suffixes (must match game mode) */
#define GFX_SPECIAL_ARG_DEFAULT		0
#define GFX_SPECIAL_ARG_TITLE		1
#define GFX_SPECIAL_ARG_MAIN		2
#define GFX_SPECIAL_ARG_LEVELS		3
#define GFX_SPECIAL_ARG_SCORES		4
#define GFX_SPECIAL_ARG_EDITOR		5
#define GFX_SPECIAL_ARG_INFO		6
#define GFX_SPECIAL_ARG_SETUP		7
#define GFX_SPECIAL_ARG_PLAYING		8
#define GFX_SPECIAL_ARG_DOOR		9
#define GFX_SPECIAL_ARG_PREVIEW		10
#define GFX_SPECIAL_ARG_CRUMBLED	11

#define NUM_SPECIAL_GFX_ARGS		12


/* values for image configuration suffixes */
#define GFX_ARG_X			0
#define GFX_ARG_Y			1
#define GFX_ARG_XPOS			2
#define GFX_ARG_YPOS			3
#define GFX_ARG_WIDTH			4
#define GFX_ARG_HEIGHT			5
#define GFX_ARG_VERTICAL		6
#define GFX_ARG_OFFSET			7
#define GFX_ARG_XOFFSET			8
#define GFX_ARG_YOFFSET			9
#define GFX_ARG_2ND_MOVEMENT_TILE	10
#define GFX_ARG_2ND_VERTICAL		11
#define GFX_ARG_2ND_OFFSET		12
#define GFX_ARG_2ND_XOFFSET		13
#define GFX_ARG_2ND_YOFFSET		14
#define GFX_ARG_2ND_SWAP_TILES		15	
#define GFX_ARG_FRAMES			16
#define GFX_ARG_FRAMES_PER_LINE		17
#define GFX_ARG_START_FRAME		18
#define GFX_ARG_DELAY			19
#define GFX_ARG_ANIM_MODE		20
#define GFX_ARG_GLOBAL_SYNC		21
#define GFX_ARG_CRUMBLED_LIKE		22
#define GFX_ARG_DIGGABLE_LIKE		23
#define GFX_ARG_BORDER_SIZE		24
#define GFX_ARG_STEP_OFFSET		25
#define GFX_ARG_STEP_DELAY		26
#define GFX_ARG_DIRECTION		27
#define GFX_ARG_POSITION		28
#define GFX_ARG_DRAW_XOFFSET		29
#define GFX_ARG_DRAW_YOFFSET		30
#define GFX_ARG_DRAW_MASKED		31
#define GFX_ARG_ANIM_DELAY_FIXED	32
#define GFX_ARG_ANIM_DELAY_RANDOM	33
#define GFX_ARG_POST_DELAY_FIXED	34
#define GFX_ARG_POST_DELAY_RANDOM	35
#define GFX_ARG_NAME			36
#define GFX_ARG_SCALE_UP_FACTOR		37
#define GFX_ARG_CLONE_FROM		38

#define NUM_GFX_ARGS			39


/* values for sound configuration suffixes */
#define SND_ARG_MODE_LOOP		0
#define SND_ARG_VOLUME			1
#define SND_ARG_PRIORITY		2

#define NUM_SND_ARGS			3


/* values for music configuration suffixes */
#define MUS_ARG_MODE_LOOP		0

#define NUM_MUS_ARGS			1


/* values for font configuration */
#define FONT_INITIAL_1			0
#define FONT_INITIAL_2			1
#define FONT_INITIAL_3			2
#define FONT_INITIAL_4			3
#define FONT_TITLE_1			4
#define FONT_TITLE_2			5
#define FONT_MENU_1			6
#define FONT_MENU_2			7
#define FONT_TEXT_1_ACTIVE		8
#define FONT_TEXT_2_ACTIVE		9
#define FONT_TEXT_3_ACTIVE		10
#define FONT_TEXT_4_ACTIVE		11
#define FONT_TEXT_1			12
#define FONT_TEXT_2			13
#define FONT_TEXT_3			14
#define FONT_TEXT_4			15
#define FONT_ENVELOPE_1			16
#define FONT_ENVELOPE_2			17
#define FONT_ENVELOPE_3			18
#define FONT_ENVELOPE_4			19
#define FONT_INPUT_1_ACTIVE		20
#define FONT_INPUT_2_ACTIVE		21
#define FONT_INPUT_1			22
#define FONT_INPUT_2			23
#define FONT_OPTION_OFF			24
#define FONT_OPTION_ON			25
#define FONT_VALUE_1			26
#define FONT_VALUE_2			27
#define FONT_VALUE_OLD			28
#define FONT_LEVEL_NUMBER_ACTIVE	29
#define FONT_LEVEL_NUMBER		30
#define FONT_TAPE_RECORDER		31
#define FONT_GAME_INFO			32

#define NUM_FONTS			33
#define NUM_INITIAL_FONTS		4

/* values for game_status (must match special image configuration suffixes) */
#define GAME_MODE_DEFAULT		0
#define GAME_MODE_TITLE			1
#define GAME_MODE_MAIN			2
#define GAME_MODE_LEVELS		3
#define GAME_MODE_SCORES		4
#define GAME_MODE_EDITOR		5
#define GAME_MODE_INFO			6
#define GAME_MODE_SETUP			7
#define GAME_MODE_PLAYING		8
#define GAME_MODE_PSEUDO_DOOR		9
#define GAME_MODE_PSEUDO_PREVIEW	10
#define GAME_MODE_PSEUDO_CRUMBLED	11

/* there are no special config file suffixes for these modes */
#define GAME_MODE_PSEUDO_TYPENAME	12
#define GAME_MODE_QUIT			13

/* special definitions currently only used for custom artwork configuration */
#define MUSIC_PREFIX_BACKGROUND		0
#define NUM_MUSIC_PREFIXES		1
#define MAX_LEVELS			1000

/* definitions for demo animation lists */
#define HELPANIM_LIST_NEXT		-1
#define HELPANIM_LIST_END		-999


/* program information and versioning definitions */
#define PROGRAM_VERSION_MAJOR		3
#define PROGRAM_VERSION_MINOR		2
#define PROGRAM_VERSION_PATCH		1
#define PROGRAM_VERSION_BUILD		0

#define PROGRAM_TITLE_STRING		"Rocks'n'Diamonds"
#define PROGRAM_AUTHOR_STRING		"Holger Schemel"
#define PROGRAM_COPYRIGHT_STRING       "Copyright �1995-2006 by Holger Schemel"

#define ICON_TITLE_STRING		PROGRAM_TITLE_STRING
#define COOKIE_PREFIX			"ROCKSNDIAMONDS"
#define FILENAME_PREFIX			"Rocks"

#if defined(PLATFORM_UNIX)
#define USERDATA_DIRECTORY		".rocksndiamonds"
#elif defined(PLATFORM_WIN32)
#define USERDATA_DIRECTORY		PROGRAM_TITLE_STRING
#else
#define USERDATA_DIRECTORY		"userdata"
#endif

#define X11_ICON_FILENAME		"rocks_icon.xbm"
#define X11_ICONMASK_FILENAME		"rocks_iconmask.xbm"
#define MSDOS_POINTER_FILENAME		"mouse.pcx"

/* file version numbers for resource files (levels, tapes, score, setup, etc.)
** currently supported/known file version numbers:
**	1.0 (old)
**	1.2 (still in use)
**	1.4 (still in use)
**	2.0 (actual)
*/
#define FILE_VERSION_1_0		VERSION_IDENT(1,0,0,0)
#define FILE_VERSION_1_2		VERSION_IDENT(1,2,0,0)
#define FILE_VERSION_1_4		VERSION_IDENT(1,4,0,0)
#define FILE_VERSION_2_0		VERSION_IDENT(2,0,0,0)
#define FILE_VERSION_3_0		VERSION_IDENT(3,0,0,0)

/* file version does not change for every program version, but is changed
   when new features are introduced that are incompatible with older file
   versions, so that they can be treated accordingly */
#define FILE_VERSION_ACTUAL		FILE_VERSION_3_0

#define GAME_VERSION_1_0		FILE_VERSION_1_0
#define GAME_VERSION_1_2		FILE_VERSION_1_2
#define GAME_VERSION_1_4		FILE_VERSION_1_4
#define GAME_VERSION_2_0		FILE_VERSION_2_0
#define GAME_VERSION_3_0		FILE_VERSION_3_0

#define GAME_VERSION_ACTUAL		VERSION_IDENT(PROGRAM_VERSION_MAJOR, \
						      PROGRAM_VERSION_MINOR, \
						      PROGRAM_VERSION_PATCH, \
						      PROGRAM_VERSION_BUILD)

/* values for game_emulation */
#define EMU_NONE			0
#define EMU_BOULDERDASH			1
#define EMU_SOKOBAN			2
#define EMU_SUPAPLEX			3

/* values for level file type identifier */
#define LEVEL_FILE_TYPE_UNKNOWN		0
#define LEVEL_FILE_TYPE_RND		1
#define LEVEL_FILE_TYPE_BD		2
#define LEVEL_FILE_TYPE_EM		3
#define LEVEL_FILE_TYPE_SP		4
#define LEVEL_FILE_TYPE_DX		5
#define LEVEL_FILE_TYPE_SB		6
#define LEVEL_FILE_TYPE_DC		7

#define NUM_LEVEL_FILE_TYPES		8

/* values for game engine type identifier */
#define GAME_ENGINE_TYPE_UNKNOWN	LEVEL_FILE_TYPE_UNKNOWN
#define GAME_ENGINE_TYPE_RND		LEVEL_FILE_TYPE_RND
#define GAME_ENGINE_TYPE_EM		LEVEL_FILE_TYPE_EM

#define NUM_ENGINE_TYPES		3


struct MenuInfo
{
  int draw_xoffset[NUM_SPECIAL_GFX_ARGS];
  int draw_yoffset[NUM_SPECIAL_GFX_ARGS];

  int scrollbar_xoffset;

  int list_size[NUM_SPECIAL_GFX_ARGS];

  int sound[NUM_SPECIAL_GFX_ARGS];
  int music[NUM_SPECIAL_GFX_ARGS];
};

struct DoorInfo
{
  int width;
  int height;
  int step_offset;
  int step_delay;
  int anim_mode;
};

struct HiScore
{
  char Name[MAX_PLAYER_NAME_LEN + 1];
  int Score;
};

struct Content
{
  int e[3][3];
};

struct EnvelopeInfo
{
  int xsize;
  int ysize;

  char text[MAX_ENVELOPE_TEXT_LEN + 1];
};

struct LevelSetInfo
{
  int music[MAX_LEVELS];
};

struct LevelFileInfo
{
  int nr;
  int type;
  boolean packed;
  char *basename;
  char *filename;
};

struct DateInfo
{
  int year;
  int month;
  int day;
};

struct LevelInfo
{
  struct LevelFileInfo file_info;

  int game_engine_type;

  /* level stored in native format for the alternative native game engines */
  struct LevelInfo_EM *native_em_level;

  int file_version;	/* file format version the level is stored with    */
  int game_version;	/* game release version the level was created with */

  struct DateInfo creation_date;

  boolean encoding_16bit_field;		/* level contains 16-bit elements  */
  boolean encoding_16bit_yamyam;	/* yamyam contains 16-bit elements */
  boolean encoding_16bit_amoeba;	/* amoeba contains 16-bit elements */

  int fieldx, fieldy;

  int time;				/* available time (seconds) */
  int gems_needed;

  char name[MAX_LEVEL_NAME_LEN + 1];
  char author[MAX_LEVEL_AUTHOR_LEN + 1];

  struct EnvelopeInfo envelope[NUM_ENVELOPES];

  int score[LEVEL_SCORE_ELEMENTS];

  struct Content yamyam_content[MAX_ELEMENT_CONTENTS];
  int num_yamyam_contents;

  int amoeba_speed;
  int amoeba_content;

  int game_of_life[4];
  int biomaze[4];

  int time_magic_wall;
  int time_wheel;
  int time_light;
  int time_timegate;

  int shield_normal_time;
  int shield_deadly_time;

  int extra_time;
  int time_orb_time;

  int extra_time_score;

  int start_element[MAX_PLAYERS];
  boolean use_start_element[MAX_PLAYERS];

  int artwork_element[MAX_PLAYERS];
  boolean use_artwork_element[MAX_PLAYERS];

  int explosion_element[MAX_PLAYERS];
  boolean use_explosion_element[MAX_PLAYERS];

#if 1
  /* values for the new EMC elements */
  int android_move_time;
  int android_clone_time;
  boolean ball_random;
  boolean ball_state_initial;
  int ball_time;
  int lenses_score;
  int magnify_score;
  int slurp_score;
  int lenses_time;
  int magnify_time;
  int wind_direction_initial;

  struct Content ball_content[MAX_ELEMENT_CONTENTS];
  int num_ball_contents;

#if 0
  boolean android_array[16];
#endif
  int num_android_clone_elements;
  int android_clone_element[MAX_ANDROID_ELEMENTS];
#endif

  int can_move_into_acid_bits;	/* bitfield to store property for elements */
  int dont_collide_with_bits;	/* bitfield to store property for elements */

  int initial_player_stepsize[MAX_PLAYERS];	/* initial player speed */
  boolean initial_player_gravity[MAX_PLAYERS];

  boolean em_slippery_gems;	/* EM style "gems slip from wall" behaviour */
  boolean use_spring_bug;	/* for compatibility with old levels */
  boolean use_time_orb_bug;	/* for compatibility with old levels */
  boolean instant_relocation;	/* no visual delay when relocating player */
  boolean can_pass_to_walkable;	/* player can pass to empty or walkable tile */
  boolean grow_into_diggable;	/* amoeba can grow into anything diggable */

  boolean continuous_snapping;	/* repeated snapping without releasing key */
  boolean block_snap_field;	/* snapping blocks field to show animation */
  boolean block_last_field;	/* player blocks previous field while moving */
  boolean sp_block_last_field;	/* player blocks previous field while moving */

  /* ('int' instead of 'boolean' because used as selectbox value in editor) */
  int use_step_counter;		/* count steps instead of seconds for level */

  short field[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

  boolean use_custom_template;	/* use custom properties from template file */

  boolean no_valid_file;	/* set when level file missing or invalid */

  boolean changed;		/* set when level was changed in the editor */
};

struct GlobalInfo
{
  char *autoplay_leveldir;
  int autoplay_level[MAX_TAPES_PER_SET];
  boolean autoplay_all;

  char *convert_leveldir;
  int convert_level_nr;

  int num_toons;

  float frames_per_second;
  boolean fps_slowdown;
  int fps_slowdown_factor;
};

struct ElementChangeInfo
{
  boolean can_change;		/* use or ignore this change info */

  boolean has_event[NUM_CHANGE_EVENTS];		/* change events */

  int trigger_player;		/* player triggering change */
  int trigger_side;		/* side triggering change */
  int trigger_page;		/* page triggering change */

  int target_element;		/* target element after change */

  int delay_fixed;		/* added frame delay before changed (fixed) */
  int delay_random;		/* added frame delay before changed (random) */
  int delay_frames;		/* either 1 (frames) or 50 (seconds; 50 fps) */

  int trigger_element;		/* element triggering change */

  struct Content target_content;/* elements for extended change target */
  boolean use_target_content;	/* use extended change target */
  boolean only_if_complete;	/* only use complete target content */
  boolean use_random_replace;	/* use random value for replacing elements */
  int random_percentage;	/* random value for replacing elements */
  int replace_when;		/* type of elements that can be replaced */

  boolean explode;		/* explode instead of change */

  boolean has_action;		/* execute action on specified condition */
  int action_type;		/* type of action */
  int action_mode;		/* mode of action */
  int action_arg;		/* parameter of action */

  /* ---------- internal values used at runtime when playing ---------- */

  /* functions that are called before, while and after the change of an
     element -- currently only used for non-custom elements */
  void (*pre_change_function)(int x, int y);
  void (*change_function)(int x, int y);
  void (*post_change_function)(int x, int y);

  short actual_trigger_element;	/* element that actually triggered change */
  int actual_trigger_side;	/* element side that triggered the change */
  int actual_trigger_player;	/* player which actually triggered change */
  int actual_trigger_ce_value;	/* CE value of element that triggered change */
  int actual_trigger_ce_score;	/* CE score of element that triggered change */

  boolean can_change_or_has_action;	/* can_change | has_action */

  /* ---------- internal values used in level editor ---------- */

  int direct_action;		/* change triggered by actions on element */
  int other_action;		/* change triggered by other element actions */
};

struct ElementGroupInfo
{
  int num_elements;			/* number of elements in this group */
  int element[MAX_ELEMENTS_IN_GROUP];	/* list of elements in this group */

  int choice_mode;		/* how to choose element from group */

  /* ---------- internal values used at runtime when playing ---------- */

  /* the following is the same as above, but with recursively resolved group
     elements (group elements may also contain further group elements!) */
  int num_elements_resolved;
  short element_resolved[NUM_FILE_ELEMENTS];

  int choice_pos;		/* current element choice position */
};

struct ElementNameInfo
{
  /* ---------- token and description strings ---------- */

  char *token_name;		/* element token used in config files */
  char *class_name;		/* element class used in config files */
  char *editor_description;	/* pre-defined description for level editor */
};

struct ElementInfo
{
  /* ---------- token and description strings ---------- */

  char *token_name;		/* element token used in config files */
  char *class_name;		/* element class used in config files */
  char *editor_description;	/* pre-defined description for level editor */
  char *custom_description;	/* alternative description from config file */
  char description[MAX_ELEMENT_NAME_LEN + 1];	/* for custom/group elements */

  /* ---------- graphic and sound definitions ---------- */

  int graphic[NUM_ACTIONS];	/* default graphics for several actions */
  int direction_graphic[NUM_ACTIONS][NUM_DIRECTIONS_FULL];
				/* special graphics for left/right/up/down */

  int crumbled[NUM_ACTIONS];	/* crumbled graphics for several actions */
  int direction_crumbled[NUM_ACTIONS][NUM_DIRECTIONS_FULL];
				/* crumbled graphics for left/right/up/down */

  int special_graphic[NUM_SPECIAL_GFX_ARGS];
  				/* special graphics for certain screens */

  int sound[NUM_ACTIONS];	/* default sounds for several actions */

  /* ---------- special element property values ---------- */

  unsigned long properties[NUM_EP_BITFIELDS];	/* element base properties */

  boolean use_gfx_element;	/* use custom graphic element */
  int gfx_element;		/* optional custom graphic element */

  int access_direction;		/* accessible from which direction */

  int collect_score_initial;	/* initial score value for collecting */
  int collect_count_initial;	/* initial count value for collecting */

  int ce_value_fixed_initial;	/* initial value for custom variable (fix) */
  int ce_value_random_initial;	/* initial value for custom variable (rnd) */
  boolean use_last_ce_value;	/* use value from element before change */

  int push_delay_fixed;		/* constant delay before pushing */
  int push_delay_random;	/* additional random delay before pushing */
  int drop_delay_fixed;		/* constant delay after dropping */
  int drop_delay_random;	/* additional random delay after dropping */
  int move_delay_fixed;		/* constant delay after moving */
  int move_delay_random;	/* additional random delay after moving */

  int move_pattern;		/* direction movable element moves to */
  int move_direction_initial;	/* initial direction element moves to */
  int move_stepsize;		/* step size element moves with */

  int move_enter_element;	/* element that can be entered (and removed) */
  int move_leave_element;	/* element that can be left behind */
  int move_leave_type;		/* change (limited) or leave (unlimited) */

  int slippery_type;		/* how/where other elements slip away */

  struct Content content;	/* new elements after explosion */

  int explosion_type;		/* type of explosion, like 3x3, 3+3 or 1x1 */
  int explosion_delay;		/* duration of explosion of this element */
  int ignition_delay;		/* delay for explosion by other explosion */

  struct ElementChangeInfo *change_page; /* actual list of change pages */
  struct ElementChangeInfo *change;	 /* pointer to current change page */

  int num_change_pages;		/* actual number of change pages */
  int current_change_page;	/* currently edited change page */

  struct ElementGroupInfo *group;	/* pointer to element group info */

  /* ---------- internal values used at runtime when playing ---------- */

  boolean has_change_event[NUM_CHANGE_EVENTS];

  int event_page_nr[NUM_CHANGE_EVENTS]; /* page number for each event */
  struct ElementChangeInfo *event_page[NUM_CHANGE_EVENTS]; /* page for event */

  boolean in_group[NUM_GROUP_ELEMENTS];

  int collect_score;		/* runtime score value for collecting */

  /* ---------- internal values used in level editor ---------- */

  int access_type;		/* walkable or passable */
  int access_layer;		/* accessible over/inside/under */
  int access_protected;		/* protection against deadly elements */
  int walk_to_action;		/* diggable/collectible/pushable */
  int smash_targets;		/* can smash player/enemies/everything */
  int deadliness;		/* deadly when running/colliding/touching */

  boolean can_explode_by_fire;	/* element explodes by fire */
  boolean can_explode_smashed;	/* element explodes when smashed */
  boolean can_explode_impact;	/* element explodes on impact */

  boolean modified_settings;	/* set for all modified custom elements */
};

struct FontInfo
{
  char *token_name;		/* font token used in config files */

  int graphic;			/* default graphic for this font */
  int special_graphic[NUM_SPECIAL_GFX_ARGS];
  				/* special graphics for certain screens */
  int special_bitmap_id[NUM_SPECIAL_GFX_ARGS];
  				/* internal bitmap ID for special graphics */
};

struct GraphicInfo
{
  Bitmap *bitmap;
  int src_image_width;		/* scaled bitmap size, but w/o small images */
  int src_image_height;		/* scaled bitmap size, but w/o small images */

  int src_x, src_y;		/* start position of animation frames */
  int width, height;		/* width/height of each animation frame */
  int offset_x, offset_y;	/* x/y offset to next animation frame */
  int offset2_x, offset2_y;	/* x/y offset to second movement tile */
  boolean double_movement;	/* animation has second movement tile */
  int swap_double_tiles;	/* explicitely force or forbid tile swapping */
  int anim_frames;
  int anim_frames_per_line;
  int anim_start_frame;
  int anim_delay;		/* important: delay of 1 means "no delay"! */
  int anim_mode;
  boolean anim_global_sync;
  int crumbled_like;		/* element for cloning crumble graphics */
  int diggable_like;		/* element for cloning digging graphics */
  int border_size;		/* border size for "crumbled" graphics */
  int scale_up_factor;		/* optional factor for scaling image up */
  int clone_from;		/* graphic for cloning *all* settings */

  int anim_delay_fixed;		/* optional delay values for bored and   */
  int anim_delay_random;	/* sleeping player animations (animation */
  int post_delay_fixed;		/* intervall and following pause before  */
  int post_delay_random;	/* next intervall (bored animation only) */

  int step_offset;		/* optional step offset of toon animations */
  int step_delay;		/* optional step delay of toon animations */

  int draw_xoffset;		/* optional offset for drawing font chars */
  int draw_yoffset;		/* optional offset for drawing font chars */

  int draw_masked;		/* optional setting for drawing envelope gfx */

#if defined(TARGET_X11_NATIVE_PERFORMANCE_WORKAROUND)
  Pixmap clip_mask;		/* single-graphic-only clip mask for X11 */
  GC clip_gc;			/* single-graphic-only clip gc for X11 */
#endif
};

struct SoundInfo
{
  boolean loop;
  int volume;
  int priority;
};

struct MusicInfo
{
  boolean loop;
};

struct MusicPrefixInfo
{
  char *prefix;
  boolean is_loop_music;
};

struct MusicFileInfo
{
  char *basename;

  char *title_header;
  char *artist_header;
  char *album_header;
  char *year_header;

  char *title;
  char *artist;
  char *album;
  char *year;

  int music;

  boolean is_sound;

  struct MusicFileInfo *next;
};

struct ElementActionInfo
{
  char *suffix;
  int value;
  boolean is_loop_sound;
};

struct ElementDirectionInfo
{
  char *suffix;
  int value;
};

struct SpecialSuffixInfo
{
  char *suffix;
  int value;
};

struct HelpAnimInfo
{
  int element;
  int action;
  int direction;

  int delay;
};


extern Bitmap		       *bitmap_db_title;
extern Bitmap		       *bitmap_db_field;
extern Bitmap		       *bitmap_db_door;
extern Pixmap			tile_clipmask[];
extern DrawBuffer	       *fieldbuffer;
extern DrawBuffer	       *drawto_field;

extern int			game_status;
extern boolean			level_editor_test_game;
extern boolean			network_playing;

#if defined(TARGET_SDL)
extern boolean			network_server;
extern SDL_Thread	       *server_thread;
#endif

extern int			key_joystick_mapping;

extern boolean			redraw[MAX_BUF_XSIZE][MAX_BUF_YSIZE];
extern int			redraw_x1, redraw_y1;

extern short			Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ChangeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ChangePage[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			CustomValue[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			Back[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean			Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean			Pushed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ChangeCount[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ChangeEvent[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			WasJustMoving[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			WasJustFalling[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			CheckCollision[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			AmoebaCnt[MAX_NUM_AMOEBA];
extern short			AmoebaCnt2[MAX_NUM_AMOEBA];
extern short			ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short			ExplodeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			RunnerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			PlayerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

#if 0
extern unsigned long		Properties[MAX_NUM_ELEMENTS][NUM_EP_BITFIELDS];
#endif

extern int			GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 			GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int			GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 			GfxDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern int			lev_fieldx, lev_fieldy;
extern int			scroll_x, scroll_y;

extern int			FX, FY;
extern int			ScrollStepSize;
extern int			ScreenMovDir, ScreenMovPos, ScreenGfxPos;
extern int			BorderElement;
extern int			GameFrameDelay;
extern int			FfwdFrameDelay;
extern int			BX1, BY1;
extern int			BX2, BY2;
extern int			SBX_Left, SBX_Right;
extern int			SBY_Upper, SBY_Lower;
extern int			ZX, ZY;
extern int			ExitX, ExitY;
extern int			AllPlayersGone;

extern int			TimeFrames, TimePlayed, TimeLeft, TapeTime;
extern boolean			SiebAktiv;
extern int			SiebCount;

extern boolean			network_player_action_received;

extern int			graphics_action_mapping[];

extern struct LevelSetInfo	levelset;
extern struct LevelInfo		level, level_template;
extern struct HiScore		highscore[];
extern struct TapeInfo		tape;
extern struct GlobalInfo	global;
extern struct MenuInfo		menu;
extern struct DoorInfo		door_1, door_2;
extern struct ElementInfo	element_info[];
extern struct ElementNameInfo	element_name_info[];
extern struct ElementActionInfo	element_action_info[];
extern struct ElementDirectionInfo element_direction_info[];
extern struct SpecialSuffixInfo special_suffix_info[];
extern struct TokenIntPtrInfo	image_config_vars[];
extern struct FontInfo		font_info[];
extern struct MusicPrefixInfo	music_prefix_info[];
extern struct GraphicInfo      *graphic_info;
extern struct SoundInfo	       *sound_info;
extern struct MusicInfo	       *music_info;
extern struct MusicFileInfo    *music_file_info;
extern struct HelpAnimInfo     *helpanim_info;
extern SetupFileHash           *helptext_info;
extern struct ConfigTypeInfo	image_config_suffix[];
extern struct ConfigTypeInfo	sound_config_suffix[];
extern struct ConfigTypeInfo	music_config_suffix[];
extern struct ConfigInfo	image_config[];
extern struct ConfigInfo	sound_config[];
extern struct ConfigInfo	music_config[];
extern struct ConfigInfo	helpanim_config[];
extern struct ConfigInfo	helptext_config[];

#endif	/* MAIN_H */
