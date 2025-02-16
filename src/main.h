// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// main.h
// ============================================================================

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

#include "game_bd/game_bd.h"
#include "game_em/game_em.h"
#include "game_sp/game_sp.h"
#include "game_mm/game_mm.h"

#include "conf_gfx.h"	// include auto-generated data structure definitions
#include "conf_snd.h"	// include auto-generated data structure definitions
#include "conf_mus.h"	// include auto-generated data structure definitions


#define IMG_UNDEFINED			(-1)
#define IMG_EMPTY			IMG_EMPTY_SPACE
#define IMG_EXPLOSION			IMG_DEFAULT_EXPLODING
#define IMG_CHAR_START			IMG_CHAR_SPACE
#define IMG_STEEL_CHAR_START		IMG_STEEL_CHAR_SPACE
#define IMG_CUSTOM_START		IMG_CUSTOM_1

#define SND_UNDEFINED			(-1)
#define MUS_UNDEFINED			(-1)

#define WIN_XSIZE_DEFAULT		672
#define WIN_YSIZE_DEFAULT		560

#define SCR_FIELDX_DEFAULT		17
#define SCR_FIELDY_DEFAULT		17

#define SXSIZE_DEFAULT			(SCR_FIELDX_DEFAULT * TILEX)
#define SYSIZE_DEFAULT			(SCR_FIELDY_DEFAULT * TILEY)

#define MAX_BUF_XSIZE			(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE			(SCR_FIELDY + 2)
#define MIN_LEV_FIELDX			3
#define MIN_LEV_FIELDY			3
#define STD_LEV_FIELDX			64
#define STD_LEV_FIELDY			32
#define MAX_LEV_FIELDX			MAX_PLAYFIELD_WIDTH
#define MAX_LEV_FIELDY			MAX_PLAYFIELD_HEIGHT

#define MIN_SCROLL_DELAY		0
#define STD_SCROLL_DELAY		3
#define MAX_SCROLL_DELAY		8

#define SCREENX(a)			((a) - scroll_x)
#define SCREENY(a)			((a) - scroll_y)
#define LEVELX(a)			((a) + scroll_x)
#define LEVELY(a)			((a) + scroll_y)

#define IN_FIELD(x, y, xsize, ysize)	((x) >= 0 && (x) < (xsize) &&	   \
					 (y) >= 0 && (y) < (ysize))
#define IN_FIELD_MINMAX(x, y, xmin, ymin, xmax, ymax)			   \
					((x) >= (xmin) && (x) <= (xmax) && \
					 (y) >= (ymin) && (y) <= (ymax))

#define IN_PIX_FIELD(x, y)		IN_FIELD(x, y, SXSIZE, SYSIZE)
#define IN_VIS_FIELD(x, y)		IN_FIELD(x, y, SCR_FIELDX, SCR_FIELDY)
#define IN_LEV_FIELD(x, y)		IN_FIELD(x, y, lev_fieldx, lev_fieldy)
#define IN_SCR_FIELD(x, y)		IN_FIELD_MINMAX(x,y, BX1,BY1, BX2,BY2)

// values for configurable properties (custom elem's only, else pre-defined)
// (never change these values, as they are stored in level files!)
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
#define EP_DONT_GET_HIT_BY		31

// values for pre-defined properties
// (from here on, values can be changed by inserting new values)
#define EP_EMPTY_SPACE			32
#define EP_PLAYER			33
#define EP_CAN_PASS_MAGIC_WALL		34
#define EP_CAN_PASS_DC_MAGIC_WALL	35
#define EP_SWITCHABLE			36
#define EP_BD_ELEMENT			37
#define EP_SP_ELEMENT			38
#define EP_SB_ELEMENT			39
#define EP_GEM				40
#define EP_FOOD_DARK_YAMYAM		41
#define EP_FOOD_PENGUIN			42
#define EP_FOOD_PIG			43
#define EP_HISTORIC_WALL		44
#define EP_HISTORIC_SOLID		45
#define EP_CLASSIC_ENEMY		46
#define EP_BELT				47
#define EP_BELT_ACTIVE			48
#define EP_BELT_SWITCH			49
#define EP_TUBE				50
#define EP_ACID_POOL			51
#define EP_KEYGATE			52
#define EP_AMOEBOID			53
#define EP_AMOEBALIVE			54
#define EP_HAS_EDITOR_CONTENT		55
#define EP_CAN_TURN_EACH_MOVE		56
#define EP_CAN_GROW			57
#define EP_ACTIVE_BOMB			58
#define EP_INACTIVE			59

// values for special configurable properties (depending on level settings)
#define EP_EM_SLIPPERY_WALL		60

// values for special graphics properties (no effect on game engine)
#define EP_GFX_CRUMBLED			61

// values for derived properties (determined from properties above)
#define EP_ACCESSIBLE_OVER		62
#define EP_ACCESSIBLE_INSIDE		63
#define EP_ACCESSIBLE_UNDER		64
#define EP_WALKABLE			65
#define EP_PASSABLE			66
#define EP_ACCESSIBLE			67
#define EP_COLLECTIBLE			68
#define EP_SNAPPABLE			69
#define EP_WALL				70
#define EP_SOLID_FOR_PUSHING		71
#define EP_DRAGONFIRE_PROOF		72
#define EP_EXPLOSION_PROOF		73
#define EP_CAN_SMASH			74
#define EP_EXPLODES_3X3_OLD		75
#define EP_CAN_EXPLODE_BY_FIRE		76
#define EP_CAN_EXPLODE_SMASHED		77
#define EP_CAN_EXPLODE_IMPACT		78
#define EP_SP_PORT			79
#define EP_CAN_EXPLODE_BY_DRAGONFIRE	80
#define EP_CAN_EXPLODE_BY_EXPLOSION	81
#define EP_COULD_MOVE_INTO_ACID		82
#define EP_MAYBE_DONT_COLLIDE_WITH	83
#define EP_CAN_BE_CLONED_BY_ANDROID	84

// values for internal purpose only (level editor)
#define EP_WALK_TO_OBJECT		85
#define EP_DEADLY			86
#define EP_EDITOR_CASCADE		87
#define EP_EDITOR_CASCADE_ACTIVE	88
#define EP_EDITOR_CASCADE_INACTIVE	89

// values for internal purpose only (game engine)
#define EP_HAS_ACTION			90
#define EP_CAN_CHANGE_OR_HAS_ACTION	91

// values for internal purpose only (other)
#define EP_OBSOLETE			92

#define NUM_ELEMENT_PROPERTIES		93

#define NUM_EP_BITFIELDS		((NUM_ELEMENT_PROPERTIES + 31) / 32)
#define EP_BITFIELD_BASE_NR		0

#define EP_BITMASK_BASE_DEFAULT		(1 << EP_CAN_MOVE_INTO_ACID)
#define EP_BITMASK_DEFAULT		0

#define PROPERTY_BIT(p)			(1u << ((p) % 32))
#define PROPERTY_VAR(e, p)		(element_info[e].properties[(p) / 32])
#define HAS_PROPERTY(e, p)		((PROPERTY_VAR(e, p) & PROPERTY_BIT(p)) != 0)
#define SET_PROPERTY(e, p, v)		((v) ?					   \
					 (PROPERTY_VAR(e,p) |=  PROPERTY_BIT(p)) : \
					 (PROPERTY_VAR(e,p) &= ~PROPERTY_BIT(p)))


// values for change events for custom elements (stored in level file)
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
#define CE_CLICKED_BY_MOUSE		44
#define CE_PRESSED_BY_MOUSE		45
#define CE_MOUSE_CLICKED_ON_X		46
#define CE_MOUSE_PRESSED_ON_X		47
#define CE_NEXT_TO_PLAYER		48
#define CE_NEXT_TO_X			49
#define CE_PLAYER_NEXT_TO_X		50

#define NUM_CHANGE_EVENTS		51

#define NUM_CE_BITFIELDS		((NUM_CHANGE_EVENTS + 31) / 32)

#define CE_HEADLINE_SPECIAL_EVENTS	250
#define CE_UNDEFINED			255

#define CE_BITMASK_DEFAULT		0

#define CH_EVENT_BITFIELD_NR(e)		(e / 32)
#define CH_EVENT_BIT(e)			(1 << ((e) % 32))

#define CH_EVENT_VAR(e, c)		(element_info[e].change->has_event[c])
#define CH_ANY_EVENT_VAR(e, c)		(element_info[e].has_change_event[c])

#define PAGE_HAS_CHANGE_EVENT(p, c)	((p)->has_event[c])
#define HAS_CHANGE_EVENT(e, c)		(IS_CUSTOM_ELEMENT(e) && CH_EVENT_VAR(e, c))
#define HAS_ANY_CHANGE_EVENT(e, c)	(IS_CUSTOM_ELEMENT(e) && CH_ANY_EVENT_VAR(e, c))

#define SET_CHANGE_EVENT(e, c, v)	(IS_CUSTOM_ELEMENT(e) ? CH_EVENT_VAR(e, c) = (v) : 0)
#define SET_ANY_CHANGE_EVENT(e, c, v)	(IS_CUSTOM_ELEMENT(e) ?	CH_ANY_EVENT_VAR(e, c) = (v) : 0)

// values for player bitmasks
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
#define PLAYER_BITS_ACTION		(1 << 5)

// values for move directions (bits 0 - 3: basic move directions)
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

// values for move stepsize
#define STEPSIZE_NOT_MOVING		0
#define STEPSIZE_VERY_SLOW		1
#define STEPSIZE_SLOW			2
#define STEPSIZE_NORMAL			4
#define STEPSIZE_FAST			8
#define STEPSIZE_VERY_FAST		16
#define STEPSIZE_EVEN_FASTER		32
#define STEPSIZE_SLOWER			50	// (symbolic value only)
#define STEPSIZE_FASTER			200	// (symbolic value only)
#define STEPSIZE_RESET			100	// (symbolic value only)

// values for change side for custom elements
#define CH_SIDE_NONE			MV_NONE
#define CH_SIDE_LEFT			MV_LEFT
#define CH_SIDE_RIGHT			MV_RIGHT
#define CH_SIDE_TOP			MV_UP
#define CH_SIDE_BOTTOM			MV_DOWN
#define CH_SIDE_LEFT_RIGHT		MV_HORIZONTAL
#define CH_SIDE_TOP_BOTTOM		MV_VERTICAL
#define CH_SIDE_ANY			MV_ANY_DIRECTION

#define CH_SIDE_FROM_BUTTON(b)		((b) == MB_LEFTBUTTON   ? CH_SIDE_LEFT :       \
					 (b) == MB_RIGHTBUTTON  ? CH_SIDE_RIGHT :      \
					 (b) == MB_MIDDLEBUTTON ? CH_SIDE_TOP_BOTTOM : \
					 CH_SIDE_NONE)

// values for change player for custom elements
#define CH_PLAYER_NONE			PLAYER_BITS_NONE
#define CH_PLAYER_1			PLAYER_BITS_1
#define CH_PLAYER_2			PLAYER_BITS_2
#define CH_PLAYER_3			PLAYER_BITS_3
#define CH_PLAYER_4			PLAYER_BITS_4
#define CH_PLAYER_ANY			PLAYER_BITS_ANY

// values for change page for custom elements
#define CH_PAGE_ANY_FILE		(0xff)
#define CH_PAGE_ANY			(0xffffffff)

// values for change power for custom elements
#define CP_WHEN_EMPTY			0
#define CP_WHEN_DIGGABLE		1
#define CP_WHEN_DESTRUCTIBLE		2
#define CP_WHEN_COLLECTIBLE		3
#define CP_WHEN_REMOVABLE		4
#define CP_WHEN_WALKABLE		5

// values for change actions for custom elements (stored in level file)
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
#define CA_SET_PLAYER_INVENTORY		18
#define CA_SET_CE_ARTWORK		19
#define CA_SET_LEVEL_RANDOM_SEED	20
#define CA_MOVE_PLAYER_NEW		21

#define CA_HEADLINE_LEVEL_ACTIONS	250
#define CA_HEADLINE_PLAYER_ACTIONS	251
#define CA_HEADLINE_CE_ACTIONS		252
#define CA_HEADLINE_ENGINE_ACTIONS	253
#define CA_UNDEFINED			255

// values for change action mode for custom elements
#define CA_MODE_UNDEFINED		0
#define CA_MODE_SET			1
#define CA_MODE_ADD			2
#define CA_MODE_SUBTRACT		3
#define CA_MODE_MULTIPLY		4
#define CA_MODE_DIVIDE			5
#define CA_MODE_MODULO			6

// values for change action parameters for custom elements
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
#define CA_ARG_PLAYER_ACTION		(CA_ARG_PLAYER + PLAYER_BITS_ACTION)
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
#define CA_ARG_ELEMENT_ACTION		(CA_ARG_ELEMENT + 7)
#define CA_ARG_ELEMENT_HEADLINE		(CA_ARG_ELEMENT + 997)
#define CA_ARG_ELEMENT_CV_TARGET	(CA_ARG_ELEMENT_TARGET)
#define CA_ARG_ELEMENT_CV_TRIGGER	(CA_ARG_ELEMENT_TRIGGER)
#define CA_ARG_ELEMENT_CV_ACTION	(CA_ARG_ELEMENT_ACTION)
#define CA_ARG_ELEMENT_CV_HEADLINE	(CA_ARG_ELEMENT_HEADLINE)
#define CA_ARG_ELEMENT_NR_TARGET	(CA_ARG_ELEMENT + 3)
#define CA_ARG_ELEMENT_NR_TRIGGER	(CA_ARG_ELEMENT + 4)
#define CA_ARG_ELEMENT_NR_ACTION	(CA_ARG_ELEMENT + 8)
#define CA_ARG_ELEMENT_NR_HEADLINE	(CA_ARG_ELEMENT + 998)
#define CA_ARG_ELEMENT_CS_TARGET	(CA_ARG_ELEMENT + 5)
#define CA_ARG_ELEMENT_CS_TRIGGER	(CA_ARG_ELEMENT + 6)
#define CA_ARG_ELEMENT_CS_ACTION	(CA_ARG_ELEMENT + 9)
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
#define CA_ARG_INVENTORY		18000
#define CA_ARG_INVENTORY_RESET		(CA_ARG_INVENTORY + 0)
#define CA_ARG_INVENTORY_RM_TARGET	(CA_ARG_INVENTORY + 1)
#define CA_ARG_INVENTORY_RM_TRIGGER	(CA_ARG_INVENTORY + 2)
#define CA_ARG_INVENTORY_RM_ACTION	(CA_ARG_INVENTORY + 3)
#define CA_ARG_INVENTORY_RM_FIRST	(CA_ARG_INVENTORY + 4)
#define CA_ARG_INVENTORY_RM_LAST	(CA_ARG_INVENTORY + 5)
#define CA_ARG_INVENTORY_RM_ALL		(CA_ARG_INVENTORY + 6)
#define CA_ARG_INVENTORY_HEADLINE	(CA_ARG_INVENTORY + 998)
#define CA_ARG_INVENTORY_RM_HEADLINE	(CA_ARG_INVENTORY + 999)
#define CA_ARG_UNDEFINED		65535

// values for custom move patterns (bits 0 - 3: basic move directions)
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

// values for custom move patterns
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

// values for initial move direction
#define MV_START_NONE			(MV_NONE)
#define MV_START_AUTOMATIC		(MV_NONE)
#define MV_START_LEFT			(MV_LEFT)
#define MV_START_RIGHT			(MV_RIGHT)
#define MV_START_UP			(MV_UP)
#define MV_START_DOWN			(MV_DOWN)
#define MV_START_RANDOM			(MV_ALL_DIRECTIONS)
#define MV_START_PREVIOUS		(MV_PREVIOUS)

// values for elements left behind by custom elements
#define LEAVE_TYPE_UNLIMITED		0
#define LEAVE_TYPE_LIMITED		1

// values for slippery property for custom elements
#define SLIPPERY_ANY_RANDOM		0
#define SLIPPERY_ANY_LEFT_RIGHT		1
#define SLIPPERY_ANY_RIGHT_LEFT		2
#define SLIPPERY_ONLY_LEFT		3
#define SLIPPERY_ONLY_RIGHT		4

// values for explosion type for custom elements
#define EXPLODES_3X3			0
#define EXPLODES_1X1			1
#define EXPLODES_CROSS			2

// macros for configurable properties
#define IS_DIGGABLE(e)			HAS_PROPERTY(e, EP_DIGGABLE)
#define IS_COLLECTIBLE_ONLY(e)		HAS_PROPERTY(e, EP_COLLECTIBLE_ONLY)
#define DONT_RUN_INTO(e)		HAS_PROPERTY(e, EP_DONT_RUN_INTO)
#define DONT_COLLIDE_WITH(e)		HAS_PROPERTY(e, EP_DONT_COLLIDE_WITH)
#define DONT_TOUCH(e)			HAS_PROPERTY(e, EP_DONT_TOUCH)
#define IS_INDESTRUCTIBLE(e)		HAS_PROPERTY(e, EP_INDESTRUCTIBLE)
#define IS_SLIPPERY(e)			HAS_PROPERTY(e, EP_SLIPPERY)
#define CAN_CHANGE(e)			HAS_PROPERTY(e, EP_CAN_CHANGE)
#define CAN_MOVE(e)			HAS_PROPERTY(e, EP_CAN_MOVE)
#define CAN_FALL(e)			HAS_PROPERTY(e, EP_CAN_FALL)
#define CAN_SMASH_PLAYER(e)		HAS_PROPERTY(e, EP_CAN_SMASH_PLAYER)
#define CAN_SMASH_ENEMIES(e)		HAS_PROPERTY(e, EP_CAN_SMASH_ENEMIES)
#define CAN_SMASH_EVERYTHING(e)		HAS_PROPERTY(e, EP_CAN_SMASH_EVERYTHING)
#define EXPLODES_BY_FIRE(e)		HAS_PROPERTY(e, EP_EXPLODES_BY_FIRE)
#define EXPLODES_SMASHED(e)		HAS_PROPERTY(e, EP_EXPLODES_SMASHED)
#define EXPLODES_IMPACT(e)		HAS_PROPERTY(e, EP_EXPLODES_IMPACT)
#define IS_WALKABLE_OVER(e)		HAS_PROPERTY(e, EP_WALKABLE_OVER)
#define IS_WALKABLE_INSIDE(e)		HAS_PROPERTY(e, EP_WALKABLE_INSIDE)
#define IS_WALKABLE_UNDER(e)		HAS_PROPERTY(e, EP_WALKABLE_UNDER)
#define IS_PASSABLE_OVER(e)		HAS_PROPERTY(e, EP_PASSABLE_OVER)
#define IS_PASSABLE_INSIDE(e)		HAS_PROPERTY(e, EP_PASSABLE_INSIDE)
#define IS_PASSABLE_UNDER(e)		HAS_PROPERTY(e, EP_PASSABLE_UNDER)
#define IS_DROPPABLE(e)			HAS_PROPERTY(e, EP_DROPPABLE)
#define EXPLODES_1X1_OLD(e)		HAS_PROPERTY(e, EP_EXPLODES_1X1_OLD)
#define IS_PUSHABLE(e)			HAS_PROPERTY(e, EP_PUSHABLE)
#define EXPLODES_CROSS_OLD(e)		HAS_PROPERTY(e, EP_EXPLODES_CROSS_OLD)
#define IS_PROTECTED(e)			HAS_PROPERTY(e, EP_PROTECTED)
#define CAN_MOVE_INTO_ACID(e)		HAS_PROPERTY(e, EP_CAN_MOVE_INTO_ACID)
#define IS_THROWABLE(e)			HAS_PROPERTY(e, EP_THROWABLE)
#define CAN_EXPLODE(e)			HAS_PROPERTY(e, EP_CAN_EXPLODE)
#define IS_GRAVITY_REACHABLE(e)		HAS_PROPERTY(e, EP_GRAVITY_REACHABLE)
#define DONT_GET_HIT_BY(e)		HAS_PROPERTY(e, EP_DONT_GET_HIT_BY)

// macros for special configurable properties
#define IS_EM_SLIPPERY_WALL(e)		HAS_PROPERTY(e, EP_EM_SLIPPERY_WALL)

// macros for special graphics properties
#define GFX_CRUMBLED(e)			HAS_PROPERTY(GFX_ELEMENT(e), EP_GFX_CRUMBLED)

// macros for pre-defined properties
#define IS_EMPTY_SPACE(e)		HAS_PROPERTY(e, EP_EMPTY_SPACE)
#define IS_PLAYER_ELEMENT(e)		HAS_PROPERTY(e, EP_PLAYER)
#define CAN_PASS_MAGIC_WALL(e)		HAS_PROPERTY(e, EP_CAN_PASS_MAGIC_WALL)
#define CAN_PASS_DC_MAGIC_WALL(e)	HAS_PROPERTY(e, EP_CAN_PASS_DC_MAGIC_WALL)
#define IS_SWITCHABLE(e)		HAS_PROPERTY(e, EP_SWITCHABLE)
#define IS_BD_ELEMENT(e)		HAS_PROPERTY(e, EP_BD_ELEMENT)
#define IS_SP_ELEMENT(e)		HAS_PROPERTY(e, EP_SP_ELEMENT)
#define IS_SB_ELEMENT(e)		HAS_PROPERTY(e, EP_SB_ELEMENT)
#define IS_GEM(e)			HAS_PROPERTY(e, EP_GEM)
#define IS_FOOD_DARK_YAMYAM(e)		HAS_PROPERTY(e, EP_FOOD_DARK_YAMYAM)
#define IS_FOOD_PENGUIN(e)		HAS_PROPERTY(e, EP_FOOD_PENGUIN)
#define IS_FOOD_PIG(e)			HAS_PROPERTY(e, EP_FOOD_PIG)
#define IS_HISTORIC_WALL(e)		HAS_PROPERTY(e, EP_HISTORIC_WALL)
#define IS_HISTORIC_SOLID(e)		HAS_PROPERTY(e, EP_HISTORIC_SOLID)
#define IS_CLASSIC_ENEMY(e)		HAS_PROPERTY(e, EP_CLASSIC_ENEMY)
#define IS_BELT(e)			HAS_PROPERTY(e, EP_BELT)
#define IS_BELT_ACTIVE(e)		HAS_PROPERTY(e, EP_BELT_ACTIVE)
#define IS_BELT_SWITCH(e)		HAS_PROPERTY(e, EP_BELT_SWITCH)
#define IS_TUBE(e)			HAS_PROPERTY(e, EP_TUBE)
#define IS_ACID_POOL(e)			HAS_PROPERTY(e, EP_ACID_POOL)
#define IS_KEYGATE(e)			HAS_PROPERTY(e, EP_KEYGATE)
#define IS_AMOEBOID(e)			HAS_PROPERTY(e, EP_AMOEBOID)
#define IS_AMOEBALIVE(e)		HAS_PROPERTY(e, EP_AMOEBALIVE)
#define HAS_EDITOR_CONTENT(e)		HAS_PROPERTY(e, EP_HAS_EDITOR_CONTENT)
#define CAN_TURN_EACH_MOVE(e)		HAS_PROPERTY(e, EP_CAN_TURN_EACH_MOVE)
#define CAN_GROW(e)			HAS_PROPERTY(e, EP_CAN_GROW)
#define IS_ACTIVE_BOMB(e)		HAS_PROPERTY(e, EP_ACTIVE_BOMB)
#define IS_INACTIVE(e)			HAS_PROPERTY(e, EP_INACTIVE)

// macros for derived properties
#define IS_ACCESSIBLE_OVER(e)		HAS_PROPERTY(e, EP_ACCESSIBLE_OVER)
#define IS_ACCESSIBLE_INSIDE(e)		HAS_PROPERTY(e, EP_ACCESSIBLE_INSIDE)
#define IS_ACCESSIBLE_UNDER(e)		HAS_PROPERTY(e, EP_ACCESSIBLE_UNDER)
#define IS_WALKABLE(e)			HAS_PROPERTY(e, EP_WALKABLE)
#define IS_PASSABLE(e)			HAS_PROPERTY(e, EP_PASSABLE)
#define IS_ACCESSIBLE(e)		HAS_PROPERTY(e, EP_ACCESSIBLE)
#define IS_COLLECTIBLE(e)		HAS_PROPERTY(e, EP_COLLECTIBLE)
#define IS_SNAPPABLE(e)			HAS_PROPERTY(e, EP_SNAPPABLE)
#define IS_WALL(e)			HAS_PROPERTY(e, EP_WALL)
#define IS_SOLID_FOR_PUSHING(e)		HAS_PROPERTY(e, EP_SOLID_FOR_PUSHING)
#define IS_DRAGONFIRE_PROOF(e)		HAS_PROPERTY(e, EP_DRAGONFIRE_PROOF)
#define IS_EXPLOSION_PROOF(e)		HAS_PROPERTY(e, EP_EXPLOSION_PROOF)
#define CAN_SMASH(e)			HAS_PROPERTY(e, EP_CAN_SMASH)
#define EXPLODES_3X3_OLD(e)		HAS_PROPERTY(e, EP_EXPLODES_3X3_OLD)
#define CAN_EXPLODE_BY_FIRE(e)		HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_FIRE)
#define CAN_EXPLODE_SMASHED(e)		HAS_PROPERTY(e, EP_CAN_EXPLODE_SMASHED)
#define CAN_EXPLODE_IMPACT(e)		HAS_PROPERTY(e, EP_CAN_EXPLODE_IMPACT)
#define IS_SP_PORT(e)			HAS_PROPERTY(e, EP_SP_PORT)
#define CAN_EXPLODE_BY_DRAGONFIRE(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_DRAGONFIRE)
#define CAN_EXPLODE_BY_EXPLOSION(e)	HAS_PROPERTY(e, EP_CAN_EXPLODE_BY_EXPLOSION)
#define COULD_MOVE_INTO_ACID(e)		HAS_PROPERTY(e, EP_COULD_MOVE_INTO_ACID)
#define MAYBE_DONT_COLLIDE_WITH(e)	HAS_PROPERTY(e, EP_MAYBE_DONT_COLLIDE_WITH)
#define CAN_BE_CLONED_BY_ANDROID(e)	HAS_PROPERTY(e, EP_CAN_BE_CLONED_BY_ANDROID)

#define IS_EDITOR_CASCADE(e)		HAS_PROPERTY(e, EP_EDITOR_CASCADE)
#define IS_EDITOR_CASCADE_ACTIVE(e)	HAS_PROPERTY(e, EP_EDITOR_CASCADE_ACTIVE)
#define IS_EDITOR_CASCADE_INACTIVE(e)	HAS_PROPERTY(e, EP_EDITOR_CASCADE_INACTIVE)

#define HAS_ACTION(e)			HAS_PROPERTY(e, EP_HAS_ACTION)
#define CAN_CHANGE_OR_HAS_ACTION(e)	HAS_PROPERTY(e, EP_CAN_CHANGE_OR_HAS_ACTION)

#define IS_OBSOLETE(e)			HAS_PROPERTY(e, EP_OBSOLETE)

#define IS_EMPTY(e)			IS_EMPTY_SPACE(e)
#define IS_EMPTY_ELEMENT(e)		IS_EMPTY_SPACE(e)

// special macros used in game engine
#define IS_FILE_ELEMENT(e)		((e) >= 0 &&				\
					 (e) <= NUM_FILE_ELEMENTS)

#define IS_DRAWABLE_ELEMENT(e)		((e) >= 0 &&				\
					 (e) <= NUM_DRAWABLE_ELEMENTS)

#define IS_RUNTIME_ELEMENT(e)		((e) >= 0 &&				\
					 (e) <= NUM_RUNTIME_ELEMENTS)

#define IS_VALID_ELEMENT(e)		((e) >= 0 &&				\
					 (e) <= MAX_NUM_ELEMENTS)

#define IS_CUSTOM_ELEMENT(e)		((e) >= EL_CUSTOM_START &&		\
					 (e) <= EL_CUSTOM_END)

#define IS_GROUP_ELEMENT(e)		((e) >= EL_GROUP_START &&		\
					 (e) <= EL_GROUP_END)

#define IS_CLIPBOARD_ELEMENT(e)		((e) >= EL_INTERNAL_CLIPBOARD_START &&	\
					 (e) <= EL_INTERNAL_CLIPBOARD_END)

#define IS_INTERNAL_ELEMENT(e)		((e) >= EL_INTERNAL_START &&		\
					 (e) <= EL_INTERNAL_END)

#define IS_MM_ELEMENT_1(e)		((e) >= EL_MM_START_1 &&		\
					 (e) <= EL_MM_END_1)
#define IS_MM_ELEMENT_2(e)		((e) >= EL_MM_START_2 &&		\
					 (e) <= EL_MM_END_2)
#define IS_MM_ELEMENT_3(e)		((e) >= EL_MM_START_3 &&		\
					 (e) <= EL_MM_END_3)
#define IS_MM_ELEMENT(e)		(IS_MM_ELEMENT_1(e) ||			\
					 IS_MM_ELEMENT_2(e) ||			\
					 IS_MM_ELEMENT_3(e))

#define IS_DF_ELEMENT_1(e)		((e) >= EL_DF_START_1 &&		\
					 (e) <= EL_DF_END_1)
