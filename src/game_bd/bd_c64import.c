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


// make all caves selectable.
static boolean gd_import_as_all_caves_selectable = TRUE;

// conversion table for imported bd1 caves.
static const GdElement import_table_bd1[0x40] =
{
  /*  0 */ O_SPACE, O_DIRT, O_BRICK, O_MAGIC_WALL,
  /*  4 */ O_PRE_OUTBOX, O_OUTBOX, O_STEEL_EXPLODABLE, O_STEEL,
  /*  8 */ O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,
  /*  c */ O_FIREFLY_1_scanned, O_FIREFLY_2_scanned, O_FIREFLY_3_scanned, O_FIREFLY_4_scanned,
  /* 10 */ O_STONE, O_STONE_scanned, O_STONE_F, O_STONE_F_scanned,
  /* 14 */ O_DIAMOND, O_DIAMOND_scanned, O_DIAMOND_F, O_DIAMOND_F_scanned,
  // ----- ACID: marek roth extension in crazy dream 3
  /* 18 */ O_ACID, O_ACID_scanned, O_UNKNOWN, O_EXPLODE_0,
  /* 1c */ O_EXPLODE_2, O_EXPLODE_3, O_EXPLODE_4, O_EXPLODE_5,
  /* 20 */ O_PRE_DIA_0, O_PRE_DIA_2, O_PRE_DIA_3, O_PRE_DIA_4,
  /* 24 */ O_PRE_DIA_5, O_INBOX, O_PRE_PL_1, O_PRE_PL_2,
  /* 28 */ O_PRE_PL_3, O_UNKNOWN, O_H_EXPANDING_WALL, O_H_EXPANDING_WALL_scanned,
  /* 2c */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_DIRT_GLUED,
  /* 30 */ O_BUTTER_4, O_BUTTER_1, O_BUTTER_2, O_BUTTER_3,
  /* 34 */ O_BUTTER_4_scanned, O_BUTTER_1_scanned, O_BUTTER_2_scanned, O_BUTTER_3_scanned,
  /* 38 */ O_PLAYER, O_PLAYER_scanned, O_AMOEBA, O_AMOEBA_scanned,
  /* 3c */ O_VOODOO, O_INVIS_OUTBOX, O_SLIME, O_UNKNOWN
};

// conversion table for imported plck caves.
static const GdElement import_table_plck[0x10] =
{
  /*  0 */ O_STONE, O_DIAMOND, O_MAGIC_WALL, O_BRICK,
  /*  4 */ O_STEEL, O_H_EXPANDING_WALL, O_VOODOO, O_DIRT,
  /*  8 */ O_FIREFLY_1, O_BUTTER_4, O_AMOEBA, O_SLIME,
  /* 12 */ O_PRE_INVIS_OUTBOX, O_PRE_OUTBOX, O_INBOX, O_SPACE
};

// conversion table for imported 1stb caves.
static const GdElement import_table_1stb[0x80] =
{
  /*  0 */ O_SPACE, O_DIRT, O_BRICK, O_MAGIC_WALL,
  /*  4 */ O_PRE_OUTBOX, O_OUTBOX, O_PRE_INVIS_OUTBOX, O_INVIS_OUTBOX,
  /*  8 */ O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,
  /*  c */ O_FIREFLY_1_scanned, O_FIREFLY_2_scanned, O_FIREFLY_3_scanned, O_FIREFLY_4_scanned,
  /* 10 */ O_STONE, O_STONE_scanned, O_STONE_F, O_STONE_F_scanned,
  /* 14 */ O_DIAMOND, O_DIAMOND_scanned, O_DIAMOND_F, O_DIAMOND_F_scanned,
  /* 18 */ O_PRE_CLOCK_1, O_PRE_CLOCK_2, O_PRE_CLOCK_3, O_PRE_CLOCK_4,
  /* 1c */ O_BITER_SWITCH, O_BITER_SWITCH, O_BLADDER_SPENDER, O_PRE_DIA_0,
  /* 20 */ O_PRE_DIA_1, O_PRE_DIA_2, O_PRE_DIA_3, O_PRE_DIA_4,
  /* 24 */ O_PRE_DIA_5, O_INBOX, O_PRE_PL_1, O_PRE_PL_2,
  // ----- CLOCK: not mentioned in marek's bd inside faq
  /* 28 */ O_PRE_PL_3, O_CLOCK, O_H_EXPANDING_WALL, O_H_EXPANDING_WALL_scanned,
  /* 2c */ O_CREATURE_SWITCH, O_CREATURE_SWITCH, O_EXPANDING_WALL_SWITCH, O_EXPANDING_WALL_SWITCH,
  /* 30 */ O_BUTTER_3, O_BUTTER_4, O_BUTTER_1, O_BUTTER_2,
  /* 34 */ O_BUTTER_3_scanned, O_BUTTER_4_scanned, O_BUTTER_1_scanned, O_BUTTER_2_scanned,
  /* 38 */ O_STEEL, O_SLIME, O_BOMB, O_SWEET,
  /* 3c */ O_PRE_STONE_1, O_PRE_STONE_2, O_PRE_STONE_3, O_PRE_STONE_4,
  /* 40 */ O_BLADDER, O_BLADDER_1, O_BLADDER_2, O_BLADDER_3,
  /* 44 */ O_BLADDER_4, O_BLADDER_5, O_BLADDER_6, O_BLADDER_7,
  /* 48 */ O_BLADDER_8, O_BLADDER_8, O_EXPLODE_0, O_EXPLODE_1,
  /* 4c */ O_EXPLODE_2, O_EXPLODE_3, O_EXPLODE_4, O_EXPLODE_5,
  /* 50 */ O_PLAYER, O_PLAYER_scanned, O_PLAYER_BOMB, O_PLAYER_BOMB_scanned,
  /* 54 */ O_PLAYER_GLUED, O_PLAYER_GLUED, O_VOODOO, O_AMOEBA,
  /* 58 */ O_AMOEBA_scanned, O_BOMB_TICK_1, O_BOMB_TICK_2, O_BOMB_TICK_3,
  /* 5c */ O_BOMB_TICK_4, O_BOMB_TICK_5, O_BOMB_TICK_6, O_BOMB_TICK_7,
  /* 60 */ O_BOMB_EXPL_1, O_BOMB_EXPL_2, O_BOMB_EXPL_3, O_BOMB_EXPL_4,
  /* 64 */ O_GHOST, O_GHOST_scanned, O_GHOST_EXPL_1, O_GHOST_EXPL_2,
  /* 68 */ O_GHOST_EXPL_3, O_GHOST_EXPL_4, O_GRAVESTONE, O_STONE_GLUED,
  /* 6c */ O_DIAMOND_GLUED, O_DIAMOND_KEY, O_TRAPPED_DIAMOND, O_TIME_PENALTY,
  /* 70 */ O_WAITING_STONE, O_WAITING_STONE_scanned, O_CHASING_STONE, O_CHASING_STONE_scanned,
  /* 74 */ O_PRE_STEEL_1, O_PRE_STEEL_2, O_PRE_STEEL_3, O_PRE_STEEL_4,
  /* 78 */ O_BITER_1, O_BITER_2, O_BITER_3, O_BITER_4,
  /* 7c */ O_BITER_1_scanned, O_BITER_2_scanned, O_BITER_3_scanned, O_BITER_4_scanned,
};

// conversion table for imported crazy dream caves.
static const GdElement import_table_crdr[0x100] =
{
  /*  0 */ O_SPACE, O_DIRT, O_BRICK, O_MAGIC_WALL,
  /*  4 */ O_PRE_OUTBOX, O_OUTBOX, O_PRE_INVIS_OUTBOX, O_INVIS_OUTBOX,
  /*  8 */ O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,
  /*  c */ O_FIREFLY_1_scanned, O_FIREFLY_2_scanned, O_FIREFLY_3_scanned, O_FIREFLY_4_scanned,
  /* 10 */ O_STONE, O_STONE_scanned, O_STONE_F, O_STONE_F_scanned,
  /* 14 */ O_DIAMOND, O_DIAMOND_scanned, O_DIAMOND_F, O_DIAMOND_F_scanned,
  /* 18 */ O_PRE_CLOCK_1, O_PRE_CLOCK_2, O_PRE_CLOCK_3, O_PRE_CLOCK_4,
  // ----- 6 different stages, the first is the pre_dia_0
  /* 1c */ O_BITER_SWITCH, O_BITER_SWITCH, O_BLADDER_SPENDER, O_PRE_DIA_0,
  /* 20 */ O_PRE_DIA_1, O_PRE_DIA_2, O_PRE_DIA_3, O_PRE_DIA_4,
  /* 24 */ O_PRE_DIA_5, O_INBOX, O_PRE_PL_1, O_PRE_PL_2,
  // ----- CLOCK: not mentioned in marek's bd inside faq
  /* 28 */ O_PRE_PL_3, O_CLOCK, O_H_EXPANDING_WALL, O_H_EXPANDING_WALL_scanned,
  /* 2c */ O_CREATURE_SWITCH, O_CREATURE_SWITCH, O_EXPANDING_WALL_SWITCH, O_EXPANDING_WALL_SWITCH,
  /* 30 */ O_BUTTER_3, O_BUTTER_4, O_BUTTER_1, O_BUTTER_2,
  /* 34 */ O_BUTTER_3_scanned, O_BUTTER_4_scanned, O_BUTTER_1_scanned, O_BUTTER_2_scanned,
  /* 38 */ O_STEEL, O_SLIME, O_BOMB, O_SWEET,
  /* 3c */ O_PRE_STONE_1, O_PRE_STONE_2, O_PRE_STONE_3, O_PRE_STONE_4,
  /* 40 */ O_BLADDER, O_BLADDER_1, O_BLADDER_2, O_BLADDER_3,
  /* 44 */ O_BLADDER_4, O_BLADDER_5, O_BLADDER_6, O_BLADDER_7,
  /* 48 */ O_BLADDER_8, O_BLADDER_8, O_EXPLODE_0, O_EXPLODE_1,
  /* 4c */ O_EXPLODE_2, O_EXPLODE_3, O_EXPLODE_4, O_EXPLODE_5,
  /* 50 */ O_PLAYER, O_PLAYER_scanned, O_PLAYER_BOMB, O_PLAYER_BOMB_scanned,
  /* 54 */ O_PLAYER_GLUED, O_PLAYER_GLUED, O_VOODOO, O_AMOEBA,
  /* 58 */ O_AMOEBA_scanned, O_BOMB_TICK_1, O_BOMB_TICK_2, O_BOMB_TICK_3,
  /* 5c */ O_BOMB_TICK_4, O_BOMB_TICK_5, O_BOMB_TICK_6, O_BOMB_TICK_7,
  /* 60 */ O_BOMB_EXPL_1, O_BOMB_EXPL_2, O_BOMB_EXPL_3, O_BOMB_EXPL_4,
  /* 64 */ O_GHOST, O_GHOST_scanned, O_GHOST_EXPL_1, O_GHOST_EXPL_2,
  /* 68 */ O_GHOST_EXPL_3, O_GHOST_EXPL_4, O_GRAVESTONE, O_STONE_GLUED,
  /* 6c */ O_DIAMOND_GLUED, O_DIAMOND_KEY, O_TRAPPED_DIAMOND, O_TIME_PENALTY,
  /* 70 */ O_WAITING_STONE, O_WAITING_STONE_scanned, O_CHASING_STONE, O_CHASING_STONE_scanned,
  /* 74 */ O_PRE_STEEL_1, O_PRE_STEEL_2, O_PRE_STEEL_3, O_PRE_STEEL_4,
  /* 78 */ O_BITER_1, O_BITER_2, O_BITER_3, O_BITER_4,
  /* 7c */ O_BITER_1_scanned, O_BITER_2_scanned, O_BITER_3_scanned, O_BITER_4_scanned,

  /* 80 */ O_POT, O_PLAYER_STIRRING, O_GRAVITY_SWITCH, O_GRAVITY_SWITCH,
  /* 84 */ O_PNEUMATIC_HAMMER, O_PNEUMATIC_HAMMER, O_BOX, O_BOX,
  /* 88 */ O_UNKNOWN, O_UNKNOWN, O_ACID, O_ACID_scanned,
  /* 8c */ O_KEY_1, O_KEY_2, O_KEY_3, O_UNKNOWN,
  /* 90 */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* 94 */ O_UNKNOWN, O_TELEPORTER, O_UNKNOWN, O_SKELETON,
  /* 98 */ O_WATER, O_WATER_16, O_WATER_15, O_WATER_14,
  /* 9c */ O_WATER_13, O_WATER_12, O_WATER_11, O_WATER_10,
  /* a0 */ O_WATER_9, O_WATER_8, O_WATER_7, O_WATER_6,
  /* a4 */ O_WATER_5, O_WATER_4, O_WATER_3, O_WATER_2,
  /* a8 */ O_WATER_1, O_COW_ENCLOSED_1, O_COW_ENCLOSED_2, O_COW_ENCLOSED_3,
  /* ac */ O_COW_ENCLOSED_4, O_COW_ENCLOSED_5, O_COW_ENCLOSED_6, O_COW_ENCLOSED_7,
  /* b0 */ O_COW_1, O_COW_2, O_COW_3, O_COW_4,
  /* b4 */ O_COW_1_scanned, O_COW_2_scanned, O_COW_3_scanned, O_COW_4_scanned,
  /* b8 */ O_DIRT_GLUED, O_STEEL_EXPLODABLE, O_DOOR_1, O_DOOR_2,
  /* bc */ O_DOOR_3, O_FALLING_WALL, O_FALLING_WALL_F, O_FALLING_WALL_F_scanned,
  /* c0 */ O_WALLED_DIAMOND, O_UNKNOWN, O_WALLED_KEY_1, O_WALLED_KEY_2,
  // ----- c5 = brick?! (vital key),
  // ----- c7 = dirt?! (think twice - it has a code which will change it to dirt.)
  /* c4 */ O_WALLED_KEY_3, O_BRICK, O_UNKNOWN, O_DIRT,
  /* c8 */ O_DIRT2, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* cc */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* d0 */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* d4 */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* d8 */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* dc */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  /* e0 */ O_ALT_FIREFLY_1, O_ALT_FIREFLY_2, O_ALT_FIREFLY_3, O_ALT_FIREFLY_4,
  /* e4 */ O_ALT_FIREFLY_1_scanned, O_ALT_FIREFLY_2_scanned, O_ALT_FIREFLY_3_scanned, O_ALT_FIREFLY_4_scanned,
  /* e8 */ O_ALT_BUTTER_3, O_ALT_BUTTER_4, O_ALT_BUTTER_1, O_ALT_BUTTER_2,
  /* ec */ O_ALT_BUTTER_3_scanned, O_ALT_BUTTER_4_scanned, O_ALT_BUTTER_1_scanned, O_ALT_BUTTER_2_scanned,
  /* f0 */ O_WATER, O_WATER, O_WATER, O_WATER,
  /* f4 */ O_WATER, O_WATER, O_WATER, O_WATER,
  /* f8 */ O_WATER, O_WATER, O_WATER, O_WATER,
  /* fc */ O_WATER, O_WATER, O_WATER, O_WATER,
};

