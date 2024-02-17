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

#include "main_bd.h"


// !!! (can be removed later) !!!
#define DO_GFX_SANITY_CHECK	TRUE

/* distance to screen edge in cells when scrolling the screen */
#define SCROLL_EDGE_DISTANCE	4

/* these can't be larger than 31, or they mess up utf8 coding or are the same
   as some ascii letter */
#define GD_DOWN_CHAR		1
#define GD_LEFT_CHAR		2
#define GD_UP_CHAR		3
#define GD_RIGHT_CHAR		4

#define GD_BALL_CHAR		5
#define GD_UNCHECKED_BOX_CHAR	6
#define GD_CHECKED_BOX_CHAR	7

#define GD_PLAYER_CHAR		8
#define GD_DIAMOND_CHAR		9
#define GD_SKELETON_CHAR	11
#define GD_KEY_CHAR		12
#define GD_COMMENT_CHAR		13

/* screen area */
Bitmap *gd_screen_bitmap = NULL;

static int play_area_w = 0;
static int play_area_h = 0;

static int scroll_x, scroll_y;

/* quit, global variable which is set to true if the application should quit */
boolean gd_quit = FALSE;

const guint8 *gd_keystate;

static int cell_size = 0;

/* graphic info for game objects/frames and players/actions/frames */
struct GraphicInfo_BD graphic_info_bd_object[O_MAX_ALL][8];

void set_cell_size(int s)
{
  cell_size = s;
}

void set_play_area(int w, int h)
{
  play_area_w = w;
  play_area_h = h;
}

void gd_init_keystate(void)
{
  set_play_area(SXSIZE, SYSIZE);

  gd_keystate = SDL_GetKeyboardState(NULL);
}

/*
  logical_size: logical pixel size of playfield, usually larger than the screen.
  physical_size: visible part. (remember: player_x-x1!)

  center: the coordinates to scroll to.
  exact: scroll exactly
  start: start scrolling if difference is larger than
  to: scroll to, if started, until difference is smaller than
  current

  desired: the function stores its data here
  speed: the function stores its data here

  cell_size: size of one cell. used to determine if the play field is only a
  slightly larger than the screen, in that case no scrolling is desirable
*/
static boolean cave_scroll(int logical_size, int physical_size, int center, boolean exact,
			   int *current, int *desired, int speed)
{
  int max = MAX(0, logical_size - physical_size);
  int edge_distance = SCROLL_EDGE_DISTANCE;
  int cell_size = TILESIZE_VAR;
  boolean changed = FALSE;

  /* start scrolling when player reaches certain distance to screen edge */
  int start = physical_size / 2 - cell_size * edge_distance;

  /* scroll so that the player is at the center; the allowed difference is this */
  int to = cell_size;

  /* if cave size smaller than the screen, no scrolling req'd */
  if (logical_size < physical_size)
  {
    *desired = 0;

    if (*current != 0)
    {
      *current = 0;
      changed = TRUE;
    }

    return changed;
  }

  if (logical_size <= physical_size + cell_size)
  {
    /* if cave size is only a slightly larger than the screen, also no scrolling */
    /* scroll to the middle of the cell */
    *desired = max / 2;
  }
  else
  {
    if (exact)
    {
      /* if exact scrolling, just go exactly to the center. */
      *desired = center;
    }
    else
    {
      /* hystheresis function.
       * when scrolling left, always go a bit less left than player being at the middle.
       * when scrolling right, always go a bit less to the right. */
      if (*current < center - start)
	*desired = center - to;
      if (*current > center + start)
	*desired = center + to;
    }
  }

  *desired = MIN(MAX(0, *desired), max);

  if (*current < *desired)
  {
    *current = MIN(*current + speed, *desired);

    changed = TRUE;
  }

  if (*current > *desired)
  {
    *current = MAX(*current - speed, *desired);

    changed = TRUE;
  }

  return changed;
}

/* just set current viewport to upper left. */
void gd_scroll_to_origin(void)
{
  scroll_x = 0;
  scroll_y = 0;
}

int get_scroll_x(void)
{
  return scroll_x / cell_size;
}

int get_scroll_y(void)
{
  return scroll_y / cell_size;
}

int get_play_area_w(void)
{
  return play_area_w / cell_size;
}

int get_play_area_h(void)
{
  return play_area_h / cell_size;
}

/* SCROLLING
 *
 * scrolls to the player during game play.
 * called by drawcave
 * returns true, if player is not visible-ie it is out of the visible size in the drawing area.
 */