#define IS_DF_ELEMENT_2(e)		((e) >= EL_DF_START_2 &&		\
					 (e) <= EL_DF_END_2)
#define IS_DF_ELEMENT(e)		(IS_DF_ELEMENT_1(e) ||			\
					 IS_DF_ELEMENT_2(e))

#define IS_MM_MCDUFFIN(e)		((e) >= EL_MM_MCDUFFIN_START &&		\
					 (e) <= EL_MM_MCDUFFIN_END)

#define IS_DF_LASER(e)			((e) >= EL_DF_LASER_START &&		\
					 (e) <= EL_DF_LASER_END)

#define IS_MM_WALL(e)			(((e) >= EL_MM_WALL_START &&		\
					  (e) <= EL_MM_WALL_END) ||		\
					 ((e) >= EL_DF_WALL_START &&		\
					  (e) <= EL_DF_WALL_END))

#define IS_DF_WALL(e)			((e) >= EL_DF_WALL_START &&		\
					 (e) <= EL_DF_WALL_END)

#define IS_MM_WALL_EDITOR(e)		((e) == EL_MM_STEEL_WALL ||		\
					 (e) == EL_MM_WOODEN_WALL ||		\
					 (e) == EL_MM_ICE_WALL ||		\
					 (e) == EL_MM_AMOEBA_WALL ||		\
					 (e) == EL_DF_STEEL_WALL ||		\
					 (e) == EL_DF_WOODEN_WALL)

#define IS_ENVELOPE(e)			((e) >= EL_ENVELOPE_1 &&		\
					 (e) <= EL_ENVELOPE_4)

#define IS_MM_ENVELOPE(e)		((e) >= EL_MM_ENVELOPE_1 &&		\
					 (e) <= EL_MM_ENVELOPE_4)

#define IS_BALLOON_ELEMENT(e)		((e) == EL_BALLOON ||			\
					 (e) == EL_BALLOON_SWITCH_LEFT ||	\
					 (e) == EL_BALLOON_SWITCH_RIGHT ||	\
					 (e) == EL_BALLOON_SWITCH_UP ||		\
					 (e) == EL_BALLOON_SWITCH_DOWN ||	\
					 (e) == EL_BALLOON_SWITCH_ANY ||	\
					 (e) == EL_BALLOON_SWITCH_NONE)

#define IS_RND_KEY(e)			((e) >= EL_KEY_1 &&			\
					 (e) <= EL_KEY_4)
#define IS_EM_KEY(e)			((e) >= EL_EM_KEY_1 &&			\
					 (e) <= EL_EM_KEY_4)
#define IS_EMC_KEY(e)			((e) >= EL_EMC_KEY_5 &&			\
					 (e) <= EL_EMC_KEY_8)
#define IS_KEY(e)			(IS_RND_KEY(e) ||			\
					 IS_EM_KEY(e) ||			\
					 IS_EMC_KEY(e))
#define RND_KEY_NR(e)			((e) - EL_KEY_1)
#define EM_KEY_NR(e)			((e) - EL_EM_KEY_1)
#define EMC_KEY_NR(e)			((e) - EL_EMC_KEY_5 + 4)
#define KEY_NR(e)			(IS_RND_KEY(e) ? RND_KEY_NR(e) :	\
					 IS_EM_KEY(e)  ? EM_KEY_NR(e)  :	\
					 IS_EMC_KEY(e) ? EMC_KEY_NR(e) : 0)

#define IS_RND_GATE(e)			((e) >= EL_GATE_1 &&			\
					 (e) <= EL_GATE_4)
#define IS_EM_GATE(e)			((e) >= EL_EM_GATE_1 &&			\
					 (e) <= EL_EM_GATE_4)
#define IS_EMC_GATE(e)			((e) >= EL_EMC_GATE_5 &&		\
					 (e) <= EL_EMC_GATE_8)
#define IS_DC_GATE(e)			((e) == EL_DC_GATE_WHITE)
#define IS_GATE(e)			(IS_RND_GATE(e) ||			\
					 IS_EM_GATE(e) ||			\
					 IS_EMC_GATE(e) ||			\
					 IS_DC_GATE(e))
#define RND_GATE_NR(e)			((e) - EL_GATE_1)
#define EM_GATE_NR(e)			((e) - EL_EM_GATE_1)
#define EMC_GATE_NR(e)			((e) - EL_EMC_GATE_5 + 4)
#define GATE_NR(e)			(IS_RND_GATE(e) ? RND_GATE_NR(e) :	\
					 IS_EM_GATE(e)  ? EM_GATE_NR(e) :	\
					 IS_EMC_GATE(e) ? EMC_GATE_NR(e) : 0)

#define IS_RND_GATE_GRAY(e)		((e) >= EL_GATE_1_GRAY &&		\
					 (e) <= EL_GATE_4_GRAY)
#define IS_RND_GATE_GRAY_ACTIVE(e)	((e) >= EL_GATE_1_GRAY_ACTIVE &&	\
					 (e) <= EL_GATE_4_GRAY_ACTIVE)
#define IS_EM_GATE_GRAY(e)		((e) >= EL_EM_GATE_1_GRAY &&		\
					 (e) <= EL_EM_GATE_4_GRAY)
#define IS_EM_GATE_GRAY_ACTIVE(e)	((e) >= EL_EM_GATE_1_GRAY_ACTIVE &&	\
					 (e) <= EL_EM_GATE_4_GRAY_ACTIVE)
#define IS_EMC_GATE_GRAY(e)		((e) >= EL_EMC_GATE_5_GRAY &&		\
					 (e) <= EL_EMC_GATE_8_GRAY)
#define IS_EMC_GATE_GRAY_ACTIVE(e)	((e) >= EL_EMC_GATE_5_GRAY_ACTIVE &&	\
					 (e) <= EL_EMC_GATE_8_GRAY_ACTIVE)
#define IS_DC_GATE_GRAY(e)		((e) == EL_DC_GATE_WHITE_GRAY)
#define IS_DC_GATE_GRAY_ACTIVE(e)	((e) == EL_DC_GATE_WHITE_GRAY_ACTIVE)

#define IS_GATE_GRAY(e)			(IS_RND_GATE_GRAY(e) ||			\
					 IS_EM_GATE_GRAY(e) ||			\
					 IS_EMC_GATE_GRAY(e) ||			\
					 IS_DC_GATE_GRAY(e))
#define IS_GATE_GRAY_ACTIVE(e)		(IS_RND_GATE_GRAY_ACTIVE(e) ||		\
					 IS_EM_GATE_GRAY_ACTIVE(e) ||		\
					 IS_EMC_GATE_GRAY_ACTIVE(e) ||		\
					 IS_DC_GATE_GRAY_ACTIVE(e))
#define RND_GATE_GRAY_NR(e)		((e) - EL_GATE_1_GRAY)
#define RND_GATE_GRAY_ACTIVE_NR(e)	((e) - EL_GATE_1_GRAY_ACTIVE)
#define EM_GATE_GRAY_NR(e)		((e) - EL_EM_GATE_1_GRAY)
#define EM_GATE_GRAY_ACTIVE_NR(e)	((e) - EL_EM_GATE_1_GRAY_ACTIVE)
#define EMC_GATE_GRAY_NR(e)		((e) - EL_EMC_GATE_5_GRAY + 4)
#define EMC_GATE_GRAY_ACTIVE_NR(e)	((e) - EL_EMC_GATE_5_GRAY_ACTIVE + 4)
#define GATE_GRAY_NR(e)			(IS_RND_GATE_GRAY(e) ? RND_GATE_GRAY_NR(e) :	\
					 IS_EM_GATE_GRAY(e)  ? EM_GATE_GRAY_NR(e)  :	\
					 IS_EMC_GATE_GRAY(e) ? EMC_GATE_GRAY_NR(e) : 0)

#define RND_ENVELOPE_NR(e)		((e) - EL_ENVELOPE_1)
#define MM_ENVELOPE_NR(e)		((e) - EL_MM_ENVELOPE_1)
#define ENVELOPE_NR(e)			(IS_ENVELOPE(e) ? RND_ENVELOPE_NR(e) :	\
					 MM_ENVELOPE_NR(e))

#define IS_ACID_POOL_OR_ACID(e)		(IS_ACID_POOL(e) || (e) == EL_ACID)

#define IS_EMC_PILLAR(e)		((e) >= EL_EMC_WALL_1 &&		\
					 (e) <= EL_EMC_WALL_3)
#define IS_SP_CHIP(e)			((e) == EL_SP_CHIP_SINGLE ||		\
					 (e) == EL_SP_CHIP_LEFT ||		\
					 (e) == EL_SP_CHIP_RIGHT ||		\
					 (e) == EL_SP_CHIP_TOP ||		\
					 (e) == EL_SP_CHIP_BOTTOM)
#define IS_SP_HARDWARE_BASE(e)		((e) == EL_SP_HARDWARE_BASE_1 ||	\
					 (e) == EL_SP_HARDWARE_BASE_2 ||	\
					 (e) == EL_SP_HARDWARE_BASE_3 ||	\
					 (e) == EL_SP_HARDWARE_BASE_4 ||	\
					 (e) == EL_SP_HARDWARE_BASE_5 ||	\
					 (e) == EL_SP_HARDWARE_BASE_6)

#define IS_DC_STEELWALL_2(e)		((e) >= EL_DC_STEELWALL_2_LEFT &&	\
					 (e) <= EL_DC_STEELWALL_2_SINGLE)

// !!! IMPROVE THIS !!!
#define IS_EM_ELEMENT(e)	(map_element_EM_to_RND_cave(map_element_RND_to_EM_cave(e)) == (e))

#define MM_WALL_BASE(e)			((e) & 0xfff0)
#define MM_WALL_BITS(e)			((e) & 0x000f)

#define GFX_ELEMENT(e)			(element_info[e].gfx_element)

// !!! CHECK THIS !!!
#if 1
#define TILE_GFX_ELEMENT(x, y)							\
				   (GfxElement[x][y] != EL_UNDEFINED &&		\
				    Tile[x][y] != EL_EXPLOSION ?		\
				    GfxElement[x][y] : Tile[x][y])
#else
#define TILE_GFX_ELEMENT(x, y)							\
			GFX_ELEMENT(GfxElement[x][y] != EL_UNDEFINED &&		\
				    Tile[x][y] != EL_EXPLOSION ?		\
				    GfxElement[x][y] : Tile[x][y])
#endif

// !!! "use sound" deactivated due to problems with level "bug machine" !!!
// (solution: add separate "use sound of element" to level file and editor)
#if 0
#define SND_ELEMENT(e)			GFX_ELEMENT(e)
#else
#define SND_ELEMENT(e)			(e)
#endif

#define GROUP_NR(e)			((e) - EL_GROUP_START)
#define IS_IN_GROUP(e, nr)		(element_info[e].in_group[nr] == TRUE)
#define IS_IN_GROUP_EL(e, ge)		(IS_IN_GROUP(e, (ge) - EL_GROUP_START))

#define IS_EQUAL_OR_IN_GROUP(e, ge)					\
	(ge == EL_ANY_ELEMENT ? TRUE :					\
	 IS_GROUP_ELEMENT(ge) ? IS_IN_GROUP(e, GROUP_NR(ge)) : (e) == (ge))

#define IS_PLAYER(x, y)			(IS_PLAYER_ELEMENT(StorePlayer[x][y]))

#define IS_FREE(x, y)			(Tile[x][y] == EL_EMPTY && !IS_PLAYER(x, y))
#define IS_FREE_OR_PLAYER(x, y)		(Tile[x][y] == EL_EMPTY)

#define IS_MOVING(x, y)			(MovPos[x][y] != 0)
#define IS_FALLING(x, y)		(MovPos[x][y] != 0 && MovDir[x][y] == MV_DOWN)
#define IS_BLOCKED(x, y)		(Tile[x][y] == EL_BLOCKED)

#define IS_MV_DIAGONAL(x)		((x) & MV_HORIZONTAL && (x) & MV_VERTICAL)

#define EL_CHANGED(e)			((e) == EL_ROCK           ? EL_EMERALD :    \
					 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
					 (e) == EL_EMERALD        ? EL_DIAMOND :    \
					 (e) == EL_EMERALD_YELLOW ? EL_DIAMOND :    \
					 (e) == EL_EMERALD_RED    ? EL_DIAMOND :    \
					 (e) == EL_EMERALD_PURPLE ? EL_DIAMOND :    \
					 EL_ROCK)
#define EL_CHANGED_BD(e)		((e) == EL_ROCK           ? EL_BD_DIAMOND : \
					 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
					 EL_BD_ROCK)
#define EL_CHANGED_DC(e)		((e) == EL_ROCK           ? EL_EMERALD :    \
					 (e) == EL_BD_ROCK        ? EL_BD_DIAMOND : \
					 (e) == EL_EMERALD        ? EL_DIAMOND :    \
					 (e) == EL_EMERALD_YELLOW ? EL_DIAMOND :    \
					 (e) == EL_EMERALD_RED    ? EL_DIAMOND :    \
					 (e) == EL_EMERALD_PURPLE ? EL_DIAMOND :    \
					 (e) == EL_PEARL          ? EL_BOMB    :    \
					 (e) == EL_CRYSTAL        ? EL_CRYSTAL :    \
					 EL_ROCK)

#define IS_BDX_PLAYER_ELEMENT(e)	((e) == EL_BDX_INBOX ||			\
					 (e) == EL_BDX_PLAYER ||		\
					 (e) == EL_BDX_PLAYER_WITH_BOMB ||	\
					 (e) == EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER || \
					 (e) == EL_BDX_PLAYER_GLUED ||		\
					 (e) == EL_BDX_PLAYER_STIRRING)

#define IS_BD_FIREFLY(e)		((e) == EL_BD_FIREFLY ||		\
					 (e) == EL_BD_FIREFLY_RIGHT ||		\
					 (e) == EL_BD_FIREFLY_UP ||		\
					 (e) == EL_BD_FIREFLY_LEFT ||		\
					 (e) == EL_BD_FIREFLY_DOWN)

#define IS_BDX_FIREFLY_1(e)		((e) == EL_BDX_FIREFLY_1 ||		\
					 (e) == EL_BDX_FIREFLY_1_RIGHT ||	\
					 (e) == EL_BDX_FIREFLY_1_UP ||		\
					 (e) == EL_BDX_FIREFLY_1_LEFT ||	\
					 (e) == EL_BDX_FIREFLY_1_DOWN)

#define IS_BDX_FIREFLY_2(e)		((e) == EL_BDX_FIREFLY_2 ||		\
					 (e) == EL_BDX_FIREFLY_2_RIGHT ||	\
					 (e) == EL_BDX_FIREFLY_2_UP ||		\
					 (e) == EL_BDX_FIREFLY_2_LEFT ||	\
					 (e) == EL_BDX_FIREFLY_2_DOWN)

#define IS_BD_BUTTERFLY(e)		((e) == EL_BD_BUTTERFLY ||		\
					 (e) == EL_BD_BUTTERFLY_RIGHT ||	\
					 (e) == EL_BD_BUTTERFLY_UP ||		\
					 (e) == EL_BD_BUTTERFLY_LEFT ||		\
					 (e) == EL_BD_BUTTERFLY_DOWN)

#define IS_BDX_BUTTERFLY_1(e)		((e) == EL_BDX_BUTTERFLY_1 ||		\
					 (e) == EL_BDX_BUTTERFLY_1_RIGHT ||	\
					 (e) == EL_BDX_BUTTERFLY_1_UP ||	\
					 (e) == EL_BDX_BUTTERFLY_1_LEFT ||	\
					 (e) == EL_BDX_BUTTERFLY_1_DOWN)

#define IS_BDX_BUTTERFLY_2(e)		((e) == EL_BDX_BUTTERFLY_2 ||		\
					 (e) == EL_BDX_BUTTERFLY_2_RIGHT ||	\
					 (e) == EL_BDX_BUTTERFLY_2_UP ||	\
					 (e) == EL_BDX_BUTTERFLY_2_LEFT ||	\
					 (e) == EL_BDX_BUTTERFLY_2_DOWN)

#define IS_BDX_STONEFLY(e)		((e) == EL_BDX_STONEFLY ||		\
					 (e) == EL_BDX_STONEFLY_RIGHT ||	\
					 (e) == EL_BDX_STONEFLY_UP ||		\
					 (e) == EL_BDX_STONEFLY_LEFT ||		\
					 (e) == EL_BDX_STONEFLY_DOWN)

#define IS_BDX_DRAGONFLY(e)		((e) == EL_BDX_DRAGONFLY ||		\
					 (e) == EL_BDX_DRAGONFLY_RIGHT ||	\
					 (e) == EL_BDX_DRAGONFLY_UP ||		\
					 (e) == EL_BDX_DRAGONFLY_LEFT ||	\
					 (e) == EL_BDX_DRAGONFLY_DOWN)

#define IS_BDX_BITER(e)			((e) == EL_BDX_BITER ||			\
					 (e) == EL_BDX_BITER_RIGHT ||		\
					 (e) == EL_BDX_BITER_UP ||		\
					 (e) == EL_BDX_BITER_LEFT ||		\
					 (e) == EL_BDX_BITER_DOWN)

#define IS_BDX_EXPANDABLE_WALL(e)	((e) == EL_BDX_EXPANDABLE_WALL_HORIZONTAL ||		\
					 (e) == EL_BDX_EXPANDABLE_WALL_VERTICAL ||		\
					 (e) == EL_BDX_EXPANDABLE_WALL_ANY)

#define IS_BDX_EXPANDABLE_STEELWALL(e)	((e) == EL_BDX_EXPANDABLE_STEELWALL_HORIZONTAL ||	\
					 (e) == EL_BDX_EXPANDABLE_STEELWALL_VERTICAL ||		\
					 (e) == EL_BDX_EXPANDABLE_STEELWALL_ANY)

#define IS_BDX_CONVEYOR_BELT(e)		((e) == EL_BDX_CONVEYOR_LEFT ||		\
					 (e) == EL_BDX_CONVEYOR_LEFT_ACTIVE ||	\
					 (e) == EL_BDX_CONVEYOR_RIGHT ||	\
					 (e) == EL_BDX_CONVEYOR_RIGHT_ACTIVE)

#define IS_BDX_CONVEYOR_BELT_SWITCH(e)	((e) == EL_BDX_CONVEYOR_SWITCH ||	\
					 (e) == EL_BDX_CONVEYOR_SWITCH_ACTIVE ||\
					 (e) == EL_BDX_CONVEYOR_DIR_SWITCH ||	\
					 (e) == EL_BDX_CONVEYOR_DIR_SWITCH_ACTIVE)

#define IS_BDX_ELEMENT(e)		((e) >= EL_BDX_START &&			\
					 (e) <= EL_BDX_END)

#define IS_BDX_RUNTIME_ELEMENT(e)	((e) >= EL_BDX_RUNTIME_START &&		\
					 (e) <= EL_BDX_RUNTIME_END)

#define IS_BDX_EFFECTS_ELEMENT(e)	((e) >= EL_BDX_EFFECTS_START &&		\
					 (e) <= EL_BDX_EFFECTS_END)

#define IS_BDX_SCANNED_ELEMENT(e)	((e) >= EL_BDX_SCANNED_START &&		\
					 (e) <= EL_BDX_SCANNED_END)

#define IS_SOKOBAN_OBJECT_OR_FIELD(e)	((e) == EL_SOKOBAN_OBJECT ||		\
					 (e) == EL_SOKOBAN_FIELD_EMPTY ||	\
					 (e) == EL_SOKOBAN_FIELD_FULL)

#define IS_DRAWABLE(e)			((e) < EL_BLOCKED)
#define IS_NOT_DRAWABLE(e)		((e) >= EL_BLOCKED)
#define TAPE_IS_EMPTY(x)		((x).length == 0)
#define TAPE_IS_STOPPED(x)		(!(x).recording && !(x).playing)

#define PLAYERINFO(x, y)		(&stored_player[StorePlayer[x][y] - EL_PLAYER_1])
#define SHIELD_ON(p)			((p)->shield_normal_time_left > 0)

#define ENEMY_PROTECTED_FIELD(x, y)	(IS_PROTECTED(Tile[x][y]) ||			\
					 IS_PROTECTED(Back[x][y]))
#define EXPLOSION_PROTECTED_FIELD(x, y)	(IS_EXPLOSION_PROOF(Tile[x][y]))
#define PLAYER_ENEMY_PROTECTED(x, y)	(SHIELD_ON(PLAYERINFO(x, y)) ||			\
					 ENEMY_PROTECTED_FIELD(x, y))
#define PLAYER_EXPLOSION_PROTECTED(x,y) (SHIELD_ON(PLAYERINFO(x, y)) ||			\
					 EXPLOSION_PROTECTED_FIELD(x, y))

#define PLAYER_SWITCHING(p,x,y)		((p)->is_switching &&				\
					 (p)->switch_x == (x) && (p)->switch_y == (y))

#define PLAYER_DROPPING(p,x,y)		((p)->is_dropping &&				\
					 (p)->drop_x == (x) && (p)->drop_y == (y))

#define PLAYER_NR_GFX(g, i)		((g) + i * (IMG_PLAYER_2 - IMG_PLAYER_1))

#define GET_PLAYER_ELEMENT(e)		((e) >= EL_PLAYER_1 && (e) <= EL_PLAYER_4 ?	\
					 (e) : EL_PLAYER_1)

#define GET_PLAYER_NR(e)		(GET_PLAYER_ELEMENT(e) - EL_PLAYER_1)

#define GET_EMPTY_ELEMENT(i)		((i) == 0 ? EL_EMPTY_SPACE :			\
					 EL_EMPTY_SPACE_1 + (i) - 1)

#define ANIM_FRAMES(g)			(graphic_info[g].anim_frames)
#define ANIM_DELAY(g)			(graphic_info[g].anim_delay)
#define ANIM_MODE(g)			(graphic_info[g].anim_mode)

#define IS_ANIM_MODE_CE(g)		(graphic_info[g].anim_mode & (ANIM_CE_VALUE |	\
								      ANIM_CE_SCORE |	\
								      ANIM_CE_DELAY))
#define IS_ANIMATED(g)			(ANIM_FRAMES(g) > 1)
#define IS_NEW_DELAY(f, g)		((f) % ANIM_DELAY(g) == 0)
#define IS_NEW_FRAME(f, g)		(IS_ANIMATED(g) && IS_NEW_DELAY(f, g))
#define IS_NEXT_FRAME(f, g)		(IS_NEW_FRAME(f, g) && (f) > 0)

#define IS_LOOP_SOUND(s)		((s) >= 0 && sound_info[s].loop)
#define IS_LOOP_MUSIC(s)		((s) <  0 || music_info[s].loop)

#define IS_SPECIAL_GFX_ARG(a)		((a) >= 0 && (a) < NUM_SPECIAL_GFX_ARGS)

#define IS_GLOBAL_ANIM_PART(a)		((a) >= 0 && (a) < NUM_GLOBAL_ANIM_PARTS)

#define EL_CASCADE_ACTIVE(e)		(IS_EDITOR_CASCADE_INACTIVE(e) ? (e) + 1 : (e))
#define EL_CASCADE_INACTIVE(e)		(IS_EDITOR_CASCADE_ACTIVE(e)   ? (e) - 1 : (e))
#define EL_CASCADE_TOGGLE(e)		(IS_EDITOR_CASCADE_INACTIVE(e) ? (e) + 1 :    \
					 IS_EDITOR_CASCADE_ACTIVE(e)   ? (e) - 1 : (e))

#define EL_NAME(e)			((e) >= 0 ? element_info[e].token_name : "(?)")
#define MV_TEXT(d)			((d) == MV_NONE  ? "MV_NONE"  :			\
					 (d) == MV_LEFT  ? "MV_LEFT"  :			\
					 (d) == MV_RIGHT ? "MV_RIGHT" :			\
					 (d) == MV_UP    ? "MV_UP"    :			\
					 (d) == MV_DOWN  ? "MV_DOWN"  : "(various)")

#define ELEMENT_ACTIVE(e)		(ActiveElement[e])
#define BUTTON_ACTIVE(b)		(ActiveButton[b])
#define FONT_ACTIVE(f)			(ActiveFont[f])

// fundamental game speed values
#define MICROLEVEL_SCROLL_DELAY			50	// delay for scrolling micro level
#define MICROLEVEL_LABEL_DELAY			250	// delay for micro level label

// boundaries of arrays etc.
#define MAX_LEVEL_NAME_LEN			32
#define MAX_LEVEL_AUTHOR_LEN			32
#define MAX_ELEMENT_NAME_LEN			32
#define MAX_TAPES_PER_SET			1024
#define MAX_SCORE_ENTRIES			100
#define MAX_NUM_TITLE_IMAGES			5
#define MAX_NUM_TITLE_MESSAGES			5

#define MAX_NUM_AMOEBA				100

#define NUM_ENVELOPES				4
#define MIN_ENVELOPE_XSIZE			1
#define MIN_ENVELOPE_YSIZE			1
#define MAX_ENVELOPE_XSIZE			30
#define MAX_ENVELOPE_YSIZE			20
#define MAX_ENVELOPE_TEXT_LEN			(MAX_ENVELOPE_XSIZE * MAX_ENVELOPE_YSIZE)
#define MIN_CHANGE_PAGES			1
#define MAX_CHANGE_PAGES			32
#define MIN_ELEMENTS_IN_GROUP			1
#define MAX_ELEMENTS_IN_GROUP			16
#define MIN_ANDROID_ELEMENTS			1
#define MAX_ANDROID_ELEMENTS			32
#define MAX_ANDROID_ELEMENTS_OLD		16	// (extended since version 4.2.0.0)

#define MAX_ISO_DATE_LEN			10
#define MAX_PLATFORM_TEXT_LEN			16
#define MAX_VERSION_TEXT_LEN			16
#define MAX_COUNTRY_CODE_LEN			2
#define MAX_COUNTRY_NAME_LEN			64

// values for elements with content
#define MIN_ELEMENT_CONTENTS			1
#define STD_ELEMENT_CONTENTS			4
#define MAX_ELEMENT_CONTENTS			8

#define MIN_MM_BALL_CONTENTS			1
#define STD_MM_BALL_CONTENTS			8
#define MAX_MM_BALL_CONTENTS			16

// values for initial player inventory
#define MIN_INITIAL_INVENTORY_SIZE		1
#define MAX_INITIAL_INVENTORY_SIZE		8

// often used screen positions
#define TILESIZE				32
#define TILEX					TILESIZE
#define TILEY					TILESIZE
#define TILEX_VAR				TILESIZE_VAR
#define TILEY_VAR				TILESIZE_VAR
#define MINI_TILESIZE				(TILESIZE / 2)
#define MINI_TILEX				MINI_TILESIZE
#define MINI_TILEY				MINI_TILESIZE
#define MICRO_TILESIZE				(TILESIZE / 8)
#define MICRO_TILEX				MICRO_TILESIZE
#define MICRO_TILEY				MICRO_TILESIZE
#define MIDPOSX					(SCR_FIELDX / 2)
#define MIDPOSY					(SCR_FIELDY / 2)
#define FXSIZE					((2 + SCR_FIELDX + 2) * TILEX_VAR)
#define FYSIZE					((2 + SCR_FIELDY + 2) * TILEY_VAR)

#define MICROLEVEL_XSIZE			((STD_LEV_FIELDX + 2) * MICRO_TILEX)
#define MICROLEVEL_YSIZE			((STD_LEV_FIELDY + 2) * MICRO_TILEY)
#define MICROLEVEL_XPOS				(SX + (SXSIZE - MICROLEVEL_XSIZE) / 2)
#define MICROLEVEL_YPOS				(SY + 12 * TILEY - MICRO_TILEY)
#define MICROLABEL1_YPOS			(MICROLEVEL_YPOS - 36)
#define MICROLABEL2_YPOS			(MICROLEVEL_YPOS + MICROLEVEL_YSIZE + 7)

// values for GfxRedraw
#define GFX_REDRAW_NONE				(0)
#define GFX_REDRAW_TILE				(1 << 0)
#define GFX_REDRAW_TILE_CRUMBLED		(1 << 1)
#define GFX_REDRAW_TILE_CRUMBLED_NEIGHBOURS	(1 << 2)
#define GFX_REDRAW_TILE_TWINKLED		(1 << 3)

// score for elements
#define SC_EMERALD				0
#define SC_DIAMOND				1
#define SC_BUG					2
#define SC_SPACESHIP				3
#define SC_YAMYAM				4
#define SC_ROBOT				5
#define SC_PACMAN				6
#define SC_NUT					7
#define SC_DYNAMITE				8
#define SC_KEY					9
#define SC_TIME_BONUS				10
#define SC_CRYSTAL				11
#define SC_PEARL				12
#define SC_SHIELD				13
#define SC_ELEM_BONUS				14
#define SC_DIAMOND_EXTRA			15

#define LEVEL_SCORE_ELEMENTS			16	// level elements with score


// "real" level file elements
#define EL_UNDEFINED				-1

