/* 2008-09-24 23:20:29
 *
 * David Tritscher
 *
 * my own version of the emerald mine engine
 */

#include "main_em.h"


#define SPRING_ROLL	/* spring rolling off round things continues to roll */
#define ACID_ROLL	/* rolling objects go into acid rather than remove it */
#define ACID_PLAYER	/* player gets killed by acid, but without explosion */

#define RANDOM_BUG	/* handle problem with old tapes using 64-bit random */

#define RANDOM(x)	((seed = seed << 31 | seed >> 1) % x)

static short **cave, **next, **boom;
static unsigned int seed;
static int score;

static const byte is_blank[GAME_TILE_MAX] =
{
  [Xblank]		= 1,
  [Xsplash_e]		= 1,
  [Xsplash_w]		= 1,
  [Xfake_acid_1]	= 1,
  [Xfake_acid_2]	= 1,
  [Xfake_acid_3]	= 1,
  [Xfake_acid_4]	= 1,
  [Xfake_acid_5]	= 1,
  [Xfake_acid_6]	= 1,
  [Xfake_acid_7]	= 1,
  [Xfake_acid_8]	= 1
};

static const byte is_blank_or_acid[GAME_TILE_MAX] =
{
  [Xblank]		= 1,
  [Xsplash_e]		= 1,
  [Xsplash_w]		= 1,
  [Xfake_acid_1]	= 1,
  [Xfake_acid_2]	= 1,
  [Xfake_acid_3]	= 1,
  [Xfake_acid_4]	= 1,
  [Xfake_acid_5]	= 1,
  [Xfake_acid_6]	= 1,
  [Xfake_acid_7]	= 1,
  [Xfake_acid_8]	= 1,
  [Xacid_1]		= 1,
  [Xacid_2]		= 1,
  [Xacid_3]		= 1,
  [Xacid_4]		= 1,
  [Xacid_5]		= 1,
  [Xacid_6]		= 1,
  [Xacid_7]		= 1,
  [Xacid_8]		= 1
};

static const byte is_player[GAME_TILE_MAX] =
{
  [Zplayer]		= 1,
  [Xfake_acid_1_player]	= 1,
  [Xfake_acid_2_player]	= 1,
  [Xfake_acid_3_player]	= 1,
  [Xfake_acid_4_player]	= 1,
  [Xfake_acid_5_player]	= 1,
  [Xfake_acid_6_player]	= 1,
  [Xfake_acid_7_player]	= 1,
  [Xfake_acid_8_player]	= 1
};

static const byte add_player[GAME_TILE_MAX] =
{
  [Xblank]		= Zplayer,
  [Xsplash_e]		= Zplayer,
  [Xsplash_w]		= Zplayer,
  [Xfake_acid_1]	= Xfake_acid_1_player,
  [Xfake_acid_2]	= Xfake_acid_2_player,
  [Xfake_acid_3]	= Xfake_acid_3_player,
  [Xfake_acid_4]	= Xfake_acid_4_player,
  [Xfake_acid_5]	= Xfake_acid_5_player,
  [Xfake_acid_6]	= Xfake_acid_6_player,
  [Xfake_acid_7]	= Xfake_acid_7_player,
  [Xfake_acid_8]	= Xfake_acid_8_player
};

static const byte remove_player[GAME_TILE_MAX] =
{
  [Zplayer]		= Xblank,
  [Xfake_acid_1_player]	= Xfake_acid_1,
  [Xfake_acid_2_player]	= Xfake_acid_2,
  [Xfake_acid_3_player]	= Xfake_acid_3,
  [Xfake_acid_4_player]	= Xfake_acid_4,
  [Xfake_acid_5_player]	= Xfake_acid_5,
  [Xfake_acid_6_player]	= Xfake_acid_6,
  [Xfake_acid_7_player]	= Xfake_acid_7,
  [Xfake_acid_8_player]	= Xfake_acid_8
};

static const byte is_amoeba[GAME_TILE_MAX] =
{
  [Xfake_amoeba]	= 1,
  [Yfake_amoeba]	= 1,
  [Xamoeba_1]		= 1,
  [Xamoeba_2]		= 1,
  [Xamoeba_3]		= 1,
  [Xamoeba_4]		= 1,
  [Xamoeba_5]		= 1,
  [Xamoeba_6]		= 1,
  [Xamoeba_7]		= 1,
  [Xamoeba_8]		= 1
};

static byte is_android_blank[GAME_TILE_MAX] =
{
  [Xblank]		= 1,
  [Xsplash_e]		= 1,
  [Xsplash_w]		= 1,
  [Xfake_acid_1]	= 1,
  [Xfake_acid_2]	= 1,
  [Xfake_acid_3]	= 1,
  [Xfake_acid_4]	= 1,
  [Xfake_acid_5]	= 1,
  [Xfake_acid_6]	= 1,
  [Xfake_acid_7]	= 1,
  [Xfake_acid_8]	= 1
};

static const byte is_android_walkable[GAME_TILE_MAX] =
{
  [Xblank]		= 1,
  [Xsplash_e]		= 1,
  [Xsplash_w]		= 1,
  [Xfake_acid_1]	= 1,
  [Xfake_acid_2]	= 1,
  [Xfake_acid_3]	= 1,
  [Xfake_acid_4]	= 1,
  [Xfake_acid_5]	= 1,
  [Xfake_acid_6]	= 1,
  [Xfake_acid_7]	= 1,
  [Xfake_acid_8]	= 1,
  [Xplant]		= 1
};

static void Lboom_generic(int x, int y, int element, int element_middle)
{
  boom[x-1][y-1] = element;
  boom[x][y-1]   = element;
  boom[x+1][y-1] = element;
  boom[x-1][y]   = element;
  boom[x][y]     = element_middle;
  boom[x+1][y]   = element;
  boom[x-1][y+1] = element;
  boom[x][y+1]   = element;
  boom[x+1][y+1] = element;
}

static void Lboom_bug(int x, int y)
{
  if (game_em.use_old_explosions)
    next[x][y] = Zbug;

  Lboom_generic(x, y, Xemerald, Xdiamond);

#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, Xbug_1_n);
#endif
}

static void Lboom_tank(int x, int y)
{
  if (game_em.use_old_explosions)
    next[x][y] = Ztank;

  Lboom_generic(x, y, Xblank, Xblank);

#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, Xtank_1_n);
#endif
}

static void Lboom_eater(int x, int y)
{
  if (game_em.use_old_explosions)
    next[x][y] = Zeater;

  boom[x-1][y-1] = lev.eater_array[lev.eater_pos][0];
  boom[x][y-1]   = lev.eater_array[lev.eater_pos][1];
  boom[x+1][y-1] = lev.eater_array[lev.eater_pos][2];
  boom[x-1][y]   = lev.eater_array[lev.eater_pos][3];
  boom[x][y]     = lev.eater_array[lev.eater_pos][4];
  boom[x+1][y]   = lev.eater_array[lev.eater_pos][5];
  boom[x-1][y+1] = lev.eater_array[lev.eater_pos][6];
  boom[x][y+1]   = lev.eater_array[lev.eater_pos][7];
  boom[x+1][y+1] = lev.eater_array[lev.eater_pos][8];

  lev.eater_pos = (lev.eater_pos + 1) % lev.num_eater_arrays;

#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, Xeater_n);
#endif
}

static void Lboom_bug_old(int x, int y)
{
  if (!game_em.use_old_explosions)
    return;

  Lboom_bug(x, y);
}

static void Lboom_tank_old(int x, int y)
{
  if (!game_em.use_old_explosions)
    return;

  Lboom_tank(x, y);
}

static void Lboom_eater_old(int x, int y)
{
  if (!game_em.use_old_explosions)
    return;

  Lboom_eater(x, y);
}

static void Lboom_bug_new(int x, int y, boolean chain_explosion)
{
  if (game_em.use_old_explosions)
    return;

  if (chain_explosion)
    cave[x][y] = Xchain;

  Lboom_bug(x, y);
}

static void Lboom_tank_new(int x, int y, boolean chain_explosion)
{
  if (game_em.use_old_explosions)
    return;

  if (chain_explosion)
    cave[x][y] = Xchain;

  Lboom_tank(x, y);
}

static void Lboom_eater_new(int x, int y, boolean chain_explosion)
{
  if (game_em.use_old_explosions)
    return;

  if (chain_explosion)
    cave[x][y] = Xchain;

  Lboom_eater(x, y);
}

static void Lboom_cave_new(int x, int y, int element)
{
  if (game_em.use_old_explosions)
    return;

  cave[x][y] = element;
}

static void Lboom_next_new(int x, int y, int element)
{
  if (game_em.use_old_explosions)
    return;

  next[x][y] = element;
}

static void Lpush_element_old(int x, int y, int element)
{
  if (!game_em.use_old_push_elements)
    return;

  cave[x][y] = element;
  next[x][y] = element;
}

static boolean player_killed(struct PLAYER *ply)
{
  int x = ply->x;
  int y = ply->y;

  if (!ply->alive)
    return FALSE;

  if (lev.killed_out_of_time && game.time_limit)
    return TRUE;

  switch (cave[x][y-1])
  {
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
      return TRUE;
  }

  switch (cave[x+1][y])
  {
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
      return TRUE;
  }

  switch (cave[x][y+1])
  {
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
      return TRUE;
  }

  switch (cave[x-1][y])
  {
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
      return TRUE;
  }

  switch (cave[x][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xdynamite_1:
    case Xdynamite_2:
    case Xdynamite_3:
    case Xdynamite_4:
      return FALSE;
  }

  return TRUE;
}

static void kill_player(struct PLAYER *ply)
{
  int x = ply->x;
  int y = ply->y;

  ply->alive = FALSE;

  switch (cave[x][y-1])
  {
    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x][y-1] = Xboom_bug;
      Lboom_bug_new(x, y-1, TRUE);
      break;

    case Xtank_1_n:
    case Xtank_1_e:
    case Xtank_1_s:
    case Xtank_1_w:
    case Xtank_2_n:
    case Xtank_2_e:
    case Xtank_2_s:
    case Xtank_2_w:
      cave[x][y-1] = Xboom_tank;
      Lboom_tank_new(x, y-1, TRUE);
      break;
  }

  switch (cave[x+1][y])
  {
    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x+1][y] = Xboom_bug;
      Lboom_bug_new(x+1, y, TRUE);
      break;

    case Xtank_1_n:
    case Xtank_1_e:
    case Xtank_1_s:
    case Xtank_1_w:
    case Xtank_2_n:
    case Xtank_2_e:
    case Xtank_2_s:
    case Xtank_2_w:
      cave[x+1][y] = Xboom_tank;
      Lboom_tank_new(x+1, y, TRUE);
      break;
  }

  switch (cave[x][y+1])
  {
    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x][y+1] = Xboom_bug;
      Lboom_bug_new(x, y+1, TRUE);
      break;

    case Xtank_1_n:
    case Xtank_1_e:
    case Xtank_1_s:
    case Xtank_1_w:
    case Xtank_2_n:
    case Xtank_2_e:
    case Xtank_2_s:
    case Xtank_2_w:
      cave[x][y+1] = Xboom_tank;
      Lboom_tank_new(x, y+1, TRUE);
      break;
  }

  switch (cave[x-1][y])
  {
    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x-1][y] = Xboom_bug;
      Lboom_bug_new(x-1, y, TRUE);
      break;

    case Xtank_1_n:
    case Xtank_1_e:
    case Xtank_1_s:
    case Xtank_1_w:
    case Xtank_2_n:
    case Xtank_2_e:
    case Xtank_2_s:
    case Xtank_2_w:
      cave[x-1][y] = Xboom_tank;
      Lboom_tank_new(x-1, y, TRUE);
      break;
  }

  switch (cave[x][y])
  {
    case Xexit_1:
    case Xexit_2:
    case Xexit_3:
      lev.exit_x = x;
      lev.exit_y = y;
      play_element_sound(x, y, SOUND_exit_leave, Xexit_1);
      break;

    default:
      play_element_sound(x, y, SOUND_die, Zplayer);
      break;
  }

  switch (cave[x][y])
  {
#ifdef ACID_PLAYER
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      break;
#endif

    default:
      cave[x][y] = Xboom_1;
      boom[x][y] = Xblank;
      break;
  }
}

