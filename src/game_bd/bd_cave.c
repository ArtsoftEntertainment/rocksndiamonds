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


// arrays for movements
// also no1 and bd2 cave data import helpers; line direction coordinates
const int gd_dx[] =
{
  0, 0, 1, 1, 1, 0, -1, -1, -1, 0, 2, 2, 2, 0, -2, -2, -2
};
const int gd_dy[] =
{
  0, -1, -1, 0, 1, 1, 1, 0, -1, -2, -2, 0, 2, 2, 2, 0, -2
};

// TRANSLATORS:
// None here means "no direction to move"; when there is no gravity while stirring the pot.
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

static HashTable *name_to_element;
GdElement gd_char_to_element[256];

// color of flashing the screen, gate opening to exit
const GdColor gd_flash_color = 0xFFFFC0;

// selected object in editor
const GdColor gd_select_color = 0x8080FF;

// direction to string and vice versa
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

  for (i = 1; i < ARRAY_SIZE(direction_filename); i++)
    if (strcasecmp(str, direction_filename[i]) == 0)
      return (GdDirection) i;

  Warn("invalid direction name '%s', defaulting to down", str);
  return GD_MV_DOWN;
}

// scheduling name to string and vice versa
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

  for (i = 0; i < ARRAY_SIZE(scheduling_filename); i++)
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
void gd_struct_set_defaults_from_array(void *str,
				       const GdStructDescriptor *properties,
				       GdPropertyDefault *defaults)
{
  int i;

  for (i = 0; defaults[i].offset != -1; i++)
  {
    void *pvalue = STRUCT_MEMBER_P(str, defaults[i].offset);
    // these point to the same, but to avoid the awkward cast syntax
    int *ivalue = pvalue;
    GdElement *evalue = pvalue;
    GdDirection *dvalue = pvalue;
    GdScheduling *svalue = pvalue;
    boolean *bvalue = pvalue;
    GdColor *cvalue = pvalue;
    int j, n;

    // check which property we are talking about: find it in gd_cave_properties.
    n = defaults[i].property_index;
    if (n == 0)
    {
      while (properties[n].identifier != NULL &&
	     properties[n].offset != defaults[i].offset)
	n++;

      // remember so we will be fast later
      defaults[i].property_index = n;
    }

    // some properties are arrays. this loop fills all with the same values
    for (j = 0; j < properties[n].count; j++)
    {
      switch (properties[n].type)
      {
	// these are for the gui; do nothing
	case GD_TAB:
	case GD_LABEL:
	  // no default value for strings
	case GD_TYPE_STRING:
	case GD_TYPE_LONGSTRING:
	  break;

	case GD_TYPE_RATIO:
	  // this is also an integer, difference is only when saving to bdcff
	case GD_TYPE_INT:
	  if (defaults[i].defval < properties[n].min ||
	      defaults[i].defval > properties[n].max)
	    Warn("integer property %s out of range", properties[n].identifier);
	  ivalue[j] = defaults[i].defval;
	  break;

	case GD_TYPE_PROBABILITY:
	  // floats are stored as integer, /million; but are integers
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

/*
  creates the character->element conversion table; using
  the fixed-in-the-bdcff characters. later, this table
  may be filled with more elements.
*/
void gd_create_char_to_element_table(void)
{
  int i;

  // fill all with unknown
  for (i = 0; i < ARRAY_SIZE(gd_char_to_element); i++)
    gd_char_to_element[i] = O_UNKNOWN;

  // then set fixed characters
  for (i = 0; i < O_MAX; i++)
  {
    int c = gd_element_properties[i].character;

    if (c)
    {
      if (gd_char_to_element[c] != O_UNKNOWN)
	Warn("Character %c already used for element %x", c, gd_char_to_element[c]);

      gd_char_to_element[c] = i;
    }
  }
}

// search the element database for the specified character, and return the element.
GdElement gd_get_element_from_character (byte character)
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

  // put names to a hash table
  // this is a helper for file read operations
  // maps copied strings to elements (integers)
  name_to_element = create_hashtable(gd_str_case_hash, gd_str_case_equal, NULL, NULL);

  for (i = 0; i < O_MAX; i++)
  {
    char *key_1 = getStringToUpper(gd_element_properties[i].filename);

    if (hashtable_exists(name_to_element, key_1))	// hash value may be 0
      Warn("BDCFF token '%s' already used for element 0x%x", key_1, i);

    hashtable_insert(name_to_element, key_1, INT_TO_PTR(i));
    // ^^^ do not free "key_1", as hash table needs it during the whole time!
  }

  // for compatibility with tim stridmann's memorydump->bdcff converter... .... ...
  hashtable_insert(name_to_element, "HEXPANDING_WALL", INT_TO_PTR(O_H_EXPANDING_WALL));
  hashtable_insert(name_to_element, "FALLING_DIAMOND", INT_TO_PTR(O_DIAMOND_F));
  hashtable_insert(name_to_element, "FALLING_BOULDER", INT_TO_PTR(O_STONE_F));
  hashtable_insert(name_to_element, "EXPLOSION1S", INT_TO_PTR(O_EXPLODE_1));
  hashtable_insert(name_to_element, "EXPLOSION2S", INT_TO_PTR(O_EXPLODE_2));
  hashtable_insert(name_to_element, "EXPLOSION3S", INT_TO_PTR(O_EXPLODE_3));
  hashtable_insert(name_to_element, "EXPLOSION4S", INT_TO_PTR(O_EXPLODE_4));
  hashtable_insert(name_to_element, "EXPLOSION5S", INT_TO_PTR(O_EXPLODE_5));
  hashtable_insert(name_to_element, "EXPLOSION1D", INT_TO_PTR(O_PRE_DIA_1));
  hashtable_insert(name_to_element, "EXPLOSION2D", INT_TO_PTR(O_PRE_DIA_2));
  hashtable_insert(name_to_element, "EXPLOSION3D", INT_TO_PTR(O_PRE_DIA_3));
  hashtable_insert(name_to_element, "EXPLOSION4D", INT_TO_PTR(O_PRE_DIA_4));
  hashtable_insert(name_to_element, "EXPLOSION5D", INT_TO_PTR(O_PRE_DIA_5));
  hashtable_insert(name_to_element, "WALL2", INT_TO_PTR(O_STEEL_EXPLODABLE));

  // compatibility with old bd-faq (pre disassembly of bladder)
  hashtable_insert(name_to_element, "BLADDERd9", INT_TO_PTR(O_BLADDER_8));

  // create table to show errors at the start of the application
  gd_create_char_to_element_table();
}

// search the element database for the specified name, and return the element
GdElement gd_get_element_from_string (const char *string)
{
  char *upper = getStringToUpper(string);
  void *value;
  boolean found;

  if (!string)
  {
    Warn("Invalid string representing element: (null)");
    return O_UNKNOWN;
  }

  found = hashtable_exists(name_to_element, upper);	// hash value may be 0
  if (found)
    value = hashtable_search(name_to_element, upper);
  free(upper);
  if (found)
    return (GdElement) (PTR_TO_INT(value));

  Warn("Invalid string representing element: '%s'", string);

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

  // these did not fit into the descriptor array
  for (i = 0; i < 5; i++)
  {
    cave->level_rand[i] = i;
    cave->level_timevalue[i] = i + 1;
  }
}

// for quicksort. compares two highscores.
int gd_highscore_compare(const void *a, const void *b)
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

// return true if score achieved is a highscore
boolean gd_is_highscore(GdHighScore *scores, int score)
{
  // if score is above zero AND bigger than the last one
  if (score > 0 && score > scores[GD_HIGHSCORE_NUM-1].score)
    return TRUE;

  return FALSE;
}

int gd_add_highscore(GdHighScore *highscores, const char *name, int score)
{
  int i;

  if (!gd_is_highscore(highscores, score))
    return -1;

  // overwrite the last one
  gd_strcpy(highscores[GD_HIGHSCORE_NUM-1].name, name);
  highscores[GD_HIGHSCORE_NUM-1].score = score;

  // and sort
  qsort(highscores, GD_HIGHSCORE_NUM, sizeof(GdHighScore), gd_highscore_compare);

  for (i = 0; i < GD_HIGHSCORE_NUM; i++)
    if (strEqual(highscores[i].name, name) && highscores[i].score == score)
      return i;

  return -1;
}

// for the case-insensitive hash keys
int gd_str_case_equal(void *s1, void *s2)
{
  return strcasecmp(s1, s2) == 0;
}

unsigned int gd_str_case_hash(void *v)
{
  char *upper = getStringToUpper(v);
  unsigned int hash = get_hash_from_string(upper);

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

  // hash table which stores unknown tags as strings.
  cave->tags = create_hashtable(gd_str_case_hash, gd_str_case_equal, free, free);

  gd_cave_set_gdash_defaults(cave);

  return cave;
}

/*
  cave maps.
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
void *gd_cave_map_new_for_cave(const GdCave *cave, const int cell_size)
{
  void **rows;                // this is void**, pointer to array of ...
  int y;

  rows = checked_malloc((cave->h) * sizeof(void *));
  rows[0] = checked_calloc(cell_size * cave->w * cave->h);

  for (y = 1; y < cave->h; y++)
    // base pointer + num_of_bytes_per_element * width * number_of_row; as sizeof(char) = 1
    rows[y] = (char *)rows[0] + cell_size * cave->w * y;

  return rows;
}

/*
  duplicate map

  if map is null, this also returns null.
*/
void *gd_cave_map_dup_size(const GdCave *cave, const void *map, const int cell_size)
{
  void **rows;
  void **maplines = (void **)map;
  int y;

  if (!map)
    return NULL;

  rows = checked_malloc((cave->h) * sizeof(void *));
  rows[0] = get_memcpy (maplines[0], cell_size * cave->w * cave->h);

  for (y = 1; y < cave->h; y++)
    rows[y] = (char *)rows[0] + cell_size * cave->w * y;

  return rows;
}

void gd_cave_map_free(void *map)
{
  void **maplines = (void **) map;

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
    hashtable_destroy(cave->tags);

  if (cave->random)    // random generator is a GdRand *
    gd_rand_free(cave->random);

  // free strings
  for (i = 0; gd_cave_properties[i].identifier != NULL; i++)
    if (gd_cave_properties[i].type == GD_TYPE_LONGSTRING)
      checked_free(STRUCT_MEMBER(char *, cave, gd_cave_properties[i].offset));

  // map
  gd_cave_map_free(cave->map);

  // rendered data
  gd_cave_map_free(cave->objects_order);

  // hammered walls to reappear data
  gd_cave_map_free(cave->hammered_reappear);

  // free objects
  list_foreach(cave->objects, (list_fn) free, NULL);
  list_free(cave->objects);

  // free replays
  list_foreach(cave->replays, (list_fn) gd_replay_free, NULL);
  list_free(cave->replays);

  // freeing main pointer
  free (cave);
}

static void hash_copy_foreach(const char *key, const char *value, HashTable *dest)
{
  hashtable_insert(dest, getStringCopy(key), getStringCopy(value));
}

// copy cave from src to destination, with duplicating dynamically allocated data
void gd_cave_copy(GdCave *dest, const GdCave *src)
{
  int i;

  // copy entire data
  memmove(dest, src, sizeof(GdCave));

  // but duplicate dynamic data
  dest->tags = create_hashtable(gd_str_case_hash, gd_str_case_equal, free, free);

  if (src->tags)
    hashtable_foreach(src->tags, (hashtable_fn)hash_copy_foreach, dest->tags);

  dest->map = gd_cave_map_dup(src, map);
  dest->hammered_reappear = gd_cave_map_dup(src, hammered_reappear);

  // for longstrings
  for (i = 0; gd_cave_properties[i].identifier != NULL; i++)
    if (gd_cave_properties[i].type == GD_TYPE_LONGSTRING)
      STRUCT_MEMBER(char *, dest, gd_cave_properties[i].offset) =
	getStringCopy(STRUCT_MEMBER(char *, src, gd_cave_properties[i].offset));

  // no reason to copy this
  dest->objects_order = NULL;

  // copy objects list
  if (src->objects)
  {
    List *iter;

    dest->objects = NULL;    // new empty list
    for (iter = src->objects; iter != NULL; iter = iter->next) // do a deep copy
      dest->objects = list_append(dest->objects, get_memcpy (iter->data, sizeof (GdObject)));
  }

  // copy replays
  if (src->replays)
  {
    List *iter;

    dest->replays = NULL;
    for (iter = src->replays; iter != NULL; iter = iter->next) // do a deep copy
      dest->replays = list_append(dest->replays, gd_replay_new_from_replay(iter->data));
  }

  // copy random number generator
  if (src->random)
    dest->random = gd_rand_copy(src->random);
}

// create new cave, which is a copy of the cave given.
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
  order is a pointer to the GdObject describing this object. Thus the editor can identify
  which cell was created by which object.
*/
void gd_cave_store_rc(GdCave *cave, int x, int y, const GdElement element, const void *order)
{
  // if we do not need to draw, exit now
  if (element == O_NONE)
    return;

  // check bounds
  if (cave->wraparound_objects)
  {
    if (cave->lineshift)
    {
      // fit x coordinate within range, with correcting y at the same time
      while (x < 0)
      {
	x += cave->w;    // out of bounds on the left...
	y--;             // previous row
      }

      while (x >= cave->w)
      {
	x -= cave->w;
	y++;
      }

      // lineshifting does not fix the y coordinates.
      // if out of bounds, element will not be displayed.
      // if such an object appeared in the c64 game, well, it was a buffer overrun.
    }
    else
    {
      // non lineshifting: changing x does not change y coordinate.
      while (x < 0)
	x += cave->w;

      while (x >= cave->w)
	x -= cave->w;

      // after that, fix y coordinate
      while (y < 0)
	y += cave->h;

      while (y >= cave->h)
	y -= cave->h;
    }
  }

  // if the above wraparound code fixed the coordinates, this will always be true.
  // but see the above comment for lineshifting y coordinate
  if (x >= 0 && x < cave->w && y >= 0 && y < cave->h)
  {
    cave->map[y][x] = element;
    cave->objects_order[y][x] = (void *)order;
  }
}

GdElement gd_cave_get_rc(const GdCave *cave, int x, int y)
{
  // always fix coordinates as if cave was wraparound.

  // fix x coordinate
  if (cave->lineshift)
  {
    // fit x coordinate within range, with correcting y at the same time
    while (x < 0)
    {
      x += cave->w;    // out of bounds on the left...
      y--;             // previous row
    }
    while (x >= cave->w)
    {
      x -= cave->w;
      y++;
    }
  }
  else
  {
    // non lineshifting: changing x does not change y coordinate.
    while (x < 0)
      x += cave->w;

    while (x >= cave->w)
      x -= cave->w;
  }

  // after that, fix y coordinate
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

void gd_cave_set_random_c64_colors(GdCave *cave)
{
  const int bright_colors[] = { 1, 3, 7 };
  const int dark_colors[] = { 2, 6, 8, 9, 11 };

  // always black
  cave->colorb = gd_c64_color(0);
  cave->color0 = gd_c64_color(0);

  // choose some bright color for brick
  cave->color3 = gd_c64_color(bright_colors[gd_random_int_range(0, ARRAY_SIZE(bright_colors))]);

  // choose a dark color for dirt, but should not be == color of brick
  do
  {
    cave->color1 = gd_c64_color(dark_colors[gd_random_int_range(0, ARRAY_SIZE(dark_colors))]);
  }
  while (cave->color1 == cave->color3);    // so it is not the same as color 1

  // choose any but black for steel wall, but should not be == brick or dirt
  do
  {
    // between 1 and 15 - do not use black for this.
    cave->color2 = gd_c64_color(gd_random_int_range(1, 16));
  }
  while (cave->color1 == cave->color2 || cave->color2 == cave->color3);    // so colors are not the same

  // copy amoeba and slime color
  cave->color4 = cave->color3;
  cave->color5 = cave->color1;
}

static void cave_set_random_indexed_colors(GdCave *cave, GdColor (*color_indexer_func) (int, int))
{
  int hue = gd_random_int_range(0, 15);
  int hue_spread = gd_random_int_range(1, 6);    // 1..5

  // we only use 0..6, as saturation 15 is too bright (almost always white)
  // also, saturation 0..1..2 is too dark. the color0=black is there for dark.
  // so this is also 1..5. when hue spread is low, brightness spread is high
  int bri_spread = 6 - hue_spread;
  int bri1 = 8, bri2 = 8 - bri_spread, bri3 = 8 + bri_spread;

  // there are 15 valid choices for hue, so we do a %15
  int col1 = hue, col2 = (hue + hue_spread + 15) % 15, col3 = (hue - hue_spread + 15) % 15;

  // this makes up a random color, and selects a color triad by hue+5 and hue+10.
  // also creates a random saturation.
  // color of brick is 8+sat, so it is always a bright color.
  // another two are 8-sat and 8.
  // order of colors is also changed randomly.
  if (gd_random_boolean())    swap(&bri1, &bri2);

  // we do not touch bri3 (8+sat), as it should be a bright color
  if (gd_random_boolean())    swap(&col1, &col2);
  if (gd_random_boolean())    swap(&col2, &col3);
  if (gd_random_boolean())    swap(&col1, &col3);

  cave->colorb = color_indexer_func(0, 0);
  cave->color0 = color_indexer_func(0, 0);
  cave->color1 = color_indexer_func(col1 + 1, bri1);
  cave->color2 = color_indexer_func(col2 + 1, bri2);
  cave->color3 = color_indexer_func(col3 + 1, bri3);
  // amoeba and slime are different
  // some green thing
  cave->color4 = color_indexer_func(gd_random_int_range(11, 13), gd_random_int_range(6, 12));
  // some blueish thing
  cave->color5 = color_indexer_func(gd_random_int_range(7, 10),  gd_random_int_range(0, 6));
}

static void gd_cave_set_random_atari_colors(GdCave *cave)
{
  cave_set_random_indexed_colors(cave, gd_atari_color_huesat);
}

static void gd_cave_set_random_c64dtv_colors(GdCave *cave)
{
  cave_set_random_indexed_colors(cave, gd_c64dtv_color_huesat);
}

static inline void swapd(double *i1, double *i2)
{
  double t = *i1;

  *i1 = *i2;
  *i2 = t;
}

static void gd_cave_set_random_rgb_colors(GdCave *cave)
{
  const double hue_max = 10.0 / 30.0;
  // any hue allowed
  double hue = gd_random_double();
  // hue 360 degress=1.  hue spread is min. 24 degrees, max 120 degrees (1/3)
  double hue_spread = gd_random_double_range(2.0 / 30.0, hue_max);
  double h1 = hue, h2 = hue + hue_spread, h3 = hue + 2 * hue_spread;
  double v1, v2, v3;
  double s1, s2, s3;

  if (gd_random_boolean())
  {
    // when hue spread is low, brightness(saturation) spread is high
    // this formula gives a number (x) between 0.1 and 0.4,
    // which will be 0.5-x and 0.5+x, so the range is 0.1->0.9
    double spread = 0.1 + 0.3 * (1 - hue_spread / hue_max);
    v1 = 0.6;                // brightness variation, too
    v2 = 0.7;
    v3 = 0.8;
    s1 = 0.5;                // saturation is different
    s2 = 0.5 - spread;
    s3 = 0.5 + spread;
  }
  else
  {
    // when hue spread is low, brightness(saturation) spread is high
    // this formula gives a number (x) between 0.1 and 0.25,
    // which will be 0.5+x and 0.5+2x, so the range is 0.5->0.9
    double spread = 0.1 + 0.15 * (1 - hue_spread / hue_max);
    v1 = 0.5;                // brightness is different
    v2 = 0.5 + spread;
    v3 = 0.5 + 2 * spread;
    s1 = 0.7;                // saturation is same - a not fully saturated one
    s2 = 0.8;
    s3 = 0.9;
  }

  // randomly change values, but do not touch v3, as cave->color3 should be a bright color
  if (gd_random_boolean())    swapd(&v1, &v2);

  // randomly change hues and saturations
  if (gd_random_boolean())    swapd(&h1, &h2);
  if (gd_random_boolean())    swapd(&h2, &h3);
  if (gd_random_boolean())    swapd(&h1, &h3);
  if (gd_random_boolean())    swapd(&s1, &s2);
  if (gd_random_boolean())    swapd(&s2, &s3);
  if (gd_random_boolean())    swapd(&s1, &s3);

  h1 = h1 * 360.0;
  h2 = h2 * 360.0;
  h3 = h3 * 360.0;

  cave->colorb = gd_color_get_from_hsv(0, 0, 0);
  cave->color0 = gd_color_get_from_hsv(0, 0, 0);       // black for background
  cave->color1 = gd_color_get_from_hsv(h1, s1, v1);    // dirt
  cave->color2 = gd_color_get_from_hsv(h2, s2, v2);    // steel
  cave->color3 = gd_color_get_from_hsv(h3, s3, v3);    // brick
  // green(120+-20) with the saturation and brightness of brick
  cave->color4 = gd_color_get_from_hsv(gd_random_int_range(100, 140), s2, v2);
  // blue(240+-20) with saturation and brightness of dirt
  cave->color5 = gd_color_get_from_hsv(gd_random_int_range(220, 260), s1, v1);
}

void gd_cave_set_random_colors(GdCave *cave, GdColorType type)
{
  switch (type)
  {
    case GD_COLOR_TYPE_RGB:
      gd_cave_set_random_rgb_colors(cave);
      break;

    case GD_COLOR_TYPE_C64:
      gd_cave_set_random_c64_colors(cave);
      break;

    case GD_COLOR_TYPE_C64DTV:
      gd_cave_set_random_c64dtv_colors(cave);
      break;

    case GD_COLOR_TYPE_ATARI:
      gd_cave_set_random_atari_colors(cave);
      break;

    default:
      break;
  }
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

  // set to maximum size, then try to shrink
  cave->x1 = 0;
  cave->y1 = 0;
  cave->x2 = cave->w - 1;
  cave->y2 = cave->h - 1;

  // search for empty, steel-wall-only last rows.
  // clear all lines, which are only steel wall.
  // and clear only one line, which is steel wall, but also has a player or an outbox.
  empty = STEEL_ONLY;

  do
  {
    for (y = cave->y2 - 1; y <= cave->y2; y++)
    {
      for (x = cave->x1; x <= cave->x2; x++)
      {
	switch (gd_cave_get_rc (cave, x, y))
	{
	  // if steels only, this is to be deleted.
	  case O_STEEL:
	    break;

	  case O_PRE_OUTBOX:
	  case O_PRE_INVIS_OUTBOX:
	  case O_INBOX:
	    if (empty == STEEL_OR_OTHER)
	      empty = NO_SHRINK;

	    // if this, delete only this one, and exit.
	    if (empty == STEEL_ONLY)
	      empty = STEEL_OR_OTHER;
	    break;

	  default:
	    // anything else, that should be left in the cave.
	    empty = NO_SHRINK;
	    break;
	}
      }
    }

    // shrink if full steel or steel and player/outbox.
    if (empty != NO_SHRINK)
      cave->y2--;            // one row shorter
  }
  while (empty == STEEL_ONLY);    // if found just steels, repeat.

  // search for empty, steel-wall-only first rows.
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
	    // shrink only lines, which have only ONE player or outbox.
	    // this is for bd4 intermission 2, for example.
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
  while (empty == STEEL_ONLY);    // if found one, repeat.

  // empty last columns.
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

    // just remember that one column shorter.
    // free will know the size of memchunk, no need to realloc!
    if (empty != NO_SHRINK)
      cave->x2--;
  }
  while (empty == STEEL_ONLY);    // if found one, repeat.

  // empty first columns.
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
  while (empty == STEEL_ONLY);    // if found one, repeat.
}

/*
  check if cave visible part coordinates
  are outside cave sizes, or not in the right order.
  correct them if needed.
*/
void gd_cave_correct_visible_size(GdCave *cave)
{
  // change visible coordinates if they do not point to upperleft and lowerright
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

// Add extra ckdelay to cave by checking the existence of some animated elements.
//
// BD1 and similar engines had animation bits in cave data, to set which elements to animate
// (firefly, butterfly, amoeba).
// animating an element also caused some delay each frame; according to my measurements,
// around 2.6 ms/element.
//
// Also calculate the per iteration ckdelay value, as if we were iterating the cave.
// So when setting up a cave for the first time, update_scheduling() can be called right after
// calling this function, and it will immediately calculate the correct speed of the cave, even
// without iterating it.
static void cave_set_ckdelay_extra_for_animation(GdCave *cave)
{
  int x, y;
  boolean has_amoeba = FALSE, has_firefly = FALSE, has_butterfly = FALSE;

  for (y = 0; y < cave->h; y++)
  {
    for (x = 0; x < cave->w; x++)
    {
      switch (non_scanned_pair(cave->map[y][x]))
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

	default:
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

// Do some init - setup some cave variables before the game.
// Put in a different function, so things which are not
// important for the editor are not done when constructing the cave.
void gd_cave_setup_for_game(GdCave *cave)
{
  int i, x, y;

  // find the player which will be the one to scroll to at the beginning of the game
  // (before the player's birth)
  if (cave->active_is_first_found)
  {
    // uppermost player is active
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
    // lowermost player is active
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

  for (i = 0; i < GD_PLAYER_MEM_SIZE; i++)
  {
    cave->player_x_mem[i] = cave->player_x;
    cave->player_y_mem[i] = cave->player_y;
  }

  // select number of milliseconds (for pal and ntsc)
  cave->timing_factor = cave->pal_timing ? 1200 : 1000;

  // apply timing factor to time values
  cave->time			*= cave->timing_factor;
  cave->magic_wall_time		*= cave->timing_factor;
  cave->amoeba_time		*= cave->timing_factor;
  cave->amoeba_2_time		*= cave->timing_factor;
  cave->hatching_delay_time	*= cave->timing_factor;

  if (cave->hammered_walls_reappear)
    cave->hammered_reappear = gd_cave_map_new(cave, int);

  // set speed
  cave_set_ckdelay_extra_for_animation(cave);
}

// Count diamonds in a cave, and set diamonds_needed accordingly.
// Cave diamonds needed can be set to n <= 0. If so, count the diamonds at the time of the
// hatching, and decrement that value from the number of diamonds found. Of course, this
// function is to be called from the cave engine, at the exact time of hatching.
void gd_cave_count_diamonds(GdCave *cave)
{
  int x, y;

  // if automatically counting diamonds. if this was negative,
  // the sum will be this less than the number of all the diamonds in the cave
  if (cave->diamonds_needed <= 0)
  {
    for (y = 0; y < cave->h; y++)
    {
      for (x = 0; x < cave->w; x++)
      {
        switch (cave->map[y][x])
        {
          case O_DIAMOND:
          case O_DIAMOND_F:
          case O_FLYING_DIAMOND:
          case O_FLYING_DIAMOND_F:
            cave->diamonds_needed++;
            break;

          case O_SKELETON:
            cave->diamonds_needed += cave->skeletons_worth_diamonds;
            break;

          default:
            break;
        }
      }
    }

    // if still below zero, let this be 0, so gate will be open immediately
    if (cave->diamonds_needed < 0)
      cave->diamonds_needed = 0;
  }
}

// Draw a cave into a gfx buffer (integer map) - set the cave cell index from the png.
//
// Takes a cave and a gfx buffer, and fills the buffer with cell indexes.
// The indexes might change if bonus life flash is active (small lines in "SPACE" cells),
// for the paused state (which is used in gdash but not in sdash) - yellowish color.
// Also one can select the animation frame (0..7) to draw the cave on. So the caller manages
// increasing that.
// If a cell is changed, it is flagged with GD_REDRAW; the flag can be cleared by the caller.
//
// @param gfx_buffer A map, which must be the same size as the map of the cave.
// @param bonus_life_flash Set to true, if the player got a bonus life.
//                         The space element will change accordingly.
// @param animcycle Animation cycle - an integer between 0 and 7 to select animated frames.
// @param hate_invisible_outbox Show invisible outboxes as visible (blinking) ones.
void gd_drawcave_game(const GdCave *cave,
		      int **element_buffer, int **last_element_buffer,
		      int **drawing_buffer, int **last_drawing_buffer, int **gfx_buffer,
		      int **covered_buffer,
		      boolean bonus_life_flash, int animcycle, boolean hate_invisible_outbox)
{
  static int player_blinking = 0;
  static int player_tapping = 0;
  int elemmapping[O_MAX_ALL];
  int elemdrawing[O_MAX_ALL];
  int x, y, map, draw;

  if (cave->last_direction)
  {
    // he is moving, so stop blinking and tapping.
    player_blinking = 0;
    player_tapping = 0;
  }
  else
  {
    // he is idle, so animations can be done.
    if (animcycle == 0)
    {
      // blinking and tapping is started at the beginning of animation sequences.
      // 1/4 chance of blinking, every sequence.
      player_blinking = gd_random_int_range(0, 4) == 0;

      // 1/16 chance of starting or stopping tapping.
      if (gd_random_int_range(0, 16) == 0)
	player_tapping = !player_tapping;
    }
  }

  for (x = 0; x < O_MAX_ALL; x++)
  {
    elemmapping[x] = x;
    elemdrawing[x] = gd_element_properties[x].image_game;
  }

  if (bonus_life_flash)
  {
    elemmapping[O_SPACE] = O_FAKE_BONUS;
    elemdrawing[O_SPACE] = gd_element_properties[O_FAKE_BONUS].image_game;
  }

  elemmapping[O_MAGIC_WALL] = (cave->magic_wall_state == GD_MW_ACTIVE ? O_MAGIC_WALL : O_BRICK);
  elemdrawing[O_MAGIC_WALL] = gd_element_properties[cave->magic_wall_state == GD_MW_ACTIVE ? O_MAGIC_WALL : O_BRICK].image_game;

  elemmapping[O_CREATURE_SWITCH] = (cave->creatures_backwards ? O_CREATURE_SWITCH_ON : O_CREATURE_SWITCH);
  elemdrawing[O_CREATURE_SWITCH] = gd_element_properties[cave->creatures_backwards ? O_CREATURE_SWITCH_ON : O_CREATURE_SWITCH].image_game;

  elemmapping[O_EXPANDING_WALL_SWITCH] = (cave->expanding_wall_changed ? O_EXPANDING_WALL_SWITCH_VERT : O_EXPANDING_WALL_SWITCH_HORIZ);
  elemdrawing[O_EXPANDING_WALL_SWITCH] = gd_element_properties[cave->expanding_wall_changed ? O_EXPANDING_WALL_SWITCH_VERT : O_EXPANDING_WALL_SWITCH_HORIZ].image_game;

  elemmapping[O_GRAVITY_SWITCH] = (cave->gravity_switch_active ? O_GRAVITY_SWITCH_ACTIVE : O_GRAVITY_SWITCH);
  elemdrawing[O_GRAVITY_SWITCH] = gd_element_properties[cave->gravity_switch_active ? O_GRAVITY_SWITCH_ACTIVE : O_GRAVITY_SWITCH].image_game;

  elemmapping[O_REPLICATOR_SWITCH] = (cave->replicators_active ? O_REPLICATOR_SWITCH_ON : O_REPLICATOR_SWITCH_OFF);
  elemdrawing[O_REPLICATOR_SWITCH] = gd_element_properties[cave->replicators_active ? O_REPLICATOR_SWITCH_ON : O_REPLICATOR_SWITCH_OFF].image_game;

  if (cave->replicators_active)
    // if the replicators are active, animate them.
    elemmapping[O_REPLICATOR] = O_REPLICATOR_ACTIVE;

  if (!cave->replicators_active)
    // if the replicators are inactive, do not animate them.
    elemdrawing[O_REPLICATOR] = ABS(elemdrawing[O_REPLICATOR]);

  elemmapping[O_CONVEYOR_SWITCH] = (cave->conveyor_belts_active ? O_CONVEYOR_SWITCH_ON : O_CONVEYOR_SWITCH_OFF);
  elemdrawing[O_CONVEYOR_SWITCH] = gd_element_properties[cave->conveyor_belts_active ? O_CONVEYOR_SWITCH_ON : O_CONVEYOR_SWITCH_OFF].image_game;

  if (cave->conveyor_belts_direction_changed)
  {
    // if direction is changed, animation is changed.
    int temp;

    elemmapping[O_CONVEYOR_LEFT] = O_CONVEYOR_RIGHT;
    elemmapping[O_CONVEYOR_RIGHT] = O_CONVEYOR_LEFT;

    temp = elemdrawing[O_CONVEYOR_LEFT];
    elemdrawing[O_CONVEYOR_LEFT] = elemdrawing[O_CONVEYOR_RIGHT];
    elemdrawing[O_CONVEYOR_RIGHT] = temp;

    elemmapping[O_CONVEYOR_DIR_SWITCH] = O_CONVEYOR_DIR_CHANGED;
    elemdrawing[O_CONVEYOR_DIR_SWITCH] = gd_element_properties[O_CONVEYOR_DIR_CHANGED].image_game;
  }
  else
  {
    elemmapping[O_CONVEYOR_DIR_SWITCH] = O_CONVEYOR_DIR_NORMAL;
    elemdrawing[O_CONVEYOR_DIR_SWITCH] = gd_element_properties[O_CONVEYOR_DIR_NORMAL].image_game;
  }

  if (cave->conveyor_belts_active)
  {
    // keep potentially changed direction
    int offset = (O_CONVEYOR_LEFT_ACTIVE - O_CONVEYOR_LEFT);

    // if they are running, animate them.
    elemmapping[O_CONVEYOR_LEFT]  += offset;
    elemmapping[O_CONVEYOR_RIGHT] += offset;
  }
  if (!cave->conveyor_belts_active)
  {
    // if they are not running, do not animate them.
    elemdrawing[O_CONVEYOR_LEFT] = ABS(elemdrawing[O_CONVEYOR_LEFT]);
    elemdrawing[O_CONVEYOR_RIGHT] = ABS(elemdrawing[O_CONVEYOR_RIGHT]);
  }

  if (animcycle & 2)
  {
    // also a hack, like biter_switch
    elemdrawing[O_PNEUMATIC_ACTIVE_LEFT]  += 2;
    elemdrawing[O_PNEUMATIC_ACTIVE_RIGHT] += 2;
    elemdrawing[O_PLAYER_PNEUMATIC_LEFT]  += 2;
    elemdrawing[O_PLAYER_PNEUMATIC_RIGHT] += 2;
  }

  if ((cave->last_direction) == GD_MV_STILL)
  {
    // player is idle.
    if (player_blinking && player_tapping)
    {
      map = O_PLAYER_TAP_BLINK;
      draw = gd_element_properties[O_PLAYER_TAP_BLINK].image_game;
    }
    else if (player_blinking)
    {
      map = O_PLAYER_BLINK;
      draw = gd_element_properties[O_PLAYER_BLINK].image_game;
    }
    else if (player_tapping)
    {
      map = O_PLAYER_TAP;
      draw = gd_element_properties[O_PLAYER_TAP].image_game;
    }
    else
    {
      map = O_PLAYER;
      draw = gd_element_properties[O_PLAYER].image_game;
    }
  }
  else if (cave->last_direction == GD_MV_UP && use_bd_up_down_graphics())
  {
    map = O_PLAYER_UP;
    draw = gd_element_properties[O_PLAYER_UP].image_game;
  }
  else if (cave->last_direction == GD_MV_DOWN && use_bd_up_down_graphics())
  {
    map = O_PLAYER_DOWN;
    draw = gd_element_properties[O_PLAYER_DOWN].image_game;
  }
  else if (cave->last_horizontal_direction == GD_MV_LEFT)
  {
    map = O_PLAYER_LEFT;
    draw = gd_element_properties[O_PLAYER_LEFT].image_game;
  }
  else
  {
    // of course this is GD_MV_RIGHT.
    map = O_PLAYER_RIGHT;
    draw = gd_element_properties[O_PLAYER_RIGHT].image_game;
  }

  elemmapping[O_PLAYER] = map;
  elemmapping[O_PLAYER_GLUED] = map;

  elemdrawing[O_PLAYER] = draw;
  elemdrawing[O_PLAYER_GLUED] = draw;

  // player with bomb/rocketlauncher does not blink or tap - no graphics drawn for that.
  // running is drawn using w/o bomb/rocketlauncher cells */
  if (cave->last_direction != GD_MV_STILL)
  {
    elemmapping[O_PLAYER_BOMB] = map;
    elemdrawing[O_PLAYER_BOMB] = draw;

    elemmapping[O_PLAYER_ROCKET_LAUNCHER] = map;
    elemdrawing[O_PLAYER_ROCKET_LAUNCHER] = draw;
  }

  elemmapping[O_INBOX] = (cave->inbox_flash_toggle ? O_INBOX_OPEN : O_INBOX_CLOSED);
  elemdrawing[O_INBOX] = gd_element_properties[cave->inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED].image_game;

  elemmapping[O_OUTBOX] = (cave->inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED);
  elemdrawing[O_OUTBOX] = gd_element_properties[cave->inbox_flash_toggle ? O_OUTBOX_OPEN : O_OUTBOX_CLOSED].image_game;

  // hack, not fit into gd_element_properties
  elemmapping[O_BITER_SWITCH] = O_BITER_SWITCH_1 + cave->biter_delay_frame;
  // hack, not fit into gd_element_properties
  elemdrawing[O_BITER_SWITCH] = gd_element_properties[O_BITER_SWITCH].image_game + cave->biter_delay_frame;

  // visual effects
  elemmapping[O_DIRT] = cave->dirt_looks_like;
  elemmapping[O_EXPANDING_WALL] = cave->expanding_wall_looks_like;
  elemmapping[O_V_EXPANDING_WALL] = cave->expanding_wall_looks_like;
  elemmapping[O_H_EXPANDING_WALL] = cave->expanding_wall_looks_like;
  elemmapping[O_AMOEBA_2] = cave->amoeba_2_looks_like;

  // visual effects
  elemdrawing[O_DIRT] = elemdrawing[cave->dirt_looks_like];
  elemdrawing[O_EXPANDING_WALL] = elemdrawing[cave->expanding_wall_looks_like];
  elemdrawing[O_V_EXPANDING_WALL] = elemdrawing[cave->expanding_wall_looks_like];
  elemdrawing[O_H_EXPANDING_WALL] = elemdrawing[cave->expanding_wall_looks_like];
  elemdrawing[O_AMOEBA_2] = elemdrawing[cave->amoeba_2_looks_like];

  // change only graphically
  if (hate_invisible_outbox)
  {
    elemmapping[O_PRE_INVIS_OUTBOX] = elemmapping[O_PRE_OUTBOX];
    elemmapping[O_INVIS_OUTBOX] = elemmapping[O_OUTBOX];
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

      // if covered, real element is not important
      if (covered_buffer[y][x])
	map = O_COVERED;
      else
	map = elemmapping[actual];

      // if covered, real element is not important
      if (covered_buffer[y][x])
	draw = gd_element_properties[O_COVERED].image_game;
      else
	draw = elemdrawing[actual];

      // draw special graphics if player is pushing something
      if (use_bd_pushing_graphics() &&
	  (cave->last_direction == GD_MV_LEFT || cave->last_direction == GD_MV_RIGHT) &&
	  is_player(cave, x, y) && can_be_pushed(cave, x, y, cave->last_direction))
      {
	// special check needed when smooth game element movements selected in setup menu:
	// last element must either be player (before pushing) or pushable element (while pushing)
	// (extra check needed to prevent pushing animation when moving towards pushable element)
	if (!use_bd_smooth_movements() || last_element_buffer[y][x] != O_SPACE)
	{
	  if (cave->last_direction == GD_MV_LEFT)
	    map = O_PLAYER_PUSH_LEFT;
	  else
	    map = O_PLAYER_PUSH_RIGHT;

	  if (cave->last_direction == GD_MV_LEFT)
	    draw = elemdrawing[O_PLAYER_PUSH_LEFT];
	  else
	    draw = elemdrawing[O_PLAYER_PUSH_RIGHT];
	}
      }

      // if negative, animated.
      if (draw < 0)
	draw = -draw + animcycle;

      // flash
      if (cave->gate_open_flash)
	draw += GD_NUM_OF_CELLS;

      // set to buffer, with caching
      if (element_buffer[y][x] != actual)
	element_buffer[y][x] = actual;

      if (drawing_buffer[y][x] != map)
	drawing_buffer[y][x] = map;

      if (gfx_buffer[y][x] != draw)
	gfx_buffer[y][x] = draw | GD_REDRAW;
    }
  }
}

/*
  cave time is rounded _UP_ to seconds. so at the exact moment when it
  changes from
  2sec remaining to 1sec remaining, the player has exactly one second.
  when it changes
  to zero, it is the exact moment of timeout.

  internal time is milliseconds (or 1200 milliseconds for pal timing).
*/
int gd_cave_time_show(const GdCave *cave, int internal_time)
{
  return (internal_time + cave->timing_factor - 1) / cave->timing_factor;
}

GdReplay *gd_replay_new(void)
{
  GdReplay *rep;

  rep = checked_calloc(sizeof(GdReplay));
  rep->movements = checked_calloc(sizeof(GdReplayMovements));

  return rep;
}

GdReplay *gd_replay_new_from_replay(GdReplay *orig)
{
  GdReplay *rep;

  rep = get_memcpy(orig, sizeof(GdReplay));

  // replicate dynamic data
  rep->comment = getStringCopy(orig->comment);
  rep->movements = get_memcpy(orig->movements, sizeof(GdReplayMovements));

  return rep;
}

void gd_replay_free(GdReplay *replay)
{
  checked_free(replay->movements);
  checked_free(replay->comment);
  free(replay);
}

// store movement in a replay
void gd_replay_store_movement(GdReplay *replay, GdDirection player_move,
			      boolean player_fire, boolean suicide)
{
  byte data[1];

  data[0] = ((player_move) |
	     (player_fire ? GD_REPLAY_FIRE_MASK : 0) |
	     (suicide ? GD_REPLAY_SUICIDE_MASK : 0));

  if (replay->movements->len < MAX_REPLAY_LEN)
  {
    replay->movements->data[replay->movements->len++] = data[0];

    if (replay->movements->len == MAX_REPLAY_LEN)
      Warn("BD replay truncated: size exceeds maximum replay size %d", MAX_REPLAY_LEN);
  }
}

// calculate adler checksum for a rendered cave; this can be used for more caves.
void gd_cave_adler_checksum_more(GdCave *cave, unsigned int *a, unsigned int *b)
{
  int x, y;

  for (y = 0; y < cave->h; y++)
    for (x = 0; x < cave->w; x++)
    {
      *a += gd_element_properties[cave->map[y][x]].character;
      *b += *a;

      *a %= 65521;
      *b %= 65521;
    }
}

// calculate adler checksum for a single rendered cave.
unsigned int gd_cave_adler_checksum(GdCave *cave)
{
  unsigned int a = 1;
  unsigned int b = 0;

  gd_cave_adler_checksum_more(cave, &a, &b);
  return (b << 16) + a;
}

boolean gd_cave_has_levels(GdCave *cave)
{
  GdCave c = *cave;
  int *cave_level_value[] =
  {
    c.level_diamonds,
    c.level_speed,
    c.level_ckdelay,
    c.level_time,
    c.level_magic_wall_time,
    c.level_amoeba_time,
    c.level_amoeba_threshold,
    c.level_amoeba_2_time,
    c.level_amoeba_2_threshold,
    c.level_slime_permeability,
    c.level_slime_permeability_c64,
    c.level_slime_seed_c64,
    c.level_hatching_delay_frame,
    c.level_hatching_delay_time,
    c.level_bonus_time,
    c.level_penalty_time,

    NULL
  };
  int i, j;

  for (i = 0; cave_level_value[i] != NULL; i++)
    for (j = 1; j < 5; j++)
      if (cave_level_value[i][j] != cave_level_value[i][0])
	return TRUE;

  for (j = 1; j < 5; j++)
    if (cave->level_rand[j] != j &&
	cave->level_rand[j - 1] != j - 1 &&
	cave->level_rand[j] != cave->level_rand[0])
      return TRUE;

  for (j = 1; j < 5; j++)
    if (cave->level_timevalue[j] != j + 1 &&
	cave->level_timevalue[j - 1] != j &&
	cave->level_timevalue[j] != cave->level_timevalue[0])
      return TRUE;

  return FALSE;
}

boolean gd_caveset_has_levels(void)
{
  List *iter;

  for (iter = gd_caveset; iter != NULL; iter = iter->next)
    if (gd_cave_has_levels((GdCave *)iter->data))
      return TRUE;

  return FALSE;
}

// set all elements in a cave to their non-scanned counterparts
void unscan_cave(GdCave *cave)
{
  if (cave == NULL || cave->map == NULL)
    return;

  int x, y;

  for (y = 0; y < cave->h; y++)
    for (x = 0; x < cave->w; x++)
      cave->map[y][x] = non_scanned_pair(cave->map[y][x]);

  cave->snap_element			= non_scanned_pair(cave->snap_element);
  cave->magic_diamond_to		= non_scanned_pair(cave->magic_diamond_to);
  cave->magic_stone_to			= non_scanned_pair(cave->magic_stone_to);
  cave->magic_mega_stone_to		= non_scanned_pair(cave->magic_mega_stone_to);
  cave->magic_nut_to			= non_scanned_pair(cave->magic_nut_to);
  cave->magic_nitro_pack_to		= non_scanned_pair(cave->magic_nitro_pack_to);
  cave->magic_flying_diamond_to		= non_scanned_pair(cave->magic_flying_diamond_to);
  cave->magic_flying_stone_to		= non_scanned_pair(cave->magic_flying_stone_to);
  cave->amoeba_too_big_effect		= non_scanned_pair(cave->amoeba_too_big_effect);
  cave->amoeba_enclosed_effect		= non_scanned_pair(cave->amoeba_enclosed_effect);
  cave->amoeba_2_too_big_effect		= non_scanned_pair(cave->amoeba_2_too_big_effect);
  cave->amoeba_2_enclosed_effect	= non_scanned_pair(cave->amoeba_2_enclosed_effect);
  cave->amoeba_2_explosion_effect	= non_scanned_pair(cave->amoeba_2_explosion_effect);
  cave->amoeba_2_looks_like		= non_scanned_pair(cave->amoeba_2_looks_like);
  cave->slime_eats_1			= non_scanned_pair(cave->slime_eats_1);
  cave->slime_converts_1		= non_scanned_pair(cave->slime_converts_1);
  cave->slime_eats_2			= non_scanned_pair(cave->slime_eats_2);
  cave->slime_converts_2		= non_scanned_pair(cave->slime_converts_2);
  cave->slime_eats_3			= non_scanned_pair(cave->slime_eats_3);
  cave->slime_converts_3		= non_scanned_pair(cave->slime_converts_3);
  cave->acid_eats_this			= non_scanned_pair(cave->acid_eats_this);
  cave->acid_turns_to			= non_scanned_pair(cave->acid_turns_to);
  cave->biter_eat			= non_scanned_pair(cave->biter_eat);
  cave->bladder_converts_by		= non_scanned_pair(cave->bladder_converts_by);
  cave->nut_turns_to_when_crushed	= non_scanned_pair(cave->nut_turns_to_when_crushed);
  cave->expanding_wall_looks_like	= non_scanned_pair(cave->expanding_wall_looks_like);
  cave->dirt_looks_like			= non_scanned_pair(cave->dirt_looks_like);
  cave->stone_falling_effect		= non_scanned_pair(cave->stone_falling_effect);
  cave->stone_bouncing_effect		= non_scanned_pair(cave->stone_bouncing_effect);
  cave->diamond_falling_effect		= non_scanned_pair(cave->diamond_falling_effect);
  cave->diamond_bouncing_effect		= non_scanned_pair(cave->diamond_bouncing_effect);
  cave->firefly_explode_to		= non_scanned_pair(cave->firefly_explode_to);
  cave->alt_firefly_explode_to		= non_scanned_pair(cave->alt_firefly_explode_to);
  cave->butterfly_explode_to		= non_scanned_pair(cave->butterfly_explode_to);
  cave->alt_butterfly_explode_to	= non_scanned_pair(cave->alt_butterfly_explode_to);
  cave->stonefly_explode_to		= non_scanned_pair(cave->stonefly_explode_to);
  cave->dragonfly_explode_to		= non_scanned_pair(cave->dragonfly_explode_to);
  cave->diamond_birth_effect		= non_scanned_pair(cave->diamond_birth_effect);
  cave->bomb_explosion_effect		= non_scanned_pair(cave->bomb_explosion_effect);
  cave->nitro_explosion_effect		= non_scanned_pair(cave->nitro_explosion_effect);
  cave->explosion_effect		= non_scanned_pair(cave->explosion_effect);
  cave->explosion_3_effect		= non_scanned_pair(cave->explosion_3_effect);
}
