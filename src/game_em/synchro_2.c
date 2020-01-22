/* second part of synchro.
 *
 * game logic for monsters.
 *
 * one giant switch statement to process everything.
 *
 * this whole thing is a major bottleneck. the compiler must use registers.
 * compilers suck.
 */

#include "main_em.h"


#define SPRING_ROLL	/* spring rolling off round things continues to roll */

#define RANDOM_RAW	(seed = seed << 31 | seed >> 1)
#define RANDOM(x)	(RANDOM_RAW & (x - 1))

static unsigned int seed;
static int score;

static void set_nearest_player_xy(int x, int y, int *dx, int *dy)
{
  int distance, distance_shortest = EM_MAX_CAVE_WIDTH + EM_MAX_CAVE_HEIGHT;
  int i;

  /* default values if no players are alive anymore */
  *dx = 0;
  *dy = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (!ply[i].alive)
      continue;

    distance = ABS(ply[i].x - x) + ABS(ply[i].y - y);

    if (distance < distance_shortest)
    {
      *dx = ply[i].x;
      *dy = ply[i].y;

      distance_shortest = distance;
    }
  }
}

static void Lboom_bug(int x, int y)
{
  Next[y][x] = Znormal;
  Boom[y-1][x-1] = Xemerald;
  Boom[y-1][x] = Xemerald;
  Boom[y-1][x+1] = Xemerald;
  Boom[y][x-1] = Xemerald;
  Boom[y][x] = Xdiamond;
  Boom[y][x+1] = Xemerald;
  Boom[y+1][x-1] = Xemerald;
  Boom[y+1][x] = Xemerald;
  Boom[y+1][x+1] = Xemerald;

#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, element);
#endif
}

static void Lboom_tank(int x, int y)
{
  Next[y][x] = Znormal;
  Boom[y-1][x-1] = Xblank;
  Boom[y-1][x] = Xblank;
  Boom[y-1][x+1] = Xblank;
  Boom[y][x-1] = Xblank;
  Boom[y][x] = Xblank;
  Boom[y][x+1] = Xblank;
  Boom[y+1][x-1] = Xblank;
  Boom[y+1][x] = Xblank;
  Boom[y+1][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, element);
#endif
}

