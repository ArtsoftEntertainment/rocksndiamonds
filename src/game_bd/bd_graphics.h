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

#ifndef BD_GRAPHICS_H
#define BD_GRAPHICS_H

#include "bd_cave.h"
#include "bd_gameplay.h"


extern Bitmap *gd_screen_bitmap;

typedef unsigned int GdColor;

void set_cell_size(int s);
void set_play_area(int w, int h);

int get_play_area_w(void);
int get_play_area_h(void);

void gd_init_play_area(void);

boolean gd_bitmap_has_c64_colors(Bitmap *bitmap);
void gd_prepare_tile_bitmap(GdCave *cave, Bitmap *bitmap, int scale_down_factor);
void gd_set_tile_bitmap_reference(Bitmap *bitmap);
Bitmap *gd_get_tile_bitmap(Bitmap *bitmap);
Bitmap *gd_get_colored_bitmap_from_template(Bitmap *template_bitmap);

int gd_drawcave(Bitmap *dest, GdGame *gameplay, boolean);
boolean gd_scroll(GdGame *gameplay, boolean exact_scroll, boolean immediate);
void gd_scroll_to_origin(void);
int get_scroll_x(void);
int get_scroll_y(void);

Bitmap **gd_get_title_screen_bitmaps(void);

#endif	// BD_GRAPHICS_H
