#ifndef MAIN_EM_H
#define MAIN_EM_H

#include "../engines.h"


/* 2000-07-30T11:06:03Z ---------------------------------------------------- */

#define EM_MAX_CAVE_WIDTH		102
#define EM_MAX_CAVE_HEIGHT		102

/* define these for backwards compatibility */
#define EM_ENGINE_BAD_ROLL
#define EM_ENGINE_BAD_SPRING


/*
  -----------------------------------------------------------------------------
  definition of elements used in the Emerald Mine Club engine;
  the element names have the following properties:
  - elements that start with an 'X' can be stored in a level file;
  - elements that start with an 'Y' indicate moving elements;
  - elements that end with a 'B' are the "backside" of a moving element.
  -----------------------------------------------------------------------------
*/

enum
{
  Xblank = 0,		/* still */
  Yacid_splash_eB,	/* hmm */
  Yacid_splash_wB,	/* hmm */

#ifdef EM_ENGINE_BAD_ROLL
  Xstone_force_e,	/* only use these in eater */
  Xstone_force_w,
  Xnut_force_e,
  Xnut_force_w,
  Xspring_force_e,
  Xspring_force_w,
  Xemerald_force_e,
  Xemerald_force_w,
  Xdiamond_force_e,
  Xdiamond_force_w,
  Xbomb_force_e,
  Xbomb_force_w,
#endif

  Xstone,
  Xstone_pause,
  Xstone_fall,
  Ystone_s,
  Ystone_sB,
  Ystone_e,
  Ystone_eB,
  Ystone_w,
  Ystone_wB,
  Xnut,
  Xnut_pause,
  Xnut_fall,
  Ynut_s,
  Ynut_sB,
  Ynut_e,
  Ynut_eB,
  Ynut_w,
  Ynut_wB,
  Xbug_n,
  Xbug_e,
  Xbug_s,
  Xbug_w,
  Xbug_gon,
  Xbug_goe,
  Xbug_gos,
  Xbug_gow,
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
  Xtank_n,
  Xtank_e,
  Xtank_s,
  Xtank_w,
  Xtank_gon,
  Xtank_goe,
  Xtank_gos,
  Xtank_gow,
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
  Xandroid,
  Xandroid_1_n,
  Xandroid_2_n,
  Xandroid_1_e,
  Xandroid_2_e,
  Xandroid_1_w,
  Xandroid_2_w,
  Xandroid_1_s,
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
  Yspring_kill_e,
  Yspring_kill_eB,
  Yspring_kill_w,
  Yspring_kill_wB,
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
  Yemerald_eat,
  Yemerald_stone,
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
  Ydiamond_eat,
  Ydiamond_stone,
  Xdrip_fall,
  Xdrip_stretch,
  Xdrip_stretchB,
  Xdrip_eat,
  Ydrip_s1,
  Ydrip_s1B,
  Ydrip_s2,
  Ydrip_s2B,
  Xbomb,
  Xbomb_pause,
  Xbomb_fall,
  Ybomb_s,
  Ybomb_sB,
  Ybomb_e,
  Ybomb_eB,
  Ybomb_w,
  Ybomb_wB,
  Ybomb_eat,
  Xballoon,
  Yballoon_n,
  Yballoon_nB,
  Yballoon_e,
  Yballoon_eB,
  Yballoon_s,
  Yballoon_sB,
  Yballoon_w,
  Yballoon_wB,
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
  Xacid_ne,
  Xacid_se,
  Xacid_s,
  Xacid_sw,
  Xacid_nw,
  Xacid_1,
  Xacid_2,
  Xacid_3,
  Xacid_4,
  Xacid_5,
  Xacid_6,
  Xacid_7,
  Xacid_8,
  Xball_1,
  Xball_1B,
  Xball_2,
  Xball_2B,
  Yball_eat,

#if 1
  Ykey_1_eat,
  Ykey_2_eat,
  Ykey_3_eat,
  Ykey_4_eat,
  Ykey_5_eat,
  Ykey_6_eat,
  Ykey_7_eat,
  Ykey_8_eat,
  Ylenses_eat,
  Ymagnify_eat,
  Ygrass_eat,
  Ydirt_eat,
#endif