static void Landroid(int x, int y)
{
  int dx, dy, temp;

  if (lev.android_clone_cnt == 0)
  {
    if (Cave[y-1][x-1] != Xblank &&
	Cave[y-1][x]   != Xblank &&
	Cave[y-1][x+1] != Xblank &&
	Cave[y][x-1]   != Xblank &&
	Cave[y][x+1]   != Xblank &&
	Cave[y+1][x-1] != Xblank &&
	Cave[y+1][x]   != Xblank &&
	Cave[y+1][x+1] != Xblank)
      goto android_move;

    switch (RANDOM(8))
    {
      /* randomly find an object to clone */

      case 0: /* S,NE,W,NW,SE,E,SW,N */
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	goto android_move;

      case 1: /* NW,SE,N,S,NE,SW,E,W */
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	goto android_move;

      case 2: /* SW,E,S,W,N,NW,SE,NE */
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	goto android_move;

      case 3: /* N,SE,NE,E,W,S,NW,SW */
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	goto android_move;

      case 4: /* SE,NW,E,NE,SW,W,N,S */
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	goto android_move;

      case 5: /* NE,W,SE,SW,S,N,E,NW */
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	goto android_move;

      case 6: /* E,N,SW,S,NW,NE,SE,W */
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	goto android_move;

      case 7: /* W,SW,NW,N,E,SE,NE,S */
	temp = lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	temp = lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	goto android_move;
    }

    Next[y][x] = temp;	/* the item we chose to clone */
    play_element_sound(x, y, SOUND_android_clone, temp);

    switch (RANDOM(8))
    {
      /* randomly find a direction to move */

      case 0: /* S,NE,W,NW,SE,E,SW,N */
	if (Cave[y+1][x] == Xblank)   goto android_s;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	goto android_move;

      case 1: /* NW,SE,N,S,NE,SW,E,W */
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	goto android_move;

      case 2: /* SW,E,S,W,N,NW,SE,NE */
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	goto android_move;

      case 3: /* N,SE,NE,E,W,S,NW,SW */
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	goto android_move;

      case 4: /* SE,NW,E,NE,SW,W,N,S */
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	goto android_move;

      case 5: /* NE,W,SE,SW,S,N,E,NW */
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	goto android_move;

      case 6: /* E,N,SW,S,NW,NE,SE,W */
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y][x-1] == Xblank)   goto android_w;
	goto android_move;

      case 7: /* W,SW,NW,N,E,SE,NE,S */
	if (Cave[y][x-1] == Xblank)   goto android_w;
	if (Cave[y+1][x-1] == Xblank) goto android_sw;
	if (Cave[y-1][x-1] == Xblank) goto android_nw;
	if (Cave[y-1][x] == Xblank)   goto android_n;
	if (Cave[y][x+1] == Xblank)   goto android_e;
	if (Cave[y+1][x+1] == Xblank) goto android_se;
	if (Cave[y-1][x+1] == Xblank) goto android_ne;
	if (Cave[y+1][x] == Xblank)   goto android_s;
	goto android_move;
    }
  }

 android_move:
  if (lev.android_move_cnt == 0)
  {
    if (Cave[y-1][x-1] == Zplayer ||
	Cave[y-1][x]   == Zplayer ||
	Cave[y-1][x+1] == Zplayer ||
	Cave[y][x-1]   == Zplayer ||
	Cave[y][x+1]   == Zplayer ||
	Cave[y+1][x-1] == Zplayer ||
	Cave[y+1][x]   == Zplayer ||
	Cave[y+1][x+1] == Zplayer)
      goto android_still;

    set_nearest_player_xy(x, y, &dx, &dy);

    Next[y][x] = Xblank;	/* assume we will move */
    temp = ((x < dx) + 1 - (x > dx)) + ((y < dy) + 1 - (y > dy)) * 3;

    if (RANDOM(2))
    {
      switch (temp)
      {
	/* attempt clockwise move first if direct path is blocked */

	case 0: /* north west */
	  if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
	  if (tab_android_move[Cave[y-1][x]])   goto android_n;
	  if (tab_android_move[Cave[y][x-1]])   goto android_w;
	  break;

	case 1: /* north */
	  if (tab_android_move[Cave[y-1][x]])   goto android_n;
	  if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
	  if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
	  break;

	case 2: /* north east */
	  if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
	  if (tab_android_move[Cave[y][x+1]])   goto android_e;
	  if (tab_android_move[Cave[y-1][x]])   goto android_n;
	  break;

	case 3: /* west */
	  if (tab_android_move[Cave[y][x-1]])   goto android_w;
	  if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
	  if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
	  break;

	case 4: /* nowhere */
	  break;

	case 5: /* east */
	  if (tab_android_move[Cave[y][x+1]])   goto android_e;
	  if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
	  if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
	  break;

	case 6: /* south west */
	  if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
	  if (tab_android_move[Cave[y][x-1]])   goto android_w;
	  if (tab_android_move[Cave[y+1][x]])   goto android_s;
	  break;

	case 7: /* south */
	  if (tab_android_move[Cave[y+1][x]])   goto android_s;
	  if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
	  if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
	  break;

	case 8: /* south east */
	  if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
	  if (tab_android_move[Cave[y+1][x]])   goto android_s;
	  if (tab_android_move[Cave[y][x+1]])   goto android_e;
	  break;
      }
    }
    else
    {
      switch (temp)
      {
	/* attempt counterclockwise move first if direct path is blocked */

	case 0: /* north west */
	  if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
	  if (tab_android_move[Cave[y][x-1]])   goto android_w;
	  if (tab_android_move[Cave[y-1][x]])   goto android_n;
	  break;

	case 1: /* north */
	  if (tab_android_move[Cave[y-1][x]])   goto android_n;
	  if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
	  if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
	  break;

	case 2: /* north east */
	  if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
	  if (tab_android_move[Cave[y-1][x]])   goto android_n;
	  if (tab_android_move[Cave[y][x+1]])   goto android_e;
	  break;

	case 3: /* west */
	  if (tab_android_move[Cave[y][x-1]])   goto android_w;
	  if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
	  if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
	  break;

	case 4: /* nowhere */
	  break;

	case 5: /* east */
	  if (tab_android_move[Cave[y][x+1]])   goto android_e;
	  if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
	  if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
	  break;

	case 6: /* south west */
	  if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
	  if (tab_android_move[Cave[y+1][x]])   goto android_s;
	  if (tab_android_move[Cave[y][x-1]])   goto android_w;
	  break;

	case 7: /* south */
	  if (tab_android_move[Cave[y+1][x]])   goto android_s;
	  if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
	  if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
	  break;

	case 8: /* south east */
	  if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
	  if (tab_android_move[Cave[y][x+1]])   goto android_e;
	  if (tab_android_move[Cave[y+1][x]])   goto android_s;
	  break;
      }
    }
  }

 android_still:
  Next[y][x] = Xandroid;
  return;

 android_n:
  Cave[y][x] = Yandroid_nB;
  Cave[y-1][x] = Yandroid_n;
  Next[y-1][x] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_ne:
  Cave[y][x] = Yandroid_neB;
  Cave[y-1][x+1] = Yandroid_ne;
  Next[y-1][x+1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_e:
  Cave[y][x] = Yandroid_eB;
  Cave[y][x+1] = Yandroid_e;
  Next[y][x+1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_se:
  Cave[y][x] = Yandroid_seB;
  Cave[y+1][x+1] = Yandroid_se;
  Next[y+1][x+1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_s:
  Cave[y][x] = Yandroid_sB;
  Cave[y+1][x] = Yandroid_s;
  Next[y+1][x] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_sw:
  Cave[y][x] = Yandroid_swB;
  Cave[y+1][x-1] = Yandroid_sw;
  Next[y+1][x-1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_w:
  Cave[y][x] = Yandroid_wB;
  Cave[y][x-1] = Yandroid_w;
  Next[y][x-1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_nw:
  Cave[y][x] = Yandroid_nwB;
  Cave[y-1][x-1] = Yandroid_nw;
  Next[y-1][x-1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;
}

static void Landroid_1_n(int x, int y)
{
  switch (Cave[y-1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_nB;
      if (Cave[y-2][x+1] == Xblank)
	Cave[y-2][x+1] = Xacid_splash_e;
      if (Cave[y-2][x-1] == Xblank)
	Cave[y-2][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_nB;
      Cave[y-1][x] = Yandroid_n;
      Next[y][x] = Xblank;
      Next[y-1][x] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_n);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_n(int x, int y)
{
  switch (Cave[y-1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_nB;
      if (Cave[y-2][x+1] == Xblank)
	Cave[y-2][x+1] = Xacid_splash_e;
      if (Cave[y-2][x-1] == Xblank)
	Cave[y-2][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_nB;
      Cave[y-1][x] = Yandroid_n;
      Next[y][x] = Xblank;
      Next[y-1][x] = Xandroid_1_n;
      play_element_sound(x, y, SOUND_android_move, Xandroid_2_n);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_1_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_eB;
      if (Cave[y-1][x+2] == Xblank)
	Cave[y-1][x+2] = Xacid_splash_e;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_eB;
      Cave[y][x+1] = Yandroid_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_e);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_eB;
      if (Cave[y-1][x+2] == Xblank)
	Cave[y-1][x+2] = Xacid_splash_e;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_eB;
      Cave[y][x+1] = Yandroid_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xandroid_1_e;
      play_element_sound(x, y, SOUND_android_move, Xandroid_2_e);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_1_s(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_sB;
      Cave[y+1][x] = Yandroid_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_s);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_s(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_sB;
      Cave[y+1][x] = Yandroid_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xandroid_1_s;
      play_element_sound(x, y, SOUND_android_move, Xandroid_2_s);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_1_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_wB;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_e;
      if (Cave[y-1][x-2] == Xblank)
	Cave[y-1][x-2] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_wB;
      Cave[y][x-1] = Yandroid_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_w);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yandroid_wB;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_e;
      if (Cave[y-1][x-2] == Xblank)
	Cave[y-1][x-2] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yandroid_wB;
      Cave[y][x-1] = Yandroid_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xandroid_1_w;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_w);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Leater_n(int x, int y)
{
  if (Cave[y][x+1] == Xdiamond)
  {
    Cave[y][x+1] = Ydiamond_blank;
    Next[y][x+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  if (Cave[y+1][x] == Xdiamond)
  {
    Cave[y+1][x] = Ydiamond_blank;
    Next[y+1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  if (Cave[y][x-1] == Xdiamond)
  {
    Cave[y][x-1] = Ydiamond_blank;
    Next[y][x-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  if (Cave[y-1][x] == Xdiamond)
  {
    Cave[y-1][x] = Ydiamond_blank;
    Next[y-1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  switch (Cave[y-1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yeater_nB;
      if (Cave[y-2][x+1] == Xblank)
	Cave[y-2][x+1] = Xacid_splash_e;
      if (Cave[y-2][x-1] == Xblank)
	Cave[y-2][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Yeater_nB;
      Cave[y-1][x] = Yeater_n;
      Next[y][x] = Xblank;
      Next[y-1][x] = Xeater_n;
      return;

    default:
      Next[y][x] = RANDOM(2) ? Xeater_e : Xeater_w;
      play_element_sound(x, y, SOUND_eater, Xeater_n);
      return;
  }
}

static void Leater_e(int x, int y)
{
  if (Cave[y+1][x] == Xdiamond)
  {
    Cave[y+1][x] = Ydiamond_blank;
    Next[y+1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  if (Cave[y][x-1] == Xdiamond)
  {
    Cave[y][x-1] = Ydiamond_blank;
    Next[y][x-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  if (Cave[y-1][x] == Xdiamond)
  {
    Cave[y-1][x] = Ydiamond_blank;
    Next[y-1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  if (Cave[y][x+1] == Xdiamond)
  {
    Cave[y][x+1] = Ydiamond_blank;
    Next[y][x+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  switch (Cave[y][x+1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yeater_eB;
      if (Cave[y-1][x+2] == Xblank)
	Cave[y-1][x+2] = Xacid_splash_e;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Yeater_eB;
      Cave[y][x+1] = Yeater_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xeater_e;
      return;

    default:
      Next[y][x] = RANDOM(2) ? Xeater_n : Xeater_s;
      play_element_sound(x, y, SOUND_eater, Xeater_e);
      return;
  }
}

static void Leater_s(int x, int y)
{
  if (Cave[y][x-1] == Xdiamond)
  {
    Cave[y][x-1] = Ydiamond_blank;
    Next[y][x-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  if (Cave[y-1][x] == Xdiamond)
  {
    Cave[y-1][x] = Ydiamond_blank;
    Next[y-1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  if (Cave[y][x+1] == Xdiamond)
  {
    Cave[y][x+1] = Ydiamond_blank;
    Next[y][x+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  if (Cave[y+1][x] == Xdiamond)
  {
    Cave[y+1][x] = Ydiamond_blank;
    Next[y+1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yeater_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Yeater_sB;
      Cave[y+1][x] = Yeater_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xeater_s;
      return;

    default:
      Next[y][x] = RANDOM(2) ? Xeater_e : Xeater_w;
      play_element_sound(x, y, SOUND_eater, Xeater_s);
      return;
  }
}

static void Leater_w(int x, int y)
{
  if (Cave[y-1][x] == Xdiamond)
  {
    Cave[y-1][x] = Ydiamond_blank;
    Next[y-1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  if (Cave[y][x+1] == Xdiamond)
  {
    Cave[y][x+1] = Ydiamond_blank;
    Next[y][x+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  if (Cave[y+1][x] == Xdiamond)
  {
    Cave[y+1][x] = Ydiamond_blank;
    Next[y+1][x] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  if (Cave[y][x-1] == Xdiamond)
  {
    Cave[y][x-1] = Ydiamond_blank;
    Next[y][x-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  switch (Cave[y][x-1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yeater_wB;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_e;
      if (Cave[y-1][x-2] == Xblank)
	Cave[y-1][x-2] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Yeater_wB;
      Cave[y][x-1] = Yeater_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xeater_w;
      return;

    default:
      Next[y][x] = RANDOM(2) ? Xeater_n : Xeater_s;
      play_element_sound(x, y, SOUND_eater, Xeater_w);
      return;
  }
}

static void Lalien(int x, int y)
{
  int dx, dy;

  if (lev.wheel_cnt)
  {
    dx = lev.wheel_x;
    dy = lev.wheel_y;
  }
  else
  {
    set_nearest_player_xy(x, y, &dx, &dy);
  }

  if (RANDOM(2))
  {
    if (y > dy)
    {
      switch (Cave[y-1][x])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yalien_nB;
	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Xacid_splash_e;
	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	case Xplant:
	case Yplant:
	case Zplayer:
	  Cave[y][x] = Yalien_nB;
	  Cave[y-1][x] = Yalien_n;
	  Next[y][x] = Xblank;
	  Next[y-1][x] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;
      }
    }
    else if (y < dy)
    {
      switch (Cave[y+1][x])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yalien_sB;
	  Next[y][x] = Xblank;
	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Xacid_splash_e;
	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Xacid_splash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	case Xplant:
	case Yplant:
	case Zplayer:
	  Cave[y][x] = Yalien_sB;
	  Cave[y+1][x] = Yalien_s;
	  Next[y][x] = Xblank;
	  Next[y+1][x] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;
      }
    }
  }
  else
  {
    if (x < dx)
    {
      switch (Cave[y][x+1])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yalien_eB;
	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Xacid_splash_e;
	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	case Xplant:
	case Yplant:
	case Zplayer:
	  Cave[y][x] = Yalien_eB;
	  Cave[y][x+1] = Yalien_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;
      }
    }
    else if (x > dx)
    {
      switch (Cave[y][x-1])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yalien_wB;
	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Xacid_splash_e;
	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	case Xplant:
	case Yplant:
	case Zplayer:
	  Cave[y][x] = Yalien_wB;
	  Cave[y][x-1] = Yalien_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;
      }
    }
  }
}

static void Lalien_pause(int x, int y)
{
  Next[y][x] = Xalien;
}

static void Lbug_n(int x, int y)
{
  switch (Cave[y-1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybug_nB;
      if (Cave[y-2][x+1] == Xblank)
	Cave[y-2][x+1] = Xacid_splash_e;
      if (Cave[y-2][x-1] == Xblank)
	Cave[y-2][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ybug_nB;
      Cave[y-1][x] = Ybug_n;
      Next[y][x] = Xblank;
      Next[y-1][x] = Xbug_1_n;
      play_element_sound(x, y, SOUND_bug, Xbug_1_n);
      return;

    default:
      Cave[y][x] = Ybug_n_w;
      Next[y][x] = Xbug_2_w;
      play_element_sound(x, y, SOUND_bug, Xbug_1_n);
      return;
  }
}

static void Lbug_1_n(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  switch (Cave[y][x+1])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ybug_n_e;
      Next[y][x] = Xbug_2_e;
      play_element_sound(x, y, SOUND_bug, Xbug_1_n);
      return;

    default:
      Lbug_n(x, y);
      return;
  }
}

static void Lbug_2_n(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  Lbug_n(x, y);
}

static void Lbug_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybug_eB;
      if (Cave[y-1][x+2] == Xblank)
	Cave[y-1][x+2] = Xacid_splash_e;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ybug_eB;
      Cave[y][x+1] = Ybug_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xbug_1_e;
      play_element_sound(x, y, SOUND_bug, Xbug_1_e);
      return;

    default:
      Cave[y][x] = Ybug_e_n;
      Next[y][x] = Xbug_2_n;
      play_element_sound(x, y, SOUND_bug, Xbug_1_e);
      return;
  }
}

static void Lbug_1_e(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  switch (Cave[y+1][x])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ybug_e_s;
      Next[y][x] = Xbug_2_s;
      play_element_sound(x, y, SOUND_bug, Xbug_1_e);
      return;

    default:
      Lbug_e(x, y);
      return;
  }
}

static void Lbug_2_e(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  Lbug_e(x, y);
}

static void Lbug_s(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybug_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ybug_sB;
      Cave[y+1][x] = Ybug_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xbug_1_s;
      play_element_sound(x, y, SOUND_bug, Xbug_1_s);
      return;

    default:
      Cave[y][x] = Ybug_s_e;
      Next[y][x] = Xbug_2_e;
      play_element_sound(x, y, SOUND_bug, Xbug_1_s);
      return;
  }
}

static void Lbug_1_s(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  switch (Cave[y][x-1])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ybug_s_w;
      Next[y][x] = Xbug_2_w;
      play_element_sound(x, y, SOUND_bug, Xbug_1_s);
      return;

    default:
      Lbug_s(x, y);
      return;
  }
}

static void Lbug_2_s(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  Lbug_s(x, y);
}

static void Lbug_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybug_wB;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_e;
      if (Cave[y-1][x-2] == Xblank)
	Cave[y-1][x-2] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ybug_wB;
      Cave[y][x-1] = Ybug_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xbug_1_w;
      play_element_sound(x, y, SOUND_bug, Xbug_1_w);
      return;

    default:
      Cave[y][x] = Ybug_w_s;
      Next[y][x] = Xbug_2_s;
      play_element_sound(x, y, SOUND_bug, Xbug_1_w);
      return;
  }
}

static void Lbug_1_w(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  switch (Cave[y-1][x])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ybug_w_n;
      Next[y][x] = Xbug_2_n;
      play_element_sound(x, y, SOUND_bug, Xbug_1_w);
      return;

    default:
      Lbug_w(x, y);
      return;
  }
}

static void Lbug_2_w(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_bug(x, y);

    return;
  }

  Lbug_w(x, y);
}

static void Ltank_n(int x, int y)
{
  switch (Cave[y-1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ytank_nB;
      if (Cave[y-2][x+1] == Xblank)
	Cave[y-2][x+1] = Xacid_splash_e;
      if (Cave[y-2][x-1] == Xblank)
	Cave[y-2][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ytank_nB;
      Cave[y-1][x] = Ytank_n;
      Next[y][x] = Xblank;
      Next[y-1][x] = Xtank_1_n;
      play_element_sound(x, y, SOUND_tank, Xtank_1_n);
      return;

    default:
      Cave[y][x] = Ytank_n_e;
      Next[y][x] = Xtank_2_e;
      play_element_sound(x, y, SOUND_tank, Xtank_1_n);
      return;
  }
}

static void Ltank_1_n(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  switch (Cave[y][x-1])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ytank_n_w;
      Next[y][x] = Xtank_2_w;
      play_element_sound(x, y, SOUND_tank, Xtank_1_n);
      return;

    default:
      Ltank_n(x, y);
      return;
  }
}

static void Ltank_2_n(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  Ltank_n(x, y);
}

static void Ltank_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ytank_eB;
      if (Cave[y-1][x+2] == Xblank)
	Cave[y-1][x+2] = Xacid_splash_e;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ytank_eB;
      Cave[y][x+1] = Ytank_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xtank_1_e;
      play_element_sound(x, y, SOUND_tank, Xtank_1_e);
      return;

    default:
      Cave[y][x] = Ytank_e_s;
      Next[y][x] = Xtank_2_s;
      play_element_sound(x, y, SOUND_tank, Xtank_1_e);
      return;
  }
}

static void Ltank_1_e(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  switch (Cave[y-1][x])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ytank_e_n;
      Next[y][x] = Xtank_2_n;
      play_element_sound(x, y, SOUND_tank, Xtank_1_e);
      return;

    default:
      Ltank_e(x, y);
      return;
  }
}

static void Ltank_2_e(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  Ltank_e(x, y);
}

static void Ltank_s(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ytank_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ytank_sB;
      Cave[y+1][x] = Ytank_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xtank_1_s;
      play_element_sound(x, y, SOUND_tank, Xtank_1_s);
      return;

    default:
      Cave[y][x] = Ytank_s_w;
      Next[y][x] = Xtank_2_w;
      play_element_sound(x, y, SOUND_tank, Xtank_1_s);
      return;
  }
}

static void Ltank_1_s(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  switch (Cave[y][x+1])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ytank_s_e;
      Next[y][x] = Xtank_2_e;
      play_element_sound(x, y, SOUND_tank, Xtank_1_s);
      return;

    default:
      Ltank_s(x, y);
      return;
  }
}

static void Ltank_2_s(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  Ltank_s(x, y);
}

static void Ltank_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ytank_wB;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_e;
      if (Cave[y-1][x-2] == Xblank)
	Cave[y-1][x-2] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ytank_wB;
      Cave[y][x-1] = Ytank_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xtank_1_w;
      play_element_sound(x, y, SOUND_tank, Xtank_1_w);
      return;

    default:
      Cave[y][x] = Ytank_w_n;
      Next[y][x] = Xtank_2_n;
      play_element_sound(x, y, SOUND_tank, Xtank_1_w);
      return;
  }
}

static void Ltank_1_w(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  switch (Cave[y+1][x])
  {
    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Zplayer:
      Cave[y][x] = Ytank_w_s;
      Next[y][x] = Xtank_2_s;
      play_element_sound(x, y, SOUND_tank, Xtank_1_w);
      return;

    default:
      Ltank_w(x, y);
      return;
  }
}

static void Ltank_2_w(int x, int y)
{
  if (tab_amoeba[Cave[y-1][x]] ||
      tab_amoeba[Cave[y][x+1]] ||
      tab_amoeba[Cave[y+1][x]] ||
      tab_amoeba[Cave[y][x-1]])
  {
    Lboom_tank(x, y);

    return;
  }

  Ltank_w(x, y);
}

static void Lemerald(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yemerald_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yemerald_sB;
      Cave[y+1][x] = Yemerald_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xemerald_fall;
      return;

    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xstone:
    case Xstone_pause:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xballoon:
    case Xacid_ne:
    case Xacid_nw:
    case Xball_1:
    case Xball_2:
    case Xnut:
    case Xnut_pause:
    case Xslidewall_ns:
    case Xslidewall_ew:
    case Xwonderwall:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xbumper:
    case Xswitch:
    case Xsteel_1:
    case Xsteel_2:
    case Xsteel_3:
    case Xsteel_4:
    case Xwall_1:
    case Xwall_2:
    case Xwall_3:
    case Xwall_4:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Yemerald_eB;
	  Cave[y][x+1] = Yemerald_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xemerald_pause;
	  return;
	}

	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Yemerald_wB;
	  Cave[y][x-1] = Yemerald_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xemerald_pause;
	  return;
	}
      }
      else
      {
	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Yemerald_wB;
	  Cave[y][x-1] = Yemerald_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xemerald_pause;
	  return;
	}

	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Yemerald_eB;
	  Cave[y][x+1] = Yemerald_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xemerald_pause;
	  return;
	}
      }

    default:
      if (++lev.shine_cnt > 50)
      {
	lev.shine_cnt = RANDOM(8);
	Cave[y][x] = Xemerald_shine;
      }

      return;
  }
}

static void Lemerald_pause(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yemerald_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yemerald_sB;
      Cave[y+1][x] = Yemerald_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xemerald_fall;
      return;

    default:
      Cave[y][x] = Xemerald;
      Next[y][x] = Xemerald;
      return;
  }
}

static void Lemerald_fall(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yemerald_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Zplayer:
      Cave[y][x] = Yemerald_sB;
      Cave[y+1][x] = Yemerald_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xemerald_fall;
      return;

    case Xwonderwall:
      if (lev.wonderwall_time)
      {
	lev.wonderwall_state = 1;
	Cave[y][x] = Yemerald_sB;
	if (tab_blank[Cave[y+2][x]])
	{
	  Cave[y+2][x] = Ydiamond_s;
	  Next[y+2][x] = Xdiamond_fall;
	}

	Next[y][x] = Xblank;
	play_element_sound(x, y, SOUND_wonderfall, Xwonderwall);
	return;
      }

    default:
      Cave[y][x] = Xemerald;
      Next[y][x] = Xemerald;
      play_element_sound(x, y, SOUND_diamond, Xemerald);
      return;
  }
}

static void Ldiamond(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ydiamond_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ydiamond_sB;
      Cave[y+1][x] = Ydiamond_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xdiamond_fall;
      return;

    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xstone:
    case Xstone_pause:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xballoon:
    case Xacid_ne:
    case Xacid_nw:
    case Xball_1:
    case Xball_2:
    case Xnut:
    case Xnut_pause:
    case Xslidewall_ns:
    case Xslidewall_ew:
    case Xwonderwall:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xbumper:
    case Xswitch:
    case Xsteel_1:
    case Xsteel_2:
    case Xsteel_3:
    case Xsteel_4:
    case Xwall_1:
    case Xwall_2:
    case Xwall_3:
    case Xwall_4:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ydiamond_eB;
	  Cave[y][x+1] = Ydiamond_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xdiamond_pause;
	  return;
	}

	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ydiamond_wB;
	  Cave[y][x-1] = Ydiamond_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xdiamond_pause;
	  return;
	}
      }
      else
      {
	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ydiamond_wB;
	  Cave[y][x-1] = Ydiamond_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xdiamond_pause;
	  return;
	}

	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ydiamond_eB;
	  Cave[y][x+1] = Ydiamond_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xdiamond_pause;
	  return;
	}
      }

    default:
      if (++lev.shine_cnt > 50)
      {
	lev.shine_cnt = RANDOM(8);
	Cave[y][x] = Xdiamond_shine;
      }

      return;
  }
}

static void Ldiamond_pause(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ydiamond_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ydiamond_sB;
      Cave[y+1][x] = Ydiamond_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xdiamond_fall;
      return;

    default:
      Cave[y][x] = Xdiamond;
      Next[y][x] = Xdiamond;
      return;
  }
}

static void Ldiamond_fall(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ydiamond_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Zplayer:
      Cave[y][x] = Ydiamond_sB;
      Cave[y+1][x] = Ydiamond_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xdiamond_fall;
      return;

    case Xwonderwall:
      if (lev.wonderwall_time)
      {
	lev.wonderwall_state = 1;
	Cave[y][x] = Ydiamond_sB;
	if (tab_blank[Cave[y+2][x]])
	{
	  Cave[y+2][x] = Ystone_s;
	  Next[y+2][x] = Xstone_fall;
	}

	Next[y][x] = Xblank;
	play_element_sound(x, y, SOUND_wonderfall, Xwonderwall);
	return;
      }

    default:
      Cave[y][x] = Xdiamond;
      Next[y][x] = Xdiamond;
      play_element_sound(x, y, SOUND_diamond, Xdiamond);
      return;
  }
}

static void Lstone(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ystone_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
#endif
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Ystone_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xstone_fall;
      return;

    case Xsand:
      Cave[y][x] = Xsand_stonein_1;
      Cave[y+1][x] = Xsand_sandstone_1;
      Next[y][x] = Xsand_stonein_2;
      Next[y+1][x] = Xsand_sandstone_2;
      return;

    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xstone:
    case Xstone_pause:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xballoon:
    case Xacid_ne:
    case Xacid_nw:
    case Xball_1:
    case Xball_2:
    case Xnut:
    case Xnut_pause:
    case Xslidewall_ns:
    case Xslidewall_ew:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xbumper:
    case Xswitch:
    case Xlenses:
    case Xmagnify:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ystone_eB;
	  Cave[y][x+1] = Ystone_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xstone_pause;
	  return;
	}

	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ystone_wB;
	  Cave[y][x-1] = Ystone_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xstone_pause;
	  return;
	}
      }
      else
      {
	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ystone_wB;
	  Cave[y][x-1] = Ystone_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xstone_pause;
	  return;
	}

	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ystone_eB;
	  Cave[y][x+1] = Ystone_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xstone_pause;
	  return;
	}
      }
  }
}

static void Lstone_pause(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ystone_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
#endif
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Ystone_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xstone_fall;
      return;

    default:
      Cave[y][x] = Xstone;
      Next[y][x] = Xstone;
      return;
  }
}

static void Lstone_fall(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ystone_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Zplayer:
#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
#endif
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Ystone_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xstone_fall;
      return;

    case Xnut:
    case Xnut_pause:
      Cave[y+1][x] = Ynut_stone;
      Next[y][x] = Xstone;
      Next[y+1][x] = Xemerald;
      play_element_sound(x, y, SOUND_crack, Xnut);
      score += lev.nut_score;
      return;

    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Ybug_stone;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xemerald;
      Boom[y][x] = Xemerald;
      Boom[y][x+1] = Xemerald;
      Boom[y+1][x-1] = Xemerald;
      Boom[y+1][x] = Xdiamond;
      Boom[y+1][x+1] = Xemerald;
      Boom[y+2][x-1] = Xemerald;
      Boom[y+2][x] = Xemerald;
      Boom[y+2][x+1] = Xemerald;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xstone_fall);
#endif
      score += lev.bug_score;
      return;

    case Xtank_1_n:
    case Xtank_1_e:
    case Xtank_1_s:
    case Xtank_1_w:
    case Xtank_2_n:
    case Xtank_2_e:
    case Xtank_2_s:
    case Xtank_2_w:
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Ytank_stone;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      Boom[y+2][x-1] = Xblank;
      Boom[y+2][x] = Xblank;
      Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xstone_fall);
#endif
      score += lev.tank_score;
      return;

    case Xspring:
      if (RANDOM(2))
      {
	switch (Cave[y+1][x+1])
	{
	  case Xblank:
	  case Xacid_splash_e:
	  case Xacid_splash_w:
	  case Xalien:
	  case Xalien_pause:
	    Cave[y+1][x] = Xspring_e;
	    break;

	  default:
	    Cave[y+1][x] = Xspring_w;
	    break;
	}
      }
      else
      {
	switch (Cave[y+1][x-1])
	{
	  case Xblank:
	  case Xacid_splash_e:
	  case Xacid_splash_w:
	  case Xalien:
	  case Xalien_pause:
	    Cave[y+1][x] = Xspring_w;
	    break;
	  default:
	    Cave[y+1][x] = Xspring_e;
	    break;
	}
      }

      Next[y][x] = Xstone;
      return;

    case Xeater_n:
    case Xeater_e:
    case Xeater_s:
    case Xeater_w:
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Yeater_stone;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = lev.eater_array[lev.eater_pos][0];
      Boom[y][x] = lev.eater_array[lev.eater_pos][1];
      Boom[y][x+1] = lev.eater_array[lev.eater_pos][2];
      Boom[y+1][x-1] = lev.eater_array[lev.eater_pos][3];
      Boom[y+1][x] = lev.eater_array[lev.eater_pos][4];
      Boom[y+1][x+1] = lev.eater_array[lev.eater_pos][5];
      Boom[y+2][x-1] = lev.eater_array[lev.eater_pos][6];
      Boom[y+2][x] = lev.eater_array[lev.eater_pos][7];
      Boom[y+2][x+1] = lev.eater_array[lev.eater_pos][8];
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xstone_fall);
#endif
      lev.eater_pos = (lev.eater_pos + 1) & 7;
      score += lev.eater_score;
      return;

    case Xalien:
    case Xalien_pause:
      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Yalien_stone;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      Boom[y+2][x-1] = Xblank;
      Boom[y+2][x] = Xblank;
      Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xstone_fall);
