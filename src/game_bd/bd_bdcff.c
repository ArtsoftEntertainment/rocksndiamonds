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

#include <errno.h>

#include "main_bd.h"


#define BDCFF_VERSION "0.5"

// these are used for bdcff loading, storing the sizes of caves
static int cavesize[6], intermissionsize[6];

static boolean replay_store_from_bdcff(GdReplay *replay, const char *str)
{
  GdDirection dir;
  boolean up, down, left, right;
  boolean fire, suicide;
  const char *num = NULL;
  int count, i;

  fire = suicide = up = down = left = right = FALSE;

  for (i = 0; str[i] != 0; i++)
  {
    switch (str[i])
    {
      case 'U':
	fire = TRUE;
      case 'u':
	up = TRUE;
	break;

      case 'D':
	fire = TRUE;
      case 'd':
	down = TRUE;
	break;

      case 'L':
	fire = TRUE;
      case 'l':
	left = TRUE;
	break;

      case 'R':
	fire = TRUE;
      case 'r':
	right = TRUE;
	break;

      case 'F':
	fire = TRUE;
	break;
      case 'k':
	suicide = TRUE;
	break;

      case '.':
	// do nothing, as all other movements are false
	break;

      case 'c':
      case 'C':
	// bdcff 'combined' flags. do nothing.
	break;

      default:
	if (str[i] >= '0' && str[i] <= '9')
	{
	  if (!num)
	    num = str + i;
	}
    }
  }

  dir = gd_direction_from_keypress(up, down, left, right);
  count = 1;

  if (num)
    sscanf(num, "%d", &count);

  for (i = 0; i < count; i++)
    gd_replay_store_movement(replay, dir, fire, suicide);

  return TRUE;
}

static boolean attrib_is_valid_for_cave(const char *attrib)
{
  int i;

  // bdcff engine flag............
  if (strcasecmp(attrib, "Engine") == 0)
    return TRUE;

  // old flags - for compatibility
  if (strcasecmp(attrib, "BD1Scheduling") == 0)
    return TRUE;

  if (strcasecmp(attrib, "SnapExplosions") == 0)
    return TRUE;

  if (strcasecmp(attrib, "AmoebaProperties") == 0)
    return TRUE;

  // search in property database
  for (i = 0; gd_cave_properties[i].identifier != NULL; i++)
    if (strcasecmp(gd_cave_properties[i].identifier, attrib) == 0)
      return TRUE;

  return FALSE;
}

static boolean attrib_is_valid_for_caveset(const char *attrib)
{
  int i;

  // search in property database
  for (i = 0; gd_caveset_properties[i].identifier != NULL; i++)
    if (strcasecmp(gd_caveset_properties[i].identifier, attrib) == 0)
      return TRUE;

  return FALSE;
}

static boolean struct_set_property(void *str, const GdStructDescriptor *prop_desc,
				   const char *attrib, const char *param, int ratio)
{
  char **params;
  int paramcount;
  boolean identifier_found;
  int paramindex = 0;
  int i;
  boolean was_string;

  params = getSplitStringArray(param, " ", -1);
  paramcount = getStringArrayLength(params);
  identifier_found = FALSE;

  // check all known tags. do not exit this loop if identifier_found == true...
  // as there are more lines in the array which have the same identifier.
  was_string = FALSE;

  for (i = 0; prop_desc[i].identifier != NULL; i++)
  {
    if (strcasecmp(prop_desc[i].identifier, attrib) == 0)
    {
      // found the identifier
      void *value = STRUCT_MEMBER_P(str, prop_desc[i].offset);

      // these point to the same, but to avoid the awkward cast syntax
      int *ivalue = value;
      GdElement *evalue = value;
      GdDirection *dvalue = value;
      GdScheduling *svalue = value;
      boolean *bvalue = value;
      int j, k;

      identifier_found = TRUE;

      if (prop_desc[i].type == GD_TYPE_STRING)
      {
	// strings are treated different, as occupy the whole length of the line
	char *str = getUnescapedString(param);

	gd_strcpy(value, str);
	checked_free(str);

	// remember this to skip checking the number of parameters at the end of the function
	was_string = TRUE;

	continue;
      }

      if (prop_desc[i].type == GD_TYPE_LONGSTRING)
      {
	char **str = (char **)value;

	checked_free(*str);
	*str = getUnescapedString(param);

	// remember this to skip checking the number of parameters at the end of the function
	was_string = TRUE;

	continue;
      }

      // not a string, so use scanf calls
      // ALSO, if no more parameters to process, exit loop
      for (j = 0; j < prop_desc[i].count && params[paramindex] != NULL; j++)
      {
	boolean success = FALSE;
	double res;

	switch (prop_desc[i].type)
	{
	  case GD_TYPE_LONGSTRING:
	  case GD_TYPE_STRING:
	    // handled above
	  case GD_TAB:
	  case GD_LABEL:
	    // do nothing
	    break;

	  case GD_TYPE_BOOLEAN:
	    success = sscanf(params[paramindex], "%d", &bvalue[j]) == 1;
	    if (!success)
	    {
	      if (strcasecmp(params[paramindex], "true") == 0 ||
		  strcasecmp(params[paramindex], "on") == 0 ||
		  strcasecmp(params[paramindex], "yes") == 0)
	      {
		bvalue[j] = TRUE;
		success = TRUE;
	      }
	      else if (strcasecmp(params[paramindex], "false") == 0 ||
		       strcasecmp(params[paramindex], "off") == 0 ||
		       strcasecmp(params[paramindex], "no") == 0)
	      {
		bvalue[j] = FALSE;
		success = TRUE;
	      }
	    }

	    // if we are processing an array, fill other values with these.
	    // if there are other values specified, those will be overwritten.
	    if (success)
	      for (k = j + 1; k < prop_desc[i].count; k++)
		bvalue[k] = bvalue[j];

	    break;

	  case GD_TYPE_INT:
	    success = sscanf(params[paramindex], "%d", &ivalue[j]) == 1;
	    if (success)
	      // copy to other if array
	      for (k = j + 1; k < prop_desc[i].count; k++)
		ivalue[k] = ivalue[j];

	    break;

	  case GD_TYPE_PROBABILITY:
	    // must be reset before calling strtod() to detect overflow/underflow
	    errno = 0;
	    res = strtod(params[paramindex], NULL);
	    if (errno == 0 && res >= 0 && res <= 1)
	    {
	      // fill all remaining items in array - may be only one
	      for (k = j; k < prop_desc[i].count; k++)
		// probabilities are stored inside as ppm (1E6)
		ivalue[k] = res * 1E6 + 0.5;

	      success = TRUE;
	    }

	    break;

	  case GD_TYPE_RATIO:
	    // must be reset before calling strtod() to detect overflow/underflow
	    errno = 0;
	    res = strtod (params[paramindex], NULL);
	    if (errno == 0 && res >= 0 && res <= 1)
	    {
	      for (k = j; k < prop_desc[i].count; k++)
		ivalue[k] = (int)(res * ratio + 0.5);

	      success = TRUE;
	    }

	    break;

	  case GD_TYPE_ELEMENT:
	    evalue[j] = gd_get_element_from_string(params[paramindex]);

	    // copy to all remaining elements in array
	    for (k = j + 1; k < prop_desc[i].count; k++)
	      evalue[k] = evalue[j];

	    // this shows error message on its own, do treat as always succeeded
	    success = TRUE;
	    break;

	  case GD_TYPE_DIRECTION:
	    dvalue[j] = gd_direction_from_string(params[paramindex]);
	    // copy to all remaining items in array
	    for (k = j + 1; k < prop_desc[i].count; k++)
	      dvalue[k] = dvalue[j];

	    success = TRUE;
	    break;

	  case GD_TYPE_SCHEDULING:
	    svalue[j] = gd_scheduling_from_string(params[paramindex]);
	    // copy to all remaining items in array
	    for (k = j + 1; k < prop_desc[i].count; k++)
	      svalue[k] = svalue[j];

	    // if there was an error, already reported by gd_scheduling_from_string
	    success = TRUE;
	    break;

	  case GD_TYPE_COLOR:
	  case GD_TYPE_EFFECT:
	    // shoud have handled this elsewhere
	    break;
	}

	if (success)
	  paramindex++;    // go to next parameter to process
	else
	  Warn("invalid parameter '%s' for attribute %s", params[paramindex], attrib);
      }
    }
  }

  // if we found the identifier, but still could not process all parameters...
  // of course, not for strings, as the whole line is the string
  if (identifier_found && !was_string && paramindex < paramcount)
    Warn("excess parameters for attribute '%s': '%s'", attrib, params[paramindex]);

  freeStringArray(params);

  return identifier_found;
}


