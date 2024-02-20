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

#include <glib.h>
#include <glib/gi18n.h>

#include "main_bd.h"


GdObjectLevels gd_levels_mask[] =
{
  GD_OBJECT_LEVEL1,
  GD_OBJECT_LEVEL2,
  GD_OBJECT_LEVEL3,
  GD_OBJECT_LEVEL4,
  GD_OBJECT_LEVEL5
};

/* create an INDIVIDUAL POINT CAVE OBJECT */
GdObject *gd_object_new_point(GdObjectLevels levels, int x, int y, GdElement elem)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_POINT;
  newobj->x1 = x;
  newobj->y1 = y;
  newobj->element = elem;

  return newobj;
}

/* create a LINE OBJECT */
GdObject *gd_object_new_line(GdObjectLevels levels, int x1, int y1, int x2, int y2,
			     GdElement elem)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_LINE;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->element = elem;

  return newobj;
}

/* create a RECTANGLE OBJECT */
GdObject *gd_object_new_rectangle(GdObjectLevels levels, int x1, int y1, int x2, int y2,
				  GdElement elem)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_RECTANGLE;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->element = elem;

  return newobj;
}

/* create a RECTANGLE OBJECT */
GdObject *gd_object_new_filled_rectangle(GdObjectLevels levels, int x1, int y1, int x2, int y2,
					 GdElement elem, GdElement fill_elem)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_FILLED_RECTANGLE;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->element = elem;
  newobj->fill_element = fill_elem;

  return newobj;
}

/* create a raster object */
GdObject *gd_object_new_raster(GdObjectLevels levels, int x1, int y1, int x2, int y2,
			       int dx, int dy, GdElement elem)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_RASTER;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->dx = dx;
  newobj->dy = dy;
  newobj->element = elem;

  return newobj;
}

/* create a raster object */
GdObject *gd_object_new_join(GdObjectLevels levels, int dx, int dy,
			     GdElement search, GdElement replace)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_JOIN;
  newobj->dx = dx;
  newobj->dy = dy;
  newobj->element = search;
  newobj->fill_element = replace;

  return newobj;
}

/* create a new boundary fill object */
GdObject *gd_object_new_floodfill_border(GdObjectLevels levels, int x1, int y1,
					 GdElement fill, GdElement border)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_FLOODFILL_BORDER;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->element = border;
  newobj->fill_element = fill;

  return newobj;
}

GdObject *gd_object_new_floodfill_replace(GdObjectLevels levels, int x1, int y1,
					  GdElement fill, GdElement to_replace)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_FLOODFILL_REPLACE;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->element = to_replace;
  newobj->fill_element = fill;

  return newobj;
}

GdObject *gd_object_new_maze(GdObjectLevels levels, int x1, int y1, int x2, int y2,
			     int wall_w, int path_w, GdElement wall_e, GdElement path_e,
			     int horiz_percent, const gint32 seed[5])
{
  int i;
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_MAZE;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->dx = wall_w;
  newobj->dy = path_w;
  newobj->element = wall_e;
  newobj->fill_element = path_e;
  newobj->horiz = horiz_percent;

  for (i = 0; i < 5; ++i)
    newobj->seed[i] = seed[i];

  return newobj;
}

GdObject *gd_object_new_maze_unicursal(GdObjectLevels levels, int x1, int y1, int x2, int y2,
				       int wall_w, int path_w, GdElement wall_e, GdElement path_e,
				       int horiz_percent, const gint32 seed[5])
{
  int i;
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_MAZE_UNICURSAL;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->dx = wall_w;
  newobj->dy = path_w;
  newobj->element = wall_e;
  newobj->fill_element = path_e;
  newobj->horiz = horiz_percent;

  for (i = 0; i < 5; ++i)
    newobj->seed[i] = seed[i];

  return newobj;
}

GdObject *gd_object_new_maze_braid(GdObjectLevels levels, int x1, int y1, int x2, int y2,
				   int wall_w, int path_w, GdElement wall_e, GdElement path_e,
				   int horiz_percent, const gint32 seed[5])
{
  int i;
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_MAZE_BRAID;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->dx = wall_w;
  newobj->dy = path_w;
  newobj->element = wall_e;
  newobj->fill_element = path_e;
  newobj->horiz = horiz_percent;

  for (i = 0; i < 5; ++i)
    newobj->seed[i] = seed[i];

  return newobj;
}

GdObject *gd_object_new_random_fill(GdObjectLevels levels, int x1, int y1, int x2, int y2,
				    const gint32 seed[5], GdElement initial,
				    const GdElement random[4], const gint32 prob[4],
				    GdElement replace_only, boolean c64)
{
  int i;
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_RANDOM_FILL;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->fill_element = initial;

  for (i = 0; i < 5; ++i)
    newobj->seed[i] = seed[i];

  for (i = 0; i < 4; ++i)
  {
    newobj->random_fill[i] = random[i];
    newobj->random_fill_probability[i] = prob[i];
  }

  newobj->element = replace_only;
  newobj->c64_random = c64;

  return newobj;
}

