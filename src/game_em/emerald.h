/*

This program "Emerald Mine for X11"
is copyright © 2009 David Tritscher.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. The origin of this software must not be misrepresented; you must
   not claim that you wrote the original software.  If you use this
   software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

3. Altered source versions must be plainly marked as such, and must
   not be misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
/* 2007-03-31 06:22:47
 *
 * emerald mine game engine defines
 */

// ----------------------------------------------------------------------------
// EM game engine source code was altered for integration in Rocks'n'Diamonds
// ----------------------------------------------------------------------------

#ifndef EMERALD_H
#define EMERALD_H


// ----------------------------------------------------------------------------
// constant definitions
// ----------------------------------------------------------------------------

/* maximum cave size */
#define CAVE_WIDTH			MAX_PLAYFIELD_WIDTH
#define CAVE_HEIGHT			MAX_PLAYFIELD_HEIGHT

/* additional padding for Zborder elements and linked cave buffer columns */
#define CAVE_BUFFER_XOFFSET		4
#define CAVE_BUFFER_YOFFSET		2
#define CAVE_BUFFER_WIDTH		(CAVE_WIDTH  + 2 * CAVE_BUFFER_XOFFSET)
#define CAVE_BUFFER_HEIGHT		(CAVE_HEIGHT + 2 * CAVE_BUFFER_YOFFSET)


// ----------------------------------------------------------------------------
// definition of elements used in the Emerald Mine Club engine;
// the element names (mostly) have the following properties:
// - elements that start with 'X' are not animated (more or less)
// - elements that start with 'Y' are animated elements
// - elements that start with 'Z' are internal elements (without graphics)
// - elements that end with 'B' or '_blank' change to the "Xblank" element
// ----------------------------------------------------------------------------

enum
{
  Zborder,		/* special code to indicate border */
  Zplayer,		/* special code to indicate player */

  Zbug,			/* internal bug explosion */
  Ztank,		/* internal tank/alien/bomb explosion */
  Zeater,		/* internal eater explosion */
  Zdynamite,		/* internal dynamite explosion */
  Zboom,		/* explosion */

  Xchain,		/* chain explosion; transition to Zboom */
  Xboom_bug,		/* bug explosion; transition to Zbug */
  Xboom_tank,		/* tank/alien/bomb explosion; transition to Ztank */
  Xboom_android,	/* android explosion; transition to Xboom_2 */
  Xboom_1,		/* tile explosion; transition to Xboom_2 */
  Xboom_2,		/* transition to boom[] */

  Xblank,

  Xsplash_e,
  Xsplash_w,

  Xplant,
  Yplant,

  Xacid_1,
  Xacid_2,
  Xacid_3,
  Xacid_4,
  Xacid_5,
  Xacid_6,
  Xacid_7,
  Xacid_8,

  Xfake_acid_1,			/* newly added to EM engine */
  Xfake_acid_2,
  Xfake_acid_3,
  Xfake_acid_4,
  Xfake_acid_5,
  Xfake_acid_6,
  Xfake_acid_7,
  Xfake_acid_8,

  Xfake_acid_1_player,		/* newly added to EM engine */
  Xfake_acid_2_player,
  Xfake_acid_3_player,
  Xfake_acid_4_player,
  Xfake_acid_5_player,
  Xfake_acid_6_player,
  Xfake_acid_7_player,
  Xfake_acid_8_player,

  Xgrass,
  Ygrass_nB,
  Ygrass_eB,
  Ygrass_sB,
  Ygrass_wB,

  Xdirt,
  Ydirt_nB,
  Ydirt_eB,
  Ydirt_sB,
  Ydirt_wB,