#define EL_EMPTY_SPACE				0
#define EL_EMPTY				EL_EMPTY_SPACE
#define EL_SAND					1
#define EL_WALL					2
#define EL_WALL_SLIPPERY			3
#define EL_ROCK					4
#define EL_KEY_OBSOLETE				5	// obsolete; now EL_KEY_1
#define EL_EMERALD				6
#define EL_EXIT_CLOSED				7
#define EL_PLAYER_OBSOLETE			8	// obsolete; now EL_PLAYER_1
#define EL_BUG					9
#define EL_SPACESHIP				10
#define EL_YAMYAM				11
#define EL_ROBOT				12
#define EL_STEELWALL				13
#define EL_DIAMOND				14
#define EL_AMOEBA_DEAD				15
#define EL_QUICKSAND_EMPTY			16
#define EL_QUICKSAND_FULL			17
#define EL_AMOEBA_DROP				18
#define EL_BOMB					19
#define EL_MAGIC_WALL				20
#define EL_SPEED_PILL				21
#define EL_ACID					22
#define EL_AMOEBA_WET				23
#define EL_AMOEBA_DRY				24
#define EL_NUT					25
#define EL_GAME_OF_LIFE				26
#define EL_BIOMAZE				27
#define EL_DYNAMITE_ACTIVE			28
#define EL_STONEBLOCK				29
#define EL_ROBOT_WHEEL				30
#define EL_ROBOT_WHEEL_ACTIVE			31
#define EL_KEY_1				32
#define EL_KEY_2				33
#define EL_KEY_3				34
#define EL_KEY_4				35
#define EL_GATE_1				36
#define EL_GATE_2				37
#define EL_GATE_3				38
#define EL_GATE_4				39
#define EL_GATE_1_GRAY				40
#define EL_GATE_2_GRAY				41
#define EL_GATE_3_GRAY				42
#define EL_GATE_4_GRAY				43
#define EL_DYNAMITE				44
#define EL_PACMAN				45
#define EL_INVISIBLE_WALL			46
#define EL_LAMP					47
#define EL_LAMP_ACTIVE				48
#define EL_WALL_EMERALD				49
#define EL_WALL_DIAMOND				50
#define EL_AMOEBA_FULL				51
#define EL_BD_AMOEBA				52
#define EL_TIME_ORB_FULL			53
#define EL_TIME_ORB_EMPTY			54
#define EL_EXPANDABLE_WALL			55
#define EL_BD_DIAMOND				56
#define EL_EMERALD_YELLOW			57
#define EL_WALL_BD_DIAMOND			58
#define EL_WALL_EMERALD_YELLOW			59
#define EL_DARK_YAMYAM				60
#define EL_BD_MAGIC_WALL			61
#define EL_INVISIBLE_STEELWALL			62
#define EL_SOKOBAN_FIELD_PLAYER			63
#define EL_DYNABOMB_INCREASE_NUMBER		64
#define EL_DYNABOMB_INCREASE_SIZE		65
#define EL_DYNABOMB_INCREASE_POWER		66
#define EL_SOKOBAN_OBJECT			67
#define EL_SOKOBAN_FIELD_EMPTY			68
#define EL_SOKOBAN_FIELD_FULL			69
#define EL_BD_BUTTERFLY_RIGHT			70
#define EL_BD_BUTTERFLY_UP			71
#define EL_BD_BUTTERFLY_LEFT			72
#define EL_BD_BUTTERFLY_DOWN			73
#define EL_BD_FIREFLY_RIGHT			74
#define EL_BD_FIREFLY_UP			75
#define EL_BD_FIREFLY_LEFT			76
#define EL_BD_FIREFLY_DOWN			77
#define EL_BD_BUTTERFLY				78
#define EL_BD_FIREFLY				79
#define EL_PLAYER_1				80
#define EL_PLAYER_2				81
#define EL_PLAYER_3				82
#define EL_PLAYER_4				83
#define EL_BUG_RIGHT				84
#define EL_BUG_UP				85
#define EL_BUG_LEFT				86
#define EL_BUG_DOWN				87
#define EL_SPACESHIP_RIGHT			88
#define EL_SPACESHIP_UP				89
#define EL_SPACESHIP_LEFT			90
#define EL_SPACESHIP_DOWN			91
#define EL_PACMAN_RIGHT				92
#define EL_PACMAN_UP				93
#define EL_PACMAN_LEFT				94
#define EL_PACMAN_DOWN				95
#define EL_EMERALD_RED				96
#define EL_EMERALD_PURPLE			97
#define EL_WALL_EMERALD_RED			98
#define EL_WALL_EMERALD_PURPLE			99
#define EL_ACID_POOL_TOPLEFT			100
#define EL_ACID_POOL_TOPRIGHT			101
#define EL_ACID_POOL_BOTTOMLEFT			102
#define EL_ACID_POOL_BOTTOM			103
#define EL_ACID_POOL_BOTTOMRIGHT		104
#define EL_BD_WALL				105
#define EL_BD_ROCK				106
#define EL_EXIT_OPEN				107
#define EL_BLACK_ORB				108
#define EL_AMOEBA_TO_DIAMOND			109
#define EL_MOLE					110
#define EL_PENGUIN				111
#define EL_SATELLITE				112
#define EL_ARROW_LEFT				113
#define EL_ARROW_RIGHT				114
#define EL_ARROW_UP				115
#define EL_ARROW_DOWN				116
#define EL_PIG					117
#define EL_DRAGON				118

#define EL_EM_KEY_1_FILE_OBSOLETE		119	// obsolete; now EL_EM_KEY_1

// text character elements
#define EL_CHAR_START				120
#define EL_CHAR_ASCII0				(EL_CHAR_START  - 32)
#define EL_CHAR_ASCII0_START			(EL_CHAR_ASCII0 + 32)

#include "conf_chr.h"	// include auto-generated data structure definitions

#define EL_CHAR_ASCII0_END			(EL_CHAR_ASCII0 + 111)
#define EL_CHAR_END				(EL_CHAR_START  + 79)

#define EL_CHAR(c)				(EL_CHAR_ASCII0 + MAP_FONT_ASCII(c))

#define EL_EXPANDABLE_WALL_HORIZONTAL		200
#define EL_EXPANDABLE_WALL_VERTICAL		201
#define EL_EXPANDABLE_WALL_ANY			202

// EM style elements
#define EL_EM_GATE_1				203
#define EL_EM_GATE_2				204
#define EL_EM_GATE_3				205
#define EL_EM_GATE_4				206

#define EL_EM_KEY_2_FILE_OBSOLETE		207	// obsolete; now EL_EM_KEY_2
#define EL_EM_KEY_3_FILE_OBSOLETE		208	// obsolete; now EL_EM_KEY_3
#define EL_EM_KEY_4_FILE_OBSOLETE		209	// obsolete; now EL_EM_KEY_4

// SP style elements
#define EL_SP_START				210
#define EL_SP_EMPTY_SPACE			(EL_SP_START + 0)
#define EL_SP_EMPTY				EL_SP_EMPTY_SPACE
#define EL_SP_ZONK				(EL_SP_START + 1)
#define EL_SP_BASE				(EL_SP_START + 2)
#define EL_SP_MURPHY				(EL_SP_START + 3)
#define EL_SP_INFOTRON				(EL_SP_START + 4)
#define EL_SP_CHIP_SINGLE			(EL_SP_START + 5)
#define EL_SP_HARDWARE_GRAY			(EL_SP_START + 6)
#define EL_SP_EXIT_CLOSED			(EL_SP_START + 7)
#define EL_SP_DISK_ORANGE			(EL_SP_START + 8)
#define EL_SP_PORT_RIGHT			(EL_SP_START + 9)
#define EL_SP_PORT_DOWN				(EL_SP_START + 10)
#define EL_SP_PORT_LEFT				(EL_SP_START + 11)
#define EL_SP_PORT_UP				(EL_SP_START + 12)
#define EL_SP_GRAVITY_PORT_RIGHT		(EL_SP_START + 13)
#define EL_SP_GRAVITY_PORT_DOWN			(EL_SP_START + 14)
#define EL_SP_GRAVITY_PORT_LEFT			(EL_SP_START + 15)
#define EL_SP_GRAVITY_PORT_UP			(EL_SP_START + 16)
#define EL_SP_SNIKSNAK				(EL_SP_START + 17)
#define EL_SP_DISK_YELLOW			(EL_SP_START + 18)
#define EL_SP_TERMINAL				(EL_SP_START + 19)
#define EL_SP_DISK_RED				(EL_SP_START + 20)
#define EL_SP_PORT_VERTICAL			(EL_SP_START + 21)
#define EL_SP_PORT_HORIZONTAL			(EL_SP_START + 22)
#define EL_SP_PORT_ANY				(EL_SP_START + 23)
#define EL_SP_ELECTRON				(EL_SP_START + 24)
#define EL_SP_BUGGY_BASE			(EL_SP_START + 25)
#define EL_SP_CHIP_LEFT				(EL_SP_START + 26)
#define EL_SP_CHIP_RIGHT			(EL_SP_START + 27)
#define EL_SP_HARDWARE_BASE_1			(EL_SP_START + 28)
#define EL_SP_HARDWARE_GREEN			(EL_SP_START + 29)
#define EL_SP_HARDWARE_BLUE			(EL_SP_START + 30)
#define EL_SP_HARDWARE_RED			(EL_SP_START + 31)
#define EL_SP_HARDWARE_YELLOW			(EL_SP_START + 32)
#define EL_SP_HARDWARE_BASE_2			(EL_SP_START + 33)
#define EL_SP_HARDWARE_BASE_3			(EL_SP_START + 34)
#define EL_SP_HARDWARE_BASE_4			(EL_SP_START + 35)
#define EL_SP_HARDWARE_BASE_5			(EL_SP_START + 36)
#define EL_SP_HARDWARE_BASE_6			(EL_SP_START + 37)
#define EL_SP_CHIP_TOP				(EL_SP_START + 38)
#define EL_SP_CHIP_BOTTOM			(EL_SP_START + 39)
#define EL_SP_END				(EL_SP_START + 39)

// EM style elements
#define EL_EM_GATE_1_GRAY			250
#define EL_EM_GATE_2_GRAY			251
#define EL_EM_GATE_3_GRAY			252
#define EL_EM_GATE_4_GRAY			253

#define EL_EM_DYNAMITE				254
#define EL_EM_DYNAMITE_ACTIVE			255

// DC2 style elements
#define EL_PEARL				256
#define EL_CRYSTAL				257
#define EL_WALL_PEARL				258
#define EL_WALL_CRYSTAL				259
#define EL_DC_GATE_WHITE			260
#define EL_DC_GATE_WHITE_GRAY			261
#define EL_DC_KEY_WHITE				262
#define EL_SHIELD_NORMAL			263
#define EL_EXTRA_TIME				264
#define EL_SWITCHGATE_OPEN			265
#define EL_SWITCHGATE_CLOSED			266
#define EL_SWITCHGATE_SWITCH_UP			267
#define EL_SWITCHGATE_SWITCH_DOWN		268

#define EL_UNUSED_269				269
#define EL_UNUSED_270				270

#define EL_CONVEYOR_BELT_1_LEFT			271
#define EL_CONVEYOR_BELT_1_MIDDLE		272
#define EL_CONVEYOR_BELT_1_RIGHT		273
#define EL_CONVEYOR_BELT_1_SWITCH_LEFT		274
#define EL_CONVEYOR_BELT_1_SWITCH_MIDDLE	275
#define EL_CONVEYOR_BELT_1_SWITCH_RIGHT		276
#define EL_CONVEYOR_BELT_2_LEFT			277
#define EL_CONVEYOR_BELT_2_MIDDLE		278
#define EL_CONVEYOR_BELT_2_RIGHT		279
#define EL_CONVEYOR_BELT_2_SWITCH_LEFT		280
#define EL_CONVEYOR_BELT_2_SWITCH_MIDDLE	281
#define EL_CONVEYOR_BELT_2_SWITCH_RIGHT		282
#define EL_CONVEYOR_BELT_3_LEFT			283
#define EL_CONVEYOR_BELT_3_MIDDLE		284
#define EL_CONVEYOR_BELT_3_RIGHT		285
#define EL_CONVEYOR_BELT_3_SWITCH_LEFT		286
#define EL_CONVEYOR_BELT_3_SWITCH_MIDDLE	287
#define EL_CONVEYOR_BELT_3_SWITCH_RIGHT		288
#define EL_CONVEYOR_BELT_4_LEFT			289
#define EL_CONVEYOR_BELT_4_MIDDLE		290
#define EL_CONVEYOR_BELT_4_RIGHT		291
#define EL_CONVEYOR_BELT_4_SWITCH_LEFT		292
#define EL_CONVEYOR_BELT_4_SWITCH_MIDDLE	293
#define EL_CONVEYOR_BELT_4_SWITCH_RIGHT		294
#define EL_LANDMINE				295
#define EL_ENVELOPE_OBSOLETE			296	// obsolete; now EL_ENVELOPE_1
#define EL_LIGHT_SWITCH				297
#define EL_LIGHT_SWITCH_ACTIVE			298
#define EL_SIGN_EXCLAMATION			299
#define EL_SIGN_RADIOACTIVITY			300
#define EL_SIGN_STOP				301
#define EL_SIGN_WHEELCHAIR			302
#define EL_SIGN_PARKING				303
#define EL_SIGN_NO_ENTRY			304
#define EL_SIGN_UNUSED_1			305
#define EL_SIGN_GIVE_WAY			306
#define EL_SIGN_ENTRY_FORBIDDEN			307
#define EL_SIGN_EMERGENCY_EXIT			308
#define EL_SIGN_YIN_YANG			309
#define EL_SIGN_UNUSED_2			310
#define EL_MOLE_LEFT				311
#define EL_MOLE_RIGHT				312
#define EL_MOLE_UP				313
#define EL_MOLE_DOWN				314
#define EL_STEELWALL_SLIPPERY			315
#define EL_INVISIBLE_SAND			316
#define EL_DX_UNKNOWN_15			317
#define EL_DX_UNKNOWN_42			318

#define EL_UNUSED_319				319
#define EL_UNUSED_320				320

#define EL_SHIELD_DEADLY			321
#define EL_TIMEGATE_OPEN			322
#define EL_TIMEGATE_CLOSED			323
#define EL_TIMEGATE_SWITCH_ACTIVE		324
#define EL_TIMEGATE_SWITCH			325

// EMC style elements
#define EL_BALLOON				326
#define EL_BALLOON_SWITCH_LEFT			327
#define EL_BALLOON_SWITCH_RIGHT			328
#define EL_BALLOON_SWITCH_UP			329
#define EL_BALLOON_SWITCH_DOWN			330
#define EL_BALLOON_SWITCH_ANY			331

#define EL_EMC_STEELWALL_1			332
#define EL_EMC_STEELWALL_2			333
#define EL_EMC_STEELWALL_3 			334
#define EL_EMC_STEELWALL_4			335
#define EL_EMC_WALL_1				336
#define EL_EMC_WALL_2				337
#define EL_EMC_WALL_3				338
#define EL_EMC_WALL_4				339
#define EL_EMC_WALL_5				340
#define EL_EMC_WALL_6				341
#define EL_EMC_WALL_7				342
#define EL_EMC_WALL_8				343

// DX style elements
#define EL_TUBE_ANY				344
#define EL_TUBE_VERTICAL			345
#define EL_TUBE_HORIZONTAL			346
#define EL_TUBE_VERTICAL_LEFT			347
#define EL_TUBE_VERTICAL_RIGHT			348
#define EL_TUBE_HORIZONTAL_UP			349
#define EL_TUBE_HORIZONTAL_DOWN			350
#define EL_TUBE_LEFT_UP				351
#define EL_TUBE_LEFT_DOWN			352
#define EL_TUBE_RIGHT_UP			353
#define EL_TUBE_RIGHT_DOWN			354
#define EL_SPRING				355
#define EL_TRAP					356
#define EL_DX_SUPABOMB				357

#define EL_UNUSED_358				358
#define EL_UNUSED_359				359

// ---------- begin of custom elements section --------------------------------
#define EL_CUSTOM_START				360

#include "conf_cus.h"	// include auto-generated data structure definitions

#define NUM_CUSTOM_ELEMENTS			256
#define EL_CUSTOM_END				615
// ---------- end of custom elements section ----------------------------------

// EM style elements
#define EL_EM_KEY_1				616
#define EL_EM_KEY_2				617
#define EL_EM_KEY_3				618
#define EL_EM_KEY_4				619

// DC2 style elements
#define EL_ENVELOPE_1				620
#define EL_ENVELOPE_2				621
#define EL_ENVELOPE_3				622
#define EL_ENVELOPE_4				623

// ---------- begin of group elements section ---------------------------------
#define EL_GROUP_START				624

#include "conf_grp.h"	// include auto-generated data structure definitions

#define NUM_GROUP_ELEMENTS			32
#define EL_GROUP_END				655
// ---------- end of group elements section -----------------------------------

#define EL_UNKNOWN				656
#define EL_TRIGGER_ELEMENT			657
#define EL_TRIGGER_PLAYER			658

// SP style elements
#define EL_SP_GRAVITY_ON_PORT_RIGHT		659
#define EL_SP_GRAVITY_ON_PORT_DOWN		660
#define EL_SP_GRAVITY_ON_PORT_LEFT		661
#define EL_SP_GRAVITY_ON_PORT_UP		662
#define EL_SP_GRAVITY_OFF_PORT_RIGHT		663
#define EL_SP_GRAVITY_OFF_PORT_DOWN		664
#define EL_SP_GRAVITY_OFF_PORT_LEFT		665
#define EL_SP_GRAVITY_OFF_PORT_UP		666

// EMC style elements
#define EL_BALLOON_SWITCH_NONE			667
#define EL_EMC_GATE_5				668
#define EL_EMC_GATE_6				669
#define EL_EMC_GATE_7				670
#define EL_EMC_GATE_8				671
#define EL_EMC_GATE_5_GRAY			672
#define EL_EMC_GATE_6_GRAY			673
#define EL_EMC_GATE_7_GRAY			674
#define EL_EMC_GATE_8_GRAY			675
#define EL_EMC_KEY_5				676
#define EL_EMC_KEY_6				677
#define EL_EMC_KEY_7				678
#define EL_EMC_KEY_8				679
#define EL_EMC_ANDROID				680
#define EL_EMC_GRASS				681
#define EL_EMC_MAGIC_BALL			682
#define EL_EMC_MAGIC_BALL_ACTIVE		683
#define EL_EMC_MAGIC_BALL_SWITCH		684
#define EL_EMC_MAGIC_BALL_SWITCH_ACTIVE		685
#define EL_EMC_SPRING_BUMPER			686
#define EL_EMC_PLANT				687
#define EL_EMC_LENSES				688
#define EL_EMC_MAGNIFIER			689
#define EL_EMC_WALL_9				690
#define EL_EMC_WALL_10				691
#define EL_EMC_WALL_11				692
#define EL_EMC_WALL_12				693
#define EL_EMC_WALL_13				694
#define EL_EMC_WALL_14				695
#define EL_EMC_WALL_15				696
#define EL_EMC_WALL_16				697
#define EL_EMC_WALL_SLIPPERY_1			698
#define EL_EMC_WALL_SLIPPERY_2			699
#define EL_EMC_WALL_SLIPPERY_3			700
#define EL_EMC_WALL_SLIPPERY_4			701
#define EL_EMC_FAKE_GRASS			702
#define EL_EMC_FAKE_ACID			703
#define EL_EMC_DRIPPER				704

#define EL_TRIGGER_CE_VALUE			705
#define EL_TRIGGER_CE_SCORE			706
#define EL_CURRENT_CE_VALUE			707
#define EL_CURRENT_CE_SCORE			708

#define EL_YAMYAM_LEFT				709
#define EL_YAMYAM_RIGHT				710
#define EL_YAMYAM_UP				711
#define EL_YAMYAM_DOWN				712

#define EL_BD_EXPANDABLE_WALL			713

// reference elements
#define EL_PREV_CE_8				714
#define EL_PREV_CE_7				715
#define EL_PREV_CE_6				716
#define EL_PREV_CE_5				717
#define EL_PREV_CE_4				718
#define EL_PREV_CE_3				719
#define EL_PREV_CE_2				720
#define EL_PREV_CE_1				721
#define EL_SELF					722
#define EL_NEXT_CE_1				723
#define EL_NEXT_CE_2				724
#define EL_NEXT_CE_3				725
#define EL_NEXT_CE_4				726
#define EL_NEXT_CE_5				727
#define EL_NEXT_CE_6				728
#define EL_NEXT_CE_7				729
#define EL_NEXT_CE_8				730
#define EL_ANY_ELEMENT				731

// text character elements
#define EL_STEEL_CHAR_START			732
#define EL_STEEL_CHAR_ASCII0			(EL_STEEL_CHAR_START  - 32)
#define EL_STEEL_CHAR_ASCII0_START		(EL_STEEL_CHAR_ASCII0 + 32)

// (auto-generated data structure definitions included with normal chars)

#define EL_STEEL_CHAR_ASCII0_END		(EL_STEEL_CHAR_ASCII0 + 111)
#define EL_STEEL_CHAR_END			(EL_STEEL_CHAR_START  + 79)

#define EL_STEEL_CHAR(c)			(EL_STEEL_CHAR_ASCII0+MAP_FONT_ASCII(c))

// unused elements
#define EL_SPERMS				812
#define EL_BULLET				813
#define EL_HEART				814
#define EL_CROSS				815
#define EL_FRANKIE				816
#define EL_SIGN_SPERMS				817
#define EL_SIGN_BULLET				818
#define EL_SIGN_HEART				819
#define EL_SIGN_CROSS				820
#define EL_SIGN_FRANKIE				821

// DC2 style elements
#define EL_STEEL_EXIT_CLOSED			822
#define EL_STEEL_EXIT_OPEN			823

#define EL_DC_STEELWALL_1_LEFT			824
#define EL_DC_STEELWALL_1_RIGHT			825
#define EL_DC_STEELWALL_1_TOP			826
#define EL_DC_STEELWALL_1_BOTTOM		827
#define EL_DC_STEELWALL_1_HORIZONTAL		828
#define EL_DC_STEELWALL_1_VERTICAL		829
#define EL_DC_STEELWALL_1_TOPLEFT		830
#define EL_DC_STEELWALL_1_TOPRIGHT		831
#define EL_DC_STEELWALL_1_BOTTOMLEFT		832
#define EL_DC_STEELWALL_1_BOTTOMRIGHT		833
#define EL_DC_STEELWALL_1_TOPLEFT_2		834
#define EL_DC_STEELWALL_1_TOPRIGHT_2		835
#define EL_DC_STEELWALL_1_BOTTOMLEFT_2		836
#define EL_DC_STEELWALL_1_BOTTOMRIGHT_2		837

#define EL_DC_STEELWALL_2_LEFT			838
#define EL_DC_STEELWALL_2_RIGHT			839
#define EL_DC_STEELWALL_2_TOP			840
#define EL_DC_STEELWALL_2_BOTTOM		841
#define EL_DC_STEELWALL_2_HORIZONTAL		842
#define EL_DC_STEELWALL_2_VERTICAL		843
#define EL_DC_STEELWALL_2_MIDDLE		844
#define EL_DC_STEELWALL_2_SINGLE		845

#define EL_DC_SWITCHGATE_SWITCH_UP		846
#define EL_DC_SWITCHGATE_SWITCH_DOWN		847
#define EL_DC_TIMEGATE_SWITCH			848
#define EL_DC_TIMEGATE_SWITCH_ACTIVE		849

#define EL_DC_LANDMINE				850

#define EL_EXPANDABLE_STEELWALL			851
#define EL_EXPANDABLE_STEELWALL_HORIZONTAL	852
#define EL_EXPANDABLE_STEELWALL_VERTICAL	853
#define EL_EXPANDABLE_STEELWALL_ANY		854

#define EL_EM_EXIT_CLOSED			855
#define EL_EM_EXIT_OPEN				856
#define EL_EM_STEEL_EXIT_CLOSED			857
#define EL_EM_STEEL_EXIT_OPEN			858

#define EL_DC_GATE_FAKE_GRAY			859

#define EL_DC_MAGIC_WALL			860

#define EL_QUICKSAND_FAST_EMPTY			861
#define EL_QUICKSAND_FAST_FULL			862

#define EL_FROM_LEVEL_TEMPLATE			863

// MM style elements
#define EL_MM_START				864
#define EL_MM_START_1				EL_MM_START

