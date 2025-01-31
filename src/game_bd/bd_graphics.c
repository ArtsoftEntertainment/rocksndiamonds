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

// optional title screen bitmaps (foreground and background)
static Bitmap *gd_title_screen_bitmaps[2] = { NULL, NULL };

// screen area
Bitmap *gd_screen_bitmap = NULL;

static int play_area_w = 0;
static int play_area_h = 0;

static int scroll_x, scroll_y;

// quit, global variable which is set to true if the application should quit
boolean gd_quit = FALSE;

static int cell_size = 0;

// graphic info for game objects/frames and players/actions/frames
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

void gd_init_play_area(void)
{
  set_play_area(SXSIZE, SYSIZE);
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

static boolean player_out_of_window(GdGame *game, int player_x, int player_y)
{
  // if not yet born, we treat as visible. so cave will run.
  // the user is unable to control an unborn player, so this is the right behaviour.
  if (game->cave->player_state == GD_PL_NOT_YET)
    return FALSE;

  // check if active player is outside drawing area. if yes, we should wait for scrolling
  if ((player_x * cell_size) < scroll_x ||
      (player_x * cell_size + cell_size - 1) > scroll_x + play_area_w)
  {
    // but only do the wait, if the player SHOULD BE visible, ie. he is inside
    // the defined visible area of the cave
    if (game->cave->player_x >= game->cave->x1 &&
	game->cave->player_x <= game->cave->x2)
      return TRUE;
  }

  if ((player_y * cell_size) < scroll_y ||
      (player_y * cell_size + cell_size - 1) > scroll_y + play_area_h)
  {
    // but only do the wait, if the player SHOULD BE visible, ie. he is inside
    // the defined visible area of the cave
    if (game->cave->player_y >= game->cave->y1 &&
	game->cave->player_y <= game->cave->y2)
      return TRUE;
  }

  // player is inside visible window
  return FALSE;
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
  static int scroll_speed_last = -1;
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

  // if player is outside visible playfield area, use faster scrolling
  // (might happen when wrapping around the playfield, teleporting etc.)
  if (player_out_of_window(game, player_x, player_y))
    scroll_speed *= 4;

  // if scrolling started with player outside visible playfield area, keep faster scrolling
  if (scroll_speed_last > scroll_speed)
    scroll_speed = scroll_speed_last;

  // store current (potentially faster) scrolling speed for next time
  scroll_speed_last = scroll_speed;

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

  // if no scrolling required, reset last (potentially faster) scrolling speed
  if (!changed)
    scroll_speed_last = -1;

  // check if active player is visible at the moment.
  // but only if scrolling happened at all!
  if (!changed)
    return FALSE;

  // check if active player is outside drawing area. if yes, we should wait for scrolling.
  return player_out_of_window(game, player_x, player_y);
}

static SDL_Color get_template_color(GdColor color_raw)
{
  GdColor color_rgb = gd_color_get_rgb(color_raw);
  SDL_Color color_sdl;

  color_sdl.r = gd_color_get_r(color_rgb);
  color_sdl.g = gd_color_get_g(color_rgb);
  color_sdl.b = gd_color_get_b(color_rgb);

  return color_sdl;
}

// Copy surface, replace all "pure" RGB colors with BD colors, and keep all other colors.
// (A "pure" RGB color in this sense has the same value for all RGB channels, or zero.)
static SDL_Surface *get_colored_surface_from_template(GdCave *cave, SDL_Surface *surface)
{
  static SDL_Surface *new_surface = NULL;
  static unsigned int *pixels = NULL;
  SDL_PixelFormat *format = surface->format;
  SDL_Color color[MAX_LEVEL_COLORS];
  SDL_Color base_color[MAX_LEVEL_COLORS];
  int color_mapping[MAX_LEVEL_COLORS] = { 0, 1, 4, 3, 5, 2, 6, 7 };
  int width  = surface->w;
  int height = surface->h;
  int bytes_per_pixel = 4;
  int out = 0;
  int i, x, y;

  if (format->BytesPerPixel != bytes_per_pixel)
    Fail("Color template bitmap has wrong color depth -- should not happen");

  if (new_surface != NULL)
    SDL_FreeSurface(new_surface);

  checked_free(pixels);

  pixels = checked_malloc(width * height * bytes_per_pixel);

  SDL_LockSurface(surface);

  // set surface color palette to cave colors
  for (i = 0; i < MAX_LEVEL_COLORS; i++)
  {
    color[i]      = get_template_color(cave->color[color_mapping[i]]);
    base_color[i] = get_template_color(cave->base_color[color_mapping[i]]);
  }

  for (y = 0; y < height; y++)
  {
    unsigned int *p = (unsigned int *)((char *)surface->pixels + y * surface->pitch);

    for (x = 0; x < width; x++)
    {
      int r = (p[x] & format->Rmask) >> format->Rshift << format->Rloss;
      int g = (p[x] & format->Gmask) >> format->Gshift << format->Gloss;
      int b = (p[x] & format->Bmask) >> format->Bshift << format->Bloss;
      int a = (p[x] & format->Amask) >> format->Ashift << format->Aloss;
      int color_value = (r > 0 ? r :
                         g > 0 ? g :
                         b > 0 ? b : 255);

      if ((r == 0 || r == color_value) &&
          (g == 0 || g == color_value) &&
          (b == 0 || b == color_value))
      {
        int index = ((r > 0 ? 1 : 0) +
                     (g > 0 ? 2 : 0) +
                     (b > 0 ? 4 : 0));

        r = (base_color[index].r * (255 - color_value) + color[index].r * color_value) / 255;
        g = (base_color[index].g * (255 - color_value) + color[index].g * color_value) / 255;
        b = (base_color[index].b * (255 - color_value) + color[index].b * color_value) / 255;

        if (index > 5)
          gfx.has_reduced_color_template = FALSE;

        if (color_value > 0 && color_value < 255)
          gfx.has_extended_color_template = TRUE;
      }

      pixels[out++] = ((r << format->Rshift >> format->Rloss) |
                       (g << format->Gshift >> format->Gloss) |
                       (b << format->Bshift >> format->Bloss) |
                       (a << format->Ashift >> format->Aloss));
    }
  }

  SDL_UnlockSurface(surface);

  // create new surface from pixel data
  new_surface = SDL_CreateRGBSurfaceFrom(pixels, width, height, 32, width * bytes_per_pixel,
                                         format->Rmask,
                                         format->Gmask,
                                         format->Bmask,
                                         format->Amask);

  if (new_surface == NULL)
    Fail("SDL_CreateRGBSurfaceFrom() failed: %s", SDL_GetError());

  return new_surface;
}

static Bitmap *get_masked_bitmap_from_surface(GdCave *cave, SDL_Surface *surface)
{
  static Bitmap *bitmap = NULL;

  if (surface == NULL)
    return NULL;

  if (bitmap != NULL)
    FreeBitmap(bitmap);

  // set background color to be transparent for masked bitmap
  int color = gd_color_get_rgb(cave->color[0]);
  int r = gd_color_get_r(color);
  int g = gd_color_get_g(color);
  int b = gd_color_get_b(color);

  // create bitmap from surface
  bitmap = SDLGetBitmapFromSurface_WithMaskedColor(surface, r, g, b);

  return bitmap;
}

Bitmap *gd_get_colored_bitmap_from_template(Bitmap *template_bitmap)
{
  GdCave *cave = native_bd_level.cave;
  SDL_Surface *template_surface = template_bitmap->surface;
  SDL_Surface *colored_surface = get_colored_surface_from_template(cave, template_surface);
  Bitmap *colored_bitmap = get_masked_bitmap_from_surface(cave, colored_surface);

  return colored_bitmap;
}

// returns true if the element has a certain property
static inline boolean has_property(const int element, const int property)
{
  return (gd_element_properties[element].properties & property) != 0;
}

// returns true if the element is a player
static inline boolean el_player(const int element)
{
  return has_property(element, P_PLAYER);
}

// returns true if the element is a player who is pushing some element
static inline boolean el_player_pushing(const int element)
{
  return has_property(element, P_PLAYER_PUSHING);
}

#if 0
// returns true if the element is walkable
static inline boolean el_walkable(const int element)
{
  return has_property(element, P_WALKABLE);
}
#endif

// returns true if the element is diggable
static inline boolean el_diggable(const int element)
{
  return has_property(element, P_DIGGABLE);
}

#if 0
// returns true if the element is collectible
static inline boolean el_collectible(const int element)
{
  return has_property(element, P_COLLECTIBLE);
}
#endif

// returns true if the element is pushable
static inline boolean el_pushable(const int element)
{
  return has_property(element, P_PUSHABLE);
}

// returns true if the element can move
static inline boolean el_can_move(const int element)
{
  return has_property(element, P_CAN_MOVE);
}

// returns true if the element can fall
static inline boolean el_can_fall(const int element)
{
  return has_property(element, P_CAN_FALL);
}

// returns true if the element can grow
static inline boolean el_can_grow(const int element)
{
  return has_property(element, P_CAN_GROW);
}

// returns true if the element can dig
static inline boolean el_can_dig(const int element)
{
  return has_property(element, P_CAN_DIG);
}

// returns true if the element has crumbled graphics
static inline boolean el_is_crumbled(const int element)
{
  int tile_gfx = element;
  int tile_crm = (element == O_DIRT                   ? O_DIRT_CRUMBLED                   :
                  element == O_DIRT2                  ? O_DIRT2_CRUMBLED                  :
                  element == O_DIRT_GLUED             ? O_DIRT_GLUED_CRUMBLED             :
                  element == O_DIRT_SLOPED_UP_RIGHT   ? O_DIRT_SLOPED_UP_RIGHT_CRUMBLED   :
                  element == O_DIRT_SLOPED_UP_LEFT    ? O_DIRT_SLOPED_UP_LEFT_CRUMBLED    :
                  element == O_DIRT_SLOPED_DOWN_LEFT  ? O_DIRT_SLOPED_DOWN_LEFT_CRUMBLED  :
                  element == O_DIRT_SLOPED_DOWN_RIGHT ? O_DIRT_SLOPED_DOWN_RIGHT_CRUMBLED :
                  element == O_BITER_SWITCH_1         ? O_BITER_SWITCH_1_CRUMBLED         :
                  element == O_BITER_SWITCH_2         ? O_BITER_SWITCH_2_CRUMBLED         :
                  element == O_BITER_SWITCH_3         ? O_BITER_SWITCH_3_CRUMBLED         :
                  element == O_BITER_SWITCH_4         ? O_BITER_SWITCH_4_CRUMBLED         :
                  element);
  struct GraphicInfo_BD *gfx = &graphic_info_bd_object[tile_gfx][0];
  struct GraphicInfo_BD *crm = &graphic_info_bd_object[tile_crm][0];

  return (gfx->graphic != crm->graphic);
}

// returns true if the element has animated graphics
static inline boolean el_is_animated(const int element)
{
  return graphic_info_bd_object[element][0].animated;
}

// returns true if the element can fall
static inline boolean el_falling(const int element)
{
  return has_property(element, P_FALLING);
}

// returns true if the element is growing
static inline boolean el_growing(const int element)
{
  return has_property(element, P_GROWING);
}

// returns true if the element is exploding
static inline boolean el_explosion(const int element)
{
  return has_property(element, P_EXPLOSION);
}

static inline boolean el_destroying(const int element)
{
  return (el_growing(element) ||
          el_explosion(element));
}

static inline boolean el_smooth_movable(const int element)
{
  // special case of non-moving player
  if (element == O_PLAYER_PNEUMATIC_LEFT ||
      element == O_PLAYER_PNEUMATIC_RIGHT)
    return FALSE;

  return (el_player(element) ||
          el_can_move(element) ||
          el_can_fall(element) ||
          el_can_grow(element) ||
          el_can_dig(element) ||
          el_falling(element) ||
          el_pushable(element));
}

static int get_dirt_element(int element, int dir, boolean crumbled)
{
  switch (element)
  {
    case O_DIRT:
      return (crumbled ?
              (dir == GD_MV_LEFT  ? O_DIRT_DIGGING_LEFT_CRUMBLED  :
               dir == GD_MV_RIGHT ? O_DIRT_DIGGING_RIGHT_CRUMBLED :
               dir == GD_MV_UP    ? O_DIRT_DIGGING_UP_CRUMBLED    :
               dir == GD_MV_DOWN  ? O_DIRT_DIGGING_DOWN_CRUMBLED  : O_DIRT_CRUMBLED) :

              (dir == GD_MV_LEFT  ? O_DIRT_DIGGING_LEFT   :
               dir == GD_MV_RIGHT ? O_DIRT_DIGGING_RIGHT  :
               dir == GD_MV_UP    ? O_DIRT_DIGGING_UP     :
               dir == GD_MV_DOWN  ? O_DIRT_DIGGING_DOWN   : O_DIRT));

    case O_DIRT2:
      return (crumbled ?
              (dir == GD_MV_LEFT  ? O_DIRT2_DIGGING_LEFT_CRUMBLED  :
               dir == GD_MV_RIGHT ? O_DIRT2_DIGGING_RIGHT_CRUMBLED :
               dir == GD_MV_UP    ? O_DIRT2_DIGGING_UP_CRUMBLED    :
               dir == GD_MV_DOWN  ? O_DIRT2_DIGGING_DOWN_CRUMBLED  : O_DIRT2_CRUMBLED) :

              (dir == GD_MV_LEFT  ? O_DIRT2_DIGGING_LEFT   :
               dir == GD_MV_RIGHT ? O_DIRT2_DIGGING_RIGHT  :
               dir == GD_MV_UP    ? O_DIRT2_DIGGING_UP     :
               dir == GD_MV_DOWN  ? O_DIRT2_DIGGING_DOWN   : O_DIRT2));

    case O_DIRT_GLUED:
      return (crumbled ? O_DIRT_GLUED_CRUMBLED : O_DIRT_GLUED);

    case O_DIRT_SLOPED_UP_RIGHT:
    case O_DIRT_SLOPED_UP_LEFT:
    case O_DIRT_SLOPED_DOWN_LEFT:
    case O_DIRT_SLOPED_DOWN_RIGHT:
      return (crumbled ?
              (dir == GD_MV_LEFT  ? O_DIRT_DIGGING_LEFT_CRUMBLED  :
               dir == GD_MV_RIGHT ? O_DIRT_DIGGING_RIGHT_CRUMBLED :
               dir == GD_MV_UP    ? O_DIRT_DIGGING_UP_CRUMBLED    :
               dir == GD_MV_DOWN  ? O_DIRT_DIGGING_DOWN_CRUMBLED  :
               element == O_DIRT_SLOPED_UP_RIGHT   ? O_DIRT_SLOPED_UP_RIGHT_CRUMBLED   :
               element == O_DIRT_SLOPED_UP_LEFT    ? O_DIRT_SLOPED_UP_LEFT_CRUMBLED    :
               element == O_DIRT_SLOPED_DOWN_LEFT  ? O_DIRT_SLOPED_DOWN_LEFT_CRUMBLED  :
               element == O_DIRT_SLOPED_DOWN_RIGHT ? O_DIRT_SLOPED_DOWN_RIGHT_CRUMBLED : element) :

              (dir == GD_MV_LEFT  ? O_DIRT_DIGGING_LEFT  :
               dir == GD_MV_RIGHT ? O_DIRT_DIGGING_RIGHT :
               dir == GD_MV_UP    ? O_DIRT_DIGGING_UP    :
               dir == GD_MV_DOWN  ? O_DIRT_DIGGING_DOWN  : element));

    case O_BITER_SWITCH_1:
      return (crumbled ? O_BITER_SWITCH_1_CRUMBLED : O_BITER_SWITCH_1);

    case O_BITER_SWITCH_2:
      return (crumbled ? O_BITER_SWITCH_2_CRUMBLED : O_BITER_SWITCH_2);

    case O_BITER_SWITCH_3:
      return (crumbled ? O_BITER_SWITCH_3_CRUMBLED : O_BITER_SWITCH_3);

    case O_BITER_SWITCH_4:
      return (crumbled ? O_BITER_SWITCH_4_CRUMBLED : O_BITER_SWITCH_4);

    default:
      break;
  }

  return element;
}

static void gd_drawcave_crumbled(Bitmap *dest, GdGame *game, int x, int y, boolean draw_masked)
{
  void (*blit_bitmap)(Bitmap *, Bitmap *, int, int, int, int, int, int) =
    (draw_masked ? BlitBitmapMasked : BlitBitmap);
  GdCave *cave = game->cave;
  int sx = x * cell_size - scroll_x;
  int sy = y * cell_size - scroll_y;
  int border_size = cell_size / 8;
  int draw = game->drawing_buffer[y][x];
  int draw_last = game->last_drawing_buffer[y][x];
  int dir_to = game->dir_buffer_to[y][x];
  boolean is_moving_to = (dir_to != GD_MV_STILL);
  boolean is_diggable_last = el_diggable(draw_last);
  boolean is_digging = (is_moving_to && is_diggable_last);
  int tile = (is_digging ? draw_last : draw);
  int tile_gfx = get_dirt_element(tile, dir_to, FALSE);
  int tile_crm = get_dirt_element(tile, dir_to, TRUE);
  int animcycle = game->animcycle;
  int itercycle = MIN(game->itercycle * 8 / game->itermax, 7);
  int frame = (is_digging ? itercycle : animcycle);
  struct GraphicInfo_BD *gfx = &graphic_info_bd_object[tile_gfx][frame];
  struct GraphicInfo_BD *crm = &graphic_info_bd_object[tile_crm][frame];
  int dirs[] = { GD_MV_UP, GD_MV_LEFT, GD_MV_RIGHT, GD_MV_DOWN };
  int i;

  // first draw middle part only (because element might be drawn in masked mode)
  blit_bitmap(gfx->bitmap, dest, gfx->src_x + border_size, gfx->src_y + border_size,
              cell_size - 2 * border_size, cell_size - 2 * border_size,
              sx + border_size, sy + border_size);

  // then draw crumbled sand borders if not next to sand, else draw normal sand borders
  for (i = 0; i < ARRAY_SIZE(dirs); i++)
  {
    int dir = dirs[i];
    int dx = gd_dx[dir];
    int dy = gd_dy[dir];
    int xx = (x + dx + cave->w) % cave->w;
    int yy = (y + dy + cave->h) % cave->h;
    int xoffset = (dx > 0 ? cell_size - border_size : 0);
    int yoffset = (dy > 0 ? cell_size - border_size : 0);
    int xsize = (dx == 0 ? cell_size : border_size);
    int ysize = (dy == 0 ? cell_size : border_size);

    draw = game->drawing_buffer[yy][xx];
    draw_last = game->last_drawing_buffer[yy][xx];
    dir_to = game->dir_buffer_to[yy][xx];
    is_moving_to = (dir_to != GD_MV_STILL);

    // do not crumble border if next tile is also crumbled or is just being digged away
    boolean is_diagonal_movement = (gd_dx[dir_to] != 0 && gd_dy[dir_to] != 0);
    boolean draw_normal = ((el_is_crumbled(draw)) ||
                           (el_is_crumbled(draw_last) && is_moving_to && !is_diagonal_movement));

    // special case: handle sloped sand sides separately
    if ((dir == GD_MV_UP    && (draw == O_DIRT_SLOPED_DOWN_LEFT ||
                                draw == O_DIRT_SLOPED_DOWN_RIGHT)) ||
        (dir == GD_MV_DOWN  && (draw == O_DIRT_SLOPED_UP_LEFT ||
                                draw == O_DIRT_SLOPED_UP_RIGHT)) ||
        (dir == GD_MV_LEFT  && (draw == O_DIRT_SLOPED_UP_RIGHT ||
                                draw == O_DIRT_SLOPED_DOWN_RIGHT)) ||
        (dir == GD_MV_RIGHT && (draw == O_DIRT_SLOPED_UP_LEFT ||
                                draw == O_DIRT_SLOPED_DOWN_LEFT)))
      draw_normal = FALSE;

    if (draw_normal)
      blit_bitmap(gfx->bitmap, dest, gfx->src_x + xoffset, gfx->src_y + yoffset,
                  xsize, ysize, sx + xoffset, sy + yoffset);
    else
      blit_bitmap(crm->bitmap, dest, crm->src_x + xoffset, crm->src_y + yoffset,
                  xsize, ysize, sx + xoffset, sy + yoffset);
  }
}

static void gd_drawcave_tile(Bitmap *dest, GdGame *game, int x, int y, boolean draw_masked)
{
  void (*blit_bitmap)(Bitmap *, Bitmap *, int, int, int, int, int, int) =
    (draw_masked ? BlitBitmapMasked : BlitBitmap);
  GdCave *cave = game->cave;
  int sx = x * cell_size - scroll_x;
  int sy = y * cell_size - scroll_y;
  int dir_from = game->dir_buffer_from[y][x];
  int dir_to = game->dir_buffer_to[y][x];
  int tile = game->element_buffer[y][x];
  int draw = game->drawing_buffer[y][x];
  int tile_last = game->last_element_buffer[y][x];
  int draw_last = game->last_drawing_buffer[y][x];
  int tile_from = O_NONE;	// source element if element is moving (will be set later)
  int draw_from = O_NONE;	// source element if element is moving (will be set later)
  int tile_to = tile;		// target element if element is moving
  int draw_to = draw;		// target element if element is moving
  int frame = game->animcycle;
  int dx_from = gd_dx[dir_from];
  int dy_from = gd_dy[dir_from];
  int dx_to = gd_dx[dir_to];
  int dy_to = gd_dy[dir_to];
  boolean is_moving_from = (dir_from != GD_MV_STILL);
  boolean is_moving_to   = (dir_to   != GD_MV_STILL);
  boolean is_moving = (is_moving_from || is_moving_to);
  boolean is_diagonal_movement_from = (dx_from != 0 && dy_from != 0);
  boolean is_diagonal_movement_to = (dx_to != 0 && dy_to != 0);
  boolean is_diagonal_movement = (is_diagonal_movement_from || is_diagonal_movement_to);
  boolean is_double_movement = (dir_from > GD_MV_TWICE);
  boolean use_smooth_movements = use_bd_smooth_movements();

  // if element is moving away from this tile, determine element that is moving
  if (is_moving_from)
  {
    int new_x = cave->getx(cave, x + dx_from, y + dy_from);
    int new_y = cave->gety(cave, x + dx_from, y + dy_from);
    int new_dir_to = game->dir_buffer_to[new_y][new_x];

    tile_from = game->element_buffer[new_y][new_x];
    draw_from = game->drawing_buffer[new_y][new_x];

    if (is_double_movement || tile_from == O_MAGIC_WALL)
    {
      // for magic wall or slime, use source tile instead of (changed) target tile
      // also required for element falling into magic wall with no space below it
      tile_from = tile_last;
      draw_from = draw_last;
    }

    // handle special case of player running into enemy/explosion from top or left side
    if (el_player(tile_last) && !el_player(tile) && el_destroying(tile_from))
      tile_from = tile_last;

    // handle special case of player digging or snapping clock (which gets replaced by sand)
    if (el_diggable(tile_from) && el_player(tile))
      use_smooth_movements = FALSE;

    // do not use smooth movement if from and to directions are different; for example,
    // player has snapped field, but another element immediately moved to that field
    if (dir_from != new_dir_to)
      use_smooth_movements = FALSE;

    // handle special case of element falling or moving into lava
    if (tile_from == O_LAVA)
    {
      // show element that is moving/falling into lava
      tile_from = tile_last;
      draw_from = draw_last;

      // do not use smooth movement if element not moving into lava (like player snapping lava)
      if (tile == tile_last)
        use_smooth_movements = FALSE;
    }

    // player killed by lava or explosion was standing still before moving into lava or enemy
    if (el_player(tile_from) && !el_player_pushing(draw_from))
      draw_from = (dir_from == GD_MV_LEFT  ? O_PLAYER_LEFT  :
                   dir_from == GD_MV_RIGHT ? O_PLAYER_RIGHT :
                   dir_from == GD_MV_UP    ? O_PLAYER_UP    :
                   dir_from == GD_MV_DOWN  ? O_PLAYER_DOWN  : O_PLAYER);

    // do not use smooth movement if tile has stopped falling and crashed something (like a nut)
    if (el_can_fall(tile) && el_explosion(tile_from))
      use_smooth_movements = FALSE;
  }

  // --------------------------------------------------------------------------------
  // check if we should use smooth movement animations or not
  // --------------------------------------------------------------------------------

  // do not use smooth movement animation for player entering exit (engine stopped)
  if (cave->player_state == GD_PL_EXITED)
    use_smooth_movements = FALSE;

  // do not use smooth movement animation for player stirring the pot
  if (tile_from == O_PLAYER_STIRRING || tile_to == O_PLAYER_STIRRING)
    use_smooth_movements = FALSE;

  // do not use smooth movement animation for growing or exploding game elements
  if (el_growing(tile) || el_explosion(tile))
    use_smooth_movements = FALSE;

  // do not use smooth movement animation for game elements that cannot move smoothly
  // (but handle special case of player digging or snapping diggable element, like sand)
  if (!(is_moving_from && el_smooth_movable(tile_from)) &&
      !(is_moving_to   && el_smooth_movable(tile_to)) &&
      !(is_moving_to   && el_diggable(tile_last)))
    use_smooth_movements = FALSE;

#if DO_GFX_SANITY_CHECK
  if (use_native_bd_graphics_engine() && !setup.small_game_graphics && !program.headless)
  {
    struct GraphicInfo_BD *g = &graphic_info_bd_object[draw][frame];
    int old_x = (game->gfx_buffer[y][x] % GD_NUM_OF_CELLS) % GD_NUM_OF_CELLS_X;
    int old_y = (game->gfx_buffer[y][x] % GD_NUM_OF_CELLS) / GD_NUM_OF_CELLS_X;
    int new_x = g->src_x / g->width;
    int new_y = g->src_y / g->height;

    if (new_x != old_x || new_y != old_y)
    {
      printf("::: BAD ANIMATION FOR TILE %d, FRAME %d [NEW(%d, %d) != OLD(%d, %d)] ['%s']\n",
	     draw, frame,
	     new_x, new_y,
	     old_x, old_y,
	     gd_element_properties[draw].name);
    }
  }
#endif

  // if game element not moving (or no smooth movements requested), simply draw tile
  if (!is_moving || !use_smooth_movements)
  {
    struct GraphicInfo_BD *g = &graphic_info_bd_object[draw][frame];
    int itercycle_capped = MIN(game->itercycle * 8 / game->itermax, 7);

    if (draw == O_PLAYER_BOMB_TURNING ||
        draw == O_PLAYER_ROCKET_LAUNCHER_TURNING)
      g = &graphic_info_bd_object[draw][itercycle_capped];

    if (el_is_crumbled(draw))
      gd_drawcave_crumbled(dest, game, x, y, draw_masked);
    else
      blit_bitmap(g->bitmap, dest, g->src_x, g->src_y, cell_size, cell_size, sx, sy);

    return;
  }

  // --------------------------------------------------------------------------------
  // draw smooth animation for game element moving between two cave tiles
  // --------------------------------------------------------------------------------

  // ---------- 1st step: draw background element for this tile ----------
  {
    // check if player or amoeba is digging or player is snapping a diggable game element
    boolean digging_tile = ((el_can_dig(tile) || tile == O_SPACE) && el_diggable(tile_last));
    int draw_back = (!is_moving_to ? draw : digging_tile ? draw_last : O_SPACE);
    struct GraphicInfo_BD *g = &graphic_info_bd_object[draw_back][frame];

    if (el_is_crumbled(draw_back) && !is_diagonal_movement)
    {
      gd_drawcave_crumbled(dest, game, x, y, draw_masked);

      blit_bitmap = BlitBitmapMasked;
    }
    else
    {
      blit_bitmap(g->bitmap, dest, g->src_x, g->src_y, cell_size, cell_size, sx, sy);
    }
  }

  // get shifted position between cave fields the game element is moving from/to
  int itercycle = MIN(MAX(0, game->itermax - game->itercycle - 1), game->itermax);
  int shift = cell_size * itercycle / game->itermax;

  // ---------- 2nd step: draw element that is moving away from this tile  ----------

  if (is_moving_from && !is_diagonal_movement_from)	// skip if source moving diagonally
  {
    struct GraphicInfo_BD *g = &graphic_info_bd_object[draw_from][frame];
    int xsize = (dx_from != 0 ? shift : cell_size);
    int ysize = (dy_from != 0 ? shift : cell_size);
    int gx = g->src_x + (dx_from < 0 ? cell_size - shift : 0);
    int gy = g->src_y + (dy_from < 0 ? cell_size - shift : 0);
    int tx = sx + (dx_from < 0 ? 0 : dx_from > 0 ? cell_size - shift : 0);
    int ty = sy + (dy_from < 0 ? 0 : dy_from > 0 ? cell_size - shift : 0);

    if (!el_diggable(tile))
      blit_bitmap = BlitBitmapMasked;

    blit_bitmap(g->bitmap, dest, gx, gy, xsize, ysize, tx, ty);

    // when using dynamic scheduling (mainly BD1 levels), redraw tile in next frame
    game->gfx_buffer[y][x] |= GD_REDRAW;
  }

  // ---------- 3rd step: draw element that is moving towards this tile  ----------

  if (is_moving_to)
  {
    if (is_diagonal_movement_to)	// no smooth movement if target moving diagonally
      dx_to = dy_to = 0;

    struct GraphicInfo_BD *g = &graphic_info_bd_object[draw_to][frame];
    int xsize = (dx_to != 0 ? cell_size - shift : cell_size);
    int ysize = (dy_to != 0 ? cell_size - shift : cell_size);
    int gx = g->src_x + (dx_to > 0 ? shift : 0);
    int gy = g->src_y + (dy_to > 0 ? shift : 0);
    int tx = sx + (dx_to > 0 ? 0 : dx_to < 0 ? shift : 0);
    int ty = sy + (dy_to > 0 ? 0 : dy_to < 0 ? shift : 0);

    if (is_moving_from)
      blit_bitmap = BlitBitmapMasked;

    // if player is snapping a diggable game element, draw non-moving "space" element
    // (special case required if "space" element is graphically defined as non-black)
    if (tile == O_SPACE && el_diggable(tile_last))
    {
      gx = (dx_to != 0 ? shift - gx : gx);
      gy = (dy_to != 0 ? shift - gy : gy);
    }

    blit_bitmap(g->bitmap, dest, gx, gy, xsize, ysize, tx, ty);

    // when using dynamic scheduling (mainly BD1 levels), redraw tile in next frame
    game->gfx_buffer[y][x] |= GD_REDRAW;
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
	  el_is_animated(game->drawing_buffer[y][x]) ||
	  el_is_crumbled(game->drawing_buffer[y][x]) ||
	  game->drawing_buffer[y][x] != game->last_drawing_buffer[y][x] ||
	  game->gfx_buffer[y][x] & GD_REDRAW ||
	  game->dir_buffer_from[y][x] != GD_MV_STILL ||
	  game->dir_buffer_to[y][x]   != GD_MV_STILL)
      {
	// now we have drawn it
	game->gfx_buffer[y][x] = game->gfx_buffer[y][x] & ~GD_REDRAW;

	gd_drawcave_tile(dest, game, x, y, draw_masked);
      }
    }
  }

  return 0;
}

