// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// game.c
// ============================================================================

#include "libgame/libgame.h"

#include "game.h"
#include "init.h"
#include "tools.h"
#include "screens.h"
#include "events.h"
#include "files.h"
#include "tape.h"
#include "network.h"
#include "anim.h"


// DEBUG SETTINGS
#define DEBUG_INIT_PLAYER	1
#define DEBUG_PLAYER_ACTIONS	0

// EXPERIMENTAL STUFF
#define USE_NEW_AMOEBA_CODE	FALSE

// EXPERIMENTAL STUFF
#define USE_QUICKSAND_BD_ROCK_BUGFIX	0
#define USE_QUICKSAND_IMPACT_BUGFIX	0
#define USE_DELAYED_GFX_REDRAW		0
#define USE_NEW_PLAYER_ASSIGNMENTS	1

#if USE_DELAYED_GFX_REDRAW
#define TEST_DrawLevelField(x, y)				\
	GfxRedraw[x][y] |= GFX_REDRAW_TILE
#define TEST_DrawLevelFieldCrumbled(x, y)			\
	GfxRedraw[x][y] |= GFX_REDRAW_TILE_CRUMBLED
#define TEST_DrawLevelFieldCrumbledNeighbours(x, y)		\
	GfxRedraw[x][y] |= GFX_REDRAW_TILE_CRUMBLED_NEIGHBOURS
#define TEST_DrawTwinkleOnField(x, y)				\
	GfxRedraw[x][y] |= GFX_REDRAW_TILE_TWINKLED
#else
#define TEST_DrawLevelField(x, y)				\
	     DrawLevelField(x, y)
#define TEST_DrawLevelFieldCrumbled(x, y)			\
	     DrawLevelFieldCrumbled(x, y)
#define TEST_DrawLevelFieldCrumbledNeighbours(x, y)		\
	     DrawLevelFieldCrumbledNeighbours(x, y)
#define TEST_DrawTwinkleOnField(x, y)				\
	     DrawTwinkleOnField(x, y)
#endif


// for DigField()
#define DF_NO_PUSH		0
#define DF_DIG			1
#define DF_SNAP			2

// for MovePlayer()
#define MP_NO_ACTION		0
#define MP_MOVING		1
#define MP_ACTION		2
#define MP_DONT_RUN_INTO	(MP_MOVING | MP_ACTION)

// for ScrollPlayer()
#define SCROLL_INIT		0
#define SCROLL_GO_ON		1

// for Bang()/Explode()
#define EX_PHASE_START		0
#define EX_TYPE_NONE		0
#define EX_TYPE_NORMAL		(1 << 0)
#define EX_TYPE_CENTER		(1 << 1)
#define EX_TYPE_BORDER		(1 << 2)
#define EX_TYPE_CROSS		(1 << 3)
#define EX_TYPE_DYNA		(1 << 4)
#define EX_TYPE_SINGLE_TILE	(EX_TYPE_CENTER | EX_TYPE_BORDER)

#define PANEL_OFF()		(game.panel.active == FALSE)
#define	PANEL_DEACTIVATED(p)	((p)->x < 0 || (p)->y < 0 || PANEL_OFF())
#define PANEL_SHOW_EXTRA()	(setup.show_extra_panel_items)
#define PANEL_TOGGLE_EXTRA()	(game.panel.show_extra_items && !setup.show_extra_panel_items)
#define PANEL_XOFFSET_2ND(p)	(PANEL_SHOW_EXTRA() ? (p)->xoffset2 : 0)
#define PANEL_YOFFSET_2ND(p)	(PANEL_SHOW_EXTRA() ? (p)->yoffset2 : 0)
#define PANEL_XOFFSET_TOGGLE(p)	(PANEL_TOGGLE_EXTRA() ? (p)->xoffset : 0)
#define PANEL_YOFFSET_TOGGLE(p)	(PANEL_TOGGLE_EXTRA() ? (p)->yoffset : 0)
#define PANEL_XOFFSETS(p)	(PANEL_XOFFSET_2ND(p) + PANEL_XOFFSET_TOGGLE(p))
#define PANEL_YOFFSETS(p)	(PANEL_YOFFSET_2ND(p) + PANEL_YOFFSET_TOGGLE(p))
#define PANEL_XPOS(p)		(DX + ALIGNED_TEXT_XPOS(p) + PANEL_XOFFSETS(p))
#define PANEL_YPOS(p)		(DY + ALIGNED_TEXT_YPOS(p) + PANEL_YOFFSETS(p))

// game panel display and control definitions
#define GAME_PANEL_LEVEL_NUMBER			0
#define GAME_PANEL_GEMS				1
#define GAME_PANEL_GEMS_NEEDED			2
#define GAME_PANEL_GEMS_COLLECTED		3
#define GAME_PANEL_GEMS_SCORE			4
#define GAME_PANEL_INVENTORY_COUNT		5
#define GAME_PANEL_INVENTORY_FIRST_1		6
#define GAME_PANEL_INVENTORY_FIRST_2		7
#define GAME_PANEL_INVENTORY_FIRST_3		8
#define GAME_PANEL_INVENTORY_FIRST_4		9
#define GAME_PANEL_INVENTORY_FIRST_5		10
#define GAME_PANEL_INVENTORY_FIRST_6		11
#define GAME_PANEL_INVENTORY_FIRST_7		12
#define GAME_PANEL_INVENTORY_FIRST_8		13
#define GAME_PANEL_INVENTORY_LAST_1		14
#define GAME_PANEL_INVENTORY_LAST_2		15
#define GAME_PANEL_INVENTORY_LAST_3		16
#define GAME_PANEL_INVENTORY_LAST_4		17
#define GAME_PANEL_INVENTORY_LAST_5		18
#define GAME_PANEL_INVENTORY_LAST_6		19
#define GAME_PANEL_INVENTORY_LAST_7		20
#define GAME_PANEL_INVENTORY_LAST_8		21
#define GAME_PANEL_KEY_1			22
#define GAME_PANEL_KEY_2			23
#define GAME_PANEL_KEY_3			24
#define GAME_PANEL_KEY_4			25
#define GAME_PANEL_KEY_5			26
#define GAME_PANEL_KEY_6			27
#define GAME_PANEL_KEY_7			28
#define GAME_PANEL_KEY_8			29
#define GAME_PANEL_KEY_WHITE			30
#define GAME_PANEL_KEY_WHITE_COUNT		31
#define GAME_PANEL_SCORE			32
#define GAME_PANEL_HIGHSCORE			33
#define GAME_PANEL_TIME				34
#define GAME_PANEL_TIME_HH			35
#define GAME_PANEL_TIME_MM			36
#define GAME_PANEL_TIME_SS			37
#define GAME_PANEL_TIME_ANIM			38
#define GAME_PANEL_HEALTH			39
#define GAME_PANEL_HEALTH_ANIM			40
#define GAME_PANEL_FRAME			41
#define GAME_PANEL_SHIELD_NORMAL		42
#define GAME_PANEL_SHIELD_NORMAL_TIME		43
#define GAME_PANEL_SHIELD_DEADLY		44
#define GAME_PANEL_SHIELD_DEADLY_TIME		45
#define GAME_PANEL_EXIT				46
#define GAME_PANEL_EMC_MAGIC_BALL		47
#define GAME_PANEL_EMC_MAGIC_BALL_SWITCH	48
#define GAME_PANEL_LIGHT_SWITCH			49
#define GAME_PANEL_LIGHT_SWITCH_TIME		50
#define GAME_PANEL_TIMEGATE_SWITCH		51
#define GAME_PANEL_TIMEGATE_SWITCH_TIME		52
#define GAME_PANEL_SWITCHGATE_SWITCH		53
#define GAME_PANEL_EMC_LENSES			54
#define GAME_PANEL_EMC_LENSES_TIME		55
#define GAME_PANEL_EMC_MAGNIFIER		56
#define GAME_PANEL_EMC_MAGNIFIER_TIME		57
#define GAME_PANEL_BALLOON_SWITCH		58
#define GAME_PANEL_DYNABOMB_NUMBER		59
#define GAME_PANEL_DYNABOMB_SIZE		60
#define GAME_PANEL_DYNABOMB_POWER		61
#define GAME_PANEL_PENGUINS			62
#define GAME_PANEL_SOKOBAN_OBJECTS		63
#define GAME_PANEL_SOKOBAN_FIELDS		64
#define GAME_PANEL_ROBOT_WHEEL			65
#define GAME_PANEL_CONVEYOR_BELT_1		66
#define GAME_PANEL_CONVEYOR_BELT_2		67
#define GAME_PANEL_CONVEYOR_BELT_3		68
#define GAME_PANEL_CONVEYOR_BELT_4		69
#define GAME_PANEL_CONVEYOR_BELT_1_SWITCH	70
#define GAME_PANEL_CONVEYOR_BELT_2_SWITCH	71
#define GAME_PANEL_CONVEYOR_BELT_3_SWITCH	72
#define GAME_PANEL_CONVEYOR_BELT_4_SWITCH	73
#define GAME_PANEL_MAGIC_WALL			74
#define GAME_PANEL_MAGIC_WALL_TIME		75
#define GAME_PANEL_GRAVITY_STATE		76
#define GAME_PANEL_GRAPHIC_1			77
#define GAME_PANEL_GRAPHIC_2			78
#define GAME_PANEL_GRAPHIC_3			79
#define GAME_PANEL_GRAPHIC_4			80
#define GAME_PANEL_GRAPHIC_5			81
#define GAME_PANEL_GRAPHIC_6			82
#define GAME_PANEL_GRAPHIC_7			83
#define GAME_PANEL_GRAPHIC_8			84
#define GAME_PANEL_ELEMENT_1			85
#define GAME_PANEL_ELEMENT_2			86
#define GAME_PANEL_ELEMENT_3			87
#define GAME_PANEL_ELEMENT_4			88
#define GAME_PANEL_ELEMENT_5			89
#define GAME_PANEL_ELEMENT_6			90
#define GAME_PANEL_ELEMENT_7			91
#define GAME_PANEL_ELEMENT_8			92
#define GAME_PANEL_ELEMENT_COUNT_1		93
#define GAME_PANEL_ELEMENT_COUNT_2		94
#define GAME_PANEL_ELEMENT_COUNT_3		95
#define GAME_PANEL_ELEMENT_COUNT_4		96
#define GAME_PANEL_ELEMENT_COUNT_5		97
#define GAME_PANEL_ELEMENT_COUNT_6		98
#define GAME_PANEL_ELEMENT_COUNT_7		99
#define GAME_PANEL_ELEMENT_COUNT_8		100
#define GAME_PANEL_CE_SCORE_1			101
#define GAME_PANEL_CE_SCORE_2			102
#define GAME_PANEL_CE_SCORE_3			103
#define GAME_PANEL_CE_SCORE_4			104
#define GAME_PANEL_CE_SCORE_5			105
#define GAME_PANEL_CE_SCORE_6			106
#define GAME_PANEL_CE_SCORE_7			107
#define GAME_PANEL_CE_SCORE_8			108
#define GAME_PANEL_CE_SCORE_1_ELEMENT		109
#define GAME_PANEL_CE_SCORE_2_ELEMENT		110
#define GAME_PANEL_CE_SCORE_3_ELEMENT		111
#define GAME_PANEL_CE_SCORE_4_ELEMENT		112
#define GAME_PANEL_CE_SCORE_5_ELEMENT		113
#define GAME_PANEL_CE_SCORE_6_ELEMENT		114
#define GAME_PANEL_CE_SCORE_7_ELEMENT		115
#define GAME_PANEL_CE_SCORE_8_ELEMENT		116
#define GAME_PANEL_BDX_LIVES			117
#define GAME_PANEL_BDX_KEY_1			118
#define GAME_PANEL_BDX_KEY_2			119
#define GAME_PANEL_BDX_KEY_3			120
#define GAME_PANEL_BDX_KEY_1_COUNT		121
#define GAME_PANEL_BDX_KEY_2_COUNT		122
#define GAME_PANEL_BDX_KEY_3_COUNT		123
#define GAME_PANEL_BDX_DIAMOND_KEY		124
#define GAME_PANEL_BDX_GRAVITY			125
#define GAME_PANEL_BDX_GRAVITY_NEXT		126
#define GAME_PANEL_BDX_GRAVITY_TIME		127
#define GAME_PANEL_BDX_GRAVITY_STATE		128
#define GAME_PANEL_BDX_SKELETON			129
#define GAME_PANEL_BDX_SKELETON_COUNT		130
#define GAME_PANEL_BDX_SWEET			131
#define GAME_PANEL_BDX_PNEUMATIC_HAMMER		132
#define GAME_PANEL_BDX_ROCKET_COUNT		133
#define GAME_PANEL_BDX_ROCKET_STATE		134
#define GAME_PANEL_BDX_MAGIC_WALL		135
#define GAME_PANEL_BDX_MAGIC_WALL_TIME		136
#define GAME_PANEL_BDX_CREATURE_SWITCH		137
#define GAME_PANEL_BDX_EXPANDABLE_WALL_SWITCH	138
#define GAME_PANEL_BDX_BITER_SWITCH_TIME	139
#define GAME_PANEL_BDX_REPLICATOR		140
#define GAME_PANEL_BDX_REPLICATOR_SWITCH	141
#define GAME_PANEL_BDX_CONVEYOR_LEFT		142
#define GAME_PANEL_BDX_CONVEYOR_RIGHT		143
#define GAME_PANEL_BDX_CONVEYOR_SWITCH		144
#define GAME_PANEL_BDX_CONVEYOR_DIR_SWITCH	145
#define GAME_PANEL_PLAYER_NAME			146
#define GAME_PANEL_LEVELSET_NAME		147
#define GAME_PANEL_LEVEL_NAME			148
#define GAME_PANEL_LEVEL_AUTHOR			149

#define NUM_GAME_PANEL_CONTROLS			150

struct GamePanelOrderInfo
{
  int nr;
  int sort_priority;
};

static struct GamePanelOrderInfo game_panel_order[NUM_GAME_PANEL_CONTROLS];

struct GamePanelControlInfo
{
  int nr;

  struct TextPosInfo *pos;
  int type;

  int graphic, graphic_active;

  int value, last_value;
  int frame, last_frame;
  int gfx_frame;
  int gfx_random;
};

static struct GamePanelControlInfo game_panel_controls[] =
{
  {
    GAME_PANEL_LEVEL_NUMBER,
    &game.panel.level_number,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_GEMS,
    &game.panel.gems,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_GEMS_NEEDED,
    &game.panel.gems_needed,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_GEMS_COLLECTED,
    &game.panel.gems_collected,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_GEMS_SCORE,
    &game.panel.gems_score,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_INVENTORY_COUNT,
    &game.panel.inventory_count,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_1,
    &game.panel.inventory_first[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_2,
    &game.panel.inventory_first[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_3,
    &game.panel.inventory_first[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_4,
    &game.panel.inventory_first[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_5,
    &game.panel.inventory_first[4],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_6,
    &game.panel.inventory_first[5],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_7,
    &game.panel.inventory_first[6],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_FIRST_8,
    &game.panel.inventory_first[7],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_1,
    &game.panel.inventory_last[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_2,
    &game.panel.inventory_last[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_3,
    &game.panel.inventory_last[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_4,
    &game.panel.inventory_last[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_5,
    &game.panel.inventory_last[4],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_6,
    &game.panel.inventory_last[5],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_7,
    &game.panel.inventory_last[6],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_INVENTORY_LAST_8,
    &game.panel.inventory_last[7],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_1,
    &game.panel.key[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_2,
    &game.panel.key[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_3,
    &game.panel.key[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_4,
    &game.panel.key[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_5,
    &game.panel.key[4],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_6,
    &game.panel.key[5],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_7,
    &game.panel.key[6],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_8,
    &game.panel.key[7],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_WHITE,
    &game.panel.key_white,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_KEY_WHITE_COUNT,
    &game.panel.key_white_count,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_SCORE,
    &game.panel.score,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_HIGHSCORE,
    &game.panel.highscore,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_TIME,
    &game.panel.time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_TIME_HH,
    &game.panel.time_hh,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_TIME_MM,
    &game.panel.time_mm,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_TIME_SS,
    &game.panel.time_ss,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_TIME_ANIM,
    &game.panel.time_anim,
    TYPE_GRAPHIC,

    IMG_GFX_GAME_PANEL_TIME_ANIM,
    IMG_GFX_GAME_PANEL_TIME_ANIM_ACTIVE
  },
  {
    GAME_PANEL_HEALTH,
    &game.panel.health,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_HEALTH_ANIM,
    &game.panel.health_anim,
    TYPE_GRAPHIC,

    IMG_GFX_GAME_PANEL_HEALTH_ANIM,
    IMG_GFX_GAME_PANEL_HEALTH_ANIM_ACTIVE
  },
  {
    GAME_PANEL_FRAME,
    &game.panel.frame,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_SHIELD_NORMAL,
    &game.panel.shield_normal,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_SHIELD_NORMAL_TIME,
    &game.panel.shield_normal_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_SHIELD_DEADLY,
    &game.panel.shield_deadly,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_SHIELD_DEADLY_TIME,
    &game.panel.shield_deadly_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_EXIT,
    &game.panel.exit,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_EMC_MAGIC_BALL,
    &game.panel.emc_magic_ball,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_EMC_MAGIC_BALL_SWITCH,
    &game.panel.emc_magic_ball_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_LIGHT_SWITCH,
    &game.panel.light_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_LIGHT_SWITCH_TIME,
    &game.panel.light_switch_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_TIMEGATE_SWITCH,
    &game.panel.timegate_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_TIMEGATE_SWITCH_TIME,
    &game.panel.timegate_switch_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_SWITCHGATE_SWITCH,
    &game.panel.switchgate_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_EMC_LENSES,
    &game.panel.emc_lenses,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_EMC_LENSES_TIME,
    &game.panel.emc_lenses_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_EMC_MAGNIFIER,
    &game.panel.emc_magnifier,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_EMC_MAGNIFIER_TIME,
    &game.panel.emc_magnifier_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BALLOON_SWITCH,
    &game.panel.balloon_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_DYNABOMB_NUMBER,
    &game.panel.dynabomb_number,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_DYNABOMB_SIZE,
    &game.panel.dynabomb_size,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_DYNABOMB_POWER,
    &game.panel.dynabomb_power,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_PENGUINS,
    &game.panel.penguins,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_SOKOBAN_OBJECTS,
    &game.panel.sokoban_objects,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_SOKOBAN_FIELDS,
    &game.panel.sokoban_fields,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ROBOT_WHEEL,
    &game.panel.robot_wheel,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_1,
    &game.panel.conveyor_belt[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_2,
    &game.panel.conveyor_belt[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_3,
    &game.panel.conveyor_belt[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_4,
    &game.panel.conveyor_belt[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_1_SWITCH,
    &game.panel.conveyor_belt_switch[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_2_SWITCH,
    &game.panel.conveyor_belt_switch[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_3_SWITCH,
    &game.panel.conveyor_belt_switch[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CONVEYOR_BELT_4_SWITCH,
    &game.panel.conveyor_belt_switch[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_MAGIC_WALL,
    &game.panel.magic_wall,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_MAGIC_WALL_TIME,
    &game.panel.magic_wall_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_GRAVITY_STATE,
    &game.panel.gravity_state,
    TYPE_STRING,
  },
  {
    GAME_PANEL_GRAPHIC_1,
    &game.panel.graphic[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_2,
    &game.panel.graphic[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_3,
    &game.panel.graphic[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_4,
    &game.panel.graphic[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_5,
    &game.panel.graphic[4],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_6,
    &game.panel.graphic[5],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_7,
    &game.panel.graphic[6],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_GRAPHIC_8,
    &game.panel.graphic[7],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_1,
    &game.panel.element[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_2,
    &game.panel.element[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_3,
    &game.panel.element[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_4,
    &game.panel.element[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_5,
    &game.panel.element[4],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_6,
    &game.panel.element[5],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_7,
    &game.panel.element[6],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_8,
    &game.panel.element[7],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_1,
    &game.panel.element_count[0],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_2,
    &game.panel.element_count[1],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_3,
    &game.panel.element_count[2],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_4,
    &game.panel.element_count[3],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_5,
    &game.panel.element_count[4],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_6,
    &game.panel.element_count[5],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_7,
    &game.panel.element_count[6],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_ELEMENT_COUNT_8,
    &game.panel.element_count[7],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_1,
    &game.panel.ce_score[0],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_2,
    &game.panel.ce_score[1],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_3,
    &game.panel.ce_score[2],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_4,
    &game.panel.ce_score[3],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_5,
    &game.panel.ce_score[4],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_6,
    &game.panel.ce_score[5],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_7,
    &game.panel.ce_score[6],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_8,
    &game.panel.ce_score[7],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_CE_SCORE_1_ELEMENT,
    &game.panel.ce_score_element[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_2_ELEMENT,
    &game.panel.ce_score_element[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_3_ELEMENT,
    &game.panel.ce_score_element[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_4_ELEMENT,
    &game.panel.ce_score_element[3],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_5_ELEMENT,
    &game.panel.ce_score_element[4],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_6_ELEMENT,
    &game.panel.ce_score_element[5],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_7_ELEMENT,
    &game.panel.ce_score_element[6],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_CE_SCORE_8_ELEMENT,
    &game.panel.ce_score_element[7],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_LIVES,
    &game.panel.bdx_lives,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_KEY_1,
    &game.panel.bdx_key[0],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_KEY_2,
    &game.panel.bdx_key[1],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_KEY_3,
    &game.panel.bdx_key[2],
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_KEY_1_COUNT,
    &game.panel.bdx_key_count[0],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_KEY_2_COUNT,
    &game.panel.bdx_key_count[1],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_KEY_3_COUNT,
    &game.panel.bdx_key_count[2],
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_DIAMOND_KEY,
    &game.panel.bdx_diamond_key,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_GRAVITY,
    &game.panel.bdx_gravity,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_GRAVITY_NEXT,
    &game.panel.bdx_gravity_next,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_GRAVITY_TIME,
    &game.panel.bdx_gravity_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_GRAVITY_STATE,
    &game.panel.bdx_gravity_state,
    TYPE_STRING,
  },
  {
    GAME_PANEL_BDX_SKELETON,
    &game.panel.bdx_skeleton,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_SKELETON_COUNT,
    &game.panel.bdx_skeleton_count,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_SWEET,
    &game.panel.bdx_sweet,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_PNEUMATIC_HAMMER,
    &game.panel.bdx_pneumatic_hammer,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_ROCKET_COUNT,
    &game.panel.bdx_rocket_count,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_ROCKET_STATE,
    &game.panel.bdx_rocket_state,
    TYPE_STRING,
  },
  {
    GAME_PANEL_BDX_MAGIC_WALL,
    &game.panel.bdx_magic_wall,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_MAGIC_WALL_TIME,
    &game.panel.bdx_magic_wall_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_CREATURE_SWITCH,
    &game.panel.bdx_creature_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_EXPANDABLE_WALL_SWITCH,
    &game.panel.bdx_expandable_wall_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_BITER_SWITCH_TIME,
    &game.panel.bdx_biter_switch_time,
    TYPE_INTEGER,
  },
  {
    GAME_PANEL_BDX_REPLICATOR,
    &game.panel.bdx_replicator,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_REPLICATOR_SWITCH,
    &game.panel.bdx_replicator_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_CONVEYOR_LEFT,
    &game.panel.bdx_conveyor_left,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_CONVEYOR_RIGHT,
    &game.panel.bdx_conveyor_right,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_CONVEYOR_SWITCH,
    &game.panel.bdx_conveyor_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_BDX_CONVEYOR_DIR_SWITCH,
    &game.panel.bdx_conveyor_dir_switch,
    TYPE_ELEMENT,
  },
  {
    GAME_PANEL_PLAYER_NAME,
    &game.panel.player_name,
    TYPE_STRING,
  },
  {
    GAME_PANEL_LEVELSET_NAME,
    &game.panel.levelset_name,
    TYPE_STRING,
  },
  {
    GAME_PANEL_LEVEL_NAME,
    &game.panel.level_name,
    TYPE_STRING,
  },
  {
    GAME_PANEL_LEVEL_AUTHOR,
    &game.panel.level_author,
    TYPE_STRING,
  },

  {
    -1,
    NULL,
    -1,
  }
};

// values for delayed check of falling and moving elements and for collision
#define CHECK_DELAY_MOVING	3
#define CHECK_DELAY_FALLING	CHECK_DELAY_MOVING
#define CHECK_DELAY_COLLISION	2
#define CHECK_DELAY_IMPACT	CHECK_DELAY_COLLISION

// values for initial player move delay (initial delay counter value)
#define INITIAL_MOVE_DELAY_OFF	-1
#define INITIAL_MOVE_DELAY_ON	0

// values for player movement speed (which is in fact a delay value)
#define MOVE_DELAY_MIN_SPEED	32
#define MOVE_DELAY_NORMAL_SPEED	8
#define MOVE_DELAY_HIGH_SPEED	4
#define MOVE_DELAY_MAX_SPEED	1

#define DOUBLE_MOVE_DELAY(x)	(x = (x < MOVE_DELAY_MIN_SPEED ? x * 2 : x))
#define HALVE_MOVE_DELAY(x)	(x = (x > MOVE_DELAY_MAX_SPEED ? x / 2 : x))

#define DOUBLE_PLAYER_SPEED(p)	(HALVE_MOVE_DELAY( (p)->move_delay_value))
#define HALVE_PLAYER_SPEED(p)	(DOUBLE_MOVE_DELAY((p)->move_delay_value))

// values for scroll positions
#define SCROLL_POSITION_X(x)	((x) < SBX_Left  + MIDPOSX ? SBX_Left :	\
				 (x) > SBX_Right + MIDPOSX ? SBX_Right :\
				 (x) - MIDPOSX)
#define SCROLL_POSITION_Y(y)	((y) < SBY_Upper + MIDPOSY ? SBY_Upper :\
				 (y) > SBY_Lower + MIDPOSY ? SBY_Lower :\
				 (y) - MIDPOSY)

// values for other actions
#define MOVE_STEPSIZE_NORMAL	(TILEX / MOVE_DELAY_NORMAL_SPEED)
#define MOVE_STEPSIZE_MIN	(1)
#define MOVE_STEPSIZE_MAX	(TILEX)

#define GET_DX_FROM_DIR(d)	((d) == MV_LEFT ? -1 : (d) == MV_RIGHT ? 1 : 0)
#define GET_DY_FROM_DIR(d)	((d) == MV_UP   ? -1 : (d) == MV_DOWN  ? 1 : 0)

#define	INIT_GFX_RANDOM()	(GetSimpleRandom(1000000))

#define GET_NEW_PUSH_DELAY(e)	(   (element_info[e].push_delay_fixed) + \
				 RND(element_info[e].push_delay_random))
#define GET_NEW_DROP_DELAY(e)	(   (element_info[e].drop_delay_fixed) + \
				 RND(element_info[e].drop_delay_random))
#define GET_NEW_MOVE_DELAY(e)	(   (element_info[e].move_delay_fixed) + \
				 RND(element_info[e].move_delay_random))
#define GET_MAX_MOVE_DELAY(e)	(   (element_info[e].move_delay_fixed) + \
				    (element_info[e].move_delay_random))
#define GET_NEW_STEP_DELAY(e)	(   (element_info[e].step_delay_fixed) + \
				 RND(element_info[e].step_delay_random))
#define GET_MAX_STEP_DELAY(e)	(   (element_info[e].step_delay_fixed) + \
				    (element_info[e].step_delay_random))
#define GET_NEW_CE_VALUE(e)	(   (element_info[e].ce_value_fixed_initial) +\
				 RND(element_info[e].ce_value_random_initial))
#define GET_CE_SCORE(e)		(   (element_info[e].collect_score))
#define GET_CHANGE_DELAY(c)	(   ((c)->delay_fixed  * (c)->delay_frames) + \
				 RND((c)->delay_random * (c)->delay_frames))
#define GET_CE_DELAY_VALUE(c)	(   ((c)->delay_fixed) + \
				 RND((c)->delay_random))


#define GET_VALID_RUNTIME_ELEMENT(e)					\
	 ((e) >= NUM_RUNTIME_ELEMENTS ? EL_UNKNOWN : (e))

#define RESOLVED_REFERENCE_ELEMENT(be, e)				\
	((be) + (e) - EL_SELF < EL_CUSTOM_START ? EL_CUSTOM_START :	\
	 (be) + (e) - EL_SELF > EL_CUSTOM_END   ? EL_CUSTOM_END :	\
	 (be) + (e) - EL_SELF)

#define GET_PLAYER_FROM_BITS(p)						\
	(EL_PLAYER_1 + ((p) != PLAYER_BITS_ANY ? log_2(p) : 0))

#define GET_TARGET_ELEMENT(be, e, ch, cv, cs)				\
	((e) == EL_TRIGGER_PLAYER   ? (ch)->actual_trigger_player    :	\
	 (e) == EL_TRIGGER_ELEMENT  ? (ch)->actual_trigger_element   :	\
	 (e) == EL_TRIGGER_CE_VALUE ? (ch)->actual_trigger_ce_value  :	\
	 (e) == EL_TRIGGER_CE_SCORE ? (ch)->actual_trigger_ce_score  :	\
	 (e) == EL_CURRENT_CE_VALUE ? (cv) :				\
	 (e) == EL_CURRENT_CE_SCORE ? (cs) :				\
	 (e) >= EL_PREV_CE_8 && (e) <= EL_NEXT_CE_8 ?			\
	 RESOLVED_REFERENCE_ELEMENT(be, e) :				\
	 (e))

#define CAN_GROW_INTO(e)						\
	((e) == EL_SAND || (IS_DIGGABLE(e) && level.grow_into_diggable))

#define ELEMENT_CAN_ENTER_FIELD_BASE_X(x, y, condition)			\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Tile[x][y] == EL_ACID) ||	\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE_OR_PLAYER(x, y) ||	\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Tile[x][y] == EL_ACID) ||	\
					(condition)))

#define ELEMENT_CAN_ENTER_FIELD_BASE_4(e, x, y, condition)		\
		(IN_LEV_FIELD(x, y) && (IS_FREE(x, y) ||		\
					(condition) ||			\
					(CAN_MOVE_INTO_ACID(e) &&	\
					 Tile[x][y] == EL_ACID) ||	\
					(DONT_COLLIDE_WITH(e) &&	\
					 IS_PLAYER(x, y) &&		\
					 !PLAYER_ENEMY_PROTECTED(x, y))))

#define ELEMENT_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_4(e, x, y, 0)

#define SATELLITE_CAN_ENTER_FIELD(x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(EL_SATELLITE, x, y, 0)

#define ANDROID_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, Tile[x][y] == EL_EMC_PLANT)

#define ANDROID_CAN_CLONE_FIELD(x, y)					\
	(IN_LEV_FIELD(x, y) && (CAN_BE_CLONED_BY_ANDROID(Tile[x][y]) ||	\
				CAN_BE_CLONED_BY_ANDROID(EL_TRIGGER_ELEMENT)))

#define ENEMY_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, 0)

#define YAMYAM_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, Tile[x][y] == EL_DIAMOND)

#define DARK_YAMYAM_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, IS_FOOD_DARK_YAMYAM(Tile[x][y]))

#define PACMAN_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_3(e, x, y, IS_AMOEBOID(Tile[x][y]))

#define PIG_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, IS_FOOD_PIG(Tile[x][y]))

#define PENGUIN_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, (Tile[x][y] == EL_EXIT_OPEN || \
						 Tile[x][y] == EL_EM_EXIT_OPEN || \
						 Tile[x][y] == EL_STEEL_EXIT_OPEN || \
						 Tile[x][y] == EL_EM_STEEL_EXIT_OPEN || \
						 IS_FOOD_PENGUIN(Tile[x][y])))
#define DRAGON_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, 0)

#define MOLE_CAN_ENTER_FIELD(e, x, y, condition)			\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, (condition))

#define SPRING_CAN_ENTER_FIELD(e, x, y)					\
	ELEMENT_CAN_ENTER_FIELD_BASE_2(e, x, y, 0)

#define SPRING_CAN_BUMP_FROM_FIELD(x, y)				\
	(IN_LEV_FIELD(x, y) && (Tile[x][y] == EL_EMC_SPRING_BUMPER ||	\
				Tile[x][y] == EL_EMC_SPRING_BUMPER_ACTIVE))

#define MOVE_ENTER_EL(e)	(element_info[e].move_enter_element)

#define CE_ENTER_FIELD_COND(e, x, y)					\
		(!IS_PLAYER(x, y) &&					\
		 IS_EQUAL_OR_IN_GROUP(Tile[x][y], MOVE_ENTER_EL(e)))

#define CUSTOM_ELEMENT_CAN_ENTER_FIELD(e, x, y)				\
	ELEMENT_CAN_ENTER_FIELD_BASE_4(e, x, y, CE_ENTER_FIELD_COND(e, x, y))

#define IN_LEV_FIELD_AND_IS_FREE(x, y)  (IN_LEV_FIELD(x, y) &&  IS_FREE(x, y))
#define IN_LEV_FIELD_AND_NOT_FREE(x, y) (IN_LEV_FIELD(x, y) && !IS_FREE(x, y))

#define ACCESS_FROM(e, d)		(element_info[e].access_direction &(d))
#define IS_WALKABLE_FROM(e, d)		(IS_WALKABLE(e)   && ACCESS_FROM(e, d))
#define IS_PASSABLE_FROM(e, d)		(IS_PASSABLE(e)   && ACCESS_FROM(e, d))
#define IS_ACCESSIBLE_FROM(e, d)	(IS_ACCESSIBLE(e) && ACCESS_FROM(e, d))

#define MM_HEALTH(x)		(MIN(MAX(0, MAX_HEALTH - (x)), MAX_HEALTH))

// game button identifiers
#define GAME_CTRL_ID_STOP		0
#define GAME_CTRL_ID_PAUSE		1
#define GAME_CTRL_ID_PLAY		2
#define GAME_CTRL_ID_UNDO		3
#define GAME_CTRL_ID_REDO		4
#define GAME_CTRL_ID_SAVE		5
#define GAME_CTRL_ID_PAUSE2		6
#define GAME_CTRL_ID_LOAD		7
#define GAME_CTRL_ID_RESTART		8
#define GAME_CTRL_ID_PANEL_STOP		9
#define GAME_CTRL_ID_PANEL_PAUSE	10
#define GAME_CTRL_ID_PANEL_PLAY		11
#define GAME_CTRL_ID_PANEL_RESTART	12
#define GAME_CTRL_ID_TOUCH_STOP		13
#define GAME_CTRL_ID_TOUCH_PAUSE	14
#define GAME_CTRL_ID_TOUCH_RESTART	15
#define SOUND_CTRL_ID_MUSIC		16
#define SOUND_CTRL_ID_LOOPS		17
#define SOUND_CTRL_ID_SIMPLE		18
#define SOUND_CTRL_ID_PANEL_MUSIC	19
#define SOUND_CTRL_ID_PANEL_LOOPS	20
#define SOUND_CTRL_ID_PANEL_SIMPLE	21

#define NUM_GAME_BUTTONS		22


// forward declaration for internal use

static void CreateField(int, int, int);

static void ResetGfxAnimation(int, int);

static void SetPlayerWaiting(struct PlayerInfo *, boolean);
static void AdvanceFrameAndPlayerCounters(int);

static boolean MovePlayerOneStep(struct PlayerInfo *, int, int, int, int);
static boolean MovePlayer(struct PlayerInfo *, int, int);
static void ScrollPlayer(struct PlayerInfo *, int);
static void ScrollScreen(struct PlayerInfo *, int);

static int DigField(struct PlayerInfo *, int, int, int, int, int, int, int);
static boolean DigFieldByCE(int, int, int);
static boolean SnapField(struct PlayerInfo *, int, int);
static boolean DropElement(struct PlayerInfo *);

static void InitBeltMovement(void);
static void CloseAllOpenTimegates(void);
static void CheckGravityMovement(struct PlayerInfo *);
static void CheckGravityMovementWhenNotMoving(struct PlayerInfo *);
static void KillPlayerUnlessEnemyProtected(int, int);
static void KillPlayerUnlessExplosionProtected(int, int);

static void CheckNextToConditions(int, int);
static void TestIfPlayerNextToCustomElement(int, int);
static void TestIfPlayerTouchesCustomElement(int, int);
static void TestIfElementNextToCustomElement(int, int);
static void TestIfElementTouchesCustomElement(int, int);
static void TestIfElementHitsCustomElement(int, int, int);

static void HandleElementChange(int, int, int);
static void ExecuteCustomElementAction(int, int, int, int);
static boolean ChangeElement(int, int, int, int);

static boolean CheckTriggeredElementChangeExt(int, int, int, int, int, int, int);
#define CheckTriggeredElementChange(x, y, e, ev)			\
	CheckTriggeredElementChangeExt(x,y,e,ev, CH_PLAYER_ANY, CH_SIDE_ANY, -1)
#define CheckTriggeredElementChangeByPlayer(x, y, e, ev, p, s)		\
	CheckTriggeredElementChangeExt(x, y, e, ev, p, s, -1)
#define CheckTriggeredElementChangeBySide(x, y, e, ev, s)		\
	CheckTriggeredElementChangeExt(x, y, e, ev, CH_PLAYER_ANY, s, -1)
#define CheckTriggeredElementChangeByPage(x, y, e, ev, p)		\
	CheckTriggeredElementChangeExt(x,y,e,ev, CH_PLAYER_ANY, CH_SIDE_ANY, p)
#define CheckTriggeredElementChangeByMouse(x, y, e, ev, s)		\
	CheckTriggeredElementChangeExt(x, y, e, ev, CH_PLAYER_ANY, s, -1)

static boolean CheckElementChangeExt(int, int, int, int, int, int, int);
#define CheckElementChange(x, y, e, te, ev)				\
	CheckElementChangeExt(x, y, e, te, ev, CH_PLAYER_ANY, CH_SIDE_ANY)
#define CheckElementChangeByPlayer(x, y, e, ev, p, s)			\
	CheckElementChangeExt(x, y, e, EL_EMPTY, ev, p, s)
#define CheckElementChangeBySide(x, y, e, te, ev, s)			\
	CheckElementChangeExt(x, y, e, te, ev, CH_PLAYER_ANY, s)
#define CheckElementChangeByMouse(x, y, e, ev, s)			\
	CheckElementChangeExt(x, y, e, EL_UNDEFINED, ev, CH_PLAYER_ANY, s)

static void PlayLevelSound(int, int, int);
static void PlayLevelSoundNearest(int, int, int);
static void PlayLevelSoundAction(int, int, int);
static void PlayLevelSoundElementAction(int, int, int, int);
static void PlayLevelSoundElementActionIfLoop(int, int, int, int);
static void PlayLevelSoundActionIfLoop(int, int, int);
static void StopLevelSoundActionIfLoop(int, int, int);
static void PlayLevelMusic(void);
static void FadeLevelMusic(void);
static void FadeLevelSoundsAndMusic(void);

static void HandleGameButtons(struct GadgetInfo *);

int AmoebaNeighbourNr(int, int);
void AmoebaToDiamond(int, int);
void ContinueMoving(int, int);
void Bang(int, int);
void InitMovDir(int, int);
void InitAmoebaNr(int, int);
void NewHighScore(int, boolean);

void TestIfGoodThingHitsBadThing(int, int, int);
void TestIfBadThingHitsGoodThing(int, int, int);
void TestIfPlayerTouchesBadThing(int, int);
void TestIfPlayerRunsIntoBadThing(int, int, int);
void TestIfBadThingTouchesPlayer(int, int);
void TestIfBadThingRunsIntoPlayer(int, int, int);
void TestIfFriendTouchesBadThing(int, int);
void TestIfBadThingTouchesFriend(int, int);
void TestIfBadThingTouchesOtherBadThing(int, int);
void TestIfGoodThingGetsHitByBadThing(int, int, int);

void KillPlayer(struct PlayerInfo *);
void BuryPlayer(struct PlayerInfo *);
void RemovePlayer(struct PlayerInfo *);
void ExitPlayer(struct PlayerInfo *);

static int getInvisibleActiveFromInvisibleElement(int);
static int getInvisibleFromInvisibleActiveElement(int);

static void TestFieldAfterSnapping(int, int, int, int, int);

static struct GadgetInfo *game_gadget[NUM_GAME_BUTTONS];

// for detection of endless loops, caused by custom element programming
// (using maximal playfield width x 10 is just a rough approximation)
#define MAX_ELEMENT_CHANGE_RECURSION_DEPTH	(MAX_PLAYFIELD_WIDTH * 10)

#define RECURSION_LOOP_DETECTION_START(e, rc)				\
{									\
  if (recursion_loop_detected)						\
    return (rc);							\
									\
  if (recursion_loop_depth > MAX_ELEMENT_CHANGE_RECURSION_DEPTH)	\
  {									\
    recursion_loop_detected = TRUE;					\
    recursion_loop_element = (e);					\
  }									\
									\
  recursion_loop_depth++;						\
}

#define RECURSION_LOOP_DETECTION_END()					\
{									\
  recursion_loop_depth--;						\
}

static int recursion_loop_depth;
static boolean recursion_loop_detected;
static boolean recursion_loop_element;

static int map_player_action[MAX_PLAYERS];


// ----------------------------------------------------------------------------
// definition of elements that automatically change to other elements after
// a specified time, eventually calling a function when changing
// ----------------------------------------------------------------------------

// forward declaration for changer functions
static void InitBuggyBase(int, int);
static void WarnBuggyBase(int, int);

static void InitTrap(int, int);
static void ActivateTrap(int, int);
static void ChangeActiveTrap(int, int);

static void InitRobotWheel(int, int);
static void RunRobotWheel(int, int);
static void StopRobotWheel(int, int);

static void InitTimegateWheel(int, int);
static void RunTimegateWheel(int, int);

static void InitMagicBallDelay(int, int);
static void ActivateMagicBall(int, int);

struct ChangingElementInfo
{
  int element;
  int target_element;
  int change_delay;
  void (*pre_change_function)(int x, int y);
  void (*change_function)(int x, int y);
  void (*post_change_function)(int x, int y);
};

static struct ChangingElementInfo change_delay_list[] =
{
  {
    EL_NUT_BREAKING,
    EL_EMERALD,
    6,
    NULL,
    NULL,
    NULL
  },
  {
    EL_PEARL_BREAKING,
    EL_EMPTY,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EXIT_CLOSING,
    EL_EXIT_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_STEEL_EXIT_OPENING,
    EL_STEEL_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_STEEL_EXIT_CLOSING,
    EL_STEEL_EXIT_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EM_EXIT_OPENING,
    EL_EM_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EM_EXIT_CLOSING,
    EL_EMPTY,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EM_STEEL_EXIT_OPENING,
    EL_EM_STEEL_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_EM_STEEL_EXIT_CLOSING,
    EL_STEELWALL,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SP_EXIT_OPENING,
    EL_SP_EXIT_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SP_EXIT_CLOSING,
    EL_SP_EXIT_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SWITCHGATE_CLOSING,
    EL_SWITCHGATE_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_OPEN,
    29,
    NULL,
    NULL,
    NULL
  },
  {
    EL_TIMEGATE_CLOSING,
    EL_TIMEGATE_CLOSED,
    29,
    NULL,
    NULL,
    NULL
  },

  {
    EL_ACID_SPLASH_LEFT,
    EL_EMPTY,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_ACID_SPLASH_RIGHT,
    EL_EMPTY,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_SP_BUGGY_BASE,
    EL_SP_BUGGY_BASE_ACTIVATING,
    0,
    InitBuggyBase,
    NULL,
    NULL
  },
  {
    EL_SP_BUGGY_BASE_ACTIVATING,
    EL_SP_BUGGY_BASE_ACTIVE,
    0,
    InitBuggyBase,
    NULL,
    NULL
  },
  {
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_SP_BUGGY_BASE,
    0,
    InitBuggyBase,
    WarnBuggyBase,
    NULL
  },
  {
    EL_TRAP,
    EL_TRAP_ACTIVE,
    0,
    InitTrap,
    NULL,
    ActivateTrap
  },
  {
    EL_TRAP_ACTIVE,
    EL_TRAP,
    31,
    NULL,
    ChangeActiveTrap,
    NULL
  },
  {
    EL_ROBOT_WHEEL_ACTIVE,
    EL_ROBOT_WHEEL,
    0,
    InitRobotWheel,
    RunRobotWheel,
    StopRobotWheel
  },
  {
    EL_TIMEGATE_SWITCH_ACTIVE,
    EL_TIMEGATE_SWITCH,
    0,
    InitTimegateWheel,
    RunTimegateWheel,
    NULL
  },
  {
    EL_DC_TIMEGATE_SWITCH_ACTIVE,
    EL_DC_TIMEGATE_SWITCH,
    0,
    InitTimegateWheel,
    RunTimegateWheel,
    NULL
  },
  {
    EL_EMC_MAGIC_BALL_ACTIVE,
    EL_EMC_MAGIC_BALL_ACTIVE,
    0,
    InitMagicBallDelay,
    NULL,
    ActivateMagicBall
  },
  {
    EL_EMC_SPRING_BUMPER_ACTIVE,
    EL_EMC_SPRING_BUMPER,
    8,
    NULL,
    NULL,
    NULL
  },
  {
    EL_DIAGONAL_SHRINKING,
    EL_UNDEFINED,
    0,
    NULL,
    NULL,
    NULL
  },
  {
    EL_DIAGONAL_GROWING,
    EL_UNDEFINED,
    0,
    NULL,
    NULL,
    NULL,
  },

  {
    EL_UNDEFINED,
    EL_UNDEFINED,
    -1,
    NULL,
    NULL,
    NULL
  }
};

struct
{
  int element;
  int push_delay_fixed, push_delay_random;
}
push_delay_list[] =
{
  { EL_SPRING,			0, 0 },
  { EL_BALLOON,			0, 0 },

  { EL_SOKOBAN_OBJECT,		2, 0 },
  { EL_SOKOBAN_FIELD_FULL,	2, 0 },
  { EL_SATELLITE,		2, 0 },
  { EL_SP_DISK_YELLOW,		2, 0 },

  { EL_UNDEFINED,		0, 0 },
};

struct
{
  int element;
  int move_stepsize;
}
move_stepsize_list[] =
{
  { EL_AMOEBA_DROP,		2 },
  { EL_AMOEBA_DROPPING,		2 },
  { EL_QUICKSAND_FILLING,	1 },
  { EL_QUICKSAND_EMPTYING,	1 },
  { EL_QUICKSAND_FAST_FILLING,	2 },
  { EL_QUICKSAND_FAST_EMPTYING,	2 },
  { EL_MAGIC_WALL_FILLING,	2 },
  { EL_MAGIC_WALL_EMPTYING,	2 },
  { EL_BD_MAGIC_WALL_FILLING,	2 },
  { EL_BD_MAGIC_WALL_EMPTYING,	2 },
  { EL_DC_MAGIC_WALL_FILLING,	2 },
  { EL_DC_MAGIC_WALL_EMPTYING,	2 },

  { EL_UNDEFINED,		0 },
};

struct
{
  int element;
  int count;
}
collect_count_list[] =
{
  { EL_EMERALD,			1 },
  { EL_BD_DIAMOND,		1 },
  { EL_EMERALD_YELLOW,		1 },
  { EL_EMERALD_RED,		1 },
  { EL_EMERALD_PURPLE,		1 },
  { EL_DIAMOND,			3 },
  { EL_SP_INFOTRON,		1 },
  { EL_PEARL,			5 },
  { EL_CRYSTAL,			8 },

  { EL_UNDEFINED,		0 },
};

struct
{
  int element;
  int direction;
}
access_direction_list[] =
{
  { EL_TUBE_ANY,			MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
  { EL_TUBE_VERTICAL,			                     MV_UP | MV_DOWN },
  { EL_TUBE_HORIZONTAL,			MV_LEFT | MV_RIGHT                   },
  { EL_TUBE_VERTICAL_LEFT,		MV_LEFT |            MV_UP | MV_DOWN },
  { EL_TUBE_VERTICAL_RIGHT,		          MV_RIGHT | MV_UP | MV_DOWN },
  { EL_TUBE_HORIZONTAL_UP,		MV_LEFT | MV_RIGHT | MV_UP           },
  { EL_TUBE_HORIZONTAL_DOWN,		MV_LEFT | MV_RIGHT |         MV_DOWN },
  { EL_TUBE_LEFT_UP,			MV_LEFT |            MV_UP           },
  { EL_TUBE_LEFT_DOWN,			MV_LEFT |                    MV_DOWN },
  { EL_TUBE_RIGHT_UP,			          MV_RIGHT | MV_UP           },
  { EL_TUBE_RIGHT_DOWN,			          MV_RIGHT |         MV_DOWN },

  { EL_SP_PORT_LEFT,			          MV_RIGHT                   },
  { EL_SP_PORT_RIGHT,			MV_LEFT                              },
  { EL_SP_PORT_UP,			                             MV_DOWN },
  { EL_SP_PORT_DOWN,			                     MV_UP           },
  { EL_SP_PORT_HORIZONTAL,		MV_LEFT | MV_RIGHT                   },
  { EL_SP_PORT_VERTICAL,		                     MV_UP | MV_DOWN },
  { EL_SP_PORT_ANY,			MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN },
  { EL_SP_GRAVITY_PORT_LEFT,		          MV_RIGHT                   },
  { EL_SP_GRAVITY_PORT_RIGHT,		MV_LEFT                              },
  { EL_SP_GRAVITY_PORT_UP,		                             MV_DOWN },
  { EL_SP_GRAVITY_PORT_DOWN,		                     MV_UP           },
  { EL_SP_GRAVITY_ON_PORT_LEFT,		          MV_RIGHT                   },
  { EL_SP_GRAVITY_ON_PORT_RIGHT,	MV_LEFT                              },
  { EL_SP_GRAVITY_ON_PORT_UP,		                             MV_DOWN },
  { EL_SP_GRAVITY_ON_PORT_DOWN,		                     MV_UP           },
  { EL_SP_GRAVITY_OFF_PORT_LEFT,	          MV_RIGHT                   },
  { EL_SP_GRAVITY_OFF_PORT_RIGHT,	MV_LEFT                              },
  { EL_SP_GRAVITY_OFF_PORT_UP,		                             MV_DOWN },
  { EL_SP_GRAVITY_OFF_PORT_DOWN,	                     MV_UP           },

  { EL_UNDEFINED,			MV_NONE				     }
};

static struct XY xy_topdown[] =
{
  {  0, -1 },
  { -1,  0 },
  { +1,  0 },
  {  0, +1 }
};

static boolean trigger_events[MAX_NUM_ELEMENTS][NUM_CHANGE_EVENTS];

#define IS_AUTO_CHANGING(e)	(element_info[e].has_change_event[CE_DELAY])
#define IS_JUST_CHANGING(x, y)	(ChangeDelay[x][y] != 0)
#define IS_CHANGING(x, y)	(IS_AUTO_CHANGING(Tile[x][y]) || \
				 IS_JUST_CHANGING(x, y))

#define CE_PAGE(e, ce)		(element_info[e].event_page[ce])

// static variables for playfield scan mode (scanning forward or backward)
static int playfield_scan_start_x = 0;
static int playfield_scan_start_y = 0;
static int playfield_scan_delta_x = 1;
static int playfield_scan_delta_y = 1;

#define SCAN_PLAYFIELD(x, y)	for ((y) = playfield_scan_start_y;	\
				     (y) >= 0 && (y) <= lev_fieldy - 1;	\
				     (y) += playfield_scan_delta_y)	\
				for ((x) = playfield_scan_start_x;	\
				     (x) >= 0 && (x) <= lev_fieldx - 1;	\
				     (x) += playfield_scan_delta_x)

#ifdef DEBUG
void DEBUG_SetMaximumDynamite(void)
{
  int i;

  for (i = 0; i < MAX_INVENTORY_SIZE; i++)
    if (local_player->inventory_size < MAX_INVENTORY_SIZE)
      local_player->inventory_element[local_player->inventory_size++] =
	EL_DYNAMITE;
}
#endif

static void InitPlayfieldScanModeVars(void)
{
  if (game.use_reverse_scan_direction)
  {
    playfield_scan_start_x = lev_fieldx - 1;
    playfield_scan_start_y = lev_fieldy - 1;

    playfield_scan_delta_x = -1;
    playfield_scan_delta_y = -1;
  }
  else
  {
    playfield_scan_start_x = 0;
    playfield_scan_start_y = 0;

    playfield_scan_delta_x = 1;
    playfield_scan_delta_y = 1;
  }
}

static void InitPlayfieldScanMode(int mode)
{
  game.use_reverse_scan_direction =
    (mode == CA_ARG_SCAN_MODE_REVERSE ? TRUE : FALSE);

  InitPlayfieldScanModeVars();
}

static int get_move_delay_from_stepsize(int move_stepsize)
{
  move_stepsize =
    MIN(MAX(MOVE_STEPSIZE_MIN, move_stepsize), MOVE_STEPSIZE_MAX);

  // make sure that stepsize value is always a power of 2
  move_stepsize = (1 << log_2(move_stepsize));

  return TILEX / move_stepsize;
}

static void SetPlayerMoveSpeed(struct PlayerInfo *player, int move_stepsize,
			       boolean init_game)
{
  int player_nr = player->index_nr;
  int move_delay = get_move_delay_from_stepsize(move_stepsize);
  boolean cannot_move = (move_stepsize == STEPSIZE_NOT_MOVING ? TRUE : FALSE);

  // do no immediately change move delay -- the player might just be moving
  player->move_delay_value_next = move_delay;

  // information if player can move must be set separately
  player->cannot_move = cannot_move;

  if (init_game)
  {
    player->move_delay       = game.initial_move_delay[player_nr];
    player->move_delay_value = game.initial_move_delay_value[player_nr];

    player->move_delay_value_next = -1;

    player->move_delay_reset_counter = 0;
  }
}

void GetPlayerConfig(void)
{
  GameFrameDelay = setup.game_frame_delay;

  if (!audio.sound_available)
    setup.sound_simple = FALSE;

  if (!audio.loops_available)
    setup.sound_loops = FALSE;

  if (!audio.music_available)
    setup.sound_music = FALSE;

  if (!video.fullscreen_available)
    setup.fullscreen = FALSE;

  setup.sound = (setup.sound_simple || setup.sound_loops || setup.sound_music);

  SetAudioMode(setup.sound);
}

int GetElementFromGroupElement(int element)
{
  if (IS_GROUP_ELEMENT(element))
  {
    struct ElementGroupInfo *group = element_info[element].group;
    int last_anim_random_frame = gfx.anim_random_frame;
    int element_pos;

    if (group->choice_mode == ANIM_RANDOM)
      gfx.anim_random_frame = RND(group->num_elements_resolved);

    element_pos = getAnimationFrame(group->num_elements_resolved, 1,
				    group->choice_mode, 0,
				    group->choice_pos);

    if (group->choice_mode == ANIM_RANDOM)
      gfx.anim_random_frame = last_anim_random_frame;

    group->choice_pos++;

    element = group->element_resolved[element_pos];
  }

  return element;
}

static void IncrementSokobanFieldsNeeded(void)
{
  if (level.sb_fields_needed)
    game.sokoban_fields_still_needed++;
}

static void IncrementSokobanObjectsNeeded(void)
{
  if (level.sb_objects_needed)
    game.sokoban_objects_still_needed++;
}

static void DecrementSokobanFieldsNeeded(void)
{
  if (game.sokoban_fields_still_needed > 0)
    game.sokoban_fields_still_needed--;
}

static void DecrementSokobanObjectsNeeded(void)
{
  if (game.sokoban_objects_still_needed > 0)
    game.sokoban_objects_still_needed--;
}

static void InitPlayerField(int x, int y, int element, boolean init_game)
{
  if (element == EL_SP_MURPHY)
  {
    if (init_game)
    {
      if (stored_player[0].present)
      {
	Tile[x][y] = EL_SP_MURPHY_CLONE;

	return;
      }
      else
      {
	stored_player[0].initial_element = element;
	stored_player[0].use_murphy = TRUE;

	if (!level.use_artwork_element[0])
	  stored_player[0].artwork_element = EL_SP_MURPHY;
      }

      Tile[x][y] = EL_PLAYER_1;
    }
  }
  else if (element < EL_PLAYER_1 || element > EL_PLAYER_4)
  {
    // needed for BD engine with preferred player other than first player
    Tile[x][y] = GET_PLAYER_ELEMENT(element);
  }

  if (init_game)
  {
    struct PlayerInfo *player = &stored_player[Tile[x][y] - EL_PLAYER_1];
    int jx = player->jx, jy = player->jy;

    player->present = TRUE;

    player->block_last_field = (element == EL_SP_MURPHY ?
				level.sp_block_last_field :
				level.block_last_field);

    // ---------- initialize player's last field block delay ------------------

    // always start with reliable default value (no adjustment needed)
    player->block_delay_adjustment = 0;

    // special case 1: in Supaplex, Murphy blocks last field one more frame
    if (player->block_last_field && element == EL_SP_MURPHY)
      player->block_delay_adjustment = 1;

    // special case 2: in game engines before 3.1.1, blocking was different
    if (game.use_block_last_field_bug)
      player->block_delay_adjustment = (player->block_last_field ? -1 : 1);

    if (!network.enabled || player->connected_network)
    {
      player->active = TRUE;

      // remove potentially duplicate players
      if (IN_LEV_FIELD(jx, jy) && StorePlayer[jx][jy] == Tile[x][y])
	StorePlayer[jx][jy] = 0;

      StorePlayer[x][y] = Tile[x][y];

#if DEBUG_INIT_PLAYER
      Debug("game:init:player", "- player element %d activated",
	    player->element_nr);
      Debug("game:init:player", "  (local player is %d and currently %s)",
	    local_player->element_nr,
	    local_player->active ? "active" : "not active");
    }
#endif

    Tile[x][y] = EL_EMPTY;

    player->jx = player->last_jx = x;
    player->jy = player->last_jy = y;
  }

  // always check if player was just killed and should be reanimated
  {
    int player_nr = GET_PLAYER_NR(element);
    struct PlayerInfo *player = &stored_player[player_nr];

    if (player->active && player->killed)
      player->reanimated = TRUE; // if player was just killed, reanimate him
  }
}

static void InitFieldForEngine_RND(int x, int y)
{
  int element = Tile[x][y];

  // convert BD engine elements to corresponding R'n'D engine elements
  element = (element == EL_BDX_EMPTY				? EL_EMPTY :
	     element == EL_BDX_PLAYER				? EL_PLAYER_1 :
	     element == EL_BDX_INBOX				? EL_PLAYER_1 :
	     element == EL_BDX_SAND				? EL_SAND :
	     element == EL_BDX_GRASS				? EL_EMC_GRASS :
	     element == EL_BDX_WALL				? EL_BD_WALL :
	     element == EL_BDX_STEELWALL			? EL_STEELWALL :
	     element == EL_BDX_ROCK				? EL_BD_ROCK :
	     element == EL_BDX_DIAMOND				? EL_BD_DIAMOND :
	     element == EL_BDX_AMOEBA_1				? EL_BD_AMOEBA :
	     element == EL_BDX_MAGIC_WALL			? EL_BD_MAGIC_WALL :
	     element == EL_BDX_BUTTERFLY_1_RIGHT		? EL_BD_BUTTERFLY_RIGHT :
	     element == EL_BDX_BUTTERFLY_1_UP			? EL_BD_BUTTERFLY_UP :
	     element == EL_BDX_BUTTERFLY_1_LEFT			? EL_BD_BUTTERFLY_LEFT :
	     element == EL_BDX_BUTTERFLY_1_DOWN			? EL_BD_BUTTERFLY_DOWN :
	     element == EL_BDX_BUTTERFLY_1			? EL_BD_BUTTERFLY :
	     element == EL_BDX_FIREFLY_1_RIGHT			? EL_BD_FIREFLY_RIGHT :
	     element == EL_BDX_FIREFLY_1_UP			? EL_BD_FIREFLY_UP :
	     element == EL_BDX_FIREFLY_1_LEFT			? EL_BD_FIREFLY_LEFT :
	     element == EL_BDX_FIREFLY_1_DOWN			? EL_BD_FIREFLY_DOWN :
	     element == EL_BDX_FIREFLY_1			? EL_BD_FIREFLY :
	     element == EL_BDX_EXPANDABLE_WALL_HORIZONTAL	? EL_BD_EXPANDABLE_WALL :
	     element == EL_BDX_WALL_DIAMOND			? EL_WALL_BD_DIAMOND :
	     element == EL_BDX_EXIT_CLOSED			? EL_EXIT_CLOSED :
	     element == EL_BDX_EXIT_OPEN			? EL_EXIT_OPEN :
	     element);

  Tile[x][y] = element;
}

static void InitFieldForEngine(int x, int y)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    InitFieldForEngine_RND(x, y);
}

static void InitField(int x, int y, boolean init_game)
{
  int element = Tile[x][y];

  switch (element)
  {
    case EL_SP_MURPHY:
    case EL_PLAYER_1:
    case EL_PLAYER_2:
    case EL_PLAYER_3:
    case EL_PLAYER_4:
    case EL_BDX_INBOX:
    case EL_BDX_PLAYER:
    case EL_BDX_PLAYER_WITH_BOMB:
    case EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER:
    case EL_BDX_PLAYER_GLUED:
    case EL_BDX_PLAYER_SCANNED:
      InitPlayerField(x, y, element, init_game);
      break;

    case EL_SOKOBAN_FIELD_PLAYER:
      element = Tile[x][y] = EL_PLAYER_1;
      InitField(x, y, init_game);

      element = Tile[x][y] = EL_SOKOBAN_FIELD_EMPTY;
      InitField(x, y, init_game);
      break;

    case EL_SOKOBAN_FIELD_EMPTY:
      IncrementSokobanFieldsNeeded();
      break;

    case EL_SOKOBAN_OBJECT:
      IncrementSokobanObjectsNeeded();
      break;

    case EL_STONEBLOCK:
      if (x < lev_fieldx - 1 && Tile[x + 1][y] == EL_ACID)
	Tile[x][y] = EL_ACID_POOL_TOPLEFT;
      else if (x > 0 && Tile[x - 1][y] == EL_ACID)
	Tile[x][y] = EL_ACID_POOL_TOPRIGHT;
      else if (y > 0 && Tile[x][y - 1] == EL_ACID_POOL_TOPLEFT)
	Tile[x][y] = EL_ACID_POOL_BOTTOMLEFT;
      else if (y > 0 && Tile[x][y - 1] == EL_ACID)
	Tile[x][y] = EL_ACID_POOL_BOTTOM;
      else if (y > 0 && Tile[x][y - 1] == EL_ACID_POOL_TOPRIGHT)
	Tile[x][y] = EL_ACID_POOL_BOTTOMRIGHT;
      break;

    case EL_BUG:
    case EL_BUG_RIGHT:
    case EL_BUG_UP:
    case EL_BUG_LEFT:
    case EL_BUG_DOWN:
    case EL_SPACESHIP:
    case EL_SPACESHIP_RIGHT:
    case EL_SPACESHIP_UP:
    case EL_SPACESHIP_LEFT:
    case EL_SPACESHIP_DOWN:
    case EL_BD_BUTTERFLY:
    case EL_BD_BUTTERFLY_RIGHT:
    case EL_BD_BUTTERFLY_UP:
    case EL_BD_BUTTERFLY_LEFT:
    case EL_BD_BUTTERFLY_DOWN:
    case EL_BD_FIREFLY:
    case EL_BD_FIREFLY_RIGHT:
    case EL_BD_FIREFLY_UP:
    case EL_BD_FIREFLY_LEFT:
    case EL_BD_FIREFLY_DOWN:
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
    case EL_YAMYAM:
    case EL_YAMYAM_LEFT:
    case EL_YAMYAM_RIGHT:
    case EL_YAMYAM_UP:
    case EL_YAMYAM_DOWN:
    case EL_DARK_YAMYAM:
    case EL_ROBOT:
    case EL_PACMAN:
    case EL_SP_SNIKSNAK:
    case EL_SP_ELECTRON:
    case EL_MOLE:
    case EL_MOLE_LEFT:
    case EL_MOLE_RIGHT:
    case EL_MOLE_UP:
    case EL_MOLE_DOWN:
    case EL_SPRING_LEFT:
    case EL_SPRING_RIGHT:
      InitMovDir(x, y);
      break;

    case EL_AMOEBA_FULL:
    case EL_BD_AMOEBA:
      InitAmoebaNr(x, y);
      break;

    case EL_AMOEBA_DROP:
      if (y == lev_fieldy - 1)
      {
	Tile[x][y] = EL_AMOEBA_GROWING;
	Store[x][y] = EL_AMOEBA_WET;
      }
      break;

    case EL_DYNAMITE_ACTIVE:
    case EL_SP_DISK_RED_ACTIVE:
    case EL_DYNABOMB_PLAYER_1_ACTIVE:
    case EL_DYNABOMB_PLAYER_2_ACTIVE:
    case EL_DYNABOMB_PLAYER_3_ACTIVE:
    case EL_DYNABOMB_PLAYER_4_ACTIVE:
      MovDelay[x][y] = 96;
      break;

    case EL_EM_DYNAMITE_ACTIVE:
      MovDelay[x][y] = 32;
      break;

    case EL_LAMP:
      game.lights_still_needed++;
      break;

    case EL_PENGUIN:
      game.friends_still_needed++;
      break;

    case EL_PIG:
    case EL_DRAGON:
      GfxDir[x][y] = MovDir[x][y] = 1 << RND(4);
      break;

    case EL_CONVEYOR_BELT_1_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_1_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_1_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_2_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_2_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_2_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_3_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_3_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_3_SWITCH_RIGHT:
    case EL_CONVEYOR_BELT_4_SWITCH_LEFT:
    case EL_CONVEYOR_BELT_4_SWITCH_MIDDLE:
    case EL_CONVEYOR_BELT_4_SWITCH_RIGHT:
      if (init_game)
      {
	int belt_nr = getBeltNrFromBeltSwitchElement(Tile[x][y]);
	int belt_dir = getBeltDirFromBeltSwitchElement(Tile[x][y]);
	int belt_dir_nr = getBeltDirNrFromBeltSwitchElement(Tile[x][y]);

	if (game.belt_dir_nr[belt_nr] == 3)	// initial value
	{
	  game.belt_dir[belt_nr] = belt_dir;
	  game.belt_dir_nr[belt_nr] = belt_dir_nr;
	}
	else	// more than one switch -- set it like the first switch
	{
	  Tile[x][y] = Tile[x][y] - belt_dir_nr + game.belt_dir_nr[belt_nr];
	}
      }
      break;

    case EL_LIGHT_SWITCH_ACTIVE:
      if (init_game)
	game.light_time_left = level.time_light * FRAMES_PER_SECOND;
      break;

    case EL_INVISIBLE_STEELWALL:
    case EL_INVISIBLE_WALL:
    case EL_INVISIBLE_SAND:
      if (game.light_time_left > 0 ||
	  game.lenses_time_left > 0)
        Tile[x][y] = getInvisibleActiveFromInvisibleElement(element);
      break;

    case EL_EMC_MAGIC_BALL:
      if (game.ball_active)
	Tile[x][y] = EL_EMC_MAGIC_BALL_ACTIVE;
      break;

    case EL_EMC_MAGIC_BALL_SWITCH:
      if (game.ball_active)
	Tile[x][y] = EL_EMC_MAGIC_BALL_SWITCH_ACTIVE;
      break;

    case EL_TRIGGER_PLAYER:
    case EL_TRIGGER_ELEMENT:
    case EL_TRIGGER_CE_VALUE:
    case EL_TRIGGER_CE_SCORE:
    case EL_SELF:
    case EL_ANY_ELEMENT:
    case EL_CURRENT_CE_VALUE:
    case EL_CURRENT_CE_SCORE:
    case EL_PREV_CE_1:
    case EL_PREV_CE_2:
    case EL_PREV_CE_3:
    case EL_PREV_CE_4:
    case EL_PREV_CE_5:
    case EL_PREV_CE_6:
    case EL_PREV_CE_7:
    case EL_PREV_CE_8:
    case EL_NEXT_CE_1:
    case EL_NEXT_CE_2:
    case EL_NEXT_CE_3:
    case EL_NEXT_CE_4:
    case EL_NEXT_CE_5:
    case EL_NEXT_CE_6:
    case EL_NEXT_CE_7:
    case EL_NEXT_CE_8:
      // reference elements should not be used on the playfield
      Tile[x][y] = EL_EMPTY;
      break;

    default:
      if (IS_CUSTOM_ELEMENT(element))
      {
	if (CAN_MOVE(element))
	  InitMovDir(x, y);

	if (!element_info[element].use_last_ce_value || init_game)
	  CustomValue[x][y] = GET_NEW_CE_VALUE(Tile[x][y]);
      }
      else if (IS_GROUP_ELEMENT(element))
      {
	Tile[x][y] = GetElementFromGroupElement(element);

	InitField(x, y, init_game);
      }
      else if (IS_EMPTY_ELEMENT(element))
      {
	GfxElementEmpty[x][y] = element;
	Tile[x][y] = EL_EMPTY;

	if (element_info[element].use_gfx_element)
	  game.use_masked_elements = TRUE;
      }

      break;
  }

  if (!init_game)
    CheckTriggeredElementChange(x, y, element, CE_CREATION_OF_X);
}

static void InitField_WithBug1(int x, int y, boolean init_game)
{
  InitField(x, y, init_game);

  // not needed to call InitMovDir() -- already done by InitField()!
  if (game.engine_version < VERSION_IDENT(3,1,0,0) &&
      CAN_MOVE(Tile[x][y]))
    InitMovDir(x, y);
}

static void InitField_WithBug2(int x, int y, boolean init_game)
{
  int old_element = Tile[x][y];

  InitField(x, y, init_game);

  // not needed to call InitMovDir() -- already done by InitField()!
  if (game.engine_version < VERSION_IDENT(3,1,0,0) &&
      CAN_MOVE(old_element) &&
      (old_element < EL_MOLE_LEFT || old_element > EL_MOLE_DOWN))
    InitMovDir(x, y);

  /* this case is in fact a combination of not less than three bugs:
     first, it calls InitMovDir() for elements that can move, although this is
     already done by InitField(); then, it checks the element that was at this
     field _before_ the call to InitField() (which can change it); lastly, it
     was not called for "mole with direction" elements, which were treated as
     "cannot move" due to (fixed) wrong element initialization in "src/init.c"
  */
}

static int get_key_element_from_nr(int key_nr)
{
  int key_base_element = (key_nr >= STD_NUM_KEYS ? EL_EMC_KEY_5 - STD_NUM_KEYS :
			  level.game_engine_type == GAME_ENGINE_TYPE_EM ?
			  EL_EM_KEY_1 : EL_KEY_1);

  return key_base_element + key_nr;
}

static int get_next_dropped_element(struct PlayerInfo *player)
{
  return (player->inventory_size > 0 ?
	  player->inventory_element[player->inventory_size - 1] :
	  player->inventory_infinite_element != EL_UNDEFINED ?
	  player->inventory_infinite_element :
	  player->dynabombs_left > 0 ?
	  EL_DYNABOMB_PLAYER_1_ACTIVE + player->index_nr :
	  EL_UNDEFINED);
}

static int get_inventory_element_from_pos(struct PlayerInfo *player, int pos)
{
  // pos >= 0: get element from bottom of the stack;
  // pos <  0: get element from top of the stack

  if (pos < 0)
  {
    int min_inventory_size = -pos;
    int inventory_pos = player->inventory_size - min_inventory_size;
    int min_dynabombs_left = min_inventory_size - player->inventory_size;

    return (player->inventory_size >= min_inventory_size ?
	    player->inventory_element[inventory_pos] :
	    player->inventory_infinite_element != EL_UNDEFINED ?
	    player->inventory_infinite_element :
	    player->dynabombs_left >= min_dynabombs_left ?
	    EL_DYNABOMB_PLAYER_1 + player->index_nr :
	    EL_UNDEFINED);
  }
  else
  {
    int min_dynabombs_left = pos + 1;
    int min_inventory_size = pos + 1 - player->dynabombs_left;
    int inventory_pos = pos - player->dynabombs_left;

    return (player->inventory_infinite_element != EL_UNDEFINED ?
	    player->inventory_infinite_element :
	    player->dynabombs_left >= min_dynabombs_left ?
	    EL_DYNABOMB_PLAYER_1 + player->index_nr :
	    player->inventory_size >= min_inventory_size ?
	    player->inventory_element[inventory_pos] :
	    EL_UNDEFINED);
  }
}

static int compareGamePanelOrderInfo(const void *object1, const void *object2)
{
  const struct GamePanelOrderInfo *gpo1 = (struct GamePanelOrderInfo *)object1;
  const struct GamePanelOrderInfo *gpo2 = (struct GamePanelOrderInfo *)object2;
  int compare_result;

  if (gpo1->sort_priority != gpo2->sort_priority)
    compare_result = gpo1->sort_priority - gpo2->sort_priority;
  else
    compare_result = gpo1->nr - gpo2->nr;

  return compare_result;
}

int getPlayerInventorySize(int player_nr)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return game_em.ply[player_nr]->dynamite;
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return game_sp.red_disk_count;
  else
    return stored_player[player_nr].inventory_size;
}

static void InitGameControlValues(void)
{
  int i;

  for (i = 0; game_panel_controls[i].nr != -1; i++)
  {
    struct GamePanelControlInfo *gpc = &game_panel_controls[i];
    struct GamePanelOrderInfo *gpo = &game_panel_order[i];
    struct TextPosInfo *pos = gpc->pos;
    int nr = gpc->nr;
    int type = gpc->type;

    if (nr != i)
    {
      Error("'game_panel_controls' structure corrupted at %d", i);

      Fail("this should not happen -- please debug");
    }

    // force update of game controls after initialization
    gpc->value = gpc->last_value = -1;
    gpc->frame = gpc->last_frame = -1;
    gpc->gfx_frame = -1;

    // determine panel value width for later calculation of alignment
    if (type == TYPE_INTEGER || type == TYPE_STRING)
    {
      pos->width = pos->size * getFontWidth(pos->font);
      pos->height = getFontHeight(pos->font);
    }
    else if (type == TYPE_ELEMENT)
    {
      pos->width = pos->size;
      pos->height = pos->size;
    }

    // fill structure for game panel draw order
    gpo->nr = gpc->nr;
    gpo->sort_priority = pos->sort_priority;
  }

  // sort game panel controls according to sort_priority and control number
  qsort(game_panel_order, NUM_GAME_PANEL_CONTROLS,
	sizeof(struct GamePanelOrderInfo), compareGamePanelOrderInfo);
}

static void UpdatePlayfieldElementCount(void)
{
  boolean use_element_count = FALSE;
  int i, j, x, y;

  // first check if it is needed at all to calculate playfield element count
  for (i = GAME_PANEL_ELEMENT_COUNT_1; i <= GAME_PANEL_ELEMENT_COUNT_8; i++)
    if (!PANEL_DEACTIVATED(game_panel_controls[i].pos))
      use_element_count = TRUE;

  if (!use_element_count)
    return;

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    element_info[i].element_count = 0;

  SCAN_PLAYFIELD(x, y)
  {
    element_info[Tile[x][y]].element_count++;
  }

  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
    for (j = 0; j < MAX_NUM_ELEMENTS; j++)
      if (IS_IN_GROUP(j, i))
	element_info[EL_GROUP_START + i].element_count +=
	  element_info[j].element_count;
}

static void UpdateGameControlValues(void)
{
  int i, k;
  int time = (game.LevelSolved ?
	      game.LevelSolved_CountingTime :
	      level.game_engine_type == GAME_ENGINE_TYPE_BD ?
	      game_bd.time_left :
	      level.game_engine_type == GAME_ENGINE_TYPE_EM ?
	      game_em.lev->time :
	      level.game_engine_type == GAME_ENGINE_TYPE_SP ?
	      game_sp.time_played :
	      level.game_engine_type == GAME_ENGINE_TYPE_MM ?
	      game_mm.energy_left :
	      game.no_level_time_limit ? TimePlayed : TimeLeft);
  int score = (game.LevelSolved ?
	       game.LevelSolved_CountingScore :
	       level.game_engine_type == GAME_ENGINE_TYPE_BD ?
	       game_bd.global_score :
	       level.game_engine_type == GAME_ENGINE_TYPE_EM ?
	       game_em.lev->score :
	       level.game_engine_type == GAME_ENGINE_TYPE_SP ?
	       game_sp.score :
	       level.game_engine_type == GAME_ENGINE_TYPE_MM ?
	       game_mm.score :
	       game.score);
  int gems = (level.game_engine_type == GAME_ENGINE_TYPE_BD ?
	      game_bd.gems_still_needed :
	      level.game_engine_type == GAME_ENGINE_TYPE_EM ?
	      game_em.lev->gems_needed :
	      level.game_engine_type == GAME_ENGINE_TYPE_SP ?
	      game_sp.infotrons_still_needed :
	      level.game_engine_type == GAME_ENGINE_TYPE_MM ?
	      game_mm.kettles_still_needed :
	      game.gems_still_needed);
  int gems_needed = (level.game_engine_type == GAME_ENGINE_TYPE_BD ?
		     game_bd.game->cave->diamonds_needed :
		     level.gems_needed);
  int gems_collected = (level.game_engine_type == GAME_ENGINE_TYPE_BD ?
			game_bd.game->cave->diamonds_collected :
			gems_needed - gems);
  int gems_score = (level.game_engine_type == GAME_ENGINE_TYPE_BD ?
		    game_bd.game->cave->diamond_value :
		    level.score[SC_EMERALD]);
  int exit_closed = (level.game_engine_type == GAME_ENGINE_TYPE_BD ?
		     game_bd.gems_still_needed > 0 :
		     level.game_engine_type == GAME_ENGINE_TYPE_EM ?
		     game_em.lev->gems_needed > 0 :
		     level.game_engine_type == GAME_ENGINE_TYPE_SP ?
		     game_sp.infotrons_still_needed > 0 :
		     level.game_engine_type == GAME_ENGINE_TYPE_MM ?
		     game_mm.kettles_still_needed > 0 ||
		     game_mm.lights_still_needed > 0 :
		     game.gems_still_needed > 0 ||
		     game.sokoban_fields_still_needed > 0 ||
		     game.sokoban_objects_still_needed > 0 ||
		     game.lights_still_needed > 0);
  int health = (game.LevelSolved ?
		game.LevelSolved_CountingHealth :
		level.game_engine_type == GAME_ENGINE_TYPE_MM ?
		MM_HEALTH(game_mm.laser_overload_value) :
		game.health);
  int sync_random_frame = INIT_GFX_RANDOM();	// random, but synchronized

  UpdatePlayfieldElementCount();

  // update game panel control values

  // used instead of "level_nr" (for network games)
  game_panel_controls[GAME_PANEL_LEVEL_NUMBER].value = levelset.level_nr;
  game_panel_controls[GAME_PANEL_GEMS].value = gems;
  game_panel_controls[GAME_PANEL_GEMS_NEEDED].value = gems_needed;
  game_panel_controls[GAME_PANEL_GEMS_COLLECTED].value = gems_collected;
  game_panel_controls[GAME_PANEL_GEMS_SCORE].value = gems_score;

  game_panel_controls[GAME_PANEL_INVENTORY_COUNT].value = 0;
  for (i = 0; i < MAX_NUM_KEYS; i++)
    game_panel_controls[GAME_PANEL_KEY_1 + i].value = EL_EMPTY;
  game_panel_controls[GAME_PANEL_KEY_WHITE].value = EL_EMPTY;
  game_panel_controls[GAME_PANEL_KEY_WHITE_COUNT].value = 0;

  if (game.centered_player_nr == -1)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      // only one player in Supaplex game engine
      if (level.game_engine_type == GAME_ENGINE_TYPE_SP && i > 0)
	break;

      for (k = 0; k < MAX_NUM_KEYS; k++)
      {
	if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
	{
	  if (game_em.ply[i]->keys & (1 << k))
	    game_panel_controls[GAME_PANEL_KEY_1 + k].value =
	      get_key_element_from_nr(k);
	}
	else if (stored_player[i].key[k])
	  game_panel_controls[GAME_PANEL_KEY_1 + k].value =
	    get_key_element_from_nr(k);
      }

      game_panel_controls[GAME_PANEL_INVENTORY_COUNT].value +=
	getPlayerInventorySize(i);

      if (stored_player[i].num_white_keys > 0)
	game_panel_controls[GAME_PANEL_KEY_WHITE].value =
	  EL_DC_KEY_WHITE;

      game_panel_controls[GAME_PANEL_KEY_WHITE_COUNT].value +=
	stored_player[i].num_white_keys;
    }
  }
  else
  {
    int player_nr = game.centered_player_nr;

    for (k = 0; k < MAX_NUM_KEYS; k++)
    {
      if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
      {
	if (game_em.ply[player_nr]->keys & (1 << k))
	  game_panel_controls[GAME_PANEL_KEY_1 + k].value =
	    get_key_element_from_nr(k);
      }
      else if (stored_player[player_nr].key[k])
	game_panel_controls[GAME_PANEL_KEY_1 + k].value =
	  get_key_element_from_nr(k);
    }

    game_panel_controls[GAME_PANEL_INVENTORY_COUNT].value +=
      getPlayerInventorySize(player_nr);

    if (stored_player[player_nr].num_white_keys > 0)
      game_panel_controls[GAME_PANEL_KEY_WHITE].value = EL_DC_KEY_WHITE;

    game_panel_controls[GAME_PANEL_KEY_WHITE_COUNT].value +=
      stored_player[player_nr].num_white_keys;
  }

  // re-arrange keys on game panel, if needed or if defined by style settings
  for (i = 0; i < MAX_NUM_KEYS + 1; i++)	// all normal keys + white key
  {
    int nr = GAME_PANEL_KEY_1 + i;
    struct GamePanelControlInfo *gpc = &game_panel_controls[nr];
    struct TextPosInfo *pos = gpc->pos;

    // skip check if key is not in the player's inventory
    if (gpc->value == EL_EMPTY)
      continue;

    // check if keys should be arranged on panel from left to right
    if (pos->style == STYLE_LEFTMOST_POSITION)
    {
      // check previous key positions (left from current key)
      for (k = 0; k < i; k++)
      {
	int nr_new = GAME_PANEL_KEY_1 + k;

	if (game_panel_controls[nr_new].value == EL_EMPTY)
	{
	  game_panel_controls[nr_new].value = gpc->value;
	  gpc->value = EL_EMPTY;

	  break;
	}
      }
    }

    // check if "undefined" keys can be placed at some other position
    if (pos->x == -1 && pos->y == -1)
    {
      int nr_new = GAME_PANEL_KEY_1 + i % STD_NUM_KEYS;

      // 1st try: display key at the same position as normal or EM keys
      if (game_panel_controls[nr_new].value == EL_EMPTY)
      {
	game_panel_controls[nr_new].value = gpc->value;
      }
      else
      {
	// 2nd try: display key at the next free position in the key panel
	for (k = 0; k < STD_NUM_KEYS; k++)
	{
	  nr_new = GAME_PANEL_KEY_1 + k;

	  if (game_panel_controls[nr_new].value == EL_EMPTY)
	  {
	    game_panel_controls[nr_new].value = gpc->value;

	    break;
	  }
	}
      }
    }
  }

  for (i = 0; i < NUM_PANEL_INVENTORY; i++)
  {
    game_panel_controls[GAME_PANEL_INVENTORY_FIRST_1 + i].value =
      get_inventory_element_from_pos(local_player, i);
    game_panel_controls[GAME_PANEL_INVENTORY_LAST_1 + i].value =
      get_inventory_element_from_pos(local_player, -i - 1);
  }

  game_panel_controls[GAME_PANEL_SCORE].value = score;
  game_panel_controls[GAME_PANEL_HIGHSCORE].value = scores.entry[0].score;

  game_panel_controls[GAME_PANEL_TIME].value = time;

  game_panel_controls[GAME_PANEL_TIME_HH].value = time / 3600;
  game_panel_controls[GAME_PANEL_TIME_MM].value = (time / 60) % 60;
  game_panel_controls[GAME_PANEL_TIME_SS].value = time % 60;

  if (level.time == 0)
    game_panel_controls[GAME_PANEL_TIME_ANIM].value = 100;
  else
    game_panel_controls[GAME_PANEL_TIME_ANIM].value = time * 100 / level.time;

  game_panel_controls[GAME_PANEL_HEALTH].value = health;
  game_panel_controls[GAME_PANEL_HEALTH_ANIM].value = health;

  game_panel_controls[GAME_PANEL_FRAME].value = FrameCounter;

  game_panel_controls[GAME_PANEL_SHIELD_NORMAL].value =
    (local_player->shield_normal_time_left > 0 ? EL_SHIELD_NORMAL_ACTIVE :
     EL_EMPTY);
  game_panel_controls[GAME_PANEL_SHIELD_NORMAL_TIME].value =
    local_player->shield_normal_time_left;
  game_panel_controls[GAME_PANEL_SHIELD_DEADLY].value =
    (local_player->shield_deadly_time_left > 0 ? EL_SHIELD_DEADLY_ACTIVE :
     EL_EMPTY);
  game_panel_controls[GAME_PANEL_SHIELD_DEADLY_TIME].value =
    local_player->shield_deadly_time_left;

  game_panel_controls[GAME_PANEL_EXIT].value =
    (exit_closed ? EL_EXIT_CLOSED : EL_EXIT_OPEN);

  game_panel_controls[GAME_PANEL_EMC_MAGIC_BALL].value =
    (game.ball_active ? EL_EMC_MAGIC_BALL_ACTIVE : EL_EMC_MAGIC_BALL);
  game_panel_controls[GAME_PANEL_EMC_MAGIC_BALL_SWITCH].value =
    (game.ball_active ? EL_EMC_MAGIC_BALL_SWITCH_ACTIVE :
     EL_EMC_MAGIC_BALL_SWITCH);

  game_panel_controls[GAME_PANEL_LIGHT_SWITCH].value =
    (game.light_time_left > 0 ? EL_LIGHT_SWITCH_ACTIVE : EL_LIGHT_SWITCH);
  game_panel_controls[GAME_PANEL_LIGHT_SWITCH_TIME].value =
    game.light_time_left;

  game_panel_controls[GAME_PANEL_TIMEGATE_SWITCH].value =
    (game.timegate_time_left > 0 ? EL_TIMEGATE_OPEN : EL_TIMEGATE_CLOSED);
  game_panel_controls[GAME_PANEL_TIMEGATE_SWITCH_TIME].value =
    game.timegate_time_left;

  game_panel_controls[GAME_PANEL_SWITCHGATE_SWITCH].value =
    EL_SWITCHGATE_SWITCH_UP + game.switchgate_pos;

  game_panel_controls[GAME_PANEL_EMC_LENSES].value =
    (game.lenses_time_left > 0 ? EL_EMC_LENSES : EL_EMPTY);
  game_panel_controls[GAME_PANEL_EMC_LENSES_TIME].value =
    game.lenses_time_left;

  game_panel_controls[GAME_PANEL_EMC_MAGNIFIER].value =
    (game.magnify_time_left > 0 ? EL_EMC_MAGNIFIER : EL_EMPTY);
  game_panel_controls[GAME_PANEL_EMC_MAGNIFIER_TIME].value =
    game.magnify_time_left;

  game_panel_controls[GAME_PANEL_BALLOON_SWITCH].value =
    (game.wind_direction == MV_LEFT  ? EL_BALLOON_SWITCH_LEFT  :
     game.wind_direction == MV_RIGHT ? EL_BALLOON_SWITCH_RIGHT :
     game.wind_direction == MV_UP    ? EL_BALLOON_SWITCH_UP    :
     game.wind_direction == MV_DOWN  ? EL_BALLOON_SWITCH_DOWN  :
     EL_BALLOON_SWITCH_NONE);

  game_panel_controls[GAME_PANEL_DYNABOMB_NUMBER].value =
    local_player->dynabomb_count;
  game_panel_controls[GAME_PANEL_DYNABOMB_SIZE].value =
    local_player->dynabomb_size;
  game_panel_controls[GAME_PANEL_DYNABOMB_POWER].value =
    (local_player->dynabomb_xl ? EL_DYNABOMB_INCREASE_POWER : EL_EMPTY);

  game_panel_controls[GAME_PANEL_PENGUINS].value =
    game.friends_still_needed;

  game_panel_controls[GAME_PANEL_SOKOBAN_OBJECTS].value =
    game.sokoban_objects_still_needed;
  game_panel_controls[GAME_PANEL_SOKOBAN_FIELDS].value =
    game.sokoban_fields_still_needed;

  game_panel_controls[GAME_PANEL_ROBOT_WHEEL].value =
    (game.robot_wheel_active ? EL_ROBOT_WHEEL_ACTIVE : EL_ROBOT_WHEEL);

  for (i = 0; i < NUM_BELTS; i++)
  {
    game_panel_controls[GAME_PANEL_CONVEYOR_BELT_1 + i].value =
      (game.belt_dir[i] != MV_NONE ? EL_CONVEYOR_BELT_1_MIDDLE_ACTIVE :
       EL_CONVEYOR_BELT_1_MIDDLE) + i;
    game_panel_controls[GAME_PANEL_CONVEYOR_BELT_1_SWITCH + i].value =
      getBeltSwitchElementFromBeltNrAndBeltDir(i, game.belt_dir[i]);
  }

  game_panel_controls[GAME_PANEL_MAGIC_WALL].value =
    (game.magic_wall_active ? EL_MAGIC_WALL_ACTIVE : EL_MAGIC_WALL);
  game_panel_controls[GAME_PANEL_MAGIC_WALL_TIME].value =
    game.magic_wall_time_left;

  game_panel_controls[GAME_PANEL_GRAVITY_STATE].value =
    local_player->gravity;

  for (i = 0; i < NUM_PANEL_GRAPHICS; i++)
    game_panel_controls[GAME_PANEL_GRAPHIC_1 + i].value = EL_GRAPHIC_1 + i;

  for (i = 0; i < NUM_PANEL_ELEMENTS; i++)
    game_panel_controls[GAME_PANEL_ELEMENT_1 + i].value =
      (IS_DRAWABLE_ELEMENT(game.panel.element[i].id) ?
       game.panel.element[i].id : EL_UNDEFINED);

  for (i = 0; i < NUM_PANEL_ELEMENTS; i++)
    game_panel_controls[GAME_PANEL_ELEMENT_COUNT_1 + i].value =
      (IS_VALID_ELEMENT(game.panel.element_count[i].id) ?
       element_info[game.panel.element_count[i].id].element_count : 0);

  for (i = 0; i < NUM_PANEL_CE_SCORE; i++)
    game_panel_controls[GAME_PANEL_CE_SCORE_1 + i].value =
      (IS_CUSTOM_ELEMENT(game.panel.ce_score[i].id) ?
       element_info[game.panel.ce_score[i].id].collect_score : 0);

  for (i = 0; i < NUM_PANEL_CE_SCORE; i++)
    game_panel_controls[GAME_PANEL_CE_SCORE_1_ELEMENT + i].value =
      (IS_CUSTOM_ELEMENT(game.panel.ce_score_element[i].id) ?
       element_info[game.panel.ce_score_element[i].id].collect_score :
       EL_UNDEFINED);

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    game_panel_controls[GAME_PANEL_BDX_LIVES].value = game_bd.global_lives;

    for (i = 0; i < NUM_BDX_KEYS; i++)
    {
      int key_value = (i == 0 ? game_bd.game->cave->key1 :
                       i == 1 ? game_bd.game->cave->key2 :
                       i == 2 ? game_bd.game->cave->key3 : 0);
      game_panel_controls[GAME_PANEL_BDX_KEY_1 + i].value =
        (key_value > 0 ? EL_BDX_KEY_1 + i : EL_BDX_EMPTY);
      game_panel_controls[GAME_PANEL_BDX_KEY_1_COUNT + i].value = key_value;
    }

    game_panel_controls[GAME_PANEL_BDX_DIAMOND_KEY].value =
      (game_bd.game->cave->diamond_key_collected ? EL_BDX_DIAMOND_KEY : EL_BDX_EMPTY);

    game_panel_controls[GAME_PANEL_BDX_GRAVITY].value =
      (game_bd.game->cave->gravity == GD_MV_LEFT  ? EL_ARROW_LEFT  :
       game_bd.game->cave->gravity == GD_MV_RIGHT ? EL_ARROW_RIGHT :
       game_bd.game->cave->gravity == GD_MV_UP    ? EL_ARROW_UP    :
       game_bd.game->cave->gravity == GD_MV_DOWN  ? EL_ARROW_DOWN  :
       EL_BDX_EMPTY);

    game_panel_controls[GAME_PANEL_BDX_GRAVITY_NEXT].value =
      (game_bd.game->cave->gravity_next_direction == GD_MV_LEFT  ? EL_ARROW_LEFT  :
       game_bd.game->cave->gravity_next_direction == GD_MV_RIGHT ? EL_ARROW_RIGHT :
       game_bd.game->cave->gravity_next_direction == GD_MV_UP    ? EL_ARROW_UP    :
       game_bd.game->cave->gravity_next_direction == GD_MV_DOWN  ? EL_ARROW_DOWN  :
       game_panel_controls[GAME_PANEL_BDX_GRAVITY].value);

    game_panel_controls[GAME_PANEL_BDX_GRAVITY_TIME].value =
      (game_bd.game->cave->gravity_will_change + 999) / 1000;
    game_panel_controls[GAME_PANEL_BDX_GRAVITY_STATE].value =
      !game_bd.game->cave->gravity_disabled;

    game_panel_controls[GAME_PANEL_BDX_SKELETON].value =
      (game_bd.game->cave->skeletons_collected ? EL_BDX_SKELETON : EL_BDX_EMPTY);
    game_panel_controls[GAME_PANEL_BDX_SKELETON_COUNT].value =
      game_bd.game->cave->skeletons_collected;

    game_panel_controls[GAME_PANEL_BDX_SWEET].value =
      (game_bd.game->cave->sweet_eaten ? EL_BDX_SWEET : EL_BDX_EMPTY);

    game_panel_controls[GAME_PANEL_BDX_PNEUMATIC_HAMMER].value =
      (game_bd.game->cave->got_pneumatic_hammer ?  : EL_BDX_EMPTY);

    game_panel_controls[GAME_PANEL_BDX_ROCKET_COUNT].value =
      game_bd.game->cave->rockets_collected;
    game_panel_controls[GAME_PANEL_BDX_ROCKET_STATE].value =
      game_bd.game->cave->infinite_rockets;

    game_panel_controls[GAME_PANEL_BDX_MAGIC_WALL].value =
      (game_bd.game->cave->magic_wall_state == GD_MW_ACTIVE ?
       EL_BDX_MAGIC_WALL_ACTIVE : EL_BDX_MAGIC_WALL);
    game_panel_controls[GAME_PANEL_BDX_MAGIC_WALL_TIME].value =
      game_bd.game->cave->magic_wall_time;

    game_panel_controls[GAME_PANEL_BDX_CREATURE_SWITCH].value =
      (game_bd.game->cave->creatures_backwards ?
       EL_BDX_CREATURE_SWITCH_ACTIVE : EL_BDX_CREATURE_SWITCH);

    game_panel_controls[GAME_PANEL_BDX_EXPANDABLE_WALL_SWITCH].value =
      (game_bd.game->cave->expanding_wall_changed ?
       EL_BDX_EXPANDABLE_WALL_SWITCH_ACTIVE : EL_BDX_EXPANDABLE_WALL_SWITCH);

    game_panel_controls[GAME_PANEL_BDX_BITER_SWITCH_TIME].value =
      game_bd.game->cave->biter_delay_frame;

    game_panel_controls[GAME_PANEL_BDX_REPLICATOR].value =
      (game_bd.game->cave->replicators_active ?
       EL_BDX_REPLICATOR_ACTIVE : EL_BDX_REPLICATOR);
    game_panel_controls[GAME_PANEL_BDX_REPLICATOR_SWITCH].value =
      (game_bd.game->cave->replicators_active ?
       EL_BDX_REPLICATOR_SWITCH_ACTIVE : EL_BDX_REPLICATOR_SWITCH);

    game_panel_controls[GAME_PANEL_BDX_CONVEYOR_LEFT].value =
      (game_bd.game->cave->conveyor_belts_active ?
       (game_bd.game->cave->conveyor_belts_direction_changed ?
        EL_BDX_CONVEYOR_RIGHT_ACTIVE : EL_BDX_CONVEYOR_LEFT_ACTIVE) :
       (game_bd.game->cave->conveyor_belts_direction_changed ?
        EL_BDX_CONVEYOR_RIGHT : EL_BDX_CONVEYOR_LEFT));
    game_panel_controls[GAME_PANEL_BDX_CONVEYOR_RIGHT].value =
      (game_bd.game->cave->conveyor_belts_active ?
       (game_bd.game->cave->conveyor_belts_direction_changed ?
        EL_BDX_CONVEYOR_LEFT_ACTIVE : EL_BDX_CONVEYOR_RIGHT_ACTIVE) :
       (game_bd.game->cave->conveyor_belts_direction_changed ?
        EL_BDX_CONVEYOR_LEFT : EL_BDX_CONVEYOR_RIGHT));
    game_panel_controls[GAME_PANEL_BDX_CONVEYOR_SWITCH].value =
      (game_bd.game->cave->conveyor_belts_active ?
       EL_BDX_CONVEYOR_SWITCH_ACTIVE : EL_BDX_CONVEYOR_SWITCH);
    game_panel_controls[GAME_PANEL_BDX_CONVEYOR_DIR_SWITCH].value =
      (game_bd.game->cave->conveyor_belts_direction_changed ?
       EL_BDX_CONVEYOR_DIR_SWITCH_ACTIVE : EL_BDX_CONVEYOR_DIR_SWITCH);
  }

  game_panel_controls[GAME_PANEL_PLAYER_NAME].value = 0;
  game_panel_controls[GAME_PANEL_LEVELSET_NAME].value = 0;
  game_panel_controls[GAME_PANEL_LEVEL_NAME].value = 0;
  game_panel_controls[GAME_PANEL_LEVEL_AUTHOR].value = 0;

  // update game panel control frames

  for (i = 0; game_panel_controls[i].nr != -1; i++)
  {
    struct GamePanelControlInfo *gpc = &game_panel_controls[i];

    if (gpc->type == TYPE_ELEMENT)
    {
      if (gpc->value != EL_UNDEFINED && gpc->value != EL_EMPTY)
      {
	int last_anim_random_frame = gfx.anim_random_frame;
	int element = gpc->value;
	int graphic = el2panelimg(element);
	int init_gfx_random = (graphic_info[graphic].anim_global_sync ?
			       sync_random_frame :
			       graphic_info[graphic].anim_global_anim_sync ?
			       getGlobalAnimSyncFrame() : INIT_GFX_RANDOM());

	if (gpc->value != gpc->last_value)
	{
	  gpc->gfx_frame = 0;
	  gpc->gfx_random = init_gfx_random;
	}
	else
	{
	  gpc->gfx_frame++;

	  if (ANIM_MODE(graphic) == ANIM_RANDOM &&
	      IS_NEXT_FRAME(gpc->gfx_frame, graphic))
	    gpc->gfx_random = init_gfx_random;
	}

	if (ANIM_MODE(graphic) == ANIM_RANDOM)
	  gfx.anim_random_frame = gpc->gfx_random;

	if (ANIM_MODE(graphic) == ANIM_CE_SCORE)
	  gpc->gfx_frame = element_info[element].collect_score;

	gpc->frame = getGraphicAnimationFrame(graphic, gpc->gfx_frame);

	if (ANIM_MODE(graphic) == ANIM_RANDOM)
	  gfx.anim_random_frame = last_anim_random_frame;
      }
    }
    else if (gpc->type == TYPE_GRAPHIC)
    {
      if (gpc->graphic != IMG_UNDEFINED)
      {
	int last_anim_random_frame = gfx.anim_random_frame;
	int graphic = gpc->graphic;
	int init_gfx_random = (graphic_info[graphic].anim_global_sync ?
			       sync_random_frame :
			       graphic_info[graphic].anim_global_anim_sync ?
			       getGlobalAnimSyncFrame() : INIT_GFX_RANDOM());

	if (gpc->value != gpc->last_value)
	{
	  gpc->gfx_frame = 0;
	  gpc->gfx_random = init_gfx_random;
	}
	else
	{
	  gpc->gfx_frame++;

	  if (ANIM_MODE(graphic) == ANIM_RANDOM &&
	      IS_NEXT_FRAME(gpc->gfx_frame, graphic))
	    gpc->gfx_random = init_gfx_random;
	}

	if (ANIM_MODE(graphic) == ANIM_RANDOM)
	  gfx.anim_random_frame = gpc->gfx_random;

	gpc->frame = getGraphicAnimationFrame(graphic, gpc->gfx_frame);

	if (ANIM_MODE(graphic) == ANIM_RANDOM)
	  gfx.anim_random_frame = last_anim_random_frame;
      }
    }
  }
}

static void DisplayGameControlValuesExt(boolean force_redraw)
{
  boolean redraw_panel = force_redraw;
  int i;

  for (i = 0; game_panel_controls[i].nr != -1; i++)
  {
    struct GamePanelControlInfo *gpc = &game_panel_controls[i];

    if (PANEL_DEACTIVATED(gpc->pos))
      continue;

    if (gpc->value == gpc->last_value &&
	gpc->frame == gpc->last_frame)
      continue;

    redraw_panel = TRUE;
  }

  if (!redraw_panel)
    return;

  // copy default game door content to main double buffer

  // !!! CHECK AGAIN !!!
  SetPanelBackground();
  // SetDoorBackgroundImage(IMG_BACKGROUND_PANEL);
  DrawBackground(DX, DY, DXSIZE, DYSIZE);

  // redraw game control buttons
  RedrawGameButtons();

  SetGameStatus(GAME_MODE_PSEUDO_PANEL);

  for (i = 0; i < NUM_GAME_PANEL_CONTROLS; i++)
  {
    int nr = game_panel_order[i].nr;
    struct GamePanelControlInfo *gpc = &game_panel_controls[nr];
    struct TextPosInfo *pos = gpc->pos;
    int type = gpc->type;
    int value = gpc->value;
    int frame = gpc->frame;
    int size = pos->size;
    int font = pos->font;
    boolean draw_masked = pos->draw_masked;
    int mask_mode = (draw_masked ? BLIT_MASKED : BLIT_OPAQUE);

    if (PANEL_DEACTIVATED(pos))
      continue;

    if (hasClass(pos->class, CLASS_EXTRA_PANEL_ITEMS) &&
	!setup.show_extra_panel_items &&
        !game.panel.show_extra_items)
      continue;

    if (hasClass(pos->class, CLASS_BD_PRE_HATCHING) &&
        (level.game_engine_type != GAME_ENGINE_TYPE_BD || game_bd.game->cave->hatched))
      continue;

    if (hasClass(pos->class, CLASS_BD_POST_HATCHING) &&
        (level.game_engine_type == GAME_ENGINE_TYPE_BD && !game_bd.game->cave->hatched))
      continue;

    gpc->last_value = value;
    gpc->last_frame = frame;

    if (type == TYPE_INTEGER)
    {
      if (nr == GAME_PANEL_LEVEL_NUMBER ||
	  nr == GAME_PANEL_INVENTORY_COUNT ||
	  nr == GAME_PANEL_SCORE ||
	  nr == GAME_PANEL_HIGHSCORE ||
	  nr == GAME_PANEL_TIME)
      {
	boolean use_dynamic_size = (size == -1 ? TRUE : FALSE);

	if (use_dynamic_size)		// use dynamic number of digits
	{
	  int value_change = (nr == GAME_PANEL_LEVEL_NUMBER ? 100 :
			      nr == GAME_PANEL_INVENTORY_COUNT ||
			      nr == GAME_PANEL_TIME ? 1000 : 100000);
	  int size_add = (nr == GAME_PANEL_LEVEL_NUMBER ||
			  nr == GAME_PANEL_INVENTORY_COUNT ||
			  nr == GAME_PANEL_TIME ? 1 : 2);
	  int size1 = (nr == GAME_PANEL_LEVEL_NUMBER ? 2 :
		       nr == GAME_PANEL_INVENTORY_COUNT ||
		       nr == GAME_PANEL_TIME ? 3 : 5);
	  int size2 = size1 + size_add;
	  int font1 = pos->font;
	  int font2 = pos->font_alt;

	  size = (value < value_change ? size1 : size2);
	  font = (value < value_change ? font1 : font2);
	}
      }

      // correct text size if "digits" is zero or less
      if (size <= 0)
	size = strlen(int2str(value, size));

      // dynamically correct text alignment
      pos->width = size * getFontWidth(font);

      DrawTextExt(drawto, PANEL_XPOS(pos), PANEL_YPOS(pos),
		  int2str(value, size), font, mask_mode);
    }
    else if (type == TYPE_ELEMENT)
    {
      int element, graphic;
      Bitmap *src_bitmap;
      int src_x, src_y;
      int width, height;
      int dst_x = PANEL_XPOS(pos);
      int dst_y = PANEL_YPOS(pos);

      if (value != EL_UNDEFINED && value != EL_EMPTY)
      {
	element = value;
	graphic = el2panelimg(value);

#if 0
	Debug("game:DisplayGameControlValues", "%d, '%s' [%d]",
	      element, EL_NAME(element), size);
#endif

	if (element >= EL_GRAPHIC_1 && element <= EL_GRAPHIC_8 && size == 0)
	  size = TILESIZE;

	getSizedGraphicSource(graphic, frame, size, &src_bitmap,
			      &src_x, &src_y);

	width  = graphic_info[graphic].width  * size / TILESIZE;
	height = graphic_info[graphic].height * size / TILESIZE;

	if (draw_masked)
	  BlitBitmapMasked(src_bitmap, drawto, src_x, src_y, width, height,
			   dst_x, dst_y);
	else
	  BlitBitmap(src_bitmap, drawto, src_x, src_y, width, height,
		     dst_x, dst_y);
      }
    }
    else if (type == TYPE_GRAPHIC)
    {
      int graphic        = gpc->graphic;
      int graphic_active = gpc->graphic_active;
      Bitmap *src_bitmap;
      int src_x, src_y;
      int width, height;
      int dst_x = PANEL_XPOS(pos);
      int dst_y = PANEL_YPOS(pos);
      boolean skip = (hasClass(pos->class, CLASS_MM_ENGINE_ONLY) &&
		      level.game_engine_type != GAME_ENGINE_TYPE_MM);

      if (graphic != IMG_UNDEFINED && !skip)
      {
	if (pos->style == STYLE_REVERSE)
	  value = 100 - value;

	getGraphicSource(graphic_active, frame, &src_bitmap, &src_x, &src_y);

	if (pos->direction & MV_HORIZONTAL)
	{
	  width  = graphic_info[graphic_active].width * value / 100;
	  height = graphic_info[graphic_active].height;

	  if (pos->direction == MV_LEFT)
	  {
	    src_x += graphic_info[graphic_active].width - width;
	    dst_x += graphic_info[graphic_active].width - width;
	  }
	}
	else
	{
	  width  = graphic_info[graphic_active].width;
	  height = graphic_info[graphic_active].height * value / 100;

	  if (pos->direction == MV_UP)
	  {
	    src_y += graphic_info[graphic_active].height - height;
	    dst_y += graphic_info[graphic_active].height - height;
	  }
	}

	if (draw_masked)
	  BlitBitmapMasked(src_bitmap, drawto, src_x, src_y, width, height,
			   dst_x, dst_y);
	else
	  BlitBitmap(src_bitmap, drawto, src_x, src_y, width, height,
		     dst_x, dst_y);

	getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

	if (pos->direction & MV_HORIZONTAL)
	{
	  if (pos->direction == MV_RIGHT)
	  {
	    src_x += width;
	    dst_x += width;
	  }
	  else
	  {
	    dst_x = PANEL_XPOS(pos);
	  }

	  width = graphic_info[graphic].width - width;
	}
	else
	{
	  if (pos->direction == MV_DOWN)
	  {
	    src_y += height;
	    dst_y += height;
	  }
	  else
	  {
	    dst_y = PANEL_YPOS(pos);
	  }

	  height = graphic_info[graphic].height - height;
	}

	if (draw_masked)
	  BlitBitmapMasked(src_bitmap, drawto, src_x, src_y, width, height,
			   dst_x, dst_y);
	else
	  BlitBitmap(src_bitmap, drawto, src_x, src_y, width, height,
		     dst_x, dst_y);
      }
    }
    else if (type == TYPE_STRING)
    {
      boolean active = (value != 0);
      char *state_normal = "off";
      char *state_active = "on";
      char *state = (active ? state_active : state_normal);
      char *s = (nr == GAME_PANEL_GRAVITY_STATE ? state :
		 nr == GAME_PANEL_PLAYER_NAME   ? setup.player_name :
		 nr == GAME_PANEL_LEVELSET_NAME ? leveldir_current->name :
		 nr == GAME_PANEL_LEVEL_NAME    ? level.name :
		 nr == GAME_PANEL_LEVEL_AUTHOR  ? level.author : NULL);

      if (nr == GAME_PANEL_GRAVITY_STATE)
      {
	int font1 = pos->font;		// (used for normal state)
	int font2 = pos->font_alt;	// (used for active state)

	font = (active ? font2 : font1);
      }
      else if (nr == GAME_PANEL_PLAYER_NAME ||
               nr == GAME_PANEL_LEVELSET_NAME ||
               nr == GAME_PANEL_LEVEL_NAME ||
               nr == GAME_PANEL_LEVEL_AUTHOR)
      {
        if (nr == GAME_PANEL_LEVEL_NAME &&
            level.game_engine_type == GAME_ENGINE_TYPE_BD && setup.bd_multiple_lives)
        {
          static char *level_name_and_lives = NULL;

          setStringPrint(&level_name_and_lives, "%s (%d)", s, game_bd.global_lives);

          s = level_name_and_lives;
        }

        // use alternative (narrow) font if text larger than specified size or game panel width
        if ((size > 0 && strlen(s) > size) || strlen(s) * getFontWidth(font) > DXSIZE)
          font = pos->font_alt;
      }

      if (s != NULL)
      {
	char *s_cut;

	if (size <= 0)
	{
	  // don't truncate output if "chars" is zero or less
	  size = strlen(s);

	  // dynamically correct text alignment
	  pos->width = size * getFontWidth(font);
	}

	s_cut = getStringCopyN(s, size);

	DrawTextExt(drawto, PANEL_XPOS(pos), PANEL_YPOS(pos),
		    s_cut, font, mask_mode);

	free(s_cut);
      }
    }

    redraw_mask |= REDRAW_DOOR_1;
  }

  SetGameStatus(GAME_MODE_PLAYING);
}

static void DisplayGameControlValues(void)
{
  DisplayGameControlValuesExt(FALSE);
}

static void DisplayGameControlValues_ForceRedraw(void)
{
  DisplayGameControlValuesExt(TRUE);
}

void UpdateAndDisplayGameControlValues(void)
{
  if (tape.deactivate_display)
    return;

  UpdateGameControlValues();
  DisplayGameControlValues();
}

void UpdateGameDoorValues(void)
{
  UpdateGameControlValues();
}

void DrawGameDoorValues(void)
{
  DisplayGameControlValues();
}

void DrawGameDoorValues_ForceRedraw(void)
{
  DisplayGameControlValues_ForceRedraw();
}


// ============================================================================
// InitGameEngine()
// ----------------------------------------------------------------------------
// initialize game engine due to level / tape version number
// ============================================================================

static void InitGameEngine(void)
{
  int i, j, k, l, x, y;

  // set game engine from tape file when re-playing, else from level file
  game.engine_version = (tape.playing ? tape.engine_version :
			 level.game_version);

  // set single or multi-player game mode (needed for re-playing tapes)
  game.team_mode = setup.team_mode;

  if (tape.playing)
  {
    int num_players = 0;

    for (i = 0; i < MAX_PLAYERS; i++)
      if (tape.player_participates[i])
	num_players++;

    // multi-player tapes contain input data for more than one player
    game.team_mode = (num_players > 1);
  }

#if 0
  Debug("game:init:level", "level %d: level.game_version  == %s", level_nr,
	getVersionString(level.game_version));
  Debug("game:init:level", "          tape.file_version   == %s",
	getVersionString(tape.file_version));
  Debug("game:init:level", "          tape.game_version   == %s",
	getVersionString(tape.game_version));
  Debug("game:init:level", "          tape.engine_version == %s",
	getVersionString(tape.engine_version));
  Debug("game:init:level", "       => game.engine_version == %s [tape mode: %s]",
	getVersionString(game.engine_version), (tape.playing ? "PLAYING" : "RECORDING"));
#endif

  // --------------------------------------------------------------------------
  // set flags for bugs and changes according to active game engine version
  // --------------------------------------------------------------------------

  /*
    Summary of bugfix:
    Fixed property "can fall" for run-time element "EL_AMOEBA_DROPPING"

    Bug was introduced in version:
    2.0.1

    Bug was fixed in version:
    4.2.0.0

    Description:
    In version 2.0.1, a new run-time element "EL_AMOEBA_DROPPING" was added,
    but the property "can fall" was missing, which caused some levels to be
    unsolvable. This was fixed in version 4.2.0.0.

    Affected levels/tapes:
    An example for a tape that was fixed by this bugfix is tape 029 from the
    level set "rnd_sam_bateman".
    The wrong behaviour will still be used for all levels or tapes that were
    created/recorded with it. An example for this is tape 023 from the level
    set "rnd_gerhard_haeusler", which was recorded with a buggy game engine.
  */

  boolean use_amoeba_dropping_cannot_fall_bug =
    ((game.engine_version >= VERSION_IDENT(2,0,1,0) &&
      game.engine_version <  VERSION_IDENT(4,2,0,0)) ||
     (tape.playing &&
      tape.game_version >= VERSION_IDENT(2,0,1,0) &&
      tape.game_version <  VERSION_IDENT(4,2,0,0)));

  /*
    Summary of bugfix/change:
    Fixed move speed of elements entering or leaving magic wall.

    Fixed/changed in version:
    2.0.1

    Description:
    Before 2.0.1, move speed of elements entering or leaving magic wall was
    twice as fast as it is now.
    Since 2.0.1, this is set to a lower value by using move_stepsize_list[].

    Affected levels/tapes:
    The first condition is generally needed for all levels/tapes before version
    2.0.1, which might use the old behaviour before it was changed; known tapes
    that are affected: Tape 014 from the level set "rnd_conor_mancone".
    The second condition is an exception from the above case and is needed for
    the special case of tapes recorded with game (not engine!) version 2.0.1 or
    above, but before it was known that this change would break tapes like the
    above and was fixed in 4.2.0.0, so that the changed behaviour was active
    although the engine version while recording maybe was before 2.0.1. There
    are a lot of tapes that are affected by this exception, like tape 006 from
    the level set "rnd_conor_mancone".
  */

  boolean use_old_move_stepsize_for_magic_wall =
    (game.engine_version < VERSION_IDENT(2,0,1,0) &&
     !(tape.playing &&
       tape.game_version >= VERSION_IDENT(2,0,1,0) &&
       tape.game_version <  VERSION_IDENT(4,2,0,0)));

  /*
    Summary of bugfix/change:
    Fixed handling for custom elements that change when pushed by the player.

    Fixed/changed in version:
    3.1.0

    Description:
    Before 3.1.0, custom elements that "change when pushing" changed directly
    after the player started pushing them (until then handled in "DigField()").
    Since 3.1.0, these custom elements are not changed until the "pushing"
    move of the element is finished (now handled in "ContinueMoving()").

    Affected levels/tapes:
    The first condition is generally needed for all levels/tapes before version
    3.1.0, which might use the old behaviour before it was changed; known tapes
    that are affected are some tapes from the level set "Walpurgis Gardens" by
    Jamie Cullen.
    The second condition is an exception from the above case and is needed for
    the special case of tapes recorded with game (not engine!) version 3.1.0 or
    above (including some development versions of 3.1.0), but before it was
    known that this change would break tapes like the above and was fixed in
    3.1.1, so that the changed behaviour was active although the engine version
    while recording maybe was before 3.1.0. There is at least one tape that is
    affected by this exception, which is the tape for the one-level set "Bug
    Machine" by Juergen Bonhagen.
  */

  game.use_change_when_pushing_bug =
    (game.engine_version < VERSION_IDENT(3,1,0,0) &&
     !(tape.playing &&
       tape.game_version >= VERSION_IDENT(3,1,0,0) &&
       tape.game_version <  VERSION_IDENT(3,1,1,0)));

  /*
    Summary of bugfix/change:
    Fixed handling for blocking the field the player leaves when moving.

    Fixed/changed in version:
    3.1.1

    Description:
    Before 3.1.1, when "block last field when moving" was enabled, the field
    the player is leaving when moving was blocked for the time of the move,
    and was directly unblocked afterwards. This resulted in the last field
    being blocked for exactly one less than the number of frames of one player
    move. Additionally, even when blocking was disabled, the last field was
    blocked for exactly one frame.
    Since 3.1.1, due to changes in player movement handling, the last field
    is not blocked at all when blocking is disabled. When blocking is enabled,
    the last field is blocked for exactly the number of frames of one player
    move. Additionally, if the player is Murphy, the hero of Supaplex, the
    last field is blocked for exactly one more than the number of frames of
    one player move.

    Affected levels/tapes:
    (!!! yet to be determined -- probably many !!!)
  */

  game.use_block_last_field_bug =
    (game.engine_version < VERSION_IDENT(3,1,1,0));

  /* various special flags and settings for native Emerald Mine game engine */

  game_em.use_single_button =
    (game.engine_version > VERSION_IDENT(4,0,0,2));

  game_em.use_push_delay =
    (game.engine_version > VERSION_IDENT(4,3,7,1));

  game_em.use_snap_key_bug =
    (game.engine_version < VERSION_IDENT(4,0,1,0));

  game_em.use_random_bug =
    (tape.property_bits & TAPE_PROPERTY_EM_RANDOM_BUG);

  boolean use_old_em_engine = (game.engine_version < VERSION_IDENT(4,2,0,0));

  game_em.use_old_explosions		= use_old_em_engine;
  game_em.use_old_android		= use_old_em_engine;
  game_em.use_old_push_elements		= use_old_em_engine;
  game_em.use_old_push_into_acid	= use_old_em_engine;

  game_em.use_wrap_around		= !use_old_em_engine;

  // --------------------------------------------------------------------------

  // set maximal allowed number of custom element changes per game frame
  game.max_num_changes_per_frame = 1;

  // default scan direction: scan playfield from top/left to bottom/right
  InitPlayfieldScanMode(CA_ARG_SCAN_MODE_NORMAL);

  // dynamically adjust element properties according to game engine version
  InitElementPropertiesEngine(game.engine_version);

  // ---------- initialize special element properties -------------------------

  // "EL_AMOEBA_DROPPING" missed property "can fall" in older game versions
  if (use_amoeba_dropping_cannot_fall_bug)
    SET_PROPERTY(EL_AMOEBA_DROPPING, EP_CAN_FALL, FALSE);

  // ---------- initialize player's initial move delay ------------------------

  // dynamically adjust player properties according to level information
  for (i = 0; i < MAX_PLAYERS; i++)
    game.initial_move_delay_value[i] =
      get_move_delay_from_stepsize(level.initial_player_stepsize[i]);

  // dynamically adjust player properties according to game engine version
  for (i = 0; i < MAX_PLAYERS; i++)
    game.initial_move_delay[i] =
      (game.engine_version <= VERSION_IDENT(2,0,1,0) ?
       game.initial_move_delay_value[i] : 0);

  // ---------- initialize player's initial push delay ------------------------

  // dynamically adjust player properties according to game engine version
  game.initial_push_delay_value =
    (game.engine_version < VERSION_IDENT(3,0,7,1) ? 5 : -1);

  // ---------- initialize changing elements ----------------------------------

  // initialize changing elements information
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    // this pointer might have been changed in the level editor
    ei->change = &ei->change_page[0];

    if (!IS_CUSTOM_ELEMENT(i))
    {
      ei->change->target_element = EL_EMPTY_SPACE;
      ei->change->delay_fixed = 0;
      ei->change->delay_random = 0;
      ei->change->delay_frames = 1;
    }

    for (j = 0; j < NUM_CHANGE_EVENTS; j++)
    {
      ei->has_change_event[j] = FALSE;

      ei->event_page_nr[j] = 0;
      ei->event_page[j] = &ei->change_page[0];
    }
  }

  // add changing elements from pre-defined list
  for (i = 0; change_delay_list[i].element != EL_UNDEFINED; i++)
  {
    struct ChangingElementInfo *ch_delay = &change_delay_list[i];
    struct ElementInfo *ei = &element_info[ch_delay->element];

    ei->change->target_element       = ch_delay->target_element;
    ei->change->delay_fixed          = ch_delay->change_delay;

    ei->change->pre_change_function  = ch_delay->pre_change_function;
    ei->change->change_function      = ch_delay->change_function;
    ei->change->post_change_function = ch_delay->post_change_function;

    ei->change->can_change = TRUE;
    ei->change->can_change_or_has_action = TRUE;

    ei->has_change_event[CE_DELAY] = TRUE;

    SET_PROPERTY(ch_delay->element, EP_CAN_CHANGE, TRUE);
    SET_PROPERTY(ch_delay->element, EP_CAN_CHANGE_OR_HAS_ACTION, TRUE);
  }

  // ---------- initialize if element can trigger global animations -----------

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    ei->has_anim_event = FALSE;
  }

  InitGlobalAnimEventsForCustomElements();

  // ---------- initialize internal run-time variables ------------------------

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[EL_CUSTOM_START + i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      ei->change_page[j].can_change_or_has_action =
	(ei->change_page[j].can_change |
	 ei->change_page[j].has_action);
    }
  }

  // add change events from custom element configuration
  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[EL_CUSTOM_START + i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      if (!ei->change_page[j].can_change_or_has_action)
	continue;

      for (k = 0; k < NUM_CHANGE_EVENTS; k++)
      {
	// only add event page for the first page found with this event
	if (ei->change_page[j].has_event[k] && !(ei->has_change_event[k]))
	{
	  ei->has_change_event[k] = TRUE;

	  ei->event_page_nr[k] = j;
	  ei->event_page[k] = &ei->change_page[j];
	}
      }
    }
  }

  // ---------- initialize reference elements in change conditions ------------

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;
    struct ElementInfo *ei = &element_info[element];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      int trigger_element = ei->change_page[j].initial_trigger_element;

      if (trigger_element >= EL_PREV_CE_8 &&
	  trigger_element <= EL_NEXT_CE_8)
	trigger_element = RESOLVED_REFERENCE_ELEMENT(element, trigger_element);

      ei->change_page[j].trigger_element = trigger_element;
    }
  }

  // ---------- initialize run-time trigger player and element ----------------

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[EL_CUSTOM_START + i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      struct ElementChangeInfo *change = &ei->change_page[j];

      change->actual_trigger_element = EL_EMPTY;
      change->actual_trigger_player = EL_EMPTY;
      change->actual_trigger_player_bits = CH_PLAYER_NONE;
      change->actual_trigger_side = CH_SIDE_NONE;
      change->actual_trigger_ce_value = 0;
      change->actual_trigger_ce_score = 0;
      change->actual_trigger_x = -1;
      change->actual_trigger_y = -1;
    }
  }

  // ---------- initialize trigger events -------------------------------------

  // initialize trigger events information
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (j = 0; j < NUM_CHANGE_EVENTS; j++)
      trigger_events[i][j] = FALSE;

  // add trigger events from element change event properties
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    for (j = 0; j < ei->num_change_pages; j++)
    {
      struct ElementChangeInfo *change = &ei->change_page[j];

      if (!change->can_change_or_has_action)
	continue;

      if (change->has_event[CE_BY_OTHER_ACTION])
      {
	int trigger_element = change->trigger_element;

	for (k = 0; k < NUM_CHANGE_EVENTS; k++)
	{
	  if (change->has_event[k])
	  {
	    if (IS_GROUP_ELEMENT(trigger_element))
	    {
	      struct ElementGroupInfo *group =
		element_info[trigger_element].group;

	      for (l = 0; l < group->num_elements_resolved; l++)
		trigger_events[group->element_resolved[l]][k] = TRUE;
	    }
	    else if (trigger_element == EL_ANY_ELEMENT)
	      for (l = 0; l < MAX_NUM_ELEMENTS; l++)
		trigger_events[l][k] = TRUE;
	    else
	      trigger_events[trigger_element][k] = TRUE;
	  }
	}
      }
    }
  }

  // ---------- initialize push delay -----------------------------------------

  // initialize push delay values to default
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (!IS_CUSTOM_ELEMENT(i))
    {
      // set default push delay values (corrected since version 3.0.7-1)
      if (game.engine_version < VERSION_IDENT(3,0,7,1))
      {
	element_info[i].push_delay_fixed = 2;
	element_info[i].push_delay_random = 8;
      }
      else
      {
	element_info[i].push_delay_fixed = 8;
	element_info[i].push_delay_random = 8;
      }
    }
  }

  // set push delay value for certain elements from pre-defined list
  for (i = 0; push_delay_list[i].element != EL_UNDEFINED; i++)
  {
    int e = push_delay_list[i].element;

    element_info[e].push_delay_fixed  = push_delay_list[i].push_delay_fixed;
    element_info[e].push_delay_random = push_delay_list[i].push_delay_random;
  }

  // set push delay value for Supaplex elements for newer engine versions
  if (game.engine_version >= VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    {
      if (IS_SP_ELEMENT(i))
      {
	// set SP push delay to just enough to push under a falling zonk
	int delay = (game.engine_version >= VERSION_IDENT(3,1,1,0) ? 8 : 6);

	element_info[i].push_delay_fixed  = delay;
	element_info[i].push_delay_random = 0;
      }
    }
  }

  // ---------- initialize move stepsize --------------------------------------

  // initialize move stepsize values to default
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].move_stepsize = MOVE_STEPSIZE_NORMAL;

  // set move stepsize value for certain elements from pre-defined list
  for (i = 0; move_stepsize_list[i].element != EL_UNDEFINED; i++)
  {
    int e = move_stepsize_list[i].element;

    element_info[e].move_stepsize = move_stepsize_list[i].move_stepsize;

    // set move stepsize value for certain elements for older engine versions
    if (use_old_move_stepsize_for_magic_wall)
    {
      if (e == EL_MAGIC_WALL_FILLING ||
	  e == EL_MAGIC_WALL_EMPTYING ||
	  e == EL_BD_MAGIC_WALL_FILLING ||
	  e == EL_BD_MAGIC_WALL_EMPTYING)
	element_info[e].move_stepsize *= 2;
    }
  }

  // ---------- initialize collect score --------------------------------------

  // initialize collect score values for custom elements from initial value
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (IS_CUSTOM_ELEMENT(i))
      element_info[i].collect_score = element_info[i].collect_score_initial;

  // ---------- initialize collect count --------------------------------------

  // initialize collect count values for non-custom elements
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].collect_count_initial = 0;

  // add collect count values for all elements from pre-defined list
  for (i = 0; collect_count_list[i].element != EL_UNDEFINED; i++)
    element_info[collect_count_list[i].element].collect_count_initial =
      collect_count_list[i].count;

  // ---------- initialize access direction -----------------------------------

  // initialize access direction values to default (access from every side)
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CUSTOM_ELEMENT(i))
      element_info[i].access_direction = MV_ALL_DIRECTIONS;

  // set access direction value for certain elements from pre-defined list
  for (i = 0; access_direction_list[i].element != EL_UNDEFINED; i++)
    element_info[access_direction_list[i].element].access_direction =
      access_direction_list[i].direction;

  // ---------- initialize explosion content ----------------------------------
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (IS_CUSTOM_ELEMENT(i))
      continue;

    for (y = 0; y < 3; y++) for (x = 0; x < 3; x++)
    {
      // (content for EL_YAMYAM set at run-time with game.yamyam_content_nr)

      element_info[i].content.e[x][y] =
	(i == EL_PLAYER_1 ? EL_EMERALD_YELLOW :
	 i == EL_PLAYER_2 ? EL_EMERALD_RED :
	 i == EL_PLAYER_3 ? EL_EMERALD :
	 i == EL_PLAYER_4 ? EL_EMERALD_PURPLE :
	 i == EL_MOLE ? EL_EMERALD_RED :
	 i == EL_PENGUIN ? EL_EMERALD_PURPLE :
	 i == EL_BUG ? (x == 1 && y == 1 ? EL_DIAMOND : EL_EMERALD) :
	 i == EL_BD_BUTTERFLY ? EL_BD_DIAMOND :
	 i == EL_SP_ELECTRON ? EL_SP_INFOTRON :
	 i == EL_AMOEBA_TO_DIAMOND ? level.amoeba_content :
	 i == EL_WALL_EMERALD ? EL_EMERALD :
	 i == EL_WALL_DIAMOND ? EL_DIAMOND :
	 i == EL_WALL_BD_DIAMOND ? EL_BD_DIAMOND :
	 i == EL_WALL_EMERALD_YELLOW ? EL_EMERALD_YELLOW :
	 i == EL_WALL_EMERALD_RED ? EL_EMERALD_RED :
	 i == EL_WALL_EMERALD_PURPLE ? EL_EMERALD_PURPLE :
	 i == EL_WALL_PEARL ? EL_PEARL :
	 i == EL_WALL_CRYSTAL ? EL_CRYSTAL :
	 EL_EMPTY);
    }
  }

  // ---------- initialize recursion detection --------------------------------
  recursion_loop_depth = 0;
  recursion_loop_detected = FALSE;
  recursion_loop_element = EL_UNDEFINED;

  // ---------- initialize graphics engine ------------------------------------
  game.scroll_delay_value =
    (game.forced_scroll_delay_value != -1 ? game.forced_scroll_delay_value :
     level.game_engine_type == GAME_ENGINE_TYPE_EM &&
     !setup.forced_scroll_delay           ? 0 :
     setup.scroll_delay                   ? setup.scroll_delay_value       : 0);
  if (game.forced_scroll_delay_value == -1)
    game.scroll_delay_value =
      MIN(MAX(MIN_SCROLL_DELAY, game.scroll_delay_value), MAX_SCROLL_DELAY);

  // ---------- initialize game engine snapshots ------------------------------
  for (i = 0; i < MAX_PLAYERS; i++)
    game.snapshot.last_action[i] = 0;
  game.snapshot.changed_action = FALSE;
  game.snapshot.collected_item = FALSE;
  game.snapshot.mode =
    (strEqual(setup.engine_snapshot_mode, STR_SNAPSHOT_MODE_EVERY_STEP) ?
     SNAPSHOT_MODE_EVERY_STEP :
     strEqual(setup.engine_snapshot_mode, STR_SNAPSHOT_MODE_EVERY_MOVE) ?
     SNAPSHOT_MODE_EVERY_MOVE :
     strEqual(setup.engine_snapshot_mode, STR_SNAPSHOT_MODE_EVERY_COLLECT) ?
     SNAPSHOT_MODE_EVERY_COLLECT : SNAPSHOT_MODE_OFF);
  game.snapshot.save_snapshot = FALSE;

  // ---------- initialize level time for Supaplex engine ---------------------
  // Supaplex levels with time limit currently unsupported -- should be added
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    level.time = 0;

  // ---------- initialize flags for handling game actions --------------------

  // set flags for game actions to default values
  game.use_key_actions = TRUE;
  game.use_mouse_actions = FALSE;

  // when using Mirror Magic game engine, handle mouse events only
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    game.use_key_actions = FALSE;
    game.use_mouse_actions = TRUE;
  }

  // check for custom elements with mouse click events
  if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (HAS_ANY_CHANGE_EVENT(element, CE_CLICKED_BY_MOUSE) ||
	  HAS_ANY_CHANGE_EVENT(element, CE_PRESSED_BY_MOUSE) ||
	  HAS_ANY_CHANGE_EVENT(element, CE_MOUSE_CLICKED_ON_X) ||
	  HAS_ANY_CHANGE_EVENT(element, CE_MOUSE_PRESSED_ON_X))
	game.use_mouse_actions = TRUE;
    }
  }
}

static int get_num_special_action(int element, int action_first,
				  int action_last)
{
  int num_special_action = 0;
  int i, j;

  for (i = action_first; i <= action_last; i++)
  {
    boolean found = FALSE;

    for (j = 0; j < NUM_DIRECTIONS; j++)
      if (el_act_dir2img(element, i, j) !=
	  el_act_dir2img(element, ACTION_DEFAULT, j))
	found = TRUE;

    if (found)
      num_special_action++;
    else
      break;
  }

  return num_special_action;
}


// ============================================================================
// InitGame()
// ----------------------------------------------------------------------------
// initialize and start new game
// ============================================================================

#if DEBUG_INIT_PLAYER
static void DebugPrintPlayerStatus(char *message)
{
  int i;

  if (!options.debug)
    return;

  Debug("game:init:player", "%s:", message);

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    Debug("game:init:player",
	  "- player %d: present == %d, connected == %d [%d/%d], active == %d%s",
	  i + 1,
	  player->present,
	  player->connected,
	  player->connected_locally,
	  player->connected_network,
	  player->active,
	  (local_player == player ? " (local player)" : ""));
  }
}
#endif

static boolean checkKeepPlayingSoundsWhenRestarting(boolean restarting)
{
  // keep playing continuous loop sound during covering and uncovering screen for BD engine
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD &&
      restarting && game.keep_panel_open_when_restarting)
    return TRUE;

  return FALSE;
}

static boolean checkCloseGamePanelDoor(boolean restarting)
{
  // do not close game panel door if game restart was triggered by CE action
  if (game.restart_level)
    return FALSE;

  // do not close game panel door if configured to keep open when restarting game
  if (restarting && game.keep_panel_open_when_restarting)
    return FALSE;

  // always close game panel door in all other cases
  return TRUE;
}

void InitGame(void)
{
  static int last_level_nr = -1;
  static int last_game_engine_type = -1;
  int full_lev_fieldx = lev_fieldx + (BorderElement != EL_EMPTY ? 2 : 0);
  int full_lev_fieldy = lev_fieldy + (BorderElement != EL_EMPTY ? 2 : 0);
  int fade_mask = REDRAW_FIELD;
  boolean restarting = (game_status == GAME_MODE_PLAYING);
  boolean restarting_same_level = (restarting && level_nr == last_level_nr);
  boolean level_story = (game_status == GAME_MODE_STORY);
  boolean level_story_after_main = (level_story && game_status_last_screen == GAME_MODE_MAIN);
  boolean starting_from_main = (game_status == GAME_MODE_MAIN || level_story_after_main);
  boolean last_game_engine_type_was_bd = (last_game_engine_type == GAME_ENGINE_TYPE_BD);
  boolean emulate_bd = TRUE;	// unless non-BOULDERDASH elements found
  boolean emulate_sp = TRUE;	// unless non-SUPAPLEX    elements found
  int initial_move_dir = MV_DOWN;
  int i, j, x, y;

  // store current level number to be able to detect restarting of same level later
  last_level_nr = level_nr;

  // store current game engine type to be able to cover BD screen of last level later
  last_game_engine_type = level.game_engine_type;

  // show level info before starting the game (if any exists)
  if (!level_editor_test_game && !restarting_same_level && !level_story && !tape.playing)
    if (ShowStoryScreen_FromInitGame())
      return;

  // required to prevent handling game actions when moving doors (via "checkGameEnded()")
  game.InitGameRequested = TRUE;

  // required here to update video display before fading (FIX THIS)
  DrawMaskedBorder(REDRAW_DOOR_2);

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    if (!setup.bd_multiple_lives)
    {
      // newly started BD game with normal, single life (resetting global score is important here)
      game_bd.global_lives = 0;
      game_bd.global_score = 0;
    }
    else if (restarting_same_level && game_bd.global_lives > 1)
    {
      // restarted BD game with multiple (and remaining) lives, so decrement number of lives
      game_bd.global_lives--;

      // subtract another life for restarted intermissions (extra life will be added again later)
      if (level.bd_intermission)
        game_bd.global_lives--;
    }
    else if (restarting_same_level || starting_from_main)
    {
      // newly started BD game with multiple lives (or restarted BD game, but no remaining lives),
      // so set initial number of lives and reset global score
      game_bd.global_lives = level.native_bd_level->caveset->initial_lives;
      game_bd.global_score = 0;
    }
  }

  if (restarting)
  {
    // force fading out global animations displayed during game play
    SetGameStatus(GAME_MODE_PSEUDO_RESTARTING);

    // when last level was using BD engine, cover screen before fading out when restarting game
    if (last_game_engine_type_was_bd)
    {
      game_bd.cover_screen = TRUE;

      // skip fading when covering screen, but only if not also skipping BD style uncovering
      if (level.game_engine_type == GAME_ENGINE_TYPE_BD && !setup.bd_skip_uncovering)
      {
        FadeSkipNextFadeOut();
        FadeSkipNextFadeIn();
      }
    }
  }
  else
  {
    SetGameStatus(GAME_MODE_PLAYING);

    // when using BD engine, do not cover screen before fading out when starting from main menu
    if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
      game_bd.cover_screen = FALSE;
  }

  // cover BD screen before closing game panel door
  CoverScreen();

  if (checkKeepPlayingSoundsWhenRestarting(restarting))
    FadeLevelMusic();
  else
    FadeLevelSoundsAndMusic();

  if (checkCloseGamePanelDoor(restarting))
    CloseDoor(DOOR_CLOSE_1);

  if (level_editor_test_game)
    FadeSkipNextFadeOut();
  else
    FadeSetEnterScreen();

  if (CheckFadeAll())
    fade_mask = REDRAW_ALL;

  ExpireSoundLoops(TRUE);

  FadeOut(fade_mask);

  if (game.InitColorTemplateImagesNeeded)
    InitColorTemplateImages();

  if (restarting)
  {
    // force restarting global animations displayed during game play
    RestartGlobalAnimsByStatus(GAME_MODE_PSEUDO_RESTARTING);

    // this is required for "transforming" fade modes like cross-fading
    // (else global animations will be stopped, but not restarted here)
    SetAnimStatusBeforeFading(GAME_MODE_PSEUDO_RESTARTING);

    SetGameStatus(GAME_MODE_PLAYING);
  }

  if (level_editor_test_game)
    FadeSkipNextFadeIn();

  // needed if different viewport properties defined for playing
  ChangeViewportPropertiesIfNeeded();

  ClearField();

  DrawCompleteVideoDisplay();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  InitGameEngine();
  InitGameControlValues();

  if (tape.recording)
  {
    // initialize tape actions from game when recording tape
    tape.use_key_actions   = game.use_key_actions;
    tape.use_mouse_actions = game.use_mouse_actions;

    // initialize visible playfield size when recording tape (for team mode)
    tape.scr_fieldx = SCR_FIELDX;
    tape.scr_fieldy = SCR_FIELDY;
  }

  // don't play tapes over network
  network_playing = (network.enabled && !tape.playing);

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    player->index_nr = i;
    player->index_bit = (1 << i);
    player->element_nr = EL_PLAYER_1 + i;

    player->present = FALSE;
    player->active = FALSE;
    player->mapped = FALSE;

    player->killed = FALSE;
    player->reanimated = FALSE;
    player->buried = FALSE;

    player->action = 0;
    player->effective_action = 0;
    player->programmed_action = 0;
    player->snap_action = 0;

    player->mouse_action.lx = 0;
    player->mouse_action.ly = 0;
    player->mouse_action.button = 0;
    player->mouse_action.button_hint = 0;

    player->effective_mouse_action.lx = 0;
    player->effective_mouse_action.ly = 0;
    player->effective_mouse_action.button = 0;
    player->effective_mouse_action.button_hint = 0;

    for (j = 0; j < MAX_NUM_KEYS; j++)
      player->key[j] = FALSE;

    player->num_white_keys = 0;

    player->dynabomb_count = 0;
    player->dynabomb_size = 1;
    player->dynabombs_left = 0;
    player->dynabomb_xl = FALSE;

    player->MovDir = initial_move_dir;
    player->MovPos = 0;
    player->GfxPos = 0;
    player->GfxDir = initial_move_dir;
    player->GfxAction = ACTION_DEFAULT;
    player->Frame = 0;
    player->StepFrame = 0;

    player->initial_element = player->element_nr;
    player->artwork_element =
      (level.use_artwork_element[i] ? level.artwork_element[i] :
       player->element_nr);
    player->use_murphy = FALSE;

    player->block_last_field = FALSE;	// initialized in InitPlayerField()
    player->block_delay_adjustment = 0;	// initialized in InitPlayerField()

    player->gravity = level.initial_player_gravity[i];

    player->can_fall_into_acid = CAN_MOVE_INTO_ACID(player->element_nr);

    player->actual_frame_counter.count = 0;
    player->actual_frame_counter.value = 1;

    player->step_counter = 0;

    player->last_move_dir = initial_move_dir;

    player->is_active = FALSE;

    player->is_waiting = FALSE;
    player->is_moving = FALSE;
    player->is_auto_moving = FALSE;
    player->is_digging = FALSE;
    player->is_snapping = FALSE;
    player->is_collecting = FALSE;
    player->is_pushing = FALSE;
    player->is_switching = FALSE;
    player->is_dropping = FALSE;
    player->is_dropping_pressed = FALSE;

    player->is_bored = FALSE;
    player->is_sleeping = FALSE;

    player->was_waiting = TRUE;
    player->was_moving = FALSE;
    player->was_snapping = FALSE;
    player->was_dropping = FALSE;

    player->force_dropping = FALSE;

    player->frame_counter_bored = -1;
    player->frame_counter_sleeping = -1;

    player->anim_delay_counter = 0;
    player->post_delay_counter = 0;

    player->dir_waiting = initial_move_dir;
    player->action_waiting = ACTION_DEFAULT;
    player->last_action_waiting = ACTION_DEFAULT;
    player->special_action_bored = ACTION_DEFAULT;
    player->special_action_sleeping = ACTION_DEFAULT;

    player->switch_x = -1;
    player->switch_y = -1;

    player->drop_x = -1;
    player->drop_y = -1;

    player->show_envelope = 0;

    SetPlayerMoveSpeed(player, level.initial_player_stepsize[i], TRUE);

    player->push_delay       = -1;	// initialized when pushing starts
    player->push_delay_value = game.initial_push_delay_value;

    player->drop_delay = 0;
    player->drop_pressed_delay = 0;

    player->last_jx = -1;
    player->last_jy = -1;
    player->jx = -1;
    player->jy = -1;

    player->shield_normal_time_left = 0;
    player->shield_deadly_time_left = 0;

    player->last_removed_element = EL_UNDEFINED;

    player->inventory_infinite_element = EL_UNDEFINED;
    player->inventory_size = 0;

    if (level.use_initial_inventory[i])
    {
      for (j = 0; j < level.initial_inventory_size[i]; j++)
      {
	int element = level.initial_inventory_content[i][j];
	int collect_count = element_info[element].collect_count_initial;
	int k;

	if (!IS_CUSTOM_ELEMENT(element))
	  collect_count = 1;

	if (collect_count == 0)
	  player->inventory_infinite_element = element;
	else
	  for (k = 0; k < collect_count; k++)
	    if (player->inventory_size < MAX_INVENTORY_SIZE)
	      player->inventory_element[player->inventory_size++] = element;
      }
    }

    DigField(player, 0, 0, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);

    map_player_action[i] = i;
  }

  network_player_action_received = FALSE;

  // initial null action
  if (network_playing)
    SendToServer_MovePlayer(MV_NONE);

  FrameCounter = 0;
  TimeFrames = 0;
  TimePlayed = 0;
  TimeLeft = level.time;

  TapeTimeFrames = 0;
  TapeTime = 0;

  ScreenMovDir = MV_NONE;
  ScreenMovPos = 0;
  ScreenGfxPos = 0;

  ScrollStepSize = 0;	// will be correctly initialized by ScrollScreen()

  game.robot_wheel_x = -1;
  game.robot_wheel_y = -1;

  game.exit_x = -1;
  game.exit_y = -1;

  game.all_players_gone = FALSE;

  game.LevelSolved = FALSE;
  game.GameOver = FALSE;

  game.GamePlayed = !tape.playing;

  game.LevelSolved_GameWon = FALSE;
  game.LevelSolved_GameEnd = FALSE;
  game.LevelSolved_SaveTape = FALSE;
  game.LevelSolved_SaveScore = FALSE;

  game.LevelSolved_CountingTime = 0;
  game.LevelSolved_CountingScore = 0;
  game.LevelSolved_CountingHealth = 0;

  game.RestartGameRequested = FALSE;

  game.panel.active = TRUE;
  game.panel.show_extra_items = FALSE;

  game.no_level_time_limit = (level.time == 0);
  game.time_limit = (leveldir_current->time_limit && setup.time_limit);

  game.yamyam_content_nr = 0;
  game.robot_wheel_active = FALSE;
  game.magic_wall_active = FALSE;
  game.magic_wall_time_left = 0;
  game.light_time_left = 0;
  game.timegate_time_left = 0;
  game.switchgate_pos = 0;
  game.wind_direction = level.wind_direction_initial;

  game.time_final = 0;
  game.score_time_final = 0;

  game.score = 0;
  game.score_final = 0;

  game.health = MAX_HEALTH;
  game.health_final = MAX_HEALTH;

  game.gems_still_needed = level.gems_needed;
  game.sokoban_fields_still_needed = 0;
  game.sokoban_objects_still_needed = 0;
  game.lights_still_needed = 0;
  game.players_still_needed = 0;
  game.friends_still_needed = 0;

  game.lenses_time_left = 0;
  game.magnify_time_left = 0;

  game.ball_active = level.ball_active_initial;
  game.ball_content_nr = 0;

  game.explosions_delayed = TRUE;

  // special case: set custom artwork setting to initial value
  game.use_masked_elements = game.use_masked_elements_initial;

  // negative number of gems only supported in BD engine
  if (level.game_engine_type != GAME_ENGINE_TYPE_BD)
    if (game.gems_still_needed < 0)
      game.gems_still_needed = 0;

  for (i = 0; i < NUM_BELTS; i++)
  {
    game.belt_dir[i] = MV_NONE;
    game.belt_dir_nr[i] = 3;		// not moving, next moving left
  }

  for (i = 0; i < MAX_NUM_AMOEBA; i++)
    AmoebaCnt[i] = AmoebaCnt2[i] = 0;

#if DEBUG_INIT_PLAYER
  DebugPrintPlayerStatus("Player status at level initialization");
#endif

  SCAN_PLAYFIELD(x, y)
  {
    Tile[x][y] = Last[x][y] = level.field[x][y];
    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    ChangeDelay[x][y] = 0;
    ChangePage[x][y] = -1;
    CustomValue[x][y] = 0;		// initialized in InitField()
    Store[x][y] = Store2[x][y] = StorePlayer[x][y] = Back[x][y] = 0;
    AmoebaNr[x][y] = 0;
    WasJustMoving[x][y] = 0;
    WasJustFalling[x][y] = 0;
    CheckCollision[x][y] = 0;
    CheckImpact[x][y] = 0;
    Stop[x][y] = FALSE;
    Pushed[x][y] = FALSE;

    ChangeCount[x][y] = 0;
    ChangeEvent[x][y] = -1;

    ExplodePhase[x][y] = 0;
    ExplodeDelay[x][y] = 0;
    ExplodeField[x][y] = EX_TYPE_NONE;

    RunnerVisit[x][y] = 0;
    PlayerVisit[x][y] = 0;

    GfxFrame[x][y] = 0;
    GfxRandom[x][y] = INIT_GFX_RANDOM();
    GfxRandomStatic[x][y] = INIT_GFX_RANDOM();
    GfxElement[x][y] = EL_UNDEFINED;
    GfxElementEmpty[x][y] = EL_EMPTY;
    GfxAction[x][y] = ACTION_DEFAULT;
    GfxDir[x][y] = MV_NONE;
    GfxRedraw[x][y] = GFX_REDRAW_NONE;
  }

  SCAN_PLAYFIELD(x, y)
  {
    InitFieldForEngine(x, y);

    if (emulate_bd && !IS_BD_ELEMENT(Tile[x][y]))
      emulate_bd = FALSE;
    if (emulate_sp && !IS_SP_ELEMENT(Tile[x][y]))
      emulate_sp = FALSE;

    InitField(x, y, TRUE);

    ResetGfxAnimation(x, y);
  }

  InitBeltMovement();

  // required if level does not contain any "empty space" element
  if (element_info[EL_EMPTY].use_gfx_element)
    game.use_masked_elements = TRUE;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    // set number of special actions for bored and sleeping animation
    player->num_special_action_bored =
      get_num_special_action(player->artwork_element,
			     ACTION_BORING_1, ACTION_BORING_LAST);
    player->num_special_action_sleeping =
      get_num_special_action(player->artwork_element,
			     ACTION_SLEEPING_1, ACTION_SLEEPING_LAST);
  }

  game.emulation = (emulate_bd ? EMU_BOULDERDASH :
		    emulate_sp ? EMU_SUPAPLEX : EMU_NONE);

  // initialize type of slippery elements
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (!IS_CUSTOM_ELEMENT(i))
    {
      // default: elements slip down either to the left or right randomly
      element_info[i].slippery_type = SLIPPERY_ANY_RANDOM;

      // SP style elements prefer to slip down on the left side
      if (game.engine_version >= VERSION_IDENT(3,1,1,0) && IS_SP_ELEMENT(i))
	element_info[i].slippery_type = SLIPPERY_ANY_LEFT_RIGHT;

      // BD style elements prefer to slip down on the left side
      if (game.emulation == EMU_BOULDERDASH)
	element_info[i].slippery_type = SLIPPERY_ANY_LEFT_RIGHT;
    }
  }

  // initialize explosion and ignition delay
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (!IS_CUSTOM_ELEMENT(i))
    {
      int num_phase = 8;
      int delay = (((IS_SP_ELEMENT(i) && i != EL_EMPTY_SPACE) &&
		    game.engine_version >= VERSION_IDENT(3,1,0,0)) ||
		   game.emulation == EMU_SUPAPLEX ? 3 : 2);
      int last_phase = (num_phase + 1) * delay;
      int half_phase = (num_phase / 2) * delay;

      element_info[i].explosion_delay = last_phase - 1;
      element_info[i].ignition_delay = half_phase;

      if (i == EL_BLACK_ORB)
	element_info[i].ignition_delay = 1;
    }
  }

  // correct non-moving belts to start moving left
  for (i = 0; i < NUM_BELTS; i++)
    if (game.belt_dir[i] == MV_NONE)
      game.belt_dir_nr[i] = 3;		// not moving, next moving left

#if USE_NEW_PLAYER_ASSIGNMENTS
  // use preferred player also in local single-player mode
  if (!network.enabled && !game.team_mode)
  {
    int new_index_nr = setup.network_player_nr;

    if (new_index_nr >= 0 && new_index_nr < MAX_PLAYERS)
    {
      for (i = 0; i < MAX_PLAYERS; i++)
	stored_player[i].connected_locally = FALSE;

      stored_player[new_index_nr].connected_locally = TRUE;
    }
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    stored_player[i].connected = FALSE;

    // in network game mode, the local player might not be the first player
    if (stored_player[i].connected_locally)
      local_player = &stored_player[i];
  }

  if (!network.enabled)
    local_player->connected = TRUE;

  if (tape.playing)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      stored_player[i].connected = tape.player_participates[i];
  }
  else if (network.enabled)
  {
    // add team mode players connected over the network (needed for correct
    // assignment of player figures from level to locally playing players)

    for (i = 0; i < MAX_PLAYERS; i++)
      if (stored_player[i].connected_network)
	stored_player[i].connected = TRUE;
  }
  else if (game.team_mode)
  {
    // try to guess locally connected team mode players (needed for correct
    // assignment of player figures from level to locally playing players)

    for (i = 0; i < MAX_PLAYERS; i++)
      if (setup.input[i].use_joystick ||
	  setup.input[i].key.left != KSYM_UNDEFINED)
	stored_player[i].connected = TRUE;
  }

#if DEBUG_INIT_PLAYER
  DebugPrintPlayerStatus("Player status after level initialization");
#endif

#if DEBUG_INIT_PLAYER
  Debug("game:init:player", "Reassigning players ...");
#endif

  // check if any connected player was not found in playfield
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->connected && !player->present)
    {
      struct PlayerInfo *field_player = NULL;

#if DEBUG_INIT_PLAYER
      Debug("game:init:player",
	    "- looking for field player for player %d ...", i + 1);
#endif

      // assign first free player found that is present in the playfield

      // first try: look for unmapped playfield player that is not connected
      for (j = 0; j < MAX_PLAYERS; j++)
	if (field_player == NULL &&
	    stored_player[j].present &&
	    !stored_player[j].mapped &&
	    !stored_player[j].connected)
	  field_player = &stored_player[j];

      // second try: look for *any* unmapped playfield player
      for (j = 0; j < MAX_PLAYERS; j++)
	if (field_player == NULL &&
	    stored_player[j].present &&
	    !stored_player[j].mapped)
	  field_player = &stored_player[j];

      if (field_player != NULL)
      {
	int jx = field_player->jx, jy = field_player->jy;

#if DEBUG_INIT_PLAYER
	Debug("game:init:player", "- found player %d",
	      field_player->index_nr + 1);
#endif

	player->present = FALSE;
	player->active = FALSE;

	field_player->present = TRUE;
	field_player->active = TRUE;

	/*
	player->initial_element = field_player->initial_element;
	player->artwork_element = field_player->artwork_element;

	player->block_last_field       = field_player->block_last_field;
	player->block_delay_adjustment = field_player->block_delay_adjustment;
	*/

	StorePlayer[jx][jy] = field_player->element_nr;

	field_player->jx = field_player->last_jx = jx;
	field_player->jy = field_player->last_jy = jy;

	if (local_player == player)
	  local_player = field_player;

	map_player_action[field_player->index_nr] = i;

	field_player->mapped = TRUE;

#if DEBUG_INIT_PLAYER
	Debug("game:init:player", "- map_player_action[%d] == %d",
	      field_player->index_nr + 1, i + 1);
#endif
      }
    }

    if (player->connected && player->present)
      player->mapped = TRUE;
  }

#if DEBUG_INIT_PLAYER
  DebugPrintPlayerStatus("Player status after player assignment (first stage)");
#endif

#else

  // check if any connected player was not found in playfield
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->connected && !player->present)
    {
      for (j = 0; j < MAX_PLAYERS; j++)
      {
	struct PlayerInfo *field_player = &stored_player[j];
	int jx = field_player->jx, jy = field_player->jy;

	// assign first free player found that is present in the playfield
	if (field_player->present && !field_player->connected)
	{
	  player->present = TRUE;
	  player->active = TRUE;

	  field_player->present = FALSE;
	  field_player->active = FALSE;

	  player->initial_element = field_player->initial_element;
	  player->artwork_element = field_player->artwork_element;

	  player->block_last_field       = field_player->block_last_field;
	  player->block_delay_adjustment = field_player->block_delay_adjustment;

	  StorePlayer[jx][jy] = player->element_nr;

	  player->jx = player->last_jx = jx;
	  player->jy = player->last_jy = jy;

	  break;
	}
      }
    }
  }
#endif

#if 0
  Debug("game:init:player", "local_player->present == %d",
	local_player->present);
#endif

  // set focus to local player for network games, else to all players
  game.centered_player_nr = (network_playing ? local_player->index_nr : -1);
  game.centered_player_nr_next = game.centered_player_nr;
  game.set_centered_player = FALSE;
  game.set_centered_player_wrap = FALSE;

  if (network_playing && tape.recording)
  {
    // store client dependent player focus when recording network games
    tape.centered_player_nr_next = game.centered_player_nr_next;
    tape.set_centered_player = TRUE;
  }

  if (tape.playing)
  {
    // when playing a tape, eliminate all players who do not participate

#if USE_NEW_PLAYER_ASSIGNMENTS

    if (!game.team_mode)
    {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
	if (stored_player[i].active &&
	    !tape.player_participates[map_player_action[i]])
	{
	  struct PlayerInfo *player = &stored_player[i];
	  int jx = player->jx, jy = player->jy;

#if DEBUG_INIT_PLAYER
	  Debug("game:init:player", "Removing player %d at (%d, %d)",
		i + 1, jx, jy);
#endif

	  player->active = FALSE;
	  StorePlayer[jx][jy] = 0;
	  Tile[jx][jy] = EL_EMPTY;
	}
      }
    }

#else

    for (i = 0; i < MAX_PLAYERS; i++)
    {
      if (stored_player[i].active &&
	  !tape.player_participates[i])
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	player->active = FALSE;
	StorePlayer[jx][jy] = 0;
	Tile[jx][jy] = EL_EMPTY;
      }
    }
#endif
  }
  else if (!network.enabled && !game.team_mode)		// && !tape.playing
  {
    // when in single player mode, eliminate all but the local player

    for (i = 0; i < MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];

      if (player->active && player != local_player)
      {
	int jx = player->jx, jy = player->jy;

	player->active = FALSE;
	player->present = FALSE;

	StorePlayer[jx][jy] = 0;
	Tile[jx][jy] = EL_EMPTY;
      }
    }
  }

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].active)
      game.players_still_needed++;

  if (level.solved_by_one_player)
    game.players_still_needed = 1;

  // when recording the game, store which players take part in the game
  if (tape.recording)
  {
#if USE_NEW_PLAYER_ASSIGNMENTS
    for (i = 0; i < MAX_PLAYERS; i++)
      if (stored_player[i].connected)
	tape.player_participates[i] = TRUE;
#else
    for (i = 0; i < MAX_PLAYERS; i++)
      if (stored_player[i].active)
	tape.player_participates[i] = TRUE;
#endif
  }

#if DEBUG_INIT_PLAYER
  DebugPrintPlayerStatus("Player status after player assignment (final stage)");
#endif

  if (BorderElement == EL_EMPTY)
  {
    SBX_Left = 0;
    SBX_Right = lev_fieldx - SCR_FIELDX;
    SBY_Upper = 0;
    SBY_Lower = lev_fieldy - SCR_FIELDY;
  }
  else
  {
    SBX_Left = -1;
    SBX_Right = lev_fieldx - SCR_FIELDX + 1;
    SBY_Upper = -1;
    SBY_Lower = lev_fieldy - SCR_FIELDY + 1;
  }

  if (full_lev_fieldx <= SCR_FIELDX)
    SBX_Left = SBX_Right = -1 * (SCR_FIELDX - lev_fieldx) / 2;
  if (full_lev_fieldy <= SCR_FIELDY)
    SBY_Upper = SBY_Lower = -1 * (SCR_FIELDY - lev_fieldy) / 2;

  if (EVEN(SCR_FIELDX) && full_lev_fieldx > SCR_FIELDX)
    SBX_Left--;
  if (EVEN(SCR_FIELDY) && full_lev_fieldy > SCR_FIELDY)
    SBY_Upper--;

  // if local player not found, look for custom element that might create
  // the player (make some assumptions about the right custom element)
  if (!local_player->present)
  {
    int start_x = 0, start_y = 0;
    int found_rating = 0;
    int found_element = EL_UNDEFINED;
    int player_nr = local_player->index_nr;

    SCAN_PLAYFIELD(x, y)
    {
      int element = Tile[x][y];
      int content;
      int xx, yy;
      boolean is_player;

      if (level.use_start_element[player_nr] &&
	  level.start_element[player_nr] == element &&
	  found_rating < 4)
      {
	start_x = x;
	start_y = y;

	found_rating = 4;
	found_element = element;
      }

      if (!IS_CUSTOM_ELEMENT(element))
	continue;

      if (CAN_CHANGE(element))
      {
	for (i = 0; i < element_info[element].num_change_pages; i++)
	{
	  // check for player created from custom element as single target
	  content = element_info[element].change_page[i].target_element;
	  is_player = IS_PLAYER_ELEMENT(content);

	  if (is_player && (found_rating < 3 ||
			    (found_rating == 3 && element < found_element)))
	  {
	    start_x = x;
	    start_y = y;

	    found_rating = 3;
	    found_element = element;
	  }
	}
      }

      for (yy = 0; yy < 3; yy++) for (xx = 0; xx < 3; xx++)
      {
	// check for player created from custom element as explosion content
	content = element_info[element].content.e[xx][yy];
	is_player = IS_PLAYER_ELEMENT(content);

	if (is_player && (found_rating < 2 ||
			  (found_rating == 2 && element < found_element)))
	{
	  start_x = x + xx - 1;
	  start_y = y + yy - 1;

	  found_rating = 2;
	  found_element = element;
	}

	if (!CAN_CHANGE(element))
	  continue;

	for (i = 0; i < element_info[element].num_change_pages; i++)
	{
	  // check for player created from custom element as extended target
	  content =
	    element_info[element].change_page[i].target_content.e[xx][yy];

	  is_player = IS_PLAYER_ELEMENT(content);

	  if (is_player && (found_rating < 1 ||
			    (found_rating == 1 && element < found_element)))
	  {
	    start_x = x + xx - 1;
	    start_y = y + yy - 1;

	    found_rating = 1;
	    found_element = element;
	  }
	}
      }
    }

    scroll_x = SCROLL_POSITION_X(start_x);
    scroll_y = SCROLL_POSITION_Y(start_y);
  }
  else
  {
    scroll_x = SCROLL_POSITION_X(local_player->jx);
    scroll_y = SCROLL_POSITION_Y(local_player->jy);
  }

  if (game.forced_scroll_x != ARG_UNDEFINED_VALUE)
    scroll_x = game.forced_scroll_x;
  if (game.forced_scroll_y != ARG_UNDEFINED_VALUE)
    scroll_y = game.forced_scroll_y;

  // !!! FIX THIS (START) !!!
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    InitGameEngine_BD();
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    InitGameEngine_EM();
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    InitGameEngine_SP();
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    InitGameEngine_MM();
  }
  else
  {
    DrawLevel(REDRAW_FIELD);
    DrawAllPlayers();

    // after drawing the level, correct some elements
    if (game.timegate_time_left == 0)
      CloseAllOpenTimegates();
  }

  // blit playfield from scroll buffer to normal back buffer for fading in
  BlitScreenToBitmap(backbuffer);
  // !!! FIX THIS (END) !!!

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);

#if 1
  // full screen redraw is required at this point in the following cases:
  // - special editor door undrawn when game was started from level editor
  // - drawing area (playfield) was changed and has to be removed completely
  redraw_mask = REDRAW_ALL;
  BackToFront();
#endif

  if (!game.restart_level)
  {
    // copy default game door content to main double buffer

    // !!! CHECK AGAIN !!!
    SetPanelBackground();
    // SetDoorBackgroundImage(IMG_BACKGROUND_PANEL);
    DrawBackground(DX, DY, DXSIZE, DYSIZE);
  }

  SetPanelBackground();
  SetDrawBackgroundMask(REDRAW_DOOR_1);

  UpdateAndDisplayGameControlValues();

  if (!game.restart_level)
  {
    UnmapGameButtons();
    UnmapTapeButtons();

    FreeGameButtons();
    CreateGameButtons();

    MapGameButtons();
    MapTapeButtons();

    // copy actual game door content to door double buffer for OpenDoor()
    BlitBitmap(drawto, bitmap_db_door_1, DX, DY, DXSIZE, DYSIZE, 0, 0);

    OpenDoor(DOOR_OPEN_ALL);

    KeyboardAutoRepeatOffUnlessAutoplay();

#if DEBUG_INIT_PLAYER
    DebugPrintPlayerStatus("Player status (final)");
#endif
  }

  UnmapAllGadgets();

  MapGameButtons();
  MapTapeButtons();

  if (!game.restart_level && !tape.playing)
  {
    LevelStats_incPlayed(level_nr);

    SaveLevelSetup_SeriesInfo();
  }

  game.restart_level = FALSE;
  game.request_open = FALSE;
  game.request_active = FALSE;
  game.envelope_active = FALSE;
  game.any_door_active = FALSE;

  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    InitGameActions_MM();

  SaveEngineSnapshotToListInitial();

  if (!game.restart_level)
  {
    PlaySound(SND_GAME_STARTING);

    if (setup.sound_music)
      PlayLevelMusic();
  }

  SetPlayfieldMouseCursorEnabled(!game.use_mouse_actions);

  game.InitGameRequested = FALSE;
}

void UpdateEngineValues(int actual_scroll_x, int actual_scroll_y,
			int actual_player_x, int actual_player_y)
{
  // this is used for non-R'n'D game engines to update certain engine values

  // needed to determine if sounds are played within the visible screen area
  scroll_x = actual_scroll_x;
  scroll_y = actual_scroll_y;

  // needed to get player position for "follow finger" playing input method
  local_player->jx = actual_player_x;
  local_player->jy = actual_player_y;
}

void InitMovDir(int x, int y)
{
  int i, element = Tile[x][y];
  static int xy[4][2] =
  {
    {  0, +1 },
    { +1,  0 },
    {  0, -1 },
    { -1,  0 }
  };
  static int direction[3][4] =
  {
    { MV_RIGHT, MV_UP,   MV_LEFT,  MV_DOWN },
    { MV_LEFT,  MV_DOWN, MV_RIGHT, MV_UP },
    { MV_LEFT,  MV_RIGHT, MV_UP, MV_DOWN }
  };

  switch (element)
  {
    case EL_BUG_RIGHT:
    case EL_BUG_UP:
    case EL_BUG_LEFT:
    case EL_BUG_DOWN:
      Tile[x][y] = EL_BUG;
      MovDir[x][y] = direction[0][element - EL_BUG_RIGHT];
      break;

    case EL_SPACESHIP_RIGHT:
    case EL_SPACESHIP_UP:
    case EL_SPACESHIP_LEFT:
    case EL_SPACESHIP_DOWN:
      Tile[x][y] = EL_SPACESHIP;
      MovDir[x][y] = direction[0][element - EL_SPACESHIP_RIGHT];
      break;

    case EL_BD_BUTTERFLY_RIGHT:
    case EL_BD_BUTTERFLY_UP:
    case EL_BD_BUTTERFLY_LEFT:
    case EL_BD_BUTTERFLY_DOWN:
      Tile[x][y] = EL_BD_BUTTERFLY;
      MovDir[x][y] = direction[0][element - EL_BD_BUTTERFLY_RIGHT];
      break;

    case EL_BD_FIREFLY_RIGHT:
    case EL_BD_FIREFLY_UP:
    case EL_BD_FIREFLY_LEFT:
    case EL_BD_FIREFLY_DOWN:
      Tile[x][y] = EL_BD_FIREFLY;
      MovDir[x][y] = direction[0][element - EL_BD_FIREFLY_RIGHT];
      break;

    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      Tile[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element - EL_PACMAN_RIGHT];
      break;

    case EL_YAMYAM_LEFT:
    case EL_YAMYAM_RIGHT:
    case EL_YAMYAM_UP:
    case EL_YAMYAM_DOWN:
      Tile[x][y] = EL_YAMYAM;
      MovDir[x][y] = direction[2][element - EL_YAMYAM_LEFT];
      break;

    case EL_SP_SNIKSNAK:
      MovDir[x][y] = MV_UP;
      break;

    case EL_SP_ELECTRON:
      MovDir[x][y] = MV_LEFT;
      break;

    case EL_MOLE_LEFT:
    case EL_MOLE_RIGHT:
    case EL_MOLE_UP:
    case EL_MOLE_DOWN:
      Tile[x][y] = EL_MOLE;
      MovDir[x][y] = direction[2][element - EL_MOLE_LEFT];
      break;

    case EL_SPRING_LEFT:
    case EL_SPRING_RIGHT:
      Tile[x][y] = EL_SPRING;
      MovDir[x][y] = direction[2][element - EL_SPRING_LEFT];
      break;

    default:
      if (IS_CUSTOM_ELEMENT(element))
      {
	struct ElementInfo *ei = &element_info[element];
	int move_direction_initial = ei->move_direction_initial;
	int move_pattern = ei->move_pattern;

	if (move_direction_initial == MV_START_PREVIOUS)
	{
	  if (MovDir[x][y] != MV_NONE)
	    return;

	  move_direction_initial = MV_START_AUTOMATIC;
	}

	if (move_direction_initial == MV_START_RANDOM)
	  MovDir[x][y] = 1 << RND(4);
	else if (move_direction_initial & MV_ANY_DIRECTION)
	  MovDir[x][y] = move_direction_initial;
	else if (move_pattern == MV_ALL_DIRECTIONS ||
		 move_pattern == MV_TURNING_LEFT ||
		 move_pattern == MV_TURNING_RIGHT ||
		 move_pattern == MV_TURNING_LEFT_RIGHT ||
		 move_pattern == MV_TURNING_RIGHT_LEFT ||
		 move_pattern == MV_TURNING_RANDOM)
	  MovDir[x][y] = 1 << RND(4);
	else if (move_pattern == MV_HORIZONTAL)
	  MovDir[x][y] = (RND(2) ? MV_LEFT : MV_RIGHT);
	else if (move_pattern == MV_VERTICAL)
	  MovDir[x][y] = (RND(2) ? MV_UP : MV_DOWN);
	else if (move_pattern & MV_ANY_DIRECTION)
	  MovDir[x][y] = element_info[element].move_pattern;
	else if (move_pattern == MV_ALONG_LEFT_SIDE ||
		 move_pattern == MV_ALONG_RIGHT_SIDE)
	{
	  // use random direction as default start direction
	  if (game.engine_version >= VERSION_IDENT(3,1,0,0))
	    MovDir[x][y] = 1 << RND(4);

	  for (i = 0; i < NUM_DIRECTIONS; i++)
	  {
	    int x1 = x + xy[i][0];
	    int y1 = y + xy[i][1];

	    if (!IN_LEV_FIELD(x1, y1) || !IS_FREE(x1, y1))
	    {
	      if (move_pattern == MV_ALONG_RIGHT_SIDE)
		MovDir[x][y] = direction[0][i];
	      else
		MovDir[x][y] = direction[1][i];

	      break;
	    }
	  }
	}		 
      }
      else
      {
	MovDir[x][y] = 1 << RND(4);

	if (element != EL_BUG &&
	    element != EL_SPACESHIP &&
	    element != EL_BD_BUTTERFLY &&
	    element != EL_BD_FIREFLY)
	  break;

	for (i = 0; i < NUM_DIRECTIONS; i++)
	{
	  int x1 = x + xy[i][0];
	  int y1 = y + xy[i][1];

	  if (!IN_LEV_FIELD(x1, y1) || !IS_FREE(x1, y1))
	  {
	    if (element == EL_BUG || element == EL_BD_BUTTERFLY)
	    {
	      MovDir[x][y] = direction[0][i];
	      break;
	    }
	    else if (element == EL_SPACESHIP || element == EL_BD_FIREFLY ||
		     element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
	    {
	      MovDir[x][y] = direction[1][i];
	      break;
	    }
	  }
	}
      }
      break;
  }

  GfxDir[x][y] = MovDir[x][y];
}

void InitAmoebaNr(int x, int y)
{
  int i;
  int group_nr = AmoebaNeighbourNr(x, y);

  if (group_nr == 0)
  {
    for (i = 1; i < MAX_NUM_AMOEBA; i++)
    {
      if (AmoebaCnt[i] == 0)
      {
	group_nr = i;
	break;
      }
    }
  }

  AmoebaNr[x][y] = group_nr;
  AmoebaCnt[group_nr]++;
  AmoebaCnt2[group_nr]++;
}

static void LevelSolved_UpdateFinalGameValues(int time, int score, int health)
{
  // correct game panel score value when playing BD games with multiple lives
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD && setup.bd_multiple_lives)
    score = game_bd.global_score;

  game.LevelSolved_CountingTime = time;
  game.LevelSolved_CountingScore = score;
  game.LevelSolved_CountingHealth = health;

  game_panel_controls[GAME_PANEL_TIME].value = time;
  game_panel_controls[GAME_PANEL_SCORE].value = score;
  game_panel_controls[GAME_PANEL_HEALTH].value = health;
}

static void LevelSolved_SetFinalGameValues(void)
{
  game.time_final = (level.game_engine_type == GAME_ENGINE_TYPE_BD ? game_bd.time_left :
		     game.no_level_time_limit ? TimePlayed : TimeLeft);
  game.score_time_final = (level.game_engine_type == GAME_ENGINE_TYPE_BD ? game_bd.frames_played :
                           level.use_step_counter ? TimePlayed :
			   TimePlayed * FRAMES_PER_SECOND + TimeFrames);

  game.score_final = (level.game_engine_type == GAME_ENGINE_TYPE_BD ? game_bd.score :
		      level.game_engine_type == GAME_ENGINE_TYPE_EM ? game_em.lev->score :
		      level.game_engine_type == GAME_ENGINE_TYPE_MM ? game_mm.score :
		      game.score);

  game.health_final = (level.game_engine_type == GAME_ENGINE_TYPE_MM ?
		       MM_HEALTH(game_mm.laser_overload_value) :
		       game.health);

  LevelSolved_UpdateFinalGameValues(game.time_final, game.score_final, game.health_final);
}

static void LevelSolved_DisplayFinalGameValues(int time, int score, int health)
{
  LevelSolved_UpdateFinalGameValues(time, score, health);

  DisplayGameControlValues();
}

static void LevelSolved(void)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_RND &&
      game.players_still_needed > 0)
    return;

  game.LevelSolved = TRUE;
  game.GameOver = TRUE;

  tape.solved = TRUE;

  // needed here to display correct panel values while player walks into exit
  LevelSolved_SetFinalGameValues();
}

static boolean AdvanceToNextLevel(void)
{
  if (setup.increment_levels &&
      level_nr < leveldir_current->last_level &&
      !network_playing)
  {
    level_nr++;		// advance to next level
    TapeErase();	// start with empty tape

    if (setup.auto_play_next_level)
    {
      scores.continue_playing = TRUE;
      scores.next_level_nr = level_nr;

      // cover screen before loading next level (which may have different size and color)
      CoverScreen();

      LoadLevel(level_nr);

      SaveLevelSetup_SeriesInfo();
    }

    return TRUE;
  }

  return FALSE;
}

static boolean GameWon(void)
{
  static int time_count_steps;
  static int time, time_final;
  static float score, score_final; // needed for time score < 10 for 10 seconds
  static int health, health_final;
  static int game_over_delay_1 = 0;
  static int game_over_delay_2 = 0;
  static int game_over_delay_3 = 0;
  int time_score_base = MIN(MAX(1, level.time_score_base), 10);
  float time_score = (float)level.score[SC_TIME_BONUS] / time_score_base;

  if (!game.LevelSolved_GameWon)
  {
    int i;

    // do not start end game actions before the player stops moving (to exit)
    if (local_player->active && local_player->MovPos)
      return FALSE;

    // calculate final game values after player finished walking into exit
    LevelSolved_SetFinalGameValues();

    game.LevelSolved_GameWon = TRUE;
    game.LevelSolved_SaveTape = tape.recording;
    game.LevelSolved_SaveScore = !tape.playing;

    if (!tape.playing)
    {
      LevelStats_incSolved(level_nr);

      SaveLevelSetup_SeriesInfo();
    }

    if (tape.auto_play)		// tape might already be stopped here
      tape.auto_play_level_solved = TRUE;

    TapeStop();

    game_over_delay_1 = FRAMES_PER_SECOND;	// delay before counting time
    game_over_delay_2 = FRAMES_PER_SECOND / 2;	// delay before counting health
    game_over_delay_3 = FRAMES_PER_SECOND;	// delay before ending the game

    time = time_final = game.time_final;
    score = score_final = game.score_final;
    health = health_final = game.health_final;

    // update game panel values before (delayed) counting of score (if any)
    LevelSolved_DisplayFinalGameValues(time, score, health);

    // if level has time score defined, calculate new final game values
    if (time_score > 0)
    {
      int time_final_max = 999;
      int time_frames_final_max = time_final_max * FRAMES_PER_SECOND;
      int time_frames = 0;
      int time_frames_left = TimeLeft * FRAMES_PER_SECOND - TimeFrames;
      int time_frames_played = TimePlayed * FRAMES_PER_SECOND + TimeFrames;

      if (TimeLeft > 0)
      {
	time_final = 0;
	time_frames = time_frames_left;
      }
      else if (game.no_level_time_limit && TimePlayed < time_final_max)
      {
	time_final = time_final_max;
	time_frames = time_frames_final_max - time_frames_played;
      }

      score_final += time_score * time_frames / FRAMES_PER_SECOND + 0.5;

      time_count_steps = MAX(1, ABS(time_final - time) / 100);

      if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
      {
	// keep previous values (final values already processed here)
	time_final = time;
	score_final = score;
      }
      else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
      {
	health_final = 0;
	score_final += health * time_score;
      }

      game.score_final = score_final;
      game.health_final = health_final;
    }

    // if not counting score after game, immediately update game panel values
    if (level_editor_test_game || !setup.count_score_after_game ||
	level.game_engine_type == GAME_ENGINE_TYPE_BD)
    {
      time = time_final;
      score = score_final;

      LevelSolved_DisplayFinalGameValues(time, score, health);
    }

    if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    {
      // check if last player has left the level
      if (game.exit_x >= 0 &&
	  game.exit_y >= 0)
      {
	int x = game.exit_x;
	int y = game.exit_y;
	int element = Tile[x][y];

	// close exit door after last player
	if ((game.all_players_gone &&
	     (element == EL_EXIT_OPEN ||
	      element == EL_SP_EXIT_OPEN ||
	      element == EL_STEEL_EXIT_OPEN)) ||
	    element == EL_EM_EXIT_OPEN ||
	    element == EL_EM_STEEL_EXIT_OPEN)
	{

	  Tile[x][y] =
	    (element == EL_EXIT_OPEN		? EL_EXIT_CLOSING :
	     element == EL_EM_EXIT_OPEN		? EL_EM_EXIT_CLOSING :
	     element == EL_SP_EXIT_OPEN		? EL_SP_EXIT_CLOSING:
	     element == EL_STEEL_EXIT_OPEN	? EL_STEEL_EXIT_CLOSING:
	     EL_EM_STEEL_EXIT_CLOSING);

	  PlayLevelSoundElementAction(x, y, element, ACTION_CLOSING);
	}

	// player disappears
	DrawLevelField(x, y);
      }

      for (i = 0; i < MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];

	if (player->present)
	{
	  RemovePlayer(player);

	  // player disappears
	  DrawLevelField(player->jx, player->jy);
	}
      }
    }

    PlaySound(SND_GAME_WINNING);
  }

  if (setup.count_score_after_game)
  {
    if (time != time_final)
    {
      if (game_over_delay_1 > 0)
      {
	game_over_delay_1--;

	return FALSE;
      }

      int time_to_go = ABS(time_final - time);
      int time_count_dir = (time < time_final ? +1 : -1);

      if (time_to_go < time_count_steps)
	time_count_steps = 1;

      time  += time_count_steps * time_count_dir;
      score += time_count_steps * time_score;

      // set final score to correct rounding differences after counting score
      if (time == time_final)
	score = score_final;

      LevelSolved_DisplayFinalGameValues(time, score, health);

      if (time == time_final)
	StopSound(SND_GAME_LEVELTIME_BONUS);
      else if (setup.sound_loops)
	PlaySoundLoop(SND_GAME_LEVELTIME_BONUS);
      else
	PlaySound(SND_GAME_LEVELTIME_BONUS);

      return FALSE;
    }

    if (health != health_final)
    {
      if (game_over_delay_2 > 0)
      {
	game_over_delay_2--;

	return FALSE;
      }

      int health_count_dir = (health < health_final ? +1 : -1);

      health += health_count_dir;
      score  += time_score;

      LevelSolved_DisplayFinalGameValues(time, score, health);

      if (health == health_final)
	StopSound(SND_GAME_LEVELTIME_BONUS);
      else if (setup.sound_loops)
	PlaySoundLoop(SND_GAME_LEVELTIME_BONUS);
      else
	PlaySound(SND_GAME_LEVELTIME_BONUS);

      return FALSE;
    }
  }

  game.panel.active = FALSE;

  if (game_over_delay_3 > 0)
  {
    game_over_delay_3--;

    return FALSE;
  }

  GameEnd();

  return TRUE;
}

void GameEnd(void)
{
  // used instead of "level_nr" (needed for network games)
  int last_level_nr = levelset.level_nr;
  boolean tape_saved = FALSE;
  boolean game_over = checkGameFailed();

  // Important note: This function is not only called after "GameWon()", but also after
  // "game over" (if automatically asking for restarting the game is disabled in setup)

  if (game.GamePlayed && game_over)
  {
    boolean restart_game = FALSE;

    if (level.game_engine_type == GAME_ENGINE_TYPE_BD && setup.bd_multiple_lives &&
        game_bd.global_lives > 0)
    {
      // do not restart intermission after game over (but continue with next level)
      if (!level.bd_intermission)
      {
        // do not handle game end if game over and playing with remaining multiple lives
        if (game_bd.global_lives > 1)
          restart_game = TRUE;
      }
    }
    else if (setup.ask_on_game_over)
    {
      // do not handle game end if game over and automatically asking for game restart
      // (this is a special case: player pressed "return" key or fire button shortly before
      // automatically asking to restart the game, so skip asking and restart right away)

      restart_game = TRUE;
    }

    if (restart_game)
    {
      StartGameActions(network.enabled, setup.autorecord, level.random_seed);

      return;
    }
  }

  // do not handle game end if request dialog is already active
  if (checkRequestActive())
    return;

  if (game.LevelSolved)
    game.LevelSolved_GameEnd = TRUE;

  if (game.LevelSolved_SaveTape && !score_info_tape_play)
  {
    // make sure that request dialog to save tape does not open door again
    if (!global.use_envelope_request)
      CloseDoor(DOOR_CLOSE_1);

    // ask to save tape
    tape_saved = SaveTapeChecked_LevelSolved(tape.level_nr);

    // set unique basename for score tape (also saved in high score table)
    strcpy(tape.score_tape_basename, getScoreTapeBasename(setup.player_name));
  }

  if (level_editor_test_game || score_info_tape_play)
  {
    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();

    return;
  }

  if (!game.GamePlayed || (!game.LevelSolved_SaveScore && !level.bd_intermission))
  {
    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();

    return;
  }

  if (level_nr == leveldir_current->handicap_level)
  {
    leveldir_current->handicap_level++;

    SaveLevelSetup_SeriesInfo();
  }

  // save score and score tape before potentially erasing tape below
  if (game.LevelSolved_SaveScore)
    NewHighScore(last_level_nr, tape_saved);

  // increment and load next level (if possible and not configured otherwise)
  AdvanceToNextLevel();

  if (game.LevelSolved_SaveScore && scores.last_added >= 0 && setup.show_scores_after_game)
  {
    SetGameStatus(GAME_MODE_SCORES);

    CloseDoor(DOOR_CLOSE_ALL);

    DrawHallOfFame(last_level_nr);
  }
  else if (scores.continue_playing)
  {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
  }
  else
  {
    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();
  }
}

static int addScoreEntry(struct ScoreInfo *list, struct ScoreEntry *new_entry,
			 boolean one_score_entry_per_name)
{
  int i;

  if (strEqual(new_entry->name, EMPTY_PLAYER_NAME))
    return -1;

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    struct ScoreEntry *entry = &list->entry[i];
    boolean score_is_better = (new_entry->score >  entry->score);
    boolean score_is_equal  = (new_entry->score == entry->score);
    boolean time_is_better  = (new_entry->time  <  entry->time);
    boolean time_is_equal   = (new_entry->time  == entry->time);
    boolean better_by_score = (score_is_better ||
			       (score_is_equal && time_is_better));
    boolean better_by_time  = (time_is_better ||
			       (time_is_equal && score_is_better));
    boolean is_better = (level.rate_time_over_score ? better_by_time :
			 better_by_score);
    boolean entry_is_empty = (entry->score == 0 &&
			      entry->time == 0);

    // prevent adding server score entries if also existing in local score file
    // (special case: historic score entries have an empty tape basename entry)
    if (strEqual(new_entry->tape_basename, entry->tape_basename) &&
	!strEqual(new_entry->tape_basename, UNDEFINED_FILENAME))
    {
      // add fields from server score entry not stored in local score entry
      // (currently, this means setting platform, version and country fields;
      // in rare cases, this may also correct an invalid score value, as
      // historic scores might have been truncated to 16-bit values locally)
      *entry = *new_entry;

      return -1;
    }

    if (is_better || entry_is_empty)
    {
      // player has made it to the hall of fame

      if (i < MAX_SCORE_ENTRIES - 1)
      {
	int m = MAX_SCORE_ENTRIES - 1;
	int l;

	if (one_score_entry_per_name)
	{
	  for (l = i; l < MAX_SCORE_ENTRIES; l++)
	    if (strEqual(list->entry[l].name, new_entry->name))
	      m = l;

	  if (m == i)	// player's new highscore overwrites his old one
	    goto put_into_list;
	}

	for (l = m; l > i; l--)
	  list->entry[l] = list->entry[l - 1];
      }

      put_into_list:

      *entry = *new_entry;

      return i;
    }
    else if (one_score_entry_per_name &&
	     strEqual(entry->name, new_entry->name))
    {
      // player already in high score list with better score or time

      return -1;
    }
  }

  // special case: new score is beyond the last high score list position
  return MAX_SCORE_ENTRIES;
}

void NewHighScore(int level_nr, boolean tape_saved)
{
  struct ScoreEntry new_entry = {{ 0 }}; // (prevent warning from GCC bug 53119)
  boolean one_per_name = FALSE;

  strncpy(new_entry.tape_basename, tape.score_tape_basename, MAX_FILENAME_LEN);
  strncpy(new_entry.name, setup.player_name, MAX_PLAYER_NAME_LEN);

  new_entry.score = game.score_final;
  new_entry.time = game.score_time_final;

  LoadScore(level_nr);

  scores.last_added = addScoreEntry(&scores, &new_entry, one_per_name);

  if (scores.last_added >= MAX_SCORE_ENTRIES)
  {
    scores.last_added = MAX_SCORE_ENTRIES - 1;
    scores.force_last_added = TRUE;

    scores.entry[scores.last_added] = new_entry;

    // store last added local score entry (before merging server scores)
    scores.last_added_local = scores.last_added;

    return;
  }

  if (scores.last_added < 0)
    return;

  SaveScore(level_nr);

  // store last added local score entry (before merging server scores)
  scores.last_added_local = scores.last_added;

  if (!game.LevelSolved_SaveTape)
    return;

  SaveScoreTape(level_nr);

  if (setup.ask_for_using_api_server)
  {
    setup.use_api_server =
      Request("Upload your score and tape to the high score server?", REQ_ASK);

    if (!setup.use_api_server)
      Request("Not using high score server! Use setup menu to enable again!",
	      REQ_CONFIRM);

    runtime.use_api_server = setup.use_api_server;

    // after asking for using API server once, do not ask again
    setup.ask_for_using_api_server = FALSE;

    SaveSetup_ServerSetup();
  }

  SaveServerScore(level_nr, tape_saved);
}

void MergeServerScore(void)
{
  struct ScoreEntry last_added_entry;
  boolean one_per_name = FALSE;
  int i;

  if (scores.last_added >= 0)
    last_added_entry = scores.entry[scores.last_added];

  for (i = 0; i < server_scores.num_entries; i++)
  {
    int pos = addScoreEntry(&scores, &server_scores.entry[i], one_per_name);

    if (pos >= 0 && pos <= scores.last_added)
      scores.last_added++;
  }

  if (scores.last_added >= MAX_SCORE_ENTRIES)
  {
    scores.last_added = MAX_SCORE_ENTRIES - 1;
    scores.force_last_added = TRUE;

    scores.entry[scores.last_added] = last_added_entry;
  }
}

static int getElementMoveStepsizeExt(int x, int y, int direction)
{
  int element = Tile[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int horiz_move = (dx != 0);
  int sign = (horiz_move ? dx : dy);
  int step = sign * element_info[element].move_stepsize;

  // special values for move stepsize for spring and things on conveyor belt
  if (horiz_move)
  {
    if (CAN_FALL(element) &&
	y < lev_fieldy - 1 && IS_BELT_ACTIVE(Tile[x][y + 1]))
      step = sign * MOVE_STEPSIZE_NORMAL / 2;
    else if (element == EL_SPRING)
      step = sign * MOVE_STEPSIZE_NORMAL * 2;
  }

  return step;
}

static int getElementMoveStepsize(int x, int y)
{
  return getElementMoveStepsizeExt(x, y, MovDir[x][y]);
}

void InitPlayerGfxAnimation(struct PlayerInfo *player, int action, int dir)
{
  if (player->GfxAction != action || player->GfxDir != dir)
  {
    player->GfxAction = action;
    player->GfxDir = dir;
    player->Frame = 0;
    player->StepFrame = 0;
  }
}

static void ResetGfxFrame(int x, int y)
{
  // profiling showed that "autotest" spends 10~20% of its time in this function
  if (DrawingDeactivatedField())
    return;

  int element = Tile[x][y];
  int graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  if (graphic_info[graphic].anim_global_sync)
    GfxFrame[x][y] = FrameCounter;
  else if (graphic_info[graphic].anim_global_anim_sync)
    GfxFrame[x][y] = getGlobalAnimSyncFrame();
  else if (ANIM_MODE(graphic) == ANIM_CE_VALUE)
    GfxFrame[x][y] = CustomValue[x][y];
  else if (ANIM_MODE(graphic) == ANIM_CE_SCORE)
    GfxFrame[x][y] = element_info[element].collect_score;
  else if (ANIM_MODE(graphic) == ANIM_CE_DELAY)
    GfxFrame[x][y] = ChangeDelay[x][y];
}

static void ResetGfxAnimation(int x, int y)
{
  GfxAction[x][y] = ACTION_DEFAULT;
  GfxDir[x][y] = MovDir[x][y];
  GfxFrame[x][y] = 0;

  ResetGfxFrame(x, y);
}

static void ResetRandomAnimationValue(int x, int y)
{
  GfxRandom[x][y] = INIT_GFX_RANDOM();
}

static void InitMovingField(int x, int y, int direction)
{
  int element = Tile[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int newx = x + dx;
  int newy = y + dy;
  boolean is_moving_before, is_moving_after;

  // check if element was/is moving or being moved before/after mode change
  is_moving_before = (WasJustMoving[x][y] != 0);
  is_moving_after  = (getElementMoveStepsizeExt(x, y, direction)    != 0);

  // reset animation only for moving elements which change direction of moving
  // or which just started or stopped moving
  // (else CEs with property "can move" / "not moving" are reset each frame)
  if (is_moving_before != is_moving_after ||
      direction != MovDir[x][y])
    ResetGfxAnimation(x, y);

  MovDir[x][y] = direction;
  GfxDir[x][y] = direction;

  GfxAction[x][y] = (!is_moving_after ? ACTION_WAITING :
		     direction == MV_DOWN && CAN_FALL(element) ?
		     ACTION_FALLING : ACTION_MOVING);

  // this is needed for CEs with property "can move" / "not moving"

  if (is_moving_after)
  {
    if (Tile[newx][newy] == EL_EMPTY)
      Tile[newx][newy] = EL_BLOCKED;

    MovDir[newx][newy] = MovDir[x][y];

    CustomValue[newx][newy] = CustomValue[x][y];

    GfxFrame[newx][newy] = GfxFrame[x][y];
    GfxRandom[newx][newy] = GfxRandom[x][y];
    GfxAction[newx][newy] = GfxAction[x][y];
    GfxDir[newx][newy] = GfxDir[x][y];
  }
}

void Moving2Blocked(int x, int y, int *goes_to_x, int *goes_to_y)
{
  int direction = MovDir[x][y];
  int newx = x + ((direction & MV_LEFT) ? -1 : (direction & MV_RIGHT) ? +1 : 0);
  int newy = y + ((direction & MV_UP)   ? -1 : (direction & MV_DOWN)  ? +1 : 0);

  *goes_to_x = newx;
  *goes_to_y = newy;
}

void Blocked2Moving(int x, int y, int *comes_from_x, int *comes_from_y)
{
  int direction = MovDir[x][y];
  int oldx = x + ((direction & MV_LEFT) ? +1 : (direction & MV_RIGHT) ? -1 : 0);
  int oldy = y + ((direction & MV_UP)   ? +1 : (direction & MV_DOWN)  ? -1 : 0);

  *comes_from_x = oldx;
  *comes_from_y = oldy;
}

static int MovingOrBlocked2Element(int x, int y)
{
  int element = Tile[x][y];

  if (element == EL_BLOCKED)
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);

    return Tile[oldx][oldy];
  }

  return element;
}

static int MovingOrBlocked2ElementIfNotLeaving(int x, int y)
{
  // like MovingOrBlocked2Element(), but if element is moving
  // and (x, y) is the field the moving element is just leaving,
  // return EL_BLOCKED instead of the element value
  int element = Tile[x][y];

  if (IS_MOVING(x, y))
  {
    if (element == EL_BLOCKED)
    {
      int oldx, oldy;

      Blocked2Moving(x, y, &oldx, &oldy);
      return Tile[oldx][oldy];
    }
    else
      return EL_BLOCKED;
  }
  else
    return element;
}

static void RemoveField(int x, int y)
{
  Tile[x][y] = EL_EMPTY;

  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;

  CustomValue[x][y] = 0;

  AmoebaNr[x][y] = 0;
  ChangeDelay[x][y] = 0;
  ChangePage[x][y] = -1;
  Pushed[x][y] = FALSE;

  GfxElement[x][y] = EL_UNDEFINED;
  GfxAction[x][y] = ACTION_DEFAULT;
  GfxDir[x][y] = MV_NONE;
}

static void RemoveMovingField(int x, int y)
{
  int oldx = x, oldy = y, newx = x, newy = y;
  int element = Tile[x][y];
  int next_element = EL_UNDEFINED;

  if (element != EL_BLOCKED && !IS_MOVING(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    Moving2Blocked(x, y, &newx, &newy);

    if (Tile[newx][newy] != EL_BLOCKED)
    {
      // element is moving, but target field is not free (blocked), but
      // already occupied by something different (example: acid pool);
      // in this case, only remove the moving field, but not the target

      RemoveField(oldx, oldy);

      Store[oldx][oldy] = Store2[oldx][oldy] = 0;

      TEST_DrawLevelField(oldx, oldy);

      return;
    }
  }
  else if (element == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &oldx, &oldy);
    if (!IS_MOVING(oldx, oldy))
      return;
  }

  if (element == EL_BLOCKED &&
      (Tile[oldx][oldy] == EL_QUICKSAND_EMPTYING ||
       Tile[oldx][oldy] == EL_QUICKSAND_FAST_EMPTYING ||
       Tile[oldx][oldy] == EL_MAGIC_WALL_EMPTYING ||
       Tile[oldx][oldy] == EL_BD_MAGIC_WALL_EMPTYING ||
       Tile[oldx][oldy] == EL_DC_MAGIC_WALL_EMPTYING ||
       Tile[oldx][oldy] == EL_AMOEBA_DROPPING))
    next_element = get_next_element(Tile[oldx][oldy]);

  RemoveField(oldx, oldy);
  RemoveField(newx, newy);

  Store[oldx][oldy] = Store2[oldx][oldy] = 0;

  if (next_element != EL_UNDEFINED)
    Tile[oldx][oldy] = next_element;

  TEST_DrawLevelField(oldx, oldy);
  TEST_DrawLevelField(newx, newy);
}

void DrawDynamite(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic = el2img(Tile[x][y]);
  int frame;

  if (!IN_SCR_FIELD(sx, sy) || IS_PLAYER(x, y))
    return;

  if (IS_WALKABLE_INSIDE(Back[x][y]))
    return;

  if (Back[x][y])
    DrawLevelElement(x, y, Back[x][y]);
  else if (Store[x][y])
    DrawLevelElement(x, y, Store[x][y]);
  else if (game.use_masked_elements)
    DrawLevelElement(x, y, EL_EMPTY);

  frame = getGraphicAnimationFrameXY(graphic, x, y);

  if (Back[x][y] || Store[x][y] || game.use_masked_elements)
    DrawGraphicThruMask(sx, sy, graphic, frame);
  else
    DrawGraphic(sx, sy, graphic, frame);
}

static void CheckDynamite(int x, int y)
{
  if (MovDelay[x][y] != 0)	// dynamite is still waiting to explode
  {
    MovDelay[x][y]--;

    if (MovDelay[x][y] != 0)
    {
      DrawDynamite(x, y);
      PlayLevelSoundActionIfLoop(x, y, ACTION_ACTIVE);

      return;
    }
  }

  StopLevelSoundActionIfLoop(x, y, ACTION_ACTIVE);

  Bang(x, y);
}

static void setMinimalPlayerBoundaries(int *sx1, int *sy1, int *sx2, int *sy2)
{
  boolean num_checked_players = 0;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (stored_player[i].active)
    {
      int sx = stored_player[i].jx;
      int sy = stored_player[i].jy;

      if (num_checked_players == 0)
      {
	*sx1 = *sx2 = sx;
	*sy1 = *sy2 = sy;
      }
      else
      {
	*sx1 = MIN(*sx1, sx);
	*sy1 = MIN(*sy1, sy);
	*sx2 = MAX(*sx2, sx);
	*sy2 = MAX(*sy2, sy);
      }

      num_checked_players++;
    }
  }
}

static boolean checkIfAllPlayersFitToScreen_RND(void)
{
  int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;

  setMinimalPlayerBoundaries(&sx1, &sy1, &sx2, &sy2);

  return (sx2 - sx1 < SCR_FIELDX &&
	  sy2 - sy1 < SCR_FIELDY);
}

static void setScreenCenteredToAllPlayers(int *sx, int *sy)
{
  int sx1 = scroll_x, sy1 = scroll_y, sx2 = scroll_x, sy2 = scroll_y;

  setMinimalPlayerBoundaries(&sx1, &sy1, &sx2, &sy2);

  *sx = (sx1 + sx2) / 2;
  *sy = (sy1 + sy2) / 2;
}

static void DrawRelocateScreen(int old_x, int old_y, int x, int y,
			       boolean center_screen, boolean quick_relocation)
{
  unsigned int frame_delay_value_old = GetVideoFrameDelay();
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int wait_delay_value = (no_delay ? 0 : frame_delay_value);
  int new_scroll_x, new_scroll_y;

  if (level.lazy_relocation && IN_VIS_FIELD(SCREENX(x), SCREENY(y)))
  {
    // case 1: quick relocation inside visible screen (without scrolling)

    RedrawPlayfield();

    return;
  }

  if (!level.shifted_relocation || center_screen)
  {
    // relocation _with_ centering of screen

    new_scroll_x = SCROLL_POSITION_X(x);
    new_scroll_y = SCROLL_POSITION_Y(y);
  }
  else
  {
    // relocation _without_ centering of screen

    if (IN_LEV_FIELD(old_x, old_y))
    {
      // apply distance between old and new player position to scroll position
      int shifted_scroll_x = scroll_x + (x - old_x);
      int shifted_scroll_y = scroll_y + (y - old_y);

      // make sure that shifted scroll position does not scroll beyond screen
      new_scroll_x = SCROLL_POSITION_X(shifted_scroll_x + MIDPOSX);
      new_scroll_y = SCROLL_POSITION_Y(shifted_scroll_y + MIDPOSY);
    }
    else
    {
      // player not on playfield before -- scroll to initial player position
      new_scroll_x = SCROLL_POSITION_X(x);
      new_scroll_y = SCROLL_POSITION_Y(y);
    }

    // special case for teleporting from one end of the playfield to the other
    // (this kludge prevents the destination area to be shifted by half a tile
    // against the source destination for even screen width or screen height;
    // probably most useful when used with high "game.forced_scroll_delay_value"
    // in combination with "game.forced_scroll_x" and "game.forced_scroll_y")
    if (quick_relocation)
    {
      if (EVEN(SCR_FIELDX))
      {
	// relocate (teleport) between left and right border (half or full)
	if (scroll_x == SBX_Left && new_scroll_x == SBX_Right - 1)
	  new_scroll_x = SBX_Right;
	else if (scroll_x == SBX_Left + 1 && new_scroll_x == SBX_Right)
	  new_scroll_x = SBX_Right - 1;
	else if (scroll_x == SBX_Right && new_scroll_x == SBX_Left + 1)
	  new_scroll_x = SBX_Left;
	else if (scroll_x == SBX_Right - 1 && new_scroll_x == SBX_Left)
	  new_scroll_x = SBX_Left + 1;
      }

      if (EVEN(SCR_FIELDY))
      {
	// relocate (teleport) between top and bottom border (half or full)
	if (scroll_y == SBY_Upper && new_scroll_y == SBY_Lower - 1)
	  new_scroll_y = SBY_Lower;
	else if (scroll_y == SBY_Upper + 1 && new_scroll_y == SBY_Lower)
	  new_scroll_y = SBY_Lower - 1;
	else if (scroll_y == SBY_Lower && new_scroll_y == SBY_Upper + 1)
	  new_scroll_y = SBY_Upper;
	else if (scroll_y == SBY_Lower - 1 && new_scroll_y == SBY_Upper)
	  new_scroll_y = SBY_Upper + 1;
      }
    }
  }

  if (quick_relocation)
  {
    // case 2: quick relocation (redraw without visible scrolling)

    scroll_x = new_scroll_x;
    scroll_y = new_scroll_y;

    RedrawPlayfield();

    return;
  }

  // case 3: visible relocation (with scrolling to new position)

  ScrollScreen(NULL, SCROLL_GO_ON);	// scroll last frame to full tile

  SetVideoFrameDelay(wait_delay_value);

  while (scroll_x != new_scroll_x || scroll_y != new_scroll_y)
  {
    int dx = (new_scroll_x < scroll_x ? +1 : new_scroll_x > scroll_x ? -1 : 0);
    int dy = (new_scroll_y < scroll_y ? +1 : new_scroll_y > scroll_y ? -1 : 0);

    if (dx == 0 && dy == 0)		// no scrolling needed at all
      break;

    scroll_x -= dx;
    scroll_y -= dy;

    // set values for horizontal/vertical screen scrolling (half tile size)
    int dir_x = (dx != 0 ? MV_HORIZONTAL : 0);
    int dir_y = (dy != 0 ? MV_VERTICAL   : 0);
    int pos_x = dx * TILEX / 2;
    int pos_y = dy * TILEY / 2;
    int fx = getFieldbufferOffsetX_RND(dir_x, pos_x);
    int fy = getFieldbufferOffsetY_RND(dir_y, pos_y);

    ScrollLevel(dx, dy);
    DrawAllPlayers();

    // scroll in two steps of half tile size to make things smoother
    BlitScreenToBitmapExt_RND(window, fx, fy);

    // scroll second step to align at full tile size
    BlitScreenToBitmap(window);
  }

  DrawAllPlayers();
  BackToFront();

  SetVideoFrameDelay(frame_delay_value_old);
}

static void RelocatePlayer(int jx, int jy, int el_player_raw)
{
  int el_player = GET_PLAYER_ELEMENT(el_player_raw);
  int player_nr = GET_PLAYER_NR(el_player);
  struct PlayerInfo *player = &stored_player[player_nr];
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int wait_delay_value = (no_delay ? 0 : frame_delay_value);
  int old_jx = player->jx;
  int old_jy = player->jy;
  int element = Tile[jx][jy];
  boolean player_relocated = (old_jx != jx || old_jy != jy);

  int move_dir_horiz = (jx < old_jx ? MV_LEFT : jx > old_jx ? MV_RIGHT : 0);
  int move_dir_vert  = (jy < old_jy ? MV_UP   : jy > old_jy ? MV_DOWN  : 0);
  int enter_side_horiz = MV_DIR_OPPOSITE(move_dir_horiz);
  int enter_side_vert  = MV_DIR_OPPOSITE(move_dir_vert);
  int leave_side_horiz = move_dir_horiz;
  int leave_side_vert  = move_dir_vert;
  int enter_side = enter_side_horiz | enter_side_vert;
  int leave_side = leave_side_horiz | leave_side_vert;

  if (player->buried)		// do not reanimate dead player
    return;

  if (!player_relocated)	// no need to relocate the player
    return;

  if (IS_PLAYER(jx, jy))	// player already placed at new position
  {
    RemoveField(jx, jy);	// temporarily remove newly placed player
    DrawLevelField(jx, jy);
  }

  if (player->present)
  {
    while (player->MovPos)
    {
      ScrollPlayer(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);

      AdvanceFrameAndPlayerCounters(player->index_nr);

      DrawPlayer(player);

      BackToFront_WithFrameDelay(wait_delay_value);
    }

    DrawPlayer(player);		// needed here only to cleanup last field
    DrawLevelField(player->jx, player->jy);	// remove player graphic

    player->is_moving = FALSE;
  }

  if (IN_LEV_FIELD(old_jx, old_jy))
  {
    int old_element = Tile[old_jx][old_jy];

    if (IS_CUSTOM_ELEMENT(old_element))
      CheckElementChangeByPlayer(old_jx, old_jy, old_element,
				 CE_LEFT_BY_PLAYER,
				 player->index_bit, leave_side);

    CheckTriggeredElementChangeByPlayer(old_jx, old_jy, old_element,
					CE_PLAYER_LEAVES_X,
					player->index_bit, leave_side);
  }

  Tile[jx][jy] = el_player;
  InitPlayerField(jx, jy, el_player, TRUE);

  /* "InitPlayerField()" above sets Tile[jx][jy] to EL_EMPTY, but it may be
     possible that the relocation target field did not contain a player element,
     but a walkable element, to which the new player was relocated -- in this
     case, restore that (already initialized!) element on the player field */
  if (!IS_PLAYER_ELEMENT(element))	// player may be set on walkable element
  {
    Tile[jx][jy] = element;	// restore previously existing element
  }

  // only visually relocate centered player
  DrawRelocateScreen(old_jx, old_jy, player->jx, player->jy,
		     FALSE, level.instant_relocation);

  TestIfPlayerTouchesBadThing(jx, jy);
  TestIfPlayerTouchesCustomElement(jx, jy);

  if (IN_LEV_FIELD(old_jx, old_jy))
  {
    if (IS_CUSTOM_ELEMENT(element))
      CheckElementChangeByPlayer(jx, jy, element, CE_ENTERED_BY_PLAYER,
				 player->index_bit, enter_side);

    CheckTriggeredElementChangeByPlayer(jx, jy, element, CE_PLAYER_ENTERS_X,
					player->index_bit, enter_side);

    if (player->is_switching)
    {
      /* ensure that relocation while still switching an element does not cause
         a new element to be treated as also switched directly after relocation
         (this is important for teleporter switches that teleport the player to
         a place where another teleporter switch is in the same direction, which
         would then incorrectly be treated as immediately switched before the
         direction key that caused the switch was released) */

      player->switch_x += jx - old_jx;
      player->switch_y += jy - old_jy;
    }
  }
}

static void Explode(int ex, int ey, int phase, int mode)
{
  int x, y;
  int last_phase;
  int border_element;

  if (game.explosions_delayed)
  {
    ExplodeField[ex][ey] = mode;
    return;
  }

  if (phase == EX_PHASE_START)		// initialize 'Store[][]' field
  {
    int center_element = Tile[ex][ey];
    int ce_value = CustomValue[ex][ey];
    int ce_score = element_info[center_element].collect_score;
    int artwork_element, explosion_element;	// set these values later

    // remove things displayed in background while burning dynamite
    if (Back[ex][ey] != EL_EMPTY && !IS_INDESTRUCTIBLE(Back[ex][ey]))
      Back[ex][ey] = 0;

    if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
    {
      // put moving element to center field (and let it explode there)
      center_element = MovingOrBlocked2Element(ex, ey);
      RemoveMovingField(ex, ey);
      Tile[ex][ey] = center_element;
    }

    // now "center_element" is finally determined -- set related values now
    artwork_element = center_element;		// for custom player artwork
    explosion_element = center_element;		// for custom player artwork

    if (IS_PLAYER(ex, ey))
    {
      int player_nr = GET_PLAYER_NR(StorePlayer[ex][ey]);

      artwork_element = stored_player[player_nr].artwork_element;

      if (level.use_explosion_element[player_nr])
      {
	explosion_element = level.explosion_element[player_nr];
	artwork_element = explosion_element;
      }
    }

    if (mode == EX_TYPE_NORMAL ||
	mode == EX_TYPE_CENTER ||
	mode == EX_TYPE_CROSS)
      PlayLevelSoundElementAction(ex, ey, artwork_element, ACTION_EXPLODING);

    last_phase = element_info[explosion_element].explosion_delay + 1;

    for (y = ey - 1; y <= ey + 1; y++) for (x = ex - 1; x <= ex + 1; x++)
    {
      int xx = x - ex + 1;
      int yy = y - ey + 1;
      int element;

      if (!IN_LEV_FIELD(x, y) ||
	  (mode & EX_TYPE_SINGLE_TILE && (x != ex || y != ey)) ||
	  (mode == EX_TYPE_CROSS      && (x != ex && y != ey)))
	continue;

      element = Tile[x][y];

      if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
      {
	element = MovingOrBlocked2Element(x, y);

	if (!IS_EXPLOSION_PROOF(element))
	  RemoveMovingField(x, y);
      }

      // indestructible elements can only explode in center (but not flames)
      if ((IS_EXPLOSION_PROOF(element) && (x != ex || y != ey ||
					   mode == EX_TYPE_BORDER)) ||
	  element == EL_FLAMES)
	continue;

      /* no idea why this was changed from 3.0.8 to 3.1.0 -- this causes buggy
	 behaviour, for example when touching a yamyam that explodes to rocks
	 with active deadly shield, a rock is created under the player !!! */
      // (case 1 (surely buggy): >= 3.1.0, case 2 (maybe buggy): <= 3.0.8)
#if 0
      if (IS_PLAYER(x, y) && SHIELD_ON(PLAYERINFO(x, y)) &&
	  (game.engine_version < VERSION_IDENT(3,1,0,0) ||
	   (x == ex && y == ey && mode != EX_TYPE_BORDER)))
#else
      if (IS_PLAYER(x, y) && SHIELD_ON(PLAYERINFO(x, y)))
#endif
      {
	if (IS_ACTIVE_BOMB(element))
	{
	  // re-activate things under the bomb like gate or penguin
	  Tile[x][y] = (Back[x][y] ? Back[x][y] : EL_EMPTY);
	  Back[x][y] = 0;
	}

	continue;
      }

      // save walkable background elements while explosion on same tile
      if (IS_WALKABLE(element) && IS_INDESTRUCTIBLE(element) &&
	  (x != ex || y != ey || mode == EX_TYPE_BORDER))
	Back[x][y] = element;

      // ignite explodable elements reached by other explosion
      if (element == EL_EXPLOSION)
	element = Store2[x][y];

      if (AmoebaNr[x][y] &&
	  (element == EL_AMOEBA_FULL ||
	   element == EL_BD_AMOEBA ||
	   element == EL_AMOEBA_GROWING))
      {
	AmoebaCnt[AmoebaNr[x][y]]--;
	AmoebaCnt2[AmoebaNr[x][y]]--;
      }

      RemoveField(x, y);

      if (IS_PLAYER(ex, ey) && !PLAYER_EXPLOSION_PROTECTED(ex, ey))
      {
	int player_nr = StorePlayer[ex][ey] - EL_PLAYER_1;

	Store[x][y] = EL_PLAYER_IS_EXPLODING_1 + player_nr;

	if (PLAYERINFO(ex, ey)->use_murphy)
	  Store[x][y] = EL_EMPTY;
      }

      // !!! check this case -- currently needed for rnd_rado_negundo_v,
      // !!! levels 015 018 019 020 021 022 023 026 027 028 !!!
      else if (IS_PLAYER_ELEMENT(center_element))
	Store[x][y] = EL_EMPTY;
      else if (center_element == EL_YAMYAM)
	Store[x][y] = level.yamyam_content[game.yamyam_content_nr].e[xx][yy];
      else if (element_info[center_element].content.e[xx][yy] != EL_EMPTY)
	Store[x][y] = element_info[center_element].content.e[xx][yy];
#if 1
      // needed because EL_BD_BUTTERFLY is not defined as "CAN_EXPLODE"
      // (killing EL_BD_BUTTERFLY with dynamite would result in BD diamond
      // otherwise) -- FIX THIS !!!
      else if (!CAN_EXPLODE(element) && element != EL_BD_BUTTERFLY)
	Store[x][y] = element_info[element].content.e[1][1];
#else
      else if (!CAN_EXPLODE(element))
	Store[x][y] = element_info[element].content.e[1][1];
#endif
      else
	Store[x][y] = EL_EMPTY;

      if (IS_CUSTOM_ELEMENT(center_element))
	Store[x][y] = (Store[x][y] == EL_CURRENT_CE_VALUE ? ce_value :
		       Store[x][y] == EL_CURRENT_CE_SCORE ? ce_score :
		       Store[x][y] >= EL_PREV_CE_8 &&
		       Store[x][y] <= EL_NEXT_CE_8 ?
		       RESOLVED_REFERENCE_ELEMENT(center_element, Store[x][y]) :
		       Store[x][y]);

      if (x != ex || y != ey || mode == EX_TYPE_BORDER ||
	  center_element == EL_AMOEBA_TO_DIAMOND)
	Store2[x][y] = element;

      Tile[x][y] = EL_EXPLOSION;
      GfxElement[x][y] = artwork_element;

      ExplodePhase[x][y] = 1;
      ExplodeDelay[x][y] = last_phase;

      Stop[x][y] = TRUE;
    }

    if (center_element == EL_YAMYAM)
      game.yamyam_content_nr =
	(game.yamyam_content_nr + 1) % level.num_yamyam_contents;

    return;
  }

  if (Stop[ex][ey])
    return;

  x = ex;
  y = ey;

  if (phase == 1)
    GfxFrame[x][y] = 0;		// restart explosion animation

  last_phase = ExplodeDelay[x][y];

  ExplodePhase[x][y] = (phase < last_phase ? phase + 1 : 0);

  // this can happen if the player leaves an explosion just in time
  if (GfxElement[x][y] == EL_UNDEFINED)
    GfxElement[x][y] = EL_EMPTY;

  border_element = Store2[x][y];
  if (IS_PLAYER(x, y) && !PLAYER_EXPLOSION_PROTECTED(x, y))
    border_element = StorePlayer[x][y];

  if (phase == element_info[border_element].ignition_delay ||
      phase == last_phase)
  {
    boolean border_explosion = FALSE;

    if (IS_PLAYER(x, y) && PLAYERINFO(x, y)->present &&
	!PLAYER_EXPLOSION_PROTECTED(x, y))
    {
      KillPlayerUnlessExplosionProtected(x, y);
      border_explosion = TRUE;
    }
    else if (CAN_EXPLODE_BY_EXPLOSION(border_element))
    {
      Tile[x][y] = Store2[x][y];
      Store2[x][y] = 0;
      Bang(x, y);
      border_explosion = TRUE;
    }
    else if (border_element == EL_AMOEBA_TO_DIAMOND)
    {
      AmoebaToDiamond(x, y);
      Store2[x][y] = 0;
      border_explosion = TRUE;
    }

    // if an element just explodes due to another explosion (chain-reaction),
    // do not immediately end the new explosion when it was the last frame of
    // the explosion (as it would be done in the following "if"-statement!)
    if (border_explosion && phase == last_phase)
      return;
  }

  // this can happen if the player was just killed by an explosion
  if (GfxElement[x][y] == EL_UNDEFINED)
    GfxElement[x][y] = EL_EMPTY;

  if (phase == last_phase)
  {
    int element;

    element = Tile[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    GfxElement[x][y] = EL_UNDEFINED;

    // player can escape from explosions and might therefore be still alive
    if (element >= EL_PLAYER_IS_EXPLODING_1 &&
	element <= EL_PLAYER_IS_EXPLODING_4)
    {
      int player_nr = element - EL_PLAYER_IS_EXPLODING_1;
      int explosion_element = EL_PLAYER_1 + player_nr;
      int xx = MIN(MAX(0, x - stored_player[player_nr].jx + 1), 2);
      int yy = MIN(MAX(0, y - stored_player[player_nr].jy + 1), 2);

      if (level.use_explosion_element[player_nr])
	explosion_element = level.explosion_element[player_nr];

      Tile[x][y] = (stored_player[player_nr].active ? EL_EMPTY :
		    element_info[explosion_element].content.e[xx][yy]);
    }

    // restore probably existing indestructible background element
    if (Back[x][y] && IS_INDESTRUCTIBLE(Back[x][y]))
      element = Tile[x][y] = Back[x][y];
    Back[x][y] = 0;

    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    GfxDir[x][y] = MV_NONE;
    ChangeDelay[x][y] = 0;
    ChangePage[x][y] = -1;

    CustomValue[x][y] = 0;

    InitField_WithBug2(x, y, FALSE);

    TEST_DrawLevelField(x, y);

    TestIfElementTouchesCustomElement(x, y);

    if (GFX_CRUMBLED(element))
      TEST_DrawLevelFieldCrumbledNeighbours(x, y);

    if (IS_PLAYER(x, y) && !PLAYERINFO(x, y)->present)
      StorePlayer[x][y] = 0;

    if (IS_PLAYER_ELEMENT(element))
      RelocatePlayer(x, y, element);
  }
  else if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    int graphic = el_act2img(GfxElement[x][y], ACTION_EXPLODING);
    int frame = getGraphicAnimationFrameXY(graphic, x, y);

    if (phase == 1)
      TEST_DrawLevelFieldCrumbled(x, y);

    if (IS_WALKABLE_OVER(Back[x][y]) && Back[x][y] != EL_EMPTY)
    {
      DrawLevelElement(x, y, Back[x][y]);
      DrawGraphicThruMask(SCREENX(x), SCREENY(y), graphic, frame);
    }
    else if (IS_WALKABLE_UNDER(Back[x][y]))
    {
      DrawLevelGraphic(x, y, graphic, frame);
      DrawLevelElementThruMask(x, y, Back[x][y]);
    }
    else if (!IS_WALKABLE_INSIDE(Back[x][y]))
      DrawLevelGraphic(x, y, graphic, frame);
  }
}

static void DynaExplode(int ex, int ey)
{
  int i, j;
  int dynabomb_element = Tile[ex][ey];
  int dynabomb_size = 1;
  boolean dynabomb_xl = FALSE;
  struct PlayerInfo *player;
  struct XY *xy = xy_topdown;

  if (IS_ACTIVE_BOMB(dynabomb_element))
  {
    player = &stored_player[dynabomb_element - EL_DYNABOMB_PLAYER_1_ACTIVE];
    dynabomb_size = player->dynabomb_size;
    dynabomb_xl = player->dynabomb_xl;
    player->dynabombs_left++;
  }

  Explode(ex, ey, EX_PHASE_START, EX_TYPE_CENTER);

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    for (j = 1; j <= dynabomb_size; j++)
    {
      int x = ex + j * xy[i].x;
      int y = ey + j * xy[i].y;
      int element;

      if (!IN_LEV_FIELD(x, y) || IS_INDESTRUCTIBLE(Tile[x][y]))
	break;

      element = Tile[x][y];

      // do not restart explosions of fields with active bombs
      if (element == EL_EXPLOSION && IS_ACTIVE_BOMB(Store2[x][y]))
	continue;

      Explode(x, y, EX_PHASE_START, EX_TYPE_BORDER);

      if (element != EL_EMPTY && element != EL_EXPLOSION &&
	  !IS_DIGGABLE(element) && !dynabomb_xl)
	break;
    }
  }
}

void Bang(int x, int y)
{
  int element = MovingOrBlocked2Element(x, y);
  int explosion_type = EX_TYPE_NORMAL;

  if (IS_PLAYER(x, y) && !PLAYER_EXPLOSION_PROTECTED(x, y))
  {
    struct PlayerInfo *player = PLAYERINFO(x, y);

    element = Tile[x][y] = player->initial_element;

    if (level.use_explosion_element[player->index_nr])
    {
      int explosion_element = level.explosion_element[player->index_nr];

      if (element_info[explosion_element].explosion_type == EXPLODES_CROSS)
	explosion_type = EX_TYPE_CROSS;
      else if (element_info[explosion_element].explosion_type == EXPLODES_1X1)
	explosion_type = EX_TYPE_CENTER;
    }
  }

  switch (element)
  {
    case EL_BUG:
    case EL_SPACESHIP:
    case EL_BD_BUTTERFLY:
    case EL_BD_FIREFLY:
    case EL_YAMYAM:
    case EL_DARK_YAMYAM:
    case EL_ROBOT:
    case EL_PACMAN:
    case EL_MOLE:
      RaiseScoreElement(element);
      break;

    case EL_DYNABOMB_PLAYER_1_ACTIVE:
    case EL_DYNABOMB_PLAYER_2_ACTIVE:
    case EL_DYNABOMB_PLAYER_3_ACTIVE:
    case EL_DYNABOMB_PLAYER_4_ACTIVE:
    case EL_DYNABOMB_INCREASE_NUMBER:
    case EL_DYNABOMB_INCREASE_SIZE:
    case EL_DYNABOMB_INCREASE_POWER:
      explosion_type = EX_TYPE_DYNA;
      break;

    case EL_DC_LANDMINE:
      explosion_type = EX_TYPE_CENTER;
      break;

    case EL_PENGUIN:
    case EL_LAMP:
    case EL_LAMP_ACTIVE:
    case EL_AMOEBA_TO_DIAMOND:
      if (!IS_PLAYER(x, y))	// penguin and player may be at same field
	explosion_type = EX_TYPE_CENTER;
      break;

    default:
      if (element_info[element].explosion_type == EXPLODES_CROSS)
	explosion_type = EX_TYPE_CROSS;
      else if (element_info[element].explosion_type == EXPLODES_1X1)
	explosion_type = EX_TYPE_CENTER;
      break;
  }

  if (explosion_type == EX_TYPE_DYNA)
    DynaExplode(x, y);
  else
    Explode(x, y, EX_PHASE_START, explosion_type);

  CheckTriggeredElementChange(x, y, element, CE_EXPLOSION_OF_X);
}

static void SplashAcid(int x, int y)
{
  if (IN_LEV_FIELD(x - 1, y - 1) && IS_FREE(x - 1, y - 1) &&
      (!IN_LEV_FIELD(x - 1, y - 2) ||
       !CAN_FALL(MovingOrBlocked2Element(x - 1, y - 2))))
    Tile[x - 1][y - 1] = EL_ACID_SPLASH_LEFT;

  if (IN_LEV_FIELD(x + 1, y - 1) && IS_FREE(x + 1, y - 1) &&
      (!IN_LEV_FIELD(x + 1, y - 2) ||
       !CAN_FALL(MovingOrBlocked2Element(x + 1, y - 2))))
    Tile[x + 1][y - 1] = EL_ACID_SPLASH_RIGHT;

  PlayLevelSound(x, y, SND_ACID_SPLASHING);
}

static void InitBeltMovement(void)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE
  };

  int x, y, i, j;

  // set frame order for belt animation graphic according to belt direction
  for (i = 0; i < NUM_BELTS; i++)
  {
    int belt_nr = i;

    for (j = 0; j < NUM_BELT_PARTS; j++)
    {
      int element = belt_base_active_element[belt_nr] + j;
      int graphic_1 = el2img(element);
      int graphic_2 = el2panelimg(element);

      if (game.belt_dir[i] == MV_LEFT)
      {
	graphic_info[graphic_1].anim_mode &= ~ANIM_REVERSE;
	graphic_info[graphic_2].anim_mode &= ~ANIM_REVERSE;
      }
      else
      {
	graphic_info[graphic_1].anim_mode |=  ANIM_REVERSE;
	graphic_info[graphic_2].anim_mode |=  ANIM_REVERSE;
      }
    }
  }

  SCAN_PLAYFIELD(x, y)
  {
    int element = Tile[x][y];

    for (i = 0; i < NUM_BELTS; i++)
    {
      if (IS_BELT(element) && game.belt_dir[i] != MV_NONE)
      {
	int e_belt_nr = getBeltNrFromBeltElement(element);
	int belt_nr = i;

	if (e_belt_nr == belt_nr)
	{
	  int belt_part = Tile[x][y] - belt_base_element[belt_nr];

	  Tile[x][y] = belt_base_active_element[belt_nr] + belt_part;
	}
      }
    }
  }
}

static void ToggleBeltSwitch(int x, int y)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE
  };
  static int belt_base_switch_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
  };
  static int belt_move_dir[4] =
  {
    MV_LEFT,
    MV_NONE,
    MV_RIGHT,
    MV_NONE,
  };

  int element = Tile[x][y];
  int belt_nr = getBeltNrFromBeltSwitchElement(element);
  int belt_dir_nr = (game.belt_dir_nr[belt_nr] + 1) % 4;
  int belt_dir = belt_move_dir[belt_dir_nr];
  int xx, yy, i;

  if (!IS_BELT_SWITCH(element))
    return;

  game.belt_dir_nr[belt_nr] = belt_dir_nr;
  game.belt_dir[belt_nr] = belt_dir;

  if (belt_dir_nr == 3)
    belt_dir_nr = 1;

  // set frame order for belt animation graphic according to belt direction
  for (i = 0; i < NUM_BELT_PARTS; i++)
  {
    int element = belt_base_active_element[belt_nr] + i;
    int graphic_1 = el2img(element);
    int graphic_2 = el2panelimg(element);

    if (belt_dir == MV_LEFT)
    {
      graphic_info[graphic_1].anim_mode &= ~ANIM_REVERSE;
      graphic_info[graphic_2].anim_mode &= ~ANIM_REVERSE;
    }
    else
    {
      graphic_info[graphic_1].anim_mode |=  ANIM_REVERSE;
      graphic_info[graphic_2].anim_mode |=  ANIM_REVERSE;
    }
  }

  SCAN_PLAYFIELD(xx, yy)
  {
    int element = Tile[xx][yy];

    if (IS_BELT_SWITCH(element))
    {
      int e_belt_nr = getBeltNrFromBeltSwitchElement(element);

      if (e_belt_nr == belt_nr)
      {
	Tile[xx][yy] = belt_base_switch_element[belt_nr] + belt_dir_nr;
	TEST_DrawLevelField(xx, yy);
      }
    }
    else if (IS_BELT(element) && belt_dir != MV_NONE)
    {
      int e_belt_nr = getBeltNrFromBeltElement(element);

      if (e_belt_nr == belt_nr)
      {
	int belt_part = Tile[xx][yy] - belt_base_element[belt_nr];

	Tile[xx][yy] = belt_base_active_element[belt_nr] + belt_part;
	TEST_DrawLevelField(xx, yy);
      }
    }
    else if (IS_BELT_ACTIVE(element) && belt_dir == MV_NONE)
    {
      int e_belt_nr = getBeltNrFromBeltActiveElement(element);

      if (e_belt_nr == belt_nr)
      {
	int belt_part = Tile[xx][yy] - belt_base_active_element[belt_nr];

	Tile[xx][yy] = belt_base_element[belt_nr] + belt_part;
	TEST_DrawLevelField(xx, yy);
      }
    }
  }
}

static void ToggleSwitchgateSwitch(void)
{
  int xx, yy;

  game.switchgate_pos = !game.switchgate_pos;

  SCAN_PLAYFIELD(xx, yy)
  {
    int element = Tile[xx][yy];

    if (element == EL_SWITCHGATE_SWITCH_UP)
    {
      Tile[xx][yy] = EL_SWITCHGATE_SWITCH_DOWN;
      TEST_DrawLevelField(xx, yy);
    }
    else if (element == EL_SWITCHGATE_SWITCH_DOWN)
    {
      Tile[xx][yy] = EL_SWITCHGATE_SWITCH_UP;
      TEST_DrawLevelField(xx, yy);
    }
    else if (element == EL_DC_SWITCHGATE_SWITCH_UP)
    {
      Tile[xx][yy] = EL_DC_SWITCHGATE_SWITCH_DOWN;
      TEST_DrawLevelField(xx, yy);
    }
    else if (element == EL_DC_SWITCHGATE_SWITCH_DOWN)
    {
      Tile[xx][yy] = EL_DC_SWITCHGATE_SWITCH_UP;
      TEST_DrawLevelField(xx, yy);
    }
    else if (element == EL_SWITCHGATE_OPEN ||
	     element == EL_SWITCHGATE_OPENING)
    {
      Tile[xx][yy] = EL_SWITCHGATE_CLOSING;

      PlayLevelSoundAction(xx, yy, ACTION_CLOSING);
    }
    else if (element == EL_SWITCHGATE_CLOSED ||
	     element == EL_SWITCHGATE_CLOSING)
    {
      Tile[xx][yy] = EL_SWITCHGATE_OPENING;

      PlayLevelSoundAction(xx, yy, ACTION_OPENING);
    }
  }
}

static int getInvisibleActiveFromInvisibleElement(int element)
{
  return (element == EL_INVISIBLE_STEELWALL ? EL_INVISIBLE_STEELWALL_ACTIVE :
	  element == EL_INVISIBLE_WALL      ? EL_INVISIBLE_WALL_ACTIVE :
	  element == EL_INVISIBLE_SAND      ? EL_INVISIBLE_SAND_ACTIVE :
	  element);
}

static int getInvisibleFromInvisibleActiveElement(int element)
{
  return (element == EL_INVISIBLE_STEELWALL_ACTIVE ? EL_INVISIBLE_STEELWALL :
	  element == EL_INVISIBLE_WALL_ACTIVE      ? EL_INVISIBLE_WALL :
	  element == EL_INVISIBLE_SAND_ACTIVE      ? EL_INVISIBLE_SAND :
	  element);
}

static void RedrawAllLightSwitchesAndInvisibleElements(void)
{
  int x, y;

  SCAN_PLAYFIELD(x, y)
  {
    int element = Tile[x][y];

    if (element == EL_LIGHT_SWITCH &&
	game.light_time_left > 0)
    {
      Tile[x][y] = EL_LIGHT_SWITCH_ACTIVE;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_LIGHT_SWITCH_ACTIVE &&
	     game.light_time_left == 0)
    {
      Tile[x][y] = EL_LIGHT_SWITCH;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_EMC_DRIPPER &&
	     game.light_time_left > 0)
    {
      Tile[x][y] = EL_EMC_DRIPPER_ACTIVE;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_EMC_DRIPPER_ACTIVE &&
	     game.light_time_left == 0)
    {
      Tile[x][y] = EL_EMC_DRIPPER;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_INVISIBLE_STEELWALL ||
	     element == EL_INVISIBLE_WALL ||
	     element == EL_INVISIBLE_SAND)
    {
      if (game.light_time_left > 0)
	Tile[x][y] = getInvisibleActiveFromInvisibleElement(element);

      TEST_DrawLevelField(x, y);

      // uncrumble neighbour fields, if needed
      if (element == EL_INVISIBLE_SAND)
	TEST_DrawLevelFieldCrumbledNeighbours(x, y);
    }
    else if (element == EL_INVISIBLE_STEELWALL_ACTIVE ||
	     element == EL_INVISIBLE_WALL_ACTIVE ||
	     element == EL_INVISIBLE_SAND_ACTIVE)
    {
      if (game.light_time_left == 0)
	Tile[x][y] = getInvisibleFromInvisibleActiveElement(element);

      TEST_DrawLevelField(x, y);

      // re-crumble neighbour fields, if needed
      if (element == EL_INVISIBLE_SAND)
	TEST_DrawLevelFieldCrumbledNeighbours(x, y);
    }
  }
}

static void RedrawAllInvisibleElementsForLenses(void)
{
  int x, y;

  SCAN_PLAYFIELD(x, y)
  {
    int element = Tile[x][y];

    if (element == EL_EMC_DRIPPER &&
	game.lenses_time_left > 0)
    {
      Tile[x][y] = EL_EMC_DRIPPER_ACTIVE;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_EMC_DRIPPER_ACTIVE &&
	     game.lenses_time_left == 0)
    {
      Tile[x][y] = EL_EMC_DRIPPER;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_INVISIBLE_STEELWALL ||
	     element == EL_INVISIBLE_WALL ||
	     element == EL_INVISIBLE_SAND)
    {
      if (game.lenses_time_left > 0)
	Tile[x][y] = getInvisibleActiveFromInvisibleElement(element);

      TEST_DrawLevelField(x, y);

      // uncrumble neighbour fields, if needed
      if (element == EL_INVISIBLE_SAND)
	TEST_DrawLevelFieldCrumbledNeighbours(x, y);
    }
    else if (element == EL_INVISIBLE_STEELWALL_ACTIVE ||
	     element == EL_INVISIBLE_WALL_ACTIVE ||
	     element == EL_INVISIBLE_SAND_ACTIVE)
    {
      if (game.lenses_time_left == 0)
	Tile[x][y] = getInvisibleFromInvisibleActiveElement(element);

      TEST_DrawLevelField(x, y);

      // re-crumble neighbour fields, if needed
      if (element == EL_INVISIBLE_SAND)
	TEST_DrawLevelFieldCrumbledNeighbours(x, y);
    }
  }
}

static void RedrawAllInvisibleElementsForMagnifier(void)
{
  int x, y;

  SCAN_PLAYFIELD(x, y)
  {
    int element = Tile[x][y];

    if (element == EL_EMC_FAKE_GRASS &&
	game.magnify_time_left > 0)
    {
      Tile[x][y] = EL_EMC_FAKE_GRASS_ACTIVE;
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_EMC_FAKE_GRASS_ACTIVE &&
	     game.magnify_time_left == 0)
    {
      Tile[x][y] = EL_EMC_FAKE_GRASS;
      TEST_DrawLevelField(x, y);
    }
    else if (IS_GATE_GRAY(element) &&
	     game.magnify_time_left > 0)
    {
      Tile[x][y] = (IS_RND_GATE_GRAY(element) ?
		    element - EL_GATE_1_GRAY + EL_GATE_1_GRAY_ACTIVE :
		    IS_EM_GATE_GRAY(element) ?
		    element - EL_EM_GATE_1_GRAY + EL_EM_GATE_1_GRAY_ACTIVE :
		    IS_EMC_GATE_GRAY(element) ?
		    element - EL_EMC_GATE_5_GRAY + EL_EMC_GATE_5_GRAY_ACTIVE :
		    IS_DC_GATE_GRAY(element) ?
		    EL_DC_GATE_WHITE_GRAY_ACTIVE :
		    element);
      TEST_DrawLevelField(x, y);
    }
    else if (IS_GATE_GRAY_ACTIVE(element) &&
	     game.magnify_time_left == 0)
    {
      Tile[x][y] = (IS_RND_GATE_GRAY_ACTIVE(element) ?
		    element - EL_GATE_1_GRAY_ACTIVE + EL_GATE_1_GRAY :
		    IS_EM_GATE_GRAY_ACTIVE(element) ?
		    element - EL_EM_GATE_1_GRAY_ACTIVE + EL_EM_GATE_1_GRAY :
		    IS_EMC_GATE_GRAY_ACTIVE(element) ?
		    element - EL_EMC_GATE_5_GRAY_ACTIVE + EL_EMC_GATE_5_GRAY :
		    IS_DC_GATE_GRAY_ACTIVE(element) ?
		    EL_DC_GATE_WHITE_GRAY :
		    element);
      TEST_DrawLevelField(x, y);
    }
  }
}

static void ToggleLightSwitch(int x, int y)
{
  int element = Tile[x][y];

  game.light_time_left =
    (element == EL_LIGHT_SWITCH ?
     level.time_light * FRAMES_PER_SECOND : 0);

  RedrawAllLightSwitchesAndInvisibleElements();
}

static void ActivateTimegateSwitch(int x, int y)
{
  int xx, yy;

  game.timegate_time_left = level.time_timegate * FRAMES_PER_SECOND;

  SCAN_PLAYFIELD(xx, yy)
  {
    int element = Tile[xx][yy];

    if (element == EL_TIMEGATE_CLOSED ||
	element == EL_TIMEGATE_CLOSING)
    {
      Tile[xx][yy] = EL_TIMEGATE_OPENING;
      PlayLevelSound(xx, yy, SND_CLASS_TIMEGATE_OPENING);
    }

    /*
    else if (element == EL_TIMEGATE_SWITCH_ACTIVE)
    {
      Tile[xx][yy] = EL_TIMEGATE_SWITCH;
      TEST_DrawLevelField(xx, yy);
    }
    */

  }

  Tile[x][y] = (Tile[x][y] == EL_TIMEGATE_SWITCH ? EL_TIMEGATE_SWITCH_ACTIVE :
		EL_DC_TIMEGATE_SWITCH_ACTIVE);
}

static void Impact(int x, int y)
{
  boolean last_line = (y == lev_fieldy - 1);
  boolean object_hit = FALSE;
  boolean impact = (last_line || object_hit);
  int element = Tile[x][y];
  int smashed = EL_STEELWALL;

  if (!last_line)	// check if element below was hit
  {
    if (Tile[x][y + 1] == EL_PLAYER_IS_LEAVING)
      return;

    object_hit = (!IS_FREE(x, y + 1) && (!IS_MOVING(x, y + 1) ||
					 MovDir[x][y + 1] != MV_DOWN ||
					 MovPos[x][y + 1] <= TILEY / 2));

    // do not smash moving elements that left the smashed field in time
    if (game.engine_version >= VERSION_IDENT(2,2,0,7) && IS_MOVING(x, y + 1) &&
	ABS(MovPos[x][y + 1] + getElementMoveStepsize(x, y + 1)) >= TILEX)
      object_hit = FALSE;

#if USE_QUICKSAND_IMPACT_BUGFIX
    if (Tile[x][y + 1] == EL_QUICKSAND_EMPTYING && object_hit == FALSE)
    {
      RemoveMovingField(x, y + 1);
      Tile[x][y + 1] = EL_QUICKSAND_EMPTY;
      Tile[x][y + 2] = EL_ROCK;
      TEST_DrawLevelField(x, y + 2);

      object_hit = TRUE;
    }

    if (Tile[x][y + 1] == EL_QUICKSAND_FAST_EMPTYING && object_hit == FALSE)
    {
      RemoveMovingField(x, y + 1);
      Tile[x][y + 1] = EL_QUICKSAND_FAST_EMPTY;
      Tile[x][y + 2] = EL_ROCK;
      TEST_DrawLevelField(x, y + 2);

      object_hit = TRUE;
    }
#endif

    if (object_hit)
      smashed = MovingOrBlocked2Element(x, y + 1);

    impact = (last_line || object_hit);
  }

  if (!last_line && smashed == EL_ACID)	// element falls into acid
  {
    SplashAcid(x, y + 1);
    return;
  }

  // !!! not sufficient for all cases -- see EL_PEARL below !!!
  // only reset graphic animation if graphic really changes after impact
  if (impact &&
      el_act_dir2img(element, GfxAction[x][y], MV_DOWN) != el2img(element))
  {
    ResetGfxAnimation(x, y);
    TEST_DrawLevelField(x, y);
  }

  if (impact && CAN_EXPLODE_IMPACT(element))
  {
    Bang(x, y);
    return;
  }
  else if (impact && element == EL_PEARL &&
	   smashed != EL_DC_MAGIC_WALL && smashed != EL_DC_MAGIC_WALL_ACTIVE)
  {
    ResetGfxAnimation(x, y);

    Tile[x][y] = EL_PEARL_BREAKING;
    PlayLevelSound(x, y, SND_PEARL_BREAKING);
    return;
  }
  else if (impact && CheckElementChange(x, y, element, smashed, CE_IMPACT))
  {
    PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);

    return;
  }

  if (impact && element == EL_AMOEBA_DROP)
  {
    if (object_hit && IS_PLAYER(x, y + 1))
      KillPlayerUnlessEnemyProtected(x, y + 1);
    else if (object_hit && smashed == EL_PENGUIN)
      Bang(x, y + 1);
    else
    {
      Tile[x][y] = EL_AMOEBA_GROWING;
      Store[x][y] = EL_AMOEBA_WET;

      ResetRandomAnimationValue(x, y);
    }
    return;
  }

  if (object_hit)		// check which object was hit
  {
    if ((CAN_PASS_MAGIC_WALL(element) && 
	 (smashed == EL_MAGIC_WALL ||
	  smashed == EL_BD_MAGIC_WALL)) ||
	(CAN_PASS_DC_MAGIC_WALL(element) &&
	 smashed == EL_DC_MAGIC_WALL))
    {
      int xx, yy;
      int activated_magic_wall =
	(smashed == EL_MAGIC_WALL ? EL_MAGIC_WALL_ACTIVE :
	 smashed == EL_BD_MAGIC_WALL ? EL_BD_MAGIC_WALL_ACTIVE :
	 EL_DC_MAGIC_WALL_ACTIVE);

      // activate magic wall / mill
      SCAN_PLAYFIELD(xx, yy)
      {
	if (Tile[xx][yy] == smashed)
	  Tile[xx][yy] = activated_magic_wall;
      }

      game.magic_wall_time_left = level.time_magic_wall * FRAMES_PER_SECOND;
      game.magic_wall_active = TRUE;

      PlayLevelSound(x, y, (smashed == EL_MAGIC_WALL ?
			    SND_MAGIC_WALL_ACTIVATING :
			    smashed == EL_BD_MAGIC_WALL ?
			    SND_BD_MAGIC_WALL_ACTIVATING :
			    SND_DC_MAGIC_WALL_ACTIVATING));
    }

    if (IS_PLAYER(x, y + 1))
    {
      if (CAN_SMASH_PLAYER(element))
      {
	KillPlayerUnlessEnemyProtected(x, y + 1);
	return;
      }
    }
    else if (smashed == EL_PENGUIN)
    {
      if (CAN_SMASH_PLAYER(element))
      {
	Bang(x, y + 1);
	return;
      }
    }
    else if (element == EL_BD_DIAMOND)
    {
      if (IS_CLASSIC_ENEMY(smashed) && IS_BD_ELEMENT(smashed))
      {
	Bang(x, y + 1);
	return;
      }
    }
    else if (((element == EL_SP_INFOTRON ||
	       element == EL_SP_ZONK) &&
	      (smashed == EL_SP_SNIKSNAK ||
	       smashed == EL_SP_ELECTRON ||
	       smashed == EL_SP_DISK_ORANGE)) ||
	     (element == EL_SP_INFOTRON &&
	      smashed == EL_SP_DISK_YELLOW))
    {
      Bang(x, y + 1);
      return;
    }
    else if (CAN_SMASH_EVERYTHING(element))
    {
      if (IS_CLASSIC_ENEMY(smashed) ||
	  CAN_EXPLODE_SMASHED(smashed))
      {
	Bang(x, y + 1);
	return;
      }
      else if (!IS_MOVING(x, y + 1) && !IS_BLOCKED(x, y + 1))
      {
	if (smashed == EL_LAMP ||
	    smashed == EL_LAMP_ACTIVE)
	{
	  Bang(x, y + 1);
	  return;
	}
	else if (smashed == EL_NUT)
	{
	  Tile[x][y + 1] = EL_NUT_BREAKING;
	  PlayLevelSound(x, y, SND_NUT_BREAKING);
	  RaiseScoreElement(EL_NUT);
	  return;
	}
	else if (smashed == EL_PEARL)
	{
	  ResetGfxAnimation(x, y);

	  Tile[x][y + 1] = EL_PEARL_BREAKING;
	  PlayLevelSound(x, y, SND_PEARL_BREAKING);
	  return;
	}
	else if (smashed == EL_DIAMOND)
	{
	  Tile[x][y + 1] = EL_DIAMOND_BREAKING;
	  PlayLevelSound(x, y, SND_DIAMOND_BREAKING);
	  return;
	}
	else if (IS_BELT_SWITCH(smashed))
	{
	  ToggleBeltSwitch(x, y + 1);
	}
	else if (smashed == EL_SWITCHGATE_SWITCH_UP ||
		 smashed == EL_SWITCHGATE_SWITCH_DOWN ||
		 smashed == EL_DC_SWITCHGATE_SWITCH_UP ||
		 smashed == EL_DC_SWITCHGATE_SWITCH_DOWN)
	{
	  ToggleSwitchgateSwitch();
	}
	else if (smashed == EL_LIGHT_SWITCH ||
		 smashed == EL_LIGHT_SWITCH_ACTIVE)
	{
	  ToggleLightSwitch(x, y + 1);
	}
	else
	{
	  CheckElementChange(x, y + 1, smashed, element, CE_SMASHED);

	  CheckElementChangeBySide(x, y + 1, smashed, element,
				   CE_SWITCHED, CH_SIDE_TOP);
	  CheckTriggeredElementChangeBySide(x, y + 1, smashed, CE_SWITCH_OF_X,
					    CH_SIDE_TOP);
	}
      }
      else
      {
	CheckElementChange(x, y + 1, smashed, element, CE_SMASHED);
      }
    }
  }

  // play sound of magic wall / mill
  if (!last_line &&
      (Tile[x][y + 1] == EL_MAGIC_WALL_ACTIVE ||
       Tile[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE ||
       Tile[x][y + 1] == EL_DC_MAGIC_WALL_ACTIVE))
  {
    if (Tile[x][y + 1] == EL_MAGIC_WALL_ACTIVE)
      PlayLevelSound(x, y, SND_MAGIC_WALL_FILLING);
    else if (Tile[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE)
      PlayLevelSound(x, y, SND_BD_MAGIC_WALL_FILLING);
    else if (Tile[x][y + 1] == EL_DC_MAGIC_WALL_ACTIVE)
      PlayLevelSound(x, y, SND_DC_MAGIC_WALL_FILLING);

    return;
  }

  // play sound of object that hits the ground
  if (last_line || object_hit)
    PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
}

static void TurnRoundExt(int x, int y)
{
  static struct
  {
    int dx, dy;
  } move_xy[] =
  {
    {  0,  0 },
    { -1,  0 },
    { +1,  0 },
    {  0,  0 },
    {  0, -1 },
    {  0,  0 }, { 0, 0 }, { 0, 0 },
    {  0, +1 }
  };
  static struct
  {
    int left, right, back;
  } turn[] =
  {
    { 0,	0,		0	 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT	 },
    { 0,	0,		0	 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN	 },
    { 0,	0,		0	 },
    { 0,	0,		0	 },
    { 0,	0,		0	 },
    { MV_RIGHT,	MV_LEFT,	MV_UP	 }
  };

  int element = Tile[x][y];
  int move_pattern = element_info[element].move_pattern;

  int old_move_dir = MovDir[x][y];
  int left_dir  = turn[old_move_dir].left;
  int right_dir = turn[old_move_dir].right;
  int back_dir  = turn[old_move_dir].back;

  int left_dx  = move_xy[left_dir].dx,     left_dy  = move_xy[left_dir].dy;
  int right_dx = move_xy[right_dir].dx,    right_dy = move_xy[right_dir].dy;
  int move_dx  = move_xy[old_move_dir].dx, move_dy  = move_xy[old_move_dir].dy;
  int back_dx  = move_xy[back_dir].dx,     back_dy  = move_xy[back_dir].dy;

  int left_x  = x + left_dx,  left_y  = y + left_dy;
  int right_x = x + right_dx, right_y = y + right_dy;
  int move_x  = x + move_dx,  move_y  = y + move_dy;

  int xx, yy;

  if (element == EL_BUG || element == EL_BD_BUTTERFLY)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ENEMY_CAN_ENTER_FIELD(element, right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = left_dir;

    if (element == EL_BUG && MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_BUTTERFLY)     // && MovDir[x][y] == left_dir)
      MovDelay[x][y] = 1;
  }
  else if (element == EL_SPACESHIP || element == EL_BD_FIREFLY)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ENEMY_CAN_ENTER_FIELD(element, left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!ENEMY_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = right_dir;

    if (element == EL_SPACESHIP	&& MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
    else if (element == EL_BD_FIREFLY)	    // && MovDir[x][y] == right_dir)
      MovDelay[x][y] = 1;
  }
  else if (element == EL_SP_SNIKSNAK || element == EL_SP_ELECTRON)
  {
    TestIfBadThingTouchesOtherBadThing(x, y);

    if (ELEMENT_CAN_ENTER_FIELD_BASE_4(element, left_x, left_y, 0))
      MovDir[x][y] = left_dir;
    else if (!ELEMENT_CAN_ENTER_FIELD_BASE_4(element, move_x, move_y, 0))
      MovDir[x][y] = right_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
  }
  else if (element == EL_YAMYAM)
  {
    boolean can_turn_left  = YAMYAM_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = YAMYAM_CAN_ENTER_FIELD(element, right_x, right_y);

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16 + 16 * RND(3);
  }
  else if (element == EL_DARK_YAMYAM)
  {
    boolean can_turn_left  = DARK_YAMYAM_CAN_ENTER_FIELD(element,
							 left_x, left_y);
    boolean can_turn_right = DARK_YAMYAM_CAN_ENTER_FIELD(element,
							 right_x, right_y);

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 16 + 16 * RND(3);
  }
  else if (element == EL_PACMAN)
  {
    boolean can_turn_left  = PACMAN_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = PACMAN_CAN_ENTER_FIELD(element, right_x, right_y);

    if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 6 + RND(40);
  }
  else if (element == EL_PIG)
  {
    boolean can_turn_left  = PIG_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = PIG_CAN_ENTER_FIELD(element, right_x, right_y);
    boolean can_move_on    = PIG_CAN_ENTER_FIELD(element, move_x, move_y);
    boolean should_turn_left, should_turn_right, should_move_on;
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    should_turn_left = (can_turn_left &&
			(!can_move_on ||
			 IN_LEV_FIELD_AND_NOT_FREE(x + back_dx + left_dx,
						   y + back_dy + left_dy)));
    should_turn_right = (can_turn_right &&
			 (!can_move_on ||
			  IN_LEV_FIELD_AND_NOT_FREE(x + back_dx + right_dx,
						    y + back_dy + right_dy)));
    should_move_on = (can_move_on &&
		      (!can_turn_left ||
		       !can_turn_right ||
		       IN_LEV_FIELD_AND_NOT_FREE(x + move_dx + left_dx,
						 y + move_dy + left_dy) ||
		       IN_LEV_FIELD_AND_NOT_FREE(x + move_dx + right_dx,
						 y + move_dy + right_dy)));

    if (should_turn_left || should_turn_right || should_move_on)
    {
      if (should_turn_left && should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value / 3     ? left_dir :
			rnd < 2 * rnd_value / 3 ? right_dir :
			old_move_dir);
      else if (should_turn_left && should_turn_right)
	MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : right_dir);
      else if (should_turn_left && should_move_on)
	MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : old_move_dir);
      else if (should_turn_right && should_move_on)
	MovDir[x][y] = (rnd < rnd_value / 2 ? right_dir : old_move_dir);
      else if (should_turn_left)
	MovDir[x][y] = left_dir;
      else if (should_turn_right)
	MovDir[x][y] = right_dir;
      else if (should_move_on)
	MovDir[x][y] = old_move_dir;
    }
    else if (can_move_on && rnd > rnd_value / 8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value / 8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value/8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    xx = x + move_xy[MovDir[x][y]].dx;
    yy = y + move_xy[MovDir[x][y]].dy;

    if (!IN_LEV_FIELD(xx, yy) ||
        (!IS_FREE(xx, yy) && !IS_FOOD_PIG(Tile[xx][yy])))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_DRAGON)
  {
    boolean can_turn_left  = DRAGON_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right = DRAGON_CAN_ENTER_FIELD(element, right_x, right_y);
    boolean can_move_on    = DRAGON_CAN_ENTER_FIELD(element, move_x, move_y);
    int rnd_value = 24;
    int rnd = RND(rnd_value);

    if (can_move_on && rnd > rnd_value / 8)
      MovDir[x][y] = old_move_dir;
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (rnd < rnd_value / 2 ? left_dir : right_dir);
    else if (can_turn_left && rnd > rnd_value / 8)
      MovDir[x][y] = left_dir;
    else if (can_turn_right && rnd > rnd_value / 8)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    xx = x + move_xy[MovDir[x][y]].dx;
    yy = y + move_xy[MovDir[x][y]].dy;

    if (!IN_LEV_FIELD_AND_IS_FREE(xx, yy))
      MovDir[x][y] = old_move_dir;

    MovDelay[x][y] = 0;
  }
  else if (element == EL_MOLE)
  {
    boolean can_move_on =
      (MOLE_CAN_ENTER_FIELD(element, move_x, move_y,
			    IS_AMOEBOID(Tile[move_x][move_y]) ||
			    Tile[move_x][move_y] == EL_AMOEBA_SHRINKING));
    if (!can_move_on)
    {
      boolean can_turn_left =
	(MOLE_CAN_ENTER_FIELD(element, left_x, left_y,
			      IS_AMOEBOID(Tile[left_x][left_y])));

      boolean can_turn_right =
	(MOLE_CAN_ENTER_FIELD(element, right_x, right_y,
			      IS_AMOEBOID(Tile[right_x][right_y])));

      if (can_turn_left && can_turn_right)
	MovDir[x][y] = (RND(2) ? left_dir : right_dir);
      else if (can_turn_left)
	MovDir[x][y] = left_dir;
      else
	MovDir[x][y] = right_dir;
    }

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = 9;
  }
  else if (element == EL_BALLOON)
  {
    MovDir[x][y] = game.wind_direction;
    MovDelay[x][y] = 0;
  }
  else if (element == EL_SPRING)
  {
    if (MovDir[x][y] & MV_HORIZONTAL)
    {
      if (SPRING_CAN_BUMP_FROM_FIELD(move_x, move_y) &&
	  !SPRING_CAN_ENTER_FIELD(element, x, y + 1))
      {
	Tile[move_x][move_y] = EL_EMC_SPRING_BUMPER_ACTIVE;
	ResetGfxAnimation(move_x, move_y);
	TEST_DrawLevelField(move_x, move_y);

	MovDir[x][y] = back_dir;
      }
      else if (!SPRING_CAN_ENTER_FIELD(element, move_x, move_y) ||
	       SPRING_CAN_ENTER_FIELD(element, x, y + 1))
	MovDir[x][y] = MV_NONE;
    }

    MovDelay[x][y] = 0;
  }
  else if (element == EL_ROBOT ||
	   element == EL_SATELLITE ||
	   element == EL_PENGUIN ||
	   element == EL_EMC_ANDROID)
  {
    int attr_x = -1, attr_y = -1;

    if (game.all_players_gone)
    {
      attr_x = game.exit_x;
      attr_y = game.exit_y;
    }
    else
    {
      int i;

      for (i = 0; i < MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	if (!player->active)
	  continue;

	if (attr_x == -1 ||
	    ABS(jx - x) + ABS(jy - y) < ABS(attr_x - x) + ABS(attr_y - y))
	{
	  attr_x = jx;
	  attr_y = jy;
	}
      }
    }

    if (element == EL_ROBOT &&
	game.robot_wheel_x >= 0 &&
	game.robot_wheel_y >= 0 &&
	(Tile[game.robot_wheel_x][game.robot_wheel_y] == EL_ROBOT_WHEEL_ACTIVE ||
	 game.engine_version < VERSION_IDENT(3,1,0,0)))
    {
      attr_x = game.robot_wheel_x;
      attr_y = game.robot_wheel_y;
    }

    if (element == EL_PENGUIN)
    {
      int i;
      struct XY *xy = xy_topdown;

      for (i = 0; i < NUM_DIRECTIONS; i++)
      {
	int ex = x + xy[i].x;
	int ey = y + xy[i].y;

    	if (IN_LEV_FIELD(ex, ey) && (Tile[ex][ey] == EL_EXIT_OPEN ||
				     Tile[ex][ey] == EL_EM_EXIT_OPEN ||
				     Tile[ex][ey] == EL_STEEL_EXIT_OPEN ||
				     Tile[ex][ey] == EL_EM_STEEL_EXIT_OPEN))
	{
	  attr_x = ex;
 	  attr_y = ey;
	  break;
	}
      }
    }

    MovDir[x][y] = MV_NONE;
    if (attr_x < x)
      MovDir[x][y] |= (game.all_players_gone ? MV_RIGHT : MV_LEFT);
    else if (attr_x > x)
      MovDir[x][y] |= (game.all_players_gone ? MV_LEFT : MV_RIGHT);
    if (attr_y < y)
      MovDir[x][y] |= (game.all_players_gone ? MV_DOWN : MV_UP);
    else if (attr_y > y)
      MovDir[x][y] |= (game.all_players_gone ? MV_UP : MV_DOWN);

    if (element == EL_ROBOT)
    {
      int newx, newy;

      if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
	MovDir[x][y] &= (RND(2) ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (IN_LEV_FIELD(newx, newy) && IS_FREE_OR_PLAYER(newx, newy))
	MovDelay[x][y] = 8 + 8 * !RND(3);
      else
	MovDelay[x][y] = 16;
    }
    else if (element == EL_PENGUIN)
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (PENGUIN_CAN_ENTER_FIELD(element, newx, newy))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (PENGUIN_CAN_ENTER_FIELD(element, newx, newy))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
    else if (element == EL_SATELLITE)
    {
      int newx, newy;

      MovDelay[x][y] = 1;

      if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
      {
	boolean first_horiz = RND(2);
	int new_move_dir = MovDir[x][y];

	MovDir[x][y] =
	  new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (SATELLITE_CAN_ENTER_FIELD(newx, newy))
	  return;

	MovDir[x][y] =
	  new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
	Moving2Blocked(x, y, &newx, &newy);

	if (SATELLITE_CAN_ENTER_FIELD(newx, newy))
	  return;

	MovDir[x][y] = old_move_dir;
	return;
      }
    }
    else if (element == EL_EMC_ANDROID)
    {
      static int check_pos[16] =
      {
	-1,		//  0 => (invalid)
	7,		//  1 => MV_LEFT
	3,		//  2 => MV_RIGHT
	-1,		//  3 => (invalid)
	1,		//  4 =>            MV_UP
	0,		//  5 => MV_LEFT  | MV_UP
	2,		//  6 => MV_RIGHT | MV_UP
	-1,		//  7 => (invalid)
	5,		//  8 =>            MV_DOWN
	6,		//  9 => MV_LEFT  | MV_DOWN
	4,		// 10 => MV_RIGHT | MV_DOWN
	-1,		// 11 => (invalid)
	-1,		// 12 => (invalid)
	-1,		// 13 => (invalid)
	-1,		// 14 => (invalid)
	-1,		// 15 => (invalid)
      };
      static struct
      {
	int dx, dy;
	int dir;
      } check_xy[8] =
      {
        { -1, -1,	MV_LEFT  | MV_UP   },
       	{  0, -1,	           MV_UP   },
	{ +1, -1,	MV_RIGHT | MV_UP   },
	{ +1,  0,	MV_RIGHT           },
	{ +1, +1,	MV_RIGHT | MV_DOWN },
	{  0, +1,	           MV_DOWN },
	{ -1, +1,	MV_LEFT  | MV_DOWN },
	{ -1,  0,	MV_LEFT            },
      };
      int start_pos, check_order;
      boolean can_clone = FALSE;
      int i;

      // check if there is any free field around current position
      for (i = 0; i < 8; i++)
      {
	int newx = x + check_xy[i].dx;
	int newy = y + check_xy[i].dy;

	if (IN_LEV_FIELD_AND_IS_FREE(newx, newy))
	{
	  can_clone = TRUE;

	  break;
	}
      }

      if (can_clone)		// randomly find an element to clone
      {
	can_clone = FALSE;

	start_pos = check_pos[RND(8)];
	check_order = (RND(2) ? -1 : +1);

	for (i = 0; i < 8; i++)
	{
	  int pos_raw = start_pos + i * check_order;
	  int pos = (pos_raw + 8) % 8;
	  int newx = x + check_xy[pos].dx;
	  int newy = y + check_xy[pos].dy;

	  if (ANDROID_CAN_CLONE_FIELD(newx, newy))
	  {
	    element_info[element].move_leave_type = LEAVE_TYPE_LIMITED;
	    element_info[element].move_leave_element = EL_TRIGGER_ELEMENT;

	    Store[x][y] = Tile[newx][newy];

	    can_clone = TRUE;

	    break;
	  }
	}
      }

      if (can_clone)		// randomly find a direction to move
      {
	can_clone = FALSE;

	start_pos = check_pos[RND(8)];
	check_order = (RND(2) ? -1 : +1);

	for (i = 0; i < 8; i++)
	{
	  int pos_raw = start_pos + i * check_order;
	  int pos = (pos_raw + 8) % 8;
	  int newx = x + check_xy[pos].dx;
	  int newy = y + check_xy[pos].dy;
	  int new_move_dir = check_xy[pos].dir;

	  if (IN_LEV_FIELD_AND_IS_FREE(newx, newy))
	  {
	    MovDir[x][y] = new_move_dir;
	    MovDelay[x][y] = level.android_clone_time * 8 + 1;

	    can_clone = TRUE;

	    break;
	  }
	}
      }

      if (can_clone)		// cloning and moving successful
	return;

      // cannot clone -- try to move towards player

      start_pos = check_pos[MovDir[x][y] & 0x0f];
      check_order = (RND(2) ? -1 : +1);

      for (i = 0; i < 3; i++)
      {
	// first check start_pos, then previous/next or (next/previous) pos
	int pos_raw = start_pos + (i < 2 ? i : -1) * check_order;
	int pos = (pos_raw + 8) % 8;
	int newx = x + check_xy[pos].dx;
	int newy = y + check_xy[pos].dy;
	int new_move_dir = check_xy[pos].dir;

	if (IS_PLAYER(newx, newy))
	  break;

	if (ANDROID_CAN_ENTER_FIELD(element, newx, newy))
	{
	  MovDir[x][y] = new_move_dir;
	  MovDelay[x][y] = level.android_move_time * 8 + 1;

	  break;
	}
      }
    }
  }
  else if (move_pattern == MV_TURNING_LEFT ||
	   move_pattern == MV_TURNING_RIGHT ||
	   move_pattern == MV_TURNING_LEFT_RIGHT ||
	   move_pattern == MV_TURNING_RIGHT_LEFT ||
	   move_pattern == MV_TURNING_RANDOM ||
	   move_pattern == MV_ALL_DIRECTIONS)
  {
    boolean can_turn_left =
      CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, left_x, left_y);
    boolean can_turn_right =
      CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, right_x, right_y);

    if (element_info[element].move_stepsize == 0)	// "not moving"
      return;

    if (move_pattern == MV_TURNING_LEFT)
      MovDir[x][y] = left_dir;
    else if (move_pattern == MV_TURNING_RIGHT)
      MovDir[x][y] = right_dir;
    else if (move_pattern == MV_TURNING_LEFT_RIGHT)
      MovDir[x][y] = (can_turn_left || !can_turn_right ? left_dir : right_dir);
    else if (move_pattern == MV_TURNING_RIGHT_LEFT)
      MovDir[x][y] = (can_turn_right || !can_turn_left ? right_dir : left_dir);
    else if (move_pattern == MV_TURNING_RANDOM)
      MovDir[x][y] = (can_turn_left && !can_turn_right ? left_dir :
		      can_turn_right && !can_turn_left ? right_dir :
		      RND(2) ? left_dir : right_dir);
    else if (can_turn_left && can_turn_right)
      MovDir[x][y] = (RND(3) ? (RND(2) ? left_dir : right_dir) : back_dir);
    else if (can_turn_left)
      MovDir[x][y] = (RND(2) ? left_dir : back_dir);
    else if (can_turn_right)
      MovDir[x][y] = (RND(2) ? right_dir : back_dir);
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_HORIZONTAL ||
	   move_pattern == MV_VERTICAL)
  {
    if (move_pattern & old_move_dir)
      MovDir[x][y] = back_dir;
    else if (move_pattern == MV_HORIZONTAL)
      MovDir[x][y] = (RND(2) ? MV_LEFT : MV_RIGHT);
    else if (move_pattern == MV_VERTICAL)
      MovDir[x][y] = (RND(2) ? MV_UP : MV_DOWN);

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern & MV_ANY_DIRECTION)
  {
    MovDir[x][y] = move_pattern;
    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern & MV_WIND_DIRECTION)
  {
    MovDir[x][y] = game.wind_direction;
    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_ALONG_LEFT_SIDE)
  {
    if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, left_x, left_y))
      MovDir[x][y] = left_dir;
    else if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = right_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_ALONG_RIGHT_SIDE)
  {
    if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, right_x, right_y))
      MovDir[x][y] = right_dir;
    else if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = left_dir;

    if (MovDir[x][y] != old_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
  else if (move_pattern == MV_TOWARDS_PLAYER ||
	   move_pattern == MV_AWAY_FROM_PLAYER)
  {
    int attr_x = -1, attr_y = -1;
    int newx, newy;
    boolean move_away = (move_pattern == MV_AWAY_FROM_PLAYER);

    if (game.all_players_gone)
    {
      attr_x = game.exit_x;
      attr_y = game.exit_y;
    }
    else
    {
      int i;

      for (i = 0; i < MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];
	int jx = player->jx, jy = player->jy;

	if (!player->active)
	  continue;

	if (attr_x == -1 ||
	    ABS(jx - x) + ABS(jy - y) < ABS(attr_x - x) + ABS(attr_y - y))
	{
	  attr_x = jx;
	  attr_y = jy;
	}
      }
    }

    MovDir[x][y] = MV_NONE;
    if (attr_x < x)
      MovDir[x][y] |= (move_away ? MV_RIGHT : MV_LEFT);
    else if (attr_x > x)
      MovDir[x][y] |= (move_away ? MV_LEFT : MV_RIGHT);
    if (attr_y < y)
      MovDir[x][y] |= (move_away ? MV_DOWN : MV_UP);
    else if (attr_y > y)
      MovDir[x][y] |= (move_away ? MV_UP : MV_DOWN);

    MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);

    if (MovDir[x][y] & MV_HORIZONTAL && MovDir[x][y] & MV_VERTICAL)
    {
      boolean first_horiz = RND(2);
      int new_move_dir = MovDir[x][y];

      if (element_info[element].move_stepsize == 0)	// "not moving"
      {
	first_horiz = (ABS(attr_x - x) >= ABS(attr_y - y));
	MovDir[x][y] &= (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);

	return;
      }

      MovDir[x][y] =
	new_move_dir & (first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, newx, newy))
	return;

      MovDir[x][y] =
	new_move_dir & (!first_horiz ? MV_HORIZONTAL : MV_VERTICAL);
      Moving2Blocked(x, y, &newx, &newy);

      if (CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, newx, newy))
	return;

      MovDir[x][y] = old_move_dir;
    }
  }
  else if (move_pattern == MV_WHEN_PUSHED ||
	   move_pattern == MV_WHEN_DROPPED)
  {
    if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, move_x, move_y))
      MovDir[x][y] = MV_NONE;

    MovDelay[x][y] = 0;
  }
  else if (move_pattern & MV_MAZE_RUNNER_STYLE)
  {
    struct XY *test_xy = xy_topdown;
    static int test_dir[4] =
    {
      MV_UP,
      MV_LEFT,
      MV_RIGHT,
      MV_DOWN
    };
    boolean hunter_mode = (move_pattern == MV_MAZE_HUNTER);
    int move_preference = -1000000;	// start with very low preference
    int new_move_dir = MV_NONE;
    int start_test = RND(4);
    int i;

    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      int j = (start_test + i) % 4;
      int move_dir = test_dir[j];
      int move_dir_preference;

      xx = x + test_xy[j].x;
      yy = y + test_xy[j].y;

      if (hunter_mode && IN_LEV_FIELD(xx, yy) &&
	  (IS_PLAYER(xx, yy) || Tile[xx][yy] == EL_PLAYER_IS_LEAVING))
      {
	new_move_dir = move_dir;

	break;
      }

      if (!CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, xx, yy))
	continue;

      move_dir_preference = -1 * RunnerVisit[xx][yy];
      if (hunter_mode && PlayerVisit[xx][yy] > 0)
	move_dir_preference = PlayerVisit[xx][yy];

      if (move_dir_preference > move_preference)
      {
	// prefer field that has not been visited for the longest time
	move_preference = move_dir_preference;
	new_move_dir = move_dir;
      }
      else if (move_dir_preference == move_preference &&
	       move_dir == old_move_dir)
      {
	// prefer last direction when all directions are preferred equally
	move_preference = move_dir_preference;
	new_move_dir = move_dir;
      }
    }

    MovDir[x][y] = new_move_dir;
    if (old_move_dir != new_move_dir)
      MovDelay[x][y] = GET_NEW_MOVE_DELAY(element);
  }
}

static void TurnRound(int x, int y)
{
  int direction = MovDir[x][y];

  TurnRoundExt(x, y);

  GfxDir[x][y] = MovDir[x][y];

  if (direction != MovDir[x][y])
    GfxFrame[x][y] = 0;

  if (MovDelay[x][y])
    GfxAction[x][y] = ACTION_TURNING_FROM_LEFT + MV_DIR_TO_BIT(direction);

  ResetGfxFrame(x, y);
}

static boolean JustBeingPushed(int x, int y)
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->active && player->is_pushing && player->MovPos)
    {
      int next_jx = player->jx + (player->jx - player->last_jx);
      int next_jy = player->jy + (player->jy - player->last_jy);

      if (x == next_jx && y == next_jy)
	return TRUE;
    }
  }

  return FALSE;
}

static void StartMoving(int x, int y)
{
  boolean started_moving = FALSE;	// some elements can fall _and_ move
  int element = Tile[x][y];

  if (Stop[x][y])
    return;

  if (MovDelay[x][y] == 0)
    GfxAction[x][y] = ACTION_DEFAULT;

  if (CAN_FALL(element) && y < lev_fieldy - 1)
  {
    if ((x > 0              && IS_PLAYER(x - 1, y)) ||
	(x < lev_fieldx - 1 && IS_PLAYER(x + 1, y)))
      if (JustBeingPushed(x, y))
	return;

    if (element == EL_QUICKSAND_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Tile[x][y] = EL_QUICKSAND_EMPTYING;
#if USE_QUICKSAND_BD_ROCK_BUGFIX
	if (Store[x][y] != EL_ROCK && Store[x][y] != EL_BD_ROCK)
	  Store[x][y] = EL_ROCK;
#else
	Store[x][y] = EL_ROCK;
#endif

	PlayLevelSoundAction(x, y, ACTION_EMPTYING);
      }
      else if (Tile[x][y + 1] == EL_QUICKSAND_EMPTY)
      {
	if (!MovDelay[x][y])
	{
	  MovDelay[x][y] = TILEY + 1;

	  ResetGfxAnimation(x, y);
	  ResetGfxAnimation(x, y + 1);
	}

	if (MovDelay[x][y])
	{
	  DrawLevelElement(x, y, EL_QUICKSAND_EMPTYING);
	  DrawLevelElement(x, y + 1, EL_QUICKSAND_FILLING);

	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_QUICKSAND_EMPTY;
	Tile[x][y + 1] = EL_QUICKSAND_FULL;
	Store[x][y + 1] = Store[x][y];
	Store[x][y] = 0;

	PlayLevelSoundAction(x, y, ACTION_FILLING);
      }
      else if (Tile[x][y + 1] == EL_QUICKSAND_FAST_EMPTY)
      {
	if (!MovDelay[x][y])
	{
	  MovDelay[x][y] = TILEY + 1;

	  ResetGfxAnimation(x, y);
	  ResetGfxAnimation(x, y + 1);
	}

	if (MovDelay[x][y])
	{
	  DrawLevelElement(x, y, EL_QUICKSAND_EMPTYING);
	  DrawLevelElement(x, y + 1, EL_QUICKSAND_FAST_FILLING);

	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_QUICKSAND_EMPTY;
	Tile[x][y + 1] = EL_QUICKSAND_FAST_FULL;
	Store[x][y + 1] = Store[x][y];
	Store[x][y] = 0;

	PlayLevelSoundAction(x, y, ACTION_FILLING);
      }
    }
    else if (element == EL_QUICKSAND_FAST_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Tile[x][y] = EL_QUICKSAND_FAST_EMPTYING;
#if USE_QUICKSAND_BD_ROCK_BUGFIX
	if (Store[x][y] != EL_ROCK && Store[x][y] != EL_BD_ROCK)
	  Store[x][y] = EL_ROCK;
#else
	Store[x][y] = EL_ROCK;
#endif

	PlayLevelSoundAction(x, y, ACTION_EMPTYING);
      }
      else if (Tile[x][y + 1] == EL_QUICKSAND_FAST_EMPTY)
      {
	if (!MovDelay[x][y])
	{
	  MovDelay[x][y] = TILEY + 1;

	  ResetGfxAnimation(x, y);
	  ResetGfxAnimation(x, y + 1);
	}

	if (MovDelay[x][y])
	{
	  DrawLevelElement(x, y, EL_QUICKSAND_FAST_EMPTYING);
	  DrawLevelElement(x, y + 1, EL_QUICKSAND_FAST_FILLING);

	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_QUICKSAND_FAST_EMPTY;
	Tile[x][y + 1] = EL_QUICKSAND_FAST_FULL;
	Store[x][y + 1] = Store[x][y];
	Store[x][y] = 0;

	PlayLevelSoundAction(x, y, ACTION_FILLING);
      }
      else if (Tile[x][y + 1] == EL_QUICKSAND_EMPTY)
      {
	if (!MovDelay[x][y])
	{
	  MovDelay[x][y] = TILEY + 1;

	  ResetGfxAnimation(x, y);
	  ResetGfxAnimation(x, y + 1);
	}

	if (MovDelay[x][y])
	{
	  DrawLevelElement(x, y, EL_QUICKSAND_FAST_EMPTYING);
	  DrawLevelElement(x, y + 1, EL_QUICKSAND_FILLING);

	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_QUICKSAND_FAST_EMPTY;
	Tile[x][y + 1] = EL_QUICKSAND_FULL;
	Store[x][y + 1] = Store[x][y];
	Store[x][y] = 0;

	PlayLevelSoundAction(x, y, ACTION_FILLING);
      }
    }
    else if ((element == EL_ROCK || element == EL_BD_ROCK) &&
	     Tile[x][y + 1] == EL_QUICKSAND_EMPTY)
    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Tile[x][y] = EL_QUICKSAND_FILLING;
      Store[x][y] = element;

      PlayLevelSoundAction(x, y, ACTION_FILLING);
    }
    else if ((element == EL_ROCK || element == EL_BD_ROCK) &&
	     Tile[x][y + 1] == EL_QUICKSAND_FAST_EMPTY)
    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Tile[x][y] = EL_QUICKSAND_FAST_FILLING;
      Store[x][y] = element;

      PlayLevelSoundAction(x, y, ACTION_FILLING);
    }
    else if (element == EL_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Tile[x][y] = EL_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED(Store[x][y]);
      }
      else if (Tile[x][y + 1] == EL_MAGIC_WALL_ACTIVE)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY / 4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_MAGIC_WALL_ACTIVE;
	Tile[x][y + 1] = EL_MAGIC_WALL_FULL;
	Store[x][y + 1] = EL_CHANGED(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if (element == EL_BD_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Tile[x][y] = EL_BD_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED_BD(Store[x][y]);
      }
      else if (Tile[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY / 4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_BD_MAGIC_WALL_ACTIVE;
	Tile[x][y + 1] = EL_BD_MAGIC_WALL_FULL;
	Store[x][y + 1] = EL_CHANGED_BD(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if (element == EL_DC_MAGIC_WALL_FULL)
    {
      if (IS_FREE(x, y + 1))
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;

	Tile[x][y] = EL_DC_MAGIC_WALL_EMPTYING;
	Store[x][y] = EL_CHANGED_DC(Store[x][y]);
      }
      else if (Tile[x][y + 1] == EL_DC_MAGIC_WALL_ACTIVE)
      {
	if (!MovDelay[x][y])
	  MovDelay[x][y] = TILEY / 4 + 1;

	if (MovDelay[x][y])
	{
	  MovDelay[x][y]--;
	  if (MovDelay[x][y])
	    return;
	}

	Tile[x][y] = EL_DC_MAGIC_WALL_ACTIVE;
	Tile[x][y + 1] = EL_DC_MAGIC_WALL_FULL;
	Store[x][y + 1] = EL_CHANGED_DC(Store[x][y]);
	Store[x][y] = 0;
      }
    }
    else if ((CAN_PASS_MAGIC_WALL(element) &&
	      (Tile[x][y + 1] == EL_MAGIC_WALL_ACTIVE ||
	       Tile[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE)) ||
	     (CAN_PASS_DC_MAGIC_WALL(element) &&
	      (Tile[x][y + 1] == EL_DC_MAGIC_WALL_ACTIVE)))

    {
      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Tile[x][y] =
	(Tile[x][y + 1] == EL_MAGIC_WALL_ACTIVE ? EL_MAGIC_WALL_FILLING :
	 Tile[x][y + 1] == EL_BD_MAGIC_WALL_ACTIVE ? EL_BD_MAGIC_WALL_FILLING :
	 EL_DC_MAGIC_WALL_FILLING);
      Store[x][y] = element;
    }
    else if (CAN_FALL(element) && Tile[x][y + 1] == EL_ACID)
    {
      SplashAcid(x, y + 1);

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;

      Store[x][y] = EL_ACID;
    }
    else if (
	     (game.engine_version >= VERSION_IDENT(3,1,0,0) &&
	      CheckImpact[x][y] && !IS_FREE(x, y + 1)) ||
	     (game.engine_version >= VERSION_IDENT(3,0,7,0) &&
	      CAN_FALL(element) && WasJustFalling[x][y] &&
	      (Tile[x][y + 1] == EL_BLOCKED || IS_PLAYER(x, y + 1))) ||

	     (game.engine_version < VERSION_IDENT(2,2,0,7) &&
	      CAN_FALL(element) && WasJustMoving[x][y] && !Pushed[x][y + 1] &&
	      (Tile[x][y + 1] == EL_BLOCKED)))
    {
      /* this is needed for a special case not covered by calling "Impact()"
	 from "ContinueMoving()": if an element moves to a tile directly below
	 another element which was just falling on that tile (which was empty
	 in the previous frame), the falling element above would just stop
	 instead of smashing the element below (in previous version, the above
	 element was just checked for "moving" instead of "falling", resulting
	 in incorrect smashes caused by horizontal movement of the above
	 element; also, the case of the player being the element to smash was
	 simply not covered here... :-/ ) */

      CheckCollision[x][y] = 0;
      CheckImpact[x][y] = 0;

      Impact(x, y);
    }
    else if (IS_FREE(x, y + 1) && element == EL_SPRING && level.use_spring_bug)
    {
      if (MovDir[x][y] == MV_NONE)
      {
	InitMovingField(x, y, MV_DOWN);
	started_moving = TRUE;
      }
    }
    else if (IS_FREE(x, y + 1) || Tile[x][y + 1] == EL_DIAMOND_BREAKING)
    {
      if (WasJustFalling[x][y])	// prevent animation from being restarted
	MovDir[x][y] = MV_DOWN;

      InitMovingField(x, y, MV_DOWN);
      started_moving = TRUE;
    }
    else if (element == EL_AMOEBA_DROP)
    {
      Tile[x][y] = EL_AMOEBA_GROWING;
      Store[x][y] = EL_AMOEBA_WET;
    }
    else if (((IS_SLIPPERY(Tile[x][y + 1]) && !IS_PLAYER(x, y + 1)) ||
	      (IS_EM_SLIPPERY_WALL(Tile[x][y + 1]) && IS_GEM(element))) &&
	     !IS_FALLING(x, y + 1) && !WasJustMoving[x][y + 1] &&
	     element != EL_DX_SUPABOMB && element != EL_SP_DISK_ORANGE)
    {
      boolean can_fall_left  = (x > 0 && IS_FREE(x - 1, y) &&
				(IS_FREE(x - 1, y + 1) ||
				 Tile[x - 1][y + 1] == EL_ACID));
      boolean can_fall_right = (x < lev_fieldx - 1 && IS_FREE(x + 1, y) &&
				(IS_FREE(x + 1, y + 1) ||
				 Tile[x + 1][y + 1] == EL_ACID));
      boolean can_fall_any  = (can_fall_left || can_fall_right);
      boolean can_fall_both = (can_fall_left && can_fall_right);
      int slippery_type = element_info[Tile[x][y + 1]].slippery_type;

      if (can_fall_any && slippery_type != SLIPPERY_ANY_RANDOM)
      {
	if (slippery_type == SLIPPERY_ANY_LEFT_RIGHT && can_fall_both)
	  can_fall_right = FALSE;
	else if (slippery_type == SLIPPERY_ANY_RIGHT_LEFT && can_fall_both)
	  can_fall_left = FALSE;
	else if (slippery_type == SLIPPERY_ONLY_LEFT)
	  can_fall_right = FALSE;
	else if (slippery_type == SLIPPERY_ONLY_RIGHT)
	  can_fall_left = FALSE;

	can_fall_any  = (can_fall_left || can_fall_right);
	can_fall_both = FALSE;
      }

      if (can_fall_both)
      {
	if (element == EL_BD_ROCK || element == EL_BD_DIAMOND)
	  can_fall_right = FALSE;	// slip down on left side
	else
	  can_fall_left = !(can_fall_right = RND(2));

	can_fall_both = FALSE;
      }

      if (can_fall_any)
      {
	// if not determined otherwise, prefer left side for slipping down
	InitMovingField(x, y, can_fall_left ? MV_LEFT : MV_RIGHT);
	started_moving = TRUE;
      }
    }
    else if (IS_BELT_ACTIVE(Tile[x][y + 1]))
    {
      boolean left_is_free  = (x > 0 && IS_FREE(x - 1, y));
      boolean right_is_free = (x < lev_fieldx - 1 && IS_FREE(x + 1, y));
      int belt_nr = getBeltNrFromBeltActiveElement(Tile[x][y + 1]);
      int belt_dir = game.belt_dir[belt_nr];

      if ((belt_dir == MV_LEFT  && left_is_free) ||
	  (belt_dir == MV_RIGHT && right_is_free))
      {
	int nextx = (belt_dir == MV_LEFT ? x - 1 : x + 1);

	InitMovingField(x, y, belt_dir);
	started_moving = TRUE;

	Pushed[x][y] = TRUE;
	Pushed[nextx][y] = TRUE;

	GfxAction[x][y] = ACTION_DEFAULT;
      }
      else
      {
	MovDir[x][y] = 0;	// if element was moving, stop it
      }
    }
  }

  // not "else if" because of elements that can fall and move (EL_SPRING)
  if (CAN_MOVE(element) && !started_moving)
  {
    int move_pattern = element_info[element].move_pattern;
    int newx, newy;

    Moving2Blocked(x, y, &newx, &newy);

    if (IS_PUSHABLE(element) && JustBeingPushed(x, y))
      return;

    if (game.engine_version >= VERSION_IDENT(3,1,0,0) &&
	CheckCollision[x][y] && !IN_LEV_FIELD_AND_IS_FREE(newx, newy))
    {
      WasJustMoving[x][y] = 0;
      CheckCollision[x][y] = 0;

      TestIfElementHitsCustomElement(x, y, MovDir[x][y]);

      if (Tile[x][y] != element)	// element has changed
	return;
    }

    if (!MovDelay[x][y])	// start new movement phase
    {
      // all objects that can change their move direction after each step
      // (YAMYAM, DARK_YAMYAM and PACMAN go straight until they hit a wall

      if (element != EL_YAMYAM &&
	  element != EL_DARK_YAMYAM &&
	  element != EL_PACMAN &&
	  !(move_pattern & MV_ANY_DIRECTION) &&
	  move_pattern != MV_TURNING_LEFT &&
	  move_pattern != MV_TURNING_RIGHT &&
	  move_pattern != MV_TURNING_LEFT_RIGHT &&
	  move_pattern != MV_TURNING_RIGHT_LEFT &&
	  move_pattern != MV_TURNING_RANDOM)
      {
	TurnRound(x, y);

	if (MovDelay[x][y] && (element == EL_BUG ||
			       element == EL_SPACESHIP ||
			       element == EL_SP_SNIKSNAK ||
			       element == EL_SP_ELECTRON ||
			       element == EL_MOLE))
	  TEST_DrawLevelField(x, y);
      }
    }

    if (MovDelay[x][y])		// wait some time before next movement
    {
      MovDelay[x][y]--;

      if (element == EL_ROBOT ||
	  element == EL_YAMYAM ||
	  element == EL_DARK_YAMYAM)
      {
	DrawLevelElementAnimationIfNeeded(x, y, element);
	PlayLevelSoundAction(x, y, ACTION_WAITING);
      }
      else if (element == EL_SP_ELECTRON)
	DrawLevelElementAnimationIfNeeded(x, y, element);
      else if (element == EL_DRAGON)
      {
	int i;
	int dir = MovDir[x][y];
	int dx = (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
	int dy = (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);
	int graphic = (dir == MV_LEFT	? IMG_FLAMES_1_LEFT :
		       dir == MV_RIGHT	? IMG_FLAMES_1_RIGHT :
		       dir == MV_UP	? IMG_FLAMES_1_UP :
		       dir == MV_DOWN	? IMG_FLAMES_1_DOWN : IMG_EMPTY);
	int frame = getGraphicAnimationFrameXY(graphic, x, y);

	GfxAction[x][y] = ACTION_ATTACKING;

	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  TEST_DrawLevelField(x, y);

	PlayLevelSoundActionIfLoop(x, y, ACTION_ATTACKING);

	for (i = 1; i <= 3; i++)
	{
	  int xx = x + i * dx;
	  int yy = y + i * dy;
	  int sx = SCREENX(xx);
	  int sy = SCREENY(yy);
	  int flame_graphic = graphic + (i - 1);

	  if (!IN_LEV_FIELD(xx, yy) || IS_DRAGONFIRE_PROOF(Tile[xx][yy]))
	    break;

	  if (MovDelay[x][y])
	  {
	    int flamed = MovingOrBlocked2Element(xx, yy);

	    if (IS_CLASSIC_ENEMY(flamed) || CAN_EXPLODE_BY_DRAGONFIRE(flamed))
	      Bang(xx, yy);
	    else
	      RemoveMovingField(xx, yy);

	    ChangeDelay[xx][yy] = 0;

	    Tile[xx][yy] = EL_FLAMES;

	    if (IN_SCR_FIELD(sx, sy))
	    {
	      TEST_DrawLevelFieldCrumbled(xx, yy);
	      DrawScreenGraphic(sx, sy, flame_graphic, frame);
	    }
	  }
	  else
	  {
	    if (Tile[xx][yy] == EL_FLAMES)
	      Tile[xx][yy] = EL_EMPTY;
	    TEST_DrawLevelField(xx, yy);
	  }
	}
      }

      if (MovDelay[x][y])	// element still has to wait some time
      {
	PlayLevelSoundAction(x, y, ACTION_WAITING);

	return;
      }
    }

    // now make next step

    Moving2Blocked(x, y, &newx, &newy);	// get next screen position

    if (DONT_COLLIDE_WITH(element) &&
	IN_LEV_FIELD(newx, newy) && IS_PLAYER(newx, newy) &&
	!PLAYER_ENEMY_PROTECTED(newx, newy))
    {
      TestIfBadThingRunsIntoPlayer(x, y, MovDir[x][y]);

      return;
    }

    else if (CAN_MOVE_INTO_ACID(element) &&
	     IN_LEV_FIELD(newx, newy) && Tile[newx][newy] == EL_ACID &&
	     !IS_MV_DIAGONAL(MovDir[x][y]) &&
	     (MovDir[x][y] == MV_DOWN ||
	      game.engine_version >= VERSION_IDENT(3,1,0,0)))
    {
      SplashAcid(newx, newy);
      Store[x][y] = EL_ACID;
    }
    else if (element == EL_PENGUIN && IN_LEV_FIELD(newx, newy))
    {
      if (Tile[newx][newy] == EL_EXIT_OPEN ||
	  Tile[newx][newy] == EL_EM_EXIT_OPEN ||
	  Tile[newx][newy] == EL_STEEL_EXIT_OPEN ||
	  Tile[newx][newy] == EL_EM_STEEL_EXIT_OPEN)
      {
	RemoveField(x, y);
	TEST_DrawLevelField(x, y);

	PlayLevelSound(newx, newy, SND_PENGUIN_PASSING);
	if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
	  DrawGraphicThruMask(SCREENX(newx), SCREENY(newy), el2img(element), 0);

	game.friends_still_needed--;
	if (!game.friends_still_needed &&
	    !game.GameOver &&
	    game.all_players_gone)
	  LevelSolved();

	return;
      }
      else if (IS_FOOD_PENGUIN(Tile[newx][newy]))
      {
	if (DigField(local_player, x, y, newx, newy, 0, 0, DF_DIG) == MP_MOVING)
	  TEST_DrawLevelField(newx, newy);
	else
	  GfxDir[x][y] = MovDir[x][y] = MV_NONE;
      }
      else if (!IS_FREE(newx, newy))
      {
	GfxAction[x][y] = ACTION_WAITING;

	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  TEST_DrawLevelField(x, y);

	return;
      }
    }
    else if (element == EL_PIG && IN_LEV_FIELD(newx, newy))
    {
      if (IS_FOOD_PIG(Tile[newx][newy]))
      {
	if (IS_MOVING(newx, newy))
	  RemoveMovingField(newx, newy);
	else
	{
	  Tile[newx][newy] = EL_EMPTY;
	  TEST_DrawLevelField(newx, newy);
	}

	PlayLevelSound(x, y, SND_PIG_DIGGING);
      }
      else if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  TEST_DrawLevelField(x, y);

	return;
      }
    }
    else if (element == EL_EMC_ANDROID && IN_LEV_FIELD(newx, newy))
    {
      if (Store[x][y] != EL_EMPTY)
      {
	boolean can_clone = FALSE;
	int xx, yy;

	// check if element to clone is still there
	for (yy = y - 1; yy <= y + 1; yy++) for (xx = x - 1; xx <= x + 1; xx++)
	{
	  if (IN_LEV_FIELD(xx, yy) && Tile[xx][yy] == Store[x][y])
	  {
	    can_clone = TRUE;

	    break;
	  }
	}

	// cannot clone or target field not free anymore -- do not clone
	if (!can_clone || !ANDROID_CAN_ENTER_FIELD(element, newx, newy))
	  Store[x][y] = EL_EMPTY;
      }

      if (ANDROID_CAN_ENTER_FIELD(element, newx, newy))
      {
	if (IS_MV_DIAGONAL(MovDir[x][y]))
	{
	  int diagonal_move_dir = MovDir[x][y];
	  int stored = Store[x][y];
	  int change_delay = 8;
	  int graphic;

	  // android is moving diagonally

	  CreateField(x, y, EL_DIAGONAL_SHRINKING);

	  Store[x][y] = (stored == EL_ACID ? EL_EMPTY : stored);
	  GfxElement[x][y] = EL_EMC_ANDROID;
	  GfxAction[x][y] = ACTION_SHRINKING;
	  GfxDir[x][y] = diagonal_move_dir;
	  ChangeDelay[x][y] = change_delay;

	  if (Store[x][y] == EL_EMPTY)
	    Store[x][y] = GfxElementEmpty[x][y];

	  graphic = el_act_dir2img(GfxElement[x][y], GfxAction[x][y],
				   GfxDir[x][y]);

	  DrawLevelGraphicAnimation(x, y, graphic);
	  PlayLevelSoundAction(x, y, ACTION_SHRINKING);

	  if (Tile[newx][newy] == EL_ACID)
	  {
	    SplashAcid(newx, newy);

	    return;
	  }

	  CreateField(newx, newy, EL_DIAGONAL_GROWING);

	  Store[newx][newy] = EL_EMC_ANDROID;
	  GfxElement[newx][newy] = EL_EMC_ANDROID;
	  GfxAction[newx][newy] = ACTION_GROWING;
	  GfxDir[newx][newy] = diagonal_move_dir;
	  ChangeDelay[newx][newy] = change_delay;

	  graphic = el_act_dir2img(GfxElement[newx][newy],
				   GfxAction[newx][newy], GfxDir[newx][newy]);

	  DrawLevelGraphicAnimation(newx, newy, graphic);
	  PlayLevelSoundAction(newx, newy, ACTION_GROWING);

	  return;
	}
	else
	{
	  Tile[newx][newy] = EL_EMPTY;
	  TEST_DrawLevelField(newx, newy);

	  PlayLevelSoundAction(x, y, ACTION_DIGGING);
	}
      }
      else if (!IS_FREE(newx, newy))
      {
	return;
      }
    }
    else if (IS_CUSTOM_ELEMENT(element) &&
	     CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, newx, newy))
    {
      if (!DigFieldByCE(newx, newy, element))
	return;

      if (move_pattern & MV_MAZE_RUNNER_STYLE)
      {
	RunnerVisit[x][y] = FrameCounter;
	PlayerVisit[x][y] /= 8;		// expire player visit path
      }
    }
    else if (element == EL_DRAGON && IN_LEV_FIELD(newx, newy))
    {
      if (!IS_FREE(newx, newy))
      {
	if (IS_PLAYER(x, y))
	  DrawPlayerField(x, y);
	else
	  TEST_DrawLevelField(x, y);

	return;
      }
      else
      {
	boolean wanna_flame = !RND(10);
	int dx = newx - x, dy = newy - y;
	int newx1 = newx + 1 * dx, newy1 = newy + 1 * dy;
	int newx2 = newx + 2 * dx, newy2 = newy + 2 * dy;
	int element1 = (IN_LEV_FIELD(newx1, newy1) ?
			MovingOrBlocked2Element(newx1, newy1) : EL_STEELWALL);
	int element2 = (IN_LEV_FIELD(newx2, newy2) ?
			MovingOrBlocked2Element(newx2, newy2) : EL_STEELWALL);

	if ((wanna_flame ||
	     IS_CLASSIC_ENEMY(element1) ||
	     IS_CLASSIC_ENEMY(element2)) &&
	    element1 != EL_DRAGON && element2 != EL_DRAGON &&
	    element1 != EL_FLAMES && element2 != EL_FLAMES)
	{
	  ResetGfxAnimation(x, y);
	  GfxAction[x][y] = ACTION_ATTACKING;

	  if (IS_PLAYER(x, y))
	    DrawPlayerField(x, y);
	  else
	    TEST_DrawLevelField(x, y);

	  PlayLevelSound(x, y, SND_DRAGON_ATTACKING);

	  MovDelay[x][y] = 50;

	  Tile[newx][newy] = EL_FLAMES;
	  if (IN_LEV_FIELD(newx1, newy1) && Tile[newx1][newy1] == EL_EMPTY)
	    Tile[newx1][newy1] = EL_FLAMES;
	  if (IN_LEV_FIELD(newx2, newy2) && Tile[newx2][newy2] == EL_EMPTY)
	    Tile[newx2][newy2] = EL_FLAMES;

	  return;
	}
      }
    }
    else if (element == EL_YAMYAM && IN_LEV_FIELD(newx, newy) &&
	     Tile[newx][newy] == EL_DIAMOND)
    {
      if (IS_MOVING(newx, newy))
	RemoveMovingField(newx, newy);
      else
      {
	Tile[newx][newy] = EL_EMPTY;
	TEST_DrawLevelField(newx, newy);
      }

      PlayLevelSound(x, y, SND_YAMYAM_DIGGING);
    }
    else if (element == EL_DARK_YAMYAM && IN_LEV_FIELD(newx, newy) &&
	     IS_FOOD_DARK_YAMYAM(Tile[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Tile[newx][newy] == EL_AMOEBA_FULL ||
	    Tile[newx][newy] == EL_BD_AMOEBA)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (IS_MOVING(newx, newy))
      {
	RemoveMovingField(newx, newy);
      }
      else
      {
	Tile[newx][newy] = EL_EMPTY;
	TEST_DrawLevelField(newx, newy);
      }

      PlayLevelSound(x, y, SND_DARK_YAMYAM_DIGGING);
    }
    else if ((element == EL_PACMAN || element == EL_MOLE)
	     && IN_LEV_FIELD(newx, newy) && IS_AMOEBOID(Tile[newx][newy]))
    {
      if (AmoebaNr[newx][newy])
      {
	AmoebaCnt2[AmoebaNr[newx][newy]]--;
	if (Tile[newx][newy] == EL_AMOEBA_FULL ||
	    Tile[newx][newy] == EL_BD_AMOEBA)
	  AmoebaCnt[AmoebaNr[newx][newy]]--;
      }

      if (element == EL_MOLE)
      {
	Tile[newx][newy] = EL_AMOEBA_SHRINKING;
	PlayLevelSound(x, y, SND_MOLE_DIGGING);

	ResetGfxAnimation(x, y);
	GfxAction[x][y] = ACTION_DIGGING;
	TEST_DrawLevelField(x, y);

	MovDelay[newx][newy] = 0;	// start amoeba shrinking delay

	return;				// wait for shrinking amoeba
      }
      else	// element == EL_PACMAN
      {
	Tile[newx][newy] = EL_EMPTY;
	TEST_DrawLevelField(newx, newy);
	PlayLevelSound(x, y, SND_PACMAN_DIGGING);
      }
    }
    else if (element == EL_MOLE && IN_LEV_FIELD(newx, newy) &&
	     (Tile[newx][newy] == EL_AMOEBA_SHRINKING ||
	      (Tile[newx][newy] == EL_EMPTY && Stop[newx][newy])))
    {
      // wait for shrinking amoeba to completely disappear
      return;
    }
    else if (!IN_LEV_FIELD(newx, newy) || !IS_FREE(newx, newy))
    {
      // object was running against a wall

      TurnRound(x, y);

      if (GFX_ELEMENT(element) != EL_SAND)     // !!! FIX THIS (crumble) !!!
	DrawLevelElementAnimation(x, y, element);

      if (DONT_TOUCH(element))
	TestIfBadThingTouchesPlayer(x, y);

      return;
    }

    InitMovingField(x, y, MovDir[x][y]);

    PlayLevelSoundAction(x, y, ACTION_MOVING);
  }

  if (MovDir[x][y])
    ContinueMoving(x, y);
}

void ContinueMoving(int x, int y)
{
  int element = Tile[x][y];
  struct ElementInfo *ei = &element_info[element];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int newx = x + dx, newy = y + dy;
  int stored = Store[x][y];
  int stored_new = Store[newx][newy];
  boolean pushed_by_player   = (Pushed[x][y] && IS_PLAYER(x, y));
  boolean pushed_by_conveyor = (Pushed[x][y] && !IS_PLAYER(x, y));
  boolean last_line = (newy == lev_fieldy - 1);
  boolean use_step_delay = (GET_MAX_STEP_DELAY(element) != 0);

  if (pushed_by_player)		// special case: moving object pushed by player
  {
    MovPos[x][y] = SIGN(MovPos[x][y]) * (TILEX - ABS(PLAYERINFO(x, y)->MovPos));
  }
  else if (use_step_delay)	// special case: moving object has step delay
  {
    if (!MovDelay[x][y])
      MovPos[x][y] += getElementMoveStepsize(x, y);

    if (MovDelay[x][y])
      MovDelay[x][y]--;
    else
      MovDelay[x][y] = GET_NEW_STEP_DELAY(element);

    if (MovDelay[x][y])
    {
      TEST_DrawLevelField(x, y);

      return;	// element is still waiting
    }
  }
  else				// normal case: generically moving object
  {
    MovPos[x][y] += getElementMoveStepsize(x, y);
  }

  if (ABS(MovPos[x][y]) < TILEX)
  {
    TEST_DrawLevelField(x, y);

    return;	// element is still moving
  }

  // element reached destination field

  Tile[x][y] = EL_EMPTY;
  Tile[newx][newy] = element;
  MovPos[x][y] = 0;	// force "not moving" for "crumbled sand"

  if (Store[x][y] == EL_ACID)	// element is moving into acid pool
  {
    element = Tile[newx][newy] = EL_ACID;
  }
  else if (element == EL_MOLE)
  {
    Tile[x][y] = EL_SAND;

    TEST_DrawLevelFieldCrumbledNeighbours(x, y);
  }
  else if (element == EL_QUICKSAND_FILLING)
  {
    element = Tile[newx][newy] = get_next_element(element);
    Store[newx][newy] = Store[x][y];
  }
  else if (element == EL_QUICKSAND_EMPTYING)
  {
    Tile[x][y] = get_next_element(element);
    element = Tile[newx][newy] = Store[x][y];
  }
  else if (element == EL_QUICKSAND_FAST_FILLING)
  {
    element = Tile[newx][newy] = get_next_element(element);
    Store[newx][newy] = Store[x][y];
  }
  else if (element == EL_QUICKSAND_FAST_EMPTYING)
  {
    Tile[x][y] = get_next_element(element);
    element = Tile[newx][newy] = Store[x][y];
  }
  else if (element == EL_MAGIC_WALL_FILLING)
  {
    element = Tile[newx][newy] = get_next_element(element);
    if (!game.magic_wall_active)
      element = Tile[newx][newy] = EL_MAGIC_WALL_DEAD;
    Store[newx][newy] = Store[x][y];
  }
  else if (element == EL_MAGIC_WALL_EMPTYING)
  {
    Tile[x][y] = get_next_element(element);
    if (!game.magic_wall_active)
      Tile[x][y] = EL_MAGIC_WALL_DEAD;
    element = Tile[newx][newy] = Store[x][y];

    InitField(newx, newy, FALSE);
  }
  else if (element == EL_BD_MAGIC_WALL_FILLING)
  {
    element = Tile[newx][newy] = get_next_element(element);
    if (!game.magic_wall_active)
      element = Tile[newx][newy] = EL_BD_MAGIC_WALL_DEAD;
    Store[newx][newy] = Store[x][y];
  }
  else if (element == EL_BD_MAGIC_WALL_EMPTYING)
  {
    Tile[x][y] = get_next_element(element);
    if (!game.magic_wall_active)
      Tile[x][y] = EL_BD_MAGIC_WALL_DEAD;
    element = Tile[newx][newy] = Store[x][y];

    InitField(newx, newy, FALSE);
  }
  else if (element == EL_DC_MAGIC_WALL_FILLING)
  {
    element = Tile[newx][newy] = get_next_element(element);
    if (!game.magic_wall_active)
      element = Tile[newx][newy] = EL_DC_MAGIC_WALL_DEAD;
    Store[newx][newy] = Store[x][y];
  }
  else if (element == EL_DC_MAGIC_WALL_EMPTYING)
  {
    Tile[x][y] = get_next_element(element);
    if (!game.magic_wall_active)
      Tile[x][y] = EL_DC_MAGIC_WALL_DEAD;
    element = Tile[newx][newy] = Store[x][y];

    InitField(newx, newy, FALSE);
  }
  else if (element == EL_AMOEBA_DROPPING)
  {
    Tile[x][y] = get_next_element(element);
    element = Tile[newx][newy] = Store[x][y];
  }
  else if (element == EL_SOKOBAN_OBJECT)
  {
    if (Back[x][y])
      Tile[x][y] = Back[x][y];

    if (Back[newx][newy])
      Tile[newx][newy] = EL_SOKOBAN_FIELD_FULL;

    Back[x][y] = Back[newx][newy] = 0;
  }

  Store[x][y] = EL_EMPTY;
  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;

  MovDelay[newx][newy] = 0;

  if (CAN_CHANGE_OR_HAS_ACTION(element))
  {
    // copy element change control values to new field
    ChangeDelay[newx][newy] = ChangeDelay[x][y];
    ChangePage[newx][newy]  = ChangePage[x][y];
    ChangeCount[newx][newy] = ChangeCount[x][y];
    ChangeEvent[newx][newy] = ChangeEvent[x][y];
  }

  CustomValue[newx][newy] = CustomValue[x][y];

  ChangeDelay[x][y] = 0;
  ChangePage[x][y] = -1;
  ChangeCount[x][y] = 0;
  ChangeEvent[x][y] = -1;

  CustomValue[x][y] = 0;

  // copy animation control values to new field
  GfxFrame[newx][newy]  = GfxFrame[x][y];
  GfxRandom[newx][newy] = GfxRandom[x][y];	// keep same random value
  GfxAction[newx][newy] = GfxAction[x][y];	// keep action one frame
  GfxDir[newx][newy]    = GfxDir[x][y];		// keep element direction

  Pushed[x][y] = Pushed[newx][newy] = FALSE;

  // some elements can leave other elements behind after moving
  if (ei->move_leave_element != EL_EMPTY &&
      (ei->move_leave_type == LEAVE_TYPE_UNLIMITED || stored != EL_EMPTY) &&
      (!IS_PLAYER(x, y) || IS_WALKABLE(ei->move_leave_element)))
  {
    int move_leave_element = ei->move_leave_element;

    // this makes it possible to leave the removed element again
    if (ei->move_leave_element == EL_TRIGGER_ELEMENT)
      move_leave_element = (stored == EL_ACID ? EL_EMPTY : stored);

    Tile[x][y] = move_leave_element;

    if (element_info[Tile[x][y]].move_direction_initial == MV_START_PREVIOUS)
      MovDir[x][y] = direction;

    InitField(x, y, FALSE);

    if (GFX_CRUMBLED(Tile[x][y]))
      TEST_DrawLevelFieldCrumbledNeighbours(x, y);

    if (IS_PLAYER_ELEMENT(move_leave_element))
      RelocatePlayer(x, y, move_leave_element);
  }

  // do this after checking for left-behind element
  ResetGfxAnimation(x, y);	// reset animation values for old field

  if (!CAN_MOVE(element) ||
      (CAN_FALL(element) && direction == MV_DOWN &&
       (element == EL_SPRING ||
	element_info[element].move_pattern == MV_WHEN_PUSHED ||
	element_info[element].move_pattern == MV_WHEN_DROPPED)))
    GfxDir[x][y] = MovDir[newx][newy] = 0;

  TEST_DrawLevelField(x, y);
  TEST_DrawLevelField(newx, newy);

  Stop[newx][newy] = TRUE;	// ignore this element until the next frame

  // prevent pushed element from moving on in pushed direction
  if (pushed_by_player && CAN_MOVE(element) &&
      element_info[element].move_pattern & MV_ANY_DIRECTION &&
      !(element_info[element].move_pattern & direction))
    TurnRound(newx, newy);

  // prevent elements on conveyor belt from moving on in last direction
  if (pushed_by_conveyor && CAN_FALL(element) &&
      direction & MV_HORIZONTAL)
    MovDir[newx][newy] = 0;

  if (!pushed_by_player)
  {
    int nextx = newx + dx, nexty = newy + dy;
    boolean check_collision_again = IN_LEV_FIELD_AND_IS_FREE(nextx, nexty);

    WasJustMoving[newx][newy] = CHECK_DELAY_MOVING;

    if (CAN_FALL(element) && direction == MV_DOWN)
      WasJustFalling[newx][newy] = CHECK_DELAY_FALLING;

    if ((!CAN_FALL(element) || direction == MV_DOWN) && check_collision_again)
      CheckCollision[newx][newy] = CHECK_DELAY_COLLISION;

    if (CAN_FALL(element) && direction == MV_DOWN && check_collision_again)
      CheckImpact[newx][newy] = CHECK_DELAY_IMPACT;
  }

  if (DONT_TOUCH(element))	// object may be nasty to player or others
  {
    TestIfBadThingTouchesPlayer(newx, newy);
    TestIfBadThingTouchesFriend(newx, newy);

    if (!IS_CUSTOM_ELEMENT(element))
      TestIfBadThingTouchesOtherBadThing(newx, newy);
  }
  else if (element == EL_PENGUIN)
    TestIfFriendTouchesBadThing(newx, newy);

  if (DONT_GET_HIT_BY(element))
  {
    TestIfGoodThingGetsHitByBadThing(newx, newy, direction);
  }

  // give the player one last chance (one more frame) to move away
  if (CAN_FALL(element) && direction == MV_DOWN &&
      (last_line || (!IS_FREE(x, newy + 1) &&
		     (!IS_PLAYER(x, newy + 1) ||
		      game.engine_version < VERSION_IDENT(3,1,1,0)))))
    Impact(x, newy);

  if (pushed_by_player && !game.use_change_when_pushing_bug)
  {
    int push_side = MV_DIR_OPPOSITE(direction);
    struct PlayerInfo *player = PLAYERINFO(x, y);

    CheckElementChangeByPlayer(newx, newy, element, CE_PUSHED_BY_PLAYER,
			       player->index_bit, push_side);
    CheckTriggeredElementChangeByPlayer(newx, newy, element, CE_PLAYER_PUSHES_X,
					player->index_bit, push_side);
  }

  if (element == EL_EMC_ANDROID && pushed_by_player)	// make another move
    MovDelay[newx][newy] = 1;

  CheckTriggeredElementChangeBySide(x, y, element, CE_MOVE_OF_X, direction);

  TestIfElementTouchesCustomElement(x, y);	// empty or new element
  TestIfElementHitsCustomElement(newx, newy, direction);
  TestIfPlayerTouchesCustomElement(newx, newy);
  TestIfElementTouchesCustomElement(newx, newy);

  if (IS_CUSTOM_ELEMENT(element) && ei->move_enter_element != EL_EMPTY &&
      IS_EQUAL_OR_IN_GROUP(stored_new, ei->move_enter_element))
    CheckElementChangeBySide(newx, newy, element, stored_new, CE_DIGGING_X,
			     MV_DIR_OPPOSITE(direction));
}

int AmoebaNeighbourNr(int ax, int ay)
{
  int i;
  int element = Tile[ax][ay];
  int group_nr = 0;
  struct XY *xy = xy_topdown;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int x = ax + xy[i].x;
    int y = ay + xy[i].y;

    if (!IN_LEV_FIELD(x, y))
      continue;

    if (Tile[x][y] == element && AmoebaNr[x][y] > 0)
      group_nr = AmoebaNr[x][y];
  }

  return group_nr;
}

static void AmoebaMerge(int ax, int ay)
{
  int i, x, y, xx, yy;
  int new_group_nr = AmoebaNr[ax][ay];
  struct XY *xy = xy_topdown;

  if (new_group_nr == 0)
    return;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    x = ax + xy[i].x;
    y = ay + xy[i].y;

    if (!IN_LEV_FIELD(x, y))
      continue;

    if ((Tile[x][y] == EL_AMOEBA_FULL ||
	 Tile[x][y] == EL_BD_AMOEBA ||
	 Tile[x][y] == EL_AMOEBA_DEAD) &&
	AmoebaNr[x][y] != new_group_nr)
    {
      int old_group_nr = AmoebaNr[x][y];

      if (old_group_nr == 0)
	return;

      AmoebaCnt[new_group_nr] += AmoebaCnt[old_group_nr];
      AmoebaCnt[old_group_nr] = 0;
      AmoebaCnt2[new_group_nr] += AmoebaCnt2[old_group_nr];
      AmoebaCnt2[old_group_nr] = 0;

      SCAN_PLAYFIELD(xx, yy)
      {
	if (AmoebaNr[xx][yy] == old_group_nr)
	  AmoebaNr[xx][yy] = new_group_nr;
      }
    }
  }
}

void AmoebaToDiamond(int ax, int ay)
{
  int i, x, y;

  if (Tile[ax][ay] == EL_AMOEBA_DEAD)
  {
    int group_nr = AmoebaNr[ax][ay];

#ifdef DEBUG
    if (group_nr == 0)
    {
      Debug("game:playing:AmoebaToDiamond", "ax = %d, ay = %d", ax, ay);
      Debug("game:playing:AmoebaToDiamond", "This should never happen!");

      return;
    }
#endif

    SCAN_PLAYFIELD(x, y)
    {
      if (Tile[x][y] == EL_AMOEBA_DEAD && AmoebaNr[x][y] == group_nr)
      {
	AmoebaNr[x][y] = 0;
	Tile[x][y] = EL_AMOEBA_TO_DIAMOND;
      }
    }

    PlayLevelSound(ax, ay, (IS_GEM(level.amoeba_content) ?
			    SND_AMOEBA_TURNING_TO_GEM :
			    SND_AMOEBA_TURNING_TO_ROCK));
    Bang(ax, ay);
  }
  else
  {
    struct XY *xy = xy_topdown;

    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      x = ax + xy[i].x;
      y = ay + xy[i].y;

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (Tile[x][y] == EL_AMOEBA_TO_DIAMOND)
      {
	PlayLevelSound(x, y, (IS_GEM(level.amoeba_content) ?
			      SND_AMOEBA_TURNING_TO_GEM :
			      SND_AMOEBA_TURNING_TO_ROCK));
	Bang(x, y);
      }
    }
  }
}

static void AmoebaToDiamondBD(int ax, int ay, int new_element)
{
  int x, y;
  int group_nr = AmoebaNr[ax][ay];
  boolean done = FALSE;

#ifdef DEBUG
  if (group_nr == 0)
  {
    Debug("game:playing:AmoebaToDiamondBD", "ax = %d, ay = %d", ax, ay);
    Debug("game:playing:AmoebaToDiamondBD", "This should never happen!");

    return;
  }
#endif

  SCAN_PLAYFIELD(x, y)
  {
    if (AmoebaNr[x][y] == group_nr &&
	(Tile[x][y] == EL_AMOEBA_DEAD ||
	 Tile[x][y] == EL_BD_AMOEBA ||
	 Tile[x][y] == EL_AMOEBA_GROWING))
    {
      AmoebaNr[x][y] = 0;
      Tile[x][y] = new_element;
      InitField(x, y, FALSE);
      TEST_DrawLevelField(x, y);
      done = TRUE;
    }
  }

  if (done)
    PlayLevelSound(ax, ay, (new_element == EL_BD_ROCK ?
			    SND_BD_AMOEBA_TURNING_TO_ROCK :
			    SND_BD_AMOEBA_TURNING_TO_GEM));
}

static void AmoebaGrowing(int x, int y)
{
  static DelayCounter sound_delay = { 0 };

  if (!MovDelay[x][y])		// start new growing cycle
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay))
    {
      PlayLevelSoundElementAction(x, y, Store[x][y], ACTION_GROWING);
      sound_delay.value = 30;
    }
  }

  if (MovDelay[x][y])		// wait some time before growing bigger
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int frame = getGraphicAnimationFrame(IMG_AMOEBA_GROWING,
					   6 - MovDelay[x][y]);

      DrawLevelGraphic(x, y, IMG_AMOEBA_GROWING, frame);
    }

    if (!MovDelay[x][y])
    {
      Tile[x][y] = Store[x][y];
      Store[x][y] = 0;
      TEST_DrawLevelField(x, y);
    }
  }
}

static void AmoebaShrinking(int x, int y)
{
  static DelayCounter sound_delay = { 0 };

  if (!MovDelay[x][y])		// start new shrinking cycle
  {
    MovDelay[x][y] = 7;

    if (DelayReached(&sound_delay))
      sound_delay.value = 30;
  }

  if (MovDelay[x][y])		// wait some time before shrinking
  {
    MovDelay[x][y]--;
    if (MovDelay[x][y]/2 && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int frame = getGraphicAnimationFrame(IMG_AMOEBA_SHRINKING,
					   6 - MovDelay[x][y]);

      DrawLevelGraphic(x, y, IMG_AMOEBA_SHRINKING, frame);
    }

    if (!MovDelay[x][y])
    {
      Tile[x][y] = EL_EMPTY;
      TEST_DrawLevelField(x, y);

      // don't let mole enter this field in this cycle;
      // (give priority to objects falling to this field from above)
      Stop[x][y] = TRUE;
    }
  }
}

static void AmoebaReproduce(int ax, int ay)
{
  int i;
  int element = Tile[ax][ay];
  int graphic = el2img(element);
  int newax = ax, neway = ay;
  boolean can_drop = (element == EL_AMOEBA_WET || element == EL_EMC_DRIPPER);
  struct XY *xy = xy_topdown;

  if (!level.amoeba_speed && element != EL_EMC_DRIPPER)
  {
    Tile[ax][ay] = EL_AMOEBA_DEAD;
    TEST_DrawLevelField(ax, ay);
    return;
  }

  if (IS_ANIMATED(graphic))
    DrawLevelGraphicAnimationIfNeeded(ax, ay, graphic);

  if (!MovDelay[ax][ay])	// start making new amoeba field
    MovDelay[ax][ay] = RND(FRAMES_PER_SECOND * 25 / (1 + level.amoeba_speed));

  if (MovDelay[ax][ay])		// wait some time before making new amoeba
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (can_drop)			// EL_AMOEBA_WET or EL_EMC_DRIPPER
  {
    int start = RND(4);
    int x = ax + xy[start].x;
    int y = ay + xy[start].y;

    if (!IN_LEV_FIELD(x, y))
      return;

    if (IS_FREE(x, y) ||
	CAN_GROW_INTO(Tile[x][y]) ||
	Tile[x][y] == EL_QUICKSAND_EMPTY ||
	Tile[x][y] == EL_QUICKSAND_FAST_EMPTY)
    {
      newax = x;
      neway = y;
    }

    if (newax == ax && neway == ay)
      return;
  }
  else				// normal or "filled" (BD style) amoeba
  {
    int start = RND(4);
    boolean waiting_for_player = FALSE;

    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      int j = (start + i) % 4;
      int x = ax + xy[j].x;
      int y = ay + xy[j].y;

      if (!IN_LEV_FIELD(x, y))
	continue;

      if (IS_FREE(x, y) ||
	  CAN_GROW_INTO(Tile[x][y]) ||
	  Tile[x][y] == EL_QUICKSAND_EMPTY ||
	  Tile[x][y] == EL_QUICKSAND_FAST_EMPTY)
      {
	newax = x;
	neway = y;
	break;
      }
      else if (IS_PLAYER(x, y))
	waiting_for_player = TRUE;
    }

    if (newax == ax && neway == ay)		// amoeba cannot grow
    {
      if (i == 4 && (!waiting_for_player || element == EL_BD_AMOEBA))
      {
	Tile[ax][ay] = EL_AMOEBA_DEAD;
	TEST_DrawLevelField(ax, ay);
	AmoebaCnt[AmoebaNr[ax][ay]]--;

	if (AmoebaCnt[AmoebaNr[ax][ay]] <= 0)	// amoeba is completely dead
	{
	  if (element == EL_AMOEBA_FULL)
	    AmoebaToDiamond(ax, ay);
	  else if (element == EL_BD_AMOEBA)
	    AmoebaToDiamondBD(ax, ay, level.amoeba_content);
	}
      }
      return;
    }
    else if (element == EL_AMOEBA_FULL || element == EL_BD_AMOEBA)
    {
      // amoeba gets larger by growing in some direction

      int new_group_nr = AmoebaNr[ax][ay];

#ifdef DEBUG
  if (new_group_nr == 0)
  {
    Debug("game:playing:AmoebaReproduce", "newax = %d, neway = %d",
	  newax, neway);
    Debug("game:playing:AmoebaReproduce", "This should never happen!");

    return;
  }
#endif

      AmoebaNr[newax][neway] = new_group_nr;
      AmoebaCnt[new_group_nr]++;
      AmoebaCnt2[new_group_nr]++;

      // if amoeba touches other amoeba(s) after growing, unify them
      AmoebaMerge(newax, neway);

      if (element == EL_BD_AMOEBA && AmoebaCnt2[new_group_nr] >= 200)
      {
	AmoebaToDiamondBD(newax, neway, EL_BD_ROCK);
	return;
      }
    }
  }

  if (!can_drop || neway < ay || !IS_FREE(newax, neway) ||
      (neway == lev_fieldy - 1 && newax != ax))
  {
    Tile[newax][neway] = EL_AMOEBA_GROWING;	// creation of new amoeba
    Store[newax][neway] = element;
  }
  else if (neway == ay || element == EL_EMC_DRIPPER)
  {
    Tile[newax][neway] = EL_AMOEBA_DROP;	// drop left/right of amoeba

    PlayLevelSoundAction(newax, neway, ACTION_GROWING);
  }
  else
  {
    InitMovingField(ax, ay, MV_DOWN);		// drop dripping from amoeba
    Tile[ax][ay] = EL_AMOEBA_DROPPING;
    Store[ax][ay] = EL_AMOEBA_DROP;
    ContinueMoving(ax, ay);
    return;
  }

  TEST_DrawLevelField(newax, neway);
}

static void Life(int ax, int ay)
{
  int x1, y1, x2, y2;
  int life_time = 40;
  int element = Tile[ax][ay];
  int graphic = el2img(element);
  int *life_parameter = (element == EL_GAME_OF_LIFE ? level.game_of_life :
			 level.biomaze);
  boolean changed = FALSE;

  if (IS_ANIMATED(graphic))
    DrawLevelGraphicAnimationIfNeeded(ax, ay, graphic);

  if (Stop[ax][ay])
    return;

  if (!MovDelay[ax][ay])	// start new "game of life" cycle
    MovDelay[ax][ay] = life_time;

  if (MovDelay[ax][ay])		// wait some time before next cycle
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  for (y1 = -1; y1 < 2; y1++) for (x1 = -1; x1 < 2; x1++)
  {
    int xx = ax + x1, yy = ay + y1;
    int old_element = Tile[xx][yy];
    int num_neighbours = 0;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    for (y2 = -1; y2 < 2; y2++) for (x2 = -1; x2 < 2; x2++)
    {
      int x = xx + x2, y = yy + y2;

      if (!IN_LEV_FIELD(x, y) || (x == xx && y == yy))
	continue;

      boolean is_player_cell = (element == EL_GAME_OF_LIFE && IS_PLAYER(x, y));
      boolean is_neighbour = FALSE;

      if (level.use_life_bugs)
	is_neighbour =
	  (((Tile[x][y] == element || is_player_cell) && !Stop[x][y]) ||
	   (IS_FREE(x, y)                             &&  Stop[x][y]));
      else
	is_neighbour =
	  (Last[x][y] == element || is_player_cell);

      if (is_neighbour)
	num_neighbours++;
    }

    boolean is_free = FALSE;

    if (level.use_life_bugs)
      is_free = (IS_FREE(xx, yy));
    else
      is_free = (IS_FREE(xx, yy) && Last[xx][yy] == EL_EMPTY);

    if (xx == ax && yy == ay)		// field in the middle
    {
      if (num_neighbours < life_parameter[0] ||
	  num_neighbours > life_parameter[1])
      {
	Tile[xx][yy] = EL_EMPTY;
	if (Tile[xx][yy] != old_element)
	  TEST_DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
	changed = TRUE;
      }
    }
    else if (is_free || CAN_GROW_INTO(Tile[xx][yy]))
    {					// free border field
      if (num_neighbours >= life_parameter[2] &&
	  num_neighbours <= life_parameter[3])
      {
	Tile[xx][yy] = element;
	MovDelay[xx][yy] = (element == EL_GAME_OF_LIFE ? 0 : life_time - 1);
	if (Tile[xx][yy] != old_element)
	  TEST_DrawLevelField(xx, yy);
	Stop[xx][yy] = TRUE;
	changed = TRUE;
      }
    }
  }

  if (changed)
    PlayLevelSound(ax, ay, element == EL_BIOMAZE ? SND_BIOMAZE_GROWING :
		   SND_GAME_OF_LIFE_GROWING);
}

static void InitRobotWheel(int x, int y)
{
  ChangeDelay[x][y] = level.time_wheel * FRAMES_PER_SECOND;
}

static void RunRobotWheel(int x, int y)
{
  PlayLevelSound(x, y, SND_ROBOT_WHEEL_ACTIVE);
}

static void StopRobotWheel(int x, int y)
{
  if (game.robot_wheel_x == x &&
      game.robot_wheel_y == y)
  {
    game.robot_wheel_x = -1;
    game.robot_wheel_y = -1;
    game.robot_wheel_active = FALSE;
  }
}

static void InitTimegateWheel(int x, int y)
{
  ChangeDelay[x][y] = level.time_timegate * FRAMES_PER_SECOND;
}

static void RunTimegateWheel(int x, int y)
{
  PlayLevelSound(x, y, SND_CLASS_TIMEGATE_SWITCH_ACTIVE);
}

static void InitMagicBallDelay(int x, int y)
{
  ChangeDelay[x][y] = (level.ball_time + 1) * 8 + 1;
}

static void ActivateMagicBall(int bx, int by)
{
  int x, y;

  if (level.ball_random)
  {
    int pos_border = RND(8);	// select one of the eight border elements
    int pos_content = (pos_border > 3 ? pos_border + 1 : pos_border);
    int xx = pos_content % 3;
    int yy = pos_content / 3;

    x = bx - 1 + xx;
    y = by - 1 + yy;

    if (IN_LEV_FIELD(x, y) && Tile[x][y] == EL_EMPTY)
      CreateField(x, y, level.ball_content[game.ball_content_nr].e[xx][yy]);
  }
  else
  {
    for (y = by - 1; y <= by + 1; y++) for (x = bx - 1; x <= bx + 1; x++)
    {
      int xx = x - bx + 1;
      int yy = y - by + 1;

      if (IN_LEV_FIELD(x, y) && Tile[x][y] == EL_EMPTY)
	CreateField(x, y, level.ball_content[game.ball_content_nr].e[xx][yy]);
    }
  }

  game.ball_content_nr = (game.ball_content_nr + 1) % level.num_ball_contents;
}

static void CheckExit(int x, int y)
{
  if (game.gems_still_needed > 0 ||
      game.sokoban_fields_still_needed > 0 ||
      game.sokoban_objects_still_needed > 0 ||
      game.lights_still_needed > 0)
  {
    int element = Tile[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  // do not re-open exit door closed after last player
  if (game.all_players_gone)
    return;

  Tile[x][y] = EL_EXIT_OPENING;

  PlayLevelSoundNearest(x, y, SND_CLASS_EXIT_OPENING);
}

static void CheckExitEM(int x, int y)
{
  if (game.gems_still_needed > 0 ||
      game.sokoban_fields_still_needed > 0 ||
      game.sokoban_objects_still_needed > 0 ||
      game.lights_still_needed > 0)
  {
    int element = Tile[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  // do not re-open exit door closed after last player
  if (game.all_players_gone)
    return;

  Tile[x][y] = EL_EM_EXIT_OPENING;

  PlayLevelSoundNearest(x, y, SND_CLASS_EM_EXIT_OPENING);
}

static void CheckExitSteel(int x, int y)
{
  if (game.gems_still_needed > 0 ||
      game.sokoban_fields_still_needed > 0 ||
      game.sokoban_objects_still_needed > 0 ||
      game.lights_still_needed > 0)
  {
    int element = Tile[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  // do not re-open exit door closed after last player
  if (game.all_players_gone)
    return;

  Tile[x][y] = EL_STEEL_EXIT_OPENING;

  PlayLevelSoundNearest(x, y, SND_CLASS_STEEL_EXIT_OPENING);
}

static void CheckExitSteelEM(int x, int y)
{
  if (game.gems_still_needed > 0 ||
      game.sokoban_fields_still_needed > 0 ||
      game.sokoban_objects_still_needed > 0 ||
      game.lights_still_needed > 0)
  {
    int element = Tile[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  // do not re-open exit door closed after last player
  if (game.all_players_gone)
    return;

  Tile[x][y] = EL_EM_STEEL_EXIT_OPENING;

  PlayLevelSoundNearest(x, y, SND_CLASS_EM_STEEL_EXIT_OPENING);
}

static void CheckExitSP(int x, int y)
{
  if (game.gems_still_needed > 0)
  {
    int element = Tile[x][y];
    int graphic = el2img(element);

    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    return;
  }

  // do not re-open exit door closed after last player
  if (game.all_players_gone)
    return;

  Tile[x][y] = EL_SP_EXIT_OPENING;

  PlayLevelSoundNearest(x, y, SND_CLASS_SP_EXIT_OPENING);
}

static void CloseAllOpenTimegates(void)
{
  int x, y;

  SCAN_PLAYFIELD(x, y)
  {
    int element = Tile[x][y];

    if (element == EL_TIMEGATE_OPEN || element == EL_TIMEGATE_OPENING)
    {
      Tile[x][y] = EL_TIMEGATE_CLOSING;

      PlayLevelSoundAction(x, y, ACTION_CLOSING);
    }
  }
}

static void DrawTwinkleOnField(int x, int y)
{
  if (!IN_SCR_FIELD(SCREENX(x), SCREENY(y)) || IS_MOVING(x, y))
    return;

  if (Tile[x][y] == EL_BD_DIAMOND)
    return;

  if (MovDelay[x][y] == 0)	// next animation frame
    MovDelay[x][y] = 11 * !GetSimpleRandom(500);

  if (MovDelay[x][y] != 0)	// wait some time before next frame
  {
    MovDelay[x][y]--;

    DrawLevelElementAnimation(x, y, Tile[x][y]);

    if (MovDelay[x][y] != 0)
    {
      int frame = getGraphicAnimationFrame(IMG_TWINKLE_WHITE,
					   10 - MovDelay[x][y]);

      DrawGraphicThruMask(SCREENX(x), SCREENY(y), IMG_TWINKLE_WHITE, frame);
    }
  }
}

static void WallGrowing(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		// next animation frame
    MovDelay[x][y] = 3 * delay;

  if (MovDelay[x][y])		// wait some time before next frame
  {
    MovDelay[x][y]--;

    if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    {
      int graphic = el_dir2img(Tile[x][y], GfxDir[x][y]);
      int frame = getGraphicAnimationFrame(graphic, 17 - MovDelay[x][y]);

      DrawLevelGraphic(x, y, graphic, frame);
    }

    if (!MovDelay[x][y])
    {
      if (MovDir[x][y] == MV_LEFT)
      {
	if (IN_LEV_FIELD(x - 1, y) && IS_WALL(Tile[x - 1][y]))
	  TEST_DrawLevelField(x - 1, y);
      }
      else if (MovDir[x][y] == MV_RIGHT)
      {
	if (IN_LEV_FIELD(x + 1, y) && IS_WALL(Tile[x + 1][y]))
	  TEST_DrawLevelField(x + 1, y);
      }
      else if (MovDir[x][y] == MV_UP)
      {
	if (IN_LEV_FIELD(x, y - 1) && IS_WALL(Tile[x][y - 1]))
	  TEST_DrawLevelField(x, y - 1);
      }
      else
      {
	if (IN_LEV_FIELD(x, y + 1) && IS_WALL(Tile[x][y + 1]))
	  TEST_DrawLevelField(x, y + 1);
      }

      Tile[x][y] = Store[x][y];
      Store[x][y] = 0;
      GfxDir[x][y] = MovDir[x][y] = MV_NONE;
      TEST_DrawLevelField(x, y);
    }
  }
}

static void CheckWallGrowing(int ax, int ay)
{
  int element = Tile[ax][ay];
  int graphic = el2img(element);
  boolean free_top    = FALSE;
  boolean free_bottom = FALSE;
  boolean free_left   = FALSE;
  boolean free_right  = FALSE;
  boolean stop_top    = FALSE;
  boolean stop_bottom = FALSE;
  boolean stop_left   = FALSE;
  boolean stop_right  = FALSE;
  boolean new_wall    = FALSE;

  boolean is_steelwall  = (element == EL_EXPANDABLE_STEELWALL_HORIZONTAL ||
			   element == EL_EXPANDABLE_STEELWALL_VERTICAL ||
			   element == EL_EXPANDABLE_STEELWALL_ANY);

  boolean grow_vertical   = (element == EL_EXPANDABLE_WALL_VERTICAL ||
			     element == EL_EXPANDABLE_WALL_ANY ||
			     element == EL_EXPANDABLE_STEELWALL_VERTICAL ||
			     element == EL_EXPANDABLE_STEELWALL_ANY);

  boolean grow_horizontal = (element == EL_EXPANDABLE_WALL_HORIZONTAL ||
			     element == EL_EXPANDABLE_WALL_ANY ||
			     element == EL_EXPANDABLE_WALL ||
			     element == EL_BD_EXPANDABLE_WALL ||
			     element == EL_EXPANDABLE_STEELWALL_HORIZONTAL ||
			     element == EL_EXPANDABLE_STEELWALL_ANY);

  boolean stop_vertical   = (element == EL_EXPANDABLE_WALL_VERTICAL ||
			     element == EL_EXPANDABLE_STEELWALL_VERTICAL);

  boolean stop_horizontal = (element == EL_EXPANDABLE_WALL_HORIZONTAL ||
			     element == EL_EXPANDABLE_WALL ||
			     element == EL_EXPANDABLE_STEELWALL_HORIZONTAL);

  int wall_growing = (is_steelwall ?
		      EL_EXPANDABLE_STEELWALL_GROWING :
		      EL_EXPANDABLE_WALL_GROWING);

  int gfx_wall_growing_up    = (is_steelwall ?
				IMG_EXPANDABLE_STEELWALL_GROWING_UP :
				IMG_EXPANDABLE_WALL_GROWING_UP);
  int gfx_wall_growing_down  = (is_steelwall ?
				IMG_EXPANDABLE_STEELWALL_GROWING_DOWN :
				IMG_EXPANDABLE_WALL_GROWING_DOWN);
  int gfx_wall_growing_left  = (is_steelwall ?
				IMG_EXPANDABLE_STEELWALL_GROWING_LEFT :
				IMG_EXPANDABLE_WALL_GROWING_LEFT);
  int gfx_wall_growing_right = (is_steelwall ?
				IMG_EXPANDABLE_STEELWALL_GROWING_RIGHT :
				IMG_EXPANDABLE_WALL_GROWING_RIGHT);

  if (IS_ANIMATED(graphic))
    DrawLevelGraphicAnimationIfNeeded(ax, ay, graphic);

  if (!MovDelay[ax][ay])	// start building new wall
    MovDelay[ax][ay] = 6;

  if (MovDelay[ax][ay])		// wait some time before building new wall
  {
    MovDelay[ax][ay]--;
    if (MovDelay[ax][ay])
      return;
  }

  if (IN_LEV_FIELD(ax, ay - 1) && IS_FREE(ax, ay - 1))
    free_top = TRUE;
  if (IN_LEV_FIELD(ax, ay + 1) && IS_FREE(ax, ay + 1))
    free_bottom = TRUE;
  if (IN_LEV_FIELD(ax - 1, ay) && IS_FREE(ax - 1, ay))
    free_left = TRUE;
  if (IN_LEV_FIELD(ax + 1, ay) && IS_FREE(ax + 1, ay))
    free_right = TRUE;

  if (grow_vertical)
  {
    if (free_top)
    {
      Tile[ax][ay - 1] = wall_growing;
      Store[ax][ay - 1] = element;
      GfxDir[ax][ay - 1] = MovDir[ax][ay - 1] = MV_UP;

      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay - 1)))
	DrawLevelGraphic(ax, ay - 1, gfx_wall_growing_up, 0);

      new_wall = TRUE;
    }

    if (free_bottom)
    {
      Tile[ax][ay + 1] = wall_growing;
      Store[ax][ay + 1] = element;
      GfxDir[ax][ay + 1] = MovDir[ax][ay + 1] = MV_DOWN;

      if (IN_SCR_FIELD(SCREENX(ax), SCREENY(ay + 1)))
	DrawLevelGraphic(ax, ay + 1, gfx_wall_growing_down, 0);

      new_wall = TRUE;
    }
  }

  if (grow_horizontal)
  {
    if (free_left)
    {
      Tile[ax - 1][ay] = wall_growing;
      Store[ax - 1][ay] = element;
      GfxDir[ax - 1][ay] = MovDir[ax - 1][ay] = MV_LEFT;

      if (IN_SCR_FIELD(SCREENX(ax - 1), SCREENY(ay)))
	DrawLevelGraphic(ax - 1, ay, gfx_wall_growing_left, 0);

      new_wall = TRUE;
    }

    if (free_right)
    {
      Tile[ax + 1][ay] = wall_growing;
      Store[ax + 1][ay] = element;
      GfxDir[ax + 1][ay] = MovDir[ax + 1][ay] = MV_RIGHT;

      if (IN_SCR_FIELD(SCREENX(ax + 1), SCREENY(ay)))
	DrawLevelGraphic(ax + 1, ay, gfx_wall_growing_right, 0);

      new_wall = TRUE;
    }
  }

  if (element == EL_EXPANDABLE_WALL && (free_left || free_right))
    TEST_DrawLevelField(ax, ay);

  if (!IN_LEV_FIELD(ax, ay - 1) || IS_WALL(Tile[ax][ay - 1]))
    stop_top = TRUE;
  if (!IN_LEV_FIELD(ax, ay + 1) || IS_WALL(Tile[ax][ay + 1]))
    stop_bottom = TRUE;
  if (!IN_LEV_FIELD(ax - 1, ay) || IS_WALL(Tile[ax - 1][ay]))
    stop_left = TRUE;
  if (!IN_LEV_FIELD(ax + 1, ay) || IS_WALL(Tile[ax + 1][ay]))
    stop_right = TRUE;

  if (((stop_top && stop_bottom) || stop_horizontal) &&
      ((stop_left && stop_right) || stop_vertical))
    Tile[ax][ay] = (is_steelwall ? EL_STEELWALL : EL_WALL);

  if (new_wall)
    PlayLevelSoundAction(ax, ay, ACTION_GROWING);
}

static void CheckForDragon(int x, int y)
{
  int i, j;
  boolean dragon_found = FALSE;
  struct XY *xy = xy_topdown;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    for (j = 0; j < 4; j++)
    {
      int xx = x + j * xy[i].x;
      int yy = y + j * xy[i].y;

      if (IN_LEV_FIELD(xx, yy) &&
	  (Tile[xx][yy] == EL_FLAMES || Tile[xx][yy] == EL_DRAGON))
      {
	if (Tile[xx][yy] == EL_DRAGON)
	  dragon_found = TRUE;
      }
      else
	break;
    }
  }

  if (!dragon_found)
  {
    for (i = 0; i < NUM_DIRECTIONS; i++)
    {
      for (j = 0; j < 3; j++)
      {
	int xx = x + j * xy[i].x;
	int yy = y + j * xy[i].y;

  	if (IN_LEV_FIELD(xx, yy) && Tile[xx][yy] == EL_FLAMES)
  	{
	  Tile[xx][yy] = EL_EMPTY;
	  TEST_DrawLevelField(xx, yy);
  	}
  	else
  	  break;
      }
    }
  }
}

static void InitBuggyBase(int x, int y)
{
  int element = Tile[x][y];
  int activating_delay = FRAMES_PER_SECOND / 4;

  ChangeDelay[x][y] =
    (element == EL_SP_BUGGY_BASE ?
     2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND) - activating_delay :
     element == EL_SP_BUGGY_BASE_ACTIVATING ?
     activating_delay :
     element == EL_SP_BUGGY_BASE_ACTIVE ?
     1 * FRAMES_PER_SECOND + RND(1 * FRAMES_PER_SECOND) : 1);
}

static void WarnBuggyBase(int x, int y)
{
  int i;
  struct XY *xy = xy_topdown;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;

    if (IN_LEV_FIELD(xx, yy) && IS_PLAYER(xx, yy))
    {
      PlayLevelSound(x, y, SND_SP_BUGGY_BASE_ACTIVE);

      break;
    }
  }
}

static void InitTrap(int x, int y)
{
  ChangeDelay[x][y] = 2 * FRAMES_PER_SECOND + RND(5 * FRAMES_PER_SECOND);
}

static void ActivateTrap(int x, int y)
{
  PlayLevelSound(x, y, SND_TRAP_ACTIVATING);
}

static void ChangeActiveTrap(int x, int y)
{
  int graphic = IMG_TRAP_ACTIVE;

  // if new animation frame was drawn, correct crumbled sand border
  if (IS_NEW_FRAME(GfxFrame[x][y], graphic))
    TEST_DrawLevelFieldCrumbled(x, y);
}

static int getSpecialActionElement(int element, int number, int base_element)
{
  return (element != EL_EMPTY ? element :
	  number != -1 ? base_element + number - 1 :
	  EL_EMPTY);
}

static int getModifiedActionNumber(int value_old, int operator, int operand,
				   int value_min, int value_max)
{
  int value_new = (operator == CA_MODE_SET      ? operand :
		   operator == CA_MODE_ADD      ? value_old + operand :
		   operator == CA_MODE_SUBTRACT ? value_old - operand :
		   operator == CA_MODE_MULTIPLY ? value_old * operand :
		   operator == CA_MODE_DIVIDE   ? value_old / MAX(1, operand) :
		   operator == CA_MODE_MODULO   ? value_old % MAX(1, operand) :
		   value_old);

  return (value_new < value_min ? value_min :
	  value_new > value_max ? value_max :
	  value_new);
}

static void ExecuteCustomElementAction(int x, int y, int element, int page)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementChangeInfo *change = &ei->change_page[page];
  int target_element = change->target_element;
  int action_type = change->action_type;
  int action_mode = change->action_mode;
  int action_arg = change->action_arg;
  int action_element = change->action_element;
  int i;

  if (!change->has_action)
    return;

  // ---------- determine action paramater values -----------------------------

  int level_time_value =
    (level.time > 0 ? TimeLeft :
     TimePlayed);

  int action_arg_element_raw =
    (action_arg == CA_ARG_PLAYER_TRIGGER  ? change->actual_trigger_player :
     action_arg == CA_ARG_ELEMENT_TRIGGER ? change->actual_trigger_element :
     action_arg == CA_ARG_ELEMENT_TARGET  ? change->target_element :
     action_arg == CA_ARG_ELEMENT_ACTION  ? change->action_element :
     action_arg == CA_ARG_INVENTORY_RM_TRIGGER ? change->actual_trigger_element:
     action_arg == CA_ARG_INVENTORY_RM_TARGET  ? change->target_element :
     action_arg == CA_ARG_INVENTORY_RM_ACTION  ? change->action_element :
     EL_EMPTY);
  int action_arg_element = GetElementFromGroupElement(action_arg_element_raw);

  int action_arg_direction =
    (action_arg >= CA_ARG_DIRECTION_LEFT &&
     action_arg <= CA_ARG_DIRECTION_DOWN ? action_arg - CA_ARG_DIRECTION :
     action_arg == CA_ARG_DIRECTION_TRIGGER ?
     change->actual_trigger_side :
     action_arg == CA_ARG_DIRECTION_TRIGGER_BACK ?
     MV_DIR_OPPOSITE(change->actual_trigger_side) :
     MV_NONE);

  int action_arg_number_min =
    (action_type == CA_SET_PLAYER_SPEED ? STEPSIZE_NOT_MOVING :
     CA_ARG_MIN);

  int action_arg_number_max =
    (action_type == CA_SET_PLAYER_SPEED ? STEPSIZE_EVEN_FASTER :
     action_type == CA_SET_LEVEL_GEMS ? 999 :
     action_type == CA_SET_LEVEL_TIME ? 9999 :
     action_type == CA_SET_LEVEL_SCORE ? 99999 :
     action_type == CA_SET_CE_VALUE ? 9999 :
     action_type == CA_SET_CE_SCORE ? 9999 :
     CA_ARG_MAX);

  int action_arg_number_reset =
    (action_type == CA_SET_PLAYER_SPEED ? level.initial_player_stepsize[0] :
     action_type == CA_SET_LEVEL_GEMS ? level.gems_needed :
     action_type == CA_SET_LEVEL_TIME ? level.time :
     action_type == CA_SET_LEVEL_SCORE ? 0 :
     action_type == CA_SET_CE_VALUE ? GET_NEW_CE_VALUE(element) :
     action_type == CA_SET_CE_SCORE ? 0 :
     0);

  int action_arg_number =
    (action_arg <= CA_ARG_MAX ? action_arg :
     action_arg >= CA_ARG_SPEED_NOT_MOVING &&
     action_arg <= CA_ARG_SPEED_EVEN_FASTER ? (action_arg - CA_ARG_SPEED) :
     action_arg == CA_ARG_SPEED_RESET ? action_arg_number_reset :
     action_arg == CA_ARG_NUMBER_MIN ? action_arg_number_min :
     action_arg == CA_ARG_NUMBER_MAX ? action_arg_number_max :
     action_arg == CA_ARG_NUMBER_RESET ? action_arg_number_reset :
     action_arg == CA_ARG_NUMBER_CE_VALUE ? CustomValue[x][y] :
     action_arg == CA_ARG_NUMBER_CE_SCORE ? ei->collect_score :
     action_arg == CA_ARG_NUMBER_CE_DELAY ? GET_CE_DELAY_VALUE(change) :
     action_arg == CA_ARG_NUMBER_LEVEL_TIME ? level_time_value :
     action_arg == CA_ARG_NUMBER_LEVEL_GEMS ? game.gems_still_needed :
     action_arg == CA_ARG_NUMBER_LEVEL_SCORE ? game.score :
     action_arg == CA_ARG_ELEMENT_CV_TARGET ? GET_NEW_CE_VALUE(target_element):
     action_arg == CA_ARG_ELEMENT_CV_TRIGGER ? change->actual_trigger_ce_value:
     action_arg == CA_ARG_ELEMENT_CV_ACTION ? GET_NEW_CE_VALUE(action_element):
     action_arg == CA_ARG_ELEMENT_CS_TARGET ? GET_CE_SCORE(target_element) :
     action_arg == CA_ARG_ELEMENT_CS_TRIGGER ? change->actual_trigger_ce_score:
     action_arg == CA_ARG_ELEMENT_CS_ACTION ? GET_CE_SCORE(action_element) :
     action_arg == CA_ARG_ELEMENT_NR_TARGET  ? change->target_element :
     action_arg == CA_ARG_ELEMENT_NR_TRIGGER ? change->actual_trigger_element :
     action_arg == CA_ARG_ELEMENT_NR_ACTION  ? change->action_element :
     -1);

  int action_arg_number_old =
    (action_type == CA_SET_LEVEL_GEMS ? game.gems_still_needed :
     action_type == CA_SET_LEVEL_TIME ? TimeLeft :
     action_type == CA_SET_LEVEL_SCORE ? game.score :
     action_type == CA_SET_CE_VALUE ? CustomValue[x][y] :
     action_type == CA_SET_CE_SCORE ? ei->collect_score :
     0);

  int action_arg_number_new =
    getModifiedActionNumber(action_arg_number_old,
			    action_mode, action_arg_number,
			    action_arg_number_min, action_arg_number_max);

  int trigger_player_bits =
    (change->actual_trigger_player_bits != CH_PLAYER_NONE ?
     change->actual_trigger_player_bits : change->trigger_player);

  int action_arg_player_bits =
    (action_arg >= CA_ARG_PLAYER_1 &&
     action_arg <= CA_ARG_PLAYER_4 ? action_arg - CA_ARG_PLAYER :
     action_arg == CA_ARG_PLAYER_TRIGGER ? trigger_player_bits :
     action_arg == CA_ARG_PLAYER_ACTION ? 1 << GET_PLAYER_NR(action_element) :
     PLAYER_BITS_ANY);

  // ---------- execute action  -----------------------------------------------

  switch (action_type)
  {
    case CA_NO_ACTION:
    {
      return;
    }

    // ---------- level actions  ----------------------------------------------

    case CA_RESTART_LEVEL:
    {
      game.restart_level = TRUE;

      break;
    }

    case CA_SHOW_ENVELOPE:
    {
      int element = getSpecialActionElement(action_arg_element,
					    action_arg_number, EL_ENVELOPE_1);

      if (IS_ENVELOPE(element))
	local_player->show_envelope = element;

      break;
    }

    case CA_SET_LEVEL_TIME:
    {
      if (level.time > 0)	// only modify limited time value
      {
	TimeLeft = action_arg_number_new;

	game_panel_controls[GAME_PANEL_TIME].value = TimeLeft;

	DisplayGameControlValues();

	if (!TimeLeft && game.time_limit)
	  for (i = 0; i < MAX_PLAYERS; i++)
	    KillPlayer(&stored_player[i]);
      }

      break;
    }

    case CA_SET_LEVEL_SCORE:
    {
      game.score = action_arg_number_new;

      game_panel_controls[GAME_PANEL_SCORE].value = game.score;

      DisplayGameControlValues();

      break;
    }

    case CA_SET_LEVEL_GEMS:
    {
      game.gems_still_needed = action_arg_number_new;

      game.snapshot.collected_item = TRUE;

      game_panel_controls[GAME_PANEL_GEMS].value = game.gems_still_needed;

      DisplayGameControlValues();

      break;
    }

    case CA_SET_LEVEL_WIND:
    {
      game.wind_direction = action_arg_direction;

      break;
    }

    case CA_SET_LEVEL_RANDOM_SEED:
    {
      // ensure that setting a new random seed while playing is predictable
      InitRND(action_arg_number_new ? action_arg_number_new : RND(1000000) + 1);

      break;
    }

    // ---------- player actions  ---------------------------------------------

    case CA_MOVE_PLAYER:
    case CA_MOVE_PLAYER_NEW:
    {
      // automatically move to the next field in specified direction
      for (i = 0; i < MAX_PLAYERS; i++)
	if (trigger_player_bits & (1 << i))
	  if (action_type == CA_MOVE_PLAYER ||
	      stored_player[i].MovPos == 0)
	    stored_player[i].programmed_action = action_arg_direction;

      break;
    }

    case CA_EXIT_PLAYER:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
	if (action_arg_player_bits & (1 << i))
	  ExitPlayer(&stored_player[i]);

      if (game.players_still_needed == 0)
	LevelSolved();

      break;
    }

    case CA_KILL_PLAYER:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
	if (action_arg_player_bits & (1 << i))
	  KillPlayer(&stored_player[i]);

      break;
    }

    case CA_SET_PLAYER_KEYS:
    {
      int key_state = (action_mode == CA_MODE_ADD ? TRUE : FALSE);
      int element = getSpecialActionElement(action_arg_element,
					    action_arg_number, EL_KEY_1);

      if (IS_KEY(element))
      {
	for (i = 0; i < MAX_PLAYERS; i++)
	{
	  if (trigger_player_bits & (1 << i))
	  {
	    stored_player[i].key[KEY_NR(element)] = key_state;

	    DrawGameDoorValues();
	  }
	}
      }

      break;
    }

    case CA_SET_PLAYER_SPEED:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
	if (trigger_player_bits & (1 << i))
	{
	  int move_stepsize = TILEX / stored_player[i].move_delay_value;

	  if (action_arg == CA_ARG_SPEED_FASTER &&
	      stored_player[i].cannot_move)
	  {
	    action_arg_number = STEPSIZE_VERY_SLOW;
	  }
	  else if (action_arg == CA_ARG_SPEED_SLOWER ||
		   action_arg == CA_ARG_SPEED_FASTER)
	  {
	    action_arg_number = 2;
	    action_mode = (action_arg == CA_ARG_SPEED_SLOWER ? CA_MODE_DIVIDE :
			   CA_MODE_MULTIPLY);
	  }
	  else if (action_arg == CA_ARG_NUMBER_RESET)
	  {
	    action_arg_number = level.initial_player_stepsize[i];
	  }

	  move_stepsize =
	    getModifiedActionNumber(move_stepsize,
				    action_mode,
				    action_arg_number,
				    action_arg_number_min,
				    action_arg_number_max);

	  SetPlayerMoveSpeed(&stored_player[i], move_stepsize, FALSE);
	}
      }

      break;
    }

    case CA_SET_PLAYER_SHIELD:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
	if (trigger_player_bits & (1 << i))
	{
	  if (action_arg == CA_ARG_SHIELD_OFF)
	  {
	    stored_player[i].shield_normal_time_left = 0;
	    stored_player[i].shield_deadly_time_left = 0;
	  }
	  else if (action_arg == CA_ARG_SHIELD_NORMAL)
	  {
	    stored_player[i].shield_normal_time_left = 999999;
	  }
	  else if (action_arg == CA_ARG_SHIELD_DEADLY)
	  {
	    stored_player[i].shield_normal_time_left = 999999;
	    stored_player[i].shield_deadly_time_left = 999999;
	  }
	}
      }

      break;
    }

    case CA_SET_PLAYER_GRAVITY:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
	if (trigger_player_bits & (1 << i))
	{
	  stored_player[i].gravity =
	    (action_arg == CA_ARG_GRAVITY_OFF    ? FALSE                     :
	     action_arg == CA_ARG_GRAVITY_ON     ? TRUE                      :
	     action_arg == CA_ARG_GRAVITY_TOGGLE ? !stored_player[i].gravity :
	     stored_player[i].gravity);
	}
      }

      break;
    }

    case CA_SET_PLAYER_ARTWORK:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
	if (trigger_player_bits & (1 << i))
	{
	  int artwork_element = action_arg_element;

	  if (action_arg == CA_ARG_ELEMENT_RESET)
	    artwork_element =
	      (level.use_artwork_element[i] ? level.artwork_element[i] :
	       stored_player[i].element_nr);

	  if (stored_player[i].artwork_element != artwork_element)
	    stored_player[i].Frame = 0;

	  stored_player[i].artwork_element = artwork_element;

	  SetPlayerWaiting(&stored_player[i], FALSE);

	  // set number of special actions for bored and sleeping animation
	  stored_player[i].num_special_action_bored =
	    get_num_special_action(artwork_element,
				   ACTION_BORING_1, ACTION_BORING_LAST);
	  stored_player[i].num_special_action_sleeping =
	    get_num_special_action(artwork_element,
				   ACTION_SLEEPING_1, ACTION_SLEEPING_LAST);
	}
      }

      break;
    }

    case CA_SET_PLAYER_INVENTORY:
    {
      for (i = 0; i < MAX_PLAYERS; i++)
      {
	struct PlayerInfo *player = &stored_player[i];
	int j, k;

	if (trigger_player_bits & (1 << i))
	{
	  int inventory_element = action_arg_element;

	  if (action_arg == CA_ARG_ELEMENT_TARGET ||
	      action_arg == CA_ARG_ELEMENT_TRIGGER ||
	      action_arg == CA_ARG_ELEMENT_ACTION)
	  {
	    int element = inventory_element;
	    int collect_count = element_info[element].collect_count_initial;

	    if (!IS_CUSTOM_ELEMENT(element))
	      collect_count = 1;

	    if (collect_count == 0)
	      player->inventory_infinite_element = element;
	    else
	      for (k = 0; k < collect_count; k++)
		if (player->inventory_size < MAX_INVENTORY_SIZE)
		  player->inventory_element[player->inventory_size++] =
		    element;
	  }
	  else if (action_arg == CA_ARG_INVENTORY_RM_TARGET ||
		   action_arg == CA_ARG_INVENTORY_RM_TRIGGER ||
		   action_arg == CA_ARG_INVENTORY_RM_ACTION)
	  {
	    if (player->inventory_infinite_element != EL_UNDEFINED &&
		IS_EQUAL_OR_IN_GROUP(player->inventory_infinite_element,
				     action_arg_element_raw))
	      player->inventory_infinite_element = EL_UNDEFINED;

	    for (k = 0, j = 0; j < player->inventory_size; j++)
	    {
	      if (!IS_EQUAL_OR_IN_GROUP(player->inventory_element[j],
					action_arg_element_raw))
		player->inventory_element[k++] = player->inventory_element[j];
	    }

	    player->inventory_size = k;
	  }
	  else if (action_arg == CA_ARG_INVENTORY_RM_FIRST)
	  {
	    if (player->inventory_size > 0)
	    {
	      for (j = 0; j < player->inventory_size - 1; j++)
		player->inventory_element[j] = player->inventory_element[j + 1];

	      player->inventory_size--;
	    }
	  }
	  else if (action_arg == CA_ARG_INVENTORY_RM_LAST)
	  {
	    if (player->inventory_size > 0)
	      player->inventory_size--;
	  }
	  else if (action_arg == CA_ARG_INVENTORY_RM_ALL)
	  {
	    player->inventory_infinite_element = EL_UNDEFINED;
	    player->inventory_size = 0;
	  }
	  else if (action_arg == CA_ARG_INVENTORY_RESET)
	  {
	    player->inventory_infinite_element = EL_UNDEFINED;
	    player->inventory_size = 0;

	    if (level.use_initial_inventory[i])
	    {
	      for (j = 0; j < level.initial_inventory_size[i]; j++)
	      {
		int element = level.initial_inventory_content[i][j];
		int collect_count = element_info[element].collect_count_initial;

		if (!IS_CUSTOM_ELEMENT(element))
		  collect_count = 1;

		if (collect_count == 0)
		  player->inventory_infinite_element = element;
		else
		  for (k = 0; k < collect_count; k++)
		    if (player->inventory_size < MAX_INVENTORY_SIZE)
		      player->inventory_element[player->inventory_size++] =
			element;
	      }
	    }
	  }
	}
      }

      break;
    }

    // ---------- CE actions  -------------------------------------------------

    case CA_SET_CE_VALUE:
    {
      int last_ce_value = CustomValue[x][y];

      CustomValue[x][y] = action_arg_number_new;

      if (CustomValue[x][y] != last_ce_value)
      {
	CheckElementChange(x, y, element, EL_UNDEFINED, CE_VALUE_CHANGES);
	CheckTriggeredElementChange(x, y, element, CE_VALUE_CHANGES_OF_X);

	if (CustomValue[x][y] == 0)
	{
	  // reset change counter (else CE_VALUE_GETS_ZERO would not work)
	  ChangeCount[x][y] = 0;	// allow at least one more change

	  CheckElementChange(x, y, element, EL_UNDEFINED, CE_VALUE_GETS_ZERO);
	  CheckTriggeredElementChange(x, y, element, CE_VALUE_GETS_ZERO_OF_X);
	}
      }

      break;
    }

    case CA_SET_CE_SCORE:
    {
      int last_ce_score = ei->collect_score;

      ei->collect_score = action_arg_number_new;

      if (ei->collect_score != last_ce_score)
      {
	CheckElementChange(x, y, element, EL_UNDEFINED, CE_SCORE_CHANGES);
	CheckTriggeredElementChange(x, y, element, CE_SCORE_CHANGES_OF_X);

	if (ei->collect_score == 0)
	{
	  int xx, yy;

	  // reset change counter (else CE_SCORE_GETS_ZERO would not work)
	  ChangeCount[x][y] = 0;	// allow at least one more change

	  CheckElementChange(x, y, element, EL_UNDEFINED, CE_SCORE_GETS_ZERO);
	  CheckTriggeredElementChange(x, y, element, CE_SCORE_GETS_ZERO_OF_X);

	  /*
	    This is a very special case that seems to be a mixture between
	    CheckElementChange() and CheckTriggeredElementChange(): while
	    the first one only affects single elements that are triggered
	    directly, the second one affects multiple elements in the playfield
	    that are triggered indirectly by another element. This is a third
	    case: Changing the CE score always affects multiple identical CEs,
	    so every affected CE must be checked, not only the single CE for
	    which the CE score was changed in the first place (as every instance
	    of that CE shares the same CE score, and therefore also can change)!
	  */
	  SCAN_PLAYFIELD(xx, yy)
	  {
	    if (Tile[xx][yy] == element)
	      CheckElementChange(xx, yy, element, EL_UNDEFINED,
				 CE_SCORE_GETS_ZERO);
	  }
	}
      }

      break;
    }

    case CA_SET_CE_ARTWORK:
    {
      int artwork_element = action_arg_element;
      boolean reset_frame = FALSE;
      int xx, yy;

      if (action_arg == CA_ARG_ELEMENT_RESET)
	artwork_element = (ei->use_gfx_element ? ei->gfx_element_initial :
			   element);

      if (ei->gfx_element != artwork_element)
	reset_frame = TRUE;

      ei->gfx_element = artwork_element;

      SCAN_PLAYFIELD(xx, yy)
      {
	if (Tile[xx][yy] == element)
	{
	  if (reset_frame)
	  {
	    ResetGfxAnimation(xx, yy);
	    ResetRandomAnimationValue(xx, yy);
	  }

	  TEST_DrawLevelField(xx, yy);
	}
      }

      break;
    }

    // ---------- engine actions  ---------------------------------------------

    case CA_SET_ENGINE_SCAN_MODE:
    {
      InitPlayfieldScanMode(action_arg);

      break;
    }

    default:
      break;
  }
}

static void CreateFieldExt(int x, int y, int element, boolean is_change)
{
  int old_element = Tile[x][y];
  int new_element = GetElementFromGroupElement(element);
  int previous_move_direction = MovDir[x][y];
  int last_ce_value = CustomValue[x][y];
  boolean player_explosion_protected = PLAYER_EXPLOSION_PROTECTED(x, y);
  boolean new_element_is_player = IS_PLAYER_ELEMENT(new_element);
  boolean add_player_onto_element = (new_element_is_player &&
				     new_element != EL_SOKOBAN_FIELD_PLAYER &&
				     IS_WALKABLE(old_element));

  if (!add_player_onto_element)
  {
    if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
      RemoveMovingField(x, y);
    else
      RemoveField(x, y);

    Tile[x][y] = new_element;

    if (element_info[new_element].move_direction_initial == MV_START_PREVIOUS)
      MovDir[x][y] = previous_move_direction;

    if (element_info[new_element].use_last_ce_value)
      CustomValue[x][y] = last_ce_value;

    InitField_WithBug1(x, y, FALSE);

    new_element = Tile[x][y];	// element may have changed

    ResetGfxAnimation(x, y);
    ResetRandomAnimationValue(x, y);

    TEST_DrawLevelField(x, y);

    if (GFX_CRUMBLED(new_element))
      TEST_DrawLevelFieldCrumbledNeighbours(x, y);

    if (old_element == EL_EXPLOSION)
    {
      Store[x][y] = Store2[x][y] = 0;

      // check if new element replaces an exploding player, requiring cleanup
      if (IS_PLAYER(x, y) && !PLAYERINFO(x, y)->present)
	StorePlayer[x][y] = 0;
    }

    // check if element under the player changes from accessible to unaccessible
    // (needed for special case of dropping element which then changes)
    // (must be checked after creating new element for walkable group elements)
    if (IS_PLAYER(x, y) && !player_explosion_protected &&
	IS_ACCESSIBLE(old_element) && !IS_ACCESSIBLE(new_element))
    {
      KillPlayer(PLAYERINFO(x, y));

      return;
    }
  }

  // "ChangeCount" not set yet to allow "entered by player" change one time
  if (new_element_is_player)
    RelocatePlayer(x, y, new_element);

  if (is_change)
    ChangeCount[x][y]++;	// count number of changes in the same frame

  TestIfBadThingTouchesPlayer(x, y);
  TestIfPlayerTouchesCustomElement(x, y);
  TestIfElementTouchesCustomElement(x, y);
}

static void CreateField(int x, int y, int element)
{
  CreateFieldExt(x, y, element, FALSE);
}

static void CreateElementFromChange(int x, int y, int element)
{
  element = GET_VALID_RUNTIME_ELEMENT(element);

  if (game.engine_version >= VERSION_IDENT(3,2,0,7))
  {
    int old_element = Tile[x][y];

    // prevent changed element from moving in same engine frame
    // unless both old and new element can either fall or move
    if ((!CAN_FALL(old_element) || !CAN_FALL(element)) &&
	(!CAN_MOVE(old_element) || !CAN_MOVE(element)))
      Stop[x][y] = TRUE;
  }

  CreateFieldExt(x, y, element, TRUE);
}

static boolean ChangeElement(int x, int y, int element, int page)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementChangeInfo *change = &ei->change_page[page];
  int ce_value = CustomValue[x][y];
  int ce_score = ei->collect_score;
  int target_element;
  int old_element = Tile[x][y];

  // always use default change event to prevent running into a loop
  if (ChangeEvent[x][y] == -1)
    ChangeEvent[x][y] = CE_DELAY;

  if (ChangeEvent[x][y] == CE_DELAY)
  {
    // reset actual trigger element, trigger player and action element
    change->actual_trigger_element = EL_EMPTY;
    change->actual_trigger_player = EL_EMPTY;
    change->actual_trigger_player_bits = CH_PLAYER_NONE;
    change->actual_trigger_side = CH_SIDE_NONE;
    change->actual_trigger_ce_value = 0;
    change->actual_trigger_ce_score = 0;
    change->actual_trigger_x = -1;
    change->actual_trigger_y = -1;
  }

  // do not change elements more than a specified maximum number of changes
  if (ChangeCount[x][y] >= game.max_num_changes_per_frame)
    return FALSE;

  ChangeCount[x][y]++;		// count number of changes in the same frame

  if (ei->has_anim_event)
    HandleGlobalAnimEventByElementChange(element, page, x, y,
					 change->actual_trigger_x,
					 change->actual_trigger_y);

  if (change->explode)
  {
    Bang(x, y);

    return TRUE;
  }

  if (change->use_target_content)
  {
    boolean complete_replace = TRUE;
    boolean can_replace[3][3];
    int xx, yy;

    for (yy = 0; yy < 3; yy++) for (xx = 0; xx < 3 ; xx++)
    {
      boolean is_empty;
      boolean is_walkable;
      boolean is_diggable;
      boolean is_collectible;
      boolean is_removable;
      boolean is_destructible;
      int ex = x + xx - 1;
      int ey = y + yy - 1;
      int content_element = change->target_content.e[xx][yy];
      int e;

      can_replace[xx][yy] = TRUE;

      if (ex == x && ey == y)	// do not check changing element itself
	continue;

      if (content_element == EL_EMPTY_SPACE)
      {
	can_replace[xx][yy] = FALSE;	// do not replace border with space

	continue;
      }

      if (!IN_LEV_FIELD(ex, ey))
      {
	can_replace[xx][yy] = FALSE;
	complete_replace = FALSE;

	continue;
      }

      e = Tile[ex][ey];

      if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
	e = MovingOrBlocked2Element(ex, ey);

      is_empty = (IS_FREE(ex, ey) ||
		  (IS_FREE_OR_PLAYER(ex, ey) && IS_WALKABLE(content_element)));

      is_walkable     = (is_empty || IS_WALKABLE(e));
      is_diggable     = (is_empty || IS_DIGGABLE(e));
      is_collectible  = (is_empty || IS_COLLECTIBLE(e));
      is_destructible = (is_empty || !IS_INDESTRUCTIBLE(e));
      is_removable    = (is_diggable || is_collectible);

      can_replace[xx][yy] =
	(((change->replace_when == CP_WHEN_EMPTY        && is_empty) ||
	  (change->replace_when == CP_WHEN_WALKABLE     && is_walkable) ||
	  (change->replace_when == CP_WHEN_DIGGABLE     && is_diggable) ||
	  (change->replace_when == CP_WHEN_COLLECTIBLE  && is_collectible) ||
	  (change->replace_when == CP_WHEN_REMOVABLE    && is_removable) ||
	  (change->replace_when == CP_WHEN_DESTRUCTIBLE && is_destructible)) &&
	 !(IS_PLAYER(ex, ey) && IS_PLAYER_ELEMENT(content_element)));

      if (!can_replace[xx][yy])
	complete_replace = FALSE;
    }

    if (!change->only_if_complete || complete_replace)
    {
      boolean something_has_changed = FALSE;

      if (change->only_if_complete && change->use_random_replace &&
	  RND(100) < change->random_percentage)
	return FALSE;

      for (yy = 0; yy < 3; yy++) for (xx = 0; xx < 3 ; xx++)
      {
	int ex = x + xx - 1;
	int ey = y + yy - 1;
	int content_element;

	if (can_replace[xx][yy] && (!change->use_random_replace ||
				    RND(100) < change->random_percentage))
	{
	  if (IS_MOVING(ex, ey) || IS_BLOCKED(ex, ey))
	    RemoveMovingField(ex, ey);

	  ChangeEvent[ex][ey] = ChangeEvent[x][y];

	  content_element = change->target_content.e[xx][yy];
	  target_element = GET_TARGET_ELEMENT(element, content_element, change,
					      ce_value, ce_score);

	  CreateElementFromChange(ex, ey, target_element);

	  something_has_changed = TRUE;

	  // for symmetry reasons, freeze newly created border elements
	  if (ex != x || ey != y)
	    Stop[ex][ey] = TRUE;	// no more moving in this frame
	}
      }

      if (something_has_changed)
      {
	PlayLevelSoundElementAction(x, y, element, ACTION_CHANGING);
	PlayLevelSoundElementAction(x, y, element, ACTION_PAGE_1 + page);
      }
    }
  }
  else
  {
    target_element = GET_TARGET_ELEMENT(element, change->target_element, change,
					ce_value, ce_score);

    if (element == EL_DIAGONAL_GROWING ||
	element == EL_DIAGONAL_SHRINKING)
    {
      target_element = Store[x][y];

      Store[x][y] = EL_EMPTY;
    }

    // special case: element changes to player (and may be kept if walkable)
    if (IS_PLAYER_ELEMENT(target_element) && !level.keep_walkable_ce)
      CreateElementFromChange(x, y, EL_EMPTY);

    CreateElementFromChange(x, y, target_element);

    PlayLevelSoundElementAction(x, y, element, ACTION_CHANGING);
    PlayLevelSoundElementAction(x, y, element, ACTION_PAGE_1 + page);
  }

  // this uses direct change before indirect change
  CheckTriggeredElementChangeByPage(x, y, old_element, CE_CHANGE_OF_X, page);

  return TRUE;
}

static void HandleElementChange(int x, int y, int page)
{
  int element = MovingOrBlocked2Element(x, y);
  struct ElementInfo *ei = &element_info[element];
  struct ElementChangeInfo *change = &ei->change_page[page];
  boolean handle_action_before_change = FALSE;

#ifdef DEBUG
  if (!CAN_CHANGE_OR_HAS_ACTION(element) &&
      !CAN_CHANGE_OR_HAS_ACTION(Back[x][y]))
  {
    Debug("game:playing:HandleElementChange", "%d,%d: element = %d ('%s')",
	  x, y, element, element_info[element].token_name);
    Debug("game:playing:HandleElementChange", "This should never happen!");
  }
#endif

  // this can happen with classic bombs on walkable, changing elements
  if (!CAN_CHANGE_OR_HAS_ACTION(element))
  {
    return;
  }

  if (ChangeDelay[x][y] == 0)		// initialize element change
  {
    ChangeDelay[x][y] = GET_CHANGE_DELAY(change) + 1;

    if (change->can_change)
    {
      // !!! not clear why graphic animation should be reset at all here !!!
      // !!! UPDATE: but is needed for correct Snake Bite tail animation !!!
      // !!! SOLUTION: do not reset if graphics engine set to 4 or above !!!

      /*
	GRAPHICAL BUG ADDRESSED BY CHECKING GRAPHICS ENGINE VERSION:

	When using an animation frame delay of 1 (this only happens with
	"sp_zonk.moving.left/right" in the classic graphics), the default
	(non-moving) animation shows wrong animation frames (while the
	moving animation, like "sp_zonk.moving.left/right", is correct,
	so this graphical bug never shows up with the classic graphics).
	For an animation with 4 frames, this causes wrong frames 0,0,1,2
	be drawn instead of the correct frames 0,1,2,3. This is caused by
	"GfxFrame[][]" being reset *twice* (in two successive frames) after
	an element change: First when the change delay ("ChangeDelay[][]")
	counter has reached zero after decrementing, then a second time in
	the next frame (after "GfxFrame[][]" was already incremented) when
	"ChangeDelay[][]" is reset to the initial delay value again.

	This causes frame 0 to be drawn twice, while the last frame won't
	be drawn anymore, resulting in the wrong frame sequence 0,0,1,2.

	As some animations may already be cleverly designed around this bug
	(at least the "Snake Bite" snake tail animation does this), it cannot
	simply be fixed here without breaking such existing animations.
	Unfortunately, it cannot easily be detected if a graphics set was
	designed "before" or "after" the bug was fixed. As a workaround,
	a new graphics set option "game.graphics_engine_version" was added
	to be able to specify the game's major release version for which the
	graphics set was designed, which can then be used to decide if the
	bugfix should be used (version 4 and above) or not (version 3 or
	below, or if no version was specified at all, as with old sets).

	(The wrong/fixed animation frames can be tested with the test level set
	"test_gfxframe" and level "000", which contains a specially prepared
	custom element at level position (x/y) == (11/9) which uses the zonk
	animation mentioned above. Using "game.graphics_engine_version: 4"
	fixes the wrong animation frames, showing the correct frames 0,1,2,3.
	This can also be seen from the debug output for this test element.)
      */

      // when a custom element is about to change (for example by change delay),
      // do not reset graphic animation when the custom element is moving
      if (game.graphics_engine_version < 4 &&
	  !IS_MOVING(x, y))
      {
	ResetGfxAnimation(x, y);
	ResetRandomAnimationValue(x, y);
      }

      if (change->pre_change_function)
	change->pre_change_function(x, y);
    }
  }

  ChangeDelay[x][y]--;

  if (ChangeDelay[x][y] != 0)		// continue element change
  {
    int graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

    // also needed if CE can not change, but has CE delay with CE action
    if (IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    if (change->can_change)
    {
      if (change->change_function)
	change->change_function(x, y);
    }
  }
  else					// finish element change
  {
    if (ChangePage[x][y] != -1)		// remember page from delayed change
    {
      page = ChangePage[x][y];
      ChangePage[x][y] = -1;

      change = &ei->change_page[page];
    }

    if (IS_MOVING(x, y))		// never change a running system ;-)
    {
      ChangeDelay[x][y] = 1;		// try change after next move step
      ChangePage[x][y] = page;		// remember page to use for change

      return;
    }

    // special case: set new level random seed before changing element
    if (change->has_action && change->action_type == CA_SET_LEVEL_RANDOM_SEED)
      handle_action_before_change = TRUE;

    if (change->has_action && handle_action_before_change)
      ExecuteCustomElementAction(x, y, element, page);

    if (change->can_change)
    {
      if (ChangeElement(x, y, element, page))
      {
	if (change->post_change_function)
	  change->post_change_function(x, y);
      }
    }

    if (change->has_action && !handle_action_before_change)
      ExecuteCustomElementAction(x, y, element, page);
  }
}

static boolean CheckTriggeredElementChangeExt(int trigger_x, int trigger_y,
					      int trigger_element,
					      int trigger_event,
					      int trigger_player,
					      int trigger_side,
					      int trigger_page)
{
  boolean change_done_any = FALSE;
  int trigger_page_bits = (trigger_page < 0 ? CH_PAGE_ANY : 1 << trigger_page);
  int i;

  if (!(trigger_events[trigger_element][trigger_event]))
    return FALSE;

  RECURSION_LOOP_DETECTION_START(trigger_element, FALSE);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;
    boolean change_done = FALSE;
    int p;

    if (!CAN_CHANGE_OR_HAS_ACTION(element) ||
	!HAS_ANY_CHANGE_EVENT(element, trigger_event))
      continue;

    for (p = 0; p < element_info[element].num_change_pages; p++)
    {
      struct ElementChangeInfo *change = &element_info[element].change_page[p];

      if (change->can_change_or_has_action &&
	  change->has_event[trigger_event] &&
	  change->trigger_side & trigger_side &&
	  change->trigger_player & trigger_player &&
	  change->trigger_page & trigger_page_bits &&
	  IS_EQUAL_OR_IN_GROUP(trigger_element, change->trigger_element))
      {
	change->actual_trigger_element = trigger_element;
	change->actual_trigger_player = GET_PLAYER_FROM_BITS(trigger_player);
	change->actual_trigger_player_bits = trigger_player;
	change->actual_trigger_side = trigger_side;
	change->actual_trigger_ce_value = CustomValue[trigger_x][trigger_y];
	change->actual_trigger_ce_score = GET_CE_SCORE(trigger_element);
	change->actual_trigger_x = trigger_x;
	change->actual_trigger_y = trigger_y;

	if ((change->can_change && !change_done) || change->has_action)
	{
	  int x, y;

	  SCAN_PLAYFIELD(x, y)
	  {
	    if (Tile[x][y] == element)
	    {
	      if (change->can_change && !change_done)
	      {
		// if element already changed in this frame, not only prevent
		// another element change (checked in ChangeElement()), but
		// also prevent additional element actions for this element

		if (ChangeCount[x][y] >= game.max_num_changes_per_frame &&
		    !level.use_action_after_change_bug)
		  continue;

		ChangeDelay[x][y] = 1;
		ChangeEvent[x][y] = trigger_event;

		HandleElementChange(x, y, p);
	      }
	      else if (change->has_action)
	      {
		// if element already changed in this frame, not only prevent
		// another element change (checked in ChangeElement()), but
		// also prevent additional element actions for this element

		if (ChangeCount[x][y] >= game.max_num_changes_per_frame &&
		    !level.use_action_after_change_bug)
		  continue;

		ExecuteCustomElementAction(x, y, element, p);
		PlayLevelSoundElementAction(x, y, element, ACTION_PAGE_1 + p);
	      }
	    }
	  }

	  if (change->can_change)
	  {
	    change_done = TRUE;
	    change_done_any = TRUE;
	  }
	}
      }
    }
  }

  RECURSION_LOOP_DETECTION_END();

  return change_done_any;
}

static boolean CheckElementChangeExt(int x, int y,
				     int element,
				     int trigger_element,
				     int trigger_event,
				     int trigger_player,
				     int trigger_side)
{
  boolean change_done = FALSE;
  int p;

  if (!CAN_CHANGE_OR_HAS_ACTION(element) ||
      !HAS_ANY_CHANGE_EVENT(element, trigger_event))
    return FALSE;

  if (Tile[x][y] == EL_BLOCKED)
  {
    Blocked2Moving(x, y, &x, &y);
    element = Tile[x][y];
  }

  // check if element has already changed or is about to change after moving
  if ((game.engine_version < VERSION_IDENT(3,2,0,7) &&
       Tile[x][y] != element) ||

      (game.engine_version >= VERSION_IDENT(3,2,0,7) &&
       (ChangeCount[x][y] >= game.max_num_changes_per_frame ||
	ChangePage[x][y] != -1)))
    return FALSE;

  RECURSION_LOOP_DETECTION_START(trigger_element, FALSE);

  for (p = 0; p < element_info[element].num_change_pages; p++)
  {
    struct ElementChangeInfo *change = &element_info[element].change_page[p];

    /* check trigger element for all events where the element that is checked
       for changing interacts with a directly adjacent element -- this is
       different to element changes that affect other elements to change on the
       whole playfield (which is handeld by CheckTriggeredElementChangeExt()) */
    boolean check_trigger_element =
      (trigger_event == CE_NEXT_TO_X ||
       trigger_event == CE_TOUCHING_X ||
       trigger_event == CE_HITTING_X ||
       trigger_event == CE_HIT_BY_X ||
       trigger_event == CE_DIGGING_X); // this one was forgotten until 3.2.3

    if (change->can_change_or_has_action &&
	change->has_event[trigger_event] &&
	change->trigger_side & trigger_side &&
	change->trigger_player & trigger_player &&
	(!check_trigger_element ||
	 IS_EQUAL_OR_IN_GROUP(trigger_element, change->trigger_element)))
    {
      change->actual_trigger_element = trigger_element;
      change->actual_trigger_player = GET_PLAYER_FROM_BITS(trigger_player);
      change->actual_trigger_player_bits = trigger_player;
      change->actual_trigger_side = trigger_side;
      change->actual_trigger_ce_value = CustomValue[x][y];
      change->actual_trigger_ce_score = GET_CE_SCORE(trigger_element);
      change->actual_trigger_x = x;
      change->actual_trigger_y = y;

      // special case: trigger element not at (x,y) position for some events
      if (check_trigger_element)
      {
	static struct
	{
	  int dx, dy;
	} move_xy[] =
	  {
	    {  0,  0 },
	    { -1,  0 },
	    { +1,  0 },
	    {  0,  0 },
	    {  0, -1 },
	    {  0,  0 }, { 0, 0 }, { 0, 0 },
	    {  0, +1 }
	  };

	int xx = x + move_xy[MV_DIR_OPPOSITE(trigger_side)].dx;
	int yy = y + move_xy[MV_DIR_OPPOSITE(trigger_side)].dy;

	change->actual_trigger_ce_value = CustomValue[xx][yy];
	change->actual_trigger_ce_score = GET_CE_SCORE(trigger_element);
	change->actual_trigger_x = xx;
	change->actual_trigger_y = yy;
      }

      if (change->can_change && !change_done)
      {
	ChangeDelay[x][y] = 1;
	ChangeEvent[x][y] = trigger_event;

	HandleElementChange(x, y, p);

	change_done = TRUE;
      }
      else if (change->has_action)
      {
	ExecuteCustomElementAction(x, y, element, p);
	PlayLevelSoundElementAction(x, y, element, ACTION_PAGE_1 + p);
      }
    }
  }

  RECURSION_LOOP_DETECTION_END();

  return change_done;
}

static void PlayPlayerSound(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int sound_element = player->artwork_element;
  int last_action = player->last_action_waiting;
  int action = player->action_waiting;

  if (player->is_waiting)
  {
    if (action != last_action)
      PlayLevelSoundElementAction(jx, jy, sound_element, action);
    else
      PlayLevelSoundElementActionIfLoop(jx, jy, sound_element, action);
  }
  else
  {
    if (action != last_action)
      StopSound(element_info[sound_element].sound[last_action]);

    if (last_action == ACTION_SLEEPING)
      PlayLevelSoundElementAction(jx, jy, sound_element, ACTION_AWAKENING);
  }
}

static void PlayAllPlayersSound(void)
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].active)
      PlayPlayerSound(&stored_player[i]);
}

static void SetPlayerWaiting(struct PlayerInfo *player, boolean is_waiting)
{
  boolean last_waiting = player->is_waiting;
  int move_dir = player->MovDir;

  player->dir_waiting = move_dir;
  player->last_action_waiting = player->action_waiting;

  if (is_waiting)
  {
    if (!last_waiting)		// not waiting -> waiting
    {
      player->is_waiting = TRUE;

      player->frame_counter_bored =
	FrameCounter +
	game.player_boring_delay_fixed +
	GetSimpleRandom(game.player_boring_delay_random);
      player->frame_counter_sleeping =
	FrameCounter +
	game.player_sleeping_delay_fixed +
	GetSimpleRandom(game.player_sleeping_delay_random);

      InitPlayerGfxAnimation(player, ACTION_WAITING, move_dir);
    }

    if (game.player_sleeping_delay_fixed +
	game.player_sleeping_delay_random > 0 &&
	player->anim_delay_counter == 0 &&
	player->post_delay_counter == 0 &&
	FrameCounter >= player->frame_counter_sleeping)
      player->is_sleeping = TRUE;
    else if (game.player_boring_delay_fixed +
	     game.player_boring_delay_random > 0 &&
	     FrameCounter >= player->frame_counter_bored)
      player->is_bored = TRUE;

    player->action_waiting = (player->is_sleeping ? ACTION_SLEEPING :
			      player->is_bored ? ACTION_BORING :
			      ACTION_WAITING);

    if (player->is_sleeping && player->use_murphy)
    {
      // special case for sleeping Murphy when leaning against non-free tile

      if (!IN_LEV_FIELD(player->jx - 1, player->jy) ||
	  (Tile[player->jx - 1][player->jy] != EL_EMPTY &&
	   !IS_MOVING(player->jx - 1, player->jy)))
	move_dir = MV_LEFT;
      else if (!IN_LEV_FIELD(player->jx + 1, player->jy) ||
	       (Tile[player->jx + 1][player->jy] != EL_EMPTY &&
		!IS_MOVING(player->jx + 1, player->jy)))
	move_dir = MV_RIGHT;
      else
	player->is_sleeping = FALSE;

      player->dir_waiting = move_dir;
    }

    if (player->is_sleeping)
    {
      if (player->num_special_action_sleeping > 0)
      {
	if (player->anim_delay_counter == 0 && player->post_delay_counter == 0)
	{
	  int last_special_action = player->special_action_sleeping;
	  int num_special_action = player->num_special_action_sleeping;
	  int special_action =
	    (last_special_action == ACTION_DEFAULT ? ACTION_SLEEPING_1 :
	     last_special_action == ACTION_SLEEPING ? ACTION_SLEEPING :
	     last_special_action < ACTION_SLEEPING_1 + num_special_action - 1 ?
	     last_special_action + 1 : ACTION_SLEEPING);
	  int special_graphic =
	    el_act_dir2img(player->artwork_element, special_action, move_dir);

	  player->anim_delay_counter =
	    graphic_info[special_graphic].anim_delay_fixed +
	    GetSimpleRandom(graphic_info[special_graphic].anim_delay_random);
	  player->post_delay_counter =
	    graphic_info[special_graphic].post_delay_fixed +
	    GetSimpleRandom(graphic_info[special_graphic].post_delay_random);

	  player->special_action_sleeping = special_action;
	}

	if (player->anim_delay_counter > 0)
	{
	  player->action_waiting = player->special_action_sleeping;
	  player->anim_delay_counter--;
	}
	else if (player->post_delay_counter > 0)
	{
	  player->post_delay_counter--;
	}
      }
    }
    else if (player->is_bored)
    {
      if (player->num_special_action_bored > 0)
      {
	if (player->anim_delay_counter == 0 && player->post_delay_counter == 0)
	{
	  int special_action =
	    ACTION_BORING_1 + GetSimpleRandom(player->num_special_action_bored);
	  int special_graphic =
	    el_act_dir2img(player->artwork_element, special_action, move_dir);

	  player->anim_delay_counter =
	    graphic_info[special_graphic].anim_delay_fixed +
	    GetSimpleRandom(graphic_info[special_graphic].anim_delay_random);
	  player->post_delay_counter =
	    graphic_info[special_graphic].post_delay_fixed +
	    GetSimpleRandom(graphic_info[special_graphic].post_delay_random);

	  player->special_action_bored = special_action;
	}

	if (player->anim_delay_counter > 0)
	{
	  player->action_waiting = player->special_action_bored;
	  player->anim_delay_counter--;
	}
	else if (player->post_delay_counter > 0)
	{
	  player->post_delay_counter--;
	}
      }
    }
  }
  else if (last_waiting)	// waiting -> not waiting
  {
    player->is_waiting = FALSE;
    player->is_bored = FALSE;
    player->is_sleeping = FALSE;

    player->frame_counter_bored = -1;
    player->frame_counter_sleeping = -1;

    player->anim_delay_counter = 0;
    player->post_delay_counter = 0;

    player->dir_waiting = player->MovDir;
    player->action_waiting = ACTION_DEFAULT;

    player->special_action_bored = ACTION_DEFAULT;
    player->special_action_sleeping = ACTION_DEFAULT;
  }
}

static void CheckSaveEngineSnapshot(struct PlayerInfo *player)
{
  if ((!player->is_moving  && player->was_moving) ||
      (player->MovPos == 0 && player->was_moving) ||
      (player->is_snapping && !player->was_snapping) ||
      (player->is_dropping && !player->was_dropping))
  {
    if (!CheckSaveEngineSnapshotToList())
      return;

    player->was_moving = FALSE;
    player->was_snapping = TRUE;
    player->was_dropping = TRUE;
  }
  else
  {
    if (player->is_moving)
      player->was_moving = TRUE;

    if (!player->is_snapping)
      player->was_snapping = FALSE;

    if (!player->is_dropping)
      player->was_dropping = FALSE;
  }

  static struct MouseActionInfo mouse_action_last = { 0 };
  struct MouseActionInfo mouse_action = player->effective_mouse_action;
  boolean new_released = (!mouse_action.button && mouse_action_last.button);

  if (new_released)
    CheckSaveEngineSnapshotToList();

  mouse_action_last = mouse_action;
}

static void CheckSingleStepMode(struct PlayerInfo *player)
{
  if (tape.single_step && tape.recording && !tape.pausing)
  {
    // as it is called "single step mode", just return to pause mode when the
    // player stopped moving after one tile (or never starts moving at all)
    // (reverse logic needed here in case single step mode used in team mode)
    if (player->is_moving ||
	player->is_pushing ||
	player->is_dropping_pressed ||
	player->effective_mouse_action.button)
      game.enter_single_step_mode = FALSE;
  }

  CheckSaveEngineSnapshot(player);
}

static byte PlayerActions(struct PlayerInfo *player, byte player_action)
{
  int left	= player_action & JOY_LEFT;
  int right	= player_action & JOY_RIGHT;
  int up	= player_action & JOY_UP;
  int down	= player_action & JOY_DOWN;
  int button1	= player_action & JOY_BUTTON_1;
  int button2	= player_action & JOY_BUTTON_2;
  int dx	= (left ? -1 : right ? 1 : 0);
  int dy	= (up   ? -1 : down  ? 1 : 0);

  if (!player->active || tape.pausing)
    return 0;

  if (player_action)
  {
    if (button1)
      SnapField(player, dx, dy);
    else
    {
      if (button2)
	DropElement(player);

      MovePlayer(player, dx, dy);
    }

    CheckSingleStepMode(player);

    SetPlayerWaiting(player, FALSE);

    return player_action;
  }
  else
  {
    // no actions for this player (no input at player's configured device)

    DigField(player, 0, 0, 0, 0, 0, 0, DF_NO_PUSH);
    SnapField(player, 0, 0);
    CheckGravityMovementWhenNotMoving(player);

    if (player->MovPos == 0)
      SetPlayerWaiting(player, TRUE);

    if (player->MovPos == 0)	// needed for tape.playing
      player->is_moving = FALSE;

    player->is_dropping = FALSE;
    player->is_dropping_pressed = FALSE;
    player->drop_pressed_delay = 0;

    CheckSingleStepMode(player);

    return 0;
  }
}

static void SetMouseActionFromTapeAction(struct MouseActionInfo *mouse_action,
					 byte *tape_action)
{
  if (!tape.use_mouse_actions)
    return;

  mouse_action->lx     = tape_action[TAPE_ACTION_LX];
  mouse_action->ly     = tape_action[TAPE_ACTION_LY];
  mouse_action->button = tape_action[TAPE_ACTION_BUTTON];
}

static void SetTapeActionFromMouseAction(byte *tape_action,
					 struct MouseActionInfo *mouse_action)
{
  if (!tape.use_mouse_actions)
    return;

  tape_action[TAPE_ACTION_LX]     = mouse_action->lx;
  tape_action[TAPE_ACTION_LY]     = mouse_action->ly;
  tape_action[TAPE_ACTION_BUTTON] = mouse_action->button;
}

static void CheckLevelSolved(void)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    if (game_bd.level_solved &&
	!game_bd.game_over)				// game won
    {
      LevelSolved();

      game_bd.game_over = TRUE;

      game.all_players_gone = TRUE;
    }

    if (game_bd.game_over)				// game lost
      game.all_players_gone = TRUE;
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    if (game_em.level_solved &&
	!game_em.game_over)				// game won
    {
      LevelSolved();

      game_em.game_over = TRUE;

      game.all_players_gone = TRUE;
    }

    if (game_em.game_over)				// game lost
      game.all_players_gone = TRUE;
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    if (game_sp.level_solved &&
	!game_sp.game_over)				// game won
    {
      LevelSolved();

      game_sp.game_over = TRUE;

      game.all_players_gone = TRUE;
    }

    if (game_sp.game_over)				// game lost
      game.all_players_gone = TRUE;
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    if (game_mm.level_solved &&
	!game_mm.game_over)				// game won
    {
      LevelSolved();

      game_mm.game_over = TRUE;

      game.all_players_gone = TRUE;
    }

    if (game_mm.game_over)				// game lost
      game.all_players_gone = TRUE;
  }
}

static void PlayTimeoutSound(int seconds_left)
{
  // will be played directly by BD engine (for classic bonus time sounds)
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD && checkBonusTime_BD())
    return;

  // try to use individual "running out of time" sound for each second left
  int sound = SND_GAME_RUNNING_OUT_OF_TIME_0 - seconds_left;

  // if special sound per second not defined, use default sound
  if (getSoundInfoEntryFilename(sound) == NULL)
    sound = SND_GAME_RUNNING_OUT_OF_TIME;

  // if out of time, but player still alive, play special "timeout" sound, if defined
  if (seconds_left == 0 && !checkGameFailed())
    if (getSoundInfoEntryFilename(SND_GAME_TIMEOUT) != NULL)
      sound = SND_GAME_TIMEOUT;

  PlaySound(sound);
}

static void CheckLevelTime_StepCounter(void)
{
  int i;

  TimePlayed++;

  if (TimeLeft > 0)
  {
    TimeLeft--;

    if (TimeLeft <= 10 && game.time_limit && !game.LevelSolved)
      PlayTimeoutSound(TimeLeft);

    game_panel_controls[GAME_PANEL_TIME].value = TimeLeft;

    DisplayGameControlValues();

    if (!TimeLeft && game.time_limit && !game.LevelSolved)
      for (i = 0; i < MAX_PLAYERS; i++)
	KillPlayer(&stored_player[i]);
  }
  else if (game.no_level_time_limit && !game.all_players_gone)
  {
    game_panel_controls[GAME_PANEL_TIME].value = TimePlayed;

    DisplayGameControlValues();
  }
}

static void CheckLevelTime(void)
{
  int frames_per_second = FRAMES_PER_SECOND;
  int i;

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    // level time may be running slower in native BD engine
    frames_per_second = getFramesPerSecond_BD();

    // if native engine time changed, force main engine time change
    if (getTimeLeft_BD() < TimeLeft)
      TimeFrames = frames_per_second;

    // if last second running, wait for native engine time to exactly reach zero
    if (getTimeLeft_BD() == 1 && TimeLeft == 1)
      TimeFrames = frames_per_second - 1;

    // needed to store final time after solving game (before counting down remaining time)
    SetTimeFrames_BD(TimePlayed * FRAMES_PER_SECOND + TimeFrames);
  }

  if (TimeFrames >= frames_per_second)
  {
    TimeFrames = 0;

    for (i = 0; i < MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];

      if (SHIELD_ON(player))
      {
	player->shield_normal_time_left--;

	if (player->shield_deadly_time_left > 0)
	  player->shield_deadly_time_left--;
      }
    }

    if (!game.LevelSolved && !level.use_step_counter)
    {
      TimePlayed++;

      if (TimeLeft > 0)
      {
	TimeLeft--;

	if (TimeLeft <= 10 && game.time_limit)
	  PlayTimeoutSound(TimeLeft);

	/* this does not make sense: game_panel_controls[GAME_PANEL_TIME].value
	   is reset from other values in UpdateGameDoorValues() -- FIX THIS */

	game_panel_controls[GAME_PANEL_TIME].value = TimeLeft;

	if (!TimeLeft && game.time_limit)
	{
	  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
	  {
	    if (game_bd.game->cave->player_state == GD_PL_LIVING)
	      game_bd.game->cave->player_state = GD_PL_TIMEOUT;
	  }
	  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
	  {
	    game_em.lev->killed_out_of_time = TRUE;
	  }
	  else
	  {
	    for (i = 0; i < MAX_PLAYERS; i++)
	      KillPlayer(&stored_player[i]);
	  }
	}
      }
      else if (game.no_level_time_limit && !game.all_players_gone)
      {
	game_panel_controls[GAME_PANEL_TIME].value = TimePlayed;
      }

      game_em.lev->time = (game.no_level_time_limit ? TimePlayed : TimeLeft);
    }
  }

  if (TapeTimeFrames >= FRAMES_PER_SECOND)
  {
    TapeTimeFrames = 0;
    TapeTime++;

    if (tape.recording || tape.playing)
      DrawVideoDisplay(VIDEO_STATE_TIME_ON, TapeTime);
  }

  if (tape.recording || tape.playing)
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, FrameCounter);

  UpdateAndDisplayGameControlValues();
}

void AdvanceFrameAndPlayerCounters(int player_nr)
{
  int i;

  // handle game and tape time differently for native BD game engine

  // tape time is running in native BD engine even if player is not hatched yet
  if (!checkGameRunning())
    return;

  // advance frame counters (global frame counter and tape time frame counter)
  FrameCounter++;
  TapeTimeFrames++;

  // level time is running in native BD engine after player is being hatched
  if (!checkGamePlaying())
    return;

  // advance time frame counter (used to control available time to solve level)
  TimeFrames++;

  // advance player counters (counters for move delay, move animation etc.)
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    boolean advance_player_counters = (player_nr == -1 || player_nr == i);
    int move_delay_value = stored_player[i].move_delay_value;
    int move_frames = MOVE_DELAY_NORMAL_SPEED / move_delay_value;

    if (!advance_player_counters)	// not all players may be affected
      continue;

    if (move_frames == 0)	// less than one move per game frame
    {
      int stepsize = TILEX / move_delay_value;
      int delay = move_delay_value / MOVE_DELAY_NORMAL_SPEED;
      int count = (stored_player[i].is_moving ?
		   ABS(stored_player[i].MovPos) / stepsize : FrameCounter);

      if (count % delay == 0)
	move_frames = 1;
    }

    stored_player[i].Frame += move_frames;

    if (stored_player[i].MovPos != 0)
      stored_player[i].StepFrame += move_frames;

    if (stored_player[i].move_delay > 0)
      stored_player[i].move_delay--;

    // due to bugs in previous versions, counter must count up, not down
    if (stored_player[i].push_delay != -1)
      stored_player[i].push_delay++;

    if (stored_player[i].drop_delay > 0)
      stored_player[i].drop_delay--;

    if (stored_player[i].is_dropping_pressed)
      stored_player[i].drop_pressed_delay++;
  }
}

void AdvanceFrameCounter(void)
{
  FrameCounter++;
}

void AdvanceGfxFrame(void)
{
  int x, y;

  SCAN_PLAYFIELD(x, y)
  {
    GfxFrame[x][y]++;
  }
}

static void HandleMouseAction(struct MouseActionInfo *mouse_action,
			      struct MouseActionInfo *mouse_action_last)
{
  if (mouse_action->button)
  {
    int new_button = (mouse_action->button && mouse_action_last->button == 0);
    int ch_button = CH_SIDE_FROM_BUTTON(mouse_action->button);
    int x = mouse_action->lx;
    int y = mouse_action->ly;
    int element = Tile[x][y];

    if (new_button)
    {
      CheckElementChangeByMouse(x, y, element, CE_CLICKED_BY_MOUSE, ch_button);
      CheckTriggeredElementChangeByMouse(x, y, element, CE_MOUSE_CLICKED_ON_X,
					 ch_button);
    }

    CheckElementChangeByMouse(x, y, element, CE_PRESSED_BY_MOUSE, ch_button);
    CheckTriggeredElementChangeByMouse(x, y, element, CE_MOUSE_PRESSED_ON_X,
				       ch_button);

    if (level.use_step_counter)
    {
      boolean counted_click = FALSE;

      // element clicked that can change when clicked/pressed
      if (CAN_CHANGE_OR_HAS_ACTION(element) &&
	  (HAS_ANY_CHANGE_EVENT(element, CE_CLICKED_BY_MOUSE) ||
	   HAS_ANY_CHANGE_EVENT(element, CE_PRESSED_BY_MOUSE)))
	counted_click = TRUE;

      // element clicked that can trigger change when clicked/pressed
      if (trigger_events[element][CE_MOUSE_CLICKED_ON_X] ||
	  trigger_events[element][CE_MOUSE_PRESSED_ON_X])
	counted_click = TRUE;

      if (new_button && counted_click)
	CheckLevelTime_StepCounter();
    }
  }
}

void StartGameActions(boolean init_network_game, boolean record_tape,
		      int random_seed)
{
  unsigned int new_random_seed = InitRND(random_seed);

  if (record_tape)
    TapeStartRecording(new_random_seed);

  if (setup.auto_pause_on_start && !tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_MANUAL);

  if (init_network_game)
  {
    SendToServer_LevelFile();
    SendToServer_StartPlaying();

    return;
  }

  InitGame();
}

static void GameActionsExt(void)
{
#if 0
  static unsigned int game_frame_delay = 0;
#endif
  unsigned int game_frame_delay_value;
  byte *recorded_player_action;
  byte summarized_player_action = 0;
  byte tape_action[MAX_TAPE_ACTIONS] = { 0 };
  int i;

  // detect endless loops, caused by custom element programming
  if (recursion_loop_detected && recursion_loop_depth == 0)
  {
    char *message = getStringCat3("Internal Error! Element ",
				  EL_NAME(recursion_loop_element),
				  " caused endless loop! Quit the game?");

    Warn("element '%s' caused endless loop in game engine",
	 EL_NAME(recursion_loop_element));

    RequestQuitGameExt(program.headless, level_editor_test_game, message);

    recursion_loop_detected = FALSE;	// if game should be continued

    free(message);

    return;
  }

  if (game.restart_level)
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);

  CheckLevelSolved();

  if (game.LevelSolved && !game.LevelSolved_GameEnd)
  {
    // handle winning game until completely finished and game ended
    if (GameWon())
      return;
  }

  if (game.all_players_gone && !TAPE_IS_STOPPED(tape))
    TapeStop();

  if (game_status != GAME_MODE_PLAYING)		// status might have changed
    return;

  game_frame_delay_value =
    (tape.playing && tape.fast_forward ? FfwdFrameDelay : GameFrameDelay);

  if (tape.playing && tape.warp_forward && !tape.pausing)
    game_frame_delay_value = 0;

  SetVideoFrameDelay(game_frame_delay_value);

  // (de)activate virtual buttons depending on current game status
  if (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS))
  {
    if (game.all_players_gone)	// if no players there to be controlled anymore
      SetOverlayActive(FALSE);
    else if (!tape.playing)	// if game continues after tape stopped playing
      SetOverlayActive(TRUE);
  }

#if 0
#if 0
  // ---------- main game synchronization point ----------

  int skip = WaitUntilDelayReached(&game_frame_delay, game_frame_delay_value);

  Debug("game:playing:skip", "skip == %d", skip);

#else
  // ---------- main game synchronization point ----------

  WaitUntilDelayReached(&game_frame_delay, game_frame_delay_value);
#endif
#endif

  if (network_playing && !network_player_action_received)
  {
    // try to get network player actions in time

    // last chance to get network player actions without main loop delay
    HandleNetworking();

    // game was quit by network peer
    if (game_status != GAME_MODE_PLAYING)
      return;

    // check if network player actions still missing and game still running
    if (!network_player_action_received && !checkGameEnded())
      return;		// failed to get network player actions in time

    // do not yet reset "network_player_action_received" (for tape.pausing)
  }

  if (tape.pausing)
    return;

  // at this point we know that we really continue executing the game

  network_player_action_received = FALSE;

  // when playing tape, read previously recorded player input from tape data
  recorded_player_action = (tape.playing ? TapePlayAction() : NULL);

  local_player->effective_mouse_action = local_player->mouse_action;

  if (recorded_player_action != NULL)
    SetMouseActionFromTapeAction(&local_player->effective_mouse_action,
				 recorded_player_action);

  // TapePlayAction() may return NULL when toggling to "pause before death"
  if (tape.pausing)
    return;

  if (tape.set_centered_player)
  {
    game.centered_player_nr_next = tape.centered_player_nr_next;
    game.set_centered_player = TRUE;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    summarized_player_action |= stored_player[i].action;

    if (!network_playing && (game.team_mode || tape.playing))
      stored_player[i].effective_action = stored_player[i].action;
  }

  if (network_playing && !checkGameEnded())
    SendToServer_MovePlayer(summarized_player_action);

  // summarize all actions at local players mapped input device position
  // (this allows using different input devices in single player mode)
  if (!network.enabled && !game.team_mode)
    stored_player[map_player_action[local_player->index_nr]].effective_action =
      summarized_player_action;

  // summarize all actions at centered player in local team mode
  if (tape.recording &&
      setup.team_mode && !network.enabled &&
      setup.input_on_focus &&
      game.centered_player_nr != -1)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      stored_player[map_player_action[i]].effective_action =
	(i == game.centered_player_nr ? summarized_player_action : 0);
  }

  if (recorded_player_action != NULL)
    for (i = 0; i < MAX_PLAYERS; i++)
      stored_player[i].effective_action = recorded_player_action[i];

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    tape_action[i] = stored_player[i].effective_action;

    /* (this may happen in the RND game engine if a player was not present on
       the playfield on level start, but appeared later from a custom element */
    if (setup.team_mode &&
	tape.recording &&
	tape_action[i] &&
	!tape.player_participates[i])
      tape.player_participates[i] = TRUE;
  }

  SetTapeActionFromMouseAction(tape_action,
			       &local_player->effective_mouse_action);

  // only record actions from input devices, but not programmed actions
  if (tape.recording)
    TapeRecordAction(tape_action);

  // remember if game was played (especially after tape stopped playing)
  if (!tape.playing && summarized_player_action && !checkGameFailed())
    game.GamePlayed = TRUE;

#if USE_NEW_PLAYER_ASSIGNMENTS
  // !!! also map player actions in single player mode !!!
  // if (game.team_mode)
  if (1)
  {
    byte mapped_action[MAX_PLAYERS];

#if DEBUG_PLAYER_ACTIONS
    for (i = 0; i < MAX_PLAYERS; i++)
      DebugContinued("", "%d, ", stored_player[i].effective_action);
#endif

    for (i = 0; i < MAX_PLAYERS; i++)
      mapped_action[i] = stored_player[map_player_action[i]].effective_action;

    for (i = 0; i < MAX_PLAYERS; i++)
      stored_player[i].effective_action = mapped_action[i];

#if DEBUG_PLAYER_ACTIONS
    DebugContinued("", "=> ");
    for (i = 0; i < MAX_PLAYERS; i++)
      DebugContinued("", "%d, ", stored_player[i].effective_action);
    DebugContinued("game:playing:player", "\n");
#endif
  }
#if DEBUG_PLAYER_ACTIONS
  else
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      DebugContinued("", "%d, ", stored_player[i].effective_action);
    DebugContinued("game:playing:player", "\n");
  }
#endif
#endif

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    // allow engine snapshot in case of changed movement attempt
    if ((game.snapshot.last_action[i] & KEY_MOTION) !=
	(stored_player[i].effective_action & KEY_MOTION))
      game.snapshot.changed_action = TRUE;

    // allow engine snapshot in case of snapping/dropping attempt
    if ((game.snapshot.last_action[i] & KEY_BUTTON) == 0 &&
	(stored_player[i].effective_action & KEY_BUTTON) != 0)
      game.snapshot.changed_action = TRUE;

    game.snapshot.last_action[i] = stored_player[i].effective_action;
  }

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    GameActions_BD_Main();
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    GameActions_EM_Main();
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    GameActions_SP_Main();
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    GameActions_MM_Main();
  }
  else
  {
    GameActions_RND_Main();
  }

  BlitScreenToBitmap(backbuffer);

  CheckLevelSolved();
  CheckLevelTime();

  AdvanceFrameAndPlayerCounters(-1);	// advance counters for all players

  if (global.show_frames_per_second)
  {
    static unsigned int fps_counter = 0;
    static int fps_frames = 0;
    unsigned int fps_delay_ms = Counter() - fps_counter;

    fps_frames++;

    if (fps_delay_ms >= 500)	// calculate FPS every 0.5 seconds
    {
      global.frames_per_second = 1000 * (float)fps_frames / fps_delay_ms;

      fps_frames = 0;
      fps_counter = Counter();

      // always draw FPS to screen after FPS value was updated
      redraw_mask |= REDRAW_FPS;
    }

    // only draw FPS if no screen areas are deactivated (invisible warp mode)
    if (GetDrawDeactivationMask() == REDRAW_NONE)
      redraw_mask |= REDRAW_FPS;
  }
}

static void GameActions_CheckSaveEngineSnapshot(void)
{
  if (!game.snapshot.save_snapshot)
    return;

  // clear flag for saving snapshot _before_ saving snapshot
  game.snapshot.save_snapshot = FALSE;

  SaveEngineSnapshotToList();
}

void GameActions(void)
{
  GameActionsExt();

  GameActions_CheckSaveEngineSnapshot();
}

void GameActions_BD_Main(void)
{
  byte effective_action[MAX_PLAYERS];
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    effective_action[i] = stored_player[i].effective_action;

  GameActions_BD(effective_action);
}

void GameActions_EM_Main(void)
{
  byte effective_action[MAX_PLAYERS];
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    effective_action[i] = stored_player[i].effective_action;

  GameActions_EM(effective_action);
}

void GameActions_SP_Main(void)
{
  byte effective_action[MAX_PLAYERS];
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    effective_action[i] = stored_player[i].effective_action;

  GameActions_SP(effective_action);

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (stored_player[i].force_dropping)
      stored_player[i].action |= KEY_BUTTON_DROP;

    stored_player[i].force_dropping = FALSE;
  }
}

void GameActions_MM_Main(void)
{
  AdvanceGfxFrame();

  GameActions_MM(local_player->effective_mouse_action);
}

void GameActions_RND_Main(void)
{
  GameActions_RND();
}

void GameActions_RND(void)
{
  static struct MouseActionInfo mouse_action_last = { 0 };
  struct MouseActionInfo mouse_action = local_player->effective_mouse_action;
  int magic_wall_x = 0, magic_wall_y = 0;
  int i, x, y, element, graphic, last_gfx_frame;

  InitPlayfieldScanModeVars();

  if (game.engine_version >= VERSION_IDENT(3,2,0,7))
  {
    SCAN_PLAYFIELD(x, y)
    {
      ChangeCount[x][y] = 0;
      ChangeEvent[x][y] = -1;
    }
  }

  if (game.set_centered_player)
  {
    boolean all_players_fit_to_screen = checkIfAllPlayersFitToScreen_RND();

    // switching to "all players" only possible if all players fit to screen
    if (game.centered_player_nr_next == -1 && !all_players_fit_to_screen)
    {
      game.centered_player_nr_next = game.centered_player_nr;
      game.set_centered_player = FALSE;
    }

    // do not switch focus to non-existing (or non-active) player
    if (game.centered_player_nr_next >= 0 &&
	!stored_player[game.centered_player_nr_next].active)
    {
      game.centered_player_nr_next = game.centered_player_nr;
      game.set_centered_player = FALSE;
    }
  }

  if (game.set_centered_player &&
      ScreenMovPos == 0)	// screen currently aligned at tile position
  {
    int sx, sy;

    if (game.centered_player_nr_next == -1)
    {
      setScreenCenteredToAllPlayers(&sx, &sy);
    }
    else
    {
      sx = stored_player[game.centered_player_nr_next].jx;
      sy = stored_player[game.centered_player_nr_next].jy;
    }

    game.centered_player_nr = game.centered_player_nr_next;
    game.set_centered_player = FALSE;

    DrawRelocateScreen(0, 0, sx, sy, TRUE, setup.quick_switch);
    DrawGameDoorValues();
  }

  // check single step mode (set flag and clear again if any player is active)
  game.enter_single_step_mode =
    (tape.single_step && tape.recording && !tape.pausing);

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int actual_player_action = stored_player[i].effective_action;

#if 1
    /* !!! THIS BREAKS THE FOLLOWING TAPES: !!!
       - rnd_equinox_tetrachloride 048
       - rnd_equinox_tetrachloride_ii 096
       - rnd_emanuel_schmieg 002
       - doctor_sloan_ww 001, 020
    */
    if (stored_player[i].MovPos == 0)
      CheckGravityMovement(&stored_player[i]);
#endif

    // overwrite programmed action with tape action
    if (stored_player[i].programmed_action)
      actual_player_action = stored_player[i].programmed_action;

    PlayerActions(&stored_player[i], actual_player_action);

    ScrollPlayer(&stored_player[i], SCROLL_GO_ON);
  }

  // single step pause mode may already have been toggled by "ScrollPlayer()"
  if (game.enter_single_step_mode && !tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  ScrollScreen(NULL, SCROLL_GO_ON);

  /* for backwards compatibility, the following code emulates a fixed bug that
     occured when pushing elements (causing elements that just made their last
     pushing step to already (if possible) make their first falling step in the
     same game frame, which is bad); this code is also needed to use the famous
     "spring push bug" which is used in older levels and might be wanted to be
     used also in newer levels, but in this case the buggy pushing code is only
     affecting the "spring" element and no other elements */

  if (game.engine_version < VERSION_IDENT(2,2,0,7) || level.use_spring_bug)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      struct PlayerInfo *player = &stored_player[i];
      int x = player->jx;
      int y = player->jy;

      if (player->active && player->is_pushing && player->is_moving &&
	  IS_MOVING(x, y) &&
	  (game.engine_version < VERSION_IDENT(2,2,0,7) ||
	   Tile[x][y] == EL_SPRING))
      {
	ContinueMoving(x, y);

	// continue moving after pushing (this is actually a bug)
	if (!IS_MOVING(x, y))
	  Stop[x][y] = FALSE;
      }
    }
  }

  SCAN_PLAYFIELD(x, y)
  {
    Last[x][y] = Tile[x][y];

    ChangeCount[x][y] = 0;
    ChangeEvent[x][y] = -1;

    // this must be handled before main playfield loop
    if (Tile[x][y] == EL_PLAYER_IS_LEAVING)
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y] <= 0)
	RemoveField(x, y);
    }

    if (Tile[x][y] == EL_ELEMENT_SNAPPING)
    {
      MovDelay[x][y]--;
      if (MovDelay[x][y] <= 0)
      {
	int element = Store[x][y];
	int move_direction = MovDir[x][y];
	int player_index_bit = Store2[x][y];

	Store[x][y] = 0;
	Store2[x][y] = 0;

	RemoveField(x, y);
	TEST_DrawLevelField(x, y);

	TestFieldAfterSnapping(x, y, element, move_direction, player_index_bit);

	if (IS_ENVELOPE(element))
	  local_player->show_envelope = element;
      }
    }

#if DEBUG
    if (ChangePage[x][y] != -1 && ChangeDelay[x][y] != 1)
    {
      Debug("game:playing:GameActions_RND", "x = %d, y = %d: ChangePage != -1",
	    x, y);
      Debug("game:playing:GameActions_RND", "This should never happen!");

      ChangePage[x][y] = -1;
    }
#endif

    Stop[x][y] = FALSE;
    if (WasJustMoving[x][y] > 0)
      WasJustMoving[x][y]--;
    if (WasJustFalling[x][y] > 0)
      WasJustFalling[x][y]--;
    if (CheckCollision[x][y] > 0)
      CheckCollision[x][y]--;
    if (CheckImpact[x][y] > 0)
      CheckImpact[x][y]--;

    GfxFrame[x][y]++;

    /* reset finished pushing action (not done in ContinueMoving() to allow
       continuous pushing animation for elements with zero push delay) */
    if (GfxAction[x][y] == ACTION_PUSHING && !IS_MOVING(x, y))
    {
      ResetGfxAnimation(x, y);
      TEST_DrawLevelField(x, y);
    }

#if DEBUG
    if (IS_BLOCKED(x, y))
    {
      int oldx, oldy;

      Blocked2Moving(x, y, &oldx, &oldy);
      if (!IS_MOVING(oldx, oldy))
      {
	Debug("game:playing:GameActions_RND", "(BLOCKED => MOVING) context corrupted!");
	Debug("game:playing:GameActions_RND", "BLOCKED: x = %d, y = %d", x, y);
	Debug("game:playing:GameActions_RND", "!MOVING: oldx = %d, oldy = %d", oldx, oldy);
	Debug("game:playing:GameActions_RND", "This should never happen!");
      }
    }
#endif
  }

  HandleMouseAction(&mouse_action, &mouse_action_last);

  SCAN_PLAYFIELD(x, y)
  {
    element = Tile[x][y];
    graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);
    last_gfx_frame = GfxFrame[x][y];

    if (element == EL_EMPTY)
      graphic = el2img(GfxElementEmpty[x][y]);

    ResetGfxFrame(x, y);

    if (GfxFrame[x][y] != last_gfx_frame && !Stop[x][y])
      DrawLevelGraphicAnimation(x, y, graphic);

    if (ANIM_MODE(graphic) == ANIM_RANDOM &&
	IS_NEXT_FRAME(GfxFrame[x][y], graphic))
      ResetRandomAnimationValue(x, y);

    SetRandomAnimationValue(x, y);

    PlayLevelSoundActionIfLoop(x, y, GfxAction[x][y]);

    if (IS_INACTIVE(element))
    {
      if (IS_ANIMATED(graphic))
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

      continue;
    }

    // this may take place after moving, so 'element' may have changed
    if (IS_CHANGING(x, y) &&
	(game.engine_version < VERSION_IDENT(3,0,7,1) || !Stop[x][y]))
    {
      int page = element_info[element].event_page_nr[CE_DELAY];

      HandleElementChange(x, y, page);

      element = Tile[x][y];
      graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);
    }

    CheckNextToConditions(x, y);

    if (!IS_MOVING(x, y) && (CAN_FALL(element) || CAN_MOVE(element)))
    {
      StartMoving(x, y);

      element = Tile[x][y];
      graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

      if (IS_ANIMATED(graphic) &&
	  !IS_MOVING(x, y) &&
	  !Stop[x][y])
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

      if (IS_GEM(element) || element == EL_SP_INFOTRON)
	TEST_DrawTwinkleOnField(x, y);
    }
    else if (element == EL_ACID)
    {
      if (!Stop[x][y])
	DrawLevelGraphicAnimationIfNeeded(x, y, graphic);
    }
    else if ((element == EL_EXIT_OPEN ||
	      element == EL_EM_EXIT_OPEN ||
	      element == EL_SP_EXIT_OPEN ||
	      element == EL_STEEL_EXIT_OPEN ||
	      element == EL_EM_STEEL_EXIT_OPEN ||
	      element == EL_SP_TERMINAL ||
	      element == EL_SP_TERMINAL_ACTIVE ||
	      element == EL_EXTRA_TIME ||
	      element == EL_SHIELD_NORMAL ||
	      element == EL_SHIELD_DEADLY) &&
	     IS_ANIMATED(graphic))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);
    else if (IS_MOVING(x, y))
      ContinueMoving(x, y);
    else if (IS_ACTIVE_BOMB(element))
      CheckDynamite(x, y);
    else if (element == EL_AMOEBA_GROWING)
      AmoebaGrowing(x, y);
    else if (element == EL_AMOEBA_SHRINKING)
      AmoebaShrinking(x, y);

#if !USE_NEW_AMOEBA_CODE
    else if (IS_AMOEBALIVE(element))
      AmoebaReproduce(x, y);
#endif

    else if (element == EL_GAME_OF_LIFE || element == EL_BIOMAZE)
      Life(x, y);
    else if (element == EL_EXIT_CLOSED)
      CheckExit(x, y);
    else if (element == EL_EM_EXIT_CLOSED)
      CheckExitEM(x, y);
    else if (element == EL_STEEL_EXIT_CLOSED)
      CheckExitSteel(x, y);
    else if (element == EL_EM_STEEL_EXIT_CLOSED)
      CheckExitSteelEM(x, y);
    else if (element == EL_SP_EXIT_CLOSED)
      CheckExitSP(x, y);
    else if (element == EL_EXPANDABLE_WALL_GROWING ||
	     element == EL_EXPANDABLE_STEELWALL_GROWING)
      WallGrowing(x, y);
    else if (element == EL_EXPANDABLE_WALL ||
	     element == EL_EXPANDABLE_WALL_HORIZONTAL ||
	     element == EL_EXPANDABLE_WALL_VERTICAL ||
	     element == EL_EXPANDABLE_WALL_ANY ||
	     element == EL_BD_EXPANDABLE_WALL ||
	     element == EL_EXPANDABLE_STEELWALL_HORIZONTAL ||
	     element == EL_EXPANDABLE_STEELWALL_VERTICAL ||
	     element == EL_EXPANDABLE_STEELWALL_ANY)
      CheckWallGrowing(x, y);
    else if (element == EL_FLAMES)
      CheckForDragon(x, y);
    else if (element == EL_EXPLOSION)
      ;	// drawing of correct explosion animation is handled separately
    else if (element == EL_ELEMENT_SNAPPING ||
	     element == EL_DIAGONAL_SHRINKING ||
	     element == EL_DIAGONAL_GROWING)
    {
      graphic = el_act_dir2img(GfxElement[x][y], GfxAction[x][y], GfxDir[x][y]);

      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);
    }
    else if (IS_ANIMATED(graphic) && !IS_CHANGING(x, y))
      DrawLevelGraphicAnimationIfNeeded(x, y, graphic);

    if (IS_BELT_ACTIVE(element))
      PlayLevelSoundAction(x, y, ACTION_ACTIVE);

    if (game.magic_wall_active)
    {
      int jx = local_player->jx, jy = local_player->jy;

      // play the element sound at the position nearest to the player
      if ((element == EL_MAGIC_WALL_FULL ||
	   element == EL_MAGIC_WALL_ACTIVE ||
	   element == EL_MAGIC_WALL_EMPTYING ||
	   element == EL_BD_MAGIC_WALL_FULL ||
	   element == EL_BD_MAGIC_WALL_ACTIVE ||
	   element == EL_BD_MAGIC_WALL_EMPTYING ||
	   element == EL_DC_MAGIC_WALL_FULL ||
	   element == EL_DC_MAGIC_WALL_ACTIVE ||
	   element == EL_DC_MAGIC_WALL_EMPTYING) &&
	  ABS(x - jx) + ABS(y - jy) <
	  ABS(magic_wall_x - jx) + ABS(magic_wall_y - jy))
      {
	magic_wall_x = x;
	magic_wall_y = y;
      }
    }
  }

#if USE_NEW_AMOEBA_CODE
  // new experimental amoeba growth stuff
  if (!(FrameCounter % 8))
  {
    static unsigned int random = 1684108901;

    for (i = 0; i < level.amoeba_speed * 28 / 8; i++)
    {
      x = RND(lev_fieldx);
      y = RND(lev_fieldy);
      element = Tile[x][y];

      if (!IS_PLAYER(x, y) &&
	  (element == EL_EMPTY ||
	   CAN_GROW_INTO(element) ||
	   element == EL_QUICKSAND_EMPTY ||
	   element == EL_QUICKSAND_FAST_EMPTY ||
	   element == EL_ACID_SPLASH_LEFT ||
	   element == EL_ACID_SPLASH_RIGHT))
      {
	if ((IN_LEV_FIELD(x, y - 1) && Tile[x][y - 1] == EL_AMOEBA_WET) ||
	    (IN_LEV_FIELD(x - 1, y) && Tile[x - 1][y] == EL_AMOEBA_WET) ||
	    (IN_LEV_FIELD(x + 1, y) && Tile[x + 1][y] == EL_AMOEBA_WET) ||
	    (IN_LEV_FIELD(x, y + 1) && Tile[x][y + 1] == EL_AMOEBA_WET))
	  Tile[x][y] = EL_AMOEBA_DROP;
      }

      random = random * 129 + 1;
    }
  }
#endif

  game.explosions_delayed = FALSE;

  SCAN_PLAYFIELD(x, y)
  {
    element = Tile[x][y];

    if (ExplodeField[x][y])
      Explode(x, y, EX_PHASE_START, ExplodeField[x][y]);
    else if (element == EL_EXPLOSION)
      Explode(x, y, ExplodePhase[x][y], EX_TYPE_NORMAL);

    ExplodeField[x][y] = EX_TYPE_NONE;
  }

  game.explosions_delayed = TRUE;

  if (game.magic_wall_active)
  {
    if (!(game.magic_wall_time_left % 4))
    {
      int element = Tile[magic_wall_x][magic_wall_y];

      if (element == EL_BD_MAGIC_WALL_FULL ||
	  element == EL_BD_MAGIC_WALL_ACTIVE ||
	  element == EL_BD_MAGIC_WALL_EMPTYING)
	PlayLevelSound(magic_wall_x, magic_wall_y, SND_BD_MAGIC_WALL_ACTIVE);
      else if (element == EL_DC_MAGIC_WALL_FULL ||
	       element == EL_DC_MAGIC_WALL_ACTIVE ||
	       element == EL_DC_MAGIC_WALL_EMPTYING)
	PlayLevelSound(magic_wall_x, magic_wall_y, SND_DC_MAGIC_WALL_ACTIVE);
      else
	PlayLevelSound(magic_wall_x, magic_wall_y, SND_MAGIC_WALL_ACTIVE);
    }

    if (game.magic_wall_time_left > 0)
    {
      game.magic_wall_time_left--;

      if (!game.magic_wall_time_left)
      {
	SCAN_PLAYFIELD(x, y)
	{
	  element = Tile[x][y];

	  if (element == EL_MAGIC_WALL_ACTIVE ||
	      element == EL_MAGIC_WALL_FULL)
	  {
	    Tile[x][y] = EL_MAGIC_WALL_DEAD;
	    TEST_DrawLevelField(x, y);
	  }
	  else if (element == EL_BD_MAGIC_WALL_ACTIVE ||
		   element == EL_BD_MAGIC_WALL_FULL)
	  {
	    Tile[x][y] = EL_BD_MAGIC_WALL_DEAD;
	    TEST_DrawLevelField(x, y);
	  }
	  else if (element == EL_DC_MAGIC_WALL_ACTIVE ||
		   element == EL_DC_MAGIC_WALL_FULL)
	  {
	    Tile[x][y] = EL_DC_MAGIC_WALL_DEAD;
	    TEST_DrawLevelField(x, y);
	  }
	}

	game.magic_wall_active = FALSE;
      }
    }
  }

  if (game.light_time_left > 0)
  {
    game.light_time_left--;

    if (game.light_time_left == 0)
      RedrawAllLightSwitchesAndInvisibleElements();
  }

  if (game.timegate_time_left > 0)
  {
    game.timegate_time_left--;

    if (game.timegate_time_left == 0)
      CloseAllOpenTimegates();
  }

  if (game.lenses_time_left > 0)
  {
    game.lenses_time_left--;

    if (game.lenses_time_left == 0)
      RedrawAllInvisibleElementsForLenses();
  }

  if (game.magnify_time_left > 0)
  {
    game.magnify_time_left--;

    if (game.magnify_time_left == 0)
      RedrawAllInvisibleElementsForMagnifier();
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (SHIELD_ON(player))
    {
      if (player->shield_deadly_time_left)
	PlayLevelSound(player->jx, player->jy, SND_SHIELD_DEADLY_ACTIVE);
      else if (player->shield_normal_time_left)
	PlayLevelSound(player->jx, player->jy, SND_SHIELD_NORMAL_ACTIVE);
    }
  }

#if USE_DELAYED_GFX_REDRAW
  SCAN_PLAYFIELD(x, y)
  {
    if (GfxRedraw[x][y] != GFX_REDRAW_NONE)
    {
      /* !!! PROBLEM: THIS REDRAWS THE PLAYFIELD _AFTER_ THE SCAN, BUT TILES
	 !!! MAY HAVE CHANGED AFTER BEING DRAWN DURING PLAYFIELD SCAN !!! */

      if (GfxRedraw[x][y] & GFX_REDRAW_TILE)
	DrawLevelField(x, y);

      if (GfxRedraw[x][y] & GFX_REDRAW_TILE_CRUMBLED)
	DrawLevelFieldCrumbled(x, y);

      if (GfxRedraw[x][y] & GFX_REDRAW_TILE_CRUMBLED_NEIGHBOURS)
	DrawLevelFieldCrumbledNeighbours(x, y);

      if (GfxRedraw[x][y] & GFX_REDRAW_TILE_TWINKLED)
	DrawTwinkleOnField(x, y);
    }

    GfxRedraw[x][y] = GFX_REDRAW_NONE;
  }
#endif

  DrawAllPlayers();
  PlayAllPlayersSound();

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    struct PlayerInfo *player = &stored_player[i];

    if (player->show_envelope != 0 && (!player->active ||
				       player->MovPos == 0))
    {
      ShowEnvelope(player->show_envelope - EL_ENVELOPE_1);

      player->show_envelope = 0;
    }
  }

  // use random number generator in every frame to make it less predictable
  if (game.engine_version >= VERSION_IDENT(3,1,1,0))
    RND(1);

  mouse_action_last = mouse_action;
}

static boolean AllPlayersInSight(struct PlayerInfo *player, int x, int y)
{
  int min_x = x, min_y = y, max_x = x, max_y = y;
  int scr_fieldx = getScreenFieldSizeX();
  int scr_fieldy = getScreenFieldSizeY();
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active || &stored_player[i] == player)
      continue;

    min_x = MIN(min_x, jx);
    min_y = MIN(min_y, jy);
    max_x = MAX(max_x, jx);
    max_y = MAX(max_y, jy);
  }

  return (max_x - min_x < scr_fieldx && max_y - min_y < scr_fieldy);
}

static boolean AllPlayersInVisibleScreen(void)
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int jx = stored_player[i].jx, jy = stored_player[i].jy;

    if (!stored_player[i].active)
      continue;

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
      return FALSE;
  }

  return TRUE;
}

void ScrollLevel(int dx, int dy)
{
  int scroll_offset = 2 * TILEX_VAR;
  int x, y;

  BlitBitmap(drawto_field, drawto_field,
	     FX + TILEX_VAR * (dx == -1) - scroll_offset,
	     FY + TILEY_VAR * (dy == -1) - scroll_offset,
	     SXSIZE - TILEX_VAR * (dx != 0) + 2 * scroll_offset,
	     SYSIZE - TILEY_VAR * (dy != 0) + 2 * scroll_offset,
	     FX + TILEX_VAR * (dx == 1) - scroll_offset,
	     FY + TILEY_VAR * (dy == 1) - scroll_offset);

  if (dx != 0)
  {
    x = (dx == 1 ? BX1 : BX2);
    for (y = BY1; y <= BY2; y++)
      DrawScreenField(x, y);
  }

  if (dy != 0)
  {
    y = (dy == 1 ? BY1 : BY2);
    for (x = BX1; x <= BX2; x++)
      DrawScreenField(x, y);
  }

  redraw_mask |= REDRAW_FIELD;
}

static boolean canFallDown(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  return (IN_LEV_FIELD(jx, jy + 1) &&
	  (IS_FREE(jx, jy + 1) ||
	   (Tile[jx][jy + 1] == EL_ACID && player->can_fall_into_acid)) &&
	  IS_WALKABLE_FROM(Tile[jx][jy], MV_DOWN) &&
	  !IS_WALKABLE_INSIDE(Tile[jx][jy]));
}

static boolean canPassField(int x, int y, int move_dir)
{
  int opposite_dir = MV_DIR_OPPOSITE(move_dir);
  int dx = ((move_dir & MV_LEFT) ? -1 : (move_dir & MV_RIGHT) ? +1 : 0);
  int dy = ((move_dir & MV_UP)   ? -1 : (move_dir & MV_DOWN)  ? +1 : 0);
  int nextx = x + dx;
  int nexty = y + dy;
  int element = Tile[x][y];

  return (IS_PASSABLE_FROM(element, opposite_dir) &&
	  !CAN_MOVE(element) &&
	  IN_LEV_FIELD(nextx, nexty) && !IS_PLAYER(nextx, nexty) &&
	  IS_WALKABLE_FROM(Tile[nextx][nexty], move_dir) &&
	  (level.can_pass_to_walkable || IS_FREE(nextx, nexty)));
}

static boolean canMoveToValidFieldWithGravity(int x, int y, int move_dir)
{
  int opposite_dir = MV_DIR_OPPOSITE(move_dir);
  int dx = ((move_dir & MV_LEFT) ? -1 : (move_dir & MV_RIGHT) ? +1 : 0);
  int dy = ((move_dir & MV_UP)   ? -1 : (move_dir & MV_DOWN)  ? +1 : 0);
  int newx = x + dx;
  int newy = y + dy;

  return (IN_LEV_FIELD(newx, newy) && !IS_FREE_OR_PLAYER(newx, newy) &&
	  IS_GRAVITY_REACHABLE(Tile[newx][newy]) &&
	  (IS_DIGGABLE(Tile[newx][newy]) ||
	   IS_WALKABLE_FROM(Tile[newx][newy], opposite_dir) ||
	   canPassField(newx, newy, move_dir)));
}

static void CheckGravityMovement(struct PlayerInfo *player)
{
  if (player->gravity && !player->programmed_action)
  {
    int move_dir_horizontal = player->effective_action & MV_HORIZONTAL;
    int move_dir_vertical   = player->effective_action & MV_VERTICAL;
    boolean player_is_snapping = (player->effective_action & JOY_BUTTON_1);
    int jx = player->jx, jy = player->jy;
    boolean player_is_moving_to_valid_field =
      (!player_is_snapping &&
       (canMoveToValidFieldWithGravity(jx, jy, move_dir_horizontal) ||
	canMoveToValidFieldWithGravity(jx, jy, move_dir_vertical)));
    boolean player_can_fall_down = canFallDown(player);

    if (player_can_fall_down &&
	!player_is_moving_to_valid_field)
      player->programmed_action = MV_DOWN;
  }
}

static void CheckGravityMovementWhenNotMoving(struct PlayerInfo *player)
{
  return CheckGravityMovement(player);

  if (player->gravity && !player->programmed_action)
  {
    int jx = player->jx, jy = player->jy;
    boolean field_under_player_is_free =
      (IN_LEV_FIELD(jx, jy + 1) && IS_FREE(jx, jy + 1));
    boolean player_is_standing_on_valid_field =
      (IS_WALKABLE_INSIDE(Tile[jx][jy]) ||
       (IS_WALKABLE(Tile[jx][jy]) &&
	!(element_info[Tile[jx][jy]].access_direction & MV_DOWN)));

    if (field_under_player_is_free && !player_is_standing_on_valid_field)
      player->programmed_action = MV_DOWN;
  }
}

/*
  MovePlayerOneStep()
  -----------------------------------------------------------------------------
  dx, dy:		direction (non-diagonal) to try to move the player to
  real_dx, real_dy:	direction as read from input device (can be diagonal)
*/

boolean MovePlayerOneStep(struct PlayerInfo *player,
			  int dx, int dy, int real_dx, int real_dy)
{
  int jx = player->jx, jy = player->jy;
  int new_jx = jx + dx, new_jy = jy + dy;
  int can_move;
  boolean player_can_move = !player->cannot_move;

  if (!player->active || (!dx && !dy))
    return MP_NO_ACTION;

  player->MovDir = (dx < 0 ? MV_LEFT :
		    dx > 0 ? MV_RIGHT :
		    dy < 0 ? MV_UP :
		    dy > 0 ? MV_DOWN :	MV_NONE);

  if (!IN_LEV_FIELD(new_jx, new_jy))
    return MP_NO_ACTION;

  if (!player_can_move)
  {
    if (player->MovPos == 0)
    {
      player->is_moving = FALSE;
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
      player->is_snapping = FALSE;
      player->is_pushing = FALSE;
    }
  }

  if (!network.enabled && game.centered_player_nr == -1 &&
      !AllPlayersInSight(player, new_jx, new_jy))
    return MP_NO_ACTION;

  can_move = DigField(player, jx, jy, new_jx, new_jy, real_dx, real_dy, DF_DIG);
  if (can_move != MP_MOVING)
    return can_move;

  // check if DigField() has caused relocation of the player
  if (player->jx != jx || player->jy != jy)
    return MP_NO_ACTION;	// <-- !!! CHECK THIS [-> MP_ACTION ?] !!!

  StorePlayer[jx][jy] = 0;
  player->last_jx = jx;
  player->last_jy = jy;
  player->jx = new_jx;
  player->jy = new_jy;
  StorePlayer[new_jx][new_jy] = player->element_nr;

  if (player->move_delay_value_next != -1)
  {
    player->move_delay_value = player->move_delay_value_next;
    player->move_delay_value_next = -1;
  }

  player->MovPos =
    (dx > 0 || dy > 0 ? -1 : 1) * (TILEX - TILEX / player->move_delay_value);

  player->step_counter++;

  PlayerVisit[jx][jy] = FrameCounter;

  player->is_moving = TRUE;

#if 1
  // should better be called in MovePlayer(), but this breaks some tapes
  ScrollPlayer(player, SCROLL_INIT);
#endif

  return MP_MOVING;
}

boolean MovePlayer(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int old_jx = jx, old_jy = jy;
  int moved = MP_NO_ACTION;

  if (!player->active)
    return FALSE;

  if (!dx && !dy)
  {
    if (player->MovPos == 0)
    {
      player->is_moving = FALSE;
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
      player->is_snapping = FALSE;
      player->is_pushing = FALSE;
    }

    return FALSE;
  }

  if (player->move_delay > 0)
    return FALSE;

  player->move_delay = -1;		// set to "uninitialized" value

  // store if player is automatically moved to next field
  player->is_auto_moving = (player->programmed_action != MV_NONE);

  // remove the last programmed player action
  player->programmed_action = 0;

  if (player->MovPos)
  {
    // should only happen if pre-1.2 tape recordings are played
    // this is only for backward compatibility

    int original_move_delay_value = player->move_delay_value;

#if DEBUG
    Debug("game:playing:MovePlayer",
	  "THIS SHOULD ONLY HAPPEN WITH PRE-1.2 LEVEL TAPES. [%d]",
	  tape.counter);
#endif

    // scroll remaining steps with finest movement resolution
    player->move_delay_value = MOVE_DELAY_NORMAL_SPEED;

    while (player->MovPos)
    {
      ScrollPlayer(player, SCROLL_GO_ON);
      ScrollScreen(NULL, SCROLL_GO_ON);

      AdvanceFrameAndPlayerCounters(player->index_nr);

      DrawAllPlayers();
      BackToFront_WithFrameDelay(0);
    }

    player->move_delay_value = original_move_delay_value;
  }

  player->is_active = FALSE;

  if (player->last_move_dir & MV_HORIZONTAL)
  {
    if (!(moved |= MovePlayerOneStep(player, 0, dy, dx, dy)))
      moved |= MovePlayerOneStep(player, dx, 0, dx, dy);
  }
  else
  {
    if (!(moved |= MovePlayerOneStep(player, dx, 0, dx, dy)))
      moved |= MovePlayerOneStep(player, 0, dy, dx, dy);
  }

  if (!moved && !player->is_active)
  {
    player->is_moving = FALSE;
    player->is_digging = FALSE;
    player->is_collecting = FALSE;
    player->is_snapping = FALSE;
    player->is_pushing = FALSE;
  }

  jx = player->jx;
  jy = player->jy;

  if (moved & MP_MOVING && !ScreenMovPos &&
      (player->index_nr == game.centered_player_nr ||
       game.centered_player_nr == -1))
  {
    int old_scroll_x = scroll_x, old_scroll_y = scroll_y;

    if (!IN_VIS_FIELD(SCREENX(jx), SCREENY(jy)))
    {
      // actual player has left the screen -- scroll in that direction
      if (jx != old_jx)		// player has moved horizontally
	scroll_x += (jx - old_jx);
      else			// player has moved vertically
	scroll_y += (jy - old_jy);
    }
    else
    {
      int offset_raw = game.scroll_delay_value;

      if (jx != old_jx)		// player has moved horizontally
      {
	int offset = MIN(offset_raw, (SCR_FIELDX - 2) / 2);
	int offset_x = offset * (player->MovDir == MV_LEFT ? +1 : -1);
	int new_scroll_x = jx - MIDPOSX + offset_x;

	if ((player->MovDir == MV_LEFT  && scroll_x > new_scroll_x) ||
	    (player->MovDir == MV_RIGHT && scroll_x < new_scroll_x))
	  scroll_x = new_scroll_x;

	// don't scroll over playfield boundaries
	scroll_x = MIN(MAX(SBX_Left, scroll_x), SBX_Right);

	// don't scroll more than one field at a time
	scroll_x = old_scroll_x + SIGN(scroll_x - old_scroll_x);

	// don't scroll against the player's moving direction
	if ((player->MovDir == MV_LEFT  && scroll_x > old_scroll_x) ||
	    (player->MovDir == MV_RIGHT && scroll_x < old_scroll_x))
	  scroll_x = old_scroll_x;
      }
      else			// player has moved vertically
      {
	int offset = MIN(offset_raw, (SCR_FIELDY - 2) / 2);
	int offset_y = offset * (player->MovDir == MV_UP ? +1 : -1);
	int new_scroll_y = jy - MIDPOSY + offset_y;

	if ((player->MovDir == MV_UP   && scroll_y > new_scroll_y) ||
	    (player->MovDir == MV_DOWN && scroll_y < new_scroll_y))
	  scroll_y = new_scroll_y;

	// don't scroll over playfield boundaries
	scroll_y = MIN(MAX(SBY_Upper, scroll_y), SBY_Lower);

	// don't scroll more than one field at a time
	scroll_y = old_scroll_y + SIGN(scroll_y - old_scroll_y);

	// don't scroll against the player's moving direction
	if ((player->MovDir == MV_UP   && scroll_y > old_scroll_y) ||
	    (player->MovDir == MV_DOWN && scroll_y < old_scroll_y))
	  scroll_y = old_scroll_y;
      }
    }

    if (scroll_x != old_scroll_x || scroll_y != old_scroll_y)
    {
      if (!network.enabled && game.centered_player_nr == -1 &&
	  !AllPlayersInVisibleScreen())
      {
	scroll_x = old_scroll_x;
	scroll_y = old_scroll_y;
      }
      else
      {
	ScrollScreen(player, SCROLL_INIT);
	ScrollLevel(old_scroll_x - scroll_x, old_scroll_y - scroll_y);
      }
    }
  }

  player->StepFrame = 0;

  if (moved & MP_MOVING)
  {
    if (old_jx != jx && old_jy == jy)
      player->MovDir = (old_jx < jx ? MV_RIGHT : MV_LEFT);
    else if (old_jx == jx && old_jy != jy)
      player->MovDir = (old_jy < jy ? MV_DOWN : MV_UP);

    TEST_DrawLevelField(jx, jy);	// for "crumbled sand"

    player->last_move_dir = player->MovDir;
    player->is_moving = TRUE;
    player->is_snapping = FALSE;
    player->is_switching = FALSE;
    player->is_dropping = FALSE;
    player->is_dropping_pressed = FALSE;
    player->drop_pressed_delay = 0;

#if 0
    // should better be called here than above, but this breaks some tapes
    ScrollPlayer(player, SCROLL_INIT);
#endif
  }
  else
  {
    CheckGravityMovementWhenNotMoving(player);

    player->is_moving = FALSE;

    /* at this point, the player is allowed to move, but cannot move right now
       (e.g. because of something blocking the way) -- ensure that the player
       is also allowed to move in the next frame (in old versions before 3.1.1,
       the player was forced to wait again for eight frames before next try) */

    if (game.engine_version >= VERSION_IDENT(3,1,1,0))
      player->move_delay = 0;	// allow direct movement in the next frame
  }

  if (player->move_delay == -1)		// not yet initialized by DigField()
    player->move_delay = player->move_delay_value;

  if (game.engine_version < VERSION_IDENT(3,0,7,0))
  {
    TestIfPlayerTouchesBadThing(jx, jy);
    TestIfPlayerTouchesCustomElement(jx, jy);
  }

  if (!player->active)
    RemovePlayer(player);

  return moved;
}

void ScrollPlayer(struct PlayerInfo *player, int mode)
{
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;
  int move_stepsize = TILEX / player->move_delay_value;

  if (!player->active)
    return;

  if (player->MovPos == 0 && mode == SCROLL_GO_ON)	// player not moving
    return;

  if (mode == SCROLL_INIT)
  {
    player->actual_frame_counter.count = FrameCounter;
    player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

    if ((player->block_last_field || player->block_delay_adjustment > 0) &&
	Tile[last_jx][last_jy] == EL_EMPTY)
    {
      int last_field_block_delay = 0;	// start with no blocking at all
      int block_delay_adjustment = player->block_delay_adjustment;

      // if player blocks last field, add delay for exactly one move
      if (player->block_last_field)
      {
	last_field_block_delay += player->move_delay_value;

	// when blocking enabled, prevent moving up despite gravity
	if (player->gravity && player->MovDir == MV_UP)
	  block_delay_adjustment = -1;
      }

      // add block delay adjustment (also possible when not blocking)
      last_field_block_delay += block_delay_adjustment;

      Tile[last_jx][last_jy] = EL_PLAYER_IS_LEAVING;
      MovDelay[last_jx][last_jy] = last_field_block_delay + 1;
    }

    if (player->MovPos != 0)	// player has not yet reached destination
      return;
  }
  else if (!FrameReached(&player->actual_frame_counter))
    return;

  if (player->MovPos != 0)
  {
    player->MovPos += (player->MovPos > 0 ? -1 : 1) * move_stepsize;
    player->GfxPos = move_stepsize * (player->MovPos / move_stepsize);

    // before DrawPlayer() to draw correct player graphic for this case
    if (player->MovPos == 0)
      CheckGravityMovement(player);
  }

  if (player->MovPos == 0)	// player reached destination field
  {
    if (player->move_delay_reset_counter > 0)
    {
      player->move_delay_reset_counter--;

      if (player->move_delay_reset_counter == 0)
      {
	// continue with normal speed after quickly moving through gate
	HALVE_PLAYER_SPEED(player);

	// be able to make the next move without delay
	player->move_delay = 0;
      }
    }

    if (Tile[jx][jy] == EL_EXIT_OPEN ||
	Tile[jx][jy] == EL_EM_EXIT_OPEN ||
	Tile[jx][jy] == EL_EM_EXIT_OPENING ||
	Tile[jx][jy] == EL_STEEL_EXIT_OPEN ||
	Tile[jx][jy] == EL_EM_STEEL_EXIT_OPEN ||
	Tile[jx][jy] == EL_EM_STEEL_EXIT_OPENING ||
	Tile[jx][jy] == EL_SP_EXIT_OPEN ||
	Tile[jx][jy] == EL_SP_EXIT_OPENING)	// <-- special case
    {
      ExitPlayer(player);

      if (game.players_still_needed == 0 &&
	  (game.friends_still_needed == 0 ||
	   IS_SP_ELEMENT(Tile[jx][jy])))
	LevelSolved();
    }

    player->last_jx = jx;
    player->last_jy = jy;

    // this breaks one level: "machine", level 000
    {
      int move_direction = player->MovDir;
      int enter_side = MV_DIR_OPPOSITE(move_direction);
      int leave_side = move_direction;
      int old_jx = last_jx;
      int old_jy = last_jy;
      int old_element = Tile[old_jx][old_jy];
      int new_element = Tile[jx][jy];

      if (IS_CUSTOM_ELEMENT(old_element))
	CheckElementChangeByPlayer(old_jx, old_jy, old_element,
				   CE_LEFT_BY_PLAYER,
				   player->index_bit, leave_side);

      CheckTriggeredElementChangeByPlayer(old_jx, old_jy, old_element,
					  CE_PLAYER_LEAVES_X,
					  player->index_bit, leave_side);

      // needed because pushed element has not yet reached its destination,
      // so it would trigger a change event at its previous field location
      if (!player->is_pushing)
      {
	if (IS_CUSTOM_ELEMENT(new_element))
	  CheckElementChangeByPlayer(jx, jy, new_element, CE_ENTERED_BY_PLAYER,
				     player->index_bit, enter_side);

	CheckTriggeredElementChangeByPlayer(jx, jy, new_element,
					    CE_PLAYER_ENTERS_X,
					    player->index_bit, enter_side);
      }

      CheckTriggeredElementChangeBySide(jx, jy, player->initial_element,
					CE_MOVE_OF_X, move_direction);
    }

    if (game.engine_version >= VERSION_IDENT(3,0,7,0))
    {
      TestIfPlayerTouchesBadThing(jx, jy);
      TestIfPlayerTouchesCustomElement(jx, jy);

      // needed because pushed element has not yet reached its destination,
      // so it would trigger a change event at its previous field location
      if (!player->is_pushing)
	TestIfElementTouchesCustomElement(jx, jy);	// for empty space

      if (level.finish_dig_collect &&
	  (player->is_digging || player->is_collecting))
      {
	int last_element = player->last_removed_element;
	int move_direction = player->MovDir;
	int enter_side = MV_DIR_OPPOSITE(move_direction);
	int change_event = (player->is_digging ? CE_PLAYER_DIGS_X :
			    CE_PLAYER_COLLECTS_X);

	CheckTriggeredElementChangeByPlayer(jx, jy, last_element, change_event,
					    player->index_bit, enter_side);

	player->last_removed_element = EL_UNDEFINED;
      }

      if (!player->active)
	RemovePlayer(player);
    }

    if (level.use_step_counter)
      CheckLevelTime_StepCounter();

    if (tape.single_step && tape.recording && !tape.pausing &&
	!player->programmed_action)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

    if (!player->programmed_action)
      CheckSaveEngineSnapshot(player);
  }
}

void ScrollScreen(struct PlayerInfo *player, int mode)
{
  static DelayCounter screen_frame_counter = { 0 };

  if (mode == SCROLL_INIT)
  {
    // set scrolling step size according to actual player's moving speed
    ScrollStepSize = TILEX / player->move_delay_value;

    screen_frame_counter.count = FrameCounter;
    screen_frame_counter.value = 1;

    ScreenMovDir = player->MovDir;
    ScreenMovPos = player->MovPos;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    return;
  }
  else if (!FrameReached(&screen_frame_counter))
    return;

  if (ScreenMovPos)
  {
    ScreenMovPos += (ScreenMovPos > 0 ? -1 : 1) * ScrollStepSize;
    ScreenGfxPos = ScrollStepSize * (ScreenMovPos / ScrollStepSize);
    redraw_mask |= REDRAW_FIELD;
  }
  else
    ScreenMovDir = MV_NONE;
}

void CheckNextToConditions(int x, int y)
{
  int element = Tile[x][y];

  if (IS_PLAYER(x, y))
    TestIfPlayerNextToCustomElement(x, y);

  if (CAN_CHANGE_OR_HAS_ACTION(element) &&
      HAS_ANY_CHANGE_EVENT(element, CE_NEXT_TO_X))
    TestIfElementNextToCustomElement(x, y);
}

void TestIfPlayerNextToCustomElement(int x, int y)
{
  struct XY *xy = xy_topdown;
  static int trigger_sides[4][2] =
  {
    // center side       border side
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	// check top
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	// check left
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	// check right
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	// check bottom
  };
  int i;

  if (!IS_PLAYER(x, y))
    return;

  struct PlayerInfo *player = PLAYERINFO(x, y);

  if (player->is_moving)
    return;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int border_side = trigger_sides[i][1];
    int border_element;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    if (IS_MOVING(xx, yy) || IS_BLOCKED(xx, yy))
      continue;		// center and border element not connected

    border_element = Tile[xx][yy];

    CheckElementChangeByPlayer(xx, yy, border_element, CE_NEXT_TO_PLAYER,
                               player->index_bit, border_side);
    CheckTriggeredElementChangeByPlayer(xx, yy, border_element,
                                        CE_PLAYER_NEXT_TO_X,
                                        player->index_bit, border_side);

    /* use player element that is initially defined in the level playfield,
       not the player element that corresponds to the runtime player number
       (example: a level that contains EL_PLAYER_3 as the only player would
       incorrectly give EL_PLAYER_1 for "player->element_nr") */

    CheckElementChangeBySide(xx, yy, border_element, player->initial_element,
                             CE_NEXT_TO_X, border_side);
  }
}

void TestIfPlayerTouchesCustomElement(int x, int y)
{
  struct XY *xy = xy_topdown;
  static int trigger_sides[4][2] =
  {
    // center side       border side
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	// check top
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	// check left
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	// check right
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	// check bottom
  };
  static int touch_dir[4] =
  {
    MV_LEFT | MV_RIGHT,
    MV_UP   | MV_DOWN,
    MV_UP   | MV_DOWN,
    MV_LEFT | MV_RIGHT
  };
  int center_element = Tile[x][y];	// should always be non-moving!
  int i;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int center_side = trigger_sides[i][0];
    int border_side = trigger_sides[i][1];
    int border_element;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    if (IS_PLAYER(x, y))		// player found at center element
    {
      struct PlayerInfo *player = PLAYERINFO(x, y);

      if (game.engine_version < VERSION_IDENT(3,0,7,0))
	border_element = Tile[xx][yy];		// may be moving!
      else if (!IS_MOVING(xx, yy) && !IS_BLOCKED(xx, yy))
	border_element = Tile[xx][yy];
      else if (MovDir[xx][yy] & touch_dir[i])	// elements are touching
	border_element = MovingOrBlocked2Element(xx, yy);
      else
	continue;		// center and border element do not touch

      CheckElementChangeByPlayer(xx, yy, border_element, CE_TOUCHED_BY_PLAYER,
				 player->index_bit, border_side);
      CheckTriggeredElementChangeByPlayer(xx, yy, border_element,
					  CE_PLAYER_TOUCHES_X,
					  player->index_bit, border_side);

      {
	/* use player element that is initially defined in the level playfield,
	   not the player element that corresponds to the runtime player number
	   (example: a level that contains EL_PLAYER_3 as the only player would
	   incorrectly give EL_PLAYER_1 for "player->element_nr") */
	int player_element = PLAYERINFO(x, y)->initial_element;

	// as element "X" is the player here, check opposite (center) side
	CheckElementChangeBySide(xx, yy, border_element, player_element,
				 CE_TOUCHING_X, center_side);
      }
    }
    else if (IS_PLAYER(xx, yy))		// player found at border element
    {
      struct PlayerInfo *player = PLAYERINFO(xx, yy);

      if (game.engine_version >= VERSION_IDENT(3,0,7,0))
      {
	if (player->MovPos != 0 && !(player->MovDir & touch_dir[i]))
	  continue;		// center and border element do not touch
      }

      CheckElementChangeByPlayer(x, y, center_element, CE_TOUCHED_BY_PLAYER,
				 player->index_bit, center_side);
      CheckTriggeredElementChangeByPlayer(x, y, center_element,
					  CE_PLAYER_TOUCHES_X,
					  player->index_bit, center_side);

      {
	/* use player element that is initially defined in the level playfield,
	   not the player element that corresponds to the runtime player number
	   (example: a level that contains EL_PLAYER_3 as the only player would
	   incorrectly give EL_PLAYER_1 for "player->element_nr") */
	int player_element = PLAYERINFO(xx, yy)->initial_element;

	// as element "X" is the player here, check opposite (border) side
	CheckElementChangeBySide(x, y, center_element, player_element,
				 CE_TOUCHING_X, border_side);
      }

      break;
    }
  }
}

void TestIfElementNextToCustomElement(int x, int y)
{
  struct XY *xy = xy_topdown;
  static int trigger_sides[4][2] =
  {
    // center side	border side
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	// check top
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	// check left
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	// check right
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	// check bottom
  };
  int center_element = Tile[x][y];	// should always be non-moving!
  int i;

  if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
    return;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int border_side = trigger_sides[i][1];
    int border_element;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    if (IS_MOVING(xx, yy) || IS_BLOCKED(xx, yy))
      continue;			// center and border element not connected

    border_element = Tile[xx][yy];

    // check for change of center element (but change it only once)
    if (CheckElementChangeBySide(x, y, center_element, border_element,
                                 CE_NEXT_TO_X, border_side))
      break;
  }
}

void TestIfElementTouchesCustomElement(int x, int y)
{
  struct XY *xy = xy_topdown;
  static int trigger_sides[4][2] =
  {
    // center side	border side
    { CH_SIDE_TOP,	CH_SIDE_BOTTOM	},	// check top
    { CH_SIDE_LEFT,	CH_SIDE_RIGHT	},	// check left
    { CH_SIDE_RIGHT,	CH_SIDE_LEFT	},	// check right
    { CH_SIDE_BOTTOM,	CH_SIDE_TOP	}	// check bottom
  };
  static int touch_dir[4] =
  {
    MV_LEFT | MV_RIGHT,
    MV_UP   | MV_DOWN,
    MV_UP   | MV_DOWN,
    MV_LEFT | MV_RIGHT
  };
  boolean change_center_element = FALSE;
  int center_element = Tile[x][y];	// should always be non-moving!
  int border_element_old[NUM_DIRECTIONS];
  int i;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int border_element;

    border_element_old[i] = -1;

    if (!IN_LEV_FIELD(xx, yy))
      continue;

    if (game.engine_version < VERSION_IDENT(3,0,7,0))
      border_element = Tile[xx][yy];	// may be moving!
    else if (!IS_MOVING(xx, yy) && !IS_BLOCKED(xx, yy))
      border_element = Tile[xx][yy];
    else if (MovDir[xx][yy] & touch_dir[i])	// elements are touching
      border_element = MovingOrBlocked2Element(xx, yy);
    else
      continue;			// center and border element do not touch

    border_element_old[i] = border_element;
  }

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int center_side = trigger_sides[i][0];
    int border_element = border_element_old[i];

    if (border_element == -1)
      continue;

    // check for change of border element
    CheckElementChangeBySide(xx, yy, border_element, center_element,
			     CE_TOUCHING_X, center_side);

    // (center element cannot be player, so we don't have to check this here)
  }

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int border_side = trigger_sides[i][1];
    int border_element = border_element_old[i];

    if (border_element == -1)
      continue;

    // check for change of center element (but change it only once)
    if (!change_center_element)
      change_center_element =
	CheckElementChangeBySide(x, y, center_element, border_element,
				 CE_TOUCHING_X, border_side);

    if (IS_PLAYER(xx, yy))
    {
      /* use player element that is initially defined in the level playfield,
	 not the player element that corresponds to the runtime player number
	 (example: a level that contains EL_PLAYER_3 as the only player would
	 incorrectly give EL_PLAYER_1 for "player->element_nr") */
      int player_element = PLAYERINFO(xx, yy)->initial_element;

      // as element "X" is the player here, check opposite (border) side
      CheckElementChangeBySide(x, y, center_element, player_element,
			       CE_TOUCHING_X, border_side);
    }
  }
}

void TestIfElementHitsCustomElement(int x, int y, int direction)
{
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int hitx = x + dx, hity = y + dy;
  int hitting_element = Tile[x][y];
  int touched_element;

  if (IN_LEV_FIELD(hitx, hity) && IS_FREE(hitx, hity))
    return;

  touched_element = (IN_LEV_FIELD(hitx, hity) ?
		     MovingOrBlocked2Element(hitx, hity) : EL_STEELWALL);

  if (IN_LEV_FIELD(hitx, hity))
  {
    int opposite_direction = MV_DIR_OPPOSITE(direction);
    int hitting_side = direction;
    int touched_side = opposite_direction;
    boolean object_hit = (!IS_MOVING(hitx, hity) ||
			  MovDir[hitx][hity] != direction ||
			  ABS(MovPos[hitx][hity]) <= TILEY / 2);

    object_hit = TRUE;

    if (object_hit)
    {
      CheckElementChangeBySide(x, y, hitting_element, touched_element,
			       CE_HITTING_X, touched_side);

      CheckElementChangeBySide(hitx, hity, touched_element, hitting_element,
			       CE_HIT_BY_X, hitting_side);

      CheckElementChangeBySide(hitx, hity, touched_element, hitting_element,
			       CE_HIT_BY_SOMETHING, opposite_direction);

      if (IS_PLAYER(hitx, hity))
      {
	/* use player element that is initially defined in the level playfield,
	   not the player element that corresponds to the runtime player number
	   (example: a level that contains EL_PLAYER_3 as the only player would
	   incorrectly give EL_PLAYER_1 for "player->element_nr") */
	int player_element = PLAYERINFO(hitx, hity)->initial_element;

	CheckElementChangeBySide(x, y, hitting_element, player_element,
				 CE_HITTING_X, touched_side);
      }
    }
  }

  // "hitting something" is also true when hitting the playfield border
  CheckElementChangeBySide(x, y, hitting_element, touched_element,
			   CE_HITTING_SOMETHING, direction);
}

void TestIfGoodThingHitsBadThing(int good_x, int good_y, int good_move_dir)
{
  int i, kill_x = -1, kill_y = -1;

  int bad_element = -1;
  struct XY *test_xy = xy_topdown;
  static int test_dir[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int test_x, test_y, test_move_dir, test_element;

    test_x = good_x + test_xy[i].x;
    test_y = good_y + test_xy[i].y;

    if (!IN_LEV_FIELD(test_x, test_y))
      continue;

    test_move_dir =
      (IS_MOVING(test_x, test_y) ? MovDir[test_x][test_y] : MV_NONE);

    test_element = MovingOrBlocked2ElementIfNotLeaving(test_x, test_y);

    /* 1st case: good thing is moving towards DONT_RUN_INTO style bad thing;
       2nd case: DONT_TOUCH style bad thing does not move away from good thing
    */
    if ((DONT_RUN_INTO(test_element) && good_move_dir == test_dir[i]) ||
	(DONT_TOUCH(test_element)    && test_move_dir != test_dir[i]))
    {
      kill_x = test_x;
      kill_y = test_y;
      bad_element = test_element;

      break;
    }
  }

  if (kill_x != -1 || kill_y != -1)
  {
    if (IS_PLAYER(good_x, good_y))
    {
      struct PlayerInfo *player = PLAYERINFO(good_x, good_y);

      if (player->shield_deadly_time_left > 0 &&
	  !IS_INDESTRUCTIBLE(bad_element))
	Bang(kill_x, kill_y);
      else if (!PLAYER_ENEMY_PROTECTED(good_x, good_y))
	KillPlayer(player);
    }
    else
      Bang(good_x, good_y);
  }
}

void TestIfBadThingHitsGoodThing(int bad_x, int bad_y, int bad_move_dir)
{
  int i, kill_x = -1, kill_y = -1;
  int bad_element = Tile[bad_x][bad_y];
  struct XY *test_xy = xy_topdown;
  static int touch_dir[4] =
  {
    MV_LEFT | MV_RIGHT,
    MV_UP   | MV_DOWN,
    MV_UP   | MV_DOWN,
    MV_LEFT | MV_RIGHT
  };
  static int test_dir[4] =
  {
    MV_UP,
    MV_LEFT,
    MV_RIGHT,
    MV_DOWN
  };

  if (bad_element == EL_EXPLOSION)	// skip just exploding bad things
    return;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int test_x, test_y, test_move_dir, test_element;

    test_x = bad_x + test_xy[i].x;
    test_y = bad_y + test_xy[i].y;

    if (!IN_LEV_FIELD(test_x, test_y))
      continue;

    test_move_dir =
      (IS_MOVING(test_x, test_y) ? MovDir[test_x][test_y] : MV_NONE);

    test_element = Tile[test_x][test_y];

    /* 1st case: good thing is moving towards DONT_RUN_INTO style bad thing;
       2nd case: DONT_TOUCH style bad thing does not move away from good thing
    */
    if ((DONT_RUN_INTO(bad_element) &&  bad_move_dir == test_dir[i]) ||
	(DONT_TOUCH(bad_element)    && test_move_dir != test_dir[i]))
    {
      // good thing is player or penguin that does not move away
      if (IS_PLAYER(test_x, test_y))
      {
	struct PlayerInfo *player = PLAYERINFO(test_x, test_y);

	if (bad_element == EL_ROBOT && player->is_moving)
	  continue;	// robot does not kill player if he is moving

	if (game.engine_version >= VERSION_IDENT(3,0,7,0))
	{
	  if (player->MovPos != 0 && !(player->MovDir & touch_dir[i]))
	    continue;		// center and border element do not touch
	}

	kill_x = test_x;
	kill_y = test_y;

	break;
      }
      else if (test_element == EL_PENGUIN)
      {
	kill_x = test_x;
	kill_y = test_y;

	break;
      }
    }
  }

  if (kill_x != -1 || kill_y != -1)
  {
    if (IS_PLAYER(kill_x, kill_y))
    {
      struct PlayerInfo *player = PLAYERINFO(kill_x, kill_y);

      if (player->shield_deadly_time_left > 0 &&
	  !IS_INDESTRUCTIBLE(bad_element))
	Bang(bad_x, bad_y);
      else if (!PLAYER_ENEMY_PROTECTED(kill_x, kill_y))
	KillPlayer(player);
    }
    else
      Bang(kill_x, kill_y);
  }
}

void TestIfGoodThingGetsHitByBadThing(int bad_x, int bad_y, int bad_move_dir)
{
  int bad_element = Tile[bad_x][bad_y];
  int dx = (bad_move_dir == MV_LEFT ? -1 : bad_move_dir == MV_RIGHT ? +1 : 0);
  int dy = (bad_move_dir == MV_UP   ? -1 : bad_move_dir == MV_DOWN  ? +1 : 0);
  int test_x = bad_x + dx, test_y = bad_y + dy;
  int test_move_dir, test_element;
  int kill_x = -1, kill_y = -1;

  if (!IN_LEV_FIELD(test_x, test_y))
    return;

  test_move_dir =
    (IS_MOVING(test_x, test_y) ? MovDir[test_x][test_y] : MV_NONE);

  test_element = Tile[test_x][test_y];

  if (test_move_dir != bad_move_dir)
  {
    // good thing can be player or penguin that does not move away
    if (IS_PLAYER(test_x, test_y))
    {
      struct PlayerInfo *player = PLAYERINFO(test_x, test_y);

      /* (note: in comparison to DONT_RUN_TO and DONT_TOUCH, also handle the
	 player as being hit when he is moving towards the bad thing, because
	 the "get hit by" condition would be lost after the player stops) */
      if (player->MovPos != 0 && player->MovDir == bad_move_dir)
	return;		// player moves away from bad thing

      kill_x = test_x;
      kill_y = test_y;
    }
    else if (test_element == EL_PENGUIN)
    {
      kill_x = test_x;
      kill_y = test_y;
    }
  }

  if (kill_x != -1 || kill_y != -1)
  {
    if (IS_PLAYER(kill_x, kill_y))
    {
      struct PlayerInfo *player = PLAYERINFO(kill_x, kill_y);

      if (player->shield_deadly_time_left > 0 &&
	  !IS_INDESTRUCTIBLE(bad_element))
	Bang(bad_x, bad_y);
      else if (!PLAYER_ENEMY_PROTECTED(kill_x, kill_y))
	KillPlayer(player);
    }
    else
      Bang(kill_x, kill_y);
  }
}

void TestIfPlayerTouchesBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x, y, MV_NONE);
}

void TestIfPlayerRunsIntoBadThing(int x, int y, int move_dir)
{
  TestIfGoodThingHitsBadThing(x, y, move_dir);
}

void TestIfBadThingTouchesPlayer(int x, int y)
{
  TestIfBadThingHitsGoodThing(x, y, MV_NONE);
}

void TestIfBadThingRunsIntoPlayer(int x, int y, int move_dir)
{
  TestIfBadThingHitsGoodThing(x, y, move_dir);
}

void TestIfFriendTouchesBadThing(int x, int y)
{
  TestIfGoodThingHitsBadThing(x, y, MV_NONE);
}

void TestIfBadThingTouchesFriend(int x, int y)
{
  TestIfBadThingHitsGoodThing(x, y, MV_NONE);
}

void TestIfBadThingTouchesOtherBadThing(int bad_x, int bad_y)
{
  int i, kill_x = bad_x, kill_y = bad_y;
  struct XY *xy = xy_topdown;

  for (i = 0; i < NUM_DIRECTIONS; i++)
  {
    int x, y, element;

    x = bad_x + xy[i].x;
    y = bad_y + xy[i].y;
    if (!IN_LEV_FIELD(x, y))
      continue;

    element = Tile[x][y];
    if (IS_AMOEBOID(element) || element == EL_GAME_OF_LIFE ||
	element == EL_AMOEBA_GROWING || element == EL_AMOEBA_DROP)
    {
      kill_x = x;
      kill_y = y;
      break;
    }
  }

  if (kill_x != bad_x || kill_y != bad_y)
    Bang(bad_x, bad_y);
}

void KillPlayer(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

#if 0
  Debug("game:playing:KillPlayer",
	"0: killed == %d, active == %d, reanimated == %d",
	player->killed, player->active, player->reanimated);
#endif

  /* the following code was introduced to prevent an infinite loop when calling
     -> Bang()
     -> CheckTriggeredElementChangeExt()
     -> ExecuteCustomElementAction()
     -> KillPlayer()
     -> (infinitely repeating the above sequence of function calls)
     which occurs when killing the player while having a CE with the setting
     "kill player X when explosion of <player X>"; the solution using a new
     field "player->killed" was chosen for backwards compatibility, although
     clever use of the fields "player->active" etc. would probably also work */
#if 1
  if (player->killed)
    return;
#endif

  player->killed = TRUE;

  // remove accessible field at the player's position
  RemoveField(jx, jy);

  // deactivate shield (else Bang()/Explode() would not work right)
  player->shield_normal_time_left = 0;
  player->shield_deadly_time_left = 0;

#if 0
  Debug("game:playing:KillPlayer",
	"1: killed == %d, active == %d, reanimated == %d",
	player->killed, player->active, player->reanimated);
#endif

  Bang(jx, jy);

#if 0
  Debug("game:playing:KillPlayer",
	"2: killed == %d, active == %d, reanimated == %d",
	player->killed, player->active, player->reanimated);
#endif

  if (player->reanimated)	// killed player may have been reanimated
    player->killed = player->reanimated = FALSE;
  else
    BuryPlayer(player);
}

static void KillPlayerUnlessEnemyProtected(int x, int y)
{
  if (!PLAYER_ENEMY_PROTECTED(x, y))
    KillPlayer(PLAYERINFO(x, y));
}

static void KillPlayerUnlessExplosionProtected(int x, int y)
{
  if (!PLAYER_EXPLOSION_PROTECTED(x, y))
    KillPlayer(PLAYERINFO(x, y));
}

void BuryPlayer(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;

  if (!player->active)
    return;

  PlayLevelSoundElementAction(jx, jy, player->artwork_element, ACTION_DYING);

  RemovePlayer(player);

  player->buried = TRUE;

  if (game.all_players_gone)
    game.GameOver = TRUE;
}

void RemovePlayer(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int i, found = FALSE;

  player->present = FALSE;
  player->active = FALSE;

  // required for some CE actions (even if the player is not active anymore)
  player->MovPos = 0;

  if (!ExplodeField[jx][jy])
    StorePlayer[jx][jy] = 0;

  if (player->is_moving)
    TEST_DrawLevelField(player->last_jx, player->last_jy);

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].active)
      found = TRUE;

  if (!found)
  {
    game.all_players_gone = TRUE;
    game.GameOver = TRUE;
  }

  game.exit_x = game.robot_wheel_x = jx;
  game.exit_y = game.robot_wheel_y = jy;
}

void ExitPlayer(struct PlayerInfo *player)
{
  DrawPlayer(player);	// needed here only to cleanup last field
  RemovePlayer(player);

  if (game.players_still_needed > 0)
    game.players_still_needed--;
}

static void SetFieldForSnapping(int x, int y, int element, int direction,
				int player_index_bit)
{
  struct ElementInfo *ei = &element_info[element];
  int direction_bit = MV_DIR_TO_BIT(direction);
  int graphic_snapping = ei->direction_graphic[ACTION_SNAPPING][direction_bit];
  int action = (graphic_snapping != IMG_EMPTY_SPACE ? ACTION_SNAPPING :
		IS_DIGGABLE(element) ? ACTION_DIGGING : ACTION_COLLECTING);

  Tile[x][y] = EL_ELEMENT_SNAPPING;
  MovDelay[x][y] = MOVE_DELAY_NORMAL_SPEED + 1 - 1;
  MovDir[x][y] = direction;
  Store[x][y] = element;
  Store2[x][y] = player_index_bit;

  ResetGfxAnimation(x, y);

  GfxElement[x][y] = element;
  GfxAction[x][y] = action;
  GfxDir[x][y] = direction;
  GfxFrame[x][y] = -1;
}

static void TestFieldAfterSnapping(int x, int y, int element, int direction,
				   int player_index_bit)
{
  TestIfElementTouchesCustomElement(x, y);	// for empty space

  if (level.finish_dig_collect)
  {
    int dig_side = MV_DIR_OPPOSITE(direction);
    int change_event = (IS_DIGGABLE(element) ? CE_PLAYER_DIGS_X :
			CE_PLAYER_COLLECTS_X);

    CheckTriggeredElementChangeByPlayer(x, y, element, change_event,
					player_index_bit, dig_side);
    CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_SNAPS_X,
					player_index_bit, dig_side);
  }
}

/*
  =============================================================================
  checkDiagonalPushing()
  -----------------------------------------------------------------------------
  check if diagonal input device direction results in pushing of object
  (by checking if the alternative direction is walkable, diggable, ...)
  =============================================================================
*/

static boolean checkDiagonalPushing(struct PlayerInfo *player,
				    int x, int y, int real_dx, int real_dy)
{
  int jx, jy, dx, dy, xx, yy;

  if (real_dx == 0 || real_dy == 0)	// no diagonal direction => push
    return TRUE;

  // diagonal direction: check alternative direction
  jx = player->jx;
  jy = player->jy;
  dx = x - jx;
  dy = y - jy;
  xx = jx + (dx == 0 ? real_dx : 0);
  yy = jy + (dy == 0 ? real_dy : 0);

  return (!IN_LEV_FIELD(xx, yy) || IS_SOLID_FOR_PUSHING(Tile[xx][yy]));
}

/*
  =============================================================================
  DigField()
  -----------------------------------------------------------------------------
  x, y:			field next to player (non-diagonal) to try to dig to
  real_dx, real_dy:	direction as read from input device (can be diagonal)
  =============================================================================
*/

static int DigField(struct PlayerInfo *player,
		    int oldx, int oldy, int x, int y,
		    int real_dx, int real_dy, int mode)
{
  boolean is_player = (IS_PLAYER(oldx, oldy) || mode != DF_DIG);
  boolean player_was_pushing = player->is_pushing;
  boolean player_can_move = (!player->cannot_move && mode != DF_SNAP);
  boolean player_can_move_or_snap = (!player->cannot_move || mode == DF_SNAP);
  int jx = oldx, jy = oldy;
  int dx = x - jx, dy = y - jy;
  int nextx = x + dx, nexty = y + dy;
  int move_direction = (dx == -1 ? MV_LEFT  :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP    :
			dy == +1 ? MV_DOWN  : MV_NONE);
  int opposite_direction = MV_DIR_OPPOSITE(move_direction);
  int dig_side = MV_DIR_OPPOSITE(move_direction);
  int old_element = Tile[jx][jy];
  int element = MovingOrBlocked2ElementIfNotLeaving(x, y);
  int collect_count;

  if (is_player)		// function can also be called by EL_PENGUIN
  {
    if (player->MovPos == 0)
    {
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
    }

    if (player->MovPos == 0)	// last pushing move finished
      player->is_pushing = FALSE;

    if (mode == DF_NO_PUSH)	// player just stopped pushing
    {
      player->is_switching = FALSE;
      player->push_delay = -1;

      return MP_NO_ACTION;
    }
  }
  if (IS_TUBE(Back[jx][jy]) && game.engine_version >= VERSION_IDENT(2,2,0,0))
    old_element = Back[jx][jy];

  // in case of element dropped at player position, check background
  else if (Back[jx][jy] != EL_EMPTY &&
	   game.engine_version >= VERSION_IDENT(2,2,0,0))
    old_element = Back[jx][jy];

  if (IS_WALKABLE(old_element) && !ACCESS_FROM(old_element, move_direction))
    return MP_NO_ACTION;	// field has no opening in this direction

  if (IS_PASSABLE(old_element) && !ACCESS_FROM(old_element, opposite_direction))
    return MP_NO_ACTION;	// field has no opening in this direction

  if (player_can_move && element == EL_ACID && move_direction == MV_DOWN)
  {
    SplashAcid(x, y);

    Tile[jx][jy] = player->artwork_element;
    InitMovingField(jx, jy, MV_DOWN);
    Store[jx][jy] = EL_ACID;
    ContinueMoving(jx, jy);
    BuryPlayer(player);

    return MP_DONT_RUN_INTO;
  }

  if (player_can_move && DONT_RUN_INTO(element))
  {
    TestIfPlayerRunsIntoBadThing(jx, jy, player->MovDir);

    return MP_DONT_RUN_INTO;
  }

  if (IS_MOVING(x, y) || IS_PLAYER(x, y))
    return MP_NO_ACTION;

  collect_count = element_info[element].collect_count_initial;

  if (!is_player && !IS_COLLECTIBLE(element))	// penguin cannot collect it
    return MP_NO_ACTION;

  if (game.engine_version < VERSION_IDENT(2,2,0,0))
    player_can_move = player_can_move_or_snap;

  if (mode == DF_SNAP && !IS_SNAPPABLE(element) &&
      game.engine_version >= VERSION_IDENT(2,2,0,0))
  {
    CheckElementChangeByPlayer(x, y, element, CE_SNAPPED_BY_PLAYER,
			       player->index_bit, dig_side);
    CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_SNAPS_X,
					player->index_bit, dig_side);

    if (element == EL_DC_LANDMINE)
      Bang(x, y);

    if (Tile[x][y] != element)		// field changed by snapping
      return MP_ACTION;

    return MP_NO_ACTION;
  }

  if (player->gravity && is_player && !player->is_auto_moving &&
      canFallDown(player) && move_direction != MV_DOWN &&
      !canMoveToValidFieldWithGravity(jx, jy, move_direction))
    return MP_NO_ACTION;	// player cannot walk here due to gravity

  if (player_can_move &&
      IS_WALKABLE(element) && ACCESS_FROM(element, opposite_direction))
  {
    int sound_element = SND_ELEMENT(element);
    int sound_action = ACTION_WALKING;

    if (IS_RND_GATE(element))
    {
      if (!player->key[RND_GATE_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_RND_GATE_GRAY(element))
    {
      if (!player->key[RND_GATE_GRAY_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_RND_GATE_GRAY_ACTIVE(element))
    {
      if (!player->key[RND_GATE_GRAY_ACTIVE_NR(element)])
	return MP_NO_ACTION;
    }
    else if (element == EL_EXIT_OPEN ||
	     element == EL_EM_EXIT_OPEN ||
	     element == EL_EM_EXIT_OPENING ||
	     element == EL_STEEL_EXIT_OPEN ||
	     element == EL_EM_STEEL_EXIT_OPEN ||
	     element == EL_EM_STEEL_EXIT_OPENING ||
	     element == EL_SP_EXIT_OPEN ||
	     element == EL_SP_EXIT_OPENING)
    {
      sound_action = ACTION_PASSING;	// player is passing exit
    }
    else if (element == EL_EMPTY)
    {
      sound_action = ACTION_MOVING;		// nothing to walk on
    }

    // play sound from background or player, whatever is available
    if (element_info[sound_element].sound[sound_action] != SND_UNDEFINED)
      PlayLevelSoundElementAction(x, y, sound_element, sound_action);
    else
      PlayLevelSoundElementAction(x, y, player->artwork_element, sound_action);
  }
  else if (player_can_move &&
	   IS_PASSABLE(element) && canPassField(x, y, move_direction))
  {
    if (!ACCESS_FROM(element, opposite_direction))
      return MP_NO_ACTION;	// field not accessible from this direction

    if (CAN_MOVE(element))	// only fixed elements can be passed!
      return MP_NO_ACTION;

    if (IS_EM_GATE(element))
    {
      if (!player->key[EM_GATE_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_EM_GATE_GRAY(element))
    {
      if (!player->key[EM_GATE_GRAY_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_EM_GATE_GRAY_ACTIVE(element))
    {
      if (!player->key[EM_GATE_GRAY_ACTIVE_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_EMC_GATE(element))
    {
      if (!player->key[EMC_GATE_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_EMC_GATE_GRAY(element))
    {
      if (!player->key[EMC_GATE_GRAY_NR(element)])
	return MP_NO_ACTION;
    }
    else if (IS_EMC_GATE_GRAY_ACTIVE(element))
    {
      if (!player->key[EMC_GATE_GRAY_ACTIVE_NR(element)])
	return MP_NO_ACTION;
    }
    else if (element == EL_DC_GATE_WHITE ||
	     element == EL_DC_GATE_WHITE_GRAY ||
	     element == EL_DC_GATE_WHITE_GRAY_ACTIVE)
    {
      if (player->num_white_keys == 0)
	return MP_NO_ACTION;

      player->num_white_keys--;
    }
    else if (IS_SP_PORT(element))
    {
      if (element == EL_SP_GRAVITY_PORT_LEFT ||
	  element == EL_SP_GRAVITY_PORT_RIGHT ||
	  element == EL_SP_GRAVITY_PORT_UP ||
	  element == EL_SP_GRAVITY_PORT_DOWN)
	player->gravity = !player->gravity;
      else if (element == EL_SP_GRAVITY_ON_PORT_LEFT ||
	       element == EL_SP_GRAVITY_ON_PORT_RIGHT ||
	       element == EL_SP_GRAVITY_ON_PORT_UP ||
	       element == EL_SP_GRAVITY_ON_PORT_DOWN)
	player->gravity = TRUE;
      else if (element == EL_SP_GRAVITY_OFF_PORT_LEFT ||
	       element == EL_SP_GRAVITY_OFF_PORT_RIGHT ||
	       element == EL_SP_GRAVITY_OFF_PORT_UP ||
	       element == EL_SP_GRAVITY_OFF_PORT_DOWN)
	player->gravity = FALSE;
    }

    // automatically move to the next field with double speed
    player->programmed_action = move_direction;

    if (player->move_delay_reset_counter == 0)
    {
      player->move_delay_reset_counter = 2;	// two double speed steps

      DOUBLE_PLAYER_SPEED(player);
    }

    PlayLevelSoundAction(x, y, ACTION_PASSING);
  }
  else if (player_can_move_or_snap && IS_DIGGABLE(element))
  {
    RemoveField(x, y);

    if (mode != DF_SNAP)
    {
      GfxElement[x][y] = GFX_ELEMENT(element);
      player->is_digging = TRUE;
    }

    PlayLevelSoundElementAction(x, y, element, ACTION_DIGGING);

    // use old behaviour for old levels (digging)
    if (!level.finish_dig_collect)
    {
      CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_DIGS_X,
					  player->index_bit, dig_side);

      // if digging triggered player relocation, finish digging tile
      if (mode == DF_DIG && (player->jx != jx || player->jy != jy))
	SetFieldForSnapping(x, y, element, move_direction, player->index_bit);
    }

    if (mode == DF_SNAP)
    {
      if (level.block_snap_field)
	SetFieldForSnapping(x, y, element, move_direction, player->index_bit);
      else
	TestFieldAfterSnapping(x, y, element, move_direction, player->index_bit);

      // use old behaviour for old levels (snapping)
      if (!level.finish_dig_collect)
	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_SNAPS_X,
					    player->index_bit, dig_side);
    }
  }
  else if (player_can_move_or_snap && IS_COLLECTIBLE(element))
  {
    RemoveField(x, y);

    if (is_player && mode != DF_SNAP)
    {
      GfxElement[x][y] = element;
      player->is_collecting = TRUE;
    }

    if (element == EL_SPEED_PILL)
    {
      player->move_delay_value = MOVE_DELAY_HIGH_SPEED;
    }
    else if (element == EL_EXTRA_TIME && level.time > 0)
    {
      TimeLeft += level.extra_time;

      game_panel_controls[GAME_PANEL_TIME].value = TimeLeft;

      DisplayGameControlValues();
    }
    else if (element == EL_SHIELD_NORMAL || element == EL_SHIELD_DEADLY)
    {
      int shield_time = (element == EL_SHIELD_DEADLY ?
			 level.shield_deadly_time :
			 level.shield_normal_time);

      player->shield_normal_time_left += shield_time;
      if (element == EL_SHIELD_DEADLY)
	player->shield_deadly_time_left += shield_time;
    }
    else if (element == EL_DYNAMITE ||
	     element == EL_EM_DYNAMITE ||
	     element == EL_SP_DISK_RED)
    {
      if (player->inventory_size < MAX_INVENTORY_SIZE)
	player->inventory_element[player->inventory_size++] = element;

      DrawGameDoorValues();
    }
    else if (element == EL_DYNABOMB_INCREASE_NUMBER)
    {
      player->dynabomb_count++;
      player->dynabombs_left++;
    }
    else if (element == EL_DYNABOMB_INCREASE_SIZE)
    {
      player->dynabomb_size++;
    }
    else if (element == EL_DYNABOMB_INCREASE_POWER)
    {
      player->dynabomb_xl = TRUE;
    }
    else if (IS_KEY(element))
    {
      player->key[KEY_NR(element)] = TRUE;

      DrawGameDoorValues();
    }
    else if (element == EL_DC_KEY_WHITE)
    {
      player->num_white_keys++;

      // display white keys?
      // DrawGameDoorValues();
    }
    else if (IS_ENVELOPE(element))
    {
      boolean wait_for_snapping = (mode == DF_SNAP && level.block_snap_field);

      if (!wait_for_snapping)
	player->show_envelope = element;
    }
    else if (element == EL_EMC_LENSES)
    {
      game.lenses_time_left = level.lenses_time * FRAMES_PER_SECOND;

      RedrawAllInvisibleElementsForLenses();
    }
    else if (element == EL_EMC_MAGNIFIER)
    {
      game.magnify_time_left = level.magnify_time * FRAMES_PER_SECOND;

      RedrawAllInvisibleElementsForMagnifier();
    }
    else if (IS_DROPPABLE(element) ||
	     IS_THROWABLE(element))	// can be collected and dropped
    {
      int i;

      if (collect_count == 0)
	player->inventory_infinite_element = element;
      else
	for (i = 0; i < collect_count; i++)
	  if (player->inventory_size < MAX_INVENTORY_SIZE)
	    player->inventory_element[player->inventory_size++] = element;

      DrawGameDoorValues();
    }
    else if (collect_count > 0)
    {
      game.gems_still_needed -= collect_count;
      if (game.gems_still_needed < 0)
	game.gems_still_needed = 0;

      game.snapshot.collected_item = TRUE;

      game_panel_controls[GAME_PANEL_GEMS].value = game.gems_still_needed;

      DisplayGameControlValues();
    }

    RaiseScoreElement(element);
    PlayLevelSoundElementAction(x, y, element, ACTION_COLLECTING);

    // use old behaviour for old levels (collecting)
    if (!level.finish_dig_collect && is_player)
    {
      CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_COLLECTS_X,
					  player->index_bit, dig_side);

      // if collecting triggered player relocation, finish collecting tile
      if (mode == DF_DIG && (player->jx != jx || player->jy != jy))
	SetFieldForSnapping(x, y, element, move_direction, player->index_bit);
    }

    if (mode == DF_SNAP)
    {
      if (level.block_snap_field)
	SetFieldForSnapping(x, y, element, move_direction, player->index_bit);
      else
	TestFieldAfterSnapping(x, y, element, move_direction, player->index_bit);

      // use old behaviour for old levels (snapping)
      if (!level.finish_dig_collect)
	CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_SNAPS_X,
					    player->index_bit, dig_side);
    }
  }
  else if (player_can_move_or_snap && IS_PUSHABLE(element))
  {
    if (mode == DF_SNAP && element != EL_BD_ROCK)
      return MP_NO_ACTION;

    if (CAN_FALL(element) && dy)
      return MP_NO_ACTION;

    if (CAN_FALL(element) && IN_LEV_FIELD(x, y + 1) && IS_FREE(x, y + 1) &&
	!(element == EL_SPRING && level.use_spring_bug))
      return MP_NO_ACTION;

    if (CAN_MOVE(element) && GET_MAX_MOVE_DELAY(element) == 0 &&
	((move_direction & MV_VERTICAL &&
	  ((element_info[element].move_pattern & MV_LEFT &&
	    IN_LEV_FIELD(x - 1, y) && IS_FREE(x - 1, y)) ||
	   (element_info[element].move_pattern & MV_RIGHT &&
	    IN_LEV_FIELD(x + 1, y) && IS_FREE(x + 1, y)))) ||
	 (move_direction & MV_HORIZONTAL &&
	  ((element_info[element].move_pattern & MV_UP &&
	    IN_LEV_FIELD(x, y - 1) && IS_FREE(x, y - 1)) ||
	   (element_info[element].move_pattern & MV_DOWN &&
	    IN_LEV_FIELD(x, y + 1) && IS_FREE(x, y + 1))))))
      return MP_NO_ACTION;

    // do not push elements already moving away faster than player
    if (CAN_MOVE(element) && MovDir[x][y] == move_direction &&
	ABS(getElementMoveStepsize(x, y)) > MOVE_STEPSIZE_NORMAL)
      return MP_NO_ACTION;

    if (game.engine_version >= VERSION_IDENT(3,1,0,0))
    {
      if (player->push_delay_value == -1 || !player_was_pushing)
	player->push_delay_value = GET_NEW_PUSH_DELAY(element);
    }
    else if (game.engine_version >= VERSION_IDENT(3,0,7,1))
    {
      if (player->push_delay_value == -1)
	player->push_delay_value = GET_NEW_PUSH_DELAY(element);
    }
    else if (game.engine_version >= VERSION_IDENT(2,2,0,7))
    {
      if (!player->is_pushing)
	player->push_delay_value = GET_NEW_PUSH_DELAY(element);
    }

    player->is_pushing = TRUE;
    player->is_active = TRUE;

    if (!(IN_LEV_FIELD(nextx, nexty) &&
	  (IS_FREE(nextx, nexty) ||
	   (IS_SB_ELEMENT(element) &&
	    Tile[nextx][nexty] == EL_SOKOBAN_FIELD_EMPTY) ||
	   (IS_CUSTOM_ELEMENT(element) &&
	    CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, nextx, nexty)))))
      return MP_NO_ACTION;

    if (!checkDiagonalPushing(player, x, y, real_dx, real_dy))
      return MP_NO_ACTION;

    if (player->push_delay == -1)	// new pushing; restart delay
      player->push_delay = 0;

    if (player->push_delay < player->push_delay_value &&
	!(tape.playing && tape.file_version < FILE_VERSION_2_0) &&
	element != EL_SPRING && element != EL_BALLOON)
    {
      // make sure that there is no move delay before next try to push
      if (game.engine_version >= VERSION_IDENT(3,0,7,1))
	player->move_delay = 0;

      return MP_NO_ACTION;
    }

    if (IS_CUSTOM_ELEMENT(element) &&
	CUSTOM_ELEMENT_CAN_ENTER_FIELD(element, nextx, nexty))
    {
      if (!DigFieldByCE(nextx, nexty, element))
	return MP_NO_ACTION;
    }

    if (IS_SB_ELEMENT(element))
    {
      boolean sokoban_task_solved = FALSE;

      if (element == EL_SOKOBAN_FIELD_FULL)
      {
	Back[x][y] = EL_SOKOBAN_FIELD_EMPTY;

	IncrementSokobanFieldsNeeded();
	IncrementSokobanObjectsNeeded();
      }

      if (Tile[nextx][nexty] == EL_SOKOBAN_FIELD_EMPTY)
      {
	Back[nextx][nexty] = EL_SOKOBAN_FIELD_EMPTY;

	DecrementSokobanFieldsNeeded();
	DecrementSokobanObjectsNeeded();

	// sokoban object was pushed from empty field to sokoban field
	if (Back[x][y] == EL_EMPTY)
	  sokoban_task_solved = TRUE;
      }

      Tile[x][y] = EL_SOKOBAN_OBJECT;

      if (Back[x][y] == Back[nextx][nexty])
	PlayLevelSoundAction(x, y, ACTION_PUSHING);
      else if (Back[x][y] != 0)
	PlayLevelSoundElementAction(x, y, EL_SOKOBAN_FIELD_FULL,
				    ACTION_EMPTYING);
      else
	PlayLevelSoundElementAction(nextx, nexty, EL_SOKOBAN_FIELD_EMPTY,
				    ACTION_FILLING);

      if (sokoban_task_solved &&
	  game.sokoban_fields_still_needed == 0 &&
	  game.sokoban_objects_still_needed == 0 &&
	  level.auto_exit_sokoban)
      {
	game.players_still_needed = 0;

	LevelSolved();

	PlaySound(SND_GAME_SOKOBAN_SOLVING);
      }
    }
    else
      PlayLevelSoundElementAction(x, y, element, ACTION_PUSHING);

    InitMovingField(x, y, move_direction);
    GfxAction[x][y] = ACTION_PUSHING;

    if (mode == DF_SNAP)
      ContinueMoving(x, y);
    else
      MovPos[x][y] = (dx != 0 ? dx : dy);

    Pushed[x][y] = TRUE;
    Pushed[nextx][nexty] = TRUE;

    if (game.engine_version < VERSION_IDENT(2,2,0,7))
      player->push_delay_value = GET_NEW_PUSH_DELAY(element);
    else
      player->push_delay_value = -1;	// get new value later

    // check for element change _after_ element has been pushed
    if (game.use_change_when_pushing_bug)
    {
      CheckElementChangeByPlayer(x, y, element, CE_PUSHED_BY_PLAYER,
				 player->index_bit, dig_side);
      CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PUSHES_X,
					  player->index_bit, dig_side);
    }
  }
  else if (IS_SWITCHABLE(element))
  {
    if (PLAYER_SWITCHING(player, x, y))
    {
      CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PRESSES_X,
					  player->index_bit, dig_side);

      return MP_ACTION;
    }

    player->is_switching = TRUE;
    player->switch_x = x;
    player->switch_y = y;

    PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVATING);

    if (element == EL_ROBOT_WHEEL)
    {
      Tile[x][y] = EL_ROBOT_WHEEL_ACTIVE;

      game.robot_wheel_x = x;
      game.robot_wheel_y = y;
      game.robot_wheel_active = TRUE;

      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_SP_TERMINAL)
    {
      int xx, yy;

      SCAN_PLAYFIELD(xx, yy)
      {
	if (Tile[xx][yy] == EL_SP_DISK_YELLOW)
	{
	  Bang(xx, yy);
	}
	else if (Tile[xx][yy] == EL_SP_TERMINAL)
	{
	  Tile[xx][yy] = EL_SP_TERMINAL_ACTIVE;

	  ResetGfxAnimation(xx, yy);
	  TEST_DrawLevelField(xx, yy);
	}
      }
    }
    else if (IS_BELT_SWITCH(element))
    {
      ToggleBeltSwitch(x, y);
    }
    else if (element == EL_SWITCHGATE_SWITCH_UP ||
	     element == EL_SWITCHGATE_SWITCH_DOWN ||
	     element == EL_DC_SWITCHGATE_SWITCH_UP ||
	     element == EL_DC_SWITCHGATE_SWITCH_DOWN)
    {
      ToggleSwitchgateSwitch();
    }
    else if (element == EL_LIGHT_SWITCH ||
	     element == EL_LIGHT_SWITCH_ACTIVE)
    {
      ToggleLightSwitch(x, y);
    }
    else if (element == EL_TIMEGATE_SWITCH ||
	     element == EL_DC_TIMEGATE_SWITCH)
    {
      ActivateTimegateSwitch(x, y);
    }
    else if (element == EL_BALLOON_SWITCH_LEFT  ||
	     element == EL_BALLOON_SWITCH_RIGHT ||
	     element == EL_BALLOON_SWITCH_UP    ||
	     element == EL_BALLOON_SWITCH_DOWN  ||
	     element == EL_BALLOON_SWITCH_NONE  ||
	     element == EL_BALLOON_SWITCH_ANY)
    {
      game.wind_direction = (element == EL_BALLOON_SWITCH_LEFT  ? MV_LEFT  :
			     element == EL_BALLOON_SWITCH_RIGHT ? MV_RIGHT :
			     element == EL_BALLOON_SWITCH_UP    ? MV_UP    :
			     element == EL_BALLOON_SWITCH_DOWN  ? MV_DOWN  :
			     element == EL_BALLOON_SWITCH_NONE  ? MV_NONE  :
			     move_direction);
    }
    else if (element == EL_LAMP)
    {
      Tile[x][y] = EL_LAMP_ACTIVE;
      game.lights_still_needed--;

      ResetGfxAnimation(x, y);
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_TIME_ORB_FULL)
    {
      Tile[x][y] = EL_TIME_ORB_EMPTY;

      if (level.time > 0 || level.use_time_orb_bug)
      {
	TimeLeft += level.time_orb_time;
	game.no_level_time_limit = FALSE;

	game_panel_controls[GAME_PANEL_TIME].value = TimeLeft;

	DisplayGameControlValues();
      }

      ResetGfxAnimation(x, y);
      TEST_DrawLevelField(x, y);
    }
    else if (element == EL_EMC_MAGIC_BALL_SWITCH ||
	     element == EL_EMC_MAGIC_BALL_SWITCH_ACTIVE)
    {
      int xx, yy;

      game.ball_active = !game.ball_active;

      SCAN_PLAYFIELD(xx, yy)
      {
	int e = Tile[xx][yy];

	if (game.ball_active)
	{
	  if (e == EL_EMC_MAGIC_BALL)
	    CreateField(xx, yy, EL_EMC_MAGIC_BALL_ACTIVE);
	  else if (e == EL_EMC_MAGIC_BALL_SWITCH)
	    CreateField(xx, yy, EL_EMC_MAGIC_BALL_SWITCH_ACTIVE);
	}
	else
	{
	  if (e == EL_EMC_MAGIC_BALL_ACTIVE)
	    CreateField(xx, yy, EL_EMC_MAGIC_BALL);
	  else if (e == EL_EMC_MAGIC_BALL_SWITCH_ACTIVE)
	    CreateField(xx, yy, EL_EMC_MAGIC_BALL_SWITCH);
	}
      }
    }

    CheckTriggeredElementChangeByPlayer(x, y, element, CE_SWITCH_OF_X,
					player->index_bit, dig_side);

    CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_SWITCHES_X,
					player->index_bit, dig_side);

    CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PRESSES_X,
					player->index_bit, dig_side);

    return MP_ACTION;
  }
  else
  {
    if (!PLAYER_SWITCHING(player, x, y))
    {
      player->is_switching = TRUE;
      player->switch_x = x;
      player->switch_y = y;

      CheckElementChangeByPlayer(x, y, element, CE_SWITCHED,
				 player->index_bit, dig_side);
      CheckTriggeredElementChangeByPlayer(x, y, element, CE_SWITCH_OF_X,
					  player->index_bit, dig_side);

      CheckElementChangeByPlayer(x, y, element, CE_SWITCHED_BY_PLAYER,
				 player->index_bit, dig_side);
      CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_SWITCHES_X,
					  player->index_bit, dig_side);
    }

    CheckElementChangeByPlayer(x, y, element, CE_PRESSED_BY_PLAYER,
			       player->index_bit, dig_side);
    CheckTriggeredElementChangeByPlayer(x, y, element, CE_PLAYER_PRESSES_X,
					player->index_bit, dig_side);

    return MP_NO_ACTION;
  }

  player->push_delay = -1;

  if (is_player)		// function can also be called by EL_PENGUIN
  {
    if (Tile[x][y] != element)		// really digged/collected something
    {
      player->is_collecting = !player->is_digging;
      player->is_active = TRUE;

      player->last_removed_element = element;
    }
  }

  return MP_MOVING;
}

static boolean DigFieldByCE(int x, int y, int digging_element)
{
  int element = Tile[x][y];

  if (!IS_FREE(x, y))
  {
    int action = (IS_DIGGABLE(element) ? ACTION_DIGGING :
		  IS_COLLECTIBLE(element) ? ACTION_COLLECTING :
		  ACTION_BREAKING);

    // no element can dig solid indestructible elements
    if (IS_INDESTRUCTIBLE(element) &&
	!IS_DIGGABLE(element) &&
	!IS_COLLECTIBLE(element))
      return FALSE;

    if (AmoebaNr[x][y] &&
	(element == EL_AMOEBA_FULL ||
	 element == EL_BD_AMOEBA ||
	 element == EL_AMOEBA_GROWING))
    {
      AmoebaCnt[AmoebaNr[x][y]]--;
      AmoebaCnt2[AmoebaNr[x][y]]--;
    }

    if (IS_MOVING(x, y))
      RemoveMovingField(x, y);
    else
    {
      RemoveField(x, y);
      TEST_DrawLevelField(x, y);
    }

    // if digged element was about to explode, prevent the explosion
    ExplodeField[x][y] = EX_TYPE_NONE;

    PlayLevelSoundAction(x, y, action);
  }

  Store[x][y] = EL_EMPTY;

  // this makes it possible to leave the removed element again
  if (IS_EQUAL_OR_IN_GROUP(element, MOVE_ENTER_EL(digging_element)))
    Store[x][y] = element;

  return TRUE;
}

static boolean SnapField(struct PlayerInfo *player, int dx, int dy)
{
  int jx = player->jx, jy = player->jy;
  int x = jx + dx, y = jy + dy;

  if (player->MovPos != 0 && game.engine_version >= VERSION_IDENT(2,2,0,0))
    return FALSE;

  if (!player->active || !IN_LEV_FIELD(x, y))
    return FALSE;

  if (dx && dy)
    return FALSE;

  if (!dx && !dy)
  {
    if (player->MovPos == 0)
      player->is_pushing = FALSE;

    player->is_snapping = FALSE;

    if (player->MovPos == 0)
    {
      player->is_moving = FALSE;
      player->is_digging = FALSE;
      player->is_collecting = FALSE;
    }

    return FALSE;
  }

  int snap_direction = (dx == -1 ? MV_LEFT  :
			dx == +1 ? MV_RIGHT :
			dy == -1 ? MV_UP    :
			dy == +1 ? MV_DOWN  : MV_NONE);
  boolean can_continue_snapping = (level.continuous_snapping &&
				   WasJustFalling[x][y] < CHECK_DELAY_FALLING);

  // prevent snapping with already pressed snap key when not allowed
  if (player->is_snapping && !can_continue_snapping)
    return FALSE;

  player->MovDir = snap_direction;

  if (player->MovPos == 0)
  {
    player->is_moving = FALSE;
    player->is_digging = FALSE;
    player->is_collecting = FALSE;
  }

  player->is_dropping = FALSE;
  player->is_dropping_pressed = FALSE;
  player->drop_pressed_delay = 0;

  if (DigField(player, jx, jy, x, y, 0, 0, DF_SNAP) == MP_NO_ACTION)
    return FALSE;

  player->is_snapping = TRUE;
  player->is_active = TRUE;

  if (player->MovPos == 0)
  {
    player->is_moving = FALSE;
    player->is_digging = FALSE;
    player->is_collecting = FALSE;
  }

  if (player->MovPos != 0)	// prevent graphic bugs in versions < 2.2.0
    TEST_DrawLevelField(player->last_jx, player->last_jy);

  TEST_DrawLevelField(x, y);

  return TRUE;
}

static boolean DropElement(struct PlayerInfo *player)
{
  int old_element, new_element;
  int dropx = player->jx, dropy = player->jy;
  int drop_direction = player->MovDir;
  int drop_side = drop_direction;
  int drop_element = get_next_dropped_element(player);

  /* do not drop an element on top of another element; when holding drop key
     pressed without moving, dropped element must move away before the next
     element can be dropped (this is especially important if the next element
     is dynamite, which can be placed on background for historical reasons) */
  if (PLAYER_DROPPING(player, dropx, dropy) && Tile[dropx][dropy] != EL_EMPTY)
    return MP_ACTION;

  if (IS_THROWABLE(drop_element))
  {
    dropx += GET_DX_FROM_DIR(drop_direction);
    dropy += GET_DY_FROM_DIR(drop_direction);

    if (!IN_LEV_FIELD(dropx, dropy))
      return FALSE;
  }

  old_element = Tile[dropx][dropy];	// old element at dropping position
  new_element = drop_element;		// default: no change when dropping

  // check if player is active, not moving and ready to drop
  if (!player->active || player->MovPos || player->drop_delay > 0)
    return FALSE;

  // check if player has anything that can be dropped
  if (new_element == EL_UNDEFINED)
    return FALSE;

  // only set if player has anything that can be dropped
  player->is_dropping_pressed = TRUE;

  // check if drop key was pressed long enough for EM style dynamite
  if (new_element == EL_EM_DYNAMITE && player->drop_pressed_delay < 40)
    return FALSE;

  // check if anything can be dropped at the current position
  if (IS_ACTIVE_BOMB(old_element) || old_element == EL_EXPLOSION)
    return FALSE;

  // collected custom elements can only be dropped on empty fields
  if (IS_CUSTOM_ELEMENT(new_element) && old_element != EL_EMPTY)
    return FALSE;

  if (old_element != EL_EMPTY)
    Back[dropx][dropy] = old_element;	// store old element on this field

  ResetGfxAnimation(dropx, dropy);
  ResetRandomAnimationValue(dropx, dropy);

  if (player->inventory_size > 0 ||
      player->inventory_infinite_element != EL_UNDEFINED)
  {
    if (player->inventory_size > 0)
    {
      player->inventory_size--;

      DrawGameDoorValues();

      if (new_element == EL_DYNAMITE)
	new_element = EL_DYNAMITE_ACTIVE;
      else if (new_element == EL_EM_DYNAMITE)
	new_element = EL_EM_DYNAMITE_ACTIVE;
      else if (new_element == EL_SP_DISK_RED)
	new_element = EL_SP_DISK_RED_ACTIVE;
    }

    Tile[dropx][dropy] = new_element;

    if (IN_SCR_FIELD(SCREENX(dropx), SCREENY(dropy)))
      DrawGraphicThruMask(SCREENX(dropx), SCREENY(dropy),
			  el2img(Tile[dropx][dropy]), 0);

    PlayLevelSoundAction(dropx, dropy, ACTION_DROPPING);

    // needed if previous element just changed to "empty" in the last frame
    ChangeCount[dropx][dropy] = 0;	// allow at least one more change

    CheckElementChangeByPlayer(dropx, dropy, new_element, CE_DROPPED_BY_PLAYER,
			       player->index_bit, drop_side);
    CheckTriggeredElementChangeByPlayer(dropx, dropy, new_element,
					CE_PLAYER_DROPS_X,
					player->index_bit, drop_side);

    TestIfElementTouchesCustomElement(dropx, dropy);
  }
  else		// player is dropping a dyna bomb
  {
    player->dynabombs_left--;

    Tile[dropx][dropy] = new_element;

    if (IN_SCR_FIELD(SCREENX(dropx), SCREENY(dropy)))
      DrawGraphicThruMask(SCREENX(dropx), SCREENY(dropy),
			  el2img(Tile[dropx][dropy]), 0);

    PlayLevelSoundAction(dropx, dropy, ACTION_DROPPING);
  }

  if (Tile[dropx][dropy] == new_element) // uninitialized unless CE change
    InitField_WithBug1(dropx, dropy, FALSE);

  new_element = Tile[dropx][dropy];	// element might have changed

  if (IS_CUSTOM_ELEMENT(new_element) && CAN_MOVE(new_element) &&
      element_info[new_element].move_pattern == MV_WHEN_DROPPED)
  {
    if (element_info[new_element].move_direction_initial == MV_START_AUTOMATIC)
      MovDir[dropx][dropy] = drop_direction;

    ChangeCount[dropx][dropy] = 0;	// allow at least one more change

    // do not cause impact style collision by dropping elements that can fall
    CheckCollision[dropx][dropy] = CHECK_DELAY_COLLISION;
  }

  player->drop_delay = GET_NEW_DROP_DELAY(drop_element);
  player->is_dropping = TRUE;

  player->drop_pressed_delay = 0;
  player->is_dropping_pressed = FALSE;

  player->drop_x = dropx;
  player->drop_y = dropy;

  return TRUE;
}

// ----------------------------------------------------------------------------
// game sound playing functions
// ----------------------------------------------------------------------------

static int *loop_sound_frame = NULL;
static int *loop_sound_volume = NULL;

void InitPlayLevelSound(void)
{
  int num_sounds = getSoundListSize();

  checked_free(loop_sound_frame);
  checked_free(loop_sound_volume);

  loop_sound_frame  = checked_calloc(num_sounds * sizeof(int));
  loop_sound_volume = checked_calloc(num_sounds * sizeof(int));
}

static void PlayLevelSoundExt(int x, int y, int nr, boolean is_loop_sound)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int volume, stereo_position;
  int max_distance = 8;
  int type = (is_loop_sound ? SND_CTRL_PLAY_LOOP : SND_CTRL_PLAY_SOUND);

  if ((!setup.sound_simple && !is_loop_sound) ||
      (!setup.sound_loops && is_loop_sound))
    return;

  if (!IN_LEV_FIELD(x, y) ||
      sx < -max_distance || sx >= SCR_FIELDX + max_distance ||
      sy < -max_distance || sy >= SCR_FIELDY + max_distance)
    return;

  volume = SOUND_MAX_VOLUME;

  if (!IN_SCR_FIELD(sx, sy))
  {
    int dx = ABS(sx - SCR_FIELDX / 2) - SCR_FIELDX / 2;
    int dy = ABS(sy - SCR_FIELDY / 2) - SCR_FIELDY / 2;

    volume -= volume * (dx > dy ? dx : dy) / max_distance;
  }

  stereo_position = (SOUND_MAX_LEFT +
		     (sx + max_distance) * SOUND_MAX_LEFT2RIGHT /
		     (SCR_FIELDX + 2 * max_distance));

  if (is_loop_sound)
  {
    /* This assures that quieter loop sounds do not overwrite louder ones,
       while restarting sound volume comparison with each new game frame. */

    if (loop_sound_volume[nr] > volume && loop_sound_frame[nr] == FrameCounter)
      return;

    loop_sound_volume[nr] = volume;
    loop_sound_frame[nr] = FrameCounter;
  }

  PlaySoundExt(nr, volume, stereo_position, type);
}

static void PlayLevelSound(int x, int y, int nr)
{
  PlayLevelSoundExt(x, y, nr, IS_LOOP_SOUND(nr));
}

static void PlayLevelSoundNearest(int x, int y, int sound_action)
{
  PlayLevelSound(x < LEVELX(BX1) ? LEVELX(BX1) :
		 x > LEVELX(BX2) ? LEVELX(BX2) : x,
		 y < LEVELY(BY1) ? LEVELY(BY1) :
		 y > LEVELY(BY2) ? LEVELY(BY2) : y,
		 sound_action);
}

static void PlayLevelSoundAction(int x, int y, int action)
{
  PlayLevelSoundElementAction(x, y, Tile[x][y], action);
}

static void PlayLevelSoundElementAction(int x, int y, int element, int action)
{
  int sound_effect = element_info[SND_ELEMENT(element)].sound[action];

  if (sound_effect != SND_UNDEFINED)
    PlayLevelSound(x, y, sound_effect);
}

static void PlayLevelSoundElementActionIfLoop(int x, int y, int element,
					      int action)
{
  int sound_effect = element_info[SND_ELEMENT(element)].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    PlayLevelSound(x, y, sound_effect);
}

static void PlayLevelSoundActionIfLoop(int x, int y, int action)
{
  int sound_effect = element_info[SND_ELEMENT(Tile[x][y])].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    PlayLevelSound(x, y, sound_effect);
}

static void StopLevelSoundActionIfLoop(int x, int y, int action)
{
  int sound_effect = element_info[SND_ELEMENT(Tile[x][y])].sound[action];

  if (sound_effect != SND_UNDEFINED && IS_LOOP_SOUND(sound_effect))
    StopSound(sound_effect);
}

static int getLevelMusicNr(void)
{
  int level_pos = level_nr - leveldir_current->first_level;

  if (levelset.music[level_nr] != MUS_UNDEFINED)
    return levelset.music[level_nr];		// from config file
  else
    return MAP_NOCONF_MUSIC(level_pos);		// from music dir
}

static void FadeLevelSounds(void)
{
  FadeSounds();
}

static void FadeLevelMusic(void)
{
  int music_nr = getLevelMusicNr();
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(music_nr);

  if (!strEqual(curr_music, next_music))
    FadeMusic();
}

void FadeLevelSoundsAndMusic(void)
{
  FadeLevelSounds();
  FadeLevelMusic();
}

static void PlayLevelMusic(void)
{
  int music_nr = getLevelMusicNr();
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(music_nr);

  if (!strEqual(curr_music, next_music))
    PlayMusicLoop(music_nr);
}

static int getSoundAction_BD(int sample)
{
  switch (sample)
  {
    case GD_S_STONE_PUSHING:
    case GD_S_MEGA_STONE_PUSHING:
    case GD_S_LIGHT_STONE_PUSHING:
    case GD_S_FLYING_STONE_PUSHING:
    case GD_S_WAITING_STONE_PUSHING:
    case GD_S_CHASING_STONE_PUSHING:
    case GD_S_NUT_PUSHING:
    case GD_S_NITRO_PACK_PUSHING:
    case GD_S_BLADDER_PUSHING:
    case GD_S_BOX_PUSHING:
      return ACTION_PUSHING;

    case GD_S_STONE_FALLING:
    case GD_S_MEGA_STONE_FALLING:
    case GD_S_LIGHT_STONE_FALLING:
    case GD_S_FLYING_STONE_FALLING:
    case GD_S_NUT_FALLING:
    case GD_S_DIRT_BALL_FALLING:
    case GD_S_DIRT_LOOSE_FALLING:
    case GD_S_NITRO_PACK_FALLING:
    case GD_S_FALLING_WALL_FALLING:
      return ACTION_FALLING;

    case GD_S_STONE_IMPACT:
    case GD_S_MEGA_STONE_IMPACT:
    case GD_S_LIGHT_STONE_IMPACT:
    case GD_S_FLYING_STONE_IMPACT:
    case GD_S_NUT_IMPACT:
    case GD_S_DIRT_BALL_IMPACT:
    case GD_S_DIRT_LOOSE_IMPACT:
    case GD_S_NITRO_PACK_IMPACT:
    case GD_S_FALLING_WALL_IMPACT:
      return ACTION_IMPACT;

    case GD_S_NUT_CRACKING:
      return ACTION_BREAKING;

    case GD_S_EXPANDING_WALL:
    case GD_S_WALL_REAPPEARING:
    case GD_S_SLIME:
    case GD_S_LAVA:
    case GD_S_ACID_SPREADING:
    case GD_S_BLADDER_CONVERTING:
    case GD_S_AMOEBA_GROWING:
    case GD_S_AMOEBA_2_GROWING:
      return ACTION_GROWING;

    case GD_S_DIAMOND_COLLECTING:
    case GD_S_FLYING_DIAMOND_COLLECTING:
    case GD_S_SKELETON_COLLECTING:
    case GD_S_PNEUMATIC_COLLECTING:
    case GD_S_BOMB_COLLECTING:
    case GD_S_CLOCK_COLLECTING:
    case GD_S_SWEET_COLLECTING:
    case GD_S_KEY_COLLECTING:
    case GD_S_DIAMOND_KEY_COLLECTING:
      return ACTION_COLLECTING;

    case GD_S_BOMB_PLACING:
    case GD_S_REPLICATOR:
      return ACTION_DROPPING;

    case GD_S_BLADDER_MOVING:
      return ACTION_MOVING;

    case GD_S_BLADDER_SPENDER:
    case GD_S_GRAVITY_CHANGING:
      return ACTION_CHANGING;

    case GD_S_BITER_EATING:
      return ACTION_EATING;

    case GD_S_DOOR_OPENING:
    case GD_S_CRACKING:
      return ACTION_OPENING;

    case GD_S_DIRT_WALKING:
      return ACTION_DIGGING;

    case GD_S_EMPTY_WALKING:
      return ACTION_WALKING;

    case GD_S_SWITCH_BITER:
    case GD_S_SWITCH_CREATURES:
    case GD_S_SWITCH_GRAVITY:
    case GD_S_SWITCH_EXPANDING:
    case GD_S_SWITCH_CONVEYOR:
    case GD_S_SWITCH_REPLICATOR:
    case GD_S_STIRRING:
      return ACTION_ACTIVATING;

    case GD_S_TELEPORTER:
      return ACTION_PASSING;

    case GD_S_EXPLODING:
    case GD_S_BOMB_EXPLODING:
    case GD_S_GHOST_EXPLODING:
    case GD_S_VOODOO_EXPLODING:
    case GD_S_NITRO_PACK_EXPLODING:
      return ACTION_EXPLODING;

    case GD_S_DYING:
      return ACTION_DYING;

    case GD_S_COVERING:
    case GD_S_AMOEBA:
    case GD_S_AMOEBA_2:
    case GD_S_MAGIC_WALL:
    case GD_S_PNEUMATIC_HAMMER:
    case GD_S_WATER:
    case GD_S_COW:
    case GD_S_BITER:
    case GD_S_FIREFLY:
    case GD_S_ALT_FIREFLY:
    case GD_S_BUTTER:
    case GD_S_ALT_BUTTER:
    case GD_S_STONEFLY:
    case GD_S_DRAGONFLY:
      return ACTION_ACTIVE;

    case GD_S_DIAMOND_FALLING_RANDOM:
    case GD_S_DIAMOND_FALLING_1:
    case GD_S_DIAMOND_FALLING_2:
    case GD_S_DIAMOND_FALLING_3:
    case GD_S_DIAMOND_FALLING_4:
    case GD_S_DIAMOND_FALLING_5:
    case GD_S_DIAMOND_FALLING_6:
    case GD_S_DIAMOND_FALLING_7:
    case GD_S_DIAMOND_FALLING_8:
    case GD_S_DIAMOND_IMPACT_RANDOM:
    case GD_S_DIAMOND_IMPACT_1:
    case GD_S_DIAMOND_IMPACT_2:
    case GD_S_DIAMOND_IMPACT_3:
    case GD_S_DIAMOND_IMPACT_4:
    case GD_S_DIAMOND_IMPACT_5:
    case GD_S_DIAMOND_IMPACT_6:
    case GD_S_DIAMOND_IMPACT_7:
    case GD_S_DIAMOND_IMPACT_8:
    case GD_S_FLYING_DIAMOND_FALLING_RANDOM:
    case GD_S_FLYING_DIAMOND_FALLING_1:
    case GD_S_FLYING_DIAMOND_FALLING_2:
    case GD_S_FLYING_DIAMOND_FALLING_3:
    case GD_S_FLYING_DIAMOND_FALLING_4:
    case GD_S_FLYING_DIAMOND_FALLING_5:
    case GD_S_FLYING_DIAMOND_FALLING_6:
    case GD_S_FLYING_DIAMOND_FALLING_7:
    case GD_S_FLYING_DIAMOND_FALLING_8:
    case GD_S_FLYING_DIAMOND_IMPACT_RANDOM:
    case GD_S_FLYING_DIAMOND_IMPACT_1:
    case GD_S_FLYING_DIAMOND_IMPACT_2:
    case GD_S_FLYING_DIAMOND_IMPACT_3:
    case GD_S_FLYING_DIAMOND_IMPACT_4:
    case GD_S_FLYING_DIAMOND_IMPACT_5:
    case GD_S_FLYING_DIAMOND_IMPACT_6:
    case GD_S_FLYING_DIAMOND_IMPACT_7:
    case GD_S_FLYING_DIAMOND_IMPACT_8:
    case GD_S_TIMEOUT_0:
    case GD_S_TIMEOUT_1:
    case GD_S_TIMEOUT_2:
    case GD_S_TIMEOUT_3:
    case GD_S_TIMEOUT_4:
    case GD_S_TIMEOUT_5:
    case GD_S_TIMEOUT_6:
    case GD_S_TIMEOUT_7:
    case GD_S_TIMEOUT_8:
    case GD_S_TIMEOUT_9:
    case GD_S_TIMEOUT_10:
    case GD_S_BONUS_LIFE:
      // trigger special post-processing (and force sound to be non-looping)
      return ACTION_OTHER;

    case GD_S_AMOEBA_MAGIC:
    case GD_S_AMOEBA_2_MAGIC:
    case GD_S_FINISHED:
      // trigger special post-processing (and force sound to be looping)
      return ACTION_DEFAULT;

    default:
      return ACTION_DEFAULT;
  }
}

static int getSoundEffect_BD(int element_bd, int sample)
{
  int sound_action = getSoundAction_BD(sample);
  int sound_effect = element_info[SND_ELEMENT(element_bd)].sound[sound_action];
  int nr;

  // standard sounds
  if (sound_action != ACTION_OTHER &&
      sound_action != ACTION_DEFAULT)
    return sound_effect;

  // special post-processing for some sounds
  switch (sample)
  {
    case GD_S_DIAMOND_FALLING_RANDOM:
    case GD_S_DIAMOND_FALLING_1:
    case GD_S_DIAMOND_FALLING_2:
    case GD_S_DIAMOND_FALLING_3:
    case GD_S_DIAMOND_FALLING_4:
    case GD_S_DIAMOND_FALLING_5:
    case GD_S_DIAMOND_FALLING_6:
    case GD_S_DIAMOND_FALLING_7:
    case GD_S_DIAMOND_FALLING_8:
      nr = (sample == GD_S_DIAMOND_FALLING_RANDOM ? GetSimpleRandom(8) :
	    sample - GD_S_DIAMOND_FALLING_1);
      sound_effect = SND_BDX_DIAMOND_FALLING_RANDOM_1 + nr;

      if (getSoundInfoEntryFilename(sound_effect) == NULL)
	sound_effect = SND_BDX_DIAMOND_FALLING;
      break;

    case GD_S_DIAMOND_IMPACT_RANDOM:
    case GD_S_DIAMOND_IMPACT_1:
    case GD_S_DIAMOND_IMPACT_2:
    case GD_S_DIAMOND_IMPACT_3:
    case GD_S_DIAMOND_IMPACT_4:
    case GD_S_DIAMOND_IMPACT_5:
    case GD_S_DIAMOND_IMPACT_6:
    case GD_S_DIAMOND_IMPACT_7:
    case GD_S_DIAMOND_IMPACT_8:
      nr = (sample == GD_S_DIAMOND_IMPACT_RANDOM ? GetSimpleRandom(8) :
	    sample - GD_S_DIAMOND_IMPACT_1);
      sound_effect = SND_BDX_DIAMOND_IMPACT_RANDOM_1 + nr;

      if (getSoundInfoEntryFilename(sound_effect) == NULL)
	sound_effect = SND_BDX_DIAMOND_IMPACT;
      break;

    case GD_S_FLYING_DIAMOND_FALLING_RANDOM:
    case GD_S_FLYING_DIAMOND_FALLING_1:
    case GD_S_FLYING_DIAMOND_FALLING_2:
    case GD_S_FLYING_DIAMOND_FALLING_3:
    case GD_S_FLYING_DIAMOND_FALLING_4:
    case GD_S_FLYING_DIAMOND_FALLING_5:
    case GD_S_FLYING_DIAMOND_FALLING_6:
    case GD_S_FLYING_DIAMOND_FALLING_7:
    case GD_S_FLYING_DIAMOND_FALLING_8:
      nr = (sample == GD_S_FLYING_DIAMOND_FALLING_RANDOM ? GetSimpleRandom(8) :
	    sample - GD_S_FLYING_DIAMOND_FALLING_1);
      sound_effect = SND_BDX_FLYING_DIAMOND_FALLING_RANDOM_1 + nr;

      if (getSoundInfoEntryFilename(sound_effect) == NULL)
	sound_effect = SND_BDX_FLYING_DIAMOND_FALLING;
      break;

    case GD_S_FLYING_DIAMOND_IMPACT_RANDOM:
    case GD_S_FLYING_DIAMOND_IMPACT_1:
    case GD_S_FLYING_DIAMOND_IMPACT_2:
    case GD_S_FLYING_DIAMOND_IMPACT_3:
    case GD_S_FLYING_DIAMOND_IMPACT_4:
    case GD_S_FLYING_DIAMOND_IMPACT_5:
    case GD_S_FLYING_DIAMOND_IMPACT_6:
    case GD_S_FLYING_DIAMOND_IMPACT_7:
    case GD_S_FLYING_DIAMOND_IMPACT_8:
      nr = (sample == GD_S_FLYING_DIAMOND_IMPACT_RANDOM ? GetSimpleRandom(8) :
	    sample - GD_S_FLYING_DIAMOND_IMPACT_1);
      sound_effect = SND_BDX_FLYING_DIAMOND_IMPACT_RANDOM_1 + nr;

      if (getSoundInfoEntryFilename(sound_effect) == NULL)
	sound_effect = SND_BDX_FLYING_DIAMOND_IMPACT;
      break;

    case GD_S_TIMEOUT_0:
    case GD_S_TIMEOUT_1:
    case GD_S_TIMEOUT_2:
    case GD_S_TIMEOUT_3:
    case GD_S_TIMEOUT_4:
    case GD_S_TIMEOUT_5:
    case GD_S_TIMEOUT_6:
    case GD_S_TIMEOUT_7:
    case GD_S_TIMEOUT_8:
    case GD_S_TIMEOUT_9:
    case GD_S_TIMEOUT_10:
      nr = sample - GD_S_TIMEOUT_0;
      sound_effect = SND_GAME_RUNNING_OUT_OF_TIME_0 + nr;

      if (getSoundInfoEntryFilename(sound_effect) == NULL && sample != GD_S_TIMEOUT_0)
	sound_effect = SND_GAME_RUNNING_OUT_OF_TIME;
      break;

    case GD_S_BONUS_LIFE:
      sound_effect = SND_GAME_HEALTH_BONUS;
      break;

    case GD_S_AMOEBA_MAGIC:
      sound_effect = SND_BDX_AMOEBA_1_OTHER;
      break;

    case GD_S_AMOEBA_2_MAGIC:
      sound_effect = SND_BDX_AMOEBA_2_OTHER;
      break;

    case GD_S_FINISHED:
      sound_effect = SND_GAME_LEVELTIME_BONUS;
      break;

    default:
      sound_effect = SND_UNDEFINED;
      break;
  }

  return sound_effect;
}

void PlayLevelSound_BD(int xx, int yy, int element_bd, int sample)
{
  int element = (element_bd > -1 ? map_element_BD_to_RND_game(element_bd) : 0);
  int sound_effect = getSoundEffect_BD(element, sample);
  int sound_action = getSoundAction_BD(sample);
  boolean is_loop_sound = IS_LOOP_SOUND(sound_effect);
  int offset = 0;
  int x = xx - offset;
  int y = yy - offset;

  // some sound actions are always looping in native BD game engine
  if (sound_action == ACTION_DEFAULT)
    is_loop_sound = TRUE;

  // some sound actions are always non-looping in native BD game engine
  if (sound_action == ACTION_FALLING ||
      sound_action == ACTION_MOVING ||
      sound_action == ACTION_GROWING ||
      sound_action == ACTION_OTHER)
    is_loop_sound = FALSE;

  if (sound_effect != SND_UNDEFINED)
    PlayLevelSoundExt(x, y, sound_effect, is_loop_sound);
}

void StopSound_BD(int element_bd, int sample)
{
  int element = (element_bd > -1 ? map_element_BD_to_RND_game(element_bd) : 0);
  int sound_effect = getSoundEffect_BD(element, sample);

  if (sound_effect != SND_UNDEFINED)
    StopSound(sound_effect);
}

boolean isSoundPlaying_BD(int element_bd, int sample)
{
  int element = (element_bd > -1 ? map_element_BD_to_RND_game(element_bd) : 0);
  int sound_effect = getSoundEffect_BD(element, sample);

  if (sound_effect != SND_UNDEFINED)
    return isSoundPlaying(sound_effect);

  return FALSE;
}

void SetTimeLeft(int time_left)
{
  TimeLeft = time_left;
}

void PlayLevelSound_EM(int xx, int yy, int element_em, int sample)
{
  int element = (element_em > -1 ? map_element_EM_to_RND_game(element_em) : 0);
  int offset = 0;
  int x = xx - offset;
  int y = yy - offset;

  switch (sample)
  {
    case SOUND_blank:
      PlayLevelSoundElementAction(x, y, element, ACTION_WALKING);
      break;

    case SOUND_roll:
      PlayLevelSoundElementAction(x, y, element, ACTION_PUSHING);
      break;

    case SOUND_stone:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SOUND_nut:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SOUND_crack:
      PlayLevelSoundElementAction(x, y, element, ACTION_BREAKING);
      break;

    case SOUND_bug:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SOUND_tank:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SOUND_android_clone:
      PlayLevelSoundElementAction(x, y, element, ACTION_DROPPING);
      break;

    case SOUND_android_move:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SOUND_spring:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SOUND_slurp:
      PlayLevelSoundElementAction(x, y, element, ACTION_EATING);
      break;

    case SOUND_eater:
      PlayLevelSoundElementAction(x, y, element, ACTION_WAITING);
      break;

    case SOUND_eater_eat:
      PlayLevelSoundElementAction(x, y, element, ACTION_DIGGING);
      break;

    case SOUND_alien:
      PlayLevelSoundElementAction(x, y, element, ACTION_MOVING);
      break;

    case SOUND_collect:
      PlayLevelSoundElementAction(x, y, element, ACTION_COLLECTING);
      break;

    case SOUND_diamond:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SOUND_squash:
      // !!! CHECK THIS !!!
#if 1
      PlayLevelSoundElementAction(x, y, element, ACTION_BREAKING);
#else
      PlayLevelSoundElementAction(x, y, element, ACTION_SMASHED_BY_ROCK);
#endif
      break;

    case SOUND_wonderfall:
      PlayLevelSoundElementAction(x, y, element, ACTION_FILLING);
      break;

    case SOUND_drip:
      PlayLevelSoundElementAction(x, y, element, ACTION_IMPACT);
      break;

    case SOUND_push:
      PlayLevelSoundElementAction(x, y, element, ACTION_PUSHING);
      break;

    case SOUND_dirt:
      PlayLevelSoundElementAction(x, y, element, ACTION_DIGGING);
      break;

    case SOUND_acid:
      PlayLevelSoundElementAction(x, y, element, ACTION_SPLASHING);
      break;

    case SOUND_ball:
      PlayLevelSoundElementAction(x, y, element, ACTION_DROPPING);
      break;

    case SOUND_slide:
      PlayLevelSoundElementAction(x, y, element, ACTION_GROWING);
      break;

    case SOUND_wonder:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVE);
      break;

    case SOUND_door:
      PlayLevelSoundElementAction(x, y, element, ACTION_PASSING);
      break;

    case SOUND_exit_open:
      PlayLevelSoundElementAction(x, y, element, ACTION_OPENING);
      break;

    case SOUND_exit_leave:
      PlayLevelSoundElementAction(x, y, element, ACTION_PASSING);
      break;

    case SOUND_dynamite:
      PlayLevelSoundElementAction(x, y, element, ACTION_DROPPING);
      break;

    case SOUND_tick:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVE);
      break;

    case SOUND_press:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVATING);
      break;

    case SOUND_wheel:
      PlayLevelSoundElementAction(x, y, element, ACTION_ACTIVE);
      break;

    case SOUND_boom:
      PlayLevelSoundElementAction(x, y, element, ACTION_EXPLODING);
      break;

    case SOUND_die:
      PlayLevelSoundElementAction(x, y, element, ACTION_DYING);
      break;

    case SOUND_time:
      PlaySound(SND_GAME_RUNNING_OUT_OF_TIME);
      break;

    default:
      PlayLevelSoundElementAction(x, y, element, ACTION_DEFAULT);
      break;
  }
}

void PlayLevelSound_SP(int xx, int yy, int element_sp, int action_sp)
{
  int element = map_element_SP_to_RND(element_sp);
  int action = map_action_SP_to_RND(action_sp);
  int offset = (setup.sp_show_border_elements ? 0 : 1);
  int x = xx - offset;
  int y = yy - offset;

  PlayLevelSoundElementAction(x, y, element, action);
}

void PlayLevelSound_MM(int xx, int yy, int element_mm, int action_mm)
{
  int element = map_element_MM_to_RND(element_mm);
  int action = map_action_MM_to_RND(action_mm);
  int offset = 0;
  int x = xx - offset;
  int y = yy - offset;

  if (!IS_MM_ELEMENT(element))
    element = EL_MM_DEFAULT;

  PlayLevelSoundElementAction(x, y, element, action);
}

void PlaySound_MM(int sound_mm)
{
  int sound = map_sound_MM_to_RND(sound_mm);

  if (sound == SND_UNDEFINED)
    return;

  PlaySound(sound);
}

void PlaySoundLoop_MM(int sound_mm)
{
  int sound = map_sound_MM_to_RND(sound_mm);

  if (sound == SND_UNDEFINED)
    return;

  PlaySoundLoop(sound);
}

void StopSound_MM(int sound_mm)
{
  int sound = map_sound_MM_to_RND(sound_mm);

  if (sound == SND_UNDEFINED)
    return;

  StopSound(sound);
}

void RaiseScore(int value)
{
  game.score += value;

  game_panel_controls[GAME_PANEL_SCORE].value = game.score;

  DisplayGameControlValues();
}

void RaiseScoreElement(int element)
{
  switch (element)
  {
    case EL_EMERALD:
    case EL_BD_DIAMOND:
    case EL_EMERALD_YELLOW:
    case EL_EMERALD_RED:
    case EL_EMERALD_PURPLE:
    case EL_SP_INFOTRON:
      RaiseScore(level.score[SC_EMERALD]);
      break;
    case EL_DIAMOND:
      RaiseScore(level.score[SC_DIAMOND]);
      break;
    case EL_CRYSTAL:
      RaiseScore(level.score[SC_CRYSTAL]);
      break;
    case EL_PEARL:
      RaiseScore(level.score[SC_PEARL]);
      break;
    case EL_BUG:
    case EL_BD_BUTTERFLY:
    case EL_SP_ELECTRON:
      RaiseScore(level.score[SC_BUG]);
      break;
    case EL_SPACESHIP:
    case EL_BD_FIREFLY:
    case EL_SP_SNIKSNAK:
      RaiseScore(level.score[SC_SPACESHIP]);
      break;
    case EL_YAMYAM:
    case EL_DARK_YAMYAM:
      RaiseScore(level.score[SC_YAMYAM]);
      break;
    case EL_ROBOT:
      RaiseScore(level.score[SC_ROBOT]);
      break;
    case EL_PACMAN:
      RaiseScore(level.score[SC_PACMAN]);
      break;
    case EL_NUT:
      RaiseScore(level.score[SC_NUT]);
      break;
    case EL_DYNAMITE:
    case EL_EM_DYNAMITE:
    case EL_SP_DISK_RED:
    case EL_DYNABOMB_INCREASE_NUMBER:
    case EL_DYNABOMB_INCREASE_SIZE:
    case EL_DYNABOMB_INCREASE_POWER:
      RaiseScore(level.score[SC_DYNAMITE]);
      break;
    case EL_SHIELD_NORMAL:
    case EL_SHIELD_DEADLY:
      RaiseScore(level.score[SC_SHIELD]);
      break;
    case EL_EXTRA_TIME:
      RaiseScore(level.extra_time_score);
      break;
    case EL_KEY_1:
    case EL_KEY_2:
    case EL_KEY_3:
    case EL_KEY_4:
    case EL_EM_KEY_1:
    case EL_EM_KEY_2:
    case EL_EM_KEY_3:
    case EL_EM_KEY_4:
    case EL_EMC_KEY_5:
    case EL_EMC_KEY_6:
    case EL_EMC_KEY_7:
    case EL_EMC_KEY_8:
    case EL_DC_KEY_WHITE:
      RaiseScore(level.score[SC_KEY]);
      break;
    default:
      RaiseScore(element_info[element].collect_score);
      break;
  }
}

void RequestQuitGameExt(boolean skip_request, boolean quick_quit, char *message)
{
  if (skip_request || Request(message, REQ_ASK | REQ_STAY_CLOSED))
  {
    if (!quick_quit)
    {
      // prevent short reactivation of overlay buttons while closing door
      SetOverlayActive(FALSE);
      UnmapGameButtons();

      // door may still be open due to skipped or envelope style request
      CloseDoor(score_info_tape_play ? DOOR_CLOSE_ALL : DOOR_CLOSE_1);
    }

    if (network.enabled)
    {
      SendToServer_StopPlaying(NETWORK_STOP_BY_PLAYER);
    }
    else
    {
      // when using BD game engine, cover screen before fading out
      if (!quick_quit && level.game_engine_type == GAME_ENGINE_TYPE_BD)
	game_bd.cover_screen = TRUE;

      if (quick_quit)
	FadeSkipNextFadeIn();

      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }
  }
  else		// continue playing the game
  {
    if (tape.playing && tape.deactivate_display)
      TapeDeactivateDisplayOff(TRUE);

    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

    if (tape.playing && tape.deactivate_display)
      TapeDeactivateDisplayOn();
  }
}

void RequestQuitGame(boolean escape_key_pressed)
{
  boolean ask_on_escape = (setup.ask_on_escape && setup.ask_on_quit_game);
  boolean quick_quit = ((escape_key_pressed && !ask_on_escape) ||
			level_editor_test_game);
  boolean skip_request = (game.all_players_gone || !setup.ask_on_quit_game ||
			  quick_quit || score_info_tape_play);

  RequestQuitGameExt(skip_request, quick_quit,
		     "Do you really want to quit the game?");
}

static char *getRestartGameMessage(void)
{
  boolean play_again = hasStartedNetworkGame();
  static char message[MAX_OUTPUT_LINESIZE];
  char *game_over_text = "Game over!";
  char *play_again_text = " Play it again?";

  if (level.game_engine_type == GAME_ENGINE_TYPE_MM &&
      game_mm.game_over_message != NULL)
    game_over_text = game_mm.game_over_message;

  snprintf(message, MAX_OUTPUT_LINESIZE, "%s%s", game_over_text,
	   (play_again ? play_again_text : ""));

  return message;
}

static void RequestRestartGame(void)
{
  char *message = getRestartGameMessage();
  boolean has_started_game = hasStartedNetworkGame();
  int request_mode = (has_started_game ? REQ_ASK : REQ_CONFIRM);

  boolean restart_wanted = (Request(message, request_mode | REQ_STAY_OPEN) && has_started_game);

  // if no restart wanted, continue with next level for BD style intermission levels
  if (!restart_wanted && !level_editor_test_game && level.bd_intermission)
  {
    boolean success = AdvanceToNextLevel();

    restart_wanted = (success && setup.auto_play_next_level);
  }

  if (restart_wanted)
  {
    StartGameActions(network.enabled, setup.autorecord, level.random_seed);
  }
  else
  {
    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();
  }
}

boolean CheckRestartGame(void)
{
  static int game_over_delay = 0;
  int game_over_delay_value = 50;
  boolean game_over = checkGameFailed();

  if (!game_over)
  {
    game_over_delay = game_over_delay_value;

    return FALSE;
  }

  if (game_over_delay > 0)
  {
    if (game_over_delay == game_over_delay_value / 2)
      PlaySound(SND_GAME_LOSING);

    game_over_delay--;

    return FALSE;
  }

  // do not ask to play again if request dialog is already active
  if (checkRequestActive())
    return FALSE;

  // do not ask to play again if request dialog already handled
  if (game.RestartGameRequested)
    return FALSE;

  // do not ask to play again if game was never actually played
  if (!game.GamePlayed)
    return FALSE;

  // do not ask to play again if this was disabled in setup menu
  if (!setup.ask_on_game_over)
    return FALSE;

  // do not ask to play again if playing BD game with multiple lifes
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD && setup.bd_multiple_lives &&
      game_bd.global_lives > 0)
    return FALSE;

  game.RestartGameRequested = TRUE;

  RequestRestartGame();

  return TRUE;
}

boolean checkGameRunning(void)
{
  if (game_status != GAME_MODE_PLAYING)
    return FALSE;

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD && !checkGameRunning_BD())
    return FALSE;

  return TRUE;
}

boolean checkGamePlaying(void)
{
  if (game_status != GAME_MODE_PLAYING)
    return FALSE;

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD && !checkGamePlaying_BD())
    return FALSE;

  return TRUE;
}

boolean checkGameSolved(void)
{
  // set for all game engines if level was solved
  return game.LevelSolved_GameEnd;
}

boolean checkGameFailed(void)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    return (game_bd.game_over && !game_bd.level_solved);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return (game_em.game_over && !game_em.level_solved);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return (game_sp.game_over && !game_sp.level_solved);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return (game_mm.game_over && !game_mm.level_solved);
  else				// GAME_ENGINE_TYPE_RND
    return (game.GameOver && !game.LevelSolved);
}

boolean checkGameEnded(void)
{
  // required to prevent handling game actions when moving doors (during "InitGame()")
  if (game.InitGameRequested)
    return FALSE;

  return (checkGameSolved() || checkGameFailed());
}

boolean checkRequestActive(void)
{
  return (game.request_active || game.envelope_active || game.any_door_active);
}


// ----------------------------------------------------------------------------
// random generator functions
// ----------------------------------------------------------------------------

unsigned int InitEngineRandom_RND(int seed)
{
  game.num_random_calls = 0;

  return InitEngineRandom(seed);
}

unsigned int RND(int max)
{
  if (max > 0)
  {
    game.num_random_calls++;

    return GetEngineRandom(max);
  }

  return 0;
}


// ----------------------------------------------------------------------------
// game engine snapshot handling functions
// ----------------------------------------------------------------------------

struct EngineSnapshotInfo
{
  // runtime values for custom element collect score
  int collect_score[NUM_CUSTOM_ELEMENTS];

  // runtime values for group element choice position
  int choice_pos[NUM_GROUP_ELEMENTS];

  // runtime values for belt position animations
  int belt_graphic[4][NUM_BELT_PARTS];
  int belt_anim_mode[4][NUM_BELT_PARTS];
};

static struct EngineSnapshotInfo engine_snapshot_rnd;
static char *snapshot_level_identifier = NULL;
static int snapshot_level_nr = -1;

static void SaveEngineSnapshotValues_RND(void)
{
  static int belt_base_active_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE
  };
  int i, j;

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    engine_snapshot_rnd.collect_score[i] = element_info[element].collect_score;
  }

  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
  {
    int element = EL_GROUP_START + i;

    engine_snapshot_rnd.choice_pos[i] = element_info[element].group->choice_pos;
  }

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < NUM_BELT_PARTS; j++)
    {
      int element = belt_base_active_element[i] + j;
      int graphic = el2img(element);
      int anim_mode = graphic_info[graphic].anim_mode;

      engine_snapshot_rnd.belt_graphic[i][j] = graphic;
      engine_snapshot_rnd.belt_anim_mode[i][j] = anim_mode;
    }
  }
}

static void LoadEngineSnapshotValues_RND(void)
{
  unsigned int num_random_calls = game.num_random_calls;
  int i, j;

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    element_info[element].collect_score = engine_snapshot_rnd.collect_score[i];
  }

  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
  {
    int element = EL_GROUP_START + i;

    element_info[element].group->choice_pos = engine_snapshot_rnd.choice_pos[i];
  }

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < NUM_BELT_PARTS; j++)
    {
      int graphic = engine_snapshot_rnd.belt_graphic[i][j];
      int anim_mode = engine_snapshot_rnd.belt_anim_mode[i][j];

      graphic_info[graphic].anim_mode = anim_mode;
    }
  }

  if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
  {
    InitRND(tape.random_seed);
    for (i = 0; i < num_random_calls; i++)
      RND(1);
  }

  if (game.num_random_calls != num_random_calls)
  {
    Error("number of random calls out of sync");
    Error("number of random calls should be %d", num_random_calls);
    Error("number of random calls is %d", game.num_random_calls);

    Fail("this should not happen -- please debug");
  }
}

void FreeEngineSnapshotSingle(void)
{
  FreeSnapshotSingle();

  setString(&snapshot_level_identifier, NULL);
  snapshot_level_nr = -1;
}

void FreeEngineSnapshotList(void)
{
  FreeSnapshotList();
}

static ListNode *SaveEngineSnapshotBuffers(void)
{
  ListNode *buffers = NULL;

  // copy some special values to a structure better suited for the snapshot

  if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    SaveEngineSnapshotValues_RND();
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    SaveEngineSnapshotValues_BD();
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    SaveEngineSnapshotValues_EM();
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    SaveEngineSnapshotValues_SP(&buffers);
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    SaveEngineSnapshotValues_MM();

  // save values stored in special snapshot structure

  if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(engine_snapshot_rnd));
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(engine_snapshot_bd));
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(engine_snapshot_em));
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(engine_snapshot_sp));
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(engine_snapshot_mm));

  // save further RND engine values

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(stored_player));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(game));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(tape));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(FrameCounter));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(TimeFrames));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(TimePlayed));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(TimeLeft));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(TapeTimeFrames));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(TapeTime));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ScreenMovDir));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ScreenMovPos));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ScreenGfxPos));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollStepSize));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(AmoebaCnt));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(AmoebaCnt2));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(Tile));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(MovPos));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(MovDir));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(MovDelay));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ChangeDelay));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ChangePage));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(CustomValue));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(Store));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(Store2));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(StorePlayer));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(Back));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(AmoebaNr));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(WasJustMoving));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(WasJustFalling));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(CheckCollision));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(CheckImpact));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(Stop));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(Pushed));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ChangeCount));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ChangeEvent));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ExplodePhase));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ExplodeDelay));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(ExplodeField));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(RunnerVisit));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(PlayerVisit));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(GfxFrame));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(GfxRandom));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(GfxRandomStatic));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(GfxElement));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(GfxAction));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(GfxDir));

  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(scroll_x));
  SaveSnapshotBuffer(&buffers, ARGS_ADDRESS_AND_SIZEOF(scroll_y));

#if 0
  ListNode *node = engine_snapshot_list_rnd;
  int num_bytes = 0;

  while (node != NULL)
  {
    num_bytes += ((struct EngineSnapshotNodeInfo *)node->content)->size;

    node = node->next;
  }

  Debug("game:playing:SaveEngineSnapshotBuffers",
	"size of engine snapshot: %d bytes", num_bytes);
#endif

  return buffers;
}

void SaveEngineSnapshotSingle(void)
{
  ListNode *buffers = SaveEngineSnapshotBuffers();

  // finally save all snapshot buffers to single snapshot
  SaveSnapshotSingle(buffers);

  // save level identification information
  setString(&snapshot_level_identifier, leveldir_current->identifier);
  snapshot_level_nr = level_nr;
}

boolean CheckSaveEngineSnapshotToList(void)
{
  boolean save_snapshot =
    ((game.snapshot.mode == SNAPSHOT_MODE_EVERY_STEP) ||
     (game.snapshot.mode == SNAPSHOT_MODE_EVERY_MOVE &&
      game.snapshot.changed_action) ||
     (game.snapshot.mode == SNAPSHOT_MODE_EVERY_COLLECT &&
      game.snapshot.collected_item));

  game.snapshot.changed_action = FALSE;
  game.snapshot.collected_item = FALSE;
  game.snapshot.save_snapshot = save_snapshot;

  return save_snapshot;
}

void SaveEngineSnapshotToList(void)
{
  if (game.snapshot.mode == SNAPSHOT_MODE_OFF ||
      tape.quick_resume)
    return;

  ListNode *buffers = SaveEngineSnapshotBuffers();

  // finally save all snapshot buffers to snapshot list
  SaveSnapshotToList(buffers);
}

void SaveEngineSnapshotToListInitial(void)
{
  FreeEngineSnapshotList();

  SaveEngineSnapshotToList();
}

static void LoadEngineSnapshotValues(void)
{
  // restore special values from snapshot structure

  if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    LoadEngineSnapshotValues_RND();
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    LoadEngineSnapshotValues_BD();
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    LoadEngineSnapshotValues_EM();
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    LoadEngineSnapshotValues_SP();
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    LoadEngineSnapshotValues_MM();
}

void LoadEngineSnapshotSingle(void)
{
  LoadSnapshotSingle();

  LoadEngineSnapshotValues();
}

static void LoadEngineSnapshot_Undo(int steps)
{
  LoadSnapshotFromList_Older(steps);

  LoadEngineSnapshotValues();
}

static void LoadEngineSnapshot_Redo(int steps)
{
  LoadSnapshotFromList_Newer(steps);

  LoadEngineSnapshotValues();
}

boolean CheckEngineSnapshotSingle(void)
{
  return (strEqual(snapshot_level_identifier, leveldir_current->identifier) &&
	  snapshot_level_nr == level_nr);
}

boolean CheckEngineSnapshotList(void)
{
  return CheckSnapshotList();
}


// ---------- new game button stuff -------------------------------------------

static struct
{
  int graphic;
  struct XY *pos;
  int gadget_id;
  boolean *setup_value;
  boolean allowed_on_tape;
  boolean is_touch_button;
  char *infotext;
} gamebutton_info[NUM_GAME_BUTTONS] =
{
  {
    IMG_GFX_GAME_BUTTON_STOP,			&game.button.stop,
    GAME_CTRL_ID_STOP,				NULL,
    TRUE, FALSE,				"stop game"
  },
  {
    IMG_GFX_GAME_BUTTON_PAUSE,			&game.button.pause,
    GAME_CTRL_ID_PAUSE,				NULL,
    TRUE, FALSE,				"pause game"
  },
  {
    IMG_GFX_GAME_BUTTON_PLAY,			&game.button.play,
    GAME_CTRL_ID_PLAY,				NULL,
    TRUE, FALSE,				"play game"
  },
  {
    IMG_GFX_GAME_BUTTON_UNDO,			&game.button.undo,
    GAME_CTRL_ID_UNDO,				NULL,
    TRUE, FALSE,				"undo step"
  },
  {
    IMG_GFX_GAME_BUTTON_REDO,			&game.button.redo,
    GAME_CTRL_ID_REDO,				NULL,
    TRUE, FALSE,				"redo step"
  },
  {
    IMG_GFX_GAME_BUTTON_SAVE,			&game.button.save,
    GAME_CTRL_ID_SAVE,				NULL,
    TRUE, FALSE,				"save game"
  },
  {
    IMG_GFX_GAME_BUTTON_PAUSE2,			&game.button.pause2,
    GAME_CTRL_ID_PAUSE2,			NULL,
    TRUE, FALSE,				"pause game"
  },
  {
    IMG_GFX_GAME_BUTTON_LOAD,			&game.button.load,
    GAME_CTRL_ID_LOAD,				NULL,
    TRUE, FALSE,				"load game"
  },
  {
    IMG_GFX_GAME_BUTTON_RESTART,		&game.button.restart,
    GAME_CTRL_ID_RESTART,			NULL,
    TRUE, FALSE,				"restart game"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_STOP,		&game.button.panel_stop,
    GAME_CTRL_ID_PANEL_STOP,			NULL,
    FALSE, FALSE,				"stop game"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_PAUSE,		&game.button.panel_pause,
    GAME_CTRL_ID_PANEL_PAUSE,			NULL,
    FALSE, FALSE,				"pause game"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_PLAY,		&game.button.panel_play,
    GAME_CTRL_ID_PANEL_PLAY,			NULL,
    FALSE, FALSE,				"play game"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_RESTART,		&game.button.panel_restart,
    GAME_CTRL_ID_PANEL_RESTART,			NULL,
    FALSE, FALSE,				"restart game"
  },
  {
    IMG_GFX_GAME_BUTTON_TOUCH_STOP,		&game.button.touch_stop,
    GAME_CTRL_ID_TOUCH_STOP,			NULL,
    FALSE, TRUE,				"stop game"
  },
  {
    IMG_GFX_GAME_BUTTON_TOUCH_PAUSE,		&game.button.touch_pause,
    GAME_CTRL_ID_TOUCH_PAUSE,			NULL,
    FALSE, TRUE,				"pause game"
  },
  {
    IMG_GFX_GAME_BUTTON_TOUCH_RESTART,		&game.button.touch_restart,
    GAME_CTRL_ID_TOUCH_RESTART,			NULL,
    FALSE, TRUE,				"restart game"
  },
  {
    IMG_GFX_GAME_BUTTON_SOUND_MUSIC,		&game.button.sound_music,
    SOUND_CTRL_ID_MUSIC,			&setup.sound_music,
    TRUE, FALSE,				"background music on/off"
  },
  {
    IMG_GFX_GAME_BUTTON_SOUND_LOOPS,		&game.button.sound_loops,
    SOUND_CTRL_ID_LOOPS,			&setup.sound_loops,
    TRUE, FALSE,				"sound loops on/off"
  },
  {
    IMG_GFX_GAME_BUTTON_SOUND_SIMPLE,		&game.button.sound_simple,
    SOUND_CTRL_ID_SIMPLE,			&setup.sound_simple,
    TRUE, FALSE,				"normal sounds on/off"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_SOUND_MUSIC,	&game.button.panel_sound_music,
    SOUND_CTRL_ID_PANEL_MUSIC,			&setup.sound_music,
    FALSE, FALSE,				"background music on/off"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_SOUND_LOOPS,	&game.button.panel_sound_loops,
    SOUND_CTRL_ID_PANEL_LOOPS,			&setup.sound_loops,
    FALSE, FALSE,				"sound loops on/off"
  },
  {
    IMG_GFX_GAME_BUTTON_PANEL_SOUND_SIMPLE,	&game.button.panel_sound_simple,
    SOUND_CTRL_ID_PANEL_SIMPLE,			&setup.sound_simple,
    FALSE, FALSE,				"normal sounds on/off"
  }
};

void CreateGameButtons(void)
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
  {
    int graphic = gamebutton_info[i].graphic;
    struct GraphicInfo *gfx = &graphic_info[graphic];
    struct XY *pos = gamebutton_info[i].pos;
    struct GadgetInfo *gi;
    int button_type;
    boolean checked;
    unsigned int event_mask;
    boolean is_touch_button = gamebutton_info[i].is_touch_button;
    boolean allowed_on_tape = gamebutton_info[i].allowed_on_tape;
    boolean on_tape = (tape.show_game_buttons && allowed_on_tape);
    int base_x = (is_touch_button ? 0 : on_tape ? VX : DX);
    int base_y = (is_touch_button ? 0 : on_tape ? VY : DY);
    int gd_x   = gfx->src_x;
    int gd_y   = gfx->src_y;
    int gd_xp  = gfx->src_x + gfx->pressed_xoffset;
    int gd_yp  = gfx->src_y + gfx->pressed_yoffset;
    int gd_xa  = gfx->src_x + gfx->active_xoffset;
    int gd_ya  = gfx->src_y + gfx->active_yoffset;
    int gd_xap = gfx->src_x + gfx->active_xoffset + gfx->pressed_xoffset;
    int gd_yap = gfx->src_y + gfx->active_yoffset + gfx->pressed_yoffset;
    int x = (is_touch_button ? pos->x : GDI_ACTIVE_POS(pos->x));
    int y = (is_touch_button ? pos->y : GDI_ACTIVE_POS(pos->y));
    int id = i;

    // do not use touch buttons if overlay touch buttons are disabled
    if (is_touch_button && !setup.touch.overlay_buttons)
      continue;

    if (gfx->bitmap == NULL)
    {
      game_gadget[id] = NULL;

      continue;
    }

    if (id == GAME_CTRL_ID_STOP ||
	id == GAME_CTRL_ID_PANEL_STOP ||
	id == GAME_CTRL_ID_TOUCH_STOP ||
	id == GAME_CTRL_ID_PLAY ||
	id == GAME_CTRL_ID_PANEL_PLAY ||
	id == GAME_CTRL_ID_SAVE ||
	id == GAME_CTRL_ID_LOAD ||
	id == GAME_CTRL_ID_RESTART ||
	id == GAME_CTRL_ID_PANEL_RESTART ||
	id == GAME_CTRL_ID_TOUCH_RESTART)
    {
      button_type = GD_TYPE_NORMAL_BUTTON;
      checked = FALSE;
      event_mask = GD_EVENT_RELEASED;
    }
    else if (id == GAME_CTRL_ID_UNDO ||
	     id == GAME_CTRL_ID_REDO)
    {
      button_type = GD_TYPE_NORMAL_BUTTON;
      checked = FALSE;
      event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;
    }
    else
    {
      button_type = GD_TYPE_CHECK_BUTTON;
      checked = (gamebutton_info[i].setup_value != NULL ?
		 *gamebutton_info[i].setup_value : FALSE);
      event_mask = GD_EVENT_PRESSED;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_IMAGE_ID, graphic,
		      GDI_INFO_TEXT, gamebutton_info[i].infotext,
		      GDI_X, base_x + x,
		      GDI_Y, base_y + y,
		      GDI_WIDTH, gfx->width,
		      GDI_HEIGHT, gfx->height,
		      GDI_TYPE, button_type,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_CHECKED, checked,
		      GDI_DESIGN_UNPRESSED, gfx->bitmap, gd_x, gd_y,
		      GDI_DESIGN_PRESSED, gfx->bitmap, gd_xp, gd_yp,
		      GDI_ALT_DESIGN_UNPRESSED, gfx->bitmap, gd_xa, gd_ya,
		      GDI_ALT_DESIGN_PRESSED, gfx->bitmap, gd_xap, gd_yap,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_OVERLAY_TOUCH_BUTTON, is_touch_button,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleGameButtons,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    game_gadget[id] = gi;
  }
}

void FreeGameButtons(void)
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
    FreeGadget(game_gadget[i]);
}

static void UnmapGameButtonsAtSamePosition(int id)
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
    if (i != id &&
	gamebutton_info[i].pos->x == gamebutton_info[id].pos->x &&
	gamebutton_info[i].pos->y == gamebutton_info[id].pos->y)
      UnmapGadget(game_gadget[i]);
}

static void UnmapGameButtonsAtSamePosition_All(void)
{
  if (setup.show_load_save_buttons)
  {
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_SAVE);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PAUSE2);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_LOAD);
  }
  else if (setup.show_undo_redo_buttons)
  {
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_UNDO);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PAUSE2);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_REDO);
  }
  else
  {
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_STOP);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PAUSE);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PLAY);

    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PANEL_STOP);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PANEL_PAUSE);
    UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_PANEL_PLAY);
  }
}

void MapLoadSaveButtons(void)
{
  UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_LOAD);
  UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_SAVE);

  MapGadget(game_gadget[GAME_CTRL_ID_LOAD]);
  MapGadget(game_gadget[GAME_CTRL_ID_SAVE]);
}

void MapUndoRedoButtons(void)
{
  UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_UNDO);
  UnmapGameButtonsAtSamePosition(GAME_CTRL_ID_REDO);

  MapGadget(game_gadget[GAME_CTRL_ID_UNDO]);
  MapGadget(game_gadget[GAME_CTRL_ID_REDO]);
}

void ModifyPauseButtons(void)
{
  static int ids[] =
  {
    GAME_CTRL_ID_PAUSE,
    GAME_CTRL_ID_PAUSE2,
    GAME_CTRL_ID_PANEL_PAUSE,
    GAME_CTRL_ID_TOUCH_PAUSE,
    -1
  };
  int i;

  // do not redraw pause button on closed door (may happen when restarting game)
  if (!(GetDoorState() & DOOR_OPEN_1))
    return;

  for (i = 0; ids[i] > -1; i++)
    ModifyGadget(game_gadget[ids[i]], GDI_CHECKED, tape.pausing, GDI_END);
}

static void MapGameButtonsExt(boolean on_tape)
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
  {
    if ((i == GAME_CTRL_ID_UNDO ||
	 i == GAME_CTRL_ID_REDO) &&
	game_status != GAME_MODE_PLAYING)
      continue;

    if (!on_tape || gamebutton_info[i].allowed_on_tape)
      MapGadget(game_gadget[i]);
  }

  UnmapGameButtonsAtSamePosition_All();

  RedrawGameButtons();
}

static void UnmapGameButtonsExt(boolean on_tape)
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
    if (!on_tape || gamebutton_info[i].allowed_on_tape)
      UnmapGadget(game_gadget[i]);
}

static void RedrawGameButtonsExt(boolean on_tape)
{
  int i;

  for (i = 0; i < NUM_GAME_BUTTONS; i++)
    if (!on_tape || gamebutton_info[i].allowed_on_tape)
      RedrawGadget(game_gadget[i]);
}

static void SetGadgetState(struct GadgetInfo *gi, boolean state)
{
  if (gi == NULL)
    return;

  gi->checked = state;
}

static void RedrawSoundButtonGadget(int id)
{
  int id2 = (id == SOUND_CTRL_ID_MUSIC        ? SOUND_CTRL_ID_PANEL_MUSIC :
	     id == SOUND_CTRL_ID_LOOPS        ? SOUND_CTRL_ID_PANEL_LOOPS :
	     id == SOUND_CTRL_ID_SIMPLE       ? SOUND_CTRL_ID_PANEL_SIMPLE :
	     id == SOUND_CTRL_ID_PANEL_MUSIC  ? SOUND_CTRL_ID_MUSIC :
	     id == SOUND_CTRL_ID_PANEL_LOOPS  ? SOUND_CTRL_ID_LOOPS :
	     id == SOUND_CTRL_ID_PANEL_SIMPLE ? SOUND_CTRL_ID_SIMPLE :
	     id);

  SetGadgetState(game_gadget[id2], *gamebutton_info[id2].setup_value);
  RedrawGadget(game_gadget[id2]);
}

void MapGameButtons(void)
{
  MapGameButtonsExt(FALSE);
}

void UnmapGameButtons(void)
{
  UnmapGameButtonsExt(FALSE);
}

void RedrawGameButtons(void)
{
  RedrawGameButtonsExt(FALSE);
}

void MapGameButtonsOnTape(void)
{
  MapGameButtonsExt(TRUE);
}

void UnmapGameButtonsOnTape(void)
{
  UnmapGameButtonsExt(TRUE);
}

void RedrawGameButtonsOnTape(void)
{
  RedrawGameButtonsExt(TRUE);
}

static void GameUndoRedoExt(void)
{
  ClearPlayerAction();

  tape.pausing = TRUE;

  RedrawPlayfield();
  UpdateAndDisplayGameControlValues();

  DrawCompleteVideoDisplay();
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, TapeTime);
  DrawVideoDisplay(VIDEO_STATE_FRAME_ON, FrameCounter);
  DrawVideoDisplay(VIDEO_STATE_1STEP(tape.single_step), 0);

  ModifyPauseButtons();

  BackToFront();
}

static void GameUndo(int steps)
{
  if (!CheckEngineSnapshotList())
    return;

  unsigned short tape_property_bits = tape.property_bits;

  LoadEngineSnapshot_Undo(steps);

  tape.property_bits |= tape_property_bits | TAPE_PROPERTY_SNAPSHOT;

  GameUndoRedoExt();
}

static void GameRedo(int steps)
{
  if (!CheckEngineSnapshotList())
    return;

  unsigned short tape_property_bits = tape.property_bits;

  LoadEngineSnapshot_Redo(steps);

  tape.property_bits |= tape_property_bits | TAPE_PROPERTY_SNAPSHOT;

  GameUndoRedoExt();
}

static void HandleGameButtonsExt(int id, int button)
{
  static boolean game_undo_executed = FALSE;
  int steps = BUTTON_STEPSIZE(button);
  boolean handle_game_buttons =
    (game_status == GAME_MODE_PLAYING ||
     (game_status == GAME_MODE_MAIN && tape.show_game_buttons));

  if (!handle_game_buttons)
    return;

  switch (id)
  {
    case GAME_CTRL_ID_STOP:
    case GAME_CTRL_ID_PANEL_STOP:
    case GAME_CTRL_ID_TOUCH_STOP:
      TapeStopGame();

      break;

    case GAME_CTRL_ID_PAUSE:
    case GAME_CTRL_ID_PAUSE2:
    case GAME_CTRL_ID_PANEL_PAUSE:
    case GAME_CTRL_ID_TOUCH_PAUSE:
      if (network.enabled && game_status == GAME_MODE_PLAYING)
      {
	if (tape.pausing)
	  SendToServer_ContinuePlaying();
	else
	  SendToServer_PausePlaying();
      }
      else
	TapeTogglePause(TAPE_TOGGLE_MANUAL);

      game_undo_executed = FALSE;

      break;

    case GAME_CTRL_ID_PLAY:
    case GAME_CTRL_ID_PANEL_PLAY:
      if (game_status == GAME_MODE_MAIN)
      {
        StartGameActions(network.enabled, setup.autorecord, level.random_seed);
      }
      else if (tape.pausing)
      {
	if (network.enabled)
	  SendToServer_ContinuePlaying();
	else
	  TapeTogglePause(TAPE_TOGGLE_MANUAL | TAPE_TOGGLE_PLAY_PAUSE);
      }
      break;

    case GAME_CTRL_ID_UNDO:
      // Important: When using "save snapshot when collecting an item" mode,
      // load last (current) snapshot for first "undo" after pressing "pause"
      // (else the last-but-one snapshot would be loaded, because the snapshot
      // pointer already points to the last snapshot when pressing "pause",
      // which is fine for "every step/move" mode, but not for "every collect")
      if (game.snapshot.mode == SNAPSHOT_MODE_EVERY_COLLECT &&
	  !game_undo_executed)
	steps--;

      game_undo_executed = TRUE;

      GameUndo(steps);
      break;

    case GAME_CTRL_ID_REDO:
      GameRedo(steps);
      break;

    case GAME_CTRL_ID_SAVE:
      TapeQuickSave();
      break;

    case GAME_CTRL_ID_LOAD:
      TapeQuickLoad();
      break;

    case GAME_CTRL_ID_RESTART:
    case GAME_CTRL_ID_PANEL_RESTART:
    case GAME_CTRL_ID_TOUCH_RESTART:
      TapeRestartGame();

      break;

    case SOUND_CTRL_ID_MUSIC:
    case SOUND_CTRL_ID_PANEL_MUSIC:
      if (setup.sound_music)
      { 
	setup.sound_music = FALSE;

	FadeMusic();
      }
      else if (audio.music_available)
      { 
	setup.sound = setup.sound_music = TRUE;

	SetAudioMode(setup.sound);

	if (game_status == GAME_MODE_PLAYING)
	  PlayLevelMusic();
	else
	  PlayMenuMusic();
      }

      RedrawSoundButtonGadget(id);

      break;

    case SOUND_CTRL_ID_LOOPS:
    case SOUND_CTRL_ID_PANEL_LOOPS:
      if (setup.sound_loops)
      {
	setup.sound_loops = FALSE;

	if (game_status != GAME_MODE_PLAYING)
	  FadeSounds();
      }
      else if (audio.loops_available)
      {
	setup.sound = setup.sound_loops = TRUE;

	SetAudioMode(setup.sound);

	if (game_status != GAME_MODE_PLAYING)
	  PlayMenuSound();
      }

      RedrawSoundButtonGadget(id);

      break;

    case SOUND_CTRL_ID_SIMPLE:
    case SOUND_CTRL_ID_PANEL_SIMPLE:
      if (setup.sound_simple)
      {
	setup.sound_simple = FALSE;

	if (game_status != GAME_MODE_PLAYING)
	  FadeSounds();
      }
      else if (audio.sound_available)
      {
	setup.sound = setup.sound_simple = TRUE;

	SetAudioMode(setup.sound);

	if (game_status != GAME_MODE_PLAYING)
	  PlayMenuSound();
      }

      RedrawSoundButtonGadget(id);

      break;

    default:
      break;
  }
}

static void HandleGameButtons(struct GadgetInfo *gi)
{
  HandleGameButtonsExt(gi->custom_id, gi->event.button);
}

void HandleSoundButtonKeys(Key key)
{
  if (key == setup.shortcut.sound_simple)
    ClickOnGadget(game_gadget[SOUND_CTRL_ID_SIMPLE], MB_LEFTBUTTON);
  else if (key == setup.shortcut.sound_loops)
    ClickOnGadget(game_gadget[SOUND_CTRL_ID_LOOPS], MB_LEFTBUTTON);
  else if (key == setup.shortcut.sound_music)
    ClickOnGadget(game_gadget[SOUND_CTRL_ID_MUSIC], MB_LEFTBUTTON);
}