#define EL_MM_EMPTY_SPACE			(EL_MM_START + 0)
#define EL_MM_EMPTY				EL_MM_EMPTY_SPACE
#define EL_MM_MIRROR_START			(EL_MM_START + 1)
#define EL_MM_MIRROR_1				(EL_MM_MIRROR_START + 0)
#define EL_MM_MIRROR_2				(EL_MM_MIRROR_START + 1)
#define EL_MM_MIRROR_3				(EL_MM_MIRROR_START + 2)
#define EL_MM_MIRROR_4				(EL_MM_MIRROR_START + 3)
#define EL_MM_MIRROR_5				(EL_MM_MIRROR_START + 4)
#define EL_MM_MIRROR_6				(EL_MM_MIRROR_START + 5)
#define EL_MM_MIRROR_7				(EL_MM_MIRROR_START + 6)
#define EL_MM_MIRROR_8				(EL_MM_MIRROR_START + 7)
#define EL_MM_MIRROR_9				(EL_MM_MIRROR_START + 8)
#define EL_MM_MIRROR_10				(EL_MM_MIRROR_START + 9)
#define EL_MM_MIRROR_11				(EL_MM_MIRROR_START + 10)
#define EL_MM_MIRROR_12				(EL_MM_MIRROR_START + 11)
#define EL_MM_MIRROR_13				(EL_MM_MIRROR_START + 12)
#define EL_MM_MIRROR_14				(EL_MM_MIRROR_START + 13)
#define EL_MM_MIRROR_15				(EL_MM_MIRROR_START + 14)
#define EL_MM_MIRROR_16				(EL_MM_MIRROR_START + 15)
#define EL_MM_MIRROR_END			EL_MM_MIRROR_15
#define EL_MM_STEEL_GRID_FIXED_START		(EL_MM_START + 17)
#define EL_MM_STEEL_GRID_FIXED_1		(EL_MM_STEEL_GRID_FIXED_START + 0)
#define EL_MM_STEEL_GRID_FIXED_2		(EL_MM_STEEL_GRID_FIXED_START + 1)
#define EL_MM_STEEL_GRID_FIXED_3		(EL_MM_STEEL_GRID_FIXED_START + 2)
#define EL_MM_STEEL_GRID_FIXED_4		(EL_MM_STEEL_GRID_FIXED_START + 3)
#define EL_MM_STEEL_GRID_FIXED_END		EL_MM_STEEL_GRID_FIXED_03
#define EL_MM_MCDUFFIN_START			(EL_MM_START + 21)
#define EL_MM_MCDUFFIN_RIGHT			(EL_MM_MCDUFFIN_START + 0)
#define EL_MM_MCDUFFIN_UP			(EL_MM_MCDUFFIN_START + 1)
#define EL_MM_MCDUFFIN_LEFT			(EL_MM_MCDUFFIN_START + 2)
#define EL_MM_MCDUFFIN_DOWN			(EL_MM_MCDUFFIN_START + 3)
#define EL_MM_MCDUFFIN_END			EL_MM_MCDUFFIN_DOWN
#define EL_MM_EXIT_CLOSED			(EL_MM_START + 25)
#define EL_MM_EXIT_OPENING_1			(EL_MM_START + 26)
#define EL_MM_EXIT_OPENING_2			(EL_MM_START + 27)
#define EL_MM_EXIT_OPEN				(EL_MM_START + 28)
#define EL_MM_KETTLE				(EL_MM_START + 29)
#define EL_MM_BOMB				(EL_MM_START + 30)
#define EL_MM_PRISM				(EL_MM_START + 31)
#define EL_MM_WALL_START			(EL_MM_START + 32)
#define EL_MM_WALL_EMPTY			EL_MM_WALL_START
#define EL_MM_WALL_00				EL_MM_WALL_START
#define EL_MM_STEEL_WALL_START			EL_MM_WALL_00
#define EL_MM_STEEL_WALL_1			EL_MM_STEEL_WALL_START
#define EL_MM_WALL_15				(EL_MM_START + 47)
#define EL_MM_STEEL_WALL_END			EL_MM_WALL_15
#define EL_MM_WALL_16				(EL_MM_START + 48)
#define EL_MM_WOODEN_WALL_START			EL_MM_WALL_16
#define EL_MM_WOODEN_WALL_1			EL_MM_WOODEN_WALL_START
#define EL_MM_WALL_31				(EL_MM_START + 63)
#define EL_MM_WOODEN_WALL_END			EL_MM_WALL_31
#define EL_MM_WALL_32				(EL_MM_START + 64)
#define EL_MM_ICE_WALL_START			EL_MM_WALL_32
#define EL_MM_ICE_WALL_1			EL_MM_ICE_WALL_START
#define EL_MM_WALL_47				(EL_MM_START + 79)
#define EL_MM_ICE_WALL_END			EL_MM_WALL_47
#define EL_MM_WALL_48				(EL_MM_START + 80)
#define EL_MM_AMOEBA_WALL_START			EL_MM_WALL_48
#define EL_MM_AMOEBA_WALL_1			EL_MM_AMOEBA_WALL_START
#define EL_MM_WALL_63				(EL_MM_START + 95)
#define EL_MM_AMOEBA_WALL_END			EL_MM_WALL_63
#define EL_MM_WALL_END				EL_MM_WALL_63
#define EL_MM_WOODEN_BLOCK			(EL_MM_START + 96)
#define EL_MM_GRAY_BALL				(EL_MM_START + 97)
#define EL_MM_TELEPORTER_START			(EL_MM_START + 98)
#define EL_MM_TELEPORTER_1			(EL_MM_TELEPORTER_START + 0)
#define EL_MM_TELEPORTER_2			(EL_MM_TELEPORTER_START + 1)
#define EL_MM_TELEPORTER_3			(EL_MM_TELEPORTER_START + 2)
#define EL_MM_TELEPORTER_4			(EL_MM_TELEPORTER_START + 3)
#define EL_MM_TELEPORTER_5			(EL_MM_TELEPORTER_START + 4)
#define EL_MM_TELEPORTER_6			(EL_MM_TELEPORTER_START + 5)
#define EL_MM_TELEPORTER_7			(EL_MM_TELEPORTER_START + 6)
#define EL_MM_TELEPORTER_8			(EL_MM_TELEPORTER_START + 7)
#define EL_MM_TELEPORTER_9			(EL_MM_TELEPORTER_START + 8)
#define EL_MM_TELEPORTER_10			(EL_MM_TELEPORTER_START + 9)
#define EL_MM_TELEPORTER_11			(EL_MM_TELEPORTER_START + 10)
#define EL_MM_TELEPORTER_12			(EL_MM_TELEPORTER_START + 11)
#define EL_MM_TELEPORTER_13			(EL_MM_TELEPORTER_START + 12)
#define EL_MM_TELEPORTER_14			(EL_MM_TELEPORTER_START + 13)
#define EL_MM_TELEPORTER_15			(EL_MM_TELEPORTER_START + 14)
#define EL_MM_TELEPORTER_16			(EL_MM_TELEPORTER_START + 15)
#define EL_MM_TELEPORTER_END			EL_MM_TELEPORTER_15
#define EL_MM_FUSE_ACTIVE			(EL_MM_START + 114)
#define EL_MM_PACMAN_START			(EL_MM_START + 115)
#define EL_MM_PACMAN_RIGHT			(EL_MM_PACMAN_START + 0)
#define EL_MM_PACMAN_UP				(EL_MM_PACMAN_START + 1)
#define EL_MM_PACMAN_LEFT			(EL_MM_PACMAN_START + 2)
#define EL_MM_PACMAN_DOWN			(EL_MM_PACMAN_START + 3)
#define EL_MM_PACMAN_END			EL_MM_PACMAN_DOWN
#define EL_MM_POLARIZER_START			(EL_MM_START + 119)
#define EL_MM_POLARIZER_1			(EL_MM_POLARIZER_START + 0)
#define EL_MM_POLARIZER_2			(EL_MM_POLARIZER_START + 1)
#define EL_MM_POLARIZER_3			(EL_MM_POLARIZER_START + 2)
#define EL_MM_POLARIZER_4			(EL_MM_POLARIZER_START + 3)
#define EL_MM_POLARIZER_5			(EL_MM_POLARIZER_START + 4)
#define EL_MM_POLARIZER_6			(EL_MM_POLARIZER_START + 5)
#define EL_MM_POLARIZER_7			(EL_MM_POLARIZER_START + 6)
#define EL_MM_POLARIZER_8			(EL_MM_POLARIZER_START + 7)
#define EL_MM_POLARIZER_9			(EL_MM_POLARIZER_START + 8)
#define EL_MM_POLARIZER_10			(EL_MM_POLARIZER_START + 9)
#define EL_MM_POLARIZER_11			(EL_MM_POLARIZER_START + 10)
#define EL_MM_POLARIZER_12			(EL_MM_POLARIZER_START + 11)
#define EL_MM_POLARIZER_13			(EL_MM_POLARIZER_START + 12)
#define EL_MM_POLARIZER_14			(EL_MM_POLARIZER_START + 13)
#define EL_MM_POLARIZER_15			(EL_MM_POLARIZER_START + 14)
#define EL_MM_POLARIZER_16			(EL_MM_POLARIZER_START + 15)
#define EL_MM_POLARIZER_END			EL_MM_POLARIZER_15
#define EL_MM_POLARIZER_CROSS_START		(EL_MM_START + 135)
#define EL_MM_POLARIZER_CROSS_1			(EL_MM_POLARIZER_CROSS_START + 0)
#define EL_MM_POLARIZER_CROSS_2			(EL_MM_POLARIZER_CROSS_START + 1)
#define EL_MM_POLARIZER_CROSS_3			(EL_MM_POLARIZER_CROSS_START + 2)
#define EL_MM_POLARIZER_CROSS_4			(EL_MM_POLARIZER_CROSS_START + 3)
#define EL_MM_POLARIZER_CROSS_END		EL_MM_POLARIZER_CROSS_03
#define EL_MM_MIRROR_FIXED_START		(EL_MM_START + 139)
#define EL_MM_MIRROR_FIXED_1			(EL_MM_MIRROR_FIXED_START + 0)
#define EL_MM_MIRROR_FIXED_2			(EL_MM_MIRROR_FIXED_START + 1)
#define EL_MM_MIRROR_FIXED_3			(EL_MM_MIRROR_FIXED_START + 2)
#define EL_MM_MIRROR_FIXED_4			(EL_MM_MIRROR_FIXED_START + 3)
#define EL_MM_MIRROR_FIXED_END			EL_MM_MIRROR_FIXED_03
#define EL_MM_STEEL_LOCK			(EL_MM_START + 143)
#define EL_MM_KEY				(EL_MM_START + 144)
#define EL_MM_LIGHTBULB				(EL_MM_START + 145)
#define EL_MM_LIGHTBULB_ACTIVE			(EL_MM_START + 146)
#define EL_MM_LIGHTBALL				(EL_MM_START + 147)
#define EL_MM_STEEL_BLOCK			(EL_MM_START + 148)
#define EL_MM_WOODEN_LOCK			(EL_MM_START + 149)
#define EL_MM_FUEL_FULL				(EL_MM_START + 150)
#define EL_MM_WOODEN_GRID_FIXED_START		(EL_MM_START + 151)
#define EL_MM_WOODEN_GRID_FIXED_1		(EL_MM_WOODEN_GRID_FIXED_START + 0)
#define EL_MM_WOODEN_GRID_FIXED_2		(EL_MM_WOODEN_GRID_FIXED_START + 1)
#define EL_MM_WOODEN_GRID_FIXED_3		(EL_MM_WOODEN_GRID_FIXED_START + 2)
#define EL_MM_WOODEN_GRID_FIXED_4		(EL_MM_WOODEN_GRID_FIXED_START + 3)
#define EL_MM_WOODEN_GRID_FIXED_END		EL_MM_WOODEN_GRID_FIXED_03
#define EL_MM_FUEL_EMPTY			(EL_MM_START + 155)
#define EL_MM_ENVELOPE_1			(EL_MM_START + 156)
#define EL_MM_ENVELOPE_2			(EL_MM_START + 157)
#define EL_MM_ENVELOPE_3			(EL_MM_START + 158)
#define EL_MM_ENVELOPE_4			(EL_MM_START + 159)

#define EL_MM_END_1				(EL_MM_START + 159)
#define EL_MM_START_2				(EL_MM_START + 160)

// DF style elements
#define EL_DF_START				EL_MM_START_2
#define EL_DF_START_1				EL_MM_START_2
#define EL_DF_START2				(EL_DF_START - 240)

#define EL_DF_MIRROR_START			EL_DF_START
#define EL_DF_MIRROR_1				(EL_DF_MIRROR_START + 0)
#define EL_DF_MIRROR_2				(EL_DF_MIRROR_START + 1)
#define EL_DF_MIRROR_3				(EL_DF_MIRROR_START + 2)
#define EL_DF_MIRROR_4				(EL_DF_MIRROR_START + 3)
#define EL_DF_MIRROR_5				(EL_DF_MIRROR_START + 4)
#define EL_DF_MIRROR_6				(EL_DF_MIRROR_START + 5)
#define EL_DF_MIRROR_7				(EL_DF_MIRROR_START + 6)
#define EL_DF_MIRROR_8				(EL_DF_MIRROR_START + 7)
#define EL_DF_MIRROR_9				(EL_DF_MIRROR_START + 8)
#define EL_DF_MIRROR_10				(EL_DF_MIRROR_START + 9)
#define EL_DF_MIRROR_11				(EL_DF_MIRROR_START + 10)
#define EL_DF_MIRROR_12				(EL_DF_MIRROR_START + 11)
#define EL_DF_MIRROR_13				(EL_DF_MIRROR_START + 12)
#define EL_DF_MIRROR_14				(EL_DF_MIRROR_START + 13)
#define EL_DF_MIRROR_15				(EL_DF_MIRROR_START + 14)
#define EL_DF_MIRROR_16				(EL_DF_MIRROR_START + 15)
#define EL_DF_MIRROR_END			EL_DF_MIRROR_15

#define EL_DF_WOODEN_GRID_FIXED_START		(EL_DF_START2 + 256)
#define EL_DF_WOODEN_GRID_FIXED_1		(EL_DF_WOODEN_GRID_FIXED_START + 0)
#define EL_DF_WOODEN_GRID_FIXED_2		(EL_DF_WOODEN_GRID_FIXED_START + 1)
#define EL_DF_WOODEN_GRID_FIXED_3		(EL_DF_WOODEN_GRID_FIXED_START + 2)
#define EL_DF_WOODEN_GRID_FIXED_4		(EL_DF_WOODEN_GRID_FIXED_START + 3)
#define EL_DF_WOODEN_GRID_FIXED_5		(EL_DF_WOODEN_GRID_FIXED_START + 4)
#define EL_DF_WOODEN_GRID_FIXED_6		(EL_DF_WOODEN_GRID_FIXED_START + 5)
#define EL_DF_WOODEN_GRID_FIXED_7		(EL_DF_WOODEN_GRID_FIXED_START + 6)
#define EL_DF_WOODEN_GRID_FIXED_8		(EL_DF_WOODEN_GRID_FIXED_START + 7)
#define EL_DF_WOODEN_GRID_FIXED_END		EL_DF_WOODEN_GRID_FIXED_07

#define EL_DF_STEEL_GRID_FIXED_START		(EL_DF_START2 + 264)
#define EL_DF_STEEL_GRID_FIXED_1		(EL_DF_STEEL_GRID_FIXED_START + 0)
#define EL_DF_STEEL_GRID_FIXED_2		(EL_DF_STEEL_GRID_FIXED_START + 1)
#define EL_DF_STEEL_GRID_FIXED_3		(EL_DF_STEEL_GRID_FIXED_START + 2)
#define EL_DF_STEEL_GRID_FIXED_4		(EL_DF_STEEL_GRID_FIXED_START + 3)
#define EL_DF_STEEL_GRID_FIXED_5		(EL_DF_STEEL_GRID_FIXED_START + 4)
#define EL_DF_STEEL_GRID_FIXED_6		(EL_DF_STEEL_GRID_FIXED_START + 5)
#define EL_DF_STEEL_GRID_FIXED_7		(EL_DF_STEEL_GRID_FIXED_START + 6)
#define EL_DF_STEEL_GRID_FIXED_8		(EL_DF_STEEL_GRID_FIXED_START + 7)
#define EL_DF_STEEL_GRID_FIXED_END		EL_DF_STEEL_GRID_FIXED_07

#define EL_DF_WOODEN_WALL_START			(EL_DF_START2 + 272)
#define EL_DF_WOODEN_WALL_1			(EL_DF_WOODEN_WALL_START + 0)
#define EL_DF_WOODEN_WALL_END			(EL_DF_WOODEN_WALL_START + 15)

#define EL_DF_STEEL_WALL_START			(EL_DF_START2 + 288)
#define EL_DF_STEEL_WALL_1			(EL_DF_STEEL_WALL_START + 0)
#define EL_DF_STEEL_WALL_END			(EL_DF_STEEL_WALL_START + 15)

#define EL_DF_WALL_START			EL_DF_WOODEN_WALL_START
#define EL_DF_WALL_END				EL_DF_STEEL_WALL_END

#define EL_DF_EMPTY_SPACE			(EL_DF_START2 + 304)
#define EL_DF_EMPTY				EL_DF_EMPTY_SPACE
#define EL_DF_CELL				(EL_DF_START2 + 305)
#define EL_DF_MINE				(EL_DF_START2 + 306)
#define EL_DF_REFRACTOR				(EL_DF_START2 + 307)

#define EL_DF_LASER_START			(EL_DF_START2 + 308)
#define EL_DF_LASER_RIGHT			(EL_DF_LASER_START + 0)
#define EL_DF_LASER_UP				(EL_DF_LASER_START + 1)
#define EL_DF_LASER_LEFT			(EL_DF_LASER_START + 2)
#define EL_DF_LASER_DOWN			(EL_DF_LASER_START + 3)
#define EL_DF_LASER_END				EL_DF_LASER_DOWN

#define EL_DF_RECEIVER_START			(EL_DF_START2 + 312)
#define EL_DF_RECEIVER_RIGHT			(EL_DF_RECEIVER_START + 0)
#define EL_DF_RECEIVER_UP			(EL_DF_RECEIVER_START + 1)
#define EL_DF_RECEIVER_LEFT			(EL_DF_RECEIVER_START + 2)
#define EL_DF_RECEIVER_DOWN			(EL_DF_RECEIVER_START + 3)
#define EL_DF_RECEIVER_END			EL_DF_RECEIVER_DOWN

#define EL_DF_FIBRE_OPTIC_START			(EL_DF_START2 + 316)
#define EL_DF_FIBRE_OPTIC_RED_1			(EL_DF_FIBRE_OPTIC_START + 0)
#define EL_DF_FIBRE_OPTIC_RED_2			(EL_DF_FIBRE_OPTIC_START + 1)
#define EL_DF_FIBRE_OPTIC_YELLOW_1		(EL_DF_FIBRE_OPTIC_START + 2)
#define EL_DF_FIBRE_OPTIC_YELLOW_2		(EL_DF_FIBRE_OPTIC_START + 3)
#define EL_DF_FIBRE_OPTIC_GREEN_1		(EL_DF_FIBRE_OPTIC_START + 4)
#define EL_DF_FIBRE_OPTIC_GREEN_2		(EL_DF_FIBRE_OPTIC_START + 5)
#define EL_DF_FIBRE_OPTIC_BLUE_1		(EL_DF_FIBRE_OPTIC_START + 6)
#define EL_DF_FIBRE_OPTIC_BLUE_2		(EL_DF_FIBRE_OPTIC_START + 7)
#define EL_DF_FIBRE_OPTIC_END			EL_DF_FIBRE_OPTIC_07

#define EL_DF_MIRROR_ROTATING_START		(EL_DF_START2 + 324)
#define EL_DF_MIRROR_ROTATING_1			(EL_DF_MIRROR_ROTATING_START + 0)
#define EL_DF_MIRROR_ROTATING_2			(EL_DF_MIRROR_ROTATING_START + 1)
#define EL_DF_MIRROR_ROTATING_3			(EL_DF_MIRROR_ROTATING_START + 2)
#define EL_DF_MIRROR_ROTATING_4			(EL_DF_MIRROR_ROTATING_START + 3)
#define EL_DF_MIRROR_ROTATING_5			(EL_DF_MIRROR_ROTATING_START + 4)
#define EL_DF_MIRROR_ROTATING_6			(EL_DF_MIRROR_ROTATING_START + 5)
#define EL_DF_MIRROR_ROTATING_7			(EL_DF_MIRROR_ROTATING_START + 6)
#define EL_DF_MIRROR_ROTATING_8			(EL_DF_MIRROR_ROTATING_START + 7)
#define EL_DF_MIRROR_ROTATING_9			(EL_DF_MIRROR_ROTATING_START + 8)
#define EL_DF_MIRROR_ROTATING_10		(EL_DF_MIRROR_ROTATING_START + 9)
#define EL_DF_MIRROR_ROTATING_11		(EL_DF_MIRROR_ROTATING_START + 10)
#define EL_DF_MIRROR_ROTATING_12		(EL_DF_MIRROR_ROTATING_START + 11)
#define EL_DF_MIRROR_ROTATING_13		(EL_DF_MIRROR_ROTATING_START + 12)
#define EL_DF_MIRROR_ROTATING_14		(EL_DF_MIRROR_ROTATING_START + 13)
#define EL_DF_MIRROR_ROTATING_15		(EL_DF_MIRROR_ROTATING_START + 14)
#define EL_DF_MIRROR_ROTATING_16		(EL_DF_MIRROR_ROTATING_START + 15)
#define EL_DF_MIRROR_ROTATING_END		EL_DF_MIRROR_ROTATING_15

#define EL_DF_WOODEN_GRID_ROTATING_START	(EL_DF_START2 + 340)
#define EL_DF_WOODEN_GRID_ROTATING_1		(EL_DF_WOODEN_GRID_ROTATING_START + 0)
#define EL_DF_WOODEN_GRID_ROTATING_2		(EL_DF_WOODEN_GRID_ROTATING_START + 1)
#define EL_DF_WOODEN_GRID_ROTATING_3		(EL_DF_WOODEN_GRID_ROTATING_START + 2)
#define EL_DF_WOODEN_GRID_ROTATING_4		(EL_DF_WOODEN_GRID_ROTATING_START + 3)
#define EL_DF_WOODEN_GRID_ROTATING_5		(EL_DF_WOODEN_GRID_ROTATING_START + 4)
#define EL_DF_WOODEN_GRID_ROTATING_6		(EL_DF_WOODEN_GRID_ROTATING_START + 5)
#define EL_DF_WOODEN_GRID_ROTATING_7		(EL_DF_WOODEN_GRID_ROTATING_START + 6)
#define EL_DF_WOODEN_GRID_ROTATING_8		(EL_DF_WOODEN_GRID_ROTATING_START + 7)
#define EL_DF_WOODEN_GRID_ROTATING_END		EL_DF_WOODEN_GRID_ROTATING_07

#define EL_DF_STEEL_GRID_ROTATING_START		(EL_DF_START2 + 348)
#define EL_DF_STEEL_GRID_ROTATING_1		(EL_DF_STEEL_GRID_ROTATING_START + 0)
#define EL_DF_STEEL_GRID_ROTATING_2		(EL_DF_STEEL_GRID_ROTATING_START + 1)
#define EL_DF_STEEL_GRID_ROTATING_3		(EL_DF_STEEL_GRID_ROTATING_START + 2)
#define EL_DF_STEEL_GRID_ROTATING_4		(EL_DF_STEEL_GRID_ROTATING_START + 3)
#define EL_DF_STEEL_GRID_ROTATING_5		(EL_DF_STEEL_GRID_ROTATING_START + 4)
#define EL_DF_STEEL_GRID_ROTATING_6		(EL_DF_STEEL_GRID_ROTATING_START + 5)
#define EL_DF_STEEL_GRID_ROTATING_7		(EL_DF_STEEL_GRID_ROTATING_START + 6)
#define EL_DF_STEEL_GRID_ROTATING_8		(EL_DF_STEEL_GRID_ROTATING_START + 7)
#define EL_DF_STEEL_GRID_ROTATING_END		EL_DF_STEEL_GRID_ROTATING_07

#define EL_DF_END_1				(EL_DF_START2 + 355)

// MM style elements
#define EL_MM_TELEPORTER_RED_START		(EL_DF_START2 + 356)
#define EL_MM_TELEPORTER_RED_1			(EL_MM_TELEPORTER_RED_START + 0)
#define EL_MM_TELEPORTER_RED_2			(EL_MM_TELEPORTER_RED_START + 1)
#define EL_MM_TELEPORTER_RED_3			(EL_MM_TELEPORTER_RED_START + 2)
#define EL_MM_TELEPORTER_RED_4			(EL_MM_TELEPORTER_RED_START + 3)
#define EL_MM_TELEPORTER_RED_5			(EL_MM_TELEPORTER_RED_START + 4)
#define EL_MM_TELEPORTER_RED_6			(EL_MM_TELEPORTER_RED_START + 5)
#define EL_MM_TELEPORTER_RED_7			(EL_MM_TELEPORTER_RED_START + 6)
#define EL_MM_TELEPORTER_RED_8			(EL_MM_TELEPORTER_RED_START + 7)
#define EL_MM_TELEPORTER_RED_9			(EL_MM_TELEPORTER_RED_START + 8)
#define EL_MM_TELEPORTER_RED_10			(EL_MM_TELEPORTER_RED_START + 9)
#define EL_MM_TELEPORTER_RED_11			(EL_MM_TELEPORTER_RED_START + 10)
#define EL_MM_TELEPORTER_RED_12			(EL_MM_TELEPORTER_RED_START + 11)
#define EL_MM_TELEPORTER_RED_13			(EL_MM_TELEPORTER_RED_START + 12)
#define EL_MM_TELEPORTER_RED_14			(EL_MM_TELEPORTER_RED_START + 13)
#define EL_MM_TELEPORTER_RED_15			(EL_MM_TELEPORTER_RED_START + 14)
#define EL_MM_TELEPORTER_RED_16			(EL_MM_TELEPORTER_RED_START + 15)
#define EL_MM_TELEPORTER_RED_END		EL_MM_TELEPORTER_RED_16
#define EL_MM_TELEPORTER_YELLOW_START		(EL_DF_START2 + 372)
#define EL_MM_TELEPORTER_YELLOW_1		(EL_MM_TELEPORTER_YELLOW_START + 0)
#define EL_MM_TELEPORTER_YELLOW_2		(EL_MM_TELEPORTER_YELLOW_START + 1)
#define EL_MM_TELEPORTER_YELLOW_3		(EL_MM_TELEPORTER_YELLOW_START + 2)
#define EL_MM_TELEPORTER_YELLOW_4		(EL_MM_TELEPORTER_YELLOW_START + 3)
#define EL_MM_TELEPORTER_YELLOW_5		(EL_MM_TELEPORTER_YELLOW_START + 4)
#define EL_MM_TELEPORTER_YELLOW_6		(EL_MM_TELEPORTER_YELLOW_START + 5)
#define EL_MM_TELEPORTER_YELLOW_7		(EL_MM_TELEPORTER_YELLOW_START + 6)
#define EL_MM_TELEPORTER_YELLOW_8		(EL_MM_TELEPORTER_YELLOW_START + 7)
#define EL_MM_TELEPORTER_YELLOW_9		(EL_MM_TELEPORTER_YELLOW_START + 8)
#define EL_MM_TELEPORTER_YELLOW_10		(EL_MM_TELEPORTER_YELLOW_START + 9)
#define EL_MM_TELEPORTER_YELLOW_11		(EL_MM_TELEPORTER_YELLOW_START + 10)
#define EL_MM_TELEPORTER_YELLOW_12		(EL_MM_TELEPORTER_YELLOW_START + 11)
#define EL_MM_TELEPORTER_YELLOW_13		(EL_MM_TELEPORTER_YELLOW_START + 12)
#define EL_MM_TELEPORTER_YELLOW_14		(EL_MM_TELEPORTER_YELLOW_START + 13)
#define EL_MM_TELEPORTER_YELLOW_15		(EL_MM_TELEPORTER_YELLOW_START + 14)
#define EL_MM_TELEPORTER_YELLOW_16		(EL_MM_TELEPORTER_YELLOW_START + 15)
#define EL_MM_TELEPORTER_YELLOW_END		EL_MM_TELEPORTER_YELLOW_16
#define EL_MM_TELEPORTER_GREEN_START		(EL_DF_START2 + 388)
#define EL_MM_TELEPORTER_GREEN_1		(EL_MM_TELEPORTER_GREEN_START + 0)
#define EL_MM_TELEPORTER_GREEN_2		(EL_MM_TELEPORTER_GREEN_START + 1)
#define EL_MM_TELEPORTER_GREEN_3		(EL_MM_TELEPORTER_GREEN_START + 2)
#define EL_MM_TELEPORTER_GREEN_4		(EL_MM_TELEPORTER_GREEN_START + 3)
#define EL_MM_TELEPORTER_GREEN_5		(EL_MM_TELEPORTER_GREEN_START + 4)
#define EL_MM_TELEPORTER_GREEN_6		(EL_MM_TELEPORTER_GREEN_START + 5)
#define EL_MM_TELEPORTER_GREEN_7		(EL_MM_TELEPORTER_GREEN_START + 6)
#define EL_MM_TELEPORTER_GREEN_8		(EL_MM_TELEPORTER_GREEN_START + 7)
#define EL_MM_TELEPORTER_GREEN_9		(EL_MM_TELEPORTER_GREEN_START + 8)
#define EL_MM_TELEPORTER_GREEN_10		(EL_MM_TELEPORTER_GREEN_START + 9)
#define EL_MM_TELEPORTER_GREEN_11		(EL_MM_TELEPORTER_GREEN_START + 10)
#define EL_MM_TELEPORTER_GREEN_12		(EL_MM_TELEPORTER_GREEN_START + 11)
#define EL_MM_TELEPORTER_GREEN_13		(EL_MM_TELEPORTER_GREEN_START + 12)
#define EL_MM_TELEPORTER_GREEN_14		(EL_MM_TELEPORTER_GREEN_START + 13)
#define EL_MM_TELEPORTER_GREEN_15		(EL_MM_TELEPORTER_GREEN_START + 14)
#define EL_MM_TELEPORTER_GREEN_16		(EL_MM_TELEPORTER_GREEN_START + 15)
#define EL_MM_TELEPORTER_GREEN_END		EL_MM_TELEPORTER_GREEN_16
#define EL_MM_TELEPORTER_BLUE_START		(EL_DF_START2 + 404)
#define EL_MM_TELEPORTER_BLUE_1			(EL_MM_TELEPORTER_BLUE_START + 0)
#define EL_MM_TELEPORTER_BLUE_2			(EL_MM_TELEPORTER_BLUE_START + 1)
#define EL_MM_TELEPORTER_BLUE_3			(EL_MM_TELEPORTER_BLUE_START + 2)
#define EL_MM_TELEPORTER_BLUE_4			(EL_MM_TELEPORTER_BLUE_START + 3)
#define EL_MM_TELEPORTER_BLUE_5			(EL_MM_TELEPORTER_BLUE_START + 4)
#define EL_MM_TELEPORTER_BLUE_6			(EL_MM_TELEPORTER_BLUE_START + 5)
#define EL_MM_TELEPORTER_BLUE_7			(EL_MM_TELEPORTER_BLUE_START + 6)
#define EL_MM_TELEPORTER_BLUE_8			(EL_MM_TELEPORTER_BLUE_START + 7)
#define EL_MM_TELEPORTER_BLUE_9			(EL_MM_TELEPORTER_BLUE_START + 8)
#define EL_MM_TELEPORTER_BLUE_10		(EL_MM_TELEPORTER_BLUE_START + 9)
#define EL_MM_TELEPORTER_BLUE_11		(EL_MM_TELEPORTER_BLUE_START + 10)
#define EL_MM_TELEPORTER_BLUE_12		(EL_MM_TELEPORTER_BLUE_START + 11)
#define EL_MM_TELEPORTER_BLUE_13		(EL_MM_TELEPORTER_BLUE_START + 12)
#define EL_MM_TELEPORTER_BLUE_14		(EL_MM_TELEPORTER_BLUE_START + 13)
#define EL_MM_TELEPORTER_BLUE_15		(EL_MM_TELEPORTER_BLUE_START + 14)
#define EL_MM_TELEPORTER_BLUE_16		(EL_MM_TELEPORTER_BLUE_START + 15)
#define EL_MM_TELEPORTER_BLUE_END		EL_MM_TELEPORTER_BLUE_16

#define EL_MM_MCDUFFIN				1204
#define EL_MM_PACMAN				1205
#define EL_MM_FUSE				1206
#define EL_MM_STEEL_WALL			1207
#define EL_MM_WOODEN_WALL			1208
#define EL_MM_ICE_WALL				1209
#define EL_MM_AMOEBA_WALL			1210
#define EL_DF_LASER				1211
#define EL_DF_RECEIVER				1212
#define EL_DF_STEEL_WALL			1213
#define EL_DF_WOODEN_WALL			1214

#define EL_MM_END_2				(EL_DF_START2 + 430)

// EMC style elements
#define EL_SPRING_LEFT				1215
#define EL_SPRING_RIGHT				1216

// ---------- begin of empty space elements section ---------------------------
#define EL_EMPTY_SPACE_START			1217

#include "conf_emp.h"	// include auto-generated data structure definitions

#define NUM_EMPTY_SPACE_ELEMENTS		16
#define NUM_EMPTY_ELEMENTS_ALL			(NUM_EMPTY_SPACE_ELEMENTS + 1)
#define EL_EMPTY_SPACE_END			1232
// ---------- end of empty space elements section -----------------------------

#define EL_MM_START_3				EL_DF_MIRROR_FIXED_START
#define EL_DF_START_2				EL_DF_MIRROR_FIXED_START

