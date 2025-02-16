// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// files.c
// ============================================================================

#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>

#include "libgame/libgame.h"

#include "files.h"
#include "init.h"
#include "screens.h"
#include "editor.h"
#include "tools.h"
#include "tape.h"
#include "config.h"
#include "api.h"

#define ENABLE_UNUSED_CODE	0	// currently unused functions
#define ENABLE_HISTORIC_CHUNKS	0	// only for historic reference
#define ENABLE_RESERVED_CODE	0	// reserved for later use

#define CHUNK_ID_LEN		4	// IFF style chunk id length
#define CHUNK_SIZE_UNDEFINED	0	// undefined chunk size == 0
#define CHUNK_SIZE_NONE		-1	// do not write chunk size

#define LEVEL_CHUNK_NAME_SIZE	MAX_LEVEL_NAME_LEN
#define LEVEL_CHUNK_AUTH_SIZE	MAX_LEVEL_AUTHOR_LEN

#define LEVEL_CHUNK_VERS_SIZE	8	// size of file version chunk
#define LEVEL_CHUNK_DATE_SIZE	4	// size of file date chunk
#define LEVEL_CHUNK_HEAD_SIZE	80	// size of level file header
#define LEVEL_CHUNK_HEAD_UNUSED	0	// unused level header bytes
#define LEVEL_CHUNK_CNT2_SIZE	160	// size of level CNT2 chunk
#define LEVEL_CHUNK_CNT2_UNUSED	11	// unused CNT2 chunk bytes
#define LEVEL_CHUNK_CNT3_HEADER	16	// size of level CNT3 header
#define LEVEL_CHUNK_CNT3_UNUSED	10	// unused CNT3 chunk bytes
#define LEVEL_CPART_CUS3_SIZE	134	// size of CUS3 chunk part
#define LEVEL_CPART_CUS3_UNUSED	15	// unused CUS3 bytes / part
#define LEVEL_CHUNK_GRP1_SIZE	74	// size of level GRP1 chunk

// (element number, number of change pages, change page number)
#define LEVEL_CHUNK_CUSX_UNCHANGED	(2 + (1 + 1) + (1 + 1))

// (element number only)
#define LEVEL_CHUNK_GRPX_UNCHANGED	2
#define LEVEL_CHUNK_EMPX_UNCHANGED	2
#define LEVEL_CHUNK_NOTE_UNCHANGED	2

// (nothing at all if unchanged)
#define LEVEL_CHUNK_ELEM_UNCHANGED	0

#define TAPE_CHUNK_VERS_SIZE	8	// size of standard tape versions chunk
#define TAPE_CHUNK_VERX_SIZE	8	// size of extended tape versions chunk
#define TAPE_CHUNK_HEAD_SIZE	20	// size of tape file header
#define TAPE_CHUNK_SCRN_SIZE	2	// size of screen size chunk

#define SCORE_CHUNK_VERS_SIZE	8	// size of file version chunk

#define LEVEL_CHUNK_CNT3_SIZE(x)	 (LEVEL_CHUNK_CNT3_HEADER + (x))
#define LEVEL_CHUNK_CUS3_SIZE(x)	 (2 + (x) * LEVEL_CPART_CUS3_SIZE)
#define LEVEL_CHUNK_CUS4_SIZE(x)	 (96 + (x) * 48)

// file identifier strings
#define LEVEL_COOKIE_TMPL		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_x.x"
#define TAPE_COOKIE_TMPL		"ROCKSNDIAMONDS_TAPE_FILE_VERSION_x.x"
#define SCORE_COOKIE_TMPL		"ROCKSNDIAMONDS_SCORE_FILE_VERSION_x.x"

// values for deciding when (not) to save configuration data
#define SAVE_CONF_NEVER			0
#define SAVE_CONF_ALWAYS		1
#define SAVE_CONF_WHEN_CHANGED		-1

// values for chunks using micro chunks
#define CONF_MASK_1_BYTE		0x00
#define CONF_MASK_2_BYTE		0x40
#define CONF_MASK_4_BYTE		0x80
#define CONF_MASK_MULTI_BYTES		0xc0

#define CONF_MASK_BYTES			0xc0
#define CONF_MASK_TOKEN			0x3f

#define CONF_VALUE_1_BYTE(x)		(CONF_MASK_1_BYTE	| (x))
#define CONF_VALUE_2_BYTE(x)		(CONF_MASK_2_BYTE	| (x))
#define CONF_VALUE_4_BYTE(x)		(CONF_MASK_4_BYTE	| (x))
#define CONF_VALUE_MULTI_BYTES(x)	(CONF_MASK_MULTI_BYTES	| (x))

// these definitions are just for convenience of use and readability
#define CONF_VALUE_8_BIT(x)		CONF_VALUE_1_BYTE(x)
#define CONF_VALUE_16_BIT(x)		CONF_VALUE_2_BYTE(x)
#define CONF_VALUE_32_BIT(x)		CONF_VALUE_4_BYTE(x)
#define CONF_VALUE_BYTES(x)		CONF_VALUE_MULTI_BYTES(x)

#define CONF_VALUE_NUM_BYTES(x)		((x) == CONF_MASK_1_BYTE ? 1 :	\
					 (x) == CONF_MASK_2_BYTE ? 2 :	\
					 (x) == CONF_MASK_4_BYTE ? 4 : 0)

#define CONF_CONTENT_NUM_ELEMENTS	(3 * 3)
#define CONF_CONTENT_NUM_BYTES		(CONF_CONTENT_NUM_ELEMENTS * 2)
#define CONF_ELEMENT_NUM_BYTES		(2)

#define CONF_ENTITY_NUM_BYTES(t)	((t) == TYPE_ELEMENT ||		\
					 (t) == TYPE_ELEMENT_LIST ?	\
					 CONF_ELEMENT_NUM_BYTES :	\
					 (t) == TYPE_CONTENT ||		\
					 (t) == TYPE_CONTENT_LIST ?	\
					 CONF_CONTENT_NUM_BYTES : 1)

#define CONF_ELEMENT_BYTE_POS(i)	((i) * CONF_ELEMENT_NUM_BYTES)
#define CONF_ELEMENTS_ELEMENT(b, i)	((b[CONF_ELEMENT_BYTE_POS(i)] << 8) | \
					(b[CONF_ELEMENT_BYTE_POS(i) + 1]))

#define CONF_CONTENT_ELEMENT_POS(c,x,y)	((c) * CONF_CONTENT_NUM_ELEMENTS +    \
					 (y) * 3 + (x))
#define CONF_CONTENT_BYTE_POS(c,x,y)	(CONF_CONTENT_ELEMENT_POS(c,x,y) *    \
					 CONF_ELEMENT_NUM_BYTES)
#define CONF_CONTENTS_ELEMENT(b,c,x,y) ((b[CONF_CONTENT_BYTE_POS(c,x,y)]<< 8)|\
					(b[CONF_CONTENT_BYTE_POS(c,x,y) + 1]))

// temporary variables used to store pointers to structure members
static struct LevelInfo li;
static struct ElementInfo xx_ei, yy_ei;
static struct ElementChangeInfo xx_change;
static struct ElementGroupInfo xx_group;
static struct EnvelopeInfo xx_envelope;
static unsigned int xx_event_bits[NUM_CE_BITFIELDS];
static char xx_default_description[MAX_ELEMENT_NAME_LEN + 1];
static int xx_num_contents;
static int xx_current_change_page;
static char xx_default_string_empty[1] = "";
static int xx_string_length_unused;

struct LevelFileConfigInfo
{
  int element;			// element for which data is to be stored
  int save_type;		// save data always, never or when changed
  int data_type;		// data type (used internally, not stored)
  int conf_type;		// micro chunk identifier (stored in file)

  // (mandatory)
  void *value;			// variable that holds the data to be stored
  int default_value;		// initial default value for this variable

  // (optional)
  void *value_copy;		// variable that holds the data to be copied
  void *num_entities;		// number of entities for multi-byte data
  int default_num_entities;	// default number of entities for this data
  int max_num_entities;		// maximal number of entities for this data
  char *default_string;		// optional default string for string data
};

static struct LevelFileConfigInfo chunk_config_INFO[] =
{
  // ---------- values not related to single elements -------------------------

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.game_engine_type,		GAME_ENGINE_TYPE_RND
  },
  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.fieldx,				STD_LEV_FIELDX
  },
  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.fieldy,				STD_LEV_FIELDY
  },
  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(3),
    &li.time,				100
  },
  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(4),
    &li.gems_needed,			0
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(2),
    &li.random_seed,			0
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.use_step_counter,		FALSE
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(4),
    &li.wind_direction_initial,		MV_NONE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(5),
    &li.em_slippery_gems,		FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(6),
    &li.use_custom_template,		FALSE
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(1),
    &li.can_move_into_acid_bits,	~0	// default: everything can
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(7),
    &li.dont_collide_with_bits,		~0	// default: always deadly
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.em_explodes_by_fire,		FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(5),
    &li.score[SC_TIME_BONUS],		1
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.auto_exit_sokoban,		FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.auto_count_gems,		FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.solved_by_one_player,		FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(12),
    &li.time_score_base,		1
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.rate_time_over_score,		FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.bd_intermission,		FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(15),
    &li.bd_scheduling_type,		GD_SCHEDULING_MILLISECONDS
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(16),
    &li.bd_pal_timing,			FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(6),
    &li.bd_cycle_delay_ms,		160
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(17),
    &li.bd_cycle_delay_c64,		0
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(18),
    &li.bd_hatching_delay_cycles,	21
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(19),
    &li.bd_hatching_delay_seconds,	2
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(20),
    &li.bd_line_shifting_borders,	FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(21),
    &li.bd_scan_first_and_last_row,	TRUE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(22),
    &li.bd_short_explosions,		TRUE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(23),
    &li.bd_cave_random_seed_c64,	0
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(24),
    &li.bd_intermission_clipped,	FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(25),
    &li.bd_coloring_type,		GD_COLORING_TYPE_GRADIENTS
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(4),
    &li.bd_color[0],			GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(5),
    &li.bd_color[1],			GD_C64_COLOR_ORANGE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(6),
    &li.bd_color[2],			GD_C64_COLOR_GRAY1
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(7),
    &li.bd_color[3],			GD_C64_COLOR_WHITE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(8),
    &li.bd_color[4],			GD_C64_COLOR_GREEN
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(9),
    &li.bd_color[5],			GD_C64_COLOR_BLUE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(10),
    &li.bd_color[6],			GD_C64_COLOR_GRAY2
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(11),
    &li.bd_color[7],			GD_C64_COLOR_WHITE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(12),
    &li.bd_base_color[0],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(13),
    &li.bd_base_color[1],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(14),
    &li.bd_base_color[2],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(15),
    &li.bd_base_color[3],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(16),
    &li.bd_base_color[4],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(17),
    &li.bd_base_color[5],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(18),
    &li.bd_base_color[6],		GD_C64_COLOR_BLACK
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(19),
    &li.bd_base_color[7],		GD_C64_COLOR_BLACK
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_ELEM[] =
{
  // (these values are the same for each player)
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.block_last_field,		FALSE	// default case for EM levels
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.sp_block_last_field,		TRUE	// default case for SP levels
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.instant_relocation,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &li.can_pass_to_walkable,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(5),
    &li.block_snap_field,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(6),
    &li.continuous_snapping,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(12),
    &li.shifted_relocation,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(15),
    &li.lazy_relocation,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(16),
    &li.finish_dig_collect,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(17),
    &li.keep_walkable_ce,		FALSE
  },

  // (these values are different for each player)
  {
    EL_PLAYER_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[0],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[0],	1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[0][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[0],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_PLAYER_2,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[1],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[1],	FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[1],		FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[1],		EL_PLAYER_2
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[1],		FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[1],		EL_PLAYER_2
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[1],	FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[1],		EL_PLAYER_2
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[1],	FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[1],	1
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[1][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[1],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_PLAYER_3,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[2],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[2],	FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[2],		FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[2],		EL_PLAYER_3
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[2],		FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[2],		EL_PLAYER_3
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[2],	FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[2],		EL_PLAYER_3
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[2],	FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[2],	1
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[2][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[2],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_PLAYER_4,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[3],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[3],	FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[3],		FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[3],		EL_PLAYER_4
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[3],		FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[3],		EL_PLAYER_4
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[3],	FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[3],		EL_PLAYER_4
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[3],	FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[3],	1
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[3][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[3],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  // (these values are only valid for BD style levels)
  // (some values for BD style amoeba following below)
  {
    EL_BDX_PLAYER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_diagonal_movements,		FALSE
  },
  {
    EL_BDX_PLAYER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_topmost_player_active,	TRUE
  },
  {
    EL_BDX_PLAYER,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.bd_pushing_prob,		25
  },
  {
    EL_BDX_PLAYER,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(4),
    &li.bd_pushing_prob_with_sweet,	100
  },
  {
    EL_BDX_PLAYER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(5),
    &li.bd_push_heavy_rock_with_sweet,	FALSE
  },
  {
    EL_BDX_PLAYER,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_snap_element,		EL_EMPTY
  },

  {
    EL_BDX_SAND,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_sand_looks_like,		EL_BDX_SAND
  },

  {
    EL_BDX_ROCK,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_rock_turns_to_on_falling,	EL_BDX_ROCK_FALLING
  },
  {
    EL_BDX_ROCK,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.bd_rock_turns_to_on_impact,	EL_BDX_ROCK
  },

  {
    EL_BDX_DIAMOND,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_DIAMOND_EXTRA],	20
  },
  {
    EL_BDX_DIAMOND,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.bd_diamond_turns_to_on_falling,	EL_BDX_DIAMOND_FALLING
  },
  {
    EL_BDX_DIAMOND,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.bd_diamond_turns_to_on_impact,	EL_BDX_DIAMOND
  },

  {
    EL_BDX_FIREFLY_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_firefly_1_explodes_to,	EL_BDX_EXPLODING_1
  },

  {
    EL_BDX_FIREFLY_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_firefly_2_explodes_to,	EL_BDX_EXPLODING_1
  },

  {
    EL_BDX_BUTTERFLY_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_butterfly_1_explodes_to,	EL_BDX_DIAMOND_GROWING_1
  },

  {
    EL_BDX_BUTTERFLY_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_butterfly_2_explodes_to,	EL_BDX_DIAMOND_GROWING_1
  },

  {
    EL_BDX_STONEFLY,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_stonefly_explodes_to,	EL_BDX_ROCK_GROWING_1
  },

  {
    EL_BDX_DRAGONFLY,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_dragonfly_explodes_to,	EL_BDX_EXPLODING_1
  },

  {
    EL_BDX_DIAMOND_GROWING_5,	-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_diamond_birth_turns_to,	EL_BDX_DIAMOND
  },

  {
    EL_BDX_BOMB_EXPLODING_4,		-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_bomb_explosion_turns_to,	EL_BDX_WALL
  },

  {
    EL_BDX_NITRO_PACK_EXPLODING_4,	-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_nitro_explosion_turns_to,	EL_EMPTY
  },

  {
    EL_BDX_EXPLODING_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_explosion_3_turns_to,	EL_BDX_EXPLODING_4
  },
  {
    EL_BDX_EXPLODING_5,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_explosion_turns_to,		EL_EMPTY
  },

  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_magic_wall_wait_hatching,	FALSE
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_magic_wall_stops_amoeba,	TRUE
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.bd_magic_wall_zero_infinite,	TRUE
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &li.bd_magic_wall_break_scan,	FALSE
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.bd_magic_wall_time,		999
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.bd_magic_wall_diamond_to,	EL_BDX_ROCK_FALLING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.bd_magic_wall_rock_to,		EL_BDX_DIAMOND_FALLING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(4),
    &li.bd_magic_wall_heavy_rock_to,	EL_BDX_NITRO_PACK_FALLING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(5),
    &li.bd_magic_wall_light_rock_to,	EL_BDX_LIGHT_ROCK_FALLING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(6),
    &li.bd_magic_wall_nut_to,		EL_BDX_NUT_FALLING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(7),
    &li.bd_magic_wall_nitro_pack_to,	EL_BDX_HEAVY_ROCK_FALLING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(8),
    &li.bd_magic_wall_flying_diamond_to, EL_BDX_FLYING_ROCK_FLYING
  },
  {
    EL_BDX_MAGIC_WALL,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(9),
    &li.bd_magic_wall_flying_rock_to,	EL_BDX_FLYING_DIAMOND_FLYING
  },

  {
    EL_BDX_CLOCK,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_clock_extra_time,		30
  },

  {
    EL_BDX_VOODOO_DOLL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_voodoo_collects_diamonds,	FALSE
  },
  {
    EL_BDX_VOODOO_DOLL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_voodoo_hurt_kills_player,	FALSE
  },
  {
    EL_BDX_VOODOO_DOLL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.bd_voodoo_dies_by_rock,		FALSE
  },
  {
    EL_BDX_VOODOO_DOLL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &li.bd_voodoo_vanish_by_explosion,	TRUE
  },
  {
    EL_BDX_VOODOO_DOLL,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(5),
    &li.bd_voodoo_penalty_time,		30
  },

  {
    EL_BDX_SLIME,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_slime_is_predictable,	TRUE
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.bd_slime_permeability_rate,	100
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.bd_slime_permeability_bits_c64,	0
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(1),
    &li.bd_slime_random_seed_c64,	-1
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_slime_eats_element_1,	EL_BDX_DIAMOND
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.bd_slime_converts_to_element_1,	EL_BDX_DIAMOND_FALLING
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.bd_slime_eats_element_2,	EL_BDX_ROCK
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(4),
    &li.bd_slime_converts_to_element_2,	EL_BDX_ROCK_FALLING
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(5),
    &li.bd_slime_eats_element_3,	EL_BDX_NUT
  },
  {
    EL_BDX_SLIME,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(6),
    &li.bd_slime_converts_to_element_3,	EL_BDX_NUT_FALLING
  },

  {
    EL_BDX_ACID,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_acid_eats_element,		EL_BDX_SAND
  },
  {
    EL_BDX_ACID,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_acid_spread_rate,		3
  },
  {
    EL_BDX_ACID,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.bd_acid_turns_to_element,	EL_BDX_EXPLODING_3
  },

  {
    EL_BDX_BITER,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_biter_move_delay,		0
  },
  {
    EL_BDX_BITER,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_biter_eats_element,		EL_BDX_DIAMOND
  },

  {
    EL_BDX_BUBBLE,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_bubble_converts_by_element,	EL_BDX_VOODOO_DOLL
  },

  {
    EL_BDX_EXPANDABLE_WALL_ANY,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_change_expanding_wall,	FALSE
  },
  {
    EL_BDX_EXPANDABLE_WALL_ANY,		-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_expanding_wall_looks_like,	EL_BDX_EXPANDABLE_WALL_ANY
  },

  {
    EL_BDX_REPLICATOR,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_replicators_active,		TRUE
  },
  {
    EL_BDX_REPLICATOR,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.bd_replicator_create_delay,	4
  },

  {
    EL_BDX_CONVEYOR_LEFT,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_conveyor_belts_active,	TRUE
  },
  {
    EL_BDX_CONVEYOR_LEFT,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_conveyor_belts_changed,	FALSE
  },
  {
    EL_BDX_CONVEYOR_LEFT,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.bd_conveyor_belts_buggy,	FALSE
  },

  {
    EL_BDX_WATER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_water_cannot_flow_down,	FALSE
  },

  {
    EL_BDX_NUT,				-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.bd_nut_content,			EL_BDX_NUT_BREAKING_1
  },

  {
    EL_BDX_PNEUMATIC_HAMMER,		-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_hammer_walls_break_delay,	5
  },
  {
    EL_BDX_PNEUMATIC_HAMMER,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_hammer_walls_reappear,	FALSE
  },
  {
    EL_BDX_PNEUMATIC_HAMMER,		-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.bd_hammer_walls_reappear_delay,	100
  },

  {
    EL_BDX_ROCKET_LAUNCHER,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_infinite_rockets,		FALSE
  },

  {
    EL_BDX_TELEPORTER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_buggy_teleporter,		FALSE
  },

  {
    EL_BDX_SKELETON,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_num_skeletons_needed_for_pot, 5
  },
  {
    EL_BDX_SKELETON,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.bd_skeleton_worth_num_diamonds,	0
  },

  {
    EL_BDX_CREATURE_SWITCH,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.bd_creatures_start_backwards,	FALSE
  },
  {
    EL_BDX_CREATURE_SWITCH,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_creatures_turn_on_hatching,	FALSE
  },
  {
    EL_BDX_CREATURE_SWITCH,		-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.bd_creatures_auto_turn_delay,	0
  },

  {
    EL_BDX_GRAVITY_SWITCH,		-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_gravity_direction,		GD_MV_DOWN
  },
  {
    EL_BDX_GRAVITY_SWITCH,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.bd_gravity_switch_active,	FALSE
  },
  {
    EL_BDX_GRAVITY_SWITCH,		-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.bd_gravity_switch_delay,	10
  },
  {
    EL_BDX_GRAVITY_SWITCH,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &li.bd_gravity_affects_all,		TRUE
  },

  // (the following values are related to various game elements)

  {
    EL_EMERALD,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_EMERALD],		10
  },

  {
    EL_DIAMOND,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_DIAMOND],		10
  },

  {
    EL_BUG,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_BUG],			10
  },

  {
    EL_SPACESHIP,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_SPACESHIP],		10
  },

  {
    EL_PACMAN,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_PACMAN],		10
  },

  {
    EL_NUT,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_NUT],			10
  },

  {
    EL_DYNAMITE,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_DYNAMITE],		10
  },

  {
    EL_KEY_1,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_KEY],			10
  },

  {
    EL_PEARL,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_PEARL],		10
  },

  {
    EL_CRYSTAL,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_CRYSTAL],		10
  },

  {
    EL_BD_AMOEBA,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.amoeba_content,			EL_DIAMOND
  },
  {
    EL_BD_AMOEBA,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.amoeba_speed,			10
  },
  {
    EL_BD_AMOEBA,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.grow_into_diggable,		TRUE
  },

  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.bd_amoeba_1_threshold_too_big,	200
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.bd_amoeba_1_slow_growth_time,	200
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.bd_amoeba_1_content_too_big,	EL_BDX_ROCK
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(4),
    &li.bd_amoeba_1_content_enclosed,	EL_BDX_DIAMOND
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_amoeba_1_slow_growth_rate,	3
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.bd_amoeba_1_fast_growth_rate,	25
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.bd_amoeba_wait_for_hatching,	FALSE
  },
  {
    EL_BDX_AMOEBA_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &li.bd_amoeba_start_immediately,	TRUE
  },

  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.bd_amoeba_2_threshold_too_big,	200
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.bd_amoeba_2_slow_growth_time,	200
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.bd_amoeba_2_content_too_big,	EL_BDX_ROCK
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(4),
    &li.bd_amoeba_2_content_enclosed,	EL_BDX_DIAMOND
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(5),
    &li.bd_amoeba_2_content_exploding,	EL_EMPTY
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(6),
    &li.bd_amoeba_2_content_looks_like,	EL_BDX_AMOEBA_2
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.bd_amoeba_2_slow_growth_rate,	3
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.bd_amoeba_2_fast_growth_rate,	25
  },
  {
    EL_BDX_AMOEBA_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.bd_amoeba_2_explode_by_amoeba,	TRUE
  },

  {
    EL_YAMYAM,				-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(1),
    &li.yamyam_content,			EL_ROCK, NULL,
    &li.num_yamyam_contents,		4, MAX_ELEMENT_CONTENTS
  },
  {
    EL_YAMYAM,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_YAMYAM],		10
  },

  {
    EL_ROBOT,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_ROBOT],		10
  },
  {
    EL_ROBOT,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.slurp_score,			10
  },

  {
    EL_ROBOT_WHEEL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_wheel,			10
  },

  {
    EL_MAGIC_WALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_magic_wall,		10
  },

  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.game_of_life[0],		2
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.game_of_life[1],		3
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.game_of_life[2],		3
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(4),
    &li.game_of_life[3],		3
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(5),
    &li.use_life_bugs,			FALSE
  },

  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.biomaze[0],			2
  },
  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.biomaze[1],			3
  },
  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.biomaze[2],			3
  },
  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(4),
    &li.biomaze[3],			3
  },

  {
    EL_TIMEGATE_SWITCH,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_timegate,			10
  },

  {
    EL_LIGHT_SWITCH_ACTIVE,		-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_light,			10
  },

  {
    EL_SHIELD_NORMAL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.shield_normal_time,		10
  },
  {
    EL_SHIELD_NORMAL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.score[SC_SHIELD],		10
  },

  {
    EL_SHIELD_DEADLY,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.shield_deadly_time,		10
  },
  {
    EL_SHIELD_DEADLY,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.score[SC_SHIELD],		10
  },

  {
    EL_EXTRA_TIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.extra_time,			10
  },
  {
    EL_EXTRA_TIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.extra_time_score,		10
  },

  {
    EL_TIME_ORB_FULL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_orb_time,			10
  },
  {
    EL_TIME_ORB_FULL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.use_time_orb_bug,		FALSE
  },

  {
    EL_SPRING,				-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.use_spring_bug,			FALSE
  },

  {
    EL_EMC_ANDROID,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.android_move_time,		10
  },
  {
    EL_EMC_ANDROID,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.android_clone_time,		10
  },
  {
    EL_EMC_ANDROID,			SAVE_CONF_NEVER,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.android_clone_element[0],	EL_EMPTY, NULL,
    &li.num_android_clone_elements,	1, MAX_ANDROID_ELEMENTS_OLD
  },
  {
    EL_EMC_ANDROID,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(2),
    &li.android_clone_element[0],	EL_EMPTY, NULL,
    &li.num_android_clone_elements,	1, MAX_ANDROID_ELEMENTS
  },

  {
    EL_EMC_LENSES,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.lenses_score,			10
  },
  {
    EL_EMC_LENSES,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.lenses_time,			10
  },

  {
    EL_EMC_MAGNIFIER,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.magnify_score,			10
  },
  {
    EL_EMC_MAGNIFIER,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.magnify_time,			10
  },

  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.ball_time,			10
  },
  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.ball_random,			FALSE
  },
  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.ball_active_initial,		FALSE
  },
  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(1),
    &li.ball_content,			EL_EMPTY, NULL,
    &li.num_ball_contents,		4, MAX_ELEMENT_CONTENTS
  },

  {
    EL_SOKOBAN_FIELD_EMPTY,		-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.sb_fields_needed,		TRUE
  },

  {
    EL_SOKOBAN_OBJECT,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.sb_objects_needed,		TRUE
  },

  {
    EL_MM_MCDUFFIN,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.mm_laser_red,			FALSE
  },
  {
    EL_MM_MCDUFFIN,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.mm_laser_green,			FALSE
  },
  {
    EL_MM_MCDUFFIN,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.mm_laser_blue,			TRUE
  },

  {
    EL_DF_LASER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.df_laser_red,			TRUE
  },
  {
    EL_DF_LASER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.df_laser_green,			TRUE
  },
  {
    EL_DF_LASER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.df_laser_blue,			FALSE
  },

  {
    EL_MM_FUSE_ACTIVE,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_fuse,			25
  },
  {
    EL_MM_BOMB,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_bomb,			75
  },

  {
    EL_MM_GRAY_BALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_ball,			75
  },
  {
    EL_MM_GRAY_BALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.mm_ball_choice_mode,		ANIM_RANDOM
  },
  {
    EL_MM_GRAY_BALL,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.mm_ball_content,		EL_EMPTY, NULL,
    &li.num_mm_ball_contents,		8, MAX_MM_BALL_CONTENTS
  },
  {
    EL_MM_GRAY_BALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.rotate_mm_ball_content,		TRUE
  },
  {
    EL_MM_GRAY_BALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.explode_mm_ball,		FALSE
  },

  {
    EL_MM_STEEL_BLOCK,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_block,			75
  },
  {
    EL_MM_LIGHTBALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_ELEM_BONUS],		10
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_NOTE[] =
{
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &xx_envelope.xsize,			MAX_ENVELOPE_XSIZE,
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &xx_envelope.ysize,			MAX_ENVELOPE_YSIZE,
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &xx_envelope.autowrap,		FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &xx_envelope.centered,		FALSE
  },

  {
    -1,					-1,
    TYPE_STRING,			CONF_VALUE_BYTES(1),
    &xx_envelope.text,			-1, NULL,
    &xx_string_length_unused,		-1, MAX_ENVELOPE_TEXT_LEN,
    &xx_default_string_empty[0]
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_CUSX_base[] =
{
  {
    -1,					-1,
    TYPE_STRING,			CONF_VALUE_BYTES(1),
    &xx_ei.description[0],		-1,
    &yy_ei.description[0],
    &xx_string_length_unused,		-1, MAX_ELEMENT_NAME_LEN,
    &xx_default_description[0]
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(1),
    &xx_ei.properties[EP_BITFIELD_BASE_NR], EP_BITMASK_BASE_DEFAULT,
    &yy_ei.properties[EP_BITFIELD_BASE_NR]
  },
#if ENABLE_RESERVED_CODE
  // (reserved for later use)
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(2),
    &xx_ei.properties[EP_BITFIELD_BASE_NR + 1], EP_BITMASK_DEFAULT,
    &yy_ei.properties[EP_BITFIELD_BASE_NR + 1]
  },
#endif

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &xx_ei.use_gfx_element,		FALSE,
    &yy_ei.use_gfx_element
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_ei.gfx_element_initial,		EL_EMPTY_SPACE,
    &yy_ei.gfx_element_initial
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(2),
    &xx_ei.access_direction,		MV_ALL_DIRECTIONS,
    &yy_ei.access_direction
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &xx_ei.collect_score_initial,	10,
    &yy_ei.collect_score_initial
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(3),
    &xx_ei.collect_count_initial,	1,
    &yy_ei.collect_count_initial
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(4),
    &xx_ei.ce_value_fixed_initial,	0,
    &yy_ei.ce_value_fixed_initial
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(5),
    &xx_ei.ce_value_random_initial,	0,
    &yy_ei.ce_value_random_initial
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &xx_ei.use_last_ce_value,		FALSE,
    &yy_ei.use_last_ce_value
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(6),
    &xx_ei.push_delay_fixed,		8,
    &yy_ei.push_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(7),
    &xx_ei.push_delay_random,		8,
    &yy_ei.push_delay_random
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(8),
    &xx_ei.drop_delay_fixed,		0,
    &yy_ei.drop_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(9),
    &xx_ei.drop_delay_random,		0,
    &yy_ei.drop_delay_random
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(10),
    &xx_ei.move_delay_fixed,		0,
    &yy_ei.move_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(11),
    &xx_ei.move_delay_random,		0,
    &yy_ei.move_delay_random
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(16),
    &xx_ei.step_delay_fixed,		0,
    &yy_ei.step_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(17),
    &xx_ei.step_delay_random,		0,
    &yy_ei.step_delay_random
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(3),
    &xx_ei.move_pattern,		MV_ALL_DIRECTIONS,
    &yy_ei.move_pattern
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(4),
    &xx_ei.move_direction_initial,	MV_START_AUTOMATIC,
    &yy_ei.move_direction_initial
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(5),
    &xx_ei.move_stepsize,		TILEX / 8,
    &yy_ei.move_stepsize
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(12),
    &xx_ei.move_enter_element,		EL_EMPTY_SPACE,
    &yy_ei.move_enter_element
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(13),
    &xx_ei.move_leave_element,		EL_EMPTY_SPACE,
    &yy_ei.move_leave_element
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(6),
    &xx_ei.move_leave_type,		LEAVE_TYPE_UNLIMITED,
    &yy_ei.move_leave_type
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &xx_ei.slippery_type,		SLIPPERY_ANY_RANDOM,
    &yy_ei.slippery_type
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(8),
    &xx_ei.explosion_type,		EXPLODES_3X3,
    &yy_ei.explosion_type
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(14),
    &xx_ei.explosion_delay,		16,
    &yy_ei.explosion_delay
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(15),
    &xx_ei.ignition_delay,		8,
    &yy_ei.ignition_delay
  },

  {
    -1,					-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(2),
    &xx_ei.content,			EL_EMPTY_SPACE,
    &yy_ei.content,
    &xx_num_contents,			1, 1
  },

  // ---------- "num_change_pages" must be the last entry ---------------------

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(9),
    &xx_ei.num_change_pages,		1,
    &yy_ei.num_change_pages
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1,
    NULL
  }
};

static struct LevelFileConfigInfo chunk_config_CUSX_change[] =
{
  // ---------- "current_change_page" must be the first entry -----------------

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &xx_current_change_page,		-1
  },

  // ---------- (the remaining entries can be in any order) -------------------

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &xx_change.can_change,		FALSE
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(1),
    &xx_event_bits[0],			0
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(2),
    &xx_event_bits[1],			0
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(3),
    &xx_change.trigger_player,		CH_PLAYER_ANY
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(4),
    &xx_change.trigger_side,		CH_SIDE_ANY
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(3),
    &xx_change.trigger_page,		CH_PAGE_ANY
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_change.target_element,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &xx_change.delay_fixed,		0
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(3),
    &xx_change.delay_random,		0
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(4),
    &xx_change.delay_frames,		FRAMES_PER_SECOND
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(5),
    &xx_change.initial_trigger_element,	EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(6),
    &xx_change.explode,			FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(7),
    &xx_change.use_target_content,	FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &xx_change.only_if_complete,	FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &xx_change.use_random_replace,	FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(10),
    &xx_change.random_percentage,	100
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(11),
    &xx_change.replace_when,		CP_WHEN_EMPTY
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(12),
    &xx_change.has_action,		FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(13),
    &xx_change.action_type,		CA_NO_ACTION
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(14),
    &xx_change.action_mode,		CA_MODE_UNDEFINED
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(6),
    &xx_change.action_arg,		CA_ARG_UNDEFINED
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(7),
    &xx_change.action_element,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(1),
    &xx_change.target_content,		EL_EMPTY_SPACE, NULL,
    &xx_num_contents,			1, 1
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_GRPX[] =
{
  {
    -1,					-1,
    TYPE_STRING,			CONF_VALUE_BYTES(1),
    &xx_ei.description[0],		-1, NULL,
    &xx_string_length_unused,		-1, MAX_ELEMENT_NAME_LEN,
    &xx_default_description[0]
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &xx_ei.use_gfx_element,		FALSE
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_ei.gfx_element_initial,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &xx_group.choice_mode,		ANIM_RANDOM
  },

  {
    -1,					-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(2),
    &xx_group.element[0],		EL_EMPTY_SPACE, NULL,
    &xx_group.num_elements,		1, MAX_ELEMENTS_IN_GROUP
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_EMPX[] =
{
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &xx_ei.use_gfx_element,		FALSE
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_ei.gfx_element_initial,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_CONF[] =		// (OBSOLETE)
{
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.block_snap_field,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.continuous_snapping,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.initial_player_stepsize[0],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_start_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_artwork_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(12),
    &li.use_explosion_element[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[0],		EL_PLAYER_1
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct
{
  int filetype;
  char *id;
}
filetype_id_list[] =
{
  { LEVEL_FILE_TYPE_RND,	"RND"	},
  { LEVEL_FILE_TYPE_BD,		"BD"	},
  { LEVEL_FILE_TYPE_EM,		"EM"	},
  { LEVEL_FILE_TYPE_SP,		"SP"	},
  { LEVEL_FILE_TYPE_DX,		"DX"	},
  { LEVEL_FILE_TYPE_SB,		"SB"	},
  { LEVEL_FILE_TYPE_DC,		"DC"	},
  { LEVEL_FILE_TYPE_MM,		"MM"	},
  { LEVEL_FILE_TYPE_MM,		"DF"	},
  { -1,				NULL	},
};


// ============================================================================
// level file functions
// ============================================================================

static boolean check_special_flags(char *flag)
{
  if (strEqual(options.special_flags, flag) ||
      (leveldir_current != NULL && strEqual(leveldir_current->special_flags, flag)))
    return TRUE;

  return FALSE;
}

static struct DateInfo getCurrentDate(void)
{
  time_t epoch_seconds = time(NULL);
  struct tm *now = localtime(&epoch_seconds);
  struct DateInfo date;

  date.year  = now->tm_year + 1900;
  date.month = now->tm_mon  + 1;
  date.day   = now->tm_mday;

  date.src   = DATE_SRC_CLOCK;

  return date;
}

static void resetEventFlags(struct ElementChangeInfo *change)
{
  int i;

  for (i = 0; i < NUM_CHANGE_EVENTS; i++)
    change->has_event[i] = FALSE;
}

static void resetEventBits(void)
{
  int i;

  for (i = 0; i < NUM_CE_BITFIELDS; i++)
    xx_event_bits[i] = 0;
}

static void setEventFlagsFromEventBits(struct ElementChangeInfo *change)
{
  int i;

  /* important: only change event flag if corresponding event bit is set
     (this is because all xx_event_bits[] values are loaded separately,
     and all xx_event_bits[] values are set back to zero before loading
     another value xx_event_bits[x] (each value representing 32 flags)) */

  for (i = 0; i < NUM_CHANGE_EVENTS; i++)
    if (xx_event_bits[CH_EVENT_BITFIELD_NR(i)] & CH_EVENT_BIT(i))
      change->has_event[i] = TRUE;
}

static void setEventBitsFromEventFlags(struct ElementChangeInfo *change)
{
  int i;

  /* in contrast to the above function setEventFlagsFromEventBits(), it
     would also be possible to set all bits in xx_event_bits[] to 0 or 1
     depending on the corresponding change->has_event[i] values here, as
     all xx_event_bits[] values are reset in resetEventBits() before */

  for (i = 0; i < NUM_CHANGE_EVENTS; i++)
    if (change->has_event[i])
      xx_event_bits[CH_EVENT_BITFIELD_NR(i)] |= CH_EVENT_BIT(i);
}

static char *getDefaultElementDescription(struct ElementInfo *ei)
{
  static char description[MAX_ELEMENT_NAME_LEN + 1];
  char *default_description = (ei->custom_description != NULL ?
			       ei->custom_description :
			       ei->editor_description);
  int i;

  // always start with reliable default values
  for (i = 0; i < MAX_ELEMENT_NAME_LEN + 1; i++)
    description[i] = '\0';

  // truncate element description to MAX_ELEMENT_NAME_LEN bytes
  strncpy(description, default_description, MAX_ELEMENT_NAME_LEN);

  return &description[0];
}

static void setElementDescriptionToDefault(struct ElementInfo *ei)
{
  char *default_description = getDefaultElementDescription(ei);
  int i;

  for (i = 0; i < MAX_ELEMENT_NAME_LEN + 1; i++)
    ei->description[i] = default_description[i];
}

static void setConfigToDefaultsFromConfigList(struct LevelFileConfigInfo *conf)
{
  int i;

  for (i = 0; conf[i].data_type != -1; i++)
  {
    int default_value = conf[i].default_value;
    int data_type = conf[i].data_type;
    int conf_type = conf[i].conf_type;
    int byte_mask = conf_type & CONF_MASK_BYTES;

    if (byte_mask == CONF_MASK_MULTI_BYTES)
    {
      int default_num_entities = conf[i].default_num_entities;
      int max_num_entities = conf[i].max_num_entities;

      *(int *)(conf[i].num_entities) = default_num_entities;

      if (data_type == TYPE_STRING)
      {
	char *default_string = conf[i].default_string;
	char *string = (char *)(conf[i].value);

	strncpy(string, default_string, max_num_entities);
      }
      else if (data_type == TYPE_ELEMENT_LIST)
      {
	int *element_array = (int *)(conf[i].value);
	int j;

	for (j = 0; j < max_num_entities; j++)
	  element_array[j] = default_value;
      }
      else if (data_type == TYPE_CONTENT_LIST)
      {
	struct Content *content = (struct Content *)(conf[i].value);
	int c, x, y;

	for (c = 0; c < max_num_entities; c++)
	  for (y = 0; y < 3; y++)
	    for (x = 0; x < 3; x++)
	      content[c].e[x][y] = default_value;
      }
    }
    else	// constant size configuration data (1, 2 or 4 bytes)
    {
      if (data_type == TYPE_BOOLEAN)
	*(boolean *)(conf[i].value) = default_value;
      else
	*(int *)    (conf[i].value) = default_value;
    }
  }
}

static void copyConfigFromConfigList(struct LevelFileConfigInfo *conf)
{
  int i;

  for (i = 0; conf[i].data_type != -1; i++)
  {
    int data_type = conf[i].data_type;
    int conf_type = conf[i].conf_type;
    int byte_mask = conf_type & CONF_MASK_BYTES;

    if (byte_mask == CONF_MASK_MULTI_BYTES)
    {
      int max_num_entities = conf[i].max_num_entities;

      if (data_type == TYPE_STRING)
      {
	char *string      = (char *)(conf[i].value);
	char *string_copy = (char *)(conf[i].value_copy);

	strncpy(string_copy, string, max_num_entities);
      }
      else if (data_type == TYPE_ELEMENT_LIST)
      {
	int *element_array      = (int *)(conf[i].value);
	int *element_array_copy = (int *)(conf[i].value_copy);
	int j;

	for (j = 0; j < max_num_entities; j++)
	  element_array_copy[j] = element_array[j];
      }
      else if (data_type == TYPE_CONTENT_LIST)
      {
	struct Content *content      = (struct Content *)(conf[i].value);
	struct Content *content_copy = (struct Content *)(conf[i].value_copy);
	int c, x, y;

	for (c = 0; c < max_num_entities; c++)
	  for (y = 0; y < 3; y++)
	    for (x = 0; x < 3; x++)
	      content_copy[c].e[x][y] = content[c].e[x][y];
      }
    }
    else	// constant size configuration data (1, 2 or 4 bytes)
    {
      if (data_type == TYPE_BOOLEAN)
	*(boolean *)(conf[i].value_copy) = *(boolean *)(conf[i].value);
      else
	*(int *)    (conf[i].value_copy) = *(int *)    (conf[i].value);
    }
  }
}

void copyElementInfo(struct ElementInfo *ei_from, struct ElementInfo *ei_to)
{
  int i;

  xx_ei = *ei_from;	// copy element data into temporary buffer
  yy_ei = *ei_to;	// copy element data into temporary buffer

  copyConfigFromConfigList(chunk_config_CUSX_base);

  *ei_from = xx_ei;
  *ei_to   = yy_ei;

  // ---------- reinitialize and copy change pages ----------

  ei_to->num_change_pages = ei_from->num_change_pages;
  ei_to->current_change_page = ei_from->current_change_page;

  setElementChangePages(ei_to, ei_to->num_change_pages);

  for (i = 0; i < ei_to->num_change_pages; i++)
    ei_to->change_page[i] = ei_from->change_page[i];

  // ---------- copy group element info ----------
  if (ei_from->group != NULL && ei_to->group != NULL)	// group or internal
    *ei_to->group = *ei_from->group;

  // mark this custom element as modified
  ei_to->modified_settings = TRUE;
}

void setElementChangePages(struct ElementInfo *ei, int change_pages)
{
  int change_page_size = sizeof(struct ElementChangeInfo);

  ei->num_change_pages = MAX(1, change_pages);

  ei->change_page =
    checked_realloc(ei->change_page, ei->num_change_pages * change_page_size);

  if (ei->current_change_page >= ei->num_change_pages)
    ei->current_change_page = ei->num_change_pages - 1;

  ei->change = &ei->change_page[ei->current_change_page];
}

void setElementChangeInfoToDefaults(struct ElementChangeInfo *change)
{
  xx_change = *change;		// copy change data into temporary buffer

  setConfigToDefaultsFromConfigList(chunk_config_CUSX_change);

  *change = xx_change;

  resetEventFlags(change);

  change->direct_action = 0;
  change->other_action = 0;

  change->pre_change_function = NULL;
  change->change_function = NULL;
  change->post_change_function = NULL;
}

static void setLevelInfoToDefaults_Level(struct LevelInfo *level, boolean prepare_loading_level)
{
  boolean add_border = FALSE;
  int x1 = 0;
  int y1 = 0;
  int x2 = STD_LEV_FIELDX - 1;
  int y2 = STD_LEV_FIELDY - 1;
  int i, x, y;

  li = *level;		// copy level data into temporary buffer
  setConfigToDefaultsFromConfigList(chunk_config_INFO);
  *level = li;		// copy temporary buffer back to level data

  setLevelInfoToDefaults_BD();
  setLevelInfoToDefaults_EM();
  setLevelInfoToDefaults_SP();
  setLevelInfoToDefaults_MM();

  level->native_bd_level = &native_bd_level;
  level->native_em_level = &native_em_level;
  level->native_sp_level = &native_sp_level;
  level->native_mm_level = &native_mm_level;

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  level->creation_date = getCurrentDate();

  level->encoding_16bit_field  = TRUE;
  level->encoding_16bit_yamyam = TRUE;
  level->encoding_16bit_amoeba = TRUE;

  // clear level name and level author string buffers
  for (i = 0; i < MAX_OUTPUT_LINESIZE; i++)
    level->name_native[i] = '\0';
  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = '\0';
  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = '\0';

  // set level name and level author to default values
  strcpy(level->name_native, NAMELESS_LEVEL_NAME);
  strcpy(level->name, NAMELESS_LEVEL_NAME);
  strcpy(level->author, ANONYMOUS_NAME);

  // set default game engine type
  level->game_engine_type = setup.default_game_engine_type;

  // some game engines should have a default playfield with border elements
  if (level->game_engine_type == GAME_ENGINE_TYPE_BD ||
      level->game_engine_type == GAME_ENGINE_TYPE_EM ||
      level->game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    add_border = TRUE;
    x1++;
    y1++;
    x2--;
    y2--;
  }

  // set level playfield to playable default level with player and exit
  for (x = 0; x < MAX_LEV_FIELDX; x++)
  {
    for (y = 0; y < MAX_LEV_FIELDY; y++)
    {
      if (add_border && (x == 0 || x == STD_LEV_FIELDX - 1 ||
			 y == 0 || y == STD_LEV_FIELDY - 1))
	level->field[x][y] = getEngineElement(EL_STEELWALL);
      else
	level->field[x][y] = getEngineElement(EL_SAND);
    }
  }

  level->field[x1][y1] = getEngineElement(EL_PLAYER_1);
  level->field[x2][y2] = getEngineElement(EL_EXIT_CLOSED);

  BorderElement = getEngineElement(EL_STEELWALL);

  // detect custom elements when loading them
  level->file_has_custom_elements = FALSE;

  // set random colors for new levels only, but never when loading existing level
  // (as default colors are not stored in level file, which would result in wrong colors)
  if (!prepare_loading_level)
  {
    // set random colors for BD style levels according to preferred color type
    SetRandomLevelColors_BD(setup.bd_default_color_type);

    // set default color type and colors for BD style level colors
    SetDefaultLevelColorType_BD();
    SetDefaultLevelColors_BD();
  }

  // set all bug compatibility flags to "false" => do not emulate this bug
  level->use_action_after_change_bug = FALSE;

  if (leveldir_current)
  {
    // try to determine better author name than 'anonymous'
    if (!strEqual(leveldir_current->author, ANONYMOUS_NAME))
    {
      strncpy(level->author, leveldir_current->author, MAX_LEVEL_AUTHOR_LEN);
      level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
    }
    else
    {
      switch (LEVELCLASS(leveldir_current))
      {
	case LEVELCLASS_TUTORIAL:
	  strcpy(level->author, PROGRAM_AUTHOR_STRING);
	  break;

        case LEVELCLASS_CONTRIB:
	  strncpy(level->author, leveldir_current->name, MAX_LEVEL_AUTHOR_LEN);
	  level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	  break;

        case LEVELCLASS_PRIVATE:
	  strncpy(level->author, getRealName(), MAX_LEVEL_AUTHOR_LEN);
	  level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	  break;

        default:
	  // keep default value
	  break;
      }
    }
  }
}

static void setLevelInfoToDefaults_Elements(struct LevelInfo *level)
{
  static boolean clipboard_elements_initialized = FALSE;
  int i;

  InitElementPropertiesStatic();

  li = *level;		// copy level data into temporary buffer
  setConfigToDefaultsFromConfigList(chunk_config_ELEM);
  *level = li;		// copy temporary buffer back to level data

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    int element = i;
    struct ElementInfo *ei = &element_info[element];

    if (element == EL_MM_GRAY_BALL)
    {
      struct LevelInfo_MM *level_mm = level->native_mm_level;
      int j;

      for (j = 0; j < level->num_mm_ball_contents; j++)
	level->mm_ball_content[j] =
	  map_element_MM_to_RND(level_mm->ball_content[j]);
    }

    // never initialize clipboard elements after the very first time
    // (to be able to use clipboard elements between several levels)
    if (IS_CLIPBOARD_ELEMENT(element) && clipboard_elements_initialized)
      continue;

    if (IS_ENVELOPE(element))
    {
      int envelope_nr = element - EL_ENVELOPE_1;

      setConfigToDefaultsFromConfigList(chunk_config_NOTE);

      level->envelope[envelope_nr] = xx_envelope;
    }

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      xx_ei = *ei;	// copy element data into temporary buffer

      setConfigToDefaultsFromConfigList(chunk_config_CUSX_base);

      *ei = xx_ei;
    }

    setElementChangePages(ei, 1);
    setElementChangeInfoToDefaults(ei->change);

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_GROUP_ELEMENT(element))
    {
      setElementDescriptionToDefault(ei);

      ei->modified_settings = FALSE;
    }

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      // internal values used in level editor

      ei->access_type = 0;
      ei->access_layer = 0;
      ei->access_protected = 0;
      ei->walk_to_action = 0;
      ei->smash_targets = 0;
      ei->deadliness = 0;

      ei->can_explode_by_fire = FALSE;
      ei->can_explode_smashed = FALSE;
      ei->can_explode_impact = FALSE;

      ei->current_change_page = 0;
    }

    if (IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      struct ElementGroupInfo *group;

      // initialize memory for list of elements in group
      if (ei->group == NULL)
	ei->group = checked_malloc(sizeof(struct ElementGroupInfo));

      group = ei->group;

      xx_group = *group;	// copy group data into temporary buffer

      setConfigToDefaultsFromConfigList(chunk_config_GRPX);

      *group = xx_group;
    }

    if (IS_EMPTY_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      xx_ei = *ei;		// copy element data into temporary buffer

      setConfigToDefaultsFromConfigList(chunk_config_EMPX);

      *ei = xx_ei;
    }
  }

  clipboard_elements_initialized = TRUE;
}

static void setLevelInfoToDefaults(struct LevelInfo *level,
				   boolean level_info_only,
				   boolean prepare_loading_level)
{
  setLevelInfoToDefaults_Level(level, prepare_loading_level);

  if (!level_info_only)
    setLevelInfoToDefaults_Elements(level);

  if (prepare_loading_level)
  {
    level->no_valid_file = FALSE;
    level->no_level_file = FALSE;
  }

  level->changed = FALSE;
}

static void setFileInfoToDefaults(struct LevelFileInfo *level_file_info)
{
  level_file_info->nr = 0;
  level_file_info->type = LEVEL_FILE_TYPE_UNKNOWN;
  level_file_info->packed = FALSE;

  setString(&level_file_info->basename, NULL);
  setString(&level_file_info->filename, NULL);
}

int getMappedElement_SB(int, boolean);

static void ActivateLevelTemplate(void)
{
  int x, y;

  if (check_special_flags("load_xsb_to_ces"))
  {
    // fill smaller playfields with padding "beyond border wall" elements
    if (level.fieldx < level_template.fieldx ||
	level.fieldy < level_template.fieldy)
    {
      short field[level.fieldx][level.fieldy];
      int new_fieldx = MAX(level.fieldx, level_template.fieldx);
      int new_fieldy = MAX(level.fieldy, level_template.fieldy);
      int pos_fieldx = (new_fieldx - level.fieldx) / 2;
      int pos_fieldy = (new_fieldy - level.fieldy) / 2;

      // copy old playfield (which is smaller than the visible area)
      for (y = 0; y < level.fieldy; y++) for (x = 0; x < level.fieldx; x++)
	field[x][y] = level.field[x][y];

      // fill new, larger playfield with "beyond border wall" elements
      for (y = 0; y < new_fieldy; y++) for (x = 0; x < new_fieldx; x++)
	level.field[x][y] = getMappedElement_SB('_', TRUE);

      // copy the old playfield to the middle of the new playfield
      for (y = 0; y < level.fieldy; y++) for (x = 0; x < level.fieldx; x++)
	level.field[pos_fieldx + x][pos_fieldy + y] = field[x][y];

      level.fieldx = new_fieldx;
      level.fieldy = new_fieldy;
    }
  }

  // Currently there is no special action needed to activate the template
  // data, because 'element_info' property settings overwrite the original
  // level data, while all other variables do not change.

  // Exception: 'from_level_template' elements in the original level playfield
  // are overwritten with the corresponding elements at the same position in
  // playfield from the level template.

  for (x = 0; x < level.fieldx; x++)
    for (y = 0; y < level.fieldy; y++)
      if (level.field[x][y] == EL_FROM_LEVEL_TEMPLATE)
	level.field[x][y] = level_template.field[x][y];

  if (check_special_flags("load_xsb_to_ces"))
  {
    struct LevelInfo level_backup = level;

    // overwrite all individual level settings from template level settings
    level = level_template;

    // restore level file info
    level.file_info = level_backup.file_info;

    // restore playfield size
    level.fieldx = level_backup.fieldx;
    level.fieldy = level_backup.fieldy;

    // restore playfield content
    for (x = 0; x < level.fieldx; x++)
      for (y = 0; y < level.fieldy; y++)
	level.field[x][y] = level_backup.field[x][y];

    // restore name and author from individual level
    strcpy(level.name,   level_backup.name);
    strcpy(level.author, level_backup.author);

    // restore flag "use_custom_template"
    level.use_custom_template = level_backup.use_custom_template;
  }
}

boolean isLevelsetFilename_BD(char *filename)
{
  return (strSuffixLower(filename, ".bd") ||
	  strSuffixLower(filename, ".bdr") ||
	  strSuffixLower(filename, ".brc") ||
	  strSuffixLower(filename, ".gds"));
}

static boolean checkForPackageFromBasename_BD(char *basename)
{
  // check for native BD level file extensions
  if (!isLevelsetFilename_BD(basename))
    return FALSE;

  // check for standard single-level BD files (like "001.bd")
  if (strSuffixLower(basename, ".bd") &&
      strlen(basename) == 6 &&
      basename[0] >= '0' && basename[0] <= '9' &&
      basename[1] >= '0' && basename[1] <= '9' &&
      basename[2] >= '0' && basename[2] <= '9')
    return FALSE;

  // this is a level package in native BD file format
  return TRUE;
}

static char *getLevelFilenameFromBasename(char *basename)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getCurrentLevelDir(), basename);

  return filename;
}

static int getFileTypeFromBasename(char *basename)
{
  // !!! ALSO SEE COMMENT IN checkForPackageFromBasename() !!!

  static char *filename = NULL;
  struct stat file_status;

  // ---------- try to determine file type from filename ----------

  // check for typical filename of a Supaplex level package file
  if (strlen(basename) == 10 && strPrefixLower(basename, "levels.d"))
    return LEVEL_FILE_TYPE_SP;

  // check for typical filename of a Diamond Caves II level package file
  if (strSuffixLower(basename, ".dc") ||
      strSuffixLower(basename, ".dc2"))
    return LEVEL_FILE_TYPE_DC;

  // check for typical filename of a Sokoban level package file
  if (strSuffixLower(basename, ".xsb") &&
      strchr(basename, '%') == NULL)
    return LEVEL_FILE_TYPE_SB;

  // check for typical filename of a Boulder Dash (GDash) level package file
  if (checkForPackageFromBasename_BD(basename))
    return LEVEL_FILE_TYPE_BD;

  // ---------- try to determine file type from filesize ----------

  checked_free(filename);
  filename = getPath2(getCurrentLevelDir(), basename);

  if (stat(filename, &file_status) == 0)
  {
    // check for typical filesize of a Supaplex level package file
    if (file_status.st_size == 170496)
      return LEVEL_FILE_TYPE_SP;
  }

  return LEVEL_FILE_TYPE_UNKNOWN;
}

static int getFileTypeFromMagicBytes(char *filename, int type)
{
  File *file;

  if ((file = openFile(filename, MODE_READ)))
  {
    char chunk_name[CHUNK_ID_LEN + 1];

    getFileChunkBE(file, chunk_name, NULL);

    if (strEqual(chunk_name, "MMII") ||
	strEqual(chunk_name, "MIRR"))
      type = LEVEL_FILE_TYPE_MM;

    closeFile(file);
  }

  return type;
}

static boolean checkForPackageFromBasename(char *basename)
{
  // !!! WON'T WORK ANYMORE IF getFileTypeFromBasename() ALSO DETECTS !!!
  // !!! SINGLE LEVELS (CURRENTLY ONLY DETECTS LEVEL PACKAGES         !!!

  return (getFileTypeFromBasename(basename) != LEVEL_FILE_TYPE_UNKNOWN);
}

static char *getSingleLevelBasenameExt(int nr, char *extension)
{
  static char basename[MAX_FILENAME_LEN];

  if (nr < 0)
    sprintf(basename, "%s", LEVELTEMPLATE_FILENAME);
  else
    sprintf(basename, "%03d.%s", nr, extension);

  return basename;
}

static char *getSingleLevelBasename(int nr)
{
  return getSingleLevelBasenameExt(nr, LEVELFILE_EXTENSION);
}

static char *getPackedLevelBasename(int type)
{
  static char basename[MAX_FILENAME_LEN];
  char *directory = getCurrentLevelDir();
  Directory *dir;
  DirectoryEntry *dir_entry;

  strcpy(basename, UNDEFINED_FILENAME);		// default: undefined file

  if ((dir = openDirectory(directory)) == NULL)
  {
    Warn("cannot read current level directory '%s'", directory);

    return basename;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	// loop all entries
  {
    char *entry_basename = dir_entry->basename;
    int entry_type = getFileTypeFromBasename(entry_basename);

    if (entry_type != LEVEL_FILE_TYPE_UNKNOWN)	// found valid level package
    {
      if (type == LEVEL_FILE_TYPE_UNKNOWN ||
	  type == entry_type)
      {
	strcpy(basename, entry_basename);

	break;
      }
    }
  }

  closeDirectory(dir);

  return basename;
}

static char *getSingleLevelFilename(int nr)
{
  return getLevelFilenameFromBasename(getSingleLevelBasename(nr));
}

#if ENABLE_UNUSED_CODE
static char *getPackedLevelFilename(int type)
{
  return getLevelFilenameFromBasename(getPackedLevelBasename(type));
}
#endif

char *getDefaultLevelFilename(int nr)
{
  return getSingleLevelFilename(nr);
}

#if ENABLE_UNUSED_CODE
static void setLevelFileInfo_SingleLevelFilename(struct LevelFileInfo *lfi,
						 int type)
{
  lfi->type = type;
  lfi->packed = FALSE;

  setString(&lfi->basename, getSingleLevelBasename(lfi->nr, lfi->type));
  setString(&lfi->filename, getLevelFilenameFromBasename(lfi->basename));
}
#endif

static void setLevelFileInfo_FormatLevelFilename(struct LevelFileInfo *lfi,
						 int type, char *format, ...)
{
  static char basename[MAX_FILENAME_LEN];
  va_list ap;

  va_start(ap, format);
  vsprintf(basename, format, ap);
  va_end(ap);

  lfi->type = type;
  lfi->packed = FALSE;

  setString(&lfi->basename, basename);
  setString(&lfi->filename, getLevelFilenameFromBasename(lfi->basename));
}

static void setLevelFileInfo_PackedLevelFilename(struct LevelFileInfo *lfi,
						 int type)
{
  lfi->type = type;
  lfi->packed = TRUE;

  setString(&lfi->basename, getPackedLevelBasename(lfi->type));
  setString(&lfi->filename, getLevelFilenameFromBasename(lfi->basename));
}

static int getFiletypeFromID(char *filetype_id)
{
  char *filetype_id_lower;
  int filetype = LEVEL_FILE_TYPE_UNKNOWN;
  int i;

  if (filetype_id == NULL)
    return LEVEL_FILE_TYPE_UNKNOWN;

  filetype_id_lower = getStringToLower(filetype_id);

  for (i = 0; filetype_id_list[i].id != NULL; i++)
  {
    char *id_lower = getStringToLower(filetype_id_list[i].id);
    
    if (strEqual(filetype_id_lower, id_lower))
      filetype = filetype_id_list[i].filetype;

    free(id_lower);

    if (filetype != LEVEL_FILE_TYPE_UNKNOWN)
      break;
  }

  free(filetype_id_lower);

  return filetype;
}

char *getLocalLevelTemplateFilename(void)
{
  return getLevelFilenameFromBasename(LEVELTEMPLATE_FILENAME);
}

char *getGlobalLevelTemplateFilename(void)
{
  return getFilenameFromCurrentLevelDirUpward(LEVELTEMPLATE_FILENAME);
}

static void determineLevelFileInfo_Filename(struct LevelFileInfo *lfi)
{
  int nr = lfi->nr;

  // special case: level number is negative => check for level template file
  if (nr < 0)
  {
    setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_RND,
					 getSingleLevelBasename(-1));

    // replace local level template filename with global template filename
    setString(&lfi->filename, getGlobalLevelTemplateFilename());

    // no fallback if template file not existing
    return;
  }

  // special case: check for file name/pattern specified in "levelinfo.conf"
  if (leveldir_current->level_filename != NULL)
  {
    int filetype = getFiletypeFromID(leveldir_current->level_filetype);

    setLevelFileInfo_FormatLevelFilename(lfi, filetype,
					 leveldir_current->level_filename, nr);

    lfi->packed = checkForPackageFromBasename(leveldir_current->level_filename);

    if (fileExists(lfi->filename))
      return;
  }
  else if (leveldir_current->level_filetype != NULL)
  {
    int filetype = getFiletypeFromID(leveldir_current->level_filetype);

    // check for specified native level file with standard file name
    setLevelFileInfo_FormatLevelFilename(lfi, filetype,
					 "%03d.%s", nr, LEVELFILE_EXTENSION);
    if (fileExists(lfi->filename))
      return;
  }

  // check for native Rocks'n'Diamonds level file
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_RND,
				       "%03d.%s", nr, LEVELFILE_EXTENSION);
  if (fileExists(lfi->filename))
    return;

  // check for native Boulder Dash level file
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_BD, "%03d.bd", nr);
  if (fileExists(lfi->filename))
    return;

  // check for Emerald Mine level file (V1)
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "a%c%c",
				       'a' + (nr / 10) % 26, '0' + nr % 10);
  if (fileExists(lfi->filename))
    return;
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "A%c%c",
				       'A' + (nr / 10) % 26, '0' + nr % 10);
  if (fileExists(lfi->filename))
    return;

  // check for Emerald Mine level file (V2 to V5)
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%d", nr);
  if (fileExists(lfi->filename))
    return;

  // check for Emerald Mine level file (V6 / single mode)
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02ds", nr);
  if (fileExists(lfi->filename))
    return;
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02dS", nr);
  if (fileExists(lfi->filename))
    return;

  // check for Emerald Mine level file (V6 / teamwork mode)
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02dt", nr);
  if (fileExists(lfi->filename))
    return;
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02dT", nr);
  if (fileExists(lfi->filename))
    return;

  // check for various packed level file formats
  setLevelFileInfo_PackedLevelFilename(lfi, LEVEL_FILE_TYPE_UNKNOWN);
  if (fileExists(lfi->filename))
    return;

  // no known level file found -- use default values (and fail later)
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_RND,
				       "%03d.%s", nr, LEVELFILE_EXTENSION);
}

static void determineLevelFileInfo_Filetype(struct LevelFileInfo *lfi)
{
  if (lfi->type == LEVEL_FILE_TYPE_UNKNOWN)
    lfi->type = getFileTypeFromBasename(lfi->basename);

  if (lfi->type == LEVEL_FILE_TYPE_RND)
    lfi->type = getFileTypeFromMagicBytes(lfi->filename, lfi->type);
}

static void setLevelFileInfo(struct LevelFileInfo *level_file_info, int nr)
{
  // always start with reliable default values
  setFileInfoToDefaults(level_file_info);

  level_file_info->nr = nr;	// set requested level number

  determineLevelFileInfo_Filename(level_file_info);
  determineLevelFileInfo_Filetype(level_file_info);
}

static void copyLevelFileInfo(struct LevelFileInfo *lfi_from,
			      struct LevelFileInfo *lfi_to)
{
  lfi_to->nr     = lfi_from->nr;
  lfi_to->type   = lfi_from->type;
  lfi_to->packed = lfi_from->packed;

  setString(&lfi_to->basename, lfi_from->basename);
  setString(&lfi_to->filename, lfi_from->filename);
}

// ----------------------------------------------------------------------------
// functions for loading R'n'D level
// ----------------------------------------------------------------------------

int getMappedElement(int element)
{
  // remap some (historic, now obsolete) elements

  switch (element)
  {
    case EL_PLAYER_OBSOLETE:
      element = EL_PLAYER_1;
      break;

    case EL_KEY_OBSOLETE:
      element = EL_KEY_1;
      break;

    case EL_EM_KEY_1_FILE_OBSOLETE:
      element = EL_EM_KEY_1;
      break;

    case EL_EM_KEY_2_FILE_OBSOLETE:
      element = EL_EM_KEY_2;
      break;

    case EL_EM_KEY_3_FILE_OBSOLETE:
      element = EL_EM_KEY_3;
      break;

    case EL_EM_KEY_4_FILE_OBSOLETE:
      element = EL_EM_KEY_4;
      break;

    case EL_ENVELOPE_OBSOLETE:
      element = EL_ENVELOPE_1;
      break;

    case EL_SP_EMPTY:
      element = EL_EMPTY;
      break;

    default:
      if (element >= NUM_FILE_ELEMENTS)
      {
	Warn("invalid level element %d", element);

	element = EL_UNKNOWN;
      }
      break;
  }

  return element;
}

static int getMappedElementByVersion(int element, VersionType game_version)
{
  // remap some elements due to certain game version

  if (game_version <= VERSION_IDENT(2,2,0,0))
  {
    // map game font elements
    element = (element == EL_CHAR('[')  ? EL_CHAR_AUMLAUT :
	       element == EL_CHAR('\\') ? EL_CHAR_OUMLAUT :
	       element == EL_CHAR(']')  ? EL_CHAR_UUMLAUT :
	       element == EL_CHAR('^')  ? EL_CHAR_COPYRIGHT : element);
  }

  if (game_version < VERSION_IDENT(3,0,0,0))
  {
    // map Supaplex gravity tube elements
    element = (element == EL_SP_GRAVITY_PORT_LEFT  ? EL_SP_PORT_LEFT  :
	       element == EL_SP_GRAVITY_PORT_RIGHT ? EL_SP_PORT_RIGHT :
	       element == EL_SP_GRAVITY_PORT_UP    ? EL_SP_PORT_UP    :
	       element == EL_SP_GRAVITY_PORT_DOWN  ? EL_SP_PORT_DOWN  :
	       element);
  }

  return element;
}

static int LoadLevel_VERS(File *file, int chunk_size, struct LevelInfo *level)
{
  level->file_version = getFileVersion(file);
  level->game_version = getFileVersion(file);

  return chunk_size;
}

static int LoadLevel_DATE(File *file, int chunk_size, struct LevelInfo *level)
{
  level->creation_date.year  = getFile16BitBE(file);
  level->creation_date.month = getFile8Bit(file);
  level->creation_date.day   = getFile8Bit(file);

  level->creation_date.src   = DATE_SRC_LEVELFILE;

  return chunk_size;
}

static int LoadLevel_HEAD(File *file, int chunk_size, struct LevelInfo *level)
{
  int initial_player_stepsize;
  int initial_player_gravity;
  int i, x, y;

  level->fieldx = getFile8Bit(file);
  level->fieldy = getFile8Bit(file);

  level->time		= getFile16BitBE(file);
  level->gems_needed	= getFile16BitBE(file);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = getFile8Bit(file);

  level->num_yamyam_contents = STD_ELEMENT_CONTENTS;
  for (i = 0; i < STD_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i].e[x][y] = getMappedElement(getFile8Bit(file));

  level->amoeba_speed		= getFile8Bit(file);
  level->time_magic_wall	= getFile8Bit(file);
  level->time_wheel		= getFile8Bit(file);
  level->amoeba_content		= getMappedElement(getFile8Bit(file));

  initial_player_stepsize	= (getFile8Bit(file) == 1 ? STEPSIZE_FAST :
				   STEPSIZE_NORMAL);

  for (i = 0; i < MAX_PLAYERS; i++)
    level->initial_player_stepsize[i] = initial_player_stepsize;

  initial_player_gravity	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  for (i = 0; i < MAX_PLAYERS; i++)
    level->initial_player_gravity[i] = initial_player_gravity;

  level->encoding_16bit_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->em_slippery_gems	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->use_custom_template	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->block_last_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->sp_block_last_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->can_move_into_acid_bits = getFile32BitBE(file);
  level->dont_collide_with_bits = getFile8Bit(file);

  level->use_spring_bug		= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->use_step_counter	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->instant_relocation	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->can_pass_to_walkable	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->grow_into_diggable	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->game_engine_type	= getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_HEAD_UNUSED);

  return chunk_size;
}

static int LoadLevel_NAME(File *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  strncpy(level->name_native, level->name, MAX_OUTPUT_LINESIZE);

  return chunk_size;
}

static int LoadLevel_AUTH(File *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = getFile8Bit(file);
  level->author[MAX_LEVEL_AUTHOR_LEN] = 0;

  return chunk_size;
}

static int LoadLevel_BODY(File *file, int chunk_size, struct LevelInfo *level)
{
  int x, y;
  int chunk_size_expected = level->fieldx * level->fieldy;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= FILE_VERSION_2_0)
    chunk_size_expected *= 2;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      level->field[x][y] =
	getMappedElement(level->encoding_16bit_field ? getFile16BitBE(file) :
			 getFile8Bit(file));
  return chunk_size;
}

static int LoadLevel_CONT(File *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int header_size = 4;
  int content_size = MAX_ELEMENT_CONTENTS * 3 * 3;
  int chunk_size_expected = header_size + content_size;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= FILE_VERSION_2_0)
    chunk_size_expected += content_size;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  getFile8Bit(file);
  level->num_yamyam_contents = getFile8Bit(file);
  getFile8Bit(file);
  getFile8Bit(file);

  // correct invalid number of content fields -- should never happen
  if (level->num_yamyam_contents < 1 ||
      level->num_yamyam_contents > MAX_ELEMENT_CONTENTS)
    level->num_yamyam_contents = STD_ELEMENT_CONTENTS;

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i].e[x][y] =
	  getMappedElement(level->encoding_16bit_field ?
			   getFile16BitBE(file) : getFile8Bit(file));
  return chunk_size;
}

static int LoadLevel_CNT2(File *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int element;
  int num_contents;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  element = getMappedElement(getFile16BitBE(file));
  num_contents = getFile8Bit(file);

  getFile8Bit(file);	// content x size (unused)
  getFile8Bit(file);	// content y size (unused)

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	content_array[i][x][y] = getMappedElement(getFile16BitBE(file));

  // correct invalid number of content fields -- should never happen
  if (num_contents < 1 || num_contents > MAX_ELEMENT_CONTENTS)
    num_contents = STD_ELEMENT_CONTENTS;

  if (element == EL_YAMYAM)
  {
    level->num_yamyam_contents = num_contents;

    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  level->yamyam_content[i].e[x][y] = content_array[i][x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    level->amoeba_content = content_array[0][0][0];
  }
  else
  {
    Warn("cannot load content for element '%d'", element);
  }

  return chunk_size;
}

static int LoadLevel_CNT3(File *file, int chunk_size, struct LevelInfo *level)
{
  int i;
  int element;
  int envelope_nr;
  int envelope_len;
  int chunk_size_expected;

  element = getMappedElement(getFile16BitBE(file));
  if (!IS_ENVELOPE(element))
    element = EL_ENVELOPE_1;

  envelope_nr = element - EL_ENVELOPE_1;

  envelope_len = getFile16BitBE(file);

  level->envelope[envelope_nr].xsize = getFile8Bit(file);
  level->envelope[envelope_nr].ysize = getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT3_UNUSED);

  chunk_size_expected = LEVEL_CHUNK_CNT3_SIZE(envelope_len);
  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - LEVEL_CHUNK_CNT3_HEADER);
    return chunk_size_expected;
  }

  for (i = 0; i < envelope_len; i++)
    level->envelope[envelope_nr].text[i] = getFile8Bit(file);

  return chunk_size;
}

static int LoadLevel_CUS1(File *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = 2 + num_changed_custom_elements * 6;
  int i;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getMappedElement(getFile16BitBE(file));
    int properties = getFile32BitBE(file);

    if (IS_CUSTOM_ELEMENT(element))
      element_info[element].properties[EP_BITFIELD_BASE_NR] = properties;
    else
      Warn("invalid custom element number %d", element);

    // older game versions that wrote level files with CUS1 chunks used
    // different default push delay values (not yet stored in level file)
    element_info[element].push_delay_fixed = 2;
    element_info[element].push_delay_random = 8;
  }

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_CUS2(File *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = 2 + num_changed_custom_elements * 4;
  int i;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getMappedElement(getFile16BitBE(file));
    int custom_target_element = getMappedElement(getFile16BitBE(file));

    if (IS_CUSTOM_ELEMENT(element))
      element_info[element].change->target_element = custom_target_element;
    else
      Warn("invalid custom element number %d", element);
  }

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_CUS3(File *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = LEVEL_CHUNK_CUS3_SIZE(num_changed_custom_elements);
  int i, j, x, y;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getMappedElement(getFile16BitBE(file));
    struct ElementInfo *ei = &element_info[element];
    unsigned int event_bits;

    if (!IS_CUSTOM_ELEMENT(element))
    {
      Warn("invalid custom element number %d", element);

      element = EL_INTERNAL_DUMMY;
    }

    for (j = 0; j < MAX_ELEMENT_NAME_LEN; j++)
      ei->description[j] = getFile8Bit(file);
    ei->description[MAX_ELEMENT_NAME_LEN] = 0;

    ei->properties[EP_BITFIELD_BASE_NR] = getFile32BitBE(file);

    // some free bytes for future properties and padding
    ReadUnusedBytesFromFile(file, 7);

    ei->use_gfx_element = getFile8Bit(file);
    ei->gfx_element_initial = getMappedElement(getFile16BitBE(file));

    ei->collect_score_initial = getFile8Bit(file);
    ei->collect_count_initial = getFile8Bit(file);

    ei->push_delay_fixed = getFile16BitBE(file);
    ei->push_delay_random = getFile16BitBE(file);
    ei->move_delay_fixed = getFile16BitBE(file);
    ei->move_delay_random = getFile16BitBE(file);

    ei->move_pattern = getFile16BitBE(file);
    ei->move_direction_initial = getFile8Bit(file);
    ei->move_stepsize = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	ei->content.e[x][y] = getMappedElement(getFile16BitBE(file));

    // bits 0 - 31 of "has_event[]"
    event_bits = getFile32BitBE(file);
    for (j = 0; j < MIN(NUM_CHANGE_EVENTS, 32); j++)
      if (event_bits & (1u << j))
	ei->change->has_event[j] = TRUE;

    ei->change->target_element = getMappedElement(getFile16BitBE(file));

    ei->change->delay_fixed = getFile16BitBE(file);
    ei->change->delay_random = getFile16BitBE(file);
    ei->change->delay_frames = getFile16BitBE(file);

    ei->change->initial_trigger_element = getMappedElement(getFile16BitBE(file));

    ei->change->explode = getFile8Bit(file);
    ei->change->use_target_content = getFile8Bit(file);
    ei->change->only_if_complete = getFile8Bit(file);
    ei->change->use_random_replace = getFile8Bit(file);

    ei->change->random_percentage = getFile8Bit(file);
    ei->change->replace_when = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	ei->change->target_content.e[x][y] =
	  getMappedElement(getFile16BitBE(file));

    ei->slippery_type = getFile8Bit(file);

    // some free bytes for future properties and padding
    ReadUnusedBytesFromFile(file, LEVEL_CPART_CUS3_UNUSED);

    // mark that this custom element has been modified
    ei->modified_settings = TRUE;
  }

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_CUS4(File *file, int chunk_size, struct LevelInfo *level)
{
  struct ElementInfo *ei;
  int chunk_size_expected;
  int element;
  int i, j, x, y;

  // ---------- custom element base property values (96 bytes) ----------------

  element = getMappedElement(getFile16BitBE(file));

  if (!IS_CUSTOM_ELEMENT(element))
  {
    Warn("invalid custom element number %d", element);

    ReadUnusedBytesFromFile(file, chunk_size - 2);

    return chunk_size;
  }

  ei = &element_info[element];

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    ei->description[i] = getFile8Bit(file);
  ei->description[MAX_ELEMENT_NAME_LEN] = 0;

  ei->properties[EP_BITFIELD_BASE_NR] = getFile32BitBE(file);

  ReadUnusedBytesFromFile(file, 4);	// reserved for more base properties

  ei->num_change_pages = getFile8Bit(file);

  chunk_size_expected = LEVEL_CHUNK_CUS4_SIZE(ei->num_change_pages);
  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 43);
    return chunk_size_expected;
  }

  ei->ce_value_fixed_initial = getFile16BitBE(file);
  ei->ce_value_random_initial = getFile16BitBE(file);
  ei->use_last_ce_value = getFile8Bit(file);

  ei->use_gfx_element = getFile8Bit(file);
  ei->gfx_element_initial = getMappedElement(getFile16BitBE(file));

  ei->collect_score_initial = getFile8Bit(file);
  ei->collect_count_initial = getFile8Bit(file);

  ei->drop_delay_fixed = getFile8Bit(file);
  ei->push_delay_fixed = getFile8Bit(file);
  ei->drop_delay_random = getFile8Bit(file);
  ei->push_delay_random = getFile8Bit(file);
  ei->move_delay_fixed = getFile16BitBE(file);
  ei->move_delay_random = getFile16BitBE(file);

  // bits 0 - 15 of "move_pattern" ...
  ei->move_pattern = getFile16BitBE(file);
  ei->move_direction_initial = getFile8Bit(file);
  ei->move_stepsize = getFile8Bit(file);

  ei->slippery_type = getFile8Bit(file);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      ei->content.e[x][y] = getMappedElement(getFile16BitBE(file));

  ei->move_enter_element = getMappedElement(getFile16BitBE(file));
  ei->move_leave_element = getMappedElement(getFile16BitBE(file));
  ei->move_leave_type = getFile8Bit(file);

  // ... bits 16 - 31 of "move_pattern" (not nice, but downward compatible)
  ei->move_pattern |= (getFile16BitBE(file) << 16);

  ei->access_direction = getFile8Bit(file);

  ei->explosion_delay = getFile8Bit(file);
  ei->ignition_delay = getFile8Bit(file);
  ei->explosion_type = getFile8Bit(file);

  // some free bytes for future custom property values and padding
  ReadUnusedBytesFromFile(file, 1);

  // ---------- change page property values (48 bytes) ------------------------

  setElementChangePages(ei, ei->num_change_pages);

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];
    unsigned int event_bits;

    // always start with reliable default values
    setElementChangeInfoToDefaults(change);

    // bits 0 - 31 of "has_event[]" ...
    event_bits = getFile32BitBE(file);
    for (j = 0; j < MIN(NUM_CHANGE_EVENTS, 32); j++)
      if (event_bits & (1u << j))
	change->has_event[j] = TRUE;

    change->target_element = getMappedElement(getFile16BitBE(file));

    change->delay_fixed = getFile16BitBE(file);
    change->delay_random = getFile16BitBE(file);
    change->delay_frames = getFile16BitBE(file);

    change->initial_trigger_element = getMappedElement(getFile16BitBE(file));

    change->explode = getFile8Bit(file);
    change->use_target_content = getFile8Bit(file);
    change->only_if_complete = getFile8Bit(file);
    change->use_random_replace = getFile8Bit(file);

    change->random_percentage = getFile8Bit(file);
    change->replace_when = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	change->target_content.e[x][y]= getMappedElement(getFile16BitBE(file));

    change->can_change = getFile8Bit(file);

    change->trigger_side = getFile8Bit(file);

    change->trigger_player = getFile8Bit(file);
    change->trigger_page = getFile8Bit(file);

    change->trigger_page = (change->trigger_page == CH_PAGE_ANY_FILE ?
			    CH_PAGE_ANY : (1 << change->trigger_page));

    change->has_action = getFile8Bit(file);
    change->action_type = getFile8Bit(file);
    change->action_mode = getFile8Bit(file);
    change->action_arg = getFile16BitBE(file);

    // ... bits 32 - 39 of "has_event[]" (not nice, but downward compatible)
    event_bits = getFile8Bit(file);
    for (j = 32; j < NUM_CHANGE_EVENTS; j++)
      if (event_bits & (1u << (j - 32)))
	change->has_event[j] = TRUE;
  }

  // mark this custom element as modified
  ei->modified_settings = TRUE;

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_GRP1(File *file, int chunk_size, struct LevelInfo *level)
{
  struct ElementInfo *ei;
  struct ElementGroupInfo *group;
  int element;
  int i;

  element = getMappedElement(getFile16BitBE(file));

  if (!IS_GROUP_ELEMENT(element))
  {
    Warn("invalid group element number %d", element);

    ReadUnusedBytesFromFile(file, chunk_size - 2);

    return chunk_size;
  }

  ei = &element_info[element];

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    ei->description[i] = getFile8Bit(file);
  ei->description[MAX_ELEMENT_NAME_LEN] = 0;

  group = element_info[element].group;

  group->num_elements = getFile8Bit(file);

  ei->use_gfx_element = getFile8Bit(file);
  ei->gfx_element_initial = getMappedElement(getFile16BitBE(file));

  group->choice_mode = getFile8Bit(file);

  // some free bytes for future values and padding
  ReadUnusedBytesFromFile(file, 3);

  for (i = 0; i < MAX_ELEMENTS_IN_GROUP; i++)
    group->element[i] = getMappedElement(getFile16BitBE(file));

  // mark this group element as modified
  element_info[element].modified_settings = TRUE;

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_MicroChunk(File *file, struct LevelFileConfigInfo *conf,
				int element, int real_element)
{
  int micro_chunk_size = 0;
  int conf_type = getFile8Bit(file);
  int byte_mask = conf_type & CONF_MASK_BYTES;
  boolean element_found = FALSE;
  int i;

  micro_chunk_size += 1;

  if (byte_mask == CONF_MASK_MULTI_BYTES)
  {
    int num_bytes = getFile16BitBE(file);
    byte *buffer = checked_malloc(num_bytes);

    ReadBytesFromFile(file, buffer, num_bytes);

    for (i = 0; conf[i].data_type != -1; i++)
    {
      if (conf[i].element == element &&
	  conf[i].conf_type == conf_type)
      {
	int data_type = conf[i].data_type;
	int num_entities = num_bytes / CONF_ENTITY_NUM_BYTES(data_type);
	int max_num_entities = conf[i].max_num_entities;

	if (num_entities > max_num_entities)
	{
	  Warn("truncating number of entities for element %d from %d to %d",
	       element, num_entities, max_num_entities);

	  num_entities = max_num_entities;
	}

	if (num_entities == 0 && (data_type == TYPE_ELEMENT_LIST ||
				  data_type == TYPE_CONTENT_LIST))
	{
	  // for element and content lists, zero entities are not allowed
	  Warn("found empty list of entities for element %d", element);

	  // do not set "num_entities" here to prevent reading behind buffer

	  *(int *)(conf[i].num_entities) = 1;	// at least one is required
	}
	else
	{
	  *(int *)(conf[i].num_entities) = num_entities;
	}

	element_found = TRUE;

	if (data_type == TYPE_STRING)
	{
	  char *string = (char *)(conf[i].value);
	  int j;

	  for (j = 0; j < max_num_entities; j++)
	    string[j] = (j < num_entities ? buffer[j] : '\0');
	}
	else if (data_type == TYPE_ELEMENT_LIST)
	{
	  int *element_array = (int *)(conf[i].value);
	  int j;

	  for (j = 0; j < num_entities; j++)
	    element_array[j] =
	      getMappedElement(CONF_ELEMENTS_ELEMENT(buffer, j));
	}
	else if (data_type == TYPE_CONTENT_LIST)
	{
	  struct Content *content= (struct Content *)(conf[i].value);
	  int c, x, y;

	  for (c = 0; c < num_entities; c++)
	    for (y = 0; y < 3; y++)
	      for (x = 0; x < 3; x++)
		content[c].e[x][y] =
		  getMappedElement(CONF_CONTENTS_ELEMENT(buffer, c, x, y));
	}
	else
	  element_found = FALSE;

	break;
      }
    }

    checked_free(buffer);

    micro_chunk_size += 2 + num_bytes;
  }
  else		// constant size configuration data (1, 2 or 4 bytes)
  {
    int value = (byte_mask == CONF_MASK_1_BYTE ? (byte)  getFile8Bit   (file) :
		 byte_mask == CONF_MASK_2_BYTE ? (short) getFile16BitBE(file) :
		 byte_mask == CONF_MASK_4_BYTE ? (int)   getFile32BitBE(file) : 0);

    for (i = 0; conf[i].data_type != -1; i++)
    {
      if (conf[i].element == element &&
	  conf[i].conf_type == conf_type)
      {
	int data_type = conf[i].data_type;

	if (data_type == TYPE_ELEMENT)
	  value = getMappedElement(value);

	if (data_type == TYPE_BOOLEAN)
	  *(boolean *)(conf[i].value) = (value ? TRUE : FALSE);
	else
	  *(int *)    (conf[i].value) = value;

	element_found = TRUE;

	break;
      }
    }

    micro_chunk_size += CONF_VALUE_NUM_BYTES(byte_mask);
  }

  if (!element_found)
  {
    char *error_conf_chunk_bytes =
      (byte_mask == CONF_MASK_1_BYTE ? "CONF_VALUE_8_BIT" :
       byte_mask == CONF_MASK_2_BYTE ? "CONF_VALUE_16_BIT" :
       byte_mask == CONF_MASK_4_BYTE ? "CONF_VALUE_32_BIT" :"CONF_VALUE_BYTES");
    int error_conf_chunk_token = conf_type & CONF_MASK_TOKEN;
    int error_element = real_element;

    Warn("cannot load micro chunk '%s(%d)' value for element %d ['%s']",
	 error_conf_chunk_bytes, error_conf_chunk_token,
	 error_element, EL_NAME(error_element));
  }

  return micro_chunk_size;
}

static int LoadLevel_INFO(File *file, int chunk_size, struct LevelInfo *level)
{
  int real_chunk_size = 0;

  li = *level;		// copy level data into temporary buffer

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_INFO, -1, -1);

    if (real_chunk_size >= chunk_size)
      break;
  }

  *level = li;		// copy temporary buffer back to level data

  return real_chunk_size;
}

static int LoadLevel_CONF(File *file, int chunk_size, struct LevelInfo *level)
{
  int real_chunk_size = 0;

  li = *level;		// copy level data into temporary buffer

  while (!checkEndOfFile(file))
  {
    int element = getMappedElement(getFile16BitBE(file));

    real_chunk_size += 2;
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_CONF,
					    element, element);
    if (real_chunk_size >= chunk_size)
      break;
  }

  *level = li;		// copy temporary buffer back to level data

  return real_chunk_size;
}

static int LoadLevel_ELEM(File *file, int chunk_size, struct LevelInfo *level)
{
  int real_chunk_size = 0;

  li = *level;		// copy level data into temporary buffer

  while (!checkEndOfFile(file))
  {
    int element = getMappedElement(getFile16BitBE(file));

    real_chunk_size += 2;
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_ELEM,
					    element, element);
    if (real_chunk_size >= chunk_size)
      break;
  }

  *level = li;		// copy temporary buffer back to level data

  return real_chunk_size;
}

static int LoadLevel_NOTE(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int envelope_nr = element - EL_ENVELOPE_1;
  int real_chunk_size = 2;

  xx_envelope = level->envelope[envelope_nr];	// copy into temporary buffer

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_NOTE,
					    -1, element);

    if (real_chunk_size >= chunk_size)
      break;
  }

  level->envelope[envelope_nr] = xx_envelope;	// copy from temporary buffer

  return real_chunk_size;
}

static int LoadLevel_CUSX(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int real_chunk_size = 2;
  struct ElementInfo *ei = &element_info[element];
  int i;

  xx_ei = *ei;		// copy element data into temporary buffer

  xx_ei.num_change_pages = -1;

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_CUSX_base,
					    -1, element);
    if (xx_ei.num_change_pages != -1)
      break;

    if (real_chunk_size >= chunk_size)
      break;
  }

  *ei = xx_ei;

  if (ei->num_change_pages == -1)
  {
    Warn("LoadLevel_CUSX(): missing 'num_change_pages' for '%s'",
	 EL_NAME(element));

    ei->num_change_pages = 1;

    setElementChangePages(ei, 1);
    setElementChangeInfoToDefaults(ei->change);

    return real_chunk_size;
  }

  // initialize number of change pages stored for this custom element
  setElementChangePages(ei, ei->num_change_pages);
  for (i = 0; i < ei->num_change_pages; i++)
    setElementChangeInfoToDefaults(&ei->change_page[i]);

  // start with reading properties for the first change page
  xx_current_change_page = 0;

  while (!checkEndOfFile(file))
  {
    // level file might contain invalid change page number
    if (xx_current_change_page >= ei->num_change_pages)
      break;

    struct ElementChangeInfo *change = &ei->change_page[xx_current_change_page];

    xx_change = *change;	// copy change data into temporary buffer

    resetEventBits();		// reset bits; change page might have changed

    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_CUSX_change,
					    -1, element);

    *change = xx_change;

    setEventFlagsFromEventBits(change);

    if (real_chunk_size >= chunk_size)
      break;
  }

  level->file_has_custom_elements = TRUE;

  return real_chunk_size;
}

static int LoadLevel_GRPX(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int real_chunk_size = 2;
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;

  if (group == NULL)
    return -1;

  xx_ei = *ei;		// copy element data into temporary buffer
  xx_group = *group;	// copy group data into temporary buffer

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_GRPX,
					    -1, element);

    if (real_chunk_size >= chunk_size)
      break;
  }

  *ei = xx_ei;
  *group = xx_group;

  level->file_has_custom_elements = TRUE;

  return real_chunk_size;
}

static int LoadLevel_EMPX(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int real_chunk_size = 2;
  struct ElementInfo *ei = &element_info[element];

  xx_ei = *ei;		// copy element data into temporary buffer

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_EMPX,
					    -1, element);

    if (real_chunk_size >= chunk_size)
      break;
  }

  *ei = xx_ei;

  level->file_has_custom_elements = TRUE;

  return real_chunk_size;
}

static void LoadLevelFromFileInfo_RND(struct LevelInfo *level,
				      struct LevelFileInfo *level_file_info,
				      boolean level_info_only)
{
  char *filename = level_file_info->filename;
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  File *file;

  if (!(file = openFile(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;
    level->no_level_file = TRUE;

    if (level_info_only)
      return;

    Warn("cannot read level '%s' -- using empty level", filename);

    if (!setup.editor.use_template_for_new_levels)
      return;

    // if level file not found, try to initialize level data from template
    filename = getGlobalLevelTemplateFilename();

    if (!(file = openFile(filename, MODE_READ)))
      return;

    // default: for empty levels, use level template for custom elements
    level->use_custom_template = TRUE;

    level->no_valid_file = FALSE;
  }

  getFileChunkBE(file, chunk_name, NULL);
  if (strEqual(chunk_name, "RND1"))
  {
    getFile32BitBE(file);		// not used

    getFileChunkBE(file, chunk_name, NULL);
    if (!strEqual(chunk_name, "CAVE"))
    {
      level->no_valid_file = TRUE;

      Warn("unknown format of level file '%s'", filename);

      closeFile(file);

      return;
    }
  }
  else	// check for pre-2.0 file format with cookie string
  {
    strcpy(cookie, chunk_name);
    if (getStringFromFile(file, &cookie[4], MAX_LINE_LEN - 4) == NULL)
      cookie[4] = '\0';
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, LEVEL_COOKIE_TMPL))
    {
      level->no_valid_file = TRUE;

      Warn("unknown format of level file '%s'", filename);

      closeFile(file);

      return;
    }

    if ((level->file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      level->no_valid_file = TRUE;

      Warn("unsupported version of level file '%s'", filename);

      closeFile(file);

      return;
    }

    // pre-2.0 level files have no game version, so use file version here
    level->game_version = level->file_version;
  }

  if (level->file_version < FILE_VERSION_1_2)
  {
    // level files from versions before 1.2.0 without chunk structure
    LoadLevel_HEAD(file, LEVEL_CHUNK_HEAD_SIZE,         level);
    LoadLevel_BODY(file, level->fieldx * level->fieldy, level);
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(File *, int, struct LevelInfo *);
    }
    chunk_info[] =
    {
      { "VERS", LEVEL_CHUNK_VERS_SIZE,	LoadLevel_VERS },
      { "DATE", LEVEL_CHUNK_DATE_SIZE,	LoadLevel_DATE },
      { "HEAD", LEVEL_CHUNK_HEAD_SIZE,	LoadLevel_HEAD },
      { "NAME", LEVEL_CHUNK_NAME_SIZE,	LoadLevel_NAME },
      { "AUTH", LEVEL_CHUNK_AUTH_SIZE,	LoadLevel_AUTH },
      { "INFO", -1,			LoadLevel_INFO },
      { "BODY", -1,			LoadLevel_BODY },
      { "CONT", -1,			LoadLevel_CONT },
      { "CNT2", LEVEL_CHUNK_CNT2_SIZE,	LoadLevel_CNT2 },
      { "CNT3", -1,			LoadLevel_CNT3 },
      { "CUS1", -1,			LoadLevel_CUS1 },
      { "CUS2", -1,			LoadLevel_CUS2 },
      { "CUS3", -1,			LoadLevel_CUS3 },
      { "CUS4", -1,			LoadLevel_CUS4 },
      { "GRP1", -1,			LoadLevel_GRP1 },
      { "CONF", -1,			LoadLevel_CONF },
      { "ELEM", -1,			LoadLevel_ELEM },
      { "NOTE", -1,			LoadLevel_NOTE },
      { "CUSX", -1,			LoadLevel_CUSX },
      { "GRPX", -1,			LoadLevel_GRPX },
      { "EMPX", -1,			LoadLevel_EMPX },

      {  NULL,  0,			NULL }
    };

    while (getFileChunkBE(file, chunk_name, &chunk_size))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     !strEqual(chunk_name, chunk_info[i].name))
	i++;

      if (chunk_info[i].name == NULL)
      {
	Warn("unknown chunk '%s' in level file '%s'",
	     chunk_name, filename);

	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Warn("wrong size (%d) of chunk '%s' in level file '%s'",
	     chunk_size, chunk_name, filename);

	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	// call function to load this level chunk
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, level);

	if (chunk_size_expected < 0)
	{
	  Warn("error reading chunk '%s' in level file '%s'",
	       chunk_name, filename);

	  break;
	}

	// the size of some chunks cannot be checked before reading other
	// chunks first (like "HEAD" and "BODY") that contain some header
	// information, so check them here
	if (chunk_size_expected != chunk_size)
	{
	  Warn("wrong size (%d) of chunk '%s' in level file '%s'",
	       chunk_size, chunk_name, filename);

	  break;
	}
      }
    }
  }

  closeFile(file);
}


// ----------------------------------------------------------------------------
// functions for loading BD level
// ----------------------------------------------------------------------------

#define LEVEL_TO_CAVE(e)	(map_element_RND_to_BD_cave(e))
#define CAVE_TO_LEVEL(e)	(map_element_BD_to_RND_cave(e))

static void CopyNativeLevel_RND_to_BD(struct LevelInfo *level)
{
  struct LevelInfo_BD *level_bd = level->native_bd_level;
  GdCave *cave = NULL;	// will be changed below
  int cave_w = MIN(level->fieldx, MAX_PLAYFIELD_WIDTH);
  int cave_h = MIN(level->fieldy, MAX_PLAYFIELD_HEIGHT);
  int i, x, y;

  setLevelInfoToDefaults_BD_Ext(cave_w, cave_h);

  // cave and map newly allocated when set to defaults above
  cave = level_bd->cave;

  // level type
  cave->intermission			= level->bd_intermission;

  // level clipping
  if (level->bd_intermission && level->bd_intermission_clipped)
  {
    cave->x1 = 0;
    cave->y1 = 0;
    cave->x2 = MIN(19, cave->w - 1);
    cave->y2 = MIN(11, cave->h - 1);
  }

  // level settings
  cave->level_time[0]			= level->time;
  cave->level_diamonds[0]		= level->gems_needed;

  // game timing
  cave->scheduling			= level->bd_scheduling_type;
  cave->pal_timing			= level->bd_pal_timing;
  cave->level_speed[0]			= level->bd_cycle_delay_ms;
  cave->level_ckdelay[0]		= level->bd_cycle_delay_c64;
  cave->level_hatching_delay_frame[0]	= level->bd_hatching_delay_cycles;
  cave->level_hatching_delay_time[0]	= level->bd_hatching_delay_seconds;

  // scores
  cave->level_timevalue[0]		= level->score[SC_TIME_BONUS];
  cave->diamond_value			= level->score[SC_EMERALD];
  cave->extra_diamond_value		= level->score[SC_DIAMOND_EXTRA];

  // compatibility settings
  cave->lineshift			= level->bd_line_shifting_borders;
  cave->border_scan_first_and_last	= level->bd_scan_first_and_last_row;
  cave->short_explosions		= level->bd_short_explosions;

  // player properties
  cave->diagonal_movements		= level->bd_diagonal_movements;
  cave->active_is_first_found		= level->bd_topmost_player_active;
  cave->pushing_stone_prob		= level->bd_pushing_prob            * 10000;
  cave->pushing_stone_prob_sweet	= level->bd_pushing_prob_with_sweet * 10000;
  cave->mega_stones_pushable_with_sweet	= level->bd_push_heavy_rock_with_sweet;
  cave->snap_element			= LEVEL_TO_CAVE(level->bd_snap_element);

  // element properties
  cave->level_bonus_time[0]		= level->bd_clock_extra_time;
  cave->voodoo_collects_diamonds	= level->bd_voodoo_collects_diamonds;
  cave->voodoo_any_hurt_kills_player	= level->bd_voodoo_hurt_kills_player;
  cave->voodoo_dies_by_stone		= level->bd_voodoo_dies_by_rock;
  cave->voodoo_disappear_in_explosion	= level->bd_voodoo_vanish_by_explosion;
  cave->level_penalty_time[0]		= level->bd_voodoo_penalty_time;
  cave->level_magic_wall_time[0]	= level->bd_magic_wall_time;
  cave->magic_timer_zero_is_infinite	= level->bd_magic_wall_zero_infinite;
  cave->magic_timer_wait_for_hatching	= level->bd_magic_wall_wait_hatching;
  cave->magic_wall_stops_amoeba		= level->bd_magic_wall_stops_amoeba;
  cave->magic_wall_breakscan		= level->bd_magic_wall_break_scan;

  cave->magic_diamond_to		= LEVEL_TO_CAVE(level->bd_magic_wall_diamond_to);
  cave->magic_stone_to			= LEVEL_TO_CAVE(level->bd_magic_wall_rock_to);
  cave->magic_mega_stone_to		= LEVEL_TO_CAVE(level->bd_magic_wall_heavy_rock_to);
  cave->magic_light_stone_to		= LEVEL_TO_CAVE(level->bd_magic_wall_light_rock_to);
  cave->magic_nut_to			= LEVEL_TO_CAVE(level->bd_magic_wall_nut_to);
  cave->magic_nitro_pack_to		= LEVEL_TO_CAVE(level->bd_magic_wall_nitro_pack_to);
  cave->magic_flying_diamond_to		= LEVEL_TO_CAVE(level->bd_magic_wall_flying_diamond_to);
  cave->magic_flying_stone_to		= LEVEL_TO_CAVE(level->bd_magic_wall_flying_rock_to);

  cave->amoeba_timer_wait_for_hatching	= level->bd_amoeba_wait_for_hatching;
  cave->amoeba_timer_started_immediately= level->bd_amoeba_start_immediately;
  cave->amoeba_2_explodes_by_amoeba	= level->bd_amoeba_2_explode_by_amoeba;
  cave->level_amoeba_threshold[0]	= level->bd_amoeba_1_threshold_too_big;
  cave->level_amoeba_time[0]		= level->bd_amoeba_1_slow_growth_time;
  cave->amoeba_growth_prob		= level->bd_amoeba_1_slow_growth_rate * 10000;
  cave->amoeba_fast_growth_prob		= level->bd_amoeba_1_fast_growth_rate * 10000;
  cave->level_amoeba_2_threshold[0]	= level->bd_amoeba_2_threshold_too_big;
  cave->level_amoeba_2_time[0]		= level->bd_amoeba_2_slow_growth_time;
  cave->amoeba_2_growth_prob		= level->bd_amoeba_2_slow_growth_rate * 10000;
  cave->amoeba_2_fast_growth_prob	= level->bd_amoeba_2_fast_growth_rate * 10000;

  cave->amoeba_too_big_effect		= LEVEL_TO_CAVE(level->bd_amoeba_1_content_too_big);
  cave->amoeba_enclosed_effect		= LEVEL_TO_CAVE(level->bd_amoeba_1_content_enclosed);
  cave->amoeba_2_too_big_effect		= LEVEL_TO_CAVE(level->bd_amoeba_2_content_too_big);
  cave->amoeba_2_enclosed_effect	= LEVEL_TO_CAVE(level->bd_amoeba_2_content_enclosed);
  cave->amoeba_2_explosion_effect	= LEVEL_TO_CAVE(level->bd_amoeba_2_content_exploding);
  cave->amoeba_2_looks_like		= LEVEL_TO_CAVE(level->bd_amoeba_2_content_looks_like);

  cave->slime_predictable		= level->bd_slime_is_predictable;
  cave->slime_correct_random		= level->bd_slime_correct_random;
  cave->level_slime_permeability[0]	= level->bd_slime_permeability_rate * 10000;
  cave->level_slime_permeability_c64[0]	= level->bd_slime_permeability_bits_c64;
  cave->level_slime_seed_c64[0]		= level->bd_slime_random_seed_c64;
  cave->level_rand[0]			= level->bd_cave_random_seed_c64;
  cave->slime_eats_1			= LEVEL_TO_CAVE(level->bd_slime_eats_element_1);
  cave->slime_converts_1		= LEVEL_TO_CAVE(level->bd_slime_converts_to_element_1);
  cave->slime_eats_2			= LEVEL_TO_CAVE(level->bd_slime_eats_element_2);
  cave->slime_converts_2		= LEVEL_TO_CAVE(level->bd_slime_converts_to_element_2);
  cave->slime_eats_3			= LEVEL_TO_CAVE(level->bd_slime_eats_element_3);
  cave->slime_converts_3		= LEVEL_TO_CAVE(level->bd_slime_converts_to_element_3);

  cave->acid_eats_this			= LEVEL_TO_CAVE(level->bd_acid_eats_element);
  cave->acid_spread_ratio		= level->bd_acid_spread_rate * 10000;
  cave->acid_turns_to			= LEVEL_TO_CAVE(level->bd_acid_turns_to_element);

  cave->biter_delay_frame		= level->bd_biter_move_delay;
  cave->biter_eat			= LEVEL_TO_CAVE(level->bd_biter_eats_element);

  cave->bladder_converts_by		= LEVEL_TO_CAVE(level->bd_bubble_converts_by_element);

  cave->expanding_wall_changed		= level->bd_change_expanding_wall;

  cave->replicators_active		= level->bd_replicators_active;
  cave->replicator_delay_frame		= level->bd_replicator_create_delay;

  cave->conveyor_belts_active		= level->bd_conveyor_belts_active;
  cave->conveyor_belts_direction_changed= level->bd_conveyor_belts_changed;
  cave->conveyor_belts_buggy		= level->bd_conveyor_belts_buggy;

  cave->water_does_not_flow_down	= level->bd_water_cannot_flow_down;

  cave->nut_turns_to_when_crushed	= LEVEL_TO_CAVE(level->bd_nut_content);

  cave->pneumatic_hammer_frame		= level->bd_hammer_walls_break_delay;
  cave->hammered_walls_reappear		= level->bd_hammer_walls_reappear;
  cave->hammered_wall_reappear_frame	= level->bd_hammer_walls_reappear_delay;

  cave->infinite_rockets		= level->bd_infinite_rockets;

  cave->buggy_teleporter		= level->bd_buggy_teleporter;

  cave->skeletons_needed_for_pot	= level->bd_num_skeletons_needed_for_pot;
  cave->skeletons_worth_diamonds	= level->bd_skeleton_worth_num_diamonds;

  cave->expanding_wall_looks_like	= LEVEL_TO_CAVE(level->bd_expanding_wall_looks_like);
  cave->dirt_looks_like			= LEVEL_TO_CAVE(level->bd_sand_looks_like);

  cave->creatures_backwards			 = level->bd_creatures_start_backwards;
  cave->creatures_direction_auto_change_on_start = level->bd_creatures_turn_on_hatching;
  cave->creatures_direction_auto_change_time	 = level->bd_creatures_auto_turn_delay;

  cave->gravity				= level->bd_gravity_direction;
  cave->gravity_switch_active		= level->bd_gravity_switch_active;
  cave->gravity_change_time		= level->bd_gravity_switch_delay;
  cave->gravity_affects_all		= level->bd_gravity_affects_all;

  cave->stone_falling_effect		= LEVEL_TO_CAVE(level->bd_rock_turns_to_on_falling);
  cave->stone_bouncing_effect		= LEVEL_TO_CAVE(level->bd_rock_turns_to_on_impact);
  cave->diamond_falling_effect		= LEVEL_TO_CAVE(level->bd_diamond_turns_to_on_falling);
  cave->diamond_bouncing_effect		= LEVEL_TO_CAVE(level->bd_diamond_turns_to_on_impact);

  cave->firefly_explode_to		= LEVEL_TO_CAVE(level->bd_firefly_1_explodes_to);
  cave->alt_firefly_explode_to		= LEVEL_TO_CAVE(level->bd_firefly_2_explodes_to);
  cave->butterfly_explode_to		= LEVEL_TO_CAVE(level->bd_butterfly_1_explodes_to);
  cave->alt_butterfly_explode_to	= LEVEL_TO_CAVE(level->bd_butterfly_2_explodes_to);
  cave->stonefly_explode_to		= LEVEL_TO_CAVE(level->bd_stonefly_explodes_to);
  cave->dragonfly_explode_to		= LEVEL_TO_CAVE(level->bd_dragonfly_explodes_to);

  cave->diamond_birth_effect		= LEVEL_TO_CAVE(level->bd_diamond_birth_turns_to);
  cave->bomb_explosion_effect		= LEVEL_TO_CAVE(level->bd_bomb_explosion_turns_to);
  cave->nitro_explosion_effect		= LEVEL_TO_CAVE(level->bd_nitro_explosion_turns_to);
  cave->explosion_effect		= LEVEL_TO_CAVE(level->bd_explosion_turns_to);
  cave->explosion_3_effect		= LEVEL_TO_CAVE(level->bd_explosion_3_turns_to);

  for (i = 0; i < MAX_LEVEL_COLORS; i++)
  {
    cave->color[i]			= level->bd_color[i];
    cave->base_color[i]			= level->bd_base_color[i];
  }

  // level name
  strncpy(cave->name, level->name, sizeof(GdString));
  cave->name[sizeof(GdString) - 1] = '\0';

  // playfield elements
  for (x = 0; x < cave->w; x++)
    for (y = 0; y < cave->h; y++)
      cave->map[y][x] = LEVEL_TO_CAVE(level->field[x][y]);
}

static void CopyNativeLevel_BD_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_BD *level_bd = level->native_bd_level;
  GdCave *cave = level_bd->cave;
  int bd_level_nr = level_bd->level_nr;
  int i, x, y;

  level->fieldx = MIN(cave->w, MAX_LEV_FIELDX);
  level->fieldy = MIN(cave->h, MAX_LEV_FIELDY);

  // level type
  level->bd_intermission		= cave->intermission;

  // level clipping
  if (cave->intermission &&
      cave->w  == 40 &&
      cave->h  == 22 &&
      cave->x1 == 0  &&
      cave->y1 == 0  &&
      cave->x2 == 19 &&
      cave->y2 == 11)
    level->bd_intermission_clipped = TRUE;

  // level settings
  level->time				= cave->level_time[bd_level_nr];
  level->gems_needed			= cave->level_diamonds[bd_level_nr];

  // game timing
  level->bd_scheduling_type		= cave->scheduling;
  level->bd_pal_timing			= cave->pal_timing;
  level->bd_cycle_delay_ms		= cave->level_speed[bd_level_nr];
  level->bd_cycle_delay_c64		= cave->level_ckdelay[bd_level_nr];
  level->bd_hatching_delay_cycles	= cave->level_hatching_delay_frame[bd_level_nr];
  level->bd_hatching_delay_seconds	= cave->level_hatching_delay_time[bd_level_nr];

  // scores
  level->score[SC_TIME_BONUS]		= cave->level_timevalue[bd_level_nr];
  level->score[SC_EMERALD]		= cave->diamond_value;
  level->score[SC_DIAMOND_EXTRA]	= cave->extra_diamond_value;

  // compatibility settings
  level->bd_line_shifting_borders	= cave->lineshift;
  level->bd_scan_first_and_last_row	= cave->border_scan_first_and_last;
  level->bd_short_explosions		= cave->short_explosions;

  // player properties
  level->bd_diagonal_movements		= cave->diagonal_movements;
  level->bd_topmost_player_active	= cave->active_is_first_found;
  level->bd_pushing_prob		= cave->pushing_stone_prob       / 10000;
  level->bd_pushing_prob_with_sweet	= cave->pushing_stone_prob_sweet / 10000;
  level->bd_push_heavy_rock_with_sweet	= cave->mega_stones_pushable_with_sweet;
  level->bd_snap_element		= CAVE_TO_LEVEL(cave->snap_element);

  // element properties
  level->bd_clock_extra_time		= cave->level_bonus_time[bd_level_nr];
  level->bd_voodoo_collects_diamonds	= cave->voodoo_collects_diamonds;
  level->bd_voodoo_hurt_kills_player	= cave->voodoo_any_hurt_kills_player;
  level->bd_voodoo_dies_by_rock		= cave->voodoo_dies_by_stone;
  level->bd_voodoo_vanish_by_explosion	= cave->voodoo_disappear_in_explosion;
  level->bd_voodoo_penalty_time		= cave->level_penalty_time[bd_level_nr];
  level->bd_magic_wall_time		= cave->level_magic_wall_time[bd_level_nr];
  level->bd_magic_wall_zero_infinite	= cave->magic_timer_zero_is_infinite;
  level->bd_magic_wall_wait_hatching	= cave->magic_timer_wait_for_hatching;
  level->bd_magic_wall_stops_amoeba	= cave->magic_wall_stops_amoeba;
  level->bd_magic_wall_break_scan	= cave->magic_wall_breakscan;

  level->bd_magic_wall_diamond_to	= CAVE_TO_LEVEL(cave->magic_diamond_to);
  level->bd_magic_wall_rock_to		= CAVE_TO_LEVEL(cave->magic_stone_to);
  level->bd_magic_wall_heavy_rock_to	= CAVE_TO_LEVEL(cave->magic_mega_stone_to);
  level->bd_magic_wall_light_rock_to	= CAVE_TO_LEVEL(cave->magic_light_stone_to);
  level->bd_magic_wall_nut_to		= CAVE_TO_LEVEL(cave->magic_nut_to);
  level->bd_magic_wall_nitro_pack_to	= CAVE_TO_LEVEL(cave->magic_nitro_pack_to);
  level->bd_magic_wall_flying_diamond_to= CAVE_TO_LEVEL(cave->magic_flying_diamond_to);
  level->bd_magic_wall_flying_rock_to	= CAVE_TO_LEVEL(cave->magic_flying_stone_to);

  level->bd_amoeba_wait_for_hatching	= cave->amoeba_timer_wait_for_hatching;
  level->bd_amoeba_start_immediately	= cave->amoeba_timer_started_immediately;
  level->bd_amoeba_2_explode_by_amoeba	= cave->amoeba_2_explodes_by_amoeba;
  level->bd_amoeba_1_threshold_too_big	= cave->level_amoeba_threshold[bd_level_nr];
  level->bd_amoeba_1_slow_growth_time	= cave->level_amoeba_time[bd_level_nr];
  level->bd_amoeba_1_slow_growth_rate	= cave->amoeba_growth_prob      / 10000;
  level->bd_amoeba_1_fast_growth_rate	= cave->amoeba_fast_growth_prob / 10000;
  level->bd_amoeba_2_threshold_too_big	= cave->level_amoeba_2_threshold[bd_level_nr];
  level->bd_amoeba_2_slow_growth_time	= cave->level_amoeba_2_time[bd_level_nr];
  level->bd_amoeba_2_slow_growth_rate	= cave->amoeba_2_growth_prob      / 10000;
  level->bd_amoeba_2_fast_growth_rate	= cave->amoeba_2_fast_growth_prob / 10000;

  level->bd_amoeba_1_content_too_big	= CAVE_TO_LEVEL(cave->amoeba_too_big_effect);
  level->bd_amoeba_1_content_enclosed	= CAVE_TO_LEVEL(cave->amoeba_enclosed_effect);
  level->bd_amoeba_2_content_too_big	= CAVE_TO_LEVEL(cave->amoeba_2_too_big_effect);
  level->bd_amoeba_2_content_enclosed	= CAVE_TO_LEVEL(cave->amoeba_2_enclosed_effect);
  level->bd_amoeba_2_content_exploding	= CAVE_TO_LEVEL(cave->amoeba_2_explosion_effect);
  level->bd_amoeba_2_content_looks_like	= CAVE_TO_LEVEL(cave->amoeba_2_looks_like);

  level->bd_slime_is_predictable	= cave->slime_predictable;
  level->bd_slime_correct_random	= cave->slime_correct_random;
  level->bd_slime_permeability_rate	= cave->level_slime_permeability[bd_level_nr] / 10000;
  level->bd_slime_permeability_bits_c64	= cave->level_slime_permeability_c64[bd_level_nr];
  level->bd_slime_random_seed_c64	= cave->level_slime_seed_c64[bd_level_nr];
  level->bd_cave_random_seed_c64	= cave->level_rand[bd_level_nr];
  level->bd_slime_eats_element_1	= CAVE_TO_LEVEL(cave->slime_eats_1);
  level->bd_slime_converts_to_element_1	= CAVE_TO_LEVEL(cave->slime_converts_1);
  level->bd_slime_eats_element_2	= CAVE_TO_LEVEL(cave->slime_eats_2);
  level->bd_slime_converts_to_element_2	= CAVE_TO_LEVEL(cave->slime_converts_2);
  level->bd_slime_eats_element_3	= CAVE_TO_LEVEL(cave->slime_eats_3);
  level->bd_slime_converts_to_element_3	= CAVE_TO_LEVEL(cave->slime_converts_3);

  level->bd_acid_eats_element		= CAVE_TO_LEVEL(cave->acid_eats_this);
  level->bd_acid_spread_rate		= cave->acid_spread_ratio / 10000;
  level->bd_acid_turns_to_element	= CAVE_TO_LEVEL(cave->acid_turns_to);

  level->bd_biter_move_delay		= cave->biter_delay_frame;
  level->bd_biter_eats_element		= CAVE_TO_LEVEL(cave->biter_eat);

  level->bd_bubble_converts_by_element	= CAVE_TO_LEVEL(cave->bladder_converts_by);

  level->bd_change_expanding_wall	= cave->expanding_wall_changed;

  level->bd_replicators_active		= cave->replicators_active;
  level->bd_replicator_create_delay	= cave->replicator_delay_frame;

  level->bd_conveyor_belts_active	= cave->conveyor_belts_active;
  level->bd_conveyor_belts_changed	= cave->conveyor_belts_direction_changed;
  level->bd_conveyor_belts_buggy	= cave->conveyor_belts_buggy;

  level->bd_water_cannot_flow_down	= cave->water_does_not_flow_down;

  level->bd_nut_content			= CAVE_TO_LEVEL(cave->nut_turns_to_when_crushed);

  level->bd_hammer_walls_break_delay	= cave->pneumatic_hammer_frame;
  level->bd_hammer_walls_reappear	= cave->hammered_walls_reappear;
  level->bd_hammer_walls_reappear_delay	= cave->hammered_wall_reappear_frame;

  level->bd_infinite_rockets		= cave->infinite_rockets;

  level->bd_buggy_teleporter		= cave->buggy_teleporter;

  level->bd_num_skeletons_needed_for_pot= cave->skeletons_needed_for_pot;
  level->bd_skeleton_worth_num_diamonds	= cave->skeletons_worth_diamonds;

  level->bd_expanding_wall_looks_like	= CAVE_TO_LEVEL(cave->expanding_wall_looks_like);
  level->bd_sand_looks_like		= CAVE_TO_LEVEL(cave->dirt_looks_like);

  level->bd_creatures_start_backwards	= cave->creatures_backwards;
  level->bd_creatures_turn_on_hatching	= cave->creatures_direction_auto_change_on_start;
  level->bd_creatures_auto_turn_delay	= cave->creatures_direction_auto_change_time;

  level->bd_gravity_direction		= cave->gravity;
  level->bd_gravity_switch_active	= cave->gravity_switch_active;
  level->bd_gravity_switch_delay	= cave->gravity_change_time;
  level->bd_gravity_affects_all		= cave->gravity_affects_all;

  level->bd_rock_turns_to_on_falling	= CAVE_TO_LEVEL(cave->stone_falling_effect);
  level->bd_rock_turns_to_on_impact	= CAVE_TO_LEVEL(cave->stone_bouncing_effect);
  level->bd_diamond_turns_to_on_falling	= CAVE_TO_LEVEL(cave->diamond_falling_effect);
  level->bd_diamond_turns_to_on_impact	= CAVE_TO_LEVEL(cave->diamond_bouncing_effect);

  level->bd_firefly_1_explodes_to	= CAVE_TO_LEVEL(cave->firefly_explode_to);
  level->bd_firefly_2_explodes_to	= CAVE_TO_LEVEL(cave->alt_firefly_explode_to);
  level->bd_butterfly_1_explodes_to	= CAVE_TO_LEVEL(cave->butterfly_explode_to);
  level->bd_butterfly_2_explodes_to	= CAVE_TO_LEVEL(cave->alt_butterfly_explode_to);
  level->bd_stonefly_explodes_to	= CAVE_TO_LEVEL(cave->stonefly_explode_to);
  level->bd_dragonfly_explodes_to	= CAVE_TO_LEVEL(cave->dragonfly_explode_to);

  level->bd_diamond_birth_turns_to	= CAVE_TO_LEVEL(cave->diamond_birth_effect);
  level->bd_bomb_explosion_turns_to	= CAVE_TO_LEVEL(cave->bomb_explosion_effect);
  level->bd_nitro_explosion_turns_to	= CAVE_TO_LEVEL(cave->nitro_explosion_effect);
  level->bd_explosion_turns_to		= CAVE_TO_LEVEL(cave->explosion_effect);
  level->bd_explosion_3_turns_to	= CAVE_TO_LEVEL(cave->explosion_3_effect);

  for (i = 0; i < MAX_LEVEL_COLORS; i++)
  {
    level->bd_color[i]			= cave->color[i];
    level->bd_base_color[i]		= cave->base_color[i];
  }

  // set default template coloring type for BD style level colors
  level->bd_coloring_type = GD_COLORING_TYPE_SINGLE;

  // set default color type and colors for BD style level colors
  SetDefaultLevelColorType_BD();
  SetDefaultLevelColors_BD();

  // level name
  char *cave_name_latin1 = getLatin1FromUTF8(cave->name);
  char *cave_name_final = (gd_caveset_has_levels() ?
                           getStringPrint("%s / %d", cave_name_latin1, bd_level_nr + 1) :
                           getStringCopy(cave_name_latin1));

  strncpy(level->name_native, cave_name_latin1, MAX_OUTPUT_LINESIZE);
  level->name_native[MAX_OUTPUT_LINESIZE] = '\0';

  strncpy(level->name, cave_name_final, MAX_LEVEL_NAME_LEN);
  level->name[MAX_LEVEL_NAME_LEN] = '\0';

  // playfield elements
  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      level->field[x][y] = CAVE_TO_LEVEL(cave->map[y][x]);

  checked_free(cave_name_latin1);
  checked_free(cave_name_final);
}

static void setTapeInfoToDefaults(void);

static void CopyNativeTape_BD_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_BD *level_bd = level->native_bd_level;
  GdCave *cave = level_bd->cave;
  GdReplay *replay = level_bd->replay;
  int i;

  if (replay == NULL)
    return;

  // always start with reliable default values
  setTapeInfoToDefaults();

  tape.level_nr = level_nr;		// (currently not used)
  tape.random_seed = replay->seed;
  tape.solved = replay->success;

  TapeSetDateFromIsoDateString(replay->date);

  tape.counter = 0;
  tape.pos[tape.counter].delay = 0;

  tape.bd_replay = TRUE;

  // use old BD game engine if playing specifically tagged BD replays
  if (leveldir_current->replay_with_old_engine)
    tape.property_bits |= TAPE_PROPERTY_BD_OLD_ENGINE;

  // all time calculations only used to display approximate tape time
  int cave_speed = cave->speed;
  int milliseconds_game = 0;
  int milliseconds_elapsed = 20;

  for (i = 0; i < replay->movements->len; i++)
  {
    int replay_action = replay->movements->data[i];
    int tape_action = map_action_BD_to_RND(replay_action);
    byte action[MAX_TAPE_ACTIONS] = { tape_action };
    boolean success = 0;

    while (1)
    {
      success = TapeAddAction(action);

      milliseconds_game += milliseconds_elapsed;

      if (milliseconds_game >= cave_speed)
      {
	milliseconds_game -= cave_speed;

	break;
      }
    }

    tape.counter++;
    tape.pos[tape.counter].delay = 0;
    tape.pos[tape.counter].action[0] = 0;

    if (!success)
    {
      Warn("BD replay truncated: size exceeds maximum tape size %d", MAX_TAPE_LEN);

      break;
    }
  }

  TapeHaltRecording();

  if (!replay->success)
    Warn("BD replay is marked as not successful");
}


// ----------------------------------------------------------------------------
// functions for loading EM level
// ----------------------------------------------------------------------------

static void CopyNativeLevel_RND_to_EM(struct LevelInfo *level)
{
  static int ball_xy[8][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 },
    { 0, 1 },
    { 2, 1 },
    { 0, 2 },
    { 1, 2 },
    { 2, 2 },
  };
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct CAVE *cav = level_em->cav;
  int i, j, x, y;

  cav->width  = MIN(level->fieldx, MAX_PLAYFIELD_WIDTH);
  cav->height = MIN(level->fieldy, MAX_PLAYFIELD_HEIGHT);

  cav->time_seconds	= level->time;
  cav->gems_needed	= level->gems_needed;

  cav->emerald_score	= level->score[SC_EMERALD];
  cav->diamond_score	= level->score[SC_DIAMOND];
  cav->alien_score	= level->score[SC_ROBOT];
  cav->tank_score	= level->score[SC_SPACESHIP];
  cav->bug_score	= level->score[SC_BUG];
  cav->eater_score	= level->score[SC_YAMYAM];
  cav->nut_score	= level->score[SC_NUT];
  cav->dynamite_score	= level->score[SC_DYNAMITE];
  cav->key_score	= level->score[SC_KEY];
  cav->exit_score	= level->score[SC_TIME_BONUS];

  cav->num_eater_arrays	= level->num_yamyam_contents;

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	cav->eater_array[i][y * 3 + x] =
	  map_element_RND_to_EM_cave(level->yamyam_content[i].e[x][y]);

  cav->amoeba_time		= level->amoeba_speed;
  cav->wonderwall_time		= level->time_magic_wall;
  cav->wheel_time		= level->time_wheel;

  cav->android_move_time	= level->android_move_time;
  cav->android_clone_time	= level->android_clone_time;
  cav->ball_random		= level->ball_random;
  cav->ball_active		= level->ball_active_initial;
  cav->ball_time		= level->ball_time;
  cav->num_ball_arrays		= level->num_ball_contents;

  cav->lenses_score		= level->lenses_score;
  cav->magnify_score		= level->magnify_score;
  cav->slurp_score		= level->slurp_score;

  cav->lenses_time		= level->lenses_time;
  cav->magnify_time		= level->magnify_time;

  cav->wind_time = 9999;
  cav->wind_direction =
    map_direction_RND_to_EM(level->wind_direction_initial);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (j = 0; j < 8; j++)
      cav->ball_array[i][j] =
	map_element_RND_to_EM_cave(level->ball_content[i].
				   e[ball_xy[j][0]][ball_xy[j][1]]);

  map_android_clone_elements_RND_to_EM(level);

  // first fill the complete playfield with the empty space element
  for (y = 0; y < EM_MAX_CAVE_HEIGHT; y++)
    for (x = 0; x < EM_MAX_CAVE_WIDTH; x++)
      cav->cave[x][y] = Cblank;

  // then copy the real level contents from level file into the playfield
  for (y = 0; y < cav->height; y++) for (x = 0; x < cav->width; x++)
  {
    int new_element = map_element_RND_to_EM_cave(level->field[x][y]);

    if (level->field[x][y] == EL_AMOEBA_DEAD)
      new_element = map_element_RND_to_EM_cave(EL_AMOEBA_WET);

    cav->cave[x][y] = new_element;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    cav->player_x[i] = -1;
    cav->player_y[i] = -1;
  }

  // initialize player positions and delete players from the playfield
  for (y = 0; y < cav->height; y++) for (x = 0; x < cav->width; x++)
  {
    if (IS_PLAYER_ELEMENT(level->field[x][y]))
    {
      int player_nr = GET_PLAYER_NR(level->field[x][y]);

      cav->player_x[player_nr] = x;
      cav->player_y[player_nr] = y;

      cav->cave[x][y] = map_element_RND_to_EM_cave(EL_EMPTY);
    }
  }
}

static void CopyNativeLevel_EM_to_RND(struct LevelInfo *level)
{
  static int ball_xy[8][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 },
    { 0, 1 },
    { 2, 1 },
    { 0, 2 },
    { 1, 2 },
    { 2, 2 },
  };
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct CAVE *cav = level_em->cav;
  int i, j, x, y;

  level->fieldx = MIN(cav->width,  MAX_LEV_FIELDX);
  level->fieldy = MIN(cav->height, MAX_LEV_FIELDY);

  level->time        = cav->time_seconds;
  level->gems_needed = cav->gems_needed;

  sprintf(level->name, "Level %d", level->file_info.nr);

  level->score[SC_EMERALD]	= cav->emerald_score;
  level->score[SC_DIAMOND]	= cav->diamond_score;
  level->score[SC_ROBOT]	= cav->alien_score;
  level->score[SC_SPACESHIP]	= cav->tank_score;
  level->score[SC_BUG]		= cav->bug_score;
  level->score[SC_YAMYAM]	= cav->eater_score;
  level->score[SC_NUT]		= cav->nut_score;
  level->score[SC_DYNAMITE]	= cav->dynamite_score;
  level->score[SC_KEY]		= cav->key_score;
  level->score[SC_TIME_BONUS]	= cav->exit_score;

  level->num_yamyam_contents	= cav->num_eater_arrays;

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i].e[x][y] =
	  map_element_EM_to_RND_cave(cav->eater_array[i][y * 3 + x]);

  level->amoeba_speed		= cav->amoeba_time;
  level->time_magic_wall	= cav->wonderwall_time;
  level->time_wheel		= cav->wheel_time;

  level->android_move_time	= cav->android_move_time;
  level->android_clone_time	= cav->android_clone_time;
  level->ball_random		= cav->ball_random;
  level->ball_active_initial	= cav->ball_active;
  level->ball_time		= cav->ball_time;
  level->num_ball_contents	= cav->num_ball_arrays;

  level->lenses_score		= cav->lenses_score;
  level->magnify_score		= cav->magnify_score;
  level->slurp_score		= cav->slurp_score;

  level->lenses_time		= cav->lenses_time;
  level->magnify_time		= cav->magnify_time;

  level->wind_direction_initial =
    map_direction_EM_to_RND(cav->wind_direction);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (j = 0; j < 8; j++)
      level->ball_content[i].e[ball_xy[j][0]][ball_xy[j][1]] =
	map_element_EM_to_RND_cave(cav->ball_array[i][j]);

  map_android_clone_elements_EM_to_RND(level);

  // convert the playfield (some elements need special treatment)
  for (y = 0; y < level->fieldy; y++) for (x = 0; x < level->fieldx; x++)
  {
    int new_element = map_element_EM_to_RND_cave(cav->cave[x][y]);

    if (new_element == EL_AMOEBA_WET && level->amoeba_speed == 0)
      new_element = EL_AMOEBA_DEAD;

    level->field[x][y] = new_element;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    // in case of all players set to the same field, use the first player
    int nr = MAX_PLAYERS - i - 1;
    int jx = cav->player_x[nr];
    int jy = cav->player_y[nr];

    if (jx != -1 && jy != -1)
      level->field[jx][jy] = EL_PLAYER_1 + nr;
  }

  // time score is counted for each 10 seconds left in Emerald Mine levels
  level->time_score_base = 10;
}


// ----------------------------------------------------------------------------
// functions for loading SP level
// ----------------------------------------------------------------------------

static void CopyNativeLevel_RND_to_SP(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  LevelInfoType *header = &level_sp->header;
  int i, x, y;

  level_sp->width  = level->fieldx;
  level_sp->height = level->fieldy;

  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      level_sp->playfield[x][y] = map_element_RND_to_SP(level->field[x][y]);

  header->InitialGravity = (level->initial_player_gravity[0] ? 1 : 0);

  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    header->LevelTitle[i] = level->name[i];
  // !!! NO STRING TERMINATION IN SUPAPLEX VB CODE YET -- FIX THIS !!!

  header->InfotronsNeeded = level->gems_needed;

  header->SpecialPortCount = 0;

  for (x = 0; x < level->fieldx; x++) for (y = 0; y < level->fieldy; y++)
  {
    boolean gravity_port_found = FALSE;
    boolean gravity_port_valid = FALSE;
    int gravity_port_flag;
    int gravity_port_base_element;
    int element = level->field[x][y];

    if (element >= EL_SP_GRAVITY_ON_PORT_RIGHT &&
	element <= EL_SP_GRAVITY_ON_PORT_UP)
    {
      gravity_port_found = TRUE;
      gravity_port_valid = TRUE;
      gravity_port_flag = 1;
      gravity_port_base_element = EL_SP_GRAVITY_ON_PORT_RIGHT;
    }
    else if (element >= EL_SP_GRAVITY_OFF_PORT_RIGHT &&
	     element <= EL_SP_GRAVITY_OFF_PORT_UP)
    {
      gravity_port_found = TRUE;
      gravity_port_valid = TRUE;
      gravity_port_flag = 0;
      gravity_port_base_element = EL_SP_GRAVITY_OFF_PORT_RIGHT;
    }
    else if (element >= EL_SP_GRAVITY_PORT_RIGHT &&
	     element <= EL_SP_GRAVITY_PORT_UP)
    {
      // change R'n'D style gravity inverting special port to normal port
      // (there are no gravity inverting ports in native Supaplex engine)

      gravity_port_found = TRUE;
      gravity_port_valid = FALSE;
      gravity_port_base_element = EL_SP_GRAVITY_PORT_RIGHT;
    }

    if (gravity_port_found)
    {
      if (gravity_port_valid &&
	  header->SpecialPortCount < SP_MAX_SPECIAL_PORTS)
      {
	SpecialPortType *port = &header->SpecialPort[header->SpecialPortCount];

	port->PortLocation = (y * level->fieldx + x) * 2;
	port->Gravity = gravity_port_flag;

	element += EL_SP_GRAVITY_PORT_RIGHT - gravity_port_base_element;

	header->SpecialPortCount++;
      }
      else
      {
	// change special gravity port to normal port

	element += EL_SP_PORT_RIGHT - gravity_port_base_element;
      }

      level_sp->playfield[x][y] = element - EL_SP_START;
    }
  }
}

static void CopyNativeLevel_SP_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  LevelInfoType *header = &level_sp->header;
  boolean num_invalid_elements = 0;
  int i, j, x, y;

  level->fieldx = level_sp->width;
  level->fieldy = level_sp->height;

  for (x = 0; x < level->fieldx; x++)
  {
    for (y = 0; y < level->fieldy; y++)
    {
      int element_old = level_sp->playfield[x][y];
      int element_new = getMappedElement(map_element_SP_to_RND(element_old));

      if (element_new == EL_UNKNOWN)
      {
	num_invalid_elements++;

	Debug("level:native:SP", "invalid element %d at position %d, %d",
	      element_old, x, y);
      }

      level->field[x][y] = element_new;
    }
  }

  if (num_invalid_elements > 0)
    Warn("found %d invalid elements%s", num_invalid_elements,
	 (!options.debug ? " (use '--debug' for more details)" : ""));

  for (i = 0; i < MAX_PLAYERS; i++)
    level->initial_player_gravity[i] =
      (header->InitialGravity == 1 ? TRUE : FALSE);

  // skip leading spaces
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    if (header->LevelTitle[i] != ' ')
      break;

  // copy level title
  for (j = 0; i < SP_LEVEL_NAME_LEN; i++, j++)
    level->name[j] = header->LevelTitle[i];
  level->name[j] = '\0';

  // cut trailing spaces
  for (; j > 0; j--)
    if (level->name[j - 1] == ' ' && level->name[j] == '\0')
      level->name[j - 1] = '\0';

  level->gems_needed = header->InfotronsNeeded;

  for (i = 0; i < header->SpecialPortCount; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];
    int port_location = port->PortLocation;
    int gravity = port->Gravity;
    int port_x, port_y, port_element;

    port_x = (port_location / 2) % level->fieldx;
    port_y = (port_location / 2) / level->fieldx;

    if (port_x < 0 || port_x >= level->fieldx ||
	port_y < 0 || port_y >= level->fieldy)
    {
      Warn("special port position (%d, %d) out of bounds", port_x, port_y);

      continue;
    }

    port_element = level->field[port_x][port_y];

    if (port_element < EL_SP_GRAVITY_PORT_RIGHT ||
	port_element > EL_SP_GRAVITY_PORT_UP)
    {
      Warn("no special port at position (%d, %d)", port_x, port_y);

      continue;
    }

    // change previous (wrong) gravity inverting special port to either
    // gravity enabling special port or gravity disabling special port
    level->field[port_x][port_y] +=
      (gravity == 1 ? EL_SP_GRAVITY_ON_PORT_RIGHT :
       EL_SP_GRAVITY_OFF_PORT_RIGHT) - EL_SP_GRAVITY_PORT_RIGHT;
  }

  // change special gravity ports without database entries to normal ports
  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      if (level->field[x][y] >= EL_SP_GRAVITY_PORT_RIGHT &&
	  level->field[x][y] <= EL_SP_GRAVITY_PORT_UP)
	level->field[x][y] += EL_SP_PORT_RIGHT - EL_SP_GRAVITY_PORT_RIGHT;

  level->time = 0;			// no time limit
  level->amoeba_speed = 0;
  level->time_magic_wall = 0;
  level->time_wheel = 0;
  level->amoeba_content = EL_EMPTY;

  // original Supaplex does not use score values -- rate by playing time
  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = 0;

  level->rate_time_over_score = TRUE;

  // there are no yamyams in supaplex levels
  for (i = 0; i < level->num_yamyam_contents; i++)
    for (x = 0; x < 3; x++)
      for (y = 0; y < 3; y++)
	level->yamyam_content[i].e[x][y] = EL_EMPTY;
}

static void CopyNativeTape_RND_to_SP(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  struct DemoInfo_SP *demo = &level_sp->demo;
  int i, j;

  // always start with reliable default values
  demo->is_available = FALSE;
  demo->length = 0;

  if (TAPE_IS_EMPTY(tape))
    return;

  demo->level_nr = tape.level_nr;	// (currently not used)

  level_sp->header.DemoRandomSeed = tape.random_seed;

  demo->length = 0;

  for (i = 0; i < tape.length; i++)
  {
    int demo_action = map_key_RND_to_SP(tape.pos[i].action[0]);
    int demo_repeat = tape.pos[i].delay;
    int demo_entries = (demo_repeat + 15) / 16;

    if (demo->length + demo_entries >= SP_MAX_TAPE_LEN)
    {
      Warn("tape truncated: size exceeds maximum SP demo size %d",
	   SP_MAX_TAPE_LEN);

      break;
    }

    for (j = 0; j < demo_repeat / 16; j++)
      demo->data[demo->length++] = 0xf0 | demo_action;

    if (demo_repeat % 16)
      demo->data[demo->length++] = ((demo_repeat % 16 - 1) << 4) | demo_action;
  }

  demo->is_available = TRUE;
}

static void CopyNativeTape_SP_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  struct DemoInfo_SP *demo = &level_sp->demo;
  char *filename = level->file_info.filename;
  int i;

  // always start with reliable default values
  setTapeInfoToDefaults();

  if (!demo->is_available)
    return;

  tape.level_nr = demo->level_nr;	// (currently not used)
  tape.random_seed = level_sp->header.DemoRandomSeed;

  TapeSetDateFromEpochSeconds(getFileTimestampEpochSeconds(filename));

  tape.counter = 0;
  tape.pos[tape.counter].delay = 0;

  for (i = 0; i < demo->length; i++)
  {
    int demo_action = demo->data[i] & 0x0f;
    int demo_repeat = (demo->data[i] & 0xf0) >> 4;
    int tape_action = map_key_SP_to_RND(demo_action);
    int tape_repeat = demo_repeat + 1;
    byte action[MAX_TAPE_ACTIONS] = { tape_action };
    boolean success = 0;
    int j;

    for (j = 0; j < tape_repeat; j++)
      success = TapeAddAction(action);

    if (!success)
    {
      Warn("SP demo truncated: size exceeds maximum tape size %d",
	   MAX_TAPE_LEN);

      break;
    }
  }

  TapeHaltRecording();
}


// ----------------------------------------------------------------------------
// functions for loading MM level
// ----------------------------------------------------------------------------

static void CopyNativeLevel_RND_to_MM(struct LevelInfo *level)
{
  struct LevelInfo_MM *level_mm = level->native_mm_level;
  int i, x, y;

  level_mm->fieldx = MIN(level->fieldx, MM_MAX_PLAYFIELD_WIDTH);
  level_mm->fieldy = MIN(level->fieldy, MM_MAX_PLAYFIELD_HEIGHT);

  level_mm->time = level->time;
  level_mm->kettles_needed = level->gems_needed;
  level_mm->auto_count_kettles = level->auto_count_gems;

  level_mm->mm_laser_red   = level->mm_laser_red;
  level_mm->mm_laser_green = level->mm_laser_green;
  level_mm->mm_laser_blue  = level->mm_laser_blue;

  level_mm->df_laser_red   = level->df_laser_red;
  level_mm->df_laser_green = level->df_laser_green;
  level_mm->df_laser_blue  = level->df_laser_blue;

  strcpy(level_mm->name, level->name);
  strcpy(level_mm->author, level->author);

  level_mm->score[SC_EMERALD]    = level->score[SC_EMERALD];
  level_mm->score[SC_PACMAN]     = level->score[SC_PACMAN];
  level_mm->score[SC_KEY]        = level->score[SC_KEY];
  level_mm->score[SC_TIME_BONUS] = level->score[SC_TIME_BONUS];
  level_mm->score[SC_ELEM_BONUS] = level->score[SC_ELEM_BONUS];

  level_mm->amoeba_speed = level->amoeba_speed;
  level_mm->time_fuse    = level->mm_time_fuse;
  level_mm->time_bomb    = level->mm_time_bomb;
  level_mm->time_ball    = level->mm_time_ball;
  level_mm->time_block   = level->mm_time_block;

  level_mm->num_ball_contents = level->num_mm_ball_contents;
  level_mm->ball_choice_mode = level->mm_ball_choice_mode;
  level_mm->rotate_ball_content = level->rotate_mm_ball_content;
  level_mm->explode_ball = level->explode_mm_ball;

  for (i = 0; i < level->num_mm_ball_contents; i++)
    level_mm->ball_content[i] =
      map_element_RND_to_MM(level->mm_ball_content[i]);

  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      Ur[x][y] =
	level_mm->field[x][y] = map_element_RND_to_MM(level->field[x][y]);
}

static void CopyNativeLevel_MM_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_MM *level_mm = level->native_mm_level;
  int i, x, y;

  level->fieldx = MIN(level_mm->fieldx, MAX_LEV_FIELDX);
  level->fieldy = MIN(level_mm->fieldy, MAX_LEV_FIELDY);

  level->time = level_mm->time;
  level->gems_needed = level_mm->kettles_needed;
  level->auto_count_gems = level_mm->auto_count_kettles;

  level->mm_laser_red   = level_mm->mm_laser_red;
  level->mm_laser_green = level_mm->mm_laser_green;
  level->mm_laser_blue  = level_mm->mm_laser_blue;

  level->df_laser_red   = level_mm->df_laser_red;
  level->df_laser_green = level_mm->df_laser_green;
  level->df_laser_blue  = level_mm->df_laser_blue;

  strcpy(level->name_native, level_mm->name);
  strcpy(level->name, level_mm->name);

  // only overwrite author from 'levelinfo.conf' if author defined in level
  if (!strEqual(level_mm->author, ANONYMOUS_NAME))
    strcpy(level->author, level_mm->author);

  level->score[SC_EMERALD]    = level_mm->score[SC_EMERALD];
  level->score[SC_PACMAN]     = level_mm->score[SC_PACMAN];
  level->score[SC_KEY]        = level_mm->score[SC_KEY];
  level->score[SC_TIME_BONUS] = level_mm->score[SC_TIME_BONUS];
  level->score[SC_ELEM_BONUS] = level_mm->score[SC_ELEM_BONUS];

  level->amoeba_speed  = level_mm->amoeba_speed;
  level->mm_time_fuse  = level_mm->time_fuse;
  level->mm_time_bomb  = level_mm->time_bomb;
  level->mm_time_ball  = level_mm->time_ball;
  level->mm_time_block = level_mm->time_block;

  level->num_mm_ball_contents = level_mm->num_ball_contents;
  level->mm_ball_choice_mode = level_mm->ball_choice_mode;
  level->rotate_mm_ball_content = level_mm->rotate_ball_content;
  level->explode_mm_ball = level_mm->explode_ball;

  for (i = 0; i < level->num_mm_ball_contents; i++)
    level->mm_ball_content[i] =
      map_element_MM_to_RND(level_mm->ball_content[i]);

  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      level->field[x][y] = map_element_MM_to_RND(level_mm->field[x][y]);
}


// ----------------------------------------------------------------------------
// functions for loading DC level
// ----------------------------------------------------------------------------

#define DC_LEVEL_HEADER_SIZE		344

static unsigned short getDecodedWord_DC(unsigned short data_encoded,
					boolean init)
{
  static int last_data_encoded;
  static int offset1;
  static int offset2;
  int diff;
  int diff_hi, diff_lo;
  int data_hi, data_lo;
  unsigned short data_decoded;

  if (init)
  {
    last_data_encoded = 0;
    offset1 = -1;
    offset2 = 0;

    return 0;
  }

  diff = data_encoded - last_data_encoded;
  diff_hi = diff & ~0xff;
  diff_lo = diff &  0xff;

  offset2 += diff_lo;

  data_hi = diff_hi - (offset1 << 8) + (offset2 & 0xff00);
  data_lo = (diff_lo + (data_hi >> 16)) & 0x00ff;
  data_hi = data_hi & 0xff00;

  data_decoded = data_hi | data_lo;

  last_data_encoded = data_encoded;

  offset1 = (offset1 + 1) % 31;
  offset2 = offset2 & 0xff;

  return data_decoded;
}

static int getMappedElement_DC(int element)
{
  switch (element)
  {
    case 0x0000:
      element = EL_ROCK;
      break;

      // 0x0117 - 0x036e: (?)
      // EL_DIAMOND

      // 0x042d - 0x0684: (?)
      // EL_EMERALD

    case 0x06f1:
      element = EL_NUT;
      break;

    case 0x074c:
      element = EL_BOMB;
      break;

    case 0x07a4:
      element = EL_PEARL;
      break;

    case 0x0823:
      element = EL_CRYSTAL;
      break;

    case 0x0e77:	// quicksand (boulder)
      element = EL_QUICKSAND_FAST_FULL;
      break;

    case 0x0e99:	// slow quicksand (boulder)
      element = EL_QUICKSAND_FULL;
      break;

    case 0x0ed2:
      element = EL_EM_EXIT_OPEN;
      break;

    case 0x0ee3:
      element = EL_EM_EXIT_CLOSED;
      break;

    case 0x0eeb:
      element = EL_EM_STEEL_EXIT_OPEN;
      break;

    case 0x0efc:
      element = EL_EM_STEEL_EXIT_CLOSED;
      break;

    case 0x0f4f:	// dynamite (lit 1)
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f57:	// dynamite (lit 2)
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f5f:	// dynamite (lit 3)
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f67:	// dynamite (lit 4)
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f81:
    case 0x0f82:
    case 0x0f83:
    case 0x0f84:
      element = EL_AMOEBA_WET;
      break;

    case 0x0f85:
      element = EL_AMOEBA_DROP;
      break;

    case 0x0fb9:
      element = EL_DC_MAGIC_WALL;
      break;

    case 0x0fd0:
      element = EL_SPACESHIP_UP;
      break;

    case 0x0fd9:
      element = EL_SPACESHIP_DOWN;
      break;

    case 0x0ff1:
      element = EL_SPACESHIP_LEFT;
      break;

    case 0x0ff9:
      element = EL_SPACESHIP_RIGHT;
      break;

    case 0x1057:
      element = EL_BUG_UP;
      break;

    case 0x1060:
      element = EL_BUG_DOWN;
      break;

    case 0x1078:
      element = EL_BUG_LEFT;
      break;

    case 0x1080:
      element = EL_BUG_RIGHT;
      break;

    case 0x10de:
      element = EL_MOLE_UP;
      break;

    case 0x10e7:
      element = EL_MOLE_DOWN;
      break;

    case 0x10ff:
      element = EL_MOLE_LEFT;
      break;

    case 0x1107:
      element = EL_MOLE_RIGHT;
      break;

    case 0x11c0:
      element = EL_ROBOT;
      break;

    case 0x13f5:
      element = EL_YAMYAM_UP;
      break;

    case 0x1425:
      element = EL_SWITCHGATE_OPEN;
      break;

    case 0x1426:
      element = EL_SWITCHGATE_CLOSED;
      break;

    case 0x1437:
      element = EL_DC_SWITCHGATE_SWITCH_UP;
      break;

    case 0x143a:
      element = EL_TIMEGATE_CLOSED;
      break;

    case 0x144c:	// conveyor belt switch (green)
      element = EL_CONVEYOR_BELT_3_SWITCH_MIDDLE;
      break;

    case 0x144f:	// conveyor belt switch (red)
      element = EL_CONVEYOR_BELT_1_SWITCH_MIDDLE;
      break;

    case 0x1452:	// conveyor belt switch (blue)
      element = EL_CONVEYOR_BELT_4_SWITCH_MIDDLE;
      break;

    case 0x145b:
      element = EL_CONVEYOR_BELT_3_MIDDLE;
      break;

    case 0x1463:
      element = EL_CONVEYOR_BELT_3_LEFT;
      break;

    case 0x146b:
      element = EL_CONVEYOR_BELT_3_RIGHT;
      break;

    case 0x1473:
      element = EL_CONVEYOR_BELT_1_MIDDLE;
      break;

    case 0x147b:
      element = EL_CONVEYOR_BELT_1_LEFT;
      break;

    case 0x1483:
      element = EL_CONVEYOR_BELT_1_RIGHT;
      break;

    case 0x148b:
      element = EL_CONVEYOR_BELT_4_MIDDLE;
      break;

    case 0x1493:
      element = EL_CONVEYOR_BELT_4_LEFT;
      break;

    case 0x149b:
      element = EL_CONVEYOR_BELT_4_RIGHT;
      break;

    case 0x14ac:
      element = EL_EXPANDABLE_WALL_HORIZONTAL;
      break;

    case 0x14bd:
      element = EL_EXPANDABLE_WALL_VERTICAL;
      break;

    case 0x14c6:
      element = EL_EXPANDABLE_WALL_ANY;
      break;

    case 0x14ce:	// growing steel wall (left/right)
      element = EL_EXPANDABLE_STEELWALL_HORIZONTAL;
      break;

    case 0x14df:	// growing steel wall (up/down)
      element = EL_EXPANDABLE_STEELWALL_VERTICAL;
      break;

    case 0x14e8:	// growing steel wall (up/down/left/right)
      element = EL_EXPANDABLE_STEELWALL_ANY;
      break;

    case 0x14e9:
      element = EL_SHIELD_DEADLY;
      break;

    case 0x1501:
      element = EL_EXTRA_TIME;
      break;

    case 0x154f:
      element = EL_ACID;
      break;

    case 0x1577:
      element = EL_EMPTY_SPACE;
      break;

    case 0x1578:	// quicksand (empty)
      element = EL_QUICKSAND_FAST_EMPTY;
      break;

    case 0x1579:	// slow quicksand (empty)
      element = EL_QUICKSAND_EMPTY;
      break;

      // 0x157c - 0x158b:
      // EL_SAND

      // 0x1590 - 0x159f:
      // EL_DC_LANDMINE

    case 0x15a0:
      element = EL_EM_DYNAMITE;
      break;

    case 0x15a1:	// key (red)
      element = EL_EM_KEY_1;
      break;

    case 0x15a2:	// key (yellow)
      element = EL_EM_KEY_2;
      break;

    case 0x15a3:	// key (blue)
      element = EL_EM_KEY_4;
      break;

    case 0x15a4:	// key (green)
      element = EL_EM_KEY_3;
      break;

    case 0x15a5:	// key (white)
      element = EL_DC_KEY_WHITE;
      break;

    case 0x15a6:
      element = EL_WALL_SLIPPERY;
      break;

    case 0x15a7:
      element = EL_WALL;
      break;

    case 0x15a8:	// wall (not round)
      element = EL_WALL;
      break;

    case 0x15a9:	// (blue)
      element = EL_CHAR_A;
      break;

    case 0x15aa:	// (blue)
      element = EL_CHAR_B;
      break;

    case 0x15ab:	// (blue)
      element = EL_CHAR_C;
      break;

    case 0x15ac:	// (blue)
      element = EL_CHAR_D;
      break;

    case 0x15ad:	// (blue)
      element = EL_CHAR_E;
      break;

    case 0x15ae:	// (blue)
      element = EL_CHAR_F;
      break;

    case 0x15af:	// (blue)
      element = EL_CHAR_G;
      break;

    case 0x15b0:	// (blue)
      element = EL_CHAR_H;
      break;

    case 0x15b1:	// (blue)
      element = EL_CHAR_I;
      break;

    case 0x15b2:	// (blue)
      element = EL_CHAR_J;
      break;

    case 0x15b3:	// (blue)
      element = EL_CHAR_K;
      break;

    case 0x15b4:	// (blue)
      element = EL_CHAR_L;
      break;

    case 0x15b5:	// (blue)
      element = EL_CHAR_M;
      break;

    case 0x15b6:	// (blue)
      element = EL_CHAR_N;
      break;

    case 0x15b7:	// (blue)
      element = EL_CHAR_O;
      break;

    case 0x15b8:	// (blue)
      element = EL_CHAR_P;
      break;

    case 0x15b9:	// (blue)
      element = EL_CHAR_Q;
      break;

    case 0x15ba:	// (blue)
      element = EL_CHAR_R;
      break;

    case 0x15bb:	// (blue)
      element = EL_CHAR_S;
      break;

    case 0x15bc:	// (blue)
      element = EL_CHAR_T;
      break;

    case 0x15bd:	// (blue)
      element = EL_CHAR_U;
      break;

    case 0x15be:	// (blue)
      element = EL_CHAR_V;
      break;

    case 0x15bf:	// (blue)
      element = EL_CHAR_W;
      break;

    case 0x15c0:	// (blue)
      element = EL_CHAR_X;
      break;

    case 0x15c1:	// (blue)
      element = EL_CHAR_Y;
      break;

    case 0x15c2:	// (blue)
      element = EL_CHAR_Z;
      break;

    case 0x15c3:	// (blue)
      element = EL_CHAR_AUMLAUT;
      break;

    case 0x15c4:	// (blue)
      element = EL_CHAR_OUMLAUT;
      break;

    case 0x15c5:	// (blue)
      element = EL_CHAR_UUMLAUT;
      break;

    case 0x15c6:	// (blue)
      element = EL_CHAR_0;
      break;

    case 0x15c7:	// (blue)
      element = EL_CHAR_1;
      break;

    case 0x15c8:	// (blue)
      element = EL_CHAR_2;
      break;

    case 0x15c9:	// (blue)
      element = EL_CHAR_3;
      break;

    case 0x15ca:	// (blue)
      element = EL_CHAR_4;
      break;

    case 0x15cb:	// (blue)
      element = EL_CHAR_5;
      break;

    case 0x15cc:	// (blue)
      element = EL_CHAR_6;
      break;

    case 0x15cd:	// (blue)
      element = EL_CHAR_7;
      break;

    case 0x15ce:	// (blue)
      element = EL_CHAR_8;
      break;

    case 0x15cf:	// (blue)
      element = EL_CHAR_9;
      break;

    case 0x15d0:	// (blue)
      element = EL_CHAR_PERIOD;
      break;

    case 0x15d1:	// (blue)
      element = EL_CHAR_EXCLAM;
      break;

    case 0x15d2:	// (blue)
      element = EL_CHAR_COLON;
      break;

    case 0x15d3:	// (blue)
      element = EL_CHAR_LESS;
      break;

    case 0x15d4:	// (blue)
      element = EL_CHAR_GREATER;
      break;

    case 0x15d5:	// (blue)
      element = EL_CHAR_QUESTION;
      break;

    case 0x15d6:	// (blue)
      element = EL_CHAR_COPYRIGHT;
      break;

    case 0x15d7:	// (blue)
      element = EL_CHAR_UP;
      break;

    case 0x15d8:	// (blue)
      element = EL_CHAR_DOWN;
      break;

    case 0x15d9:	// (blue)
      element = EL_CHAR_BUTTON;
      break;

    case 0x15da:	// (blue)
      element = EL_CHAR_PLUS;
      break;

    case 0x15db:	// (blue)
      element = EL_CHAR_MINUS;
      break;

    case 0x15dc:	// (blue)
      element = EL_CHAR_APOSTROPHE;
      break;

    case 0x15dd:	// (blue)
      element = EL_CHAR_PARENLEFT;
      break;

    case 0x15de:	// (blue)
      element = EL_CHAR_PARENRIGHT;
      break;

    case 0x15df:	// (green)
      element = EL_CHAR_A;
      break;

    case 0x15e0:	// (green)
      element = EL_CHAR_B;
      break;

    case 0x15e1:	// (green)
      element = EL_CHAR_C;
      break;

    case 0x15e2:	// (green)
      element = EL_CHAR_D;
      break;

    case 0x15e3:	// (green)
      element = EL_CHAR_E;
      break;

    case 0x15e4:	// (green)
      element = EL_CHAR_F;
      break;

    case 0x15e5:	// (green)
      element = EL_CHAR_G;
      break;

    case 0x15e6:	// (green)
      element = EL_CHAR_H;
      break;

    case 0x15e7:	// (green)
      element = EL_CHAR_I;
      break;

    case 0x15e8:	// (green)
      element = EL_CHAR_J;
      break;

    case 0x15e9:	// (green)
      element = EL_CHAR_K;
      break;

    case 0x15ea:	// (green)
      element = EL_CHAR_L;
      break;

    case 0x15eb:	// (green)
      element = EL_CHAR_M;
      break;

    case 0x15ec:	// (green)
      element = EL_CHAR_N;
      break;

    case 0x15ed:	// (green)
      element = EL_CHAR_O;
      break;

    case 0x15ee:	// (green)
      element = EL_CHAR_P;
      break;

    case 0x15ef:	// (green)
      element = EL_CHAR_Q;
      break;

    case 0x15f0:	// (green)
      element = EL_CHAR_R;
      break;

    case 0x15f1:	// (green)
      element = EL_CHAR_S;
      break;

    case 0x15f2:	// (green)
      element = EL_CHAR_T;
      break;

    case 0x15f3:	// (green)
      element = EL_CHAR_U;
      break;

    case 0x15f4:	// (green)
      element = EL_CHAR_V;
      break;

    case 0x15f5:	// (green)
      element = EL_CHAR_W;
      break;

    case 0x15f6:	// (green)
      element = EL_CHAR_X;
      break;

    case 0x15f7:	// (green)
      element = EL_CHAR_Y;
      break;

    case 0x15f8:	// (green)
      element = EL_CHAR_Z;
      break;

    case 0x15f9:	// (green)
      element = EL_CHAR_AUMLAUT;
      break;

    case 0x15fa:	// (green)
      element = EL_CHAR_OUMLAUT;
      break;

    case 0x15fb:	// (green)
      element = EL_CHAR_UUMLAUT;
      break;

    case 0x15fc:	// (green)
      element = EL_CHAR_0;
      break;

    case 0x15fd:	// (green)
      element = EL_CHAR_1;
      break;

    case 0x15fe:	// (green)
      element = EL_CHAR_2;
      break;

    case 0x15ff:	// (green)
      element = EL_CHAR_3;
      break;

    case 0x1600:	// (green)
      element = EL_CHAR_4;
      break;

    case 0x1601:	// (green)
      element = EL_CHAR_5;
      break;

    case 0x1602:	// (green)
      element = EL_CHAR_6;
      break;

    case 0x1603:	// (green)
      element = EL_CHAR_7;
      break;

    case 0x1604:	// (green)
      element = EL_CHAR_8;
      break;

    case 0x1605:	// (green)
      element = EL_CHAR_9;
      break;

    case 0x1606:	// (green)
      element = EL_CHAR_PERIOD;
      break;

    case 0x1607:	// (green)
      element = EL_CHAR_EXCLAM;
      break;

    case 0x1608:	// (green)
      element = EL_CHAR_COLON;
      break;

    case 0x1609:	// (green)
      element = EL_CHAR_LESS;
      break;

    case 0x160a:	// (green)
      element = EL_CHAR_GREATER;
      break;

    case 0x160b:	// (green)
      element = EL_CHAR_QUESTION;
      break;

    case 0x160c:	// (green)
      element = EL_CHAR_COPYRIGHT;
      break;

    case 0x160d:	// (green)
      element = EL_CHAR_UP;
      break;

    case 0x160e:	// (green)
      element = EL_CHAR_DOWN;
      break;

    case 0x160f:	// (green)
      element = EL_CHAR_BUTTON;
      break;

    case 0x1610:	// (green)
      element = EL_CHAR_PLUS;
      break;

    case 0x1611:	// (green)
      element = EL_CHAR_MINUS;
      break;

    case 0x1612:	// (green)
      element = EL_CHAR_APOSTROPHE;
      break;

    case 0x1613:	// (green)
      element = EL_CHAR_PARENLEFT;
      break;

    case 0x1614:	// (green)
      element = EL_CHAR_PARENRIGHT;
      break;

    case 0x1615:	// (blue steel)
      element = EL_STEEL_CHAR_A;
      break;

    case 0x1616:	// (blue steel)
      element = EL_STEEL_CHAR_B;
      break;

    case 0x1617:	// (blue steel)
      element = EL_STEEL_CHAR_C;
      break;

    case 0x1618:	// (blue steel)
      element = EL_STEEL_CHAR_D;
      break;

    case 0x1619:	// (blue steel)
      element = EL_STEEL_CHAR_E;
      break;

    case 0x161a:	// (blue steel)
      element = EL_STEEL_CHAR_F;
      break;

    case 0x161b:	// (blue steel)
      element = EL_STEEL_CHAR_G;
      break;

    case 0x161c:	// (blue steel)
      element = EL_STEEL_CHAR_H;
      break;

    case 0x161d:	// (blue steel)
      element = EL_STEEL_CHAR_I;
      break;

    case 0x161e:	// (blue steel)
      element = EL_STEEL_CHAR_J;
      break;

    case 0x161f:	// (blue steel)
      element = EL_STEEL_CHAR_K;
      break;

    case 0x1620:	// (blue steel)
      element = EL_STEEL_CHAR_L;
      break;

    case 0x1621:	// (blue steel)
      element = EL_STEEL_CHAR_M;
      break;

    case 0x1622:	// (blue steel)
      element = EL_STEEL_CHAR_N;
      break;

    case 0x1623:	// (blue steel)
      element = EL_STEEL_CHAR_O;
      break;

    case 0x1624:	// (blue steel)
      element = EL_STEEL_CHAR_P;
      break;

    case 0x1625:	// (blue steel)
      element = EL_STEEL_CHAR_Q;
      break;

    case 0x1626:	// (blue steel)
      element = EL_STEEL_CHAR_R;
      break;

    case 0x1627:	// (blue steel)
      element = EL_STEEL_CHAR_S;
      break;

    case 0x1628:	// (blue steel)
      element = EL_STEEL_CHAR_T;
      break;

    case 0x1629:	// (blue steel)
      element = EL_STEEL_CHAR_U;
      break;

    case 0x162a:	// (blue steel)
      element = EL_STEEL_CHAR_V;
      break;

    case 0x162b:	// (blue steel)
      element = EL_STEEL_CHAR_W;
      break;

    case 0x162c:	// (blue steel)
      element = EL_STEEL_CHAR_X;
      break;

    case 0x162d:	// (blue steel)
      element = EL_STEEL_CHAR_Y;
      break;

    case 0x162e:	// (blue steel)
      element = EL_STEEL_CHAR_Z;
      break;

    case 0x162f:	// (blue steel)
      element = EL_STEEL_CHAR_AUMLAUT;
      break;

    case 0x1630:	// (blue steel)
      element = EL_STEEL_CHAR_OUMLAUT;
      break;

    case 0x1631:	// (blue steel)
      element = EL_STEEL_CHAR_UUMLAUT;
      break;

    case 0x1632:	// (blue steel)
      element = EL_STEEL_CHAR_0;
      break;

    case 0x1633:	// (blue steel)
      element = EL_STEEL_CHAR_1;
      break;

    case 0x1634:	// (blue steel)
      element = EL_STEEL_CHAR_2;
      break;

    case 0x1635:	// (blue steel)
      element = EL_STEEL_CHAR_3;
      break;

    case 0x1636:	// (blue steel)
      element = EL_STEEL_CHAR_4;
      break;

    case 0x1637:	// (blue steel)
      element = EL_STEEL_CHAR_5;
      break;

    case 0x1638:	// (blue steel)
      element = EL_STEEL_CHAR_6;
      break;

    case 0x1639:	// (blue steel)
      element = EL_STEEL_CHAR_7;
      break;

    case 0x163a:	// (blue steel)
      element = EL_STEEL_CHAR_8;
      break;

    case 0x163b:	// (blue steel)
      element = EL_STEEL_CHAR_9;
      break;

    case 0x163c:	// (blue steel)
      element = EL_STEEL_CHAR_PERIOD;
      break;

    case 0x163d:	// (blue steel)
      element = EL_STEEL_CHAR_EXCLAM;
      break;

    case 0x163e:	// (blue steel)
      element = EL_STEEL_CHAR_COLON;
      break;

    case 0x163f:	// (blue steel)
      element = EL_STEEL_CHAR_LESS;
      break;

    case 0x1640:	// (blue steel)
      element = EL_STEEL_CHAR_GREATER;
      break;

    case 0x1641:	// (blue steel)
      element = EL_STEEL_CHAR_QUESTION;
      break;

    case 0x1642:	// (blue steel)
      element = EL_STEEL_CHAR_COPYRIGHT;
      break;

    case 0x1643:	// (blue steel)
      element = EL_STEEL_CHAR_UP;
      break;

    case 0x1644:	// (blue steel)
      element = EL_STEEL_CHAR_DOWN;
      break;

    case 0x1645:	// (blue steel)
      element = EL_STEEL_CHAR_BUTTON;
      break;

    case 0x1646:	// (blue steel)
      element = EL_STEEL_CHAR_PLUS;
      break;

    case 0x1647:	// (blue steel)
      element = EL_STEEL_CHAR_MINUS;
      break;

    case 0x1648:	// (blue steel)
      element = EL_STEEL_CHAR_APOSTROPHE;
      break;

    case 0x1649:	// (blue steel)
      element = EL_STEEL_CHAR_PARENLEFT;
      break;

    case 0x164a:	// (blue steel)
      element = EL_STEEL_CHAR_PARENRIGHT;
      break;

    case 0x164b:	// (green steel)
      element = EL_STEEL_CHAR_A;
      break;

    case 0x164c:	// (green steel)
      element = EL_STEEL_CHAR_B;
      break;

    case 0x164d:	// (green steel)
      element = EL_STEEL_CHAR_C;
      break;

    case 0x164e:	// (green steel)
      element = EL_STEEL_CHAR_D;
      break;

    case 0x164f:	// (green steel)
      element = EL_STEEL_CHAR_E;
      break;

    case 0x1650:	// (green steel)
      element = EL_STEEL_CHAR_F;
      break;

    case 0x1651:	// (green steel)
      element = EL_STEEL_CHAR_G;
      break;

    case 0x1652:	// (green steel)
      element = EL_STEEL_CHAR_H;
      break;

    case 0x1653:	// (green steel)
      element = EL_STEEL_CHAR_I;
      break;

    case 0x1654:	// (green steel)
      element = EL_STEEL_CHAR_J;
      break;

    case 0x1655:	// (green steel)
      element = EL_STEEL_CHAR_K;
      break;

    case 0x1656:	// (green steel)
      element = EL_STEEL_CHAR_L;
      break;

    case 0x1657:	// (green steel)
      element = EL_STEEL_CHAR_M;
      break;

    case 0x1658:	// (green steel)
      element = EL_STEEL_CHAR_N;
      break;

    case 0x1659:	// (green steel)
      element = EL_STEEL_CHAR_O;
      break;

    case 0x165a:	// (green steel)
      element = EL_STEEL_CHAR_P;
      break;

    case 0x165b:	// (green steel)
      element = EL_STEEL_CHAR_Q;
      break;

    case 0x165c:	// (green steel)
      element = EL_STEEL_CHAR_R;
      break;

    case 0x165d:	// (green steel)
      element = EL_STEEL_CHAR_S;
      break;

    case 0x165e:	// (green steel)
      element = EL_STEEL_CHAR_T;
      break;

    case 0x165f:	// (green steel)
      element = EL_STEEL_CHAR_U;
      break;

    case 0x1660:	// (green steel)
      element = EL_STEEL_CHAR_V;
      break;

    case 0x1661:	// (green steel)
      element = EL_STEEL_CHAR_W;
      break;

    case 0x1662:	// (green steel)
      element = EL_STEEL_CHAR_X;
      break;

    case 0x1663:	// (green steel)
      element = EL_STEEL_CHAR_Y;
      break;

    case 0x1664:	// (green steel)
      element = EL_STEEL_CHAR_Z;
      break;

    case 0x1665:	// (green steel)
      element = EL_STEEL_CHAR_AUMLAUT;
      break;

    case 0x1666:	// (green steel)
      element = EL_STEEL_CHAR_OUMLAUT;
      break;

    case 0x1667:	// (green steel)
      element = EL_STEEL_CHAR_UUMLAUT;
      break;

    case 0x1668:	// (green steel)
      element = EL_STEEL_CHAR_0;
      break;

    case 0x1669:	// (green steel)
      element = EL_STEEL_CHAR_1;
      break;

    case 0x166a:	// (green steel)
      element = EL_STEEL_CHAR_2;
      break;

    case 0x166b:	// (green steel)
      element = EL_STEEL_CHAR_3;
      break;

    case 0x166c:	// (green steel)
      element = EL_STEEL_CHAR_4;
      break;

    case 0x166d:	// (green steel)
      element = EL_STEEL_CHAR_5;
      break;

    case 0x166e:	// (green steel)
      element = EL_STEEL_CHAR_6;
      break;

    case 0x166f:	// (green steel)
      element = EL_STEEL_CHAR_7;
      break;

    case 0x1670:	// (green steel)
      element = EL_STEEL_CHAR_8;
      break;

    case 0x1671:	// (green steel)
      element = EL_STEEL_CHAR_9;
      break;

    case 0x1672:	// (green steel)
      element = EL_STEEL_CHAR_PERIOD;
      break;

    case 0x1673:	// (green steel)
      element = EL_STEEL_CHAR_EXCLAM;
      break;

    case 0x1674:	// (green steel)
      element = EL_STEEL_CHAR_COLON;
      break;

    case 0x1675:	// (green steel)
      element = EL_STEEL_CHAR_LESS;
      break;

    case 0x1676:	// (green steel)
      element = EL_STEEL_CHAR_GREATER;
      break;

    case 0x1677:	// (green steel)
      element = EL_STEEL_CHAR_QUESTION;
      break;

    case 0x1678:	// (green steel)
      element = EL_STEEL_CHAR_COPYRIGHT;
      break;

    case 0x1679:	// (green steel)
      element = EL_STEEL_CHAR_UP;
      break;

    case 0x167a:	// (green steel)
      element = EL_STEEL_CHAR_DOWN;
      break;

    case 0x167b:	// (green steel)
      element = EL_STEEL_CHAR_BUTTON;
      break;

    case 0x167c:	// (green steel)
      element = EL_STEEL_CHAR_PLUS;
      break;

    case 0x167d:	// (green steel)
      element = EL_STEEL_CHAR_MINUS;
      break;

    case 0x167e:	// (green steel)
      element = EL_STEEL_CHAR_APOSTROPHE;
      break;

    case 0x167f:	// (green steel)
      element = EL_STEEL_CHAR_PARENLEFT;
      break;

    case 0x1680:	// (green steel)
      element = EL_STEEL_CHAR_PARENRIGHT;
      break;

    case 0x1681:	// gate (red)
      element = EL_EM_GATE_1;
      break;

    case 0x1682:	// secret gate (red)
      element = EL_EM_GATE_1_GRAY;
      break;

    case 0x1683:	// gate (yellow)
      element = EL_EM_GATE_2;
      break;

    case 0x1684:	// secret gate (yellow)
      element = EL_EM_GATE_2_GRAY;
      break;

    case 0x1685:	// gate (blue)
      element = EL_EM_GATE_4;
      break;

    case 0x1686:	// secret gate (blue)
      element = EL_EM_GATE_4_GRAY;
      break;

    case 0x1687:	// gate (green)
      element = EL_EM_GATE_3;
      break;

    case 0x1688:	// secret gate (green)
      element = EL_EM_GATE_3_GRAY;
      break;

    case 0x1689:	// gate (white)
      element = EL_DC_GATE_WHITE;
      break;

    case 0x168a:	// secret gate (white)
      element = EL_DC_GATE_WHITE_GRAY;
      break;

    case 0x168b:	// secret gate (no key)
      element = EL_DC_GATE_FAKE_GRAY;
      break;

    case 0x168c:
      element = EL_ROBOT_WHEEL;
      break;

    case 0x168d:
      element = EL_DC_TIMEGATE_SWITCH;
      break;

    case 0x168e:
      element = EL_ACID_POOL_BOTTOM;
      break;

    case 0x168f:
      element = EL_ACID_POOL_TOPLEFT;
      break;

    case 0x1690:
      element = EL_ACID_POOL_TOPRIGHT;
      break;

    case 0x1691:
      element = EL_ACID_POOL_BOTTOMLEFT;
      break;

    case 0x1692:
      element = EL_ACID_POOL_BOTTOMRIGHT;
      break;

    case 0x1693:
      element = EL_STEELWALL;
      break;

    case 0x1694:
      element = EL_STEELWALL_SLIPPERY;
      break;

    case 0x1695:	// steel wall (not round)
      element = EL_STEELWALL;
      break;

    case 0x1696:	// steel wall (left)
      element = EL_DC_STEELWALL_1_LEFT;
      break;

    case 0x1697:	// steel wall (bottom)
      element = EL_DC_STEELWALL_1_BOTTOM;
      break;

    case 0x1698:	// steel wall (right)
      element = EL_DC_STEELWALL_1_RIGHT;
      break;

    case 0x1699:	// steel wall (top)
      element = EL_DC_STEELWALL_1_TOP;
      break;

    case 0x169a:	// steel wall (left/bottom)
      element = EL_DC_STEELWALL_1_BOTTOMLEFT;
      break;

    case 0x169b:	// steel wall (right/bottom)
      element = EL_DC_STEELWALL_1_BOTTOMRIGHT;
      break;

    case 0x169c:	// steel wall (right/top)
      element = EL_DC_STEELWALL_1_TOPRIGHT;
      break;

    case 0x169d:	// steel wall (left/top)
      element = EL_DC_STEELWALL_1_TOPLEFT;
      break;

    case 0x169e:	// steel wall (right/bottom small)
      element = EL_DC_STEELWALL_1_BOTTOMRIGHT_2;
      break;

    case 0x169f:	// steel wall (left/bottom small)
      element = EL_DC_STEELWALL_1_BOTTOMLEFT_2;
      break;

    case 0x16a0:	// steel wall (right/top small)
      element = EL_DC_STEELWALL_1_TOPRIGHT_2;
      break;

    case 0x16a1:	// steel wall (left/top small)
      element = EL_DC_STEELWALL_1_TOPLEFT_2;
      break;

    case 0x16a2:	// steel wall (left/right)
      element = EL_DC_STEELWALL_1_VERTICAL;
      break;

    case 0x16a3:	// steel wall (top/bottom)
      element = EL_DC_STEELWALL_1_HORIZONTAL;
      break;

    case 0x16a4:	// steel wall 2 (left end)
      element = EL_DC_STEELWALL_2_LEFT;
      break;

    case 0x16a5:	// steel wall 2 (right end)
      element = EL_DC_STEELWALL_2_RIGHT;
      break;

    case 0x16a6:	// steel wall 2 (top end)
      element = EL_DC_STEELWALL_2_TOP;
      break;

    case 0x16a7:	// steel wall 2 (bottom end)
      element = EL_DC_STEELWALL_2_BOTTOM;
      break;

    case 0x16a8:	// steel wall 2 (left/right)
      element = EL_DC_STEELWALL_2_HORIZONTAL;
      break;

    case 0x16a9:	// steel wall 2 (up/down)
      element = EL_DC_STEELWALL_2_VERTICAL;
      break;

    case 0x16aa:	// steel wall 2 (mid)
      element = EL_DC_STEELWALL_2_MIDDLE;
      break;

    case 0x16ab:
      element = EL_SIGN_EXCLAMATION;
      break;

    case 0x16ac:
      element = EL_SIGN_RADIOACTIVITY;
      break;

    case 0x16ad:
      element = EL_SIGN_STOP;
      break;

    case 0x16ae:
      element = EL_SIGN_WHEELCHAIR;
      break;

    case 0x16af:
      element = EL_SIGN_PARKING;
      break;

    case 0x16b0:
      element = EL_SIGN_NO_ENTRY;
      break;

    case 0x16b1:
      element = EL_SIGN_HEART;
      break;

    case 0x16b2:
      element = EL_SIGN_GIVE_WAY;
      break;

    case 0x16b3:
      element = EL_SIGN_ENTRY_FORBIDDEN;
      break;

    case 0x16b4:
      element = EL_SIGN_EMERGENCY_EXIT;
      break;

    case 0x16b5:
      element = EL_SIGN_YIN_YANG;
      break;

    case 0x16b6:
      element = EL_WALL_EMERALD;
      break;

    case 0x16b7:
      element = EL_WALL_DIAMOND;
      break;

    case 0x16b8:
      element = EL_WALL_PEARL;
      break;

    case 0x16b9:
      element = EL_WALL_CRYSTAL;
      break;

    case 0x16ba:
      element = EL_INVISIBLE_WALL;
      break;

    case 0x16bb:
      element = EL_INVISIBLE_STEELWALL;
      break;

      // 0x16bc - 0x16cb:
      // EL_INVISIBLE_SAND

    case 0x16cc:
      element = EL_LIGHT_SWITCH;
      break;

    case 0x16cd:
      element = EL_ENVELOPE_1;
      break;

    default:
      if (element >= 0x0117 && element <= 0x036e)	// (?)
	element = EL_DIAMOND;
      else if (element >= 0x042d && element <= 0x0684)	// (?)
	element = EL_EMERALD;
      else if (element >= 0x157c && element <= 0x158b)
	element = EL_SAND;
      else if (element >= 0x1590 && element <= 0x159f)
	element = EL_DC_LANDMINE;
      else if (element >= 0x16bc && element <= 0x16cb)
	element = EL_INVISIBLE_SAND;
      else
      {
	Warn("unknown Diamond Caves element 0x%04x", element);

	element = EL_UNKNOWN;
      }
      break;
  }

  return getMappedElement(element);
}

static void LoadLevelFromFileStream_DC(File *file, struct LevelInfo *level)
{
  byte header[DC_LEVEL_HEADER_SIZE];
  int envelope_size;
  int envelope_header_pos = 62;
  int envelope_content_pos = 94;
  int level_name_pos = 251;
  int level_author_pos = 292;
  int envelope_header_len;
  int envelope_content_len;
  int level_name_len;
  int level_author_len;
  int fieldx, fieldy;
  int num_yamyam_contents;
  int i, x, y;

  getDecodedWord_DC(0, TRUE);		// initialize DC2 decoding engine

  for (i = 0; i < DC_LEVEL_HEADER_SIZE / 2; i++)
  {
    unsigned short header_word = getDecodedWord_DC(getFile16BitBE(file), FALSE);

    header[i * 2 + 0] = header_word >> 8;
    header[i * 2 + 1] = header_word & 0xff;
  }

  // read some values from level header to check level decoding integrity
  fieldx = header[6] | (header[7] << 8);
  fieldy = header[8] | (header[9] << 8);
  num_yamyam_contents = header[60] | (header[61] << 8);

  // do some simple sanity checks to ensure that level was correctly decoded
  if (fieldx < 1 || fieldx > 256 ||
      fieldy < 1 || fieldy > 256 ||
      num_yamyam_contents < 1 || num_yamyam_contents > 8)
  {
    level->no_valid_file = TRUE;

    Warn("cannot decode level from stream -- using empty level");

    return;
  }

  // maximum envelope header size is 31 bytes
  envelope_header_len	= header[envelope_header_pos];
  // maximum envelope content size is 110 (156?) bytes
  envelope_content_len	= header[envelope_content_pos];

  // maximum level title size is 40 bytes
  level_name_len	= MIN(header[level_name_pos],   MAX_LEVEL_NAME_LEN);
  // maximum level author size is 30 (51?) bytes
  level_author_len	= MIN(header[level_author_pos], MAX_LEVEL_AUTHOR_LEN);

  envelope_size = 0;

  for (i = 0; i < envelope_header_len; i++)
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] =
	header[envelope_header_pos + 1 + i];

  if (envelope_header_len > 0 && envelope_content_len > 0)
  {
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] = '\n';
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] = '\n';
  }

  for (i = 0; i < envelope_content_len; i++)
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] =
	header[envelope_content_pos + 1 + i];

  level->envelope[0].text[envelope_size] = '\0';

  level->envelope[0].xsize = MAX_ENVELOPE_XSIZE;
  level->envelope[0].ysize = 10;
  level->envelope[0].autowrap = TRUE;
  level->envelope[0].centered = TRUE;

  for (i = 0; i < level_name_len; i++)
    level->name_native[i] = header[level_name_pos + 1 + i];
  level->name_native[level_name_len] = '\0';

  for (i = 0; i < level_name_len; i++)
    level->name[i] = header[level_name_pos + 1 + i];
  level->name[level_name_len] = '\0';

  for (i = 0; i < level_author_len; i++)
    level->author[i] = header[level_author_pos + 1 + i];
  level->author[level_author_len] = '\0';

  num_yamyam_contents = header[60] | (header[61] << 8);
  level->num_yamyam_contents =
    MIN(MAX(MIN_ELEMENT_CONTENTS, num_yamyam_contents), MAX_ELEMENT_CONTENTS);

  for (i = 0; i < num_yamyam_contents; i++)
  {
    for (y = 0; y < 3; y++) for (x = 0; x < 3; x++)
    {
      unsigned short word = getDecodedWord_DC(getFile16BitBE(file), FALSE);
      int element_dc = ((word & 0xff) << 8) | ((word >> 8) & 0xff);

      if (i < MAX_ELEMENT_CONTENTS)
	level->yamyam_content[i].e[x][y] = getMappedElement_DC(element_dc);
    }
  }

  fieldx = header[6] | (header[7] << 8);
  fieldy = header[8] | (header[9] << 8);
  level->fieldx = MIN(MAX(MIN_LEV_FIELDX, fieldx), MAX_LEV_FIELDX);
  level->fieldy = MIN(MAX(MIN_LEV_FIELDY, fieldy), MAX_LEV_FIELDY);

  for (y = 0; y < fieldy; y++) for (x = 0; x < fieldx; x++)
  {
    unsigned short word = getDecodedWord_DC(getFile16BitBE(file), FALSE);
    int element_dc = ((word & 0xff) << 8) | ((word >> 8) & 0xff);

    if (x < MAX_LEV_FIELDX && y < MAX_LEV_FIELDY)
      level->field[x][y] = getMappedElement_DC(element_dc);
  }

  x = MIN(MAX(0, (header[10] | (header[11] << 8)) - 1), MAX_LEV_FIELDX - 1);
  y = MIN(MAX(0, (header[12] | (header[13] << 8)) - 1), MAX_LEV_FIELDY - 1);
  level->field[x][y] = EL_PLAYER_1;

  x = MIN(MAX(0, (header[14] | (header[15] << 8)) - 1), MAX_LEV_FIELDX - 1);
  y = MIN(MAX(0, (header[16] | (header[17] << 8)) - 1), MAX_LEV_FIELDY - 1);
  level->field[x][y] = EL_PLAYER_2;

  level->gems_needed		= header[18] | (header[19] << 8);

  level->score[SC_EMERALD]	= header[20] | (header[21] << 8);
  level->score[SC_DIAMOND]	= header[22] | (header[23] << 8);
  level->score[SC_PEARL]	= header[24] | (header[25] << 8);
  level->score[SC_CRYSTAL]	= header[26] | (header[27] << 8);
  level->score[SC_NUT]		= header[28] | (header[29] << 8);
  level->score[SC_ROBOT]	= header[30] | (header[31] << 8);
  level->score[SC_SPACESHIP]	= header[32] | (header[33] << 8);
  level->score[SC_BUG]		= header[34] | (header[35] << 8);
  level->score[SC_YAMYAM]	= header[36] | (header[37] << 8);
  level->score[SC_DYNAMITE]	= header[38] | (header[39] << 8);
  level->score[SC_KEY]		= header[40] | (header[41] << 8);
  level->score[SC_TIME_BONUS]	= header[42] | (header[43] << 8);

  level->time			= header[44] | (header[45] << 8);

  level->amoeba_speed		= header[46] | (header[47] << 8);
  level->time_light		= header[48] | (header[49] << 8);
  level->time_timegate		= header[50] | (header[51] << 8);
  level->time_wheel		= header[52] | (header[53] << 8);
  level->time_magic_wall	= header[54] | (header[55] << 8);
  level->extra_time		= header[56] | (header[57] << 8);
  level->shield_normal_time	= header[58] | (header[59] << 8);

  // shield and extra time elements do not have a score
  level->score[SC_SHIELD]	= 0;
  level->extra_time_score	= 0;

  // set time for normal and deadly shields to the same value
  level->shield_deadly_time	= level->shield_normal_time;

  // Diamond Caves has the same (strange) behaviour as Emerald Mine that gems
  // can slip down from flat walls, like normal walls and steel walls
  level->em_slippery_gems = TRUE;

  // time score is counted for each 10 seconds left in Diamond Caves levels
  level->time_score_base = 10;
}

static void LoadLevelFromFileInfo_DC(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  char *filename = level_file_info->filename;
  File *file;
  int num_magic_bytes = 8;
  char magic_bytes[num_magic_bytes + 1];
  int num_levels_to_skip = level_file_info->nr - leveldir_current->first_level;

  if (!(file = openFile(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    if (!level_info_only)
      Warn("cannot read level '%s' -- using empty level", filename);

    return;
  }

  // fseek(file, 0x0000, SEEK_SET);

  if (level_file_info->packed)
  {
    // read "magic bytes" from start of file
    if (getStringFromFile(file, magic_bytes, num_magic_bytes + 1) == NULL)
      magic_bytes[0] = '\0';

    // check "magic bytes" for correct file format
    if (!strPrefix(magic_bytes, "DC2"))
    {
      level->no_valid_file = TRUE;

      Warn("unknown DC level file '%s' -- using empty level", filename);

      return;
    }

    if (strPrefix(magic_bytes, "DC2Win95") ||
	strPrefix(magic_bytes, "DC2Win98"))
    {
      int position_first_level = 0x00fa;
      int extra_bytes = 4;
      int skip_bytes;

      // advance file stream to first level inside the level package
      skip_bytes = position_first_level - num_magic_bytes - extra_bytes;

      // each block of level data is followed by block of non-level data
      num_levels_to_skip *= 2;

      // at least skip header bytes, therefore use ">= 0" instead of "> 0"
      while (num_levels_to_skip >= 0)
      {
	// advance file stream to next level inside the level package
	if (seekFile(file, skip_bytes, SEEK_CUR) != 0)
	{
	  level->no_valid_file = TRUE;

	  Warn("cannot fseek in file '%s' -- using empty level", filename);

	  return;
	}

	// skip apparently unused extra bytes following each level
	ReadUnusedBytesFromFile(file, extra_bytes);

	// read size of next level in level package
	skip_bytes = getFile32BitLE(file);

	num_levels_to_skip--;
      }
    }
    else
    {
      level->no_valid_file = TRUE;

      Warn("unknown DC2 level file '%s' -- using empty level", filename);

      return;
    }
  }

  LoadLevelFromFileStream_DC(file, level);

  closeFile(file);
}


// ----------------------------------------------------------------------------
// functions for loading SB level
// ----------------------------------------------------------------------------

int getMappedElement_SB(int element_ascii, boolean use_ces)
{
  static struct
  {
    int ascii;
    int sb;
    int ce;
  }
  sb_element_mapping[] =
  {
    { ' ', EL_EMPTY,                EL_CUSTOM_1 },  // floor (space)
    { '#', EL_STEELWALL,            EL_CUSTOM_2 },  // wall
    { '@', EL_PLAYER_1,             EL_CUSTOM_3 },  // player
    { '$', EL_SOKOBAN_OBJECT,       EL_CUSTOM_4 },  // box
    { '.', EL_SOKOBAN_FIELD_EMPTY,  EL_CUSTOM_5 },  // goal square
    { '*', EL_SOKOBAN_FIELD_FULL,   EL_CUSTOM_6 },  // box on goal square
    { '+', EL_SOKOBAN_FIELD_PLAYER, EL_CUSTOM_7 },  // player on goal square
    { '_', EL_INVISIBLE_STEELWALL,  EL_FROM_LEVEL_TEMPLATE },  // floor beyond border

    { 0,   -1,                      -1          },
  };

  int i;

  for (i = 0; sb_element_mapping[i].ascii != 0; i++)
    if (element_ascii == sb_element_mapping[i].ascii)
      return (use_ces ? sb_element_mapping[i].ce : sb_element_mapping[i].sb);

  return EL_UNDEFINED;
}

static void SetLevelSettings_SB(struct LevelInfo *level)
{
  // time settings
  level->time = 0;
  level->use_step_counter = TRUE;

  // score settings
  level->score[SC_TIME_BONUS] = 0;
  level->time_score_base = 1;
  level->rate_time_over_score = TRUE;

  // game settings
  level->auto_exit_sokoban = TRUE;
}

static void LoadLevelFromFileInfo_SB(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  char *filename = level_file_info->filename;
  char line[MAX_LINE_LEN], line_raw[MAX_LINE_LEN], previous_line[MAX_LINE_LEN];
  char last_comment[MAX_LINE_LEN];
  char level_name[MAX_LINE_LEN];
  char *line_ptr;
  File *file;
  int num_levels_to_skip = level_file_info->nr - leveldir_current->first_level;
  boolean read_continued_line = FALSE;
  boolean reading_playfield = FALSE;
  boolean got_valid_playfield_line = FALSE;
  boolean invalid_playfield_char = FALSE;
  boolean load_xsb_to_ces = check_special_flags("load_xsb_to_ces");
  int file_level_nr = 0;
  int x = 0, y = 0;		// initialized to make compilers happy

  last_comment[0] = '\0';
  level_name[0] = '\0';

  if (!(file = openFile(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    if (!level_info_only)
      Warn("cannot read level '%s' -- using empty level", filename);

    return;
  }

  while (!checkEndOfFile(file))
  {
    // level successfully read, but next level may follow here
    if (!got_valid_playfield_line && reading_playfield)
    {
      // read playfield from single level file -- skip remaining file
      if (!level_file_info->packed)
	break;

      if (file_level_nr >= num_levels_to_skip)
	break;

      file_level_nr++;

      last_comment[0] = '\0';
      level_name[0] = '\0';

      reading_playfield = FALSE;
    }

    got_valid_playfield_line = FALSE;

    // read next line of input file
    if (!getStringFromFile(file, line, MAX_LINE_LEN))
      break;

    // cut trailing line break (this can be newline and/or carriage return)
    for (line_ptr = &line[strlen(line)]; line_ptr >= line; line_ptr--)
      if ((*line_ptr == '\n' || *line_ptr == '\r') && *(line_ptr + 1) == '\0')
        *line_ptr = '\0';

    // copy raw input line for later use (mainly debugging output)
    strcpy(line_raw, line);

    if (read_continued_line)
    {
      // append new line to existing line, if there is enough space
      if (strlen(previous_line) + strlen(line_ptr) < MAX_LINE_LEN)
        strcat(previous_line, line_ptr);

      strcpy(line, previous_line);      // copy storage buffer to line

      read_continued_line = FALSE;
    }

    // if the last character is '\', continue at next line
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\\')
    {
      line[strlen(line) - 1] = '\0';    // cut off trailing backslash
      strcpy(previous_line, line);      // copy line to storage buffer

      read_continued_line = TRUE;

      continue;
    }

    // skip empty lines
    if (line[0] == '\0')
      continue;

    // extract comment text from comment line
    if (line[0] == ';')
    {
      for (line_ptr = line; *line_ptr; line_ptr++)
        if (*line_ptr != ' ' && *line_ptr != '\t' && *line_ptr != ';')
          break;

      strcpy(last_comment, line_ptr);

      continue;
    }

    // extract level title text from line containing level title
    if (line[0] == '\'')
    {
      strcpy(level_name, &line[1]);

      if (strlen(level_name) > 0 && level_name[strlen(level_name) - 1] == '\'')
	level_name[strlen(level_name) - 1] = '\0';

      continue;
    }

    // skip lines containing only spaces (or empty lines)
    for (line_ptr = line; *line_ptr; line_ptr++)
      if (*line_ptr != ' ')
	break;
    if (*line_ptr == '\0')
      continue;

    // at this point, we have found a line containing part of a playfield

    got_valid_playfield_line = TRUE;

    if (!reading_playfield)
    {
      reading_playfield = TRUE;
      invalid_playfield_char = FALSE;

      for (x = 0; x < MAX_LEV_FIELDX; x++)
	for (y = 0; y < MAX_LEV_FIELDY; y++)
	  level->field[x][y] = getMappedElement_SB(' ', load_xsb_to_ces);

      level->fieldx = 0;
      level->fieldy = 0;

      // start with topmost tile row
      y = 0;
    }

    // skip playfield line if larger row than allowed
    if (y >= MAX_LEV_FIELDY)
      continue;

    // start with leftmost tile column
    x = 0;

    // read playfield elements from line
    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      int mapped_sb_element = getMappedElement_SB(*line_ptr, load_xsb_to_ces);

      // stop parsing playfield line if larger column than allowed
      if (x >= MAX_LEV_FIELDX)
	break;

      if (mapped_sb_element == EL_UNDEFINED)
      {
	invalid_playfield_char = TRUE;

	break;
      }

      level->field[x][y] = mapped_sb_element;

      // continue with next tile column
      x++;

      level->fieldx = MAX(x, level->fieldx);
    }

    if (invalid_playfield_char)
    {
      // if first playfield line, treat invalid lines as comment lines
      if (y == 0)
	reading_playfield = FALSE;

      continue;
    }

    // continue with next tile row
    y++;
  }

  closeFile(file);

  level->fieldy = y;

  level->fieldx = MIN(MAX(MIN_LEV_FIELDX, level->fieldx), MAX_LEV_FIELDX);
  level->fieldy = MIN(MAX(MIN_LEV_FIELDY, level->fieldy), MAX_LEV_FIELDY);

  if (!reading_playfield)
  {
    level->no_valid_file = TRUE;

    Warn("cannot read level '%s' -- using empty level", filename);

    return;
  }

  if (*level_name != '\0')
  {
    strncpy(level->name, level_name, MAX_LEVEL_NAME_LEN);
    level->name[MAX_LEVEL_NAME_LEN] = '\0';
  }
  else if (*last_comment != '\0')
  {
    strncpy(level->name, last_comment, MAX_LEVEL_NAME_LEN);
    level->name[MAX_LEVEL_NAME_LEN] = '\0';
  }
  else
  {
    sprintf(level->name, "--> Level %d <--", level_file_info->nr);
  }

  // set all empty fields beyond the border walls to invisible steel wall
  for (y = 0; y < level->fieldy; y++) for (x = 0; x < level->fieldx; x++)
  {
    if ((x == 0 || x == level->fieldx - 1 ||
	 y == 0 || y == level->fieldy - 1) &&
	level->field[x][y] == getMappedElement_SB(' ', load_xsb_to_ces))
      FloodFillLevel(x, y, getMappedElement_SB('_', load_xsb_to_ces),
		     level->field, level->fieldx, level->fieldy);
  }

  // set special level settings for Sokoban levels
  SetLevelSettings_SB(level);

  if (load_xsb_to_ces)
  {
    // special global settings can now be set in level template
    level->use_custom_template = TRUE;
  }
}


// -------------------------------------------------------------------------
// functions for handling native levels
// -------------------------------------------------------------------------

static void LoadLevelFromFileInfo_BD(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  int pos = 0;

  // determine position of requested level inside level package
  if (level_file_info->packed)
    pos = level_file_info->nr - leveldir_current->first_level;

  if (!LoadNativeLevel_BD(level_file_info->filename, pos, level_info_only))
    level->no_valid_file = TRUE;
}

static void LoadLevelFromFileInfo_EM(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  if (!LoadNativeLevel_EM(level_file_info->filename, level_info_only))
    level->no_valid_file = TRUE;
}

static void LoadLevelFromFileInfo_SP(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  int pos = 0;

  // determine position of requested level inside level package
  if (level_file_info->packed)
    pos = level_file_info->nr - leveldir_current->first_level;

  if (!LoadNativeLevel_SP(level_file_info->filename, pos, level_info_only))
    level->no_valid_file = TRUE;
}

static void LoadLevelFromFileInfo_MM(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  if (!LoadNativeLevel_MM(level_file_info->filename, level_info_only))
    level->no_valid_file = TRUE;
}

void CopyNativeLevel_RND_to_Native(struct LevelInfo *level)
{
  if (level->game_engine_type == GAME_ENGINE_TYPE_BD)
    CopyNativeLevel_RND_to_BD(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_EM)
    CopyNativeLevel_RND_to_EM(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_SP)
    CopyNativeLevel_RND_to_SP(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_MM)
    CopyNativeLevel_RND_to_MM(level);
}

void CopyNativeLevel_Native_to_RND(struct LevelInfo *level)
{
  if (level->game_engine_type == GAME_ENGINE_TYPE_BD)
    CopyNativeLevel_BD_to_RND(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_EM)
    CopyNativeLevel_EM_to_RND(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_SP)
    CopyNativeLevel_SP_to_RND(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_MM)
    CopyNativeLevel_MM_to_RND(level);
}

void SaveNativeLevel(struct LevelInfo *level)
{
  // saving native level files only supported for some game engines
  if (level->game_engine_type != GAME_ENGINE_TYPE_BD &&
      level->game_engine_type != GAME_ENGINE_TYPE_SP)
    return;

  char *file_ext = (level->game_engine_type == GAME_ENGINE_TYPE_BD ? "bd" :
		    level->game_engine_type == GAME_ENGINE_TYPE_SP ? "sp" : "");
  char *basename = getSingleLevelBasenameExt(level->file_info.nr, file_ext);
  char *filename = getLevelFilenameFromBasename(basename);

  if (fileExists(filename) && !Request("Native level file already exists! Overwrite it?", REQ_ASK))
    return;

  boolean success = FALSE;

  if (level->game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    CopyNativeLevel_RND_to_BD(level);
    // CopyNativeTape_RND_to_BD(level);

    success = SaveNativeLevel_BD(filename);
  }
  else if (level->game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    CopyNativeLevel_RND_to_SP(level);
    CopyNativeTape_RND_to_SP(level);

    success = SaveNativeLevel_SP(filename);
  }

  if (success)
    Request("Native level file saved!", REQ_CONFIRM);
  else
    Request("Failed to save native level file!", REQ_CONFIRM);
}


// ----------------------------------------------------------------------------
// functions for loading generic level
// ----------------------------------------------------------------------------

static void LoadLevelFromFileInfo(struct LevelInfo *level,
				  struct LevelFileInfo *level_file_info,
				  boolean level_info_only)
{
  // always start with reliable default values
  setLevelInfoToDefaults(level, level_info_only, TRUE);

  switch (level_file_info->type)
  {
    case LEVEL_FILE_TYPE_RND:
      LoadLevelFromFileInfo_RND(level, level_file_info, level_info_only);
      break;

    case LEVEL_FILE_TYPE_BD:
      LoadLevelFromFileInfo_BD(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_BD;
      break;

    case LEVEL_FILE_TYPE_EM:
      LoadLevelFromFileInfo_EM(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_EM;
      break;

    case LEVEL_FILE_TYPE_SP:
      LoadLevelFromFileInfo_SP(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_SP;
      break;

    case LEVEL_FILE_TYPE_MM:
      LoadLevelFromFileInfo_MM(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_MM;
      break;

    case LEVEL_FILE_TYPE_DC:
      LoadLevelFromFileInfo_DC(level, level_file_info, level_info_only);
      break;

    case LEVEL_FILE_TYPE_SB:
      LoadLevelFromFileInfo_SB(level, level_file_info, level_info_only);
      break;

    default:
      LoadLevelFromFileInfo_RND(level, level_file_info, level_info_only);
      break;
  }

  // if level file is invalid, restore level structure to default values
  if (level->no_valid_file)
    setLevelInfoToDefaults(level, level_info_only, FALSE);

  if (check_special_flags("use_native_bd_game_engine"))
    level->game_engine_type = GAME_ENGINE_TYPE_BD;

  if (level->game_engine_type == GAME_ENGINE_TYPE_UNKNOWN)
    level->game_engine_type = GAME_ENGINE_TYPE_RND;

  if (level_file_info->type != LEVEL_FILE_TYPE_RND)
    CopyNativeLevel_Native_to_RND(level);
}

void LoadLevelFromFilename(struct LevelInfo *level, char *filename)
{
  static struct LevelFileInfo level_file_info;

  // always start with reliable default values
  setFileInfoToDefaults(&level_file_info);

  level_file_info.nr = 0;			// unknown level number
  level_file_info.type = LEVEL_FILE_TYPE_RND;	// no others supported yet

  setString(&level_file_info.filename, filename);

  LoadLevelFromFileInfo(level, &level_file_info, FALSE);
}

static void LoadLevel_FixEnvelopes(struct LevelInfo *level, boolean skip_single_lines)
{
  // This function removes newlines in envelopes after lines of text ending in the last column
  // of the envelope. In earlier versions, these newlines were removed when displaying envelopes,
  // but caused trouble in the level editor. In version 4.3.2.3, this problem was partially
  // fixed in the level editor (but only for single full-width text lines followed by a newline,
  // not for multiple lines ending in the last column, followed by a newline), but now produced
  // unwanted newlines in the game for envelopes stored by previous game versions, which was not
  // intended by the level author (and sometimes caused text lines not being displayed anymore at
  // the bottom of the envelope).
  //
  // This function should solve these problems by removing such newline characters from envelopes
  // stored by older game versions.

  int envelope_nr;

  for (envelope_nr = 0; envelope_nr < NUM_ENVELOPES; envelope_nr++)
  {
    char *envelope_ptr = level->envelope[envelope_nr].text;
    int envelope_xsize = level->envelope[envelope_nr].xsize;
    int envelope_size = strlen(envelope_ptr);
    int start = 0;
    int i;

    for (i = 0; i < envelope_size; i++)
    {
      // check for newlines in envelope
      if (envelope_ptr[i] == '\n')
      {
        int line_length = i - start;

        // check for (non-empty) lines that are a multiple of the envelope width,
        // causing a line break inside the envelope (text area in editor and in game)
        if (line_length > 0 && line_length % envelope_xsize == 0)
        {
          // special case: skip fixing single lines for newer versions
          boolean skip_fixing_line = (line_length == 1 && skip_single_lines);

          if (!skip_fixing_line)
          {
            int j;

            // remove newline character from string
            for (j = i; j < envelope_size; j++)
              envelope_ptr[j] = envelope_ptr[j + 1];
          }

          // continue with next line (that was copied over the newline)
          start = i;
        }
        else
        {
          // continue with next character after newline
          start = i + 1;
        }
      }
    }
  }
}

static void LoadLevel_InitVersion(struct LevelInfo *level)
{
  int i, j;

  if (leveldir_current == NULL)		// only when dumping level
    return;

  // all engine modifications also valid for levels which use latest engine
  if (level->game_version < VERSION_IDENT(3,2,0,5))
  {
    // time bonus score was given for 10 s instead of 1 s before 3.2.0-5
    level->time_score_base = 10;
  }

  if (leveldir_current->latest_engine)
  {
    // ---------- use latest game engine --------------------------------------

    /* For all levels which are forced to use the latest game engine version
       (normally all but user contributed, private and undefined levels), set
       the game engine version to the actual version; this allows for actual
       corrections in the game engine to take effect for existing, converted
       levels (from "classic" or other existing games) to make the emulation
       of the corresponding game more accurate, while (hopefully) not breaking
       existing levels created from other players. */

    level->game_version = GAME_VERSION_ACTUAL;

    /* Set special EM style gems behaviour: EM style gems slip down from
       normal, steel and growing wall. As this is a more fundamental change,
       it seems better to set the default behaviour to "off" (as it is more
       natural) and make it configurable in the level editor (as a property
       of gem style elements). Already existing converted levels (neither
       private nor contributed levels) are changed to the new behaviour. */

    if (level->file_version < FILE_VERSION_2_0)
      level->em_slippery_gems = TRUE;

    return;
  }

  // ---------- use game engine the level was created with --------------------

  /* For all levels which are not forced to use the latest game engine
     version (normally user contributed, private and undefined levels),
     use the version of the game engine the levels were created for.

     Since 2.0.1, the game engine version is now directly stored
     in the level file (chunk "VERS"), so there is no need anymore
     to set the game version from the file version (except for old,
     pre-2.0 levels, where the game version is still taken from the
     file format version used to store the level -- see above). */

  // player was faster than enemies in 1.0.0 and before
  if (level->file_version == FILE_VERSION_1_0)
    for (i = 0; i < MAX_PLAYERS; i++)
      level->initial_player_stepsize[i] = STEPSIZE_FAST;

  // default behaviour for EM style gems was "slippery" only in 2.0.1
  if (level->game_version == VERSION_IDENT(2,0,1,0))
    level->em_slippery_gems = TRUE;

  // springs could be pushed over pits before (pre-release version) 2.2.0
  if (level->game_version < VERSION_IDENT(2,2,0,0))
    level->use_spring_bug = TRUE;

  if (level->game_version < VERSION_IDENT(3,2,0,5))
  {
    // time orb caused limited time in endless time levels before 3.2.0-5
    level->use_time_orb_bug = TRUE;

    // default behaviour for snapping was "no snap delay" before 3.2.0-5
    level->block_snap_field = FALSE;

    // extra time score was same value as time left score before 3.2.0-5
    level->extra_time_score = level->score[SC_TIME_BONUS];
  }

  if (level->game_version < VERSION_IDENT(3,2,0,7))
  {
    // default behaviour for snapping was "not continuous" before 3.2.0-7
    level->continuous_snapping = FALSE;
  }

  // only few elements were able to actively move into acid before 3.1.0
  // trigger settings did not exist before 3.1.0; set to default "any"
  if (level->game_version < VERSION_IDENT(3,1,0,0))
  {
    // correct "can move into acid" settings (all zero in old levels)

    level->can_move_into_acid_bits = 0; // nothing can move into acid
    level->dont_collide_with_bits = 0; // nothing is deadly when colliding

    setMoveIntoAcidProperty(level, EL_ROBOT,     TRUE);
    setMoveIntoAcidProperty(level, EL_SATELLITE, TRUE);
    setMoveIntoAcidProperty(level, EL_PENGUIN,   TRUE);
    setMoveIntoAcidProperty(level, EL_BALLOON,   TRUE);

    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
      SET_PROPERTY(EL_CUSTOM_START + i, EP_CAN_MOVE_INTO_ACID, TRUE);

    // correct trigger settings (stored as zero == "none" in old levels)

    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      for (j = 0; j < ei->num_change_pages; j++)
      {
	struct ElementChangeInfo *change = &ei->change_page[j];

	change->trigger_player = CH_PLAYER_ANY;
	change->trigger_page = CH_PAGE_ANY;
      }
    }
  }

  // try to detect and fix "Snake Bite" levels, which are broken with 3.2.0
  {
    int element = EL_CUSTOM_256;
    struct ElementInfo *ei = &element_info[element];
    struct ElementChangeInfo *change = &ei->change_page[0];

    /* This is needed to fix a problem that was caused by a bugfix in function
       game.c/CreateFieldExt() introduced with 3.2.0 that corrects the behaviour
       when a custom element changes to EL_SOKOBAN_FIELD_PLAYER (before, it did
       not replace walkable elements, but instead just placed the player on it,
       without placing the Sokoban field under the player). Unfortunately, this
       breaks "Snake Bite" style levels when the snake is halfway through a door
       that just closes (the snake head is still alive and can be moved in this
       case). This can be fixed by replacing the EL_SOKOBAN_FIELD_PLAYER by the
       player (without Sokoban element) which then gets killed as designed). */

    if ((strncmp(leveldir_current->identifier, "snake_bite", 10) == 0 ||
	 strncmp(ei->description, "pause b4 death", 14) == 0) &&
	change->target_element == EL_SOKOBAN_FIELD_PLAYER)
      change->target_element = EL_PLAYER_1;
  }

  // try to detect and fix "Zelda" style levels, which are broken with 3.2.5
  if (level->game_version < VERSION_IDENT(3,2,5,0))
  {
    /* This is needed to fix a problem that was caused by a bugfix in function
       game.c/CheckTriggeredElementChangeExt() introduced with 3.2.5 that
       corrects the behaviour when a custom element changes to another custom
       element with a higher element number that has change actions defined.
       Normally, only one change per frame is allowed for custom elements.
       Therefore, it is checked if a custom element already changed in the
       current frame; if it did, subsequent changes are suppressed.
       Unfortunately, this is only checked for element changes, but not for
       change actions, which are still executed. As the function above loops
       through all custom elements from lower to higher, an element change
       resulting in a lower CE number won't be checked again, while a target
       element with a higher number will also be checked, and potential change
       actions will get executed for this CE, too (which is wrong), while
       further changes are ignored (which is correct). As this bugfix breaks
       Zelda II (and introduces graphical bugs to Zelda I, and also breaks a
       few other levels like Alan Bond's "FMV"), allow the previous, incorrect
       behaviour for existing levels and tapes that make use of this bug */

    level->use_action_after_change_bug = TRUE;
  }

  // not centering level after relocating player was default only in 3.2.3
  if (level->game_version == VERSION_IDENT(3,2,3,0))	// (no pre-releases)
    level->shifted_relocation = TRUE;

  // EM style elements always chain-exploded in R'n'D engine before 3.2.6
  if (level->game_version < VERSION_IDENT(3,2,6,0))
    level->em_explodes_by_fire = TRUE;

  // levels were solved by the first player entering an exit up to 4.1.0.0
  if (level->game_version <= VERSION_IDENT(4,1,0,0))
    level->solved_by_one_player = TRUE;

  // game logic of "game of life" and "biomaze" was buggy before 4.1.1.1
  if (level->game_version < VERSION_IDENT(4,1,1,1))
    level->use_life_bugs = TRUE;

  // only Sokoban fields (but not objects) had to be solved before 4.1.1.1
  if (level->game_version < VERSION_IDENT(4,1,1,1))
    level->sb_objects_needed = FALSE;

  // CE actions were triggered by unfinished digging/collecting up to 4.2.2.0
  if (level->game_version <= VERSION_IDENT(4,2,2,0))
    level->finish_dig_collect = FALSE;

  // CE changing to player was kept under the player if walkable up to 4.2.3.1
  if (level->game_version <= VERSION_IDENT(4,2,3,1))
    level->keep_walkable_ce = TRUE;

  // envelopes may contain broken or too many line breaks before 4.4.0.0
  if (level->game_version < VERSION_IDENT(4,4,0,0))
    LoadLevel_FixEnvelopes(level, (level->game_version >= VERSION_IDENT(4,3,2,3)));
}

static void LoadLevel_InitSettings_SB(struct LevelInfo *level)
{
  boolean is_sokoban_level = TRUE;    // unless non-Sokoban elements found
  int x, y;

  // check if this level is (not) a Sokoban level
  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      if (!IS_SB_ELEMENT(Tile[x][y]))
	is_sokoban_level = FALSE;

  if (is_sokoban_level)
  {
    // set special level settings for Sokoban levels
    SetLevelSettings_SB(level);
  }
}

static void LoadLevel_InitColorSettings(struct LevelInfo *level)
{
  GdCave *cave = level->native_bd_level->cave;
  int i;

  // copy level colors to native BD level
  // (this workaround is needed as long as color template handling is still BD specific)
  for (i = 0; i < MAX_LEVEL_COLORS; i++)
  {
    cave->color[i]	= level->bd_color[i];
    cave->base_color[i]	= level->bd_base_color[i];
  }

  // set default color type and colors for BD style level colors
  SetDefaultLevelColorType_BD();
  SetDefaultLevelColors_BD();
}

static void LoadLevel_InitSettings(struct LevelInfo *level)
{
  // adjust level settings for (non-native) Sokoban-style levels
  LoadLevel_InitSettings_SB(level);

  // rename levels with title "nameless level" or if renaming is forced
  if (leveldir_current->empty_level_name != NULL &&
      (strEqual(level->name, NAMELESS_LEVEL_NAME) ||
       leveldir_current->force_level_name))
    snprintf(level->name, MAX_LEVEL_NAME_LEN + 1,
	     leveldir_current->empty_level_name, level_nr);

  // initialize level specific colors
  LoadLevel_InitColorSettings(level);
}

static void LoadLevel_InitStandardElements(struct LevelInfo *level)
{
  int i, x, y;

  // map elements that have changed in newer versions
  level->amoeba_content = getMappedElementByVersion(level->amoeba_content,
						    level->game_version);
  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (x = 0; x < 3; x++)
      for (y = 0; y < 3; y++)
	level->yamyam_content[i].e[x][y] =
	  getMappedElementByVersion(level->yamyam_content[i].e[x][y],
				    level->game_version);

}

static void LoadLevel_InitCustomElements(struct LevelInfo *level)
{
  int i, j;

  // map custom element change events that have changed in newer versions
  // (these following values were accidentally changed in version 3.0.1)
  // (this seems to be needed only for 'ab_levelset3' and 'ab_levelset4')
  if (level->game_version <= VERSION_IDENT(3,0,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      // order of checking and copying events to be mapped is important
      // (do not change the start and end value -- they are constant)
      for (j = CE_BY_OTHER_ACTION; j >= CE_VALUE_GETS_ZERO; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 2))
	{
	  SET_CHANGE_EVENT(element, j - 2, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }

      // order of checking and copying events to be mapped is important
      // (do not change the start and end value -- they are constant)
      for (j = CE_PLAYER_COLLECTS_X; j >= CE_HITTING_SOMETHING; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 1))
	{
	  SET_CHANGE_EVENT(element, j - 1, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }
    }
  }

  // initialize "can_change" field for old levels with only one change page
  if (level->game_version <= VERSION_IDENT(3,0,2,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (CAN_CHANGE(element))
	element_info[element].change->can_change = TRUE;
    }
  }

  // correct custom element values (for old levels without these options)
  if (level->game_version < VERSION_IDENT(3,1,1,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      if (ei->access_direction == MV_NO_DIRECTION)
	ei->access_direction = MV_ALL_DIRECTIONS;
    }
  }

  // correct custom element values (fix invalid values for all versions)
  if (1)
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      for (j = 0; j < ei->num_change_pages; j++)
      {
	struct ElementChangeInfo *change = &ei->change_page[j];

	if (change->trigger_player == CH_PLAYER_NONE)
	  change->trigger_player = CH_PLAYER_ANY;

	if (change->trigger_side == CH_SIDE_NONE)
	  change->trigger_side = CH_SIDE_ANY;
      }
    }
  }

  // initialize "can_explode" field for old levels which did not store this
  // !!! CHECK THIS -- "<= 3,1,0,0" IS PROBABLY WRONG !!!
  if (level->game_version <= VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (EXPLODES_1X1_OLD(element))
	element_info[element].explosion_type = EXPLODES_1X1;

      SET_PROPERTY(element, EP_CAN_EXPLODE, (EXPLODES_BY_FIRE(element) ||
					     EXPLODES_SMASHED(element) ||
					     EXPLODES_IMPACT(element)));
    }
  }

  // correct previously hard-coded move delay values for maze runner style
  if (level->game_version < VERSION_IDENT(3,1,1,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (element_info[element].move_pattern & MV_MAZE_RUNNER_STYLE)
      {
	// previously hard-coded and therefore ignored
	element_info[element].move_delay_fixed = 9;
	element_info[element].move_delay_random = 0;
      }
    }
  }

  // set some other uninitialized values of custom elements in older levels
  if (level->game_version < VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      element_info[element].access_direction = MV_ALL_DIRECTIONS;

      element_info[element].explosion_delay = 17;
      element_info[element].ignition_delay = 8;
    }
  }

  // set mouse click change events to work for left/middle/right mouse button
  if (level->game_version < VERSION_IDENT(4,2,3,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      for (j = 0; j < ei->num_change_pages; j++)
      {
	struct ElementChangeInfo *change = &ei->change_page[j];

	if (change->has_event[CE_CLICKED_BY_MOUSE] ||
	    change->has_event[CE_PRESSED_BY_MOUSE] ||
	    change->has_event[CE_MOUSE_CLICKED_ON_X] ||
	    change->has_event[CE_MOUSE_PRESSED_ON_X])
	  change->trigger_side = CH_SIDE_ANY;
      }
    }
  }
}

static void LoadLevel_InitElements(struct LevelInfo *level)
{
  LoadLevel_InitStandardElements(level);

  if (level->file_has_custom_elements)
    LoadLevel_InitCustomElements(level);

  // initialize element properties for level editor etc.
  InitElementPropertiesEngine(level->game_version);
  InitElementPropertiesGfxElement();
}

static void LoadLevel_InitPlayfield(struct LevelInfo *level)
{
  int x, y;

  // map elements that have changed in newer versions
  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      level->field[x][y] = getMappedElementByVersion(level->field[x][y],
						     level->game_version);

  // clear unused playfield data (nicer if level gets resized in editor)
  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      if (x >= level->fieldx || y >= level->fieldy)
	level->field[x][y] = EL_EMPTY;

  // copy elements to runtime playfield array
  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      Tile[x][y] = level->field[x][y];

  // initialize level size variables for faster access
  lev_fieldx = level->fieldx;
  lev_fieldy = level->fieldy;

  // determine border element for this level
  if (level->file_info.type == LEVEL_FILE_TYPE_DC)
    BorderElement = EL_EMPTY;	// (in editor, SetBorderElement() is used)
  else
    SetBorderElement();
}

static void LoadLevel_InitNativeEngines(struct LevelInfo *level)
{
  struct LevelFileInfo *level_file_info = &level->file_info;

  if (level_file_info->type == LEVEL_FILE_TYPE_RND)
    CopyNativeLevel_RND_to_Native(level);
}

static void LoadLevelTemplate_LoadAndInit(void)
{
  LoadLevelFromFileInfo(&level_template, &level_template.file_info, FALSE);

  LoadLevel_InitVersion(&level_template);
  LoadLevel_InitElements(&level_template);
  LoadLevel_InitSettings(&level_template);

  ActivateLevelTemplate();
}

void LoadLevelTemplate(int nr)
{
  if (!fileExists(getGlobalLevelTemplateFilename()))
  {
    Warn("no level template found for this level");

    return;
  }

  setLevelFileInfo(&level_template.file_info, nr);

  LoadLevelTemplate_LoadAndInit();
}

static void LoadNetworkLevelTemplate(struct NetworkLevelInfo *network_level)
{
  copyLevelFileInfo(&network_level->tmpl_info, &level_template.file_info);

  LoadLevelTemplate_LoadAndInit();
}

static void LoadLevel_LoadAndInit(struct NetworkLevelInfo *network_level)
{
  LoadLevelFromFileInfo(&level, &level.file_info, FALSE);

  if (level.use_custom_template)
  {
    if (network_level != NULL)
      LoadNetworkLevelTemplate(network_level);
    else
      LoadLevelTemplate(-1);
  }

  LoadLevel_InitVersion(&level);
  LoadLevel_InitElements(&level);
  LoadLevel_InitPlayfield(&level);
  LoadLevel_InitSettings(&level);

  LoadLevel_InitNativeEngines(&level);

  InitColorTemplateImagesIfNeeded();
}

void LoadLevel(int nr)
{
  SetLevelSetInfo(leveldir_current->identifier, nr);

  setLevelFileInfo(&level.file_info, nr);

  LoadLevel_LoadAndInit(NULL);
}

void LoadLevelInfoOnly(int nr)
{
  setLevelFileInfo(&level.file_info, nr);

  LoadLevelFromFileInfo(&level, &level.file_info, TRUE);
}

void LoadNetworkLevel(struct NetworkLevelInfo *network_level)
{
  SetLevelSetInfo(network_level->leveldir_identifier,
		  network_level->file_info.nr);

  copyLevelFileInfo(&network_level->file_info, &level.file_info);

  LoadLevel_LoadAndInit(network_level);
}

static int SaveLevel_VERS(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;

  chunk_size += putFileVersion(file, level->file_version);
  chunk_size += putFileVersion(file, level->game_version);

  return chunk_size;
}

static int SaveLevel_DATE(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;

  chunk_size += putFile16BitBE(file, level->creation_date.year);
  chunk_size += putFile8Bit(file,    level->creation_date.month);
  chunk_size += putFile8Bit(file,    level->creation_date.day);

  return chunk_size;
}

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_HEAD(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  putFile8Bit(file, level->fieldx);
  putFile8Bit(file, level->fieldy);

  putFile16BitBE(file, level->time);
  putFile16BitBE(file, level->gems_needed);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    putFile8Bit(file, level->name[i]);

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    putFile8Bit(file, level->score[i]);

  for (i = 0; i < STD_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile8Bit(file, (level->encoding_16bit_yamyam ? EL_EMPTY :
			   level->yamyam_content[i].e[x][y]));
  putFile8Bit(file, level->amoeba_speed);
  putFile8Bit(file, level->time_magic_wall);
  putFile8Bit(file, level->time_wheel);
  putFile8Bit(file, (level->encoding_16bit_amoeba ? EL_EMPTY :
		     level->amoeba_content));
  putFile8Bit(file, (level->initial_player_stepsize == STEPSIZE_FAST ? 1 : 0));
  putFile8Bit(file, (level->initial_gravity ? 1 : 0));
  putFile8Bit(file, (level->encoding_16bit_field ? 1 : 0));
  putFile8Bit(file, (level->em_slippery_gems ? 1 : 0));

  putFile8Bit(file, (level->use_custom_template ? 1 : 0));

  putFile8Bit(file, (level->block_last_field ? 1 : 0));
  putFile8Bit(file, (level->sp_block_last_field ? 1 : 0));
  putFile32BitBE(file, level->can_move_into_acid_bits);
  putFile8Bit(file, level->dont_collide_with_bits);

  putFile8Bit(file, (level->use_spring_bug ? 1 : 0));
  putFile8Bit(file, (level->use_step_counter ? 1 : 0));

  putFile8Bit(file, (level->instant_relocation ? 1 : 0));
  putFile8Bit(file, (level->can_pass_to_walkable ? 1 : 0));
  putFile8Bit(file, (level->grow_into_diggable ? 1 : 0));

  putFile8Bit(file, level->game_engine_type);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_HEAD_UNUSED);
}
#endif

static int SaveLevel_NAME(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    chunk_size += putFile8Bit(file, level->name[i]);

  return chunk_size;
}

static int SaveLevel_AUTH(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    chunk_size += putFile8Bit(file, level->author[i]);

  return chunk_size;
}

#if ENABLE_HISTORIC_CHUNKS
static int SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int x, y;

  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      if (level->encoding_16bit_field)
	chunk_size += putFile16BitBE(file, level->field[x][y]);
      else
	chunk_size += putFile8Bit(file, level->field[x][y]);

  return chunk_size;
}
#endif

static int SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int x, y;

  for (y = 0; y < level->fieldy; y++) 
    for (x = 0; x < level->fieldx; x++) 
      chunk_size += putFile16BitBE(file, level->field[x][y]);

  return chunk_size;
}

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CONT(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  putFile8Bit(file, EL_YAMYAM);
  putFile8Bit(file, level->num_yamyam_contents);
  putFile8Bit(file, 0);
  putFile8Bit(file, 0);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	if (level->encoding_16bit_field)
	  putFile16BitBE(file, level->yamyam_content[i].e[x][y]);
	else
	  putFile8Bit(file, level->yamyam_content[i].e[x][y]);
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CNT2(FILE *file, struct LevelInfo *level, int element)
{
  int i, x, y;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  if (element == EL_YAMYAM)
  {
    num_contents = level->num_yamyam_contents;
    content_xsize = 3;
    content_ysize = 3;

    for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  content_array[i][x][y] = level->yamyam_content[i].e[x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    num_contents = 1;
    content_xsize = 1;
    content_ysize = 1;

    for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  content_array[i][x][y] = EL_EMPTY;
    content_array[0][0][0] = level->amoeba_content;
  }
  else
  {
    // chunk header already written -- write empty chunk data
    WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_SIZE);

    Warn("cannot save content for element '%d'", element);

    return;
  }

  putFile16BitBE(file, element);
  putFile8Bit(file, num_contents);
  putFile8Bit(file, content_xsize);
  putFile8Bit(file, content_ysize);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile16BitBE(file, content_array[i][x][y]);
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static int SaveLevel_CNT3(FILE *file, struct LevelInfo *level, int element)
{
  int envelope_nr = element - EL_ENVELOPE_1;
  int envelope_len = strlen(level->envelope_text[envelope_nr]) + 1;
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);
  chunk_size += putFile16BitBE(file, envelope_len);
  chunk_size += putFile8Bit(file, level->envelope_xsize[envelope_nr]);
  chunk_size += putFile8Bit(file, level->envelope_ysize[envelope_nr]);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT3_UNUSED);
  chunk_size += LEVEL_CHUNK_CNT3_UNUSED;

  for (i = 0; i < envelope_len; i++)
    chunk_size += putFile8Bit(file, level->envelope_text[envelope_nr][i]);

  return chunk_size;
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS1(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    struct ElementInfo *ei = &element_info[element];

    if (ei->properties[EP_BITFIELD_BASE_NR] != EP_BITMASK_DEFAULT)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);
	putFile32BitBE(file, ei->properties[EP_BITFIELD_BASE_NR]);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	// should not happen
    Warn("inconsistent number of custom element properties");
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS2(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (element_info[element].change->target_element != EL_EMPTY_SPACE)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);
	putFile16BitBE(file, element_info[element].change->target_element);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	// should not happen
    Warn("inconsistent number of custom target elements");
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS3(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, j, x, y, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;
    struct ElementInfo *ei = &element_info[element];

    if (ei->modified_settings)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);

	for (j = 0; j < MAX_ELEMENT_NAME_LEN; j++)
	  putFile8Bit(file, ei->description[j]);

	putFile32BitBE(file, ei->properties[EP_BITFIELD_BASE_NR]);

	// some free bytes for future properties and padding
	WriteUnusedBytesToFile(file, 7);

	putFile8Bit(file, ei->use_gfx_element);
	putFile16BitBE(file, ei->gfx_element_initial);

	putFile8Bit(file, ei->collect_score_initial);
	putFile8Bit(file, ei->collect_count_initial);

	putFile16BitBE(file, ei->push_delay_fixed);
	putFile16BitBE(file, ei->push_delay_random);
	putFile16BitBE(file, ei->move_delay_fixed);
	putFile16BitBE(file, ei->move_delay_random);

	putFile16BitBE(file, ei->move_pattern);
	putFile8Bit(file, ei->move_direction_initial);
	putFile8Bit(file, ei->move_stepsize);

	for (y = 0; y < 3; y++)
	  for (x = 0; x < 3; x++)
	    putFile16BitBE(file, ei->content.e[x][y]);

	putFile32BitBE(file, ei->change->events);

	putFile16BitBE(file, ei->change->target_element);

	putFile16BitBE(file, ei->change->delay_fixed);
	putFile16BitBE(file, ei->change->delay_random);
	putFile16BitBE(file, ei->change->delay_frames);

	putFile16BitBE(file, ei->change->initial_trigger_element);

	putFile8Bit(file, ei->change->explode);
	putFile8Bit(file, ei->change->use_target_content);
	putFile8Bit(file, ei->change->only_if_complete);
	putFile8Bit(file, ei->change->use_random_replace);

	putFile8Bit(file, ei->change->random_percentage);
	putFile8Bit(file, ei->change->replace_when);

	for (y = 0; y < 3; y++)
	  for (x = 0; x < 3; x++)
	    putFile16BitBE(file, ei->change->content.e[x][y]);

	putFile8Bit(file, ei->slippery_type);

	// some free bytes for future properties and padding
	WriteUnusedBytesToFile(file, LEVEL_CPART_CUS3_UNUSED);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	// should not happen
    Warn("inconsistent number of custom element properties");
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS4(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  int i, j, x, y;

  // ---------- custom element base property values (96 bytes) ----------------

  putFile16BitBE(file, element);

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    putFile8Bit(file, ei->description[i]);

  putFile32BitBE(file, ei->properties[EP_BITFIELD_BASE_NR]);

  WriteUnusedBytesToFile(file, 4);	// reserved for more base properties

  putFile8Bit(file, ei->num_change_pages);

  putFile16BitBE(file, ei->ce_value_fixed_initial);
  putFile16BitBE(file, ei->ce_value_random_initial);
  putFile8Bit(file, ei->use_last_ce_value);

  putFile8Bit(file, ei->use_gfx_element);
  putFile16BitBE(file, ei->gfx_element_initial);

  putFile8Bit(file, ei->collect_score_initial);
  putFile8Bit(file, ei->collect_count_initial);

  putFile8Bit(file, ei->drop_delay_fixed);
  putFile8Bit(file, ei->push_delay_fixed);
  putFile8Bit(file, ei->drop_delay_random);
  putFile8Bit(file, ei->push_delay_random);
  putFile16BitBE(file, ei->move_delay_fixed);
  putFile16BitBE(file, ei->move_delay_random);

  // bits 0 - 15 of "move_pattern" ...
  putFile16BitBE(file, ei->move_pattern & 0xffff);
  putFile8Bit(file, ei->move_direction_initial);
  putFile8Bit(file, ei->move_stepsize);

  putFile8Bit(file, ei->slippery_type);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      putFile16BitBE(file, ei->content.e[x][y]);

  putFile16BitBE(file, ei->move_enter_element);
  putFile16BitBE(file, ei->move_leave_element);
  putFile8Bit(file, ei->move_leave_type);

  // ... bits 16 - 31 of "move_pattern" (not nice, but downward compatible)
  putFile16BitBE(file, (ei->move_pattern >> 16) & 0xffff);

  putFile8Bit(file, ei->access_direction);

  putFile8Bit(file, ei->explosion_delay);
  putFile8Bit(file, ei->ignition_delay);
  putFile8Bit(file, ei->explosion_type);

  // some free bytes for future custom property values and padding
  WriteUnusedBytesToFile(file, 1);

  // ---------- change page property values (48 bytes) ------------------------

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];
    unsigned int event_bits;

    // bits 0 - 31 of "has_event[]" ...
    event_bits = 0;
    for (j = 0; j < MIN(NUM_CHANGE_EVENTS, 32); j++)
      if (change->has_event[j])
	event_bits |= (1u << j);
    putFile32BitBE(file, event_bits);

    putFile16BitBE(file, change->target_element);

    putFile16BitBE(file, change->delay_fixed);
    putFile16BitBE(file, change->delay_random);
    putFile16BitBE(file, change->delay_frames);

    putFile16BitBE(file, change->initial_trigger_element);

    putFile8Bit(file, change->explode);
    putFile8Bit(file, change->use_target_content);
    putFile8Bit(file, change->only_if_complete);
    putFile8Bit(file, change->use_random_replace);

    putFile8Bit(file, change->random_percentage);
    putFile8Bit(file, change->replace_when);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile16BitBE(file, change->target_content.e[x][y]);

    putFile8Bit(file, change->can_change);

    putFile8Bit(file, change->trigger_side);

    putFile8Bit(file, change->trigger_player);
    putFile8Bit(file, (change->trigger_page == CH_PAGE_ANY ? CH_PAGE_ANY_FILE :
		       log_2(change->trigger_page)));

    putFile8Bit(file, change->has_action);
    putFile8Bit(file, change->action_type);
    putFile8Bit(file, change->action_mode);
    putFile16BitBE(file, change->action_arg);

    // ... bits 32 - 39 of "has_event[]" (not nice, but downward compatible)
    event_bits = 0;
    for (j = 32; j < NUM_CHANGE_EVENTS; j++)
      if (change->has_event[j])
	event_bits |= (1u << (j - 32));
    putFile8Bit(file, event_bits);
  }
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_GRP1(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;
  int i;

  putFile16BitBE(file, element);

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    putFile8Bit(file, ei->description[i]);

  putFile8Bit(file, group->num_elements);

  putFile8Bit(file, ei->use_gfx_element);
  putFile16BitBE(file, ei->gfx_element_initial);

  putFile8Bit(file, group->choice_mode);

  // some free bytes for future values and padding
  WriteUnusedBytesToFile(file, 3);

  for (i = 0; i < MAX_ELEMENTS_IN_GROUP; i++)
    putFile16BitBE(file, group->element[i]);
}
#endif

static int SaveLevel_MicroChunk(FILE *file, struct LevelFileConfigInfo *entry,
				boolean write_element)
{
  int save_type = entry->save_type;
  int data_type = entry->data_type;
  int conf_type = entry->conf_type;
  int byte_mask = conf_type & CONF_MASK_BYTES;
  int element = entry->element;
  int default_value = entry->default_value;
  int num_bytes = 0;
  boolean modified = FALSE;

  if (byte_mask != CONF_MASK_MULTI_BYTES)
  {
    void *value_ptr = entry->value;
    int value = (data_type == TYPE_BOOLEAN ? *(boolean *)value_ptr :
		 *(int *)value_ptr);

    // check if any settings have been modified before saving them
    if (value != default_value)
      modified = TRUE;

    // do not save if explicitly told or if unmodified default settings
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += (byte_mask == CONF_MASK_1_BYTE ? putFile8Bit   (file, value) :
		  byte_mask == CONF_MASK_2_BYTE ? putFile16BitBE(file, value) :
		  byte_mask == CONF_MASK_4_BYTE ? putFile32BitBE(file, value) :
		  0);
  }
  else if (data_type == TYPE_STRING)
  {
    char *default_string = entry->default_string;
    char *string = (char *)(entry->value);
    int string_length = strlen(string);
    int i;

    // check if any settings have been modified before saving them
    if (!strEqual(string, default_string))
      modified = TRUE;

    // do not save if explicitly told or if unmodified default settings
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += putFile16BitBE(file, string_length);

    for (i = 0; i < string_length; i++)
      num_bytes += putFile8Bit(file, string[i]);
  }
  else if (data_type == TYPE_ELEMENT_LIST)
  {
    int *element_array = (int *)(entry->value);
    int num_elements = *(int *)(entry->num_entities);
    int i;

    // check if any settings have been modified before saving them
    for (i = 0; i < num_elements; i++)
      if (element_array[i] != default_value)
	modified = TRUE;

    // do not save if explicitly told or if unmodified default settings
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += putFile16BitBE(file, num_elements * CONF_ELEMENT_NUM_BYTES);

    for (i = 0; i < num_elements; i++)
      num_bytes += putFile16BitBE(file, element_array[i]);
  }
  else if (data_type == TYPE_CONTENT_LIST)
  {
    struct Content *content = (struct Content *)(entry->value);
    int num_contents = *(int *)(entry->num_entities);
    int i, x, y;

    // check if any settings have been modified before saving them
    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  if (content[i].e[x][y] != default_value)
	    modified = TRUE;

    // do not save if explicitly told or if unmodified default settings
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += putFile16BitBE(file, num_contents * CONF_CONTENT_NUM_BYTES);

    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  num_bytes += putFile16BitBE(file, content[i].e[x][y]);
  }

  return num_bytes;
}

static int SaveLevel_INFO(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  li = *level;		// copy level data into temporary buffer

  for (i = 0; chunk_config_INFO[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_INFO[i], FALSE);

  return chunk_size;
}

static int SaveLevel_ELEM(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  li = *level;		// copy level data into temporary buffer

  for (i = 0; chunk_config_ELEM[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_ELEM[i], TRUE);

  return chunk_size;
}

static int SaveLevel_NOTE(FILE *file, struct LevelInfo *level, int element)
{
  int envelope_nr = element - EL_ENVELOPE_1;
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);

  // copy envelope data into temporary buffer
  xx_envelope = level->envelope[envelope_nr];

  for (i = 0; chunk_config_NOTE[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_NOTE[i], FALSE);

  return chunk_size;
}

static int SaveLevel_CUSX(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  int chunk_size = 0;
  int i, j;

  chunk_size += putFile16BitBE(file, element);

  xx_ei = *ei;		// copy element data into temporary buffer

  // set default description string for this specific element
  strcpy(xx_default_description, getDefaultElementDescription(ei));

  for (i = 0; chunk_config_CUSX_base[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_CUSX_base[i], FALSE);

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];

    xx_current_change_page = i;

    xx_change = *change;	// copy change data into temporary buffer

    resetEventBits();
    setEventBitsFromEventFlags(change);

    for (j = 0; chunk_config_CUSX_change[j].data_type != -1; j++)
      chunk_size += SaveLevel_MicroChunk(file, &chunk_config_CUSX_change[j],
					 FALSE);
  }

  return chunk_size;
}

static int SaveLevel_GRPX(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);

  xx_ei = *ei;		// copy element data into temporary buffer
  xx_group = *group;	// copy group data into temporary buffer

  // set default description string for this specific element
  strcpy(xx_default_description, getDefaultElementDescription(ei));

  for (i = 0; chunk_config_GRPX[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_GRPX[i], FALSE);

  return chunk_size;
}

static int SaveLevel_EMPX(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);

  xx_ei = *ei;		// copy element data into temporary buffer

  for (i = 0; chunk_config_EMPX[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_EMPX[i], FALSE);

  return chunk_size;
}

static void SaveLevelFromFilename(struct LevelInfo *level, char *filename,
				  boolean save_as_template)
{
  int chunk_size;
  int i;
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot save level file '%s'", filename);

    return;
  }

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  level->creation_date = getCurrentDate();

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "CAVE", CHUNK_SIZE_NONE);

  chunk_size = SaveLevel_VERS(NULL, level);
  putFileChunkBE(file, "VERS", chunk_size);
  SaveLevel_VERS(file, level);

  chunk_size = SaveLevel_DATE(NULL, level);
  putFileChunkBE(file, "DATE", chunk_size);
  SaveLevel_DATE(file, level);

  chunk_size = SaveLevel_NAME(NULL, level);
  putFileChunkBE(file, "NAME", chunk_size);
  SaveLevel_NAME(file, level);

  chunk_size = SaveLevel_AUTH(NULL, level);
  putFileChunkBE(file, "AUTH", chunk_size);
  SaveLevel_AUTH(file, level);

  chunk_size = SaveLevel_INFO(NULL, level);
  putFileChunkBE(file, "INFO", chunk_size);
  SaveLevel_INFO(file, level);

  chunk_size = SaveLevel_BODY(NULL, level);
  putFileChunkBE(file, "BODY", chunk_size);
  SaveLevel_BODY(file, level);

  chunk_size = SaveLevel_ELEM(NULL, level);
  if (chunk_size > LEVEL_CHUNK_ELEM_UNCHANGED)		// save if changed
  {
    putFileChunkBE(file, "ELEM", chunk_size);
    SaveLevel_ELEM(file, level);
  }

  for (i = 0; i < NUM_ENVELOPES; i++)
  {
    int element = EL_ENVELOPE_1 + i;

    chunk_size = SaveLevel_NOTE(NULL, level, element);
    if (chunk_size > LEVEL_CHUNK_NOTE_UNCHANGED)	// save if changed
    {
      putFileChunkBE(file, "NOTE", chunk_size);
      SaveLevel_NOTE(file, level, element);
    }
  }

  // if not using template level, check for non-default custom/group elements
  if (!level->use_custom_template || save_as_template)
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      chunk_size = SaveLevel_CUSX(NULL, level, element);
      if (chunk_size > LEVEL_CHUNK_CUSX_UNCHANGED)	// save if changed
      {
	putFileChunkBE(file, "CUSX", chunk_size);
	SaveLevel_CUSX(file, level, element);
      }
    }

    for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
    {
      int element = EL_GROUP_START + i;

      chunk_size = SaveLevel_GRPX(NULL, level, element);
      if (chunk_size > LEVEL_CHUNK_GRPX_UNCHANGED)	// save if changed
      {
	putFileChunkBE(file, "GRPX", chunk_size);
	SaveLevel_GRPX(file, level, element);
      }
    }

    for (i = 0; i < NUM_EMPTY_ELEMENTS_ALL; i++)
    {
      int element = GET_EMPTY_ELEMENT(i);

      chunk_size = SaveLevel_EMPX(NULL, level, element);
      if (chunk_size > LEVEL_CHUNK_EMPX_UNCHANGED)	// save if changed
      {
	putFileChunkBE(file, "EMPX", chunk_size);
	SaveLevel_EMPX(file, level, element);
      }
    }
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveLevel(int nr)
{
  char *filename = getDefaultLevelFilename(nr);

  SaveLevelFromFilename(&level, filename, FALSE);
}

void SaveLevelTemplate(void)
{
  char *filename = getLocalLevelTemplateFilename();

  SaveLevelFromFilename(&level, filename, TRUE);
}

boolean SaveLevelChecked(int nr)
{
  char *filename = getDefaultLevelFilename(nr);
  boolean new_level = !fileExists(filename);
  boolean level_saved = FALSE;

  if (new_level || Request("Save this level and kill the old?", REQ_ASK))
  {
    SaveLevel(nr);

    if (new_level)
      Request("Level saved!", REQ_CONFIRM);

    level_saved = TRUE;
  }

  return level_saved;
}

void DumpLevel(struct LevelInfo *level)
{
  if (level->no_level_file || level->no_valid_file)
  {
    Warn("cannot dump -- no valid level file found");

    return;
  }

  PrintLine("-", 79);
  Print("Level xxx (file version %s, game version %s)\n",
	getVersionString(level->file_version),
	getVersionString(level->game_version));
  PrintLine("-", 79);

  Print("Level author: '%s'\n", level->author);
  Print("Level title:  '%s'\n", level->name);
  Print("\n");
  Print("Playfield size: %d x %d\n", level->fieldx, level->fieldy);
  Print("\n");
  Print("Level time:  %d seconds\n", level->time);
  Print("Gems needed: %d\n", level->gems_needed);
  Print("\n");
  Print("Time for magic wall: %d seconds\n", level->time_magic_wall);
  Print("Time for wheel:      %d seconds\n", level->time_wheel);
  Print("Time for light:      %d seconds\n", level->time_light);
  Print("Time for timegate:   %d seconds\n", level->time_timegate);
  Print("\n");
  Print("Amoeba speed: %d\n", level->amoeba_speed);
  Print("\n");

  Print("EM style slippery gems:      %s\n", (level->em_slippery_gems ? "yes" : "no"));
  Print("Player blocks last field:    %s\n", (level->block_last_field ? "yes" : "no"));
  Print("SP player blocks last field: %s\n", (level->sp_block_last_field ? "yes" : "no"));
  Print("use spring bug: %s\n", (level->use_spring_bug ? "yes" : "no"));
  Print("use step counter: %s\n", (level->use_step_counter ? "yes" : "no"));
  Print("rate time over score: %s\n", (level->rate_time_over_score ? "yes" : "no"));

  if (options.debug)
  {
    int i, j;

    for (i = 0; i < NUM_ENVELOPES; i++)
    {
      char *text = level->envelope[i].text;
      int text_len = strlen(text);
      boolean has_text = FALSE;

      for (j = 0; j < text_len; j++)
	if (text[j] != ' ' && text[j] != '\n')
	  has_text = TRUE;

      if (has_text)
      {
	Print("\n");
	Print("Envelope %d:\n'%s'\n", i + 1, text);
      }
    }
  }

  PrintLine("-", 79);
}

void DumpLevels(void)
{
  static LevelDirTree *dumplevel_leveldir = NULL;

  dumplevel_leveldir = getTreeInfoFromIdentifier(leveldir_first,
						 global.dumplevel_leveldir);

  if (dumplevel_leveldir == NULL)
    Fail("no such level identifier: '%s'", global.dumplevel_leveldir);

  if (global.dumplevel_level_nr < dumplevel_leveldir->first_level ||
      global.dumplevel_level_nr > dumplevel_leveldir->last_level)
    Fail("no such level number: %d", global.dumplevel_level_nr);

  leveldir_current = dumplevel_leveldir;

  LoadLevel(global.dumplevel_level_nr);
  DumpLevel(&level);

  CloseAllAndExit(0);
}

void DumpLevelsetFromFilename_BD(char *filename)
{
  if (leveldir_current == NULL)	// no levelsets loaded yet
    bd_open_all();

  if (!LoadNativeLevel_BD(filename, 0, FALSE))
    CloseAllAndExit(0);		// function has already printed warning

  PrintLine("-", 79);
  Print("Levelset '%s'\n", filename);
  PrintLine("-", 79);

  DumpLevelset_BD();

  PrintLine("-", 79);

  CloseAllAndExit(0);
}

void DumpLevelset(void)
{
  static LevelDirTree *dumplevelset_leveldir = NULL;

  dumplevelset_leveldir = getTreeInfoFromIdentifier(leveldir_first,
                                                    global.dumplevelset_leveldir);
  if (dumplevelset_leveldir == NULL)
    Fail("no such level identifier: '%s'", global.dumplevelset_leveldir);

  PrintLine("-", 79);
  Print("Levelset '%s'\n", dumplevelset_leveldir->identifier);
  PrintLine("-", 79);

  Print("Number of levels:   %d\n", dumplevelset_leveldir->levels);
  Print("First level number: %d\n", dumplevelset_leveldir->first_level);

  PrintLine("-", 79);

  CloseAllAndExit(0);
}


// ============================================================================
// tape file functions
// ============================================================================

static void setTapeInfoToDefaults(void)
{
  int i;

  // always start with reliable default values (empty tape)
  TapeErase();

  // default values (also for pre-1.2 tapes) with only the first player
  tape.player_participates[0] = TRUE;
  for (i = 1; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  // at least one (default: the first) player participates in every tape
  tape.num_participating_players = 1;

  tape.property_bits = TAPE_PROPERTY_NONE;

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;
  tape.solved = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  tape.scr_fieldx = SCR_FIELDX_DEFAULT;
  tape.scr_fieldy = SCR_FIELDY_DEFAULT;

  tape.no_info_chunk = TRUE;
  tape.no_valid_file = FALSE;
}

static int getTapePosSize(struct TapeInfo *tape)
{
  int tape_pos_size = 0;

  if (tape->use_key_actions)
    tape_pos_size += tape->num_participating_players;

  if (tape->use_mouse_actions)
    tape_pos_size += 3;		// x and y position and mouse button mask

  tape_pos_size += 1;		// tape action delay value

  return tape_pos_size;
}

static void setTapeActionFlags(struct TapeInfo *tape, int value)
{
  tape->use_key_actions = FALSE;
  tape->use_mouse_actions = FALSE;

  if (value != TAPE_USE_MOUSE_ACTIONS_ONLY)
    tape->use_key_actions = TRUE;

  if (value != TAPE_USE_KEY_ACTIONS_ONLY)
    tape->use_mouse_actions = TRUE;
}

static int getTapeActionValue(struct TapeInfo *tape)
{
  return (tape->use_key_actions &&
	  tape->use_mouse_actions ? TAPE_USE_KEY_AND_MOUSE_ACTIONS :
	  tape->use_key_actions   ? TAPE_USE_KEY_ACTIONS_ONLY :
	  tape->use_mouse_actions ? TAPE_USE_MOUSE_ACTIONS_ONLY :
	  TAPE_ACTIONS_DEFAULT);
}

static int LoadTape_VERS(File *file, int chunk_size, struct TapeInfo *tape)
{
  tape->file_version = getFileVersion(file);
  tape->game_version = getFileVersion(file);

  tape->game_version_full = tape->game_version;

  return chunk_size;
}

static int LoadTape_HEAD(File *file, int chunk_size, struct TapeInfo *tape)
{
  int i;

  tape->random_seed = getFile32BitBE(file);
  tape->date        = getFile32BitBE(file);
  tape->length      = getFile32BitBE(file);

  // read header fields that are new since version 1.2
  if (tape->file_version >= FILE_VERSION_1_2)
  {
    byte store_participating_players = getFile8Bit(file);
    VersionType engine_version;

    // since version 1.2, tapes store which players participate in the tape
    tape->num_participating_players = 0;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      tape->player_participates[i] = FALSE;

      if (store_participating_players & (1 << i))
      {
	tape->player_participates[i] = TRUE;
	tape->num_participating_players++;
      }
    }

    setTapeActionFlags(tape, getFile8Bit(file));

    tape->property_bits = getFile16BitBE(file);

    // set flag for marking if this tape solves the level or not
    tape->solved = (tape->property_bits & TAPE_PROPERTY_LEVEL_SOLVED) != 0;

    engine_version = getFileVersion(file);
    if (engine_version > 0)
      tape->engine_version = engine_version;
    else
      tape->engine_version = tape->game_version;

    tape->engine_version_full = tape->engine_version;
  }

  return chunk_size;
}

static int LoadTape_VERX(File *file, int chunk_size, struct TapeInfo *tape)
{
  tape->game_version_full   = tape->game_version   | getFileVersionExtended(file);
  tape->engine_version_full = tape->engine_version | getFileVersionExtended(file);

  return chunk_size;
}

static int LoadTape_SCRN(File *file, int chunk_size, struct TapeInfo *tape)
{
  tape->scr_fieldx = getFile8Bit(file);
  tape->scr_fieldy = getFile8Bit(file);

  return chunk_size;
}

static int LoadTape_INFO(File *file, int chunk_size, struct TapeInfo *tape)
{
  char *level_identifier = NULL;
  int level_identifier_size;
  int i;

  tape->no_info_chunk = FALSE;

  level_identifier_size = getFile16BitBE(file);

  level_identifier = checked_malloc(level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    level_identifier[i] = getFile8Bit(file);

  strncpy(tape->level_identifier, level_identifier, MAX_FILENAME_LEN);
  tape->level_identifier[MAX_FILENAME_LEN] = '\0';

  checked_free(level_identifier);

  tape->level_nr = getFile16BitBE(file);

  chunk_size = 2 + level_identifier_size + 2;

  return chunk_size;
}

static int LoadTape_BODY(File *file, int chunk_size, struct TapeInfo *tape)
{
  int i, j;
  int tape_pos_size = getTapePosSize(tape);
  int chunk_size_expected = tape_pos_size * tape->length;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  for (i = 0; i < tape->length; i++)
  {
    if (i >= MAX_TAPE_LEN)
    {
      Warn("tape truncated -- size exceeds maximum tape size %d",
	    MAX_TAPE_LEN);

      // tape too large; read and ignore remaining tape data from this chunk
      for (;i < tape->length; i++)
	ReadUnusedBytesFromFile(file, tape_pos_size);

      break;
    }

    if (tape->use_key_actions)
    {
      for (j = 0; j < MAX_PLAYERS; j++)
      {
	tape->pos[i].action[j] = MV_NONE;

	if (tape->player_participates[j])
	  tape->pos[i].action[j] = getFile8Bit(file);
      }
    }

    if (tape->use_mouse_actions)
    {
      tape->pos[i].action[TAPE_ACTION_LX]     = getFile8Bit(file);
      tape->pos[i].action[TAPE_ACTION_LY]     = getFile8Bit(file);
      tape->pos[i].action[TAPE_ACTION_BUTTON] = getFile8Bit(file);
    }

    tape->pos[i].delay = getFile8Bit(file);

    if (tape->file_version == FILE_VERSION_1_0)
    {
      // eliminate possible diagonal moves in old tapes
      // this is only for backward compatibility

      byte joy_dir[4] = { JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
      byte action = tape->pos[i].action[0];
      int k, num_moves = 0;

      for (k = 0; k < 4; k++)
      {
	if (action & joy_dir[k])
	{
	  tape->pos[i + num_moves].action[0] = joy_dir[k];
	  if (num_moves > 0)
	    tape->pos[i + num_moves].delay = 0;
	  num_moves++;
	}
      }

      if (num_moves > 1)
      {
	num_moves--;
	i += num_moves;
	tape->length += num_moves;
      }
    }
    else if (tape->file_version < FILE_VERSION_2_0)
    {
      // convert pre-2.0 tapes to new tape format

      if (tape->pos[i].delay > 1)
      {
	// action part
	tape->pos[i + 1] = tape->pos[i];
	tape->pos[i + 1].delay = 1;

	// delay part
	for (j = 0; j < MAX_PLAYERS; j++)
	  tape->pos[i].action[j] = MV_NONE;
	tape->pos[i].delay--;

	i++;
	tape->length++;
      }
    }

    if (checkEndOfFile(file))
      break;
  }

  if (i != tape->length)
    chunk_size = tape_pos_size * i;

  return chunk_size;
}

static void LoadTape_SokobanSolution(char *filename)
{
  File *file;
  int move_delay = TILESIZE / level.initial_player_stepsize[0];

  if (!(file = openFile(filename, MODE_READ)))
  {
    tape.no_valid_file = TRUE;

    return;
  }

  while (!checkEndOfFile(file))
  {
    unsigned char c = getByteFromFile(file);

    if (checkEndOfFile(file))
      break;

    switch (c)
    {
      case 'u':
      case 'U':
	tape.pos[tape.length].action[0] = MV_UP;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case 'd':
      case 'D':
	tape.pos[tape.length].action[0] = MV_DOWN;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case 'l':
      case 'L':
	tape.pos[tape.length].action[0] = MV_LEFT;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case 'r':
      case 'R':
	tape.pos[tape.length].action[0] = MV_RIGHT;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case '\n':
      case '\r':
      case '\t':
      case ' ':
	// ignore white-space characters
	break;

      default:
	tape.no_valid_file = TRUE;

	Warn("unsupported Sokoban solution file '%s' ['%d']", filename, c);

	break;
    }
  }

  closeFile(file);

  if (tape.no_valid_file)
    return;

  tape.length_frames  = GetTapeLengthFrames();
  tape.length_seconds = GetTapeLengthSeconds();
}

void LoadTapeFromFilename(char *filename)
{
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  File *file;
  int chunk_size;

  // always start with reliable default values
  setTapeInfoToDefaults();

  if (strSuffix(filename, ".sln"))
  {
    LoadTape_SokobanSolution(filename);

    return;
  }

  if (!(file = openFile(filename, MODE_READ)))
  {
    tape.no_valid_file = TRUE;

    return;
  }

  getFileChunkBE(file, chunk_name, NULL);
  if (strEqual(chunk_name, "RND1"))
  {
    getFile32BitBE(file);		// not used

    getFileChunkBE(file, chunk_name, NULL);
    if (!strEqual(chunk_name, "TAPE"))
    {
      tape.no_valid_file = TRUE;

      Warn("unknown format of tape file '%s'", filename);

      closeFile(file);

      return;
    }
  }
  else	// check for pre-2.0 file format with cookie string
  {
    strcpy(cookie, chunk_name);
    if (getStringFromFile(file, &cookie[4], MAX_LINE_LEN - 4) == NULL)
      cookie[4] = '\0';
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, TAPE_COOKIE_TMPL))
    {
      tape.no_valid_file = TRUE;

      Warn("unknown format of tape file '%s'", filename);

      closeFile(file);

      return;
    }

    if ((tape.file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      tape.no_valid_file = TRUE;

      Warn("unsupported version of tape file '%s'", filename);

      closeFile(file);

      return;
    }

    // pre-2.0 tape files have no game version, so use file version here
    tape.game_version      = tape.file_version;
    tape.game_version_full = tape.file_version;
  }

  if (tape.file_version < FILE_VERSION_1_2)
  {
    // tape files from versions before 1.2.0 without chunk structure
    LoadTape_HEAD(file, TAPE_CHUNK_HEAD_SIZE, &tape);
    LoadTape_BODY(file, 2 * tape.length,      &tape);
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(File *, int, struct TapeInfo *);
    }
    chunk_info[] =
    {
      { "VERS", TAPE_CHUNK_VERS_SIZE,	LoadTape_VERS },
      { "HEAD", TAPE_CHUNK_HEAD_SIZE,	LoadTape_HEAD },
      { "VERX", TAPE_CHUNK_VERX_SIZE,	LoadTape_VERX },
      { "SCRN", TAPE_CHUNK_SCRN_SIZE,	LoadTape_SCRN },
      { "INFO", -1,			LoadTape_INFO },
      { "BODY", -1,			LoadTape_BODY },
      {  NULL,  0,			NULL }
    };

    while (getFileChunkBE(file, chunk_name, &chunk_size))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     !strEqual(chunk_name, chunk_info[i].name))
	i++;

      if (chunk_info[i].name == NULL)
      {
	Warn("unknown chunk '%s' in tape file '%s'",
	      chunk_name, filename);

	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Warn("wrong size (%d) of chunk '%s' in tape file '%s'",
	      chunk_size, chunk_name, filename);

	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	// call function to load this tape chunk
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, &tape);

	// the size of some chunks cannot be checked before reading other
	// chunks first (like "HEAD" and "BODY") that contain some header
	// information, so check them here
	if (chunk_size_expected != chunk_size)
	{
	  Warn("wrong size (%d) of chunk '%s' in tape file '%s'",
		chunk_size, chunk_name, filename);
	}
      }
    }
  }

  closeFile(file);

  tape.length_frames  = GetTapeLengthFrames();
  tape.length_seconds = GetTapeLengthSeconds();

#if 0
  Debug("files:LoadTapeFromFilename", "tape file version:   %s",
        getVersionString(tape.file_version));
  Debug("files:LoadTapeFromFilename", "tape game version:   %s",
	getVersionString(tape.game_version_full));
  Debug("files:LoadTapeFromFilename", "tape engine version: %s",
	getVersionString(tape.engine_version_full));
#endif
}

void LoadTape(int nr)
{
  char *filename = getTapeFilename(nr);

  LoadTapeFromFilename(filename);
}

void LoadSolutionTape(int nr)
{
  char *filename = getSolutionTapeFilename(nr);

  LoadTapeFromFilename(filename);

  if (TAPE_IS_EMPTY(tape))
  {
    if (level.game_engine_type == GAME_ENGINE_TYPE_BD &&
	level.native_bd_level->replay != NULL)
      CopyNativeTape_BD_to_RND(&level);
    else if (level.game_engine_type == GAME_ENGINE_TYPE_SP &&
	level.native_sp_level->demo.is_available)
      CopyNativeTape_SP_to_RND(&level);
  }
}

void LoadScoreTape(char *score_tape_basename, int nr)
{
  char *filename = getScoreTapeFilename(score_tape_basename, nr);

  LoadTapeFromFilename(filename);
}

void LoadScoreCacheTape(char *score_tape_basename, int nr)
{
  char *filename = getScoreCacheTapeFilename(score_tape_basename, nr);

  LoadTapeFromFilename(filename);
}

static boolean checkSaveTape_SCRN(struct TapeInfo *tape)
{
  // chunk required for team mode tapes with non-default screen size
  return (tape->num_participating_players > 1 &&
	  (tape->scr_fieldx != SCR_FIELDX_DEFAULT ||
	   tape->scr_fieldy != SCR_FIELDY_DEFAULT));
}

static void SaveTape_VERS(FILE *file, struct TapeInfo *tape)
{
  putFileVersion(file, tape->file_version);
  putFileVersion(file, tape->game_version);
}

static void SaveTape_HEAD(FILE *file, struct TapeInfo *tape)
{
  int i;
  byte store_participating_players = 0;

  // set bits for participating players for compact storage
  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape->player_participates[i])
      store_participating_players |= (1 << i);

  // if this tape solves the level, set corresponding tape property bit
  if (tape->solved)
    tape->property_bits |= TAPE_PROPERTY_LEVEL_SOLVED;

  putFile32BitBE(file, tape->random_seed);
  putFile32BitBE(file, tape->date);
  putFile32BitBE(file, tape->length);

  putFile8Bit(file, store_participating_players);

  putFile8Bit(file, getTapeActionValue(tape));

  putFile16BitBE(file, tape->property_bits);

  putFileVersion(file, tape->engine_version);
}

static void SaveTape_VERX(FILE *file, struct TapeInfo *tape)
{
  putFileVersionExtended(file, tape->game_version_full);
  putFileVersionExtended(file, tape->engine_version_full);
}

static void SaveTape_SCRN(FILE *file, struct TapeInfo *tape)
{
  putFile8Bit(file, tape->scr_fieldx);
  putFile8Bit(file, tape->scr_fieldy);
}

static void SaveTape_INFO(FILE *file, struct TapeInfo *tape)
{
  int level_identifier_size = strlen(tape->level_identifier) + 1;
  int i;

  putFile16BitBE(file, level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    putFile8Bit(file, tape->level_identifier[i]);

  putFile16BitBE(file, tape->level_nr);
}

static void SaveTape_BODY(FILE *file, struct TapeInfo *tape)
{
  int i, j;

  for (i = 0; i < tape->length; i++)
  {
    if (tape->use_key_actions)
    {
      for (j = 0; j < MAX_PLAYERS; j++)
	if (tape->player_participates[j])
	  putFile8Bit(file, tape->pos[i].action[j]);
    }

    if (tape->use_mouse_actions)
    {
      putFile8Bit(file, tape->pos[i].action[TAPE_ACTION_LX]);
      putFile8Bit(file, tape->pos[i].action[TAPE_ACTION_LY]);
      putFile8Bit(file, tape->pos[i].action[TAPE_ACTION_BUTTON]);
    }

    putFile8Bit(file, tape->pos[i].delay);
  }
}

void SaveTapeToFilename(char *filename)
{
  FILE *file;
  int tape_pos_size;
  int info_chunk_size;
  int body_chunk_size;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot save level recording file '%s'", filename);

    return;
  }

  tape_pos_size = getTapePosSize(&tape);

  info_chunk_size = 2 + (strlen(tape.level_identifier) + 1) + 2;
  body_chunk_size = tape_pos_size * tape.length;

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "TAPE", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", TAPE_CHUNK_VERS_SIZE);
  SaveTape_VERS(file, &tape);

  putFileChunkBE(file, "HEAD", TAPE_CHUNK_HEAD_SIZE);
  SaveTape_HEAD(file, &tape);

  putFileChunkBE(file, "VERX", TAPE_CHUNK_VERX_SIZE);
  SaveTape_VERX(file, &tape);

  if (checkSaveTape_SCRN(&tape))
  {
    putFileChunkBE(file, "SCRN", TAPE_CHUNK_SCRN_SIZE);
    SaveTape_SCRN(file, &tape);
  }

  putFileChunkBE(file, "INFO", info_chunk_size);
  SaveTape_INFO(file, &tape);

  putFileChunkBE(file, "BODY", body_chunk_size);
  SaveTape_BODY(file, &tape);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

static void SaveTapeExt(char *filename)
{
  int i;

  tape.file_version      = FILE_VERSION_ACTUAL;
  tape.game_version      = GAME_VERSION_ACTUAL;
  tape.game_version_full = GAME_VERSION_ACTUAL_FULL;

  tape.num_participating_players = 0;

  // count number of participating players
  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape.player_participates[i])
      tape.num_participating_players++;

  SaveTapeToFilename(filename);

  tape.changed = FALSE;
}

void SaveTape(int nr)
{
  char *filename = getTapeFilename(nr);

  InitTapeDirectory(leveldir_current->subdir);

  SaveTapeExt(filename);
}

void SaveScoreTape(int nr)
{
  char *filename = getScoreTapeFilename(tape.score_tape_basename, nr);

  // used instead of "leveldir_current->subdir" (for network games)
  InitScoreTapeDirectory(levelset.identifier, nr);

  SaveTapeExt(filename);
}

static boolean SaveTapeRequest(char *msg, unsigned int req_state)
{
  return (!setup.ask_on_save_tape || Request(msg, req_state));
}

static boolean SaveTapeCheckedExt(int nr, char *msg_replace, char *msg_saved,
				  unsigned int req_state_added)
{
  char *filename = getTapeFilename(nr);
  boolean new_tape = !fileExists(filename);
  boolean tape_saved = FALSE;

  if (new_tape || SaveTapeRequest(msg_replace, REQ_ASK | req_state_added))
  {
    SaveTape(nr);

    if (new_tape)
      SaveTapeRequest(msg_saved, REQ_CONFIRM | req_state_added);

    tape_saved = TRUE;
  }

  return tape_saved;
}

boolean SaveTapeChecked(int nr)
{
  return SaveTapeCheckedExt(nr, "Replace old tape?", "Tape saved!", 0);
}

boolean SaveTapeChecked_LevelSolved(int nr)
{
  return SaveTapeCheckedExt(nr, "Level solved! Replace old tape?",
			        "Level solved! Tape saved!", REQ_STAY_OPEN);
}

void DumpTape(struct TapeInfo *tape)
{
  int tape_frame_counter;
  int i, j;

  if (tape->no_valid_file)
  {
    Warn("cannot dump -- no valid tape file found");

    return;
  }

  int year2 = tape->date / 10000;
  int year4 = (year2 < 70 ? 2000 + year2 : 1900 + year2);
  int month_index_raw = (tape->date / 100) % 100;
  int month_index = month_index_raw % 12;	// prevent invalid index
  int month = month_index + 1;
  int day = tape->date % 100;

  PrintLine("-", 79);

  Print("Tape of level set '%s', level %03d\n", tape->level_identifier, tape->level_nr);
  Print("- tape date: %04d-%02d-%02d\n", year4, month, day);
  Print("- file version:   %s\n", getVersionString(tape->file_version));
  Print("- game version:   %s\n", getVersionString(tape->game_version_full));
  Print("- engine version: %s\n", getVersionString(tape->engine_version_full));

  Print("- solution tape: %s\n",
	tape->solved ? "yes" : tape->game_version < VERSION_IDENT(4,3,2,3) ? "unknown" : "no");

  Print("- special tape properties: ");
  if (tape->property_bits == TAPE_PROPERTY_NONE)
    Print("[none]");
  if (tape->property_bits & TAPE_PROPERTY_BD_OLD_ENGINE)
    Print("[bd_old_engine]");
  if (tape->property_bits & TAPE_PROPERTY_EM_RANDOM_BUG)
    Print("[em_random_bug]");
  if (tape->property_bits & TAPE_PROPERTY_GAME_SPEED)
    Print("[game_speed]");
  if (tape->property_bits & TAPE_PROPERTY_PAUSE_MODE)
    Print("[pause]");
  if (tape->property_bits & TAPE_PROPERTY_SINGLE_STEP)
    Print("[single_step]");
  if (tape->property_bits & TAPE_PROPERTY_SNAPSHOT)
    Print("[snapshot]");
  if (tape->property_bits & TAPE_PROPERTY_REPLAYED)
    Print("[replayed]");
  if (tape->property_bits & TAPE_PROPERTY_TAS_KEYS)
    Print("[tas_keys]");
  if (tape->property_bits & TAPE_PROPERTY_SMALL_GRAPHICS)
    Print("[small_graphics]");
  Print("\n");

  PrintLine("-", 79);

  tape_frame_counter = 0;

  for (i = 0; i < tape->length; i++)
  {
    if (i >= MAX_TAPE_LEN)
      break;

    Print("%04d: ", i);

    for (j = 0; j < MAX_PLAYERS; j++)
    {
      if (tape->player_participates[j])
      {
	int action = tape->pos[i].action[j];

	Print("%d:%02x ", j, action);
	Print("[%c%c%c%c|%c%c] - ",
	      (action & JOY_LEFT ? '<' : ' '),
	      (action & JOY_RIGHT ? '>' : ' '),
	      (action & JOY_UP ? '^' : ' '),
	      (action & JOY_DOWN ? 'v' : ' '),
	      (action & JOY_BUTTON_1 ? '1' : ' '),
	      (action & JOY_BUTTON_2 ? '2' : ' '));
      }
    }

    Print("(%03d) ", tape->pos[i].delay);
    Print("[%05d]\n", tape_frame_counter);

    tape_frame_counter += tape->pos[i].delay;
  }

  PrintLine("-", 79);
}

void DumpTapes(void)
{
  static LevelDirTree *dumptape_leveldir = NULL;

  dumptape_leveldir = getTreeInfoFromIdentifier(leveldir_first,
						global.dumptape_leveldir);

  if (dumptape_leveldir == NULL)
    Fail("no such level identifier: '%s'", global.dumptape_leveldir);

  if (global.dumptape_level_nr < dumptape_leveldir->first_level ||
      global.dumptape_level_nr > dumptape_leveldir->last_level)
    Fail("no such level number: %d", global.dumptape_level_nr);

  leveldir_current = dumptape_leveldir;

  if (options.mytapes)
    LoadTape(global.dumptape_level_nr);
  else
    LoadSolutionTape(global.dumptape_level_nr);

  DumpTape(&tape);

  CloseAllAndExit(0);
}


// ============================================================================
// score file functions
// ============================================================================

static void setScoreInfoToDefaultsExt(struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    strcpy(scores->entry[i].tape_basename, UNDEFINED_FILENAME);
    strcpy(scores->entry[i].name, EMPTY_PLAYER_NAME);
    scores->entry[i].score = 0;
    scores->entry[i].time = 0;

    scores->entry[i].id = -1;
    strcpy(scores->entry[i].tape_date,    UNKNOWN_NAME);
    strcpy(scores->entry[i].platform,     UNKNOWN_NAME);
    strcpy(scores->entry[i].version,      UNKNOWN_NAME);
    strcpy(scores->entry[i].country_name, UNKNOWN_NAME);
    strcpy(scores->entry[i].country_code, "??");
  }

  scores->num_entries = 0;
  scores->last_added = -1;
  scores->last_added_local = -1;

  scores->updated = FALSE;
  scores->uploaded = FALSE;
  scores->tape_downloaded = FALSE;
  scores->force_last_added = FALSE;

  // The following values are intentionally not reset here:
  // - last_level_nr
  // - last_entry_nr
  // - next_level_nr
  // - continue_playing
  // - continue_on_return
}

static void setScoreInfoToDefaults(void)
{
  setScoreInfoToDefaultsExt(&scores);
}

static void setServerScoreInfoToDefaults(void)
{
  setScoreInfoToDefaultsExt(&server_scores);
}

static void LoadScore_OLD(int nr)
{
  int i;
  char *filename = getScoreFilename(nr);
  char cookie[MAX_LINE_LEN];
  char line[MAX_LINE_LEN];
  char *line_ptr;
  FILE *file;

  if (!(file = fopen(filename, MODE_READ)))
    return;

  // check file identifier
  if (fgets(cookie, MAX_LINE_LEN, file) == NULL)
    cookie[0] = '\0';
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (!checkCookieString(cookie, SCORE_COOKIE_TMPL))
  {
    Warn("unknown format of score file '%s'", filename);

    fclose(file);

    return;
  }

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    if (fscanf(file, "%d", &scores.entry[i].score) == EOF)
      Warn("fscanf() failed; %s", strerror(errno));

    if (fgets(line, MAX_LINE_LEN, file) == NULL)
      line[0] = '\0';

    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';

    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr != ' ' && *line_ptr != '\t' && *line_ptr != '\0')
      {
	strncpy(scores.entry[i].name, line_ptr, MAX_PLAYER_NAME_LEN);
	scores.entry[i].name[MAX_PLAYER_NAME_LEN] = '\0';
	break;
      }
    }
  }

  fclose(file);
}

static void ConvertScore_OLD(void)
{
  // only convert score to time for levels that rate playing time over score
  if (!level.rate_time_over_score)
    return;

  // convert old score to playing time for score-less levels (like Supaplex)
  int time_final_max = 999;
  int i;

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    int score = scores.entry[i].score;

    if (score > 0 && score < time_final_max)
      scores.entry[i].time = (time_final_max - score - 1) * FRAMES_PER_SECOND;
  }
}

static int LoadScore_VERS(File *file, int chunk_size, struct ScoreInfo *scores)
{
  scores->file_version = getFileVersion(file);
  scores->game_version = getFileVersion(file);

  return chunk_size;
}

static int LoadScore_INFO(File *file, int chunk_size, struct ScoreInfo *scores)
{
  char *level_identifier = NULL;
  int level_identifier_size;
  int i;

  level_identifier_size = getFile16BitBE(file);

  level_identifier = checked_malloc(level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    level_identifier[i] = getFile8Bit(file);

  strncpy(scores->level_identifier, level_identifier, MAX_FILENAME_LEN);
  scores->level_identifier[MAX_FILENAME_LEN] = '\0';

  checked_free(level_identifier);

  scores->level_nr = getFile16BitBE(file);
  scores->num_entries = getFile16BitBE(file);

  chunk_size = 2 + level_identifier_size + 2 + 2;

  return chunk_size;
}

static int LoadScore_NAME(File *file, int chunk_size, struct ScoreInfo *scores)
{
  int i, j;

  for (i = 0; i < scores->num_entries; i++)
  {
    for (j = 0; j < MAX_PLAYER_NAME_LEN; j++)
      scores->entry[i].name[j] = getFile8Bit(file);

    scores->entry[i].name[MAX_PLAYER_NAME_LEN] = '\0';
  }

  chunk_size = scores->num_entries * MAX_PLAYER_NAME_LEN;

  return chunk_size;
}

static int LoadScore_SCOR(File *file, int chunk_size, struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < scores->num_entries; i++)
    scores->entry[i].score = getFile16BitBE(file);

  chunk_size = scores->num_entries * 2;

  return chunk_size;
}

static int LoadScore_SC4R(File *file, int chunk_size, struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < scores->num_entries; i++)
    scores->entry[i].score = getFile32BitBE(file);

  chunk_size = scores->num_entries * 4;

  return chunk_size;
}

static int LoadScore_TIME(File *file, int chunk_size, struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < scores->num_entries; i++)
    scores->entry[i].time = getFile32BitBE(file);

  chunk_size = scores->num_entries * 4;

  return chunk_size;
}

static int LoadScore_TAPE(File *file, int chunk_size, struct ScoreInfo *scores)
{
  int i, j;

  for (i = 0; i < scores->num_entries; i++)
  {
    for (j = 0; j < MAX_SCORE_TAPE_BASENAME_LEN; j++)
      scores->entry[i].tape_basename[j] = getFile8Bit(file);

    scores->entry[i].tape_basename[MAX_SCORE_TAPE_BASENAME_LEN] = '\0';
  }

  chunk_size = scores->num_entries * MAX_SCORE_TAPE_BASENAME_LEN;

  return chunk_size;
}

void LoadScore(int nr)
{
  char *filename = getScoreFilename(nr);
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  boolean old_score_file_format = FALSE;
  File *file;

  // always start with reliable default values
  setScoreInfoToDefaults();

  if (!(file = openFile(filename, MODE_READ)))
    return;

  getFileChunkBE(file, chunk_name, NULL);
  if (strEqual(chunk_name, "RND1"))
  {
    getFile32BitBE(file);		// not used

    getFileChunkBE(file, chunk_name, NULL);
    if (!strEqual(chunk_name, "SCOR"))
    {
      Warn("unknown format of score file '%s'", filename);

      closeFile(file);

      return;
    }
  }
  else	// check for old file format with cookie string
  {
    strcpy(cookie, chunk_name);
    if (getStringFromFile(file, &cookie[4], MAX_LINE_LEN - 4) == NULL)
      cookie[4] = '\0';
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, SCORE_COOKIE_TMPL))
    {
      Warn("unknown format of score file '%s'", filename);

      closeFile(file);

      return;
    }

    old_score_file_format = TRUE;
  }

  if (old_score_file_format)
  {
    // score files from versions before 4.2.4.0 without chunk structure
    LoadScore_OLD(nr);

    // convert score to time, if possible (mainly for Supaplex levels)
    ConvertScore_OLD();
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(File *, int, struct ScoreInfo *);
    }
    chunk_info[] =
    {
      { "VERS", SCORE_CHUNK_VERS_SIZE,	LoadScore_VERS },
      { "INFO", -1,			LoadScore_INFO },
      { "NAME", -1,			LoadScore_NAME },
      { "SCOR", -1,			LoadScore_SCOR },
      { "SC4R", -1,			LoadScore_SC4R },
      { "TIME", -1,			LoadScore_TIME },
      { "TAPE", -1,			LoadScore_TAPE },

      {  NULL,  0,			NULL }
    };

    while (getFileChunkBE(file, chunk_name, &chunk_size))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     !strEqual(chunk_name, chunk_info[i].name))
	i++;

      if (chunk_info[i].name == NULL)
      {
	Warn("unknown chunk '%s' in score file '%s'",
	      chunk_name, filename);

	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Warn("wrong size (%d) of chunk '%s' in score file '%s'",
	      chunk_size, chunk_name, filename);

	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	// call function to load this score chunk
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, &scores);

	// the size of some chunks cannot be checked before reading other
	// chunks first (like "HEAD" and "BODY") that contain some header
	// information, so check them here
	if (chunk_size_expected != chunk_size)
	{
	  Warn("wrong size (%d) of chunk '%s' in score file '%s'",
		chunk_size, chunk_name, filename);
	}
      }
    }
  }

  closeFile(file);
}

#if ENABLE_HISTORIC_CHUNKS
void SaveScore_OLD(int nr)
{
  int i;
  char *filename = getScoreFilename(nr);
  FILE *file;

  // used instead of "leveldir_current->subdir" (for network games)
  InitScoreDirectory(levelset.identifier);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot save score for level %d", nr);

    return;
  }

  fprintf(file, "%s\n\n", SCORE_COOKIE);

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
    fprintf(file, "%d %s\n", scores.entry[i].score, scores.entry[i].name);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}
#endif

static void SaveScore_VERS(FILE *file, struct ScoreInfo *scores)
{
  putFileVersion(file, scores->file_version);
  putFileVersion(file, scores->game_version);
}

static void SaveScore_INFO(FILE *file, struct ScoreInfo *scores)
{
  int level_identifier_size = strlen(scores->level_identifier) + 1;
  int i;

  putFile16BitBE(file, level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    putFile8Bit(file, scores->level_identifier[i]);

  putFile16BitBE(file, scores->level_nr);
  putFile16BitBE(file, scores->num_entries);
}

static void SaveScore_NAME(FILE *file, struct ScoreInfo *scores)
{
  int i, j;

  for (i = 0; i < scores->num_entries; i++)
  {
    int name_size = strlen(scores->entry[i].name);

    for (j = 0; j < MAX_PLAYER_NAME_LEN; j++)
      putFile8Bit(file, (j < name_size ? scores->entry[i].name[j] : 0));
  }
}

static void SaveScore_SCOR(FILE *file, struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < scores->num_entries; i++)
    putFile16BitBE(file, scores->entry[i].score);
}

static void SaveScore_SC4R(FILE *file, struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < scores->num_entries; i++)
    putFile32BitBE(file, scores->entry[i].score);
}

static void SaveScore_TIME(FILE *file, struct ScoreInfo *scores)
{
  int i;

  for (i = 0; i < scores->num_entries; i++)
    putFile32BitBE(file, scores->entry[i].time);
}

static void SaveScore_TAPE(FILE *file, struct ScoreInfo *scores)
{
  int i, j;

  for (i = 0; i < scores->num_entries; i++)
  {
    int size = strlen(scores->entry[i].tape_basename);

    for (j = 0; j < MAX_SCORE_TAPE_BASENAME_LEN; j++)
      putFile8Bit(file, (j < size ? scores->entry[i].tape_basename[j] : 0));
  }
}

static void SaveScoreToFilename(char *filename)
{
  FILE *file;
  int info_chunk_size;
  int name_chunk_size;
  int scor_chunk_size;
  int sc4r_chunk_size;
  int time_chunk_size;
  int tape_chunk_size;
  boolean has_large_score_values;
  int i;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot save score file '%s'", filename);

    return;
  }

  info_chunk_size = 2 + (strlen(scores.level_identifier) + 1) + 2 + 2;
  name_chunk_size = scores.num_entries * MAX_PLAYER_NAME_LEN;
  scor_chunk_size = scores.num_entries * 2;
  sc4r_chunk_size = scores.num_entries * 4;
  time_chunk_size = scores.num_entries * 4;
  tape_chunk_size = scores.num_entries * MAX_SCORE_TAPE_BASENAME_LEN;

  has_large_score_values = FALSE;
  for (i = 0; i < scores.num_entries; i++)
    if (scores.entry[i].score > 0xffff)
      has_large_score_values = TRUE;

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "SCOR", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", SCORE_CHUNK_VERS_SIZE);
  SaveScore_VERS(file, &scores);

  putFileChunkBE(file, "INFO", info_chunk_size);
  SaveScore_INFO(file, &scores);

  putFileChunkBE(file, "NAME", name_chunk_size);
  SaveScore_NAME(file, &scores);

  if (has_large_score_values)
  {
    putFileChunkBE(file, "SC4R", sc4r_chunk_size);
    SaveScore_SC4R(file, &scores);
  }
  else
  {
    putFileChunkBE(file, "SCOR", scor_chunk_size);
    SaveScore_SCOR(file, &scores);
  }

  putFileChunkBE(file, "TIME", time_chunk_size);
  SaveScore_TIME(file, &scores);

  putFileChunkBE(file, "TAPE", tape_chunk_size);
  SaveScore_TAPE(file, &scores);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveScore(int nr)
{
  char *filename = getScoreFilename(nr);
  int i;

  // used instead of "leveldir_current->subdir" (for network games)
  InitScoreDirectory(levelset.identifier);

  scores.file_version = FILE_VERSION_ACTUAL;
  scores.game_version = GAME_VERSION_ACTUAL;

  strncpy(scores.level_identifier, levelset.identifier, MAX_FILENAME_LEN);
  scores.level_identifier[MAX_FILENAME_LEN] = '\0';
  scores.level_nr = level_nr;

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
    if (scores.entry[i].score == 0 &&
        scores.entry[i].time == 0 &&
        strEqual(scores.entry[i].name, EMPTY_PLAYER_NAME))
      break;

  scores.num_entries = i;

  if (scores.num_entries == 0)
    return;

  SaveScoreToFilename(filename);
}

static void LoadServerScoreFromCache(int nr)
{
  struct ScoreEntry score_entry;
  struct
  {
    void *value;
    boolean is_string;
    int string_size;
  }
  score_mapping[] =
  {
    { &score_entry.score,		FALSE,	0			},
    { &score_entry.time,		FALSE,	0			},
    { score_entry.name,			TRUE,	MAX_PLAYER_NAME_LEN	},
    { score_entry.tape_basename,	TRUE,	MAX_FILENAME_LEN	},
    { score_entry.tape_date,		TRUE,	MAX_ISO_DATE_LEN	},
    { &score_entry.id,			FALSE,	0			},
    { score_entry.platform,		TRUE,	MAX_PLATFORM_TEXT_LEN	},
    { score_entry.version,		TRUE,	MAX_VERSION_TEXT_LEN	},
    { score_entry.country_code,		TRUE,	MAX_COUNTRY_CODE_LEN	},
    { score_entry.country_name,		TRUE,	MAX_COUNTRY_NAME_LEN	},

    { NULL,				FALSE,	0			}
  };
  char *filename = getScoreCacheFilename(nr);
  SetupFileHash *score_hash = loadSetupFileHash(filename);
  int i, j;

  server_scores.num_entries = 0;

  if (score_hash == NULL)
    return;

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    score_entry = server_scores.entry[i];

    for (j = 0; score_mapping[j].value != NULL; j++)
    {
      char token[10];

      sprintf(token, "%02d.%d", i, j);

      char *value = getHashEntry(score_hash, token);

      if (value == NULL)
	continue;

      if (score_mapping[j].is_string)
      {
	char *score_value = (char *)score_mapping[j].value;
	int value_size = score_mapping[j].string_size;

	strncpy(score_value, value, value_size);
	score_value[value_size] = '\0';
      }
      else
      {
	int *score_value = (int *)score_mapping[j].value;

	*score_value = atoi(value);
      }

      server_scores.num_entries = i + 1;
    }

    server_scores.entry[i] = score_entry;
  }

  freeSetupFileHash(score_hash);
}

void LoadServerScore(int nr, boolean download_score)
{
  if (!setup.use_api_server)
    return;

  // always start with reliable default values
  setServerScoreInfoToDefaults();

  // 1st step: load server scores from cache file (which may not exist)
  // (this should prevent reading it while the thread is writing to it)
  LoadServerScoreFromCache(nr);

  if (download_score && runtime.use_api_server)
  {
    // 2nd step: download server scores from score server to cache file
    // (as thread, as it might time out if the server is not reachable)
    ApiGetScoreAsThread(nr);
  }
}

void PrepareScoreTapesForUpload(char *leveldir_subdir)
{
  MarkTapeDirectoryUploadsAsIncomplete(leveldir_subdir);

  // if score tape not uploaded, ask for uploading missing tapes later
  if (!setup.has_remaining_tapes)
    setup.ask_for_remaining_tapes = TRUE;

  setup.provide_uploading_tapes = TRUE;
  setup.has_remaining_tapes = TRUE;

  SaveSetup_ServerSetup();
}

void SaveServerScore(int nr, boolean tape_saved)
{
  if (!runtime.use_api_server)
  {
    PrepareScoreTapesForUpload(leveldir_current->subdir);

    return;
  }

  ApiAddScoreAsThread(nr, tape_saved, NULL);
}

void SaveServerScoreFromFile(int nr, boolean tape_saved,
			     char *score_tape_filename)
{
  if (!runtime.use_api_server)
    return;

  ApiAddScoreAsThread(nr, tape_saved, score_tape_filename);
}

void LoadLocalAndServerScore(int nr, boolean download_score)
{
  int last_added_local = scores.last_added_local;
  boolean force_last_added = scores.force_last_added;

  // needed if only showing server scores
  setScoreInfoToDefaults();

  if (!strEqual(setup.scores_in_highscore_list, STR_SCORES_TYPE_SERVER_ONLY))
    LoadScore(nr);

  // restore last added local score entry (before merging server scores)
  scores.last_added = scores.last_added_local = last_added_local;

  if (setup.use_api_server &&
      !strEqual(setup.scores_in_highscore_list, STR_SCORES_TYPE_LOCAL_ONLY))
  {
    // load server scores from cache file and trigger update from server
    LoadServerScore(nr, download_score);

    // merge local scores with scores from server
    MergeServerScore();
  }

  if (force_last_added)
    scores.force_last_added = force_last_added;
}


// ============================================================================
// setup file functions
// ============================================================================

#define TOKEN_STR_PLAYER_PREFIX			"player_"


static struct TokenInfo global_setup_tokens[] =
{
  {
    TYPE_STRING,
    &setup.player_name,				"player_name"
  },
  {
    TYPE_SWITCH,
    &setup.multiple_users,			"multiple_users"
  },
  {
    TYPE_SWITCH,
    &setup.sound,				"sound"
  },
  {
    TYPE_SWITCH,
    &setup.sound_loops,				"repeating_sound_loops"
  },
  {
    TYPE_SWITCH,
    &setup.sound_music,				"background_music"
  },
  {
    TYPE_SWITCH,
    &setup.sound_simple,			"simple_sound_effects"
  },
  {
    TYPE_SWITCH,
    &setup.toons,				"toons"
  },
  {
    TYPE_SWITCH,
    &setup.global_animations,			"global_animations"
  },
  {
    TYPE_SWITCH,
    &setup.scroll_delay,			"scroll_delay"
  },
  {
    TYPE_SWITCH,
    &setup.forced_scroll_delay,			"forced_scroll_delay"
  },
  {
    TYPE_INTEGER,
    &setup.scroll_delay_value,			"scroll_delay_value"
  },
  {
    TYPE_STRING,
    &setup.engine_snapshot_mode,		"engine_snapshot_mode"
  },
  {
    TYPE_INTEGER,
    &setup.engine_snapshot_memory,		"engine_snapshot_memory"
  },
  {
    TYPE_SWITCH,
    &setup.fade_screens,			"fade_screens"
  },
  {
    TYPE_SWITCH,
    &setup.autorecord,				"automatic_tape_recording"
  },
  {
    TYPE_SWITCH,
    &setup.autorecord_after_replay,		"autorecord_after_replay"
  },
  {
    TYPE_SWITCH,
    &setup.auto_pause_on_start,			"auto_pause_on_start"
  },
  {
    TYPE_SWITCH,
    &setup.show_titlescreen,			"show_titlescreen"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.show_level_story,			"show_level_story"
  },
  {
    TYPE_SWITCH,
    &setup.quick_doors,				"quick_doors"
  },
  {
    TYPE_SWITCH,
    &setup.team_mode,				"team_mode"
  },
  {
    TYPE_SWITCH,
    &setup.handicap,				"handicap"
  },
  {
    TYPE_SWITCH,
    &setup.skip_levels,				"skip_levels"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.allow_skipping_levels,		"allow_skipping_levels"
  },
  {
    TYPE_SWITCH,
    &setup.increment_levels,			"increment_levels"
  },
  {
    TYPE_SWITCH,
    &setup.auto_play_next_level,		"auto_play_next_level"
  },
  {
    TYPE_SWITCH,
    &setup.count_score_after_game,		"count_score_after_game"
  },
  {
    TYPE_SWITCH,
    &setup.show_scores_after_game,		"show_scores_after_game"
  },
  {
    TYPE_SWITCH,
    &setup.time_limit,				"time_limit"
  },
  {
    TYPE_SWITCH,
    &setup.fullscreen,				"fullscreen"
  },
  {
    TYPE_INTEGER,
    &setup.window_scaling_percent,		"window_scaling_percent"
  },
  {
    TYPE_STRING,
    &setup.window_scaling_quality,		"window_scaling_quality"
  },
  {
    TYPE_STRING,
    &setup.screen_rendering_mode,		"screen_rendering_mode"
  },
  {
    TYPE_STRING,
    &setup.vsync_mode,				"vsync_mode"
  },
  {
    TYPE_SWITCH,
    &setup.ask_on_escape,			"ask_on_escape"
  },
  {
    TYPE_SWITCH,
    &setup.ask_on_escape_editor,		"ask_on_escape_editor"
  },
  {
    TYPE_SWITCH,
    &setup.ask_on_save_tape,			"ask_on_save_tape"
  },
  {
    TYPE_SWITCH,
    &setup.ask_on_game_over,			"ask_on_game_over"
  },
  {
    TYPE_SWITCH,
    &setup.ask_on_quit_game,			"ask_on_quit_game"
  },
  {
    TYPE_SWITCH,
    &setup.ask_on_quit_program,			"ask_on_quit_program"
  },
  {
    TYPE_SWITCH,
    &setup.quick_switch,			"quick_player_switch"
  },
  {
    TYPE_SWITCH,
    &setup.input_on_focus,			"input_on_focus"
  },
  {
    TYPE_SWITCH,
    &setup.prefer_aga_graphics,			"prefer_aga_graphics"
  },
  {
    TYPE_SWITCH,
    &setup.prefer_lowpass_sounds,		"prefer_lowpass_sounds"
  },
  {
    TYPE_SWITCH,
    &setup.show_extra_panel_items,		"show_extra_panel_items"
  },
  {
    TYPE_SWITCH,
    &setup.game_speed_extended,			"game_speed_extended"
  },
  {
    TYPE_INTEGER,
    &setup.game_frame_delay,			"game_frame_delay"
  },
  {
    TYPE_INTEGER,
    &setup.default_game_engine_type,		"default_game_engine_type"
  },
  {
    TYPE_SWITCH,
    &setup.bd_multiple_lives,			"bd_multiple_lives"
  },
  {
    TYPE_SWITCH,
    &setup.bd_skip_uncovering,			"bd_skip_uncovering"
  },
  {
    TYPE_SWITCH,
    &setup.bd_skip_hatching,			"bd_skip_hatching"
  },
  {
    TYPE_SWITCH,
    &setup.bd_scroll_delay,			"bd_scroll_delay"
  },
  {
    TYPE_SWITCH,
    &setup.bd_show_invisible_outbox,		"bd_show_invisible_outbox"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.bd_smooth_movements,			"bd_smooth_movements"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.bd_pushing_graphics,			"bd_pushing_graphics"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.bd_up_down_graphics,			"bd_up_down_graphics"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.bd_falling_sounds,			"bd_falling_sounds"
  },
  {
    TYPE_INTEGER,
    &setup.bd_palette_c64,			"bd_palette_c64"
  },
  {
    TYPE_INTEGER,
    &setup.bd_palette_c64dtv,			"bd_palette_c64dtv"
  },
  {
    TYPE_INTEGER,
    &setup.bd_palette_atari,			"bd_palette_atari"
  },
  {
    TYPE_INTEGER,
    &setup.bd_default_color_type,		"bd_default_color_type"
  },
  {
    TYPE_SWITCH,
    &setup.bd_random_colors,			"bd_random_colors"
  },
  {
    TYPE_SWITCH,
    &setup.sp_show_border_elements,		"sp_show_border_elements"
  },
  {
    TYPE_SWITCH,
    &setup.small_game_graphics,			"small_game_graphics"
  },
  {
    TYPE_SWITCH,
    &setup.show_load_save_buttons,		"show_load_save_buttons"
  },
  {
    TYPE_SWITCH,
    &setup.show_undo_redo_buttons,		"show_undo_redo_buttons"
  },
  {
    TYPE_SWITCH,
    &setup.show_menu_to_save_setup,		"show_menu_to_save_setup"
  },
  {
    TYPE_STRING,
    &setup.scores_in_highscore_list,		"scores_in_highscore_list"
  },
  {
    TYPE_STRING,
    &setup.graphics_set,			"graphics_set"
  },
  {
    TYPE_STRING,
    &setup.sounds_set,				"sounds_set"
  },
  {
    TYPE_STRING,
    &setup.music_set,				"music_set"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.override_level_graphics,		"override_level_graphics"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.override_level_sounds,		"override_level_sounds"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.override_level_music,		"override_level_music"
  },
  {
    TYPE_INTEGER,
    &setup.volume_simple,			"volume_simple"
  },
  {
    TYPE_INTEGER,
    &setup.volume_loops,			"volume_loops"
  },
  {
    TYPE_INTEGER,
    &setup.volume_music,			"volume_music"
  },
  {
    TYPE_SWITCH,
    &setup.audio_sample_rate_44100,		"audio_sample_rate_44100"
  },
  {
    TYPE_SWITCH,
    &setup.network_mode,			"network_mode"
  },
  {
    TYPE_PLAYER,
    &setup.network_player_nr,			"network_player"
  },
  {
    TYPE_STRING,
    &setup.network_server_hostname,		"network_server_hostname"
  },
  {
    TYPE_STRING,
    &setup.touch.control_type,			"touch.control_type"
  },
  {
    TYPE_INTEGER,
    &setup.touch.move_distance,			"touch.move_distance"
  },
  {
    TYPE_INTEGER,
    &setup.touch.drop_distance,			"touch.drop_distance"
  },
  {
    TYPE_INTEGER,
    &setup.touch.transparency,			"touch.transparency"
  },
  {
    TYPE_INTEGER,
    &setup.touch.draw_outlined,			"touch.draw_outlined"
  },
  {
    TYPE_INTEGER,
    &setup.touch.draw_pressed,			"touch.draw_pressed"
  },
  {
    TYPE_INTEGER,
    &setup.touch.grid_xsize[0],			"touch.virtual_buttons.0.xsize"
  },
  {
    TYPE_INTEGER,
    &setup.touch.grid_ysize[0],			"touch.virtual_buttons.0.ysize"
  },
  {
    TYPE_INTEGER,
    &setup.touch.grid_xsize[1],			"touch.virtual_buttons.1.xsize"
  },
  {
    TYPE_INTEGER,
    &setup.touch.grid_ysize[1],			"touch.virtual_buttons.1.ysize"
  },
  {
    TYPE_SWITCH,
    &setup.touch.overlay_buttons,		"touch.overlay_buttons"
  },
};

static struct TokenInfo auto_setup_tokens[] =
{
  {
    TYPE_INTEGER,
    &setup.auto_setup.editor_zoom_tilesize,	"editor.zoom_tilesize"
  },
};

static struct TokenInfo server_setup_tokens[] =
{
  {
    TYPE_STRING,
    &setup.player_uuid,				"player_uuid"
  },
  {
    TYPE_INTEGER,
    &setup.player_version,			"player_version"
  },
  {
    TYPE_SWITCH,
    &setup.use_api_server,          TEST_PREFIX	"use_api_server"
  },
  {
    TYPE_STRING,
    &setup.api_server_hostname,     TEST_PREFIX	"api_server_hostname"
  },
  {
    TYPE_STRING,
    &setup.api_server_password,     TEST_PREFIX	"api_server_password"
  },
  {
    TYPE_SWITCH,
    &setup.ask_for_uploading_tapes, TEST_PREFIX	"ask_for_uploading_tapes"
  },
  {
    TYPE_SWITCH,
    &setup.ask_for_remaining_tapes, TEST_PREFIX	"ask_for_remaining_tapes"
  },
  {
    TYPE_SWITCH,
    &setup.provide_uploading_tapes, TEST_PREFIX	"provide_uploading_tapes"
  },
  {
    TYPE_SWITCH,
    &setup.ask_for_using_api_server,TEST_PREFIX	"ask_for_using_api_server"
  },
  {
    TYPE_SWITCH,
    &setup.has_remaining_tapes,     TEST_PREFIX	"has_remaining_tapes"
  },
};

static struct TokenInfo editor_setup_tokens[] =
{
  {
    TYPE_SWITCH,
    &setup.editor.el_classic,			"editor.el_classic"
  },
  {
    TYPE_SWITCH,
    &setup.editor.el_custom,			"editor.el_custom"
  },
  {
    TYPE_SWITCH,
    &setup.editor.el_user_defined,		"editor.el_user_defined"
  },
  {
    TYPE_SWITCH,
    &setup.editor.el_dynamic,			"editor.el_dynamic"
  },
  {
    TYPE_SWITCH,
    &setup.editor.el_headlines,			"editor.el_headlines"
  },
  {
    TYPE_SWITCH,
    &setup.editor.show_element_token,		"editor.show_element_token"
  },
  {
    TYPE_SWITCH,
    &setup.editor.fast_game_start,		"editor.fast_game_start"
  },
  {
    TYPE_SWITCH,
    &setup.editor.show_read_only_warning,	"editor.show_read_only_warning"
  },
};

static struct TokenInfo editor_cascade_setup_tokens[] =
{
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_bd,		"editor.cascade.el_bd"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_bdx,		"editor.cascade.el_bdx"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_bdx_effects,	"editor.cascade.el_bdx_effects"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_bdx_scanned,	"editor.cascade.el_bdx_scanned"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_em,		"editor.cascade.el_em"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_emc,		"editor.cascade.el_emc"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_rnd,		"editor.cascade.el_rnd"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_sb,		"editor.cascade.el_sb"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_sp,		"editor.cascade.el_sp"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_dc,		"editor.cascade.el_dc"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_dx,		"editor.cascade.el_dx"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_mm,		"editor.cascade.el_mm"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_df,		"editor.cascade.el_df"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_chars,		"editor.cascade.el_chars"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_steel_chars,	"editor.cascade.el_steel_chars"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_ce,		"editor.cascade.el_ce"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_ge,		"editor.cascade.el_ge"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_es,		"editor.cascade.el_es"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_ref,		"editor.cascade.el_ref"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_user,		"editor.cascade.el_user"
  },
  {
    TYPE_SWITCH,
    &setup.editor_cascade.el_dynamic,		"editor.cascade.el_dynamic"
  },
};

static struct TokenInfo shortcut_setup_tokens[] =
{
  {
    TYPE_KEY_X11,
    &setup.shortcut.save_game,			"shortcut.save_game"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.load_game,			"shortcut.load_game"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.restart_game,		"shortcut.restart_game"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.pause_before_end,		"shortcut.pause_before_end"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.toggle_pause,		"shortcut.toggle_pause"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.focus_player[0],		"shortcut.focus_player_1"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.focus_player[1],		"shortcut.focus_player_2"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.focus_player[2],		"shortcut.focus_player_3"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.focus_player[3],		"shortcut.focus_player_4"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.focus_player_all,		"shortcut.focus_player_all"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.tape_eject,			"shortcut.tape_eject"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.tape_extra,			"shortcut.tape_extra"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.tape_stop,			"shortcut.tape_stop"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.tape_pause,			"shortcut.tape_pause"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.tape_record,		"shortcut.tape_record"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.tape_play,			"shortcut.tape_play"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.sound_simple,		"shortcut.sound_simple"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.sound_loops,		"shortcut.sound_loops"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.sound_music,		"shortcut.sound_music"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.snap_left,			"shortcut.snap_left"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.snap_right,			"shortcut.snap_right"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.snap_up,			"shortcut.snap_up"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.snap_down,			"shortcut.snap_down"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.speed_fast,			"shortcut.speed_fast"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.speed_slow,			"shortcut.speed_slow"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.suicide,			"shortcut.suicide"
  },
  {
    TYPE_KEY_X11,
    &setup.shortcut.toggle_panel,		"shortcut.toggle_panel"
  },
};

static struct SetupInputInfo setup_input;
static struct TokenInfo player_setup_tokens[] =
{
  {
    TYPE_BOOLEAN,
    &setup_input.use_joystick,			".use_joystick"
  },
  {
    TYPE_STRING,
    &setup_input.joy.device_name,		".joy.device_name"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.xleft,			".joy.xleft"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.xmiddle,			".joy.xmiddle"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.xright,			".joy.xright"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.yupper,			".joy.yupper"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.ymiddle,			".joy.ymiddle"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.ylower,			".joy.ylower"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.snap,			".joy.snap_field"
  },
  {
    TYPE_INTEGER,
    &setup_input.joy.drop,			".joy.place_bomb"
  },
  {
    TYPE_KEY_X11,
    &setup_input.key.left,			".key.move_left"
  },
  {
    TYPE_KEY_X11,
    &setup_input.key.right,			".key.move_right"
  },
  {
    TYPE_KEY_X11,
    &setup_input.key.up,			".key.move_up"
  },
  {
    TYPE_KEY_X11,
    &setup_input.key.down,			".key.move_down"
  },
  {
    TYPE_KEY_X11,
    &setup_input.key.snap,			".key.snap_field"
  },
  {
    TYPE_KEY_X11,
    &setup_input.key.drop,			".key.place_bomb"
  },
};

static struct TokenInfo system_setup_tokens[] =
{
  {
    TYPE_STRING,
    &setup.system.sdl_renderdriver,		"system.sdl_renderdriver"
  },
  {
    TYPE_STRING,
    &setup.system.sdl_videodriver,		"system.sdl_videodriver"
  },
  {
    TYPE_STRING,
    &setup.system.sdl_audiodriver,		"system.sdl_audiodriver"
  },
  {
    TYPE_INTEGER,
    &setup.system.audio_fragment_size,		"system.audio_fragment_size"
  },
};

static struct TokenInfo internal_setup_tokens[] =
{
  {
    TYPE_STRING,
    &setup.internal.program_title,		"program_title"
  },
  {
    TYPE_STRING,
    &setup.internal.program_version,		"program_version"
  },
  {
    TYPE_STRING,
    &setup.internal.program_author,		"program_author"
  },
  {
    TYPE_STRING,
    &setup.internal.program_email,		"program_email"
  },
  {
    TYPE_STRING,
    &setup.internal.program_website,		"program_website"
  },
  {
    TYPE_STRING,
    &setup.internal.program_copyright,		"program_copyright"
  },
  {
    TYPE_STRING,
    &setup.internal.program_company,		"program_company"
  },
  {
    TYPE_STRING,
    &setup.internal.program_icon_file,		"program_icon_file"
  },
  {
    TYPE_STRING,
    &setup.internal.default_graphics_set,	"default_graphics_set"
  },
  {
    TYPE_STRING,
    &setup.internal.default_sounds_set,		"default_sounds_set"
  },
  {
    TYPE_STRING,
    &setup.internal.default_music_set,		"default_music_set"
  },
  {
    TYPE_STRING,
    &setup.internal.fallback_graphics_file,	"fallback_graphics_file"
  },
  {
    TYPE_STRING,
    &setup.internal.fallback_sounds_file,	"fallback_sounds_file"
  },
  {
    TYPE_STRING,
    &setup.internal.fallback_music_file,	"fallback_music_file"
  },
  {
    TYPE_STRING,
    &setup.internal.default_level_series,	"default_level_series"
  },
  {
    TYPE_INTEGER,
    &setup.internal.default_window_width,	"default_window_width"
  },
  {
    TYPE_INTEGER,
    &setup.internal.default_window_height,	"default_window_height"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.choose_from_top_leveldir,	"choose_from_top_leveldir"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.show_scaling_in_title,	"show_scaling_in_title"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.create_user_levelset,	"create_user_levelset"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_screens_from_main,	"info_screens_from_main"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_game,			"menu_game"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_engines,		"menu_engines"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_editor,		"menu_editor"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_graphics,		"menu_graphics"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_sound,			"menu_sound"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_artwork,		"menu_artwork"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_input,			"menu_input"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_touch,			"menu_touch"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts,		"menu_shortcuts"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_exit,			"menu_exit"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_save_and_exit,		"menu_save_and_exit"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_various,	"menu_shortcuts_various"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_focus,	"menu_shortcuts_focus"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_tape,	"menu_shortcuts_tape"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_sound,	"menu_shortcuts_sound"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_snap,	"menu_shortcuts_snap"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_speed,	"menu_shortcuts_speed"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.menu_shortcuts_engine,	"menu_shortcuts_engine"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_title,			"info_title"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_elements,		"info_elements"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_music,			"info_music"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_credits,		"info_credits"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_program,		"info_program"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_version,		"info_version"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_levelset,		"info_levelset"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_level,			"info_level"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_story,			"info_story"
  },
  {
    TYPE_BOOLEAN,
    &setup.internal.info_exit,			"info_exit"
  },
};

static struct TokenInfo debug_setup_tokens[] =
{
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[0],		"debug.frame_delay_0"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[1],		"debug.frame_delay_1"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[2],		"debug.frame_delay_2"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[3],		"debug.frame_delay_3"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[4],		"debug.frame_delay_4"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[5],		"debug.frame_delay_5"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[6],		"debug.frame_delay_6"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[7],		"debug.frame_delay_7"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[8],		"debug.frame_delay_8"
  },
  {
    TYPE_INTEGER,
    &setup.debug.frame_delay[9],		"debug.frame_delay_9"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[0],		"debug.key.frame_delay_0"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[1],		"debug.key.frame_delay_1"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[2],		"debug.key.frame_delay_2"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[3],		"debug.key.frame_delay_3"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[4],		"debug.key.frame_delay_4"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[5],		"debug.key.frame_delay_5"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[6],		"debug.key.frame_delay_6"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[7],		"debug.key.frame_delay_7"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[8],		"debug.key.frame_delay_8"
  },
  {
    TYPE_KEY_X11,
    &setup.debug.frame_delay_key[9],		"debug.key.frame_delay_9"
  },
  {
    TYPE_BOOLEAN,
    &setup.debug.frame_delay_use_mod_key,	"debug.frame_delay.use_mod_key"},
  {
    TYPE_BOOLEAN,
    &setup.debug.frame_delay_game_only,		"debug.frame_delay.game_only"
  },
  {
    TYPE_BOOLEAN,
    &setup.debug.show_frames_per_second,	"debug.show_frames_per_second"
  },
  {
    TYPE_SWITCH_3_STATES,
    &setup.debug.xsn_mode,			"debug.xsn_mode"
  },
  {
    TYPE_INTEGER,
    &setup.debug.xsn_percent,			"debug.xsn_percent"
  },
};

static struct TokenInfo options_setup_tokens[] =
{
  {
    TYPE_BOOLEAN,
    &setup.options.verbose,			"options.verbose"
  },
  {
    TYPE_BOOLEAN,
    &setup.options.debug,			"options.debug"
  },
  {
    TYPE_STRING,
    &setup.options.debug_mode,			"options.debug_mode"
  },
};

static void setSetupInfoToDefaults(struct SetupInfo *si)
{
  int i;

  si->player_name = getStringCopy(getDefaultUserName(user.nr));

  si->multiple_users = TRUE;

  si->sound = TRUE;
  si->sound_loops = TRUE;
  si->sound_music = TRUE;
  si->sound_simple = TRUE;
  si->toons = TRUE;
  si->global_animations = TRUE;
  si->scroll_delay = TRUE;
  si->forced_scroll_delay = FALSE;
  si->scroll_delay_value = STD_SCROLL_DELAY;
  si->engine_snapshot_mode = getStringCopy(STR_SNAPSHOT_MODE_DEFAULT);
  si->engine_snapshot_memory = SNAPSHOT_MEMORY_DEFAULT;
  si->fade_screens = TRUE;
  si->autorecord = TRUE;
  si->autorecord_after_replay = TRUE;
  si->auto_pause_on_start = FALSE;
  si->show_titlescreen = TRUE;
  si->show_level_story = STATE_ONCE;
  si->quick_doors = FALSE;
  si->team_mode = FALSE;
  si->handicap = TRUE;
  si->skip_levels = TRUE;
  si->allow_skipping_levels = ARG_UNDEFINED_VALUE;
  si->increment_levels = TRUE;
  si->auto_play_next_level = TRUE;
  si->count_score_after_game = TRUE;
  si->show_scores_after_game = TRUE;
  si->time_limit = TRUE;
  si->fullscreen = FALSE;
  si->window_scaling_percent = STD_WINDOW_SCALING_PERCENT;
  si->window_scaling_quality = getStringCopy(SCALING_QUALITY_DEFAULT);
  si->screen_rendering_mode = getStringCopy(STR_SPECIAL_RENDERING_DEFAULT);
  si->vsync_mode = getStringCopy(STR_VSYNC_MODE_DEFAULT);
  si->ask_on_escape = TRUE;
  si->ask_on_escape_editor = TRUE;
  si->ask_on_save_tape = TRUE;
  si->ask_on_game_over = TRUE;
  si->ask_on_quit_game = TRUE;
  si->ask_on_quit_program = TRUE;
  si->quick_switch = FALSE;
  si->input_on_focus = FALSE;
  si->prefer_aga_graphics = TRUE;
  si->prefer_lowpass_sounds = FALSE;
  si->show_extra_panel_items = FALSE;
  si->game_speed_extended = FALSE;
  si->game_frame_delay = GAME_FRAME_DELAY;
  si->default_game_engine_type	= GAME_ENGINE_TYPE_RND;
  si->bd_multiple_lives = FALSE;
  si->bd_skip_uncovering = FALSE;
  si->bd_skip_hatching = FALSE;
  si->bd_scroll_delay = TRUE;
  si->bd_show_invisible_outbox = FALSE;
  si->bd_smooth_movements = STATE_TRUE;
  si->bd_pushing_graphics = STATE_TRUE;
  si->bd_up_down_graphics = STATE_TRUE;
  si->bd_falling_sounds = STATE_AUTO;
  si->bd_palette_c64 = GD_DEFAULT_PALETTE_C64;
  si->bd_palette_c64dtv = GD_DEFAULT_PALETTE_C64DTV;
  si->bd_palette_atari = GD_DEFAULT_PALETTE_ATARI;
  si->bd_default_color_type = GD_DEFAULT_COLOR_TYPE;
  si->bd_random_colors = FALSE;
  si->sp_show_border_elements = FALSE;
  si->small_game_graphics = FALSE;
  si->show_load_save_buttons = FALSE;
  si->show_undo_redo_buttons = FALSE;
  si->show_menu_to_save_setup = FALSE;
  si->scores_in_highscore_list = getStringCopy(STR_SCORES_TYPE_DEFAULT);

  si->graphics_set = getStringCopy(GFX_CLASSIC_SUBDIR);
  si->sounds_set   = getStringCopy(SND_CLASSIC_SUBDIR);
  si->music_set    = getStringCopy(MUS_CLASSIC_SUBDIR);

  si->override_level_graphics = STATE_FALSE;
  si->override_level_sounds = STATE_FALSE;
  si->override_level_music = STATE_FALSE;

  si->volume_simple = 100;		// percent
  si->volume_loops = 100;		// percent
  si->volume_music = 100;		// percent
  si->audio_sample_rate_44100 = FALSE;

  si->network_mode = FALSE;
  si->network_player_nr = 0;		// first player
  si->network_server_hostname = getStringCopy(STR_NETWORK_AUTO_DETECT);

  si->touch.control_type = getStringCopy(TOUCH_CONTROL_DEFAULT);
  si->touch.move_distance = TOUCH_MOVE_DISTANCE_DEFAULT;	// percent
  si->touch.drop_distance = TOUCH_DROP_DISTANCE_DEFAULT;	// percent
  si->touch.transparency = TOUCH_TRANSPARENCY_DEFAULT;		// percent
  si->touch.draw_outlined = TRUE;
  si->touch.draw_pressed = TRUE;

  for (i = 0; i < 2; i++)
  {
    char *default_grid_button[6][2] =
    {
      { "      ", "  ^^  " },
      { "      ", "  ^^  " },
      { "      ", "<<  >>" },
      { "      ", "<<  >>" },
      { "111222", "  vv  " },
      { "111222", "  vv  " }
    };
    int grid_xsize = DEFAULT_GRID_XSIZE(i);
    int grid_ysize = DEFAULT_GRID_YSIZE(i);
    int min_xsize = MIN(6, grid_xsize);
    int min_ysize = MIN(6, grid_ysize);
    int startx = grid_xsize - min_xsize;
    int starty = grid_ysize - min_ysize;
    int x, y;

    // virtual buttons grid can only be set to defaults if video is initialized
    // (this will be repeated if virtual buttons are not loaded from setup file)
    if (video.initialized)
    {
      si->touch.grid_xsize[i] = grid_xsize;
      si->touch.grid_ysize[i] = grid_ysize;
    }
    else
    {
      si->touch.grid_xsize[i] = -1;
      si->touch.grid_ysize[i] = -1;
    }

    for (x = 0; x < MAX_GRID_XSIZE; x++)
      for (y = 0; y < MAX_GRID_YSIZE; y++)
	si->touch.grid_button[i][x][y] = CHAR_GRID_BUTTON_NONE;

    for (x = 0; x < min_xsize; x++)
      for (y = 0; y < min_ysize; y++)
	si->touch.grid_button[i][x][starty + y] =
	  default_grid_button[y][0][x];

    for (x = 0; x < min_xsize; x++)
      for (y = 0; y < min_ysize; y++)
	si->touch.grid_button[i][startx + x][starty + y] =
	  default_grid_button[y][1][x];
  }

  si->touch.grid_initialized		= video.initialized;

  si->touch.overlay_buttons		= FALSE;

  si->editor.el_boulderdash		= TRUE;
  si->editor.el_boulderdash_native	= TRUE;
  si->editor.el_boulderdash_effects	= TRUE;
  si->editor.el_boulderdash_scanned	= FALSE;
  si->editor.el_emerald_mine		= TRUE;
  si->editor.el_emerald_mine_club	= TRUE;
  si->editor.el_more			= TRUE;
  si->editor.el_sokoban			= TRUE;
  si->editor.el_supaplex		= TRUE;
  si->editor.el_diamond_caves		= TRUE;
  si->editor.el_dx_boulderdash		= TRUE;

  si->editor.el_mirror_magic		= TRUE;
  si->editor.el_deflektor		= TRUE;

  si->editor.el_chars			= TRUE;
  si->editor.el_steel_chars		= TRUE;

  si->editor.el_classic			= TRUE;
  si->editor.el_custom			= TRUE;

  si->editor.el_user_defined		= FALSE;
  si->editor.el_dynamic			= TRUE;

  si->editor.el_headlines		= TRUE;

  si->editor.show_element_token		= FALSE;
  si->editor.fast_game_start		= FALSE;

  si->editor.show_read_only_warning	= TRUE;

  si->editor.use_template_for_new_levels = TRUE;

  si->shortcut.save_game	= DEFAULT_KEY_SAVE_GAME;
  si->shortcut.load_game	= DEFAULT_KEY_LOAD_GAME;
  si->shortcut.restart_game	= DEFAULT_KEY_RESTART_GAME;
  si->shortcut.pause_before_end	= DEFAULT_KEY_PAUSE_BEFORE_END;
  si->shortcut.toggle_pause	= DEFAULT_KEY_TOGGLE_PAUSE;

  si->shortcut.focus_player[0]	= DEFAULT_KEY_FOCUS_PLAYER_1;
  si->shortcut.focus_player[1]	= DEFAULT_KEY_FOCUS_PLAYER_2;
  si->shortcut.focus_player[2]	= DEFAULT_KEY_FOCUS_PLAYER_3;
  si->shortcut.focus_player[3]	= DEFAULT_KEY_FOCUS_PLAYER_4;
  si->shortcut.focus_player_all	= DEFAULT_KEY_FOCUS_PLAYER_ALL;

  si->shortcut.tape_eject	= DEFAULT_KEY_TAPE_EJECT;
  si->shortcut.tape_extra	= DEFAULT_KEY_TAPE_EXTRA;
  si->shortcut.tape_stop	= DEFAULT_KEY_TAPE_STOP;
  si->shortcut.tape_pause	= DEFAULT_KEY_TAPE_PAUSE;
  si->shortcut.tape_record	= DEFAULT_KEY_TAPE_RECORD;
  si->shortcut.tape_play	= DEFAULT_KEY_TAPE_PLAY;

  si->shortcut.sound_simple	= DEFAULT_KEY_SOUND_SIMPLE;
  si->shortcut.sound_loops	= DEFAULT_KEY_SOUND_LOOPS;
  si->shortcut.sound_music	= DEFAULT_KEY_SOUND_MUSIC;

  si->shortcut.snap_left	= DEFAULT_KEY_SNAP_LEFT;
  si->shortcut.snap_right	= DEFAULT_KEY_SNAP_RIGHT;
  si->shortcut.snap_up		= DEFAULT_KEY_SNAP_UP;
  si->shortcut.snap_down	= DEFAULT_KEY_SNAP_DOWN;

  si->shortcut.speed_fast	= DEFAULT_KEY_SPEED_FAST;
  si->shortcut.speed_slow	= DEFAULT_KEY_SPEED_SLOW;

  si->shortcut.suicide		= DEFAULT_KEY_SUICIDE;
  si->shortcut.toggle_panel	= DEFAULT_KEY_TOGGLE_PANEL;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    si->input[i].use_joystick = FALSE;
    si->input[i].joy.device_name = getStringCopy(getDeviceNameFromJoystickNr(i));
    si->input[i].joy.xleft   = JOYSTICK_XLEFT;
    si->input[i].joy.xmiddle = JOYSTICK_XMIDDLE;
    si->input[i].joy.xright  = JOYSTICK_XRIGHT;
    si->input[i].joy.yupper  = JOYSTICK_YUPPER;
    si->input[i].joy.ymiddle = JOYSTICK_YMIDDLE;
    si->input[i].joy.ylower  = JOYSTICK_YLOWER;
    si->input[i].joy.snap  = (i == 0 ? JOY_BUTTON_1 : 0);
    si->input[i].joy.drop  = (i == 0 ? JOY_BUTTON_2 : 0);
    si->input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KSYM_UNDEFINED);
    si->input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KSYM_UNDEFINED);
    si->input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KSYM_UNDEFINED);
    si->input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KSYM_UNDEFINED);
    si->input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KSYM_UNDEFINED);
    si->input[i].key.drop  = (i == 0 ? DEFAULT_KEY_DROP  : KSYM_UNDEFINED);
  }

  si->system.sdl_renderdriver = getStringCopy(ARG_DEFAULT);
  si->system.sdl_videodriver = getStringCopy(ARG_DEFAULT);
  si->system.sdl_audiodriver = getStringCopy(ARG_DEFAULT);
  si->system.audio_fragment_size = DEFAULT_AUDIO_FRAGMENT_SIZE;

  si->internal.program_title     = getStringCopy(PROGRAM_TITLE_STRING);
  si->internal.program_version   = getStringCopy(getProgramRealVersionString());
  si->internal.program_author    = getStringCopy(PROGRAM_AUTHOR_STRING);
  si->internal.program_email     = getStringCopy(PROGRAM_EMAIL_STRING);
  si->internal.program_website   = getStringCopy(PROGRAM_WEBSITE_STRING);
  si->internal.program_copyright = getStringCopy(PROGRAM_COPYRIGHT_STRING);
  si->internal.program_company   = getStringCopy(PROGRAM_COMPANY_STRING);

  si->internal.program_icon_file = getStringCopy(PROGRAM_ICON_FILENAME);

  si->internal.default_graphics_set = getStringCopy(GFX_CLASSIC_SUBDIR);
  si->internal.default_sounds_set   = getStringCopy(SND_CLASSIC_SUBDIR);
  si->internal.default_music_set    = getStringCopy(MUS_CLASSIC_SUBDIR);

  si->internal.fallback_graphics_file = getStringCopy(UNDEFINED_FILENAME);
  si->internal.fallback_sounds_file   = getStringCopy(UNDEFINED_FILENAME);
  si->internal.fallback_music_file    = getStringCopy(UNDEFINED_FILENAME);

  si->internal.default_level_series = getStringCopy(UNDEFINED_LEVELSET);
  si->internal.choose_from_top_leveldir = FALSE;
  si->internal.show_scaling_in_title = TRUE;
  si->internal.create_user_levelset = TRUE;
  si->internal.info_screens_from_main = FALSE;

  si->internal.default_window_width  = WIN_XSIZE_DEFAULT;
  si->internal.default_window_height = WIN_YSIZE_DEFAULT;

  si->debug.frame_delay[0] = DEFAULT_FRAME_DELAY_0;
  si->debug.frame_delay[1] = DEFAULT_FRAME_DELAY_1;
  si->debug.frame_delay[2] = DEFAULT_FRAME_DELAY_2;
  si->debug.frame_delay[3] = DEFAULT_FRAME_DELAY_3;
  si->debug.frame_delay[4] = DEFAULT_FRAME_DELAY_4;
  si->debug.frame_delay[5] = DEFAULT_FRAME_DELAY_5;
  si->debug.frame_delay[6] = DEFAULT_FRAME_DELAY_6;
  si->debug.frame_delay[7] = DEFAULT_FRAME_DELAY_7;
  si->debug.frame_delay[8] = DEFAULT_FRAME_DELAY_8;
  si->debug.frame_delay[9] = DEFAULT_FRAME_DELAY_9;

  si->debug.frame_delay_key[0] = DEFAULT_KEY_FRAME_DELAY_0;
  si->debug.frame_delay_key[1] = DEFAULT_KEY_FRAME_DELAY_1;
  si->debug.frame_delay_key[2] = DEFAULT_KEY_FRAME_DELAY_2;
  si->debug.frame_delay_key[3] = DEFAULT_KEY_FRAME_DELAY_3;
  si->debug.frame_delay_key[4] = DEFAULT_KEY_FRAME_DELAY_4;
  si->debug.frame_delay_key[5] = DEFAULT_KEY_FRAME_DELAY_5;
  si->debug.frame_delay_key[6] = DEFAULT_KEY_FRAME_DELAY_6;
  si->debug.frame_delay_key[7] = DEFAULT_KEY_FRAME_DELAY_7;
  si->debug.frame_delay_key[8] = DEFAULT_KEY_FRAME_DELAY_8;
  si->debug.frame_delay_key[9] = DEFAULT_KEY_FRAME_DELAY_9;

  si->debug.frame_delay_use_mod_key = DEFAULT_FRAME_DELAY_USE_MOD_KEY;
  si->debug.frame_delay_game_only   = DEFAULT_FRAME_DELAY_GAME_ONLY;

  si->debug.show_frames_per_second = FALSE;

  si->debug.xsn_mode = STATE_AUTO;
  si->debug.xsn_percent = 0;

  si->options.verbose = FALSE;
  si->options.debug = FALSE;
  si->options.debug_mode = getStringCopy(ARG_UNDEFINED_STRING);

#if defined(PLATFORM_ANDROID)
  si->fullscreen = TRUE;
  si->touch.overlay_buttons = TRUE;
#endif

  setHideSetupEntry(&setup.debug.xsn_mode);
}

static void setSetupInfoToDefaults_AutoSetup(struct SetupInfo *si)
{
  si->auto_setup.editor_zoom_tilesize = MINI_TILESIZE;
}

static void setSetupInfoToDefaults_ServerSetup(struct SetupInfo *si)
{
  si->player_uuid = NULL;	// (will be set later)
  si->player_version = 1;	// (will be set later)

  si->use_api_server = TRUE;
  si->api_server_hostname = getStringCopy(API_SERVER_HOSTNAME);
  si->api_server_password = getStringCopy(UNDEFINED_PASSWORD);
  si->ask_for_uploading_tapes = TRUE;
  si->ask_for_remaining_tapes = FALSE;
  si->provide_uploading_tapes = TRUE;
  si->ask_for_using_api_server = TRUE;
  si->has_remaining_tapes = FALSE;
}

static void setSetupInfoToDefaults_EditorCascade(struct SetupInfo *si)
{
  si->editor_cascade.el_bd		= TRUE;
  si->editor_cascade.el_bdx		= TRUE;
  si->editor_cascade.el_bdx_effects	= FALSE;
  si->editor_cascade.el_bdx_scanned	= FALSE;
  si->editor_cascade.el_em		= TRUE;
  si->editor_cascade.el_emc		= TRUE;
  si->editor_cascade.el_rnd		= TRUE;
  si->editor_cascade.el_sb		= TRUE;
  si->editor_cascade.el_sp		= TRUE;
  si->editor_cascade.el_dc		= TRUE;
  si->editor_cascade.el_dx		= TRUE;

  si->editor_cascade.el_mm		= TRUE;
  si->editor_cascade.el_df		= TRUE;

  si->editor_cascade.el_chars		= FALSE;
  si->editor_cascade.el_steel_chars	= FALSE;
  si->editor_cascade.el_ce		= FALSE;
  si->editor_cascade.el_ge		= FALSE;
  si->editor_cascade.el_es		= FALSE;
  si->editor_cascade.el_ref		= FALSE;
  si->editor_cascade.el_user		= FALSE;
  si->editor_cascade.el_dynamic		= FALSE;
}

#define MAX_HIDE_SETUP_TOKEN_SIZE		20

static char *getHideSetupToken(void *setup_value)
{
  static char hide_setup_token[MAX_HIDE_SETUP_TOKEN_SIZE];

  if (setup_value != NULL)
    snprintf(hide_setup_token, MAX_HIDE_SETUP_TOKEN_SIZE, "%p", setup_value);

  return hide_setup_token;
}

void setHideSetupEntry(void *setup_value)
{
  char *hide_setup_token = getHideSetupToken(setup_value);

  if (hide_setup_hash == NULL)
    hide_setup_hash = newSetupFileHash();

  if (setup_value != NULL)
    setHashEntry(hide_setup_hash, hide_setup_token, "");
}

void removeHideSetupEntry(void *setup_value)
{
  char *hide_setup_token = getHideSetupToken(setup_value);

  if (setup_value != NULL)
    removeHashEntry(hide_setup_hash, hide_setup_token);
}

boolean hideSetupEntry(void *setup_value)
{
  char *hide_setup_token = getHideSetupToken(setup_value);

  return (setup_value != NULL &&
	  getHashEntry(hide_setup_hash, hide_setup_token) != NULL);
}

static void setSetupInfoFromTokenText(SetupFileHash *setup_file_hash,
				      struct TokenInfo *token_info,
				      int token_nr, char *token_text)
{
  char *token_hide_text = getStringCat2(token_text, ".hide");
  char *token_hide_value = getHashEntry(setup_file_hash, token_hide_text);

  // set the value of this setup option in the setup option structure
  setSetupInfo(token_info, token_nr, getHashEntry(setup_file_hash, token_text));

  // check if this setup option should be hidden in the setup menu
  if (token_hide_value != NULL && get_boolean_from_string(token_hide_value))
    setHideSetupEntry(token_info[token_nr].value);

  free(token_hide_text);
}

static void setSetupInfoFromTokenInfo(SetupFileHash *setup_file_hash,
				      struct TokenInfo *token_info,
				      int token_nr)
{
  setSetupInfoFromTokenText(setup_file_hash, token_info, token_nr,
			    token_info[token_nr].text);
}

static void decodeSetupFileHash_Default(SetupFileHash *setup_file_hash)
{
  int i, pnr;

  if (!setup_file_hash)
    return;

  for (i = 0; i < ARRAY_SIZE(global_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, global_setup_tokens, i);

  setup.touch.grid_initialized = TRUE;
  for (i = 0; i < 2; i++)
  {
    int grid_xsize = setup.touch.grid_xsize[i];
    int grid_ysize = setup.touch.grid_ysize[i];
    int x, y;

    // if virtual buttons are not loaded from setup file, repeat initializing
    // virtual buttons grid with default values later when video is initialized
    if (grid_xsize == -1 ||
	grid_ysize == -1)
    {
      setup.touch.grid_initialized = FALSE;

      continue;
    }

    for (y = 0; y < grid_ysize; y++)
    {
      char token_string[MAX_LINE_LEN];

      sprintf(token_string, "touch.virtual_buttons.%d.%02d", i, y);

      char *value_string = getHashEntry(setup_file_hash, token_string);

      if (value_string == NULL)
	continue;

      for (x = 0; x < grid_xsize; x++)
      {
	char c = value_string[x];

	setup.touch.grid_button[i][x][y] =
	  (c == '.' ? CHAR_GRID_BUTTON_NONE : c);
      }
    }
  }

  for (i = 0; i < ARRAY_SIZE(editor_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, editor_setup_tokens, i);

  for (i = 0; i < ARRAY_SIZE(shortcut_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, shortcut_setup_tokens, i);

  for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);

    setup_input = setup.input[pnr];
    for (i = 0; i < ARRAY_SIZE(player_setup_tokens); i++)
    {
      char full_token[100];

      sprintf(full_token, "%s%s", prefix, player_setup_tokens[i].text);
      setSetupInfoFromTokenText(setup_file_hash, player_setup_tokens, i,
				full_token);
    }
    setup.input[pnr] = setup_input;
  }

  for (i = 0; i < ARRAY_SIZE(system_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, system_setup_tokens, i);

  for (i = 0; i < ARRAY_SIZE(internal_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, internal_setup_tokens, i);

  for (i = 0; i < ARRAY_SIZE(debug_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, debug_setup_tokens, i);

  for (i = 0; i < ARRAY_SIZE(options_setup_tokens); i++)
    setSetupInfoFromTokenInfo(setup_file_hash, options_setup_tokens, i);

  setHideRelatedSetupEntries();
}

static void decodeSetupFileHash_AutoSetup(SetupFileHash *setup_file_hash)
{
  int i;

  if (!setup_file_hash)
    return;

  for (i = 0; i < ARRAY_SIZE(auto_setup_tokens); i++)
    setSetupInfo(auto_setup_tokens, i,
		 getHashEntry(setup_file_hash,
			      auto_setup_tokens[i].text));
}

static void decodeSetupFileHash_ServerSetup(SetupFileHash *setup_file_hash)
{
  int i;

  if (!setup_file_hash)
    return;

  for (i = 0; i < ARRAY_SIZE(server_setup_tokens); i++)
    setSetupInfo(server_setup_tokens, i,
		 getHashEntry(setup_file_hash,
			      server_setup_tokens[i].text));
}

static void decodeSetupFileHash_EditorCascade(SetupFileHash *setup_file_hash)
{
  int i;

  if (!setup_file_hash)
    return;

  for (i = 0; i < ARRAY_SIZE(editor_cascade_setup_tokens); i++)
    setSetupInfo(editor_cascade_setup_tokens, i,
		 getHashEntry(setup_file_hash,
			      editor_cascade_setup_tokens[i].text));
}

void LoadUserNames(void)
{
  int last_user_nr = user.nr;
  int i;

  if (global.user_names != NULL)
  {
    for (i = 0; i < MAX_PLAYER_NAMES; i++)
      checked_free(global.user_names[i]);

    checked_free(global.user_names);
  }

  global.user_names = checked_calloc(MAX_PLAYER_NAMES * sizeof(char *));

  for (i = 0; i < MAX_PLAYER_NAMES; i++)
  {
    user.nr = i;

    SetupFileHash *setup_file_hash = loadSetupFileHash(getSetupFilename());

    if (setup_file_hash)
    {
      char *player_name = getHashEntry(setup_file_hash, "player_name");

      global.user_names[i] = getFixedUserName(player_name);

      freeSetupFileHash(setup_file_hash);
    }

    if (global.user_names[i] == NULL)
      global.user_names[i] = getStringCopy(getDefaultUserName(i));
  }

  user.nr = last_user_nr;
}

void LoadSetupFromFilename(char *filename)
{
  SetupFileHash *setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash_Default(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }
  else
  {
    Debug("setup", "using default setup values");
  }
}

static void LoadSetup_SpecialPostProcessing(void)
{
  char *player_name_new;

  // needed to work around problems with fixed length strings
  player_name_new = getFixedUserName(setup.player_name);
  free(setup.player_name);
  setup.player_name = player_name_new;

  // "scroll_delay: on(3) / off(0)" was replaced by scroll delay value
  if (setup.scroll_delay == FALSE)
  {
    setup.scroll_delay_value = MIN_SCROLL_DELAY;
    setup.scroll_delay = TRUE;			// now always "on"
  }

  // make sure that scroll delay value stays inside valid range
  setup.scroll_delay_value =
    MIN(MAX(MIN_SCROLL_DELAY, setup.scroll_delay_value), MAX_SCROLL_DELAY);

  if (setup.allow_skipping_levels == ARG_UNDEFINED_VALUE)
  {
    // if undefined, set from previously used setup options for same purpose
    setup.allow_skipping_levels = (setup.handicap && setup.skip_levels ? STATE_ASK :
                                   setup.handicap ? STATE_FALSE : STATE_TRUE);
  }
}

void LoadSetup_Default(void)
{
  char *filename;

  // always start with reliable default values
  setSetupInfoToDefaults(&setup);

  // try to load setup values from default setup file
  filename = getDefaultSetupFilename();

  if (fileExists(filename))
    LoadSetupFromFilename(filename);

  // try to load setup values from platform setup file
  filename = getPlatformSetupFilename();

  if (fileExists(filename))
    LoadSetupFromFilename(filename);

  // try to load setup values from user setup file
  filename = getSetupFilename();

  LoadSetupFromFilename(filename);

  LoadSetup_SpecialPostProcessing();
}

void LoadSetup_AutoSetup(void)
{
  char *filename = getPath2(getSetupDir(), AUTOSETUP_FILENAME);
  SetupFileHash *setup_file_hash = NULL;

  // always start with reliable default values
  setSetupInfoToDefaults_AutoSetup(&setup);

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash_AutoSetup(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }

  free(filename);
}

void LoadSetup_ServerSetup(void)
{
  char *filename = getPath2(getSetupDir(), SERVERSETUP_FILENAME);
  SetupFileHash *setup_file_hash = NULL;

  // always start with reliable default values
  setSetupInfoToDefaults_ServerSetup(&setup);

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash_ServerSetup(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }

  free(filename);

  if (setup.player_uuid == NULL)
  {
    // player UUID does not yet exist in setup file
    setup.player_uuid = getStringCopy(getUUID());
    setup.player_version = 2;

    SaveSetup_ServerSetup();
  }
}

void LoadSetup_EditorCascade(void)
{
  char *filename = getPath2(getSetupDir(), EDITORCASCADE_FILENAME);
  SetupFileHash *setup_file_hash = NULL;

  // always start with reliable default values
  setSetupInfoToDefaults_EditorCascade(&setup);

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash_EditorCascade(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }

  free(filename);
}

void LoadSetup(void)
{
  LoadSetup_Default();
  LoadSetup_AutoSetup();
  LoadSetup_ServerSetup();
  LoadSetup_EditorCascade();
}

static void addGameControllerMappingToHash(SetupFileHash *mappings_hash,
					   char *mapping_line)
{
  char mapping_guid[MAX_LINE_LEN];
  char *mapping_start, *mapping_end;

  // get GUID from game controller mapping line: copy complete line
  strncpy(mapping_guid, mapping_line, MAX_LINE_LEN - 1);
  mapping_guid[MAX_LINE_LEN - 1] = '\0';

  // get GUID from game controller mapping line: cut after GUID part
  mapping_start = strchr(mapping_guid, ',');
  if (mapping_start != NULL)
    *mapping_start = '\0';

  // cut newline from game controller mapping line
  mapping_end = strchr(mapping_line, '\n');
  if (mapping_end != NULL)
    *mapping_end = '\0';

  // add mapping entry to game controller mappings hash
  setHashEntry(mappings_hash, mapping_guid, mapping_line);
}

static void LoadSetup_ReadGameControllerMappings(SetupFileHash *mappings_hash,
						 char *filename)
{
  FILE *file;

  if (!(file = fopen(filename, MODE_READ)))
  {
    Warn("cannot read game controller mappings file '%s'", filename);

    return;
  }

  while (!feof(file))
  {
    char line[MAX_LINE_LEN];

    if (!fgets(line, MAX_LINE_LEN, file))
      break;

    addGameControllerMappingToHash(mappings_hash, line);
  }

  fclose(file);
}

void SaveSetup_Default(void)
{
  char *filename = getSetupFilename();
  FILE *file;
  int i, pnr;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write setup file '%s'", filename);

    return;
  }

  fprintFileHeader(file, SETUP_FILENAME);

  for (i = 0; i < ARRAY_SIZE(global_setup_tokens); i++)
  {
    // just to make things nicer :)
    if (global_setup_tokens[i].value == &setup.multiple_users		||
	global_setup_tokens[i].value == &setup.sound			||
	global_setup_tokens[i].value == &setup.graphics_set		||
	global_setup_tokens[i].value == &setup.volume_simple		||
	global_setup_tokens[i].value == &setup.network_mode		||
	global_setup_tokens[i].value == &setup.touch.control_type	||
	global_setup_tokens[i].value == &setup.touch.grid_xsize[0]	||
	global_setup_tokens[i].value == &setup.touch.grid_xsize[1])
      fprintf(file, "\n");

    fprintf(file, "%s\n", getSetupLine(global_setup_tokens, "", i));
  }

  for (i = 0; i < 2; i++)
  {
    int grid_xsize = setup.touch.grid_xsize[i];
    int grid_ysize = setup.touch.grid_ysize[i];
    int x, y;

    fprintf(file, "\n");

    for (y = 0; y < grid_ysize; y++)
    {
      char token_string[MAX_LINE_LEN];
      char value_string[MAX_LINE_LEN];

      sprintf(token_string, "touch.virtual_buttons.%d.%02d", i, y);

      for (x = 0; x < grid_xsize; x++)
      {
	char c = setup.touch.grid_button[i][x][y];

	value_string[x] = (c == CHAR_GRID_BUTTON_NONE ? '.' : c);
      }

      value_string[grid_xsize] = '\0';

      fprintf(file, "%s\n", getFormattedSetupEntry(token_string, value_string));
    }
  }

  fprintf(file, "\n");
  for (i = 0; i < ARRAY_SIZE(editor_setup_tokens); i++)
    fprintf(file, "%s\n", getSetupLine(editor_setup_tokens, "", i));

  fprintf(file, "\n");
  for (i = 0; i < ARRAY_SIZE(shortcut_setup_tokens); i++)
    fprintf(file, "%s\n", getSetupLine(shortcut_setup_tokens, "", i));

  for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);
    fprintf(file, "\n");

    setup_input = setup.input[pnr];
    for (i = 0; i < ARRAY_SIZE(player_setup_tokens); i++)
      fprintf(file, "%s\n", getSetupLine(player_setup_tokens, prefix, i));
  }

  fprintf(file, "\n");
  for (i = 0; i < ARRAY_SIZE(system_setup_tokens); i++)
    fprintf(file, "%s\n", getSetupLine(system_setup_tokens, "", i));

  // (internal setup values not saved to user setup file)

  fprintf(file, "\n");
  for (i = 0; i < ARRAY_SIZE(debug_setup_tokens); i++)
    if (!strPrefix(debug_setup_tokens[i].text, "debug.xsn_") ||
	setup.debug.xsn_mode != STATE_AUTO)
      fprintf(file, "%s\n", getSetupLine(debug_setup_tokens, "", i));

  fprintf(file, "\n");
  for (i = 0; i < ARRAY_SIZE(options_setup_tokens); i++)
    fprintf(file, "%s\n", getSetupLine(options_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveSetup_AutoSetup(void)
{
  char *filename = getPath2(getSetupDir(), AUTOSETUP_FILENAME);
  FILE *file;
  int i;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write auto setup file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, AUTOSETUP_FILENAME);

  for (i = 0; i < ARRAY_SIZE(auto_setup_tokens); i++)
    fprintf(file, "%s\n", getSetupLine(auto_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

void SaveSetup_ServerSetup(void)
{
  char *filename = getPath2(getSetupDir(), SERVERSETUP_FILENAME);
  FILE *file;
  int i;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write server setup file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, SERVERSETUP_FILENAME);

  for (i = 0; i < ARRAY_SIZE(server_setup_tokens); i++)
  {
    // just to make things nicer :)
    if (server_setup_tokens[i].value == &setup.use_api_server)
      fprintf(file, "\n");

    fprintf(file, "%s\n", getSetupLine(server_setup_tokens, "", i));
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

void SaveSetup_EditorCascade(void)
{
  char *filename = getPath2(getSetupDir(), EDITORCASCADE_FILENAME);
  FILE *file;
  int i;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write editor cascade state file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, EDITORCASCADE_FILENAME);

  for (i = 0; i < ARRAY_SIZE(editor_cascade_setup_tokens); i++)
    fprintf(file, "%s\n", getSetupLine(editor_cascade_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

void SaveSetup(void)
{
  SaveSetup_Default();
  SaveSetup_AutoSetup();
  SaveSetup_ServerSetup();
  SaveSetup_EditorCascade();
}

void SaveSetupIfNeeded(void)
{
  // save changed setup value if "save and exit" menu disabled
  if (!setup.show_menu_to_save_setup)
    SaveSetup();
}

static void SaveSetup_WriteGameControllerMappings(SetupFileHash *mappings_hash,
						  char *filename)
{
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Warn("cannot write game controller mappings file '%s'", filename);

    return;
  }

  BEGIN_HASH_ITERATION(mappings_hash, itr)
  {
    fprintf(file, "%s\n", HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(mappings_hash, itr)

  fclose(file);
}

void SaveSetup_AddGameControllerMapping(char *mapping)
{
  char *filename = getPath2(getSetupDir(), GAMECONTROLLER_BASENAME);
  SetupFileHash *mappings_hash = newSetupFileHash();

  InitUserDataDirectory();

  // load existing personal game controller mappings
  LoadSetup_ReadGameControllerMappings(mappings_hash, filename);

  // add new mapping to personal game controller mappings
  addGameControllerMappingToHash(mappings_hash, mapping);

  // save updated personal game controller mappings
  SaveSetup_WriteGameControllerMappings(mappings_hash, filename);

  freeSetupFileHash(mappings_hash);
  free(filename);
}

void LoadCustomElementDescriptions(void)
{
  char *filename = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);
  SetupFileHash *setup_file_hash;
  int i;

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    if (element_info[i].custom_description != NULL)
    {
      free(element_info[i].custom_description);
      element_info[i].custom_description = NULL;
    }
  }

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    char *token = getStringCat2(element_info[i].token_name, ".name");
    char *value = getHashEntry(setup_file_hash, token);

    if (value != NULL)
      element_info[i].custom_description = getStringCopy(value);

    free(token);
  }

  freeSetupFileHash(setup_file_hash);
}

static int getElementFromToken(char *token)
{
  char *value = getHashEntry(element_token_hash, token);

  if (value != NULL)
    return atoi(value);

  Warn("unknown element token '%s'", token);

  return EL_UNDEFINED;
}

void FreeGlobalAnimEventInfo(void)
{
  struct GlobalAnimEventInfo *gaei = &global_anim_event_info;

  if (gaei->event_list == NULL)
    return;

  int i;

  for (i = 0; i < gaei->num_event_lists; i++)
  {
    checked_free(gaei->event_list[i]->event_value);
    checked_free(gaei->event_list[i]);
  }

  checked_free(gaei->event_list);

  gaei->event_list = NULL;
  gaei->num_event_lists = 0;
}

static int AddGlobalAnimEventList(void)
{
  struct GlobalAnimEventInfo *gaei = &global_anim_event_info;
  int list_pos = gaei->num_event_lists++;

  gaei->event_list = checked_realloc(gaei->event_list, gaei->num_event_lists *
				     sizeof(struct GlobalAnimEventListInfo *));

  gaei->event_list[list_pos] =
    checked_calloc(sizeof(struct GlobalAnimEventListInfo));

  struct GlobalAnimEventListInfo *gaeli = gaei->event_list[list_pos];

  gaeli->event_value = NULL;
  gaeli->num_event_values = 0;

  return list_pos;
}

static int AddGlobalAnimEventValue(int list_pos, int event_value)
{
  // do not add empty global animation events
  if (event_value == ANIM_EVENT_NONE)
    return list_pos;

  // if list position is undefined, create new list
  if (list_pos == ANIM_EVENT_UNDEFINED)
    list_pos = AddGlobalAnimEventList();

  struct GlobalAnimEventInfo *gaei = &global_anim_event_info;
  struct GlobalAnimEventListInfo *gaeli = gaei->event_list[list_pos];
  int value_pos = gaeli->num_event_values++;

  gaeli->event_value = checked_realloc(gaeli->event_value,
				       gaeli->num_event_values * sizeof(int *));

  gaeli->event_value[value_pos] = event_value;

  return list_pos;
}

int GetGlobalAnimEventValue(int list_pos, int value_pos)
{
  if (list_pos == ANIM_EVENT_UNDEFINED)
    return ANIM_EVENT_NONE;

  struct GlobalAnimEventInfo *gaei = &global_anim_event_info;
  struct GlobalAnimEventListInfo *gaeli = gaei->event_list[list_pos];

  return gaeli->event_value[value_pos];
}

int GetGlobalAnimEventValueCount(int list_pos)
{
  if (list_pos == ANIM_EVENT_UNDEFINED)
    return 0;

  struct GlobalAnimEventInfo *gaei = &global_anim_event_info;
  struct GlobalAnimEventListInfo *gaeli = gaei->event_list[list_pos];

  return gaeli->num_event_values;
}

// This function checks if a string <s> of the format "string1, string2, ..."
// exactly contains a string <s_contained>.

static boolean string_has_parameter(char *s, char *s_contained)
{
  char *substring;

  if (s == NULL || s_contained == NULL)
    return FALSE;

  if (strlen(s_contained) > strlen(s))
    return FALSE;

  if (strncmp(s, s_contained, strlen(s_contained)) == 0)
  {
    char next_char = s[strlen(s_contained)];

    // check if next character is delimiter or whitespace
    if (next_char == ',' || next_char == '\0' ||
	next_char == ' ' || next_char == '\t')
      return TRUE;
  }

  // check if string contains another parameter string after a comma
  substring = strchr(s, ',');
  if (substring == NULL)	// string does not contain a comma
    return FALSE;

  // advance string pointer to next character after the comma
  substring++;

  // skip potential whitespaces after the comma
  while (*substring == ' ' || *substring == '\t')
    substring++;

  return string_has_parameter(substring, s_contained);
}

static int get_anim_parameter_value_ce(char *s)
{
  char *s_ptr = s;
  char *pattern_1 = "ce_change:custom_";
  char *pattern_2 = ".page_";
  int pattern_1_len = strlen(pattern_1);
  char *matching_char = strstr(s_ptr, pattern_1);
  int result = ANIM_EVENT_NONE;

  if (matching_char == NULL)
    return ANIM_EVENT_NONE;

  result = ANIM_EVENT_CE_CHANGE;

  s_ptr = matching_char + pattern_1_len;

  // check for custom element number ("custom_X", "custom_XX" or "custom_XXX")
  if (*s_ptr >= '0' && *s_ptr <= '9')
  {
    int gic_ce_nr = (*s_ptr++ - '0');

    if (*s_ptr >= '0' && *s_ptr <= '9')
    {
      gic_ce_nr = 10 * gic_ce_nr + (*s_ptr++ - '0');

      if (*s_ptr >= '0' && *s_ptr <= '9')
	gic_ce_nr = 10 * gic_ce_nr + (*s_ptr++ - '0');
    }

    if (gic_ce_nr < 1 || gic_ce_nr > NUM_CUSTOM_ELEMENTS)
      return ANIM_EVENT_NONE;

    // custom element stored as 0 to 255
    gic_ce_nr--;

    result |= gic_ce_nr << ANIM_EVENT_CE_BIT;
  }
  else
  {
    // invalid custom element number specified

    return ANIM_EVENT_NONE;
  }

  // check for change page number ("page_X" or "page_XX") (optional)
  if (strPrefix(s_ptr, pattern_2))
  {
    s_ptr += strlen(pattern_2);

    if (*s_ptr >= '0' && *s_ptr <= '9')
    {
      int gic_page_nr = (*s_ptr++ - '0');

      if (*s_ptr >= '0' && *s_ptr <= '9')
	gic_page_nr = 10 * gic_page_nr + (*s_ptr++ - '0');

      if (gic_page_nr < 1 || gic_page_nr > MAX_CHANGE_PAGES)
	return ANIM_EVENT_NONE;

      // change page stored as 1 to 32 (0 means "all change pages")

      result |= gic_page_nr << ANIM_EVENT_PAGE_BIT;
    }
    else
    {
      // invalid animation part number specified

      return ANIM_EVENT_NONE;
    }
  }

  // discard result if next character is neither delimiter nor whitespace
  if (!(*s_ptr == ',' || *s_ptr == '\0' ||
	*s_ptr == ' ' || *s_ptr == '\t'))
    return ANIM_EVENT_NONE;

  return result;
}

static int get_anim_parameter_value(char *s)
{
  int event_value[] =
  {
    ANIM_EVENT_CLICK,
    ANIM_EVENT_INIT,
    ANIM_EVENT_START,
    ANIM_EVENT_END,
    ANIM_EVENT_POST
  };
  char *pattern_1[] =
  {
    "click:anim_",
    "init:anim_",
    "start:anim_",
    "end:anim_",
    "post:anim_"
  };
  char *pattern_2 = ".part_";
  char *matching_char = NULL;
  char *s_ptr = s;
  int pattern_1_len = 0;
  int result = ANIM_EVENT_NONE;
  int i;

  result = get_anim_parameter_value_ce(s);

  if (result != ANIM_EVENT_NONE)
    return result;

  for (i = 0; i < ARRAY_SIZE(event_value); i++)
  {
    matching_char = strstr(s_ptr, pattern_1[i]);
    pattern_1_len = strlen(pattern_1[i]);
    result = event_value[i];

    if (matching_char != NULL)
      break;
  }

  if (matching_char == NULL)
    return ANIM_EVENT_NONE;

  s_ptr = matching_char + pattern_1_len;

  // check for main animation number ("anim_X" or "anim_XX")
  if (*s_ptr >= '0' && *s_ptr <= '9')
  {
    int gic_anim_nr = (*s_ptr++ - '0');

    if (*s_ptr >= '0' && *s_ptr <= '9')
      gic_anim_nr = 10 * gic_anim_nr + (*s_ptr++ - '0');

    if (gic_anim_nr < 1 || gic_anim_nr > MAX_GLOBAL_ANIMS)
      return ANIM_EVENT_NONE;

    result |= gic_anim_nr << ANIM_EVENT_ANIM_BIT;
  }
  else
  {
    // invalid main animation number specified

    return ANIM_EVENT_NONE;
  }

  // check for animation part number ("part_X" or "part_XX") (optional)
  if (strPrefix(s_ptr, pattern_2))
  {
    s_ptr += strlen(pattern_2);

    if (*s_ptr >= '0' && *s_ptr <= '9')
    {
      int gic_part_nr = (*s_ptr++ - '0');

      if (*s_ptr >= '0' && *s_ptr <= '9')
	gic_part_nr = 10 * gic_part_nr + (*s_ptr++ - '0');

      if (gic_part_nr < 1 || gic_part_nr > MAX_GLOBAL_ANIM_PARTS)
	return ANIM_EVENT_NONE;

      result |= gic_part_nr << ANIM_EVENT_PART_BIT;
    }
    else
    {
      // invalid animation part number specified

      return ANIM_EVENT_NONE;
    }
  }

  // discard result if next character is neither delimiter nor whitespace
  if (!(*s_ptr == ',' || *s_ptr == '\0' ||
	*s_ptr == ' ' || *s_ptr == '\t'))
    return ANIM_EVENT_NONE;

  return result;
}

static int get_anim_parameter_values(char *s)
{
  int list_pos = ANIM_EVENT_UNDEFINED;
  int event_value = ANIM_EVENT_DEFAULT;

  if (string_has_parameter(s, "any"))
    event_value |= ANIM_EVENT_ANY;

  if (string_has_parameter(s, "click:self") ||
      string_has_parameter(s, "click") ||
      string_has_parameter(s, "self"))
    event_value |= ANIM_EVENT_SELF;

  if (string_has_parameter(s, "unclick:any"))
    event_value |= ANIM_EVENT_UNCLICK_ANY;

  // if animation event found, add it to global animation event list
  if (event_value != ANIM_EVENT_NONE)
    list_pos = AddGlobalAnimEventValue(list_pos, event_value);

  while (s != NULL)
  {
    // add optional "click:anim_X" or "click:anim_X.part_X" parameter
    event_value = get_anim_parameter_value(s);

    // if animation event found, add it to global animation event list
    if (event_value != ANIM_EVENT_NONE)
      list_pos = AddGlobalAnimEventValue(list_pos, event_value);

    // continue with next part of the string, starting with next comma
    s = strchr(s + 1, ',');
  }

  return list_pos;
}

static int get_anim_action_parameter_value(char *token)
{
  // check most common default case first to massively speed things up
  if (strEqual(token, ARG_UNDEFINED))
    return ANIM_EVENT_ACTION_NONE;

  int result = getImageIDFromToken(token);

  if (result == -1)
  {
    char *gfx_token = getStringCat2("gfx.", token);

    result = getImageIDFromToken(gfx_token);

    checked_free(gfx_token);
  }

  if (result == -1)
  {
    Key key = getKeyFromX11KeyName(token);

    if (key != KSYM_UNDEFINED)
      result = -(int)key;
  }

  if (result == -1)
  {
    if (isURL(token))
    {
      result = get_hash_from_string(token);	// unsigned int => int
      result = ABS(result);			// may be negative now
      result += (result < MAX_IMAGE_FILES ? MAX_IMAGE_FILES : 0);

      setHashEntry(anim_url_hash, int2str(result, 0), token);
    }
  }

  if (result == -1)
    result = ANIM_EVENT_ACTION_NONE;

  return result;
}

static int get_class_parameter_value(char *value)
{
  int result = CLASS_DEFAULT;

  if (strEqual(value, ARG_UNDEFINED))
    return ARG_UNDEFINED_VALUE;

  if (string_has_parameter(value, "mm_engine_only"))
    result |= CLASS_MM_ENGINE_ONLY;

  if (string_has_parameter(value, "extra_panel_items"))
    result |= CLASS_EXTRA_PANEL_ITEMS;

  if (string_has_parameter(value, "bd_pre_hatching"))
    result |= CLASS_BD_PRE_HATCHING;

  if (string_has_parameter(value, "bd_post_hatching"))
    result |= CLASS_BD_POST_HATCHING;

  if (result == CLASS_DEFAULT)
    result = get_hash_from_string(value);

  return result;
}

static int get_style_parameter_value(char *value)
{
  int result = STYLE_DEFAULT;

  if (string_has_parameter(value, "accurate_borders"))
    result |= STYLE_ACCURATE_BORDERS;

  if (string_has_parameter(value, "inner_corners"))
    result |= STYLE_INNER_CORNERS;

  if (string_has_parameter(value, "reverse"))
    result |= STYLE_REVERSE;

  if (string_has_parameter(value, "leftmost_position"))
    result |= STYLE_LEFTMOST_POSITION;

  if (string_has_parameter(value, "block_clicks"))
    result |= STYLE_BLOCK;

  if (string_has_parameter(value, "passthrough_clicks"))
    result |= STYLE_PASSTHROUGH;

  if (string_has_parameter(value, "multiple_actions"))
    result |= STYLE_MULTIPLE_ACTIONS;

  if (string_has_parameter(value, "consume_ce_event"))
    result |= STYLE_CONSUME_CE_EVENT;

  return result;
}

int get_parameter_value(char *value_raw, char *suffix, int type)
{
  char *value = getStringToLower(value_raw);
  int result = 0;	// probably a save default value

  if (strEqual(suffix, ".direction"))
  {
    result = (strEqual(value, "left")  ? MV_LEFT :
	      strEqual(value, "right") ? MV_RIGHT :
	      strEqual(value, "up")    ? MV_UP :
	      strEqual(value, "down")  ? MV_DOWN : MV_NONE);
  }
  else if (strEqual(suffix, ".position"))
  {
    result = (strEqual(value, "left")   ? POS_LEFT :
	      strEqual(value, "right")  ? POS_RIGHT :
	      strEqual(value, "top")    ? POS_TOP :
	      strEqual(value, "upper")  ? POS_UPPER :
	      strEqual(value, "middle") ? POS_MIDDLE :
	      strEqual(value, "lower")  ? POS_LOWER :
	      strEqual(value, "bottom") ? POS_BOTTOM :
	      strEqual(value, "any")    ? POS_ANY :
	      strEqual(value, "ce")     ? POS_CE :
	      strEqual(value, "ce_trigger") ? POS_CE_TRIGGER :
	      strEqual(value, "last")   ? POS_LAST : POS_UNDEFINED);
  }
  else if (strEqual(suffix, ".align"))
  {
    result = (strEqual(value, "left")   ? ALIGN_LEFT :
	      strEqual(value, "right")  ? ALIGN_RIGHT :
	      strEqual(value, "center") ? ALIGN_CENTER :
	      strEqual(value, "middle") ? ALIGN_CENTER : ALIGN_DEFAULT);
  }
  else if (strEqual(suffix, ".valign"))
  {
    result = (strEqual(value, "top")    ? VALIGN_TOP :
	      strEqual(value, "bottom") ? VALIGN_BOTTOM :
	      strEqual(value, "middle") ? VALIGN_MIDDLE :
	      strEqual(value, "center") ? VALIGN_MIDDLE : VALIGN_DEFAULT);
  }
  else if (strEqual(suffix, ".anim_mode"))
  {
    result = (string_has_parameter(value, "none")	? ANIM_NONE :
	      string_has_parameter(value, "loop")	? ANIM_LOOP :
	      string_has_parameter(value, "linear")	? ANIM_LINEAR :
	      string_has_parameter(value, "pingpong")	? ANIM_PINGPONG :
	      string_has_parameter(value, "pingpong2")	? ANIM_PINGPONG2 :
	      string_has_parameter(value, "random")	? ANIM_RANDOM :
	      string_has_parameter(value, "random_static") ? ANIM_RANDOM_STATIC :
	      string_has_parameter(value, "ce_value")	? ANIM_CE_VALUE :
	      string_has_parameter(value, "ce_score")	? ANIM_CE_SCORE :
	      string_has_parameter(value, "ce_delay")	? ANIM_CE_DELAY :
	      string_has_parameter(value, "horizontal")	? ANIM_HORIZONTAL :
	      string_has_parameter(value, "vertical")	? ANIM_VERTICAL :
	      string_has_parameter(value, "centered")	? ANIM_CENTERED :
	      string_has_parameter(value, "all")	? ANIM_ALL :
	      string_has_parameter(value, "tiled")	? ANIM_TILED :
	      string_has_parameter(value, "level_nr")	? ANIM_LEVEL_NR :
	      ANIM_DEFAULT);

    if (string_has_parameter(value, "once"))
      result |= ANIM_ONCE;

    if (string_has_parameter(value, "reverse"))
      result |= ANIM_REVERSE;

    if (string_has_parameter(value, "opaque_player"))
      result |= ANIM_OPAQUE_PLAYER;

    if (string_has_parameter(value, "static_panel"))
      result |= ANIM_STATIC_PANEL;
  }
  else if (strEqual(suffix, ".init_event") ||
	   strEqual(suffix, ".anim_event"))
  {
    result = get_anim_parameter_values(value);
  }
  else if (strEqual(suffix, ".init_delay_action") ||
	   strEqual(suffix, ".anim_delay_action") ||
	   strEqual(suffix, ".post_delay_action") ||
	   strEqual(suffix, ".init_event_action") ||
	   strEqual(suffix, ".anim_event_action"))
  {
    result = get_anim_action_parameter_value(value_raw);
  }
  else if (strEqual(suffix, ".class"))
  {
    result = get_class_parameter_value(value);
  }
  else if (strEqual(suffix, ".style"))
  {
    result = get_style_parameter_value(value);
  }
  else if (strEqual(suffix, ".fade_mode"))
  {
    result = (string_has_parameter(value, "none")	? FADE_MODE_NONE :
	      string_has_parameter(value, "fade")	? FADE_MODE_FADE :
	      string_has_parameter(value, "fade_in")	? FADE_MODE_FADE_IN :
	      string_has_parameter(value, "fade_out")	? FADE_MODE_FADE_OUT :
	      string_has_parameter(value, "crossfade")	? FADE_MODE_CROSSFADE :
	      string_has_parameter(value, "melt")	? FADE_MODE_MELT :
	      string_has_parameter(value, "curtain")	? FADE_MODE_CURTAIN :
	      FADE_MODE_DEFAULT);
  }
  else if (strEqual(suffix, ".auto_delay_unit"))
  {
    result = (string_has_parameter(value, "ms")     ? AUTO_DELAY_UNIT_MS :
	      string_has_parameter(value, "frames") ? AUTO_DELAY_UNIT_FRAMES :
	      AUTO_DELAY_UNIT_DEFAULT);
  }
  else if (strPrefix(suffix, ".font"))		// (may also be ".font_xyz")
  {
    result = gfx.get_font_from_token_function(value);
  }
  else		// generic parameter of type integer or boolean
  {
    result = (strEqual(value, ARG_UNDEFINED) ? ARG_UNDEFINED_VALUE :
	      type == TYPE_INTEGER ? get_integer_from_string(value) :
	      type == TYPE_BOOLEAN ? get_boolean_from_string(value) :
	      ARG_UNDEFINED_VALUE);
  }

  free(value);

  return result;
}

static int get_token_parameter_value(char *token, char *value_raw)
{
  char *suffix;

  if (token == NULL || value_raw == NULL)
    return ARG_UNDEFINED_VALUE;

  suffix = strrchr(token, '.');
  if (suffix == NULL)
    suffix = token;

  if (strEqual(suffix, ".element"))
    return getElementFromToken(value_raw);

  // !!! USE CORRECT VALUE TYPE (currently works also for TYPE_BOOLEAN) !!!
  return get_parameter_value(value_raw, suffix, TYPE_INTEGER);
}

boolean isClass(int class, char *value)
{
  return (class == get_hash_from_string(value));
}

boolean hasClass(int class, int value)
{
  if (class < CLASS_NONE ||
      class > CLASS_MAX)
    return FALSE;

  return ((class & value) != 0);
}

void InitMenuDesignSettings_FromHash(SetupFileHash *setup_file_hash,
				     boolean ignore_defaults)
{
  int i;

  for (i = 0; image_config_vars[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, image_config_vars[i].token);

    // (ignore definitions set to "[DEFAULT]" which are already initialized)
    if (ignore_defaults && strEqual(value, ARG_DEFAULT))
      continue;

    if (value != NULL)
      *image_config_vars[i].value =
	get_token_parameter_value(image_config_vars[i].token, value);
  }
}

void InitMenuDesignSettings_Static(void)
{
  // always start with reliable default values from static default config
  InitMenuDesignSettings_FromHash(image_config_hash, FALSE);
}

static void InitMenuDesignSettings_SpecialPreProcessing(void)
{
  int i;

  // the following initializes hierarchical values from static configuration

  // special case: initialize "ARG_DEFAULT" values in static default config
  // (e.g., initialize "[titlemessage].fade_mode" from "[title].fade_mode")
  titlescreen_initial_first_default.fade_mode  =
    title_initial_first_default.fade_mode;
  titlescreen_initial_first_default.fade_delay =
    title_initial_first_default.fade_delay;
  titlescreen_initial_first_default.post_delay =
    title_initial_first_default.post_delay;
  titlescreen_initial_first_default.auto_delay =
    title_initial_first_default.auto_delay;
  titlescreen_initial_first_default.auto_delay_unit =
    title_initial_first_default.auto_delay_unit;
  titlescreen_first_default.fade_mode  = title_first_default.fade_mode;
  titlescreen_first_default.fade_delay = title_first_default.fade_delay;
  titlescreen_first_default.post_delay = title_first_default.post_delay;
  titlescreen_first_default.auto_delay = title_first_default.auto_delay;
  titlescreen_first_default.auto_delay_unit =
    title_first_default.auto_delay_unit;
  titlemessage_initial_first_default.fade_mode  =
    title_initial_first_default.fade_mode;
  titlemessage_initial_first_default.fade_delay =
    title_initial_first_default.fade_delay;
  titlemessage_initial_first_default.post_delay =
    title_initial_first_default.post_delay;
  titlemessage_initial_first_default.auto_delay =
    title_initial_first_default.auto_delay;
  titlemessage_initial_first_default.auto_delay_unit =
    title_initial_first_default.auto_delay_unit;
  titlemessage_first_default.fade_mode  = title_first_default.fade_mode;
  titlemessage_first_default.fade_delay = title_first_default.fade_delay;
  titlemessage_first_default.post_delay = title_first_default.post_delay;
  titlemessage_first_default.auto_delay = title_first_default.auto_delay;
  titlemessage_first_default.auto_delay_unit =
    title_first_default.auto_delay_unit;

  titlescreen_initial_default.fade_mode  = title_initial_default.fade_mode;
  titlescreen_initial_default.fade_delay = title_initial_default.fade_delay;
  titlescreen_initial_default.post_delay = title_initial_default.post_delay;
  titlescreen_initial_default.auto_delay = title_initial_default.auto_delay;
  titlescreen_initial_default.auto_delay_unit =
    title_initial_default.auto_delay_unit;
  titlescreen_default.fade_mode  = title_default.fade_mode;
  titlescreen_default.fade_delay = title_default.fade_delay;
  titlescreen_default.post_delay = title_default.post_delay;
  titlescreen_default.auto_delay = title_default.auto_delay;
  titlescreen_default.auto_delay_unit = title_default.auto_delay_unit;
  titlemessage_initial_default.fade_mode  = title_initial_default.fade_mode;
  titlemessage_initial_default.fade_delay = title_initial_default.fade_delay;
  titlemessage_initial_default.post_delay = title_initial_default.post_delay;
  titlemessage_initial_default.auto_delay_unit =
    title_initial_default.auto_delay_unit;
  titlemessage_default.fade_mode  = title_default.fade_mode;
  titlemessage_default.fade_delay = title_default.fade_delay;
  titlemessage_default.post_delay = title_default.post_delay;
  titlemessage_default.auto_delay = title_default.auto_delay;
  titlemessage_default.auto_delay_unit = title_default.auto_delay_unit;

  // special case: initialize "ARG_DEFAULT" values in static default config
  // (e.g., init "titlemessage_1.fade_mode" from "[titlemessage].fade_mode")
  for (i = 0; i < MAX_NUM_TITLE_MESSAGES; i++)
  {
    titlescreen_initial_first[i] = titlescreen_initial_first_default;
    titlescreen_first[i] = titlescreen_first_default;
    titlemessage_initial_first[i] = titlemessage_initial_first_default;
    titlemessage_first[i] = titlemessage_first_default;

    titlescreen_initial[i] = titlescreen_initial_default;
    titlescreen[i] = titlescreen_default;
    titlemessage_initial[i] = titlemessage_initial_default;
    titlemessage[i] = titlemessage_default;
  }

  // special case: initialize "ARG_DEFAULT" values in static default config
  // (eg, init "menu.enter_screen.SCORES.xyz" from "menu.enter_screen.xyz")
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    if (i == GFX_SPECIAL_ARG_TITLE)	// title values already initialized
      continue;

    menu.enter_screen[i] = menu.enter_screen[GFX_SPECIAL_ARG_DEFAULT];
    menu.leave_screen[i] = menu.leave_screen[GFX_SPECIAL_ARG_DEFAULT];
    menu.next_screen[i]  = menu.next_screen[GFX_SPECIAL_ARG_DEFAULT];
  }

  // special case: initialize "ARG_DEFAULT" values in static default config
  // (eg, init "viewport.door_1.MAIN.xyz" from "viewport.door_1.xyz")
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    viewport.window[i]    = viewport.window[GFX_SPECIAL_ARG_DEFAULT];
    viewport.playfield[i] = viewport.playfield[GFX_SPECIAL_ARG_DEFAULT];
    viewport.door_1[i]    = viewport.door_1[GFX_SPECIAL_ARG_DEFAULT];

    if (i == GFX_SPECIAL_ARG_EDITOR)	// editor values already initialized
      continue;

    viewport.door_2[i] = viewport.door_2[GFX_SPECIAL_ARG_DEFAULT];
  }
}

static void InitMenuDesignSettings_SpecialPostProcessing(void)
{
  static struct
  {
    struct XY *dst, *src;
  }
  game_buttons_xy[] =
  {
    { &game.button.save,	&game.button.stop	},
    { &game.button.pause2,	&game.button.pause	},
    { &game.button.load,	&game.button.play	},
    { &game.button.undo,	&game.button.stop	},
    { &game.button.redo,	&game.button.play	},

    { NULL,			NULL			}
  };
  int i, j;

  // special case: initialize later added SETUP list size from LEVELS value
  if (menu.list_size[GAME_MODE_SETUP] == -1)
    menu.list_size[GAME_MODE_SETUP] = menu.list_size[GAME_MODE_LEVELS];

  // set default position for snapshot buttons to stop/pause/play buttons
  for (i = 0; game_buttons_xy[i].dst != NULL; i++)
    if ((*game_buttons_xy[i].dst).x == -1 &&
	(*game_buttons_xy[i].dst).y == -1)
      *game_buttons_xy[i].dst = *game_buttons_xy[i].src;

  // --------------------------------------------------------------------------
  // dynamic viewports (including playfield margins, borders and alignments)
  // --------------------------------------------------------------------------

  // dynamic viewports currently only supported for landscape mode
  int display_width  = MAX(video.display_width, video.display_height);
  int display_height = MIN(video.display_width, video.display_height);

  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    struct RectWithBorder *vp_window    = &viewport.window[i];
    struct RectWithBorder *vp_playfield = &viewport.playfield[i];
    struct RectWithBorder *vp_door_1    = &viewport.door_1[i];
    struct RectWithBorder *vp_door_2    = &viewport.door_2[i];
    boolean dynamic_window_width     = (vp_window->min_width     != -1);
    boolean dynamic_window_height    = (vp_window->min_height    != -1);
    boolean dynamic_playfield_width  = (vp_playfield->min_width  != -1);
    boolean dynamic_playfield_height = (vp_playfield->min_height != -1);

    // adjust window size if min/max width/height is specified

    if (vp_window->min_width != -1)
    {
      int window_width = display_width;

      // when using static window height, use aspect ratio of display
      if (vp_window->min_height == -1)
	window_width = vp_window->height * display_width / display_height;

      vp_window->width = MAX(vp_window->min_width, window_width);
    }

    if (vp_window->min_height != -1)
    {
      int window_height = display_height;

      // when using static window width, use aspect ratio of display
      if (vp_window->min_width == -1)
	window_height = vp_window->width * display_height / display_width;

      vp_window->height = MAX(vp_window->min_height, window_height);
    }

    if (vp_window->max_width != -1)
      vp_window->width = MIN(vp_window->width, vp_window->max_width);

    if (vp_window->max_height != -1)
      vp_window->height = MIN(vp_window->height, vp_window->max_height);

    int playfield_width  = vp_window->width;
    int playfield_height = vp_window->height;

    // adjust playfield size and position according to specified margins

    playfield_width  -= vp_playfield->margin_left;
    playfield_width  -= vp_playfield->margin_right;

    playfield_height -= vp_playfield->margin_top;
    playfield_height -= vp_playfield->margin_bottom;

    // adjust playfield size if min/max width/height is specified

    if (vp_playfield->min_width != -1)
      vp_playfield->width = MAX(vp_playfield->min_width, playfield_width);

    if (vp_playfield->min_height != -1)
      vp_playfield->height = MAX(vp_playfield->min_height, playfield_height);

    if (vp_playfield->max_width != -1)
      vp_playfield->width = MIN(vp_playfield->width, vp_playfield->max_width);

    if (vp_playfield->max_height != -1)
      vp_playfield->height = MIN(vp_playfield->height, vp_playfield->max_height);

    // adjust playfield position according to specified alignment

    if (vp_playfield->align == ALIGN_LEFT || vp_playfield->x > 0)
      vp_playfield->x = ALIGNED_VP_XPOS(vp_playfield);
    else if (vp_playfield->align == ALIGN_CENTER)
      vp_playfield->x = playfield_width / 2 - vp_playfield->width / 2;
    else if (vp_playfield->align == ALIGN_RIGHT)
      vp_playfield->x += playfield_width - vp_playfield->width;

    if (vp_playfield->valign == VALIGN_TOP || vp_playfield->y > 0)
      vp_playfield->y = ALIGNED_VP_YPOS(vp_playfield);
    else if (vp_playfield->valign == VALIGN_MIDDLE)
      vp_playfield->y = playfield_height / 2 - vp_playfield->height / 2;
    else if (vp_playfield->valign == VALIGN_BOTTOM)
      vp_playfield->y += playfield_height - vp_playfield->height;

    vp_playfield->x += vp_playfield->margin_left;
    vp_playfield->y += vp_playfield->margin_top;

    // adjust individual playfield borders if only default border is specified

    if (vp_playfield->border_left == -1)
      vp_playfield->border_left = vp_playfield->border_size;
    if (vp_playfield->border_right == -1)
      vp_playfield->border_right = vp_playfield->border_size;
    if (vp_playfield->border_top == -1)
      vp_playfield->border_top = vp_playfield->border_size;
    if (vp_playfield->border_bottom == -1)
      vp_playfield->border_bottom = vp_playfield->border_size;

    // set dynamic playfield borders if borders are specified as undefined
    // (but only if window size was dynamic and playfield size was static)

    if (dynamic_window_width && !dynamic_playfield_width)
    {
      if (vp_playfield->border_left == -1)
      {
	vp_playfield->border_left = (vp_playfield->x -
				     vp_playfield->margin_left);
	vp_playfield->x     -= vp_playfield->border_left;
	vp_playfield->width += vp_playfield->border_left;
      }

      if (vp_playfield->border_right == -1)
      {
	vp_playfield->border_right = (vp_window->width -
				      vp_playfield->x -
				      vp_playfield->width -
				      vp_playfield->margin_right);
	vp_playfield->width += vp_playfield->border_right;
      }
    }

    if (dynamic_window_height && !dynamic_playfield_height)
    {
      if (vp_playfield->border_top == -1)
      {
	vp_playfield->border_top = (vp_playfield->y -
				    vp_playfield->margin_top);
	vp_playfield->y      -= vp_playfield->border_top;
	vp_playfield->height += vp_playfield->border_top;
      }

      if (vp_playfield->border_bottom == -1)
      {
	vp_playfield->border_bottom = (vp_window->height -
				       vp_playfield->y -
				       vp_playfield->height -
				       vp_playfield->margin_bottom);
	vp_playfield->height += vp_playfield->border_bottom;
      }
    }

    // adjust playfield size to be a multiple of a defined alignment tile size

    int align_size = vp_playfield->align_size;
    int playfield_xtiles = vp_playfield->width  / align_size;
    int playfield_ytiles = vp_playfield->height / align_size;
    int playfield_width_corrected  = playfield_xtiles * align_size;
    int playfield_height_corrected = playfield_ytiles * align_size;
    boolean is_playfield_mode = (i == GFX_SPECIAL_ARG_PLAYING ||
				 i == GFX_SPECIAL_ARG_EDITOR);

    if (is_playfield_mode &&
	dynamic_playfield_width &&
	vp_playfield->width != playfield_width_corrected)
    {
      int playfield_xdiff = vp_playfield->width - playfield_width_corrected;

      vp_playfield->width = playfield_width_corrected;

      if (vp_playfield->align == ALIGN_LEFT)
      {
	vp_playfield->border_left += playfield_xdiff;
      }
      else if (vp_playfield->align == ALIGN_RIGHT)
      {
	vp_playfield->border_right += playfield_xdiff;
      }
      else if (vp_playfield->align == ALIGN_CENTER)
      {
	int border_left_diff  = playfield_xdiff / 2;
	int border_right_diff = playfield_xdiff - border_left_diff;

	vp_playfield->border_left  += border_left_diff;
	vp_playfield->border_right += border_right_diff;
      }
    }

    if (is_playfield_mode &&
	dynamic_playfield_height &&
	vp_playfield->height != playfield_height_corrected)
    {
      int playfield_ydiff = vp_playfield->height - playfield_height_corrected;

      vp_playfield->height = playfield_height_corrected;

      if (vp_playfield->valign == VALIGN_TOP)
      {
	vp_playfield->border_top += playfield_ydiff;
      }
      else if (vp_playfield->align == VALIGN_BOTTOM)
      {
	vp_playfield->border_right += playfield_ydiff;
      }
      else if (vp_playfield->align == VALIGN_MIDDLE)
      {
	int border_top_diff    = playfield_ydiff / 2;
	int border_bottom_diff = playfield_ydiff - border_top_diff;

	vp_playfield->border_top    += border_top_diff;
	vp_playfield->border_bottom += border_bottom_diff;
      }
    }

    // adjust door positions according to specified alignment

    for (j = 0; j < 2; j++)
    {
      struct RectWithBorder *vp_door = (j == 0 ? vp_door_1 : vp_door_2);

      if (vp_door->align == ALIGN_LEFT || vp_door->x > 0)
	vp_door->x = ALIGNED_VP_XPOS(vp_door);
      else if (vp_door->align == ALIGN_CENTER)
	vp_door->x = vp_window->width / 2 - vp_door->width / 2;
      else if (vp_door->align == ALIGN_RIGHT)
	vp_door->x += vp_window->width - vp_door->width;

      if (vp_door->valign == VALIGN_TOP || vp_door->y > 0)
	vp_door->y = ALIGNED_VP_YPOS(vp_door);
      else if (vp_door->valign == VALIGN_MIDDLE)
	vp_door->y = vp_window->height / 2 - vp_door->height / 2;
      else if (vp_door->valign == VALIGN_BOTTOM)
	vp_door->y += vp_window->height - vp_door->height;
    }
  }
}

static void InitMenuDesignSettings_SpecialPostProcessing_AfterGraphics(void)
{
  static struct
  {
    struct XYTileSize *dst, *src;
    int graphic;
  }
  editor_buttons_xy[] =
  {
    {
      &editor.button.element_left,	&editor.palette.element_left,
      IMG_GFX_EDITOR_BUTTON_ELEMENT_LEFT
    },
    {
      &editor.button.element_middle,	&editor.palette.element_middle,
      IMG_GFX_EDITOR_BUTTON_ELEMENT_MIDDLE
    },
    {
      &editor.button.element_right,	&editor.palette.element_right,
      IMG_GFX_EDITOR_BUTTON_ELEMENT_RIGHT
    },

    { NULL,			NULL			}
  };
  int i;

  // set default position for element buttons to element graphics
  for (i = 0; editor_buttons_xy[i].dst != NULL; i++)
  {
    if ((*editor_buttons_xy[i].dst).x == -1 &&
	(*editor_buttons_xy[i].dst).y == -1)
    {
      struct GraphicInfo *gd = &graphic_info[editor_buttons_xy[i].graphic];

      gd->width = gd->height = editor_buttons_xy[i].src->tile_size;

      *editor_buttons_xy[i].dst = *editor_buttons_xy[i].src;
    }
  }

  // adjust editor palette rows and columns if specified to be dynamic

  if (editor.palette.cols == -1)
  {
    int vp_width = viewport.playfield[GFX_SPECIAL_ARG_EDITOR].width;
    int bt_width = graphic_info[IMG_EDITOR_PALETTE_BUTTON].width;
    int sc_width = graphic_info[IMG_EDITOR_PALETTE_SCROLLBAR].width;

    editor.palette.cols = (vp_width - sc_width) / bt_width;

    if (editor.palette.x == -1)
    {
      int palette_width = editor.palette.cols * bt_width + sc_width;

      editor.palette.x = (vp_width - palette_width) / 2;
    }
  }

  if (editor.palette.rows == -1)
  {
    int vp_height = viewport.playfield[GFX_SPECIAL_ARG_EDITOR].height;
    int bt_height = graphic_info[IMG_EDITOR_PALETTE_BUTTON].height;
    int tx_height = getFontHeight(FONT_TEXT_2);

    editor.palette.rows = (vp_height - tx_height) / bt_height;

    if (editor.palette.y == -1)
    {
      int palette_height = editor.palette.rows * bt_height + tx_height;

      editor.palette.y = (vp_height - palette_height) / 2;
    }
  }
}

static void InitMenuDesignSettings_PreviewPlayers_Ext(SetupFileHash *hash,
                                                      boolean initialize)
{
  // special case: check if network and preview player positions are redefined,
  // to compare this later against the main menu level preview being redefined
  struct TokenIntPtrInfo menu_config_players[] =
  {
    { "main.network_players.x",	&menu.main.network_players.redefined	},
    { "main.network_players.y",	&menu.main.network_players.redefined	},
    { "main.preview_players.x",	&menu.main.preview_players.redefined	},
    { "main.preview_players.y",	&menu.main.preview_players.redefined	},
    { "preview.x",		&preview.redefined			},
    { "preview.y",		&preview.redefined			}
  };
  int i;

  if (initialize)
  {
    for (i = 0; i < ARRAY_SIZE(menu_config_players); i++)
      *menu_config_players[i].value = FALSE;
  }
  else
  {
    for (i = 0; i < ARRAY_SIZE(menu_config_players); i++)
      if (getHashEntry(hash, menu_config_players[i].token) != NULL)
        *menu_config_players[i].value = TRUE;
  }
}

static void InitMenuDesignSettings_PreviewPlayers(void)
{
  InitMenuDesignSettings_PreviewPlayers_Ext(NULL, TRUE);
}

static void InitMenuDesignSettings_PreviewPlayers_FromHash(SetupFileHash *hash)
{
  InitMenuDesignSettings_PreviewPlayers_Ext(hash, FALSE);
}

static void LoadMenuDesignSettingsFromFilename(char *filename)
{
  static struct TitleFadingInfo tfi;
  static struct TitleMessageInfo tmi;
  static struct TokenInfo title_tokens[] =
  {
    { TYPE_INTEGER,	&tfi.fade_mode,		".fade_mode"		},
    { TYPE_INTEGER,	&tfi.fade_delay,	".fade_delay"		},
    { TYPE_INTEGER,	&tfi.post_delay,	".post_delay"		},
    { TYPE_INTEGER,	&tfi.auto_delay,	".auto_delay"		},
    { TYPE_INTEGER,	&tfi.auto_delay_unit,	".auto_delay_unit"	},

    { -1,		NULL,			NULL			}
  };
  static struct TokenInfo titlemessage_tokens[] =
  {
    { TYPE_INTEGER,	&tmi.x,			".x"			},
    { TYPE_INTEGER,	&tmi.y,			".y"			},
    { TYPE_INTEGER,	&tmi.width,		".width"		},
    { TYPE_INTEGER,	&tmi.height,		".height"		},
    { TYPE_INTEGER,	&tmi.chars,		".chars"		},
    { TYPE_INTEGER,	&tmi.lines,		".lines"		},
    { TYPE_INTEGER,	&tmi.align,		".align"		},
    { TYPE_INTEGER,	&tmi.valign,		".valign"		},
    { TYPE_INTEGER,	&tmi.font,		".font"			},
    { TYPE_BOOLEAN,	&tmi.autowrap,		".autowrap"		},
    { TYPE_BOOLEAN,	&tmi.centered,		".centered"		},
    { TYPE_BOOLEAN,	&tmi.parse_comments,	".parse_comments"	},
    { TYPE_INTEGER,	&tmi.sort_priority,	".sort_priority"	},
    { TYPE_INTEGER,	&tmi.fade_mode,		".fade_mode"		},
    { TYPE_INTEGER,	&tmi.fade_delay,	".fade_delay"		},
    { TYPE_INTEGER,	&tmi.post_delay,	".post_delay"		},
    { TYPE_INTEGER,	&tmi.auto_delay,	".auto_delay"		},
    { TYPE_INTEGER,	&tmi.auto_delay_unit,	".auto_delay_unit"	},

    { -1,		NULL,			NULL			}
  };
  static struct
  {
    struct TitleFadingInfo *info;
    char *text;
  }
  title_info[] =
  {
    // initialize first titles from "enter screen" definitions, if defined
    { &title_initial_first_default,	"menu.enter_screen.TITLE"	},
    { &title_first_default,		"menu.enter_screen.TITLE"	},

    // initialize title screens from "next screen" definitions, if defined
    { &title_initial_default,		"menu.next_screen.TITLE"	},
    { &title_default,			"menu.next_screen.TITLE"	},

    { NULL,				NULL				}
  };
  static struct
  {
    struct TitleMessageInfo *array;
    char *text;
  }
  titlemessage_arrays[] =
  {
    // initialize first titles from "enter screen" definitions, if defined
    { titlescreen_initial_first,	"menu.enter_screen.TITLE"	},
    { titlescreen_first,		"menu.enter_screen.TITLE"	},
    { titlemessage_initial_first,	"menu.enter_screen.TITLE"	},
    { titlemessage_first,		"menu.enter_screen.TITLE"	},

    // initialize titles from "next screen" definitions, if defined
    { titlescreen_initial,		"menu.next_screen.TITLE"	},
    { titlescreen,			"menu.next_screen.TITLE"	},
    { titlemessage_initial,		"menu.next_screen.TITLE"	},
    { titlemessage,			"menu.next_screen.TITLE"	},

    // overwrite titles with title definitions, if defined
    { titlescreen_initial_first,	"[title_initial]"		},
    { titlescreen_first,		"[title]"			},
    { titlemessage_initial_first,	"[title_initial]"		},
    { titlemessage_first,		"[title]"			},

    { titlescreen_initial,		"[title_initial]"		},
    { titlescreen,			"[title]"			},
    { titlemessage_initial,		"[title_initial]"		},
    { titlemessage,			"[title]"			},

    // overwrite titles with title screen/message definitions, if defined
    { titlescreen_initial_first,	"[titlescreen_initial]"		},
    { titlescreen_first,		"[titlescreen]"			},
    { titlemessage_initial_first,	"[titlemessage_initial]"	},
    { titlemessage_first,		"[titlemessage]"		},

    { titlescreen_initial,		"[titlescreen_initial]"		},
    { titlescreen,			"[titlescreen]"			},
    { titlemessage_initial,		"[titlemessage_initial]"	},
    { titlemessage,			"[titlemessage]"		},

    { NULL,				NULL				}
  };
  SetupFileHash *setup_file_hash;
  int i, j, k;

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  // the following initializes hierarchical values from dynamic configuration

  // special case: initialize with default values that may be overwritten
  // (e.g., init "menu.draw_xoffset.INFO" from "menu.draw_xoffset")
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    struct TokenIntPtrInfo menu_config[] =
    {
      { "menu.draw_xoffset",	&menu.draw_xoffset[i]	},
      { "menu.draw_yoffset",	&menu.draw_yoffset[i]	},
      { "menu.list_size",	&menu.list_size[i]	}
    };

    for (j = 0; j < ARRAY_SIZE(menu_config); j++)
    {
      char *token = menu_config[j].token;
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
        *menu_config[j].value = get_integer_from_string(value);
    }
  }

  // special case: initialize with default values that may be overwritten
  // (eg, init "menu.draw_xoffset.INFO[XXX]" from "menu.draw_xoffset.INFO")
  for (i = 0; i < NUM_SPECIAL_GFX_INFO_ARGS; i++)
  {
    struct TokenIntPtrInfo menu_config[] =
    {
      { "menu.draw_xoffset.INFO",	&menu.draw_xoffset_info[i]	},
      { "menu.draw_yoffset.INFO",	&menu.draw_yoffset_info[i]	},
      { "menu.list_size.INFO",		&menu.list_size_info[i]		},
      { "menu.list_entry_size.INFO",	&menu.list_entry_size_info[i]	},
      { "menu.tile_size.INFO",		&menu.tile_size_info[i]		}
    };

    for (j = 0; j < ARRAY_SIZE(menu_config); j++)
    {
      char *token = menu_config[j].token;
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
        *menu_config[j].value = get_integer_from_string(value);
    }
  }

  // special case: initialize with default values that may be overwritten
  // (eg, init "menu.draw_xoffset.SETUP[XXX]" from "menu.draw_xoffset.SETUP")
  for (i = 0; i < NUM_SPECIAL_GFX_SETUP_ARGS; i++)
  {
    struct TokenIntPtrInfo menu_config[] =
    {
      { "menu.draw_xoffset.SETUP",	&menu.draw_xoffset_setup[i]	},
      { "menu.draw_yoffset.SETUP",	&menu.draw_yoffset_setup[i]	}
    };

    for (j = 0; j < ARRAY_SIZE(menu_config); j++)
    {
      char *token = menu_config[j].token;
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
        *menu_config[j].value = get_integer_from_string(value);
    }
  }

  // special case: initialize with default values that may be overwritten
  // (eg, init "menu.line_spacing.INFO[XXX]" from "menu.line_spacing.INFO")
  for (i = 0; i < NUM_SPECIAL_GFX_INFO_ARGS; i++)
  {
    struct TokenIntPtrInfo menu_config[] =
    {
      { "menu.left_spacing.INFO",	&menu.left_spacing_info[i]	},
      { "menu.middle_spacing.INFO",	&menu.middle_spacing_info[i]	},
      { "menu.right_spacing.INFO",	&menu.right_spacing_info[i]	},
      { "menu.top_spacing.INFO",	&menu.top_spacing_info[i]	},
      { "menu.bottom_spacing.INFO",	&menu.bottom_spacing_info[i]	},
      { "menu.paragraph_spacing.INFO",	&menu.paragraph_spacing_info[i]	},
      { "menu.headline1_spacing.INFO",	&menu.headline1_spacing_info[i]	},
      { "menu.headline2_spacing.INFO",	&menu.headline2_spacing_info[i]	},
      { "menu.line_spacing.INFO",	&menu.line_spacing_info[i]	},
      { "menu.extra_spacing.INFO",	&menu.extra_spacing_info[i]	},
    };

    for (j = 0; j < ARRAY_SIZE(menu_config); j++)
    {
      char *token = menu_config[j].token;
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
        *menu_config[j].value = get_integer_from_string(value);
    }
  }

  // special case: initialize with default values that may be overwritten
  // (eg, init "menu.enter_screen.SCORES.xyz" from "menu.enter_screen.xyz")
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    struct TokenIntPtrInfo menu_config[] =
    {
      { "menu.enter_screen.fade_mode",	&menu.enter_screen[i].fade_mode	 },
      { "menu.enter_screen.fade_delay",	&menu.enter_screen[i].fade_delay },
      { "menu.enter_screen.post_delay",	&menu.enter_screen[i].post_delay },
      { "menu.leave_screen.fade_mode",	&menu.leave_screen[i].fade_mode	 },
      { "menu.leave_screen.fade_delay",	&menu.leave_screen[i].fade_delay },
      { "menu.leave_screen.post_delay",	&menu.leave_screen[i].post_delay },
      { "menu.next_screen.fade_mode",	&menu.next_screen[i].fade_mode	 },
      { "menu.next_screen.fade_delay",	&menu.next_screen[i].fade_delay	 },
      { "menu.next_screen.post_delay",	&menu.next_screen[i].post_delay	 }
    };

    for (j = 0; j < ARRAY_SIZE(menu_config); j++)
    {
      char *token = menu_config[j].token;
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
        *menu_config[j].value = get_token_parameter_value(token, value);
    }
  }

  // special case: initialize with default values that may be overwritten
  // (eg, init "viewport.door_1.MAIN.xyz" from "viewport.door_1.xyz")
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    struct
    {
      char *token_prefix;
      struct RectWithBorder *struct_ptr;
    }
    vp_struct[] =
    {
      { "viewport.window",	&viewport.window[i]	},
      { "viewport.playfield",	&viewport.playfield[i]	},
      { "viewport.door_1",	&viewport.door_1[i]	},
      { "viewport.door_2",	&viewport.door_2[i]	}
    };

    for (j = 0; j < ARRAY_SIZE(vp_struct); j++)
    {
      struct TokenIntPtrInfo vp_config[] =
      {
        { ".x",			&vp_struct[j].struct_ptr->x		},
        { ".y",			&vp_struct[j].struct_ptr->y		},
        { ".width",		&vp_struct[j].struct_ptr->width		},
        { ".height",		&vp_struct[j].struct_ptr->height	},
        { ".min_width",		&vp_struct[j].struct_ptr->min_width	},
        { ".min_height",	&vp_struct[j].struct_ptr->min_height	},
        { ".max_width",		&vp_struct[j].struct_ptr->max_width	},
        { ".max_height",	&vp_struct[j].struct_ptr->max_height	},
        { ".margin_left",	&vp_struct[j].struct_ptr->margin_left	},
        { ".margin_right",	&vp_struct[j].struct_ptr->margin_right	},
        { ".margin_top",	&vp_struct[j].struct_ptr->margin_top	},
        { ".margin_bottom",	&vp_struct[j].struct_ptr->margin_bottom	},
        { ".border_left",	&vp_struct[j].struct_ptr->border_left	},
        { ".border_right",	&vp_struct[j].struct_ptr->border_right	},
        { ".border_top",	&vp_struct[j].struct_ptr->border_top	},
        { ".border_bottom",	&vp_struct[j].struct_ptr->border_bottom	},
        { ".border_size",	&vp_struct[j].struct_ptr->border_size	},
        { ".align_size",	&vp_struct[j].struct_ptr->align_size	},
        { ".align",		&vp_struct[j].struct_ptr->align		},
        { ".valign",		&vp_struct[j].struct_ptr->valign	}
      };

      for (k = 0; k < ARRAY_SIZE(vp_config); k++)
      {
        char *token = getStringCat2(vp_struct[j].token_prefix,
                                    vp_config[k].token);
        char *value = getHashEntry(setup_file_hash, token);

        if (value != NULL)
          *vp_config[k].value = get_token_parameter_value(token, value);

        free(token);
      }
    }
  }

  // special case: initialize with default values that may be overwritten
  // (e.g., init "[title].fade_mode" from "menu.next_screen.TITLE.fade_mode")
  for (i = 0; title_info[i].info != NULL; i++)
  {
    struct TitleFadingInfo *info = title_info[i].info;
    char *base_token = title_info[i].text;

    for (j = 0; title_tokens[j].type != -1; j++)
    {
      char *token = getStringCat2(base_token, title_tokens[j].text);
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
      {
	int parameter_value = get_token_parameter_value(token, value);

	tfi = *info;

	*(int *)title_tokens[j].value = (int)parameter_value;

	*info = tfi;
      }

      free(token);
    }
  }

  // special case: initialize with default values that may be overwritten
  // (e.g., init "titlemessage_1.fade_mode" from "[titlemessage].fade_mode")
  for (i = 0; titlemessage_arrays[i].array != NULL; i++)
  {
    struct TitleMessageInfo *array = titlemessage_arrays[i].array;
    char *base_token = titlemessage_arrays[i].text;

    for (j = 0; titlemessage_tokens[j].type != -1; j++)
    {
      char *token = getStringCat2(base_token, titlemessage_tokens[j].text);
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
      {
	int parameter_value = get_token_parameter_value(token, value);

	for (k = 0; k < MAX_NUM_TITLE_MESSAGES; k++)
	{
	  tmi = array[k];

	  if (titlemessage_tokens[j].type == TYPE_INTEGER)
	    *(int     *)titlemessage_tokens[j].value = (int)parameter_value;
	  else
	    *(boolean *)titlemessage_tokens[j].value = (boolean)parameter_value;

	  array[k] = tmi;
	}
      }

      free(token);
    }
  }

  // read (and overwrite with) values that may be specified in config file
  InitMenuDesignSettings_FromHash(setup_file_hash, TRUE);

  // special case: check if network and preview player positions are redefined
  InitMenuDesignSettings_PreviewPlayers_FromHash(setup_file_hash);

  freeSetupFileHash(setup_file_hash);
}

void LoadMenuDesignSettings(void)
{
  char *filename_base = UNDEFINED_FILENAME, *filename_local;

  InitMenuDesignSettings_Static();
  InitMenuDesignSettings_SpecialPreProcessing();
  InitMenuDesignSettings_PreviewPlayers();

  if (!GFX_OVERRIDE_ARTWORK(ARTWORK_TYPE_GRAPHICS))
  {
    // first look for special settings configured in level series config
    filename_base = getCustomArtworkLevelConfigFilename(ARTWORK_TYPE_GRAPHICS);

    if (fileExists(filename_base))
      LoadMenuDesignSettingsFromFilename(filename_base);
  }

  filename_local = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);

  if (filename_local != NULL && !strEqual(filename_base, filename_local))
    LoadMenuDesignSettingsFromFilename(filename_local);

  InitMenuDesignSettings_SpecialPostProcessing();
}

void LoadMenuDesignSettings_AfterGraphics(void)
{
  InitMenuDesignSettings_SpecialPostProcessing_AfterGraphics();
}

void InitSoundSettings_FromHash(SetupFileHash *setup_file_hash,
				boolean ignore_defaults)
{
  int i;

  for (i = 0; sound_config_vars[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, sound_config_vars[i].token);

    // (ignore definitions set to "[DEFAULT]" which are already initialized)
    if (ignore_defaults && strEqual(value, ARG_DEFAULT))
      continue;

    if (value != NULL)
      *sound_config_vars[i].value =
	get_token_parameter_value(sound_config_vars[i].token, value);
  }
}

void InitSoundSettings_Static(void)
{
  // always start with reliable default values from static default config
  InitSoundSettings_FromHash(sound_config_hash, FALSE);
}

static void LoadSoundSettingsFromFilename(char *filename)
{
  SetupFileHash *setup_file_hash;

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  // read (and overwrite with) values that may be specified in config file
  InitSoundSettings_FromHash(setup_file_hash, TRUE);

  freeSetupFileHash(setup_file_hash);
}

void LoadSoundSettings(void)
{
  char *filename_base = UNDEFINED_FILENAME, *filename_local;

  InitSoundSettings_Static();

  if (!GFX_OVERRIDE_ARTWORK(ARTWORK_TYPE_SOUNDS))
  {
    // first look for special settings configured in level series config
    filename_base = getCustomArtworkLevelConfigFilename(ARTWORK_TYPE_SOUNDS);

    if (fileExists(filename_base))
      LoadSoundSettingsFromFilename(filename_base);
  }

  filename_local = getCustomArtworkConfigFilename(ARTWORK_TYPE_SOUNDS);

  if (filename_local != NULL && !strEqual(filename_base, filename_local))
    LoadSoundSettingsFromFilename(filename_local);
}

void LoadUserDefinedEditorElementList(int **elements, int *num_elements)
{
  char *filename = getEditorSetupFilename();
  SetupFileList *setup_file_list, *list;
  SetupFileHash *element_hash;
  int num_unknown_tokens = 0;
  int i;

  if ((setup_file_list = loadSetupFileList(filename)) == NULL)
    return;

  element_hash = newSetupFileHash();

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
    setHashEntry(element_hash, element_info[i].token_name, i_to_a(i));

  // determined size may be larger than needed (due to unknown elements)
  *num_elements = 0;
  for (list = setup_file_list; list != NULL; list = list->next)
    (*num_elements)++;

  // add space for up to 3 more elements for padding that may be needed
  *num_elements += 3;

  // free memory for old list of elements, if needed
  checked_free(*elements);

  // allocate memory for new list of elements
  *elements = checked_malloc(*num_elements * sizeof(int));

  *num_elements = 0;
  for (list = setup_file_list; list != NULL; list = list->next)
  {
    char *value = getHashEntry(element_hash, list->token);

    if (value == NULL)		// try to find obsolete token mapping
    {
      char *mapped_token = get_mapped_token(list->token);

      if (mapped_token != NULL)
      {
	value = getHashEntry(element_hash, mapped_token);

	free(mapped_token);
      }
    }

    if (value != NULL)
    {
      (*elements)[(*num_elements)++] = atoi(value);
    }
    else
    {
      if (num_unknown_tokens == 0)
      {
	Warn("---");
	Warn("unknown token(s) found in config file:");
	Warn("- config file: '%s'", filename);

	num_unknown_tokens++;
      }

      Warn("- token: '%s'", list->token);
    }
  }

  if (num_unknown_tokens > 0)
    Warn("---");

  while (*num_elements % 4)	// pad with empty elements, if needed
    (*elements)[(*num_elements)++] = EL_EMPTY;

  freeSetupFileList(setup_file_list);
  freeSetupFileHash(element_hash);

#if 0
  for (i = 0; i < *num_elements; i++)
    Debug("editor", "element '%s' [%d]\n",
	  element_info[(*elements)[i]].token_name, (*elements)[i]);
#endif
}

static struct MusicFileInfo *get_music_file_info_ext(char *basename, int music,
						     boolean is_sound)
{
  SetupFileHash *setup_file_hash = NULL;
  struct MusicFileInfo tmp_music_file_info, *new_music_file_info;
  char *filename_music, *filename_prefix, *filename_info;
  struct
  {
    char *token;
    char **value_ptr;
  }
  token_to_value_ptr[] =
  {
    { "title_header",	&tmp_music_file_info.title_header	},
    { "artist_header",	&tmp_music_file_info.artist_header	},
    { "album_header",	&tmp_music_file_info.album_header	},
    { "year_header",	&tmp_music_file_info.year_header	},
    { "played_header",	&tmp_music_file_info.played_header	},

    { "title",		&tmp_music_file_info.title		},
    { "artist",		&tmp_music_file_info.artist		},
    { "album",		&tmp_music_file_info.album		},
    { "year",		&tmp_music_file_info.year		},
    { "played",		&tmp_music_file_info.played		},

    { NULL,		NULL					},
  };
  int i;

  filename_music = (is_sound ? getCustomSoundFilename(basename) :
		    getCustomMusicFilename(basename));

  if (filename_music == NULL)
    return NULL;

  // ---------- try to replace file extension ----------

  filename_prefix = getStringCopy(filename_music);
  if (strrchr(filename_prefix, '.') != NULL)
    *strrchr(filename_prefix, '.') = '\0';
  filename_info = getStringCat2(filename_prefix, ".txt");

  if (fileExists(filename_info))
    setup_file_hash = loadSetupFileHash(filename_info);

  free(filename_prefix);
  free(filename_info);

  if (setup_file_hash == NULL)
  {
    // ---------- try to add file extension ----------

    filename_prefix = getStringCopy(filename_music);
    filename_info = getStringCat2(filename_prefix, ".txt");

    if (fileExists(filename_info))
      setup_file_hash = loadSetupFileHash(filename_info);

    free(filename_prefix);
    free(filename_info);
  }

  if (setup_file_hash == NULL)
    return NULL;

  // ---------- music file info found ----------

  clear_mem(&tmp_music_file_info, sizeof(struct MusicFileInfo));

  for (i = 0; token_to_value_ptr[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, token_to_value_ptr[i].token);

    *token_to_value_ptr[i].value_ptr =
      getStringCopy(value != NULL && *value != '\0' ? value : UNKNOWN_NAME);
  }

  tmp_music_file_info.basename = getStringCopy(basename);
  tmp_music_file_info.music = music;
  tmp_music_file_info.is_sound = is_sound;

  new_music_file_info = checked_malloc(sizeof(struct MusicFileInfo));
  *new_music_file_info = tmp_music_file_info;

  return new_music_file_info;
}

static struct MusicFileInfo *get_music_file_info(char *basename, int music)
{
  return get_music_file_info_ext(basename, music, FALSE);
}

static struct MusicFileInfo *get_sound_file_info(char *basename, int sound)
{
  return get_music_file_info_ext(basename, sound, TRUE);
}

static boolean music_info_listed_ext(struct MusicFileInfo *list,
				     char *basename, boolean is_sound)
{
  for (; list != NULL; list = list->next)
    if (list->is_sound == is_sound && strEqual(list->basename, basename))
      return TRUE;

  return FALSE;
}

static boolean music_info_listed(struct MusicFileInfo *list, char *basename)
{
  return music_info_listed_ext(list, basename, FALSE);
}

static boolean sound_info_listed(struct MusicFileInfo *list, char *basename)
{
  return music_info_listed_ext(list, basename, TRUE);
}

void LoadMusicInfo(void)
{
  int num_music_noconf = getMusicListSize_NoConf();
  int num_music = getMusicListSize();
  int num_sounds = getSoundListSize();
  struct FileInfo *music, *sound;
  struct MusicFileInfo *next, **new;

  int i;

  while (music_file_info != NULL)
  {
    next = music_file_info->next;

    checked_free(music_file_info->basename);

    checked_free(music_file_info->title_header);
    checked_free(music_file_info->artist_header);
    checked_free(music_file_info->album_header);
    checked_free(music_file_info->year_header);
    checked_free(music_file_info->played_header);

    checked_free(music_file_info->title);
    checked_free(music_file_info->artist);
    checked_free(music_file_info->album);
    checked_free(music_file_info->year);
    checked_free(music_file_info->played);

    free(music_file_info);

    music_file_info = next;
  }

  new = &music_file_info;

  // get (configured or unconfigured) music file info for all levels
  for (i = leveldir_current->first_level;
       i <= leveldir_current->last_level; i++)
  {
    int music_nr;

    if (levelset.music[i] != MUS_UNDEFINED)
    {
      // get music file info for configured level music
      music_nr = levelset.music[i];
    }
    else if (num_music_noconf > 0)
    {
      // get music file info for unconfigured level music
      int level_pos = i - leveldir_current->first_level;

      music_nr = MAP_NOCONF_MUSIC(level_pos % num_music_noconf);
    }
    else
    {
      continue;
    }

    char *basename = getMusicInfoEntryFilename(music_nr);

    if (basename == NULL)
      continue;

    if (!music_info_listed(music_file_info, basename))
    {
      *new = get_music_file_info(basename, music_nr);

      if (*new != NULL)
	new = &(*new)->next;
    }
  }

  // get music file info for all remaining configured music files
  for (i = 0; i < num_music; i++)
  {
    music = getMusicListEntry(i);

    if (music->filename == NULL)
      continue;

    if (strEqual(music->filename, UNDEFINED_FILENAME))
      continue;

    // a configured file may be not recognized as music
    if (!FileIsMusic(music->filename))
      continue;

    if (!music_info_listed(music_file_info, music->filename))
    {
      *new = get_music_file_info(music->filename, i);

      if (*new != NULL)
	new = &(*new)->next;
    }
  }

  // get sound file info for all configured sound files
  for (i = 0; i < num_sounds; i++)
  {
    sound = getSoundListEntry(i);

    if (sound->filename == NULL)
      continue;

    if (strEqual(sound->filename, UNDEFINED_FILENAME))
      continue;

    // a configured file may be not recognized as sound
    if (!FileIsSound(sound->filename))
      continue;

    if (!sound_info_listed(music_file_info, sound->filename))
    {
      *new = get_sound_file_info(sound->filename, i);
      if (*new != NULL)
	new = &(*new)->next;
    }
  }

  // add pointers to previous list nodes

  struct MusicFileInfo *node = music_file_info;

  while (node != NULL)
  {
    if (node->next)
      node->next->prev = node;

    node = node->next;
  }
}

static void add_helpanim_entry(int element, int action, int direction,
			       int delay, int *num_list_entries)
{
  struct HelpAnimInfo *new_list_entry;
  (*num_list_entries)++;

  helpanim_info =
    checked_realloc(helpanim_info,
		    *num_list_entries * sizeof(struct HelpAnimInfo));
  new_list_entry = &helpanim_info[*num_list_entries - 1];

  new_list_entry->element = element;
  new_list_entry->action = action;
  new_list_entry->direction = direction;
  new_list_entry->delay = delay;
}

static void print_unknown_token(char *filename, char *token, int token_nr)
{
  if (token_nr == 0)
  {
    Warn("---");
    Warn("unknown token(s) found in config file:");
    Warn("- config file: '%s'", filename);
  }

  Warn("- token: '%s'", token);
}

static void print_unknown_token_end(int token_nr)
{
  if (token_nr > 0)
    Warn("---");
}

void LoadHelpAnimInfo(void)
{
  char *filename = getHelpAnimFilename();
  SetupFileList *setup_file_list = NULL, *list;
  SetupFileHash *element_hash, *action_hash, *direction_hash;
  int num_list_entries = 0;
  int num_unknown_tokens = 0;
  int i;

  if (fileExists(filename))
    setup_file_list = loadSetupFileList(filename);

  if (setup_file_list == NULL)
  {
    // use reliable default values from static configuration
    SetupFileList *insert_ptr;

    insert_ptr = setup_file_list =
      newSetupFileList(helpanim_config[0].token,
		       helpanim_config[0].value);

    for (i = 1; helpanim_config[i].token; i++)
      insert_ptr = addListEntry(insert_ptr,
				helpanim_config[i].token,
				helpanim_config[i].value);
  }

  element_hash   = newSetupFileHash();
  action_hash    = newSetupFileHash();
  direction_hash = newSetupFileHash();

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    setHashEntry(element_hash, element_info[i].token_name, i_to_a(i));

  for (i = 0; i < NUM_ACTIONS; i++)
    setHashEntry(action_hash, element_action_info[i].suffix,
		 i_to_a(element_action_info[i].value));

  // do not store direction index (bit) here, but direction value!
  for (i = 0; i < NUM_DIRECTIONS_FULL; i++)
    setHashEntry(direction_hash, element_direction_info[i].suffix,
		 i_to_a(1 << element_direction_info[i].value));

  for (list = setup_file_list; list != NULL; list = list->next)
  {
    char *element_token, *action_token, *direction_token;
    char *element_value, *action_value, *direction_value;
    int delay = atoi(list->value);

    if (strEqual(list->token, "end"))
    {
      add_helpanim_entry(HELPANIM_LIST_NEXT, -1, -1, -1, &num_list_entries);

      continue;
    }

    /* first try to break element into element/action/direction parts;
       if this does not work, also accept combined "element[.act][.dir]"
       elements (like "dynamite.active"), which are unique elements */

    if (strchr(list->token, '.') == NULL)	// token contains no '.'
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	// element found
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
      {
	// no further suffixes found -- this is not an element
	print_unknown_token(filename, list->token, num_unknown_tokens++);
      }

      continue;
    }

    // token has format "<prefix>.<something>"

    action_token = strchr(list->token, '.');	// suffix may be action ...
    direction_token = action_token;		// ... or direction

    element_token = getStringCopy(list->token);
    *strchr(element_token, '.') = '\0';

    element_value = getHashEntry(element_hash, element_token);

    if (element_value == NULL)		// this is no element
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	// combined element found
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);

      continue;
    }

    action_value = getHashEntry(action_hash, action_token);

    if (action_value != NULL)		// action found
    {
      add_helpanim_entry(atoi(element_value), atoi(action_value), -1, delay,
		    &num_list_entries);

      free(element_token);

      continue;
    }

    direction_value = getHashEntry(direction_hash, direction_token);

    if (direction_value != NULL)	// direction found
    {
      add_helpanim_entry(atoi(element_value), -1, atoi(direction_value), delay,
			 &num_list_entries);

      free(element_token);

      continue;
    }

    if (strchr(action_token + 1, '.') == NULL)
    {
      // no further suffixes found -- this is not an action nor direction

      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	// combined element found
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);

      continue;
    }

    // token has format "<prefix>.<suffix>.<something>"

    direction_token = strchr(action_token + 1, '.');

    action_token = getStringCopy(action_token);
    *strchr(action_token + 1, '.') = '\0';

    action_value = getHashEntry(action_hash, action_token);

    if (action_value == NULL)		// this is no action
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	// combined element found
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);
      free(action_token);

      continue;
    }

    direction_value = getHashEntry(direction_hash, direction_token);

    if (direction_value != NULL)	// direction found
    {
      add_helpanim_entry(atoi(element_value), atoi(action_value),
			 atoi(direction_value), delay, &num_list_entries);

      free(element_token);
      free(action_token);

      continue;
    }

    // this is no direction

    element_value = getHashEntry(element_hash, list->token);
    if (element_value != NULL)		// combined element found
      add_helpanim_entry(atoi(element_value), -1, -1, delay,
			 &num_list_entries);
    else
      print_unknown_token(filename, list->token, num_unknown_tokens++);

    free(element_token);
    free(action_token);
  }

  print_unknown_token_end(num_unknown_tokens);

  add_helpanim_entry(HELPANIM_LIST_NEXT, -1, -1, -1, &num_list_entries);
  add_helpanim_entry(HELPANIM_LIST_END,  -1, -1, -1, &num_list_entries);

  freeSetupFileList(setup_file_list);
  freeSetupFileHash(element_hash);
  freeSetupFileHash(action_hash);
  freeSetupFileHash(direction_hash);

#if 0
  for (i = 0; i < num_list_entries; i++)
    Debug("files:LoadHelpAnimInfo", "'%s': %d, %d, %d => %d",
	  EL_NAME(helpanim_info[i].element),
	  helpanim_info[i].element,
	  helpanim_info[i].action,
	  helpanim_info[i].direction,
	  helpanim_info[i].delay);
#endif
}

void LoadHelpTextInfo(void)
{
  char *filename = getHelpTextFilename();
  int i;

  if (helptext_info != NULL)
  {
    freeSetupFileHash(helptext_info);
    helptext_info = NULL;
  }

  if (fileExists(filename))
    helptext_info = loadSetupFileHash(filename);

  if (helptext_info == NULL)
  {
    // use reliable default values from static configuration
    helptext_info = newSetupFileHash();

    for (i = 0; helptext_config[i].token; i++)
      setHashEntry(helptext_info,
		   helptext_config[i].token,
		   helptext_config[i].value);
  }

#if 0
  BEGIN_HASH_ITERATION(helptext_info, itr)
  {
    Debug("files:LoadHelpTextInfo", "'%s' => '%s'",
	  HASH_ITERATION_TOKEN(itr), HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(hash, itr)
#endif
}


// ----------------------------------------------------------------------------
// convert levels
// ----------------------------------------------------------------------------

#define MAX_NUM_CONVERT_LEVELS		1000

void ConvertLevels(void)
{
  static LevelDirTree *convert_leveldir = NULL;
  static int convert_level_nr = -1;
  static int num_levels_handled = 0;
  static int num_levels_converted = 0;
  static boolean levels_failed[MAX_NUM_CONVERT_LEVELS];
  int i;

  convert_leveldir = getTreeInfoFromIdentifier(leveldir_first,
					       global.convert_leveldir);

  if (convert_leveldir == NULL)
    Fail("no such level identifier: '%s'", global.convert_leveldir);

  leveldir_current = convert_leveldir;

  if (global.convert_level_nr != -1)
  {
    convert_leveldir->first_level = global.convert_level_nr;
    convert_leveldir->last_level  = global.convert_level_nr;
  }

  convert_level_nr = convert_leveldir->first_level;

  PrintLine("=", 79);
  Print("Converting levels\n");
  PrintLine("-", 79);
  Print("Level series identifier: '%s'\n", convert_leveldir->identifier);
  Print("Level series name:       '%s'\n", convert_leveldir->name);
  Print("Level series author:     '%s'\n", convert_leveldir->author);
  Print("Number of levels:        %d\n",   convert_leveldir->levels);
  PrintLine("=", 79);
  Print("\n");

  for (i = 0; i < MAX_NUM_CONVERT_LEVELS; i++)
    levels_failed[i] = FALSE;

  while (convert_level_nr <= convert_leveldir->last_level)
  {
    char *level_filename;
    boolean new_level;

    level_nr = convert_level_nr++;

    Print("Level %03d: ", level_nr);

    LoadLevel(level_nr);
    if (level.no_level_file || level.no_valid_file)
    {
      Print("(no level)\n");
      continue;
    }

    Print("converting level ... ");

#if 0
    // special case: conversion of some EMC levels as requested by ACME
    level.game_engine_type = GAME_ENGINE_TYPE_RND;
#endif

    level_filename = getDefaultLevelFilename(level_nr);
    new_level = !fileExists(level_filename);

    if (new_level)
    {
      SaveLevel(level_nr);

      num_levels_converted++;

      Print("converted.\n");
    }
    else
    {
      if (level_nr >= 0 && level_nr < MAX_NUM_CONVERT_LEVELS)
	levels_failed[level_nr] = TRUE;

      Print("NOT CONVERTED -- LEVEL ALREADY EXISTS.\n");
    }

    num_levels_handled++;
  }

  Print("\n");
  PrintLine("=", 79);
  Print("Number of levels handled: %d\n", num_levels_handled);
  Print("Number of levels converted: %d (%d%%)\n", num_levels_converted,
	 (num_levels_handled ?
	  num_levels_converted * 100 / num_levels_handled : 0));
  PrintLine("-", 79);
  Print("Summary (for automatic parsing by scripts):\n");
  Print("LEVELDIR '%s', CONVERTED %d/%d (%d%%)",
	 convert_leveldir->identifier, num_levels_converted,
	 num_levels_handled,
	 (num_levels_handled ?
	  num_levels_converted * 100 / num_levels_handled : 0));

  if (num_levels_handled != num_levels_converted)
  {
    Print(", FAILED:");
    for (i = 0; i < MAX_NUM_CONVERT_LEVELS; i++)
      if (levels_failed[i])
	Print(" %03d", i);
  }

  Print("\n");
  PrintLine("=", 79);

  CloseAllAndExit(0);
}


// ----------------------------------------------------------------------------
// create and save images for use in level sketches (raw BMP format)
// ----------------------------------------------------------------------------

void CreateLevelSketchImages(void)
{
  Bitmap *bitmap1;
  Bitmap *bitmap2;
  int i;

  InitElementPropertiesGfxElement();

  bitmap1 = CreateBitmap(TILEX, TILEY, DEFAULT_DEPTH);
  bitmap2 = CreateBitmap(MINI_TILEX, MINI_TILEY, DEFAULT_DEPTH);

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    int element = getMappedElement(i);
    char basename1[16];
    char basename2[16];
    char *filename1;
    char *filename2;

    sprintf(basename1, "%04d.bmp", i);
    sprintf(basename2, "%04ds.bmp", i);

    filename1 = getPath2(global.create_sketch_images_dir, basename1);
    filename2 = getPath2(global.create_sketch_images_dir, basename2);

    DrawSizedElement(0, 0, element, TILESIZE);
    BlitBitmap(drawto, bitmap1, SX, SY, TILEX, TILEY, 0, 0);

    if (SDL_SaveBMP(bitmap1->surface, filename1) != 0)
      Fail("cannot save level sketch image file '%s'", filename1);

    DrawSizedElement(0, 0, element, MINI_TILESIZE);
    BlitBitmap(drawto, bitmap2, SX, SY, MINI_TILEX, MINI_TILEY, 0, 0);

    if (SDL_SaveBMP(bitmap2->surface, filename2) != 0)
      Fail("cannot save level sketch image file '%s'", filename2);

    free(filename1);
    free(filename2);

    // create corresponding SQL statements (for normal and small images)
    if (i < 1000)
    {
      printf("insert into phpbb_words values (NULL, '`%03d', '<IMG class=\"levelsketch\" src=\"/I/%04d.png\"/>');\n", i, i);
      printf("insert into phpbb_words values (NULL, '¸%03d', '<IMG class=\"levelsketch\" src=\"/I/%04ds.png\"/>');\n", i, i);
    }

    printf("insert into phpbb_words values (NULL, '`%04d', '<IMG class=\"levelsketch\" src=\"/I/%04d.png\"/>');\n", i, i);
    printf("insert into phpbb_words values (NULL, '¸%04d', '<IMG class=\"levelsketch\" src=\"/I/%04ds.png\"/>');\n", i, i);

    // optional: create content for forum level sketch demonstration post
    if (options.debug)
      fprintf(stderr, "%03d `%03d%c", i, i, (i % 10 < 9 ? ' ' : '\n'));
  }

  FreeBitmap(bitmap1);
  FreeBitmap(bitmap2);

  if (options.debug)
    fprintf(stderr, "\n");

  Info("%d normal and small images created", NUM_FILE_ELEMENTS);

  CloseAllAndExit(0);
}


// ----------------------------------------------------------------------------
// create and save images for element collecting animations (raw BMP format)
// ----------------------------------------------------------------------------

static boolean createCollectImage(int element)
{
  return (IS_COLLECTIBLE(element) && !IS_SP_ELEMENT(element));
}

void CreateCollectElementImages(void)
{
  int i, j;
  int num_steps = 8;
  int anim_frames = num_steps - 1;
  int tile_size = TILESIZE;
  int anim_width  = tile_size * anim_frames;
  int anim_height = tile_size;
  int num_collect_images = 0;
  int pos_collect_images = 0;

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (createCollectImage(i))
      num_collect_images++;

  Info("Creating %d element collecting animation images ...",
       num_collect_images);

  int dst_width  = anim_width * 2;
  int dst_height = anim_height * num_collect_images / 2;
  Bitmap *dst_bitmap = CreateBitmap(dst_width, dst_height, DEFAULT_DEPTH);
  char *basename_bmp = "RocksCollect.bmp";
  char *basename_png = "RocksCollect.png";
  char *filename_bmp = getPath2(global.create_collect_images_dir, basename_bmp);
  char *filename_png = getPath2(global.create_collect_images_dir, basename_png);
  int len_filename_bmp = strlen(filename_bmp);
  int len_filename_png = strlen(filename_png);
  int max_command_len = MAX_FILENAME_LEN + len_filename_bmp + len_filename_png;
  char cmd_convert[max_command_len];

  snprintf(cmd_convert, max_command_len, "convert \"%s\" \"%s\"",
	   filename_bmp,
	   filename_png);

  // force using RGBA surface for destination bitmap
  SDL_SetColorKey(dst_bitmap->surface, SET_TRANSPARENT_PIXEL,
		  SDL_MapRGB(dst_bitmap->surface->format, 0x00, 0x00, 0x00));

  dst_bitmap->surface =
    SDL_ConvertSurfaceFormat(dst_bitmap->surface, SDL_PIXELFORMAT_ARGB8888, 0);

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (!createCollectImage(i))
      continue;

    int dst_x = (pos_collect_images / (num_collect_images / 2)) * anim_width;
    int dst_y = (pos_collect_images % (num_collect_images / 2)) * anim_height;
    int graphic = el2img(i);
    char *token_name = element_info[i].token_name;
    Bitmap *tmp_bitmap = CreateBitmap(tile_size, tile_size, DEFAULT_DEPTH);
    Bitmap *src_bitmap;
    int src_x, src_y;

    Info("- creating collecting image for '%s' ...", token_name);

    getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

    BlitBitmap(src_bitmap, tmp_bitmap, src_x, src_y,
	       tile_size, tile_size, 0, 0);

    // force using RGBA surface for temporary bitmap (using transparent black)
    SDL_SetColorKey(tmp_bitmap->surface, SET_TRANSPARENT_PIXEL,
		    SDL_MapRGB(tmp_bitmap->surface->format, 0x00, 0x00, 0x00));

    tmp_bitmap->surface =
      SDL_ConvertSurfaceFormat(tmp_bitmap->surface, SDL_PIXELFORMAT_ARGB8888, 0);

    tmp_bitmap->surface_masked = tmp_bitmap->surface;

    for (j = 0; j < anim_frames; j++)
    {
      int frame_size_final = tile_size * (anim_frames - j) / num_steps;
      int frame_size = frame_size_final * num_steps;
      int offset = (tile_size - frame_size_final) / 2;
      Bitmap *frame_bitmap = ZoomBitmap(tmp_bitmap, frame_size, frame_size);

      while (frame_size > frame_size_final)
      {
	frame_size /= 2;

	Bitmap *half_bitmap = ZoomBitmap(frame_bitmap, frame_size, frame_size);

	FreeBitmap(frame_bitmap);

	frame_bitmap = half_bitmap;
      }

      BlitBitmapMasked(frame_bitmap, dst_bitmap, 0, 0,
		       frame_size_final, frame_size_final,
		       dst_x + j * tile_size + offset, dst_y + offset);

      FreeBitmap(frame_bitmap);
    }

    tmp_bitmap->surface_masked = NULL;

    FreeBitmap(tmp_bitmap);

    pos_collect_images++;
  }

  if (SDL_SaveBMP(dst_bitmap->surface, filename_bmp) != 0)
    Fail("cannot save element collecting image file '%s'", filename_bmp);

  FreeBitmap(dst_bitmap);

  Info("Converting image file from BMP to PNG ...");

  if (system(cmd_convert) != 0)
    Fail("converting image file failed");

  unlink(filename_bmp);

  Info("Done.");

  CloseAllAndExit(0);
}


// ----------------------------------------------------------------------------
// create and save images for custom and group elements (raw BMP format)
// ----------------------------------------------------------------------------

void CreateCustomElementImages(char *directory)
{
  char *src_basename = "RocksCE-template.ilbm";
  char *dst_basename = "RocksCE.bmp";
  char *src_filename = getPath2(directory, src_basename);
  char *dst_filename = getPath2(directory, dst_basename);
  Bitmap *src_bitmap;
  Bitmap *bitmap;
  int yoffset_ce = 0;
  int yoffset_ge = (TILEY * NUM_CUSTOM_ELEMENTS / 16);
  int i;

  InitVideoDefaults();

  ReCreateBitmap(&backbuffer, video.width, video.height);

  src_bitmap = LoadImage(src_filename);

  bitmap = CreateBitmap(TILEX * 16 * 2,
			TILEY * (NUM_CUSTOM_ELEMENTS + NUM_GROUP_ELEMENTS) / 16,
			DEFAULT_DEPTH);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int x = i % 16;
    int y = i / 16;
    int ii = i + 1;
    int j;

    BlitBitmap(src_bitmap, bitmap, 0, 0, TILEX, TILEY,
	       TILEX * x, TILEY * y + yoffset_ce);

    BlitBitmap(src_bitmap, bitmap, 0, TILEY,
	       TILEX, TILEY,
	       TILEX * x + TILEX * 16,
	       TILEY * y + yoffset_ce);

    for (j = 2; j >= 0; j--)
    {
      int c = ii % 10;

      BlitBitmap(src_bitmap, bitmap,
		 TILEX + c * 7, 0, 6, 10,
		 TILEX * x + 6 + j * 7,
		 TILEY * y + 11 + yoffset_ce);

      BlitBitmap(src_bitmap, bitmap,
		 TILEX + c * 8, TILEY, 6, 10,
		 TILEX * 16 + TILEX * x + 6 + j * 8,
		 TILEY * y + 10 + yoffset_ce);

      ii /= 10;
    }
  }

  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
  {
    int x = i % 16;
    int y = i / 16;
    int ii = i + 1;
    int j;

    BlitBitmap(src_bitmap, bitmap, 0, 0, TILEX, TILEY,
	       TILEX * x, TILEY * y + yoffset_ge);

    BlitBitmap(src_bitmap, bitmap, 0, TILEY,
	       TILEX, TILEY,
	       TILEX * x + TILEX * 16,
	       TILEY * y + yoffset_ge);

    for (j = 1; j >= 0; j--)
    {
      int c = ii % 10;

      BlitBitmap(src_bitmap, bitmap, TILEX + c * 10, 11, 10, 10,
		 TILEX * x + 6 + j * 10,
		 TILEY * y + 11 + yoffset_ge);

      BlitBitmap(src_bitmap, bitmap,
		 TILEX + c * 8, TILEY + 12, 6, 10,
		 TILEX * 16 + TILEX * x + 10 + j * 8,
		 TILEY * y + 10 + yoffset_ge);

      ii /= 10;
    }
  }

  if (SDL_SaveBMP(bitmap->surface, dst_filename) != 0)
    Fail("cannot save CE graphics file '%s'", dst_filename);

  FreeBitmap(bitmap);

  CloseAllAndExit(0);
}
