/* 2007-04-04 08:33:37
 *
 * convert intermediate cave format to internal cave structure for
 * use in logic(). initializes entire internal structure.
 */

#include "main_em.h"


void prepare_em_level(void)
{
  int i, j, x, y;
  int players_left;
  boolean team_mode;

  /* reset all runtime variables to their initial values */

  game_init_cave_buffers();

  lev.width  = cav.width;
  lev.height = cav.height;

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
      lev.cave[lev.left + x][lev.top + y] = cav.cave[x][y];

  for (x = lev.left; x < lev.right; x++)
    for (y = lev.top; y < lev.bottom; y++)
      lev.next[x][y] = lev.draw[x][y] = lev.cave[x][y];

  lev.time = cav.time_seconds;
  lev.gems_needed = cav.gems_needed;
  lev.score = 0;

  lev.eater_score	= cav.eater_score;
  lev.alien_score	= cav.alien_score;
  lev.bug_score		= cav.bug_score;
  lev.tank_score	= cav.tank_score;
  lev.emerald_score	= cav.emerald_score;
  lev.diamond_score	= cav.diamond_score;
  lev.nut_score		= cav.nut_score;
  lev.slurp_score	= cav.slurp_score;
  lev.dynamite_score	= cav.dynamite_score;
  lev.key_score		= cav.key_score;
  lev.lenses_score	= cav.lenses_score;
  lev.magnify_score	= cav.magnify_score;
  lev.exit_score	= cav.exit_score;

  lev.amoeba_time = cav.amoeba_time;

  lev.android_move_time = cav.android_move_time;
  lev.android_move_cnt  = cav.android_move_time;

  lev.android_clone_time = cav.android_clone_time;
  lev.android_clone_cnt  = cav.android_clone_time;

  lev.ball_time   = cav.ball_time;
  lev.ball_cnt    = cav.ball_time;
  lev.ball_state  = cav.ball_state;
  lev.ball_random = cav.ball_random;
  lev.ball_pos    = 0;

  lev.eater_pos = 0;
  lev.shine_cnt = 0;

  lev.lenses_time = cav.lenses_time;
  lev.lenses_cnt  = cav.lenses_cnt;

  lev.magnify_time = cav.magnify_time;
  lev.magnify_cnt  = cav.magnify_cnt;

  lev.wheel_time = cav.wheel_time;
  lev.wheel_cnt  = cav.wheel_cnt;
  lev.wheel_x    = cav.wheel_x;
  lev.wheel_y    = cav.wheel_y;

  lev.wind_time      = cav.wind_time;
  lev.wind_cnt       = cav.wind_cnt;
  lev.wind_direction = cav.wind_direction;

  lev.wonderwall_time  = cav.wonderwall_time;
  lev.wonderwall_state = cav.wonderwall_state;

  lev.killed_out_of_time = FALSE;

  lev.num_ball_arrays = cav.num_ball_arrays;

  for (i = 0; i < 8; i++)
    for (j = 0; j < 9; j++)
      lev.eater_array[i][j] = cav.eater_array[i][j];

  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
      lev.ball_array[i][j] = cav.ball_array[i][j];

  for (i = 0; i < TILE_MAX; i++)
    lev.android_array[i] = cav.android_array[i];

  /* determine number of players in this level */
  lev.home_initial = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].exists = 0;
    ply[i].alive_initial = FALSE;

    if (cav.player_x[i] != -1 &&
	cav.player_y[i] != -1)
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
	int x = cav.player_x[i];
	int y = cav.player_y[i];

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
    ply[i].oldx = ply[i].x = cav.player_x[i] + lev.left;
    ply[i].oldy = ply[i].y = cav.player_y[i] + lev.top;
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