static SDL_Surface *get_surface_from_raw_data(const unsigned char *data, int size)
{
  SDL_RWops *rwop = SDL_RWFromConstMem(data, size);
  SDL_Surface *surface = IMG_Load_RW(rwop, 1);	// 1 = automatically closes rwop

  return surface;
}

static SDL_Surface *get_surface_from_base64(const char *base64_data)
{
  int decoded_data_size = base64_decoded_size(base64_data);
  unsigned char *decoded_data = checked_malloc(decoded_data_size);

  base64_decode(decoded_data, base64_data);

  SDL_Surface *surface = get_surface_from_raw_data(decoded_data, decoded_data_size);

  checked_free(decoded_data);

  return surface;
}

static SDL_Surface *get_title_screen_background_surface(SDL_Surface *tile)
{
  if (tile == NULL)
    return NULL;

  SDL_Surface *foreground_surface = gd_title_screen_bitmaps[0]->surface_masked;

  // if foreground image has no transparency, no background image needed
  if (foreground_surface->format->Amask == 0)
    return NULL;

  // use foreground image size for background image size
  int w = foreground_surface->w;
  int h = foreground_surface->h + tile->h;	// background is one scrolling tile higher

  // create background surface
  SDL_Surface *back = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
  int x, y;

  // fill background surface with tile
  for (y = 0; y < h; y += tile->h)
    for (x = 0; x < w; x += tile->w)
      SDLBlitSurface(tile, back, 0, 0, tile->w, tile->h, x, y);

  // background tile surface not needed anymore
  SDL_FreeSurface(tile);

  return back;
}

