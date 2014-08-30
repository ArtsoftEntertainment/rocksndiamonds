/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  tools.c                                                 *
***********************************************************/

#include <stdarg.h>

#ifdef __FreeBSD__
#include <machine/joystick.h>
#endif

#include "tools.h"
#include "game.h"
#include "events.h"
#include "sound.h"
#include "misc.h"
#include "buttons.h"
#include "joystick.h"
#include "cartoons.h"
#include "network.h"

#ifdef MSDOS
extern boolean wait_for_vsync;
#endif

/* tool button identifiers */
#define TOOL_CTRL_ID_YES	0
#define TOOL_CTRL_ID_NO		1
#define TOOL_CTRL_ID_CONFIRM	2
#define TOOL_CTRL_ID_PLAYER_1	3
#define TOOL_CTRL_ID_PLAYER_2	4
#define TOOL_CTRL_ID_PLAYER_3	5
#define TOOL_CTRL_ID_PLAYER_4	6

#define NUM_TOOL_BUTTONS	7

/* forward declaration for internal use */
static int getGraphicAnimationPhase(int, int, int);
static void DrawGraphicAnimationShiftedThruMask(int, int, int, int, int,
						int, int, int);
static void UnmapToolButtons();
static void HandleToolButtons(struct GadgetInfo *);

static struct GadgetInfo *tool_gadget[NUM_TOOL_BUTTONS];
static int request_gadget_id = -1;

void SetDrawtoField(int mode)
{
  if (mode == DRAW_BUFFERED && setup.soft_scrolling)
  {
    FX = TILEX;
    FY = TILEY;
    BX1 = -1;
    BY1 = -1;
    BX2 = SCR_FIELDX;
    BY2 = SCR_FIELDY;
    redraw_x1 = 1;
    redraw_y1 = 1;

    drawto_field = fieldbuffer;
  }
  else	/* DRAW_DIRECT, DRAW_BACKBUFFER */
  {
    FX = SX;
    FY = SY;
    BX1 = 0;
    BY1 = 0;
    BX2 = SCR_FIELDX - 1;
    BY2 = SCR_FIELDY - 1;
    redraw_x1 = 0;
    redraw_y1 = 0;

    drawto_field = (mode == DRAW_DIRECT ? window :  backbuffer);
  }
}

void BackToFront()
{
  int x,y;
  Drawable buffer = (drawto_field == window ? backbuffer : drawto_field);

  if (setup.direct_draw && game_status == PLAYING)
    redraw_mask &= ~REDRAW_MAIN;

  if (redraw_mask & REDRAW_TILES && redraw_tiles > REDRAWTILES_THRESHOLD)
    redraw_mask |= REDRAW_FIELD;

  if (redraw_mask & REDRAW_FIELD)
    redraw_mask &= ~REDRAW_TILES;

  if (!redraw_mask)
    return;

  /* synchronize X11 graphics at this point; if we would synchronize the
     display immediately after the buffer switching (after the XFlush),
     this could mean that we have to wait for the graphics to complete,
     although we could go on doing calculations for the next frame */

  XSync(display, FALSE);

  if (redraw_mask & REDRAW_ALL)
  {
    XCopyArea(display, backbuffer, window, gc,
	      0, 0, WIN_XSIZE, WIN_YSIZE,
	      0, 0);
    redraw_mask = 0;
  }

  if (redraw_mask & REDRAW_FIELD)
  {
    if (game_status != PLAYING || redraw_mask & REDRAW_FROM_BACKBUFFER)
    {
      XCopyArea(display, backbuffer, window, gc,
		REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		REAL_SX, REAL_SY);
    }
    else
    {
      int fx = FX, fy = FY;

      if (setup.soft_scrolling)
      {
	fx += (ScreenMovDir & (MV_LEFT | MV_RIGHT) ? ScreenGfxPos : 0);
	fy += (ScreenMovDir & (MV_UP | MV_DOWN)    ? ScreenGfxPos : 0);
      }

      if (setup.soft_scrolling ||
	  ABS(ScreenMovPos) + ScrollStepSize == TILEX ||
	  ABS(ScreenMovPos) == ScrollStepSize ||
	  redraw_tiles > REDRAWTILES_THRESHOLD)
      {
	XCopyArea(display, buffer, window, gc, fx, fy, SXSIZE, SYSIZE, SX, SY);

#ifdef DEBUG
#if 0
	printf("redrawing all (ScreenGfxPos == %d) because %s\n",
	       ScreenGfxPos,
	       (setup.soft_scrolling ?
		"setup.soft_scrolling" :
		ABS(ScreenGfxPos) + ScrollStepSize == TILEX ?
		"ABS(ScreenGfxPos) + ScrollStepSize == TILEX" :
		ABS(ScreenGfxPos) == ScrollStepSize ?
		"ABS(ScreenGfxPos) == ScrollStepSize" :
		"redraw_tiles > REDRAWTILES_THRESHOLD"));
#endif
#endif
      }
    }
    redraw_mask &= ~REDRAW_MAIN;
  }

  if (redraw_mask & REDRAW_DOORS)
  {
    if (redraw_mask & REDRAW_DOOR_1)
      XCopyArea(display, backbuffer, window, gc,
		DX, DY, DXSIZE, DYSIZE,
		DX, DY);
    if (redraw_mask & REDRAW_DOOR_2)
    {
      if ((redraw_mask & REDRAW_DOOR_2) == REDRAW_DOOR_2)
	XCopyArea(display,backbuffer,window,gc,
		  VX,VY, VXSIZE,VYSIZE,
		  VX,VY);
      else
      {
	if (redraw_mask & REDRAW_VIDEO_1)
	  XCopyArea(display,backbuffer,window,gc,
		    VX+VIDEO_DISPLAY1_XPOS,VY+VIDEO_DISPLAY1_YPOS,
		    VIDEO_DISPLAY_XSIZE,VIDEO_DISPLAY_YSIZE,
		    VX+VIDEO_DISPLAY1_XPOS,VY+VIDEO_DISPLAY1_YPOS);
	if (redraw_mask & REDRAW_VIDEO_2)
	  XCopyArea(display,backbuffer,window,gc,
		    VX+VIDEO_DISPLAY2_XPOS,VY+VIDEO_DISPLAY2_YPOS,
		    VIDEO_DISPLAY_XSIZE,VIDEO_DISPLAY_YSIZE,
		    VX+VIDEO_DISPLAY2_XPOS,VY+VIDEO_DISPLAY2_YPOS);
	if (redraw_mask & REDRAW_VIDEO_3)
	  XCopyArea(display,backbuffer,window,gc,
		    VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS,
		    VIDEO_CONTROL_XSIZE,VIDEO_CONTROL_YSIZE,
		    VX+VIDEO_CONTROL_XPOS,VY+VIDEO_CONTROL_YPOS);
      }
    }
    if (redraw_mask & REDRAW_DOOR_3)
      XCopyArea(display, backbuffer, window, gc,
		EX, EY, EXSIZE, EYSIZE,
		EX, EY);
    redraw_mask &= ~REDRAW_DOORS;
  }

  if (redraw_mask & REDRAW_MICROLEVEL)
  {
    XCopyArea(display,backbuffer,window,gc,
	      MICROLEV_XPOS, MICROLEV_YPOS, MICROLEV_XSIZE, MICROLEV_YSIZE,
	      MICROLEV_XPOS, MICROLEV_YPOS);
    XCopyArea(display,backbuffer,window,gc,
	      SX, MICROLABEL_YPOS, SXSIZE, FONT4_YSIZE,
	      SX, MICROLABEL_YPOS);
    redraw_mask &= ~REDRAW_MICROLEVEL;
  }

  if (redraw_mask & REDRAW_TILES)
  {
    for(x=0; x<SCR_FIELDX; x++)
      for(y=0; y<SCR_FIELDY; y++)
	if (redraw[redraw_x1 + x][redraw_y1 + y])
	  XCopyArea(display, buffer, window, gc,
		    FX + x * TILEX, FX + y * TILEY, TILEX, TILEY,
		    SX + x * TILEX, SY + y * TILEY);
  }

  XFlush(display);

  for(x=0; x<MAX_BUF_XSIZE; x++)
    for(y=0; y<MAX_BUF_YSIZE; y++)
      redraw[x][y] = 0;
  redraw_tiles = 0;
  redraw_mask = 0;
}

void FadeToFront()
{
/*
  long fading_delay = 300;

  if (setup.fading && (redraw_mask & REDRAW_FIELD))
  {
*/

/*
    int x,y;

    XFillRectangle(display,window,gc,
		   REAL_SX,REAL_SY,FULL_SXSIZE,FULL_SYSIZE);
    XFlush(display);

    for(i=0;i<2*FULL_SYSIZE;i++)
    {
      for(y=0;y<FULL_SYSIZE;y++)
      {
	XCopyArea(display,backbuffer,window,gc,
		  REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
      }
      XFlush(display);
      Delay(10);
    }
*/

/*
    for(i=1;i<FULL_SYSIZE;i+=2)
      XCopyArea(display,backbuffer,window,gc,
		REAL_SX,REAL_SY+i, FULL_SXSIZE,1, REAL_SX,REAL_SY+i);
    XFlush(display);
    Delay(fading_delay);
*/

/*
    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],0,0);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],-1,-1);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],0,-1);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    XSetClipOrigin(display,clip_gc[PIX_FADEMASK],-1,0);
    XCopyArea(display,backbuffer,window,clip_gc[PIX_FADEMASK],
	      REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE, REAL_SX,REAL_SY);
    XFlush(display);
    Delay(fading_delay);

    redraw_mask &= ~REDRAW_MAIN;
  }
*/

  BackToFront();
}

