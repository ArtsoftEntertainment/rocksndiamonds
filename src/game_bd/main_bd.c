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

  gd_cave_db_init();
  gd_cave_init();

  gd_c64_import_init_tables();

  gd_caveset_clear();

  gd_init_play_area();

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
  static GdCavesetData *caveset = NULL;		// used if no native cave loaded
  GdCave *cave = native_bd_level.cave;

  if (caveset != NULL)
    gd_caveset_data_free(caveset);

  if (cave != NULL)
    gd_cave_free(cave);

  // get empty caveset, using default values
  caveset = gd_caveset_data_new();

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

  native_bd_level.caveset = caveset;
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

static List *getNativeLevelReplay_BD_Ext(List *item, boolean only_successful_replays)
{
  // look for replay that was recorded for the current difficulty level
  while (item != NULL &&
	 (item->data == NULL ||
	  (((GdReplay *)item->data)->success == FALSE && only_successful_replays) ||
	  ((GdReplay *)item->data)->level != native_bd_level.level_nr))
    item = item->next;

  return item;
}

static List *getNativeLevelReplay_BD(List *replays)
{
  // 1st try: look for successful replay
  List *item = getNativeLevelReplay_BD_Ext(replays, TRUE);

  if (item != NULL)
    return item;

  // 2nd try: look for any replay
  return getNativeLevelReplay_BD_Ext(replays, FALSE);
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

  // caveset data for native levels managed separately -- do not free it!
  native_bd_level.caveset = gd_caveset_data;
  native_bd_level.loaded_from_caveset = TRUE;

  // check if this cave has any replays
  if (native_bd_level.cave->replays != NULL)
  {
    List *item = getNativeLevelReplay_BD(native_bd_level.cave->replays);

    // check if any matching replay was found
    if (item != NULL)
      native_bd_level.replay = (GdReplay *)item->data;
  }

  return TRUE;
}

boolean SaveNativeLevel_BD(char *filename)
{
  GdCave *cave = gd_cave_new_from_cave(native_bd_level.cave);

  gd_caveset_clear();
  gd_caveset = list_append(gd_caveset, cave);

  return gd_caveset_save_to_file(filename);
}

void DumpLevelset_BD(void)
{
  int num_levels_per_cave = (gd_caveset_has_levels() ? 5 : 1);

  Print("Number of levels:   %d\n", num_levels_per_cave * gd_caveset_count());
  Print("First level number: %d\n", 1);
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
  int player_fire    = ((action & JOY_BUTTON)  != 0 ? GD_REPLAY_FIRE_MASK    : 0);
  int player_suicide = ((action & KEY_SUICIDE) != 0 ? GD_REPLAY_SUICIDE_MASK : 0);

  return (player_move | player_fire | player_suicide);
}