// conversion table for imported 1stb caves.
// @todo check O_PRE_DIA_0 and O_EXPLODE_0
static const GdElement import_table_crli[0x80] =
{
  /*  0 */ O_SPACE, O_DIRT, O_BRICK, O_MAGIC_WALL,
  /*  4 */ O_PRE_OUTBOX, O_OUTBOX, O_PRE_INVIS_OUTBOX, O_INVIS_OUTBOX,
  /*  8 */ O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,
  /*  c */ O_FIREFLY_1_scanned, O_FIREFLY_2_scanned, O_FIREFLY_3_scanned, O_FIREFLY_4_scanned,
  /* 10 */ O_STONE, O_STONE_scanned, O_STONE_F, O_STONE_F_scanned,
  /* 14 */ O_DIAMOND, O_DIAMOND_scanned, O_DIAMOND_F, O_DIAMOND_F_scanned,
  /* 18 */ O_PRE_CLOCK_1, O_PRE_CLOCK_2, O_PRE_CLOCK_3, O_PRE_CLOCK_4,
  // ----- 6 different stages, the first is the pre_dia_0
  /* 1c */ O_BITER_SWITCH, O_BITER_SWITCH, O_BLADDER_SPENDER, O_PRE_DIA_0,
  /* 20 */ O_PRE_DIA_1, O_PRE_DIA_2, O_PRE_DIA_3, O_PRE_DIA_4,
  /* 24 */ O_PRE_DIA_5, O_INBOX, O_PRE_PL_1, O_PRE_PL_2,
  // ----- CLOCK: not mentioned in marek's bd inside faq
  /* 28 */ O_PRE_PL_3, O_CLOCK, O_H_EXPANDING_WALL, O_H_EXPANDING_WALL_scanned,
  /* 2c */ O_CREATURE_SWITCH, O_CREATURE_SWITCH, O_EXPANDING_WALL_SWITCH, O_EXPANDING_WALL_SWITCH,
  /* 30    O_BUTTER_3, O_BUTTER_4, O_BUTTER_1, O_BUTTER_2, */
  /* 34    O_BUTTER_3_scanned, O_BUTTER_4_scanned, O_BUTTER_1_scanned, O_BUTTER_2_scanned, */
  /* 30 */ O_BUTTER_4, O_BUTTER_1, O_BUTTER_2, O_BUTTER_3, // fix order
  /* 34 */ O_BUTTER_4_scanned, O_BUTTER_1_scanned, O_BUTTER_2_scanned, O_BUTTER_3_scanned, // fix order
  /* 38 */ O_STEEL, O_SLIME, O_BOMB, O_SWEET,
  /* 3c */ O_PRE_STONE_1, O_PRE_STONE_2, O_PRE_STONE_3, O_PRE_STONE_4,
  /* 40 */ O_BLADDER, O_BLADDER_1, O_BLADDER_2, O_BLADDER_3,
  /* 44 */ O_BLADDER_4, O_BLADDER_5, O_BLADDER_6, O_BLADDER_7,
  /* 48 */ O_BLADDER_8, O_BLADDER_8, O_EXPLODE_0, O_EXPLODE_1,
  /* 4c */ O_EXPLODE_2, O_EXPLODE_3, O_EXPLODE_4, O_EXPLODE_5,
  /* 50 */ O_PLAYER, O_PLAYER_scanned, O_PLAYER_BOMB, O_PLAYER_BOMB_scanned,
  /* 54 */ O_PLAYER_GLUED, O_PLAYER_GLUED, O_VOODOO, O_AMOEBA,
  /* 58 */ O_AMOEBA_scanned, O_BOMB_TICK_1, O_BOMB_TICK_2, O_BOMB_TICK_3,
  /* 5c */ O_BOMB_TICK_4, O_BOMB_TICK_5, O_BOMB_TICK_6, O_BOMB_TICK_7,
  /* 60 */ O_BOMB_EXPL_1, O_BOMB_EXPL_2, O_BOMB_EXPL_3, O_BOMB_EXPL_4,
  /* 64 */ O_ACID, O_ACID, O_FALLING_WALL, O_FALLING_WALL_F,
  /* 68 */ O_FALLING_WALL_F_scanned, O_BOX, O_GRAVESTONE, O_STONE_GLUED,
  /* 6c */ O_DIAMOND_GLUED, O_DIAMOND_KEY, O_TRAPPED_DIAMOND, O_GRAVESTONE,
  /* 70 */ O_WAITING_STONE, O_WAITING_STONE_scanned, O_CHASING_STONE, O_CHASING_STONE_scanned,
  /* 74 */ O_PRE_STEEL_1, O_PRE_STEEL_2, O_PRE_STEEL_3, O_PRE_STEEL_4,
  /* 78 */ O_BITER_1, O_BITER_2, O_BITER_3, O_BITER_4,
  /* 7c */ O_BITER_1_scanned, O_BITER_2_scanned, O_BITER_3_scanned, O_BITER_4_scanned,
};

// Internal character (letter) codes in c64 games.
// Used for converting names of caves imported from crli and other types of binary data.
// Missing: "triple line" after >, diamond between ()s, player's head after ).
static const char bd_internal_character_encoding[] =
  "            ,!./0123456789:*<=>  ABCDEFGHIJKLMNOPQRSTUVWXYZ( ) _";

// Set engine parameters to BD1 defaults in cave.
GdPropertyDefault gd_defaults_bd1[] =
{
  { CAVE_OFFSET(level_amoeba_threshold),			200			},
  { CAVE_OFFSET(amoeba_growth_prob),				31250			},
  { CAVE_OFFSET(amoeba_fast_growth_prob),			250000			},
  { CAVE_OFFSET(amoeba_timer_started_immediately),		TRUE			},
  { CAVE_OFFSET(amoeba_timer_wait_for_hatching),		FALSE			},
  { CAVE_OFFSET(lineshift),					TRUE			},
  { CAVE_OFFSET(wraparound_objects),				TRUE			},
  { CAVE_OFFSET(diagonal_movements),				FALSE			},
  { CAVE_OFFSET(voodoo_collects_diamonds),			FALSE			},
  { CAVE_OFFSET(voodoo_dies_by_stone),				FALSE			},
  { CAVE_OFFSET(voodoo_disappear_in_explosion),			TRUE			},
  { CAVE_OFFSET(voodoo_any_hurt_kills_player),			FALSE			},
  { CAVE_OFFSET(creatures_backwards),				FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_on_start),	FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_time),		0			},
  { CAVE_OFFSET(level_hatching_delay_time),			2			},
  { CAVE_OFFSET(intermission_instantlife),			TRUE			},
  { CAVE_OFFSET(intermission_rewardlife),			FALSE			},
  { CAVE_OFFSET(magic_wall_stops_amoeba),			FALSE			},
  { CAVE_OFFSET(magic_wall_breakscan),				TRUE			},
  { CAVE_OFFSET(magic_timer_zero_is_infinite),			TRUE			},
  { CAVE_OFFSET(magic_timer_wait_for_hatching),			FALSE			},
  { CAVE_OFFSET(pushing_stone_prob),				250000			},
  { CAVE_OFFSET(pushing_stone_prob_sweet),			1000000			},
  { CAVE_OFFSET(active_is_first_found),				FALSE			},
  { CAVE_OFFSET(short_explosions),				TRUE			},
  { CAVE_OFFSET(slime_predictable),				TRUE			},
  { CAVE_OFFSET(snap_element),					O_SPACE			},
  { CAVE_OFFSET(max_time),					999			},

  { CAVE_OFFSET(pal_timing),					TRUE			},
  { CAVE_OFFSET(scheduling),					GD_SCHEDULING_BD1	},

  { -1 },
};

// Set engine parameters to BD2 defaults in cave.
GdPropertyDefault gd_defaults_bd2[] =
{
  { CAVE_OFFSET(level_amoeba_threshold),			200			},
  { CAVE_OFFSET(amoeba_growth_prob),				31250			},
  { CAVE_OFFSET(amoeba_fast_growth_prob),			250000			},
  { CAVE_OFFSET(amoeba_timer_started_immediately),		FALSE			},
  { CAVE_OFFSET(amoeba_timer_wait_for_hatching),		FALSE			},
  { CAVE_OFFSET(lineshift),					TRUE			},
  { CAVE_OFFSET(wraparound_objects),				TRUE			},
  { CAVE_OFFSET(diagonal_movements),				FALSE			},
  { CAVE_OFFSET(voodoo_collects_diamonds),			FALSE			},
  { CAVE_OFFSET(voodoo_dies_by_stone),				FALSE			},
  { CAVE_OFFSET(voodoo_disappear_in_explosion),			TRUE			},
  { CAVE_OFFSET(voodoo_any_hurt_kills_player),			FALSE			},
  { CAVE_OFFSET(creatures_backwards),				FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_on_start),	FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_time),		0			},
  { CAVE_OFFSET(level_hatching_delay_time),			2			},
  { CAVE_OFFSET(intermission_instantlife),			TRUE			},
  { CAVE_OFFSET(intermission_rewardlife),			FALSE			},
  // marek roth bd inside faq 3.0
  { CAVE_OFFSET(magic_wall_stops_amoeba),			FALSE			},
  { CAVE_OFFSET(magic_timer_zero_is_infinite),			TRUE			},
  { CAVE_OFFSET(magic_timer_wait_for_hatching),			FALSE			},
  { CAVE_OFFSET(pushing_stone_prob),				250000			},
  { CAVE_OFFSET(pushing_stone_prob_sweet),			1000000			},
  { CAVE_OFFSET(active_is_first_found),				FALSE			},
  { CAVE_OFFSET(short_explosions),				TRUE			},
  { CAVE_OFFSET(slime_predictable),				TRUE			},
  { CAVE_OFFSET(snap_element),					O_SPACE			},
  { CAVE_OFFSET(max_time),					999			},

  { CAVE_OFFSET(pal_timing),					TRUE			},
  { CAVE_OFFSET(scheduling),					GD_SCHEDULING_BD2	},

  { -1 },
};

// Set engine parameters to Construction Kit defaults in cave.
GdPropertyDefault gd_defaults_plck[] =
{
  { CAVE_OFFSET(amoeba_growth_prob),				31250			},
  { CAVE_OFFSET(amoeba_fast_growth_prob),			250000			},
  { CAVE_OFFSET(amoeba_timer_started_immediately),		FALSE			},
  { CAVE_OFFSET(amoeba_timer_wait_for_hatching),		FALSE			},
  { CAVE_OFFSET(lineshift),					TRUE			},
  { CAVE_OFFSET(wraparound_objects),				TRUE			},
  { CAVE_OFFSET(border_scan_first_and_last),			FALSE			},
  { CAVE_OFFSET(diagonal_movements),				FALSE			},
  { CAVE_OFFSET(voodoo_collects_diamonds),			FALSE			},
  { CAVE_OFFSET(voodoo_dies_by_stone),				FALSE			},
  { CAVE_OFFSET(voodoo_disappear_in_explosion),			TRUE			},
  { CAVE_OFFSET(voodoo_any_hurt_kills_player),			FALSE			},
  { CAVE_OFFSET(creatures_backwards),				FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_on_start),	FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_time),		0			},
  { CAVE_OFFSET(level_hatching_delay_time),			2			},
  { CAVE_OFFSET(intermission_instantlife),			TRUE			},
  { CAVE_OFFSET(intermission_rewardlife),			FALSE			},
  { CAVE_OFFSET(magic_wall_stops_amoeba),			FALSE			},
  { CAVE_OFFSET(magic_timer_zero_is_infinite),			TRUE			},
  { CAVE_OFFSET(magic_timer_wait_for_hatching),			FALSE			},
  { CAVE_OFFSET(pushing_stone_prob),				250000			},
  { CAVE_OFFSET(pushing_stone_prob_sweet),			1000000			},
  { CAVE_OFFSET(active_is_first_found),				FALSE			},
  { CAVE_OFFSET(short_explosions),				TRUE			},
  { CAVE_OFFSET(snap_element),					O_SPACE			},
  { CAVE_OFFSET(max_time),					999			},

  { CAVE_OFFSET(pal_timing),					TRUE			},
  { CAVE_OFFSET(scheduling),					GD_SCHEDULING_PLCK	},

  { -1 },
};

// Set engine parameters to 1stB defaults in cave.
GdPropertyDefault gd_defaults_1stb[] =
{
  { CAVE_OFFSET(amoeba_growth_prob),				31250			},
  { CAVE_OFFSET(amoeba_fast_growth_prob),			250000			},
  { CAVE_OFFSET(amoeba_timer_started_immediately),		FALSE			},
  { CAVE_OFFSET(amoeba_timer_wait_for_hatching),		TRUE			},
  { CAVE_OFFSET(lineshift),					TRUE			},
  { CAVE_OFFSET(wraparound_objects),				TRUE			},
  { CAVE_OFFSET(voodoo_collects_diamonds),			TRUE			},
  { CAVE_OFFSET(voodoo_dies_by_stone),				TRUE			},
  { CAVE_OFFSET(voodoo_disappear_in_explosion),			TRUE			},
  { CAVE_OFFSET(voodoo_any_hurt_kills_player),			FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_on_start),	TRUE			},
  { CAVE_OFFSET(level_hatching_delay_time),			2			},
  { CAVE_OFFSET(intermission_instantlife),			FALSE			},
  { CAVE_OFFSET(intermission_rewardlife),			TRUE			},
  { CAVE_OFFSET(magic_timer_zero_is_infinite),			TRUE			},
  { CAVE_OFFSET(magic_timer_wait_for_hatching),			TRUE			},
  { CAVE_OFFSET(pushing_stone_prob),				250000			},
  { CAVE_OFFSET(pushing_stone_prob_sweet),			1000000			},
  { CAVE_OFFSET(active_is_first_found),				TRUE			},
  { CAVE_OFFSET(short_explosions),				FALSE			},
  { CAVE_OFFSET(slime_predictable),				TRUE			},
  { CAVE_OFFSET(snap_element),					O_SPACE			},
  { CAVE_OFFSET(max_time),					999			},

  { CAVE_OFFSET(pal_timing),					TRUE			},
  { CAVE_OFFSET(scheduling),					GD_SCHEDULING_PLCK	},

  // not immediately to diamond, but with animation
  { CAVE_OFFSET(amoeba_enclosed_effect),			O_PRE_DIA_1		},
  { CAVE_OFFSET(dirt_looks_like),				O_DIRT2			},

  { -1 },
};