  Xgrow_ns,
  Ygrow_ns_eat,
  Xgrow_ew,
  Ygrow_ew_eat,
  Xwonderwall,
  XwonderwallB,
  Xamoeba_1,
  Xamoeba_2,
  Xamoeba_3,
  Xamoeba_4,
  Xamoeba_5,
  Xamoeba_6,
  Xamoeba_7,
  Xamoeba_8,
  Xdoor_1,
  Xdoor_2,
  Xdoor_3,
  Xdoor_4,
  Xdoor_5,
  Xdoor_6,
  Xdoor_7,
  Xdoor_8,
  Xkey_1,
  Xkey_2,
  Xkey_3,
  Xkey_4,
  Xkey_5,
  Xkey_6,
  Xkey_7,
  Xkey_8,
  Xwind_n,
  Xwind_e,
  Xwind_s,
  Xwind_w,
  Xwind_nesw,
  Xwind_stop,
  Xexit,
  Xexit_1,
  Xexit_2,
  Xexit_3,
  Xdynamite,
  Ydynamite_eat,
  Xdynamite_1,
  Xdynamite_2,
  Xdynamite_3,
  Xdynamite_4,
  Xbumper,
  XbumperB,
  Xwheel,
  XwheelB,
  Xswitch,
  XswitchB,
  Xsand,
  Xsand_stone,
  Xsand_stonein_1,
  Xsand_stonein_2,
  Xsand_stonein_3,
  Xsand_stonein_4,
  Xsand_stonesand_1,
  Xsand_stonesand_2,
  Xsand_stonesand_3,
  Xsand_stonesand_4,
  Xsand_stoneout_1,
  Xsand_stoneout_2,
  Xsand_sandstone_1,
  Xsand_sandstone_2,
  Xsand_sandstone_3,
  Xsand_sandstone_4,
  Xplant,
  Yplant,
  Xlenses,
  Xmagnify,
  Xdripper,
  XdripperB,
  Xfake_blank,
  Xfake_blankB,
  Xfake_grass,
  Xfake_grassB,
  Xfake_door_1,
  Xfake_door_2,
  Xfake_door_3,
  Xfake_door_4,
  Xfake_door_5,
  Xfake_door_6,
  Xfake_door_7,
  Xfake_door_8,
  Xsteel_1,
  Xsteel_2,
  Xsteel_3,
  Xsteel_4,
  Xwall_1,
  Xwall_2,
  Xwall_3,
  Xwall_4,
  Xround_wall_1,
  Xround_wall_2,
  Xround_wall_3,
  Xround_wall_4,
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
  Xalpha_quote,
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

  Xboom_bug,		/* passed from explode to synchro (linked explosion);
			   transition to explode_normal */
  Xboom_bomb,		/* passed from explode to synchro (linked explosion);
			   transition to explode_normal */
  Xboom_android,	/* passed from explode to synchro;
			   transition to boom_2 */
  Xboom_1,		/* passed from explode to synchro;
			   transition to boom_2 */
  Xboom_2,		/* transition to boom[] */

  Znormal,		/* passed from synchro to explode, only in next[];
			   no picture */
  Zdynamite,		/* passed from synchro to explode, only in next[];
			   no picture */
  Zplayer,		/* special code to indicate player;
			   no picture */
  ZBORDER,		/* special code to indicate border;
			   no picture */

  TILE_MAX
};

enum
{
  SPR_still = 0,
  SPR_walk  = 1,
  SPR_push  = 5,
  SPR_spray = 9,

  SPR_MAX   = 13
};