// DF style elements
#define EL_DF_MIRROR_FIXED_START		1233
#define EL_DF_MIRROR_FIXED_1			(EL_DF_MIRROR_FIXED_START + 0)
#define EL_DF_MIRROR_FIXED_2			(EL_DF_MIRROR_FIXED_START + 1)
#define EL_DF_MIRROR_FIXED_3			(EL_DF_MIRROR_FIXED_START + 2)
#define EL_DF_MIRROR_FIXED_4			(EL_DF_MIRROR_FIXED_START + 3)
#define EL_DF_MIRROR_FIXED_5			(EL_DF_MIRROR_FIXED_START + 4)
#define EL_DF_MIRROR_FIXED_6			(EL_DF_MIRROR_FIXED_START + 5)
#define EL_DF_MIRROR_FIXED_7			(EL_DF_MIRROR_FIXED_START + 6)
#define EL_DF_MIRROR_FIXED_8			(EL_DF_MIRROR_FIXED_START + 7)
#define EL_DF_MIRROR_FIXED_9			(EL_DF_MIRROR_FIXED_START + 8)
#define EL_DF_MIRROR_FIXED_10			(EL_DF_MIRROR_FIXED_START + 9)
#define EL_DF_MIRROR_FIXED_11			(EL_DF_MIRROR_FIXED_START + 10)
#define EL_DF_MIRROR_FIXED_12			(EL_DF_MIRROR_FIXED_START + 11)
#define EL_DF_MIRROR_FIXED_13			(EL_DF_MIRROR_FIXED_START + 12)
#define EL_DF_MIRROR_FIXED_14			(EL_DF_MIRROR_FIXED_START + 13)
#define EL_DF_MIRROR_FIXED_15			(EL_DF_MIRROR_FIXED_START + 14)
#define EL_DF_MIRROR_FIXED_16			(EL_DF_MIRROR_FIXED_START + 15)
#define EL_DF_MIRROR_FIXED_END			EL_DF_MIRROR_FIXED_16

#define EL_DF_SLOPE_START			1249
#define EL_DF_SLOPE_1				(EL_DF_SLOPE_START + 0)
#define EL_DF_SLOPE_2				(EL_DF_SLOPE_START + 1)
#define EL_DF_SLOPE_3				(EL_DF_SLOPE_START + 2)
#define EL_DF_SLOPE_4				(EL_DF_SLOPE_START + 3)
#define EL_DF_SLOPE_END				EL_DF_SLOPE_4

#define EL_MM_END_3				EL_DF_SLOPE_END
#define EL_DF_END_2				EL_DF_SLOPE_END

// BD style elements (normal)
#define EL_BDX_START				1253
#define EL_BDX_EMPTY_SPACE			EL_BDX_START
#define EL_BDX_EMPTY				EL_BDX_EMPTY_SPACE
#define EL_BDX_SAND				1254
#define EL_BDX_GRASS				1255
#define EL_BDX_GRASS_BALL			1256
#define EL_BDX_GRASS_LOOSE			1257
#define EL_BDX_SAND_SLOPED_UP_RIGHT		1258
#define EL_BDX_SAND_SLOPED_UP_LEFT		1259
#define EL_BDX_SAND_SLOPED_DOWN_LEFT		1260
#define EL_BDX_SAND_SLOPED_DOWN_RIGHT		1261
#define EL_BDX_SAND_GLUED			1262
#define EL_BDX_WALL_SLOPED_UP_RIGHT		1263
#define EL_BDX_WALL_SLOPED_UP_LEFT		1264
#define EL_BDX_WALL_SLOPED_DOWN_LEFT		1265
#define EL_BDX_WALL_SLOPED_DOWN_RIGHT		1266
#define EL_BDX_WALL_NON_SLOPED			1267
#define EL_BDX_WALL_DIGGABLE			1268
#define EL_BDX_WALL_DIAMOND			1269
#define EL_BDX_WALL_KEY_1			1270
#define EL_BDX_WALL_KEY_2			1271
#define EL_BDX_WALL_KEY_3			1272
#define EL_BDX_FALLING_WALL			1273
#define EL_BDX_STEELWALL			1274
#define EL_BDX_STEELWALL_SLOPED_UP_RIGHT	1275
#define EL_BDX_STEELWALL_SLOPED_UP_LEFT		1276
#define EL_BDX_STEELWALL_SLOPED_DOWN_LEFT	1277
#define EL_BDX_STEELWALL_SLOPED_DOWN_RIGHT	1278
#define EL_BDX_STEELWALL_EXPLODABLE		1279
#define EL_BDX_STEELWALL_DIGGABLE		1280
#define EL_BDX_EXPANDABLE_WALL_HORIZONTAL	1281
#define EL_BDX_EXPANDABLE_WALL_VERTICAL		1282
#define EL_BDX_EXPANDABLE_WALL_ANY		1283
#define EL_BDX_EXPANDABLE_STEELWALL_HORIZONTAL	1284
#define EL_BDX_EXPANDABLE_STEELWALL_VERTICAL	1285
#define EL_BDX_EXPANDABLE_STEELWALL_ANY		1286
#define EL_BDX_EXPANDABLE_WALL_SWITCH		1287
#define EL_BDX_EXPANDABLE_WALL_SWITCH_ACTIVE	1288
#define EL_BDX_INBOX				1289
#define EL_BDX_EXIT_CLOSED			1290
#define EL_BDX_EXIT_OPEN			1291
#define EL_BDX_INVISIBLE_EXIT_CLOSED		1292
#define EL_BDX_INVISIBLE_EXIT_OPEN		1293
#define EL_BDX_FLYING_ROCK			1294
#define EL_BDX_HEAVY_ROCK			1295
#define EL_BDX_ROCK_GLUED			1296
#define EL_BDX_FLYING_DIAMOND			1297
#define EL_BDX_DIAMOND_GLUED			1298
#define EL_BDX_DIAMOND_KEY			1299
#define EL_BDX_TRAPPED_DIAMOND			1300
#define EL_BDX_NUT				1301
#define EL_BDX_AMOEBA_1				1302
#define EL_BDX_AMOEBA_2				1303
#define EL_BDX_BUBBLE				1304
#define EL_BDX_TRAPPED_BUBBLE			1305
#define EL_BDX_CREATURE_SWITCH			1306
#define EL_BDX_CREATURE_SWITCH_ACTIVE		1307
#define EL_BDX_BITER_SWITCH_1			1308
#define EL_BDX_BITER_SWITCH_2			1309
#define EL_BDX_BITER_SWITCH_3			1310
#define EL_BDX_BITER_SWITCH_4			1311
#define EL_BDX_REPLICATOR			1312
#define EL_BDX_REPLICATOR_ACTIVE		1313
#define EL_BDX_REPLICATOR_SWITCH		1314
#define EL_BDX_REPLICATOR_SWITCH_ACTIVE		1315
#define EL_BDX_CONVEYOR_LEFT			1316
#define EL_BDX_CONVEYOR_LEFT_ACTIVE		1317
#define EL_BDX_CONVEYOR_RIGHT			1318
#define EL_BDX_CONVEYOR_RIGHT_ACTIVE		1319
#define EL_BDX_CONVEYOR_SWITCH			1320
#define EL_BDX_CONVEYOR_SWITCH_ACTIVE		1321
#define EL_BDX_CONVEYOR_DIR_SWITCH		1322
#define EL_BDX_CONVEYOR_DIR_SWITCH_ACTIVE	1323
#define EL_BDX_GRAVITY_SWITCH			1324
#define EL_BDX_GRAVITY_SWITCH_ACTIVE		1325
#define EL_BDX_ACID				1326
#define EL_BDX_BOX				1327
#define EL_BDX_TIME_PENALTY			1328
#define EL_BDX_GRAVESTONE			1329
#define EL_BDX_CLOCK				1330
#define EL_BDX_POT				1331
#define EL_BDX_PNEUMATIC_HAMMER			1332
#define EL_BDX_TELEPORTER			1333
#define EL_BDX_SKELETON				1334
#define EL_BDX_WATER				1335
#define EL_BDX_KEY_1				1336
#define EL_BDX_KEY_2				1337
#define EL_BDX_KEY_3				1338
#define EL_BDX_GATE_1				1339
#define EL_BDX_GATE_2				1340
#define EL_BDX_GATE_3				1341
#define EL_BDX_LAVA				1342
#define EL_BDX_SWEET				1343
#define EL_BDX_VOODOO_DOLL			1344
#define EL_BDX_SLIME				1345
#define EL_BDX_WAITING_ROCK			1346
#define EL_BDX_CHASING_ROCK			1347
#define EL_BDX_GHOST				1348
#define EL_BDX_COW				1349
#define EL_BDX_COW_LEFT				1350
#define EL_BDX_COW_UP				1351
#define EL_BDX_COW_RIGHT			1352
#define EL_BDX_COW_DOWN				1353
#define EL_BDX_BUTTERFLY_1			1354
#define EL_BDX_BUTTERFLY_1_RIGHT		1355
#define EL_BDX_BUTTERFLY_1_UP			1356
#define EL_BDX_BUTTERFLY_1_LEFT			1357
#define EL_BDX_BUTTERFLY_1_DOWN			1358
#define EL_BDX_BUTTERFLY_2			1359
#define EL_BDX_BUTTERFLY_2_RIGHT		1360
#define EL_BDX_BUTTERFLY_2_UP			1361
#define EL_BDX_BUTTERFLY_2_LEFT			1362
#define EL_BDX_BUTTERFLY_2_DOWN			1363
#define EL_BDX_FIREFLY_1			1364
#define EL_BDX_FIREFLY_1_RIGHT			1365
#define EL_BDX_FIREFLY_1_UP			1366
#define EL_BDX_FIREFLY_1_LEFT			1367
#define EL_BDX_FIREFLY_1_DOWN			1368
#define EL_BDX_FIREFLY_2			1369
#define EL_BDX_FIREFLY_2_RIGHT			1370
#define EL_BDX_FIREFLY_2_UP			1371
#define EL_BDX_FIREFLY_2_LEFT			1372
#define EL_BDX_FIREFLY_2_DOWN			1373
#define EL_BDX_STONEFLY				1374
#define EL_BDX_STONEFLY_RIGHT			1375
#define EL_BDX_STONEFLY_UP			1376
#define EL_BDX_STONEFLY_LEFT			1377
#define EL_BDX_STONEFLY_DOWN			1378
#define EL_BDX_BITER				1379
#define EL_BDX_BITER_RIGHT			1380
#define EL_BDX_BITER_UP				1381
#define EL_BDX_BITER_LEFT			1382
#define EL_BDX_BITER_DOWN			1383
#define EL_BDX_DRAGONFLY			1384
#define EL_BDX_DRAGONFLY_RIGHT			1385
#define EL_BDX_DRAGONFLY_UP			1386
#define EL_BDX_DRAGONFLY_LEFT			1387
#define EL_BDX_DRAGONFLY_DOWN			1388
#define EL_BDX_BOMB				1389
#define EL_BDX_NITRO_PACK			1390
#define EL_BDX_PLAYER				1391
#define EL_BDX_PLAYER_WITH_BOMB			1392
#define EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER	1393
#define EL_BDX_PLAYER_GLUED			1394
#define EL_BDX_PLAYER_STIRRING			1395
#define EL_BDX_ROCKET_LAUNCHER			1396
#define EL_BDX_ROCKET				1397
#define EL_BDX_ROCKET_RIGHT			1398
#define EL_BDX_ROCKET_UP			1399
#define EL_BDX_ROCKET_LEFT			1400
#define EL_BDX_ROCKET_DOWN			1401
#define EL_BDX_FAKE_BONUS			1402
#define EL_BDX_COVERED				1403
#define EL_BDX_WALL				1404
#define EL_BDX_ROCK				1405
#define EL_BDX_DIAMOND				1406
#define EL_BDX_MAGIC_WALL			1407
#define EL_BDX_LIGHT_ROCK			1408

// BD style elements (effects/scanned; runtime elements, but can also be stored in level file)
#define EL_BDX_RUNTIME_START			1409

#define EL_BDX_EFFECTS_START			EL_BDX_RUNTIME_START
#define EL_BDX_GRASS_BALL_FALLING		EL_BDX_EFFECTS_START
#define EL_BDX_GRASS_LOOSE_FALLING		1410
#define EL_BDX_ROCK_FALLING			1411
#define EL_BDX_FLYING_ROCK_FLYING		1412
#define EL_BDX_HEAVY_ROCK_FALLING		1413
#define EL_BDX_LIGHT_ROCK_FALLING		1414
#define EL_BDX_DIAMOND_FALLING			1415
#define EL_BDX_FLYING_DIAMOND_FLYING		1416
#define EL_BDX_NUT_FALLING			1417
#define EL_BDX_FALLING_WALL_FALLING		1418
#define EL_BDX_NITRO_PACK_FALLING		1419
#define EL_BDX_WATER_1				1420
#define EL_BDX_WATER_2				1421
#define EL_BDX_WATER_3				1422
#define EL_BDX_WATER_4				1423
#define EL_BDX_WATER_5				1424
#define EL_BDX_WATER_6				1425
#define EL_BDX_WATER_7				1426
#define EL_BDX_WATER_8				1427
#define EL_BDX_WATER_9				1428
#define EL_BDX_WATER_10				1429
#define EL_BDX_WATER_11				1430
#define EL_BDX_WATER_12				1431
#define EL_BDX_WATER_13				1432
#define EL_BDX_WATER_14				1433
#define EL_BDX_WATER_15				1434
#define EL_BDX_WATER_16				1435
#define EL_BDX_COW_ENCLOSED_1			1436
#define EL_BDX_COW_ENCLOSED_2			1437
#define EL_BDX_COW_ENCLOSED_3			1438
#define EL_BDX_COW_ENCLOSED_4			1439
#define EL_BDX_COW_ENCLOSED_5			1440
#define EL_BDX_COW_ENCLOSED_6			1441
#define EL_BDX_COW_ENCLOSED_7			1442
#define EL_BDX_BUBBLE_1				1443
#define EL_BDX_BUBBLE_2				1444
#define EL_BDX_BUBBLE_3				1445
#define EL_BDX_BUBBLE_4				1446
#define EL_BDX_BUBBLE_5				1447
#define EL_BDX_BUBBLE_6				1448
#define EL_BDX_BUBBLE_7				1449
#define EL_BDX_BUBBLE_8				1450
#define EL_BDX_PLAYER_GROWING_1			1451
#define EL_BDX_PLAYER_GROWING_2			1452
#define EL_BDX_PLAYER_GROWING_3			1453
#define EL_BDX_BOMB_TICKING_1			1454
#define EL_BDX_BOMB_TICKING_2			1455
#define EL_BDX_BOMB_TICKING_3			1456
#define EL_BDX_BOMB_TICKING_4			1457
#define EL_BDX_BOMB_TICKING_5			1458
#define EL_BDX_BOMB_TICKING_6			1459
#define EL_BDX_BOMB_TICKING_7			1460
#define EL_BDX_CLOCK_GROWING_1			1461
#define EL_BDX_CLOCK_GROWING_2			1462
#define EL_BDX_CLOCK_GROWING_3			1463
#define EL_BDX_CLOCK_GROWING_4			1464
#define EL_BDX_DIAMOND_GROWING_1		1465
#define EL_BDX_DIAMOND_GROWING_2		1466
#define EL_BDX_DIAMOND_GROWING_3		1467
#define EL_BDX_DIAMOND_GROWING_4		1468
#define EL_BDX_DIAMOND_GROWING_5		1469
#define EL_BDX_EXPLODING_1			1470
#define EL_BDX_EXPLODING_2			1471
#define EL_BDX_EXPLODING_3			1472
#define EL_BDX_EXPLODING_4			1473
#define EL_BDX_EXPLODING_5			1474
#define EL_BDX_ROCK_GROWING_1			1475
#define EL_BDX_ROCK_GROWING_2			1476
#define EL_BDX_ROCK_GROWING_3			1477
#define EL_BDX_ROCK_GROWING_4			1478
#define EL_BDX_STEELWALL_GROWING_1		1479
#define EL_BDX_STEELWALL_GROWING_2		1480
#define EL_BDX_STEELWALL_GROWING_3		1481
#define EL_BDX_STEELWALL_GROWING_4		1482
#define EL_BDX_GHOST_EXPLODING_1		1483
#define EL_BDX_GHOST_EXPLODING_2		1484
#define EL_BDX_GHOST_EXPLODING_3		1485
#define EL_BDX_GHOST_EXPLODING_4		1486
#define EL_BDX_BOMB_EXPLODING_1			1487
#define EL_BDX_BOMB_EXPLODING_2			1488
#define EL_BDX_BOMB_EXPLODING_3			1489
#define EL_BDX_BOMB_EXPLODING_4			1490
#define EL_BDX_NITRO_PACK_EXPLODING		1491
#define EL_BDX_NITRO_PACK_EXPLODING_1		1492
#define EL_BDX_NITRO_PACK_EXPLODING_2		1493
#define EL_BDX_NITRO_PACK_EXPLODING_3		1494
#define EL_BDX_NITRO_PACK_EXPLODING_4		1495
#define EL_BDX_AMOEBA_2_EXPLODING_1		1496
#define EL_BDX_AMOEBA_2_EXPLODING_2		1497
#define EL_BDX_AMOEBA_2_EXPLODING_3		1498
#define EL_BDX_AMOEBA_2_EXPLODING_4		1499
#define EL_BDX_NUT_BREAKING_1			1500
#define EL_BDX_NUT_BREAKING_2			1501
#define EL_BDX_NUT_BREAKING_3			1502
#define EL_BDX_NUT_BREAKING_4			1503
#define EL_BDX_EFFECTS_END			EL_BDX_NUT_BREAKING_4

#define EL_BDX_SCANNED_START			1504
#define EL_BDX_GRASS_BALL_SCANNED		EL_BDX_SCANNED_START
#define EL_BDX_GRASS_BALL_FALLING_SCANNED	1505
#define EL_BDX_GRASS_LOOSE_SCANNED		1506
#define EL_BDX_GRASS_LOOSE_FALLING_SCANNED	1507
#define EL_BDX_ROCK_SCANNED			1508
#define EL_BDX_ROCK_FALLING_SCANNED		1509
#define EL_BDX_FLYING_ROCK_SCANNED		1510
#define EL_BDX_FLYING_ROCK_FLYING_SCANNED	1511
#define EL_BDX_HEAVY_ROCK_SCANNED		1512
#define EL_BDX_HEAVY_ROCK_FALLING_SCANNED	1513
#define EL_BDX_LIGHT_ROCK_SCANNED		1514
#define EL_BDX_LIGHT_ROCK_FALLING_SCANNED	1515
#define EL_BDX_DIAMOND_SCANNED			1516
#define EL_BDX_DIAMOND_FALLING_SCANNED		1517
#define EL_BDX_FLYING_DIAMOND_SCANNED		1518
#define EL_BDX_FLYING_DIAMOND_FLYING_SCANNED	1519
#define EL_BDX_NUT_SCANNED			1520
#define EL_BDX_NUT_FALLING_SCANNED		1521
#define EL_BDX_EXPANDABLE_WALL_HORIZONTAL_SCANNED	1522
#define EL_BDX_EXPANDABLE_WALL_VERTICAL_SCANNED		1523
#define EL_BDX_EXPANDABLE_WALL_ANY_SCANNED		1524
#define EL_BDX_EXPANDABLE_STEELWALL_HORIZONTAL_SCANNED	1525
#define EL_BDX_EXPANDABLE_STEELWALL_VERTICAL_SCANNED	1526
#define EL_BDX_EXPANDABLE_STEELWALL_ANY_SCANNED		1527
#define EL_BDX_ACID_SCANNED			1528
#define EL_BDX_FALLING_WALL_FALLING_SCANNED	1529
#define EL_BDX_COW_LEFT_SCANNED			1530
#define EL_BDX_COW_UP_SCANNED			1531
#define EL_BDX_COW_RIGHT_SCANNED		1532
#define EL_BDX_COW_DOWN_SCANNED			1533
#define EL_BDX_AMOEBA_1_SCANNED			1534
#define EL_BDX_AMOEBA_2_SCANNED			1535
#define EL_BDX_WAITING_ROCK_SCANNED		1536
#define EL_BDX_CHASING_ROCK_SCANNED		1537
#define EL_BDX_GHOST_SCANNED			1538
#define EL_BDX_FIREFLY_1_RIGHT_SCANNED		1539
#define EL_BDX_FIREFLY_1_UP_SCANNED		1540
#define EL_BDX_FIREFLY_1_LEFT_SCANNED		1541
#define EL_BDX_FIREFLY_1_DOWN_SCANNED		1542
#define EL_BDX_FIREFLY_2_RIGHT_SCANNED		1543
#define EL_BDX_FIREFLY_2_UP_SCANNED		1544
#define EL_BDX_FIREFLY_2_LEFT_SCANNED		1545
#define EL_BDX_FIREFLY_2_DOWN_SCANNED		1546
#define EL_BDX_BUTTERFLY_1_RIGHT_SCANNED	1547
#define EL_BDX_BUTTERFLY_1_UP_SCANNED		1548
#define EL_BDX_BUTTERFLY_1_LEFT_SCANNED		1549
#define EL_BDX_BUTTERFLY_1_DOWN_SCANNED		1550
#define EL_BDX_BUTTERFLY_2_RIGHT_SCANNED	1551
#define EL_BDX_BUTTERFLY_2_UP_SCANNED		1552
#define EL_BDX_BUTTERFLY_2_LEFT_SCANNED		1553
#define EL_BDX_BUTTERFLY_2_DOWN_SCANNED		1554
#define EL_BDX_STONEFLY_RIGHT_SCANNED		1555
#define EL_BDX_STONEFLY_UP_SCANNED		1556
#define EL_BDX_STONEFLY_LEFT_SCANNED		1557
#define EL_BDX_STONEFLY_DOWN_SCANNED		1558
#define EL_BDX_BITER_RIGHT_SCANNED		1559
#define EL_BDX_BITER_UP_SCANNED			1560
#define EL_BDX_BITER_LEFT_SCANNED		1561
#define EL_BDX_BITER_DOWN_SCANNED		1562
#define EL_BDX_DRAGONFLY_RIGHT_SCANNED		1563
#define EL_BDX_DRAGONFLY_UP_SCANNED		1564
#define EL_BDX_DRAGONFLY_LEFT_SCANNED		1565
#define EL_BDX_DRAGONFLY_DOWN_SCANNED		1566
#define EL_BDX_PLAYER_SCANNED			1567
#define EL_BDX_PLAYER_WITH_BOMB_SCANNED		1568
#define EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER_SCANNED	1569
#define EL_BDX_ROCKET_RIGHT_SCANNED		1570
#define EL_BDX_ROCKET_UP_SCANNED		1571
#define EL_BDX_ROCKET_LEFT_SCANNED		1572
#define EL_BDX_ROCKET_DOWN_SCANNED		1573
#define EL_BDX_NITRO_PACK_SCANNED		1574
#define EL_BDX_NITRO_PACK_FALLING_SCANNED	1575
#define EL_BDX_NITRO_PACK_EXPLODING_SCANNED	1576
#define EL_BDX_CLOCK_GROWING_0			1577
#define EL_BDX_DIAMOND_GROWING_0		1578
#define EL_BDX_EXPLODING_0			1579
#define EL_BDX_ROCK_GROWING_0			1580
#define EL_BDX_STEELWALL_GROWING_0		1581
#define EL_BDX_GHOST_EXPLODING_0		1582
#define EL_BDX_BOMB_EXPLODING_0			1583
#define EL_BDX_NITRO_PACK_EXPLODING_0		1584
#define EL_BDX_AMOEBA_2_EXPLODING_0		1585
#define EL_BDX_NUT_BREAKING_0			1586
#define EL_BDX_SCANNED_END			EL_BDX_NUT_BREAKING_0

#define EL_BDX_RUNTIME_END			EL_BDX_SCANNED_END
#define EL_BDX_END				EL_BDX_RUNTIME_END

#define NUM_FILE_ELEMENTS			1587


// "real" (and therefore drawable) runtime elements
#define EL_FIRST_RUNTIME_REAL			NUM_FILE_ELEMENTS

#define EL_DYNABOMB_PLAYER_1_ACTIVE		(EL_FIRST_RUNTIME_REAL + 0)
#define EL_DYNABOMB_PLAYER_2_ACTIVE		(EL_FIRST_RUNTIME_REAL + 1)
#define EL_DYNABOMB_PLAYER_3_ACTIVE		(EL_FIRST_RUNTIME_REAL + 2)
#define EL_DYNABOMB_PLAYER_4_ACTIVE		(EL_FIRST_RUNTIME_REAL + 3)
#define EL_SP_DISK_RED_ACTIVE			(EL_FIRST_RUNTIME_REAL + 4)
#define EL_SWITCHGATE_OPENING			(EL_FIRST_RUNTIME_REAL + 5)
#define EL_SWITCHGATE_CLOSING			(EL_FIRST_RUNTIME_REAL + 6)
#define EL_TIMEGATE_OPENING			(EL_FIRST_RUNTIME_REAL + 7)
#define EL_TIMEGATE_CLOSING			(EL_FIRST_RUNTIME_REAL + 8)
#define EL_PEARL_BREAKING			(EL_FIRST_RUNTIME_REAL + 9)
#define EL_TRAP_ACTIVE				(EL_FIRST_RUNTIME_REAL + 10)
#define EL_INVISIBLE_STEELWALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 11)
#define EL_INVISIBLE_WALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 12)
#define EL_INVISIBLE_SAND_ACTIVE		(EL_FIRST_RUNTIME_REAL + 13)
#define EL_CONVEYOR_BELT_1_LEFT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 14)
#define EL_CONVEYOR_BELT_1_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 15)
#define EL_CONVEYOR_BELT_1_RIGHT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 16)
#define EL_CONVEYOR_BELT_2_LEFT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 17)
#define EL_CONVEYOR_BELT_2_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 18)
#define EL_CONVEYOR_BELT_2_RIGHT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 19)
#define EL_CONVEYOR_BELT_3_LEFT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 20)
#define EL_CONVEYOR_BELT_3_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 21)
#define EL_CONVEYOR_BELT_3_RIGHT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 22)
#define EL_CONVEYOR_BELT_4_LEFT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 23)
#define EL_CONVEYOR_BELT_4_MIDDLE_ACTIVE	(EL_FIRST_RUNTIME_REAL + 24)
#define EL_CONVEYOR_BELT_4_RIGHT_ACTIVE		(EL_FIRST_RUNTIME_REAL + 25)
#define EL_EXIT_OPENING				(EL_FIRST_RUNTIME_REAL + 26)
#define EL_EXIT_CLOSING				(EL_FIRST_RUNTIME_REAL + 27)
#define EL_STEEL_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 28)
#define EL_STEEL_EXIT_CLOSING			(EL_FIRST_RUNTIME_REAL + 29)
#define EL_EM_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 30)
#define EL_EM_EXIT_CLOSING			(EL_FIRST_RUNTIME_REAL + 31)
#define EL_EM_STEEL_EXIT_OPENING		(EL_FIRST_RUNTIME_REAL + 32)
#define EL_EM_STEEL_EXIT_CLOSING		(EL_FIRST_RUNTIME_REAL + 33)
#define EL_SP_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 34)
#define EL_SP_EXIT_CLOSING			(EL_FIRST_RUNTIME_REAL + 35)
#define EL_SP_EXIT_OPEN				(EL_FIRST_RUNTIME_REAL + 36)
#define EL_SP_TERMINAL_ACTIVE			(EL_FIRST_RUNTIME_REAL + 37)
#define EL_SP_BUGGY_BASE_ACTIVATING		(EL_FIRST_RUNTIME_REAL + 38)
#define EL_SP_BUGGY_BASE_ACTIVE			(EL_FIRST_RUNTIME_REAL + 39)
#define EL_SP_MURPHY_CLONE			(EL_FIRST_RUNTIME_REAL + 40)
#define EL_AMOEBA_DROPPING			(EL_FIRST_RUNTIME_REAL + 41)
#define EL_QUICKSAND_EMPTYING			(EL_FIRST_RUNTIME_REAL + 42)
#define EL_QUICKSAND_FAST_EMPTYING		(EL_FIRST_RUNTIME_REAL + 43)
#define EL_MAGIC_WALL_ACTIVE			(EL_FIRST_RUNTIME_REAL + 44)
#define EL_BD_MAGIC_WALL_ACTIVE			(EL_FIRST_RUNTIME_REAL + 45)
#define EL_DC_MAGIC_WALL_ACTIVE			(EL_FIRST_RUNTIME_REAL + 46)
#define EL_MAGIC_WALL_FULL			(EL_FIRST_RUNTIME_REAL + 47)
#define EL_BD_MAGIC_WALL_FULL			(EL_FIRST_RUNTIME_REAL + 48)
#define EL_DC_MAGIC_WALL_FULL			(EL_FIRST_RUNTIME_REAL + 49)
#define EL_MAGIC_WALL_EMPTYING			(EL_FIRST_RUNTIME_REAL + 50)
#define EL_BD_MAGIC_WALL_EMPTYING		(EL_FIRST_RUNTIME_REAL + 51)
#define EL_DC_MAGIC_WALL_EMPTYING		(EL_FIRST_RUNTIME_REAL + 52)
#define EL_MAGIC_WALL_DEAD			(EL_FIRST_RUNTIME_REAL + 53)
#define EL_BD_MAGIC_WALL_DEAD			(EL_FIRST_RUNTIME_REAL + 54)
#define EL_DC_MAGIC_WALL_DEAD			(EL_FIRST_RUNTIME_REAL + 55)
#define EL_EMC_FAKE_GRASS_ACTIVE		(EL_FIRST_RUNTIME_REAL + 56)
#define EL_GATE_1_GRAY_ACTIVE			(EL_FIRST_RUNTIME_REAL + 57)
#define EL_GATE_2_GRAY_ACTIVE			(EL_FIRST_RUNTIME_REAL + 58)
#define EL_GATE_3_GRAY_ACTIVE			(EL_FIRST_RUNTIME_REAL + 59)
#define EL_GATE_4_GRAY_ACTIVE			(EL_FIRST_RUNTIME_REAL + 60)
#define EL_EM_GATE_1_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 61)
#define EL_EM_GATE_2_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 62)
#define EL_EM_GATE_3_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 63)
#define EL_EM_GATE_4_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 64)
#define EL_EMC_GATE_5_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 65)
#define EL_EMC_GATE_6_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 66)
#define EL_EMC_GATE_7_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 67)
#define EL_EMC_GATE_8_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 68)
#define EL_DC_GATE_WHITE_GRAY_ACTIVE		(EL_FIRST_RUNTIME_REAL + 69)
#define EL_EMC_DRIPPER_ACTIVE			(EL_FIRST_RUNTIME_REAL + 70)
#define EL_EMC_SPRING_BUMPER_ACTIVE		(EL_FIRST_RUNTIME_REAL + 71)
#define EL_MM_EXIT_OPENING			(EL_FIRST_RUNTIME_REAL + 72)
#define EL_MM_EXIT_CLOSING			(EL_FIRST_RUNTIME_REAL + 73)
#define EL_MM_GRAY_BALL_ACTIVE			(EL_FIRST_RUNTIME_REAL + 74)
#define EL_MM_GRAY_BALL_OPENING			(EL_FIRST_RUNTIME_REAL + 75)
#define EL_MM_ICE_WALL_SHRINKING		(EL_FIRST_RUNTIME_REAL + 76)
#define EL_MM_AMOEBA_WALL_GROWING		(EL_FIRST_RUNTIME_REAL + 77)
#define EL_MM_PACMAN_EATING_RIGHT		(EL_FIRST_RUNTIME_REAL + 78)
#define EL_MM_PACMAN_EATING_UP			(EL_FIRST_RUNTIME_REAL + 79)
#define EL_MM_PACMAN_EATING_LEFT		(EL_FIRST_RUNTIME_REAL + 80)
#define EL_MM_PACMAN_EATING_DOWN		(EL_FIRST_RUNTIME_REAL + 81)
#define EL_MM_BOMB_ACTIVE			(EL_FIRST_RUNTIME_REAL + 82)
#define EL_DF_MINE_ACTIVE			(EL_FIRST_RUNTIME_REAL + 83)
#define EL_BDX_MAGIC_WALL_ACTIVE		(EL_FIRST_RUNTIME_REAL + 84)