// Set engine parameters to Crazy Dream 7 defaults in cave.
GdPropertyDefault gd_defaults_crdr_7[] =
{
  { CAVE_OFFSET(amoeba_growth_prob),				31250			},
  { CAVE_OFFSET(amoeba_fast_growth_prob),			250000			},
  { CAVE_OFFSET(amoeba_timer_started_immediately),		FALSE			},
  { CAVE_OFFSET(amoeba_timer_wait_for_hatching),		TRUE			},
  { CAVE_OFFSET(lineshift),					TRUE			},
  { CAVE_OFFSET(wraparound_objects),				TRUE			},
  { CAVE_OFFSET(voodoo_collects_diamonds),			TRUE			},
  { CAVE_OFFSET(voodoo_dies_by_stone),				TRUE			},
  { CAVE_OFFSET(voodoo_disappear_in_explosion),			TRUE			},
  { CAVE_OFFSET(voodoo_any_hurt_kills_player),			FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_on_start),	FALSE			},
  { CAVE_OFFSET(level_hatching_delay_time),			2			},
  { CAVE_OFFSET(intermission_instantlife),			FALSE			},
  { CAVE_OFFSET(intermission_rewardlife),			TRUE			},
  { CAVE_OFFSET(magic_timer_zero_is_infinite),			FALSE			},
  { CAVE_OFFSET(magic_timer_wait_for_hatching),			TRUE			},
  { CAVE_OFFSET(pushing_stone_prob),				250000			},
  { CAVE_OFFSET(pushing_stone_prob_sweet),			1000000			},
  { CAVE_OFFSET(active_is_first_found),				TRUE			},
  { CAVE_OFFSET(short_explosions),				FALSE			},
  { CAVE_OFFSET(slime_predictable),				TRUE			},
  { CAVE_OFFSET(snap_element),					O_SPACE			},
  { CAVE_OFFSET(max_time),					999			},

  { CAVE_OFFSET(pal_timing),					TRUE			},
  { CAVE_OFFSET(scheduling),					GD_SCHEDULING_CRDR	},

  // not immediately to diamond, but with animation
  { CAVE_OFFSET(amoeba_enclosed_effect),			O_PRE_DIA_1		},
  { CAVE_OFFSET(water_does_not_flow_down),			TRUE			},

  // in crdr, skeletons can also be used to open the gate
  { CAVE_OFFSET(skeletons_worth_diamonds),			1			},

  // the intermission "survive" needs this flag
  { CAVE_OFFSET(gravity_affects_all),				FALSE			},

  { -1 },
};

// Set engine parameters to Crazy Light defaults in cave.
GdPropertyDefault gd_defaults_crli[] =
{
  { CAVE_OFFSET(amoeba_growth_prob),				31250			},
  { CAVE_OFFSET(amoeba_fast_growth_prob),			250000			},
  { CAVE_OFFSET(amoeba_timer_started_immediately),		FALSE			},
  { CAVE_OFFSET(amoeba_timer_wait_for_hatching),		TRUE			},
  { CAVE_OFFSET(lineshift),					TRUE			},
  { CAVE_OFFSET(wraparound_objects),				TRUE			},
  { CAVE_OFFSET(voodoo_collects_diamonds),			TRUE			},
  { CAVE_OFFSET(voodoo_dies_by_stone),				TRUE			},
  { CAVE_OFFSET(voodoo_disappear_in_explosion),			TRUE			},
  { CAVE_OFFSET(voodoo_any_hurt_kills_player),			FALSE			},
  { CAVE_OFFSET(creatures_direction_auto_change_on_start),	FALSE			},
  { CAVE_OFFSET(level_hatching_delay_time),			2			},
  { CAVE_OFFSET(intermission_instantlife),			FALSE			},
  { CAVE_OFFSET(intermission_rewardlife),			TRUE			},
  { CAVE_OFFSET(magic_timer_zero_is_infinite),			FALSE			},
  { CAVE_OFFSET(magic_timer_wait_for_hatching),			TRUE			},
  { CAVE_OFFSET(pushing_stone_prob),				250000			},
  { CAVE_OFFSET(pushing_stone_prob_sweet),			1000000			},
  { CAVE_OFFSET(active_is_first_found),				TRUE			},
  { CAVE_OFFSET(short_explosions),				FALSE			},
  { CAVE_OFFSET(slime_predictable),				TRUE			},
  { CAVE_OFFSET(max_time),					999			},

  { CAVE_OFFSET(pal_timing),					TRUE			},
  { CAVE_OFFSET(scheduling),					GD_SCHEDULING_PLCK	},

  // not immediately to diamond, but with animation
  { CAVE_OFFSET(amoeba_enclosed_effect),			O_PRE_DIA_1		},

  { -1 },
};

// Get array with default engine parameters according to cave engine type.
GdPropertyDefault *gd_get_engine_default_array(GdEngine engine)
{
  switch (engine)
  {
    case GD_ENGINE_BD1:
      return gd_defaults_bd1;
      break;

    case GD_ENGINE_BD2:
      return gd_defaults_bd2;
      break;

    case GD_ENGINE_PLCK:
      return gd_defaults_plck;
      break;

    case GD_ENGINE_1STB:
      return gd_defaults_1stb;
      break;

    case GD_ENGINE_CRDR7:
      return gd_defaults_crdr_7;
      break;

    case GD_ENGINE_CRLI:
      return gd_defaults_crli;
      break;

      // to avoid compiler warning
    case GD_ENGINE_INVALID:
      break;
  }

  return gd_defaults_bd1;
}

// Set engine parameters to some defaults in cave.
void gd_cave_set_engine_defaults(GdCave *cave, GdEngine engine)
{
  gd_cave_set_defaults_from_array(cave, gd_get_engine_default_array(engine));

  // these have hardcoded ckdelay.
  // setting this ckdelay array does not fit into the gd_struct_default scheme.
  if (engine == GD_ENGINE_BD1)
  {
    cave->level_ckdelay[0] = 12;
    cave->level_ckdelay[1] = 6;
    cave->level_ckdelay[2] = 3;
    cave->level_ckdelay[3] = 1;
    cave->level_ckdelay[4] = 0;
  }

  if (engine == GD_ENGINE_BD2)
  {
    cave->level_ckdelay[0] = 9;    // 180ms
    cave->level_ckdelay[1] = 8;    // 160ms
    cave->level_ckdelay[2] = 7;    // 140ms
    cave->level_ckdelay[3] = 6;    // 120ms
    cave->level_ckdelay[4] = 6;    // 120ms (!) same as level 4
  }
}

// used for bdcff engine flag.
static const char *gd_engines[] =
{
  "BD1",
  "BD2",
  "PLCK",
  "1stB",
  "CrDr",
  "CrLi"
};

// Take a slime predictability byte, and convert it to a bitmask.
// Used for caves created with the original construction kit.
// (This conversion method is used by the new game engine.)
static int slime_perm_plck_new(int c64_data)
{
  const int values[] = { 0x00, 0x10, 0x18, 0x38, 0x3c, 0x7c, 0x7e, 0xfe, 0xff };

  if (c64_data > ARRAY_SIZE(values))
  {
    Warn("Invalid PLCK slime permeability value %x", c64_data);

    return 0xff;
  }

  return values[c64_data];
}

// Take a slime predictability byte, and convert it to a bitmask.
// Used for caves created with the original construction kit.
// (This conversion method was used by the old game engine.)
static int slime_perm_plck_old(int c64_data)
{
  int i, perm;

  perm = 0;

  for (i = 0; i < c64_data; i++)
    // shift in this many msb 1's
    perm = (0x100 | perm) >> 1;

  return perm;
}

// Import a BD1 element.
//
// @param c The code byte used in BD1
// @param i Index in the array, from which the byte was read.
//          Only to be able to report the error correctly
// @return The GDash element.
static GdElement bd1_import_byte(byte c, int i)
{
  GdElement e = O_UNKNOWN;

  if (c < ARRAY_SIZE(import_table_bd1))
    e = import_table_bd1[c];

  if (e == O_UNKNOWN)
    Warn("Invalid BD1 element in imported file at cave data %d: %d", i, c);

  return non_scanned_pair(e);
}

static GdElement bd1_import(const byte data[], int i)
{
  return bd1_import_byte(data[i], i);
}

// Import a deluxe caves 1 element.
// Deluxe caves 1 contained a special element, non-sloped brick.
// No import table only for that.
//
// @param c The code byte used in the imported cave data.
// @param i Index in the array, from which the byte was read.
//          Only to be able to report the error correctly
// @return The GDash element.
static GdElement deluxecaves_1_import_byte(byte c, int i)
{
  if (c == 0x2a)
    return O_BRICK_NON_SLOPED;

  return bd1_import_byte(c, i);
}

static GdElement deluxecaves_1_import(const byte data[], int i)
{
  return deluxecaves_1_import_byte(data[i], i);
}

// Import a deluxe caves 3 element.
// Deluxe caves 3 contained some special elements, like non-sloped brick.
// No import table only for that.
//
// @param c The code byte used in the imported cave data.
// @param i Index in the array, from which the byte was read.
//          Only to be able to report the error correctly
// @return The GDash element.
static GdElement deluxecaves_3_import_byte(byte c, int i)
{
  switch (c)
  {
    case 0x2a:
      return O_BRICK_NON_SLOPED;
    case 0x18:
      return O_DIRT_GLUED;
    case 0x19:
      return O_DIAMOND_GLUED;
    case 0x1a:
      return O_STONE_GLUED;
  }

  return bd1_import_byte(c, i);
}

static GdElement deluxecaves_3_import(const byte data[], int i)
{
  return deluxecaves_3_import_byte(data[i], i);
}

// Import a 1stB element.
// @param c The code byte used in the original data file
// @param i Index in the array, from which the byte was read.
//          Only to be able to report the error correctly
// @return The GDash element.
static GdElement firstboulder_import_byte(byte c, int i)
{
  GdElement e = O_UNKNOWN;

  if (c < ARRAY_SIZE(import_table_1stb))
    e = import_table_1stb[c];

  if (e == O_UNKNOWN)
    Warn("Invalid 1stB element in imported file at cave data %d: %d", i, c);

  return non_scanned_pair(e);
}

static GdElement firstboulder_import(const byte data[], int i)
{
  return firstboulder_import_byte(data[i], i);
}

// Import a Crazy Light element.
// @param c The code byte used in the original data file
// @param i Index in the array, from which the byte was read.
//          Only to be able to report the error correctly
// @return The GDash element.
static GdElement crazylight_import_byte(byte c, int i)
{
  GdElement e = O_UNKNOWN;

  if (c < ARRAY_SIZE(import_table_crli))
    e = import_table_crli[c];

  if (e == O_UNKNOWN)
    Warn("Invalid CrLi element in imported file at cave data %d: %d", i, c);

  return non_scanned_pair(e);
}

static GdElement crazylight_import(const byte data[], int i)
{
  return crazylight_import_byte(data[i], i);
}

GdEngine gd_cave_get_engine_from_string(const char *param)
{
  int i;

  for (i = 0; i < GD_ENGINE_INVALID; i++)
    if (strcasecmp(param, gd_engines[i]) == 0)
      return (GdEngine)i;

  return GD_ENGINE_INVALID;
}

// ============================================================================
//
// cave import routines.
// take a cave, data, and maybe remaining bytes.
// return the number of bytes read, -1 if error.
//
// ============================================================================

/*
 * Checksum function for cave import routines.
 * Used to recognize caves which need some hacks added,
 * besides normal importing.
 * @param data The input array of bytes
 * @param length The size
 * @return 16-bit checksum
 */

static unsigned int checksum(const byte *data, int length)
{
  unsigned int a = 1, b = 0;
  int i;

  for (i = 0; i < length; i++)
  {
    a = (a + data[i]) % 251;    // the prime closest to (and less than) 256
    b = (b + a) % 251;
  }

  return b * 256 + a;
}

/*
  take care of required diamonds values == 0 or > 100.
  in original bd, the counter was only two-digit. so bd3 cave f
  says 150 diamonds required, but you only had to collect 50.
  also, gate opening is triggered by incrementing diamond
  count and THEN checking if more required; so if required was
  0, you had to collect 100. (also check crazy light 8 cave "1000")

  http://www.boulder-dash.nl/forum/viewtopic.php?t=88
*/