static boolean player_digfield(struct PLAYER *ply, int dx, int dy)
{
  int anim = (dx < 0 ? 3 : dx > 0 ? 1 : dy < 0 ? 0 : dy > 0 ? 2 : 2);
  int oldx = ply->x;
  int oldy = ply->y;
  int x = oldx + dx;
  int y = oldy + dy;
  boolean result = TRUE;

  if (!dx && !dy)			/* no direction specified */
    return FALSE;

  if (dx && dy && ply->joy_snap)	/* more than one direction specified */
    return FALSE;

  if (!ply->joy_snap)			/* player wants to move */
  {
    int element = cave[x][y];

    switch (cave[x][y])
    {
      /* fire is released */
      case Xblank:
      case Xsplash_e:
      case Xsplash_w:
      case Xfake_acid_1:
      case Xfake_acid_2:
      case Xfake_acid_3:
      case Xfake_acid_4:
      case Xfake_acid_5:
      case Xfake_acid_6:
      case Xfake_acid_7:
      case Xfake_acid_8:
	cave[x][y] = add_player[element];
	next[x][y] = add_player[element];

	play_element_sound(x, y, SOUND_blank, Xblank);
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xacid_1:
      case Xacid_2:
      case Xacid_3:
      case Xacid_4:
      case Xacid_5:
      case Xacid_6:
      case Xacid_7:
      case Xacid_8:
#ifdef ACID_PLAYER
	if (cave[x+1][y-1] == Xblank)
	  cave[x+1][y-1] = Xsplash_e;
	if (cave[x-1][y-1] == Xblank)
	  cave[x-1][y-1] = Xsplash_w;
	play_element_sound(x, y, SOUND_acid, Xacid_1);
	// FALL THROUGH
#endif
      case Xboom_android:
      case Xboom_1:
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
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xgrass:
	cave[x][y] = (dy ? (dy < 0 ? Ygrass_nB : Ygrass_sB) :
		      (dx > 0 ? Ygrass_eB : Ygrass_wB));
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_dirt, Xgrass);
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xdirt:
	cave[x][y] = (dy ? (dy < 0 ? Ydirt_nB : Ydirt_sB) :
		      (dx > 0 ? Ydirt_eB : Ydirt_wB));
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_dirt, Xdirt);
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xdiamond:
      case Xdiamond_pause:
	cave[x][y] = Ydiamond_blank;
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.diamond_score;
	lev.gems_needed = lev.gems_needed < 3 ? 0 : lev.gems_needed - 3;
	game.snapshot.collected_item = TRUE;
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xemerald:
      case Xemerald_pause:
	cave[x][y] = Yemerald_blank;
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.emerald_score;
	lev.gems_needed = lev.gems_needed < 1 ? 0 : lev.gems_needed - 1;
	game.snapshot.collected_item = TRUE;
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xdynamite:
	cave[x][y] = Ydynamite_blank;
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.dynamite_score;
	ply->dynamite = ply->dynamite > 9998 ? 9999 : ply->dynamite + 1;
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xkey_1:
	ply->keys |= 0x01;
	cave[x][y] = Ykey_1_blank;
	goto key_walk;

      case Xkey_2:
	ply->keys |= 0x02;
	cave[x][y] = Ykey_2_blank;
	goto key_walk;

      case Xkey_3:
	ply->keys |= 0x04;
	cave[x][y] = Ykey_3_blank;
	goto key_walk;

      case Xkey_4:
	ply->keys |= 0x08;
	cave[x][y] = Ykey_4_blank;
	goto key_walk;

      case Xkey_5:
	ply->keys |= 0x10;
	cave[x][y] = Ykey_5_blank;
	goto key_walk;

      case Xkey_6:
	ply->keys |= 0x20;
	cave[x][y] = Ykey_6_blank;
	goto key_walk;

      case Xkey_7:
	ply->keys |= 0x40;
	cave[x][y] = Ykey_7_blank;
	goto key_walk;

      case Xkey_8:
	ply->keys |= 0x80;
	cave[x][y] = Ykey_8_blank;
	goto key_walk;

      key_walk:

	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.key_score;
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xlenses:
	cave[x][y] = Ylenses_blank;
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.lenses_score;
	lev.lenses_cnt = lev.lenses_time;
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xmagnify:
	cave[x][y] = Ymagnify_blank;
	next[x][y] = Zplayer;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.magnify_score;
	lev.magnify_cnt = lev.magnify_time;
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xstone:
	if (dy)
	  break;

	switch (cave[x+dx][y])
	{
          case Xblank:
          case Xsplash_e:
          case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	    cave[x+dx][y] = dx > 0 ? Ystone_e : Ystone_w;
	    next[x+dx][y] = Xstone_pause;
	    goto stone_walk;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (cave[x+dx+1][y-1] == Xblank)
	      cave[x+dx+1][y-1] = Xsplash_e;
	    if (cave[x+dx-1][y-1] == Xblank)
	      cave[x+dx-1][y-1] = Xsplash_w;
	    play_element_sound(x, y, SOUND_acid, Xacid_1);

          stone_walk:

	    cave[x][y] = dx > 0 ? Ystone_eB : Ystone_wB;
	    next[x][y] = Zplayer;
	    play_element_sound(x, y, SOUND_roll, Xstone);
	    ply->x = x;
	    break;
	}

	ply->anim = PLY_push_n + anim;
	break;

      case Xbomb:
	if (dy)
	  break;

	switch (cave[x+dx][y])
	{
	  case Xblank:
          case Xsplash_e:
          case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	    cave[x+dx][y] = dx > 0 ? Ybomb_e : Ybomb_w;
	    next[x+dx][y] = Xbomb_pause;
	    goto bomb_walk;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (cave[x+dx+1][y-1] == Xblank)
	      cave[x+dx+1][y-1] = Xsplash_e;
	    if (cave[x+dx-1][y-1] == Xblank)
	      cave[x+dx-1][y-1] = Xsplash_w;
	    play_element_sound(x, y, SOUND_acid, Xacid_1);

          bomb_walk:

	    cave[x][y] = dx > 0 ? Ybomb_eB : Ybomb_wB;
	    next[x][y] = Zplayer;
	    play_element_sound(x, y, SOUND_roll, Xbomb);
	    ply->x = x;
	    break;
	}

	ply->anim = PLY_push_n + anim;
	break;

      case Xnut:
	if (dy)
	  break;

	switch (cave[x+dx][y])
	{
          case Xblank:
          case Xsplash_e:
          case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	    cave[x+dx][y] = dx > 0 ? Ynut_e : Ynut_w;
	    next[x+dx][y] = Xnut_pause;
	    goto nut_walk;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (cave[x+dx+1][y-1] == Xblank)
	      cave[x+dx+1][y-1] = Xsplash_e;
	    if (cave[x+dx-1][y-1] == Xblank)
	      cave[x+dx-1][y-1] = Xsplash_w;
	    play_element_sound(x, y, SOUND_acid, Xacid_1);

          nut_walk:

	    cave[x][y] = dx > 0 ? Ynut_eB : Ynut_wB;
	    next[x][y] = Zplayer;
	    play_element_sound(x, y, SOUND_roll, Xnut);
	    ply->x = x;
	    break;
	}

	ply->anim = PLY_push_n + anim;
	break;

      case Xspring:
	if (dy)
	  break;

	switch (cave[x+dx][y])
	{
          case Xblank:
          case Xsplash_e:
          case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	    cave[x+dx][y] = dx > 0 ? Yspring_e : Yspring_w;
	    next[x+dx][y] = dx > 0 ? Xspring_e : Xspring_w;
	    goto spring_walk;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (cave[x+dx+1][y-1] == Xblank)
	      cave[x+dx+1][y-1] = Xsplash_e;
	    if (cave[x+dx-1][y-1] == Xblank)
	      cave[x+dx-1][y-1] = Xsplash_w;
	    play_element_sound(x, y, SOUND_acid, Xacid_1);

	  spring_walk:

	    cave[x][y] = dx > 0 ? Yspring_eB : Yspring_wB;
	    next[x][y] = Zplayer;
	    play_element_sound(x, y, SOUND_roll, Xspring);
	    ply->x = x;
	    break;

          case Xalien:
          case Xalien_pause:
	    cave[x][y] = dx > 0 ? Yspring_alien_eB : Yspring_alien_wB;
	    cave[x+dx][y] = dx > 0 ? Yspring_alien_e : Yspring_alien_w;
	    next[x][y] = Zplayer;
	    next[x+dx][y] = dx > 0 ? Xspring_e : Xspring_w;
	    play_element_sound(x, y, SOUND_slurp, Xalien);
	    lev.score += lev.slurp_score;
	    ply->x = x;
	    break;
	}

	ply->anim = PLY_push_n + anim;
	break;

      case Xspring_pause:
      case Xstone_pause:
      case Xbomb_pause:
      case Xnut_pause:
      case Xsand_stonein_1:
      case Xsand_stonein_2:
      case Xsand_stonein_3:
      case Xsand_stonein_4:
	if (dy)
	  break;

	ply->anim = PLY_push_n + anim;
	break;

      case Xballoon:
	switch (cave[x+dx][y+dy])
	{
          case Xblank:
          case Xsplash_e:
          case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	    cave[x+dx][y+dy] = (dy ? (dy < 0 ? Yballoon_n : Yballoon_s) :
				(dx > 0 ? Yballoon_e : Yballoon_w));
	    next[x+dx][y+dy] = Xballoon;
	    goto balloon_walk;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (cave[x+dx+1][y+dy-1] == Xblank)
	      cave[x+dx+1][y+dy-1] = Xsplash_e;
	    if (cave[x+dx-1][y+dy-1] == Xblank)
	      cave[x+dx-1][y+dy-1] = Xsplash_w;
	    play_element_sound(x, y, SOUND_acid, Xacid_1);

	  balloon_walk:

	    cave[x][y] = (dy ? (dy < 0 ? Yballoon_nB : Yballoon_sB) :
			  (dx > 0 ? Yballoon_eB : Yballoon_wB));
	    next[x][y] = Zplayer;
	    play_element_sound(x, y, SOUND_push, Xballoon);
	    ply->x = x;
	    ply->y = y;
	    break;
	}

	ply->anim = PLY_push_n + anim;
	break;

      case Xandroid:
      case Xandroid_1_n:
      case Xandroid_2_n:
      case Xandroid_1_e:
      case Xandroid_2_e:
      case Xandroid_1_s:
      case Xandroid_2_s:
      case Xandroid_1_w:
      case Xandroid_2_w:
	switch (cave[x+dx][y+dy])
	{
          case Xblank:
          case Xsplash_e:
          case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	    cave[x+dx][y+dy] = (dy ? (dy < 0 ? Yandroid_n : Yandroid_s) :
				(dx > 0 ? Yandroid_e : Yandroid_w));
	    next[x+dx][y+dy] = (dy ? (dy < 0 ? Xandroid_2_n : Xandroid_2_s) :
				(dx > 0 ? Xandroid_2_e : Xandroid_2_w));
	    goto android_walk;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (cave[x+dx+1][y+dy-1] == Xblank)
	      cave[x+dx+1][y+dy-1] = Xsplash_e;
	    if (cave[x+dx-1][y+dy-1] == Xblank)
	      cave[x+dx-1][y+dy-1] = Xsplash_w;
	    play_element_sound(x, y, SOUND_acid, Xacid_1);

	  android_walk:

	    cave[x][y] = (dy ? (dy < 0 ? Yandroid_nB : Yandroid_sB) :
			  (dx > 0 ? Yandroid_eB : Yandroid_wB));
	    next[x][y] = Zplayer;
	    play_element_sound(x, y, SOUND_push, Xandroid);
	    ply->x = x;
	    ply->y = y;
	    break;
	}

	ply->anim = PLY_push_n + anim;
	break;

      case Xdoor_1:
      case Xfake_door_1:
	if (ply->keys & 0x01)
	  goto door_walk;
	else
	  break;

      case Xdoor_2:
      case Xfake_door_2:
	if (ply->keys & 0x02)
	  goto door_walk;
	else
	  break;

      case Xdoor_3:
      case Xfake_door_3:
	if (ply->keys & 0x04)
	  goto door_walk;
	else
	  break;

      case Xdoor_4:
      case Xfake_door_4:
	if (ply->keys & 0x08)
	  goto door_walk;
	else
	  break;

      case Xdoor_5:
      case Xfake_door_5:
	if (ply->keys & 0x10)
	  goto door_walk;
	else
	  break;

      case Xdoor_6:
      case Xfake_door_6:
	if (ply->keys & 0x20)
	  goto door_walk;
	else
	  break;

      case Xdoor_7:
      case Xfake_door_7:
	if (ply->keys & 0x40)
	  goto door_walk;
	else
	  break;

      case Xdoor_8:
      case Xfake_door_8:
	if (ply->keys & 0x80)
	  goto door_walk;
	else
	  break;

      door_walk:

	if (!is_blank[cave[x+dx][y+dy]])
	  break;

	int element_next = cave[x+dx][y+dy];

	cave[x+dx][y+dy] = add_player[element_next];
	next[x+dx][y+dy] = add_player[element_next];

	play_element_sound(x, y, SOUND_door, element);
	ply->anim = PLY_walk_n + anim;
	ply->x = x + dx;
	ply->y = y + dy;
	break;

      case Xwheel:
	play_element_sound(x, y, SOUND_press, element);
	lev.wheel_cnt = lev.wheel_time;
	lev.wheel_x = x;
	lev.wheel_y = y;
	break;

      case Xwind_n:
	lev.wind_direction = 0;
	goto wind_walk;

      case Xwind_e:
	lev.wind_direction = 1;
	goto wind_walk;

      case Xwind_s:
	lev.wind_direction = 2;
	goto wind_walk;

      case Xwind_w:
	lev.wind_direction = 3;
	goto wind_walk;

      case Xwind_any:
	lev.wind_direction = dy ? (dy < 0 ? 0 : 2) : (dx > 0 ? 1 : 3);
	goto wind_walk;

      wind_walk:

	play_element_sound(x, y, SOUND_press, element);
	lev.wind_cnt = lev.wind_time;
	break;

      case Xwind_stop:
	play_element_sound(x, y, SOUND_press, element);
	lev.wind_cnt = 0;
	break;

      case Xswitch:
	play_element_sound(x, y, SOUND_press, element);
	lev.ball_cnt = lev.ball_time;
	lev.ball_active = !lev.ball_active;
	break;

      case Xplant:
	cave[x][y] = Yplant;
	next[x][y] = Xplant;
	play_element_sound(x, y, SOUND_blank, Xplant);
	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xexit_1:
      case Xexit_2:
      case Xexit_3:
	lev.home--;

	if (lev.home == 0)
	  game_em.level_solved = TRUE;

	ply->anim = PLY_walk_n + anim;
	ply->x = x;
	ply->y = y;

	break;
    }

    if (ply->x == oldx && ply->y == oldy)	/* no movement */
      result = FALSE;
  }
  else					/* player wants to snap */
  {
    int element = cave[x][y];

    switch (cave[x][y])
    {
      /* fire is pressed */

      case Xgrass:
	cave[x][y] = Ygrass_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_dirt, element);
	ply->anim = PLY_shoot_n + anim;
	break;

      case Xdirt:
	cave[x][y] = Ydirt_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_dirt, element);
	ply->anim = PLY_shoot_n + anim;
	break;

      case Xdiamond:
      case Xdiamond_pause:
	cave[x][y] = Ydiamond_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.diamond_score;
	lev.gems_needed = lev.gems_needed < 3 ? 0 : lev.gems_needed - 3;
	game.snapshot.collected_item = TRUE;
	ply->anim = PLY_walk_n + anim;
	break;

      case Xemerald:
      case Xemerald_pause:
	cave[x][y] = Yemerald_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.emerald_score;
	lev.gems_needed = lev.gems_needed < 1 ? 0 : lev.gems_needed - 1;
	game.snapshot.collected_item = TRUE;
	ply->anim = PLY_walk_n + anim;
	break;

      case Xdynamite:
	cave[x][y] = Ydynamite_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.dynamite_score;
	ply->dynamite = ply->dynamite > 9998 ? 9999 : ply->dynamite + 1;
	ply->anim = PLY_walk_n + anim;
	break;

      case Xkey_1:
	ply->keys |= 0x01;
	cave[x][y] = Ykey_1_blank;
	goto key_shoot;

      case Xkey_2:
	ply->keys |= 0x02;
	cave[x][y] = Ykey_2_blank;
	goto key_shoot;

      case Xkey_3:
	ply->keys |= 0x04;
	cave[x][y] = Ykey_3_blank;
	goto key_shoot;

      case Xkey_4:
	ply->keys |= 0x08;
	cave[x][y] = Ykey_4_blank;
	goto key_shoot;

      case Xkey_5:
	ply->keys |= 0x10;
	cave[x][y] = Ykey_5_blank;
	goto key_shoot;

      case Xkey_6:
	ply->keys |= 0x20;
	cave[x][y] = Ykey_6_blank;
	goto key_shoot;

      case Xkey_7:
	ply->keys |= 0x40;
	cave[x][y] = Ykey_7_blank;
	goto key_shoot;

      case Xkey_8:
	ply->keys |= 0x80;
	cave[x][y] = Ykey_8_blank;
	goto key_shoot;

      key_shoot:

	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.key_score;
	ply->anim = PLY_walk_n + anim;
	break;

      case Xlenses:
	cave[x][y] = Ylenses_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.lenses_score;
	lev.lenses_cnt = lev.lenses_time;
	ply->anim = PLY_walk_n + anim;
	break;

      case Xmagnify:
	cave[x][y] = Ymagnify_blank;
	next[x][y] = Xblank;
	play_element_sound(x, y, SOUND_collect, element);
	lev.score += lev.magnify_score;
	lev.magnify_cnt = lev.magnify_time;
	ply->anim = PLY_walk_n + anim;
	break;

      default:
	result = FALSE;
    }
  }

  /* check for wrap-around movement */
  if (ply->x < lev.left ||
      ply->x > lev.right - 1)
    play_element_sound(oldx, oldy, SOUND_door, Xdoor_1);

  return result;
}

