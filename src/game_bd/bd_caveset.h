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

#ifndef BD_CAVESET_H
#define BD_CAVESET_H


typedef struct _gd_caveset_data
{
  GdString name;                // Name of caveset
  GdString description;         // Some words about the caveset
  GdString author;              // Author
  GdString difficulty;          // difficulty of the caveset, for info purposes
  GdString www;                 // link to author's webpage
  GdString date;                // date of creation

  char *story;                  // story for the caves
  char *remark;                 // notes about the game
    
  char *title_screen;           // base64-encoded title screen image
  char *title_screen_scroll;    // scrolling background for title screen image

  GdString charset;             // these are not used by gdash
  GdString fontset;

  // these are only for a game.
  int initial_lives;            // initial lives at game start
  int maximum_lives;            // maximum lives
  int bonus_life_score;         // bonus life / number of points

  boolean use_krissz_engine;	// for game engine compatibility with Krissz engine

  // and this one the highscores
  GdHighScore highscore[GD_HIGHSCORE_NUM];

  char *levelset_subdir;	// current level set identifier
} GdCavesetData;

extern const GdStructDescriptor gd_caveset_properties[];

extern GdCavesetData *gd_caveset_data;
extern List *gd_caveset;
extern boolean gd_caveset_edited;
extern int gd_caveset_last_selected;
extern int gd_caveset_last_selected_level;

extern char *gd_caveset_extensions[];

// #included cavesets; configdir passed to look for .hsc file
boolean gd_caveset_load_from_internal(int caveset, const char *configdir);
const char **gd_caveset_get_internal_game_names(void);

// caveset load from file
boolean gd_caveset_load_from_file(char *filename);
// caveset save to bdcff file
boolean gd_caveset_save_to_file(const char *filename);

// misc caveset functions
int gd_caveset_count(void);
void gd_caveset_clear(void);
GdCave *gd_return_nth_cave(const int cave);

GdCave *gd_get_original_cave_from_caveset(const int cave);
GdCave *gd_get_prepared_cave_from_caveset(const int cave, const int level);
GdCave *gd_get_prepared_cave(const GdCave *cave, const int level);

// highscore in config directory
void gd_save_highscore(const char* directory);
boolean gd_load_highscore(const char *directory);

GdCavesetData *gd_caveset_data_new(void);
void gd_caveset_data_free(GdCavesetData *data);

// check replays and optionally remove
int gd_cave_check_replays(GdCave *cave, boolean report, boolean remove, boolean repair);

boolean gd_caveset_has_replays(void);

#endif	// BD_CAVESET_H