#endif
      score += lev.alien_score;
      return;

    case Xdiamond:
    case Xdiamond_pause:
      switch (Cave[y+2][x])
      {
	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	case Zplayer:
	case Xbug_1_n:
	case Xbug_1_e:
	case Xbug_1_s:
	case Xbug_1_w:
	case Xbug_2_n:
	case Xbug_2_e:
	case Xbug_2_s:
	case Xbug_2_w:
	case Xtank_1_n:
	case Xtank_1_e:
	case Xtank_1_s:
	case Xtank_1_w:
	case Xtank_2_n:
	case Xtank_2_e:
	case Xtank_2_s:
	case Xtank_2_w:
	case Xspring_fall:
	case Xandroid:
	case Xandroid_1_n:
	case Xandroid_2_n:
	case Xandroid_1_e:
	case Xandroid_2_e:
	case Xandroid_1_s:
	case Xandroid_2_s:
	case Xandroid_1_w:
	case Xandroid_2_w:
	case Xstone_fall:
	case Xemerald_fall:
	case Xdiamond_fall:
	case Xbomb_fall:
	case Xacid_s:
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	case Xnut_fall:
	case Xplant:
	case Yplant:
	  Next[y][x] = Xstone;
	  play_element_sound(x, y, SOUND_stone, Xstone);
	  return;
      }

      Cave[y][x] = Ystone_sB;
      Cave[y+1][x] = Ydiamond_stone;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xstone_pause;
      play_element_sound(x, y, SOUND_squash, Xdiamond);
      return;

    case Xbomb:
    case Xbomb_pause:
      Cave[y+1][x] = Ybomb_blank;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      Boom[y+2][x-1] = Xblank;
      Boom[y+2][x] = Xblank;
      Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xstone_fall);
