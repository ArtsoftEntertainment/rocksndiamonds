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


/* arrays for movements */
/* also no1 and bd2 cave data import helpers; line direction coordinates */
const int gd_dx[] =
{
  0, 0, 1, 1, 1, 0, -1, -1, -1, 0, 2, 2, 2, 0, -2, -2, -2
};
const int gd_dy[] =
{
  0, -1, -1, 0, 1, 1, 1, 0, -1, -2, -2, 0, 2, 2, 2, 0, -2
};

/* TRANSLATORS:
   None here means "no direction to move"; when there is no gravity while stirring the pot. */
static const char* direction_name[] =
{
  N_("None"),
  N_("Up"),
  N_("Up+right"),
  N_("Right"),
  N_("Down+right"),
  N_("Down"),
  N_("Down+left"),
  N_("Left"),
  N_("Up+left")
};

static const char* direction_filename[] =
{
  "none",
  "up",
  "upright",
  "right",
  "downright",
  "down",
  "downleft",
  "left",
  "upleft"
};

static const char* scheduling_name[] =
{
  N_("Milliseconds"),
  "BD1",
  "BD2",
  "Construction Kit",
  "Crazy Dream 7",
  "Atari BD1",
  "Atari BD2/Construction Kit"
};

static const char* scheduling_filename[] =
{
  "ms",
  "bd1",
  "bd2",
  "plck",
  "crdr7",
  "bd1atari",
  "bd2ckatari"
};

static GHashTable *name_to_element;
GdElement gd_char_to_element[256];

/* color of flashing the screen, gate opening to exit */
const GdColor gd_flash_color = 0xFFFFC0;

/* selected object in editor */
const GdColor gd_select_color = 0x8080FF;

/* direction to string and vice versa */
const char *gd_direction_get_visible_name(GdDirection dir)
{
  return direction_name[dir];
}

const char *gd_direction_get_filename(GdDirection dir)
{
  return direction_filename[dir];
}

GdDirection gd_direction_from_string(const char *str)
{
  int i;

  for (i = 1; i<G_N_ELEMENTS(direction_filename); i++)
    if (strcasecmp(str, direction_filename[i]) == 0)
      return (GdDirection) i;

  Warn("invalid direction name '%s', defaulting to down", str);
  return GD_MV_DOWN;
}

/* scheduling name to string and vice versa */
const char *gd_scheduling_get_filename(GdScheduling sched)
{
  return scheduling_filename[sched];
}

const char *gd_scheduling_get_visible_name(GdScheduling sched)
{
  return scheduling_name[sched];
}

GdScheduling gd_scheduling_from_string(const char *str)
{
  int i;

  for (i = 0; i < G_N_ELEMENTS(scheduling_filename); i++)
    if (strcasecmp(str, scheduling_filename[i]) == 0)
      return (GdScheduling) i;

  Warn("invalid scheduling name '%s', defaulting to plck", str);

  return GD_SCHEDULING_PLCK;
}

/*
  fill a given struct with default properties.
  "str" is the struct (data),
  "properties" describes the structure and its pointers,
  "defaults" are the pieces of data which will be copied to str.
*/
void gd_struct_set_defaults_from_array(gpointer str,
				       const GdStructDescriptor *properties,
				       GdPropertyDefault *defaults)
{
  int i;

  for (i = 0; defaults[i].offset != -1; i++)
  {
    gpointer pvalue = G_STRUCT_MEMBER_P(str, defaults[i].offset);
    /* these point to the same, but to avoid the awkward cast syntax */
    int *ivalue = pvalue;
    GdElement *evalue = pvalue;
    GdDirection *dvalue = pvalue;
    GdScheduling *svalue = pvalue;
    boolean *bvalue = pvalue;
    GdColor *cvalue = pvalue;
    int j, n;

    /* check which property we are talking about: find it in gd_cave_properties. */
    n = defaults[i].property_index;
    if (n == 0)
    {
      while (properties[n].identifier != NULL &&
	     properties[n].offset != defaults[i].offset)
	n++;

      /* remember so we will be fast later*/
      defaults[i].property_index = n;
    }

    /* some properties are arrays. this loop fills all with the same values */
    for (j = 0; j < properties[n].count; j++)
    {
      switch (properties[n].type)
      {
	/* these are for the gui; do nothing */
	case GD_TAB:
	case GD_LABEL:
	  /* no default value for strings */
	case GD_TYPE_STRING:
	case GD_TYPE_LONGSTRING:
	  break;

	case GD_TYPE_RATIO:
	  /* this is also an integer, difference is only when saving to bdcff */
	case GD_TYPE_INT:
	  if (defaults[i].defval < properties[n].min ||
	      defaults[i].defval > properties[n].max)
	    Warn("integer property %s out of range", properties[n].identifier);
	  ivalue[j] = defaults[i].defval;
	  break;

	case GD_TYPE_PROBABILITY:
	  /* floats are stored as integer, /million; but are integers */
	  if (defaults[i].defval < 0 ||
	      defaults[i].defval > 1000000)
	    Warn("integer property %s out of range", properties[n].identifier);
	  ivalue[j] = defaults[i].defval;
	  break;

	case GD_TYPE_BOOLEAN:
	  bvalue[j] = defaults[i].defval != 0;
	  break;

	case GD_TYPE_ELEMENT:
	case GD_TYPE_EFFECT:
	  evalue[j] = (GdElement) defaults[i].defval;
	  break;

	case GD_TYPE_COLOR:
	  cvalue[j] = gd_c64_color(defaults[i].defval);
	  break;

	case GD_TYPE_DIRECTION:
	  dvalue[j] = (GdDirection) defaults[i].defval;
	  break;

	case GD_TYPE_SCHEDULING:
	  svalue[j] = (GdScheduling) defaults[i].defval;
	  break;
      }
    }
  }
}