// ============================================================================
// BDCFF LOADING
// ============================================================================

static boolean replay_store_more_from_bdcff(GdReplay *replay, const char *param)
{
  char **split;
  int i;
  boolean result = TRUE;

  split = getSplitStringArray(param, " ", -1);

  for (i = 0; split[i] != 0; i++)
    result = result && replay_store_from_bdcff(replay, split[i]);

  freeStringArray(split);

  return result;
}

// report all remaining tags; called after the above function.
static void replay_report_unknown_tags_func(const char *attrib, const char *param, void *data)
{
  Warn("unknown replay tag '%s'", attrib);
}

// a GHashTable foreach func.
// keys are attribs; values are params;
// the user data is the cave the hash table belongs to.
static boolean replay_process_tags_func(const char *attrib, const char *param, GdReplay *replay)
{
  boolean identifier_found = FALSE;

  // movements
  if (strcasecmp(attrib, "Movements") == 0)
  {
    identifier_found = TRUE;
    replay_store_more_from_bdcff(replay, param);
  }
  else
  {
    // any other tag
    // 0: for ratio types; not used
    identifier_found = struct_set_property(replay, gd_replay_properties,
					   attrib, param, 0);
  }

  // a ghrfunc should return true if the identifier is to be removed
  return identifier_found;
}

// ...
static void replay_process_tags(GdReplay *replay, HashTable *tags)
{
  // process all tags
  hashtable_foreach_remove(tags, (hashtable_remove_fn)replay_process_tags_func, replay);
}

// a GHashTable foreach func.
// keys are attribs; values are params;
// the user data is the cave the hash table belongs to.
static boolean cave_process_tags_func(const char *attrib, const char *param, GdCave *cave)
{
  char **params;
  int paramcount;
  boolean identifier_found;

  params = getSplitStringArray(param, " ", -1);
  paramcount = getStringArrayLength(params);
  identifier_found = FALSE;

  if (strcasecmp(attrib, "SnapExplosions") == 0)
  {
    // handle compatibility with old snapexplosions flag

    identifier_found = TRUE;

    if (strcasecmp(param, "true") == 0)
    {
      cave->snap_element = O_EXPLODE_1;
    }
    else if (strcasecmp(param, "false") == 0)
    {
      cave->snap_element = O_SPACE;
    }
    else
    {
      Warn("invalid param for '%s': '%s'", attrib, param);
    }
  }
  else if (strcasecmp(attrib, "BD1Scheduling") == 0)
  {
    // handle compatibility with old bd1scheduling flag

    identifier_found = TRUE;

    if (strcasecmp(param, "true") == 0)
    {
      if (cave->scheduling == GD_SCHEDULING_PLCK)
	cave->scheduling = GD_SCHEDULING_BD1;
    }
  }
  else if (strcasecmp(attrib, "Engine") == 0)
  {
    // handle bdcff engine flag

    identifier_found = TRUE;

    GdEngine engine = gd_cave_get_engine_from_string(param);

    if (engine == GD_ENGINE_INVALID)
      Warn(_("invalid parameter \"%s\" for attribute %s"), param, attrib);
    else
      gd_cave_set_engine_defaults(cave, engine);
  }
  else if (strcasecmp(attrib, "AmoebaProperties") == 0)
  {
    // handle compatibility with old AmoebaProperties flag

    GdElement elem1 = O_STONE, elem2 = O_DIAMOND;

    identifier_found = TRUE;
    elem1 = gd_get_element_from_string(params[0]);
    elem2 = gd_get_element_from_string(params[1]);
    cave->amoeba_too_big_effect = elem1;
    cave->amoeba_enclosed_effect = elem2;
  }
  else if (strcasecmp(attrib, "Colors") == 0)
  {
    // colors attribute is a mess, have to process explicitly
    boolean ok = TRUE;

    // Colors = [border background] foreground1 foreground2 foreground3 [amoeba slime]
    identifier_found = TRUE;

    if (paramcount == 3)
    {
      // only colors 1,2,3
      cave->color_b  = gd_c64_color(0);   // border - black
      cave->color[0] = gd_c64_color(0);   // background - black
      cave->color[1] = gd_color_get_from_string(params[0]);
      cave->color[2] = gd_color_get_from_string(params[1]);
      cave->color[3] = gd_color_get_from_string(params[2]);
      cave->color[4] = cave->color[3];    // amoeba
      cave->color[5] = cave->color[1];    // slime
      cave->color[6] = gd_c64_color(12);  // extra color 1 - gray
      cave->color[7] = gd_c64_color(1);   // extra color 2 - white
    }
    else if (paramcount == 5)
    {
      // bg, color 0,1,2,3
      cave->color_b  = gd_color_get_from_string(params[0]);
      cave->color[0] = gd_color_get_from_string(params[1]);
      cave->color[1] = gd_color_get_from_string(params[2]);
      cave->color[2] = gd_color_get_from_string(params[3]);
      cave->color[3] = gd_color_get_from_string(params[4]);
      cave->color[4] = cave->color[3];    // amoeba
      cave->color[5] = cave->color[1];    // slime
      cave->color[6] = gd_c64_color(12);  // extra color 1 - gray
      cave->color[7] = gd_c64_color(1);   // extra color 2 - white
    }
    else if (paramcount == 7)
    {
      // bg, color 0,1,2,3, amoeba, slime
      cave->color_b  = gd_color_get_from_string(params[0]);
      cave->color[0] = gd_color_get_from_string(params[1]);
      cave->color[1] = gd_color_get_from_string(params[2]);
      cave->color[2] = gd_color_get_from_string(params[3]);
      cave->color[3] = gd_color_get_from_string(params[4]);
      cave->color[4] = gd_color_get_from_string(params[5]);    // amoeba
      cave->color[5] = gd_color_get_from_string(params[6]);    // slime
      cave->color[6] = gd_c64_color(12);                       // extra color 1 - gray
      cave->color[7] = gd_c64_color(1);                        // extra color 2 - white
    }
    else
    {
      Warn("invalid number of color strings: %s", param);

      ok = FALSE;
    }

    // now check and maybe make up some new.
    if (!ok ||
	gd_color_is_unknown(cave->color_b)  ||
	gd_color_is_unknown(cave->color[0]) ||
	gd_color_is_unknown(cave->color[1]) ||
	gd_color_is_unknown(cave->color[2]) ||
	gd_color_is_unknown(cave->color[3]) ||
	gd_color_is_unknown(cave->color[4]) ||
	gd_color_is_unknown(cave->color[5]) ||
	gd_color_is_unknown(cave->color[6]) ||
	gd_color_is_unknown(cave->color[7]))
    {
      Warn("created a new C64 color scheme.");

      gd_cave_set_random_c64_colors(cave);    // just create some random
    }
  }
  else
  {
    identifier_found = struct_set_property(cave, gd_cave_properties, attrib, param, cave->w * cave->h);
  }

  freeStringArray(params);

  // a ghrfunc should return true if the identifier is to be removed
  return identifier_found;
}