GdObject *gd_object_new_copy_paste(GdObjectLevels levels, int x1, int y1, int x2, int y2,
				   int dx, int dy, boolean mirror, boolean flip)
{
  GdObject *newobj = checked_calloc(sizeof(GdObject));

  newobj->levels = levels;
  newobj->type = GD_COPY_PASTE;
  newobj->x1 = x1;
  newobj->y1 = y1;
  newobj->x2 = x2;
  newobj->y2 = y2;
  newobj->dx = dx;
  newobj->dy = dy;
  newobj->mirror = mirror;
  newobj->flip = flip;

  return newobj;
}

/* create new object from bdcff description.
   return new object if ok; return null if failed.
 */
GdObject *gd_object_new_from_string(char *str)
{
  char *equalsign;
  char *name, *param;
  GdObject object;
  char elem0[100], elem1[100];

  equalsign = strchr(str, '=');
  if (!equalsign)
    return NULL;

  /* split string by replacing the equal sign with zero */
  *equalsign = '\0';
  name = str;
  param = equalsign + 1;

  /* INDIVIDUAL POINT CAVE OBJECT */
  if (strcasecmp(name, "Point") == 0)
  {
    object.type = GD_POINT;
    if (sscanf(param, "%d %d %s", &object.x1, &object.y1, elem0) == 3)
    {
      object.element = gd_get_element_from_string(elem0);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* LINE OBJECT */
  if (strcasecmp(name, "Line") == 0)
  {
    object.type = GD_LINE;
    if (sscanf(param, "%d %d %d %d %s", &object.x1, &object.y1, &object.x2, &object.y2, elem0) == 5)
    {
      object.element = gd_get_element_from_string(elem0);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* RECTANGLE OBJECT */
  if (strcasecmp(name, "Rectangle") == 0)
  {
    if (sscanf(param, "%d %d %d %d %s", &object.x1, &object.y1, &object.x2, &object.y2, elem0) == 5)
    {
      object.type = GD_RECTANGLE;
      object.element = gd_get_element_from_string (elem0);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* FILLED RECTANGLE OBJECT */
  if (strcasecmp(name, "FillRect") == 0)
  {
    int paramcount;

    paramcount = sscanf(param, "%d %d %d %d %s %s", &object.x1, &object.y1, &object.x2, &object.y2, elem0, elem1);
    object.type = GD_FILLED_RECTANGLE;

    if (paramcount == 6)
    {
      object.element = gd_get_element_from_string (elem0);
      object.fill_element = gd_get_element_from_string (elem1);

      return g_memdup(&object, sizeof (GdObject));
    }

    if (paramcount == 5)
    {
      object.element = object.fill_element = gd_get_element_from_string (elem0);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* RASTER */
  if (strcasecmp(name, "Raster") == 0)
  {
    int nx, ny;

    if (sscanf(param, "%d %d %d %d %d %d %s", &object.x1, &object.y1, &nx, &ny, &object.dx, &object.dy, elem0) == 7)
    {
      nx--;
      ny--;
      object.x2 = object.x1 + nx * object.dx;
      object.y2 = object.y1 + ny * object.dy;
      object.type = GD_RASTER;
      object.element = gd_get_element_from_string (elem0);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* JOIN */
  if (strcasecmp(name, "Join") == 0 ||
      strcasecmp(name, "Add") == 0)
  {
    if (sscanf(param, "%d %d %s %s", &object.dx, &object.dy, elem0, elem1) == 4)
    {
      object.type = GD_JOIN;
      object.element = gd_get_element_from_string (elem0);
      object.fill_element = gd_get_element_from_string (elem1);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* FILL TO BORDER OBJECT */
  if (strcasecmp(name, "BoundaryFill") == 0)
  {
    if (sscanf(param, "%d %d %s %s", &object.x1, &object.y1, elem0, elem1) == 4)
    {
      object.type = GD_FLOODFILL_BORDER;
      object.fill_element = gd_get_element_from_string (elem0);
      object.element = gd_get_element_from_string (elem1);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* REPLACE FILL OBJECT */
  if (strcasecmp(name, "FloodFill") == 0)
  {
    if (sscanf(param, "%d %d %s %s", &object.x1, &object.y1, elem0, elem1) == 4)
    {
      object.type = GD_FLOODFILL_REPLACE;
      object.fill_element = gd_get_element_from_string (elem0);
      object.element = gd_get_element_from_string (elem1);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* MAZE OBJECT */
  /* MAZE UNICURSAL OBJECT */
  /* BRAID MAZE OBJECT */
  if (strcasecmp(name, "Maze") == 0)
  {
    char type[100] = "perfect";

    if (sscanf(param, "%d %d %d %d %d %d %d %d %d %d %d %d %s %s %s", &object.x1, &object.y1, &object.x2, &object.y2, &object.dx, &object.dy, &object.horiz, &object.seed[0], &object.seed[1], &object.seed[2], &object.seed[3], &object.seed[4], elem0, elem1, type) >= 14)
    {
      if (strcasecmp(type, "unicursal") == 0)
	object.type = GD_MAZE_UNICURSAL;
      else if (strcasecmp(type, "perfect") == 0)
	object.type = GD_MAZE;
      else if (strcasecmp(type, "braid") == 0)
	object.type = GD_MAZE_BRAID;
      else
      {
	Warn("unknown maze type: %s, defaulting to perfect", type);
	object.type = GD_MAZE;
      }

      object.element = gd_get_element_from_string (elem0);
      object.fill_element = gd_get_element_from_string (elem1);

      return g_memdup(&object, sizeof (GdObject));
    }

    return NULL;
  }

  /* RANDOM FILL OBJECT */
  if (strcasecmp(name, "RandomFill") == 0 ||
      strcasecmp(name, "RandomFillC64") == 0)
  {
    static char **words = NULL;
    int l, i;

    object.type = GD_RANDOM_FILL;
    if (strcasecmp(name, "RandomFillC64") == 0)
      /* totally the same, but uses c64 random generator */
      object.c64_random = TRUE;
    else
      object.c64_random = FALSE;

    if (sscanf(param, "%d %d %d %d", &object.x1, &object.y1, &object.x2, &object.y2) != 4)
      return NULL;

    if (words)
      freeStringArray(words);

    words = getSplitStringArray(param, " ", -1);
    l = getStringArrayLength(words);

    if (l < 10 || l > 19)
      return NULL;

    for (i = 0; i < 5; i++)
      if (sscanf(words[4 + i], "%d", &object.seed[i]) != 1)
	return NULL;

    object.fill_element = gd_get_element_from_string(words[9]);

    for (i = 0; i < 4; i++)
    {
      object.random_fill[i] = O_DIRT;
      object.random_fill_probability[i] = 0;
    }

    for (i = 10; i < l - 1; i += 2)
    {
      object.random_fill[(i - 10) / 2] = gd_get_element_from_string(words[i]);
      if (sscanf(words[i + 1], "%d", &object.random_fill_probability[(i - 10) / 2]) == 0)
	return NULL;
    }

    object.element = O_NONE;

    if (l > 10 && l % 2 == 1)
      object.element = gd_get_element_from_string(words[l - 1]);

    return g_memdup(&object, sizeof (GdObject));
  }

  /* COPY PASTE OBJECT */
  if (strcasecmp(name, "CopyPaste") == 0)
  {
    char mirror[100] = "nomirror";
    char flip[100] = "noflip";
    object.type = GD_COPY_PASTE;

    object.flip = object.mirror = FALSE;

    if (sscanf(param, "%d %d %d %d %d %d %s %s", &object.x1, &object.y1, &object.x2, &object.y2, &object.dx, &object.dy, mirror, flip) < 6)
      return NULL;

    /* MIRROR PROPERTY */
    if (strcasecmp(mirror, "mirror") == 0)
      object.mirror = TRUE;
    else if (strcasecmp(mirror, "nomirror") == 0)
      object.mirror = FALSE;
    else
      Warn("invalid setting for copypaste mirror property: %s", mirror);

    /* FLIP PROPERTY */
    if (strcasecmp(flip, "flip") == 0)
      object.flip = TRUE;
    else if (strcasecmp(flip, "noflip") == 0)
      object.flip = FALSE;
    else
      Warn("invalid setting for copypaste flip property: %s", flip);

    return g_memdup(&object, sizeof(GdObject));
  }

  return NULL;
}

/** drawing a line, using bresenham's */
static void draw_line (GdCave *cave, const GdObject *object)
{
  int x, y, x1, y1, x2, y2;
  boolean steep;
  int error, dx, dy, ystep;

  x1 = object->x1;
  y1 = object->y1, x2 = object->x2;
  y2 = object->y2;
  steep = ABS (y2 - y1) > ABS (x2 - x1);

  if (steep)
  {
    x = x1;
    x1 = y1;
    y1 = x;
    x = x2;
    x2 = y2;
    y2 = x;
  }

  if (x1 > x2)
  {
    x = x1;
    x1 = x2;
    x2 = x;
    x = y1;
    y1 = y2;
    y2 = x;
  }

  dx = x2 - x1;
  dy = ABS (y2 - y1);
  y = y1;
  error = 0;
  ystep = (y1 < y2) ? 1 : -1;

  for (x = x1; x <= x2; x++)
  {
    if (steep)
      gd_cave_store_rc (cave, y, x, object->element, object);
    else
      gd_cave_store_rc (cave, x, y, object->element, object);

    error += dy;

    if (error * 2 >= dx)
    {
      y += ystep;
      error -= dx;
    }
  }
}



static void draw_fill_replace_proc(GdCave *cave, int x, int y, const GdObject *object)
{
  /* fill with border so we do not come back */
  gd_cave_store_rc(cave, x, y, object->fill_element, object);

  if (x > 0 && gd_cave_get_rc(cave, x - 1, y) == object->element)
    draw_fill_replace_proc(cave, x - 1, y, object);

  if (y > 0 && gd_cave_get_rc(cave, x, y - 1) == object->element)
    draw_fill_replace_proc(cave, x, y - 1, object);

  if (x < cave->w - 1 && gd_cave_get_rc(cave, x + 1, y) == object->element)
    draw_fill_replace_proc(cave, x + 1, y, object);

  if (y < cave->h - 1 && gd_cave_get_rc(cave, x, y + 1) == object->element)
    draw_fill_replace_proc(cave, x, y + 1, object);
}

static void draw_fill_replace (GdCave *cave, const GdObject *object)
{
  /* check bounds */
  if (object->x1 < 0 ||
      object->y1 < 0 ||
      object->x1 >= cave->w ||
      object->y1 >= cave->h)
    return;

  if (object->element == object->fill_element)
    return;

  /* this procedure fills the area with the object->element. */
  draw_fill_replace_proc(cave, object->x1, object->y1, object);
}

static void draw_fill_border_proc (GdCave *cave, int x, int y, const GdObject *object)
{
  /* fill with border so we do not come back */
  gd_cave_store_rc(cave, x, y, object->element, object);

  if (x > 0 && gd_cave_get_rc(cave, x - 1, y) != object->element)
    draw_fill_border_proc(cave, x - 1, y, object);

  if (y > 0 && gd_cave_get_rc(cave, x, y - 1) != object->element)
    draw_fill_border_proc(cave, x, y - 1, object);

  if (x < cave->w - 1 && gd_cave_get_rc(cave, x + 1, y) != object->element)
    draw_fill_border_proc(cave, x + 1, y, object);

  if (y < cave->h-1 && gd_cave_get_rc(cave, x, y + 1) != object->element)
    draw_fill_border_proc(cave, x, y + 1, object);
}

static void draw_fill_border (GdCave *cave, const GdObject *object)
{
  int x, y;

  /* check bounds */
  if (object->x1 < 0 ||
      object->y1 < 0 ||
      object->x1 >= cave->w ||
      object->y1 >= cave->h)
    return;

  /* this procedure fills the area with the object->element. */
  draw_fill_border_proc(cave, object->x1, object->y1, object);

  /* after the fill, we change all filled cells to the fill_element. */
  /* we find those by looking at the object_order[][] */
  for (y = 0; y < cave->h; y++)
    for (x = 0; x < cave->w; x++)
      if (cave->objects_order[y][x] == object)
	cave->map[y][x] = object->fill_element;
}

/* rectangle, frame only */
static void draw_rectangle(GdCave *cave, const GdObject *object)
{
  int x1, y1, x2, y2, x, y;

  /* reorder coordinates if not drawing from northwest to southeast */
  x1 = object->x1;
  y1 = object->y1, x2 = object->x2;
  y2 = object->y2;

  if (y1 > y2)
  {
    y = y1;
    y1 = y2;
    y2 = y;
  }

  if (x1 > x2)
  {
    x = x1;
    x1 = x2;
    x2 = x;
  }

  for (x = x1; x <= x2; x++)
  {
    gd_cave_store_rc(cave, x, object->y1, object->element, object);
    gd_cave_store_rc(cave, x, object->y2, object->element, object);
  }

  for (y = y1; y <= y2; y++)
  {
    gd_cave_store_rc(cave, object->x1, y, object->element, object);
    gd_cave_store_rc(cave, object->x2, y, object->element, object);
  }
}

/* rectangle, filled one */
static void draw_filled_rectangle(GdCave *cave, const GdObject *object)
{
  int x1, y1, x2, y2, x, y;

  /* reorder coordinates if not drawing from northwest to southeast */
  x1 = object->x1;
  y1 = object->y1, x2 = object->x2;
  y2 = object->y2;

  if (y1 > y2)
  {
    y = y1;
    y1 = y2;
    y2 = y;
  }

  if (x1 > x2)
  {
    x = x1;
    x1 = x2;
    x2 = x;
  }

  for (y = y1; y <= y2; y++)
    for (x = x1; x <= x2; x++)
      gd_cave_store_rc(cave, x, y, (y == object->y1 ||
				    y == object->y2 ||
				    x == object->x1 ||
				    x == object->x2) ? object->element : object->fill_element, object);
}

/* something like ordered fill, increment is dx and dy. */
static void draw_raster(GdCave *cave, const GdObject *object)
{
  int x, y, x1, y1, x2, y2;
  int dx, dy;

  /* reorder coordinates if not drawing from northwest to southeast */
  x1 = object->x1;
  y1 = object->y1;
  x2 = object->x2;
  y2 = object->y2;

  if (y1 > y2)
  {
    y = y1;
    y1 = y2;
    y2 = y;
  }

  if (x1 > x2)
  {
    x = x1;
    x1 = x2;
    x2 = x;
  }

  dx = object->dx;

  if (dx < 1)
    dx = 1;

  dy = object->dy;

  if (dy < 1)
    dy = 1;

  for (y = y1; y <= y2; y += dy)
    for (x = x1; x <= x2; x += dx)
      gd_cave_store_rc(cave, x, y, object->element, object);
}

/* find every object, and put fill_element next to it. relative coordinates dx,dy */
static void draw_join(GdCave *cave, const GdObject *object)
{
  int x, y;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      if (cave->map[y][x] == object->element)
      {
	int nx = x + object->dx;
	int ny = y + object->dy;
	/* this one implements wraparound for joins.
	   it is needed by many caves in profi boulder series */
	while (nx >= cave->w)
	  nx -= cave->w, ny++;

	gd_cave_store_rc(cave, nx, ny, object->fill_element, object);
      }
    }
  }
}

/* create a maze in a boolean **maze. */
/* recursive algorithm. */
static void mazegen(GRand *rand, boolean **maze, int width, int height, int x, int y, int horiz)
{
  int dirmask = 15;

  maze[y][x] = TRUE;
  while (dirmask != 0)
  {
    int dir;

    /* horiz or vert */
    dir = g_rand_int_range(rand, 0, 100) <horiz ? 2 : 0;

    /* if no horizontal movement possible, choose vertical */
    if (dir == 2 && (dirmask & 12) == 0)
      dir = 0;
    else if (dir == 0 && (dirmask&3) == 0)    /* and vice versa */
      dir = 2;

    dir += g_rand_int_range(rand, 0, 2);                /* dir */
    if (dirmask & (1 << dir))
    {
      dirmask &= ~(1 << dir);

      switch (dir)
      {
	case 0:    /* up */
	  if (y >= 2 && !maze[y - 2][x])
	  {
	    maze[y - 1][x] = TRUE;
	    mazegen(rand, maze, width, height, x, y - 2, horiz);
	  }
	  break;

	case 1:    /* down */
	  if (y < height-2 && !maze[y + 2][x]) {
	    maze[y + 1][x] = TRUE;
	    mazegen(rand, maze, width, height, x, y + 2, horiz);
	  }
	  break;

	case 2:    /* left */
	  if (x >= 2 && !maze[y][x - 2]) {
	    maze[y][x - 1] = TRUE;
	    mazegen(rand, maze, width, height, x - 2, y, horiz);
	  }
	  break;

	case 3:    /* right */
	  if (x < width - 2 && !maze[y][x + 2]) {
	    maze[y][x + 1] = TRUE;
	    mazegen(rand, maze, width, height, x + 2, y, horiz);
	  }
	  break;

	default:
	  break;
      }
    }
  }
}

static void braidmaze(GRand *rand, boolean **maze, int w, int h)
{
  int x, y;

  for (y = 0; y < h; y += 2)
  {
    for (x = 0; x < w; x += 2)
    {
      int closed = 0, dirs = 0;
      int closed_dirs[4];

      /* if it is the edge of the map, OR no path carved, then we can't go in that direction. */
      if (x < 1 || !maze[y][x - 1])
      {
	/* closed from this side. */
	closed++;

	/* if not the edge, we might open this wall (carve a path) to remove a dead end */
	if (x > 0)
	  closed_dirs[dirs++] = GD_MV_LEFT;
      }

      /* other 3 directions similar */
      if (y < 1 || !maze[y - 1][x])
      {
	closed++;
	if (y > 0)
	  closed_dirs[dirs++] = GD_MV_UP;
      }

      if (x >= w - 1 || !maze[y][x + 1])
      {
	closed++;
	if (x < w - 1)
	  closed_dirs[dirs++] = GD_MV_RIGHT;
      }

      if (y >= h - 1 || !maze[y + 1][x]) {
	closed++;
	if (y < h - 1)
	  closed_dirs[dirs++] = GD_MV_DOWN;
      }

      /* if closed from 3 sides, then it is a dead end. also check dirs != 0,
	 that might fail for a 1x1 maze :) */
      if (closed == 3 && dirs != 0)
      {
	/* make up a random direction, and open in that direction, so dead end is removed */
	int dir = closed_dirs[g_rand_int_range(rand, 0, dirs)];

	switch (dir)
	{
	  case GD_MV_LEFT:
	    maze[y][x - 1] = TRUE; break;
	  case GD_MV_UP:
	    maze[y - 1][x] = TRUE; break;
	  case GD_MV_RIGHT:
	    maze[y][x + 1] = TRUE; break;
	  case GD_MV_DOWN:
	    maze[y + 1][x] = TRUE; break;
	}
      }
    }
  }
}

static void draw_maze(GdCave *cave, const GdObject *object, int level)
{
  int x, y;
  boolean **map;
  int x1 = object->x1;
  int y1 = object->y1;
  int x2 = object->x2;
  int y2 = object->y2;
  int w, h, path, wall;
  int xk, yk;
  GRand *rand;
  int i,j;

  /* change coordinates if not in correct order */
  if (y1 > y2)
  {
    y = y1;
    y1 = y2;
    y2 = y;
  }

  if (x1 > x2)
  {
    x = x1;
    x1 = x2;
    x2 = x;
  }

  wall = object->dx;
  if (wall < 1)
    wall = 1;

  path = object->dy;
  if (path < 1)
    path = 1;

  /* calculate the width and height of the maze.
     n = number of passages, path = path width, wall = wall width, maze = maze width.
     if given the number of passages, the width of the maze is:

     n * path + (n - 1) * wall = maze
     n * path + n * wall - wall = maze
     n * (path + wall) = maze + wall
     n = (maze + wall) / (path + wall)
  */

  /* number of passages for each side */
  w = (x2 - x1 + 1 + wall) / (path + wall);
  h = (y2 - y1 + 1 + wall) / (path + wall);

  /* and we calculate the size of the internal map */
  if (object->type == GD_MAZE_UNICURSAL)
  {
    /* for unicursal maze, width and height must be mod2 = 0,
       and we will convert to paths & walls later */
    w = w / 2 * 2;
    h = h / 2 * 2;
  }
  else
  {
    /* for normal maze */
    w = 2 * (w - 1) + 1;
    h = 2 * (h - 1) + 1;
  }

  /* twodimensional boolean array to generate map in */
  map = checked_malloc((h) * sizeof(boolean *));
  for (y = 0; y < h; y++)
    map[y] = checked_calloc(w * sizeof(boolean));

  /* start generation, if map is big enough.
     otherwise the application would crash, as the editor places maze objects
     during mouse click & drag that have no sense */
  rand = g_rand_new_with_seed(object->seed[level] == -1 ?
			      g_rand_int(cave->random) : object->seed[level]);

  if (w >= 1 && h >= 1)
    mazegen(rand, map, w, h, 0, 0, object->horiz);

  if (object->type == GD_MAZE_BRAID)
    braidmaze(rand, map, w, h);

  g_rand_free(rand);

  if (w >= 1 && h >= 1 && object->type == GD_MAZE_UNICURSAL)
  {
    boolean **unicursal;

    /* convert to unicursal maze */
    /* original:
        xxx x
          x x
        xxxxx

        unicursal:
        xxxxxxx xxx
        x     x x x
        xxxxx x x x
            x x x x
        xxxxx xxx x
        x         x
        xxxxxxxxxxx
    */

    unicursal = checked_malloc((h * 2 - 1) * sizeof(boolean *));

    for (y = 0; y < h * 2 - 1; y++)
      unicursal[y] = checked_calloc((w * 2 - 1) * sizeof(boolean));

    for (y = 0; y < h; y++)
    {
      for(x = 0; x < w; x++)
      {
	if (map[y][x]) {
	  unicursal[y * 2][x * 2] = TRUE;
	  unicursal[y * 2][x * 2 + 2] = TRUE;
	  unicursal[y * 2 + 2][x * 2] = TRUE;
	  unicursal[y * 2 + 2][x * 2 + 2] = TRUE;

	  if (x < 1      || !map[y][x - 1]) unicursal[y * 2 + 1][x * 2] = TRUE;
	  if (y < 1      || !map[y - 1][x]) unicursal[y * 2][x * 2 + 1] = TRUE;
	  if (x >= w - 1 || !map[y][x + 1]) unicursal[y * 2 + 1][x * 2 + 2] = TRUE;
	  if (y >= h - 1 || !map[y + 1][x]) unicursal[y * 2 + 2][x * 2 + 1] = TRUE;
	}
      }
    }

    /* free original map */
    for (y = 0; y < h; y++)
      free(map[y]);
    free(map);

    /* change to new map - the unicursal maze */
    map = unicursal;
    h = h * 2 - 1;
    w = w * 2 - 1;
  }

  /* copy map to cave with correct elements and size */
  /* now copy the map into the cave. the copying works like this...
     pwpwp
     xxxxx p
     x x   w
     x xxx p
     x     w
     xxxxx p
     columns and rows denoted with "p" are to be drawn with path width,
     the others with wall width. */

  yk = y1;

  for (y = 0; y < h; y++)
  {
    for (i = 0; i < (y % 2 == 0 ? path : wall); i++)
    {
      xk = x1;

      for (x = 0; x < w; x++)
	for (j = 0; j < (x % 2 == 0 ? path : wall); j++)
	  gd_cave_store_rc(cave, xk++, yk, map[y][x] ? object->fill_element : object->element, object);

      /* if width is smaller than requested, fill with wall */
      for(x = xk; x <= x2; x++)
	gd_cave_store_rc(cave, x, yk, object->element, object);

      yk++;
    }
  }

  /* if height is smaller than requested, fill with wall */
  for (y = yk; y <= y2; y++)
    for (x = x1; x <= x2; x++)
      gd_cave_store_rc(cave, x, y, object->element, object);

  /* free map */
  for (y = 0; y < h; y++)
    free(map[y]);
  free(map);
}

static void draw_random_fill(GdCave *cave, const GdObject *object, int level)
{
  int x, y;
  int x1 = object->x1;
  int y1 = object->y1;
  int x2 = object->x2;
  int y2 = object->y2;
  GRand *rand;
  GdC64RandomGenerator c64_rand;
  guint32 seed;

  /* -1 means that it should be different every time played. */
  if (object->seed[level] == -1)
    seed = g_rand_int(cave->random);
  else
    seed = object->seed[level];

  rand = g_rand_new_with_seed(seed);
  /* for c64 random, use the 2*8 lsb. */
  gd_c64_random_set_seed(&c64_rand, seed / 256 % 256, seed % 256);

  /* change coordinates if not in correct order */
  if (y1 > y2)
  {
    y = y1;
    y1 = y2;
    y2 = y;
  }

  if (x1 > x2)
  {
    x = x1;
    x1 = x2;
    x2 = x;
  }

  for (y = y1; y <= y2; y++)
  {
    for (x = x1; x <= x2; x++)
    {
      unsigned int randm;
      GdElement element;

      if (object->c64_random)
	/* use c64 random generator */
	randm = gd_c64_random(&c64_rand);
      else
	/* use the much better glib random generator */
	randm = g_rand_int_range(rand, 0, 256);

      element = object->fill_element;
      if (randm < object->random_fill_probability[0])
	element = object->random_fill[0];
      if (randm < object->random_fill_probability[1])
	element = object->random_fill[1];
      if (randm < object->random_fill_probability[2])
	element = object->random_fill[2];
      if (randm < object->random_fill_probability[3])
	element = object->random_fill[3];

      if (object->element == O_NONE ||
	  gd_cave_get_rc(cave, x, y) == object->element)
	gd_cave_store_rc(cave, x, y, element, object);
    }
  }

  g_rand_free(rand);
}


static void draw_copy_paste(GdCave *cave, const GdObject *object)
{
  int x1 = object->x1, y1 = object->y1, x2 = object->x2, y2 = object->y2;
  int x, y;    /* iterators */
  int w, h;
  GdElement *clipboard;

  /* reorder coordinates if not drawing from northwest to southeast */
  if (x2 < x1)
  {
    x = x2;
    x2 = x1;
    x1 = x;
  }

  if (y2 < y1)
  {
    y = y2;
    y2 = y1;
    y1 = y;
  }

  w = x2 - x1 + 1;
  h = y2 - y1 + 1;

  clipboard = checked_malloc((w * h) * sizeof(GdElement));

  /* copy to "clipboard" */
  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
      clipboard[y * w + x] = gd_cave_get_rc(cave, x + x1, y + y1);

  for (y = 0; y < h; y++)
  {
    int ydest;

    ydest = object->flip ? h - 1 - y : y;

    for (x = 0; x < w; x++)
    {
      int xdest;

      xdest = object->mirror ? w - 1 - x : x;

      /* dx and dy are used here are "paste to" coordinates */
      gd_cave_store_rc(cave, object->dx + xdest, object->dy + ydest,
		       clipboard[y * w + x], object);
    }
  }

  free(clipboard);
}

/* draw the specified game object into cave's data.
   also remember, which cell was set by which cave object. */
void gd_cave_draw_object(GdCave *cave, const GdObject *object, int level)
{
  switch (object->type)
  {
    case GD_POINT:
      /* single point */
      gd_cave_store_rc(cave, object->x1, object->y1, object->element, object);
      break;

    case GD_LINE:
      draw_line(cave, object);
      break;

    case GD_RECTANGLE:
      draw_rectangle(cave, object);
      break;

    case GD_FILLED_RECTANGLE:
      draw_filled_rectangle(cave, object);
      break;

    case GD_RASTER:
      draw_raster(cave, object);
      break;

    case GD_JOIN:
      draw_join(cave, object);
      break;

    case GD_FLOODFILL_BORDER:
      draw_fill_border(cave, object);
      break;

    case GD_FLOODFILL_REPLACE:
      draw_fill_replace(cave, object);
      break;

    case GD_MAZE:
    case GD_MAZE_UNICURSAL:
    case GD_MAZE_BRAID:
      draw_maze(cave, object, level);
      break;

    case GD_RANDOM_FILL:
      draw_random_fill(cave, object, level);
      break;

    case GD_COPY_PASTE:
      draw_copy_paste(cave, object);
      break;

    case NONE:
      break;

    default:
      Error("Unknown object %d", object->type);
      break;
  }
}

/* load cave to play... also can be called rendering the cave elements */
GdCave *gd_cave_new_rendered(const GdCave *data, const int level, const guint32 seed)
{
  GdCave *cave;
  GdElement element;
  int x, y;
  GList *iter;

  /* make a copy */
  cave = gd_cave_new_from_cave(data);
  cave->rendered = level + 1;

  cave->render_seed = seed;
  cave->random = g_rand_new_with_seed(cave->render_seed);

  /* maps needed during drawing and gameplay */
  cave->objects_order = gd_cave_map_new(cave, gpointer);

  cave->time                   = data->level_time[level];
  cave->timevalue              = data->level_timevalue[level];
  cave->diamonds_needed        = data->level_diamonds[level];
  cave->magic_wall_time        = data->level_magic_wall_time[level];
  cave->slime_permeability     = data->level_slime_permeability[level];
  cave->slime_permeability_c64 = data->level_slime_permeability_c64[level];
  cave->time_bonus             = data->level_bonus_time[level];
  cave->time_penalty           = data->level_penalty_time[level];
  cave->amoeba_time            = data->level_amoeba_time[level];
  cave->amoeba_max_count       = data->level_amoeba_threshold[level];
  cave->amoeba_2_time          = data->level_amoeba_2_time[level];
  cave->amoeba_2_max_count     = data->level_amoeba_2_threshold[level];
  cave->hatching_delay_time    = data->level_hatching_delay_time[level];
  cave->hatching_delay_frame   = data->level_hatching_delay_frame[level];

  if (!cave->map)
  {
    /* if we have no map, fill with predictable random generator. */
    cave->map = gd_cave_map_new(cave, GdElement);

    /* IF CAVE HAS NO MAP, USE THE RANDOM NUMBER GENERATOR */
    /* init c64 randomgenerator */
    if (data->level_rand[level] < 0)
      gd_cave_c64_random_set_seed(cave, g_rand_int_range(cave->random, 0, 256),
				  g_rand_int_range(cave->random, 0, 256));
    else
      gd_cave_c64_random_set_seed(cave, 0, data->level_rand[level]);

    /* generate random fill
     * start from row 1 (0 skipped), and fill also the borders on left and right hand side,
     * as c64 did. this way works the original random generator the right way.
     * also, do not fill last row, that is needed for the random seeds to be correct
     * after filling! predictable slime will use it. */
    for (y = 1; y < cave->h - 1; y++)
    {
      for (x = 0; x < cave->w; x++)
      {
	unsigned int randm;

	if (data->level_rand[level] < 0)
	  /* use the much better glib random generator */
	  randm = g_rand_int_range(cave->random, 0, 256);
	else
	  /* use c64 */
	  randm = gd_cave_c64_random(cave);

	element = data->initial_fill;
	if (randm < data->random_fill_probability[0])
	  element = data->random_fill[0];
	if (randm < data->random_fill_probability[1])
	  element = data->random_fill[1];
	if (randm < data->random_fill_probability[2])
	  element = data->random_fill[2];
	if (randm < data->random_fill_probability[3])
	  element = data->random_fill[3];

	gd_cave_store_rc(cave, x, y, element, NULL);
      }
    }

    /* draw initial border */
    for (y = 0; y < cave->h; y++)
    {
      gd_cave_store_rc(cave, 0,           y, cave->initial_border, NULL);
      gd_cave_store_rc(cave, cave->w - 1, y, cave->initial_border, NULL);
    }

    for (x = 0; x < cave->w; x++)
    {
      gd_cave_store_rc(cave, x,           0, cave->initial_border, NULL);
      gd_cave_store_rc(cave, x, cave->h - 1, cave->initial_border, NULL);
    }
  }
  else
  {
    /* IF CAVE HAS A MAP, SIMPLY USE IT... no need to fill with random elements */

    /* initialize c64 predictable random for slime.
       the values were taken from afl bd, see docs/internals.txt */
    gd_cave_c64_random_set_seed(cave, 0, 0x1e);
  }

  if (data->level_slime_seed_c64[level] != -1)
  {
    /* if a specific slime seed is requested, change it now. */

    gd_cave_c64_random_set_seed(cave,
				data->level_slime_seed_c64[level] / 256,
				data->level_slime_seed_c64[level] % 256);
  }

  /* render cave objects above random data or map */
  for (iter = data->objects; iter; iter = g_list_next(iter))
  {
    GdObject *object = (GdObject *)iter->data;

    if (object->levels & gd_levels_mask[level])
      gd_cave_draw_object(cave, iter->data, level);
  }

  /* check if we use c64 ckdelay or milliseconds for timing */
  if (cave->scheduling == GD_SCHEDULING_MILLISECONDS)
    cave->speed = data->level_speed[level];        /* exact timing */
  else
  {
    /* delay loop based timing... set something for first iteration,
       then later it will be calculated */
    cave->speed = 120;

    /* this one may be used by iterate routine to calculate actual delay
       if c64scheduling is selected */
    cave->c64_timing = data->level_ckdelay[level];
  }

  gd_cave_correct_visible_size(cave);

  return cave;
}

/*
   render cave at specified level.
   copy result to the map; remove objects.
   the cave will be map-based.
 */
void gd_flatten_cave(GdCave *cave, const int level)
{
  GdCave *rendered;

  if (cave == NULL)
    return;

  /* render cave at specified level to obtain map. seed = 0 */
  rendered = gd_cave_new_rendered(cave, level, 0);

  /* forget old map without objects */
  gd_cave_map_free(cave->map);

  /* copy new map to cave */
  cave->map = gd_cave_map_dup(rendered, map);
  gd_cave_free(rendered);

  /* forget objects */
  g_list_foreach(cave->objects, (GFunc) free, NULL);
  cave->objects = NULL;
}