/* creates the character->element conversion table; using
   the fixed-in-the-bdcff characters. later, this table
   may be filled with more elements.
*/
void gd_create_char_to_element_table(void)
{
  int i;

  /* fill all with unknown */
  for (i = 0; i < G_N_ELEMENTS(gd_char_to_element); i++)
    gd_char_to_element[i] = O_UNKNOWN;

  /* then set fixed characters */
  for (i = 0; i < O_MAX; i++)
  {
    int c = gd_elements[i].character;

    if (c)
    {
      if (gd_char_to_element[c]!=O_UNKNOWN)
	Warn("Character %c already used for element %x", c, gd_char_to_element[c]);

      gd_char_to_element[c] = i;
    }
  }
}

/* search the element database for the specified character, and return the element. */
GdElement gd_get_element_from_character (guint8 character)
{
  if (gd_char_to_element[character] != O_UNKNOWN)
    return gd_char_to_element[character];

  Warn ("Invalid character representing element: %c", character);

  return O_UNKNOWN;
}

/*
  do some init; this function is to be called at the start of the application
*/
void gd_cave_init(void)
{
  int i;

  /* put names to a hash table */
  /* this is a helper for file read operations */
  /* maps g_strdupped strings to elemenets (integers) */
  name_to_element = g_hash_table_new_full(gd_str_case_hash, gd_str_case_equal,
					  free, NULL);

  for (i = 0; i < O_MAX; i++)
  {
    char *key;

    key = g_ascii_strup(gd_elements[i].filename, -1);

    if (g_hash_table_lookup_extended(name_to_element, key, NULL, NULL))
      Warn("Name %s already used for element %x", key, i);

    g_hash_table_insert(name_to_element, key, GINT_TO_POINTER(i));
    /* ^^^ do not free "key", as hash table needs it during the whole time! */

    key = g_strdup_printf("SCANNED_%s", key);        /* new string */

    g_hash_table_insert(name_to_element, key, GINT_TO_POINTER(i));
    /* once again, do not free "key" ^^^ */
  }

  /* for compatibility with tim stridmann's memorydump->bdcff converter... .... ... */
  g_hash_table_insert(name_to_element, "HEXPANDING_WALL", GINT_TO_POINTER(O_H_EXPANDING_WALL));
  g_hash_table_insert(name_to_element, "FALLING_DIAMOND", GINT_TO_POINTER(O_DIAMOND_F));
  g_hash_table_insert(name_to_element, "FALLING_BOULDER", GINT_TO_POINTER(O_STONE_F));
  g_hash_table_insert(name_to_element, "EXPLOSION1S", GINT_TO_POINTER(O_EXPLODE_1));
  g_hash_table_insert(name_to_element, "EXPLOSION2S", GINT_TO_POINTER(O_EXPLODE_2));
  g_hash_table_insert(name_to_element, "EXPLOSION3S", GINT_TO_POINTER(O_EXPLODE_3));
  g_hash_table_insert(name_to_element, "EXPLOSION4S", GINT_TO_POINTER(O_EXPLODE_4));
  g_hash_table_insert(name_to_element, "EXPLOSION5S", GINT_TO_POINTER(O_EXPLODE_5));
  g_hash_table_insert(name_to_element, "EXPLOSION1D", GINT_TO_POINTER(O_PRE_DIA_1));
  g_hash_table_insert(name_to_element, "EXPLOSION2D", GINT_TO_POINTER(O_PRE_DIA_2));
  g_hash_table_insert(name_to_element, "EXPLOSION3D", GINT_TO_POINTER(O_PRE_DIA_3));
  g_hash_table_insert(name_to_element, "EXPLOSION4D", GINT_TO_POINTER(O_PRE_DIA_4));
  g_hash_table_insert(name_to_element, "EXPLOSION5D", GINT_TO_POINTER(O_PRE_DIA_5));
  g_hash_table_insert(name_to_element, "WALL2", GINT_TO_POINTER(O_STEEL_EXPLODABLE));

  /* compatibility with old bd-faq (pre disassembly of bladder) */
  g_hash_table_insert(name_to_element, "BLADDERd9", GINT_TO_POINTER(O_BLADDER_8));

  /* create table to show errors at the start of the application */
  gd_create_char_to_element_table();
}

/* search the element database for the specified name, and return the element */
GdElement gd_get_element_from_string (const char *string)
{
  char *upper = g_ascii_strup(string, -1);
  gpointer value;
  boolean found;

  if (!string)
  {
    Warn("Invalid string representing element: (null)");
    return O_UNKNOWN;
  }

  found = g_hash_table_lookup_extended(name_to_element, upper, NULL, &value);
  free(upper);
  if (found)
    return (GdElement) (GPOINTER_TO_INT(value));

  Warn("Invalid string representing element: %s", string);
  return O_UNKNOWN;
}

void gd_cave_set_defaults_from_array(GdCave* cave, GdPropertyDefault *defaults)
{
  gd_struct_set_defaults_from_array(cave, gd_cave_properties, defaults);
}

/*
  load default values from description array
  these are default for gdash and bdcff.
*/
void gd_cave_set_gdash_defaults(GdCave* cave)
{
  int i;

  gd_cave_set_defaults_from_array(cave, gd_cave_defaults_gdash);

  /* these did not fit into the descriptor array */
  for (i = 0; i < 5; i++)
  {
    cave->level_rand[i] = i;
    cave->level_timevalue[i] = i + 1;
  }
}

/* for quicksort. compares two highscores. */
int gd_highscore_compare(gconstpointer a, gconstpointer b)
{
  const GdHighScore *ha = a;
  const GdHighScore *hb = b;
  return hb->score - ha->score;
}

void gd_clear_highscore(GdHighScore *hs)
{
  int i;

  for (i = 0; i < GD_HIGHSCORE_NUM; i++)
  {
    strcpy(hs[i].name, "");
    hs[i].score = 0;
  }
}

boolean gd_has_highscore(GdHighScore *hs)
{
  return hs[0].score > 0;
}

