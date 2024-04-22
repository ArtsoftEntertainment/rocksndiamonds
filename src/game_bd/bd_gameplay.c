/*
 * Copyright (c) 2007, 2008, 2009, Czirkos Zoltan <cirix@fw.hu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "main_bd.h"


void gd_game_free(GdGame *game)
{
  // stop sounds
  gd_sound_off();

  if (game->element_buffer)
    gd_cave_map_free(game->element_buffer);
  if (game->last_element_buffer)
    gd_cave_map_free(game->last_element_buffer);
  if (game->dir_buffer)
    gd_cave_map_free(game->dir_buffer);
  if (game->gfx_buffer)
    gd_cave_map_free(game->gfx_buffer);

  game->player_lives = 0;

  if (game->cave)
    gd_cave_free(game->cave);

  free(game);
}

// add bonus life. if sound enabled, play sound, too.
static void add_bonus_life(GdGame *game, boolean inform_user)
{
  if (inform_user)
  {
    gd_sound_play_bonus_life();
    game->bonus_life_flash = 100;
  }

  // really increment number of lifes? only in a real game, nowhere else.
  if (game->player_lives &&
      game->player_lives < gd_caveset_data->maximum_lives)
  {
    // only add a life, if lives is > 0.
    // lives == 0 is a test run or a snapshot, no bonus life then.
    // also, obey max number of bonus lives.
    game->player_lives++;
  }
}

// increment score of player.
// flash screen if bonus life
static void increment_score(GdGame *game, int increment)
{
  int i;

  i = game->player_score / gd_caveset_data->bonus_life_score;
  game->player_score += increment;
  game->cave_score += increment;

  // if score crossed bonus_life_score point boundary, player won a bonus life
  if (game->player_score / gd_caveset_data->bonus_life_score > i)
    add_bonus_life(game, TRUE);
}

// do the things associated with loading a new cave. function creates gfx buffer and the like.
static void load_cave(GdGame *game)
{
  int x, y;

  // delete element buffer
  if (game->element_buffer)
    gd_cave_map_free(game->element_buffer);
  game->element_buffer = NULL;

  // delete last element buffer
  if (game->last_element_buffer)
    gd_cave_map_free(game->last_element_buffer);
  game->last_element_buffer = NULL;

  // delete direction buffer
  if (game->dir_buffer)
    gd_cave_map_free(game->dir_buffer);
  game->dir_buffer = NULL;

  // delete gfx buffer
  if (game->gfx_buffer)
    gd_cave_map_free(game->gfx_buffer);
  game->gfx_buffer = NULL;

  // load the cave
  game->cave_score = 0;

  // delete previous cave
  gd_cave_free(game->cave);

  if (native_bd_level.loaded_from_caveset)
    game->original_cave = gd_get_original_cave_from_caveset(game->cave_num);
  else
    game->original_cave = native_bd_level.cave;

  game->cave = gd_get_prepared_cave(game->original_cave, game->level_num);

  // if requested, recolor cave (cave is a copy only, so no worries)
  if (setup.bd_random_colors)
    gd_cave_set_random_colors(game->cave, setup.bd_default_color_type);

  if (game->cave->intermission && game->cave->intermission_instantlife)
    add_bonus_life(game, FALSE);

  game->milliseconds_anim = 0;
  game->milliseconds_game = 0;        // set game timer to zero, too

  // create new element buffer
  game->element_buffer = gd_cave_map_new(game->cave, int);

  for (y = 0; y < game->cave->h; y++)
    for (x = 0; x < game->cave->w; x++)
      game->element_buffer[y][x] = O_NONE;

  // create new last element buffer
  game->last_element_buffer = gd_cave_map_new(game->cave, int);

  for (y = 0; y < game->cave->h; y++)
    for (x = 0; x < game->cave->w; x++)
      game->last_element_buffer[y][x] = O_NONE;

  // create new direction buffer
  game->dir_buffer = gd_cave_map_new(game->cave, int);

  for (y = 0; y < game->cave->h; y++)
    for (x = 0; x < game->cave->w; x++)
      game->dir_buffer[y][x] = GD_MV_STILL;

  // create new gfx buffer
  game->gfx_buffer = gd_cave_map_new(game->cave, int);

  for (y = 0; y < game->cave->h; y++)
    for (x = 0; x < game->cave->w; x++)
      game->gfx_buffer[y][x] = -1;    // fill with "invalid"
}

GdCave *gd_create_snapshot(GdGame *game)
{
  GdCave *snapshot;

  if (game->cave == NULL)
    return NULL;

  // make an exact copy
  snapshot = gd_cave_new_from_cave(game->cave);

  return snapshot;
}

// this starts a new game
GdGame *gd_game_new(const int cave, const int level)
{
  GdGame *game;

  game = checked_calloc(sizeof(GdGame));

  game->cave_num = cave;
  game->level_num = level;

  game->player_lives = gd_caveset_data->initial_lives;
  game->player_score = 0;

  game->player_move = GD_MV_STILL;
  game->player_move_stick = FALSE;
  game->player_fire = FALSE;

  game->state_counter = GAME_INT_LOAD_CAVE;

  game->show_story = TRUE;

  return game;
}

static void iterate_cave(GdGame *game, GdDirection player_move, boolean fire)
{
  boolean suicide = FALSE;

  // ANYTHING EXCEPT A TIMEOUT, WE ITERATE THE CAVE
  if (game->cave->player_state != GD_PL_TIMEOUT)
  {
    if (TapeIsPlaying_ReplayBD())
    {
      byte *action_rnd = TapePlayAction_BD();

      if (action_rnd != NULL)
      {
	int action_bd = map_action_RND_to_BD(action_rnd[0]);

	player_move = (action_bd & GD_REPLAY_MOVE_MASK);
	fire        = (action_bd & GD_REPLAY_FIRE_MASK) != 0;
      }
    }

    // iterate cave
    gd_cave_iterate(game->cave, player_move, fire, suicide);

    if (game->cave->score)
      increment_score(game, game->cave->score);

    gd_sound_play_cave(game->cave);
  }

  if (game->cave->player_state == GD_PL_EXITED)
  {
    if (game->cave->intermission &&
	game->cave->intermission_rewardlife &&
	game->player_lives != 0)
    {
      // one life extra for completing intermission
      add_bonus_life(game, FALSE);
    }

    // start adding points for remaining time
    game->state_counter = GAME_INT_CHECK_BONUS_TIME;
    gd_cave_clear_sounds(game->cave);

    // play cave finished sound
    gd_sound_play(game->cave, GD_S_FINISHED, O_NONE, -1, -1);
    gd_sound_play_cave(game->cave);
  }

  if (game->cave->player_state == GD_PL_DIED ||
      game->cave->player_state == GD_PL_TIMEOUT)
  {
    game_bd.game_over = TRUE;
    game_bd.cover_screen = TRUE;
  }
}

static GdGameState gd_game_main_int(GdGame *game, boolean allow_iterate, boolean fast_forward)
{
  int millisecs_elapsed = 20;
  boolean frame;    // set to true, if this will be an animation frame
  GdGameState return_state;
  int counter_next;
  int x, y;

  counter_next = GAME_INT_INVALID;
  return_state = GD_GAME_INVALID_STATE;
  game->milliseconds_anim += millisecs_elapsed;    // keep track of time
  frame = FALSE;    // set to true, if this will be an animation frame

  if (game->milliseconds_anim >= 40)
  {
    frame = TRUE;
    game->milliseconds_anim -= 40;
  }

  // cannot be less than uncover start.
  if (game->state_counter < GAME_INT_LOAD_CAVE)
  {
    ;
  }
  else if (game->state_counter == GAME_INT_LOAD_CAVE)
  {
    // do the things associated with loading a new cave. function creates gfx buffer and the like.
    load_cave(game);

    return_state = GD_GAME_NOTHING;
    counter_next = GAME_INT_SHOW_STORY;
  }
  else if (game->state_counter == GAME_INT_SHOW_STORY)
  {
    // for normal game, every cave can have a long string of description/story. show that.

    // if we have a story...
#if 0
    if (game->show_story && game->original_cave && game->original_cave->story != NULL)
      Info("Cave Story: %s", game->original_cave->story);
#endif

    counter_next = GAME_INT_START_UNCOVER;
    return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter == GAME_INT_START_UNCOVER)
  {
    // the very beginning.

    // cover all cells of cave
    for (y = 0; y < game->cave->h; y++)
      for (x = 0; x < game->cave->w; x++)
	game->cave->map[y][x] |= COVERED;

    counter_next = game->state_counter + 1;

    // very important: tell the caller that we loaded a new cave.
    // size of the cave might be new, colors might be new, and so on.
    return_state = GD_GAME_CAVE_LOADED;
  }
  else if (game->state_counter < GAME_INT_UNCOVER_ALL)
  {
    // uncover animation

    // to play cover sound
    gd_sound_play(game->cave, GD_S_COVERING, O_COVERED, -1, -1);
    gd_sound_play_cave(game->cave);

    counter_next = game->state_counter;

    if (frame)
    {
      int j;

      // original game uncovered one cell per line each frame.
      // we have different cave sizes, so uncover width * height / 40 random
      // cells each frame. (original was width = 40).
      // this way the uncovering is the same speed also for intermissions.
      for (j = 0; j < game->cave->w * game->cave->h / 40; j++)
      {
	y = gd_random_int_range(0, game->cave->h);
	x = gd_random_int_range(0, game->cave->w);

	game->cave->map[y][x] &= ~COVERED;
      }

      counter_next++;    // as we did something, advance the counter.
    }

    return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter == GAME_INT_UNCOVER_ALL)
  {
    // time to uncover the whole cave.
    for (y = 0; y < game->cave->h; y++)
      for (x = 0; x < game->cave->w; x++)
	game->cave->map[y][x] &= ~COVERED;

    // to stop uncover sound.
    gd_cave_clear_sounds(game->cave);
    gd_sound_play_cave(game->cave);

    counter_next = GAME_INT_CAVE_RUNNING;
    return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter == GAME_INT_CAVE_RUNNING)
  {
    // normal.
    int cavespeed;

    if (!fast_forward)
      cavespeed = game->cave->speed;   // cave speed in ms, like 175ms/frame
    else
      cavespeed = 40;    // if fast forward, ignore cave speed, and go as 25 iterations/sec

    // ITERATION - cave is running.

    // normally nothing happes. but if we iterate, this might change.
    return_state = GD_GAME_NOTHING;

    // if allowing cave movements, add elapsed time to timer. and then we can check what to do.
    if (allow_iterate)
      game->milliseconds_game += millisecs_elapsed;

    // increment cycle (frame) counter for the current cave iteration
    game->itercycle++;

    if (game->milliseconds_game >= cavespeed)
    {
      GdPlayerState pl;

      game->milliseconds_game -= cavespeed;
      pl = game->cave->player_state;

      // initialize buffers for last cave element and direction for next iteration
      for (y = 0; y < game->cave->h; y++)
      {
	for (x = 0; x < game->cave->w; x++)
	{
	  game->last_element_buffer[y][x] = game->element_buffer[y][x] & ~SKIPPED;
	  game->dir_buffer[y][x] = GD_MV_STILL;
	}
      }

      // store last maximum number of cycles (to force redraw if changed)
      game->itermax_last = game->itermax;

      // update maximum number of cycles (frame) per cave iteration
      game->itermax = game->itercycle;

      // reset cycle (frame) counter for the next cave iteration
      game->itercycle = 0;

      iterate_cave(game, game->player_move, game->player_fire);

      if (game->player_move == GD_MV_STILL)
      {
	game->player_move_stick = FALSE;
      }
      else
      {
	game->player_move_stick = TRUE;
	game->player_move = GD_MV_STILL;
      }

      // as we iterated, the score and the like could have been changed.
      return_state = GD_GAME_LABELS_CHANGED;

      // and if the cave timeouted at this moment, that is a special case.
      if (pl != GD_PL_TIMEOUT && game->cave->player_state == GD_PL_TIMEOUT)
	return_state = GD_GAME_TIMEOUT_NOW;
    }

    // do not change counter state, as it is set by iterate_cave()
    counter_next = game->state_counter;
  }
  else if (game->state_counter == GAME_INT_CHECK_BONUS_TIME)
  {
    // before covering, we check for time bonus score
    if (frame)
    {
      // if time remaining, bonus points are added. do not start animation yet.
      if (game->cave->time > 0)
      {
        // subtract number of "milliseconds" - nothing to do with gameplay->millisecs!
	game->cave->time -= game->cave->timing_factor;

	// higher levels get more bonus points per second remained
	increment_score(game, game->cave->timevalue);

	// if much time (> 60s) remained, fast counter :)
	if (game->cave->time > 60 * game->cave->timing_factor)
	{
	  // decrement by nine each frame, so it also looks like a fast counter. 9 is 8 + 1!
	  game->cave->time -= 8 * game->cave->timing_factor;
	  increment_score(game, game->cave->timevalue * 8);
	}

	// just to be neat
	if (game->cave->time < 0)
	  game->cave->time = 0;

	counter_next = game->state_counter;    // do not change yet
      }
      else
      {
	game_bd.level_solved = TRUE;
	game_bd.cover_screen = TRUE;

	// if no more points, start waiting a bit, and later start covering.
	counter_next = GAME_INT_WAIT_BEFORE_COVER;
      }

      if (game->cave->time / game->cave->timing_factor > 8)
	gd_sound_play(game->cave, GD_S_FINISHED, O_NONE, -1, -1); // play cave finished sound

      // play bonus sound
      gd_cave_set_seconds_sound(game->cave);
      gd_sound_play_cave(game->cave);

      return_state = GD_GAME_LABELS_CHANGED;
    }
    else
    {
      return_state = GD_GAME_NOTHING;

      // do not change counter state, as it is set by iterate_cave()
      counter_next = game->state_counter;
    }
  }
  else if (game->state_counter == GAME_INT_WAIT_BEFORE_COVER)
  {
    // after adding bonus points, we wait some time before starting to cover.
    // this is the FIRST frame... so we check for game over and maybe jump there
    // if no more lives, game is over.
    counter_next = game->state_counter;

    if (game->player_lives == 0)
      return_state = GD_GAME_NO_MORE_LIVES;
    else
      return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter > GAME_INT_WAIT_BEFORE_COVER &&
	   game->state_counter < GAME_INT_COVER_START)
  {
    // after adding bonus points, we wait some time before starting to cover.
    // ... and the other frames.
    counter_next = game->state_counter;
    if (frame)
      counter_next++;    // 40 ms elapsed, advance counter

    return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter == GAME_INT_COVER_START)
  {
    // starting to cover. start cover sound.

    gd_cave_clear_sounds(game->cave);
    gd_sound_play(game->cave, GD_S_COVERING, O_COVERED, -1, -1);

    // to play cover sound
    gd_sound_play_cave(game->cave);

    counter_next = game->state_counter + 1;
    return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter > GAME_INT_COVER_START &&
	   game->state_counter < GAME_INT_COVER_ALL)
  {
    // covering.
    gd_sound_play(game->cave, GD_S_COVERING, O_COVERED, -1, -1);

    counter_next = game->state_counter;

    if (frame)
    {
      int j;

      counter_next++;    // 40 ms elapsed, doing cover: advance counter

      // covering eight times faster than uncovering.
      for (j = 0; j < game->cave->w * game->cave->h * 8 / 40; j++)
	game->cave->map[gd_random_int_range(0, game->cave->h)][gd_random_int_range (0, game->cave->w)] |= COVERED;
    }

    return_state = GD_GAME_NOTHING;
  }
  else if (game->state_counter == GAME_INT_COVER_ALL)
  {
    // cover all
    for (y = 0; y < game->cave->h; y++)
      for (x = 0; x < game->cave->w; x++)
	game->cave->map[y][x] |= COVERED;

    counter_next = game->state_counter + 1;
    return_state = GD_GAME_NOTHING;

    // to stop uncover sound.
    gd_cave_clear_sounds(game->cave);
    gd_sound_play_cave(game->cave);
  }
  else
  {
    // cover all + 1

    if (game->player_lives != 0)
      return_state = GD_GAME_NOTHING;    // and go to next level
    else
      return_state = GD_GAME_GAME_OVER;
  }

  // draw the cave
  if (frame)
  {
    if (game->bonus_life_flash)    // bonus life - frames
      game->bonus_life_flash--;

    game->animcycle = (game->animcycle + 1) % 8;
  }

  // always render the cave to the gfx buffer;
  // however it may do nothing if animcycle was not changed.
  if (game->element_buffer && game->gfx_buffer)
    gd_drawcave_game(game->cave, game->element_buffer, game->last_element_buffer, game->gfx_buffer,
		     game->bonus_life_flash != 0, game->animcycle, setup.bd_show_invisible_outbox);

  game->state_counter = counter_next;

  return return_state;
}

void play_game_func(GdGame *game, int action)
{
  GdGameState state;
  boolean move_up    = ((action & JOY_UP)    != 0);
  boolean move_down  = ((action & JOY_DOWN)  != 0);
  boolean move_left  = ((action & JOY_LEFT)  != 0);
  boolean move_right = ((action & JOY_RIGHT) != 0);
  boolean fire  = ((action & (JOY_BUTTON_1 | JOY_BUTTON_2)) != 0);

  if (game->player_move_stick || move_up || move_down || move_left || move_right) // no "fire"!
  {
    // get movement
    game->player_move = gd_direction_from_keypress(move_up, move_down, move_left, move_right);

    // when storing last action, only update fire action with direction
    // (prevents clearing direction if snapping stopped before action is performed)
    game->player_fire = fire;
  }

  // if no direction was stored before, allow setting fire to current state
  if (game->player_move == GD_MV_STILL)
    game->player_fire = fire;

  // tell the interrupt "20ms has passed"
  state = gd_game_main_int(game, !game->out_of_window, gd_keystate[SDL_SCANCODE_F]);

  // state of game, returned by gd_game_main_int
  switch (state)
  {
    case GD_GAME_CAVE_LOADED:
      // scroll to start position
      gd_scroll_to_origin();

      // fill whole screen with black - cave might be smaller than previous!
      FillRectangle(gd_screen_bitmap, 0, 0, SXSIZE, SYSIZE, BLACK_PIXEL);
      break;

    default:
      break;
  }

  // for the sdl version, it seems nicer if we first scroll, and then draw.
  // scrolling for the sdl version will merely invalidate the whole gfx buffer.
  // if drawcave was before scrolling, it would draw, scroll would invalidate,
  // and then it should be drawn again
  // only do the drawing if the cave already exists.
  if (game->cave && game->element_buffer && game->gfx_buffer)
  {
    // if fine scrolling, scroll at 50hz. if not, only scroll at every second call, so 25hz.
    // do the scrolling. scroll exactly, if player is not yet alive
    game->out_of_window = gd_scroll(game, game->cave->player_state == GD_PL_NOT_YET, FALSE);
  }
}
