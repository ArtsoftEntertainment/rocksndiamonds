// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_tools.c
// ============================================================================

#include <time.h>

#include "main_mm.h"

#include "mm_main.h"
#include "mm_tools.h"


void SetDrawtoField_MM(int mode)
{
  int full_xsize = lev_fieldx * TILESIZE_VAR;
  int full_ysize = lev_fieldy * TILESIZE_VAR;

  // distance (delta) from screen border (SX/SY) to centered level playfield
  dSX = (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);
  dSY = (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  // for convenience, absolute screen position to centered level playfield
  cSX = SX + dSX;
  cSY = SY + dSY;
  cSX2 = SX + dSX + 2;		// including half laser line size
  cSY2 = SY + dSY + 2;		// including half laser line size

  if (mode == DRAW_TO_BACKBUFFER)
  {
    cFX = FX + dSX;
    cFY = FY + dSY;
  }

  SetTileCursorSXSY(cSX, cSY);
}

void BackToFront_MM(void)
{
  BlitScreenToBitmap_MM(backbuffer);

  BackToFront();
}

void ClearWindow(void)
{
  ClearRectangle(drawto_mm, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  SetDrawtoField(DRAW_TO_BACKBUFFER);
  SetDrawtoField_MM(DRAW_TO_BACKBUFFER);

  redraw_mask |= REDRAW_FIELD;
}

void DrawGraphicAnimation_MM(int x, int y, int graphic, int frame)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &bitmap, &src_x, &src_y);

  BlitBitmap(bitmap, drawto_mm, src_x, src_y, TILEX, TILEY,
	     cFX + x * TILEX, cFY + y * TILEY);
}

void DrawGraphic_MM(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("game:mm:DrawGraphic_MM", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("game:mm:DrawGraphic_MM", "This should never happen!");

    return;
  }
#endif

  int frame = getGraphicAnimationFrameXY(graphic, x, y);

  DrawGraphicAnimation_MM(x, y, graphic, frame);

  MarkTileDirty(x, y);
}

void DrawGraphicExt_MM(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, 0, &bitmap, &src_x, &src_y);

  BlitBitmap(bitmap, d, src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask_MM(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("game:mm:DrawGraphicThruMask_MM", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("game:mm:DrawGraphicThruMask_MM", "This should never happen!");

    return;
  }
#endif

  DrawGraphicThruMaskExt_MM(drawto_mm, cFX + x * TILEX, cFY + y * TILEY,
			    graphic, frame);

  MarkTileDirty(x, y);
}

void DrawGraphicThruMaskExt_MM(DrawBuffer *d, int dest_x, int dest_y,
			       int graphic, int frame)
{
  int src_x, src_y;
  Bitmap *src_bitmap;

  if (graphic == IMG_EMPTY)
    return;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY, dest_x, dest_y);
}

void DrawMiniGraphic_MM(int x, int y, int graphic)
{
  DrawMiniGraphicExt_MM(drawto_mm, cSX + x * MINI_TILEX, cSY + y * MINI_TILEY,
			graphic);

  MarkTileDirty(x / 2, y / 2);
}

void DrawMiniGraphicExt_MM(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &bitmap, &src_x, &src_y);

  BlitBitmap(bitmap, d, src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

void DrawGraphicShifted_MM(int x, int y, int dx, int dy, int graphic,
			   int cut_mode, int mask_mode)
{
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;
  int src_x, src_y, dest_x, dest_y;
  Bitmap *src_bitmap;

  if (graphic < 0)
  {
    DrawGraphic_MM(x, y, graphic);

    return;
  }

  if (dx || dy)			// Verschiebung der Grafik?
  {
    if (x < BX1)		// Element kommt von links ins Bild
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		// Element kommt von rechts ins Bild
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x==BX1 && dx < 0)	// Element verläßt links das Bild
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx > 0)	// Element verläßt rechts das Bild
      width -= dx;
    else if (dx)		// allg. Bewegung in x-Richtung
      MarkTileDirty(x + SIGN(dx), y);

    if (y < BY1)		// Element kommt von oben ins Bild
    {
      if (cut_mode==CUT_BELOW)	// Element oberhalb des Bildes
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		// Element kommt von unten ins Bild
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y==BY1 && dy < 0)	// Element verläßt oben das Bild
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		// Element unterhalb des Bildes
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				// Element verläßt unten das Bild
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
    {
      height -= dy;
    }
    else if (dy)		// allg. Bewegung in y-Richtung
    {
      MarkTileDirty(x, y + SIGN(dy));
    }
  }

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  src_x += cx;
  src_y += cy;

  dest_x = cFX + x * TILEX + dx;
  dest_y = cFY + y * TILEY + dy;

#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("game:mm:DrawGraphicShifted_MM", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("game:mm:DrawGraphicShifted_MM", "This should never happen!");

    return;
  }
#endif

  if (mask_mode == USE_MASKING)
    BlitBitmapMasked(src_bitmap, drawto_mm,
		     src_x, src_y, TILEX, TILEY, dest_x, dest_y);
  else
    BlitBitmap(src_bitmap, drawto_mm,
	       src_x, src_y, width, height, dest_x, dest_y);

  MarkTileDirty(x, y);
}

