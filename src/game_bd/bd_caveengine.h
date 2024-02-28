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

#ifndef BD_CAVEENGINE_H
#define BD_CAVEENGINE_H

#include "bd_cave.h"


// the game itself
GdDirection gd_direction_from_keypress(boolean up, boolean down, boolean left, boolean right);
void gd_cave_iterate(GdCave *cave, GdDirection player_move, boolean player_fire, boolean suicide);
void set_initial_cave_speed(GdCave *cave);
void gd_cave_set_seconds_sound(GdCave *cave);
void gd_cave_clear_sounds(GdCave *cave);

#endif	// BD_CAVEENGINE_H
