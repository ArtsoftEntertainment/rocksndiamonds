#ifndef GAME_BD_EXPORT_H
#define GAME_BD_EXPORT_H

// ============================================================================
// functions and definitions exported from game_bd to main program
// ============================================================================


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

#endif	// GAME_BD_EXPORT_H