static void check_player(struct PLAYER *ply)
{
  int oldx = ply->x;
  int oldy = ply->y;
  int x = oldx;
  int y = oldy;
  int dx = 0, dy = 0;

  game_em.last_player_direction[ply->num] = MV_NONE;

  if (ply->joy_w)		/* west */
  {
    x--;
    dx = -1;
  }
  else if (ply->joy_e)		/* east */
  {
    x++;
    dx = 1;
  }

  if (ply->joy_n)		/* north */
  {
    y--;
    dy = -1;
  }
  else if (ply->joy_s)		/* south */
  {
    y++;
    dy = 1;
  }

  if (dx || dy)
  {
    int oldx = ply->x;
    int oldy = ply->y;
    int x = oldx + dx;
    int y = oldy + dy;
    boolean players_visible_before_move;
    boolean players_visible_after_move;
    boolean can_move;

    players_visible_before_move = checkIfAllPlayersFitToScreen();

    ply->x = x;
    ply->y = y;

    players_visible_after_move = checkIfAllPlayersFitToScreen();

    /*
      player is allowed to move only in the following cases:
      - it is not needed to display all players (not focussed to all players)
      - all players are (still or again) visible after the move
      - some players were already outside visible screen area before the move
    */
    can_move = (game.centered_player_nr != -1 ||
		players_visible_after_move ||
		!players_visible_before_move);

    ply->x = oldx;
    ply->y = oldy;

    if (!can_move)
    {
      ply->joy_n = FALSE;
      ply->joy_e = FALSE;
      ply->joy_s = FALSE;
      ply->joy_w = FALSE;

      return;
    }
  }

  if (dx == 0 && dy == 0)
  {
    ply->joy_stick = FALSE;

    if (ply->joy_drop)
    {
      if (++ply->dynamite_cnt == 5 && ply->dynamite)
      {
	cave[x][y] = Xdynamite_1;
	play_element_sound(x, y, SOUND_dynamite, Xdynamite_1);
	ply->dynamite--;
      }
    }
    else
    {
      ply->dynamite_cnt = 0;
    }

    /* be a bit more random if the player doesn't move */
    game_em.random += 7;

    return;
  }

  ply->joy_stick = TRUE;
  ply->joy_n = FALSE;
  ply->joy_e = FALSE;
  ply->joy_s = FALSE;
  ply->joy_w = FALSE;

  ply->dynamite_cnt = 0;	/* reset dynamite timer if we move */

  if (!ply->joy_snap)		/* player wants to move */
  {
    boolean moved = FALSE;

    if (ply->last_move_dir & MV_HORIZONTAL)
    {
      if (!(moved = player_digfield(ply, 0, dy)))
	moved = player_digfield(ply, dx, 0);
    }
    else
    {
      if (!(moved = player_digfield(ply, dx, 0)))
	moved = player_digfield(ply, 0, dy);
    }

    if (moved)
    {
      if (oldx != ply->x)
	ply->last_move_dir = (dx < 0 ? MV_LEFT : MV_RIGHT);
      else if (oldy != ply->y)
	ply->last_move_dir = (dy < 0 ? MV_UP : MV_DOWN);

      game_em.any_player_moving = TRUE;
      game_em.last_moving_player = ply->num;
      game_em.last_player_direction[ply->num] = ply->last_move_dir;
    }
  }
  else					/* player wants to snap */
  {
    game_em.any_player_snapping = player_digfield(ply, dx, dy);
  }
}

