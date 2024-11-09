// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// main.c
// ============================================================================

#include "libgame/libgame.h"

#include "main.h"
#include "init.h"
#include "game.h"
#include "tape.h"
#include "tools.h"
#include "files.h"
#include "events.h"
#include "config.h"

Bitmap			       *bitmap_db_field;
Bitmap			       *bitmap_db_door_1;
Bitmap			       *bitmap_db_door_2;
Bitmap			       *bitmap_db_store_1;
Bitmap			       *bitmap_db_store_2;
DrawBuffer		       *fieldbuffer;
DrawBuffer		       *drawto_field;

int				game_status = -1;
int				game_status_last_screen = -1;
boolean				level_editor_test_game = FALSE;
boolean				score_info_tape_play = FALSE;
boolean				network_playing = FALSE;

int				key_joystick_mapping = 0;

short				Tile[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				Last[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				ChangeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				ChangePage[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				CustomValue[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				Back[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean				Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean				Pushed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				ChangeCount[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				ChangeEvent[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				WasJustMoving[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				WasJustFalling[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				CheckCollision[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				CheckImpact[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				AmoebaCnt[MAX_NUM_AMOEBA];
short				AmoebaCnt2[MAX_NUM_AMOEBA];
short				ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short				ExplodeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int				RunnerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int				PlayerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

int				GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 				GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 				GfxRandomStatic[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 				GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 				GfxElementEmpty[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int				GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 				GfxDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 				GfxRedraw[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

int				ActiveElement[MAX_NUM_ELEMENTS];
int				ActiveButton[NUM_IMAGE_FILES];
int				ActiveFont[NUM_FONTS];

int				lev_fieldx, lev_fieldy;
int				scroll_x, scroll_y;

int				WIN_XSIZE = WIN_XSIZE_DEFAULT;
int				WIN_YSIZE = WIN_YSIZE_DEFAULT;

int				SCR_FIELDX = SCR_FIELDX_DEFAULT;
int				SCR_FIELDY = SCR_FIELDY_DEFAULT;

int				REAL_SX = 6, REAL_SY = 6;
int				SX = 8, SY = 8;
int				DX = 566, DY = 60;
int				VX = 566, VY = 400;
int				EX = 566, EY = 356;
int				dDX, dDY;

int				FULL_SXSIZE = 2 + SXSIZE_DEFAULT + 2;
int				FULL_SYSIZE = 2 + SYSIZE_DEFAULT + 2;
int				SXSIZE = SXSIZE_DEFAULT;
int				SYSIZE = SYSIZE_DEFAULT;

int				FADE_SX = 6, FADE_SY = 6;
int				FADE_SXSIZE = 2 + SXSIZE_DEFAULT + 2;
int				FADE_SYSIZE = 2 + SXSIZE_DEFAULT + 2;

int				DXSIZE = 100;
int				DYSIZE = 280;
int				VXSIZE = 100;
int				VYSIZE = 100;
int				EXSIZE = 100;
int				EYSIZE = 144;
int				TILESIZE_VAR = TILESIZE;

int				FX, FY;
int				ScrollStepSize;
int				ScreenMovDir = MV_NONE, ScreenMovPos = 0;
int				ScreenGfxPos = 0;
int				BorderElement = EL_STEELWALL;
int				MenuFrameDelay = MENU_FRAME_DELAY;
int				GameFrameDelay = GAME_FRAME_DELAY;
int				FfwdFrameDelay = FFWD_FRAME_DELAY;
int				BX1, BY1;
int				BX2, BY2;
int				SBX_Left, SBX_Right;
int				SBY_Upper, SBY_Lower;

int				TimeFrames, TimePlayed, TimeLeft;
int				TapeTimeFrames, TapeTime;

boolean				network_player_action_received = FALSE;

struct LevelInfo		level, level_template;
struct PlayerInfo		stored_player[MAX_PLAYERS], *local_player = NULL;
struct ScoreInfo		scores, server_scores;
struct TapeInfo			tape;
struct GameInfo			game;
struct GlobalInfo		global;
struct BorderInfo		border;
struct ViewportInfo		viewport;
struct TitleFadingInfo		fading;
struct TitleFadingInfo		title_initial_first_default;
struct TitleFadingInfo		title_initial_default;
struct TitleFadingInfo		title_first_default;
struct TitleFadingInfo		title_default;
struct TitleMessageInfo		titlescreen_initial_first_default;
struct TitleMessageInfo		titlescreen_initial_first[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo		titlescreen_initial_default;
struct TitleMessageInfo		titlescreen_initial[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo		titlescreen_first_default;
struct TitleMessageInfo		titlescreen_first[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo		titlescreen_default;
struct TitleMessageInfo		titlescreen[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo		titlemessage_initial_first_default;
struct TitleMessageInfo		titlemessage_initial_first[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo		titlemessage_initial_default;
struct TitleMessageInfo		titlemessage_initial[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo		titlemessage_first_default;
struct TitleMessageInfo		titlemessage_first[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo		titlemessage_default;
struct TitleMessageInfo		titlemessage[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo		readme;
struct InitInfo			init, init_last;
struct MenuInfo			menu;
struct DoorInfo			door_1, door_2;
struct RequestInfo		request;
struct PreviewInfo		preview;
struct EditorInfo		editor;

struct GraphicInfo	       *graphic_info = NULL;
struct SoundInfo	       *sound_info = NULL;
struct MusicInfo	       *music_info = NULL;
struct MusicFileInfo	       *music_file_info = NULL;
struct HelpAnimInfo	       *helpanim_info = NULL;

SetupFileHash		       *helptext_info = NULL;
SetupFileHash		       *image_config_hash = NULL;
SetupFileHash		       *sound_config_hash = NULL;
SetupFileHash		       *element_token_hash = NULL;
SetupFileHash		       *graphic_token_hash = NULL;
SetupFileHash		       *font_token_hash = NULL;
SetupFileHash		       *hide_setup_hash = NULL;
SetupFileHash		       *anim_url_hash = NULL;


// ----------------------------------------------------------------------------
// element definitions
// ----------------------------------------------------------------------------

struct ElementInfo element_info[MAX_NUM_ELEMENTS + 1];

// this contains predefined structure elements to initialize "element_info"
struct ElementNameInfo element_name_info[MAX_NUM_ELEMENTS + 1] =
{
  // keyword to start parser: "ELEMENT_INFO_START" <-- do not change!

  // --------------------------------------------------------------------------
  // "real" level file elements
  // --------------------------------------------------------------------------

  {
    "empty_space",
    "empty_space",
    "Empty space"
  },
  {
    "sand",
    "sand",
    "Sand"
  },
  {
    "wall",
    "wall",
    "Normal wall"
  },
  {
    "wall_slippery",
    "wall",
    "Slippery wall"
  },
  {
    "rock",
    "rock",
    "Rock"
  },
  {
    "key_obsolete",
    "obsolete",
    "Key (obsolete)"
  },
  {
    "emerald",
    "emerald",
    "Emerald"
  },
  {
    "exit_closed",
    "exit",
    "Closed exit"
  },
  {
    "player_obsolete",
    "obsolete",
    "Player (obsolete)"
  },
  {
    "bug",
    "bug",
    "Bug (random start direction)"
  },
  {
    "spaceship",
    "spaceship",
    "Spaceship (random start direction)"
  },
  {
    "yamyam",
    "yamyam",
    "Yam yam (random start direction)"
  },
  {
    "robot",
    "robot",
    "Robot"
  },
  {
    "steelwall",
    "steelwall",
    "Steel wall"
  },
  {
    "diamond",
    "diamond",
    "Diamond"
  },
  {
    "amoeba_dead",
    "amoeba",
    "Dead amoeba"
  },
  {
    "quicksand_empty",
    "quicksand",
    "Quicksand (empty)"
  },
  {
    "quicksand_full",
    "quicksand",
    "Quicksand (with rock)"
  },
  {
    "amoeba_drop",
    "amoeba",
    "Amoeba drop"
  },
  {
    "bomb",
    "bomb",
    "Bomb"
  },
  {
    "magic_wall",
    "magic_wall",
    "Magic wall"
  },
  {
    "speed_pill",
    "speed_pill",
    "Speed pill"
  },
  {
    "acid",
    "acid",
    "Acid"
  },
  {
    "amoeba_wet",
    "amoeba",
    "Dropping amoeba (EM style)"
  },
  {
    "amoeba_dry",
    "amoeba",
    "Normal amoeba"
  },
  {
    "nut",
    "nut",
    "Nut with emerald"
  },
  {
    "game_of_life",
    "game_of_life",
    "Conway's wall of life"
  },
  {
    "biomaze",
    "biomaze",
    "Biomaze"
  },
  {
    "dynamite.active",
    "dynamite",
    "Burning dynamite"
  },
  {
    "stoneblock",
    "wall",
    "Wall"
  },
  {
    "robot_wheel",
    "robot_wheel",
    "Magic wheel"
  },
  {
    "robot_wheel.active",
    "robot_wheel",
    "Magic wheel (running)"
  },
  {
    "key_1",
    "key",
    "Key 1"
  },
  {
    "key_2",
    "key",
    "Key 2"
  },
  {
    "key_3",
    "key",
    "Key 3"
  },
  {
    "key_4",
    "key",
    "Key 4"
  },
  {
    "gate_1",
    "gate",
    "Door 1"
  },
  {
    "gate_2",
    "gate",
    "Door 2"
  },
  {
    "gate_3",
    "gate",
    "Door 3"
  },
  {
    "gate_4",
    "gate",
    "Door 4"
  },
  {
    "gate_1_gray",
    "gate",
    "Gray door (opened by key 1)"
  },
  {
    "gate_2_gray",
    "gate",
    "Gray door (opened by key 2)"
  },
  {
    "gate_3_gray",
    "gate",
    "Gray door (opened by key 3)"
  },
  {
    "gate_4_gray",
    "gate",
    "Gray door (opened by key 4)"
  },
  {
    "dynamite",
    "dynamite",
    "Dynamite"
  },
  {
    "pacman",
    "pacman",
    "Pac man (random start direction)"
  },
  {
    "invisible_wall",
    "wall",
    "Invisible normal wall"
  },
  {
    "lamp",
    "lamp",
    "Lamp (off)"
  },
  {
    "lamp.active",
    "lamp",
    "Lamp (on)"
  },
  {
    "wall_emerald",
    "wall",
    "Wall with emerald"
  },
  {
    "wall_diamond",
    "wall",
    "Wall with diamond"
  },
  {
    "amoeba_full",
    "amoeba",
    "Amoeba with content"
  },
  {
    "bd_amoeba",
    "bd_amoeba",
    "Amoeba (BD style)"
  },
  {
    "time_orb_full",
    "time_orb_full",
    "Time orb (full)"
  },
  {
    "time_orb_empty",
    "time_orb_empty",
    "Time orb (empty)"
  },
  {
    "expandable_wall",
    "wall",
    "Growing wall (horizontal, visible)"
  },
  {
    "bd_diamond",
    "bd_diamond",
    "Diamond (BD style)"
  },
  {
    "emerald_yellow",
    "emerald",
    "Yellow emerald"
  },
  {
    "wall_bd_diamond",
    "wall",
    "Wall with BD style diamond"
  },
  {
    "wall_emerald_yellow",
    "wall",
    "Wall with yellow emerald"
  },
  {
    "dark_yamyam",
    "dark_yamyam",
    "Dark yam yam"
  },
  {
    "bd_magic_wall",
    "bd_magic_wall",
    "Magic wall (BD style)"
  },
  {
    "invisible_steelwall",
    "steelwall",
    "Invisible steel wall"
  },
  {
    "sokoban_field_player",
    "sokoban",
    "Sokoban field with player"
  },
  {
    "dynabomb_increase_number",
    "dynabomb",
    "Increases number of bombs"
  },
  {
    "dynabomb_increase_size",
    "dynabomb",
    "Increases explosion size"
  },
  {
    "dynabomb_increase_power",
    "dynabomb",
    "Increases power of explosion"
  },
  {
    "sokoban_object",
    "sokoban",
    "Sokoban object"
  },
  {
    "sokoban_field_empty",
    "sokoban",
    "Sokoban empty field"
  },
  {
    "sokoban_field_full",
    "sokoban",
    "Sokoban field with object"
  },
  {
    "bd_butterfly.right",
    "bd_butterfly",
    "Butterfly (starts moving right)"
  },
  {
    "bd_butterfly.up",
    "bd_butterfly",
    "Butterfly (starts moving up)"
  },
  {
    "bd_butterfly.left",
    "bd_butterfly",
    "Butterfly (starts moving left)"
  },
  {
    "bd_butterfly.down",
    "bd_butterfly",
    "Butterfly (starts moving down)"
  },
  {
    "bd_firefly.right",
    "bd_firefly",
    "Firefly (starts moving right)"
  },
  {
    "bd_firefly.up",
    "bd_firefly",
    "Firefly (starts moving up)"
  },
  {
    "bd_firefly.left",
    "bd_firefly",
    "Firefly (starts moving left)"
  },
  {
    "bd_firefly.down",
    "bd_firefly",
    "Firefly (starts moving down)"
  },
  {
    "bd_butterfly",
    "bd_butterfly",
    "Butterfly (random start direction)"
  },
  {
    "bd_firefly",
    "bd_firefly",
    "Firefly (random start direction)"
  },
  {
    "player_1",
    "player",
    "Player 1"
  },
  {
    "player_2",
    "player",
    "Player 2"
  },
  {
    "player_3",
    "player",
    "Player 3"
  },
  {
    "player_4",
    "player",
    "Player 4"
  },
  {
    "bug.right",
    "bug",
    "Bug (starts moving right)"
  },
  {
    "bug.up",
    "bug",
    "Bug (starts moving up)"
  },
  {
    "bug.left",
    "bug",
    "Bug (starts moving left)"
  },
  {
    "bug.down",
    "bug",
    "Bug (starts moving down)"
  },
  {
    "spaceship.right",
    "spaceship",
    "Spaceship (starts moving right)"
  },
  {
    "spaceship.up",
    "spaceship",
    "Spaceship (starts moving up)"
  },
  {
    "spaceship.left",
    "spaceship",
    "Spaceship (starts moving left)"
  },
  {
    "spaceship.down",
    "spaceship",
    "Spaceship (starts moving down)"
  },
  {
    "pacman.right",
    "pacman",
    "Pac man (starts moving right)"
  },
  {
    "pacman.up",
    "pacman",
    "Pac man (starts moving up)"
  },
  {
    "pacman.left",
    "pacman",
    "Pac man (starts moving left)"
  },
  {
    "pacman.down",
    "pacman",
    "Pac man (starts moving down)"
  },
  {
    "emerald_red",
    "emerald",
    "Red emerald"
  },
  {
    "emerald_purple",
    "emerald",
    "Purple emerald"
  },
  {
    "wall_emerald_red",
    "wall",
    "Wall with red emerald"
  },
  {
    "wall_emerald_purple",
    "wall",
    "Wall with purple emerald"
  },
  {
    "acid_pool_topleft",
    "wall",
    "Acid pool (top left)"
  },
  {
    "acid_pool_topright",
    "wall",
    "Acid pool (top right)"
  },
  {
    "acid_pool_bottomleft",
    "wall",
    "Acid pool (bottom left)"
  },
  {
    "acid_pool_bottom",
    "wall",
    "Acid pool (bottom)"
  },
  {
    "acid_pool_bottomright",
    "wall",
    "Acid pool (bottom right)"
  },
  {
    "bd_wall",
    "wall",
    "Normal wall (BD style)"
  },
  {
    "bd_rock",
    "bd_rock",
    "Rock (BD style)"
  },
  {
    "exit_open",
    "exit",
    "Open exit"
  },
  {
    "black_orb",
    "black_orb",
    "Black orb bomb"
  },
  {
    "amoeba_to_diamond",
    "amoeba",
    "Amoeba"
  },
  {
    "mole",
    "mole",
    "Mole (random start direction)"
  },
  {
    "penguin",
    "penguin",
    "Penguin"
  },
  {
    "satellite",
    "satellite",
    "Satellite"
  },
  {
    "arrow_left",
    "arrow",
    "Arrow left"
  },
  {
    "arrow_right",
    "arrow",
    "Arrow right"
  },
  {
    "arrow_up",
    "arrow",
    "Arrow up"
  },
  {
    "arrow_down",
    "arrow",
    "Arrow down"
  },
  {
    "pig",
    "pig",
    "Pig"
  },
  {
    "dragon",
    "dragon",
    "Fire breathing dragon"
  },
  {
    "em_key_1_file_obsolete",
    "obsolete",
    "Key (obsolete)"
  },
  {
    "char_space",
    "char",
    "Letter ' '"
  },
  {
    "char_exclam",
    "char",
    "Letter '!'"
  },
  {
    "char_quotedbl",
    "char",
    "Letter '\"'"
  },
  {
    "char_numbersign",
    "char",
    "Letter '#'"
  },
  {
    "char_dollar",
    "char",
    "Letter '$'"
  },
  {
    "char_percent",
    "char",
    "Letter '%'"
  },
  {
    "char_ampersand",
    "char",
    "Letter '&'"
  },
  {
    "char_apostrophe",
    "char",
    "Letter '''"
  },
  {
    "char_parenleft",
    "char",
    "Letter '('"
  },
  {
    "char_parenright",
    "char",
    "Letter ')'"
  },
  {
    "char_asterisk",
    "char",
    "Letter '*'"
  },
  {
    "char_plus",
    "char",
    "Letter '+'"
  },
  {
    "char_comma",
    "char",
    "Letter ','"
  },
  {
    "char_minus",
    "char",
    "Letter '-'"
  },
  {
    "char_period",
    "char",
    "Letter '.'"
  },
  {
    "char_slash",
    "char",
    "Letter '/'"
  },
  {
    "char_0",
    "char",
    "Letter '0'"
  },
  {
    "char_1",
    "char",
    "Letter '1'"
  },
  {
    "char_2",
    "char",
    "Letter '2'"
  },
  {
    "char_3",
    "char",
    "Letter '3'"
  },
  {
    "char_4",
    "char",
    "Letter '4'"
  },
  {
    "char_5",
    "char",
    "Letter '5'"
  },
  {
    "char_6",
    "char",
    "Letter '6'"
  },
  {
    "char_7",
    "char",
    "Letter '7'"
  },
  {
    "char_8",
    "char",
    "Letter '8'"
  },
  {
    "char_9",
    "char",
    "Letter '9'"
  },
  {
    "char_colon",
    "char",
    "Letter ':'"
  },
  {
    "char_semicolon",
    "char",
    "Letter ';'"
  },
  {
    "char_less",
    "char",
    "Letter '<'"
  },
  {
    "char_equal",
    "char",
    "Letter '='"
  },
  {
    "char_greater",
    "char",
    "Letter '>'"
  },
  {
    "char_question",
    "char",
    "Letter '?'"
  },
  {
    "char_at",
    "char",
    "Letter '@'"
  },
  {
    "char_a",
    "char",
    "Letter 'A'"
  },
  {
    "char_b",
    "char",
    "Letter 'B'"
  },
  {
    "char_c",
    "char",
    "Letter 'C'"
  },
  {
    "char_d",
    "char",
    "Letter 'D'"
  },
  {
    "char_e",
    "char",
    "Letter 'E'"
  },
  {
    "char_f",
    "char",
    "Letter 'F'"
  },
  {
    "char_g",
    "char",
    "Letter 'G'"
  },
  {
    "char_h",
    "char",
    "Letter 'H'"
  },
  {
    "char_i",
    "char",
    "Letter 'I'"
  },
  {
    "char_j",
    "char",
    "Letter 'J'"
  },
  {
    "char_k",
    "char",
    "Letter 'K'"
  },
  {
    "char_l",
    "char",
    "Letter 'L'"
  },
  {
    "char_m",
    "char",
    "Letter 'M'"
  },
  {
    "char_n",
    "char",
    "Letter 'N'"
  },
  {
    "char_o",
    "char",
    "Letter 'O'"
  },
  {
    "char_p",
    "char",
    "Letter 'P'"
  },
  {
    "char_q",
    "char",
    "Letter 'Q'"
  },
  {
    "char_r",
    "char",
    "Letter 'R'"
  },
  {
    "char_s",
    "char",
    "Letter 'S'"
  },
  {
    "char_t",
    "char",
    "Letter 'T'"
  },
  {
    "char_u",
    "char",
    "Letter 'U'"
  },
  {
    "char_v",
    "char",
    "Letter 'V'"
  },
  {
    "char_w",
    "char",
    "Letter 'W'"
  },
  {
    "char_x",
    "char",
    "Letter 'X'"
  },
  {
    "char_y",
    "char",
    "Letter 'Y'"
  },
  {
    "char_z",
    "char",
    "Letter 'Z'"
  },
  {
    "char_bracketleft",
    "char",
    "Letter '['"
  },
  {
    "char_backslash",
    "char",
    "Letter '\\'"
  },
  {
    "char_bracketright",
    "char",
    "Letter ']'"
  },
  {
    "char_asciicircum",
    "char",
    "Letter '^'"
  },
  {
    "char_underscore",
    "char",
    "Letter '_'"
  },
  {
    "char_copyright",
    "char",
    "Letter '\xa9'"
  },
  {
    "char_aumlaut",
    "char",
    "Letter '\xc4'"
  },
  {
    "char_oumlaut",
    "char",
    "Letter '\xd6'"
  },
  {
    "char_uumlaut",
    "char",
    "Letter '\xdc'"
  },
  {
    "char_degree",
    "char",
    "Letter '\xb0'"
  },
  {
    "char_trademark",
    "char",
    "Letter '\xae'"
  },
  {
    "char_cursor",
    "char",
    "Letter '\xa0'"
  },
  {
    "char_unused",
    "char",
    "Letter ''"
  },
  {
    "char_unused",
    "char",
    "Letter ''"
  },
  {
    "char_unused",
    "char",
    "Letter ''"
  },
  {
    "char_unused",
    "char",
    "Letter ''"
  },
  {
    "char_unused",
    "char",
    "Letter ''"
  },
  {
    "char_unused",
    "char",
    "Letter ''"
  },
  {
    "char_button",
    "char",
    "Letter 'button'"
  },
  {
    "char_up",
    "char",
    "Letter 'up'"
  },
  {
    "char_down",
    "char",
    "Letter 'down'"
  },
  {
    "expandable_wall_horizontal",
    "wall",
    "Growing wall (horizontal)"
  },
  {
    "expandable_wall_vertical",
    "wall",
    "Growing wall (vertical)"
  },
  {
    "expandable_wall_any",
    "wall",
    "Growing wall (any direction)"
  },
  {
    "em_gate_1",
    "gate",
    "Door 1 (EM style)"
  },
  {
    "em_gate_2",
    "gate",
    "Door 2 (EM style)"
  },
  {
    "em_gate_3",
    "gate",
    "Door 3 (EM style)"
  },
  {
    "em_gate_4",
    "gate",
    "Door 4 (EM style)"
  },
  {
    "em_key_2_file_obsolete",
    "obsolete",
    "Key (obsolete)"
  },
  {
    "em_key_3_file_obsolete",
    "obsolete",
    "Key (obsolete)"
  },
  {
    "em_key_4_file_obsolete",
    "obsolete",
    "Key (obsolete)"
  },
  {
    "sp_empty_space",
    "empty_space",
    "Empty space"
  },
  {
    "sp_zonk",
    "sp_zonk",
    "Zonk"
  },
  {
    "sp_base",
    "sp_base",
    "Base"
  },
  {
    "sp_murphy",
    "player",
    "Murphy"
  },
  {
    "sp_infotron",
    "sp_infotron",
    "Infotron"
  },
  {
    "sp_chip_single",
    "wall",
    "Chip (single)"
  },
  {
    "sp_hardware_gray",
    "wall",
    "Hardware"
  },
  {
    "sp_exit_closed",
    "sp_exit",
    "Exit"
  },
  {
    "sp_disk_orange",
    "sp_disk_orange",
    "Orange disk"
  },
  {
    "sp_port_right",
    "sp_port",
    "Port (leading right)"
  },
  {
    "sp_port_down",
    "sp_port",
    "Port (leading down)"
  },
  {
    "sp_port_left",
    "sp_port",
    "Port (leading left)"
  },
  {
    "sp_port_up",
    "sp_port",
    "Port (leading up)"
  },
  {
    "sp_gravity_port_right",
    "sp_gravity_port",
    "Gravity-on/off port (leading right)"
  },
  {
    "sp_gravity_port_down",
    "sp_gravity_port",
    "Gravity-on/off port (leading down)"
  },
  {
    "sp_gravity_port_left",
    "sp_gravity_port",
    "Gravity-on/off port (leading left)"
  },
  {
    "sp_gravity_port_up",
    "sp_gravity_port",
    "Gravity-on/off port (leading up)"
  },
  {
    "sp_sniksnak",
    "sp_sniksnak",
    "Snik snak"
  },
  {
    "sp_disk_yellow",
    "sp_disk_yellow",
    "Yellow disk"
  },
  {
    "sp_terminal",
    "sp_terminal",
    "Terminal"
  },
  {
    "sp_disk_red",
    "dynamite",
    "Red disk"
  },
  {
    "sp_port_vertical",
    "sp_port",
    "Port (vertical)"
  },
  {
    "sp_port_horizontal",
    "sp_port",
    "Port (horizontal)"
  },
  {
    "sp_port_any",
    "sp_port",
    "Port (any direction)"
  },
  {
    "sp_electron",
    "sp_electron",
    "Electron"
  },
  {
    "sp_buggy_base",
    "sp_buggy_base",
    "Buggy base"
  },
  {
    "sp_chip_left",
    "wall",
    "Chip (left half)"
  },
  {
    "sp_chip_right",
    "wall",
    "Chip (right half)"
  },
  {
    "sp_hardware_base_1",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_green",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_blue",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_red",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_yellow",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_base_2",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_base_3",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_base_4",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_base_5",
    "wall",
    "Hardware"
  },
  {
    "sp_hardware_base_6",
    "wall",
    "Hardware"
  },
  {
    "sp_chip_top",
    "wall",
    "Chip (upper half)"
  },
  {
    "sp_chip_bottom",
    "wall",
    "Chip (lower half)"
  },
  {
    "em_gate_1_gray",
    "gate",
    "Gray door (EM style, key 1)"
  },
  {
    "em_gate_2_gray",
    "gate",
    "Gray door (EM style, key 2)"
  },
  {
    "em_gate_3_gray",
    "gate",
    "Gray door (EM style, key 3)"
  },
  {
    "em_gate_4_gray",
    "gate",
    "Gray door (EM style, key 4)"
  },
  {
    "em_dynamite",
    "dynamite",
    "Dynamite (EM style)"
  },
  {
    "em_dynamite.active",
    "dynamite",
    "Burning dynamite (EM style)"
  },
  {
    "pearl",
    "pearl",
    "Pearl"
  },
  {
    "crystal",
    "crystal",
    "Crystal"
  },
  {
    "wall_pearl",
    "wall",
    "Wall with pearl"
  },
  {
    "wall_crystal",
    "wall",
    "Wall with crystal"
  },
  {
    "dc_gate_white",
    "gate",
    "White door"
  },
  {
    "dc_gate_white_gray",
    "gate",
    "Gray door (opened by white key)"
  },
  {
    "dc_key_white",
    "key",
    "White key"
  },
  {
    "shield_normal",
    "shield_normal",
    "Shield (normal)"
  },
  {
    "extra_time",
    "extra_time",
    "Extra time"
  },
  {
    "switchgate_open",
    "switchgate",
    "Switch gate (open)"
  },
  {
    "switchgate_closed",
    "switchgate",
    "Switch gate (closed)"
  },
  {
    "switchgate_switch_up",
    "switchgate_switch",
    "Switch for switch gate"
  },
  {
    "switchgate_switch_down",
    "switchgate_switch",
    "Switch for switch gate"
  },
  {
    "unused_269",
    "unused",
    "-"
  },
  {
    "unused_270",
    "unused",
    "-"
  },
  {
    "conveyor_belt_1_left",
    "conveyor_belt",
    "Conveyor belt 1 (left)"
  },
  {
    "conveyor_belt_1_middle",
    "conveyor_belt",
    "Conveyor belt 1 (middle)"
  },
  {
    "conveyor_belt_1_right",
    "conveyor_belt",
    "Conveyor belt 1 (right)"
  },
  {
    "conveyor_belt_1_switch_left",
    "conveyor_belt_switch",
    "Switch for conveyor belt 1 (left)"
  },
  {
    "conveyor_belt_1_switch_middle",
    "conveyor_belt_switch",
    "Switch for conveyor belt 1 (middle)"
  },
  {
    "conveyor_belt_1_switch_right",
    "conveyor_belt_switch",
    "Switch for conveyor belt 1 (right)"
  },
  {
    "conveyor_belt_2_left",
    "conveyor_belt",
    "Conveyor belt 2 (left)"
  },
  {
    "conveyor_belt_2_middle",
    "conveyor_belt",
    "Conveyor belt 2 (middle)"
  },
  {
    "conveyor_belt_2_right",
    "conveyor_belt",
    "Conveyor belt 2 (right)"
  },
  {
    "conveyor_belt_2_switch_left",
    "conveyor_belt_switch",
    "Switch for conveyor belt 2 (left)"
  },
  {
    "conveyor_belt_2_switch_middle",
    "conveyor_belt_switch",
    "Switch for conveyor belt 2 (middle)"
  },
  {
    "conveyor_belt_2_switch_right",
    "conveyor_belt_switch",
    "Switch for conveyor belt 2 (right)"
  },
  {
    "conveyor_belt_3_left",
    "conveyor_belt",
    "Conveyor belt 3 (left)"
  },
  {
    "conveyor_belt_3_middle",
    "conveyor_belt",
    "Conveyor belt 3 (middle)"
  },
  {
    "conveyor_belt_3_right",
    "conveyor_belt",
    "Conveyor belt 3 (right)"
  },
  {
    "conveyor_belt_3_switch_left",
    "conveyor_belt_switch",
    "Switch for conveyor belt 3 (left)"
  },
  {
    "conveyor_belt_3_switch_middle",
    "conveyor_belt_switch",
    "Switch for conveyor belt 3 (middle)"
  },
  {
    "conveyor_belt_3_switch_right",
    "conveyor_belt_switch",
    "Switch for conveyor belt 3 (right)"
  },
  {
    "conveyor_belt_4_left",
    "conveyor_belt",
    "Conveyor belt 4 (left)"
  },
  {
    "conveyor_belt_4_middle",
    "conveyor_belt",
    "Conveyor belt 4 (middle)"
  },
  {
    "conveyor_belt_4_right",
    "conveyor_belt",
    "Conveyor belt 4 (right)"
  },
  {
    "conveyor_belt_4_switch_left",
    "conveyor_belt_switch",
    "Switch for conveyor belt 4 (left)"
  },
  {
    "conveyor_belt_4_switch_middle",
    "conveyor_belt_switch",
    "Switch for conveyor belt 4 (middle)"
  },
  {
    "conveyor_belt_4_switch_right",
    "conveyor_belt_switch",
    "Switch for conveyor belt 4 (right)"
  },
  {
    "landmine",
    "landmine",
    "Land mine (not removable)"
  },
  {
    "envelope_obsolete",
    "obsolete",
    "Envelope (obsolete)"
  },
  {
    "light_switch",
    "light_switch",
    "Light switch (off)"
  },
  {
    "light_switch.active",
    "light_switch",
    "Light switch (on)"
  },
  {
    "sign_exclamation",
    "sign",
    "Sign (exclamation)"
  },
  {
    "sign_radioactivity",
    "sign",
    "Sign (radio activity)"
  },
  {
    "sign_stop",
    "sign",
    "Sign (stop)"
  },
  {
    "sign_wheelchair",
    "sign",
    "Sign (wheel chair)"
  },
  {
    "sign_parking",
    "sign",
    "Sign (parking)"
  },
  {
    "sign_no_entry",
    "sign",
    "Sign (no entry)"
  },
  {
    "sign_unused_1",
    "sign",
    "Sign (unused)"
  },
  {
    "sign_give_way",
    "sign",
    "Sign (give way)"
  },
  {
    "sign_entry_forbidden",
    "sign",
    "Sign (entry forbidden)"
  },
  {
    "sign_emergency_exit",
    "sign",
    "Sign (emergency exit)"
  },
  {
    "sign_yin_yang",
    "sign",
    "Sign (yin yang)"
  },
  {
    "sign_unused_2",
    "sign",
    "Sign (unused)"
  },
  {
    "mole.left",
    "mole",
    "Mole (starts moving left)"
  },
  {
    "mole.right",
    "mole",
    "Mole (starts moving right)"
  },
  {
    "mole.up",
    "mole",
    "Mole (starts moving up)"
  },
  {
    "mole.down",
    "mole",
    "Mole (starts moving down)"
  },
  {
    "steelwall_slippery",
    "steelwall",
    "Slippery steel wall"
  },
  {
    "invisible_sand",
    "sand",
    "Invisible sand"
  },
  {
    "dx_unknown_15",
    "unknown",
    "DX unknown element 15"
  },
  {
    "dx_unknown_42",
    "unknown",
    "DX unknown element 42"
  },
  {
    "unused_319",
    "unused",
    "(not used)"
  },
  {
    "unused_320",
    "unused",
    "(not used)"
  },
  {
    "shield_deadly",
    "shield_deadly",
    "Shield (deadly, kills enemies)"
  },
  {
    "timegate_open",
    "timegate",
    "Time gate (open)"
  },
  {
    "timegate_closed",
    "timegate",
    "Time gate (closed)"
  },
  {
    "timegate_switch.active",
    "timegate_switch",
    "Switch for time gate"
  },
  {
    "timegate_switch",
    "timegate_switch",
    "Switch for time gate"
  },
  {
    "balloon",
    "balloon",
    "Balloon"
  },
  {
    "balloon_switch_left",
    "balloon_switch",
    "Wind switch (left)"
  },
  {
    "balloon_switch_right",
    "balloon_switch",
    "Wind switch (right)"
  },
  {
    "balloon_switch_up",
    "balloon_switch",
    "Wind switch (up)"
  },
  {
    "balloon_switch_down",
    "balloon_switch",
    "Wind switch (down)"
  },
  {
    "balloon_switch_any",
    "balloon_switch",
    "Wind switch (any direction)"
  },
  {
    "emc_steelwall_1",
    "steelwall",
    "Steel wall 1 (EMC style)"
  },
  {
    "emc_steelwall_2",
    "steelwall",
    "Steel wall 2 (EMC style)"
  },
  {
    "emc_steelwall_3",
    "steelwall",
    "Steel wall 3 (EMC style)"
  },
  {
    "emc_steelwall_4",
    "steelwall",
    "Steel wall 4 (EMC style)"
  },
  {
    "emc_wall_1",
    "wall",
    "Normal wall 1 (EMC style)"
  },
  {
    "emc_wall_2",
    "wall",
    "Normal wall 2 (EMC style)"
  },
  {
    "emc_wall_3",
    "wall",
    "Normal wall 3 (EMC style)"
  },
  {
    "emc_wall_4",
    "wall",
    "Normal wall 4 (EMC style)"
  },
  {
    "emc_wall_5",
    "wall",
    "Normal wall 5 (EMC style)"
  },
  {
    "emc_wall_6",
    "wall",
    "Normal wall 6 (EMC style)"
  },
  {
    "emc_wall_7",
    "wall",
    "Normal wall 7 (EMC style)"
  },
  {
    "emc_wall_8",
    "wall",
    "Normal wall 8 (EMC style)"
  },
  {
    "tube_any",
    "tube",
    "Tube (any direction)"
  },
  {
    "tube_vertical",
    "tube",
    "Tube (vertical)"
  },
  {
    "tube_horizontal",
    "tube",
    "Tube (horizontal)"
  },
  {
    "tube_vertical_left",
    "tube",
    "Tube (vertical & left)"
  },
  {
    "tube_vertical_right",
    "tube",
    "Tube (vertical & right)"
  },
  {
    "tube_horizontal_up",
    "tube",
    "Tube (horizontal & up)"
  },
  {
    "tube_horizontal_down",
    "tube",
    "Tube (horizontal & down)"
  },
  {
    "tube_left_up",
    "tube",
    "Tube (left & up)"
  },
  {
    "tube_left_down",
    "tube",
    "Tube (left & down)"
  },
  {
    "tube_right_up",
    "tube",
    "Tube (right & up)"
  },
  {
    "tube_right_down",
    "tube",
    "Tube (right & down)"
  },
  {
    "spring",
    "spring",
    "Spring"
  },
  {
    "trap",
    "trap",
    "Trap"
  },
  {
    "dx_supabomb",
    "bomb",
    "Stable bomb (DX style)"
  },
  {
    "unused_358",
    "unused",
    "-"
  },
  {
    "unused_359",
    "unused",
    "-"
  },
  {
    "custom_1",
    "custom",
    "Custom element 1"
  },
  {
    "custom_2",
    "custom",
    "Custom element 2"
  },
  {
    "custom_3",
    "custom",
    "Custom element 3"
  },
  {
    "custom_4",
    "custom",
    "Custom element 4"
  },
  {
    "custom_5",
    "custom",
    "Custom element 5"
  },
  {
    "custom_6",
    "custom",
    "Custom element 6"
  },
  {
    "custom_7",
    "custom",
    "Custom element 7"
  },
  {
    "custom_8",
    "custom",
    "Custom element 8"
  },
  {
    "custom_9",
    "custom",
    "Custom element 9"
  },
  {
    "custom_10",
    "custom",
    "Custom element 10"
  },
  {
    "custom_11",
    "custom",
    "Custom element 11"
  },
  {
    "custom_12",
    "custom",
    "Custom element 12"
  },
  {
    "custom_13",
    "custom",
    "Custom element 13"
  },
  {
    "custom_14",
    "custom",
    "Custom element 14"
  },
  {
    "custom_15",
    "custom",
    "Custom element 15"
  },
  {
    "custom_16",
    "custom",
    "Custom element 16"
  },
  {
    "custom_17",
    "custom",
    "Custom element 17"
  },
  {
    "custom_18",
    "custom",
    "Custom element 18"
  },
  {
    "custom_19",
    "custom",
    "Custom element 19"
  },
  {
    "custom_20",
    "custom",
    "Custom element 20"
  },
  {
    "custom_21",
    "custom",
    "Custom element 21"
  },
  {
    "custom_22",
    "custom",
    "Custom element 22"
  },
  {
    "custom_23",
    "custom",
    "Custom element 23"
  },
  {
    "custom_24",
    "custom",
    "Custom element 24"
  },
  {
    "custom_25",
    "custom",
    "Custom element 25"
  },
  {
    "custom_26",
    "custom",
    "Custom element 26"
  },
  {
    "custom_27",
    "custom",
    "Custom element 27"
  },
  {
    "custom_28",
    "custom",
    "Custom element 28"
  },
  {
    "custom_29",
    "custom",
    "Custom element 29"
  },
  {
    "custom_30",
    "custom",
    "Custom element 30"
  },
  {
    "custom_31",
    "custom",
    "Custom element 31"
  },
  {
    "custom_32",
    "custom",
    "Custom element 32"
  },
  {
    "custom_33",
    "custom",
    "Custom element 33"
  },
  {
    "custom_34",
    "custom",
    "Custom element 34"
  },
  {
    "custom_35",
    "custom",
    "Custom element 35"
  },
  {
    "custom_36",
    "custom",
    "Custom element 36"
  },
  {
    "custom_37",
    "custom",
    "Custom element 37"
  },
  {
    "custom_38",
    "custom",
    "Custom element 38"
  },
  {
    "custom_39",
    "custom",
    "Custom element 39"
  },
  {
    "custom_40",
    "custom",
    "Custom element 40"
  },
  {
    "custom_41",
    "custom",
    "Custom element 41"
  },
  {
    "custom_42",
    "custom",
    "Custom element 42"
  },
  {
    "custom_43",
    "custom",
    "Custom element 43"
  },
  {
    "custom_44",
    "custom",
    "Custom element 44"
  },
  {
    "custom_45",
    "custom",
    "Custom element 45"
  },
  {
    "custom_46",
    "custom",
    "Custom element 46"
  },
  {
    "custom_47",
    "custom",
    "Custom element 47"
  },
  {
    "custom_48",
    "custom",
    "Custom element 48"
  },
  {
    "custom_49",
    "custom",
    "Custom element 49"
  },
  {
    "custom_50",
    "custom",
    "Custom element 50"
  },
  {
    "custom_51",
    "custom",
    "Custom element 51"
  },
  {
    "custom_52",
    "custom",
    "Custom element 52"
  },
  {
    "custom_53",
    "custom",
    "Custom element 53"
  },
  {
    "custom_54",
    "custom",
    "Custom element 54"
  },
  {
    "custom_55",
    "custom",
    "Custom element 55"
  },
  {
    "custom_56",
    "custom",
    "Custom element 56"
  },
  {
    "custom_57",
    "custom",
    "Custom element 57"
  },
  {
    "custom_58",
    "custom",
    "Custom element 58"
  },
  {
    "custom_59",
    "custom",
    "Custom element 59"
  },
  {
    "custom_60",
    "custom",
    "Custom element 60"
  },
  {
    "custom_61",
    "custom",
    "Custom element 61"
  },
  {
    "custom_62",
    "custom",
    "Custom element 62"
  },
  {
    "custom_63",
    "custom",
    "Custom element 63"
  },
  {
    "custom_64",
    "custom",
    "Custom element 64"
  },
  {
    "custom_65",
    "custom",
    "Custom element 65"
  },
  {
    "custom_66",
    "custom",
    "Custom element 66"
  },
  {
    "custom_67",
    "custom",
    "Custom element 67"
  },
  {
    "custom_68",
    "custom",
    "Custom element 68"
  },
  {
    "custom_69",
    "custom",
    "Custom element 69"
  },
  {
    "custom_70",
    "custom",
    "Custom element 70"
  },
  {
    "custom_71",
    "custom",
    "Custom element 71"
  },
  {
    "custom_72",
    "custom",
    "Custom element 72"
  },
  {
    "custom_73",
    "custom",
    "Custom element 73"
  },
  {
    "custom_74",
    "custom",
    "Custom element 74"
  },
  {
    "custom_75",
    "custom",
    "Custom element 75"
  },
  {
    "custom_76",
    "custom",
    "Custom element 76"
  },
  {
    "custom_77",
    "custom",
    "Custom element 77"
  },
  {
    "custom_78",
    "custom",
    "Custom element 78"
  },
  {
    "custom_79",
    "custom",
    "Custom element 79"
  },
  {
    "custom_80",
    "custom",
    "Custom element 80"
  },
  {
    "custom_81",
    "custom",
    "Custom element 81"
  },
  {
    "custom_82",
    "custom",
    "Custom element 82"
  },
  {
    "custom_83",
    "custom",
    "Custom element 83"
  },
  {
    "custom_84",
    "custom",
    "Custom element 84"
  },
  {
    "custom_85",
    "custom",
    "Custom element 85"
  },
  {
    "custom_86",
    "custom",
    "Custom element 86"
  },
  {
    "custom_87",
    "custom",
    "Custom element 87"
  },
  {
    "custom_88",
    "custom",
    "Custom element 88"
  },
  {
    "custom_89",
    "custom",
    "Custom element 89"
  },
  {
    "custom_90",
    "custom",
    "Custom element 90"
  },
  {
    "custom_91",
    "custom",
    "Custom element 91"
  },
  {
    "custom_92",
    "custom",
    "Custom element 92"
  },
  {
    "custom_93",
    "custom",
    "Custom element 93"
  },
  {
    "custom_94",
    "custom",
    "Custom element 94"
  },
  {
    "custom_95",
    "custom",
    "Custom element 95"
  },
  {
    "custom_96",
    "custom",
    "Custom element 96"
  },
  {
    "custom_97",
    "custom",
    "Custom element 97"
  },
  {
    "custom_98",
    "custom",
    "Custom element 98"
  },
  {
    "custom_99",
    "custom",
    "Custom element 99"
  },
  {
    "custom_100",
    "custom",
    "Custom element 100"
  },
  {
    "custom_101",
    "custom",
    "Custom element 101"
  },
  {
    "custom_102",
    "custom",
    "Custom element 102"
  },
  {
    "custom_103",
    "custom",
    "Custom element 103"
  },
  {
    "custom_104",
    "custom",
    "Custom element 104"
  },
  {
    "custom_105",
    "custom",
    "Custom element 105"
  },
  {
    "custom_106",
    "custom",
    "Custom element 106"
  },
  {
    "custom_107",
    "custom",
    "Custom element 107"
  },
  {
    "custom_108",
    "custom",
    "Custom element 108"
  },
  {
    "custom_109",
    "custom",
    "Custom element 109"
  },
  {
    "custom_110",
    "custom",
    "Custom element 110"
  },
  {
    "custom_111",
    "custom",
    "Custom element 111"
  },
  {
    "custom_112",
    "custom",
    "Custom element 112"
  },
  {
    "custom_113",
    "custom",
    "Custom element 113"
  },
  {
    "custom_114",
    "custom",
    "Custom element 114"
  },
  {
    "custom_115",
    "custom",
    "Custom element 115"
  },
  {
    "custom_116",
    "custom",
    "Custom element 116"
  },
  {
    "custom_117",
    "custom",
    "Custom element 117"
  },
  {
    "custom_118",
    "custom",
    "Custom element 118"
  },
  {
    "custom_119",
    "custom",
    "Custom element 119"
  },
  {
    "custom_120",
    "custom",
    "Custom element 120"
  },
  {
    "custom_121",
    "custom",
    "Custom element 121"
  },
  {
    "custom_122",
    "custom",
    "Custom element 122"
  },
  {
    "custom_123",
    "custom",
    "Custom element 123"
  },
  {
    "custom_124",
    "custom",
    "Custom element 124"
  },
  {
    "custom_125",
    "custom",
    "Custom element 125"
  },
  {
    "custom_126",
    "custom",
    "Custom element 126"
  },
  {
    "custom_127",
    "custom",
    "Custom element 127"
  },
  {
    "custom_128",
    "custom",
    "Custom element 128"
  },
  {
    "custom_129",
    "custom",
    "Custom element 129"
  },
  {
    "custom_130",
    "custom",
    "Custom element 130"
  },
  {
    "custom_131",
    "custom",
    "Custom element 131"
  },
  {
    "custom_132",
    "custom",
    "Custom element 132"
  },
  {
    "custom_133",
    "custom",
    "Custom element 133"
  },
  {
    "custom_134",
    "custom",
    "Custom element 134"
  },
  {
    "custom_135",
    "custom",
    "Custom element 135"
  },
  {
    "custom_136",
    "custom",
    "Custom element 136"
  },
  {
    "custom_137",
    "custom",
    "Custom element 137"
  },
  {
    "custom_138",
    "custom",
    "Custom element 138"
  },
  {
    "custom_139",
    "custom",
    "Custom element 139"
  },
  {
    "custom_140",
    "custom",
    "Custom element 140"
  },
  {
    "custom_141",
    "custom",
    "Custom element 141"
  },
  {
    "custom_142",
    "custom",
    "Custom element 142"
  },
  {
    "custom_143",
    "custom",
    "Custom element 143"
  },
  {
    "custom_144",
    "custom",
    "Custom element 144"
  },
  {
    "custom_145",
    "custom",
    "Custom element 145"
  },
  {
    "custom_146",
    "custom",
    "Custom element 146"
  },
  {
    "custom_147",
    "custom",
    "Custom element 147"
  },
  {
    "custom_148",
    "custom",
    "Custom element 148"
  },
  {
    "custom_149",
    "custom",
    "Custom element 149"
  },
  {
    "custom_150",
    "custom",
    "Custom element 150"
  },
  {
    "custom_151",
    "custom",
    "Custom element 151"
  },
  {
    "custom_152",
    "custom",
    "Custom element 152"
  },
  {
    "custom_153",
    "custom",
    "Custom element 153"
  },
  {
    "custom_154",
    "custom",
    "Custom element 154"
  },
  {
    "custom_155",
    "custom",
    "Custom element 155"
  },
  {
    "custom_156",
    "custom",
    "Custom element 156"
  },
  {
    "custom_157",
    "custom",
    "Custom element 157"
  },
  {
    "custom_158",
    "custom",
    "Custom element 158"
  },
  {
    "custom_159",
    "custom",
    "Custom element 159"
  },
  {
    "custom_160",
    "custom",
    "Custom element 160"
  },
  {
    "custom_161",
    "custom",
    "Custom element 161"
  },
  {
    "custom_162",
    "custom",
    "Custom element 162"
  },
  {
    "custom_163",
    "custom",
    "Custom element 163"
  },
  {
    "custom_164",
    "custom",
    "Custom element 164"
  },
  {
    "custom_165",
    "custom",
    "Custom element 165"
  },
  {
    "custom_166",
    "custom",
    "Custom element 166"
  },
  {
    "custom_167",
    "custom",
    "Custom element 167"
  },
  {
    "custom_168",
    "custom",
    "Custom element 168"
  },
  {
    "custom_169",
    "custom",
    "Custom element 169"
  },
  {
    "custom_170",
    "custom",
    "Custom element 170"
  },
  {
    "custom_171",
    "custom",
    "Custom element 171"
  },
  {
    "custom_172",
    "custom",
    "Custom element 172"
  },
  {
    "custom_173",
    "custom",
    "Custom element 173"
  },
  {
    "custom_174",
    "custom",
    "Custom element 174"
  },
  {
    "custom_175",
    "custom",
    "Custom element 175"
  },
  {
    "custom_176",
    "custom",
    "Custom element 176"
  },
  {
    "custom_177",
    "custom",
    "Custom element 177"
  },
  {
    "custom_178",
    "custom",
    "Custom element 178"
  },
  {
    "custom_179",
    "custom",
    "Custom element 179"
  },
  {
    "custom_180",
    "custom",
    "Custom element 180"
  },
  {
    "custom_181",
    "custom",
    "Custom element 181"
  },
  {
    "custom_182",
    "custom",
    "Custom element 182"
  },
  {
    "custom_183",
    "custom",
    "Custom element 183"
  },
  {
    "custom_184",
    "custom",
    "Custom element 184"
  },
  {
    "custom_185",
    "custom",
    "Custom element 185"
  },
  {
    "custom_186",
    "custom",
    "Custom element 186"
  },
  {
    "custom_187",
    "custom",
    "Custom element 187"
  },
  {
    "custom_188",
    "custom",
    "Custom element 188"
  },
  {
    "custom_189",
    "custom",
    "Custom element 189"
  },
  {
    "custom_190",
    "custom",
    "Custom element 190"
  },
  {
    "custom_191",
    "custom",
    "Custom element 191"
  },
  {
    "custom_192",
    "custom",
    "Custom element 192"
  },
  {
    "custom_193",
    "custom",
    "Custom element 193"
  },
  {
    "custom_194",
    "custom",
    "Custom element 194"
  },
  {
    "custom_195",
    "custom",
    "Custom element 195"
  },
  {
    "custom_196",
    "custom",
    "Custom element 196"
  },
  {
    "custom_197",
    "custom",
    "Custom element 197"
  },
  {
    "custom_198",
    "custom",
    "Custom element 198"
  },
  {
    "custom_199",
    "custom",
    "Custom element 199"
  },
  {
    "custom_200",
    "custom",
    "Custom element 200"
  },
  {
    "custom_201",
    "custom",
    "Custom element 201"
  },
  {
    "custom_202",
    "custom",
    "Custom element 202"
  },
  {
    "custom_203",
    "custom",
    "Custom element 203"
  },
  {
    "custom_204",
    "custom",
    "Custom element 204"
  },
  {
    "custom_205",
    "custom",
    "Custom element 205"
  },
  {
    "custom_206",
    "custom",
    "Custom element 206"
  },
  {
    "custom_207",
    "custom",
    "Custom element 207"
  },
  {
    "custom_208",
    "custom",
    "Custom element 208"
  },
  {
    "custom_209",
    "custom",
    "Custom element 209"
  },
  {
    "custom_210",
    "custom",
    "Custom element 210"
  },
  {
    "custom_211",
    "custom",
    "Custom element 211"
  },
  {
    "custom_212",
    "custom",
    "Custom element 212"
  },
  {
    "custom_213",
    "custom",
    "Custom element 213"
  },
  {
    "custom_214",
    "custom",
    "Custom element 214"
  },
  {
    "custom_215",
    "custom",
    "Custom element 215"
  },
  {
    "custom_216",
    "custom",
    "Custom element 216"
  },
  {
    "custom_217",
    "custom",
    "Custom element 217"
  },
  {
    "custom_218",
    "custom",
    "Custom element 218"
  },
  {
    "custom_219",
    "custom",
    "Custom element 219"
  },
  {
    "custom_220",
    "custom",
    "Custom element 220"
  },
  {
    "custom_221",
    "custom",
    "Custom element 221"
  },
  {
    "custom_222",
    "custom",
    "Custom element 222"
  },
  {
    "custom_223",
    "custom",
    "Custom element 223"
  },
  {
    "custom_224",
    "custom",
    "Custom element 224"
  },
  {
    "custom_225",
    "custom",
    "Custom element 225"
  },
  {
    "custom_226",
    "custom",
    "Custom element 226"
  },
  {
    "custom_227",
    "custom",
    "Custom element 227"
  },
  {
    "custom_228",
    "custom",
    "Custom element 228"
  },
  {
    "custom_229",
    "custom",
    "Custom element 229"
  },
  {
    "custom_230",
    "custom",
    "Custom element 230"
  },
  {
    "custom_231",
    "custom",
    "Custom element 231"
  },
  {
    "custom_232",
    "custom",
    "Custom element 232"
  },
  {
    "custom_233",
    "custom",
    "Custom element 233"
  },
  {
    "custom_234",
    "custom",
    "Custom element 234"
  },
  {
    "custom_235",
    "custom",
    "Custom element 235"
  },
  {
    "custom_236",
    "custom",
    "Custom element 236"
  },
  {
    "custom_237",
    "custom",
    "Custom element 237"
  },
  {
    "custom_238",
    "custom",
    "Custom element 238"
  },
  {
    "custom_239",
    "custom",
    "Custom element 239"
  },
  {
    "custom_240",
    "custom",
    "Custom element 240"
  },
  {
    "custom_241",
    "custom",
    "Custom element 241"
  },
  {
    "custom_242",
    "custom",
    "Custom element 242"
  },
  {
    "custom_243",
    "custom",
    "Custom element 243"
  },
  {
    "custom_244",
    "custom",
    "Custom element 244"
  },
  {
    "custom_245",
    "custom",
    "Custom element 245"
  },
  {
    "custom_246",
    "custom",
    "Custom element 246"
  },
  {
    "custom_247",
    "custom",
    "Custom element 247"
  },
  {
    "custom_248",
    "custom",
    "Custom element 248"
  },
  {
    "custom_249",
    "custom",
    "Custom element 249"
  },
  {
    "custom_250",
    "custom",
    "Custom element 250"
  },
  {
    "custom_251",
    "custom",
    "Custom element 251"
  },
  {
    "custom_252",
    "custom",
    "Custom element 252"
  },
  {
    "custom_253",
    "custom",
    "Custom element 253"
  },
  {
    "custom_254",
    "custom",
    "Custom element 254"
  },
  {
    "custom_255",
    "custom",
    "Custom element 255"
  },
  {
    "custom_256",
    "custom",
    "Custom element 256"
  },
  {
    "em_key_1",
    "key",
    "Key 1 (EM style)"
    },
  {
    "em_key_2",
    "key",
    "Key 2 (EM style)"
    },
  {
    "em_key_3",
    "key",
    "Key 3 (EM style)"
  },
  {
    "em_key_4",
    "key",
    "Key 4 (EM style)"
  },
  {
    "envelope_1",
    "envelope",
    "Mail envelope 1"
  },
  {
    "envelope_2",
    "envelope",
    "Mail envelope 2"
  },
  {
    "envelope_3",
    "envelope",
    "Mail envelope 3"
  },
  {
    "envelope_4",
    "envelope",
    "Mail envelope 4"
  },
  {
    "group_1",
    "group",
    "Group element 1"
  },
  {
    "group_2",
    "group",
    "Group element 2"
  },
  {
    "group_3",
    "group",
    "Group element 3"
  },
  {
    "group_4",
    "group",
    "Group element 4"
  },
  {
    "group_5",
    "group",
    "Group element 5"
  },
  {
    "group_6",
    "group",
    "Group element 6"
  },
  {
    "group_7",
    "group",
    "Group element 7"
  },
  {
    "group_8",
    "group",
    "Group element 8"
  },
  {
    "group_9",
    "group",
    "Group element 9"
  },
  {
    "group_10",
    "group",
    "Group element 10"
  },
  {
    "group_11",
    "group",
    "Group element 11"
  },
  {
    "group_12",
    "group",
    "Group element 12"
  },
  {
    "group_13",
    "group",
    "Group element 13"
  },
  {
    "group_14",
    "group",
    "Group element 14"
  },
  {
    "group_15",
    "group",
    "Group element 15"
  },
  {
    "group_16",
    "group",
    "Group element 16"
  },
  {
    "group_17",
    "group",
    "Group element 17"
  },
  {
    "group_18",
    "group",
    "Group element 18"
  },
  {
    "group_19",
    "group",
    "Group element 19"
  },
  {
    "group_20",
    "group",
    "Group element 20"
  },
  {
    "group_21",
    "group",
    "Group element 21"
  },
  {
    "group_22",
    "group",
    "Group element 22"
  },
  {
    "group_23",
    "group",
    "Group element 23"
  },
  {
    "group_24",
    "group",
    "Group element 24"
  },
  {
    "group_25",
    "group",
    "Group element 25"
  },
  {
    "group_26",
    "group",
    "Group element 26"
  },
  {
    "group_27",
    "group",
    "Group element 27"
  },
  {
    "group_28",
    "group",
    "Group element 28"
  },
  {
    "group_29",
    "group",
    "Group element 29"
  },
  {
    "group_30",
    "group",
    "Group element 30"
  },
  {
    "group_31",
    "group",
    "Group element 31"
  },
  {
    "group_32",
    "group",
    "Group element 32"
  },
  {
    "unknown",
    "unknown",
    "Unknown element"
  },
  {
    "trigger_element",
    "trigger",
    "Element triggering change"
  },
  {
    "trigger_player",
    "trigger",
    "Player triggering change"
  },
  {
    "sp_gravity_on_port_right",
    "sp_gravity_on_port",
    "Gravity-on port (leading right)"
  },
  {
    "sp_gravity_on_port_down",
    "sp_gravity_on_port",
    "Gravity-on port (leading down)"
  },
  {
    "sp_gravity_on_port_left",
    "sp_gravity_on_port",
    "Gravity-on port (leading left)"
  },
  {
    "sp_gravity_on_port_up",
    "sp_gravity_on_port",
    "Gravity-on port (leading up)"
  },
  {
    "sp_gravity_off_port_right",
    "sp_gravity_off_port",
    "Gravity-off port (leading right)"
  },
  {
    "sp_gravity_off_port_down",
    "sp_gravity_off_port",
    "Gravity-off port (leading down)"
  },
  {
    "sp_gravity_off_port_left",
    "sp_gravity_off_port",
    "Gravity-off port (leading left)"
  },
  {
    "sp_gravity_off_port_up",
    "sp_gravity_off_port",
    "Gravity-off port (leading up)"
  },
  {
    "balloon_switch_none",
    "balloon_switch",
    "Wind switch (off)"
  },
  {
    "emc_gate_5",
    "gate",
    "Door 5 (EMC style)",
  },
  {
    "emc_gate_6",
    "gate",
    "Door 6 (EMC style)",
  },
  {
    "emc_gate_7",
    "gate",
    "Door 7 (EMC style)",
  },
  {
    "emc_gate_8",
    "gate",
    "Door 8 (EMC style)",
  },
  {
    "emc_gate_5_gray",
    "gate",
    "Gray door (EMC style, key 5)",
  },
  {
    "emc_gate_6_gray",
    "gate",
    "Gray door (EMC style, key 6)",
  },
  {
    "emc_gate_7_gray",
    "gate",
    "Gray door (EMC style, key 7)",
  },
  {
    "emc_gate_8_gray",
    "gate",
    "Gray door (EMC style, key 8)",
  },
  {
    "emc_key_5",
    "key",
    "Key 5 (EMC style)",
  },
  {
    "emc_key_6",
    "key",
    "Key 6 (EMC style)",
  },
  {
    "emc_key_7",
    "key",
    "Key 7 (EMC style)",
  },
  {
    "emc_key_8",
    "key",
    "Key 8 (EMC style)",
  },
  {
    "emc_android",
    "emc_android",
    "Android",
  },
  {
    "emc_grass",
    "emc_grass",
    "Grass",
  },
  {
    "emc_magic_ball",
    "emc_magic_ball",
    "Magic ball",
  },
  {
    "emc_magic_ball.active",
    "emc_magic_ball",
    "Magic ball (activated)",
  },
  {
    "emc_magic_ball_switch",
    "emc_magic_ball_switch",
    "Magic ball switch (off)",
  },
  {
    "emc_magic_ball_switch.active",
    "emc_magic_ball_switch",
    "Magic ball switch (on)",
  },
  {
    "emc_spring_bumper",
    "emc_spring_bumper",
    "Spring bumper",
  },
  {
    "emc_plant",
    "emc_plant",
    "Plant",
  },
  {
    "emc_lenses",
    "emc_lenses",
    "Lenses",
  },
  {
    "emc_magnifier",
    "emc_magnifier",
    "Magnifier",
  },
  {
    "emc_wall_9",
    "wall",
    "Normal wall 9 (EMC style)"
  },
  {
    "emc_wall_10",
    "wall",
    "Normal wall 10 (EMC style)"
  },
  {
    "emc_wall_11",
    "wall",
    "Normal wall 11 (EMC style)"
  },
  {
    "emc_wall_12",
    "wall",
    "Normal wall 12 (EMC style)"
  },
  {
    "emc_wall_13",
    "wall",
    "Normal wall 13 (EMC style)"
  },
  {
    "emc_wall_14",
    "wall",
    "Normal wall 14 (EMC style)"
  },
  {
    "emc_wall_15",
    "wall",
    "Normal wall 15 (EMC style)"
  },
  {
    "emc_wall_16",
    "wall",
    "Normal wall 16 (EMC style)"
  },
  {
    "emc_wall_slippery_1",
    "wall",
    "Slippery wall 1 (EMC style)"
  },
  {
    "emc_wall_slippery_2",
    "wall",
    "Slippery wall 2 (EMC style)"
  },
  {
    "emc_wall_slippery_3",
    "wall",
    "Slippery wall 3 (EMC style)"
  },
  {
    "emc_wall_slippery_4",
    "wall",
    "Slippery wall 4 (EMC style)"
  },
  {
    "emc_fake_grass",
    "fake_grass",
    "Fake grass"
  },
  {
    "emc_fake_acid",
    "fake_acid",
    "Fake acid"
  },
  {
    "emc_dripper",
    "dripper",
    "Dripper"
  },
  {
    "trigger_ce_value",
    "trigger",
    "CE value of element triggering change"
  },
  {
    "trigger_ce_score",
    "trigger",
    "CE score of element triggering change"
  },
  {
    "current_ce_value",
    "current",
    "CE value of current element"
  },
  {
    "current_ce_score",
    "current",
    "CE score of current element"
  },
  {
    "yamyam.left",
    "yamyam",
    "Yam yam (starts moving left)"
  },
  {
    "yamyam.right",
    "yamyam",
    "Yam yam (starts moving right)"
  },
  {
    "yamyam.up",
    "yamyam",
    "Yam yam (starts moving up)"
  },
  {
    "yamyam.down",
    "yamyam",
    "Yam yam (starts moving down)"
  },
  {
    "bd_expandable_wall",
    "bd_expandable_wall",
    "Growing wall (horizontal, BD style)"
  },
  {
    "prev_ce_8",
    "prev_ce",
    "CE 8 positions earlier in list"
  },
  {
    "prev_ce_7",
    "prev_ce",
    "CE 7 positions earlier in list"
  },
  {
    "prev_ce_6",
    "prev_ce",
    "CE 6 positions earlier in list"
  },
  {
    "prev_ce_5",
    "prev_ce",
    "CE 5 positions earlier in list"
  },
  {
    "prev_ce_4",
    "prev_ce",
    "CE 4 positions earlier in list"
  },
  {
    "prev_ce_3",
    "prev_ce",
    "CE 3 positions earlier in list"
  },
  {
    "prev_ce_2",
    "prev_ce",
    "CE 2 positions earlier in list"
  },
  {
    "prev_ce_1",
    "prev_ce",
    "CE 1 position earlier in list"
  },
  {
    "self",
    "self",
    "The current custom element"
  },
  {
    "next_ce_1",
    "next_ce",
    "CE 1 position later in list"
  },
  {
    "next_ce_2",
    "next_ce",
    "CE 2 positions later in list"
  },
  {
    "next_ce_3",
    "next_ce",
    "CE 3 positions later in list"
  },
  {
    "next_ce_4",
    "next_ce",
    "CE 4 positions later in list"
  },
  {
    "next_ce_5",
    "next_ce",
    "CE 5 positions later in list"
  },
  {
    "next_ce_6",
    "next_ce",
    "CE 6 positions later in list"
  },
  {
    "next_ce_7",
    "next_ce",
    "CE 7 positions later in list"
  },
  {
    "next_ce_8",
    "next_ce",
    "CE 8 positions later in list"
  },
  {
    "any_element",
    "any_element",
    "This element matches any element"
  },
  {
    "steel_char_space",
    "steel_char",
    "Steel letter ' '"
  },
  {
    "steel_char_exclam",
    "steel_char",
    "Steel letter '!'"
  },
  {
    "steel_char_quotedbl",
    "steel_char",
    "Steel letter '\"'"
  },
  {
    "steel_char_numbersign",
    "steel_char",
    "Steel letter '#'"
  },
  {
    "steel_char_dollar",
    "steel_char",
    "Steel letter '$'"
  },
  {
    "steel_char_percent",
    "steel_char",
    "Steel letter '%'"
  },
  {
    "steel_char_ampersand",
    "steel_char",
    "Steel letter '&'"
  },
  {
    "steel_char_apostrophe",
    "steel_char",
    "Steel letter '''"
  },
  {
    "steel_char_parenleft",
    "steel_char",
    "Steel letter '('"
  },
  {
    "steel_char_parenright",
    "steel_char",
    "Steel letter ')'"
  },
  {
    "steel_char_asterisk",
    "steel_char",
    "Steel letter '*'"
  },
  {
    "steel_char_plus",
    "steel_char",
    "Steel letter '+'"
  },
  {
    "steel_char_comma",
    "steel_char",
    "Steel letter ','"
  },
  {
    "steel_char_minus",
    "steel_char",
    "Steel letter '-'"
  },
  {
    "steel_char_period",
    "steel_char",
    "Steel letter '.'"
  },
  {
    "steel_char_slash",
    "steel_char",
    "Steel letter '/'"
  },
  {
    "steel_char_0",
    "steel_char",
    "Steel letter '0'"
  },
  {
    "steel_char_1",
    "steel_char",
    "Steel letter '1'"
  },
  {
    "steel_char_2",
    "steel_char",
    "Steel letter '2'"
  },
  {
    "steel_char_3",
    "steel_char",
    "Steel letter '3'"
  },
  {
    "steel_char_4",
    "steel_char",
    "Steel letter '4'"
  },
  {
    "steel_char_5",
    "steel_char",
    "Steel letter '5'"
  },
  {
    "steel_char_6",
    "steel_char",
    "Steel letter '6'"
  },
  {
    "steel_char_7",
    "steel_char",
    "Steel letter '7'"
  },
  {
    "steel_char_8",
    "steel_char",
    "Steel letter '8'"
  },
  {
    "steel_char_9",
    "steel_char",
    "Steel letter '9'"
  },
  {
    "steel_char_colon",
    "steel_char",
    "Steel letter ':'"
  },
  {
    "steel_char_semicolon",
    "steel_char",
    "Steel letter ';'"
  },
  {
    "steel_char_less",
    "steel_char",
    "Steel letter '<'"
  },
  {
    "steel_char_equal",
    "steel_char",
    "Steel letter '='"
  },
  {
    "steel_char_greater",
    "steel_char",
    "Steel letter '>'"
  },
  {
    "steel_char_question",
    "steel_char",
    "Steel letter '?'"
  },
  {
    "steel_char_at",
    "steel_char",
    "Steel letter '@'"
  },
  {
    "steel_char_a",
    "steel_char",
    "Steel letter 'A'"
  },
  {
    "steel_char_b",
    "steel_char",
    "Steel letter 'B'"
  },
  {
    "steel_char_c",
    "steel_char",
    "Steel letter 'C'"
  },
  {
    "steel_char_d",
    "steel_char",
    "Steel letter 'D'"
  },
  {
    "steel_char_e",
    "steel_char",
    "Steel letter 'E'"
  },
  {
    "steel_char_f",
    "steel_char",
    "Steel letter 'F'"
  },
  {
    "steel_char_g",
    "steel_char",
    "Steel letter 'G'"
  },
  {
    "steel_char_h",
    "steel_char",
    "Steel letter 'H'"
  },
  {
    "steel_char_i",
    "steel_char",
    "Steel letter 'I'"
  },
  {
    "steel_char_j",
    "steel_char",
    "Steel letter 'J'"
  },
  {
    "steel_char_k",
    "steel_char",
    "Steel letter 'K'"
  },
  {
    "steel_char_l",
    "steel_char",
    "Steel letter 'L'"
  },
  {
    "steel_char_m",
    "steel_char",
    "Steel letter 'M'"
  },
  {
    "steel_char_n",
    "steel_char",
    "Steel letter 'N'"
  },
  {
    "steel_char_o",
    "steel_char",
    "Steel letter 'O'"
  },
  {
    "steel_char_p",
    "steel_char",
    "Steel letter 'P'"
  },
  {
    "steel_char_q",
    "steel_char",
    "Steel letter 'Q'"
  },
  {
    "steel_char_r",
    "steel_char",
    "Steel letter 'R'"
  },
  {
    "steel_char_s",
    "steel_char",
    "Steel letter 'S'"
  },
  {
    "steel_char_t",
    "steel_char",
    "Steel letter 'T'"
  },
  {
    "steel_char_u",
    "steel_char",
    "Steel letter 'U'"
  },
  {
    "steel_char_v",
    "steel_char",
    "Steel letter 'V'"
  },
  {
    "steel_char_w",
    "steel_char",
    "Steel letter 'W'"
  },
  {
    "steel_char_x",
    "steel_char",
    "Steel letter 'X'"
  },
  {
    "steel_char_y",
    "steel_char",
    "Steel letter 'Y'"
  },
  {
    "steel_char_z",
    "steel_char",
    "Steel letter 'Z'"
  },
  {
    "steel_char_bracketleft",
    "steel_char",
    "Steel letter '['"
  },
  {
    "steel_char_backslash",
    "steel_char",
    "Steel letter '\\'"
  },
  {
    "steel_char_bracketright",
    "steel_char",
    "Steel letter ']'"
  },
  {
    "steel_char_asciicircum",
    "steel_char",
    "Steel letter '^'"
  },
  {
    "steel_char_underscore",
    "steel_char",
    "Steel letter '_'"
  },
  {
    "steel_char_copyright",
    "steel_char",
    "Steel letter '\xa9'"
  },
  {
    "steel_char_aumlaut",
    "steel_char",
    "Steel letter '\xc4'"
  },
  {
    "steel_char_oumlaut",
    "steel_char",
    "Steel letter '\xd6'"
  },
  {
    "steel_char_uumlaut",
    "steel_char",
    "Steel letter '\xdc'"
  },
  {
    "steel_char_degree",
    "steel_char",
    "Steel letter '\xb0'"
  },
  {
    "steel_char_trademark",
    "steel_char",
    "Steel letter '\xae'"
  },
  {
    "steel_char_cursor",
    "steel_char",
    "Steel letter '\xa0'"
  },
  {
    "steel_char_unused",
    "steel_char",
    "Steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "Steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "Steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "Steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "Steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "Steel letter ''"
  },
  {
    "steel_char_button",
    "steel_char",
    "Steel letter 'button'"
  },
  {
    "steel_char_up",
    "steel_char",
    "Steel letter 'up'"
  },
  {
    "steel_char_down",
    "steel_char",
    "Steel letter 'down'"
  },
  {
    "sperms",
    "frankie",
    "Sperms"
  },
  {
    "bullet",
    "frankie",
    "Bullet"
  },
  {
    "heart",
    "frankie",
    "Heart"
  },
  {
    "cross",
    "frankie",
    "Cross"
  },
  {
    "frankie",
    "frankie",
    "Frankie"
  },
  {
    "sign_sperms",
    "sign",
    "Sign (sperms)"
  },
  {
    "sign_bullet",
    "sign",
    "Sign (bullet)"
  },
  {
    "sign_heart",
    "sign",
    "Sign (heart)"
  },
  {
    "sign_cross",
    "sign",
    "Sign (cross)"
  },
  {
    "sign_frankie",
    "sign",
    "Sign (frankie)"
  },
  {
    "steel_exit_closed",
    "steel_exit",
    "Closed steel exit"
  },
  {
    "steel_exit_open",
    "steel_exit",
    "Open steel exit"
  },
  {
    "dc_steelwall_1_left",
    "steelwall",
    "Steel wall 1 (left)"
  },
  {
    "dc_steelwall_1_right",
    "steelwall",
    "Steel wall 1 (right)"
  },
  {
    "dc_steelwall_1_top",
    "steelwall",
    "Steel wall 1 (top)"
  },
  {
    "dc_steelwall_1_bottom",
    "steelwall",
    "Steel wall 1 (bottom)"
  },
  {
    "dc_steelwall_1_horizontal",
    "steelwall",
    "Steel wall 1 (top/bottom)"
  },
  {
    "dc_steelwall_1_vertical",
    "steelwall",
    "Steel wall 1 (left/right)"
  },
  {
    "dc_steelwall_1_topleft",
    "steelwall",
    "Steel wall 1 (top/left)"
  },
  {
    "dc_steelwall_1_topright",
    "steelwall",
    "Steel wall 1 (top/right)"
  },
  {
    "dc_steelwall_1_bottomleft",
    "steelwall",
    "Steel wall 1 (bottom/left)"
  },
  {
    "dc_steelwall_1_bottomright",
    "steelwall",
    "Steel wall 1 (bottom/right)"
  },
  {
    "dc_steelwall_1_topleft_2",
    "steelwall",
    "Steel wall 1 (top/left corner)"
  },
  {
    "dc_steelwall_1_topright_2",
    "steelwall",
    "Steel wall 1 (top/right corner)"
  },
  {
    "dc_steelwall_1_bottomleft_2",
    "steelwall",
    "Steel wall 1 (bottom/left corner)"
  },
  {
    "dc_steelwall_1_bottomright_2",
    "steelwall",
    "Steel wall 1 (bottom/right corner)"
  },
  {
    "dc_steelwall_2_left",
    "steelwall",
    "Steel wall 2 (left)"
  },
  {
    "dc_steelwall_2_right",
    "steelwall",
    "Steel wall 2 (right)"
  },
  {
    "dc_steelwall_2_top",
    "steelwall",
    "Steel wall 2 (top)"
  },
  {
    "dc_steelwall_2_bottom",
    "steelwall",
    "Steel wall 2 (bottom)"
  },
  {
    "dc_steelwall_2_horizontal",
    "steelwall",
    "Steel wall 2 (horizontal)"
  },
  {
    "dc_steelwall_2_vertical",
    "steelwall",
    "Steel wall 2 (vertical)"
  },
  {
    "dc_steelwall_2_middle",
    "steelwall",
    "Steel wall 2 (middle)"
  },
  {
    "dc_steelwall_2_single",
    "steelwall",
    "Steel wall 2 (single)"
  },
  {
    "dc_switchgate_switch_up",
    "switchgate_switch",
    "Switch for switch gate (steel)"
  },
  {
    "dc_switchgate_switch_down",
    "switchgate_switch",
    "Switch for switch gate (steel)"
  },
  {
    "dc_timegate_switch",
    "timegate_switch",
    "Switch for time gate (steel)"
  },
  {
    "dc_timegate_switch.active",
    "timegate_switch",
    "Switch for time gate (steel)"
  },
  {
    "dc_landmine",
    "dc_landmine",
    "Land mine (DC style, removable)"
  },
  {
    "expandable_steelwall",
    "steelwall",
    "Growing steel wall"
  },
  {
    "expandable_steelwall_horizontal",
    "steelwall",
    "Growing steel wall (horizontal)"
  },
  {
    "expandable_steelwall_vertical",
    "steelwall",
    "Growing steel wall (vertical)"
  },
  {
    "expandable_steelwall_any",
    "steelwall",
    "Growing steel wall (any direction)"
  },
  {
    "em_exit_closed",
    "em_exit",
    "Closed exit (EM style)"
  },
  {
    "em_exit_open",
    "em_exit",
    "Open exit (EM style)"
  },
  {
    "em_steel_exit_closed",
    "em_steel_exit",
    "Closed steel exit (EM style)"
  },
  {
    "em_steel_exit_open",
    "em_steel_exit",
    "Open steel exit (EM style)"
  },
  {
    "dc_gate_fake_gray",
    "gate",
    "Gray door (opened by no key)"
  },
  {
    "dc_magic_wall",
    "dc_magic_wall",
    "Magic wall (DC style)"
  },
  {
    "quicksand_fast_empty",
    "quicksand",
    "Fast quicksand (empty)"
  },
  {
    "quicksand_fast_full",
    "quicksand",
    "Fast quicksand (with rock)"
  },
  {
    "from_level_template",
    "from_level_template",
    "Element taken from level template"
  },
  {
    "mm_empty_space",
    "empty_space",
    "Empty space"
  },
  {
    "mm_mirror_1",
    "mm_mirror",
    "Mirror (0\xb0)"
  },
  {
    "mm_mirror_2",
    "mm_mirror",
    "Mirror (11.25\xb0)"
  },
  {
    "mm_mirror_3",
    "mm_mirror",
    "Mirror (22.5\xb0)"
  },
  {
    "mm_mirror_4",
    "mm_mirror",
    "Mirror (33.75\xb0)"
  },
  {
    "mm_mirror_5",
    "mm_mirror",
    "Mirror (45\xb0)"
  },
  {
    "mm_mirror_6",
    "mm_mirror",
    "Mirror (56.25\xb0)"
  },
  {
    "mm_mirror_7",
    "mm_mirror",
    "Mirror (67.5\xb0)"
  },
  {
    "mm_mirror_8",
    "mm_mirror",
    "Mirror (78.75\xb0)"
  },
  {
    "mm_mirror_9",
    "mm_mirror",
    "Mirror (90\xb0)"
  },
  {
    "mm_mirror_10",
    "mm_mirror",
    "Mirror (101.25\xb0)"
  },
  {
    "mm_mirror_11",
    "mm_mirror",
    "Mirror (112.5\xb0)"
  },
  {
    "mm_mirror_12",
    "mm_mirror",
    "Mirror (123.75\xb0)"
  },
  {
    "mm_mirror_13",
    "mm_mirror",
    "Mirror (135\xb0)"
  },
  {
    "mm_mirror_14",
    "mm_mirror",
    "Mirror (146.25\xb0)"
  },
  {
    "mm_mirror_15",
    "mm_mirror",
    "Mirror (157.5\xb0)"
  },
  {
    "mm_mirror_16",
    "mm_mirror",
    "Mirror (168.75\xb0)"
  },
  {
    "mm_steel_grid_fixed_1",
    "mm_steel_grid_fixed",
    "Fixed steel polarizer (0\xb0)"
  },
  {
    "mm_steel_grid_fixed_2",
    "mm_steel_grid_fixed",
    "Fixed steel polarizer (90\xb0)"
  },
  {
    "mm_steel_grid_fixed_3",
    "mm_steel_grid_fixed",
    "Fixed steel polarizer (45\xb0)"
  },
  {
    "mm_steel_grid_fixed_4",
    "mm_steel_grid_fixed",
    "Fixed steel polarizer (135\xb0)"
  },
  {
    "mm_mcduffin.right",
    "mm_mcduffin",
    "Gregor McDuffin (looking right)"
  },
  {
    "mm_mcduffin.up",
    "mm_mcduffin",
    "Gregor McDuffin (looking up)"
  },
  {
    "mm_mcduffin.left",
    "mm_mcduffin",
    "Gregor McDuffin (looking left)"
  },
  {
    "mm_mcduffin.down",
    "mm_mcduffin",
    "Gregor McDuffin (looking down)"
  },
  {
    "mm_exit_closed",
    "mm_exit",
    "Closed exit (MM style)"
  },
  {
    "mm_exit_opening_1",
    "mm_exit",
    "Opening exit 1"
  },
  {
    "mm_exit_opening_2",
    "mm_exit",
    "Opening exit 2"
  },
  {
    "mm_exit_open",
    "mm_exit",
    "Open exit (MM style)"
  },
  {
    "mm_kettle",
    "mm_kettle",
    "Magic cauldron"
  },
  {
    "mm_bomb",
    "mm_bomb",
    "Bomb (MM style)"
  },
  {
    "mm_prism",
    "mm_prism",
    "Prism"
  },
  {
    "mm_steel_wall_1",
    "mm_steel_wall",
    "Steel wall 1 (MM style)"
  },
  {
    "mm_steel_wall_2",
    "mm_steel_wall",
    "Steel wall 2 (MM style)"
  },
  {
    "mm_steel_wall_3",
    "mm_steel_wall",
    "Steel wall 3 (MM style)"
  },
  {
    "mm_steel_wall_4",
    "mm_steel_wall",
    "Steel wall 4 (MM style)"
  },
  {
    "mm_steel_wall_5",
    "mm_steel_wall",
    "Steel wall 5 (MM style)"
  },
  {
    "mm_steel_wall_6",
    "mm_steel_wall",
    "Steel wall 6 (MM style)"
  },
  {
    "mm_steel_wall_7",
    "mm_steel_wall",
    "Steel wall 7 (MM style)"
  },
  {
    "mm_steel_wall_8",
    "mm_steel_wall",
    "Steel wall 8 (MM style)"
  },
  {
    "mm_steel_wall_9",
    "mm_steel_wall",
    "Steel wall 9 (MM style)"
  },
  {
    "mm_steel_wall_10",
    "mm_steel_wall",
    "Steel wall 10 (MM style)"
  },
  {
    "mm_steel_wall_11",
    "mm_steel_wall",
    "Steel wall 11 (MM style)"
  },
  {
    "mm_steel_wall_12",
    "mm_steel_wall",
    "Steel wall 12 (MM style)"
  },
  {
    "mm_steel_wall_13",
    "mm_steel_wall",
    "Steel wall 13 (MM style)"
  },
  {
    "mm_steel_wall_14",
    "mm_steel_wall",
    "Steel wall 14 (MM style)"
  },
  {
    "mm_steel_wall_15",
    "mm_steel_wall",
    "Steel wall 15 (MM style)"
  },
  {
    "mm_steel_wall_16",
    "mm_steel_wall",
    "Steel wall 16 (MM style)"
  },
  {
    "mm_wooden_wall_1",
    "mm_wooden_wall",
    "Wooden wall 1 (MM style)"
  },
  {
    "mm_wooden_wall_2",
    "mm_wooden_wall",
    "Wooden wall 2 (MM style)"
  },
  {
    "mm_wooden_wall_3",
    "mm_wooden_wall",
    "Wooden wall 3 (MM style)"
  },
  {
    "mm_wooden_wall_4",
    "mm_wooden_wall",
    "Wooden wall 4 (MM style)"
  },
  {
    "mm_wooden_wall_5",
    "mm_wooden_wall",
    "Wooden wall 5 (MM style)"
  },
  {
    "mm_wooden_wall_6",
    "mm_wooden_wall",
    "Wooden wall 6 (MM style)"
  },
  {
    "mm_wooden_wall_7",
    "mm_wooden_wall",
    "Wooden wall 7 (MM style)"
  },
  {
    "mm_wooden_wall_8",
    "mm_wooden_wall",
    "Wooden wall 8 (MM style)"
  },
  {
    "mm_wooden_wall_9",
    "mm_wooden_wall",
    "Wooden wall 9 (MM style)"
  },
  {
    "mm_wooden_wall_10",
    "mm_wooden_wall",
    "Wooden wall 10 (MM style)"
  },
  {
    "mm_wooden_wall_11",
    "mm_wooden_wall",
    "Wooden wall 11 (MM style)"
  },
  {
    "mm_wooden_wall_12",
    "mm_wooden_wall",
    "Wooden wall 12 (MM style)"
  },
  {
    "mm_wooden_wall_13",
    "mm_wooden_wall",
    "Wooden wall 13 (MM style)"
  },
  {
    "mm_wooden_wall_14",
    "mm_wooden_wall",
    "Wooden wall 14 (MM style)"
  },
  {
    "mm_wooden_wall_15",
    "mm_wooden_wall",
    "Wooden wall 15 (MM style)"
  },
  {
    "mm_wooden_wall_16",
    "mm_wooden_wall",
    "Wooden wall 16 (MM style)"
  },
  {
    "mm_ice_wall_1",
    "mm_ice_wall",
    "Ice wall 1"
  },
  {
    "mm_ice_wall_2",
    "mm_ice_wall",
    "Ice wall 2"
  },
  {
    "mm_ice_wall_3",
    "mm_ice_wall",
    "Ice wall 3"
  },
  {
    "mm_ice_wall_4",
    "mm_ice_wall",
    "Ice wall 4"
  },
  {
    "mm_ice_wall_5",
    "mm_ice_wall",
    "Ice wall 5"
  },
  {
    "mm_ice_wall_6",
    "mm_ice_wall",
    "Ice wall 6"
  },
  {
    "mm_ice_wall_7",
    "mm_ice_wall",
    "Ice wall 7"
  },
  {
    "mm_ice_wall_8",
    "mm_ice_wall",
    "Ice wall 8"
  },
  {
    "mm_ice_wall_9",
    "mm_ice_wall",
    "Ice wall 9"
  },
  {
    "mm_ice_wall_10",
    "mm_ice_wall",
    "Ice wall 10"
  },
  {
    "mm_ice_wall_11",
    "mm_ice_wall",
    "Ice wall 11"
  },
  {
    "mm_ice_wall_12",
    "mm_ice_wall",
    "Ice wall 12"
  },
  {
    "mm_ice_wall_13",
    "mm_ice_wall",
    "Ice wall 13"
  },
  {
    "mm_ice_wall_14",
    "mm_ice_wall",
    "Ice wall 14"
  },
  {
    "mm_ice_wall_15",
    "mm_ice_wall",
    "Ice wall 15"
  },
  {
    "mm_ice_wall_16",
    "mm_ice_wall",
    "Ice wall 16"
  },
  {
    "mm_amoeba_wall_1",
    "mm_amoeba_wall",
    "Amoeba wall 1"
  },
  {
    "mm_amoeba_wall_2",
    "mm_amoeba_wall",
    "Amoeba wall 2"
  },
  {
    "mm_amoeba_wall_3",
    "mm_amoeba_wall",
    "Amoeba wall 3"
  },
  {
    "mm_amoeba_wall_4",
    "mm_amoeba_wall",
    "Amoeba wall 4"
  },
  {
    "mm_amoeba_wall_5",
    "mm_amoeba_wall",
    "Amoeba wall 5"
  },
  {
    "mm_amoeba_wall_6",
    "mm_amoeba_wall",
    "Amoeba wall 6"
  },
  {
    "mm_amoeba_wall_7",
    "mm_amoeba_wall",
    "Amoeba wall 7"
  },
  {
    "mm_amoeba_wall_8",
    "mm_amoeba_wall",
    "Amoeba wall 8"
  },
  {
    "mm_amoeba_wall_9",
    "mm_amoeba_wall",
    "Amoeba wall 9"
  },
  {
    "mm_amoeba_wall_10",
    "mm_amoeba_wall",
    "Amoeba wall 10"
  },
  {
    "mm_amoeba_wall_11",
    "mm_amoeba_wall",
    "Amoeba wall 11"
  },
  {
    "mm_amoeba_wall_12",
    "mm_amoeba_wall",
    "Amoeba wall 12"
  },
  {
    "mm_amoeba_wall_13",
    "mm_amoeba_wall",
    "Amoeba wall 13"
  },
  {
    "mm_amoeba_wall_14",
    "mm_amoeba_wall",
    "Amoeba wall 14"
  },
  {
    "mm_amoeba_wall_15",
    "mm_amoeba_wall",
    "Amoeba wall 15"
  },
  {
    "mm_amoeba_wall_16",
    "mm_amoeba_wall",
    "Amoeba wall 16"
  },
  {
    "mm_wooden_block",
    "mm_wooden_block",
    "Wooden block"
  },
  {
    "mm_gray_ball",
    "mm_gray_ball",
    "Gray ball"
  },
  {
    "mm_teleporter_1",
    "mm_teleporter",
    "Teleporter (0\xb0)"
  },
  {
    "mm_teleporter_2",
    "mm_teleporter",
    "Teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_3",
    "mm_teleporter",
    "Teleporter (45\xb0)"
  },
  {
    "mm_teleporter_4",
    "mm_teleporter",
    "Teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_5",
    "mm_teleporter",
    "Teleporter (90\xb0)"
  },
  {
    "mm_teleporter_6",
    "mm_teleporter",
    "Teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_7",
    "mm_teleporter",
    "Teleporter (135\xb0)"
  },
  {
    "mm_teleporter_8",
    "mm_teleporter",
    "Teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_9",
    "mm_teleporter",
    "Teleporter (180\xb0)"
  },
  {
    "mm_teleporter_10",
    "mm_teleporter",
    "Teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_11",
    "mm_teleporter",
    "Teleporter (225\xb0)"
  },
  {
    "mm_teleporter_12",
    "mm_teleporter",
    "Teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_13",
    "mm_teleporter",
    "Teleporter (270\xb0)"
  },
  {
    "mm_teleporter_14",
    "mm_teleporter",
    "Teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_15",
    "mm_teleporter",
    "Teleporter (315\xb0)"
  },
  {
    "mm_teleporter_16",
    "mm_teleporter",
    "Teleporter (337.5\xb0)"
  },
  {
    "mm_fuse.active",
    "mm_fuse",
    "Fuse (on)"
  },
  {
    "mm_pacman.right",
    "mm_pacman",
    "Pac man (starts moving right)"
  },
  {
    "mm_pacman.up",
    "mm_pacman",
    "Pac man (starts moving up)"
  },
  {
    "mm_pacman.left",
    "mm_pacman",
    "Pac man (starts moving left)"
  },
  {
    "mm_pacman.down",
    "mm_pacman",
    "Pac man (starts moving down)"
  },
  {
    "mm_polarizer_1",
    "mm_polarizer",
    "Polarizer (0\xb0)"
  },
  {
    "mm_polarizer_2",
    "mm_polarizer",
    "Polarizer (11.25\xb0)"
  },
  {
    "mm_polarizer_3",
    "mm_polarizer",
    "Polarizer (22.5\xb0)"
  },
  {
    "mm_polarizer_4",
    "mm_polarizer",
    "Polarizer (33.75\xb0)"
  },
  {
    "mm_polarizer_5",
    "mm_polarizer",
    "Polarizer (45\xb0)"
  },
  {
    "mm_polarizer_6",
    "mm_polarizer",
    "Polarizer (56.25\xb0)"
  },
  {
    "mm_polarizer_7",
    "mm_polarizer",
    "Polarizer (67.5\xb0)"
  },
  {
    "mm_polarizer_8",
    "mm_polarizer",
    "Polarizer (78.75\xb0)"
  },
  {
    "mm_polarizer_9",
    "mm_polarizer",
    "Polarizer (90\xb0)"
  },
  {
    "mm_polarizer_10",
    "mm_polarizer",
    "Polarizer (101.25\xb0)"
  },
  {
    "mm_polarizer_11",
    "mm_polarizer",
    "Polarizer (112.5\xb0)"
  },
  {
    "mm_polarizer_12",
    "mm_polarizer",
    "Polarizer (123.75\xb0)"
  },
  {
    "mm_polarizer_13",
    "mm_polarizer",
    "Polarizer (135\xb0)"
  },
  {
    "mm_polarizer_14",
    "mm_polarizer",
    "Polarizer (146.25\xb0)"
  },
  {
    "mm_polarizer_15",
    "mm_polarizer",
    "Polarizer (157.5\xb0)"
  },
  {
    "mm_polarizer_16",
    "mm_polarizer",
    "Polarizer (168.75\xb0)"
  },
  {
    "mm_polarizer_cross_1",
    "mm_polarizer_cross",
    "Two-way polarizer (0\xb0)"
  },
  {
    "mm_polarizer_cross_2",
    "mm_polarizer_cross",
    "Two-way polarizer (22.5\xb0)"
  },
  {
    "mm_polarizer_cross_3",
    "mm_polarizer_cross",
    "Two-way polarizer (45\xb0)"
  },
  {
    "mm_polarizer_cross_4",
    "mm_polarizer_cross",
    "Two-way polarizer (67.5\xb0)"
  },
  {
    "mm_mirror_fixed_1",
    "mm_mirror_fixed",
    "Fixed mirror (0\xb0)"
  },
  {
    "mm_mirror_fixed_2",
    "mm_mirror_fixed",
    "Fixed mirror (45\xb0)"
  },
  {
    "mm_mirror_fixed_3",
    "mm_mirror_fixed",
    "Fixed mirror (90\xb0)"
  },
  {
    "mm_mirror_fixed_4",
    "mm_mirror_fixed",
    "Fixed mirror (135\xb0)"
  },
  {
    "mm_steel_lock",
    "mm_steel_lock",
    "Steel lock"
  },
  {
    "mm_key",
    "mm_key",
    "Key (MM style)"
  },
  {
    "mm_lightbulb",
    "mm_lightbulb",
    "Light bulb (off)"
  },
  {
    "mm_lightbulb.active",
    "mm_lightbulb",
    "Light bulb (on)"
  },
  {
    "mm_lightball",
    "mm_lightball",
    "Bonus ball"
  },
  {
    "mm_steel_block",
    "mm_steel_block",
    "Steel block"
  },
  {
    "mm_wooden_lock",
    "mm_wooden_lock",
    "Wooden lock"
  },
  {
    "mm_fuel_full",
    "mm_fuel",
    "Extra energy ball (full)"
  },
  {
    "mm_wooden_grid_fixed_1",
    "mm_wooden_grid_fixed",
    "Fixed wooden polarizer (0\xb0)"
  },
  {
    "mm_wooden_grid_fixed_2",
    "mm_wooden_grid_fixed",
    "Fixed wooden polarizer (90\xb0)"
  },
  {
    "mm_wooden_grid_fixed_3",
    "mm_wooden_grid_fixed",
    "Fixed wooden polarizer (45\xb0)"
  },
  {
    "mm_wooden_grid_fixed_4",
    "mm_wooden_grid_fixed",
    "Fixed wooden polarizer (135\xb0)"
  },
  {
    "mm_fuel_empty",
    "mm_fuel",
    "Extra energy ball (empty)"
  },
  {
    "mm_envelope_1",
    "mm_envelope",
    "Mail envelope 1 (MM style)"
  },
  {
    "mm_envelope_2",
    "mm_envelope",
    "Mail envelope 2 (MM style)"
  },
  {
    "mm_envelope_3",
    "mm_envelope",
    "Mail envelope 3 (MM style)"
  },
  {
    "mm_envelope_4",
    "mm_envelope",
    "Mail envelope 4 (MM style)"
  },
  {
    "df_mirror_1",
    "df_mirror",
    "Mirror (DF style) (0\xb0)"
  },
  {
    "df_mirror_2",
    "df_mirror",
    "Mirror (DF style) (11.25\xb0)"
  },
  {
    "df_mirror_3",
    "df_mirror",
    "Mirror (DF style) (22.5\xb0)"
  },
  {
    "df_mirror_4",
    "df_mirror",
    "Mirror (DF style) (33.75\xb0)"
  },
  {
    "df_mirror_5",
    "df_mirror",
    "Mirror (DF style) (45\xb0)"
  },
  {
    "df_mirror_6",
    "df_mirror",
    "Mirror (DF style) (56.25\xb0)"
  },
  {
    "df_mirror_7",
    "df_mirror",
    "Mirror (DF style) (67.5\xb0)"
  },
  {
    "df_mirror_8",
    "df_mirror",
    "Mirror (DF style) (78.75\xb0)"
  },
  {
    "df_mirror_9",
    "df_mirror",
    "Mirror (DF style) (90\xb0)"
  },
  {
    "df_mirror_10",
    "df_mirror",
    "Mirror (DF style) (101.25\xb0)"
  },
  {
    "df_mirror_11",
    "df_mirror",
    "Mirror (DF style) (112.5\xb0)"
  },
  {
    "df_mirror_12",
    "df_mirror",
    "Mirror (DF style) (123.75\xb0)"
  },
  {
    "df_mirror_13",
    "df_mirror",
    "Mirror (DF style) (135\xb0)"
  },
  {
    "df_mirror_14",
    "df_mirror",
    "Mirror (DF style) (146.25\xb0)"
  },
  {
    "df_mirror_15",
    "df_mirror",
    "Mirror (DF style) (157.5\xb0)"
  },
  {
    "df_mirror_16",
    "df_mirror",
    "Mirror (DF style) (168.75\xb0)"
  },
  {
    "df_wooden_grid_fixed_1",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (0\xb0)"
  },
  {
    "df_wooden_grid_fixed_2",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (22.5\xb0)"
  },
  {
    "df_wooden_grid_fixed_3",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (45\xb0)"
  },
  {
    "df_wooden_grid_fixed_4",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (67.5\xb0)"
  },
  {
    "df_wooden_grid_fixed_5",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (90\xb0)"
  },
  {
    "df_wooden_grid_fixed_6",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (112.5\xb0)"
  },
  {
    "df_wooden_grid_fixed_7",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (135\xb0)"
  },
  {
    "df_wooden_grid_fixed_8",
    "df_wooden_grid_fixed",
    "Fixed wooden polarizer (DF) (157.5\xb0)"
  },
  {
    "df_steel_grid_fixed_1",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (0\xb0)"
  },
  {
    "df_steel_grid_fixed_2",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (22.5\xb0)"
  },
  {
    "df_steel_grid_fixed_3",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (45\xb0)"
  },
  {
    "df_steel_grid_fixed_4",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (67.5\xb0)"
  },
  {
    "df_steel_grid_fixed_5",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (90\xb0)"
  },
  {
    "df_steel_grid_fixed_6",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (112.5\xb0)"
  },
  {
    "df_steel_grid_fixed_7",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (135\xb0)"
  },
  {
    "df_steel_grid_fixed_8",
    "df_steel_grid_fixed",
    "Fixed steel polarizer (DF) (157.5\xb0)"
  },
  {
    "df_wooden_wall_1",
    "df_wooden_wall",
    "Wooden wall 1 (DF style)"
  },
  {
    "df_wooden_wall_2",
    "df_wooden_wall",
    "Wooden wall 2 (DF style)"
  },
  {
    "df_wooden_wall_3",
    "df_wooden_wall",
    "Wooden wall 3 (DF style)"
  },
  {
    "df_wooden_wall_4",
    "df_wooden_wall",
    "Wooden wall 4 (DF style)"
  },
  {
    "df_wooden_wall_5",
    "df_wooden_wall",
    "Wooden wall 5 (DF style)"
  },
  {
    "df_wooden_wall_6",
    "df_wooden_wall",
    "Wooden wall 6 (DF style)"
  },
  {
    "df_wooden_wall_7",
    "df_wooden_wall",
    "Wooden wall 7 (DF style)"
  },
  {
    "df_wooden_wall_8",
    "df_wooden_wall",
    "Wooden wall 8 (DF style)"
  },
  {
    "df_wooden_wall_9",
    "df_wooden_wall",
    "Wooden wall 9 (DF style)"
  },
  {
    "df_wooden_wall_10",
    "df_wooden_wall",
    "Wooden wall 10 (DF style)"
  },
  {
    "df_wooden_wall_11",
    "df_wooden_wall",
    "Wooden wall 11 (DF style)"
  },
  {
    "df_wooden_wall_12",
    "df_wooden_wall",
    "Wooden wall 12 (DF style)"
  },
  {
    "df_wooden_wall_13",
    "df_wooden_wall",
    "Wooden wall 13 (DF style)"
  },
  {
    "df_wooden_wall_14",
    "df_wooden_wall",
    "Wooden wall 14 (DF style)"
  },
  {
    "df_wooden_wall_15",
    "df_wooden_wall",
    "Wooden wall 15 (DF style)"
  },
  {
    "df_wooden_wall_16",
    "df_wooden_wall",
    "Wooden wall 16 (DF style)"
  },
  {
    "df_steel_wall_1",
    "df_steel_wall",
    "Steel wall 1 (DF style)"
  },
  {
    "df_steel_wall_2",
    "df_steel_wall",
    "Steel wall 2 (DF style)"
  },
  {
    "df_steel_wall_3",
    "df_steel_wall",
    "Steel wall 3 (DF style)"
  },
  {
    "df_steel_wall_4",
    "df_steel_wall",
    "Steel wall 4 (DF style)"
  },
  {
    "df_steel_wall_5",
    "df_steel_wall",
    "Steel wall 5 (DF style)"
  },
  {
    "df_steel_wall_6",
    "df_steel_wall",
    "Steel wall 6 (DF style)"
  },
  {
    "df_steel_wall_7",
    "df_steel_wall",
    "Steel wall 7 (DF style)"
  },
  {
    "df_steel_wall_8",
    "df_steel_wall",
    "Steel wall 8 (DF style)"
  },
  {
    "df_steel_wall_9",
    "df_steel_wall",
    "Steel wall 9 (DF style)"
  },
  {
    "df_steel_wall_10",
    "df_steel_wall",
    "Steel wall 10 (DF style)"
  },
  {
    "df_steel_wall_11",
    "df_steel_wall",
    "Steel wall 11 (DF style)"
  },
  {
    "df_steel_wall_12",
    "df_steel_wall",
    "Steel wall 12 (DF style)"
  },
  {
    "df_steel_wall_13",
    "df_steel_wall",
    "Steel wall 13 (DF style)"
  },
  {
    "df_steel_wall_14",
    "df_steel_wall",
    "Steel wall 14 (DF style)"
  },
  {
    "df_steel_wall_15",
    "df_steel_wall",
    "Steel wall 15 (DF style)"
  },
  {
    "df_steel_wall_16",
    "df_steel_wall",
    "Steel wall 16 (DF style)"
  },
  {
    "df_empty_space",
    "empty_space",
    "Empty space"
  },
  {
    "df_cell",
    "df_cell",
    "Cell"
  },
  {
    "df_mine",
    "df_mine",
    "Mine"
  },
  {
    "df_refractor",
    "df_refractor",
    "Refractor"
  },
  {
    "df_laser.right",
    "df_laser",
    "Laser cannon (shooting right)"
  },
  {
    "df_laser.up",
    "df_laser",
    "Laser cannon (shooting up)"
  },
  {
    "df_laser.left",
    "df_laser",
    "Laser cannon (shooting left)"
  },
  {
    "df_laser.down",
    "df_laser",
    "Laser cannon (shooting down)"
  },
  {
    "df_receiver.right",
    "df_receiver",
    "Laser receiver (directed right)"
  },
  {
    "df_receiver.up",
    "df_receiver",
    "Laser receiver (directed up)"
  },
  {
    "df_receiver.left",
    "df_receiver",
    "Laser receiver (directed left)"
  },
  {
    "df_receiver.down",
    "df_receiver",
    "Laser receiver (directed down)"
  },
  {
    "df_fibre_optic_red_1",
    "df_fibre_optic",
    "Red fibre optic (part 1)"
  },
  {
    "df_fibre_optic_red_2",
    "df_fibre_optic",
    "Red fibre optic (part 2)"
  },
  {
    "df_fibre_optic_yellow_1",
    "df_fibre_optic",
    "Yellow fibre optic (part 1)"
  },
  {
    "df_fibre_optic_yellow_2",
    "df_fibre_optic",
    "Yellow fibre optic (part 2)"
  },
  {
    "df_fibre_optic_green_1",
    "df_fibre_optic",
    "Green fibre optic (part 1)"
  },
  {
    "df_fibre_optic_green_2",
    "df_fibre_optic",
    "Green fibre optic (part 2)"
  },
  {
    "df_fibre_optic_blue_1",
    "df_fibre_optic",
    "Blue fibre optic (part 1)"
  },
  {
    "df_fibre_optic_blue_2",
    "df_fibre_optic",
    "Blue fibre optic (part 2)"
  },
  {
    "df_mirror_rotating_1",
    "df_mirror_rotating",
    "Rotating mirror (0\xb0)"
  },
  {
    "df_mirror_rotating_2",
    "df_mirror_rotating",
    "Rotating mirror (11.25\xb0)"
  },
  {
    "df_mirror_rotating_3",
    "df_mirror_rotating",
    "Rotating mirror (22.5\xb0)"
  },
  {
    "df_mirror_rotating_4",
    "df_mirror_rotating",
    "Rotating mirror (33.75\xb0)"
  },
  {
    "df_mirror_rotating_5",
    "df_mirror_rotating",
    "Rotating mirror (45\xb0)"
  },
  {
    "df_mirror_rotating_6",
    "df_mirror_rotating",
    "Rotating mirror (56.25\xb0)"
  },
  {
    "df_mirror_rotating_7",
    "df_mirror_rotating",
    "Rotating mirror (67.5\xb0)"
  },
  {
    "df_mirror_rotating_8",
    "df_mirror_rotating",
    "Rotating mirror (78.75\xb0)"
  },
  {
    "df_mirror_rotating_9",
    "df_mirror_rotating",
    "Rotating mirror (90\xb0)"
  },
  {
    "df_mirror_rotating_10",
    "df_mirror_rotating",
    "Rotating mirror (101.25\xb0)"
  },
  {
    "df_mirror_rotating_11",
    "df_mirror_rotating",
    "Rotating mirror (112.5\xb0)"
  },
  {
    "df_mirror_rotating_12",
    "df_mirror_rotating",
    "Rotating mirror (123.75\xb0)"
  },
  {
    "df_mirror_rotating_13",
    "df_mirror_rotating",
    "Rotating mirror (135\xb0)"
  },
  {
    "df_mirror_rotating_14",
    "df_mirror_rotating",
    "Rotating mirror (146.25\xb0)"
  },
  {
    "df_mirror_rotating_15",
    "df_mirror_rotating",
    "Rotating mirror (157.5\xb0)"
  },
  {
    "df_mirror_rotating_16",
    "df_mirror_rotating",
    "Rotating mirror (168.75\xb0)"
  },
  {
    "df_wooden_grid_rotating_1",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (0\xb0)"
  },
  {
    "df_wooden_grid_rotating_2",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (22.5\xb0)"
  },
  {
    "df_wooden_grid_rotating_3",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (45\xb0)"
  },
  {
    "df_wooden_grid_rotating_4",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (67.5\xb0)"
  },
  {
    "df_wooden_grid_rotating_5",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (90\xb0)"
  },
  {
    "df_wooden_grid_rotating_6",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (112.5\xb0)"
  },
  {
    "df_wooden_grid_rotating_7",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (135\xb0)"
  },
  {
    "df_wooden_grid_rotating_8",
    "df_wooden_grid_rotating",
    "Rotating wooden polarizer (157.5\xb0)"
  },
  {
    "df_steel_grid_rotating_1",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (0\xb0)"
  },
  {
    "df_steel_grid_rotating_2",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (22.5\xb0)"
  },
  {
    "df_steel_grid_rotating_3",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (45\xb0)"
  },
  {
    "df_steel_grid_rotating_4",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (67.5\xb0)"
  },
  {
    "df_steel_grid_rotating_5",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (90\xb0)"
  },
  {
    "df_steel_grid_rotating_6",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (112.5\xb0)"
  },
  {
    "df_steel_grid_rotating_7",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (135\xb0)"
  },
  {
    "df_steel_grid_rotating_8",
    "df_steel_grid_rotating",
    "Rotating steel polarizer (157.5\xb0)"
  },
  {
    "mm_teleporter_red_1",
    "mm_teleporter",
    "Red teleporter (0\xb0)"
  },
  {
    "mm_teleporter_red_2",
    "mm_teleporter",
    "Red teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_red_3",
    "mm_teleporter",
    "Red teleporter (45\xb0)"
  },
  {
    "mm_teleporter_red_4",
    "mm_teleporter",
    "Red teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_red_5",
    "mm_teleporter",
    "Red teleporter (90\xb0)"
  },
  {
    "mm_teleporter_red_6",
    "mm_teleporter",
    "Red teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_red_7",
    "mm_teleporter",
    "Red teleporter (135\xb0)"
  },
  {
    "mm_teleporter_red_8",
    "mm_teleporter",
    "Red teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_red_9",
    "mm_teleporter",
    "Red teleporter (180\xb0)"
  },
  {
    "mm_teleporter_red_10",
    "mm_teleporter",
    "Red teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_red_11",
    "mm_teleporter",
    "Red teleporter (225\xb0)"
  },
  {
    "mm_teleporter_red_12",
    "mm_teleporter",
    "Red teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_red_13",
    "mm_teleporter",
    "Red teleporter (270\xb0)"
  },
  {
    "mm_teleporter_red_14",
    "mm_teleporter",
    "Red teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_red_15",
    "mm_teleporter",
    "Red teleporter (315\xb0)"
  },
  {
    "mm_teleporter_red_16",
    "mm_teleporter",
    "Red teleporter (337.5\xb0)"
  },
  {
    "mm_teleporter_yellow_1",
    "mm_teleporter",
    "Yellow teleporter (0\xb0)"
  },
  {
    "mm_teleporter_yellow_2",
    "mm_teleporter",
    "Yellow teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_yellow_3",
    "mm_teleporter",
    "Yellow teleporter (45\xb0)"
  },
  {
    "mm_teleporter_yellow_4",
    "mm_teleporter",
    "Yellow teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_yellow_5",
    "mm_teleporter",
    "Yellow teleporter (90\xb0)"
  },
  {
    "mm_teleporter_yellow_6",
    "mm_teleporter",
    "Yellow teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_yellow_7",
    "mm_teleporter",
    "Yellow teleporter (135\xb0)"
  },
  {
    "mm_teleporter_yellow_8",
    "mm_teleporter",
    "Yellow teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_yellow_9",
    "mm_teleporter",
    "Yellow teleporter (180\xb0)"
  },
  {
    "mm_teleporter_yellow_10",
    "mm_teleporter",
    "Yellow teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_yellow_11",
    "mm_teleporter",
    "Yellow teleporter (225\xb0)"
  },
  {
    "mm_teleporter_yellow_12",
    "mm_teleporter",
    "Yellow teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_yellow_13",
    "mm_teleporter",
    "Yellow teleporter (270\xb0)"
  },
  {
    "mm_teleporter_yellow_14",
    "mm_teleporter",
    "Yellow teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_yellow_15",
    "mm_teleporter",
    "Yellow teleporter (315\xb0)"
  },
  {
    "mm_teleporter_yellow_16",
    "mm_teleporter",
    "Yellow teleporter (337.5\xb0)"
  },
  {
    "mm_teleporter_green_1",
    "mm_teleporter",
    "Green teleporter (0\xb0)"
  },
  {
    "mm_teleporter_green_2",
    "mm_teleporter",
    "Green teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_green_3",
    "mm_teleporter",
    "Green teleporter (45\xb0)"
  },
  {
    "mm_teleporter_green_4",
    "mm_teleporter",
    "Green teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_green_5",
    "mm_teleporter",
    "Green teleporter (90\xb0)"
  },
  {
    "mm_teleporter_green_6",
    "mm_teleporter",
    "Green teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_green_7",
    "mm_teleporter",
    "Green teleporter (135\xb0)"
  },
  {
    "mm_teleporter_green_8",
    "mm_teleporter",
    "Green teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_green_9",
    "mm_teleporter",
    "Green teleporter (180\xb0)"
  },
  {
    "mm_teleporter_green_10",
    "mm_teleporter",
    "Green teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_green_11",
    "mm_teleporter",
    "Green teleporter (225\xb0)"
  },
  {
    "mm_teleporter_green_12",
    "mm_teleporter",
    "Green teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_green_13",
    "mm_teleporter",
    "Green teleporter (270\xb0)"
  },
  {
    "mm_teleporter_green_14",
    "mm_teleporter",
    "Green teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_green_15",
    "mm_teleporter",
    "Green teleporter (315\xb0)"
  },
  {
    "mm_teleporter_green_16",
    "mm_teleporter",
    "Green teleporter (337.5\xb0)"
  },
  {
    "mm_teleporter_blue_1",
    "mm_teleporter",
    "Blue teleporter (0\xb0)"
  },
  {
    "mm_teleporter_blue_2",
    "mm_teleporter",
    "Blue teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_blue_3",
    "mm_teleporter",
    "Blue teleporter (45\xb0)"
  },
  {
    "mm_teleporter_blue_4",
    "mm_teleporter",
    "Blue teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_blue_5",
    "mm_teleporter",
    "Blue teleporter (90\xb0)"
  },
  {
    "mm_teleporter_blue_6",
    "mm_teleporter",
    "Blue teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_blue_7",
    "mm_teleporter",
    "Blue teleporter (135\xb0)"
  },
  {
    "mm_teleporter_blue_8",
    "mm_teleporter",
    "Blue teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_blue_9",
    "mm_teleporter",
    "Blue teleporter (180\xb0)"
  },
  {
    "mm_teleporter_blue_10",
    "mm_teleporter",
    "Blue teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_blue_11",
    "mm_teleporter",
    "Blue teleporter (225\xb0)"
  },
  {
    "mm_teleporter_blue_12",
    "mm_teleporter",
    "Blue teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_blue_13",
    "mm_teleporter",
    "Blue teleporter (270\xb0)"
  },
  {
    "mm_teleporter_blue_14",
    "mm_teleporter",
    "Blue teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_blue_15",
    "mm_teleporter",
    "Blue teleporter (315\xb0)"
  },
  {
    "mm_teleporter_blue_16",
    "mm_teleporter",
    "Blue teleporter (337.5\xb0)"
  },
  {
    "mm_mcduffin",
    "mm_mcduffin",
    "Gregor McDuffin"
  },
  {
    "mm_pacman",
    "mm_pacman",
    "Pac man (MM style)"
  },
  {
    "mm_fuse",
    "mm_fuse",
    "Fuse (off)",
  },
  {
    "mm_steel_wall",
    "mm_steel_wall",
    "Steel wall (MM style)",
  },
  {
    "mm_wooden_wall",
    "mm_wooden_wall",
    "Wooden wall (MM style)",
  },
  {
    "mm_ice_wall",
    "mm_ice_wall",
    "Ice wall",
  },
  {
    "mm_amoeba_wall",
    "mm_amoeba_wall",
    "Amoeba wall",
  },
  {
    "df_laser",
    "df_laser",
    "Laser cannon"
  },
  {
    "df_receiver",
    "df_receiver",
    "Laser receiver"
  },
  {
    "df_steel_wall",
    "df_steel_wall",
    "Steel wall (DF style)",
  },
  {
    "df_wooden_wall",
    "df_wooden_wall",
    "Wooden wall (DF style)",
  },
  {
    "spring.left",
    "spring",
    "Spring (starts moving left)"
  },
  {
    "spring.right",
    "spring",
    "Spring (starts moving right)"
  },
  {
    "empty_space_1",
    "empty_space",
    "Empty space 1"
  },
  {
    "empty_space_2",
    "empty_space",
    "Empty space 2"
  },
  {
    "empty_space_3",
    "empty_space",
    "Empty space 3"
  },
  {
    "empty_space_4",
    "empty_space",
    "Empty space 4"
  },
  {
    "empty_space_5",
    "empty_space",
    "Empty space 5"
  },
  {
    "empty_space_6",
    "empty_space",
    "Empty space 6"
  },
  {
    "empty_space_7",
    "empty_space",
    "Empty space 7"
  },
  {
    "empty_space_8",
    "empty_space",
    "Empty space 8"
  },
  {
    "empty_space_9",
    "empty_space",
    "Empty space 9"
  },
  {
    "empty_space_10",
    "empty_space",
    "Empty space 10"
  },
  {
    "empty_space_11",
    "empty_space",
    "Empty space 11"
  },
  {
    "empty_space_12",
    "empty_space",
    "Empty space 12"
  },
  {
    "empty_space_13",
    "empty_space",
    "Empty space 13"
  },
  {
    "empty_space_14",
    "empty_space",
    "Empty space 14"
  },
  {
    "empty_space_15",
    "empty_space",
    "Empty space 15"
  },
  {
    "empty_space_16",
    "empty_space",
    "Empty space 16"
  },
  {
    "df_mirror_fixed_1",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (0\xb0)"
  },
  {
    "df_mirror_fixed_2",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (11.25\xb0)"
  },
  {
    "df_mirror_fixed_3",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (22.5\xb0)"
  },
  {
    "df_mirror_fixed_4",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (33.75\xb0)"
  },
  {
    "df_mirror_fixed_5",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (45\xb0)"
  },
  {
    "df_mirror_fixed_6",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (56.25\xb0)"
  },
  {
    "df_mirror_fixed_7",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (67.5\xb0)"
  },
  {
    "df_mirror_fixed_8",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (78.75\xb0)"
  },
  {
    "df_mirror_fixed_9",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (90\xb0)"
  },
  {
    "df_mirror_fixed_10",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (101.25\xb0)"
  },
  {
    "df_mirror_fixed_11",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (112.5\xb0)"
  },
  {
    "df_mirror_fixed_12",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (123.75\xb0)"
  },
  {
    "df_mirror_fixed_13",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (135\xb0)"
  },
  {
    "df_mirror_fixed_14",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (146.25\xb0)"
  },
  {
    "df_mirror_fixed_15",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (157.5\xb0)"
  },
  {
    "df_mirror_fixed_16",
    "df_mirror_fixed",
    "Fixed mirror (DF style) (168.75\xb0)"
  },
  {
    "df_slope_1",
    "df_slope",
    "Slope (DF style) (45\xb0)"
  },
  {
    "df_slope_2",
    "df_slope",
    "Slope (DF style) (135\xb0)"
  },
  {
    "df_slope_3",
    "df_slope",
    "Slope (DF style) (225\xb0)"
  },
  {
    "df_slope_4",
    "df_slope",
    "Slope (DF style) (315\xb0)"
  },
  {
    "bdx_empty_space",
    "empty_space",
    "Empty space"
  },
  {
    "bdx_sand",
    "bdx_sand",
    "Sand"
  },
  {
    "bdx_grass",
    "bdx_grass",
    "Grass"
  },
  {
    "bdx_grass_ball",
    "bdx_grass_ball",
    "Grass ball"
  },
  {
    "bdx_grass_loose",
    "bdx_grass_loose",
    "Loose grass"
  },
  {
    "bdx_sand_sloped_up_right",
    "bdx_sand_sloped",
    "Sand (sloped up right)"
  },
  {
    "bdx_sand_sloped_up_left",
    "bdx_sand_sloped",
    "Sand (sloped up left)"
  },
  {
    "bdx_sand_sloped_down_left",
    "bdx_sand_sloped",
    "Sand (sloped down left)"
  },
  {
    "bdx_sand_sloped_down_right",
    "bdx_sand_sloped",
    "Sand (sloped down right)"
  },
  {
    "bdx_sand_glued",
    "bdx_sand_glued",
    "Glued sand"
  },
  {
    "bdx_wall_sloped_up_right",
    "bdx_wall_sloped",
    "Wall (sloped up right)"
  },
  {
    "bdx_wall_sloped_up_left",
    "bdx_wall_sloped",
    "Wall (sloped up left)"
  },
  {
    "bdx_wall_sloped_down_left",
    "bdx_wall_sloped",
    "Wall (sloped down left)"
  },
  {
    "bdx_wall_sloped_down_right",
    "bdx_wall_sloped",
    "Wall (sloped down right)"
  },
  {
    "bdx_wall_non_sloped",
    "bdx_wall",
    "Wall (non sloped)"
  },
  {
    "bdx_wall_diggable",
    "bdx_wall",
    "Diggable wall"
  },
  {
    "bdx_wall_diamond",
    "bdx_wall",
    "Wall with diamond"
  },
  {
    "bdx_wall_key_1",
    "bdx_wall_key",
    "Wall with key 1"
  },
  {
    "bdx_wall_key_2",
    "bdx_wall_key",
    "Wall with key 2"
  },
  {
    "bdx_wall_key_3",
    "bdx_wall_key",
    "Wall with key 3"
  },
  {
    "bdx_falling_wall",
    "bdx_wall",
    "Falling wall"
  },
  {
    "bdx_steelwall",
    "bdx_steelwall",
    "Steel wall"
  },
  {
    "bdx_steelwall_sloped_up_right",
    "bdx_steelwall_sloped",
    "Steel wall (sloped up right)"
  },
  {
    "bdx_steelwall_sloped_up_left",
    "bdx_steelwall_sloped",
    "Steel wall (sloped up left)"
  },
  {
    "bdx_steelwall_sloped_down_left",
    "bdx_steelwall_sloped",
    "Steel wall (sloped down left)"
  },
  {
    "bdx_steelwall_sloped_down_right",
    "bdx_steelwall_sloped",
    "Steel wall (sloped down right)"
  },
  {
    "bdx_steelwall_explodable",
    "bdx_steelwall",
    "Explodable steel wall"
  },
  {
    "bdx_steelwall_diggable",
    "bdx_steelwall",
    "Diggable steel wall"
  },
  {
    "bdx_expandable_wall_horizontal",
    "bdx_expandable_wall",
    "Expandable wall (horizontal)"
  },
  {
    "bdx_expandable_wall_vertical",
    "bdx_expandable_wall",
    "Expandable wall (vertical)"
  },
  {
    "bdx_expandable_wall_any",
    "bdx_expandable_wall",
    "Expandable wall (any direction)"
  },
  {
    "bdx_expandable_steelwall_horizontal",
    "bdx_expandable_steelwall",
    "Expandable steelwall (horizontal)"
  },
  {
    "bdx_expandable_steelwall_vertical",
    "bdx_expandable_steelwall",
    "Expandable steelwall (vertical)"
  },
  {
    "bdx_expandable_steelwall_any",
    "bdx_expandable_steelwall",
    "Expandable steelwall (any direction)"
  },
  {
    "bdx_expandable_wall_switch",
    "bdx_expandable_wall_switch",
    "Expandable wall switch"
  },
  {
    "bdx_expandable_wall_switch.active",
    "bdx_expandable_wall_switch",
    "Expandable wall switch (active)"
  },
  {
    "bdx_inbox",
    "bdx_inbox",
    "Inbox with player"
  },
  {
    "bdx_exit_closed",
    "bdx_exit",
    "Closed exit"
  },
  {
    "bdx_exit_open",
    "bdx_exit",
    "Open exit"
  },
  {
    "bdx_invisible_exit_closed",
    "bdx_invisible_exit",
    "Invisible exit (closed)"
  },
  {
    "bdx_invisible_exit_open",
    "bdx_invisible_exit",
    "Invisible exit (open)"
  },
  {
    "bdx_flying_rock",
    "bdx_flying_rock",
    "Flying rock"
  },
  {
    "bdx_heavy_rock",
    "bdx_heavy_rock",
    "Heavy rock"
  },
  {
    "bdx_rock_glued",
    "bdx_rock_glued",
    "Glued rock"
  },
  {
    "bdx_flying_diamond",
    "bdx_flying_diamond",
    "Flying diamond"
  },
  {
    "bdx_diamond_glued",
    "bdx_diamond_glued",
    "Glued diamond"
  },
  {
    "bdx_diamond_key",
    "bdx_diamond_key",
    "Diamond key"
  },
  {
    "bdx_trapped_diamond",
    "bdx_trapped_diamond",
    "Trapped diamond"
  },
  {
    "bdx_nut",
    "bdx_nut",
    "Nut"
  },
  {
    "bdx_amoeba_1",
    "bdx_amoeba",
    "Amoeba 1"
  },
  {
    "bdx_amoeba_2",
    "bdx_amoeba",
    "Amoeba 2"
  },
  {
    "bdx_bubble",
    "bdx_bubble",
    "Bubble"
  },
  {
    "bdx_trapped_bubble",
    "bdx_trapped_bubble",
    "Trapped bubble"
  },
  {
    "bdx_creature_switch",
    "bdx_creature_switch",
    "Creature direction switch"
  },
  {
    "bdx_creature_switch.active",
    "bdx_creature_switch",
    "Creature direction switch (active)"
  },
  {
    "bdx_biter_switch_1",
    "bdx_biter_switch",
    "Biter switch"
  },
  {
    "bdx_biter_switch_2",
    "bdx_biter_switch",
    "Biter switch"
  },
  {
    "bdx_biter_switch_3",
    "bdx_biter_switch",
    "Biter switch"
  },
  {
    "bdx_biter_switch_4",
    "bdx_biter_switch",
    "Biter switch"
  },
  {
    "bdx_replicator",
    "bdx_replicator",
    "Replicator"
  },
  {
    "bdx_replicator.active",
    "bdx_replicator",
    "Replicator (active)"
  },
  {
    "bdx_replicator_switch",
    "bdx_replicator_switch",
    "Replicator switch"
  },
  {
    "bdx_replicator_switch.active",
    "bdx_replicator_switch",
    "Replicator switch (active)"
  },
  {
    "bdx_conveyor_left",
    "bdx_conveyor",
    "Conveyor belt (moving left)"
  },
  {
    "bdx_conveyor_left.active",
    "bdx_conveyor",
    "Conveyor belt (moving left) (active)"
  },
  {
    "bdx_conveyor_right",
    "bdx_conveyor",
    "Conveyor belt (moving right)"
  },
  {
    "bdx_conveyor_right.active",
    "bdx_conveyor",
    "Conveyor belt (moving right) (active)"
  },
  {
    "bdx_conveyor_switch",
    "bdx_conveyor_switch",
    "Conveyor belt power switch"
  },
  {
    "bdx_conveyor_switch.active",
    "bdx_conveyor_switch",
    "Conveyor belt power switch (active)"
  },
  {
    "bdx_conveyor_dir_switch",
    "bdx_conveyor_dir_switch",
    "Conveyor belt direction switch"
  },
  {
    "bdx_conveyor_dir_switch.active",
    "bdx_conveyor_dir_switch",
    "Conveyor belt direction switch (active)"
  },
  {
    "bdx_gravity_switch",
    "bdx_gravity_switch",
    "Gravity switch"
  },
  {
    "bdx_gravity_switch.active",
    "bdx_gravity_switch",
    "Gravity switch (active)"
  },
  {
    "bdx_acid",
    "bdx_acid",
    "Acid"
  },
  {
    "bdx_box",
    "bdx_box",
    "Box"
  },
  {
    "bdx_time_penalty",
    "bdx_time_penalty",
    "Time penalty"
  },
  {
    "bdx_gravestone",
    "bdx_gravestone",
    "Gravestone"
  },
  {
    "bdx_clock",
    "bdx_clock",
    "Clock"
  },
  {
    "bdx_pot",
    "bdx_pot",
    "Pot"
  },
  {
    "bdx_pneumatic_hammer",
    "bdx_pneumatic_hammer",
    "Pneumatic hammer"
  },
  {
    "bdx_teleporter",
    "bdx_teleporter",
    "Teleporter"
  },
  {
    "bdx_skeleton",
    "bdx_skeleton",
    "Skeleton"
  },
  {
    "bdx_water",
    "bdx_water",
    "Water"
  },
  {
    "bdx_key_1",
    "bdx_key",
    "Key 1"
    },
  {
    "bdx_key_2",
    "bdx_key",
    "Key 2"
    },
  {
    "bdx_key_3",
    "bdx_key",
    "Key 3"
  },
  {
    "bdx_gate_1",
    "bdx_gate",
    "Door 1"
  },
  {
    "bdx_gate_2",
    "bdx_gate",
    "Door 2"
  },
  {
    "bdx_gate_3",
    "bdx_gate",
    "Door 3"
  },
  {
    "bdx_lava",
    "bdx_lava",
    "Lava"
  },
  {
    "bdx_sweet",
    "bdx_sweet",
    "Sweet"
  },
  {
    "bdx_voodoo_doll",
    "bdx_voodoo_doll",
    "Voodoo doll"
  },
  {
    "bdx_slime",
    "bdx_slime",
    "Slime"
  },
  {
    "bdx_waiting_rock",
    "bdx_waiting_rock",
    "Waiting rock"
  },
  {
    "bdx_chasing_rock",
    "bdx_chasing_rock",
    "Chasing rock"
  },
  {
    "bdx_ghost",
    "bdx_ghost",
    "Ghost"
  },
  {
    "bdx_cow",
    "bdx_cow",
    "Cow (random start direction)"
  },
  {
    "bdx_cow.left",
    "bdx_cow",
    "Cow (starts moving left)"
  },
  {
    "bdx_cow.up",
    "bdx_cow",
    "Cow (starts moving up)"
  },
  {
    "bdx_cow.right",
    "bdx_cow",
    "Cow (starts moving right)"
  },
  {
    "bdx_cow.down",
    "bdx_cow",
    "Cow (starts moving down)"
  },
  {
    "bdx_butterfly_1",
    "bdx_butterfly",
    "Butterfly 1 (random start direction)"
  },
  {
    "bdx_butterfly_1.right",
    "bdx_butterfly",
    "Butterfly 1 (starts moving right)"
  },
  {
    "bdx_butterfly_1.up",
    "bdx_butterfly",
    "Butterfly 1 (starts moving up)"
  },
  {
    "bdx_butterfly_1.left",
    "bdx_butterfly",
    "Butterfly 1 (starts moving left)"
  },
  {
    "bdx_butterfly_1.down",
    "bdx_butterfly",
    "Butterfly 1 (starts moving down)"
  },
  {
    "bdx_butterfly_2",
    "bdx_butterfly",
    "Butterfly 2 (random start direction)"
  },
  {
    "bdx_butterfly_2.right",
    "bdx_butterfly",
    "Butterfly 2 (starts moving right)"
  },
  {
    "bdx_butterfly_2.up",
    "bdx_butterfly",
    "Butterfly 2 (starts moving up)"
  },
  {
    "bdx_butterfly_2.left",
    "bdx_butterfly",
    "Butterfly 2 (starts moving left)"
  },
  {
    "bdx_butterfly_2.down",
    "bdx_butterfly",
    "Butterfly 2 (starts moving down)"
  },
  {
    "bdx_firefly_1",
    "bdx_firefly",
    "Firefly 1 (random start direction)"
  },
  {
    "bdx_firefly_1.right",
    "bdx_firefly",
    "Firefly 1 (starts moving right)"
  },
  {
    "bdx_firefly_1.up",
    "bdx_firefly",
    "Firefly 1 (starts moving up)"
  },
  {
    "bdx_firefly_1.left",
    "bdx_firefly",
    "Firefly 1 (starts moving left)"
  },
  {
    "bdx_firefly_1.down",
    "bdx_firefly",
    "Firefly 1 (starts moving down)"
  },
  {
    "bdx_firefly_2",
    "bdx_firefly",
    "Firefly 2 (random start direction)"
  },
  {
    "bdx_firefly_2.right",
    "bdx_firefly",
    "Firefly 2 (starts moving right)"
  },
  {
    "bdx_firefly_2.up",
    "bdx_firefly",
    "Firefly 2 (starts moving up)"
  },
  {
    "bdx_firefly_2.left",
    "bdx_firefly",
    "Firefly 2 (starts moving left)"
  },
  {
    "bdx_firefly_2.down",
    "bdx_firefly",
    "Firefly 2 (starts moving down)"
  },
  {
    "bdx_stonefly",
    "bdx_stonefly",
    "Stonefly (random start direction)"
  },
  {
    "bdx_stonefly.right",
    "bdx_stonefly",
    "Stonefly (starts moving right)"
  },
  {
    "bdx_stonefly.up",
    "bdx_stonefly",
    "Stonefly (starts moving up)"
  },
  {
    "bdx_stonefly.left",
    "bdx_stonefly",
    "Stonefly (starts moving left)"
  },
  {
    "bdx_stonefly.down",
    "bdx_stonefly",
    "Stonefly (starts moving down)"
  },
  {
    "bdx_biter",
    "bdx_biter",
    "Biter (random start direction)"
  },
  {
    "bdx_biter.right",
    "bdx_biter",
    "Biter (starts moving right)"
  },
  {
    "bdx_biter.up",
    "bdx_biter",
    "Biter (starts moving up)"
  },
  {
    "bdx_biter.left",
    "bdx_biter",
    "Biter (starts moving left)"
  },
  {
    "bdx_biter.down",
    "bdx_biter",
    "Biter (starts moving down)"
  },
  {
    "bdx_dragonfly",
    "bdx_dragonfly",
    "Dragonfly (random start direction)"
  },
  {
    "bdx_dragonfly.right",
    "bdx_dragonfly",
    "Dragonfly (starts moving right)"
  },
  {
    "bdx_dragonfly.up",
    "bdx_dragonfly",
    "Dragonfly (starts moving up)"
  },
  {
    "bdx_dragonfly.left",
    "bdx_dragonfly",
    "Dragonfly (starts moving left)"
  },
  {
    "bdx_dragonfly.down",
    "bdx_dragonfly",
    "Dragonfly (starts moving down)"
  },
  {
    "bdx_bomb",
    "bdx_bomb",
    "Bomb"
  },
  {
    "bdx_nitro_pack",
    "bdx_nitro_pack",
    "Nitro pack"
  },
  {
    "bdx_player",
    "bdx_player",
    "Player"
  },
  {
    "bdx_player_with_bomb",
    "bdx_player",
    "Player with bomb",
  },
  {
    "bdx_player_with_rocket_launcher",
    "bdx_player",
    "Player with rocket launcher",
  },
  {
    "bdx_player_glued",
    "bdx_player",
    "Glued player",
  },
  {
    "bdx_player_stirring",
    "bdx_player",
    "Stirring player"
  },
  {
    "bdx_rocket_launcher",
    "bdx_rocket_launcher",
    "Rocket launcher",
  },
  {
    "bdx_rocket",
    "bdx_rocket",
    "Rocket",
  },
  {
    "bdx_rocket.right",
    "bdx_rocket",
    "Rocket (starts moving right)"
  },
  {
    "bdx_rocket.up",
    "bdx_rocket",
    "Rocket (starts moving up)"
  },
  {
    "bdx_rocket.left",
    "bdx_rocket",
    "Rocket (starts moving left)"
  },
  {
    "bdx_rocket.down",
    "bdx_rocket",
    "Rocket (starts moving down)"
  },
  {
    "bdx_fake_bonus",
    "bdx_fake_bonus",
    "Fake bonus"
  },
  {
    "bdx_covered",
    "bdx_covered",
    "Covered"
  },
  {
    "bdx_wall",
    "wall",
    "Normal wall"
  },
  {
    "bdx_rock",
    "bdx_rock",
    "Rock"
  },
  {
    "bdx_diamond",
    "bdx_diamond",
    "Diamond"
  },
  {
    "bdx_magic_wall",
    "bdx_magic_wall",
    "Magic wall"
  },
  {
    "bdx_light_rock",
    "bdx_light_rock",
    "Light rock"
  },
  {
    "bdx_grass_ball.falling",
    "bdx_grass_ball",
    "Grass ball (falling)"
  },
  {
    "bdx_grass_loose.falling",
    "bdx_grass_loose",
    "Loose grass (falling)"
  },
  {
    "bdx_rock.falling",
    "bdx_rock",
    "Rock (falling)"
  },
  {
    "bdx_flying_rock.flying",
    "bdx_flying_rock",
    "Flying rock (flying)"
  },
  {
    "bdx_heavy_rock.falling",
    "bdx_heavy_rock",
    "Heavy rock (falling)"
  },
  {
    "bdx_light_rock.falling",
    "bdx_light_rock",
    "Light rock (falling)"
  },
  {
    "bdx_diamond.falling",
    "bdx_diamond",
    "Diamond (falling)"
  },
  {
    "bdx_flying_diamond.flying",
    "bdx_flying_diamond",
    "Flying diamond (flying)"
  },
  {
    "bdx_nut.falling",
    "bdx_nut",
    "Nut (falling)"
  },
  {
    "bdx_falling_wall.falling",
    "bdx_wall",
    "Falling wall (falling)"
  },
  {
    "bdx_nitro_pack.falling",
    "bdx_nitro_pack",
    "Nitro pack (falling)"
  },
  {
    "bdx_water_1",
    "bdx_water",
    "Water (1)"
  },
  {
    "bdx_water_2",
    "bdx_water",
    "Water (2)"
  },
  {
    "bdx_water_3",
    "bdx_water",
    "Water (3)"
  },
  {
    "bdx_water_4",
    "bdx_water",
    "Water (4)"
  },
  {
    "bdx_water_5",
    "bdx_water",
    "Water (5)"
  },
  {
    "bdx_water_6",
    "bdx_water",
    "Water (6)"
  },
  {
    "bdx_water_7",
    "bdx_water",
    "Water (7)"
  },
  {
    "bdx_water_8",
    "bdx_water",
    "Water (8)"
  },
  {
    "bdx_water_9",
    "bdx_water",
    "Water (9)"
  },
  {
    "bdx_water_10",
    "bdx_water",
    "Water (10)"
  },
  {
    "bdx_water_11",
    "bdx_water",
    "Water (11)"
  },
  {
    "bdx_water_12",
    "bdx_water",
    "Water (12)"
  },
  {
    "bdx_water_13",
    "bdx_water",
    "Water (13)"
  },
  {
    "bdx_water_14",
    "bdx_water",
    "Water (14)"
  },
  {
    "bdx_water_15",
    "bdx_water",
    "Water (15)"
  },
  {
    "bdx_water_16",
    "bdx_water",
    "Water (16)"
  },
  {
    "bdx_cow_enclosed_1",
    "bdx_cow",
    "Enclosed cow (1)"
  },
  {
    "bdx_cow_enclosed_2",
    "bdx_cow",
    "Enclosed cow (2)"
  },
  {
    "bdx_cow_enclosed_3",
    "bdx_cow",
    "Enclosed cow (3)"
  },
  {
    "bdx_cow_enclosed_4",
    "bdx_cow",
    "Enclosed cow (4)"
  },
  {
    "bdx_cow_enclosed_5",
    "bdx_cow",
    "Enclosed cow (5)"
  },
  {
    "bdx_cow_enclosed_6",
    "bdx_cow",
    "Enclosed cow (6)"
  },
  {
    "bdx_cow_enclosed_7",
    "bdx_cow",
    "Enclosed cow (7)"
  },
  {
    "bdx_bubble_1",
    "bdx_bubble",
    "Bubble (1)"
  },
  {
    "bdx_bubble_2",
    "bdx_bubble",
    "Bubble (2)"
  },
  {
    "bdx_bubble_3",
    "bdx_bubble",
    "Bubble (3)"
  },
  {
    "bdx_bubble_4",
    "bdx_bubble",
    "Bubble (4)"
  },
  {
    "bdx_bubble_5",
    "bdx_bubble",
    "Bubble (5)"
  },
  {
    "bdx_bubble_6",
    "bdx_bubble",
    "Bubble (6)"
  },
  {
    "bdx_bubble_7",
    "bdx_bubble",
    "Bubble (7)"
  },
  {
    "bdx_bubble_8",
    "bdx_bubble",
    "Bubble (8)"
  },
  {
    "bdx_player.growing_1",
    "bdx_player",
    "Player birth (1)"
  },
  {
    "bdx_player.growing_2",
    "bdx_player",
    "Player birth (2)"
  },
  {
    "bdx_player.growing_3",
    "bdx_player",
    "Player birth (3)"
  },
  {
    "bdx_bomb.ticking_1",
    "bdx_bomb",
    "Ticking bomb (1)"
  },
  {
    "bdx_bomb.ticking_2",
    "bdx_bomb",
    "Ticking bomb (2)"
  },
  {
    "bdx_bomb.ticking_3",
    "bdx_bomb",
    "Ticking bomb (3)"
  },
  {
    "bdx_bomb.ticking_4",
    "bdx_bomb",
    "Ticking bomb (4)"
  },
  {
    "bdx_bomb.ticking_5",
    "bdx_bomb",
    "Ticking bomb (5)"
  },
  {
    "bdx_bomb.ticking_6",
    "bdx_bomb",
    "Ticking bomb (6)"
  },
  {
    "bdx_bomb.ticking_7",
    "bdx_bomb",
    "Ticking bomb (7)"
  },
  {
    "bdx_clock.growing_1",
    "bdx_clock",
    "Clock birth (1)"
  },
  {
    "bdx_clock.growing_2",
    "bdx_clock",
    "Clock birth (2)"
  },
  {
    "bdx_clock.growing_3",
    "bdx_clock",
    "Clock birth (3)"
  },
  {
    "bdx_clock.growing_4",
    "bdx_clock",
    "Clock birth (4)"
  },
  {
    "bdx_diamond.growing_1",
    "bdx_diamond",
    "Diamond birth (1)"
  },
  {
    "bdx_diamond.growing_2",
    "bdx_diamond",
    "Diamond birth (2)"
  },
  {
    "bdx_diamond.growing_3",
    "bdx_diamond",
    "Diamond birth (3)"
  },
  {
    "bdx_diamond.growing_4",
    "bdx_diamond",
    "Diamond birth (4)"
  },
  {
    "bdx_diamond.growing_5",
    "bdx_diamond",
    "Diamond birth (5)"
  },
  {
    "bdx_exploding_1",
    "bdx_exploding",
    "Explosion (1)"
  },
  {
    "bdx_exploding_2",
    "bdx_exploding",
    "Explosion (2)"
  },
  {
    "bdx_exploding_3",
    "bdx_exploding",
    "Explosion (3)"
  },
  {
    "bdx_exploding_4",
    "bdx_exploding",
    "Explosion (4)"
  },
  {
    "bdx_exploding_5",
    "bdx_exploding",
    "Explosion (5)"
  },
  {
    "bdx_rock.growing_1",
    "bdx_rock",
    "Rock birth (1)"
  },
  {
    "bdx_rock.growing_2",
    "bdx_rock",
    "Rock birth (2)"
  },
  {
    "bdx_rock.growing_3",
    "bdx_rock",
    "Rock birth (3)"
  },
  {
    "bdx_rock.growing_4",
    "bdx_rock",
    "Rock birth (4)"
  },
  {
    "bdx_steelwall.growing_1",
    "bdx_steelwall",
    "Steelwall birth (1)"
  },
  {
    "bdx_steelwall.growing_2",
    "bdx_steelwall",
    "Steelwall birth (2)"
  },
  {
    "bdx_steelwall.growing_3",
    "bdx_steelwall",
    "Steelwall birth (3)"
  },
  {
    "bdx_steelwall.growing_4",
    "bdx_steelwall",
    "Steelwall birth (4)"
  },
  {
    "bdx_ghost.exploding_1",
    "bdx_ghost",
    "Ghost explosion (1)"
  },
  {
    "bdx_ghost.exploding_2",
    "bdx_ghost",
    "Ghost explosion (2)"
  },
  {
    "bdx_ghost.exploding_3",
    "bdx_ghost",
    "Ghost explosion (3)"
  },
  {
    "bdx_ghost.exploding_4",
    "bdx_ghost",
    "Ghost explosion (4)"
  },
  {
    "bdx_bomb.exploding_1",
    "bdx_bomb",
    "Bomb explosion (1)"
  },
  {
    "bdx_bomb.exploding_2",
    "bdx_bomb",
    "Bomb explosion (2)"
  },
  {
    "bdx_bomb.exploding_3",
    "bdx_bomb",
    "Bomb explosion (3)"
  },
  {
    "bdx_bomb.exploding_4",
    "bdx_bomb",
    "Bomb explosion (4)"
  },
  {
    "bdx_nitro_pack.exploding",
    "bdx_nitro_pack",
    "Nitro pack (triggered)"
  },
  {
    "bdx_nitro_pack.exploding_1",
    "bdx_nitro_pack",
    "Nitro pack explosion (1)"
  },
  {
    "bdx_nitro_pack.exploding_2",
    "bdx_nitro_pack",
    "Nitro pack explosion (2)"
  },
  {
    "bdx_nitro_pack.exploding_3",
    "bdx_nitro_pack",
    "Nitro pack explosion (3)"
  },
  {
    "bdx_nitro_pack.exploding_4",
    "bdx_nitro_pack",
    "Nitro pack explosion (4)"
  },
  {
    "bdx_amoeba_2.exploding_1",
    "bdx_amoeba",
    "Amoeba 2 explosion (1)"
  },
  {
    "bdx_amoeba_2.exploding_2",
    "bdx_amoeba",
    "Amoeba 2 explosion (2)"
  },
  {
    "bdx_amoeba_2.exploding_3",
    "bdx_amoeba",
    "Amoeba 2 explosion (3)"
  },
  {
    "bdx_amoeba_2.exploding_4",
    "bdx_amoeba",
    "Amoeba 2 explosion (4)"
  },
  {
    "bdx_nut.breaking_1",
    "bdx_nut",
    "Nut explosion (1)"
  },
  {
    "bdx_nut.breaking_2",
    "bdx_nut",
    "Nut explosion (2)"
  },
  {
    "bdx_nut.breaking_3",
    "bdx_nut",
    "Nut explosion (3)"
  },
  {
    "bdx_nut.breaking_4",
    "bdx_nut",
    "Nut explosion (4)"
  },
  {
    "bdx_grass_ball.scanned",
    "bdx_grass_ball",
    "Grass ball (scanned)"
  },
  {
    "bdx_grass_ball.falling.scanned",
    "bdx_grass_ball",
    "Grass ball (falling) (scanned)"
  },
  {
    "bdx_grass_loose.scanned",
    "bdx_grass_loose",
    "Loose grass (scanned)"
  },
  {
    "bdx_grass_loose.falling.scanned",
    "bdx_grass_loose",
    "Loose grass (falling) (scanned)"
  },
  {
    "bdx_rock.scanned",
    "bdx_rock",
    "Rock (scanned)"
  },
  {
    "bdx_rock.falling.scanned",
    "bdx_rock",
    "Rock (falling) (scanned)"
  },
  {
    "bdx_flying_rock.scanned",
    "bdx_flying_rock",
    "Flying rock (scanned)"
  },
  {
    "bdx_flying_rock.flying.scanned",
    "bdx_flying_rock",
    "Flying rock (flying) (scanned)"
  },
  {
    "bdx_heavy_rock.scanned",
    "bdx_heavy_rock",
    "Heavy rock (scanned)"
  },
  {
    "bdx_heavy_rock.falling.scanned",
    "bdx_heavy_rock",
    "Heavy rock (falling) (scanned)"
  },
  {
    "bdx_light_rock.scanned",
    "bdx_light_rock",
    "Light rock (scanned)"
  },
  {
    "bdx_light_rock.falling.scanned",
    "bdx_light_rock",
    "Light rock (falling) (scanned)"
  },
  {
    "bdx_diamond.scanned",
    "bdx_diamond",
    "Diamond (scanned)"
  },
  {
    "bdx_diamond.falling.scanned",
    "bdx_diamond",
    "Diamond (falling) (scanned)"
  },
  {
    "bdx_flying_diamond.scanned",
    "bdx_flying_diamond",
    "Flying diamond (scanned)"
  },
  {
    "bdx_flying_diamond.flying.scanned",
    "bdx_flying_diamond",
    "Flying diamond (flying) (scanned)"
  },
  {
    "bdx_nut.scanned",
    "bdx_nut",
    "Nut (scanned)"
  },
  {
    "bdx_nut.falling.scanned",
    "bdx_nut",
    "Nut (falling) (scanned)"
  },
  {
    "bdx_expandable_wall_horizontal.scanned",
    "bdx_expandable_wall",
    "Expandable wall (horizontal) (scanned)"
  },
  {
    "bdx_expandable_wall_vertical.scanned",
    "bdx_expandable_wall",
    "Expandable wall (vertical) (scanned)"
  },
  {
    "bdx_expandable_wall_any.scanned",
    "bdx_expandable_wall",
    "Expandable wall (any direction) (scanned)"
  },
  {
    "bdx_expandable_steelwall_horizontal.scanned",
    "bdx_expandable_steelwall",
    "Expandable steelwall (horizontal) (scanned)"
  },
  {
    "bdx_expandable_steelwall_vertical.scanned",
    "bdx_expandable_steelwall",
    "Expandable steelwall (vertical) (scanned)"
  },
  {
    "bdx_expandable_steelwall_any.scanned",
    "bdx_expandable_steelwall",
    "Expandable steelwall (any direction) (scanned)"
  },
  {
    "bdx_acid.scanned",
    "bdx_acid",
    "Acid (scanned)"
  },
  {
    "bdx_falling_wall.falling.scanned",
    "bdx_wall",
    "Falling wall (falling) (scanned)"
  },
  {
    "bdx_cow.left.scanned",
    "bdx_cow",
    "Cow (starts moving left) (scanned)"
  },
  {
    "bdx_cow.up.scanned",
    "bdx_cow",
    "Cow (starts moving up) (scanned)"
  },
  {
    "bdx_cow.right.scanned",
    "bdx_cow",
    "Cow (starts moving right) (scanned)"
  },
  {
    "bdx_cow.down.scanned",
    "bdx_cow",
    "Cow (starts moving down) (scanned)"
  },
  {
    "bdx_amoeba_1.scanned",
    "bdx_amoeba",
    "Amoeba 1 (scanned)"
  },
  {
    "bdx_amoeba_2.scanned",
    "bdx_amoeba",
    "Amoeba 2 (scanned)"
  },
  {
    "bdx_waiting_rock.scanned",
    "bdx_waiting_rock",
    "Waiting rock (scanned)"
  },
  {
    "bdx_chasing_rock.scanned",
    "bdx_chasing_rock",
    "Chasing rock (scanned)"
  },
  {
    "bdx_ghost.scanned",
    "bdx_ghost",
    "Ghost (scanned)"
  },
  {
    "bdx_firefly_1.right.scanned",
    "bdx_firefly",
    "Firefly 1 (starts moving right) (scanned)"
  },
  {
    "bdx_firefly_1.up.scanned",
    "bdx_firefly",
    "Firefly 1 (starts moving up) (scanned)"
  },
  {
    "bdx_firefly_1.left.scanned",
    "bdx_firefly",
    "Firefly 1 (starts moving left) (scanned)"
  },
  {
    "bdx_firefly_1.down.scanned",
    "bdx_firefly",
    "Firefly 1 (starts moving down) (scanned)"
  },
  {
    "bdx_firefly_2.right.scanned",
    "bdx_firefly",
    "Firefly 2 (starts moving right) (scanned)"
  },
  {
    "bdx_firefly_2.up.scanned",
    "bdx_firefly",
    "Firefly 2 (starts moving up) (scanned)"
  },
  {
    "bdx_firefly_2.left.scanned",
    "bdx_firefly",
    "Firefly 2 (starts moving left) (scanned)"
  },
  {
    "bdx_firefly_2.down.scanned",
    "bdx_firefly",
    "Firefly 2 (starts moving down) (scanned)"
  },
  {
    "bdx_butterfly_1.right.scanned",
    "bdx_butterfly",
    "Butterfly 1 (starts moving right) (scanned)"
  },
  {
    "bdx_butterfly_1.up.scanned",
    "bdx_butterfly",
    "Butterfly 1 (starts moving up) (scanned)"
  },
  {
    "bdx_butterfly_1.left.scanned",
    "bdx_butterfly",
    "Butterfly 1 (starts moving left) (scanned)"
  },
  {
    "bdx_butterfly_1.down.scanned",
    "bdx_butterfly",
    "Butterfly 1 (starts moving down) (scanned)"
  },
  {
    "bdx_butterfly_2.right.scanned",
    "bdx_butterfly",
    "Butterfly 2 (starts moving right) (scanned)"
  },
  {
    "bdx_butterfly_2.up.scanned",
    "bdx_butterfly",
    "Butterfly 2 (starts moving up) (scanned)"
  },
  {
    "bdx_butterfly_2.left.scanned",
    "bdx_butterfly",
    "Butterfly 2 (starts moving left) (scanned)"
  },
  {
    "bdx_butterfly_2.down.scanned",
    "bdx_butterfly",
    "Butterfly 2 (starts moving down) (scanned)"
  },
  {
    "bdx_stonefly.right.scanned",
    "bdx_stonefly",
    "Stonefly (starts moving right) (scanned)"
  },
  {
    "bdx_stonefly.up.scanned",
    "bdx_stonefly",
    "Stonefly (starts moving up) (scanned)"
  },
  {
    "bdx_stonefly.left.scanned",
    "bdx_stonefly",
    "Stonefly (starts moving left) (scanned)"
  },
  {
    "bdx_stonefly.down.scanned",
    "bdx_stonefly",
    "Stonefly (starts moving down) (scanned)"
  },
  {
    "bdx_biter.right.scanned",
    "bdx_biter",
    "Biter (starts moving right) (scanned)"
  },
  {
    "bdx_biter.up.scanned",
    "bdx_biter",
    "Biter (starts moving up) (scanned)"
  },
  {
    "bdx_biter.left.scanned",
    "bdx_biter",
    "Biter (starts moving left) (scanned)"
  },
  {
    "bdx_biter.down.scanned",
    "bdx_biter",
    "Biter (starts moving down) (scanned)"
  },
  {
    "bdx_dragonfly.right.scanned",
    "bdx_dragonfly",
    "Dragonfly (starts moving right) (scanned)"
  },
  {
    "bdx_dragonfly.up.scanned",
    "bdx_dragonfly",
    "Dragonfly (starts moving up) (scanned)"
  },
  {
    "bdx_dragonfly.left.scanned",
    "bdx_dragonfly",
    "Dragonfly (starts moving left) (scanned)"
  },
  {
    "bdx_dragonfly.down.scanned",
    "bdx_dragonfly",
    "Dragonfly (starts moving down) (scanned)"
  },
  {
    "bdx_player.scanned",
    "bdx_player",
    "Player (scanned)"
  },
  {
    "bdx_player_with_bomb.scanned",
    "bdx_player",
    "Player with bomb (scanned)",
  },
  {
    "bdx_player_with_rocket_launcher.scanned",
    "bdx_player",
    "Player with rocket launcher (scanned)",
  },
  {
    "bdx_rocket.right.scanned",
    "bdx_rocket",
    "Rocket (starts moving right) (scanned)"
  },
  {
    "bdx_rocket.up.scanned",
    "bdx_rocket",
    "Rocket (starts moving up) (scanned)"
  },
  {
    "bdx_rocket.left.scanned",
    "bdx_rocket",
    "Rocket (starts moving left) (scanned)"
  },
  {
    "bdx_rocket.down.scanned",
    "bdx_rocket",
    "Rocket (starts moving down) (scanned)"
  },
  {
    "bdx_nitro_pack.scanned",
    "bdx_nitro_pack",
    "Nitro pack (scanned)"
  },
  {
    "bdx_nitro_pack.falling.scanned",
    "bdx_nitro_pack",
    "Nitro pack (falling) (scanned)"
  },
  {
    "bdx_nitro_pack.exploding.scanned",
    "bdx_nitro_pack",
    "Nitro pack (triggered) (scanned)"
  },
  {
    "bdx_clock.growing_0",
    "bdx_clock",
    "Clock birth (0)"
  },
  {
    "bdx_diamond.growing_0",
    "bdx_diamond",
    "Diamond birth (0)"
  },
  {
    "bdx_exploding_0",
    "bdx_exploding",
    "Explosion (0)"
  },
  {
    "bdx_rock.growing_0",
    "bdx_rock",
    "Rock birth (0)"
  },
  {
    "bdx_steelwall.growing_0",
    "bdx_steelwall",
    "Steelwall birth (0)"
  },
  {
    "bdx_ghost.exploding_0",
    "bdx_ghost",
    "Ghost explosion (0)"
  },
  {
    "bdx_bomb.exploding_0",
    "bdx_bomb",
    "Bomb explosion (0)"
  },
  {
    "bdx_nitro_pack.exploding_0",
    "bdx_nitro_pack",
    "Nitro pack explosion (0)"
  },
  {
    "bdx_amoeba_2.exploding_0",
    "bdx_amoeba",
    "Amoeba 2 explosion (0)"
  },
  {
    "bdx_nut.breaking_0",
    "bdx_nut",
    "Nut explosion (0)"
  },

  // --------------------------------------------------------------------------
  // "real" (and therefore drawable) runtime elements
  // --------------------------------------------------------------------------

  {
    "dynabomb_player_1.active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_2.active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_3.active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_4.active",
    "dynabomb",
    "-"
  },
  {
    "sp_disk_red.active",
    "dynamite",
    "-"
  },
  {
    "switchgate.opening",
    "switchgate",
    "-"
  },
  {
    "switchgate.closing",
    "switchgate",
    "-"
  },
  {
    "timegate.opening",
    "timegate",
    "-"
  },
  {
    "timegate.closing",
    "timegate",
    "-"
  },
  {
    "pearl.breaking",
    "pearl",
    "-"
  },
  {
    "trap.active",
    "trap",
    "-"
  },
  {
    "invisible_steelwall.active",
    "steelwall",
    "-"
  },
  {
    "invisible_wall.active",
    "wall",
    "-"
  },
  {
    "invisible_sand.active",
    "sand",
    "-"
  },
  {
    "conveyor_belt_1_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_1_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_1_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "exit.opening",
    "exit",
    "-"
  },
  {
    "exit.closing",
    "exit",
    "-"
  },
  {
    "steel_exit.opening",
    "steel_exit",
    "-"
  },
  {
    "steel_exit.closing",
    "steel_exit",
    "-"
  },
  {
    "em_exit.opening",
    "em_exit",
    "-"
  },
  {
    "em_exit.closing",
    "em_exit",
    "-"
  },
  {
    "em_steel_exit.opening",
    "em_steel_exit",
    "-"
  },
  {
    "em_steel_exit.closing",
    "em_steel_exit",
    "-"
  },
  {
    "sp_exit.opening",
    "sp_exit",
    "-"
  },
  {
    "sp_exit.closing",
    "sp_exit",
    "-"
  },
  {
    "sp_exit_open",
    "sp_exit",
    "-"
  },
  {
    "sp_terminal.active",
    "sp_terminal",
    "-"
  },
  {
    "sp_buggy_base.activating",
    "sp_buggy_base",
    "-"
  },
  {
    "sp_buggy_base.active",
    "sp_buggy_base",
    "-"
  },
  {
    "sp_murphy_clone",
    "murphy_clone",
    "-"
  },
  {
    "amoeba.dropping",
    "amoeba",
    "-"
  },
  {
    "quicksand.emptying",
    "quicksand",
    "-"
  },
  {
    "quicksand_fast.emptying",
    "quicksand",
    "-"
  },
  {
    "magic_wall.active",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall.active",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall.active",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "magic_wall.emptying",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall.emptying",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall.emptying",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_dead",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_dead",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall_dead",
    "magic_wall",
    "-"
  },

  {
    "emc_fake_grass.active",
    "fake_grass",
    "-"
  },
  {
    "gate_1_gray.active",
    "gate",
    ""
  },
  {
    "gate_2_gray.active",
    "gate",
    ""
  },
  {
    "gate_3_gray.active",
    "gate",
    ""
  },
  {
    "gate_4_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_1_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_2_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_3_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_4_gray.active",
    "gate",
    ""
  },
  {
    "emc_gate_5_gray.active",
    "gate",
    "",
  },
  {
    "emc_gate_6_gray.active",
    "gate",
    "",
  },
  {
    "emc_gate_7_gray.active",
    "gate",
    "",
  },
  {
    "emc_gate_8_gray.active",
    "gate",
    "",
  },
  {
    "dc_gate_white_gray.active",
    "gate",
    "",
  },
  {
    "emc_dripper.active",
    "dripper",
    "Dripper"
  },
  {
    "emc_spring_bumper.active",
    "emc_spring_bumper",
    "Spring bumper",
  },
  {
    "mm_exit.opening",
    "mm_exit",
    "-"
  },
  {
    "mm_exit.closing",
    "mm_exit",
    "-"
  },
  {
    "mm_gray_ball.active",
    "mm_gray_ball",
    "-",
  },
  {
    "mm_gray_ball.opening",
    "mm_gray_ball",
    "-",
  },
  {
    "mm_ice_wall.shrinking",
    "mm_ice_wall",
    "-",
  },
  {
    "mm_amoeba_wall.growing",
    "mm_amoeba_wall",
    "-",
  },
  {
    "mm_pacman.eating.right",
    "mm_pacman",
    "Pac man (eating right)"
  },
  {
    "mm_pacman.eating.up",
    "mm_pacman",
    "Pac man (eating up)"
  },
  {
    "mm_pacman.eating.left",
    "mm_pacman",
    "Pac man (eating left)"
  },
  {
    "mm_pacman.eating.down",
    "mm_pacman",
    "Pac man (eating down)"
  },
  {
    "mm_bomb.active",
    "mm_bomb",
    "Active bomb (MM style)"
  },
  {
    "df_mine.active",
    "df_mine",
    "Active mine"
  },
  {
    "bdx_magic_wall.active",
    "magic_wall",
    "-"
  },

  // --------------------------------------------------------------------------
  // "unreal" (and therefore not drawable) runtime elements
  // --------------------------------------------------------------------------

  {
    "blocked",
    "-",
    "-"
  },
  {
    "explosion",
    "-",
    "-"
  },
  {
    "nut.breaking",
    "-",
    "-"
  },
  {
    "diamond.breaking",
    "-",
    "-"
  },
  {
    "acid_splash_left",
    "-",
    "-"
  },
  {
    "acid_splash_right",
    "-",
    "-"
  },
  {
    "amoeba.growing",
    "-",
    "-"
  },
  {
    "amoeba.shrinking",
    "-",
    "-"
  },
  {
    "expandable_wall.growing",
    "-",
    "-"
  },
  {
    "expandable_steelwall.growing",
    "-",
    "-"
  },
  {
    "flames",
    "-",
    "-"
  },
  {
    "player_is_leaving",
    "-",
    "-"
  },
  {
    "player_is_exploding_1",
    "-",
    "-"
  },
  {
    "player_is_exploding_2",
    "-",
    "-"
  },
  {
    "player_is_exploding_3",
    "-",
    "-"
  },
  {
    "player_is_exploding_4",
    "-",
    "-"
  },
  {
    "quicksand.filling",
    "quicksand",
    "-"
  },
  {
    "quicksand_fast.filling",
    "quicksand",
    "-"
  },
  {
    "magic_wall.filling",
    "-",
    "-"
  },
  {
    "bd_magic_wall.filling",
    "-",
    "-"
  },
  {
    "dc_magic_wall.filling",
    "-",
    "-"
  },
  {
    "element.snapping",
    "-",
    "-"
  },
  {
    "diagonal.shrinking",
    "-",
    "-"
  },
  {
    "diagonal.growing",
    "-",
    "-"
  },

  // --------------------------------------------------------------------------
  // dummy elements (never used as game elements, only used as graphics)
  // --------------------------------------------------------------------------

  {
    "steelwall_topleft",
    "-",
    "-"
  },
  {
    "steelwall_topright",
    "-",
    "-"
  },
  {
    "steelwall_bottomleft",
    "-",
    "-"
  },
  {
    "steelwall_bottomright",
    "-",
    "-"
  },
  {
    "steelwall_horizontal",
    "-",
    "-"
  },
  {
    "steelwall_vertical",
    "-",
    "-"
  },
  {
    "invisible_steelwall_topleft",
    "-",
    "-"
  },
  {
    "invisible_steelwall_topright",
    "-",
    "-"
  },
  {
    "invisible_steelwall_bottomleft",
    "-",
    "-"
  },
  {
    "invisible_steelwall_bottomright",
    "-",
    "-"
  },
  {
    "invisible_steelwall_horizontal",
    "-",
    "-"
  },
  {
    "invisible_steelwall_vertical",
    "-",
    "-"
  },
  {
    "dynabomb",
    "-",
    "-"
  },
  {
    "dynabomb.active",
    "-",
    "-"
  },
  {
    "dynabomb_player_1",
    "-",
    "-"
  },
  {
    "dynabomb_player_2",
    "-",
    "-"
  },
  {
    "dynabomb_player_3",
    "-",
    "-"
  },
  {
    "dynabomb_player_4",
    "-",
    "-"
  },
  {
    "shield_normal.active",
    "-",
    "-"
  },
  {
    "shield_deadly.active",
    "-",
    "-"
  },
  {
    "amoeba",
    "amoeba",
    "-"
  },
  {
    "mm_lightball_red",
    "-",
    "-"
  },
  {
    "mm_lightball_blue",
    "-",
    "-"
  },
  {
    "mm_lightball_yellow",
    "-",
    "-"
  },
  {
    "[default]",
    "default",
    "-"
  },
  {
    "[bd_default]",
    "bd_default",
    "-"
  },
  {
    "[bdx_default]",
    "bdx_default",
    "-"
  },
  {
    "[sp_default]",
    "sp_default",
    "-"
  },
  {
    "[sb_default]",
    "sb_default",
    "-"
  },
  {
    "[mm_default]",
    "mm_default",
    "-"
  },
  {
    "graphic_1",
    "graphic",
    "-"
  },
  {
    "graphic_2",
    "graphic",
    "-"
  },
  {
    "graphic_3",
    "graphic",
    "-"
  },
  {
    "graphic_4",
    "graphic",
    "-"
  },
  {
    "graphic_5",
    "graphic",
    "-"
  },
  {
    "graphic_6",
    "graphic",
    "-"
  },
  {
    "graphic_7",
    "graphic",
    "-"
  },
  {
    "graphic_8",
    "graphic",
    "-"
  },
  {
    "internal_clipboard_custom",
    "internal",
    "Empty custom element"
  },
  {
    "internal_clipboard_change",
    "internal",
    "Empty change page"
  },
  {
    "internal_clipboard_group",
    "internal",
    "Empty group element"
  },
  {
    "internal_dummy",
    "internal",
    "-"
  },
  {
    "internal_cascade_bd",
    "internal",
    "Show Boulder Dash elements"
  },
  {
    "internal_cascade_bd.active",
    "internal",
    "Hide Boulder Dash elements"
  },
  {
    "internal_cascade_bdx",
    "internal",
    "Show Boulder Dash native elements"
  },
  {
    "internal_cascade_bdx.active",
    "internal",
    "Hide Boulder Dash native elements"
  },
  {
    "internal_cascade_bdx_effects",
    "internal",
    "Show Boulder Dash effects elements"
  },
  {
    "internal_cascade_bdx_effects.active",
    "internal",
    "Hide Boulder Dash effects elements"
  },
  {
    "internal_cascade_bdx_scanned",
    "internal",
    "Show Boulder Dash scanned elements"
  },
  {
    "internal_cascade_bdx_scanned.active",
    "internal",
    "Hide Boulder Dash scanned elements"
  },
  {
    "internal_cascade_em",
    "internal",
    "Show Emerald Mine elements"
  },
  {
    "internal_cascade_em.active",
    "internal",
    "Hide Emerald Mine elements"
  },
  {
    "internal_cascade_emc",
    "internal",
    "Show Emerald Mine Club elements"
  },
  {
    "internal_cascade_emc.active",
    "internal",
    "Hide Emerald Mine Club elements"
  },
  {
    "internal_cascade_rnd",
    "internal",
    "Show Rocks'n'Diamonds elements"
  },
  {
    "internal_cascade_rnd.active",
    "internal",
    "Hide Rocks'n'Diamonds elements"
  },
  {
    "internal_cascade_sb",
    "internal",
    "Show Sokoban elements"
  },
  {
    "internal_cascade_sb.active",
    "internal",
    "Hide Sokoban elements"
  },
  {
    "internal_cascade_sp",
    "internal",
    "Show Supaplex elements"
  },
  {
    "internal_cascade_sp.active",
    "internal",
    "Hide Supaplex elements"
  },
  {
    "internal_cascade_dc",
    "internal",
    "Show Diamond Caves II elements"
  },
  {
    "internal_cascade_dc.active",
    "internal",
    "Hide Diamond Caves II elements"
  },
  {
    "internal_cascade_dx",
    "internal",
    "Show DX Boulderdash elements"
  },
  {
    "internal_cascade_dx.active",
    "internal",
    "Hide DX Boulderdash elements"
  },
  {
    "internal_cascade_mm",
    "internal",
    "Show Mirror Magic elements"
  },
  {
    "internal_cascade_mm.active",
    "internal",
    "Hide Mirror Magic elements"
  },
  {
    "internal_cascade_df",
    "internal",
    "Show Deflektor elements"
  },
  {
    "internal_cascade_df.active",
    "internal",
    "Hide Deflektor elements"
  },
  {
    "internal_cascade_chars",
    "internal",
    "Show text elements"
  },
  {
    "internal_cascade_chars.active",
    "internal",
    "Hide text elements"
  },
  {
    "internal_cascade_steel_chars",
    "internal",
    "Show steel text elements"
  },
  {
    "internal_cascade_steel_chars.active",
    "internal",
    "Hide steel text elements"
  },
  {
    "internal_cascade_ce",
    "internal",
    "Show custom elements"
  },
  {
    "internal_cascade_ce.active",
    "internal",
    "Hide custom elements"
  },
  {
    "internal_cascade_ge",
    "internal",
    "Show group elements"
  },
  {
    "internal_cascade_ge.active",
    "internal",
    "Hide group elements"
  },
  {
    "internal_cascade_es",
    "internal",
    "Show empty space elements"
  },
  {
    "internal_cascade_es.active",
    "internal",
    "Hide empty space elements"
  },
  {
    "internal_cascade_ref",
    "internal",
    "Show reference elements"
  },
  {
    "internal_cascade_ref.active",
    "internal",
    "Hide reference elements"
  },
  {
    "internal_cascade_user",
    "internal",
    "Show user defined elements"
  },
  {
    "internal_cascade_user.active",
    "internal",
    "Hide user defined elements"
  },
  {
    "internal_cascade_dynamic",
    "internal",
    "Show elements used in this level"
  },
  {
    "internal_cascade_dynamic.active",
    "internal",
    "Hide elements used in this level"
  },

  // keyword to stop parser: "ELEMENT_INFO_END" <-- do not change!

  {
    NULL,
    NULL,
    NULL
  }
};


// ----------------------------------------------------------------------------
// element action and direction definitions
// ----------------------------------------------------------------------------

struct ElementActionInfo element_action_info[NUM_ACTIONS + 1 + 1] =
{
  { ".[DEFAULT]",		ACTION_DEFAULT,			TRUE	},
  { ".waiting",			ACTION_WAITING,			TRUE	},
  { ".falling",			ACTION_FALLING,			TRUE	},
  { ".moving",			ACTION_MOVING,			TRUE	},
  { ".digging",			ACTION_DIGGING,			FALSE	},
  { ".snapping",		ACTION_SNAPPING,		FALSE	},
  { ".collecting",		ACTION_COLLECTING,		FALSE	},
  { ".dropping",		ACTION_DROPPING,		FALSE	},
  { ".pushing",			ACTION_PUSHING,			FALSE	},
  { ".walking",			ACTION_WALKING,			FALSE	},
  { ".passing",			ACTION_PASSING,			FALSE	},
  { ".impact",			ACTION_IMPACT,			FALSE	},
  { ".breaking",		ACTION_BREAKING,		FALSE	},
  { ".activating",		ACTION_ACTIVATING,		FALSE	},
  { ".deactivating",		ACTION_DEACTIVATING,		FALSE	},
  { ".opening",			ACTION_OPENING,			FALSE	},
  { ".closing",			ACTION_CLOSING,			FALSE	},
  { ".attacking",		ACTION_ATTACKING,		TRUE	},
  { ".growing",			ACTION_GROWING,			TRUE	},
  { ".shrinking",		ACTION_SHRINKING,		FALSE	},
  { ".active",			ACTION_ACTIVE,			TRUE	},
  { ".filling",			ACTION_FILLING,			FALSE	},
  { ".emptying",		ACTION_EMPTYING,		FALSE	},
  { ".changing",		ACTION_CHANGING,		FALSE	},
  { ".exploding",		ACTION_EXPLODING,		FALSE	},
  { ".boring",			ACTION_BORING,			FALSE	},
  { ".boring[1]",		ACTION_BORING_1,		FALSE	},
  { ".boring[2]",		ACTION_BORING_2,		FALSE	},
  { ".boring[3]",		ACTION_BORING_3,		FALSE	},
  { ".boring[4]",		ACTION_BORING_4,		FALSE	},
  { ".boring[5]",		ACTION_BORING_5,		FALSE	},
  { ".boring[6]",		ACTION_BORING_6,		FALSE	},
  { ".boring[7]",		ACTION_BORING_7,		FALSE	},
  { ".boring[8]",		ACTION_BORING_8,		FALSE	},
  { ".boring[9]",		ACTION_BORING_9,		FALSE	},
  { ".boring[10]",		ACTION_BORING_10,		FALSE	},
  { ".sleeping",		ACTION_SLEEPING,		FALSE	},
  { ".sleeping[1]",		ACTION_SLEEPING_1,		FALSE	},
  { ".sleeping[2]",		ACTION_SLEEPING_2,		FALSE	},
  { ".sleeping[3]",		ACTION_SLEEPING_3,		FALSE	},
  { ".awakening",		ACTION_AWAKENING,		FALSE	},
  { ".dying",			ACTION_DYING,			FALSE	},
  { ".turning",			ACTION_TURNING,			FALSE	},
  { ".turning_from_left",	ACTION_TURNING_FROM_LEFT,	FALSE	},
  { ".turning_from_right",	ACTION_TURNING_FROM_RIGHT,	FALSE	},
  { ".turning_from_up",		ACTION_TURNING_FROM_UP,		FALSE	},
  { ".turning_from_down",	ACTION_TURNING_FROM_DOWN,	FALSE	},
  { ".smashed_by_rock",		ACTION_SMASHED_BY_ROCK,		FALSE	},
  { ".smashed_by_spring",	ACTION_SMASHED_BY_SPRING,	FALSE	},
  { ".eating",			ACTION_EATING,			FALSE	},
  { ".twinkling",		ACTION_TWINKLING,		FALSE	},
  { ".splashing",		ACTION_SPLASHING,		FALSE	},
  { ".hitting",			ACTION_HITTING,			FALSE	},
  { ".flying",			ACTION_FLYING,			FALSE	},
  { ".page[1]",			ACTION_PAGE_1,			FALSE	},
  { ".page[2]",			ACTION_PAGE_2,			FALSE	},
  { ".page[3]",			ACTION_PAGE_3,			FALSE	},
  { ".page[4]",			ACTION_PAGE_4,			FALSE	},
  { ".page[5]",			ACTION_PAGE_5,			FALSE	},
  { ".page[6]",			ACTION_PAGE_6,			FALSE	},
  { ".page[7]",			ACTION_PAGE_7,			FALSE	},
  { ".page[8]",			ACTION_PAGE_8,			FALSE	},
  { ".page[9]",			ACTION_PAGE_9,			FALSE	},
  { ".page[10]",		ACTION_PAGE_10,			FALSE	},
  { ".page[11]",		ACTION_PAGE_11,			FALSE	},
  { ".page[12]",		ACTION_PAGE_12,			FALSE	},
  { ".page[13]",		ACTION_PAGE_13,			FALSE	},
  { ".page[14]",		ACTION_PAGE_14,			FALSE	},
  { ".page[15]",		ACTION_PAGE_15,			FALSE	},
  { ".page[16]",		ACTION_PAGE_16,			FALSE	},
  { ".page[17]",		ACTION_PAGE_17,			FALSE	},
  { ".page[18]",		ACTION_PAGE_18,			FALSE	},
  { ".page[19]",		ACTION_PAGE_19,			FALSE	},
  { ".page[20]",		ACTION_PAGE_20,			FALSE	},
  { ".page[21]",		ACTION_PAGE_21,			FALSE	},
  { ".page[22]",		ACTION_PAGE_22,			FALSE	},
  { ".page[23]",		ACTION_PAGE_23,			FALSE	},
  { ".page[24]",		ACTION_PAGE_24,			FALSE	},
  { ".page[25]",		ACTION_PAGE_25,			FALSE	},
  { ".page[26]",		ACTION_PAGE_26,			FALSE	},
  { ".page[27]",		ACTION_PAGE_27,			FALSE	},
  { ".page[28]",		ACTION_PAGE_28,			FALSE	},
  { ".page[29]",		ACTION_PAGE_29,			FALSE	},
  { ".page[30]",		ACTION_PAGE_30,			FALSE	},
  { ".page[31]",		ACTION_PAGE_31,			FALSE	},
  { ".page[32]",		ACTION_PAGE_32,			FALSE	},
  { ".part_1",			ACTION_PART_1,			FALSE	},
  { ".part_2",			ACTION_PART_2,			FALSE	},
  { ".part_3",			ACTION_PART_3,			FALSE	},
  { ".part_4",			ACTION_PART_4,			FALSE	},
  { ".part_5",			ACTION_PART_5,			FALSE	},
  { ".part_6",			ACTION_PART_6,			FALSE	},
  { ".part_7",			ACTION_PART_7,			FALSE	},
  { ".part_8",			ACTION_PART_8,			FALSE	},
  { ".part_9",			ACTION_PART_9,			FALSE	},
  { ".part_10",			ACTION_PART_10,			FALSE	},
  { ".part_11",			ACTION_PART_11,			FALSE	},
  { ".part_12",			ACTION_PART_12,			FALSE	},
  { ".part_13",			ACTION_PART_13,			FALSE	},
  { ".part_14",			ACTION_PART_14,			FALSE	},
  { ".part_15",			ACTION_PART_15,			FALSE	},
  { ".part_16",			ACTION_PART_16,			FALSE	},
  { ".part_17",			ACTION_PART_17,			FALSE	},
  { ".part_18",			ACTION_PART_18,			FALSE	},
  { ".part_19",			ACTION_PART_19,			FALSE	},
  { ".part_20",			ACTION_PART_20,			FALSE	},
  { ".part_21",			ACTION_PART_21,			FALSE	},
  { ".part_22",			ACTION_PART_22,			FALSE	},
  { ".part_23",			ACTION_PART_23,			FALSE	},
  { ".part_24",			ACTION_PART_24,			FALSE	},
  { ".part_25",			ACTION_PART_25,			FALSE	},
  { ".part_26",			ACTION_PART_26,			FALSE	},
  { ".part_27",			ACTION_PART_27,			FALSE	},
  { ".part_28",			ACTION_PART_28,			FALSE	},
  { ".part_29",			ACTION_PART_29,			FALSE	},
  { ".part_30",			ACTION_PART_30,			FALSE	},
  { ".part_31",			ACTION_PART_31,			FALSE	},
  { ".part_32",			ACTION_PART_32,			FALSE	},
  { ".other",			ACTION_OTHER,			FALSE	},

  // empty suffix always matches -- check as last entry in InitSoundInfo()
  { "",				ACTION_DEFAULT,			TRUE	},

  { NULL,			0,				0	}
};

struct ElementDirectionInfo element_direction_info[NUM_DIRECTIONS_FULL + 1] =
{
  { ".left",			MV_BIT_LEFT				},
  { ".right",			MV_BIT_RIGHT				},
  { ".up",			MV_BIT_UP				},
  { ".down",			MV_BIT_DOWN				},
  { ".upleft",			MV_BIT_UP				},
  { ".upright",			MV_BIT_RIGHT				},
  { ".downleft",		MV_BIT_LEFT				},
  { ".downright",		MV_BIT_DOWN				},

  { NULL,			0					}
};

struct SpecialSuffixInfo special_suffix_info[NUM_SPECIAL_GFX_ARGS + 1 + 1] =
{
  { ".[DEFAULT]",		GFX_SPECIAL_ARG_DEFAULT,		},
  { ".LOADING_INITIAL",		GFX_SPECIAL_ARG_LOADING_INITIAL,	},
  { ".LOADING",			GFX_SPECIAL_ARG_LOADING,		},
  { ".TITLE_INITIAL",		GFX_SPECIAL_ARG_TITLE_INITIAL,		},
  { ".TITLE_INITIAL_1",		GFX_SPECIAL_ARG_TITLE_INITIAL_1,	},
  { ".TITLE_INITIAL_2",		GFX_SPECIAL_ARG_TITLE_INITIAL_2,	},
  { ".TITLE_INITIAL_3",		GFX_SPECIAL_ARG_TITLE_INITIAL_3,	},
  { ".TITLE_INITIAL_4",		GFX_SPECIAL_ARG_TITLE_INITIAL_4,	},
  { ".TITLE_INITIAL_5",		GFX_SPECIAL_ARG_TITLE_INITIAL_5,	},
  { ".TITLE",			GFX_SPECIAL_ARG_TITLE,			},
  { ".TITLE_1",			GFX_SPECIAL_ARG_TITLE_1,		},
  { ".TITLE_2",			GFX_SPECIAL_ARG_TITLE_2,		},
  { ".TITLE_3",			GFX_SPECIAL_ARG_TITLE_3,		},
  { ".TITLE_4",			GFX_SPECIAL_ARG_TITLE_4,		},
  { ".TITLE_5",			GFX_SPECIAL_ARG_TITLE_5,		},
  { ".MAIN",			GFX_SPECIAL_ARG_MAIN,			},
  { ".NAMES",			GFX_SPECIAL_ARG_NAMES,			},
  { ".LEVELS",			GFX_SPECIAL_ARG_LEVELS			},
  { ".LEVELNR",			GFX_SPECIAL_ARG_LEVELNR			},
  { ".SCORES",			GFX_SPECIAL_ARG_SCORES,			},
  { ".SCOREINFO",		GFX_SPECIAL_ARG_SCOREINFO,		},
  { ".EDITOR",			GFX_SPECIAL_ARG_EDITOR,			},
  { ".INFO",			GFX_SPECIAL_ARG_INFO,			},
  { ".SETUP",			GFX_SPECIAL_ARG_SETUP,			},
  { ".PLAYING",			GFX_SPECIAL_ARG_PLAYING,		},
  { ".DOOR",			GFX_SPECIAL_ARG_DOOR,			},
  { ".TAPE",			GFX_SPECIAL_ARG_TAPE,			},
  { ".PANEL",			GFX_SPECIAL_ARG_PANEL,			},
  { ".PREVIEW",			GFX_SPECIAL_ARG_PREVIEW,		},
  { ".CRUMBLED",		GFX_SPECIAL_ARG_CRUMBLED,		},
  { ".MAINONLY",		GFX_SPECIAL_ARG_MAINONLY,		},
  { ".NAMESONLY",		GFX_SPECIAL_ARG_NAMESONLY,		},
  { ".SCORESONLY",		GFX_SPECIAL_ARG_SCORESONLY,		},
  { ".TYPENAME",		GFX_SPECIAL_ARG_TYPENAME,		},
  { ".TYPENAMES",		GFX_SPECIAL_ARG_TYPENAMES,		},
  { ".SUBMENU",			GFX_SPECIAL_ARG_SUBMENU,		},
  { ".MENU",			GFX_SPECIAL_ARG_MENU,			},
  { ".TOONS",			GFX_SPECIAL_ARG_TOONS,			},
  { ".SCORESOLD",		GFX_SPECIAL_ARG_SCORESOLD,		},
  { ".SCORESNEW",		GFX_SPECIAL_ARG_SCORESNEW,		},
  { ".NO_TITLE",		GFX_SPECIAL_ARG_NO_TITLE,		},
  { ".FADING",			GFX_SPECIAL_ARG_FADING,			},
  { ".QUIT",			GFX_SPECIAL_ARG_QUIT,			},

  // empty suffix always matches -- check as last entry in InitMusicInfo()
  { "",				GFX_SPECIAL_ARG_DEFAULT,		},

  { NULL,			0,					}
};

#include "conf_var.c"	// include auto-generated data structure definitions


// ----------------------------------------------------------------------------
// font definitions
// ----------------------------------------------------------------------------

// Important: When one entry is a prefix of another entry, the longer entry
// must come first, because the dynamic configuration does prefix matching!
// (These definitions must match the corresponding definitions in "main.h"!)

struct FontInfo font_info[NUM_FONTS + 1] =
{
  { "font.initial_1"		},
  { "font.initial_2"		},
  { "font.initial_3"		},
  { "font.initial_4"		},
  { "font.title_1"		},
  { "font.title_2"		},
  { "font.title_story"		},
  { "font.footer"		},
  { "font.menu_1.active"	},
  { "font.menu_2.active"	},
  { "font.menu_1"		},
  { "font.menu_2"		},
  { "font.text_1.active"	},
  { "font.text_2.active"	},
  { "font.text_3.active"	},
  { "font.text_4.active"	},
  { "font.text_1"		},
  { "font.text_2"		},
  { "font.text_3"		},
  { "font.text_4"		},
  { "font.envelope_1"		},
  { "font.envelope_2"		},
  { "font.envelope_3"		},
  { "font.envelope_4"		},
  { "font.request_narrow"	},
  { "font.request"		},
  { "font.input_1.active"	},
  { "font.input_2.active"	},
  { "font.input_1"		},
  { "font.input_2"		},
  { "font.option_off_narrow"	},
  { "font.option_off"		},
  { "font.option_on_narrow"	},
  { "font.option_on"		},
  { "font.value_1"		},
  { "font.value_2"		},
  { "font.value_old_narrow"	},
  { "font.value_old"		},
  { "font.value_narrow"		},
  { "font.level_number.active"	},
  { "font.level_number"		},
  { "font.tape_recorder"	},
  { "font.game_info"		},
  { "font.info.elements"	},
  { "font.info.levelset"	},
  { "font.info.level"		},
  { "font.info.story"		},
  { "font.main.network_players"	},

  { NULL			}
};

struct GlobalAnimInfo global_anim_info[NUM_GLOBAL_ANIM_TOKENS + 1];

// this contains predefined structure elements to init "global_anim_info"
struct GlobalAnimNameInfo global_anim_name_info[NUM_GLOBAL_ANIM_TOKENS + 1] =
{
  // (real) graphic definitions used to define animation graphics
  { "gfx.global.anim_1",	},
  { "gfx.global.anim_2",	},
  { "gfx.global.anim_3",	},
  { "gfx.global.anim_4",	},
  { "gfx.global.anim_5",	},
  { "gfx.global.anim_6",	},
  { "gfx.global.anim_7",	},
  { "gfx.global.anim_8",	},
  { "gfx.global.anim_9",	},
  { "gfx.global.anim_10",	},
  { "gfx.global.anim_11",	},
  { "gfx.global.anim_12",	},
  { "gfx.global.anim_13",	},
  { "gfx.global.anim_14",	},
  { "gfx.global.anim_15",	},
  { "gfx.global.anim_16",	},
  { "gfx.global.anim_17",	},
  { "gfx.global.anim_18",	},
  { "gfx.global.anim_19",	},
  { "gfx.global.anim_20",	},
  { "gfx.global.anim_21",	},
  { "gfx.global.anim_22",	},
  { "gfx.global.anim_23",	},
  { "gfx.global.anim_24",	},
  { "gfx.global.anim_25",	},
  { "gfx.global.anim_26",	},
  { "gfx.global.anim_27",	},
  { "gfx.global.anim_28",	},
  { "gfx.global.anim_29",	},
  { "gfx.global.anim_30",	},
  { "gfx.global.anim_31",	},
  { "gfx.global.anim_32",	},

  // (dummy) graphic definitions used to define animation controls
  { "global.anim_1",		},
  { "global.anim_2",		},
  { "global.anim_3",		},
  { "global.anim_4",		},
  { "global.anim_5",		},
  { "global.anim_6",		},
  { "global.anim_7",		},
  { "global.anim_8",		},
  { "global.anim_9",		},
  { "global.anim_10",		},
  { "global.anim_11",		},
  { "global.anim_12",		},
  { "global.anim_13",		},
  { "global.anim_14",		},
  { "global.anim_15",		},
  { "global.anim_16",		},
  { "global.anim_17",		},
  { "global.anim_18",		},
  { "global.anim_19",		},
  { "global.anim_20",		},
  { "global.anim_21",		},
  { "global.anim_22",		},
  { "global.anim_23",		},
  { "global.anim_24",		},
  { "global.anim_25",		},
  { "global.anim_26",		},
  { "global.anim_27",		},
  { "global.anim_28",		},
  { "global.anim_29",		},
  { "global.anim_30",		},
  { "global.anim_31",		},
  { "global.anim_32",		},

  { NULL			}
};

struct GlobalAnimEventInfo global_anim_event_info =
{
  NULL, 0
};


// ----------------------------------------------------------------------------
// music token prefix definitions
// ----------------------------------------------------------------------------

struct MusicPrefixInfo music_prefix_info[NUM_MUSIC_PREFIXES + 1] =
{
  { "background",		TRUE	},

  { NULL,			0	}
};


// ============================================================================
// main()
// ============================================================================

static void print_usage(void)
{
  Print("\n"
	"Usage: %s [OPTION]... [HOSTNAME [PORT]]\n"
	"\n"
	"Options:\n"
	"  -b, --basepath DIRECTORY         alternative base DIRECTORY\n"
	"  -l, --levels DIRECTORY           alternative levels DIRECTORY\n"
	"  -g, --graphics DIRECTORY         alternative graphics DIRECTORY\n"
	"  -s, --sounds DIRECTORY           alternative sounds DIRECTORY\n"
	"  -m, --music DIRECTORY            alternative music DIRECTORY\n"
	"      --drop-file FILE             drop FILE into program window\n"
	"      --display NR                 open program window on display NR\n"
	"      --mytapes                    use private tapes for tape tests\n"
	"  -n, --network                    network multiplayer game\n"
	"      --serveronly                 only start network server\n"
	"  -v, --verbose                    verbose mode\n"
	"  -V, --version                    show program version\n"
	"      --debug[=MODE]               show (and limit) debug output\n"
	"  -e, --execute COMMAND            execute batch COMMAND\n"
	"\n"
	"Valid commands for '--execute' option:\n"
	"  \"print graphicsinfo.conf\"        print default graphics config\n"
	"  \"print soundsinfo.conf\"          print default sounds config\n"
	"  \"print musicinfo.conf\"           print default music config\n"
	"  \"print editorsetup.conf\"         print default editor config\n"
	"  \"print helpanim.conf\"            print default helpanim config\n"
	"  \"print helptext.conf\"            print default helptext config\n"
	"  \"dump levelset FILE|LEVELDIR\"    dump levelset info for LEVELDIR\n"
	"  \"dump level FILE\"                dump level data from FILE\n"
	"  \"dump tape FILE\"                 dump tape data from FILE\n"
	"  \"autoplay LEVELDIR [NR ...]\"     play level tapes for LEVELDIR\n"
	"  \"autoffwd LEVELDIR [NR ...]\"     ffwd level tapes for LEVELDIR\n"
	"  \"autowarp LEVELDIR [NR ...]\"     warp level tapes for LEVELDIR\n"
	"  \"autotest LEVELDIR [NR ...]\"     test level tapes for LEVELDIR\n"
	"  \"autofix LEVELDIR [NR ...]\"      test and fix tapes for LEVELDIR\n"
	"  \"patch tapes MODE LEVELDIR [NR]\" patch level tapes for LEVELDIR\n"
	"  \"convert LEVELDIR [NR]\"          convert levels in LEVELDIR\n"
	"  \"create sketch images DIRECTORY\" write BMP images to DIRECTORY\n"
	"  \"create collect image DIRECTORY\" write BMP image to DIRECTORY\n"
	"\n",
	program.command_basename);
}

static void print_version(void)
{
  Print("%s", getProgramInitString());

  if (!strEqual(getProgramVersionString(), getProgramRealVersionString()))
  {
    Print(" (%s %d.%d.%d.%d%s)",
	  PROGRAM_TITLE_STRING,
	  PROGRAM_VERSION_SUPER,
	  PROGRAM_VERSION_MAJOR,
	  PROGRAM_VERSION_MINOR,
	  PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_EXTRA);
  }

  Print("\n");

  if (options.debug)
  {
    SDL_version sdl_version;

    SDL_VERSION(&sdl_version);
    Print("- SDL %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);

    SDL_IMAGE_VERSION(&sdl_version);
    Print("- SDL_image %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);

    SDL_MIXER_VERSION(&sdl_version);
    Print("- SDL_mixer %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);

    SDL_NET_VERSION(&sdl_version);
    Print("- SDL_net %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);
  }
}

static void InitProgramConfig(char *command_filename)
{
  char *program_title = PROGRAM_TITLE_STRING;
  char *program_icon_file = PROGRAM_ICON_FILENAME;
  char *program_version = getProgramRealVersionString();
  char *program_basename = getBaseNameNoSuffix(command_filename);
  char *config_filename = getProgramConfigFilename(command_filename);
  char *userdata_subdir;

  // read default program config, if existing
  if (fileExists(config_filename))
    LoadSetupFromFilename(config_filename);

  // set program title from potentially redefined program title
  if (setup.internal.program_title != NULL &&
      strlen(setup.internal.program_title) > 0)
    program_title = getStringCopy(setup.internal.program_title);

  // set program version from potentially redefined program version
  if (setup.internal.program_version != NULL &&
      strlen(setup.internal.program_version) > 0)
    program_version = getStringCopy(setup.internal.program_version);

  // set program icon file from potentially redefined program icon file
  if (setup.internal.program_icon_file != NULL &&
      strlen(setup.internal.program_icon_file) > 0)
    program_icon_file = getStringCopy(setup.internal.program_icon_file);

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_MAC) || defined(PLATFORM_EMSCRIPTEN)
  userdata_subdir = program_title;
#elif defined(PLATFORM_UNIX)
  userdata_subdir = getStringCat2(".", program_basename);
#else
  userdata_subdir = USERDATA_DIRECTORY_OTHER;
#endif

  // set default window size (only relevant on program startup)
  if (setup.internal.default_window_width  != 0 &&
      setup.internal.default_window_height != 0)
  {
    WIN_XSIZE = setup.internal.default_window_width;
    WIN_YSIZE = setup.internal.default_window_height;
  }

  InitProgramInfo(command_filename,
		  config_filename,
		  userdata_subdir,
		  program_basename,
		  program_title,
		  program_icon_file,
		  COOKIE_PREFIX,
		  program_version,
		  GAME_VERSION_ACTUAL);
}

int main(int argc, char *argv[])
{
  InitProgramConfig(argv[0]);

  InitWindowTitleFunction(getWindowTitleString);
  InitExitMessageFunction(DisplayExitMessage);
  InitExitFunction(CloseAllAndExit);
  InitPlatformDependentStuff();

  GetOptions(argc, argv, print_usage, print_version);
  OpenAll();

  EventLoop();
  CloseAllAndExit(0);

  return 0;	// to keep compilers happy
}