// report all remaining tags; called after the above function.
static void cave_report_and_copy_unknown_tags_func(char *attrib, char *param, void *data)
{
  GdCave *cave = (GdCave *)data;

  Warn("unknown tag '%s'", attrib);

  hashtable_insert(cave->tags, getStringCopy(attrib), getStringCopy(param));
}

// having read all strings belonging to the cave, process it.
static void cave_process_tags(GdCave *cave, HashTable *tags, List *maplines)
{
  char *value;

  // first check cave name, so we can report errors correctly (saying that GdCave xy: error foobar)
  value = hashtable_search(tags, "Name");
  if (value)
    cave_process_tags_func("Name", value, cave);

  // process lame engine tag first so its settings may be overwritten later
  value = hashtable_search(tags, "Engine");
  if (value)
  {
    cave_process_tags_func("Engine", value, cave);
    hashtable_remove(tags, "Engine");
  }

  // check if this is an intermission, so we can set to cavesize or intermissionsize
  value = hashtable_search(tags, "Intermission");
  if (value)
  {
    cave_process_tags_func("Intermission", value, cave);
    hashtable_remove(tags, "Intermission");
  }

  if (cave->intermission)
  {
    // set to IntermissionSize
    cave->w  = intermissionsize[0];
    cave->h  = intermissionsize[1];
    cave->x1 = intermissionsize[2];
    cave->y1 = intermissionsize[3];
    cave->x2 = intermissionsize[4];
    cave->y2 = intermissionsize[5];
  }
  else
  {
    // set to CaveSize
    cave->w = cavesize[0];
    cave->h = cavesize[1];
    cave->x1 = cavesize[2];
    cave->y1 = cavesize[3];
    cave->x2 = cavesize[4];
    cave->y2 = cavesize[5];
  }

  // process size at the beginning... as ratio types depend on this.
  value = hashtable_search(tags, "Size");
  if (value)
  {
    cave_process_tags_func("Size", value, cave);
    hashtable_remove(tags, "Size");
  }

  // these are read from the hash table, but also have some implications
  // we do not delete them from the hash table here; as _their values will be processed later_.
  // here we only set their implicite meanings.
  // these also set predictability
  if (hashtable_search(tags, "SlimePermeability"))
    cave->slime_predictable = FALSE;

  if (hashtable_search(tags, "SlimePermeabilityC64"))
    cave->slime_predictable = TRUE;

  // these set scheduling type.
  // framedelay takes precedence, if there are both; so we check it later.
  if (hashtable_search(tags, "CaveDelay"))
  {
    // only set scheduling type, when it is not the gdash-default.
    // this allows settings cavescheduling = bd1 in the [game] section, for example.
    // in that case, this one will not overwrite it.
    if (cave->scheduling == GD_SCHEDULING_MILLISECONDS)
      cave->scheduling = GD_SCHEDULING_PLCK;
  }

  // but if the cave has a frametime setting, always switch to milliseconds.
  if (hashtable_search(tags, "FrameTime"))
    cave->scheduling = GD_SCHEDULING_MILLISECONDS;

  // process all tags
  hashtable_foreach_remove(tags, (hashtable_remove_fn)cave_process_tags_func, cave);

  // and at the end, when read all tags (especially the size= tag)
  // process map, if any.
  // only report if map read is bigger than size= specified.
  // some old bdcff files use smaller intermissions than the one specified.
  if (maplines)
  {
    int x, y, length = list_length(maplines);
    List *iter;

    // create map and fill with initial border, in case that map strings are shorter or somewhat
    cave->map = gd_cave_map_new(cave, GdElement);

    for (y = 0; y < cave->h; y++)
      for (x = 0; x < cave->w; x++)
	cave->map[y][x] = cave->initial_border;

    if (length != cave->h && length != (cave->y2-cave->y1 + 1))
      Warn("map error: cave height = %d (%d visible), map height = %d",
	   cave->h, cave->y2 - cave->y1 + 1, length);

    for (iter = maplines, y = 0; y < length && iter != NULL; iter = iter->next, y++)
    {
      const char *line = iter->data;
      int slen = strlen(line);

      if (slen != cave->w && slen != (cave->x2 - cave->x1 + 1))
	Warn("map error in row %d: cave width = %d (%d visible), map width = %d",
	     y, cave->w, cave->x2 - cave->x1 + 1, slen);

      // use number of cells from cave or string, whichever is smaller.
      // so will not overwrite array!
      for (x = 0; x < MIN(cave->w, slen); x++)
	cave->map[y][x] = gd_get_element_from_character (line[x]);
    }
  }
}