void ClearWindow()
{
  XFillRectangle(display, backbuffer, gc,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  if (setup.soft_scrolling && game_status == PLAYING)
  {
    XFillRectangle(display, fieldbuffer, gc, 0, 0, FXSIZE, FYSIZE);
    SetDrawtoField(DRAW_BUFFERED);
  }
  else
    SetDrawtoField(DRAW_BACKBUFFER);

  if (setup.direct_draw && game_status == PLAYING)
  {
    XFillRectangle(display, window, gc,
		   REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
    SetDrawtoField(DRAW_DIRECT);
  }

  redraw_mask |= REDRAW_FIELD;
}

int getFontWidth(int font_size, int font_type)
{
  return (font_size == FS_BIG ? FONT1_XSIZE :
	  font_size == FS_MEDIUM ? FONT6_XSIZE :
	  font_type == FC_SPECIAL1 ? FONT3_XSIZE :
	  font_type == FC_SPECIAL2 ? FONT4_XSIZE :
	  font_type == FC_SPECIAL3 ? FONT5_XSIZE :
	  FONT2_XSIZE);
}

int getFontHeight(int font_size, int font_type)
{
  return (font_size == FS_BIG ? FONT1_YSIZE :
	  font_size == FS_MEDIUM ? FONT6_YSIZE :
	  font_type == FC_SPECIAL1 ? FONT3_YSIZE :
	  font_type == FC_SPECIAL2 ? FONT4_YSIZE :
	  font_type == FC_SPECIAL3 ? FONT5_YSIZE :
	  FONT2_YSIZE);
}

void DrawInitText(char *text, int ypos, int color)
{
  if (display && window && pix[PIX_SMALLFONT])
  {
    XFillRectangle(display, window, gc, 0, ypos, WIN_XSIZE, FONT2_YSIZE);
    DrawTextExt(window, gc, (WIN_XSIZE - strlen(text) * FONT2_XSIZE)/2,
		ypos,text,FS_SMALL,color);
    XFlush(display);
  }
}

void DrawTextFCentered(int y, int font_type, char *format, ...)
{
  char buffer[FULL_SXSIZE / FONT5_XSIZE + 10];
  int font_width = getFontWidth(FS_SMALL, font_type);
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  DrawText(SX + (SXSIZE - strlen(buffer) * font_width) / 2, SY + y,
	   buffer, FS_SMALL, font_type);
}

void DrawTextF(int x, int y, int font_type, char *format, ...)
{
  char buffer[FULL_SXSIZE / FONT5_XSIZE + 10];
  va_list ap;

  va_start(ap, format);
  vsprintf(buffer, format, ap);
  va_end(ap);

  DrawText(SX + x, SY + y, buffer, FS_SMALL, font_type);
}

void DrawText(int x, int y, char *text, int font_size, int font_type)
{
  DrawTextExt(drawto, gc, x, y, text, font_size, font_type);

  if (x < DX)
    redraw_mask |= REDRAW_FIELD;
  else if (y < VY)
    redraw_mask |= REDRAW_DOOR_1;
}

void DrawTextExt(Drawable d, GC gc, int x, int y,
		 char *text, int font_size, int font_type)
{
  int font_width, font_height, font_start;
  int font_pixmap;
  boolean print_inverse = FALSE;

  if (font_size != FS_SMALL && font_size != FS_BIG && font_size != FS_MEDIUM)
    font_size = FS_SMALL;
  if (font_type < FC_RED || font_type > FC_SPECIAL3)
    font_type = FC_RED;

  font_width = getFontWidth(font_size, font_type);
  font_height = getFontHeight(font_size, font_type);

  font_pixmap = (font_size == FS_BIG ? PIX_BIGFONT :
		 font_size == FS_MEDIUM ? PIX_MEDIUMFONT :
		 PIX_SMALLFONT);
  font_start = (font_type * (font_size == FS_BIG ? FONT1_YSIZE :
			     font_size == FS_MEDIUM ? FONT6_YSIZE :
			     FONT2_YSIZE) *
		FONT_LINES_PER_FONT);

  if (font_type == FC_SPECIAL3)
    font_start += (FONT4_YSIZE - FONT2_YSIZE) * FONT_LINES_PER_FONT;

  while (*text)
  {
    char c = *text++;

    if (c == '~' && font_size == FS_SMALL)
    {
      print_inverse = TRUE;
      continue;
    }

    if (c >= 'a' && c <= 'z')
      c = 'A' + (c - 'a');
    else if (c == '�' || c == '�')
      c = 91;
    else if (c == '�' || c == '�')
      c = 92;
    else if (c == '�' || c == '�')
      c = 93;

    if (c >= 32 && c <= 95)
    {
      int src_x = ((c - 32) % FONT_CHARS_PER_LINE) * font_width;
      int src_y = ((c - 32) / FONT_CHARS_PER_LINE) * font_height + font_start;
      int dest_x = x, dest_y = y;

      if (print_inverse)
      {
	XCopyArea(display, pix[font_pixmap], d, gc,
		  FONT_CHARS_PER_LINE * font_width,
		  3 * font_height + font_start,
		  font_width, font_height, x, y);

	XSetClipOrigin(display, clip_gc[font_pixmap],
		       dest_x - src_x, dest_y - src_y);
	XCopyArea(display, pix[font_pixmap], d, clip_gc[font_pixmap],
		  0, 0, font_width, font_height, dest_x, dest_y);
      }
      else
	XCopyArea(display, pix[font_pixmap], d, gc,
		  src_x, src_y, font_width, font_height, dest_x, dest_y);
    }

    x += font_width;
  }
}

void DrawAllPlayers()
{
  int i;

  for(i=0; i<MAX_PLAYERS; i++)
    if (stored_player[i].active)
      DrawPlayer(&stored_player[i]);
}

void DrawPlayerField(int x, int y)
{
  if (!IS_PLAYER(x, y))
    return;

  DrawPlayer(PLAYERINFO(x, y));
}

void DrawPlayer(struct PlayerInfo *player)
{
  int jx = player->jx, jy = player->jy;
  int last_jx = player->last_jx, last_jy = player->last_jy;
  int next_jx = jx + (jx - last_jx), next_jy = jy + (jy - last_jy);
  int sx = SCREENX(jx), sy = SCREENY(jy);
  int sxx = 0, syy = 0;
  int element = Feld[jx][jy], last_element = Feld[last_jx][last_jy];
  int graphic, phase;
  boolean player_is_moving = (last_jx != jx || last_jy != jy ? TRUE : FALSE);

  if (!player->active || !IN_SCR_FIELD(SCREENX(last_jx), SCREENY(last_jy)))
    return;

#if DEBUG
  if (!IN_LEV_FIELD(jx,jy))
  {
    printf("DrawPlayerField(): x = %d, y = %d\n",jx,jy);
    printf("DrawPlayerField(): sx = %d, sy = %d\n",sx,sy);
    printf("DrawPlayerField(): This should never happen!\n");
    return;
  }
#endif

  if (element == EL_EXPLODING)
    return;

  /* draw things in the field the player is leaving, if needed */

  if (player_is_moving)
  {
    if (Store[last_jx][last_jy] && IS_DRAWABLE(last_element))
    {
      DrawLevelElement(last_jx, last_jy, Store[last_jx][last_jy]);
      DrawLevelFieldThruMask(last_jx, last_jy);
    }
    else if (last_element == EL_DYNAMITE_ACTIVE)
      DrawDynamite(last_jx, last_jy);
    else
      DrawLevelField(last_jx, last_jy);

    if (player->Pushing && IN_SCR_FIELD(SCREENX(next_jx), SCREENY(next_jy)))
    {
      if (player->GfxPos)
      {
  	if (Feld[next_jx][next_jy] == EL_SOKOBAN_FELD_VOLL)
  	  DrawLevelElement(next_jx, next_jy, EL_SOKOBAN_FELD_LEER);
  	else
  	  DrawLevelElement(next_jx, next_jy, EL_LEERRAUM);
      }
      else
  	DrawLevelField(next_jx, next_jy);
    }
  }

  if (!IN_SCR_FIELD(sx, sy))
    return;

  if (setup.direct_draw)
    SetDrawtoField(DRAW_BUFFERED);

  /* draw things behind the player, if needed */

  if (Store[jx][jy])
    DrawLevelElement(jx, jy, Store[jx][jy]);
  else if (!IS_ACTIVE_BOMB(element))
    DrawLevelField(jx, jy);

  /* draw player himself */

  if (game.emulation == EMU_SUPAPLEX)
  {
    static int last_dir = MV_LEFT;
    int action = (player->programmed_action ? player->programmed_action :
		  player->action);
    boolean action_moving =
      (player_is_moving ||
       ((action & (MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN)) &&
	!(action & ~(MV_LEFT | MV_RIGHT | MV_UP | MV_DOWN))));

    graphic = GFX_SP_MURPHY;

    if (player->Pushing)
    {
      if (player->MovDir == MV_LEFT)
	graphic = GFX_MURPHY_PUSH_LEFT;
      else if (player->MovDir == MV_RIGHT)
	graphic = GFX_MURPHY_PUSH_RIGHT;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_LEFT)
	graphic = GFX_MURPHY_PUSH_LEFT;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_RIGHT)
	graphic = GFX_MURPHY_PUSH_RIGHT;
    }
    else if (player->snapped)
    {
      if (player->MovDir == MV_LEFT)
	graphic = GFX_MURPHY_SNAP_LEFT;
      else if (player->MovDir == MV_RIGHT)
	graphic = GFX_MURPHY_SNAP_RIGHT;
      else if (player->MovDir == MV_UP)
	graphic = GFX_MURPHY_SNAP_UP;
      else if (player->MovDir == MV_DOWN)
	graphic = GFX_MURPHY_SNAP_DOWN;
    }
    else if (action_moving)
    {
      if (player->MovDir == MV_LEFT)
	graphic = GFX_MURPHY_GO_LEFT;
      else if (player->MovDir == MV_RIGHT)
	graphic = GFX_MURPHY_GO_RIGHT;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_LEFT)
	graphic = GFX_MURPHY_GO_LEFT;
      else if (player->MovDir & (MV_UP | MV_DOWN) && last_dir == MV_RIGHT)
	graphic = GFX_MURPHY_GO_RIGHT;

      graphic += getGraphicAnimationPhase(3, 2, ANIM_OSCILLATE);
    }

    if (player->MovDir == MV_LEFT || player->MovDir == MV_RIGHT)
      last_dir = player->MovDir;
  }
  else
  {
    if (player->MovDir == MV_LEFT)
      graphic =
	(player->Pushing ? GFX_SPIELER1_PUSH_LEFT : GFX_SPIELER1_LEFT);
    else if (player->MovDir == MV_RIGHT)
      graphic =
	(player->Pushing ? GFX_SPIELER1_PUSH_RIGHT : GFX_SPIELER1_RIGHT);
    else if (player->MovDir == MV_UP)
      graphic = GFX_SPIELER1_UP;
    else	/* MV_DOWN || MV_NO_MOVING */
      graphic = GFX_SPIELER1_DOWN;

    graphic += player->index_nr * 3 * HEROES_PER_LINE;
    graphic += player->Frame;
  }

  if (player->GfxPos)
  {
    if (player->MovDir == MV_LEFT || player->MovDir == MV_RIGHT)
      sxx = player->GfxPos;
    else
      syy = player->GfxPos;
  }

  if (!setup.soft_scrolling && ScreenMovPos)
    sxx = syy = 0;

  DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, NO_CUTTING);

  if (SHIELD_ON(player))
  {
    int graphic = (player->shield_active_time_left ? GFX2_SHIELD_ACTIVE :
		   GFX2_SHIELD_PASSIVE);

    DrawGraphicAnimationShiftedThruMask(sx, sy, sxx, syy, graphic,
					3, 8, ANIM_OSCILLATE);
  }

  if (player->Pushing && player->GfxPos)
  {
    int px = SCREENX(next_jx), py = SCREENY(next_jy);

    if (element == EL_SOKOBAN_FELD_LEER ||
	Feld[next_jx][next_jy] == EL_SOKOBAN_FELD_VOLL)
      DrawGraphicShiftedThruMask(px, py, sxx, syy, GFX_SOKOBAN_OBJEKT,
				 NO_CUTTING);
    else
    {
      int element = Feld[next_jx][next_jy];
      int graphic = el2gfx(element);

      if ((element == EL_FELSBROCKEN || element == EL_SP_ZONK) && sxx)
      {
	int phase = (player->GfxPos / (TILEX / 4));

	if (player->MovDir == MV_LEFT)
	  graphic += phase;
	else
	  graphic += (phase + 4) % 4;
      }

      DrawGraphicShifted(px, py, sxx, syy, graphic, NO_CUTTING, NO_MASKING);
    }
  }

  /* draw things in front of player (active dynamite or dynabombs) */

  if (IS_ACTIVE_BOMB(element))
  {
    graphic = el2gfx(element);

    if (element == EL_DYNAMITE_ACTIVE)
    {
      if ((phase = (96 - MovDelay[jx][jy]) / 12) > 6)
	phase = 6;
    }
    else
    {
      if ((phase = ((96 - MovDelay[jx][jy]) / 6) % 8) > 3)
	phase = 7 - phase;
    }

    if (game.emulation == EMU_SUPAPLEX)
      DrawGraphic(sx, sy, GFX_SP_DISK_RED);
    else
      DrawGraphicThruMask(sx, sy, graphic + phase);
  }

  if (player_is_moving && last_element == EL_EXPLODING)
  {
    int phase = Frame[last_jx][last_jy];
    int delay = 2;

    if (phase > 2)
      DrawGraphicThruMask(SCREENX(last_jx), SCREENY(last_jy),
			  GFX_EXPLOSION + ((phase - 1) / delay - 1));
  }

  /* draw elements that stay over the player */
  /* handle the field the player is leaving ... */
  if (player_is_moving && IS_OVER_PLAYER(last_element))
    DrawLevelField(last_jx, last_jy);
  /* ... and the field the player is entering */
  if (IS_OVER_PLAYER(element))
    DrawLevelField(jx, jy);

  if (setup.direct_draw)
  {
    int dest_x = SX + SCREENX(MIN(jx, last_jx)) * TILEX;
    int dest_y = SY + SCREENY(MIN(jy, last_jy)) * TILEY;
    int x_size = TILEX * (1 + ABS(jx - last_jx));
    int y_size = TILEY * (1 + ABS(jy - last_jy));

    XCopyArea(display, drawto_field, window, gc,
	      dest_x, dest_y, x_size, y_size, dest_x, dest_y);
    SetDrawtoField(DRAW_DIRECT);
  }

  MarkTileDirty(sx,sy);
}