#endif
      return;

    case Xwonderwall:
      if (lev.wonderwall_time)
      {
	lev.wonderwall_state = 1;
	Cave[y][x] = Ystone_sB;

	if (tab_blank[Cave[y+2][x]])
	{
	  Cave[y+2][x] = Yemerald_s;
	  Next[y+2][x] = Xemerald_fall;
	}

	Next[y][x] = Xblank;
	play_element_sound(x, y, SOUND_wonderfall, Xwonderwall);
	return;
      }

    default:
      Cave[y][x] = Xstone;
      Next[y][x] = Xstone;
      play_element_sound(x, y, SOUND_stone, Xstone);
      return;
  }
}

static void Lbomb(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybomb_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ybomb_sB;
      Cave[y+1][x] = Ybomb_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xbomb_fall;
      return;

    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xstone:
    case Xstone_pause:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xballoon:
    case Xacid_ne:
    case Xacid_nw:
    case Xball_1:
    case Xball_2:
    case Xnut:
    case Xnut_pause:
    case Xslidewall_ns:
    case Xslidewall_ew:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xbumper:
    case Xswitch:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ybomb_eB;
	  Cave[y][x+1] = Ybomb_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xbomb_pause;
	  return;
	}

	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ybomb_wB;
	  Cave[y][x-1] = Ybomb_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xbomb_pause;
	  return;
	}
      }
      else
      {
	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ybomb_wB;
	  Cave[y][x-1] = Ybomb_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xbomb_pause;
	  return;
	}

	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ybomb_eB;
	  Cave[y][x+1] = Ybomb_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xbomb_pause;
	  return;
	}
      }
  }
}

static void Lbomb_pause(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybomb_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ybomb_sB;
      Cave[y+1][x] = Ybomb_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xbomb_fall;
      return;

    default:
      Cave[y][x] = Xbomb;
      Next[y][x] = Xbomb;
      return;
  }
}

static void Lbomb_fall(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ybomb_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ybomb_sB;
      Cave[y+1][x] = Ybomb_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xbomb_fall;
      return;

    default:
      Cave[y][x] = Ybomb_blank;
      Next[y][x] = Znormal;
      Boom[y-1][x-1] = Xblank;
      Boom[y-1][x] = Xblank;
      Boom[y-1][x+1] = Xblank;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xbomb_fall);
#endif
      return;
  }
}

static void Lnut(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ynut_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ynut_sB;
      Cave[y+1][x] = Ynut_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xnut_fall;
      return;

    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xstone:
    case Xstone_pause:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xballoon:
    case Xacid_ne:
    case Xacid_nw:
    case Xball_1:
    case Xball_2:
    case Xnut:
    case Xnut_pause:
    case Xslidewall_ns:
    case Xslidewall_ew:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xbumper:
    case Xswitch:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ynut_eB;
	  Cave[y][x+1] = Ynut_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xnut_pause;
	  return;
	}

	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ynut_wB;
	  Cave[y][x-1] = Ynut_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xnut_pause;
	  return;
	}
      }
      else
      {
	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Ynut_wB;
	  Cave[y][x-1] = Ynut_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xnut_pause;
	  return;
	}

	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Ynut_eB;
	  Cave[y][x+1] = Ynut_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xnut_pause;
	  return;
	}
      }
  }
}