boolean gd_scroll(GdGame *game, boolean exact_scroll, boolean immediate)
{
  static int scroll_desired_x = 0, scroll_desired_y = 0;
  boolean out_of_window;
  int player_x, player_y, visible_x, visible_y;
  boolean changed;
  int scroll_divisor;

  /* max scrolling speed depends on the speed of the cave. */
  /* game moves cell_size_game * 1s / cave time pixels in a second. */
  /* scrolling moves scroll speed * 1s / scroll_time in a second. */
  /* these should be almost equal; scrolling speed a little slower. */
  /* that way, the player might reach the border with a small probability, */
  /* but the scrolling will not "oscillate", ie. turn on for little intervals as it has */
  /* caught up with the desired position. smaller is better. */
  int scroll_speed = cell_size * 20 / game->cave->speed;

  if (immediate)
    scroll_speed = cell_size * MAX(game->cave->w, game->cave->h);

  player_x = game->cave->player_x - game->cave->x1; /* cell coordinates of player */
  player_y = game->cave->player_y - game->cave->y1;

  /* pixel size of visible part of the cave (may be smaller in intermissions) */
  visible_x = (game->cave->x2 - game->cave->x1 + 1) * cell_size;
  visible_y = (game->cave->y2 - game->cave->y1 + 1) * cell_size;

  /* cell_size contains the scaled size, but we need the original. */
  changed = FALSE;

  /* some sort of scrolling speed.
     with larger cells, the divisor must be smaller, so the scrolling faster. */
  scroll_divisor = 256 / cell_size;

  /* fine scrolling is 50hz (normal would be 25hz only) */
  scroll_divisor *= 2;

  if (cave_scroll(visible_x, play_area_w, player_x * cell_size + cell_size / 2 - play_area_w / 2,
		  exact_scroll, &scroll_x, &scroll_desired_x, scroll_speed))
    changed = TRUE;

  if (cave_scroll(visible_y, play_area_h, player_y * cell_size + cell_size / 2 - play_area_h / 2,
		  exact_scroll, &scroll_y, &scroll_desired_y, scroll_speed))
    changed = TRUE;

  /* if scrolling, we should update entire screen. */
  if (changed)
  {
    int x, y;

    for (y = 0; y < game->cave->h; y++)
      for (x = 0; x < game->cave->w; x++)
	game->gfx_buffer[y][x] |= GD_REDRAW;
  }

  /* check if active player is visible at the moment. */
  out_of_window = FALSE;

  /* check if active player is outside drawing area. if yes, we should wait for scrolling */
  if ((player_x * cell_size) < scroll_x ||
      (player_x * cell_size + cell_size - 1) > scroll_x + play_area_w)
  {
    /* but only do the wait, if the player SHOULD BE visible, ie. he is inside
       the defined visible area of the cave */
    if (game->cave->player_x >= game->cave->x1 &&
	game->cave->player_x <= game->cave->x2)
      out_of_window = TRUE;
  }

  if ((player_y * cell_size) < scroll_y ||
      (player_y * cell_size + cell_size - 1) > scroll_y + play_area_h)
    /* but only do the wait, if the player SHOULD BE visible, ie. he is inside
       the defined visible area of the cave */
    if (game->cave->player_y >= game->cave->y1 &&
	game->cave->player_y <= game->cave->y2)
      out_of_window = TRUE;

  /* if not yet born, we treat as visible. so cave will run.
     the user is unable to control an unborn player, so this is the right behaviour. */
  if (game->cave->player_state == GD_PL_NOT_YET)
    return FALSE;

  return out_of_window;
}

#if DO_GFX_SANITY_CHECK
/* workaround to prevent variable name scope problem */
static boolean use_native_bd_graphics_engine(void)
{
  return game.use_native_bd_graphics_engine;
}
#endif

