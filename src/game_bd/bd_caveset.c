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

#include <sys/stat.h>

#include "main_bd.h"


// this stores the caves.
List *gd_caveset;

// the data of the caveset: name, highscore, max number of lives, etc.
GdCavesetData *gd_caveset_data;

// is set to true, when the caveset was edited since the last save.
boolean gd_caveset_edited;

// last selected-to-play cave
int gd_caveset_last_selected;
int gd_caveset_last_selected_level;

// list of possible extensions which can be opened
char *gd_caveset_extensions[] =
{
  "*.gds",
  "*.bd",
  "*.bdr",
  "*.brc",

  NULL
};

#define CAVESET_OFFSET(property) (STRUCT_OFFSET(GdCavesetData, property))

const GdStructDescriptor gd_caveset_properties[] =
{
  // default data
  {
    "", GD_TAB, 0,
    N_("Caveset data")
  },
  {
    "Name", GD_TYPE_STRING, 0,
    N_("Name"), CAVESET_OFFSET(name), 1,
    N_("Name of the game")
  },
  {
    "Description", GD_TYPE_STRING, 0,
    N_("Description"), CAVESET_OFFSET(description), 1,
    N_("Some words about the game")
  },
  {
    "Author", GD_TYPE_STRING, 0,
    N_("Author"), CAVESET_OFFSET(author), 1,
    N_("Name of author")
  },
  {
    "Date", GD_TYPE_STRING, 0,
    N_("Date"), CAVESET_OFFSET(date), 1,
    N_("Date of creation")
  },
  {
    "WWW", GD_TYPE_STRING, 0,
    N_("WWW"), CAVESET_OFFSET(www), 1,
    N_("Web page or e-mail address")
  },
  {
    "Difficulty", GD_TYPE_STRING, 0,
    N_("Difficulty"), CAVESET_OFFSET(difficulty), 1,
    N_("Difficulty (informative)")
  },
  {
    "Lives", GD_TYPE_INT, 0,
    N_("Initial lives"), CAVESET_OFFSET(initial_lives), 1,
    N_("Number of lives you get at game start."), 3, 9
  },
  {
    "Lives", GD_TYPE_INT, 0,
    N_("Maximum lives"), CAVESET_OFFSET(maximum_lives), 1,
    N_("Maximum number of lives you can have by collecting bonus points."), 3, 99
  },
  {
    "BonusLife", GD_TYPE_INT, 0,
    N_("Bonus life score"), CAVESET_OFFSET(bonus_life_score), 1,
    N_("Number of points to collect for a bonus life."), 100, 5000
  },
  {
    "Story", GD_TYPE_LONGSTRING, 0,
    N_("Story"), CAVESET_OFFSET(story), 1,
    N_("Long description of the game.")
  },
  {
    "Remark", GD_TYPE_LONGSTRING, 0,
    N_("Remark"), CAVESET_OFFSET(remark), 1,
    N_("Remark (informative).")
  },
  {
    "TitleScreen", GD_TYPE_LONGSTRING, GD_DONT_SHOW_IN_EDITOR,
    N_("Title screen"), CAVESET_OFFSET(title_screen), 1,
    N_("Title screen image")
  },
  {
    "TitleScreenScroll", GD_TYPE_LONGSTRING, GD_DONT_SHOW_IN_EDITOR,
    N_("Title screen, scrolling"), CAVESET_OFFSET(title_screen_scroll), 1,
    N_("Scrolling background for title screen image")
  },

  {
    NULL
  },
};

static GdPropertyDefault caveset_defaults[] =
{
  // default data
  { CAVESET_OFFSET(initial_lives),	3	},
  { CAVESET_OFFSET(maximum_lives),	9	},
  { CAVESET_OFFSET(bonus_life_score),	500	},

  { -1 },
};