static int getGraphicAnimationPhase(int frames, int delay, int mode)
{
  int phase;

  if (mode == ANIM_OSCILLATE)
  {
    int max_anim_frames = 2 * frames - 2;
    phase = (FrameCounter % (delay * max_anim_frames)) / delay;
    phase = (phase < frames ? phase : max_anim_frames - phase);
  }
  else
    phase = (FrameCounter % (delay * frames)) / delay;

  if (mode == ANIM_REVERSE)
    phase = -phase;

  return(phase);
}

void DrawGraphicAnimationExt(int x, int y, int graphic,
			     int frames, int delay, int mode, int mask_mode)
{
  int phase = getGraphicAnimationPhase(frames, delay, mode);

  if (!(FrameCounter % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    if (mask_mode == USE_MASKING)
      DrawGraphicThruMask(SCREENX(x), SCREENY(y), graphic + phase);
    else
      DrawGraphic(SCREENX(x), SCREENY(y), graphic + phase);
  }
}

void DrawGraphicAnimation(int x, int y, int graphic,
			  int frames, int delay, int mode)
{
  DrawGraphicAnimationExt(x, y, graphic, frames, delay, mode, NO_MASKING);
}

void DrawGraphicAnimationThruMask(int x, int y, int graphic,
				  int frames, int delay, int mode)
{
  DrawGraphicAnimationExt(x, y, graphic, frames, delay, mode, USE_MASKING);
}

static void DrawGraphicAnimationShiftedThruMask(int sx, int sy,
						int sxx, int syy,
						int graphic,
						int frames, int delay,
						int mode)
{
  int phase = getGraphicAnimationPhase(frames, delay, mode);

  DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic + phase, NO_CUTTING);
}

