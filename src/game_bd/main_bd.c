// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2024 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// main_bd.c
// ============================================================================

#include "main_bd.h"


struct GameInfo_BD game_bd;
struct LevelInfo_BD native_bd_level;
struct EngineSnapshotInfo_BD engine_snapshot_bd;


// ============================================================================
// level file functions
// ============================================================================

void setLevelInfoToDefaults_BD_Ext(int width, int height)
{
  GdCave *cave = native_bd_level.cave;

  if (cave != NULL)
    gd_cave_free(cave);

  // get empty cave, using default values
  cave = gd_cave_new();

  // set cave size, if defined
  if (width > 0 && height > 0)
  {
    cave->w = width;
    cave->h = height;
  }

  gd_flatten_cave(cave, 0);

  cave->selectable = TRUE;
  cave->intermission = FALSE;

  native_bd_level.cave = cave;
  native_bd_level.replay = NULL;

  native_bd_level.cave_nr = 0;
  native_bd_level.level_nr = 0;

  native_bd_level.loaded_from_caveset = FALSE;
}

void setLevelInfoToDefaults_BD(void)
{
  setLevelInfoToDefaults_BD_Ext(0, 0);
}

boolean LoadNativeLevel_BD(char *filename, int level_pos, boolean level_info_only)
{
  static char *filename_loaded = NULL;

  if (filename_loaded == NULL || !strEqual(filename, filename_loaded))
  {
    if (!gd_caveset_load_from_file(filename))
    {
      if (!level_info_only)
	Warn("cannot load BD cave set from file '%s'", filename);

      return FALSE;
    }

    setString(&filename_loaded, filename);
  }

  if (level_pos < 0 || level_pos >= 5 * gd_caveset_count())
  {
    Warn("invalid level position %d in BD cave set", level_pos);

    return FALSE;
  }

  native_bd_level.cave_nr  = level_pos % gd_caveset_count();
  native_bd_level.level_nr = level_pos / gd_caveset_count();

  if (native_bd_level.cave != NULL)
    gd_cave_free(native_bd_level.cave);

  // get selected cave, prepared for playing
  native_bd_level.cave = gd_get_prepared_cave_from_caveset(native_bd_level.cave_nr,
							   native_bd_level.level_nr);

  // set better initial cave speed (to set better native replay tape length)
  set_initial_cave_speed(native_bd_level.cave);

  native_bd_level.loaded_from_caveset = TRUE;

  // check if this cave has any replays
  if (native_bd_level.cave->replays != NULL)
  {
    GList *item = native_bd_level.cave->replays;

    // try to find replay that was recorded for this difficulty level
    while (item != NULL &&
	   (item->data == NULL ||
	    ((GdReplay *)item->data)->success == FALSE ||
	    ((GdReplay *)item->data)->level != native_bd_level.level_nr))
      item = item->next;

    // matching replay found
    if (item != NULL)
      native_bd_level.replay = (GdReplay *)item->data;
  }

  return TRUE;
}


// ============================================================================
// game engine functions
// ============================================================================

int map_action_RND_to_BD(int action)
{
  GdDirection player_move = gd_direction_from_keypress(action & JOY_UP,
						       action & JOY_DOWN,
						       action & JOY_LEFT,
						       action & JOY_RIGHT);
  boolean player_fire = (action & (JOY_BUTTON_1 | JOY_BUTTON_2));

  return (player_move | (player_fire ? GD_REPLAY_FIRE_MASK : 0));
}

int map_action_BD_to_RND(int action)
{
  GdDirection player_move = action & GD_REPLAY_MOVE_MASK;
  boolean     player_fire = action & GD_REPLAY_FIRE_MASK;

  int action_move = (player_move == GD_MV_UP		? JOY_UP		:
		     player_move == GD_MV_UP_RIGHT	? JOY_UP   | JOY_RIGHT	:
		     player_move == GD_MV_RIGHT		?            JOY_RIGHT	:
		     player_move == GD_MV_DOWN_RIGHT	? JOY_DOWN | JOY_RIGHT	:
		     player_move == GD_MV_DOWN		? JOY_DOWN		:
		     player_move == GD_MV_DOWN_LEFT	? JOY_DOWN | JOY_LEFT	:
		     player_move == GD_MV_LEFT		?            JOY_LEFT	:
		     player_move == GD_MV_UP_LEFT	? JOY_UP   | JOY_LEFT	: JOY_NO_ACTION);
  int action_fire = (player_fire ? JOY_BUTTON_1 : JOY_NO_ACTION);

  return (action_move | action_fire);
}

boolean checkGameRunning_BD(void)
{
  return (game_bd.game != NULL && game_bd.game->state_counter == GAME_INT_CAVE_RUNNING);
}
