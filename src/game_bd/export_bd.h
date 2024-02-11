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

struct EngineSnapshotInfo_BD
{
};


// ----------------------------------------------------------------------------
// exported functions
// ----------------------------------------------------------------------------

extern struct GameInfo_BD game_bd;
extern struct LevelInfo_BD native_bd_level;
extern struct EngineSnapshotInfo_BD engine_snapshot_bd;

int map_action_BD_to_RND(int);

void setLevelInfoToDefaults_BD_Ext(int, int);
void setLevelInfoToDefaults_BD(void);

#endif	// EXPORT_BD_H