/* return true if score achieved is a highscore */
boolean gd_is_highscore(GdHighScore *scores, int score)
{
  /* if score is above zero AND bigger than the last one */
  if (score > 0 && score > scores[GD_HIGHSCORE_NUM-1].score)
    return TRUE;

  return FALSE;
}

int gd_add_highscore(GdHighScore *highscores, const char *name, int score)
{
  int i;

  if (!gd_is_highscore(highscores, score))
    return -1;

  /* overwrite the last one */
  gd_strcpy(highscores[GD_HIGHSCORE_NUM-1].name, name);
  highscores[GD_HIGHSCORE_NUM-1].score = score;

  /* and sort */
  qsort(highscores, GD_HIGHSCORE_NUM, sizeof(GdHighScore), gd_highscore_compare);

  for (i = 0; i < GD_HIGHSCORE_NUM; i++)
    if (g_str_equal(highscores[i].name, name) && highscores[i].score == score)
      return i;

  return -1;
}

/* for the case-insensitive hash keys */
boolean gd_str_case_equal(gconstpointer s1, gconstpointer s2)
{
  return strcasecmp(s1, s2) == 0;
}

guint gd_str_case_hash(gconstpointer v)
{
  char *upper;
  guint hash;

  upper = g_ascii_strup(v, -1);
  hash = g_str_hash(v);
  free(upper);
  return hash;
}

/*
  create new cave with default values.
  sets every value, also default size, diamond value etc.
*/
GdCave *gd_cave_new(void)
{
  GdCave *cave;

  cave = checked_calloc(sizeof(GdCave));

  /* hash table which stores unknown tags as strings. */
  cave->tags = g_hash_table_new_full(gd_str_case_hash, gd_str_case_equal, free, free);

  gd_cave_set_gdash_defaults(cave);

  return cave;
}

/* cave maps.
   cave maps are continuous areas in memory. the allocated memory
   is width * height * bytes_per_cell long.
   the cave map[0] stores the pointer given by g_malloc().
   the map itself is also an allocated array of pointers to the
   beginning of rows.
   therefore:
   rows = new (pointers to rows);
   rows[0] = new map
   rows[1..h-1] = rows[0] + width * bytes

   freeing this:
   free(rows[0])
   free(rows)
*/

/*
  allocate a cave map-like array, and initialize to zero.
  one cell is cell_size bytes long.
*/
gpointer gd_cave_map_new_for_cave(const GdCave *cave, const int cell_size)
{
  gpointer *rows;                /* this is void**, pointer to array of ... */
  int y;

  rows = checked_malloc((cave->h) * sizeof(gpointer));
  rows[0] = checked_calloc(cell_size * cave->w * cave->h);

  for (y = 1; y < cave->h; y++)
    /* base pointer + num_of_bytes_per_element * width * number_of_row; as sizeof(char) = 1 */
    rows[y] = (char *)rows[0] + cell_size * cave->w * y;

  return rows;
}

/*
  duplicate map

  if map is null, this also returns null.
*/
gpointer gd_cave_map_dup_size(const GdCave *cave, const gpointer map, const int cell_size)
{
  gpointer *rows;
  gpointer *maplines = (gpointer *)map;
  int y;

  if (!map)
    return NULL;

  rows = checked_malloc((cave->h) * sizeof(gpointer));
  rows[0] = g_memdup (maplines[0], cell_size * cave->w * cave->h);

  for (y = 1; y < cave->h; y++)
    rows[y] = (char *)rows[0] + cell_size * cave->w * y;

  return rows;
}

void gd_cave_map_free(gpointer map)
{
  gpointer *maplines = (gpointer *) map;

  if (!map)
    return;

  free(maplines[0]);
  free(map);
}

/*
  frees memory associated to cave
*/
void gd_cave_free(GdCave *cave)
{
  int i;

  if (!cave)
    return;

  if (cave->tags)
    g_hash_table_destroy(cave->tags);

  if (cave->random)    /* random generator is a GRand * */
    g_rand_free(cave->random);

  /* free strings */
  for (i = 0; gd_cave_properties[i].identifier != NULL; i++)
    if (gd_cave_properties[i].type == GD_TYPE_LONGSTRING)
      checked_free(G_STRUCT_MEMBER(char *, cave, gd_cave_properties[i].offset));

  /* map */
  gd_cave_map_free(cave->map);

  /* rendered data */
  gd_cave_map_free(cave->objects_order);

  /* hammered walls to reappear data */
  gd_cave_map_free(cave->hammered_reappear);

  /* free objects */
  g_list_foreach(cave->objects, (GFunc) free, NULL);
  g_list_free (cave->objects);

  /* free replays */
  g_list_foreach(cave->replays, (GFunc) gd_replay_free, NULL);
  g_list_free(cave->replays);

  /* freeing main pointer */
  free (cave);
}

static void hash_copy_foreach(const char *key, const char *value, GHashTable *dest)
{
  g_hash_table_insert(dest, g_strdup(key), g_strdup(value));
}

/* copy cave from src to destination, with duplicating dynamically allocated data */
void gd_cave_copy(GdCave *dest, const GdCave *src)
{
  int i;

  /* copy entire data */
  g_memmove(dest, src, sizeof(GdCave));

  /* but duplicate dynamic data */
  dest->tags = g_hash_table_new_full(gd_str_case_hash, gd_str_case_equal,
				     free, free);
  if (src->tags)
    g_hash_table_foreach(src->tags, (GHFunc) hash_copy_foreach, dest->tags);

  dest->map = gd_cave_map_dup(src, map);
  dest->hammered_reappear = gd_cave_map_dup(src, hammered_reappear);

  /* for longstrings */
  for (i = 0; gd_cave_properties[i].identifier != NULL; i++)
    if (gd_cave_properties[i].type == GD_TYPE_LONGSTRING)
      G_STRUCT_MEMBER(char *, dest, gd_cave_properties[i].offset) =
	getStringCopy(G_STRUCT_MEMBER(char *, src, gd_cave_properties[i].offset));

  /* no reason to copy this */
  dest->objects_order = NULL;

  /* copy objects list */
  if (src->objects)
  {
    GList *iter;

    dest->objects = NULL;    /* new empty list */
    for (iter = src->objects; iter != NULL; iter = iter->next) /* do a deep copy */
      dest->objects = g_list_append(dest->objects, g_memdup (iter->data, sizeof (GdObject)));
  }

  /* copy replays */
  if (src->replays)
  {
    GList *iter;

    dest->replays = NULL;
    for (iter = src->replays; iter != NULL; iter = iter->next) /* do a deep copy */
      dest->replays = g_list_append(dest->replays, gd_replay_new_from_replay(iter->data));
  }

  /* copy random number generator */
  if (src->random)
    dest->random = g_rand_copy(src->random);
}