GdCavesetData *gd_caveset_data_new(void)
{
  GdCavesetData *data;

  data = checked_calloc(sizeof(GdCavesetData));

  gd_struct_set_defaults_from_array(data, gd_caveset_properties, caveset_defaults);

  if (leveldir_current != NULL)
    data->levelset_subdir = getStringCopy(leveldir_current->subdir);

  return data;
}

void gd_caveset_data_free(GdCavesetData *data)
{
  int i;

  // free strings
  for (i = 0; gd_caveset_properties[i].identifier != NULL; i++)
    if (gd_caveset_properties[i].type == GD_TYPE_LONGSTRING)
      checked_free(STRUCT_MEMBER(char *, data, gd_caveset_properties[i].offset));

  checked_free(data->levelset_subdir);

  checked_free(data);
}

// ============================================================================
// Misc caveset functions
// ============================================================================

// Clears all caves in the caveset. also to be called at application start
void gd_caveset_clear(void)
{
  if (gd_caveset)
  {
    list_foreach(gd_caveset, (list_fn) gd_cave_free, NULL);
    list_free(gd_caveset);
    gd_caveset = NULL;
  }

  if (gd_caveset_data)
  {
    gd_caveset_data_free(gd_caveset_data);
    gd_caveset_data = NULL;
  }

  // always newly create this
  // create pseudo cave containing default values
  gd_caveset_data = gd_caveset_data_new();

  if (leveldir_current != NULL)
    gd_strcpy(gd_caveset_data->name, leveldir_current->name);
}

// return number of caves currently in memory.
int gd_caveset_count(void)
{
  return list_length(gd_caveset);
}

// return index of first selectable cave
static int caveset_first_selectable_cave_index(void)
{
  List *iter;
  int i;

  for (i = 0, iter = gd_caveset; iter != NULL; i++, iter = iter->next)
  {
    GdCave *cave = (GdCave *)iter->data;

    if (cave->selectable)
      return i;
  }

  Warn("no selectable cave in caveset!");

  // and return the first one.
  return 0;
}

// return a cave identified by its index
GdCave *gd_return_nth_cave(const int cave)
{
  return list_nth_data(gd_caveset, cave);
}

// get a selected cave from the loaded caveset (original, unmodified cave)
GdCave *gd_get_original_cave_from_caveset(const int cave)
{
  // get specified cave from caveset already stored in memory
  GdCave *original_cave = gd_return_nth_cave(cave);

  return original_cave;
}

// get a selected cave from the loaded caveset (cave prepared for playing)
GdCave *gd_get_prepared_cave_from_caveset(const int cave, const int level)
{
  // get specified cave from caveset already stored in memory
  GdCave *original_cave = gd_return_nth_cave(cave);

  // get prepared cave from original cave
  GdCave *prepared_cave = gd_get_prepared_cave(original_cave, level);

  return prepared_cave;
}

// get a cave prepared for playing from a given original, unmodified cave (with seed)
GdCave *gd_get_prepared_cave(const GdCave *original_cave, const int level)
{
  // get rendered cave using the selected seed for playing
  GdCave *prepared_cave = gd_cave_new_rendered(original_cave, level, game_bd.random_seed);

  // initialize some cave variables (like player position)
  gd_cave_setup_for_game(prepared_cave);

  return prepared_cave;
}

// colors: 4: purple  3: ciklamen 2: orange 1: blue 0: green

