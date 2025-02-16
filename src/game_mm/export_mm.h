// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// export_mm.h
// ============================================================================

#ifndef EXPORT_MM_H
#define EXPORT_MM_H

// ============================================================================
// functions and definitions exported from game_mm to main program
// ============================================================================

// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

#define MM_MAX_PLAYFIELD_WIDTH		MAX_PLAYFIELD_WIDTH
#define MM_MAX_PLAYFIELD_HEIGHT		MAX_PLAYFIELD_HEIGHT

#define MM_STD_PLAYFIELD_WIDTH		16
#define MM_STD_PLAYFIELD_HEIGHT		12

#define MM_MAX_PLAYFIELD_SIZE		(MM_MAX_PLAYFIELD_WIDTH *	\
					 MM_MAX_PLAYFIELD_HEIGHT)

#define MAX_NUM_AMOEBA			100
#define MAX_NUM_BEAMERS			8

#define MAX_LASER_LEN			256
#define MAX_LASER_ENERGY		100
#define MAX_LASER_OVERLOAD		100

#define MM_LEVEL_SCORE_ELEMENTS		16

#define MM_MAX_BALL_CONTENTS		16

#define MM_MAX_LEVEL_NAME_LEN		32
#define MM_MAX_LEVEL_AUTHOR_LEN		32


#define EL_MM_START_1_NATIVE		0
#define EL_MM_END_1_NATIVE		159

#define EL_MM_CHAR_START_NATIVE		160
#define EL_MM_CHAR_END_NATIVE		239

#define EL_MM_START_2_NATIVE		240
#define EL_MM_END_2_NATIVE		430

#define EL_MM_START_3_NATIVE		431
#define EL_MM_END_3_NATIVE		450

#define EL_MM_RUNTIME_START_NATIVE	500
#define EL_MM_RUNTIME_END_NATIVE	504

// elements to be specially mapped
#define EL_MM_EMPTY_NATIVE		0
#define EL_DF_EMPTY_NATIVE		304

// sounds
#define SND_MM_GAME_LEVELTIME_CHARGING	0
#define SND_MM_GAME_HEALTH_CHARGING	1


// ----------------------------------------------------------------------------
// data structure definitions
// ----------------------------------------------------------------------------

struct CycleList
{
  int x, y;
  int steps;
};

struct MovingList
{
  int x, y;
  int dir;
};

struct DamageList
{
  int x, y;
  int edge, angle;
  boolean is_mirror;
};

struct BeamerInfo
{
  int x, y;
  int num;
};

struct PacMan
{
  int XP, YP;
  int Dr;
};

struct LaserInfo
{
  struct XY start_edge;
  int start_angle;

  int current_angle;

  struct DamageList damage[MAX_LASER_LEN + 10];
  int num_damages;

  struct XY edge[MAX_LASER_LEN + 10];
  int num_edges;

  struct BeamerInfo beamer[MAX_NUM_BEAMERS][2];
  int beamer_edge[MAX_NUM_BEAMERS];
  int beamer_nr[MAX_NUM_BEAMERS];
  int num_beamers;

  boolean overloaded;
  int overload_value;

  boolean fuse_off;
  int fuse_x, fuse_y;

  int dest_element;
  int dest_element_last;
  int dest_element_last_x;
  int dest_element_last_y;
  boolean stops_inside_element;

  boolean redraw;

  int wall_mask;
};

struct GameInfo_MM
{
  boolean LevelSolved;
  boolean GameOver;

  struct CycleList cycle[MM_MAX_PLAYFIELD_SIZE];
  int num_cycle;

  struct MovingList pacman[MM_MAX_PLAYFIELD_SIZE];
  int num_pacman;

  int score;
  int energy_left;
  int kettles_still_needed;
  int lights_still_needed;
  int num_keys;
  int ball_choice_pos;		// current content element choice position
  boolean laser_red, laser_green, laser_blue;

  boolean has_mcduffin;
  boolean level_solved;
  boolean game_over;
  int game_over_cause;
  char *game_over_message;

  boolean cheat_no_overload;
  boolean cheat_no_explosion;

  int laser_overload_value;
  boolean laser_enabled;
};

struct LevelInfo_MM
{
  VersionType file_version;		// version of file the level was stored with
  VersionType game_version;		// version of game engine to play this level
  boolean encoding_16bit_field;		// level contains 16-bit elements

  int fieldx;
  int fieldy;
  int time;
  int kettles_needed;
  boolean auto_count_kettles;
  boolean mm_laser_red, mm_laser_green, mm_laser_blue;
  boolean df_laser_red, df_laser_green, df_laser_blue;
  char name[MM_MAX_LEVEL_NAME_LEN + 1];
  char author[MM_MAX_LEVEL_AUTHOR_LEN + 1];
  int score[MM_LEVEL_SCORE_ELEMENTS];
  int amoeba_speed;
  int time_fuse;
  int time_bomb;
  int time_ball;
  int time_block;

  int num_ball_contents;
  int ball_choice_mode;
  int ball_content[MM_MAX_BALL_CONTENTS];
  boolean rotate_ball_content;
  boolean explode_ball;

  short field[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
};

struct EngineSnapshotInfo_MM
{
  struct GameInfo_MM game_mm;

  struct LaserInfo laser;

  short Ur[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Hit[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Box[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Angle[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];

  short LX, LY;
  short XS, YS;
  short ELX, ELY;
  short CT, Ct;

  int last_LX, last_LY, last_hit_mask;
  int hold_x, hold_y;
  int pacman_nr;

  DelayCounter rotate_delay;
  DelayCounter pacman_delay;
  DelayCounter energy_delay;
  DelayCounter overload_delay;
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GameInfo_MM game_mm;
extern struct LevelInfo_MM native_mm_level;
extern struct EngineSnapshotInfo_MM engine_snapshot_mm;

extern short Ur[MM_MAX_PLAYFIELD_WIDTH][MM_MAX_PLAYFIELD_HEIGHT];

void mm_open_all(void);

void InitElementProperties_MM(void);

void InitGfxBuffers_MM(void);

void InitGameEngine_MM(void);
void InitGameActions_MM(void);
void GameActions_MM(struct MouseActionInfo);

void DrawLaser_MM(void);
void DrawTileCursor_MM(int, int, boolean);
void FreeTileCursorTextures_MM(void);

boolean ClickElement(int, int, int);

unsigned int InitEngineRandom_MM(int);

void setLevelInfoToDefaults_MM(void);
void copyInternalEngineVars_MM(void);
boolean LoadNativeLevel_MM(char *, boolean);
void SaveNativeLevel_MM(char *);

int getFieldbufferOffsetX_MM(void);
int getFieldbufferOffsetY_MM(void);

int getFlippedTileX_MM(int);
int getFlippedTileY_MM(int);
int getFlippedTileXY_MM(int);

void BlitScreenToBitmap_MM(Bitmap *);
void RedrawPlayfield_MM(void);

void LoadEngineSnapshotValues_MM(void);
void SaveEngineSnapshotValues_MM(void);

int getButtonFromTouchPosition(int, int, int, int);

#endif	// EXPORT_MM_H
