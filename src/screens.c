// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// screens.c
// ============================================================================

#include "libgame/libgame.h"

#include "screens.h"
#include "events.h"
#include "game.h"
#include "tools.h"
#include "editor.h"
#include "files.h"
#include "tape.h"
#include "anim.h"
#include "network.h"
#include "init.h"
#include "config.h"
#include "api.h"


#define DEBUG_JOYSTICKS				0


// screens on the info screen
#define INFO_MODE_MAIN				0
#define INFO_MODE_TITLE				1
#define INFO_MODE_ELEMENTS			2
#define INFO_MODE_MUSIC				3
#define INFO_MODE_CREDITS			4
#define INFO_MODE_PROGRAM			5
#define INFO_MODE_VERSION			6
#define INFO_MODE_LEVELSET			7
#define INFO_MODE_LEVEL				8
#define INFO_MODE_STORY				9

#define MAX_INFO_MODES				10

// screens on the setup screen
// (must match GFX_SPECIAL_ARG_SETUP_* values as defined in src/main.h)
// (should also match corresponding entries in src/conf_gfx.c)
#define SETUP_MODE_MAIN				0
#define SETUP_MODE_GAME				1
#define SETUP_MODE_ENGINES			2
#define SETUP_MODE_EDITOR			3
#define SETUP_MODE_GRAPHICS			4
#define SETUP_MODE_SOUND			5
#define SETUP_MODE_ARTWORK			6
#define SETUP_MODE_INPUT			7
#define SETUP_MODE_TOUCH			8
#define SETUP_MODE_SHORTCUTS			9
#define SETUP_MODE_SHORTCUTS_1			10
#define SETUP_MODE_SHORTCUTS_2			11
#define SETUP_MODE_SHORTCUTS_3			12
#define SETUP_MODE_SHORTCUTS_4			13
#define SETUP_MODE_SHORTCUTS_5			14
#define SETUP_MODE_SHORTCUTS_6			15
#define SETUP_MODE_SHORTCUTS_7			16

// sub-screens on the setup screen (generic)
#define SETUP_MODE_CHOOSE_ARTWORK		17
#define SETUP_MODE_CHOOSE_OTHER			18

// sub-screens on the setup screen (specific)
#define SETUP_MODE_CHOOSE_SCORES_TYPE		19
#define SETUP_MODE_CHOOSE_GAME_SPEED		20
#define SETUP_MODE_CHOOSE_SCROLL_DELAY		21
#define SETUP_MODE_CHOOSE_SNAPSHOT_MODE		22
#define SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE	23
#define SETUP_MODE_CHOOSE_BD_PALETTE_C64	24
#define SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV	25
#define SETUP_MODE_CHOOSE_BD_PALETTE_ATARI	26
#define SETUP_MODE_CHOOSE_BD_COLOR_TYPE		27
#define SETUP_MODE_CHOOSE_WINDOW_SIZE		28
#define SETUP_MODE_CHOOSE_SCALING_TYPE		29
#define SETUP_MODE_CHOOSE_RENDERING		30
#define SETUP_MODE_CHOOSE_VSYNC			31
#define SETUP_MODE_CHOOSE_GRAPHICS		32
#define SETUP_MODE_CHOOSE_SOUNDS		33
#define SETUP_MODE_CHOOSE_MUSIC			34
#define SETUP_MODE_CHOOSE_VOLUME_SIMPLE		35
#define SETUP_MODE_CHOOSE_VOLUME_LOOPS		36
#define SETUP_MODE_CHOOSE_VOLUME_MUSIC		37
#define SETUP_MODE_CHOOSE_TOUCH_CONTROL		38
#define SETUP_MODE_CHOOSE_MOVE_DISTANCE		39
#define SETUP_MODE_CHOOSE_DROP_DISTANCE		40
#define SETUP_MODE_CHOOSE_TRANSPARENCY		41
#define SETUP_MODE_CHOOSE_GRID_XSIZE_0		42
#define SETUP_MODE_CHOOSE_GRID_YSIZE_0		43
#define SETUP_MODE_CHOOSE_GRID_XSIZE_1		44
#define SETUP_MODE_CHOOSE_GRID_YSIZE_1		45
#define SETUP_MODE_CONFIG_VIRT_BUTTONS		46

#define MAX_SETUP_MODES				47

#define MAX_MENU_MODES				MAX(MAX_INFO_MODES, MAX_SETUP_MODES)

// info screen titles
#define STR_INFO_MAIN				"Info Screen"
#define STR_INFO_TITLE				"Title Screen"
#define STR_INFO_ELEMENTS			"Game Elements"
#define STR_INFO_MUSIC				"Music Info"
#define STR_INFO_CREDITS			"Credits"
#define STR_INFO_PROGRAM			"Program Info"
#define STR_INFO_VERSION			"Version Info"
#define STR_INFO_LEVELSET			"Level Set Info"
#define STR_INFO_LEVEL				"Level Info"
#define STR_INFO_STORY				"Level Story"
#define STR_INFO_EXIT				"Exit"

// setup screen titles
#define STR_SETUP_MAIN				"Setup"
#define STR_SETUP_GAME				"Game & Menu"
#define STR_SETUP_ENGINES			"Game Engines"
#define STR_SETUP_EDITOR			"Editor"
#define STR_SETUP_GRAPHICS			"Graphics"
#define STR_SETUP_SOUND				"Sound & Music"
#define STR_SETUP_ARTWORK			"Custom Artwork"
#define STR_SETUP_INPUT				"Input Devices"
#define STR_SETUP_TOUCH				"Touch Controls"
#define STR_SETUP_SHORTCUTS			"Key Shortcuts"
#define STR_SETUP_EXIT				"Exit"
#define STR_SETUP_SAVE_AND_EXIT			"Save and Exit"

#define STR_SETUP_CHOOSE_SCORES_TYPE		"Scores Type"
#define STR_SETUP_CHOOSE_GAME_SPEED		"Game Speed"
#define STR_SETUP_CHOOSE_SCROLL_DELAY		"Scroll Delay"
#define STR_SETUP_CHOOSE_SNAPSHOT_MODE		"Snapshot Mode"
#define STR_SETUP_CHOOSE_GAME_ENGINE_TYPE	"Game Engine"
#define STR_SETUP_CHOOSE_BD_PALETTE_C64		"Palette (C64)"
#define STR_SETUP_CHOOSE_BD_PALETTE_C64DTV	"Palette (C64DTV)"
#define STR_SETUP_CHOOSE_BD_PALETTE_ATARI	"Palette (Atari)"
#define STR_SETUP_CHOOSE_BD_COLOR_TYPE		"Color Type"
#define STR_SETUP_CHOOSE_WINDOW_SIZE		"Window Scaling"
#define STR_SETUP_CHOOSE_SCALING_TYPE		"Anti-Aliasing"
#define STR_SETUP_CHOOSE_RENDERING		"Rendering Mode"
#define STR_SETUP_CHOOSE_VSYNC			"VSync Mode"
#define STR_SETUP_CHOOSE_VOLUME_SIMPLE		"Sound Volume"
#define STR_SETUP_CHOOSE_VOLUME_LOOPS		"Loops Volume"
#define STR_SETUP_CHOOSE_VOLUME_MUSIC		"Music Volume"
#define STR_SETUP_CHOOSE_TOUCH_CONTROL		"Control Type"
#define STR_SETUP_CHOOSE_MOVE_DISTANCE		"Move Distance"
#define STR_SETUP_CHOOSE_DROP_DISTANCE		"Drop Distance"
#define STR_SETUP_CHOOSE_TRANSPARENCY		"Transparency"
#define STR_SETUP_CHOOSE_GRID_XSIZE_0		"Horiz. Buttons"
#define STR_SETUP_CHOOSE_GRID_YSIZE_0		"Vert. Buttons"
#define STR_SETUP_CHOOSE_GRID_XSIZE_1		"Horiz. Buttons"
#define STR_SETUP_CHOOSE_GRID_YSIZE_1		"Vert. Buttons"

// other screen text constants
#define STR_CHOOSE_TREE_EDIT			"Edit"
#define MENU_CHOOSE_TREE_FONT(x)		(FONT_TEXT_1 + (x))
#define MENU_CHOOSE_TREE_COLOR(ti, a)		TREE_COLOR(ti, a)

#define TEXT_MAIN_MENU				"Press any key or button for main menu"
#define TEXT_INFO_MENU				"Press any key or button for info menu"
#define TEXT_NEXT_PAGE				"Press any key or button for next page"
#define TEXT_INIT_GAME				"Press any key or button to start game"
#define TEXT_NEXT_MENU				(info_screens_from_main ? TEXT_MAIN_MENU :	\
						 info_screens_from_game ? TEXT_INIT_GAME :	\
						 TEXT_INFO_MENU)

// for input setup functions
#define SETUPINPUT_SCREEN_POS_START		0
#define SETUPINPUT_SCREEN_POS_EMPTY1		3
#define SETUPINPUT_SCREEN_POS_EMPTY2		12
#define SETUPINPUT_SCREEN_POS_END		13

#define MENU_SETUP_FONT_TITLE			FONT_TEXT_1
#define MENU_SETUP_FONT_TEXT			FONT_TITLE_2

#define MAX_SETUP_TEXT_INPUT_LEN		28

// for various menu stuff
#define MENU_TEXT_ALIGNED_YPOS(t)		(ALIGNED_YPOS((t).y + ((t).y < 0 ? SYSIZE : 0),	\
							      getFontHeight((t).font), (t).valign))
#define MENU_SCREEN_START_XPOS			1
#define MENU_SCREEN_START_YPOS			2
#define MENU_SCREEN_VALUE_XPOS			(SCR_FIELDX - 3)
#define MENU_SCREEN_TEXT2_XPOS			(SCR_FIELDX - 2)
#define MENU_SCREEN_MAX_XPOS			(SCR_FIELDX - 1)
#define MENU_TITLE_YPOS				MENU_TEXT_ALIGNED_YPOS(menu.text.title)
#define MENU_TITLE1_YPOS			MENU_TEXT_ALIGNED_YPOS(menu.text.title_1)
#define MENU_TITLE2_YPOS			MENU_TEXT_ALIGNED_YPOS(menu.text.title_2)
#define MENU_TITLE_STORY_YPOS			MENU_TEXT_ALIGNED_YPOS(menu.text.title_story)
#define MENU_FOOTER_YPOS			MENU_TEXT_ALIGNED_YPOS(menu.text.footer)
#define MENU_INFO_FONT_TITLE			FONT_TEXT_1
#define MENU_INFO_FONT_HEAD			FONT_TEXT_2
#define MENU_INFO_FONT_TEXT			FONT_TEXT_3
#define MENU_INFO_FONT_FOOT			FONT_FOOTER
#define MENU_INFO_SPACE_HEAD			(menu.headline2_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_LEFT		(menu.left_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_MIDDLE		(menu.middle_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_RIGHT		(menu.right_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_TOP		(menu.top_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_BOTTOM		(menu.bottom_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_LINE		(menu.line_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_EXTRA		(menu.extra_spacing_info[info_mode])
#define MENU_SCREEN_INFO_TILE_SIZE_RAW		(menu.tile_size_info[info_mode])
#define MENU_SCREEN_INFO_TILE_SIZE		(MENU_SCREEN_INFO_TILE_SIZE_RAW > 0 ?		\
						 MENU_SCREEN_INFO_TILE_SIZE_RAW : TILEY)
#define MENU_SCREEN_INFO_ENTRY_SIZE_RAW		(menu.list_entry_size_info[info_mode])
#define MENU_SCREEN_INFO_ENTRY_SIZE		(MAX(MENU_SCREEN_INFO_ENTRY_SIZE_RAW,		\
						     MENU_SCREEN_INFO_TILE_SIZE))
#define MENU_SCREEN_INFO_YSTART			MENU_SCREEN_INFO_SPACE_TOP
#define MENU_SCREEN_INFO_YSTEP			(MENU_SCREEN_INFO_ENTRY_SIZE +			\
						 MENU_SCREEN_INFO_SPACE_EXTRA)
#define MENU_SCREEN_INFO_YBOTTOM		(SYSIZE - MENU_SCREEN_INFO_SPACE_BOTTOM)
#define MENU_SCREEN_INFO_YSIZE			(MENU_SCREEN_INFO_YBOTTOM -			\
						 MENU_SCREEN_INFO_YSTART - TILEY / 2)
#define MENU_SCREEN_INFO_FOOTER			MENU_FOOTER_YPOS
#define MAX_INFO_ELEMENTS_IN_ARRAY		128
#define MAX_INFO_ELEMENTS_ON_SCREEN		(SYSIZE / TILEY)
#define MAX_INFO_ELEMENTS			MIN(MAX_INFO_ELEMENTS_IN_ARRAY,			\
						    MAX_INFO_ELEMENTS_ON_SCREEN)
#define STD_INFO_ELEMENTS_ON_SCREEN		10
#define DYN_INFO_ELEMENTS_ON_SCREEN		(MENU_SCREEN_INFO_YSIZE / MENU_SCREEN_INFO_YSTEP)
#define DEFAULT_INFO_ELEMENTS			MIN(STD_INFO_ELEMENTS_ON_SCREEN,		\
						    DYN_INFO_ELEMENTS_ON_SCREEN)
#define NUM_INFO_ELEMENTS_FROM_CONF					\
				(menu.list_size_info[GFX_SPECIAL_ARG_INFO_ELEMENTS] > 0 ?	\
				 menu.list_size_info[GFX_SPECIAL_ARG_INFO_ELEMENTS] :		\
				 DEFAULT_INFO_ELEMENTS)
#define NUM_INFO_ELEMENTS_ON_SCREEN		MIN(NUM_INFO_ELEMENTS_FROM_CONF, MAX_INFO_ELEMENTS)
#define MAX_MENU_ENTRIES_ON_SCREEN		(SCR_FIELDY - MENU_SCREEN_START_YPOS)
#define MAX_MENU_TEXT_LENGTH_BIG		13
#define MAX_MENU_TEXT_LENGTH_MEDIUM		(MAX_MENU_TEXT_LENGTH_BIG * 2)

// screen gadget identifiers
#define SCREEN_CTRL_ID_PREV_LEVEL		0
#define SCREEN_CTRL_ID_NEXT_LEVEL		1
#define SCREEN_CTRL_ID_PREV_LEVEL2		2
#define SCREEN_CTRL_ID_NEXT_LEVEL2		3
#define SCREEN_CTRL_ID_PREV_LEVEL3		4
#define SCREEN_CTRL_ID_NEXT_LEVEL3		5
#define SCREEN_CTRL_ID_PREV_SCORE		6
#define SCREEN_CTRL_ID_NEXT_SCORE		7
#define SCREEN_CTRL_ID_PLAY_TAPE		8
#define SCREEN_CTRL_ID_FIRST_LEVEL		9
#define SCREEN_CTRL_ID_LAST_LEVEL		10
#define SCREEN_CTRL_ID_LEVEL_NUMBER		11
#define SCREEN_CTRL_ID_PREV_PLAYER		12
#define SCREEN_CTRL_ID_NEXT_PLAYER		13
#define SCREEN_CTRL_ID_INSERT_SOLUTION		14
#define SCREEN_CTRL_ID_PLAY_SOLUTION		15
#define SCREEN_CTRL_ID_LEVELSET_INFO		16
#define SCREEN_CTRL_ID_LEVEL_INFO		17
#define SCREEN_CTRL_ID_SWITCH_ECS_AGA		18
#define SCREEN_CTRL_ID_TOUCH_PREV_PAGE		19
#define SCREEN_CTRL_ID_TOUCH_NEXT_PAGE		20
#define SCREEN_CTRL_ID_TOUCH_PREV_PAGE2		21
#define SCREEN_CTRL_ID_TOUCH_NEXT_PAGE2		22

#define NUM_SCREEN_MENUBUTTONS			23

#define SCREEN_CTRL_ID_SCROLL_UP		23
#define SCREEN_CTRL_ID_SCROLL_DOWN		24
#define SCREEN_CTRL_ID_SCROLL_VERTICAL		25
#define SCREEN_CTRL_ID_NETWORK_SERVER		26

#define NUM_SCREEN_GADGETS			27

#define NUM_SCREEN_SCROLLBUTTONS		2
#define NUM_SCREEN_SCROLLBARS			1
#define NUM_SCREEN_TEXTINPUT			1

#define SCREEN_MASK_MAIN			(1 << 0)
#define SCREEN_MASK_MAIN_HAS_SOLUTION		(1 << 1)
#define SCREEN_MASK_MAIN_HAS_LEVELSET_INFO	(1 << 2)
#define SCREEN_MASK_MAIN_HAS_LEVEL_INFO		(1 << 3)
#define SCREEN_MASK_INPUT			(1 << 4)
#define SCREEN_MASK_TOUCH			(1 << 5)
#define SCREEN_MASK_TOUCH2			(1 << 6)
#define SCREEN_MASK_SCORES			(1 << 7)
#define SCREEN_MASK_SCORES_INFO			(1 << 8)
#define SCREEN_MASK_INFO			(1 << 9)

// graphic position and size values for buttons and scrollbars
#define SC_MENUBUTTON_XSIZE			TILEX
#define SC_MENUBUTTON_YSIZE			TILEY

#define SC_SCROLLBUTTON_XSIZE			TILEX
#define SC_SCROLLBUTTON_YSIZE			TILEY

#define SC_SCROLLBAR_XPOS			(SXSIZE - SC_SCROLLBUTTON_XSIZE)

#define SC_SCROLL_VERTICAL_XSIZE		SC_SCROLLBUTTON_XSIZE
#define SC_SCROLL_VERTICAL_YSIZE		((MAX_MENU_ENTRIES_ON_SCREEN - 2) * \
						 SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_UP_XPOS			SC_SCROLLBAR_XPOS
#define SC_SCROLL_UP_YPOS			(2 * SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_VERTICAL_XPOS			SC_SCROLLBAR_XPOS
#define SC_SCROLL_VERTICAL_YPOS			(SC_SCROLL_UP_YPOS + \
						 SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_DOWN_XPOS			SC_SCROLLBAR_XPOS
#define SC_SCROLL_DOWN_YPOS			(SC_SCROLL_VERTICAL_YPOS + \
						 SC_SCROLL_VERTICAL_YSIZE)

#define SC_BORDER_SIZE				14


// forward declarations of internal functions
static void HandleScreenGadgets(struct GadgetInfo *);
static void HandleSetupScreen_Generic(int, int, int, int, int);
static void HandleSetupScreen_Input(int, int, int, int, int);
static void CustomizeKeyboard(int);
static void ConfigureJoystick(int);
static void ConfigureVirtualButtons(void);
static void execSetupGame(void);
static void execSetupEngines(void);
static void execSetupEditor(void);
static void execSetupGraphics(void);
static void execSetupSound(void);
static void execSetupTouch(void);
static void execSetupArtwork(void);
static void HandleChooseTree(int, int, int, int, int, TreeInfo **);

static void DrawChoosePlayerName(void);
static void DrawChooseLevelSet(void);
static void DrawChooseLevelNr(void);
static void DrawScoreInfo(int);
static void DrawScoreInfo_Content(int);
static void DrawInfoScreen(void);
static void DrawSetupScreen(void);
static void DrawTypeName(void);

static void DrawInfoScreen_NotAvailable(char *, char *);
static void DrawInfoScreen_HelpAnim(int, int, boolean);
static void DrawInfoScreen_HelpText(int, int, int, int);
static void HandleInfoScreen_Main(int, int, int, int, int);
static void HandleInfoScreen_TitleScreen(int, int, int);
static void HandleInfoScreen_Elements(int, int, int);
static void HandleInfoScreen_Music(int, int, int);
static void HandleInfoScreen_Version(int);
static void HandleInfoScreen_Generic(int, int, int, int, int);

static void ModifyGameSpeedIfNeeded(void);
static void DisableVsyncIfNeeded(void);

static void RedrawScreenMenuGadgets(int);
static void MapScreenMenuGadgets(int);
static void UnmapScreenMenuGadgets(int);
static void MapScreenGadgets(int);
static void UnmapScreenGadgets(void);
static void MapScreenTreeGadgets(TreeInfo *);
static void UnmapScreenTreeGadgets(void);
static void MapScreenInfoGadgets(void);

static void UpdateScreenMenuGadgets(int, boolean);
static void AdjustScoreInfoButtons_SelectScore(int, int, int);
static void AdjustScoreInfoButtons_PlayTape(int, int, boolean);

static boolean OfferUploadTapes(void);
static void execOfferUploadTapes(void);
static void execSaveAndExitSetup(void);

static void DrawHallOfFame_setScoreEntries(void);
static void HandleHallOfFame_SelectLevel(int, int);
static void HandleInfoScreen_SelectLevel(int, int);
static char *getHallOfFameRankText(int, int);
static char *getHallOfFameScoreText(int, int);
static char *getInfoScreenTitle_Generic(void);
static int getInfoScreenBackgroundImage_Generic(void);
static int getInfoScreenBackgroundSound_Generic(void);
static int getInfoScreenBackgroundMusic_Generic(void);

static struct TokenInfo *getSetupInfoFinal(struct TokenInfo *);

static struct GadgetInfo *screen_gadget[NUM_SCREEN_GADGETS];

static int info_mode = INFO_MODE_MAIN;
static int setup_mode = SETUP_MODE_MAIN;

static boolean info_screens_from_main = FALSE;
static boolean info_screens_from_game = FALSE;

static TreeInfo *window_sizes = NULL;
static TreeInfo *window_size_current = NULL;

static TreeInfo *scaling_types = NULL;
static TreeInfo *scaling_type_current = NULL;

static TreeInfo *rendering_modes = NULL;
static TreeInfo *rendering_mode_current = NULL;

static TreeInfo *vsync_modes = NULL;
static TreeInfo *vsync_mode_current = NULL;

static TreeInfo *scroll_delays = NULL;
static TreeInfo *scroll_delay_current = NULL;

static TreeInfo *snapshot_modes = NULL;
static TreeInfo *snapshot_mode_current = NULL;

static TreeInfo *game_engine_types = NULL;
static TreeInfo *game_engine_type_current = NULL;

static TreeInfo *bd_palettes_c64 = NULL;
static TreeInfo *bd_palette_c64_current = NULL;

static TreeInfo *bd_palettes_c64dtv = NULL;
static TreeInfo *bd_palette_c64dtv_current = NULL;

static TreeInfo *bd_palettes_atari = NULL;
static TreeInfo *bd_palette_atari_current = NULL;

static TreeInfo *bd_color_types = NULL;
static TreeInfo *bd_color_type_current = NULL;

static TreeInfo *scores_types = NULL;
static TreeInfo *scores_type_current = NULL;

static TreeInfo *game_speeds_normal = NULL;
static TreeInfo *game_speeds_extended = NULL;
static TreeInfo *game_speeds = NULL;
static TreeInfo *game_speed_current = NULL;

static TreeInfo *volumes_simple = NULL;
static TreeInfo *volume_simple_current = NULL;

static TreeInfo *volumes_loops = NULL;
static TreeInfo *volume_loops_current = NULL;

static TreeInfo *volumes_music = NULL;
static TreeInfo *volume_music_current = NULL;

static TreeInfo *touch_controls = NULL;
static TreeInfo *touch_control_current = NULL;

static TreeInfo *move_distances = NULL;
static TreeInfo *move_distance_current = NULL;

static TreeInfo *drop_distances = NULL;
static TreeInfo *drop_distance_current = NULL;

static TreeInfo *transparencies = NULL;
static TreeInfo *transparency_current = NULL;

static TreeInfo *grid_sizes[2][2] = { { NULL, NULL }, { NULL, NULL } };
static TreeInfo *grid_size_current[2][2] = { { NULL, NULL }, { NULL, NULL } };

static TreeInfo *player_name = NULL;
static TreeInfo *player_name_current = NULL;

static TreeInfo *level_number = NULL;
static TreeInfo *level_number_current = NULL;

static TreeInfo *score_entries = NULL;
static TreeInfo *score_entry_current = NULL;

static struct ValueTextInfo window_sizes_list[] =
{
  { 50,					"50 %"				},
  { 80,					"80 %"				},
  { 90,					"90 %"				},
  { 100,				"100 % (Default)"		},
  { 110,				"110 %"				},
  { 120,				"120 %"				},
  { 130,				"130 %"				},
  { 140,				"140 %"				},
  { 150,				"150 %"				},
  { 200,				"200 %"				},
  { 250,				"250 %"				},
  { 300,				"300 %"				},

  { -1,					NULL				},
};

static struct StringValueTextInfo scaling_types_list[] =
{
  { SCALING_QUALITY_NEAREST,		 "Off"				},
  { SCALING_QUALITY_LINEAR,		 "Linear"			},
  { SCALING_QUALITY_BEST,		 "Anisotropic"			},

  { NULL,				 NULL				},
};

static struct StringValueTextInfo rendering_modes_list[] =
{
  { STR_SPECIAL_RENDERING_OFF,		"Off (May show artifacts, fast)"},
  { STR_SPECIAL_RENDERING_BITMAP,	"Bitmap/Texture mode (slower)"	},
#if DEBUG
  // this mode may work under certain conditions, but does not work on Windows
  { STR_SPECIAL_RENDERING_TARGET,	"Target Texture mode (slower)"	},
#endif
  { STR_SPECIAL_RENDERING_DOUBLE,	"Double Texture mode (slower)"	},

  { NULL,				 NULL				},
};

static struct StringValueTextInfo vsync_modes_list[] =
{
  { STR_VSYNC_MODE_OFF,			"Off"				},
  { STR_VSYNC_MODE_NORMAL,		"Normal"			},
  { STR_VSYNC_MODE_ADAPTIVE,		"Adaptive"			},

  { NULL,				NULL				},
};

static struct StringValueTextInfo scores_types_list[] =
{
  { STR_SCORES_TYPE_LOCAL_ONLY,		"Local scores only"		},
  { STR_SCORES_TYPE_SERVER_ONLY,	"Server scores only"		},
  { STR_SCORES_TYPE_LOCAL_AND_SERVER,	"Local and server scores"	},

  { NULL,				NULL				},
};

static struct ValueTextInfo game_speeds_list_normal[] =
{
  { 30,					"Very Slow"			},
  { 25,					"Slow"				},
  { 20,					"Normal"			},
  { 15,					"Fast"				},
  { 10,					"Very Fast"			},

  { -1,					NULL				},
};

static struct ValueTextInfo game_speeds_list_extended[] =
{
  { 1000,				"1 fps (Extremely Slow)"	},
  { 500,				"2 fps"				},
  { 200,				"5 fps"				},
  { 100,				"10 fps"			},
  { 50,					"20 fps"			},
  { 29,					"35 fps (Original Supaplex)"	},
  { 25,					"40 fps"			},
  { 20,					"50 fps (=== Normal Speed ===)"	},
  { 16,					"60 fps (60 Hz VSync Speed)"	},
  { 14,					"70 fps (Maximum Supaplex)"	},
  { 10,					"100 fps"			},
  { 5,					"200 fps"			},
  { 2,					"500 fps"			},
  { 1,					"1000 fps (Extremely Fast)"	},

  { -1,					NULL				},
};

static struct ValueTextInfo *game_speeds_list;

static struct ValueTextInfo scroll_delays_list[] =
{
  { 0,					"0 Tiles (No Scroll Delay)"	},
  { 1,					"1 Tile"			},
  { 2,					"2 Tiles"			},
  { 3,					"3 Tiles (Default)"		},
  { 4,					"4 Tiles"			},
  { 5,					"5 Tiles"			},
  { 6,					"6 Tiles"			},
  { 7,					"7 Tiles"			},
  { 8,					"8 Tiles (Maximum Scroll Delay)"},

  { -1,					NULL				},
};

static struct StringValueTextInfo snapshot_modes_list[] =
{
  { STR_SNAPSHOT_MODE_OFF,		"Off"				},
  { STR_SNAPSHOT_MODE_EVERY_STEP,	"Every Step"			},
  { STR_SNAPSHOT_MODE_EVERY_MOVE,	"Every Move"			},
  { STR_SNAPSHOT_MODE_EVERY_COLLECT,	"Every Collect"			},

  { NULL,		 		NULL				},
};

static struct ValueTextInfo game_engine_types_list[] =
{
  { GAME_ENGINE_TYPE_RND,		"Rocks'n'Diamonds"		},
  { GAME_ENGINE_TYPE_BD,		"Boulder Dash"			},
  { GAME_ENGINE_TYPE_EM,		"Emerald Mine"			},
  { GAME_ENGINE_TYPE_SP,		"Supaplex"			},
  { GAME_ENGINE_TYPE_MM,		"Mirror Magic"			},

  { -1,					NULL				}
};

static struct ValueTextInfo bd_palettes_c64_list[] =
{
  { GD_PALETTE_C64_VICE_NEW,		"Vice new"			},
  { GD_PALETTE_C64_VICE_OLD,		"Vice old"			},
  { GD_PALETTE_C64_VIDE_DEFAULT,	"Vice default"			},
  { GD_PALETTE_C64_C64HQ,		"C64HQ"				},
  { GD_PALETTE_C64_C64S,		"C64S"				},
  { GD_PALETTE_C64_CCS64,		"CCS64"				},
  { GD_PALETTE_C64_FRODO,		"Frodo"				},
  { GD_PALETTE_C64_GODOT,		"GoDot"				},
  { GD_PALETTE_C64_PC64,		"PC64"				},
  { GD_PALETTE_C64_RTADASH,		"RTADash"			},

  { -1,					NULL				},
};

static struct ValueTextInfo bd_palettes_c64dtv_list[] =
{
  { GD_PALETTE_C64DTV_SPIFF,		"Spiff"				},
  { GD_PALETTE_C64DTV_MURRAY,		"Murray"			},

  { -1,					NULL				},
};

static struct ValueTextInfo bd_palettes_atari_list[] =
{
  { GD_PALETTE_ATARI_BUILTIN,		"BuiltIn"			},
  { GD_PALETTE_ATARI_BUILTIN_CONTRAST,	"BuiltIn contrast"		},
  { GD_PALETTE_ATARI_DEFAULT,		"Default"			},
  { GD_PALETTE_ATARI_JAKUB,		"Jakub"				},
  { GD_PALETTE_ATARI_JAKUB_CONTRAST,	"Jakub contrast"		},
  { GD_PALETTE_ATARI_REAL,		"Real"				},
  { GD_PALETTE_ATARI_REAL_CONTRAST,	"Real contrast"			},
  { GD_PALETTE_ATARI_XFORMER,		"XFormer"			},

  { -1,					NULL				},
};

static struct ValueTextInfo bd_color_types_list[] =
{
  { GD_COLOR_TYPE_RGB,			"RGB colors"			},
  { GD_COLOR_TYPE_C64,			"C64 colors"			},
  { GD_COLOR_TYPE_C64DTV,		"C64DTV colors"			},
  { GD_COLOR_TYPE_ATARI,		"Atari colors"			},

  { -1,					NULL				},
};

static struct ValueTextInfo volumes_list[] =
{
  { 0,					"0 %"				},
  { 1,					"1 %"				},
  { 2,					"2 %"				},
  { 5,					"5 %"				},
  { 10,					"10 %"				},
  { 20,					"20 %"				},
  { 30,					"30 %"				},
  { 40,					"40 %"				},
  { 50,					"50 %"				},
  { 60,					"60 %"				},
  { 70,					"70 %"				},
  { 80,					"80 %"				},
  { 90,					"90 %"				},
  { 100,				"100 %"				},

  { -1,					NULL				},
};

static struct StringValueTextInfo touch_controls_list[] =
{
  { TOUCH_CONTROL_OFF,			"Off"				},
  { TOUCH_CONTROL_VIRTUAL_BUTTONS,	"Virtual Buttons"		},
  { TOUCH_CONTROL_WIPE_GESTURES,	"Wipe Gestures"			},
  { TOUCH_CONTROL_FOLLOW_FINGER,	"Follow Finger"			},

  { NULL,			 	NULL				},
};

static struct ValueTextInfo distances_list[] =
{
  { 1,					"1 %"				},
  { 2,					"2 %"				},
  { 3,					"3 %"				},
  { 4,					"4 %"				},
  { 5,					"5 %"				},
  { 10,					"10 %"				},
  { 15,					"15 %"				},
  { 20,					"20 %"				},
  { 25,					"25 %"				},

  { -1,					NULL				},
};

static struct ValueTextInfo transparencies_list[] =
{
  { 0,					"0 % (Opaque)"			},
  { 10,					"10 %"				},
  { 20,					"20 %"				},
  { 30,					"30 %"				},
  { 40,					"40 %"				},
  { 50,					"50 %"				},
  { 60,					"60 %"				},
  { 70,					"70 %"				},
  { 80,					"80 %"				},
  { 90,					"90 %"				},
  { 100,				"100 % (Invisible)"		},

  { -1,					NULL				},
};

static struct ValueTextInfo grid_sizes_list[] =
{
  { 3,					"3"				},
  { 4,					"4"				},
  { 5,					"5"				},
  { 6,					"6"				},
  { 7,					"7"				},
  { 8,					"8"				},
  { 9,					"9"				},
  { 10,					"10"				},
  { 11,					"11"				},
  { 12,					"12"				},
  { 13,					"13"				},
  { 14,					"14"				},
  { 15,					"15"				},
  { 16,					"16"				},
  { 17,					"17"				},
  { 18,					"18"				},
  { 19,					"19"				},
  { 20,					"20"				},
  { 21,					"21"				},
  { 22,					"22"				},
  { 23,					"23"				},
  { 24,					"24"				},
  { 25,					"25"				},
  { 26,					"26"				},
  { 27,					"27"				},
  { 28,					"28"				},
  { 29,					"29"				},
  { 30,					"30"				},
  { 31,					"31"				},
  { 32,					"32"				},

  { -1,					NULL				},
};

static int align_xoffset = 0;
static int align_yoffset = 0;

#define DRAW_MODE(s)		((s) >= GAME_MODE_MAIN &&			\
				 (s) <= GAME_MODE_SETUP ? (s) :			\
				 (s) == GAME_MODE_PSEUDO_TYPENAME ?		\
				 GAME_MODE_MAIN :				\
				 (s) == GAME_MODE_PSEUDO_TYPENAMES ?		\
				 GAME_MODE_NAMES : GAME_MODE_DEFAULT)

// (there are no draw offset definitions needed for INFO_MODE_TITLE)
#define DRAW_MODE_INFO(i)	((i) >= INFO_MODE_TITLE &&			\
				 (i) <= INFO_MODE_STORY ? (i) :			\
				 INFO_MODE_MAIN)

#define DRAW_MODE_SETUP(i)	((i) >= SETUP_MODE_MAIN &&			\
				 (i) <= SETUP_MODE_SHORTCUTS_7 ? (i) :		\
				 (i) >= SETUP_MODE_CHOOSE_GRAPHICS &&		\
				 (i) <= SETUP_MODE_CHOOSE_MUSIC ?		\
				 SETUP_MODE_CHOOSE_ARTWORK :			\
				 SETUP_MODE_CHOOSE_OTHER)

#define DRAW_XOFFSET_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?		\
				 menu.draw_xoffset[GAME_MODE_INFO] :		\
				 menu.draw_xoffset_info[DRAW_MODE_INFO(i)])
#define DRAW_YOFFSET_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?		\
				 menu.draw_yoffset[GAME_MODE_INFO] :		\
				 menu.draw_yoffset_info[DRAW_MODE_INFO(i)])
#define EXTRA_SPACING_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?		\
				 menu.extra_spacing[GAME_MODE_INFO] :		\
				 menu.extra_spacing_info[DRAW_MODE_INFO(i)])

#define DRAW_XOFFSET_SETUP(i)	(DRAW_MODE_SETUP(i) == SETUP_MODE_MAIN ?	\
				 menu.draw_xoffset[GAME_MODE_SETUP] :		\
				 menu.draw_xoffset_setup[DRAW_MODE_SETUP(i)])
#define DRAW_YOFFSET_SETUP(i)	(DRAW_MODE_SETUP(i) == SETUP_MODE_MAIN ?	\
				 menu.draw_yoffset[GAME_MODE_SETUP] :		\
				 menu.draw_yoffset_setup[DRAW_MODE_SETUP(i)])
#define EXTRA_SPACING_SETUP(i)	(DRAW_MODE_SETUP(i) == SETUP_MODE_MAIN ?	\
				 menu.extra_spacing[GAME_MODE_SETUP] :		\
				 menu.extra_spacing_setup[DRAW_MODE_SETUP(i)])

#define EXTRA_SPACING_SCORES(i)	(EXTRA_SPACING_INFO(i))

#define EXTRA_SPACING_SCOREINFO(i) (menu.extra_spacing[GAME_MODE_SCOREINFO])

#define DRAW_XOFFSET(s)		((s) == GAME_MODE_INFO  ? DRAW_XOFFSET_INFO(info_mode) :	\
				 (s) == GAME_MODE_SETUP ? DRAW_XOFFSET_SETUP(setup_mode) :	\
				 menu.draw_xoffset[DRAW_MODE(s)])

#define DRAW_YOFFSET(s)		((s) == GAME_MODE_INFO  ? DRAW_YOFFSET_INFO(info_mode) :	\
				 (s) == GAME_MODE_SETUP ? DRAW_YOFFSET_SETUP(setup_mode) :	\
				 menu.draw_yoffset[DRAW_MODE(s)])

#define EXTRA_SPACING(s)	((s) == GAME_MODE_INFO   ? EXTRA_SPACING_INFO(info_mode) :	\
				 (s) == GAME_MODE_SETUP  ? EXTRA_SPACING_SETUP(setup_mode) :	\
				 (s) == GAME_MODE_SCORES ? EXTRA_SPACING_SCORES(info_mode) :	\
				 menu.extra_spacing[DRAW_MODE(s)])

#define mSX			(SX + DRAW_XOFFSET(game_status))
#define mSY			(SY + DRAW_YOFFSET(game_status))

#define amSX			(mSX + align_xoffset)
#define amSY			(mSY + align_yoffset)

#define NUM_MENU_ENTRIES_ON_SCREEN (menu.list_size[game_status] > 2 ?	\
				    menu.list_size[game_status] :	\
				    MAX_MENU_ENTRIES_ON_SCREEN)

#define IN_VIS_MENU(x, y)	IN_FIELD(x, y, SCR_FIELDX, NUM_MENU_ENTRIES_ON_SCREEN)


// title display and control definitions

#define MAX_NUM_TITLE_SCREENS	(2 * MAX_NUM_TITLE_IMAGES + 2 * MAX_NUM_TITLE_MESSAGES)

#define NO_DIRECT_LEVEL_SELECT	(-1)


static int num_title_screens = 0;

struct TitleControlInfo
{
  boolean is_image;
  boolean initial;
  boolean first;
  int local_nr;
  int sort_priority;
};

struct TitleControlInfo title_controls[MAX_NUM_TITLE_SCREENS];


// main menu display and control definitions

#define MAIN_CONTROL_NAME			0
#define MAIN_CONTROL_LEVELS			1
#define MAIN_CONTROL_SCORES			2
#define MAIN_CONTROL_EDITOR			3
#define MAIN_CONTROL_INFO			4
#define MAIN_CONTROL_GAME			5
#define MAIN_CONTROL_SETUP			6
#define MAIN_CONTROL_QUIT			7
#define MAIN_CONTROL_PREV_LEVEL			8
#define MAIN_CONTROL_NEXT_LEVEL			9
#define MAIN_CONTROL_FIRST_LEVEL		10
#define MAIN_CONTROL_LAST_LEVEL			11
#define MAIN_CONTROL_LEVEL_NUMBER		12
#define MAIN_CONTROL_LEVEL_INFO_1		13
#define MAIN_CONTROL_LEVEL_INFO_2		14
#define MAIN_CONTROL_LEVEL_NAME			15
#define MAIN_CONTROL_LEVEL_AUTHOR		16
#define MAIN_CONTROL_LEVEL_YEAR			17
#define MAIN_CONTROL_LEVEL_IMPORTED_FROM	18
#define MAIN_CONTROL_LEVEL_IMPORTED_BY		19
#define MAIN_CONTROL_LEVEL_TESTED_BY		20
#define MAIN_CONTROL_TITLE_1			21
#define MAIN_CONTROL_TITLE_2			22
#define MAIN_CONTROL_TITLE_3			23

static char str_main_text_name[10];
static char str_main_text_first_level[10];
static char str_main_text_last_level[10];
static char str_main_text_level_number[10];

static char network_server_hostname[MAX_SETUP_TEXT_INPUT_LEN + 1];

static char *main_text_name			= str_main_text_name;
static char *main_text_first_level		= str_main_text_first_level;
static char *main_text_last_level		= str_main_text_last_level;
static char *main_text_level_number		= str_main_text_level_number;
static char *main_text_levels			= "Levelset";
static char *main_text_scores			= "Hall Of Fame";
static char *main_text_editor			= "Level Creator";
static char *main_text_info			= "Info Screen";
static char *main_text_game			= "Start Game";
static char *main_text_setup			= "Setup";
static char *main_text_quit			= "Quit";
static char *main_text_level_name		= level.name;
static char *main_text_level_author		= level.author;
static char *main_text_level_year		= NULL;
static char *main_text_level_imported_from	= NULL;
static char *main_text_level_imported_by	= NULL;
static char *main_text_level_tested_by		= NULL;
static char *main_text_title_1			= NULL;
static char *main_text_title_2			= NULL;
static char *main_text_title_3			= NULL;

static struct WrappedTextInfo *wrapped_text = NULL;
static struct TitleMessageInfo *wrapped_tmi = NULL;

extern char debug_xsn_mode[];

struct MainControlInfo
{
  int nr;

  struct MenuPosInfo *pos_button;
  int button_graphic;

  struct TextPosInfo *pos_text;
  char **text;

  struct TextPosInfo *pos_input;
  char **input;
};

static struct MainControlInfo main_controls[] =
{
  {
    MAIN_CONTROL_NAME,
    &menu.main.button.name,		IMG_MENU_BUTTON_NAME,
    &menu.main.text.name,		&main_text_name,
    &menu.main.input.name,		&setup.player_name,
  },
  {
    MAIN_CONTROL_LEVELS,
    &menu.main.button.levels,		IMG_MENU_BUTTON_LEVELS,
    &menu.main.text.levels,		&main_text_levels,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_SCORES,
    &menu.main.button.scores,		IMG_MENU_BUTTON_SCORES,
    &menu.main.text.scores,		&main_text_scores,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_EDITOR,
    &menu.main.button.editor,		IMG_MENU_BUTTON_EDITOR,
    &menu.main.text.editor,		&main_text_editor,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_INFO,
    &menu.main.button.info,		IMG_MENU_BUTTON_INFO,
    &menu.main.text.info,		&main_text_info,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_GAME,
    &menu.main.button.game,		IMG_MENU_BUTTON_GAME,
    &menu.main.text.game,		&main_text_game,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_SETUP,
    &menu.main.button.setup,		IMG_MENU_BUTTON_SETUP,
    &menu.main.text.setup,		&main_text_setup,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_QUIT,
    &menu.main.button.quit,		IMG_MENU_BUTTON_QUIT,
    &menu.main.text.quit,		&main_text_quit,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_PREV_LEVEL,
    NULL,				-1,
    NULL,				NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_NEXT_LEVEL,
    NULL,				-1,
    NULL,				NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_FIRST_LEVEL,
    NULL,				-1,
    &menu.main.text.first_level,	&main_text_first_level,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LAST_LEVEL,
    NULL,				-1,
    &menu.main.text.last_level,		&main_text_last_level,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_NUMBER,
    NULL,				-1,
    &menu.main.text.level_number,	&main_text_level_number,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_INFO_1,
    NULL,				-1,
    &menu.main.text.level_info_1,	NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_INFO_2,
    NULL,				-1,
    &menu.main.text.level_info_2,	NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_NAME,
    NULL,				-1,
    &menu.main.text.level_name,		&main_text_level_name,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_AUTHOR,
    NULL,				-1,
    &menu.main.text.level_author,	&main_text_level_author,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_YEAR,
    NULL,				-1,
    &menu.main.text.level_year,		&main_text_level_year,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_IMPORTED_FROM,
    NULL,				-1,
    &menu.main.text.level_imported_from, &main_text_level_imported_from,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_IMPORTED_BY,
    NULL,				-1,
    &menu.main.text.level_imported_by,	&main_text_level_imported_by,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_TESTED_BY,
    NULL,				-1,
    &menu.main.text.level_tested_by,	&main_text_level_tested_by,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_TITLE_1,
    NULL,				-1,
    &menu.main.text.title_1,		&main_text_title_1,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_TITLE_2,
    NULL,				-1,
    &menu.main.text.title_2,		&main_text_title_2,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_TITLE_3,
    NULL,				-1,
    &menu.main.text.title_3,		&main_text_title_3,
    NULL,				NULL,
  },

  {
    -1,
    NULL,				-1,
    NULL,				NULL,
    NULL,				NULL,
  }
};

static char *getInfoTextBuffer_BD(char *text_raw)
{
  static char *text_final = NULL;
  int max_text_size = 3 * strlen(text_raw) + 1;

  checked_free(text_final);

  text_final = checked_calloc(max_text_size);

  unsigned char *src = (unsigned char *)text_raw;
  unsigned char *dst = (unsigned char *)text_final;

  while (*src)
  {
    // add two spaces (to indicate line break) before single newlines
    if (src[0] != '\n' && src[1] == '\n' && src[2] != '\n')
    {
      *dst++ = *src++;
      *dst++ = ' ';
      *dst++ = ' ';
    }

    *dst++ = *src++;
  }

  // only use the smallest possible string buffer size
  text_final = checked_realloc(text_final, strlen(text_final) + 1);

  return text_final;
}

static void addLineOrTextToInfoBuffer(char **buffer, char *header, char *text, boolean centered)
{
  if (strlen(*buffer) > 0)
    appendStringPrint(buffer, "\n\n");

  // do not use centered text if too long
  if (strlen(text) > 80)
    centered = FALSE;

  // header always centered
  appendStringPrint(buffer, "# .centered: true\n");

  appendStringPrint(buffer, "# .font: font.text_2\n");
  appendStringPrint(buffer, "%s\n\n", header);

  // text may or may not be centered
  if (!centered)
    appendStringPrint(buffer, "# .centered: false\n");

  appendStringPrint(buffer, "# .font: font.text_3\n");
  appendStringPrint(buffer, "%s", getInfoTextBuffer_BD(text));
}

static void addLineToInfoBuffer(char **buffer, char *header, char *text)
{
  addLineOrTextToInfoBuffer(buffer, header, text, TRUE);
}

static void addTextToInfoBuffer(char **buffer, char *header, char *text)
{
  addLineOrTextToInfoBuffer(buffer, header, text, FALSE);
}

static char *getLevelSetInfoBuffer(boolean from_info_menu)
{
  static char *buffer = NULL;

  if (level.file_info.type != LEVEL_FILE_TYPE_BD ||
      level.native_bd_level->caveset == NULL)
    return NULL;

  checked_free(buffer);

  buffer = getStringCopy("");	// start with empty buffer

  if (from_info_menu)
  {
    if (*level.native_bd_level->caveset->name)
      addLineToInfoBuffer(&buffer, "Cave Set Name", level.native_bd_level->caveset->name);

    if (*level.native_bd_level->caveset->author)
      addLineToInfoBuffer(&buffer, "Author", level.native_bd_level->caveset->author);

    if (*level.native_bd_level->caveset->date)
      addLineToInfoBuffer(&buffer, "Date", level.native_bd_level->caveset->date);

    if (*level.native_bd_level->caveset->difficulty)
      addLineToInfoBuffer(&buffer, "Difficulty", level.native_bd_level->caveset->difficulty);

    if (*level.native_bd_level->caveset->www)
      addLineToInfoBuffer(&buffer, "Web Site", level.native_bd_level->caveset->www);
  }

  if (*level.native_bd_level->caveset->description)
    addLineToInfoBuffer(&buffer, "Description", level.native_bd_level->caveset->description);

  if (level.native_bd_level->caveset->story)
    addTextToInfoBuffer(&buffer, "Story", level.native_bd_level->caveset->story);

  if (level.native_bd_level->caveset->remark)
    addTextToInfoBuffer(&buffer, "Remark", level.native_bd_level->caveset->remark);

  if (strlen(buffer) > 0)
    return buffer;

  return NULL;
}

static char *getLevelInfoBuffer(boolean from_info_menu)
{
  static char *buffer = NULL;

  if (level.file_info.type != LEVEL_FILE_TYPE_BD ||
      level.native_bd_level->cave == NULL)
    return NULL;

  checked_free(buffer);

  buffer = getStringCopy("");	// start with empty buffer

  if (from_info_menu)
  {
    if (*level.native_bd_level->cave->name)
      addLineToInfoBuffer(&buffer, "Cave Name", level.native_bd_level->cave->name);

    if (*level.native_bd_level->cave->author)
      addLineToInfoBuffer(&buffer, "Author", level.native_bd_level->cave->author);

    if (*level.native_bd_level->cave->date)
      addLineToInfoBuffer(&buffer, "Date", level.native_bd_level->cave->date);

    if (*level.native_bd_level->cave->difficulty)
      addLineToInfoBuffer(&buffer, "Difficulty", level.native_bd_level->cave->difficulty);

    if (*level.native_bd_level->cave->www)
      addLineToInfoBuffer(&buffer, "Web Site", level.native_bd_level->cave->www);
  }

  if (*level.native_bd_level->cave->description)
    addLineToInfoBuffer(&buffer, "Description", level.native_bd_level->cave->description);

  if (from_info_menu)
  {
    if (level.native_bd_level->cave->story)
      addTextToInfoBuffer(&buffer, "Story", level.native_bd_level->cave->story);
  }

  if (level.native_bd_level->cave->remark)
    addTextToInfoBuffer(&buffer, "Remark", level.native_bd_level->cave->remark);

  if (strlen(buffer) > 0)
    return buffer;

  return NULL;
}

static char *getLevelStoryBuffer(void)
{
  if (level.file_info.type != LEVEL_FILE_TYPE_BD ||
      level.native_bd_level->cave == NULL)
    return NULL;

  if (level.native_bd_level->cave->story)
    return getInfoTextBuffer_BD(level.native_bd_level->cave->story);

  return NULL;
}

static boolean hasLevelSetInfo(boolean from_info_menu)
{
  return (getLevelSetInfoFilename(0) != NULL ||
          getLevelSetInfoBuffer(from_info_menu) != NULL);
}

static boolean hasLevelInfo(boolean from_info_menu)
{
  return (getLevelInfoFilename(level_nr) != NULL ||
          getLevelInfoBuffer(from_info_menu) != NULL);
}

static boolean hasLevelStory(void)
{
  return (getLevelStoryFilename(level_nr) != NULL ||
          getLevelStoryBuffer() != NULL);
}

static int getTitleScreenGraphic(int nr, boolean initial)
{
  return (initial ? IMG_TITLESCREEN_INITIAL_1 : IMG_TITLESCREEN_1) + nr;
}

static struct TitleMessageInfo *getTitleMessageInfo(int nr, boolean initial)
{
  return (initial ? &titlemessage_initial[nr] : &titlemessage[nr]);
}

#if 0
static int getTitleScreenGameMode(boolean initial)
{
  return (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
}
#endif

static int getTitleMessageGameMode(boolean initial)
{
  return (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
}

static int getTitleAnimMode(struct TitleControlInfo *tci)
{
  int base = (tci->initial ? GAME_MODE_TITLE_INITIAL_1 : GAME_MODE_TITLE_1);

  return base + tci->local_nr;
}

#if 0
static int getTitleScreenBackground(boolean initial)
{
  return (initial ? IMG_BACKGROUND_TITLE_INITIAL : IMG_BACKGROUND_TITLE);
}
#endif

#if 0
static int getTitleMessageBackground(int nr, boolean initial)
{
  return (initial ? IMG_BACKGROUND_TITLE_INITIAL : IMG_BACKGROUND_TITLE);
}
#endif

static int getTitleBackground(int nr, boolean initial, boolean is_image)
{
  int base = (is_image ?
	      (initial ? IMG_BACKGROUND_TITLESCREEN_INITIAL_1 :
	                 IMG_BACKGROUND_TITLESCREEN_1) :
	      (initial ? IMG_BACKGROUND_TITLEMESSAGE_INITIAL_1 :
	                 IMG_BACKGROUND_TITLEMESSAGE_1));
  int graphic_global = (initial ? IMG_BACKGROUND_TITLE_INITIAL :
			          IMG_BACKGROUND_TITLE);
  int graphic_local = base + nr;

  if (graphic_info[graphic_local].bitmap != NULL)
    return graphic_local;

  if (graphic_info[graphic_global].bitmap != NULL)
    return graphic_global;

  return IMG_UNDEFINED;
}

static int getTitleSound(struct TitleControlInfo *tci)
{
  boolean is_image = tci->is_image;
  int initial = tci->initial;
  int nr = tci->local_nr;
  int mode = (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
  int base = (is_image ?
	      (initial ? SND_BACKGROUND_TITLESCREEN_INITIAL_1 :
	                 SND_BACKGROUND_TITLESCREEN_1) :
	      (initial ? SND_BACKGROUND_TITLEMESSAGE_INITIAL_1 :
	                 SND_BACKGROUND_TITLEMESSAGE_1));
  int sound_global = menu.sound[mode];
  int sound_local = base + nr;

#if 0
  Debug("screens:getTitleSound", "%d, %d, %d: %d ['%s'], %d ['%s']",
	nr, initial, is_image,
	sound_global, getSoundListEntry(sound_global)->filename,
	sound_local, getSoundListEntry(sound_local)->filename);
#endif

  if (!strEqual(getSoundListEntry(sound_local)->filename, UNDEFINED_FILENAME))
    return sound_local;

  if (!strEqual(getSoundListEntry(sound_global)->filename, UNDEFINED_FILENAME))
    return sound_global;

  return SND_UNDEFINED;
}

static int getTitleMusic(struct TitleControlInfo *tci)
{
  boolean is_image = tci->is_image;
  int initial = tci->initial;
  int nr = tci->local_nr;
  int mode = (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
  int base = (is_image ?
	      (initial ? MUS_BACKGROUND_TITLESCREEN_INITIAL_1 :
	                 MUS_BACKGROUND_TITLESCREEN_1) :
	      (initial ? MUS_BACKGROUND_TITLEMESSAGE_INITIAL_1 :
	                 MUS_BACKGROUND_TITLEMESSAGE_1));
  int music_global = menu.music[mode];
  int music_local = base + nr;

#if 0
  Debug("screens:getTitleMusic", "%d, %d, %d: %d ['%s'], %d ['%s']",
	nr, initial, is_image,
	music_global, getMusicListEntry(music_global)->filename,
	music_local, getMusicListEntry(music_local)->filename);
#endif

  if (!strEqual(getMusicListEntry(music_local)->filename, UNDEFINED_FILENAME))
    return music_local;

  if (!strEqual(getMusicListEntry(music_global)->filename, UNDEFINED_FILENAME))
    return music_global;

  return MUS_UNDEFINED;
}

static struct TitleFadingInfo getTitleFading(struct TitleControlInfo *tci)
{
  boolean is_image = tci->is_image;
  boolean initial = tci->initial;
  boolean first = tci->first;
  int nr = tci->local_nr;
  struct TitleMessageInfo tmi;
  struct TitleFadingInfo ti;

  tmi = (is_image ? (initial ? (first ?
				titlescreen_initial_first[nr] :
				titlescreen_initial[nr])
		             : (first ?
				titlescreen_first[nr] :
				titlescreen[nr]))
	          : (initial ? (first ?
				titlemessage_initial_first[nr] :
				titlemessage_initial[nr])
		             : (first ?
				titlemessage_first[nr] :
				titlemessage[nr])));

  ti.fade_mode  = tmi.fade_mode;
  ti.fade_delay = tmi.fade_delay;
  ti.post_delay = tmi.post_delay;
  ti.auto_delay = tmi.auto_delay;
  ti.auto_delay_unit = tmi.auto_delay_unit;

  return ti;
}

static int compareTitleControlInfo(const void *object1, const void *object2)
{
  const struct TitleControlInfo *tci1 = (struct TitleControlInfo *)object1;
  const struct TitleControlInfo *tci2 = (struct TitleControlInfo *)object2;
  int compare_result;

  if (tci1->initial != tci2->initial)
    compare_result = (tci1->initial ? -1 : +1);
  else if (tci1->sort_priority != tci2->sort_priority)
    compare_result = tci1->sort_priority - tci2->sort_priority;
  else if (tci1->is_image != tci2->is_image)
    compare_result = (tci1->is_image ? -1 : +1);
  else
    compare_result = tci1->local_nr - tci2->local_nr;

  return compare_result;
}

static boolean CheckTitleScreen_BD(int nr, boolean initial)
{
  // only show BD style title screen for native BD level sets
  if (level.game_engine_type != GAME_ENGINE_TYPE_BD)
    return FALSE;

  // only show BD style title screen for first title screen of a level set
  if (initial || nr != 0)
    return FALSE;

  int graphic = getTitleScreenGraphic(nr, initial);
  Bitmap *bitmap = graphic_info[graphic].bitmap;

  // only show BD style title screen if no other title screen defined
  if (bitmap != NULL)
    return FALSE;

  // check if BD style title screen defined
  return (GetTitleScreenBitmaps_BD() != NULL);
}

static void InitializeTitleControlsExt_AddTitleInfo(boolean is_image,
						    boolean initial,
						    int nr, int sort_priority)
{
  title_controls[num_title_screens].is_image = is_image;
  title_controls[num_title_screens].initial = initial;
  title_controls[num_title_screens].local_nr = nr;
  title_controls[num_title_screens].sort_priority = sort_priority;

  title_controls[num_title_screens].first = FALSE;	// will be set later

  num_title_screens++;
}

static void InitializeTitleControls_CheckTitleInfo(boolean initial)
{
  int i;

  for (i = 0; i < MAX_NUM_TITLE_IMAGES; i++)
  {
    int graphic = getTitleScreenGraphic(i, initial);
    Bitmap *bitmap = graphic_info[graphic].bitmap;
    int sort_priority = graphic_info[graphic].sort_priority;
    boolean has_title_screen = (bitmap != NULL);

    // check for optional title screen of native BD style level set
    if (CheckTitleScreen_BD(i, initial))
      has_title_screen = TRUE;

    if (has_title_screen)
      InitializeTitleControlsExt_AddTitleInfo(TRUE, initial, i, sort_priority);
  }

  for (i = 0; i < MAX_NUM_TITLE_MESSAGES; i++)
  {
    struct TitleMessageInfo *tmi = getTitleMessageInfo(i, initial);
    char *filename = getLevelSetTitleMessageFilename(i, initial);
    int sort_priority = tmi->sort_priority;

    if (filename != NULL)
      InitializeTitleControlsExt_AddTitleInfo(FALSE, initial, i, sort_priority);
  }
}

static void InitializeTitleControls(boolean show_title_initial)
{
  num_title_screens = 0;

  // 1st step: initialize title screens for game start (only when starting)
  if (show_title_initial)
    InitializeTitleControls_CheckTitleInfo(TRUE);

  // 2nd step: initialize title screens for current level set
  InitializeTitleControls_CheckTitleInfo(FALSE);

  // sort title screens according to sort_priority and title number
  qsort(title_controls, num_title_screens, sizeof(struct TitleControlInfo),
	compareTitleControlInfo);

  // mark first title screen
  title_controls[0].first = TRUE;
}

static boolean visibleMenuPos(struct MenuPosInfo *pos)
{
  return (pos != NULL && pos->x != -1 && pos->y != -1);
}

static boolean visibleTextPos(struct TextPosInfo *pos)
{
  return (pos != NULL && pos->x != -1 && pos->y != -1);
}

static void InitializeMainControls(void)
{
  boolean local_team_mode = (!network.enabled && setup.team_mode);
  int i;

  // set main control text values to dynamically determined values
  sprintf(main_text_name,         "%s",   local_team_mode ? "Team:" : "Name:");

  strcpy(main_text_first_level,  int2str(leveldir_current->first_level,
					 menu.main.text.first_level.size));
  strcpy(main_text_last_level,   int2str(leveldir_current->last_level,
					 menu.main.text.last_level.size));
  strcpy(main_text_level_number, int2str(level_nr,
					 menu.main.text.level_number.size));

  main_text_level_year		= leveldir_current->year;
  main_text_level_imported_from	= leveldir_current->imported_from;
  main_text_level_imported_by	= leveldir_current->imported_by;
  main_text_level_tested_by	= leveldir_current->tested_by;

  main_text_title_1 = getConfigProgramTitleString();
  main_text_title_2 = getConfigProgramCopyrightString();
  main_text_title_3 = getConfigProgramCompanyString();

  // set main control screen positions to dynamically determined values
  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];
    int nr                         = mci->nr;
    struct MenuPosInfo *pos_button = mci->pos_button;
    struct TextPosInfo *pos_text   = mci->pos_text;
    struct TextPosInfo *pos_input  = mci->pos_input;
    char *text                     = (mci->text  ? *mci->text  : NULL);
    char *input                    = (mci->input ? *mci->input : NULL);
    int button_graphic             = mci->button_graphic;
    int font_text                  = (pos_text  ? pos_text->font  : -1);
    int font_input                 = (pos_input ? pos_input->font : -1);

    int font_text_width   = (font_text  != -1 ? getFontWidth(font_text)   : 0);
    int font_text_height  = (font_text  != -1 ? getFontHeight(font_text)  : 0);
    int font_input_width  = (font_input != -1 ? getFontWidth(font_input)  : 0);
    int font_input_height = (font_input != -1 ? getFontHeight(font_input) : 0);
    int text_chars  = (text  != NULL ? strlen(text)  : 0);
    int input_chars = (input != NULL ? strlen(input) : 0);

    int button_width =
      (button_graphic != -1 ? graphic_info[button_graphic].width  : 0);
    int button_height =
      (button_graphic != -1 ? graphic_info[button_graphic].height : 0);
    int text_width   = font_text_width * text_chars;
    int text_height  = font_text_height;
    int input_width  = font_input_width * input_chars;
    int input_height = font_input_height;

    if (nr == MAIN_CONTROL_NAME)
    {
      menu.main.input.name.width  = input_width;
      menu.main.input.name.height = input_height;
    }

    if (pos_button != NULL)		// (x/y may be -1/-1 here)
    {
      pos_button->width  = button_width;
      pos_button->height = button_height;
    }

    if (pos_text != NULL)		// (x/y may be -1/-1 here)
    {
      // calculate text size -- needed for text alignment
      boolean calculate_text_size = (text != NULL);

      if (pos_text->width == -1 || calculate_text_size)
	pos_text->width = text_width;
      if (pos_text->height == -1 || calculate_text_size)
	pos_text->height = text_height;

      if (visibleMenuPos(pos_button))
      {
	if (pos_text->x == -1)
	  pos_text->x = pos_button->x + pos_button->width;
	if (pos_text->y == -1)
	  pos_text->y =
	    pos_button->y + (pos_button->height - pos_text->height) / 2;
      }
    }

    if (pos_input != NULL)		// (x/y may be -1/-1 here)
    {
      if (visibleTextPos(pos_text))
      {
	if (pos_input->x == -1)
	  pos_input->x = pos_text->x + pos_text->width;
	if (pos_input->y == -1)
	  pos_input->y = pos_text->y;
      }

      if (pos_input->width == -1)
	pos_input->width = input_width;
      if (pos_input->height == -1)
	pos_input->height = input_height;
    }
  }
}

static void DrawPressedGraphicThruMask(int dst_x, int dst_y,
				       int graphic, boolean pressed)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  Bitmap *src_bitmap;
  int src_x, src_y;
  int xoffset = (pressed ? g->pressed_xoffset : 0);
  int yoffset = (pressed ? g->pressed_yoffset : 0);

  getFixedGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, drawto, src_x + xoffset, src_y + yoffset,
		   g->width, g->height, dst_x, dst_y);
}

static void DrawCursorAndText_Main_Ext(int nr, boolean active_text,
				       boolean active_input,
				       boolean pressed_button)
{
  int i;

  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];

    if (mci->nr == nr || nr == -1)
    {
      struct MenuPosInfo *pos_button = mci->pos_button;
      struct TextPosInfo *pos_text   = mci->pos_text;
      struct TextPosInfo *pos_input  = mci->pos_input;
      char *text                     = (mci->text  ? *mci->text  : NULL);
      char *input                    = (mci->input ? *mci->input : NULL);
      int button_graphic             = mci->button_graphic;
      int font_text                  = (pos_text  ? pos_text->font  : -1);
      int font_input                 = (pos_input ? pos_input->font : -1);

      if (active_text)
      {
	button_graphic = BUTTON_ACTIVE(button_graphic);
	font_text = FONT_ACTIVE(font_text);
      }

      if (active_input)
      {
	font_input = FONT_ACTIVE(font_input);
      }

      if (visibleMenuPos(pos_button))
      {
	struct MenuPosInfo *pos = pos_button;
	int x = mSX + pos->x;
	int y = mSY + pos->y;

	DrawBackgroundForGraphic(x, y, pos->width, pos->height, button_graphic);
	DrawPressedGraphicThruMask(x, y, button_graphic, pressed_button);
      }

      if (visibleTextPos(pos_text) && text != NULL)
      {
	struct TextPosInfo *pos = pos_text;
	int x = mSX + ALIGNED_TEXT_XPOS(pos);
	int y = mSY + ALIGNED_TEXT_YPOS(pos);

#if 1
	// (check why/if this is needed)
	DrawBackgroundForFont(x, y, pos->width, pos->height, font_text);
#endif
	DrawText(x, y, text, font_text);
      }

      if (visibleTextPos(pos_input) && input != NULL)
      {
	struct TextPosInfo *pos = pos_input;
	int x = mSX + ALIGNED_TEXT_XPOS(pos);
	int y = mSY + ALIGNED_TEXT_YPOS(pos);

#if 1
	// (check why/if this is needed)
	DrawBackgroundForFont(x, y, pos->width, pos->height, font_input);
#endif
	DrawText(x, y, input, font_input);
      }
    }
  }
}

static void DrawCursorAndText_Main(int nr, boolean active_text,
				   boolean pressed_button)
{
  DrawCursorAndText_Main_Ext(nr, active_text, FALSE, pressed_button);
}

#if 0
static void DrawCursorAndText_Main_Input(int nr, boolean active_text,
					 boolean pressed_button)
{
  DrawCursorAndText_Main_Ext(nr, active_text, TRUE, pressed_button);
}
#endif

static struct MainControlInfo *getMainControlInfo(int nr)
{
  int i;

  for (i = 0; main_controls[i].nr != -1; i++)
    if (main_controls[i].nr == nr)
      return &main_controls[i];

  return NULL;
}

static boolean insideMenuPosRect(struct MenuPosInfo *rect, int x, int y)
{
  if (rect == NULL)
    return FALSE;

  int rect_x = ALIGNED_TEXT_XPOS(rect);
  int rect_y = ALIGNED_TEXT_YPOS(rect);

  return (x >= rect_x && x < rect_x + rect->width &&
	  y >= rect_y && y < rect_y + rect->height);
}

static boolean insideTextPosRect(struct TextPosInfo *rect, int x, int y)
{
  if (rect == NULL)
    return FALSE;

  int rect_x = ALIGNED_TEXT_XPOS(rect);
  int rect_y = ALIGNED_TEXT_YPOS(rect);

#if 0
  Debug("screens:insideTextPosRect",
	"(%d, %d), (%d, %d) [%d, %d] (%d, %d) => %d",
	x, y, rect_x, rect_y, rect->x, rect->y, rect->width, rect->height,
	(x >= rect_x && x < rect_x + rect->width &&
	 y >= rect_y && y < rect_y + rect->height));
#endif

  return (x >= rect_x && x < rect_x + rect->width &&
	  y >= rect_y && y < rect_y + rect->height);
}

static boolean insidePreviewRect(struct PreviewInfo *preview, int x, int y)
{
  int rect_width  = preview->xsize * preview->tile_size;
  int rect_height = preview->ysize * preview->tile_size;
  int rect_x = ALIGNED_XPOS(preview->x, rect_width,  preview->align);
  int rect_y = ALIGNED_YPOS(preview->y, rect_height, preview->valign);

  return (x >= rect_x && x < rect_x + rect_width &&
	  y >= rect_y && y < rect_y + rect_height);
}

static void AdjustScrollbar(int id, int items_max, int items_visible,
			    int item_position)
{
  struct GadgetInfo *gi = screen_gadget[id];

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  ModifyGadget(gi, GDI_SCROLLBAR_ITEMS_MAX, items_max,
	       GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
	       GDI_SCROLLBAR_ITEM_POSITION, item_position, GDI_END);
}

static void AdjustChooseTreeScrollbar(TreeInfo *ti, int id)
{
  AdjustScrollbar(id, numTreeInfoInGroup(ti), NUM_MENU_ENTRIES_ON_SCREEN,
		  ti->cl_first);
}

static void AdjustInfoScreenScrollbar(int items_max, int items_visible, int item_position)
{
  AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, items_max, items_visible, item_position);
}

static void AdjustInfoScreenGadgets(int y, int height,
                                    int items_max, int items_visible, int item_position)
{
  struct GadgetInfo *gi_up     = screen_gadget[SCREEN_CTRL_ID_SCROLL_UP];
  struct GadgetInfo *gi_down   = screen_gadget[SCREEN_CTRL_ID_SCROLL_DOWN];
  struct GadgetInfo *gi_scroll = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL];
  int y_up = y;
  int y_down = y + height - SC_SCROLLBUTTON_YSIZE;
  int y_scroll = y + SC_SCROLLBUTTON_YSIZE;
  int height_scroll = height - 2 * SC_SCROLLBUTTON_YSIZE;

  ModifyGadget(gi_up, GDI_Y, y_up, GDI_END);
  ModifyGadget(gi_down, GDI_Y, y_down, GDI_END);
  ModifyGadget(gi_scroll, GDI_Y, y_scroll, GDI_HEIGHT, height_scroll, GDI_END);

  AdjustInfoScreenScrollbar(items_max, items_visible, item_position);
}

static void clearMenuListArea(void)
{
  int scrollbar_xpos = mSX + SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset;

  // correct scrollbar position if placed outside menu (playfield) area
  if (scrollbar_xpos > SX + SC_SCROLLBAR_XPOS)
    scrollbar_xpos = SX + SC_SCROLLBAR_XPOS;

  // clear menu list area, but not title or scrollbar
  DrawBackground(mSX, mSY + MENU_SCREEN_START_YPOS * 32,
                 scrollbar_xpos - mSX, NUM_MENU_ENTRIES_ON_SCREEN * 32);

  // special compatibility handling for "Snake Bite" graphics set
  if (strPrefix(leveldir_current->identifier, "snake_bite"))
    ClearRectangle(drawto, mSX, mSY + MENU_SCREEN_START_YPOS * 32,
		   scrollbar_xpos - mSX, NUM_MENU_ENTRIES_ON_SCREEN * 32);
}

static void drawCursorExt(int xpos, int ypos, boolean active, int graphic)
{
  static int cursor_array[MAX_LEV_FIELDY];
  int x = amSX + TILEX * xpos;
  int y = amSY + TILEY * (MENU_SCREEN_START_YPOS + ypos);

  if (xpos == 0)
  {
    if (graphic != -1)
      cursor_array[ypos] = graphic;
    else
      graphic = cursor_array[ypos];
  }

  if (active)
    graphic = BUTTON_ACTIVE(graphic);

  DrawBackgroundForGraphic(x, y, TILEX, TILEY, graphic);
  DrawFixedGraphicThruMaskExt(drawto, x, y, graphic, 0);
}

static void initCursor(int ypos, int graphic)
{
  drawCursorExt(0, ypos, FALSE, graphic);
}

static void drawCursor(int ypos, boolean active)
{
  drawCursorExt(0, ypos, active, -1);
}

static void drawCursorXY(int xpos, int ypos, int graphic)
{
  drawCursorExt(xpos, ypos, FALSE, graphic);
}

static void drawChooseTreeCursor(int ypos, boolean active)
{
  drawCursorExt(0, ypos, active, -1);
}

static int getChooseTreeEditFont(boolean active)
{
  return (active ? FONT_MENU_2_ACTIVE : FONT_MENU_2);
}

static int getChooseTreeEditXPos(int pos)
{
  boolean has_scrollbar = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->mapped;
  int xoffset = (has_scrollbar ? -1 : 0);
  int xpos = MENU_SCREEN_TEXT2_XPOS + xoffset;
  int sx = amSX + xpos * TILEX;
  int font_nr = getChooseTreeEditFont(FALSE);
  int width = getTextWidth(STR_CHOOSE_TREE_EDIT, font_nr);

  return (pos == POS_RIGHT ? sx + width - 1 : sx);
}

static int getChooseTreeEditYPos(int ypos_raw)
{
  int ypos = MENU_SCREEN_START_YPOS + ypos_raw;
  int sy = amSY + ypos * TILEY;

  return sy;
}

static int getChooseTreeEditXPosReal(int pos)
{
  int xpos = getChooseTreeEditXPos(pos);
  int font_nr = getChooseTreeEditFont(FALSE);
  int font_xoffset = getFontDrawOffsetX(font_nr);

  return xpos + font_xoffset;
}

static void drawChooseTreeEdit(int ypos_raw, boolean active)
{
  int sx = getChooseTreeEditXPos(POS_LEFT);
  int sy = getChooseTreeEditYPos(ypos_raw);
  int font_nr = getChooseTreeEditFont(active);

  DrawText(sx, sy, STR_CHOOSE_TREE_EDIT, font_nr);
}

static char *getInfoScreenSubtitle(int screen_nr, int num_screens, int use_global_screens)
{
  static char info_text_title[MAX_LINE_LEN + 1];
  boolean draw_story_headline = (info_mode == INFO_MODE_STORY && hasLevelStory());

  if (num_screens > 1)
  {
    sprintf(info_text_title, "Page %d of %d", screen_nr + 1, num_screens);
  }
  else if ((info_mode == INFO_MODE_LEVEL) ||
           (info_mode == INFO_MODE_STORY && !hasLevelStory()))
  {
    snprintf(info_text_title, MAX_LINE_LEN, "for level %d", level_nr);
  }
  else
  {
    char *text_format = (draw_story_headline ? "%s" :
                         use_global_screens ? "for %s" : "for \"%s\"");
    int text_format_len = strlen(text_format) - strlen("%s");
    int text_font = (draw_story_headline ? FONT_TITLE_STORY : FONT_TITLE_2);
    int max_text_width = SXSIZE - MENU_SCREEN_INFO_SPACE_LEFT - MENU_SCREEN_INFO_SPACE_RIGHT;
    int max_text_len = max_text_width / getFontWidth(text_font);
    int max_name_len = max_text_len - text_format_len;
    char name_cut[max_name_len];
    char *name_full = (draw_story_headline ? level.name_native :
                       use_global_screens ? getProgramTitleString() :
		       leveldir_current->name);

    snprintf(name_cut, max_name_len, "%s", name_full);
    snprintf(info_text_title, max_text_len, text_format, name_cut);
  }

  return info_text_title;
}

static void DrawInfoScreen_Headline(int screen_nr, int num_screens, int use_global_screens)
{
  char *info_text_title_1 = getInfoScreenTitle_Generic();
  char *info_text_title_2 = getInfoScreenSubtitle(screen_nr, num_screens, use_global_screens);
  boolean draw_story_headline = (info_mode == INFO_MODE_STORY && hasLevelStory());

  if (draw_story_headline)
  {
    DrawTextSCentered(MENU_TITLE_STORY_YPOS, FONT_TITLE_STORY, info_text_title_2);
  }
  else
  {
    DrawTextSCentered(MENU_TITLE1_YPOS, FONT_TITLE_1, info_text_title_1);
    DrawTextSCentered(MENU_TITLE2_YPOS, FONT_TITLE_2, info_text_title_2);
  }
}

static void DrawTitleScreenImage(int nr, boolean initial)
{
  static int frame_counter = 0;
  int graphic = getTitleScreenGraphic(nr, initial);
  Bitmap *bitmap = graphic_info[graphic].bitmap;
  Bitmap *bitmap_background = NULL;
  int draw_masked = graphic_info[graphic].draw_masked;
  int width  = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  int src_x = graphic_info[graphic].src_x;
  int src_y = graphic_info[graphic].src_y;
  int dst_x, dst_y;

  // check for optional title screen of native BD style level set
  if (CheckTitleScreen_BD(nr, initial))
  {
    Bitmap **title_screen_bitmaps = GetTitleScreenBitmaps_BD();

    bitmap            = title_screen_bitmaps[0];
    bitmap_background = title_screen_bitmaps[1];

    if (bitmap != NULL)
    {
      width  = bitmap->width;
      height = bitmap->height;
      src_x = 0;
      src_y = 0;
    }
  }

  if (bitmap == NULL)
    return;

  if (width > WIN_XSIZE)
  {
    // image width too large for window => center image horizontally
    src_x = (width - WIN_XSIZE) / 2;
    width = WIN_XSIZE;
  }

  if (height > WIN_YSIZE)
  {
    // image height too large for window => center image vertically
    src_y = (height - WIN_YSIZE) / 2;
    height = WIN_YSIZE;
  }

  // always display title screens centered
  dst_x = (WIN_XSIZE - width) / 2;
  dst_y = (WIN_YSIZE - height) / 2;

  SetDrawBackgroundMask(REDRAW_ALL);
  SetWindowBackgroundImage(getTitleBackground(nr, initial, TRUE));

  ClearRectangleOnBackground(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  boolean draw_masked_final = (DrawingOnBackground(dst_x, dst_y) && draw_masked);

  if (bitmap_background != NULL)
  {
    int size = bitmap_background->height - bitmap->height;
    int offset = frame_counter++ % size;

    BlitBitmap(bitmap_background, drawto, src_x, src_y + offset, width, height, dst_x, dst_y);

    draw_masked_final = TRUE;
  }

  if (draw_masked_final)
    BlitBitmapMasked(bitmap, drawto, src_x, src_y, width, height, dst_x, dst_y);
  else
    BlitBitmap(bitmap, drawto, src_x, src_y, width, height, dst_x, dst_y);

  redraw_mask = REDRAW_ALL;
}

static void DrawTitleScreenMessage(int nr, boolean initial)
{
  char *filename = getLevelSetTitleMessageFilename(nr, initial);
  struct TitleMessageInfo *tmi = getTitleMessageInfo(nr, initial);

  if (filename == NULL)
    return;

  // force TITLE font on title message screen
  SetFontStatus(getTitleMessageGameMode(initial));

  // if chars *and* width set to "-1", automatically determine width
  if (tmi->chars == -1 && tmi->width == -1)
    tmi->width = viewport.window[game_status].width;

  // if lines *and* height set to "-1", automatically determine height
  if (tmi->lines == -1 && tmi->height == -1)
    tmi->height = viewport.window[game_status].height;

  // if chars set to "-1", automatically determine by text and font width
  if (tmi->chars == -1)
    tmi->chars = tmi->width / getFontWidth(tmi->font);
  else
    tmi->width = tmi->chars * getFontWidth(tmi->font);

  // if lines set to "-1", automatically determine by text and font height
  if (tmi->lines == -1)
    tmi->lines = tmi->height / getFontHeight(tmi->font);
  else
    tmi->height = tmi->lines * getFontHeight(tmi->font);

  // if x set to "-1", automatically determine by width and alignment
  if (tmi->x == -1)
    tmi->x = -1 * ALIGNED_XPOS(0, tmi->width, tmi->align);

  // if y set to "-1", automatically determine by height and alignment
  if (tmi->y == -1)
    tmi->y = -1 * ALIGNED_YPOS(0, tmi->height, tmi->valign);

  SetDrawBackgroundMask(REDRAW_ALL);
  SetWindowBackgroundImage(getTitleBackground(nr, initial, FALSE));

  ClearRectangleOnBackground(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  DrawTextFile(ALIGNED_TEXT_XPOS(tmi), ALIGNED_TEXT_YPOS(tmi),
	       filename, tmi->font, tmi->chars, -1, tmi->lines, -1, -1, -1, 0, -1,
	       tmi->autowrap, tmi->centered, tmi->parse_comments);

  ResetFontStatus();
}

static void DrawTitleScreen(void)
{
  KeyboardAutoRepeatOff();

  HandleTitleScreen(0, 0, 0, 0, MB_MENU_INITIALIZE);
}

static boolean CheckTitleScreen(boolean levelset_has_changed)
{
  static boolean show_title_initial = TRUE;
  boolean show_titlescreen = FALSE;

  // needed to be able to skip title screen, if no image or message defined
  InitializeTitleControls(show_title_initial);

  if (setup.show_titlescreen && (show_title_initial || levelset_has_changed))
    show_titlescreen = TRUE;

  // show initial title images and messages only once at program start
  show_title_initial = FALSE;

  return (show_titlescreen && num_title_screens > 0);
}

void DrawMainMenu(void)
{
  static LevelDirTree *leveldir_last_valid = NULL;
  boolean levelset_has_changed = FALSE;
  int fade_mask = REDRAW_FIELD;

  LimitScreenUpdates(FALSE);

  FadeSetLeaveScreen();

  // do not fade out here -- function may continue and fade on editor screen

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  ExpireSoundLoops(FALSE);

  KeyboardAutoRepeatOn();

  audio.sound_deactivated = FALSE;

  GetPlayerConfig();

  // needed if last screen was the playing screen, invoked from level editor
  if (level_editor_test_game)
  {
    CloseDoor(DOOR_CLOSE_ALL);

    SetGameStatus(GAME_MODE_EDITOR);

    DrawLevelEd();

    return;
  }

  // needed if last screen was the playing screen, invoked from hall of fame
  if (score_info_tape_play)
  {
    CloseDoor(DOOR_CLOSE_ALL);

    SetGameStatus(GAME_MODE_SCOREINFO);

    DrawScoreInfo(scores.last_entry_nr);

    return;
  }

  // close game panel door in main menu, but keep tape recorder door open
  CloseDoor(DOOR_CLOSE_1);

  // reset flag to continue playing next level from hall of fame
  scores.continue_playing = FALSE;

  // leveldir_current may be invalid (level group, parent link, node copy)
  leveldir_current = getValidLevelSeries(leveldir_current, leveldir_last_valid);

  if (leveldir_current != leveldir_last_valid)
  {
    // level setup config may have been loaded to "last played" tree node copy,
    // but "leveldir_current" now points to the "original" level set tree node,
    // in which case "handicap_level" may still default to the first level
    LoadLevelSetup_SeriesInfo();

    UpdateLastPlayedLevels_TreeInfo();

    levelset_has_changed = TRUE;
  }

  // store valid level series information
  leveldir_last_valid = leveldir_current;

  // store first level of this level set for "level_nr" style animations
  SetAnimationFirstLevel(leveldir_current->first_level);

  // level_nr may have been set to value over handicap with level editor
  if (setup.allow_skipping_levels != STATE_TRUE && level_nr > leveldir_current->handicap_level)
    level_nr = leveldir_current->handicap_level;

  // needed if last screen (level choice) changed graphics, sounds or music
  ReloadCustomArtwork(0);

  // level may use image color template, so reload custom artwork before loading level
  LoadLevel(level_nr);
  LoadScore(level_nr);

  SaveLevelSetup_SeriesInfo();

  if (CheckTitleScreen(levelset_has_changed))
  {
    SetGameStatus(GAME_MODE_TITLE);

    DrawTitleScreen();

    return;
  }

  if (redraw_mask & REDRAW_ALL)
    fade_mask = REDRAW_ALL;

  if (CheckFadeAll())
    fade_mask = REDRAW_ALL;

  FadeOut(fade_mask);

  // needed if different viewport properties defined for menues
  ChangeViewportPropertiesIfNeeded();

  SetDrawtoField(DRAW_TO_BACKBUFFER);

  // set this after "ChangeViewportPropertiesIfNeeded()" (which may reset it)
  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  SetMainBackgroundImage(IMG_BACKGROUND_MAIN);

#if 0
  if (fade_mask == REDRAW_ALL)
    RedrawGlobalBorder();
#endif

  ClearField();

  InitializeMainControls();

  DrawCursorAndText_Main(-1, FALSE, FALSE);
  DrawPreviewLevelInitial();
  DrawNetworkPlayers();

  HandleMainMenu(0, 0, 0, 0, MB_MENU_INITIALIZE);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadTape(level_nr);
  DrawCompleteVideoDisplay();

  PlayMenuSoundsAndMusic();

  // create gadgets for main menu screen
  FreeScreenGadgets();
  CreateScreenGadgets();

  // may be required if audio buttons shown on tape and changed in setup menu
  FreeGameButtons();
  CreateGameButtons();

  // map gadgets for main menu screen
  MapTapeButtons();
  MapScreenMenuGadgets(SCREEN_MASK_MAIN);
  UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_SOLUTION, hasSolutionTape());
  UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_LEVELSET_INFO, hasLevelSetInfo(FALSE));
  UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_LEVEL_INFO, hasLevelInfo(FALSE));

  // copy actual game door content to door double buffer for OpenDoor()
  BlitBitmap(drawto, bitmap_db_door_1, DX, DY, DXSIZE, DYSIZE, 0, 0);
  BlitBitmap(drawto, bitmap_db_door_2, VX, VY, VXSIZE, VYSIZE, 0, 0);

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
  FadeSetEnterMenu();

  // update screen area with special editor door
  redraw_mask |= REDRAW_ALL;
  BackToFront();

  SetMouseCursor(CURSOR_DEFAULT);

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);

  SyncEmscriptenFilesystem();

  // needed once after program start or after user change
  CheckApiServerTasks();
}

static void gotoTopLevelDir(void)
{
  // move upwards until inside (but not above) top level directory
  while (leveldir_current->node_parent &&
	 !strEqual(leveldir_current->node_parent->subdir, STRING_TOP_DIRECTORY))
  {
    // write a "path" into level tree for easy navigation to last level
    if (leveldir_current->node_parent->node_group->cl_first == -1)
    {
      int num_leveldirs = numTreeInfoInGroup(leveldir_current);
      int leveldir_pos = getPosFromTreeInfo(leveldir_current);
      int num_page_entries = MIN(num_leveldirs, NUM_MENU_ENTRIES_ON_SCREEN);
      int cl_first, cl_cursor;

      cl_first = MAX(0, leveldir_pos - num_page_entries + 1);
      cl_cursor = leveldir_pos - cl_first;

      leveldir_current->node_parent->node_group->cl_first = cl_first;
      leveldir_current->node_parent->node_group->cl_cursor = cl_cursor;
    }

    leveldir_current = leveldir_current->node_parent;
  }
}

static unsigned int getAutoDelayCounter(struct TitleFadingInfo *fi)
{
  boolean use_frame_counter = (fi->auto_delay_unit == AUTO_DELAY_UNIT_FRAMES);

  return (use_frame_counter ? video.frame_counter : Counter());
}

static boolean TitleAutoDelayReached(unsigned int *counter_var,
				     struct TitleFadingInfo *fi)
{
  return DelayReachedExt2(counter_var, fi->auto_delay, getAutoDelayCounter(fi));
}

static void ResetTitleAutoDelay(unsigned int *counter_var,
				struct TitleFadingInfo *fi)
{
  *counter_var = getAutoDelayCounter(fi);
}

void HandleTitleScreen(int mx, int my, int dx, int dy, int button)
{
  static unsigned int title_delay = 0;
  static int title_screen_nr = 0;
  static int last_sound = -1, last_music = -1;
  boolean return_to_main_menu = FALSE;
  struct TitleControlInfo *tci;
  int sound, music;

  if (button == MB_MENU_INITIALIZE)
  {
    title_delay = 0;
    title_screen_nr = 0;
    tci = &title_controls[title_screen_nr];

    SetAnimStatus(getTitleAnimMode(tci));

    last_sound = SND_UNDEFINED;
    last_music = MUS_UNDEFINED;

    if (num_title_screens != 0)
    {
      FadeSetEnterScreen();

      // use individual title fading instead of global "enter screen" fading
      fading = getTitleFading(tci);
    }

    if (game_status_last_screen == GAME_MODE_INFO)
    {
      if (num_title_screens == 0)
      {
	// switch game mode from title screen mode back to info screen mode
	SetGameStatus(GAME_MODE_INFO);

	// store that last screen was info screen, not main menu screen
	game_status_last_screen = GAME_MODE_INFO;

	DrawInfoScreen_NotAvailable("Title screen information:",
				    "No title screen for this level set.");
	return;
      }
    }

    FadeMenuSoundsAndMusic();

    FadeOut(REDRAW_ALL);

    // title screens may have different window size
    ChangeViewportPropertiesIfNeeded();

    // only required to update logic for redrawing global border
    ClearField();

    if (tci->is_image)
      DrawTitleScreenImage(tci->local_nr, tci->initial);
    else
      DrawTitleScreenMessage(tci->local_nr, tci->initial);

    sound = getTitleSound(tci);
    music = getTitleMusic(tci);

    if (sound != last_sound)
      PlayMenuSoundExt(sound);
    if (music != last_music)
      PlayMenuMusicExt(music);

    last_sound = sound;
    last_music = music;

    SetMouseCursor(CURSOR_NONE);

    FadeIn(REDRAW_ALL);

    ResetTitleAutoDelay(&title_delay, &fading);

    return;
  }

  if (fading.auto_delay > 0 && TitleAutoDelayReached(&title_delay, &fading))
    button = MB_MENU_CHOICE;

  if (button == MB_MENU_LEAVE)
  {
    return_to_main_menu = TRUE;
  }
  else if (button == MB_MENU_CHOICE || dx)
  {
    if (game_status_last_screen == GAME_MODE_INFO && num_title_screens == 0)
    {
      SetGameStatus(GAME_MODE_INFO);

      info_mode = INFO_MODE_MAIN;

      DrawInfoScreen();

      return;
    }

    title_screen_nr +=
      (game_status_last_screen == GAME_MODE_INFO && dx < 0 ? -1 : +1);

    if (title_screen_nr >= 0 && title_screen_nr < num_title_screens)
    {
      tci = &title_controls[title_screen_nr];

      SetAnimStatus(getTitleAnimMode(tci));

      sound = getTitleSound(tci);
      music = getTitleMusic(tci);

      if (last_sound != SND_UNDEFINED && sound != last_sound)
	FadeSound(last_sound);
      if (last_music != MUS_UNDEFINED && music != last_music)
	FadeMusic();

      fading = getTitleFading(tci);

      FadeOut(REDRAW_ALL);

      if (tci->is_image)
	DrawTitleScreenImage(tci->local_nr, tci->initial);
      else
	DrawTitleScreenMessage(tci->local_nr, tci->initial);

      sound = getTitleSound(tci);
      music = getTitleMusic(tci);

      if (sound != last_sound)
	PlayMenuSoundExt(sound);
      if (music != last_music)
	PlayMenuMusicExt(music);

      last_sound = sound;
      last_music = music;

      FadeIn(REDRAW_ALL);

      ResetTitleAutoDelay(&title_delay, &fading);
    }
    else
    {
      FadeMenuSoundsAndMusic();

      return_to_main_menu = TRUE;
    }
  }
  else
  {
    tci = &title_controls[title_screen_nr];

    // check for optional title screen of native BD style level set
    if (tci->is_image && CheckTitleScreen_BD(tci->local_nr, tci->initial))
    {
      Bitmap **title_screen_bitmaps = GetTitleScreenBitmaps_BD();

      // if title screen is animated, draw title screen animation
      if (title_screen_bitmaps[0] != NULL &&
	  title_screen_bitmaps[1] != NULL)
	DrawTitleScreenImage(tci->local_nr, tci->initial);
    }
  }

  if (return_to_main_menu)
  {
    SetMouseCursor(CURSOR_DEFAULT);

    // force full menu screen redraw after displaying title screens
    redraw_mask = REDRAW_ALL;

    if (game_status_last_screen == GAME_MODE_INFO)
    {
      SetGameStatus(GAME_MODE_INFO);

      info_mode = INFO_MODE_MAIN;

      DrawInfoScreen();
    }
    else	// default: return to main menu
    {
      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }
  }
}

static void HandleMainMenu_ToggleTeamMode(void)
{
  setup.team_mode = !setup.team_mode;

  InitializeMainControls();
  DrawCursorAndText_Main(MAIN_CONTROL_NAME, TRUE, FALSE);

  DrawPreviewPlayers();
}

static void HandleMainMenu_SelectLevel(int step, int direction,
				       int selected_level_nr)
{
  int old_level_nr = level_nr;
  int new_level_nr;

  if (selected_level_nr != NO_DIRECT_LEVEL_SELECT)
    new_level_nr = selected_level_nr;
  else
    new_level_nr = old_level_nr + step * direction;

  if (new_level_nr < leveldir_current->first_level)
    new_level_nr = leveldir_current->first_level;
  if (new_level_nr > leveldir_current->last_level)
    new_level_nr = leveldir_current->last_level;

  if (setup.allow_skipping_levels != STATE_TRUE && new_level_nr > leveldir_current->handicap_level)
  {
    // skipping levels is only allowed when trying to skip single level
    // (also, skipping BD style intermission levels is always possible)
    if (new_level_nr == old_level_nr + 1 &&
	(level.bd_intermission ||
	 (setup.allow_skipping_levels == STATE_ASK &&
	  Request("Level still unsolved! Skip it anyway?", REQ_ASK))))
    {
      leveldir_current->handicap_level++;
      SaveLevelSetup_SeriesInfo();
    }

    new_level_nr = leveldir_current->handicap_level;
  }

  if (new_level_nr != old_level_nr)
  {
    struct MainControlInfo *mci = getMainControlInfo(MAIN_CONTROL_LEVEL_NUMBER);

    PlaySound(SND_MENU_ITEM_SELECTING);

    level_nr = new_level_nr;

    DrawText(mSX + mci->pos_text->x, mSY + mci->pos_text->y,
	     int2str(level_nr, menu.main.text.level_number.size),
	     mci->pos_text->font);

    LoadLevel(level_nr);
    DrawPreviewLevelInitial();

    TapeErase();
    LoadTape(level_nr);
    DrawCompleteVideoDisplay();

    SaveLevelSetup_SeriesInfo();

    // when using icon graphics with color template, icons must be redrawn for each level
    UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_SOLUTION, FALSE);
    UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_SOLUTION, hasSolutionTape());
    UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_LEVELSET_INFO, FALSE);
    UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_LEVELSET_INFO, hasLevelSetInfo(FALSE));
    UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_LEVEL_INFO, FALSE);
    UpdateScreenMenuGadgets(SCREEN_MASK_MAIN_HAS_LEVEL_INFO, hasLevelInfo(FALSE));

    // force redraw of playfield area (may be reset at this point)
    redraw_mask |= REDRAW_FIELD;
  }
}

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = MAIN_CONTROL_GAME;
  static boolean button_pressed_last = FALSE;
  boolean button_pressed = FALSE;
  int pos = choice;
  int i = 0;	// needed to prevent compiler warning due to bad code below

  if (button == MB_MENU_INITIALIZE)
  {
    DrawCursorAndText_Main(choice, TRUE, FALSE);

    return;
  }

  if (mx || my)		// mouse input
  {
    pos = -1;

    for (i = 0; main_controls[i].nr != -1; i++)
    {
      if (insideMenuPosRect(main_controls[i].pos_button, mx - mSX, my - mSY) ||
	  insideTextPosRect(main_controls[i].pos_text,   mx - mSX, my - mSY) ||
	  insideTextPosRect(main_controls[i].pos_input,  mx - mSX, my - mSY))
      {
	pos = main_controls[i].nr;

	break;
      }
    }

    // check if level preview was clicked
    if (insidePreviewRect(&preview, mx - SX, my - SY))
      pos = MAIN_CONTROL_GAME;

    // handle pressed/unpressed state for active/inactive menu buttons
    // (if pos != -1, "i" contains index position corresponding to "pos")
    if (button &&
	pos >= MAIN_CONTROL_NAME && pos <= MAIN_CONTROL_QUIT &&
	insideMenuPosRect(main_controls[i].pos_button, mx - mSX, my - mSY))
      button_pressed = TRUE;

    if (button_pressed != button_pressed_last)
    {
      DrawCursorAndText_Main(choice, TRUE, button_pressed);

      if (button_pressed)
	PlaySound(SND_MENU_BUTTON_PRESSING);
      else
	PlaySound(SND_MENU_BUTTON_RELEASING);
    }
  }
  else if (dx || dy)	// keyboard input
  {
    if (dx > 0 && (choice == MAIN_CONTROL_INFO ||
		   choice == MAIN_CONTROL_SETUP))
      button = MB_MENU_CHOICE;
    else if (dy)
      pos = choice + dy;
  }

  if (pos == MAIN_CONTROL_FIRST_LEVEL && !button)
  {
    HandleMainMenu_SelectLevel(MAX_LEVELS, -1, NO_DIRECT_LEVEL_SELECT);
  }
  else if (pos == MAIN_CONTROL_LAST_LEVEL && !button)
  {
    HandleMainMenu_SelectLevel(MAX_LEVELS, +1, NO_DIRECT_LEVEL_SELECT);
  }
  else if (pos == MAIN_CONTROL_LEVEL_NUMBER && !button)
  {
    CloseDoor(DOOR_CLOSE_2);

    SetGameStatus(GAME_MODE_LEVELNR);

    DrawChooseLevelNr();
  }
  else if (pos >= MAIN_CONTROL_NAME && pos <= MAIN_CONTROL_QUIT)
  {
    if (button)
    {
      if (pos != choice)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	DrawCursorAndText_Main(choice, FALSE, FALSE);
	DrawCursorAndText_Main(pos, TRUE, button_pressed);

	choice = pos;
      }
      else if (dx != 0)
      {
	if (choice == MAIN_CONTROL_NAME)
	{
	  // special case: cursor left or right pressed -- toggle team mode
	  HandleMainMenu_ToggleTeamMode();
	}
	else if (choice != MAIN_CONTROL_INFO &&
		 choice != MAIN_CONTROL_SETUP)
	{
	  HandleMainMenu_SelectLevel(1, dx, NO_DIRECT_LEVEL_SELECT);
	}
      }
    }
    else
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (pos == MAIN_CONTROL_NAME)
      {
	if ((mx || my) &&
	    insideTextPosRect(main_controls[i].pos_text, mx - mSX, my - mSY))
	{
	  // special case: menu text "name/team" clicked -- toggle team mode
	  HandleMainMenu_ToggleTeamMode();
	}
	else
	{
	  if (setup.multiple_users)
	  {
	    CloseDoor(DOOR_CLOSE_2);

	    SetGameStatus(GAME_MODE_NAMES);

	    DrawChoosePlayerName();
	  }
	  else
	  {
	    SetGameStatus(GAME_MODE_PSEUDO_TYPENAME);

	    DrawTypeName();
	  }
	}
      }
      else if (pos == MAIN_CONTROL_LEVELS)
      {
	if (leveldir_first)
	{
	  CloseDoor(DOOR_CLOSE_2);

	  SetGameStatus(GAME_MODE_LEVELS);

	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();

	  // restore level set if chosen from "last played level set" menu
	  RestoreLastPlayedLevels(&leveldir_current);

	  if (setup.internal.choose_from_top_leveldir)
	    gotoTopLevelDir();

	  DrawChooseLevelSet();
	}
      }
      else if (pos == MAIN_CONTROL_SCORES)
      {
	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_SCORES);

	DrawHallOfFame(level_nr);
      }
      else if (pos == MAIN_CONTROL_EDITOR)
      {
	if (leveldir_current->readonly &&
	    setup.editor.show_read_only_warning)
	  Request("This level is read-only!", REQ_CONFIRM | REQ_STAY_OPEN);

	CloseDoor(DOOR_CLOSE_ALL);

	SetGameStatus(GAME_MODE_EDITOR);

	FadeSetEnterScreen();

	DrawLevelEd();
      }
      else if (pos == MAIN_CONTROL_INFO)
      {
	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_INFO);

	info_mode = INFO_MODE_MAIN;

	DrawInfoScreen();
      }
      else if (pos == MAIN_CONTROL_GAME)
      {
	StartGameActions(network.enabled, setup.autorecord, level.random_seed);
      }
      else if (pos == MAIN_CONTROL_SETUP)
      {
	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_SETUP);

	setup_mode = SETUP_MODE_MAIN;

	DrawSetupScreen();
      }
      else if (pos == MAIN_CONTROL_QUIT)
      {
	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();

#if defined(PLATFORM_EMSCRIPTEN)
	Request("Close the browser window to quit!", REQ_CONFIRM);
#else
	if (!setup.ask_on_quit_program ||
	    Request("Do you really want to quit?", REQ_ASK | REQ_STAY_CLOSED))
	  SetGameStatus(GAME_MODE_QUIT);
#endif
      }
    }
  }

  button_pressed_last = button_pressed;
}


// ============================================================================
// info screen functions
// ============================================================================

static struct TokenInfo *info_info;
static int num_info_info;	// number of info entries shown on screen
static int max_info_info;	// total number of info entries in list

static void execInfoTitleScreen(void)
{
  info_mode = INFO_MODE_TITLE;

  DrawInfoScreen();
}

static void execInfoElements(void)
{
  info_mode = INFO_MODE_ELEMENTS;

  DrawInfoScreen();
}

static void execInfoMusic(void)
{
  info_mode = INFO_MODE_MUSIC;

  DrawInfoScreen();
}

static void execInfoCredits(void)
{
  info_mode = INFO_MODE_CREDITS;

  DrawInfoScreen();
}

static void execInfoProgram(void)
{
  info_mode = INFO_MODE_PROGRAM;

  DrawInfoScreen();
}

static void execInfoVersion(void)
{
  info_mode = INFO_MODE_VERSION;

  DrawInfoScreen();
}

static void execInfoLevelSet(void)
{
  info_mode = INFO_MODE_LEVELSET;

  DrawInfoScreen();
}

static void execInfoLevel(void)
{
  info_mode = INFO_MODE_LEVEL;

  DrawInfoScreen();
}

static void execInfoStory(void)
{
  info_mode = INFO_MODE_STORY;

  DrawInfoScreen();
}

static void execExitInfo(void)
{
  SetGameStatus(GAME_MODE_MAIN);

  DrawMainMenu();
}

static struct TokenInfo info_info_main[] =
{
  { TYPE_ENTER_SCREEN,	execInfoTitleScreen,	STR_INFO_TITLE		},
  { TYPE_ENTER_SCREEN,	execInfoElements,	STR_INFO_ELEMENTS	},
  { TYPE_ENTER_SCREEN,	execInfoMusic,		STR_INFO_MUSIC		},
  { TYPE_ENTER_SCREEN,	execInfoCredits,	STR_INFO_CREDITS	},
  { TYPE_ENTER_SCREEN,	execInfoProgram,	STR_INFO_PROGRAM	},
  { TYPE_ENTER_SCREEN,	execInfoVersion,	STR_INFO_VERSION	},
  { TYPE_ENTER_SCREEN,	execInfoLevelSet,	STR_INFO_LEVELSET	},
  { TYPE_ENTER_SCREEN,	execInfoLevel,		STR_INFO_LEVEL		},
  { TYPE_ENTER_SCREEN,	execInfoStory,		STR_INFO_STORY		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitInfo, 		STR_INFO_EXIT		},

  { 0,			NULL,			NULL			}
};

static int getMenuTextFont(int type)
{
  if (type & (TYPE_SWITCH	|
	      TYPE_YES_NO	|
	      TYPE_YES_NO_AUTO	|
	      TYPE_YES_NO_ASK	|
	      TYPE_YES_NO_ONCE	|
	      TYPE_STRING	|
	      TYPE_PLAYER	|
	      TYPE_ECS_AGA	|
	      TYPE_KEYTEXT	|
	      TYPE_ENTER_LIST	|
	      TYPE_TEXT_INPUT))
    return FONT_MENU_2;
  else
    return FONT_MENU_1;
}

static struct TokenInfo *setup_info;
static struct TokenInfo setup_info_input[];

static struct TokenInfo *menu_info;

static void PlayInfoSound(void)
{
  int info_sound = getInfoScreenBackgroundSound_Generic();
  char *next_sound = getSoundInfoEntryFilename(info_sound);

  if (next_sound != NULL)
    PlayMenuSoundExt(info_sound);
  else
    PlayMenuSound();
}

static void PlayInfoSoundIfLoop(void)
{
  int info_sound = getInfoScreenBackgroundSound_Generic();
  char *next_sound = getSoundInfoEntryFilename(info_sound);

  if (next_sound != NULL)
    PlayMenuSoundIfLoopExt(info_sound);
  else
    PlayMenuSoundIfLoop();
}

static void PlayInfoMusic(void)
{
  int info_music = getInfoScreenBackgroundMusic_Generic();
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(info_music);

  if (next_music != NULL)
  {
    // play music if info screen music differs from current music
    if (!strEqual(curr_music, next_music))
      PlayMenuMusicExt(info_music);
  }
  else
  {
    // only needed if info screen was directly invoked from main menu
    PlayMenuMusic();
  }
}

static void PlayInfoSoundsAndMusic(void)
{
  PlayInfoSound();
  PlayInfoMusic();
}

static void FadeInfoSounds(void)
{
  FadeSounds();
}

static void FadeInfoMusic(void)
{
  int info_music = getInfoScreenBackgroundMusic_Generic();
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(info_music);

  if (next_music != NULL)
  {
    // fade music if info screen music differs from current music
    if (!strEqual(curr_music, next_music))
      FadeMusic();
  }
}

static void FadeInfoSoundsAndMusic(void)
{
  FadeInfoSounds();
  FadeInfoMusic();
}

static void DrawCursorAndText_Menu_Ext(struct TokenInfo *token_info,
				       int screen_pos, int menu_info_pos_raw,
				       boolean active)
{
  int pos = (menu_info_pos_raw < 0 ? screen_pos : menu_info_pos_raw);
  struct TokenInfo *ti = &token_info[pos];
  int xpos = MENU_SCREEN_START_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + screen_pos;
  int font_nr = getMenuTextFont(ti->type);

  if (setup_mode == SETUP_MODE_INPUT)
    font_nr = FONT_MENU_1;

  if (active)
    font_nr = FONT_ACTIVE(font_nr);

  DrawText(mSX + xpos * 32, mSY + ypos * 32, ti->text, font_nr);

  if (ti->type & ~TYPE_SKIP_ENTRY)
    drawCursor(screen_pos, active);
}

static void DrawCursorAndText_Menu(int screen_pos, int menu_info_pos_raw,
				   boolean active)
{
  DrawCursorAndText_Menu_Ext(menu_info, screen_pos, menu_info_pos_raw, active);
}

static void DrawCursorAndText_Setup(int screen_pos, int menu_info_pos_raw,
				    boolean active)
{
  DrawCursorAndText_Menu_Ext(setup_info, screen_pos, menu_info_pos_raw, active);
}

static char *window_size_text;
static char *scaling_type_text;
static char *network_server_text;

static void drawSetupValue(int, int);

static void drawMenuInfoList(int first_entry, int num_page_entries,
			     int max_page_entries)
{
  int i;

  if (first_entry + num_page_entries > max_page_entries)
    first_entry = 0;

  clearMenuListArea();

  for (i = 0; i < num_page_entries; i++)
  {
    int menu_info_pos = first_entry + i;
    struct TokenInfo *si = &menu_info[menu_info_pos];
    void *value_ptr = si->value;

    // set some entries to "unchangeable" according to other variables
    if ((value_ptr == &setup.sound_simple && !audio.sound_available) ||
	(value_ptr == &setup.sound_loops  && !audio.loops_available) ||
	(value_ptr == &setup.sound_music  && !audio.music_available) ||
	(value_ptr == &setup.fullscreen   && !video.fullscreen_available) ||
	(value_ptr == &window_size_text   && !video.window_scaling_available) ||
	(value_ptr == &scaling_type_text  && !video.window_scaling_available))
      si->type |= TYPE_GHOSTED;

    if (si->type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (si->type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (si->type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

    DrawCursorAndText_Menu(i, menu_info_pos, FALSE);

    if (si->type & TYPE_STRING)
    {
      int gadget_id = -1;

      if (value_ptr == &network_server_text)
	gadget_id = SCREEN_CTRL_ID_NETWORK_SERVER;

      if (gadget_id != -1)
      {
	struct GadgetInfo *gi = screen_gadget[gadget_id];
	int xpos = MENU_SCREEN_START_XPOS;
	int ypos = MENU_SCREEN_START_YPOS + i;
	int x = mSX + xpos * 32;
	int y = mSY + ypos * 32;

	ModifyGadget(gi, GDI_X, x, GDI_Y, y, GDI_END);
      }
    }

    if (si->type & TYPE_VALUE &&
	menu_info == setup_info)
      drawSetupValue(i, menu_info_pos);
  }
}

static void DrawInfoScreen_Main(void)
{
  int fade_mask = REDRAW_FIELD;
  int i;

  // (needed after displaying info sub-screens directly from main menu)
  if (info_screens_from_main)
  {
    info_screens_from_main = FALSE;

    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();

    return;
  }

  // (needed after displaying info sub-screens directly from init game)
  if (info_screens_from_game)
  {
    info_screens_from_game = FALSE;

    InitGame();

    return;
  }

  if (redraw_mask & REDRAW_ALL)
    fade_mask = REDRAW_ALL;

  if (CheckFadeAll())
    fade_mask = REDRAW_ALL;

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  FreeScreenGadgets();
  CreateScreenGadgets();

  // (needed after displaying title screens which disable auto repeat)
  KeyboardAutoRepeatOn();

  FadeSetLeaveScreen();

  FadeOut(fade_mask);

  // needed if different viewport properties defined for info screen
  ChangeViewportPropertiesIfNeeded();

  SetMainBackgroundImage(IMG_BACKGROUND_INFO);

  ClearField();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  DrawTextSCentered(MENU_TITLE_YPOS, FONT_TITLE_1, STR_INFO_MAIN);

  info_info = info_info_main;

  // use modified info screen info without info screen entries marked as hidden
  info_info = getSetupInfoFinal(info_info);

  // determine maximal number of info entries that can be displayed on screen
  num_info_info = 0;
  for (i = 0; info_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
    num_info_info++;

  // determine maximal number of info entries available for menu of info screen
  max_info_info = 0;
  for (i = 0; info_info[i].type != 0; i++)
    max_info_info++;

  HandleInfoScreen_Main(0, 0, 0, 0, MB_MENU_INITIALIZE);

  MapScreenGadgets(max_info_info);

  PlayMenuSoundsAndMusic();

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
}

static void changeSetupValue(int, int, int);

static void HandleMenuScreen(int mx, int my, int dx, int dy, int button,
			     int mode, int num_page_entries,
			     int max_page_entries)
{
  static int num_page_entries_all_last[NUM_SPECIAL_GFX_ARGS][MAX_MENU_MODES];
  static int choice_stores[NUM_SPECIAL_GFX_ARGS][MAX_MENU_MODES];
  static int first_entry_stores[NUM_SPECIAL_GFX_ARGS][MAX_MENU_MODES];
  boolean has_scrollbar = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->mapped;
  int mx_scrollbar = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->x;
  int mx_right_border = (has_scrollbar ? mx_scrollbar : SX + SXSIZE);
  int *num_page_entries_last = num_page_entries_all_last[game_status];
  int *choice_store = choice_stores[game_status];
  int *first_entry_store = first_entry_stores[game_status];
  int choice = choice_store[mode];		// starts with 0
  int first_entry = first_entry_store[mode];	// starts with 0
  int x = 0;
  int y = choice - first_entry;
  int y_old = y;
  boolean position_set_by_scrollbar = (dx == 999);
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int i;

  if (button == MB_MENU_INITIALIZE)
  {
    // check if number of menu page entries has changed (may happen by change
    // of custom artwork definition value for 'list_size' for this menu screen)
    // (in this case, the last menu position most probably has to be corrected)
    if (num_page_entries != num_page_entries_last[mode])
    {
      choice_store[mode] = first_entry_store[mode] = 0;

      choice = first_entry = 0;
      y = y_old = 0;

      num_page_entries_last[mode] = num_page_entries;
    }

    // advance to first valid menu entry
    while (choice < num_page_entries &&
	   menu_info[choice].type & TYPE_SKIP_ENTRY)
      choice++;

    if (position_set_by_scrollbar)
      first_entry = first_entry_store[mode] = dy;
    else
      AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, max_page_entries,
		      NUM_MENU_ENTRIES_ON_SCREEN, first_entry);

    drawMenuInfoList(first_entry, num_page_entries, max_page_entries);

    if (choice < first_entry)
    {
      choice = first_entry;

      if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	choice++;
    }
    else if (choice > first_entry + num_page_entries - 1)
    {
      choice = first_entry + num_page_entries - 1;

      if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	choice--;
    }

    choice_store[mode] = choice;

    DrawCursorAndText_Menu(choice - first_entry, choice, TRUE);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    for (i = 0; i < max_page_entries; i++)
    {
      if (menu_info[i].type & TYPE_LEAVE_MENU)
      {
	void (*menu_callback_function)(void) = menu_info[i].value;

	FadeSetLeaveMenu();

	menu_callback_function();

	break;	// absolutely needed because function changes 'menu_info'!
      }
    }

    return;
  }

  if (mx || my)		// mouse input
  {
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	// keyboard or scrollbar/scrollbutton input
  {
    // move cursor instead of scrolling when already at start/end of list
    if (dy == -1 * SCROLL_LINE && first_entry == 0)
      dy = -1;
    else if (dy == +1 * SCROLL_LINE &&
	     first_entry + num_page_entries == max_page_entries)
      dy = 1;

    // handle scrolling screen one line or page
    if (y + dy < 0 ||
	y + dy > num_page_entries - 1)
    {
      boolean redraw = FALSE;

      if (ABS(dy) == SCROLL_PAGE)
	step = num_page_entries - 1;

      if (dy < 0 && first_entry > 0)
      {
	// scroll page/line up

	first_entry -= step;
	if (first_entry < 0)
	  first_entry = 0;

	redraw = TRUE;
      }
      else if (dy > 0 && first_entry + num_page_entries < max_page_entries)
      {
	// scroll page/line down

	first_entry += step;
	if (first_entry + num_page_entries > max_page_entries)
	  first_entry = MAX(0, max_page_entries - num_page_entries);

	redraw = TRUE;
      }

      if (redraw)
      {
	choice += first_entry - first_entry_store[mode];

	if (choice < first_entry)
	{
	  choice = first_entry;

	  if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	    choice++;
	}
	else if (choice > first_entry + num_page_entries - 1)
	{
	  choice = first_entry + num_page_entries - 1;

	  if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	    choice--;
	}
	else if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	{
	  choice += SIGN(dy);

	  if (choice < first_entry ||
	      choice > first_entry + num_page_entries - 1)
	  first_entry += SIGN(dy);
	}

	first_entry_store[mode] = first_entry;
	choice_store[mode] = choice;

	drawMenuInfoList(first_entry, num_page_entries, max_page_entries);

	DrawCursorAndText_Menu(choice - first_entry, choice, TRUE);

	AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, max_page_entries,
			NUM_MENU_ENTRIES_ON_SCREEN, first_entry);
      }

      return;
    }

    if (dx)
    {
      int menu_navigation_type = (dx < 0 ? TYPE_LEAVE : TYPE_ENTER);

      if (menu_info[choice].type & menu_navigation_type ||
	  menu_info[choice].type & TYPE_BOOLEAN_STYLE ||
	  menu_info[choice].type & TYPE_YES_NO_AUTO ||
	  menu_info[choice].type & TYPE_YES_NO_ASK ||
	  menu_info[choice].type & TYPE_YES_NO_ONCE ||
	  menu_info[choice].type & TYPE_PLAYER)
	button = MB_MENU_CHOICE;
    }
    else if (dy)
      y += dy;

    // jump to next non-empty menu entry (up or down)
    while (first_entry + y > 0 &&
	   first_entry + y < max_page_entries - 1 &&
	   menu_info[first_entry + y].type & TYPE_SKIP_ENTRY)
      y += dy;

    if (!IN_VIS_MENU(x, y))
    {
      choice += y - y_old;

      if (choice < first_entry)
	first_entry = choice;
      else if (choice > first_entry + num_page_entries - 1)
	first_entry = choice - num_page_entries + 1;

      if (first_entry >= 0 &&
	  first_entry + num_page_entries <= max_page_entries)
      {
	first_entry_store[mode] = first_entry;

	if (choice < first_entry)
	  choice = first_entry;
	else if (choice > first_entry + num_page_entries - 1)
	  choice = first_entry + num_page_entries - 1;

	choice_store[mode] = choice;

	drawMenuInfoList(first_entry, num_page_entries, max_page_entries);

	DrawCursorAndText_Menu(choice - first_entry, choice, TRUE);

	AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, max_page_entries,
			NUM_MENU_ENTRIES_ON_SCREEN, first_entry);
      }

      return;
    }
  }

  if (!anyScrollbarGadgetActive() &&
      IN_VIS_MENU(x, y) &&
      mx < mx_right_border &&
      y >= 0 && y < num_page_entries)
  {
    if (button)
    {
      if (first_entry + y != choice &&
	  menu_info[first_entry + y].type & ~TYPE_SKIP_ENTRY)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	DrawCursorAndText_Menu(choice - first_entry, choice, FALSE);
	DrawCursorAndText_Menu(y, first_entry + y, TRUE);

	choice = choice_store[mode] = first_entry + y;
      }
      else if (dx < 0)
      {
	PlaySound(SND_MENU_ITEM_SELECTING);

	for (i = 0; menu_info[i].type != 0; i++)
	{
	  if (menu_info[i].type & TYPE_LEAVE_MENU)
	  {
	    void (*menu_callback_function)(void) = menu_info[i].value;

	    FadeSetLeaveMenu();

	    menu_callback_function();

	    // absolutely needed because function changes 'menu_info'!
	    break;
	  }
	}

	return;
      }
    }
    else if (!(menu_info[first_entry + y].type & TYPE_GHOSTED))
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      // when selecting key headline, execute function for key value change
      if (menu_info[first_entry + y].type & TYPE_KEYTEXT &&
	  menu_info[first_entry + y + 1].type & TYPE_KEY)
	y++;

      // when selecting string value, execute function for list selection
      if (menu_info[first_entry + y].type & TYPE_STRING && y > 0 &&
	  menu_info[first_entry + y - 1].type & TYPE_ENTER_LIST)
	y--;

      // when selecting string value, execute function for text input gadget
      if (menu_info[first_entry + y].type & TYPE_STRING && y > 0 &&
	  menu_info[first_entry + y - 1].type & TYPE_TEXT_INPUT)
	y--;

      if (menu_info[first_entry + y].type & TYPE_ENTER_OR_LEAVE)
      {
	void (*menu_callback_function)(void) =
	  menu_info[first_entry + y].value;

	FadeSetFromType(menu_info[first_entry + y].type);

	menu_callback_function();
      }
      else if (menu_info[first_entry + y].type & TYPE_TEXT_INPUT)
      {
	void (*gadget_callback_function)(void) =
	  menu_info[first_entry + y].value;

	gadget_callback_function();
      }
      else if (menu_info[first_entry + y].type & TYPE_VALUE &&
	       menu_info == setup_info)
      {
	changeSetupValue(y, first_entry + y, dx);
      }
    }
  }
}

void HandleInfoScreen_Main(int mx, int my, int dx, int dy, int button)
{
  menu_info = info_info;

  HandleMenuScreen(mx, my, dx, dy, button,
		   info_mode, num_info_info, max_info_info);
}

static int getMenuFontSpacing(int spacing_height, int font_nr)
{
  int font_spacing = getFontHeight(font_nr) + EXTRA_SPACING(game_status);

  return (spacing_height < 0 ? ABS(spacing_height) * font_spacing :
	  spacing_height);
}

static int getMenuTextSpacing(int spacing_height, int font_nr)
{
  return (getMenuFontSpacing(spacing_height, font_nr) +
	  EXTRA_SPACING(game_status));
}

static int getMenuTextStep(int spacing_height, int font_nr)
{
  return getFontHeight(font_nr) + getMenuTextSpacing(spacing_height, font_nr);
}

static int getHeadlineSpacing(void)
{
  // special compatibility handling for "R'n'D jue 2022" game editions
  int spacing_check = menu.headline1_spacing[GAME_MODE_SCOREINFO];
  int spacing = (game_status == GAME_MODE_SCOREINFO ?
		 menu.headline1_spacing[GAME_MODE_SCOREINFO] :
		 menu.headline1_spacing_info[info_mode]);
  int font = MENU_INFO_FONT_TITLE;

  return (spacing_check != -2 ? getMenuTextStep(spacing, font) : 0);
}

void DrawInfoScreen_NotAvailable(char *text_title, char *text_error)
{
  int font_error = FONT_TEXT_2;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART + getHeadlineSpacing();
  int yfooter = MENU_SCREEN_INFO_FOOTER;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO);

  FadeOut(REDRAW_FIELD);

  ClearField();

  DrawInfoScreen_Headline(0, 1, FALSE);

  DrawTextSCentered(ystart, font_error, text_error);
  DrawTextSCentered(yfooter, font_foot, TEXT_NEXT_MENU);

  FadeIn(REDRAW_FIELD);
}

void DrawInfoScreen_HelpAnim(int start, int max_anims, boolean init)
{
  static int infoscreen_step[MAX_INFO_ELEMENTS_IN_ARRAY];
  static int infoscreen_frame[MAX_INFO_ELEMENTS_IN_ARRAY];
  int font_foot = MENU_INFO_FONT_FOOT;
  int xstart = mSX + MENU_SCREEN_INFO_SPACE_LEFT;
  int ystart = mSY + MENU_SCREEN_INFO_YSTART + getHeadlineSpacing();
  int yfooter = MENU_SCREEN_INFO_FOOTER;
  int ystep = MENU_SCREEN_INFO_YSTEP;
  int row_height = MENU_SCREEN_INFO_ENTRY_SIZE;
  int tilesize = MENU_SCREEN_INFO_TILE_SIZE;
  int yoffset = (row_height - tilesize) / 2;
  int element, action, direction;
  int graphic;
  int delay;
  int sync_frame;
  int i, j;

  if (init)
  {
    for (i = 0; i < NUM_INFO_ELEMENTS_ON_SCREEN; i++)
      infoscreen_step[i] = infoscreen_frame[i] = 0;

    DrawTextSCentered(yfooter, font_foot, TEXT_NEXT_PAGE);

    FrameCounter = 0;
  }

  i = j = 0;
  while (helpanim_info[j].element != HELPANIM_LIST_END)
  {
    if (i >= start + NUM_INFO_ELEMENTS_ON_SCREEN ||
	i >= max_anims)
      break;
    else if (i < start)
    {
      while (helpanim_info[j].element != HELPANIM_LIST_NEXT)
	j++;

      j++;
      i++;

      continue;
    }

    int ypos = i - start;
    int ystart_pos = ystart + ypos * ystep + yoffset;

    j += infoscreen_step[ypos];

    element = helpanim_info[j].element;
    action = helpanim_info[j].action;
    direction = helpanim_info[j].direction;

    if (element < 0)
      element = EL_UNKNOWN;

    if (action != -1 && direction != -1)
      graphic = el_act_dir2img(element, action, direction);
    else if (action != -1)
      graphic = el_act2img(element, action);
    else if (direction != -1)
      graphic = el_dir2img(element, direction);
    else
      graphic = el2img(element);

    delay = helpanim_info[j++].delay;

    if (delay == -1)
      delay = 1000000;

    if (infoscreen_frame[ypos] == 0)
    {
      sync_frame = 0;
      infoscreen_frame[ypos] = delay - 1;
    }
    else
    {
      sync_frame = delay - infoscreen_frame[ypos];
      infoscreen_frame[ypos]--;
    }

    if (helpanim_info[j].element == HELPANIM_LIST_NEXT)
    {
      if (!infoscreen_frame[ypos])
	infoscreen_step[ypos] = 0;
    }
    else
    {
      if (!infoscreen_frame[ypos])
	infoscreen_step[ypos]++;
      while (helpanim_info[j].element != HELPANIM_LIST_NEXT)
	j++;
    }

    j++;

    ClearRectangleOnBackground(drawto, xstart, ystart_pos, tilesize, tilesize);
    DrawSizedGraphicAnimationExt(drawto, xstart, ystart_pos,
				 graphic, sync_frame, tilesize, USE_MASKING);

    if (init)
      DrawInfoScreen_HelpText(element, action, direction, ypos);

    i++;
  }

  redraw_mask |= REDRAW_FIELD;

  FrameCounter++;
}

static char *getHelpText(int element, int action, int direction)
{
  char token[MAX_LINE_LEN];

  strcpy(token, element_info[element].token_name);

  if (action != -1)
    strcat(token, element_action_info[action].suffix);

  if (direction != -1)
    strcat(token, element_direction_info[MV_DIR_TO_BIT(direction)].suffix);

  return getHashEntry(helptext_info, token);
}

void DrawInfoScreen_HelpText(int element, int action, int direction, int ypos)
{
  int font_nr = FONT_INFO_ELEMENTS;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int line_spacing = MENU_SCREEN_INFO_SPACE_LINE;
  int left_spacing = MENU_SCREEN_INFO_SPACE_LEFT;
  int middle_spacing = MENU_SCREEN_INFO_SPACE_MIDDLE;
  int right_spacing = MENU_SCREEN_INFO_SPACE_RIGHT;
  int line_height = font_height + line_spacing;
  int row_height = MENU_SCREEN_INFO_ENTRY_SIZE;
  int tilesize = MENU_SCREEN_INFO_TILE_SIZE;
  int xstart = mSX + left_spacing + tilesize + middle_spacing;
  int ystart = mSY + MENU_SCREEN_INFO_YSTART + getHeadlineSpacing();
  int ystep = MENU_SCREEN_INFO_YSTEP;
  int pad_left = xstart - SX;
  int pad_right = right_spacing;
  int max_chars_per_line = (SXSIZE - pad_left - pad_right) / font_width;
  int max_lines_per_text = (row_height + line_spacing) / line_height;
  char *text = NULL;
  boolean autowrap = TRUE;
  boolean centered = FALSE;
  boolean parse_comments = FALSE;

  if (action != -1 && direction != -1)		// element.action.direction
    text = getHelpText(element, action, direction);

  if (text == NULL && action != -1)		// element.action
    text = getHelpText(element, action, -1);

  if (text == NULL && direction != -1)		// element.direction
    text = getHelpText(element, -1, direction);

  if (text == NULL)				// base element
    text = getHelpText(element, -1, -1);

  if (text == NULL)				// not found
    text = "No description available";

  DisableDrawingText();

  // first get number of text lines to calculate offset for centering text
  int num_lines_printed =
    DrawTextBuffer(0, 0, text, font_nr,
		   max_chars_per_line, -1, max_lines_per_text, -1, -1, -1, line_spacing, -1,
		   autowrap, centered, parse_comments);

  EnableDrawingText();

  int size_lines_printed = num_lines_printed * line_height - line_spacing;
  int yoffset = (row_height - size_lines_printed) / 2;

  DrawTextBuffer(xstart, ystart + ypos * ystep + yoffset, text, font_nr,
		 max_chars_per_line, -1, max_lines_per_text, -1, -1, -1, line_spacing, -1,
		 autowrap, centered, parse_comments);
}

static void DrawInfoScreen_TitleScreen(void)
{
  SetGameStatus(GAME_MODE_TITLE);

  UnmapAllGadgets();

  DrawTitleScreen();
}

void HandleInfoScreen_TitleScreen(int dx, int dy, int button)
{
  HandleTitleScreen(0, 0, dx, dy, button);
}

static void DrawInfoScreen_Elements(void)
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_ELEMENTS);

  UnmapAllGadgets();
  FadeInfoSoundsAndMusic();

  FadeOut(REDRAW_FIELD);

  LoadHelpAnimInfo();
  LoadHelpTextInfo();

  HandleInfoScreen_Elements(0, 0, MB_MENU_INITIALIZE);

  PlayInfoSoundsAndMusic();

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Elements(int dx, int dy, int button)
{
  static DelayCounter info_delay = { 0 };
  static int num_anims;
  static int num_pages;
  static int page;
  int anims_per_page = NUM_INFO_ELEMENTS_ON_SCREEN;
  int i;

  info_delay.value = GameFrameDelay;

  if (button == MB_MENU_INITIALIZE)
  {
    boolean new_element = TRUE;

    num_anims = 0;

    for (i = 0; helpanim_info[i].element != HELPANIM_LIST_END; i++)
    {
      if (helpanim_info[i].element == HELPANIM_LIST_NEXT)
	new_element = TRUE;
      else if (new_element)
      {
	num_anims++;
	new_element = FALSE;
      }
    }

    num_pages = (num_anims + anims_per_page - 1) / anims_per_page;
    page = 0;
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE || dx)
  {
    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      page += (dx < 0 ? -1 : +1);
    }

    if (page < 0 || page >= num_pages)
    {
      FadeInfoSoundsAndMusic();

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreen();

      return;
    }

    if (button != MB_MENU_INITIALIZE)
      FadeSetNextScreen();

    if (button != MB_MENU_INITIALIZE)
      FadeOut(REDRAW_FIELD);

    ClearField();

    DrawInfoScreen_Headline(page, num_pages, TRUE);
    DrawInfoScreen_HelpAnim(page * anims_per_page, num_anims, TRUE);

    if (button != MB_MENU_INITIALIZE)
      FadeIn(REDRAW_FIELD);
  }
  else
  {
    if (DelayReached(&info_delay))
      if (page < num_pages)
	DrawInfoScreen_HelpAnim(page * anims_per_page, num_anims, FALSE);

    PlayInfoSoundIfLoop();
  }
}

static void DrawInfoScreen_Music(void)
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_MUSIC);

  UnmapAllGadgets();

  FadeOut(REDRAW_FIELD);

  ClearField();

  DrawInfoScreen_Headline(0, 1, TRUE);

  LoadMusicInfo();

  HandleInfoScreen_Music(0, 0, MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Music(int dx, int dy, int button)
{
  static struct MusicFileInfo *list = NULL;
  static int num_screens = 0;
  static int screen_nr = 0;
  int font_title = MENU_INFO_FONT_TITLE;
  int font_head  = MENU_INFO_FONT_HEAD;
  int font_text  = MENU_INFO_FONT_TEXT;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int spacing_head = menu.headline2_spacing_info[info_mode];
  int ystep_head = getMenuTextStep(spacing_head,  font_head);
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART;
  int yfooter = MENU_SCREEN_INFO_FOOTER;

  if (button == MB_MENU_INITIALIZE)
  {
    struct MusicFileInfo *list_ptr = music_file_info;

    num_screens = 0;
    screen_nr = 0;

    while (list_ptr != NULL)
    {
      list_ptr = list_ptr->next;
      num_screens++;
    }

    list = music_file_info;

    if (list == NULL)
    {
      FadeMenuSoundsAndMusic();

      ClearField();

      DrawInfoScreen_Headline(0, 1, TRUE);

      DrawTextSCentered(ystart, font_title, "No music info for this level set.");
      DrawTextSCentered(yfooter, font_foot, TEXT_NEXT_MENU);

      return;
    }
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeMenuSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE || dx)
  {
    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (list != NULL)
      {
	list = (dx < 0 ? list->prev : list->next);
	screen_nr += (dx < 0 ? -1 : +1);
      }
    }

    if (list == NULL)
    {
      FadeMenuSoundsAndMusic();

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreen();

      return;
    }

    FadeMenuSoundsAndMusic();

    if (list != music_file_info)
      FadeSetNextScreen();

    if (button != MB_MENU_INITIALIZE)
      FadeOut(REDRAW_FIELD);

    ClearField();

    DrawInfoScreen_Headline(screen_nr, num_screens, TRUE);

    if (list->is_sound)
    {
      int sound = list->music;

      if (IS_LOOP_SOUND(sound))
	PlaySoundLoop(sound);
      else
	PlaySound(sound);
    }
    else
    {
      int music = list->music;

      if (IS_LOOP_MUSIC(music))
	PlayMusicLoop(music);
      else
	PlayMusic(music);
    }

    if (!strEqual(list->title, UNKNOWN_NAME))
    {
      if (!strEqual(list->title_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->title_header);
      else
	DrawTextSCentered(ystart, font_head, "Track");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "\"%s\"", list->title);
      ystart += ystep_head;
    }

    if (!strEqual(list->artist, UNKNOWN_NAME))
    {
      if (!strEqual(list->artist_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->artist_header);
      else
	DrawTextSCentered(ystart, font_head, "by");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "%s", list->artist);
      ystart += ystep_head;
    }

    if (!strEqual(list->album, UNKNOWN_NAME))
    {
      if (!strEqual(list->album_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->album_header);
      else
	DrawTextSCentered(ystart, font_head, "from the album");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "\"%s\"", list->album);
      ystart += ystep_head;
    }

    if (!strEqual(list->year, UNKNOWN_NAME))
    {
      if (!strEqual(list->year_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->year_header);
      else
	DrawTextSCentered(ystart, font_head, "from the year");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "%s", list->year);
      ystart += ystep_head;
    }

    if (!strEqual(list->played, UNKNOWN_NAME))
    {
      if (!strEqual(list->played_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->played_header);
      else
	DrawTextSCentered(ystart, font_head, "played in");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "%s", list->played);
      ystart += ystep_head;
    }
    else if (!list->is_sound)
    {
      int music_level_nr = -1;
      int i;

      // check if this music is configured for a certain level
      for (i = leveldir_current->first_level;
	   i <= leveldir_current->last_level; i++)
      {
	// (special case: "list->music" may be negative for unconfigured music)
	if (levelset.music[i] != MUS_UNDEFINED &&
	    levelset.music[i] == list->music)
	{
	  music_level_nr = i;

	  break;
	}
      }

      if (music_level_nr != -1)
      {
	if (!strEqual(list->played_header, UNKNOWN_NAME))
	  DrawTextSCentered(ystart, font_head, list->played_header);
	else
	  DrawTextSCentered(ystart, font_head, "played in");

	ystart += ystep_head;

	DrawTextFCentered(ystart, font_text, "level %03d", music_level_nr);
	ystart += ystep_head;
      }
    }

    DrawTextSCentered(yfooter, font_foot, TEXT_NEXT_PAGE);

    if (button != MB_MENU_INITIALIZE)
      FadeIn(REDRAW_FIELD);
  }

  if (list != NULL && list->is_sound && IS_LOOP_SOUND(list->music))
    PlaySoundLoop(list->music);
}

static void DrawInfoScreen_Version(void)
{
  int max_text_1_len = strlen("Version (xxxx)");
  int max_text_2_len = strlen("requested");
  int font_head = MENU_INFO_FONT_HEAD;
  int font_text = MENU_INFO_FONT_TEXT;
  int font_foot = MENU_INFO_FONT_FOOT;
  int spacing_head = menu.headline2_spacing_info[info_mode];
  int spacing_para = menu.paragraph_spacing_info[info_mode];
  int spacing_line = menu.line_spacing_info[info_mode];
  int spacing_midd = menu.middle_spacing_info[info_mode];
  int ystep_head = getMenuTextStep(spacing_head, font_head);
  int ystep_para = getMenuTextStep(spacing_para, font_text);
  int ystep_line = getMenuTextStep(spacing_line, font_text);
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART + getHeadlineSpacing();
  int yfooter = MENU_SCREEN_INFO_FOOTER;
  int xstart1 = mSX - SX + MENU_SCREEN_INFO_SPACE_LEFT;
  int xstart2 = xstart1 + max_text_1_len * getFontWidth(font_text) + spacing_midd;
  int xstart3 = xstart2 + max_text_2_len * getFontWidth(font_text) + spacing_midd;
  SDL_version sdl_version_compiled;
  const SDL_version *sdl_version_linked;
  int driver_name_len = 10;
  SDL_version sdl_version_linked_ext;
  const char *driver_name = NULL;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_VERSION);

  UnmapAllGadgets();
  FadeInfoSoundsAndMusic();

  FadeOut(REDRAW_FIELD);

  ClearField();

  DrawInfoScreen_Headline(0, 1, TRUE);

  DrawTextF(xstart1, ystart, font_head, "Name");
  DrawTextF(xstart2, ystart, font_text, getProgramTitleString());
  ystart += ystep_line;

  if (!strEqual(getProgramVersionString(), getProgramRealVersionString()))
  {
    DrawTextF(xstart1, ystart, font_head, "Version (fake)");
    DrawTextF(xstart2, ystart, font_text, getProgramVersionString());
    ystart += ystep_line;

    DrawTextF(xstart1, ystart, font_head, "Version (real)");
    DrawTextF(xstart2, ystart, font_text, getProgramRealVersionString());
    ystart += ystep_line;
  }
  else
  {
    DrawTextF(xstart1, ystart, font_head, "Version");
    DrawTextF(xstart2, ystart, font_text, getProgramVersionString());
    ystart += ystep_line;
  }

  DrawTextF(xstart1, ystart, font_head, "Platform");
  DrawTextF(xstart2, ystart, font_text, "%s (%s)",
	    PLATFORM_STRING,
	    PLATFORM_XX_BIT_STRING);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Target");
  DrawTextF(xstart2, ystart, font_text, TARGET_STRING);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Source date");
  DrawTextF(xstart2, ystart, font_text, getSourceDateString());
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Commit hash");
  DrawTextF(xstart2, ystart, font_text, getSourceHashString());
  ystart += ystep_para;

  DrawTextF(xstart1, ystart, font_head, "Library");
  DrawTextF(xstart2, ystart, font_head, "compiled");
  DrawTextF(xstart3, ystart, font_head, "linked");
  ystart += ystep_head;

  SDL_VERSION(&sdl_version_compiled);
  SDL_GetVersion(&sdl_version_linked_ext);
  sdl_version_linked = &sdl_version_linked_ext;

  DrawTextF(xstart1, ystart, font_text, "SDL");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_line;

  SDL_IMAGE_VERSION(&sdl_version_compiled);
  sdl_version_linked = IMG_Linked_Version();

  DrawTextF(xstart1, ystart, font_text, "SDL_image");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_line;

  SDL_MIXER_VERSION(&sdl_version_compiled);
  sdl_version_linked = Mix_Linked_Version();

  DrawTextF(xstart1, ystart, font_text, "SDL_mixer");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_line;

  SDL_NET_VERSION(&sdl_version_compiled);
  sdl_version_linked = SDLNet_Linked_Version();

  DrawTextF(xstart1, ystart, font_text, "SDL_net");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_para;

  DrawTextF(xstart1, ystart, font_head, "Driver");
  DrawTextF(xstart2, ystart, font_head, "requested");
  DrawTextF(xstart3, ystart, font_head, "used");
  ystart += ystep_head;

  driver_name =
    getStringCopyNStatic(SDLGetRendererName(), driver_name_len);

  DrawTextF(xstart1, ystart, font_text, "Render Driver");
  DrawTextF(xstart2, ystart, font_text, "%s", setup.system.sdl_renderdriver);
  DrawTextF(xstart3, ystart, font_text, "%s", driver_name);
  ystart += ystep_line;

  driver_name =
    getStringCopyNStatic(SDL_GetCurrentVideoDriver(), driver_name_len);

  DrawTextF(xstart1, ystart, font_text, "Video Driver");
  DrawTextF(xstart2, ystart, font_text, "%s", setup.system.sdl_videodriver);
  DrawTextF(xstart3, ystart, font_text, "%s", driver_name);
  ystart += ystep_line;

  driver_name =
    getStringCopyNStatic(SDL_GetCurrentAudioDriver(), driver_name_len);

  DrawTextF(xstart1, ystart, font_text, "Audio Driver");
  DrawTextF(xstart2, ystart, font_text, "%s", setup.system.sdl_audiodriver);
  DrawTextF(xstart3, ystart, font_text, "%s", driver_name);

  DrawTextSCentered(yfooter, font_foot, TEXT_NEXT_MENU);

  PlayInfoSoundsAndMusic();

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Version(int button)
{
  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeMenuSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

static char *getInfoScreenTitle_Generic(void)
{
  return (info_mode == INFO_MODE_MAIN     ? STR_INFO_MAIN     :
	  info_mode == INFO_MODE_TITLE    ? STR_INFO_TITLE    :
	  info_mode == INFO_MODE_ELEMENTS ? STR_INFO_ELEMENTS :
	  info_mode == INFO_MODE_MUSIC    ? STR_INFO_MUSIC    :
	  info_mode == INFO_MODE_CREDITS  ? STR_INFO_CREDITS  :
	  info_mode == INFO_MODE_PROGRAM  ? STR_INFO_PROGRAM  :
	  info_mode == INFO_MODE_VERSION  ? STR_INFO_VERSION  :
	  info_mode == INFO_MODE_LEVELSET ? STR_INFO_LEVELSET :
	  info_mode == INFO_MODE_LEVEL    ? STR_INFO_LEVEL    :
	  info_mode == INFO_MODE_STORY    ? STR_INFO_STORY    :
	  "");
}

static int getInfoScreenBackgroundImage_Generic(void)
{
  return (info_mode == INFO_MODE_ELEMENTS ? IMG_BACKGROUND_INFO_ELEMENTS :
	  info_mode == INFO_MODE_MUSIC    ? IMG_BACKGROUND_INFO_MUSIC    :
	  info_mode == INFO_MODE_CREDITS  ? IMG_BACKGROUND_INFO_CREDITS  :
	  info_mode == INFO_MODE_PROGRAM  ? IMG_BACKGROUND_INFO_PROGRAM  :
	  info_mode == INFO_MODE_VERSION  ? IMG_BACKGROUND_INFO_VERSION  :
	  info_mode == INFO_MODE_LEVELSET ? IMG_BACKGROUND_INFO_LEVELSET :
	  info_mode == INFO_MODE_LEVEL    ? IMG_BACKGROUND_INFO_LEVEL    :
	  info_mode == INFO_MODE_STORY    ? IMG_BACKGROUND_STORY         :
	  IMG_BACKGROUND_INFO);
}

static int getInfoScreenBackgroundSound_Generic(void)
{
  return (info_mode == INFO_MODE_ELEMENTS ? SND_BACKGROUND_INFO_ELEMENTS :
	  info_mode == INFO_MODE_CREDITS  ? SND_BACKGROUND_INFO_CREDITS  :
	  info_mode == INFO_MODE_PROGRAM  ? SND_BACKGROUND_INFO_PROGRAM  :
	  info_mode == INFO_MODE_VERSION  ? SND_BACKGROUND_INFO_VERSION  :
	  info_mode == INFO_MODE_LEVELSET ? SND_BACKGROUND_INFO_LEVELSET :
	  info_mode == INFO_MODE_LEVEL    ? SND_BACKGROUND_INFO_LEVEL    :
	  info_mode == INFO_MODE_STORY    ? SND_BACKGROUND_STORY         :
	  SND_BACKGROUND_INFO);
}

static int getInfoScreenBackgroundMusic_Generic(void)
{
  return (info_mode == INFO_MODE_ELEMENTS ? MUS_BACKGROUND_INFO_ELEMENTS :
	  info_mode == INFO_MODE_CREDITS  ? MUS_BACKGROUND_INFO_CREDITS  :
	  info_mode == INFO_MODE_PROGRAM  ? MUS_BACKGROUND_INFO_PROGRAM  :
	  info_mode == INFO_MODE_VERSION  ? MUS_BACKGROUND_INFO_VERSION  :
	  info_mode == INFO_MODE_LEVELSET ? MUS_BACKGROUND_INFO_LEVELSET :
	  info_mode == INFO_MODE_LEVEL    ? MUS_BACKGROUND_INFO_LEVEL    :
	  info_mode == INFO_MODE_STORY    ? MUS_BACKGROUND_STORY         :
	  MUS_BACKGROUND_INFO);
}

static char *getInfoScreenFilename_Generic(int nr, boolean global)
{
  return (info_mode == INFO_MODE_CREDITS  ? getCreditsFilename(nr, global)  :
	  info_mode == INFO_MODE_PROGRAM  ? getProgramInfoFilename(nr)      :
	  info_mode == INFO_MODE_LEVELSET ? getLevelSetInfoFilename(nr)     :
	  info_mode == INFO_MODE_LEVEL    ? getLevelInfoFilename(level_nr)  :
	  info_mode == INFO_MODE_STORY    ? getLevelStoryFilename(level_nr) :
	  NULL);
}

static char *getInfoScreenBuffer_Generic(void)
{
  return (info_mode == INFO_MODE_LEVELSET ? getLevelSetInfoBuffer(!info_screens_from_main) :
	  info_mode == INFO_MODE_LEVEL    ? getLevelInfoBuffer(!info_screens_from_main)    :
	  info_mode == INFO_MODE_STORY    ? getLevelStoryBuffer()                          :
	  NULL);
}

static void DrawInfoScreen_GenericText(struct WrappedTextInfo *wrapped_text,
                                       struct TitleMessageInfo *tmi, int start_pos)
{
  int x = mSX + ALIGNED_TEXT_XPOS(tmi);
  int y = mSY + ALIGNED_TEXT_YPOS(tmi);

  // clear info text area, but not title or scrollbar
  DrawBackground(x, y, tmi->width, tmi->height);

  DrawWrappedText(x, y, wrapped_text, start_pos);
}

static void SetWrappedText_GenericScreen(struct TitleMessageInfo *tmi,
                                         int screen_nr, int use_global_screens)
{
  char *filename = getInfoScreenFilename_Generic(screen_nr, use_global_screens);
  char *buffer   = getInfoScreenBuffer_Generic();
  char *raw_text = (filename != NULL ?	// always prefer info text files over buffers
                    GetTextBufferFromFile(filename, MAX_OUTPUT_LINES) :
                    getLatin1FromUTF8(buffer));
  int line_spacing = getMenuTextSpacing(menu.line_spacing_info[info_mode], tmi->font);

  FreeWrappedText(wrapped_text);

  wrapped_text = GetWrappedTextBuffer(raw_text, tmi->font, -1, -1, -1, tmi->width, -1, tmi->height,
                                      line_spacing, -1,
                                      tmi->autowrap, tmi->centered, tmi->parse_comments);
  checked_free(raw_text);
}

static void DrawInfoScreen_GenericScreen(int screen_nr, int num_screens, int use_global_screens)
{
  static struct TitleMessageInfo tmi_info;
  struct TitleMessageInfo *tmi = &tmi_info;
  int font_text = MENU_INFO_FONT_TEXT;
  int font_foot = MENU_INFO_FONT_FOOT;
  int yfooter = MENU_SCREEN_INFO_FOOTER;

  // unmap optional scroll bar gadgets (may not be used on this screen)
  UnmapScreenGadgets();

  ClearField();

  DrawInfoScreen_Headline(screen_nr, num_screens, use_global_screens);

  int draw_xoffset = mSX - SX;
  int draw_yoffset = mSY - SY;

  tmi->x = MENU_SCREEN_INFO_SPACE_LEFT;
  tmi->y = MENU_SCREEN_INFO_SPACE_TOP + getHeadlineSpacing();
  tmi->width  = SXSIZE - draw_xoffset - tmi->x - MENU_SCREEN_INFO_SPACE_RIGHT;
  tmi->height = SYSIZE - draw_yoffset - tmi->y - MENU_SCREEN_INFO_SPACE_BOTTOM - 10;
  tmi->align = ALIGN_LEFT;
  tmi->valign = VALIGN_TOP;
  tmi->font = font_text;

  if (info_mode == INFO_MODE_CREDITS ||
      info_mode == INFO_MODE_PROGRAM)
  {
    tmi->autowrap = FALSE;
    tmi->centered = TRUE;
    tmi->parse_comments = TRUE;
  }
  else if (info_mode == INFO_MODE_LEVELSET ||
           info_mode == INFO_MODE_LEVEL ||
           info_mode == INFO_MODE_STORY)
  {
    tmi->autowrap = readme.autowrap;
    tmi->centered = readme.centered;
    tmi->parse_comments = readme.parse_comments;

    tmi->font = (info_mode == INFO_MODE_LEVELSET ? FONT_INFO_LEVELSET :
                 info_mode == INFO_MODE_LEVEL    ? FONT_INFO_LEVEL    : FONT_INFO_STORY);
  }

  SetWrappedText_GenericScreen(tmi, screen_nr, use_global_screens);

  if (wrapped_text != NULL && wrapped_text->total_height > wrapped_text->max_height)
  {
    // re-wrap text with text width reduced by scroll bar width
    tmi->width -= SC_SCROLL_VERTICAL_XSIZE;

    // adjust left border and text width if text should be centered
    if (wrapped_text->line[0].centered &&
        MENU_SCREEN_INFO_SPACE_LEFT == MENU_SCREEN_INFO_SPACE_RIGHT)
    {
      tmi->x += SC_SCROLL_VERTICAL_XSIZE;
      tmi->width -= SC_SCROLL_VERTICAL_XSIZE;
    }

    SetWrappedText_GenericScreen(tmi, screen_nr, use_global_screens);

    int start_pos = 0;

    while (wrapped_text->line_visible_last < wrapped_text->num_lines - 1)
      InitWrappedText(0, 0, wrapped_text, ++start_pos);

    int items_max = wrapped_text->num_lines;
    int items_visible = wrapped_text->num_lines - start_pos;
    int item_position = 0;

    AdjustInfoScreenGadgets(mSY + ALIGNED_TEXT_YPOS(tmi), wrapped_text->max_height,
                            items_max, items_visible, item_position);

    MapScreenInfoGadgets();
  }

  DrawInfoScreen_GenericText(wrapped_text, tmi, 0);

  wrapped_tmi = tmi;

  boolean last_screen = (screen_nr == num_screens - 1);
  char *text_foot = (last_screen ? TEXT_NEXT_MENU : TEXT_NEXT_PAGE);

  DrawTextSCentered(yfooter, font_foot, text_foot);

  // redraw level selection buttons (which have just been erased)
  if (info_mode == INFO_MODE_LEVEL)
    RedrawScreenMenuGadgets(SCREEN_MASK_INFO);
}

static void DrawInfoScreen_Generic(void)
{
  if (info_mode == INFO_MODE_STORY)
  {
    SetMainBackgroundImage(IMG_BACKGROUND_STORY);
  }
  else
  {
    SetMainBackgroundImage(IMG_BACKGROUND_INFO);
    SetMainBackgroundImageIfDefined(getInfoScreenBackgroundImage_Generic());
  }

  UnmapAllGadgets();
  FadeInfoSoundsAndMusic();

  FadeOut(REDRAW_FIELD);

  FreeScreenGadgets();
  CreateScreenGadgets();

  // map gadgets for level info screen
  if (info_mode == INFO_MODE_LEVEL)
    MapScreenMenuGadgets(SCREEN_MASK_INFO);

  HandleInfoScreen_Generic(0, 0, 0, 0, MB_MENU_INITIALIZE);

  PlayInfoSoundsAndMusic();

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Generic(int mx, int my, int dx, int dy, int button)
{
  static char *text_no_info = "";
  static int num_screens = 0;
  static int screen_nr = 0;
  static int start_pos = 0;
  static boolean use_global_screens = FALSE;
  boolean position_set_by_scrollbar = (dx == 999);

  if (button == MB_MENU_INITIALIZE)
  {
    if (position_set_by_scrollbar)
    {
      int items_max = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->scrollbar.items_max;
      int items_visible = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->scrollbar.items_visible;

      // check if scrollbar was moved by one screen, or to top or bottom position
      if (ABS(dy - start_pos) == items_visible - 1 || dy == 0 || dy == items_max - items_visible)
      {
        // use dynamic "next screen" calculation instead of adding static offset
        // (required for text using multiple fonts with different vertical sizes)
        HandleInfoScreen(0, 0, 0, SIGN(dy - start_pos) * SCROLL_PAGE, MB_MENU_MARK);
      }
      else
      {
        start_pos = dy;

        DrawInfoScreen_GenericText(wrapped_text, wrapped_tmi, start_pos);
      }

      return;
    }

    num_screens = 0;
    screen_nr = 0;
    start_pos = 0;

    if (info_mode == INFO_MODE_CREDITS)
    {
      int i;

      for (i = 0; i < 2; i++)
      {
	use_global_screens = i;		// check for "FALSE", then "TRUE"

	// determine number of (global or level set specific) credits screens
	while (getCreditsFilename(num_screens, use_global_screens) != NULL)
	  num_screens++;

	if (num_screens > 0)
	  break;
      }

      text_no_info = "No credits available.";
    }
    else if (info_mode == INFO_MODE_PROGRAM)
    {
      use_global_screens = TRUE;

      // determine number of program info screens
      while (getProgramInfoFilename(num_screens) != NULL)
	num_screens++;

      text_no_info = "No program info available.";
    }
    else if (info_mode == INFO_MODE_LEVELSET)
    {
      use_global_screens = FALSE;

      // determine number of levelset info screens
      while (getLevelSetInfoFilename(num_screens) != NULL)
	num_screens++;

      if (num_screens == 0 && hasLevelSetInfo(TRUE))
        num_screens = 1;

      text_no_info = "No level set info available.";
    }
    else if (info_mode == INFO_MODE_LEVEL)
    {
      use_global_screens = FALSE;

      // determine number of level info screens
      if (hasLevelInfo(TRUE))
        num_screens = 1;

      text_no_info = "No level info available.";
    }
    else if (info_mode == INFO_MODE_STORY)
    {
      // copy all ".STORY" settings to ".INFO[STORY]", which is internally used to show story
      menu.draw_xoffset_info[INFO_MODE_STORY]		= menu.draw_xoffset[GAME_MODE_STORY];
      menu.draw_yoffset_info[INFO_MODE_STORY]		= menu.draw_yoffset[GAME_MODE_STORY];
      menu.left_spacing_info[INFO_MODE_STORY]		= menu.left_spacing[GAME_MODE_STORY];
      menu.right_spacing_info[INFO_MODE_STORY]		= menu.right_spacing[GAME_MODE_STORY];
      menu.top_spacing_info[INFO_MODE_STORY]		= menu.top_spacing[GAME_MODE_STORY];
      menu.bottom_spacing_info[INFO_MODE_STORY]		= menu.bottom_spacing[GAME_MODE_STORY];
      menu.paragraph_spacing_info[INFO_MODE_STORY]	= menu.paragraph_spacing[GAME_MODE_STORY];
      menu.headline1_spacing_info[INFO_MODE_STORY]	= menu.headline1_spacing[GAME_MODE_STORY];
      menu.headline2_spacing_info[INFO_MODE_STORY]	= menu.headline2_spacing[GAME_MODE_STORY];
      menu.line_spacing_info[INFO_MODE_STORY]		= menu.line_spacing[GAME_MODE_STORY];
      menu.extra_spacing_info[INFO_MODE_STORY]		= menu.extra_spacing[GAME_MODE_STORY];

      use_global_screens = FALSE;

      // determine number of level story screens
      if (hasLevelStory())
        num_screens = 1;

      text_no_info = "No level story available.";
    }

    if (num_screens == 0)
    {
      int font_title = MENU_INFO_FONT_TITLE;
      int font_foot  = MENU_INFO_FONT_FOOT;
      int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART;
      int yfooter = MENU_SCREEN_INFO_FOOTER;

      ClearField();

      DrawInfoScreen_Headline(screen_nr, num_screens, use_global_screens);

      DrawTextSCentered(ystart, font_title, text_no_info);
      DrawTextSCentered(yfooter, font_foot, TEXT_NEXT_MENU);

      // redraw level selection buttons (which have just been erased)
      if (info_mode == INFO_MODE_LEVEL)
        RedrawScreenMenuGadgets(SCREEN_MASK_INFO);

      return;
    }

    DrawInfoScreen_GenericScreen(screen_nr, num_screens, use_global_screens);
  }
  else if (info_mode == INFO_MODE_LEVEL && ABS(dx) == 1)
  {
    HandleInfoScreen_SelectLevel(1, dx);

    return;
  }
  else if (button == MB_MENU_LEAVE || dx < 0)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    // if escaping from level info screen on game start, go back to main menu
    if (info_screens_from_game)
    {
      info_screens_from_main = TRUE;
      info_screens_from_game = FALSE;
    }

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();
  }
  else if (button == MB_MENU_CONTINUE)
  {
    // if space key was pressed, show next page of info screen, if available
    if (wrapped_text->line_visible_last < wrapped_text->num_lines - 1)
      HandleInfoScreen(0, 0, 0, +1 * SCROLL_PAGE, MB_MENU_MARK);
    else
      HandleInfoScreen(0, 0, 0, 0, MB_MENU_CHOICE);

    return;
  }
  else if ((mx >= 0 && my >= 0 && button == MB_MENU_CHOICE) || dx > 0)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    screen_nr += (dx < 0 ? -1 : +1);
    start_pos = 0;

    if (screen_nr < 0 || screen_nr >= num_screens)
    {
      FadeInfoSoundsAndMusic();

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreen();
    }
    else
    {
      FadeSetNextScreen();

      FadeOut(REDRAW_FIELD);

      DrawInfoScreen_GenericScreen(screen_nr, num_screens, use_global_screens);

      FadeIn(REDRAW_FIELD);
    }
  }
  else if ((dy < 0 && wrapped_text->line_visible_first > 0) ||
           (dy > 0 && wrapped_text->line_visible_last < wrapped_text->num_lines - 1))
  {
    if (ABS(dy) == SCROLL_PAGE)
    {
      if (dy < 0)
      {
        int old_line_visible_first = wrapped_text->line_visible_first;

        while (wrapped_text->line_visible_first > 0 &&
               wrapped_text->line_visible_last > old_line_visible_first)
          InitWrappedText(0, 0, wrapped_text, --start_pos);
      }
      else
      {
        int old_line_visible_last = wrapped_text->line_visible_last;

        while (wrapped_text->line_visible_last < wrapped_text->num_lines - 1 &&
               wrapped_text->line_visible_first < old_line_visible_last)
          InitWrappedText(0, 0, wrapped_text, ++start_pos);
      }
    }
    else
    {
      start_pos += SIGN(dy);
    }

    DrawInfoScreen_GenericText(wrapped_text, wrapped_tmi, start_pos);

    ModifyGadget(screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL],
                 GDI_SCROLLBAR_ITEM_POSITION, start_pos, GDI_END);
  }
  else
  {
    PlayInfoSoundIfLoop();
  }
}

static void DrawInfoScreen(void)
{
  if (info_mode == INFO_MODE_TITLE)
    DrawInfoScreen_TitleScreen();
  else if (info_mode == INFO_MODE_ELEMENTS)
    DrawInfoScreen_Elements();
  else if (info_mode == INFO_MODE_MUSIC)
    DrawInfoScreen_Music();
  else if (info_mode == INFO_MODE_CREDITS)
    DrawInfoScreen_Generic();
  else if (info_mode == INFO_MODE_PROGRAM)
    DrawInfoScreen_Generic();
  else if (info_mode == INFO_MODE_VERSION)
    DrawInfoScreen_Version();
  else if (info_mode == INFO_MODE_LEVELSET)
    DrawInfoScreen_Generic();
  else if (info_mode == INFO_MODE_LEVEL)
    DrawInfoScreen_Generic();
  else if (info_mode == INFO_MODE_STORY)
    DrawInfoScreen_Generic();
  else
    DrawInfoScreen_Main();
}

static void DrawInfoScreen_FromMainMenuOrInitGame(int nr, boolean from_game_status)
{
  int fade_mask = REDRAW_FIELD;

  if (nr < INFO_MODE_MAIN || nr >= MAX_INFO_MODES)
    return;

  if (from_game_status == GAME_MODE_MAIN)
    info_screens_from_main = TRUE;
  else if (from_game_status == GAME_MODE_PLAYING)
    info_screens_from_game = TRUE;
  else
    return;

  if (from_game_status == GAME_MODE_MAIN)
    SetGameStatus(GAME_MODE_INFO);
  else if (from_game_status == GAME_MODE_PLAYING)
    SetGameStatus(GAME_MODE_STORY);
  else
    return;

  CloseDoor(DOOR_CLOSE_ALL);

  info_mode = nr;

  if (redraw_mask & REDRAW_ALL)
    fade_mask = REDRAW_ALL;

  if (CheckFadeAll())
    fade_mask = REDRAW_ALL;

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  FadeSetEnterScreen();

  FadeOut(fade_mask);

  FadeSkipNextFadeOut();

  // needed if different viewport properties defined for info screen
  ChangeViewportPropertiesIfNeeded();

  DrawInfoScreen();
}

void DrawInfoScreen_FromMainMenu(int nr)
{
  DrawInfoScreen_FromMainMenuOrInitGame(nr, GAME_MODE_MAIN);
}

void DrawInfoScreen_FromInitGame(int nr)
{
  DrawInfoScreen_FromMainMenuOrInitGame(nr, GAME_MODE_PLAYING);
}

boolean ShowStoryScreen_FromInitGame(void)
{
  if (!hasLevelStory())
    return FALSE;

  if (setup.show_level_story == STATE_FALSE ||
      (setup.show_level_story == STATE_ONCE && levelset.level_story_shown[level_nr]))
    return FALSE;

  levelset.level_story_shown[level_nr] = TRUE;

  DrawInfoScreen_FromInitGame(INFO_MODE_STORY);

  return TRUE;
}

static void HandleInfoScreen_SelectLevel(int step, int direction)
{
  int old_level_nr = level_nr;
  int new_level_nr = old_level_nr + step * direction;

  if (new_level_nr < leveldir_current->first_level)
    new_level_nr = leveldir_current->first_level;
  if (new_level_nr > leveldir_current->last_level)
    new_level_nr = leveldir_current->last_level;

  if (setup.allow_skipping_levels != STATE_TRUE && new_level_nr > leveldir_current->handicap_level)
    new_level_nr = leveldir_current->handicap_level;

  if (new_level_nr != old_level_nr)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    level_nr = new_level_nr;

    LoadLevel(level_nr);

    HandleInfoScreen_Generic(0, 0, 0, 0, MB_MENU_INITIALIZE);

    SaveLevelSetup_SeriesInfo();
  }
}

void HandleInfoScreen(int mx, int my, int dx, int dy, int button)
{
  // fix "continue" button mode for screens that do not support it
  if (button == MB_MENU_CONTINUE && (info_mode != INFO_MODE_CREDITS &&
                                     info_mode != INFO_MODE_PROGRAM &&
                                     info_mode != INFO_MODE_LEVELSET &&
                                     info_mode != INFO_MODE_LEVEL &&
                                     info_mode != INFO_MODE_STORY))
    button = MB_MENU_CHOICE;

  if (info_mode == INFO_MODE_TITLE)
    HandleInfoScreen_TitleScreen(dx, dy, button);
  else if (info_mode == INFO_MODE_ELEMENTS)
    HandleInfoScreen_Elements(dx, dy, button);
  else if (info_mode == INFO_MODE_MUSIC)
    HandleInfoScreen_Music(dx, dy, button);
  else if (info_mode == INFO_MODE_CREDITS)
    HandleInfoScreen_Generic(mx, my, dx, dy, button);
  else if (info_mode == INFO_MODE_PROGRAM)
    HandleInfoScreen_Generic(mx, my, dx, dy, button);
  else if (info_mode == INFO_MODE_VERSION)
    HandleInfoScreen_Version(button);
  else if (info_mode == INFO_MODE_LEVELSET)
    HandleInfoScreen_Generic(mx, my, dx, dy, button);
  else if (info_mode == INFO_MODE_LEVEL)
    HandleInfoScreen_Generic(mx, my, dx, dy, button);
  else if (info_mode == INFO_MODE_STORY)
    HandleInfoScreen_Generic(mx, my, dx, dy, button);
  else
    HandleInfoScreen_Main(mx, my, dx, dy, button);
}

void HandleStoryScreen(int mx, int my, int dx, int dy, int button)
{
  HandleInfoScreen(mx, my, dx, dy, button);
}


// ============================================================================
// type name functions
// ============================================================================

static TreeInfo *type_name_node = NULL;
static char type_name_last[MAX_PLAYER_NAME_LEN + 1] = { 0 };
static int type_name_nr = 0;

static int getPlayerNameColor(char *name)
{
  return (strEqual(name, EMPTY_PLAYER_NAME) ? FC_BLUE : FC_RED);
}

static void drawTypeNameText(char *name, struct TextPosInfo *pos,
			     boolean active)
{
  char text[MAX_PLAYER_NAME_LEN + 2] = { 0 };
  boolean multiple_users = (game_status == GAME_MODE_PSEUDO_TYPENAMES);
  int sx = (multiple_users ? amSX + pos->x : mSX + ALIGNED_TEXT_XPOS(pos));
  int sy = (multiple_users ? amSY + pos->y : mSY + ALIGNED_TEXT_YPOS(pos));
  int font_nr = (active ? FONT_ACTIVE(pos->font) : pos->font);
  int font_width = getFontWidth(font_nr);
  int font_xoffset = getFontDrawOffsetX(font_nr);
  int font_yoffset = getFontDrawOffsetY(font_nr);
  int font_sx = sx + font_xoffset;
  int font_sy = sy + font_yoffset;

  DrawBackgroundForFont(font_sx, font_sy, pos->width, pos->height, font_nr);

  sprintf(text, "%s%c", name, (active ? '_' : '\0'));

  pos->width = strlen(text) * font_width;
  sx = (multiple_users ? amSX + pos->x : mSX + ALIGNED_TEXT_XPOS(pos));

  DrawText(sx, sy, text, font_nr);
}

static void getTypeNameValues(char *name, struct TextPosInfo *pos, int *xpos)
{
  struct MainControlInfo *mci = getMainControlInfo(MAIN_CONTROL_NAME);

  *pos = *mci->pos_input;

  if (game_status == GAME_MODE_PSEUDO_TYPENAMES)
  {
    TreeInfo *ti = player_name_current;
    int first_entry = ti->cl_first;
    int entry_pos = first_entry + ti->cl_cursor;
    TreeInfo *node_first = getTreeInfoFirstGroupEntry(ti);
    int xpos = MENU_SCREEN_START_XPOS;
    int ypos = MENU_SCREEN_START_YPOS + ti->cl_cursor;

    type_name_node = getTreeInfoFromPos(node_first, entry_pos);
    type_name_nr = entry_pos;

    strcpy(name, type_name_node->name);

    pos->x = xpos * 32;
    pos->y = ypos * 32;
    pos->width = MAX_PLAYER_NAME_LEN * 32;
  }
  else
  {
    type_name_nr = user.nr;

    strcpy(name, setup.player_name);
  }

  strcpy(type_name_last, name);

  if (strEqual(name, EMPTY_PLAYER_NAME))
    strcpy(name, "");

  *xpos = strlen(name);
}

static void setTypeNameValues_Name(char *name, struct TextPosInfo *pos)
{
  // change name of edited user in global list of user names
  setString(&global.user_names[type_name_nr], name);

  if (game_status == GAME_MODE_PSEUDO_TYPENAMES)
  {
    TreeInfo *node = type_name_node;

    // change name of edited user in local menu tree structure
    setString(&node->name, name);
    setString(&node->name_sorting, name);

    node->color = getPlayerNameColor(name);
    pos->font = MENU_CHOOSE_TREE_FONT(node->color);
  }
}

static void setTypeNameValues(char *name, struct TextPosInfo *pos,
			      boolean changed)
{
  boolean reset_setup = strEqual(name, "");
  boolean remove_user = strEqual(name, EMPTY_PLAYER_NAME);
  boolean create_user = strEqual(type_name_last, EMPTY_PLAYER_NAME);

  if (!changed)
    strcpy(name, type_name_last);

  if (strEqual(name, ""))
    strcpy(name, EMPTY_PLAYER_NAME);

  setTypeNameValues_Name(name, pos);

  // if player name not changed, no further action required
  if (strEqual(name, type_name_last))
    return;

  // redraw player name before (possibly) opening request dialogs
  drawTypeNameText(name, pos, FALSE);

  int last_user_nr = user.nr;

  if (game_status == GAME_MODE_PSEUDO_TYPENAMES)
  {
    // save setup of currently active user (may differ from edited user)
    SaveSetup();

    // temporarily change active user to edited user
    user.nr = type_name_nr;

    if (create_user &&
        Request("Use current setup values for the new player?", REQ_ASK))
    {
      // use current setup values for new user, but create new player UUID
      setup.player_uuid = getStringCopy(getUUID());
    }
    else
    {
      // load setup for existing user (or start with defaults for new user)
      LoadSetup();
    }
  }

  char *setup_filename = getSetupFilename();
  boolean setup_exists = fileExists(setup_filename);

  // change name of edited user in setup structure
  strcpy(setup.player_name, name);

  // save setup of edited user
  SaveSetup();

  // change name of edited user on score server
  ApiRenamePlayerAsThread();

  if (game_status == GAME_MODE_PSEUDO_TYPENAMES || reset_setup)
  {
    if (reset_setup)
    {
      if (Request("Reset setup values for this player?", REQ_ASK))
      {
	// remove setup config file
	unlink(setup_filename);

	// set player name to default player name
	LoadSetup();

	// update player name used by name typing functions
	strcpy(name, setup.player_name);

	setTypeNameValues_Name(name, pos);
      }
    }
    else if (remove_user && type_name_nr != 0)
    {
      if (Request("Remove settings and tapes for deleted player?", REQ_ASK))
      {
	char *user_dir = getUserGameDataDir();
	char *user_dir_removed =
	  getStringCat3WithSeparator(user_dir, "REMOVED",
				     getCurrentTimestamp(), ".");

	if (rename(user_dir, user_dir_removed) != 0)
	  Request("Removing settings and tapes failed!", REQ_CONFIRM);

	checked_free(user_dir_removed);
      }
    }
    else if (create_user && type_name_nr != 0 && !setup_exists)
    {
      if (Request("Create empty level set for the new player?", REQ_ASK))
      {
	char *levelset_subdir = getNewUserLevelSubdir();

	if (CreateUserLevelSet(levelset_subdir, name, name, 100, FALSE))
	{
	  AddUserLevelSetToLevelInfo(levelset_subdir);

	  LevelDirTree *leveldir_current_last = leveldir_current;

	  leveldir_current = getTreeInfoFromIdentifier(leveldir_first,
						       levelset_subdir);

	  // set level number of newly created level set to default value
	  LoadLevelSetup_SeriesInfo();

	  // set newly created level set as current level set for new user
	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();

	  leveldir_current = leveldir_current_last;
	}
	else
	{
	  Request("Creating new level set failed!", REQ_CONFIRM);
	}
      }
    }

    // restore currently active user
    user.nr = last_user_nr;

    // restore setup of currently active user
    LoadSetup();

    // restore last level set of currently active user
    LoadLevelSetup_LastSeries();
    LoadLevelSetup_SeriesInfo();
  }
}

static void HandleTypeNameExt(boolean initialize, Key key)
{
  static struct TextPosInfo pos_name = { 0 };
  static char name[MAX_PLAYER_NAME_LEN + 1] = { 0 };
  static int xpos = 0;
  struct TextPosInfo *pos = &pos_name;
  char key_char = getValidConfigValueChar(getCharFromKey(key));
  boolean is_valid_key_char = (key_char != 0 && (key_char != ' ' || xpos > 0));
  boolean active = TRUE;

  if (initialize)
  {
    getTypeNameValues(name, pos, &xpos);

    int sx = mSX + ALIGNED_TEXT_XPOS(pos);
    int sy = mSY + ALIGNED_TEXT_YPOS(pos);

    StartTextInput(sx, sy, pos->width, pos->height);
  }
  else if (is_valid_key_char && xpos < MAX_PLAYER_NAME_LEN)
  {
    name[xpos] = key_char;
    name[xpos + 1] = 0;

    xpos++;
  }
  else if ((key == KSYM_Delete || key == KSYM_BackSpace) && xpos > 0)
  {
    xpos--;

    name[xpos] = 0;
  }
  else if (key == KSYM_Return || key == KSYM_Escape)
  {
    boolean changed = (key == KSYM_Return);

    StopTextInput();

    setTypeNameValues(name, pos, changed);

    active = FALSE;
  }

  drawTypeNameText(name, pos, active);

  if (!active)
  {
    SetGameStatus(game_status_last_screen);

    if (game_status == GAME_MODE_MAIN)
      InitializeMainControls();
  }
}

static void DrawTypeName(void)
{
  HandleTypeNameExt(TRUE, 0);
}

void HandleTypeName(Key key)
{
  HandleTypeNameExt(FALSE, key);
}


// ============================================================================
// tree menu functions
// ============================================================================

static int getAlignXOffsetFromTreeInfo(TreeInfo *ti)
{
  if (game_status != GAME_MODE_SETUP ||
      DRAW_MODE_SETUP(setup_mode) != SETUP_MODE_CHOOSE_OTHER)
    return 0;

  int max_text_size = 0;
  TreeInfo *node;

  for (node = getTreeInfoFirstGroupEntry(ti); node != NULL; node = node->next)
    max_text_size = MAX(max_text_size, strlen(node->name));

  int num_entries = numTreeInfoInGroup(ti);
  boolean scrollbar_needed = (num_entries > NUM_MENU_ENTRIES_ON_SCREEN);
  int font_nr = MENU_CHOOSE_TREE_FONT(FC_RED);
  int text_width = max_text_size * getFontWidth(font_nr);
  int button_width = SC_MENUBUTTON_XSIZE;
  int scrollbar_xpos = SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset;
  int screen_width = (scrollbar_needed ? scrollbar_xpos : SXSIZE);
  int align = menu.list_setup[SETUP_MODE_CHOOSE_OTHER].align;
  int x = ALIGNED_XPOS(0, screen_width, align) * -1;
  int align_xoffset_raw = ALIGNED_XPOS(x, button_width + text_width, align);
  int align_xoffset = MAX(0, align_xoffset_raw);

  return align_xoffset;
}

static int getAlignYOffsetFromTreeInfo(TreeInfo *ti)
{
  if (game_status != GAME_MODE_SETUP ||
      DRAW_MODE_SETUP(setup_mode) != SETUP_MODE_CHOOSE_OTHER)
    return 0;

  int num_entries = numTreeInfoInGroup(ti);
  int num_page_entries = MIN(num_entries, NUM_MENU_ENTRIES_ON_SCREEN);
  int font_nr = MENU_CHOOSE_TREE_FONT(FC_RED);
  int font_height = getFontHeight(font_nr);
  int text_height = font_height * num_page_entries;
  int page_height = font_height * NUM_MENU_ENTRIES_ON_SCREEN;
  int align = menu.list_setup[SETUP_MODE_CHOOSE_OTHER].valign;
  int y = ALIGNED_YPOS(0, page_height, align) * -1;
  int align_yoffset_raw = ALIGNED_YPOS(y, text_height, align);
  int align_yoffset = MAX(0, align_yoffset_raw);

  return align_yoffset;
}

static void StartPlayingFromHallOfFame(void)
{
  level_nr = scores.next_level_nr;
  LoadLevel(level_nr);

  StartGameActions(network.enabled, setup.autorecord, level.random_seed);
}

static void DrawChooseTree(TreeInfo **ti_ptr)
{
  int fade_mask = REDRAW_FIELD;
  boolean restart_music = (game_status != game_status_last_screen &&
			   game_status_last_screen != GAME_MODE_SCOREINFO);

  scores.continue_on_return = (game_status == GAME_MODE_SCORES &&
			       game_status_last_screen == GAME_MODE_PLAYING);

  if (CheckFadeAll())
    fade_mask = REDRAW_ALL;

  if (*ti_ptr != NULL && strEqual((*ti_ptr)->subdir, STRING_TOP_DIRECTORY))
  {
    if (game_status == GAME_MODE_SETUP)
    {
      execSetupArtwork();
    }
    else if (game_status == GAME_MODE_SCORES && scores.continue_playing)
    {
      StartPlayingFromHallOfFame();
    }
    else
    {
      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }

    return;
  }

  UnmapAllGadgets();

  if (restart_music)
    FadeMenuSoundsAndMusic();

  FadeOut(fade_mask);

  // needed if different viewport properties defined for this screen
  ChangeViewportPropertiesIfNeeded();

  if (game_status == GAME_MODE_NAMES)
    SetMainBackgroundImage(IMG_BACKGROUND_NAMES);
  else if (game_status == GAME_MODE_LEVELNR)
    SetMainBackgroundImage(IMG_BACKGROUND_LEVELNR);
  else if (game_status == GAME_MODE_LEVELS)
    SetMainBackgroundImage(IMG_BACKGROUND_LEVELS);
  else if (game_status == GAME_MODE_SCORES)
    SetMainBackgroundImage(IMG_BACKGROUND_SCORES);

  FreeScreenGadgets();
  CreateScreenGadgets();

  ClearField();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  // map gadgets for high score screen
  if (game_status == GAME_MODE_SCORES)
    MapScreenMenuGadgets(SCREEN_MASK_SCORES);

  MapScreenTreeGadgets(*ti_ptr);

  HandleChooseTree(0, 0, 0, 0, MB_MENU_INITIALIZE, ti_ptr);

  DrawMaskedBorder(fade_mask);

  if (restart_music)
    PlayMenuSoundsAndMusic();

  FadeIn(fade_mask);
}

static int getChooseTreeFont(TreeInfo *node, boolean active)
{
  if (game_status == GAME_MODE_SCORES)
    return (active ? FONT_TEXT_1_ACTIVE : FONT_TEXT_1);
  else
    return MENU_CHOOSE_TREE_FONT(MENU_CHOOSE_TREE_COLOR(node, active));
}

static void drawChooseTreeText(TreeInfo *ti, int y, boolean active)
{
  int num_entries = numTreeInfoInGroup(ti);
  boolean scrollbar_needed = (num_entries > NUM_MENU_ENTRIES_ON_SCREEN);
  int scrollbar_xpos = SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset;
  int screen_width = (scrollbar_needed ? scrollbar_xpos : SXSIZE);
  int first_entry = ti->cl_first;
  int entry_pos = first_entry + y;
  TreeInfo *node_first = getTreeInfoFirstGroupEntry(ti);
  TreeInfo *node = getTreeInfoFromPos(node_first, entry_pos);
  int font_nr = getChooseTreeFont(node, active);
  int font_xoffset = getFontDrawOffsetX(font_nr);
  int xpos = MENU_SCREEN_START_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + y;
  int startdx = xpos * 32;
  int startdy = ypos * 32;
  int startx = amSX + startdx;
  int starty = amSY + startdy;
  int startx_text = startx + font_xoffset;
  int endx_text = amSX + screen_width;
  int max_text_size = endx_text - startx_text;
  int max_buffer_len = max_text_size / getFontWidth(font_nr);
  char buffer[max_buffer_len + 1];

  if (game_status == GAME_MODE_SCORES && !node->parent_link)
  {
    int font_nr1 = (active ? FONT_TEXT_1_ACTIVE : FONT_TEXT_1);
    int font_nr2 = (active ? FONT_TEXT_2_ACTIVE : FONT_TEXT_2);
    int font_nr3 = (active ? FONT_TEXT_3_ACTIVE : FONT_TEXT_3);
    int font_nr4 = (active ? FONT_TEXT_4_ACTIVE : FONT_TEXT_4);
    int font_size_1 = getFontWidth(font_nr1);
    int font_size_3 = getFontWidth(font_nr3);
    int font_size_4 = getFontWidth(font_nr4);
    int text_size_1 = 4 * font_size_1;
    int text_size_4 = 5 * font_size_4;
    int border = amSX - SX + getFontDrawOffsetX(font_nr1);
    int dx1 = 0;
    int dx3 = text_size_1;
    int dx4 = SXSIZE - 2 * startdx - 2 * border - text_size_4;
    int num_dots = (dx4 - dx3) / font_size_3;
    int startx1 = startx + dx1;
    int startx3 = startx + dx3;
    int startx4 = startx + dx4;
    int pos = node->pos;
    char *pos_text = getHallOfFameRankText(pos, 3);
    int i;

    // highlight all high score entries of the current player
    if (strEqual(scores.entry[pos].name, setup.player_name))
      font_nr2 = FONT_TEXT_2_ACTIVE;

    DrawText(startx1, starty, pos_text, font_nr1);

    for (i = 0; i < num_dots; i++)
      DrawText(startx3 + i * font_size_3, starty, ".", font_nr3);

    if (!strEqual(scores.entry[pos].name, EMPTY_PLAYER_NAME))
      DrawText(startx3, starty, scores.entry[pos].name, font_nr2);

    DrawText(startx4, starty, getHallOfFameScoreText(pos, 5), font_nr4);
  }
  else
  {
    strncpy(buffer, node->name, max_buffer_len);
    buffer[max_buffer_len] = '\0';

    DrawText(startx, starty, buffer, font_nr);
  }
}

static void drawChooseTreeHeadExt(int type, char *title_string)
{
  int y = (type == TREE_TYPE_SCORE_ENTRY ||
	   type == TREE_TYPE_LEVEL_DIR ||
	   type == TREE_TYPE_LEVEL_NR ? MENU_TITLE1_YPOS : MENU_TITLE_YPOS);

  DrawTextSCentered(y, FONT_TITLE_1, title_string);
}

static void drawChooseTreeHead(TreeInfo *ti)
{
  drawChooseTreeHeadExt(ti->type, ti->infotext);
}

static void drawChooseTreeList(TreeInfo *ti)
{
  int first_entry = ti->cl_first;
  int num_entries = numTreeInfoInGroup(ti);
  int num_page_entries = MIN(num_entries, NUM_MENU_ENTRIES_ON_SCREEN);
  int i;

  clearMenuListArea();

  for (i = 0; i < num_page_entries; i++)
  {
    TreeInfo *node, *node_first;
    int entry_pos = first_entry + i;

    node_first = getTreeInfoFirstGroupEntry(ti);
    node = getTreeInfoFromPos(node_first, entry_pos);

    drawChooseTreeText(ti, i, FALSE);

    if (node->parent_link)
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (node->level_group)
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else
      initCursor(i, IMG_MENU_BUTTON);

    if (game_status == GAME_MODE_SCORES && node->pos == scores.last_added)
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);

    if (game_status == GAME_MODE_NAMES)
      drawChooseTreeEdit(i, FALSE);
  }

  redraw_mask |= REDRAW_FIELD;
}

static void drawChooseTreeInfo(TreeInfo *ti)
{
  int entry_pos = ti->cl_first + ti->cl_cursor;
  int last_redraw_mask = redraw_mask;
  int ypos = MENU_TITLE2_YPOS;
  int font_nr = FONT_TITLE_2;
  int x;

  if (ti->type == TREE_TYPE_LEVEL_NR)
    DrawTextFCentered(ypos, font_nr, leveldir_current->name);

  if (ti->type == TREE_TYPE_SCORE_ENTRY)
    DrawTextFCentered(ypos, font_nr, "HighScores of Level %d",
		      scores.last_level_nr);

  if (ti->type != TREE_TYPE_LEVEL_DIR)
    return;

  TreeInfo *node_first = getTreeInfoFirstGroupEntry(ti);
  TreeInfo *node = getTreeInfoFromPos(node_first, entry_pos);

  DrawBackgroundForFont(SX, SY + ypos, SXSIZE, getFontHeight(font_nr), font_nr);

  if (node->parent_link)
    DrawTextFCentered(ypos, font_nr, "leave \"%s\"",
		      node->node_parent->name);
  else if (node->level_group)
    DrawTextFCentered(ypos, font_nr, "enter \"%s\"",
		      node->name);
  else if (ti->type == TREE_TYPE_LEVEL_DIR)
    DrawTextFCentered(ypos, font_nr, "%3d %s (%s)",
		      node->levels, (node->levels > 1 ? "levels" : "level"),
		      node->class_desc);

  // let BackToFront() redraw only what is needed
  redraw_mask = last_redraw_mask;
  for (x = 0; x < SCR_FIELDX; x++)
    MarkTileDirty(x, 1);
}

static void drawChooseTreeCursorAndText(TreeInfo *ti, boolean active)
{
  drawChooseTreeCursor(ti->cl_cursor, active);
  drawChooseTreeText(ti, ti->cl_cursor, active);
}

static void drawChooseTreeScreen(TreeInfo *ti)
{
  drawChooseTreeHead(ti);
  drawChooseTreeList(ti);
  drawChooseTreeInfo(ti);
  drawChooseTreeCursorAndText(ti, TRUE);

  AdjustChooseTreeScrollbar(ti, SCREEN_CTRL_ID_SCROLL_VERTICAL);

  // scroll bar and buttons may just have been added after reloading scores
  if (game_status == GAME_MODE_SCORES)
    MapScreenTreeGadgets(ti);
}

static TreeInfo *setHallOfFameActiveEntry(TreeInfo **ti_ptr)
{
  int score_pos = scores.last_added;

  if (game_status_last_screen == GAME_MODE_SCOREINFO)
    score_pos = scores.last_entry_nr;

  // set current tree entry to last added score entry
  *ti_ptr = getTreeInfoFromIdentifier(score_entries, i_to_a(score_pos));

  // if that fails, set current tree entry to first entry (back link)
  if (*ti_ptr == NULL)
    *ti_ptr = score_entries->node_group;

  int num_entries = numTreeInfoInGroup(*ti_ptr);
  int num_page_entries = MIN(num_entries, NUM_MENU_ENTRIES_ON_SCREEN);
  int pos_score = getPosFromTreeInfo(*ti_ptr);
  int pos_first_raw = pos_score - (num_page_entries + 1) / 2 + 1;
  int pos_first = MIN(MAX(0, pos_first_raw), num_entries - num_page_entries);

  (*ti_ptr)->cl_first = pos_first;
  (*ti_ptr)->cl_cursor = pos_score - pos_first;

  return *ti_ptr;
}

static void HandleChooseTree(int mx, int my, int dx, int dy, int button,
			     TreeInfo **ti_ptr)
{
  TreeInfo *ti = *ti_ptr;
  boolean has_scrollbar = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->mapped;
  int mx_scrollbar = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->x;
  int mx_right_border = (has_scrollbar ? mx_scrollbar : SX + SXSIZE);
  int sx1_edit_name = getChooseTreeEditXPosReal(POS_LEFT);
  int sx2_edit_name = getChooseTreeEditXPosReal(POS_RIGHT);
  int x = 0;
  int y = (ti != NULL ? ti->cl_cursor : 0);
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int num_entries = numTreeInfoInGroup(ti);
  int num_page_entries = MIN(num_entries, NUM_MENU_ENTRIES_ON_SCREEN);
  boolean position_set_by_scrollbar = (dx == 999);

  if (game_status == GAME_MODE_SCORES)
  {
    if (server_scores.updated)
    {
      // reload scores, using updated server score cache file
      LoadLocalAndServerScore(scores.last_level_nr, FALSE);

      server_scores.updated = FALSE;

      DrawHallOfFame_setScoreEntries();

      ti = setHallOfFameActiveEntry(ti_ptr);

      if (button != MB_MENU_INITIALIZE)
	drawChooseTreeScreen(ti);
    }
  }

  if (button == MB_MENU_INITIALIZE)
  {
    int num_entries = numTreeInfoInGroup(ti);
    int entry_pos = getPosFromTreeInfo(ti);

    align_xoffset = getAlignXOffsetFromTreeInfo(ti);
    align_yoffset = getAlignYOffsetFromTreeInfo(ti);

    if (game_status == GAME_MODE_SCORES)
    {
      ti = setHallOfFameActiveEntry(ti_ptr);
    }
    else if (ti->cl_first == -1)
    {
      // only on initialization
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor = entry_pos - ti->cl_first;

    }
    else if (ti->cl_cursor >= num_page_entries ||
	     (num_entries > num_page_entries &&
	      num_entries - ti->cl_first < num_page_entries))
    {
      // only after change of list size (by custom graphic configuration)
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor = entry_pos - ti->cl_first;
    }

    if (position_set_by_scrollbar)
      ti->cl_first = dy;

    drawChooseTreeScreen(ti);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    if (game_status != GAME_MODE_SCORES)
      FadeSetLeaveMenu();

    PlaySound(SND_MENU_ITEM_SELECTING);

    if (ti->node_parent)
    {
      *ti_ptr = ti->node_parent;
      DrawChooseTree(ti_ptr);
    }
    else if (game_status == GAME_MODE_SETUP)
    {
      if (setup_mode == SETUP_MODE_CHOOSE_SCORES_TYPE ||
	  setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED ||
	  setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY ||
	  setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
	execSetupGame();
      else if (setup_mode == SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE ||
	       setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64 ||
	       setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV ||
	       setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_ATARI ||
	       setup_mode == SETUP_MODE_CHOOSE_BD_COLOR_TYPE)
	execSetupEngines();
      else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE ||
	       setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE ||
	       setup_mode == SETUP_MODE_CHOOSE_RENDERING ||
	       setup_mode == SETUP_MODE_CHOOSE_VSYNC)
	execSetupGraphics();
      else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE ||
	       setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS ||
	       setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
	execSetupSound();
      else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL ||
	       setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE ||
	       setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE ||
	       setup_mode == SETUP_MODE_CHOOSE_TRANSPARENCY ||
	       setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_0 ||
	       setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_0 ||
	       setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_1 ||
	       setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_1)
	execSetupTouch();
      else
	execSetupArtwork();
    }
    else
    {
      if (game_status == GAME_MODE_LEVELNR)
      {
	int new_level_nr = atoi(level_number_current->identifier);

	HandleMainMenu_SelectLevel(0, 0, new_level_nr);
      }

      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }

    return;
  }

#if defined(PLATFORM_ANDROID)
  // directly continue when touching the screen after playing
  if ((mx || my) && scores.continue_on_return)
  {
    // ignore touch events until released
    mx = my = 0;
  }
#endif

  // any mouse click or cursor key stops leaving scores by "Return" key
  if ((mx || my || dx || dy) && scores.continue_on_return)
  {
    scores.continue_on_return = FALSE;
    level_nr = scores.last_level_nr;
    LoadLevel(level_nr);
  }

  if (mx || my)		// mouse input
  {
    x = (mx - amSX) / 32;
    y = (my - amSY) / 32 - MENU_SCREEN_START_YPOS;

    if (game_status == GAME_MODE_NAMES)
      drawChooseTreeEdit(ti->cl_cursor, FALSE);
  }
  else if (dx || dy)	// keyboard or scrollbar/scrollbutton input
  {
    // move cursor instead of scrolling when already at start/end of list
    if (dy == -1 * SCROLL_LINE && ti->cl_first == 0)
      dy = -1;
    else if (dy == +1 * SCROLL_LINE &&
	     ti->cl_first + num_page_entries == num_entries)
      dy = 1;

    // handle scrolling screen one line or page
    if (ti->cl_cursor + dy < 0 ||
	ti->cl_cursor + dy > num_page_entries - 1)
    {
      boolean redraw = FALSE;

      if (ABS(dy) == SCROLL_PAGE)
	step = num_page_entries - 1;

      if (dy < 0 && ti->cl_first > 0)
      {
	// scroll page/line up

	ti->cl_first -= step;
	if (ti->cl_first < 0)
	  ti->cl_first = 0;

	redraw = TRUE;
      }
      else if (dy > 0 && ti->cl_first + num_page_entries < num_entries)
      {
	// scroll page/line down

	ti->cl_first += step;
	if (ti->cl_first + num_page_entries > num_entries)
	  ti->cl_first = MAX(0, num_entries - num_page_entries);

	redraw = TRUE;
      }

      if (redraw)
	drawChooseTreeScreen(ti);

      return;
    }

    // handle moving cursor one line
    y = ti->cl_cursor + dy;
  }

  if (game_status == GAME_MODE_SCORES && ABS(dx) == 1)
  {
    HandleHallOfFame_SelectLevel(1, dx);

    return;
  }
  else if (game_status == GAME_MODE_NAMES && dx == 1)
  {
    SetGameStatus(GAME_MODE_PSEUDO_TYPENAMES);

    DrawTypeName();

    return;
  }
  else if (dx == 1)
  {
    TreeInfo *node_first, *node_cursor;
    int entry_pos = ti->cl_first + y;

    node_first = getTreeInfoFirstGroupEntry(ti);
    node_cursor = getTreeInfoFromPos(node_first, entry_pos);

    if (node_cursor->node_group)
    {
      FadeSetEnterMenu();

      PlaySound(SND_MENU_ITEM_SELECTING);

      node_cursor->cl_first = ti->cl_first;
      node_cursor->cl_cursor = ti->cl_cursor;

      *ti_ptr = node_cursor->node_group;
      DrawChooseTree(ti_ptr);

      return;
    }
  }
  else if ((dx == -1 || button == MB_MENU_CONTINUE) && ti->node_parent)
  {
    if (game_status != GAME_MODE_SCORES)
      FadeSetLeaveMenu();

    PlaySound(SND_MENU_ITEM_SELECTING);

    *ti_ptr = ti->node_parent;
    DrawChooseTree(ti_ptr);

    return;
  }

  if (!anyScrollbarGadgetActive() &&
      IN_VIS_MENU(x, y) &&
      mx < mx_right_border &&
      y >= 0 && y < num_page_entries)
  {
    if (button)
    {
      if (game_status == GAME_MODE_NAMES)
      {
	if (mx >= sx1_edit_name && mx <= sx2_edit_name)
	  drawChooseTreeEdit(y, TRUE);
      }

      if (y != ti->cl_cursor)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	drawChooseTreeCursorAndText(ti, FALSE);

	ti->cl_cursor = y;

	drawChooseTreeCursorAndText(ti, TRUE);

	drawChooseTreeInfo(ti);
      }
      else if (dx < 0)
      {
	if (game_status == GAME_MODE_SETUP)
	{
	  if (setup_mode == SETUP_MODE_CHOOSE_SCORES_TYPE ||
	      setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED ||
	      setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY ||
	      setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
	    execSetupGame();
	  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64 ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_ATARI ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_COLOR_TYPE)
	    execSetupEngines();
	  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE ||
		   setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE ||
		   setup_mode == SETUP_MODE_CHOOSE_RENDERING ||
		   setup_mode == SETUP_MODE_CHOOSE_VSYNC)
	    execSetupGraphics();
	  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
	    execSetupSound();
	  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL ||
		   setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE ||
		   setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE ||
		   setup_mode == SETUP_MODE_CHOOSE_TRANSPARENCY ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_0 ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_0 ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_1 ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_1)
	    execSetupTouch();
	  else
	    execSetupArtwork();
	}
      }
    }
    else
    {
      TreeInfo *node_first, *node_cursor;
      int entry_pos = ti->cl_first + y;

      PlaySound(SND_MENU_ITEM_SELECTING);

      node_first = getTreeInfoFirstGroupEntry(ti);
      node_cursor = getTreeInfoFromPos(node_first, entry_pos);

      if (node_cursor->node_group)
      {
	FadeSetEnterMenu();

	node_cursor->cl_first = ti->cl_first;
	node_cursor->cl_cursor = ti->cl_cursor;

	*ti_ptr = node_cursor->node_group;
	DrawChooseTree(ti_ptr);
      }
      else if (node_cursor->parent_link)
      {
	if (game_status != GAME_MODE_SCORES)
	  FadeSetLeaveMenu();

	*ti_ptr = node_cursor->node_parent;
	DrawChooseTree(ti_ptr);
      }
      else
      {
	if (game_status != GAME_MODE_SCORES)
	  FadeSetEnterMenu();

	node_cursor->cl_first = ti->cl_first;
	node_cursor->cl_cursor = ti->cl_cursor;

	*ti_ptr = node_cursor;

	if (ti->type == TREE_TYPE_LEVEL_DIR)
	{
	  LoadLevelSetup_SeriesInfo();

	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();
	  TapeErase();
	}

	if (game_status == GAME_MODE_SETUP)
	{
	  if (setup_mode == SETUP_MODE_CHOOSE_SCORES_TYPE ||
	      setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED ||
	      setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY ||
	      setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
	    execSetupGame();
	  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64 ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_ATARI ||
		   setup_mode == SETUP_MODE_CHOOSE_BD_COLOR_TYPE)
	    execSetupEngines();
	  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE ||
		   setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE ||
		   setup_mode == SETUP_MODE_CHOOSE_RENDERING ||
		   setup_mode == SETUP_MODE_CHOOSE_VSYNC)
	    execSetupGraphics();
	  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
	    execSetupSound();
	  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL ||
		   setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE ||
		   setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE ||
		   setup_mode == SETUP_MODE_CHOOSE_TRANSPARENCY ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_0 ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_0 ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_1 ||
		   setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_1)
	    execSetupTouch();
	  else
	    execSetupArtwork();

          SaveSetupIfNeeded();
	}
	else
	{
	  if (game_status == GAME_MODE_LEVELNR)
	  {
	    int new_level_nr = atoi(level_number_current->identifier);

	    HandleMainMenu_SelectLevel(0, 0, new_level_nr);
	  }
	  else if (game_status == GAME_MODE_LEVELS)
	  {
	    // store level set if chosen from "last played level set" menu
	    StoreLastPlayedLevels(leveldir_current);

	    // store if level set chosen from "last played level set" menu
	    SaveLevelSetup_LastSeries();
	  }
	  else if (game_status == GAME_MODE_NAMES)
	  {
	    if (mx >= sx1_edit_name && mx <= sx2_edit_name)
	    {
	      SetGameStatus(GAME_MODE_PSEUDO_TYPENAMES);

	      DrawTypeName();

	      return;
	    }

	    // change active user to selected user
	    user.nr = entry_pos;

	    // save number of new active user
	    SaveUserSetup();

	    // load setup of new active user
	    LoadSetup();

	    // load last level set of new active user
	    LoadLevelSetup_LastSeries();
	    LoadLevelSetup_SeriesInfo();

	    // update list of last played level sets
	    UpdateLastPlayedLevels_TreeInfo();

	    TapeErase();

	    ToggleFullscreenIfNeeded();
	    ChangeWindowScalingIfNeeded();

	    ChangeCurrentArtworkIfNeeded(ARTWORK_TYPE_GRAPHICS);
	    ChangeCurrentArtworkIfNeeded(ARTWORK_TYPE_SOUNDS);
	    ChangeCurrentArtworkIfNeeded(ARTWORK_TYPE_MUSIC);
	  }
	  else if (game_status == GAME_MODE_SCORES)
	  {
	    if (scores.continue_playing && scores.continue_on_return)
	    {
	      StartPlayingFromHallOfFame();

	      return;
	    }
	    else if (!scores.continue_on_return)
	    {
	      SetGameStatus(GAME_MODE_SCOREINFO);

	      DrawScoreInfo(node_cursor->pos);

	      return;
	    }
	  }

	  SetGameStatus(GAME_MODE_MAIN);

	  DrawMainMenu();
	}
      }
    }
  }

  if (game_status == GAME_MODE_SCORES)
    PlayMenuSoundIfLoop();
}

void DrawChoosePlayerName(void)
{
  int i;

  if (player_name != NULL)
  {
    freeTreeInfo(player_name);

    player_name = NULL;
  }

  for (i = 0; i < MAX_PLAYER_NAMES; i++)
  {
    TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_PLAYER_NAME);
    char identifier[32], name[MAX_PLAYER_NAME_LEN + 1];
    int value = i;

    ti->node_top = &player_name;
    ti->sort_priority = 10000 + value;
    ti->color = getPlayerNameColor(global.user_names[i]);

    snprintf(identifier, sizeof(identifier), "%d", value);
    snprintf(name, sizeof(name), "%s", global.user_names[i]);

    setString(&ti->identifier, identifier);
    setString(&ti->name, name);
    setString(&ti->name_sorting, name);

    pushTreeInfo(&player_name, ti);
  }

  // sort player entries by player number
  sortTreeInfo(&player_name);

  // set current player entry to selected player entry
  player_name_current =
    getTreeInfoFromIdentifier(player_name, i_to_a(user.nr));

  // if that fails, set current player name to first available name
  if (player_name_current == NULL)
    player_name_current = player_name;

  // set text size for main name input (also used on name selection screen)
  InitializeMainControls();

  DrawChooseTree(&player_name_current);
}

void HandleChoosePlayerName(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &player_name_current);
}

void DrawChooseLevelSet(void)
{
  DrawChooseTree(&leveldir_current);
}

void HandleChooseLevelSet(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &leveldir_current);
}

void DrawChooseLevelNr(void)
{
  int i;

  if (level_number != NULL)
  {
    freeTreeInfo(level_number);

    level_number = NULL;
  }

  for (i = leveldir_current->first_level; i <= leveldir_current->last_level;i++)
  {
    TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_LEVEL_NR);
    char identifier[32], name[64];
    int value = i;

    // temporarily load level info to get level name
    LoadLevelInfoOnly(i);

    ti->node_top = &level_number;
    ti->sort_priority = 10000 + value;
    ti->color = (level.no_level_file ? FC_BLUE :
		 LevelStats_getSolved(i) ? FC_GREEN :
		 LevelStats_getPlayed(i) ? FC_YELLOW : FC_RED);

    snprintf(identifier, sizeof(identifier), "%d", value);
    snprintf(name, sizeof(name), "%03d: %s", value,
	     (level.no_level_file ? "(no file)" : level.name));

    setString(&ti->identifier, identifier);
    setString(&ti->name, name);
    setString(&ti->name_sorting, name);

    pushTreeInfo(&level_number, ti);
  }

  // sort level number values to start with lowest level number
  sortTreeInfo(&level_number);

  // set current level number to current level number
  level_number_current =
    getTreeInfoFromIdentifier(level_number, i_to_a(level_nr));

  // if that also fails, set current level number to first available level
  if (level_number_current == NULL)
    level_number_current = level_number;

  DrawChooseTree(&level_number_current);
}

void HandleChooseLevelNr(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &level_number_current);
}

static void DrawHallOfFame_setScoreEntries(void)
{
  int max_empty_entries = 10;	// at least show "top ten" list, if empty
  int max_visible_entries = NUM_MENU_ENTRIES_ON_SCREEN - 1;   // w/o back link
  int min_score_entries = MIN(max_empty_entries, max_visible_entries);
  int score_pos = (scores.last_added >= 0 ? scores.last_added : 0);
  int i;

  if (score_entries != NULL)
  {
    freeTreeInfo(score_entries);

    score_entries = NULL;
  }

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    // do not add empty score entries if off-screen
    if (scores.entry[i].score == 0 &&
	scores.entry[i].time == 0 &&
	i >= min_score_entries)
      break;

    TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_SCORE_ENTRY);
    char identifier[32], name[64];
    int value = i;

    ti->node_top = &score_entries;
    ti->sort_priority = 10000 + value;
    ti->color = FC_YELLOW;
    ti->pos = i;

    snprintf(identifier, sizeof(identifier), "%d", value);
    snprintf(name, sizeof(name), "%03d.", value + 1);

    setString(&ti->identifier, identifier);
    setString(&ti->name, name);
    setString(&ti->name_sorting, name);

    pushTreeInfo(&score_entries, ti);
  }

  // sort score entries to start with highest score entry
  sortTreeInfo(&score_entries);

  // add top tree node to create back link to main menu
  score_entries = addTopTreeInfoNode(score_entries);

  // set current score entry to last added or highest score entry
  score_entry_current =
    getTreeInfoFromIdentifier(score_entries, i_to_a(score_pos));

  // if that fails, set current score entry to first valid score entry
  if (score_entry_current == NULL)
    score_entry_current = getFirstValidTreeInfoEntry(score_entries);

  if (score_entries != NULL && scores.continue_playing)
    setString(&score_entries->node_group->name, BACKLINK_TEXT_NEXT);
}

void DrawHallOfFame(int nr)
{
  scores.last_level_nr = nr;

  // (this is needed when called from GameEnd() after winning a game)
  KeyboardAutoRepeatOn();

  // (this is needed when called from GameEnd() after winning a game)
  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  LoadLocalAndServerScore(scores.last_level_nr, TRUE);

  DrawHallOfFame_setScoreEntries();

  if (scores.last_added >= 0)
    SetAnimStatus(GAME_MODE_PSEUDO_SCORESNEW);

  FadeSetEnterScreen();

  DrawChooseTree(&score_entry_current);
}

static char *getHallOfFameRankText(int nr, int size)
{
  static char rank_text[10];
  boolean forced = (scores.force_last_added && nr == scores.last_added);
  char *rank_text_raw = (forced ? "???" : int2str(nr + 1, size));

  sprintf(rank_text, "%s%s", rank_text_raw, (size > 0 || !forced ? "." : ""));

  return rank_text;
}

static char *getHallOfFameTimeText(int nr)
{
  static char score_text[10];
  int time_seconds = scores.entry[nr].time / FRAMES_PER_SECOND;
  int mm = (time_seconds / 60) % 60;
  int ss = (time_seconds % 60);

  sprintf(score_text, "%02d:%02d", mm, ss);	// show playing time

  return score_text;
}

static char *getHallOfFameScoreText(int nr, int size)
{
  if (!level.rate_time_over_score)
    return int2str(scores.entry[nr].score, size);	// show normal score
  else if (level.use_step_counter)
    return int2str(scores.entry[nr].time, size);	// show number of steps
  else
    return getHallOfFameTimeText(nr);			// show playing time
}

static char *getHallOfFameTapeDateText(struct ScoreEntry *entry)
{
  static char tape_date[MAX_ISO_DATE_LEN + 1];
  int i, j;

  if (!strEqual(entry->tape_date, UNKNOWN_NAME) ||
      strEqual(entry->tape_basename, UNDEFINED_FILENAME))
    return entry->tape_date;

  for (i = 0, j = 0; i < 8; i++, j++)
  {
    tape_date[j] = entry->tape_basename[i];

    if (i == 3 || i == 5)
      tape_date[++j] = '-';
  }

  tape_date[MAX_ISO_DATE_LEN] = '\0';

  return tape_date;
}

static void HandleHallOfFame_SelectLevel(int step, int direction)
{
  int old_level_nr = scores.last_level_nr;
  int new_level_nr = old_level_nr + step * direction;

  if (new_level_nr < leveldir_current->first_level)
    new_level_nr = leveldir_current->first_level;
  if (new_level_nr > leveldir_current->last_level)
    new_level_nr = leveldir_current->last_level;

  if (setup.allow_skipping_levels != STATE_TRUE && new_level_nr > leveldir_current->handicap_level)
    new_level_nr = leveldir_current->handicap_level;

  if (new_level_nr != old_level_nr)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    scores.last_level_nr = level_nr = new_level_nr;
    scores.last_entry_nr = 0;

    LoadLevel(level_nr);
    LoadLocalAndServerScore(level_nr, TRUE);

    DrawHallOfFame_setScoreEntries();

    if (game_status == GAME_MODE_SCORES)
    {
      // force remapping optional gadgets (especially scroll bar)
      UnmapScreenTreeGadgets();

      // redraw complete high score screen, as sub-title has changed
      ClearField();

      // redraw level selection buttons (which have just been erased)
      RedrawScreenMenuGadgets(SCREEN_MASK_SCORES);

      HandleChooseTree(0, 0, 0, 0, MB_MENU_INITIALIZE, &score_entry_current);
    }
    else
    {
      DrawScoreInfo_Content(scores.last_entry_nr);
    }

    SaveLevelSetup_SeriesInfo();
  }
}

void HandleHallOfFame(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &score_entry_current);
}

static void DrawScoreInfo_Content(int entry_nr)
{
  struct ScoreEntry *entry = &scores.entry[entry_nr];
  char *pos_text = getHallOfFameRankText(entry_nr, 0);
  char *tape_date = getHallOfFameTapeDateText(entry);
  int max_text_1_len = strlen("Level Set");
  int font_head = MENU_INFO_FONT_HEAD;
  int font_text = MENU_INFO_FONT_TEXT;
  int font_foot = MENU_INFO_FONT_FOOT;
  int spacing_para = menu.paragraph_spacing[GAME_MODE_SCOREINFO];
  int spacing_line = menu.line_spacing[GAME_MODE_SCOREINFO];
  int spacing_left = menu.left_spacing[GAME_MODE_SCOREINFO];
  int spacing_top  = menu.top_spacing[GAME_MODE_SCOREINFO];
  int spacing_midd = menu.middle_spacing[GAME_MODE_SCOREINFO];
  int ystep_para = getMenuTextStep(spacing_para, font_text);
  int ystep_line = getMenuTextStep(spacing_line, font_text);
  int ystart  = mSY - SY + spacing_top + getHeadlineSpacing();
  int yfooter = MENU_FOOTER_YPOS;
  int xstart1 = mSX - SX + spacing_left;
  int xstart2 = xstart1 + max_text_1_len * getFontWidth(font_text) + spacing_midd;
  int select_x = SX + xstart1;
  int select_y1, select_y2;
  int play_x, play_y;
  int play_height = screen_gadget[SCREEN_CTRL_ID_PLAY_TAPE]->height;
  boolean play_visible = !strEqual(tape_date, UNKNOWN_NAME);
  int font_width = getFontWidth(font_text);
  int font_height = getFontHeight(font_text);
  int tape_date_width = getTextWidth(tape_date, font_text);
  int pad_left = xstart2;
  int pad_right = menu.right_spacing[GAME_MODE_SCOREINFO];
  int max_chars_per_line = (SXSIZE - pad_left - pad_right) / font_width;
  int max_lines_per_text = 5;
  int lines;

  ClearField();

  // redraw level selection buttons (which have just been erased)
  RedrawScreenMenuGadgets(SCREEN_MASK_SCORES);

  drawChooseTreeHead(score_entries);
  drawChooseTreeInfo(score_entries);

  DrawTextF(xstart1, ystart, font_head, "Level Set");
  lines = DrawTextBufferS(xstart2, ystart, leveldir_current->name, font_text,
			  max_chars_per_line, -1, max_lines_per_text, -1, -1, -1, 0, -1,
			  TRUE, FALSE, FALSE);
  ystart += ystep_line + (lines > 0 ? lines - 1 : 0) * font_height;

  DrawTextF(xstart1, ystart, font_head, "Level");
  lines = DrawTextBufferS(xstart2, ystart, level.name, font_text,
			  max_chars_per_line, -1, max_lines_per_text, -1, -1, -1, 0, -1,
			  TRUE, FALSE, FALSE);
  ystart += ystep_para + (lines > 0 ? lines - 1 : 0) * font_height;

  select_y1 = SY + ystart;
  ystart += graphic_info[IMG_MENU_BUTTON_PREV_SCORE].height;

  DrawTextF(xstart1, ystart, font_head, "Rank");
  DrawTextF(xstart2, ystart, font_text, pos_text);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Player");
  DrawTextF(xstart2, ystart, font_text, entry->name);
  ystart += ystep_line;

  if (level.use_step_counter)
  {
    DrawTextF(xstart1, ystart, font_head, "Steps");
    DrawTextF(xstart2, ystart, font_text, int2str(entry->time, 5));
    ystart += ystep_line;
  }
  else
  {
    DrawTextF(xstart1, ystart, font_head, "Time");
    DrawTextF(xstart2, ystart, font_text, getHallOfFameTimeText(entry_nr));
    ystart += ystep_line;
  }

  if (!level.rate_time_over_score || entry->score > 0)
  {
    DrawTextF(xstart1, ystart, font_head, "Score");
    DrawTextF(xstart2, ystart, font_text, int2str(entry->score, 5));
    ystart += ystep_line;
  }

  ystart += ystep_line;

  play_x = SX + xstart2 + tape_date_width + font_width;
  play_y = SY + ystart + (font_height - play_height) / 2;

  DrawTextF(xstart1, ystart, font_head, "Tape Date");
  DrawTextF(xstart2, ystart, font_text, tape_date);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Platform");
  DrawTextF(xstart2, ystart, font_text, entry->platform);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Version");
  DrawTextF(xstart2, ystart, font_text, entry->version);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Country");
  lines = DrawTextBufferS(xstart2, ystart, entry->country_name, font_text,
			  max_chars_per_line, -1, max_lines_per_text, -1, -1, -1, 0, -1,
			  TRUE, FALSE, FALSE);
  ystart += ystep_line;

  select_y2 = SY + ystart;

  DrawTextSCentered(yfooter, font_foot, "Press any key or button to go back");

  AdjustScoreInfoButtons_SelectScore(select_x, select_y1, select_y2);
  AdjustScoreInfoButtons_PlayTape(play_x, play_y, play_visible);
}

static void DrawScoreInfo(int entry_nr)
{
  scores.last_entry_nr = entry_nr;
  score_info_tape_play = FALSE;

  UnmapAllGadgets();

  FreeScreenGadgets();
  CreateScreenGadgets();

  FadeOut(REDRAW_FIELD);

  // needed if different viewport properties defined after playing score tape
  ChangeViewportPropertiesIfNeeded();

  // set this after "ChangeViewportPropertiesIfNeeded()" (which may reset it)
  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  // needed if different background image defined after playing score tape
  SetMainBackgroundImage(IMG_BACKGROUND_SCORES);
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_SCOREINFO);

  // special compatibility handling for "Snake Bite" graphics set
  if (strPrefix(leveldir_current->identifier, "snake_bite"))
    ClearRectangle(gfx.background_bitmap, gfx.real_sx, gfx.real_sy + 64,
		   gfx.full_sxsize, gfx.full_sysize - 64);

  DrawScoreInfo_Content(entry_nr);

  // map gadgets for score info screen
  MapScreenMenuGadgets(SCREEN_MASK_SCORES_INFO);

  FadeIn(REDRAW_FIELD);
}

static void HandleScoreInfo_SelectScore(int step, int direction)
{
  int old_entry_nr = scores.last_entry_nr;
  int new_entry_nr = old_entry_nr + step * direction;
  int num_nodes = numTreeInfoInGroup(score_entry_current);
  int num_entries = num_nodes - 1;	// score nodes only, without back link

  if (new_entry_nr < 0)
    new_entry_nr = 0;
  if (new_entry_nr > num_entries - 1)
    new_entry_nr = num_entries - 1;

  if (new_entry_nr != old_entry_nr)
  {
    scores.last_entry_nr = new_entry_nr;

    DrawScoreInfo_Content(new_entry_nr);
  }
}

static void HandleScoreInfo_PlayTape(void)
{
  if (!PlayScoreTape(scores.last_entry_nr))
  {
    DrawScoreInfo_Content(scores.last_entry_nr);

    FadeIn(REDRAW_FIELD);
  }
}

void HandleScoreInfo(int mx, int my, int dx, int dy, int button)
{
  boolean button_action = (button == MB_MENU_LEAVE || button == MB_MENU_CHOICE);
  boolean button_is_valid = (mx >= 0 && my >= 0);
  boolean button_screen_clicked = (button_action && button_is_valid);

  if (server_scores.updated)
  {
    // reload scores, using updated server score cache file
    LoadLocalAndServerScore(scores.last_level_nr, FALSE);

    server_scores.updated = FALSE;

    DrawHallOfFame_setScoreEntries();

    DrawScoreInfo_Content(scores.last_entry_nr);
  }

  if (button_screen_clicked)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    SetGameStatus(GAME_MODE_SCORES);

    DrawHallOfFame(scores.last_level_nr);
  }
  else if (dx)
  {
    HandleHallOfFame_SelectLevel(1, SIGN(dx) * (ABS(dx) > 1 ? 10 : 1));
  }
  else if (dy)
  {
    HandleScoreInfo_SelectScore(1, SIGN(dy) * (ABS(dy) > 1 ? 10 : 1));
  }
}


// ============================================================================
// setup screen functions
// ============================================================================

static struct TokenInfo *setup_info;
static int num_setup_info;	// number of setup entries shown on screen
static int max_setup_info;	// total number of setup entries in list

static char *window_size_text;
static char *scaling_type_text;
static char *rendering_mode_text;
static char *vsync_mode_text;
static char *scroll_delay_text;
static char *snapshot_mode_text;
static char *game_engine_type_text;
static char *bd_palette_c64_text;
static char *bd_palette_c64dtv_text;
static char *bd_palette_atari_text;
static char *bd_color_type_text;
static char *game_speed_text;
static char *scores_type_text;
static char *network_server_text;
static char *graphics_set_name;
static char *sounds_set_name;
static char *music_set_name;
static char *volume_simple_text;
static char *volume_loops_text;
static char *volume_music_text;
static char *touch_controls_text;
static char *move_distance_text;
static char *drop_distance_text;
static char *transparency_text;
static char *grid_size_text[2][2];

static void execSetupMain(void)
{
  setup_mode = SETUP_MODE_MAIN;

  DrawSetupScreen();
}

static void execSetupGame_setScoresType(void)
{
  if (scores_types == NULL)
  {
    int i;

    for (i = 0; scores_types_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = scores_types_list[i].value;
      char *text = scores_types_list[i].text;

      ti->node_top = &scores_types;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SCORES_TYPE);

      pushTreeInfo(&scores_types, ti);
    }

    // sort scores type values to start with lowest scores type value
    sortTreeInfo(&scores_types);

    // set current scores type value to configured scores type value
    scores_type_current =
      getTreeInfoFromIdentifier(scores_types, setup.scores_in_highscore_list);

    // if that fails, set current scores type to reliable default value
    if (scores_type_current == NULL)
      scores_type_current =
	getTreeInfoFromIdentifier(scores_types, STR_SCORES_TYPE_DEFAULT);

    // if that also fails, set current scores type to first available value
    if (scores_type_current == NULL)
      scores_type_current = scores_types;
  }

  setup.scores_in_highscore_list = scores_type_current->identifier;

  // needed for displaying scores type text instead of identifier
  scores_type_text = scores_type_current->name;
}

static void execSetupGame_setGameSpeeds(boolean update_value)
{
  if (setup.game_speed_extended)
  {
    game_speeds_list = game_speeds_list_extended;
    game_speeds      = game_speeds_extended;
  }
  else
  {
    game_speeds_list = game_speeds_list_normal;
    game_speeds      = game_speeds_normal;
  }

  if (game_speeds == NULL)
  {
    int i;

    for (i = 0; game_speeds_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = game_speeds_list[i].value;
      char *text = game_speeds_list[i].text;

      ti->node_top = &game_speeds;
      ti->sort_priority = 10000 - value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_GAME_SPEED);

      pushTreeInfo(&game_speeds, ti);
    }

    // sort game speed values to start with slowest game speed
    sortTreeInfo(&game_speeds);

    update_value = TRUE;
  }

  if (update_value)
  {
    // set current game speed to configured game speed value
    game_speed_current =
      getTreeInfoFromIdentifier(game_speeds, i_to_a(setup.game_frame_delay));

    // if that fails, set current game speed to reliable default value
    if (game_speed_current == NULL)
      game_speed_current =
	getTreeInfoFromIdentifier(game_speeds, i_to_a(GAME_FRAME_DELAY));

    // if that also fails, set current game speed to first available speed
    if (game_speed_current == NULL)
      game_speed_current = game_speeds;

    if (setup.game_speed_extended)
      game_speeds_extended = game_speeds;
    else
      game_speeds_normal = game_speeds;
  }

  setup.game_frame_delay = atoi(game_speed_current->identifier);

  // needed for displaying game speed text instead of identifier
  game_speed_text = game_speed_current->name;
}

static void execSetupGame_setScrollDelays(void)
{
  if (scroll_delays == NULL)
  {
    int i;

    for (i = 0; scroll_delays_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = scroll_delays_list[i].value;
      char *text = scroll_delays_list[i].text;

      ti->node_top = &scroll_delays;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SCROLL_DELAY);

      pushTreeInfo(&scroll_delays, ti);
    }

    // sort scroll delay values to start with lowest scroll delay value
    sortTreeInfo(&scroll_delays);

    // set current scroll delay value to configured scroll delay value
    scroll_delay_current =
      getTreeInfoFromIdentifier(scroll_delays, i_to_a(setup.scroll_delay_value));

    // if that fails, set current scroll delay to reliable default value
    if (scroll_delay_current == NULL)
      scroll_delay_current =
	getTreeInfoFromIdentifier(scroll_delays, i_to_a(STD_SCROLL_DELAY));

    // if that also fails, set current scroll delay to first available value
    if (scroll_delay_current == NULL)
      scroll_delay_current = scroll_delays;
  }

  setup.scroll_delay_value = atoi(scroll_delay_current->identifier);

  // needed for displaying scroll delay text instead of identifier
  scroll_delay_text = scroll_delay_current->name;
}

static void execSetupGame_setSnapshotModes(void)
{
  if (snapshot_modes == NULL)
  {
    int i;

    for (i = 0; snapshot_modes_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = snapshot_modes_list[i].value;
      char *text = snapshot_modes_list[i].text;

      ti->node_top = &snapshot_modes;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SNAPSHOT_MODE);

      pushTreeInfo(&snapshot_modes, ti);
    }

    // sort snapshot mode values to start with lowest snapshot mode value
    sortTreeInfo(&snapshot_modes);

    // set current snapshot mode value to configured snapshot mode value
    snapshot_mode_current =
      getTreeInfoFromIdentifier(snapshot_modes, setup.engine_snapshot_mode);

    // if that fails, set current snapshot mode to reliable default value
    if (snapshot_mode_current == NULL)
      snapshot_mode_current =
	getTreeInfoFromIdentifier(snapshot_modes, STR_SNAPSHOT_MODE_DEFAULT);

    // if that also fails, set current snapshot mode to first available value
    if (snapshot_mode_current == NULL)
      snapshot_mode_current = snapshot_modes;
  }

  setup.engine_snapshot_mode = snapshot_mode_current->identifier;

  // needed for displaying snapshot mode text instead of identifier
  snapshot_mode_text = snapshot_mode_current->name;
}

static void execSetupGame_setNetworkServerText(void)
{
  if (strEqual(setup.network_server_hostname, STR_NETWORK_AUTO_DETECT))
  {
    strcpy(network_server_hostname, STR_NETWORK_AUTO_DETECT_SETUP);
  }
  else
  {
    strncpy(network_server_hostname, setup.network_server_hostname,
	    MAX_SETUP_TEXT_INPUT_LEN);
    network_server_hostname[MAX_SETUP_TEXT_INPUT_LEN] = '\0';
  }

  // needed for displaying network server text instead of identifier
  network_server_text = network_server_hostname;
}

static void execSetupGame(void)
{
  boolean check_vsync_mode = (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED);

  execSetupGame_setGameSpeeds(FALSE);
  execSetupGame_setScoresType();
  execSetupGame_setScrollDelays();
  execSetupGame_setSnapshotModes();

  execSetupGame_setNetworkServerText();

  if (!setup.provide_uploading_tapes)
    setHideSetupEntry(execOfferUploadTapes);

  setup_mode = SETUP_MODE_GAME;

  DrawSetupScreen();

  // check if vsync needs to be disabled for this game speed to work
  if (check_vsync_mode)
    DisableVsyncIfNeeded();
}

static void execSetupChooseScoresType(void)
{
  setup_mode = SETUP_MODE_CHOOSE_SCORES_TYPE;

  DrawSetupScreen();
}

static void execSetupChooseGameSpeed(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GAME_SPEED;

  DrawSetupScreen();
}

static void execSetupChooseScrollDelay(void)
{
  setup_mode = SETUP_MODE_CHOOSE_SCROLL_DELAY;

  DrawSetupScreen();
}

static void execSetupChooseSnapshotMode(void)
{
  setup_mode = SETUP_MODE_CHOOSE_SNAPSHOT_MODE;

  DrawSetupScreen();
}

static void execSetupEngines_setGameEngineType(void)
{
  if (game_engine_types == NULL)
  {
    int i;

    for (i = 0; game_engine_types_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = game_engine_types_list[i].value;
      char *text = game_engine_types_list[i].text;

      ti->node_top = &game_engine_types;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_GAME_ENGINE_TYPE);

      pushTreeInfo(&game_engine_types, ti);
    }

    // sort game engine type values to start with lowest game engine type value
    sortTreeInfo(&game_engine_types);

    // set current game engine type value to configured game engine type value
    game_engine_type_current =
      getTreeInfoFromIdentifier(game_engine_types, i_to_a(setup.default_game_engine_type));

    // if that fails, set current game engine type to reliable default value
    if (game_engine_type_current == NULL)
      game_engine_type_current =
	getTreeInfoFromIdentifier(game_engine_types, i_to_a(GAME_ENGINE_TYPE_RND));

    // if that also fails, set current game engine type to first available value
    if (game_engine_type_current == NULL)
      game_engine_type_current = game_engine_types;
  }

  setup.default_game_engine_type = atoi(game_engine_type_current->identifier);

  // needed for displaying game engine type text instead of identifier
  game_engine_type_text = game_engine_type_current->name;
}

static void execSetupEngines_setPalettesC64(void)
{
  if (bd_palettes_c64 == NULL)
  {
    int i;

    for (i = 0; bd_palettes_c64_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = bd_palettes_c64_list[i].value;
      char *text = bd_palettes_c64_list[i].text;

      ti->node_top = &bd_palettes_c64;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_BD_PALETTE_C64);

      pushTreeInfo(&bd_palettes_c64, ti);
    }

    // sort palette values to start with lowest palette value
    sortTreeInfo(&bd_palettes_c64);

    // set current palette value to configured palette value
    bd_palette_c64_current =
      getTreeInfoFromIdentifier(bd_palettes_c64, i_to_a(setup.bd_palette_c64));

    // if that fails, set current palette to reliable default value
    if (bd_palette_c64_current == NULL)
      bd_palette_c64_current =
	getTreeInfoFromIdentifier(bd_palettes_c64, i_to_a(GD_DEFAULT_PALETTE_C64));

    // if that also fails, set current palette to first available value
    if (bd_palette_c64_current == NULL)
      bd_palette_c64_current = bd_palettes_c64;
  }

  setup.bd_palette_c64 = atoi(bd_palette_c64_current->identifier);

  // needed for displaying palette text instead of identifier
  bd_palette_c64_text = bd_palette_c64_current->name;
}

static void execSetupEngines_setPalettesC64DTV(void)
{
  if (bd_palettes_c64dtv == NULL)
  {
    int i;

    for (i = 0; bd_palettes_c64dtv_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = bd_palettes_c64dtv_list[i].value;
      char *text = bd_palettes_c64dtv_list[i].text;

      ti->node_top = &bd_palettes_c64dtv;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_BD_PALETTE_C64DTV);

      pushTreeInfo(&bd_palettes_c64dtv, ti);
    }

    // sort palette values to start with lowest palette value
    sortTreeInfo(&bd_palettes_c64dtv);

    // set current palette value to configured palette value
    bd_palette_c64dtv_current =
      getTreeInfoFromIdentifier(bd_palettes_c64dtv, i_to_a(setup.bd_palette_c64dtv));

    // if that fails, set current palette to reliable default value
    if (bd_palette_c64dtv_current == NULL)
      bd_palette_c64dtv_current =
	getTreeInfoFromIdentifier(bd_palettes_c64dtv, i_to_a(GD_DEFAULT_PALETTE_C64DTV));

    // if that also fails, set current palette to first available value
    if (bd_palette_c64dtv_current == NULL)
      bd_palette_c64dtv_current = bd_palettes_c64dtv;
  }

  setup.bd_palette_c64dtv = atoi(bd_palette_c64dtv_current->identifier);

  // needed for displaying palette text instead of identifier
  bd_palette_c64dtv_text = bd_palette_c64dtv_current->name;
}

static void execSetupEngines_setPalettesAtari(void)
{
  if (bd_palettes_atari == NULL)
  {
    int i;

    for (i = 0; bd_palettes_atari_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = bd_palettes_atari_list[i].value;
      char *text = bd_palettes_atari_list[i].text;

      ti->node_top = &bd_palettes_atari;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_BD_PALETTE_ATARI);

      pushTreeInfo(&bd_palettes_atari, ti);
    }

    // sort palette values to start with lowest palette value
    sortTreeInfo(&bd_palettes_atari);

    // set current palette value to configured palette value
    bd_palette_atari_current =
      getTreeInfoFromIdentifier(bd_palettes_atari, i_to_a(setup.bd_palette_atari));

    // if that fails, set current palette to reliable default value
    if (bd_palette_atari_current == NULL)
      bd_palette_atari_current =
	getTreeInfoFromIdentifier(bd_palettes_atari, i_to_a(GD_DEFAULT_PALETTE_ATARI));

    // if that also fails, set current palette to first available value
    if (bd_palette_atari_current == NULL)
      bd_palette_atari_current = bd_palettes_atari;
  }

  setup.bd_palette_atari = atoi(bd_palette_atari_current->identifier);

  // needed for displaying palette text instead of identifier
  bd_palette_atari_text = bd_palette_atari_current->name;
}

static void execSetupEngines_setColorType(void)
{
  if (bd_color_types == NULL)
  {
    int i;

    for (i = 0; bd_color_types_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = bd_color_types_list[i].value;
      char *text = bd_color_types_list[i].text;

      ti->node_top = &bd_color_types;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_BD_COLOR_TYPE);

      pushTreeInfo(&bd_color_types, ti);
    }

    // sort color type values to start with lowest color type value
    sortTreeInfo(&bd_color_types);

    // set current color type value to configured color type value
    bd_color_type_current =
      getTreeInfoFromIdentifier(bd_color_types, i_to_a(setup.bd_default_color_type));

    // if that fails, set current color type to reliable default value
    if (bd_color_type_current == NULL)
      bd_color_type_current =
	getTreeInfoFromIdentifier(bd_color_types, i_to_a(GD_DEFAULT_COLOR_TYPE));

    // if that also fails, set current color type to first available value
    if (bd_color_type_current == NULL)
      bd_color_type_current = bd_color_types;
  }

  setup.bd_default_color_type = atoi(bd_color_type_current->identifier);

  // needed for displaying color type text instead of identifier
  bd_color_type_text = bd_color_type_current->name;
}

static void execSetupEngines(void)
{
  setup_mode = SETUP_MODE_ENGINES;

  execSetupEngines_setGameEngineType();
  execSetupEngines_setPalettesC64();
  execSetupEngines_setPalettesC64DTV();
  execSetupEngines_setPalettesAtari();
  execSetupEngines_setColorType();

  DrawSetupScreen();
}

static void execSetupChooseGameEngineType(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE;

  DrawSetupScreen();
}

static void execSetupChoosePaletteC64(void)
{
  setup_mode = SETUP_MODE_CHOOSE_BD_PALETTE_C64;

  DrawSetupScreen();
}

static void execSetupChoosePaletteC64DTV(void)
{
  setup_mode = SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV;

  DrawSetupScreen();
}

static void execSetupChoosePaletteAtari(void)
{
  setup_mode = SETUP_MODE_CHOOSE_BD_PALETTE_ATARI;

  DrawSetupScreen();
}

static void execSetupChooseColorType(void)
{
  setup_mode = SETUP_MODE_CHOOSE_BD_COLOR_TYPE;

  DrawSetupScreen();
}

static void execSetupEditor(void)
{
  setup_mode = SETUP_MODE_EDITOR;

  DrawSetupScreen();
}

static void execSetupGraphics_setWindowSizes(boolean update_list)
{
  if (window_sizes != NULL && update_list)
  {
    freeTreeInfo(window_sizes);

    window_sizes = NULL;
  }

  if (window_sizes == NULL)
  {
    boolean current_window_size_found = FALSE;
    int i;

    for (i = 0; window_sizes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = window_sizes_list[i].value;
      char *text = window_sizes_list[i].text;

      ti->node_top = &window_sizes;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_WINDOW_SIZE);

      pushTreeInfo(&window_sizes, ti);

      if (value == setup.window_scaling_percent)
	current_window_size_found = TRUE;
    }

    if (!current_window_size_found)
    {
      // add entry for non-preset window scaling value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.window_scaling_percent;

      ti->node_top = &window_sizes;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_WINDOW_SIZE);

      pushTreeInfo(&window_sizes, ti);
    }

    // sort window size values to start with lowest window size value
    sortTreeInfo(&window_sizes);

    // set current window size value to configured window size value
    window_size_current =
      getTreeInfoFromIdentifier(window_sizes,
				i_to_a(setup.window_scaling_percent));

    // if that fails, set current window size to reliable default value
    if (window_size_current == NULL)
      window_size_current =
	getTreeInfoFromIdentifier(window_sizes,
				  i_to_a(STD_WINDOW_SCALING_PERCENT));

    // if that also fails, set current window size to first available value
    if (window_size_current == NULL)
      window_size_current = window_sizes;
  }

  setup.window_scaling_percent = atoi(window_size_current->identifier);

  // needed for displaying window size text instead of identifier
  window_size_text = window_size_current->name;
}

static void execSetupGraphics_setScalingTypes(void)
{
  if (scaling_types == NULL)
  {
    int i;

    for (i = 0; scaling_types_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = scaling_types_list[i].value;
      char *text = scaling_types_list[i].text;

      ti->node_top = &scaling_types;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SCALING_TYPE);

      pushTreeInfo(&scaling_types, ti);
    }

    // sort scaling type values to start with lowest scaling type value
    sortTreeInfo(&scaling_types);

    // set current scaling type value to configured scaling type value
    scaling_type_current =
      getTreeInfoFromIdentifier(scaling_types, setup.window_scaling_quality);

    // if that fails, set current scaling type to reliable default value
    if (scaling_type_current == NULL)
      scaling_type_current =
	getTreeInfoFromIdentifier(scaling_types, SCALING_QUALITY_DEFAULT);

    // if that also fails, set current scaling type to first available value
    if (scaling_type_current == NULL)
      scaling_type_current = scaling_types;
  }

  setup.window_scaling_quality = scaling_type_current->identifier;

  // needed for displaying scaling type text instead of identifier
  scaling_type_text = scaling_type_current->name;
}

static void execSetupGraphics_setRenderingModes(void)
{
  if (rendering_modes == NULL)
  {
    int i;

    for (i = 0; rendering_modes_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = rendering_modes_list[i].value;
      char *text = rendering_modes_list[i].text;

      ti->node_top = &rendering_modes;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_RENDERING);

      pushTreeInfo(&rendering_modes, ti);
    }

    // sort rendering mode values to start with lowest rendering mode value
    sortTreeInfo(&rendering_modes);

    // set current rendering mode value to configured rendering mode value
    rendering_mode_current =
      getTreeInfoFromIdentifier(rendering_modes, setup.screen_rendering_mode);

    // if that fails, set current rendering mode to reliable default value
    if (rendering_mode_current == NULL)
      rendering_mode_current =
	getTreeInfoFromIdentifier(rendering_modes,
				  STR_SPECIAL_RENDERING_DEFAULT);

    // if that also fails, set current rendering mode to first available one
    if (rendering_mode_current == NULL)
      rendering_mode_current = rendering_modes;
  }

  setup.screen_rendering_mode = rendering_mode_current->identifier;

  // needed for displaying rendering mode text instead of identifier
  rendering_mode_text = rendering_mode_current->name;
}

static void execSetupGraphics_setVsyncModes(boolean update_value)
{
  if (vsync_modes == NULL)
  {
    int i;

    for (i = 0; vsync_modes_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = vsync_modes_list[i].value;
      char *text = vsync_modes_list[i].text;

      ti->node_top = &vsync_modes;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VSYNC);

      pushTreeInfo(&vsync_modes, ti);
    }

    // sort vsync mode values to start with lowest vsync mode value
    sortTreeInfo(&vsync_modes);

    update_value = TRUE;
  }

  if (update_value)
  {
    // set current vsync mode value to configured vsync mode value
    vsync_mode_current =
      getTreeInfoFromIdentifier(vsync_modes, setup.vsync_mode);

    // if that fails, set current vsync mode to reliable default value
    if (vsync_mode_current == NULL)
      vsync_mode_current =
	getTreeInfoFromIdentifier(vsync_modes, STR_VSYNC_MODE_DEFAULT);

    // if that also fails, set current vsync mode to first available one
    if (vsync_mode_current == NULL)
      vsync_mode_current = vsync_modes;
  }

  setup.vsync_mode = vsync_mode_current->identifier;

  // needed for displaying vsync mode text instead of identifier
  vsync_mode_text = vsync_mode_current->name;
}

static void execSetupGraphics(void)
{
  boolean check_game_speed = (setup_mode == SETUP_MODE_CHOOSE_VSYNC);

  // update "setup.window_scaling_percent" from list selection
  // (in this case, window scaling was changed on setup screen)
  if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
    execSetupGraphics_setWindowSizes(FALSE);

  // update "setup.vsync_mode" from list selection
  // (in this case, vsync mode was changed on setup screen)
  if (setup_mode == SETUP_MODE_CHOOSE_VSYNC)
    execSetupGraphics_setVsyncModes(FALSE);

  // update list selection from "setup.window_scaling_percent"
  // (window scaling may have changed by resizing the window)
  execSetupGraphics_setWindowSizes(TRUE);

  // update list selection from "setup.vsync_mode"
  // (vsync_mode may have changed by re-creating the renderer)
  execSetupGraphics_setVsyncModes(TRUE);

  execSetupGraphics_setScalingTypes();
  execSetupGraphics_setRenderingModes();

  setup_mode = SETUP_MODE_GRAPHICS;

  DrawSetupScreen();

  // check if game speed is high enough for 60 Hz vsync to work
  if (check_game_speed)
    ModifyGameSpeedIfNeeded();

  // window scaling may have changed at this point
  ChangeWindowScalingIfNeeded();

  // window scaling quality may have changed at this point
  if (!strEqual(setup.window_scaling_quality, video.window_scaling_quality))
    SDLSetWindowScalingQuality(setup.window_scaling_quality);

  // screen rendering mode may have changed at this point
  SDLSetScreenRenderingMode(setup.screen_rendering_mode);

  int setup_vsync_mode = VSYNC_MODE_STR_TO_INT(setup.vsync_mode);
  int video_vsync_mode = video.vsync_mode;

  // screen vsync mode may have changed at this point
  ChangeVsyncModeIfNeeded();

  // check if setting vsync mode to selected value failed
  if (setup_vsync_mode != video_vsync_mode &&
      setup_vsync_mode != video.vsync_mode)
  {
    // changing vsync mode to selected value failed -- reset displayed value
    execSetupGraphics_setVsyncModes(TRUE);

    Request("Setting VSync failed!", REQ_CONFIRM);

    DrawSetupScreen();
  }
}

static void execSetupChooseWindowSize(void)
{
  setup_mode = SETUP_MODE_CHOOSE_WINDOW_SIZE;

  DrawSetupScreen();
}

static void execSetupChooseScalingType(void)
{
  setup_mode = SETUP_MODE_CHOOSE_SCALING_TYPE;

  DrawSetupScreen();
}

static void execSetupChooseRenderingMode(void)
{
  setup_mode = SETUP_MODE_CHOOSE_RENDERING;

  DrawSetupScreen();
}

static void execSetupChooseVsyncMode(void)
{
  setup_mode = SETUP_MODE_CHOOSE_VSYNC;

  DrawSetupScreen();
}

static void execSetupChooseVolumeSimple(void)
{
  setup_mode = SETUP_MODE_CHOOSE_VOLUME_SIMPLE;

  DrawSetupScreen();
}

static void execSetupChooseVolumeLoops(void)
{
  setup_mode = SETUP_MODE_CHOOSE_VOLUME_LOOPS;

  DrawSetupScreen();
}

static void execSetupChooseVolumeMusic(void)
{
  setup_mode = SETUP_MODE_CHOOSE_VOLUME_MUSIC;

  DrawSetupScreen();
}

static void execSetupSound(void)
{
  if (volumes_simple == NULL)
  {
    boolean current_volume_simple_found = FALSE;
    int i;

    for (i = 0; volumes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = volumes_list[i].value;
      char *text = volumes_list[i].text;

      ti->node_top = &volumes_simple;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_SIMPLE);

      pushTreeInfo(&volumes_simple, ti);

      if (value == setup.volume_simple)
        current_volume_simple_found = TRUE;
    }

    if (!current_volume_simple_found)
    {
      // add entry for non-preset volume value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.volume_simple;

      ti->node_top = &volumes_simple;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_SIMPLE);

      pushTreeInfo(&volumes_simple, ti);
    }

    // sort volume values to start with lowest volume value
    sortTreeInfo(&volumes_simple);

    // set current volume value to configured volume value
    volume_simple_current =
      getTreeInfoFromIdentifier(volumes_simple, i_to_a(setup.volume_simple));

    // if that fails, set current volume to reliable default value
    if (volume_simple_current == NULL)
      volume_simple_current =
	getTreeInfoFromIdentifier(volumes_simple, i_to_a(100));

    // if that also fails, set current volume to first available value
    if (volume_simple_current == NULL)
      volume_simple_current = volumes_simple;
  }

  if (volumes_loops == NULL)
  {
    boolean current_volume_loops_found = FALSE;
    int i;

    for (i = 0; volumes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = volumes_list[i].value;
      char *text = volumes_list[i].text;

      ti->node_top = &volumes_loops;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_LOOPS);

      pushTreeInfo(&volumes_loops, ti);

      if (value == setup.volume_loops)
        current_volume_loops_found = TRUE;
    }

    if (!current_volume_loops_found)
    {
      // add entry for non-preset volume value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.volume_loops;

      ti->node_top = &volumes_loops;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_LOOPS);

      pushTreeInfo(&volumes_loops, ti);
    }

    // sort volume values to start with lowest volume value
    sortTreeInfo(&volumes_loops);

    // set current volume value to configured volume value
    volume_loops_current =
      getTreeInfoFromIdentifier(volumes_loops, i_to_a(setup.volume_loops));

    // if that fails, set current volume to reliable default value
    if (volume_loops_current == NULL)
      volume_loops_current =
	getTreeInfoFromIdentifier(volumes_loops, i_to_a(100));

    // if that also fails, set current volume to first available value
    if (volume_loops_current == NULL)
      volume_loops_current = volumes_loops;
  }

  if (volumes_music == NULL)
  {
    boolean current_volume_music_found = FALSE;
    int i;

    for (i = 0; volumes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = volumes_list[i].value;
      char *text = volumes_list[i].text;

      ti->node_top = &volumes_music;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_MUSIC);

      pushTreeInfo(&volumes_music, ti);

      if (value == setup.volume_music)
        current_volume_music_found = TRUE;
    }

    if (!current_volume_music_found)
    {
      // add entry for non-preset volume value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.volume_music;

      ti->node_top = &volumes_music;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_MUSIC);

      pushTreeInfo(&volumes_music, ti);
    }

    // sort volume values to start with lowest volume value
    sortTreeInfo(&volumes_music);

    // set current volume value to configured volume value
    volume_music_current =
      getTreeInfoFromIdentifier(volumes_music, i_to_a(setup.volume_music));

    // if that fails, set current volume to reliable default value
    if (volume_music_current == NULL)
      volume_music_current =
	getTreeInfoFromIdentifier(volumes_music, i_to_a(100));

    // if that also fails, set current volume to first available value
    if (volume_music_current == NULL)
      volume_music_current = volumes_music;
  }

  setup.volume_simple = atoi(volume_simple_current->identifier);
  setup.volume_loops  = atoi(volume_loops_current->identifier);
  setup.volume_music  = atoi(volume_music_current->identifier);

  // needed for displaying volume text instead of identifier
  volume_simple_text = volume_simple_current->name;
  volume_loops_text = volume_loops_current->name;
  volume_music_text = volume_music_current->name;

  setup_mode = SETUP_MODE_SOUND;

  DrawSetupScreen();
}

static void execSetupChooseTouchControls(void)
{
  setup_mode = SETUP_MODE_CHOOSE_TOUCH_CONTROL;

  DrawSetupScreen();
}

static void execSetupChooseMoveDistance(void)
{
  setup_mode = SETUP_MODE_CHOOSE_MOVE_DISTANCE;

  DrawSetupScreen();
}

static void execSetupChooseDropDistance(void)
{
  setup_mode = SETUP_MODE_CHOOSE_DROP_DISTANCE;

  DrawSetupScreen();
}

static void execSetupChooseTransparency(void)
{
  setup_mode = SETUP_MODE_CHOOSE_TRANSPARENCY;

  DrawSetupScreen();
}

static void execSetupChooseGridXSize_0(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GRID_XSIZE_0;

  DrawSetupScreen();
}

static void execSetupChooseGridYSize_0(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GRID_YSIZE_0;

  DrawSetupScreen();
}

static void execSetupChooseGridXSize_1(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GRID_XSIZE_1;

  DrawSetupScreen();
}

static void execSetupChooseGridYSize_1(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GRID_YSIZE_1;

  DrawSetupScreen();
}

static void execSetupConfigureVirtualButtons(void)
{
  setup_mode = SETUP_MODE_CONFIG_VIRT_BUTTONS;

  ConfigureVirtualButtons();

  setup_mode = SETUP_MODE_TOUCH;

  DrawSetupScreen();
}

static void execSetupTouch(void)
{
  int i, j, k;

  if (touch_controls == NULL)
  {
    for (i = 0; touch_controls_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = touch_controls_list[i].value;
      char *text = touch_controls_list[i].text;

      ti->node_top = &touch_controls;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_TOUCH_CONTROL);

      pushTreeInfo(&touch_controls, ti);
    }

    // sort touch control values to start with lowest touch control value
    sortTreeInfo(&touch_controls);

    // set current touch control value to configured touch control value
    touch_control_current =
      getTreeInfoFromIdentifier(touch_controls, setup.touch.control_type);

    // if that fails, set current touch control to reliable default value
    if (touch_control_current == NULL)
      touch_control_current =
	getTreeInfoFromIdentifier(touch_controls, TOUCH_CONTROL_DEFAULT);

    // if that also fails, set current touch control to first available value
    if (touch_control_current == NULL)
      touch_control_current = touch_controls;
  }

  if (move_distances == NULL)
  {
    for (i = 0; distances_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = distances_list[i].value;
      char *text = distances_list[i].text;

      ti->node_top = &move_distances;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_MOVE_DISTANCE);

      pushTreeInfo(&move_distances, ti);
    }

    // sort distance values to start with lowest distance value
    sortTreeInfo(&move_distances);

    // set current distance value to configured distance value
    move_distance_current =
      getTreeInfoFromIdentifier(move_distances,
				i_to_a(setup.touch.move_distance));

    // if that fails, set current distance to reliable default value
    if (move_distance_current == NULL)
      move_distance_current =
	getTreeInfoFromIdentifier(move_distances,
				  i_to_a(TOUCH_MOVE_DISTANCE_DEFAULT));

    // if that also fails, set current distance to first available value
    if (move_distance_current == NULL)
      move_distance_current = move_distances;
  }

  if (drop_distances == NULL)
  {
    for (i = 0; distances_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = distances_list[i].value;
      char *text = distances_list[i].text;

      ti->node_top = &drop_distances;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_DROP_DISTANCE);

      pushTreeInfo(&drop_distances, ti);
    }

    // sort distance values to start with lowest distance value
    sortTreeInfo(&drop_distances);

    // set current distance value to configured distance value
    drop_distance_current =
      getTreeInfoFromIdentifier(drop_distances,
				i_to_a(setup.touch.drop_distance));

    // if that fails, set current distance to reliable default value
    if (drop_distance_current == NULL)
      drop_distance_current =
	getTreeInfoFromIdentifier(drop_distances,
				  i_to_a(TOUCH_DROP_DISTANCE_DEFAULT));

    // if that also fails, set current distance to first available value
    if (drop_distance_current == NULL)
      drop_distance_current = drop_distances;
  }

  if (transparencies == NULL)
  {
    for (i = 0; transparencies_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = transparencies_list[i].value;
      char *text = transparencies_list[i].text;

      ti->node_top = &transparencies;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_TRANSPARENCY);

      pushTreeInfo(&transparencies, ti);
    }

    // sort transparency values to start with lowest transparency value
    sortTreeInfo(&transparencies);

    // set current transparency value to configured transparency value
    transparency_current =
      getTreeInfoFromIdentifier(transparencies,
				i_to_a(setup.touch.transparency));

    // if that fails, set current transparency to reliable default value
    if (transparency_current == NULL)
      transparency_current =
	getTreeInfoFromIdentifier(transparencies,
				  i_to_a(TOUCH_TRANSPARENCY_DEFAULT));

    // if that also fails, set current transparency to first available value
    if (transparency_current == NULL)
      transparency_current = transparencies;
  }

  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < 2; j++)
    {
      if (grid_sizes[i][j] == NULL)
      {
	for (k = 0; grid_sizes_list[k].value != -1; k++)
	{
	  TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
	  char identifier[32], name[32];
	  int value = grid_sizes_list[k].value;
	  char *text = grid_sizes_list[k].text;

	  ti->node_top = &grid_sizes[i][j];
	  ti->sort_priority = value;

	  sprintf(identifier, "%d", value);
	  sprintf(name, "%s", text);

	  setString(&ti->identifier, identifier);
	  setString(&ti->name, name);
	  setString(&ti->name_sorting, name);
	  setString(&ti->infotext,
		    (i == 0 ?
		     (j == 0 ?
		      STR_SETUP_CHOOSE_GRID_XSIZE_0 :
		      STR_SETUP_CHOOSE_GRID_YSIZE_0) :
		     (j == 0 ?
		      STR_SETUP_CHOOSE_GRID_XSIZE_1 :
		      STR_SETUP_CHOOSE_GRID_YSIZE_1)));

	  pushTreeInfo(&grid_sizes[i][j], ti);
	}

	// sort grid size values to start with lowest grid size value
	sortTreeInfo(&grid_sizes[i][j]);

	// set current grid size value to configured grid size value
	grid_size_current[i][j] =
	  getTreeInfoFromIdentifier(grid_sizes[i][j],
				    i_to_a(j == 0 ?
					   setup.touch.grid_xsize[i] :
					   setup.touch.grid_ysize[i]));

	// if that fails, set current grid size to reliable default value
	if (grid_size_current[i][j] == NULL)
	  grid_size_current[i][j] =
	    getTreeInfoFromIdentifier(grid_sizes[i][j],
				      i_to_a(j == 0 ?
					     DEFAULT_GRID_XSIZE(i) :
					     DEFAULT_GRID_YSIZE(i)));

	// if that also fails, set current grid size to first available value
	if (grid_size_current[i][j] == NULL)
	  grid_size_current[i][j] = grid_sizes[i][j];
      }
    }
  }

  setup.touch.control_type = touch_control_current->identifier;
  setup.touch.move_distance = atoi(move_distance_current->identifier);
  setup.touch.drop_distance = atoi(drop_distance_current->identifier);
  setup.touch.transparency = atoi(transparency_current->identifier);

  for (i = 0; i < 2; i++)
  {
    setup.touch.grid_xsize[i] = atoi(grid_size_current[i][0]->identifier);
    setup.touch.grid_ysize[i] = atoi(grid_size_current[i][1]->identifier);

    if (i == GRID_ACTIVE_NR())
    {
      overlay.grid_xsize = setup.touch.grid_xsize[i];
      overlay.grid_ysize = setup.touch.grid_ysize[i];
    }
  }

  // needed for displaying value text instead of identifier
  touch_controls_text = touch_control_current->name;
  move_distance_text = move_distance_current->name;
  drop_distance_text = drop_distance_current->name;
  transparency_text = transparency_current->name;

  for (i = 0; i < 2; i++)
    for (j = 0; j < 2; j++)
      grid_size_text[i][j] = grid_size_current[i][j]->name;

  setup_mode = SETUP_MODE_TOUCH;

  DrawSetupScreen();
}

static void execSetupArtwork(void)
{
  static ArtworkDirTree *gfx_last_valid = NULL;
  static ArtworkDirTree *snd_last_valid = NULL;
  static ArtworkDirTree *mus_last_valid = NULL;

  // current artwork directory may be invalid (level group, parent link)
  if (!validLevelSeries(artwork.gfx_current))
    artwork.gfx_current = getFirstValidTreeInfoEntry(gfx_last_valid);
  if (!validLevelSeries(artwork.snd_current))
    artwork.snd_current = getFirstValidTreeInfoEntry(snd_last_valid);
  if (!validLevelSeries(artwork.mus_current))
    artwork.mus_current = getFirstValidTreeInfoEntry(mus_last_valid);

  // store valid artwork directory information
  gfx_last_valid = artwork.gfx_current;
  snd_last_valid = artwork.snd_current;
  mus_last_valid = artwork.mus_current;

#if 0
  Debug("screens:execSetupArtwork", "'%s', '%s', '%s'",
	artwork.gfx_current->subdir,
	artwork.gfx_current->fullpath,
	artwork.gfx_current->basepath);
#endif

  setup.graphics_set = artwork.gfx_current->identifier;
  setup.sounds_set = artwork.snd_current->identifier;
  setup.music_set = artwork.mus_current->identifier;

  // needed if last screen (setup choice) changed graphics, sounds or music
  ReloadCustomArtwork(0);

  // needed for displaying artwork name instead of artwork identifier
  graphics_set_name = artwork.gfx_current->name;
  sounds_set_name = artwork.snd_current->name;
  music_set_name = artwork.mus_current->name;

  setup_mode = SETUP_MODE_ARTWORK;

  DrawSetupScreen();
}

static void execSetupChooseGraphics(void)
{
  setup_mode = SETUP_MODE_CHOOSE_GRAPHICS;

  DrawSetupScreen();
}

static void execSetupChooseSounds(void)
{
  setup_mode = SETUP_MODE_CHOOSE_SOUNDS;

  DrawSetupScreen();
}

static void execSetupChooseMusic(void)
{
  setup_mode = SETUP_MODE_CHOOSE_MUSIC;

  DrawSetupScreen();
}

static void execSetupInput(void)
{
  setup_mode = SETUP_MODE_INPUT;

  DrawSetupScreen();
}

static void execSetupShortcuts(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS;

  DrawSetupScreen();
}

static void execSetupShortcuts1(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_1;

  DrawSetupScreen();
}

static void execSetupShortcuts2(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_2;

  DrawSetupScreen();
}

static void execSetupShortcuts3(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_3;

  DrawSetupScreen();
}

static void execSetupShortcuts4(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_4;

  DrawSetupScreen();
}

static void execSetupShortcuts5(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_5;

  DrawSetupScreen();
}

static void execSetupShortcuts6(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_6;

  DrawSetupScreen();
}

static void execSetupShortcuts7(void)
{
  setup_mode = SETUP_MODE_SHORTCUTS_7;

  DrawSetupScreen();
}

static void execExitSetup(void)
{
  SetGameStatus(GAME_MODE_MAIN);

  DrawMainMenu();
}

static void execSaveAndExitSetup(void)
{
  SaveSetup();
  execExitSetup();
}

static void execGadgetNetworkServer(void)
{
  int gadget_id = SCREEN_CTRL_ID_NETWORK_SERVER;
  struct GadgetInfo *gi = screen_gadget[gadget_id];

  if (strEqual(setup.network_server_hostname, STR_NETWORK_AUTO_DETECT))
    network_server_hostname[0] = '\0';

  ModifyGadget(gi, GDI_TEXT_VALUE, network_server_hostname, GDI_END);

  MapGadget(gi);

  ClickOnGadget(gi, MB_LEFTBUTTON);
}

static void execOfferUploadTapes(void)
{
  OfferUploadTapes();
}

static void ToggleNetworkModeIfNeeded(void)
{
  int font_title = FONT_TITLE_1;
  int font_foot = FC_BLUE;
  int ystart  = MENU_TITLE_YPOS;
  int yfooter = MENU_FOOTER_YPOS;
  char *text = (setup.network_mode ? "Start Network" : "Stop Network");

  if (setup.network_mode == network.enabled)
    return;

  network.enabled = setup.network_mode;

  FadeOut(REDRAW_ALL);

  ClearField();

  DrawTextSCentered(ystart, font_title, text);

  FadeIn(REDRAW_ALL);

  if (network.enabled)
    InitNetworkServer();
  else
    DisconnectFromNetworkServer();

  DrawTextSCentered(yfooter, font_foot,
		    "Press any key or button for setup menu");

  WaitForEventToContinue();

  DrawSetupScreen();
}

static void ToggleGameSpeedsListIfNeeded(void)
{
  boolean using_game_speeds_extended = (game_speeds == game_speeds_extended);

  if (setup.game_speed_extended == using_game_speeds_extended)
    return;

  // try to match similar values when changing game speeds list
  if (setup.game_speed_extended)
    setup.game_frame_delay = (setup.game_frame_delay == 15 ? 16 :
			      setup.game_frame_delay == 30 ? 29 :
			      setup.game_frame_delay);
  else
    setup.game_frame_delay = (setup.game_frame_delay == 14 ? 15 :
			      setup.game_frame_delay == 16 ? 15 :
			      setup.game_frame_delay >= 29 ? 30 :
			      setup.game_frame_delay <= 10 ? 10 :
			      setup.game_frame_delay);

  execSetupGame_setGameSpeeds(TRUE);

  DrawSetupScreen();
}

static void ToggleUseApiServerIfNeeded(void)
{
  if (runtime.use_api_server == setup.use_api_server)
    return;

  runtime.use_api_server = setup.use_api_server;

  if (runtime.use_api_server)
  {
    if (setup.has_remaining_tapes)
      setup.ask_for_uploading_tapes = TRUE;

    CheckApiServerTasks();
  }
}

static void UpdateHandicapAndSkipLevels(void)
{
  // these setup options are obsolete and not used anymore;
  // update them anyway for testing with older game versions

  setup.handicap    = (setup.allow_skipping_levels != STATE_TRUE);
  setup.skip_levels = (setup.allow_skipping_levels != STATE_FALSE);
}

static void ModifyGameSpeedIfNeeded(void)
{
  if (strEqual(setup.vsync_mode, STR_VSYNC_MODE_OFF) ||
      setup.game_frame_delay <= MAX_VSYNC_FRAME_DELAY)
    return;

  char message[100];
  char *game_speed_text = "Fast";
  int game_speed_value = 15;

  if (setup.game_speed_extended)
  {
    game_speed_text = "60 fps";
    game_speed_value = 16;
  }

  sprintf(message, "Game speed set to %s for VSync to work!", game_speed_text);

  // set game speed to existing list value that is fast enough for vsync
  setup.game_frame_delay = game_speed_value;

  execSetupGame_setGameSpeeds(TRUE);

  Request(message, REQ_CONFIRM);
}

static void DisableVsyncIfNeeded(void)
{
  if (strEqual(setup.vsync_mode, STR_VSYNC_MODE_OFF) ||
      (setup.game_frame_delay >= MIN_VSYNC_FRAME_DELAY &&
       setup.game_frame_delay <= MAX_VSYNC_FRAME_DELAY))
    return;

  // disable vsync for the selected game speed to work
  setup.vsync_mode = STR_VSYNC_MODE_OFF;

  execSetupGraphics_setVsyncModes(TRUE);

  Request("VSync disabled for this game speed to work!", REQ_CONFIRM);
}

static struct
{
  void *value;
  void *related_value;
} hide_related_entry_list[] =
{
  { &setup.network_server_hostname,		execGadgetNetworkServer		},
  { &setup.network_server_hostname,		&network_server_text		},

  { &setup.scores_in_highscore_list,		execSetupChooseScoresType	},
  { &setup.scores_in_highscore_list,		&scores_type_text		},

  { &setup.game_frame_delay,			execSetupChooseGameSpeed	},
  { &setup.game_frame_delay,			&game_speed_text		},

  { &setup.scroll_delay_value,			execSetupChooseScrollDelay	},
  { &setup.scroll_delay_value,			&scroll_delay_text		},

  { &setup.engine_snapshot_mode,		execSetupChooseSnapshotMode	},
  { &setup.engine_snapshot_mode,		&snapshot_mode_text		},

  { &setup.default_game_engine_type,		execSetupChooseGameEngineType	},
  { &setup.default_game_engine_type,		&game_engine_type_text		},

  { &setup.bd_palette_c64,			execSetupChoosePaletteC64	},
  { &setup.bd_palette_c64,			&bd_palette_c64_text		},

  { &setup.bd_palette_c64dtv,			execSetupChoosePaletteC64DTV	},
  { &setup.bd_palette_c64dtv,			&bd_palette_c64dtv_text		},

  { &setup.bd_palette_atari,			execSetupChoosePaletteAtari	},
  { &setup.bd_palette_atari,			&bd_palette_atari_text		},

  { &setup.bd_default_color_type,		execSetupChooseColorType	},
  { &setup.bd_default_color_type,		&bd_color_type_text		},

  { &setup.window_scaling_percent,		execSetupChooseWindowSize	},
  { &setup.window_scaling_percent,		&window_size_text		},

  { &setup.window_scaling_quality,		execSetupChooseScalingType	},
  { &setup.window_scaling_quality,		&scaling_type_text		},

  { &setup.screen_rendering_mode,		execSetupChooseRenderingMode	},
  { &setup.screen_rendering_mode,		&rendering_mode_text		},

  { &setup.vsync_mode,				execSetupChooseVsyncMode	},
  { &setup.vsync_mode,				&vsync_mode_text		},

  { &setup.graphics_set,			execSetupChooseGraphics		},
  { &setup.graphics_set,			&graphics_set_name		},

  { &setup.sounds_set,				execSetupChooseSounds		},
  { &setup.sounds_set,				&sounds_set_name		},

  { &setup.music_set,				execSetupChooseMusic		},
  { &setup.music_set,				&music_set_name			},

  { &setup.volume_simple,			execSetupChooseVolumeSimple	},
  { &setup.volume_simple,			&volume_simple_text		},

  { &setup.volume_loops,			execSetupChooseVolumeLoops	},
  { &setup.volume_loops,			&volume_loops_text		},

  { &setup.volume_music,			execSetupChooseVolumeMusic	},
  { &setup.volume_music,			&volume_music_text		},

  { &setup.touch.control_type,			execSetupChooseTouchControls	},
  { &setup.touch.control_type,			&touch_controls_text		},

  { &setup.touch.move_distance,			execSetupChooseMoveDistance	},
  { &setup.touch.move_distance,			&move_distance_text		},

  { &setup.touch.drop_distance,			execSetupChooseDropDistance	},
  { &setup.touch.drop_distance,			&drop_distance_text		},

  { &setup.touch.transparency,			execSetupChooseTransparency	},
  { &setup.touch.transparency,			&transparency_text		},

  { &setup.touch.grid_xsize[0],			execSetupChooseGridXSize_0	},
  { &setup.touch.grid_xsize[0],			&grid_size_text[0][0]		},

  { &setup.touch.grid_ysize[0],			execSetupChooseGridYSize_0	},
  { &setup.touch.grid_ysize[0],			&grid_size_text[0][1]		},

  { &setup.touch.grid_xsize[1],			execSetupChooseGridXSize_1	},
  { &setup.touch.grid_xsize[1],			&grid_size_text[1][0]		},

  { &setup.touch.grid_ysize[1],			execSetupChooseGridYSize_1	},
  { &setup.touch.grid_ysize[1],			&grid_size_text[1][1]		},

  { &setup.internal.menu_game,			execSetupGame			},
  { &setup.internal.menu_engines,		execSetupEngines		},
  { &setup.internal.menu_editor,		execSetupEditor			},
  { &setup.internal.menu_graphics,		execSetupGraphics		},
  { &setup.internal.menu_sound,			execSetupSound			},
  { &setup.internal.menu_artwork,		execSetupArtwork		},
  { &setup.internal.menu_input,			execSetupInput			},
  { &setup.internal.menu_touch,			execSetupTouch			},
  { &setup.internal.menu_shortcuts,		execSetupShortcuts		},
  { &setup.internal.menu_exit,			execExitSetup			},
  { &setup.internal.menu_save_and_exit,		execSaveAndExitSetup		},

  { &setup.internal.menu_shortcuts_various,	execSetupShortcuts1		},
  { &setup.internal.menu_shortcuts_focus,	execSetupShortcuts2		},
  { &setup.internal.menu_shortcuts_tape,	execSetupShortcuts3		},
  { &setup.internal.menu_shortcuts_sound,	execSetupShortcuts4		},
  { &setup.internal.menu_shortcuts_snap,	execSetupShortcuts5		},
  { &setup.internal.menu_shortcuts_speed,	execSetupShortcuts6		},
  { &setup.internal.menu_shortcuts_engine,	execSetupShortcuts7		},

  { &setup.internal.info_title,			execInfoTitleScreen		},
  { &setup.internal.info_elements,		execInfoElements		},
  { &setup.internal.info_music,			execInfoMusic			},
  { &setup.internal.info_credits,		execInfoCredits			},
  { &setup.internal.info_program,		execInfoProgram			},
  { &setup.internal.info_version,		execInfoVersion			},
  { &setup.internal.info_levelset,		execInfoLevelSet		},
  { &setup.internal.info_level,			execInfoLevel			},
  { &setup.internal.info_story,			execInfoStory			},
  { &setup.internal.info_exit,			execExitInfo			},

  { NULL,					NULL				}
};

void setHideRelatedSetupEntries(void)
{
  int i;

  for (i = 0; hide_related_entry_list[i].value != NULL; i++)
    if (hideSetupEntry(hide_related_entry_list[i].value))
      setHideSetupEntry(hide_related_entry_list[i].related_value);
}

static struct TokenInfo setup_info_main[] =
{
  { TYPE_ENTER_MENU,	execSetupGame,			STR_SETUP_GAME			},
  { TYPE_ENTER_MENU,	execSetupEngines,		STR_SETUP_ENGINES		},
  { TYPE_ENTER_MENU,	execSetupEditor,		STR_SETUP_EDITOR		},
  { TYPE_ENTER_MENU,	execSetupGraphics,		STR_SETUP_GRAPHICS		},
  { TYPE_ENTER_MENU,	execSetupSound,			STR_SETUP_SOUND			},
  { TYPE_ENTER_MENU,	execSetupArtwork,		STR_SETUP_ARTWORK		},
  { TYPE_ENTER_MENU,	execSetupInput,			STR_SETUP_INPUT			},
  { TYPE_ENTER_MENU,	execSetupTouch,			STR_SETUP_TOUCH			},
  { TYPE_ENTER_MENU,	execSetupShortcuts,		STR_SETUP_SHORTCUTS		},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execExitSetup, 			STR_SETUP_EXIT			},
  { TYPE_LEAVE_MENU,	execSaveAndExitSetup,		STR_SETUP_SAVE_AND_EXIT		},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_game[] =
{
  { TYPE_SWITCH,	&setup.team_mode,		"Team-Mode (Multi-Player):"	},
  { TYPE_SWITCH,	&setup.network_mode,		"Network Multi-Player Mode:"	},
  { TYPE_PLAYER,	&setup.network_player_nr,	"Preferred Network Player:"	},
  { TYPE_TEXT_INPUT,	execGadgetNetworkServer,	"Network Server Hostname:"	},
  { TYPE_STRING,	&network_server_text,		""				},
  { TYPE_SWITCH,	&setup.use_api_server,		"Use Highscore Server:"		},
  { TYPE_ENTER_LIST,	execSetupChooseScoresType,	"Scores in Highscore List:"	},
  { TYPE_STRING,	&scores_type_text,		""				},
  { TYPE_ENTER_LIST,	execOfferUploadTapes,		"Upload Tapes to Server"	},
  { TYPE_SWITCH,	&setup.multiple_users,		"Multiple Users/Teams:"		},
  { TYPE_YES_NO,	&setup.input_on_focus,		"Only Move Focussed Player:"	},
  { TYPE_SWITCH,	&setup.time_limit,		"Time Limit:"			},
  { TYPE_YES_NO_ASK,	&setup.allow_skipping_levels,	"Allow Skipping Levels:"	},
  { TYPE_SWITCH,	&setup.increment_levels,	"Increment Solved Levels:"	},
  { TYPE_SWITCH,	&setup.auto_play_next_level,	"Auto-play Next Level:"		},
  { TYPE_SWITCH,	&setup.count_score_after_game,	"Count Score After Game:"	},
  { TYPE_SWITCH,	&setup.show_scores_after_game,	"Show Scores After Game:"	},
  { TYPE_YES_NO,	&setup.ask_on_save_tape,	"Ask on Save Tape:"		},
  { TYPE_YES_NO,	&setup.ask_on_game_over,	"Ask on Game Over:"		},
  { TYPE_YES_NO,	&setup.ask_on_quit_game,	"Ask on Quit Game:"		},
  { TYPE_YES_NO,	&setup.ask_on_quit_program,	"Ask on Quit Program:"		},
  { TYPE_SWITCH,	&setup.autorecord,		"Auto-Record When Playing:"	},
  { TYPE_SWITCH,	&setup.autorecord_after_replay,	"Auto-Record After Replay:"	},
  { TYPE_SWITCH,	&setup.auto_pause_on_start,	"Start Game in Pause Mode:"	},
  { TYPE_ENTER_LIST,	execSetupChooseGameSpeed,	"Game Speed:"			},
  { TYPE_STRING,	&game_speed_text,		""				},
  { TYPE_SWITCH,	&setup.game_speed_extended,	"Game Speed Extended List:"	},
#if 1
  { TYPE_ENTER_LIST,	execSetupChooseScrollDelay,	"Scroll Delay:"			},
  { TYPE_STRING,	&scroll_delay_text,		""				},
#endif
  { TYPE_ENTER_LIST,	execSetupChooseSnapshotMode,	"Game Engine Snapshot Mode:"	},
  { TYPE_STRING,	&snapshot_mode_text,		""				},
  { TYPE_SWITCH,	&setup.show_load_save_buttons,	"Show Load/Save Buttons:"	},
  { TYPE_SWITCH,	&setup.show_undo_redo_buttons,	"Show Undo/Redo Buttons:"	},
  { TYPE_SWITCH,	&setup.show_menu_to_save_setup,	"Show Menu to Save Setup:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_engines[] =
{
  { TYPE_ENTER_LIST,	&execSetupChooseGameEngineType,	"Default Game Engine:"		},
  { TYPE_STRING,	&game_engine_type_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ECS_AGA,	&setup.prefer_aga_graphics,	"Game Graphics Style:"		},
  { TYPE_SWITCH,	&setup.show_extra_panel_items,	"Show Extra Panel Items:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_HEADLINE,	NULL,				"Boulder Dash"			},
  { TYPE_SWITCH,	&setup.bd_multiple_lives,	"Play with multiple lives:"	},
  { TYPE_SWITCH,	&setup.bd_skip_uncovering,	"Skip (un)covering screen:"	},
  { TYPE_SWITCH,	&setup.bd_skip_hatching,	"Skip hatching player:"		},
  { TYPE_SWITCH,	&setup.bd_scroll_delay,		"Scroll Delay:"			},
  { TYPE_YES_NO_AUTO,	&setup.bd_smooth_movements,	"Smooth Element Movement:"	},
  { TYPE_YES_NO_AUTO,	&setup.bd_pushing_graphics,	"Player Pushing Graphics:"	},
  { TYPE_YES_NO_AUTO,	&setup.bd_up_down_graphics,	"Player Up/Down Graphics:"	},
  { TYPE_YES_NO_AUTO,	&setup.bd_falling_sounds,	"Double Falling Sounds:"	},
  { TYPE_SWITCH,	&setup.bd_show_invisible_outbox,"Show invisible outbox:"	},
  { TYPE_ENTER_LIST,	&execSetupChoosePaletteC64,	"Color Palette (C64):"		},
  { TYPE_STRING,	&bd_palette_c64_text,		""				},
  { TYPE_ENTER_LIST,	&execSetupChoosePaletteC64DTV,	"Color Palette (C64DTV):"	},
  { TYPE_STRING,	&bd_palette_c64dtv_text,	""				},
  { TYPE_ENTER_LIST,	&execSetupChoosePaletteAtari,	"Color Palette (Atari):"	},
  { TYPE_STRING,	&bd_palette_atari_text,		""				},
  { TYPE_ENTER_LIST,	&execSetupChooseColorType,	"Preferred Color Type:"		},
  { TYPE_STRING,	&bd_color_type_text,		""				},
  { TYPE_SWITCH,	&setup.bd_random_colors,	"Random Colors:"		},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_HEADLINE,	NULL,				"Emerald Mine"			},
  { TYPE_SWITCH,	&setup.forced_scroll_delay,	"Scroll Delay:"			},
  { TYPE_SWITCH,	&setup.prefer_lowpass_sounds,	"Low-Pass Filter Sounds:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_HEADLINE,	NULL,				"Supaplex"			},
  { TYPE_SWITCH,	&setup.sp_show_border_elements, "Border Elements:"		},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_editor[] =
{
#if 0
  { TYPE_SWITCH,	&setup.editor.el_boulderdash,	"Boulder Dash:"			},
  { TYPE_SWITCH,	&setup.editor.el_boulderdash_native, "Boulder Dash Native:"	},
  { TYPE_SWITCH,	&setup.editor.el_emerald_mine,	"Emerald Mine:"			},
  { TYPE_SWITCH,	&setup.editor.el_emerald_mine_club, "Emerald Mine Club:"	},
  { TYPE_SWITCH,	&setup.editor.el_more,		"Rocks'n'Diamonds:"		},
  { TYPE_SWITCH,	&setup.editor.el_sokoban,	"Sokoban:"			},
  { TYPE_SWITCH,	&setup.editor.el_supaplex,	"Supaplex:"			},
  { TYPE_SWITCH,	&setup.editor.el_diamond_caves,	"Diamond Caves II:"		},
  { TYPE_SWITCH,	&setup.editor.el_dx_boulderdash,"DX-Boulderdash:"		},
  { TYPE_SWITCH,	&setup.editor.el_chars,		"Text Characters:"		},
  { TYPE_SWITCH,	&setup.editor.el_steel_chars,	"Text Characters (Steel):" 	},
#endif
  { TYPE_SWITCH,	&setup.editor.el_classic,	"Classic Elements:"		},
  { TYPE_SWITCH,	&setup.editor.el_custom,	"Custom & Group Elements:"	},
#if 0
  { TYPE_SWITCH,	&setup.editor.el_headlines,	"Headlines:"			},
#endif
  { TYPE_SWITCH,	&setup.editor.el_user_defined,	"User defined element list:"	},
  { TYPE_SWITCH,	&setup.editor.el_dynamic,	"Dynamic level elements:"	},
  { TYPE_EMPTY,		NULL,				""				},
#if 0
  { TYPE_SWITCH,	&setup.editor.el_by_game,	"Show elements by game:"	},
  { TYPE_SWITCH,	&setup.editor.el_by_type,	"Show elements by type:"	},
  { TYPE_EMPTY,		NULL,				""				},
#endif
  { TYPE_SWITCH,	&setup.editor.show_element_token, "Show element token:"		},
  { TYPE_SWITCH,	&setup.editor.fast_game_start,	"Fast game start:"		},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_SWITCH,	&setup.editor.show_read_only_warning, "Show read-only warning:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_graphics[] =
{
#if !defined(PLATFORM_ANDROID) && !defined(PLATFORM_EMSCRIPTEN)
  { TYPE_SWITCH,	&setup.fullscreen,		"Fullscreen:"			},
  { TYPE_ENTER_LIST,	execSetupChooseWindowSize,	"Window Scaling:"		},
  { TYPE_STRING,	&window_size_text,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseScalingType,	"Anti-Aliasing:"		},
  { TYPE_STRING,	&scaling_type_text,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseRenderingMode,	"Special Rendering:"		},
  { TYPE_STRING,	&rendering_mode_text,		""				},
#endif
#if 0
  { TYPE_ENTER_LIST,	execSetupChooseScrollDelay,	"Scroll Delay:"			},
  { TYPE_STRING,	&scroll_delay_text,		""				},
#endif
#if !defined(PLATFORM_EMSCRIPTEN)
  { TYPE_ENTER_LIST,	execSetupChooseVsyncMode,	"Vertical Sync (VSync):"	},
  { TYPE_STRING,	&vsync_mode_text,		""				},
#endif
  { TYPE_SWITCH,	&setup.fade_screens,		"Fade Screens:"			},
  { TYPE_SWITCH,	&setup.quick_switch,		"Quick Player Focus Switch:"	},
  { TYPE_SWITCH,	&setup.quick_doors,		"Quick Menu Doors:"		},
  { TYPE_SWITCH,	&setup.show_titlescreen,	"Show Title Screens:"		},
  { TYPE_YES_NO_ONCE,	&setup.show_level_story,	"Show Level Stories:"		},
  { TYPE_SWITCH,	&setup.toons,			"Show Toons:"			},
  { TYPE_SWITCH,	&setup.small_game_graphics,	"Small Game Graphics:"		},
  { TYPE_YES_NO_AUTO,	&setup.debug.xsn_mode,		debug_xsn_mode			},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_sound[] =
{
  { TYPE_SWITCH,	&setup.sound_simple,		"Sound Effects (Normal):" 	},
  { TYPE_SWITCH,	&setup.sound_loops,		"Sound Effects (Looping):"	},
  { TYPE_SWITCH,	&setup.sound_music,		"Music:"			},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ENTER_LIST,	execSetupChooseVolumeSimple,	"Sound Volume (Normal):"	},
  { TYPE_STRING,	&volume_simple_text,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseVolumeLoops,	"Sound Volume (Looping):"	},
  { TYPE_STRING,	&volume_loops_text,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseVolumeMusic,	"Music Volume:"			},
  { TYPE_STRING,	&volume_music_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_SWITCH,	&setup.audio_sample_rate_44100,	"44100 Hz audio mixing:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_artwork[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseGraphics,	"Custom Graphics:"		},
  { TYPE_STRING,	&graphics_set_name,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseSounds,		"Custom Sounds:"		},
  { TYPE_STRING,	&sounds_set_name,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseMusic,		"Custom Music:"			},
  { TYPE_STRING,	&music_set_name,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_YES_NO_AUTO,	&setup.override_level_graphics,"Override Level Graphics:"	},
  { TYPE_YES_NO_AUTO,	&setup.override_level_sounds,	"Override Level Sounds:" 	},
  { TYPE_YES_NO_AUTO,	&setup.override_level_music,	"Override Level Music:"  	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_input[] =
{
  { TYPE_SWITCH,	NULL,				"Player:"			},
  { TYPE_SWITCH,	NULL,				"Device:"			},
  { TYPE_SWITCH,	NULL,				""				},
  { TYPE_SKIPPABLE,	NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_SKIPPABLE,	NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_touch[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseTouchControls,	"Touch Control Type:"		},
  { TYPE_STRING,	&touch_controls_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_touch_virtual_buttons_0[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseTouchControls,	"Touch Control Type:"		},
  { TYPE_STRING,	&touch_controls_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ENTER_LIST,	execSetupChooseGridXSize_0,	"Horizontal Buttons (Landscape):" },
  { TYPE_STRING,	&grid_size_text[0][0],		""				},
  { TYPE_ENTER_LIST,	execSetupChooseGridYSize_0,	"Vertical Buttons (Landscape):"	},
  { TYPE_STRING,	&grid_size_text[0][1],		""				},
  { TYPE_ENTER_LIST,	execSetupChooseTransparency,	"Button Transparency:"		},
  { TYPE_STRING,	&transparency_text,		""				},
  { TYPE_SWITCH,	&setup.touch.draw_outlined,	"Draw Buttons Outlined:"	},
  { TYPE_SWITCH,	&setup.touch.draw_pressed,	"Highlight Pressed Buttons:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ENTER_LIST,	execSetupConfigureVirtualButtons, "Configure Virtual Buttons"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_touch_virtual_buttons_1[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseTouchControls,	"Touch Control Type:"		},
  { TYPE_STRING,	&touch_controls_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ENTER_LIST,	execSetupChooseGridXSize_1,	"Horizontal Buttons (Portrait):" },
  { TYPE_STRING,	&grid_size_text[1][0],		""				},
  { TYPE_ENTER_LIST,	execSetupChooseGridYSize_1,	"Vertical Buttons (Portrait):"	},
  { TYPE_STRING,	&grid_size_text[1][1],		""				},
  { TYPE_ENTER_LIST,	execSetupChooseTransparency,	"Button Transparency:"		},
  { TYPE_STRING,	&transparency_text,		""				},
  { TYPE_SWITCH,	&setup.touch.draw_outlined,	"Draw Buttons Outlined:"	},
  { TYPE_SWITCH,	&setup.touch.draw_pressed,	"Highlight Pressed Buttons:"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ENTER_LIST,	execSetupConfigureVirtualButtons, "Configure Virtual Buttons"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo *setup_info_touch_virtual_buttons[] =
{
  setup_info_touch_virtual_buttons_0,
  setup_info_touch_virtual_buttons_1
};

static struct TokenInfo setup_info_touch_wipe_gestures[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseTouchControls,	"Touch Control Type:"		},
  { TYPE_STRING,	&touch_controls_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_ENTER_LIST,	execSetupChooseMoveDistance,	"Move Trigger Distance:"	},
  { TYPE_STRING,	&move_distance_text,		""				},
  { TYPE_ENTER_LIST,	execSetupChooseDropDistance,	"Drop Trigger Distance:"	},
  { TYPE_STRING,	&drop_distance_text,		""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts[] =
{
  { TYPE_ENTER_MENU,	execSetupShortcuts1,		"Various Keys"			},
  { TYPE_ENTER_MENU,	execSetupShortcuts2,		"Player Focus"			},
  { TYPE_ENTER_MENU,	execSetupShortcuts3,		"Tape Buttons"			},
  { TYPE_ENTER_MENU,	execSetupShortcuts4,		"Sound & Music"			},
  { TYPE_ENTER_MENU,	execSetupShortcuts5,		"TAS Snap Keys"			},
  { TYPE_ENTER_MENU,	execSetupShortcuts6,		"Speed Keys"			},
  { TYPE_ENTER_MENU,	execSetupShortcuts7,		"Engine Keys"			},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupMain, 			"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_1[] =
{
  { TYPE_KEYTEXT,	NULL,				"Quick Save Game to Tape:"	},
  { TYPE_KEY,		&setup.shortcut.save_game,	""				},
  { TYPE_KEYTEXT,	NULL,				"Quick Load Game from Tape:"	},
  { TYPE_KEY,		&setup.shortcut.load_game,	""				},
  { TYPE_KEYTEXT,	NULL,				"Restart Game:"			},
  { TYPE_KEY,		&setup.shortcut.restart_game,	""				},
  { TYPE_KEYTEXT,	NULL,				"Replay & Pause Before End:"	},
  { TYPE_KEY,		&setup.shortcut.pause_before_end, ""				},
  { TYPE_KEYTEXT,	NULL,				"Start Game & Toggle Pause:"	},
  { TYPE_KEY,		&setup.shortcut.toggle_pause,	""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_YES_NO,	&setup.ask_on_escape,		"Ask on 'Esc' Key:"		},
  { TYPE_YES_NO, &setup.ask_on_escape_editor,		"Ask on 'Esc' Key (Editor):"	},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_2[] =
{
  { TYPE_KEYTEXT,	NULL,				"Set Focus to Player 1:"	},
  { TYPE_KEY,		&setup.shortcut.focus_player[0], ""				},
  { TYPE_KEYTEXT,	NULL,				"Set Focus to Player 2:"	},
  { TYPE_KEY,		&setup.shortcut.focus_player[1], ""				},
  { TYPE_KEYTEXT,	NULL,				"Set Focus to Player 3:"	},
  { TYPE_KEY,		&setup.shortcut.focus_player[2], ""				},
  { TYPE_KEYTEXT,	NULL,				"Set Focus to Player 4:"	},
  { TYPE_KEY,		&setup.shortcut.focus_player[3], ""				},
  { TYPE_KEYTEXT,	NULL,				"Set Focus to All Players:"	},
  { TYPE_KEY,		&setup.shortcut.focus_player_all, ""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_3[] =
{
  { TYPE_KEYTEXT,	NULL,				"Eject Tape:"			},
  { TYPE_KEY,		&setup.shortcut.tape_eject,	""				},
  { TYPE_KEYTEXT,	NULL,				"Warp / Single Step:"		},
  { TYPE_KEY,		&setup.shortcut.tape_extra,	""				},
  { TYPE_KEYTEXT,	NULL,				"Stop Tape:"			},
  { TYPE_KEY,		&setup.shortcut.tape_stop,	""				},
  { TYPE_KEYTEXT,	NULL,				"Pause / Unpause Tape:"		},
  { TYPE_KEY,		&setup.shortcut.tape_pause,	""				},
  { TYPE_KEYTEXT,	NULL,				"Record Tape:"			},
  { TYPE_KEY,		&setup.shortcut.tape_record,	""				},
  { TYPE_KEYTEXT,	NULL,				"Play Tape:"			},
  { TYPE_KEY,		&setup.shortcut.tape_play,	""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_4[] =
{
  { TYPE_KEYTEXT,	NULL,				"Toggle Sound Effects (Normal):" },
  { TYPE_KEY,		&setup.shortcut.sound_simple,	""				},
  { TYPE_KEYTEXT,	NULL,				"Toggle Sound Effects (Looping):" },
  { TYPE_KEY,		&setup.shortcut.sound_loops,	""				},
  { TYPE_KEYTEXT,	NULL,				"Toggle Music:"			},
  { TYPE_KEY,		&setup.shortcut.sound_music,	""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_5[] =
{
  { TYPE_KEYTEXT,	NULL,				"Snap Left:"			},
  { TYPE_KEY,		&setup.shortcut.snap_left,	""				},
  { TYPE_KEYTEXT,	NULL,				"Snap Right:"			},
  { TYPE_KEY,		&setup.shortcut.snap_right,	""				},
  { TYPE_KEYTEXT,	NULL,				"Snap Up:"			},
  { TYPE_KEY,		&setup.shortcut.snap_up,	""				},
  { TYPE_KEYTEXT,	NULL,				"Snap Down:"			},
  { TYPE_KEY,		&setup.shortcut.snap_down,	""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_6[] =
{
  { TYPE_KEYTEXT,	NULL,				"Fast Playing Speed:"		},
  { TYPE_KEY,		&setup.shortcut.speed_fast,	""				},
  { TYPE_KEYTEXT,	NULL,				"Slow Playing Speed:"		},
  { TYPE_KEY,		&setup.shortcut.speed_slow,	""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static struct TokenInfo setup_info_shortcuts_7[] =
{
  { TYPE_KEYTEXT,	NULL,				"Boulder Dash Suicide Key:"	},
  { TYPE_KEY,		&setup.shortcut.suicide,	""				},
  { TYPE_KEYTEXT,	NULL,				"Toggle Extra Panel Items:"	},
  { TYPE_KEY,		&setup.shortcut.toggle_panel,	""				},
  { TYPE_EMPTY,		NULL,				""				},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,		"Back"				},

  { 0,			NULL,				NULL				}
};

static Key getSetupKey(void)
{
  Key key = KSYM_UNDEFINED;
  boolean got_key_event = FALSE;

  while (!got_key_event)
  {
    Event event;

    if (NextValidEvent(&event))
    {
      switch (event.type)
      {
        case EVENT_KEYPRESS:
	  {
	    key = GetEventKey((KeyEvent *)&event);

	    // press 'Escape' or 'Enter' to keep the existing key binding
	    if (key == KSYM_Escape || key == KSYM_Return)
	      key = KSYM_UNDEFINED;	// keep old value

	    got_key_event = TRUE;
	  }
	  break;

        case EVENT_KEYRELEASE:
	  key_joystick_mapping = 0;
	  break;

        default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

    BackToFront();
  }

  return key;
}

static int getSetupValueFont(int type, void *value)
{
  if (type & TYPE_GHOSTED)
    return FONT_OPTION_OFF;
  else if (type & TYPE_KEY)
    return ((type & TYPE_QUERY) ? FONT_INPUT_1_ACTIVE : FONT_VALUE_1);
  else if (type & TYPE_STRING)
    return FONT_VALUE_2;
  else if (type & TYPE_ECS_AGA)
    return FONT_VALUE_1;
  else if (type & TYPE_BOOLEAN_STYLE)
    return (*(boolean *)value ? FONT_OPTION_ON : FONT_OPTION_OFF);
  else if (type & TYPE_YES_NO_AUTO)
    return (*(int *)value == STATE_AUTO  ? FONT_OPTION_ON :
	    *(int *)value == STATE_FALSE ? FONT_OPTION_OFF : FONT_OPTION_ON);
  else if (type & TYPE_YES_NO_ASK)
    return (*(int *)value == STATE_ASK   ? FONT_OPTION_ON :
	    *(int *)value == STATE_FALSE ? FONT_OPTION_OFF : FONT_OPTION_ON);
  else if (type & TYPE_YES_NO_ONCE)
    return (*(int *)value == STATE_ONCE  ? FONT_OPTION_ON :
	    *(int *)value == STATE_FALSE ? FONT_OPTION_OFF : FONT_OPTION_ON);
  else if (type & TYPE_PLAYER)
    return FONT_VALUE_1;
  else
    return FONT_VALUE_1;
}

static int getSetupValueFontNarrow(int type, int font_nr)
{
  return (font_nr == FONT_VALUE_1    ? FONT_VALUE_NARROW :
	  font_nr == FONT_OPTION_ON  ? FONT_OPTION_ON_NARROW :
	  font_nr == FONT_OPTION_OFF ? FONT_OPTION_OFF_NARROW :
	  font_nr);
}

static void drawSetupValue(int screen_pos, int setup_info_pos_raw)
{
  int si_pos = (setup_info_pos_raw < 0 ? screen_pos : setup_info_pos_raw);
  struct TokenInfo *si = &setup_info[si_pos];
  boolean font_draw_xoffset_modified = FALSE;
  boolean scrollbar_needed = (num_setup_info < max_setup_info);
  int mx_scrollbar = screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->x;
  int mx_right_border = (scrollbar_needed ? mx_scrollbar : SX + SXSIZE);
  int font_draw_xoffset_old = -1;
  int xoffset = (scrollbar_needed ? 0 : 1);
  int menu_screen_value_xpos = MENU_SCREEN_VALUE_XPOS + xoffset;
  int xpos = menu_screen_value_xpos;
  int ypos = MENU_SCREEN_START_YPOS + screen_pos;
  int startx = mSX + xpos * 32;
  int starty = mSY + ypos * 32;
  int type = si->type;
  void *value = si->value;
  char *value_string = getSetupValue(type, value);
  int font_nr_default = getSetupValueFont(type, value);
  int font_width_default = getFontWidth(font_nr_default);
  int font_nr = font_nr_default;

  if (value_string == NULL)
    return;

  if (type & TYPE_KEY)
  {
    xpos = MENU_SCREEN_START_XPOS;

    if (type & TYPE_QUERY)
      value_string = "<press key>";
  }
  else if (type & TYPE_STRING)
  {
    int max_value_len = (SXSIZE - 2 * TILEX) / font_width_default;

    xpos = MENU_SCREEN_START_XPOS;

    if (strlen(value_string) > max_value_len)
      value_string[max_value_len] = '\0';
  }
  else if (type & TYPE_PLAYER)
  {
    int displayed_player_nr = *(int *)value + 1;

    value_string = getSetupValue(TYPE_INTEGER, (void *)&displayed_player_nr);
  }

  startx = mSX + xpos * 32;
  starty = mSY + ypos * 32;

  // always use narrow font for setup values on right screen side
  if (xpos > MENU_SCREEN_START_XPOS)
    font_nr = getSetupValueFontNarrow(type, font_nr);

  // downward compatibility correction for Juergen Bonhagen's menu settings
  if (setup_mode != SETUP_MODE_INPUT)
  {
    int max_menu_text_length_big = (menu_screen_value_xpos -
				    MENU_SCREEN_START_XPOS);
    int max_menu_text_length_medium = max_menu_text_length_big * 2;
    int check_font_nr = FONT_OPTION_ON; // known font that needs correction
    int font1_xoffset = getFontDrawOffsetX(font_nr);
    int font2_xoffset = getFontDrawOffsetX(check_font_nr);
    int text_startx = mSX + MENU_SCREEN_START_XPOS * 32;
    int text_font_nr = getMenuTextFont(FONT_MENU_2);
    int text_font_xoffset = getFontDrawOffsetX(text_font_nr);
    int text_width = max_menu_text_length_medium * getFontWidth(text_font_nr);
    boolean correct_font_draw_xoffset = FALSE;

    if (xpos == MENU_SCREEN_START_XPOS &&
	startx + font1_xoffset < text_startx + text_font_xoffset)
      correct_font_draw_xoffset = TRUE;

    if (xpos == menu_screen_value_xpos &&
	startx + font2_xoffset < text_startx + text_width + text_font_xoffset)
      correct_font_draw_xoffset = TRUE;

    // check if setup value would overlap with setup text when printed
    // (this can happen for extreme/wrong values for font draw offset)
    if (correct_font_draw_xoffset)
    {
      font_draw_xoffset_old = getFontDrawOffsetX(font_nr);
      font_draw_xoffset_modified = TRUE;

      if (type & TYPE_KEY)
	getFontBitmapInfo(font_nr)->draw_xoffset += 2 * getFontWidth(font_nr);
      else if (!(type & TYPE_STRING))
	getFontBitmapInfo(font_nr)->draw_xoffset = text_font_xoffset + 20 -
	  max_menu_text_length_medium * (16 - getFontWidth(text_font_nr));
    }
  }

  DrawBackground(startx, starty, mx_right_border - startx, getFontHeight(font_nr));
  DrawText(startx, starty, value_string, font_nr);

  if (type & TYPE_PLAYER)
  {
    struct FontBitmapInfo *font = getFontBitmapInfo(font_nr);
    int player_nr = *(int *)value;
    int xoff = font->draw_xoffset + getFontWidth(font_nr);
    int yoff = font->draw_yoffset + (getFontHeight(font_nr) - TILEY) / 2;
    int startx2 = startx + xoff;
    int starty2 = starty + yoff;

    if (DrawingOnBackground(startx2, starty2))
      ClearRectangleOnBackground(drawto, startx2, starty2, TILEX, TILEY);

    DrawFixedGraphicThruMaskExt(drawto, startx2, starty2,
				PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0);
  }

  if (font_draw_xoffset_modified)
    getFontBitmapInfo(font_nr)->draw_xoffset = font_draw_xoffset_old;
}

static void changeSetupValue(int screen_pos, int setup_info_pos_raw, int dx)
{
  int si_pos = (setup_info_pos_raw < 0 ? screen_pos : setup_info_pos_raw);
  struct TokenInfo *si = &setup_info[si_pos];

  if (si->type & TYPE_BOOLEAN_STYLE)
  {
    *(boolean *)si->value ^= TRUE;
  }
  else if (si->type & TYPE_YES_NO_AUTO)
  {
    *(int *)si->value =
      (dx == -1 ?
       (*(int *)si->value == STATE_AUTO ? STATE_TRUE :
	*(int *)si->value == STATE_TRUE ? STATE_FALSE : STATE_AUTO) :
       (*(int *)si->value == STATE_TRUE ? STATE_AUTO :
	*(int *)si->value == STATE_AUTO ? STATE_FALSE : STATE_TRUE));
  }
  else if (si->type & TYPE_YES_NO_ASK)
  {
    *(int *)si->value =
      (dx == -1 ?
       (*(int *)si->value == STATE_ASK  ? STATE_TRUE :
	*(int *)si->value == STATE_TRUE ? STATE_FALSE : STATE_ASK) :
       (*(int *)si->value == STATE_TRUE ? STATE_ASK :
	*(int *)si->value == STATE_ASK  ? STATE_FALSE : STATE_TRUE));
  }
  else if (si->type & TYPE_YES_NO_ONCE)
  {
    *(int *)si->value =
      (dx == -1 ?
       (*(int *)si->value == STATE_ONCE ? STATE_TRUE :
	*(int *)si->value == STATE_TRUE ? STATE_FALSE : STATE_ONCE) :
       (*(int *)si->value == STATE_TRUE ? STATE_ONCE :
	*(int *)si->value == STATE_ONCE ? STATE_FALSE : STATE_TRUE));
  }
  else if (si->type & TYPE_KEY)
  {
    Key key;

    si->type |= TYPE_QUERY;
    drawSetupValue(screen_pos, setup_info_pos_raw);
    si->type &= ~TYPE_QUERY;

    key = getSetupKey();
    if (key != KSYM_UNDEFINED)
      *(Key *)si->value = key;
  }
  else if (si->type & TYPE_PLAYER)
  {
    int player_nr = *(int *)si->value;

    if (dx)
      player_nr += dx;
    else
      player_nr = Request("Choose player", REQ_PLAYER) - 1;

    *(int *)si->value = MIN(MAX(0, player_nr), MAX_PLAYERS - 1);
  }

  drawSetupValue(screen_pos, setup_info_pos_raw);

  // fullscreen state may have changed at this point
  if (si->value == &setup.fullscreen)
    ToggleFullscreenIfNeeded();

  // audio sample rate may have changed at this point
  if (si->value == &setup.audio_sample_rate_44100)
    ToggleAudioSampleRateIfNeeded();

  // network mode may have changed at this point
  if (si->value == &setup.network_mode)
    ToggleNetworkModeIfNeeded();

  // API server mode may have changed at this point
  if (si->value == &setup.use_api_server)
    ToggleUseApiServerIfNeeded();

  // game speed list may have changed at this point
  if (si->value == &setup.game_speed_extended)
    ToggleGameSpeedsListIfNeeded();

  // music state may have changed at this point
  if (si->value == &setup.sound_music)
    ToggleMenuMusicIfNeeded();

  // sound state may have changed at this point
  if (si->value == &setup.sound_simple ||
      si->value == &setup.sound_loops)
    ToggleMenuSoundsIfNeeded();

  // update old setup options from new setup options
  if (si->value == &setup.allow_skipping_levels)
    UpdateHandicapAndSkipLevels();

  SaveSetupIfNeeded();
}

static struct TokenInfo *getSetupInfoFinal(struct TokenInfo *setup_info_orig)
{
  static struct TokenInfo *setup_info_final = NULL;
  int list_size = 0;
  int list_pos = 0;
  int i;

  // determine maximum list size of target list
  while (setup_info_orig[list_size++].type != 0);

  // free, allocate and clear memory for target list
  checked_free(setup_info_final);
  setup_info_final = checked_calloc(list_size * sizeof(struct TokenInfo));

  // copy setup info list without setup entries marked as hidden
  for (i = 0; setup_info_orig[i].type != 0; i++)
  {
    // skip setup entries configured to be hidden
    if (hideSetupEntry(setup_info_orig[i].value))
      continue;

    // skip skippable setup entries if screen is lower than usual
    if (SCR_FIELDY < SCR_FIELDY_DEFAULT &&
	setup_info_orig[i].type == TYPE_SKIPPABLE)
      continue;

    setup_info_final[list_pos++] = setup_info_orig[i];
  }

  return setup_info_final;
}

static void DrawSetupScreen_Generic(void)
{
  int fade_mask = REDRAW_FIELD;
  boolean redraw_all = FALSE;
  char *title_string = NULL;
  int i;

  if (CheckFadeAll())
    fade_mask = REDRAW_ALL;

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  FreeScreenGadgets();
  CreateScreenGadgets();

  if (redraw_mask & REDRAW_ALL)
    redraw_all = TRUE;

  FadeOut(fade_mask);

  // needed if different viewport properties defined for setup screen
  ChangeViewportPropertiesIfNeeded();

  SetMainBackgroundImage(IMG_BACKGROUND_SETUP);

  ClearField();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  if (setup_mode == SETUP_MODE_MAIN)
  {
    setup_info = setup_info_main;
    title_string = STR_SETUP_MAIN;

    if (!setup.show_menu_to_save_setup)
      setHideSetupEntry(execSaveAndExitSetup);
    else
      removeHideSetupEntry(execSaveAndExitSetup);
  }
  else if (setup_mode == SETUP_MODE_GAME)
  {
    setup_info = setup_info_game;
    title_string = STR_SETUP_GAME;
  }
  else if (setup_mode == SETUP_MODE_ENGINES)
  {
    setup_info = setup_info_engines;
    title_string = STR_SETUP_ENGINES;
  }
  else if (setup_mode == SETUP_MODE_EDITOR)
  {
    setup_info = setup_info_editor;
    title_string = STR_SETUP_EDITOR;
  }
  else if (setup_mode == SETUP_MODE_GRAPHICS)
  {
    setup_info = setup_info_graphics;
    title_string = STR_SETUP_GRAPHICS;
  }
  else if (setup_mode == SETUP_MODE_SOUND)
  {
    setup_info = setup_info_sound;
    title_string = STR_SETUP_SOUND;
  }
  else if (setup_mode == SETUP_MODE_ARTWORK)
  {
    setup_info = setup_info_artwork;
    title_string = STR_SETUP_ARTWORK;
  }
  else if (setup_mode == SETUP_MODE_TOUCH)
  {
    setup_info = setup_info_touch;
    title_string = STR_SETUP_TOUCH;

    if (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS))
      setup_info = setup_info_touch_virtual_buttons[GRID_ACTIVE_NR()];
    else if (strEqual(setup.touch.control_type, TOUCH_CONTROL_WIPE_GESTURES))
      setup_info = setup_info_touch_wipe_gestures;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS)
  {
    setup_info = setup_info_shortcuts;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_1)
  {
    setup_info = setup_info_shortcuts_1;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_2)
  {
    setup_info = setup_info_shortcuts_2;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_3)
  {
    setup_info = setup_info_shortcuts_3;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_4)
  {
    setup_info = setup_info_shortcuts_4;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_5)
  {
    setup_info = setup_info_shortcuts_5;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_6)
  {
    setup_info = setup_info_shortcuts_6;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_7)
  {
    setup_info = setup_info_shortcuts_7;
    title_string = STR_SETUP_SHORTCUTS;
  }

  // use modified setup info without setup entries marked as hidden
  setup_info = getSetupInfoFinal(setup_info);

  DrawTextSCentered(MENU_TITLE_YPOS, FONT_TITLE_1, title_string);

  // determine maximal number of setup entries that can be displayed on screen
  num_setup_info = 0;
  for (i = 0; setup_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
    num_setup_info++;

  // determine maximal number of setup entries available for this setup screen
  max_setup_info = 0;
  for (i = 0; setup_info[i].type != 0; i++)
    max_setup_info++;

  HandleSetupScreen_Generic(0, 0, 0, 0, MB_MENU_INITIALIZE);

  MapScreenGadgets(max_setup_info);

  if (redraw_all)
    redraw_mask = fade_mask = REDRAW_ALL;

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
}

void HandleSetupScreen_Generic(int mx, int my, int dx, int dy, int button)
{
  menu_info = setup_info;

  HandleMenuScreen(mx, my, dx, dy, button,
		   setup_mode, num_setup_info, max_setup_info);
}

static void DrawSetupScreen_Input(void)
{
  int i;

  FadeOut(REDRAW_FIELD);

  ClearField();

  setup_info = getSetupInfoFinal(setup_info_input);

  DrawTextSCentered(MENU_TITLE_YPOS, FONT_TITLE_1, STR_SETUP_INPUT);

  for (i = 0; setup_info[i].type != 0; i++)
  {
    if (setup_info[i].type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (setup_info[i].type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (setup_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

    DrawCursorAndText_Setup(i, -1, FALSE);
  }

  // create gadgets for setup input menu screen
  FreeScreenGadgets();
  CreateScreenGadgets();

  // map gadgets for setup input menu screen
  MapScreenMenuGadgets(SCREEN_MASK_INPUT);

  HandleSetupScreen_Input(0, 0, 0, 0, MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

static void setJoystickDeviceToNr(char *device_name, int device_nr)
{
  if (device_name == NULL)
    return;

  if (device_nr < 0 || device_nr >= MAX_PLAYERS)
    device_nr = 0;

  if (strlen(device_name) > 1)
  {
    char c1 = device_name[strlen(device_name) - 1];
    char c2 = device_name[strlen(device_name) - 2];

    if (c1 >= '0' && c1 <= '9' && !(c2 >= '0' && c2 <= '9'))
      device_name[strlen(device_name) - 1] = '0' + (char)(device_nr % 10);
  }
  else
    strncpy(device_name, getDeviceNameFromJoystickNr(device_nr),
	    strlen(device_name));
}

static void drawPlayerSetupInputInfo(int player_nr, boolean active)
{
  int i;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    Key *key;
    char *text;
  } custom[] =
  {
    { &custom_key.left,  "Axis/Pad Left"  },
    { &custom_key.right, "Axis/Pad Right" },
    { &custom_key.up,    "Axis/Pad Up"    },
    { &custom_key.down,  "Axis/Pad Down"  },
    { &custom_key.snap,  "Button 1/A/X"   },
    { &custom_key.drop,  "Button 2/B/Y"   }
  };
  static char *joystick_name[MAX_PLAYERS] =
  {
    "Joystick1",
    "Joystick2",
    "Joystick3",
    "Joystick4"
  };
  int font_nr_menu = (active ? FONT_MENU_1_ACTIVE : FONT_MENU_1);
  int font_nr_info = FONT_MENU_1;
  int font_nr_name = FONT_VALUE_OLD;
  int font_nr_on   = FONT_VALUE_1;
  int font_nr_off  = FONT_VALUE_OLD;
  int pos = 4;

  if (SCR_FIELDX < SCR_FIELDX_DEFAULT)
  {
    font_nr_info = FONT_MENU_2;
    font_nr_on   = FONT_VALUE_NARROW;
    font_nr_off  = FONT_VALUE_OLD_NARROW;
  }

  custom_key = setup.input[player_nr].key;

  DrawText(mSX + 11 * 32, mSY + 2 * 32, int2str(player_nr + 1, 1),
	   FONT_INPUT_1_ACTIVE);

  ClearRectangleOnBackground(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			     TILEX, TILEY);
  DrawFixedGraphicThruMaskExt(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			      PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0);

  if (setup.input[player_nr].use_joystick)
  {
    char *device_name = setup.input[player_nr].joy.device_name;
    int joystick_nr = getJoystickNrFromDeviceName(device_name);
    boolean joystick_active = CheckJoystickOpened(joystick_nr);
    char *text = joystick_name[joystick_nr];
    int font_nr = (joystick_active ? font_nr_on : font_nr_off);

    DrawText(mSX + 8 * 32, mSY + 3 * 32, text, font_nr);
    DrawText(mSX + 32, mSY + 4 * 32, "Configure", font_nr_menu);
  }
  else
  {
    DrawText(mSX + 8 * 32, mSY + 3 * 32, "Keyboard ", font_nr_on);
    DrawText(mSX + 1 * 32, mSY + 4 * 32, "Customize", font_nr_menu);
  }

  if (SCR_FIELDY >= SCR_FIELDY_DEFAULT)
    DrawText(mSX + 32, mSY + 5 * 32, "Actual Settings:", font_nr_info);
  else
    pos = 3;

  drawCursorXY(1, pos + 0, IMG_MENU_BUTTON_LEFT);
  drawCursorXY(1, pos + 1, IMG_MENU_BUTTON_RIGHT);
  drawCursorXY(1, pos + 2, IMG_MENU_BUTTON_UP);
  drawCursorXY(1, pos + 3, IMG_MENU_BUTTON_DOWN);

  DrawText(mSX + 2 * 32, mSY + (pos + 2) * 32, ":", font_nr_name);
  DrawText(mSX + 2 * 32, mSY + (pos + 3) * 32, ":", font_nr_name);
  DrawText(mSX + 2 * 32, mSY + (pos + 4) * 32, ":", font_nr_name);
  DrawText(mSX + 2 * 32, mSY + (pos + 5) * 32, ":", font_nr_name);
  DrawText(mSX + 1 * 32, mSY + (pos + 6) * 32, "Snap Field:", font_nr_name);
  DrawText(mSX + 1 * 32, mSY + (pos + 8) * 32, "Drop Element:", font_nr_name);

  for (i = 0; i < 6; i++)
  {
    int ypos = (pos + 2) + i + (i > 3 ? i - 3 : 0);

    DrawText(mSX + 3 * 32, mSY + ypos * 32,
	     "              ", font_nr_on);
    DrawText(mSX + 3 * 32, mSY + ypos * 32,
	     (setup.input[player_nr].use_joystick ?
	      custom[i].text :
	      getKeyNameFromKey(*custom[i].key)), font_nr_on);
  }
}

static int input_player_nr = 0;

static void HandleSetupScreen_Input_Player(int step, int direction)
{
  int old_player_nr = input_player_nr;
  int new_player_nr;

  new_player_nr = old_player_nr + step * direction;
  if (new_player_nr < 0)
    new_player_nr = 0;
  if (new_player_nr > MAX_PLAYERS - 1)
    new_player_nr = MAX_PLAYERS - 1;

  if (new_player_nr != old_player_nr)
  {
    input_player_nr = new_player_nr;

    drawPlayerSetupInputInfo(input_player_nr, FALSE);
  }
}

void HandleSetupScreen_Input(int mx, int my, int dx, int dy, int button)
{
  static int choice = 0;
  int x = 0;
  int y = choice;
  int pos_start  = SETUPINPUT_SCREEN_POS_START;
  int pos_empty1 = SETUPINPUT_SCREEN_POS_EMPTY1;
  int pos_empty2 = SETUPINPUT_SCREEN_POS_EMPTY2;
  int pos_end    = SETUPINPUT_SCREEN_POS_END;

  if (SCR_FIELDY < SCR_FIELDY_DEFAULT)
  {
    int i;

    for (i = 0; setup_info_input[i].type != 0; i++)
    {
      // adjust menu structure according to skipped setup entries
      if (setup_info_input[i].type == TYPE_SKIPPABLE)
      {
	pos_empty2--;
	pos_end--;
      }
    }
  }

  if (button == MB_MENU_INITIALIZE)
  {
    // input setup menu may have changed size due to graphics configuration
    if (choice >= pos_empty1)
      choice = pos_end;

    drawPlayerSetupInputInfo(input_player_nr, (choice == 2));

    DrawCursorAndText_Setup(choice, -1, TRUE);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    setup_mode = SETUP_MODE_MAIN;
    DrawSetupScreen();
    InitJoysticks();

    return;
  }

  if (mx || my)		// mouse input
  {
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	// keyboard input
  {
    if (dx && choice == 0)
      x = (dx < 0 ? 10 : 12);
    else if ((dx && choice == 1) ||
	     (dx == -1 && choice == pos_end))
      button = MB_MENU_CHOICE;
    else if (dy)
      y = choice + dy;

    if (y >= pos_empty1 && y <= pos_empty2)
      y = (dy > 0 ? pos_empty2 + 1 : pos_empty1 - 1);
  }

  if (y == 0 && dx != 0 && button)
  {
    HandleSetupScreen_Input_Player(1, dx < 0 ? -1 : +1);
  }
  else if (IN_VIS_FIELD(x, y) &&	// (does not use "IN_VIS_MENU()" yet)
	   y >= pos_start && y <= pos_end &&
	   !(y >= pos_empty1 && y <= pos_empty2))
  {
    if (button)
    {
      if (y != choice)
      {
	DrawCursorAndText_Setup(choice, -1, FALSE);
	DrawCursorAndText_Setup(y, -1, TRUE);

	drawPlayerSetupInputInfo(input_player_nr, (y == 2));

	choice = y;
      }
    }
    else
    {
      if (y == 1)
      {
	char *device_name = setup.input[input_player_nr].joy.device_name;

	if (!setup.input[input_player_nr].use_joystick)
	{
	  int new_device_nr = (dx >= 0 ? 0 : MAX_PLAYERS - 1);

	  setJoystickDeviceToNr(device_name, new_device_nr);
	  setup.input[input_player_nr].use_joystick = TRUE;
	}
	else
	{
	  int device_nr = getJoystickNrFromDeviceName(device_name);
	  int new_device_nr = device_nr + (dx >= 0 ? +1 : -1);

	  if (new_device_nr < 0 || new_device_nr >= MAX_PLAYERS)
	    setup.input[input_player_nr].use_joystick = FALSE;
	  else
	    setJoystickDeviceToNr(device_name, new_device_nr);
	}

	drawPlayerSetupInputInfo(input_player_nr, FALSE);
      }
      else if (y == 2)
      {
	if (setup.input[input_player_nr].use_joystick)
	  ConfigureJoystick(input_player_nr);
	else
	  CustomizeKeyboard(input_player_nr);
      }
      else if (y == pos_end)
      {
	InitJoysticks();

	FadeSetLeaveMenu();

	setup_mode = SETUP_MODE_MAIN;
	DrawSetupScreen();
      }
    }
  }
}

static boolean CustomizeKeyboardMain(int player_nr)
{
  int i;
  int step_nr;
  boolean finished = FALSE;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    Key *key;
    char *text;
  } customize_step[] =
  {
    { &custom_key.left,  "Move Left"	},
    { &custom_key.right, "Move Right"	},
    { &custom_key.up,    "Move Up"	},
    { &custom_key.down,  "Move Down"	},
    { &custom_key.snap,  "Snap Field"	},
    { &custom_key.drop,  "Drop Element"	}
  };
  int font_nr_old = FONT_VALUE_OLD;
  int font_nr_new = FONT_VALUE_1;
  boolean success = FALSE;

  if (SCR_FIELDX < SCR_FIELDX_DEFAULT)
  {
    font_nr_old = FONT_VALUE_OLD_NARROW;
    font_nr_new = FONT_VALUE_NARROW;
  }

  // read existing key bindings from player setup
  custom_key = setup.input[player_nr].key;

  FadeSetEnterMenu();
  FadeOut(REDRAW_FIELD);

  ClearField();

  DrawTextSCentered(MENU_TITLE_YPOS, FONT_TITLE_1, "Keyboard Input");

  step_nr = 0;
  DrawText(mSX, mSY + (2 + 2 * step_nr) * 32,
	   customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
  DrawText(mSX, mSY + (2 + 2 * step_nr + 1) * 32,
	   "Key:", FONT_INPUT_1_ACTIVE);
  DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
	   getKeyNameFromKey(*customize_step[step_nr].key), font_nr_old);

  FadeIn(REDRAW_FIELD);

  while (!finished)
  {
    Event event;
    DelayCounter event_frame_delay = { GAME_FRAME_DELAY };

    // reset frame delay counter directly after updating screen
    ResetDelayCounter(&event_frame_delay);

    while (NextValidEvent(&event))
    {
      switch (event.type)
      {
        case EVENT_KEYPRESS:
	  {
	    Key key = GetEventKey((KeyEvent *)&event);

	    // press 'Escape' to abort and keep the old key bindings
	    if (key == KSYM_Escape)
	    {
	      FadeSkipNextFadeIn();

	      finished = TRUE;

	      break;
	    }

	    // press 'Enter' to keep the existing key binding
	    if (key == KSYM_Return)
	      key = *customize_step[step_nr].key;

	    // check if key already used
	    for (i = 0; i < step_nr; i++)
	      if (*customize_step[i].key == key)
		break;
	    if (i < step_nr)
	      break;

	    // got new key binding
	    *customize_step[step_nr].key = key;
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     "             ", font_nr_new);
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     getKeyNameFromKey(key), font_nr_new);
	    step_nr++;

	    // un-highlight last query
	    DrawText(mSX, mSY + (2 + 2 * (step_nr - 1)) * 32,
		     customize_step[step_nr - 1].text, FONT_MENU_1);
	    DrawText(mSX, mSY + (2 + 2 * (step_nr - 1) + 1) * 32,
		     "Key:", FONT_MENU_1);

	    // all keys configured
	    if (step_nr == 6)
	    {
	      finished = TRUE;
	      success = TRUE;

	      break;
	    }

	    // query next key binding
	    DrawText(mSX, mSY + (2 + 2 * step_nr) * 32,
		     customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
	    DrawText(mSX, mSY + (2 + 2 * step_nr + 1) * 32,
		     "Key:", FONT_INPUT_1_ACTIVE);
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     getKeyNameFromKey(*customize_step[step_nr].key),
		     font_nr_old);
	  }
	  break;

        case EVENT_KEYRELEASE:
	  key_joystick_mapping = 0;
	  break;

        default:
	  HandleOtherEvents(&event);
	  break;
      }

      // do not handle events for longer than standard frame delay period
      if (DelayReached(&event_frame_delay))
	break;
    }

    BackToFront();
  }

  // write new key bindings back to player setup, if successfully finished
  if (success)
    setup.input[player_nr].key = custom_key;

  return success;
}

void CustomizeKeyboard(int player_nr)
{
  boolean success = CustomizeKeyboardMain(player_nr);

  if (success)
  {
    int font_nr = FONT_TITLE_1;
    int font_height = getFontHeight(font_nr);
    int ypos1 = SYSIZE / 2 - font_height * 2;
    int ypos2 = SYSIZE / 2 - font_height * 1;
    DelayCounter wait_frame_delay = { 2000 };

    ResetDelayCounter(&wait_frame_delay);

    ClearField();

    DrawTextSCentered(ypos1, font_nr, "Keyboard");
    DrawTextSCentered(ypos2, font_nr, "configured!");

    while (!DelayReached(&wait_frame_delay))
      BackToFront();

    ClearEventQueue();
  }

  DrawSetupScreen_Input();
}

// game controller mapping generator by Gabriel Jacobo <gabomdq@gmail.com>

#define MARKER_BUTTON		1
#define MARKER_AXIS_X		2
#define MARKER_AXIS_Y		3

static boolean ConfigureJoystickMapButtonsAndAxes(SDL_Joystick *joystick)
{
  static boolean bitmaps_initialized = FALSE;
  boolean screen_initialized = FALSE;
  static Bitmap *controller, *button, *axis_x, *axis_y;
  char *name;
  boolean success = TRUE;
  boolean done = FALSE, next = FALSE;
  Event event;
  int alpha = 200, alpha_step = -1;
  int alpha_ticks = 0;
  char mapping[4096], temp[256];
  int font_name = MENU_SETUP_FONT_TITLE;
  int font_info = MENU_SETUP_FONT_TEXT;
  int spacing_name = menu.line_spacing_setup[SETUP_MODE_INPUT];
  int spacing_line = menu.line_spacing_setup[SETUP_MODE_INPUT];
  int spacing_para = menu.paragraph_spacing_setup[SETUP_MODE_INPUT];
  int ystep_name = getMenuTextStep(spacing_name, font_name);
  int ystep_line = getMenuTextStep(spacing_line, font_info);
  int ystep_para = getMenuTextStep(spacing_para, font_info);
  int i, j;

  struct
  {
    int x, y;
    int marker;
    char *field;
    int axis, button, hat, hat_value;
    char mapping[4096];
  }
  *step, *prev_step, steps[] =
  {
    { 356, 155, MARKER_BUTTON, "a",		},
    { 396, 122, MARKER_BUTTON, "b",		},
    { 320, 125, MARKER_BUTTON, "x",		},
    { 358,  95, MARKER_BUTTON, "y",		},
    { 162, 125, MARKER_BUTTON, "back",		},
    { 216, 125, MARKER_BUTTON, "guide",		},
    { 271, 125, MARKER_BUTTON, "start",		},
    { 110, 200, MARKER_BUTTON, "dpleft",	},
    { 146, 228, MARKER_BUTTON, "dpdown",	},
    { 178, 200, MARKER_BUTTON, "dpright",	},
    { 146, 172, MARKER_BUTTON, "dpup",		},
    {  50,  40, MARKER_BUTTON, "leftshoulder",	},
    {  88, -10, MARKER_AXIS_Y, "lefttrigger",	},
    { 382,  40, MARKER_BUTTON, "rightshoulder",	},
    { 346, -10, MARKER_AXIS_Y, "righttrigger",	},
    {  73, 141, MARKER_BUTTON, "leftstick",	},
    { 282, 210, MARKER_BUTTON, "rightstick",	},
    {  73, 141, MARKER_AXIS_X, "leftx",		},
    {  73, 141, MARKER_AXIS_Y, "lefty",		},
    { 282, 210, MARKER_AXIS_X, "rightx",	},
    { 282, 210, MARKER_AXIS_Y, "righty",	},
  };

  if (!bitmaps_initialized)
  {
    controller = LoadCustomImage("joystick/controller.png");
    button     = LoadCustomImage("joystick/button.png");
    axis_x     = LoadCustomImage("joystick/axis_x.png");
    axis_y     = LoadCustomImage("joystick/axis_y.png");

    bitmaps_initialized = TRUE;
  }

  name = getFormattedJoystickName(SDL_JoystickName(joystick));

#if DEBUG_JOYSTICKS
  // print info about the joystick we are watching
  Debug("joystick", "watching joystick %d: (%s)",
	SDL_JoystickInstanceID(joystick), name);
  Debug("joystick", "joystick has %d axes, %d hats, %d balls, and %d buttons",
	SDL_JoystickNumAxes(joystick), SDL_JoystickNumHats(joystick),
	SDL_JoystickNumBalls(joystick), SDL_JoystickNumButtons(joystick));
#endif

  // initialize mapping with GUID and name
  SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), temp, sizeof(temp));

  snprintf(mapping, sizeof(mapping), "%s,%s,platform:%s,",
	   temp, name, SDL_GetPlatform());

  // loop through all steps (buttons and axes), getting joystick events
  for (i = 0; i < SDL_arraysize(steps) && !done;)
  {
    Bitmap *marker = button;	// initialize with reliable default value

    step = &steps[i];
    strcpy(step->mapping, mapping);
    step->axis = -1;
    step->button = -1;
    step->hat = -1;
    step->hat_value = -1;

    marker = (step->marker == MARKER_BUTTON ? button :
	      step->marker == MARKER_AXIS_X ? axis_x :
	      step->marker == MARKER_AXIS_Y ? axis_y : marker);

    next = FALSE;

    while (!done && !next)
    {
      alpha += alpha_step * (int)(SDL_GetTicks() - alpha_ticks) / 5;
      alpha_ticks = SDL_GetTicks();

      if (alpha >= 255)
      {
	alpha = 255;
	alpha_step = -1;
      }
      else if (alpha < 128)
      {
	alpha = 127;
	alpha_step = 1;
      }

      int controller_x = SX + (SXSIZE - controller->width) / 2;
      int controller_y = SY + ystep_line;

      int marker_x = controller_x + step->x;
      int marker_y = controller_y + step->y;

      int ystart1 = mSY - 2 * SY + controller_y + controller->height;
      int ystart2 = ystart1 + ystep_name + ystep_line;

      ClearField();

      DrawTextSCentered(ystart1, font_name, name);

      DrawTextSCentered(ystart2, font_info,
			"Press buttons and move axes on");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"your controller when indicated.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"(Your controller may look different.)");
      ystart2 += ystep_para;

#if defined(PLATFORM_ANDROID)
      DrawTextSCentered(ystart2, font_info,
			"To correct a mistake,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'back' button.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"To skip a button or axis,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'menu' button.");
#else
      DrawTextSCentered(ystart2, font_info,
			"To correct a mistake,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'backspace' key.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"To skip a button or axis,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'return' key.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"To exit, press the 'escape' key.");
#endif

      BlitBitmapMasked(controller, drawto, 0, 0,
		       controller->width, controller->height,
		       controller_x, controller_y);

      SDL_SetSurfaceBlendMode(marker->surface_masked, SDL_BLENDMODE_BLEND);
      SDL_SetSurfaceAlphaMod(marker->surface_masked, alpha);

      BlitBitmapMasked(marker, drawto, 0, 0,
		       marker->width, marker->height,
		       marker_x, marker_y);

      if (!screen_initialized)
	FadeIn(REDRAW_FIELD);
      else
	BackToFront();

      screen_initialized = TRUE;

      DelayCounter event_frame_delay = { GAME_FRAME_DELAY };

      // reset frame delay counter directly after updating screen
      ResetDelayCounter(&event_frame_delay);

      while (NextValidEvent(&event))
      {
	switch (event.type)
	{
	  case SDL_JOYAXISMOTION:
	    if (event.jaxis.value > 20000 ||
		event.jaxis.value < -20000)
	    {
	      for (j = 0; j < i; j++)
		if (steps[j].axis == event.jaxis.axis)
		  break;

	      if (j == i)
	      {
		if (step->marker != MARKER_AXIS_X &&
		    step->marker != MARKER_AXIS_Y)
		  break;

		step->axis = event.jaxis.axis;
		strcat(mapping, step->field);
		snprintf(temp, sizeof(temp), ":a%u,", event.jaxis.axis);
		strcat(mapping, temp);
		i++;
		next = TRUE;
	      }
	    }

	    break;

	  case SDL_JOYHATMOTION:
	    // ignore centering; we're probably just coming back
	    // to the center from the previous item we set
	    if (event.jhat.value == SDL_HAT_CENTERED)
	      break;

	    for (j = 0; j < i; j++)
	      if (steps[j].hat == event.jhat.hat &&
		  steps[j].hat_value == event.jhat.value)
		break;

	    if (j == i)
	    {
	      step->hat = event.jhat.hat;
	      step->hat_value = event.jhat.value;
	      strcat(mapping, step->field);
	      snprintf(temp, sizeof(temp), ":h%u.%u,",
		       event.jhat.hat, event.jhat.value );
	      strcat(mapping, temp);
	      i++;
	      next = TRUE;
	    }

	    break;

	  case SDL_JOYBALLMOTION:
	    break;

	  case SDL_JOYBUTTONUP:
	    for (j = 0; j < i; j++)
	      if (steps[j].button == event.jbutton.button)
		break;

	    if (j == i)
	    {
	      step->button = event.jbutton.button;
	      strcat(mapping, step->field);
	      snprintf(temp, sizeof(temp), ":b%u,", event.jbutton.button);
	      strcat(mapping, temp);
	      i++;
	      next = TRUE;
	    }

	    break;

	  case SDL_FINGERDOWN:
	  case SDL_MOUSEBUTTONDOWN:
	    // skip this step
	    i++;
	    next = TRUE;

	    break;

	  case SDL_KEYDOWN:
	    if (event.key.keysym.sym == KSYM_BackSpace ||
		event.key.keysym.sym == KSYM_Back)
	    {
	      if (i == 0)
	      {
		// leave screen
		success = FALSE;
		done = TRUE;

		break;
	      }

	      // undo this step
	      prev_step = &steps[i - 1];
	      strcpy(mapping, prev_step->mapping);
	      i--;
	      next = TRUE;

	      break;
	    }

	    if (event.key.keysym.sym == KSYM_space ||
		event.key.keysym.sym == KSYM_Return ||
		event.key.keysym.sym == KSYM_Menu)
	    {
	      // skip this step
	      i++;
	      next = TRUE;

	      break;
	    }

	    if (event.key.keysym.sym == KSYM_Escape)
	    {
	      // leave screen
	      success = FALSE;
	      done = TRUE;
	    }

	    break;

	  case SDL_QUIT:
	    program.exit_function(0);
	    break;

	  default:
	    break;
	}

	// do not handle events for longer than standard frame delay period
	if (DelayReached(&event_frame_delay))
	  break;
      }
    }
  }

  if (success)
  {
#if DEBUG_JOYSTICKS
    Debug("joystick", "New game controller mapping:\n\n%s\n\n", mapping);
#endif

    // activate mapping for this game
    SDL_GameControllerAddMapping(mapping);

    // save mapping to personal mappings
    SaveSetup_AddGameControllerMapping(mapping);
  }

  // wait until the last pending event was removed from event queue
  while (NextValidEvent(&event));

  return success;
}

static int ConfigureJoystickMain(int player_nr)
{
  char *device_name = setup.input[player_nr].joy.device_name;
  int joystick_nr = getJoystickNrFromDeviceName(device_name);
  boolean joystick_active = CheckJoystickOpened(joystick_nr);
  int success = FALSE;
  int i;

  if (joystick.status == JOYSTICK_NOT_AVAILABLE)
    return JOYSTICK_NOT_AVAILABLE;

  if (!joystick_active || !setup.input[player_nr].use_joystick)
    return JOYSTICK_NOT_AVAILABLE;

  FadeSetEnterMenu();
  FadeOut(REDRAW_FIELD);

  // close all joystick devices (potentially opened as game controllers)
  for (i = 0; i < SDL_NumJoysticks(); i++)
    SDLCloseJoystick(i);

  // open joystick device as plain joystick to configure as game controller
  SDL_Joystick *joystick = SDL_JoystickOpen(joystick_nr);

  // as the joystick was successfully opened before, this should not happen
  if (joystick == NULL)
    return FALSE;

  // create new game controller mapping (buttons and axes) for joystick device
  success = ConfigureJoystickMapButtonsAndAxes(joystick);

  // close joystick (and maybe re-open as configured game controller later)
  SDL_JoystickClose(joystick);

  // re-open all joystick devices (potentially as game controllers)
  for (i = 0; i < SDL_NumJoysticks(); i++)
    SDLOpenJoystick(i);

  // clear all joystick input actions for all joystick devices
  SDLClearJoystickState();

  return (success ? JOYSTICK_CONFIGURED : JOYSTICK_NOT_CONFIGURED);
}

void ConfigureJoystick(int player_nr)
{
  boolean state = ConfigureJoystickMain(player_nr);

  if (state != JOYSTICK_NOT_CONFIGURED)
  {
    boolean success = (state == JOYSTICK_CONFIGURED);
    char message1[MAX_OUTPUT_LINESIZE + 1];
    char *message2 = (success ? "configured!" : "not available!");
    char *device_name = setup.input[player_nr].joy.device_name;
    int nr = getJoystickNrFromDeviceName(device_name) + 1;
    int font_nr = FONT_TITLE_1;
    int font_height = getFontHeight(font_nr);
    int ypos1 = SYSIZE / 2 - font_height * 2;
    int ypos2 = SYSIZE / 2 - font_height * 1;
    DelayCounter wait_frame_delay = { 2000 };

    ResetDelayCounter(&wait_frame_delay);

    ClearField();

    sprintf(message1, "Joystick %d", nr);

    DrawTextSCentered(ypos1, font_nr, message1);
    DrawTextSCentered(ypos2, font_nr, message2);

    while (!DelayReached(&wait_frame_delay))
      BackToFront();

    ClearEventQueue();
  }

  DrawSetupScreen_Input();
}

static void MapScreenMenuGadgets_OverlayTouchButtons(int y)
{
  if (y < video.screen_height / 3)
  {
    // remap touch gadgets to access upper part of the screen
    UnmapScreenMenuGadgets(SCREEN_MASK_TOUCH);
    MapScreenMenuGadgets(SCREEN_MASK_TOUCH2);
  }
  else if (y > 2 * video.screen_height / 3)
  {
    // remap touch gadgets to access lower part of the screen
    MapScreenMenuGadgets(SCREEN_MASK_TOUCH);
    UnmapScreenMenuGadgets(SCREEN_MASK_TOUCH2);
  }
}

static boolean ConfigureVirtualButtonsMain(void)
{
  static char *customize_step_text[] =
  {
    "Move Left",
    "Move Right",
    "Move Up",
    "Move Down",
    "Snap Field",
    "Drop Element"
  };
  char grid_button[] =
  {
    CHAR_GRID_BUTTON_LEFT,
    CHAR_GRID_BUTTON_RIGHT,
    CHAR_GRID_BUTTON_UP,
    CHAR_GRID_BUTTON_DOWN,
    CHAR_GRID_BUTTON_SNAP,
    CHAR_GRID_BUTTON_DROP
  };
  enum
  {
    ACTION_NONE,
    ACTION_ESCAPE,
    ACTION_BACK,
    ACTION_NEXT
  };
  int font_nr = FONT_INPUT_1_ACTIVE;
  int font_height = getFontHeight(font_nr);
  int ypos1 = SYSIZE / 2 - font_height * 2;
  int ypos2 = SYSIZE / 2 - font_height * 1;
  boolean success = FALSE;
  boolean finished = FALSE;
  int step_nr = 0;
  char grid_button_draw = CHAR_GRID_BUTTON_NONE;
  char grid_button_old[MAX_GRID_XSIZE][MAX_GRID_YSIZE];
  char grid_button_tmp[MAX_GRID_XSIZE][MAX_GRID_YSIZE];
  boolean set_grid_button = FALSE;
  int nr = GRID_ACTIVE_NR();
  int x, y;

  for (x = 0; x < MAX_GRID_XSIZE; x++)
    for (y = 0; y < MAX_GRID_YSIZE; y++)
      grid_button_old[x][y] = grid_button_tmp[x][y] = overlay.grid_button[x][y];

  overlay.grid_button_highlight = grid_button[step_nr];

  UnmapAllGadgets();

  FadeSetEnterMenu();
  FadeOut(REDRAW_FIELD);

  ClearField();

  DrawTextSCentered(MENU_TITLE_YPOS, FONT_TITLE_1, "Virtual Buttons");
  DrawTextSCentered(ypos1, font_nr, "Select tiles to");
  DrawTextSCentered(ypos2, font_nr, customize_step_text[step_nr]);

  FadeIn(REDRAW_FIELD);

  SetOverlayShowGrid(TRUE);

  // map gadgets for setup touch buttons menu screen
  MapScreenMenuGadgets(SCREEN_MASK_TOUCH);

  while (!finished)
  {
    Event event;

    while (NextValidEvent(&event))
    {
      int action = ACTION_NONE;

      // ---------- handle events and set the resulting action ----------

      switch (event.type)
      {
	case EVENT_USER:
	  {
	    UserEvent *user = (UserEvent *)&event;
	    int id = user->value1;

	    action = (id == SCREEN_CTRL_ID_TOUCH_PREV_PAGE ||
		      id == SCREEN_CTRL_ID_TOUCH_PREV_PAGE2 ? ACTION_BACK :
		      id == SCREEN_CTRL_ID_TOUCH_NEXT_PAGE ||
		      id == SCREEN_CTRL_ID_TOUCH_NEXT_PAGE2 ? ACTION_NEXT :
		      ACTION_NONE);
	  }
	  break;

        case EVENT_KEYPRESS:
	  {
	    Key key = GetEventKey((KeyEvent *)&event);

	    action = (key == KSYM_Escape ?	ACTION_ESCAPE :
		      key == KSYM_BackSpace ||
		      key == KSYM_Back ?	ACTION_BACK :
		      key == KSYM_Return ||
		      key == KSYM_Menu ||
		      key == KSYM_space ?	ACTION_NEXT :
		      ACTION_NONE);
	  }
	  break;

        case EVENT_KEYRELEASE:
	  key_joystick_mapping = 0;
	  break;

	case EVENT_BUTTONPRESS:
	case EVENT_BUTTONRELEASE:
	  {
	    ButtonEvent *button = (ButtonEvent *)&event;

	    motion_status = FALSE;

	    if (button->type == EVENT_BUTTONPRESS)
	      button_status = button->button;
	    else
	      button_status = MB_RELEASED;

	    if (HandleGadgets(button->x, button->y, button_status))
	    {
	      // do not handle this button event anymore
	      break;
	    }

	    button->x += video.screen_xoffset;
	    button->y += video.screen_yoffset;

	    x = button->x * overlay.grid_xsize / video.screen_width;
	    y = button->y * overlay.grid_ysize / video.screen_height;

	    if (button->type == EVENT_BUTTONPRESS)
	    {
	      grid_button_draw =
		(overlay.grid_button[x][y] != grid_button[step_nr] ?
		 grid_button[step_nr] : CHAR_GRID_BUTTON_NONE);

	      set_grid_button = TRUE;
	    }

	    MapScreenMenuGadgets_OverlayTouchButtons(button->y);
	  }
	  break;

	case EVENT_MOTIONNOTIFY:
	  {
	    MotionEvent *motion = (MotionEvent *)&event;

	    motion_status = TRUE;

	    if (HandleGadgets(motion->x, motion->y, button_status))
	    {
	      // do not handle this button event anymore
	      break;
	    }

	    motion->x += video.screen_xoffset;
	    motion->y += video.screen_yoffset;

	    x = motion->x * overlay.grid_xsize / video.screen_width;
	    y = motion->y * overlay.grid_ysize / video.screen_height;

	    set_grid_button = TRUE;

	    MapScreenMenuGadgets_OverlayTouchButtons(motion->y);
	  }
	  break;

	case SDL_WINDOWEVENT:
	  HandleWindowEvent((WindowEvent *) &event);

	  // check if device has been rotated
	  if (nr != GRID_ACTIVE_NR())
	  {
	    nr = GRID_ACTIVE_NR();

	    for (x = 0; x < MAX_GRID_XSIZE; x++)
	      for (y = 0; y < MAX_GRID_YSIZE; y++)
		grid_button_old[x][y] = grid_button_tmp[x][y] =
		  overlay.grid_button[x][y];
	  }

	  break;

	case SDL_APP_WILLENTERBACKGROUND:
	case SDL_APP_DIDENTERBACKGROUND:
	case SDL_APP_WILLENTERFOREGROUND:
	case SDL_APP_DIDENTERFOREGROUND:
	  HandlePauseResumeEvent((PauseResumeEvent *) &event);
	  break;

        default:
	  HandleOtherEvents(&event);
	  break;
      }

      // ---------- perform action set by handling events ----------

      if (action == ACTION_ESCAPE)
      {
	// abort and restore the old key bindings

	for (x = 0; x < MAX_GRID_XSIZE; x++)
	  for (y = 0; y < MAX_GRID_YSIZE; y++)
	    overlay.grid_button[x][y] = grid_button_old[x][y];

	FadeSkipNextFadeIn();

	finished = TRUE;
      }
      else if (action == ACTION_BACK)
      {
	// keep the configured key bindings and go to previous page

	step_nr--;

	if (step_nr < 0)
	{
	  FadeSkipNextFadeIn();

	  finished = TRUE;
	}
      }
      else if (action == ACTION_NEXT)
      {
	// keep the configured key bindings and go to next page

	step_nr++;

	// all virtual buttons configured
	if (step_nr == 6)
	{
	  finished = TRUE;
	  success = TRUE;
	}
      }

      if (action != ACTION_NONE && !finished)
      {
	for (x = 0; x < MAX_GRID_XSIZE; x++)
	  for (y = 0; y < MAX_GRID_YSIZE; y++)
	    grid_button_tmp[x][y] = overlay.grid_button[x][y];

	overlay.grid_button_highlight = grid_button[step_nr];

	// configure next virtual button

	ClearField();

	DrawTextSCentered(MENU_TITLE_YPOS, FONT_TITLE_1, "Virtual Buttons");
	DrawTextSCentered(ypos1, font_nr, "Select tiles to");
	DrawTextSCentered(ypos2, font_nr, customize_step_text[step_nr]);
      }

      if (set_grid_button)
      {
	overlay.grid_button[x][y] =
	  (grid_button_draw != CHAR_GRID_BUTTON_NONE ? grid_button_draw :
	   grid_button_tmp[x][y] == grid_button[step_nr] ? CHAR_GRID_BUTTON_NONE :
	   grid_button_tmp[x][y]);

	set_grid_button = FALSE;
      }
    }

    BackToFront();
  }

  for (x = 0; x < MAX_GRID_XSIZE; x++)
    for (y = 0; y < MAX_GRID_YSIZE; y++)
      setup.touch.grid_button[nr][x][y] = overlay.grid_button[x][y];

  overlay.grid_button_highlight = CHAR_GRID_BUTTON_NONE;

  SetOverlayShowGrid(FALSE);

  return success;
}

void ConfigureVirtualButtons(void)
{
  boolean success = ConfigureVirtualButtonsMain();

  UnmapScreenMenuGadgets(SCREEN_MASK_TOUCH |
			 SCREEN_MASK_TOUCH2);

  if (success)
  {
    int font_nr = FONT_TITLE_1;
    int font_height = getFontHeight(font_nr);
    int ypos1 = SYSIZE / 2 - font_height * 2;
    int ypos2 = SYSIZE / 2 - font_height * 1;
    DelayCounter wait_frame_delay = { 2000 };

    ResetDelayCounter(&wait_frame_delay);

    ClearField();

    DrawTextSCentered(ypos1, font_nr, "Virtual buttons");
    DrawTextSCentered(ypos2, font_nr, "configured!");

    while (!DelayReached(&wait_frame_delay))
      BackToFront();

    ClearEventQueue();
  }
}

void DrawSetupScreen(void)
{
  align_xoffset = 0;
  align_yoffset = 0;

  if (setup_mode == SETUP_MODE_INPUT)
    DrawSetupScreen_Input();
  else if (setup_mode == SETUP_MODE_CHOOSE_SCORES_TYPE)
    DrawChooseTree(&scores_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED)
    DrawChooseTree(&game_speed_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY)
    DrawChooseTree(&scroll_delay_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
    DrawChooseTree(&snapshot_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE)
    DrawChooseTree(&game_engine_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64)
    DrawChooseTree(&bd_palette_c64_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV)
    DrawChooseTree(&bd_palette_c64dtv_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_ATARI)
    DrawChooseTree(&bd_palette_atari_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_COLOR_TYPE)
    DrawChooseTree(&bd_color_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
    DrawChooseTree(&window_size_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE)
    DrawChooseTree(&scaling_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_RENDERING)
    DrawChooseTree(&rendering_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VSYNC)
    DrawChooseTree(&vsync_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    DrawChooseTree(&artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    DrawChooseTree(&artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    DrawChooseTree(&artwork.mus_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE)
    DrawChooseTree(&volume_simple_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS)
    DrawChooseTree(&volume_loops_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
    DrawChooseTree(&volume_music_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL)
    DrawChooseTree(&touch_control_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE)
    DrawChooseTree(&move_distance_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
    DrawChooseTree(&drop_distance_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_TRANSPARENCY)
    DrawChooseTree(&transparency_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_0)
    DrawChooseTree(&grid_size_current[0][0]);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_0)
    DrawChooseTree(&grid_size_current[0][1]);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_1)
    DrawChooseTree(&grid_size_current[1][0]);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_1)
    DrawChooseTree(&grid_size_current[1][1]);
  else
    DrawSetupScreen_Generic();

  PlayMenuSoundsAndMusic();
}

void RedrawSetupScreenAfterFullscreenToggle(void)
{
  if (setup_mode == SETUP_MODE_GRAPHICS ||
      setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
  {
    // update list selection from "setup.window_scaling_percent"
    execSetupGraphics_setWindowSizes(TRUE);

    DrawSetupScreen();
  }
}

void RedrawSetupScreenAfterScreenRotation(int nr)
{
  int x, y;

  if (setup_mode == SETUP_MODE_TOUCH)
  {
    // update virtual button settings (depending on screen orientation)
    DrawSetupScreen();
  }
  else if (setup_mode == SETUP_MODE_CONFIG_VIRT_BUTTONS)
  {
    // save already configured virtual buttons
    for (x = 0; x < MAX_GRID_XSIZE; x++)
      for (y = 0; y < MAX_GRID_YSIZE; y++)
	setup.touch.grid_button[nr][x][y] = overlay.grid_button[x][y];
  }
}

void HandleSetupScreen(int mx, int my, int dx, int dy, int button)
{
  if (setup_mode == SETUP_MODE_INPUT)
    HandleSetupScreen_Input(mx, my, dx, dy, button);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCORES_TYPE)
    HandleChooseTree(mx, my, dx, dy, button, &scores_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED)
    HandleChooseTree(mx, my, dx, dy, button, &game_speed_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY)
    HandleChooseTree(mx, my, dx, dy, button, &scroll_delay_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
    HandleChooseTree(mx, my, dx, dy, button, &snapshot_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_ENGINE_TYPE)
    HandleChooseTree(mx, my, dx, dy, button, &game_engine_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64)
    HandleChooseTree(mx, my, dx, dy, button, &bd_palette_c64_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_C64DTV)
    HandleChooseTree(mx, my, dx, dy, button, &bd_palette_c64dtv_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_PALETTE_ATARI)
    HandleChooseTree(mx, my, dx, dy, button, &bd_palette_atari_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_BD_COLOR_TYPE)
    HandleChooseTree(mx, my, dx, dy, button, &bd_color_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
    HandleChooseTree(mx, my, dx, dy, button, &window_size_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE)
    HandleChooseTree(mx, my, dx, dy, button, &scaling_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_RENDERING)
    HandleChooseTree(mx, my, dx, dy, button, &rendering_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VSYNC)
    HandleChooseTree(mx, my, dx, dy, button, &vsync_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.mus_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE)
    HandleChooseTree(mx, my, dx, dy, button, &volume_simple_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS)
    HandleChooseTree(mx, my, dx, dy, button, &volume_loops_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
    HandleChooseTree(mx, my, dx, dy, button, &volume_music_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL)
    HandleChooseTree(mx, my, dx, dy, button, &touch_control_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE)
    HandleChooseTree(mx, my, dx, dy, button, &move_distance_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
    HandleChooseTree(mx, my, dx, dy, button, &drop_distance_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_TRANSPARENCY)
    HandleChooseTree(mx, my, dx, dy, button, &transparency_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_0)
    HandleChooseTree(mx, my, dx, dy, button, &grid_size_current[0][0]);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_0)
    HandleChooseTree(mx, my, dx, dy, button, &grid_size_current[0][1]);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_XSIZE_1)
    HandleChooseTree(mx, my, dx, dy, button, &grid_size_current[1][0]);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRID_YSIZE_1)
    HandleChooseTree(mx, my, dx, dy, button, &grid_size_current[1][1]);
  else
    HandleSetupScreen_Generic(mx, my, dx, dy, button);
}

void HandleGameActions(void)
{
  if (CheckRestartGame())
    return;

  if (game_status != GAME_MODE_PLAYING)
    return;

  GameActions();		// main game loop

  if (tape.auto_play && !tape.playing)
    AutoPlayTapesContinue();	// continue automatically playing next tape
}


// ---------- new screen button stuff --------------------------------------

static struct
{
  int gfx_unpressed, gfx_pressed, gfx_active;
  struct MenuPosInfo *pos;
  boolean *check_value;
  int gadget_id;
  int screen_mask;
  unsigned int event_mask;
  boolean is_touch_button;
  char *infotext;
} menubutton_info[NUM_SCREEN_MENUBUTTONS] =
{
  {
    IMG_MENU_BUTTON_PREV_LEVEL, IMG_MENU_BUTTON_PREV_LEVEL_ACTIVE, -1,
    &menu.main.button.prev_level, NULL,
    SCREEN_CTRL_ID_PREV_LEVEL,
    SCREEN_MASK_MAIN,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "previous level"
  },
  {
    IMG_MENU_BUTTON_NEXT_LEVEL, IMG_MENU_BUTTON_NEXT_LEVEL_ACTIVE, -1,
    &menu.main.button.next_level, NULL,
    SCREEN_CTRL_ID_NEXT_LEVEL,
    SCREEN_MASK_MAIN,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "next level"
  },
  {
    IMG_MENU_BUTTON_PREV_LEVEL2, IMG_MENU_BUTTON_PREV_LEVEL2_ACTIVE, -1,
    &menu.scores.button.prev_level, NULL,
    SCREEN_CTRL_ID_PREV_LEVEL2,
    SCREEN_MASK_SCORES | SCREEN_MASK_SCORES_INFO,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "previous level"
  },
  {
    IMG_MENU_BUTTON_NEXT_LEVEL2, IMG_MENU_BUTTON_NEXT_LEVEL2_ACTIVE, -1,
    &menu.scores.button.next_level, NULL,
    SCREEN_CTRL_ID_NEXT_LEVEL2,
    SCREEN_MASK_SCORES | SCREEN_MASK_SCORES_INFO,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "next level"
  },
  {
    IMG_MENU_BUTTON_PREV_LEVEL3, IMG_MENU_BUTTON_PREV_LEVEL3_ACTIVE, -1,
    &menu.info.button.prev_level, NULL,
    SCREEN_CTRL_ID_PREV_LEVEL3,
    SCREEN_MASK_INFO,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "previous level"
  },
  {
    IMG_MENU_BUTTON_NEXT_LEVEL3, IMG_MENU_BUTTON_NEXT_LEVEL3_ACTIVE, -1,
    &menu.info.button.next_level, NULL,
    SCREEN_CTRL_ID_NEXT_LEVEL3,
    SCREEN_MASK_INFO,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "next level"
  },
  {
    IMG_MENU_BUTTON_PREV_SCORE, IMG_MENU_BUTTON_PREV_SCORE_ACTIVE, -1,
    &menu.scores.button.prev_score, NULL,
    SCREEN_CTRL_ID_PREV_SCORE,
    SCREEN_MASK_SCORES_INFO,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "previous score"
  },
  {
    IMG_MENU_BUTTON_NEXT_SCORE, IMG_MENU_BUTTON_NEXT_SCORE_ACTIVE, -1,
    &menu.scores.button.next_score, NULL,
    SCREEN_CTRL_ID_NEXT_SCORE,
    SCREEN_MASK_SCORES_INFO,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "next score"
  },
  {
    IMG_MENU_BUTTON_PLAY_TAPE, IMG_MENU_BUTTON_PLAY_TAPE, -1,
    &menu.scores.button.play_tape, NULL,
    SCREEN_CTRL_ID_PLAY_TAPE,
    SCREEN_MASK_SCORES_INFO,
    GD_EVENT_RELEASED,
    FALSE, "play tape"
  },
  {
    IMG_MENU_BUTTON_FIRST_LEVEL, IMG_MENU_BUTTON_FIRST_LEVEL_ACTIVE, -1,
    &menu.main.button.first_level, NULL,
    SCREEN_CTRL_ID_FIRST_LEVEL,
    SCREEN_MASK_MAIN,
    GD_EVENT_RELEASED,
    FALSE, "first level"
  },
  {
    IMG_MENU_BUTTON_LAST_LEVEL, IMG_MENU_BUTTON_LAST_LEVEL_ACTIVE, -1,
    &menu.main.button.last_level, NULL,
    SCREEN_CTRL_ID_LAST_LEVEL,
    SCREEN_MASK_MAIN,
    GD_EVENT_RELEASED,
    FALSE, "last level"
  },
  {
    IMG_MENU_BUTTON_LEVEL_NUMBER, IMG_MENU_BUTTON_LEVEL_NUMBER_ACTIVE, -1,
    &menu.main.button.level_number, NULL,
    SCREEN_CTRL_ID_LEVEL_NUMBER,
    SCREEN_MASK_MAIN,
    GD_EVENT_RELEASED,
    FALSE, "level number"
  },
  {
    IMG_MENU_BUTTON_LEFT, IMG_MENU_BUTTON_LEFT_ACTIVE, -1,
    &menu.setup.button.prev_player, NULL,
    SCREEN_CTRL_ID_PREV_PLAYER,
    SCREEN_MASK_INPUT,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "previous player"
  },
  {
    IMG_MENU_BUTTON_RIGHT, IMG_MENU_BUTTON_RIGHT_ACTIVE, -1,
    &menu.setup.button.next_player, NULL,
    SCREEN_CTRL_ID_NEXT_PLAYER,
    SCREEN_MASK_INPUT,
    GD_EVENT_PRESSED | GD_EVENT_REPEATED,
    FALSE, "next player"
  },
  {
    IMG_MENU_BUTTON_INSERT_SOLUTION, IMG_MENU_BUTTON_INSERT_SOLUTION_ACTIVE, -1,
    &menu.main.button.insert_solution, NULL,
    SCREEN_CTRL_ID_INSERT_SOLUTION,
    SCREEN_MASK_MAIN_HAS_SOLUTION,
    GD_EVENT_RELEASED,
    FALSE, "insert solution tape"
  },
  {
    IMG_MENU_BUTTON_PLAY_SOLUTION, IMG_MENU_BUTTON_PLAY_SOLUTION_ACTIVE, -1,
    &menu.main.button.play_solution, NULL,
    SCREEN_CTRL_ID_PLAY_SOLUTION,
    SCREEN_MASK_MAIN_HAS_SOLUTION,
    GD_EVENT_RELEASED,
    FALSE, "play solution tape"
  },
  {
    IMG_MENU_BUTTON_LEVELSET_INFO, IMG_MENU_BUTTON_LEVELSET_INFO_PRESSED,
    IMG_MENU_BUTTON_LEVELSET_INFO_ACTIVE,
    &menu.main.button.levelset_info, NULL,
    SCREEN_CTRL_ID_LEVELSET_INFO,
    SCREEN_MASK_MAIN_HAS_LEVELSET_INFO,
    GD_EVENT_RELEASED,
    FALSE, "show level set info"
  },
  {
    IMG_MENU_BUTTON_LEVEL_INFO, IMG_MENU_BUTTON_LEVEL_INFO_PRESSED,
    IMG_MENU_BUTTON_LEVEL_INFO_ACTIVE,
    &menu.main.button.level_info, NULL,
    SCREEN_CTRL_ID_LEVEL_INFO,
    SCREEN_MASK_MAIN_HAS_LEVEL_INFO,
    GD_EVENT_RELEASED,
    FALSE, "show level info"
  },
  {
    IMG_MENU_BUTTON_SWITCH_ECS_AGA, IMG_MENU_BUTTON_SWITCH_ECS_AGA_ACTIVE, -1,
    &menu.main.button.switch_ecs_aga, &setup.prefer_aga_graphics,
    SCREEN_CTRL_ID_SWITCH_ECS_AGA,
    SCREEN_MASK_MAIN,
    GD_EVENT_RELEASED | GD_EVENT_OFF_BORDERS,
    FALSE, "switch old/new graphics"
  },
  {
    IMG_MENU_BUTTON_TOUCH_BACK, IMG_MENU_BUTTON_TOUCH_BACK, -1,
    &menu.setup.button.touch_back, NULL,
    SCREEN_CTRL_ID_TOUCH_PREV_PAGE,
    SCREEN_MASK_TOUCH,
    GD_EVENT_RELEASED,
    TRUE, "previous page"
  },
  {
    IMG_MENU_BUTTON_TOUCH_NEXT, IMG_MENU_BUTTON_TOUCH_NEXT, -1,
    &menu.setup.button.touch_next, NULL,
    SCREEN_CTRL_ID_TOUCH_NEXT_PAGE,
    SCREEN_MASK_TOUCH,
    GD_EVENT_RELEASED,
    TRUE, "next page"
  },
  {
    IMG_MENU_BUTTON_TOUCH_BACK2, IMG_MENU_BUTTON_TOUCH_BACK2, -1,
    &menu.setup.button.touch_back2, NULL,
    SCREEN_CTRL_ID_TOUCH_PREV_PAGE2,
    SCREEN_MASK_TOUCH2,
    GD_EVENT_RELEASED,
    TRUE, "previous page"
  },
  {
    IMG_MENU_BUTTON_TOUCH_NEXT2, IMG_MENU_BUTTON_TOUCH_NEXT2, -1,
    &menu.setup.button.touch_next2, NULL,
    SCREEN_CTRL_ID_TOUCH_NEXT_PAGE2,
    SCREEN_MASK_TOUCH2,
    GD_EVENT_RELEASED,
    TRUE, "next page"
  },
};

static struct
{
  int gfx_unpressed, gfx_pressed;
  int x, y;
  int gadget_id;
  char *infotext;
} scrollbutton_info[NUM_SCREEN_SCROLLBUTTONS] =
{
  {
    IMG_MENU_BUTTON_UP, IMG_MENU_BUTTON_UP_ACTIVE,
    -1, -1,	// these values are not constant, but can change at runtime
    SCREEN_CTRL_ID_SCROLL_UP,
    "scroll up"
  },
  {
    IMG_MENU_BUTTON_DOWN, IMG_MENU_BUTTON_DOWN_ACTIVE,
    -1, -1,	// these values are not constant, but can change at runtime
    SCREEN_CTRL_ID_SCROLL_DOWN,
    "scroll down"
  }
};

static struct
{
  int gfx_unpressed, gfx_pressed;
  int x, y;
  int width, height;
  int type;
  int gadget_id;
  char *infotext;
} scrollbar_info[NUM_SCREEN_SCROLLBARS] =
{
  {
    IMG_MENU_SCROLLBAR, IMG_MENU_SCROLLBAR_ACTIVE,
    -1, -1,	// these values are not constant, but can change at runtime
    -1, -1,	// these values are not constant, but can change at runtime
    GD_TYPE_SCROLLBAR_VERTICAL,
    SCREEN_CTRL_ID_SCROLL_VERTICAL,
    "scroll level series vertically"
  }
};

static struct
{
  int graphic;
  int gadget_id;
  int x, y;
  int size;
  char *value;
  char *infotext;
} textinput_info[NUM_SCREEN_TEXTINPUT] =
{
  {
    IMG_SETUP_INPUT_TEXT,
    SCREEN_CTRL_ID_NETWORK_SERVER,
    -1, -1,	// these values are not constant, but can change at runtime
    MAX_SETUP_TEXT_INPUT_LEN,
    network_server_hostname,
    "Network Server Hostname / IP"
  },
};

static void CreateScreenMenubuttons(void)
{
  struct GadgetInfo *gi;
  unsigned int event_mask;
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
  {
    struct MenuPosInfo *pos = menubutton_info[i].pos;
    int screen_mask = menubutton_info[i].screen_mask;
    boolean is_touch_button = menubutton_info[i].is_touch_button;
    boolean is_check_button = menubutton_info[i].check_value != NULL;
    boolean is_score_button = (screen_mask & SCREEN_MASK_SCORES_INFO);
    boolean is_info_button  = (screen_mask & SCREEN_MASK_INFO);
    boolean has_gfx_pressed = (menubutton_info[i].gfx_pressed ==
                               menubutton_info[i].gfx_unpressed);
    boolean has_gfx_active = (menubutton_info[i].gfx_active != -1);
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    Bitmap *gd_bitmap_unpressed_alt, *gd_bitmap_pressed_alt;
    int gfx_unpressed, gfx_pressed;
    int gfx_unpressed_alt, gfx_pressed_alt;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int gd_x1a, gd_x2a, gd_y1a, gd_y2a;
    int id = menubutton_info[i].gadget_id;
    int type = GD_TYPE_NORMAL_BUTTON;
    boolean checked = FALSE;

    // do not use touch buttons if overlay touch buttons are disabled
    if (is_touch_button && !setup.touch.overlay_buttons)
      continue;

    event_mask = menubutton_info[i].event_mask;

    x = (is_touch_button ? pos->x : mSX + GDI_ACTIVE_POS(pos->x));
    y = (is_touch_button ? pos->y : mSY + GDI_ACTIVE_POS(pos->y));

    width  = graphic_info[menubutton_info[i].gfx_pressed].width;
    height = graphic_info[menubutton_info[i].gfx_pressed].height;

    gfx_unpressed = menubutton_info[i].gfx_unpressed;
    gfx_pressed   = menubutton_info[i].gfx_pressed;
    gfx_unpressed_alt = gfx_unpressed;
    gfx_pressed_alt   = gfx_pressed;

    if (has_gfx_active)
    {
      gfx_unpressed_alt = menubutton_info[i].gfx_active;

      type = GD_TYPE_CHECK_BUTTON_2;

      if (menubutton_info[i].check_value != NULL)
	checked = *menubutton_info[i].check_value;
    }

    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_bitmap_unpressed_alt = graphic_info[gfx_unpressed_alt].bitmap;
    gd_bitmap_pressed_alt   = graphic_info[gfx_pressed_alt].bitmap;

    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gd_x1a = graphic_info[gfx_unpressed_alt].src_x;
    gd_y1a = graphic_info[gfx_unpressed_alt].src_y;
    gd_x2a = graphic_info[gfx_pressed_alt].src_x;
    gd_y2a = graphic_info[gfx_pressed_alt].src_y;

    if (has_gfx_pressed)
    {
      gd_x2 += graphic_info[gfx_pressed].pressed_xoffset;
      gd_y2 += graphic_info[gfx_pressed].pressed_yoffset;
    }

    if (is_check_button)
    {
      gd_x1a += graphic_info[gfx_unpressed].active_xoffset;
      gd_y1a += graphic_info[gfx_unpressed].active_yoffset;
      gd_x2a += graphic_info[gfx_pressed].active_xoffset;
      gd_y2a += graphic_info[gfx_pressed].active_yoffset;

      type = GD_TYPE_CHECK_BUTTON;

      if (menubutton_info[i].check_value != NULL)
	checked = *menubutton_info[i].check_value;
    }

    if (is_score_button)
    {
      // if x/y set to -1, dynamically place buttons next to title text
      int title_width = getTextWidth(INFOTEXT_SCORE_ENTRY, FONT_TITLE_1);

      // special compatibility handling for "Snake Bite" graphics set
      if (strPrefix(leveldir_current->identifier, "snake_bite"))
	title_width = strlen(INFOTEXT_SCORE_ENTRY) * 32;

      // use "SX" here to center buttons (ignore horizontal draw offset)
      if (pos->x == -1)
	x = (id == SCREEN_CTRL_ID_PREV_LEVEL2 ?
	     SX + (SXSIZE - title_width) / 2 - width * 3 / 2 :
	     id == SCREEN_CTRL_ID_NEXT_LEVEL2 ?
	     SX + (SXSIZE + title_width) / 2 + width / 2 : 0);

      // use "mSY" here to place buttons (respect vertical draw offset)
      if (pos->y == -1)
	y = (id == SCREEN_CTRL_ID_PREV_LEVEL2 ||
	     id == SCREEN_CTRL_ID_NEXT_LEVEL2 ? mSY + MENU_TITLE1_YPOS : 0);
    }
    else if (is_info_button)
    {
      // if x/y set to -1, dynamically place buttons next to title text
      int title_width = getTextWidth(STR_INFO_LEVEL, FONT_TITLE_1);

      // use "SX" here to center buttons (ignore horizontal draw offset)
      if (pos->x == -1)
	x = (id == SCREEN_CTRL_ID_PREV_LEVEL3 ?
	     SX + (SXSIZE - title_width) / 2 - width * 3 / 2 :
	     id == SCREEN_CTRL_ID_NEXT_LEVEL3 ?
	     SX + (SXSIZE + title_width) / 2 + width / 2 : 0);

      // use "mSY" here to place buttons (respect vertical draw offset)
      if (pos->y == -1)
	y = (id == SCREEN_CTRL_ID_PREV_LEVEL3 ||
	     id == SCREEN_CTRL_ID_NEXT_LEVEL3 ? mSY + MENU_TITLE1_YPOS : 0);
    }

    if (id == SCREEN_CTRL_ID_LEVELSET_INFO)
    {
      if (pos->x == -1 && pos->y == -1)
      {
	// use "SX" here to place button (ignore draw offsets)
	x = SX + SXSIZE - 2 * TILESIZE;
	y = SY + SYSIZE - 2 * TILESIZE;

	// special compatibility handling for "BD2K3" graphics set
	if (strPrefix(leveldir_current->identifier, "BD2K3"))
	  x = SX + TILESIZE + MINI_TILESIZE;

	// special compatibility handling for "jue0" graphics set
	if (strPrefix(artwork.gfx_current_identifier, "jue0"))
	{
	  x = SX + SXSIZE - 4 * TILESIZE;
	  y = SY + SYSIZE - 3 * TILESIZE;
	}
      }
    }
    else if (id == SCREEN_CTRL_ID_LEVEL_INFO)
    {
      if (pos->x == -1 && pos->y == -1)
      {
	// use "SX" here to place button (ignore draw offsets)
	x = SX + TILESIZE;
	y = SY + SYSIZE - 2 * TILESIZE;

	// special compatibility handling for "BD2K3" graphics set
	if (strPrefix(leveldir_current->identifier, "BD2K3"))
	  x = -1000;	// hide button (to prevent partially undrawing level set info button)
      }
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_IMAGE_ID, gfx_unpressed,
		      GDI_INFO_TEXT, menubutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, type,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_CHECKED, checked,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
                      GDI_ALT_DESIGN_UNPRESSED, gd_bitmap_unpressed_alt, gd_x1a, gd_y1a,
                      GDI_ALT_DESIGN_PRESSED, gd_bitmap_pressed_alt, gd_x2a, gd_y2a,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_OVERLAY_TOUCH_BUTTON, is_touch_button,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    screen_gadget[id] = gi;
  }
}

static void CreateScreenScrollbuttons(void)
{
  struct GadgetInfo *gi;
  unsigned int event_mask;
  int i;

  // these values are not constant, but can change at runtime
  scrollbutton_info[0].x = SC_SCROLL_UP_XPOS;
  scrollbutton_info[0].y = SC_SCROLL_UP_YPOS;
  scrollbutton_info[1].x = SC_SCROLL_DOWN_XPOS;
  scrollbutton_info[1].y = SC_SCROLL_DOWN_YPOS;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = scrollbutton_info[i].gadget_id;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    x = mSX + scrollbutton_info[i].x + menu.scrollbar_xoffset;
    y = mSY + scrollbutton_info[i].y;
    width = SC_SCROLLBUTTON_XSIZE;
    height = SC_SCROLLBUTTON_YSIZE;

    // correct scrollbar position if placed outside menu (playfield) area
    if (x > SX + SC_SCROLL_UP_XPOS)
      x = SX + SC_SCROLL_UP_XPOS;

    if (id == SCREEN_CTRL_ID_SCROLL_DOWN)
      y = mSY + (SC_SCROLL_VERTICAL_YPOS +
		 (NUM_MENU_ENTRIES_ON_SCREEN - 2) * SC_SCROLLBUTTON_YSIZE);

    gfx_unpressed = scrollbutton_info[i].gfx_unpressed;
    gfx_pressed   = scrollbutton_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_IMAGE_ID, gfx_unpressed,
		      GDI_INFO_TEXT, scrollbutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    screen_gadget[id] = gi;
  }
}

static void CreateScreenScrollbars(void)
{
  int i;

  // these values are not constant, but can change at runtime
  scrollbar_info[0].x = SC_SCROLL_VERTICAL_XPOS;
  scrollbar_info[0].y = SC_SCROLL_VERTICAL_YPOS;
  scrollbar_info[0].width  = SC_SCROLL_VERTICAL_XSIZE;
  scrollbar_info[0].height = SC_SCROLL_VERTICAL_YSIZE;

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    struct GadgetInfo *gi;
    int items_max, items_visible, item_position;
    unsigned int event_mask;
    int num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;
    int id = scrollbar_info[i].gadget_id;

    event_mask = GD_EVENT_MOVING | GD_EVENT_OFF_BORDERS;

    x = mSX + scrollbar_info[i].x + menu.scrollbar_xoffset;
    y = mSY + scrollbar_info[i].y;
    width  = scrollbar_info[i].width;
    height = scrollbar_info[i].height;

    // correct scrollbar position if placed outside menu (playfield) area
    if (x > SX + SC_SCROLL_VERTICAL_XPOS)
      x = SX + SC_SCROLL_VERTICAL_XPOS;

    if (id == SCREEN_CTRL_ID_SCROLL_VERTICAL)
      height = (NUM_MENU_ENTRIES_ON_SCREEN - 2) * SC_SCROLLBUTTON_YSIZE;

    items_max = num_page_entries;
    items_visible = num_page_entries;
    item_position = 0;

    gfx_unpressed = scrollbar_info[i].gfx_unpressed;
    gfx_pressed   = scrollbar_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_IMAGE_ID, gfx_unpressed,
		      GDI_INFO_TEXT, scrollbar_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, scrollbar_info[i].type,
		      GDI_SCROLLBAR_ITEMS_MAX, items_max,
		      GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
		      GDI_SCROLLBAR_ITEM_POSITION, item_position,
		      GDI_WHEEL_AREA_X, SX,
		      GDI_WHEEL_AREA_Y, SY,
		      GDI_WHEEL_AREA_WIDTH, SXSIZE,
		      GDI_WHEEL_AREA_HEIGHT, SYSIZE,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_BORDER_SIZE, SC_BORDER_SIZE, SC_BORDER_SIZE,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    screen_gadget[id] = gi;
  }
}

static void CreateScreenTextInputGadgets(void)
{
  int i;

  for (i = 0; i < NUM_SCREEN_TEXTINPUT; i++)
  {
    int graphic = textinput_info[i].graphic;
    struct GraphicInfo *gd = &graphic_info[graphic];
    int gd_x1 = gd->src_x;
    int gd_y1 = gd->src_y;
    int gd_x2 = gd->src_x + gd->active_xoffset;
    int gd_y2 = gd->src_y + gd->active_yoffset;
    struct GadgetInfo *gi;
    unsigned int event_mask;
    int id = textinput_info[i].gadget_id;
    int x = textinput_info[i].x;
    int y = textinput_info[i].y;

    event_mask = GD_EVENT_TEXT_RETURN | GD_EVENT_TEXT_LEAVING;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, textinput_info[i].infotext,
		      GDI_X, SX + x,
		      GDI_Y, SY + y,
		      GDI_TYPE, GD_TYPE_TEXT_INPUT_ALPHANUMERIC,
		      GDI_TEXT_VALUE, textinput_info[i].value,
		      GDI_TEXT_SIZE, textinput_info[i].size,
		      GDI_TEXT_FONT, getSetupValueFont(TYPE_STRING, NULL),
		      GDI_TEXT_FONT_ACTIVE, FONT_TEXT_1,
		      GDI_DESIGN_UNPRESSED, gd->bitmap, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd->bitmap, gd_x2, gd_y2,
		      GDI_BORDER_SIZE, gd->border_size, gd->border_size,
		      GDI_DESIGN_WIDTH, gd->width,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_CALLBACK_ACTION_ALWAYS, TRUE,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    screen_gadget[id] = gi;
  }
}

void CreateScreenGadgets(void)
{
  CreateScreenMenubuttons();

  CreateScreenScrollbuttons();
  CreateScreenScrollbars();

  CreateScreenTextInputGadgets();
}

void FreeScreenGadgets(void)
{
  int i;

  for (i = 0; i < NUM_SCREEN_GADGETS; i++)
    FreeGadget(screen_gadget[i]);
}

static void RedrawScreenMenuGadgets(int screen_mask)
{
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
    if (screen_mask & menubutton_info[i].screen_mask)
      RedrawGadget(screen_gadget[menubutton_info[i].gadget_id]);
}

static void MapScreenMenuGadgets(int screen_mask)
{
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
    if (screen_mask & menubutton_info[i].screen_mask)
      MapGadget(screen_gadget[menubutton_info[i].gadget_id]);
}

static void UnmapScreenMenuGadgets(int screen_mask)
{
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
  {
    if (screen_mask & menubutton_info[i].screen_mask)
    {
      UnmapGadget(screen_gadget[menubutton_info[i].gadget_id]);

      // undraw buttons for solution tapes or level info that may not exist for the selected level
      if (screen_mask & SCREEN_MASK_MAIN_HAS_SOLUTION ||
          screen_mask & SCREEN_MASK_MAIN_HAS_LEVEL_INFO)
	DrawBackground(screen_gadget[menubutton_info[i].gadget_id]->x,
		       screen_gadget[menubutton_info[i].gadget_id]->y,
		       screen_gadget[menubutton_info[i].gadget_id]->width,
		       screen_gadget[menubutton_info[i].gadget_id]->height);
    }
  }
}

static void UpdateScreenMenuGadgets(int screen_mask, boolean map_gadgets)
{
  if (map_gadgets)
    MapScreenMenuGadgets(screen_mask);
  else
    UnmapScreenMenuGadgets(screen_mask);
}

static void MapScreenGadgets(int num_entries)
{
  int i;

  if (num_entries <= NUM_MENU_ENTRIES_ON_SCREEN)
    return;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
    MapGadget(screen_gadget[scrollbutton_info[i].gadget_id]);

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
    MapGadget(screen_gadget[scrollbar_info[i].gadget_id]);
}

static void UnmapScreenGadgets(void)
{
  int i;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
    UnmapGadget(screen_gadget[scrollbutton_info[i].gadget_id]);

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
    UnmapGadget(screen_gadget[scrollbar_info[i].gadget_id]);
}

static void MapScreenTreeGadgets(TreeInfo *ti)
{
  MapScreenGadgets(numTreeInfoInGroup(ti));
}

static void UnmapScreenTreeGadgets(void)
{
  UnmapScreenGadgets();
}

static void MapScreenInfoGadgets(void)
{
  int i;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
    MapGadget(screen_gadget[scrollbutton_info[i].gadget_id]);

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
    MapGadget(screen_gadget[scrollbar_info[i].gadget_id]);
}

static void AdjustScoreInfoButtons_SelectScore(int x, int y1, int y2)
{
  struct GadgetInfo *gi_1 = screen_gadget[SCREEN_CTRL_ID_PREV_SCORE];
  struct GadgetInfo *gi_2 = screen_gadget[SCREEN_CTRL_ID_NEXT_SCORE];
  struct MenuPosInfo *pos_1 = menubutton_info[SCREEN_CTRL_ID_PREV_SCORE].pos;
  struct MenuPosInfo *pos_2 = menubutton_info[SCREEN_CTRL_ID_NEXT_SCORE].pos;

  if (pos_1->x == -1 && pos_1->y == -1)
    ModifyGadget(gi_1, GDI_X, x, GDI_Y, y1, GDI_END);

  if (pos_2->x == -1 && pos_2->y == -1)
    ModifyGadget(gi_2, GDI_X, x, GDI_Y, y2, GDI_END);
}

static void AdjustScoreInfoButtons_PlayTape(int x, int y, boolean visible)
{
  struct GadgetInfo *gi = screen_gadget[SCREEN_CTRL_ID_PLAY_TAPE];
  struct MenuPosInfo *pos = menubutton_info[SCREEN_CTRL_ID_PLAY_TAPE].pos;

  // set gadget position dynamically, pre-defined or off-screen
  int xx = (visible ? (pos->x == -1 ? x : pos->x) : POS_OFFSCREEN);
  int yy = (visible ? (pos->y == -1 ? y : pos->y) : POS_OFFSCREEN);

  ModifyGadget(gi, GDI_X, xx, GDI_Y, yy, GDI_END);
  MapGadget(gi);	// (needed if deactivated on last score page)
}

static void HandleScreenGadgets(struct GadgetInfo *gi)
{
  int id = gi->custom_id;
  int button = gi->event.button;
  int step = (button == MB_LEFTBUTTON   ? 1 :
	      button == MB_MIDDLEBUTTON ? 5 :
	      button == MB_RIGHTBUTTON  ? 10 : 1);

  switch (id)
  {
    case SCREEN_CTRL_ID_PREV_LEVEL:
      HandleMainMenu_SelectLevel(step, -1, NO_DIRECT_LEVEL_SELECT);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL:
      HandleMainMenu_SelectLevel(step, +1, NO_DIRECT_LEVEL_SELECT);
      break;

    case SCREEN_CTRL_ID_PREV_LEVEL2:
      HandleHallOfFame_SelectLevel(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL2:
      HandleHallOfFame_SelectLevel(step, +1);
      break;

    case SCREEN_CTRL_ID_PREV_LEVEL3:
      HandleInfoScreen_SelectLevel(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL3:
      HandleInfoScreen_SelectLevel(step, +1);
      break;

    case SCREEN_CTRL_ID_PREV_SCORE:
      HandleScoreInfo_SelectScore(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_SCORE:
      HandleScoreInfo_SelectScore(step, +1);
      break;

    case SCREEN_CTRL_ID_PLAY_TAPE:
      HandleScoreInfo_PlayTape();
      break;

    case SCREEN_CTRL_ID_FIRST_LEVEL:
      HandleMainMenu_SelectLevel(MAX_LEVELS, -1, NO_DIRECT_LEVEL_SELECT);
      break;

    case SCREEN_CTRL_ID_LAST_LEVEL:
      HandleMainMenu_SelectLevel(MAX_LEVELS, +1, NO_DIRECT_LEVEL_SELECT);
      break;

    case SCREEN_CTRL_ID_LEVEL_NUMBER:
      CloseDoor(DOOR_CLOSE_2);
      SetGameStatus(GAME_MODE_LEVELNR);
      DrawChooseLevelNr();
      break;

    case SCREEN_CTRL_ID_PREV_PLAYER:
      HandleSetupScreen_Input_Player(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_PLAYER:
      HandleSetupScreen_Input_Player(step, +1);
      break;

    case SCREEN_CTRL_ID_INSERT_SOLUTION:
      InsertSolutionTape();
      break;

    case SCREEN_CTRL_ID_PLAY_SOLUTION:
      PlaySolutionTape();
      break;

    case SCREEN_CTRL_ID_LEVELSET_INFO:
      DrawInfoScreen_FromMainMenu(INFO_MODE_LEVELSET);
      break;

    case SCREEN_CTRL_ID_LEVEL_INFO:
      DrawInfoScreen_FromMainMenu(INFO_MODE_LEVEL);
      break;

    case SCREEN_CTRL_ID_SWITCH_ECS_AGA:
      setup.prefer_aga_graphics = !setup.prefer_aga_graphics;
      DrawMainMenu();
      break;

    case SCREEN_CTRL_ID_TOUCH_PREV_PAGE:
    case SCREEN_CTRL_ID_TOUCH_NEXT_PAGE:
    case SCREEN_CTRL_ID_TOUCH_PREV_PAGE2:
    case SCREEN_CTRL_ID_TOUCH_NEXT_PAGE2:
      PushUserEvent(USEREVENT_GADGET_PRESSED, id, 0);
      break;

    case SCREEN_CTRL_ID_SCROLL_UP:
      if (game_status == GAME_MODE_NAMES)
	HandleChoosePlayerName(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevelSet(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELNR)
	HandleChooseLevelNr(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_STORY)
	HandleStoryScreen(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SCORES)
	HandleHallOfFame(0, 0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_DOWN:
      if (game_status == GAME_MODE_NAMES)
	HandleChoosePlayerName(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevelSet(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELNR)
	HandleChooseLevelNr(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_STORY)
	HandleStoryScreen(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SCORES)
	HandleHallOfFame(0, 0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_VERTICAL:
      if (game_status == GAME_MODE_NAMES)
	HandleChoosePlayerName(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevelSet(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_LEVELNR)
	HandleChooseLevelNr(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_STORY)
	HandleStoryScreen(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_SCORES)
	HandleHallOfFame(0, 0, 999, gi->event.item_position, MB_MENU_INITIALIZE);
      break;

    case SCREEN_CTRL_ID_NETWORK_SERVER:
    {
      if (!strEqual(gi->textinput.value, ""))
      {
	setString(&setup.network_server_hostname, gi->textinput.value);

	network.server_host = setup.network_server_hostname;
      }
      else
      {
	setString(&setup.network_server_hostname, STR_NETWORK_AUTO_DETECT);

	network.server_host = NULL;
      }

      if (strEqual(network.server_host, STR_NETWORK_AUTO_DETECT))
	network.server_host = NULL;

      execSetupGame_setNetworkServerText();

      DrawSetupScreen();

      break;
    }

    default:
      break;
  }
}

void HandleScreenGadgetKeys(Key key)
{
  if (key == setup.shortcut.tape_play || key == KSYM_Return)
    HandleScreenGadgets(screen_gadget[SCREEN_CTRL_ID_PLAY_TAPE]);
}

void DumpScreenIdentifiers(void)
{
  int i;

  Print("Active screen elements on current screen:\n");

  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];

    if (mci->button_graphic != -1)
    {
      char *token = getTokenFromImageID(mci->button_graphic);

      Print("- '%s'\n", token);
    }
  }

  Print("Done.\n");
}

boolean DoScreenAction(int image_id)
{
  int i;

  if (game_status != GAME_MODE_MAIN)
    return FALSE;

  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];
    struct MenuPosInfo *pos = mci->pos_button;

    if (mci->button_graphic == image_id)
    {
      int x = mSX + pos->x;
      int y = mSY + pos->y;

      HandleMainMenu(x, y, 0, 0, MB_MENU_CHOICE);

      return TRUE;
    }
  }

  return FALSE;
}

void DrawScreenAfterAddingSet(char *tree_subdir_new, int tree_type)
{
  // get tree info node of newly added level or artwork set
  TreeInfo *tree_node_first = TREE_FIRST_NODE(tree_type);
  TreeInfo *tree_node_new = getTreeInfoFromIdentifier(tree_node_first,
						      tree_subdir_new);
  if (tree_node_new == NULL)	// should not happen
    return;

  // if request dialog is active, do nothing
  if (game.request_active)
    return;

  if (game_status == GAME_MODE_MAIN &&
      tree_type == TREE_TYPE_LEVEL_DIR)
  {
    // when adding new level set in main menu, select it as current level set

    // change current level set to newly added level set from zip file
    leveldir_current = tree_node_new;

    // change current level number to first level of newly added level set
    level_nr = leveldir_current->first_level;

    // redraw screen to reflect changed level set
    DrawMainMenu();

    // save this level set and level number as last selected level set
    SaveLevelSetup_LastSeries();
    SaveLevelSetup_SeriesInfo();
  }
  else if (game_status == GAME_MODE_LEVELS &&
	   tree_type == TREE_TYPE_LEVEL_DIR)
  {
    // when adding new level set in level set menu, set cursor and update screen

    leveldir_current = tree_node_new;

    DrawChooseTree(&leveldir_current);
  }
  else if (game_status == GAME_MODE_SETUP)
  {
    // when adding new artwork set in setup menu, set cursor and update screen

    if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS &&
	tree_type == TREE_TYPE_GRAPHICS_DIR)
    {
      artwork.gfx_current = tree_node_new;

      DrawChooseTree(&artwork.gfx_current);
    }
    else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS &&
	     tree_type == TREE_TYPE_SOUNDS_DIR)
    {
      artwork.snd_current = tree_node_new;

      DrawChooseTree(&artwork.snd_current);
    }
    else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC &&
	     tree_type == TREE_TYPE_MUSIC_DIR)
    {
      artwork.mus_current = tree_node_new;

      DrawChooseTree(&artwork.mus_current);
    }
  }
}

static int UploadTapes(void)
{
  SetGameStatus(GAME_MODE_LOADING);

  FadeSetEnterScreen();
  FadeOut(REDRAW_ALL);

  ClearRectangle(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  FadeIn(REDRAW_ALL);

  DrawInitTextHead("Uploading tapes");

  global.autoplay_mode = AUTOPLAY_MODE_UPLOAD;
  global.autoplay_leveldir = "ALL";
  global.autoplay_all = TRUE;

  int num_tapes_uploaded = AutoPlayTapes();

  global.autoplay_mode = AUTOPLAY_MODE_NONE;
  global.autoplay_leveldir = NULL;
  global.autoplay_all = FALSE;

  SetGameStatus(GAME_MODE_MAIN);

  DrawMainMenu();

  return num_tapes_uploaded;
}

static boolean OfferUploadTapes(void)
{
  if (!Request(setup.has_remaining_tapes ?
	       "Upload missing tapes to the high score server now?" :
	       "Upload all your tapes to the high score server now?", REQ_ASK))
    return FALSE;

  // when uploading tapes, make sure that high score server is enabled
  runtime.use_api_server = setup.use_api_server = TRUE;

  int num_tapes_uploaded = UploadTapes();
  char message[100];

  if (num_tapes_uploaded < 0)
  {
    num_tapes_uploaded = -num_tapes_uploaded - 1;

    if (num_tapes_uploaded == 0)
      sprintf(message, "Upload failed! No tapes uploaded!");
    else if (num_tapes_uploaded == 1)
      sprintf(message, "Upload failed! Only 1 tape uploaded!");
    else
      sprintf(message, "Upload failed! Only %d tapes uploaded!",
	      num_tapes_uploaded);

    Request(message, REQ_CONFIRM);

    // if uploading tapes failed, add tape upload entry to setup menu
    setup.provide_uploading_tapes = TRUE;
    setup.has_remaining_tapes = TRUE;

    SaveSetup_ServerSetup();

    return FALSE;
  }

  if (num_tapes_uploaded == 0)
    sprintf(message, "No tapes uploaded!");
  else if (num_tapes_uploaded == 1)
    sprintf(message, "1 tape uploaded!");
  else
    sprintf(message, "%d tapes uploaded!", num_tapes_uploaded);

  Request(message, REQ_CONFIRM);

  if (num_tapes_uploaded > 0)
    Request("New scores will be visible after a few minutes!", REQ_CONFIRM);

  // after all tapes have been uploaded, remove entry from setup menu
  setup.provide_uploading_tapes = FALSE;
  setup.has_remaining_tapes = FALSE;

  SaveSetup_ServerSetup();

  return TRUE;
}

static void CheckUploadTapes(void)
{
  if (!setup.ask_for_uploading_tapes)
    return;

  // after asking for uploading tapes, do not ask again
  setup.ask_for_uploading_tapes = FALSE;
  setup.ask_for_remaining_tapes = FALSE;

  if (directoryExists(getTapeDir(NULL)))
  {
    boolean tapes_uploaded = OfferUploadTapes();

    if (!tapes_uploaded)
    {
      Request(setup.has_remaining_tapes ?
	      "You can upload missing tapes from the setup menu later!" :
	      "You can upload your tapes from the setup menu later!",
	      REQ_CONFIRM);
    }
  }
  else
  {
    // if tapes directory does not exist yet, never offer uploading all tapes
    setup.provide_uploading_tapes = FALSE;
  }

  SaveSetup_ServerSetup();
}

static void UpgradePlayerUUID(void)
{
  ApiResetUUIDAsThread(getUUID());
}

static void CheckUpgradePlayerUUID(void)
{
  if (setup.player_version > 1)
    return;

  UpgradePlayerUUID();
}

void CheckApiServerTasks(void)
{
  // check if the player's UUID has to be upgraded
  CheckUpgradePlayerUUID();

  // check if there are any tapes to be uploaded
  CheckUploadTapes();
}