static void Lnut_pause(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ynut_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Ynut_sB;
      Cave[y+1][x] = Ynut_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xnut_fall;
      return;

    default:
      Cave[y][x] = Xnut;
      Next[y][x] = Xnut;
      return;
  }
}

static void Lnut_fall(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ynut_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Zplayer:
      Cave[y][x] = Ynut_sB;
      Cave[y+1][x] = Ynut_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xnut_fall;
      return;

    default:
      Cave[y][x] = Xnut;
      Next[y][x] = Xnut;
      play_element_sound(x, y, SOUND_nut, Xnut);
      return;
  }
}

static void Lspring(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yspring_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xspring_fall;
      return;

    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xstone:
    case Xstone_pause:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xballoon:
    case Xacid_ne:
    case Xacid_nw:
    case Xball_1:
    case Xball_2:
    case Xnut:
    case Xnut_pause:
    case Xslidewall_ns:
    case Xslidewall_ew:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xbumper:
    case Xswitch:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Yspring_eB;
	  Cave[y][x+1] = Yspring_e;
	  if (Cave[y+1][x] == Xbumper)
	    Cave[y+1][x] = XbumperB;
	  Next[y][x] = Xblank;

#ifdef SPRING_ROLL
	  Next[y][x+1] = Xspring_e;
#else	
	  Next[y][x+1] = Xspring_pause;
#endif
	  return;
	}

	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Yspring_wB;
	  Cave[y][x-1] = Yspring_w;
	  if (Cave[y+1][x] == Xbumper)
	    Cave[y+1][x] = XbumperB;
	  Next[y][x] = Xblank;

#ifdef SPRING_ROLL
	  Next[y][x-1] = Xspring_w;
#else
	  Next[y][x-1] = Xspring_pause;
#endif
	  return;
	}
      }
      else
      {
	if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	{
	  Cave[y][x] = Yspring_wB;
	  Cave[y][x-1] = Yspring_w;
	  if (Cave[y+1][x] == Xbumper)
	    Cave[y+1][x] = XbumperB;
	  Next[y][x] = Xblank;

#ifdef SPRING_ROLL
	  Next[y][x-1] = Xspring_w;
#else
	  Next[y][x-1] = Xspring_pause;
#endif
	  return;
	}

	if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	{
	  Cave[y][x] = Yspring_eB;
	  Cave[y][x+1] = Yspring_e;
	  if (Cave[y+1][x] == Xbumper)
	    Cave[y+1][x] = XbumperB;
	  Next[y][x] = Xblank;

#ifdef SPRING_ROLL
	  Next[y][x+1] = Xspring_e;
#else
	  Next[y][x+1] = Xspring_pause;
#endif
	  return;
	}
      }
  }
}

static void Lspring_pause(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yspring_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xspring_fall;
      return;

    default:
      Cave[y][x] = Xspring;
      Next[y][x] = Xspring;
      return;
  }
}

static void Lspring_e(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yspring_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xspring_fall;
      return;

    case Xbumper:
      Cave[y+1][x] = XbumperB;
  }

  switch (Cave[y][x+1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_eB;
      if (Cave[y-1][x+2] == Xblank)
	Cave[y-1][x+2] = Xacid_splash_e;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Yalien_nB:
    case Yalien_eB:
    case Yalien_sB:
    case Yalien_wB:
      Cave[y][x] = Yspring_eB;
      Cave[y][x+1] = Yspring_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xspring_e;
      return;

    case Xalien:
    case Xalien_pause:
    case Yalien_n:
    case Yalien_e:
    case Yalien_s:
    case Yalien_w:
      Cave[y][x] = Yspring_alien_eB;
      Cave[y][x+1] = Yspring_alien_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xspring_e;
      play_element_sound(x, y, SOUND_slurp, Xalien);
      score += lev.slurp_score;
      return;

    case Xbumper:
    case XbumperB:
      Cave[y][x+1] = XbumperB;
      Next[y][x] = Xspring_w;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;

    default:
      Cave[y][x] = Xspring;
      Next[y][x] = Xspring;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;
  }
}

static void Lspring_w(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yspring_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xspring_fall;
      return;

    case Xbumper:
      Cave[y+1][x] = XbumperB;
  }

  switch (Cave[y][x-1])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_wB;
      if (Cave[y-1][x] == Xblank)
	Cave[y-1][x] = Xacid_splash_e;
      if (Cave[y-1][x-2] == Xblank)
	Cave[y-1][x-2] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Yalien_nB:
    case Yalien_eB:
    case Yalien_sB:
    case Yalien_wB:
      Cave[y][x] = Yspring_wB;
      Cave[y][x-1] = Yspring_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xspring_w;
      return;

    case Xalien:
    case Xalien_pause:
    case Yalien_n:
    case Yalien_e:
    case Yalien_s:
    case Yalien_w:
      Cave[y][x] = Yspring_alien_wB;
      Cave[y][x-1] = Yspring_alien_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xspring_w;
      play_element_sound(x, y, SOUND_slurp, Xalien);
      score += lev.slurp_score;
      return;

    case Xbumper:
    case XbumperB:
      Cave[y][x-1] = XbumperB;
      Next[y][x] = Xspring_e;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;

    default:
      Cave[y][x] = Xspring;
      Next[y][x] = Xspring;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;
  }
}

static void Lspring_fall(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Yspring_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xblank;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Zplayer:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yspring_s;
      Next[y][x] = Xblank;
      Next[y+1][x] = Xspring_fall;
      return;

    case Xbomb:
    case Xbomb_pause:
      Cave[y+1][x] = Ybomb_blank;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      Boom[y+2][x-1] = Xblank;
      Boom[y+2][x] = Xblank;
      Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xspring_fall);
#endif
      return;

    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Ybug_spring;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xemerald;
      Boom[y][x] = Xemerald;
      Boom[y][x+1] = Xemerald;
      Boom[y+1][x-1] = Xemerald;
      Boom[y+1][x] = Xdiamond;
      Boom[y+1][x+1] = Xemerald;
      Boom[y+2][x-1] = Xemerald;
      Boom[y+2][x] = Xemerald;
      Boom[y+2][x+1] = Xemerald;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xspring_fall);
#endif
      score += lev.bug_score;
      return;

    case Xtank_1_n:
    case Xtank_1_e:
    case Xtank_1_s:
    case Xtank_1_w:
    case Xtank_2_n:
    case Xtank_2_e:
    case Xtank_2_s:
    case Xtank_2_w:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Ytank_spring;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      Boom[y+2][x-1] = Xblank;
      Boom[y+2][x] = Xblank;
      Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xspring_fall);
#endif
      score += lev.tank_score;
      return;

    case Xeater_n:
    case Xeater_e:
    case Xeater_s:
    case Xeater_w:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yeater_spring;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = lev.eater_array[lev.eater_pos][0];
      Boom[y][x] = lev.eater_array[lev.eater_pos][1];
      Boom[y][x+1] = lev.eater_array[lev.eater_pos][2];
      Boom[y+1][x-1] = lev.eater_array[lev.eater_pos][3];
      Boom[y+1][x] = lev.eater_array[lev.eater_pos][4];
      Boom[y+1][x+1] = lev.eater_array[lev.eater_pos][5];
      Boom[y+2][x-1] = lev.eater_array[lev.eater_pos][6];
      Boom[y+2][x] = lev.eater_array[lev.eater_pos][7];
      Boom[y+2][x+1] = lev.eater_array[lev.eater_pos][8];
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xspring_fall);
#endif
      lev.eater_pos = (lev.eater_pos + 1) & 7;
      score += lev.eater_score;
      return;

    case Xalien:
    case Xalien_pause:
      Cave[y][x] = Yspring_sB;
      Cave[y+1][x] = Yalien_spring;
      Next[y+1][x] = Znormal;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      Boom[y+2][x-1] = Xblank;
      Boom[y+2][x] = Xblank;
      Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SOUND_boom, Xspring_fall);
#endif
      score += lev.alien_score;
      return;

    default:
      Cave[y][x] = Xspring;
      Next[y][x] = Xspring;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;
  }
}

static void Lpush_emerald_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xemerald;
      Next[y][x] = Xemerald;
      return;

    default:
      Cave[y][x] = Yemerald_eB;
      Cave[y][x+1] = Yemerald_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xemerald_pause;
      return;
  }
}

static void Lpush_emerald_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xemerald;
      Next[y][x] = Xemerald;
      return;

    default:
      Cave[y][x] = Yemerald_wB;
      Cave[y][x-1] = Yemerald_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xemerald_pause;
      return;
  }
}

static void Lpush_diamond_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xdiamond;
      Next[y][x] = Xdiamond;
      return;

    default:
      Cave[y][x] = Ydiamond_eB;
      Cave[y][x+1] = Ydiamond_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xdiamond_pause;
      return;
  }
}

static void Lpush_diamond_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xdiamond;
      Next[y][x] = Xdiamond;
      return;

    default:
      Cave[y][x] = Ydiamond_wB;
      Cave[y][x-1] = Ydiamond_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xdiamond_pause;
      return;
  }
}

static void Lpush_stone_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xstone;
      Next[y][x] = Xstone;
      return;

    default:
      Cave[y][x] = Ystone_eB;
      Cave[y][x+1] = Ystone_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xstone_pause;
      return;
  }
}

static void Lpush_stone_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xstone;
      Next[y][x] = Xstone;
      return;

    default:
      Cave[y][x] = Ystone_wB;
      Cave[y][x-1] = Ystone_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xstone_pause;
      return;
  }
}

static void Lpush_bomb_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xbomb;
      Next[y][x] = Xbomb;
      return;

    default:
      Cave[y][x] = Ybomb_eB;
      Cave[y][x+1] = Ybomb_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xbomb_pause;
      return;
  }
}

static void Lpush_bomb_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xbomb;
      Next[y][x] = Xbomb;
      return;

    default:
      Cave[y][x] = Ybomb_wB;
      Cave[y][x-1] = Ybomb_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xbomb_pause;
      return;
  }
}