// sets the cavesize array to default values
static void set_cavesize_defaults(void)
{
  cavesize[0] = 40;
  cavesize[1] = 22;
  cavesize[2] = 0;
  cavesize[3] = 0;
  cavesize[4] = 39;
  cavesize[5] = 21;
}

// sets the cavesize array to default values
static void set_intermissionsize_defaults(void)
{
  intermissionsize[0] = 40;
  intermissionsize[1] = 22;
  intermissionsize[2] = 0;
  intermissionsize[3] = 0;
  intermissionsize[4] = 19;
  intermissionsize[5] = 11;
}

boolean gd_caveset_load_from_bdcff(const char *contents)
{
  char **lines;
  int lineno;
  GdCave *cave;
  List *iter;
  boolean reading_replay = FALSE;
  boolean reading_map = FALSE;
  boolean reading_mapcodes = FALSE;
  boolean reading_highscore = FALSE;
  boolean reading_objects = FALSE;
  boolean reading_bdcff_demo = FALSE;
  // assume version to be 0.32, also when the file does not specify it explicitly
  GdString version_read = "0.32";
  List *mapstrings = NULL;
  int linenum;
  HashTable *tags, *replay_tags;
  GdObjectLevels levels = GD_OBJECT_LEVEL_ALL;
  GdCave *default_cave;

  gd_caveset_clear();

  set_cavesize_defaults();
  set_intermissionsize_defaults();
  gd_create_char_to_element_table();

  tags        = create_hashtable(gd_str_case_hash, gd_str_case_equal, free, free);
  replay_tags = create_hashtable(gd_str_case_hash, gd_str_case_equal, free, free);

  // split into lines
  lines = getSplitStringArray (contents, "\n", 0);

  // attributes read will be set in cave. if no [cave]; they are stored
  // in the default cave; like in a [game] */
  default_cave = gd_cave_new();
  cave = default_cave;

  linenum = getStringArrayLength(lines);

  for (lineno = 0; lineno < linenum; lineno++)
  {
    char *line = lines[lineno];
    char *r;

    // remove windows-nightmare \r-s
    while ((r = strchr(line, '\r')))
      strcpy(r, r + 1);

    // skip empty lines
    if (strlen(line) == 0)
      continue;

    // just skip comments. be aware that map lines may start with a semicolon...
    if (!reading_map && line[0] == ';')
      continue;

    // STARTING WITH A BRACKET [ IS A SECTION
    if (line[0] == '[')
    {
      if (strcasecmp(line, "[cave]") == 0)
      {
	// new cave
	if (mapstrings)
	{
	  Warn("incorrect file format: new [cave] section, but already read some map lines");
	  list_free(mapstrings);
	  mapstrings = NULL;
	}

	// process any pending tags for game ...
	cave_process_tags(default_cave, tags, NULL);

	// ... to be able to create a copy for a new cave.
	cave = gd_cave_new_from_cave(default_cave);
	gd_caveset = list_append (gd_caveset, cave);
      }
      else if (strcasecmp(line, "[/cave]") == 0)
      {
	cave_process_tags(cave, tags, mapstrings);
	list_free(mapstrings);
	mapstrings = NULL;

	hashtable_foreach(tags, (hashtable_fn)cave_report_and_copy_unknown_tags_func, cave);
	hashtable_remove_all(tags);

	// set this to point the pseudo-cave which holds default values
	cave = default_cave;
      }
      else if (strcasecmp(line, "[map]") == 0)
      {
	reading_map = TRUE;
	if (mapstrings != NULL)
	{
	  Warn("incorrect file format: new [map] section, but already read some map lines");
	  list_free(mapstrings);
	  mapstrings = NULL;
	}
      }
      else if (strcasecmp(line, "[/map]") == 0)
      {
	reading_map = FALSE;
      }
      else if (strcasecmp(line, "[mapcodes]") == 0)
      {
	reading_mapcodes = TRUE;
      }
      else if (strcasecmp(line, "[/mapcodes]") == 0)
      {
	reading_mapcodes = FALSE;
      }
      else if (strcasecmp(line, "[highscore]") == 0)
      {
	reading_highscore = TRUE;
      }
      else if (strcasecmp(line, "[/highscore]") == 0)
      {
	reading_highscore = FALSE;
      }
      else if (strcasecmp(line, "[objects]") == 0)
      {
	reading_objects = TRUE;
      }
      else if (strcasecmp(line, "[/objects]") == 0)
      {
	reading_objects = FALSE;
      }
      else if (strcasecmp(line, "[demo]") == 0)
      {
	GdReplay *replay;

	reading_bdcff_demo = TRUE;

	if (cave != default_cave)
	{
	  replay = gd_replay_new();
	  replay->saved = TRUE;
	  replay->success = TRUE;   // we think that it is a successful demo
	  cave->replays = list_append(cave->replays, replay);
	  gd_strcpy(replay->player_name, "???");    // name not saved
	}
	else
	{
	  Warn("[demo] section must be in [cave] section!");
	}
      }
      else if (strcasecmp(line, "[/demo]") == 0)
      {
	reading_bdcff_demo = FALSE;
      }
      else if (strcasecmp(line, "[replay]") == 0)
      {
	reading_replay = TRUE;
      }
      else if (strcasecmp(line, "[/replay]") == 0)
      {
	GdReplay *replay;

	reading_replay = FALSE;
	replay = gd_replay_new();

	// set "saved" flag, so this replay will be written when the caveset is saved again
	replay->saved = TRUE;
	replay_process_tags(replay, replay_tags);

#if 1
	// BDCFF numbers levels from 1 to 5, but internally we number levels from 0 to 4
	if (replay->level > 0)
	  replay->level--;
#endif

	// report any remaining unknown tags
	hashtable_foreach(replay_tags, (hashtable_fn)replay_report_unknown_tags_func, NULL);
	hashtable_remove_all(replay_tags);

	if (replay->movements->len != 0)
	{
	  cave->replays = list_append(cave->replays, replay);
	}
	else
	{
	  Warn("no movements in replay!");
	  gd_replay_free(replay);
	}
      }
      // GOSH i hate bdcff
      else if (strncasecmp(line, "[level=", strlen("[level=")) == 0)
      {
	int l[5];
	int num;
	char *nums;

	// there IS an equal sign, and we also skip that, so this points to the numbers
	nums = strchr(line, '=') + 1;
	num = sscanf(nums, "%d,%d,%d,%d,%d", l + 0, l + 1, l + 2, l + 3, l + 4);
	levels = 0;

	if (num == 0)
	{
	  Warn("invalid Levels tag: %s", line);
	  levels = GD_OBJECT_LEVEL_ALL;
	}
	else
	{
	  int n;

	  for (n = 0; n < num; n++)
	  {
	    if (l[n] <= 5 && l[n] >= 1)
	      levels |= gd_levels_mask[l[n] - 1];
	    else
	      Warn("invalid level number %d", l[n]);
	  }
	}
      }
      else if (strcasecmp(line, "[/level]") == 0)
      {
	levels = GD_OBJECT_LEVEL_ALL;
      }
      else if (strcasecmp(line, "[game]") == 0)
      {
      }
      else if (strcasecmp(line, "[/game]") == 0)
      {
      }
      else if (strcasecmp(line, "[BDCFF]") == 0)
      {
      }
      else if (strcasecmp(line, "[/BDCFF]") == 0)
      {
      }
      else
      {
	Warn("unknown section: \"%s\"", line);
      }

      continue;
    }

    if (reading_map)
    {
      // just append to the mapstrings list. we will process it later
      mapstrings = list_append(mapstrings, line);

      continue;
    }

    // strip leading and trailing spaces AFTER checking if we are reading a map.
    // map lines might begin or end with spaces */
    stripString(line);

    if (reading_highscore)
    {
      int score;

      if (sscanf(line, "%d", &score) != 1 || strchr(line, ' ') == NULL)
      {
	// first word is the score
	Warn("highscore format incorrect");
      }
      else
      {
	if (cave == default_cave)
	  // if we are reading the [game], add highscore to that one.
	  // from first space: the name
	  gd_add_highscore(gd_caveset_data->highscore, strchr(line, ' ') + 1, score);
	else
	  // if a cave, add highscore to that.
	  gd_add_highscore(cave->highscore, strchr(line, ' ') + 1, score);
      }

      continue;
    }

    // read bdcff-style [demo], similar to a complete replay but cannot store like anything
    if (reading_bdcff_demo)
    {
      GdReplay *replay;
      List *iter;

      // demo must be in [cave] section. we already showed an error message for this.
      if (cave == default_cave)
	continue;

      iter = list_last(cave->replays);

      replay = (GdReplay *)iter->data;
      replay_store_more_from_bdcff(replay, line);

      continue;
    }

    if (reading_objects)
    {
      GdObject *new_object;

      new_object = gd_object_new_from_string(line);
      if (new_object)
      {
	new_object->levels = levels;    // apply levels to new object
	cave->objects = list_append(cave->objects, new_object);
      }
      else
      {
	Error("invalid object specification: %s", line);
      }

      continue;
    }

    // has an equal sign ->  some_attrib = parameters  type line.
    if (strchr (line, '=') != NULL)
    {
      char *attrib, *param;

      attrib = line;                   // attrib is from the first char
      param = strchr(line, '=') + 1;   // param is after equal sign
      *strchr (line, '=') = 0;         // delete equal sign - line is therefore splitted

      // own tag: not too much thinking :P
      if (reading_replay)
      {
	hashtable_insert(replay_tags, getStringCopy(attrib), getStringCopy(param));
      }
      else if (reading_mapcodes)
      {
	if (strcasecmp("Length", attrib) == 0)
	{
	  // we do not support map code width != 1
	  if (strcmp(param, "1") != 0)
	    Warn(_("Only one-character map codes are currently supported!"));
	}
	else
	{
	  // the first character of the attribute is the element code itself
	  gd_char_to_element[(int)attrib[0]] = gd_get_element_from_string(param);
	}
      }
      else if (strcasecmp("Version", attrib) == 0)
      {
	// BDCFF version
	gd_strcpy(version_read, param);
      }
      else if (strcasecmp(attrib, "Caves") == 0)
      {
	// CAVES = x

	// BDCFF files sometimes state how many caves they have
	// we ignore this field.
      }
      else if (strcasecmp(attrib, "Levels") == 0)
      {
	// LEVELS = x

	// BDCFF files sometimes state how many levels they have
	// we ignore this field.
      }
      else if (strcasecmp(attrib, "CaveSize") == 0)
      {
	int i;

	i = sscanf(param, "%d %d %d %d %d %d",
		   cavesize + 0,
		   cavesize + 1,
		   cavesize + 2,
		   cavesize + 3,
		   cavesize + 4,
		   cavesize + 5);

	// allowed: 2 or 6 numbers
	if (i == 2)
	{
	  cavesize[2] = 0;
	  cavesize[3] = 0;
	  cavesize[4] = cavesize[0]-1;
	  cavesize[5] = cavesize[1]-1;
	}
	else if (i != 6)
	{
	  set_cavesize_defaults();
	  Warn("invalid CaveSize tag: %s", line);
	}
      }
      else if (strcasecmp(attrib, "IntermissionSize") == 0)
      {
	int i;

	i = sscanf(param, "%d %d %d %d %d %d",
		   intermissionsize + 0,
		   intermissionsize + 1,
		   intermissionsize + 2,
		   intermissionsize + 3,
		   intermissionsize + 4,
		   intermissionsize + 5);

	// allowed: 2 or 6 numbers
	if (i == 2)
	{
	  intermissionsize[2] = 0;
	  intermissionsize[3] = 0;
	  intermissionsize[4] = intermissionsize[0]-1;
	  intermissionsize[5] = intermissionsize[1]-1;
	}
	else if (i != 6)
	{
	  set_intermissionsize_defaults();
	  Warn("invalid IntermissionSize tag: '%s'", line);
	}
      }
      else if (strcasecmp(attrib, "Effect") == 0)
      {
	// CHECK IF IT IS AN EFFECT
	char **params;

	params = getSplitStringArray(param, " ", -1);

	// an effect command has two parameters
	if (getStringArrayLength(params) == 2)
	{
	  int i;

	  for (i = 0; gd_cave_properties[i].identifier != NULL; i++)
	  {
	    // we have to search for this effect
	    if (gd_cave_properties[i].type == GD_TYPE_EFFECT &&
		strcasecmp(params[0], gd_cave_properties[i].identifier) == 0)
	    {
	      // found identifier
	      void *value = STRUCT_MEMBER_P (cave, gd_cave_properties[i].offset);

	      *((GdElement *) value) = non_scanned_pair(gd_get_element_from_string(params[1]));

	      break;
	    }
	  }

	  // if we didn't find first element name
	  if (gd_cave_properties[i].identifier == NULL)
	  {
	    // for compatibility with tim stridmann's memorydump->bdcff converter... .... ...
	    if (strcasecmp(params[0], "BOUNCING_BOULDER") == 0)
	      cave->stone_bouncing_effect =
                non_scanned_pair(gd_get_element_from_string(params[1]));
	    else if (strcasecmp(params[0], "EXPLOSION3S") == 0)
	      cave->explosion_3_effect =
                non_scanned_pair(gd_get_element_from_string(params[1]));
	    // falling with one l...
	    else if (strcasecmp(params[0], "STARTING_FALING_DIAMOND") == 0)
	      cave->diamond_falling_effect =
                non_scanned_pair(gd_get_element_from_string(params[1]));
	    // dirt lookslike
	    else if (strcasecmp(params[0], "DIRT") == 0)
	      cave->dirt_looks_like = gd_get_element_from_string (params[1]);
	    else if (strcasecmp(params[0], "HEXPANDING_WALL") == 0 &&
		     strcasecmp(params[1], "STEEL_HEXPANDING_WALL") == 0)
	    {
	      cave->expanding_wall_looks_like = O_STEEL;
	    }
	    else
	    {
	      // didn't find at all
	      Warn("invalid effect name '%s'", params[0]);
	    }
	  }
	}
	else
	{
	  Warn("invalid effect specification '%s'", param);
	}

	freeStringArray(params);
      }
      else
      {
	// no special handling: this is a normal attribute.

	if (cave == default_cave)
	{
	  // we are reading the [game]
	  if (attrib_is_valid_for_caveset(attrib))
	  {
	    // if it is a caveset attrib, process it for the caveset.
	    struct_set_property(gd_caveset_data, gd_caveset_properties, attrib, param, 0);
	  }
	  else if (attrib_is_valid_for_cave(attrib))
	  {
	    // it must be a default setting for all caves. is it a valid identifier?
	    // yes, it is. add to the hash table, which will be copied for all caves.
	    hashtable_insert(tags, getStringCopy(attrib), getStringCopy(param));
	  }
	  else
	  {
	    // unknown setting - report.
	    Warn("invalid attribute for [game] '%s'", attrib);
	  }
	}
	else
	{
	  // we are reading a [cave]
	  // cave settings are immediately added to cave hash table.
	  // if it is unknown, we have to remember it, and save it again.
	  hashtable_insert(tags, getStringCopy(attrib), getStringCopy(param));
	}
      }

      continue;
    }

    Error("cannot parse line: %s", line);
  }

  if (mapstrings)
  {
    Warn("incorrect file format: end of file, but still have some map lines read");
    list_free(mapstrings);
    mapstrings = NULL;
  }

  // the [game] section had some values which are default if not specified in [cave] sections.
  // these are used only for loading, so forget them now
  if (default_cave->map)
    Warn(_("Invalid BDCFF: [game] section has a map"));
  if (default_cave->objects)
    Warn(_("Invalid BDCFF: [game] section has drawing objects defined"));

  // cleanup
  freeStringArray(lines);
  hashtable_destroy(tags);
  hashtable_destroy(replay_tags);
  gd_cave_free(default_cave);

  // old bdcff files hack. explanation follows.
  // there were 40x22 caves in c64 bd, intermissions were also 40x22, but the visible
  // part was the upper left corner, 20x12. 40x22 caves are needed, as 20x12 caves would
  // look different (random cave elements needs the correct size.)
  // also, in older bdcff files, there is no size= tag. caves default to 40x22 and 20x12.
  // even the explicit drawrect and other drawing instructions, which did set up intermissions
  // to be 20x12, are deleted. very very bad decision.
  // here we try to detect and correct this.

  if (strEqual(version_read, "0.32"))
  {
    List *iter;

    Warn("No BDCFF version, or 0.32. Using unspecified-intermission-size hack.");

    for (iter = gd_caveset; iter != NULL; iter = iter->next)
    {
      GdCave *cave = (GdCave *)iter->data;

      // only applies to intermissions; not applied to mapped caves, as maps are filled with
      // initial border, if the map read is smaller
      if (cave->intermission && !cave->map)
      {
	// we do not set the cave to 20x12, rather to 40x22 with 20x12 visible.
	GdObject object;

	cave->w = 40;
	cave->h = 22;
	cave->x1 = 0;
	cave->y1 = 0;
	cave->x2 = 19;
	cave->y2 = 11;

	// and cover the invisible area
	object.type = GD_FILLED_RECTANGLE;
	object.x1 = 0;
	object.y1 = 11;    // 11, because this will also be the border
	object.x2 = 39;
	object.y2 = 21;
	object.element = cave->initial_border;
	object.fill_element = cave->initial_border;

	cave->objects = list_prepend(cave->objects, get_memcpy(&object, sizeof(object)));

	object.x1 = 19;
	object.y1 = 0;    // 19, as it is also the border

	// another
	cave->objects = list_prepend(cave->objects, get_memcpy(&object, sizeof(object)));
      }
    }
  }

  if (!strEqual(version_read, BDCFF_VERSION))
    Warn("BDCFF version %s, loaded caveset may have errors.", version_read);

  // check for replays which are problematic
  for (iter = gd_caveset; iter != NULL; iter = iter->next)
    gd_cave_check_replays((GdCave *)iter->data, TRUE, FALSE, FALSE);

  // if there was some error message - return fail XXX
  return TRUE;
}


