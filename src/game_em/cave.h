/* 2009-01-25 23:00:34
 *
 * intermediate cave structure
 */

#ifndef CAVE_H
#define CAVE_H

enum
{
  Cblank,
  Cgrass,
  Cdirt,
  Cplant,
  Cwall_1,
  Cwall_2,
  Cwall_3,
  Cwall_4,
  Croundwall_1,
  Croundwall_2,
  Croundwall_3,
  Croundwall_4,
  Csteel_1,
  Csteel_2,
  Csteel_3,
  Csteel_4,
  Candroid,
  Ceater_n,
  Ceater_e,
  Ceater_s,
  Ceater_w,
  Calien,
  Cbug_1_n,
  Cbug_1_e,
  Cbug_1_s,
  Cbug_1_w,
  Cbug_2_n,
  Cbug_2_e,
  Cbug_2_s,
  Cbug_2_w,
  Ctank_1_n,
  Ctank_1_e,
  Ctank_1_s,
  Ctank_1_w,
  Ctank_2_n,
  Ctank_2_e,
  Ctank_2_s,
  Ctank_2_w,
  Cemerald,
  Cdiamond,
  Cstone,
  Cbomb,
  Cnut,
  Cspring,
  Cspring_e,
  Cspring_w,
  Cpush_emerald_e,
  Cpush_emerald_w,
  Cpush_diamond_e,
  Cpush_diamond_w,
  Cpush_stone_e,
  Cpush_stone_w,
  Cpush_bomb_e,
  Cpush_bomb_w,
  Cpush_nut_e,
  Cpush_nut_w,
  Cpush_spring_e,
  Cpush_spring_w,
  Cdynamite,
  Cdynamite_1,
  Cdynamite_2,
  Cdynamite_3,
  Cdynamite_4,
  Ckey_1,
  Ckey_2,
  Ckey_3,
  Ckey_4,
  Ckey_5,
  Ckey_6,
  Ckey_7,
  Ckey_8,
  Cdoor_1,
  Cdoor_2,
  Cdoor_3,
  Cdoor_4,
  Cdoor_5,
  Cdoor_6,
  Cdoor_7,
  Cdoor_8,
  Cfake_door_1,
  Cfake_door_2,
  Cfake_door_3,
  Cfake_door_4,
  Cfake_door_5,
  Cfake_door_6,
  Cfake_door_7,
  Cfake_door_8,
  Cballoon,
  Cball_1,
  Cball_2,
  Camoeba_1,
  Camoeba_2,
  Camoeba_3,
  Camoeba_4,
  Camoeba_5,
  Camoeba_6,
  Camoeba_7,
  Camoeba_8,
  Cdrip,
  Cwonderwall,
  Cwheel,
  Cswitch,
  Cbumper,
  Cacid_nw,
  Cacid_ne,
  Cacid_sw,
  Cacid_s,
  Cacid_se,
  Cacid_1,
  Cacid_2,
  Cacid_3,
  Cacid_4,
  Cacid_5,
  Cacid_6,
  Cacid_7,
  Cacid_8,
  Cfake_acid_1,
  Cfake_acid_2,
  Cfake_acid_3,
  Cfake_acid_4,
  Cfake_acid_5,
  Cfake_acid_6,
  Cfake_acid_7,
  Cfake_acid_8,
  Cfake_blank,
  Cfake_grass,
  Cfake_amoeba,
  Clenses,
  Cmagnify,
  Csand,
  Csand_stone,
  Cslide_ns,
  Cslide_ew,
  Cwind_n,
  Cwind_e,
  Cwind_s,
  Cwind_w,
  Cwind_any,
  Cwind_stop,
  Cexit,
  Cexit_1,
  Cexit_2,
  Cexit_3,
  Cpause,
  Cdecor_1,
  Cdecor_2,
  Cdecor_3,
  Cdecor_4,
  Cdecor_5,
  Cdecor_6,
  Cdecor_7,
  Cdecor_8,
  Cdecor_9,
  Cdecor_10,
  Cdecor_11,
  Cdecor_12,
  Calpha_0,
  Calpha_1,
  Calpha_2,
  Calpha_3,
  Calpha_4,
  Calpha_5,
  Calpha_6,
  Calpha_7,
  Calpha_8,
  Calpha_9,
  Calpha_excla,
  Calpha_apost,
  Calpha_comma,
  Calpha_minus,
  Calpha_perio,
  Calpha_colon,
  Calpha_quest,
  Calpha_a,
  Calpha_b,
  Calpha_c,
  Calpha_d,
  Calpha_e,
  Calpha_f,
  Calpha_g,
  Calpha_h,
  Calpha_i,
  Calpha_j,
  Calpha_k,
  Calpha_l,
  Calpha_m,
  Calpha_n,
  Calpha_o,
  Calpha_p,
  Calpha_q,
  Calpha_r,
  Calpha_s,
  Calpha_t,
  Calpha_u,
  Calpha_v,
  Calpha_w,
  Calpha_x,
  Calpha_y,
  Calpha_z,
  Calpha_arrow_e,
  Calpha_arrow_w,
  Calpha_copyr,