/* create new cave, which is a copy of the cave given. */
GdCave *gd_cave_new_from_cave(const GdCave *orig)
{
  GdCave *cave;

  cave = gd_cave_new();
  gd_cave_copy(cave, orig);

  return cave;
}

/*
  Put an object to the specified position.
  Performs range checking.
  If wraparound objects are selected, wraps around x coordinates, with or without lineshift.
  (The y coordinate is not wrapped, as it did not work like that on the c64)
  order is a pointer to the GdObject describing this object. Thus the editor can identify which cell was created by which object.
*/
void gd_cave_store_rc(GdCave *cave, int x, int y, const GdElement element, const void *order)
{
  /* if we do not need to draw, exit now */
  if (element == O_NONE)
    return;

  /* check bounds */
  if (cave->wraparound_objects)
  {
    if (cave->lineshift)
    {
      /* fit x coordinate within range, with correcting y at the same time */
      while (x < 0)
      {
	x += cave->w;    /* out of bounds on the left... */
	y--;             /* previous row */
      }

      while (x >= cave->w)
      {
	x -= cave->w;
	y++;
      }

      /* lineshifting does not fix the y coordinates.
	 if out of bounds, element will not be displayed. */
      /* if such an object appeared in the c64 game, well, it was a buffer overrun. */
    }
    else
    {
      /* non lineshifting: changing x does not change y coordinate. */
      while (x < 0)
	x += cave->w;

      while (x >= cave->w)
	x -= cave->w;

      /* after that, fix y coordinate */
      while (y < 0)
	y += cave->h;

      while (y >= cave->h)
	y -= cave->h;
    }
  }

  /* if the above wraparound code fixed the coordinates, this will always be true. */
  /* but see the above comment for lineshifting y coordinate */
  if (x >= 0 && x < cave->w && y >= 0 && y < cave->h)
  {
    cave->map[y][x] = element;
    cave->objects_order[y][x] = (void *)order;
  }
}

GdElement gd_cave_get_rc(const GdCave *cave, int x, int y)
{
  /* always fix coordinates as if cave was wraparound. */

  /* fix x coordinate */
  if (cave->lineshift)
  {
    /* fit x coordinate within range, with correcting y at the same time */
    while (x < 0)
    {
      x += cave->w;    /* out of bounds on the left... */
      y--;             /* previous row */
    }
    while (x >= cave->w)
    {
      x -= cave->w;
      y++;
    }
  }
  else
  {
    /* non lineshifting: changing x does not change y coordinate. */
    while (x < 0)
      x += cave->w;

    while (x >= cave->w)
      x -= cave->w;
  }

  /* after that, fix y coordinate */
  while (y < 0)
    y += cave->h;

  while (y >= cave->h)
    y -= cave->h;

  return cave->map[y][x];
}

unsigned int gd_c64_random(GdC64RandomGenerator *rand)
{
  unsigned int temp_rand_1, temp_rand_2, carry, result;

  temp_rand_1 = (rand->rand_seed_1 & 0x0001) << 7;
  temp_rand_2 = (rand->rand_seed_2 >> 1) & 0x007F;
  result = (rand->rand_seed_2) + ((rand->rand_seed_2 & 0x0001) << 7);
  carry = (result >> 8);
  result = result & 0x00FF;
  result = result + carry + 0x13;
  carry = (result >> 8);
  rand->rand_seed_2 = result & 0x00FF;
  result = rand->rand_seed_1 + carry + temp_rand_1;
  carry = (result >> 8);
  result = result & 0x00FF;
  result = result + carry + temp_rand_2;
  rand->rand_seed_1 = result & 0x00FF;

  return rand->rand_seed_1;
}

/*
  C64 BD predictable random number generator.
  Used to load the original caves imported from c64 files.
  Also by the predictable slime.
*/
unsigned int gd_cave_c64_random(GdCave *cave)
{
  return gd_c64_random(&cave->c64_rand);
}

void gd_c64_random_set_seed(GdC64RandomGenerator *rand, int seed1, int seed2)
{
  rand->rand_seed_1 = seed1;
  rand->rand_seed_2 = seed2;
}

void gd_cave_c64_random_set_seed(GdCave *cave, int seed1, int seed2)
{
  gd_c64_random_set_seed(&cave->c64_rand, seed1, seed2);
}

/*
  select random colors for a given cave.
  this function will select colors so that they should look somewhat nice; for example
  brick walls won't be the darkest color, for example.
*/
static inline void swap(int *i1, int *i2)
{
  int t = *i1;
  *i1 = *i2;
  *i2 = t;
}