static GdElement brc_import_table[] =
{
  /* 0 */
  O_SPACE, O_DIRT, O_BRICK, O_MAGIC_WALL,
  O_PRE_OUTBOX, O_OUTBOX, O_UNKNOWN, O_STEEL,
  O_H_EXPANDING_WALL, O_H_EXPANDING_WALL_scanned, O_FIREFLY_1_scanned, O_FIREFLY_1_scanned,
  O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4,

  /* 1 */
  O_BUTTER_1_scanned, O_BUTTER_1_scanned, O_BUTTER_1, O_BUTTER_2,
  O_BUTTER_3, O_BUTTER_4, O_PLAYER, O_PLAYER_scanned,
  O_STONE, O_STONE_scanned, O_STONE_F, O_STONE_F_scanned,
  O_DIAMOND, O_DIAMOND_scanned, O_DIAMOND_F, O_DIAMOND_F_scanned,

  /* 2 */
  O_NONE /* WILL_EXPLODE_THING */, O_EXPLODE_1, O_EXPLODE_2, O_EXPLODE_3,
  O_EXPLODE_4, O_EXPLODE_5, O_NONE /* WILL EXPLODE TO DIAMOND_THING */, O_PRE_DIA_1,
  O_PRE_DIA_2, O_PRE_DIA_3, O_PRE_DIA_4, O_PRE_DIA_5,
  O_AMOEBA, O_AMOEBA_scanned, O_SLIME, O_NONE,

  /* 3 */
  O_CLOCK, O_NONE /* clock eaten */, O_INBOX, O_PRE_PL_1,
  O_PRE_PL_2, O_PRE_PL_3, O_NONE, O_NONE,
  O_NONE, O_NONE, O_V_EXPANDING_WALL, O_NONE,
  O_VOODOO, O_UNKNOWN, O_EXPANDING_WALL, O_EXPANDING_WALL_scanned,

  /* 4 */
  O_FALLING_WALL, O_FALLING_WALL_F, O_FALLING_WALL_F_scanned, O_UNKNOWN,
  O_ACID, O_ACID_scanned, O_NITRO_PACK, O_NITRO_PACK_scanned,
  O_NITRO_PACK_F, O_NITRO_PACK_F_scanned, O_NONE, O_NONE,
  O_NONE, O_NONE, O_NONE, O_NONE,

  /* 5 */
  O_NONE /* bomb explosion utolso */, O_UNKNOWN, O_NONE /* solid bomb glued */, O_UNKNOWN,
  O_STONE_GLUED, O_UNKNOWN, O_DIAMOND_GLUED, O_UNKNOWN,
  O_UNKNOWN, O_UNKNOWN, O_NONE, O_NONE,
  O_NONE, O_NONE, O_NONE, O_NONE,

  /* 6 */
  O_ALT_FIREFLY_1_scanned, O_ALT_FIREFLY_1_scanned, O_ALT_FIREFLY_1, O_ALT_FIREFLY_2,
  O_ALT_FIREFLY_3, O_ALT_FIREFLY_4, O_PLAYER_BOMB, O_PLAYER_BOMB_scanned,
  O_BOMB, O_BOMB_TICK_1, O_BOMB_TICK_2, O_BOMB_TICK_3,
  O_BOMB_TICK_4, O_BOMB_TICK_5, O_BOMB_TICK_6, O_BOMB_TICK_7,

  /* 7 */
  O_BOMB_TICK_7, O_BOMB_EXPL_1, O_BOMB_EXPL_2, O_BOMB_EXPL_3,
  O_BOMB_EXPL_4, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
  O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN,
};

static GdElement brc_effect_table[] =
{
  O_STEEL, O_DIRT, O_SPACE, O_STONE, O_STONE_F,
  O_STONE_GLUED, O_DIAMOND, O_DIAMOND_F, O_DIAMOND_GLUED, O_PRE_DIA_1,
  O_PLAYER, O_PRE_PL_1, O_PLAYER_BOMB, O_PRE_OUTBOX, O_OUTBOX,
  O_FIREFLY_1, O_FIREFLY_2, O_FIREFLY_3, O_FIREFLY_4, O_BUTTER_1,
  O_BUTTER_2, O_BUTTER_3, O_BUTTER_4, O_BRICK, O_MAGIC_WALL,
  O_H_EXPANDING_WALL, O_V_EXPANDING_WALL, O_EXPANDING_WALL, O_FALLING_WALL, O_FALLING_WALL_F,
  O_AMOEBA, O_SLIME, O_ACID, O_VOODOO, O_CLOCK,
  O_BOMB, O_UNKNOWN, O_UNKNOWN, O_UNKNOWN, O_ALT_FIREFLY_1,
  O_ALT_FIREFLY_2, O_ALT_FIREFLY_3, O_ALT_FIREFLY_4, O_ALT_BUTTER_1, O_ALT_BUTTER_2,
  O_ALT_BUTTER_3, O_ALT_BUTTER_4, O_EXPLODE_1, O_BOMB_EXPL_1, O_UNKNOWN,
};

