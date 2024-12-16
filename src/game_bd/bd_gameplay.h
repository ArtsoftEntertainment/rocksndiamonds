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

#ifndef BD_GAMEPLAY_H
#define BD_GAMEPLAY_H

#include "bd_cave.h"


#define GAME_INT_INVALID		-100

// prepare cave, gfx buffer
#define GAME_INT_LOAD_CAVE		-73

// show description/note of cave.
#define GAME_INT_SHOW_STORY		-72

// waiting fire button after showing the story.
#define GAME_INT_SHOW_STORY_WAIT	-71

// start uncovering
#define GAME_INT_START_UNCOVER		-70

// ...70 frames until full uncover...
#define GAME_INT_UNCOVER_ALL		-1

// normal running state.
#define GAME_INT_CAVE_RUNNING		0

// add points for remaining time
#define GAME_INT_CHECK_BONUS_TIME	1

// ...2..99 = wait and do nothing, after adding time
#define GAME_INT_WAIT_BEFORE_COVER	2

// start covering
#define GAME_INT_COVER_START		100

// ... 8 frames of cover animation
#define GAME_INT_COVER_ALL		108


typedef struct _gd_game
{
  GdString player_name;         // Name of player
  int player_score;             // Score of player
  int player_lives;             // Remaining lives of player

  GdDirection player_move;
  boolean player_move_stick;
  boolean player_fire;
  boolean player_suicide;
  boolean player_suicide_stick;

  GdCave *cave;                 // Copy of the cave. This is the iterated, changed (ruined...) one
  GdCave *original_cave;        // original cave from caveset. used to record highscore

  boolean out_of_window;        // will be set to true, if player is not visible in the window,
                                // and we have to wait for scrolling

  int cave_num;                 // actual playing cave number
  int cave_score;               // score collected in this cave
  int level_num;                // actual playing level
  int bonus_life_flash;         // different kind of flashing, for bonus life

  int state_counter;            // counter used to control the game flow, rendering of caves
  int **element_buffer;
  int **last_element_buffer;
  int **drawing_buffer;
  int **last_drawing_buffer;
  int **dir_buffer_from;
  int **dir_buffer_to;
  int **gfx_buffer;             // contains the indexes to the cells;
                                // created by *start_level, deleted by *stop_game
  int **covered_buffer;
  int itercycle;
  int itermax;
  int itermax_last;
  int animcycle;
  int milliseconds_game;
  int milliseconds_anim;

  int replay_no_more_movements;
  boolean show_story;           // to remember that story for a particular cave was already shown.

  boolean use_old_engine;	// for game engine compatibility with old replays
} GdGame;

typedef enum _gd_game_state
{
  GD_GAME_INVALID_STATE,
  GD_GAME_SHOW_STORY,
  GD_GAME_SHOW_STORY_WAIT,
  GD_GAME_CAVE_LOADED,
  GD_GAME_NOTHING,
  GD_GAME_LABELS_CHANGED,
  GD_GAME_TIMEOUT_NOW,          // this signals the moment of time out
  GD_GAME_NO_MORE_LIVES,
  GD_GAME_STOP,
  GD_GAME_GAME_OVER,
} GdGameState;

GdCave *gd_create_snapshot(GdGame *gameplay);

void gd_game_free(GdGame *gameplay);
GdGame *gd_game_new(const int cave, const int level);
GdGame *gd_game_new_snapshot(GdCave *snapshot);
GdGame *gd_game_new_test(GdCave *cave, int level);

boolean check_iteration_reached(GdGame *game);

void play_game_func(GdGame *game, int action);

#endif	// BD_GAMEPLAY_H