// import bd1 cave data into our format.
static int cave_copy_from_bd1(GdCave *cave, const byte *data, int remaining_bytes,
			      GdCavefileFormat format, GdImportHack hack)
{
  int index;
  int level;
  GdElement (*import_func) (const byte data[], int i);
  GdElement (*import_func_byte) (byte c, int i);
  int i;

  // cant be shorted than this: header + no objects + delimiter
  if (remaining_bytes < 33)
  {
    Error("truncated BD1 cave data, %d bytes", remaining_bytes);

    return -1;
  }

  gd_cave_set_engine_defaults(cave, GD_ENGINE_BD1);

  if (format == GD_FORMAT_BD1_ATARI)
    cave->scheduling = GD_SCHEDULING_BD1_ATARI;

  // set import func
  if (hack == GD_HACK_DC1)
  {
    import_func      = deluxecaves_1_import;
    import_func_byte = deluxecaves_1_import_byte;
  }
  else if (hack == GD_HACK_DC3)
  {
    import_func      = deluxecaves_3_import;
    import_func_byte = deluxecaves_3_import_byte;
  }
  else
  {
    import_func      = bd1_import;
    import_func_byte = bd1_import_byte;
  }

  // set visible size for intermission
  if (cave->intermission)
  {
    cave->x2 = 19;
    cave->y2 = 11;
  }

  // cave number data[0]
  cave->diamond_value = data[2];
  cave->extra_diamond_value = data[3];

  for (level = 0; level < 5; level++)
  {
    cave->level_amoeba_time[level] = data[1];

    // 0 immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[level] == 0)
      cave->level_amoeba_time[level] = 999;

    cave->level_magic_wall_time[level] = data[1];
    cave->level_rand[level] = data[4 + level];
    cave->level_diamonds[level] = data[9 + level] % 100;    // check comment above

    // gate opening is checked AFTER adding to diamonds collected, so 0 here means 100 to collect
    if (cave->level_diamonds[level] == 0)
      cave->level_diamonds[level] = 100;

    cave->level_time[level] = data[14 + level];

    // 0 immediately underflowed to 999.
    if (cave->level_time[level] == 0)
      cave->level_time[level] = 999;
  }

  /*
    LogicDeLuxe extension: acid
    $16 Acid speed (unused in the original BD1)
    $17 Bit 2: if set, Acid's original position converts to explosion puff during spreading.
    Otherwise, Acid remains intact, ie. it's just growing. (unused in the original BD1)
    $1C Acid eats this element. (also Probability of element 1)

    there is no problem importing these; as other bd1 caves did not contain acid at all,
    so it does not matter how we set the values.
  */

  if (data[0x1c] < 0x40)
  {
    GdElement acid_eat = import_func(data, 0x1c);

    if (acid_eat != O_UNKNOWN)
      cave->acid_eats_this = acid_eat;
  }

  // acid speed, * 1000000.0 as probabilities are stored in int
  cave->acid_spread_ratio = data[0x16] * 1000000.0 / 255.0 + 0.5;

  cave->acid_turns_to = ((data[0x17] & (1 << 2)) ? O_EXPLODE_3 : O_ACID);

  if (format == GD_FORMAT_BD1_ATARI)
  {
    // atari colors
    cave->color1 = gd_atari_color(data[0x13]);
    cave->color2 = gd_atari_color(data[0x14]);
    cave->color3 = gd_atari_color(data[0x15]);
    cave->color4 = gd_atari_color(data[0x16]);      // in atari, amoeba was green
    cave->color5 = gd_atari_color(data[0x16]);      // in atari, slime was green
    cave->colorb = gd_atari_color(data[0x17]);      // border = background
    cave->color0 = gd_atari_color(data[0x17]);      // background
    cave->color6 = gd_atari_color(8);               // extra color 1 - gray
    cave->color7 = gd_atari_color(15);              // extra color 2 - white
  }
  else
  {
    // c64 colors
    cave->colorb = gd_c64_color(0);                 // border = background, fixed color
    cave->color0 = gd_c64_color(0);                 // background, fixed color
    cave->color1 = gd_c64_color(data[0x13] & 0xf);
    cave->color2 = gd_c64_color(data[0x14] & 0xf);
    cave->color3 = gd_c64_color(data[0x15] & 0x7);  // lower 3 bits only (vic-ii worked this way)
    cave->color4 = cave->color3;                    // in bd1, amoeba was color3
    cave->color5 = cave->color3;                    // no slime, but let it be color 3
    cave->color6 = gd_c64_color(12);                // extra color 1 - gray
    cave->color7 = gd_c64_color(1);                 // extra color 2 - white
  }

  // random fill
  for (i = 0; i < 4; i++)
  {
    cave->random_fill[i] = import_func(data, 24 + i);
    cave->random_fill_probability[i] = data[28 + i];
  }

  /*
   * Decode the explicit cave data
   */

  index = 32;

  while (index < remaining_bytes && data[index] != 0xFF && index < 255)
  {
    byte code = data[index];

    // crazy dream 3 extension:
    if (code == 0x0f)
    {
      int x, y;

      // as this one uses nonstandard dx dy values, create points instead
      int x1 = data[index + 2];
      int y1 = data[index + 3] - 2;
      int nx = data[index + 4];
      int ny = data[index + 5];
      int dx = data[index + 6];
      int dy = data[index + 7] + 1;
      GdElement element = import_func(data, index + 1);

      for (y = 0; y < ny; y++)
      {
	for (x = 0; x < nx; x++)
	{
	  int pos = x1 + y1 * 40 + y * dy * 40 + x * dx;

	  cave->objects =
            list_append(cave->objects,
                        gd_object_new_point(GD_OBJECT_LEVEL_ALL,
                                            pos % 40, pos / 40, element));
	}
      }

      index += 8;
    }
    else
    {
      // object is code & 3f, object type is upper 2 bits
      GdElement element = import_func_byte(code & 0x3F, index);

      // 2 bits MSB determine object type
      switch ((code >> 6) & 3)
      {
	case 0:                // 00: POINT
        {
	  int x1 = data[index + 1];
	  int y1 = data[index + 2] - 2;

          if (x1 >= cave->w || y1 >= cave->h)
            Warn("invalid point coordinates %d,%d at byte %d", x1, y1, index);

	  cave->objects =
            list_append(cave->objects,
                        gd_object_new_point(GD_OBJECT_LEVEL_ALL,
                                            x1, y1, element));

	  index += 3;
	  break;
        }

	case 1:                // 01: LINE
        {
	  int x1 = data[index + 1];
	  int y1 = data[index + 2] - 2;
	  int length = (byte)data[index + 3] - 1;
	  int direction = data[index + 4];

	  if (length >= 0)
	  {
	    if (direction > GD_MV_UP_LEFT)
	    {
	      Warn("invalid line direction %d at byte %d", direction, index);
	      direction = GD_MV_STILL;
	    }

	    int x2 = x1 + length * gd_dx[direction + 1];
	    int y2 = y1 + length * gd_dy[direction + 1];

	    if (x1 >= cave->w ||
		y1 >= cave->h ||
		x2 >= cave->w ||
		y2 >= cave->h)
	      Warn("invalid line coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	    cave->objects =
              list_append(cave->objects,
                          gd_object_new_line(GD_OBJECT_LEVEL_ALL,
                                             x1, y1, x2, y2, element));
	  }
	  else
	  {
	    Warn("line length negative, not displaying line at all, at byte %d", index);
	  }

	  index += 5;
	  break;
        }

	case 2:                // 10: FILLED RECTANGLE
        {
	  int x1 = data[index + 1];
	  int y1 = data[index + 2] - 2;
	  int x2 = x1 + data[index + 3] - 1;    // width
	  int y2 = y1 + data[index + 4] - 1;    // height

	  if (x1 >= cave->w ||
	      y1 >= cave->h ||
	      x2 >= cave->w ||
	      y2 >= cave->h)
	    Warn("invalid filled rectangle coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	  cave->objects =
            list_append(cave->objects,
                        gd_object_new_filled_rectangle(GD_OBJECT_LEVEL_ALL,
                                                       x1, y1, x2, y2, element,
                                                       import_func(data, index + 5)));

	  index += 6;
	  break;
        }

	case 3:                // 11: OPEN RECTANGLE (OUTLINE)
        {
	  int x1 = data[index + 1];
	  int y1 = data[index + 2] - 2;
	  int x2 = x1 + data[index + 3] - 1;    // width
	  int y2 = y1 + data[index + 4] - 1;    // height

	  if (x1 >= cave->w ||
	      y1 >= cave->h ||
	      x2 >= cave->w ||
	      y2 >= cave->h)
	    Warn("invalid rectangle coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	  cave->objects =
            list_append(cave->objects,
                        gd_object_new_rectangle(GD_OBJECT_LEVEL_ALL,
                                                x1, y1, x2, y2, element));

	  index += 5;
	  break;
        }
      }
    }
  }

  if (data[index] != 0xFF)
  {
    Error("import error, cave not delimited with 0xFF");

    return -1;
  }

  // the delimiter
  index++;

  return index;
}

// Import bd2 cave data into our format.
// Very similar to the bd1 importer, only the encoding was different.
static int cave_copy_from_bd2(GdCave *cave, const byte *data, int remaining_bytes,
			      GdCavefileFormat format)
{
  int index;
  int x, y;
  int i;

  if (remaining_bytes < 0x1A + 5)
  {
    Error("truncated BD2 cave data, %d bytes", remaining_bytes);

    return -1;
  }

  gd_cave_set_engine_defaults(cave, GD_ENGINE_BD2);

  if (format == GD_FORMAT_BD2_ATARI)
    cave->scheduling = GD_SCHEDULING_BD2_PLCK_ATARI;

  // set visible size for intermission
  if (cave->intermission)
  {
    cave->x2 = 19;
    cave->y2 = 11;
  }

  cave->diamond_value = data[1];
  cave->extra_diamond_value = data[2];

  for (i = 0; i < 5; i++)
  {
    cave->level_amoeba_time[i] = data[0];

    // 0 immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[i] == 0)
      cave->level_amoeba_time[i] = 999;

    cave->level_rand[i] = data[13 + i];
    cave->level_diamonds[i] = data[8 + i];

    // gate opening is checked AFTER adding to diamonds collected, so 0 here is 1000 needed
    if (cave->level_diamonds[i] == 0)
      cave->level_diamonds[i] = 1000;

    cave->level_time[i] = data[3 + i];
    cave->level_magic_wall_time[i] = data[0];
  }

  for (i = 0; i < 4; i++)
  {
    cave->random_fill[i] = bd1_import(data, 0x16 + i);
    cave->random_fill_probability[i] = data[0x12 + i];
  }

  /*
   * Decode the explicit cave data
   */

  index = 0x1A;

  while (index < remaining_bytes && data[index] != 0xFF)
  {
    // EXTREME CARE HERE! coordinates are in "wrong" order, first y then x.

    switch (data[index])
    {
      case 0:                // LINE
      {
	GdElement element = bd1_import(data, index + 1);
	int x1 = data[index + 3];
	int y1 = data[index + 2];

	// they are multiplied by two - 0 is up, 2 is upright, 4 is right...
	int direction = data[index + 4] / 2;
	int length = data[index + 5] - 1;

	if (direction > GD_MV_UP_LEFT)
	{
	  Warn("invalid line direction %d at byte %d", direction, index);

	  direction = GD_MV_STILL;
	}

	int x2 = x1 + length * gd_dx[direction + 1];
	int y2 = y1 + length * gd_dy[direction + 1];

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid line coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_line(GD_OBJECT_LEVEL_ALL,
                                         x1, y1, x2, y2, element));

	index += 6;
	break;
      }

      case 1:                // OPEN RECTANGLE
      {
	GdElement element = bd1_import(data, index + 1);
	int x1 = data[index + 3];
	int y1 = data[index + 2];
	int x2 = x1 + data[index + 5] - 1;    // width
	int y2 = y1 + data[index + 4] - 1;    // height

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid rectangle coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_rectangle(GD_OBJECT_LEVEL_ALL,
                                              x1, y1, x2, y2, element));

	index += 6;
	break;
      }

      case 2:                // FILLED RECTANGLE
      {
	GdElement element = bd1_import(data, index + 1);
	int x1 = data[index + 3];
	int y1 = data[index + 2];
	int x2 = x1 + data[index + 5] - 1;    // width
	int y2 = y1 + data[index + 4] - 1;    // height
	GdElement fill_element = bd1_import(data, index + 6);

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid filled rectangle coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_filled_rectangle(GD_OBJECT_LEVEL_ALL,
                                                     x1, y1, x2, y2, element, fill_element));

	index += 7;
	break;
      }

      case 3:                // POINT
      {
	GdElement element = bd1_import(data, index + 1);
	int x1 = data[index + 3];
	int y1 = data[index + 2];

	if (x1 >= cave->w ||
	    y1 >= cave->h)
	  Warn("invalid point coordinates %d,%d at byte %d", x1, y1, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_point(GD_OBJECT_LEVEL_ALL,
                                          x1, y1, element));

	index += 4;
	break;
      }

      case 4:                // RASTER
      {
	GdElement element = bd1_import(data, index + 1);
	int x1 = data[index + 3];     // starting pos
	int y1 = data[index + 2];
	int nx = data[index + 5] - 1; // number of elements
	int ny = data[index + 4] - 1;
	int dx = data[index + 7];     // displacement
	int dy = data[index + 6];
	int x2 = x1 + dx * nx;        // calculate rectangle
	int y2 = y1 + dy * ny;

	// guess this has to be here, after x2,y2 calculation,
        // because of some bugs in imported data
	if (dx < 1)
	  dx = 1;
	if (dy < 1)
	  dy = 1;

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid raster coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_raster(GD_OBJECT_LEVEL_ALL,
                                           x1, y1, x2, y2, dx, dy, element));

	index += 8;
	break;
      }

      case 5:
      {
	// profi boulder extension: bitmap
	GdElement element = bd1_import(data, index + 1);
	int bytes = data[index + 2];    // number of bytes in bitmap

	if (bytes >= cave->w * cave->h / 8)
	  Warn("invalid bitmap length at byte %d", index - 4);

	int addr = 0;

	addr += data[index + 3];         // msb
	addr += data[index + 4] << 8;    // lsb

	// this was a pointer to the cave work memory (used during game).
	addr -= 0x0850;

	if (addr >= cave->w * cave->h)
	  Warn("invalid bitmap start address at byte %d", index - 4);

	int x1 = addr % 40;
	int y1 = addr / 40;

	for (i = 0; i < bytes; i++)
	{
	  // for ("bytes" number of bytes)
	  int val = data[index + 5 + i];
          int n;

	  for (n = 0; n < 8; n++)
	  {
	    // for (8 bits in a byte)
	    if (val & 1) // convert to single points...
	      cave->objects =
                list_append(cave->objects,
                            gd_object_new_point(GD_OBJECT_LEVEL_ALL,
                                                x1, y1, element));

	    val >>= 1;
	    x1++;   // next cave pos

	    if (x1 >= cave->w)
	    {
	      // maybe next line in map
	      x1 = 0;
	      y1++;
	    }
	  }
	}

	index += 5 + bytes;    // 5 description bytes and "bytes" data bytes
	break;
      }

      case 6:                // JOIN
      {
	GdElement search_element = bd1_import(data, index + 1);
	GdElement put_element    = bd1_import(data, index + 2);
	int dx = data[index + 3] % 40;    // they are stored ...
	int dy = data[index + 3] / 40;    // ... in the same byte!

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_join(GD_OBJECT_LEVEL_ALL,
                                         dx, dy, search_element, put_element));

	index += 4;
	break;
      }

      case 7:             // SLIME PERMEABILITY
      {
	// interesting this is set here, and not in the cave header
	for (i = 0; i < 5; i++)
	  cave->level_slime_permeability_c64[i] = data[index + 1];

	index += 2;
	break;
      }

      case 9:
      {
	// profi boulder extension by player: plck-like cave map.
        // the import routine (any2gdash) inserts it here.
	if (cave->map != NULL)
	{
	  Error("contains more than one PLCK map");

	  gd_cave_map_free(cave->map);
	}

	cave->map = gd_cave_map_new(cave, GdElement);

	for (x = 0; x < cave->w; x++)
	{
	  // fill the first and the last row with steel wall.
	  cave->map[0][x]           = O_STEEL;
	  cave->map[cave->h - 1][x] = O_STEEL;
	}

	int n = 0;    // number of bytes read from map

	// the first and the last rows are not stored.
	for (y = 1; y < cave->h - 1; y++)
	{
	  for (x = 0; x < cave->w; x += 2)
	  {
	    cave->map[y][x]     = import_table_plck[data[index + 3 + n] >> 4];    // msb 4 bits
	    cave->map[y][x + 1] = import_table_plck[data[index + 3 + n] % 16];    // lsb 4 bits
	    n++;
	  }
	}

	// the position of inbox is stored. this is to check the cave
	int ry = data[index + 1] - 2;
	int rx = data[index + 2];

	// at the start of the cave, bd scrolled to the last player placed during the drawing
	// (setup) of the cave.
	// i think this is why a map also stored the coordinates of the player - we can use
	// this to check its integrity
	if (rx >= cave->w || ry < 0 ||
	    ry >= cave->h || cave->map[ry][rx] != O_INBOX)
	  Warn("embedded PLCK map may be corrupted, player coordinates %d,%d", rx, rx);

	index += 3 + n;
	break;
      }

      default:
      {
	Warn("unknown bd2 extension no. %02x at byte %d", data[index], index);

	index += 1;    // skip that byte
      }
    }
  }

  if (data[index] != 0xFF)
  {
    Error("import error, cave not delimited with 0xFF");

    return -1;
  }

  // skip delimiter
  index++;

  // animation byte - told the engine which objects to animate - to make game faster
  index++;

  // the colors from the memory dump are appended here by any2gdash
  if (format == GD_FORMAT_BD2)
  {
    // c64 colors
    cave->color0 = gd_c64_color(0);
    cave->color1 = gd_c64_color(data[index + 0] & 0xf);
    cave->color2 = gd_c64_color(data[index + 1] & 0xf);
    cave->color3 = gd_c64_color(data[index + 2] & 0x7); // lower 3 bits only!
    cave->color4 = cave->color1;                        // in bd2, amoeba was color1
    cave->color5 = cave->color1;                        // slime too
    cave->color6 = gd_c64_color(12);                    // extra color 1 - gray
    cave->color7 = gd_c64_color(1);                     // extra color 2 - white
    index += 3;
  }
  else
  {
    // atari colors
    cave->color1 = gd_atari_color(data[index + 0]);
    cave->color2 = gd_atari_color(data[index + 1]);
    cave->color3 = gd_atari_color(data[index + 2]);
    cave->color4 = gd_atari_color(data[index + 3]);     // amoeba and slime
    cave->color5 = gd_atari_color(data[index + 3]);
    cave->colorb = gd_atari_color(data[index + 4]);     // background and border
    cave->color0 = gd_atari_color(data[index + 4]);
    cave->color6 = gd_atari_color(8);                   // extra color 1 - gray
    cave->color7 = gd_atari_color(15);                  // extra color 2 - white
    index += 5;
  }

  return index;
}

