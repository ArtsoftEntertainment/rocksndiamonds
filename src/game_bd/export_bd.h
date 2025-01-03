// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// export_bd.h
// ============================================================================

#ifndef EXPORT_BD_H
#define EXPORT_BD_H

// ============================================================================
// functions and definitions exported from game_bd to main program
// ============================================================================

#include "bd_cave.h"
#include "bd_caveset.h"
#include "bd_elements.h"
#include "bd_gameplay.h"


// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

#define BD_MAX_CAVE_WIDTH		MAX_PLAYFIELD_WIDTH
#define BD_MAX_CAVE_HEIGHT		MAX_PLAYFIELD_HEIGHT


// ----------------------------------------------------------------------------
// data structure definitions
// ----------------------------------------------------------------------------

struct GameInfo_BD
{
  GdGame *game;

  unsigned int random_seed;

  boolean level_solved;
  boolean game_over;
  boolean cover_screen;

  boolean player_moving;
  boolean player_snapping;

  // needed for updating panel
  int time_left;
  int gems_still_needed;
  int score;

  // needed for saving score time
  int frames_played;

  // global lives and score over more than one game
  int global_lives;
  int global_score;
};

struct LevelInfo_BD
{
  GdCavesetData *caveset;
  GdCave *cave;
  GdReplay *replay;

  int cave_nr;
  int level_nr;

  boolean loaded_from_caveset;
};

struct GraphicInfo_BD
{
  Bitmap *bitmap;
  int src_x, src_y;
  int width, height;

  int graphic;
  int frame;
};

struct EngineSnapshotInfo_BD
{
  GdGame game;

  // data from pointers in game structure
  int element_buffer[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int last_element_buffer[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int drawing_buffer[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int last_drawing_buffer[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int dir_buffer_from[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int dir_buffer_to[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int gfx_buffer[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  int covered_buffer[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];

  GdCave cave;

  // data from pointers in cave structure
  short map[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short hammered_reappear[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GameInfo_BD game_bd;
extern struct LevelInfo_BD native_bd_level;
extern struct GraphicInfo_BD graphic_info_bd_object[O_MAX_ALL][8];
extern struct EngineSnapshotInfo_BD engine_snapshot_bd;

void bd_open_all(void);
void bd_close_all(void);

int map_action_RND_to_BD(int);
int map_action_BD_to_RND(int);

boolean checkGameRunning_BD(void);
boolean checkGamePlaying_BD(void);
boolean checkBonusTime_BD(void);
int getNonScannedElement_BD(int);
int getFramesPerSecond_BD(void);
int getTimeLeft_BD(void);
void setTimeLeft_BD(void);
void SetTimeFrames_BD(int);

void InitGfxBuffers_BD(void);

void setLevelInfoToDefaults_BD_Ext(int, int);
void setLevelInfoToDefaults_BD(void);
boolean LoadNativeLevel_BD(char *, int, boolean);
boolean SaveNativeLevel_BD(char *);
void DumpLevelset_BD(void);

Bitmap *GetColoredBitmapFromTemplate_BD(Bitmap *);

unsigned int InitEngineRandom_BD(int);
void InitGameEngine_BD(void);
void GameActions_BD(byte[MAX_PLAYERS]);

boolean use_native_bd_graphics_engine(void);
boolean use_bd_smooth_movements(void);
boolean use_bd_pushing_graphics(void);
boolean use_bd_up_down_graphics(void);
boolean use_bd_falling_sounds(void);

Bitmap **GetTitleScreenBitmaps_BD(void);
void CoverScreen_BD(void);

void BlitScreenToBitmap_BD(Bitmap *);
void RedrawPlayfield_BD(boolean);

void SaveEngineSnapshotValues_BD(void);
void LoadEngineSnapshotValues_BD(void);

#endif	// EXPORT_BD_H