#define NUM_DRAWABLE_ELEMENTS			(EL_FIRST_RUNTIME_REAL + 85)

#define EL_MM_RUNTIME_START			EL_MM_EXIT_OPENING
#define EL_MM_RUNTIME_END			EL_MM_AMOEBA_WALL_GROWING

// "unreal" (and therefore not drawable) runtime elements
#define EL_FIRST_RUNTIME_UNREAL			(NUM_DRAWABLE_ELEMENTS)

#define EL_BLOCKED				(EL_FIRST_RUNTIME_UNREAL + 0)
#define EL_EXPLOSION				(EL_FIRST_RUNTIME_UNREAL + 1)
#define EL_NUT_BREAKING				(EL_FIRST_RUNTIME_UNREAL + 2)
#define EL_DIAMOND_BREAKING			(EL_FIRST_RUNTIME_UNREAL + 3)
#define EL_ACID_SPLASH_LEFT			(EL_FIRST_RUNTIME_UNREAL + 4)
#define EL_ACID_SPLASH_RIGHT			(EL_FIRST_RUNTIME_UNREAL + 5)
#define EL_AMOEBA_GROWING			(EL_FIRST_RUNTIME_UNREAL + 6)
#define EL_AMOEBA_SHRINKING			(EL_FIRST_RUNTIME_UNREAL + 7)
#define EL_EXPANDABLE_WALL_GROWING		(EL_FIRST_RUNTIME_UNREAL + 8)
#define EL_EXPANDABLE_STEELWALL_GROWING		(EL_FIRST_RUNTIME_UNREAL + 9)
#define EL_FLAMES				(EL_FIRST_RUNTIME_UNREAL + 10)
#define EL_PLAYER_IS_LEAVING			(EL_FIRST_RUNTIME_UNREAL + 11)
#define EL_PLAYER_IS_EXPLODING_1		(EL_FIRST_RUNTIME_UNREAL + 12)
#define EL_PLAYER_IS_EXPLODING_2		(EL_FIRST_RUNTIME_UNREAL + 13)
#define EL_PLAYER_IS_EXPLODING_3		(EL_FIRST_RUNTIME_UNREAL + 14)
#define EL_PLAYER_IS_EXPLODING_4		(EL_FIRST_RUNTIME_UNREAL + 15)
#define EL_QUICKSAND_FILLING			(EL_FIRST_RUNTIME_UNREAL + 16)
#define EL_QUICKSAND_FAST_FILLING		(EL_FIRST_RUNTIME_UNREAL + 17)
#define EL_MAGIC_WALL_FILLING			(EL_FIRST_RUNTIME_UNREAL + 18)
#define EL_BD_MAGIC_WALL_FILLING		(EL_FIRST_RUNTIME_UNREAL + 19)
#define EL_DC_MAGIC_WALL_FILLING		(EL_FIRST_RUNTIME_UNREAL + 20)
#define EL_ELEMENT_SNAPPING			(EL_FIRST_RUNTIME_UNREAL + 21)
#define EL_DIAGONAL_SHRINKING			(EL_FIRST_RUNTIME_UNREAL + 22)
#define EL_DIAGONAL_GROWING			(EL_FIRST_RUNTIME_UNREAL + 23)

#define NUM_RUNTIME_ELEMENTS			(EL_FIRST_RUNTIME_UNREAL + 24)

// dummy elements (never used as game elements, only used as graphics)
#define EL_FIRST_DUMMY				NUM_RUNTIME_ELEMENTS

#define EL_STEELWALL_TOPLEFT			(EL_FIRST_DUMMY + 0)
#define EL_STEELWALL_TOPRIGHT			(EL_FIRST_DUMMY + 1)
#define EL_STEELWALL_BOTTOMLEFT			(EL_FIRST_DUMMY + 2)
#define EL_STEELWALL_BOTTOMRIGHT		(EL_FIRST_DUMMY + 3)
#define EL_STEELWALL_HORIZONTAL			(EL_FIRST_DUMMY + 4)
#define EL_STEELWALL_VERTICAL			(EL_FIRST_DUMMY + 5)
#define EL_INVISIBLE_STEELWALL_TOPLEFT		(EL_FIRST_DUMMY + 6)
#define EL_INVISIBLE_STEELWALL_TOPRIGHT		(EL_FIRST_DUMMY + 7)
#define EL_INVISIBLE_STEELWALL_BOTTOMLEFT	(EL_FIRST_DUMMY + 8)
#define EL_INVISIBLE_STEELWALL_BOTTOMRIGHT	(EL_FIRST_DUMMY + 9)
#define EL_INVISIBLE_STEELWALL_HORIZONTAL	(EL_FIRST_DUMMY + 10)
#define EL_INVISIBLE_STEELWALL_VERTICAL		(EL_FIRST_DUMMY + 11)
#define EL_DYNABOMB				(EL_FIRST_DUMMY + 12)
#define EL_DYNABOMB_ACTIVE			(EL_FIRST_DUMMY + 13)
#define EL_DYNABOMB_PLAYER_1			(EL_FIRST_DUMMY + 14)
#define EL_DYNABOMB_PLAYER_2			(EL_FIRST_DUMMY + 15)
#define EL_DYNABOMB_PLAYER_3			(EL_FIRST_DUMMY + 16)
#define EL_DYNABOMB_PLAYER_4			(EL_FIRST_DUMMY + 17)
#define EL_SHIELD_NORMAL_ACTIVE			(EL_FIRST_DUMMY + 18)
#define EL_SHIELD_DEADLY_ACTIVE			(EL_FIRST_DUMMY + 19)
#define EL_AMOEBA				(EL_FIRST_DUMMY + 20)
#define EL_MM_LIGHTBALL_RED			(EL_FIRST_DUMMY + 21)
#define EL_MM_LIGHTBALL_BLUE			(EL_FIRST_DUMMY + 22)
#define EL_MM_LIGHTBALL_YELLOW			(EL_FIRST_DUMMY + 23)
#define EL_DEFAULT				(EL_FIRST_DUMMY + 24)
#define EL_BD_DEFAULT				(EL_FIRST_DUMMY + 25)
#define EL_BDX_DEFAULT				(EL_FIRST_DUMMY + 26)
#define EL_SP_DEFAULT				(EL_FIRST_DUMMY + 27)
#define EL_SB_DEFAULT				(EL_FIRST_DUMMY + 28)
#define EL_MM_DEFAULT				(EL_FIRST_DUMMY + 29)
#define EL_GRAPHIC_1				(EL_FIRST_DUMMY + 30)
#define EL_GRAPHIC_2				(EL_FIRST_DUMMY + 31)
#define EL_GRAPHIC_3				(EL_FIRST_DUMMY + 32)
#define EL_GRAPHIC_4				(EL_FIRST_DUMMY + 33)
#define EL_GRAPHIC_5				(EL_FIRST_DUMMY + 34)
#define EL_GRAPHIC_6				(EL_FIRST_DUMMY + 35)
#define EL_GRAPHIC_7				(EL_FIRST_DUMMY + 36)
#define EL_GRAPHIC_8				(EL_FIRST_DUMMY + 37)

// internal elements (only used for internal purposes like copying)
#define EL_FIRST_INTERNAL			(EL_FIRST_DUMMY + 38)

#define EL_INTERNAL_CLIPBOARD_CUSTOM		(EL_FIRST_INTERNAL + 0)
#define EL_INTERNAL_CLIPBOARD_CHANGE		(EL_FIRST_INTERNAL + 1)
#define EL_INTERNAL_CLIPBOARD_GROUP		(EL_FIRST_INTERNAL + 2)
#define EL_INTERNAL_DUMMY			(EL_FIRST_INTERNAL + 3)

#define EL_INTERNAL_CASCADE_BD			(EL_FIRST_INTERNAL + 4)
#define EL_INTERNAL_CASCADE_BD_ACTIVE		(EL_FIRST_INTERNAL + 5)
#define EL_INTERNAL_CASCADE_BDX			(EL_FIRST_INTERNAL + 6)
#define EL_INTERNAL_CASCADE_BDX_ACTIVE		(EL_FIRST_INTERNAL + 7)
#define EL_INTERNAL_CASCADE_BDX_EFFECTS		(EL_FIRST_INTERNAL + 8)
#define EL_INTERNAL_CASCADE_BDX_EFFECTS_ACTIVE	(EL_FIRST_INTERNAL + 9)
#define EL_INTERNAL_CASCADE_BDX_SCANNED		(EL_FIRST_INTERNAL + 10)
#define EL_INTERNAL_CASCADE_BDX_SCANNED_ACTIVE	(EL_FIRST_INTERNAL + 11)
#define EL_INTERNAL_CASCADE_EM			(EL_FIRST_INTERNAL + 12)
#define EL_INTERNAL_CASCADE_EM_ACTIVE		(EL_FIRST_INTERNAL + 13)
#define EL_INTERNAL_CASCADE_EMC			(EL_FIRST_INTERNAL + 14)
#define EL_INTERNAL_CASCADE_EMC_ACTIVE		(EL_FIRST_INTERNAL + 15)
#define EL_INTERNAL_CASCADE_RND			(EL_FIRST_INTERNAL + 16)
#define EL_INTERNAL_CASCADE_RND_ACTIVE		(EL_FIRST_INTERNAL + 17)
#define EL_INTERNAL_CASCADE_SB			(EL_FIRST_INTERNAL + 18)
#define EL_INTERNAL_CASCADE_SB_ACTIVE		(EL_FIRST_INTERNAL + 19)
#define EL_INTERNAL_CASCADE_SP			(EL_FIRST_INTERNAL + 20)
#define EL_INTERNAL_CASCADE_SP_ACTIVE		(EL_FIRST_INTERNAL + 21)
#define EL_INTERNAL_CASCADE_DC			(EL_FIRST_INTERNAL + 22)
#define EL_INTERNAL_CASCADE_DC_ACTIVE		(EL_FIRST_INTERNAL + 23)
#define EL_INTERNAL_CASCADE_DX			(EL_FIRST_INTERNAL + 24)
#define EL_INTERNAL_CASCADE_DX_ACTIVE		(EL_FIRST_INTERNAL + 25)
#define EL_INTERNAL_CASCADE_MM			(EL_FIRST_INTERNAL + 26)
#define EL_INTERNAL_CASCADE_MM_ACTIVE		(EL_FIRST_INTERNAL + 27)
#define EL_INTERNAL_CASCADE_DF			(EL_FIRST_INTERNAL + 28)
#define EL_INTERNAL_CASCADE_DF_ACTIVE		(EL_FIRST_INTERNAL + 29)
#define EL_INTERNAL_CASCADE_CHARS		(EL_FIRST_INTERNAL + 30)
#define EL_INTERNAL_CASCADE_CHARS_ACTIVE	(EL_FIRST_INTERNAL + 31)
#define EL_INTERNAL_CASCADE_STEEL_CHARS		(EL_FIRST_INTERNAL + 32)
#define EL_INTERNAL_CASCADE_STEEL_CHARS_ACTIVE	(EL_FIRST_INTERNAL + 33)
#define EL_INTERNAL_CASCADE_CE			(EL_FIRST_INTERNAL + 34)
#define EL_INTERNAL_CASCADE_CE_ACTIVE		(EL_FIRST_INTERNAL + 35)
#define EL_INTERNAL_CASCADE_GE			(EL_FIRST_INTERNAL + 36)
#define EL_INTERNAL_CASCADE_GE_ACTIVE		(EL_FIRST_INTERNAL + 37)
#define EL_INTERNAL_CASCADE_ES			(EL_FIRST_INTERNAL + 38)
#define EL_INTERNAL_CASCADE_ES_ACTIVE		(EL_FIRST_INTERNAL + 39)
#define EL_INTERNAL_CASCADE_REF			(EL_FIRST_INTERNAL + 40)
#define EL_INTERNAL_CASCADE_REF_ACTIVE		(EL_FIRST_INTERNAL + 41)
#define EL_INTERNAL_CASCADE_USER		(EL_FIRST_INTERNAL + 42)
#define EL_INTERNAL_CASCADE_USER_ACTIVE		(EL_FIRST_INTERNAL + 43)
#define EL_INTERNAL_CASCADE_DYNAMIC		(EL_FIRST_INTERNAL + 44)
#define EL_INTERNAL_CASCADE_DYNAMIC_ACTIVE	(EL_FIRST_INTERNAL + 45)

#define EL_INTERNAL_CLIPBOARD_START		(EL_FIRST_INTERNAL + 0)
#define EL_INTERNAL_CLIPBOARD_END		(EL_FIRST_INTERNAL + 2)
#define EL_INTERNAL_START			(EL_FIRST_INTERNAL + 0)
#define EL_INTERNAL_END				(EL_FIRST_INTERNAL + 45)

#define MAX_NUM_ELEMENTS			(EL_FIRST_INTERNAL + 46)


// values for graphics/sounds action types
enum
{
  ACTION_DEFAULT = 0,
  ACTION_WAITING,
  ACTION_FALLING,
  ACTION_MOVING,
  ACTION_DIGGING,
  ACTION_SNAPPING,
  ACTION_COLLECTING,
  ACTION_DROPPING,
  ACTION_PUSHING,
  ACTION_WALKING,
  ACTION_PASSING,
  ACTION_IMPACT,
  ACTION_BREAKING,
  ACTION_ACTIVATING,
  ACTION_DEACTIVATING,
  ACTION_OPENING,
  ACTION_CLOSING,
  ACTION_ATTACKING,
  ACTION_GROWING,
  ACTION_SHRINKING,
  ACTION_ACTIVE,
  ACTION_FILLING,
  ACTION_EMPTYING,
  ACTION_CHANGING,
  ACTION_EXPLODING,
  ACTION_BORING,
  ACTION_BORING_1,
  ACTION_BORING_2,
  ACTION_BORING_3,
  ACTION_BORING_4,
  ACTION_BORING_5,
  ACTION_BORING_6,
  ACTION_BORING_7,
  ACTION_BORING_8,
  ACTION_BORING_9,
  ACTION_BORING_10,
  ACTION_SLEEPING,
  ACTION_SLEEPING_1,
  ACTION_SLEEPING_2,
  ACTION_SLEEPING_3,
  ACTION_AWAKENING,
  ACTION_DYING,
  ACTION_TURNING,
  ACTION_TURNING_FROM_LEFT,
  ACTION_TURNING_FROM_RIGHT,
  ACTION_TURNING_FROM_UP,
  ACTION_TURNING_FROM_DOWN,
  ACTION_SMASHED_BY_ROCK,
  ACTION_SMASHED_BY_SPRING,
  ACTION_EATING,
  ACTION_TWINKLING,
  ACTION_SPLASHING,
  ACTION_HITTING,
  ACTION_FLYING,
  ACTION_PAGE_1,
  ACTION_PAGE_2,
  ACTION_PAGE_3,
  ACTION_PAGE_4,
  ACTION_PAGE_5,
  ACTION_PAGE_6,
  ACTION_PAGE_7,
  ACTION_PAGE_8,
  ACTION_PAGE_9,
  ACTION_PAGE_10,
  ACTION_PAGE_11,
  ACTION_PAGE_12,
  ACTION_PAGE_13,
  ACTION_PAGE_14,
  ACTION_PAGE_15,
  ACTION_PAGE_16,
  ACTION_PAGE_17,
  ACTION_PAGE_18,
  ACTION_PAGE_19,
  ACTION_PAGE_20,
  ACTION_PAGE_21,
  ACTION_PAGE_22,
  ACTION_PAGE_23,
  ACTION_PAGE_24,
  ACTION_PAGE_25,
  ACTION_PAGE_26,
  ACTION_PAGE_27,
  ACTION_PAGE_28,
  ACTION_PAGE_29,
  ACTION_PAGE_30,
  ACTION_PAGE_31,
  ACTION_PAGE_32,
  ACTION_PART_1,
  ACTION_PART_2,
  ACTION_PART_3,
  ACTION_PART_4,
  ACTION_PART_5,
  ACTION_PART_6,
  ACTION_PART_7,
  ACTION_PART_8,
  ACTION_PART_9,
  ACTION_PART_10,
  ACTION_PART_11,
  ACTION_PART_12,
  ACTION_PART_13,
  ACTION_PART_14,
  ACTION_PART_15,
  ACTION_PART_16,
  ACTION_PART_17,
  ACTION_PART_18,
  ACTION_PART_19,
  ACTION_PART_20,
  ACTION_PART_21,
  ACTION_PART_22,
  ACTION_PART_23,
  ACTION_PART_24,
  ACTION_PART_25,
  ACTION_PART_26,
  ACTION_PART_27,
  ACTION_PART_28,
  ACTION_PART_29,
  ACTION_PART_30,
  ACTION_PART_31,
  ACTION_PART_32,
  ACTION_OTHER,

  NUM_ACTIONS
};

#define ACTION_BORING_LAST		ACTION_BORING_10
#define ACTION_SLEEPING_LAST		ACTION_SLEEPING_3


// values for special image configuration suffixes (must match game mode)
enum
{
  GFX_SPECIAL_ARG_DEFAULT = 0,
  GFX_SPECIAL_ARG_LOADING_INITIAL,
  GFX_SPECIAL_ARG_LOADING,
  GFX_SPECIAL_ARG_TITLE_INITIAL,
  GFX_SPECIAL_ARG_TITLE_INITIAL_1,
  GFX_SPECIAL_ARG_TITLE_INITIAL_2,
  GFX_SPECIAL_ARG_TITLE_INITIAL_3,
  GFX_SPECIAL_ARG_TITLE_INITIAL_4,
  GFX_SPECIAL_ARG_TITLE_INITIAL_5,
  GFX_SPECIAL_ARG_TITLE,
  GFX_SPECIAL_ARG_TITLE_1,
  GFX_SPECIAL_ARG_TITLE_2,
  GFX_SPECIAL_ARG_TITLE_3,
  GFX_SPECIAL_ARG_TITLE_4,
  GFX_SPECIAL_ARG_TITLE_5,
  GFX_SPECIAL_ARG_MAIN,
  GFX_SPECIAL_ARG_NAMES,
  GFX_SPECIAL_ARG_LEVELS,
  GFX_SPECIAL_ARG_LEVELNR,
  GFX_SPECIAL_ARG_SCORES,
  GFX_SPECIAL_ARG_SCOREINFO,
  GFX_SPECIAL_ARG_EDITOR,
  GFX_SPECIAL_ARG_INFO,
  GFX_SPECIAL_ARG_STORY,
  GFX_SPECIAL_ARG_SETUP,
  GFX_SPECIAL_ARG_PLAYING,
  GFX_SPECIAL_ARG_DOOR,
  GFX_SPECIAL_ARG_TAPE,
  GFX_SPECIAL_ARG_PANEL,
  GFX_SPECIAL_ARG_PREVIEW,
  GFX_SPECIAL_ARG_CRUMBLED,
  GFX_SPECIAL_ARG_MAINONLY,
  GFX_SPECIAL_ARG_NAMESONLY,
  GFX_SPECIAL_ARG_SCORESONLY,
  GFX_SPECIAL_ARG_TYPENAME,
  GFX_SPECIAL_ARG_TYPENAMES,
  GFX_SPECIAL_ARG_SUBMENU,
  GFX_SPECIAL_ARG_MENU,
  GFX_SPECIAL_ARG_TOONS,
  GFX_SPECIAL_ARG_SCORESOLD,
  GFX_SPECIAL_ARG_SCORESNEW,
  GFX_SPECIAL_ARG_NO_TITLE,
  GFX_SPECIAL_ARG_FADING,
  GFX_SPECIAL_ARG_RESTARTING,
  GFX_SPECIAL_ARG_QUIT,

  NUM_SPECIAL_GFX_ARGS
};

// these additional definitions are currently only used for draw offsets
enum
{
  GFX_SPECIAL_ARG_INFO_MAIN = 0,
  GFX_SPECIAL_ARG_INFO_TITLE,
  GFX_SPECIAL_ARG_INFO_ELEMENTS,
  GFX_SPECIAL_ARG_INFO_MUSIC,
  GFX_SPECIAL_ARG_INFO_CREDITS,
  GFX_SPECIAL_ARG_INFO_PROGRAM,
  GFX_SPECIAL_ARG_INFO_VERSION,
  GFX_SPECIAL_ARG_INFO_LEVELSET,
  GFX_SPECIAL_ARG_INFO_LEVEL,
  GFX_SPECIAL_ARG_INFO_STORY,

  NUM_SPECIAL_GFX_INFO_ARGS
};

// these additional definitions are currently only used for draw offsets
// (must match SETUP_MODE_* values as defined in src/screens.c)
// (should also match corresponding entries in src/conf_gfx.c)
enum
{
  GFX_SPECIAL_ARG_SETUP_MAIN = 0,
  GFX_SPECIAL_ARG_SETUP_GAME,
  GFX_SPECIAL_ARG_SETUP_ENGINES,
  GFX_SPECIAL_ARG_SETUP_EDITOR,
  GFX_SPECIAL_ARG_SETUP_GRAPHICS,
  GFX_SPECIAL_ARG_SETUP_SOUND,
  GFX_SPECIAL_ARG_SETUP_ARTWORK,
  GFX_SPECIAL_ARG_SETUP_INPUT,
  GFX_SPECIAL_ARG_SETUP_TOUCH,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_1,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_2,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_3,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_4,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_5,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_6,
  GFX_SPECIAL_ARG_SETUP_SHORTCUTS_7,
  GFX_SPECIAL_ARG_SETUP_CHOOSE_ARTWORK,
  GFX_SPECIAL_ARG_SETUP_CHOOSE_OTHER,

  NUM_SPECIAL_GFX_SETUP_ARGS
};

// values for image configuration suffixes
enum
{
  GFX_ARG_X = 0,
  GFX_ARG_Y,
  GFX_ARG_XPOS,
  GFX_ARG_YPOS,
  GFX_ARG_WIDTH,
  GFX_ARG_HEIGHT,
  GFX_ARG_VERTICAL,
  GFX_ARG_OFFSET,
  GFX_ARG_XOFFSET,
  GFX_ARG_YOFFSET,
  GFX_ARG_2ND_MOVEMENT_TILE,
  GFX_ARG_2ND_VERTICAL,
  GFX_ARG_2ND_OFFSET,
  GFX_ARG_2ND_XOFFSET,
  GFX_ARG_2ND_YOFFSET,
  GFX_ARG_2ND_SWAP_TILES,
  GFX_ARG_FRAMES,
  GFX_ARG_FRAMES_PER_LINE,
  GFX_ARG_START_FRAME,
  GFX_ARG_DELAY,
  GFX_ARG_ANIM_MODE,
  GFX_ARG_GLOBAL_SYNC,
  GFX_ARG_GLOBAL_ANIM_SYNC,
  GFX_ARG_CRUMBLED_LIKE,
  GFX_ARG_DIGGABLE_LIKE,
  GFX_ARG_BORDER_SIZE,
  GFX_ARG_STEP_OFFSET,
  GFX_ARG_STEP_XOFFSET,
  GFX_ARG_STEP_YOFFSET,
  GFX_ARG_STEP_DELAY,
  GFX_ARG_DIRECTION,
  GFX_ARG_POSITION,
  GFX_ARG_DRAW_XOFFSET,
  GFX_ARG_DRAW_YOFFSET,
  GFX_ARG_DRAW_MASKED,
  GFX_ARG_DRAW_ORDER,
  GFX_ARG_INIT_DELAY_FIXED,
  GFX_ARG_INIT_DELAY_RANDOM,
  GFX_ARG_INIT_DELAY_ACTION,
  GFX_ARG_ANIM_DELAY_FIXED,
  GFX_ARG_ANIM_DELAY_RANDOM,
  GFX_ARG_ANIM_DELAY_ACTION,
  GFX_ARG_POST_DELAY_FIXED,
  GFX_ARG_POST_DELAY_RANDOM,
  GFX_ARG_POST_DELAY_ACTION,
  GFX_ARG_INIT_EVENT,
  GFX_ARG_INIT_EVENT_ACTION,
  GFX_ARG_ANIM_EVENT,
  GFX_ARG_ANIM_EVENT_ACTION,
  GFX_ARG_NAME,
  GFX_ARG_SCALE_UP_FACTOR,
  GFX_ARG_TILE_SIZE,
  GFX_ARG_CLONE_FROM,
  GFX_ARG_FADE_MODE,
  GFX_ARG_FADE_DELAY,
  GFX_ARG_POST_DELAY,
  GFX_ARG_AUTO_DELAY,
  GFX_ARG_AUTO_DELAY_UNIT,
  GFX_ARG_ALIGN,
  GFX_ARG_VALIGN,
  GFX_ARG_SORT_PRIORITY,
  GFX_ARG_CLASS,
  GFX_ARG_STYLE,
  GFX_ARG_ALPHA,
  GFX_ARG_ACTIVE_XOFFSET,
  GFX_ARG_ACTIVE_YOFFSET,
  GFX_ARG_PRESSED_XOFFSET,
  GFX_ARG_PRESSED_YOFFSET,
  GFX_ARG_STACKED_XFACTOR,
  GFX_ARG_STACKED_YFACTOR,
  GFX_ARG_STACKED_XOFFSET,
  GFX_ARG_STACKED_YOFFSET,
  GFX_ARG_COLOR_TEMPLATE,

  NUM_GFX_ARGS
};

// values for sound configuration suffixes
enum
{
  SND_ARG_MODE_LOOP = 0,
  SND_ARG_VOLUME,
  SND_ARG_PRIORITY,

  NUM_SND_ARGS
};

// values for music configuration suffixes
enum
{
  MUS_ARG_MODE_LOOP = 0,

  NUM_MUS_ARGS
};

