/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* cartoons.c                                               *
***********************************************************/

#include "cartoons.h"
#include "main.h"
#include "tools.h"


/* values for toon definition */
#define NUM_TOONS	18

#define DWARF_XSIZE	40
#define DWARF_YSIZE	48
#define DWARF_X		2
#define DWARF_Y		72
#define DWARF2_Y	186
#define DWARF_FRAMES	8
#define DWARF_FPS	10
#define DWARF_STEPSIZE	4
#define JUMPER_XSIZE	48
#define JUMPER_YSIZE	56
#define JUMPER_X	2
#define JUMPER_Y	125
#define JUMPER_FRAMES	8
#define JUMPER_FPS	10
#define JUMPER_STEPSIZE	4
#define CLOWN_XSIZE	80
#define CLOWN_YSIZE	110
#define CLOWN_X		327
#define CLOWN_Y		10
#define CLOWN_FRAMES	1
#define CLOWN_FPS	10
#define CLOWN_STEPSIZE	4
#define BIRD_XSIZE	32
#define BIRD_YSIZE	30
#define BIRD1_X		2
#define BIRD1_Y		2
#define BIRD2_X		2
#define BIRD2_Y		37
#define BIRD_FRAMES	8
#define BIRD_FPS	20
#define BIRD_STEPSIZE	4

#define GAMETOON_XSIZE		TILEX
#define GAMETOON_YSIZE		TILEY
#define GAMETOON_FRAMES_4	4
#define GAMETOON_FRAMES_8	8
#define GAMETOON_FPS		20
#define GAMETOON_STEPSIZE	4

struct ToonInfo toons[NUM_TOONS] =
{
  {
    PIX_TOONS,
    DWARF_XSIZE, DWARF_YSIZE,
    DWARF_X, DWARF_Y,
    DWARF_FRAMES,
    DWARF_FPS,
    DWARF_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_TOONS,
    DWARF_XSIZE, DWARF_YSIZE,
    DWARF_X, DWARF2_Y,
    DWARF_FRAMES,
    DWARF_FPS,
    DWARF_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    PIX_TOONS,
    JUMPER_XSIZE, JUMPER_YSIZE,
    JUMPER_X, JUMPER_Y,
    JUMPER_FRAMES,
    JUMPER_FPS,
    JUMPER_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_TOONS,
    CLOWN_XSIZE, CLOWN_YSIZE,
    CLOWN_X, CLOWN_Y,
    CLOWN_FRAMES,
    CLOWN_FPS,
    CLOWN_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_UP,
    ANIMPOS_ANY
  },
  {
    PIX_TOONS,
    BIRD_XSIZE, BIRD_YSIZE,
    BIRD1_X, BIRD1_Y,
    BIRD_FRAMES,
    BIRD_FPS,
    BIRD_STEPSIZE,
    ANIM_PINGPONG,
    ANIMDIR_RIGHT,
    ANIMPOS_UPPER
  },
  {
    PIX_TOONS,
    BIRD_XSIZE, BIRD_YSIZE,
    BIRD2_X, BIRD2_Y,
    BIRD_FRAMES,
    BIRD_FPS,
    BIRD_STEPSIZE,
    ANIM_PINGPONG,
    ANIMDIR_LEFT,
    ANIMPOS_UPPER
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_SPIELER1_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_SPIELER1_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_SPIELER1_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_SPIELER1_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_PINGUIN_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_PINGUIN_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_PINGUIN_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_PINGUIN_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_MOLE_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_MOLE_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_MOLE_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_MOLE_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_SCHWEIN_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_SCHWEIN_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_SCHWEIN_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_SCHWEIN_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_DRACHE_LEFT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_DRACHE_LEFT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_DRACHE_RIGHT - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_DRACHE_RIGHT - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_4,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_DOWN
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_SONDE - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_SONDE - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_8,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_LEFT,
    ANIMPOS_ANY
  },
  {
    PIX_HEROES,
    GAMETOON_XSIZE, GAMETOON_YSIZE,
    ((GFX_SONDE - GFX_START_ROCKSHEROES) % HEROES_PER_LINE)*TILEX,
    ((GFX_SONDE - GFX_START_ROCKSHEROES) / HEROES_PER_LINE)*TILEY,
    GAMETOON_FRAMES_8,
    GAMETOON_FPS,
    GAMETOON_STEPSIZE,
    ANIM_LOOP,
    ANIMDIR_RIGHT,
    ANIMPOS_ANY
  },
};

static void PrepareBackbuffer()
{
  /* Fill empty backbuffer for animation functions */
  if (setup.direct_draw && game_status == PLAYING)
  {
    int xx,yy;

    SetDrawtoField(DRAW_BACKBUFFER);

    for(xx=0; xx<SCR_FIELDX; xx++)
      for(yy=0; yy<SCR_FIELDY; yy++)
	DrawScreenField(xx,yy);
    DrawAllPlayers();

    SetDrawtoField(DRAW_DIRECT);
  }

  if (setup.soft_scrolling && game_status == PLAYING)
  {
    int fx = FX, fy = FY;

    fx += (ScreenMovDir & (MV_LEFT|MV_RIGHT) ? ScreenGfxPos : 0);
    fy += (ScreenMovDir & (MV_UP|MV_DOWN)    ? ScreenGfxPos : 0);

    BlitBitmap(fieldbuffer, backbuffer, fx,fy, SXSIZE,SYSIZE, SX,SY);
  }
}

boolean ToonNeedsRedraw()
{
  return (game_status == HELPSCREEN ||
	  (game_status == MAINMENU &&
	   ((redraw_mask & REDRAW_MICROLEVEL) ||
	    (redraw_mask & REDRAW_MICROLABEL))));
}

void InitToons()
{
  InitToonScreen(pix, pix[PIX_DB_DOOR],
		 BackToFront, PrepareBackbuffer, ToonNeedsRedraw,
		 toons, NUM_TOONS,
		 REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
}