static GdColor brc_color_table[] =
{
  0x518722, 0x3a96fa, 0xdb7618, 0xff3968,
  0x9b5fff, 0x0ee06c, 0xc25ea6, 0xf54826,
  0xf1ff26,
};

static GdColor brc_color_table_comp[] =
{
  0x582287, 0xfa9d39, 0x187ddb, 0x38ffd1,
  0xc1ff5e, 0xe00d81, 0x5dc27a, 0x27d3f5,
  0x3526ff,
};

static GdElement brc_effect(byte byt)
{
  if (byt >= ARRAY_SIZE(brc_effect_table))
  {
    Warn("invalid element identifier for brc effect: %02x", byt);

    return O_UNKNOWN;
  }

  return brc_effect_table[byt];
}

static GdElement brc_import_elem(unsigned char c)
{
  if (c >= ARRAY_SIZE(brc_import_table))
  {
    Warn("invalid brc element byte %x", c);

    return O_UNKNOWN;
  }

  return non_scanned_pair(brc_import_table[c]);
}

static void brc_import(byte *data)
{
  int x, y;
  int level;

  // we import 100 caves, and the put them in the correct order.
  GdCave *imported[100];
  boolean import_effect;

  gd_caveset_clear();

  // this is some kind of a version number
  import_effect = FALSE;

  switch (data[23])
  {
    case 0x0:
      // nothing to do
      break;

    case 0xde:
      // import effects
      import_effect = TRUE;
      break;

    default:
      Warn("unknown brc version %02x", data[23]);
      break;
  }

  for (level = 0; level < 5; level++)
  {
    int cavenum;
    int i;

    for (cavenum = 0; cavenum < 20; cavenum++)
    {
      GdCave *cave;

      // 5 levels, 20 caves, 24 bytes - max 40*2 properties for each cave
      int c = 5 * 20 * 24;

      int datapos = (cavenum * 5 + level) * 24 + 22;
      int colind;

      cave = gd_cave_new();
      imported[level * 20 + cavenum] = cave;

      if (cavenum < 16)
	snprintf(cave->name, sizeof(GdString), "Cave %c/%d", 'A' + cavenum, level + 1);
      else
	snprintf(cave->name, sizeof(GdString), "Intermission %d/%d", cavenum - 15, level + 1);

      // fixed intermission caves; are smaller.
      if (cavenum >= 16)
      {
	cave->w = 20;
	cave->h = 12;
      }

      cave->map = gd_cave_map_new(cave, GdElement);

      for (y = 0; y < cave->h; y++)
      {
	for (x = 0; x < cave->w; x++)
	{
	  byte import = data[y + level * 24 + cavenum * 24 * 5 + x * 24 * 5 * 20];

          cave->map[y][x] = brc_import_elem(import);
	}
      }

      for (i = 0; i < 5; i++)
      {
	cave->level_time[i]             = data[0 * c + datapos];
	cave->level_diamonds[i]         = data[1 * c + datapos];
	cave->level_magic_wall_time[i]  = data[4 * c + datapos];
	cave->level_amoeba_time[i]      = data[5 * c + datapos];
	cave->level_amoeba_threshold[i] = data[6 * c + datapos];

	// bonus time: 100 was added, so it could also be negative
	cave->level_bonus_time[i]      = (int)data[11 * c + datapos + 1] - 100;
	cave->level_hatching_delay_frame[i] = data[10 * c + datapos];

        if (data[9 * c + datapos] != 0)
          cave->level_slime_permeability[i] = 1E6 / data[9 * c + datapos] + 0.5; // 0.5 for rounding
        else
          Warn("slime permeability cannot be zero, error at byte %d", 9 * c + datapos);

	// this was not set in boulder remake.
	cave->level_speed[i] = 150;
      }

      cave->diamond_value = data[2 * c + datapos];
      cave->extra_diamond_value = data[3 * c +datapos];

      // BRC PROBABILITIES
      /*
	a typical code example:
	46:if (random(slime*4)<4) and (tab[x,y+2] = 0) then
	Begin tab[x,y]:=0;col[x,y+2]:=col[x,y];tab[x,y+2]:=27;mat[x,y+2]:=9;Voice4:=2;end;
	where slime is the byte loaded from the file as it is.
	pascal random function generates a random number between 0..limit-1,
	inclusive, for random(limit).

	so a random number between 0..limit*4-1 is generated.
	for limit=1, 0..3, which is always < 4, so P=1.
	for limit=2, 0..7, 0..7 is < 4 in P=50%.
	for limit=3, 0..11, is < 4 in P=33%.
	So the probability is exactly 100%/limit.
	just make sure we do not divide by zero for some broken input.
      */

      if (data[7 * c + datapos] != 0)
	cave->amoeba_growth_prob = 1E6 / data[7 * c + datapos] + 0.5; // 0.5 for rounding
      else
	Warn("amoeba growth cannot be zero, error at byte %d", 7 * c + datapos);

      if (data[8 * c + datapos] != 0)
	cave->amoeba_fast_growth_prob = 1E6 / data[8 * c + datapos] + 0.5; // 0.5 for rounding
      else
	Warn("amoeba fast growth cannot be zero, error at byte %d", 8 * c + datapos);

      cave->slime_predictable = FALSE;

      // probability -> *1E6
      if (data[10 * c + datapos] != 0)
        cave->acid_spread_ratio = 1E6 / data[10 * c + datapos] + 0.5;
      else
	Warn("acid spread ratio cannot be zero, error at byte %d", 10 * c + datapos);

      // br only allowed values 1..8 in here, but works the same way. prob -> *1E6
      if (data[11 * c + datapos] != 0)
        cave->pushing_stone_prob = 1E6 / data[11 * c + datapos] + 0.5;
      else
	Warn("pushing stone probability cannot be zero, error at byte %d", 11 * c + datapos);

      cave->magic_wall_stops_amoeba = (data[12 * c + datapos + 1] != 0);
      cave->intermission = (cavenum >= 16 || data[14 * c + datapos + 1] != 0);

      // colors
      colind = data[31 * c + datapos] % ARRAY_SIZE(brc_color_table);

      cave->color_b  = 0x000000;    // fixed rgb black
      cave->color[0] = 0x000000;    // fixed rgb black
      cave->color[1] = brc_color_table[colind];         // brc specified dirt color
      cave->color[2] = brc_color_table_comp[colind];    // complement
      cave->color[3] = 0xffffff;    // white for brick
      cave->color[4] = 0xe5ad23;    // fixed for amoeba
      cave->color[5] = 0x8af713;    // fixed for slime
      cave->color[6] = 0x888888;    // extra color 1 - gray
      cave->color[7] = 0xffffff;    // extra color 2 - white

      if (import_effect)
      {
	cave->amoeba_enclosed_effect = brc_effect(data[14 * c + datapos + 1]);
	cave->amoeba_too_big_effect  = brc_effect(data[15 * c + datapos + 1]);
	cave->explosion_effect       = brc_effect(data[16 * c + datapos + 1]);
	cave->bomb_explosion_effect  = brc_effect(data[17 * c + datapos + 1]);

	// 18 solid bomb explode to
	cave->diamond_birth_effect    = brc_effect(data[19 * c + datapos + 1]);
	cave->stone_bouncing_effect   = brc_effect(data[20 * c + datapos + 1]);
	cave->diamond_bouncing_effect = brc_effect(data[21 * c + datapos + 1]);
	cave->magic_diamond_to        = brc_effect(data[22 * c + datapos + 1]);
	cave->acid_eats_this          = brc_effect(data[23 * c + datapos + 1]);

	/*
	  slime eats:
	  (diamond,boulder,bomb),
	  (diamond,boulder),
	  (diamond,bomb),
	  (boulder,bomb)
	*/
	cave->amoeba_enclosed_effect = brc_effect(data[14 * c + datapos + 1]);
      }
    }
  }

  // put them in the caveset - take correct order into consideration.
  for (level = 0; level < 5; level++)
  {
    int cavenum;

    for (cavenum = 0; cavenum < 20; cavenum++)
    {
      static const int reorder[] =
      {
	0, 1, 2, 3, 16, 4, 5, 6, 7, 17, 8, 9, 10, 11, 18, 12, 13, 14, 15, 19
      };
      GdCave *cave = imported[level * 20 + reorder[cavenum]];
      boolean only_dirt;
      int x, y;

      // check if cave contains only dirt. that is an empty cave, and do not import.
      only_dirt = TRUE;

      for (y = 1; y < cave->h - 1 && only_dirt; y++)
	for (x = 1; x < cave->w - 1 && only_dirt; x++)
	  if (cave->map[y][x] != O_DIRT)
	    only_dirt = FALSE;

      // append to caveset or forget it.
      if (!only_dirt)
	gd_caveset = list_append(gd_caveset, cave);
      else
	gd_cave_free(cave);
    }
  }
}