  Xandroid,
  Xandroid_1_n,
  Xandroid_1_e,
  Xandroid_1_w,
  Xandroid_1_s,
  Xandroid_2_n,
  Xandroid_2_e,
  Xandroid_2_w,
  Xandroid_2_s,
  Yandroid_n,
  Yandroid_nB,
  Yandroid_ne,
  Yandroid_neB,
  Yandroid_e,
  Yandroid_eB,
  Yandroid_se,
  Yandroid_seB,
  Yandroid_s,
  Yandroid_sB,
  Yandroid_sw,
  Yandroid_swB,
  Yandroid_w,
  Yandroid_wB,
  Yandroid_nw,
  Yandroid_nwB,

  Xeater_n,
  Xeater_e,
  Xeater_w,
  Xeater_s,
  Yeater_n,
  Yeater_nB,
  Yeater_e,
  Yeater_eB,
  Yeater_s,
  Yeater_sB,
  Yeater_w,
  Yeater_wB,
  Yeater_stone,
  Yeater_spring,

  Xalien,
  Xalien_pause,
  Yalien_n,
  Yalien_nB,
  Yalien_e,
  Yalien_eB,
  Yalien_s,
  Yalien_sB,
  Yalien_w,
  Yalien_wB,
  Yalien_stone,
  Yalien_spring,

  Xbug_1_n,
  Xbug_1_e,
  Xbug_1_s,
  Xbug_1_w,
  Xbug_2_n,
  Xbug_2_e,
  Xbug_2_s,
  Xbug_2_w,
  Ybug_n,
  Ybug_nB,
  Ybug_e,
  Ybug_eB,
  Ybug_s,
  Ybug_sB,
  Ybug_w,
  Ybug_wB,
  Ybug_w_n,
  Ybug_n_e,
  Ybug_e_s,
  Ybug_s_w,
  Ybug_e_n,
  Ybug_s_e,
  Ybug_w_s,
  Ybug_n_w,
  Ybug_stone,
  Ybug_spring,

  Xtank_1_n,
  Xtank_1_e,
  Xtank_1_s,
  Xtank_1_w,
  Xtank_2_n,
  Xtank_2_e,
  Xtank_2_s,
  Xtank_2_w,
  Ytank_n,
  Ytank_nB,
  Ytank_e,
  Ytank_eB,
  Ytank_s,
  Ytank_sB,
  Ytank_w,
  Ytank_wB,
  Ytank_w_n,
  Ytank_n_e,
  Ytank_e_s,
  Ytank_s_w,
  Ytank_e_n,
  Ytank_s_e,
  Ytank_w_s,
  Ytank_n_w,
  Ytank_stone,
  Ytank_spring,

  Xemerald,
  Xemerald_pause,
  Xemerald_fall,
  Xemerald_shine,
  Yemerald_s,
  Yemerald_sB,
  Yemerald_e,
  Yemerald_eB,
  Yemerald_w,
  Yemerald_wB,
  Yemerald_blank,

  Xdiamond,
  Xdiamond_pause,
  Xdiamond_fall,
  Xdiamond_shine,
  Ydiamond_s,
  Ydiamond_sB,
  Ydiamond_e,
  Ydiamond_eB,
  Ydiamond_w,
  Ydiamond_wB,
  Ydiamond_blank,
  Ydiamond_stone,

  Xstone,
  Xstone_pause,
  Xstone_fall,
  Ystone_s,
  Ystone_sB,
  Ystone_e,
  Ystone_eB,
  Ystone_w,
  Ystone_wB,

  Xbomb,
  Xbomb_pause,
  Xbomb_fall,
  Ybomb_s,
  Ybomb_sB,
  Ybomb_e,
  Ybomb_eB,
  Ybomb_w,
  Ybomb_wB,
  Ybomb_blank,

  Xnut,
  Xnut_pause,
  Xnut_fall,
  Ynut_s,
  Ynut_sB,
  Ynut_e,
  Ynut_eB,
  Ynut_w,
  Ynut_wB,
  Ynut_stone,

