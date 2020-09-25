/* 2007-04-04 08:33:37
 *
 * convert intermediate cave format to internal cave structure for
 * use in logic(). initializes entire internal structure.
 */

#include "main_em.h"


static const short map[CAVE_TILE_MAX] =
{
  [Cblank]		= Xblank,
  [Cgrass]		= Xgrass,
  [Cdirt]		= Xdirt,
  [Cplant]		= Xplant,
  [Cwall_1]		= Xwall_1,
  [Cwall_2]		= Xwall_2,
  [Cwall_3]		= Xwall_3,
  [Cwall_4]		= Xwall_4,
  [Croundwall_1]	= Xroundwall_1,
  [Croundwall_2]	= Xroundwall_2,
  [Croundwall_3]	= Xroundwall_3,
  [Croundwall_4]	= Xroundwall_4,
  [Csteel_1]		= Xsteel_1,
  [Csteel_2]		= Xsteel_2,
  [Csteel_3]		= Xsteel_3,
  [Csteel_4]		= Xsteel_4,
  [Candroid]		= Xandroid,
  [Ceater_n]		= Xeater_n,
  [Ceater_e]		= Xeater_e,
  [Ceater_s]		= Xeater_s,
  [Ceater_w]		= Xeater_w,
  [Calien]		= Xalien,
  [Cbug_1_n]		= Xbug_1_n,
  [Cbug_1_e]		= Xbug_1_e,
  [Cbug_1_s]		= Xbug_1_s,
  [Cbug_1_w]		= Xbug_1_w,
  [Cbug_2_n]		= Xbug_2_n,
  [Cbug_2_e]		= Xbug_2_e,
  [Cbug_2_s]		= Xbug_2_s,
  [Cbug_2_w]		= Xbug_2_w,
  [Ctank_1_n]		= Xtank_1_n,
  [Ctank_1_e]		= Xtank_1_e,
  [Ctank_1_s]		= Xtank_1_s,
  [Ctank_1_w]		= Xtank_1_w,
  [Ctank_2_n]		= Xtank_2_n,
  [Ctank_2_e]		= Xtank_2_e,
  [Ctank_2_s]		= Xtank_2_s,
  [Ctank_2_w]		= Xtank_2_w,
  [Cemerald]		= Xemerald,
  [Cdiamond]		= Xdiamond,
  [Cstone]		= Xstone,
  [Cbomb]		= Xbomb,
  [Cnut]		= Xnut,
  [Cspring]		= Xspring,
  [Cspring_e]		= Xspring_e,
  [Cspring_w]		= Xspring_w,
  [Cpush_emerald_e]	= Xpush_emerald_e,
  [Cpush_emerald_w]	= Xpush_emerald_w,
  [Cpush_diamond_e]	= Xpush_diamond_e,
  [Cpush_diamond_w]	= Xpush_diamond_w,
  [Cpush_stone_e]	= Xpush_stone_e,
  [Cpush_stone_w]	= Xpush_stone_w,
  [Cpush_bomb_e]	= Xpush_bomb_e,
  [Cpush_bomb_w]	= Xpush_bomb_w,
  [Cpush_nut_e]		= Xpush_nut_e,
  [Cpush_nut_w]		= Xpush_nut_w,
  [Cpush_spring_e]	= Xpush_spring_e,
  [Cpush_spring_w]	= Xpush_spring_w,
  [Cdynamite]		= Xdynamite,
  [Cdynamite_1]		= Xdynamite_1,
  [Cdynamite_2]		= Xdynamite_2,
  [Cdynamite_3]		= Xdynamite_3,
  [Cdynamite_4]		= Xdynamite_4,
  [Ckey_1]		= Xkey_1,
  [Ckey_2]		= Xkey_2,
  [Ckey_3]		= Xkey_3,
  [Ckey_4]		= Xkey_4,
  [Ckey_5]		= Xkey_5,
  [Ckey_6]		= Xkey_6,
  [Ckey_7]		= Xkey_7,
  [Ckey_8]		= Xkey_8,
  [Cdoor_1]		= Xdoor_1,
  [Cdoor_2]		= Xdoor_2,
  [Cdoor_3]		= Xdoor_3,
  [Cdoor_4]		= Xdoor_4,
  [Cdoor_5]		= Xdoor_5,
  [Cdoor_6]		= Xdoor_6,
  [Cdoor_7]		= Xdoor_7,
  [Cdoor_8]		= Xdoor_8,
  [Cfake_door_1]	= Xfake_door_1,
  [Cfake_door_2]	= Xfake_door_2,
  [Cfake_door_3]	= Xfake_door_3,
  [Cfake_door_4]	= Xfake_door_4,
  [Cfake_door_5]	= Xfake_door_5,
  [Cfake_door_6]	= Xfake_door_6,
  [Cfake_door_7]	= Xfake_door_7,
  [Cfake_door_8]	= Xfake_door_8,
  [Cballoon]		= Xballoon,
  [Cball_1]		= Xball_1,
  [Cball_2]		= Xball_2,
  [Camoeba_1]		= Xamoeba_1,
  [Camoeba_2]		= Xamoeba_2,
  [Camoeba_3]		= Xamoeba_3,
  [Camoeba_4]		= Xamoeba_4,
  [Camoeba_5]		= Xamoeba_5,
  [Camoeba_6]		= Xamoeba_6,
  [Camoeba_7]		= Xamoeba_7,
  [Camoeba_8]		= Xamoeba_8,
  [Cdrip]		= Xdrip,
  [Cwonderwall]		= Xwonderwall,
  [Cwheel]		= Xwheel,
  [Cswitch]		= Xswitch,
  [Cbumper]		= Xbumper,
  [Cacid_nw]		= Xacid_nw,
  [Cacid_ne]		= Xacid_ne,
  [Cacid_sw]		= Xacid_sw,
  [Cacid_s]		= Xacid_s,
  [Cacid_se]		= Xacid_se,
  [Cacid_1]		= Xacid_1,
  [Cacid_2]		= Xacid_2,
  [Cacid_3]		= Xacid_3,
  [Cacid_4]		= Xacid_4,
  [Cacid_5]		= Xacid_5,
  [Cacid_6]		= Xacid_6,
  [Cacid_7]		= Xacid_7,
  [Cacid_8]		= Xacid_8,
  [Cfake_acid_1]	= Xfake_acid_1,
  [Cfake_acid_2]	= Xfake_acid_2,
  [Cfake_acid_3]	= Xfake_acid_3,
  [Cfake_acid_4]	= Xfake_acid_4,
  [Cfake_acid_5]	= Xfake_acid_5,
  [Cfake_acid_6]	= Xfake_acid_6,
  [Cfake_acid_7]	= Xfake_acid_7,
  [Cfake_acid_8]	= Xfake_acid_8,
  [Cfake_blank]		= Xfake_blank,
  [Cfake_grass]		= Xfake_grass,
  [Cfake_amoeba]	= Xfake_amoeba,
  [Clenses]		= Xlenses,
  [Cmagnify]		= Xmagnify,
  [Csand]		= Xsand,
  [Csand_stone]		= Xsand_stone,
  [Cslide_ns]		= Xslide_ns,
  [Cslide_ew]		= Xslide_ew,
  [Cwind_n]		= Xwind_n,
  [Cwind_e]		= Xwind_e,
  [Cwind_s]		= Xwind_s,
  [Cwind_w]		= Xwind_w,
  [Cwind_any]		= Xwind_any,
  [Cwind_stop]		= Xwind_stop,
  [Cexit]		= Xexit,
  [Cexit_1]		= Xexit_1,
  [Cexit_2]		= Xexit_2,
  [Cexit_3]		= Xexit_3,
  [Cpause]		= Xpause,
  [Cdecor_1]		= Xdecor_1,
  [Cdecor_2]		= Xdecor_2,
  [Cdecor_3]		= Xdecor_3,
  [Cdecor_4]		= Xdecor_4,
  [Cdecor_5]		= Xdecor_5,
  [Cdecor_6]		= Xdecor_6,
  [Cdecor_7]		= Xdecor_7,
  [Cdecor_8]		= Xdecor_8,
  [Cdecor_9]		= Xdecor_9,
  [Cdecor_10]		= Xdecor_10,
  [Cdecor_11]		= Xdecor_11,
  [Cdecor_12]		= Xdecor_12,
  [Calpha_0]		= Xalpha_0,
  [Calpha_1]		= Xalpha_1,
  [Calpha_2]		= Xalpha_2,
  [Calpha_3]		= Xalpha_3,
  [Calpha_4]		= Xalpha_4,
  [Calpha_5]		= Xalpha_5,
  [Calpha_6]		= Xalpha_6,
  [Calpha_7]		= Xalpha_7,
  [Calpha_8]		= Xalpha_8,
  [Calpha_9]		= Xalpha_9,
  [Calpha_excla]	= Xalpha_excla,
  [Calpha_apost]	= Xalpha_apost,
  [Calpha_comma]	= Xalpha_comma,
  [Calpha_minus]	= Xalpha_minus,
  [Calpha_perio]	= Xalpha_perio,
  [Calpha_colon]	= Xalpha_colon,
  [Calpha_quest]	= Xalpha_quest,
  [Calpha_a]		= Xalpha_a,
  [Calpha_b]		= Xalpha_b,
  [Calpha_c]		= Xalpha_c,
  [Calpha_d]		= Xalpha_d,
  [Calpha_e]		= Xalpha_e,
  [Calpha_f]		= Xalpha_f,
  [Calpha_g]		= Xalpha_g,
  [Calpha_h]		= Xalpha_h,
  [Calpha_i]		= Xalpha_i,
  [Calpha_j]		= Xalpha_j,
  [Calpha_k]		= Xalpha_k,
  [Calpha_l]		= Xalpha_l,
  [Calpha_m]		= Xalpha_m,
  [Calpha_n]		= Xalpha_n,
  [Calpha_o]		= Xalpha_o,
  [Calpha_p]		= Xalpha_p,
  [Calpha_q]		= Xalpha_q,
  [Calpha_r]		= Xalpha_r,
  [Calpha_s]		= Xalpha_s,
  [Calpha_t]		= Xalpha_t,
  [Calpha_u]		= Xalpha_u,
  [Calpha_v]		= Xalpha_v,
  [Calpha_w]		= Xalpha_w,
  [Calpha_x]		= Xalpha_x,
  [Calpha_y]		= Xalpha_y,
  [Calpha_z]		= Xalpha_z,
  [Calpha_arrow_e]	= Xalpha_arrow_e,
  [Calpha_arrow_w]	= Xalpha_arrow_w,
  [Calpha_copyr]	= Xalpha_copyr
};