// Import plck cave data into our format.
// Length is always 512 bytes, and contains if it is an intermission cave.
static int cave_copy_from_plck(GdCave *cave, const byte *data,
			       int remaining_bytes, GdCavefileFormat format)
{
  // i don't really think that all this table is needed, but included to be complete.
  // this is for the dirt and expanding wall looks like effect.
  // it also contains the individual frames
  static GdElement plck_graphic_table[] =
  {
    /* 3000 */ O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
               O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
    /* 3100 */ O_BUTTER_1, O_MAGIC_WALL, O_PRE_DIA_1, O_PRE_DIA_2,
               O_PRE_DIA_3, O_PRE_DIA_4, O_PRE_DIA_5, O_OUTBOX_CLOSED,
    /* 3200 */ O_AMOEBA, O_VOODOO, O_STONE, O_DIRT,
               O_DIAMOND, O_STEEL, O_PLAYER, O_BRICK,
    /* 3300 */ O_SPACE, O_OUTBOX_OPEN, O_FIREFLY_1, O_EXPLODE_1,
               O_EXPLODE_2, O_EXPLODE_3, O_MAGIC_WALL, O_MAGIC_WALL,
    /* 3400 */ O_PLAYER_TAP_BLINK, O_PLAYER_TAP_BLINK, O_PLAYER_TAP_BLINK, O_PLAYER_TAP_BLINK,
               O_PLAYER_TAP_BLINK, O_PLAYER_TAP_BLINK, O_PLAYER_TAP_BLINK, O_PLAYER_TAP_BLINK,
    /* 3500 */ O_PLAYER_LEFT, O_PLAYER_LEFT, O_PLAYER_LEFT, O_PLAYER_LEFT,
               O_PLAYER_LEFT, O_PLAYER_LEFT, O_PLAYER_LEFT, O_PLAYER_LEFT,
    /* 3600 */ O_PLAYER_RIGHT, O_PLAYER_RIGHT, O_PLAYER_RIGHT, O_PLAYER_RIGHT,
               O_PLAYER_RIGHT, O_PLAYER_RIGHT, O_PLAYER_RIGHT, O_PLAYER_RIGHT,
    /* 3700 */ O_BUTTER_1, O_BUTTER_1, O_BUTTER_1, O_BUTTER_1,
               O_BUTTER_1, O_BUTTER_1, O_BUTTER_1, O_BUTTER_1,
    /* 3800 */ O_AMOEBA, O_AMOEBA, O_AMOEBA, O_AMOEBA,
               O_AMOEBA, O_AMOEBA, O_AMOEBA, O_AMOEBA,
  };

  int x, y;
  int i;

  if (remaining_bytes < 512)
  {
    Error("truncated plck cave data!");

    return -1;
  }

  gd_cave_set_engine_defaults(cave, GD_ENGINE_PLCK);

  if (format == GD_FORMAT_PLC_ATARI)
    cave->scheduling = GD_SCHEDULING_BD2_PLCK_ATARI;

  cave->intermission = (data[0x1da] != 0);

  if (cave->intermission)
  {
    // set visible size for intermission
    cave->x2 = 19;
    cave->y2 = 11;
  }

  // cave selection table, was not part of cave data, rather given in game packers.
  // if a new enough version of any2gdash is used, it will put information after the cave.
  // detect this here and act accordingly
  if ((data[0x1f0] == data[0x1f1] - 1) &&
      (data[0x1f0] == 0x19 ||
       data[0x1f0] == 0x0e))
  {
    int j;

    // found selection table
    cave->selectable = (data[0x1f0] == 0x19);
    gd_strcpy(cave->name, "              ");

    for (j = 0; j < 12; j++)
      if (data[0x1f2 + j] >= 32 &&
          data[0x1f2 + j] < 128)
        cave->name[j] = data[0x1f2 + j];

    chompString(cave->name);    // remove spaces
  }
  else
  {
    // no selection info found, let intermissions be unselectable
    cave->selectable = !cave->intermission;
  }

  cave->diamond_value = data[0x1be];
  cave->extra_diamond_value = data[0x1c0];

  for (i = 0; i < 5; i++)
  {
    // plck does not really have levels, so just duplicate data five times
    cave->level_amoeba_time[i] = data[0x1c4];

    // immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[i] == 0)
      cave->level_amoeba_time[i] = 999;

    cave->level_time[i] = data[0x1ba];

    // immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_time[i] == 0)
      cave->level_time[i] = 999;

    cave->level_diamonds[i] = data[0x1bc];

    // gate opening is checked AFTER adding to diamonds collected, so 0 here is 1000 needed
    if (cave->level_diamonds[i] == 0)
      cave->level_diamonds[i] = 1000;

    cave->level_ckdelay[i] = data[0x1b8];
    cave->level_magic_wall_time[i] = data[0x1c6];

    cave->level_slime_permeability_c64[i] = slime_perm_plck_new(data[0x1c2]);
    cave->level_slime_permeability_old[i] = slime_perm_plck_old(data[0x1c2]);

    // set these to the same value on each level, so the engine does
    // not think that levels are different
    cave->level_timevalue[i] = 1;
    cave->level_rand[i] = 0;
    cave->level_speed[i] = cave->level_ckdelay[i] * 20;

    if (cave->level_speed[i] < 80)
      cave->level_speed[i] = 80;
  }

  if (format == GD_FORMAT_PLC_ATARI)
  {
    // use atari colors
    cave->colorb = gd_atari_color(0);               // border

    // indexes in data are not the same order as on c64!!!
    cave->color0 = gd_atari_color(data[0x1e3]);     // background
    cave->color1 = gd_atari_color(data[0x1db]);
    cave->color2 = gd_atari_color(data[0x1dd]);
    cave->color3 = gd_atari_color(data[0x1df]);

    // in atari plck, slime and amoeba could not coexist in the same cave.
    // if amoeba was used, the graphics turned to green, and data at 0x1e1 was set to 0xd4.
    // if slime was used, graphics to blue, and data at 0x1e1 was set to 0x72.
    // these two colors could not be changed in the editor at all.
    // (maybe they could have been changed in a hex editor)
    cave->color4 = gd_atari_color(data[0x1e1]);
    cave->color5 = gd_atari_color(data[0x1e1]);

    // extra colors
    cave->color6 = gd_atari_color(8);               // extra color 1 - gray
    cave->color7 = gd_atari_color(15);              // extra color 2 - white
  }
  else
  {
    // use c64 colors
    cave->colorb = gd_c64_color(data[0x1db] & 0xf); // border
    cave->color0 = gd_c64_color(data[0x1dd] & 0xf);
    cave->color1 = gd_c64_color(data[0x1df] & 0xf);
    cave->color2 = gd_c64_color(data[0x1e1] & 0xf);
    cave->color3 = gd_c64_color(data[0x1e3] & 0x7); // lower 3 bits only!
    cave->color4 = cave->color3;                    // in plck, amoeba was color3
    cave->color5 = cave->color3;                    // same for slime
    cave->color6 = gd_c64_color(12);                // extra color 1 - gray
    cave->color7 = gd_c64_color(1);                 // extra color 2 - white
  }

  // ... the cave is stored like a map.
  cave->map = gd_cave_map_new(cave, GdElement);

  // cave map looked like this.
  // two rows of steel wall ($44's), then cave description, 20 bytes (40 nybbles) for each line.
  // the bottom and top lines were not stored... originally.
  // some games write to the top line; so we import that, too.
  // also dlp 155 allowed writing to the bottom line; the first 20 $44-s now store the bottom line.
  // so the cave is essentially shifted one row down in the file:
  // cave->map[y][x] = data[... y+1 mod height ][x]

  // FOR NOW, WE DO NOT IMPORT THE BOTTOM BORDER
  for (y = 0; y < cave->h - 1; y++)
  {
    for (x = 0; x < cave->w; x += 2)
    {
      // msb 4 bits: we do not check index ranges, as >>4 and %16 will result in 0..15
      cave->map[y][x]     = import_table_plck[data[((y + 1) % cave->h) * 20 + x / 2] >> 4];

      // lsb 4 bits
      cave->map[y][x + 1] = import_table_plck[data[((y + 1) % cave->h) * 20 + x / 2] % 16];
    }
  }

  // (... instead, set bottom border to steel wall)
  for (x = 0; x < cave->w; x++)
    cave->map[cave->h - 1][x] = O_STEEL;

  // check for diego-effects
  // c64 magic values (byte sequences)  0x20 0x90 0x46, also 0xa9 0x1c 0x85
  if ((data[0x1e5] == 0x20 && data[0x1e6] == 0x90 && data[0x1e7] == 0x46) ||
      (data[0x1e5] == 0xa9 && data[0x1e6] == 0x1c && data[0x1e7] == 0x85))
  {
    // diego effects enabled.
    cave->stone_bouncing_effect  = bd1_import(data, 0x1ea);
    cave->diamond_falling_effect = bd1_import(data, 0x1eb);

    // explosions: 0x1e was explosion 4.
    cave->explosion_3_effect = bd1_import(data, 0x1ec);

    /*
      pointer to element graphic.
      two bytes/column (one element), that is data[xxx] % 16 / 2.
      also there are 16bytes/row.
      that is, 0x44 = stone, upper left character. 0x45 = upper right,
      0x54 = lower right, 0x55 = lower right.
      so high nybble must be shifted right twice -> data[xxx]/16*4.
    */
    cave->dirt_looks_like           = plck_graphic_table[(data[0x1ed] / 16) * 4 + (data[0x1ed] % 16) / 2];
    cave->expanding_wall_looks_like = plck_graphic_table[(data[0x1ee] / 16) * 4 + (data[0x1ee] % 16) / 2];

    for (i = 0; i < 5; i++)
      cave->level_amoeba_threshold[i] = data[0x1ef];
  }

  return 512;
}

// No one's delight boulder dash importer.
// essentially rle compressed plck maps.
static int cave_copy_from_dlb(GdCave *cave, const byte *data, int remaining_bytes)
{
  int x, y;
  int i;

  gd_cave_set_engine_defaults(cave, GD_ENGINE_PLCK); // essentially the plck engine

  for (i = 0; i < 5; i++)
  {
    // does not really have levels, so just duplicate data five times
    cave->level_time[i] = data[1];
    cave->level_diamonds[i] = data[2];

    // gate opening is checked AFTER adding to diamonds collected, so 0 here is 1000 needed
    if (cave->level_diamonds[i] == 0)
      cave->level_diamonds[i] = 1000;

    cave->level_ckdelay[i] = data[0];
    cave->level_amoeba_time[i] = data[6];

    // 0 immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[i] == 0)
      cave->level_amoeba_time[i] = 999;

    cave->level_magic_wall_time[i] = data[7];
    cave->level_slime_permeability_c64[i] = slime_perm_plck_new(data[5]);
    cave->level_slime_permeability_old[i] = slime_perm_plck_old(data[5]);

    // set these to the same value on each level, so the engine does
    // not think that levels are different */
    cave->level_timevalue[i] = 1;
    cave->level_rand[i] = 0;
    cave->level_speed[i] = cave->level_ckdelay[i] * 20;
    if (cave->level_speed[i] < 80)
      cave->level_speed[i] = 80;
  }

  cave->diamond_value = data[3];
  cave->extra_diamond_value = data[4];

  // then 5 color bytes follow
  cave->colorb = gd_c64_color(data[8]  & 0xf);    // border
  cave->color0 = gd_c64_color(data[9]  & 0xf);
  cave->color1 = gd_c64_color(data[10] & 0xf);
  cave->color2 = gd_c64_color(data[11] & 0xf);
  cave->color3 = gd_c64_color(data[12] & 0x7);    // lower 3 bits only!
  cave->color4 = cave->color3;                    // in plck, amoeba was color3
  cave->color5 = cave->color3;                    // same for slime
  cave->color6 = gd_c64_color(12);                // extra color 1 - gray
  cave->color7 = gd_c64_color(1);                 // extra color 2 - white

  // cave map is compressed.
  // employ a state machine to decompress data.

  // cave map
  int pos = 13;                     // those 13 bytes were the cave values above
  int cavepos = 0;
  byte databyte = 0;                // just to get rid of compiler warning
  byte separator = 0;               // just to get rid of compiler warning
  byte decomp[512];

  enum
  {
    START,        // initial state
    SEPARATOR,    // got a separator
    RLE,          // after a separator, got the byte to duplicate
    NORMAL        // normal, copy bytes till separator
  } state;

  state = START;

  while (cavepos < 400 && pos < remaining_bytes)
  {
    switch (state)
    {
      case START:
	// first byte is a separator. remember it
	separator = data[pos];

	// after the first separator, no rle data, just copy.
	state = NORMAL;
	break;

      case SEPARATOR:
	// we had a separator. remember this byte, as this will be duplicated (or more)
	databyte = data[pos];
	state = RLE;
	break;

      case RLE:
	// we had the first byte, duplicate this n times.
	if (data[pos] == 0xff)
	{
	  // if it is a 0xff, we will have another byte, which is also a length specifier.
	  // and for this one, duplicate only 254 times
	  if (cavepos + 254 > 400)
	  {
	    Error("DLB import error: RLE data overflows buffer");

	    return -1;
	  }

	  for (i = 0; i < 254; i++)
	    decomp[cavepos++] = databyte;
	}
	else
	{
	  // if not 0xff, duplicate n times and back to copy mode
	  if (cavepos + data[pos] > 400)
	  {
	    Error("DLB import error: RLE data overflows buffer");

	    return -1;
	  }

	  for (i = 0; i < data[pos]; i++)
	    decomp[cavepos++] = databyte;

	  state = NORMAL;
	}
	break;

      case NORMAL:
	// bytes duplicated; now only copy the remaining, till the next separator.
	if (data[pos] == separator)
	  state = SEPARATOR;
	else
	  decomp[cavepos++] = data[pos];    // copy this byte and state is still NORMAL
	break;
    }

    pos++;
  }

  if (cavepos != 400)
  {
    Error("DLB import error: RLE processing, cave length %d, should be 400", cavepos);

    return -1;
  }

  cave->map = gd_cave_map_new(cave, GdElement);

  // first and last row was not stored - it is always steel wall
  for (x = 0; x < cave->w; x++)
  {
    // fill the first and the last row with steel wall.
    cave->map[0][x]           = O_STEEL;
    cave->map[cave->h - 1][x] = O_STEEL;
  }

  // process uncompressed map
  for (y = 1; y < cave->h - 1; y++)
  {
    for (x = 0; x < cave->w; x += 2)
    {
      // msb 4 bits
      cave->map[y][x]     = import_table_plck[decomp[((y - 1) * cave->w + x) / 2] >> 4];
      // lsb 4 bits
      cave->map[y][x + 1] = import_table_plck[decomp[((y - 1) * cave->w + x) / 2] % 16];
    }
  }

  // return number of bytes read from buffer
  return pos;
}