  CAVE_TILE_MAX
};

struct CAVE
{
  int width;			/* cave width */
  int height;			/* cave height */

  int player_x[MAX_PLAYERS];	/* player x position */
  int player_y[MAX_PLAYERS];	/* player y position */

  int time_seconds;		/* available time (seconds) */
  int gems_needed;		/* emeralds needed */

  int eater_score;		/* score for killing eater */
  int alien_score;		/* score for killing alien */
  int bug_score;		/* score for killing bug */
  int tank_score;		/* score for killing tank */
  int slurp_score;		/* score for slurping alien with spring */
  int nut_score;		/* score for cracking nut to emerald */
  int emerald_score;		/* score for collecting emerald */
  int diamond_score;		/* score for collecting diamond */
  int dynamite_score;		/* score for collecting dynamite */
  int key_score;		/* score for colleting key */
  int lenses_score;		/* score for collecting lenses */
  int magnify_score;		/* score for collecting magnifier */
  int exit_score;		/* score for entering exit */

  int android_move_time;	/* reset time for android movement */
  int android_clone_time;	/* reset time for android cloning */
  int ball_time;		/* reset time for ball activity */
  int amoeba_time;		/* amoeba growth speed */
  int wonderwall_time;		/* reset time for wonderwall activity */
  int wheel_time;		/* reset time for wheel activity */
  int wheel_x;			/* wheel x position */
  int wheel_y;			/* wheel y position */
  int lenses_time;		/* reset time for lenses activity */
  int magnify_time;		/* reset time for magnifier activity */
  int wind_time;		/* reset time for wind activity */
  int wind_direction;		/* wind direction */

  int num_eater_arrays;		/* number of eater data arrays used */
  int num_ball_arrays;		/* number of ball data arrays used */

  boolean testmode;		/* flag for test mode */
  boolean teamwork;		/* flag for two player mode */
  boolean infinite;		/* flag for infinitely wide cave */
  boolean infinite_true;	/* flag for truely infinitely wide cave */

  boolean ball_random;		/* flag if ball is random */
  boolean ball_active;		/* flag if ball is already active */
  boolean wonderwall_active;	/* flag if wonderwall is already active */
  boolean wheel_active;		/* flag if wheel is already active */
  boolean lenses_active;	/* flag if lenses are already active */
  boolean magnify_active;	/* flag if magnifier is already active */

  short eater_array[8][9];		/* eater data */
  short ball_array[8][8];		/* ball data */
  short android_array[GAME_TILE_MAX];	/* android clone data */

  short cave[CAVE_WIDTH][CAVE_HEIGHT];	/* cave data */
};

#endif	// CAVE_H