void DrawScreenElementExt_MM(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int graphic = el2gfx(element);
  int phase8 = ABS(MovPos[ux][uy]) / (TILEX / 8);
  int phase2  = phase8 / 4;
  int dir = MovDir[ux][uy];

  if (element == EL_PACMAN)
  {
    graphic = (phase2 ? IMG_MM_PACMAN_RIGHT : IMG_MM_PACMAN_EATING_RIGHT);

    if (dir == MV_UP)
      graphic += 1;
    else if (dir == MV_LEFT)
      graphic += 2;
    else if (dir == MV_DOWN)
      graphic += 3;
  }

  if (dx || dy)
    DrawGraphicShifted_MM(x, y, dx, dy, graphic, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask_MM(x, y, graphic, 0);
  else
    DrawGraphic_MM(x, y, graphic);
}

void DrawLevelElementExt_MM(int x, int y, int dx, int dy, int element,
			 int cut_mode, int mask_mode)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElementExt_MM(SCREENX(x), SCREENY(y), dx, dy, element,
			 cut_mode, mask_mode);
}

void DrawScreenElementShifted_MM(int x, int y, int dx, int dy, int element,
			      int cut_mode)
{
  DrawScreenElementExt_MM(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawScreenElement_MM(int x, int y, int element)
{
  DrawScreenElementExt_MM(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
}

void DrawScreenField_MM(int x, int y)
{
  int element = Tile[x][y];

  if (!IN_LEV_FIELD(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    int horiz_move = (MovDir[x][y] == MV_LEFT || MovDir[x][y] == MV_RIGHT);

    DrawScreenElement_MM(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted_MM(x, y, MovPos[x][y], 0, element, NO_CUTTING);
    else
      DrawScreenElementShifted_MM(x, y, 0, MovPos[x][y], element, NO_CUTTING);
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;

    Blocked2Moving(x, y, &oldx, &oldy);

    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    DrawScreenElement_MM(x, y, EL_EMPTY);

    element = Tile[oldx][oldy];

    if (horiz_move)
      DrawScreenElementShifted_MM(sx, sy, MovPos[oldx][oldy], 0, element,
				  NO_CUTTING);
    else
      DrawScreenElementShifted_MM(sx, sy, 0, MovPos[oldx][oldy], element,
				  NO_CUTTING);
  }
  else if (IS_DRAWABLE(element))
  {
    DrawScreenElement_MM(x, y, element);
  }
  else
  {
    DrawScreenElement_MM(x, y, EL_EMPTY);
  }
}

void DrawLevelField_MM(int x, int y)
{
  if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenField_MM(SCREENX(x), SCREENY(y));
  else if (IS_MOVING(x, y))
  {
    int newx, newy;

    Moving2Blocked(x, y, &newx, &newy);
    if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
      DrawScreenField_MM(SCREENX(newx), SCREENY(newy));
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);
    if (IN_SCR_FIELD(SCREENX(oldx), SCREENY(oldy)))
      DrawScreenField_MM(SCREENX(oldx), SCREENY(oldy));
  }
}

void DrawMiniElement_MM(int x, int y, int element)
{
  int graphic;

  if (!element)
  {
    DrawMiniGraphic_MM(x, y, IMG_EMPTY);

    return;
  }

  graphic = el2gfx(element);

  DrawMiniGraphic_MM(x, y, graphic);
}

void DrawMiniElementOrWall_MM(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement_MM(sx, sy, EL_EMPTY);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement_MM(sx, sy, Tile[x][y]);
}

void DrawField_MM(int x, int y)
{
  int element = Tile[x][y];

  DrawElement_MM(x, y, element);
}

void DrawLevel_MM(void)
{
  int x, y;

  ClearWindow();

  for (x = 0; x < lev_fieldx; x++)
    for (y = 0; y < lev_fieldy; y++)
      DrawField_MM(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawWallsExt_MM(int x, int y, int element, int draw_mask)
{
  Bitmap *bitmap;
  int graphic = el2gfx(WALL_BASE(element));
  int gx, gy;
  int i;

  getMiniGraphicSource(graphic, &bitmap, &gx, &gy);

  DrawGraphic_MM(x, y, IMG_EMPTY);

  /*
  if (IS_WALL_WOOD(element) || IS_WALL_AMOEBA(element) ||
      IS_DF_WALL_WOOD(element))
    gx += MINI_TILEX;
  if (IS_WALL_ICE(element) || IS_WALL_AMOEBA(element))
    gy += MINI_TILEY;
  */

  for (i = 0; i < 4; i++)
  {
    int dest_x = cSX + x * TILEX + MINI_TILEX * (i % 2);
    int dest_y = cSY + y * TILEY + MINI_TILEY * (i / 2);

    if (!((1 << i) & draw_mask))
      continue;

    if (element & (1 << i))
      BlitBitmap(bitmap, drawto_mm, gx, gy, MINI_TILEX, MINI_TILEY,
		 dest_x, dest_y);
    else
      ClearRectangle(drawto_mm, dest_x, dest_y, MINI_TILEX, MINI_TILEY);
  }

  MarkTileDirty(x, y);
}

void DrawWalls_MM(int x, int y, int element)
{
  DrawWallsExt_MM(x, y, element, HIT_MASK_ALL);
}

void DrawWallsAnimation_MM(int x, int y, int element, int phase, int bit_mask)
{
  int i;

  if (phase == 0)
  {
    DrawWalls_MM(x, y, element);

    return;
  }

  for (i = 0; i < 4; i++)
  {
    if (element & (1 << i))
    {
      int graphic;
      int frame;
      Bitmap *bitmap;
      int src_x, src_y;
      int dst_x = cSX + x * TILEX + (i % 2) * MINI_TILEX;
      int dst_y = cSY + y * TILEY + (i / 2) * MINI_TILEY;

      if (bit_mask & (1 << i))
      {
	graphic = (IS_WALL_AMOEBA(element) ?
		   IMG_MM_AMOEBA_WALL_GROWING :
		   IMG_MM_ICE_WALL_SHRINKING);
	frame = phase;
      }
      else
      {
	graphic = (IS_WALL_AMOEBA(element) ?
		   IMG_MM_AMOEBA_WALL :
		   IMG_MM_ICE_WALL);
	frame = 0;
      }

      getSizedGraphicSource(graphic, frame, MINI_TILESIZE, &bitmap,
			    &src_x, &src_y);

      BlitBitmap(bitmap, drawto_mm, src_x, src_y, MINI_TILEX, MINI_TILEY,
		 dst_x, dst_y);
    }
  }

  MarkTileDirty(x, y);
}

void DrawElement_MM(int x, int y, int element)
{
  if (element == EL_EMPTY)
    DrawGraphic_MM(x, y, IMG_EMPTY);
  else if (IS_WALL(element))
    DrawWalls_MM(x, y, element);
#if 0
  else if (IS_WALL_CHANGING(element) && IS_WALL_CHANGING(Tile[x][y]))
  {
    int wall_element = Tile[x][y] - EL_WALL_CHANGING + Store[x][y];

    DrawWalls_MM(x, y, wall_element);
  }
#endif
  else if (element == EL_PACMAN)
    DrawLevelField_MM(x, y);
  else if (element == EL_FUSE_ON &&
	   laser.fuse_off &&
	   laser.fuse_x == x &&
	   laser.fuse_y == y)
    DrawGraphic_MM(x, y, IMG_MM_FUSE);
  else if (element == EL_GRAY_BALL_ACTIVE)
    DrawGraphic_MM(x, y, el_act2gfx(EL_GRAY_BALL, MM_ACTION_ACTIVE));
  else if (element == EL_GRAY_BALL_OPENING)
    DrawGraphic_MM(x, y, el_act2gfx(EL_GRAY_BALL, MM_ACTION_OPENING));
  else if (element == EL_BOMB_ACTIVE)
    DrawGraphic_MM(x, y, el_act2gfx(EL_BOMB, MM_ACTION_ACTIVE));
  else if (element == EL_MINE_ACTIVE)
    DrawGraphic_MM(x, y, el_act2gfx(EL_MINE, MM_ACTION_ACTIVE));
  else
    DrawGraphic_MM(x, y, el2gfx(element));
}


// ----------------------------------------------------------------------------
// XSN
// ----------------------------------------------------------------------------

#define XSN_RND(x)		((x) != 0 ? rand() % (x) : 0)
#define XSN_ALPHA_VALUE(x)	(SDL_ALPHA_OPAQUE * (x) / 100)

#define XSN_MAX_ITEMS		100
#define XSN_MAX_HEIGHT		40
#define XSN_MAX_DX		2
#define XSN_MAX_DY		10
#define XSN_CHECK_DELAY		3
#define XSN_START_DELAY		60
#define XSN_UPDATE_DELAY	50
#define XSN_GROWTH_DELAY	3
#define XSN_GROWTH_RATE		3
#define XSN_CHANGE_DELAY	30
#define XSN_CHANGE_FACTOR	3
#define XSN_ALPHA_DEFAULT	XSN_ALPHA_VALUE(95)
#define XSN_ALPHA_VISIBLE	XSN_ALPHA_VALUE(50)
#define XSN_DEBUG_STEPS		5

static byte xsn_bits_0[] = { 0x05, 0x02, 0x05 };
static byte xsn_bits_1[] = { 0x22, 0x6b, 0x14, 0x2a, 0x14, 0x6b, 0x22 };
static byte xsn_bits_2[] = { 0x14, 0x08, 0x49, 0x36, 0x49, 0x08, 0x14 };

char debug_xsn_mode[] = { 76,101,116,32,105,116,32,115,110,111,119,33,0 };

void setHideSetupEntry(void *);
void removeHideSetupEntry(void *);

static struct
{
  int size;
  byte *bits;
  Bitmap *bitmap;
}
xsn_data[] =
{
  { ARRAY_SIZE(xsn_bits_0), xsn_bits_0 },
  { ARRAY_SIZE(xsn_bits_1), xsn_bits_1 },
  { ARRAY_SIZE(xsn_bits_2), xsn_bits_2 },
  { ARRAY_SIZE(xsn_bits_2), xsn_bits_2 },
  { ARRAY_SIZE(xsn_bits_1), xsn_bits_1 },
  { ARRAY_SIZE(xsn_bits_2), xsn_bits_2 },
  { ARRAY_SIZE(xsn_bits_0), xsn_bits_0 },
};
static int num_xsn_data = ARRAY_SIZE(xsn_data);

struct XsnItem
{
  int x;
  int y;
  int dx;
  int dy;
  int type;
  int active;
};

struct Xsn
{
  int area_xsize;
  int area_ysize;

  int num_items;
  int max_items;
  int max_height;
  int max_dx;
  int max_dy;

  int change_delay;
  int change_type;
  int change_dir;

  int *height;

  struct XsnItem items[XSN_MAX_ITEMS];

  Bitmap *bitmap;

  int alpha;
};

static struct Xsn xsn = { 0 };

static int xsn_percent(void)
{
  int xsn_m0 = -3;
  int xsn_m1 = xsn_m0 + 10;
  int xsn_m2 = xsn_m1 + 10;
  int xsn_m3 = xsn_m2 + 10;
  time_t xsn_e0 = time(NULL);
  struct tm *xsn_t0 = localtime(&xsn_e0);
  struct tm xsn_t1 = { 0,0,0, xsn_m2 * 3, xsn_m3 / 3, xsn_t0->tm_year, 0,0,-1 };
  time_t xsn_e1 = mktime(&xsn_t1);
  int xsn_c0 = (25 * xsn_m3) << xsn_m1;
  int xsn_c1 = (xsn_t1.tm_wday - xsn_m1) * !!xsn_t1.tm_wday;

  for (xsn_m0 = 5; xsn_m0 > 0; xsn_m0--)
  {
    int xsn_c2 = (xsn_m0 > 4 ? 0 : xsn_c1) - xsn_m1 * xsn_m0;
    int xsn_off = (xsn_m0 > 4 ? xsn_c0 : 0);
    time_t xsn_e3 = xsn_e1 - xsn_c2 * xsn_c0;

    if (xsn_e0 > xsn_e3 - xsn_off &&
        xsn_e0 < xsn_e3 + xsn_off + xsn_c0)
      return xsn_m0 * (xsn_m3 - xsn_m1);
  }

  return xsn_m0;
}

static void xsn_init_item(int nr)
{
  struct XsnItem *item = &xsn.items[nr];

  item->type = XSN_RND(num_xsn_data);

  if (xsn.change_type != 0)
  {
    int new_x = XSN_RND(xsn.area_xsize / 3);

    item->x = (xsn.change_dir == 1 ? new_x : xsn.area_xsize - new_x);
    item->y = XSN_RND(xsn.area_ysize);
  }
  else
  {
    item->x = XSN_RND(xsn.area_xsize - xsn_data[item->type].size);
    item->y = XSN_RND(xsn.area_ysize / 10);
  }

  item->dy = XSN_RND(xsn.max_dy + 1) + 1;
  item->dx = XSN_RND(item->dy / 4 + 1) * (XSN_RND(1000) > 500 ? -1 : 1);

  item->active = 1;
}

static void xsn_update_item(int nr)
{
  struct XsnItem *item = &xsn.items[nr];

  if (!item->active)
    xsn_init_item(nr);

  if (xsn.change_type != 0)
  {
    int dx_new = ABS(item->dx) +
      (xsn.change_type == 1 ?
       XSN_RND(XSN_CHANGE_FACTOR + 1) - XSN_CHANGE_FACTOR / 2 :
       XSN_RND(20));

    item->dx = MIN(MAX(-50, dx_new * xsn.change_dir), 50);
  }

  int new_x = item->x + item->dx;
  int new_y = item->y + item->dy;

  item->active = (new_y < xsn.area_ysize);

  if (xsn.change_type != 0)
    item->active = (item->active && new_x > 0 && new_x < xsn.area_xsize);

  int item_size = xsn_data[item->type].size;
  int half_item_size = item_size / 2;
  int mid_x = new_x + half_item_size;
  int mid_y = new_y + half_item_size;
  int upper_border = xsn.area_ysize - xsn.max_height;

  if (item->active &&
      new_y >= upper_border &&
      new_x >= 0 &&
      new_x <= xsn.area_xsize - item_size &&
      mid_y >= xsn.height[mid_x] &&
      mid_y < xsn.area_ysize)
  {
    Bitmap *item_bitmap = xsn_data[item->type].bitmap;
    SDL_Surface *surface = xsn.bitmap->surface;
    SDL_Surface *surface_masked = xsn.bitmap->surface_masked;
    int item_alpha = XSN_ALPHA_VALUE(81 + XSN_RND(20));
    int shrink = 1;
    int i;

    xsn.bitmap->surface = surface_masked;

    SDLSetAlpha(item_bitmap->surface_masked, TRUE, item_alpha);

    // blit to masked surface instead of opaque surface
    BlitBitmapMasked(item_bitmap, xsn.bitmap, 0, 0, item_size, item_size,
		     new_x, new_y - upper_border);

    SDLSetAlpha(item_bitmap->surface_masked, TRUE, XSN_ALPHA_DEFAULT);

    for (i = -half_item_size; i <= half_item_size; i++)
    {
      int xpos = mid_x + i;

      if (xpos >= 0 && xpos < xsn.area_xsize)
	xsn.height[xpos] = MIN(new_y + ABS(i), xsn.height[xpos]);
    }

    if (xsn.height[mid_x] <= upper_border + shrink)
    {
      int xpos1 = MAX(0, new_x - half_item_size);
      int xpos2 = MIN(new_x + 3 * half_item_size, xsn.area_xsize);
      int xsize = xpos2 - xpos1;
      int ysize1 = XSN_RND(xsn.max_height - shrink);
      int ysize2 = xsn.max_height - ysize1;

      SDLSetAlpha(surface_masked, FALSE, 0);

      FillRectangle(xsn.bitmap, xpos1, xsn.max_height, xsize, xsn.max_height,
		    BLACK_PIXEL);
      BlitBitmapMasked(xsn.bitmap, xsn.bitmap, xpos1, 0, xsize, ysize1,
		       xpos1, xsn.max_height + shrink);
      BlitBitmapMasked(xsn.bitmap, xsn.bitmap, xpos1, ysize1, xsize, ysize2,
		       xpos1, xsn.max_height + ysize1);
      FillRectangle(xsn.bitmap, xpos1, 0, xsize, xsn.max_height,
		    BLACK_PIXEL);
      BlitBitmapMasked(xsn.bitmap, xsn.bitmap, xpos1, xsn.max_height,
		       xsize, xsn.max_height, xpos1, 0);

      SDLSetAlpha(surface_masked, TRUE, xsn.alpha);

      for (i = xpos1; i < xpos2; i++)
	xsn.height[i] = MIN(xsn.height[i] + shrink, xsn.area_ysize - 1);
    }

    SDLFreeBitmapTextures(xsn.bitmap);
    SDLCreateBitmapTextures(xsn.bitmap);

    xsn.bitmap->surface = surface;

    item->active = 0;
  }

  item->dx += XSN_RND(XSN_CHANGE_FACTOR) * (XSN_RND(1000) > 500 ? -1 : 1);

  if (xsn.change_type == 0)
    item->dx = MIN(MAX(-xsn.max_dx, item->dx), xsn.max_dx);

  item->x = new_x;
  item->y = new_y;
}

static void xsn_update_change(void)
{
  if (XSN_RND(100) > 65)
  {
    xsn.change_dir = (XSN_RND(10) > 4 ? 1 : -1);
    xsn.change_delay = XSN_RND(5) + 1;
    xsn.change_type = 2;
  }
  else if (xsn.change_type == 2)
  {
    xsn.change_delay = XSN_RND(3) + 1;
    xsn.change_type = 1;
  }
  else
  {
    xsn.change_delay = XSN_CHANGE_DELAY;
    xsn.change_type = 0;
  }
}

static void DrawTileCursor_Xsn(int draw_target)
{
  static boolean initialized = FALSE;
  static boolean started = FALSE;
  static boolean active = FALSE;
  static boolean debug = FALSE;
  static DelayCounter check_delay = { XSN_CHECK_DELAY * 1000 };
  static DelayCounter start_delay = { 0 };
  static DelayCounter growth_delay = { 0 };
  static DelayCounter update_delay = { 0 };
  static DelayCounter change_delay = { 0 };
  static int percent = 0;
  static int debug_value = 0;
  boolean reinitialize = FALSE;
  boolean active_last = active;
  int i, x, y;

  if (draw_target != DRAW_TO_SCREEN)
    return;

  if (DelayReached(&check_delay))
  {
    percent = (debug ? debug_value * 100 / XSN_DEBUG_STEPS : xsn_percent());

    if (debug)
      setup.debug.xsn_percent = percent;

    if (setup.debug.xsn_mode != AUTO)
      percent = setup.debug.xsn_percent;

    setup.debug.xsn_percent = percent;

    active = (percent > 0);

    if ((active && !active_last) || setup.debug.xsn_mode != AUTO)
      removeHideSetupEntry(&setup.debug.xsn_mode);
    else if (!active && active_last)
      setHideSetupEntry(&setup.debug.xsn_mode);

    if (setup.debug.xsn_mode == FALSE)
      active = FALSE;
  }
  else if (tile_cursor.xsn_debug)
  {
    debug_value = (active ? 0 : MIN(debug_value + 1, XSN_DEBUG_STEPS));
    debug = TRUE;
    active = FALSE;

    ResetDelayCounter(&check_delay);

    setup.debug.xsn_mode = (debug_value > 0);
    tile_cursor.xsn_debug = FALSE;
  }

  if (!active)
    return;

  if (!initialized)
  {
    xsn.area_xsize = gfx.win_xsize;
    xsn.area_ysize = gfx.win_ysize;

    for (i = 0; i < num_xsn_data; i++)
    {
      int size = xsn_data[i].size;
      byte *bits = xsn_data[i].bits;
      Bitmap *bitmap = CreateBitmap(size, size, DEFAULT_DEPTH);

      FillRectangle(bitmap, 0, 0, size, size, BLACK_PIXEL);

      for (y = 0; y < size; y++)
	for (x = 0; x < size; x++)
	  if ((bits[y] >> x) & 0x01)
	    SDLPutPixel(bitmap, x, y, WHITE_PIXEL);

      SDL_Surface *surface = bitmap->surface;

      if ((bitmap->surface_masked = SDLGetNativeSurface(surface)) == NULL)
	Fail("SDLGetNativeSurface() failed");

      SDL_Surface *surface_masked = bitmap->surface_masked;

      SDL_SetColorKey(surface_masked, SET_TRANSPARENT_PIXEL,
		      SDL_MapRGB(surface_masked->format, 0x00, 0x00, 0x00));

      SDLSetAlpha(surface, TRUE, XSN_ALPHA_DEFAULT);
      SDLSetAlpha(surface_masked, TRUE, XSN_ALPHA_DEFAULT);

      xsn_data[i].bitmap = bitmap;
    }

    srand((unsigned int)time(NULL));

    initialized = TRUE;
  }

  if (!active_last)
  {
    start_delay.value = (debug || setup.debug.xsn_mode == TRUE ? 0 :
			 (XSN_START_DELAY + XSN_RND(XSN_START_DELAY)) * 1000);
    started = FALSE;

    ResetDelayCounter(&start_delay);

    reinitialize = TRUE;
  }

  if (reinitialize)
  {
    xsn.num_items  = 0;
    xsn.max_items  = percent * XSN_MAX_ITEMS / 100;
    xsn.max_height = percent * XSN_MAX_HEIGHT / 100;

    xsn.max_dx = XSN_MAX_DX;
    xsn.max_dy = XSN_MAX_DY;

    xsn.change_delay = XSN_CHANGE_DELAY;
    xsn.change_type  = 0;
    xsn.change_dir   = 0;

    xsn.alpha = XSN_ALPHA_DEFAULT;

    for (i = 0; i < xsn.max_items; i++)
      xsn_init_item(i);
  }

  if (xsn.area_xsize != gfx.win_xsize ||
      xsn.area_ysize != gfx.win_ysize ||
      reinitialize)
  {
    xsn.area_xsize = gfx.win_xsize;
    xsn.area_ysize = gfx.win_ysize;

    if (xsn.bitmap != NULL)
      FreeBitmap(xsn.bitmap);

    xsn.bitmap = CreateBitmap(xsn.area_xsize, xsn.max_height * 2,
			      DEFAULT_DEPTH);

    FillRectangle(xsn.bitmap, 0, 0, xsn.area_xsize, xsn.max_height,
		  BLACK_PIXEL);

    SDL_Surface *surface = xsn.bitmap->surface;

    if ((xsn.bitmap->surface_masked = SDLGetNativeSurface(surface)) == NULL)
      Fail("SDLGetNativeSurface() failed");

    SDL_Surface *surface_masked = xsn.bitmap->surface_masked;

    SDL_SetColorKey(surface_masked, SET_TRANSPARENT_PIXEL,
		    SDL_MapRGB(surface_masked->format, 0x00, 0x00, 0x00));

    SDLSetAlpha(surface, TRUE, xsn.alpha);
    SDLSetAlpha(surface_masked, TRUE, xsn.alpha);

    SDLCreateBitmapTextures(xsn.bitmap);

    for (i = 0; i < num_xsn_data; i++)
    {
      SDLFreeBitmapTextures(xsn_data[i].bitmap);
      SDLCreateBitmapTextures(xsn_data[i].bitmap);
    }

    if (xsn.height != NULL)
      checked_free(xsn.height);

    xsn.height = checked_calloc(xsn.area_xsize * sizeof(int));

    for (i = 0; i < xsn.area_xsize; i++)
      xsn.height[i] = xsn.area_ysize - 1;
  }

  if (!started)
  {
    if (!DelayReached(&start_delay))
      return;

    update_delay.value = XSN_UPDATE_DELAY;
    growth_delay.value = XSN_GROWTH_DELAY * 1000;
    change_delay.value = XSN_CHANGE_DELAY * 1000;

    ResetDelayCounter(&growth_delay);
    ResetDelayCounter(&update_delay);
    ResetDelayCounter(&change_delay);

    started = TRUE;
  }

  if (xsn.num_items < xsn.max_items)
  {
    if (DelayReached(&growth_delay))
    {
      xsn.num_items += XSN_RND(XSN_GROWTH_RATE * 2);
      xsn.num_items = MIN(xsn.num_items, xsn.max_items);
    }
  }

  if (DelayReached(&update_delay))
  {
    for (i = 0; i < xsn.num_items; i++)
      xsn_update_item(i);
  }

  if (DelayReached(&change_delay))
  {
    xsn_update_change();

    change_delay.value = xsn.change_delay * 1000;
  }

  int xsn_alpha_dx = (gfx.mouse_y > xsn.area_ysize - xsn.max_height ?
		      (xsn.alpha > XSN_ALPHA_VISIBLE ? -1 : 0) :
		      (xsn.alpha < XSN_ALPHA_DEFAULT ? +1 : 0));

  if (xsn_alpha_dx != 0)
  {
    xsn.alpha += xsn_alpha_dx;

    SDLSetAlpha(xsn.bitmap->surface_masked, TRUE, xsn.alpha);

    SDLFreeBitmapTextures(xsn.bitmap);
    SDLCreateBitmapTextures(xsn.bitmap);
  }

  BlitToScreenMasked(xsn.bitmap, 0, 0, xsn.area_xsize, xsn.max_height,
		     0, xsn.area_ysize - xsn.max_height);

  for (i = 0; i < xsn.num_items; i++)
  {
    int dst_x = xsn.items[i].x;
    int dst_y = xsn.items[i].y;
    int type = xsn.items[i].type;
    int size = xsn_data[type].size;
    Bitmap *bitmap = xsn_data[type].bitmap;

    BlitToScreenMasked(bitmap, 0, 0, size, size, dst_x, dst_y);
  }
}

void DrawTileCursor_MM(int draw_target, boolean tile_cursor_active)
{
  if (program.headless)
    return;

  Bitmap *fade_bitmap;
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int graphic = IMG_GLOBAL_TILE_CURSOR;
  int frame = 0;
  int tilesize = TILESIZE_VAR;
  int width = tilesize;
  int height = tilesize;

  DrawTileCursor_Xsn(draw_target);

  if (!tile_cursor.enabled ||
      !tile_cursor.active ||
      !tile_cursor_active)
    return;

  if (tile_cursor.moving)
  {
    int step = TILESIZE_VAR / 4;
    int dx = tile_cursor.target_x - tile_cursor.x;
    int dy = tile_cursor.target_y - tile_cursor.y;

    if (ABS(dx) < step)
      tile_cursor.x = tile_cursor.target_x;
    else
      tile_cursor.x += SIGN(dx) * step;

    if (ABS(dy) < step)
      tile_cursor.y = tile_cursor.target_y;
    else
      tile_cursor.y += SIGN(dy) * step;

    if (tile_cursor.x == tile_cursor.target_x &&
	tile_cursor.y == tile_cursor.target_y)
      tile_cursor.moving = FALSE;
  }

  dst_x = tile_cursor.x;
  dst_y = tile_cursor.y;

  frame = getGraphicAnimationFrame(graphic, -1);

  getSizedGraphicSource(graphic, frame, tilesize, &src_bitmap, &src_x, &src_y);

  fade_bitmap =
    (draw_target == DRAW_TO_FADE_SOURCE ? gfx.fade_bitmap_source :
     draw_target == DRAW_TO_FADE_TARGET ? gfx.fade_bitmap_target : NULL);

  if (draw_target == DRAW_TO_SCREEN)
    BlitToScreenMasked(src_bitmap, src_x, src_y, width, height, dst_x, dst_y);
  else
    BlitBitmapMasked(src_bitmap, fade_bitmap, src_x, src_y, width, height,
		     dst_x, dst_y);
}

Pixel ReadPixel(DrawBuffer *bitmap, int x, int y)
{
  return GetPixel(bitmap, x, y);
}

int get_base_element(int element)
{
  if (IS_MIRROR(element))
    return EL_MIRROR_START;
  else if (IS_MIRROR_FIXED(element))
    return EL_MIRROR_FIXED_START;
  else if (IS_POLAR(element))
    return EL_POLAR_START;
  else if (IS_POLAR_CROSS(element))
    return EL_POLAR_CROSS_START;
  else if (IS_BEAMER(element))
    return EL_BEAMER_RED_START + BEAMER_NR(element) * 16;
  else if (IS_FIBRE_OPTIC(element))
    return EL_FIBRE_OPTIC_START + FIBRE_OPTIC_NR(element) * 2;
  else if (IS_MCDUFFIN(element))
    return EL_MCDUFFIN_START;
  else if (IS_LASER(element))
    return EL_LASER_START;
  else if (IS_RECEIVER(element))
    return EL_RECEIVER_START;
  else if (IS_DF_MIRROR(element))
    return EL_DF_MIRROR_START;
  else if (IS_DF_MIRROR_AUTO(element))
    return EL_DF_MIRROR_AUTO_START;
  else if (IS_DF_MIRROR_FIXED(element))
    return EL_DF_MIRROR_FIXED_START;
  else if (IS_DF_SLOPE(element))
    return EL_DF_SLOPE_START;
  else if (IS_PACMAN(element))
    return EL_PACMAN_START;
  else if (IS_GRID_STEEL(element))
    return EL_GRID_STEEL_START;
  else if (IS_GRID_WOOD(element))
    return EL_GRID_WOOD_START;
  else if (IS_GRID_STEEL_FIXED(element))
    return EL_GRID_STEEL_FIXED_START;
  else if (IS_GRID_WOOD_FIXED(element))
    return EL_GRID_WOOD_FIXED_START;
  else if (IS_GRID_STEEL_AUTO(element))
    return EL_GRID_STEEL_AUTO_START;
  else if (IS_GRID_WOOD_AUTO(element))
    return EL_GRID_WOOD_AUTO_START;
  else if (IS_WALL_STEEL(element))
    return EL_WALL_STEEL_START;
  else if (IS_WALL_WOOD(element))
    return EL_WALL_WOOD_START;
  else if (IS_WALL_ICE(element))
    return EL_WALL_ICE_START;
  else if (IS_WALL_AMOEBA(element))
    return EL_WALL_AMOEBA_START;
  else if (IS_DF_WALL_STEEL(element))
    return EL_DF_WALL_STEEL_START;
  else if (IS_DF_WALL_WOOD(element))
    return EL_DF_WALL_WOOD_START;
  else if (IS_CHAR(element))
    return EL_CHAR_START;
  else
    return element;
}

int get_element_phase(int element)
{
  return element - get_base_element(element);
}

int get_num_elements(int element)
{
  if (IS_MIRROR(element) ||
      IS_POLAR(element) ||
      IS_BEAMER(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element) ||
      IS_DF_MIRROR_FIXED(element))
    return 16;
  else if (IS_GRID_STEEL_FIXED(element) ||
	   IS_GRID_WOOD_FIXED(element) ||
	   IS_GRID_STEEL_AUTO(element) ||
	   IS_GRID_WOOD_AUTO(element))
    return 8;
  else if (IS_MIRROR_FIXED(element) ||
	   IS_POLAR_CROSS(element) ||
	   IS_MCDUFFIN(element) ||
	   IS_LASER(element) ||
	   IS_RECEIVER(element) ||
	   IS_PACMAN(element) ||
	   IS_GRID_STEEL(element) ||
	   IS_GRID_WOOD(element) ||
	   IS_DF_SLOPE(element))
    return 4;
  else
    return 1;
}

int get_rotated_element(int element, int step)
{
  int base_element = get_base_element(element);
  int num_elements = get_num_elements(element);
  int element_phase = element - base_element;

  return base_element + (element_phase + step + num_elements) % num_elements;
}

static boolean has_full_rotation(int element)
{
  return (IS_BEAMER(element) ||
	  IS_MCDUFFIN(element) ||
	  IS_LASER(element) ||
	  IS_RECEIVER(element) ||
	  IS_PACMAN(element));
}

#define MM_FLIP_X			0
#define MM_FLIP_Y			1
#define MM_FLIP_XY			2

static int getFlippedTileExt_MM(int element, int mode)
{
  if (IS_WALL(element))
  {
    int base = WALL_BASE(element);
    int bits = WALL_BITS(element);

    if (mode == MM_FLIP_X)
    {
      bits = ((bits & 1) << 1 |
	      (bits & 2) >> 1 |
	      (bits & 4) << 1 |
	      (bits & 8) >> 1);
    }
    else if (mode == MM_FLIP_Y)
    {
      bits = ((bits & 1) << 2 |
	      (bits & 2) << 2 |
	      (bits & 4) >> 2 |
	      (bits & 8) >> 2);
    }
    else if (mode == MM_FLIP_XY)
    {
      bits = ((bits & 1) << 0 |
	      (bits & 2) << 1 |
	      (bits & 4) >> 1 |
	      (bits & 8) >> 0);
    }

    element = base | bits;
  }
  else
  {
    int base_element = get_base_element(element);
    int num_elements = get_num_elements(element);
    int element_phase = element - base_element;

    if (IS_GRID_STEEL(element) || IS_GRID_WOOD(element))
    {
      if ((mode == MM_FLIP_XY && element_phase < 2) ||
	  (mode != MM_FLIP_XY && element_phase > 1))
	element_phase ^= 1;
    }
    else if (IS_DF_SLOPE(element))
    {
      element_phase = (mode == MM_FLIP_X  ? 5 - element_phase :
		       mode == MM_FLIP_Y  ? 3 - element_phase :
		       mode == MM_FLIP_XY ? 4 - element_phase :
		       element_phase);
    }
    else
    {
      int num_elements_flip = num_elements;

      if (has_full_rotation(element))
      {
	if (mode == MM_FLIP_X)
	  num_elements_flip = num_elements / 2;
	else if (mode == MM_FLIP_XY)
	  num_elements_flip = num_elements * 3 / 4;
      }
      else
      {
	if (mode == MM_FLIP_XY)
	  num_elements_flip = num_elements / 2;
      }

      element_phase = num_elements_flip - element_phase;
    }

    element = base_element + (element_phase + num_elements) % num_elements;
  }

  return element;
}

int getFlippedTileX_MM(int element)
{
  return getFlippedTileExt_MM(element, MM_FLIP_X);
}

int getFlippedTileY_MM(int element)
{
  return getFlippedTileExt_MM(element, MM_FLIP_Y);
}

int getFlippedTileXY_MM(int element)
{
  return getFlippedTileExt_MM(element, MM_FLIP_XY);
}

int map_wall_from_base_element(int element)
{
  switch (element)
  {
    case EL_WALL_STEEL_BASE:	return EL_WALL_STEEL;
    case EL_WALL_WOOD_BASE:	return EL_WALL_WOOD;
    case EL_WALL_ICE_BASE:	return EL_WALL_ICE;
    case EL_WALL_AMOEBA_BASE:	return EL_WALL_AMOEBA;
    case EL_DF_WALL_STEEL_BASE:	return EL_DF_WALL_STEEL;
    case EL_DF_WALL_WOOD_BASE:	return EL_DF_WALL_WOOD;

    default:			return element;
  }
}

int map_wall_to_base_element(int element)
{
  switch (element)
  {
    case EL_WALL_STEEL:		return EL_WALL_STEEL_BASE;
    case EL_WALL_WOOD:		return EL_WALL_WOOD_BASE;
    case EL_WALL_ICE:		return EL_WALL_ICE_BASE;
    case EL_WALL_AMOEBA:	return EL_WALL_AMOEBA_BASE;
    case EL_DF_WALL_STEEL:	return EL_DF_WALL_STEEL_BASE;
    case EL_DF_WALL_WOOD:	return EL_DF_WALL_WOOD_BASE;

    default:			return element;
  }
}

int el2gfx(int element)
{
  return el2img_mm(map_wall_from_base_element(element));
}

int el_act2gfx(int element, int action)
{
  return el_act2img_mm(map_wall_from_base_element(element), action);
}

void RedrawPlayfield_MM(void)
{
  DrawLevel_MM();
  DrawLaser_MM();
}

void BlitScreenToBitmap_MM(Bitmap *target_bitmap)
{
  BlitBitmap(drawto_mm, target_bitmap,
	     REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE, REAL_SX, REAL_SY);
}
