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


// for gravity and other routines.
// these arrays contain the rotated directions.
// ccw eighth: counter-clockwise, 1/8 turn (45 degrees)
// cw fourth: clockwise, 1/4 turn (90 degrees)
static const GdDirection ccw_eighth[] =
{
  GD_MV_STILL,
  GD_MV_UP_LEFT,
  GD_MV_UP,
  GD_MV_UP_RIGHT,
  GD_MV_RIGHT,
  GD_MV_DOWN_RIGHT,
  GD_MV_DOWN,
  GD_MV_DOWN_LEFT,
  GD_MV_LEFT
};

static const GdDirection ccw_fourth[] =
{
  GD_MV_STILL,
  GD_MV_LEFT,
  GD_MV_UP_LEFT,
  GD_MV_UP,
  GD_MV_UP_RIGHT,
  GD_MV_RIGHT,
  GD_MV_DOWN_RIGHT,
  GD_MV_DOWN,
  GD_MV_DOWN_LEFT
};

static const GdDirection cw_eighth[] =
{
  GD_MV_STILL,
  GD_MV_UP_RIGHT,
  GD_MV_RIGHT,
  GD_MV_DOWN_RIGHT,
  GD_MV_DOWN,
  GD_MV_DOWN_LEFT,
  GD_MV_LEFT,
  GD_MV_UP_LEFT,
  GD_MV_UP
};

static const GdDirection cw_fourth[] =
{
  GD_MV_STILL,
  GD_MV_RIGHT,
  GD_MV_DOWN_RIGHT,
  GD_MV_DOWN,
  GD_MV_DOWN_LEFT,
  GD_MV_LEFT,
  GD_MV_UP_LEFT,
  GD_MV_UP,
  GD_MV_UP_RIGHT
};

// 180 degrees turn of a direction
static const GdDirection opposite[] =
{
  GD_MV_STILL,
  GD_MV_DOWN,
  GD_MV_DOWN_LEFT,
  GD_MV_LEFT,
  GD_MV_UP_LEFT,
  GD_MV_UP,
  GD_MV_UP_RIGHT,
  GD_MV_RIGHT,
  GD_MV_DOWN_RIGHT
};

// doubling a direction (e.g. right = 1, 0   2x right = 2, 0
static const GdDirection twice[] =
{
  GD_MV_STILL,
  GD_MV_UP_2,
  GD_MV_UP_RIGHT_2,
  GD_MV_RIGHT_2,
  GD_MV_DOWN_RIGHT_2,
  GD_MV_DOWN_2,
  GD_MV_DOWN_LEFT_2,
  GD_MV_LEFT_2,
  GD_MV_UP_LEFT_2
};

// returns true if the element has a certain property
static inline boolean has_property(int element, const int property)
{
  if (game_bd.game->use_old_engine)
    element = non_scanned_pair(element);

  return (gd_element_properties[element].properties & property) != 0;
}

// returns true if the element can fall
static inline boolean el_can_fall(const int element)
{
  return has_property(element, P_CAN_FALL);
}

// returns true if the element is diggable
static inline boolean el_diggable(const int element)
{
  return has_property(element, P_DIGGABLE);
}

// returns true if the element can smash the player
static inline boolean el_can_smash_player(const int element)
{
  return (el_can_fall(element) && !el_diggable(element));
}

// play sound of a given element.
static void play_sound_of_element(GdCave *cave, GdElement element, int x, int y)
{
  // check if sound should be skipped for falling elements (and only be played on impact)
  if (el_can_fall(element) && !use_bd_falling_sounds())
    return;

  // stone and diamond fall sounds.
  switch (element)
  {
    case O_NUT:
      gd_sound_play(cave, GD_S_NUT_FALLING, element, x, y);
      break;

    case O_NUT_F:
      gd_sound_play(cave, GD_S_NUT_IMPACT, element, x, y);
      break;

    case O_STONE:
      gd_sound_play(cave, GD_S_STONE_FALLING, element, x, y);
      break;

    case O_STONE_F:
      gd_sound_play(cave, GD_S_STONE_IMPACT_RANDOM, element, x, y);
      break;

    case O_FLYING_STONE:
      gd_sound_play(cave, GD_S_FLYING_STONE_FALLING, element, x, y);
      break;

    case O_FLYING_STONE_F:
      gd_sound_play(cave, GD_S_FLYING_STONE_IMPACT, element, x, y);
      break;

    case O_MEGA_STONE:
      gd_sound_play(cave, GD_S_MEGA_STONE_FALLING, element, x, y);
      break;

    case O_MEGA_STONE_F:
      gd_sound_play(cave, GD_S_MEGA_STONE_IMPACT_RANDOM, element, x, y);
      break;

    case O_LIGHT_STONE:
      gd_sound_play(cave, GD_S_LIGHT_STONE_FALLING, element, x, y);
      break;

    case O_LIGHT_STONE_F:
      gd_sound_play(cave, GD_S_LIGHT_STONE_IMPACT_RANDOM, element, x, y);
      break;

    case O_NITRO_PACK:
      gd_sound_play(cave, GD_S_NITRO_PACK_FALLING, element, x, y);
      break;

    case O_NITRO_PACK_F:
      gd_sound_play(cave, GD_S_NITRO_PACK_IMPACT, element, x, y);
      break;

    case O_FALLING_WALL:
      gd_sound_play(cave, GD_S_FALLING_WALL_FALLING, element, x, y);
      break;

    case O_FALLING_WALL_F:
      gd_sound_play(cave, GD_S_FALLING_WALL_IMPACT, element, x, y);
      break;

    case O_H_EXPANDING_WALL:
    case O_V_EXPANDING_WALL:
    case O_EXPANDING_WALL:
    case O_H_EXPANDING_STEEL_WALL:
    case O_V_EXPANDING_STEEL_WALL:
    case O_EXPANDING_STEEL_WALL:
      gd_sound_play(cave, GD_S_EXPANDING_WALL, element, x, y);
      break;

    case O_DIAMOND:
      gd_sound_play(cave, GD_S_DIAMOND_FALLING_RANDOM, element, x, y);
      break;

    case O_DIAMOND_F:
      gd_sound_play(cave, GD_S_DIAMOND_IMPACT_RANDOM, element, x, y);
      break;

    case O_FLYING_DIAMOND:
      gd_sound_play(cave, GD_S_FLYING_DIAMOND_FALLING_RANDOM, element, x, y);
      break;

    case O_FLYING_DIAMOND_F:
      gd_sound_play(cave, GD_S_FLYING_DIAMOND_IMPACT_RANDOM, element, x, y);
      break;

    case O_BLADDER_SPENDER:
      gd_sound_play(cave, GD_S_BLADDER_SPENDER, element, x, y);
      break;

    case O_PRE_CLOCK_1:
      gd_sound_play(cave, GD_S_BLADDER_CONVERTING, element, x, y);
      break;

    case O_SLIME:
      gd_sound_play(cave, GD_S_SLIME, element, x, y);
      break;

    case O_LAVA:
      gd_sound_play(cave, GD_S_LAVA, element, x, y);
      break;

    case O_ACID:
      gd_sound_play(cave, GD_S_ACID_SPREADING, element, x, y);
      break;

    case O_BLADDER:
      gd_sound_play(cave, GD_S_BLADDER_MOVING, element, x, y);
      break;

    case O_BITER_1:
    case O_BITER_2:
    case O_BITER_3:
    case O_BITER_4:
      gd_sound_play(cave, GD_S_BITER_EATING, O_BITER, x, y);
      break;

    case O_DIRT_BALL:
      gd_sound_play(cave, GD_S_DIRT_BALL_FALLING, element, x, y);
      break;

    case O_DIRT_BALL_F:
      gd_sound_play(cave, GD_S_DIRT_BALL_IMPACT, element, x, y);
      break;

    case O_DIRT_LOOSE:
      gd_sound_play(cave, GD_S_DIRT_LOOSE_FALLING, element, x, y);
      break;

    case O_DIRT_LOOSE_F:
      gd_sound_play(cave, GD_S_DIRT_LOOSE_IMPACT, element, x, y);
      break;

    default:
      // do nothing.
      break;
  }
}

// play sound of given element being pushed.
static void play_sound_of_element_pushing(GdCave *cave, GdElement element, int x, int y)
{
  switch (element)
  {
    case O_NUT:
      gd_sound_play(cave, GD_S_NUT_PUSHING, element, x, y);
      break;

    case O_STONE:
      gd_sound_play(cave, GD_S_STONE_PUSHING, element, x, y);
      break;

    case O_FLYING_STONE:
      gd_sound_play(cave, GD_S_FLYING_STONE_PUSHING, element, x, y);
      break;

    case O_MEGA_STONE:
      gd_sound_play(cave, GD_S_MEGA_STONE_PUSHING, element, x, y);
      break;

    case O_LIGHT_STONE:
      gd_sound_play(cave, GD_S_LIGHT_STONE_PUSHING, element, x, y);
      break;

    case O_WAITING_STONE:
      gd_sound_play(cave, GD_S_WAITING_STONE_PUSHING, element, x, y);
      break;

    case O_CHASING_STONE:
      gd_sound_play(cave, GD_S_CHASING_STONE_PUSHING, element, x, y);
      break;

    case O_NITRO_PACK:
      gd_sound_play(cave, GD_S_NITRO_PACK_PUSHING, element, x, y);
      break;

    case O_BLADDER:
      gd_sound_play(cave, GD_S_BLADDER_PUSHING, element, x, y);
      break;

    default:
      // do nothing.
      break;
  }
}

// sets timeout sound.
void gd_cave_set_seconds_sound(GdCave *cave)
{
  if (game_bd.game == NULL)
    return;

  // when not counting bonus time, timeout sounds will be played by main game engine;
  // also skip timeout sounds when not using native sound engine
  if (game_bd.game->state_counter != GAME_INT_CHECK_BONUS_TIME ||
      !game.use_native_bd_bonus_jingle)
  {
    // needed to play sound during last seconds of counting bonus time with non-native sounds
    if (game_bd.game->state_counter == GAME_INT_CHECK_BONUS_TIME &&
        !game.use_native_bd_sound_engine)
      gd_sound_play(cave, GD_S_FINISHED, O_NONE, -1, -1);

    return;
  }

  // when counting bonus time, stop already playing "timeout" sounds to prevent sound jam
  if (game_bd.game->state_counter == GAME_INT_CHECK_BONUS_TIME && !game.use_native_bd_sound_engine)
  {
    int i;

    for (i = GD_S_TIMEOUT_10; i < GD_S_TIMEOUT_0; i++)
      gd_sound_stop(cave, i, O_NONE);
  }

  // this is an integer division, so 0 seconds can be 0.5 seconds...
  // also, when this reaches 8, the player still has 8.9999 seconds.
  // so the sound is played at almost t = 9s.
  switch (cave->time / cave->timing_factor)
  {
    case 9: gd_sound_play(cave, GD_S_TIMEOUT_10, O_NONE, -1, -1); break;
    case 8: gd_sound_play(cave, GD_S_TIMEOUT_9,  O_NONE, -1, -1); break;
    case 7: gd_sound_play(cave, GD_S_TIMEOUT_8,  O_NONE, -1, -1); break;
    case 6: gd_sound_play(cave, GD_S_TIMEOUT_7,  O_NONE, -1, -1); break;
    case 5: gd_sound_play(cave, GD_S_TIMEOUT_6,  O_NONE, -1, -1); break;
    case 4: gd_sound_play(cave, GD_S_TIMEOUT_5,  O_NONE, -1, -1); break;
    case 3: gd_sound_play(cave, GD_S_TIMEOUT_4,  O_NONE, -1, -1); break;
    case 2: gd_sound_play(cave, GD_S_TIMEOUT_3,  O_NONE, -1, -1); break;
    case 1: gd_sound_play(cave, GD_S_TIMEOUT_2,  O_NONE, -1, -1); break;
    case 0: gd_sound_play(cave, GD_S_TIMEOUT_1,  O_NONE, -1, -1); break;
  }
}

// Krissz engine: get new random value from random number generator (for playing)
static int get_krissz_new_random(GdCave *cave, int max_value)
{
  return gd_rand_int_range(cave->random, 0, max_value) + 1;
}

// Krissz engine: get next random value from list of stored random numbers (for replaying)
static int get_krissz_next_replay_random(void)
{
  if (native_bd_level.replay->current_playing_pos < native_bd_level.replay->randoms->len)
    return native_bd_level.replay->randoms->data[native_bd_level.replay->current_playing_pos++];

  return 0;
}

// Krissz engine: get random number
static int get_krissz_random(GdCave *cave, int max_value)
{
  // get random value either from random number list or from random number generator
  if (game_bd.game->use_krissz_engine && TapeIsPlaying_ReplayBD() &&
      native_bd_level.replay != NULL && native_bd_level.replay->randoms->len > 0)
  {
    return get_krissz_next_replay_random();
  }
  else
  {
    return get_krissz_new_random(cave, max_value);
  }
}

// Krissz engine: check if amoeba is growing
static boolean krissz_amoeba_grows(GdCave *cave)
{
  return (get_krissz_random(cave, 1000000 / cave->amoeba_growth_prob) == 1);
}

static inline int getx(const GdCave *cave, const int x, const int y)
{
  return cave->getx(cave, x, y);
}

static inline int gety(const GdCave *cave, const int x, const int y)
{
  return cave->gety(cave, x, y);
}

// perfect (non-lineshifting) GET x/y functions; returns range corrected x/y position
static inline int getx_perfect(const GdCave *cave, const int x, const int y)
{
  // dirty workaround: if closed border, force previous position instead of wrapping around
  if (!cave->open_borders_horizontal)
    return MIN(MAX(0, x), cave->w - 1);

  return (x + cave->w) % cave->w;
}

static inline int gety_perfect(const GdCave *cave, const int x, const int y)
{
  // dirty workaround: if closed border, force previous position instead of wrapping around
  if (!cave->open_borders_vertical)
    return MIN(MAX(0, y), cave->h - 1);

  return (y + cave->h) % cave->h;
}

// line shifting GET x/y function; returns range corrected x/y position
static inline int getx_shift(const GdCave *cave, int x, int y)
{
  // dirty workaround: if closed border, force previous position instead of wrapping around
  if (!cave->open_borders_horizontal)
    return MIN(MAX(0, x), cave->w - 1);

  return (x + cave->w) % cave->w;
}

static inline int gety_shift(const GdCave *cave, int x, int y)
{
  // dirty workaround: if closed border, force previous position instead of wrapping around
  if (!cave->open_borders_vertical)
    return MIN(MAX(0, y), cave->h - 1);

  return ((x < 0 ? y - 1 : x >= cave->w ? y + 1 : y) + cave->h) % cave->h;
}

static inline GdElement *getp(const GdCave *cave, const int x, const int y)
{
  return cave->getp(cave, x, y);
}

// perfect (non-lineshifting) GET function.
// returns a pointer to a selected cave element by its coordinates.
static inline GdElement *getp_perfect(const GdCave *cave, const int x, const int y)
{
  // if closed border, return steel wall element instead of wrapped around playfield element
  if (((x < 0 || x >= cave->w) && !cave->open_borders_horizontal) ||
      ((y < 0 || y >= cave->h) && !cave->open_borders_vertical))
  {
    static GdElement steel = O_STEEL;	// return code must be pointer to element

    return &steel;
  }

  // (x + n) mod n: this works also for x >= n and -n + 1 < x < 0
  return &(cave->map[(y + cave->h) % cave->h][(x + cave->w) % cave->w]);
}

// line shifting GET function; returns a pointer to the selected cave element.
// this is used to emulate the line-shifting behaviour of original games, so that
// the player entering one side will appear one row above or below on the other.
static inline GdElement *getp_shift(const GdCave *cave, int x, int y)
{
  // if closed border, return steel wall element instead of wrapped around playfield element
  if (((x < 0 || x >= cave->w) && !cave->open_borders_horizontal) ||
      ((y < 0 || y >= cave->h) && !cave->open_borders_vertical))
  {
    static GdElement steel = O_STEEL;	// return code must be pointer to element

    return &steel;
  }

  if (x >= cave->w)
  {
    y++;
    x -= cave->w;
  }
  else if (x < 0)
  {
    y--;
    x += cave->w;
  }

  y = (y + cave->h) % cave->h;

  return &(cave->map[y][x]);
}

// returns the element at (x, y)
static inline GdElement get(const GdCave *cave, const int x, const int y)
{
  return *getp(cave, x, y);
}

// returns the element at (x, y) + dir
static inline GdElement get_dir(const GdCave *cave, const int x, const int y,
				const GdDirection dir)
{
  return get(cave, x + gd_dx[dir], y + gd_dy[dir]);
}

// returns true if the element at (x, y) + dir explodes if hit by a stone (for example, a firefly)
static inline boolean explodes_by_hit(const GdCave *cave, const int x,
                                      const int y, GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_EXPLODES_BY_HIT);
}

// returns true if the element at (x, y) is not explodable (for example, the steel wall)
static inline boolean non_explodable(const GdCave *cave, const int x, const int y)
{
  return has_property(get(cave, x, y), P_NON_EXPLODABLE);
}

// returns true if the element at (x, y) + dir can be eaten by the amoeba (space and dirt)
static inline boolean amoeba_eats(const GdCave *cave, const int x, const int y,
                                  const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_AMOEBA_CONSUMES);
}

// Returns true if the element is sloped, so stones and diamonds roll down on it.
// For example a stone or brick wall.
// Some elements can be sloped in specific directions only; for example a wall
// like /| is sloped from the up to the left.
//
// @param x The x coordinate
// @param y The y coordinate
// @param dir The coordinate to move from (x, y), e.g. element at (x, y) + dir is checked.
// @param slop The direction in which the element should be sloped.
static inline boolean sloped(const GdCave *cave, const int x, const int y,
                             const GdDirection dir, const GdDirection slop)
{
  switch (slop)
  {
    case GD_MV_LEFT:
      return has_property(get_dir(cave, x, y, dir), P_SLOPED_LEFT);

    case GD_MV_RIGHT:
      return has_property(get_dir(cave, x, y, dir), P_SLOPED_RIGHT);

    case GD_MV_UP:
      return has_property(get_dir(cave, x, y, dir), P_SLOPED_UP);

    case GD_MV_DOWN:
      return has_property(get_dir(cave, x, y, dir), P_SLOPED_DOWN);

    default:
      break;
  }

  return FALSE;
}

// returns true if the element is sloped for bladder movement
// (brick = yes, diamond = no, for example)
static inline boolean sloped_for_bladder(const GdCave *cave, const int x, const int y,
                                         const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_BLADDER_SLOPED);
}

// returns true if the element at (x, y) + dir can blow up a fly by touching it.
static inline boolean blows_up_flies(const GdCave *cave, const int x, const int y,
                                     const GdDirection dir)
{
  // Krissz engine: newly created (scanned) amoeba blows up flies, too
  if (game_bd.game->use_krissz_engine && get_dir(cave, x, y, dir) == O_AMOEBA_scanned)
    return TRUE;

  return has_property(get_dir(cave, x, y, dir), P_BLOWS_UP_FLIES);
}

// returns true if the element is a counter-clockwise creature
static inline boolean rotates_ccw(const GdCave *cave, const int x, const int y)
{
  return has_property(get(cave, x, y), P_CCW);
}

// returns true if the element is a player (normal player, player glued, player with bomb)
boolean is_player(const GdCave *cave, const int x, const int y)
{
  return has_property(get(cave, x, y), P_PLAYER);
}

// returns true if the element is a player stirring the pot
boolean is_player_stirring(const GdCave *cave, const int x, const int y)
{
  return has_property(get(cave, x, y), P_PLAYER_STIRRING);
}

// returns true if the element is a player (normal player, player glued, player with bomb) (+ dir)
static inline boolean is_player_dir(const GdCave *cave, const int x, const int y,
				    const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_PLAYER);
}