  Xspring,
  Xspring_pause,
  Xspring_e,
  Xspring_w,
  Xspring_fall,
  Yspring_s,
  Yspring_sB,
  Yspring_e,
  Yspring_eB,
  Yspring_w,
  Yspring_wB,
  Yspring_alien_e,
  Yspring_alien_eB,
  Yspring_alien_w,
  Yspring_alien_wB,

  Xpush_emerald_e,
  Xpush_emerald_w,
  Xpush_diamond_e,
  Xpush_diamond_w,
  Xpush_stone_e,
  Xpush_stone_w,
  Xpush_bomb_e,
  Xpush_bomb_w,
  Xpush_nut_e,
  Xpush_nut_w,
  Xpush_spring_e,
  Xpush_spring_w,

  Xdynamite,
  Ydynamite_blank,
  Xdynamite_1,
  Xdynamite_2,
  Xdynamite_3,
  Xdynamite_4,

  Xkey_1,
  Xkey_2,
  Xkey_3,
  Xkey_4,
  Xkey_5,
  Xkey_6,
  Xkey_7,
  Xkey_8,

  Xdoor_1,
  Xdoor_2,
  Xdoor_3,
  Xdoor_4,
  Xdoor_5,
  Xdoor_6,
  Xdoor_7,
  Xdoor_8,

  Xfake_door_1,
  Xfake_door_2,
  Xfake_door_3,
  Xfake_door_4,
  Xfake_door_5,
  Xfake_door_6,
  Xfake_door_7,
  Xfake_door_8,

  Xballoon,
  Yballoon_n,
  Yballoon_nB,
  Yballoon_e,
  Yballoon_eB,
  Yballoon_s,
  Yballoon_sB,
  Yballoon_w,
  Yballoon_wB,

  Xball_1,
  Yball_1,
  Xball_2,
  Yball_2,
  Yball_blank,

  Xamoeba_1,
  Xamoeba_2,
  Xamoeba_3,
  Xamoeba_4,
  Xamoeba_5,
  Xamoeba_6,
  Xamoeba_7,
  Xamoeba_8,

  Xdrip,
  Xdrip_fall,
  Xdrip_stretch,
  Xdrip_stretchB,
  Ydrip_1_s,
  Ydrip_1_sB,
  Ydrip_2_s,
  Ydrip_2_sB,

  Xwonderwall,
  Ywonderwall,

  Xwheel,
  Ywheel,

  Xswitch,
  Yswitch,

  Xbumper,
  Ybumper,

  Xacid_nw,
  Xacid_ne,
  Xacid_sw,
  Xacid_s,
  Xacid_se,

  Xfake_blank,
  Yfake_blank,

  Xfake_grass,
  Yfake_grass,

  Xfake_amoeba,			/* dripper */
  Yfake_amoeba,

  Xlenses,

  Xmagnify,

  Xsand,
  Xsand_stone,
  Xsand_stonein_1,
  Xsand_stonein_2,
  Xsand_stonein_3,
  Xsand_stonein_4,
  Xsand_sandstone_1,
  Xsand_sandstone_2,
  Xsand_sandstone_3,
  Xsand_sandstone_4,
  Xsand_stonesand_1,
  Xsand_stonesand_2,
  Xsand_stonesand_3,
  Xsand_stonesand_4,
  Xsand_stoneout_1,
  Xsand_stoneout_2,
  Xsand_stonesand_quickout_1,	/* newly added to EM engine */
  Xsand_stonesand_quickout_2,

  Xslide_ns,			/* growing wall */
  Yslide_ns_blank,
  Xslide_ew,
  Yslide_ew_blank,

  Xwind_n,
  Xwind_e,
  Xwind_s,
  Xwind_w,
  Xwind_any,
  Xwind_stop,

  Xexit,
  Xexit_1,
  Xexit_2,
  Xexit_3,

  Xpause,

  Xwall_1,
  Xwall_2,
  Xwall_3,
  Xwall_4,