/*
  shrink cave
  if last line or last row is just steel wall (or (invisible) outbox).
  used after loading a game for playing.
  after this, ew and eh will contain the effective width and height.
*/
void gd_cave_auto_shrink(GdCave *cave)
{

  int x, y;
  enum
  {
    STEEL_ONLY,
    STEEL_OR_OTHER,
    NO_SHRINK
  }
  empty;

  /* set to maximum size, then try to shrink */
  cave->x1 = 0;
  cave->y1 = 0;
  cave->x2 = cave->w - 1;
  cave->y2 = cave->h - 1;

  /* search for empty, steel-wall-only last rows. */
  /* clear all lines, which are only steel wall.
   * and clear only one line, which is steel wall, but also has a player or an outbox. */
  empty = STEEL_ONLY;

  do
  {
    for (y = cave->y2 - 1; y <= cave->y2; y++)
    {
      for (x = cave->x1; x <= cave->x2; x++)
      {
	switch (gd_cave_get_rc (cave, x, y))
	{
	  /* if steels only, this is to be deleted. */
	  case O_STEEL:
	    break;

	  case O_PRE_OUTBOX:
	  case O_PRE_INVIS_OUTBOX:
	  case O_INBOX:
	    if (empty == STEEL_OR_OTHER)
	      empty = NO_SHRINK;

	    /* if this, delete only this one, and exit. */
	    if (empty == STEEL_ONLY)
	      empty = STEEL_OR_OTHER;
	    break;

	  default:
	    /* anything else, that should be left in the cave. */
	    empty = NO_SHRINK;
	    break;
	}
      }
    }

    /* shrink if full steel or steel and player/outbox. */
    if (empty != NO_SHRINK)
      cave->y2--;            /* one row shorter */
  }
  while (empty == STEEL_ONLY);    /* if found just steels, repeat. */

  /* search for empty, steel-wall-only first rows. */
  empty = STEEL_ONLY;

  do
  {
    for (y = cave->y1; y <= cave->y1 + 1; y++)
    {
      for (x = cave->x1; x <= cave->x2; x++)
      {
	switch (gd_cave_get_rc (cave, x, y))
	{
	  case O_STEEL:
	    break;

	  case O_PRE_OUTBOX:
	  case O_PRE_INVIS_OUTBOX:
	  case O_INBOX:
	    /* shrink only lines, which have only ONE player or outbox.
	       this is for bd4 intermission 2, for example. */
	    if (empty == STEEL_OR_OTHER)
	      empty = NO_SHRINK;
	    if (empty == STEEL_ONLY)
	      empty = STEEL_OR_OTHER;
	    break;

	  default:
	    empty = NO_SHRINK;
	    break;
	}
      }
    }

    if (empty != NO_SHRINK)
      cave->y1++;
  }
  while (empty == STEEL_ONLY);    /* if found one, repeat. */

  /* empty last columns. */
  empty = STEEL_ONLY;

  do
  {
    for (y = cave->y1; y <= cave->y2; y++)
    {
      for (x = cave->x2 - 1; x <= cave->x2; x++)
      {
	switch (gd_cave_get_rc (cave, x, y))
	{
	  case O_STEEL:
	    break;

	  case O_PRE_OUTBOX:
	  case O_PRE_INVIS_OUTBOX:
	  case O_INBOX:
	    if (empty == STEEL_OR_OTHER)
	      empty = NO_SHRINK;
	    if (empty == STEEL_ONLY)
	      empty = STEEL_OR_OTHER;
	    break;

	  default:
	    empty = NO_SHRINK;
	    break;
	}
      }
    }

    /* just remember that one column shorter.
       free will know the size of memchunk, no need to realloc! */
    if (empty != NO_SHRINK)
      cave->x2--;
  }
  while (empty == STEEL_ONLY);    /* if found one, repeat. */

  /* empty first columns. */
  empty = STEEL_ONLY;

  do
  {
    for (y = cave->y1; y <= cave->y2; y++)
    {
      for (x = cave->x1; x <= cave->x1 + 1; x++)
      {
	switch (gd_cave_get_rc (cave, x, y))
	{
	  case O_STEEL:
	    break;

	  case O_PRE_OUTBOX:
	  case O_PRE_INVIS_OUTBOX:
	  case O_INBOX:
	    if (empty == STEEL_OR_OTHER)
	      empty = NO_SHRINK;
	    if (empty == STEEL_ONLY)
	      empty = STEEL_OR_OTHER;
	    break;

	  default:
	    empty = NO_SHRINK;
	    break;
	}
      }
    }

    if (empty != NO_SHRINK)
      cave->x1++;
  }
  while (empty == STEEL_ONLY);    /* if found one, repeat. */
}

/* check if cave visible part coordinates
   are outside cave sizes, or not in the right order.
   correct them if needed.
*/
void gd_cave_correct_visible_size(GdCave *cave)
{
  /* change visible coordinates if they do not point to upperleft and lowerright */
  if (cave->x2 < cave->x1)
  {
    int t = cave->x2;
    cave->x2 = cave->x1;
    cave->x1 = t;
  }

  if (cave->y2 < cave->y1)
  {
    int t = cave->y2;
    cave->y2 = cave->y1;
    cave->y1 = t;
  }

  if (cave->x1 < 0)
    cave->x1 = 0;

  if (cave->y1 < 0)
    cave->y1 = 0;

  if (cave->x2 > cave->w - 1)
    cave->x2 = cave->w - 1;

  if (cave->y2 > cave->h - 1)
    cave->y2 = cave->h - 1;
}

/*
  bd1 and similar engines had animation bits in cave data, to set which elements to animate
  (firefly, butterfly, amoeba).
  animating an element also caused some delay each frame; according to my measurements,
  around 2.6 ms/element.
*/
static void cave_set_ckdelay_extra_for_animation(GdCave *cave)
{
  int x, y;
  boolean has_amoeba = FALSE, has_firefly = FALSE, has_butterfly = FALSE;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      switch (cave->map[y][x] & ~SCANNED)
      {
	case O_FIREFLY_1:
	case O_FIREFLY_2:
	case O_FIREFLY_3:
	case O_FIREFLY_4:
	  has_firefly = TRUE;
	  break;

	case O_BUTTER_1:
	case O_BUTTER_2:
	case O_BUTTER_3:
	case O_BUTTER_4:
	  has_butterfly = TRUE;
	  break;

	case O_AMOEBA:
	  has_amoeba = TRUE;
	  break;
      }
    }
  }

  cave->ckdelay_extra_for_animation = 0;
  if (has_amoeba)
    cave->ckdelay_extra_for_animation += 2600;
  if (has_firefly)
    cave->ckdelay_extra_for_animation += 2600;
  if (has_butterfly)
    cave->ckdelay_extra_for_animation += 2600;
  if (has_amoeba)
    cave->ckdelay_extra_for_animation += 2600;
}