// returns true if the element at (x, y) + dir can be hammered.
static inline boolean can_be_hammered(const GdCave *cave, const int x, const int y,
                                      const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_CAN_BE_HAMMERED);
}

// returns true if the element at (x, y) + dir can be pushed.
boolean can_be_pushed(const GdCave *cave, const int x, const int y,
                      const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_PUSHABLE);
}

// returns true if the element at (x, y) is the first animation stage of an explosion
static inline boolean is_first_stage_of_explosion(const GdCave *cave, const int x, const int y)
{
  return has_property(get(cave, x, y), P_EXPLOSION_FIRST_STAGE);
}

// returns true if the element sits on and is moved by the conveyor belt
static inline boolean moved_by_conveyor_top(const GdCave *cave, const int x, const int y,
                                            const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_MOVED_BY_CONVEYOR_TOP);
}

// returns true if the element floats upwards and is moved by the conveyor belt which is OVER it
static inline boolean moved_by_conveyor_bottom(const GdCave *cave, const int x, const int y,
                                               const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_MOVED_BY_CONVEYOR_BOTTOM);
}

// returns true if element can be moved by conveyor belt (above or below)
boolean moved_by_conveyor(const GdCave *cave, const int x, const int y)
{
  int element      = get(cave, x, y);
  int element_up   = get_dir(cave, x, y, GD_MV_UP);
  int element_down = get_dir(cave, x, y, GD_MV_DOWN);
  boolean moved_by_conveyor_top    = has_property(element, P_MOVED_BY_CONVEYOR_TOP);
  boolean moved_by_conveyor_bottom = has_property(element, P_MOVED_BY_CONVEYOR_BOTTOM);
  boolean conveyor_up   = (element_up   == O_CONVEYOR_LEFT || element_up   == O_CONVEYOR_RIGHT);
  boolean conveyor_down = (element_down == O_CONVEYOR_LEFT || element_down == O_CONVEYOR_RIGHT);

  return (!cave->gravity_disabled && cave->conveyor_belts_active &&
          ((cave->gravity == GD_MV_DOWN && ((conveyor_down && moved_by_conveyor_top) ||
                                            (conveyor_up   && moved_by_conveyor_bottom))) ||
           (cave->gravity == GD_MV_UP   && ((conveyor_up   && moved_by_conveyor_top) ||
                                            (conveyor_down && moved_by_conveyor_bottom)))));
}

// returns true if moving element is moved by conveyor belt in same direction (above or below)
boolean moved_by_conveyor_dir(const GdCave *cave, const int x, const int y, const GdDirection dir)
{
  int element_dir = (cave->conveyor_belts_direction_changed ? opposite[dir] : dir);
  int element      = get(cave, x, y);
  int element_up   = get_dir(cave, x, y, GD_MV_UP);
  int element_down = get_dir(cave, x, y, GD_MV_DOWN);
  boolean moved_by_conveyor_top    = has_property(element, P_MOVED_BY_CONVEYOR_TOP);
  boolean moved_by_conveyor_bottom = has_property(element, P_MOVED_BY_CONVEYOR_BOTTOM);
  boolean conveyor_up   = ((element_up   == O_CONVEYOR_LEFT  && element_dir == GD_MV_RIGHT) ||
                           (element_up   == O_CONVEYOR_RIGHT && element_dir == GD_MV_LEFT));
  boolean conveyor_down = ((element_down == O_CONVEYOR_LEFT  && element_dir == GD_MV_LEFT) ||
                           (element_down == O_CONVEYOR_RIGHT && element_dir == GD_MV_RIGHT));

  return (!cave->gravity_disabled && cave->conveyor_belts_active &&
          ((cave->gravity == GD_MV_DOWN && ((conveyor_down && moved_by_conveyor_top) ||
                                            (conveyor_up   && moved_by_conveyor_bottom))) ||
           (cave->gravity == GD_MV_UP   && ((conveyor_up   && moved_by_conveyor_top) ||
                                            (conveyor_down && moved_by_conveyor_bottom)))));
}

// returns true if the given element is scanned
boolean is_scanned_element(GdElement e)
{
  return (gd_element_properties[e].properties & P_SCANNED) != 0;
}

// This function converts an element to its scanned pair.
GdElement scanned_pair(GdElement of_what)
{
  if (gd_element_properties[of_what].properties & P_SCANNED) // already scanned?
    return of_what;

  return gd_element_properties[of_what].pair;
}

// This function converts an element to its non-scanned pair.
GdElement non_scanned_pair(GdElement of_what)
{
  if (!(gd_element_properties[of_what].properties & P_SCANNED)) // already non-scanned?
    return of_what;

  return gd_element_properties[of_what].pair;
}

// returns true if the element is a scanned one (needed by the engine)
static inline boolean is_scanned(const GdCave *cave, const int x, const int y)
{
  return is_scanned_element(get(cave, x, y));
}

// returns true if the element is a scanned one (needed by the engine) (with direction)
static inline boolean is_scanned_dir(const GdCave *cave, const int x, const int y,
				     const GdDirection dir)
{
  return is_scanned_element(get_dir(cave, x, y, dir));
}

// Returns true if neighboring element is "e", or equivalent to "e".
// Dirt is treated in a special way; eg. if is_like_element(O_DIRT) is
// asked, and an O_DIRT2 is there, true is returned.
// Also, lava is special; if is_like_element(O_SPACE) is asked, and
// an O_LAVA is there, true is returned. This way, any movement
// is allowed by any creature and player into lava.
//
// @param x The x coordinate on the map
// @param y The y coordinate on the map
// @param dir The direction to add to (x, y) and check the element at
// @param e The element to compare (x, y) + dir to
// @return True, if they are equivalent
static inline boolean is_like_element(const GdCave *cave, const int x, const int y,
                                      const GdDirection dir, GdElement e)
{
  GdElement examined = get_dir(cave, x, y, dir);

  // if it is a dirt-like, change to dirt, so equality will evaluate to true
  if (has_property(examined, P_DIRT))
    examined = O_DIRT;

  if (has_property(e, P_DIRT))
    e = O_DIRT;

  // if the element on the map is a lava, it should be like space
  if (examined == O_LAVA)
    examined = O_SPACE;

  // now they are "normalized", compare and return
  return (e == examined);
}

// Returns true if the neighboring element (x, y) + dir is space or lava.
// This is a shorthand function to check easily if there is space somewhere.
// Any movement which is possible into space, must also be possible into lava.
// Therefore 'if (get(cave, x, y) == O_SPACE)' must not be used!
//
// Lava absorbs everything going into it. Everything.
// But it does not "pull" elements; only the things disappear which
// _do_ go directly into it. So if the player steps into the lava,
// he will die. If a dragonfly flies over it, it will not.
//
// This behavior is implemented in the is_like_space and the store
// functions. is_like_space returns true for the lava, too. The store
// function ignores any store requests into the lava.
// The player_eat_element function will also behave for lava as it does for space.
static inline boolean is_like_space(const GdCave *cave, const int x, const int y,
                                    const GdDirection dir)
{
  GdElement e = get_dir(cave, x, y, dir);

  return (e == O_SPACE || e == O_LAVA);
}

// Returns true if element at (x, y) + dir is like dirt.
// All dirt types must be equivalent; for example, when allowing the player to
// place a bomb in dirt, or when a nitro pack is bouncing on a piece of dirt
// (without exploding).
// Therefore "if (get(cave, x, y) == O_DIRT)" must not be used!
static inline boolean is_like_dirt(const GdCave *cave, const int x, const int y,
                                   const GdDirection dir)
{
  return has_property(get_dir(cave, x, y, dir), P_DIRT);
}

// store from/to directions in special buffers for smooth movement animations
static inline void store_dir_buffer(GdCave *cave, const int x, const int y, const GdDirection dir)
{
  int new_x = getx(cave, x + gd_dx[dir], y + gd_dy[dir]);
  int new_y = gety(cave, x + gd_dx[dir], y + gd_dy[dir]);

  game_bd.game->dir_buffer_from[y][x] = dir;
  game_bd.game->dir_buffer_to[new_y][new_x] = dir;
}

// Store an element at a given position; lava absorbs everything.
// If there is a lava originally at the given position, sound is played, and
// the map is NOT changed.
// The element given is changed to its "scanned" state, if there is such.
static inline void store(GdCave *cave, const int x, const int y, const GdElement element)
{
  GdElement *e = getp(cave, x, y);

  if (*e == O_LAVA)
  {
    play_sound_of_element(cave, O_LAVA, x, y);

    return;
  }

  *e = scanned_pair(element);
}

// Store an element to (x, y) + dir.
static inline void store_dir(GdCave *cave, const int x, const int y,
			     const GdDirection dir, const GdElement element)
{
  store_dir_buffer(cave, x, y, dir);
  store(cave, x + gd_dx[dir], y + gd_dy[dir], element);
}

// Store the element to (x, y) + dir, and store a space to (x, y).
static inline void move(GdCave *cave, const int x, const int y,
			const GdDirection dir, const GdElement element)
{
  // falling/flying game elements at wrap-around cave position should not kill player instantly
  if ((x + gd_dx[dir] == cave->w && dir == GD_MV_RIGHT) ||
      (y + gd_dy[dir] == cave->h && dir == GD_MV_DOWN))
  {
    // cave width/height out of bounds, but due to wrap-around it's the first column/row again
    if (el_can_smash_player(get(cave, x, y)))
    {
      store(cave, x, y, element); // change to falling element ...

      return;                     // ... but do not move element
    }
  }

  store_dir(cave, x, y, dir, element);
  store(cave, x, y, O_SPACE);
}

// increment a cave element; can be used for elements which are one after the other,
// for example, bladder1, bladder2, bladder3...
static inline void next(GdCave *cave, const int x, const int y)
{
  (*getp(cave, x, y))++;
}

// Remove the scanned "bit" from an element.
// To be called only for scanned elements!!!
static inline void unscan(GdCave *cave, const int x, const int y)
{
  GdElement *e = getp(cave, x, y);

  if (is_scanned_element(*e))
    *e = gd_element_properties[*e].pair;
}

// Change the cell at (x, y) to a given explosion type.
// Used by 3x3 explosion functions.
// Take care of non explodable elements.
// Take care of other special cases, like a voodoo dying,
// and a nitro pack explosion triggered.
static void cell_explode(GdCave *cave, int x, int y, GdElement explode_to)
{
  if (non_explodable(cave, x, y))
    return;

  if (cave->voodoo_any_hurt_kills_player && get(cave, x, y) == O_VOODOO)
    cave->voodoo_touched = TRUE;

  if (get(cave, x, y) == O_VOODOO && !cave->voodoo_disappear_in_explosion)
  {
    // voodoo turns into a time penalty
    store(cave, x, y, O_TIME_PENALTY);
  }
  else if (get(cave, x, y) == O_NITRO_PACK ||
	   get(cave, x, y) == O_NITRO_PACK_F)
  {
    // nitro pack inside an explosion - it is now triggered
    store(cave, x, y, O_NITRO_PACK_EXPLODE);
  }
  else
  {
    // for everything else
    store(cave, x, y, explode_to);
  }
}

// A creature at (x, y) explodes to a 3x3 square.
static void creature_explode(GdCave *cave, int x, int y, GdElement explode_to)
{
  int xx, yy;

  // the processing of an explosion took pretty much time: processing 3x3 = 9 elements
  cave->ckdelay_current += 1200;
  gd_sound_play(cave, GD_S_EXPLODING, get(cave, x, y), x, y);

  for (yy = y - 1; yy <= y + 1; yy++)
    for (xx = x - 1; xx <= x + 1; xx++)
      cell_explode(cave, xx, yy, explode_to);
}

// A nitro pack at (x, y) explodes to a 3x3 square.
static void nitro_explode(GdCave *cave, int x, int y)
{
  int xx, yy;

  // the processing of an explosion took pretty much time: processing 3x3 = 9 elements
  cave->ckdelay_current += 1200;
  gd_sound_play(cave, GD_S_NITRO_PACK_EXPLODING, get(cave, x, y), x, y);

  for (yy = y - 1; yy <= y + 1; yy++)
    for (xx = x - 1; xx <= x + 1; xx++)
      cell_explode(cave, xx, yy, O_NITRO_EXPL_1);

  // the current cell is explicitly changed into a nitro expl,
  // as cell_explode changes it to a triggered nitro pack
  store(cave, x, y, O_NITRO_EXPL_1);
}

// A voodoo explodes, leaving a 3x3 steel and a time penalty behind.
static void voodoo_explode(GdCave *cave, int x, int y)
{
  int xx, yy;

  if (cave->voodoo_any_hurt_kills_player)
    cave->voodoo_touched = TRUE;

  // the processing of an explosion took pretty much time: processing 3x3 = 9 elements
  cave->ckdelay_current += 1000;
  gd_sound_play(cave, GD_S_VOODOO_EXPLODING, get(cave, x, y), x, y);

  // voodoo explodes to 3x3 steel
  for (yy = y - 1; yy <= y + 1; yy++)
    for (xx = x - 1; xx <= x + 1; xx++)
      store(cave, xx, yy, O_PRE_STEEL_1);

  // middle is a time penalty (which will be turned into a gravestone)
  store(cave, x, y, O_TIME_PENALTY);
}

// Explode cell at (x, y), but skip voodooo.
// A bomb does not explode the voodoo, neither does the ghost.
// This function checks these, and stores the new element given or not.
// Destroying the voodoo is also controlled by the voodoo_disappear_in_explosion flag.
static void cell_explode_skip_voodoo(GdCave *cave, const int x, const int y, const GdElement expl)
{
  if (non_explodable(cave, x, y))
    return;

  // bomb does not explode voodoo
  if (!cave->voodoo_disappear_in_explosion && get(cave, x, y) == O_VOODOO)
    return;

  if (cave->voodoo_any_hurt_kills_player && get(cave, x, y) == O_VOODOO)
    cave->voodoo_touched = TRUE;

  store(cave, x, y, expl);
}

// An X shaped ghost explosion; does not touch voodoo!
static void ghost_explode(GdCave *cave, const int x, const int y)
{
  // the processing of an explosion took pretty much time: processing 5 elements
  cave->ckdelay_current += 650;
  gd_sound_play(cave, GD_S_GHOST_EXPLODING, get(cave, x, y), x, y);

  cell_explode_skip_voodoo(cave, x,     y,     O_GHOST_EXPL_1);
  cell_explode_skip_voodoo(cave, x - 1, y - 1, O_GHOST_EXPL_1);
  cell_explode_skip_voodoo(cave, x + 1, y + 1, O_GHOST_EXPL_1);
  cell_explode_skip_voodoo(cave, x - 1, y + 1, O_GHOST_EXPL_1);
  cell_explode_skip_voodoo(cave, x + 1, y - 1, O_GHOST_EXPL_1);
}

// A + shaped bomb explosion; does not touch voodoo!
static void bomb_explode(GdCave *cave, const int x, const int y)
{
  // the processing of an explosion took pretty much time: processing 5 elements
  cave->ckdelay_current += 650;
  gd_sound_play(cave, GD_S_BOMB_EXPLODING, get(cave, x, y), x, y);

  cell_explode_skip_voodoo(cave, x,     y,     O_BOMB_EXPL_1);
  cell_explode_skip_voodoo(cave, x - 1, y,     O_BOMB_EXPL_1);
  cell_explode_skip_voodoo(cave, x + 1, y,     O_BOMB_EXPL_1);
  cell_explode_skip_voodoo(cave, x,     y + 1, O_BOMB_EXPL_1);
  cell_explode_skip_voodoo(cave, x,     y - 1, O_BOMB_EXPL_1);
}

// Explode the thing at (x, y).
// Checks the element, and selects the correct exploding type accordingly.
static void explode(GdCave *cave, int x, int y)
{
  GdElement e = get(cave, x, y);

  switch (e)
  {
    case O_GHOST:
      ghost_explode(cave, x, y);
      break;

    case O_BOMB_TICK_7:
      bomb_explode(cave, x, y);
      break;

    case O_VOODOO:
      voodoo_explode(cave, x, y);
      break;

    case O_NITRO_PACK:
    case O_NITRO_PACK_F:
    case O_NITRO_PACK_EXPLODE:
      nitro_explode(cave, x, y);
      break;

    case O_AMOEBA_2:
      creature_explode(cave, x, y, O_AMOEBA_2_EXPL_1);
      break;

    case O_FALLING_WALL_F:
      creature_explode(cave, x, y, O_EXPLODE_1);
      break;

    case O_ROCKET_1:
    case O_ROCKET_2:
    case O_ROCKET_3:
    case O_ROCKET_4:
      creature_explode(cave, x, y, O_EXPLODE_1);
      break;

    case O_BUTTER_1:
    case O_BUTTER_2:
    case O_BUTTER_3:
    case O_BUTTER_4:
      creature_explode(cave, x, y, cave->butterfly_explode_to);
      break;

    case O_ALT_BUTTER_1:
    case O_ALT_BUTTER_2:
    case O_ALT_BUTTER_3:
    case O_ALT_BUTTER_4:
      creature_explode(cave, x, y, cave->alt_butterfly_explode_to);
      break;

    case O_FIREFLY_1:
    case O_FIREFLY_2:
    case O_FIREFLY_3:
    case O_FIREFLY_4:
      creature_explode(cave, x, y, cave->firefly_explode_to);
      break;

    case O_ALT_FIREFLY_1:
    case O_ALT_FIREFLY_2:
    case O_ALT_FIREFLY_3:
    case O_ALT_FIREFLY_4:
      creature_explode(cave, x, y, cave->alt_firefly_explode_to);
      break;

    case O_PLAYER:
    case O_PLAYER_START:
    case O_PLAYER_BOMB:
    case O_PLAYER_GLUED:
    case O_PLAYER_STIRRING:
    case O_PLAYER_ROCKET_LAUNCHER:
    case O_PLAYER_PNEUMATIC_LEFT:
    case O_PLAYER_PNEUMATIC_RIGHT:
      creature_explode(cave, x, y, O_EXPLODE_1);
      break;

    case O_STONEFLY_1:
    case O_STONEFLY_2:
    case O_STONEFLY_3:
    case O_STONEFLY_4:
      creature_explode(cave, x, y, cave->stonefly_explode_to);
      break;

    case O_DRAGONFLY_1:
    case O_DRAGONFLY_2:
    case O_DRAGONFLY_3:
    case O_DRAGONFLY_4:
      creature_explode(cave, x, y, cave->dragonfly_explode_to);
      break;

    default:
      break;
  }
}

// Explode the element at (x, y) + dir.
// A simple wrapper for the explode(x, y) function without the dir parameter.
static void inline explode_dir(GdCave *cave, const int x, const int y, GdDirection dir)
{
  explode(cave, x + gd_dx[dir], y + gd_dy[dir]);
}