int map_action_BD_to_RND(int action)
{
  GdDirection player_move    = ((action & GD_REPLAY_MOVE_MASK));
  boolean     player_fire    = ((action & GD_REPLAY_FIRE_MASK)    != 0);
  boolean     player_suicide = ((action & GD_REPLAY_SUICIDE_MASK) != 0);

  int action_move = (player_move == GD_MV_UP		? JOY_UP		:
		     player_move == GD_MV_UP_RIGHT	? JOY_UP   | JOY_RIGHT	:
		     player_move == GD_MV_RIGHT		?            JOY_RIGHT	:
		     player_move == GD_MV_DOWN_RIGHT	? JOY_DOWN | JOY_RIGHT	:
		     player_move == GD_MV_DOWN		? JOY_DOWN		:
		     player_move == GD_MV_DOWN_LEFT	? JOY_DOWN | JOY_LEFT	:
		     player_move == GD_MV_LEFT		?            JOY_LEFT	:
		     player_move == GD_MV_UP_LEFT	? JOY_UP   | JOY_LEFT	: JOY_NO_ACTION);
  int action_fire    = (player_fire    ? JOY_BUTTON_1 : JOY_NO_ACTION);
  int action_suicide = (player_suicide ? KEY_SUICIDE  : JOY_NO_ACTION);

  return (action_move | action_fire | action_suicide);
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

int getNonScannedElement_BD(int element)
{
  return non_scanned_pair(element);
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

void setTimeLeft_BD(void)
{
  // never change "TimeLeft" for caves without time limit
  if (game.no_level_time_limit)
    return;

  SetTimeLeft(getTimeLeft_BD());
}

void SetTimeFrames_BD(int frames_played)
{
  // needed to store final time after solving game (before counting down remaining time)
  if (game_bd.game->state_counter == GAME_INT_CAVE_RUNNING)
    game_bd.frames_played = frames_played;

}

static void UpdateGameDoorValues_BD(void)
{
  GdCave *cave = game_bd.game->cave;
  int time_left = gd_cave_time_show(cave, cave->time);
  int gems_still_needed = MAX(0, (cave->diamonds_needed - cave->diamonds_collected));

  game_bd.time_left = time_left;
  game_bd.gems_still_needed = gems_still_needed;
  game_bd.score = game_bd.game->cave_score;			// use cave score here

  if (game.LevelSolved)
  {
    // update time and score in panel while counting bonus time
    game.LevelSolved_CountingTime  = game_bd.time_left;
    game.LevelSolved_CountingScore = game_bd.global_score;	// use global score here
  }
}

Bitmap *GetColoredBitmapFromTemplate_BD(Bitmap *bitmap)
{
  return gd_get_colored_bitmap_from_template(bitmap);
}

unsigned int InitEngineRandom_BD(int seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    // get randomly selected seed to render the cave
    seed = gd_random_int_range(0, GD_CAVE_SEED_MAX);
  }

  game_bd.random_seed = seed;

  return (unsigned int)seed;
}

void InitGameEngine_BD(void)
{
  game_bd.level_solved = FALSE;
  game_bd.game_over = FALSE;
  game_bd.cover_screen = FALSE;

  gd_caveset_last_selected       = native_bd_level.cave_nr;
  gd_caveset_last_selected_level = native_bd_level.level_nr;

  if (game_bd.game != NULL)
    gd_game_free(game_bd.game);

  game_bd.game = gd_game_new(native_bd_level.cave_nr, native_bd_level.level_nr);

  game_bd.game->itercycle = 0;
  game_bd.game->itermax = 8;	// default; dynamically changed at runtime
  game_bd.game->itermax_last = game_bd.game->itermax;

  game_bd.game->use_old_engine = useOldEngine_BD();
  game_bd.game->use_krissz_engine = useKrisszEngine_BD();

  game_bd.player_moving = FALSE;
  game_bd.player_snapping = FALSE;

  // only needed for replays with random values
  if (native_bd_level.replay != NULL)
    native_bd_level.replay->current_playing_pos = 0;

  // default: start with completely covered playfield
  int next_state = GAME_INT_START_UNCOVER + 1;

  // when skipping uncovering, start with uncovered playfield
  if (setup.bd_skip_uncovering)
    next_state = GAME_INT_LOAD_CAVE + 1;

  // first iteration loads and prepares the cave (may change colors)
  play_game_func(game_bd.game, 0);

  // Krissz engine: after loading cave, fix some cave timing values
  if (game_bd.game->use_krissz_engine)
  {
    game_bd.game->cave->hatching_delay_frame++;		// add one game cycle
    game_bd.game->cave->magic_wall_time++;		// add one millisecond

    // remove time for one game cycle from cave time (or add it for caves without time limit)
    game_bd.game->cave->time += game_bd.game->cave->speed * (game.no_level_time_limit ? +1 : -1);
  }

  // fast-forward game engine to selected state (covered or uncovered)
  while (game_bd.game->state_counter < next_state)
    play_game_func(game_bd.game, 0);

  // when skipping uncovering, continue with uncovered playfield
  if (setup.bd_skip_uncovering)
    game_bd.game->state_counter = GAME_INT_UNCOVER_ALL + 1;
  else if (isLevelEditorFastStart())
    game_bd.game->state_counter = GAME_INT_UNCOVER_ALL - 8;

  if (setup.bd_skip_uncovering || isLevelEditorFastStart())
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
	    element == O_PLAYER_START ||
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

  play_game_func(game_bd.game, action[0]);

  if (setup.bd_skip_hatching && !game_bd.game->cave->hatched &&
      game_bd.game->state_counter == GAME_INT_CAVE_RUNNING)
  {
    // fast-forward game engine until player hatched
    while (!game_bd.game->cave->hatched)
    {
      // also record or replay tape action during fast-forward
      action = TapeCorrectAction_BD(action);

      play_game_func(game_bd.game, 0);
    }
  }

  // scroll without iterating engine if player out of sight (mainly due to wrap-around)
  // (this is needed to prevent broken tapes in case of viewport or tile size changes)
  while (game_bd.game->out_of_window)
  {
    RedrawPlayfield_BD(TRUE);

    BlitScreenToBitmap_BD(backbuffer);

    BackToFront();

    play_game_func(game_bd.game, action[0]);
  }

  boolean single_step_mode_paused =
    CheckSingleStepMode_BD(check_iteration_reached(game_bd.game),
                           game_bd.player_moving,
                           game_bd.player_snapping);

  // draw final movement animation frame before going to single step pause mode
  if (single_step_mode_paused)
    game_bd.game->itercycle = game_bd.game->itermax - 1;

  RedrawPlayfield_BD(FALSE);

  UpdateGameDoorValues_BD();
}


// ============================================================================
// graphics functions
// ============================================================================

// check if native BD graphics engine requested in custom graphics configuration
boolean use_native_bd_graphics_engine(void)
{
  return game.use_native_bd_graphics_engine;
}

// check if smooth game element movements selected in setup menu
boolean use_bd_smooth_movements(void)
{
  return ((setup.bd_smooth_movements == STATE_TRUE) ||
	  (setup.bd_smooth_movements == STATE_AUTO && !use_native_bd_graphics_engine()));
}

// check if player pushing graphics selected in setup menu
boolean use_bd_pushing_graphics(void)
{
  return ((setup.bd_pushing_graphics == STATE_TRUE) ||
	  (setup.bd_pushing_graphics == STATE_AUTO && !use_native_bd_graphics_engine()));
}

// check if player up/down graphics selected in setup menu
boolean use_bd_up_down_graphics(void)
{
  return ((setup.bd_up_down_graphics == STATE_TRUE) ||
	  (setup.bd_up_down_graphics == STATE_AUTO && !use_native_bd_graphics_engine()));
}

// check if element falling sounds selected in setup menu
boolean use_bd_falling_sounds(void)
{
  return ((setup.bd_falling_sounds == STATE_TRUE) ||
	  (setup.bd_falling_sounds == STATE_AUTO && game.use_native_bd_sound_engine));
}

Bitmap **GetTitleScreenBitmaps_BD(void)
{
  Bitmap **title_screen_bitmaps = gd_get_title_screen_bitmaps();

  if (title_screen_bitmaps == NULL || title_screen_bitmaps[0] == NULL)
    return NULL;

  return title_screen_bitmaps;
}

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


// ============================================================================
// snapshot functions
// ============================================================================

void SaveEngineSnapshotValues_BD(void)
{
  GdGame *game = game_bd.game;
  GdCave *cave = game_bd.game->cave;
  int x, y;

  engine_snapshot_bd.game = *game;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      engine_snapshot_bd.element_buffer[x][y]      = game->element_buffer[y][x];
      engine_snapshot_bd.last_element_buffer[x][y] = game->last_element_buffer[y][x];
      engine_snapshot_bd.drawing_buffer[x][y]      = game->drawing_buffer[y][x];
      engine_snapshot_bd.last_drawing_buffer[x][y] = game->last_drawing_buffer[y][x];
      engine_snapshot_bd.dir_buffer_from[x][y]     = game->dir_buffer_from[y][x];
      engine_snapshot_bd.dir_buffer_to[x][y]       = game->dir_buffer_to[y][x];
      engine_snapshot_bd.gfx_buffer[x][y]          = game->gfx_buffer[y][x];
      engine_snapshot_bd.covered_buffer[x][y]      = game->covered_buffer[y][x];
      engine_snapshot_bd.scanned_next[x][y]        = game->scanned_next[y][x];
    }
  }

  engine_snapshot_bd.cave = *cave;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      engine_snapshot_bd.map[x][y] = cave->map[y][x];

      if (cave->hammered_walls_reappear)
        engine_snapshot_bd.hammered_reappear[x][y] = cave->hammered_reappear[y][x];
    }
  }

  if (native_bd_level.replay != NULL)
    engine_snapshot_bd.replay.current_playing_pos = native_bd_level.replay->current_playing_pos;
}

