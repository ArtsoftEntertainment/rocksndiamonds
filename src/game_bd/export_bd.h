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

  // needed for updating panel
  int time_played;
  int gems_still_needed;
  int score;
};

struct LevelInfo_BD
{
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
};

struct EngineSnapshotInfo_BD
{
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GameInfo_BD game_bd;
extern struct LevelInfo_BD native_bd_level;
extern struct GraphicInfo_BD graphic_info_bd_object[O_MAX_ALL][8];
extern struct GraphicInfo_BD graphic_info_bd_color_template;
extern struct EngineSnapshotInfo_BD engine_snapshot_bd;

void bd_open_all(void);
void bd_close_all(void);

int map_action_RND_to_BD(int);
int map_action_BD_to_RND(int);

boolean checkGameRunning_BD(void);
boolean checkGamePlaying_BD(void);
boolean checkBonusTime_BD(void);
int getFramesPerSecond_BD(void);
int getTimeLeft_BD(void);

void InitGfxBuffers_BD(void);

void setLevelInfoToDefaults_BD_Ext(int, int);
void setLevelInfoToDefaults_BD(void);
boolean LoadNativeLevel_BD(char *, int, boolean);
boolean SaveNativeLevel_BD(char *);

void PreparePreviewTileBitmap_BD(Bitmap *, int);
void SetPreviewTileBitmapReference_BD(Bitmap *);
Bitmap *GetPreviewTileBitmap_BD(Bitmap *);

unsigned int InitEngineRandom_BD(int);
void InitGameEngine_BD(void);
void GameActions_BD(byte[MAX_PLAYERS]);
void CoverScreen_BD(void);

void BlitScreenToBitmap_BD(Bitmap *);
void RedrawPlayfield_BD(boolean);

#endif	// EXPORT_BD_H