// The player eats or activates the given element.
// This function does all things that should happen when the
// player eats something - increments score, plays sound etc.
// This function is also used to activate switches, and to collect
// keys.
// It returns the remaining element, which is usually space;
// might be some other thing. (example: DIRT for CLOCK)
// This does NOT take snap_element into consideration.
//
// @param object Element to eat
// @param x The coordinate of player
// @param y The coordinate of player
// @param dir The direction the player is moving
// @return remaining element
static GdElement player_eat_element(GdCave *cave, const GdElement element, int x, int y,
                                    GdDirection dir)
{
  int i;

  x += gd_dx[dir];
  y += gd_dy[dir];

  switch (element)
  {
    case O_DIAMOND_KEY:
      gd_sound_play(cave, GD_S_DIAMOND_KEY_COLLECTING, element, x, y);
      cave->diamond_key_collected = TRUE;
      return O_SPACE;

    // KEYS AND DOORS
    case O_KEY_1:
      gd_sound_play(cave, GD_S_KEY_COLLECTING, element, x, y);
      cave->key1++;
      return O_SPACE;

    case O_KEY_2:
      gd_sound_play(cave, GD_S_KEY_COLLECTING, element, x, y);
      cave->key2++;
      return O_SPACE;

    case O_KEY_3:
      gd_sound_play(cave, GD_S_KEY_COLLECTING, element, x, y);
      cave->key3++;
      return O_SPACE;

    case O_DOOR_1:
      if (cave->key1 == 0)
	return O_NONE;
      gd_sound_play(cave, GD_S_DOOR_OPENING, element, x, y);
      cave->key1--;
      return O_SPACE;

    case O_DOOR_2:
      if (cave->key2 == 0)
	return O_NONE;
      gd_sound_play(cave, GD_S_DOOR_OPENING, element, x, y);
      cave->key2--;
      return O_SPACE;

    case O_DOOR_3:
      if (cave->key3 == 0)
	return O_NONE;
      gd_sound_play(cave, GD_S_DOOR_OPENING, element, x, y);
      cave->key3--;
      return O_SPACE;

    // SWITCHES
    case O_CREATURE_SWITCH:
      // creatures change direction.
      gd_sound_play(cave, GD_S_SWITCH_CREATURES, element, x, y);
      cave->creatures_backwards = !cave->creatures_backwards;
      return element;

    case O_EXPANDING_WALL_SWITCH:
      // expanding wall change direction.
      gd_sound_play(cave, GD_S_SWITCH_EXPANDING, element, x, y);
      cave->expanding_wall_changed = !cave->expanding_wall_changed;
      return element;

    case O_BITER_SWITCH:
      // biter change delay
      gd_sound_play(cave, GD_S_SWITCH_BITER, element, x, y);
      cave->biter_delay_frame++;
      if (cave->biter_delay_frame == 4)
	cave->biter_delay_frame = 0;
      return element;

    case O_REPLICATOR_SWITCH:
      // replicator on/off switch
      gd_sound_play(cave, GD_S_SWITCH_REPLICATOR, element, x, y);
      cave->replicators_active = !cave->replicators_active;
      return element;

    case O_CONVEYOR_SWITCH:
      // conveyor belts on/off
      gd_sound_play(cave, GD_S_SWITCH_CONVEYOR, element, x, y);
      cave->conveyor_belts_active = !cave->conveyor_belts_active;
      return element;

    case O_CONVEYOR_DIR_SWITCH:
      // conveyor belts switch direction
      gd_sound_play(cave, GD_S_SWITCH_CONVEYOR, element, x, y);
      cave->conveyor_belts_direction_changed = !cave->conveyor_belts_direction_changed;
      return element;

    case O_GRAVITY_SWITCH:
      // only allow changing direction if the new dir is not diagonal
      if (cave->gravity_switch_active &&
          (dir == GD_MV_LEFT ||
           dir == GD_MV_RIGHT ||
           dir == GD_MV_UP ||
           dir == GD_MV_DOWN))
      {
        gd_sound_play(cave, GD_S_SWITCH_GRAVITY, element, x, y);
        // (use 1 instead of 0 for immediate gravitation change)
        cave->gravity_will_change =
          MAX(1, cave->gravity_change_time * cave->timing_factor);
        cave->gravity_next_direction = dir;
        cave->gravity_switch_active = FALSE;
        return element;
      }
      return O_NONE;

    // USUAL STUFF
    case O_DIRT:
    case O_DIRT2:
    case O_DIRT_SLOPED_UP_RIGHT:
    case O_DIRT_SLOPED_UP_LEFT:
    case O_DIRT_SLOPED_DOWN_LEFT:
    case O_DIRT_SLOPED_DOWN_RIGHT:
    case O_DIRT_BALL:
    case O_DIRT_LOOSE:
    case O_STEEL_EATABLE:
    case O_BRICK_EATABLE:
      gd_sound_play(cave, GD_S_DIRT_WALKING_RANDOM, element, x, y);
      return O_SPACE;

    case O_SPACE:
    case O_LAVA:    // player goes into lava, as if it was space
      gd_sound_play(cave, GD_S_EMPTY_WALKING_RANDOM, element, x, y);
      return O_SPACE;

    case O_SWEET:
      gd_sound_play(cave, GD_S_SWEET_COLLECTING, element, x, y);
      cave->sweet_eaten = TRUE;
      return O_SPACE;

    case O_PNEUMATIC_HAMMER:
      gd_sound_play(cave, GD_S_PNEUMATIC_COLLECTING, element, x, y);
      cave->got_pneumatic_hammer = TRUE;
      return O_SPACE;

    case O_CLOCK:
      // bonus time
      gd_sound_play(cave, GD_S_CLOCK_COLLECTING, element, x, y);
      cave->time += cave->time_bonus * cave->timing_factor;
      if (cave->time > cave->max_time * cave->timing_factor)
	cave->time -= cave->max_time * cave->timing_factor;
      setTimeLeft_BD();
      // no space, rather a dirt remains there...
      return O_DIRT;

    case O_DIAMOND:
    case O_FLYING_DIAMOND:
      // prevent diamond sounds for O_SKELETON (see below)
      if (x != -1 || y != -1)
	gd_sound_play(cave, (element == O_DIAMOND ? GD_S_DIAMOND_COLLECTING :
			     GD_S_FLYING_DIAMOND_COLLECTING), element, x, y);

      cave->score += cave->diamond_value;
      cave->diamonds_collected++;

      game.snapshot.collected_item = TRUE;

      if (cave->diamonds_needed == cave->diamonds_collected)
      {
	cave->gate_open = TRUE;

	// extra is worth more points.
	cave->diamond_value = cave->extra_diamond_value;

	cave->gate_open_flash = 1;
	cave->sound3 = GD_S_CRACKING;
	gd_sound_play(cave, GD_S_CRACKING, O_OUTBOX, x, y);
      }
      return O_SPACE;

    case O_SKELETON:
      cave->skeletons_collected++;

      // as if player got a diamond
      for (i = 0; i < cave->skeletons_worth_diamonds; i++)
	player_eat_element(cave, O_DIAMOND, -1, -1, GD_MV_STILL);

      // _after_ calling get_element for the fake diamonds, so we overwrite its sounds
      gd_sound_play(cave, GD_S_SKELETON_COLLECTING, element, x, y);
      return O_SPACE;

    case O_ROCKET:
      cave->rockets_collected++;
      gd_sound_play(cave, GD_S_BOMB_COLLECTING, element, x, y);
      return O_SPACE;

    case O_OUTBOX:
    case O_INVIS_OUTBOX:
    case O_STEEL_OUTBOX:
    case O_INVIS_STEEL_OUTBOX:
      cave->player_state = GD_PL_EXITED;    // player now exits the cave!
      return O_SPACE;

    default:
      // non-eatable, does nothing
      return O_NONE;
  }
}

// Process a crazy dream-style teleporter.
// Called from gd_cave_iterate, for a player or a player_bomb.
// Player is standing at px, py, and trying to move in the direction player_move,
// where there is a teleporter at (tx_start, ty_start). We check the whole cave,
// from (tx_start + 1, ty_start), till we get back to (tx_start, ty_start) (by wrapping
// around). The first teleporter we find, and which is suitable, will be the destination.
//
// @param px The coordinate of the player which tries to move into the teleporter, x.
// @param py The coordinate of the player, y.
// @param player_move The direction he is moving into.
// @return True, if the player is teleported, false, if no suitable teleporter found.
static boolean do_teleporter(GdCave *cave, int px, int py, GdDirection player_move)
{
  // start at teleporter position (not at player position!) unless using buggy behaviour
  int tx_start = px + (cave->buggy_teleporter ? 0 : gd_dx[player_move]);
  int ty_start = py + (cave->buggy_teleporter ? 0 : gd_dy[player_move]);
  int tx = tx_start;
  int ty = ty_start;

  do
  {
    // jump to next element; wrap around columns and rows.
    tx++;

    if (tx >= cave->w)
    {
      tx = 0;
      ty++;

      if (ty >= cave->h)
	ty = 0;
    }

    // if we found a teleporter...
    if (get(cave, tx, ty) == O_TELEPORTER &&
	is_like_space(cave, tx, ty, player_move))
    {
      // new player appears near teleporter found
      store_dir(cave, tx, ty, player_move, get(cave, px, py));

      // current player disappears
      store(cave, px, py, O_SPACE);

      gd_sound_play(cave, GD_S_TELEPORTER, O_TELEPORTER, tx, ty);

      return TRUE;    // return true as teleporter worked
    }
  }
  // loop until we get back to original coordinates
  while (tx != tx_start || ty != ty_start);

  // return false as we did not find any usable teleporter
  return FALSE;
}

// check if attempt to push an element was successful
static boolean do_push_successful(GdCave *cave, int prob)
{
  if (game_bd.game->use_krissz_engine)
  {
    if (prob == 1000000)	// p = 1, always push
      return TRUE;

    return (get_krissz_random(cave, 1000000 / prob) == 1);
  }
  else
  {
    // normal case: playing game or replay
    return (gd_rand_int_range(cave->random, 0, 1000000) < prob);
  }
}

// Try to push an element.
// Also does move the specified _element_, if possible.
// Up to the caller to move the _player_itself_, as the movement might be a snap,
// not a real movement.
//
// @return true if the push is possible.
static boolean do_push(GdCave *cave, int x, int y, GdDirection player_move, boolean player_fire)
{
  GdElement what = get_dir(cave, x, y, player_move);
  int what_x = getx(cave, x + gd_dx[player_move], y + gd_dy[player_move]);
  int what_y = gety(cave, x + gd_dx[player_move], y + gd_dy[player_move]);

  // gravity for falling wall, bladder, ...
  GdDirection grav_compat = (cave->gravity_affects_all ? cave->gravity : GD_MV_DOWN);

  boolean result = FALSE;

  // do a switch on what element is being pushed to determine probability.
  switch (what)
  {
    case O_WAITING_STONE:
    case O_STONE:
    case O_NITRO_PACK:
    case O_CHASING_STONE:
    case O_MEGA_STONE:
    case O_LIGHT_STONE:
    case O_FLYING_STONE:
    case O_NUT:
      // pushing some kind of stone or nut
      // directions possible: 90degrees cw or ccw to current gravity.
      // only push if player dir is orthogonal to gravity,
      // ie. gravity down, pushing left & right possible
      if (player_move == ccw_fourth[cave->gravity] ||
	  player_move == cw_fourth[cave->gravity])
      {
	int prob = 0;

	// different probabilities for different elements.
	switch (what)
	{
	  case O_WAITING_STONE:
	    // waiting stones are light, can always push
	    prob = 1000000;
	    break;

	  case O_CHASING_STONE:
	    // chasing can be pushed if player is turbo
	    if (cave->sweet_eaten)
	      prob = 1000000; // with p = 1, always push
	    break;

	  case O_MEGA_STONE:
	    // mega may(!) be pushed if player is turbo
	    if (cave->mega_stones_pushable_with_sweet && cave->sweet_eaten)
	      prob = 1000000; // p = 1, always push
	    break;

	  case O_LIGHT_STONE:
	    // light stones are light, can always push
	    prob = 1000000;
	    break;

	  case O_STONE:
	  case O_NUT:
	  case O_FLYING_STONE:
	  case O_NITRO_PACK:
	    if (cave->sweet_eaten)
	      prob = cave->pushing_stone_prob_sweet; // probability with sweet
	    else
	      prob = cave->pushing_stone_prob; // probability without sweet.
	    break;

	  default:
	    break;
	}

	// only push game element if not already moving
	if (is_like_space(cave, x, y, twice[player_move]) &&
	    game_bd.game->dir_buffer_to[what_y][what_x] == GD_MV_STILL &&
	    do_push_successful(cave, prob))
	{
	  // if decided that he will be able to push,
	  play_sound_of_element_pushing(cave, what,
                                        x + gd_dx[player_move],
                                        y + gd_dy[player_move]);

          // if pushed a stone, it "bounces". all other elements are simply pushed.
          if (what == O_STONE)
            store_dir(cave, what_x, what_y, player_move, cave->stone_bouncing_effect);
          else
            store_dir(cave, what_x, what_y, player_move, what);

          // special case: some stones pushed beyond right playfield border do not fall down
          if (((game_bd.game->use_krissz_engine && what == O_STONE) || what == O_LIGHT_STONE) &&
              player_move == GD_MV_RIGHT && what_x == cave->w - 1)
            game_bd.game->scanned_next[what_y][0] = TRUE;

	  result = TRUE;
	}
      }
      break;

    case O_BLADDER:
    case O_BLADDER_1:
    case O_BLADDER_2:
    case O_BLADDER_3:
    case O_BLADDER_4:
    case O_BLADDER_5:
    case O_BLADDER_6:
    case O_BLADDER_7:
    case O_BLADDER_8:
      // pushing a bladder. keep in mind that after pushing, we always get an O_BLADDER,
      // not an O_BLADDER_x.

      // first check: we cannot push a bladder "up"
      if (player_move != opposite[grav_compat])
      {
	// pushing a bladder "down". p = player, o = bladder, 1, 2, 3 = directions to check.
	// player moving in the direction of gravity.
	//  p   p  g
	// 2o3  |  |
	//  1   v  v

	if (player_move == grav_compat)
	{
          // pushing bladder down
	  if (is_like_space(cave, x, y, twice[player_move]))
          {
	    store_dir(cave, what_x, what_y, player_move, O_BLADDER);
            result = TRUE;
          }
	  // if no space to push down, maybe left (down-left to player)
	  else if (is_like_space(cave, x, y, cw_eighth[grav_compat]))
          {
	    // left is "down, turned right (cw)"
	    store_dir(cave, what_x, what_y, cw_fourth[grav_compat], O_BLADDER);
            result = TRUE;
          }
	  // if not, maybe right (down-right to player)
	  else if (is_like_space(cave, x, y, ccw_eighth[grav_compat]))
          {
	    store_dir(cave, what_x, what_y, ccw_fourth[grav_compat], O_BLADDER);
            result = TRUE;
          }
	}

	// pushing a bladder "left". p = player, o = bladder, 1, 2, 3 = directions to check.
	//  3        g
	// 1op  <-p  |
	//  2        v

	else if (player_move == cw_fourth[grav_compat])
	{
          // pushing it left
	  if (is_like_space(cave, x, y, twice[cw_fourth[grav_compat]]))
          {
	    store_dir(cave, what_x, what_y, player_move, O_BLADDER);
            result = TRUE;
          }
          // maybe down, and player will move left
	  else if (is_like_space(cave, x, y, cw_eighth[grav_compat]))
          {
	    store_dir(cave, what_x, what_y, grav_compat, O_BLADDER);
            result = TRUE;
          }
          // maybe up, and player will move left
	  else if (is_like_space(cave, x, y, cw_eighth[player_move]))
          {
	    store_dir(cave, what_x, what_y, opposite[grav_compat], O_BLADDER);
            result = TRUE;
          }
	}

	// pushing a bladder "right". p = player, o = bladder, 1, 2, 3 = directions to check.
	//  3        g
	// po1  p-<  |
	//  2        v

	else if (player_move == ccw_fourth[grav_compat])
	{
          // pushing it right
	  if (is_like_space(cave, x, y, twice[player_move]))
          {
	    store_dir(cave, what_x, what_y, player_move, O_BLADDER);
            result = TRUE;
          }
          // maybe down, and player will move right
	  else if (is_like_space(cave, x, y, ccw_eighth[grav_compat]))
          {
	    store_dir(cave, what_x, what_y, grav_compat, O_BLADDER);
            result = TRUE;
          }
          // maybe up, and player will move right
	  else if (is_like_space(cave, x, y, ccw_eighth[player_move]))
          {
	    store_dir(cave, what_x, what_y, opposite[grav_compat], O_BLADDER);
            result = TRUE;
          }
	}

	if (result)
	  play_sound_of_element_pushing(cave, O_BLADDER, x, y);
      }
      break;

    case O_BOX:
      // a box is only pushed with the fire pressed
      if (player_fire)
      {
	// but always with 100% probability
	switch (player_move)
	{
	  case GD_MV_LEFT:
	  case GD_MV_RIGHT:
	  case GD_MV_UP:
	  case GD_MV_DOWN:
	    // pushing in some dir, two steps in that dir - is there space?
	    if (is_like_space(cave, x, y, twice[player_move]))
	    {
	      // yes, so push.
	      store_dir(cave, what_x, what_y, player_move, O_BOX);
	      result = TRUE;
	      gd_sound_play(cave, GD_S_BOX_PUSHING, what, x, y);
	    }
	    break;

	  default:
	    // push in no other directions possible
	    break;
	}
      }
      break;

      // pushing of other elements not possible
    default:
      break;
  }

  return result;
}

// from the key press booleans, create a direction
GdDirection gd_direction_from_keypress(boolean up, boolean down, boolean left, boolean right)
{
  GdDirection player_move;

  // from the key press booleans, create a direction
  if (up && right)
    player_move = GD_MV_UP_RIGHT;
  else if (down && right)
    player_move = GD_MV_DOWN_RIGHT;
  else if (down && left)
    player_move = GD_MV_DOWN_LEFT;
  else if (up && left)
    player_move = GD_MV_UP_LEFT;
  else if (up)
    player_move = GD_MV_UP;
  else if (down)
    player_move = GD_MV_DOWN;
  else if (left)
    player_move = GD_MV_LEFT;
  else if (right)
    player_move = GD_MV_RIGHT;
  else
    player_move = GD_MV_STILL;

  return player_move;
}

// clear these to no sound; and they will be set during iteration.
void gd_cave_clear_sounds(GdCave *cave)
{
  cave->sound1 = GD_S_NONE;
  cave->sound2 = GD_S_NONE;
  cave->sound3 = GD_S_NONE;
}

// Try to make an element start falling.
//
// @param x The x coordinate of the element.
// @param y The y coordinate of the element.
// @param falling_direction The direction to start "falling" to.
//         Down (=gravity) for a stone, Up (=opposite of gravity) for a flying stone, for example.
// @param falling_element The falling pair of the element (O_STONE -> O_STONE_F)
static void do_start_fall(GdCave *cave, int x, int y, GdDirection falling_direction,
			  GdElement falling_element)
{
  if (cave->gravity_disabled)
    return;

  if (is_like_space(cave, x, y, falling_direction))
  {
    // beginning to fall
    play_sound_of_element(cave, get(cave, x, y), x, y);
    move(cave, x, y, falling_direction, falling_element);
  }

  // check if it is on a sloped element, and it can roll.
  // for example, sloped wall looks like:
  //  /|
  // /_|
  // this is tagged as sloped up&left.
  // first check if the stone or diamond is coming from "up" (ie. opposite of gravity)
  // then check the direction to roll (left or right)
  // this way, gravity can also be pointing right, and the above slope will work as one
  // would expect
  else if (sloped(cave, x, y, falling_direction, opposite[falling_direction]))
  {
    // rolling down if sitting on a sloped object
    if (sloped(cave, x, y, falling_direction, cw_fourth[falling_direction]) &&
	is_like_space(cave, x, y, cw_fourth[falling_direction]) &&
	is_like_space(cave, x, y, cw_eighth[falling_direction]))
    {
      // rolling left? - keep in mind that ccw_fourth rotates gravity ccw,
      // so here we use cw_fourth
      play_sound_of_element(cave, get(cave, x, y), x, y);
      move(cave, x, y, cw_fourth[falling_direction], falling_element);
    }
    else if (sloped(cave, x, y, falling_direction, ccw_fourth[falling_direction]) &&
	     is_like_space(cave, x, y, ccw_fourth[falling_direction]) &&
	     is_like_space(cave, x, y, ccw_eighth[falling_direction]))
    {
      // rolling right?
      play_sound_of_element(cave, get(cave, x, y), x, y);
      move(cave, x, y, ccw_fourth[falling_direction], falling_element);
    }
  }
}

// When the element at (x, y) is falling in the direction fall_dir,
// check if it crushes a voodoo below it. If yes, explode the voodoo,
// and return true. Otherwise return false.
//
// @return true if voodoo crushed.
static boolean do_fall_try_crush_voodoo(GdCave *cave, int x, int y, GdDirection fall_dir)
{
  if (get_dir(cave, x, y, fall_dir) == O_VOODOO &&
      cave->voodoo_dies_by_stone)
  {
    // this is a 1stB-style vodo. explodes by stone, collects diamonds
    explode_dir(cave, x, y, fall_dir);

    return TRUE;
  }

  return FALSE;
}