static void Lpush_nut_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xnut;
      Next[y][x] = Xnut;
      return;

    default:
      Cave[y][x] = Ynut_eB;
      Cave[y][x+1] = Ynut_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xnut_pause;
      return;
  }
}

static void Lpush_nut_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xnut;
      Next[y][x] = Xnut;
      return;

    default:
      Cave[y][x] = Ynut_wB;
      Cave[y][x-1] = Ynut_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xnut_pause;
      return;
  }
}

static void Lpush_spring_e(int x, int y)
{
  switch (Cave[y][x+1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xspring;
      Next[y][x] = Xspring;
      return;

    default:
      Cave[y][x] = Yspring_eB;
      Cave[y][x+1] = Yspring_e;
      Next[y][x] = Xblank;
      Next[y][x+1] = Xspring_e;
      return;
  }
}

static void Lpush_spring_w(int x, int y)
{
  switch (Cave[y][x-1])
  {
    case Zborder:
    case Znormal:
    case Zdynamite:
    case Xboom_bug:
    case Xboom_bomb:
    case Xboom_android:
    case Xboom_1:
    case Zplayer:
      Cave[y][x] = Xspring;
      Next[y][x] = Xspring;
      return;

    default:
      Cave[y][x] = Yspring_wB;
      Cave[y][x-1] = Yspring_w;
      Next[y][x] = Xblank;
      Next[y][x-1] = Xspring_w;
      return;
  }
}

static void Lballoon(int x, int y)
{
  if (lev.wind_cnt == 0)
    return;

  switch (lev.wind_direction)
  {
    case 0: /* north */
      switch (Cave[y-1][x])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yballoon_nB;
	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Xacid_splash_e;
	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	  Cave[y][x] = Yballoon_nB;
	  Cave[y-1][x] = Yballoon_n;
	  Next[y][x] = Xblank;
	  Next[y-1][x] = Xballoon;
	  return;
      }
      break;

    case 1: /* east */
      switch (Cave[y][x+1])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yballoon_eB;
	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Xacid_splash_e;
	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	  Cave[y][x] = Yballoon_eB;
	  Cave[y][x+1] = Yballoon_e;
	  Next[y][x] = Xblank;
	  Next[y][x+1] = Xballoon;
	  return;
      }
      break;

    case 2: /* south */
      switch (Cave[y+1][x])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yballoon_sB;
	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Xacid_splash_e;
	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	  Cave[y][x] = Yballoon_sB;
	  Cave[y+1][x] = Yballoon_s;
	  Next[y][x] = Xblank;
	  Next[y+1][x] = Xballoon;
	  return;
      }
      break;

    case 3: /* west */
      switch (Cave[y][x-1])
      {
	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  Cave[y][x] = Yballoon_wB;
	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Xacid_splash_e;
	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Xacid_splash_w;
	  Next[y][x] = Xblank;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;

	case Xblank:
	case Xacid_splash_e:
	case Xacid_splash_w:
	  Cave[y][x] = Yballoon_wB;
	  Cave[y][x-1] = Yballoon_w;
	  Next[y][x] = Xblank;
	  Next[y][x-1] = Xballoon;
	  return;
      }
      break;
  }
}

static void Lball_common(int x, int y)
{
  play_element_sound(x, y, SOUND_ball, Xball_1);

  if (lev.ball_random)
  {
    switch (RANDOM(8))
    {
      case 0:
	if (lev.ball_array[lev.ball_pos][0] != Xblank &&
	    tab_blank[Cave[y-1][x-1]])
	{
	  Cave[y-1][x-1] = Yball_blank;
	  Next[y-1][x-1] = lev.ball_array[lev.ball_pos][0];
	}
	break;

      case 1:
	if (lev.ball_array[lev.ball_pos][1] != Xblank &&
	    tab_blank[Cave[y-1][x]])
	{
	  Cave[y-1][x] = Yball_blank;
	  Next[y-1][x] = lev.ball_array[lev.ball_pos][1];
	}
	break;

      case 2:
	if (lev.ball_array[lev.ball_pos][2] != Xblank &&
	    tab_blank[Cave[y-1][x+1]])
	{
	  Cave[y-1][x+1] = Yball_blank;
	  Next[y-1][x+1] = lev.ball_array[lev.ball_pos][2];
	}
	break;

      case 3:
	if (lev.ball_array[lev.ball_pos][3] != Xblank &&
	    tab_blank[Cave[y][x-1]])
	{
	  Cave[y][x-1] = Yball_blank;
	  Next[y][x-1] = lev.ball_array[lev.ball_pos][3];
	}
	break;

      case 4:
	if (lev.ball_array[lev.ball_pos][4] != Xblank &&
	    tab_blank[Cave[y][x+1]])
	{
	  Cave[y][x+1] = Yball_blank;
	  Next[y][x+1] = lev.ball_array[lev.ball_pos][4];
	}
	break;

      case 5:
	if (lev.ball_array[lev.ball_pos][5] != Xblank &&
	    tab_blank[Cave[y+1][x-1]])
	{
	  Cave[y+1][x-1] = Yball_blank;
	  Next[y+1][x-1] = lev.ball_array[lev.ball_pos][5];
	}
	break;

      case 6:
	if (lev.ball_array[lev.ball_pos][6] != Xblank &&
	    tab_blank[Cave[y+1][x]])
	{
	  Cave[y+1][x] = Yball_blank;
	  Next[y+1][x] = lev.ball_array[lev.ball_pos][6];
	}
	break;

      case 7:
	if (lev.ball_array[lev.ball_pos][7] != Xblank &&
	    tab_blank[Cave[y+1][x+1]])
	{
	  Cave[y+1][x+1] = Yball_blank;
	  Next[y+1][x+1] = lev.ball_array[lev.ball_pos][7];
	}
	break;
    }
  }
  else
  {
    if (lev.ball_array[lev.ball_pos][0] != Xblank &&
	tab_blank[Cave[y-1][x-1]])
    {
      Cave[y-1][x-1] = Yball_blank;
      Next[y-1][x-1] = lev.ball_array[lev.ball_pos][0];
    }

    if (lev.ball_array[lev.ball_pos][1] != Xblank &&
	tab_blank[Cave[y-1][x]])
    {
      Cave[y-1][x] = Yball_blank;
      Next[y-1][x] = lev.ball_array[lev.ball_pos][1];
    }

    if (lev.ball_array[lev.ball_pos][2] != Xblank &&
	tab_blank[Cave[y-1][x+1]])
    {
      Cave[y-1][x+1] = Yball_blank;
      Next[y-1][x+1] = lev.ball_array[lev.ball_pos][2];
    }

    if (lev.ball_array[lev.ball_pos][3] != Xblank &&
	tab_blank[Cave[y][x-1]])
    {
      Cave[y][x-1] = Yball_blank;
      Next[y][x-1] = lev.ball_array[lev.ball_pos][3];
    }

    if (lev.ball_array[lev.ball_pos][4] != Xblank &&
	tab_blank[Cave[y][x+1]])
    {
      Cave[y][x+1] = Yball_blank;
      Next[y][x+1] = lev.ball_array[lev.ball_pos][4];
    }

    if (lev.ball_array[lev.ball_pos][5] != Xblank &&
	tab_blank[Cave[y+1][x-1]])
    {
      Cave[y+1][x-1] = Yball_blank;
      Next[y+1][x-1] = lev.ball_array[lev.ball_pos][5];
    }

    if (lev.ball_array[lev.ball_pos][6] != Xblank &&
	tab_blank[Cave[y+1][x]])
    {
      Cave[y+1][x] = Yball_blank;
      Next[y+1][x] = lev.ball_array[lev.ball_pos][6];
    }

    if (lev.ball_array[lev.ball_pos][7] != Xblank &&
	tab_blank[Cave[y+1][x+1]])
    {
      Cave[y+1][x+1] = Yball_blank;
      Next[y+1][x+1] = lev.ball_array[lev.ball_pos][7];
    }
  }

  lev.ball_pos = (lev.ball_pos + 1) % lev.num_ball_arrays;
}

static void Lball_1(int x, int y)
{
  if (lev.ball_state == 0)
    return;

  Cave[y][x] = Yball_1;
  Next[y][x] = Xball_2;
  if (lev.ball_cnt)
    return;

  Lball_common(x, y);
}

static void Lball_2(int x, int y)
{
  if (lev.ball_state == 0)
    return;

  Cave[y][x] = Yball_2;
  Next[y][x] = Xball_1;
  if (lev.ball_cnt)
    return;

  Lball_common(x, y);
}

static void Ldrip(int x, int y)
{
  Next[y][x] = Xdrip_fall;
}

static void Ldrip_fall(int x, int y)
{
  int temp;

  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Ydrip_1_sB;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xdrip_stretchB;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
    case Xplant:
    case Yplant:
    case Zplayer:
      Cave[y][x] = Ydrip_1_sB;
      Cave[y+1][x] = Ydrip_1_s;
      Next[y][x] = Xdrip_stretchB;
      Next[y+1][x] = Xdrip_stretch;
      return;

    default:
      switch (RANDOM(8))
      {
	case 0: temp = Xamoeba_1; break;
	case 1: temp = Xamoeba_2; break;
	case 2: temp = Xamoeba_3; break;
	case 3: temp = Xamoeba_4; break;
	case 4: temp = Xamoeba_5; break;
	case 5: temp = Xamoeba_6; break;
	case 6: temp = Xamoeba_7; break;
	case 7: temp = Xamoeba_8; break;
      }

      Cave[y][x] = temp;
      Next[y][x] = temp;
      play_element_sound(x, y, SOUND_drip, Xdrip_fall);
      return;
  }
}

static void Ldrip_stretch(int x, int y)
{
  Cave[y][x] = Ydrip_2_s;
  Next[y][x] = Xdrip_fall;
}

static void Ldrip_stretchB(int x, int y)
{
  Cave[y][x] = Ydrip_2_sB;
  Next[y][x] = Xblank;
}

static void Lwonderwall(int x, int y)
{
  if (lev.wonderwall_time && lev.wonderwall_state)
  {
    Cave[y][x] = XwonderwallB;
    play_element_sound(x, y, SOUND_wonder, Xwonderwall);
  }
}