void getGraphicSource(int graphic, int *pixmap_nr, int *x, int *y)
{
  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    *pixmap_nr = PIX_BACK;
    *x = SX + (graphic % GFX_PER_LINE) * TILEX;
    *y = SY + (graphic / GFX_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSHEROES && graphic <= GFX_END_ROCKSHEROES)
  {
    graphic -= GFX_START_ROCKSHEROES;
    *pixmap_nr = PIX_HEROES;
    *x = (graphic % HEROES_PER_LINE) * TILEX;
    *y = (graphic / HEROES_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSSP && graphic <= GFX_END_ROCKSSP)
  {
    graphic -= GFX_START_ROCKSSP;
    *pixmap_nr = PIX_SP;
    *x = (graphic % SP_PER_LINE) * TILEX;
    *y = (graphic / SP_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSDC && graphic <= GFX_END_ROCKSDC)
  {
    graphic -= GFX_START_ROCKSDC;
    *pixmap_nr = PIX_DC;
    *x = (graphic % DC_PER_LINE) * TILEX;
    *y = (graphic / DC_PER_LINE) * TILEY;
  }
  else if (graphic >= GFX_START_ROCKSFONT && graphic <= GFX_END_ROCKSFONT)
  {
    graphic -= GFX_START_ROCKSFONT;
    *pixmap_nr = PIX_BIGFONT;
    *x = (graphic % FONT_CHARS_PER_LINE) * TILEX;
    *y = ((graphic / FONT_CHARS_PER_LINE) * TILEY +
	  FC_SPECIAL1 * FONT_LINES_PER_FONT * TILEY);
  }
  else
  {
    *pixmap_nr = PIX_SP;
    *x = 0;
    *y = 0;
  }
}

void DrawGraphic(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphic(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphic(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicExt(drawto_field, gc, FX + x*TILEX, FY + y*TILEY, graphic);
  MarkTileDirty(x,y);
}

void DrawGraphicExt(Drawable d, GC gc, int x, int y, int graphic)
{
  int pixmap_nr;
  int src_x, src_y;

  getGraphicSource(graphic, &pixmap_nr, &src_x, &src_y);
  XCopyArea(display, pix[pixmap_nr], d, gc,
	    src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicThruMask(): x = %d,y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicThruMask(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicThruMaskExt(drawto_field, FX + x*TILEX, FY + y*TILEY, graphic);
  MarkTileDirty(x,y);
}

void DrawGraphicThruMaskExt(Drawable d, int dest_x, int dest_y, int graphic)
{
  int tile = graphic;
  int pixmap_nr;
  int src_x, src_y;
  Pixmap src_pixmap;
  GC drawing_gc;

  if (graphic == GFX_LEERRAUM)
    return;

  getGraphicSource(graphic, &pixmap_nr, &src_x, &src_y);
  src_pixmap = pix[pixmap_nr];
  drawing_gc = clip_gc[pixmap_nr];

  if (tile_clipmask[tile] != None)
  {
    XSetClipMask(display, tile_clip_gc, tile_clipmask[tile]);
    XSetClipOrigin(display, tile_clip_gc, dest_x, dest_y);
    XCopyArea(display, src_pixmap, d, tile_clip_gc,
	      src_x, src_y, TILEX, TILEY, dest_x, dest_y);
  }
  else
  {
#if DEBUG
    printf("DrawGraphicThruMask(): tile '%d' needs clipping!\n", tile);
#endif

    XSetClipOrigin(display, drawing_gc, dest_x-src_x, dest_y-src_y);
    XCopyArea(display, src_pixmap, d, drawing_gc,
	      src_x, src_y, TILEX, TILEY, dest_x, dest_y);
  }
}

void DrawMiniGraphic(int x, int y, int graphic)
{
  DrawMiniGraphicExt(drawto,gc, SX + x*MINI_TILEX, SY + y*MINI_TILEY, graphic);
  MarkTileDirty(x/2, y/2);
}

void getMiniGraphicSource(int graphic, Pixmap *pixmap, int *x, int *y)
{
  if (graphic >= GFX_START_ROCKSSCREEN && graphic <= GFX_END_ROCKSSCREEN)
  {
    graphic -= GFX_START_ROCKSSCREEN;
    *pixmap = pix[PIX_BACK];
    *x = MINI_GFX_STARTX + (graphic % MINI_GFX_PER_LINE) * MINI_TILEX;
    *y = MINI_GFX_STARTY + (graphic / MINI_GFX_PER_LINE) * MINI_TILEY;
  }
  else if (graphic >= GFX_START_ROCKSSP && graphic <= GFX_END_ROCKSSP)
  {
    graphic -= GFX_START_ROCKSSP;
    graphic -= ((graphic / SP_PER_LINE) * SP_PER_LINE) / 2;
    *pixmap = pix[PIX_SP];
    *x = MINI_SP_STARTX + (graphic % MINI_SP_PER_LINE) * MINI_TILEX;
    *y = MINI_SP_STARTY + (graphic / MINI_SP_PER_LINE) * MINI_TILEY;
  }
  else if (graphic >= GFX_START_ROCKSDC && graphic <= GFX_END_ROCKSDC)
  {
    graphic -= GFX_START_ROCKSDC;
    *pixmap = pix[PIX_DC];
    *x = MINI_DC_STARTX + (graphic % MINI_DC_PER_LINE) * MINI_TILEX;
    *y = MINI_DC_STARTY + (graphic / MINI_DC_PER_LINE) * MINI_TILEY;
  }
  else if (graphic >= GFX_START_ROCKSFONT && graphic <= GFX_END_ROCKSFONT)
  {
    graphic -= GFX_START_ROCKSFONT;
    *pixmap = pix[PIX_SMALLFONT];
    *x = (graphic % FONT_CHARS_PER_LINE) * FONT4_XSIZE;
    *y = ((graphic / FONT_CHARS_PER_LINE) * FONT4_YSIZE +
	      FC_SPECIAL2 * FONT2_YSIZE * FONT_LINES_PER_FONT);
  }
  else
  {
    *pixmap = pix[PIX_SP];
    *x = MINI_SP_STARTX;
    *y = MINI_SP_STARTY;
  }
}

void DrawMiniGraphicExt(Drawable d, GC gc, int x, int y, int graphic)
{
  Pixmap pixmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &pixmap, &src_x, &src_y);
  XCopyArea(display, pixmap, d, gc,
	    src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

void DrawGraphicShifted(int x,int y, int dx,int dy, int graphic,
			int cut_mode, int mask_mode)
{
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;
  int src_x, src_y, dest_x, dest_y;
  int tile = graphic;
  int pixmap_nr;
  Pixmap src_pixmap;
  GC drawing_gc;

  if (graphic < 0)
  {
    DrawGraphic(x, y, graphic);
    return;
  }

  if (dx || dy)			/* Verschiebung der Grafik? */
  {
    if (x < BX1)		/* Element kommt von links ins Bild */
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		/* Element kommt von rechts ins Bild */
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x==BX1 && dx < 0)	/* Element verl��t links das Bild */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx > 0)	/* Element verl��t rechts das Bild */
      width -= dx;
    else if (dx)		/* allg. Bewegung in x-Richtung */
      MarkTileDirty(x + SIGN(dx), y);

    if (y < BY1)		/* Element kommt von oben ins Bild */
    {
      if (cut_mode==CUT_BELOW)	/* Element oberhalb des Bildes */
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		/* Element kommt von unten ins Bild */
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y==BY1 && dy < 0)	/* Element verl��t oben das Bild */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		/* Element unterhalb des Bildes */
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				/* Element verl��t unten das Bild */
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
      height -= dy;
    else if (dy)		/* allg. Bewegung in y-Richtung */
      MarkTileDirty(x, y + SIGN(dy));
  }

  getGraphicSource(graphic, &pixmap_nr, &src_x, &src_y);
  src_pixmap = pix[pixmap_nr];
  drawing_gc = clip_gc[pixmap_nr];

  src_x += cx;
  src_y += cy;

  dest_x = FX + x * TILEX + dx;
  dest_y = FY + y * TILEY + dy;

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicShifted(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted(): This should never happen!\n");
    return;
  }
#endif

  if (mask_mode == USE_MASKING)
  {
    if (tile_clipmask[tile] != None)
    {
      XSetClipMask(display, tile_clip_gc, tile_clipmask[tile]);
      XSetClipOrigin(display, tile_clip_gc, dest_x, dest_y);
      XCopyArea(display, src_pixmap, drawto_field, tile_clip_gc,
		src_x, src_y, TILEX, TILEY, dest_x, dest_y);
    }
    else
    {
#if DEBUG
      printf("DrawGraphicShifted(): tile '%d' needs clipping!\n", tile);
#endif

      XSetClipOrigin(display, drawing_gc, dest_x - src_x, dest_y - src_y);
      XCopyArea(display, src_pixmap, drawto_field, drawing_gc,
		src_x, src_y, width, height, dest_x, dest_y);
    }
  }
  else
    XCopyArea(display, src_pixmap, drawto_field, gc,
	      src_x, src_y, width, height, dest_x, dest_y);

  MarkTileDirty(x,y);
}

void DrawGraphicShiftedThruMask(int x,int y, int dx,int dy, int graphic,
				int cut_mode)
{
  DrawGraphicShifted(x,y, dx,dy, graphic, cut_mode, USE_MASKING);
}

void DrawScreenElementExt(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int graphic = el2gfx(element);
  int phase8 = ABS(MovPos[ux][uy]) / (TILEX / 8);
  int phase4 = phase8 / 2;
  int phase2  = phase8 / 4;
  int dir = MovDir[ux][uy];

  if (element == EL_PACMAN || element == EL_KAEFER || element == EL_FLIEGER)
  {
    graphic += 4 * !phase2;

    if (dir == MV_UP)
      graphic += 1;
    else if (dir == MV_LEFT)
      graphic += 2;
    else if (dir == MV_DOWN)
      graphic += 3;
  }
  else if (element == EL_SP_SNIKSNAK)
  {
    if (dir == MV_LEFT)
      graphic = GFX_SP_SNIKSNAK_LEFT;
    else if (dir == MV_RIGHT)
      graphic = GFX_SP_SNIKSNAK_RIGHT;
    else if (dir == MV_UP)
      graphic = GFX_SP_SNIKSNAK_UP;
    else
      graphic = GFX_SP_SNIKSNAK_DOWN;

    graphic += (phase8 < 4 ? phase8 : 7 - phase8);
  }
  else if (element == EL_SP_ELECTRON)
  {
    graphic = GFX2_SP_ELECTRON + getGraphicAnimationPhase(8, 2, ANIM_NORMAL);
  }
  else if (element == EL_MOLE || element == EL_PINGUIN ||
	   element == EL_SCHWEIN || element == EL_DRACHE)
  {
    if (dir == MV_LEFT)
      graphic = (element == EL_MOLE ? GFX_MOLE_LEFT :
		 element == EL_PINGUIN ? GFX_PINGUIN_LEFT :
		 element == EL_SCHWEIN ? GFX_SCHWEIN_LEFT : GFX_DRACHE_LEFT);
    else if (dir == MV_RIGHT)
      graphic = (element == EL_MOLE ? GFX_MOLE_RIGHT :
		 element == EL_PINGUIN ? GFX_PINGUIN_RIGHT :
		 element == EL_SCHWEIN ? GFX_SCHWEIN_RIGHT : GFX_DRACHE_RIGHT);
    else if (dir == MV_UP)
      graphic = (element == EL_MOLE ? GFX_MOLE_UP :
		 element == EL_PINGUIN ? GFX_PINGUIN_UP :
		 element == EL_SCHWEIN ? GFX_SCHWEIN_UP : GFX_DRACHE_UP);
    else
      graphic = (element == EL_MOLE ? GFX_MOLE_DOWN :
		 element == EL_PINGUIN ? GFX_PINGUIN_DOWN :
		 element == EL_SCHWEIN ? GFX_SCHWEIN_DOWN : GFX_DRACHE_DOWN);

    graphic += phase4;
  }
  else if (element == EL_SONDE)
  {
    graphic = GFX_SONDE_START + getGraphicAnimationPhase(8, 2, ANIM_NORMAL);
  }
  else if (element == EL_SALZSAEURE)
  {
    graphic = GFX_GEBLUBBER + getGraphicAnimationPhase(4, 10, ANIM_NORMAL);
  }
  else if (element == EL_BUTTERFLY || element == EL_FIREFLY)
  {
    graphic += !phase2;
  }
  else if ((element == EL_FELSBROCKEN || element == EL_SP_ZONK ||
	    IS_GEM(element)) && !cut_mode)
  {
    if (uy >= lev_fieldy-1 || !IS_BELT(Feld[ux][uy+1]))
    {
      if (element == EL_FELSBROCKEN || element == EL_SP_ZONK)
      {
	if (dir == MV_LEFT)
	  graphic += (4 - phase4) % 4;
	else if (dir == MV_RIGHT)
	  graphic += phase4;
	else
	  graphic += phase2 * 2;
      }
      else if (element != EL_SP_INFOTRON)
	graphic += phase2;
    }
  }
  else if (element == EL_SIEB_LEER || element == EL_SIEB2_LEER ||
	   element == EL_SIEB_VOLL || element == EL_SIEB2_VOLL)
  {
    graphic += 3 + getGraphicAnimationPhase(4, 4, ANIM_REVERSE);
  }
  else if (IS_AMOEBOID(element))
  {
    graphic = (element == EL_AMOEBE_TOT ? GFX_AMOEBE_TOT : GFX_AMOEBE_LEBT);
    graphic += (x + 2 * y + 4) % 4;
  }
  else if (element == EL_MAUER_LEBT)
  {
    boolean links_massiv = FALSE, rechts_massiv = FALSE;

    if (!IN_LEV_FIELD(ux-1, uy) || IS_MAUER(Feld[ux-1][uy]))
      links_massiv = TRUE;
    if (!IN_LEV_FIELD(ux+1, uy) || IS_MAUER(Feld[ux+1][uy]))
      rechts_massiv = TRUE;

    if (links_massiv && rechts_massiv)
      graphic = GFX_MAUERWERK;
    else if (links_massiv)
      graphic = GFX_MAUER_R;
    else if (rechts_massiv)
      graphic = GFX_MAUER_L;
  }
  else if ((element == EL_INVISIBLE_STEEL ||
	    element == EL_UNSICHTBAR ||
	    element == EL_SAND_INVISIBLE) && game.light_time_left)
  {
    graphic = (element == EL_INVISIBLE_STEEL ? GFX_INVISIBLE_STEEL_ON :
	       element == EL_UNSICHTBAR ? GFX_UNSICHTBAR_ON :
	       GFX_SAND_INVISIBLE_ON);
  }

  if (dx || dy)
    DrawGraphicShifted(x, y, dx, dy, graphic, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask(x, y, graphic);
  else
    DrawGraphic(x, y, graphic);
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

void DrawScreenElementThruMask(int x, int y, int element)
{
  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelElementThruMask(int x, int y, int element)
{
  DrawLevelElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask(int x, int y)
{
  DrawLevelElementExt(x, y, 0, 0, Feld[x][y], NO_CUTTING, USE_MASKING);
}

void ErdreichAnbroeckeln(int x, int y)
{
  int i, width, height, cx,cy;
  int ux = LEVELX(x), uy = LEVELY(y);
  int element, graphic;
  int snip = 4;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!IN_LEV_FIELD(ux, uy))
    return;

  element = Feld[ux][uy];

  if (element == EL_ERDREICH || element == EL_LANDMINE)
  {
    if (!IN_SCR_FIELD(x, y))
      return;

    graphic = GFX_ERDENRAND;

    for(i=0; i<4; i++)
    {
      int uxx, uyy;

      uxx = ux + xy[i][0];
      uyy = uy + xy[i][1];
      if (!IN_LEV_FIELD(uxx, uyy))
	element = EL_BETON;
      else
	element = Feld[uxx][uyy];

      if (element == EL_ERDREICH || element == EL_LANDMINE)
	continue;

      if (i == 1 || i == 2)
      {
	width = snip;
	height = TILEY;
	cx = (i == 2 ? TILEX - snip : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = snip;
	cx = 0;
	cy = (i == 3 ? TILEY - snip : 0);
      }

      XCopyArea(display, pix[PIX_BACK], drawto_field, gc,
		SX + (graphic % GFX_PER_LINE) * TILEX + cx,
		SY + (graphic / GFX_PER_LINE) * TILEY + cy,
		width, height, FX + x * TILEX + cx, FY + y * TILEY + cy);
    }

    MarkTileDirty(x, y);
  }
  else
  {
    graphic = GFX_ERDENRAND;

    for(i=0; i<4; i++)
    {
      int xx, yy, uxx, uyy;

      xx = x + xy[i][0];
      yy = y + xy[i][1];
      uxx = ux + xy[i][0];
      uyy = uy + xy[i][1];

      if (!IN_LEV_FIELD(uxx, uyy) ||
	  (Feld[uxx][uyy] != EL_ERDREICH && Feld[uxx][uyy] != EL_LANDMINE) ||
	  !IN_SCR_FIELD(xx, yy))
	continue;

      if (i == 1 || i == 2)
      {
	width = snip;
	height = TILEY;
	cx = (i == 1 ? TILEX - snip : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = snip;
	cx = 0;
	cy = (i==0 ? TILEY-snip : 0);
      }

      XCopyArea(display, pix[PIX_BACK], drawto_field, gc,
		SX + (graphic % GFX_PER_LINE) * TILEX + cx,
		SY + (graphic / GFX_PER_LINE) * TILEY + cy,
		width, height, FX + xx * TILEX + cx, FY + yy * TILEY + cy);

      MarkTileDirty(xx, yy);
    }
  }
}

void DrawScreenElement(int x, int y, int element)
{
  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
  ErdreichAnbroeckeln(x, y);
}

void DrawLevelElement(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField(int x, int y)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int element;

  if (!IN_LEV_FIELD(ux, uy))
  {
    if (ux < -1 || ux > lev_fieldx || uy < -1 || uy > lev_fieldy)
      element = EL_LEERRAUM;
    else
      element = BorderElement;

    DrawScreenElement(x, y, element);
    return;
  }

  element = Feld[ux][uy];

  if (IS_MOVING(ux, uy))
  {
    int horiz_move = (MovDir[ux][uy] == MV_LEFT || MovDir[ux][uy] == MV_RIGHT);
    boolean cut_mode = NO_CUTTING;

    if (Store[ux][uy] == EL_MORAST_LEER ||
	Store[ux][uy] == EL_SIEB_LEER ||
	Store[ux][uy] == EL_SIEB2_LEER ||
	Store[ux][uy] == EL_AMOEBE_NASS)
      cut_mode = CUT_ABOVE;
    else if (Store[ux][uy] == EL_MORAST_VOLL ||
	     Store[ux][uy] == EL_SIEB_VOLL ||
	     Store[ux][uy] == EL_SIEB2_VOLL)
      cut_mode = CUT_BELOW;

    if (cut_mode == CUT_ABOVE)
      DrawScreenElementShifted(x, y, 0, 0, Store[ux][uy], NO_CUTTING);
    else
      DrawScreenElement(x, y, EL_LEERRAUM);

    if (horiz_move)
      DrawScreenElementShifted(x, y, MovPos[ux][uy], 0, element, NO_CUTTING);
    else
      DrawScreenElementShifted(x, y, 0, MovPos[ux][uy], element, cut_mode);

    if (Store[ux][uy] == EL_SALZSAEURE)
      DrawLevelElementThruMask(ux, uy + 1, EL_SALZSAEURE);
  }
  else if (IS_BLOCKED(ux, uy))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;
    boolean cut_mode = NO_CUTTING;

    Blocked2Moving(ux, uy, &oldx, &oldy);
    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    if (Store[oldx][oldy] == EL_MORAST_LEER ||
	Store[oldx][oldy] == EL_SIEB_LEER ||
	Store[oldx][oldy] == EL_SIEB2_LEER ||
	Store[oldx][oldy] == EL_AMOEBE_NASS)
      cut_mode = CUT_ABOVE;

    DrawScreenElement(x, y, EL_LEERRAUM);
    element = Feld[oldx][oldy];

    if (horiz_move)
      DrawScreenElementShifted(sx,sy, MovPos[oldx][oldy],0,element,NO_CUTTING);
    else
      DrawScreenElementShifted(sx,sy, 0,MovPos[oldx][oldy],element,cut_mode);
  }
  else if (IS_DRAWABLE(element))
    DrawScreenElement(x, y, element);
  else
    DrawScreenElement(x, y, EL_LEERRAUM);
}

void DrawLevelField(int x, int y)
{
  if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenField(SCREENX(x), SCREENY(y));
  else if (IS_MOVING(x, y))
  {
    int newx,newy;

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

void DrawMiniElement(int x, int y, int element)
{
  int graphic;

  if (!element)
  {
    DrawMiniGraphic(x, y, -1);
    return;
  }

  graphic = el2gfx(element);
  DrawMiniGraphic(x, y, graphic);
}

void DrawMiniElementOrWall(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement(sx, sy, EL_LEERRAUM);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement(sx, sy, Feld[x][y]);
  else
  {
    int steel_type, steel_position;
    int border[6][2] =
    {
      { GFX_VSTEEL_UPPER_LEFT,	GFX_ISTEEL_UPPER_LEFT  },
      { GFX_VSTEEL_UPPER_RIGHT,	GFX_ISTEEL_UPPER_RIGHT },
      { GFX_VSTEEL_LOWER_LEFT,	GFX_ISTEEL_LOWER_LEFT  },
      { GFX_VSTEEL_LOWER_RIGHT,	GFX_ISTEEL_LOWER_RIGHT },
      { GFX_VSTEEL_VERTICAL,	GFX_ISTEEL_VERTICAL    },
      { GFX_VSTEEL_HORIZONTAL,	GFX_ISTEEL_HORIZONTAL  }
    };

    steel_type = (BorderElement == EL_BETON ? 0 : 1);
    steel_position = (x == -1 && y == -1			? 0 :
		      x == lev_fieldx && y == -1		? 1 :
		      x == -1 && y == lev_fieldy		? 2 :
		      x == lev_fieldx && y == lev_fieldy	? 3 :
		      x == -1 || x == lev_fieldx		? 4 :
		      y == -1 || y == lev_fieldy		? 5 : -1);

    if (steel_position != -1)
      DrawMiniGraphic(sx, sy, border[steel_position][steel_type]);
  }
}

void DrawMicroElement(int xpos, int ypos, int element)
{
  int graphic;

  if (element == EL_LEERRAUM)
    return;

  graphic = el2gfx(element);

  if (graphic >= GFX_START_ROCKSSP && graphic <= GFX_END_ROCKSSP)
  {
    graphic -= GFX_START_ROCKSSP;
    graphic -= ((graphic / SP_PER_LINE) * SP_PER_LINE) / 2;
    XCopyArea(display, pix[PIX_SP], drawto, gc,
	      MICRO_SP_STARTX + (graphic % MICRO_SP_PER_LINE) * MICRO_TILEX,
	      MICRO_SP_STARTY + (graphic / MICRO_SP_PER_LINE) * MICRO_TILEY,
	      MICRO_TILEX, MICRO_TILEY, xpos, ypos);
  }
  else if (graphic >= GFX_START_ROCKSDC && graphic <= GFX_END_ROCKSDC)
  {
    graphic -= GFX_START_ROCKSDC;
    XCopyArea(display, pix[PIX_DC], drawto, gc,
	      MICRO_DC_STARTX + (graphic % MICRO_DC_PER_LINE) * MICRO_TILEX,
	      MICRO_DC_STARTY + (graphic / MICRO_DC_PER_LINE) * MICRO_TILEY,
	      MICRO_TILEX, MICRO_TILEY, xpos, ypos);
  }
  else
    XCopyArea(display, pix[PIX_BACK], drawto, gc,
	      MICRO_GFX_STARTX + (graphic % MICRO_GFX_PER_LINE) * MICRO_TILEX,
	      MICRO_GFX_STARTY + (graphic / MICRO_GFX_PER_LINE) * MICRO_TILEY,
	      MICRO_TILEX, MICRO_TILEY, xpos, ypos);
}

void DrawLevel()
{
  int x,y;

  ClearWindow();

  for(x=BX1; x<=BX2; x++)
    for(y=BY1; y<=BY2; y++)
      DrawScreenField(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x,y;

  for(x=0; x<size_x; x++)
    for(y=0; y<size_y; y++)
      DrawMiniElementOrWall(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawMicroLevelExt(int xpos, int ypos, int from_x, int from_y)
{
  int x, y;

  XFillRectangle(display, drawto, gc,
		 xpos, ypos, MICROLEV_XSIZE, MICROLEV_YSIZE);

  if (lev_fieldx < STD_LEV_FIELDX)
    xpos += (STD_LEV_FIELDX - lev_fieldx) / 2 * MICRO_TILEX;
  if (lev_fieldy < STD_LEV_FIELDY)
    ypos += (STD_LEV_FIELDY - lev_fieldy) / 2 * MICRO_TILEY;

  xpos += MICRO_TILEX;
  ypos += MICRO_TILEY;

  for(x=-1; x<=STD_LEV_FIELDX; x++)
  {
    for(y=-1; y<=STD_LEV_FIELDY; y++)
    {
      int lx = from_x + x, ly = from_y + y;

      if (lx >= 0 && lx < lev_fieldx && ly >= 0 && ly < lev_fieldy)
	DrawMicroElement(xpos + x * MICRO_TILEX, ypos + y * MICRO_TILEY,
			 Ur[lx][ly]);
      else if (lx >= -1 && lx < lev_fieldx+1 && ly >= -1 && ly < lev_fieldy+1)
	DrawMicroElement(xpos + x * MICRO_TILEX, ypos + y * MICRO_TILEY,
			 BorderElement);
    }
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

#define MICROLABEL_EMPTY		0
#define MICROLABEL_LEVEL_NAME		1
#define MICROLABEL_CREATED_BY		2
#define MICROLABEL_LEVEL_AUTHOR		3
#define MICROLABEL_IMPORTED_FROM	4
#define MICROLABEL_LEVEL_IMPORT_INFO	5

#define MAX_MICROLABEL_SIZE		(SXSIZE / FONT4_XSIZE)

static void DrawMicroLevelLabelExt(int mode)
{
  char label_text[MAX_MICROLABEL_SIZE + 1];

  XFillRectangle(display, drawto,gc,
		 SX, MICROLABEL_YPOS, SXSIZE, FONT4_YSIZE);

  strncpy(label_text, (mode == MICROLABEL_LEVEL_NAME ? level.name :
		       mode == MICROLABEL_CREATED_BY ? "created by" :
		       mode == MICROLABEL_LEVEL_AUTHOR ? level.author :
		       mode == MICROLABEL_IMPORTED_FROM ? "imported from" :
		       mode == MICROLABEL_LEVEL_IMPORT_INFO ?
		       leveldir[leveldir_nr].imported_from : ""),
	  MAX_MICROLABEL_SIZE);
  label_text[MAX_MICROLABEL_SIZE] = '\0';

  if (strlen(label_text) > 0)
  {
    int lxpos = SX + (SXSIZE - strlen(label_text) * FONT4_XSIZE) / 2;
    int lypos = MICROLABEL_YPOS;

    DrawText(lxpos, lypos, label_text, FS_SMALL, FC_SPECIAL2);
  }

  redraw_mask |= REDRAW_MICROLEVEL;
}

void DrawMicroLevel(int xpos, int ypos, boolean restart)
{
  static unsigned long scroll_delay = 0;
  static unsigned long label_delay = 0;
  static int from_x, from_y, scroll_direction;
  static int label_state, label_counter;

  if (restart)
  {
    from_x = from_y = 0;
    scroll_direction = MV_RIGHT;
    label_state = 1;
    label_counter = 0;

    DrawMicroLevelExt(xpos, ypos, from_x, from_y);
    DrawMicroLevelLabelExt(label_state);

    /* initialize delay counters */
    DelayReached(&scroll_delay, 0);
    DelayReached(&label_delay, 0);

    return;
  }

  /* scroll micro level, if needed */
  if ((lev_fieldx > STD_LEV_FIELDX || lev_fieldy > STD_LEV_FIELDY) &&
      DelayReached(&scroll_delay, MICROLEVEL_SCROLL_DELAY))
  {
    switch (scroll_direction)
    {
      case MV_LEFT:
	if (from_x > 0)
	  from_x--;
	else
	  scroll_direction = MV_UP;
	break;

      case MV_RIGHT:
	if (from_x < lev_fieldx - STD_LEV_FIELDX)
	  from_x++;
	else
	  scroll_direction = MV_DOWN;
	break;

      case MV_UP:
	if (from_y > 0)
	  from_y--;
	else
	  scroll_direction = MV_RIGHT;
	break;

      case MV_DOWN:
	if (from_y < lev_fieldy - STD_LEV_FIELDY)
	  from_y++;
	else
	  scroll_direction = MV_LEFT;
	break;

      default:
	break;
    }

    DrawMicroLevelExt(xpos, ypos, from_x, from_y);
  }

  /* redraw micro level label, if needed */
  if (strcmp(level.name, NAMELESS_LEVEL_NAME) != 0 &&
      strcmp(level.author, ANONYMOUS_NAME) != 0 &&
      strcmp(level.author, leveldir[leveldir_nr].name) != 0 &&
      DelayReached(&label_delay, MICROLEVEL_LABEL_DELAY))
  {
    int max_label_counter = 23;

    if (leveldir[leveldir_nr].imported_from != NULL)
      max_label_counter += 14;

    label_counter = (label_counter + 1) % max_label_counter;
    label_state = (label_counter >= 0 && label_counter <= 7 ?
		   MICROLABEL_LEVEL_NAME :
		   label_counter >= 9 && label_counter <= 12 ?
		   MICROLABEL_CREATED_BY :
		   label_counter >= 14 && label_counter <= 21 ?
		   MICROLABEL_LEVEL_AUTHOR :
		   label_counter >= 23 && label_counter <= 26 ?
		   MICROLABEL_IMPORTED_FROM :
		   label_counter >= 28 && label_counter <= 35 ?
		   MICROLABEL_LEVEL_IMPORT_INFO : MICROLABEL_EMPTY);
    DrawMicroLevelLabelExt(label_state);
  }
}

int REQ_in_range(int x, int y)
{
  if (y > DY+249 && y < DY+278)
  {
    if (x > DX+1 && x < DX+48)
      return 1;
    else if (x > DX+51 && x < DX+98) 
      return 2;
  }
  return 0;
}

boolean Request(char *text, unsigned int req_state)
{
  int mx, my, ty, result = -1;
  unsigned int old_door_state;

#ifndef MSDOS
  /* pause network game while waiting for request to answer */
  if (options.network &&
      game_status == PLAYING &&
      req_state & REQUEST_WAIT_FOR)
    SendToServer_PausePlaying();
#endif

  old_door_state = GetDoorState();

  UnmapAllGadgets();

  CloseDoor(DOOR_CLOSE_1);

  /* save old door content */
  XCopyArea(display, pix[PIX_DB_DOOR], pix[PIX_DB_DOOR], gc,
	    DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE,
	    DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1);

  /* clear door drawing field */
  XFillRectangle(display, drawto, gc, DX, DY, DXSIZE, DYSIZE);

  /* write text for request */
  for(ty=0; ty<13; ty++)
  {
    int tx, tl, tc;
    char txt[256];

    if (!*text)
      break;

    for(tl=0,tx=0; tx<7; tl++,tx++)
    {
      tc = *(text + tx);
      if (!tc || tc == 32)
	break;
    }
    if (!tl)
    { 
      text++; 
      ty--; 
      continue; 
    }
    sprintf(txt, text); 
    txt[tl] = 0;
    DrawTextExt(drawto, gc,
		DX + 51 - (tl * 14)/2, DY + 8 + ty * 16,
		txt, FS_SMALL, FC_YELLOW);
    text += tl + (tc == 32 ? 1 : 0);
  }

  if (req_state & REQ_ASK)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_YES]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_NO]);
  }
  else if (req_state & REQ_CONFIRM)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_CONFIRM]);
  }
  else if (req_state & REQ_PLAYER)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_1]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_2]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_3]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_4]);
  }

  /* copy request gadgets to door backbuffer */
  XCopyArea(display, drawto, pix[PIX_DB_DOOR], gc,
	    DX, DY, DXSIZE, DYSIZE,
	    DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);

  OpenDoor(DOOR_OPEN_1);

#if 0
  ClearEventQueue();
#endif

  if (!(req_state & REQUEST_WAIT_FOR))
    return(FALSE);

  if (game_status != MAINMENU)
    InitAnimation();

  button_status = MB_RELEASED;

  request_gadget_id = -1;

  while(result < 0)
  {
    if (XPending(display))
    {
      XEvent event;

      XNextEvent(display, &event);

      switch(event.type)
      {
	case ButtonPress:
	case ButtonRelease:
	case MotionNotify:
	{
	  if (event.type == MotionNotify)
	  {
	    Window root, child;
	    int root_x, root_y;
	    int win_x, win_y;
	    unsigned int mask;

	    if (!XQueryPointer(display, window, &root, &child,
			       &root_x, &root_y, &win_x, &win_y, &mask))
	      continue;

	    if (!button_status)
	      continue;

	    motion_status = TRUE;
	    mx = ((XMotionEvent *) &event)->x;
	    my = ((XMotionEvent *) &event)->y;
	  }
	  else
	  {
	    motion_status = FALSE;
	    mx = ((XButtonEvent *) &event)->x;
	    my = ((XButtonEvent *) &event)->y;
	    if (event.type==ButtonPress)
	      button_status = ((XButtonEvent *) &event)->button;
	    else
	      button_status = MB_RELEASED;
	  }

	  /* this sets 'request_gadget_id' */
	  HandleGadgets(mx, my, button_status);

	  switch(request_gadget_id)
	  {
	    case TOOL_CTRL_ID_YES:
	      result = TRUE;
	      break;
	    case TOOL_CTRL_ID_NO:
	      result = FALSE;
	      break;
	    case TOOL_CTRL_ID_CONFIRM:
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

	  break;
	}

	case KeyPress:
	  switch(XLookupKeysym((XKeyEvent *)&event,
			       ((XKeyEvent *)&event)->state))
	  {
	    case XK_Return:
	      result = 1;
	      break;

	    case XK_Escape:
	      result = 0;
	      break;

	    default:
	      break;
	  }
	  if (req_state & REQ_PLAYER)
	    result = 0;
	  break;

	case KeyRelease:
	  key_joystick_mapping = 0;
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
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

    DoAnimation();

    /* don't eat all CPU time */
    Delay(10);
  }

  if (game_status != MAINMENU)
    StopAnimation();

  UnmapToolButtons();

  if (!(req_state & REQ_STAY_OPEN))
  {
    CloseDoor(DOOR_CLOSE_1);

    if (!(req_state & REQ_STAY_CLOSED) && (old_door_state & DOOR_OPEN_1))
    {
      XCopyArea(display,pix[PIX_DB_DOOR],pix[PIX_DB_DOOR],gc,
		DOOR_GFX_PAGEX2,DOOR_GFX_PAGEY1, DXSIZE,DYSIZE,
		DOOR_GFX_PAGEX1,DOOR_GFX_PAGEY1);
      OpenDoor(DOOR_OPEN_1);
    }
  }

  RemapAllGadgets();

#ifndef MSDOS
  /* continue network game after request */
  if (options.network &&
      game_status == PLAYING &&
      req_state & REQUEST_WAIT_FOR)
    SendToServer_ContinuePlaying();
#endif

  return(result);
}

unsigned int OpenDoor(unsigned int door_state)
{
  unsigned int new_door_state;

  if (door_state & DOOR_COPY_BACK)
  {
    XCopyArea(display, pix[PIX_DB_DOOR], pix[PIX_DB_DOOR], gc,
	      DOOR_GFX_PAGEX2, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE + VYSIZE,
	      DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);
    door_state &= ~DOOR_COPY_BACK;
  }

  new_door_state = MoveDoor(door_state);

  return(new_door_state);
}

unsigned int CloseDoor(unsigned int door_state)
{
  unsigned int new_door_state;

  XCopyArea(display, backbuffer, pix[PIX_DB_DOOR], gc,
	    DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);
  XCopyArea(display, backbuffer, pix[PIX_DB_DOOR], gc,
	    VX, VY, VXSIZE, VYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2);

  new_door_state = MoveDoor(door_state);

  return(new_door_state);
}

unsigned int GetDoorState()
{
  return(MoveDoor(DOOR_GET_STATE));
}

unsigned int MoveDoor(unsigned int door_state)
{
  static int door1 = DOOR_OPEN_1;
  static int door2 = DOOR_CLOSE_2;
  static unsigned long door_delay = 0;
  int x, start, stepsize = 2;
  unsigned long door_delay_value = stepsize * 5;

  if (door_state == DOOR_GET_STATE)
    return(door1 | door2);

  if (door1 == DOOR_OPEN_1 && door_state & DOOR_OPEN_1)
    door_state &= ~DOOR_OPEN_1;
  else if (door1 == DOOR_CLOSE_1 && door_state & DOOR_CLOSE_1)
    door_state &= ~DOOR_CLOSE_1;
  if (door2 == DOOR_OPEN_2 && door_state & DOOR_OPEN_2)
    door_state &= ~DOOR_OPEN_2;
  else if (door2 == DOOR_CLOSE_2 && door_state & DOOR_CLOSE_2)
    door_state &= ~DOOR_CLOSE_2;

  if (setup.quick_doors)
  {
    stepsize = 20;
    door_delay_value = 0;
    StopSound(SND_OEFFNEN);
  }

  if (door_state & DOOR_ACTION)
  {
    if (!(door_state & DOOR_NO_DELAY))
      PlaySoundStereo(SND_OEFFNEN, PSND_MAX_RIGHT);

    start = ((door_state & DOOR_NO_DELAY) ? DXSIZE : 0);

    for(x=start; x<=DXSIZE; x+=stepsize)
    {
      WaitUntilDelayReached(&door_delay, door_delay_value);

      if (door_state & DOOR_ACTION_1)
      {
	int i = (door_state & DOOR_OPEN_1 ? DXSIZE-x : x);
	int j = (DXSIZE - i) / 3;

	XCopyArea(display, pix[PIX_DB_DOOR], drawto, gc,
		  DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1 + i/2,
		  DXSIZE,DYSIZE - i/2, DX, DY);

	XFillRectangle(display, drawto, gc, DX, DY + DYSIZE - i/2, DXSIZE,i/2);

	XSetClipOrigin(display, clip_gc[PIX_DOOR],
		       DX - i, (DY + j) - DOOR_GFX_PAGEY1);
	XCopyArea(display, pix[PIX_DOOR], drawto,clip_gc[PIX_DOOR],
		  DXSIZE, DOOR_GFX_PAGEY1, i, 77, DX + DXSIZE - i, DY + j);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE, DOOR_GFX_PAGEY1 + 140, i, 63, DX + DXSIZE - i,
		  DY + 140 + j);
	XSetClipOrigin(display, clip_gc[PIX_DOOR],
		       DX - DXSIZE + i, DY - (DOOR_GFX_PAGEY1 + j));
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE - i, DOOR_GFX_PAGEY1 + j, i, 77 - j, DX, DY);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE-i, DOOR_GFX_PAGEY1 + 140, i, 63, DX, DY + 140 - j);

	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE - i, DOOR_GFX_PAGEY1 + 77, i, 63,
		  DX, DY + 77 - j);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE - i, DOOR_GFX_PAGEY1 + 203, i, 77,
		  DX, DY + 203 - j);
	XSetClipOrigin(display, clip_gc[PIX_DOOR],
		       DX - i, (DY + j) - DOOR_GFX_PAGEY1);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE, DOOR_GFX_PAGEY1 + 77, i, 63,
		  DX + DXSIZE - i, DY + 77 + j);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  DXSIZE, DOOR_GFX_PAGEY1 + 203, i, 77 - j,
		  DX + DXSIZE - i, DY + 203 + j);

	redraw_mask |= REDRAW_DOOR_1;
      }

      if (door_state & DOOR_ACTION_2)
      {
	int i = (door_state & DOOR_OPEN_2 ? VXSIZE - x : x);
	int j = (VXSIZE - i) / 3;

	XCopyArea(display, pix[PIX_DB_DOOR], drawto, gc,
		  DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY2 + i/2,
		  VXSIZE, VYSIZE - i/2, VX, VY);

	XFillRectangle(display, drawto, gc, VX, VY + VYSIZE-i/2, VXSIZE, i/2);

	XSetClipOrigin(display, clip_gc[PIX_DOOR],
		       VX - i, (VY + j) - DOOR_GFX_PAGEY2);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  VXSIZE, DOOR_GFX_PAGEY2, i, VYSIZE / 2, VX + VXSIZE-i, VY+j);
	XSetClipOrigin(display, clip_gc[PIX_DOOR],
		       VX - VXSIZE + i, VY - (DOOR_GFX_PAGEY2 + j));
	XCopyArea(display, pix[PIX_DOOR], drawto,clip_gc[PIX_DOOR],
		  VXSIZE - i, DOOR_GFX_PAGEY2 + j, i, VYSIZE / 2 - j, VX, VY);

	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  VXSIZE - i, DOOR_GFX_PAGEY2 + VYSIZE / 2, i, VYSIZE / 2,
		  VX, VY + VYSIZE / 2 - j);
	XSetClipOrigin(display, clip_gc[PIX_DOOR],
		       VX - i, (VY + j) - DOOR_GFX_PAGEY2);
	XCopyArea(display, pix[PIX_DOOR], drawto, clip_gc[PIX_DOOR],
		  VXSIZE, DOOR_GFX_PAGEY2 + VYSIZE / 2, i, VYSIZE / 2 - j,
		  VX + VXSIZE - i, VY + VYSIZE / 2 + j);

	redraw_mask |= REDRAW_DOOR_2;
      }

      BackToFront();

      if (game_status == MAINMENU)
	DoAnimation();
    }
  }

  if (setup.quick_doors)
    StopSound(SND_OEFFNEN);

  if (door_state & DOOR_ACTION_1)
    door1 = door_state & DOOR_ACTION_1;
  if (door_state & DOOR_ACTION_2)
    door2 = door_state & DOOR_ACTION_2;

  return (door1 | door2);
}