static void set_nearest_player_xy(int x, int y, int *dx, int *dy)
{
  int distance, distance_shortest = CAVE_WIDTH + CAVE_HEIGHT;
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

static void Lacid_1(int x, int y)
{
  next[x][y] = Xacid_2;
}

static void Lacid_2(int x, int y)
{
  next[x][y] = Xacid_3;
}

static void Lacid_3(int x, int y)
{
  next[x][y] = Xacid_4;
}

static void Lacid_4(int x, int y)
{
  next[x][y] = Xacid_5;
}

static void Lacid_5(int x, int y)
{
  next[x][y] = Xacid_6;
}

static void Lacid_6(int x, int y)
{
  next[x][y] = Xacid_7;
}

static void Lacid_7(int x, int y)
{
  next[x][y] = Xacid_8;
}

static void Lacid_8(int x, int y)
{
  next[x][y] = Xacid_1;
}

static void Lfake_acid_1(int x, int y)
{
  next[x][y] = Xfake_acid_2;
}

static void Lfake_acid_2(int x, int y)
{
  next[x][y] = Xfake_acid_3;
}

static void Lfake_acid_3(int x, int y)
{
  next[x][y] = Xfake_acid_4;
}

static void Lfake_acid_4(int x, int y)
{
  next[x][y] = Xfake_acid_5;
}

static void Lfake_acid_5(int x, int y)
{
  next[x][y] = Xfake_acid_6;
}

static void Lfake_acid_6(int x, int y)
{
  next[x][y] = Xfake_acid_7;
}

static void Lfake_acid_7(int x, int y)
{
  next[x][y] = Xfake_acid_8;
}

static void Lfake_acid_8(int x, int y)
{
  next[x][y] = Xfake_acid_1;
}

static void Landroid(int x, int y)
{
  int dx, dy, temp;

  if (lev.android_clone_cnt == 0)
  {
    if (!is_android_blank[cave[x-1][y-1]] &&
	!is_android_blank[cave[x][y-1]]   &&
	!is_android_blank[cave[x+1][y-1]] &&
	!is_android_blank[cave[x-1][y]]   &&
	!is_android_blank[cave[x+1][y]]   &&
	!is_android_blank[cave[x-1][y+1]] &&
	!is_android_blank[cave[x][y+1]]   &&
	!is_android_blank[cave[x+1][y+1]])
      goto android_move;

    switch (RANDOM(8))
    {
      /* randomly find an object to clone */

      case 0: /* S,NE,W,NW,SE,E,SW,N */
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	goto android_move;

      case 1: /* NW,SE,N,S,NE,SW,E,W */
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	goto android_move;

      case 2: /* SW,E,S,W,N,NW,SE,NE */
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	goto android_move;

      case 3: /* N,SE,NE,E,W,S,NW,SW */
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	goto android_move;

      case 4: /* SE,NW,E,NE,SW,W,N,S */
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	goto android_move;

      case 5: /* NE,W,SE,SW,S,N,E,NW */
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	goto android_move;

      case 6: /* E,N,SW,S,NW,NE,SE,W */
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	goto android_move;

      case 7: /* W,SW,NW,N,E,SE,NE,S */
	temp = lev.android_array[cave[x-1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x-1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y-1]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y]];   if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y+1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x+1][y-1]]; if (temp != Xblank) break;
	temp = lev.android_array[cave[x][y+1]];   if (temp != Xblank) break;
	goto android_move;
    }

    next[x][y] = temp;	/* the item we chose to clone */
    play_element_sound(x, y, SOUND_android_clone, temp);

    switch (RANDOM(8))
    {
      /* randomly find a direction to move */

      case 0: /* S,NE,W,NW,SE,E,SW,N */
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	goto android_move;

      case 1: /* NW,SE,N,S,NE,SW,E,W */
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	goto android_move;

      case 2: /* SW,E,S,W,N,NW,SE,NE */
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	goto android_move;

      case 3: /* N,SE,NE,E,W,S,NW,SW */
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	goto android_move;

      case 4: /* SE,NW,E,NE,SW,W,N,S */
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	goto android_move;

      case 5: /* NE,W,SE,SW,S,N,E,NW */
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	goto android_move;

      case 6: /* E,N,SW,S,NW,NE,SE,W */
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	goto android_move;

      case 7: /* W,SW,NW,N,E,SE,NE,S */
	if (is_android_blank[cave[x-1][y]])   goto android_w;
	if (is_android_blank[cave[x-1][y+1]]) goto android_sw;
	if (is_android_blank[cave[x-1][y-1]]) goto android_nw;
	if (is_android_blank[cave[x][y-1]])   goto android_n;
	if (is_android_blank[cave[x+1][y]])   goto android_e;
	if (is_android_blank[cave[x+1][y+1]]) goto android_se;
	if (is_android_blank[cave[x+1][y-1]]) goto android_ne;
	if (is_android_blank[cave[x][y+1]])   goto android_s;
	goto android_move;
    }
  }

 android_move:
  if (lev.android_move_cnt == 0)
  {
    if (is_player[cave[x-1][y-1]] ||
	is_player[cave[x][y-1]]   ||
	is_player[cave[x+1][y-1]] ||
	is_player[cave[x-1][y]]   ||
	is_player[cave[x+1][y]]   ||
	is_player[cave[x-1][y+1]] ||
	is_player[cave[x][y+1]]   ||
	is_player[cave[x+1][y+1]])
      goto android_still;

    set_nearest_player_xy(x, y, &dx, &dy);

    next[x][y] = Xblank;	/* assume we will move */
    temp = ((x < dx) + 1 - (x > dx)) + ((y < dy) + 1 - (y > dy)) * 3;

    if (RANDOM(2))
    {
      switch (temp)
      {
	/* attempt clockwise move first if direct path is blocked */

	case 0: /* north west */
	  if (is_android_walkable[cave[x-1][y-1]]) goto android_nw;
	  if (is_android_walkable[cave[x][y-1]])   goto android_n;
	  if (is_android_walkable[cave[x-1][y]])   goto android_w;
	  break;

	case 1: /* north */
	  if (is_android_walkable[cave[x][y-1]])   goto android_n;
	  if (is_android_walkable[cave[x+1][y-1]]) goto android_ne;
	  if (is_android_walkable[cave[x-1][y-1]]) goto android_nw;
	  break;

	case 2: /* north east */
	  if (is_android_walkable[cave[x+1][y-1]]) goto android_ne;
	  if (is_android_walkable[cave[x+1][y]])   goto android_e;
	  if (is_android_walkable[cave[x][y-1]])   goto android_n;
	  break;

	case 3: /* west */
	  if (is_android_walkable[cave[x-1][y]])   goto android_w;
	  if (is_android_walkable[cave[x-1][y-1]]) goto android_nw;
	  if (is_android_walkable[cave[x-1][y+1]]) goto android_sw;
	  break;

	case 4: /* nowhere */
	  break;

	case 5: /* east */
	  if (is_android_walkable[cave[x+1][y]])   goto android_e;
	  if (is_android_walkable[cave[x+1][y+1]]) goto android_se;
	  if (is_android_walkable[cave[x+1][y-1]]) goto android_ne;
	  break;

	case 6: /* south west */
	  if (is_android_walkable[cave[x-1][y+1]]) goto android_sw;
	  if (is_android_walkable[cave[x-1][y]])   goto android_w;
	  if (is_android_walkable[cave[x][y+1]])   goto android_s;
	  break;

	case 7: /* south */
	  if (is_android_walkable[cave[x][y+1]])   goto android_s;
	  if (is_android_walkable[cave[x-1][y+1]]) goto android_sw;
	  if (is_android_walkable[cave[x+1][y+1]]) goto android_se;
	  break;

	case 8: /* south east */
	  if (is_android_walkable[cave[x+1][y+1]]) goto android_se;
	  if (is_android_walkable[cave[x][y+1]])   goto android_s;
	  if (is_android_walkable[cave[x+1][y]])   goto android_e;
	  break;
      }
    }
    else
    {
      switch (temp)
      {
	/* attempt counterclockwise move first if direct path is blocked */

	case 0: /* north west */
	  if (is_android_walkable[cave[x-1][y-1]]) goto android_nw;
	  if (is_android_walkable[cave[x-1][y]])   goto android_w;
	  if (is_android_walkable[cave[x][y-1]])   goto android_n;
	  break;

	case 1: /* north */
	  if (is_android_walkable[cave[x][y-1]])   goto android_n;
	  if (is_android_walkable[cave[x-1][y-1]]) goto android_nw;
	  if (is_android_walkable[cave[x+1][y-1]]) goto android_ne;
	  break;

	case 2: /* north east */
	  if (is_android_walkable[cave[x+1][y-1]]) goto android_ne;
	  if (is_android_walkable[cave[x][y-1]])   goto android_n;
	  if (is_android_walkable[cave[x+1][y]])   goto android_e;
	  break;

	case 3: /* west */
	  if (is_android_walkable[cave[x-1][y]])   goto android_w;
	  if (is_android_walkable[cave[x-1][y+1]]) goto android_sw;
	  if (is_android_walkable[cave[x-1][y-1]]) goto android_nw;
	  break;

	case 4: /* nowhere */
	  break;

	case 5: /* east */
	  if (is_android_walkable[cave[x+1][y]])   goto android_e;
	  if (is_android_walkable[cave[x+1][y-1]]) goto android_ne;
	  if (is_android_walkable[cave[x+1][y+1]]) goto android_se;
	  break;

	case 6: /* south west */
	  if (is_android_walkable[cave[x-1][y+1]]) goto android_sw;
	  if (is_android_walkable[cave[x][y+1]])   goto android_s;
	  if (is_android_walkable[cave[x-1][y]])   goto android_w;
	  break;

	case 7: /* south */
	  if (is_android_walkable[cave[x][y+1]])   goto android_s;
	  if (is_android_walkable[cave[x+1][y+1]]) goto android_se;
	  if (is_android_walkable[cave[x-1][y+1]]) goto android_sw;
	  break;

	case 8: /* south east */
	  if (is_android_walkable[cave[x+1][y+1]]) goto android_se;
	  if (is_android_walkable[cave[x+1][y]])   goto android_e;
	  if (is_android_walkable[cave[x][y+1]])   goto android_s;
	  break;
      }
    }
  }

 android_still:
  next[x][y] = Xandroid;
  return;

 android_n:
  cave[x][y] = Yandroid_nB;
  cave[x][y-1] = Yandroid_n;
  next[x][y-1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_ne:
  cave[x][y] = Yandroid_neB;
  cave[x+1][y-1] = Yandroid_ne;
  next[x+1][y-1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_e:
  cave[x][y] = Yandroid_eB;
  cave[x+1][y] = Yandroid_e;
  next[x+1][y] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_se:
  cave[x][y] = Yandroid_seB;
  cave[x+1][y+1] = Yandroid_se;
  next[x+1][y+1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_s:
  cave[x][y] = Yandroid_sB;
  cave[x][y+1] = Yandroid_s;
  next[x][y+1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_sw:
  cave[x][y] = Yandroid_swB;
  cave[x-1][y+1] = Yandroid_sw;
  next[x-1][y+1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_w:
  cave[x][y] = Yandroid_wB;
  cave[x-1][y] = Yandroid_w;
  next[x-1][y] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;

 android_nw:
  cave[x][y] = Yandroid_nwB;
  cave[x-1][y-1] = Yandroid_nw;
  next[x-1][y-1] = Xandroid;
  play_element_sound(x, y, SOUND_android_move, Xandroid);
  return;
}

static void Landroid_1_n(int x, int y)
{
  switch (cave[x][y-1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_nB;
      next[x][y] = Xblank;
      cave[x][y-1] = Yandroid_n;
      next[x][y-1] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_n);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_nB;
      next[x][y] = Xblank;
      if (cave[x+1][y-2] == Xblank)
	cave[x+1][y-2] = Xsplash_e;
      if (cave[x-1][y-2] == Xblank)
	cave[x-1][y-2] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_n(int x, int y)
{
  switch (cave[x][y-1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_nB;
      next[x][y] = Xblank;
      cave[x][y-1] = Yandroid_n;
      next[x][y-1] = Xandroid_1_n;
      play_element_sound(x, y, SOUND_android_move, Xandroid_2_n);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_nB;
      next[x][y] = Xblank;
      if (cave[x+1][y-2] == Xblank)
	cave[x+1][y-2] = Xsplash_e;
      if (cave[x-1][y-2] == Xblank)
	cave[x-1][y-2] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_1_e(int x, int y)
{
  switch (cave[x+1][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Yandroid_e;
      next[x+1][y] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_e);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_eB;
      next[x][y] = Xblank;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_e(int x, int y)
{
  switch (cave[x+1][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Yandroid_e;
      next[x+1][y] = Xandroid_1_e;
      play_element_sound(x, y, SOUND_android_move, Xandroid_2_e);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_eB;
      next[x][y] = Xblank;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_1_s(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yandroid_s;
      next[x][y+1] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_s);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_s(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yandroid_s;
      next[x][y+1] = Xandroid_1_s;
      play_element_sound(x, y, SOUND_android_move, Xandroid_2_s);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_1_w(int x, int y)
{
  switch (cave[x-1][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Yandroid_w;
      next[x-1][y] = Xandroid;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_w);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_wB;
      next[x][y] = Xblank;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Landroid_2_w(int x, int y)
{
  switch (cave[x-1][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yandroid_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Yandroid_w;
      next[x-1][y] = Xandroid_1_w;
      play_element_sound(x, y, SOUND_android_move, Xandroid_1_w);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yandroid_wB;
      next[x][y] = Xblank;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      Landroid(x, y);
      return;
  }
}

static void Leater_n(int x, int y)
{
  if (cave[x+1][y] == Xdiamond)
  {
    cave[x+1][y] = Ydiamond_blank;
    next[x+1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  if (cave[x][y+1] == Xdiamond)
  {
    cave[x][y+1] = Ydiamond_blank;
    next[x][y+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  if (cave[x-1][y] == Xdiamond)
  {
    cave[x-1][y] = Ydiamond_blank;
    next[x-1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  if (cave[x][y-1] == Xdiamond)
  {
    cave[x][y-1] = Ydiamond_blank;
    next[x][y-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_n);
    return;
  }

  switch (cave[x][y-1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Yeater_nB;
      next[x][y] = Xblank;
      cave[x][y-1] = Yeater_n;
      next[x][y-1] = Xeater_n;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yeater_nB;
      next[x][y] = Xblank;
      if (cave[x+1][y-2] == Xblank)
	cave[x+1][y-2] = Xsplash_e;
      if (cave[x-1][y-2] == Xblank)
	cave[x-1][y-2] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      next[x][y] = RANDOM(2) ? Xeater_e : Xeater_w;
      play_element_sound(x, y, SOUND_eater, Xeater_n);
      return;
  }
}

static void Leater_e(int x, int y)
{
  if (cave[x][y+1] == Xdiamond)
  {
    cave[x][y+1] = Ydiamond_blank;
    next[x][y+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  if (cave[x-1][y] == Xdiamond)
  {
    cave[x-1][y] = Ydiamond_blank;
    next[x-1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  if (cave[x][y-1] == Xdiamond)
  {
    cave[x][y-1] = Ydiamond_blank;
    next[x][y-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  if (cave[x+1][y] == Xdiamond)
  {
    cave[x+1][y] = Ydiamond_blank;
    next[x+1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_e);
    return;
  }

  switch (cave[x+1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Yeater_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Yeater_e;
      next[x+1][y] = Xeater_e;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yeater_eB;
      next[x][y] = Xblank;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      next[x][y] = RANDOM(2) ? Xeater_n : Xeater_s;
      play_element_sound(x, y, SOUND_eater, Xeater_e);
      return;
  }
}

static void Leater_s(int x, int y)
{
  if (cave[x-1][y] == Xdiamond)
  {
    cave[x-1][y] = Ydiamond_blank;
    next[x-1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  if (cave[x][y-1] == Xdiamond)
  {
    cave[x][y-1] = Ydiamond_blank;
    next[x][y-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  if (cave[x+1][y] == Xdiamond)
  {
    cave[x+1][y] = Ydiamond_blank;
    next[x+1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  if (cave[x][y+1] == Xdiamond)
  {
    cave[x][y+1] = Ydiamond_blank;
    next[x][y+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_s);
    return;
  }

  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Yeater_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yeater_s;
      next[x][y+1] = Xeater_s;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yeater_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      next[x][y] = RANDOM(2) ? Xeater_e : Xeater_w;
      play_element_sound(x, y, SOUND_eater, Xeater_s);
      return;
  }
}

static void Leater_w(int x, int y)
{
  if (cave[x][y-1] == Xdiamond)
  {
    cave[x][y-1] = Ydiamond_blank;
    next[x][y-1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  if (cave[x+1][y] == Xdiamond)
  {
    cave[x+1][y] = Ydiamond_blank;
    next[x+1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  if (cave[x][y+1] == Xdiamond)
  {
    cave[x][y+1] = Ydiamond_blank;
    next[x][y+1] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  if (cave[x-1][y] == Xdiamond)
  {
    cave[x-1][y] = Ydiamond_blank;
    next[x-1][y] = Xblank;
    play_element_sound(x, y, SOUND_eater_eat, Xeater_w);
    return;
  }

  switch (cave[x-1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Yeater_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Yeater_w;
      next[x-1][y] = Xeater_w;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yeater_wB;
      next[x][y] = Xblank;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      next[x][y] = RANDOM(2) ? Xeater_n : Xeater_s;
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
      switch (cave[x][y-1])
      {
	case Zplayer:
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	case Xfake_acid_1_player:
	case Xfake_acid_2_player:
	case Xfake_acid_3_player:
	case Xfake_acid_4_player:
	case Xfake_acid_5_player:
	case Xfake_acid_6_player:
	case Xfake_acid_7_player:
	case Xfake_acid_8_player:
	case Xplant:
	case Yplant:
	  cave[x][y] = Yalien_nB;
	  next[x][y] = Xblank;
	  cave[x][y-1] = Yalien_n;
	  next[x][y-1] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yalien_nB;
	  next[x][y] = Xblank;
	  if (cave[x+1][y-2] == Xblank)
	    cave[x+1][y-2] = Xsplash_e;
	  if (cave[x-1][y-2] == Xblank)
	    cave[x-1][y-2] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
    }
    else if (y < dy)
    {
      switch (cave[x][y+1])
      {
	case Zplayer:
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	case Xfake_acid_1_player:
	case Xfake_acid_2_player:
	case Xfake_acid_3_player:
	case Xfake_acid_4_player:
	case Xfake_acid_5_player:
	case Xfake_acid_6_player:
	case Xfake_acid_7_player:
	case Xfake_acid_8_player:
	case Xplant:
	case Yplant:
	  cave[x][y] = Yalien_sB;
	  next[x][y] = Xblank;
	  cave[x][y+1] = Yalien_s;
	  next[x][y+1] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yalien_sB;
	  next[x][y] = Xblank;
	  if (cave[x+1][y] == Xblank)
	    cave[x+1][y] = Xsplash_e;
	  if (cave[x-1][y] == Xblank)
	    cave[x-1][y] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
    }
  }
  else
  {
    if (x < dx)
    {
      switch (cave[x+1][y])
      {
	case Zplayer:
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	case Xfake_acid_1_player:
	case Xfake_acid_2_player:
	case Xfake_acid_3_player:
	case Xfake_acid_4_player:
	case Xfake_acid_5_player:
	case Xfake_acid_6_player:
	case Xfake_acid_7_player:
	case Xfake_acid_8_player:
	case Xplant:
	case Yplant:
	  cave[x][y] = Yalien_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Yalien_e;
	  next[x+1][y] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yalien_eB;
	  next[x][y] = Xblank;
	  if (cave[x+2][y-1] == Xblank)
	    cave[x+2][y-1] = Xsplash_e;
	  if (cave[x][y-1] == Xblank)
	    cave[x][y-1] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
    }
    else if (x > dx)
    {
      switch (cave[x-1][y])
      {
	case Zplayer:
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	case Xfake_acid_1_player:
	case Xfake_acid_2_player:
	case Xfake_acid_3_player:
	case Xfake_acid_4_player:
	case Xfake_acid_5_player:
	case Xfake_acid_6_player:
	case Xfake_acid_7_player:
	case Xfake_acid_8_player:
	case Xplant:
	case Yplant:
	  cave[x][y] = Yalien_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Yalien_w;
	  next[x-1][y] = Xalien_pause;
	  play_element_sound(x, y, SOUND_alien, Xalien);
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yalien_wB;
	  next[x][y] = Xblank;
	  if (cave[x][y-1] == Xblank)
	    cave[x][y-1] = Xsplash_e;
	  if (cave[x-2][y-1] == Xblank)
	    cave[x-2][y-1] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
    }
  }
}

static void Lalien_pause(int x, int y)
{
  next[x][y] = Xalien;
}

static void Lbug_n(int x, int y)
{
  switch (cave[x][y-1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ybug_nB;
      next[x][y] = Xblank;
      cave[x][y-1] = Ybug_n;
      next[x][y-1] = Xbug_1_n;
      play_element_sound(x, y, SOUND_bug, Xbug_1_n);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybug_nB;
      next[x][y] = Xblank;
      if (cave[x+1][y-2] == Xblank)
	cave[x+1][y-2] = Xsplash_e;
      if (cave[x-1][y-2] == Xblank)
	cave[x-1][y-2] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ybug_n_w;
      next[x][y] = Xbug_2_w;
      play_element_sound(x, y, SOUND_bug, Xbug_1_n);
      return;
  }
}

static void Lbug_1_n(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  switch (cave[x+1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ybug_n_e;
      next[x][y] = Xbug_2_e;
      play_element_sound(x, y, SOUND_bug, Xbug_1_n);
      return;

    default:
      Lbug_n(x, y);
      return;
  }
}

static void Lbug_2_n(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  Lbug_n(x, y);
}

static void Lbug_e(int x, int y)
{
  switch (cave[x+1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ybug_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Ybug_e;
      next[x+1][y] = Xbug_1_e;
      play_element_sound(x, y, SOUND_bug, Xbug_1_e);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybug_eB;
      next[x][y] = Xblank;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ybug_e_n;
      next[x][y] = Xbug_2_n;
      play_element_sound(x, y, SOUND_bug, Xbug_1_e);
      return;
  }
}

static void Lbug_1_e(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ybug_e_s;
      next[x][y] = Xbug_2_s;
      play_element_sound(x, y, SOUND_bug, Xbug_1_e);
      return;

    default:
      Lbug_e(x, y);
      return;
  }
}

static void Lbug_2_e(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  Lbug_e(x, y);
}

static void Lbug_s(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ybug_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ybug_s;
      next[x][y+1] = Xbug_1_s;
      play_element_sound(x, y, SOUND_bug, Xbug_1_s);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybug_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ybug_s_e;
      next[x][y] = Xbug_2_e;
      play_element_sound(x, y, SOUND_bug, Xbug_1_s);
      return;
  }
}

static void Lbug_1_s(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  switch (cave[x-1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ybug_s_w;
      next[x][y] = Xbug_2_w;
      play_element_sound(x, y, SOUND_bug, Xbug_1_s);
      return;

    default:
      Lbug_s(x, y);
      return;
  }
}

static void Lbug_2_s(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  Lbug_s(x, y);
}

static void Lbug_w(int x, int y)
{
  switch (cave[x-1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ybug_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Ybug_w;
      next[x-1][y] = Xbug_1_w;
      play_element_sound(x, y, SOUND_bug, Xbug_1_w);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybug_wB;
      next[x][y] = Xblank;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ybug_w_s;
      next[x][y] = Xbug_2_s;
      play_element_sound(x, y, SOUND_bug, Xbug_1_w);
      return;
  }
}

static void Lbug_1_w(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  switch (cave[x][y-1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ybug_w_n;
      next[x][y] = Xbug_2_n;
      play_element_sound(x, y, SOUND_bug, Xbug_1_w);
      return;

    default:
      Lbug_w(x, y);
      return;
  }
}

static void Lbug_2_w(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_bug(x, y);

    return;
  }

  Lbug_w(x, y);
}

static void Ltank_n(int x, int y)
{
  switch (cave[x][y-1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ytank_nB;
      next[x][y] = Xblank;
      cave[x][y-1] = Ytank_n;
      next[x][y-1] = Xtank_1_n;
      play_element_sound(x, y, SOUND_tank, Xtank_1_n);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ytank_nB;
      next[x][y] = Xblank;
      if (cave[x+1][y-2] == Xblank)
	cave[x+1][y-2] = Xsplash_e;
      if (cave[x-1][y-2] == Xblank)
	cave[x-1][y-2] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ytank_n_e;
      next[x][y] = Xtank_2_e;
      play_element_sound(x, y, SOUND_tank, Xtank_1_n);
      return;
  }
}

static void Ltank_1_n(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  switch (cave[x-1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ytank_n_w;
      next[x][y] = Xtank_2_w;
      play_element_sound(x, y, SOUND_tank, Xtank_1_n);
      return;

    default:
      Ltank_n(x, y);
      return;
  }
}

static void Ltank_2_n(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  Ltank_n(x, y);
}

static void Ltank_e(int x, int y)
{
  switch (cave[x+1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ytank_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Ytank_e;
      next[x+1][y] = Xtank_1_e;
      play_element_sound(x, y, SOUND_tank, Xtank_1_e);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ytank_eB;
      next[x][y] = Xblank;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ytank_e_s;
      next[x][y] = Xtank_2_s;
      play_element_sound(x, y, SOUND_tank, Xtank_1_e);
      return;
  }
}

static void Ltank_1_e(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  switch (cave[x][y-1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ytank_e_n;
      next[x][y] = Xtank_2_n;
      play_element_sound(x, y, SOUND_tank, Xtank_1_e);
      return;

    default:
      Ltank_e(x, y);
      return;
  }
}

static void Ltank_2_e(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  Ltank_e(x, y);
}

static void Ltank_s(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ytank_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ytank_s;
      next[x][y+1] = Xtank_1_s;
      play_element_sound(x, y, SOUND_tank, Xtank_1_s);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ytank_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ytank_s_w;
      next[x][y] = Xtank_2_w;
      play_element_sound(x, y, SOUND_tank, Xtank_1_s);
      return;
  }
}

static void Ltank_1_s(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  switch (cave[x+1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ytank_s_e;
      next[x][y] = Xtank_2_e;
      play_element_sound(x, y, SOUND_tank, Xtank_1_s);
      return;

    default:
      Ltank_s(x, y);
      return;
  }
}

static void Ltank_2_s(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  Ltank_s(x, y);
}

static void Ltank_w(int x, int y)
{
  switch (cave[x-1][y])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ytank_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Ytank_w;
      next[x-1][y] = Xtank_1_w;
      play_element_sound(x, y, SOUND_tank, Xtank_1_w);
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ytank_wB;
      next[x][y] = Xblank;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ytank_w_n;
      next[x][y] = Xtank_2_n;
      play_element_sound(x, y, SOUND_tank, Xtank_1_w);
      return;
  }
}

static void Ltank_1_w(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
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
      cave[x][y] = Ytank_w_s;
      next[x][y] = Xtank_2_s;
      play_element_sound(x, y, SOUND_tank, Xtank_1_w);
      return;

    default:
      Ltank_w(x, y);
      return;
  }
}

static void Ltank_2_w(int x, int y)
{
  if (is_amoeba[cave[x][y-1]] ||
      is_amoeba[cave[x+1][y]] ||
      is_amoeba[cave[x][y+1]] ||
      is_amoeba[cave[x-1][y]])
  {
    next[x][y] = Zboom;
    Lboom_tank(x, y);

    return;
  }

  Ltank_w(x, y);
}

static void Lemerald(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yemerald_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yemerald_s;
      next[x][y+1] = Xemerald_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yemerald_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xstone:
    case Xstone_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xnut:
    case Xnut_pause:
    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xballoon:
    case Xball_1:
    case Xball_2:
    case Xwonderwall:
    case Xswitch:
    case Xbumper:
    case Ybumper:
    case Xacid_ne:
    case Xacid_nw:
    case Xslide_ns:
    case Xslide_ew:
    case Xwall_1:
    case Xwall_2:
    case Xwall_3:
    case Xwall_4:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
    case Xsteel_1:
    case Xsteel_2:
    case Xsteel_3:
    case Xsteel_4:
      if (RANDOM(2))
      {
	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Yemerald_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Yemerald_e;
	  next[x+1][y] = Xemerald_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Yemerald_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Yemerald_w;
	  next[x-1][y] = Xemerald_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
      else
      {
	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Yemerald_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Yemerald_w;
	  next[x-1][y] = Xemerald_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Yemerald_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Yemerald_e;
	  next[x+1][y] = Xemerald_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }

    default:
      if (++lev.shine_cnt > 50)
      {
	lev.shine_cnt = RANDOM(8);
	cave[x][y] = Xemerald_shine;
      }

      return;
  }
}

static void Lemerald_pause(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yemerald_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yemerald_s;
      next[x][y+1] = Xemerald_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yemerald_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xemerald;
      next[x][y] = Xemerald;
      return;
  }
}

static void Lemerald_fall(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
      cave[x][y] = Yemerald_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yemerald_s;
      next[x][y+1] = Xemerald_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yemerald_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xwonderwall:
      if (lev.wonderwall_time > 0)
      {
	lev.wonderwall_active = TRUE;
	cave[x][y] = Yemerald_sB;
	next[x][y] = Xblank;
	if (is_blank[cave[x][y+2]])
	{
	  cave[x][y+2] = Ydiamond_s;
	  next[x][y+2] = Xdiamond_fall;
	}
	play_element_sound(x, y, SOUND_wonderfall, Xwonderwall);
	return;
      }

    default:
      cave[x][y] = Xemerald;
      next[x][y] = Xemerald;
      play_element_sound(x, y, SOUND_diamond, Xemerald);
      return;
  }
}

static void Ldiamond(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ydiamond_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ydiamond_s;
      next[x][y+1] = Xdiamond_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ydiamond_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xstone:
    case Xstone_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xnut:
    case Xnut_pause:
    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xballoon:
    case Xball_1:
    case Xball_2:
    case Xwonderwall:
    case Xswitch:
    case Xbumper:
    case Ybumper:
    case Xacid_ne:
    case Xacid_nw:
    case Xslide_ns:
    case Xslide_ew:
    case Xwall_1:
    case Xwall_2:
    case Xwall_3:
    case Xwall_4:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
    case Xsteel_1:
    case Xsteel_2:
    case Xsteel_3:
    case Xsteel_4:
      if (RANDOM(2))
      {
	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ydiamond_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ydiamond_e;
	  next[x+1][y] = Xdiamond_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ydiamond_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ydiamond_w;
	  next[x-1][y] = Xdiamond_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
      else
      {
	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ydiamond_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ydiamond_w;
	  next[x-1][y] = Xdiamond_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ydiamond_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ydiamond_e;
	  next[x+1][y] = Xdiamond_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }

    default:
      if (++lev.shine_cnt > 50)
      {
	lev.shine_cnt = RANDOM(8);
	cave[x][y] = Xdiamond_shine;
      }

      return;
  }
}

static void Ldiamond_pause(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ydiamond_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ydiamond_s;
      next[x][y+1] = Xdiamond_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ydiamond_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xdiamond;
      next[x][y] = Xdiamond;
      return;
  }
}

static void Ldiamond_fall(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
      cave[x][y] = Ydiamond_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ydiamond_s;
      next[x][y+1] = Xdiamond_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ydiamond_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xwonderwall:
      if (lev.wonderwall_time > 0)
      {
	lev.wonderwall_active = TRUE;
	cave[x][y] = Ydiamond_sB;
	next[x][y] = Xblank;
	if (is_blank[cave[x][y+2]])
	{
	  cave[x][y+2] = Ystone_s;
	  next[x][y+2] = Xstone_fall;
	}
	play_element_sound(x, y, SOUND_wonderfall, Xwonderwall);
	return;
      }

    default:
      cave[x][y] = Xdiamond;
      next[x][y] = Xdiamond;
      play_element_sound(x, y, SOUND_diamond, Xdiamond);
      return;
  }
}

static void Lstone(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xplant:
    case Yplant:
      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ystone_s;
      next[x][y+1] = Xstone_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xsand:
      cave[x][y] = Xsand_stonein_1;
      next[x][y] = Xsand_stonein_2;
      cave[x][y+1] = Xsand_sandstone_1;
      next[x][y+1] = Xsand_sandstone_2;
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xstone:
    case Xstone_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xnut:
    case Xnut_pause:
    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xballoon:
    case Xball_1:
    case Xball_2:
    case Xswitch:
    case Xbumper:
    case Ybumper:
    case Xacid_ne:
    case Xacid_nw:
    case Xlenses:
    case Xmagnify:
    case Xslide_ns:
    case Xslide_ew:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ystone_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ystone_e;
	  next[x+1][y] = Xstone_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ystone_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ystone_w;
	  next[x-1][y] = Xstone_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
      else
      {
	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ystone_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ystone_w;
	  next[x-1][y] = Xstone_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ystone_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ystone_e;
	  next[x+1][y] = Xstone_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
  }
}

static void Lstone_pause(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ystone_s;
      next[x][y+1] = Xstone_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xstone;
      next[x][y] = Xstone;
      return;
  }
}

static void Lstone_fall(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ystone_s;
      next[x][y+1] = Xstone_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xeater_n:
    case Xeater_e:
    case Xeater_s:
    case Xeater_w:
      cave[x][y] = Ystone_sB;
      Lboom_next_new(x, y, Xblank);
      cave[x][y+1] = Yeater_stone;
      next[x][y+1] = Zeater;
      Lboom_eater_old(x, y+1);
      score += lev.eater_score;
      return;

    case Xalien:
    case Xalien_pause:
      cave[x][y] = Ystone_sB;
      Lboom_next_new(x, y, Xblank);
      cave[x][y+1] = Yalien_stone;
      next[x][y+1] = Ztank;
      Lboom_tank_old(x, y+1);
      score += lev.alien_score;
      return;

    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x][y] = Ystone_sB;
      Lboom_next_new(x, y, Xblank);
      cave[x][y+1] = Ybug_stone;
      next[x][y+1] = Zbug;
      Lboom_bug_old(x, y+1);
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
      cave[x][y] = Ystone_sB;
      Lboom_next_new(x, y, Xblank);
      cave[x][y+1] = Ytank_stone;
      next[x][y+1] = Ztank;
      Lboom_tank_old(x, y+1);
      score += lev.tank_score;
      return;

    case Xdiamond:
    case Xdiamond_pause:
      switch (cave[x][y+2])
      {
	case Zplayer:
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	case Xfake_acid_1_player:
	case Xfake_acid_2_player:
	case Xfake_acid_3_player:
	case Xfake_acid_4_player:
	case Xfake_acid_5_player:
	case Xfake_acid_6_player:
	case Xfake_acid_7_player:
	case Xfake_acid_8_player:
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
	case Xandroid:
	case Xandroid_1_n:
	case Xandroid_2_n:
	case Xandroid_1_e:
	case Xandroid_2_e:
	case Xandroid_1_s:
	case Xandroid_2_s:
	case Xandroid_1_w:
	case Xandroid_2_w:
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
	case Xemerald_fall:
	case Xdiamond_fall:
	case Xstone_fall:
	case Xbomb_fall:
	case Xnut_fall:
	case Xspring_fall:
	case Xacid_s:
	  next[x][y] = Xstone;
	  play_element_sound(x, y, SOUND_stone, Xstone);
	  return;
      }

      cave[x][y] = Ystone_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ydiamond_stone;
      next[x][y+1] = Xstone_pause;
      play_element_sound(x, y, SOUND_squash, Xdiamond);
      return;

    case Xbomb:
    case Xbomb_pause:
      Lboom_cave_new(x, y, Xstone);
      Lboom_next_new(x, y, Xstone);
      cave[x][y+1] = Ybomb_blank;
      next[x][y+1] = Ztank;
      Lboom_tank_old(x, y+1);
      return;

    case Xnut:
    case Xnut_pause:
      next[x][y] = Xstone;
      cave[x][y+1] = Ynut_stone;
      next[x][y+1] = Xemerald;
      play_element_sound(x, y, SOUND_crack, Xnut);
      score += lev.nut_score;
      return;

    case Xspring:
      if (RANDOM(2))
      {
	switch (cave[x+1][y+1])
	{
	  case Xblank:
	  case Xsplash_e:
	  case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	  case Xalien:
	  case Xalien_pause:
	    cave[x][y+1] = Xspring_e;
	    break;

	  default:
	    cave[x][y+1] = Xspring_w;
	    break;
	}
      }
      else
      {
	switch (cave[x-1][y+1])
	{
	  case Xblank:
	  case Xsplash_e:
	  case Xsplash_w:
	  case Xfake_acid_1:
	  case Xfake_acid_2:
	  case Xfake_acid_3:
	  case Xfake_acid_4:
	  case Xfake_acid_5:
	  case Xfake_acid_6:
	  case Xfake_acid_7:
	  case Xfake_acid_8:
	  case Xalien:
	  case Xalien_pause:
	    cave[x][y+1] = Xspring_w;
	    break;
	  default:
	    cave[x][y+1] = Xspring_e;
	    break;
	}
      }

      next[x][y] = Xstone;
      return;

    case Xwonderwall:
      if (lev.wonderwall_time > 0)
      {
	lev.wonderwall_active = TRUE;
	cave[x][y] = Ystone_sB;
	next[x][y] = Xblank;
	if (is_blank[cave[x][y+2]])
	{
	  cave[x][y+2] = Yemerald_s;
	  next[x][y+2] = Xemerald_fall;
	}
	play_element_sound(x, y, SOUND_wonderfall, Xwonderwall);
	return;
      }

    default:
      cave[x][y] = Xstone;
      next[x][y] = Xstone;
      play_element_sound(x, y, SOUND_stone, Xstone);
      return;
  }
}

static void Lbomb(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ybomb_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ybomb_s;
      next[x][y+1] = Xbomb_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybomb_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xstone:
    case Xstone_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xnut:
    case Xnut_pause:
    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xballoon:
    case Xball_1:
    case Xball_2:
    case Xswitch:
    case Xbumper:
    case Ybumper:
    case Xacid_ne:
    case Xacid_nw:
    case Xslide_ns:
    case Xslide_ew:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ybomb_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ybomb_e;
	  next[x+1][y] = Xbomb_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ybomb_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ybomb_w;
	  next[x-1][y] = Xbomb_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
      else
      {
	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ybomb_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ybomb_w;
	  next[x-1][y] = Xbomb_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ybomb_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ybomb_e;
	  next[x+1][y] = Xbomb_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
  }
}

static void Lbomb_pause(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ybomb_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ybomb_s;
      next[x][y+1] = Xbomb_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybomb_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xbomb;
      next[x][y] = Xbomb;
      return;
  }
}

static void Lbomb_fall(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ybomb_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ybomb_s;
      next[x][y+1] = Xbomb_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ybomb_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Ybomb_blank;
      next[x][y] = Ztank;
      Lboom_tank_old(x, y);
      return;
  }
}

static void Lnut(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ynut_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ynut_s;
      next[x][y+1] = Xnut_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ynut_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xstone:
    case Xstone_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xnut:
    case Xnut_pause:
    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xballoon:
    case Xball_1:
    case Xball_2:
    case Xswitch:
    case Xbumper:
    case Ybumper:
    case Xacid_ne:
    case Xacid_nw:
    case Xslide_ns:
    case Xslide_ew:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ynut_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ynut_e;
	  next[x+1][y] = Xnut_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ynut_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ynut_w;
	  next[x-1][y] = Xnut_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
      else
      {
	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Ynut_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Ynut_w;
	  next[x-1][y] = Xnut_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Ynut_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Ynut_e;
	  next[x+1][y] = Xnut_pause;
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
  }
}

static void Lnut_pause(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Ynut_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ynut_s;
      next[x][y+1] = Xnut_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ynut_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xnut;
      next[x][y] = Xnut;
      return;
  }
}

static void Lnut_fall(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
      cave[x][y] = Ynut_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ynut_s;
      next[x][y+1] = Xnut_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ynut_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xnut;
      next[x][y] = Xnut;
      play_element_sound(x, y, SOUND_nut, Xnut);
      return;
  }
}

static void Lspring(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xplant:
    case Yplant:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yspring_s;
      next[x][y+1] = Xspring_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
    case Xemerald:
    case Xemerald_pause:
    case Xdiamond:
    case Xdiamond_pause:
    case Xstone:
    case Xstone_pause:
    case Xbomb:
    case Xbomb_pause:
    case Xnut:
    case Xnut_pause:
    case Xspring:
    case Xspring_pause:
    case Xspring_e:
    case Xspring_w:
    case Xkey_1:
    case Xkey_2:
    case Xkey_3:
    case Xkey_4:
    case Xkey_5:
    case Xkey_6:
    case Xkey_7:
    case Xkey_8:
    case Xballoon:
    case Xball_1:
    case Xball_2:
    case Xswitch:
    case Xbumper:
    case Ybumper:
    case Xacid_ne:
    case Xacid_nw:
    case Xslide_ns:
    case Xslide_ew:
    case Xroundwall_1:
    case Xroundwall_2:
    case Xroundwall_3:
    case Xroundwall_4:
      if (RANDOM(2))
      {
	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Yspring_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Yspring_e;
#ifdef SPRING_ROLL
	  next[x+1][y] = Xspring_e;
#else
	  next[x+1][y] = Xspring_pause;
#endif
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Yspring_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Yspring_w;
#ifdef SPRING_ROLL
	  next[x-1][y] = Xspring_w;
#else
	  next[x-1][y] = Xspring_pause;
#endif
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
      else
      {
	if (is_blank[cave[x-1][y]] && is_blank_or_acid[cave[x-1][y+1]])
	{
	  cave[x][y] = Yspring_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Yspring_w;
#ifdef SPRING_ROLL
	  next[x-1][y] = Xspring_w;
#else
	  next[x-1][y] = Xspring_pause;
#endif
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}

	if (is_blank[cave[x+1][y]] && is_blank_or_acid[cave[x+1][y+1]])
	{
	  cave[x][y] = Yspring_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Yspring_e;
#ifdef SPRING_ROLL
	  next[x+1][y] = Xspring_e;
#else
	  next[x+1][y] = Xspring_pause;
#endif
	  if (cave[x][y+1] == Xbumper)
	    cave[x][y+1] = Ybumper;
	  return;
	}
      }
  }
}

static void Lspring_pause(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yspring_s;
      next[x][y+1] = Xspring_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    default:
      cave[x][y] = Xspring;
      next[x][y] = Xspring;
      return;
  }
}

static void Lspring_e(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yspring_s;
      next[x][y+1] = Xspring_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xbumper:
      cave[x][y+1] = Ybumper;
  }

  switch (cave[x+1][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Yalien_nB:
    case Yalien_eB:
    case Yalien_sB:
    case Yalien_wB:
      cave[x][y] = Yspring_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Yspring_e;
      next[x+1][y] = Xspring_e;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_eB;
      next[x][y] = Xblank;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xalien:
    case Xalien_pause:
    case Yalien_n:
    case Yalien_e:
    case Yalien_s:
    case Yalien_w:
      cave[x][y] = Yspring_alien_eB;
      next[x][y] = Xblank;
      cave[x+1][y] = Yspring_alien_e;
      next[x+1][y] = Xspring_e;
      play_element_sound(x, y, SOUND_slurp, Xalien);
      score += lev.slurp_score;
      return;

    case Xbumper:
    case Ybumper:
      cave[x+1][y] = Ybumper;
      next[x][y] = Xspring_w;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;

    default:
      cave[x][y] = Xspring;
      next[x][y] = Xspring;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;
  }
}

static void Lspring_w(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yspring_s;
      next[x][y+1] = Xspring_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xbumper:
      cave[x][y+1] = Ybumper;
  }

  switch (cave[x-1][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Yalien_nB:
    case Yalien_eB:
    case Yalien_sB:
    case Yalien_wB:
      cave[x][y] = Yspring_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Yspring_w;
      next[x-1][y] = Xspring_w;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_wB;
      next[x][y] = Xblank;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xalien:
    case Xalien_pause:
    case Yalien_n:
    case Yalien_e:
    case Yalien_s:
    case Yalien_w:
      cave[x][y] = Yspring_alien_wB;
      next[x][y] = Xblank;
      cave[x-1][y] = Yspring_alien_w;
      next[x-1][y] = Xspring_w;
      play_element_sound(x, y, SOUND_slurp, Xalien);
      score += lev.slurp_score;
      return;

    case Xbumper:
    case Ybumper:
      cave[x-1][y] = Ybumper;
      next[x][y] = Xspring_e;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;

    default:
      cave[x][y] = Xspring;
      next[x][y] = Xspring;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;
  }
}

static void Lspring_fall(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yspring_s;
      next[x][y+1] = Xspring_fall;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xeater_n:
    case Xeater_e:
    case Xeater_s:
    case Xeater_w:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yeater_spring;
      next[x][y+1] = Zeater;
      Lboom_eater_old(x, y+1);
      score += lev.eater_score;
      return;

    case Xalien:
    case Xalien_pause:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Yalien_spring;
      next[x][y+1] = Ztank;
      Lboom_tank_old(x, y+1);
      score += lev.alien_score;
      return;

    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ybug_spring;
      next[x][y+1] = Zbug;
      Lboom_bug_old(x, y+1);
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
      cave[x][y] = Yspring_sB;
      next[x][y] = Xblank;
      cave[x][y+1] = Ytank_spring;
      next[x][y+1] = Ztank;
      Lboom_tank_old(x, y+1);
      score += lev.tank_score;
      return;

    case Xbomb:
    case Xbomb_pause:
      cave[x][y] = Xspring;
      next[x][y] = Xspring;
      cave[x][y+1] = Ybomb_blank;
      next[x][y+1] = Ztank;
      Lboom_tank_old(x, y+1);
      return;

    default:
      cave[x][y] = Xspring;
      next[x][y] = Xspring;
      play_element_sound(x, y, SOUND_spring, Xspring);
      return;
  }
}

static void Lpush_emerald_e(int x, int y)
{
  cave[x][y] = Yemerald_eB;
  next[x][y] = Xblank;

  switch (cave[x+1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xemerald);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x+1][y] = Yemerald_e;
  next[x+1][y] = Xemerald_pause;
}

static void Lpush_emerald_w(int x, int y)
{
  cave[x][y] = Yemerald_wB;
  next[x][y] = Xblank;

  switch (cave[x-1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xemerald);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x-1][y] = Yemerald_w;
  next[x-1][y] = Xemerald_pause;
}

static void Lpush_diamond_e(int x, int y)
{
  cave[x][y] = Ydiamond_eB;
  next[x][y] = Xblank;

  switch (cave[x+1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xdiamond);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x+1][y] = Ydiamond_e;
  next[x+1][y] = Xdiamond_pause;
}

static void Lpush_diamond_w(int x, int y)
{
  cave[x][y] = Ydiamond_wB;
  next[x][y] = Xblank;

  switch (cave[x-1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xdiamond);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x-1][y] = Ydiamond_w;
  next[x-1][y] = Xdiamond_pause;
}

static void Lpush_stone_e(int x, int y)
{
  cave[x][y] = Ystone_eB;
  next[x][y] = Xblank;

  switch (cave[x+1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xstone);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x+1][y] = Ystone_e;
  next[x+1][y] = Xstone_pause;
}

static void Lpush_stone_w(int x, int y)
{
  cave[x][y] = Ystone_wB;
  next[x][y] = Xblank;

  switch (cave[x-1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xstone);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x-1][y] = Ystone_w;
  next[x-1][y] = Xstone_pause;
}

static void Lpush_bomb_e(int x, int y)
{
  cave[x][y] = Ybomb_eB;
  next[x][y] = Xblank;

  switch (cave[x+1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xbomb);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x+1][y] = Ybomb_e;
  next[x+1][y] = Xbomb_pause;
}

static void Lpush_bomb_w(int x, int y)
{
  cave[x][y] = Ybomb_wB;
  next[x][y] = Xblank;

  switch (cave[x-1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xbomb);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x-1][y] = Ybomb_w;
  next[x-1][y] = Xbomb_pause;
}

static void Lpush_nut_e(int x, int y)
{
  cave[x][y] = Ynut_eB;
  next[x][y] = Xblank;

  switch (cave[x+1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xnut);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x+1][y] = Ynut_e;
  next[x+1][y] = Xnut_pause;
}

static void Lpush_nut_w(int x, int y)
{
  cave[x][y] = Ynut_wB;
  next[x][y] = Xblank;

  switch (cave[x-1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xnut);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x-1][y] = Ynut_w;
  next[x-1][y] = Xnut_pause;
}

static void Lpush_spring_e(int x, int y)
{
  cave[x][y] = Yspring_eB;
  next[x][y] = Xblank;

  switch (cave[x+1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xspring);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x+2][y-1] == Xblank)
	cave[x+2][y-1] = Xsplash_e;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x+1][y] = Yspring_e;
  next[x+1][y] = Xspring_e;
}

static void Lpush_spring_w(int x, int y)
{
  cave[x][y] = Yspring_wB;
  next[x][y] = Xblank;

  switch (cave[x-1][y])
  {
    case Zplayer:
      if (!game_em.use_old_push_elements)
	break;
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xboom_1:
      Lpush_element_old(x, y, Xspring);
      return;

#ifdef ACID_ROLL
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      if (game_em.use_old_push_into_acid)
	break;
      if (cave[x][y-1] == Xblank)
	cave[x][y-1] = Xsplash_e;
      if (cave[x-2][y-1] == Xblank)
	cave[x-2][y-1] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;
#endif
  }

  cave[x-1][y] = Yspring_w;
  next[x-1][y] = Xspring_w;
}

static void Ldynamite_1(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_1);
  next[x][y] = Xdynamite_2;
}

static void Ldynamite_2(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_2);
  next[x][y] = Xdynamite_3;
}

static void Ldynamite_3(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_3);
  next[x][y] = Xdynamite_4;
}

static void Ldynamite_4(int x, int y)
{
  play_element_sound(x, y, SOUND_tick, Xdynamite_4);
  next[x][y] = Zdynamite;

  Lboom_generic(x, y, Xblank, Xblank);
}

static void Lfake_door_1(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_1;
}

static void Lfake_door_2(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_2;
}

static void Lfake_door_3(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_3;
}

static void Lfake_door_4(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_4;
}

static void Lfake_door_5(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_5;
}

static void Lfake_door_6(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_6;
}

static void Lfake_door_7(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_7;
}

static void Lfake_door_8(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Xdoor_8;
}

static void Lballoon(int x, int y)
{
  if (lev.wind_cnt == 0)
    return;

  switch (lev.wind_direction)
  {
    case 0: /* north */
      switch (cave[x][y-1])
      {
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	  cave[x][y] = Yballoon_nB;
	  next[x][y] = Xblank;
	  cave[x][y-1] = Yballoon_n;
	  next[x][y-1] = Xballoon;
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yballoon_nB;
	  next[x][y] = Xblank;
	  if (cave[x+1][y-2] == Xblank)
	    cave[x+1][y-2] = Xsplash_e;
	  if (cave[x-1][y-2] == Xblank)
	    cave[x-1][y-2] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
      break;

    case 1: /* east */
      switch (cave[x+1][y])
      {
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	  cave[x][y] = Yballoon_eB;
	  next[x][y] = Xblank;
	  cave[x+1][y] = Yballoon_e;
	  next[x+1][y] = Xballoon;
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yballoon_eB;
	  next[x][y] = Xblank;
	  if (cave[x+2][y-1] == Xblank)
	    cave[x+2][y-1] = Xsplash_e;
	  if (cave[x][y-1] == Xblank)
	    cave[x][y-1] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
      break;

    case 2: /* south */
      switch (cave[x][y+1])
      {
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	  cave[x][y] = Yballoon_sB;
	  next[x][y] = Xblank;
	  cave[x][y+1] = Yballoon_s;
	  next[x][y+1] = Xballoon;
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yballoon_sB;
	  next[x][y] = Xblank;
	  if (cave[x+1][y] == Xblank)
	    cave[x+1][y] = Xsplash_e;
	  if (cave[x-1][y] == Xblank)
	    cave[x-1][y] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
	  return;
      }
      break;

    case 3: /* west */
      switch (cave[x-1][y])
      {
	case Xblank:
	case Xsplash_e:
	case Xsplash_w:
	case Xfake_acid_1:
	case Xfake_acid_2:
	case Xfake_acid_3:
	case Xfake_acid_4:
	case Xfake_acid_5:
	case Xfake_acid_6:
	case Xfake_acid_7:
	case Xfake_acid_8:
	  cave[x][y] = Yballoon_wB;
	  next[x][y] = Xblank;
	  cave[x-1][y] = Yballoon_w;
	  next[x-1][y] = Xballoon;
	  return;

	case Xacid_1:
	case Xacid_2:
	case Xacid_3:
	case Xacid_4:
	case Xacid_5:
	case Xacid_6:
	case Xacid_7:
	case Xacid_8:
	  cave[x][y] = Yballoon_wB;
	  next[x][y] = Xblank;
	  if (cave[x][y-1] == Xblank)
	    cave[x][y-1] = Xsplash_e;
	  if (cave[x-2][y-1] == Xblank)
	    cave[x-2][y-1] = Xsplash_w;
	  play_element_sound(x, y, SOUND_acid, Xacid_1);
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
	    is_blank[cave[x-1][y-1]])
	{
	  cave[x-1][y-1] = Yball_blank;
	  next[x-1][y-1] = lev.ball_array[lev.ball_pos][0];
	}
	break;

      case 1:
	if (lev.ball_array[lev.ball_pos][1] != Xblank &&
	    is_blank[cave[x][y-1]])
	{
	  cave[x][y-1] = Yball_blank;
	  next[x][y-1] = lev.ball_array[lev.ball_pos][1];
	}
	break;

      case 2:
	if (lev.ball_array[lev.ball_pos][2] != Xblank &&
	    is_blank[cave[x+1][y-1]])
	{
	  cave[x+1][y-1] = Yball_blank;
	  next[x+1][y-1] = lev.ball_array[lev.ball_pos][2];
	}
	break;

      case 3:
	if (lev.ball_array[lev.ball_pos][3] != Xblank &&
	    is_blank[cave[x-1][y]])
	{
	  cave[x-1][y] = Yball_blank;
	  next[x-1][y] = lev.ball_array[lev.ball_pos][3];
	}
	break;

      case 4:
	if (lev.ball_array[lev.ball_pos][4] != Xblank &&
	    is_blank[cave[x+1][y]])
	{
	  cave[x+1][y] = Yball_blank;
	  next[x+1][y] = lev.ball_array[lev.ball_pos][4];
	}
	break;

      case 5:
	if (lev.ball_array[lev.ball_pos][5] != Xblank &&
	    is_blank[cave[x-1][y+1]])
	{
	  cave[x-1][y+1] = Yball_blank;
	  next[x-1][y+1] = lev.ball_array[lev.ball_pos][5];
	}
	break;

      case 6:
	if (lev.ball_array[lev.ball_pos][6] != Xblank &&
	    is_blank[cave[x][y+1]])
	{
	  cave[x][y+1] = Yball_blank;
	  next[x][y+1] = lev.ball_array[lev.ball_pos][6];
	}
	break;

      case 7:
	if (lev.ball_array[lev.ball_pos][7] != Xblank &&
	    is_blank[cave[x+1][y+1]])
	{
	  cave[x+1][y+1] = Yball_blank;
	  next[x+1][y+1] = lev.ball_array[lev.ball_pos][7];
	}
	break;
    }
  }
  else
  {
    if (lev.ball_array[lev.ball_pos][0] != Xblank &&
	is_blank[cave[x-1][y-1]])
    {
      cave[x-1][y-1] = Yball_blank;
      next[x-1][y-1] = lev.ball_array[lev.ball_pos][0];
    }

    if (lev.ball_array[lev.ball_pos][1] != Xblank &&
	is_blank[cave[x][y-1]])
    {
      cave[x][y-1] = Yball_blank;
      next[x][y-1] = lev.ball_array[lev.ball_pos][1];
    }

    if (lev.ball_array[lev.ball_pos][2] != Xblank &&
	is_blank[cave[x+1][y-1]])
    {
      cave[x+1][y-1] = Yball_blank;
      next[x+1][y-1] = lev.ball_array[lev.ball_pos][2];
    }

    if (lev.ball_array[lev.ball_pos][3] != Xblank &&
	is_blank[cave[x-1][y]])
    {
      cave[x-1][y] = Yball_blank;
      next[x-1][y] = lev.ball_array[lev.ball_pos][3];
    }

    if (lev.ball_array[lev.ball_pos][4] != Xblank &&
	is_blank[cave[x+1][y]])
    {
      cave[x+1][y] = Yball_blank;
      next[x+1][y] = lev.ball_array[lev.ball_pos][4];
    }

    if (lev.ball_array[lev.ball_pos][5] != Xblank &&
	is_blank[cave[x-1][y+1]])
    {
      cave[x-1][y+1] = Yball_blank;
      next[x-1][y+1] = lev.ball_array[lev.ball_pos][5];
    }

    if (lev.ball_array[lev.ball_pos][6] != Xblank &&
	is_blank[cave[x][y+1]])
    {
      cave[x][y+1] = Yball_blank;
      next[x][y+1] = lev.ball_array[lev.ball_pos][6];
    }

    if (lev.ball_array[lev.ball_pos][7] != Xblank &&
	is_blank[cave[x+1][y+1]])
    {
      cave[x+1][y+1] = Yball_blank;
      next[x+1][y+1] = lev.ball_array[lev.ball_pos][7];
    }
  }

  lev.ball_pos = (lev.ball_pos + 1) % lev.num_ball_arrays;
}

static void Lball_1(int x, int y)
{
  if (!lev.ball_active)
    return;

  cave[x][y] = Yball_1;
  next[x][y] = Xball_2;
  if (lev.ball_cnt)
    return;

  Lball_common(x, y);
}

static void Lball_2(int x, int y)
{
  if (!lev.ball_active)
    return;

  cave[x][y] = Yball_2;
  next[x][y] = Xball_1;
  if (lev.ball_cnt)
    return;

  Lball_common(x, y);
}

static void Ldrip(int x, int y)
{
  next[x][y] = Xdrip_fall;
}

static void Ldrip_fall(int x, int y)
{
  int temp;

  switch (cave[x][y+1])
  {
    case Zplayer:
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xfake_acid_1_player:
    case Xfake_acid_2_player:
    case Xfake_acid_3_player:
    case Xfake_acid_4_player:
    case Xfake_acid_5_player:
    case Xfake_acid_6_player:
    case Xfake_acid_7_player:
    case Xfake_acid_8_player:
    case Xplant:
    case Yplant:
      cave[x][y] = Ydrip_1_sB;
      next[x][y] = Xdrip_stretchB;
      cave[x][y+1] = Ydrip_1_s;
      next[x][y+1] = Xdrip_stretch;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Ydrip_1_sB;
      next[x][y] = Xdrip_stretchB;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
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

      cave[x][y] = temp;
      next[x][y] = temp;
      play_element_sound(x, y, SOUND_drip, Xdrip_fall);
      return;
  }
}

static void Ldrip_stretch(int x, int y)
{
  cave[x][y] = Ydrip_2_s;
  next[x][y] = Xdrip_fall;
}

static void Ldrip_stretchB(int x, int y)
{
  cave[x][y] = Ydrip_2_sB;
  next[x][y] = Xblank;
}

static void Lwonderwall(int x, int y)
{
  if (lev.wonderwall_time > 0 && lev.wonderwall_active)
  {
    cave[x][y] = Ywonderwall;
    play_element_sound(x, y, SOUND_wonder, Xwonderwall);
  }
}

static void Lwheel(int x, int y)
{
  if (lev.wheel_cnt && x == lev.wheel_x && y == lev.wheel_y)
    cave[x][y] = Ywheel;
}

static void Lswitch(int x, int y)
{
  if (lev.ball_active)
    cave[x][y] = Yswitch;
}

static void Lfake_blank(int x, int y)
{
  if (lev.lenses_cnt)
    cave[x][y] = Yfake_blank;
}

static void Lfake_grass(int x, int y)
{
  if (lev.magnify_cnt)
    cave[x][y] = Yfake_grass;
}

static void Lfake_amoeba(int x, int y)
{
  if (lev.lenses_cnt)
    cave[x][y] = Yfake_amoeba;
}

static void Lsand_stone(int x, int y)
{
  switch (cave[x][y+1])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
      cave[x][y] = Xsand_stonesand_quickout_1;
      next[x][y] = Xsand_stonesand_quickout_2;
      cave[x][y+1] = Xsand_stoneout_1;
      next[x][y+1] = Xsand_stoneout_2;
      return;

    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      cave[x][y] = Xsand_stonesand_quickout_1;
      next[x][y] = Xsand_stonesand_quickout_2;
      if (cave[x+1][y] == Xblank)
	cave[x+1][y] = Xsplash_e;
      if (cave[x-1][y] == Xblank)
	cave[x-1][y] = Xsplash_w;
      play_element_sound(x, y, SOUND_acid, Xacid_1);
      return;

    case Xsand:
      cave[x][y] = Xsand_stonesand_1;
      next[x][y] = Xsand_stonesand_2;
      cave[x][y+1] = Xsand_sandstone_1;
      next[x][y+1] = Xsand_sandstone_2;
      return;
  }
}

static void Lsand_stonein_1(int x, int y)
{
  next[x][y] = Xsand_stonein_2;
}

static void Lsand_stonein_2(int x, int y)
{
  next[x][y] = Xsand_stonein_3;
}

static void Lsand_stonein_3(int x, int y)
{
  next[x][y] = Xsand_stonein_4;
}

static void Lsand_stonein_4(int x, int y)
{
  next[x][y] = Xblank;
}

static void Lsand_sandstone_1(int x, int y)
{
  next[x][y] = Xsand_sandstone_2;
}

static void Lsand_sandstone_2(int x, int y)
{
  next[x][y] = Xsand_sandstone_3;
}

static void Lsand_sandstone_3(int x, int y)
{
  next[x][y] = Xsand_sandstone_4;
}

static void Lsand_sandstone_4(int x, int y)
{
  next[x][y] = Xsand_stone;
}

static void Lsand_stonesand_1(int x, int y)
{
  next[x][y] = Xsand_stonesand_2;
}

static void Lsand_stonesand_2(int x, int y)
{
  next[x][y] = Xsand_stonesand_3;
}

static void Lsand_stonesand_3(int x, int y)
{
  next[x][y] = Xsand_stonesand_4;
}

static void Lsand_stonesand_4(int x, int y)
{
  next[x][y] = Xsand;
}

static void Lsand_stoneout_1(int x, int y)
{
  next[x][y] = Xsand_stoneout_2;
}

static void Lsand_stoneout_2(int x, int y)
{
  next[x][y] = Xstone_fall;
}

static void Lsand_stonesand_quickout_1(int x, int y)
{
  next[x][y] = Xsand_stonesand_quickout_2;
}

static void Lsand_stonesand_quickout_2(int x, int y)
{
  next[x][y] = Xsand;
}

static void Lslide_ns(int x, int y)
{
  if (is_blank[cave[x][y-1]])
  {
    cave[x][y-1] = Yslide_ns_blank;
    next[x][y-1] = Xslide_ns;
    play_element_sound(x, y, SOUND_slide, Xslide_ns);
  }

  if (is_blank[cave[x][y+1]])
  {
    cave[x][y+1] = Yslide_ns_blank;
    next[x][y+1] = Xslide_ns;
    play_element_sound(x, y, SOUND_slide, Xslide_ns);
  }
}

static void Lslide_ew(int x, int y)
{
  if (is_blank[cave[x+1][y]])
  {
    cave[x+1][y] = Yslide_ew_blank;
    next[x+1][y] = Xslide_ew;
    play_element_sound(x, y, SOUND_slide, Xslide_ew);
  }

  if (is_blank[cave[x-1][y]])
  {
    cave[x-1][y] = Yslide_ew_blank;
    next[x-1][y] = Xslide_ew;
    play_element_sound(x, y, SOUND_slide, Xslide_ew);
  }
}

static void Lexit(int x, int y)
{
  if (lev.gems_needed > 0)
    return;

  switch (RANDOM(64) / 21)
  {
    case 0:
      cave[x][y] = Xexit_1;
      next[x][y] = Xexit_2;
      break;

    case 1:
      cave[x][y] = Xexit_2;
      next[x][y] = Xexit_3;
      break;

    default:
      cave[x][y] = Xexit_3;
      next[x][y] = Xexit_1;
      break;
  }

  play_element_sound(x, y, SOUND_exit_open, Xexit);
}

static void Lexit_1(int x, int y)
{
  next[x][y] = Xexit_2;
}

static void Lexit_2(int x, int y)
{
  next[x][y] = Xexit_3;
}

static void Lexit_3(int x, int y)
{
  next[x][y] = Xexit_1;
}

static void Lpause(int x, int y)
{
  next[x][y] = Xblank;
}

static void Lamoeba(int x, int y)
{
  switch (cave[x][y])
  {
    case Xblank:
    case Xsplash_e:
    case Xsplash_w:
    case Xfake_acid_1:
    case Xfake_acid_2:
    case Xfake_acid_3:
    case Xfake_acid_4:
    case Xfake_acid_5:
    case Xfake_acid_6:
    case Xfake_acid_7:
    case Xfake_acid_8:
    case Xplant:
    case Yplant:
    case Xgrass:
    case Xdirt:
    case Xsand:
      if (is_amoeba[cave[x][y-1]] ||
	  is_amoeba[cave[x+1][y]] ||
	  is_amoeba[cave[x][y+1]] ||
	  is_amoeba[cave[x-1][y]])
	cave[x][y] = Xdrip;
  }
}

static void Lboom_one(int x, int y, boolean by_dynamite)
{
  switch (cave[x][y])
  {
    case Zborder:
    case Zbug:
    case Ztank:
    case Zeater:
    case Zdynamite:
    case Zboom:
    case Xchain:
    case Xboom_bug:
    case Xboom_tank:
    case Xboom_android:
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
    case Xplant:
    case Yplant:
    case Xdoor_1:
    case Xdoor_2:
    case Xdoor_3:
    case Xdoor_4:
    case Xdoor_5:
    case Xdoor_6:
    case Xdoor_7:
    case Xdoor_8:
    case Xfake_door_1:
    case Xfake_door_2:
    case Xfake_door_3:
    case Xfake_door_4:
    case Xfake_door_5:
    case Xfake_door_6:
    case Xfake_door_7:
    case Xfake_door_8:
    case Xacid_ne:
    case Xacid_nw:
    case Xacid_s:
    case Xacid_se:
    case Xacid_sw:
    case Xsteel_1:
    case Xsteel_2:
    case Xsteel_3:
    case Xsteel_4:
      return;

    case Xandroid:
    case Xandroid_1_n:
    case Xandroid_2_n:
    case Xandroid_1_e:
    case Xandroid_2_e:
    case Xandroid_1_s:
    case Xandroid_2_s:
    case Xandroid_1_w:
    case Xandroid_2_w:
      if (by_dynamite)
	cave[x][y] = Xboom_android;
      return;

    case Xbug_1_n:
    case Xbug_1_e:
    case Xbug_1_s:
    case Xbug_1_w:
    case Xbug_2_n:
    case Xbug_2_e:
    case Xbug_2_s:
    case Xbug_2_w:
      cave[x][y] = Xboom_bug;
      Lboom_bug_new(x, y, TRUE);
      return;

    case Xbomb:
    case Xbomb_pause:
    case Xbomb_fall:
      cave[x][y] = Xboom_tank;
      Lboom_tank_new(x, y, TRUE);
      return;

    default:
      cave[x][y] = Xboom_1;
      return;
  }
}

static void Lboom_nine(int x, int y, boolean by_dynamite)
{
  Lboom_one(x,   y-1, by_dynamite);
  Lboom_one(x-1, y,   by_dynamite);
  Lboom_one(x+1, y,   by_dynamite);
  Lboom_one(x,   y+1, by_dynamite);
  Lboom_one(x-1, y-1, by_dynamite);
  Lboom_one(x+1, y-1, by_dynamite);
  Lboom_one(x-1, y+1, by_dynamite);
  Lboom_one(x+1, y+1, by_dynamite);

  cave[x][y] = Xboom_1;
}

static void Lexplode(int x, int y)
{
  switch (cave[x][y])
  {
    case Zbug:
      Lboom_bug_new(x, y, FALSE);
      Lboom_nine(x, y, FALSE);
      break;

    case Ztank:
      Lboom_tank_new(x, y, FALSE);
      Lboom_nine(x, y, FALSE);
      break;

    case Zeater:
      Lboom_eater_new(x, y, FALSE);
      Lboom_nine(x, y, FALSE);
      break;

    case Zdynamite:
      Lboom_nine(x, y, TRUE);
      break;

    case Zboom:
      Lboom_nine(x, y, FALSE);
      break;
  }
}

static void Lboom_1(int x, int y)
{
  next[x][y] = Xboom_2;
#if !PLAY_ELEMENT_SOUND
  if (x != lev.exit_x && y != lev.exit_y)
    play_sound(x, y, SOUND_boom);
  else
    lev.exit_x = lev.exit_y = -1;
#endif
}

static void Lboom_2(int x, int y)
{
  next[x][y] = boom[x][y];
}

static void Lboom_android(int x, int y)
{
#if PLAY_ELEMENT_SOUND
  play_element_sound(x, y, SOUND_boom, Xandroid);
#endif

  Lboom_1(x, y);
}

static void Lchain(int x, int y)
{
  next[x][y] = Zboom;
}

static void handle_tile(int x, int y)
{
  switch (cave[x][y])
  {
    case Xacid_1:		Lacid_1(x, y);			break;
    case Xacid_2:		Lacid_2(x, y);			break;
    case Xacid_3:		Lacid_3(x, y);			break;
    case Xacid_4:		Lacid_4(x, y);			break;
    case Xacid_5:		Lacid_5(x, y);			break;
    case Xacid_6:		Lacid_6(x, y);			break;
    case Xacid_7:		Lacid_7(x, y);			break;
    case Xacid_8:		Lacid_8(x, y);			break;

    case Xfake_acid_1:		Lfake_acid_1(x, y);		break;
    case Xfake_acid_2:		Lfake_acid_2(x, y);		break;
    case Xfake_acid_3:		Lfake_acid_3(x, y);		break;
    case Xfake_acid_4:		Lfake_acid_4(x, y);		break;
    case Xfake_acid_5:		Lfake_acid_5(x, y);		break;
    case Xfake_acid_6:		Lfake_acid_6(x, y);		break;
    case Xfake_acid_7:		Lfake_acid_7(x, y);		break;
    case Xfake_acid_8:		Lfake_acid_8(x, y);		break;

    case Xandroid:		Landroid(x, y);			break;
    case Xandroid_1_n:		Landroid_1_n(x, y);		break;
    case Xandroid_2_n:		Landroid_2_n(x, y);		break;
    case Xandroid_1_e:		Landroid_1_e(x, y);		break;
    case Xandroid_2_e:		Landroid_2_e(x, y);		break;
    case Xandroid_1_s:		Landroid_1_s(x, y);		break;
    case Xandroid_2_s:		Landroid_2_s(x, y);		break;
    case Xandroid_1_w:		Landroid_1_w(x, y);		break;
    case Xandroid_2_w:		Landroid_2_w(x, y);		break;

    case Xeater_n:		Leater_n(x, y);			break;
    case Xeater_e:		Leater_e(x, y);			break;
    case Xeater_s:		Leater_s(x, y);			break;
    case Xeater_w:		Leater_w(x, y);			break;

    case Xalien:		Lalien(x, y);			break;
    case Xalien_pause:		Lalien_pause(x, y);		break;

    case Xbug_1_n:		Lbug_1_n(x, y);			break;
    case Xbug_2_n:		Lbug_2_n(x, y);			break;
    case Xbug_1_e:		Lbug_1_e(x, y);			break;
    case Xbug_2_e:		Lbug_2_e(x, y);			break;
    case Xbug_1_s:		Lbug_1_s(x, y);			break;
    case Xbug_2_s:		Lbug_2_s(x, y);			break;
    case Xbug_1_w:		Lbug_1_w(x, y);			break;
    case Xbug_2_w:		Lbug_2_w(x, y);			break;

    case Xtank_1_n:		Ltank_1_n(x, y);		break;
    case Xtank_2_n:		Ltank_2_n(x, y);		break;
    case Xtank_1_e:		Ltank_1_e(x, y);		break;
    case Xtank_2_e:		Ltank_2_e(x, y);		break;
    case Xtank_1_s:		Ltank_1_s(x, y);		break;
    case Xtank_2_s:		Ltank_2_s(x, y);		break;
    case Xtank_1_w:		Ltank_1_w(x, y);		break;
    case Xtank_2_w:		Ltank_2_w(x, y);		break;

    case Xemerald:		Lemerald(x, y);			break;
    case Xemerald_pause:	Lemerald_pause(x, y);		break;
    case Xemerald_fall:		Lemerald_fall(x, y);		break;

    case Xdiamond:		Ldiamond(x, y);			break;
    case Xdiamond_pause:	Ldiamond_pause(x, y);		break;
    case Xdiamond_fall:		Ldiamond_fall(x, y);		break;

    case Xstone:		Lstone(x, y);			break;
    case Xstone_pause:		Lstone_pause(x, y);		break;
    case Xstone_fall:		Lstone_fall(x, y);		break;

    case Xbomb:			Lbomb(x, y);			break;
    case Xbomb_pause:		Lbomb_pause(x, y);		break;
    case Xbomb_fall:		Lbomb_fall(x, y);		break;

    case Xnut:			Lnut(x, y);			break;
    case Xnut_pause:		Lnut_pause(x, y);		break;
    case Xnut_fall:		Lnut_fall(x, y);		break;

    case Xspring:		Lspring(x, y);			break;
    case Xspring_pause:		Lspring_pause(x, y);		break;
    case Xspring_e:		Lspring_e(x, y);		break;
    case Xspring_w:		Lspring_w(x, y);		break;
    case Xspring_fall:		Lspring_fall(x, y);		break;

    case Xpush_emerald_e:	Lpush_emerald_e(x, y);		break;
    case Xpush_emerald_w:	Lpush_emerald_w(x, y);		break;
    case Xpush_diamond_e:	Lpush_diamond_e(x, y);		break;
    case Xpush_diamond_w:	Lpush_diamond_w(x, y);		break;
    case Xpush_stone_e:		Lpush_stone_e(x, y);		break;
    case Xpush_stone_w:		Lpush_stone_w(x, y);		break;
    case Xpush_bomb_e:		Lpush_bomb_e(x, y);		break;
    case Xpush_bomb_w:		Lpush_bomb_w(x, y);		break;
    case Xpush_nut_e:		Lpush_nut_e(x, y);		break;
    case Xpush_nut_w:		Lpush_nut_w(x, y);		break;
    case Xpush_spring_e:	Lpush_spring_e(x, y);		break;
    case Xpush_spring_w:	Lpush_spring_w(x, y);		break;

    case Xdynamite_1:		Ldynamite_1(x, y);		break;
    case Xdynamite_2:		Ldynamite_2(x, y);		break;
    case Xdynamite_3:		Ldynamite_3(x, y);		break;
    case Xdynamite_4:		Ldynamite_4(x, y);		break;

    case Xfake_door_1:		Lfake_door_1(x, y);		break;
    case Xfake_door_2:		Lfake_door_2(x, y);		break;
    case Xfake_door_3:		Lfake_door_3(x, y);		break;
    case Xfake_door_4:		Lfake_door_4(x, y);		break;
    case Xfake_door_5:		Lfake_door_5(x, y);		break;
    case Xfake_door_6:		Lfake_door_6(x, y);		break;
    case Xfake_door_7:		Lfake_door_7(x, y);		break;
    case Xfake_door_8:		Lfake_door_8(x, y);		break;

    case Xballoon:		Lballoon(x, y);			break;

    case Xball_1:		Lball_1(x, y);			break;
    case Xball_2:		Lball_2(x, y);			break;

    case Xdrip:			Ldrip(x, y);			break;
    case Xdrip_fall:		Ldrip_fall(x, y);		break;
    case Xdrip_stretch:		Ldrip_stretch(x, y);		break;
    case Xdrip_stretchB:	Ldrip_stretchB(x, y);		break;

    case Xwonderwall:		Lwonderwall(x, y);		break;

    case Xwheel:		Lwheel(x, y);			break;

    case Xswitch:		Lswitch(x, y);			break;

    case Xfake_blank:		Lfake_blank(x, y);		break;
    case Xfake_grass:		Lfake_grass(x, y);		break;
    case Xfake_amoeba:		Lfake_amoeba(x, y);		break;

    case Xsand_stone:		Lsand_stone(x, y);		break;
    case Xsand_stonein_1:	Lsand_stonein_1(x, y);		break;
    case Xsand_stonein_2:	Lsand_stonein_2(x, y);		break;
    case Xsand_stonein_3:	Lsand_stonein_3(x, y);		break;
    case Xsand_stonein_4:	Lsand_stonein_4(x, y);		break;
    case Xsand_sandstone_1:	Lsand_sandstone_1(x, y);	break;
    case Xsand_sandstone_2:	Lsand_sandstone_2(x, y);	break;
    case Xsand_sandstone_3:	Lsand_sandstone_3(x, y);	break;
    case Xsand_sandstone_4:	Lsand_sandstone_4(x, y);	break;
    case Xsand_stonesand_1:	Lsand_stonesand_1(x, y);	break;
    case Xsand_stonesand_2:	Lsand_stonesand_2(x, y);	break;
    case Xsand_stonesand_3:	Lsand_stonesand_3(x, y);	break;
    case Xsand_stonesand_4:	Lsand_stonesand_4(x, y);	break;
    case Xsand_stoneout_1:	Lsand_stoneout_1(x, y);		break;
    case Xsand_stoneout_2:	Lsand_stoneout_2(x, y);		break;
    case Xsand_stonesand_quickout_1: Lsand_stonesand_quickout_1(x, y); break;
    case Xsand_stonesand_quickout_2: Lsand_stonesand_quickout_2(x, y); break;

    case Xslide_ns:		Lslide_ns(x, y);		break;
    case Xslide_ew:		Lslide_ew(x, y);		break;

    case Xexit:			Lexit(x, y);			break;
    case Xexit_1:		Lexit_1(x, y);			break;
    case Xexit_2:		Lexit_2(x, y);			break;
    case Xexit_3:		Lexit_3(x, y);			break;

    case Xpause:		Lpause(x, y);			break;

    case Xchain:		Lchain(x, y);			break;
    case Xboom_bug:		Lboom_bug(x, y);		break;
    case Xboom_tank:		Lboom_tank(x, y);		break;
    case Xboom_android:		Lboom_android(x, y);		break;
    case Xboom_1:		Lboom_1(x, y);			break;
    case Xboom_2:		Lboom_2(x, y);			break;
  }
}

boolean logic_check_wrap(void)
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (!ply[i].alive)
      continue;

    /* check for wrap-around movement */
    if (ply[i].x < lev.left ||
	ply[i].x > lev.right - 1)
      return TRUE;
  }

  return FALSE;
}

void logic_move(void)
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (!ply[i].alive)
      continue;

    /* check for wrap-around movement */
    if (ply[i].x < lev.left ||
	ply[i].x > lev.right - 1)
    {
      int direction = (ply[i].x < lev.left ? -1 : 1);

      ply[i].x += -direction * lev.width;

      if (!lev.infinite_true)
	ply[i].y += direction;

      game.centered_player_nr_next = i;
      game.set_centered_player = TRUE;
      game.set_centered_player_wrap = TRUE;
    }

    ply[i].prev_x = ply[i].x;
    ply[i].prev_y = ply[i].y;
    ply[i].anim = PLY_still;
  }
}

static void logic_players(void)
{
  int start_check_nr;
  int i;

  cave = lev.cave;
  next = lev.next;
  boom = lev.boom;

  game_em.any_player_moving = FALSE;
  game_em.any_player_snapping = FALSE;

  /* must test for death and actually kill separately */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    boolean ply_kill = player_killed(&ply[i]);

    if (ply[i].alive && ply_kill)
      kill_player(&ply[i]);
  }

  logic_move();

  start_check_nr = ((game_em.random & 128 ? 0 : 1) * 2 +
		    (game_em.random & 256 ? 0 : 1));

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int check_nr = (start_check_nr + i) % MAX_PLAYERS;

    if (ply[check_nr].alive)
      check_player(&ply[check_nr]);
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (!ply[i].alive)
      continue;

    if (is_player[cave[ply[i].prev_x][ply[i].prev_y]])
    {
      int element = cave[ply[i].prev_x][ply[i].prev_y];

      cave[ply[i].prev_x][ply[i].prev_y] = remove_player[element];
      next[ply[i].prev_x][ply[i].prev_y] = remove_player[element];
    }

    if (is_blank[cave[ply[i].x][ply[i].y]])
    {
      int element = cave[ply[i].x][ply[i].y];

      cave[ply[i].x][ply[i].y] = add_player[element];
      next[ply[i].x][ply[i].y] = add_player[element];
    }
  }

  /* check for wheel at wrap-around position */
  if (lev.wheel_x < lev.left ||
      lev.wheel_x > lev.right - 1)
  {
    int direction = (lev.wheel_x < lev.left ? -1 : 1);

    lev.wheel_x += -direction * lev.width;

    if (!lev.infinite_true)
      lev.wheel_y += direction;
  }
}

static void logic_objects(void)
{
  int x, y;

  cave = lev.cave;
  next = lev.next;
  boom = lev.boom;

  seed = game_em.random;
  score = 0;

  for (y = lev.top; y < lev.bottom; y++)
    for (x = lev.left; x < lev.right; x++)
      handle_tile(x, y);

  if (ply[0].alive || ply[1].alive || ply[2].alive || ply[3].alive)
    lev.score += score;		/* only add a score if someone is alive */
  else
    game_em.game_over = TRUE;

  game_em.random = seed;

  /* triple buffering */
  void *temp = lev.cave;
  lev.cave = lev.next;
  lev.next = lev.draw;
  lev.draw = temp;
}

static void logic_globals(void)
{
  int x;
  int y;
  int count;
#ifdef RANDOM_BUG
  uint64_t random;
#else
  uint32_t random;
#endif

  cave = lev.cave;
  next = lev.next;
  boom = lev.boom;

  /* update variables */

  if (lev.score > 9999)
    lev.score = 9999;

  if (lev.android_move_cnt-- == 0)
    lev.android_move_cnt = lev.android_move_time;
  if (lev.android_clone_cnt-- == 0)
    lev.android_clone_cnt = lev.android_clone_time;
  if (lev.ball_active)
    if (lev.ball_cnt-- == 0)
      lev.ball_cnt = lev.ball_time;
  if (lev.lenses_cnt)
    lev.lenses_cnt--;
  if (lev.magnify_cnt)
    lev.magnify_cnt--;
  if (lev.wheel_cnt)
    lev.wheel_cnt--;
  if (lev.wind_cnt)
    lev.wind_cnt--;
  if (lev.wonderwall_time > 0 && lev.wonderwall_active)
    lev.wonderwall_time--;

  if (lev.wheel_cnt)
    play_element_sound(lev.wheel_x, lev.wheel_y, SOUND_wheel, Xwheel);

  /* grow amoeba */

  random = game_em.random;

  for (count = lev.amoeba_time; count--;)
  {
    x = lev.left - 1 + (random >> 10) % (CAVE_WIDTH  + 2);
    y = lev.top  - 1 + (random >> 20) % (CAVE_HEIGHT + 2);

    if (x >= lev.left && x < lev.right &&
	y >= lev.top  && y < lev.bottom)
      Lamoeba(x, y);

    random = random * 129 + 1;

#ifdef RANDOM_BUG
    if (!game_em.use_random_bug)
      random = (uint32_t)random;
#endif
  }

  game_em.random = random;

  /* handle explosions */

  for (y = lev.top; y < lev.bottom; y++)
    for (x = lev.left; x < lev.right; x++)
      Lexplode(x, y);

  /* triple buffering */

  for (y = lev.top; y < lev.bottom; y++)
    for (x = lev.left; x < lev.right; x++)
      next[x][y] = cave[x][y];
}

void logic_init(void)
{
  int splash_is_blank = !game_em.use_old_android;

  is_android_blank[Xsplash_e] = splash_is_blank;
  is_android_blank[Xsplash_w] = splash_is_blank;
}

void logic(void)
{
  if (frame == 0)
  {
    logic_players();
    logic_objects();
  }

  if (frame == 1)
  {
    logic_globals();
  }
}
