/*
 * Copyright (c) 2007, 2008, 2009, Czirkos Zoltan <cirix@fw.hu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef BD_CAVE_H
#define BD_CAVE_H

#include "bd_elements.h"
#include "bd_colors.h"
#include "bd_random.h"


// ============================================================================
// BIG STRUCT HANDLING
// ============================================================================

// possible types handled
typedef enum _gd_type
{
  // not real types, only used by editor to build ui
  GD_TAB,
  GD_LABEL,

  // gd types
  GD_TYPE_STRING,		// static string, fixed array of characters
  GD_TYPE_LONGSTRING,		// long string which has its own notebook page in the editor
  GD_TYPE_INT,
  GD_TYPE_RATIO,
  GD_TYPE_ELEMENT,
  GD_TYPE_BOOLEAN,
  GD_TYPE_PROBABILITY,		// probabilities are stored in parts per million,
				// ie. *1E6, converted to int.
  GD_TYPE_COLOR,
  GD_TYPE_EFFECT,
  GD_TYPE_DIRECTION,
  GD_TYPE_SCHEDULING,
} GdType;

enum _gd_property_flags
{
  GD_ALWAYS_SAVE           = 1 << 0,
  GD_DONT_SAVE             = 1 << 1,
  GD_DONT_SHOW_IN_EDITOR   = 1 << 2,
  GD_SHOW_LEVEL_LABEL      = 1 << 3,
  GD_COMPATIBILITY_SETTING = 1 << 4,
};

typedef struct _gd_struct_descriptor
{
  char *identifier;		// bdcff identifier
  GdType type;			// data type
  int flags;			// flags for bdcff saving/loading
  char *name;			// name in editor
  int offset;			// byte offset in a GdCave structure. use the CAVE_OFFSET macro
  int count;			// size of array; usually 1, for non-arrays.
  char *tooltip;		// tooltip text in editor
  int min, max;			// integers have minimum and maximum
} GdStructDescriptor;

typedef struct _gd_property_default
{
  int offset;			// data offset (bytes) in a cave structure
  int defval;			// default value, converted to int. if type is a float, *1000000

  int property_index;		// index in gd_cave_properties; created at runtime
} GdPropertyDefault;


void gd_struct_set_defaults_from_array(void *str, const GdStructDescriptor *properties, GdPropertyDefault *defaults);

// these define the number of the cells in the png file
#define GD_NUM_OF_CELLS_X	8
#define GD_NUM_OF_CELLS_Y	51

// +80: placeholder for cells which are rendered by the game;
// for example diamond + arrow = falling diamond
#define GD_NUM_OF_CELLS		(GD_NUM_OF_CELLS_X * GD_NUM_OF_CELLS_Y + 80)

// maximum replay size (maximum seconds x game cycles per second)
#define MAX_REPLAY_LEN		(20000 * FRAMES_PER_SECOND / 8)

extern const GdColor gd_flash_color;
extern const GdColor gd_select_color;

enum _element_property_enum
{
  E_P_SCANNED,                  // is a scanned element

  E_P_SLOPED_LEFT,              // stones and diamonds roll down to left on this
  E_P_SLOPED_RIGHT,             // stones and diamonds roll down to right on this
  E_P_SLOPED_UP,
  E_P_SLOPED_DOWN,
  E_P_BLADDER_SLOPED,           // element act sloped also for the bladder

  E_P_AMOEBA_CONSUMES,          // amoeba can eat this
  E_P_DIRT,                     // it is dirt, or something similar (dirt2 or sloped dirt)
  E_P_BLOWS_UP_FLIES,           // flies blow up, if they touch this
  E_P_EXPLODES_BY_HIT,          // explodes if hit by a stone

  E_P_EXPLOSION,                // set for every stage of every explosion.
  E_P_EXPLOSION_FIRST_STAGE,    // set for first stage of every explosion.
				// helps slower/faster explosions changing

  E_P_NON_EXPLODABLE,           // selfexplaining
  E_P_CCW,                      // this creature has a default counterclockwise
				// rotation (for example, o_fire_1)
  E_P_CAN_BE_HAMMERED,          // can be broken by pneumatic hammer
  E_P_VISUAL_EFFECT,            // if the element can use a visual effect.
				// used to check consistency of the code
  E_P_PLAYER,                   // easier to find out if it is a player element
  E_P_PLAYER_PUSHING,           // player pushing some element
  E_P_PLAYER_STIRRING,          // player stirring the pot
  E_P_MOVED_BY_CONVEYOR_TOP,    // can be moved by conveyor belt
  E_P_MOVED_BY_CONVEYOR_BOTTOM, // can be moved UNDER the conveyor belt

  E_P_WALKABLE,                 // can be walked
  E_P_DIGGABLE,                 // can be digged
  E_P_COLLECTIBLE,              // can be collected
  E_P_PUSHABLE,                 // can be pushed
  E_P_CAN_MOVE,                 // can move
  E_P_CAN_FALL,                 // can fall
  E_P_CAN_GROW,                 // can grow
  E_P_CAN_DIG,                  // can dig
  E_P_FALLING,                  // falling
  E_P_GROWING,                  // growing (element birth)
};

// properties
#define P_SCANNED			(1 << E_P_SCANNED)

#define P_SLOPED_LEFT			(1 << E_P_SLOPED_LEFT)
#define P_SLOPED_RIGHT			(1 << E_P_SLOPED_RIGHT)
#define P_SLOPED_UP			(1 << E_P_SLOPED_UP)
#define P_SLOPED_DOWN			(1 << E_P_SLOPED_DOWN)

// flag to say "any direction"
#define P_SLOPED			(P_SLOPED_LEFT	|		\
					 P_SLOPED_RIGHT	|		\
					 P_SLOPED_UP	|		\
					 P_SLOPED_DOWN)

#define P_BLADDER_SLOPED		(1 << E_P_BLADDER_SLOPED)

#define P_AMOEBA_CONSUMES		(1 << E_P_AMOEBA_CONSUMES)
#define P_DIRT				(1 << E_P_DIRT)
#define P_BLOWS_UP_FLIES		(1 << E_P_BLOWS_UP_FLIES)

#define P_EXPLODES_BY_HIT		(1 << E_P_EXPLODES_BY_HIT)
#define P_EXPLOSION			(1 << E_P_EXPLOSION)
#define P_EXPLOSION_FIRST_STAGE		(1 << E_P_EXPLOSION_FIRST_STAGE)

#define P_NON_EXPLODABLE		(1 << E_P_NON_EXPLODABLE)
#define P_CCW				(1 << E_P_CCW)
#define P_CAN_BE_HAMMERED		(1 << E_P_CAN_BE_HAMMERED)
#define P_VISUAL_EFFECT			(1 << E_P_VISUAL_EFFECT)
#define P_PLAYER			(1 << E_P_PLAYER)
#define P_PLAYER_PUSHING		(1 << E_P_PLAYER_PUSHING)
#define P_PLAYER_STIRRING		(1 << E_P_PLAYER_STIRRING)
#define P_MOVED_BY_CONVEYOR_TOP		(1 << E_P_MOVED_BY_CONVEYOR_TOP)
#define P_MOVED_BY_CONVEYOR_BOTTOM	(1 << E_P_MOVED_BY_CONVEYOR_BOTTOM)

#define P_WALKABLE			(1 << E_P_WALKABLE)
#define P_DIGGABLE			(1 << E_P_DIGGABLE)
#define P_COLLECTIBLE			(1 << E_P_COLLECTIBLE)
#define P_PUSHABLE			(1 << E_P_PUSHABLE)
#define P_CAN_MOVE			(1 << E_P_CAN_MOVE)
#define P_CAN_FALL			(1 << E_P_CAN_FALL)
#define P_CAN_GROW			(1 << E_P_CAN_GROW)
#define P_CAN_DIG			(1 << E_P_CAN_DIG)
#define P_FALLING			(1 << E_P_FALLING)
#define P_GROWING			(1 << E_P_GROWING)

// These are states of the magic wall.
typedef enum _magic_wall_state
{
  GD_MW_DORMANT,                // Starting with this.
  GD_MW_ACTIVE,                 // Boulder or diamond dropped into.
  GD_MW_EXPIRED                 // Turned off after magic_wall_milling_time.
} GdMagicWallState;

// These are states of Player.
typedef enum _player_state
{
  GD_PL_NOT_YET,                // Not yet living. Beginning of cave time.
  GD_PL_LIVING,                 // Ok.
  GD_PL_TIMEOUT,                // Time is up
  GD_PL_DIED,                   // Died.
  GD_PL_EXITED                  // Exited the cave, proceed to next one
} GdPlayerState;

// States of amoeba
typedef enum _amoeba_state
{
  GD_AM_SLEEPING,               // sleeping - not yet let out.
  GD_AM_AWAKE,                  // living, growing
  GD_AM_TOO_BIG,                // grown too big, will convert to stones
  GD_AM_ENCLOSED,               // enclosed, will convert to diamonds
} GdAmoebaState;

typedef enum _direction
{
  // not moving
  GD_MV_STILL		= 0,
  GD_MV_THIS		= 0,

  // directions
  GD_MV_UP		= 1,
  GD_MV_UP_RIGHT	= 2,
  GD_MV_RIGHT		= 3,
  GD_MV_DOWN_RIGHT	= 4,
  GD_MV_DOWN		= 5,
  GD_MV_DOWN_LEFT	= 6,
  GD_MV_LEFT		= 7,
  GD_MV_UP_LEFT		= 8,

  // to be able to type GD_MV_TWICE + GD_MV_DOWN, for example
  GD_MV_TWICE		= 8,

  // directions * 2
  GD_MV_UP_2		= 9,
  GD_MV_UP_RIGHT_2	= 10,
  GD_MV_RIGHT_2		= 11,
  GD_MV_DOWN_RIGHT_2	= 12,
  GD_MV_DOWN_2		= 13,
  GD_MV_DOWN_LEFT_2	= 14,
  GD_MV_LEFT_2		= 15,
  GD_MV_UP_LEFT_2	= 16,

  GD_MV_MAX,
} GdDirection;

enum
{
  GD_REPLAY_MOVE_MASK	 = 0x0f,
  GD_REPLAY_FIRE_MASK	 = 0x10,
  GD_REPLAY_SUICIDE_MASK = 0x20,
};


// ELEMENTS DESCRIPTION
typedef struct _element_property
{
  GdElement element;            // element number. for example O_DIRT
                                // In the array, should be equal to the index of the array item.
  GdElement pair;               // the scanned/not scanned pair
  char *name;                   // name in editor, for example "Dirt". some have
			        // different names than their real engine meaning!
  unsigned int properties;      // engine properties, like P_SLOPED or P_EXPLODES
  char *filename;               // name in bdcff file, like "DIRT"
  char character;               // character representation in bdcff file, like '.'
  int image;                    // image in editor (index in cells.png)
  int image_simple;             // image in editor (index in cells.png) (simple view / combo box)
  int image_game;               // image for game. negative if animated
  int ckdelay;                  // ckdelay ratio - how much time required for a c64 to
			        // process this element - in microseconds.

  char *lowercase_name;         // lowercase of translated name. for editor;
                                // generated inside the game.
  char character_new;           // character given automatically for elements which
			        // don't have one defined in original bdcff description
} GdElementProperty;


typedef char GdString[MAX_LINE_LEN];

typedef struct _highscore
{
  GdString name;
  int score;
} GdHighScore;

typedef struct _replay_movements
{
  unsigned char data[MAX_REPLAY_LEN];
  unsigned int len;
} GdReplayMovements;

typedef struct _replay_randoms
{
  unsigned int data[MAX_REPLAY_LEN];
  unsigned int len;
} GdReplayRandoms;

// maximum seed value for the cave random generator. should be smaller than a signed int.
#define GD_CAVE_SEED_MAX	(1 << 30)

typedef struct _gd_cave_replay
{
  int level;                    // replay for level n
  unsigned int seed;            // seed the cave is to be rendered with
  boolean saved;                // also store it in the saved bdcff
  GdString recorded_with;       // recorded with - application name and version

  GdString player_name;         // who played this
  GdString date;                // when played
  char *comment;                // some comments from the player

  int score;                    // score collected
  int duration;                 // number of seconds played
  boolean success;              // successful playing of cave?
  unsigned int checksum;        // checksum of the rendered cave.

  boolean wrong_checksum;
  GdReplayMovements *movements;
  GdReplayRandoms *randoms;
  int current_playing_pos;
} GdReplay;

typedef enum _gd_scheduling
{
  GD_SCHEDULING_MILLISECONDS,	// Perfect scheduling, milliseconds-based
  GD_SCHEDULING_BD1,		// C64 BD1
  GD_SCHEDULING_BD2,		// C64 BD2
  GD_SCHEDULING_PLCK,		// C64 construction kit
  GD_SCHEDULING_CRDR,		// C64 crazy dream
  GD_SCHEDULING_BD1_ATARI,	// Atari BD1
  GD_SCHEDULING_BD2_PLCK_ATARI,	// Atari BD2 and construction kit

  GD_SCHEDULING_MAX		// Number of scheduling types
} GdScheduling;

typedef struct _gd_c64_random_generator
{
  int rand_seed_1, rand_seed_2;
} GdC64RandomGenerator;

// ----------------------------------------------------------------------------
// Structure holding all data belonging to a cave.
// ----------------------------------------------------------------------------

#define GD_HIGHSCORE_NUM		20
#define GD_PLAYER_MEM_SIZE		16

#define GD_PLAYER_GONE_LIMIT_STANDARD	1
#define GD_PLAYER_GONE_LIMIT_EXTENDED	15

typedef struct _gd_cave
{
  // Defined by the editor. public data :)
  GdString name;                        // name of cave
  GdString description;                 // some words about the cave
  GdString author;                      // author
  GdString difficulty;                  // difficulty of the game, for info purposes
  GdString www;                         // link to author's webpage
  GdString date;                        // date of creation
  char *story;                          // story for the cave - will be shown when cave is played.
  char *remark;                         // some note

  GdString charset;                     // these are not used by gdash
  GdString fontset;

  // and this one the highscores
  GdHighScore highscore[GD_HIGHSCORE_NUM];

  HashTable *tags;                      // stores read-but-not-understood strings from bdcff,
                                        // so we can save them later.

  GdElement **map;                      // pointer to data for map, non-null if has a map
  List *objects;
  List *replays;

  boolean intermission;                 // is this cave an intermission?
  boolean intermission_instantlife;     // one life extra, if the intermission is reached
  boolean intermission_rewardlife;      // one life extra, if the intermission is finished
  boolean selectable;                   // is this selectable as an initial cave for a game?
  boolean diagonal_movements;           // are diagonal movements allowed?
  GdElement snap_element;               // snapping (press fire+move) usually leaves space behind,
                                        // but can be other
  boolean short_explosions;             // in >= 1stb, diamond/creature explosions were of 5 stages

  boolean use_krissz_engine;            // for game engine compatibility with Krissz engine
  boolean new_krissz_engine;            // for game engine compatibility with Krissz engine (new)

  GdScheduling scheduling;              // scheduling type; see above
  boolean pal_timing;                   // use faster seconds
  boolean no_time;                      // use no time at all for this level

  boolean active_is_first_found;        // active player is the uppermost.
  boolean lineshift;                    // true is line shifting emulation,
                                        // false is perfect borders emulation
  boolean border_scan_first_and_last;   // if true, scans the first and last line of the border.
                                        // false for plck
  boolean wraparound_objects;           // if this is true, object drawing (cave rendering)
                                        // will wraparound as well.
  boolean open_borders_horizontal;      // cave is open for crossing borders horizontally
  boolean open_borders_vertical;        // cave is open for crossing borders vertically
  boolean infinite_scrolling;           // use scrolling instead of wrapping at playfield borders

  GdElement initial_fill;
  GdElement initial_border;
  GdElement random_fill[4];             // Random fill elements.
  int random_fill_probability[4];       // Random fill, probability of each element.

  int level_rand[5];                    // Random seed.
  int level_diamonds[5];                // Must collect diamonds, on level x
  int level_speed[5];                   // Time between game cycles in ms
  int level_ckdelay[5];                 // Timing in original game units
  int level_time[5];                    // Available time, per level
  int level_timevalue[5];               // points for each second remaining, when exiting level

  int max_time;                         // the maximum time in seconds. if above, it overflows

  int w, h;                             // Sizes of cave, width and height.
  int x1,y1,x2,y2;                      // Visible part of the cave
  GdColor color_b;                      // border color
  GdColor color[MAX_LEVEL_COLORS];      // c64-style colors; color 4 and 5 are amoeba and slime.
  GdColor base_color[MAX_LEVEL_COLORS]; // base colors for color gradients
  int diamond_value;                    // Score for a diamond.
  int extra_diamond_value;              // Score for a diamond, when gate is open.

  boolean stone_sound;
  boolean nut_sound;
  boolean diamond_sound;
  boolean nitro_sound;
  boolean falling_wall_sound;
  boolean expanding_wall_sound;
  boolean bladder_spender_sound;
  boolean bladder_convert_sound;

  int level_magic_wall_time[5];         // magic wall 'on' state for each level (seconds)
  boolean magic_wall_stops_amoeba;      // Turning on magic wall changes amoeba to diamonds.
                                        // Original BD: yes, constkit: no
  boolean magic_wall_breakscan;		// Currently this setting enabled will turn the amoeba to
                                        // an enclosed state. To implement buggy BD1 behaviour.
  boolean magic_timer_zero_is_infinite;	// magic wall timer 0 is interpreted as infinite
  boolean magic_timer_wait_for_hatching;// magic wall timer does not start before player's birth
  boolean magic_wall_sound;             // magic wall has sound

  int level_amoeba_time[5];             // amoeba time for each level
  int amoeba_growth_prob;               // Amoeba slow growth probability
  int amoeba_fast_growth_prob;          // Amoeba fast growth probability
  int level_amoeba_threshold[5];        // amoeba turns to stones; if count is bigger than this
                                        // (number of cells)
  GdElement amoeba_enclosed_effect;     // an enclosed amoeba converts to this element
  GdElement amoeba_too_big_effect;      // an amoeba grown too big converts to this element

  int level_amoeba_2_time[5];           // amoeba time for each level
  int amoeba_2_growth_prob;             // Amoeba slow growth probability
  int amoeba_2_fast_growth_prob;        // Amoeba fast growth probability
  int level_amoeba_2_threshold[5];      // amoeba turns to stones; if count is bigger than this
                                        // (number of cells)
  GdElement amoeba_2_enclosed_effect;   // an enclosed amoeba converts to this element
  GdElement amoeba_2_too_big_effect;    // an amoeba grown too big converts to this element
  boolean amoeba_2_explodes_by_amoeba;  // amoeba 2 will explode if touched by amoeba1
  GdElement amoeba_2_explosion_effect;  // amoeba 2 explosion ends in ...
  GdElement amoeba_2_looks_like;        // an amoeba 2 looks like this element

  boolean amoeba_timer_started_immediately; // FALSE: amoeba will start life at the first
                                            //        possibility of growing.
  boolean amoeba_timer_wait_for_hatching;   // amoeba timer does not start before player's birth
  boolean amoeba_sound;                 // if the living amoeba has sound.

  GdElement acid_eats_this;             // acid eats this element
  int acid_spread_ratio;                // Probability of acid blowing up, each frame
  boolean acid_spread_sound;            // acid has sound
  GdElement acid_turns_to;              // whether acid converts to explosion on spreading or other

  GdElement nut_turns_to_when_crushed;  // when nut is hit by stone, it converts to this element

  int level_slime_permeability[5];      // true random slime
  int level_slime_permeability_c64[5];  // Appearing in bd 2
  int level_slime_permeability_old[5];  // Appearing in bd 2 (only used by old engine)
  int level_slime_seed_c64[5];          // predictable slime random seed
  boolean slime_predictable;            // predictable random start for slime. yes for plck.
  boolean slime_correct_random;         // correct random number generator for rendered caves
  GdElement slime_eats_1, slime_converts_1; // slime eats element x and converts to element x;
                                            // for example diamond -> falling diamond
  GdElement slime_eats_2, slime_converts_2; // this is usually stone -> stone_f
  GdElement slime_eats_3, slime_converts_3; // this is usually nut -> nut_f
  GdElement slime_eats_4, slime_converts_4; // this is usually empty (not defined)
  GdElement slime_eats_5, slime_converts_5; // this is usually empty (not defined)
  boolean slime_sound;                  // slime has sound

  boolean lava_sound;                   // elements sinking in lava have sound

  int level_hatching_delay_frame[5];    // Scan frames before Player's birth.
  int level_hatching_delay_time[5];     // Scan frames before Player's birth.

  int level_bonus_time[5];              // bonus time for clock collected.
  int level_penalty_time[5];            // Time penalty when voodoo destroyed.
  boolean voodoo_collects_diamonds;     // Voodoo can collect diamonds
  boolean voodoo_dies_by_stone;         // Voodoo can be killed by a falling stone
  boolean voodoo_disappear_in_explosion;// Voodoo can be destroyed by and explosion
  boolean voodoo_any_hurt_kills_player; // If any voodoo hurt in any way, player is killed.

  boolean water_does_not_flow_down;     // if true, water will not grow downwards,
                                        // only in other directions.
  boolean water_sound;                  // water has sound

  boolean bladder_sound;                // bladder moving and pushing has sound
  GdElement bladder_converts_by;        // bladder converts to clock by touching this element

  int biter_delay_frame;                // frame count biters do move
  GdElement biter_eat;                  // biters eat this
  boolean biter_sound;                  // biters have sound

  boolean expanding_wall_changed;       // expanding wall direction is changed

  int    replicator_delay_frame;        // replicator delay in frames (number of frames
                                        // to wait between creating a new element)
  boolean replicators_active;           // replicators are active.
  boolean replicator_sound;             // when replicating an element, play sound or not.

  boolean conveyor_belts_active;
  boolean conveyor_belts_direction_changed;
  boolean conveyor_belts_buggy;		// use old, buggy conveyor belt behavior

  // effects
  GdElement explosion_effect;           // explosion converts to this element after its last stage.
                                        // diego effect.
  GdElement explosion_3_effect;         // O_EXPLODE_3 converts to this element
                                        // diego effect, for compatibility.
  GdElement diamond_birth_effect;       // a diamond birth converts to this element after its last
                                        // stage. diego effect.
  GdElement bomb_explosion_effect;      // bombs explode to this element. diego effect (almost).
  GdElement nitro_explosion_effect;     // nitros explode to this

  GdElement firefly_explode_to;         // fireflies explode to this when hit by stone
  GdElement alt_firefly_explode_to;     // alternative fireflies explode to this when hit by stone
  GdElement butterfly_explode_to;       // butterflies explode to this when hit by stone
  GdElement alt_butterfly_explode_to;   // alternative butterflies explode to this when hit by stone
  GdElement stonefly_explode_to;        // stoneflies explode to this when hit by stone
  GdElement dragonfly_explode_to;       // dragonflies explode to this when hit by stone

  GdElement stone_falling_effect;       // falling stone converts to this element. diego effect.
  GdElement diamond_falling_effect;     // falling diamond converts to this element. diego effect.
  GdElement stone_bouncing_effect;      // bouncing stone converts to this element. diego effect.
  GdElement diamond_bouncing_effect;    // bouncing diamond converts to this element. diego effect.

  GdElement expanding_wall_looks_like;  // an expanding wall looks like this element. diego effect.
  GdElement dirt_looks_like;            // dirt looks like this element. diego effect.

  GdElement magic_stone_to;             // magic wall converts falling stone to
  GdElement magic_diamond_to;           // magic wall converts falling diamond to
  GdElement magic_mega_stone_to;        // magic wall converts a falling mega stone to
  GdElement magic_light_stone_to;       // magic wall converts a falling light stone to
  GdElement magic_nitro_pack_to;        // magic wall converts a falling nitro pack to
  GdElement magic_nut_to;               // magic wall converts a falling nut to
  GdElement magic_flying_stone_to;      // flying stones are converted to
  GdElement magic_flying_diamond_to;    // flying diamonds are converted to

  int pushing_stone_prob;               // probability of pushing stone
  int pushing_stone_prob_sweet;         // probability of pushing, after eating sweet
  boolean mega_stones_pushable_with_sweet; // mega stones may be pushed with sweet

  boolean creatures_backwards;          // creatures changed direction
  boolean creatures_direction_auto_change_on_start; // the change occurs also at the start signal
  int creatures_direction_auto_change_time; // creatures automatically change direction every x
                                            // seconds
  boolean creature_direction_auto_change_sound; // automatically changing creature direction may
                                                // have the sound of the creature dir switch

  int skeletons_needed_for_pot;         // how many skeletons to be collected, to use a pot
  int skeletons_worth_diamonds;         // for crazy dream 7 compatibility: collecting skeletons
                                        // might open the cave door.

  GdDirection gravity;
  int gravity_change_time;
  boolean gravity_change_sound;
  boolean gravity_affects_all;          // if true, gravity also affects falling wall, bladder
                                        // and waiting stones
  boolean gravity_switch_active;        // true if gravity switch is activated, and can be used.

  boolean hammered_walls_reappear;
  int pneumatic_hammer_frame;
  int hammered_wall_reappear_frame;
  boolean pneumatic_hammer_sound;

  boolean infinite_rockets;             // if true, the player which got a rocket launcher will be
                                        // able to launch an infinite number of rockets

  boolean buggy_teleporter;		// use old, buggy teleporter behavior

  // internal variables, used during the game. private data :)

  // returns range corrected x/y position (points to perfect or line shifting get function)
  int (*getx) (const struct _gd_cave*, int x, int y);
  int (*gety) (const struct _gd_cave*, int x, int y);

  // returns pointer to element at x, y (points to perfect border or a line shifting get function)
  GdElement* (*getp) (const struct _gd_cave*, int x, int y);

  boolean hatched;                      // hatching has happened. (timers may run, ...)
  boolean gate_open;                    // self-explaining
  unsigned int render_seed;             // the seed value, which was used to render the cave,
                                        // is saved here. will be used by record&playback
  GdRand *random;                       // random number generator of rendered cave
  int rendered;                         // if not zero, rendered at level x
  int timing_factor;                    // number of "milliseconds" in each second :)
                                        // 1000 for ntsc, 1200 for pal.
  void ***objects_order;                // two-dimensional map of cave; each cell is a pointer
                                        // to the drawing object, which created this element.
                                        // NULL if map or random.
  int **hammered_reappear;              // integer map of cave; if non-zero, a brick wall will
                                        // appear there

  int speed;                            // Time between game cycles in ms
  int ckdelay;                          // a ckdelay value for the level this cave is rendered for
  int ckdelay_current;                  // ckdelay value for the current iteration
  int ckdelay_extra_for_animation;      // bd1 and similar engines had animation bits in cave data,
                                        // to set which elements to animate (firefly, butterfly,
                                        // amoeba).
                                        // animating an element also caused some delay each frame;
                                        // according to my measurements, around 2.6 ms/element.

  int frame;  // XXX

  int hatching_delay_frame;
  int hatching_delay_time;
  int time_bonus;                       // bonus time for clock collected.
  int time_penalty;                     // Time penalty when voodoo destroyed.
  int time;                             // milliseconds remaining to finish cave
  int timevalue;                        // points for remaining seconds - for current level
  int diamonds_needed;                  // diamonds needed to open outbox
  int diamonds_collected;               // diamonds collected
  int skeletons_collected;              // number of skeletons collected
  int rockets_collected;                // number of rockets collected
  int gate_open_flash;                  // flashing of screen when gate opens
  int score;                            // Score got this frame.
  int amoeba_time;                      // Amoeba growing slow (low probability, default 3%) for
                                        // milliseconds. After that, fast growth default (25%)
  int amoeba_2_time;                    // Amoeba growing slow (low probability, default 3%) for
                                        // milliseconds. After that, fast growth default (25%)
  int amoeba_max_count;                 // selected amoeba 1 threshold for this level
  int amoeba_2_max_count;               // selected amoeba 2 threshold for this level
  GdAmoebaState amoeba_state;           // state of amoeba 1
  GdAmoebaState amoeba_2_state;         // state of amoeba 2
  boolean convert_amoeba_this_frame;    // To implement BD1 buggy amoeba+magic wall behaviour.
  int magic_wall_time;                  // magic wall 'on' state for seconds
  int slime_permeability;               // true random slime
  int slime_permeability_c64;           // Appearing in bd 2
  GdMagicWallState magic_wall_state;    // State of magic wall
  GdPlayerState player_state;           // Player state. not yet living, living, exited...
  int player_seen_ago;                  // player was seen this number of scans ago
  int player_seen_ago_limit;            // number of scans player has to be gone for game over
  boolean voodoo_touched;               // as its name says
  boolean kill_player;                  // Voodoo died, or used pressed escape to restart level.
  boolean sweet_eaten;                  // player ate sweet, he's strong. prob_sweet applies,
                                        // and also able to push chasing stones
  int player_x, player_y;               // Coordinates of player (for scrolling)
  int player_x_mem[GD_PLAYER_MEM_SIZE]; // coordinates of player, for chasing stone
  int player_y_mem[GD_PLAYER_MEM_SIZE];
  int key1, key2, key3;                 // The player is holding this number of keys of each color
  boolean diamond_key_collected;        // Key collected, so trapped diamonds convert to diamonds
  boolean inbox_flash_toggle;           // negated every scan. helps drawing inboxes, and making
                                        // players be born at different times.
  GdDirection last_direction;           // last direction player moved. used by draw routines
  GdDirection last_direction_2nd;       // 2nd last direction player moved. used by draw routines
  GdDirection last_horizontal_direction;
  int biters_wait_frame;                // number of frames to wait until biters will move again
  int replicators_wait_frame;           // number of frames to wait until replicators are
                                        // activated again
  int creatures_direction_will_change;  // creatures automatically change direction every x seconds
  GdC64RandomGenerator c64_rand;        // used for predictable random generator during the game.

  int gravity_will_change;              // gravity will change in this number of milliseconds
  boolean gravity_disabled;             // when player is stirring the pot, there is no gravity.
  GdDirection gravity_next_direction;   // next direction when the gravity changes.
                                        // will be set by the player "getting" a gravity switch
  boolean got_pneumatic_hammer;         // true if the player has a pneumatic hammer
  int pneumatic_hammer_active_delay;    // number of frames to wait, till pneumatic hammer will
                                        // destroy the wall
  GdSound sound1, sound2, sound3;       // sound set for 3 channels after each iteration
} GdCave;


#define CAVE_OFFSET(property) (STRUCT_OFFSET(GdCave, property))

// arrays for movements
// also no1 and bd2 cave data import helpers; line direction coordinates
extern const int gd_dx[], gd_dy[];

extern GdElement gd_char_to_element[];

void gd_create_char_to_element_table(void);
GdElement gd_get_element_from_character(unsigned char character);
GdElement gd_get_element_from_string(const char *string);

// init cave engine
void gd_cave_init(void);

// for cave tags hash table
int gd_str_case_equal(void *s1, void *s2);
unsigned int gd_str_case_hash(void *v);

// cave highscore functions
int gd_highscore_compare(const void *a, const void *b);
boolean gd_is_highscore(GdHighScore *scores, int score);
int gd_add_highscore(GdHighScore *highscores, const char *name, int score);
void gd_clear_highscore(GdHighScore *hs);
boolean gd_has_highscore(GdHighScore *hs);

// cave creator and destructor functions
GdCave *gd_cave_new(void);
GdCave *gd_cave_new_from_cave(const GdCave *orig);
void gd_cave_copy(GdCave *dest, const GdCave *src);
void gd_cave_free(GdCave *cave);

// cave manipulation
void gd_cave_set_gdash_defaults(GdCave *cave);
void gd_cave_set_defaults_from_array(GdCave* cave, GdPropertyDefault *defaults);
void gd_cave_correct_visible_size(GdCave *cave);
void gd_cave_set_random_colors(GdCave *cave, GdColorType type);
void gd_cave_auto_shrink(GdCave *cave);

void gd_cave_setup_for_game(GdCave *cave);
void gd_cave_count_diamonds(GdCave *cave);

// c64 random generator support for cave fill
unsigned int gd_c64_random(GdC64RandomGenerator *rand);
unsigned int gd_cave_c64_random(GdCave *);
void gd_c64_random_set_seed(GdC64RandomGenerator *rand, int seed1, int seed2);
void gd_cave_c64_random_set_seed(GdCave *cave, int seed1, int seed2);
void gd_cave_set_random_c64_colors(GdCave *cave);

// support
void *gd_cave_map_new_for_cave(const GdCave *cave, const int cell_size);
void *gd_cave_map_dup_size(const GdCave * cave, const void *map, const int cell_size);
#define gd_cave_map_new(CAVE, TYPE) ((TYPE **)gd_cave_map_new_for_cave((CAVE), sizeof(TYPE)))
#define gd_cave_map_dup(CAVE, MAP) ((void *)gd_cave_map_dup_size((CAVE), (void **)(CAVE)->MAP, sizeof((CAVE)->MAP[0][0])))
void gd_cave_map_free(void *map);

void gd_cave_store_rc(GdCave * cave, int x, int y, const GdElement element, const void* order);
GdElement gd_cave_get_rc (const GdCave *cave, int x, int y);

// direction
const char *gd_direction_get_visible_name(GdDirection dir);
const char *gd_direction_get_filename(GdDirection dir);
GdDirection gd_direction_from_string(const char *str);

// scheduling
const char *gd_scheduling_get_visible_name(GdScheduling sched);
const char *gd_scheduling_get_filename(GdScheduling sched);
GdScheduling gd_scheduling_from_string(const char *str);

// game playing helpers
#define GD_REDRAW (1 << 10)

void gd_drawcave_game(const GdCave *cave,
		      int **element_buffer, int **last_element_buffer,
		      int **drawing_buffer, int **last_drawing_buffer, int **gfx_buffer,
		      int **covered_buffer, int **dir_buffer_from, int **dir_buffer_to,
		      boolean bonus_life_flash, int animcycle, boolean hate_invisible_outbox);

// function to copy a GdString
static inline char *gd_strcpy(GdString dest, const char *src)
{
    return strncpy(dest, src, sizeof(GdString));
}

int gd_cave_time_show(const GdCave *cave, int internal_time);

GdReplay *gd_replay_new(void);
GdReplay *gd_replay_new_from_replay(GdReplay *orig);
void gd_replay_free(GdReplay *replay);
void gd_replay_store_movement(GdReplay *replay, GdDirection player_move, boolean player_fire, boolean suicide);
void gd_replay_store_random(GdReplay *replay, int random);

unsigned int gd_cave_adler_checksum(GdCave *cave);
void gd_cave_adler_checksum_more(GdCave *cave, unsigned int *a, unsigned int *b);

boolean gd_cave_has_levels(GdCave *cave);
boolean gd_caveset_has_levels(void);

void gd_unscan_cave(GdCave *cave);

void gd_update_scheduling_cave_speed(GdCave *cave);

#endif	// BD_CAVE_H