  Xroundwall_1,
  Xroundwall_2,
  Xroundwall_3,
  Xroundwall_4,

  Xsteel_1,
  Xsteel_2,
  Xsteel_3,
  Xsteel_4,

  Xdecor_1,
  Xdecor_2,
  Xdecor_3,
  Xdecor_4,
  Xdecor_5,
  Xdecor_6,
  Xdecor_7,
  Xdecor_8,
  Xdecor_9,
  Xdecor_10,
  Xdecor_11,
  Xdecor_12,

  Xalpha_0,
  Xalpha_1,
  Xalpha_2,
  Xalpha_3,
  Xalpha_4,
  Xalpha_5,
  Xalpha_6,
  Xalpha_7,
  Xalpha_8,
  Xalpha_9,
  Xalpha_excla,
  Xalpha_apost,
  Xalpha_comma,
  Xalpha_minus,
  Xalpha_perio,
  Xalpha_colon,
  Xalpha_quest,
  Xalpha_a,
  Xalpha_b,
  Xalpha_c,
  Xalpha_d,
  Xalpha_e,
  Xalpha_f,
  Xalpha_g,
  Xalpha_h,
  Xalpha_i,
  Xalpha_j,
  Xalpha_k,
  Xalpha_l,
  Xalpha_m,
  Xalpha_n,
  Xalpha_o,
  Xalpha_p,
  Xalpha_q,
  Xalpha_r,
  Xalpha_s,
  Xalpha_t,
  Xalpha_u,
  Xalpha_v,
  Xalpha_w,
  Xalpha_x,
  Xalpha_y,
  Xalpha_z,
  Xalpha_arrow_e,
  Xalpha_arrow_w,
  Xalpha_copyr,

  Ykey_1_blank,			/* newly added to EM engine */
  Ykey_2_blank,
  Ykey_3_blank,
  Ykey_4_blank,
  Ykey_5_blank,
  Ykey_6_blank,
  Ykey_7_blank,
  Ykey_8_blank,
  Ylenses_blank,
  Ymagnify_blank,
  Ygrass_blank,
  Ydirt_blank,

  GAME_TILE_MAX
};

/* other definitions */

enum
{
  PLY_still = 0,

  PLY_walk_n,
  PLY_walk_e,
  PLY_walk_s,
  PLY_walk_w,

  PLY_push_n,
  PLY_push_e,
  PLY_push_s,
  PLY_push_w,

  PLY_shoot_n,
  PLY_shoot_e,
  PLY_shoot_s,
  PLY_shoot_w,

  PLY_MAX
};

enum
{
  SOUND_blank,		/* player walks on blank */
  SOUND_roll,		/* player pushes stone/bomb/nut/spring */
  SOUND_stone,		/* stone hits ground */
  SOUND_nut,		/* nut hits ground */
  SOUND_crack,		/* stone hits nut */
  SOUND_bug,		/* bug moves */
  SOUND_tank,		/* tank moves */
  SOUND_android_clone,	/* android places something */
  SOUND_android_move,	/* android moves */
  SOUND_spring,		/* spring hits ground/wall/bumper, stone hits spring */
  SOUND_slurp,		/* spring kills alien */
  SOUND_eater,		/* eater sits */
  SOUND_eater_eat,	/* eater eats diamond */
  SOUND_alien,		/* alien moves */
  SOUND_collect,	/* player collects object */
  SOUND_diamond,	/* diamond/emerald hits ground */
  SOUND_squash,		/* stone squashes diamond */
  SOUND_wonderfall,	/* object falls thru wonderwall */
  SOUND_drip,		/* drip hits ground */
  SOUND_push,		/* player pushes spring/balloon/android */
  SOUND_dirt,		/* player digs into dirt */
  SOUND_acid,		/* acid splashes */
  SOUND_ball,		/* ball places something */
  SOUND_slide,		/* slide wall grows */
  SOUND_wonder,		/* wonderwall is active */
  SOUND_door,		/* player goes thru door (gate) */
  SOUND_exit_open,	/* exit opens */
  SOUND_exit_leave,	/* player goes into exit */
  SOUND_dynamite,	/* player places dynamite */
  SOUND_tick,		/* dynamite ticks */
  SOUND_press,		/* player presses wheel/wind/switch */
  SOUND_wheel,		/* wheel moves */
  SOUND_boom,		/* explosion */
  SOUND_time,		/* time runs out */
  SOUND_die,		/* player dies */