/* do some init - setup some cave variables before the game. */
void gd_cave_setup_for_game(GdCave *cave)
{
  int x, y;

  cave_set_ckdelay_extra_for_animation(cave);

  /* find the player which will be the one to scroll to at the beginning of the game
     (before the player's birth) */
  if (cave->active_is_first_found)
  {
    /* uppermost player is active */
    for (y = cave->h - 1; y >= 0; y--)
    { 
     for (x = cave->w - 1; x >= 0; x--)
     {
	if (cave->map[y][x] == O_INBOX)
	{
	  cave->player_x = x;
	  cave->player_y = y;
	}
     }
    }
  }
  else
  {
    /* lowermost player is active */
    for (y = 0; y < cave->h; y++)
    {
      for (x = 0; x < cave->w; x++)
      {
	if (cave->map[y][x] == O_INBOX)
	{
	  cave->player_x = x;
	  cave->player_y = y;
	}
      }
    }
  }

  /* select number of milliseconds (for pal and ntsc) */
  cave->timing_factor = cave->pal_timing ? 1200 : 1000;

  cave->time			*= cave->timing_factor;
  cave->magic_wall_time		*= cave->timing_factor;
  cave->amoeba_time		*= cave->timing_factor;
  cave->amoeba_2_time		*= cave->timing_factor;
  cave->hatching_delay_time	*= cave->timing_factor;

  if (cave->hammered_walls_reappear)
    cave->hammered_reappear = gd_cave_map_new(cave, int);
}

/* cave diamonds needed can be set to n<=0. */
/* if so, count the diamonds at the time of the hatching, and decrement that value from */
/* the number of diamonds found. */
/* of course, this function is to be called from the cave engine, at the exact time of hatching. */
void gd_cave_count_diamonds(GdCave *cave)
{
  int x, y;

  /* if automatically counting diamonds. if this was negative,
   * the sum will be this less than the number of all the diamonds in the cave */
  if (cave->diamonds_needed <= 0)
  {
    for (y = 0; y < cave->h; y++)
      for (x = 0; x < cave->w; x++)
	if (cave->map[y][x] == O_DIAMOND)
	  cave->diamonds_needed++;

    /* if still below zero, let this be 0, so gate will be open immediately */
    if (cave->diamonds_needed < 0)
      cave->diamonds_needed = 0;
  }
}