static char *get_name_from_c64_bin(const byte *data)
{
  static GdString name;
  int i;

  gd_strcpy(name, "              ");

  for (i = 0; i < 14; i++)
  {
    int c = data[i];

    // import cave name; a conversion table is used for each character
    if (c < 0x40)
      c = bd_internal_character_encoding[c];
    else if (c == 0x74)
      c = ' ';
    else if (c == 0x76)
      c = '?';
    else
      c = ' ';    // don't know this, so change to space

    // from the second one, change to lowercase
    if (i > 0)
      c = tolower(c);

    name[i] = c;
  }

  // remove trailing and leading spaces
  chompString(name);

  return name;
}

// This function processes an amoeba probability pattern.
//
// @param input The byte read from the file
// @return The GDash probability
static int amoeba_probability(byte input)
{
  // drop lower 2 bits
  input = input >> 2;

  // count the number of '1' bits
  int bits = 0;

  // while has any more bits
  while (input != 0)
  {
    // if the lowest bit is one, increment
    if (input & 1)
      bits++;

    // and drop the lowest one.
    input >>= 1;
  }

  // here we have the number of 1 bits in the upper six bits.
  // The order does not count. The probability of all being
  // zero (thus the amoeba growing) is 1/2^bits.
  return 1000000.0 / (1 << bits) + 0.5;
}

// Import a 1stB encoded cave.
static int cave_copy_from_1stb(GdCave *cave, const byte *data, int remaining_bytes)
{
  int x, y;
  int i;

  if (remaining_bytes < 1024)
  {
    Error("truncated 1stb cave data!");

    return -1;
  }

  gd_cave_set_engine_defaults(cave, GD_ENGINE_1STB);

  // copy name
  gd_strcpy(cave->name, get_name_from_c64_bin(&data[0x3a0]));

  cave->intermission = (data[0x389] != 0);

  // if it is intermission but not scrollable
  if (cave->intermission && !data[0x38c])
  {
    cave->x2 = 19;
    cave->y2 = 11;
  }

  cave->diamond_value       = 100 * data[0x379] + 10 * data[0x379 + 1] + data[0x379 + 2];
  cave->extra_diamond_value = 100 * data[0x376] + 10 * data[0x376 + 1] + data[0x376 + 2];

  for (i = 0; i < 5; i++)
  {
    // 1stb doesnot really have levels, so just duplicate data five times
    cave->level_time[i] = 100 * data[0x370] + 10 * data[0x370+1] + data[0x370 + 2];

    // same as gate opening after 0 diamonds
    if (cave->level_time[i] == 0)
      cave->level_time[i] = 1000;

    cave->level_diamonds[i] = 100 * data[0x373] + 10 * data[0x373 + 1] + data[0x373 + 2];

    // gate opening is checked AFTER adding to diamonds collected, so 0 here is 1000 (!) needed
    if (cave->level_diamonds[i] == 0)
      cave->level_diamonds[i] = 1000;

    cave->level_ckdelay[i] = data[0x38a];
    cave->level_amoeba_time[i] = 256 * (int)data[0x37c] + data[0x37d];

    // 0 immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[i] == 0)
      cave->level_amoeba_time[i] = 999;

    cave->level_magic_wall_time[i] = 256 * (int)data[0x37e] + data[0x37f];
    cave->level_slime_permeability_c64[i] = data[0x38b];
    cave->level_bonus_time[i] = data[0x392];
    cave->level_penalty_time[i] = data[0x393];
    cave->level_amoeba_threshold[i] = 256 * (int)data[0x390] + data[0x390 + 1];

    // set these to the same value on each level, so the engine does
    // not think that levels are different
    cave->level_timevalue[i] = 1;
    cave->level_rand[i] = 0;
    cave->level_speed[i] = cave->level_ckdelay[i] * 20;
    if (cave->level_speed[i] < 80)
      cave->level_speed[i] = 80;
  }

  // also has no random data...

  cave->colorb = gd_c64_color(data[0x384] & 0xf);    // border
  cave->color0 = gd_c64_color(data[0x385] & 0xf);
  cave->color1 = gd_c64_color(data[0x386] & 0xf);
  cave->color2 = gd_c64_color(data[0x387] & 0xf);
  cave->color3 = gd_c64_color(data[0x388] & 0x7);     // lower 3 bits only!
  cave->color4 = cave->color1;
  cave->color5 = cave->color1;
  cave->color6 = gd_c64_color(12);                    // extra color 1 - gray
  cave->color7 = gd_c64_color(1);                     // extra color 2 - white

  cave->amoeba_growth_prob      = amoeba_probability(data[0x382]);
  cave->amoeba_fast_growth_prob = amoeba_probability(data[0x383]);

  if (data[0x380] != 0)
    cave->creatures_direction_auto_change_time = data[0x381];
  else
    cave->diagonal_movements = (data[0x381] != 0);

  // ... the cave is stored like a map.
  cave->map = gd_cave_map_new(cave, GdElement);
  for (y = 0; y < cave->h; y++)
    for (x = 0; x < cave->w; x++)
      cave->map[y][x] = firstboulder_import(data, y * 40 + x);

  cave->magic_wall_sound = (data[0x38d] == 0xf1);

  // 2d was a normal switch, 2e a changed one.
  cave->creatures_backwards = (data[0x38f] == 0x2d);

  // 2e horizontal, 2f vertical.
  cave->expanding_wall_changed = (data[0x38e] == 0x2f);

  cave->biter_delay_frame	= data[0x394];
  cave->magic_wall_stops_amoeba	= (data[0x395] == 0);    // negated!!

  cave->bomb_explosion_effect	= firstboulder_import(data, 0x396);
  cave->explosion_effect	= firstboulder_import(data, 0x397);
  cave->stone_bouncing_effect	= firstboulder_import(data, 0x398);
  cave->diamond_birth_effect	= firstboulder_import(data, 0x399);
  cave->magic_diamond_to	= firstboulder_import(data, 0x39a);

  cave->bladder_converts_by	= firstboulder_import(data, 0x39b);
  cave->diamond_falling_effect	= firstboulder_import(data, 0x39c);
  cave->biter_eat		= firstboulder_import(data, 0x39d);

  // slime does this: eats element 1, and adds 3 to it.
  // for example: element1 == stone, and codes were:
  // stone, stone (scanned), stone falling, stone falling (scanned)
  // so by eating a stone it created a stone falling scanned.
  cave->slime_eats_1		= firstboulder_import(data, 0x39e);
  cave->slime_converts_1	= firstboulder_import_byte(data[0x39e] + 3, 0x39e);
  cave->slime_eats_2		= firstboulder_import(data, 0x39f);
  cave->slime_converts_2	= firstboulder_import_byte(data[0x39f] + 3, 0x39f);

  cave->magic_diamond_to	= firstboulder_import(data, 0x39a);

  // length is always 1024 bytes
  return 1024;
}

// Import a crazy dream 7 cave.
static int cave_copy_from_crdr_7(GdCave *cave, const byte *data, int remaining_bytes)
{
  int index;
  int i;

  // if we have name, convert it
  gd_strcpy(cave->name, get_name_from_c64_bin(data));

  cave->selectable = (data[14] != 0);

  // jump 15 bytes, 14 was the name and 15 selectability
  data += 15;

  if (memcmp((char *)data + 0x30, "V4\0020", 4) != 0)
    Warn("unknown crdr version %c%c%c%c", data[0x30], data[0x31], data[0x32], data[0x33]);

  gd_cave_set_engine_defaults(cave, GD_ENGINE_CRDR7);

  for (i = 0; i < 5; i++)
  {
    cave->level_time[i] = (int)data[0x0] * 100 + data[0x1] * 10 + data[0x2];

    // same as gate opening after 0 diamonds
    if (cave->level_time[i] == 0)
      cave->level_time[i] = 1000;

    cave->level_diamonds[i] = (int)data[0x3] * 100 + data[0x4] * 10 + data[0x5];

    // gate opening is checked AFTER adding to diamonds collected, so 0 here is 1000 (!) needed
    if (cave->level_diamonds[i] == 0)
      cave->level_diamonds[i] = 1000;

    cave->level_ckdelay[i] = data[0x1A];
    cave->level_rand[i] = data[0x40];
    cave->level_amoeba_time[i] = (int)data[0xC] * 256 + data[0xD];

    // 0 immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[i] == 0)
      cave->level_amoeba_time[i] = 999;

    cave->level_magic_wall_time[i] = (int)data[0xE] * 256 + data[0xF];
    cave->level_slime_permeability_c64[i] = data[0x1B];
    cave->level_bonus_time[i] = data[0x22];
    cave->level_penalty_time[i] = data[0x23];
    cave->level_amoeba_threshold[i] = 256 * (int)data[0x20] + data[0x21];

    // set these to the same value on each level, so the engine does
    // not think that levels are different
    cave->level_timevalue[i] = 1;
    cave->level_speed[i] = cave->level_ckdelay[i] * 20;
    if (cave->level_speed[i] < 80)
      cave->level_speed[i] = 80;
  }

  cave->extra_diamond_value = (int)data[0x6] * 100 + data[0x7] * 10 + data[0x8];
  cave->diamond_value       = (int)data[0x9] * 100 + data[0xA] * 10 + data[0xB];

  if (data[0x10])
    cave->creatures_direction_auto_change_time = data[0x11];

  cave->colorb = gd_c64_color(data[0x14] & 0xf);    // border
  cave->color0 = gd_c64_color(data[0x15] & 0xf);
  cave->color1 = gd_c64_color(data[0x16] & 0xf);
  cave->color2 = gd_c64_color(data[0x17] & 0xf);
  cave->color3 = gd_c64_color(data[0x18] & 0x7);    // lower 3 bits only!
  cave->color4 = cave->color3;
  cave->color5 = cave->color1;
  cave->color6 = gd_c64_color(12);                  // extra color 1 - gray
  cave->color7 = gd_c64_color(1);                   // extra color 2 - white

  cave->intermission = (data[0x19] != 0);

  // if it is intermission but not scrollable
  if (cave->intermission && !data[0x1c])
  {
    cave->x2 = 19;
    cave->y2 = 11;
  }

  /*
    AMOEBA in crazy dash 8:
    jsr $2500      ; generate true random
    and $94        ; binary and the current "probability"
    cmp #$04       ; compare to 4
    bcs out        ; jump out (do not expand) if carry set, ie. result was less than 4.

    prob values can be like num = 3, 7, 15, 31, 63, ... n lsb bits count.
    0..3 >= 4?  0..7 >= 4?  0..15 >= 4? and similar.
    this way, probability of growing is 4 / (num + 1)
  */

  cave->amoeba_growth_prob      = amoeba_probability(data[0x12]);
  cave->amoeba_fast_growth_prob = amoeba_probability(data[0x13]);

  // expanding wall direction change - 2e horizontal, 2f vertical
  cave->expanding_wall_changed = (data[0x1e] == 0x2f);

  // 2c was a normal switch, 2d a changed one.
  cave->creatures_backwards	= (data[0x1f] == 0x2d);
  cave->biter_delay_frame	= data[0x24];
  cave->magic_wall_stops_amoeba	= (data[0x25] == 0);    // negated!!

  cave->bomb_explosion_effect	= import_table_crdr[data[0x26]];
  cave->explosion_effect	= import_table_crdr[data[0x27]];
  cave->stone_bouncing_effect	= import_table_crdr[data[0x28]];
  cave->diamond_birth_effect	= import_table_crdr[data[0x29]];
  cave->magic_diamond_to	= import_table_crdr[data[0x2a]];

  cave->bladder_converts_by	= import_table_crdr[data[0x2b]];
  cave->diamond_falling_effect	= import_table_crdr[data[0x2c]];
  cave->biter_eat		= import_table_crdr[data[0x2d]];
  cave->slime_eats_1		= import_table_crdr[data[0x2e]];
  cave->slime_converts_1	= import_table_crdr[data[0x2e] + 3];
  cave->slime_eats_2		= import_table_crdr[data[0x2f]];
  cave->slime_converts_2	= import_table_crdr[data[0x2f] + 3];

  cave->diagonal_movements		= ((data[0x34] & 1) != 0);
  cave->gravity_change_time		= data[0x35];
  cave->pneumatic_hammer_frame		= data[0x36];
  cave->hammered_wall_reappear_frame	= data[0x37];
  cave->hammered_walls_reappear		= (data[0x3f] != 0);

  /*
    acid in crazy dream 8:
    jsr $2500    ; true random
    cmp $03a8    ; compare to ratio
    bcs out      ; if it was smaller, forget it for now.

    ie. random <= ratio, then acid grows.
  */

  // 1000000.0, probabilities are stored as int
  cave->acid_spread_ratio = data[0x38] * 1000000.0 / 255.0 + 0.5;
  cave->acid_eats_this = import_table_crdr[data[0x39]];

  switch (data[0x3a] & 3)
  {
    case 0: cave->gravity = GD_MV_UP;    break;
    case 1: cave->gravity = GD_MV_DOWN;  break;
    case 2: cave->gravity = GD_MV_LEFT;  break;
    case 3: cave->gravity = GD_MV_RIGHT; break;
  }

  cave->snap_element = ((data[0x3a] & 4) != 0) ? O_EXPLODE_1 : O_SPACE;

  // we do not know the values for these, so do not import
  // todo do we know?
  //    cave->dirt_looks_like... data[0x3c]
  //    cave->expanding_wall_looks_like... data[0x3b]
  for (i = 0; i < 4; i++)
  {
    cave->random_fill[i] = import_table_crdr[data[0x41 + i]];
    cave->random_fill_probability[i] = data[0x45 + i];
  }

  data += 0x49;
  index = 0;

  while (data[index] != 0xff)
  {
    // for copy&paste; copy&paste are different objects, static = ugly solution :)
    static int cx1, cy1, cw, ch;

    switch (data[index])
    {
      case 1:    // point
      {
	GdElement element = import_table_crdr[data[index + 1]];
	int x1 = data[index + 2];
	int y1 = data[index + 3];

	if (x1 >= cave->w || y1 >= cave->h)
	  Warn("invalid point coordinates %d,%d at byte %d", x1, y1, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_point(GD_OBJECT_LEVEL_ALL,
                                          x1, y1, element));

	index += 4;
	break;
      }

      case 2: // rectangle
      {
	GdElement element = import_table_crdr[data[index + 1]];
	int x1 = data[index + 2];
	int y1 = data[index + 3];
	int x2 = x1 + data[index + 4] - 1;    // width
	int y2 = y1 + data[index + 5] - 1;    // height

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid rectangle coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_rectangle(GD_OBJECT_LEVEL_ALL,
                                              x1, y1, x2, y2, element));

	index += 6;
	break;
      }

      case 3: // fillrect
      {
	GdElement element = import_table_crdr[data[index + 1]];
	int x1 = data[index + 2];
	int y1 = data[index + 3];
	int x2 = x1 + data[index + 4] - 1;
	int y2 = y1 + data[index + 5] - 1;

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid filled rectangle coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	// border and inside of fill is the same element.
	cave->objects =
          list_append(cave->objects,
                      gd_object_new_filled_rectangle(GD_OBJECT_LEVEL_ALL,
                                                     x1, y1, x2, y2, element, element));

	index += 6;
	break;
      }

      case 4: // line
      {
	GdElement element = import_table_crdr[data[index + 1]];
	int x1 = data[index + 2];
	int y1 = data[index + 3];
	int length = data[index + 4];
	int direction = data[index + 5];
	int nx = ((signed)direction - 128) % 40;
	int ny = ((signed)direction - 128) / 40;
	int x2 = x1 + (length - 1) * nx;
	int y2 = y1 + (length - 1) * ny;

	if (element == O_UNKNOWN)
	  Warn("unknown element at %d: %x", index + 1, data[index + 1]);

	// if either is bigger than one, we cannot treat this as a line. create points instead
	if (ABS(nx) > 1 || ABS(ny) > 1)
	{
	  for (i = 0; i < length; i++)
	  {
	    cave->objects =
              list_append(cave->objects,
                          gd_object_new_point(GD_OBJECT_LEVEL_ALL,
                                              x1, y1, element));
	    x1 += nx;
	    y1 += ny;
	  }
	}
	else
	{
	  // this is a normal line, and will be appended. only do the checking here
	  if (x1 >= cave->w ||
	      y1 >= cave->h ||
	      x2 >= cave->w ||
	      y2 >= cave->h)
	    Warn("invalid line coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index - 5);

	  cave->objects =
            list_append(cave->objects,
                        gd_object_new_line(GD_OBJECT_LEVEL_ALL,
                                           x1, y1, x2, y2, element));
	}

	index += 6;
	break;
      }

      case 6: // copy
      {
        // (use static variables defined above for copy & paste!)
	cx1 = data[index + 1];
	cy1 = data[index + 2];
	cw = data[index + 3];
	ch = data[index + 4];

	if (cx1 >= cave->w ||
	    cy1 >= cave->h ||
	    cx1 + cw > cave->w ||
	    cy1 + ch > cave->h)
	  Warn("invalid copy coordinates %d,%d or size %d,%d at byte %d", cx1, cy1, cw, ch, index);

	index += 5;
	break;
      }

      case 7: // paste
      {
	int x1 = cx1;
	int y1 = cy1;

	// original stored width and height, we store the coordinates of the source area
	int x2 = cx1 + cw - 1;
	int y2 = cy1 + ch - 1;
	int dx = data[index + 1];    // new pos
	int dy = data[index + 2];

	if (dx >= cave->w ||
	    dy >= cave->h ||
	    dx + cw > cave->w ||
	    dy + ch > cave->h)
	  Warn("invalid paste coordinates %d,%d at byte %d", dx, dy, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_copy_paste(GD_OBJECT_LEVEL_ALL,
                                               x1, y1, x2, y2, dx, dy, FALSE, FALSE));

	index += 3;
	break;
      }

      case 11: // raster
      {
	GdElement element = import_table_crdr[data[index + 1]];
	int x1 = data[index + 2];
	int y1 = data[index + 3];
	int dx = data[index + 4];
	int dy = data[index + 5];
	int nx = data[index + 6] - 1;
	int ny = data[index + 7] - 1;
	int x2 = x1 + dx * nx;    // calculate rectangle we use
	int y2 = y1 + dy * ny;

	if (dx < 1)
	  dx = 1;
	if (dy < 1)
	  dy = 1;

	if (x1 >= cave->w ||
	    y1 >= cave->h ||
	    x2 >= cave->w ||
	    y2 >= cave->h)
	  Warn("invalid raster coordinates %d,%d %d,%d at byte %d", x1, y1, x2, y2, index);

	cave->objects =
          list_append(cave->objects,
                      gd_object_new_raster(GD_OBJECT_LEVEL_ALL,
                                           x1, y1, x2, y2, dx, dy, element));

	index += 8;
	break;
      }

      default:
      {
	Warn("unknown crdr extension no. %02x at byte %d", data[index], index);

	index += 1;    // skip that byte
	break;
      }
    }
  }

  index++;    // skip $ff

  // crazy dream 7 hack
  byte checksum = 0;

  for (i = 0; i < 0x3b0; i++)
    checksum = checksum ^ data[i];

  if (strEqual(cave->name, "Crazy maze") && checksum == 195)
    cave->skeletons_needed_for_pot = 0;

  return 15 + 0x49 + index;
}