void DrawSpecialEditorDoor()
{
  /* draw bigger toolbox window */
  XCopyArea(display, pix[PIX_DOOR], drawto, gc,
	    DOOR_GFX_PAGEX7, 0, 108, 56, EX - 4, EY - 12);

  redraw_mask |= REDRAW_ALL;
}

void UndrawSpecialEditorDoor()
{
  /* draw normal tape recorder window */
  XCopyArea(display, pix[PIX_BACK], drawto, gc,
	    562, 344, 108, 56, EX - 4, EY - 12);

  redraw_mask |= REDRAW_ALL;
}

int ReadPixel(Drawable d, int x, int y)
{
  XImage *pixel_image;
  unsigned long pixel_value;

  pixel_image = XGetImage(display, d, x, y, 1, 1, AllPlanes, ZPixmap);
  pixel_value = XGetPixel(pixel_image, 0, 0);

  XDestroyImage(pixel_image);

  return pixel_value;
}

/* ---------- new tool button stuff ---------------------------------------- */

/* graphic position values for tool buttons */
#define TOOL_BUTTON_YES_XPOS		2
#define TOOL_BUTTON_YES_YPOS		250
#define TOOL_BUTTON_YES_GFX_YPOS	0
#define TOOL_BUTTON_YES_XSIZE		46
#define TOOL_BUTTON_YES_YSIZE		28
#define TOOL_BUTTON_NO_XPOS		52
#define TOOL_BUTTON_NO_YPOS		TOOL_BUTTON_YES_YPOS
#define TOOL_BUTTON_NO_GFX_YPOS		TOOL_BUTTON_YES_GFX_YPOS
#define TOOL_BUTTON_NO_XSIZE		TOOL_BUTTON_YES_XSIZE
#define TOOL_BUTTON_NO_YSIZE		TOOL_BUTTON_YES_YSIZE
#define TOOL_BUTTON_CONFIRM_XPOS	TOOL_BUTTON_YES_XPOS
#define TOOL_BUTTON_CONFIRM_YPOS	TOOL_BUTTON_YES_YPOS
#define TOOL_BUTTON_CONFIRM_GFX_YPOS	30
#define TOOL_BUTTON_CONFIRM_XSIZE	96
#define TOOL_BUTTON_CONFIRM_YSIZE	TOOL_BUTTON_YES_YSIZE
#define TOOL_BUTTON_PLAYER_XSIZE	30
#define TOOL_BUTTON_PLAYER_YSIZE	30
#define TOOL_BUTTON_PLAYER_GFX_XPOS	5
#define TOOL_BUTTON_PLAYER_GFX_YPOS	185
#define TOOL_BUTTON_PLAYER_XPOS		(5 + TOOL_BUTTON_PLAYER_XSIZE / 2)
#define TOOL_BUTTON_PLAYER_YPOS		(215 - TOOL_BUTTON_PLAYER_YSIZE / 2)
#define TOOL_BUTTON_PLAYER1_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 0 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER2_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 1 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER3_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 0 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER4_XPOS	(TOOL_BUTTON_PLAYER_XPOS \
					 + 1 * TOOL_BUTTON_PLAYER_XSIZE)
