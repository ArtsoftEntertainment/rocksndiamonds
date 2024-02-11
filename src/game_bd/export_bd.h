// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//                  Holger Schemel
//                  info@artsoft.org
//                  https://www.artsoft.org/
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
  boolean level_solved;
  boolean game_over;

  // needed for updating panel
  int time_played;
  int gems_still_needed;
  int score;
};

struct LevelInfo_BD
{
  int width;
  int height;

  int cave[BD_MAX_CAVE_WIDTH][BD_MAX_CAVE_HEIGHT];
};

struct EngineSnapshotInfo_BD
{
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GameInfo_BD game_bd;
extern struct LevelInfo_BD native_bd_level;
extern struct EngineSnapshotInfo_BD engine_snapshot_bd;

void setLevelInfoToDefaults_BD(void);

#endif	// EXPORT_BD_H