// Import a Crazy Light cave.
static int cave_copy_from_crli(GdCave *cave, const byte *data, int remaining_bytes)
{
  byte uncompressed[1024];
  int datapos, cavepos;
  int x, y;
  int i;
  boolean cavefile;
  const char *versions[] = { "V2.2", "V2.6", "V3.0" };
  enum
  {
    none,
    V2_2,    // XXX whats the difference between 2.2 and 2.6?
    V2_6,
    V3_0
  } version = none;
  GdElement (*import) (const byte data[], int i);
  GdElement (*import_byte) (byte c, int i);

  gd_cave_set_engine_defaults(cave, GD_ENGINE_CRLI);

  // detect if this is a cavefile
  if (data[0] == 0 &&
      data[1] == 0xc4 &&
      data[2] == 'D' &&
      data[3] == 'L' &&
      data[4] == 'P')
  {
    datapos = 5;    // cavefile, skipping 0x00 0xc4 D L P
    cavefile = TRUE;
  }
  else
  {
    // converted from snapshot, skip "selectable" and 14 byte name
    datapos = 15;
    cavefile = FALSE;
  }

  // if we have name, convert it
  if (!cavefile)
    gd_strcpy(cave->name, get_name_from_c64_bin(data + 1));

  // uncompress rle data
  cavepos = 0;

  while (cavepos < 0x3b0)   // <- loop until the uncompressed reaches its size
  {
    if (datapos >= remaining_bytes)
    {
      Error("truncated crli cave data");

      return -1;
    }

    if (data[datapos] == 0xbf)
    {
      // magic value 0xbf is the escape byte
      if (datapos + 2 >= remaining_bytes)
      {
	Error("truncated crli cave data");

	return -1;
      }

      if (data[datapos + 2] + datapos >= sizeof(uncompressed))
      {
	// we would run out of buffer, this must be some error
	Error("invalid crli cave data - RLE length value is too big");

	return -1;
      }

      // 0xbf, number, byte to dup
      for (i = 0; i < data[datapos + 2]; i++)
	uncompressed[cavepos++] = data[datapos + 1];

      datapos += 3;
    }
    else
    {
      uncompressed[cavepos++] = data[datapos++];
    }
  }

  // check crli version
  for (i = 0; i < ARRAY_SIZE(versions); i++)
    if (memcmp((char *)uncompressed + 0x3a0, versions[i], 4) == 0)
      version = i + 1;

  // v3.0 has falling wall and box, and no ghost.
  import      = (version >= V3_0 ? crazylight_import      : firstboulder_import);
  import_byte = (version >= V3_0 ? crazylight_import_byte : firstboulder_import_byte);

  if (version == none)
  {
    Warn("unknown crli version %c%c%c%c", uncompressed[0x3a0], uncompressed[0x3a1], uncompressed[0x3a2], uncompressed[0x3a3]);

    import = crazylight_import;
  }

  // process map
  cave->map = gd_cave_map_new(cave, GdElement);

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      int index = y * cave->w + x;

      cave->map[y][x] = import(uncompressed, index);
    }
  }

  // crli has no levels
  for (i = 0; i < 5; i++)
  {
    cave->level_time[i] = (int)uncompressed[0x370] * 100 + uncompressed[0x371] * 10 + uncompressed[0x372];

    // same as gate opening after 0 diamonds
    if (cave->level_time[i] == 0)
      cave->level_time[i] = 1000;

    cave->level_diamonds[i] = (int)uncompressed[0x373] * 100 + uncompressed[0x374] * 10 + uncompressed[0x375];

    // gate opening is checked AFTER adding to diamonds collected, so 0 here is 1000 (!) needed
    if (cave->level_diamonds[i] == 0)
      cave->level_diamonds[i] = 1000;

    cave->level_ckdelay[i] = uncompressed[0x38A];
    cave->level_amoeba_time[i] = (int)uncompressed[0x37C] * 256 + uncompressed[0x37D];

    // 0 immediately underflowed to 999, so we use 999. example: sendydash 3, cave 02.
    if (cave->level_amoeba_time[i] == 0)
      cave->level_amoeba_time[i] = 999;

    cave->level_magic_wall_time[i] = (int)uncompressed[0x37E] * 256 + uncompressed[0x37F];
    cave->level_slime_permeability_c64[i] = uncompressed[0x38B];
    cave->level_bonus_time[i] = uncompressed[0x392];
    cave->level_penalty_time[i] = uncompressed[0x393];
    cave->level_amoeba_threshold[i] = 256 * (int)uncompressed[0x390] + uncompressed[0x391];

    // set these to the same value on each level, so the engine does
    // not think that levels are different
    cave->level_timevalue[i] = 1;
    cave->level_rand[i] = 0;
    cave->level_speed[i] = cave->level_ckdelay[i] * 20;
    if (cave->level_speed[i] < 80)
      cave->level_speed[i] = 80;
  }

  cave->extra_diamond_value = (int)uncompressed[0x376] * 100 + uncompressed[0x377] * 10 + uncompressed[0x378];
  cave->diamond_value = (int)uncompressed[0x379] * 100 + uncompressed[0x37A] * 10 + uncompressed[0x37B];

  if (uncompressed[0x380])
    cave->creatures_direction_auto_change_time = uncompressed[0x381];

  cave->colorb = gd_c64_color(uncompressed[0x384] & 0xf);    // border
  cave->color0 = gd_c64_color(uncompressed[0x385] & 0xf);
  cave->color1 = gd_c64_color(uncompressed[0x386] & 0xf);
  cave->color2 = gd_c64_color(uncompressed[0x387] & 0xf);
  cave->color3 = gd_c64_color(uncompressed[0x388] & 0x7);    // lower 3 bits only!
  cave->color4 = cave->color3;
  cave->color5 = cave->color1;
  cave->color6 = gd_c64_color(12);                           // extra color 1 - gray
  cave->color7 = gd_c64_color(1);                            // extra color 2 - white

  cave->intermission = (uncompressed[0x389] != 0);

  // if it is intermission but not scrollable
  if (cave->intermission && !uncompressed[0x38c])
  {
    cave->x2 = 19;
    cave->y2 = 11;
  }

  /*
    AMOEBA in crazy dash 8:
    jsr $2500       ; generate true random
    and $94         ; binary and the current "probability"
    cmp #$04        ; compare to 4
    bcs out         ; jump out (do not expand) if carry set, ie. result was less than 4.

    prob values can be like num = 3, 7, 15, 31, 63, ... n lsb bits count.
    0..3 >= 4?  0..7 >= 4?  0..15 >= 4? and similar.
    this way, probability of growing is 4 / (num + 1)
  */

  // probabilities store * 1000000
  cave->amoeba_growth_prob      = amoeba_probability(data[0x382]);
  cave->amoeba_fast_growth_prob = amoeba_probability(data[0x383]);

  // 2c was a normal switch, 2d a changed one.
  cave->creatures_backwards = (uncompressed[0x38f] == 0x2d);
  cave->magic_wall_sound    = (uncompressed[0x38d] == 0xf1);

  // 2e horizontal, 2f vertical. we implement this by changing them
  if (uncompressed[0x38e] == 0x2f)
    for (y = 0; y < cave->h; y++)
      for (x = 0; x < cave->w; x++)
	if (cave->map[y][x] == O_H_EXPANDING_WALL)
	  cave->map[y][x] = O_V_EXPANDING_WALL;

  cave->biter_delay_frame	= uncompressed[0x394];
  cave->magic_wall_stops_amoeba	= (uncompressed[0x395] == 0);    // negated!!
  cave->bomb_explosion_effect	= import(uncompressed, 0x396);
  cave->explosion_effect	= import(uncompressed, 0x397);
  cave->stone_bouncing_effect	= import(uncompressed, 0x398);
  cave->diamond_birth_effect	= import(uncompressed, 0x399);
  cave->magic_diamond_to	= import(uncompressed, 0x39a);

  cave->bladder_converts_by	= import(uncompressed, 0x39b);
  cave->diamond_falling_effect	= import(uncompressed, 0x39c);
  cave->biter_eat		= import(uncompressed, 0x39d);
  cave->slime_eats_1		= import(uncompressed, 0x39e);
  cave->slime_converts_1	= import_byte(uncompressed[0x39e] + 3, 0x39e);
  cave->slime_eats_2		= import(uncompressed, 0x39f);
  cave->slime_converts_2	= import_byte(uncompressed[0x39f] + 3, 0x39f);

  // v3.0 has some new properties.
  if (version >= V3_0)
  {
    cave->diagonal_movements	 = (uncompressed[0x3a4] != 0);
    cave->amoeba_too_big_effect	 = import(uncompressed, 0x3a6);
    cave->amoeba_enclosed_effect = import(uncompressed, 0x3a7);

    /*
      acid in crazy dream 8:
      jsr $2500    ; true random
      cmp $03a8    ; compare to ratio
      bcs out      ; if it was smaller, forget it for now.

      ie. random <= ratio, then acid grows.
    */

    // * 1000000.0, probabilities are stored as int
    cave->acid_spread_ratio		= uncompressed[0x3a8] * 1000000.0 / 255.0 + 0.5;
    cave->acid_eats_this		= import(uncompressed, 0x3a9);
    cave->expanding_wall_looks_like	= import(uncompressed, 0x3ab);
    cave->dirt_looks_like		= import(uncompressed, 0x3ac);
  }
  else
  {
    // version is <= 3.0, so this is a 1stb cave.
    // the only parameters, for which this matters, are these:
    if (uncompressed[0x380] != 0)
      cave->creatures_direction_auto_change_time = uncompressed[0x381];
    else
      cave->diagonal_movements = (uncompressed[0x381] != 0);
  }

  if (cavefile)
    cave->selectable = !cave->intermission;     // best we can do
  else
    cave->selectable = !data[0];                // given by converter

  return datapos;
}

// Deluxe Caves 3 hacks
static void deluxe_caves_3_add_specials(GdCave *cave, const int cavenum)
{
  cave->snap_element = O_EXPLODE_1;
  cave->diagonal_movements = TRUE;

  switch (cavenum)
  {
    case 6:     // cave f
      cave->stone_bouncing_effect = O_BUTTER_1;
      cave->diamond_falling_effect = O_EXPLODE_3;
      break;

    case 7:     // cave g
      Warn("effects not supported");
      break;

    case 13:    // cave l
      Warn("effects not working perfectly");
      cave->stone_bouncing_effect = O_FIREFLY_1;
      break;

    case 18:
      cave->diamond_bouncing_effect = O_STONE;
      break;

    default:
      break;
  }
}

// Crazy Dream 1 hacks
static void crazy_dream_1_add_specials(GdCave *cave)
{
  cave->diagonal_movements = TRUE;
}

// Crazy Dream 7 hacks
static void crazy_dream_7_add_specials(GdCave *cave)
{
  if (strEqual(cave->name, "Crazy maze"))
    cave->skeletons_needed_for_pot = 0;
}

