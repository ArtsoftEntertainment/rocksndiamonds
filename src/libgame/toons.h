/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* toons.h                                                  *
***********************************************************/

#ifndef TOONS_H
#define TOONS_H

#include "system.h"


/* values for animation mode (frame order and direction) */
#define ANIM_NONE		0
#define ANIM_LOOP		(1 << 0)
#define ANIM_LINEAR		(1 << 1)
#define ANIM_PINGPONG		(1 << 2)
#define ANIM_PINGPONG2		(1 << 3)
#define ANIM_REVERSE		(1 << 4)

/* values for toon animation direction */
#define ANIMDIR_LEFT	1
#define ANIMDIR_RIGHT	2
#define ANIMDIR_UP	4
#define ANIMDIR_DOWN	8

#define ANIMPOS_ANY	0
#define ANIMPOS_LEFT	1
#define ANIMPOS_RIGHT	2
#define ANIMPOS_UP	4
#define ANIMPOS_DOWN	8
#define ANIMPOS_UPPER	16


struct ToonScreenInfo
{
  Bitmap **toon_bitmap_array;
  Bitmap *save_buffer;
  void (*update_function)(void);
  void (*prepare_backbuffer_function)(void);
  boolean (*redraw_needed_function)(void);

  struct ToonInfo *toons;
  int num_toons;

  int startx, starty;
  int width, height;
};

struct ToonInfo
{
  int bitmap_nr;
  int width, height;
  int src_x, src_y;
  int frames;
  int frames_per_second;
  int stepsize;
  int mode;
  int direction;
  int position;
};


void InitToonScreen();
void InitAnimation(void);
void StopAnimation(void);
void DoAnimation(void);

#endif	/* TOONS_H */
