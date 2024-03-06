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


// !!! (can be removed later) !!!
#define DO_GFX_SANITY_CHECK	TRUE

// distance to screen edge in cells when scrolling the screen
#define SCROLL_EDGE_DISTANCE	4

// these can't be larger than 31, or they mess up utf8 coding or are the same as some ascii letter
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

// pointer to tile bitmap (which may be prepared with level-specific colors)
static Bitmap *gd_tile_bitmap = NULL;
// pointer to reference tile bitmap (without level-specific colors)
static Bitmap *gd_tile_bitmap_reference = NULL;

// screen area
Bitmap *gd_screen_bitmap = NULL;

static int play_area_w = 0;
static int play_area_h = 0;

static int scroll_x, scroll_y;

// quit, global variable which is set to true if the application should quit
boolean gd_quit = FALSE;

const byte *gd_keystate;

static int cell_size = 0;

// graphic info for game objects/frames and players/actions/frames
struct GraphicInfo_BD graphic_info_bd_object[O_MAX_ALL][8];
// graphic info for game graphics template for level-specific colors
struct GraphicInfo_BD graphic_info_bd_color_template;

static inline int c64_png_colors(int r, int g, int b, int a)
{
  static const int c64_png_cols[] =
  {
    // ABGR

    /* 0000 */ 0,	// transparent
    /* 0001 */ 0,
    /* 0010 */ 0,
    /* 0011 */ 0,
    /* 0100 */ 0,
    /* 0101 */ 0,
    /* 0110 */ 0,
    /* 0111 */ 0,
    /* 1000 */ 1,	// black - background
    /* 1001 */ 2,	// red - foreg1
    /* 1010 */ 5,	// green - amoeba
    /* 1011 */ 4,	// yellow - foreg3
    /* 1100 */ 6,	// blue - slime
    /* 1101 */ 3,	// purple - foreg2
    /* 1110 */ 7,	// black around arrows (used in editor) is coded as cyan
    /* 1111 */ 8,	// white is the arrow
  };

  int c = ((a >> 7) * 8 +
	   (b >> 7) * 4 +
	   (g >> 7) * 2 +
	   (r >> 7) * 1);

  return c64_png_cols[c];
}

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

  // start scrolling when player reaches certain distance to screen edge
  int start = physical_size / 2 - cell_size * edge_distance;

  // scroll so that the player is at the center; the allowed difference is this
  int to = cell_size;

  // if cave size smaller than the screen, no scrolling req'd
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
    // if cave size is only a slightly larger than the screen, also no scrolling
    // scroll to the middle of the cell
    *desired = max / 2;
  }
  else
  {
    if (exact)
    {
      // if exact scrolling, just go exactly to the center.
      *desired = center;
    }
    else
    {
      // hystheresis function.
      // when scrolling left, always go a bit less left than player being at the middle.
      // when scrolling right, always go a bit less to the right.
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

// just set current viewport to upper left.
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

/*
  SCROLLING
  
  scrolls to the player during game play.
  called by drawcave
  returns true, if player is not visible-ie it is out of the visible size in the drawing area.
*/
boolean gd_scroll(GdGame *game, boolean exact_scroll, boolean immediate)
{
  static int scroll_desired_x = 0, scroll_desired_y = 0;
  boolean out_of_window;
  int player_x, player_y, visible_x, visible_y;
  boolean changed;

  // max scrolling speed depends on the speed of the cave.
  // game moves cell_size_game * 1s / cave time pixels in a second.
  // scrolling moves scroll speed * 1s / scroll_time in a second.
  // these should be almost equal; scrolling speed a little slower.
  // that way, the player might reach the border with a small probability,
  // but the scrolling will not "oscillate", ie. turn on for little intervals as it has
  // caught up with the desired position. smaller is better.
  int scroll_speed = cell_size * 20 / game->cave->speed;

  if (!setup.bd_scroll_delay)
    exact_scroll = TRUE;

  if (immediate)
    scroll_speed = cell_size * MAX(game->cave->w, game->cave->h);

  player_x = game->cave->player_x - game->cave->x1; // cell coordinates of player
  player_y = game->cave->player_y - game->cave->y1;

  // pixel size of visible part of the cave (may be smaller in intermissions)
  visible_x = (game->cave->x2 - game->cave->x1 + 1) * cell_size;
  visible_y = (game->cave->y2 - game->cave->y1 + 1) * cell_size;

  // cell_size contains the scaled size, but we need the original.
  changed = FALSE;

  if (cave_scroll(visible_x, play_area_w, player_x * cell_size + cell_size / 2 - play_area_w / 2,
		  exact_scroll, &scroll_x, &scroll_desired_x, scroll_speed))
    changed = TRUE;

  if (cave_scroll(visible_y, play_area_h, player_y * cell_size + cell_size / 2 - play_area_h / 2,
		  exact_scroll, &scroll_y, &scroll_desired_y, scroll_speed))
    changed = TRUE;

  // if scrolling, we should update entire screen.
  if (changed)
  {
    int x, y;

    for (y = 0; y < game->cave->h; y++)
      for (x = 0; x < game->cave->w; x++)
	game->gfx_buffer[y][x] |= GD_REDRAW;
  }

  // check if active player is visible at the moment.
  out_of_window = FALSE;

  // check if active player is outside drawing area. if yes, we should wait for scrolling
  if ((player_x * cell_size) < scroll_x ||
      (player_x * cell_size + cell_size - 1) > scroll_x + play_area_w)
  {
    // but only do the wait, if the player SHOULD BE visible, ie. he is inside
    // the defined visible area of the cave
    if (game->cave->player_x >= game->cave->x1 &&
	game->cave->player_x <= game->cave->x2)
      out_of_window = TRUE;
  }

  if ((player_y * cell_size) < scroll_y ||
      (player_y * cell_size + cell_size - 1) > scroll_y + play_area_h)
    // but only do the wait, if the player SHOULD BE visible, ie. he is inside
    // the defined visible area of the cave
    if (game->cave->player_y >= game->cave->y1 &&
	game->cave->player_y <= game->cave->y2)
      out_of_window = TRUE;

  // if not yet born, we treat as visible. so cave will run.
  // the user is unable to control an unborn player, so this is the right behaviour.
  if (game->cave->player_state == GD_PL_NOT_YET)
    return FALSE;

  return out_of_window;
}

// returns true, if the given surface seems to be a c64 imported image.
static boolean surface_has_c64_colors(SDL_Surface *surface)
{
  boolean has_c64_colors = TRUE;	// default: assume C64 colors
  const unsigned char *p;
  int x, y;

  if (surface->format->BytesPerPixel != 4)
    return FALSE;

  SDL_LockSurface(surface);

  for (y = 0; y < surface->h && has_c64_colors; y++)
  {
    p = ((unsigned char *)surface->pixels) + y * surface->pitch;

    for (x = 0; x < surface->w * surface->format->BytesPerPixel && has_c64_colors; x++)
      if (p[x] != 0 && p[x] != 255)
	has_c64_colors = FALSE;
  }

  SDL_UnlockSurface(surface);

  return has_c64_colors;
}

// sets one of the colors in the indexed palette of an sdl surface to a GdColor.
static void set_surface_palette_color(SDL_Surface *surface, int index, GdColor col)
{
  if (surface->format->BytesPerPixel != 1)
    return;

  SDL_Color c;

  c.r = gd_color_get_r(col);
  c.g = gd_color_get_g(col);
  c.b = gd_color_get_b(col);

  SDL_SetPaletteColors(surface->format->palette, &c, index, 1);
}

// takes a c64_gfx.png-coded 32-bit surface, and creates a paletted surface in our internal format.
static SDL_Surface *get_tile_surface_c64(SDL_Surface *surface, int scale_down_factor)
{
  static SDL_Surface *tile_surface_c64 = NULL;
  static unsigned char *pixels = NULL;
  int width  = surface->w;
  int height = surface->h;
  int out = 0;
  int x, y;

  if (!surface_has_c64_colors(surface))
    return NULL;

  if (surface->format->BytesPerPixel != 4)
    Fail("C64 style surface has wrong color depth -- should not happen");

  if (tile_surface_c64 != NULL)
    SDL_FreeSurface(tile_surface_c64);

  checked_free(pixels);

  pixels = checked_malloc(width * height);

  SDL_LockSurface(surface);

  for (y = 0; y < height; y++)
  {
    unsigned int *p = (unsigned int *)((char *)surface->pixels + y * surface->pitch);

    for (x = 0; x < width; x++)
    {
      int r = (p[x] & surface->format->Rmask) >> surface->format->Rshift << surface->format->Rloss;
      int g = (p[x] & surface->format->Gmask) >> surface->format->Gshift << surface->format->Gloss;
      int b = (p[x] & surface->format->Bmask) >> surface->format->Bshift << surface->format->Bloss;
      // should be:
      // a = (p[x]&surface->format->Amask) >> surface->format->Ashift << surface->format->Aloss;
      // but we do not use the alpha channel in sdash, so we just use 255 (max alpha)

      pixels[out++] = c64_png_colors(r, g, b, 255);
    }
  }

  SDL_UnlockSurface(surface);

  // create new surface from pixel data
  tile_surface_c64 =
    SDL_CreateRGBSurfaceFrom((void *)pixels, width, height, 8, width, 0, 0, 0, 0);

  if (tile_surface_c64 == NULL)
    Fail("SDL_CreateRGBSurfaceFrom() failed: %s", SDL_GetError());

  if (scale_down_factor > 1)
  {
    SDL_Surface *surface_old = tile_surface_c64;
    int width_scaled  = width  / scale_down_factor;
    int height_scaled = height / scale_down_factor;

    // replace surface with scaled-down variant
    tile_surface_c64 = SDLZoomSurface(surface_old, width_scaled, height_scaled);

    // free previous (non-scaled) surface
    SDL_FreeSurface(surface_old);
  }

  return tile_surface_c64;
}

static Bitmap *get_tile_bitmap_c64(GdCave *cave, SDL_Surface *surface)
{
  static Bitmap *tile_bitmap_c64 = NULL;

  if (surface == NULL)
    return NULL;

  if (tile_bitmap_c64 != NULL)
    FreeBitmap(tile_bitmap_c64);

  // set surface color palette to cave colors
  set_surface_palette_color(surface, 0, 0);
  set_surface_palette_color(surface, 1, gd_color_get_rgb(cave->color0));
  set_surface_palette_color(surface, 2, gd_color_get_rgb(cave->color1));
  set_surface_palette_color(surface, 3, gd_color_get_rgb(cave->color2));
  set_surface_palette_color(surface, 4, gd_color_get_rgb(cave->color3));
  set_surface_palette_color(surface, 5, gd_color_get_rgb(cave->color4));
  set_surface_palette_color(surface, 6, gd_color_get_rgb(cave->color5));
  set_surface_palette_color(surface, 7, 0);
  set_surface_palette_color(surface, 8, 0);

  // create bitmap from C64 surface
  tile_bitmap_c64 = SDLGetBitmapFromSurface(surface);

  return tile_bitmap_c64;
}

void gd_prepare_tile_bitmap(GdCave *cave, Bitmap *bitmap, int scale_down_factor)
{
  static SDL_Surface *tile_surface_c64 = NULL;
  static Bitmap *gd_tile_bitmap_original = NULL;
  static int scale_down_factor_last = -1;

  // check if tile bitmap has changed (different artwork or tile size selected)
  if (bitmap != gd_tile_bitmap_original || scale_down_factor != scale_down_factor_last)
  {
    // check if tile bitmap has limited C64 style colors
    tile_surface_c64 = get_tile_surface_c64(bitmap->surface, scale_down_factor);

    // store original tile bitmap from current artwork set and scaling factor
    gd_tile_bitmap_original = bitmap;
    scale_down_factor_last = scale_down_factor;

    // store reference tile bitmap from current artwork set (may be changed later)
    gd_tile_bitmap_reference = bitmap;
  }

  // check if tile bitmap should be colored for next game
  if (tile_surface_c64 != NULL)
  {
    // set tile bitmap to bitmap with level-specific colors
    gd_tile_bitmap = get_tile_bitmap_c64(cave, tile_surface_c64);
  }
  else
  {
    // no special tile bitmap available for this artwork set
    gd_tile_bitmap = NULL;
  }
}

void gd_set_tile_bitmap_reference(Bitmap *bitmap)
{
  gd_tile_bitmap_reference = bitmap;
}

Bitmap *gd_get_tile_bitmap(Bitmap *bitmap)
{
  // if no special tile bitmap available, keep using default tile bitmap
  if (gd_tile_bitmap == NULL)
    return bitmap;

  // if default bitmap refers to tile bitmap, use special tile bitmap
  if (bitmap == gd_tile_bitmap_reference)
    return gd_tile_bitmap;

  return bitmap;
}

#if DO_GFX_SANITY_CHECK
// workaround to prevent variable name scope problem
static boolean use_native_bd_graphics_engine(void)
{
  return game.use_native_bd_graphics_engine;
}
#endif

// returns true if the element is a player
static inline boolean is_player(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_PLAYER) != 0;
}

// returns true if the element is diggable
static inline boolean is_diggable(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_DIGGABLE) != 0;
}