// This function adds some hardcoded elements to a Crazy Dream 9 cave.
// Crazy Dream 9 had some caves and random fills, which were not encoded in the cave data.
static void crazy_dream_9_add_specials(GdCave *cave)
{
  // check cave names, which are hardcoded here (no additional checksum used anymore)

  if (strEqual(cave->name, "Rockfall"))
  {
    GdElement rand[4] = { O_DIAMOND, O_STONE, O_ACID, O_DIRT };
    int prob[4] = { 37, 32, 2, 0 };
    int seeds[5] = { -1, -1, -1, -1, -1 };

    cave->objects =
      list_append(cave->objects,
		  gd_object_new_random_fill(GD_OBJECT_LEVEL_ALL, 0, 0, 39, 21, seeds,
					    O_DIRT, rand, prob, O_BLADDER_SPENDER, FALSE));
  }

  if (strEqual(cave->name, "Roll dice now!"))
  {
    GdElement rand[4] = { O_STONE, O_BUTTER_3, O_DIRT, O_DIRT };
    int prob[4] = { 0x18, 0x08, 0, 0 };
    int seeds[5] = { -1, -1, -1, -1, -1 };

    cave->objects =
      list_append(cave->objects,
		  gd_object_new_random_fill(GD_OBJECT_LEVEL_ALL, 0, 0, 39, 21, seeds,
					    O_DIRT, rand, prob, O_BLADDER_SPENDER, FALSE));
  }

  if (strEqual(cave->name, "Random maze"))
  {
    int seeds[5] = { -1, -1, -1, -1, -1 };
    cave->objects =
      list_append(cave->objects,
		  gd_object_new_maze(GD_OBJECT_LEVEL_ALL, 1, 4, 35, 20, 1, 1,
				     O_NONE, O_DIRT, 50, seeds));
  }

  if (strEqual(cave->name, "Metamorphosis"))
  {
    GdElement rand[4] = { O_STONE, O_DIRT, O_DIRT, O_DIRT };
    int prob[4] = { 0x18, 0, 0, 0 };
    int seeds[5] = { -1, -1, -1, -1, -1 };

    cave->objects =
      list_append(cave->objects,
		  gd_object_new_maze(GD_OBJECT_LEVEL_ALL, 4, 1, 38, 19, 1, 3,
				     O_NONE, O_BLADDER_SPENDER, 50, seeds));

    cave->objects =
      list_append(cave->objects,
		  gd_object_new_random_fill(GD_OBJECT_LEVEL_ALL, 4, 1, 38, 19, seeds,
					    O_DIRT, rand, prob, O_BLADDER_SPENDER, FALSE));

    cave->creatures_backwards = TRUE;    // for some reason, this level worked like that
  }

  if (strEqual(cave->name, "All the way"))
  {
    int seeds[5] = { -1, -1, -1, -1, -1 };

    cave->objects =
      list_append(cave->objects,
		  gd_object_new_maze_unicursal(GD_OBJECT_LEVEL_ALL, 1, 1, 35, 19, 1, 1,
					       O_BRICK, O_PRE_DIA_1, 50, seeds));

    // a point which "breaks" the unicursal maze, making it one very long path
    cave->objects =
      list_append(cave->objects, gd_object_new_point(GD_OBJECT_LEVEL_ALL, 35, 18, O_BRICK));
  }
}

// Masters Boulder hacks
static void masters_boulder_add_hack(GdCave *cave, const int cavenum)
{
  int i;

  switch (cavenum)
  {
    case 1:     // cave b
      for (i = 0; i < 5; i++)
	cave->level_hatching_delay_time[i] = 3;  // secs
      break;

    default:
      break;
  }
}

GdCavefileFormat gd_caveset_imported_get_format(const byte *buf)
{
  const char *s_bd1       = "GDashBD1";
  const char *s_bd1_atari = "GDashB1A";
  const char *s_dc1       = "GDashDC1";
  const char *s_bd2       = "GDashBD2";
  const char *s_bd2_atari = "GDashB2A";
  const char *s_plc       = "GDashPLC";
  const char *s_plc_atari = "GDashPCA";
  const char *s_dlb       = "GDashDLB";
  const char *s_crl       = "GDashCRL";
  const char *s_cd7       = "GDashCD7";
  const char *s_cd9       = "GDashCD9";
  const char *s_1st       = "GDash1ST";

  if (memcmp((char *)buf, s_bd1, strlen(s_bd1)) == 0)
    return GD_FORMAT_BD1;
  if (memcmp((char *)buf, s_bd1_atari, strlen(s_bd1_atari)) == 0)
    return GD_FORMAT_BD1_ATARI;
  if (memcmp((char *)buf, s_dc1, strlen(s_dc1)) == 0)
    return GD_FORMAT_DC1;
  if (memcmp((char *)buf, s_bd2, strlen(s_bd2)) == 0)
    return GD_FORMAT_BD2;
  if (memcmp((char *)buf, s_bd2_atari, strlen(s_bd2_atari)) == 0)
    return GD_FORMAT_BD2_ATARI;
  if (memcmp((char *)buf, s_plc, strlen(s_plc)) == 0)
    return GD_FORMAT_PLC;
  if (memcmp((char *)buf, s_plc_atari, strlen(s_plc_atari)) == 0)
    return GD_FORMAT_PLC_ATARI;
  if (memcmp((char *)buf, s_dlb, strlen(s_dlb)) == 0)
    return GD_FORMAT_DLB;
  if (memcmp((char *)buf, s_crl, strlen(s_crl)) == 0)
    return GD_FORMAT_CRLI;
  if (memcmp((char *)buf, s_cd7, strlen(s_cd7)) == 0)
    return GD_FORMAT_CRDR_7;
  if (memcmp((char *)buf, s_cd9, strlen(s_cd9)) == 0)
    return GD_FORMAT_CRDR_9;
  if (memcmp((char *)buf, s_1st, strlen(s_1st)) == 0)
    return GD_FORMAT_FIRSTB;

  return GD_FORMAT_UNKNOWN;
}


// ----------------------------------------------------------------------------
//  Load caveset from memory buffer.
//  Loads the caveset from a memory buffer.
//  returns: List * of caves.
// ----------------------------------------------------------------------------

List *gd_caveset_import_from_buffer (const byte *buf, size_t length)
{
  boolean numbering;
  int intermissionnum, num;
  int cavelength;
  List *caveset = NULL, *iter;

  if (length != -1 && length < 12)
  {
    Warn("buffer too short to be a GDash datafile");

    return NULL;
  }

  unsigned int encodedlength = (unsigned int)(*((unsigned int *)(buf + 8)));

  if (length != -1 && encodedlength != length - 12)
  {
    Warn("file length and data size mismatch in GDash datafile");

    return NULL;
  }

  GdCavefileFormat format = gd_caveset_imported_get_format(buf);

  if (format == GD_FORMAT_UNKNOWN)
  {
    Warn("buffer does not contain a GDash datafile");

    return NULL;
  }

  // first 12 bytes are cave type. skip that
  buf += 12;

  length = encodedlength;

  // check for hacks.
  GdImportHack hack = GD_HACK_NONE;
  unsigned int cs = checksum(buf, length);

  if (format == GD_FORMAT_PLC    && length == 10240 && cs == 0xbdec)
    hack = GD_HACK_CRDR_1;
  if (format == GD_FORMAT_CRLI   && length == 9895  && cs == 0xbc4e)
    hack = GD_HACK_CRDR_9;
  if (format == GD_FORMAT_BD1    && length == 1634  && cs == 0xf725)
    hack = GD_HACK_DC1;
  if (format == GD_FORMAT_BD1    && length == 1452  && cs == 0xb4a6)
    hack = GD_HACK_DC3;
  if (format == GD_FORMAT_CRDR_7 && length == 3759  && cs == 0x16b5)
    hack = GD_HACK_CRDR_7;
  if (format == GD_FORMAT_BD1    && length == 1241  && cs == 0x926f)
    hack = GD_HACK_MB;

  int bufp = 0;
  int cavenum = 0;

  while (bufp < length)
  {
    // default is to append cave to caveset; list_insert appends when pos = -1
    int insertpos = -1;

    GdCave *newcave = gd_cave_new();

    cavelength = 0;    // to avoid compiler warning

    switch (format)
    {
      case GD_FORMAT_BD1:                // boulder dash 1
      case GD_FORMAT_BD1_ATARI:          // boulder dash 1, atari version
      case GD_FORMAT_BD2:                // boulder dash 2
      case GD_FORMAT_BD2_ATARI:          // boulder dash 2
      case GD_FORMAT_DC1:                // deluxe caves 1
	// these are not in the data so we guess
	newcave->selectable   = (cavenum < 16 && cavenum % 4 == 0);
	newcave->intermission = (cavenum > 15);

	// no name, so we make up one
	if (newcave->intermission)
	  snprintf(newcave->name, sizeof(newcave->name), _("Intermission %d"), cavenum - 15);
	else
	  snprintf(newcave->name, sizeof(newcave->name), _("Cave %c"), 'A' + cavenum);

	switch (format)
	{
	  case GD_FORMAT_BD1:
	  case GD_FORMAT_BD1_ATARI:
	  case GD_FORMAT_DC1:
	    cavelength = cave_copy_from_bd1(newcave, buf + bufp, length - bufp, format, hack);
	    break;

	  case GD_FORMAT_BD2:
	  case GD_FORMAT_BD2_ATARI:
	    cavelength = cave_copy_from_bd2(newcave, buf + bufp, length - bufp, format);
	    break;

	  default:
	    break;
	};

	// original bd1 had level order ABCDEFGH... and then the last four were the intermissions.
	// those should be inserted between D-E, H-I... caves.
	if (cavenum > 15)
	  insertpos = (cavenum - 15) * 5 - 1;
	break;

      case GD_FORMAT_FIRSTB:
	cavelength = cave_copy_from_1stb(newcave, buf + bufp, length - bufp);

	// every fifth cave (4 + 1 intermission) is selectable.
	newcave->selectable = (cavenum % 5 == 0);
	break;

      case GD_FORMAT_PLC:                // peter liepa construction kit
      case GD_FORMAT_PLC_ATARI:          // peter liepa construction kit, atari version
	cavelength = cave_copy_from_plck(newcave, buf + bufp, length - bufp, format);
	break;

      case GD_FORMAT_DLB:
	// no one's delight boulder dash, something like rle compressed plck caves
	// but there are 20 of them, as if it was a bd1 or bd2 game.
	// also num%5 = 4 is intermission.
	// we have to set intermission flag on our own, as the file did not contain
	// the info explicitly

	newcave->intermission = (cavenum % 5 == 4);
	if (newcave->intermission)
	{
	  // also set visible size
	  newcave->x2 = 19;
	  newcave->y2 = 11;
	}

	newcave->selectable = (cavenum % 5 == 0);    // original selection scheme
	if (newcave->intermission)
	  snprintf(newcave->name, sizeof(newcave->name), _("Intermission %d"), cavenum / 5 + 1);
	else
	  snprintf(newcave->name, sizeof(newcave->name), _("Cave %c"), 'A' + (cavenum % 5 + cavenum / 5 * 4));

	cavelength = cave_copy_from_dlb(newcave, buf + bufp, length - bufp);
	break;

      case GD_FORMAT_CRDR_7:
	cavelength = cave_copy_from_crdr_7(newcave, buf + bufp, length - bufp);
	break;

      case GD_FORMAT_CRDR_9:
      case GD_FORMAT_CRLI:
	cavelength = cave_copy_from_crli(newcave, buf + bufp, length - bufp);
	break;

      case GD_FORMAT_UNKNOWN:
	break;
    }

    if (cavelength != -1)
    {
      caveset = list_insert(caveset, newcave, insertpos);
    }
    else
    {
      gd_cave_free(newcave);

      Error("Aborting cave import.");

      break;
    }

    cavenum++;
    bufp += cavelength;

    // hack: some dlb files contain junk data after 20 caves.
    if (format == GD_FORMAT_DLB && cavenum == 20)
    {
      if (bufp < length)
	Warn("excess data in dlb file, %d bytes", (int)(length - bufp));

      break;
    }
  }

  // try to detect if plc caves are in standard layout.
  // that is, caveset looks like an original, (4 cave, 1 intermission)+
  if (format == GD_FORMAT_PLC)
  {
    // if no selection table stored by any2gdash
    if ((buf[2 + 0x1f0] != buf[2 + 0x1f1] - 1) ||
	(buf[2 + 0x1f0] != 0x19 &&
         buf[2 + 0x1f0] != 0x0e))
    {
      List *iter;
      int n;

      boolean standard = (list_length(caveset) % 5 == 0);   // cave count % 5 != 0 -> nonstandard

      for (n = 0, iter = caveset; iter != NULL; n++, iter = iter->next)
      {
	GdCave *cave = iter->data;

	if ((n % 5 == 4 && !cave->intermission) ||
	    (n % 5 != 4 && cave->intermission))
	  standard = FALSE;    // 4 cave, 1 intermission
      }

      // if test passed, update selectability
      if (standard)
      {
	for (n = 0, iter = caveset; iter != NULL; n++, iter = iter->next)
	{
	  GdCave *cave = iter->data;

	  // update "selectable"
	  cave->selectable = (n % 5 == 0);
	}
      }
    }
  }

  // try to give some names for the caves
  cavenum = 1;
  intermissionnum = 1;
  num = 1;

  // use numbering instead of letters, if following formats or too many caves
  // (as we would run out of letters)
  numbering = (format == GD_FORMAT_PLC || format == GD_FORMAT_CRLI || list_length(caveset) > 26);

  for (iter = caveset; iter != NULL; iter = iter->next)
  {
    GdCave *cave = (GdCave *)iter->data;

    if (!strEqual(cave->name, ""))    // if it already has a name, skip
      continue;

    if (cave->intermission)
    {
      if (numbering)
	snprintf(cave->name, sizeof(cave->name), _("Intermission %02d"), num);
      else
	snprintf(cave->name, sizeof(cave->name), _("Intermission %d"), intermissionnum);
    }
    else
    {
      if (numbering)
	snprintf(cave->name, sizeof(cave->name), _("Cave %02d"), num);
      else
	snprintf(cave->name, sizeof(cave->name), _("Cave %c"), 'A' - 1 + cavenum);
    }

    num++;

    if (cave->intermission)
      intermissionnum++;
    else
      cavenum++;
  }

  // if the user requests, we make all caves selectable. intermissions not.
  if (gd_import_as_all_caves_selectable)
  {
    for (iter = caveset; iter != NULL; iter = iter->next)
    {
      GdCave *cave = (GdCave *)iter->data;

      // make selectable if not an intermission.
      // also selectable, if it was selectable originally, for some reason.
      cave->selectable = (cave->selectable || !cave->intermission);
    }
  }

  // add hacks
  for (cavenum = 0, iter = caveset; iter != NULL; iter = iter->next)
  {
    GdCave *cave = (GdCave *)iter->data;

    if (hack == GD_HACK_CRDR_1)
      crazy_dream_1_add_specials(cave);
    if (hack == GD_HACK_CRDR_7)
      crazy_dream_7_add_specials(cave);
    if (hack == GD_HACK_CRDR_9)
      crazy_dream_9_add_specials(cave);
    if (hack == GD_HACK_DC3)
      deluxe_caves_3_add_specials(cave, cavenum);
    if (hack == GD_HACK_MB)
      masters_boulder_add_hack(cave, cavenum);

    cavenum++;
  }

  return caveset;
}

// to be called at program start.
void gd_c64_import_init_tables(void)
{
}