enum
{
  SAMPLE_blank = 0,	/* player walks on blank */
  SAMPLE_roll,		/* player pushes stone/bomb/nut/spring */
  SAMPLE_stone,		/* stone hits ground */
  SAMPLE_nut,		/* nut hits ground */
  SAMPLE_crack,		/* stone hits nut */
  SAMPLE_bug,		/* bug moves */
  SAMPLE_tank,		/* tank moves */
  SAMPLE_android,	/* android places something */
  SAMPLE_spring,	/* spring hits ground/wall/bumper, stone hits spring */
  SAMPLE_slurp,		/* spring kills alien */
  SAMPLE_eater,		/* eater sits/eats diamond */
  SAMPLE_alien,		/* alien moves */
  SAMPLE_collect,	/* player collects object */
  SAMPLE_diamond,	/* diamond/emerald hits ground */
  SAMPLE_squash,	/* stone squashes diamond */
  SAMPLE_wonderfall,	/* object falls thru wonderwall */
  SAMPLE_drip,		/* drip hits ground */
  SAMPLE_push,		/* player pushes balloon/android */
  SAMPLE_dirt,		/* player walks on dirt */
  SAMPLE_acid,		/* acid splashes */
  SAMPLE_ball,		/* ball places something */
  SAMPLE_grow,		/* growing wall grows */
  SAMPLE_wonder,	/* wonderwall is active */
  SAMPLE_door,		/* player goes thru door (gate) */
  SAMPLE_exit,		/* player goes into exit */
  SAMPLE_dynamite,	/* player places dynamite */
  SAMPLE_tick,		/* dynamite ticks */
  SAMPLE_press,		/* player presses wheel/wind/switch */
  SAMPLE_wheel,		/* wheel moves */
  SAMPLE_boom,		/* explosion */
  SAMPLE_time,		/* time runs out */
  SAMPLE_die,		/* player dies */

  SAMPLE_MAX
};

struct LEVEL
{
  unsigned int home_initial;          /* number of players (initial) */
  unsigned int home;                  /* number of players not yet at home */
                                      /* 0 == all players at home */

  unsigned int width;                 /* playfield width */
  unsigned int height;                /* playfield height */

  unsigned int time_seconds;          /* available time (seconds) */
  unsigned int time_initial;          /* available time (initial) */
  unsigned int time;                  /* time remaining (runtime) */

  unsigned int required_initial;      /* emeralds needed (initial) */
  unsigned int required;              /* emeralds needed (runtime) */

  unsigned int score;                 /* score */