// returns true if the element is collectible
static inline boolean is_collectible(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_COLLECTIBLE) != 0;
}

// returns true if the element is pushable
static inline boolean is_pushable(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_PUSHABLE) != 0;
}

// returns true if the element can move
static inline boolean can_move(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_CAN_MOVE) != 0;
}

// returns true if the element can fall
static inline boolean can_fall(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_CAN_FALL) != 0;
}

// returns true if the element is exploding
static inline boolean is_explosion(const int element)
{
  return (gd_elements[element & O_MASK].properties & P_EXPLOSION) != 0;
}

static void gd_drawcave_tile(Bitmap *dest, GdGame *game, int x, int y, boolean draw_masked)
{
  void (*blit_bitmap)(Bitmap *, Bitmap *, int, int, int, int, int, int) =
    (draw_masked ? BlitBitmapMasked : BlitBitmap);
  GdCave *cave = game->cave;
  int sx = x * cell_size - scroll_x;
  int sy = y * cell_size - scroll_y;
  int dir = game->dir_buffer[y][x];
  int tile = game->element_buffer[y][x];
  int frame = game->animcycle;
  struct GraphicInfo_BD *g = &graphic_info_bd_object[tile][frame];
  Bitmap *tile_bitmap = gd_get_tile_bitmap(g->bitmap);
  boolean is_movable = (can_move(tile) || can_fall(tile) || is_pushable(tile) || is_player(tile));
  boolean is_movable_or_diggable = (is_movable || is_diggable(game->last_element_buffer[y][x]));
  boolean is_moving = (is_movable_or_diggable && dir != GD_MV_STILL);
  boolean use_smooth_movements =
    ((setup.bd_smooth_movements == TRUE) ||
     (setup.bd_smooth_movements == AUTO && !use_native_bd_graphics_engine()));

  // do not use smooth movement animation for exploding game elements (like player)
  if (is_explosion(tile) && dir != GD_MV_STILL)
    use_smooth_movements = FALSE;

  // do not use smooth movement animation for player entering exit (engine stopped)
  if (cave->player_state == GD_PL_EXITED)
    use_smooth_movements = FALSE;

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

  // if game element not moving (or no smooth movements requested), simply draw tile
  if (!is_moving || !use_smooth_movements)
  {
    blit_bitmap(tile_bitmap, dest, g->src_x, g->src_y, cell_size, cell_size, sx, sy);

    return;
  }

  // draw smooth animation for game element moving between two cave tiles

  if (!(game->last_element_buffer[y][x] & SKIPPED))
  {
    // redraw previous game element on the cave field the new element is moving to
    int tile_last = game->last_element_buffer[y][x];

    // only redraw previous game element if it is not collectible (like dirt etc.)
    if (is_collectible(tile_last))
      tile_last = O_SPACE;

    struct GraphicInfo_BD *g_old = &graphic_info_bd_object[tile_last][frame];
    Bitmap *tile_bitmap_old = gd_get_tile_bitmap(g_old->bitmap);

    blit_bitmap(tile_bitmap_old, dest, g_old->src_x, g_old->src_y, cell_size, cell_size, sx, sy);
  }

  // get cave field position the game element is moving from
  int dx = (dir == GD_MV_LEFT ? +1 : dir == GD_MV_RIGHT ? -1 : 0);
  int dy = (dir == GD_MV_UP   ? +1 : dir == GD_MV_DOWN  ? -1 : 0);
  int old_x = cave->getx(cave, x + dx, y + dy);
  int old_y = cave->gety(cave, x + dx, y + dy);
  int tile_from = game->element_buffer[old_y][old_x] & ~SKIPPED;   // should never be skipped
  struct GraphicInfo_BD *g_from = &graphic_info_bd_object[tile_from][frame];
  Bitmap *tile_bitmap_from = gd_get_tile_bitmap(g_from->bitmap);
  boolean old_is_player = is_player(tile_from);
  boolean old_is_moving = (game->dir_buffer[old_y][old_x] != GD_MV_STILL);
  boolean old_is_visible = (old_x >= cave->x1 &&
			    old_x <= cave->x2 &&
			    old_y >= cave->y1 &&
			    old_y <= cave->y2);
  if (old_is_visible)
  {
    if (!old_is_moving && !old_is_player)
    {
      // redraw game element on the cave field the element is moving from
      blit_bitmap(tile_bitmap_from, dest, g_from->src_x, g_from->src_y, cell_size, cell_size,
		  sx + dx * cell_size, sy + dy * cell_size);

      game->element_buffer[old_y][old_x] |= SKIPPED;
    }
    else
    {
      // if old tile also moving (like pushing player), do not redraw tile background
      game->last_element_buffer[old_y][old_x] |= SKIPPED;
    }
  }

  // get shifted position between cave fields the game element is moving from/to
  int itercycle = MIN(MAX(0, game->itermax - game->itercycle - 1), game->itermax);
  int shift = cell_size * itercycle / game->itermax;

  blit_bitmap(tile_bitmap, dest, g->src_x, g->src_y, cell_size, cell_size,
	      sx + dx * shift, sy + dy * shift);

  // special case: redraw player snapping a game element
  if (old_is_visible && old_is_player && !old_is_moving)
  {
    // redraw game element on the cave field the element is moving from
    blit_bitmap(tile_bitmap_from, dest, g_from->src_x, g_from->src_y, cell_size, cell_size,
		sx + dx * cell_size, sy + dy * cell_size);
  }
}

int gd_drawcave(Bitmap *dest, GdGame *game, boolean force_redraw)
{
  GdCave *cave = game->cave;
  static int show_flash_count = 0;
  boolean show_flash = FALSE;
  boolean draw_masked = FALSE;
  boolean redraw_all = force_redraw;
  int x, y;

  // force redraw if maximum number of cycles has changed (to redraw moving elements)
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

    draw_masked = TRUE;
    redraw_all = TRUE;
  }

  // here we draw all cells to be redrawn. we do not take scrolling area into
  // consideration - sdl will do the clipping.
  for (y = cave->y1; y <= cave->y2; y++)
  {
    for (x = cave->x1; x <= cave->x2; x++)
    {
      if (redraw_all ||
	  game->gfx_buffer[y][x] & GD_REDRAW ||
	  game->dir_buffer[y][x] != GD_MV_STILL)
      {
	// skip redrawing already drawn element with movement
	if (game->element_buffer[y][x] & SKIPPED)
	  continue;

	// now we have drawn it
	game->gfx_buffer[y][x] = game->gfx_buffer[y][x] & ~GD_REDRAW;

	gd_drawcave_tile(dest, game, x, y, draw_masked);
      }
    }
  }

  return 0;
}