static void caveset_name_set_from_filename(char *filename)
{
  char *name;
  char *c;

  // make up a caveset name from the filename.
  name = getBaseName(filename);
  gd_strcpy(gd_caveset_data->name, name);
  free(name);

  // convert underscores to spaces
  while ((c = strchr (gd_caveset_data->name, '_')) != NULL)
    *c = ' ';

  // remove extension
  if ((c = strrchr (gd_caveset_data->name, '.')) != NULL)
    *c = 0;
}

/*
  Load caveset from file.
  Loads the caveset from a file.

  File type is autodetected by extension.
  param filename: Name of file.
  result: FALSE if failed
*/
boolean gd_caveset_load_from_file(char *filename)
{
  size_t filesize = getSizeOfFile(filename);
  size_t length;
  char *buf;
  List *new_caveset;
  File *file;

  if (filesize < 0)
  {
    Warn("cannot get size of file '%s'", filename);

    return FALSE;
  }

  if (filesize > 1048576)
  {
    Warn("cannot load file '%s' (bigger than 1 MB)", filename);

    return FALSE;
  }

  if (!(file = openFile(filename, MODE_READ)))
  {
    Warn("cannot open file '%s'", filename);

    return FALSE;
  }

  buf = checked_malloc(filesize + 1);
  length = readFile(file, buf, 1, filesize);
  buf[length] = '\0';

  closeFile(file);

  if (length < filesize)
  {
    Warn("cannot read file '%s'", filename);

    return FALSE;
  }

  if (strSuffix(filename, ".brc") ||
      strSuffix(filename, ".BRC"))
  {
    // loading a boulder remake file
    if (length != 96000)
    {
      Warn("BRC files must be 96000 bytes long");

      return FALSE;
    }
  }

  if (strSuffix(filename, ".brc") ||
      strSuffix(filename, ".BRC"))
  {
    brc_import((byte *) buf);
    gd_caveset_edited = FALSE;    // newly loaded cave is not edited
    gd_caveset_last_selected = caveset_first_selectable_cave_index();
    gd_caveset_last_selected_level = 0;
    free(buf);
    caveset_name_set_from_filename(filename);

    return TRUE;
  }

  // BDCFF
  if (gd_caveset_imported_get_format((byte *) buf) == GD_FORMAT_UNKNOWN)
  {
    // try to load as bdcff
    boolean result;

    // bdcff: start another function
    result = gd_caveset_load_from_bdcff(buf);

    // newly loaded file is not edited.
    gd_caveset_edited = FALSE;

    gd_caveset_last_selected = caveset_first_selectable_cave_index();
    gd_caveset_last_selected_level = 0;
    free(buf);

    return result;
  }

  // try to load as a binary file, as we know the format
  new_caveset = gd_caveset_import_from_buffer ((byte *) buf, length);
  free(buf);

  // if unable to load, exit here. error was reported by import_from_buffer()
  if (!new_caveset)
    return FALSE;

  // no serious error :)

  // only clear caveset here. if file read was unsuccessful, caveset remains in memory.
  gd_caveset_clear();

  gd_caveset = new_caveset;

  // newly loaded cave is not edited
  gd_caveset_edited = FALSE;

  gd_caveset_last_selected = caveset_first_selectable_cave_index();
  gd_caveset_last_selected_level = 0;
  caveset_name_set_from_filename(filename);

  return TRUE;
}