int gd_drawcave(Bitmap *dest, GdGame *game, boolean force_redraw)
{
  GdCave *cave = game->cave;
  void (*blit_bitmap)(Bitmap *, Bitmap *, int, int, int, int, int, int) = BlitBitmap;
  static int show_flash_count = 0;
  boolean show_flash = FALSE;
  boolean redraw_all = force_redraw;
  int scroll_y_aligned = scroll_y;
  int x, y, xd, yd;

  /* force redraw if maximum number of cycles has changed (to redraw moving elements) */
  if (game->itermax != game->itermax_last)
    redraw_all = TRUE;

  if (!cave->gate_open_flash)
  {
    show_flash_count = 0;
  }
  else
  {
    if (show_flash_count++ < 4)
      show_flash = TRUE;

    redraw_all = TRUE;
  }

  if (show_flash)
  {
    FillRectangle(dest, 0, 0, SXSIZE, SYSIZE, WHITE_PIXEL);

    blit_bitmap = BlitBitmapMasked;
    redraw_all = TRUE;
  }

  /* here we draw all cells to be redrawn. we do not take scrolling area into
     consideration - sdl will do the clipping. */
  for (y = cave->y1, yd = 0; y <= cave->y2; y++, yd++)
  {
    for (x = cave->x1, xd = 0; x <= cave->x2; x++, xd++)
    {
      /* potential movement direction of game element */
      int dir = game->dir_buffer[y][x];

      if (redraw_all || game->gfx_buffer[y][x] & GD_REDRAW || dir != GD_MV_STILL)
      {
	/* skip redrawing already drawn element with movement */
	if (game->element_buffer[y][x] & SKIPPED)
	  continue;

	/* if it needs to be redrawn */
	SDL_Rect offset;

	/* sdl_blitsurface destroys offset, so we have to set y here, too.
	   (ie. in every iteration) */
	offset.y = y * cell_size - scroll_y_aligned;
	offset.x = x * cell_size - scroll_x;

	/* now we have drawn it */
	game->gfx_buffer[y][x] = game->gfx_buffer[y][x] & ~GD_REDRAW;

	int tile = game->element_buffer[y][x];
	int frame = game->animcycle;
	struct GraphicInfo_BD *g = &graphic_info_bd_object[tile][frame];
	int width  = g->width  * TILESIZE_VAR / TILESIZE;
	int height = g->height * TILESIZE_VAR / TILESIZE;
	boolean use_smooth_movements = TRUE;

	/* if game element is just moving, draw movement animation between two tiles */
	if (use_smooth_movements && dir != GD_MV_STILL)
	{
	  if (!(game->last_element_buffer[y][x] & SKIPPED))
	  {
	    /* redraw previous game element on the cave field the new element is moving to */
	    int tile_old = game->last_element_buffer[y][x] & ~SKIPPED;
	    struct GraphicInfo_BD *g_old = &graphic_info_bd_object[tile_old][frame];

	    blit_bitmap(g_old->bitmap, dest, g_old->src_x, g_old->src_y, width, height,
			offset.x, offset.y);
	  }

	  /* get cave field position the game element is moving from */
	  int dx = (dir == GD_MV_LEFT ? +1 : dir == GD_MV_RIGHT ? -1 : 0);
	  int dy = (dir == GD_MV_UP   ? +1 : dir == GD_MV_DOWN  ? -1 : 0);
	  int old_x = cave->getx(cave, x + dx, y + dy);
	  int old_y = cave->gety(cave, x + dx, y + dy);

	  if (old_x >= cave->x1 &&
	      old_x <= cave->x2 &&
	      old_y >= cave->y1 &&
	      old_y <= cave->y2)
	  {
	    if (game->dir_buffer[old_y][old_x] == GD_MV_STILL)
	    {
	      /* redraw game element on the cave field the element is moving from */
	      int tile_from = game->element_buffer[old_y][old_x];
	      struct GraphicInfo_BD *g_from = &graphic_info_bd_object[tile_from][0];

	      blit_bitmap(g_from->bitmap, dest, g_from->src_x, g_from->src_y, width, height,
			  offset.x + dx * cell_size, offset.y + dy * cell_size);

	      game->element_buffer[old_y][old_x] |= SKIPPED;
	    }
	    else
	    {
	      /* if old tile also moving (like pushing player), do not redraw it again */
	      game->last_element_buffer[old_y][old_x] |= SKIPPED;
	    }
	  }

	  /* get shifted position between cave fields the game element is moving from/to */
	  int itercycle = MIN(MAX(0, game->itermax - game->itercycle - 1), game->itermax);
	  int shift = cell_size * itercycle / game->itermax;

	  offset.x += dx * shift;
	  offset.y += dy * shift;
	}

	blit_bitmap(g->bitmap, dest, g->src_x, g->src_y, width, height, offset.x, offset.y);

#if DO_GFX_SANITY_CHECK
	if (use_native_bd_graphics_engine() && !setup.small_game_graphics && !program.headless)
	{
	  int old_x = (game->gfx_buffer[y][x] % GD_NUM_OF_CELLS) % GD_NUM_OF_CELLS_X;
	  int old_y = (game->gfx_buffer[y][x] % GD_NUM_OF_CELLS) / GD_NUM_OF_CELLS_X;
	  int new_x = g->src_x / g->width;
	  int new_y = g->src_y / g->height;

	  if (new_x != old_x || new_y != old_y)
	  {
	    printf("::: BAD ANIMATION FOR TILE %d, FRAME %d [NEW(%d, %d) != OLD(%d, %d)] ['%s']\n",
		   tile, frame,
		   new_x, new_y,
		   old_x, old_y,
		   gd_elements[tile].name);
	  }
	}
#endif
      }
    }
  }

  return 0;
}