// values for font configuration (definitions must match those from main.c)
enum
{
  FONT_INITIAL_1 = MAIN_FONT_INITIAL_1,
  FONT_INITIAL_2 = MAIN_FONT_INITIAL_2,
  FONT_INITIAL_3 = MAIN_FONT_INITIAL_3,
  FONT_INITIAL_4 = MAIN_FONT_INITIAL_4,
  FONT_TITLE_1,
  FONT_TITLE_2,
  FONT_TITLE_STORY,
  FONT_FOOTER,
  FONT_MENU_1_ACTIVE,
  FONT_MENU_2_ACTIVE,
  FONT_MENU_1,
  FONT_MENU_2,
  FONT_TEXT_1_ACTIVE,
  FONT_TEXT_2_ACTIVE,
  FONT_TEXT_3_ACTIVE,
  FONT_TEXT_4_ACTIVE,
  FONT_TEXT_1,
  FONT_TEXT_2,
  FONT_TEXT_3,
  FONT_TEXT_4,
  FONT_ENVELOPE_1,
  FONT_ENVELOPE_2,
  FONT_ENVELOPE_3,
  FONT_ENVELOPE_4,
  FONT_REQUEST_NARROW,
  FONT_REQUEST,
  FONT_INPUT_1_ACTIVE,
  FONT_INPUT_2_ACTIVE,
  FONT_INPUT_1,
  FONT_INPUT_2,
  FONT_OPTION_OFF_NARROW,
  FONT_OPTION_OFF,
  FONT_OPTION_ON_NARROW,
  FONT_OPTION_ON,
  FONT_VALUE_1,
  FONT_VALUE_2,
  FONT_VALUE_OLD_NARROW,
  FONT_VALUE_OLD,
  FONT_VALUE_NARROW,
  FONT_LEVEL_NUMBER_ACTIVE,
  FONT_LEVEL_NUMBER,
  FONT_TAPE_RECORDER,
  FONT_GAME_INFO,
  FONT_INFO_ELEMENTS,
  FONT_INFO_LEVELSET,
  FONT_INFO_LEVEL,
  FONT_INFO_STORY,
  FONT_MAIN_NETWORK_PLAYERS,

  NUM_FONTS
};

#define NUM_INITIAL_FONTS		4

// values for toon animation configuration
#define MAX_NUM_TOONS			20

// values for global animation configuration (must match those from main.c)
#define NUM_GLOBAL_ANIMS		MAX_GLOBAL_ANIMS
#define NUM_GLOBAL_ANIM_PARTS		MAX_GLOBAL_ANIM_PARTS
#define NUM_GLOBAL_ANIM_PARTS_ALL	(NUM_GLOBAL_ANIM_PARTS + 1)
#define NUM_GLOBAL_ANIM_TOKENS		(2 * NUM_GLOBAL_ANIMS)

#define GLOBAL_ANIM_ID_GRAPHIC_FIRST	0
#define GLOBAL_ANIM_ID_GRAPHIC_LAST	(NUM_GLOBAL_ANIMS - 1)
#define GLOBAL_ANIM_ID_CONTROL_FIRST	(NUM_GLOBAL_ANIMS)
#define GLOBAL_ANIM_ID_CONTROL_LAST	(2 * NUM_GLOBAL_ANIMS - 1)

#define GLOBAL_ANIM_ID_PART_FIRST	0
#define GLOBAL_ANIM_ID_PART_LAST	(NUM_GLOBAL_ANIM_PARTS - 1)
#define GLOBAL_ANIM_ID_PART_BASE	(NUM_GLOBAL_ANIM_PARTS)

// values for global border graphics
#define IMG_GLOBAL_BORDER_FIRST		IMG_GLOBAL_BORDER
#define IMG_GLOBAL_BORDER_LAST		IMG_GLOBAL_BORDER_PLAYING

// values for game_status (must match special image configuration suffixes)
#define GAME_MODE_DEFAULT		GFX_SPECIAL_ARG_DEFAULT
#define GAME_MODE_LOADING_INITIAL	GFX_SPECIAL_ARG_LOADING_INITIAL
#define GAME_MODE_LOADING		GFX_SPECIAL_ARG_LOADING
#define GAME_MODE_TITLE_INITIAL		GFX_SPECIAL_ARG_TITLE_INITIAL
#define GAME_MODE_TITLE_INITIAL_1	GFX_SPECIAL_ARG_TITLE_INITIAL_1
#define GAME_MODE_TITLE_INITIAL_2	GFX_SPECIAL_ARG_TITLE_INITIAL_2
#define GAME_MODE_TITLE_INITIAL_3	GFX_SPECIAL_ARG_TITLE_INITIAL_3
#define GAME_MODE_TITLE_INITIAL_4	GFX_SPECIAL_ARG_TITLE_INITIAL_4
#define GAME_MODE_TITLE_INITIAL_5	GFX_SPECIAL_ARG_TITLE_INITIAL_5
#define GAME_MODE_TITLE			GFX_SPECIAL_ARG_TITLE
#define GAME_MODE_TITLE_1		GFX_SPECIAL_ARG_TITLE_1
#define GAME_MODE_TITLE_2		GFX_SPECIAL_ARG_TITLE_2
#define GAME_MODE_TITLE_3		GFX_SPECIAL_ARG_TITLE_3
#define GAME_MODE_TITLE_4		GFX_SPECIAL_ARG_TITLE_4
#define GAME_MODE_TITLE_5		GFX_SPECIAL_ARG_TITLE_5
#define GAME_MODE_MAIN			GFX_SPECIAL_ARG_MAIN
#define GAME_MODE_NAMES			GFX_SPECIAL_ARG_NAMES
#define GAME_MODE_LEVELS		GFX_SPECIAL_ARG_LEVELS
#define GAME_MODE_LEVELNR		GFX_SPECIAL_ARG_LEVELNR
#define GAME_MODE_SCORES		GFX_SPECIAL_ARG_SCORES
#define GAME_MODE_SCOREINFO		GFX_SPECIAL_ARG_SCOREINFO
#define GAME_MODE_EDITOR		GFX_SPECIAL_ARG_EDITOR
#define GAME_MODE_INFO			GFX_SPECIAL_ARG_INFO
#define GAME_MODE_STORY			GFX_SPECIAL_ARG_STORY
#define GAME_MODE_SETUP			GFX_SPECIAL_ARG_SETUP
#define GAME_MODE_PLAYING		GFX_SPECIAL_ARG_PLAYING
#define GAME_MODE_PSEUDO_DOOR		GFX_SPECIAL_ARG_DOOR
#define GAME_MODE_PSEUDO_TAPE		GFX_SPECIAL_ARG_TAPE
#define GAME_MODE_PSEUDO_PANEL		GFX_SPECIAL_ARG_PANEL
#define GAME_MODE_PSEUDO_PREVIEW	GFX_SPECIAL_ARG_PREVIEW
#define GAME_MODE_PSEUDO_CRUMBLED	GFX_SPECIAL_ARG_CRUMBLED
#define GAME_MODE_PSEUDO_MAINONLY	GFX_SPECIAL_ARG_MAINONLY
#define GAME_MODE_PSEUDO_NAMESONLY	GFX_SPECIAL_ARG_NAMESONLY
#define GAME_MODE_PSEUDO_SCORESONLY	GFX_SPECIAL_ARG_SCORESONLY
#define GAME_MODE_PSEUDO_TYPENAME	GFX_SPECIAL_ARG_TYPENAME
#define GAME_MODE_PSEUDO_TYPENAMES	GFX_SPECIAL_ARG_TYPENAMES
#define GAME_MODE_PSEUDO_SUBMENU	GFX_SPECIAL_ARG_SUBMENU
#define GAME_MODE_PSEUDO_MENU		GFX_SPECIAL_ARG_MENU
#define GAME_MODE_PSEUDO_TOONS		GFX_SPECIAL_ARG_TOONS
#define GAME_MODE_PSEUDO_SCORESOLD	GFX_SPECIAL_ARG_SCORESOLD
#define GAME_MODE_PSEUDO_SCORESNEW	GFX_SPECIAL_ARG_SCORESNEW
#define GAME_MODE_PSEUDO_NO_TITLE	GFX_SPECIAL_ARG_NO_TITLE
#define GAME_MODE_PSEUDO_FADING		GFX_SPECIAL_ARG_FADING
#define GAME_MODE_PSEUDO_RESTARTING	GFX_SPECIAL_ARG_RESTARTING
#define GAME_MODE_QUIT			GFX_SPECIAL_ARG_QUIT

#define NUM_GAME_MODES			NUM_SPECIAL_GFX_ARGS

// special definitions currently only used for custom artwork configuration
#define MUSIC_PREFIX_BACKGROUND		0
#define NUM_MUSIC_PREFIXES		1

// definitions for demo animation lists
#define HELPANIM_LIST_NEXT		-1
#define HELPANIM_LIST_END		-999


// program information and versioning definitions
#define PROGRAM_VERSION_SUPER		4
#define PROGRAM_VERSION_MAJOR		4
#define PROGRAM_VERSION_MINOR		0
#define PROGRAM_VERSION_PATCH		4

#define PROGRAM_VERSION_STABLE		1
#define PROGRAM_VERSION_EXTRA		0
#define PROGRAM_VERSION_BUILD		0
#define PROGRAM_VERSION_EXTRA_TEXT	"test"
#define PROGRAM_VERSION_BUILD_TEXT	"build"

#define PROGRAM_TITLE_STRING		"Rocks'n'Diamonds"
#define PROGRAM_AUTHOR_STRING		"Holger Schemel"
#define PROGRAM_EMAIL_STRING		"info@artsoft.org"
#define PROGRAM_WEBSITE_STRING		"https://www.artsoft.org/"
#define PROGRAM_COPYRIGHT_STRING	"1995-2025 by Holger Schemel"
#define PROGRAM_COMPANY_STRING		"A Game by Artsoft Entertainment"

#define PROGRAM_ICON_FILENAME		"icons/icon.png"

#define COOKIE_PREFIX			"ROCKSNDIAMONDS"

#define USERDATA_DIRECTORY_OTHER	"userdata"

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

#define GAME_VERSION_ACTUAL		VERSION_IDENT(PROGRAM_VERSION_SUPER,		\
						      PROGRAM_VERSION_MAJOR,		\
						      PROGRAM_VERSION_MINOR,		\
						      PROGRAM_VERSION_PATCH)

#define GAME_VERSION_ACTUAL_FULL	VERSION_IDENT_FULL(PROGRAM_VERSION_SUPER,	\
							   PROGRAM_VERSION_MAJOR,	\
							   PROGRAM_VERSION_MINOR,	\
							   PROGRAM_VERSION_PATCH,	\
							   PROGRAM_VERSION_STABLE,	\
							   PROGRAM_VERSION_EXTRA,	\
							   PROGRAM_VERSION_BUILD)

// values for game_emulation
#define EMU_NONE			0
#define EMU_BOULDERDASH			1
#define EMU_UNUSED_2			2
#define EMU_SUPAPLEX			3

// values for level file type identifier
#define LEVEL_FILE_TYPE_UNKNOWN		0
#define LEVEL_FILE_TYPE_RND		1
#define LEVEL_FILE_TYPE_BD		2
#define LEVEL_FILE_TYPE_EM		3
#define LEVEL_FILE_TYPE_SP		4
#define LEVEL_FILE_TYPE_DX		5
#define LEVEL_FILE_TYPE_SB		6
#define LEVEL_FILE_TYPE_DC		7
#define LEVEL_FILE_TYPE_MM		8

#define NUM_LEVEL_FILE_TYPES		9

// values for game engine type identifier
#define GAME_ENGINE_TYPE_UNKNOWN	LEVEL_FILE_TYPE_UNKNOWN
#define GAME_ENGINE_TYPE_RND		LEVEL_FILE_TYPE_RND
#define GAME_ENGINE_TYPE_BD		LEVEL_FILE_TYPE_BD
#define GAME_ENGINE_TYPE_EM		LEVEL_FILE_TYPE_EM
#define GAME_ENGINE_TYPE_SP		LEVEL_FILE_TYPE_SP
#define GAME_ENGINE_TYPE_MM		LEVEL_FILE_TYPE_MM

#define NUM_ENGINE_TYPES		5

// values for automatically playing tapes
#define AUTOPLAY_NONE			0
#define AUTOPLAY_PLAY			(1 << 0)
#define AUTOPLAY_FFWD			(1 << 1)
#define AUTOPLAY_WARP			(1 << 2)
#define AUTOPLAY_TEST			(1 << 3)
#define AUTOPLAY_SAVE			(1 << 4)
#define AUTOPLAY_UPLOAD			(1 << 5)
#define AUTOPLAY_FIX			(1 << 6)
#define AUTOPLAY_WARP_NO_DISPLAY	AUTOPLAY_TEST

#define AUTOPLAY_MODE_NONE		0
#define AUTOPLAY_MODE_PLAY		(AUTOPLAY_MODE_NONE | AUTOPLAY_PLAY)
#define AUTOPLAY_MODE_FFWD		(AUTOPLAY_MODE_PLAY | AUTOPLAY_FFWD)
#define AUTOPLAY_MODE_WARP		(AUTOPLAY_MODE_FFWD | AUTOPLAY_WARP)
#define AUTOPLAY_MODE_TEST		(AUTOPLAY_MODE_WARP | AUTOPLAY_TEST)
#define AUTOPLAY_MODE_SAVE		(AUTOPLAY_MODE_TEST | AUTOPLAY_SAVE)
#define AUTOPLAY_MODE_UPLOAD		(AUTOPLAY_MODE_TEST | AUTOPLAY_UPLOAD)
#define AUTOPLAY_MODE_FIX		(AUTOPLAY_MODE_TEST | AUTOPLAY_FIX)
#define AUTOPLAY_MODE_WARP_NO_DISPLAY	AUTOPLAY_MODE_TEST


struct BorderInfo
{
  boolean draw_masked[NUM_SPECIAL_GFX_ARGS];
  boolean draw_masked_when_fading;
};

struct RequestButtonInfo
{
  struct TextPosInfo yes;
  struct TextPosInfo no;
  struct TextPosInfo confirm;

  struct TextPosInfo player_1;
  struct TextPosInfo player_2;
  struct TextPosInfo player_3;
  struct TextPosInfo player_4;

  struct TextPosInfo touch_yes;
  struct TextPosInfo touch_no;
  struct TextPosInfo touch_confirm;
};

struct RequestInfo
{
  struct RequestButtonInfo button;
  int x, y;
  int width, height;
  int border_size;
  int line_spacing;
  int step_offset;
  int step_delay;
  int anim_mode;
  int align;
  int valign;
  int sort_priority;
  boolean autowrap;
  boolean centered;
  boolean wrap_single_words;

  // run-time values
  Bitmap *bitmap;
  int sx, sy;
  int xsize, ysize;
};

struct MenuTextInfo
{
  struct TextPosInfo title;
  struct TextPosInfo title_1;
  struct TextPosInfo title_2;
  struct TextPosInfo title_story;
  struct TextPosInfo footer;
};

struct MenuMainButtonInfo
{
  struct MenuPosInfo name;
  struct MenuPosInfo levels;
  struct MenuPosInfo scores;
  struct MenuPosInfo editor;
  struct MenuPosInfo info;
  struct MenuPosInfo game;
  struct MenuPosInfo setup;
  struct MenuPosInfo quit;

  struct MenuPosInfo prev_level;
  struct MenuPosInfo next_level;

  struct MenuPosInfo first_level;
  struct MenuPosInfo last_level;
  struct MenuPosInfo level_number;

  struct MenuPosInfo insert_solution;
  struct MenuPosInfo play_solution;

  struct MenuPosInfo levelset_info;
  struct MenuPosInfo level_info;
  struct MenuPosInfo switch_ecs_aga;
};

struct MenuMainTextInfo
{
  struct TextPosInfo name;
  struct TextPosInfo levels;
  struct TextPosInfo scores;
  struct TextPosInfo editor;
  struct TextPosInfo info;
  struct TextPosInfo game;
  struct TextPosInfo setup;
  struct TextPosInfo quit;

  struct TextPosInfo first_level;
  struct TextPosInfo last_level;
  struct TextPosInfo level_number;
  struct TextPosInfo level_info_1;
  struct TextPosInfo level_info_2;
  struct TextPosInfo level_name;
  struct TextPosInfo level_author;
  struct TextPosInfo level_year;
  struct TextPosInfo level_imported_from;
  struct TextPosInfo level_imported_by;
  struct TextPosInfo level_tested_by;
  struct TextPosInfo title_1;
  struct TextPosInfo title_2;
  struct TextPosInfo title_3;
};

struct MenuMainInputInfo
{
  struct TextPosInfo name;
};

struct MenuMainInfo
{
  struct MenuMainButtonInfo button;
  struct MenuMainTextInfo text;
  struct MenuMainInputInfo input;

  struct TextPosInfo preview_players;
  struct TextPosInfo network_players;
};

struct MenuInfoButtonInfo
{
  struct MenuPosInfo prev_level;
  struct MenuPosInfo next_level;
};

struct MenuInfoInfo
{
  struct MenuInfoButtonInfo button;
};

struct MenuSetupButtonInfo
{
  struct MenuPosInfo prev_player;
  struct MenuPosInfo next_player;

  struct MenuPosInfo touch_back;
  struct MenuPosInfo touch_next;
  struct MenuPosInfo touch_back2;
  struct MenuPosInfo touch_next2;
};

struct MenuSetupInfo
{
  struct MenuSetupButtonInfo button;
};

struct MenuScoresButtonInfo
{
  struct MenuPosInfo prev_level;
  struct MenuPosInfo next_level;
  struct MenuPosInfo prev_score;
  struct MenuPosInfo next_score;
  struct MenuPosInfo play_tape;
};

struct MenuScoresInfo
{
  struct MenuScoresButtonInfo button;
};

struct TitleFadingInfo
{
  int fade_mode;
  int fade_delay;
  int post_delay;
  int auto_delay;
  int auto_delay_unit;
};

struct TitleMessageInfo
{
  int x, y;
  int width, height;
  int chars, lines;
  int align, valign;
  int font;
  boolean autowrap;
  boolean centered;
  boolean parse_comments;
  int sort_priority;

  int fade_mode;
  int fade_delay;
  int post_delay;
  int auto_delay;
  int auto_delay_unit;
};

struct InitInfo
{
  struct MenuPosInfo busy_initial;
  struct MenuPosInfo busy;
  struct MenuPosInfo busy_playfield;
};

struct MenuInfo
{
  int draw_xoffset[NUM_SPECIAL_GFX_ARGS];
  int draw_yoffset[NUM_SPECIAL_GFX_ARGS];
  int draw_xoffset_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int draw_yoffset_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int draw_xoffset_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int draw_yoffset_setup[NUM_SPECIAL_GFX_SETUP_ARGS];

  int scrollbar_xoffset;

  struct MenuPosInfo list_setup[NUM_SPECIAL_GFX_SETUP_ARGS];

  int list_size[NUM_SPECIAL_GFX_ARGS];
  int list_size_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int list_entry_size_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int tile_size_info[NUM_SPECIAL_GFX_INFO_ARGS];

  int left_spacing[NUM_SPECIAL_GFX_ARGS];
  int left_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int left_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int middle_spacing[NUM_SPECIAL_GFX_ARGS];
  int middle_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int right_spacing[NUM_SPECIAL_GFX_ARGS];
  int right_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int right_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int top_spacing[NUM_SPECIAL_GFX_ARGS];
  int top_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int top_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int bottom_spacing[NUM_SPECIAL_GFX_ARGS];
  int bottom_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int bottom_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];

  int paragraph_spacing[NUM_SPECIAL_GFX_ARGS];
  int paragraph_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int paragraph_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int headline1_spacing[NUM_SPECIAL_GFX_ARGS];
  int headline1_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int headline1_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int headline2_spacing[NUM_SPECIAL_GFX_ARGS];
  int headline2_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int headline2_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int line_spacing[NUM_SPECIAL_GFX_ARGS];
  int line_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int line_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];
  int extra_spacing[NUM_SPECIAL_GFX_ARGS];
  int extra_spacing_info[NUM_SPECIAL_GFX_INFO_ARGS];
  int extra_spacing_setup[NUM_SPECIAL_GFX_SETUP_ARGS];

  struct TitleFadingInfo enter_menu;
  struct TitleFadingInfo leave_menu;
  struct TitleFadingInfo enter_screen[NUM_SPECIAL_GFX_ARGS];
  struct TitleFadingInfo leave_screen[NUM_SPECIAL_GFX_ARGS];
  struct TitleFadingInfo next_screen[NUM_SPECIAL_GFX_ARGS];

  int sound[NUM_SPECIAL_GFX_ARGS];
  int music[NUM_SPECIAL_GFX_ARGS];

  struct MenuTextInfo text;
  struct MenuMainInfo main;
  struct MenuInfoInfo info;
  struct MenuSetupInfo setup;
  struct MenuScoresInfo scores;

  struct RequestInfo request;
};

struct DoorInfo
{
  struct DoorPartPosInfo part_1;
  struct DoorPartPosInfo part_2;
  struct DoorPartPosInfo part_3;
  struct DoorPartPosInfo part_4;
  struct DoorPartPosInfo part_5;
  struct DoorPartPosInfo part_6;
  struct DoorPartPosInfo part_7;
  struct DoorPartPosInfo part_8;

  struct DoorPartPosInfo panel;

  int width;
  int height;
  int step_offset;
  int step_delay;
  int post_delay;
  int anim_mode;
};

struct PreviewInfo
{
  int x, y;
  int align, valign;
  int xsize, ysize;
  int xoffset, yoffset;
  int tile_size;
  int step_offset;
  int step_delay;
  int anim_mode;

  boolean redefined;		// redefined by custom artwork
};

struct EditorTabsInfo
{
  int x;
  int y;
  int yoffset2;
  int width;
  int height;
  int draw_xoffset;
  int draw_yoffset;
};

struct EditorSettingsInfo
{
  struct MenuPosInfo headline;

  struct XY element_graphic;
  struct XY element_name;

  struct EditorTabsInfo tabs;

  struct XY colorpicker;
  struct XY tooltip;
};

struct EditorGadgetInfo
{
  int normal_spacing;
  int small_spacing;
  int tiny_spacing;
  int line_spacing;
  int text_spacing;
  int tab_spacing;

  struct Rect separator_line;
};

struct EditorButtonInfo
{
  struct XYTileSize prev_level;
  struct XYTileSize next_level;

  struct XYTileSize properties;
  struct XYTileSize element_left;
  struct XYTileSize element_middle;
  struct XYTileSize element_right;
  struct XYTileSize palette;

  struct XYTileSize draw_single;
  struct XYTileSize draw_connected;
  struct XYTileSize draw_line;
  struct XYTileSize draw_arc;
  struct XYTileSize draw_rectangle;
  struct XYTileSize draw_filled_box;
  struct XYTileSize rotate_up;
  struct XYTileSize draw_text;
  struct XYTileSize flood_fill;
  struct XYTileSize rotate_left;
  struct XYTileSize zoom_level;
  struct XYTileSize rotate_right;
  struct XYTileSize draw_random;
  struct XYTileSize grab_brush;
  struct XYTileSize rotate_down;
  struct XYTileSize pick_element;

  struct XYTileSize ce_copy_from;
  struct XYTileSize ce_copy_to;
  struct XYTileSize ce_swap;
  struct XYTileSize ce_copy;
  struct XYTileSize ce_paste;

  struct XYTileSize undo;
  struct XYTileSize conf;
  struct XYTileSize save;
  struct XYTileSize clear;
  struct XYTileSize test;
  struct XYTileSize exit;
};

struct EditorInputInfo
{
  struct XY level_number;
};

struct EditorPaletteInfo
{
  int x, y;
  int cols, rows;
  int tile_size;
  boolean show_as_separate_screen;
  boolean show_on_element_buttons;

  struct XYTileSize element_left;
  struct XYTileSize element_middle;
  struct XYTileSize element_right;
};

struct EditorDrawingAreaInfo
{
  int tile_size;
};

struct EditorInfo
{
  struct EditorSettingsInfo settings;
  struct EditorGadgetInfo gadget;
  struct EditorButtonInfo button;
  struct EditorInputInfo input;
  struct EditorPaletteInfo palette;
  struct EditorDrawingAreaInfo drawingarea;
};

struct ViewportInfo
{
  struct RectWithBorder window[NUM_SPECIAL_GFX_ARGS];
  struct RectWithBorder playfield[NUM_SPECIAL_GFX_ARGS];
  struct RectWithBorder door_1[NUM_SPECIAL_GFX_ARGS];
  struct RectWithBorder door_2[NUM_SPECIAL_GFX_ARGS];
};

struct ScoreEntry
{
  char tape_basename[MAX_FILENAME_LEN + 1];
  char name[MAX_PLAYER_NAME_LEN + 1];
  int score;
  int time;		// time (in frames) or steps played

  // additional score information for score info screen
  int id;
  char tape_date[MAX_ISO_DATE_LEN + 1];
  char platform[MAX_PLATFORM_TEXT_LEN + 1];
  char version[MAX_VERSION_TEXT_LEN + 1];
  char country_code[MAX_COUNTRY_CODE_LEN + 1];
  char country_name[MAX_COUNTRY_NAME_LEN + 1];
};

struct ScoreInfo
{
  VersionType file_version;		// file format version the score is stored with
  VersionType game_version;		// game release version the score was created with

  char level_identifier[MAX_FILENAME_LEN + 1];
  int level_nr;

  int num_entries;
  int last_added;
  int last_added_local;
  int last_level_nr;
  int last_entry_nr;
  int next_level_nr;

  boolean updated;
  boolean uploaded;
  boolean tape_downloaded;
  boolean force_last_added;
  boolean continue_playing;
  boolean continue_on_return;

  struct ScoreEntry entry[MAX_SCORE_ENTRIES];
};

struct Content
{
  int e[3][3];
};

struct EnvelopeInfo
{
  int xsize;
  int ysize;

  boolean autowrap;
  boolean centered;

  char text[MAX_ENVELOPE_TEXT_LEN + 1];
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

  enum
  {
    DATE_SRC_CLOCK,
    DATE_SRC_LEVELFILE
  } src;
};

struct LevelInfo
{
  struct LevelFileInfo file_info;

  int game_engine_type;

  // level stored in native format for the alternative native game engines
  struct LevelInfo_BD *native_bd_level;
  struct LevelInfo_EM *native_em_level;
  struct LevelInfo_SP *native_sp_level;
  struct LevelInfo_MM *native_mm_level;

  VersionType file_version;		// file format version the level is stored with
  VersionType game_version;		// game release version the level was created with

  struct DateInfo creation_date;

  boolean encoding_16bit_field;		// level contains 16-bit elements
  boolean encoding_16bit_yamyam;	// yamyam contains 16-bit elements
  boolean encoding_16bit_amoeba;	// amoeba contains 16-bit elements

  int fieldx, fieldy;

  int time;				// available time (seconds)
  int gems_needed;
  boolean auto_count_gems;
  boolean rate_time_over_score;

  char name_native[MAX_OUTPUT_LINESIZE + 1];
  char name[MAX_LEVEL_NAME_LEN + 1];
  char author[MAX_LEVEL_AUTHOR_LEN + 1];

  int random_seed;

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

  // values for the new EMC elements
  int android_move_time;
  int android_clone_time;
  boolean ball_random;
  boolean ball_active_initial;
  int ball_time;
  int lenses_score;
  int magnify_score;
  int slurp_score;
  int lenses_time;
  int magnify_time;
  int wind_direction_initial;

  struct Content ball_content[MAX_ELEMENT_CONTENTS];
  int num_ball_contents;

  int num_android_clone_elements;
  int android_clone_element[MAX_ANDROID_ELEMENTS];

  int can_move_into_acid_bits;		// bitfield to store property for elements
  int dont_collide_with_bits;		// bitfield to store property for elements

  int initial_player_stepsize[MAX_PLAYERS];	// initial player speed
  boolean initial_player_gravity[MAX_PLAYERS];

  boolean use_initial_inventory[MAX_PLAYERS];
  int initial_inventory_size[MAX_PLAYERS];
  int initial_inventory_content[MAX_PLAYERS][MAX_INITIAL_INVENTORY_SIZE];