  /* fill in all below /every/ time you read a level */
  unsigned int alien_score;           /* alien popped by stone/spring score */
  unsigned int amoeba_time;           /* amoeba speed */
  unsigned int android_move_cnt_initial; /* android move time counter (initial) */
  unsigned int android_move_cnt;      /* android move time counter */
  unsigned int android_move_time;     /* android move reset time */
  unsigned int android_clone_cnt_initial; /* android clone time counter (initial) */
  unsigned int android_clone_cnt;     /* android clone time counter */
  unsigned int android_clone_time;    /* android clone reset time */
  unsigned int ball_cnt;              /* ball time counter */
  unsigned int ball_pos;              /* ball array pos counter */
  unsigned int ball_random;           /* ball is random flag */
  unsigned int ball_state_initial;    /* ball currently on flag (initial) */
  unsigned int ball_state;            /* ball currently on flag */
  unsigned int ball_time;             /* ball reset time */
  unsigned int bug_score;             /* bug popped by stone/spring score */
  unsigned int diamond_score;         /* diamond collect score */
  unsigned int dynamite_score;        /* dynamite collect scoer*/
  unsigned int eater_pos;             /* eater array pos */
  unsigned int eater_score;           /* eater popped by stone/spring score */
  unsigned int emerald_score;         /* emerald collect score */
  unsigned int exit_score;            /* exit score */
  unsigned int key_score;             /* key collect score */
  unsigned int lenses_cnt_initial;    /* lenses time counter (initial) */
  unsigned int lenses_cnt;            /* lenses time counter */
  unsigned int lenses_score;          /* lenses collect score */
  unsigned int lenses_time;           /* lenses reset time */
  unsigned int magnify_cnt_initial;   /* magnify time counter (initial) */
  unsigned int magnify_cnt;           /* magnify time counter */
  unsigned int magnify_score;         /* magnify collect score */
  unsigned int magnify_time;          /* magnify reset time */
  unsigned int nut_score;             /* nut crack score */
  unsigned int shine_cnt;             /* shine counter for emerald/diamond */
  unsigned int slurp_score;           /* slurp alien score */
  unsigned int tank_score;            /* tank popped by stone/spring */
  unsigned int wheel_cnt_initial;     /* wheel time counter (initial) */
  unsigned int wheel_cnt;             /* wheel time counter */
  unsigned int wheel_x_initial;       /* wheel x pos (initial) */
  unsigned int wheel_x;               /* wheel x pos */
  unsigned int wheel_y_initial;       /* wheel y pos (initial) */
  unsigned int wheel_y;               /* wheel y pos */
  unsigned int wheel_time;            /* wheel reset time */
  unsigned int wind_cnt_initial;      /* wind time counter (initial) */
  unsigned int wind_cnt;              /* wind time counter */
  unsigned int wind_direction_initial;/* wind direction (initial) */
  unsigned int wind_direction;        /* wind direction */
  unsigned int wind_time;             /* wind reset time */
  unsigned int wonderwall_state_initial; /* wonderwall currently on flag (initial) */
  unsigned int wonderwall_state;      /* wonderwall currently on flag */
  unsigned int wonderwall_time_initial;/* wonderwall time (initial) */
  unsigned int wonderwall_time;       /* wonderwall time */
  unsigned short eater_array[8][9];   /* eater data */
  unsigned short ball_array[8][8];    /* ball data */
  unsigned short android_array[TILE_MAX]; /* android clone table */
};

struct PLAYER
{
  unsigned int num;
  unsigned int alive_initial;
  unsigned int alive;

  unsigned int dynamite;
  unsigned int dynamite_cnt;
  unsigned int keys;
  unsigned int anim;

  unsigned int x_initial;
  unsigned int y_initial;
  unsigned int x;
  unsigned int y;
  unsigned int oldx;
  unsigned int oldy;

  unsigned joy_n:1;
  unsigned joy_e:1;
  unsigned joy_s:1;
  unsigned joy_w:1;
  unsigned joy_fire:1;
  unsigned joy_stick:1;
  unsigned joy_spin:1;
};


/* ------------------------------------------------------------------------- */
/* definitions and structures for use by the main game functions             */
/* ------------------------------------------------------------------------- */

/* values for native Emerald Mine game version */
#define FILE_VERSION_EM_V3	3
#define FILE_VERSION_EM_V4	4
#define FILE_VERSION_EM_V5	5
#define FILE_VERSION_EM_V6	6

#define FILE_VERSION_EM_ACTUAL	FILE_VERSION_EM_V6

struct LevelInfo_EM
{
  int file_version;

  struct LEVEL *lev;
  struct PLAYER *ply1, *ply2;

  unsigned short cave[EM_MAX_CAVE_WIDTH][EM_MAX_CAVE_HEIGHT];
};

struct GraphicInfo_EM
{
  Bitmap *bitmap;
  int src_x, src_y;
  int src_offset_x, src_offset_y;
  int dst_offset_x, dst_offset_y;
  int width, height;

  boolean has_crumbled_graphics;
  Bitmap *crumbled_bitmap;
  int crumbled_src_x, crumbled_src_y;
  int crumbled_border_size;

  int unique_identifier;	/* used to identify needed screen updates */
};

#endif	/* MAIN_EM_H */
