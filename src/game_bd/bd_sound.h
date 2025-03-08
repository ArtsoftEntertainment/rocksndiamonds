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

#ifndef BD_SOUND_H
#define BD_SOUND_H

#include "bd_cave.h"


// init sound. allows setting buffer size (for replays saving), 0 for default
void gd_sound_init(void);
void gd_sound_off(void);
void gd_sound_play_cave(GdCave *cave);
void gd_sound_play_bonus_life(GdCave *cave);
void gd_sound_play(GdCave *cave, GdSound sound, GdElement element, int x, int y);
void gd_sound_stop(GdCave *cave, GdSound sound, GdElement element);

#endif	// BD_SOUND_H