void LoadEngineSnapshotValues_BD(void)
{
  GdGame *game = game_bd.game;
  GdCave *cave = game_bd.game->cave;
  int x, y;

  // copy pointers
  engine_snapshot_bd.game.cave                = game->cave;
  engine_snapshot_bd.game.original_cave       = game->original_cave;

  engine_snapshot_bd.game.element_buffer      = game->element_buffer;
  engine_snapshot_bd.game.last_element_buffer = game->last_element_buffer;
  engine_snapshot_bd.game.drawing_buffer      = game->drawing_buffer;
  engine_snapshot_bd.game.last_drawing_buffer = game->last_drawing_buffer;
  engine_snapshot_bd.game.dir_buffer_from     = game->dir_buffer_from;
  engine_snapshot_bd.game.dir_buffer_to       = game->dir_buffer_to;
  engine_snapshot_bd.game.gfx_buffer          = game->gfx_buffer;
  engine_snapshot_bd.game.covered_buffer      = game->covered_buffer;
  engine_snapshot_bd.game.scanned_next        = game->scanned_next;

  *game = engine_snapshot_bd.game;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      game->element_buffer[y][x]      = engine_snapshot_bd.element_buffer[x][y];
      game->last_element_buffer[y][x] = engine_snapshot_bd.last_element_buffer[x][y];
      game->drawing_buffer[y][x]      = engine_snapshot_bd.drawing_buffer[x][y];
      game->last_drawing_buffer[y][x] = engine_snapshot_bd.last_drawing_buffer[x][y];
      game->dir_buffer_from[y][x]     = engine_snapshot_bd.dir_buffer_from[x][y];
      game->dir_buffer_to[y][x]       = engine_snapshot_bd.dir_buffer_to[x][y];
      game->gfx_buffer[y][x]          = engine_snapshot_bd.gfx_buffer[x][y];
      game->covered_buffer[y][x]      = engine_snapshot_bd.covered_buffer[x][y];
      game->scanned_next[y][x]        = engine_snapshot_bd.scanned_next[x][y];
    }
  }

  // copy pointers
  engine_snapshot_bd.cave.story             = cave->story;
  engine_snapshot_bd.cave.remark            = cave->remark;
  engine_snapshot_bd.cave.tags              = cave->tags;
  engine_snapshot_bd.cave.map               = cave->map;
  engine_snapshot_bd.cave.objects           = cave->objects;
  engine_snapshot_bd.cave.replays           = cave->replays;
  engine_snapshot_bd.cave.random            = cave->random;
  engine_snapshot_bd.cave.objects_order     = cave->objects_order;
  engine_snapshot_bd.cave.hammered_reappear = cave->hammered_reappear;

  *cave = engine_snapshot_bd.cave;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      cave->map[y][x] = engine_snapshot_bd.map[x][y];

      if (cave->hammered_walls_reappear)
        cave->hammered_reappear[y][x] = engine_snapshot_bd.hammered_reappear[x][y];
    }
  }

  gd_scroll(game_bd.game, TRUE, TRUE);

  if (native_bd_level.replay != NULL)
    native_bd_level.replay->current_playing_pos = engine_snapshot_bd.replay.current_playing_pos;
}
