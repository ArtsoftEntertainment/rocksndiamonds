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
// initialization functions
// ============================================================================

void InitGfxBuffers_BD(void)
{
  ReCreateBitmap(&gd_screen_bitmap, SXSIZE, SYSIZE);

  set_cell_size(TILESIZE_VAR);
  set_play_area(SXSIZE, SYSIZE);
}

void bd_open_all(void)
{
  InitGraphicInfo_BD();

  gd_cave_init();
  gd_cave_db_init();

  gd_c64_import_init_tables();

  gd_caveset_clear();

  gd_init_keystate();

  gd_sound_init();
}

void bd_close_all(void)
{
}


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

    cave->x1 = 0;
    cave->y1 = 0;
    cave->x2 = cave->w - 1;
    cave->y2 = cave->h - 1;
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

boolean checkGamePlaying_BD(void)
{
  return (game_bd.game != NULL && game_bd.game->state_counter == GAME_INT_CAVE_RUNNING &&
	  game_bd.game->cave != NULL && game_bd.game->cave->hatched);
}

boolean checkBonusTime_BD(void)
{
  return (game_bd.game != NULL && game_bd.game->state_counter == GAME_INT_CHECK_BONUS_TIME);
}

int getFramesPerSecond_BD(void)
{
  if (game_bd.game != NULL && game_bd.game->cave != NULL && game_bd.game->cave->pal_timing)
    return FRAMES_PER_SECOND_NTSC;

  return FRAMES_PER_SECOND_PAL;
}

int getTimeLeft_BD(void)
{
  if (game_bd.game != NULL && game_bd.game->cave != NULL)
    return gd_cave_time_show(game_bd.game->cave, game_bd.game->cave->time);

  return 0;
}

static void UpdateGameDoorValues_BD(void)
{
  GdCave *cave = game_bd.game->cave;
  int time_secs = gd_cave_time_show(cave, cave->time);
  int gems_still_needed = MAX(0, (cave->diamonds_needed - cave->diamonds_collected));

  game_bd.time_played = time_secs;
  game_bd.gems_still_needed = gems_still_needed;
  game_bd.score = game_bd.game->player_score;

  if (game.LevelSolved)
  {
    // update time and score in panel while counting bonus time
    game.LevelSolved_CountingTime  = game_bd.time_played;
    game.LevelSolved_CountingScore = game_bd.score;
  }
}

unsigned int InitEngineRandom_BD(int seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    // get randomly selected seed to render the cave
    seed = g_random_int_range(0, GD_CAVE_SEED_MAX);
  }

  game_bd.random_seed = seed;

  return (unsigned int)seed;
}

void InitGameEngine_BD(void)
{
  game_bd.level_solved = FALSE;
  game_bd.game_over = FALSE;
  game_bd.cover_screen = FALSE;

  game_bd.time_played = 0;
  game_bd.gems_still_needed = 0;
  game_bd.score = 0;

  gd_caveset_last_selected       = native_bd_level.cave_nr;
  gd_caveset_last_selected_level = native_bd_level.level_nr;

  if (game_bd.game != NULL)
    gd_game_free(game_bd.game);

  game_bd.game = gd_game_new(native_bd_level.cave_nr, native_bd_level.level_nr);

  game_bd.game->itercycle = 0;
  game_bd.game->itermax = 8;	// default; dynamically changed at runtime
  game_bd.game->itermax_last = game_bd.game->itermax;

  // default: start with completely covered playfield
  int next_state = GAME_INT_START_UNCOVER + 1;

  // when skipping uncovering, start with uncovered playfield
  if (setup.bd_skip_uncovering)
    next_state = GAME_INT_LOAD_CAVE + 1;

  // fast-forward game engine until cave loaded (covered or uncovered)
  while (game_bd.game->state_counter < next_state)
    play_game_func(game_bd.game, 0);

  // when skipping uncovering, continue with uncovered playfield
  if (setup.bd_skip_uncovering)
    game_bd.game->state_counter = GAME_INT_UNCOVER_ALL + 1;

  if (setup.bd_skip_uncovering)
    gd_scroll(game_bd.game, TRUE, TRUE);

  ClearRectangle(gd_screen_bitmap, 0, 0, SXSIZE, SYSIZE);

  RedrawPlayfield_BD(TRUE);

  UpdateGameDoorValues_BD();
}

void GameActions_BD(byte action[MAX_PLAYERS])
{
  GdCave *cave = game_bd.game->cave;
  boolean player_found = FALSE;
  int player_x = 0;
  int player_y = 0;
  int x, y;

  if (cave->getp)
  {
    for (y = 0; y < cave->h && !player_found; y++)
    {
      for (x = 0; x < cave->w && !player_found; x++)
      {
	int element = *cave->getp(cave, x, y);

	if (element == O_PLAYER ||
	    element == O_PLAYER_BOMB ||
	    element == O_PLAYER_STIRRING ||
	    element == O_PLAYER_PNEUMATIC_LEFT ||
	    element == O_PLAYER_PNEUMATIC_RIGHT)
	{
	  player_x = x;
	  player_y = y;

	  player_found = TRUE;
	}
      }
    }
  }

  UpdateEngineValues(get_scroll_x(),
		     get_scroll_y(),
		     player_x,
		     player_y);

  if (setup.bd_skip_hatching && !game_bd.game->cave->hatched &&
      game_bd.game->state_counter == GAME_INT_CAVE_RUNNING)
  {
    // fast-forward game engine until player hatched
    while (!game_bd.game->cave->hatched)
    {
      play_game_func(game_bd.game, 0);

      // also record or replay tape action during fast-forward
      action = TapeCorrectAction_BD(action);
    }
  }

  play_game_func(game_bd.game, action[0]);

  RedrawPlayfield_BD(FALSE);

  UpdateGameDoorValues_BD();
}


// ============================================================================
// graphics functions
// ============================================================================

void CoverScreen_BD(void)
{
  game_bd.cover_screen = FALSE;

  if (setup.bd_skip_uncovering)
    return;

  game_bd.game->state_counter = GAME_INT_COVER_START;

  // play game engine (with normal speed) until cave covered
  while (game_bd.game->state_counter < GAME_INT_COVER_ALL + 1)
  {
    play_game_func(game_bd.game, 0);

    RedrawPlayfield_BD(TRUE);

    BlitScreenToBitmap_BD(backbuffer);

    BackToFront();
  }

  // stop uncovering loop sound when not using native sound engine
  FadeSounds();
}

void BlitScreenToBitmap_BD(Bitmap *target_bitmap)
{
  GdCave *cave = native_bd_level.cave;
  int xsize = SXSIZE;
  int ysize = SYSIZE;
  int full_xsize = (cave->x2 - cave->x1 + 1) * TILESIZE_VAR;
  int full_ysize = (cave->y2 - cave->y1 + 1) * TILESIZE_VAR;
  int sx = SX + (full_xsize < xsize ? (xsize - full_xsize) / 2 : 0);
  int sy = SY + (full_ysize < ysize ? (ysize - full_ysize) / 2 : 0);
  int sxsize = (full_xsize < xsize ? full_xsize : xsize);
  int sysize = (full_ysize < ysize ? full_ysize : ysize);

  BlitBitmap(gd_screen_bitmap, target_bitmap, 0, 0, sxsize, sysize, sx, sy);
}

void RedrawPlayfield_BD(boolean force_redraw)
{
  gd_drawcave(gd_screen_bitmap, game_bd.game, force_redraw);
}