static void Lsand_stone(int x, int y)
{
  switch (Cave[y+1][x])
  {
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      Cave[y][x] = Xsand_stonesand_quickout_1;
      if (Cave[y][x+1] == Xblank)
	Cave[y][x+1] = Xacid_splash_e;
      if (Cave[y][x-1] == Xblank)
	Cave[y][x-1] = Xacid_splash_w;
      Next[y][x] = Xsand_stonesand_quickout_2;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xblank:
    case Xacid_splash_e:
    case Xacid_splash_w:
      Cave[y][x] = Xsand_stonesand_quickout_1;
      Cave[y+1][x] = Xsand_stoneout_1;
      Next[y][x] = Xsand_stonesand_quickout_2;
      Next[y+1][x] = Xsand_stoneout_2;
      return;

    case Xsand:
      Cave[y][x] = Xsand_stonesand_1;
      Cave[y+1][x] = Xsand_sandstone_1;
      Next[y][x] = Xsand_stonesand_2;
      Next[y+1][x] = Xsand_sandstone_2;
      return;
  }
}

static void Lsand_stonein_1(int x, int y)
{
  Next[y][x] = Xsand_stonein_2;
}

static void Lsand_stonein_2(int x, int y)
{
  Next[y][x] = Xsand_stonein_3;
}

static void Lsand_stonein_3(int x, int y)
{
  Next[y][x] = Xsand_stonein_4;
}

static void Lsand_stonein_4(int x, int y)
{
  Next[y][x] = Xblank;
}

static void Lsand_stonesand_1(int x, int y)
{
  Next[y][x] = Xsand_stonesand_2;
}

static void Lsand_stonesand_2(int x, int y)
{
  Next[y][x] = Xsand_stonesand_3;
}

static void Lsand_stonesand_3(int x, int y)
{
  Next[y][x] = Xsand_stonesand_4;
}

static void Lsand_stonesand_4(int x, int y)
{
  Next[y][x] = Xsand;
}

#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
static void Lsand_stonesand_quickout_1(int x, int y)
{
  Next[y][x] = Xsand_stonesand_quickout_2;
}

static void Lsand_stonesand_quickout_2(int x, int y)
{
  Next[y][x] = Xsand;
}
#endif

static void Lsand_stoneout_1(int x, int y)
{
  Next[y][x] = Xsand_stoneout_2;
}

static void Lsand_stoneout_2(int x, int y)
{
  Next[y][x] = Xstone_fall;
}

static void Lsand_sandstone_1(int x, int y)
{
  Next[y][x] = Xsand_sandstone_2;
}

static void Lsand_sandstone_2(int x, int y)
{
  Next[y][x] = Xsand_sandstone_3;
}

static void Lsand_sandstone_3(int x, int y)
{
  Next[y][x] = Xsand_sandstone_4;
}

static void Lsand_sandstone_4(int x, int y)
{
  Next[y][x] = Xsand_stone;
}

static void Lslidewall_ns(int x, int y)
{
  if (tab_blank[Cave[y-1][x]])
  {
    Cave[y-1][x] = Yslidewall_ns_blank;
    Next[y-1][x] = Xslidewall_ns;
    play_element_sound(x, y, SOUND_slidewall, Xslidewall_ns);
  }

  if (tab_blank[Cave[y+1][x]])
  {
    Cave[y+1][x] = Yslidewall_ns_blank;
    Next[y+1][x] = Xslidewall_ns;
    play_element_sound(x, y, SOUND_slidewall, Xslidewall_ns);
  }
}

static void Lslidewall_ew(int x, int y)
{
  if (tab_blank[Cave[y][x+1]])
  {
    Cave[y][x+1] = Yslidewall_ew_blank;
    Next[y][x+1] = Xslidewall_ew;
    play_element_sound(x, y, SOUND_slidewall, Xslidewall_ew);
  }

  if (tab_blank[Cave[y][x-1]])
  {
    Cave[y][x-1] = Yslidewall_ew_blank;
    Next[y][x-1] = Xslidewall_ew;
    play_element_sound(x, y, SOUND_slidewall, Xslidewall_ew);
  }
}

static void Lexit(int x, int y)
{
  if (lev.required > 0)
    return;

  switch (RANDOM(64) / 21)
  {
    case 0:
      Cave[y][x] = Xexit_1;
      Next[y][x] = Xexit_2;
      break;

    case 1:
      Cave[y][x] = Xexit_2;
      Next[y][x] = Xexit_3;
      break;

    default:
      Cave[y][x] = Xexit_3;
      Next[y][x] = Xexit_1;
      break;
  }

  play_element_sound(x, y, SOUND_exit_open, Xexit);
}

static void Lexit_1(int x, int y)
{
  Next[y][x] = Xexit_2;
}

static void Lexit_2(int x, int y)
{
  Next[y][x] = Xexit_3;
}

static void Lexit_3(int x, int y)
{
  Next[y][x] = Xexit_1;
}

static void Lacid_1(int x, int y)
{
  Next[y][x] = Xacid_2;
}

static void Lacid_2(int x, int y)
{
  Next[y][x] = Xacid_3;
}

static void Lacid_3(int x, int y)
{
  Next[y][x] = Xacid_4;
}

static void Lacid_4(int x, int y)
{
  Next[y][x] = Xacid_5;
}

static void Lacid_5(int x, int y)
{
  Next[y][x] = Xacid_6;
}

static void Lacid_6(int x, int y)
{
  Next[y][x] = Xacid_7;
}

static void Lacid_7(int x, int y)
{
  Next[y][x] = Xacid_8;
}

static void Lacid_8(int x, int y)
{
  Next[y][x] = Xacid_1;
}

#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
static void Lfake_acid_1(int x, int y)
{
  Next[y][x] = Xfake_acid_2;
}

static void Lfake_acid_2(int x, int y)
{
  Next[y][x] = Xfake_acid_3;
}

static void Lfake_acid_3(int x, int y)
{
  Next[y][x] = Xfake_acid_4;
}

static void Lfake_acid_4(int x, int y)
{
  Next[y][x] = Xfake_acid_5;
}

static void Lfake_acid_5(int x, int y)
{
  Next[y][x] = Xfake_acid_6;
}

static void Lfake_acid_6(int x, int y)
{
  Next[y][x] = Xfake_acid_7;
}

static void Lfake_acid_7(int x, int y)
{
  Next[y][x] = Xfake_acid_8;
}

static void Lfake_acid_8(int x, int y)
{
  Next[y][x] = Xfake_acid_1;
}
#endif

static void Lpause(int x, int y)
{
  Next[y][x] = Xblank;
}

static void Ldynamite_1(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_1);
  Next[y][x] = Xdynamite_2;
}

static void Ldynamite_2(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_2);
  Next[y][x] = Xdynamite_3;
}

static void Ldynamite_3(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_3);
  Next[y][x] = Xdynamite_4;
}

static void Ldynamite_4(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_4);
  Next[y][x] = Zdynamite;
  Boom[y-1][x-1] = Xblank;
  Boom[y-1][x] = Xblank;
  Boom[y-1][x+1] = Xblank;
  Boom[y][x-1] = Xblank;
  Boom[y][x] = Xblank;
  Boom[y][x+1] = Xblank;
  Boom[y+1][x-1] = Xblank;
  Boom[y+1][x] = Xblank;
  Boom[y+1][x+1] = Xblank;
}

static void Lwheel(int x, int y)
{
  if (lev.wheel_cnt && x == lev.wheel_x && y == lev.wheel_y)
    Cave[y][x] = XwheelB;
}

static void Lswitch(int x, int y)
{
  if (lev.ball_state)
    Cave[y][x] = XswitchB;
}

static void Lfake_amoeba(int x, int y)
{
  if (lev.lenses_cnt)
    Cave[y][x] = Xfake_amoebaB;
}

static void Lfake_blank(int x, int y)
{
  if (lev.lenses_cnt)
    Cave[y][x] = Xfake_blankB;
}

static void Lfake_grass(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xfake_grassB;
}

static void Lfake_door_1(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_1;
}

static void Lfake_door_2(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_2;
}

static void Lfake_door_3(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_3;
}

static void Lfake_door_4(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_4;
}

static void Lfake_door_5(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_5;
}

static void Lfake_door_6(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_6;
}

static void Lfake_door_7(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_7;
}

static void Lfake_door_8(int x, int y)
{
  if (lev.magnify_cnt)
    Cave[y][x] = Xdoor_8;
}

static void Lboom_1(int x, int y)
{
  Next[y][x] = Xboom_2;
#if !PLAY_ELEMENT_SOUND
  if (x != lev.exit_x && y != lev.exit_y)
    play_sound(x, y, SOUND_boom);
  else
    lev.exit_x = lev.exit_y = -1;
#endif
}

static void Lboom_2(int x, int y)
{
  Next[y][x] = Boom[y][x];
}

static void Lboom_android(int x, int y)
{
#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, Xandroid);
#endif

  Lboom_1(x, y);
}