  SOUND_MAX
};


// ----------------------------------------------------------------------------
// data structure definitions
// ----------------------------------------------------------------------------

/* structure used by logic() for gameplay */

struct PLAYER
{
  int num;			/* player number */
  int anim;

  int x;
  int y;
  int prev_x;
  int prev_y;

  boolean exists;		/* flag if player exists in cave */
  boolean alive;		/* flag if player is alive */

  int dynamite;			/* number of pieces of collected dynamite */
  int dynamite_cnt;		/* how long the player has held down fire */
  int keys;			/* keys the player has collected */

  int last_move_dir;

  boolean joy_n;
  boolean joy_e;
  boolean joy_s;
  boolean joy_w;
  boolean joy_snap;
  boolean joy_drop;
  boolean joy_stick;
};

struct LOGIC
{
  int width;			/* cave width */
  int height;			/* cave height */

  int left;			/* cave left edge */
  int top;			/* cave top edge */
  int right;			/* cave right edge */
  int bottom;			/* cave bottom edge */

  int time;			/* time remaining */
  int gems_needed;		/* emeralds needed */
  int score;			/* score */

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

  boolean ball_random;		/* flag if ball is random */
  boolean ball_active;		/* flag if ball is already active */
  boolean wonderwall_active;	/* flag if wonderwall is already active */

  int wheel_cnt;		/* counter for wheel activity */
  int lenses_cnt;		/* counter for lenses activity */
  int magnify_cnt;		/* counter for magnifier activity */
  int wind_cnt;			/* counter for wind activity */
  int android_move_cnt;		/* counter for android movement */
  int android_clone_cnt;	/* counter for android cloning */
  int ball_cnt;			/* counter for ball activity */
  int ball_pos;			/* counter for ball array position */
  int eater_pos;		/* counter for eater array position */
  int shine_cnt;		/* counter for emerald/diamond shining */

  int num_eater_arrays;		/* number of eater data arrays used */
  int num_ball_arrays;		/* number of ball data arrays used */

  int home_initial;		/* number of players (initial) */
  int home;			/* number of players not yet at home */
				/* 0 == all players at home */

  boolean testmode;		/* test mode */
  boolean infinite;		/* flag for infinitely wide cave */
  boolean infinite_true;	/* flag for truely infinitely wide cave */
  boolean killed_out_of_time;	/* kill player due to time out */

  int exit_x, exit_y;		/* kludge for playing player exit sound */

  short eater_array[8][9];		/* eater data */
  short ball_array[8][8];		/* ball data */
  short android_array[GAME_TILE_MAX];	/* android clone data */

  short cavebuf[CAVE_BUFFER_WIDTH][CAVE_BUFFER_HEIGHT];
  short nextbuf[CAVE_BUFFER_WIDTH][CAVE_BUFFER_HEIGHT];
  short drawbuf[CAVE_BUFFER_WIDTH][CAVE_BUFFER_HEIGHT];
  short boombuf[CAVE_BUFFER_WIDTH][CAVE_BUFFER_HEIGHT];

  short *cavecol[CAVE_BUFFER_WIDTH];
  short *nextcol[CAVE_BUFFER_WIDTH];
  short *drawcol[CAVE_BUFFER_WIDTH];
  short *boomcol[CAVE_BUFFER_WIDTH];

  short **cave;
  short **next;
  short **draw;
  short **boom;
};

#endif	// EMERALD_H