boolean gd_caveset_save_to_file(const char *filename)
{
  GdPtrArray *saved = gd_caveset_save_to_bdcff();
  boolean success;
  File *file;
  int i;

  if ((file = openFile(filename, MODE_WRITE)) != NULL)
  {
    for (i = 0; i < saved->size; i++)
    {
      writeFile(file, saved->data[i], 1, strlen(saved->data[i]));
      writeFile(file, "\n", 1, 1);
    }

    closeFile(file);

    // remember that it is saved
    gd_caveset_edited = FALSE;

    success = TRUE;
  }
  else
  {
    Warn("cannot open file '%s'", filename);

    success = FALSE;
  }

  gd_ptr_array_free(saved, TRUE);

  return success;
}


int gd_cave_check_replays(GdCave *cave, boolean report, boolean remove, boolean repair)
{
  List *riter;
  int wrong = 0;

  riter = cave->replays;

  while (riter != NULL)
  {
    GdReplay *replay = (GdReplay *)riter->data;
    unsigned int checksum;
    GdCave *rendered;
    List *next = riter->next;

    rendered = gd_cave_new_rendered(cave, replay->level, replay->seed);
    checksum = gd_cave_adler_checksum(rendered);
    gd_cave_free(rendered);

    replay->wrong_checksum = FALSE;

    // count wrong ones... the checksum might be changed later to "repair"
    if (replay->checksum != 0 && checksum != replay->checksum)
      wrong++;

    if (replay->checksum == 0 || repair)
    {
      // if no checksum found, add one. or if repair requested, overwrite old one.
      replay->checksum = checksum;
    }
    else
    {
      // if has a checksum, compare with this one.
      if (replay->checksum != checksum)
      {
	replay->wrong_checksum = TRUE;

	if (report)
	  Warn("%s: replay played by %s at %s has wrong checksum",
	       cave->name, replay->player_name, replay->date);

	if (remove)
	{
	  // may remove
	  cave->replays = list_remove_link(cave->replays, riter);
	  gd_replay_free(replay);
	}
      }
    }

    // advance to next list item which we remembered. the current one might have been deleted
    riter = next;
  }

  return wrong;
}

boolean gd_caveset_has_replays(void)
{
  List *citer;

  // for all caves
  for (citer = gd_caveset; citer != NULL; citer = citer->next)
  {
    GdCave *cave = (GdCave *)citer->data;

    if (cave->replays)
      return TRUE;
  }

  // if neither of the caves had a replay,
  return FALSE;
}
