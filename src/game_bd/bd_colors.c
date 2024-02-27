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


static char *c64_color_names[] =
{
  "Black",
  "White",
  "Red",
  "Cyan",
  "Purple",
  "Green",
  "Blue",
  "Yellow",
  "Orange",
  "Brown",
  "LightRed",
  "Gray1",
  "Gray2",
  "LightGreen",
  "LightBlue",
  "Gray3",
};

/* return c64 color with index. */
GdColor gd_c64_color(int index)
{
  return (GD_COLOR_TYPE_C64 << 24) + index;
}

/* return atari color with index. */
GdColor gd_atari_color(int index)
{
  return (GD_COLOR_TYPE_ATARI << 24) + index;
}

/* return c64dtv color with index. */
GdColor gd_c64dtv_color(int index)
{
  return (GD_COLOR_TYPE_C64DTV << 24) + index;
}

/* return "unknown color" */
static GdColor unknown_color(void)
{
  return (GD_COLOR_TYPE_UNKNOWN << 24);
}

/* make up GdColor from r,g,b values. */
GdColor gd_color_get_from_rgb(int r, int g, int b)
{
  return (GD_COLOR_TYPE_RGB << 24) + (r << 16) + (g << 8) + b;
}

GdColor gd_color_get_from_string(const char *color)
{
  int i, r, g, b;

  for (i = 0; i < ARRAY_SIZE(c64_color_names); i++)
    if (strEqualCase(color, c64_color_names[i]))
      return gd_c64_color(i);

  /* we do not use sscanf(color, "atari..." as may be lowercase */
  if (strEqualCaseN(color, "Atari", strlen("Atari")))
  {
    const char *b = color + strlen("Atari");
    int c;

    if (sscanf(b, "%02x", &c) == 1)
      return gd_atari_color(c);

    Warn("Unknown Atari color: %s", color);

    return unknown_color();
  }

  /* we do not use sscanf(color, "c64dtv..." as may be lowercase */
  if (strEqualCaseN(color, "C64DTV", strlen("C64DTV")))
  {
    const char *b = color + strlen("C64DTV");
    int c;

    if (sscanf(b, "%02x", &c) == 1)
      return gd_c64dtv_color(c);

    Warn("Unknown C64DTV color: %s", color);

    return unknown_color();
  }

  /* may or may not have a # */
  if (color[0] == '#')
    color++;

  if (sscanf(color, "%02x%02x%02x", &r, &g, &b) != 3)
  {
    i = gd_random_int_range(0, 16);

    Warn("Unkonwn color %s", color);

    return unknown_color();
  }

  return gd_color_get_from_rgb(r, g, b);
}

const char *gd_color_get_string(GdColor color)
{
  static char text[16];

  if (gd_color_is_c64(color))
  {
    return c64_color_names[color & 0xff];
  }

  if (gd_color_is_atari(color))
  {
    sprintf(text, "Atari%02x", color & 0xff);
    return text;
  }

  if (gd_color_is_dtv(color))
  {
    sprintf(text, "C64DTV%02x", color & 0xff);
    return text;
  }

  sprintf(text, "#%02x%02x%02x", (color >> 16) & 255, (color >> 8) & 255, color & 255);
  return text;
}

boolean gd_color_is_c64(GdColor color)
{
  return (color >> 24) == GD_COLOR_TYPE_C64;
}

boolean gd_color_is_atari(GdColor color)
{
    return (color >> 24) == GD_COLOR_TYPE_ATARI;
}

boolean gd_color_is_dtv(GdColor color)
{
    return (color >> 24) == GD_COLOR_TYPE_C64DTV;
}

boolean gd_color_is_unknown(GdColor color)
{
  return (color >> 24) == GD_COLOR_TYPE_UNKNOWN;
}

GdColor gd_gdash_color(int c)
{
  /* these values are taken from the title screen, drawn by cws. */
  /* so menus and everything else will look nice! */
  /* the 16 colors that can be used are the same as on c64. */
  /* "Black", "White", "Red", "Cyan", "Purple", "Green", "Blue", "Yellow", */
  /* "Orange", "Brown", "LightRed", "Gray1", "Gray2", "LightGreen", "LightBlue", "Gray3", */
  /* not in the png: cyan, purple. gray3 is darker in the png. */
  /* 17th color is the player's leg in the png. i not connected it to any c64 */
  /* color, but it is used for theme images for example. */
  const GdColor gdash_colors[] =
  {
    0x000000, 0xffffff, 0xe33939, 0x55aaaa, 0xaa55aa, 0x71aa55, 0x0039ff, 0xffff55,
    0xe37139, 0xaa7139, 0xe09080, 0x555555, 0x717171, 0xc6e38e, 0xaaaaff, 0x8e8e8e,

    0x5555aa,
  };

  return gdash_colors[c];
}
