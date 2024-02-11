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

void setLevelInfoToDefaults_BD_Ext(int width, int height)
{
  // ...
}

void setLevelInfoToDefaults_BD(void)
{
  setLevelInfoToDefaults_BD_Ext(0, 0);
}