// When the element at (x, y) is falling in the direction fall_dir,
// check if the voodoo below it can eat it. If yes, the voodoo eats it.
//
// @return true if successful, false if voodoo does not eat the element.
static boolean do_fall_try_eat_voodoo(GdCave *cave, int x, int y, GdDirection fall_dir)
{
  if (get_dir(cave, x, y, fall_dir) == O_VOODOO &&
      cave->voodoo_collects_diamonds)
  {
    // this is a 1stB-style voodoo. explodes by stone, collects diamonds
    player_eat_element(cave, O_DIAMOND, x, y, fall_dir);   // as if player got diamond
    store(cave, x, y, O_SPACE);    // diamond disappears

    return TRUE;
  }

  return FALSE;
}

// Element at (x, y) is falling. Try to crack nut under it.
// If successful, nut is cracked, and the element is bounced (stops moving).
//
// @param fall_dir The direction the element is falling in.
// @param bouncing The element which it is converted to, if it has cracked a nut.
// @return True, if nut is cracked.
static boolean do_fall_try_crack_nut(GdCave *cave, int x, int y, GdDirection fall_dir,
                                     GdElement bouncing)
{
  if (get_dir(cave, x, y, fall_dir) == O_NUT ||
      get_dir(cave, x, y, fall_dir) == O_NUT_F)
  {
    // stones
    store(cave, x, y, bouncing);
    store_dir(cave, x, y, fall_dir, cave->nut_turns_to_when_crushed);

    gd_sound_play(cave, GD_S_NUT_CRACKING, O_NUT, x, y);

    return TRUE;
  }

  return FALSE;
}

// For a falling element, try if a magic wall is under it.
// If yes, process element using the magic wall, and return true.
//
// @param fall_dir The direction the element is falling to.
// @param magic The element a magic wall turns it to.
// @return If The element is processed by the magic wall.
static boolean do_fall_try_magic(GdCave *cave, int x, int y, GdDirection fall_dir, GdElement magic)
{
  if (get_dir(cave, x, y, fall_dir) == O_MAGIC_WALL)
  {
    play_sound_of_element(cave, O_DIAMOND, x, y);    // always play diamond sound

    if (cave->magic_wall_state == GD_MW_DORMANT)
      cave->magic_wall_state = GD_MW_ACTIVE;

    if (cave->magic_wall_state == GD_MW_ACTIVE)
    {
      if (is_like_space(cave, x, y, twice[fall_dir]))
      {
        // if magic wall active and place underneath, it turns element
        // into anything the effect says to do.
        store_dir(cave, x, y, twice[fall_dir], magic);
      }
      else
      {
        // store from/to directions in special buffers for smooth movement animations
        store_dir_buffer(cave, x, y, fall_dir);
      }
    }

    // active or non-active or anything, element falling in will always disappear
    store(cave, x, y, O_SPACE);

    // check for buggy BD1 amoeba + magic wall behaviour (also used by Krissz engine)
    if ((cave->magic_wall_breakscan && cave->amoeba_state == GD_AM_AWAKE) ||
        (game_bd.game->use_krissz_engine && cave->magic_wall_stops_amoeba))
      cave->convert_amoeba_this_frame = TRUE;

    return TRUE;
  }

  return FALSE;
}

// For a falling element, test if an explodable element is under it;
// if yes, explode it, and return yes.
//
// @return True, if element at (x, y) + fall_dir is exploded.
static boolean do_fall_try_crush(GdCave *cave, int x, int y, GdDirection fall_dir)
{
  if (explodes_by_hit(cave, x, y, fall_dir))
  {
    explode_dir(cave, x, y, fall_dir);

    return TRUE;
  }

  return FALSE;
}

// For a falling element, try if a sloped element is under it.
// Move element if possible, or bounce element.
// If there are two directions possible for the element to roll to, left is preferred.
// If no rolling is possible, it is converted to a bouncing element.
// So this always "does something" with the element, and this should be the last
// function to call when checking what happens to a falling element.
static void do_fall_roll_or_stop(GdCave *cave, int x, int y, GdDirection fall_dir,
                                 GdElement bouncing)
{
  if (is_like_space(cave, x, y, fall_dir))
  {
    // falling further
    move(cave, x, y, fall_dir, get(cave, x, y));

    return;
  }

  // check if it is on a sloped element, and it can roll.
  // for example, sloped wall looks like:
  //  /|
  // /_|
  // this is tagged as sloped up&left.
  // first check if the stone or diamond is coming from "up" (ie. opposite of gravity)
  // then check the direction to roll (left or right)
  // this way, gravity can also be pointing right, and the above slope will work as one
  // would expect

  if (sloped(cave, x, y, fall_dir, opposite[fall_dir]))
  {
    // sloped element, falling to left or right
    if (sloped(cave, x, y, fall_dir, cw_fourth[fall_dir]) &&
	is_like_space(cave, x, y, cw_eighth[fall_dir]) &&
	is_like_space(cave, x, y, cw_fourth[fall_dir]))
    {
      play_sound_of_element(cave, get(cave, x, y), x, y);

      // try to roll left first - see O_STONE to understand why cw_fourth
      move(cave, x, y, cw_fourth[fall_dir], get(cave, x, y));
    }
    else if (sloped(cave, x, y, fall_dir, ccw_fourth[fall_dir]) &&
	     is_like_space(cave, x, y, ccw_eighth[fall_dir]) &&
	     is_like_space(cave, x, y, ccw_fourth[fall_dir]))
    {
      play_sound_of_element(cave, get(cave, x, y), x, y);

      // if not, try to roll right
      move(cave, x, y, ccw_fourth[fall_dir], get(cave, x, y));
    }
    else
    {
      // cannot roll in any direction, so it stops
      play_sound_of_element(cave, get(cave, x, y), x, y);
      store(cave, x, y, bouncing);
    }

    return;
  }

  // any other element, stops
  play_sound_of_element(cave, get(cave, x, y), x, y);
  store(cave, x, y, bouncing);
}

