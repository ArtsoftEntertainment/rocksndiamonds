// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// tools.c
// ============================================================================

#include <math.h>

#include "libgame/libgame.h"

#include "tools.h"
#include "files.h"
#include "init.h"
#include "game.h"
#include "events.h"
#include "anim.h"
#include "network.h"
#include "tape.h"
#include "screens.h"


#define DEBUG_FRAME_TIME	FALSE

// tool button identifiers
#define TOOL_CTRL_ID_YES	0
#define TOOL_CTRL_ID_NO		1
#define TOOL_CTRL_ID_CONFIRM	2
#define TOOL_CTRL_ID_PLAYER_1	3
#define TOOL_CTRL_ID_PLAYER_2	4
#define TOOL_CTRL_ID_PLAYER_3	5
#define TOOL_CTRL_ID_PLAYER_4	6
#define TOOL_CTRL_ID_TOUCH_YES	7
#define TOOL_CTRL_ID_TOUCH_NO	8
#define TOOL_CTRL_ID_TOUCH_CONFIRM 9

#define NUM_TOOL_BUTTONS	10

// constants for number of doors and door parts
#define NUM_DOORS		2
#define NUM_PANELS		NUM_DOORS
// #define NUM_PANELS		0
#define MAX_PARTS_PER_DOOR	8
#define MAX_DOOR_PARTS		(NUM_DOORS * MAX_PARTS_PER_DOOR + NUM_PANELS)
#define DOOR_PART_IS_PANEL(i)	((i) >= NUM_DOORS * MAX_PARTS_PER_DOOR)


struct DoorPartOrderInfo
{
  int nr;
  int sort_priority;
};

static struct DoorPartOrderInfo door_part_order[MAX_DOOR_PARTS];

struct DoorPartControlInfo
{
  int door_token;
  int graphic;
  struct DoorPartPosInfo *pos;
};

static struct DoorPartControlInfo door_part_controls[] =
{
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_1,
    &door_1.part_1
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_2,
    &door_1.part_2
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_3,
    &door_1.part_3
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_4,
    &door_1.part_4
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_5,
    &door_1.part_5
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_6,
    &door_1.part_6
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_7,
    &door_1.part_7
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_8,
    &door_1.part_8
  },

  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_1,
    &door_2.part_1
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_2,
    &door_2.part_2
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_3,
    &door_2.part_3
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_4,
    &door_2.part_4
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_5,
    &door_2.part_5
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_6,
    &door_2.part_6
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_7,
    &door_2.part_7
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_8,
    &door_2.part_8
  },

  {
    DOOR_1,
    IMG_BACKGROUND_PANEL,
    &door_1.panel
  },
  {
    DOOR_2,
    IMG_BACKGROUND_TAPE,
    &door_2.panel
  },

  {
    -1,
    -1,
    NULL
  }
};

static struct XY xy_topdown[] =
{
  {  0, -1 },
  { -1,  0 },
  { +1,  0 },
  {  0, +1 }
};


// forward declaration for internal use
static void MapToolButtons(unsigned int);
static void UnmapToolButtons(void);
static void HandleToolButtons(struct GadgetInfo *);
static int el_act_dir2crm(int, int, int);
static int el_act2crm(int, int);

static struct GadgetInfo *tool_gadget[NUM_TOOL_BUTTONS];
static int request_gadget_id = -1;

static char *print_if_not_empty(int element)
{
  static char *s = NULL;
  char *token_name = element_info[element].token_name;

  if (s != NULL)
    free(s);

  s = checked_malloc(strlen(token_name) + 10 + 1);

  if (element != EL_EMPTY)
    sprintf(s, "%d\t['%s']", element, token_name);
  else
    sprintf(s, "%d", element);

  return s;
}

int getFieldbufferOffsetX_RND(int dir, int pos)
{
  int full_lev_fieldx = lev_fieldx + (BorderElement != EL_EMPTY ? 2 : 0);
  int dx = (dir & MV_HORIZONTAL ? pos : 0);
  int dx_var = dx * TILESIZE_VAR / TILESIZE;
  int fx = FX;

  if (EVEN(SCR_FIELDX))
  {
    int sbx_right = SBX_Right + (BorderElement != EL_EMPTY ? 1 : 0);
    int ffx = (scroll_x - SBX_Left) * TILEX_VAR + dx_var;

    if (ffx < sbx_right * TILEX_VAR + TILEX_VAR / 2)
      fx += dx_var - MIN(ffx, TILEX_VAR / 2) + TILEX_VAR;
    else
      fx += (dx_var > 0 ? TILEX_VAR : 0);
  }
  else
  {
    fx += dx_var;
  }

  if (full_lev_fieldx <= SCR_FIELDX)
  {
    if (EVEN(SCR_FIELDX))
      fx = 2 * TILEX_VAR - (ODD(lev_fieldx)  ? TILEX_VAR / 2 : 0);
    else
      fx = 2 * TILEX_VAR - (EVEN(lev_fieldx) ? TILEX_VAR / 2 : 0);
  }

  return fx;
}

int getFieldbufferOffsetY_RND(int dir, int pos)
{
  int full_lev_fieldy = lev_fieldy + (BorderElement != EL_EMPTY ? 2 : 0);
  int dy = (dir & MV_VERTICAL ? pos : 0);
  int dy_var = dy * TILESIZE_VAR / TILESIZE;
  int fy = FY;

  if (EVEN(SCR_FIELDY))
  {
    int sby_lower = SBY_Lower + (BorderElement != EL_EMPTY ? 1 : 0);
    int ffy = (scroll_y - SBY_Upper) * TILEY_VAR + dy_var;

    if (ffy < sby_lower * TILEY_VAR + TILEY_VAR / 2)
      fy += dy_var - MIN(ffy, TILEY_VAR / 2) + TILEY_VAR;
    else
      fy += (dy_var > 0 ? TILEY_VAR : 0);
  }
  else
  {
    fy += dy_var;
  }

  if (full_lev_fieldy <= SCR_FIELDY)
  {
    if (EVEN(SCR_FIELDY))
      fy = 2 * TILEY_VAR - (ODD(lev_fieldy)  ? TILEY_VAR / 2 : 0);
    else
      fy = 2 * TILEY_VAR - (EVEN(lev_fieldy) ? TILEY_VAR / 2 : 0);
  }

  return fy;
}

static int getLevelFromScreenX_RND(int sx)
{
  int fx = getFieldbufferOffsetX_RND(ScreenMovDir, ScreenGfxPos);
  int dx = fx - FX;
  int px = sx - SX;
  int lx = LEVELX((px + dx) / TILESIZE_VAR);

  return lx;
}

static int getLevelFromScreenY_RND(int sy)
{
  int fy = getFieldbufferOffsetY_RND(ScreenMovDir, ScreenGfxPos);
  int dy = fy - FY;
  int py = sy - SY;
  int ly = LEVELY((py + dy) / TILESIZE_VAR);

  return ly;
}

static int getLevelFromScreenX_EM(int sx)
{
  int level_xsize = level.native_em_level->cav->width;
  int full_xsize = level_xsize * TILESIZE_VAR;

  sx -= (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);

  int fx = getFieldbufferOffsetX_EM();
  int dx = fx;
  int px = sx - SX;
  int lx = LEVELX((px + dx) / TILESIZE_VAR);

  return lx;
}

static int getLevelFromScreenY_EM(int sy)
{
  int level_ysize = level.native_em_level->cav->height;
  int full_ysize = level_ysize * TILESIZE_VAR;

  sy -= (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  int fy = getFieldbufferOffsetY_EM();
  int dy = fy;
  int py = sy - SY;
  int ly = LEVELY((py + dy) / TILESIZE_VAR);

  return ly;
}

static int getLevelFromScreenX_SP(int sx)
{
  int menBorder = setup.sp_show_border_elements;
  int level_xsize = level.native_sp_level->width;
  int full_xsize = (level_xsize - (menBorder ? 0 : 1)) * TILESIZE_VAR;

  sx += (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);

  int fx = getFieldbufferOffsetX_SP();
  int dx = fx - FX;
  int px = sx - SX;
  int lx = LEVELX((px + dx) / TILESIZE_VAR);

  return lx;
}

static int getLevelFromScreenY_SP(int sy)
{
  int menBorder = setup.sp_show_border_elements;
  int level_ysize = level.native_sp_level->height;
  int full_ysize = (level_ysize - (menBorder ? 0 : 1)) * TILESIZE_VAR;

  sy += (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  int fy = getFieldbufferOffsetY_SP();
  int dy = fy - FY;
  int py = sy - SY;
  int ly = LEVELY((py + dy) / TILESIZE_VAR);

  return ly;
}

static int getLevelFromScreenX_MM(int sx)
{
  int level_xsize = level.native_mm_level->fieldx;
  int full_xsize = level_xsize * TILESIZE_VAR;

  sx -= (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);

  int px = sx - SX;
  int lx = (px + TILESIZE_VAR) / TILESIZE_VAR - 1;

  return lx;
}

static int getLevelFromScreenY_MM(int sy)
{
  int level_ysize = level.native_mm_level->fieldy;
  int full_ysize = level_ysize * TILESIZE_VAR;

  sy -= (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  int py = sy - SY;
  int ly = (py + TILESIZE_VAR) / TILESIZE_VAR - 1;

  return ly;
}

int getLevelFromScreenX(int x)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return getLevelFromScreenX_EM(x);
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return getLevelFromScreenX_SP(x);
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return getLevelFromScreenX_MM(x);
  else
    return getLevelFromScreenX_RND(x);
}

int getLevelFromScreenY(int y)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return getLevelFromScreenY_EM(y);
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return getLevelFromScreenY_SP(y);
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return getLevelFromScreenY_MM(y);
  else
    return getLevelFromScreenY_RND(y);
}

int getScreenFieldSizeX(void)
{
  return (tape.playing ? tape.scr_fieldx : SCR_FIELDX);
}

int getScreenFieldSizeY(void)
{
  return (tape.playing ? tape.scr_fieldy : SCR_FIELDY);
}

void DumpTile(int x, int y)
{
  int sx = SCREENX(x);
  int sy = SCREENY(y);
  char *token_name;

  Info("---");
  Info("Field Info: SCREEN(%d, %d), LEVEL(%d, %d)", sx, sy, x, y);
  Info("---");

  if (!IN_LEV_FIELD(x, y))
  {
    Info("(not in level field)");
    Info("");

    return;
  }

  token_name = element_info[Tile[x][y]].token_name;

  Info("Tile:        %d\t['%s']",	Tile[x][y], token_name);
  Info("Back:        %s",		print_if_not_empty(Back[x][y]));
  Info("Store:       %s",		print_if_not_empty(Store[x][y]));
  Info("Store2:      %s",		print_if_not_empty(Store2[x][y]));
  Info("StorePlayer: %s",		print_if_not_empty(StorePlayer[x][y]));
  Info("MovPos:      %d",		MovPos[x][y]);
  Info("MovDir:      %d",		MovDir[x][y]);
  Info("MovDelay:    %d",		MovDelay[x][y]);
  Info("ChangeDelay: %d",		ChangeDelay[x][y]);
  Info("CustomValue: %d",		CustomValue[x][y]);
  Info("GfxElement:  %d",		GfxElement[x][y]);
  Info("GfxAction:   %d",		GfxAction[x][y]);
  Info("GfxFrame:    %d [%d]",		GfxFrame[x][y], FrameCounter);
  Info("Player x/y:  %d, %d",		local_player->jx, local_player->jy);
  Info("");
}

void DumpTileFromScreen(int sx, int sy)
{
  int lx = getLevelFromScreenX(sx);
  int ly = getLevelFromScreenY(sy);

  DumpTile(lx, ly);
}

void SetDrawtoField(int mode)
{
  if (mode == DRAW_TO_FIELDBUFFER)
  {
    FX = 2 * TILEX_VAR;
    FY = 2 * TILEY_VAR;
    BX1 = -2;
    BY1 = -2;
    BX2 = SCR_FIELDX + 1;
    BY2 = SCR_FIELDY + 1;

    drawto_field = fieldbuffer;
  }
  else	// DRAW_TO_BACKBUFFER
  {
    FX = SX;
    FY = SY;
    BX1 = 0;
    BY1 = 0;
    BX2 = SCR_FIELDX - 1;
    BY2 = SCR_FIELDY - 1;

    drawto_field = backbuffer;
  }
}

int GetDrawtoField(void)
{
  return (drawto_field == fieldbuffer ? DRAW_TO_FIELDBUFFER : DRAW_TO_BACKBUFFER);
}

static void RedrawPlayfield_RND(void)
{
  if (game.envelope_active)
    return;

  DrawLevel(REDRAW_ALL);
  DrawAllPlayers();
}

void RedrawPlayfield(void)
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    RedrawPlayfield_BD(TRUE);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    RedrawPlayfield_EM(TRUE);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    RedrawPlayfield_SP(TRUE);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    RedrawPlayfield_MM();
  else if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    RedrawPlayfield_RND();

  BlitScreenToBitmap(backbuffer);

  BlitBitmap(drawto, window, gfx.sx, gfx.sy, gfx.sxsize, gfx.sysize,
	     gfx.sx, gfx.sy);
}

static void DrawMaskedBorderExt_Rect(int x, int y, int width, int height,
				     int draw_target)
{
  Bitmap *src_bitmap = getGlobalBorderBitmapFromStatus(global.border_status);
  Bitmap *dst_bitmap = gfx.masked_border_bitmap_ptr;

  // may happen for "border.draw_masked.*" with undefined "global.border.*"
  if (src_bitmap == NULL)
    return;

  if (x == -1 && y == -1)
    return;

  if (draw_target == DRAW_TO_SCREEN)
    BlitToScreenMasked(src_bitmap, x, y, width, height, x, y);
  else
    BlitBitmapMasked(src_bitmap, dst_bitmap, x, y, width, height, x, y);
}

static void DrawMaskedBorderExt_FIELD(int draw_target)
{
  if (global.border_status >= GAME_MODE_MAIN &&
      global.border_status <= GAME_MODE_PLAYING &&
      border.draw_masked[global.border_status])
    DrawMaskedBorderExt_Rect(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
			     draw_target);
}

static void DrawMaskedBorderExt_DOOR_1(int draw_target)
{
  // when drawing to backbuffer, never draw border over open doors
  if (draw_target == DRAW_TO_BACKBUFFER &&
      (GetDoorState() & DOOR_OPEN_1))
    return;

  if (border.draw_masked[GFX_SPECIAL_ARG_DOOR] &&
      (global.border_status != GAME_MODE_EDITOR ||
       border.draw_masked[GFX_SPECIAL_ARG_EDITOR]))
    DrawMaskedBorderExt_Rect(DX, DY, DXSIZE, DYSIZE, draw_target);
}

static void DrawMaskedBorderExt_DOOR_2(int draw_target)
{
  // when drawing to backbuffer, never draw border over open doors
  if (draw_target == DRAW_TO_BACKBUFFER &&
      (GetDoorState() & DOOR_OPEN_2))
    return;

  if (border.draw_masked[GFX_SPECIAL_ARG_DOOR] &&
      global.border_status != GAME_MODE_EDITOR)
    DrawMaskedBorderExt_Rect(VX, VY, VXSIZE, VYSIZE, draw_target);
}

static void DrawMaskedBorderExt_DOOR_3(int draw_target)
{
  // currently not available
}

static void DrawMaskedBorderExt_ALL(int draw_target)
{
  DrawMaskedBorderExt_FIELD(draw_target);
  DrawMaskedBorderExt_DOOR_1(draw_target);
  DrawMaskedBorderExt_DOOR_2(draw_target);
  DrawMaskedBorderExt_DOOR_3(draw_target);
}

static void DrawMaskedBorderExt(int redraw_mask, int draw_target)
{
  // never draw masked screen borders on borderless screens
  if (global.border_status == GAME_MODE_LOADING ||
      global.border_status == GAME_MODE_TITLE)
    return;

  if (redraw_mask & REDRAW_ALL)
    DrawMaskedBorderExt_ALL(draw_target);
  else
  {
    if (redraw_mask & REDRAW_FIELD)
      DrawMaskedBorderExt_FIELD(draw_target);
    if (redraw_mask & REDRAW_DOOR_1)
      DrawMaskedBorderExt_DOOR_1(draw_target);
    if (redraw_mask & REDRAW_DOOR_2)
      DrawMaskedBorderExt_DOOR_2(draw_target);
    if (redraw_mask & REDRAW_DOOR_3)
      DrawMaskedBorderExt_DOOR_3(draw_target);
  }
}

void DrawMaskedBorder_FIELD(void)
{
  DrawMaskedBorderExt_FIELD(DRAW_TO_BACKBUFFER);
}

void DrawMaskedBorder(int redraw_mask)
{
  DrawMaskedBorderExt(redraw_mask, DRAW_TO_BACKBUFFER);
}

void DrawMaskedBorderToTarget(int draw_target)
{
  if (draw_target == DRAW_TO_BACKBUFFER ||
      draw_target == DRAW_TO_SCREEN)
  {
    DrawMaskedBorderExt(REDRAW_ALL, draw_target);
  }
  else
  {
    int last_border_status = global.border_status;

    if (draw_target == DRAW_TO_FADE_SOURCE)
    {
      global.border_status = gfx.fade_border_source_status;
      gfx.masked_border_bitmap_ptr = gfx.fade_bitmap_source;
    }
    else if (draw_target == DRAW_TO_FADE_TARGET)
    {
      global.border_status = gfx.fade_border_target_status;
      gfx.masked_border_bitmap_ptr = gfx.fade_bitmap_target;
    }

    // always use global border for PLAYING when restarting the game
    if (global.border_status == GAME_MODE_PSEUDO_RESTARTING)
      global.border_status = GAME_MODE_PLAYING;

    DrawMaskedBorderExt(REDRAW_ALL, draw_target);

    global.border_status = last_border_status;
    gfx.masked_border_bitmap_ptr = backbuffer;
  }
}

void DrawTileCursor(int draw_target, int drawing_stage)
{
  int tile_cursor_active = (game_status == GAME_MODE_PLAYING);

  DrawTileCursor_MM(draw_target, drawing_stage, tile_cursor_active);
}

void FreeTileCursorTextures(void)
{
  FreeTileCursorTextures_MM();
}

void BlitScreenToBitmapExt_RND(Bitmap *target_bitmap, int fx, int fy)
{
  BlitBitmap(drawto_field, target_bitmap, fx, fy, SXSIZE, SYSIZE, SX, SY);
}

void BlitScreenToBitmap_RND(Bitmap *target_bitmap)
{
  int fx = getFieldbufferOffsetX_RND(ScreenMovDir, ScreenGfxPos);
  int fy = getFieldbufferOffsetY_RND(ScreenMovDir, ScreenGfxPos);

  BlitScreenToBitmapExt_RND(target_bitmap, fx, fy);
}

void BlitScreenToBitmap(Bitmap *target_bitmap)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    BlitScreenToBitmap_BD(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    BlitScreenToBitmap_EM(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    BlitScreenToBitmap_SP(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    BlitScreenToBitmap_MM(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    BlitScreenToBitmap_RND(target_bitmap);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawFramesPerSecond(void)
{
  char text[100];
  int font_nr = FONT_TEXT_2;
  int font_width = getFontWidth(font_nr);
  int draw_deactivation_mask = GetDrawDeactivationMask();
  boolean draw_masked = (draw_deactivation_mask == REDRAW_NONE);

  // draw FPS with leading space (needed if field buffer deactivated)
  sprintf(text, " %04.1f fps", global.frames_per_second);

  // override draw deactivation mask (required for invisible warp mode)
  SetDrawDeactivationMask(REDRAW_NONE);

  // draw opaque FPS if field buffer deactivated, else draw masked FPS
  DrawTextExt(backbuffer, SX + SXSIZE - font_width * strlen(text), SY, text,
	      font_nr, (draw_masked ? BLIT_MASKED : BLIT_OPAQUE));

  // set draw deactivation mask to previous value
  SetDrawDeactivationMask(draw_deactivation_mask);

  // force full-screen redraw in this frame
  redraw_mask = REDRAW_ALL;
}

#if DEBUG_FRAME_TIME
static void PrintFrameTimeDebugging(void)
{
  static unsigned int last_counter = 0;
  unsigned int counter = Counter();
  int diff_1 = counter - last_counter;
  int diff_2 = diff_1 - GAME_FRAME_DELAY;
  int diff_2_max = 20;
  int diff_2_cut = MIN(ABS(diff_2), diff_2_max);
  char diff_bar[2 * diff_2_max + 5];
  int pos = 0;
  int i;

  diff_bar[pos++] = (diff_2 < -diff_2_max ? '<' : ' ');

  for (i = 0; i < diff_2_max; i++)
    diff_bar[pos++] = (diff_2 >= 0 ? ' ' :
		       i >= diff_2_max - diff_2_cut ? '-' : ' ');

  diff_bar[pos++] = '|';

  for (i = 0; i < diff_2_max; i++)
    diff_bar[pos++] = (diff_2 <= 0 ? ' ' : i < diff_2_cut ? '+' : ' ');

  diff_bar[pos++] = (diff_2 > diff_2_max ? '>' : ' ');

  diff_bar[pos++] = '\0';

  Debug("time:frame", "%06d [%02d] [%c%02d] %s",
	counter,
	diff_1,
	(diff_2 < 0 ? '-' : diff_2 > 0 ? '+' : ' '), ABS(diff_2),
	diff_bar);

  last_counter = counter;
}
#endif

static int unifiedRedrawMask(int mask)
{
  if (mask & REDRAW_ALL)
    return REDRAW_ALL;

  if (mask & REDRAW_FIELD && mask & REDRAW_DOORS)
    return REDRAW_ALL;

  return mask;
}

static boolean equalRedrawMasks(int mask_1, int mask_2)
{
  return unifiedRedrawMask(mask_1) == unifiedRedrawMask(mask_2);
}

void BackToFront(void)
{
  static int last_redraw_mask = REDRAW_NONE;

  // force screen redraw in every frame to continue drawing global animations
  // (but always use the last redraw mask to prevent unwanted side effects)
  if (redraw_mask == REDRAW_NONE)
    redraw_mask = last_redraw_mask;

  last_redraw_mask = redraw_mask;

#if 1
  // masked border now drawn immediately when blitting backbuffer to window
#else
  // draw masked border to all viewports, if defined
  DrawMaskedBorder(redraw_mask);
#endif

  // draw frames per second (only if debug mode is enabled)
  if (redraw_mask & REDRAW_FPS)
    DrawFramesPerSecond();

  // remove playfield redraw before potentially merging with doors redraw
  if (DrawingDeactivated(REAL_SX, REAL_SY))
    redraw_mask &= ~REDRAW_FIELD;

  // redraw complete window if both playfield and (some) doors need redraw
  if (redraw_mask & REDRAW_FIELD && redraw_mask & REDRAW_DOORS)
    redraw_mask = REDRAW_ALL;

  /* although redrawing the whole window would be fine for normal gameplay,
     being able to only redraw the playfield is required for deactivating
     certain drawing areas (mainly playfield) to work, which is needed for
     warp-forward to be fast enough (by skipping redraw of most frames) */

  if (redraw_mask & REDRAW_ALL)
  {
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  }
  else if (redraw_mask & REDRAW_FIELD)
  {
    BlitBitmap(backbuffer, window,
	       REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE, REAL_SX, REAL_SY);
  }
  else if (redraw_mask & REDRAW_DOORS)
  {
    // merge door areas to prevent calling screen redraw more than once
    int x1 = WIN_XSIZE;
    int y1 = WIN_YSIZE;
    int x2 = 0;
    int y2 = 0;

    if (redraw_mask & REDRAW_DOOR_1)
    {
      x1 = MIN(x1, DX);
      y1 = MIN(y1, DY);
      x2 = MAX(x2, DX + DXSIZE);
      y2 = MAX(y2, DY + DYSIZE);
    }

    if (redraw_mask & REDRAW_DOOR_2)
    {
      x1 = MIN(x1, VX);
      y1 = MIN(y1, VY);
      x2 = MAX(x2, VX + VXSIZE);
      y2 = MAX(y2, VY + VYSIZE);
    }

    if (redraw_mask & REDRAW_DOOR_3)
    {
      x1 = MIN(x1, EX);
      y1 = MIN(y1, EY);
      x2 = MAX(x2, EX + EXSIZE);
      y2 = MAX(y2, EY + EYSIZE);
    }

    // make sure that at least one pixel is blitted, and inside the screen
    // (else nothing is blitted, causing the animations not to be updated)
    x1 = MIN(MAX(0, x1), WIN_XSIZE - 1);
    y1 = MIN(MAX(0, y1), WIN_YSIZE - 1);
    x2 = MIN(MAX(1, x2), WIN_XSIZE);
    y2 = MIN(MAX(1, y2), WIN_YSIZE);

    BlitBitmap(backbuffer, window, x1, y1, x2 - x1, y2 - y1, x1, y1);
  }

  redraw_mask = REDRAW_NONE;

#if DEBUG_FRAME_TIME
  PrintFrameTimeDebugging();
#endif
}

void BackToFront_WithFrameDelay(unsigned int frame_delay_value)
{
  unsigned int frame_delay_value_old = GetVideoFrameDelay();

  SetVideoFrameDelay(frame_delay_value);

  BackToFront();

  SetVideoFrameDelay(frame_delay_value_old);
}

static int fade_type_skip = FADE_TYPE_NONE;

static void FadeExt(int fade_mask, int fade_mode, int fade_type)
{
  void (*draw_border_function)(void) = NULL;
  int x, y, width, height;
  int fade_delay, post_delay;

  if (fade_type == FADE_TYPE_FADE_OUT)
  {
    if (fade_type_skip != FADE_TYPE_NONE)
    {
      // skip all fade operations until specified fade operation
      if (fade_type & fade_type_skip)
	fade_type_skip = FADE_TYPE_NONE;

      return;
    }

    if (fading.fade_mode & FADE_TYPE_TRANSFORM)
      return;
  }

  redraw_mask |= fade_mask;

  if (fade_type == FADE_TYPE_SKIP)
  {
    fade_type_skip = fade_mode;

    return;
  }

  fade_delay = fading.fade_delay;
  post_delay = (fade_mode == FADE_MODE_FADE_OUT ? fading.post_delay : 0);

  if (fade_type_skip != FADE_TYPE_NONE)
  {
    // skip all fade operations until specified fade operation
    if (fade_type & fade_type_skip)
      fade_type_skip = FADE_TYPE_NONE;

    fade_delay = 0;
  }

  if (global.autoplay_leveldir)
  {
    return;
  }

  if (fade_mask == REDRAW_FIELD)
  {
    x = FADE_SX;
    y = FADE_SY;
    width  = FADE_SXSIZE;
    height = FADE_SYSIZE;

    if (border.draw_masked_when_fading)
      draw_border_function = DrawMaskedBorder_FIELD;	// update when fading
    else
      DrawMaskedBorder_FIELD();				// draw once
  }
  else		// REDRAW_ALL
  {
    x = 0;
    y = 0;
    width  = WIN_XSIZE;
    height = WIN_YSIZE;
  }

  // when switching screens without fading, set fade delay to zero
  if (!setup.fade_screens || fading.fade_mode == FADE_MODE_NONE)
    fade_delay = 0;

  // do not display black frame when fading out without fade delay
  if (fade_mode == FADE_MODE_FADE_OUT && fade_delay == 0)
    return;

  FadeRectangle(x, y, width, height, fade_mode, fade_delay, post_delay,
		draw_border_function);

  redraw_mask &= ~fade_mask;

  ClearAutoRepeatKeyEvents();
}

static void SetScreenStates_BeforeFadingIn(void)
{
  // temporarily set screen mode for animations to screen after fading in
  global.anim_status = global.anim_status_next;

  // store backbuffer with all animations that will be started after fading in
  PrepareFadeBitmap(DRAW_TO_FADE_TARGET);

  // set screen mode for animations back to fading
  global.anim_status = GAME_MODE_PSEUDO_FADING;
}

static void SetScreenStates_AfterFadingIn(void)
{
  // store new source screen (to use correct masked border for fading)
  gfx.fade_border_source_status = global.border_status;

  global.anim_status = global.anim_status_next;
}

static void SetScreenStates_BeforeFadingOut(void)
{
  // store new target screen (to use correct masked border for fading)
  gfx.fade_border_target_status = game_status;

  // set screen mode for animations to fading
  global.anim_status = GAME_MODE_PSEUDO_FADING;

  // store backbuffer with all animations that will be stopped for fading out
  PrepareFadeBitmap(DRAW_TO_FADE_SOURCE);
}

static void SetScreenStates_AfterFadingOut(void)
{
  global.border_status = game_status;

  // always use global border for PLAYING when restarting the game
  if (global.border_status == GAME_MODE_PSEUDO_RESTARTING)
    global.border_status = GAME_MODE_PLAYING;
}

void CoverScreen(void)
{
  if (level.game_engine_type != GAME_ENGINE_TYPE_BD || !game_bd.cover_screen)
    return;

  // do not cover screen if returning from playing to level editor
  if (game_status == GAME_MODE_EDITOR)
    return;

  // before covering screen, close request door that might still be open
  if (game.request_open)
  {
    // if request open after asking to save tape after game won, close all doors for hall of fame
    if (setup.show_scores_after_game)
      CloseDoor(DOOR_CLOSE_ALL);
    else
      CloseDoor(DOOR_CLOSE_1);
  }

  CoverScreen_BD();

  // if not playing or restarting game, stop sound loop played while covering the screen
  if (game_status != GAME_MODE_PLAYING &&
      game_status != GAME_MODE_PSEUDO_RESTARTING)
    FadeSounds();
}

void FadeIn(int fade_mask)
{
  SetScreenStates_BeforeFadingIn();

#if 1
  DrawMaskedBorder(REDRAW_ALL);
#endif

  if (fading.fade_mode & FADE_TYPE_TRANSFORM)
    FadeExt(fade_mask, fading.fade_mode, FADE_TYPE_FADE_IN);
  else
    FadeExt(fade_mask, FADE_MODE_FADE_IN, FADE_TYPE_FADE_IN);

  FADE_SX = REAL_SX;
  FADE_SY = REAL_SY;
  FADE_SXSIZE = FULL_SXSIZE;
  FADE_SYSIZE = FULL_SYSIZE;

  // activate virtual buttons depending on upcoming game status
  if (strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS) &&
      game_status == GAME_MODE_PLAYING && !tape.playing)
    SetOverlayActive(TRUE);

  SetScreenStates_AfterFadingIn();

  // force update of global animation status in case of rapid screen changes
  redraw_mask = REDRAW_ALL;
  BackToFront();
}

void FadeOut(int fade_mask)
{
  // update screen if areas covered by "fade_mask" and "redraw_mask" differ
  if (!equalRedrawMasks(fade_mask, redraw_mask) &&
      fade_type_skip != FADE_MODE_SKIP_FADE_OUT)
    BackToFront();

  // when using BD game engine, cover playfield before fading out after a game
  CoverScreen();

  SetScreenStates_BeforeFadingOut();

  SetTileCursorActive(FALSE);
  SetOverlayActive(FALSE);

#if 0
  DrawMaskedBorder(REDRAW_ALL);
#endif

  if (fading.fade_mode & FADE_TYPE_TRANSFORM)
    FadeExt(fade_mask, fading.fade_mode, FADE_TYPE_FADE_OUT);
  else
    FadeExt(fade_mask, FADE_MODE_FADE_OUT, FADE_TYPE_FADE_OUT);

  SetScreenStates_AfterFadingOut();
}

static void FadeSetLeaveNext(struct TitleFadingInfo fading_leave, boolean set)
{
  static struct TitleFadingInfo fading_leave_stored;

  if (set)
    fading_leave_stored = fading_leave;
  else
    fading = fading_leave_stored;
}

void FadeSetEnterMenu(void)
{
  fading = menu.enter_menu;

  FadeSetLeaveNext(fading, TRUE);	// (keep same fade mode)
}

void FadeSetLeaveMenu(void)
{
  fading = menu.leave_menu;

  FadeSetLeaveNext(fading, TRUE);	// (keep same fade mode)
}

void FadeSetEnterScreen(void)
{
  fading = menu.enter_screen[game_status];

  FadeSetLeaveNext(menu.leave_screen[game_status], TRUE);	// store
}

void FadeSetNextScreen(void)
{
  fading = menu.next_screen[game_status];

  // (do not overwrite fade mode set by FadeSetEnterScreen)
  // FadeSetLeaveNext(fading, TRUE);	// (keep same fade mode)
}

void FadeSetLeaveScreen(void)
{
  FadeSetLeaveNext(menu.leave_screen[game_status], FALSE);	// recall
}

void FadeSetFromType(int type)
{
  if (type & TYPE_ENTER_SCREEN)
    FadeSetEnterScreen();
  else if (type & TYPE_ENTER)
    FadeSetEnterMenu();
  else if (type & TYPE_LEAVE)
    FadeSetLeaveMenu();
}

void FadeSetDisabled(void)
{
  static struct TitleFadingInfo fading_none = { FADE_MODE_NONE, -1, -1, -1 };

  fading = fading_none;
}

void FadeSkipNextFadeIn(void)
{
  FadeExt(0, FADE_MODE_SKIP_FADE_IN, FADE_TYPE_SKIP);
}

void FadeSkipNextFadeOut(void)
{
  FadeExt(0, FADE_MODE_SKIP_FADE_OUT, FADE_TYPE_SKIP);
}

static int getGlobalGameStatus(int status)
{
  return (status == GAME_MODE_PSEUDO_TYPENAME ? GAME_MODE_MAIN :
	  status == GAME_MODE_SCOREINFO       ? GAME_MODE_SCORES :
	  status);
}

int getImageFromGraphicOrDefault(int graphic, int default_graphic)
{
  if (graphic == IMG_UNDEFINED)
    return IMG_UNDEFINED;

  boolean redefined = getImageListEntryFromImageID(graphic)->redefined;

  return (graphic_info[graphic].bitmap != NULL || redefined ?
	  graphic : default_graphic);
}

static int getBackgroundImage(int graphic)
{
  return getImageFromGraphicOrDefault(graphic, IMG_BACKGROUND);
}

static int getGlobalBorderImage(int graphic)
{
  return getImageFromGraphicOrDefault(graphic, IMG_GLOBAL_BORDER);
}

Bitmap *getGlobalBorderBitmapFromStatus(int status_raw)
{
  int status = getGlobalGameStatus(status_raw);
  int graphic =
    (status == GAME_MODE_MAIN    ? IMG_GLOBAL_BORDER_MAIN :
     status == GAME_MODE_SCORES  ? IMG_GLOBAL_BORDER_SCORES :
     status == GAME_MODE_EDITOR  ? IMG_GLOBAL_BORDER_EDITOR :
     status == GAME_MODE_PLAYING ? IMG_GLOBAL_BORDER_PLAYING :
     IMG_GLOBAL_BORDER);
  int graphic_final = getGlobalBorderImage(graphic);

  return graphic_info[graphic_final].bitmap;
}

void SetBackgroundImage(int graphic, int redraw_mask)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  struct GraphicInfo g_undefined = { 0 };

  if (graphic == IMG_UNDEFINED)
    g = &g_undefined;

  // always use original size bitmap for backgrounds, if existing
  Bitmap *bitmap = (g->bitmaps != NULL &&
		    g->bitmaps[IMG_BITMAP_PTR_ORIGINAL] != NULL ?
		    g->bitmaps[IMG_BITMAP_PTR_ORIGINAL] : g->bitmap);

  // remove every mask before setting mask for window, and
  // remove window area mask before setting mask for main or door area
  int remove_mask = (redraw_mask == REDRAW_ALL ? 0xffff : REDRAW_ALL);

  // (!!! TO BE FIXED: The whole REDRAW_* system really sucks! !!!)
  SetBackgroundBitmap(NULL, remove_mask, 0, 0, 0, 0);	// !!! FIX THIS !!!
  SetBackgroundBitmap(bitmap, redraw_mask,
		      g->src_x, g->src_y,
		      g->width, g->height);
}

void SetWindowBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetBackgroundImage(graphic, REDRAW_ALL);
}

void SetMainBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetBackgroundImage(graphic, REDRAW_FIELD);
}

void SetDoorBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetBackgroundImage(graphic, REDRAW_DOOR_1);
}

void SetWindowBackgroundImage(int graphic)
{
  SetBackgroundImage(getBackgroundImage(graphic), REDRAW_ALL);
}

void SetMainBackgroundImage(int graphic)
{
  SetBackgroundImage(getBackgroundImage(graphic), REDRAW_FIELD);
}

void SetDoorBackgroundImage(int graphic)
{
  SetBackgroundImage(getBackgroundImage(graphic), REDRAW_DOOR_1);
}

void SetPanelBackground(void)
{
  SetDoorBackgroundImage(IMG_BACKGROUND_PANEL);
}

void DrawBackground(int x, int y, int width, int height)
{
  // "drawto" might still point to playfield buffer here (hall of fame)
  ClearRectangleOnBackground(backbuffer, x, y, width, height);

  if (IN_GFX_FIELD_FULL(x, y))
    redraw_mask |= REDRAW_FIELD;
  else if (IN_GFX_DOOR_1(x, y))
    redraw_mask |= REDRAW_DOOR_1;
  else if (IN_GFX_DOOR_2(x, y))
    redraw_mask |= REDRAW_DOOR_2;
  else if (IN_GFX_DOOR_3(x, y))
    redraw_mask |= REDRAW_DOOR_3;
}

void DrawBackgroundForFont(int x, int y, int width, int height, int font_nr)
{
  struct FontBitmapInfo *font = getFontBitmapInfo(font_nr);

  if (font->bitmap == NULL)
    return;

  DrawBackground(x, y, width, height);
}

void DrawBackgroundForGraphic(int x, int y, int width, int height, int graphic)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  if (g->bitmap == NULL)
    return;

  DrawBackground(x, y, width, height);
}

static int game_status_last = -1;
static Bitmap *global_border_bitmap_last = NULL;
static Bitmap *global_border_bitmap = NULL;
static int real_sx_last = -1, real_sy_last = -1;
static int full_sxsize_last = -1, full_sysize_last = -1;
static int dx_last = -1, dy_last = -1;
static int dxsize_last = -1, dysize_last = -1;
static int vx_last = -1, vy_last = -1;
static int vxsize_last = -1, vysize_last = -1;
static int ex_last = -1, ey_last = -1;
static int exsize_last = -1, eysize_last = -1;

boolean CheckIfGlobalBorderHasChanged(void)
{
  // if game status has not changed, global border has not changed either
  if (game_status == game_status_last)
    return FALSE;

  // determine and store new global border bitmap for current game status
  global_border_bitmap = getGlobalBorderBitmapFromStatus(game_status);

  return (global_border_bitmap_last != global_border_bitmap);
}

#define ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED		0

#if ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED
static boolean CheckIfGlobalBorderRedrawIsNeeded(void)
{
  // if game status has not changed, nothing has to be redrawn
  if (game_status == game_status_last)
    return FALSE;

  // redraw if last screen was title screen
  if (game_status_last == GAME_MODE_TITLE)
    return TRUE;

  // redraw if global screen border has changed
  if (CheckIfGlobalBorderHasChanged())
    return TRUE;

  // redraw if position or size of playfield area has changed
  if (real_sx_last != REAL_SX || real_sy_last != REAL_SY ||
      full_sxsize_last != FULL_SXSIZE || full_sysize_last != FULL_SYSIZE)
    return TRUE;

  // redraw if position or size of door area has changed
  if (dx_last != DX || dy_last != DY ||
      dxsize_last != DXSIZE || dysize_last != DYSIZE)
    return TRUE;

  // redraw if position or size of tape area has changed
  if (vx_last != VX || vy_last != VY ||
      vxsize_last != VXSIZE || vysize_last != VYSIZE)
    return TRUE;

  // redraw if position or size of editor area has changed
  if (ex_last != EX || ey_last != EY ||
      exsize_last != EXSIZE || eysize_last != EYSIZE)
    return TRUE;

  return FALSE;
}
#endif

static void RedrawGlobalBorderFromBitmap(Bitmap *bitmap)
{
  if (bitmap)
    BlitBitmap(bitmap, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  else
    ClearRectangle(backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE);
}

void RedrawGlobalBorder(void)
{
  Bitmap *bitmap = getGlobalBorderBitmapFromStatus(game_status);

  RedrawGlobalBorderFromBitmap(bitmap);

  redraw_mask = REDRAW_ALL;
}

static void RedrawGlobalBorderIfNeeded(void)
{
#if ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED
  if (game_status == game_status_last)
    return;
#endif

  // copy current draw buffer to later copy back areas that have not changed
  if (game_status_last != GAME_MODE_TITLE)
    BlitBitmap(backbuffer, bitmap_db_store_1, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

#if ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED
  if (CheckIfGlobalBorderRedrawIsNeeded())
#else
  // determine and store new global border bitmap for current game status
  global_border_bitmap = getGlobalBorderBitmapFromStatus(game_status);
#endif
  {
    // redraw global screen border (or clear, if defined to be empty)
    RedrawGlobalBorderFromBitmap(global_border_bitmap);

    if (game_status == GAME_MODE_EDITOR)
      DrawSpecialEditorDoor();

    // copy previous playfield and door areas, if they are defined on both
    // previous and current screen and if they still have the same size

    if (real_sx_last != -1 && real_sy_last != -1 &&
	REAL_SX != -1 && REAL_SY != -1 &&
	full_sxsize_last == FULL_SXSIZE && full_sysize_last == FULL_SYSIZE)
      BlitBitmap(bitmap_db_store_1, backbuffer,
		 real_sx_last, real_sy_last, FULL_SXSIZE, FULL_SYSIZE,
		 REAL_SX, REAL_SY);

    if (dx_last != -1 && dy_last != -1 &&
	DX != -1 && DY != -1 &&
	dxsize_last == DXSIZE && dysize_last == DYSIZE)
      BlitBitmap(bitmap_db_store_1, backbuffer,
		 dx_last, dy_last, DXSIZE, DYSIZE, DX, DY);

    if (game_status != GAME_MODE_EDITOR)
    {
      if (vx_last != -1 && vy_last != -1 &&
	  VX != -1 && VY != -1 &&
	  vxsize_last == VXSIZE && vysize_last == VYSIZE)
	BlitBitmap(bitmap_db_store_1, backbuffer,
		   vx_last, vy_last, VXSIZE, VYSIZE, VX, VY);
    }
    else
    {
      if (ex_last != -1 && ey_last != -1 &&
	  EX != -1 && EY != -1 &&
	  exsize_last == EXSIZE && eysize_last == EYSIZE)
	BlitBitmap(bitmap_db_store_1, backbuffer,
		   ex_last, ey_last, EXSIZE, EYSIZE, EX, EY);
    }

    redraw_mask = REDRAW_ALL;
  }

  game_status_last = game_status;

  global_border_bitmap_last = global_border_bitmap;

  real_sx_last = REAL_SX;
  real_sy_last = REAL_SY;
  full_sxsize_last = FULL_SXSIZE;
  full_sysize_last = FULL_SYSIZE;
  dx_last = DX;
  dy_last = DY;
  dxsize_last = DXSIZE;
  dysize_last = DYSIZE;
  vx_last = VX;
  vy_last = VY;
  vxsize_last = VXSIZE;
  vysize_last = VYSIZE;
  ex_last = EX;
  ey_last = EY;
  exsize_last = EXSIZE;
  eysize_last = EYSIZE;
}

void ClearField(void)
{
  RedrawGlobalBorderIfNeeded();

  // !!! "drawto" might still point to playfield buffer here (see above) !!!
  // (when entering hall of fame after playing)
  DrawBackground(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  // !!! maybe this should be done before clearing the background !!!
  if (game_status == GAME_MODE_PLAYING)
  {
    ClearRectangle(fieldbuffer, 0, 0, FXSIZE, FYSIZE);
    SetDrawtoField(DRAW_TO_FIELDBUFFER);
  }
  else
  {
    SetDrawtoField(DRAW_TO_BACKBUFFER);
  }
}

void MarkTileDirty(int x, int y)
{
  redraw_mask |= REDRAW_FIELD;
}

void SetBorderElement(void)
{
  int x, y;

  BorderElement = EL_EMPTY;

  // only the R'n'D game engine may use an additional steelwall border
  if (level.game_engine_type != GAME_ENGINE_TYPE_RND)
    return;

  for (y = 0; y < lev_fieldy && BorderElement == EL_EMPTY; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      if (!IS_INDESTRUCTIBLE(Tile[x][y]))
	BorderElement = EL_STEELWALL;

      if (y != 0 && y != lev_fieldy - 1 && x != lev_fieldx - 1)
	x = lev_fieldx - 2;
    }
  }
}

void FloodFillLevelExt(int start_x, int start_y, int fill_element,
		       int max_array_fieldx, int max_array_fieldy,
		       short field[max_array_fieldx][max_array_fieldy],
		       int max_fieldx, int max_fieldy)
{
  static struct XY stack_buffer[MAX_LEV_FIELDX * MAX_LEV_FIELDY];
  struct XY *check = xy_topdown;
  int old_element = field[start_x][start_y];
  int stack_pos = 0;

  // do nothing if start field already has the desired content
  if (old_element == fill_element)
    return;

  stack_buffer[stack_pos++] = (struct XY){ start_x, start_y };

  while (stack_pos > 0)
  {
    struct XY current = stack_buffer[--stack_pos];
    int i;

    field[current.x][current.y] = fill_element;

    for (i = 0; i < 4; i++)
    {
      int x = current.x + check[i].x;
      int y = current.y + check[i].y;

      // check for stack buffer overflow (should not happen)
      if (stack_pos >= MAX_LEV_FIELDX * MAX_LEV_FIELDY)
	Fail("Stack buffer overflow in 'FloodFillLevelExt()'. Please debug.");

      if (IN_FIELD(x, y, max_fieldx, max_fieldy) && field[x][y] == old_element)
	stack_buffer[stack_pos++] = (struct XY){ x, y };
    }
  }
}

void FloodFillLevel(int from_x, int from_y, int fill_element,
		    short field[MAX_LEV_FIELDX][MAX_LEV_FIELDY],
		    int max_fieldx, int max_fieldy)
{
  FloodFillLevelExt(from_x, from_y, fill_element,
		    MAX_LEV_FIELDX, MAX_LEV_FIELDY, field,
		    max_fieldx, max_fieldy);
}

void SetRandomAnimationValue(int x, int y)
{
  gfx.anim_random_frame = GfxRandom[x][y];
}

void SetAnimationFirstLevel(int first_level)
{
  gfx.anim_first_level = first_level;
}

int getGraphicAnimationFrame(int graphic, int sync_frame)
{
  // animation synchronized with global frame counter, not move position
  if (graphic_info[graphic].anim_global_sync || sync_frame < 0)
    sync_frame = FrameCounter;
  else if (graphic_info[graphic].anim_global_anim_sync)
    sync_frame = getGlobalAnimSyncFrame();

  return getAnimationFrame(graphic_info[graphic].anim_frames,
			   graphic_info[graphic].anim_delay,
			   graphic_info[graphic].anim_mode,
			   graphic_info[graphic].anim_start_frame,
			   sync_frame);
}

int getGraphicAnimationFrameXY(int graphic, int lx, int ly)
{
  if (graphic_info[graphic].anim_mode & ANIM_TILED)
  {
    struct GraphicInfo *g = &graphic_info[graphic];
    int xsize = MAX(1, g->anim_frames_per_line);
    int ysize = MAX(1, g->anim_frames / xsize);
    int xoffset = g->anim_start_frame % xsize;
    int yoffset = g->anim_start_frame % ysize;
    // may be needed if screen field is significantly larger than playfield
    int x = (lx + xoffset + SCR_FIELDX * xsize) % xsize;
    int y = (ly + yoffset + SCR_FIELDY * ysize) % ysize;
    int sync_frame = y * xsize + x;

    return sync_frame % g->anim_frames;
  }
  else if (graphic_info[graphic].anim_mode & ANIM_RANDOM_STATIC)
  {
    struct GraphicInfo *g = &graphic_info[graphic];
    // may be needed if screen field is significantly larger than playfield
    int x = (lx + SCR_FIELDX * lev_fieldx) % lev_fieldx;
    int y = (ly + SCR_FIELDY * lev_fieldy) % lev_fieldy;
    int sync_frame = GfxRandomStatic[x][y];

    return sync_frame % g->anim_frames;
  }
  else
  {
    int sync_frame = (IN_LEV_FIELD(lx, ly) ? GfxFrame[lx][ly] : -1);

    return getGraphicAnimationFrame(graphic, sync_frame);
  }
}

void getGraphicSourceBitmap(int graphic, int tilesize, Bitmap **bitmap)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  int tilesize_capped = MIN(MAX(1, tilesize), TILESIZE);

  if (tilesize == gfx.standard_tile_size)
    *bitmap = g->bitmaps[IMG_BITMAP_STANDARD];
  else if (tilesize == game.tile_size)
    *bitmap = g->bitmaps[IMG_BITMAP_PTR_GAME];
  else
    *bitmap = g->bitmaps[IMG_BITMAP_1x1 - log_2(tilesize_capped)];
}

void getGraphicSourceXY(int graphic, int frame, int *x, int *y,
			boolean get_backside)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  int src_x = g->src_x + (get_backside ? g->offset2_x : 0);
  int src_y = g->src_y + (get_backside ? g->offset2_y : 0);

  if (g->offset_y == 0)		// frames are ordered horizontally
  {
    int max_width = g->anim_frames_per_line * g->width;
    int pos = (src_y / g->height) * max_width + src_x + frame * g->offset_x;

    *x = pos % max_width;
    *y = src_y % g->height + pos / max_width * g->height;
  }
  else if (g->offset_x == 0)	// frames are ordered vertically
  {
    int max_height = g->anim_frames_per_line * g->height;
    int pos = (src_x / g->width) * max_height + src_y + frame * g->offset_y;

    *x = src_x % g->width + pos / max_height * g->width;
    *y = pos % max_height;
  }
  else				// frames are ordered diagonally
  {
    *x = src_x + frame * g->offset_x;
    *y = src_y + frame * g->offset_y;
  }
}

void getSizedGraphicSourceExt(int graphic, int frame, int tilesize,
			      Bitmap **bitmap, int *x, int *y,
			      boolean get_backside)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  // if no graphics defined at all, use fallback graphics
  if (g->bitmaps == NULL)
    *g = graphic_info[IMG_CHAR_EXCLAM];

  // if no in-game graphics defined, always use standard graphic size
  if (g->bitmaps[IMG_BITMAP_PTR_GAME] == NULL)
    tilesize = TILESIZE;

  getGraphicSourceBitmap(graphic, tilesize, bitmap);
  getGraphicSourceXY(graphic, frame, x, y, get_backside);

  *x = *x * tilesize / g->tile_size;
  *y = *y * tilesize / g->tile_size;
}

void getSizedGraphicSource(int graphic, int frame, int tilesize,
			   Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSourceExt(graphic, frame, tilesize, bitmap, x, y, FALSE);
}

void getFixedGraphicSource(int graphic, int frame,
			   Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSourceExt(graphic, frame, TILESIZE, bitmap, x, y, FALSE);
}

void getMiniGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSource(graphic, 0, MINI_TILESIZE, bitmap, x, y);
}

void getGlobalAnimGraphicSource(int graphic, int frame,
				Bitmap **bitmap, int *x, int *y)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  // if no graphics defined at all, use fallback graphics
  if (g->bitmaps == NULL)
    *g = graphic_info[IMG_CHAR_EXCLAM];

  // use original size graphics, if existing, else use standard size graphics
  if (g->bitmaps[IMG_BITMAP_PTR_ORIGINAL])
    *bitmap = g->bitmaps[IMG_BITMAP_PTR_ORIGINAL];
  else
    *bitmap = g->bitmaps[IMG_BITMAP_STANDARD];

  getGraphicSourceXY(graphic, frame, x, y, FALSE);
}

static void getGraphicSourceExt(int graphic, int frame, Bitmap **bitmap,
				int *x, int *y, boolean get_backside)
{
  getSizedGraphicSourceExt(graphic, frame, TILESIZE_VAR, bitmap, x, y,
			   get_backside);
}

void getGraphicSource(int graphic, int frame, Bitmap **bitmap, int *x, int *y)
{
  getGraphicSourceExt(graphic, frame, bitmap, x, y, FALSE);
}

void DrawGraphic(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("draw:DrawGraphic", "x = %d, y = %d, graphic = %d", x, y, graphic);
    Debug("draw:DrawGraphic", "This should never happen!");

    return;
  }
#endif

  DrawGraphicExt(drawto_field, FX + x * TILEX_VAR, FY + y * TILEY_VAR, graphic,
		 frame);

  MarkTileDirty(x, y);
}

void DrawFixedGraphic(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("draw:DrawFixedGraphic", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("draw:DrawFixedGraphic", "This should never happen!");

    return;
  }
#endif

  DrawFixedGraphicExt(drawto_field, FX + x * TILEX, FY + y * TILEY, graphic,
		      frame);
  MarkTileDirty(x, y);
}

void DrawGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic,
		    int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, TILEX_VAR, TILEY_VAR, x, y);
}

void DrawFixedGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic,
			 int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getFixedGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("draw:DrawGraphicThruMask", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("draw:DrawGraphicThruMask", "This should never happen!");

    return;
  }
#endif

  DrawGraphicThruMaskExt(drawto_field, FX + x * TILEX_VAR, FY + y * TILEY_VAR,
			 graphic, frame);

  MarkTileDirty(x, y);
}

void DrawFixedGraphicThruMask(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("draw:DrawFixedGraphicThruMask", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("draw:DrawFixedGraphicThruMask", "This should never happen!");

    return;
  }
#endif

  DrawFixedGraphicThruMaskExt(drawto_field, FX + x * TILEX, FY + y * TILEY,
			      graphic, frame);
  MarkTileDirty(x, y);
}

void DrawGraphicThruMaskExt(DrawBuffer *d, int dst_x, int dst_y, int graphic,
			    int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX_VAR, TILEY_VAR,
		   dst_x, dst_y);
}

void DrawFixedGraphicThruMaskExt(DrawBuffer *d, int dst_x, int dst_y,
				 int graphic, int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getFixedGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY,
		   dst_x, dst_y);
}

void DrawSizedGraphic(int x, int y, int graphic, int frame, int tilesize)
{
  DrawSizedGraphicExt(drawto, SX + x * tilesize, SY + y * tilesize, graphic,
		      frame, tilesize);
  MarkTileDirty(x / tilesize, y / tilesize);
}

void DrawSizedGraphicThruMask(int x, int y, int graphic, int frame,
			      int tilesize)
{
  DrawSizedGraphicThruMaskExt(drawto, SX + x * tilesize, SY + y * tilesize,
			      graphic, frame, tilesize);
  MarkTileDirty(x / tilesize, y / tilesize);
}

void DrawSizedGraphicExt(DrawBuffer *d, int x, int y, int graphic, int frame,
			 int tilesize)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getSizedGraphicSource(graphic, frame, tilesize, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, d, src_x, src_y, tilesize, tilesize, x, y);
}

void DrawSizedGraphicThruMaskExt(DrawBuffer *d, int x, int y, int graphic,
				 int frame, int tilesize)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getSizedGraphicSource(graphic, frame, tilesize, &src_bitmap, &src_x, &src_y);
  BlitBitmapMasked(src_bitmap, d, src_x, src_y, tilesize, tilesize, x, y);
}

void DrawMiniGraphic(int x, int y, int graphic)
{
  DrawMiniGraphicExt(drawto, SX + x * MINI_TILEX, SY + y * MINI_TILEY, graphic);
  MarkTileDirty(x / 2, y / 2);
}

void DrawMiniGraphicExt(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, d, src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

static void DrawGraphicShiftedNormal(int x, int y, int dx, int dy,
				     int graphic, int frame,
				     int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;

  if (dx || dy)			// shifted graphic
  {
    if (x < BX1)		// object enters playfield from the left
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		// object enters playfield from the right
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x == BX1 && dx < 0) // object leaves playfield to the left
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x == BX2 && dx > 0) // object leaves playfield to the right
      width -= dx;
    else if (dx)		// general horizontal movement
      MarkTileDirty(x + SIGN(dx), y);

    if (y < BY1)		// object enters playfield from the top
    {
      if (cut_mode == CUT_BELOW) // object completely above top border
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		// object enters playfield from the bottom
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y == BY1 && dy < 0) // object leaves playfield to the top
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		// object completely above bottom border
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				// object leaves playfield to the bottom
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
      height -= dy;
    else if (dy)		// general vertical movement
      MarkTileDirty(x, y + SIGN(dy));
  }

#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    Debug("draw:DrawGraphicShiftedNormal", "x = %d, y = %d, graphic = %d",
	  x, y, graphic);
    Debug("draw:DrawGraphicShiftedNormal", "This should never happen!");

    return;
  }
#endif

  width = width * TILESIZE_VAR / TILESIZE;
  height = height * TILESIZE_VAR / TILESIZE;
  cx = cx * TILESIZE_VAR / TILESIZE;
  cy = cy * TILESIZE_VAR / TILESIZE;
  dx = dx * TILESIZE_VAR / TILESIZE;
  dy = dy * TILESIZE_VAR / TILESIZE;

  if (width > 0 && height > 0)
  {
    getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

    src_x += cx;
    src_y += cy;

    dst_x = FX + x * TILEX_VAR + dx;
    dst_y = FY + y * TILEY_VAR + dy;

    if (mask_mode == USE_MASKING)
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x, y);
  }
}

static void DrawGraphicShiftedDouble(int x, int y, int dx, int dy,
				     int graphic, int frame,
				     int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int width = TILEX_VAR, height = TILEY_VAR;
  int x1 = x;
  int y1 = y;
  int x2 = x + SIGN(dx);
  int y2 = y + SIGN(dy);

  // movement with two-tile animations must be sync'ed with movement position,
  // not with current GfxFrame (which can be higher when using slow movement)
  int anim_pos = (dx ? ABS(dx) : ABS(dy));
  int anim_frames = graphic_info[graphic].anim_frames;

  // (we also need anim_delay here for movement animations with less frames)
  int anim_delay = graphic_info[graphic].anim_delay;
  int sync_frame = anim_pos * anim_frames * anim_delay / TILESIZE;

  boolean draw_start_tile = (cut_mode != CUT_ABOVE);	// only for falling!
  boolean draw_end_tile   = (cut_mode != CUT_BELOW);	// only for falling!

  // re-calculate animation frame for two-tile movement animation
  frame = getGraphicAnimationFrame(graphic, sync_frame);

  // check if movement start graphic inside screen area and should be drawn
  if (draw_start_tile && IN_SCR_FIELD(x1, y1))
  {
    getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, TRUE);

    dst_x = FX + x1 * TILEX_VAR;
    dst_y = FY + y1 * TILEY_VAR;

    if (mask_mode == USE_MASKING)
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x1, y1);
  }

  // check if movement end graphic inside screen area and should be drawn
  if (draw_end_tile && IN_SCR_FIELD(x2, y2))
  {
    getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, FALSE);

    dst_x = FX + x2 * TILEX_VAR;
    dst_y = FY + y2 * TILEY_VAR;

    if (mask_mode == USE_MASKING)
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x2, y2);
  }
}

static void DrawGraphicShifted(int x, int y, int dx, int dy,
			       int graphic, int frame,
			       int cut_mode, int mask_mode)
{
  if (graphic < 0)
  {
    DrawGraphic(x, y, graphic, frame);

    return;
  }

  if (graphic_info[graphic].double_movement)	// EM style movement images
    DrawGraphicShiftedDouble(x, y, dx, dy, graphic, frame, cut_mode, mask_mode);
  else
    DrawGraphicShiftedNormal(x, y, dx, dy, graphic, frame, cut_mode, mask_mode);
}

static void DrawGraphicShiftedThruMask(int x, int y, int dx, int dy,
				       int graphic, int frame, int cut_mode)
{
  DrawGraphicShifted(x, y, dx, dy, graphic, frame, cut_mode, USE_MASKING);
}

void DrawScreenElementExt(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int graphic;
  int frame;

  if (IN_LEV_FIELD(lx, ly))
  {
    if (element == EL_EMPTY)
      element = GfxElementEmpty[lx][ly];

    SetRandomAnimationValue(lx, ly);

    graphic = el_act_dir2img(element, GfxAction[lx][ly], GfxDir[lx][ly]);
    frame = getGraphicAnimationFrameXY(graphic, lx, ly);

    // do not use double (EM style) movement graphic when not moving
    if (graphic_info[graphic].double_movement && !dx && !dy)
    {
      graphic = el_act_dir2img(element, ACTION_DEFAULT, GfxDir[lx][ly]);
      frame = getGraphicAnimationFrameXY(graphic, lx, ly);
    }

    if (game.use_masked_elements && (dx || dy))
      mask_mode = USE_MASKING;
  }
  else	// border element
  {
    graphic = el2img(element);
    frame = getGraphicAnimationFrameXY(graphic, lx, ly);
  }

  if (element == EL_EXPANDABLE_WALL)
  {
    boolean left_stopped = FALSE, right_stopped = FALSE;

    if (!IN_LEV_FIELD(lx - 1, ly) || IS_WALL(Tile[lx - 1][ly]))
      left_stopped = TRUE;
    if (!IN_LEV_FIELD(lx + 1, ly) || IS_WALL(Tile[lx + 1][ly]))
      right_stopped = TRUE;

    if (left_stopped && right_stopped)
      graphic = IMG_WALL;
    else if (left_stopped)
    {
      graphic = IMG_EXPANDABLE_WALL_GROWING_RIGHT;
      frame = graphic_info[graphic].anim_frames - 1;
    }
    else if (right_stopped)
    {
      graphic = IMG_EXPANDABLE_WALL_GROWING_LEFT;
      frame = graphic_info[graphic].anim_frames - 1;
    }
  }

  if (dx || dy)
    DrawGraphicShifted(x, y, dx, dy, graphic, frame, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask(x, y, graphic, frame);
  else
    DrawGraphic(x, y, graphic, frame);
}

void DrawLevelElementExt(int x, int y, int dx, int dy, int element,
			 int cut_mode, int mask_mode)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElementExt(SCREENX(x), SCREENY(y), dx, dy, element,
			 cut_mode, mask_mode);
}

void DrawScreenElementShifted(int x, int y, int dx, int dy, int element,
			      int cut_mode)
{
  DrawScreenElementExt(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementShifted(int x, int y, int dx, int dy, int element,
			     int cut_mode)
{
  DrawLevelElementExt(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementThruMask(int x, int y, int element)
{
  DrawLevelElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask(int x, int y)
{
  DrawLevelElementExt(x, y, 0, 0, Tile[x][y], NO_CUTTING, USE_MASKING);
}

// !!! implementation of quicksand is totally broken !!!
#define IS_CRUMBLED_TILE(x, y, e)					\
	(GFX_CRUMBLED(e) && (!IN_LEV_FIELD(x, y) ||			\
			     !IS_MOVING(x, y) ||			\
			     (e) == EL_QUICKSAND_EMPTYING ||		\
			     (e) == EL_QUICKSAND_FAST_EMPTYING))

static void DrawLevelFieldCrumbledInnerCorners(int x, int y, int dx, int dy,
					       int graphic)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int width, height, cx, cy;
  int sx = SCREENX(x), sy = SCREENY(y);
  int crumbled_border_size = graphic_info[graphic].border_size;
  int crumbled_tile_size = graphic_info[graphic].tile_size;
  int crumbled_border_size_var =
    crumbled_border_size * TILESIZE_VAR / crumbled_tile_size;
  int i;

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  for (i = 1; i < 4; i++)
  {
    int dxx = (i & 1 ? dx : 0);
    int dyy = (i & 2 ? dy : 0);
    int xx = x + dxx;
    int yy = y + dyy;
    int element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		   BorderElement);

    // check if neighbour field is of same crumble type
    boolean same = (IS_CRUMBLED_TILE(xx, yy, element) &&
		    graphic_info[graphic].class ==
		    graphic_info[el_act2crm(element, ACTION_DEFAULT)].class);

    // return if check prevents inner corner
    if (same == (dxx == dx && dyy == dy))
      return;
  }

  // if we reach this point, we have an inner corner

  getGraphicSource(graphic, 1, &src_bitmap, &src_x, &src_y);

  width  = crumbled_border_size_var;
  height = crumbled_border_size_var;
  cx = (dx > 0 ? TILESIZE_VAR - width  : 0);
  cy = (dy > 0 ? TILESIZE_VAR - height : 0);

  if (game.use_masked_elements)
  {
    int graphic0 = el2img(EL_EMPTY);
    int frame0 = getGraphicAnimationFrameXY(graphic0, x, y);
    Bitmap *src_bitmap0;
    int src_x0, src_y0;

    getGraphicSource(graphic0, frame0, &src_bitmap0, &src_x0, &src_y0);

    BlitBitmap(src_bitmap0, drawto_field, src_x0 + cx, src_y0 + cy,
	       width, height,
	       FX + sx * TILEX_VAR + cx, FY + sy * TILEY_VAR + cy);

    BlitBitmapMasked(src_bitmap, drawto_field, src_x + cx, src_y + cy,
		     width, height,
		     FX + sx * TILEX_VAR + cx, FY + sy * TILEY_VAR + cy);
  }
  else
    BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
	       width, height,
	       FX + sx * TILEX_VAR + cx, FY + sy * TILEY_VAR + cy);
}

static void DrawLevelFieldCrumbledBorders(int x, int y, int graphic, int frame,
					  int dir)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int width, height, bx, by, cx, cy;
  int sx = SCREENX(x), sy = SCREENY(y);
  int crumbled_border_size = graphic_info[graphic].border_size;
  int crumbled_tile_size = graphic_info[graphic].tile_size;
  int crumbled_border_size_var =
    crumbled_border_size * TILESIZE_VAR / crumbled_tile_size;
  int crumbled_border_pos_var = TILESIZE_VAR - crumbled_border_size_var;
  int i;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  // only needed when using masked elements
  int graphic0 = el2img(EL_EMPTY);
  int frame0 = getGraphicAnimationFrameXY(graphic0, x, y);
  Bitmap *src_bitmap0;
  int src_x0, src_y0;

  if (game.use_masked_elements)
    getGraphicSource(graphic0, frame0, &src_bitmap0, &src_x0, &src_y0);

  // draw simple, sloppy, non-corner-accurate crumbled border

  width  = (dir == 1 || dir == 2 ? crumbled_border_size_var : TILESIZE_VAR);
  height = (dir == 0 || dir == 3 ? crumbled_border_size_var : TILESIZE_VAR);
  cx = (dir == 2 ? crumbled_border_pos_var : 0);
  cy = (dir == 3 ? crumbled_border_pos_var : 0);

  if (game.use_masked_elements)
  {
    BlitBitmap(src_bitmap0, drawto_field, src_x0 + cx, src_y0 + cy,
	       width, height,
	       FX + sx * TILEX_VAR + cx,
	       FY + sy * TILEY_VAR + cy);

    BlitBitmapMasked(src_bitmap, drawto_field, src_x + cx, src_y + cy,
		     width, height,
		     FX + sx * TILEX_VAR + cx,
		     FY + sy * TILEY_VAR + cy);
  }
  else
    BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
	       width, height,
	       FX + sx * TILEX_VAR + cx,
	       FY + sy * TILEY_VAR + cy);

  // (remaining middle border part must be at least as big as corner part)
  if (!(graphic_info[graphic].style & STYLE_ACCURATE_BORDERS) ||
      crumbled_border_size_var >= TILESIZE_VAR / 3)
    return;

  // correct corners of crumbled border, if needed

  for (i = -1; i <= 1; i += 2)
  {
    int xx = x + (dir == 0 || dir == 3 ? i : 0);
    int yy = y + (dir == 1 || dir == 2 ? i : 0);
    int element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		   BorderElement);

    // check if neighbour field is of same crumble type
    if (IS_CRUMBLED_TILE(xx, yy, element) &&
	graphic_info[graphic].class ==
	graphic_info[el_act2crm(element, ACTION_DEFAULT)].class)
    {
      // no crumbled corner, but continued crumbled border

      int c1 = (dir == 2 || dir == 3 ? crumbled_border_pos_var : 0);
      int c2 = (i == 1 ? crumbled_border_pos_var : 0);
      int b1 = (i == 1 ? crumbled_border_size_var :
		TILESIZE_VAR - 2 * crumbled_border_size_var);

      width  = crumbled_border_size_var;
      height = crumbled_border_size_var;

      if (dir == 1 || dir == 2)
      {
	cx = c1;
	cy = c2;
	bx = cx;
	by = b1;
      }
      else
      {
	cx = c2;
	cy = c1;
	bx = b1;
	by = cy;
      }

      if (game.use_masked_elements)
      {
	BlitBitmap(src_bitmap0, drawto_field, src_x0 + bx, src_y0 + by,
		   width, height,
		   FX + sx * TILEX_VAR + cx,
		   FY + sy * TILEY_VAR + cy);

	BlitBitmapMasked(src_bitmap, drawto_field, src_x + bx, src_y + by,
			 width, height,
			 FX + sx * TILEX_VAR + cx,
			 FY + sy * TILEY_VAR + cy);
      }
      else
	BlitBitmap(src_bitmap, drawto_field, src_x + bx, src_y + by,
		   width, height,
		   FX + sx * TILEX_VAR + cx,
		   FY + sy * TILEY_VAR + cy);
    }
  }
}

static void DrawLevelFieldCrumbledExt(int x, int y, int graphic, int frame)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int element;
  int i;
  struct XY *xy = xy_topdown;

  if (!IN_LEV_FIELD(x, y))
    return;

  element = TILE_GFX_ELEMENT(x, y);

  if (IS_CRUMBLED_TILE(x, y, element))		// crumble field itself
  {
    if (!IN_SCR_FIELD(sx, sy))
      return;

    // crumble field borders towards direct neighbour fields
    for (i = 0; i < 4; i++)
    {
      int xx = x + xy[i].x;
      int yy = y + xy[i].y;

      element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		 BorderElement);

      // check if neighbour field is of same crumble type
      if (IS_CRUMBLED_TILE(xx, yy, element) &&
	  graphic_info[graphic].class ==
	  graphic_info[el_act2crm(element, ACTION_DEFAULT)].class)
	continue;

      DrawLevelFieldCrumbledBorders(x, y, graphic, frame, i);
    }

    // crumble inner field corners towards corner neighbour fields
    if ((graphic_info[graphic].style & STYLE_INNER_CORNERS) &&
	graphic_info[graphic].anim_frames == 2)
    {
      for (i = 0; i < 4; i++)
      {
	int dx = (i & 1 ? +1 : -1);
	int dy = (i & 2 ? +1 : -1);

	DrawLevelFieldCrumbledInnerCorners(x, y, dx, dy, graphic);
      }
    }

    MarkTileDirty(sx, sy);
  }
  else		// center field is not crumbled -- crumble neighbour fields
  {
    // crumble field borders of direct neighbour fields
    for (i = 0; i < 4; i++)
    {
      int xx = x + xy[i].x;
      int yy = y + xy[i].y;
      int sxx = sx + xy[i].x;
      int syy = sy + xy[i].y;

      if (!IN_LEV_FIELD(xx, yy) ||
	  !IN_SCR_FIELD(sxx, syy))
	continue;

      // do not crumble fields that are being digged or snapped
      if (Tile[xx][yy] == EL_EMPTY ||
	  Tile[xx][yy] == EL_ELEMENT_SNAPPING)
	continue;

      element = TILE_GFX_ELEMENT(xx, yy);

      if (!IS_CRUMBLED_TILE(xx, yy, element))
	continue;

      graphic = el_act2crm(element, ACTION_DEFAULT);

      DrawLevelFieldCrumbledBorders(xx, yy, graphic, 0, 3 - i);

      MarkTileDirty(sxx, syy);
    }

    // crumble inner field corners of corner neighbour fields
    for (i = 0; i < 4; i++)
    {
      int dx = (i & 1 ? +1 : -1);
      int dy = (i & 2 ? +1 : -1);
      int xx = x + dx;
      int yy = y + dy;
      int sxx = sx + dx;
      int syy = sy + dy;

      if (!IN_LEV_FIELD(xx, yy) ||
	  !IN_SCR_FIELD(sxx, syy))
	continue;

      if (Tile[xx][yy] == EL_ELEMENT_SNAPPING)
	continue;

      element = TILE_GFX_ELEMENT(xx, yy);

      if (!IS_CRUMBLED_TILE(xx, yy, element))
	continue;

      graphic = el_act2crm(element, ACTION_DEFAULT);

      if ((graphic_info[graphic].style & STYLE_INNER_CORNERS) &&
	  graphic_info[graphic].anim_frames == 2)
	DrawLevelFieldCrumbledInnerCorners(xx, yy, -dx, -dy, graphic);

      MarkTileDirty(sxx, syy);
    }
  }
}

void DrawLevelFieldCrumbled(int x, int y)
{
  int graphic;

  if (!IN_LEV_FIELD(x, y))
    return;

  if (Tile[x][y] == EL_ELEMENT_SNAPPING &&
      GfxElement[x][y] != EL_UNDEFINED &&
      GFX_CRUMBLED(GfxElement[x][y]))
  {
    DrawLevelFieldCrumbledDigging(x, y, GfxDir[x][y], GfxFrame[x][y]);

    return;
  }

  graphic = el_act2crm(TILE_GFX_ELEMENT(x, y), ACTION_DEFAULT);

  DrawLevelFieldCrumbledExt(x, y, graphic, 0);
}

void DrawLevelFieldCrumbledDigging(int x, int y, int direction,
				   int step_frame)
{
  int graphic1 = el_act_dir2img(GfxElement[x][y], ACTION_DIGGING, direction);
  int graphic2 = el_act_dir2crm(GfxElement[x][y], ACTION_DIGGING, direction);
  int frame1 = getGraphicAnimationFrame(graphic1, step_frame);
  int frame2 = getGraphicAnimationFrame(graphic2, step_frame);
  int sx = SCREENX(x), sy = SCREENY(y);

  DrawScreenGraphic(sx, sy, graphic1, frame1);
  DrawLevelFieldCrumbledExt(x, y, graphic2, frame2);
}

void DrawLevelFieldCrumbledNeighbours(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  struct XY *xy = xy_topdown;
  int i;

  // crumble direct neighbour fields (required for field borders)
  for (i = 0; i < 4; i++)
  {
    int xx = x + xy[i].x;
    int yy = y + xy[i].y;
    int sxx = sx + xy[i].x;
    int syy = sy + xy[i].y;

    if (!IN_LEV_FIELD(xx, yy) ||
	!IN_SCR_FIELD(sxx, syy) ||
	!GFX_CRUMBLED(Tile[xx][yy]) ||
	IS_MOVING(xx, yy))
      continue;

    DrawLevelField(xx, yy);
  }

  // crumble corner neighbour fields (required for inner field corners)
  for (i = 0; i < 4; i++)
  {
    int dx = (i & 1 ? +1 : -1);
    int dy = (i & 2 ? +1 : -1);
    int xx = x + dx;
    int yy = y + dy;
    int sxx = sx + dx;
    int syy = sy + dy;

    if (!IN_LEV_FIELD(xx, yy) ||
	!IN_SCR_FIELD(sxx, syy) ||
	!GFX_CRUMBLED(Tile[xx][yy]) ||
	IS_MOVING(xx, yy))
      continue;

    int element = TILE_GFX_ELEMENT(xx, yy);
    int graphic = el_act2crm(element, ACTION_DEFAULT);

    if ((graphic_info[graphic].style & STYLE_INNER_CORNERS) &&
	graphic_info[graphic].anim_frames == 2)
      DrawLevelField(xx, yy);
  }
}

static int getBorderElement(int x, int y)
{
  int border[7][2] =
  {
    { EL_STEELWALL_TOPLEFT,		EL_INVISIBLE_STEELWALL_TOPLEFT     },
    { EL_STEELWALL_TOPRIGHT,		EL_INVISIBLE_STEELWALL_TOPRIGHT    },
    { EL_STEELWALL_BOTTOMLEFT,		EL_INVISIBLE_STEELWALL_BOTTOMLEFT  },
    { EL_STEELWALL_BOTTOMRIGHT,		EL_INVISIBLE_STEELWALL_BOTTOMRIGHT },
    { EL_STEELWALL_VERTICAL,		EL_INVISIBLE_STEELWALL_VERTICAL    },
    { EL_STEELWALL_HORIZONTAL,		EL_INVISIBLE_STEELWALL_HORIZONTAL  },
    { EL_STEELWALL,			EL_INVISIBLE_STEELWALL		   }
  };
  int steel_type = (BorderElement == EL_STEELWALL ? 0 : 1);
  int steel_position = (x == -1		&& y == -1		? 0 :
			x == lev_fieldx	&& y == -1		? 1 :
			x == -1		&& y == lev_fieldy	? 2 :
			x == lev_fieldx	&& y == lev_fieldy	? 3 :
			x == -1		|| x == lev_fieldx	? 4 :
			y == -1		|| y == lev_fieldy	? 5 : 6);

  return border[steel_position][steel_type];
}

void DrawScreenGraphic(int x, int y, int graphic, int frame)
{
  if (game.use_masked_elements)
  {
    if (graphic != el2img(EL_EMPTY))
      DrawScreenElementExt(x, y, 0, 0, EL_EMPTY, NO_CUTTING, NO_MASKING);

    DrawGraphicThruMask(x, y, graphic, frame);
  }
  else
  {
    DrawGraphic(x, y, graphic, frame);
  }
}

void DrawLevelGraphic(int x, int y, int graphic, int frame)
{
  DrawScreenGraphic(SCREENX(x), SCREENY(y), graphic, frame);
}

void DrawScreenElement(int x, int y, int element)
{
  int mask_mode = NO_MASKING;

  if (game.use_masked_elements)
  {
    int lx = LEVELX(x), ly = LEVELY(y);

    if (IN_LEV_FIELD(lx, ly) && element != EL_EMPTY)
    {
      DrawScreenElementExt(x, y, 0, 0, EL_EMPTY, NO_CUTTING, NO_MASKING);

      mask_mode = USE_MASKING;
    }
  }

  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, mask_mode);
  DrawLevelFieldCrumbled(LEVELX(x), LEVELY(y));
}

void DrawLevelElement(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField(int x, int y)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int element, content;

  if (!IN_LEV_FIELD(lx, ly))
  {
    if (lx < -1 || lx > lev_fieldx || ly < -1 || ly > lev_fieldy)
      element = EL_EMPTY;
    else
      element = getBorderElement(lx, ly);

    DrawScreenElement(x, y, element);

    return;
  }

  element = Tile[lx][ly];
  content = Store[lx][ly];

  if (IS_MOVING(lx, ly))
  {
    int horiz_move = (MovDir[lx][ly] == MV_LEFT || MovDir[lx][ly] == MV_RIGHT);
    boolean cut_mode = NO_CUTTING;

    if (element == EL_QUICKSAND_EMPTYING ||
	element == EL_QUICKSAND_FAST_EMPTYING ||
	element == EL_MAGIC_WALL_EMPTYING ||
	element == EL_BD_MAGIC_WALL_EMPTYING ||
	element == EL_DC_MAGIC_WALL_EMPTYING ||
	element == EL_AMOEBA_DROPPING)
      cut_mode = CUT_ABOVE;
    else if (element == EL_QUICKSAND_FILLING ||
	     element == EL_QUICKSAND_FAST_FILLING ||
	     element == EL_MAGIC_WALL_FILLING ||
	     element == EL_BD_MAGIC_WALL_FILLING ||
	     element == EL_DC_MAGIC_WALL_FILLING)
      cut_mode = CUT_BELOW;

    if (cut_mode == CUT_ABOVE)
      DrawScreenElement(x, y, element);
    else
      DrawScreenElement(x, y, EL_EMPTY);

    if (cut_mode != CUT_BELOW && game.use_masked_elements)
    {
      int dir = MovDir[lx][ly];
      int newx = x + (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
      int newy = y + (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);

      if (IN_SCR_FIELD(newx, newy))
	DrawScreenElement(newx, newy, EL_EMPTY);
    }

    if (horiz_move)
      DrawScreenElementShifted(x, y, MovPos[lx][ly], 0, element, NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(x, y, 0, MovPos[lx][ly], element, cut_mode);
    else
    {
      DrawScreenElementShifted(x, y, 0, MovPos[lx][ly], content, cut_mode);

      if (cut_mode == CUT_BELOW &&
	  IN_LEV_FIELD(lx, ly + 1) && IN_SCR_FIELD(x, y + 1))
	DrawLevelElement(lx, ly + 1, element);
    }

    if (content == EL_ACID)
    {
      int dir = MovDir[lx][ly];
      int newlx = lx + (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
      int newly = ly + (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);

      DrawLevelElementThruMask(newlx, newly, EL_ACID);

      // prevent target field from being drawn again (but without masking)
      // (this would happen if target field is scanned after moving element)
      Stop[newlx][newly] = TRUE;
    }
  }
  else if (IS_BLOCKED(lx, ly))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;
    boolean cut_mode = NO_CUTTING;
    int element_old, content_old;

    Blocked2Moving(lx, ly, &oldx, &oldy);
    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    element_old = Tile[oldx][oldy];
    content_old = Store[oldx][oldy];

    if (element_old == EL_QUICKSAND_EMPTYING ||
	element_old == EL_QUICKSAND_FAST_EMPTYING ||
	element_old == EL_MAGIC_WALL_EMPTYING ||
	element_old == EL_BD_MAGIC_WALL_EMPTYING ||
	element_old == EL_DC_MAGIC_WALL_EMPTYING ||
	element_old == EL_AMOEBA_DROPPING)
      cut_mode = CUT_ABOVE;

    DrawScreenElement(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted(sx, sy, MovPos[oldx][oldy], 0, element_old,
			       NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(sx, sy, 0, MovPos[oldx][oldy], element_old,
			       cut_mode);
    else
      DrawScreenElementShifted(sx, sy, 0, MovPos[oldx][oldy], content_old,
			       cut_mode);
  }
  else if (IS_DRAWABLE(element))
    DrawScreenElement(x, y, element);
  else
    DrawScreenElement(x, y, EL_EMPTY);
}

void DrawLevelField(int x, int y)
{
  if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenField(SCREENX(x), SCREENY(y));
  else if (IS_MOVING(x, y))
  {
    int newx, newy;

    Moving2Blocked(x, y, &newx, &newy);
    if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
      DrawScreenField(SCREENX(newx), SCREENY(newy));
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);
    if (IN_SCR_FIELD(SCREENX(oldx), SCREENY(oldy)))
      DrawScreenField(SCREENX(oldx), SCREENY(oldy));
  }
}

static void DrawSizedWallExt_MM(int dst_x, int dst_y, int element, int tilesize,
				int (*el2img_function)(int), boolean masked,
				int element_bits_draw)
{
  int element_base = map_mm_wall_element(element);
  int element_bits = (IS_DF_WALL(element) ?
		      element - EL_DF_WALL_START :
		      IS_MM_WALL(element) ?
		      element - EL_MM_WALL_START : EL_EMPTY) & 0x000f;
  int graphic = el2img_function(element_base);
  int tilesize_draw = tilesize / 2;
  Bitmap *src_bitmap;
  int src_x, src_y;
  int i;

  getSizedGraphicSource(graphic, 0, tilesize_draw, &src_bitmap, &src_x, &src_y);

  for (i = 0; i < 4; i++)
  {
    int dst_draw_x = dst_x + (i % 2) * tilesize_draw;
    int dst_draw_y = dst_y + (i / 2) * tilesize_draw;

    if (!(element_bits_draw & (1 << i)))
      continue;

    if (element_bits & (1 << i))
    {
      if (masked)
	BlitBitmapMasked(src_bitmap, drawto, src_x, src_y,
			 tilesize_draw, tilesize_draw, dst_draw_x, dst_draw_y);
      else
	BlitBitmap(src_bitmap, drawto, src_x, src_y,
		   tilesize_draw, tilesize_draw, dst_draw_x, dst_draw_y);
    }
    else
    {
      if (!masked)
	ClearRectangle(drawto, dst_draw_x, dst_draw_y,
		       tilesize_draw, tilesize_draw);
    }
  }
}

void DrawSizedWallParts_MM(int x, int y, int element, int tilesize,
			   boolean masked, int element_bits_draw)
{
  DrawSizedWallExt_MM(SX + x * tilesize, SY + y * tilesize,
		      element, tilesize, el2edimg, masked, element_bits_draw);
}

static void DrawSizedWall_MM(int dst_x, int dst_y, int element, int tilesize,
			     int (*el2img_function)(int))
{
  DrawSizedWallExt_MM(dst_x, dst_y, element, tilesize, el2img_function, FALSE,
		      0x000f);
}

static void DrawSizedElementExt(int x, int y, int element, int tilesize,
				boolean masked)
{
  if (IS_MM_WALL(element))
  {
    DrawSizedWallExt_MM(SX + x * tilesize, SY + y * tilesize,
			element, tilesize, el2edimg, masked, 0x000f);
  }
  else
  {
    int graphic, frame;

    el2edimg_with_frame(element, &graphic, &frame);

    if (masked)
      DrawSizedGraphicThruMask(x, y, graphic, frame, tilesize);
    else
      DrawSizedGraphic(x, y, graphic, frame, tilesize);
  }
}

void DrawSizedElement(int x, int y, int element, int tilesize)
{
  DrawSizedElementExt(x, y, element, tilesize, FALSE);
}

void DrawSizedElementThruMask(int x, int y, int element, int tilesize)
{
  DrawSizedElementExt(x, y, element, tilesize, TRUE);
}

void DrawMiniElement(int x, int y, int element)
{
  int graphic;

  graphic = el2edimg(element);
  DrawMiniGraphic(x, y, graphic);
}

void DrawSizedElementOrWall(int sx, int sy, int scroll_x, int scroll_y,
			    int tilesize)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawSizedElement(sx, sy, EL_EMPTY, tilesize);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawSizedElement(sx, sy, Tile[x][y], tilesize);
  else
    DrawSizedGraphic(sx, sy, el2edimg(getBorderElement(x, y)), 0, tilesize);
}

void DrawMiniElementOrWall(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement(sx, sy, EL_EMPTY);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement(sx, sy, Tile[x][y]);
  else
    DrawMiniGraphic(sx, sy, el2edimg(getBorderElement(x, y)));
}

static void DrawEnvelopeBackgroundTiles(int graphic, int startx, int starty,
					int x, int y, int xsize, int ysize,
					int tile_width, int tile_height)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x = startx + x * tile_width;
  int dst_y = starty + y * tile_height;
  int width  = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  int inner_width_raw  = MAX(width  - 2 * tile_width,  tile_width);
  int inner_height_raw = MAX(height - 2 * tile_height, tile_height);
  int inner_width  = inner_width_raw  - (inner_width_raw  % tile_width);
  int inner_height = inner_height_raw - (inner_height_raw % tile_height);
  int inner_sx = (width  >= 3 * tile_width  ? tile_width  : 0);
  int inner_sy = (height >= 3 * tile_height ? tile_height : 0);
  boolean draw_masked = graphic_info[graphic].draw_masked;

  getFixedGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  if (src_bitmap == NULL || width < tile_width || height < tile_height)
  {
    ClearRectangle(drawto, dst_x, dst_y, tile_width, tile_height);
    return;
  }

  src_x += (x == 0 ? 0 : x == xsize - 1 ? width  - tile_width  :
	    inner_sx + (x - 1) * tile_width  % inner_width);
  src_y += (y == 0 ? 0 : y == ysize - 1 ? height - tile_height :
	    inner_sy + (y - 1) * tile_height % inner_height);

  if (draw_masked)
    BlitBitmapMasked(src_bitmap, drawto, src_x, src_y, tile_width, tile_height,
		     dst_x, dst_y);
  else
    BlitBitmap(src_bitmap, drawto, src_x, src_y, tile_width, tile_height,
	       dst_x, dst_y);
}

static void DrawEnvelopeBackground(int graphic, int startx, int starty,
				   int x, int y, int xsize, int ysize,
				   int font_nr)
{
  int font_width  = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);

  DrawEnvelopeBackgroundTiles(graphic, startx, starty, x, y, xsize, ysize,
			      font_width, font_height);
}

static void AnimateEnvelope(int envelope_nr, int anim_mode, int action)
{
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int mask_mode = (src_bitmap != NULL ? BLIT_MASKED : BLIT_ON_BACKGROUND);
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int anim_delay_value = MAX(1, (no_delay ? 0 : frame_delay_value) / 2);
  DelayCounter anim_delay = { anim_delay_value };
  int font_nr = FONT_ENVELOPE_1 + envelope_nr;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int max_xsize = level.envelope[envelope_nr].xsize;
  int max_ysize = level.envelope[envelope_nr].ysize;
  int xstart = (anim_mode & ANIM_VERTICAL ? max_xsize : 0);
  int ystart = (anim_mode & ANIM_HORIZONTAL ? max_ysize : 0);
  int xend = max_xsize;
  int yend = (anim_mode != ANIM_DEFAULT ? max_ysize : 0);
  int xstep = (xstart < xend ? 1 : 0);
  int ystep = (ystart < yend || xstep == 0 ? 1 : 0);
  int start = 0;
  int end = MAX(xend - xstart, yend - ystart);
  int i;

  for (i = start; i <= end; i++)
  {
    int last_frame = end;	// last frame of this "for" loop
    int x = xstart + i * xstep;
    int y = ystart + i * ystep;
    int xsize = (action == ACTION_CLOSING ? xend - (x - xstart) : x) + 2;
    int ysize = (action == ACTION_CLOSING ? yend - (y - ystart) : y) + 2;
    int sx = SX + (SXSIZE - xsize * font_width)  / 2;
    int sy = SY + (SYSIZE - ysize * font_height) / 2;
    int xx, yy;

    SetDrawtoField(DRAW_TO_FIELDBUFFER);

    BlitScreenToBitmap(backbuffer);

    SetDrawtoField(DRAW_TO_BACKBUFFER);

    for (yy = 0; yy < ysize; yy++)
      for (xx = 0; xx < xsize; xx++)
	DrawEnvelopeBackground(graphic, sx, sy, xx, yy, xsize, ysize, font_nr);

    DrawTextArea(sx + font_width, sy + font_height,
		 level.envelope[envelope_nr].text, font_nr, max_xsize,
		 xsize - 2, ysize - 2, -1, -1, -1, 0, mask_mode,
		 level.envelope[envelope_nr].autowrap,
		 level.envelope[envelope_nr].centered, FALSE);

    redraw_mask |= REDRAW_FIELD;
    BackToFront();

    SkipUntilDelayReached(&anim_delay, &i, last_frame);
  }

  ClearAutoRepeatKeyEvents();
}

void ShowEnvelope(int envelope_nr)
{
  int element = EL_ENVELOPE_1 + envelope_nr;
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  int sound_opening = element_info[element].sound[ACTION_OPENING];
  int sound_closing = element_info[element].sound[ACTION_CLOSING];
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int normal_delay_value = ONE_SECOND_DELAY / (ffwd_delay ? 2 : 1);
  int wait_delay_value = (no_delay ? 0 : normal_delay_value);
  int anim_mode = graphic_info[graphic].anim_mode;
  int main_anim_mode = (anim_mode == ANIM_NONE ? ANIM_VERTICAL|ANIM_HORIZONTAL:
			anim_mode == ANIM_DEFAULT ? ANIM_VERTICAL : anim_mode);
  boolean overlay_enabled = GetOverlayEnabled();

  game.envelope_active = TRUE;	// needed for RedrawPlayfield() events

  SetOverlayEnabled(FALSE);
  UnmapAllGadgets();

  PlayMenuSoundStereo(sound_opening, SOUND_MIDDLE);

  if (anim_mode == ANIM_DEFAULT)
    AnimateEnvelope(envelope_nr, ANIM_DEFAULT, ACTION_OPENING);

  AnimateEnvelope(envelope_nr, main_anim_mode, ACTION_OPENING);

  if (tape.playing)
    Delay_WithScreenUpdates(wait_delay_value);
  else
    WaitForEventToContinue();

  RemapAllGadgets();
  SetOverlayEnabled(overlay_enabled);

  PlayMenuSoundStereo(sound_closing, SOUND_MIDDLE);

  if (anim_mode != ANIM_NONE)
    AnimateEnvelope(envelope_nr, main_anim_mode, ACTION_CLOSING);

  if (anim_mode == ANIM_DEFAULT)
    AnimateEnvelope(envelope_nr, ANIM_DEFAULT, ACTION_CLOSING);

  game.envelope_active = FALSE;

  SetDrawtoField(DRAW_TO_FIELDBUFFER);

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
}

static void PrepareEnvelopeRequestToScreen(Bitmap *bitmap, int sx, int sy,
					   int xsize, int ysize)
{
  if (!global.use_envelope_request)
    return;

  if (menu.request.bitmap == NULL ||
      xsize > menu.request.xsize ||
      ysize > menu.request.ysize)
  {
    if (menu.request.bitmap != NULL)
      FreeBitmap(menu.request.bitmap);

    menu.request.bitmap = CreateBitmap(xsize, ysize, DEFAULT_DEPTH);

    SDL_Surface *surface = menu.request.bitmap->surface;

    if ((menu.request.bitmap->surface_masked = SDLGetNativeSurface(surface)) == NULL)
      Fail("SDLGetNativeSurface() failed");
  }

  BlitBitmap(bitmap, menu.request.bitmap, sx, sy, xsize, ysize, 0, 0);

  // create masked surface for request bitmap, if needed
  if (graphic_info[IMG_BACKGROUND_REQUEST].draw_masked)
  {
    SDL_Surface *surface        = menu.request.bitmap->surface;
    SDL_Surface *surface_masked = menu.request.bitmap->surface_masked;

    SDLBlitSurface(surface, surface_masked, 0, 0, xsize, ysize, 0, 0);
    SDL_SetColorKey(surface_masked, SET_TRANSPARENT_PIXEL,
		    SDL_MapRGB(surface_masked->format, 0x00, 0x00, 0x00));
  }

  SDLFreeBitmapTextures(menu.request.bitmap);
  SDLCreateBitmapTextures(menu.request.bitmap);

  ResetBitmapAlpha(menu.request.bitmap);

  // set envelope request run-time values
  menu.request.sx = sx;
  menu.request.sy = sy;
  menu.request.xsize = xsize;
  menu.request.ysize = ysize;
}

void DrawEnvelopeRequestToScreen(int drawing_target)
{
  if (global.use_envelope_request &&
      game.request_active &&
      drawing_target == DRAW_TO_SCREEN)
  {
    struct GraphicInfo *g = &graphic_info[IMG_BACKGROUND_REQUEST];

    SetBitmapAlphaNextBlit(menu.request.bitmap, g->alpha);

    if (g->draw_masked)
      BlitToScreenMasked(menu.request.bitmap, 0, 0, menu.request.xsize, menu.request.ysize,
			 menu.request.sx, menu.request.sy);
    else
      BlitToScreen(menu.request.bitmap, 0, 0, menu.request.xsize, menu.request.ysize,
		   menu.request.sx, menu.request.sy);
  }
}

static void setRequestBasePosition(int *x, int *y)
{
  int sx_base, sy_base;

  if (menu.request.x != -1)
    sx_base = menu.request.x;
  else if (menu.request.align == ALIGN_LEFT)
    sx_base = SX;
  else if (menu.request.align == ALIGN_RIGHT)
    sx_base = SX + SXSIZE;
  else
    sx_base = SX + SXSIZE / 2;

  if (menu.request.y != -1)
    sy_base = menu.request.y;
  else if (menu.request.valign == VALIGN_TOP)
    sy_base = SY;
  else if (menu.request.valign == VALIGN_BOTTOM)
    sy_base = SY + SYSIZE;
  else
    sy_base = SY + SYSIZE / 2;

  *x = sx_base;
  *y = sy_base;
}

static void setRequestPositionExt(int *x, int *y, int width, int height,
				  boolean add_border_size)
{
  int border_size = menu.request.border_size;
  int sx_base, sy_base;
  int sx, sy;

  setRequestBasePosition(&sx_base, &sy_base);

  if (menu.request.align == ALIGN_LEFT)
    sx = sx_base;
  else if (menu.request.align == ALIGN_RIGHT)
    sx = sx_base - width;
  else
    sx = sx_base - width  / 2;

  if (menu.request.valign == VALIGN_TOP)
    sy = sy_base;
  else if (menu.request.valign == VALIGN_BOTTOM)
    sy = sy_base - height;
  else
    sy = sy_base - height / 2;

  sx = MAX(0, MIN(sx, WIN_XSIZE - width));
  sy = MAX(0, MIN(sy, WIN_YSIZE - height));

  if (add_border_size)
  {
    sx += border_size;
    sy += border_size;
  }

  *x = sx;
  *y = sy;
}

static void setRequestPosition(int *x, int *y, boolean add_border_size)
{
  setRequestPositionExt(x, y, menu.request.width, menu.request.height, add_border_size);
}

static void DrawEnvelopeRequestText(int sx, int sy, char *text)
{
  char *text_final = text;
  char *text_door_style = NULL;
  int graphic = IMG_BACKGROUND_REQUEST;
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int mask_mode = (src_bitmap != NULL ? BLIT_MASKED : BLIT_ON_BACKGROUND);
  int font_nr = FONT_REQUEST;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int border_size = menu.request.border_size;
  int line_spacing = menu.request.line_spacing;
  int line_height = font_height + line_spacing;
  int max_text_width  = menu.request.width  - 2 * border_size;
  int max_text_height = menu.request.height - 2 * border_size;
  int line_length = max_text_width  / font_width;
  int max_lines   = max_text_height / line_height;
  int text_width = line_length * font_width;
  int sx_offset = border_size;
  int sy_offset = border_size;
  boolean use_narrow_font = FALSE;

  // force DOOR font inside door area
  SetFontStatus(GAME_MODE_PSEUDO_DOOR);

  if (menu.request.centered)
    sx_offset = (menu.request.width - text_width) / 2;

  if (menu.request.wrap_single_words && !menu.request.autowrap)
  {
    char *src_text_ptr, *dst_text_ptr;

    if (maxWordLengthInRequestString(text) > line_length)
      use_narrow_font = TRUE;

    text_door_style = checked_malloc(2 * strlen(text) + 1);

    src_text_ptr = text;
    dst_text_ptr = text_door_style;

    while (*src_text_ptr)
    {
      if (*src_text_ptr == ' ' ||
	  *src_text_ptr == '?' ||
	  *src_text_ptr == '!')
	*dst_text_ptr++ = '\n';

      if (*src_text_ptr != ' ')
	*dst_text_ptr++ = *src_text_ptr;

      src_text_ptr++;
    }

    *dst_text_ptr = '\0';

    text_final = text_door_style;
  }
  else if (strlen(text_final) * font_width > max_text_width)
  {
    // prevent multi-line request text overlapping with request buttons
    max_text_height = menu.request.button.confirm.y - sy_offset;

    struct WrappedTextInfo *wrapped_text =
      GetWrappedTextBuffer(text_final, font_nr, -1, -1, -1,
                           max_text_width, -1, max_text_height, line_spacing, mask_mode,
                           menu.request.autowrap, menu.request.centered, FALSE);

    if (wrapped_text != NULL && wrapped_text->total_height > wrapped_text->max_height)
      use_narrow_font = TRUE;

    FreeWrappedText(wrapped_text);
  }

  if (use_narrow_font)
  {
    font_nr = FONT_REQUEST_NARROW;
    font_width = getFontWidth(font_nr);
    line_length = max_text_width  / font_width;
  }

  DrawTextBuffer(sx + sx_offset, sy + sy_offset, text_final, font_nr,
		 line_length, -1, max_lines, -1, -1, -1, line_spacing, mask_mode,
		 menu.request.autowrap, menu.request.centered, FALSE);

  if (text_door_style)
    free(text_door_style);

  ResetFontStatus();
}

static void DrawEnvelopeRequest(char *text, unsigned int req_state)
{
  DrawBuffer *drawto_last = drawto;
  int graphic = IMG_BACKGROUND_REQUEST;
  int width = menu.request.width;
  int height = menu.request.height;
  int tile_size = MAX(menu.request.step_offset, 1);
  int x_steps = width  / tile_size;
  int y_steps = height / tile_size;
  int sx, sy;
  int x, y;

  setRequestPosition(&sx, &sy, FALSE);

  // draw complete envelope request to temporary bitmap
  drawto = bitmap_db_store_1;

  ClearRectangle(drawto, sx, sy, width, height);

  for (y = 0; y < y_steps; y++)
    for (x = 0; x < x_steps; x++)
      DrawEnvelopeBackgroundTiles(graphic, sx, sy,
				  x, y, x_steps, y_steps,
				  tile_size, tile_size);

  // write text for request
  DrawEnvelopeRequestText(sx, sy, text);

  MapToolButtons(req_state);

  // restore pointer to drawing buffer
  drawto = drawto_last;

  // prepare complete envelope request from temporary bitmap
  PrepareEnvelopeRequestToScreen(bitmap_db_store_1, sx, sy, width, height);
}

static void AnimateEnvelopeRequest(int anim_mode, int action)
{
  boolean game_ended = (game_status == GAME_MODE_PLAYING && checkGameEnded());
  int delay_value_normal = menu.request.step_delay;
  int delay_value_fast = delay_value_normal / 2;
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int delay_value = (ffwd_delay ? delay_value_fast : delay_value_normal);
  int anim_delay_value = MAX(1, (no_delay ? 0 : delay_value) / 2);
  DelayCounter anim_delay = { anim_delay_value };

  int tile_size = MAX(menu.request.step_offset, 1);
  int max_xsize = menu.request.width  / tile_size;
  int max_ysize = menu.request.height / tile_size;
  int max_xsize_inner = max_xsize - 2;
  int max_ysize_inner = max_ysize - 2;

  int xstart = (anim_mode & ANIM_VERTICAL ? max_xsize_inner : 0);
  int ystart = (anim_mode & ANIM_HORIZONTAL ? max_ysize_inner : 0);
  int xend = max_xsize_inner;
  int yend = (anim_mode != ANIM_DEFAULT ? max_ysize_inner : 0);
  int xstep = (xstart < xend ? 1 : 0);
  int ystep = (ystart < yend || xstep == 0 ? 1 : 0);
  int start = 0;
  int end = MAX(xend - xstart, yend - ystart);
  int i;

  if (setup.quick_doors)
  {
    xstart = xend;
    ystart = yend;
    end = 0;
  }

  for (i = start; i <= end; i++)
  {
    int last_frame = end;	// last frame of this "for" loop
    int x = xstart + i * xstep;
    int y = ystart + i * ystep;
    int xsize = (action == ACTION_CLOSING ? xend - (x - xstart) : x) + 2;
    int ysize = (action == ACTION_CLOSING ? yend - (y - ystart) : y) + 2;
    int xsize_size_left = (xsize - 1) * tile_size;
    int ysize_size_top  = (ysize - 1) * tile_size;
    int max_xsize_pos = (max_xsize - 1) * tile_size;
    int max_ysize_pos = (max_ysize - 1) * tile_size;
    int width  = xsize * tile_size;
    int height = ysize * tile_size;
    int src_x, src_y;
    int dst_x, dst_y;
    int xx, yy;

    if (game_ended)
      HandleGameActions();

    setRequestPosition(&src_x, &src_y, FALSE);
    setRequestPositionExt(&dst_x, &dst_y, width, height, FALSE);

    for (yy = 0; yy < 2; yy++)
    {
      for (xx = 0; xx < 2; xx++)
      {
	int src_xx = src_x + xx * max_xsize_pos;
	int src_yy = src_y + yy * max_ysize_pos;
	int dst_xx = dst_x + xx * xsize_size_left;
	int dst_yy = dst_y + yy * ysize_size_top;
	int xx_size = (xx ? tile_size : xsize_size_left);
	int yy_size = (yy ? tile_size : ysize_size_top);

	// draw partial (animated) envelope request to temporary bitmap
	BlitBitmap(bitmap_db_store_1, bitmap_db_store_2,
		   src_xx, src_yy, xx_size, yy_size, dst_xx, dst_yy);
      }
    }

    // prepare partial (animated) envelope request from temporary bitmap
    PrepareEnvelopeRequestToScreen(bitmap_db_store_2, dst_x, dst_y,
				   width, height);

    redraw_mask |= REDRAW_FIELD;

    BackToFront();

    SkipUntilDelayReached(&anim_delay, &i, last_frame);
  }

  ClearAutoRepeatKeyEvents();
}

static void ShowEnvelopeRequest(char *text, unsigned int req_state, int action)
{
  int graphic = IMG_BACKGROUND_REQUEST;
  int sound_opening = SND_REQUEST_OPENING;
  int sound_closing = SND_REQUEST_CLOSING;
  int anim_mode_1 = menu.request.anim_mode;		// (higher priority)
  int anim_mode_2 = graphic_info[graphic].anim_mode;	// (lower priority)
  int anim_mode = (anim_mode_1 != ANIM_DEFAULT ? anim_mode_1 : anim_mode_2);
  int main_anim_mode = (anim_mode == ANIM_NONE ? ANIM_VERTICAL|ANIM_HORIZONTAL:
			anim_mode == ANIM_DEFAULT ? ANIM_VERTICAL : anim_mode);

  game.envelope_active = TRUE;	// needed for RedrawPlayfield() events

  if (action == ACTION_OPENING)
  {
    PlayMenuSoundStereo(sound_opening, SOUND_MIDDLE);

    if (anim_mode == ANIM_DEFAULT)
      AnimateEnvelopeRequest(ANIM_DEFAULT, ACTION_OPENING);

    AnimateEnvelopeRequest(main_anim_mode, ACTION_OPENING);
  }
  else
  {
    PlayMenuSoundStereo(sound_closing, SOUND_MIDDLE);

    if (anim_mode != ANIM_NONE)
      AnimateEnvelopeRequest(main_anim_mode, ACTION_CLOSING);

    if (anim_mode == ANIM_DEFAULT)
      AnimateEnvelopeRequest(ANIM_DEFAULT, ACTION_CLOSING);
  }

  game.envelope_active = FALSE;
}

static void DrawPreviewElement(int dst_x, int dst_y, int element, int tilesize)
{
  if (IS_MM_WALL(element))
  {
    DrawSizedWall_MM(dst_x, dst_y, element, tilesize, el2preimg);
  }
  else
  {
    Bitmap *src_bitmap;
    int src_x, src_y;
    int graphic = el2preimg(element);

    getSizedGraphicSource(graphic, 0, tilesize, &src_bitmap, &src_x, &src_y);

    BlitBitmap(src_bitmap, drawto, src_x, src_y, tilesize, tilesize,
	       dst_x, dst_y);
  }
}

void DrawLevel(int draw_background_mask)
{
  int x, y;

  SetMainBackgroundImage(IMG_BACKGROUND_PLAYING);
  SetDrawBackgroundMask(draw_background_mask);

  ClearField();

  for (x = BX1; x <= BX2; x++)
    for (y = BY1; y <= BY2; y++)
      DrawScreenField(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawSizedLevel(int size_x, int size_y, int scroll_x, int scroll_y,
		    int tilesize)
{
  int x, y;

  for (x = 0; x < size_x; x++)
    for (y = 0; y < size_y; y++)
      DrawSizedElementOrWall(x, y, scroll_x, scroll_y, tilesize);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x, y;

  for (x = 0; x < size_x; x++)
    for (y = 0; y < size_y; y++)
      DrawMiniElementOrWall(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

static int getPreviewLevelWidth(void)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    return (level.native_bd_level->cave->x2 - level.native_bd_level->cave->x1 + 1);

  return lev_fieldx;
}

static int getPreviewLevelHeight(void)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    return (level.native_bd_level->cave->y2 - level.native_bd_level->cave->y1 + 1);

  return lev_fieldy;
}

static void DrawPreviewLevelPlayfield(int from_x, int from_y)
{
  boolean show_level_border = (BorderElement != EL_EMPTY);
  int level_xsize = getPreviewLevelWidth()  + (show_level_border ? 2 : 0);
  int level_ysize = getPreviewLevelHeight() + (show_level_border ? 2 : 0);
  int tile_size = preview.tile_size;
  int preview_width  = preview.xsize * tile_size;
  int preview_height = preview.ysize * tile_size;
  int real_preview_xsize = MIN(level_xsize, preview.xsize);
  int real_preview_ysize = MIN(level_ysize, preview.ysize);
  int real_preview_width  = real_preview_xsize * tile_size;
  int real_preview_height = real_preview_ysize * tile_size;
  int dst_x = SX + ALIGNED_XPOS(preview.x, preview_width, preview.align);
  int dst_y = SY + ALIGNED_YPOS(preview.y, preview_height, preview.valign);
  int x, y;

  if (!IN_GFX_FIELD_FULL(dst_x, dst_y + preview_height - 1))
    return;

  DrawBackground(dst_x, dst_y, preview_width, preview_height);

  dst_x += (preview_width  - real_preview_width)  / 2;
  dst_y += (preview_height - real_preview_height) / 2;

  for (x = 0; x < real_preview_xsize; x++)
  {
    for (y = 0; y < real_preview_ysize; y++)
    {
      int lx = from_x + x + (show_level_border ? -1 : 0);
      int ly = from_y + y + (show_level_border ? -1 : 0);
      int element = (IN_LEV_FIELD(lx, ly) ? level.field[lx][ly] :
		     getBorderElement(lx, ly));

      DrawPreviewElement(dst_x + x * tile_size, dst_y + y * tile_size,
			 element, tile_size);
    }
  }

  redraw_mask |= REDRAW_FIELD;
}

#define MICROLABEL_EMPTY		0
#define MICROLABEL_LEVEL_NAME		1
#define MICROLABEL_LEVEL_AUTHOR_HEAD	2
#define MICROLABEL_LEVEL_AUTHOR		3
#define MICROLABEL_IMPORTED_FROM_HEAD	4
#define MICROLABEL_IMPORTED_FROM	5
#define MICROLABEL_IMPORTED_BY_HEAD	6
#define MICROLABEL_IMPORTED_BY		7

static int getMaxTextLength(struct TextPosInfo *pos, int font_nr)
{
  int max_text_width = SXSIZE;
  int font_width = getFontWidth(font_nr);

  if (pos->align == ALIGN_CENTER)
    max_text_width = (pos->x < SXSIZE / 2 ? pos->x * 2 : (SXSIZE - pos->x) * 2);
  else if (pos->align == ALIGN_RIGHT)
    max_text_width = pos->x;
  else
    max_text_width = SXSIZE - pos->x;

  return max_text_width / font_width;
}

static void DrawPreviewLevelLabelExt(int mode, struct TextPosInfo *pos)
{
  char label_text[MAX_OUTPUT_LINESIZE + 1];
  int max_len_label_text;
  int font_nr = pos->font;
  int i;

  if (!IN_GFX_FIELD_FULL(pos->x, pos->y + getFontHeight(pos->font)))
    return;

  if (mode == MICROLABEL_LEVEL_AUTHOR_HEAD ||
      mode == MICROLABEL_IMPORTED_FROM_HEAD ||
      mode == MICROLABEL_IMPORTED_BY_HEAD)
    font_nr = pos->font_alt;

  max_len_label_text = getMaxTextLength(pos, font_nr);

  if (pos->size != -1)
    max_len_label_text = pos->size;

  for (i = 0; i < max_len_label_text; i++)
    label_text[i] = ' ';
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
    DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);

  strncpy(label_text,
	  (mode == MICROLABEL_LEVEL_NAME ? level.name :
	   mode == MICROLABEL_LEVEL_AUTHOR_HEAD ? "created by" :
	   mode == MICROLABEL_LEVEL_AUTHOR ? level.author :
	   mode == MICROLABEL_IMPORTED_FROM_HEAD ? "imported from" :
	   mode == MICROLABEL_IMPORTED_FROM ? leveldir_current->imported_from :
	   mode == MICROLABEL_IMPORTED_BY_HEAD ? "imported by" :
	   mode == MICROLABEL_IMPORTED_BY ? leveldir_current->imported_by :""),
	  max_len_label_text);
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
    DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawPreviewLevelLabel(int mode)
{
  DrawPreviewLevelLabelExt(mode, &menu.main.text.level_info_2);
}

static void DrawPreviewLevelInfo(int mode)
{
  if (mode == MICROLABEL_LEVEL_NAME)
    DrawPreviewLevelLabelExt(mode, &menu.main.text.level_name);
  else if (mode == MICROLABEL_LEVEL_AUTHOR)
    DrawPreviewLevelLabelExt(mode, &menu.main.text.level_author);
}

static void DrawPreviewLevelExt(boolean restart)
{
  static DelayCounter scroll_delay = { 0 };
  static DelayCounter label_delay = { 0 };
  static int from_x, from_y, scroll_direction;
  static int label_state, label_counter;
  boolean show_level_border = (BorderElement != EL_EMPTY);
  int level_xsize = lev_fieldx + (show_level_border ? 2 : 0);
  int level_ysize = lev_fieldy + (show_level_border ? 2 : 0);

  scroll_delay.value = preview.step_delay;
  label_delay.value = MICROLEVEL_LABEL_DELAY;

  if (restart)
  {
    from_x = 0;
    from_y = 0;

    if (preview.anim_mode == ANIM_CENTERED)
    {
      if (level_xsize > preview.xsize)
	from_x = (level_xsize - preview.xsize) / 2;
      if (level_ysize > preview.ysize)
	from_y = (level_ysize - preview.ysize) / 2;
    }

    from_x += preview.xoffset;
    from_y += preview.yoffset;

    scroll_direction = MV_RIGHT;
    label_state = 1;
    label_counter = 0;

    DrawPreviewLevelPlayfield(from_x, from_y);
    DrawPreviewLevelLabel(label_state);

    DrawPreviewLevelInfo(MICROLABEL_LEVEL_NAME);
    DrawPreviewLevelInfo(MICROLABEL_LEVEL_AUTHOR);

    // initialize delay counters
    ResetDelayCounter(&scroll_delay);
    ResetDelayCounter(&label_delay);

    if (leveldir_current->name)
    {
      struct TextPosInfo *pos = &menu.main.text.level_info_1;
      char label_text[MAX_OUTPUT_LINESIZE + 1];
      int font_nr = pos->font;
      int max_len_label_text = getMaxTextLength(pos, font_nr);

      if (pos->size != -1)
	max_len_label_text = pos->size;

      strncpy(label_text, leveldir_current->name, max_len_label_text);
      label_text[max_len_label_text] = '\0';

      if (IN_GFX_FIELD_FULL(pos->x, pos->y + getFontHeight(pos->font)))
	DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);
    }

    return;
  }

  // scroll preview level, if needed
  if (preview.anim_mode != ANIM_NONE &&
      (level_xsize > preview.xsize || level_ysize > preview.ysize) &&
      DelayReached(&scroll_delay))
  {
    switch (scroll_direction)
    {
      case MV_LEFT:
	if (from_x > 0)
	{
	  from_x -= preview.step_offset;
	  from_x = (from_x < 0 ? 0 : from_x);
	}
	else
	  scroll_direction = MV_UP;
	break;

      case MV_RIGHT:
	if (from_x < level_xsize - preview.xsize)
	{
	  from_x += preview.step_offset;
	  from_x = (from_x > level_xsize - preview.xsize ?
		    level_xsize - preview.xsize : from_x);
	}
	else
	  scroll_direction = MV_DOWN;
	break;

      case MV_UP:
	if (from_y > 0)
	{
	  from_y -= preview.step_offset;
	  from_y = (from_y < 0 ? 0 : from_y);
	}
	else
	  scroll_direction = MV_RIGHT;
	break;

      case MV_DOWN:
	if (from_y < level_ysize - preview.ysize)
	{
	  from_y += preview.step_offset;
	  from_y = (from_y > level_ysize - preview.ysize ?
		    level_ysize - preview.ysize : from_y);
	}
	else
	  scroll_direction = MV_LEFT;
	break;

      default:
	break;
    }

    DrawPreviewLevelPlayfield(from_x, from_y);
  }

  // !!! THIS ALL SUCKS -- SHOULD BE CLEANLY REWRITTEN !!!
  // redraw micro level label, if needed
  if (!strEqual(level.name, NAMELESS_LEVEL_NAME) &&
      !strEqual(level.author, ANONYMOUS_NAME) &&
      !strEqual(level.author, leveldir_current->name) &&
      DelayReached(&label_delay))
  {
    int max_label_counter = 23;

    if (leveldir_current->imported_from != NULL &&
	strlen(leveldir_current->imported_from) > 0)
      max_label_counter += 14;
    if (leveldir_current->imported_by != NULL &&
	strlen(leveldir_current->imported_by) > 0)
      max_label_counter += 14;

    label_counter = (label_counter + 1) % max_label_counter;
    label_state = (label_counter >= 0 && label_counter <= 7 ?
		   MICROLABEL_LEVEL_NAME :
		   label_counter >= 9 && label_counter <= 12 ?
		   MICROLABEL_LEVEL_AUTHOR_HEAD :
		   label_counter >= 14 && label_counter <= 21 ?
		   MICROLABEL_LEVEL_AUTHOR :
		   label_counter >= 23 && label_counter <= 26 ?
		   MICROLABEL_IMPORTED_FROM_HEAD :
		   label_counter >= 28 && label_counter <= 35 ?
		   MICROLABEL_IMPORTED_FROM :
		   label_counter >= 37 && label_counter <= 40 ?
		   MICROLABEL_IMPORTED_BY_HEAD :
		   label_counter >= 42 && label_counter <= 49 ?
		   MICROLABEL_IMPORTED_BY : MICROLABEL_EMPTY);

    if (leveldir_current->imported_from == NULL &&
	(label_state == MICROLABEL_IMPORTED_FROM_HEAD ||
	 label_state == MICROLABEL_IMPORTED_FROM))
      label_state = (label_state == MICROLABEL_IMPORTED_FROM_HEAD ?
		     MICROLABEL_IMPORTED_BY_HEAD : MICROLABEL_IMPORTED_BY);

    DrawPreviewLevelLabel(label_state);
  }
}

void DrawPreviewPlayers(void)
{
  if (game_status != GAME_MODE_MAIN)
    return;

  // do not draw preview players if level preview redefined, but players aren't
  if (preview.redefined && !menu.main.preview_players.redefined)
    return;

  boolean player_found[MAX_PLAYERS];
  int num_players = 0;
  int i, x, y;

  for (i = 0; i < MAX_PLAYERS; i++)
    player_found[i] = FALSE;

  // check which players can be found in the level (simple approach)
  for (x = 0; x < lev_fieldx; x++)
  {
    for (y = 0; y < lev_fieldy; y++)
    {
      int element = level.field[x][y];

      if (IS_PLAYER_ELEMENT(element))
      {
	int player_nr = GET_PLAYER_NR(element);

	player_nr = MIN(MAX(0, player_nr), MAX_PLAYERS - 1);

	if (!player_found[player_nr])
	  num_players++;

	player_found[player_nr] = TRUE;
      }
    }
  }

  struct TextPosInfo *pos = &menu.main.preview_players;
  int tile_size = pos->tile_size;
  int border_size = pos->border_size;
  int player_xoffset_raw = (pos->vertical ? 0 : tile_size + border_size);
  int player_yoffset_raw = (pos->vertical ? tile_size + border_size : 0);
  int player_xoffset = (pos->xoffset != -1 ? pos->xoffset : player_xoffset_raw);
  int player_yoffset = (pos->yoffset != -1 ? pos->yoffset : player_yoffset_raw);
  int max_players_width  = (MAX_PLAYERS - 1) * player_xoffset + tile_size;
  int max_players_height = (MAX_PLAYERS - 1) * player_yoffset + tile_size;
  int all_players_width  = (num_players - 1) * player_xoffset + tile_size;
  int all_players_height = (num_players - 1) * player_yoffset + tile_size;
  int max_xpos = SX + ALIGNED_XPOS(pos->x, max_players_width,  pos->align);
  int max_ypos = SY + ALIGNED_YPOS(pos->y, max_players_height, pos->valign);
  int xpos = SX + ALIGNED_XPOS(pos->x, all_players_width,  pos->align);
  int ypos = SY + ALIGNED_YPOS(pos->y, all_players_height, pos->valign);

  // clear area in which the players will be drawn
  ClearRectangleOnBackground(drawto, max_xpos, max_ypos,
			     max_players_width, max_players_height);

  if (!network.enabled && !setup.team_mode)
    return;

  // only draw players if level is suited for team mode
  if (num_players < 2)
    return;

  // draw all players that were found in the level
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (player_found[i])
    {
      int graphic = el2img(EL_PLAYER_1 + i);

      DrawSizedGraphicThruMaskExt(drawto, xpos, ypos, graphic, 0, tile_size);

      xpos += player_xoffset;
      ypos += player_yoffset;
    }
  }
}

void DrawPreviewLevelInitial(void)
{
  DrawPreviewLevelExt(TRUE);
  DrawPreviewPlayers();
}

void DrawPreviewLevelAnimation(void)
{
  DrawPreviewLevelExt(FALSE);
}

static void DrawNetworkPlayer(int x, int y, int player_nr, int tile_size,
			      int border_size, int font_nr)
{
  int graphic = el2img(EL_PLAYER_1 + player_nr);
  int font_height = getFontHeight(font_nr);
  int player_height = MAX(tile_size, font_height);
  int xoffset_text = tile_size + border_size;
  int yoffset_text    = (player_height - font_height) / 2;
  int yoffset_graphic = (player_height - tile_size) / 2;
  char *player_name = getNetworkPlayerName(player_nr + 1);

  DrawSizedGraphicThruMaskExt(drawto, x, y + yoffset_graphic, graphic, 0,
			      tile_size);
  DrawText(x + xoffset_text, y + yoffset_text, player_name, font_nr);
}

static void DrawNetworkPlayersExt(boolean force)
{
  if (game_status != GAME_MODE_MAIN)
    return;

  if (!network.connected && !force)
    return;

  // do not draw network players if level preview redefined, but players aren't
  if (preview.redefined && !menu.main.network_players.redefined)
    return;

  int num_players = 0;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].connected_network)
      num_players++;

  struct TextPosInfo *pos = &menu.main.network_players;
  int tile_size = pos->tile_size;
  int border_size = pos->border_size;
  int xoffset_text = tile_size + border_size;
  int font_nr = pos->font;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int player_height = MAX(tile_size, font_height);
  int player_yoffset = player_height + border_size;
  int max_players_width = xoffset_text + MAX_PLAYER_NAME_LEN * font_width;
  int max_players_height = MAX_PLAYERS * player_yoffset - border_size;
  int all_players_height = num_players * player_yoffset - border_size;
  int max_xpos = SX + ALIGNED_XPOS(pos->x, max_players_width,  pos->align);
  int max_ypos = SY + ALIGNED_YPOS(pos->y, max_players_height, pos->valign);
  int ypos = SY + ALIGNED_YPOS(pos->y, all_players_height, pos->valign);

  ClearRectangleOnBackground(drawto, max_xpos, max_ypos,
			     max_players_width, max_players_height);

  // first draw local network player ...
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (stored_player[i].connected_network &&
	stored_player[i].connected_locally)
    {
      char *player_name = getNetworkPlayerName(i + 1);
      int player_width = xoffset_text + getTextWidth(player_name, font_nr);
      int xpos = SX + ALIGNED_XPOS(pos->x, player_width,  pos->align);

      DrawNetworkPlayer(xpos, ypos, i, tile_size, border_size, font_nr);

      ypos += player_yoffset;
    }
  }

  // ... then draw all other network players
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (stored_player[i].connected_network &&
	!stored_player[i].connected_locally)
    {
      char *player_name = getNetworkPlayerName(i + 1);
      int player_width = xoffset_text + getTextWidth(player_name, font_nr);
      int xpos = SX + ALIGNED_XPOS(pos->x, player_width,  pos->align);

      DrawNetworkPlayer(xpos, ypos, i, tile_size, border_size, font_nr);

      ypos += player_yoffset;
    }
  }
}

void DrawNetworkPlayers(void)
{
  DrawNetworkPlayersExt(FALSE);
}

void ClearNetworkPlayers(void)
{
  DrawNetworkPlayersExt(TRUE);
}

static void DrawGraphicAnimationExt(DrawBuffer *dst_bitmap, int x, int y,
				    int graphic, int lx, int ly,
				    int mask_mode)
{
  int frame = getGraphicAnimationFrameXY(graphic, lx, ly);

  if (mask_mode == USE_MASKING)
    DrawGraphicThruMaskExt(dst_bitmap, x, y, graphic, frame);
  else
    DrawGraphicExt(dst_bitmap, x, y, graphic, frame);
}

void DrawFixedGraphicAnimationExt(DrawBuffer *dst_bitmap, int x, int y,
				  int graphic, int sync_frame, int mask_mode)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  if (mask_mode == USE_MASKING)
    DrawFixedGraphicThruMaskExt(dst_bitmap, x, y, graphic, frame);
  else
    DrawFixedGraphicExt(dst_bitmap, x, y, graphic, frame);
}

void DrawSizedGraphicAnimationExt(DrawBuffer *dst_bitmap, int x, int y,
				  int graphic, int sync_frame, int tilesize,
				  int mask_mode)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  if (mask_mode == USE_MASKING)
    DrawSizedGraphicThruMaskExt(dst_bitmap, x, y, graphic, frame, tilesize);
  else
    DrawSizedGraphicExt(dst_bitmap, x, y, graphic, frame, tilesize);
}

static void DrawGraphicAnimation(int x, int y, int graphic)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int mask_mode = NO_MASKING;

  if (!IN_SCR_FIELD(x, y))
    return;

  if (game.use_masked_elements)
  {
    if (Tile[lx][ly] != EL_EMPTY)
    {
      DrawScreenElementExt(x, y, 0, 0, EL_EMPTY, NO_CUTTING, NO_MASKING);

      mask_mode = USE_MASKING;
    }
  }

  DrawGraphicAnimationExt(drawto_field, FX + x * TILEX_VAR, FY + y * TILEY_VAR,
			  graphic, lx, ly, mask_mode);

  MarkTileDirty(x, y);
}

void DrawFixedGraphicAnimation(int x, int y, int graphic)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int mask_mode = NO_MASKING;

  if (!IN_SCR_FIELD(x, y))
    return;

  if (game.use_masked_elements)
  {
    if (Tile[lx][ly] != EL_EMPTY)
    {
      DrawScreenElementExt(x, y, 0, 0, EL_EMPTY, NO_CUTTING, NO_MASKING);

      mask_mode = USE_MASKING;
    }
  }

  DrawGraphicAnimationExt(drawto_field, FX + x * TILEX, FY + y * TILEY,
			  graphic, lx, ly, mask_mode);

  MarkTileDirty(x, y);
}

void DrawLevelGraphicAnimation(int x, int y, int graphic)
{
  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
}

void DrawLevelElementAnimation(int x, int y, int element)
{
  int graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
}

void DrawLevelGraphicAnimationIfNeeded(int x, int y, int graphic)
{
  int sx = SCREENX(x), sy = SCREENY(y);

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  if (Tile[x][y] == EL_EMPTY)
    graphic = el2img(GfxElementEmpty[x][y]);

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  if (ANIM_MODE(graphic) & (ANIM_TILED | ANIM_RANDOM_STATIC))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

#if 1
  if (GFX_CRUMBLED(TILE_GFX_ELEMENT(x, y)))
    DrawLevelFieldCrumbled(x, y);
#else
  if (GFX_CRUMBLED(Tile[x][y]))
    DrawLevelFieldCrumbled(x, y);
#endif
}

void DrawLevelElementAnimationIfNeeded(int x, int y, int element)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic;

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

  if (GFX_CRUMBLED(element))
    DrawLevelFieldCrumbled(x, y);
}

static int getPlayerGraphic(struct PlayerInfo *player, int move_dir)
{
  if (player->use_murphy)
  {
    // this works only because currently only one player can be "murphy" ...
    static int last_horizontal_dir = MV_LEFT;
    int graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, move_dir);

    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      last_horizontal_dir = move_dir;

    if (graphic == IMG_SP_MURPHY)	// undefined => use special graphic
    {
      int direction = (player->is_snapping ? move_dir : last_horizontal_dir);

      graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, direction);
    }

    return graphic;
  }
  else
    return el_act_dir2img(player->artwork_element, player->GfxAction, move_dir);
}

static boolean equalGraphics(int graphic1, int graphic2)
{
  struct GraphicInfo *g1 = &graphic_info[graphic1];
  struct GraphicInfo *g2 = &graphic_info[graphic2];

  return (g1->bitmap      == g2->bitmap &&
	  g1->src_x       == g2->src_x &&
	  g1->src_y       == g2->src_y &&
	  g1->anim_frames == g2->anim_frames &&
	  g1->anim_delay  == g2->anim_delay &&
	  g1->anim_mode   == g2->anim_mode);
}

#define DRAW_PLAYER_OVER_PUSHED_ELEMENT	1

enum
{
  DRAW_PLAYER_STAGE_INIT = 0,
  DRAW_PLAYER_STAGE_LAST_FIELD,
  DRAW_PLAYER_STAGE_FIELD_UNDER_PLAYER,
#if DRAW_PLAYER_OVER_PUSHED_ELEMENT
  DRAW_PLAYER_STAGE_ELEMENT_PUSHED,
  DRAW_PLAYER_STAGE_PLAYER,
#else
  DRAW_PLAYER_STAGE_PLAYER,
  DRAW_PLAYER_STAGE_ELEMENT_PUSHED,
#endif
  DRAW_PLAYER_STAGE_ELEMENT_OVER_PLAYER,
  DRAW_PLAYER_STAGE_FIELD_OVER_PLAYER,

  NUM_DRAW_PLAYER_STAGES
};

static void DrawPlayerExt(struct PlayerInfo *player, int drawing_stage)
{
  static int static_last_player_graphic[MAX_PLAYERS];
  static int static_last_player_frame[MAX_PLAYERS];
  static boolean static_player_is_opaque[MAX_PLAYERS];
  static boolean draw_player[MAX_PLAYERS];
  int pnr = player->index_nr;

  if (drawing_stage == DRAW_PLAYER_STAGE_INIT)
  {
    static_last_player_graphic[pnr] = getPlayerGraphic(player, player->MovDir);
    static_last_player_frame[pnr] = player->Frame;
    static_player_is_opaque[pnr] = FALSE;

    draw_player[pnr] = TRUE;
  }

  if (!draw_player[pnr])
    return;

#if DEBUG
  if (!IN_LEV_FIELD(player->jx, player->jy))
  {
    Debug("draw:DrawPlayerExt", "x = %d, y = %d", player->jx, player->jy);
    Debug("draw:DrawPlayerExt", "This should never happen!");

    draw_player[pnr] = FALSE;

    return;
  }
#endif

  int last_player_graphic  = static_last_player_graphic[pnr];
  int last_player_frame    = static_last_player_frame[pnr];
  boolean player_is_opaque = static_player_is_opaque[pnr];

  int jx = player->jx;
  int jy = player->jy;
  int move_dir = (player->is_waiting ? player->dir_waiting : player->MovDir);
  int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? +1 : 0);
  int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? +1 : 0);
  int last_jx = (player->is_moving ? jx - dx : jx);
  int last_jy = (player->is_moving ? jy - dy : jy);
  int next_jx = jx + dx;
  int next_jy = jy + dy;
  boolean player_is_moving = (player->MovPos != 0 ? TRUE : FALSE);
  int sx = SCREENX(jx);
  int sy = SCREENY(jy);
  int sxx = (move_dir == MV_LEFT || move_dir == MV_RIGHT ? player->GfxPos : 0);
  int syy = (move_dir == MV_UP   || move_dir == MV_DOWN  ? player->GfxPos : 0);
  int element = Tile[jx][jy];
  int last_element = Tile[last_jx][last_jy];
  int action = (player->is_pushing    ? ACTION_PUSHING         :
		player->is_digging    ? ACTION_DIGGING         :
		player->is_collecting ? ACTION_COLLECTING      :
		player->is_moving     ? ACTION_MOVING          :
		player->is_snapping   ? ACTION_SNAPPING        :
		player->is_dropping   ? ACTION_DROPPING        :
		player->is_waiting    ? player->action_waiting :
		ACTION_DEFAULT);

  if (drawing_stage == DRAW_PLAYER_STAGE_INIT)
  {
    // ------------------------------------------------------------------------
    // initialize drawing the player
    // ------------------------------------------------------------------------

    draw_player[pnr] = FALSE;

    // GfxElement[][] is set to the element the player is digging or collecting;
    // remove also for off-screen player if the player is not moving anymore
    if (IN_LEV_FIELD(jx, jy) && !player_is_moving)
      GfxElement[jx][jy] = EL_UNDEFINED;

    if (!player->active || !IN_SCR_FIELD(SCREENX(last_jx), SCREENY(last_jy)))
      return;

    if (element == EL_EXPLOSION)
      return;

    InitPlayerGfxAnimation(player, action, move_dir);

    draw_player[pnr] = TRUE;
  }
  else if (drawing_stage == DRAW_PLAYER_STAGE_LAST_FIELD)
  {
    // ------------------------------------------------------------------------
    // draw things in the field the player is leaving, if needed
    // ------------------------------------------------------------------------

    if (!IN_SCR_FIELD(sx, sy))
      draw_player[pnr] = FALSE;

    if (!player->is_moving)
      return;

    if (Back[last_jx][last_jy] && IS_DRAWABLE(last_element))
    {
      DrawLevelElement(last_jx, last_jy, Back[last_jx][last_jy]);

      if (last_element == EL_DYNAMITE_ACTIVE ||
	  last_element == EL_EM_DYNAMITE_ACTIVE ||
	  last_element == EL_SP_DISK_RED_ACTIVE)
	DrawDynamite(last_jx, last_jy);
      else
	DrawLevelFieldThruMask(last_jx, last_jy);
    }
    else if (last_element == EL_DYNAMITE_ACTIVE ||
	     last_element == EL_EM_DYNAMITE_ACTIVE ||
	     last_element == EL_SP_DISK_RED_ACTIVE)
      DrawDynamite(last_jx, last_jy);
    else
      DrawLevelField(last_jx, last_jy);
  }
  else if (drawing_stage == DRAW_PLAYER_STAGE_FIELD_UNDER_PLAYER)
  {
    // ------------------------------------------------------------------------
    // draw things behind the player, if needed
    // ------------------------------------------------------------------------

    if (Back[jx][jy])
    {
      DrawLevelElement(jx, jy, Back[jx][jy]);

      return;
    }

    if (IS_ACTIVE_BOMB(element))
    {
      DrawLevelElement(jx, jy, EL_EMPTY);

      return;
    }

    if (player_is_moving && GfxElement[jx][jy] != EL_UNDEFINED)
    {
      int old_element = GfxElement[jx][jy];
      int old_graphic = el_act_dir2img(old_element, action, move_dir);
      int frame = getGraphicAnimationFrame(old_graphic, player->StepFrame);

      if (GFX_CRUMBLED(old_element))
	DrawLevelFieldCrumbledDigging(jx, jy, move_dir, player->StepFrame);
      else
	DrawScreenGraphic(sx, sy, old_graphic, frame);

      if (graphic_info[old_graphic].anim_mode & ANIM_OPAQUE_PLAYER)
	static_player_is_opaque[pnr] = TRUE;
    }
    else
    {
      GfxElement[jx][jy] = EL_UNDEFINED;

      // make sure that pushed elements are drawn with correct frame rate
      int graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);

      if (player->is_pushing && player->is_moving && !IS_ANIM_MODE_CE(graphic))
	GfxFrame[jx][jy] = player->StepFrame;

      DrawLevelField(jx, jy);
    }
  }
  else if (drawing_stage == DRAW_PLAYER_STAGE_ELEMENT_PUSHED)
  {
    // ------------------------------------------------------------------------
    // draw things the player is pushing, if needed
    // ------------------------------------------------------------------------

    if (!player->is_pushing || !player->is_moving)
      return;

    if (Tile[next_jx][next_jy] == EL_EXPLOSION)
      return;

    int gfx_frame = GfxFrame[jx][jy];

    if (!IS_MOVING(jx, jy))		// push movement already finished
    {
      element = Tile[next_jx][next_jy];
      gfx_frame = GfxFrame[next_jx][next_jy];
    }

    int graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);
    int sync_frame = (IS_ANIM_MODE_CE(graphic) ? gfx_frame : player->StepFrame);
    int frame = getGraphicAnimationFrame(graphic, sync_frame);

    // draw background element under pushed element (like the Sokoban field)
    if (game.use_masked_pushing && IS_MOVING(jx, jy))
    {
      // this allows transparent pushing animation over non-black background

      if (Back[jx][jy])
	DrawLevelElement(jx, jy, Back[jx][jy]);
      else
	DrawLevelElement(jx, jy, EL_EMPTY);
    }

    if (Back[next_jx][next_jy])
      DrawLevelElement(next_jx, next_jy, Back[next_jx][next_jy]);
    else
      DrawLevelElement(next_jx, next_jy, EL_EMPTY);

    int px = SCREENX(jx), py = SCREENY(jy);
    int pxx = (TILEX - ABS(sxx)) * dx;
    int pyy = (TILEY - ABS(syy)) * dy;

#if 1
    // do not draw (EM style) pushing animation when pushing is finished
    // (two-tile animations usually do not contain start and end frame)
    if (graphic_info[graphic].double_movement && !IS_MOVING(jx, jy))
      DrawLevelElement(next_jx, next_jy, Tile[next_jx][next_jy]);
    else
      DrawGraphicShiftedThruMask(px, py, pxx, pyy, graphic, frame, NO_CUTTING);
#else
    // masked drawing is needed for EMC style (double) movement graphics
    // !!! (ONLY WHEN DRAWING PUSHED ELEMENT OVER THE PLAYER) !!!
    DrawGraphicShiftedThruMask(px, py, pxx, pyy, graphic, frame, NO_CUTTING);
#endif
  }
  else if (drawing_stage == DRAW_PLAYER_STAGE_PLAYER)
  {
    // ------------------------------------------------------------------------
    // draw player himself
    // ------------------------------------------------------------------------

    int graphic = getPlayerGraphic(player, move_dir);

    // in the case of changed player action or direction, prevent the current
    // animation frame from being restarted for identical animations
    if (player->Frame == 0 && equalGraphics(graphic, last_player_graphic))
      player->Frame = last_player_frame;

    int frame = getGraphicAnimationFrame(graphic, player->Frame);

    if (player_is_opaque)
      DrawGraphicShifted(sx,sy, sxx,syy, graphic, frame, NO_CUTTING, NO_MASKING);
    else
      DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);

    if (SHIELD_ON(player))
    {
      graphic = (player->shield_deadly_time_left ? IMG_SHIELD_DEADLY_ACTIVE :
		 IMG_SHIELD_NORMAL_ACTIVE);
      frame = getGraphicAnimationFrame(graphic, -1);

      DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);
    }
  }
  else if (drawing_stage == DRAW_PLAYER_STAGE_ELEMENT_OVER_PLAYER)
  {
    // ------------------------------------------------------------------------
    // draw things in front of player (active dynamite or dynabombs)
    // ------------------------------------------------------------------------

    if (IS_ACTIVE_BOMB(element))
    {
      int graphic = el2img(element);
      int frame = getGraphicAnimationFrameXY(graphic, jx, jy);

      if (game.emulation == EMU_SUPAPLEX)
	DrawGraphic(sx, sy, IMG_SP_DISK_RED, frame);
      else
	DrawGraphicThruMask(sx, sy, graphic, frame);
    }

    if (player_is_moving && last_element == EL_EXPLOSION)
    {
      int element = (GfxElement[last_jx][last_jy] != EL_UNDEFINED ?
		     GfxElement[last_jx][last_jy] :  EL_EMPTY);
      int graphic = el_act2img(element, ACTION_EXPLODING);
      int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);
      int phase = ExplodePhase[last_jx][last_jy] - 1;
      int frame = getGraphicAnimationFrame(graphic, phase - delay);

      if (phase >= delay)
	DrawGraphicThruMask(SCREENX(last_jx), SCREENY(last_jy), graphic, frame);
    }
  }
  else if (drawing_stage == DRAW_PLAYER_STAGE_FIELD_OVER_PLAYER)
  {
    // ------------------------------------------------------------------------
    // draw elements the player is just walking/passing through/under
    // ------------------------------------------------------------------------

    if (player_is_moving)
    {
      // handle the field the player is leaving ...
      if (IS_ACCESSIBLE_INSIDE(last_element))
	DrawLevelField(last_jx, last_jy);
      else if (IS_ACCESSIBLE_UNDER(last_element))
	DrawLevelFieldThruMask(last_jx, last_jy);
    }

    // do not redraw accessible elements if the player is just pushing them
    if (!player_is_moving || !player->is_pushing)
    {
      // ... and the field the player is entering
      if (IS_ACCESSIBLE_INSIDE(element))
	DrawLevelField(jx, jy);
      else if (IS_ACCESSIBLE_UNDER(element))
	DrawLevelFieldThruMask(jx, jy);
    }

    MarkTileDirty(sx, sy);
  }
}

void DrawPlayer(struct PlayerInfo *player)
{
  int i;

  for (i = 0; i < NUM_DRAW_PLAYER_STAGES; i++)
    DrawPlayerExt(player, i);
}

void DrawAllPlayers(void)
{
  int i, j;

  for (i = 0; i < NUM_DRAW_PLAYER_STAGES; i++)
    for (j = 0; j < MAX_PLAYERS; j++)
      if (stored_player[j].active)
	DrawPlayerExt(&stored_player[j], i);
}

void DrawPlayerField(int x, int y)
{
  if (!IS_PLAYER(x, y))
    return;

  DrawPlayer(PLAYERINFO(x, y));
}

// ----------------------------------------------------------------------------

void WaitForEventToContinue(void)
{
  boolean first_wait = TRUE;
  boolean still_wait = TRUE;

  if (program.headless)
    return;

  // simulate releasing mouse button over last gadget, if still pressed
  if (button_status)
    HandleGadgets(-1, -1, 0);

  button_status = MB_RELEASED;

  ClearEventQueue();
  ClearPlayerAction();

  while (still_wait)
  {
    Event event;

    if (NextValidEvent(&event))
    {
      switch (event.type)
      {
	case EVENT_BUTTONPRESS:
        case EVENT_FINGERPRESS:
	  first_wait = FALSE;
	  break;

        case EVENT_BUTTONRELEASE:
        case EVENT_FINGERRELEASE:
	  still_wait = first_wait;
	  break;

	case EVENT_KEYPRESS:
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_JOYBUTTONDOWN:
	  still_wait = FALSE;
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }
    else if (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      still_wait = FALSE;
    }

    if (!PendingEvent())
      BackToFront();
  }
}

static int RequestHandleEvents(unsigned int req_state, int draw_buffer_game)
{
  boolean game_ended = (game_status == GAME_MODE_PLAYING && checkGameEnded());
  int draw_buffer_last = GetDrawtoField();
  int width  = menu.request.width;
  int height = menu.request.height;
  int sx, sy;
  int result;

  setRequestPosition(&sx, &sy, FALSE);

  button_status = MB_RELEASED;

  request_gadget_id = -1;
  result = -1;

  while (result < 0)
  {
    if (game_ended)
    {
      SetDrawtoField(draw_buffer_game);

      HandleGameActions();

      SetDrawtoField(DRAW_TO_BACKBUFFER);
    }

    if (PendingEvent())
    {
      Event event;

      while (NextValidEvent(&event))
      {
	switch (event.type)
	{
	  case EVENT_BUTTONPRESS:
	  case EVENT_BUTTONRELEASE:
	  case EVENT_MOTIONNOTIFY:
	  {
	    DrawBuffer *drawto_last = drawto;
	    int mx, my;

	    if (event.type == EVENT_MOTIONNOTIFY)
	    {
	      if (!button_status)
		continue;

	      motion_status = TRUE;
	      mx = ((MotionEvent *) &event)->x;
	      my = ((MotionEvent *) &event)->y;
	    }
	    else
	    {
	      motion_status = FALSE;
	      mx = ((ButtonEvent *) &event)->x;
	      my = ((ButtonEvent *) &event)->y;
	      if (event.type == EVENT_BUTTONPRESS)
		button_status = ((ButtonEvent *) &event)->button;
	      else
		button_status = MB_RELEASED;
	    }

	    if (global.use_envelope_request)
	    {
	      // draw changed button states to temporary bitmap
	      drawto = bitmap_db_store_1;
	    }

	    // this sets 'request_gadget_id'
	    HandleGadgets(mx, my, button_status);

	    if (global.use_envelope_request)
	    {
	      // restore pointer to drawing buffer
	      drawto = drawto_last;

	      // prepare complete envelope request from temporary bitmap
	      PrepareEnvelopeRequestToScreen(bitmap_db_store_1, sx, sy,
					     width, height);
	    }

	    switch (request_gadget_id)
	    {
	      case TOOL_CTRL_ID_YES:
	      case TOOL_CTRL_ID_TOUCH_YES:
		result = TRUE;
		break;
	      case TOOL_CTRL_ID_NO:
	      case TOOL_CTRL_ID_TOUCH_NO:
		result = FALSE;
		break;
	      case TOOL_CTRL_ID_CONFIRM:
	      case TOOL_CTRL_ID_TOUCH_CONFIRM:
		result = TRUE | FALSE;
		break;

	      case TOOL_CTRL_ID_PLAYER_1:
		result = 1;
		break;
	      case TOOL_CTRL_ID_PLAYER_2:
		result = 2;
		break;
	      case TOOL_CTRL_ID_PLAYER_3:
		result = 3;
		break;
	      case TOOL_CTRL_ID_PLAYER_4:
		result = 4;
		break;

	      default:
		break;
	    }

	    // only needed to handle clickable pointer animations here
	    HandleGlobalAnimClicks(mx, my, button_status, FALSE);

	    break;
	  }

	  case SDL_WINDOWEVENT:
	    HandleWindowEvent((WindowEvent *) &event);
	    break;

	  case SDL_APP_WILLENTERBACKGROUND:
	  case SDL_APP_DIDENTERBACKGROUND:
	  case SDL_APP_WILLENTERFOREGROUND:
	  case SDL_APP_DIDENTERFOREGROUND:
	    HandlePauseResumeEvent((PauseResumeEvent *) &event);
	    break;

	  case EVENT_KEYPRESS:
	  {
	    Key key = GetEventKey((KeyEvent *)&event);

	    switch (key)
	    {
	      case KSYM_space:
		if (req_state & REQ_CONFIRM)
		  result = 1;
		break;

	      case KSYM_Return:
	      case KSYM_y:
	      case KSYM_Y:
	      case KSYM_Select:
	      case KSYM_Menu:
#if defined(KSYM_Rewind)
	      case KSYM_Rewind:		// for Amazon Fire TV remote
#endif
		result = 1;
		break;

	      case KSYM_Escape:
	      case KSYM_n:
	      case KSYM_N:
	      case KSYM_Back:
#if defined(KSYM_FastForward)
	      case KSYM_FastForward:	// for Amazon Fire TV remote
#endif
		result = 0;
		break;

	      default:
		HandleKeysDebug(key, KEY_PRESSED);
		break;
	    }

	    if (req_state & REQ_PLAYER)
	    {
	      int old_player_nr = setup.network_player_nr;

	      if (result != -1)
		result = old_player_nr + 1;

	      switch (key)
	      {
		case KSYM_space:
		  result = old_player_nr + 1;
		  break;

		case KSYM_Up:
		case KSYM_1:
		  result = 1;
		  break;

		case KSYM_Right:
		case KSYM_2:
		  result = 2;
		  break;

		case KSYM_Down:
		case KSYM_3:
		  result = 3;
		  break;

		case KSYM_Left:
		case KSYM_4:
		  result = 4;
		  break;

		default:
		  break;
	      }
	    }

	    break;
	  }

	  case EVENT_FINGERRELEASE:
	  case EVENT_KEYRELEASE:
	    ClearPlayerAction();
	    break;

	  case SDL_CONTROLLERBUTTONDOWN:
	    switch (event.cbutton.button)
	    {
	      case SDL_CONTROLLER_BUTTON_A:
	      case SDL_CONTROLLER_BUTTON_X:
	      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
	      case SDL_CONTROLLER_BUTTON_LEFTSTICK:
		result = 1;
		break;

	      case SDL_CONTROLLER_BUTTON_B:
	      case SDL_CONTROLLER_BUTTON_Y:
	      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
	      case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
	      case SDL_CONTROLLER_BUTTON_BACK:
		result = 0;
		break;
	    }

	    if (req_state & REQ_PLAYER)
	    {
	      int old_player_nr = setup.network_player_nr;

	      if (result != -1)
		result = old_player_nr + 1;

	      switch (event.cbutton.button)
	      {
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
		case SDL_CONTROLLER_BUTTON_Y:
		  result = 1;
		  break;

		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
		case SDL_CONTROLLER_BUTTON_B:
		  result = 2;
		  break;

		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		case SDL_CONTROLLER_BUTTON_A:
		  result = 3;
		  break;

		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
		case SDL_CONTROLLER_BUTTON_X:
		  result = 4;
		  break;

		default:
		  break;
	      }
	    }

	    break;

	  case SDL_CONTROLLERBUTTONUP:
	    HandleJoystickEvent(&event);
	    ClearPlayerAction();
	    break;

	  default:
	    HandleOtherEvents(&event);
	    break;
	}
      }
    }
    else if (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      int joy = AnyJoystick();

      if (joy & JOY_BUTTON_1)
	result = 1;
      else if (joy & JOY_BUTTON_2)
	result = 0;
    }
    else if (AnyJoystick())
    {
      int joy = AnyJoystick();

      if (req_state & REQ_PLAYER)
      {
	if (joy & JOY_UP)
	  result = 1;
	else if (joy & JOY_RIGHT)
	  result = 2;
	else if (joy & JOY_DOWN)
	  result = 3;
	else if (joy & JOY_LEFT)
	  result = 4;
      }
    }

    BackToFront();
  }

  SetDrawtoField(draw_buffer_last);

  return result;
}

static void DoRequestBefore(void)
{
  boolean game_ended = (game_status == GAME_MODE_PLAYING && checkGameEnded());

  // when showing request dialog after game ended, deactivate game panel
  if (game_ended)
    game.panel.active = FALSE;

  if (game_status == GAME_MODE_PLAYING)
    BlitScreenToBitmap(backbuffer);

  // disable deactivated drawing when quick-loading level tape recording
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOff(TRUE);

  SetMouseCursor(CURSOR_DEFAULT);

  // pause network game while waiting for request to answer
  if (network.enabled &&
      game_status == GAME_MODE_PLAYING &&
      !game.all_players_gone)
    SendToServer_PausePlaying();

  // simulate releasing mouse button over last gadget, if still pressed
  if (button_status)
    HandleGadgets(-1, -1, 0);

  UnmapAllGadgets();
}

static void DoRequestAfter(void)
{
  RemapAllGadgets();

  if (game_status == GAME_MODE_PLAYING)
  {
    SetPanelBackground();
    SetDrawBackgroundMask(REDRAW_DOOR_1);
  }
  else
  {
    SetDrawBackgroundMask(REDRAW_FIELD);
  }

  // continue network game after request
  if (network.enabled &&
      game_status == GAME_MODE_PLAYING &&
      !game.all_players_gone)
    SendToServer_ContinuePlaying();

  // restore deactivated drawing when quick-loading level tape recording
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOn();
}

static void setRequestDoorTextProperties(char *text,
					 int text_spacing,
					 int line_spacing,
					 int *set_font_nr,
					 int *set_max_lines,
					 int *set_max_line_length)
{
  struct RectWithBorder *vp_door_1 = &viewport.door_1[game_status];
  struct TextPosInfo *pos = &menu.request.button.confirm;
  int button_ypos = pos->y;
  int font_nr = FONT_TEXT_2;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int line_height = font_height + line_spacing;
  int max_text_width  = vp_door_1->width;
  int max_text_height = button_ypos - 2 * text_spacing;
  int max_line_length = max_text_width  / font_width;
  int max_lines       = max_text_height / line_height;

  if (maxWordLengthInRequestString(text) > max_line_length)
  {
    font_nr = FONT_TEXT_1;
    font_width = getFontWidth(font_nr);
    max_line_length = max_text_width  / font_width;
  }

  *set_font_nr = font_nr;
  *set_max_lines = max_lines;
  *set_max_line_length = max_line_length;
}

static void DrawRequestDoorText(char *text)
{
  char *text_ptr = text;
  int text_spacing = 8;
  int line_spacing = 2;
  int max_request_lines;
  int max_request_line_len;
  int font_nr;
  int ty;

  // force DOOR font inside door area
  SetFontStatus(GAME_MODE_PSEUDO_DOOR);

  setRequestDoorTextProperties(text, text_spacing, line_spacing, &font_nr,
			       &max_request_lines, &max_request_line_len);

  for (text_ptr = text, ty = 0; ty < max_request_lines; ty++)
  {
    char text_line[max_request_line_len + 1];
    int tx, tl, tc = 0;

    if (!*text_ptr)
      break;

    for (tl = 0, tx = 0; tx < max_request_line_len; tl++, tx++)
    {
      tc = *(text_ptr + tx);
      if (!tc || tc == ' ' || tc == '?' || tc == '!')
	break;
    }

    if ((tc == '?' || tc == '!') && tl == 0)
      tl = 1;

    if (!tl)
    { 
      text_ptr++; 
      ty--; 
      continue; 
    }

    strncpy(text_line, text_ptr, tl);
    text_line[tl] = 0;

    DrawText(DX + (DXSIZE - tl * getFontWidth(font_nr)) / 2,
	     DY + text_spacing + ty * (getFontHeight(font_nr) + line_spacing),
	     text_line, font_nr);

    text_ptr += tl + (tc == ' ' ? 1 : 0);
  }

  ResetFontStatus();
}

static int RequestDoor(char *text, unsigned int req_state)
{
  unsigned int old_door_state = GetDoorState();
  int draw_buffer_last = GetDrawtoField();
  int result;

  if (old_door_state & DOOR_OPEN_1)
  {
    CloseDoor(DOOR_CLOSE_1);

    // save old door content
    BlitBitmap(bitmap_db_door_1, bitmap_db_door_1,
	       0, 0, DXSIZE, DYSIZE, DXSIZE, 0);
  }

  SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);
  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  // clear door drawing field
  DrawBackground(DX, DY, DXSIZE, DYSIZE);

  // write text for request
  DrawRequestDoorText(text);

  MapToolButtons(req_state);

  // copy request gadgets to door backbuffer
  BlitBitmap(drawto, bitmap_db_door_1, DX, DY, DXSIZE, DYSIZE, 0, 0);

  OpenDoor(DOOR_OPEN_1);

  // ---------- handle request buttons ----------
  result = RequestHandleEvents(req_state, draw_buffer_last);

  UnmapToolButtons();

  if (!(req_state & REQ_STAY_OPEN))
  {
    CloseDoor(DOOR_CLOSE_1);

    if (((old_door_state & DOOR_OPEN_1) && !(req_state & REQ_STAY_CLOSED)) ||
	(req_state & REQ_REOPEN))
      OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

    game.request_open = FALSE;
  }
  else
  {
    game.request_open = TRUE;
  }

  return result;
}

static int RequestEnvelope(char *text, unsigned int req_state)
{
  int draw_buffer_last = GetDrawtoField();
  int result;

  DrawEnvelopeRequest(text, req_state);
  ShowEnvelopeRequest(text, req_state, ACTION_OPENING);

  // ---------- handle request buttons ----------
  result = RequestHandleEvents(req_state, draw_buffer_last);

  UnmapToolButtons();

  ShowEnvelopeRequest(text, req_state, ACTION_CLOSING);

  return result;
}

int Request(char *text, unsigned int req_state)
{
  boolean overlay_enabled = GetOverlayEnabled();
  int result;

  game.request_active = TRUE;

  SetOverlayEnabled(FALSE);

  DoRequestBefore();

  if (global.use_envelope_request)
    result = RequestEnvelope(text, req_state);
  else
    result = RequestDoor(text, req_state);

  DoRequestAfter();

  SetOverlayEnabled(overlay_enabled);

  game.request_active = FALSE;

  return result;
}

static int compareDoorPartOrderInfo(const void *object1, const void *object2)
{
  const struct DoorPartOrderInfo *dpo1 = (struct DoorPartOrderInfo *)object1;
  const struct DoorPartOrderInfo *dpo2 = (struct DoorPartOrderInfo *)object2;
  int compare_result;

  if (dpo1->sort_priority != dpo2->sort_priority)
    compare_result = dpo1->sort_priority - dpo2->sort_priority;
  else
    compare_result = dpo1->nr - dpo2->nr;

  return compare_result;
}

void InitGraphicCompatibilityInfo_Doors(void)
{
  struct
  {
    int door_token;
    int part_1, part_8;
    struct DoorInfo *door;
  }
  doors[] =
  {
    { DOOR_1,	IMG_GFX_DOOR_1_PART_1,	IMG_GFX_DOOR_1_PART_8,	&door_1	},
    { DOOR_2,	IMG_GFX_DOOR_2_PART_1,	IMG_GFX_DOOR_2_PART_8,	&door_2	},

    { -1,	-1,			-1,			NULL	}
  };
  struct Rect door_rect_list[] =
  {
    { DX, DY, DXSIZE, DYSIZE },
    { VX, VY, VXSIZE, VYSIZE }
  };
  int i, j;

  for (i = 0; doors[i].door_token != -1; i++)
  {
    int door_token = doors[i].door_token;
    int door_index = DOOR_INDEX_FROM_TOKEN(door_token);
    int part_1 = doors[i].part_1;
    int part_8 = doors[i].part_8;
    int part_2 = part_1 + 1;
    int part_3 = part_1 + 2;
    struct DoorInfo *door = doors[i].door;
    struct Rect *door_rect = &door_rect_list[door_index];
    boolean door_gfx_redefined = FALSE;

    // check if any door part graphic definitions have been redefined

    for (j = 0; door_part_controls[j].door_token != -1; j++)
    {
      struct DoorPartControlInfo *dpc = &door_part_controls[j];
      struct FileInfo *fi = getImageListEntryFromImageID(dpc->graphic);

      if (dpc->door_token == door_token && fi->redefined)
	door_gfx_redefined = TRUE;
    }

    // check for old-style door graphic/animation modifications

    if (!door_gfx_redefined)
    {
      if (door->anim_mode & ANIM_STATIC_PANEL)
      {
	door->panel.step_xoffset = 0;
	door->panel.step_yoffset = 0;
      }

      if (door->anim_mode & (ANIM_HORIZONTAL | ANIM_VERTICAL))
      {
	struct GraphicInfo *g_part_1 = &graphic_info[part_1];
	struct GraphicInfo *g_part_2 = &graphic_info[part_2];
	int num_door_steps, num_panel_steps;

	// remove door part graphics other than the two default wings

	for (j = 0; door_part_controls[j].door_token != -1; j++)
	{
	  struct DoorPartControlInfo *dpc = &door_part_controls[j];
	  struct GraphicInfo *g = &graphic_info[dpc->graphic];

	  if (dpc->graphic >= part_3 &&
	      dpc->graphic <= part_8)
	    g->bitmap = NULL;
	}

	// set graphics and screen positions of the default wings

	g_part_1->width  = door_rect->width;
	g_part_1->height = door_rect->height;
	g_part_2->width  = door_rect->width;
	g_part_2->height = door_rect->height;
	g_part_2->src_x = door_rect->width;
	g_part_2->src_y = g_part_1->src_y;

	door->part_2.x = door->part_1.x;
	door->part_2.y = door->part_1.y;

	if (door->width != -1)
	{
	  g_part_1->width = door->width;
	  g_part_2->width = door->width;

	  // special treatment for graphics and screen position of right wing
	  g_part_2->src_x += door_rect->width - door->width;
	  door->part_2.x  += door_rect->width - door->width;
	}

	if (door->height != -1)
	{
	  g_part_1->height = door->height;
	  g_part_2->height = door->height;

	  // special treatment for graphics and screen position of bottom wing
	  g_part_2->src_y += door_rect->height - door->height;
	  door->part_2.y  += door_rect->height - door->height;
	}

	// set animation delays for the default wings and panels

	door->part_1.step_delay = door->step_delay;
	door->part_2.step_delay = door->step_delay;
	door->panel.step_delay  = door->step_delay;

	// set animation draw order for the default wings

	door->part_1.sort_priority = 2;	// draw left wing over ...
	door->part_2.sort_priority = 1;	//          ... right wing

	// set animation draw offset for the default wings

	if (door->anim_mode & ANIM_HORIZONTAL)
	{
	  door->part_1.step_xoffset = door->step_offset;
	  door->part_1.step_yoffset = 0;
	  door->part_2.step_xoffset = door->step_offset * -1;
	  door->part_2.step_yoffset = 0;

	  num_door_steps = g_part_1->width / door->step_offset;
	}
	else	// ANIM_VERTICAL
	{
	  door->part_1.step_xoffset = 0;
	  door->part_1.step_yoffset = door->step_offset;
	  door->part_2.step_xoffset = 0;
	  door->part_2.step_yoffset = door->step_offset * -1;

	  num_door_steps = g_part_1->height / door->step_offset;
	}

	// set animation draw offset for the default panels

	if (door->step_offset > 1)
	{
	  num_panel_steps = 2 * door_rect->height / door->step_offset;
	  door->panel.start_step = num_panel_steps - num_door_steps;
	  door->panel.start_step_closing = door->panel.start_step;
	}
	else
	{
	  num_panel_steps = door_rect->height / door->step_offset;
	  door->panel.start_step = num_panel_steps - num_door_steps / 2;
	  door->panel.start_step_closing = door->panel.start_step;
	  door->panel.step_delay *= 2;
	}
      }
    }
  }
}

void InitDoors(void)
{
  int i;

  for (i = 0; door_part_controls[i].door_token != -1; i++)
  {
    struct DoorPartControlInfo *dpc = &door_part_controls[i];
    struct DoorPartOrderInfo *dpo = &door_part_order[i];

    // initialize "start_step_opening" and "start_step_closing", if needed
    if (dpc->pos->start_step_opening == 0 &&
	dpc->pos->start_step_closing == 0)
    {
      // dpc->pos->start_step_opening = dpc->pos->start_step;
      dpc->pos->start_step_closing = dpc->pos->start_step;
    }

    // fill structure for door part draw order (sorted below)
    dpo->nr = i;
    dpo->sort_priority = dpc->pos->sort_priority;
  }

  // sort door part controls according to sort_priority and graphic number
  qsort(door_part_order, MAX_DOOR_PARTS,
        sizeof(struct DoorPartOrderInfo), compareDoorPartOrderInfo);
}

unsigned int OpenDoor(unsigned int door_state)
{
  if (door_state & DOOR_COPY_BACK)
  {
    if (door_state & DOOR_OPEN_1)
      BlitBitmap(bitmap_db_door_1, bitmap_db_door_1,
		 1 * DXSIZE, 0, DXSIZE, DYSIZE, 0 * DXSIZE, 0);

    if (door_state & DOOR_OPEN_2)
      BlitBitmap(bitmap_db_door_2, bitmap_db_door_2,
		 1 * VXSIZE, 0, VXSIZE, VYSIZE, 0 * VXSIZE, 0);

    door_state &= ~DOOR_COPY_BACK;
  }

  return MoveDoor(door_state);
}

unsigned int CloseDoor(unsigned int door_state)
{
  unsigned int old_door_state = GetDoorState();

  if (!(door_state & DOOR_NO_COPY_BACK))
  {
    if (old_door_state & DOOR_OPEN_1)
      BlitBitmap(backbuffer, bitmap_db_door_1,
		 DX, DY, DXSIZE, DYSIZE, 0, 0);

    if (old_door_state & DOOR_OPEN_2)
      BlitBitmap(backbuffer, bitmap_db_door_2,
		 VX, VY, VXSIZE, VYSIZE, 0, 0);

    door_state &= ~DOOR_NO_COPY_BACK;
  }

  if (door_state & DOOR_CLOSE_1)
    game.request_open = FALSE;

  return MoveDoor(door_state);
}

unsigned int GetDoorState(void)
{
  return MoveDoor(DOOR_GET_STATE);
}

unsigned int SetDoorState(unsigned int door_state)
{
  return MoveDoor(door_state | DOOR_SET_STATE);
}

static int euclid(int a, int b)
{
  return (b ? euclid(b, a % b) : a);
}

unsigned int MoveDoor(unsigned int door_state)
{
  struct Rect door_rect_list[] =
  {
    { DX, DY, DXSIZE, DYSIZE },
    { VX, VY, VXSIZE, VYSIZE }
  };
  static int door1 = DOOR_CLOSE_1;
  static int door2 = DOOR_CLOSE_2;
  DelayCounter door_delay = { 0 };
  int i;

  if (door_state == DOOR_GET_STATE)
    return (door1 | door2);

  if (door_state & DOOR_SET_STATE)
  {
    if (door_state & DOOR_ACTION_1)
      door1 = door_state & DOOR_ACTION_1;
    if (door_state & DOOR_ACTION_2)
      door2 = door_state & DOOR_ACTION_2;

    return (door1 | door2);
  }

  if (!(door_state & DOOR_FORCE_REDRAW))
  {
    if (door1 == DOOR_OPEN_1 && door_state & DOOR_OPEN_1)
      door_state &= ~DOOR_OPEN_1;
    else if (door1 == DOOR_CLOSE_1 && door_state & DOOR_CLOSE_1)
      door_state &= ~DOOR_CLOSE_1;
    if (door2 == DOOR_OPEN_2 && door_state & DOOR_OPEN_2)
      door_state &= ~DOOR_OPEN_2;
    else if (door2 == DOOR_CLOSE_2 && door_state & DOOR_CLOSE_2)
      door_state &= ~DOOR_CLOSE_2;
  }

  if (global.autoplay_leveldir)
  {
    door_state |= DOOR_NO_DELAY;
    door_state &= ~DOOR_CLOSE_ALL;
  }

  if (game_status == GAME_MODE_EDITOR && !(door_state & DOOR_FORCE_ANIM))
    door_state |= DOOR_NO_DELAY;

  if (door_state & DOOR_ACTION)
  {
    boolean game_ended = (game_status == GAME_MODE_PLAYING && checkGameEnded());
    boolean door_panel_drawn[NUM_DOORS];
    boolean panel_has_doors[NUM_DOORS];
    boolean door_part_skip[MAX_DOOR_PARTS];
    boolean door_part_done[MAX_DOOR_PARTS];
    boolean door_part_done_all;
    int num_steps[MAX_DOOR_PARTS];
    int max_move_delay = 0;	// delay for complete animations of all doors
    int max_step_delay = 0;	// delay (ms) between two animation frames
    int num_move_steps = 0;	// number of animation steps for all doors
    int max_move_delay_doors_only = 0;	// delay for doors only (no panel)
    int num_move_steps_doors_only = 0;	// steps for doors only (no panel)
    int start = 0;
    int k;

    for (i = 0; i < NUM_DOORS; i++)
      panel_has_doors[i] = FALSE;

    for (i = 0; i < MAX_DOOR_PARTS; i++)
    {
      struct DoorPartControlInfo *dpc = &door_part_controls[i];
      struct GraphicInfo *g = &graphic_info[dpc->graphic];
      int door_token = dpc->door_token;

      door_part_done[i] = FALSE;
      door_part_skip[i] = (!(door_state & door_token) ||
			   !g->bitmap);
    }

    for (i = 0; i < MAX_DOOR_PARTS; i++)
    {
      int nr = door_part_order[i].nr;
      struct DoorPartControlInfo *dpc = &door_part_controls[nr];
      struct DoorPartPosInfo *pos = dpc->pos;
      struct GraphicInfo *g = &graphic_info[dpc->graphic];
      int door_token = dpc->door_token;
      int door_index = DOOR_INDEX_FROM_TOKEN(door_token);
      boolean is_panel = DOOR_PART_IS_PANEL(nr);
      int step_xoffset = ABS(pos->step_xoffset);
      int step_yoffset = ABS(pos->step_yoffset);
      int step_delay = pos->step_delay;
      int current_door_state = door_state & door_token;
      boolean door_opening = ((current_door_state & DOOR_OPEN)  != 0);
      boolean door_closing = ((current_door_state & DOOR_CLOSE) != 0);
      boolean part_opening = (is_panel ? door_closing : door_opening);
      int start_step = (part_opening ? pos->start_step_opening :
			pos->start_step_closing);
      float move_xsize = (step_xoffset ? g->width  : 0);
      float move_ysize = (step_yoffset ? g->height : 0);
      int move_xsteps = (step_xoffset ? ceil(move_xsize / step_xoffset) : 0);
      int move_ysteps = (step_yoffset ? ceil(move_ysize / step_yoffset) : 0);
      int move_steps = (move_xsteps && move_ysteps ?
			MIN(move_xsteps, move_ysteps) :
			move_xsteps ? move_xsteps : move_ysteps) - start_step;
      int move_delay = move_steps * step_delay;

      if (door_part_skip[nr])
	continue;

      max_move_delay = MAX(max_move_delay, move_delay);
      max_step_delay = (max_step_delay == 0 ? step_delay :
			euclid(max_step_delay, step_delay));
      num_steps[nr] = move_steps;

      if (!is_panel)
      {
	max_move_delay_doors_only = MAX(max_move_delay_doors_only, move_delay);

	panel_has_doors[door_index] = TRUE;
      }
    }

    max_step_delay = MAX(1, max_step_delay);	// prevent division by zero

    num_move_steps = max_move_delay / max_step_delay;
    num_move_steps_doors_only = max_move_delay_doors_only / max_step_delay;

    door_delay.value = max_step_delay;

    if ((door_state & DOOR_NO_DELAY) || setup.quick_doors)
    {
      start = num_move_steps - 1;
    }
    else
    {
      // opening door sound has priority over simultaneously closing door
      if (door_state & (DOOR_OPEN_1 | DOOR_OPEN_2))
      {
        PlayMenuSoundStereo(SND_DOOR_OPENING, SOUND_MIDDLE);

	if (door_state & DOOR_OPEN_1)
	  PlayMenuSoundStereo(SND_DOOR_1_OPENING, SOUND_MIDDLE);
	if (door_state & DOOR_OPEN_2)
	  PlayMenuSoundStereo(SND_DOOR_2_OPENING, SOUND_MIDDLE);
      }
      else if (door_state & (DOOR_CLOSE_1 | DOOR_CLOSE_2))
      {
        PlayMenuSoundStereo(SND_DOOR_CLOSING, SOUND_MIDDLE);

	if (door_state & DOOR_CLOSE_1)
	  PlayMenuSoundStereo(SND_DOOR_1_CLOSING, SOUND_MIDDLE);
	if (door_state & DOOR_CLOSE_2)
	  PlayMenuSoundStereo(SND_DOOR_2_CLOSING, SOUND_MIDDLE);
      }
    }

    SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);

    game.any_door_active = TRUE;

    for (k = start; k < num_move_steps; k++)
    {
      int last_frame = num_move_steps - 1;	// last frame of this "for" loop

      door_part_done_all = TRUE;

      for (i = 0; i < NUM_DOORS; i++)
	door_panel_drawn[i] = FALSE;

      for (i = 0; i < MAX_DOOR_PARTS; i++)
      {
	int nr = door_part_order[i].nr;
	struct DoorPartControlInfo *dpc = &door_part_controls[nr];
	struct DoorPartPosInfo *pos = dpc->pos;
	struct GraphicInfo *g = &graphic_info[dpc->graphic];
	int door_token = dpc->door_token;
	int door_index = DOOR_INDEX_FROM_TOKEN(door_token);
	boolean is_panel = DOOR_PART_IS_PANEL(nr);
	boolean is_panel_and_door_has_closed = FALSE;
	struct Rect *door_rect = &door_rect_list[door_index];
	Bitmap *bitmap_db_door = (door_token == DOOR_1 ? bitmap_db_door_1 :
				  bitmap_db_door_2);
	Bitmap *bitmap = (is_panel ? bitmap_db_door : g->bitmap);
	int current_door_state = door_state & door_token;
	boolean door_opening = ((current_door_state & DOOR_OPEN)  != 0);
	boolean door_closing = !door_opening;
	boolean part_opening = (is_panel ? door_closing : door_opening);
	boolean part_closing = !part_opening;
	int start_step = (part_opening ? pos->start_step_opening :
			  pos->start_step_closing);
	int step_delay = pos->step_delay;
	int step_factor = step_delay / max_step_delay;
	int k1 = (step_factor ? k / step_factor + 1 : k);
	int k2 = (part_opening ? k1 + start_step : num_steps[nr] - k1);
	int kk = MAX(0, k2);
	int g_src_x = 0;
	int g_src_y = 0;
	int src_x, src_y, src_xx, src_yy;
	int dst_x, dst_y, dst_xx, dst_yy;
	int width, height;

	if (door_part_skip[nr])
	  continue;

	if (!(door_state & door_token))
	  continue;

	if (!g->bitmap)
	  continue;

	if (!is_panel)
	{
	  int k2_door = (door_opening ? k : num_move_steps_doors_only - k - 1);
	  int kk_door = MAX(0, k2_door);
	  int sync_frame = kk_door * door_delay.value;
	  int frame = getGraphicAnimationFrame(dpc->graphic, sync_frame);

	  getFixedGraphicSource(dpc->graphic, frame, &bitmap,
				&g_src_x, &g_src_y);
	}

	// draw door panel

	if (!door_panel_drawn[door_index])
	{
	  ClearRectangleOnBackground(drawto, door_rect->x, door_rect->y,
				     door_rect->width, door_rect->height);

	  door_panel_drawn[door_index] = TRUE;
	}

	// draw opening or closing door parts

	if (pos->step_xoffset < 0)	// door part on right side
	{
	  src_xx = 0;
	  dst_xx = pos->x + ABS(kk * pos->step_xoffset);
	  width = g->width;

	  if (dst_xx + width > door_rect->width)
	    width = door_rect->width - dst_xx;
	}
	else				// door part on left side
	{
	  src_xx = 0;
	  dst_xx = pos->x - kk * pos->step_xoffset;

	  if (dst_xx < 0)
	  {
	    src_xx = ABS(dst_xx);
	    dst_xx = 0;
	  }

	  width = g->width - src_xx;

	  if (width > door_rect->width)
	    width = door_rect->width;

	  // Debug("tools:MoveDoor", "k == %d [%d]", k, start_step);
	}

	if (pos->step_yoffset < 0)	// door part on bottom side
	{
	  src_yy = 0;
	  dst_yy = pos->y + ABS(kk * pos->step_yoffset);
	  height = g->height;

	  if (dst_yy + height > door_rect->height)
	    height = door_rect->height - dst_yy;
	}
	else				// door part on top side
	{
	  src_yy = 0;
	  dst_yy = pos->y - kk * pos->step_yoffset;

	  if (dst_yy < 0)
	  {
	    src_yy = ABS(dst_yy);
	    dst_yy = 0;
	  }

	  height = g->height - src_yy;
	}

	src_x = g_src_x + src_xx;
	src_y = g_src_y + src_yy;

	dst_x = door_rect->x + dst_xx;
	dst_y = door_rect->y + dst_yy;

	is_panel_and_door_has_closed =
	  (is_panel &&
	   door_closing &&
	   panel_has_doors[door_index] &&
	   k >= num_move_steps_doors_only - 1);

	if (width  >= 0 && width  <= g->width &&
	    height >= 0 && height <= g->height &&
	    !is_panel_and_door_has_closed)
	{
	  if (is_panel || !pos->draw_masked)
	    BlitBitmap(bitmap, drawto, src_x, src_y, width, height,
		       dst_x, dst_y);
	  else
	    BlitBitmapMasked(bitmap, drawto, src_x, src_y, width, height,
			     dst_x, dst_y);
	}

	redraw_mask |= REDRAW_DOOR_FROM_TOKEN(door_token);

	if ((part_opening && (width < 0         || height < 0)) ||
	    (part_closing && (width >= g->width && height >= g->height)))
	  door_part_done[nr] = TRUE;

	// continue door part animations, but not panel after door has closed
	if (!door_part_done[nr] && !is_panel_and_door_has_closed)
	  door_part_done_all = FALSE;
      }

      if (!(door_state & DOOR_NO_DELAY))
      {
	if (game_ended)
	  HandleGameActions();

	BackToFront();

	SkipUntilDelayReached(&door_delay, &k, last_frame);

	// prevent OS (Windows) from complaining about program not responding
	CheckQuitEvent();
      }

      if (door_part_done_all)
	break;
    }

    if (!(door_state & DOOR_NO_DELAY))
    {
      // wait for specified door action post delay
      if (door_state & DOOR_ACTION_1 && door_state & DOOR_ACTION_2)
	door_delay.value = MAX(door_1.post_delay, door_2.post_delay);
      else if (door_state & DOOR_ACTION_1)
	door_delay.value = door_1.post_delay;
      else if (door_state & DOOR_ACTION_2)
	door_delay.value = door_2.post_delay;

      while (!DelayReached(&door_delay))
      {
	if (game_ended)
	  HandleGameActions();

	BackToFront();
      }
    }

    game.any_door_active = FALSE;
  }

  if (door_state & DOOR_ACTION_1)
    door1 = door_state & DOOR_ACTION_1;
  if (door_state & DOOR_ACTION_2)
    door2 = door_state & DOOR_ACTION_2;

  // draw masked border over door area
  DrawMaskedBorder(REDRAW_DOOR_1);
  DrawMaskedBorder(REDRAW_DOOR_2);

  ClearAutoRepeatKeyEvents();

  return (door1 | door2);
}

static boolean useSpecialEditorDoor(void)
{
  int graphic = IMG_GLOBAL_BORDER_EDITOR;
  boolean redefined = getImageListEntryFromImageID(graphic)->redefined;

  // do not draw special editor door if editor border defined or redefined
  if (graphic_info[graphic].bitmap != NULL || redefined)
    return FALSE;

  // do not draw special editor door if global border defined to be empty
  if (graphic_info[IMG_GLOBAL_BORDER].bitmap == NULL)
    return FALSE;

  // do not draw special editor door if viewport definitions do not match
  if (EX != VX ||
      EY >= VY ||
      EXSIZE != VXSIZE ||
      EY + EYSIZE != VY + VYSIZE)
    return FALSE;

  return TRUE;
}

void DrawSpecialEditorDoor(void)
{
  struct GraphicInfo *gfx1 = &graphic_info[IMG_DOOR_2_TOP_BORDER_CORRECTION];
  int top_border_width = gfx1->width;
  int top_border_height = gfx1->height;
  int outer_border = viewport.door_2[GAME_MODE_EDITOR].border_size;
  int ex = EX - outer_border;
  int ey = EY - outer_border;
  int vy = VY - outer_border;
  int exsize = EXSIZE + 2 * outer_border;

  if (!useSpecialEditorDoor())
    return;

  // draw bigger level editor toolbox window
  BlitBitmap(gfx1->bitmap, drawto, gfx1->src_x, gfx1->src_y,
	     top_border_width, top_border_height, ex, ey - top_border_height);
  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto, ex, vy,
	     exsize, EYSIZE - VYSIZE + outer_border, ex, ey);

  redraw_mask |= REDRAW_ALL;
}

void UndrawSpecialEditorDoor(void)
{
  struct GraphicInfo *gfx1 = &graphic_info[IMG_DOOR_2_TOP_BORDER_CORRECTION];
  int top_border_width = gfx1->width;
  int top_border_height = gfx1->height;
  int outer_border = viewport.door_2[GAME_MODE_EDITOR].border_size;
  int ex = EX - outer_border;
  int ey = EY - outer_border;
  int ey_top = ey - top_border_height;
  int exsize = EXSIZE + 2 * outer_border;
  int eysize = EYSIZE + 2 * outer_border;

  if (!useSpecialEditorDoor())
    return;

  // draw normal tape recorder window
  if (graphic_info[IMG_GLOBAL_BORDER].bitmap)
  {
    BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	       ex, ey_top, top_border_width, top_border_height,
	       ex, ey_top);
    BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	       ex, ey, exsize, eysize, ex, ey);
  }
  else
  {
    // if screen background is set to "[NONE]", clear editor toolbox window
    ClearRectangle(drawto, ex, ey_top, top_border_width, top_border_height);
    ClearRectangle(drawto, ex, ey, exsize, eysize);
  }

  redraw_mask |= REDRAW_ALL;
}


// ---------- new tool button stuff -------------------------------------------

static struct
{
  int graphic;
  struct TextPosInfo *pos;
  int gadget_id;
  boolean is_touch_button;
  char *infotext;
} toolbutton_info[NUM_TOOL_BUTTONS] =
{
  {
    IMG_GFX_REQUEST_BUTTON_YES,		&menu.request.button.yes,
    TOOL_CTRL_ID_YES, FALSE,		"yes"
  },
  {
    IMG_GFX_REQUEST_BUTTON_NO,		&menu.request.button.no,
    TOOL_CTRL_ID_NO, FALSE,		"no"
  },
  {
    IMG_GFX_REQUEST_BUTTON_CONFIRM,	&menu.request.button.confirm,
    TOOL_CTRL_ID_CONFIRM, FALSE,	"confirm"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_1,	&menu.request.button.player_1,
    TOOL_CTRL_ID_PLAYER_1, FALSE,	"player 1"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_2,	&menu.request.button.player_2,
    TOOL_CTRL_ID_PLAYER_2, FALSE,	"player 2"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_3,	&menu.request.button.player_3,
    TOOL_CTRL_ID_PLAYER_3, FALSE,	"player 3"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_4,	&menu.request.button.player_4,
    TOOL_CTRL_ID_PLAYER_4, FALSE,	"player 4"
  },
  {
    IMG_GFX_REQUEST_BUTTON_TOUCH_YES,	&menu.request.button.touch_yes,
    TOOL_CTRL_ID_TOUCH_YES, TRUE,	"yes"
  },
  {
    IMG_GFX_REQUEST_BUTTON_TOUCH_NO,	&menu.request.button.touch_no,
    TOOL_CTRL_ID_TOUCH_NO, TRUE,	"no"
  },
  {
    IMG_GFX_REQUEST_BUTTON_TOUCH_CONFIRM, &menu.request.button.touch_confirm,
    TOOL_CTRL_ID_TOUCH_CONFIRM, TRUE,	"confirm"
  }
};

void CreateToolButtons(void)
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
  {
    int graphic = toolbutton_info[i].graphic;
    struct GraphicInfo *gfx = &graphic_info[graphic];
    struct TextPosInfo *pos = toolbutton_info[i].pos;
    struct GadgetInfo *gi;
    Bitmap *deco_bitmap = None;
    int deco_x = 0, deco_y = 0, deco_xpos = 0, deco_ypos = 0;
    unsigned int event_mask = GD_EVENT_RELEASED;
    boolean is_touch_button = toolbutton_info[i].is_touch_button;
    int base_x = (is_touch_button ? 0 : DX);
    int base_y = (is_touch_button ? 0 : DY);
    int gd_x = gfx->src_x;
    int gd_y = gfx->src_y;
    int gd_xp = gfx->src_x + gfx->pressed_xoffset;
    int gd_yp = gfx->src_y + gfx->pressed_yoffset;
    int x = pos->x;
    int y = pos->y;
    int id = i;

    // do not use touch buttons if overlay touch buttons are disabled
    if (is_touch_button && !setup.touch.overlay_buttons)
      continue;

    if (global.use_envelope_request && !is_touch_button)
    {
      setRequestPosition(&base_x, &base_y, TRUE);

      // check if request buttons are outside of envelope and fix, if needed
      if (x < 0 || x + gfx->width  > menu.request.width ||
	  y < 0 || y + gfx->height > menu.request.height)
      {
	if (id == TOOL_CTRL_ID_YES)
	{
	  x = 0;
	  y = menu.request.height - 2 * menu.request.border_size - gfx->height;
	}
	else if (id == TOOL_CTRL_ID_NO)
	{
	  x = menu.request.width  - 2 * menu.request.border_size - gfx->width;
	  y = menu.request.height - 2 * menu.request.border_size - gfx->height;
	}
	else if (id == TOOL_CTRL_ID_CONFIRM)
	{
	  x = (menu.request.width - 2 * menu.request.border_size - gfx->width) / 2;
	  y = menu.request.height - 2 * menu.request.border_size - gfx->height;
	}
	else if (id >= TOOL_CTRL_ID_PLAYER_1 && id <= TOOL_CTRL_ID_PLAYER_4)
	{
	  int player_nr = id - TOOL_CTRL_ID_PLAYER_1;

	  x = (menu.request.width - 2 * menu.request.border_size - gfx->width) / 2;
	  y = menu.request.height - 2 * menu.request.border_size - gfx->height * 2;

	  x += (player_nr == 3 ? -1 : player_nr == 1 ? +1 : 0) * gfx->width;
	  y += (player_nr == 0 ? -1 : player_nr == 2 ? +1 : 0) * gfx->height;
	}
      }
    }

    if (id >= TOOL_CTRL_ID_PLAYER_1 && id <= TOOL_CTRL_ID_PLAYER_4 &&
	pos->draw_player)
    {
      int player_nr = id - TOOL_CTRL_ID_PLAYER_1;

      getSizedGraphicSource(PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0,
			    pos->size, &deco_bitmap, &deco_x, &deco_y);
      deco_xpos = (gfx->width  - pos->size) / 2;
      deco_ypos = (gfx->height - pos->size) / 2;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_IMAGE_ID, graphic,
		      GDI_INFO_TEXT, toolbutton_info[i].infotext,
		      GDI_X, base_x + x,
		      GDI_Y, base_y + y,
		      GDI_WIDTH, gfx->width,
		      GDI_HEIGHT, gfx->height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gfx->bitmap, gd_x, gd_y,
		      GDI_DESIGN_PRESSED, gfx->bitmap, gd_xp, gd_yp,
		      GDI_DECORATION_DESIGN, deco_bitmap, deco_x, deco_y,
		      GDI_DECORATION_POSITION, deco_xpos, deco_ypos,
		      GDI_DECORATION_SIZE, pos->size, pos->size,
		      GDI_DECORATION_SHIFTING, 1, 1,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_OVERLAY_TOUCH_BUTTON, is_touch_button,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleToolButtons,
		      GDI_END);

    if (gi == NULL)
      Fail("cannot create gadget");

    tool_gadget[id] = gi;
  }
}

void FreeToolButtons(void)
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    FreeGadget(tool_gadget[i]);
}

static void MapToolButtons(unsigned int req_state)
{
  if (req_state & REQ_ASK)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_YES]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_NO]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_TOUCH_YES]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_TOUCH_NO]);
  }
  else if (req_state & REQ_CONFIRM)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_CONFIRM]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_TOUCH_CONFIRM]);
  }
  else if (req_state & REQ_PLAYER)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_1]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_2]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_3]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_4]);
  }
}

static void UnmapToolButtons(void)
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    UnmapGadget(tool_gadget[i]);
}

static void HandleToolButtons(struct GadgetInfo *gi)
{
  request_gadget_id = gi->custom_id;
}

static int getEngineElement_Ext(int element, int game_engine_type, boolean is_drawing_element)
{
  int el_empty;
  int el_player;
  int el_sand;
  int el_wall;
  int el_steelwall;
  int el_exit_closed;

  if (game_engine_type == -1)
    game_engine_type = level.game_engine_type;

  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
  {
    el_empty		= EL_EMPTY;
    el_player		= EL_BDX_PLAYER;
    el_sand		= EL_BDX_SAND;
    el_wall		= EL_BDX_WALL;
    el_steelwall	= EL_BDX_STEELWALL;
    el_exit_closed	= EL_BDX_EXIT_CLOSED;
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
  {
    el_empty		= EL_EMPTY;
    el_player		= EL_PLAYER_1;
    el_sand		= EL_SAND;
    el_wall		= EL_WALL;
    el_steelwall	= EL_STEELWALL;
    el_exit_closed	= EL_EM_EXIT_CLOSED;
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    el_empty		= EL_EMPTY;
    el_player		= EL_SP_MURPHY;
    el_sand		= EL_SP_BASE;
    el_wall		= EL_SP_CHIP_SINGLE;
    el_steelwall	= EL_SP_HARDWARE_GRAY;
    el_exit_closed	= EL_SP_EXIT_CLOSED;
  }
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
  {
    el_empty		= EL_EMPTY;
    el_player		= EL_MM_MCDUFFIN_DOWN;
    el_sand		= EL_EMPTY;
    el_wall		= EL_MM_WOODEN_WALL;
    el_steelwall	= EL_MM_STEEL_WALL;
    el_exit_closed	= EL_MM_EXIT_CLOSED;

    if (is_drawing_element)
    {
      el_wall		= EL_MM_MIRROR_START;
      el_sand		= EL_MM_WOODEN_WALL;
    }
  }
  else
  {
    el_empty		= EL_EMPTY;
    el_player		= EL_PLAYER_1;
    el_sand		= EL_SAND;
    el_wall		= EL_WALL;
    el_steelwall	= EL_STEELWALL;
    el_exit_closed	= EL_EXIT_CLOSED;
  }

  return (element == EL_EMPTY		? el_empty :
	  element == EL_PLAYER_1	? el_player :
	  element == EL_SAND		? el_sand :
	  element == EL_WALL		? el_wall :
	  element == EL_STEELWALL	? el_steelwall :
	  element == EL_EXIT_CLOSED	? el_exit_closed : EL_EMPTY);
}

int getEngineElement(int element)
{
  return getEngineElement_Ext(element, -1, FALSE);
}

int getDrawingElement(int element)
{
  return getEngineElement_Ext(element, -1, TRUE);
}

static struct Mapping_BD_to_RND_object
{
  int element_bd;
  boolean is_rnd_to_bd_mapping;		// unique mapping BD <-> RND

  int element_rnd;
  int action;
  int direction;
}
bd_object_mapping_list[] =
{
  // additional RND style elements mapped to BD style elements (must be listed first)

  {
    O_SPACE,					TRUE,
    EL_EMPTY,					-1, -1
  },
  {
    O_DIRT,					TRUE,
    EL_SAND,					-1, -1
  },
  {
    O_STONE,					TRUE,
    EL_BD_ROCK,					-1, -1
  },
  {
    O_BRICK,					TRUE,
    EL_BD_WALL,					-1, -1
  },
  {
    O_STEEL,					TRUE,
    EL_STEELWALL,				-1, -1
  },
  {
    O_DIAMOND,					TRUE,
    EL_BD_DIAMOND,				-1, -1
  },
  {
    O_INBOX,					TRUE,
    EL_PLAYER_1,				-1, -1
  },
  {
    O_INBOX,					TRUE,
    EL_PLAYER_2,				-1, -1
  },
  {
    O_INBOX,					TRUE,
    EL_PLAYER_3,				-1, -1
  },
  {
    O_INBOX,					TRUE,
    EL_PLAYER_4,				-1, -1
  },
  {
    O_PRE_OUTBOX,				TRUE,
    EL_EXIT_CLOSED,				-1, -1
  },

  // BD style elements with their corresponding RND style elements

  {
    O_SPACE,					TRUE,
    EL_BDX_EMPTY,				-1, -1
  },
  {
    O_DIRT,					TRUE,
    EL_BDX_SAND,				-1, -1
  },
  {
    O_DIRT_CRUMBLED,				FALSE,
    EL_BDX_SAND,				-1, -1
  },
  {
    O_DIRT_DIGGING_LEFT,			FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    O_DIRT_DIGGING_RIGHT,			FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    O_DIRT_DIGGING_UP,				FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    O_DIRT_DIGGING_DOWN,			FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    O_DIRT_DIGGING_LEFT_CRUMBLED,		FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    O_DIRT_DIGGING_RIGHT_CRUMBLED,		FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    O_DIRT_DIGGING_UP_CRUMBLED,			FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    O_DIRT_DIGGING_DOWN_CRUMBLED,		FALSE,
    EL_BDX_SAND,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    O_DIRT_SLOPED_UP_RIGHT,			TRUE,
    EL_BDX_SAND_SLOPED_UP_RIGHT,		-1, -1
  },
  {
    O_DIRT_SLOPED_UP_LEFT,			TRUE,
    EL_BDX_SAND_SLOPED_UP_LEFT,			-1, -1
  },
  {
    O_DIRT_SLOPED_DOWN_LEFT,			TRUE,
    EL_BDX_SAND_SLOPED_DOWN_LEFT,		-1, -1
  },
  {
    O_DIRT_SLOPED_DOWN_RIGHT,			TRUE,
    EL_BDX_SAND_SLOPED_DOWN_RIGHT,		-1, -1
  },
  {
    O_DIRT_SLOPED_UP_RIGHT_CRUMBLED,		FALSE,
    EL_BDX_SAND_SLOPED_UP_RIGHT,		-1, -1
  },
  {
    O_DIRT_SLOPED_UP_LEFT_CRUMBLED,		FALSE,
    EL_BDX_SAND_SLOPED_UP_LEFT,			-1, -1
  },
  {
    O_DIRT_SLOPED_DOWN_LEFT_CRUMBLED,		FALSE,
    EL_BDX_SAND_SLOPED_DOWN_LEFT,		-1, -1
  },
  {
    O_DIRT_SLOPED_DOWN_RIGHT_CRUMBLED,		FALSE,
    EL_BDX_SAND_SLOPED_DOWN_RIGHT,		-1, -1
  },
  {
    O_DIRT_BALL,				TRUE,
    EL_BDX_GRASS_BALL,				-1, -1
  },
  {
    O_DIRT_BALL_F,				TRUE,
    EL_BDX_GRASS_BALL_FALLING,			-1, -1
  },
  {
    O_DIRT_BALL_F,				FALSE,
    EL_BDX_GRASS_BALL,				ACTION_FALLING, -1
  },
  {
    O_DIRT_LOOSE,				TRUE,
    EL_BDX_GRASS_LOOSE,				-1, -1
  },
  {
    O_DIRT_LOOSE_F,				TRUE,
    EL_BDX_GRASS_LOOSE_FALLING,			-1, -1
  },
  {
    O_DIRT_LOOSE_F,				FALSE,
    EL_BDX_GRASS_LOOSE,				ACTION_FALLING, -1
  },
  {
    O_DIRT2,					TRUE,
    EL_BDX_GRASS,				-1, -1
  },
  {
    O_DIRT2_CRUMBLED,				FALSE,
    EL_BDX_GRASS,				-1, -1
  },
  {
    O_DIRT2_DIGGING_LEFT,			FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    O_DIRT2_DIGGING_RIGHT,			FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    O_DIRT2_DIGGING_UP,				FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    O_DIRT2_DIGGING_DOWN,			FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    O_DIRT2_DIGGING_LEFT_CRUMBLED,		FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    O_DIRT2_DIGGING_RIGHT_CRUMBLED,		FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    O_DIRT2_DIGGING_UP_CRUMBLED,			FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    O_DIRT2_DIGGING_DOWN_CRUMBLED,		FALSE,
    EL_BDX_GRASS,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    O_BRICK,					TRUE,
    EL_BDX_WALL,				-1, -1
  },
  {
    O_BRICK_SLOPED_UP_RIGHT,			TRUE,
    EL_BDX_WALL_SLOPED_UP_RIGHT,		-1, -1
  },
  {
    O_BRICK_SLOPED_UP_LEFT,			TRUE,
    EL_BDX_WALL_SLOPED_UP_LEFT,			-1, -1
  },
  {
    O_BRICK_SLOPED_DOWN_LEFT,			TRUE,
    EL_BDX_WALL_SLOPED_DOWN_LEFT,		-1, -1
  },
  {
    O_BRICK_SLOPED_DOWN_RIGHT,			TRUE,
    EL_BDX_WALL_SLOPED_DOWN_RIGHT,		-1, -1
  },
  {
    O_BRICK_NON_SLOPED,				TRUE,
    EL_BDX_WALL_NON_SLOPED,			-1, -1
  },
  {
    O_MAGIC_WALL,				TRUE,
    EL_BDX_MAGIC_WALL,				-1, -1
  },
  {
    O_PRE_OUTBOX,				TRUE,
    EL_BDX_EXIT_CLOSED,				-1, -1
  },
  {
    O_OUTBOX,					TRUE,
    EL_BDX_EXIT_OPEN,				-1, -1
  },
  {
    O_PRE_INVIS_OUTBOX,				TRUE,
    EL_BDX_INVISIBLE_EXIT_CLOSED,		-1, -1
  },
  {
    O_INVIS_OUTBOX,				TRUE,
    EL_BDX_INVISIBLE_EXIT_OPEN,			-1, -1
  },
  {
    O_STEEL,					TRUE,
    EL_BDX_STEELWALL,				-1, -1
  },
  {
    O_STEEL_SLOPED_UP_RIGHT,			TRUE,
    EL_BDX_STEELWALL_SLOPED_UP_RIGHT,		-1, -1
  },
  {
    O_STEEL_SLOPED_UP_LEFT,			TRUE,
    EL_BDX_STEELWALL_SLOPED_UP_LEFT,		-1, -1
  },
  {
    O_STEEL_SLOPED_DOWN_LEFT,			TRUE,
    EL_BDX_STEELWALL_SLOPED_DOWN_LEFT,		-1, -1
  },
  {
    O_STEEL_SLOPED_DOWN_RIGHT,			TRUE,
    EL_BDX_STEELWALL_SLOPED_DOWN_RIGHT,		-1, -1
  },
  {
    O_STEEL_EXPLODABLE,				TRUE,
    EL_BDX_STEELWALL_EXPLODABLE,		-1, -1
  },
  {
    O_STEEL_EATABLE,				TRUE,
    EL_BDX_STEELWALL_DIGGABLE,			-1, -1
  },
  {
    O_BRICK_EATABLE,				TRUE,
    EL_BDX_WALL_DIGGABLE,			-1, -1
  },
  {
    O_STONE,					TRUE,
    EL_BDX_ROCK,				-1, -1
  },
  {
    O_STONE_F,					TRUE,
    EL_BDX_ROCK_FALLING,			-1, -1
  },
  {
    O_STONE_F,					FALSE,
    EL_BDX_ROCK,				ACTION_FALLING, -1
  },
  {
    O_FLYING_STONE,				TRUE,
    EL_BDX_FLYING_ROCK,				-1, -1
  },
  {
    O_FLYING_STONE_F,				TRUE,
    EL_BDX_FLYING_ROCK_FLYING,			-1, -1
  },
  {
    O_FLYING_STONE_F,				FALSE,
    EL_BDX_FLYING_ROCK,				ACTION_FLYING, -1
  },
  {
    O_MEGA_STONE,				TRUE,
    EL_BDX_HEAVY_ROCK,				-1, -1
  },
  {
    O_MEGA_STONE_F,				TRUE,
    EL_BDX_HEAVY_ROCK_FALLING,			-1, -1
  },
  {
    O_MEGA_STONE_F,				FALSE,
    EL_BDX_HEAVY_ROCK,				ACTION_FALLING, -1
  },
  {
    O_LIGHT_STONE,				TRUE,
    EL_BDX_LIGHT_ROCK,				-1, -1
  },
  {
    O_LIGHT_STONE_F,				TRUE,
    EL_BDX_LIGHT_ROCK_FALLING,			-1, -1
  },
  {
    O_LIGHT_STONE_F,				FALSE,
    EL_BDX_LIGHT_ROCK,				ACTION_FALLING, -1
  },
  {
    O_DIAMOND,					TRUE,
    EL_BDX_DIAMOND,				-1, -1
  },
  {
    O_DIAMOND_F,				TRUE,
    EL_BDX_DIAMOND_FALLING,			-1, -1
  },
  {
    O_DIAMOND_F,				FALSE,
    EL_BDX_DIAMOND,				ACTION_FALLING, -1
  },
  {
    O_FLYING_DIAMOND,				TRUE,
    EL_BDX_FLYING_DIAMOND,			-1, -1
  },
  {
    O_FLYING_DIAMOND_F,				TRUE,
    EL_BDX_FLYING_DIAMOND_FLYING,		-1, -1
  },
  {
    O_FLYING_DIAMOND_F,				FALSE,
    EL_BDX_FLYING_DIAMOND,			ACTION_FLYING, -1
  },
  {
    O_NUT,					TRUE,
    EL_BDX_NUT,					-1, -1
  },
  {
    O_NUT_F,					TRUE,
    EL_BDX_NUT_FALLING,				-1, -1
  },
  {
    O_NUT_F,					FALSE,
    EL_BDX_NUT,					ACTION_FALLING, -1
  },
  {
    O_BLADDER_SPENDER,				TRUE,
    EL_BDX_TRAPPED_BUBBLE,			-1, -1
  },
  {
    O_INBOX,					TRUE,
    EL_BDX_INBOX,				-1, -1
  },
  {
    O_H_EXPANDING_WALL,				TRUE,
    EL_BDX_EXPANDABLE_WALL_HORIZONTAL,		-1, -1
  },
  {
    O_V_EXPANDING_WALL,				TRUE,
    EL_BDX_EXPANDABLE_WALL_VERTICAL,		-1, -1
  },
  {
    O_EXPANDING_WALL,				TRUE,
    EL_BDX_EXPANDABLE_WALL_ANY,			-1, -1
  },
  {
    O_H_EXPANDING_STEEL_WALL,			TRUE,
    EL_BDX_EXPANDABLE_STEELWALL_HORIZONTAL,	-1, -1
  },
  {
    O_V_EXPANDING_STEEL_WALL,			TRUE,
    EL_BDX_EXPANDABLE_STEELWALL_VERTICAL,	-1, -1
  },
  {
    O_EXPANDING_STEEL_WALL,			TRUE,
    EL_BDX_EXPANDABLE_STEELWALL_ANY,		-1, -1
  },
  {
    O_EXPANDING_WALL_SWITCH,			TRUE,
    EL_BDX_EXPANDABLE_WALL_SWITCH,		-1, -1
  },
  {
    O_CREATURE_SWITCH,				TRUE,
    EL_BDX_CREATURE_SWITCH,			-1, -1
  },
  {
    O_BITER_SWITCH,				TRUE,
    EL_BDX_BITER_SWITCH_1,			-1, -1
  },
  {
    O_REPLICATOR_SWITCH,			TRUE,
    EL_BDX_REPLICATOR_SWITCH,			-1, -1
  },
  {
    O_CONVEYOR_SWITCH,				TRUE,
    EL_BDX_CONVEYOR_SWITCH,			-1, -1
  },
  {
    O_CONVEYOR_DIR_SWITCH,			TRUE,
    EL_BDX_CONVEYOR_DIR_SWITCH,			-1, -1
  },
  {
    O_ACID,					TRUE,
    EL_BDX_ACID,				-1, -1
  },
  {
    O_FALLING_WALL,				TRUE,
    EL_BDX_FALLING_WALL,			-1, -1
  },
  {
    O_FALLING_WALL_F,				TRUE,
    EL_BDX_FALLING_WALL_FALLING,		-1, -1
  },
  {
    O_FALLING_WALL_F,				FALSE,
    EL_BDX_FALLING_WALL,			ACTION_FALLING, -1
  },
  {
    O_BOX,					TRUE,
    EL_BDX_BOX,					-1, -1
  },
  {
    O_TIME_PENALTY,				TRUE,
    EL_BDX_TIME_PENALTY,			-1, -1
  },
  {
    O_GRAVESTONE,				TRUE,
    EL_BDX_GRAVESTONE,				-1, -1
  },
  {
    O_STONE_GLUED,				TRUE,
    EL_BDX_ROCK_GLUED,				-1, -1
  },
  {
    O_DIAMOND_GLUED,				TRUE,
    EL_BDX_DIAMOND_GLUED,			-1, -1
  },
  {
    O_DIAMOND_KEY,				TRUE,
    EL_BDX_DIAMOND_KEY,				-1, -1
  },
  {
    O_TRAPPED_DIAMOND,				TRUE,
    EL_BDX_TRAPPED_DIAMOND,			-1, -1
  },
  {
    O_CLOCK,					TRUE,
    EL_BDX_CLOCK,				-1, -1
  },
  {
    O_DIRT_GLUED,				TRUE,
    EL_BDX_SAND_GLUED,				-1, -1
  },
  {
    O_DIRT_GLUED_CRUMBLED,			FALSE,
    EL_BDX_SAND_GLUED,				-1, -1
  },
  {
    O_KEY_1,					TRUE,
    EL_BDX_KEY_1,				-1, -1
  },
  {
    O_KEY_2,					TRUE,
    EL_BDX_KEY_2,				-1, -1
  },
  {
    O_KEY_3,					TRUE,
    EL_BDX_KEY_3,				-1, -1
  },
  {
    O_DOOR_1,					TRUE,
    EL_BDX_GATE_1,				-1, -1
  },
  {
    O_DOOR_2,					TRUE,
    EL_BDX_GATE_2,				-1, -1
  },
  {
    O_DOOR_3,					TRUE,
    EL_BDX_GATE_3,				-1, -1
  },
  {
    O_POT,					TRUE,
    EL_BDX_POT,					-1, -1
  },
  {
    O_GRAVITY_SWITCH,				TRUE,
    EL_BDX_GRAVITY_SWITCH,			-1, -1
  },
  {
    O_PNEUMATIC_HAMMER,				TRUE,
    EL_BDX_PNEUMATIC_HAMMER,			-1, -1
  },
  {
    O_TELEPORTER,				TRUE,
    EL_BDX_TELEPORTER,				-1, -1
  },
  {
    O_SKELETON,					TRUE,
    EL_BDX_SKELETON,				-1, -1
  },
  {
    O_WATER,					TRUE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_1,					TRUE,
    EL_BDX_WATER_1,				-1, -1
  },
  {
    O_WATER_1,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_2,					TRUE,
    EL_BDX_WATER_2,				-1, -1
  },
  {
    O_WATER_2,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_3,					TRUE,
    EL_BDX_WATER_3,				-1, -1
  },
  {
    O_WATER_3,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_4,					TRUE,
    EL_BDX_WATER_4,				-1, -1
  },
  {
    O_WATER_4,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_5,					TRUE,
    EL_BDX_WATER_5,				-1, -1
  },
  {
    O_WATER_5,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_6,					TRUE,
    EL_BDX_WATER_6,				-1, -1
  },
  {
    O_WATER_6,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_7,					TRUE,
    EL_BDX_WATER_7,				-1, -1
  },
  {
    O_WATER_7,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_8,					TRUE,
    EL_BDX_WATER_8,				-1, -1
  },
  {
    O_WATER_8,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_9,					TRUE,
    EL_BDX_WATER_9,				-1, -1
  },
  {
    O_WATER_9,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_10,					TRUE,
    EL_BDX_WATER_10,				-1, -1
  },
  {
    O_WATER_10,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_11,					TRUE,
    EL_BDX_WATER_11,				-1, -1
  },
  {
    O_WATER_11,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_12,					TRUE,
    EL_BDX_WATER_12,				-1, -1
  },
  {
    O_WATER_12,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_13,					TRUE,
    EL_BDX_WATER_13,				-1, -1
  },
  {
    O_WATER_13,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_14,					TRUE,
    EL_BDX_WATER_14,				-1, -1
  },
  {
    O_WATER_14,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_15,					TRUE,
    EL_BDX_WATER_15,				-1, -1
  },
  {
    O_WATER_15,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_WATER_16,					TRUE,
    EL_BDX_WATER_16,				-1, -1
  },
  {
    O_WATER_16,					FALSE,
    EL_BDX_WATER,				-1, -1
  },
  {
    O_COW,					TRUE,
    EL_BDX_COW,					-1, -1
  },
  {
    O_COW_1,					TRUE,
    EL_BDX_COW_LEFT,				-1, -1
  },
  {
    O_COW_2,					TRUE,
    EL_BDX_COW_UP,				-1, -1
  },
  {
    O_COW_3,					TRUE,
    EL_BDX_COW_RIGHT,				-1, -1
  },
  {
    O_COW_4,					TRUE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_1,				TRUE,
    EL_BDX_COW_ENCLOSED_1,			-1, -1
  },
  {
    O_COW_ENCLOSED_1,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_2,				TRUE,
    EL_BDX_COW_ENCLOSED_2,			-1, -1
  },
  {
    O_COW_ENCLOSED_2,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_3,				TRUE,
    EL_BDX_COW_ENCLOSED_3,			-1, -1
  },
  {
    O_COW_ENCLOSED_3,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_4,				TRUE,
    EL_BDX_COW_ENCLOSED_4,			-1, -1
  },
  {
    O_COW_ENCLOSED_4,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_5,				TRUE,
    EL_BDX_COW_ENCLOSED_5,			-1, -1
  },
  {
    O_COW_ENCLOSED_5,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_6,				TRUE,
    EL_BDX_COW_ENCLOSED_6,			-1, -1
  },
  {
    O_COW_ENCLOSED_6,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_COW_ENCLOSED_7,				TRUE,
    EL_BDX_COW_ENCLOSED_7,			-1, -1
  },
  {
    O_COW_ENCLOSED_7,				FALSE,
    EL_BDX_COW_DOWN,				-1, -1
  },
  {
    O_WALLED_DIAMOND,				TRUE,
    EL_BDX_WALL_DIAMOND,			-1, -1
  },
  {
    O_WALLED_KEY_1,				TRUE,
    EL_BDX_WALL_KEY_1,				-1, -1
  },
  {
    O_WALLED_KEY_2,				TRUE,
    EL_BDX_WALL_KEY_2,				-1, -1
  },
  {
    O_WALLED_KEY_3,				TRUE,
    EL_BDX_WALL_KEY_3,				-1, -1
  },
  {
    O_AMOEBA,					TRUE,
    EL_BDX_AMOEBA_1,				-1, -1
  },
  {
    O_AMOEBA_2,					TRUE,
    EL_BDX_AMOEBA_2,				-1, -1
  },
  {
    O_REPLICATOR,				TRUE,
    EL_BDX_REPLICATOR,				-1, -1
  },
  {
    O_CONVEYOR_LEFT,				TRUE,
    EL_BDX_CONVEYOR_LEFT,			-1, -1
  },
  {
    O_CONVEYOR_RIGHT,				TRUE,
    EL_BDX_CONVEYOR_RIGHT,			-1, -1
  },
  {
    O_LAVA,					TRUE,
    EL_BDX_LAVA,				-1, -1
  },
  {
    O_SWEET,					TRUE,
    EL_BDX_SWEET,				-1, -1
  },
  {
    O_VOODOO,					TRUE,
    EL_BDX_VOODOO_DOLL,				-1, -1
  },
  {
    O_SLIME,					TRUE,
    EL_BDX_SLIME,				-1, -1
  },
  {
    O_BLADDER,					TRUE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_1,				TRUE,
    EL_BDX_BUBBLE_1,				-1, -1
  },
  {
    O_BLADDER_1,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_2,				TRUE,
    EL_BDX_BUBBLE_2,				-1, -1
  },
  {
    O_BLADDER_2,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_3,				TRUE,
    EL_BDX_BUBBLE_3,				-1, -1
  },
  {
    O_BLADDER_3,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_4,				TRUE,
    EL_BDX_BUBBLE_4,				-1, -1
  },
  {
    O_BLADDER_4,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_5,				TRUE,
    EL_BDX_BUBBLE_5,				-1, -1
  },
  {
    O_BLADDER_5,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_6,				TRUE,
    EL_BDX_BUBBLE_6,				-1, -1
  },
  {
    O_BLADDER_6,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_7,				TRUE,
    EL_BDX_BUBBLE_7,				-1, -1
  },
  {
    O_BLADDER_7,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_BLADDER_8,				TRUE,
    EL_BDX_BUBBLE_8,				-1, -1
  },
  {
    O_BLADDER_8,				FALSE,
    EL_BDX_BUBBLE,				-1, -1
  },
  {
    O_WAITING_STONE,				TRUE,
    EL_BDX_WAITING_ROCK,			-1, -1
  },
  {
    O_CHASING_STONE,				TRUE,
    EL_BDX_CHASING_ROCK,			-1, -1
  },
  {
    O_GHOST,					TRUE,
    EL_BDX_GHOST,				-1, -1
  },
  {
    O_FIREFLY,					TRUE,
    EL_BDX_FIREFLY_1,				-1, -1
  },
  {
    O_FIREFLY_1,				TRUE,
    EL_BDX_FIREFLY_1_LEFT,			-1, -1
  },
  {
    O_FIREFLY_2,				TRUE,
    EL_BDX_FIREFLY_1_UP,			-1, -1
  },
  {
    O_FIREFLY_3,				TRUE,
    EL_BDX_FIREFLY_1_RIGHT,			-1, -1
  },
  {
    O_FIREFLY_4,				TRUE,
    EL_BDX_FIREFLY_1_DOWN,			-1, -1
  },
  {
    O_ALT_FIREFLY,				TRUE,
    EL_BDX_FIREFLY_2,				-1, -1
  },
  {
    O_ALT_FIREFLY_1,				TRUE,
    EL_BDX_FIREFLY_2_LEFT,			-1, -1
  },
  {
    O_ALT_FIREFLY_2,				TRUE,
    EL_BDX_FIREFLY_2_UP,			-1, -1
  },
  {
    O_ALT_FIREFLY_3,				TRUE,
    EL_BDX_FIREFLY_2_RIGHT,			-1, -1
  },
  {
    O_ALT_FIREFLY_4,				TRUE,
    EL_BDX_FIREFLY_2_DOWN,			-1, -1
  },
  {
    O_BUTTER,					TRUE,
    EL_BDX_BUTTERFLY_1,				-1, -1
  },
  {
    O_BUTTER_1,					TRUE,
    EL_BDX_BUTTERFLY_1_LEFT,			-1, -1
  },
  {
    O_BUTTER_2,					TRUE,
    EL_BDX_BUTTERFLY_1_UP,			-1, -1
  },
  {
    O_BUTTER_3,					TRUE,
    EL_BDX_BUTTERFLY_1_RIGHT,			-1, -1
  },
  {
    O_BUTTER_4,					TRUE,
    EL_BDX_BUTTERFLY_1_DOWN,			-1, -1
  },
  {
    O_ALT_BUTTER,				TRUE,
    EL_BDX_BUTTERFLY_2,				-1, -1
  },
  {
    O_ALT_BUTTER_1,				TRUE,
    EL_BDX_BUTTERFLY_2_LEFT,			-1, -1
  },
  {
    O_ALT_BUTTER_2,				TRUE,
    EL_BDX_BUTTERFLY_2_UP,			-1, -1
  },
  {
    O_ALT_BUTTER_3,				TRUE,
    EL_BDX_BUTTERFLY_2_RIGHT,			-1, -1
  },
  {
    O_ALT_BUTTER_4,				TRUE,
    EL_BDX_BUTTERFLY_2_DOWN,			-1, -1
  },
  {
    O_STONEFLY,					TRUE,
    EL_BDX_STONEFLY,				-1, -1
  },
  {
    O_STONEFLY_1,				TRUE,
    EL_BDX_STONEFLY_LEFT,			-1, -1
  },
  {
    O_STONEFLY_2,				TRUE,
    EL_BDX_STONEFLY_UP,				-1, -1
  },
  {
    O_STONEFLY_3,				TRUE,
    EL_BDX_STONEFLY_RIGHT,			-1, -1
  },
  {
    O_STONEFLY_4,				TRUE,
    EL_BDX_STONEFLY_DOWN,			-1, -1
  },
  {
    O_BITER,					TRUE,
    EL_BDX_BITER,				-1, -1
  },
  {
    O_BITER_1,					TRUE,
    EL_BDX_BITER_UP,				-1, -1
  },
  {
    O_BITER_2,					TRUE,
    EL_BDX_BITER_RIGHT,				-1, -1
  },
  {
    O_BITER_3,					TRUE,
    EL_BDX_BITER_DOWN,				-1, -1
  },
  {
    O_BITER_4,					TRUE,
    EL_BDX_BITER_LEFT,				-1, -1
  },
  {
    O_DRAGONFLY,				TRUE,
    EL_BDX_DRAGONFLY,				-1, -1
  },
  {
    O_DRAGONFLY_1,				TRUE,
    EL_BDX_DRAGONFLY_LEFT,			-1, -1
  },
  {
    O_DRAGONFLY_2,				TRUE,
    EL_BDX_DRAGONFLY_UP,			-1, -1
  },
  {
    O_DRAGONFLY_3,				TRUE,
    EL_BDX_DRAGONFLY_RIGHT,			-1, -1
  },
  {
    O_DRAGONFLY_4,				TRUE,
    EL_BDX_DRAGONFLY_DOWN,			-1, -1
  },
  {
    O_PRE_PL_1,					TRUE,
    EL_BDX_PLAYER_GROWING_1,			-1, -1
  },
  {
    O_PRE_PL_1,					FALSE,
    EL_BDX_PLAYER,				ACTION_GROWING, -1
  },
  {
    O_PRE_PL_2,					TRUE,
    EL_BDX_PLAYER_GROWING_2,			-1, -1
  },
  {
    O_PRE_PL_2,					FALSE,
    EL_BDX_PLAYER,				ACTION_GROWING, -1
  },
  {
    O_PRE_PL_3,					TRUE,
    EL_BDX_PLAYER_GROWING_3,			-1, -1
  },
  {
    O_PRE_PL_3,					FALSE,
    EL_BDX_PLAYER,				ACTION_GROWING, -1
  },
  {
    O_PLAYER_START,				TRUE,
    EL_BDX_PLAYER,				-1, -1
  },
  {
    O_PLAYER,					TRUE,
    EL_BDX_PLAYER,				-1, -1
  },
  {
    O_PLAYER_BOMB,				TRUE,
    EL_BDX_PLAYER_WITH_BOMB,			-1, -1
  },
  {
    O_PLAYER_ROCKET_LAUNCHER,			TRUE,
    EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER,		-1, -1
  },
  {
    O_PLAYER_GLUED,				TRUE,
    EL_BDX_PLAYER_GLUED,			-1, -1
  },
  {
    O_PLAYER_STIRRING,				TRUE,
    EL_BDX_PLAYER_STIRRING,			-1, -1
  },
  {
    O_ROCKET_LAUNCHER,				TRUE,
    EL_BDX_ROCKET_LAUNCHER,			-1, -1
  },
  {
    O_ROCKET,					TRUE,
    EL_BDX_ROCKET,				-1, -1
  },
  {
    O_ROCKET_1,					TRUE,
    EL_BDX_ROCKET_RIGHT,			-1, -1
  },
  {
    O_ROCKET_2,					TRUE,
    EL_BDX_ROCKET_UP,				-1, -1
  },
  {
    O_ROCKET_3,					TRUE,
    EL_BDX_ROCKET_LEFT,				-1, -1
  },
  {
    O_ROCKET_4,					TRUE,
    EL_BDX_ROCKET_DOWN,				-1, -1
  },
  {
    O_BOMB,					TRUE,
    EL_BDX_BOMB,				-1, -1
  },
  {
    O_BOMB_TICK_1,				TRUE,
    EL_BDX_BOMB_TICKING_1,			-1, -1
  },
  {
    O_BOMB_TICK_1,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_BOMB_TICK_2,				TRUE,
    EL_BDX_BOMB_TICKING_2,			-1, -1
  },
  {
    O_BOMB_TICK_2,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_BOMB_TICK_3,				TRUE,
    EL_BDX_BOMB_TICKING_3,			-1, -1
  },
  {
    O_BOMB_TICK_3,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_BOMB_TICK_4,				TRUE,
    EL_BDX_BOMB_TICKING_4,			-1, -1
  },
  {
    O_BOMB_TICK_4,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_BOMB_TICK_5,				TRUE,
    EL_BDX_BOMB_TICKING_5,			-1, -1
  },
  {
    O_BOMB_TICK_5,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_BOMB_TICK_6,				TRUE,
    EL_BDX_BOMB_TICKING_6,			-1, -1
  },
  {
    O_BOMB_TICK_6,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_BOMB_TICK_7,				TRUE,
    EL_BDX_BOMB_TICKING_7,			-1, -1
  },
  {
    O_BOMB_TICK_7,				FALSE,
    EL_BDX_BOMB,				ACTION_ACTIVE, -1
  },
  {
    O_NITRO_PACK,				TRUE,
    EL_BDX_NITRO_PACK,				-1, -1
  },
  {
    O_NITRO_PACK_F,				TRUE,
    EL_BDX_NITRO_PACK_FALLING,			-1, -1
  },
  {
    O_NITRO_PACK_F,				FALSE,
    EL_BDX_NITRO_PACK,				ACTION_FALLING, -1
  },
  {
    O_PRE_CLOCK_1,				TRUE,
    EL_BDX_CLOCK_GROWING_1,			-1, -1
  },
  {
    O_PRE_CLOCK_1,				FALSE,
    EL_BDX_CLOCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_CLOCK_2,				TRUE,
    EL_BDX_CLOCK_GROWING_2,			-1, -1
  },
  {
    O_PRE_CLOCK_2,				FALSE,
    EL_BDX_CLOCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_CLOCK_3,				TRUE,
    EL_BDX_CLOCK_GROWING_3,			-1, -1
  },
  {
    O_PRE_CLOCK_3,				FALSE,
    EL_BDX_CLOCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_CLOCK_4,				TRUE,
    EL_BDX_CLOCK_GROWING_4,			-1, -1
  },
  {
    O_PRE_CLOCK_4,				FALSE,
    EL_BDX_CLOCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_DIA_1,				TRUE,
    EL_BDX_DIAMOND_GROWING_1,			-1, -1
  },
  {
    O_PRE_DIA_1,				FALSE,
    EL_BDX_DIAMOND,				ACTION_GROWING, -1
  },
  {
    O_PRE_DIA_2,				TRUE,
    EL_BDX_DIAMOND_GROWING_2,			-1, -1
  },
  {
    O_PRE_DIA_2,				FALSE,
    EL_BDX_DIAMOND,				ACTION_GROWING, -1
  },
  {
    O_PRE_DIA_3,				TRUE,
    EL_BDX_DIAMOND_GROWING_3,			-1, -1
  },
  {
    O_PRE_DIA_3,				FALSE,
    EL_BDX_DIAMOND,				ACTION_GROWING, -1
  },
  {
    O_PRE_DIA_4,				TRUE,
    EL_BDX_DIAMOND_GROWING_4,			-1, -1
  },
  {
    O_PRE_DIA_4,				FALSE,
    EL_BDX_DIAMOND,				ACTION_GROWING, -1
  },
  {
    O_PRE_DIA_5,				TRUE,
    EL_BDX_DIAMOND_GROWING_5,			-1, -1
  },
  {
    O_PRE_DIA_5,				FALSE,
    EL_BDX_DIAMOND,				ACTION_GROWING, -1
  },
  {
    O_EXPLODE_1,				TRUE,
    EL_BDX_EXPLODING_1,				-1, -1
  },
  {
    O_EXPLODE_1,				FALSE,
    EL_BDX_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    O_EXPLODE_2,				TRUE,
    EL_BDX_EXPLODING_2,				-1, -1
  },
  {
    O_EXPLODE_2,				FALSE,
    EL_BDX_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    O_EXPLODE_3,				TRUE,
    EL_BDX_EXPLODING_3,				-1, -1
  },
  {
    O_EXPLODE_3,				FALSE,
    EL_BDX_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    O_EXPLODE_4,				TRUE,
    EL_BDX_EXPLODING_4,				-1, -1
  },
  {
    O_EXPLODE_4,				FALSE,
    EL_BDX_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    O_EXPLODE_5,				TRUE,
    EL_BDX_EXPLODING_5,				-1, -1
  },
  {
    O_EXPLODE_5,				FALSE,
    EL_BDX_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    O_PRE_STONE_1,				TRUE,
    EL_BDX_ROCK_GROWING_1,			-1, -1
  },
  {
    O_PRE_STONE_1,				FALSE,
    EL_BDX_ROCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_STONE_2,				TRUE,
    EL_BDX_ROCK_GROWING_2,			-1, -1
  },
  {
    O_PRE_STONE_2,				FALSE,
    EL_BDX_ROCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_STONE_3,				TRUE,
    EL_BDX_ROCK_GROWING_3,			-1, -1
  },
  {
    O_PRE_STONE_3,				FALSE,
    EL_BDX_ROCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_STONE_4,				TRUE,
    EL_BDX_ROCK_GROWING_4,			-1, -1
  },
  {
    O_PRE_STONE_4,				FALSE,
    EL_BDX_ROCK,				ACTION_GROWING, -1
  },
  {
    O_PRE_STEEL_1,				TRUE,
    EL_BDX_STEELWALL_GROWING_1,			-1, -1
  },
  {
    O_PRE_STEEL_1,				FALSE,
    EL_BDX_STEELWALL,				ACTION_GROWING, -1
  },
  {
    O_PRE_STEEL_2,				TRUE,
    EL_BDX_STEELWALL_GROWING_2,			-1, -1
  },
  {
    O_PRE_STEEL_2,				FALSE,
    EL_BDX_STEELWALL,				ACTION_GROWING, -1
  },
  {
    O_PRE_STEEL_3,				TRUE,
    EL_BDX_STEELWALL_GROWING_3,			-1, -1
  },
  {
    O_PRE_STEEL_3,				FALSE,
    EL_BDX_STEELWALL,				ACTION_GROWING, -1
  },
  {
    O_PRE_STEEL_4,				TRUE,
    EL_BDX_STEELWALL_GROWING_4,			-1, -1
  },
  {
    O_PRE_STEEL_4,				FALSE,
    EL_BDX_STEELWALL,				ACTION_GROWING, -1
  },
  {
    O_GHOST_EXPL_1,				TRUE,
    EL_BDX_GHOST_EXPLODING_1,			-1, -1
  },
  {
    O_GHOST_EXPL_1,				FALSE,
    EL_BDX_GHOST,				ACTION_EXPLODING, -1
  },
  {
    O_GHOST_EXPL_2,				TRUE,
    EL_BDX_GHOST_EXPLODING_2,			-1, -1
  },
  {
    O_GHOST_EXPL_2,				FALSE,
    EL_BDX_GHOST,				ACTION_EXPLODING, -1
  },
  {
    O_GHOST_EXPL_3,				TRUE,
    EL_BDX_GHOST_EXPLODING_3,			-1, -1
  },
  {
    O_GHOST_EXPL_3,				FALSE,
    EL_BDX_GHOST,				ACTION_EXPLODING, -1
  },
  {
    O_GHOST_EXPL_4,				TRUE,
    EL_BDX_GHOST_EXPLODING_4,			-1, -1
  },
  {
    O_GHOST_EXPL_4,				FALSE,
    EL_BDX_GHOST,				ACTION_EXPLODING, -1
  },
  {
    O_BOMB_EXPL_1,				TRUE,
    EL_BDX_BOMB_EXPLODING_1,			-1, -1
  },
  {
    O_BOMB_EXPL_1,				FALSE,
    EL_BDX_BOMB,				ACTION_EXPLODING, -1
  },
  {
    O_BOMB_EXPL_2,				TRUE,
    EL_BDX_BOMB_EXPLODING_2,			-1, -1
  },
  {
    O_BOMB_EXPL_2,				FALSE,
    EL_BDX_BOMB,				ACTION_EXPLODING, -1
  },
  {
    O_BOMB_EXPL_3,				TRUE,
    EL_BDX_BOMB_EXPLODING_3,			-1, -1
  },
  {
    O_BOMB_EXPL_3,				FALSE,
    EL_BDX_BOMB,				ACTION_EXPLODING, -1
  },
  {
    O_BOMB_EXPL_4,				TRUE,
    EL_BDX_BOMB_EXPLODING_4,			-1, -1
  },
  {
    O_BOMB_EXPL_4,				FALSE,
    EL_BDX_BOMB,				ACTION_EXPLODING, -1
  },
  {
    O_NITRO_EXPL_1,				TRUE,
    EL_BDX_NITRO_PACK_EXPLODING_1,		-1, -1
  },
  {
    O_NITRO_EXPL_1,				FALSE,
    EL_BDX_NITRO_PACK,				ACTION_EXPLODING, -1
  },
  {
    O_NITRO_EXPL_2,				TRUE,
    EL_BDX_NITRO_PACK_EXPLODING_2,		-1, -1
  },
  {
    O_NITRO_EXPL_2,				FALSE,
    EL_BDX_NITRO_PACK,				ACTION_EXPLODING, -1
  },
  {
    O_NITRO_EXPL_3,				TRUE,
    EL_BDX_NITRO_PACK_EXPLODING_3,		-1, -1
  },
  {
    O_NITRO_EXPL_3,				FALSE,
    EL_BDX_NITRO_PACK,				ACTION_EXPLODING, -1
  },
  {
    O_NITRO_EXPL_4,				TRUE,
    EL_BDX_NITRO_PACK_EXPLODING_4,		-1, -1
  },
  {
    O_NITRO_EXPL_4,				FALSE,
    EL_BDX_NITRO_PACK,				ACTION_EXPLODING, -1
  },
  {
    O_NITRO_PACK_EXPLODE,			TRUE,
    EL_BDX_NITRO_PACK_EXPLODING,		-1, -1
  },
  {
    O_NITRO_PACK_EXPLODE,			FALSE,
    EL_BDX_NITRO_PACK,				ACTION_EXPLODING, -1
  },
  {
    O_AMOEBA_2_EXPL_1,				TRUE,
    EL_BDX_AMOEBA_2_EXPLODING_1,		-1, -1
  },
  {
    O_AMOEBA_2_EXPL_1,				FALSE,
    EL_BDX_AMOEBA_2,				ACTION_EXPLODING, -1
  },
  {
    O_AMOEBA_2_EXPL_2,				TRUE,
    EL_BDX_AMOEBA_2_EXPLODING_2,		-1, -1
  },
  {
    O_AMOEBA_2_EXPL_2,				FALSE,
    EL_BDX_AMOEBA_2,				ACTION_EXPLODING, -1
  },
  {
    O_AMOEBA_2_EXPL_3,				TRUE,
    EL_BDX_AMOEBA_2_EXPLODING_3,		-1, -1
  },
  {
    O_AMOEBA_2_EXPL_3,				FALSE,
    EL_BDX_AMOEBA_2,				ACTION_EXPLODING, -1
  },
  {
    O_AMOEBA_2_EXPL_4,				TRUE,
    EL_BDX_AMOEBA_2_EXPLODING_4,		-1, -1
  },
  {
    O_AMOEBA_2_EXPL_4,				FALSE,
    EL_BDX_AMOEBA_2,				ACTION_EXPLODING, -1
  },
  {
    O_NUT_CRACK_1,				TRUE,
    EL_BDX_NUT_BREAKING_1,			-1, -1
  },
  {
    O_NUT_CRACK_1,				FALSE,
    EL_BDX_NUT,					ACTION_BREAKING, -1
  },
  {
    O_NUT_CRACK_2,				TRUE,
    EL_BDX_NUT_BREAKING_2,			-1, -1
  },
  {
    O_NUT_CRACK_2,				FALSE,
    EL_BDX_NUT,					ACTION_BREAKING, -1
  },
  {
    O_NUT_CRACK_3,				TRUE,
    EL_BDX_NUT_BREAKING_3,			-1, -1
  },
  {
    O_NUT_CRACK_3,				FALSE,
    EL_BDX_NUT,					ACTION_BREAKING, -1
  },
  {
    O_NUT_CRACK_4,				TRUE,
    EL_BDX_NUT_BREAKING_4,			-1, -1
  },
  {
    O_NUT_CRACK_4,				FALSE,
    EL_BDX_NUT,					ACTION_BREAKING, -1
  },
  {
    O_PLAYER_PNEUMATIC_LEFT,			FALSE,
    EL_BDX_PLAYER,				ACTION_HITTING, MV_BIT_LEFT
  },
  {
    O_PLAYER_PNEUMATIC_RIGHT,			FALSE,
    EL_BDX_PLAYER,				ACTION_HITTING, MV_BIT_RIGHT
  },
  {
    O_PNEUMATIC_ACTIVE_LEFT,			FALSE,
    EL_BDX_PNEUMATIC_HAMMER,			ACTION_HITTING, MV_BIT_LEFT
  },
  {
    O_PNEUMATIC_ACTIVE_RIGHT,			FALSE,
    EL_BDX_PNEUMATIC_HAMMER,			ACTION_HITTING, MV_BIT_RIGHT
  },

  // scanned (runtime) elements

  {
    O_DIRT_BALL_scanned,			TRUE,
    EL_BDX_GRASS_BALL_SCANNED,			-1, -1
  },
  {
    O_DIRT_BALL_F_scanned,			TRUE,
    EL_BDX_GRASS_BALL_FALLING_SCANNED,		-1, -1
  },
  {
    O_DIRT_LOOSE_scanned,			TRUE,
    EL_BDX_GRASS_LOOSE_SCANNED,			-1, -1
  },
  {
    O_DIRT_LOOSE_F_scanned,			TRUE,
    EL_BDX_GRASS_LOOSE_FALLING_SCANNED,		-1, -1
  },
  {
    O_STONE_scanned,				TRUE,
    EL_BDX_ROCK_SCANNED,			-1, -1
  },
  {
    O_STONE_F_scanned,				TRUE,
    EL_BDX_ROCK_FALLING_SCANNED,		-1, -1
  },
  {
    O_FLYING_STONE_scanned,			TRUE,
    EL_BDX_FLYING_ROCK_SCANNED,			-1, -1
  },
  {
    O_FLYING_STONE_F_scanned,			TRUE,
    EL_BDX_FLYING_ROCK_FLYING_SCANNED,		-1, -1
  },
  {
    O_MEGA_STONE_scanned,			TRUE,
    EL_BDX_HEAVY_ROCK_SCANNED,			-1, -1
  },
  {
    O_MEGA_STONE_F_scanned,			TRUE,
    EL_BDX_HEAVY_ROCK_FALLING_SCANNED,		-1, -1
  },
  {
    O_LIGHT_STONE_scanned,			TRUE,
    EL_BDX_LIGHT_ROCK_SCANNED,			-1, -1
  },
  {
    O_LIGHT_STONE_F_scanned,			TRUE,
    EL_BDX_LIGHT_ROCK_FALLING_SCANNED,		-1, -1
  },
  {
    O_DIAMOND_scanned,				TRUE,
    EL_BDX_DIAMOND_SCANNED,			-1, -1
  },
  {
    O_DIAMOND_F_scanned,			TRUE,
    EL_BDX_DIAMOND_FALLING_SCANNED,		-1, -1
  },
  {
    O_FLYING_DIAMOND_scanned,			TRUE,
    EL_BDX_FLYING_DIAMOND_SCANNED,		-1, -1
  },
  {
    O_FLYING_DIAMOND_F_scanned,			TRUE,
    EL_BDX_FLYING_DIAMOND_FLYING_SCANNED,	-1, -1
  },
  {
    O_NUT_scanned,				TRUE,
    EL_BDX_NUT_SCANNED,				-1, -1
  },
  {
    O_NUT_F_scanned,				TRUE,
    EL_BDX_NUT_FALLING_SCANNED,			-1, -1
  },
  {
    O_H_EXPANDING_WALL_scanned,			TRUE,
    EL_BDX_EXPANDABLE_WALL_HORIZONTAL_SCANNED,	-1, -1
  },
  {
    O_V_EXPANDING_WALL_scanned,			TRUE,
    EL_BDX_EXPANDABLE_WALL_VERTICAL_SCANNED,	-1, -1
  },
  {
    O_EXPANDING_WALL_scanned,			TRUE,
    EL_BDX_EXPANDABLE_WALL_ANY_SCANNED,		-1, -1
  },
  {
    O_H_EXPANDING_STEEL_WALL_scanned,			TRUE,
    EL_BDX_EXPANDABLE_STEELWALL_HORIZONTAL_SCANNED,	-1, -1
  },
  {
    O_V_EXPANDING_STEEL_WALL_scanned,			TRUE,
    EL_BDX_EXPANDABLE_STEELWALL_VERTICAL_SCANNED,	-1, -1
  },
  {
    O_EXPANDING_STEEL_WALL_scanned,		TRUE,
    EL_BDX_EXPANDABLE_STEELWALL_ANY_SCANNED,	-1, -1
  },
  {
    O_ACID_scanned,				TRUE,
    EL_BDX_ACID_SCANNED,			-1, -1
  },
  {
    O_FALLING_WALL_F_scanned,			TRUE,
    EL_BDX_FALLING_WALL_FALLING_SCANNED,	-1, -1
  },
  {
    O_COW_1_scanned,				TRUE,
    EL_BDX_COW_LEFT_SCANNED,			-1, -1
  },
  {
    O_COW_2_scanned,				TRUE,
    EL_BDX_COW_UP_SCANNED,			-1, -1
  },
  {
    O_COW_3_scanned,				TRUE,
    EL_BDX_COW_RIGHT_SCANNED,			-1, -1
  },
  {
    O_COW_4_scanned,				TRUE,
    EL_BDX_COW_DOWN_SCANNED,			-1, -1
  },
  {
    O_AMOEBA_scanned,				TRUE,
    EL_BDX_AMOEBA_1_SCANNED,			-1, -1
  },
  {
    O_AMOEBA_2_scanned,				TRUE,
    EL_BDX_AMOEBA_2_SCANNED,			-1, -1
  },
  {
    O_WAITING_STONE_scanned,			TRUE,
    EL_BDX_WAITING_ROCK_SCANNED,		-1, -1
  },
  {
    O_CHASING_STONE_scanned,			TRUE,
    EL_BDX_CHASING_ROCK_SCANNED,		-1, -1
  },
  {
    O_GHOST_scanned,				TRUE,
    EL_BDX_GHOST_SCANNED,			-1, -1
  },
  {
    O_FIREFLY_1_scanned,			TRUE,
    EL_BDX_FIREFLY_1_LEFT_SCANNED,		-1, -1
  },
  {
    O_FIREFLY_2_scanned,			TRUE,
    EL_BDX_FIREFLY_1_UP_SCANNED,		-1, -1
  },
  {
    O_FIREFLY_3_scanned,			TRUE,
    EL_BDX_FIREFLY_1_RIGHT_SCANNED,		-1, -1
  },
  {
    O_FIREFLY_4_scanned,			TRUE,
    EL_BDX_FIREFLY_1_DOWN_SCANNED,		-1, -1
  },
  {
    O_ALT_FIREFLY_1_scanned,			TRUE,
    EL_BDX_FIREFLY_2_LEFT_SCANNED,		-1, -1
  },
  {
    O_ALT_FIREFLY_2_scanned,			TRUE,
    EL_BDX_FIREFLY_2_UP_SCANNED,		-1, -1
  },
  {
    O_ALT_FIREFLY_3_scanned,			TRUE,
    EL_BDX_FIREFLY_2_RIGHT_SCANNED,		-1, -1
  },
  {
    O_ALT_FIREFLY_4_scanned,			TRUE,
    EL_BDX_FIREFLY_2_DOWN_SCANNED,		-1, -1
  },
  {
    O_BUTTER_1_scanned,				TRUE,
    EL_BDX_BUTTERFLY_1_LEFT_SCANNED,		-1, -1
  },
  {
    O_BUTTER_2_scanned,				TRUE,
    EL_BDX_BUTTERFLY_1_UP_SCANNED,		-1, -1
  },
  {
    O_BUTTER_3_scanned,				TRUE,
    EL_BDX_BUTTERFLY_1_RIGHT_SCANNED,		-1, -1
  },
  {
    O_BUTTER_4_scanned,				TRUE,
    EL_BDX_BUTTERFLY_1_DOWN_SCANNED,		-1, -1
  },
  {
    O_ALT_BUTTER_1_scanned,			TRUE,
    EL_BDX_BUTTERFLY_2_LEFT_SCANNED,		-1, -1
  },
  {
    O_ALT_BUTTER_2_scanned,			TRUE,
    EL_BDX_BUTTERFLY_2_UP_SCANNED,		-1, -1
  },
  {
    O_ALT_BUTTER_3_scanned,			TRUE,
    EL_BDX_BUTTERFLY_2_RIGHT_SCANNED,		-1, -1
  },
  {
    O_ALT_BUTTER_4_scanned,			TRUE,
    EL_BDX_BUTTERFLY_2_DOWN_SCANNED,		-1, -1
  },
  {
    O_STONEFLY_1_scanned,			TRUE,
    EL_BDX_STONEFLY_LEFT_SCANNED,		-1, -1
  },
  {
    O_STONEFLY_2_scanned,			TRUE,
    EL_BDX_STONEFLY_UP_SCANNED,			-1, -1
  },
  {
    O_STONEFLY_3_scanned,			TRUE,
    EL_BDX_STONEFLY_RIGHT_SCANNED,		-1, -1
  },
  {
    O_STONEFLY_4_scanned,			TRUE,
    EL_BDX_STONEFLY_DOWN_SCANNED,		-1, -1
  },
  {
    O_BITER_1_scanned,				TRUE,
    EL_BDX_BITER_UP_SCANNED,			-1, -1
  },
  {
    O_BITER_2_scanned,				TRUE,
    EL_BDX_BITER_RIGHT_SCANNED,			-1, -1
  },
  {
    O_BITER_3_scanned,				TRUE,
    EL_BDX_BITER_DOWN_SCANNED,			-1, -1
  },
  {
    O_BITER_4_scanned,				TRUE,
    EL_BDX_BITER_LEFT_SCANNED,			-1, -1
  },
  {
    O_DRAGONFLY_1_scanned,			TRUE,
    EL_BDX_DRAGONFLY_LEFT_SCANNED,		-1, -1
  },
  {
    O_DRAGONFLY_2_scanned,			TRUE,
    EL_BDX_DRAGONFLY_UP_SCANNED,		-1, -1
  },
  {
    O_DRAGONFLY_3_scanned,			TRUE,
    EL_BDX_DRAGONFLY_RIGHT_SCANNED,		-1, -1
  },
  {
    O_DRAGONFLY_4_scanned,			TRUE,
    EL_BDX_DRAGONFLY_DOWN_SCANNED,		-1, -1
  },
  {
    O_PLAYER_scanned,				TRUE,
    EL_BDX_PLAYER_SCANNED,			-1, -1
  },
  {
    O_PLAYER_BOMB_scanned,			TRUE,
    EL_BDX_PLAYER_WITH_BOMB_SCANNED,		-1, -1
  },
  {
    O_PLAYER_ROCKET_LAUNCHER_scanned,		TRUE,
    EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER_SCANNED,	-1, -1
  },
  {
    O_ROCKET_1_scanned,				TRUE,
    EL_BDX_ROCKET_RIGHT_SCANNED,		-1, -1
  },
  {
    O_ROCKET_2_scanned,				TRUE,
    EL_BDX_ROCKET_UP_SCANNED,			-1, -1
  },
  {
    O_ROCKET_3_scanned,				TRUE,
    EL_BDX_ROCKET_LEFT_SCANNED,			-1, -1
  },
  {
    O_ROCKET_4_scanned,				TRUE,
    EL_BDX_ROCKET_DOWN_SCANNED,			-1, -1
  },
  {
    O_NITRO_PACK_scanned,			TRUE,
    EL_BDX_NITRO_PACK_SCANNED,			-1, -1
  },
  {
    O_NITRO_PACK_F_scanned,			TRUE,
    EL_BDX_NITRO_PACK_FALLING_SCANNED,		-1, -1
  },
  {
    O_NITRO_PACK_EXPLODE_scanned,		TRUE,
    EL_BDX_NITRO_PACK_EXPLODING_SCANNED,	-1, -1
  },
  {
    O_PRE_CLOCK_0,				TRUE,
    EL_BDX_CLOCK_GROWING_0,			-1, -1
  },
  {
    O_PRE_DIA_0,				TRUE,
    EL_BDX_DIAMOND_GROWING_0,			-1, -1
  },
  {
    O_EXPLODE_0,				TRUE,
    EL_BDX_EXPLODING_0,				-1, -1
  },
  {
    O_PRE_STONE_0,				TRUE,
    EL_BDX_ROCK_GROWING_0,			-1, -1
  },
  {
    O_PRE_STEEL_0,				TRUE,
    EL_BDX_STEELWALL_GROWING_0,			-1, -1
  },
  {
    O_GHOST_EXPL_0,				TRUE,
    EL_BDX_GHOST_EXPLODING_0,			-1, -1
  },
  {
    O_BOMB_EXPL_0,				TRUE,
    EL_BDX_BOMB_EXPLODING_0,			-1, -1
  },
  {
    O_NITRO_EXPL_0,				TRUE,
    EL_BDX_NITRO_PACK_EXPLODING_0,		-1, -1
  },
  {
    O_AMOEBA_2_EXPL_0,				TRUE,
    EL_BDX_AMOEBA_2_EXPLODING_0,		-1, -1
  },
  {
    O_NUT_CRACK_0,				TRUE,
    EL_BDX_NUT_BREAKING_0,			-1, -1
  },

  // helper (runtime) elements

  {
    O_FAKE_BONUS,				FALSE,
    EL_BDX_FAKE_BONUS,				-1, -1
  },
  {
    O_INBOX_CLOSED,				FALSE,
    EL_BDX_INBOX,				-1, -1
  },
  {
    O_INBOX_OPEN,				FALSE,
    EL_BDX_INBOX,				ACTION_OPENING, -1
  },
  {
    O_OUTBOX_CLOSED,				FALSE,
    EL_BDX_EXIT_CLOSED,				-1, -1
  },
  {
    O_OUTBOX_OPEN,				FALSE,
    EL_BDX_EXIT_OPEN,				-1, -1
  },
  {
    O_COVERED,					FALSE,
    EL_BDX_COVERED,				-1, -1
  },
  {
    O_PLAYER_LEFT,				FALSE,
    EL_BDX_PLAYER,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    O_PLAYER_RIGHT,				FALSE,
    EL_BDX_PLAYER,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    O_PLAYER_UP,				FALSE,
    EL_BDX_PLAYER,				ACTION_MOVING, MV_BIT_UP
  },
  {
    O_PLAYER_DOWN,				FALSE,
    EL_BDX_PLAYER,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    O_PLAYER_BLINK,				FALSE,
    EL_BDX_PLAYER,				ACTION_BORING_1, -1
  },
  {
    O_PLAYER_TAP,				FALSE,
    EL_BDX_PLAYER,				ACTION_BORING_2, -1
  },
  {
    O_PLAYER_TAP_BLINK,				FALSE,
    EL_BDX_PLAYER,				ACTION_BORING_3, -1
  },
  {
    O_PLAYER_PUSH_LEFT,				FALSE,
    EL_BDX_PLAYER,				ACTION_PUSHING, MV_BIT_LEFT
  },
  {
    O_PLAYER_PUSH_RIGHT,			FALSE,
    EL_BDX_PLAYER,				ACTION_PUSHING, MV_BIT_RIGHT
  },
  {
    O_PLAYER_BOMB_TURNING,			FALSE,
    EL_BDX_PLAYER_WITH_BOMB,			ACTION_TURNING, -1
  },
  {
    O_PLAYER_ROCKET_LAUNCHER_TURNING,		FALSE,
    EL_BDX_PLAYER_WITH_ROCKET_LAUNCHER,		ACTION_TURNING, -1
  },
  {
    O_CREATURE_SWITCH_ON,			FALSE,
    EL_BDX_CREATURE_SWITCH_ACTIVE,		-1, -1
  },
  {
    O_EXPANDING_WALL_SWITCH_HORIZ,		FALSE,
    EL_BDX_EXPANDABLE_WALL_SWITCH,		-1, -1
  },
  {
    O_EXPANDING_WALL_SWITCH_VERT,		FALSE,
    EL_BDX_EXPANDABLE_WALL_SWITCH_ACTIVE,	-1, -1
  },
  {
    O_GRAVITY_SWITCH_ACTIVE,			FALSE,
    EL_BDX_GRAVITY_SWITCH_ACTIVE,		-1, -1
  },
  {
    O_REPLICATOR_SWITCH_OFF,			FALSE,
    EL_BDX_REPLICATOR_SWITCH,			-1, -1
  },
  {
    O_REPLICATOR_SWITCH_ON,			FALSE,
    EL_BDX_REPLICATOR_SWITCH_ACTIVE,		-1, -1
  },
  {
    O_CONVEYOR_DIR_NORMAL,			FALSE,
    EL_BDX_CONVEYOR_DIR_SWITCH,			-1, -1
  },
  {
    O_CONVEYOR_DIR_CHANGED,			FALSE,
    EL_BDX_CONVEYOR_DIR_SWITCH_ACTIVE,		-1, -1
  },
  {
    O_CONVEYOR_SWITCH_OFF,			FALSE,
    EL_BDX_CONVEYOR_SWITCH,			-1, -1
  },
  {
    O_CONVEYOR_SWITCH_ON,			FALSE,
    EL_BDX_CONVEYOR_SWITCH_ACTIVE,		-1, -1
  },
  {
    O_MAGIC_WALL_ACTIVE,			FALSE,
    EL_BDX_MAGIC_WALL_ACTIVE,			-1, -1
  },
  {
    O_REPLICATOR_ACTIVE,			FALSE,
    EL_BDX_REPLICATOR_ACTIVE,			-1, -1
  },
  {
    O_CONVEYOR_LEFT_ACTIVE,			FALSE,
    EL_BDX_CONVEYOR_LEFT_ACTIVE,		-1, -1
  },
  {
    O_CONVEYOR_RIGHT_ACTIVE,			FALSE,
    EL_BDX_CONVEYOR_RIGHT_ACTIVE,		-1, -1
  },
  {
    O_BITER_SWITCH_1,				FALSE,
    EL_BDX_BITER_SWITCH_1,			-1, -1
  },
  {
    O_BITER_SWITCH_2,				FALSE,
    EL_BDX_BITER_SWITCH_2,			-1, -1
  },
  {
    O_BITER_SWITCH_3,				FALSE,
    EL_BDX_BITER_SWITCH_3,			-1, -1
  },
  {
    O_BITER_SWITCH_4,				FALSE,
    EL_BDX_BITER_SWITCH_4,			-1, -1
  },
  {
    O_BITER_SWITCH_1_CRUMBLED,			FALSE,
    EL_BDX_BITER_SWITCH_1,			-1, -1
  },
  {
    O_BITER_SWITCH_2_CRUMBLED,			FALSE,
    EL_BDX_BITER_SWITCH_2,			-1, -1
  },
  {
    O_BITER_SWITCH_3_CRUMBLED,			FALSE,
    EL_BDX_BITER_SWITCH_3,			-1, -1
  },
  {
    O_BITER_SWITCH_4_CRUMBLED,			FALSE,
    EL_BDX_BITER_SWITCH_4,			-1, -1
  },
  {
    O_STONE_MOVE_LEFT,				FALSE,
    EL_BDX_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    O_STONE_MOVE_RIGHT,				FALSE,
    EL_BDX_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    O_STONE_PUSH_LEFT,				FALSE,
    EL_BDX_ROCK,				ACTION_PUSHING, MV_BIT_LEFT
  },
  {
    O_STONE_PUSH_RIGHT,				FALSE,
    EL_BDX_ROCK,				ACTION_PUSHING, MV_BIT_RIGHT
  },

  {
    -1,						FALSE,
    -1,						-1, -1
  }
};

int map_element_RND_to_BD_cave(int element_rnd)
{
  static unsigned short mapping_RND_to_BD[NUM_FILE_ELEMENTS];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    // return "O_UNKNOWN" for all undefined elements in mapping array
    for (i = 0; i < NUM_FILE_ELEMENTS; i++)
      mapping_RND_to_BD[i] = O_UNKNOWN;

    for (i = 0; bd_object_mapping_list[i].element_bd != -1; i++)
      if (bd_object_mapping_list[i].is_rnd_to_bd_mapping)
	mapping_RND_to_BD[bd_object_mapping_list[i].element_rnd] =
	  bd_object_mapping_list[i].element_bd;

    mapping_initialized = TRUE;
  }

  if (element_rnd < 0 || element_rnd >= NUM_FILE_ELEMENTS)
  {
    Warn("invalid RND element %d", element_rnd);

    return O_UNKNOWN;
  }

  return mapping_RND_to_BD[element_rnd];
}

int map_element_RND_to_BD_effect(int element_rnd, int action)
{
  static unsigned short mapping_RND_to_BD[NUM_FILE_ELEMENTS][NUM_ACTIONS];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i, j;

    // return "O_UNKNOWN" for all undefined elements in mapping array
    for (i = 0; i < NUM_FILE_ELEMENTS; i++)
      for (j = 0; j < NUM_ACTIONS; j++)
	mapping_RND_to_BD[i][j] = O_UNKNOWN;

    for (i = 0; bd_object_mapping_list[i].element_bd != -1; i++)
    {
      int element_rnd = bd_object_mapping_list[i].element_rnd;
      int element_bd  = bd_object_mapping_list[i].element_bd;
      int action      = bd_object_mapping_list[i].action;

      if (action != -1)
	mapping_RND_to_BD[element_rnd][action] = element_bd;
    }

    mapping_initialized = TRUE;
  }

  if (element_rnd < 0 || element_rnd >= NUM_FILE_ELEMENTS)
  {
    Warn("invalid RND element %d", element_rnd);

    return O_UNKNOWN;
  }

  if (action < 0 || action >= NUM_ACTIONS)
  {
    Warn("invalid action %d", action);

    return O_UNKNOWN;
  }

  return mapping_RND_to_BD[element_rnd][action];
}

int map_element_BD_to_RND_cave(int element_bd)
{
  static unsigned short mapping_BD_to_RND[O_MAX_ALL];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    // return "EL_UNKNOWN" for all undefined elements in mapping array
    for (i = 0; i < O_MAX_ALL; i++)
      mapping_BD_to_RND[i] = EL_UNKNOWN;

    for (i = 0; bd_object_mapping_list[i].element_bd != -1; i++)
      if (bd_object_mapping_list[i].is_rnd_to_bd_mapping)
	mapping_BD_to_RND[bd_object_mapping_list[i].element_bd] =
	  bd_object_mapping_list[i].element_rnd;

    mapping_initialized = TRUE;
  }

  if (element_bd < 0 || element_bd >= O_MAX_ALL)
  {
    Warn("invalid BD element %d", element_bd);

    return EL_UNKNOWN;
  }

  return mapping_BD_to_RND[element_bd];
}

int map_element_BD_to_RND_game(int element_bd)
{
  static unsigned short mapping_BD_to_RND[O_MAX_ALL];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    // return "EL_UNKNOWN" for all undefined elements in mapping array
    for (i = 0; i < O_MAX_ALL; i++)
      mapping_BD_to_RND[i] = EL_UNKNOWN;

    for (i = 0; bd_object_mapping_list[i].element_bd != -1; i++)
      mapping_BD_to_RND[bd_object_mapping_list[i].element_bd] =
	bd_object_mapping_list[i].element_rnd;

    mapping_initialized = TRUE;
  }

  if (element_bd < 0 || element_bd >= O_MAX_ALL)
  {
    Warn("invalid BD element %d", element_bd);

    return EL_UNKNOWN;
  }

  return mapping_BD_to_RND[element_bd];
}

static struct Mapping_EM_to_RND_object
{
  int element_em;
  boolean is_rnd_to_em_mapping;		// unique mapping EM <-> RND
  boolean is_backside;			// backside of moving element

  int element_rnd;
  int action;
  int direction;
}
em_object_mapping_list[GAME_TILE_MAX + 1] =
{
  {
    Zborder,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zplayer,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },

  {
    Zbug,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Ztank,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zeater,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zdynamite,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zboom,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },

  {
    Xchain,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    Xboom_bug,				FALSE,	FALSE,
    EL_BUG,				ACTION_EXPLODING, -1
  },
  {
    Xboom_tank,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_EXPLODING, -1
  },
  {
    Xboom_android,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_OTHER, -1
  },
  {
    Xboom_1,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    Xboom_2,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },

  {
    Xblank,				TRUE,	FALSE,
    EL_EMPTY,				-1, -1
  },

  {
    Xsplash_e,				FALSE,	FALSE,
    EL_ACID_SPLASH_RIGHT,		-1, -1
  },
  {
    Xsplash_w,				FALSE,	FALSE,
    EL_ACID_SPLASH_LEFT,		-1, -1
  },

  {
    Xplant,				TRUE,	FALSE,
    EL_EMC_PLANT,			-1, -1
  },
  {
    Yplant,				FALSE,	FALSE,
    EL_EMC_PLANT,			-1, -1
  },

  {
    Xacid_1,				TRUE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_2,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_3,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_4,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_5,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_6,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_7,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_8,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },

  {
    Xfake_acid_1,			TRUE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_2,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_3,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_4,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_5,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_6,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_7,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_8,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },

  {
    Xfake_acid_1_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_2_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_3_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_4_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_5_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_6_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_7_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_8_player,		FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },

  {
    Xgrass,				TRUE,	FALSE,
    EL_EMC_GRASS,			-1, -1
  },
  {
    Ygrass_nB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_UP
  },
  {
    Ygrass_eB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    Ygrass_sB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    Ygrass_wB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_LEFT
  },

  {
    Xdirt,				TRUE,	FALSE,
    EL_SAND,				-1, -1
  },
  {
    Ydirt_nB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    Ydirt_eB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    Ydirt_sB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    Ydirt_wB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_LEFT
  },

  {
    Xandroid,				TRUE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, -1
  },
  {
    Xandroid_1_n,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_UP
  },
  {
    Xandroid_2_n,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_UP
  },
  {
    Xandroid_1_e,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_RIGHT
  },
  {
    Xandroid_2_e,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_RIGHT
  },
  {
    Xandroid_1_w,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_LEFT
  },
  {
    Xandroid_2_w,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_LEFT
  },
  {
    Xandroid_1_s,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_DOWN
  },
  {
    Xandroid_2_s,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_DOWN
  },
  {
    Yandroid_n,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Yandroid_nB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Yandroid_ne,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_UPRIGHT
  },
  {
    Yandroid_neB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_UPRIGHT
  },
  {
    Yandroid_e,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yandroid_eB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yandroid_se,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_DOWNRIGHT
  },
  {
    Yandroid_seB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_DOWNRIGHT
  },
  {
    Yandroid_s,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yandroid_sB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yandroid_sw,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_DOWNLEFT
  },
  {
    Yandroid_swB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_DOWNLEFT
  },
  {
    Yandroid_w,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yandroid_wB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yandroid_nw,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_UPLEFT
  },
  {
    Yandroid_nwB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_UPLEFT
  },

  {
    Xeater_n,				TRUE,	FALSE,
    EL_YAMYAM_UP,			-1, -1
  },
  {
    Xeater_e,				TRUE,	FALSE,
    EL_YAMYAM_RIGHT,			-1, -1
  },
  {
    Xeater_w,				TRUE,	FALSE,
    EL_YAMYAM_LEFT,			-1, -1
  },
  {
    Xeater_s,				TRUE,	FALSE,
    EL_YAMYAM_DOWN,			-1, -1
  },
  {
    Yeater_n,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yeater_nB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yeater_e,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yeater_eB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yeater_s,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yeater_sB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yeater_w,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yeater_wB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yeater_stone,			FALSE,	FALSE,
    EL_YAMYAM,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Yeater_spring,			FALSE,	FALSE,
    EL_YAMYAM,				ACTION_SMASHED_BY_SPRING, -1
  },

  {
    Xalien,				TRUE,	FALSE,
    EL_ROBOT,				-1, -1
  },
  {
    Xalien_pause,			FALSE,	FALSE,
    EL_ROBOT,				-1, -1
  },
  {
    Yalien_n,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yalien_nB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yalien_e,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yalien_eB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yalien_s,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yalien_sB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yalien_w,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yalien_wB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yalien_stone,			FALSE,	FALSE,
    EL_ROBOT,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Yalien_spring,			FALSE,	FALSE,
    EL_ROBOT,				ACTION_SMASHED_BY_SPRING, -1
  },

  {
    Xbug_1_n,				TRUE,	FALSE,
    EL_BUG_UP,				-1, -1
  },
  {
    Xbug_1_e,				TRUE,	FALSE,
    EL_BUG_RIGHT,			-1, -1
  },
  {
    Xbug_1_s,				TRUE,	FALSE,
    EL_BUG_DOWN,			-1, -1
  },
  {
    Xbug_1_w,				TRUE,	FALSE,
    EL_BUG_LEFT,			-1, -1
  },
  {
    Xbug_2_n,				FALSE,	FALSE,
    EL_BUG_UP,				-1, -1
  },
  {
    Xbug_2_e,				FALSE,	FALSE,
    EL_BUG_RIGHT,			-1, -1
  },
  {
    Xbug_2_s,				FALSE,	FALSE,
    EL_BUG_DOWN,			-1, -1
  },
  {
    Xbug_2_w,				FALSE,	FALSE,
    EL_BUG_LEFT,			-1, -1
  },
  {
    Ybug_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Ybug_nB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Ybug_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybug_eB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybug_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ybug_sB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ybug_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybug_wB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybug_w_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_LEFT, MV_BIT_UP
  },
  {
    Ybug_n_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_UP, MV_BIT_RIGHT
  },
  {
    Ybug_e_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_RIGHT, MV_BIT_DOWN
  },
  {
    Ybug_s_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_DOWN, MV_BIT_LEFT
  },
  {
    Ybug_e_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_RIGHT, MV_BIT_UP
  },
  {
    Ybug_s_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_DOWN, MV_BIT_RIGHT
  },
  {
    Ybug_w_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_LEFT, MV_BIT_DOWN
  },
  {
    Ybug_n_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_UP, MV_BIT_LEFT
  },
  {
    Ybug_stone,				FALSE,	FALSE,
    EL_BUG,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Ybug_spring,			FALSE,	FALSE,
    EL_BUG,				ACTION_SMASHED_BY_SPRING, -1
  },

  {
    Xtank_1_n,				TRUE,	FALSE,
    EL_SPACESHIP_UP,			-1, -1
  },
  {
    Xtank_1_e,				TRUE,	FALSE,
    EL_SPACESHIP_RIGHT,			-1, -1
  },
  {
    Xtank_1_s,				TRUE,	FALSE,
    EL_SPACESHIP_DOWN,			-1, -1
  },
  {
    Xtank_1_w,				TRUE,	FALSE,
    EL_SPACESHIP_LEFT,			-1, -1
  },
  {
    Xtank_2_n,				FALSE,	FALSE,
    EL_SPACESHIP_UP,			-1, -1
  },
  {
    Xtank_2_e,				FALSE,	FALSE,
    EL_SPACESHIP_RIGHT,			-1, -1
  },
  {
    Xtank_2_s,				FALSE,	FALSE,
    EL_SPACESHIP_DOWN,			-1, -1
  },
  {
    Xtank_2_w,				FALSE,	FALSE,
    EL_SPACESHIP_LEFT,			-1, -1
  },
  {
    Ytank_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Ytank_nB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Ytank_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ytank_eB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ytank_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ytank_sB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ytank_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ytank_wB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ytank_w_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_LEFT, MV_BIT_UP
  },
  {
    Ytank_n_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_UP, MV_BIT_RIGHT
  },
  {
    Ytank_e_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_RIGHT, MV_BIT_DOWN
  },
  {
    Ytank_s_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_DOWN, MV_BIT_LEFT
  },
  {
    Ytank_e_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_RIGHT, MV_BIT_UP
  },
  {
    Ytank_s_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_DOWN, MV_BIT_RIGHT
  },
  {
    Ytank_w_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_LEFT, MV_BIT_DOWN
  },
  {
    Ytank_n_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_UP, MV_BIT_LEFT
  },
  {
    Ytank_stone,			FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Ytank_spring,			FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_SMASHED_BY_SPRING, -1
  },

  {
    Xemerald,				TRUE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_pause,			FALSE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_fall,			FALSE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_shine,			FALSE,	FALSE,
    EL_EMERALD,				ACTION_TWINKLING, -1
  },
  {
    Yemerald_s,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_FALLING, -1
  },
  {
    Yemerald_sB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_FALLING, -1
  },
  {
    Yemerald_e,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yemerald_eB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yemerald_w,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yemerald_wB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yemerald_blank,			FALSE,	FALSE,
    EL_EMERALD,				ACTION_COLLECTING, -1
  },

  {
    Xdiamond,				TRUE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_pause,			FALSE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_fall,			FALSE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_shine,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_TWINKLING, -1
  },
  {
    Ydiamond_s,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_FALLING, -1
  },
  {
    Ydiamond_sB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_FALLING, -1
  },
  {
    Ydiamond_e,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ydiamond_eB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ydiamond_w,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ydiamond_wB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ydiamond_blank,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_COLLECTING, -1
  },
  {
    Ydiamond_stone,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_SMASHED_BY_ROCK, -1
  },

  {
    Xstone,				TRUE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Xstone_pause,			FALSE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Xstone_fall,			FALSE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Ystone_s,				FALSE,	FALSE,
    EL_ROCK,				ACTION_FALLING, -1
  },
  {
    Ystone_sB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_FALLING, -1
  },
  {
    Ystone_e,				FALSE,	FALSE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ystone_eB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ystone_w,				FALSE,	FALSE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ystone_wB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },

  {
    Xbomb,				TRUE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Xbomb_pause,			FALSE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Xbomb_fall,				FALSE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Ybomb_s,				FALSE,	FALSE,
    EL_BOMB,				ACTION_FALLING, -1
  },
  {
    Ybomb_sB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_FALLING, -1
  },
  {
    Ybomb_e,				FALSE,	FALSE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybomb_eB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybomb_w,				FALSE,	FALSE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybomb_wB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybomb_blank,			FALSE,	FALSE,
    EL_BOMB,				ACTION_ACTIVATING, -1
  },

  {
    Xnut,				TRUE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Xnut_pause,				FALSE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Xnut_fall,				FALSE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Ynut_s,				FALSE,	FALSE,
    EL_NUT,				ACTION_FALLING, -1
  },
  {
    Ynut_sB,				FALSE,	TRUE,
    EL_NUT,				ACTION_FALLING, -1
  },
  {
    Ynut_e,				FALSE,	FALSE,
    EL_NUT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ynut_eB,				FALSE,	TRUE,
    EL_NUT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ynut_w,				FALSE,	FALSE,
    EL_NUT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ynut_wB,				FALSE,	TRUE,
    EL_NUT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ynut_stone,				FALSE,	FALSE,
    EL_NUT,				ACTION_BREAKING, -1
  },

  {
    Xspring,				TRUE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_pause,			FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_e,				TRUE,	FALSE,
    EL_SPRING_RIGHT,			-1, -1
  },
  {
    Xspring_w,				TRUE,	FALSE,
    EL_SPRING_LEFT,			-1, -1
  },
  {
    Xspring_fall,			FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Yspring_s,				FALSE,	FALSE,
    EL_SPRING,				ACTION_FALLING, -1
  },
  {
    Yspring_sB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_FALLING, -1
  },
  {
    Yspring_e,				FALSE,	FALSE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yspring_eB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yspring_w,				FALSE,	FALSE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yspring_wB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yspring_alien_e,			FALSE,	FALSE,
    EL_SPRING,				ACTION_EATING, MV_BIT_RIGHT
  },
  {
    Yspring_alien_eB,			FALSE,	TRUE,
    EL_SPRING,				ACTION_EATING, MV_BIT_RIGHT
  },
  {
    Yspring_alien_w,			FALSE,	FALSE,
    EL_SPRING,				ACTION_EATING, MV_BIT_LEFT
  },
  {
    Yspring_alien_wB,			FALSE,	TRUE,
    EL_SPRING,				ACTION_EATING, MV_BIT_LEFT
  },

  {
    Xpush_emerald_e,			FALSE,	FALSE,
    EL_EMERALD,				-1, MV_BIT_RIGHT
  },
  {
    Xpush_emerald_w,			FALSE,	FALSE,
    EL_EMERALD,				-1, MV_BIT_LEFT
  },
  {
    Xpush_diamond_e,			FALSE,	FALSE,
    EL_DIAMOND,				-1, MV_BIT_RIGHT
  },
  {
    Xpush_diamond_w,			FALSE,	FALSE,
    EL_DIAMOND,				-1, MV_BIT_LEFT
  },
  {
    Xpush_stone_e,			FALSE,	FALSE,
    EL_ROCK,				-1, MV_BIT_RIGHT
  },
  {
    Xpush_stone_w,			FALSE,	FALSE,
    EL_ROCK,				-1, MV_BIT_LEFT
  },
  {
    Xpush_bomb_e,			FALSE,	FALSE,
    EL_BOMB,				-1, MV_BIT_RIGHT
  },
  {
    Xpush_bomb_w,			FALSE,	FALSE,
    EL_BOMB,				-1, MV_BIT_LEFT
  },
  {
    Xpush_nut_e,			FALSE,	FALSE,
    EL_NUT,				-1, MV_BIT_RIGHT
  },
  {
    Xpush_nut_w,			FALSE,	FALSE,
    EL_NUT,				-1, MV_BIT_LEFT
  },
  {
    Xpush_spring_e,			FALSE,	FALSE,
    EL_SPRING_RIGHT,			-1, MV_BIT_RIGHT
  },
  {
    Xpush_spring_w,			FALSE,	FALSE,
    EL_SPRING_LEFT,			-1, MV_BIT_LEFT
  },

  {
    Xdynamite,				TRUE,	FALSE,
    EL_EM_DYNAMITE,			-1, -1
  },
  {
    Ydynamite_blank,			FALSE,	FALSE,
    EL_EM_DYNAMITE,			ACTION_COLLECTING, -1
  },
  {
    Xdynamite_1,			TRUE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_2,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_3,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_4,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },

  {
    Xkey_1,				TRUE,	FALSE,
    EL_EM_KEY_1,			-1, -1
  },
  {
    Xkey_2,				TRUE,	FALSE,
    EL_EM_KEY_2,			-1, -1
  },
  {
    Xkey_3,				TRUE,	FALSE,
    EL_EM_KEY_3,			-1, -1
  },
  {
    Xkey_4,				TRUE,	FALSE,
    EL_EM_KEY_4,			-1, -1
  },
  {
    Xkey_5,				TRUE,	FALSE,
    EL_EMC_KEY_5,			-1, -1
  },
  {
    Xkey_6,				TRUE,	FALSE,
    EL_EMC_KEY_6,			-1, -1
  },
  {
    Xkey_7,				TRUE,	FALSE,
    EL_EMC_KEY_7,			-1, -1
  },
  {
    Xkey_8,				TRUE,	FALSE,
    EL_EMC_KEY_8,			-1, -1
  },

  {
    Xdoor_1,				TRUE,	FALSE,
    EL_EM_GATE_1,			-1, -1
  },
  {
    Xdoor_2,				TRUE,	FALSE,
    EL_EM_GATE_2,			-1, -1
  },
  {
    Xdoor_3,				TRUE,	FALSE,
    EL_EM_GATE_3,			-1, -1
  },
  {
    Xdoor_4,				TRUE,	FALSE,
    EL_EM_GATE_4,			-1, -1
  },
  {
    Xdoor_5,				TRUE,	FALSE,
    EL_EMC_GATE_5,			-1, -1
  },
  {
    Xdoor_6,				TRUE,	FALSE,
    EL_EMC_GATE_6,			-1, -1
  },
  {
    Xdoor_7,				TRUE,	FALSE,
    EL_EMC_GATE_7,			-1, -1
  },
  {
    Xdoor_8,				TRUE,	FALSE,
    EL_EMC_GATE_8,			-1, -1
  },

  {
    Xfake_door_1,			TRUE,	FALSE,
    EL_EM_GATE_1_GRAY,			-1, -1
  },
  {
    Xfake_door_2,			TRUE,	FALSE,
    EL_EM_GATE_2_GRAY,			-1, -1
  },
  {
    Xfake_door_3,			TRUE,	FALSE,
    EL_EM_GATE_3_GRAY,			-1, -1
  },
  {
    Xfake_door_4,			TRUE,	FALSE,
    EL_EM_GATE_4_GRAY,			-1, -1
  },
  {
    Xfake_door_5,			TRUE,	FALSE,
    EL_EMC_GATE_5_GRAY,			-1, -1
  },
  {
    Xfake_door_6,			TRUE,	FALSE,
    EL_EMC_GATE_6_GRAY,			-1, -1
  },
  {
    Xfake_door_7,			TRUE,	FALSE,
    EL_EMC_GATE_7_GRAY,			-1, -1
  },
  {
    Xfake_door_8,			TRUE,	FALSE,
    EL_EMC_GATE_8_GRAY,			-1, -1
  },

  {
    Xballoon,				TRUE,	FALSE,
    EL_BALLOON,				-1, -1
  },
  {
    Yballoon_n,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yballoon_nB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yballoon_e,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yballoon_eB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yballoon_s,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yballoon_sB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yballoon_w,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yballoon_wB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_LEFT
  },

  {
    Xball_1,				TRUE,	FALSE,
    EL_EMC_MAGIC_BALL,			-1, -1
  },
  {
    Yball_1,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Xball_2,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Yball_2,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Yball_blank,			FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_DROPPING, -1
  },

  {
    Xamoeba_1,				TRUE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_2,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_3,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_4,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_5,				TRUE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_6,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_7,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_8,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },

  {
    Xdrip,				TRUE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_GROWING, -1
  },
  {
    Xdrip_fall,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			-1, -1
  },
  {
    Xdrip_stretch,			FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xdrip_stretchB,			FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_1_s,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_1_sB,				FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_2_s,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_2_sB,				FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },

  {
    Xwonderwall,			TRUE,	FALSE,
    EL_MAGIC_WALL,			-1, -1
  },
  {
    Ywonderwall,			FALSE,	FALSE,
    EL_MAGIC_WALL,			ACTION_ACTIVE, -1
  },

  {
    Xwheel,				TRUE,	FALSE,
    EL_ROBOT_WHEEL,			-1, -1
  },
  {
    Ywheel,				FALSE,	FALSE,
    EL_ROBOT_WHEEL,			ACTION_ACTIVE, -1
  },

  {
    Xswitch,				TRUE,	FALSE,
    EL_EMC_MAGIC_BALL_SWITCH,		-1, -1
  },
  {
    Yswitch,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL_SWITCH,		ACTION_ACTIVE, -1
  },

  {
    Xbumper,				TRUE,	FALSE,
    EL_EMC_SPRING_BUMPER,		-1, -1
  },
  {
    Ybumper,				FALSE,	FALSE,
    EL_EMC_SPRING_BUMPER,		ACTION_ACTIVE, -1
  },

  {
    Xacid_nw,				TRUE,	FALSE,
    EL_ACID_POOL_TOPLEFT,		-1, -1
  },
  {
    Xacid_ne,				TRUE,	FALSE,
    EL_ACID_POOL_TOPRIGHT,		-1, -1
  },
  {
    Xacid_sw,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOMLEFT,		-1, -1
  },
  {
    Xacid_s,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOM,		-1, -1
  },
  {
    Xacid_se,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOMRIGHT,		-1, -1
  },

  {
    Xfake_blank,			TRUE,	FALSE,
    EL_INVISIBLE_WALL,			-1, -1
  },
  {
    Yfake_blank,			FALSE,	FALSE,
    EL_INVISIBLE_WALL,			ACTION_ACTIVE, -1
  },

  {
    Xfake_grass,			TRUE,	FALSE,
    EL_EMC_FAKE_GRASS,			-1, -1
  },
  {
    Yfake_grass,			FALSE,	FALSE,
    EL_EMC_FAKE_GRASS,			ACTION_ACTIVE, -1
  },

  {
    Xfake_amoeba,			TRUE,	FALSE,
    EL_EMC_DRIPPER,			-1, -1
  },
  {
    Yfake_amoeba,			FALSE,	FALSE,
    EL_EMC_DRIPPER,			ACTION_ACTIVE, -1
  },

  {
    Xlenses,				TRUE,	FALSE,
    EL_EMC_LENSES,			-1, -1
  },

  {
    Xmagnify,				TRUE,	FALSE,
    EL_EMC_MAGNIFIER,			-1, -1
  },

  {
    Xsand,				TRUE,	FALSE,
    EL_QUICKSAND_EMPTY,			-1, -1
  },
  {
    Xsand_stone,			TRUE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stonein_1,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_2,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_3,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_4,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_sandstone_1,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_sandstone_2,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_sandstone_3,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_sandstone_4,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_stonesand_1,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_2,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_3,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_4,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stoneout_1,			FALSE,	FALSE,
    EL_ROCK,				ACTION_EMPTYING, -1
  },
  {
    Xsand_stoneout_2,			FALSE,	FALSE,
    EL_ROCK,				ACTION_EMPTYING, -1
  },
  {
    Xsand_stonesand_quickout_1,		FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_quickout_2,		FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },

  {
    Xslide_ns,				TRUE,	FALSE,
    EL_EXPANDABLE_WALL_VERTICAL,	-1, -1
  },
  {
    Yslide_ns_blank,			FALSE,	FALSE,
    EL_EXPANDABLE_WALL_VERTICAL,	ACTION_GROWING, -1
  },
  {
    Xslide_ew,				TRUE,	FALSE,
    EL_EXPANDABLE_WALL_HORIZONTAL,	-1, -1
  },
  {
    Yslide_ew_blank,			FALSE,	FALSE,
    EL_EXPANDABLE_WALL_HORIZONTAL,	ACTION_GROWING, -1
  },

  {
    Xwind_n,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_UP,		-1, -1
  },
  {
    Xwind_e,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_RIGHT,		-1, -1
  },
  {
    Xwind_s,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_DOWN,		-1, -1
  },
  {
    Xwind_w,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_LEFT,		-1, -1
  },
  {
    Xwind_any,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_ANY,		-1, -1
  },
  {
    Xwind_stop,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_NONE,		-1, -1
  },

  {
    Xexit,				TRUE,	FALSE,
    EL_EM_EXIT_CLOSED,			-1, -1
  },
  {
    Xexit_1,				TRUE,	FALSE,
    EL_EM_EXIT_OPEN,			-1, -1
  },
  {
    Xexit_2,				FALSE,	FALSE,
    EL_EM_EXIT_OPEN,			-1, -1
  },
  {
    Xexit_3,				FALSE,	FALSE,
    EL_EM_EXIT_OPEN,			-1, -1
  },

  {
    Xpause,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },

  {
    Xwall_1,				TRUE,	FALSE,
    EL_WALL,				-1, -1
  },
  {
    Xwall_2,				TRUE,	FALSE,
    EL_EMC_WALL_14,			-1, -1
  },
  {
    Xwall_3,				TRUE,	FALSE,
    EL_EMC_WALL_15,			-1, -1
  },
  {
    Xwall_4,				TRUE,	FALSE,
    EL_EMC_WALL_16,			-1, -1
  },

  {
    Xroundwall_1,			TRUE,	FALSE,
    EL_WALL_SLIPPERY,			-1, -1
  },
  {
    Xroundwall_2,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_2,		-1, -1
  },
  {
    Xroundwall_3,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_3,		-1, -1
  },
  {
    Xroundwall_4,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_4,		-1, -1
  },

  {
    Xsteel_1,				TRUE,	FALSE,
    EL_STEELWALL,			-1, -1
  },
  {
    Xsteel_2,				TRUE,	FALSE,
    EL_EMC_STEELWALL_2,			-1, -1
  },
  {
    Xsteel_3,				TRUE,	FALSE,
    EL_EMC_STEELWALL_3,			-1, -1
  },
  {
    Xsteel_4,				TRUE,	FALSE,
    EL_EMC_STEELWALL_4,			-1, -1
  },

  {
    Xdecor_1,				TRUE,	FALSE,
    EL_EMC_WALL_8,			-1, -1
  },
  {
    Xdecor_2,				TRUE,	FALSE,
    EL_EMC_WALL_6,			-1, -1
  },
  {
    Xdecor_3,				TRUE,	FALSE,
    EL_EMC_WALL_4,			-1, -1
  },
  {
    Xdecor_4,				TRUE,	FALSE,
    EL_EMC_WALL_7,			-1, -1
  },
  {
    Xdecor_5,				TRUE,	FALSE,
    EL_EMC_WALL_5,			-1, -1
  },
  {
    Xdecor_6,				TRUE,	FALSE,
    EL_EMC_WALL_9,			-1, -1
  },
  {
    Xdecor_7,				TRUE,	FALSE,
    EL_EMC_WALL_10,			-1, -1
  },
  {
    Xdecor_8,				TRUE,	FALSE,
    EL_EMC_WALL_1,			-1, -1
  },
  {
    Xdecor_9,				TRUE,	FALSE,
    EL_EMC_WALL_2,			-1, -1
  },
  {
    Xdecor_10,				TRUE,	FALSE,
    EL_EMC_WALL_3,			-1, -1
  },
  {
    Xdecor_11,				TRUE,	FALSE,
    EL_EMC_WALL_11,			-1, -1
  },
  {
    Xdecor_12,				TRUE,	FALSE,
    EL_EMC_WALL_12,			-1, -1
  },

  {
    Xalpha_0,				TRUE,	FALSE,
    EL_CHAR('0'),			-1, -1
  },
  {
    Xalpha_1,				TRUE,	FALSE,
    EL_CHAR('1'),			-1, -1
  },
  {
    Xalpha_2,				TRUE,	FALSE,
    EL_CHAR('2'),			-1, -1
  },
  {
    Xalpha_3,				TRUE,	FALSE,
    EL_CHAR('3'),			-1, -1
  },
  {
    Xalpha_4,				TRUE,	FALSE,
    EL_CHAR('4'),			-1, -1
  },
  {
    Xalpha_5,				TRUE,	FALSE,
    EL_CHAR('5'),			-1, -1
  },
  {
    Xalpha_6,				TRUE,	FALSE,
    EL_CHAR('6'),			-1, -1
  },
  {
    Xalpha_7,				TRUE,	FALSE,
    EL_CHAR('7'),			-1, -1
  },
  {
    Xalpha_8,				TRUE,	FALSE,
    EL_CHAR('8'),			-1, -1
  },
  {
    Xalpha_9,				TRUE,	FALSE,
    EL_CHAR('9'),			-1, -1
  },
  {
    Xalpha_excla,			TRUE,	FALSE,
    EL_CHAR('!'),			-1, -1
  },
  {
    Xalpha_apost,			TRUE,	FALSE,
    EL_CHAR('\''),			-1, -1
  },
  {
    Xalpha_comma,			TRUE,	FALSE,
    EL_CHAR(','),			-1, -1
  },
  {
    Xalpha_minus,			TRUE,	FALSE,
    EL_CHAR('-'),			-1, -1
  },
  {
    Xalpha_perio,			TRUE,	FALSE,
    EL_CHAR('.'),			-1, -1
  },
  {
    Xalpha_colon,			TRUE,	FALSE,
    EL_CHAR(':'),			-1, -1
  },
  {
    Xalpha_quest,			TRUE,	FALSE,
    EL_CHAR('?'),			-1, -1
  },
  {
    Xalpha_a,				TRUE,	FALSE,
    EL_CHAR('A'),			-1, -1
  },
  {
    Xalpha_b,				TRUE,	FALSE,
    EL_CHAR('B'),			-1, -1
  },
  {
    Xalpha_c,				TRUE,	FALSE,
    EL_CHAR('C'),			-1, -1
  },
  {
    Xalpha_d,				TRUE,	FALSE,
    EL_CHAR('D'),			-1, -1
  },
  {
    Xalpha_e,				TRUE,	FALSE,
    EL_CHAR('E'),			-1, -1
  },
  {
    Xalpha_f,				TRUE,	FALSE,
    EL_CHAR('F'),			-1, -1
  },
  {
    Xalpha_g,				TRUE,	FALSE,
    EL_CHAR('G'),			-1, -1
  },
  {
    Xalpha_h,				TRUE,	FALSE,
    EL_CHAR('H'),			-1, -1
  },
  {
    Xalpha_i,				TRUE,	FALSE,
    EL_CHAR('I'),			-1, -1
  },
  {
    Xalpha_j,				TRUE,	FALSE,
    EL_CHAR('J'),			-1, -1
  },
  {
    Xalpha_k,				TRUE,	FALSE,
    EL_CHAR('K'),			-1, -1
  },
  {
    Xalpha_l,				TRUE,	FALSE,
    EL_CHAR('L'),			-1, -1
  },
  {
    Xalpha_m,				TRUE,	FALSE,
    EL_CHAR('M'),			-1, -1
  },
  {
    Xalpha_n,				TRUE,	FALSE,
    EL_CHAR('N'),			-1, -1
  },
  {
    Xalpha_o,				TRUE,	FALSE,
    EL_CHAR('O'),			-1, -1
  },
  {
    Xalpha_p,				TRUE,	FALSE,
    EL_CHAR('P'),			-1, -1
  },
  {
    Xalpha_q,				TRUE,	FALSE,
    EL_CHAR('Q'),			-1, -1
  },
  {
    Xalpha_r,				TRUE,	FALSE,
    EL_CHAR('R'),			-1, -1
  },
  {
    Xalpha_s,				TRUE,	FALSE,
    EL_CHAR('S'),			-1, -1
  },
  {
    Xalpha_t,				TRUE,	FALSE,
    EL_CHAR('T'),			-1, -1
  },
  {
    Xalpha_u,				TRUE,	FALSE,
    EL_CHAR('U'),			-1, -1
  },
  {
    Xalpha_v,				TRUE,	FALSE,
    EL_CHAR('V'),			-1, -1
  },
  {
    Xalpha_w,				TRUE,	FALSE,
    EL_CHAR('W'),			-1, -1
  },
  {
    Xalpha_x,				TRUE,	FALSE,
    EL_CHAR('X'),			-1, -1
  },
  {
    Xalpha_y,				TRUE,	FALSE,
    EL_CHAR('Y'),			-1, -1
  },
  {
    Xalpha_z,				TRUE,	FALSE,
    EL_CHAR('Z'),			-1, -1
  },
  {
    Xalpha_arrow_e,			TRUE,	FALSE,
    EL_CHAR('>'),			-1, -1
  },
  {
    Xalpha_arrow_w,			TRUE,	FALSE,
    EL_CHAR('<'),			-1, -1
  },
  {
    Xalpha_copyr,			TRUE,	FALSE,
    EL_CHAR(CHAR_BYTE_COPYRIGHT),	-1, -1
  },

  {
    Ykey_1_blank,			FALSE,	FALSE,
    EL_EM_KEY_1,			ACTION_COLLECTING, -1
  },
  {
    Ykey_2_blank,			FALSE,	FALSE,
    EL_EM_KEY_2,			ACTION_COLLECTING, -1
  },
  {
    Ykey_3_blank,			FALSE,	FALSE,
    EL_EM_KEY_3,			ACTION_COLLECTING, -1
  },
  {
    Ykey_4_blank,			FALSE,	FALSE,
    EL_EM_KEY_4,			ACTION_COLLECTING, -1
  },
  {
    Ykey_5_blank,			FALSE,	FALSE,
    EL_EMC_KEY_5,			ACTION_COLLECTING, -1
  },
  {
    Ykey_6_blank,			FALSE,	FALSE,
    EL_EMC_KEY_6,			ACTION_COLLECTING, -1
  },
  {
    Ykey_7_blank,			FALSE,	FALSE,
    EL_EMC_KEY_7,			ACTION_COLLECTING, -1
  },
  {
    Ykey_8_blank,			FALSE,	FALSE,
    EL_EMC_KEY_8,			ACTION_COLLECTING, -1
  },
  {
    Ylenses_blank,			FALSE,	FALSE,
    EL_EMC_LENSES,			ACTION_COLLECTING, -1
  },
  {
    Ymagnify_blank,			FALSE,	FALSE,
    EL_EMC_MAGNIFIER,			ACTION_COLLECTING, -1
  },
  {
    Ygrass_blank,			FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_SNAPPING, -1
  },
  {
    Ydirt_blank,			FALSE,	FALSE,
    EL_SAND,				ACTION_SNAPPING, -1
  },

  {
    -1,					FALSE,	FALSE,
    -1,					-1, -1
  }
};

static struct Mapping_EM_to_RND_player
{
  int action_em;
  int player_nr;

  int element_rnd;
  int action;
  int direction;
}
em_player_mapping_list[MAX_PLAYERS * PLY_MAX + 1] =
{
  {
    PLY_walk_n,				0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    PLY_walk_e,				0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    PLY_walk_s,				0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    PLY_walk_w,				0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    PLY_push_n,				0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    PLY_push_e,				0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    PLY_push_s,				0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    PLY_push_w,				0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    PLY_shoot_n,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    PLY_shoot_e,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    PLY_shoot_s,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    PLY_shoot_w,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    PLY_walk_n,				1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    PLY_walk_e,				1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    PLY_walk_s,				1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    PLY_walk_w,				1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    PLY_push_n,				1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    PLY_push_e,				1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    PLY_push_s,				1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    PLY_push_w,				1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    PLY_shoot_n,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    PLY_shoot_e,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    PLY_shoot_s,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    PLY_shoot_w,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    PLY_still,				0,
    EL_PLAYER_1,			ACTION_DEFAULT, -1,
  },
  {
    PLY_still,				1,
    EL_PLAYER_2,			ACTION_DEFAULT, -1,
  },
  {
    PLY_walk_n,				2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    PLY_walk_e,				2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    PLY_walk_s,				2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    PLY_walk_w,				2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    PLY_push_n,				2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    PLY_push_e,				2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    PLY_push_s,				2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    PLY_push_w,				2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    PLY_shoot_n,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    PLY_shoot_e,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    PLY_shoot_s,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    PLY_shoot_w,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    PLY_walk_n,				3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    PLY_walk_e,				3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    PLY_walk_s,				3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    PLY_walk_w,				3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    PLY_push_n,				3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    PLY_push_e,				3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    PLY_push_s,				3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    PLY_push_w,				3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    PLY_shoot_n,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    PLY_shoot_e,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    PLY_shoot_s,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    PLY_shoot_w,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    PLY_still,				2,
    EL_PLAYER_3,			ACTION_DEFAULT, -1,
  },
  {
    PLY_still,				3,
    EL_PLAYER_4,			ACTION_DEFAULT, -1,
  },

  {
    -1,					-1,
    -1,					-1, -1
  }
};

int map_element_RND_to_EM_cave(int element_rnd)
{
  static unsigned short mapping_RND_to_EM[NUM_FILE_ELEMENTS];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    // return "Xalpha_quest" for all undefined elements in mapping array
    for (i = 0; i < NUM_FILE_ELEMENTS; i++)
      mapping_RND_to_EM[i] = Xalpha_quest;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      if (em_object_mapping_list[i].is_rnd_to_em_mapping)
	mapping_RND_to_EM[em_object_mapping_list[i].element_rnd] =
	  em_object_mapping_list[i].element_em;

    mapping_initialized = TRUE;
  }

  if (element_rnd < 0 || element_rnd >= NUM_FILE_ELEMENTS)
  {
    Warn("invalid RND level element %d", element_rnd);

    return EL_UNKNOWN;
  }

  return map_em_element_X_to_C(mapping_RND_to_EM[element_rnd]);
}

int map_element_EM_to_RND_cave(int element_em_cave)
{
  static unsigned short mapping_EM_to_RND[GAME_TILE_MAX];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    // return "EL_UNKNOWN" for all undefined elements in mapping array
    for (i = 0; i < GAME_TILE_MAX; i++)
      mapping_EM_to_RND[i] = EL_UNKNOWN;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      mapping_EM_to_RND[em_object_mapping_list[i].element_em] =
	em_object_mapping_list[i].element_rnd;

    mapping_initialized = TRUE;
  }

  if (element_em_cave < 0 || element_em_cave >= CAVE_TILE_MAX)
  {
    Warn("invalid EM cave element %d", element_em_cave);

    return EL_UNKNOWN;
  }

  return mapping_EM_to_RND[map_em_element_C_to_X(element_em_cave)];
}

int map_element_EM_to_RND_game(int element_em_game)
{
  static unsigned short mapping_EM_to_RND[GAME_TILE_MAX];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    // return "EL_UNKNOWN" for all undefined elements in mapping array
    for (i = 0; i < GAME_TILE_MAX; i++)
      mapping_EM_to_RND[i] = EL_UNKNOWN;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      mapping_EM_to_RND[em_object_mapping_list[i].element_em] =
	em_object_mapping_list[i].element_rnd;

    mapping_initialized = TRUE;
  }

  if (element_em_game < 0 || element_em_game >= GAME_TILE_MAX)
  {
    Warn("invalid EM game element %d", element_em_game);

    return EL_UNKNOWN;
  }

  return mapping_EM_to_RND[element_em_game];
}

void map_android_clone_elements_RND_to_EM(struct LevelInfo *level)
{
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct CAVE *cav = level_em->cav;
  int i, j;

  for (i = 0; i < GAME_TILE_MAX; i++)
    cav->android_array[i] = Cblank;

  for (i = 0; i < level->num_android_clone_elements; i++)
  {
    int element_rnd = level->android_clone_element[i];
    int element_em_cave = map_element_RND_to_EM_cave(element_rnd);

    for (j = 0; em_object_mapping_list[j].element_em != -1; j++)
      if (em_object_mapping_list[j].element_rnd == element_rnd)
	cav->android_array[em_object_mapping_list[j].element_em] =
	  element_em_cave;
  }
}

void map_android_clone_elements_EM_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct CAVE *cav = level_em->cav;
  int i, j;

  level->num_android_clone_elements = 0;

  for (i = 0; i < GAME_TILE_MAX; i++)
  {
    int element_em_cave = cav->android_array[i];
    int element_rnd;
    boolean element_found = FALSE;

    if (element_em_cave == Cblank)
      continue;

    element_rnd = map_element_EM_to_RND_cave(element_em_cave);

    for (j = 0; j < level->num_android_clone_elements; j++)
      if (level->android_clone_element[j] == element_rnd)
	element_found = TRUE;

    if (!element_found)
    {
      level->android_clone_element[level->num_android_clone_elements++] =
	element_rnd;

      if (level->num_android_clone_elements == MAX_ANDROID_ELEMENTS)
	break;
    }
  }

  if (level->num_android_clone_elements == 0)
  {
    level->num_android_clone_elements = 1;
    level->android_clone_element[0] = EL_EMPTY;
  }
}

int map_direction_RND_to_EM(int direction)
{
  return (direction == MV_UP    ? 0 :
	  direction == MV_RIGHT ? 1 :
	  direction == MV_DOWN  ? 2 :
	  direction == MV_LEFT  ? 3 :
	  -1);
}

int map_direction_EM_to_RND(int direction)
{
  return (direction == 0 ? MV_UP    :
	  direction == 1 ? MV_RIGHT :
	  direction == 2 ? MV_DOWN  :
	  direction == 3 ? MV_LEFT  :
	  MV_NONE);
}

int map_element_RND_to_SP(int element_rnd)
{
  int element_sp = 0x20;	// map unknown elements to yellow "hardware"

  if (element_rnd >= EL_SP_START &&
      element_rnd <= EL_SP_END)
    element_sp = element_rnd - EL_SP_START;
  else if (element_rnd == EL_EMPTY_SPACE)
    element_sp = 0x00;
  else if (element_rnd == EL_INVISIBLE_WALL)
    element_sp = 0x28;

  return element_sp;
}

int map_element_SP_to_RND(int element_sp)
{
  int element_rnd = EL_UNKNOWN;

  if (element_sp >= 0x00 &&
      element_sp <= 0x27)
    element_rnd = EL_SP_START + element_sp;
  else if (element_sp == 0x28)
    element_rnd = EL_INVISIBLE_WALL;

  return element_rnd;
}

int map_action_SP_to_RND(int action_sp)
{
  switch (action_sp)
  {
    case actActive:		return ACTION_ACTIVE;
    case actImpact:		return ACTION_IMPACT;
    case actExploding:		return ACTION_EXPLODING;
    case actDigging:		return ACTION_DIGGING;
    case actSnapping:		return ACTION_SNAPPING;
    case actCollecting:		return ACTION_COLLECTING;
    case actPassing:		return ACTION_PASSING;
    case actPushing:		return ACTION_PUSHING;
    case actDropping:		return ACTION_DROPPING;

    default:			return ACTION_DEFAULT;
  }
}

int map_element_RND_to_MM(int element_rnd)
{
  return (element_rnd >= EL_MM_START_1 &&
	  element_rnd <= EL_MM_END_1 ?
	  EL_MM_START_1_NATIVE + element_rnd - EL_MM_START_1 :

	  element_rnd >= EL_MM_START_2 &&
	  element_rnd <= EL_MM_END_2 ?
	  EL_MM_START_2_NATIVE + element_rnd - EL_MM_START_2 :

	  element_rnd >= EL_MM_START_3 &&
	  element_rnd <= EL_MM_END_3 ?
	  EL_MM_START_3_NATIVE + element_rnd - EL_MM_START_3 :

	  element_rnd >= EL_CHAR_START &&
	  element_rnd <= EL_CHAR_END ?
	  EL_MM_CHAR_START_NATIVE + element_rnd - EL_CHAR_START :

	  element_rnd >= EL_MM_RUNTIME_START &&
	  element_rnd <= EL_MM_RUNTIME_END ?
	  EL_MM_RUNTIME_START_NATIVE + element_rnd - EL_MM_RUNTIME_START :

	  EL_MM_EMPTY_NATIVE);
}

int map_element_MM_to_RND(int element_mm)
{
  return (element_mm == EL_MM_EMPTY_NATIVE ||
	  element_mm == EL_DF_EMPTY_NATIVE ?
	  EL_EMPTY :

	  element_mm >= EL_MM_START_1_NATIVE &&
	  element_mm <= EL_MM_END_1_NATIVE ?
	  EL_MM_START_1 + element_mm - EL_MM_START_1_NATIVE :

	  element_mm >= EL_MM_START_2_NATIVE &&
	  element_mm <= EL_MM_END_2_NATIVE ?
	  EL_MM_START_2 + element_mm - EL_MM_START_2_NATIVE :

	  element_mm >= EL_MM_START_3_NATIVE &&
	  element_mm <= EL_MM_END_3_NATIVE ?
	  EL_MM_START_3 + element_mm - EL_MM_START_3_NATIVE :

	  element_mm >= EL_MM_CHAR_START_NATIVE &&
	  element_mm <= EL_MM_CHAR_END_NATIVE ?
	  EL_CHAR_START + element_mm - EL_MM_CHAR_START_NATIVE :

	  element_mm >= EL_MM_RUNTIME_START_NATIVE &&
	  element_mm <= EL_MM_RUNTIME_END_NATIVE ?
	  EL_MM_RUNTIME_START + element_mm - EL_MM_RUNTIME_START_NATIVE :

	  EL_EMPTY);
}

int map_action_MM_to_RND(int action_mm)
{
  // all MM actions are defined to exactly match their RND counterparts
  return action_mm;
}

int map_sound_MM_to_RND(int sound_mm)
{
  switch (sound_mm)
  {
    case SND_MM_GAME_LEVELTIME_CHARGING:
      return SND_GAME_LEVELTIME_CHARGING;

    case SND_MM_GAME_HEALTH_CHARGING:
      return SND_GAME_HEALTH_CHARGING;

    default:
      return SND_UNDEFINED;
  }
}

int map_mm_wall_element(int element)
{
  return (element >= EL_MM_STEEL_WALL_START &&
	  element <= EL_MM_STEEL_WALL_END ?
	  EL_MM_STEEL_WALL :

	  element >= EL_MM_WOODEN_WALL_START &&
	  element <= EL_MM_WOODEN_WALL_END ?
	  EL_MM_WOODEN_WALL :

	  element >= EL_MM_ICE_WALL_START &&
	  element <= EL_MM_ICE_WALL_END ?
	  EL_MM_ICE_WALL :

	  element >= EL_MM_AMOEBA_WALL_START &&
	  element <= EL_MM_AMOEBA_WALL_END ?
	  EL_MM_AMOEBA_WALL :

	  element >= EL_DF_STEEL_WALL_START &&
	  element <= EL_DF_STEEL_WALL_END ?
	  EL_DF_STEEL_WALL :

	  element >= EL_DF_WOODEN_WALL_START &&
	  element <= EL_DF_WOODEN_WALL_END ?
	  EL_DF_WOODEN_WALL :

	  element);
}

int map_mm_wall_element_editor(int element)
{
  switch (element)
  {
    case EL_MM_STEEL_WALL:	return EL_MM_STEEL_WALL_START;
    case EL_MM_WOODEN_WALL:	return EL_MM_WOODEN_WALL_START;
    case EL_MM_ICE_WALL:	return EL_MM_ICE_WALL_START;
    case EL_MM_AMOEBA_WALL:	return EL_MM_AMOEBA_WALL_START;
    case EL_DF_STEEL_WALL:	return EL_DF_STEEL_WALL_START;
    case EL_DF_WOODEN_WALL:	return EL_DF_WOODEN_WALL_START;

    default:			return element;
  }
}

int get_next_element(int element)
{
  switch (element)
  {
    case EL_QUICKSAND_FILLING:		return EL_QUICKSAND_FULL;
    case EL_QUICKSAND_EMPTYING:		return EL_QUICKSAND_EMPTY;
    case EL_QUICKSAND_FAST_FILLING:	return EL_QUICKSAND_FAST_FULL;
    case EL_QUICKSAND_FAST_EMPTYING:	return EL_QUICKSAND_FAST_EMPTY;
    case EL_MAGIC_WALL_FILLING:		return EL_MAGIC_WALL_FULL;
    case EL_MAGIC_WALL_EMPTYING:	return EL_MAGIC_WALL_ACTIVE;
    case EL_BD_MAGIC_WALL_FILLING:	return EL_BD_MAGIC_WALL_FULL;
    case EL_BD_MAGIC_WALL_EMPTYING:	return EL_BD_MAGIC_WALL_ACTIVE;
    case EL_DC_MAGIC_WALL_FILLING:	return EL_DC_MAGIC_WALL_FULL;
    case EL_DC_MAGIC_WALL_EMPTYING:	return EL_DC_MAGIC_WALL_ACTIVE;
    case EL_AMOEBA_DROPPING:		return EL_AMOEBA_WET;

    default:				return element;
  }
}

int el2img_mm(int element_mm)
{
  return el2img(map_element_MM_to_RND(element_mm));
}

int el_act2img_mm(int element_mm, int action)
{
  return el_act2img(map_element_MM_to_RND(element_mm), action);
}

int el_act_dir2img(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_TO_BIT(direction);	// default: MV_NONE => MV_DOWN

  // direction_graphic[][] == graphic[] for undefined direction graphics
  return element_info[element].direction_graphic[action][direction];
}

static int el_act_dir2crm(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_TO_BIT(direction);	// default: MV_NONE => MV_DOWN

  // direction_graphic[][] == graphic[] for undefined direction graphics
  return element_info[element].direction_crumbled[action][direction];
}

int el_act2img(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[action];
}

int el_act2crm(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].crumbled[action];
}

int el_dir2img(int element, int direction)
{
  element = GFX_ELEMENT(element);

  return el_act_dir2img(element, ACTION_DEFAULT, direction);
}

int el2baseimg(int element)
{
  return element_info[element].graphic[ACTION_DEFAULT];
}

int el2img(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[ACTION_DEFAULT];
}

int el2edimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_EDITOR];
}

int el2edimg_with_frame(int element, int *graphic, int *frame)
{
  // make sure to use special editor graphics for scanned elements, if available
  if (IS_BDX_SCANNED_ELEMENT(element))
  {
    int element_bd_scanned = map_element_RND_to_BD_cave(element);
    int element_bd_unscanned = getNonScannedElement_BD(element_bd_scanned);

    element = map_element_BD_to_RND_cave(element_bd_unscanned);
  }

  *graphic = el2edimg(element);
  *frame = 0;

  if (*graphic == IMG_UNKNOWN)
  {
    // no graphic defined -- if BD style, try to get runtime ("effect") element graphics
    // (normal BD style elements have graphics, but runtime ("effects") elements do not)
    int element_bd = map_element_RND_to_BD_cave(element);

    if (element_bd != O_UNKNOWN)
    {
      struct GraphicInfo_BD *g_bd = &graphic_info_bd_object[element_bd][0];

      *graphic = g_bd->graphic;
      *frame   = g_bd->frame;
    }
  }

  return *graphic;
}

int el2preimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_PREVIEW];
}

int el2panelimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_PANEL];
}

int font2baseimg(int font_nr)
{
  return font_info[font_nr].special_graphic[GFX_SPECIAL_ARG_DEFAULT];
}

int getBeltNrFromBeltElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT_3_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT_4_LEFT ? 2 : 3);
}

int getBeltNrFromBeltActiveElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_LEFT_ACTIVE ? 0 :
	  element < EL_CONVEYOR_BELT_3_LEFT_ACTIVE ? 1 :
	  element < EL_CONVEYOR_BELT_4_LEFT_ACTIVE ? 2 : 3);
}

int getBeltNrFromBeltSwitchElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_SWITCH_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT_3_SWITCH_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT_4_SWITCH_LEFT ? 2 : 3);
}

int getBeltDirNrFromBeltElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };

  int belt_nr = getBeltNrFromBeltElement(element);
  int belt_dir_nr = element - belt_base_element[belt_nr];

  return (belt_dir_nr % 3);
}

int getBeltDirNrFromBeltSwitchElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
  };

  int belt_nr = getBeltNrFromBeltSwitchElement(element);
  int belt_dir_nr = element - belt_base_element[belt_nr];

  return (belt_dir_nr % 3);
}

int getBeltDirFromBeltElement(int element)
{
  static int belt_move_dir[3] =
  {
    MV_LEFT,
    MV_NONE,
    MV_RIGHT
  };

  int belt_dir_nr = getBeltDirNrFromBeltElement(element);

  return belt_move_dir[belt_dir_nr];
}

int getBeltDirFromBeltSwitchElement(int element)
{
  static int belt_move_dir[3] =
  {
    MV_LEFT,
    MV_NONE,
    MV_RIGHT
  };

  int belt_dir_nr = getBeltDirNrFromBeltSwitchElement(element);

  return belt_move_dir[belt_dir_nr];
}

int getBeltElementFromBeltNrAndBeltDirNr(int belt_nr, int belt_dir_nr)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };

  return belt_base_element[belt_nr] + belt_dir_nr;
}

int getBeltElementFromBeltNrAndBeltDir(int belt_nr, int belt_dir)
{
  int belt_dir_nr = (belt_dir == MV_LEFT ? 0 : belt_dir == MV_RIGHT ? 2 : 1);

  return getBeltElementFromBeltNrAndBeltDirNr(belt_nr, belt_dir_nr);
}

int getBeltSwitchElementFromBeltNrAndBeltDirNr(int belt_nr, int belt_dir_nr)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
  };

  return belt_base_element[belt_nr] + belt_dir_nr;
}

int getBeltSwitchElementFromBeltNrAndBeltDir(int belt_nr, int belt_dir)
{
  int belt_dir_nr = (belt_dir == MV_LEFT ? 0 : belt_dir == MV_RIGHT ? 2 : 1);

  return getBeltSwitchElementFromBeltNrAndBeltDirNr(belt_nr, belt_dir_nr);
}

boolean useOldEngine_BD(void)
{
  // never use old BD game engine for new games (but only when playing tapes)
  if (!tape.playing)
    return FALSE;

  // use old BD game engine if playing specifically tagged native BD replays
  if (tape.bd_replay && leveldir_current->replay_with_old_engine)
    return TRUE;

  // use old BD game engine if playing specifically tagged or patched tapes
  if (tape.property_bits & TAPE_PROPERTY_BD_OLD_ENGINE)
    return TRUE;

  // use old BD game engine if playing tapes from old 4.4.0.0 pre-release versions
  if (tape.game_version_full < VERSION_IDENT_FULL(4,4,0,0, 0,4,0))
    return TRUE;

  return FALSE;
}

boolean getTimePlayed_BD(void)
{
  return TimePlayed;
}

boolean swapTiles_EM(boolean is_pre_emc_cave)
{
  return is_pre_emc_cave && leveldir_current->use_emc_tiles;
}

boolean getTeamMode_EM(void)
{
  return game.team_mode || network_playing;
}

boolean isActivePlayer_EM(int player_nr)
{
  return stored_player[player_nr].active;
}

unsigned int InitRND(int seed)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_BD)
    return InitEngineRandom_BD(seed);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return InitEngineRandom_EM(seed);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return InitEngineRandom_SP(seed);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return InitEngineRandom_MM(seed);
  else
    return InitEngineRandom_RND(seed);
}

static struct Mapping_BD_to_RND_object bd_object_mapping[O_MAX_ALL];
static struct Mapping_EM_to_RND_object em_object_mapping[GAME_TILE_MAX];
static struct Mapping_EM_to_RND_player em_player_mapping[MAX_PLAYERS][PLY_MAX];

static int get_effective_element_EM(int tile, int frame_em)
{
  int element             = em_object_mapping[tile].element_rnd;
  int action              = em_object_mapping[tile].action;
  boolean is_backside     = em_object_mapping[tile].is_backside;
  boolean action_removing = (action == ACTION_DIGGING ||
			     action == ACTION_SNAPPING ||
			     action == ACTION_COLLECTING);

  if (frame_em < 7)
  {
    switch (tile)
    {
      case Xsplash_e:
      case Xsplash_w:
	return (frame_em > 5 ? EL_EMPTY : element);

      default:
	return element;
    }
  }
  else	// frame_em == 7
  {
    switch (tile)
    {
      case Xsplash_e:
      case Xsplash_w:
	return EL_EMPTY;

      case Ynut_stone:
	return EL_EMERALD;

      case Ydiamond_stone:
	return EL_ROCK;

      case Xdrip_stretch:
      case Xdrip_stretchB:
      case Ydrip_1_s:
      case Ydrip_1_sB:
      case Yball_1:
      case Xball_2:
      case Yball_2:
      case Yball_blank:
      case Ykey_1_blank:
      case Ykey_2_blank:
      case Ykey_3_blank:
      case Ykey_4_blank:
      case Ykey_5_blank:
      case Ykey_6_blank:
      case Ykey_7_blank:
      case Ykey_8_blank:
      case Ylenses_blank:
      case Ymagnify_blank:
      case Ygrass_blank:
      case Ydirt_blank:
      case Xsand_stonein_1:
      case Xsand_stonein_2:
      case Xsand_stonein_3:
      case Xsand_stonein_4:
	return element;

      default:
	return (is_backside || action_removing ? EL_EMPTY : element);
    }
  }
}

static boolean check_linear_animation_EM(int tile)
{
  switch (tile)
  {
    case Xsand_stonesand_1:
    case Xsand_stonesand_quickout_1:
    case Xsand_sandstone_1:
    case Xsand_stonein_1:
    case Xsand_stoneout_1:
    case Xboom_1:
    case Xdynamite_1:
    case Ybug_w_n:
    case Ybug_n_e:
    case Ybug_e_s:
    case Ybug_s_w:
    case Ybug_e_n:
    case Ybug_s_e:
    case Ybug_w_s:
    case Ybug_n_w:
    case Ytank_w_n:
    case Ytank_n_e:
    case Ytank_e_s:
    case Ytank_s_w:
    case Ytank_e_n:
    case Ytank_s_e:
    case Ytank_w_s:
    case Ytank_n_w:
    case Xsplash_e:
    case Xsplash_w:
    case Ynut_stone:
      return TRUE;
  }

  return FALSE;
}

static void set_crumbled_graphics_EM(struct GraphicInfo_EM *g_em,
				     boolean has_crumbled_graphics,
				     int crumbled, int sync_frame)
{
  // if element can be crumbled, but certain action graphics are just empty
  // space (like instantly snapping sand to empty space in 1 frame), do not
  // treat these empty space graphics as crumbled graphics in EMC engine
  if (crumbled == IMG_EMPTY_SPACE)
    has_crumbled_graphics = FALSE;

  if (has_crumbled_graphics)
  {
    struct GraphicInfo *g_crumbled = &graphic_info[crumbled];
    int frame_crumbled = getAnimationFrame(g_crumbled->anim_frames,
					   g_crumbled->anim_delay,
					   g_crumbled->anim_mode,
					   g_crumbled->anim_start_frame,
					   sync_frame);

    getGraphicSource(crumbled, frame_crumbled, &g_em->crumbled_bitmap,
		     &g_em->crumbled_src_x, &g_em->crumbled_src_y);

    g_em->crumbled_border_size = graphic_info[crumbled].border_size;
    g_em->crumbled_tile_size = graphic_info[crumbled].tile_size;

    g_em->has_crumbled_graphics = TRUE;
  }
  else
  {
    g_em->crumbled_bitmap = NULL;
    g_em->crumbled_src_x = 0;
    g_em->crumbled_src_y = 0;
    g_em->crumbled_border_size = 0;
    g_em->crumbled_tile_size = 0;

    g_em->has_crumbled_graphics = FALSE;
  }
}

#if 0
void ResetGfxAnimation_EM(int x, int y, int tile)
{
  GfxFrame[x][y] = 0;
}
#endif

void SetGfxAnimation_EM(struct GraphicInfo_EM *g_em,
			int tile, int frame_em, int x, int y)
{
  int action = em_object_mapping[tile].action;
  int direction = em_object_mapping[tile].direction;
  int effective_element = get_effective_element_EM(tile, frame_em);
  int graphic = (direction == MV_NONE ?
		 el_act2img(effective_element, action) :
		 el_act_dir2img(effective_element, action, direction));
  struct GraphicInfo *g = &graphic_info[graphic];
  int sync_frame;
  boolean action_removing = (action == ACTION_DIGGING ||
			     action == ACTION_SNAPPING ||
			     action == ACTION_COLLECTING);
  boolean action_moving   = (action == ACTION_FALLING ||
			     action == ACTION_MOVING ||
			     action == ACTION_PUSHING ||
			     action == ACTION_EATING ||
			     action == ACTION_FILLING ||
			     action == ACTION_EMPTYING);
  boolean action_falling  = (action == ACTION_FALLING ||
			     action == ACTION_FILLING ||
			     action == ACTION_EMPTYING);

  // special case: graphic uses "2nd movement tile" and has defined
  // 7 frames for movement animation (or less) => use default graphic
  // for last (8th) frame which ends the movement animation
  if (g->double_movement && g->anim_frames < 8 && frame_em == 7)
  {
    action = ACTION_DEFAULT;	// (keep action_* unchanged for now)
    graphic = (direction == MV_NONE ?
	       el_act2img(effective_element, action) :
	       el_act_dir2img(effective_element, action, direction));

    g = &graphic_info[graphic];
  }

  if ((action_removing || check_linear_animation_EM(tile)) && frame_em == 0)
  {
    GfxFrame[x][y] = 0;
  }
  else if (action_moving)
  {
    boolean is_backside = em_object_mapping[tile].is_backside;

    if (is_backside)
    {
      int direction = em_object_mapping[tile].direction;
      int move_dir = (action_falling ? MV_DOWN : direction);

      GfxFrame[x][y]++;

#if 1
      // !!! TEST !!! NEW !!! DOES NOT WORK RIGHT YET !!!
      if (g->double_movement && frame_em == 0)
	GfxFrame[x][y] = 0;
#endif

      if (move_dir == MV_LEFT)
	GfxFrame[x - 1][y] = GfxFrame[x][y];
      else if (move_dir == MV_RIGHT)
	GfxFrame[x + 1][y] = GfxFrame[x][y];
      else if (move_dir == MV_UP)
	GfxFrame[x][y - 1] = GfxFrame[x][y];
      else if (move_dir == MV_DOWN)
	GfxFrame[x][y + 1] = GfxFrame[x][y];
    }
  }
  else
  {
    GfxFrame[x][y]++;

    // special case: animation for Xsand_stonesand_quickout_1/2 twice as fast
    if (tile == Xsand_stonesand_quickout_1 ||
	tile == Xsand_stonesand_quickout_2)
      GfxFrame[x][y]++;
  }

  if (graphic_info[graphic].anim_global_sync)
    sync_frame = FrameCounter;
  else if (graphic_info[graphic].anim_global_anim_sync)
    sync_frame = getGlobalAnimSyncFrame();
  else if (IN_FIELD(x, y, MAX_LEV_FIELDX, MAX_LEV_FIELDY))
    sync_frame = GfxFrame[x][y];
  else
    sync_frame = 0;	// playfield border (pseudo steel)

  SetRandomAnimationValue(x, y);

  int frame = getAnimationFrame(g->anim_frames,
				g->anim_delay,
				g->anim_mode,
				g->anim_start_frame,
				sync_frame);

  g_em->unique_identifier =
    (graphic << 16) | ((frame % 8) << 12) | (g_em->width << 6) | g_em->height;
}

void getGraphicSourceObjectExt_EM(struct GraphicInfo_EM *g_em,
				  int tile, int frame_em, int x, int y)
{
  int action = em_object_mapping[tile].action;
  int direction = em_object_mapping[tile].direction;
  boolean is_backside = em_object_mapping[tile].is_backside;
  int effective_element = get_effective_element_EM(tile, frame_em);
  int effective_action = action;
  int graphic = (direction == MV_NONE ?
		 el_act2img(effective_element, effective_action) :
		 el_act_dir2img(effective_element, effective_action,
				direction));
  int crumbled = (direction == MV_NONE ?
		  el_act2crm(effective_element, effective_action) :
		  el_act_dir2crm(effective_element, effective_action,
				 direction));
  int base_graphic = el_act2img(effective_element, ACTION_DEFAULT);
  int base_crumbled = el_act2crm(effective_element, ACTION_DEFAULT);
  boolean has_crumbled_graphics = (base_crumbled != base_graphic);
  struct GraphicInfo *g = &graphic_info[graphic];
  int sync_frame;

  // special case: graphic uses "2nd movement tile" and has defined
  // 7 frames for movement animation (or less) => use default graphic
  // for last (8th) frame which ends the movement animation
  if (g->double_movement && g->anim_frames < 8 && frame_em == 7)
  {
    effective_action = ACTION_DEFAULT;
    graphic = (direction == MV_NONE ?
	       el_act2img(effective_element, effective_action) :
	       el_act_dir2img(effective_element, effective_action,
			      direction));
    crumbled = (direction == MV_NONE ?
		el_act2crm(effective_element, effective_action) :
		el_act_dir2crm(effective_element, effective_action,
			       direction));

    g = &graphic_info[graphic];
  }

  if (graphic_info[graphic].anim_global_sync)
    sync_frame = FrameCounter;
  else if (graphic_info[graphic].anim_global_anim_sync)
    sync_frame = getGlobalAnimSyncFrame();
  else if (IN_FIELD(x, y, MAX_LEV_FIELDX, MAX_LEV_FIELDY))
    sync_frame = GfxFrame[x][y];
  else
    sync_frame = 0;	// playfield border (pseudo steel)

  SetRandomAnimationValue(x, y);

  int frame = getAnimationFrame(g->anim_frames,
				g->anim_delay,
				g->anim_mode,
				g->anim_start_frame,
				sync_frame);

  getGraphicSourceExt(graphic, frame, &g_em->bitmap, &g_em->src_x, &g_em->src_y,
		      g->double_movement && is_backside);

  // (updating the "crumbled" graphic definitions is probably not really needed,
  // as animations for crumbled graphics can't be longer than one EMC cycle)
  set_crumbled_graphics_EM(g_em, has_crumbled_graphics, crumbled,
			   sync_frame);
}

void getGraphicSourcePlayerExt_EM(struct GraphicInfo_EM *g_em,
				  int player_nr, int anim, int frame_em)
{
  int element   = em_player_mapping[player_nr][anim].element_rnd;
  int action    = em_player_mapping[player_nr][anim].action;
  int direction = em_player_mapping[player_nr][anim].direction;
  int graphic = (direction == MV_NONE ?
		 el_act2img(element, action) :
		 el_act_dir2img(element, action, direction));
  struct GraphicInfo *g = &graphic_info[graphic];
  int sync_frame;

  InitPlayerGfxAnimation(&stored_player[player_nr], action, direction);

  stored_player[player_nr].StepFrame = frame_em;

  sync_frame = stored_player[player_nr].Frame;

  int frame = getAnimationFrame(g->anim_frames,
				g->anim_delay,
				g->anim_mode,
				g->anim_start_frame,
				sync_frame);

  getGraphicSourceExt(graphic, frame, &g_em->bitmap,
		      &g_em->src_x, &g_em->src_y, FALSE);
}

#define BD_GFX_RANGE(a, n, i)		((i) >= (a) && (i) < (a) + (n))
#define BD_GFX_FRAME(b, i)		(((i) - (b)) * 8)

void InitGraphicInfo_BD(void)
{
  int i, j;

  if (graphic_info == NULL)		// still at startup phase
    return;

  // always start with reliable default values
  for (i = 0; i < O_MAX_ALL; i++)
  {
    bd_object_mapping[i].element_rnd = EL_UNKNOWN;
    bd_object_mapping[i].action = ACTION_DEFAULT;
    bd_object_mapping[i].direction = MV_NONE;
  }

  for (i = 0; bd_object_mapping_list[i].element_bd != -1; i++)
  {
    int e = bd_object_mapping_list[i].element_bd;

    bd_object_mapping[e].element_rnd = bd_object_mapping_list[i].element_rnd;

    if (bd_object_mapping_list[i].action != -1)
      bd_object_mapping[e].action = bd_object_mapping_list[i].action;

    if (bd_object_mapping_list[i].direction != -1)
      bd_object_mapping[e].direction =
	MV_DIR_FROM_BIT(bd_object_mapping_list[i].direction);
  }

  for (i = 0; i < O_MAX_ALL; i++)
  {
    int e = getNonScannedElement_BD(i);
    int element = bd_object_mapping[e].element_rnd;
    int action = bd_object_mapping[e].action;
    int direction = bd_object_mapping[e].direction;

    for (j = 0; j < 8; j++)
    {
      int effective_element = element;
      int effective_action = action;
      int graphic = (i == O_DIRT_CRUMBLED                   ||
                     i == O_DIRT2_CRUMBLED                  ||
                     i == O_DIRT_GLUED_CRUMBLED             ||
                     i == O_DIRT_SLOPED_UP_RIGHT_CRUMBLED   ||
                     i == O_DIRT_SLOPED_UP_LEFT_CRUMBLED    ||
                     i == O_DIRT_SLOPED_DOWN_LEFT_CRUMBLED  ||
                     i == O_DIRT_SLOPED_DOWN_RIGHT_CRUMBLED ||
                     i == O_BITER_SWITCH_1_CRUMBLED         ||
                     i == O_BITER_SWITCH_2_CRUMBLED         ||
                     i == O_BITER_SWITCH_3_CRUMBLED         ||
                     i == O_BITER_SWITCH_4_CRUMBLED         ?
                     graphic = el_act2crm(effective_element, effective_action) :
                     i == O_DIRT_DIGGING_LEFT_CRUMBLED   ||
                     i == O_DIRT_DIGGING_RIGHT_CRUMBLED  ||
                     i == O_DIRT_DIGGING_UP_CRUMBLED     ||
                     i == O_DIRT_DIGGING_DOWN_CRUMBLED   ||
                     i == O_DIRT2_DIGGING_LEFT_CRUMBLED  ||
                     i == O_DIRT2_DIGGING_RIGHT_CRUMBLED ||
                     i == O_DIRT2_DIGGING_UP_CRUMBLED    ||
                     i == O_DIRT2_DIGGING_DOWN_CRUMBLED ?
                     graphic = el_act_dir2crm(effective_element, effective_action, direction) :
                     direction == MV_NONE ?
                     el_act2img(effective_element, effective_action) :
                     el_act_dir2img(effective_element, effective_action, direction));
      struct GraphicInfo *g = &graphic_info[graphic];
      struct GraphicInfo_BD *g_bd = &graphic_info_bd_object[i][j];
      Bitmap *src_bitmap;
      int src_x, src_y;
      int sync_frame = (BD_GFX_RANGE(O_PRE_PL_1, 3, e)        ? BD_GFX_FRAME(O_PRE_PL_1, e) :
			BD_GFX_RANGE(O_PRE_DIA_1, 5, e)       ? BD_GFX_FRAME(O_PRE_DIA_1, e) :
			BD_GFX_RANGE(O_PRE_STONE_1, 4, e)     ? BD_GFX_FRAME(O_PRE_STONE_1, e) :
			BD_GFX_RANGE(O_PRE_STEEL_1, 4, e)     ? BD_GFX_FRAME(O_PRE_STEEL_1, e) :
			BD_GFX_RANGE(O_BOMB_TICK_1, 7, e)     ? BD_GFX_FRAME(O_BOMB_TICK_1, e) :
			BD_GFX_RANGE(O_BOMB_EXPL_1, 4, e)     ? BD_GFX_FRAME(O_BOMB_EXPL_1, e) :
			BD_GFX_RANGE(O_NUT_CRACK_1, 4, e)     ? BD_GFX_FRAME(O_NUT_CRACK_1, e) :
			BD_GFX_RANGE(O_GHOST_EXPL_1, 4, e)    ? BD_GFX_FRAME(O_GHOST_EXPL_1, e) :
			BD_GFX_RANGE(O_EXPLODE_1, 5, e)       ? BD_GFX_FRAME(O_EXPLODE_1, e) :
			BD_GFX_RANGE(O_PRE_CLOCK_1, 4, e)     ? BD_GFX_FRAME(O_PRE_CLOCK_1, e) :
			BD_GFX_RANGE(O_NITRO_EXPL_1, 4, e)    ? BD_GFX_FRAME(O_NITRO_EXPL_1, e) :
			BD_GFX_RANGE(O_AMOEBA_2_EXPL_1, 4, e) ? BD_GFX_FRAME(O_AMOEBA_2_EXPL_1, e):
			e == O_INBOX_OPEN                   ||
			e == O_OUTBOX_OPEN                  ||
			e == O_DIRT_DIGGING_LEFT            ||
			e == O_DIRT_DIGGING_RIGHT           ||
			e == O_DIRT_DIGGING_UP              ||
			e == O_DIRT_DIGGING_DOWN            ||
			e == O_DIRT_DIGGING_LEFT_CRUMBLED   ||
			e == O_DIRT_DIGGING_RIGHT_CRUMBLED  ||
			e == O_DIRT_DIGGING_UP_CRUMBLED     ||
			e == O_DIRT_DIGGING_DOWN_CRUMBLED   ||
			e == O_DIRT2_DIGGING_LEFT           ||
			e == O_DIRT2_DIGGING_RIGHT          ||
			e == O_DIRT2_DIGGING_UP             ||
			e == O_DIRT2_DIGGING_DOWN           ||
			e == O_DIRT2_DIGGING_LEFT_CRUMBLED  ||
			e == O_DIRT2_DIGGING_RIGHT_CRUMBLED ||
			e == O_DIRT2_DIGGING_UP_CRUMBLED    ||
			e == O_DIRT2_DIGGING_DOWN_CRUMBLED  ||
			e == O_PLAYER_BOMB_TURNING          ||
			e == O_PLAYER_ROCKET_LAUNCHER_TURNING ? j :
			j * 2);
      int frame = getAnimationFrame(g->anim_frames,
				    g->anim_delay,
				    g->anim_mode,
				    g->anim_start_frame,
				    sync_frame);

      getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, FALSE);

      g_bd->bitmap = src_bitmap;
      g_bd->src_x  = src_x;
      g_bd->src_y  = src_y;
      g_bd->width  = TILEX;
      g_bd->height = TILEY;

      g_bd->graphic = graphic;
      g_bd->frame = frame;

      g_bd->animated = IS_ANIMATED(graphic);
    }
  }
}

void InitGraphicInfo_EM(void)
{
  int i, j, p;

  // always start with reliable default values
  for (i = 0; i < GAME_TILE_MAX; i++)
  {
    em_object_mapping[i].element_rnd = EL_UNKNOWN;
    em_object_mapping[i].is_backside = FALSE;
    em_object_mapping[i].action = ACTION_DEFAULT;
    em_object_mapping[i].direction = MV_NONE;
  }

  // always start with reliable default values
  for (p = 0; p < MAX_PLAYERS; p++)
  {
    for (i = 0; i < PLY_MAX; i++)
    {
      em_player_mapping[p][i].element_rnd = EL_UNKNOWN;
      em_player_mapping[p][i].action = ACTION_DEFAULT;
      em_player_mapping[p][i].direction = MV_NONE;
    }
  }

  for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
  {
    int e = em_object_mapping_list[i].element_em;

    em_object_mapping[e].element_rnd = em_object_mapping_list[i].element_rnd;
    em_object_mapping[e].is_backside = em_object_mapping_list[i].is_backside;

    if (em_object_mapping_list[i].action != -1)
      em_object_mapping[e].action = em_object_mapping_list[i].action;

    if (em_object_mapping_list[i].direction != -1)
      em_object_mapping[e].direction =
	MV_DIR_FROM_BIT(em_object_mapping_list[i].direction);
  }

  for (i = 0; em_player_mapping_list[i].action_em != -1; i++)
  {
    int a = em_player_mapping_list[i].action_em;
    int p = em_player_mapping_list[i].player_nr;

    em_player_mapping[p][a].element_rnd = em_player_mapping_list[i].element_rnd;

    if (em_player_mapping_list[i].action != -1)
      em_player_mapping[p][a].action = em_player_mapping_list[i].action;

    if (em_player_mapping_list[i].direction != -1)
      em_player_mapping[p][a].direction =
	MV_DIR_FROM_BIT(em_player_mapping_list[i].direction);
  }

  for (i = 0; i < GAME_TILE_MAX; i++)
  {
    int element = em_object_mapping[i].element_rnd;
    int action = em_object_mapping[i].action;
    int direction = em_object_mapping[i].direction;
    boolean is_backside = em_object_mapping[i].is_backside;
    boolean action_exploding = ((action == ACTION_EXPLODING ||
				 action == ACTION_SMASHED_BY_ROCK ||
				 action == ACTION_SMASHED_BY_SPRING) &&
				element != EL_DIAMOND);
    boolean action_active = (action == ACTION_ACTIVE);
    boolean action_other = (action == ACTION_OTHER);

    for (j = 0; j < 8; j++)
    {
      int effective_element = get_effective_element_EM(i, j);
      int effective_action = (j < 7 ? action :
			      i == Xdrip_stretch ? action :
			      i == Xdrip_stretchB ? action :
			      i == Ydrip_1_s ? action :
			      i == Ydrip_1_sB ? action :
			      i == Yball_1 ? action :
			      i == Xball_2 ? action :
			      i == Yball_2 ? action :
			      i == Yball_blank ? action :
			      i == Ykey_1_blank ? action :
			      i == Ykey_2_blank ? action :
			      i == Ykey_3_blank ? action :
			      i == Ykey_4_blank ? action :
			      i == Ykey_5_blank ? action :
			      i == Ykey_6_blank ? action :
			      i == Ykey_7_blank ? action :
			      i == Ykey_8_blank ? action :
			      i == Ylenses_blank ? action :
			      i == Ymagnify_blank ? action :
			      i == Ygrass_blank ? action :
			      i == Ydirt_blank ? action :
			      i == Xsand_stonein_1 ? action :
			      i == Xsand_stonein_2 ? action :
			      i == Xsand_stonein_3 ? action :
			      i == Xsand_stonein_4 ? action :
			      i == Xsand_stoneout_1 ? action :
			      i == Xsand_stoneout_2 ? action :
			      i == Xboom_android ? ACTION_EXPLODING :
			      action_exploding ? ACTION_EXPLODING :
			      action_active ? action :
			      action_other ? action :
			      ACTION_DEFAULT);
      int graphic = (el_act_dir2img(effective_element, effective_action,
				    direction));
      int crumbled = (el_act_dir2crm(effective_element, effective_action,
				     direction));
      int base_graphic = el_act2img(effective_element, ACTION_DEFAULT);
      int base_crumbled = el_act2crm(effective_element, ACTION_DEFAULT);
      boolean has_action_graphics = (graphic != base_graphic);
      boolean has_crumbled_graphics = (base_crumbled != base_graphic);
      struct GraphicInfo *g = &graphic_info[graphic];
      struct GraphicInfo_EM *g_em = &graphic_info_em_object[i][j];
      Bitmap *src_bitmap;
      int src_x, src_y;
      // ensure to get symmetric 3-frame, 2-delay animations as used in EM
      boolean special_animation = (action != ACTION_DEFAULT &&
				   g->anim_frames == 3 &&
				   g->anim_delay == 2 &&
				   g->anim_mode & ANIM_LINEAR);
      int sync_frame = (i == Xdrip_stretch ? 7 :
			i == Xdrip_stretchB ? 7 :
			i == Ydrip_2_s ? j + 8 :
			i == Ydrip_2_sB ? j + 8 :
			i == Xacid_1 ? 0 :
			i == Xacid_2 ? 10 :
			i == Xacid_3 ? 20 :
			i == Xacid_4 ? 30 :
			i == Xacid_5 ? 40 :
			i == Xacid_6 ? 50 :
			i == Xacid_7 ? 60 :
			i == Xacid_8 ? 70 :
			i == Xfake_acid_1 ? 0 :
			i == Xfake_acid_2 ? 10 :
			i == Xfake_acid_3 ? 20 :
			i == Xfake_acid_4 ? 30 :
			i == Xfake_acid_5 ? 40 :
			i == Xfake_acid_6 ? 50 :
			i == Xfake_acid_7 ? 60 :
			i == Xfake_acid_8 ? 70 :
			i == Xfake_acid_1_player ? 0 :
			i == Xfake_acid_2_player ? 10 :
			i == Xfake_acid_3_player ? 20 :
			i == Xfake_acid_4_player ? 30 :
			i == Xfake_acid_5_player ? 40 :
			i == Xfake_acid_6_player ? 50 :
			i == Xfake_acid_7_player ? 60 :
			i == Xfake_acid_8_player ? 70 :
			i == Xball_2 ? 7 :
			i == Yball_2 ? j + 8 :
			i == Yball_blank ? j + 1 :
			i == Ykey_1_blank ? j + 1 :
			i == Ykey_2_blank ? j + 1 :
			i == Ykey_3_blank ? j + 1 :
			i == Ykey_4_blank ? j + 1 :
			i == Ykey_5_blank ? j + 1 :
			i == Ykey_6_blank ? j + 1 :
			i == Ykey_7_blank ? j + 1 :
			i == Ykey_8_blank ? j + 1 :
			i == Ylenses_blank ? j + 1 :
			i == Ymagnify_blank ? j + 1 :
			i == Ygrass_blank ? j + 1 :
			i == Ydirt_blank ? j + 1 :
			i == Xamoeba_1 ? 0 :
			i == Xamoeba_2 ? 1 :
			i == Xamoeba_3 ? 2 :
			i == Xamoeba_4 ? 3 :
			i == Xamoeba_5 ? 0 :
			i == Xamoeba_6 ? 1 :
			i == Xamoeba_7 ? 2 :
			i == Xamoeba_8 ? 3 :
			i == Xexit_2 ? j + 8 :
			i == Xexit_3 ? j + 16 :
			i == Xdynamite_1 ? 0 :
			i == Xdynamite_2 ? 8 :
			i == Xdynamite_3 ? 16 :
			i == Xdynamite_4 ? 24 :
			i == Xsand_stonein_1 ? j + 1 :
			i == Xsand_stonein_2 ? j + 9 :
			i == Xsand_stonein_3 ? j + 17 :
			i == Xsand_stonein_4 ? j + 25 :
			i == Xsand_stoneout_1 && j == 0 ? 0 :
			i == Xsand_stoneout_1 && j == 1 ? 0 :
			i == Xsand_stoneout_1 && j == 2 ? 1 :
			i == Xsand_stoneout_1 && j == 3 ? 2 :
			i == Xsand_stoneout_1 && j == 4 ? 2 :
			i == Xsand_stoneout_1 && j == 5 ? 3 :
			i == Xsand_stoneout_1 && j == 6 ? 4 :
			i == Xsand_stoneout_1 && j == 7 ? 4 :
			i == Xsand_stoneout_2 && j == 0 ? 5 :
			i == Xsand_stoneout_2 && j == 1 ? 6 :
			i == Xsand_stoneout_2 && j == 2 ? 7 :
			i == Xsand_stoneout_2 && j == 3 ? 8 :
			i == Xsand_stoneout_2 && j == 4 ? 9 :
			i == Xsand_stoneout_2 && j == 5 ? 11 :
			i == Xsand_stoneout_2 && j == 6 ? 13 :
			i == Xsand_stoneout_2 && j == 7 ? 15 :
			i == Xboom_bug && j == 1 ? 2 :
			i == Xboom_bug && j == 2 ? 2 :
			i == Xboom_bug && j == 3 ? 4 :
			i == Xboom_bug && j == 4 ? 4 :
			i == Xboom_bug && j == 5 ? 2 :
			i == Xboom_bug && j == 6 ? 2 :
			i == Xboom_bug && j == 7 ? 0 :
			i == Xboom_tank && j == 1 ? 2 :
			i == Xboom_tank && j == 2 ? 2 :
			i == Xboom_tank && j == 3 ? 4 :
			i == Xboom_tank && j == 4 ? 4 :
			i == Xboom_tank && j == 5 ? 2 :
			i == Xboom_tank && j == 6 ? 2 :
			i == Xboom_tank && j == 7 ? 0 :
			i == Xboom_android && j == 7 ? 6 :
			i == Xboom_1 && j == 1 ? 2 :
			i == Xboom_1 && j == 2 ? 2 :
			i == Xboom_1 && j == 3 ? 4 :
			i == Xboom_1 && j == 4 ? 4 :
			i == Xboom_1 && j == 5 ? 6 :
			i == Xboom_1 && j == 6 ? 6 :
			i == Xboom_1 && j == 7 ? 8 :
			i == Xboom_2 && j == 0 ? 8 :
			i == Xboom_2 && j == 1 ? 8 :
			i == Xboom_2 && j == 2 ? 10 :
			i == Xboom_2 && j == 3 ? 10 :
			i == Xboom_2 && j == 4 ? 10 :
			i == Xboom_2 && j == 5 ? 12 :
			i == Xboom_2 && j == 6 ? 12 :
			i == Xboom_2 && j == 7 ? 12 :
			special_animation && j == 4 ? 3 :
			effective_action != action ? 0 :
			j);
      int frame = getAnimationFrame(g->anim_frames,
				    g->anim_delay,
				    g->anim_mode,
				    g->anim_start_frame,
				    sync_frame);

      getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y,
			  g->double_movement && is_backside);

      g_em->bitmap = src_bitmap;
      g_em->src_x = src_x;
      g_em->src_y = src_y;
      g_em->src_offset_x = 0;
      g_em->src_offset_y = 0;
      g_em->dst_offset_x = 0;
      g_em->dst_offset_y = 0;
      g_em->width  = TILEX;
      g_em->height = TILEY;

      g_em->preserve_background = FALSE;

      set_crumbled_graphics_EM(g_em, has_crumbled_graphics, crumbled,
			       sync_frame);

      if ((!g->double_movement && (effective_action == ACTION_FALLING ||
				   effective_action == ACTION_MOVING  ||
				   effective_action == ACTION_PUSHING ||
				   effective_action == ACTION_EATING)) ||
	  (!has_action_graphics && (effective_action == ACTION_FILLING ||
				    effective_action == ACTION_EMPTYING)))
      {
	int move_dir =
	  (effective_action == ACTION_FALLING ||
	   effective_action == ACTION_FILLING ||
	   effective_action == ACTION_EMPTYING ? MV_DOWN : direction);
	int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? 1 : 0);
	int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? 1 : 0);
	int num_steps = (i == Ydrip_1_s  ? 16 :
			 i == Ydrip_1_sB ? 16 :
			 i == Ydrip_2_s  ? 16 :
			 i == Ydrip_2_sB ? 16 :
			 i == Xsand_stonein_1 ? 32 :
			 i == Xsand_stonein_2 ? 32 :
			 i == Xsand_stonein_3 ? 32 :
			 i == Xsand_stonein_4 ? 32 :
			 i == Xsand_stoneout_1 ? 16 :
			 i == Xsand_stoneout_2 ? 16 : 8);
	int cx = ABS(dx) * (TILEX / num_steps);
	int cy = ABS(dy) * (TILEY / num_steps);
	int step_frame = (i == Ydrip_2_s        ? j + 8 :
			  i == Ydrip_2_sB       ? j + 8 :
			  i == Xsand_stonein_2  ? j + 8 :
			  i == Xsand_stonein_3  ? j + 16 :
			  i == Xsand_stonein_4  ? j + 24 :
			  i == Xsand_stoneout_2 ? j + 8 : j) + 1;
	int step = (is_backside ? step_frame : num_steps - step_frame);

	if (is_backside)	// tile where movement starts
	{
	  if (dx < 0 || dy < 0)
	  {
	    g_em->src_offset_x = cx * step;
	    g_em->src_offset_y = cy * step;
	  }
	  else
	  {
	    g_em->dst_offset_x = cx * step;
	    g_em->dst_offset_y = cy * step;
	  }
	}
	else			// tile where movement ends
	{
	  if (dx < 0 || dy < 0)
	  {
	    g_em->dst_offset_x = cx * step;
	    g_em->dst_offset_y = cy * step;
	  }
	  else
	  {
	    g_em->src_offset_x = cx * step;
	    g_em->src_offset_y = cy * step;
	  }
	}

	g_em->width  = TILEX - cx * step;
	g_em->height = TILEY - cy * step;
      }

      // create unique graphic identifier to decide if tile must be redrawn
      /* bit 31 - 16 (16 bit): EM style graphic
	 bit 15 - 12 ( 4 bit): EM style frame
	 bit 11 -  6 ( 6 bit): graphic width
	 bit  5 -  0 ( 6 bit): graphic height */
      g_em->unique_identifier =
	(graphic << 16) | (frame << 12) | (g_em->width << 6) | g_em->height;
    }
  }

  for (i = 0; i < GAME_TILE_MAX; i++)
  {
    for (j = 0; j < 8; j++)
    {
      int element = em_object_mapping[i].element_rnd;
      int action = em_object_mapping[i].action;
      int direction = em_object_mapping[i].direction;
      boolean is_backside = em_object_mapping[i].is_backside;
      int graphic_action  = el_act_dir2img(element, action, direction);
      int graphic_default = el_act_dir2img(element, ACTION_DEFAULT, direction);

      if ((action == ACTION_SMASHED_BY_ROCK ||
	   action == ACTION_SMASHED_BY_SPRING ||
	   action == ACTION_EATING) &&
	  graphic_action == graphic_default)
      {
	int e = (action == ACTION_SMASHED_BY_ROCK   ? Ystone_s  :
		 action == ACTION_SMASHED_BY_SPRING ? Yspring_s :
		 direction == MV_LEFT  ? (is_backside? Yspring_wB: Yspring_w) :
		 direction == MV_RIGHT ? (is_backside? Yspring_eB: Yspring_e) :
		 Xspring);

	// no separate animation for "smashed by rock" -- use rock instead
	struct GraphicInfo_EM *g_em = &graphic_info_em_object[i][j];
	struct GraphicInfo_EM *g_xx = &graphic_info_em_object[e][j];

	g_em->bitmap		= g_xx->bitmap;
	g_em->src_x		= g_xx->src_x;
	g_em->src_y		= g_xx->src_y;
	g_em->src_offset_x	= g_xx->src_offset_x;
	g_em->src_offset_y	= g_xx->src_offset_y;
	g_em->dst_offset_x	= g_xx->dst_offset_x;
	g_em->dst_offset_y	= g_xx->dst_offset_y;
	g_em->width 		= g_xx->width;
	g_em->height		= g_xx->height;
	g_em->unique_identifier	= g_xx->unique_identifier;

	if (!is_backside)
	  g_em->preserve_background = TRUE;
      }
    }
  }

  for (p = 0; p < MAX_PLAYERS; p++)
  {
    for (i = 0; i < PLY_MAX; i++)
    {
      int element = em_player_mapping[p][i].element_rnd;
      int action = em_player_mapping[p][i].action;
      int direction = em_player_mapping[p][i].direction;

      for (j = 0; j < 8; j++)
      {
	int effective_element = element;
	int effective_action = action;
	int graphic = (direction == MV_NONE ?
		       el_act2img(effective_element, effective_action) :
		       el_act_dir2img(effective_element, effective_action,
				      direction));
	struct GraphicInfo *g = &graphic_info[graphic];
	struct GraphicInfo_EM *g_em = &graphic_info_em_player[p][i][j];
	Bitmap *src_bitmap;
	int src_x, src_y;
	int sync_frame = j;
	int frame = getAnimationFrame(g->anim_frames,
				      g->anim_delay,
				      g->anim_mode,
				      g->anim_start_frame,
				      sync_frame);

	getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, FALSE);

	g_em->bitmap = src_bitmap;
	g_em->src_x = src_x;
	g_em->src_y = src_y;
	g_em->src_offset_x = 0;
	g_em->src_offset_y = 0;
	g_em->dst_offset_x = 0;
	g_em->dst_offset_y = 0;
	g_em->width  = TILEX;
	g_em->height = TILEY;
      }
    }
  }
}

static void CheckSaveEngineSnapshot_BD(boolean frame_max,
				       boolean player_moving,
				       boolean player_snapping)
{
  if (frame_max)
  {
    if (!local_player->was_waiting)
    {
      if (!CheckSaveEngineSnapshotToList())
	return;

      local_player->was_waiting = TRUE;
    }
  }
  else if (player_moving || player_snapping)
  {
    local_player->was_waiting = FALSE;
  }
}

static void CheckSaveEngineSnapshot_EM(int frame,
				       boolean any_player_moving,
				       boolean any_player_snapping,
				       boolean any_player_dropping)
{
  if (frame == 7 && !any_player_dropping)
  {
    if (!local_player->was_waiting)
    {
      if (!CheckSaveEngineSnapshotToList())
	return;

      local_player->was_waiting = TRUE;
    }
  }
  else if (any_player_moving || any_player_snapping || any_player_dropping)
  {
    local_player->was_waiting = FALSE;
  }
}

static void CheckSaveEngineSnapshot_SP(boolean murphy_is_waiting,
				       boolean murphy_is_dropping)
{
  if (murphy_is_waiting)
  {
    if (!local_player->was_waiting)
    {
      if (!CheckSaveEngineSnapshotToList())
	return;

      local_player->was_waiting = TRUE;
    }
  }
  else
  {
    local_player->was_waiting = FALSE;
  }
}

static void CheckSaveEngineSnapshot_MM(boolean element_clicked,
				       boolean button_released)
{
  if (button_released)
  {
    if (game.snapshot.mode == SNAPSHOT_MODE_EVERY_MOVE)
      CheckSaveEngineSnapshotToList();
  }
  else if (element_clicked)
  {
    if (game.snapshot.mode != SNAPSHOT_MODE_EVERY_MOVE)
      CheckSaveEngineSnapshotToList();

    game.snapshot.changed_action = TRUE;
  }
}

boolean CheckSingleStepMode_BD(boolean frame_max,
                               boolean player_moving,
                               boolean player_snapping)
{
  if (tape.single_step && tape.recording && !tape.pausing)
    if (frame_max && FrameCounter > 6)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_BD(frame_max, player_moving, player_snapping);

  return tape.pausing;
}

boolean CheckSingleStepMode_EM(int frame,
			       boolean any_player_moving,
			       boolean any_player_snapping,
			       boolean any_player_dropping)
{
  if (tape.single_step && tape.recording && !tape.pausing)
    if (frame == 7 && !any_player_dropping && FrameCounter > 6)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_EM(frame, any_player_moving,
			     any_player_snapping, any_player_dropping);

  return tape.pausing;
}

void CheckSingleStepMode_SP(boolean murphy_is_waiting,
			    boolean murphy_is_dropping)
{
  boolean murphy_starts_dropping = FALSE;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].force_dropping)
      murphy_starts_dropping = TRUE;

  if (tape.single_step && tape.recording && !tape.pausing)
    if (murphy_is_waiting && !murphy_starts_dropping)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_SP(murphy_is_waiting, murphy_is_dropping);
}

void CheckSingleStepMode_MM(boolean element_clicked,
			    boolean button_released)
{
  if (tape.single_step && tape.recording && !tape.pausing)
    if (button_released)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_MM(element_clicked, button_released);
}

void getGraphicSource_SP(struct GraphicInfo_SP *g_sp,
			 int graphic, int sync_frame)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  getGraphicSource(graphic, frame, &g_sp->bitmap, &g_sp->src_x, &g_sp->src_y);
}

boolean isNextAnimationFrame_SP(int graphic, int sync_frame)
{
  return (IS_NEXT_FRAME(sync_frame, graphic));
}

int getGraphicInfo_Delay(int graphic)
{
  return graphic_info[graphic].anim_delay;
}

boolean getGraphicInfo_NewFrame(int x, int y, int graphic)
{
  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return FALSE;

  if (ANIM_MODE(graphic) & (ANIM_TILED | ANIM_RANDOM_STATIC))
    return FALSE;

  return TRUE;
}

void PlayMenuSoundExt(int sound)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
  else
    PlaySound(sound);
}

void PlayMenuSound(void)
{
  PlayMenuSoundExt(menu.sound[game_status]);
}

void PlayMenuSoundStereo(int sound, int stereo_position)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundExt(sound, SOUND_MAX_VOLUME, stereo_position, SND_CTRL_PLAY_LOOP);
  else
    PlaySoundStereo(sound, stereo_position);
}

void PlayMenuSoundIfLoopExt(int sound)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
}

void PlayMenuSoundIfLoop(void)
{
  PlayMenuSoundIfLoopExt(menu.sound[game_status]);
}

void PlayMenuMusicExt(int music)
{
  if (music == MUS_UNDEFINED)
    return;

  if (!setup.sound_music)
    return;

  if (IS_LOOP_MUSIC(music))
    PlayMusicLoop(music);
  else
    PlayMusic(music);
}

void PlayMenuMusic(void)
{
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(menu.music[game_status]);

  if (!strEqual(curr_music, next_music))
    PlayMenuMusicExt(menu.music[game_status]);
}

void PlayMenuSoundsAndMusic(void)
{
  PlayMenuSound();
  PlayMenuMusic();
}

static void FadeMenuSounds(void)
{
  FadeSounds();
}

static void FadeMenuMusic(void)
{
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(menu.music[game_status]);

  if (!strEqual(curr_music, next_music) || !setup.sound_music)
    FadeMusic();
}

void FadeMenuSoundsAndMusic(void)
{
  FadeMenuSounds();
  FadeMenuMusic();
}

void PlaySoundActivating(void)
{
#if 0
  PlaySound(SND_MENU_ITEM_ACTIVATING);
#endif
}

void PlaySoundSelecting(void)
{
#if 0
  PlaySound(SND_MENU_ITEM_SELECTING);
#endif
}

void ToggleAudioSampleRateIfNeeded(void)
{
  int setup_audio_sample_rate = (setup.audio_sample_rate_44100 ? 44100 : 22050);

  // if setup and audio sample rate are already matching, nothing do do
  if ((setup_audio_sample_rate == audio.sample_rate) ||
      !audio.sound_available)
    return;

#if 1
  // apparently changing the audio output sample rate does not work at runtime,
  // so currently the program has to be restarted to apply the new sample rate
  Request("Please restart the program to change audio sample rate!", REQ_CONFIRM);
#else
  SDLReopenAudio();

  // set setup value according to successfully changed audio sample rate
  setup.audio_sample_rate_44100 = (audio.sample_rate == 44100);
#endif
}

void ToggleMenuMusicIfNeeded(void)
{
  setup.sound = (setup.sound_simple || setup.sound_loops || setup.sound_music);

  SetAudioMode(setup.sound);

  if (setup.sound_music)
    PlayMenuMusic();
  else
    FadeMenuMusic();
}

void ToggleMenuSoundsIfNeeded(void)
{
  setup.sound = (setup.sound_simple || setup.sound_loops || setup.sound_music);

  SetAudioMode(setup.sound);

  if (setup.sound_loops)
    PlayMenuSound();
  else
    FadeMenuSounds();
}

void ToggleFullscreenIfNeeded(void)
{
  // if setup and video fullscreen state are already matching, nothing do do
  if (setup.fullscreen == video.fullscreen_enabled ||
      !video.fullscreen_available)
    return;

  SDLSetWindowFullscreen(setup.fullscreen);

  // set setup value according to successfully changed fullscreen mode
  setup.fullscreen = video.fullscreen_enabled;

  // required if executed from outside setup menu
  SaveSetupIfNeeded();
}

void ChangeWindowScalingIfNeeded(void)
{
  // if setup and video window scaling are already matching, nothing do do
  if (setup.window_scaling_percent == video.window_scaling_percent ||
      video.fullscreen_enabled)
    return;

  SDLSetWindowScaling(setup.window_scaling_percent);

  // set setup value according to successfully changed window scaling
  setup.window_scaling_percent = video.window_scaling_percent;

  // required if executed from outside setup menu
  SaveSetupIfNeeded();
}

void ChangeVsyncModeIfNeeded(void)
{
  int setup_vsync_mode = VSYNC_MODE_STR_TO_INT(setup.vsync_mode);
  int video_vsync_mode = video.vsync_mode;

  // if setup and video vsync mode are already matching, nothing do do
  if (setup_vsync_mode == video_vsync_mode)
    return;

  // if renderer is using OpenGL, vsync mode can directly be changed
  SDLSetScreenVsyncMode(setup.vsync_mode);

  // if vsync mode unchanged, try re-creating renderer to set vsync mode
  if (video.vsync_mode == video_vsync_mode)
  {
    Bitmap *tmp_backbuffer = CreateBitmap(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH);

    // save backbuffer content which gets lost when re-creating screen
    BlitBitmap(backbuffer, tmp_backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    // force re-creating screen and renderer to set new vsync mode
    video.fullscreen_enabled = !setup.fullscreen;

    // when creating new renderer, destroy textures linked to old renderer
    FreeAllTextures();		// needs old renderer to free the textures

    // re-create screen and renderer (including change of vsync mode)
    ChangeVideoModeIfNeeded(setup.fullscreen);

    // set setup value according to successfully changed fullscreen mode
    setup.fullscreen = video.fullscreen_enabled;

    // restore backbuffer content from temporary backbuffer backup bitmap
    BlitBitmap(tmp_backbuffer, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
    FreeBitmap(tmp_backbuffer);

    // update visible window/screen
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    // when changing vsync mode, re-create textures for new renderer
    InitImageTextures();
  }

  // set setup value according to successfully changed vsync mode
  setup.vsync_mode = VSYNC_MODE_INT_TO_STR(video.vsync_mode);
}

static void JoinRectangles(int *x, int *y, int *width, int *height,
			   int x2, int y2, int width2, int height2)
{
  // do not join with "off-screen" rectangle
  if (x2 == -1 || y2 == -1)
    return;

  *x = MIN(*x, x2);
  *y = MIN(*y, y2);
  *width = MAX(*width, width2);
  *height = MAX(*height, height2);
}

void SetAnimStatus(int anim_status_new)
{
  if (anim_status_new == GAME_MODE_MAIN)
    anim_status_new = GAME_MODE_PSEUDO_MAINONLY;
  else if (anim_status_new == GAME_MODE_NAMES)
    anim_status_new = GAME_MODE_PSEUDO_NAMESONLY;
  else if (anim_status_new == GAME_MODE_SCORES)
    anim_status_new = GAME_MODE_PSEUDO_SCORESOLD;

  global.anim_status_next = anim_status_new;

  // directly set screen modes that are entered without fading
  if ((global.anim_status      == GAME_MODE_PSEUDO_MAINONLY &&
       global.anim_status_next == GAME_MODE_PSEUDO_TYPENAME) ||
      (global.anim_status      == GAME_MODE_PSEUDO_TYPENAME &&
       global.anim_status_next == GAME_MODE_PSEUDO_MAINONLY) ||
      (global.anim_status      == GAME_MODE_PSEUDO_NAMESONLY &&
       global.anim_status_next == GAME_MODE_PSEUDO_TYPENAMES) ||
      (global.anim_status      == GAME_MODE_PSEUDO_TYPENAMES &&
       global.anim_status_next == GAME_MODE_PSEUDO_NAMESONLY))
    global.anim_status = global.anim_status_next;
}

void SetGameStatus(int game_status_new)
{
  if (game_status_new != game_status)
    game_status_last_screen = game_status;

  game_status = game_status_new;

  SetAnimStatus(game_status_new);
}

void SetFontStatus(int game_status_new)
{
  static int last_game_status = -1;

  if (game_status_new != -1)
  {
    // set game status for font use after storing last game status
    last_game_status = game_status;
    game_status = game_status_new;
  }
  else
  {
    // reset game status after font use from last stored game status
    game_status = last_game_status;
  }
}

void ResetFontStatus(void)
{
  SetFontStatus(-1);
}

void SetLevelSetInfo(char *identifier, int level_nr)
{
  setString(&levelset.identifier, identifier);

  levelset.level_nr = level_nr;
}

boolean CheckIfAllViewportsHaveChanged(void)
{
  // if game status has not changed, viewports have not changed either
  if (game_status == game_status_last)
    return FALSE;

  // check if all viewports have changed with current game status

  struct RectWithBorder *vp_playfield = &viewport.playfield[game_status];
  struct RectWithBorder *vp_door_1    = &viewport.door_1[game_status];
  struct RectWithBorder *vp_door_2    = &viewport.door_2[game_status];
  int new_real_sx	= vp_playfield->x;
  int new_real_sy	= vp_playfield->y;
  int new_full_sxsize	= vp_playfield->width;
  int new_full_sysize	= vp_playfield->height;
  int new_dx		= vp_door_1->x;
  int new_dy		= vp_door_1->y;
  int new_dxsize	= vp_door_1->width;
  int new_dysize	= vp_door_1->height;
  int new_vx		= vp_door_2->x;
  int new_vy		= vp_door_2->y;
  int new_vxsize	= vp_door_2->width;
  int new_vysize	= vp_door_2->height;

  boolean playfield_viewport_has_changed =
    (new_real_sx != REAL_SX ||
     new_real_sy != REAL_SY ||
     new_full_sxsize != FULL_SXSIZE ||
     new_full_sysize != FULL_SYSIZE);

  boolean door_1_viewport_has_changed =
    (new_dx != DX ||
     new_dy != DY ||
     new_dxsize != DXSIZE ||
     new_dysize != DYSIZE);

  boolean door_2_viewport_has_changed =
    (new_vx != VX ||
     new_vy != VY ||
     new_vxsize != VXSIZE ||
     new_vysize != VYSIZE ||
     game_status_last == GAME_MODE_EDITOR);

  return (playfield_viewport_has_changed &&
	  door_1_viewport_has_changed &&
	  door_2_viewport_has_changed);
}

boolean CheckFadeAll(void)
{
  return (CheckIfGlobalBorderHasChanged() ||
	  CheckIfAllViewportsHaveChanged());
}

void ChangeViewportPropertiesIfNeeded(void)
{
  boolean use_mini_tilesize = (level.game_engine_type == GAME_ENGINE_TYPE_MM ?
			       FALSE : setup.small_game_graphics);
  int gfx_game_mode = getGlobalGameStatus(game_status);
  int gfx_game_mode2 = (gfx_game_mode == GAME_MODE_EDITOR ? GAME_MODE_DEFAULT :
			gfx_game_mode);
  struct RectWithBorder *vp_window    = &viewport.window[gfx_game_mode];
  struct RectWithBorder *vp_playfield = &viewport.playfield[gfx_game_mode];
  struct RectWithBorder *vp_door_1    = &viewport.door_1[gfx_game_mode];
  struct RectWithBorder *vp_door_2    = &viewport.door_2[gfx_game_mode2];
  struct RectWithBorder *vp_door_3    = &viewport.door_2[GAME_MODE_EDITOR];
  int new_win_xsize	= vp_window->width;
  int new_win_ysize	= vp_window->height;
  int border_left	= vp_playfield->border_left;
  int border_right	= vp_playfield->border_right;
  int border_top	= vp_playfield->border_top;
  int border_bottom	= vp_playfield->border_bottom;
  int new_sx		= vp_playfield->x      + border_left;
  int new_sy		= vp_playfield->y      + border_top;
  int new_sxsize	= vp_playfield->width  - border_left - border_right;
  int new_sysize	= vp_playfield->height - border_top  - border_bottom;
  int new_real_sx	= vp_playfield->x;
  int new_real_sy	= vp_playfield->y;
  int new_full_sxsize	= vp_playfield->width;
  int new_full_sysize	= vp_playfield->height;
  int new_dx		= vp_door_1->x;
  int new_dy		= vp_door_1->y;
  int new_dxsize	= vp_door_1->width;
  int new_dysize	= vp_door_1->height;
  int new_vx		= vp_door_2->x;
  int new_vy		= vp_door_2->y;
  int new_vxsize	= vp_door_2->width;
  int new_vysize	= vp_door_2->height;
  int new_ex		= vp_door_3->x;
  int new_ey		= vp_door_3->y;
  int new_exsize	= vp_door_3->width;
  int new_eysize	= vp_door_3->height;
  int new_tilesize_var = (use_mini_tilesize ? MINI_TILESIZE : game.tile_size);
  int tilesize = (gfx_game_mode == GAME_MODE_PLAYING ? new_tilesize_var :
		  gfx_game_mode == GAME_MODE_EDITOR ? MINI_TILESIZE : TILESIZE);
  int new_scr_fieldx = new_sxsize / tilesize;
  int new_scr_fieldy = new_sysize / tilesize;
  int new_scr_fieldx_buffers = new_sxsize / new_tilesize_var;
  int new_scr_fieldy_buffers = new_sysize / new_tilesize_var;
  boolean init_gfx_buffers = FALSE;
  boolean init_video_buffer = FALSE;
  boolean init_gadgets_and_anims = FALSE;
  boolean init_bd_graphics = FALSE;
  boolean init_em_graphics = FALSE;

  if (new_win_xsize != WIN_XSIZE ||
      new_win_ysize != WIN_YSIZE)
  {
    WIN_XSIZE = new_win_xsize;
    WIN_YSIZE = new_win_ysize;

    init_video_buffer = TRUE;
    init_gfx_buffers = TRUE;
    init_gadgets_and_anims = TRUE;

    // Debug("tools:viewport", "video: init_video_buffer, init_gfx_buffers");
  }

  if (new_scr_fieldx != SCR_FIELDX ||
      new_scr_fieldy != SCR_FIELDY)
  {
    // this always toggles between MAIN and GAME when using small tile size

    SCR_FIELDX = new_scr_fieldx;
    SCR_FIELDY = new_scr_fieldy;

    // Debug("tools:viewport", "new_scr_fieldx != SCR_FIELDX ...");
  }

  if (new_sx != SX ||
      new_sy != SY ||
      new_dx != DX ||
      new_dy != DY ||
      new_vx != VX ||
      new_vy != VY ||
      new_ex != EX ||
      new_ey != EY ||
      new_sxsize != SXSIZE ||
      new_sysize != SYSIZE ||
      new_dxsize != DXSIZE ||
      new_dysize != DYSIZE ||
      new_vxsize != VXSIZE ||
      new_vysize != VYSIZE ||
      new_exsize != EXSIZE ||
      new_eysize != EYSIZE ||
      new_real_sx != REAL_SX ||
      new_real_sy != REAL_SY ||
      new_full_sxsize != FULL_SXSIZE ||
      new_full_sysize != FULL_SYSIZE ||
      new_tilesize_var != TILESIZE_VAR
      )
  {
    // ------------------------------------------------------------------------
    // determine next fading area for changed viewport definitions
    // ------------------------------------------------------------------------

    // start with current playfield area (default fading area)
    FADE_SX = REAL_SX;
    FADE_SY = REAL_SY;
    FADE_SXSIZE = FULL_SXSIZE;
    FADE_SYSIZE = FULL_SYSIZE;

    // add new playfield area if position or size has changed
    if (new_real_sx != REAL_SX || new_real_sy != REAL_SY ||
	new_full_sxsize != FULL_SXSIZE || new_full_sysize != FULL_SYSIZE)
    {
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     new_real_sx, new_real_sy, new_full_sxsize,new_full_sysize);
    }

    // add current and new door 1 area if position or size has changed
    if (new_dx != DX || new_dy != DY ||
	new_dxsize != DXSIZE || new_dysize != DYSIZE)
    {
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     DX, DY, DXSIZE, DYSIZE);
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     new_dx, new_dy, new_dxsize, new_dysize);
    }

    // add current and new door 2 area if position or size has changed
    if (new_vx != VX || new_vy != VY ||
	new_vxsize != VXSIZE || new_vysize != VYSIZE)
    {
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     VX, VY, VXSIZE, VYSIZE);
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     new_vx, new_vy, new_vxsize, new_vysize);
    }

    // ------------------------------------------------------------------------
    // handle changed tile size
    // ------------------------------------------------------------------------

    if (new_tilesize_var != TILESIZE_VAR)
    {
      // Debug("tools:viewport", "new_tilesize_var != TILESIZE_VAR");

      // changing tile size invalidates scroll values of engine snapshots
      FreeEngineSnapshotSingle();

      // changing tile size requires update of graphic mapping for BD/EM engine
      init_bd_graphics = TRUE;
      init_em_graphics = TRUE;
    }

    SX = new_sx;
    SY = new_sy;
    DX = new_dx;
    DY = new_dy;
    VX = new_vx;
    VY = new_vy;
    EX = new_ex;
    EY = new_ey;
    SXSIZE = new_sxsize;
    SYSIZE = new_sysize;
    DXSIZE = new_dxsize;
    DYSIZE = new_dysize;
    VXSIZE = new_vxsize;
    VYSIZE = new_vysize;
    EXSIZE = new_exsize;
    EYSIZE = new_eysize;
    REAL_SX = new_real_sx;
    REAL_SY = new_real_sy;
    FULL_SXSIZE = new_full_sxsize;
    FULL_SYSIZE = new_full_sysize;
    TILESIZE_VAR = new_tilesize_var;

    init_gfx_buffers = TRUE;
    init_gadgets_and_anims = TRUE;

    // Debug("tools:viewport", "viewports: init_gfx_buffers");
    // Debug("tools:viewport", "viewports: init_gadgets_and_anims");
  }

  if (init_gfx_buffers)
  {
    // Debug("tools:viewport", "init_gfx_buffers");

    SCR_FIELDX = new_scr_fieldx_buffers;
    SCR_FIELDY = new_scr_fieldy_buffers;

    InitGfxBuffers();

    SCR_FIELDX = new_scr_fieldx;
    SCR_FIELDY = new_scr_fieldy;

    SetDrawDeactivationMask(REDRAW_NONE);
    SetDrawBackgroundMask(REDRAW_FIELD);
  }

  if (init_video_buffer)
  {
    // Debug("tools:viewport", "init_video_buffer");

    FreeAllTextures();		// needs old renderer to free the textures

    InitVideoBuffer(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH, setup.fullscreen);
    InitImageTextures();
  }

  if (init_gadgets_and_anims)
  {
    // Debug("tools:viewport", "init_gadgets_and_anims");

    InitGadgets();
    InitGlobalAnimations();
  }

  if (init_bd_graphics)
  {
    InitGraphicInfo_BD();
  }

  if (init_em_graphics)
  {
    InitGraphicInfo_EM();
  }
}

void OpenURL(char *url)
{
#if SDL_VERSION_ATLEAST(2,0,14)
  SDL_OpenURL(url);
#else
  Warn("SDL_OpenURL(\"%s\") not supported by SDL %d.%d.%d!",
       url, SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
  Warn("Please upgrade to at least SDL 2.0.14 for URL support!");
#endif
}

void OpenURLFromHash(SetupFileHash *hash, int hash_key)
{
  OpenURL(getHashEntry(hash, int2str(hash_key, 0)));
}


// ============================================================================
// tests
// ============================================================================

#if defined(PLATFORM_WINDOWS)
/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

/*
 * timezone information is stored outside the kernel so tzp isn't used anymore.
 *
 * Note: this function is not for Win32 high precision timing purpose. See
 * elapsed_time().
 */
static int gettimeofday_windows(struct timeval * tp, struct timezone * tzp)
{
  FILETIME    file_time;
  SYSTEMTIME  system_time;
  ULARGE_INTEGER ularge;

  GetSystemTime(&system_time);
  SystemTimeToFileTime(&system_time, &file_time);
  ularge.LowPart = file_time.dwLowDateTime;
  ularge.HighPart = file_time.dwHighDateTime;

  tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
  tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

  return 0;
}
#endif

static char *test_init_uuid_random_function_simple(void)
{
  static char seed_text[100];
  unsigned int seed = InitSimpleRandom(NEW_RANDOMIZE);

  sprintf(seed_text, "%d", seed);

  return seed_text;
}

static char *test_init_uuid_random_function_better(void)
{
  static char seed_text[100];
  struct timeval current_time;

  gettimeofday(&current_time, NULL);

  prng_seed_bytes(&current_time, sizeof(current_time));

  sprintf(seed_text, "%ld.%ld",
	  (long)current_time.tv_sec,
	  (long)current_time.tv_usec);

  return seed_text;
}

#if defined(PLATFORM_WINDOWS)
static char *test_init_uuid_random_function_better_windows(void)
{
  static char seed_text[100];
  struct timeval current_time;

  gettimeofday_windows(&current_time, NULL);

  prng_seed_bytes(&current_time, sizeof(current_time));

  sprintf(seed_text, "%ld.%ld",
	  (long)current_time.tv_sec,
	  (long)current_time.tv_usec);

  return seed_text;
}
#endif

static unsigned int test_uuid_random_function_simple(int max)
{
  return GetSimpleRandom(max);
}

static unsigned int test_uuid_random_function_better(int max)
{
  return (max > 0 ? prng_get_uint() % max : 0);
}

#if defined(PLATFORM_WINDOWS)
#define NUM_UUID_TESTS			3
#else
#define NUM_UUID_TESTS			2
#endif

static void TestGeneratingUUIDs_RunTest(int nr, int always_seed, int num_uuids)
{
  HashTable *hash_seeds =
    create_hashtable(get_hash_from_string, hash_key_strings_are_equal, free, NULL);
  HashTable *hash_uuids =
    create_hashtable(get_hash_from_string, hash_key_strings_are_equal, free, NULL);
  static char message[100];
  int i;

  char *random_name = (nr == 0 ? "simple" : "better");
  char *random_type = (always_seed ? "always" : "only once");
  char *(*init_random_function)(void) =
    (nr == 0 ?
     test_init_uuid_random_function_simple :
     test_init_uuid_random_function_better);
  unsigned int (*random_function)(int) =
    (nr == 0 ?
     test_uuid_random_function_simple :
     test_uuid_random_function_better);
  int xpos = 40;

#if defined(PLATFORM_WINDOWS)
  if (nr == 2)
  {
    random_name = "windows";
    init_random_function = test_init_uuid_random_function_better_windows;
  }
#endif

  ClearField();

  DrawTextF(xpos, 40, FC_GREEN, "Test: Generating UUIDs");
  DrawTextF(xpos, 80, FC_YELLOW, "Test %d.%d:", nr + 1, always_seed + 1);

  DrawTextF(xpos, 100, FC_YELLOW, "Random Generator Name: %s", random_name);
  DrawTextF(xpos, 120, FC_YELLOW, "Seeding Random Generator: %s", random_type);
  DrawTextF(xpos, 140, FC_YELLOW, "Number of UUIDs generated: %d", num_uuids);

  DrawTextF(xpos, 180, FC_GREEN, "Please wait ...");

  BackToFront();

  // always initialize random number generator at least once
  init_random_function();

  unsigned int time_start = SDL_GetTicks();

  for (i = 0; i < num_uuids; i++)
  {
    if (always_seed)
    {
      char *seed = getStringCopy(init_random_function());

      hashtable_remove(hash_seeds, seed);
      hashtable_insert(hash_seeds, seed, "1");
    }

    char *uuid = getStringCopy(getUUIDExt(random_function));

    hashtable_remove(hash_uuids, uuid);
    hashtable_insert(hash_uuids, uuid, "1");
  }

  int num_unique_seeds = hashtable_count(hash_seeds);
  int num_unique_uuids = hashtable_count(hash_uuids);

  unsigned int time_needed = SDL_GetTicks() - time_start;

  DrawTextF(xpos, 220, FC_YELLOW, "Time needed: %d ms", time_needed);

  DrawTextF(xpos, 240, FC_YELLOW, "Number of unique UUIDs: %d", num_unique_uuids);

  if (always_seed)
    DrawTextF(xpos, 260, FC_YELLOW, "Number of unique seeds: %d", num_unique_seeds);

  if (nr == NUM_UUID_TESTS - 1 && always_seed)
    DrawTextF(xpos, 300, FC_GREEN, "All tests done!");
  else
    DrawTextF(xpos, 300, FC_GREEN, "Confirm dialog for next test ...");

  sprintf(message, "Test %d.%d finished!", nr + 1, always_seed + 1);

  Request(message, REQ_CONFIRM);

  hashtable_destroy(hash_seeds);
  hashtable_destroy(hash_uuids);
}

void TestGeneratingUUIDs(void)
{
  int num_uuids = 1000000;
  int i, j;

  for (i = 0; i < NUM_UUID_TESTS; i++)
    for (j = 0; j < 2; j++)
      TestGeneratingUUIDs_RunTest(i, j, num_uuids);

  CloseAllAndExit(0);
}
