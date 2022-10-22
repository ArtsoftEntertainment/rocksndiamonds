#ifndef EXPORT_H
#define EXPORT_H

// ============================================================================
// functions and definitions exported from game_em to main program
// ============================================================================

#include "emerald.h"
#include "cave.h"


// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

#define EM_MAX_CAVE_WIDTH		CAVE_WIDTH
#define EM_MAX_CAVE_HEIGHT		CAVE_HEIGHT


// ----------------------------------------------------------------------------
// exported structures
// ----------------------------------------------------------------------------

struct GlobalInfo_EM
{
  Bitmap *screenbuffer;
};

struct GameInfo_EM
{
  unsigned int random;

  boolean level_solved;
  boolean game_over;

  boolean any_player_moving;
  boolean any_player_snapping;

  int last_moving_player;
  int last_player_direction[MAX_PLAYERS];

  struct LOGIC *lev;
  struct PLAYER *ply[MAX_PLAYERS];

  // flags to handle bugs in and changes between different engine versions
  boolean use_single_button;
  boolean use_snap_key_bug;
  boolean use_random_bug;
  boolean use_old_explosions;
  boolean use_old_android;
  boolean use_old_push_elements;
  boolean use_old_push_into_acid;
  boolean use_wrap_around;
};

struct LevelInfo_EM
{
  int file_version;

  struct CAVE *cav;
};

struct GraphicInfo_EM
{
  Bitmap *bitmap;
  int src_x, src_y;
  int src_offset_x, src_offset_y;
  int dst_offset_x, dst_offset_y;
  int width, height;

  Bitmap *crumbled_bitmap;
  int crumbled_src_x, crumbled_src_y;
  int crumbled_border_size;
  int crumbled_tile_size;

  boolean has_crumbled_graphics;
  boolean preserve_background;

  int unique_identifier;	/* used to identify needed screen updates */
};

struct EngineSnapshotInfo_EM
{
  struct GameInfo_EM game_em;
  struct LOGIC lev;
  struct PLAYER ply[MAX_PLAYERS];

  int frame;
  int screen_x;
  int screen_y;
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GlobalInfo_EM global_em_info;
extern struct GameInfo_EM game_em;
extern struct LevelInfo_EM native_em_level;
extern struct GraphicInfo_EM graphic_info_em_object[GAME_TILE_MAX][8];
extern struct GraphicInfo_EM graphic_info_em_player[MAX_PLAYERS][PLY_MAX][8];
extern struct EngineSnapshotInfo_EM engine_snapshot_em;

void em_open_all(void);
void em_close_all(void);

void InitGfxBuffers_EM(void);

void InitGameEngine_EM(void);
void GameActions_EM(byte[MAX_PLAYERS]);

unsigned int InitEngineRandom_EM(int);

void setLevelInfoToDefaults_EM(void);
boolean LoadNativeLevel_EM(char *, boolean);

int getFieldbufferOffsetX_EM(void);
int getFieldbufferOffsetY_EM(void);

void BlitScreenToBitmap_EM(Bitmap *);
void RedrawPlayfield_EM(boolean);

void LoadEngineSnapshotValues_EM(void);
void SaveEngineSnapshotValues_EM(void);

boolean checkIfAllPlayersFitToScreen(void);

int map_em_element_C_to_X(int);
int map_em_element_X_to_C(int);

#endif	// EXPORT_H