// Process a cave - one iteration.
//
// @param player_move The direction the player moves to.
// @param player_fire True, if the fire button is pressed.
// @param suicide True, if the suicide button is pressed.
void gd_cave_iterate(GdCave *cave, GdDirection player_move, boolean player_fire, boolean suicide)
{
  int x, y, i;

  // for border scan
  int ymin, ymax;

  // amoeba found to be enclosed. if not, this is cleared
  boolean amoeba_found_enclosed, amoeba_2_found_enclosed;

  // counting the number of amoebas. after scan, check if too much
  int amoeba_count, amoeba_2_count;

  // cave scan found water - for sound
  boolean found_water;

  boolean inbox_toggle;
  boolean start_signal;

  // gravity for falling wall, bladder, ...
  GdDirection grav_compat = (cave->gravity_affects_all ? cave->gravity : GD_MV_DOWN);

  // directions for o_something_1, 2, 3 and 4 (creatures)
  static const GdDirection creature_dir[] =
  {
    GD_MV_LEFT,
    GD_MV_UP,
    GD_MV_RIGHT,
    GD_MV_DOWN
  };
  static const GdDirection creature_chdir[] =
  {
    GD_MV_RIGHT,
    GD_MV_DOWN,
    GD_MV_LEFT,
    GD_MV_UP
  };
  int time_decrement_sec;

  // biters eating elements preference, they try to go in this order
  GdElement biter_try[] =
  {
    O_DIRT,
    cave->biter_eat,
    O_SPACE,
    O_STONE
  };

  boolean amoeba_1_sound, amoeba_2_sound, magic_sound;

  gd_cave_clear_sounds(cave);

  game_bd.player_moving = FALSE;
  game_bd.player_snapping = FALSE;

  // if diagonal movements not allowed,
  // horizontal movements have precedence. [BROADRIBB]
  if (!cave->diagonal_movements)
  {
    switch (player_move)
    {
      case GD_MV_UP_RIGHT:
      case GD_MV_DOWN_RIGHT:
	player_move = GD_MV_RIGHT;
	break;

      case GD_MV_UP_LEFT:
      case GD_MV_DOWN_LEFT:
	player_move = GD_MV_LEFT;
	break;

      default:
	// no correction needed
	break;
    }
  }

  // set cave get function; to implement perfect or lineshifting borders
  if (cave->lineshift)
  {
    cave->getp = getp_shift;
    cave->getx = getx_shift;
    cave->gety = gety_shift;
  }
  else
  {
    cave->getp = getp_perfect;
    cave->getx = getx_perfect;
    cave->gety = gety_perfect;
  }

  // increment this. if the scan routine comes across player, clears it (sets to zero).
  if (cave->player_seen_ago < 100)
    cave->player_seen_ago++;

  if (cave->pneumatic_hammer_active_delay > 0)
    cave->pneumatic_hammer_active_delay--;

  // inboxes and outboxes flash with the rhythm of the game, not the display.
  // also, a player can be born only from an open, not from a steel-wall-like inbox.
  cave->inbox_flash_toggle = !cave->inbox_flash_toggle;
  inbox_toggle = cave->inbox_flash_toggle;

  if (cave->gate_open_flash > 0)
    cave->gate_open_flash--;

  // score collected this frame
  cave->score = 0;

  // to implement buggy BD1 amoeba + magic wall behaviour (also used by Krissz engine)
  cave->convert_amoeba_this_frame = FALSE;

  // suicide only kills the active player
  // player_x, player_y was set by the previous iterate routine, or the cave setup.
  // we must check if there is a player or not - he may have exploded or something like that
  if (suicide && cave->player_state == GD_PL_LIVING &&
      is_player(cave, cave->player_x, cave->player_y))
    store(cave, cave->player_x, cave->player_y, O_EXPLODE_1);

  // check for walls reappearing
  if (cave->hammered_reappear)
  {
    for (y = 0; y < cave->h; y++)
    {
      for (x = 0; x < cave->w; x++)
      {
	// timer for the cell > 0?
	if (cave->hammered_reappear[y][x] > 0)
	{
	  // decrease timer
	  cave->hammered_reappear[y][x]--;

	  // check if it became zero
	  if (cave->hammered_reappear[y][x] == 0)
	  {
	    store(cave, x, y, O_BRICK);
	    gd_sound_play(cave, GD_S_WALL_REAPPEARING, O_BRICK, x, y);
	  }
	}
      }
    }
  }

  // variables to check during the scan

  // will be set to false if any of the amoeba is found free.
  amoeba_found_enclosed = TRUE;
  amoeba_2_found_enclosed = TRUE;
  amoeba_count = 0;
  amoeba_2_count = 0;
  found_water = FALSE;
  cave->ckdelay_current = 0;
  time_decrement_sec = 0;

  // check whether to scan the first and last line
  if (cave->border_scan_first_and_last)
  {
    ymin = 0;
    ymax = cave->h - 1;
  }
  else
  {
    ymin = 1;
    ymax = cave->h - 2;
  }

  // set scanned state for this scan, if needed
  for (y = ymin; y <= ymax; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      if (game_bd.game->scanned_next[y][x])
        cave->map[y][x] = scanned_pair(cave->map[y][x]);

      game_bd.game->scanned_next[y][x] = FALSE;
    }
  }

  // the cave scan routine
  for (y = ymin; y <= ymax; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      // if we find a scanned element, change it to the normal one, and that's all.
      if (is_scanned(cave, x, y))
      {
        // Krissz engine: do not pass element through slime that has just been placed above it
        if (game_bd.game->use_krissz_engine && get_dir(cave, x, y, cave->gravity) == O_SLIME)
          continue;

        unscan(cave, x, y);

	continue;
      }

      // add the ckdelay correction value for every element seen.
      cave->ckdelay_current += gd_element_properties[get(cave, x, y)].ckdelay;

      switch (get(cave, x, y))
      {
	// ======================================================================================
	//    P L A Y E R S
	// ======================================================================================

	case O_PLAYER_START:
	  store(cave, x, y, O_PLAYER);
          // FALL THROUGH

	case O_PLAYER:
	  if (cave->kill_player)
	  {
	    explode (cave, x, y);

	    break;
	  }

	  cave->player_seen_ago = 0;

	  // bd4 intermission caves have many players. so if one of them has exited,
	  // do not change the flag anymore. so this if () is needed
	  if (cave->player_state != GD_PL_EXITED)
	    cave->player_state = GD_PL_LIVING;

	  // check for pneumatic hammer things
	  // 1) press fire, 2) have pneumatic hammer 4) space on left or right for hammer
          // 5) stand on something
	  if (player_fire && cave->got_pneumatic_hammer &&
	      is_like_space(cave, x, y, player_move) &&
	      !is_like_space(cave, x, y, GD_MV_DOWN))
	  {
	    if (player_move == GD_MV_LEFT &&
		can_be_hammered(cave, x, y, GD_MV_DOWN_LEFT))
	    {
	      cave->pneumatic_hammer_active_delay = cave->pneumatic_hammer_frame;
	      store_dir(cave, x, y, GD_MV_LEFT, O_PNEUMATIC_ACTIVE_LEFT);
	      store(cave, x, y, O_PLAYER_PNEUMATIC_LEFT);

	      break;    // finished.
	    }

	    if (player_move == GD_MV_RIGHT &&
		can_be_hammered(cave, x, y, GD_MV_DOWN_RIGHT))
	    {
	      cave->pneumatic_hammer_active_delay = cave->pneumatic_hammer_frame;
	      store_dir(cave, x, y, GD_MV_RIGHT, O_PNEUMATIC_ACTIVE_RIGHT);
	      store(cave, x, y, O_PLAYER_PNEUMATIC_RIGHT);

	      break;    // finished.
	    }
	  }

	  if (player_move != GD_MV_STILL)
	  {
	    // only do every check if he is not moving
	    GdElement what = get_dir(cave, x, y, player_move);
	    // O_NONE in this variable will mean that there is no change.
	    GdElement remains = O_NONE;

	    // if we are 'eating' a teleporter, and the function returns true
	    // (teleporting worked), break here
	    if (what == O_TELEPORTER && do_teleporter(cave, x, y, player_move))
	      break;

	    // try to push element; if successful, break
	    boolean push = do_push(cave, x, y, player_move, player_fire);

	    if (push)
	    {
	      remains = O_SPACE;
	    }
	    else
	    {
	      switch (what)
	      {
		case O_BOMB:
		  // if its a bomb, remember he now has one.
		  // we do not change the "remains" and "what" variables,
		  // so that part of the code will be ineffective
		  gd_sound_play(cave, GD_S_BOMB_COLLECTING, what, x, y);
		  store_dir(cave, x, y, player_move, O_SPACE);

		  if (player_fire)
		    store(cave, x, y, O_PLAYER_BOMB);
		  else
		    move(cave, x, y, player_move, O_PLAYER_BOMB);

		  break;

		case O_ROCKET_LAUNCHER:
		  // if its a rocket launcher, remember he now has one.
		  // we do not change the "remains" and "what" variables,
		  // so that part of the code will be ineffective
		  gd_sound_play(cave, GD_S_BOMB_COLLECTING, what, x, y);
		  store_dir(cave, x, y, player_move, O_SPACE);

		  if (player_fire)
		    store(cave, x, y, O_PLAYER_ROCKET_LAUNCHER);
		  else
		    move(cave, x, y, player_move, O_PLAYER_ROCKET_LAUNCHER);

		  break;

		case O_POT:
		  // we do not change the "remains" and "what" variables,
		  // so that part of the code will be ineffective
		  if (!player_fire && !cave->gravity_switch_active &&
		      cave->skeletons_collected >= cave->skeletons_needed_for_pot)
		  {
		    cave->skeletons_collected -= cave->skeletons_needed_for_pot;
		    move(cave, x, y, player_move, O_PLAYER_STIRRING);
		    cave->gravity_disabled = TRUE;
		  }
		  break;

		default:
		  // get element - process others.
		  // if cannot get, player_eat_element will return the same
		  remains = player_eat_element(cave, what, x, y, player_move);

		  break;
	      }
	    }

	    // if anything changed, apply the change.
	    if (remains != O_NONE)
	    {
	      // if snapping anything and we have snapping explosions set.
	      // but these is not true for pushing.
	      if (remains == O_SPACE && player_fire && !push)
              {
		remains = cave->snap_element;

		game_bd.player_snapping = TRUE;
              }

	      if (remains != O_SPACE || player_fire)
              {
		// if any other element than space, player cannot move.
		// also if pressing fire, will not move.
		store_dir(cave, x, y, player_move, remains);
              }
	      else
              {
		// if space remains there, the player moves.
		move(cave, x, y, player_move, O_PLAYER);

		game_bd.player_moving = TRUE;
              }
	    }
	  }
	  break;

	case O_PLAYER_BOMB:
          // much simpler; cannot snap-push stones
	  if (cave->kill_player)
	  {
	    explode(cave, x, y);

	    break;
	  }

	  cave->player_seen_ago = 0;

	  // bd4 intermission caves have many players. so if one of them has exited,
	  // do not change the flag anymore. so this if () is needed
	  if (cave->player_state != GD_PL_EXITED)
	    cave->player_state = GD_PL_LIVING;

	  if (player_move != GD_MV_STILL)
	  {
	    // if the player does not move, nothing to do
	    GdElement what = get_dir(cave, x, y, player_move);
	    GdElement remains = O_NONE;

	    if (player_fire)
	    {
	      // placing a bomb into empty space or dirt
	      if (is_like_space(cave, x, y, player_move) ||
		  is_like_dirt(cave, x, y, player_move))
	      {
		store_dir(cave, x, y, player_move, O_BOMB_TICK_1);

		// placed bomb, he is normal player again
		store(cave, x, y, O_PLAYER);
		gd_sound_play(cave, GD_S_BOMB_PLACING, O_BOMB, x, y);
	      }

	      break;
	    }

	    // pushing and collecting
	    // if we are 'eating' a teleporter, and the function returns true
	    // (teleporting worked), break here
	    if (what == O_TELEPORTER && do_teleporter(cave, x, y, player_move))
	      break;

	    // player fire is false...
	    if (do_push(cave, x, y, player_move, FALSE))
	    {
	      remains = O_SPACE;
	    }
	    else
	    {
              // get element. if cannot get, player_eat_element will return the same
              remains = player_eat_element(cave, what, x, y, player_move);
	    }

	    // if element changed, OR there is space, move.
	    if (remains != O_NONE)
	    {
	      // if anything changed, apply the change.
	      move(cave, x, y, player_move, O_PLAYER_BOMB);
	    }
	  }
	  break;

	case O_PLAYER_ROCKET_LAUNCHER:
	  // much simpler; cannot snap-push stones
	  if (cave->kill_player)
	  {
	    explode(cave, x, y);

	    break;
	  }

	  cave->player_seen_ago = 0;

	  // bd4 intermission caves have many players. so if one of them has exited,
	  // do not change the flag anymore. so this if () is needed
	  if (cave->player_state != GD_PL_EXITED)
	    cave->player_state = GD_PL_LIVING;

	  // firing a rocket?
	  if (player_move != GD_MV_STILL)
	  {
	    // if the player does not move, nothing to do
	    GdElement what = get_dir(cave, x, y, player_move);
	    GdElement remains = O_NONE;

	    // to fire a rocket, diagonal movement should not be allowed.
	    // so either x or y must be zero
	    if (player_fire)
	    {
	      // placing a rocket into empty space
	      if (is_like_space(cave, x, y, player_move))
	      {
                boolean rocket_launched = TRUE;

		switch (player_move)
		{
		  case GD_MV_RIGHT:
		    store_dir(cave, x, y, player_move, O_ROCKET_1);
                    if (cave->rockets_collected > 0)
                      cave->rockets_collected--;
		    else if (!cave->infinite_rockets)
		      store(cave, x, y, O_PLAYER);
		    break;

		  case GD_MV_UP:
		    store_dir(cave, x, y, player_move, O_ROCKET_2);
                    if (cave->rockets_collected > 0)
                      cave->rockets_collected--;
		    else if (!cave->infinite_rockets)
		      store(cave, x, y, O_PLAYER);
		    break;

		  case GD_MV_LEFT:
		    store_dir(cave, x, y, player_move, O_ROCKET_3);
                    if (cave->rockets_collected > 0)
                      cave->rockets_collected--;
		    else if (!cave->infinite_rockets)
		      store(cave, x, y, O_PLAYER);
		    break;

		  case GD_MV_DOWN:
		    store_dir(cave, x, y, player_move, O_ROCKET_4);
                    if (cave->rockets_collected > 0)
                      cave->rockets_collected--;
		    else if (!cave->infinite_rockets)
		      store(cave, x, y, O_PLAYER);
		    break;

		  default:
		    // cannot fire in other directions
                    rocket_launched = FALSE;
		    break;
		}

                if (rocket_launched)
                  gd_sound_play(cave, GD_S_BOMB_PLACING, O_BOMB, x, y);
	      }

	      // a player with rocket launcher cannot snap elements, so stop here
	      break;
	    }

	    // pushing and collecting
	    // if we are 'eating' a teleporter, and the function returns true
	    // (teleporting worked), break here
	    if (what == O_TELEPORTER && do_teleporter(cave, x, y, player_move))
	      break;

	    // player fire is false...
	    if (do_push(cave, x, y, player_move, FALSE))
	    {
	      remains = O_SPACE;
	    }
	    else
	    {
	      // get element. if cannot get, player_eat_element will return the same
	      remains = player_eat_element(cave, what, x, y, player_move);
	    }

	    // if something changed, OR there is space, move.
	    if (remains != O_NONE)
	    {
	      // if anything changed, apply the change.
	      move(cave, x, y, player_move, O_PLAYER_ROCKET_LAUNCHER);
	    }
	  }
	  break;

	case O_PLAYER_STIRRING:
	  if (cave->kill_player)
	  {
	    explode(cave, x, y);

	    break;
	  }

	  // stirring sound
	  gd_sound_play(cave, GD_S_STIRRING, O_PLAYER_STIRRING, x, y);

	  cave->player_seen_ago = 0;

	  // bd4 intermission caves have many players. so if one of them has exited,
	  // do not change the flag anymore. so this if () is needed
	  if (cave->player_state != GD_PL_EXITED)
	    cave->player_state = GD_PL_LIVING;

	  if (player_fire)
	  {
	    // player "exits" stirring the pot by pressing fire
	    cave->gravity_disabled = FALSE;
	    store(cave, x, y, O_PLAYER);
	    cave->gravity_switch_active = TRUE;
	  }
	  break;

	  // player holding pneumatic hammer
	case O_PLAYER_PNEUMATIC_LEFT:
	case O_PLAYER_PNEUMATIC_RIGHT:
	  // usual player stuff
	  if (cave->kill_player)
	  {
	    explode(cave, x, y);

	    break;
	  }

	  cave->player_seen_ago = 0;

	  if (cave->player_state != GD_PL_EXITED)
	    cave->player_state = GD_PL_LIVING;

	  // if hammering time is up, becomes a normal player again.
	  if (cave->pneumatic_hammer_active_delay == 0)
	    store(cave, x, y, O_PLAYER);

	  break;

	  // the active pneumatic hammer itself
	case O_PNEUMATIC_ACTIVE_RIGHT:
	case O_PNEUMATIC_ACTIVE_LEFT:
          // pneumatic hammer sound
          if (cave->pneumatic_hammer_active_delay > 0)
            gd_sound_play(cave, GD_S_PNEUMATIC_HAMMER, O_PNEUMATIC_HAMMER, -1, -1);

	  if (cave->pneumatic_hammer_active_delay == 0)
	  {
	    // pneumatic hammer element disappears
	    store(cave, x, y, O_SPACE);

	    // which is the new element which appears after that one is hammered?
	    GdElement new_elem = gd_element_get_hammered(get_dir(cave, x, y, GD_MV_DOWN));

	    // if there is a new element, display it
	    // O_NONE might be returned, for example if the element being
	    // hammered explodes during hammering (by a nearby explosion)
	    if (new_elem != O_NONE)
	    {
	      store_dir(cave, x, y, GD_MV_DOWN, new_elem);

	      // and if walls reappear, remember it in array
              // y + 1 is down
	      if (cave->hammered_walls_reappear)
	      {
		int wall_y = (y + 1) % cave->h;

		cave->hammered_reappear[wall_y][x] = cave->hammered_wall_reappear_frame;
	      }
	    }
	  }
	  break;


	  // ======================================================================================
	  //    S T O N E S,   D I A M O N D S
	  // ======================================================================================

	case O_STONE:           // standing stone
	  do_start_fall(cave, x, y, cave->gravity, cave->stone_falling_effect);
	  break;

	case O_MEGA_STONE:      // standing mega_stone
	  do_start_fall(cave, x, y, cave->gravity, O_MEGA_STONE_F);
	  break;

	case O_LIGHT_STONE:     // standing light_stone
	  do_start_fall(cave, x, y, cave->gravity, O_LIGHT_STONE_F);
	  break;

	case O_DIAMOND:         // standing diamond
	  do_start_fall(cave, x, y, cave->gravity, cave->diamond_falling_effect);
	  break;

	case O_NUT:             // standing nut
	  do_start_fall(cave, x, y, cave->gravity, O_NUT_F);
	  break;

	case O_DIRT_BALL:       // standing dirt ball
	  do_start_fall(cave, x, y, cave->gravity, O_DIRT_BALL_F);
	  break;

	case O_DIRT_LOOSE:      // standing loose dirt
	  do_start_fall(cave, x, y, cave->gravity, O_DIRT_LOOSE_F);
	  break;

	case O_FLYING_STONE:    // standing stone
	  do_start_fall(cave, x, y, opposite[cave->gravity], O_FLYING_STONE_F);
	  break;

	case O_FLYING_DIAMOND:  // standing diamond
	  do_start_fall(cave, x, y, opposite[cave->gravity], O_FLYING_DIAMOND_F);
	  break;


	  // ======================================================================================
	  //    F A L L I N G    E L E M E N T S,    F L Y I N G   S T O N E S,   D I A M O N D S
	  // ======================================================================================

	case O_DIRT_BALL_F:     // falling dirt ball
	  if (!cave->gravity_disabled)
	    do_fall_roll_or_stop(cave, x, y, cave->gravity, O_DIRT_BALL);
	  break;

	case O_DIRT_LOOSE_F:    // falling loose dirt
	  if (!cave->gravity_disabled)
	    do_fall_roll_or_stop(cave, x, y, cave->gravity, O_DIRT_LOOSE);
	  break;

	case O_STONE_F:         // falling stone
	  if (!cave->gravity_disabled)
	  {
	    if (do_fall_try_crush_voodoo(cave, x, y, cave->gravity))
	      break;

	    if (do_fall_try_crack_nut(cave, x, y, cave->gravity, cave->stone_bouncing_effect))
	      break;

	    if (do_fall_try_magic(cave, x, y, cave->gravity, cave->magic_stone_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, cave->gravity))
	      break;

	    do_fall_roll_or_stop(cave, x, y, cave->gravity, cave->stone_bouncing_effect);
	  }
	  break;

	case O_MEGA_STONE_F:    // falling mega
	  if (!cave->gravity_disabled)
	  {
	    if (do_fall_try_crush_voodoo(cave, x, y, cave->gravity))
	      break;

	    if (do_fall_try_crack_nut(cave, x, y, cave->gravity, O_MEGA_STONE))
	      break;

	    if (do_fall_try_magic(cave, x, y, cave->gravity, cave->magic_mega_stone_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, cave->gravity))
	      break;

	    do_fall_roll_or_stop(cave, x, y, cave->gravity, O_MEGA_STONE);
	  }
	  break;

	case O_LIGHT_STONE_F:    // falling light stone
	  if (!cave->gravity_disabled)
	  {
	    if (do_fall_try_crush_voodoo(cave, x, y, cave->gravity))
	      break;

	    if (do_fall_try_crack_nut(cave, x, y, cave->gravity, O_LIGHT_STONE))
	      break;

	    if (do_fall_try_magic(cave, x, y, cave->gravity, cave->magic_light_stone_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, cave->gravity))
	      break;

	    do_fall_roll_or_stop(cave, x, y, cave->gravity, O_LIGHT_STONE);
	  }
	  break;

	case O_DIAMOND_F:       // falling diamond
	  if (!cave->gravity_disabled)
	  {
	    if (do_fall_try_eat_voodoo(cave, x, y, cave->gravity))
	      break;

	    if (do_fall_try_magic(cave, x, y, cave->gravity, cave->magic_diamond_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, cave->gravity))
	      break;

	    do_fall_roll_or_stop(cave, x, y, cave->gravity, cave->diamond_bouncing_effect);
	  }
	  break;

	case O_NUT_F:           // falling nut
	  if (!cave->gravity_disabled)
	  {
	    if (do_fall_try_magic(cave, x, y, cave->gravity, cave->magic_nut_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, cave->gravity))
	      break;

	    do_fall_roll_or_stop(cave, x, y, cave->gravity, O_NUT);
	  }
	  break;

	case O_FLYING_STONE_F:  // falling stone
	  if (!cave->gravity_disabled)
	  {
	    GdDirection fall_dir = opposite[cave->gravity];

	    if (do_fall_try_crush_voodoo(cave, x, y, fall_dir))
	      break;

	    if (do_fall_try_crack_nut(cave, x, y, fall_dir, O_FLYING_STONE))
	      break;

	    if (do_fall_try_magic(cave, x, y, fall_dir, cave->magic_flying_stone_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, fall_dir))
	      break;

	    do_fall_roll_or_stop(cave, x, y, fall_dir, O_FLYING_STONE);
	  }
	  break;

	case O_FLYING_DIAMOND_F:    // falling diamond
	  if (!cave->gravity_disabled)
	  {
	    GdDirection fall_dir = opposite[cave->gravity];

	    if (do_fall_try_eat_voodoo(cave, x, y, fall_dir))
	      break;

	    if (do_fall_try_magic(cave, x, y, fall_dir, cave->magic_flying_diamond_to))
	      break;

	    if (do_fall_try_crush(cave, x, y, fall_dir))
	      break;

	    do_fall_roll_or_stop(cave, x, y, fall_dir, O_FLYING_DIAMOND);
	  }
	  break;


	  // ======================================================================================
	  //    N I T R O    P A C K
	  // ======================================================================================

	case O_NITRO_PACK:      // standing nitro pack
	  do_start_fall(cave, x, y, cave->gravity, O_NITRO_PACK_F);
	  break;

	case O_NITRO_PACK_F:    // falling nitro pack
	  if (!cave->gravity_disabled)
	  {
	    if (is_like_space(cave, x, y, cave->gravity))
            {
              // if space, falling further
	      move(cave, x, y, cave->gravity, O_NITRO_PACK_F);
            }
	    else if (do_fall_try_magic(cave, x, y, cave->gravity, cave->magic_nitro_pack_to))
	    {
	      // try magic wall; if true, function did the work
	    }
	    else if (is_like_dirt(cave, x, y, cave->gravity))
	    {
	      // falling on a dirt, it does NOT explode - just stops at its place.
	      store(cave, x, y, O_NITRO_PACK);
	      play_sound_of_element(cave, O_NITRO_PACK, x, y);
	    }
	    else
            {
	      // falling on any other element it explodes
	      explode(cave, x, y);
            }
	  }
	  break;

	case O_NITRO_PACK_EXPLODE:    // a triggered nitro pack
	  explode(cave, x, y);
	  break;


	  // ======================================================================================
	  //    C R E A T U R E S
	  // ======================================================================================

	case O_COW_1:
	case O_COW_2:
	case O_COW_3:
	case O_COW_4:
	  // if cannot move in any direction, becomes an enclosed cow
	  if (!is_like_space(cave, x, y, GD_MV_UP) &&
	      !is_like_space(cave, x, y, GD_MV_DOWN) &&
	      !is_like_space(cave, x, y, GD_MV_LEFT) &&
	      !is_like_space(cave, x, y, GD_MV_RIGHT))
	  {
	    store(cave, x, y, O_COW_ENCLOSED_1);
	  }
	  else
	  {
	    // THIS IS THE CREATURE MOVE thing copied.
	    const GdDirection *creature_move;
	    boolean ccw = rotates_ccw(cave, x, y);    // check if default is counterclockwise
	    GdElement base;    // base element number (which is like O_***_1)
	    int dir, dirn, dirp;    // direction

	    base = O_COW_1;

	    dir = get(cave, x, y) - base;    // facing where
	    creature_move = (cave->creatures_backwards ? creature_chdir : creature_dir);

	    // now change direction if backwards
	    if (cave->creatures_backwards)
	      ccw = !ccw;

	    if (ccw)
	    {
	      dirn = (dir + 3) & 3;    // fast turn
	      dirp = (dir + 1) & 3;    // slow turn
	    }
	    else
	    {
	      dirn = (dir + 1) & 3;    // fast turn
	      dirp = (dir + 3) & 3;    // slow turn
	    }

	    if (is_like_space(cave, x, y, creature_move[dirn]))
            {
              // turn and move to preferred dir
	      move(cave, x, y, creature_move[dirn], base + dirn);
            }
	    else if (is_like_space(cave, x, y, creature_move[dir]))
            {
              // go on
	      move(cave, x, y, creature_move[dir], base + dir);
            }
	    else
            {
              // turn in place if nothing else possible
	      store(cave, x, y, base + dirp);
            }

            gd_sound_play(cave, GD_S_COW, O_COW, x, y);
	  }
	  break;

	  // enclosed cows wait some time before turning to a skeleton
	case O_COW_ENCLOSED_1:
	case O_COW_ENCLOSED_2:
	case O_COW_ENCLOSED_3:
	case O_COW_ENCLOSED_4:
	case O_COW_ENCLOSED_5:
	case O_COW_ENCLOSED_6:
	  if (is_like_space(cave, x, y, GD_MV_UP) ||
	      is_like_space(cave, x, y, GD_MV_LEFT) ||
	      is_like_space(cave, x, y, GD_MV_RIGHT) ||
	      is_like_space(cave, x, y, GD_MV_DOWN))
	    store(cave, x, y, O_COW_1);
	  else
	    next(cave, x, y);
	  break;

	case O_COW_ENCLOSED_7:
	  if (is_like_space(cave, x, y, GD_MV_UP) ||
	      is_like_space(cave, x, y, GD_MV_LEFT) ||
	      is_like_space(cave, x, y, GD_MV_RIGHT) ||
	      is_like_space(cave, x, y, GD_MV_DOWN))
	    store(cave, x, y, O_COW_1);
	  else
	    store(cave, x, y, O_SKELETON);
	  break;

	case O_FIREFLY_1:
	case O_FIREFLY_2:
	case O_FIREFLY_3:
	case O_FIREFLY_4:
	case O_ALT_FIREFLY_1:
	case O_ALT_FIREFLY_2:
	case O_ALT_FIREFLY_3:
	case O_ALT_FIREFLY_4:
	case O_BUTTER_1:
	case O_BUTTER_2:
	case O_BUTTER_3:
	case O_BUTTER_4:
	case O_ALT_BUTTER_1:
	case O_ALT_BUTTER_2:
	case O_ALT_BUTTER_3:
	case O_ALT_BUTTER_4:
	case O_STONEFLY_1:
	case O_STONEFLY_2:
	case O_STONEFLY_3:
	case O_STONEFLY_4:
	  // check if touches a voodoo
	  if (get_dir(cave, x, y, GD_MV_LEFT)  == O_VOODOO ||
	      get_dir(cave, x, y, GD_MV_RIGHT) == O_VOODOO ||
	      get_dir(cave, x, y, GD_MV_UP)    == O_VOODOO ||
	      get_dir(cave, x, y, GD_MV_DOWN)  == O_VOODOO)
	    cave->voodoo_touched = TRUE;

	  // check if touches something bad and should explode (includes voodoo by the flags)
	  if (blows_up_flies(cave, x, y, GD_MV_DOWN) ||
	      blows_up_flies(cave, x, y, GD_MV_UP) ||
	      blows_up_flies(cave, x, y, GD_MV_LEFT) ||
	      blows_up_flies(cave, x, y, GD_MV_RIGHT))
          {
	    explode(cave, x, y);
          }
	  // otherwise move
	  else
	  {
	    const GdDirection *creature_move;
	    boolean ccw = rotates_ccw(cave, x, y);    // check if default is counterclockwise
	    GdElement base = -1;    // base element number (which is like O_***_1)
	    int dir, dirn, dirp;    // direction

	    if (get(cave, x, y) >= O_FIREFLY_1 &&
		get(cave, x, y) <= O_FIREFLY_4)
	      base = O_FIREFLY_1;
	    else if (get(cave, x, y) >= O_BUTTER_1 &&
		     get(cave, x, y) <= O_BUTTER_4)
	      base = O_BUTTER_1;
	    else if (get(cave, x, y) >= O_STONEFLY_1 &&
		     get(cave, x, y) <= O_STONEFLY_4)
	      base = O_STONEFLY_1;
	    else if (get(cave, x, y) >= O_ALT_FIREFLY_1 &&
		     get(cave, x, y) <= O_ALT_FIREFLY_4)
	      base = O_ALT_FIREFLY_1;
	    else if (get(cave, x, y) >= O_ALT_BUTTER_1 &&
		     get(cave, x, y) <= O_ALT_BUTTER_4)
	      base = O_ALT_BUTTER_1;

	    dir = get(cave, x, y) - base;    // facing where
	    creature_move = (cave->creatures_backwards ? creature_chdir : creature_dir);

	    // now change direction if backwards
	    if (cave->creatures_backwards)
	      ccw = !ccw;

	    if (ccw)
	    {
	      dirn = (dir + 3) & 3;    // fast turn
	      dirp = (dir + 1) & 3;    // slow turn
	    }
	    else
	    {
	      dirn = (dir + 1) & 3;    // fast turn
	      dirp = (dir + 3) & 3;    // slow turn
	    }

	    if (is_like_space(cave, x, y, creature_move[dirn]))
            {
              // turn and move to preferred dir
	      move(cave, x, y, creature_move[dirn], base + dirn);
            }
	    else if (is_like_space(cave, x, y, creature_move[dir]))
            {
              // go on
	      move(cave, x, y, creature_move[dir], base + dir);
            }
	    else
            {
              // turn in place if nothing else possible
	      store(cave, x, y, base + dirp);
            }

            if (base == O_FIREFLY_1)
              gd_sound_play(cave, GD_S_FIREFLY, O_FIREFLY, x, y);
            else if (base == O_ALT_FIREFLY_1)
              gd_sound_play(cave, GD_S_ALT_FIREFLY, O_ALT_FIREFLY, x, y);
            else if (base == O_BUTTER_1)
              gd_sound_play(cave, GD_S_BUTTER, O_BUTTER, x, y);
            else if (base == O_ALT_BUTTER_1)
              gd_sound_play(cave, GD_S_ALT_BUTTER, O_ALT_BUTTER, x, y);
            else if (base == O_STONEFLY_1)
              gd_sound_play(cave, GD_S_STONEFLY, O_STONEFLY, x, y);
	  }
	  break;

	case O_WAITING_STONE:
	  if (is_like_space(cave, x, y, grav_compat))
	  {
	    // beginning to fall
	    // it wakes up.
	    move(cave, x, y, grav_compat, O_CHASING_STONE);
	  }
	  else if (sloped(cave, x, y, grav_compat, opposite[grav_compat]))
	  {
	    // rolling down a brick wall or a stone
	    if (sloped(cave, x, y, grav_compat, cw_fourth[grav_compat]) &&
		is_like_space(cave, x, y, cw_fourth[grav_compat]) &&
		is_like_space(cave, x, y, cw_eighth[grav_compat]))
	    {
	      // maybe rolling left - see case O_STONE to understand why we use cw_fourth here
	      move(cave, x, y, cw_fourth[grav_compat], O_WAITING_STONE);
	    }
	    else if (sloped(cave, x, y, grav_compat, ccw_fourth[grav_compat]) &&
		     is_like_space(cave, x, y, ccw_fourth[grav_compat]) &&
		     is_like_space(cave, x, y, ccw_eighth[grav_compat]))
	    {
	      // or maybe right
	      move(cave, x, y, ccw_fourth[grav_compat], O_WAITING_STONE);
	    }
	  }
	  break;

	case O_CHASING_STONE:
	  {
	    int px = cave->player_x_mem[0];
	    int py = cave->player_y_mem[0];
	    boolean horizontal = gd_rand_boolean(cave->random);
	    boolean dont_move = FALSE;
	    int i = 3;

	    // try to move...
	    while (1)
	    {
	      if (horizontal)
	      {
		// ------------------------------------------------------------
		// check for a horizontal movement
		// ------------------------------------------------------------

		if (px == x)
		{
		  // if coordinates are the same
		  i -= 1;
		  horizontal = !horizontal;

		  if (i == 2)
		    continue;
		}
		else
		{
		  if (px > x && is_like_space(cave, x, y, GD_MV_RIGHT))
		  {
		    move(cave, x, y, GD_MV_RIGHT, O_CHASING_STONE);
		    dont_move = TRUE;

		    break;
		  }
		  else if (px < x && is_like_space(cave, x, y, GD_MV_LEFT))
		  {
		    move(cave, x, y, GD_MV_LEFT, O_CHASING_STONE);
		    dont_move = TRUE;

		    break;
		  }
		  else
		  {
		    i -= 2;

		    if (i == 1)
		    {
		      horizontal = !horizontal;

		      continue;
		    }
		  }
		}
	      }
	      else
	      {
		// ------------------------------------------------------------
		// check for a vertical movement
		// ------------------------------------------------------------

		if (py == y)
		{
		  // if coordinates are the same
		  i -= 1;
		  horizontal = !horizontal;

		  if (i == 2)
		    continue;
		}
		else
		{
		  if (py > y && is_like_space(cave, x, y, GD_MV_DOWN))
		  {
		    move(cave, x, y, GD_MV_DOWN, O_CHASING_STONE);
		    dont_move = TRUE;

		    break;
		  }
		  else if (py < y && is_like_space(cave, x, y, GD_MV_UP))
		  {
		    move(cave, x, y, GD_MV_UP, O_CHASING_STONE);
		    dont_move = TRUE;

		    break;
		  }
		  else
		  {
		    i -= 2;

		    if (i == 1)
		    {
		      horizontal = !horizontal;

		      continue;
		    }
		  }
		}
	      }

	      if (i != 0)
		dont_move = TRUE;

	      break;
	    }

	    // if we should move in both directions, but can not move in any, stop.
	    if (!dont_move)
	    {
	      if (horizontal)
	      {
		// check for horizontal
		if (x >= px)
		{
		  if (is_like_space(cave, x, y, GD_MV_UP) &&
		      is_like_space(cave, x, y, GD_MV_UP_LEFT))
		    move(cave, x, y, GD_MV_UP, O_CHASING_STONE);
		  else if (is_like_space(cave, x, y, GD_MV_DOWN) &&
			   is_like_space(cave, x, y, GD_MV_DOWN_LEFT))
		    move(cave, x, y, GD_MV_DOWN, O_CHASING_STONE);
		}
		else
		{
		  if (is_like_space(cave, x, y, GD_MV_UP) &&
		      is_like_space(cave, x, y, GD_MV_UP_RIGHT))
		    move(cave, x, y, GD_MV_UP, O_CHASING_STONE);
		  else if (is_like_space(cave, x, y, GD_MV_DOWN) &&
			   is_like_space(cave, x, y, GD_MV_DOWN_RIGHT))
		    move(cave, x, y, GD_MV_DOWN, O_CHASING_STONE);
		}
	      }
	      else
	      {
		// check for vertical
		if (y >= py)
		{
		  if (is_like_space(cave, x, y, GD_MV_LEFT) &&
		      is_like_space(cave, x, y, GD_MV_UP_LEFT))
		    move(cave, x, y, GD_MV_LEFT, O_CHASING_STONE);
		  else if (is_like_space(cave, x, y, GD_MV_RIGHT) &&
			   is_like_space(cave, x, y, GD_MV_UP_RIGHT))
		    move(cave, x, y, GD_MV_RIGHT, O_CHASING_STONE);
		}
		else
		{
		  if (is_like_space(cave, x, y, GD_MV_LEFT) &&
		      is_like_space(cave, x, y, GD_MV_DOWN_LEFT))
		    move(cave, x, y, GD_MV_LEFT, O_CHASING_STONE);
		  else if (is_like_space(cave, x, y, GD_MV_RIGHT) &&
			   is_like_space(cave, x, y, GD_MV_DOWN_RIGHT))
		    move(cave, x, y, GD_MV_RIGHT, O_CHASING_STONE);
		}
	      }
	    }
	  }
	  break;

	case O_REPLICATOR:
	  if (cave->replicators_wait_frame == 0 &&
	      cave->replicators_active &&
	      !cave->gravity_disabled)
	  {
	    // only replicate if space is under it.
	    // do not replicate players!
	    // also obeys gravity settings.
	    // only replicate element if it is not a scanned one
	    if (is_like_space(cave, x, y, cave->gravity) &&
		!is_player_dir(cave, x, y, opposite[cave->gravity]) &&
		!is_scanned_dir(cave, x, y, opposite[cave->gravity]))
	    {
	      store_dir(cave, x, y, cave->gravity, get_dir(cave, x, y, opposite[cave->gravity]));
	      gd_sound_play(cave, GD_S_REPLICATOR, O_REPLICATOR, x, y);
	    }
	  }
	  break;

	case O_BITER_1:
	case O_BITER_2:
	case O_BITER_3:
	case O_BITER_4:
	  if (cave->biters_wait_frame == 0)
	  {
	    static GdDirection biter_move[] =
	    {
	      GD_MV_UP,
	      GD_MV_RIGHT,
	      GD_MV_DOWN,
	      GD_MV_LEFT
	    };

	    // direction, last two bits 0..3
	    int dir = get(cave, x, y) - O_BITER_1;
	    int dirn = (dir + 3) & 3;
	    int dirp = (dir + 1) & 3;
	    GdElement made_sound_of = O_NONE;
	    int i;

	    for (i = 0; i < ARRAY_SIZE(biter_try); i++)
	    {
	      if (is_like_element(cave, x, y, biter_move[dir], biter_try[i]))
	      {
		move(cave, x, y, biter_move[dir], O_BITER_1 + dir);
		if (biter_try[i] != O_SPACE)
		  made_sound_of = O_BITER_1;    // sound of a biter eating
		break;
	      }
	      else if (is_like_element(cave, x, y, biter_move[dirn], biter_try[i]))
	      {
		move(cave, x, y, biter_move[dirn], O_BITER_1 + dirn);
		if (biter_try[i] != O_SPACE)
		  made_sound_of = O_BITER_1;    // sound of a biter eating
		break;
	      }
	      else if (is_like_element(cave, x, y, biter_move[dirp], biter_try[i]))
	      {
		move(cave, x, y, biter_move[dirp], O_BITER_1 + dirp);
		if (biter_try[i] != O_SPACE)
		  made_sound_of = O_BITER_1;    // sound of a biter eating
		break;
	      }
	    }

	    if (i == ARRAY_SIZE(biter_try))
            {
	      // i = number of elements in array: could not move, so just turn
	      store(cave, x, y, O_BITER_1 + dirp);
            }
	    else if (biter_try[i] == O_STONE)
	    {
	      // if there was a stone there, where we moved...
	      // do not eat stones, just throw them back
	      store(cave, x, y, O_STONE);
	      made_sound_of = O_STONE;
	    }

	    // if biter did move, we had sound. play it.
	    if (made_sound_of != O_NONE)
	      play_sound_of_element(cave, made_sound_of, x, y);
            else
              gd_sound_play(cave, GD_S_BITER, O_BITER, x, y);
	  }
	  break;

	case O_DRAGONFLY_1:
	case O_DRAGONFLY_2:
	case O_DRAGONFLY_3:
	case O_DRAGONFLY_4:
	  // check if touches a voodoo
	  if (get_dir(cave, x, y, GD_MV_LEFT)  == O_VOODOO ||
	      get_dir(cave, x, y, GD_MV_RIGHT) == O_VOODOO ||
	      get_dir(cave, x, y, GD_MV_UP)    == O_VOODOO ||
	      get_dir(cave, x, y, GD_MV_DOWN)  == O_VOODOO)
	    cave->voodoo_touched = TRUE;

	  // check if touches something bad and should explode (includes voodoo by the flags)
	  if (blows_up_flies(cave, x, y, GD_MV_DOWN) ||
	      blows_up_flies(cave, x, y, GD_MV_UP) ||
	      blows_up_flies(cave, x, y, GD_MV_LEFT) ||
	      blows_up_flies(cave, x, y, GD_MV_RIGHT))
          {
	    explode (cave, x, y);
          }
	  // otherwise move
	  else
	  {
	    const GdDirection *creature_move;
	    boolean ccw = rotates_ccw(cave, x, y);    // check if default is counterclockwise
	    GdElement base = O_DRAGONFLY_1;    // base element number (which is like O_***_1)
	    int dir, dirn;    // direction

	    dir = get(cave, x, y) - base;    // facing where
	    creature_move = (cave->creatures_backwards ? creature_chdir : creature_dir);

	    // now change direction if backwards
	    if (cave->creatures_backwards)
	      ccw = !ccw;

	    if (ccw)
	      dirn = (dir + 3) & 3;    // fast turn
	    else
	      dirn = (dir + 1) & 3;    // fast turn

	    // if can move forward, does so.
	    if (is_like_space(cave, x, y, creature_move[dir]))
            {
	      move(cave, x, y, creature_move[dir], base + dir);
            }
	    else
            {
	      // otherwise turns 90 degrees in place.
	      store(cave, x, y, base + dirn);
            }

            gd_sound_play(cave, GD_S_DRAGONFLY, O_DRAGONFLY, x, y);
	  }
	  break;

	case O_BLADDER:
	  store(cave, x, y, O_BLADDER_1);
	  break;

	case O_BLADDER_1:
	case O_BLADDER_2:
	case O_BLADDER_3:
	case O_BLADDER_4:
	case O_BLADDER_5:
	case O_BLADDER_6:
	case O_BLADDER_7:
	case O_BLADDER_8:
	  // bladder with any delay state: try to convert to clock.
	  if (is_like_element(cave, x, y, opposite[grav_compat], cave->bladder_converts_by) ||
	      is_like_element(cave, x, y, cw_fourth[grav_compat], cave->bladder_converts_by) ||
              is_like_element(cave, x, y, ccw_fourth[grav_compat], cave->bladder_converts_by))
	  {
	    // if touches the specified element, let it be a clock
	    store(cave, x, y, O_PRE_CLOCK_1);

	    // plays the bladder convert sound
	    play_sound_of_element(cave, O_PRE_CLOCK_1, x, y);
	  }
	  else
	  {
	    // is space over the bladder?
	    if (is_like_space(cave, x, y, opposite[grav_compat]))
	    {
	      if (get(cave, x, y) == O_BLADDER_8)
	      {
		// if it is a bladder 8, really move up
		move(cave, x, y, opposite[grav_compat], O_BLADDER_1);
		play_sound_of_element(cave, O_BLADDER, x, y);
	      }
	      else
              {
		// if smaller delay, just increase delay.
		next(cave, x, y);
              }
	    }
	    else
            {
	      // if not space, is something sloped over the bladder?
	      if (sloped_for_bladder(cave, x, y, opposite[grav_compat]) &&
		  sloped(cave, x, y, opposite[grav_compat], opposite[grav_compat]))
	      {
		if (sloped(cave, x, y, opposite[grav_compat], ccw_fourth[opposite[grav_compat]]) &&
		    is_like_space(cave, x, y, ccw_fourth[opposite[grav_compat]]) &&
		    is_like_space(cave, x, y, ccw_eighth[opposite[grav_compat]]))
		{
		  // rolling up, to left
		  if (get(cave, x, y) == O_BLADDER_8)
		  {
		    // if it is a bladder 8, really roll
		    move(cave, x, y, ccw_fourth[opposite[grav_compat]], O_BLADDER_8);
		    play_sound_of_element(cave, O_BLADDER, x, y);
		  }
		  else
                  {
		    // if smaller delay, just increase delay.
		    next(cave, x, y);
                  }
		}
		else if (sloped(cave, x, y, opposite[grav_compat], cw_fourth[opposite[grav_compat]]) &&
			 is_like_space(cave, x, y, cw_fourth[opposite[grav_compat]]) &&
			 is_like_space(cave, x, y, cw_eighth[opposite[grav_compat]]))
		{
		  // rolling up, to left
		  if (get(cave, x, y) == O_BLADDER_8)
		  {
		    // if it is a bladder 8, really roll
		    move(cave, x, y, cw_fourth[opposite[grav_compat]], O_BLADDER_8);
		    play_sound_of_element(cave, O_BLADDER, x, y);
		  }
		  else
                  {
		    // if smaller delay, just increase delay.
		    next(cave, x, y);
                  }
		}
	      }
              // no space, no sloped thing over it - store bladder 1 and that is for now.
	      else
              {
		store(cave, x, y, O_BLADDER_1);
              }
            }
	  }
	  break;

	case O_GHOST:
	  if (blows_up_flies(cave, x, y, GD_MV_DOWN) ||
	      blows_up_flies(cave, x, y, GD_MV_UP) ||
	      blows_up_flies(cave, x, y, GD_MV_LEFT) ||
	      blows_up_flies(cave, x, y, GD_MV_RIGHT))
          {
	    explode (cave, x, y);
          }
	  else
	  {
	    int i;

	    // the ghost is given four possibilities to move.
	    for (i = 0; i < 4; i++)
	    {
	      static GdDirection dirs[] =
	      {
		GD_MV_UP,
		GD_MV_DOWN,
		GD_MV_LEFT,
		GD_MV_RIGHT
	      };

	      GdDirection random_dir = dirs[gd_rand_int_range(cave->random, 0, ARRAY_SIZE(dirs))];

	      if (is_like_space(cave, x, y, random_dir))
	      {
		move(cave, x, y, random_dir, O_GHOST);

		break;    // ghost did move -> exit loop
	      }
	    }
	  }
	  break;


	  // ======================================================================================
	  //    A C T I V E    E L E M E N T S
	  // ======================================================================================

	case O_AMOEBA:
	  // emulating buggy BD1 amoeba + magic wall behaviour (also used by Krissz engine)
	  if (cave->convert_amoeba_this_frame && amoeba_found_enclosed)
	  {
	    store(cave, x, y, cave->amoeba_enclosed_effect);

	    break;
	  }

	  amoeba_count++;

	  switch (cave->amoeba_state)
	  {
	    case GD_AM_TOO_BIG:
	      store(cave, x, y, cave->amoeba_too_big_effect);
	      break;

	    case GD_AM_ENCLOSED:
	      store(cave, x, y, cave->amoeba_enclosed_effect);
	      break;

	    case GD_AM_SLEEPING:
	    case GD_AM_AWAKE:
	      // if no amoeba found during THIS SCAN yet, which was able to grow, check this one.
	      if (amoeba_found_enclosed)
              {
		// if still found enclosed, check all four directions, if this one is able to grow.
		if (amoeba_eats(cave, x, y, GD_MV_UP) ||
		    amoeba_eats(cave, x, y, GD_MV_DOWN) ||
		    amoeba_eats(cave, x, y, GD_MV_LEFT) ||
		    amoeba_eats(cave, x, y, GD_MV_RIGHT))
		{
		  // not enclosed. this is a local (per scan) flag!
		  amoeba_found_enclosed = FALSE;
		  cave->amoeba_state = GD_AM_AWAKE;
		}
              }

	      // if alive, check in which dir to grow (or not)
	      if (cave->amoeba_state == GD_AM_AWAKE)
	      {
                if (game_bd.game->use_krissz_engine)
                {
                  // Krissz engine: special amoeba handling

                  if (amoeba_eats(cave, x, y, GD_MV_UP) ||
                      amoeba_eats(cave, x, y, GD_MV_DOWN) ||
                      amoeba_eats(cave, x, y, GD_MV_LEFT) ||
                      amoeba_eats(cave, x, y, GD_MV_RIGHT))
                  {
                    switch (get_krissz_random(cave, 4))
                    {
                      // not yet decided to grow, but first choose a random direction.

                      case 1:
                        // let this be up. numbers indifferent.
                        if (amoeba_eats(cave, x, y, GD_MV_UP) && krissz_amoeba_grows(cave))
                        {
                          store_dir(cave, x, y, GD_MV_UP, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;

                      case 2:
                        // down
                        if (amoeba_eats(cave, x, y, GD_MV_DOWN) && krissz_amoeba_grows(cave))
                        {
                          store_dir(cave, x, y, GD_MV_DOWN, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;

                      case 3:
                        // left
                        if (amoeba_eats(cave, x, y, GD_MV_LEFT) && krissz_amoeba_grows(cave))
                        {
                          store_dir(cave, x, y, GD_MV_LEFT, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;

                      case 4:
                        // right
                        if (amoeba_eats(cave, x, y, GD_MV_RIGHT) && krissz_amoeba_grows(cave))
                        {
                          store_dir(cave, x, y, GD_MV_RIGHT, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;
                    }
                  }
                }
                else
                {
                  // normal BD engine amoeba handling

                  if (gd_rand_int_range(cave->random, 0, 1000000) < cave->amoeba_growth_prob)
                  {
                    switch (gd_rand_int_range(cave->random, 0, 4))
                    {
                      // decided to grow, choose a random direction.

                      case 0:
                        // let this be up. numbers indifferent.
                        if (amoeba_eats(cave, x, y, GD_MV_UP))
                        {
                          store_dir(cave, x, y, GD_MV_UP, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;

                      case 1:
                        // down
                        if (amoeba_eats(cave, x, y, GD_MV_DOWN))
                        {
                          store_dir(cave, x, y, GD_MV_DOWN, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;

                      case 2:
                        // left
                        if (amoeba_eats(cave, x, y, GD_MV_LEFT))
                        {
                          store_dir(cave, x, y, GD_MV_LEFT, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;

                      case 3:
                        // right
                        if (amoeba_eats(cave, x, y, GD_MV_RIGHT))
                        {
                          store_dir(cave, x, y, GD_MV_RIGHT, O_AMOEBA);
                          gd_sound_play(cave, GD_S_AMOEBA_GROWING, O_AMOEBA, -1, -1);
                        }
                        break;
                    }
                  }
                }
	      }
	      break;
	  }
	  break;

	case O_AMOEBA_2:
	  amoeba_2_count++;

	  // check if it is touching an amoeba, and explosion is enabled
	  if (cave->amoeba_2_explodes_by_amoeba &&
	      (is_like_element(cave, x, y, GD_MV_DOWN,  O_AMOEBA) ||
	       is_like_element(cave, x, y, GD_MV_UP,    O_AMOEBA) ||
	       is_like_element(cave, x, y, GD_MV_LEFT,  O_AMOEBA) ||
	       is_like_element(cave, x, y, GD_MV_RIGHT, O_AMOEBA)))
          {
	    explode (cave, x, y);
          }
	  else
          {
	    switch (cave->amoeba_2_state)
	    {
	      case GD_AM_TOO_BIG:
		store(cave, x, y, cave->amoeba_2_too_big_effect);
		break;

	      case GD_AM_ENCLOSED:
		store(cave, x, y, cave->amoeba_2_enclosed_effect);
		break;

	      case GD_AM_SLEEPING:
	      case GD_AM_AWAKE:
		// if no amoeba found during THIS SCAN yet, which was able to grow, check this one.
		if (amoeba_2_found_enclosed)
                {
		  if (amoeba_eats(cave, x, y, GD_MV_UP) ||
		      amoeba_eats(cave, x, y, GD_MV_DOWN) ||
		      amoeba_eats(cave, x, y, GD_MV_LEFT) ||
		      amoeba_eats(cave, x, y, GD_MV_RIGHT))
		  {
		    // not enclosed. this is a local (per scan) flag!
		    amoeba_2_found_enclosed = FALSE;
		    cave->amoeba_2_state = GD_AM_AWAKE;
		  }
                }

		// if it is alive, decide if it attempts to grow
		if (cave->amoeba_2_state == GD_AM_AWAKE)
                {
		  if (gd_rand_int_range(cave->random, 0, 1000000) < cave->amoeba_2_growth_prob)
		  {
		    switch (gd_rand_int_range(cave->random, 0, 4))
		    {
		      // decided to grow, choose a random direction.

		      case 0:
                        // let this be up. numbers indifferent.
			if (amoeba_eats(cave, x, y, GD_MV_UP))
                        {
			  store_dir(cave, x, y, GD_MV_UP, O_AMOEBA_2);
                          gd_sound_play(cave, GD_S_AMOEBA_2_GROWING, O_AMOEBA_2, -1, -1);
                        }
			break;

		      case 1:
                        // down
			if (amoeba_eats(cave, x, y, GD_MV_DOWN))
                        {
			  store_dir(cave, x, y, GD_MV_DOWN, O_AMOEBA_2);
                          gd_sound_play(cave, GD_S_AMOEBA_2_GROWING, O_AMOEBA_2, -1, -1);
                        }
			break;

		      case 2:
                        // left
			if (amoeba_eats(cave, x, y, GD_MV_LEFT))
                        {
			  store_dir(cave, x, y, GD_MV_LEFT, O_AMOEBA_2);
                          gd_sound_play(cave, GD_S_AMOEBA_2_GROWING, O_AMOEBA_2, -1, -1);
                        }
			break;

		      case 3:
                        // right
			if (amoeba_eats(cave, x, y, GD_MV_RIGHT))
                        {
			  store_dir(cave, x, y, GD_MV_RIGHT, O_AMOEBA_2);
                          gd_sound_play(cave, GD_S_AMOEBA_2_GROWING, O_AMOEBA_2, -1, -1);
                        }
			break;
		    }
		  }
                }
		break;
	    }
          }
	  break;

	case O_ACID:
	  // choose randomly, if it spreads
	  if (gd_rand_int_range(cave->random, 0, 1000000) <= cave->acid_spread_ratio)
	  {
	    // the current one explodes
	    store(cave, x, y, cave->acid_turns_to);

	    // and if neighbours are eaten, put acid there.
	    if (is_like_element(cave, x, y, GD_MV_UP, cave->acid_eats_this))
	    {
	      store_dir(cave, x, y, GD_MV_UP, O_ACID);
	      play_sound_of_element(cave, O_ACID, x, y);
	    }

	    if (is_like_element(cave, x, y, GD_MV_DOWN, cave->acid_eats_this))
	    {
	      store_dir(cave, x, y, GD_MV_DOWN, O_ACID);
	      play_sound_of_element(cave, O_ACID, x, y);
	    }

	    if (is_like_element(cave, x, y, GD_MV_LEFT, cave->acid_eats_this))
	    {
	      store_dir(cave, x, y, GD_MV_LEFT, O_ACID);
	      play_sound_of_element(cave, O_ACID, x, y);
	    }

	    if (is_like_element(cave, x, y, GD_MV_RIGHT, cave->acid_eats_this))
	    {
	      store_dir(cave, x, y, GD_MV_RIGHT, O_ACID);
	      play_sound_of_element(cave, O_ACID, x, y);
	    }
	  }
	  break;

	case O_WATER:
	  found_water = TRUE;

	  if (!cave->water_does_not_flow_down &&
	      is_like_space(cave, x, y, GD_MV_DOWN))
          {
	    // emulating the odd behaviour in crdr
	    store_dir(cave, x, y, GD_MV_DOWN, O_WATER_1);
          }

	  if (is_like_space(cave, x, y, GD_MV_UP))
	    store_dir(cave, x, y, GD_MV_UP, O_WATER_1);

	  if (is_like_space(cave, x, y, GD_MV_LEFT))
	    store_dir(cave, x, y, GD_MV_LEFT, O_WATER_1);

	  if (is_like_space(cave, x, y, GD_MV_RIGHT))
	    store_dir(cave, x, y, GD_MV_RIGHT, O_WATER_1);
	  break;

	case O_WATER_16:
	  store(cave, x, y, O_WATER);
	  break;

	case O_H_EXPANDING_WALL:
	case O_V_EXPANDING_WALL:
	case O_H_EXPANDING_STEEL_WALL:
	case O_V_EXPANDING_STEEL_WALL:
	  // checks first if direction is changed.
	  if (((get(cave, x, y) == O_H_EXPANDING_WALL ||
		get(cave, x, y) == O_H_EXPANDING_STEEL_WALL) &&
	       !cave->expanding_wall_changed) ||
	      ((get(cave, x, y) == O_V_EXPANDING_WALL ||
		get(cave, x, y) == O_V_EXPANDING_STEEL_WALL) &&
	       cave->expanding_wall_changed))
	  {
            // special case: check both directions when using old engine
            boolean check_both_directions = TRUE;

	    if (check_both_directions && is_like_space(cave, x, y, GD_MV_LEFT))
	    {
	      store_dir(cave, x, y, GD_MV_LEFT, get(cave, x, y));
	      play_sound_of_element(cave, get(cave, x, y), x, y);

              // if using new engine, skip checking other direction (like "if ... else if ...")
              check_both_directions = game_bd.game->use_old_engine;
	    }

	    if (check_both_directions && is_like_space(cave, x, y, GD_MV_RIGHT))
            {
	      store_dir(cave, x, y, GD_MV_RIGHT, get(cave, x, y));
	      play_sound_of_element(cave, get(cave, x, y), x, y);
	    }
	  }
	  else
	  {
            // special case: check both directions when using old engine
            boolean check_both_directions = TRUE;

	    if (check_both_directions && is_like_space(cave, x, y, GD_MV_UP))
            {
	      store_dir(cave, x, y, GD_MV_UP, get(cave, x, y));
	      play_sound_of_element(cave, get(cave, x, y), x, y);

              // if using new engine, skip checking other direction (like "if ... else if ...")
              check_both_directions = game_bd.game->use_old_engine;
	    }

	    if (check_both_directions && is_like_space(cave, x, y, GD_MV_DOWN))
            {
	      store_dir(cave, x, y, GD_MV_DOWN, get(cave, x, y));
	      play_sound_of_element(cave, get(cave, x, y), x, y);
	    }
	  }
	  break;

	case O_EXPANDING_WALL:
	case O_EXPANDING_STEEL_WALL:
	  // the wall which grows in all four directions.
	  if (is_like_space(cave, x, y, GD_MV_LEFT))
	  {
	    store_dir(cave, x, y, GD_MV_LEFT, get(cave, x, y));
	    play_sound_of_element(cave, get(cave, x, y), x, y);
	  }

	  if (is_like_space(cave, x, y, GD_MV_RIGHT)) {
	    store_dir(cave, x, y, GD_MV_RIGHT, get(cave, x, y));
	    play_sound_of_element(cave, get(cave, x, y), x, y);
	  }

	  if (is_like_space(cave, x, y, GD_MV_UP)) {
	    store_dir(cave, x, y, GD_MV_UP, get(cave, x, y));
	    play_sound_of_element(cave, get(cave, x, y), x, y);
	  }

	  if (is_like_space(cave, x, y, GD_MV_DOWN)) {
	    store_dir(cave, x, y, GD_MV_DOWN, get(cave, x, y));
	    play_sound_of_element(cave, get(cave, x, y), x, y);
	  }
	  break;

	case O_SLIME:
          // unpredictable: gd_rand_int_range
          // predictable: c64 predictable random generator.
          //    for predictable, a random number is generated,
          //    whether or not it is even possible that the stone will be able to pass.
          if (cave->slime_predictable ?
              ((gd_cave_c64_random(cave) & cave->slime_permeability_c64) == 0) :
              gd_rand_int_range(cave->random, 0, 1000000) < cave->slime_permeability)
	  {
	    GdDirection grav = cave->gravity;
	    GdDirection oppos = opposite[cave->gravity];

	    // space under the slime? elements may pass from top to bottom then.
	    if (is_like_space(cave, x, y, grav) && get_dir(cave, x, y, oppos) != O_SPACE)
	    {
	      int what_x = getx(cave, x + gd_dx[oppos], y + gd_dy[oppos]);
	      int what_y = gety(cave, x + gd_dx[oppos], y + gd_dy[oppos]);

	      if (get_dir(cave, x, y, oppos) == cave->slime_eats_1)
	      {
		// output a falling xy under
		store_dir(cave, what_x, what_y, twice[grav], cave->slime_converts_1);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	      else if (get_dir(cave, x, y, oppos) == cave->slime_eats_2)
	      {
		store_dir(cave, what_x, what_y, twice[grav], cave->slime_converts_2);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	      else if (get_dir(cave, x, y, oppos) == cave->slime_eats_3)
	      {
		store_dir(cave, what_x, what_y, twice[grav], cave->slime_converts_3);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	      else if (get_dir(cave, x, y, oppos) == cave->slime_eats_4)
	      {
		store_dir(cave, what_x, what_y, twice[grav], cave->slime_converts_4);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	      else if (get_dir(cave, x, y, oppos) == cave->slime_eats_5)
	      {
		store_dir(cave, what_x, what_y, twice[grav], cave->slime_converts_5);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	      else if (get_dir(cave, x, y, oppos) == O_WAITING_STONE)
	      {
		// waiting stones pass without awakening
		store_dir(cave, what_x, what_y, twice[grav], O_WAITING_STONE);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	      else if (get_dir(cave, x, y, oppos) == O_CHASING_STONE)
	      {
		// chasing stones pass
		store_dir(cave, what_x, what_y, twice[grav], O_CHASING_STONE);
		store_dir(cave, x, y, oppos, O_SPACE);
		play_sound_of_element(cave, O_SLIME, x, y);
	      }
	    }
	    else
	    {
	      // or space over the slime? elements may pass from bottom to up then.
              if (is_like_space(cave, x, y, oppos) && get_dir(cave, x, y, grav) != O_SPACE)
	      {
		int what_x = getx(cave, x + gd_dx[grav], y + gd_dy[grav]);
		int what_y = gety(cave, x + gd_dx[grav], y + gd_dy[grav]);

		if (get_dir(cave, x, y, grav) == O_BLADDER)
		{
		  // bladders move UP the slime
		  store_dir(cave, x, y, grav, O_SPACE);
		  store_dir(cave, what_x, what_y, twice[oppos], O_BLADDER_1);
		  play_sound_of_element(cave, O_SLIME, x, y);
		}
		else if (get_dir(cave, x, y, grav) == O_FLYING_STONE)
		{
		  store_dir(cave, x, y, grav, O_SPACE);
		  store_dir(cave, what_x, what_y, twice[oppos], O_FLYING_STONE_F);
		  play_sound_of_element(cave, O_SLIME, x, y);
		}
		else if (get_dir(cave, x, y, grav) == O_FLYING_DIAMOND)
		{
		  store_dir(cave, x, y, grav, O_SPACE);
		  store_dir(cave, what_x, what_y, twice[oppos], O_FLYING_DIAMOND_F);
		  play_sound_of_element(cave, O_SLIME, x, y);
		}
	      }
	    }
	  }
	  break;

	case O_FALLING_WALL:
	  if (is_like_space(cave, x, y, grav_compat))
	  {
	    // try falling if space under.
	    int yy;

            // yy < y + cave->h is to check everything OVER the wall - since caves wrap around !!
	    for (yy = y + 1; yy < y + cave->h; yy++)
	      if (!is_like_space(cave, x, yy, GD_MV_STILL))   // stop cycle when other than space
		break;

	    // if scanning stopped by a player... start falling!
	    if (get(cave, x, yy) == O_PLAYER ||
		get(cave, x, yy) == O_PLAYER_START ||
		get(cave, x, yy) == O_PLAYER_GLUED ||
		get(cave, x, yy) == O_PLAYER_BOMB)
	    {
	      move(cave, x, y, grav_compat, O_FALLING_WALL_F);
	      // no sound when the falling wall starts falling!
	    }
	  }
	  break;

	case O_FALLING_WALL_F:
          if (is_player_dir(cave, x, y, grav_compat))
          {
            // if player under, it explodes - the falling wall, not the player!
            explode(cave, x, y);
          }
          else if (is_like_space(cave, x, y, grav_compat))
          {
            // continue falling
            move(cave, x, y, grav_compat, O_FALLING_WALL_F);
          }
          else
          {
            // stop falling
            play_sound_of_element(cave, O_FALLING_WALL_F, x, y);
            store(cave, x, y, O_FALLING_WALL);
          }
          break;


	  // ======================================================================================
	  //    C O N V E Y O R    B E L T S
	  // ======================================================================================

	case O_CONVEYOR_RIGHT:
	case O_CONVEYOR_LEFT:
	  // only works if gravity is up or down!!!
	  // first, check for gravity and running belts.
	  if (!cave->gravity_disabled && cave->conveyor_belts_active)
	  {
	    // decide direction
	    boolean left = (get(cave, x, y) != O_CONVEYOR_RIGHT);
	    if (cave->conveyor_belts_direction_changed)
	      left = !left;

	    const GdDirection *dir = (left ? ccw_eighth : cw_eighth);

	    // CHECK IF IT CONVEYS THE ELEMENT ABOVE IT
	    //
	    // if gravity is normal, and the conveyor belt has something ABOVE which can be moved
	    // OR
	    // the gravity is up, so anything that should float now goes DOWN and touches the
	    // conveyor
	    if ((cave->gravity == GD_MV_DOWN && moved_by_conveyor_top(cave, x, y, GD_MV_UP)) ||
		(cave->gravity == GD_MV_UP   && moved_by_conveyor_bottom(cave, x, y, GD_MV_UP)))
	    {
	      if (is_like_space(cave, x, y, dir[GD_MV_UP]))
	      {
                // to allow smooth movement of game elements on conveyor belts,
                // the moving direction set by "store_dir()" must be set to the
                // direction the game element on the conveyor belt is moving;
                // without smooth movement, the following lines would do it:
                //
		// store_dir(cave, x, y, dir[GD_MV_UP], get_dir(cave, x, y, GD_MV_UP));    // move
		// store_dir(cave, x, y, GD_MV_UP, O_SPACE);    // and place a space.

		int tile = get_dir(cave, x, y, GD_MV_UP);
                int move_dir = (left ? GD_MV_LEFT : GD_MV_RIGHT); // top side direction

                // raw values without range correction
                int raw_x = x + gd_dx[GD_MV_UP];
                int raw_y = y + gd_dy[GD_MV_UP];

                // final values with range correction
                int old_x = getx(cave, raw_x, raw_y);
                int old_y = gety(cave, raw_x, raw_y);

                // only move game element if not already moving (or if buggy)
                if (game_bd.game->dir_buffer_to[old_y][old_x] == GD_MV_STILL ||
                    cave->conveyor_belts_buggy)
                {
                  store(cave, old_x, old_y, O_SPACE);              // place a space ...
                  store_dir(cave, old_x, old_y, move_dir, tile);   // and move element.
                }
	      }
	    }

	    // CHECK IF IT CONVEYS THE ELEMENT BELOW IT
	    if ((cave->gravity == GD_MV_UP   && moved_by_conveyor_top(cave, x, y, GD_MV_DOWN)) ||
		(cave->gravity == GD_MV_DOWN && moved_by_conveyor_bottom(cave, x, y, GD_MV_DOWN)))
	    {
	      if (is_like_space(cave, x, y, dir[GD_MV_DOWN]))
	      {
                // to allow smooth movement of game elements on conveyor belts,
                // the moving direction set by "store_dir()" must be set to the
                // direction the game element on the conveyor belt is moving;
                // without smooth movement, the following lines would do it:
                //
		// store_dir(cave, x, y, dir[GD_MV_DOWN], get_dir(cave, x, y, GD_MV_DOWN)); // move
		// store_dir(cave, x, y, GD_MV_DOWN, O_SPACE);    // and clear.

		int tile = get_dir(cave, x, y, GD_MV_DOWN);
                int move_dir = (left ? GD_MV_RIGHT : GD_MV_LEFT); // bottom side direction

                // raw values without range correction
                int raw_x = x + gd_dx[GD_MV_DOWN];
                int raw_y = y + gd_dy[GD_MV_DOWN];

                // final values with range correction
                int old_x = getx(cave, raw_x, raw_y);
                int old_y = gety(cave, raw_x, raw_y);

                // only move game element if not already moving (or if buggy)
                if (game_bd.game->dir_buffer_to[old_y][old_x] == GD_MV_STILL ||
                    cave->conveyor_belts_buggy)
                {
                  store(cave, old_x, old_y, O_SPACE);              // place a space ...
                  store_dir(cave, old_x, old_y, move_dir, tile);   // and move element.
                }
	      }
	    }
	  }
	  break;


	  // ======================================================================================
	  //    R O C K E T S
	  // ======================================================================================

	case O_ROCKET_1:
	  if (is_like_space(cave, x, y, GD_MV_RIGHT))
	    move(cave, x, y, GD_MV_RIGHT, O_ROCKET_1);
	  else
	    explode(cave, x, y);
	  break;

	case O_ROCKET_2:
	  if (is_like_space(cave, x, y, GD_MV_UP))
	    move(cave, x, y, GD_MV_UP, O_ROCKET_2);
	  else
	    explode(cave, x, y);
	  break;

	case O_ROCKET_3:
	  if (is_like_space(cave, x, y, GD_MV_LEFT))
	    move(cave, x, y, GD_MV_LEFT, O_ROCKET_3);
	  else
	    explode(cave, x, y);
	  break;

	case O_ROCKET_4:
	  if (is_like_space(cave, x, y, GD_MV_DOWN))
	    move(cave, x, y, GD_MV_DOWN, O_ROCKET_4);
	  else
	    explode(cave, x, y);
	  break;


	  // ======================================================================================
	  //    S I M P L E   C H A N G I N G;   E X P L O S I O N S
	  // ======================================================================================

	case O_EXPLODE_3:
	  store(cave, x, y, cave->explosion_3_effect);
	  break;

	case O_EXPLODE_5:
	  store(cave, x, y, cave->explosion_effect);
	  break;

	case O_NUT_CRACK_4:
	  store(cave, x, y, O_DIAMOND);
	  break;

	case O_PRE_DIA_5:
	  store(cave, x, y, cave->diamond_birth_effect);
	  break;

	case O_PRE_STONE_4:
	  store(cave, x, y, O_STONE);
	  break;

	case O_NITRO_EXPL_4:
	  store(cave, x, y, cave->nitro_explosion_effect);
	  break;

	case O_BOMB_EXPL_4:
	  store(cave, x, y, cave->bomb_explosion_effect);
	  break;

	case O_AMOEBA_2_EXPL_4:
	  store(cave, x, y, cave->amoeba_2_explosion_effect);
	  break;

	case O_GHOST_EXPL_4:
	  {
	    static GdElement ghost_explode[] =
	    {
	      O_SPACE, O_SPACE, O_DIRT, O_DIRT, O_CLOCK, O_CLOCK, O_PRE_OUTBOX, O_BOMB,
	      O_BOMB, O_PLAYER, O_GHOST, O_BLADDER, O_DIAMOND, O_SWEET, O_WAITING_STONE, O_BITER_1
	    };

	    store(cave, x, y, ghost_explode[gd_rand_int_range(cave->random, 0,
                                                              ARRAY_SIZE(ghost_explode))]);
	  }
	  break;

	case O_PRE_STEEL_4:
	  store(cave, x, y, O_STEEL);
	  break;

	case O_PRE_CLOCK_4:
	  store(cave, x, y, O_CLOCK);
	  break;

	case O_BOMB_TICK_7:
	  explode(cave, x, y);
	  break;

	case O_TRAPPED_DIAMOND:
	  if (cave->diamond_key_collected)
	    store(cave, x, y, O_DIAMOND);
	  break;

	case O_PRE_OUTBOX:
	  if (cave->gate_open) // if no more diamonds needed
	    store(cave, x, y, O_OUTBOX);         // open outbox
	  break;

	case O_PRE_INVIS_OUTBOX:
	  if (cave->gate_open) // if no more diamonds needed
	    store(cave, x, y, O_INVIS_OUTBOX);   // open outbox. invisible one :P
	  break;

	case O_PRE_STEEL_OUTBOX:
	  if (cave->gate_open) // if no more diamonds needed
	    store(cave, x, y, O_STEEL_OUTBOX);   // open steel outbox
	  break;

	case O_PRE_INVIS_STEEL_OUTBOX:
	  if (cave->gate_open) // if no more diamonds needed
	    store(cave, x, y, O_INVIS_STEEL_OUTBOX);   // open steel outbox. invisible one :P
	  break;

	case O_INBOX:
	  cave->player_seen_ago = 0;
	  if (cave->hatched && !inbox_toggle)    // if it is time of birth
	    store(cave, x, y, O_PRE_PL_1);
	  inbox_toggle = !inbox_toggle;
	  break;

	case O_PRE_PL_1:
	  cave->player_seen_ago = 0;
	  if (cave->hatched)                     // no player birth before hatching
	    store(cave, x, y, O_PRE_PL_2);
	  break;

	case O_PRE_PL_2:
	  cave->player_seen_ago = 0;
	  if (cave->hatched)                     // no player birth before hatching
	    store(cave, x, y, O_PRE_PL_3);
	  break;

	case O_PRE_PL_3:
	  cave->player_seen_ago = 0;
	  if (cave->hatched)                     // no player birth before hatching
	    store(cave, x, y, O_PLAYER_START);   // newly born player invulnerable for one frame
	  break;

	case O_PRE_DIA_1:
	case O_PRE_DIA_2:
	case O_PRE_DIA_3:
	case O_PRE_DIA_4:
	case O_PRE_STONE_1:
	case O_PRE_STONE_2:
	case O_PRE_STONE_3:
	case O_BOMB_TICK_1:
	case O_BOMB_TICK_2:
	case O_BOMB_TICK_3:
	case O_BOMB_TICK_4:
	case O_BOMB_TICK_5:
	case O_BOMB_TICK_6:
	case O_PRE_STEEL_1:
	case O_PRE_STEEL_2:
	case O_PRE_STEEL_3:
	case O_BOMB_EXPL_1:
	case O_BOMB_EXPL_2:
	case O_BOMB_EXPL_3:
	case O_NUT_CRACK_1:
	case O_NUT_CRACK_2:
	case O_NUT_CRACK_3:
	case O_GHOST_EXPL_1:
	case O_GHOST_EXPL_2:
	case O_GHOST_EXPL_3:
	case O_EXPLODE_1:
	case O_EXPLODE_2:
	  // O_EXPLODE_3 is "effected"
	case O_EXPLODE_4:
	case O_PRE_CLOCK_1:
	case O_PRE_CLOCK_2:
	case O_PRE_CLOCK_3:
	case O_NITRO_EXPL_1:
	case O_NITRO_EXPL_2:
	case O_NITRO_EXPL_3:
	case O_AMOEBA_2_EXPL_1:
	case O_AMOEBA_2_EXPL_2:
	case O_AMOEBA_2_EXPL_3:
	  // simply the next identifier
	  next(cave, x, y);
	  break;

	case O_WATER_1:
	case O_WATER_2:
	case O_WATER_3:
	case O_WATER_4:
	case O_WATER_5:
	case O_WATER_6:
	case O_WATER_7:
	case O_WATER_8:
	case O_WATER_9:
	case O_WATER_10:
	case O_WATER_11:
	case O_WATER_12:
	case O_WATER_13:
	case O_WATER_14:
	case O_WATER_15:
	  found_water = TRUE;    // for sound
	  // simply the next identifier
	  next(cave, x, y);
	  break;

	case O_BLADDER_SPENDER:
	  if (is_like_space(cave, x, y, opposite[grav_compat]))
	  {
	    store_dir(cave, x, y, opposite[grav_compat], O_BLADDER);
	    store(cave, x, y, O_PRE_STEEL_1);
	    play_sound_of_element(cave, O_BLADDER_SPENDER, x, y);
	  }
	  break;

        case O_MAGIC_WALL:
          // the magic wall is handled by the elements which fall onto the wall.
          break;

        case O_LAVA:
          // lava is handled by the store() routine.
          break;

	default:
	  // other inanimate elements that do nothing
	  break;
      }

      // after processing, check the current coordinate, if it became scanned.
      // the scanned bit can be cleared, as it will not be processed again.
      // and, it must be cleared, as it should not be scanned; for example,
      // if it is, a replicator will not replicate it!
      unscan(cave, x, y);
    }
  }


  // ============================================================================
  // POSTPROCESSING
  // ============================================================================

  // forget "scanned" flags for objects.
  // also, check for time penalties.
  // these is something like an effect table, but we do not really use one.
  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      unscan(cave, x, y);

      if (get(cave, x, y) == O_TIME_PENALTY)
      {
	store(cave, x, y, O_GRAVESTONE);

	// there is time penalty for destroying the voodoo
	time_decrement_sec += cave->time_penalty;
      }
    }
  }

  // another scan-like routine:
  // short explosions (for example, in bd1) started with explode_2.
  // internally we use explode_1; and change it to explode_2 if needed.
  if (cave->short_explosions)
  {
    for (y = 0; y < cave->h; y++)
    {
      for (x = 0; x < cave->w; x++)
      {
	if (is_first_stage_of_explosion(cave, x, y))
	{
	  // select next frame of explosion
	  next(cave, x, y);

	  // forget scanned flag immediately
          unscan(cave, x, y);
	}
      }
    }
  }

  // this loop finds the coordinates of the player. needed for scrolling and chasing stone.
  // but we only do this if a living player was found. otherwise "stay" at current coordinates.
  if (cave->player_state == GD_PL_LIVING)
  {
    if (cave->active_is_first_found)
    {
      // to be 1stb compatible, we do everything backwards.
      for (y = cave->h - 1; y >= 0; y--)
      {
	for (x = cave->w - 1; x >= 0; x--)
	{
	  if (is_player(cave, x, y))
	  {
	    // here we remember the coordinates.
	    cave->player_x = x;
	    cave->player_y = y;
	  }
	}
      }
    }
    else
    {
      // as in the original: look for the last one
      for (y = 0; y < cave->h; y++)
      {
	for (x = 0; x < cave->w; x++)
	{
	  if (is_player(cave, x, y))
	  {
	    // here we remember the coordinates.
	    cave->player_x = x;
	    cave->player_y = y;
	  }
	}
      }
    }
  }

  // record coordinates of player for chasing stone
  for (i = 0; i < GD_PLAYER_MEM_SIZE - 1; i++)
  {
    cave->player_x_mem[i] = cave->player_x_mem[i + 1];
    cave->player_y_mem[i] = cave->player_y_mem[i + 1];
  }

  cave->player_x_mem[GD_PLAYER_MEM_SIZE - 1] = cave->player_x;
  cave->player_y_mem[GD_PLAYER_MEM_SIZE - 1] = cave->player_y;

  // SCHEDULING

  // updates based on the calculated explosions and per element ckdelays.
  gd_update_scheduling_cave_speed(cave);

  // SPECIAL SOUNDS

  // cave 3 sounds. precedence is controlled by the sound_play function.
  // but we have to check amoeba & magic together as they had a different gritty sound when mixed
  if (found_water)
    gd_sound_play(cave, GD_S_WATER, O_WATER, -1, -1);

  magic_sound = (cave->magic_wall_state == GD_MW_ACTIVE &&
		 cave->magic_wall_sound);

  amoeba_1_sound = (cave->hatched && cave->amoeba_sound &&
                    amoeba_count > 0 && cave->amoeba_state == GD_AM_AWAKE);

  amoeba_2_sound = (cave->hatched && cave->amoeba_sound &&
                    amoeba_2_count > 0 && cave->amoeba_2_state == GD_AM_AWAKE);

  if (amoeba_1_sound && magic_sound)
  {
    gd_sound_play(cave, GD_S_AMOEBA_MAGIC, O_AMOEBA, -1, -1);
  }
  else if (amoeba_2_sound && magic_sound)
  {
    gd_sound_play(cave, GD_S_AMOEBA_2_MAGIC, O_AMOEBA_2, -1, -1);
  }
  else
  {
    if (amoeba_1_sound)
      gd_sound_play(cave, GD_S_AMOEBA, O_AMOEBA, -1, -1);
    else if (amoeba_2_sound)
      gd_sound_play(cave, GD_S_AMOEBA_2, O_AMOEBA_2, -1, -1);
    else if (magic_sound)
      gd_sound_play(cave, GD_S_MAGIC_WALL, O_MAGIC_WALL, -1, -1);
  }

  if (cave->hatched)
  {
    if (amoeba_count > 0 && cave->amoeba_state == GD_AM_AWAKE)
      play_sound_of_element(cave, O_AMOEBA, -1, -1);
    else if (amoeba_2_count > 0 && cave->amoeba_2_state == GD_AM_AWAKE)
      play_sound_of_element(cave, O_AMOEBA_2, -1, -1);
  }


  // ============================================================================
  // CAVE VARIABLES
  // ============================================================================

  // PLAYER

  // check if player is alive. (delay reduced from 15 to 1 to faster detect game over,
  // but may be set back to 15 if a dead player can be re-created from effects element)
  if (cave->kill_player ||
      (cave->player_state == GD_PL_LIVING && cave->player_seen_ago > cave->player_seen_ago_limit))
  {
    if (cave->player_state != GD_PL_DIED)
      gd_sound_play(cave, GD_S_DYING, O_PLAYER, -1, -1);

    cave->player_state = GD_PL_DIED;
  }

  // check if any voodoo exploded, and kill players the next scan if that happended.
  if (cave->voodoo_touched)
    cave->kill_player = TRUE;

  // AMOEBA

  if (cave->amoeba_state == GD_AM_AWAKE)
  {
    if (game_bd.game->use_krissz_engine)
    {
      // special case: playing Krissz engine replay

      // count amoeba after every cave scan (instead of during next cave scan)
      // (this results in turning the amoeba to boulders one frame earlier)

      amoeba_count = 0;

      for (y = ymin; y <= ymax; y++)
        for (x = 0; x < cave->w; x++)
          if (get(cave, x, y) == O_AMOEBA)
            amoeba_count++;
    }

    // check flags after evaluating.
    if (amoeba_count >= cave->amoeba_max_count)
      cave->amoeba_state = GD_AM_TOO_BIG;

    if (amoeba_found_enclosed)
      cave->amoeba_state = GD_AM_ENCLOSED;
  }

  // amoeba can also be turned into diamond by magic wall
  if (cave->magic_wall_stops_amoeba && cave->magic_wall_state == GD_MW_ACTIVE)
    cave->amoeba_state = GD_AM_ENCLOSED;

  // AMOEBA 2

  if (cave->amoeba_2_state == GD_AM_AWAKE)
  {
    // check flags after evaluating.
    if (amoeba_2_count >= cave->amoeba_2_max_count)
      cave->amoeba_2_state = GD_AM_TOO_BIG;

    if (amoeba_2_found_enclosed)
      cave->amoeba_2_state = GD_AM_ENCLOSED;
  }

  // amoeba 2 can also be turned into diamond by magic wall
  if (cave->magic_wall_stops_amoeba && cave->magic_wall_state == GD_MW_ACTIVE)
    cave->amoeba_2_state = GD_AM_ENCLOSED;

  // now check times. ---------------------------
  // decrement time if a voodoo was killed.
  cave->time -= time_decrement_sec * cave->timing_factor;
  if (cave->time < 0)
    cave->time = 0;

  // only decrement time when player is already born.
  if (cave->hatched)
  {
    int secondsbefore = cave->time / cave->timing_factor;

    if (game.no_level_time_limit)
    {
      // reverse time
      cave->time += cave->speed;
      if (cave->time > cave->max_time * cave->timing_factor)
	cave->time -= cave->max_time * cave->timing_factor;
    }
    else
    {
      // normal time
      cave->time -= cave->speed;
      if (cave->time <= 0)
        cave->time = 0;
    }

    int secondsafter = cave->time / cave->timing_factor;

    if (cave->time / cave->timing_factor < 10)
      // if less than 10 seconds, no walking sound, but play explosion sound
      gd_sound_play(cave, GD_S_NONE, O_NONE, -1, -1);

    if (secondsbefore != secondsafter)
      gd_cave_set_seconds_sound(cave);
  }

  // a gravity switch was activated; seconds counting down
  if (cave->gravity_will_change > 0)
  {
    cave->gravity_will_change -= cave->speed;
    if (cave->gravity_will_change < 0)
      cave->gravity_will_change = 0;

    if (cave->gravity_will_change == 0)
    {
      cave->gravity = cave->gravity_next_direction;

      // takes precedence over amoeba and magic wall sound
      gd_sound_play(cave, GD_S_GRAVITY_CHANGING, O_GRAVITY_SWITCH, -1, -1);
    }
  }

  // creatures direction automatically change
  if (cave->creatures_direction_will_change > 0)
  {
    cave->creatures_direction_will_change -= cave->speed;
    if (cave->creatures_direction_will_change < 0)
      cave->creatures_direction_will_change = 0;

    if (cave->creatures_direction_will_change == 0)
    {
      gd_sound_play(cave, GD_S_SWITCH_CREATURES, O_CREATURE_SWITCH, -1, -1);

      cave->creatures_backwards = !cave->creatures_backwards;
      cave->creatures_direction_will_change =
	cave->creatures_direction_auto_change_time * cave->timing_factor;
    }
  }

  // magic wall; if active&wait or not wait for hatching
  if (cave->magic_wall_state == GD_MW_ACTIVE &&
      (cave->hatched || !cave->magic_timer_wait_for_hatching) &&
      !(cave->magic_wall_time == 0 && cave->magic_timer_zero_is_infinite))
  {
    cave->magic_wall_time -= cave->speed;
    if (cave->magic_wall_time < 0)
      cave->magic_wall_time = 0;
    if (cave->magic_wall_time == 0)
      cave->magic_wall_state = GD_MW_EXPIRED;
  }

  // we may wait for hatching, when starting amoeba
  if (cave->amoeba_timer_started_immediately ||
      (cave->amoeba_state == GD_AM_AWAKE &&
       (cave->hatched || !cave->amoeba_timer_wait_for_hatching)))
  {
    cave->amoeba_time -= cave->speed;
    if (cave->amoeba_time < 0)
      cave->amoeba_time = 0;
    if (cave->amoeba_time == 0)
      cave->amoeba_growth_prob = cave->amoeba_fast_growth_prob;
  }

  // we may wait for hatching, when starting amoeba
  if (cave->amoeba_timer_started_immediately ||
      (cave->amoeba_2_state == GD_AM_AWAKE &&
       (cave->hatched || !cave->amoeba_timer_wait_for_hatching)))
  {
    cave->amoeba_2_time -= cave->speed;
    if (cave->amoeba_2_time < 0)
      cave->amoeba_2_time = 0;
    if (cave->amoeba_2_time == 0)
      cave->amoeba_2_growth_prob = cave->amoeba_2_fast_growth_prob;
  }

  // check for player hatching.
  start_signal = FALSE;

  // if not the c64 scheduling, but the correct frametime is used,
  // hatching delay should always be decremented.
  // otherwise, the if (millisecs...) condition below will set this.
  if (cave->scheduling == GD_SCHEDULING_MILLISECONDS)
  {
    // NON-C64 scheduling
    if (cave->hatching_delay_frame > 0)
    {
      // for milliseconds-based, non-c64 schedulings, hatching delay means frames.
      cave->hatching_delay_frame--;
      if (cave->hatching_delay_frame == 0)
	start_signal = TRUE;
    }
  }
  else
  {
    // C64 scheduling
    if (cave->hatching_delay_time > 0)
    {
      // for c64 schedulings, hatching delay means milliseconds.
      cave->hatching_delay_time -= cave->speed;
      if (cave->hatching_delay_time <= 0)
      {
	cave->hatching_delay_time = 0;
	start_signal = TRUE;
      }
    }
  }

  // if decremented hatching, and it became zero:
  if (start_signal)
  {
    // THIS IS THE CAVE START SIGNAL

    // record that now the cave is in its normal state
    cave->hatched = TRUE;

    // if diamonds needed is below zero, we count the available diamonds now.
    gd_cave_count_diamonds(cave);

    // setup direction auto change
    if (cave->creatures_direction_auto_change_time)
    {
      cave->creatures_direction_will_change =
	cave->creatures_direction_auto_change_time * cave->timing_factor;

      if (cave->creatures_direction_auto_change_on_start)
	cave->creatures_backwards = !cave->creatures_backwards;
    }

    if (cave->player_state == GD_PL_NOT_YET)
      cave->player_state = GD_PL_LIVING;

    gd_sound_play(cave, GD_S_CRACKING, O_INBOX, -1, -1);
  }

  // for biters
  if (cave->biters_wait_frame == 0)
    cave->biters_wait_frame = cave->biter_delay_frame;
  else
    cave->biters_wait_frame--;

  // replicators delay
  if (cave->replicators_wait_frame == 0)
    cave->replicators_wait_frame = cave->replicator_delay_frame;
  else
    cave->replicators_wait_frame--;


  // ============================================================================
  // LAST THOUGTS
  // ============================================================================

#if 1
  // check if cave failed by timeout is done in main game engine
#else
  // check if cave failed by timeout
  if (cave->player_state == GD_PL_LIVING && cave->time == 0)
  {
    gd_cave_clear_sounds(cave);
    cave->player_state = GD_PL_TIMEOUT;
    gd_sound_play(cave, GD_S_TIMEOUT_0, O_NONE, -1, -1);
  }
#endif

  // set these for drawing.
  cave->last_direction_2nd = cave->last_direction;
  cave->last_direction = player_move;

  // here we remember last movements for animation. this is needed here, as animation
  // is in sync with the game, not the keyboard directly. (for example, after exiting
  // the cave, the player was "running" in the original, till bonus points were counted
  // for remaining time and so on.
  if (player_move == GD_MV_LEFT ||
      player_move == GD_MV_UP_LEFT ||
      player_move == GD_MV_DOWN_LEFT)
    cave->last_horizontal_direction = GD_MV_LEFT;

  if (player_move == GD_MV_RIGHT ||
      player_move == GD_MV_UP_RIGHT ||
      player_move == GD_MV_DOWN_RIGHT)
    cave->last_horizontal_direction = GD_MV_RIGHT;

  cave->frame++;  // XXX
}

void set_initial_cave_speed(GdCave *cave)
{
  int ymin, ymax;
  int x, y;

  // set cave get function; to implement perfect or lineshifting borders
  if (cave->lineshift)
    cave->getp = getp_shift;
  else
    cave->getp = getp_perfect;

  // check whether to scan the first and last line
  if (cave->border_scan_first_and_last)
  {
    ymin = 0;
    ymax = cave->h - 1;
  }
  else
  {
    ymin = 1;
    ymax = cave->h - 2;
  }

  for (y = ymin; y <= ymax; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      // add the ckdelay correction value for every element seen.
      cave->ckdelay_current += gd_element_properties[get(cave, x, y)].ckdelay;
    }
  }

  // update timing calculated by iterating and counting elements
  gd_update_scheduling_cave_speed(cave);
}