static SDL_Surface *get_title_screen_surface(int nr)
{
  if (gd_caveset_data == NULL)
    return NULL;

  // do not use title screen background without foreground image
  if (nr == 1 && gd_title_screen_bitmaps[0] == NULL)
    return NULL;

  char *data = (nr == 0 ? gd_caveset_data->title_screen : gd_caveset_data->title_screen_scroll);

  if (data == NULL)
    return NULL;

  SDL_Surface *surface = get_surface_from_base64(data);

  if (surface == NULL)
    return NULL;

  return (nr == 0 ? surface : get_title_screen_background_surface(surface));
}

static void set_title_screen_bitmap(int nr)
{
  if (gd_title_screen_bitmaps[nr] != NULL)
    FreeBitmap(gd_title_screen_bitmaps[nr]);

  gd_title_screen_bitmaps[nr] = NULL;

  SDL_Surface *surface = get_title_screen_surface(nr);

  if (surface == NULL)
    return;

  int width_scaled  = surface->w * 2;
  int height_scaled = surface->h * 2;
  SDL_Surface *surface_scaled = SDLZoomSurface(surface, width_scaled, height_scaled);

  gd_title_screen_bitmaps[nr] = SDLGetBitmapFromSurface(surface_scaled);

  SDL_FreeSurface(surface);
  SDL_FreeSurface(surface_scaled);
}

static void set_title_screen_bitmaps(void)
{
  int i;

  for (i = 0; i < 2; i++)
    set_title_screen_bitmap(i);
}

Bitmap **gd_get_title_screen_bitmaps(void)
{
  static char *levelset_subdir_last = NULL;

  if (gd_caveset_data == NULL || gd_caveset_data->title_screen == NULL)
    return NULL;

  // check if stored cave set is used as current level set (may be outdated)
  if (!strEqual(gd_caveset_data->levelset_subdir, leveldir_current->subdir))
    return NULL;

  // check if stored cave set has changed
  if (!strEqual(gd_caveset_data->levelset_subdir, levelset_subdir_last))
    set_title_screen_bitmaps();

  setString(&levelset_subdir_last, gd_caveset_data->levelset_subdir);

  return gd_title_screen_bitmaps;
}