// ============================================================================
// BDCFF saving
// ============================================================================

#define GD_PTR_ARRAY_MINIMUM_INITIAL_SIZE	64

GdPtrArray *gd_ptr_array_sized_new(unsigned int size)
{
  GdPtrArray *array = checked_calloc(sizeof(GdPtrArray));

  array->data = checked_calloc(size * sizeof(void *));
  array->size_allocated = size;
  array->size_initial = size;
  array->size = 0;

  return array;
}

GdPtrArray *gd_ptr_array_new(void)
{
  return gd_ptr_array_sized_new(GD_PTR_ARRAY_MINIMUM_INITIAL_SIZE);
}

void gd_ptr_array_add(GdPtrArray *array, void *data)
{
  if (array->size == array->size_allocated)
  {
    array->size_allocated += array->size_initial;
    array->data = checked_realloc(array->data, array->size_allocated * sizeof(void *));
  }

  array->data[array->size++] = data;
}

boolean gd_ptr_array_remove(GdPtrArray *array, void *data)
{
  int i, j;

  for (i = 0; i < array->size; i++)
  {
    if (array->data[i] == data)
    {
      checked_free(array->data[i]);

      for (j = i; j < array->size - 1; j++)
	array->data[j] = array->data[j + 1];

      array->size--;

      return TRUE;
    }
  }

  return FALSE;
}