#define TOOL_BUTTON_PLAYER1_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 0 * TOOL_BUTTON_PLAYER_YSIZE)
#define TOOL_BUTTON_PLAYER2_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 0 * TOOL_BUTTON_PLAYER_YSIZE)
#define TOOL_BUTTON_PLAYER3_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 1 * TOOL_BUTTON_PLAYER_YSIZE)
#define TOOL_BUTTON_PLAYER4_YPOS	(TOOL_BUTTON_PLAYER_YPOS \
					 + 1 * TOOL_BUTTON_PLAYER_YSIZE)

static struct
{
  int xpos, ypos;
  int x, y;
  int width, height;
  int gadget_id;
  char *infotext;
} toolbutton_info[NUM_TOOL_BUTTONS] =
{
  {
    TOOL_BUTTON_YES_XPOS,	TOOL_BUTTON_YES_GFX_YPOS,
    TOOL_BUTTON_YES_XPOS,	TOOL_BUTTON_YES_YPOS,
    TOOL_BUTTON_YES_XSIZE,	TOOL_BUTTON_YES_YSIZE,
    TOOL_CTRL_ID_YES,
    "yes"
  },
  {
    TOOL_BUTTON_NO_XPOS,	TOOL_BUTTON_NO_GFX_YPOS,
    TOOL_BUTTON_NO_XPOS,	TOOL_BUTTON_NO_YPOS,
    TOOL_BUTTON_NO_XSIZE,	TOOL_BUTTON_NO_YSIZE,
    TOOL_CTRL_ID_NO,
    "no"
  },
  {
    TOOL_BUTTON_CONFIRM_XPOS,	TOOL_BUTTON_CONFIRM_GFX_YPOS,
    TOOL_BUTTON_CONFIRM_XPOS,	TOOL_BUTTON_CONFIRM_YPOS,
    TOOL_BUTTON_CONFIRM_XSIZE,	TOOL_BUTTON_CONFIRM_YSIZE,
    TOOL_CTRL_ID_CONFIRM,
    "confirm"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER1_XPOS,	TOOL_BUTTON_PLAYER1_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_1,
    "player 1"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER2_XPOS,	TOOL_BUTTON_PLAYER2_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_2,
    "player 2"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER3_XPOS,	TOOL_BUTTON_PLAYER3_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_3,
    "player 3"
  },
  {
    TOOL_BUTTON_PLAYER_GFX_XPOS,TOOL_BUTTON_PLAYER_GFX_YPOS,
    TOOL_BUTTON_PLAYER4_XPOS,	TOOL_BUTTON_PLAYER4_YPOS,
    TOOL_BUTTON_PLAYER_XSIZE,	TOOL_BUTTON_PLAYER_YSIZE,
    TOOL_CTRL_ID_PLAYER_4,
    "player 4"
  }
};