void synchro_2(void)
{
  int x = 0;
  int y = 1;
  short *cave_cache = Cave[y];	/* might be a win */
  int element;

  seed = RandomEM;
  score = 0;

 loop:

  element = cave_cache[++x];

  switch (element)
  {
    case Xacid_1:		Lacid_1(x, y);			goto loop;
    case Xacid_2:		Lacid_2(x, y);			goto loop;
    case Xacid_3:		Lacid_3(x, y);			goto loop;
    case Xacid_4:		Lacid_4(x, y);			goto loop;
    case Xacid_5:		Lacid_5(x, y);			goto loop;
    case Xacid_6:		Lacid_6(x, y);			goto loop;
    case Xacid_7:		Lacid_7(x, y);			goto loop;
    case Xacid_8:		Lacid_8(x, y);			goto loop;

#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
    case Xfake_acid_1:		Lfake_acid_1(x, y);		goto loop;
    case Xfake_acid_2:		Lfake_acid_2(x, y);		goto loop;
    case Xfake_acid_3:		Lfake_acid_3(x, y);		goto loop;
    case Xfake_acid_4:		Lfake_acid_4(x, y);		goto loop;
    case Xfake_acid_5:		Lfake_acid_5(x, y);		goto loop;
    case Xfake_acid_6:		Lfake_acid_6(x, y);		goto loop;
    case Xfake_acid_7:		Lfake_acid_7(x, y);		goto loop;
    case Xfake_acid_8:		Lfake_acid_8(x, y);		goto loop;
#endif

    case Xandroid:		Landroid(x, y);			goto loop;
    case Xandroid_1_n:		Landroid_1_n(x, y);		goto loop;
    case Xandroid_2_n:		Landroid_2_n(x, y);		goto loop;
    case Xandroid_1_e:		Landroid_1_e(x, y);		goto loop;
    case Xandroid_2_e:		Landroid_2_e(x, y);		goto loop;
    case Xandroid_1_s:		Landroid_1_s(x, y);		goto loop;
    case Xandroid_2_s:		Landroid_2_s(x, y);		goto loop;
    case Xandroid_1_w:		Landroid_1_w(x, y);		goto loop;
    case Xandroid_2_w:		Landroid_2_w(x, y);		goto loop;

    case Xeater_n:		Leater_n(x, y);			goto loop;
    case Xeater_e:		Leater_e(x, y);			goto loop;
    case Xeater_s:		Leater_s(x, y);			goto loop;
    case Xeater_w:		Leater_w(x, y);			goto loop;

    case Xalien:		Lalien(x, y);			goto loop;
    case Xalien_pause:		Lalien_pause(x, y);		goto loop;

    case Xbug_1_n:		Lbug_1_n(x, y);			goto loop;
    case Xbug_2_n:		Lbug_2_n(x, y);			goto loop;
    case Xbug_1_e:		Lbug_1_e(x, y);			goto loop;
    case Xbug_2_e:		Lbug_2_e(x, y);			goto loop;
    case Xbug_1_s:		Lbug_1_s(x, y);			goto loop;
    case Xbug_2_s:		Lbug_2_s(x, y);			goto loop;
    case Xbug_1_w:		Lbug_1_w(x, y);			goto loop;
    case Xbug_2_w:		Lbug_2_w(x, y);			goto loop;

    case Xtank_1_n:		Ltank_1_n(x, y);		goto loop;
    case Xtank_2_n:		Ltank_2_n(x, y);		goto loop;
    case Xtank_1_e:		Ltank_1_e(x, y);		goto loop;
    case Xtank_2_e:		Ltank_2_e(x, y);		goto loop;
    case Xtank_1_s:		Ltank_1_s(x, y);		goto loop;
    case Xtank_2_s:		Ltank_2_s(x, y);		goto loop;
    case Xtank_1_w:		Ltank_1_w(x, y);		goto loop;
    case Xtank_2_w:		Ltank_2_w(x, y);		goto loop;

    case Xemerald:		Lemerald(x, y);			goto loop;
    case Xemerald_pause:	Lemerald_pause(x, y);		goto loop;
    case Xemerald_fall:		Lemerald_fall(x, y);		goto loop;

    case Xdiamond:		Ldiamond(x, y);			goto loop;
    case Xdiamond_pause:	Ldiamond_pause(x, y);		goto loop;
    case Xdiamond_fall:		Ldiamond_fall(x, y);		goto loop;

    case Xstone:		Lstone(x, y);			goto loop;
    case Xstone_pause:		Lstone_pause(x, y);		goto loop;
    case Xstone_fall:		Lstone_fall(x, y);		goto loop;

    case Xbomb:			Lbomb(x, y);			goto loop;
    case Xbomb_pause:		Lbomb_pause(x, y);		goto loop;
    case Xbomb_fall:		Lbomb_fall(x, y);		goto loop;

    case Xnut:			Lnut(x, y);			goto loop;
    case Xnut_pause:		Lnut_pause(x, y);		goto loop;
    case Xnut_fall:		Lnut_fall(x, y);		goto loop;

    case Xspring:		Lspring(x, y);			goto loop;
    case Xspring_pause:		Lspring_pause(x, y);		goto loop;
    case Xspring_e:		Lspring_e(x, y);		goto loop;
    case Xspring_w:		Lspring_w(x, y);		goto loop;
    case Xspring_fall:		Lspring_fall(x, y);		goto loop;

    case Xpush_emerald_e:	Lpush_emerald_e(x, y);		goto loop;
    case Xpush_emerald_w:	Lpush_emerald_w(x, y);		goto loop;
    case Xpush_diamond_e:	Lpush_diamond_e(x, y);		goto loop;
    case Xpush_diamond_w:	Lpush_diamond_w(x, y);		goto loop;
    case Xpush_stone_e:		Lpush_stone_e(x, y);		goto loop;
    case Xpush_stone_w:		Lpush_stone_w(x, y);		goto loop;
    case Xpush_bomb_e:		Lpush_bomb_e(x, y);		goto loop;
    case Xpush_bomb_w:		Lpush_bomb_w(x, y);		goto loop;
    case Xpush_nut_e:		Lpush_nut_e(x, y);		goto loop;
    case Xpush_nut_w:		Lpush_nut_w(x, y);		goto loop;
    case Xpush_spring_e:	Lpush_spring_e(x, y);		goto loop;
    case Xpush_spring_w:	Lpush_spring_w(x, y);		goto loop;

    case Xdynamite_1:		Ldynamite_1(x, y);		goto loop;
    case Xdynamite_2:		Ldynamite_2(x, y);		goto loop;
    case Xdynamite_3:		Ldynamite_3(x, y);		goto loop;
    case Xdynamite_4:		Ldynamite_4(x, y);		goto loop;

    case Xfake_door_1:		Lfake_door_1(x, y);		goto loop;
    case Xfake_door_2:		Lfake_door_2(x, y);		goto loop;
    case Xfake_door_3:		Lfake_door_3(x, y);		goto loop;
    case Xfake_door_4:		Lfake_door_4(x, y);		goto loop;
    case Xfake_door_5:		Lfake_door_5(x, y);		goto loop;
    case Xfake_door_6:		Lfake_door_6(x, y);		goto loop;
    case Xfake_door_7:		Lfake_door_7(x, y);		goto loop;
    case Xfake_door_8:		Lfake_door_8(x, y);		goto loop;

    case Xballoon:		Lballoon(x, y);			goto loop;

    case Xball_1:		Lball_1(x, y);			goto loop;
    case Xball_2:		Lball_2(x, y);			goto loop;

    case Xdrip:			Ldrip(x, y);			goto loop;
    case Xdrip_fall:		Ldrip_fall(x, y);		goto loop;
    case Xdrip_stretch:		Ldrip_stretch(x, y);		goto loop;
    case Xdrip_stretchB:	Ldrip_stretchB(x, y);		goto loop;

    case Xwonderwall:		Lwonderwall(x, y);		goto loop;

    case Xwheel:		Lwheel(x, y);			goto loop;

    case Xswitch:		Lswitch(x, y);			goto loop;

    case Xfake_blank:		Lfake_blank(x, y);		goto loop;
    case Xfake_grass:		Lfake_grass(x, y);		goto loop;
    case Xfake_amoeba:		Lfake_amoeba(x, y);		goto loop;

    case Xsand_stone:		Lsand_stone(x, y);		goto loop;
    case Xsand_stonein_1:	Lsand_stonein_1(x, y);		goto loop;
    case Xsand_stonein_2:	Lsand_stonein_2(x, y);		goto loop;
    case Xsand_stonein_3:	Lsand_stonein_3(x, y);		goto loop;
    case Xsand_stonein_4:	Lsand_stonein_4(x, y);		goto loop;
    case Xsand_sandstone_1:	Lsand_sandstone_1(x, y);	goto loop;
    case Xsand_sandstone_2:	Lsand_sandstone_2(x, y);	goto loop;
    case Xsand_sandstone_3:	Lsand_sandstone_3(x, y);	goto loop;
    case Xsand_sandstone_4:	Lsand_sandstone_4(x, y);	goto loop;
    case Xsand_stonesand_1:	Lsand_stonesand_1(x, y);	goto loop;
    case Xsand_stonesand_2:	Lsand_stonesand_2(x, y);	goto loop;
    case Xsand_stonesand_3:	Lsand_stonesand_3(x, y);	goto loop;
    case Xsand_stonesand_4:	Lsand_stonesand_4(x, y);	goto loop;
    case Xsand_stoneout_1:	Lsand_stoneout_1(x, y);		goto loop;
    case Xsand_stoneout_2:	Lsand_stoneout_2(x, y);		goto loop;
#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
    case Xsand_stonesand_quickout_1:Lsand_stonesand_quickout_1(x, y);goto loop;
    case Xsand_stonesand_quickout_2:Lsand_stonesand_quickout_2(x, y);goto loop;
#endif

    case Xslidewall_ns:		Lslidewall_ns(x, y);		goto loop;
    case Xslidewall_ew:		Lslidewall_ew(x, y);		goto loop;

    case Xexit:			Lexit(x, y);			goto loop;
    case Xexit_1:		Lexit_1(x, y);			goto loop;
    case Xexit_2:		Lexit_2(x, y);			goto loop;
    case Xexit_3:		Lexit_3(x, y);			goto loop;

    case Xpause:		Lpause(x, y);			goto loop;

    case Xboom_bug:		Lboom_bug(x, y);		goto loop;
    case Xboom_bomb:		Lboom_tank(x, y);		goto loop;
    case Xboom_android:		Lboom_android(x, y);		goto loop;
    case Xboom_1:		Lboom_1(x, y);			goto loop;
    case Xboom_2:		Lboom_2(x, y);			goto loop;

    case Zborder:
      if (++y < HEIGHT - 1)
      {
	x = 0;
	cave_cache = Cave[y];
	goto loop;
      }

      goto done;

    default:
      goto loop;
  }

 done:

  if (ply[0].alive || ply[1].alive || ply[2].alive || ply[3].alive)
    lev.score += score;		/* only add a score if someone is alive */
  else
    game_em.game_over = TRUE;

  RandomEM = seed;

  {
    void *temp = Cave;

    /* triple buffering */
    Cave = Next;
    Next = Draw;
    Draw = temp;
  }
}