int map_em_element_C_to_X(int element_em_cave)
{
  if (element_em_cave < 0 || element_em_cave >= CAVE_TILE_MAX)
  {
    Warn("invalid EM cave element %d", element_em_cave);

    return Xblank;
  }

  return map[element_em_cave];
}

int map_em_element_X_to_C(int element_em_game)
{
  static unsigned short map_reverse[GAME_TILE_MAX];
  static boolean map_reverse_initialized = FALSE;

  if (!map_reverse_initialized)
  {
    int i;

    // return "Cblank" for all undefined elements in mapping array
    for (i = 0; i < GAME_TILE_MAX; i++)
      map_reverse[i] = Cblank;

    for (i = 0; i < CAVE_TILE_MAX; i++)
      map_reverse[map[i]] = i;

    map_reverse_initialized = TRUE;
  }

  if (element_em_game < 0 || element_em_game >= GAME_TILE_MAX)
  {
    Warn("invalid EM game element %d", element_em_game);

    return Cblank;
  }

  int element_em_cave = map_reverse[element_em_game];

  if (element_em_cave == Cblank && element_em_game != Xblank)
    Warn("unknown EM game element %d", element_em_game);

  return element_em_cave;
}

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

  lev.infinite = game_em.use_wrap_around;
  lev.infinite_true = cav.infinite_true;

  if (lev.infinite)
  {
    /* add linked cave buffer columns for wrap-around movement */
    for (x = 0; x < lev.left; x++)
    {
      int offset = (lev.infinite_true ? 0 : 1);

      lev.cavecol[x] = &lev.cavecol[lev.width + x][-offset];
      lev.nextcol[x] = &lev.nextcol[lev.width + x][-offset];
      lev.drawcol[x] = &lev.drawcol[lev.width + x][-offset];
      lev.boomcol[x] = &lev.boomcol[lev.width + x][-offset];

      lev.cavecol[lev.right + x] = &lev.cavecol[lev.left + x][offset];
      lev.nextcol[lev.right + x] = &lev.nextcol[lev.left + x][offset];
      lev.drawcol[lev.right + x] = &lev.drawcol[lev.left + x][offset];
      lev.boomcol[lev.right + x] = &lev.boomcol[lev.left + x][offset];
    }
  }

  for (x = 0; x < lev.width; x++)
    for (y = 0; y < lev.height; y++)
      lev.cave[lev.left + x][lev.top + y] = map[cav.cave[x][y]];

  for (x = lev.left; x < lev.right; x++)
    for (y = lev.top; y < lev.bottom; y++)
      lev.next[x][y] = lev.draw[x][y] = lev.cave[x][y];

  lev.time = cav.time_seconds;
  lev.gems_needed = cav.gems_needed;
  lev.score = 0;

  lev.testmode = cav.testmode;

  if (lev.testmode)
  {
    lev.time = 0;
    lev.gems_needed = 0;
  }

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
  lev.ball_active = cav.ball_active;
  lev.ball_random = cav.ball_random;
  lev.ball_pos    = 0;

  lev.eater_pos = 0;
  lev.shine_cnt = 0;

  lev.lenses_time = cav.lenses_time;
  lev.lenses_cnt  = cav.lenses_active ? cav.lenses_time : 0;

  lev.magnify_time = cav.magnify_time;
  lev.magnify_cnt  = cav.magnify_active ? cav.magnify_time : 0;

  lev.wheel_time = cav.wheel_time;
  lev.wheel_cnt  = cav.wheel_active ? cav.wheel_time : 0;
  lev.wheel_x    = cav.wheel_x + lev.left;
  lev.wheel_y    = cav.wheel_y + lev.top;

  lev.wind_time      = cav.wind_time;
  lev.wind_cnt       = cav.wind_time;
  lev.wind_direction = cav.wind_direction;

  lev.wonderwall_time   = cav.wonderwall_time;
  lev.wonderwall_active = cav.wonderwall_active;

  lev.killed_out_of_time = FALSE;

  lev.num_eater_arrays = cav.num_eater_arrays;
  lev.num_ball_arrays  = cav.num_ball_arrays;

  for (i = 0; i < 8; i++)
    for (j = 0; j < 9; j++)
      lev.eater_array[i][j] = map[cav.eater_array[i][j]];

  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)
      lev.ball_array[i][j] = map[cav.ball_array[i][j]];

  for (i = 0; i < GAME_TILE_MAX; i++)
    lev.android_array[i] = map[cav.android_array[i]];

  lev.home_initial = 0;

  /* check for players in this level */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].exists = FALSE;
    ply[i].alive = FALSE;

    if (cav.player_x[i] != -1 &&
	cav.player_y[i] != -1)
    {
      ply[i].exists = TRUE;

      lev.home_initial++;
    }
  }

  team_mode = getTeamMode_EM();

  if (!team_mode)
    lev.home_initial = 1;

  lev.home = lev.home_initial;
  players_left = lev.home_initial;

  /* assign active players */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (ply[i].exists && isActivePlayer_EM(i))
    {
      if (players_left)
      {
	ply[i].alive = TRUE;
	players_left--;
      }
    }
  }

  /* remove inactive players */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (ply[i].exists && !ply[i].alive)
    {
      int x = cav.player_x[i];
      int y = cav.player_y[i];

      lev.cave[lev.left + x][lev.top + y] = Xblank;
      lev.next[lev.left + x][lev.top + y] = Xblank;
      lev.draw[lev.left + x][lev.top + y] = Xblank;
    }
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].num = i;
    ply[i].anim = PLY_still;

    ply[i].x = cav.player_x[i] + lev.left;
    ply[i].y = cav.player_y[i] + lev.top;
    ply[i].prev_x = ply[i].x;
    ply[i].prev_y = ply[i].y;

    ply[i].dynamite	= 0;
    ply[i].dynamite_cnt	= 0;
    ply[i].keys		= 0;

    ply[i].last_move_dir = MV_NONE;

    ply[i].joy_n	= FALSE;
    ply[i].joy_e	= FALSE;
    ply[i].joy_s	= FALSE;
    ply[i].joy_w	= FALSE;
    ply[i].joy_snap	= FALSE;
    ply[i].joy_drop	= FALSE;
    ply[i].joy_stick	= FALSE;
  }

  // the following engine variables are initialized to version-specific values
  // in function InitGameEngine() (src/game.c):
  //
  // - game_em.use_single_button (default: TRUE)
  // - game_em.use_snap_key_bug (default: FALSE)
  // - game_em.use_random_bug (default: FALSE)
  // - game_em.use_old_explosions (default: FALSE)
  // - game_em.use_old_android (default: FALSE)
  // - game_em.use_old_push_elements (default: FALSE)
  // - game_em.use_old_push_into_acid (default: FALSE)
  // - game_em.use_wrap_around (default: TRUE)

  game_em.level_solved = FALSE;
  game_em.game_over = FALSE;

  game_em.any_player_moving = FALSE;
  game_em.any_player_snapping = FALSE;

  game_em.last_moving_player = 0;	/* default: first player */

  for (i = 0; i < MAX_PLAYERS; i++)
    game_em.last_player_direction[i] = MV_NONE;

  lev.exit_x = lev.exit_y = -1;	/* kludge for playing player exit sound */
}