/* takes a cave and a gfx buffer, and fills the buffer with cell indexes.
   the indexes might change if bonus life flash is active (small lines in
   "SPACE" cells),
   for the paused state (which is used in gdash but not in sdash) - yellowish
   color.
   also one can select the animation frame (0..7) to draw the cave on. so the
   caller manages
   increasing that.

   if a cell is changed, it is flagged with GD_REDRAW; the flag can be cleared
   by the caller.
*/
void gd_drawcave_game(const GdCave *cave, int **element_buffer, int **gfx_buffer,
		      boolean bonus_life_flash, int animcycle, boolean hate_invisible_outbox)
{
  static int player_blinking = 0;
  static int player_tapping = 0;
  int elemmapping[O_MAX];
  int elemdrawing[O_MAX];
  int x, y, map, draw;

  if (cave->last_direction)
  {
    /* he is moving, so stop blinking and tapping. */
    player_blinking = 0;
    player_tapping = 0;
  }
  else
  {
    /* he is idle, so animations can be done. */
    if (animcycle == 0)
    {
      /* blinking and tapping is started at the beginning of animation sequences. */
      /* 1/4 chance of blinking, every sequence. */
      player_blinking = g_random_int_range(0, 4) == 0;

      /* 1/16 chance of starting or stopping tapping. */
      if (g_random_int_range(0, 16) == 0)
	player_tapping = !player_tapping;
    }
  }

  for (x = 0; x < O_MAX; x++)
  {
    elemmapping[x] = x;
    elemdrawing[x] = gd_elements[x].image_game;
  }

  if (bonus_life_flash)
  {
    elemmapping[O_SPACE] = O_FAKE_BONUS;
    elemdrawing[O_SPACE] = gd_elements[O_FAKE_BONUS].image_game;
  }

  elemmapping[O_MAGIC_WALL] = (cave->magic_wall_state == GD_MW_ACTIVE ? O_MAGIC_WALL : O_BRICK);
  elemdrawing[O_MAGIC_WALL] = gd_elements[cave->magic_wall_state == GD_MW_ACTIVE ? O_MAGIC_WALL : O_BRICK].image_game;

  elemmapping[O_CREATURE_SWITCH] = (cave->creatures_backwards ? O_CREATURE_SWITCH_ON : O_CREATURE_SWITCH);
  elemdrawing[O_CREATURE_SWITCH] = gd_elements[cave->creatures_backwards ? O_CREATURE_SWITCH_ON : O_CREATURE_SWITCH].image_game;

  elemmapping[O_EXPANDING_WALL_SWITCH] = (cave->expanding_wall_changed ? O_EXPANDING_WALL_SWITCH_VERT : O_EXPANDING_WALL_SWITCH_HORIZ);
  elemdrawing[O_EXPANDING_WALL_SWITCH] = gd_elements[cave->expanding_wall_changed ? O_EXPANDING_WALL_SWITCH_VERT : O_EXPANDING_WALL_SWITCH_HORIZ].image_game;

  elemmapping[O_GRAVITY_SWITCH] = (cave->gravity_switch_active ? O_GRAVITY_SWITCH_ACTIVE : O_GRAVITY_SWITCH);
  elemdrawing[O_GRAVITY_SWITCH] = gd_elements[cave->gravity_switch_active ? O_GRAVITY_SWITCH_ACTIVE : O_GRAVITY_SWITCH].image_game;

  elemmapping[O_REPLICATOR_SWITCH] = (cave->replicators_active ? O_REPLICATOR_SWITCH_ON : O_REPLICATOR_SWITCH_OFF);
  elemdrawing[O_REPLICATOR_SWITCH] = gd_elements[cave->replicators_active ? O_REPLICATOR_SWITCH_ON : O_REPLICATOR_SWITCH_OFF].image_game;

  if (cave->replicators_active)
    /* if the replicators are active, animate them. */
    elemmapping[O_REPLICATOR] = O_REPLICATOR_ACTIVE;

  if (!cave->replicators_active)
    /* if the replicators are inactive, do not animate them. */
    elemdrawing[O_REPLICATOR] = ABS(elemdrawing[O_REPLICATOR]);

  elemmapping[O_CONVEYOR_SWITCH] = (cave->conveyor_belts_active ? O_CONVEYOR_SWITCH_ON : O_CONVEYOR_SWITCH_OFF);
  elemdrawing[O_CONVEYOR_SWITCH] = gd_elements[cave->conveyor_belts_active ? O_CONVEYOR_SWITCH_ON : O_CONVEYOR_SWITCH_OFF].image_game;

  if (cave->conveyor_belts_direction_changed)
  {
    /* if direction is changed, animation is changed. */
    int temp;

    elemmapping[O_CONVEYOR_LEFT] = O_CONVEYOR_RIGHT;
    elemmapping[O_CONVEYOR_RIGHT] = O_CONVEYOR_LEFT;

    temp = elemdrawing[O_CONVEYOR_LEFT];
    elemdrawing[O_CONVEYOR_LEFT] = elemdrawing[O_CONVEYOR_RIGHT];
    elemdrawing[O_CONVEYOR_RIGHT] = temp;

    elemmapping[O_CONVEYOR_DIR_SWITCH] = O_CONVEYOR_DIR_CHANGED;
    elemdrawing[O_CONVEYOR_DIR_SWITCH] = gd_elements[O_CONVEYOR_DIR_CHANGED].image_game;
  }
  else
  {
    elemmapping[O_CONVEYOR_DIR_SWITCH] = O_CONVEYOR_DIR_NORMAL;
    elemdrawing[O_CONVEYOR_DIR_SWITCH] = gd_elements[O_CONVEYOR_DIR_NORMAL].image_game;
  }

  if (cave->conveyor_belts_active)
  {
    /* keep potentially changed direction */
    int offset = (O_CONVEYOR_LEFT_ACTIVE - O_CONVEYOR_LEFT);

    /* if they are running, animate them. */
    elemmapping[O_CONVEYOR_LEFT]  += offset;
    elemmapping[O_CONVEYOR_RIGHT] += offset;
  }
  if (!cave->conveyor_belts_active)
  {
    /* if they are not running, do not animate them. */
    elemdrawing[O_CONVEYOR_LEFT] = ABS(elemdrawing[O_CONVEYOR_LEFT]);
    elemdrawing[O_CONVEYOR_RIGHT] = ABS(elemdrawing[O_CONVEYOR_RIGHT]);
  }

  if (animcycle & 2)
  {
    /* also a hack, like biter_switch */
    elemdrawing[O_PNEUMATIC_ACTIVE_LEFT]  += 2;
    elemdrawing[O_PNEUMATIC_ACTIVE_RIGHT] += 2;
    elemdrawing[O_PLAYER_PNEUMATIC_LEFT]  += 2;
    elemdrawing[O_PLAYER_PNEUMATIC_RIGHT] += 2;
  }

  if ((cave->last_direction) == GD_MV_STILL)
  {
    /* player is idle. */
    if (player_blinking && player_tapping)
    {
      map = O_PLAYER_TAP_BLINK;
      draw = gd_elements[O_PLAYER_TAP_BLINK].image_game;
    }
    else if (player_blinking)
    {
      map = O_PLAYER_BLINK;
      draw = gd_elements[O_PLAYER_BLINK].image_game;
    }
    else if (player_tapping)
    {
      map = O_PLAYER_TAP;
      draw = gd_elements[O_PLAYER_TAP].image_game;
    }
    else
    {
      map = O_PLAYER;
      draw = gd_elements[O_PLAYER].image_game;
    }
  }
  else if (cave->last_horizontal_direction == GD_MV_LEFT)
  {
    map = O_PLAYER_LEFT;
    draw = gd_elements[O_PLAYER_LEFT].image_game;
  }
  else
  {
    /* of course this is GD_MV_RIGHT. */
    map = O_PLAYER_RIGHT;
    draw = gd_elements[O_PLAYER_RIGHT].image_game;
  }

  elemmapping[O_PLAYER] = map;
  elemmapping[O_PLAYER_GLUED] = map;

  elemdrawing[O_PLAYER] = draw;
  elemdrawing[O_PLAYER_GLUED] = draw;

  /* player with bomb does not blink or tap - no graphics drawn for that.
     running is drawn using w/o bomb cells */
  if (cave->last_direction!=GD_MV_STILL)
  {
    elemmapping[O_PLAYER_BOMB] = map;
    elemdrawing[O_PLAYER_BOMB] = draw;
  }

  elemmapping[O_INBOX] = (cave->inbox_flash_toggle ? O_INBOX_OPEN : O_INBOX_CLOSED);
  elemdrawing[O_INBOX] = gd_elements[cave->inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED].image_game;

  elemmapping[O_OUTBOX] = (cave->inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED);
  elemdrawing[O_OUTBOX] = gd_elements[cave->inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED].image_game;

  /* hack, not fit into gd_elements */
  elemmapping[O_BITER_SWITCH] = O_BITER_SWITCH_1 + cave->biter_delay_frame;
  /* hack, not fit into gd_elements */
  elemdrawing[O_BITER_SWITCH] = gd_elements[O_BITER_SWITCH].image_game + cave->biter_delay_frame;

  /* visual effects */
  elemmapping[O_DIRT] = cave->dirt_looks_like;
  elemmapping[O_EXPANDING_WALL] = cave->expanding_wall_looks_like;
  elemmapping[O_V_EXPANDING_WALL] = cave->expanding_wall_looks_like;
  elemmapping[O_H_EXPANDING_WALL] = cave->expanding_wall_looks_like;
  elemmapping[O_AMOEBA_2] = cave->amoeba_2_looks_like;

  /* visual effects */
  elemdrawing[O_DIRT] = elemdrawing[cave->dirt_looks_like];
  elemdrawing[O_EXPANDING_WALL] = elemdrawing[cave->expanding_wall_looks_like];
  elemdrawing[O_V_EXPANDING_WALL] = elemdrawing[cave->expanding_wall_looks_like];
  elemdrawing[O_H_EXPANDING_WALL] = elemdrawing[cave->expanding_wall_looks_like];
  elemdrawing[O_AMOEBA_2] = elemdrawing[cave->amoeba_2_looks_like];

  /* change only graphically */
  if (hate_invisible_outbox)
  {
    elemmapping[O_PRE_INVIS_OUTBOX] = O_PRE_OUTBOX;
    elemmapping[O_INVIS_OUTBOX] = O_OUTBOX;
  }

  if (hate_invisible_outbox)
  {
    elemdrawing[O_PRE_INVIS_OUTBOX] = elemdrawing[O_PRE_OUTBOX];
    elemdrawing[O_INVIS_OUTBOX] = elemdrawing[O_OUTBOX];
  }

  for (y = cave->y1; y <= cave->y2; y++)
  {
    for (x = cave->x1; x <= cave->x2; x++)
    {
      GdElement actual = cave->map[y][x];

      /* if covered, real element is not important */
      if (actual & COVERED)
	map = O_COVERED;
      else
	map = elemmapping[actual];

      /* if covered, real element is not important */
      if (actual & COVERED)
	draw = gd_elements[O_COVERED].image_game;
      else
	draw = elemdrawing[actual];

      /* if negative, animated. */
      if (draw < 0)
	draw = -draw + animcycle;

      /* flash */
      if (cave->gate_open_flash)
	draw += GD_NUM_OF_CELLS;

      /* set to buffer, with caching */
      if (element_buffer[y][x] != map)
	element_buffer[y][x] = map;

      if (gfx_buffer[y][x] != draw)
	gfx_buffer[y][x] = draw | GD_REDRAW;
    }
  }
}

