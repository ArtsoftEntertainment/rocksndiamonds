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

#ifndef BD_COLORS_H
#define BD_COLORS_H


typedef unsigned int GdColor;

/* color internal:
   XXRRGGBB;
   XX is 0 for RGB,
         1 for c64 colors (bb=index)
         3 for c64dtv (bb=index)
         2 for atari colors (bb=index)
*/

typedef enum _coloring_type
{
  GD_COLORING_TYPE_SINGLE	= 0,
  GD_COLORING_TYPE_GRADIENTS	= 1,

  GD_COLORING_TYPE_UNKNOWN      // should be the last one
} GdColoringType;

#define GD_DEFAULT_COLORING_TYPE		GD_COLORING_TYPE_SINGLE

typedef enum _color_type
{
  GD_COLOR_TYPE_RGB	= 0,
  GD_COLOR_TYPE_C64	= 1,
  GD_COLOR_TYPE_C64DTV	= 2,
  GD_COLOR_TYPE_ATARI	= 3,

  GD_COLOR_TYPE_UNKNOWN         // should be the last one
} GdColorType;

#define GD_DEFAULT_COLOR_TYPE			GD_COLOR_TYPE_RGB

// traditional c64 color indexes.
#define GD_COLOR_INDEX_BLACK			0
#define GD_COLOR_INDEX_WHITE			1
#define GD_COLOR_INDEX_RED			2
#define GD_COLOR_INDEX_CYAN			3
#define GD_COLOR_INDEX_PURPLE			4
#define GD_COLOR_INDEX_GREEN			5
#define GD_COLOR_INDEX_BLUE			6
#define GD_COLOR_INDEX_YELLOW			7
#define GD_COLOR_INDEX_ORANGE			8
#define GD_COLOR_INDEX_BROWN			9
#define GD_COLOR_INDEX_LIGHTRED			10
#define GD_COLOR_INDEX_GRAY1			11
#define GD_COLOR_INDEX_GRAY2			12
#define GD_COLOR_INDEX_LIGHTGREEN		13
#define GD_COLOR_INDEX_LIGHTBLUE		14
#define GD_COLOR_INDEX_GRAY3			15

#define GD_C64_COLOR(index)			((GD_COLOR_TYPE_C64 << 24) + (index))

// traditional c64 color values.
#define GD_C64_COLOR_BLACK			GD_C64_COLOR(GD_COLOR_INDEX_BLACK)
#define GD_C64_COLOR_WHITE			GD_C64_COLOR(GD_COLOR_INDEX_WHITE)
#define GD_C64_COLOR_RED			GD_C64_COLOR(GD_COLOR_INDEX_RED)
#define GD_C64_COLOR_CYAN			GD_C64_COLOR(GD_COLOR_INDEX_CYAN)
#define GD_C64_COLOR_PURPLE			GD_C64_COLOR(GD_COLOR_INDEX_PURPLE)
#define GD_C64_COLOR_GREEN			GD_C64_COLOR(GD_COLOR_INDEX_GREEN)
#define GD_C64_COLOR_BLUE			GD_C64_COLOR(GD_COLOR_INDEX_BLUE)
#define GD_C64_COLOR_YELLOW			GD_C64_COLOR(GD_COLOR_INDEX_YELLOW)
#define GD_C64_COLOR_ORANGE			GD_C64_COLOR(GD_COLOR_INDEX_ORANGE)
#define GD_C64_COLOR_BROWN			GD_C64_COLOR(GD_COLOR_INDEX_BROWN)
#define GD_C64_COLOR_LIGHTRED			GD_C64_COLOR(GD_COLOR_INDEX_LIGHTRED)
#define GD_C64_COLOR_GRAY1			GD_C64_COLOR(GD_COLOR_INDEX_GRAY1)
#define GD_C64_COLOR_GRAY2			GD_C64_COLOR(GD_COLOR_INDEX_GRAY2)
#define GD_C64_COLOR_LIGHTGREEN			GD_C64_COLOR(GD_COLOR_INDEX_LIGHTGREEN)
#define GD_C64_COLOR_LIGHTBLUE			GD_C64_COLOR(GD_COLOR_INDEX_LIGHTBLUE)
#define GD_C64_COLOR_GRAY3			GD_C64_COLOR(GD_COLOR_INDEX_GRAY3)

// palette numbers must match pointer array positions in source file
#define GD_PALETTE_C64_VICE_NEW			0
#define GD_PALETTE_C64_VICE_OLD			1
#define GD_PALETTE_C64_VIDE_DEFAULT		2
#define GD_PALETTE_C64_C64HQ			3
#define GD_PALETTE_C64_C64S			4
#define GD_PALETTE_C64_CCS64			5
#define GD_PALETTE_C64_FRODO			6
#define GD_PALETTE_C64_GODOT			7
#define GD_PALETTE_C64_PC64			8
#define GD_PALETTE_C64_RTADASH			9

#define GD_DEFAULT_PALETTE_C64			GD_PALETTE_C64_VICE_NEW

// palette numbers must match pointer array positions in source file
#define GD_PALETTE_C64DTV_SPIFF			0
#define GD_PALETTE_C64DTV_MURRAY		1

#define GD_DEFAULT_PALETTE_C64DTV		GD_PALETTE_C64DTV_SPIFF

// palette numbers must match pointer array positions in source file
#define GD_PALETTE_ATARI_BUILTIN		0
#define GD_PALETTE_ATARI_BUILTIN_CONTRAST	1
#define GD_PALETTE_ATARI_DEFAULT		2
#define GD_PALETTE_ATARI_JAKUB			3
#define GD_PALETTE_ATARI_JAKUB_CONTRAST		4
#define GD_PALETTE_ATARI_REAL			5
#define GD_PALETTE_ATARI_REAL_CONTRAST		6
#define GD_PALETTE_ATARI_XFORMER		7

#define GD_DEFAULT_PALETTE_ATARI		GD_PALETTE_ATARI_BUILTIN


// color
GdColor gd_c64_color(int index);
GdColor gd_atari_color(int index);
GdColor gd_c64dtv_color(int index);

GdColor gd_atari_color_huesat(int hue, int sat);
GdColor gd_c64dtv_color_huesat(int hue, int sat);

unsigned int gd_color_get_r(GdColor color);
unsigned int gd_color_get_g(GdColor color);
unsigned int gd_color_get_b(GdColor color);

GdColor gd_color_get_rgb(GdColor color);
GdColor gd_color_get_from_rgb(int r, int g, int b);
GdColor gd_color_get_from_hsv(double h, double s, double v);
GdColor gd_color_get_from_string(const char *color);
const char *gd_color_get_string(GdColor color);

boolean gd_color_is_c64(GdColor color);
boolean gd_color_is_atari(GdColor color);
boolean gd_color_is_dtv(GdColor color);
boolean gd_color_is_unknown(GdColor color);

#endif	// BD_COLORS_H