void gd_ptr_array_free(GdPtrArray *array, boolean free_data)
{
  int i;

  if (free_data)
  {
    for (i = 0; i < array->size; i++)
      checked_free(array->data[i]);
  }

  checked_free(array);
}

// ratio: max cave size for GD_TYPE_RATIO. should be set to cave->w*cave->h when calling
static void save_properties(GdPtrArray *out, void *str, void *str_def,
			    const GdStructDescriptor *prop_desc, int ratio)
{
  int i, j;
  boolean parameter_written = FALSE, should_write = FALSE;
  char *line = NULL;
  const char *identifier = NULL;

  for (i = 0; prop_desc[i].identifier != NULL; i++)
  {
    void *value, *default_value;

    // used only by the gui
    if (prop_desc[i].type == GD_TAB || prop_desc[i].type == GD_LABEL)
      continue;

    // these are handled explicitly
    if (prop_desc[i].flags & GD_DONT_SAVE)
      continue;

    // string data
    // write together with identifier, as one string per line.
    if (prop_desc[i].type == GD_TYPE_STRING)
    {
      // treat strings as special - do not even write identifier if no string.
      char *text = STRUCT_MEMBER_P(str, prop_desc[i].offset);

      if (strlen(text) > 0)
	gd_ptr_array_add(out, getStringPrint("%s=%s", prop_desc[i].identifier, text));

      continue;
    }

    // dynamic string: need to escape newlines
    if (prop_desc[i].type == GD_TYPE_LONGSTRING)
    {
      char *string = STRUCT_MEMBER(char *, str, prop_desc[i].offset);

      if (string != NULL && strlen(string) > 0)
      {
	char *escaped = getEscapedString(string);

	gd_ptr_array_add(out, getStringPrint("%s=%s", prop_desc[i].identifier, escaped));

	checked_free(escaped);
      }

      continue;
    }

    // if identifier differs from the previous, write out the line collected, and start a new one
    if (identifier == NULL || !strEqual(prop_desc[i].identifier, identifier))
    {
      if (should_write)
      {
	// write lines only which carry information other than the default settings
	gd_ptr_array_add(out, getStringCopy(line));
      }

      if (prop_desc[i].type == GD_TYPE_EFFECT)
	setStringPrint(&line, "Effect=");
      else
	setStringPrint(&line, "%s=", prop_desc[i].identifier);

      parameter_written = FALSE;    // no value written yet
      should_write = FALSE;

      // remember identifier
      identifier = prop_desc[i].identifier;
    }

    // if we always save this identifier, remember now
    if (prop_desc[i].flags & GD_ALWAYS_SAVE)
      should_write = TRUE;

    value         = STRUCT_MEMBER_P(str,     prop_desc[i].offset);
    default_value = STRUCT_MEMBER_P(str_def, prop_desc[i].offset);

    for (j = 0; j < prop_desc[i].count; j++)
    {
      // separate values by spaces. of course no space required for the first one
      if (parameter_written)
	appendStringPrint(&line, " ");

      parameter_written = TRUE;    // at least one value written, so write space the next time

      switch (prop_desc[i].type)
      {
	case GD_TYPE_BOOLEAN:
	  appendStringPrint(&line, "%s", ((boolean *) value)[j] ? "true" : "false");
	  if (((boolean *) value)[j] != ((boolean *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_INT:
	  appendStringPrint(&line, "%d", ((int *) value)[j]);
	  if (((int *) value)[j] != ((int *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_RATIO:
	  appendStringPrint(&line, "%6.5f", ((int *) value)[j] / (double)ratio);
	  if (((int *) value)[j] != ((int *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_PROBABILITY:
	  // probabilities are stored as *1E6
	  appendStringPrint(&line, "%6.5f", ((int *) value)[j] / 1E6);

	  if (((double *) value)[j] != ((double *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_ELEMENT:
	  appendStringPrint(&line, "%s", gd_element_properties[((GdElement *) value)[j]].filename);
	  if (((GdElement *) value)[j] != ((GdElement *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_EFFECT:
	  // for effects, the property identifier is the effect name.
	  // "Effect=" is hardcoded; see above.
	  appendStringPrint(&line, "%s %s", prop_desc[i].identifier,
			    gd_element_properties[((GdElement *) value)[j]].filename);
	  if (((GdElement *) value)[j] != ((GdElement *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_COLOR:
	  appendStringPrint(&line, "%s", gd_color_get_string(((GdColor *) value)[j]));
	  should_write = TRUE;
	  break;

	case GD_TYPE_DIRECTION:
	  appendStringPrint(&line, "%s", gd_direction_get_filename(((GdDirection *) value)[j]));
	  if (((GdDirection *) value)[j] != ((GdDirection *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TYPE_SCHEDULING:
	  appendStringPrint(&line, "%s", gd_scheduling_get_filename(((GdScheduling *) value)[j]));
	  if (((GdScheduling *) value)[j] != ((GdScheduling *) default_value)[j])
	    should_write = TRUE;
	  break;

	case GD_TAB:
	case GD_LABEL:
	  // used by the editor ui
	  break;

	case GD_TYPE_STRING:
	case GD_TYPE_LONGSTRING:
	  // never reached
	  break;
      }
    }
  }

  // write remaining data
  if (should_write)
    gd_ptr_array_add(out, getStringCopy(line));

  checked_free(line);
}

// remove a line from the list of strings.
// the prefix should be a property; add an equal sign! so properties which have names like
// "slime" and "slimeproperties" won't match each other.
static void cave_properties_remove(GdPtrArray *out, const char *prefix)
{
  int i;

  if (!strSuffix(prefix, "="))
    Warn("string '%s' should have suffix '='", prefix);

  // search for strings which match, and set them to NULL.
  // also free them.
  for (i = 0; i < out->size; i++)
  {
    if (strPrefix(gd_ptr_array_index(out, i), prefix))
    {
      checked_free(gd_ptr_array_index(out, i));
      gd_ptr_array_index(out, i) = NULL;
    }
  }

  // remove all "null" occurrences
  while (gd_ptr_array_remove(out, NULL))
    ;
}

// output properties of a structure to a file.
// list_foreach_fn_2 func, so "out" is the last parameter!
static void caveset_save_cave_func(GdCave *cave, GdPtrArray *out)
{
  GdCave *default_cave;
  GdPtrArray *this_out;
  char *line = NULL;    // used for various purposes
  int i;

  gd_ptr_array_add(out, getStringCopy(""));
  gd_ptr_array_add(out, getStringCopy("[cave]"));

  // first add the properties to a local ptr array.
  // later, some are deleted (slime permeability, for example) - this is needed because of the
  //                                                             inconsistencies of the bdcff.
  // finally, remaining will be added to the normal "out" array.
  this_out = gd_ptr_array_new();

  default_cave = gd_cave_new();
  save_properties(this_out, cave, default_cave, gd_cave_properties, cave->w * cave->h);
  gd_cave_free(default_cave);

  // properties which are handled explicitly. these cannot be handled easily above,
  // as they have some special meaning. for example, slime_permeability=x sets permeability to
  // x, and sets predictable to false. bdcff format is simply inconsistent in these aspects.

  // slime permeability is always set explicitly, as it also sets predictability.
  if (cave->slime_predictable)
    // if slime is predictable, remove permeab. flag, as that would imply unpredictable slime.
    cave_properties_remove(this_out, "SlimePermeability=");
  else
    // if slime is UNpredictable, remove permeabc64 flag, as that would imply predictable slime.
    cave_properties_remove(this_out, "SlimePermeabilityC64=");

  // add tags to output, and free local array
  for (i = 0; i < this_out->size; i++)
    gd_ptr_array_add(out, gd_ptr_array_index(this_out, i));

  // do not free data pointers, which were just added to array "out"
  gd_ptr_array_free(this_out, FALSE);

#if 0
  // save unknown tags as they are
  if (cave->tags)
  {
    List *hashkeys;
    List *iter;

    hashkeys = g_hash_table_get_keys(cave->tags);
    for (iter = hashkeys; iter != NULL; iter = iter->next)
    {
      gchar *key = (gchar *)iter->data;

      gd_ptr_array_add(out, getStringPrint("%s=%s", key, (const char *) g_hash_table_lookup(cave->tags, key)));
    }

    list_free(hashkeys);
  }
#endif

  // map
  if (cave->map)
  {
    int x, y;

    gd_ptr_array_add(out, getStringCopy(""));
    gd_ptr_array_add(out, getStringCopy("[map]"));

    line = checked_calloc(cave->w + 1);

    // save map
    for (y = 0; y < cave->h; y++)
    {
      for (x = 0; x < cave->w; x++)
      {
	// check if character is non-zero;
	// the ...save() should have assigned a character to every element
	if (gd_element_properties[cave->map[y][x]].character_new == 0)
	  Warn("gd_element_properties[cave->map[y][x]].character_new should be non-zero");

	line[x] = gd_element_properties[cave->map[y][x]].character_new;
      }

      gd_ptr_array_add(out, getStringCopy(line));
    }

    gd_ptr_array_add(out, getStringCopy("[/map]"));
  }

  // save drawing objects
  if (cave->objects)
  {
    List *listiter;

    gd_ptr_array_add(out, getStringCopy(""));
    gd_ptr_array_add(out, getStringCopy("[objects]"));

    for (listiter = cave->objects; listiter; listiter = list_next(listiter))
    {
      GdObject *object = listiter->data;
      char *text;

      // not for all levels?
      if (object->levels != GD_OBJECT_LEVEL_ALL)
      {
	int i;
	boolean once;    // true if already written one number

	setStringPrint(&line, "[Level=");
	once = FALSE;

	for (i = 0; i < 5; i++)
	{
	  if (object->levels & gd_levels_mask[i])
	  {
	    if (once)    // if written at least one number so far, we need a comma
	      appendStringPrint(&line, ",");

	    appendStringPrint(&line, "%d", i+1);
	    once = TRUE;
	  }
	}

	appendStringPrint(&line, "]");
	gd_ptr_array_add(out, getStringCopy(line));
      }

      text = gd_object_get_bdcff(object);
      gd_ptr_array_add(out, getStringCopy(text));
      checked_free(text);

      if (object->levels != GD_OBJECT_LEVEL_ALL)
	gd_ptr_array_add(out, getStringCopy("[/Level]"));
    }

    gd_ptr_array_add(out, getStringCopy("[/objects]"));
  }

  gd_ptr_array_add(out, getStringCopy("[/cave]"));

  checked_free(line);
}

// save cave in bdcff format.
// "out" will be added lines of bdcff description.
GdPtrArray *gd_caveset_save_to_bdcff(void)
{
  GdPtrArray *out = gd_ptr_array_sized_new(512);
  GdCavesetData *default_caveset;
  boolean write_mapcodes = FALSE;
  List *iter;
  int i;

  // check if we need an own mapcode table ------
  // copy original characters to character_new fields; new elements will be added to that one
  for (i = 0; i < O_MAX; i++)
    gd_element_properties[i].character_new = gd_element_properties[i].character;

  // also regenerate this table as we use it
  gd_create_char_to_element_table();

  // check all caves
  for (iter = gd_caveset; iter != NULL; iter = iter->next)
  {
    GdCave *cave = (GdCave *)iter->data;

    // if they have a map (random elements+object based maps do not need characters)
    if (cave->map)
    {
      int x, y;

      // check every element of map
      for(y = 0; y < cave->h; y++)
	for (x = 0; x < cave->w; x++)
	{
	  GdElement e = cave->map[y][x];

	  // if no character assigned
	  if (gd_element_properties[e].character_new == 0)
	  {
	    int j;

	    // we found at least one, so later we have to write the mapcodes
	    write_mapcodes = TRUE;

	    // find a character which is not yet used for any element
	    for (j = 32; j < 128; j++)
	    {
	      // the string contains the characters which should not be used.
	      if (strchr("<>&[]/=\\", j) == NULL && gd_char_to_element[j] == O_UNKNOWN)
		break;
	    }

	    // if no more space... XXX we should rather report to the user
	    if (j == 128)
	      Warn("variable j should be != 128");

	    gd_element_properties[e].character_new = j;

	    // we also record to this table, as we use it ^^ a few lines above
	    gd_char_to_element[j] = e;
	  }
	}
    }
  }

  gd_ptr_array_add(out, getStringCopy("[BDCFF]"));
  gd_ptr_array_add(out, getStringPrint("Version=%s", BDCFF_VERSION));

  // this flag was set above if we need to write mapcodes
  if (write_mapcodes)
  {
    int i;

    gd_ptr_array_add(out, getStringCopy("[mapcodes]"));
    gd_ptr_array_add(out, getStringCopy("Length=1"));

    for (i = 0; i < O_MAX; i++)
    {
      // if no character assigned by specification BUT (AND) we assigned one
      if (gd_element_properties[i].character == 0 && gd_element_properties[i].character_new != 0)
	gd_ptr_array_add(out, getStringPrint("%c=%s", gd_element_properties[i].character_new, gd_element_properties[i].filename));
    }

    gd_ptr_array_add(out, getStringCopy("[/mapcodes]"));
  }

  gd_ptr_array_add(out, getStringCopy("[game]"));

  default_caveset = gd_caveset_data_new();
  save_properties(out, gd_caveset_data, default_caveset, gd_caveset_properties, 0);
  gd_caveset_data_free(default_caveset);
  gd_ptr_array_add(out, getStringCopy("Levels=5"));

  list_foreach_fn_2(gd_caveset, (list_fn_2) caveset_save_cave_func, out);

  gd_ptr_array_add(out, getStringCopy("[/game]"));
  gd_ptr_array_add(out, getStringCopy("[/BDCFF]"));

  // saved to ptrarray
  return out;
}