static void DoNotDisplayInfoText(void *ptr)
{
  return;
}

void CreateToolButtons()
{
  int i;

  for (i=0; i<NUM_TOOL_BUTTONS; i++)
  {
    Pixmap gd_pixmap = pix[PIX_DOOR];
    Pixmap deco_pixmap = None;
    int deco_x = 0, deco_y = 0, deco_xpos = 0, deco_ypos = 0;
    struct GadgetInfo *gi;
    unsigned long event_mask;
    int gd_xoffset, gd_yoffset;
    int gd_x1, gd_x2, gd_y;
    int id = i;

    event_mask = GD_EVENT_RELEASED;

    gd_xoffset = toolbutton_info[i].xpos;
    gd_yoffset = toolbutton_info[i].ypos;
    gd_x1 = DOOR_GFX_PAGEX4 + gd_xoffset;
    gd_x2 = DOOR_GFX_PAGEX3 + gd_xoffset;
    gd_y = DOOR_GFX_PAGEY1 + gd_yoffset;

    if (id >= TOOL_CTRL_ID_PLAYER_1 && id <= TOOL_CTRL_ID_PLAYER_4)
    {
      getMiniGraphicSource(GFX_SPIELER1 + id - TOOL_CTRL_ID_PLAYER_1,
			   &deco_pixmap, &deco_x, &deco_y);
      deco_xpos = (toolbutton_info[i].width - MINI_TILEX) / 2;
      deco_ypos = (toolbutton_info[i].height - MINI_TILEY) / 2;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, toolbutton_info[i].infotext,
		      GDI_X, DX + toolbutton_info[i].x,
		      GDI_Y, DY + toolbutton_info[i].y,
		      GDI_WIDTH, toolbutton_info[i].width,
		      GDI_HEIGHT, toolbutton_info[i].height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_pixmap, gd_x1, gd_y,
		      GDI_DESIGN_PRESSED, gd_pixmap, gd_x2, gd_y,
		      GDI_DECORATION_DESIGN, deco_pixmap, deco_x, deco_y,
		      GDI_DECORATION_POSITION, deco_xpos, deco_ypos,
		      GDI_DECORATION_SIZE, MINI_TILEX, MINI_TILEY,
		      GDI_DECORATION_SHIFTING, 1, 1,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleToolButtons,
		      GDI_CALLBACK_INFO, DoNotDisplayInfoText,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    tool_gadget[id] = gi;
  }
}

static void UnmapToolButtons()
{
  int i;

  for (i=0; i<NUM_TOOL_BUTTONS; i++)
    UnmapGadget(tool_gadget[i]);
}

static void HandleToolButtons(struct GadgetInfo *gi)
{
  request_gadget_id = gi->custom_id;
}

int el2gfx(int element)
{
  switch(element)
  {
    case EL_LEERRAUM:		return -1;
    case EL_ERDREICH:		return GFX_ERDREICH;
    case EL_MAUERWERK:		return GFX_MAUERWERK;
    case EL_FELSBODEN:		return GFX_FELSBODEN;
    case EL_FELSBROCKEN:	return GFX_FELSBROCKEN;
    case EL_SCHLUESSEL:		return GFX_SCHLUESSEL;
    case EL_EDELSTEIN:		return GFX_EDELSTEIN;
    case EL_AUSGANG_ZU:		return GFX_AUSGANG_ZU;
    case EL_AUSGANG_ACT:	return GFX_AUSGANG_ACT;
    case EL_AUSGANG_AUF:	return GFX_AUSGANG_AUF;
    case EL_SPIELFIGUR:		return GFX_SPIELFIGUR;
    case EL_SPIELER1:		return GFX_SPIELER1;
    case EL_SPIELER2:		return GFX_SPIELER2;
    case EL_SPIELER3:		return GFX_SPIELER3;
    case EL_SPIELER4:		return GFX_SPIELER4;
    case EL_KAEFER:		return GFX_KAEFER;
    case EL_KAEFER_RIGHT:	return GFX_KAEFER_RIGHT;
    case EL_KAEFER_UP:		return GFX_KAEFER_UP;
    case EL_KAEFER_LEFT:	return GFX_KAEFER_LEFT;
    case EL_KAEFER_DOWN:	return GFX_KAEFER_DOWN;
    case EL_FLIEGER:		return GFX_FLIEGER;
    case EL_FLIEGER_RIGHT:	return GFX_FLIEGER_RIGHT;
    case EL_FLIEGER_UP:		return GFX_FLIEGER_UP;
    case EL_FLIEGER_LEFT:	return GFX_FLIEGER_LEFT;
    case EL_FLIEGER_DOWN:	return GFX_FLIEGER_DOWN;
    case EL_BUTTERFLY:		return GFX_BUTTERFLY;
    case EL_BUTTERFLY_RIGHT:	return GFX_BUTTERFLY_RIGHT;
    case EL_BUTTERFLY_UP:	return GFX_BUTTERFLY_UP;
    case EL_BUTTERFLY_LEFT:	return GFX_BUTTERFLY_LEFT;
    case EL_BUTTERFLY_DOWN:	return GFX_BUTTERFLY_DOWN;
    case EL_FIREFLY:		return GFX_FIREFLY;
    case EL_FIREFLY_RIGHT:	return GFX_FIREFLY_RIGHT;
    case EL_FIREFLY_UP:		return GFX_FIREFLY_UP;
    case EL_FIREFLY_LEFT:	return GFX_FIREFLY_LEFT;
    case EL_FIREFLY_DOWN:	return GFX_FIREFLY_DOWN;
    case EL_MAMPFER:		return GFX_MAMPFER;
    case EL_ROBOT:		return GFX_ROBOT;
    case EL_BETON:		return GFX_BETON;
    case EL_DIAMANT:		return GFX_DIAMANT;
    case EL_MORAST_LEER:	return GFX_MORAST_LEER;
    case EL_MORAST_VOLL:	return GFX_MORAST_VOLL;
    case EL_TROPFEN:		return GFX_TROPFEN;
    case EL_BOMBE:		return GFX_BOMBE;
    case EL_SIEB_INAKTIV:	return GFX_SIEB_INAKTIV;
    case EL_SIEB_LEER:		return GFX_SIEB_LEER;
    case EL_SIEB_VOLL:		return GFX_SIEB_VOLL;
    case EL_SIEB_TOT:		return GFX_SIEB_TOT;
    case EL_SALZSAEURE:		return GFX_SALZSAEURE;
    case EL_AMOEBE_TOT:		return GFX_AMOEBE_TOT;
    case EL_AMOEBE_NASS:	return GFX_AMOEBE_NASS;
    case EL_AMOEBE_NORM:	return GFX_AMOEBE_NORM;
    case EL_AMOEBE_VOLL:	return GFX_AMOEBE_VOLL;
    case EL_AMOEBE_BD:		return GFX_AMOEBE_BD;
    case EL_AMOEBA2DIAM:	return GFX_AMOEBA2DIAM;
    case EL_KOKOSNUSS:		return GFX_KOKOSNUSS;
    case EL_LIFE:		return GFX_LIFE;
    case EL_LIFE_ASYNC:		return GFX_LIFE_ASYNC;
    case EL_DYNAMITE_ACTIVE:	return GFX_DYNAMIT;
    case EL_BADEWANNE:		return GFX_BADEWANNE;
    case EL_BADEWANNE1:		return GFX_BADEWANNE1;
    case EL_BADEWANNE2:		return GFX_BADEWANNE2;
    case EL_BADEWANNE3:		return GFX_BADEWANNE3;
    case EL_BADEWANNE4:		return GFX_BADEWANNE4;
    case EL_BADEWANNE5:		return GFX_BADEWANNE5;
    case EL_ABLENK_AUS:		return GFX_ABLENK_AUS;
    case EL_ABLENK_EIN:		return GFX_ABLENK_EIN;
    case EL_SCHLUESSEL1:	return GFX_SCHLUESSEL1;
    case EL_SCHLUESSEL2:	return GFX_SCHLUESSEL2;
    case EL_SCHLUESSEL3:	return GFX_SCHLUESSEL3;
    case EL_SCHLUESSEL4:	return GFX_SCHLUESSEL4;
    case EL_PFORTE1:		return GFX_PFORTE1;
    case EL_PFORTE2:		return GFX_PFORTE2;
    case EL_PFORTE3:		return GFX_PFORTE3;
    case EL_PFORTE4:		return GFX_PFORTE4;
    case EL_PFORTE1X:		return GFX_PFORTE1X;
    case EL_PFORTE2X:		return GFX_PFORTE2X;
    case EL_PFORTE3X:		return GFX_PFORTE3X;
    case EL_PFORTE4X:		return GFX_PFORTE4X;
    case EL_DYNAMITE_INACTIVE:	return GFX_DYNAMIT_AUS;
    case EL_PACMAN:		return GFX_PACMAN;
    case EL_PACMAN_RIGHT:	return GFX_PACMAN_RIGHT;
    case EL_PACMAN_UP:		return GFX_PACMAN_UP;
    case EL_PACMAN_LEFT:	return GFX_PACMAN_LEFT;
    case EL_PACMAN_DOWN:	return GFX_PACMAN_DOWN;
    case EL_UNSICHTBAR:		return GFX_UNSICHTBAR;
    case EL_ERZ_EDEL:		return GFX_ERZ_EDEL;
    case EL_ERZ_DIAM:		return GFX_ERZ_DIAM;
    case EL_BIRNE_AUS:		return GFX_BIRNE_AUS;
    case EL_BIRNE_EIN:		return GFX_BIRNE_EIN;
    case EL_ZEIT_VOLL:		return GFX_ZEIT_VOLL;
    case EL_ZEIT_LEER:		return GFX_ZEIT_LEER;
    case EL_MAUER_LEBT:		return GFX_MAUER_LEBT;
    case EL_MAUER_X:		return GFX_MAUER_X;
    case EL_MAUER_Y:		return GFX_MAUER_Y;
    case EL_MAUER_XY:		return GFX_MAUER_XY;
    case EL_EDELSTEIN_BD:	return GFX_EDELSTEIN_BD;
    case EL_EDELSTEIN_GELB:	return GFX_EDELSTEIN_GELB;
    case EL_EDELSTEIN_ROT:	return GFX_EDELSTEIN_ROT;
    case EL_EDELSTEIN_LILA:	return GFX_EDELSTEIN_LILA;
    case EL_ERZ_EDEL_BD:	return GFX_ERZ_EDEL_BD;
    case EL_ERZ_EDEL_GELB:	return GFX_ERZ_EDEL_GELB;
    case EL_ERZ_EDEL_ROT:	return GFX_ERZ_EDEL_ROT;
    case EL_ERZ_EDEL_LILA:	return GFX_ERZ_EDEL_LILA;
    case EL_MAMPFER2:		return GFX_MAMPFER2;
    case EL_SIEB2_INAKTIV:	return GFX_SIEB2_INAKTIV;
    case EL_SIEB2_LEER:		return GFX_SIEB2_LEER;
    case EL_SIEB2_VOLL:		return GFX_SIEB2_VOLL;
    case EL_SIEB2_TOT:		return GFX_SIEB2_TOT;
    case EL_DYNABOMB_ACTIVE_1:	return GFX_DYNABOMB;
    case EL_DYNABOMB_ACTIVE_2:	return GFX_DYNABOMB;
    case EL_DYNABOMB_ACTIVE_3:	return GFX_DYNABOMB;
    case EL_DYNABOMB_ACTIVE_4:	return GFX_DYNABOMB;
    case EL_DYNABOMB_NR:	return GFX_DYNABOMB_NR;
    case EL_DYNABOMB_SZ:	return GFX_DYNABOMB_SZ;
    case EL_DYNABOMB_XL:	return GFX_DYNABOMB_XL;
    case EL_SOKOBAN_OBJEKT:	return GFX_SOKOBAN_OBJEKT;
    case EL_SOKOBAN_FELD_LEER:	return GFX_SOKOBAN_FELD_LEER;
    case EL_SOKOBAN_FELD_VOLL:	return GFX_SOKOBAN_FELD_VOLL;
    case EL_MOLE:		return GFX_MOLE;
    case EL_PINGUIN:		return GFX_PINGUIN;
    case EL_SCHWEIN:		return GFX_SCHWEIN;
    case EL_DRACHE:		return GFX_DRACHE;
    case EL_SONDE:		return GFX_SONDE;
    case EL_PFEIL_LEFT:		return GFX_PFEIL_LEFT;
    case EL_PFEIL_RIGHT:	return GFX_PFEIL_RIGHT;
    case EL_PFEIL_UP:		return GFX_PFEIL_UP;
    case EL_PFEIL_DOWN:		return GFX_PFEIL_DOWN;
    case EL_SPEED_PILL:		return GFX_SPEED_PILL;
    case EL_SP_TERMINAL_ACTIVE:	return GFX_SP_TERMINAL;
    case EL_SP_BUG_ACTIVE:	return GFX_SP_BUG_ACTIVE;
    case EL_SP_ZONK:		return GFX_SP_ZONK;
      /* ^^^^^^^^^^ non-standard position in supaplex graphic set! */
    case EL_INVISIBLE_STEEL:	return GFX_INVISIBLE_STEEL;
    case EL_BLACK_ORB:		return GFX_BLACK_ORB;
    case EL_EM_GATE_1:		return GFX_EM_GATE_1;
    case EL_EM_GATE_2:		return GFX_EM_GATE_2;
    case EL_EM_GATE_3:		return GFX_EM_GATE_3;
    case EL_EM_GATE_4:		return GFX_EM_GATE_4;
    case EL_EM_GATE_1X:		return GFX_EM_GATE_1X;
    case EL_EM_GATE_2X:		return GFX_EM_GATE_2X;
    case EL_EM_GATE_3X:		return GFX_EM_GATE_3X;
    case EL_EM_GATE_4X:		return GFX_EM_GATE_4X;
    case EL_EM_KEY_1_FILE:	return GFX_EM_KEY_1;
    case EL_EM_KEY_2_FILE:	return GFX_EM_KEY_2;
    case EL_EM_KEY_3_FILE:	return GFX_EM_KEY_3;
    case EL_EM_KEY_4_FILE:	return GFX_EM_KEY_4;
    case EL_EM_KEY_1:		return GFX_EM_KEY_1;
    case EL_EM_KEY_2:		return GFX_EM_KEY_2;
    case EL_EM_KEY_3:		return GFX_EM_KEY_3;
    case EL_EM_KEY_4:		return GFX_EM_KEY_4;
    case EL_PEARL:		return GFX_PEARL;
    case EL_CRYSTAL:		return GFX_CRYSTAL;
    case EL_WALL_PEARL:		return GFX_WALL_PEARL;
    case EL_WALL_CRYSTAL:	return GFX_WALL_CRYSTAL;
    case EL_DOOR_WHITE:		return GFX_DOOR_WHITE;
    case EL_DOOR_WHITE_GRAY:	return GFX_DOOR_WHITE_GRAY;
    case EL_KEY_WHITE:		return GFX_KEY_WHITE;
    case EL_SHIELD_PASSIVE:	return GFX_SHIELD_PASSIVE;
    case EL_SHIELD_ACTIVE:	return GFX_SHIELD_ACTIVE;
    case EL_EXTRA_TIME:		return GFX_EXTRA_TIME;
    case EL_SWITCHGATE_OPEN:	return GFX_SWITCHGATE_OPEN;
    case EL_SWITCHGATE_CLOSED:	return GFX_SWITCHGATE_CLOSED;
    case EL_SWITCHGATE_SWITCH_1:return GFX_SWITCHGATE_SWITCH_1;
    case EL_SWITCHGATE_SWITCH_2:return GFX_SWITCHGATE_SWITCH_2;
    case EL_BELT1_LEFT:		return GFX_BELT1_LEFT;
    case EL_BELT1_MIDDLE:	return GFX_BELT1_MIDDLE;
    case EL_BELT1_RIGHT:	return GFX_BELT1_RIGHT;
    case EL_BELT1_SWITCH_LEFT:	return GFX_BELT1_SWITCH_LEFT;
    case EL_BELT1_SWITCH_MIDDLE:return GFX_BELT1_SWITCH_MIDDLE;
    case EL_BELT1_SWITCH_RIGHT:	return GFX_BELT1_SWITCH_RIGHT;
    case EL_BELT2_LEFT:		return GFX_BELT2_LEFT;
    case EL_BELT2_MIDDLE:	return GFX_BELT2_MIDDLE;
    case EL_BELT2_RIGHT:	return GFX_BELT2_RIGHT;
    case EL_BELT2_SWITCH_LEFT:	return GFX_BELT2_SWITCH_LEFT;
    case EL_BELT2_SWITCH_MIDDLE:return GFX_BELT2_SWITCH_MIDDLE;
    case EL_BELT2_SWITCH_RIGHT:	return GFX_BELT2_SWITCH_RIGHT;
    case EL_BELT3_LEFT:		return GFX_BELT3_LEFT;
    case EL_BELT3_MIDDLE:	return GFX_BELT3_MIDDLE;
    case EL_BELT3_RIGHT:	return GFX_BELT3_RIGHT;
    case EL_BELT3_SWITCH_LEFT:	return GFX_BELT3_SWITCH_LEFT;
    case EL_BELT3_SWITCH_MIDDLE:return GFX_BELT3_SWITCH_MIDDLE;
    case EL_BELT3_SWITCH_RIGHT:	return GFX_BELT3_SWITCH_RIGHT;
    case EL_BELT4_LEFT:		return GFX_BELT4_LEFT;
    case EL_BELT4_MIDDLE:	return GFX_BELT4_MIDDLE;
    case EL_BELT4_RIGHT:	return GFX_BELT4_RIGHT;
    case EL_BELT4_SWITCH_LEFT:	return GFX_BELT4_SWITCH_LEFT;
    case EL_BELT4_SWITCH_MIDDLE:return GFX_BELT4_SWITCH_MIDDLE;
    case EL_BELT4_SWITCH_RIGHT:	return GFX_BELT4_SWITCH_RIGHT;
    case EL_LANDMINE:		return GFX_LANDMINE;
    case EL_ENVELOPE:		return GFX_ENVELOPE;
    case EL_LIGHT_SWITCH_OFF:	return GFX_LIGHT_SWITCH_OFF;
    case EL_LIGHT_SWITCH_ON:	return GFX_LIGHT_SWITCH_ON;
    case EL_SIGN_EXCLAMATION:	return GFX_SIGN_EXCLAMATION;
    case EL_SIGN_RADIOACTIVITY:	return GFX_SIGN_RADIOACTIVITY;
    case EL_SIGN_STOP:		return GFX_SIGN_STOP;
    case EL_SIGN_WHEELCHAIR:	return GFX_SIGN_WHEELCHAIR;
    case EL_SIGN_PARKING:	return GFX_SIGN_PARKING;
    case EL_SIGN_ONEWAY:	return GFX_SIGN_ONEWAY;
    case EL_SIGN_HEART:		return GFX_SIGN_HEART;
    case EL_SIGN_TRIANGLE:	return GFX_SIGN_TRIANGLE;
    case EL_SIGN_ROUND:		return GFX_SIGN_ROUND;
    case EL_SIGN_EXIT:		return GFX_SIGN_EXIT;
    case EL_SIGN_YINYANG:	return GFX_SIGN_YINYANG;
    case EL_SIGN_OTHER:		return GFX_SIGN_OTHER;
    case EL_MOLE_LEFT:		return GFX_MOLE_LEFT;
    case EL_MOLE_RIGHT:		return GFX_MOLE_RIGHT;
    case EL_MOLE_UP:		return GFX_MOLE_UP;
    case EL_MOLE_DOWN:		return GFX_MOLE_DOWN;
    case EL_STEEL_SLANTED:	return GFX_STEEL_SLANTED;
    case EL_SAND_INVISIBLE:	return GFX_SAND_INVISIBLE;
    case EL_DX_UNKNOWN_15:	return GFX_DX_UNKNOWN_15;
    case EL_DX_UNKNOWN_42:	return GFX_DX_UNKNOWN_42;
    case EL_TIMEGATE_OPEN:	return GFX_TIMEGATE_OPEN;
    case EL_TIMEGATE_CLOSED:	return GFX_TIMEGATE_CLOSED;
    case EL_TIMEGATE_SWITCH_ON:	return GFX_TIMEGATE_SWITCH;
    case EL_TIMEGATE_SWITCH_OFF:return GFX_TIMEGATE_SWITCH;
    case EL_BALLOON:		return GFX_BALLOON;
    case EL_BALLOON_SEND_LEFT:	return GFX_BALLOON_SEND_LEFT;
    case EL_BALLOON_SEND_RIGHT:	return GFX_BALLOON_SEND_RIGHT;
    case EL_BALLOON_SEND_UP:	return GFX_BALLOON_SEND_UP;
    case EL_BALLOON_SEND_DOWN:	return GFX_BALLOON_SEND_DOWN;
    case EL_BALLOON_SEND_ANY:	return GFX_BALLOON_SEND_ANY;
    case EL_EMC_STEEL_WALL_1:	return GFX_EMC_STEEL_WALL_1;
    case EL_EMC_STEEL_WALL_2:	return GFX_EMC_STEEL_WALL_2;
    case EL_EMC_STEEL_WALL_3:	return GFX_EMC_STEEL_WALL_3;
    case EL_EMC_STEEL_WALL_4:	return GFX_EMC_STEEL_WALL_4;
    case EL_EMC_WALL_1:		return GFX_EMC_WALL_1;
    case EL_EMC_WALL_2:		return GFX_EMC_WALL_2;
    case EL_EMC_WALL_3:		return GFX_EMC_WALL_3;
    case EL_EMC_WALL_4:		return GFX_EMC_WALL_4;
    case EL_EMC_WALL_5:		return GFX_EMC_WALL_5;
    case EL_EMC_WALL_6:		return GFX_EMC_WALL_6;
    case EL_EMC_WALL_7:		return GFX_EMC_WALL_7;
    case EL_EMC_WALL_8:		return GFX_EMC_WALL_8;

    default:
    {
      if (IS_CHAR(element))
	return GFX_CHAR_START + (element - EL_CHAR_START);
      else if (element >= EL_SP_START && element <= EL_SP_END)
      {
	int nr_element = element - EL_SP_START;
	int gfx_per_line = 8;
	int nr_graphic =
	  (nr_element / gfx_per_line) * SP_PER_LINE +
	  (nr_element % gfx_per_line);

	return GFX_START_ROCKSSP + nr_graphic;
      }
      else
	return -1;
    }
  }
}