  int bd_cycle_delay_ms;		// BD game cycle delay (in milliseconds)
  int bd_cycle_delay_c64;		// BD game cycle delay (in C64 game units)
  int bd_hatching_delay_cycles;		// BD hatching delay (in game cycles)
  int bd_hatching_delay_seconds;	// BD hatching delay (in seconds)
  int bd_scheduling_type;		// BD engine scheduling type
  boolean bd_pal_timing;		// BD engine uses special PAL timing
  boolean bd_line_shifting_borders;	// BD engine uses line-shifting wrap-around
  boolean bd_scan_first_and_last_row;	// BD engine scans top and bottom border rows
  boolean bd_short_explosions;		// BD engine uses four game cycles for explosions
  boolean bd_intermission;		// BD level is intermission
  boolean bd_intermission_clipped;	// BD intermission should be clipped to standard size
  boolean bd_diagonal_movements;	// BD style diagonal movements
  boolean bd_topmost_player_active;	// BD engine uses first player found on playfield
  int bd_snap_element;			// BD element that is created when player is snapping
  int bd_pushing_prob;			// BD player probability to push rocks
  int bd_pushing_prob_with_sweet;	// BD player probability to push rocks after eating sweet
  boolean bd_push_heavy_rock_with_sweet;// BD player can push heavy rocks after eating sweet
  boolean bd_magic_wall_zero_infinite;	// BD magic wall with timer of zero runs infinitely
  boolean bd_magic_wall_wait_hatching;	// BD magic wall waits for player's birth
  boolean bd_magic_wall_stops_amoeba;	// BD magic wall can stop amoeba and turn to diamonds
  boolean bd_magic_wall_break_scan;	// BD magic wall setting to implement buggy BD1 behaviour
  int bd_magic_wall_time;		// BD magic wall time
  int bd_magic_wall_diamond_to;		// BD magic wall turns diamonds to specified element
  int bd_magic_wall_rock_to;		// BD magic wall turns rocks to specified element
  int bd_magic_wall_heavy_rock_to;	// BD magic wall turns heavy rocks to specified element
  int bd_magic_wall_light_rock_to;	// BD magic wall turns light rocks to specified element
  int bd_magic_wall_nut_to;		// BD magic wall turns nuts to specified element
  int bd_magic_wall_nitro_pack_to;	// BD magic wall turns nitro packs to specified element
  int bd_magic_wall_flying_diamond_to;	// BD magic wall turns flying diamonds to specified element
  int bd_magic_wall_flying_rock_to;	// BD magic wall turns flying rocks to specified element
  boolean bd_amoeba_wait_for_hatching;	// BD amoeba waits for player's birth
  boolean bd_amoeba_start_immediately;	// BD amoeba growth starts immediately
  boolean bd_amoeba_2_explode_by_amoeba;// BD amoeba 2 explodes if touched by BD amoeba
  int bd_amoeba_1_threshold_too_big;	// BD amoeba 1 turns to stones if threshold reached
  int bd_amoeba_1_slow_growth_time;	// BD amoeba 1 slow growth time (in seconds)
  int bd_amoeba_1_slow_growth_rate;	// BD amoeba 1 slow growth rate (in percent)
  int bd_amoeba_1_fast_growth_rate;	// BD amoeba 1 fast growth rate (in percent)
  int bd_amoeba_1_content_too_big;	// BD amoeba 1 changes to this element if too big
  int bd_amoeba_1_content_enclosed;	// BD amoeba 1 changes to this element if enclosed
  int bd_amoeba_2_threshold_too_big;	// BD amoeba 2 turns to stones if threshold reached
  int bd_amoeba_2_slow_growth_time;	// BD amoeba 2 slow growth time (in seconds)
  int bd_amoeba_2_slow_growth_rate;	// BD amoeba 2 slow growth rate (in percent)
  int bd_amoeba_2_fast_growth_rate;	// BD amoeba 2 fast growth rate (in percent)
  int bd_amoeba_2_content_too_big;	// BD amoeba 2 changes to this element if too big
  int bd_amoeba_2_content_enclosed;	// BD amoeba 2 changes to this element if enclosed
  int bd_amoeba_2_content_exploding;	// BD amoeba 2 changes to this element if exploding
  int bd_amoeba_2_content_looks_like;	// BD amoeba 2 looks like this other game element
  int bd_clock_extra_time;		// BD engine extra time when collecting clock
  boolean bd_voodoo_collects_diamonds;	// BD voodoo doll can collect diamonds for the player
  boolean bd_voodoo_hurt_kills_player;	// BD voodoo doll hurt in any way, player is killed
  boolean bd_voodoo_dies_by_rock;	// BD voodoo doll can be killed by a falling rock
  boolean bd_voodoo_vanish_by_explosion;// BD voodoo doll can be destroyed by explosions
  int bd_voodoo_penalty_time;		// BD engine penalty time when voodoo doll destroyed
  boolean bd_slime_is_predictable;	// BD slime uses predictable random number generator
  boolean bd_slime_correct_random;	// BD slime needs corrected random number generator
  int bd_slime_permeability_rate;	// BD slime permeability rate for unpredictable slime
  int bd_slime_permeability_bits_c64;	// BD slime permeability bits for predictable slime
  int bd_slime_random_seed_c64;		// BD slime random number seed for predictable slime
  int bd_slime_eats_element_1;		// BD slime can eat and convert this game element
  int bd_slime_converts_to_element_1;	// BD slime can convert eaten element to this game element
  int bd_slime_eats_element_2;		// BD slime can eat and convert this game element
  int bd_slime_converts_to_element_2;	// BD slime can convert eaten element to this game element
  int bd_slime_eats_element_3;		// BD slime can eat and convert this game element
  int bd_slime_converts_to_element_3;	// BD slime can convert eaten element to this game element
  int bd_cave_random_seed_c64;		// BD cave random number seed for predictable slime
  int bd_acid_eats_element;		// BD acid eats this game element when spreading
  int bd_acid_spread_rate;		// BD acid probability of spreading (in percent)
  int bd_acid_turns_to_element;		// BD acid target element after spreading
  int bd_biter_move_delay;		// BD biter delay between movements (in BD frames)
  int bd_biter_eats_element;		// BD biter eats this game element when moving
  int bd_bubble_converts_by_element;	// BD bubble converts to clock by touching this element
  boolean bd_change_expanding_wall;	// BD expanding wall direction is changed if enabled
  boolean bd_replicators_active;	// BD replicators start in active state if enabled
  int bd_replicator_create_delay;	// BD replicator delay between replications (in BD frames)
  boolean bd_conveyor_belts_active;	// BD conveyor belts start in active state if enabled
  boolean bd_conveyor_belts_changed;	// BD conveyor belts direction is changed if enabled
  boolean bd_conveyor_belts_buggy;	// BD conveyor belts setting to implement buggy behaviour
  boolean bd_water_cannot_flow_down;	// BD water does not flow downwards if enabled
  int bd_nut_content;			// BD nut contains the specified game element
  int bd_hammer_walls_break_delay;	// BD hammer time for breaking walls (in BD frames)
  boolean bd_hammer_walls_reappear;	// BD hammered walls are reappearing after some delay
  int bd_hammer_walls_reappear_delay;	// BD hammer time for reappearing walls (in BD frames)
  boolean bd_infinite_rockets;		// BD rocket launcher has infinite number of rockets
  boolean bd_buggy_teleporter;		// BD teleporter setting to implement buggy behaviour
  int bd_num_skeletons_needed_for_pot;	// BD skeletons amount must be collected to use a pot
  int bd_skeleton_worth_num_diamonds;	// BD skeleton collected is worth this number of diamonds
  int bd_expanding_wall_looks_like;	// BD expanding wall looks like this other game element
  int bd_sand_looks_like;		// BD sand looks like this other game element
  boolean bd_creatures_start_backwards;	// BD creatures start moving in opposite direction
  boolean bd_creatures_turn_on_hatching;// BD creatures change direction after hatching
  int bd_creatures_auto_turn_delay;	// BD creatures change direction after delay (in seconds)
  int bd_gravity_direction;		// BD engine initial gravity direction
  boolean bd_gravity_switch_active;	// BD engine gravity switch starts in active state
  int bd_gravity_switch_delay;		// BD engine gravity change delay for switch (in seconds)
  boolean bd_gravity_affects_all;	// BD engine gravity affects all falling objects
  int bd_rock_turns_to_on_falling;	// BD rock changes to specified element when falling
  int bd_rock_turns_to_on_impact;	// BD rock changes to specified element on impact
  int bd_diamond_turns_to_on_falling;	// BD diamond changes to specified element when falling
  int bd_diamond_turns_to_on_impact;	// BD diamond changes to specified element on impact
  int bd_firefly_1_explodes_to;		// BD firefly 1 explodes to specified element
  int bd_firefly_2_explodes_to;		// BD firefly 2 explodes to specified element
  int bd_butterfly_1_explodes_to;	// BD butterfly 1 explodes to specified element
  int bd_butterfly_2_explodes_to;	// BD butterfly 2 explodes to specified element
  int bd_stonefly_explodes_to;		// BD stonefly explodes to specified element
  int bd_dragonfly_explodes_to;		// BD dragonfly explodes to specified element
  int bd_diamond_birth_turns_to;	// BD diamond birth changes to specified element
  int bd_bomb_explosion_turns_to;	// BD bomb explosion changes to specified element
  int bd_nitro_explosion_turns_to;	// BD nitro pack explosion changes to specified element
  int bd_explosion_turns_to;		// BD other explosions change to specified element
  int bd_explosion_3_turns_to;		// BD other explosions change to specified element, stage 3
  int bd_color[MAX_LEVEL_COLORS];	// BD engine C64-style cave colors
  int bd_base_color[MAX_LEVEL_COLORS];	// BD engine C64-style cave base colors (for gradients)

  boolean em_slippery_gems;		// EM style "gems slip from wall" behaviour
  boolean em_explodes_by_fire;		// EM style chain explosion behaviour
  boolean use_spring_bug;		// for compatibility with old levels
  boolean use_time_orb_bug;		// for compatibility with old levels
  boolean use_life_bugs;		// for compatibility with old levels
  boolean instant_relocation;		// no visual delay when relocating player
  boolean shifted_relocation;		// no level centering when relocating player
  boolean lazy_relocation;		// only redraw off-screen player relocation
  boolean can_pass_to_walkable;		// player can pass to empty or walkable tile
  boolean grow_into_diggable;		// amoeba can grow into anything diggable
  boolean sb_fields_needed;		// all Sokoban fields must be solved
  boolean sb_objects_needed;		// all Sokoban objects must be solved
  boolean auto_exit_sokoban;		// automatically finish solved Sokoban levels
  boolean solved_by_one_player;		// level is solved if one player enters exit
  boolean finish_dig_collect;		// only finished dig/collect triggers ce action
  boolean keep_walkable_ce;		// keep walkable CE if it changes to the player

  boolean continuous_snapping;		// repeated snapping without releasing key
  boolean block_snap_field;		// snapping blocks field to show animation
  boolean block_last_field;		// player blocks previous field while moving
  boolean sp_block_last_field;		// player blocks previous field while moving

  // values for MM/DF elements
  boolean mm_laser_red, mm_laser_green, mm_laser_blue;
  boolean df_laser_red, df_laser_green, df_laser_blue;
  int mm_time_fuse;
  int mm_time_bomb;
  int mm_time_ball;
  int mm_time_block;

  int num_mm_ball_contents;
  int mm_ball_choice_mode;
  int mm_ball_content[MAX_MM_BALL_CONTENTS];
  boolean rotate_mm_ball_content;
  boolean explode_mm_ball;

  // ('int' instead of 'boolean' because used as selectbox value in editor)
  int use_step_counter;			// count steps instead of seconds for level

  int time_score_base;			// use time score for 1 or 10 seconds/steps

  short field[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

  boolean use_custom_template;		// use custom properties from template file

  boolean file_has_custom_elements;	// set when level file contains CEs

  int bd_coloring_type;			// type of color template coloring to be used
  int bd_color_type;			// set according to BD colors in level

  boolean no_valid_file;		// set when level file missing or invalid
  boolean no_level_file;		// set when falling back to level template

  boolean changed;			// set when level was changed in the editor

  // runtime flags to handle bugs in old levels (not stored in level file)
  boolean use_action_after_change_bug;
};

struct NetworkLevelInfo
{
  char *leveldir_identifier;		// network levelset identifier

  struct LevelFileInfo file_info;	// file info for level file
  struct LevelFileInfo tmpl_info;	// file info for level template

  boolean use_network_level_files;	// use levels from network server
  boolean use_custom_template;		// use CEs from level template
};

struct GlobalInfo
{
  char *autoplay_leveldir;
  int autoplay_level[MAX_TAPES_PER_SET];
  int autoplay_mode;
  boolean autoplay_all;
  time_t autoplay_time;

  char *patchtapes_mode;
  char *patchtapes_leveldir;
  int patchtapes_level[MAX_TAPES_PER_SET];
  boolean patchtapes_all;

  char *convert_leveldir;
  int convert_level_nr;

  char *dumplevelset_leveldir;

  char *dumplevel_leveldir;
  int dumplevel_level_nr;

  char *dumptape_leveldir;
  int dumptape_level_nr;

  char *create_sketch_images_dir;
  char *create_collect_images_dir;

  int num_toons;

  float frames_per_second;
  boolean show_frames_per_second;

  // global values for fading screens and masking borders
  int border_status;

  // values for global animations
  int anim_status;
  int anim_status_next;

  boolean use_envelope_request;

  char **user_names;
};

struct ElementChangeInfo
{
  boolean can_change;			// use or ignore this change info

  boolean has_event[NUM_CHANGE_EVENTS];	// change events

  int trigger_player;			// player triggering change
  int trigger_side;			// side triggering change
  int trigger_page;			// page triggering change

  int target_element;			// target element after change

  int delay_fixed;			// added frame delay before changed (fixed)
  int delay_random;			// added frame delay before changed (random)
  int delay_frames;			// either 1 (frames) or 50 (seconds; 50 fps)

  int initial_trigger_element;		// initial element triggering change

  struct Content target_content;	// elements for extended change target
  boolean use_target_content;		// use extended change target
  boolean only_if_complete;		// only use complete target content
  boolean use_random_replace;		// use random value for replacing elements
  int random_percentage;		// random value for replacing elements
  int replace_when;			// type of elements that can be replaced

  boolean explode;			// explode instead of change

  boolean has_action;			// execute action on specified condition
  int action_type;			// type of action
  int action_mode;			// mode of action
  int action_arg;			// parameter of action
  int action_element;			// element related to action

  // ---------- internal values used at runtime when playing ----------

  int trigger_element;			// element triggering change

  /* functions that are called before, while and after the change of an
     element -- currently only used for non-custom elements */
  void (*pre_change_function)(int x, int y);
  void (*change_function)(int x, int y);
  void (*post_change_function)(int x, int y);

  short actual_trigger_element;		// element that actually triggered change
  int actual_trigger_x;			// element x position that triggered change
  int actual_trigger_y;			// element y position that triggered change
  int actual_trigger_side;		// element side that triggered the change
  int actual_trigger_player;		// player which actually triggered change
  int actual_trigger_player_bits;	// player bits of triggering players
  int actual_trigger_ce_value;		// CE value of element that triggered change
  int actual_trigger_ce_score;		// CE score of element that triggered change

  boolean can_change_or_has_action;	// can_change | has_action

  // ---------- internal values used in level editor ----------

  int direct_action;			// change triggered by actions on element
  int other_action;			// change triggered by other element actions
};

struct ElementGroupInfo
{
  int num_elements;			// number of elements in this group
  int element[MAX_ELEMENTS_IN_GROUP];	// list of elements in this group

  int choice_mode;			// how to choose element from group

  // ---------- internal values used at runtime when playing ----------

  /* the following is the same as above, but with recursively resolved group
     elements (group elements may also contain further group elements!) */
  int num_elements_resolved;
  short element_resolved[NUM_FILE_ELEMENTS];

  int choice_pos;			// current element choice position
};

struct ElementNameInfo
{
  // ---------- token and description strings ----------

  char *token_name;			// element token used in config files
  char *class_name;			// element class used in config files
  char *editor_description;		// pre-defined description for level editor
};

struct ElementInfo
{
  // ---------- token and description strings ----------

  char *token_name;			// element token used in config files
  char *class_name;			// element class used in config files
  char *editor_description;		// pre-defined description for level editor
  char *custom_description;		// alternative description from config file
  char description[MAX_ELEMENT_NAME_LEN + 1];	// for custom/group elements

  // ---------- graphic and sound definitions ----------

  int graphic[NUM_ACTIONS];		// default graphics for several actions
  int direction_graphic[NUM_ACTIONS][NUM_DIRECTIONS_FULL];
					// special graphics for left/right/up/down

  int crumbled[NUM_ACTIONS];		// crumbled graphics for several actions
  int direction_crumbled[NUM_ACTIONS][NUM_DIRECTIONS_FULL];
					// crumbled graphics for left/right/up/down

  int special_graphic[NUM_SPECIAL_GFX_ARGS];
					// special graphics for certain screens

  int sound[NUM_ACTIONS];		// default sounds for several actions

  // ---------- special element property values ----------

  unsigned int properties[NUM_EP_BITFIELDS];	// element base properties

  boolean use_gfx_element;		// use custom graphic element
  int gfx_element_initial;		// initial optional custom graphic element

  int access_direction;			// accessible from which direction

  int collect_score_initial;		// initial score value for collecting
  int collect_count_initial;		// initial count value for collecting

  int ce_value_fixed_initial;		// initial value for custom variable (fix)
  int ce_value_random_initial;		// initial value for custom variable (rnd)
  boolean use_last_ce_value;		// use value from element before change

  int push_delay_fixed;			// constant delay before pushing
  int push_delay_random;		// additional random delay before pushing
  int drop_delay_fixed;			// constant delay after dropping
  int drop_delay_random;		// additional random delay after dropping
  int move_delay_fixed;			// constant delay after moving
  int move_delay_random;		// additional random delay after moving
  int step_delay_fixed;			// constant delay while moving
  int step_delay_random;		// additional random delay while moving

  int move_pattern;			// direction movable element moves to
  int move_direction_initial;		// initial direction element moves to
  int move_stepsize;			// step size element moves with

  int move_enter_element;		// element that can be entered (and removed)
  int move_leave_element;		// element that can be left behind
  int move_leave_type;			// change (limited) or leave (unlimited)

  int slippery_type;			// how/where other elements slip away

  struct Content content;		// new elements after explosion

  int explosion_type;			// type of explosion, like 3x3, 3+3 or 1x1
  int explosion_delay;			// duration of explosion of this element
  int ignition_delay;			// delay for explosion by other explosion

  struct ElementChangeInfo *change_page; // actual list of change pages
  struct ElementChangeInfo *change;	 // pointer to current change page

  int num_change_pages;			// actual number of change pages
  int current_change_page;		// currently edited change page

  struct ElementGroupInfo *group;	// pointer to element group info

  boolean has_anim_event;		// element can trigger global animation

  // ---------- internal values used at runtime when playing ----------

  boolean has_change_event[NUM_CHANGE_EVENTS];

  int event_page_nr[NUM_CHANGE_EVENTS]; // page number for each event
  struct ElementChangeInfo *event_page[NUM_CHANGE_EVENTS]; // page for event

  boolean in_group[NUM_GROUP_ELEMENTS];

  int gfx_element;			// runtime optional custom graphic element

  int collect_score;			// runtime score value for collecting

  // count of this element on playfield, calculated after each frame
  int element_count;

  // ---------- internal values used in level editor ----------

  int access_type;			// walkable or passable
  int access_layer;			// accessible over/inside/under
  int access_protected;			// protection against deadly elements
  int walk_to_action;			// diggable/collectible/pushable
  int smash_targets;			// can smash player/enemies/everything
  int deadliness;			// deadly when running/colliding/touching

  boolean can_explode_by_fire;		// element explodes by fire
  boolean can_explode_smashed;		// element explodes when smashed
  boolean can_explode_impact;		// element explodes on impact

  boolean modified_settings;		// set for all modified custom elements
};

struct FontInfo
{
  char *token_name;			// font token used in config files

  int graphic;				// default graphic for this font
  int special_graphic[NUM_SPECIAL_GFX_ARGS];
					// special graphics for certain screens
  int special_bitmap_id[NUM_SPECIAL_GFX_ARGS];
					// internal bitmap ID for special graphics
};

struct GlobalAnimNameInfo
{
  char *token_name;			// global animation token in config files
};

struct GlobalAnimInfo
{
  char *token_name;			// global animation token in config files

  // global animation graphic and control definitions
  int graphic[NUM_GLOBAL_ANIM_PARTS_ALL][NUM_SPECIAL_GFX_ARGS];

  // global animation sound and music definitions
  int sound[NUM_GLOBAL_ANIM_PARTS_ALL][NUM_SPECIAL_GFX_ARGS];
  int music[NUM_GLOBAL_ANIM_PARTS_ALL][NUM_SPECIAL_GFX_ARGS];
};

struct GlobalAnimEventListInfo
{
  int *event_value;
  int num_event_values;
};

struct GlobalAnimEventInfo
{
  struct GlobalAnimEventListInfo **event_list;
  int num_event_lists;
};

struct GraphicInfo
{
  Bitmap **bitmaps;			// bitmaps in all required sizes
  Bitmap *bitmap;			// bitmap in default size

  int src_image_width;			// scaled bitmap size, but w/o small images
  int src_image_height;			// scaled bitmap size, but w/o small images

  int src_x, src_y;			// start position of animation frames
  int width, height;			// width/height of each animation frame

  int offset_x, offset_y;		// x/y offset to next animation frame
  int offset2_x, offset2_y;		// x/y offset to second movement tile

  boolean double_movement;		// animation has second movement tile
  int swap_double_tiles;		// explicitely force or forbid tile swapping

  int anim_frames;
  int anim_frames_per_line;
  int anim_start_frame;
  int anim_delay;			// important: delay of 1 means "no delay"!
  int anim_mode;

  boolean anim_global_sync;
  boolean anim_global_anim_sync;

  int crumbled_like;			// element for cloning crumble graphics
  int diggable_like;			// element for cloning digging graphics

  int border_size;			// border size for "crumbled" graphics

  int scale_up_factor;			// optional factor for scaling image up
  int tile_size;			// optional explicitly defined tile size

  int clone_from;			// graphic for cloning *all* settings

  int init_delay_fixed;			// optional initial delay values for global
  int init_delay_random;		// animations (pause interval before start)
  int init_delay_action;		// optional action called on animation start
  int anim_delay_fixed;			// optional delay values for bored/sleeping
  int anim_delay_random;		// and global animations (animation length)
  int anim_delay_action;		// optional action called on animation end
  int post_delay_fixed;			// optional delay values after bored/global
  int post_delay_random;		// animations (pause before next animation)
  int post_delay_action;		// optional action called after post delay

  int init_event;			// optional event triggering animation start
  int init_event_action;		// optional action called on animation start
  int anim_event;			// optional event triggering animation end
  int anim_event_action;		// optional action called on animation end

  int step_offset;			// optional step offset of toon animations
  int step_xoffset;			// optional step offset of toon animations
  int step_yoffset;			// optional step offset of toon animations
  int step_delay;			// optional step delay of toon animations
  int direction;			// optional move direction of toon animations
  int position;				// optional draw position of toon animations
  int x;				// optional draw position of toon animations
  int y;				// optional draw position of toon animations

  int draw_xoffset;			// optional offset for drawing font chars
  int draw_yoffset;			// optional offset for drawing font chars

  int draw_masked;			// optional setting for drawing envelope gfx
  int draw_order;			// optional draw order for global animations

  int fade_mode;			// optional setting for drawing title screens
  int fade_delay;			// optional setting for drawing title screens
  int post_delay;			// optional setting for drawing title screens
  int auto_delay;			// optional setting for drawing title screens
  int auto_delay_unit;			// optional setting for drawing title screens
  int align, valign;			// optional setting for drawing title screens
  int sort_priority;			// optional setting for drawing title screens

  int class;
  int style;
  int alpha;

  int active_xoffset;
  int active_yoffset;
  int pressed_xoffset;
  int pressed_yoffset;

  int stacked_xfactor;
  int stacked_yfactor;
  int stacked_xoffset;
  int stacked_yoffset;

  boolean color_template;		// optional setting for color template images

  boolean use_image_size;		// use image size as default width and height
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
  char *played_header;

  char *title;
  char *artist;
  char *album;
  char *year;
  char *played;

  int music;

  boolean is_sound;

  struct MusicFileInfo *prev, *next;
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


extern Bitmap			       *bitmap_db_field;
extern Bitmap			       *bitmap_db_door_1;
extern Bitmap			       *bitmap_db_door_2;
extern Bitmap			       *bitmap_db_store_1;
extern Bitmap			       *bitmap_db_store_2;
extern DrawBuffer		       *fieldbuffer;
extern DrawBuffer		       *drawto_field;

extern int				game_status;
extern int				game_status_last_screen;
extern boolean				level_editor_test_game;
extern boolean				score_info_tape_play;
extern boolean				network_playing;

extern int				key_joystick_mapping;

extern short				Tile[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				Last[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				ChangeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				ChangePage[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				CustomValue[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				Back[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean				Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern boolean				Pushed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				ChangeCount[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				ChangeEvent[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				WasJustMoving[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				WasJustFalling[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				CheckCollision[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				CheckImpact[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				AmoebaCnt[MAX_NUM_AMOEBA];
extern short				AmoebaCnt2[MAX_NUM_AMOEBA];
extern short				ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern short				ExplodeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int				RunnerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int				PlayerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern int				GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int				GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int				GfxRandomStatic[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 				GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 				GfxElementEmpty[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int				GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 				GfxDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
extern int 				GfxRedraw[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

extern int				ActiveElement[MAX_NUM_ELEMENTS];
extern int				ActiveButton[NUM_IMAGE_FILES];
extern int				ActiveFont[NUM_FONTS];

extern int				lev_fieldx, lev_fieldy;
extern int				scroll_x, scroll_y;

extern int				WIN_XSIZE, WIN_YSIZE;
extern int				SCR_FIELDX, SCR_FIELDY;
extern int				REAL_SX, REAL_SY;
extern int				SX, SY;
extern int				DX, DY;
extern int				VX, VY;
extern int				EX, EY;
extern int				dDX, dDY;
extern int				FULL_SXSIZE, FULL_SYSIZE;
extern int				SXSIZE, SYSIZE;
extern int				DXSIZE, DYSIZE;
extern int				VXSIZE, VYSIZE;
extern int				EXSIZE, EYSIZE;
extern int				TILESIZE_VAR;

extern int				FADE_SX, FADE_SY;
extern int				FADE_SXSIZE, FADE_SYSIZE;

extern int				FX, FY;
extern int				ScrollStepSize;
extern int				ScreenMovDir, ScreenMovPos, ScreenGfxPos;
extern int				BorderElement;
extern int				MenuFrameDelay;
extern int				GameFrameDelay;
extern int				FfwdFrameDelay;
extern int				BX1, BY1;
extern int				BX2, BY2;
extern int				SBX_Left, SBX_Right;
extern int				SBY_Upper, SBY_Lower;

extern int				TimeFrames, TimePlayed, TimeLeft;
extern int				TapeTimeFrames, TapeTime;

extern boolean				network_player_action_received;

extern int				graphics_action_mapping[];

extern struct LevelInfo			level, level_template;
extern struct ScoreInfo			scores, server_scores;
extern struct TapeInfo			tape;
extern struct GlobalInfo		global;
extern struct BorderInfo		border;
extern struct ViewportInfo		viewport;
extern struct TitleFadingInfo		fading;
extern struct TitleFadingInfo		fading_none;
extern struct TitleFadingInfo		title_initial_first_default;
extern struct TitleFadingInfo		title_initial_default;
extern struct TitleFadingInfo		title_first_default;
extern struct TitleFadingInfo		title_default;
extern struct TitleMessageInfo		titlescreen_initial_first_default;
extern struct TitleMessageInfo		titlescreen_initial_first[];
extern struct TitleMessageInfo		titlescreen_initial_default;
extern struct TitleMessageInfo		titlescreen_initial[];
extern struct TitleMessageInfo		titlescreen_first_default;
extern struct TitleMessageInfo		titlescreen_first[];
extern struct TitleMessageInfo		titlescreen_default;
extern struct TitleMessageInfo		titlescreen[];
extern struct TitleMessageInfo		titlemessage_initial_first_default;
extern struct TitleMessageInfo		titlemessage_initial_first[];
extern struct TitleMessageInfo		titlemessage_initial_default;
extern struct TitleMessageInfo		titlemessage_initial[];
extern struct TitleMessageInfo		titlemessage_first_default;
extern struct TitleMessageInfo		titlemessage_first[];
extern struct TitleMessageInfo		titlemessage_default;
extern struct TitleMessageInfo		titlemessage[];
extern struct TitleMessageInfo		readme;
extern struct InitInfo			init, init_last;
extern struct MenuInfo			menu;
extern struct DoorInfo			door_1, door_2;
extern struct PreviewInfo		preview;
extern struct EditorInfo		editor;
extern struct ElementInfo		element_info[];
extern struct ElementNameInfo		element_name_info[];
extern struct ElementActionInfo		element_action_info[];
extern struct ElementDirectionInfo	element_direction_info[];
extern struct SpecialSuffixInfo		special_suffix_info[];
extern struct TokenIntPtrInfo		image_config_vars[];
extern struct TokenIntPtrInfo		sound_config_vars[];
extern struct FontInfo			font_info[];
extern struct GlobalAnimInfo		global_anim_info[];
extern struct GlobalAnimNameInfo	global_anim_name_info[];
extern struct GlobalAnimEventInfo	global_anim_event_info;
extern struct MusicPrefixInfo		music_prefix_info[];
extern struct GraphicInfo	       *graphic_info;
extern struct SoundInfo		       *sound_info;
extern struct MusicInfo		       *music_info;
extern struct MusicFileInfo	       *music_file_info;
extern struct HelpAnimInfo	       *helpanim_info;
extern SetupFileHash		       *helptext_info;
extern SetupFileHash		       *image_config_hash;
extern SetupFileHash		       *sound_config_hash;
extern SetupFileHash		       *element_token_hash;
extern SetupFileHash		       *graphic_token_hash;
extern SetupFileHash		       *font_token_hash;
extern SetupFileHash		       *hide_setup_hash;
extern SetupFileHash		       *anim_url_hash;
extern struct ConfigTypeInfo		image_config_suffix[];
extern struct ConfigTypeInfo		sound_config_suffix[];
extern struct ConfigTypeInfo		music_config_suffix[];
extern struct ConfigInfo		image_config[];
extern struct ConfigInfo		sound_config[];
extern struct ConfigInfo		music_config[];
extern struct ConfigInfo		helpanim_config[];
extern struct ConfigInfo		helptext_config[];

#endif	// MAIN_H
