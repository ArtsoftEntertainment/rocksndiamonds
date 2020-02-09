/* 2007-04-04 08:33:37
 *
 * convert intermediate cave format to internal cave structure for
 * use in logic(). initializes entire internal structure.
 */

#include "main_em.h"


void prepare_em_level(void)
{
  int i, x, y;
  int players_left;
  boolean team_mode;

  /* reset all runtime variables to their initial values */

  game_init_cave_buffers();

  lev.left = CAVE_BUFFER_XOFFSET;
  lev.top  = CAVE_BUFFER_YOFFSET;
  lev.right = lev.left + lev.width;
  lev.bottom = lev.top + lev.height;

  /* add linked cave buffer columns for wrap-around movement */
  for (x = 0; x < lev.left; x++)
  {
    lev.cavecol[x] = lev.cavecol[lev.width + x];
    lev.nextcol[x] = lev.nextcol[lev.width + x];
    lev.drawcol[x] = lev.drawcol[lev.width + x];
    lev.boomcol[x] = lev.boomcol[lev.width + x];

    lev.cavecol[lev.right + x] = lev.cavecol[lev.left + x];
    lev.nextcol[lev.right + x] = lev.nextcol[lev.left + x];
    lev.drawcol[lev.right + x] = lev.drawcol[lev.left + x];
    lev.boomcol[lev.right + x] = lev.boomcol[lev.left + x];
  }

  for (x = 0; x < lev.width; x++)
    for (y = 0; y < lev.height; y++)
      lev.cave[lev.left + x][lev.top + y] = native_em_level.cave[x][y];

  for (x = lev.left; x < lev.right; x++)
    for (y = lev.top; y < lev.bottom; y++)
      lev.next[x][y] = lev.draw[x][y] = lev.cave[x][y];

  lev.time_initial = lev.time_seconds;
  lev.time = lev.time_initial;

  lev.required = lev.required_initial;
  lev.score = 0;

  lev.android_move_cnt  = lev.android_move_time;
  lev.android_clone_cnt = lev.android_clone_time;

  lev.ball_pos = 0;
  lev.ball_state = lev.ball_state_initial;
  lev.ball_cnt = lev.ball_time;

  lev.eater_pos = 0;
  lev.shine_cnt = 0;

  lev.lenses_cnt = lev.lenses_cnt_initial;
  lev.magnify_cnt = lev.magnify_cnt_initial;

  lev.wheel_cnt = lev.wheel_cnt_initial;
  lev.wheel_x   = lev.wheel_x_initial;
  lev.wheel_y   = lev.wheel_y_initial;

  lev.wind_direction = lev.wind_direction_initial;
  lev.wind_cnt       = lev.wind_cnt_initial;

  lev.wonderwall_state = lev.wonderwall_state_initial;
  lev.wonderwall_time  = lev.wonderwall_time_initial;

  lev.killed_out_of_time = FALSE;

  if (lev.android_eater)
  {
    lev.android_array[Xeater_n]		= Xeater_n;
    lev.android_array[Yeater_nB]	= Xeater_n;

    lev.android_array[Xeater_e]		= Xeater_e;
    lev.android_array[Yeater_eB]	= Xeater_e;

    lev.android_array[Xeater_s]		= Xeater_s;
    lev.android_array[Yeater_sB]	= Xeater_s;

    lev.android_array[Xeater_w]		= Xeater_w;
    lev.android_array[Yeater_wB]	= Xeater_w;
  }

  if (lev.android_alien)
  {
    lev.android_array[Xalien]		= Xalien;
    lev.android_array[Xalien_pause]	= Xalien;
    lev.android_array[Yalien_nB]	= Xalien;
    lev.android_array[Yalien_eB]	= Xalien;
    lev.android_array[Yalien_sB]	= Xalien;
    lev.android_array[Yalien_wB]	= Xalien;
  }

  if (lev.android_bug)
  {
    lev.android_array[Xbug_1_n]		= Xbug_2_n;
    lev.android_array[Xbug_2_n]		= Xbug_2_n;
    lev.android_array[Ybug_nB]		= Xbug_2_n;
    lev.android_array[Ybug_n_e]		= Xbug_2_n;
    lev.android_array[Ybug_n_w]		= Xbug_2_n;

    lev.android_array[Xbug_1_e]		= Xbug_2_e;
    lev.android_array[Xbug_2_e]		= Xbug_2_e;
    lev.android_array[Ybug_eB]		= Xbug_2_e;
    lev.android_array[Ybug_e_s]		= Xbug_2_e;
    lev.android_array[Ybug_e_n]		= Xbug_2_e;

    lev.android_array[Xbug_1_s]		= Xbug_2_s;
    lev.android_array[Xbug_2_s]		= Xbug_2_s;
    lev.android_array[Ybug_sB]		= Xbug_2_s;
    lev.android_array[Ybug_s_w]		= Xbug_2_s;
    lev.android_array[Ybug_s_e]		= Xbug_2_s;

    lev.android_array[Xbug_1_w]		= Xbug_2_w;
    lev.android_array[Xbug_2_w]		= Xbug_2_w;
    lev.android_array[Ybug_wB]		= Xbug_2_w;
    lev.android_array[Ybug_w_n]		= Xbug_2_w;
    lev.android_array[Ybug_w_s]		= Xbug_2_w;
  }

  if (lev.android_tank)
  {
    lev.android_array[Xtank_1_n]	= Xtank_1_n;
    lev.android_array[Xtank_2_n]	= Xtank_1_n;
    lev.android_array[Ytank_nB]		= Xtank_1_n;
    lev.android_array[Ytank_n_e]	= Xtank_1_n;
    lev.android_array[Ytank_n_w]	= Xtank_1_n;

    lev.android_array[Xtank_1_e]	= Xtank_1_e;
    lev.android_array[Xtank_2_e]	= Xtank_1_e;
    lev.android_array[Ytank_eB]		= Xtank_1_e;
    lev.android_array[Ytank_e_s]	= Xtank_1_e;
    lev.android_array[Ytank_e_n]	= Xtank_1_e;

    lev.android_array[Xtank_1_s]	= Xtank_1_s;
    lev.android_array[Xtank_2_s]	= Xtank_1_s;
    lev.android_array[Ytank_sB]		= Xtank_1_s;
    lev.android_array[Ytank_s_w]	= Xtank_1_s;
    lev.android_array[Ytank_s_e]	= Xtank_1_s;

    lev.android_array[Xtank_1_w]	= Xtank_1_w;
    lev.android_array[Xtank_2_w]	= Xtank_1_w;
    lev.android_array[Ytank_wB]		= Xtank_1_w;
    lev.android_array[Ytank_w_n]	= Xtank_1_w;
    lev.android_array[Ytank_w_s]	= Xtank_1_w;
  }

  if (lev.android_emerald)
  {
    lev.android_array[Xemerald]		= Xemerald;
    lev.android_array[Xemerald_pause]	= Xemerald;
    lev.android_array[Xemerald_fall]	= Xemerald;
    lev.android_array[Yemerald_sB]	= Xemerald;
    lev.android_array[Yemerald_eB]	= Xemerald;
    lev.android_array[Yemerald_wB]	= Xemerald;
  }

  if (lev.android_diamond)
  {
    lev.android_array[Xdiamond]		= Xdiamond;
    lev.android_array[Xdiamond_pause]	= Xdiamond;
    lev.android_array[Xdiamond_fall]	= Xdiamond;
    lev.android_array[Ydiamond_sB]	= Xdiamond;
    lev.android_array[Ydiamond_eB]	= Xdiamond;
    lev.android_array[Ydiamond_wB]	= Xdiamond;
  }

  if (lev.android_stone)
  {
    lev.android_array[Xstone]		= Xstone;
    lev.android_array[Xstone_pause]	= Xstone;
    lev.android_array[Xstone_fall]	= Xstone;
    lev.android_array[Ystone_sB]	= Xstone;
    lev.android_array[Ystone_eB]	= Xstone;
    lev.android_array[Ystone_wB]	= Xstone;
  }

  if (lev.android_bomb)
  {
    lev.android_array[Xbomb]		= Xbomb;
    lev.android_array[Xbomb_pause]	= Xbomb;
    lev.android_array[Xbomb_fall]	= Xbomb;
    lev.android_array[Ybomb_sB]		= Xbomb;
    lev.android_array[Ybomb_eB]		= Xbomb;
    lev.android_array[Ybomb_wB]		= Xbomb;
  }

  if (lev.android_nut)
  {
    lev.android_array[Xnut]		= Xnut;
    lev.android_array[Xnut_pause]	= Xnut;
    lev.android_array[Xnut_fall]	= Xnut;
    lev.android_array[Ynut_sB]		= Xnut;
    lev.android_array[Ynut_eB]		= Xnut;
    lev.android_array[Ynut_wB]		= Xnut;
  }

  if (lev.android_spring)
  {
    lev.android_array[Xspring]		= Xspring;
    lev.android_array[Xspring_pause]	= Xspring;
    lev.android_array[Xspring_fall]	= Xspring;
    lev.android_array[Xspring_e]	= Xspring;
    lev.android_array[Xspring_w]	= Xspring;
    lev.android_array[Yspring_sB]	= Xspring;
    lev.android_array[Yspring_eB]	= Xspring;
    lev.android_array[Yspring_wB]	= Xspring;
    lev.android_array[Yspring_alien_eB]	= Xspring;
    lev.android_array[Yspring_alien_wB]	= Xspring;
  }

  if (lev.android_dynamite)
  {
    lev.android_array[Xdynamite]	= Xdynamite;
  }

  if (lev.android_balloon)
  {
    lev.android_array[Xballoon]		= Xballoon;
    lev.android_array[Yballoon_nB]	= Xballoon;
    lev.android_array[Yballoon_eB]	= Xballoon;
    lev.android_array[Yballoon_sB]	= Xballoon;
    lev.android_array[Yballoon_wB]	= Xballoon;
  }

  if (lev.android_amoeba)
  {
    lev.android_array[Xfake_amoeba]	= Xdrip;
    lev.android_array[Yfake_amoeba]	= Xdrip;
    lev.android_array[Xamoeba_1]	= Xdrip;
    lev.android_array[Xamoeba_2]	= Xdrip;
    lev.android_array[Xamoeba_3]	= Xdrip;
    lev.android_array[Xamoeba_4]	= Xdrip;
    lev.android_array[Xamoeba_5]	= Xdrip;
    lev.android_array[Xamoeba_6]	= Xdrip;
    lev.android_array[Xamoeba_7]	= Xdrip;
    lev.android_array[Xamoeba_8]	= Xdrip;
  }

  /* determine number of players in this level */
  lev.home_initial = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].exists = 0;
    ply[i].alive_initial = FALSE;

    if (ply[i].x_initial != -1 && ply[i].y_initial != -1)
    {
      ply[i].exists = 1;

      lev.home_initial++;
    }
  }

  team_mode = getTeamMode_EM();

  if (!team_mode)
    lev.home_initial = 1;

  lev.home = lev.home_initial;
  players_left = lev.home_initial;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (ply[i].exists)
    {
      if (players_left)
      {
	ply[i].alive_initial = TRUE;
	players_left--;
      }
      else
      {
	int x = ply[i].x_initial;
	int y = ply[i].y_initial;

	native_em_level.cave[x][y] = Xblank;

	lev.cave[lev.left + x][lev.top + y] = Xblank;
	lev.next[lev.left + x][lev.top + y] = Xblank;
	lev.draw[lev.left + x][lev.top + y] = Xblank;
      }
    }
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].num = i;
    ply[i].alive = ply[i].alive_initial;
    ply[i].dynamite = 0;
    ply[i].dynamite_cnt = 0;
    ply[i].keys = 0;
    ply[i].anim = 0;
    ply[i].oldx = ply[i].x = ply[i].x_initial + lev.left;
    ply[i].oldy = ply[i].y = ply[i].y_initial + lev.top;
    ply[i].last_move_dir = MV_NONE;
    ply[i].joy_n = ply[i].joy_e = ply[i].joy_s = ply[i].joy_w = 0;
    ply[i].joy_snap  = ply[i].joy_drop = 0;
    ply[i].joy_stick = ply[i].joy_spin = 0;
  }

  // the following engine variables are initialized to version-specific values
  // in function InitGameEngine() (src/game.c):
  //
  // - game_em.use_single_button (default: TRUE)
  // - game_em.use_snap_key_bug (default: FALSE)

  game_em.level_solved = FALSE;
  game_em.game_over = FALSE;

  game_em.any_player_moving = FALSE;
  game_em.any_player_snapping = FALSE;

  game_em.last_moving_player = 0;	/* default: first player */

  for (i = 0; i < MAX_PLAYERS; i++)
    game_em.last_player_direction[i] = MV_NONE;

  lev.exit_x = lev.exit_y = -1;	/* kludge for playing player exit sound */
}
