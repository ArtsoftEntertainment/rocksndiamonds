#ifndef GAME_BD_EXPORT_H
#define GAME_BD_EXPORT_H

// ============================================================================
// functions and definitions exported from game_bd to main program
// ============================================================================

// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

// ...


// ----------------------------------------------------------------------------
// data structure definitions
// ----------------------------------------------------------------------------

struct GameInfo_BD
{
};

struct LevelInfo_BD
{
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