/* cave time is rounded _UP_ to seconds. so at the exact moment when it
   changes from
   2sec remaining to 1sec remaining, the player has exactly one second.
   when it changes
   to zero, it is the exact moment of timeout. */
/* internal time is milliseconds (or 1200 milliseconds for pal timing). */
int gd_cave_time_show(const GdCave *cave, int internal_time)
{
  return (internal_time + cave->timing_factor - 1) / cave->timing_factor;
}

GdReplay *gd_replay_new(void)
{
  GdReplay *rep;

  rep = checked_calloc(sizeof(GdReplay));

  rep->movements = g_byte_array_new();

  return rep;
}

GdReplay *gd_replay_new_from_replay(GdReplay *orig)
{
  GdReplay *rep;

  rep = g_memdup(orig, sizeof(GdReplay));

  /* replicate dynamic data */
  rep->comment = getStringCopy(orig->comment);
  rep->movements = g_byte_array_new();
  g_byte_array_append(rep->movements, orig->movements->data, orig->movements->len);

  return rep;
}

void gd_replay_free(GdReplay *replay)
{
  g_byte_array_free(replay->movements, TRUE);
  checked_free(replay->comment);
  free(replay);
}

/* store movement in a replay */
void gd_replay_store_movement(GdReplay *replay, GdDirection player_move,
			      boolean player_fire, boolean suicide)
{
  guint8 data[1];

  data[0] = ((player_move) |
	     (player_fire ? GD_REPLAY_FIRE_MASK : 0) |
	     (suicide ? GD_REPLAY_SUICIDE_MASK : 0));

  g_byte_array_append(replay->movements, data, 1);
}

/* get next available movement from a replay; store variables to player_move,
   player_fire, suicide */
/* return true if successful */
boolean gd_replay_get_next_movement(GdReplay *replay, GdDirection *player_move,
				    boolean *player_fire, boolean *suicide)
{
  guint8 data;

  /* if no more available movements */
  if (replay->current_playing_pos >= replay->movements->len)
    return FALSE;

  data = replay->movements->data[replay->current_playing_pos++];
  *suicide = (data & GD_REPLAY_SUICIDE_MASK) != 0;
  *player_fire = (data & GD_REPLAY_FIRE_MASK) != 0;
  *player_move = (data & GD_REPLAY_MOVE_MASK);

  return TRUE;
}

/* calculate adler checksum for a rendered cave; this can be used for more caves. */
void gd_cave_adler_checksum_more(GdCave *cave, guint32 *a, guint32 *b)
{
  int x, y;

  for (y = 0; y < cave->h; y++)
    for (x = 0; x < cave->w; x++)
    {
      *a += gd_elements[cave->map[y][x]].character;
      *b += *a;

      *a %= 65521;
      *b %= 65521;
    }
}

/* calculate adler checksum for a single rendered cave. */
guint32
gd_cave_adler_checksum(GdCave *cave)
{
  guint32 a = 1;
  guint32 b = 0;

  gd_cave_adler_checksum_more(cave, &a, &b);
  return (b << 16) + a;
}

/* return c64 color with index. */
GdColor gd_c64_color(int index)
{
  return (GD_COLOR_TYPE_C64 << 24) + index;
}
