/* 2008-08-27 04:17:36
 *
 * handle binary emc cave file format
 */

#include "main_em.h"


/* convert emc caves to intermediate cave format.
 *
 * the intermediate format supports most emc caves, but some internal
 * objects are not supported.
 *
 * different:
 * - active objects - use global flags instead of individual on/off
 * - falling or paused objects - become still objects
 * - sand states - become either sand with/without stone
 * - collected objects - become a common pause object
 * - drip states - become a normal drip
 *
 * unsupported:
 * - exploding objects (not in emc player either)
 *
 * pushed objects are supported in the cave, eaters and magic ball.
 * they behave almost the same as in the emc player, pushing over
 * steel wall, androids, players etc. a compile time option chooses
 * if pushed objects go into the acid.
 *
 * acid is handled completely different in my player. acid top is a
 * separate object, which can be put anywhere and the beasts will
 * be gobbled, even if they go in from below. in the emc player, an
 * acid top without a bottom becomes plant.
 *
 * borderless caves are supported, almost the same as in the emc
 * player. going off the left edge of the screen resulted in the
 * beast/player moving up 1 square (and the player disappeared).
 * going off the right edge of the screen, the beast/player would
 * go down 1 square. in my player, everything stays on the same y
 * coord, which is sensible, but breaks caves which expect the old
 * behaviour.
 */

#define GET_BE16(x)		((&x)[0] << 8 | (&x)[1])
#define PUT_BE16(x, y)		{(&x)[0] = (y) >> 8; (&x)[1] = (y) & 0xff;}

static const short map_emc[256] =
{
  Xstone,		Xstone,		Xdiamond,	Xdiamond,
  Xalien,		Xalien,		Xblank,		Xblank,
  Xtank_1_n,		Xtank_1_e,	Xtank_1_s,	Xtank_1_w,
  Xtank_2_n,		Xtank_2_e,	Xtank_2_s,	Xtank_2_w,

  Xbomb,		Xbomb,		Xemerald,	Xemerald,
  Xbug_1_n,		Xbug_1_e,	Xbug_1_s,	Xbug_1_w,
  Xbug_2_n,		Xbug_2_e,	Xbug_2_s,	Xbug_2_w,
  Xdrip,		Xdrip,		Xdrip,		Xdrip,

  Xstone,		Xbomb,		Xdiamond,	Xemerald,
  Xwonderwall,		Xnut,		Xnut,		Xnut,
  Xwheel,		Xeater_n,	Xeater_s,	Xeater_w,
  Xeater_e,		Xsand_stone,	Xblank,		Xblank,

  Xblank,		Xsand,		Xsand,		Xsand,
  Xsand_stone,		Xsand_stone,	Xsand_stone,	Xsand,
  Xstone,		Xslide_ew,	Xslide_ns,	Xdynamite_1,
  Xdynamite_2,		Xdynamite_3,	Xdynamite_4,	Xacid_s,

  Xexit_1,		Xexit_2,	Xexit_3,	Xballoon,
  Xplant,		Xspring,	Xspring_fall,	Xspring_w,
  Xspring_e,		Xball_1,	Xball_2,	Xandroid,
  Xblank,		Xandroid,	Xandroid,	Xandroid,

  Xandroid,		Xandroid,	Xandroid,	Xandroid,
  Xandroid,		Xblank,		Xblank,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,

  Xblank,		Xblank,		Xblank,		Xpush_spring_w,
  Xpush_spring_e,	Xacid_1,	Xacid_2,	Xacid_3,
  Xacid_4,		Xacid_5,	Xacid_6,	Xacid_7,
  Xacid_8,		Xblank,		Xblank,		Xblank,

  Xblank,		Xblank,		Xpush_nut_w,	Xpush_nut_e,
  Xsteel_1,		Xblank,		Xblank,		Xpush_bomb_w,
  Xpush_bomb_e,		Xpush_stone_w,	Xpush_stone_e,	Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,

  Xblank,		Xroundwall_1,	Xgrass,		Xsteel_1,
  Xwall_1,		Xkey_1,		Xkey_2,		Xkey_3,
  Xkey_4,		Xdoor_1,	Xdoor_2,	Xdoor_3,
  Xdoor_4,		Xfake_amoeba,	Xfake_door_1,	Xfake_door_2,

  Xfake_door_3,		Xfake_door_4,	Xwonderwall,	Xwheel,
  Xsand,		Xacid_nw,	Xacid_ne,	Xacid_sw,
  Xacid_se,		Xfake_blank,	Xamoeba_1,	Xamoeba_2,
  Xamoeba_3,		Xamoeba_4,	Xexit,		Xalpha_arrow_w,

  Xfake_grass,		Xlenses,	Xmagnify,	Xfake_blank,
  Xfake_grass,		Xswitch,	Xswitch,	Xblank,
  Xdecor_8,		Xdecor_9,	Xdecor_10,	Xdecor_5,
  Xalpha_comma,		Xalpha_quote,	Xalpha_minus,	Xdynamite,

  Xsteel_3,		Xdecor_6,	Xdecor_7,	Xsteel_2,
  Xroundwall_2,		Xdecor_2,	Xdecor_4,	Xdecor_3,
  Xwind_any,		Xwind_e,	Xwind_s,	Xwind_w,
  Xwind_n,		Xdirt,		Xplant,		Xkey_5,

  Xkey_6,		Xkey_7,		Xkey_8,		Xdoor_5,
  Xdoor_6,		Xdoor_7,	Xdoor_8,	Xbumper,
  Xalpha_a,		Xalpha_b,	Xalpha_c,	Xalpha_d,
  Xalpha_e,		Xalpha_f,	Xalpha_g,	Xalpha_h,

  Xalpha_i,		Xalpha_j,	Xalpha_k,	Xalpha_l,
  Xalpha_m,		Xalpha_n,	Xalpha_o,	Xalpha_p,
  Xalpha_q,		Xalpha_r,	Xalpha_s,	Xalpha_t,
  Xalpha_u,		Xalpha_v,	Xalpha_w,	Xalpha_x,

  Xalpha_y,		Xalpha_z,	Xalpha_0,	Xalpha_1,
  Xalpha_2,		Xalpha_3,	Xalpha_4,	Xalpha_5,
  Xalpha_6,		Xalpha_7,	Xalpha_8,	Xalpha_9,
  Xalpha_perio,		Xalpha_excla,	Xalpha_colon,	Xalpha_quest,

  Xalpha_arrow_e,	Xdecor_1,	Xfake_door_5,	Xfake_door_6,
  Xfake_door_7,		Xfake_door_8,	Xblank,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,
  Xblank,		Xblank,		Xalpha_copyr,	Xfake_acid_1
};

static void init_android_clone_table(short android_clone_bits)
{
  boolean android_emerald	= (android_clone_bits & 1)	!= 0;
  boolean android_diamond	= (android_clone_bits & 2)	!= 0;
  boolean android_stone		= (android_clone_bits & 4)	!= 0;
  boolean android_bomb		= (android_clone_bits & 8)	!= 0;
  boolean android_nut		= (android_clone_bits & 16)	!= 0;
  boolean android_tank		= (android_clone_bits & 32)	!= 0;
  boolean android_eater		= (android_clone_bits & 64)	!= 0;
  boolean android_bug		= (android_clone_bits & 128)	!= 0;
  boolean android_alien		= (android_clone_bits & 256)	!= 0;
  boolean android_spring	= (android_clone_bits & 512)	!= 0;
  boolean android_balloon	= (android_clone_bits & 1024)	!= 0;
  boolean android_amoeba	= (android_clone_bits & 2048)	!= 0;
  boolean android_dynamite	= (android_clone_bits & 4096)	!= 0;
  int i;

  for (i = 0; i < TILE_MAX; i++)
    cav.android_array[i] = Xblank;

  if (android_eater)
  {
    cav.android_array[Xeater_n]		= Xeater_n;
    cav.android_array[Yeater_nB]	= Xeater_n;

    cav.android_array[Xeater_e]		= Xeater_e;
    cav.android_array[Yeater_eB]	= Xeater_e;

    cav.android_array[Xeater_s]		= Xeater_s;
    cav.android_array[Yeater_sB]	= Xeater_s;

    cav.android_array[Xeater_w]		= Xeater_w;
    cav.android_array[Yeater_wB]	= Xeater_w;
  }

  if (android_alien)
  {
    cav.android_array[Xalien]		= Xalien;
    cav.android_array[Xalien_pause]	= Xalien;
    cav.android_array[Yalien_nB]	= Xalien;
    cav.android_array[Yalien_eB]	= Xalien;
    cav.android_array[Yalien_sB]	= Xalien;
    cav.android_array[Yalien_wB]	= Xalien;
  }

  if (android_bug)
  {
    cav.android_array[Xbug_1_n]		= Xbug_2_n;
    cav.android_array[Xbug_2_n]		= Xbug_2_n;
    cav.android_array[Ybug_nB]		= Xbug_2_n;
    cav.android_array[Ybug_n_e]		= Xbug_2_n;
    cav.android_array[Ybug_n_w]		= Xbug_2_n;

    cav.android_array[Xbug_1_e]		= Xbug_2_e;
    cav.android_array[Xbug_2_e]		= Xbug_2_e;
    cav.android_array[Ybug_eB]		= Xbug_2_e;
    cav.android_array[Ybug_e_s]		= Xbug_2_e;
    cav.android_array[Ybug_e_n]		= Xbug_2_e;

    cav.android_array[Xbug_1_s]		= Xbug_2_s;
    cav.android_array[Xbug_2_s]		= Xbug_2_s;
    cav.android_array[Ybug_sB]		= Xbug_2_s;
    cav.android_array[Ybug_s_w]		= Xbug_2_s;
    cav.android_array[Ybug_s_e]		= Xbug_2_s;

    cav.android_array[Xbug_1_w]		= Xbug_2_w;
    cav.android_array[Xbug_2_w]		= Xbug_2_w;
    cav.android_array[Ybug_wB]		= Xbug_2_w;
    cav.android_array[Ybug_w_n]		= Xbug_2_w;
    cav.android_array[Ybug_w_s]		= Xbug_2_w;
  }

  if (android_tank)
  {
    cav.android_array[Xtank_1_n]	= Xtank_1_n;
    cav.android_array[Xtank_2_n]	= Xtank_1_n;
    cav.android_array[Ytank_nB]		= Xtank_1_n;
    cav.android_array[Ytank_n_e]	= Xtank_1_n;
    cav.android_array[Ytank_n_w]	= Xtank_1_n;

    cav.android_array[Xtank_1_e]	= Xtank_1_e;
    cav.android_array[Xtank_2_e]	= Xtank_1_e;
    cav.android_array[Ytank_eB]		= Xtank_1_e;
    cav.android_array[Ytank_e_s]	= Xtank_1_e;
    cav.android_array[Ytank_e_n]	= Xtank_1_e;

    cav.android_array[Xtank_1_s]	= Xtank_1_s;
    cav.android_array[Xtank_2_s]	= Xtank_1_s;
    cav.android_array[Ytank_sB]		= Xtank_1_s;
    cav.android_array[Ytank_s_w]	= Xtank_1_s;
    cav.android_array[Ytank_s_e]	= Xtank_1_s;

    cav.android_array[Xtank_1_w]	= Xtank_1_w;
    cav.android_array[Xtank_2_w]	= Xtank_1_w;
    cav.android_array[Ytank_wB]		= Xtank_1_w;
    cav.android_array[Ytank_w_n]	= Xtank_1_w;
    cav.android_array[Ytank_w_s]	= Xtank_1_w;
  }

  if (android_emerald)
  {
    cav.android_array[Xemerald]		= Xemerald;
    cav.android_array[Xemerald_pause]	= Xemerald;
    cav.android_array[Xemerald_fall]	= Xemerald;
    cav.android_array[Yemerald_sB]	= Xemerald;
    cav.android_array[Yemerald_eB]	= Xemerald;
    cav.android_array[Yemerald_wB]	= Xemerald;
  }

  if (android_diamond)
  {
    cav.android_array[Xdiamond]		= Xdiamond;
    cav.android_array[Xdiamond_pause]	= Xdiamond;
    cav.android_array[Xdiamond_fall]	= Xdiamond;
    cav.android_array[Ydiamond_sB]	= Xdiamond;
    cav.android_array[Ydiamond_eB]	= Xdiamond;
    cav.android_array[Ydiamond_wB]	= Xdiamond;
  }

  if (android_stone)
  {
    cav.android_array[Xstone]		= Xstone;
    cav.android_array[Xstone_pause]	= Xstone;
    cav.android_array[Xstone_fall]	= Xstone;
    cav.android_array[Ystone_sB]	= Xstone;
    cav.android_array[Ystone_eB]	= Xstone;
    cav.android_array[Ystone_wB]	= Xstone;
  }

  if (android_bomb)
  {
    cav.android_array[Xbomb]		= Xbomb;
    cav.android_array[Xbomb_pause]	= Xbomb;
    cav.android_array[Xbomb_fall]	= Xbomb;
    cav.android_array[Ybomb_sB]		= Xbomb;
    cav.android_array[Ybomb_eB]		= Xbomb;
    cav.android_array[Ybomb_wB]		= Xbomb;
  }

  if (android_nut)
  {
    cav.android_array[Xnut]		= Xnut;
    cav.android_array[Xnut_pause]	= Xnut;
    cav.android_array[Xnut_fall]	= Xnut;
    cav.android_array[Ynut_sB]		= Xnut;
    cav.android_array[Ynut_eB]		= Xnut;
    cav.android_array[Ynut_wB]		= Xnut;
  }

  if (android_spring)
  {
    cav.android_array[Xspring]		= Xspring;
    cav.android_array[Xspring_pause]	= Xspring;
    cav.android_array[Xspring_fall]	= Xspring;
    cav.android_array[Xspring_e]	= Xspring;
    cav.android_array[Xspring_w]	= Xspring;
    cav.android_array[Yspring_sB]	= Xspring;
    cav.android_array[Yspring_eB]	= Xspring;
    cav.android_array[Yspring_wB]	= Xspring;
    cav.android_array[Yspring_alien_eB]	= Xspring;
    cav.android_array[Yspring_alien_wB]	= Xspring;
  }

  if (android_dynamite)
  {
    cav.android_array[Xdynamite]	= Xdynamite;
  }

  if (android_balloon)
  {
    cav.android_array[Xballoon]		= Xballoon;
    cav.android_array[Yballoon_nB]	= Xballoon;
    cav.android_array[Yballoon_eB]	= Xballoon;
    cav.android_array[Yballoon_sB]	= Xballoon;
    cav.android_array[Yballoon_wB]	= Xballoon;
  }

  if (android_amoeba)
  {
    cav.android_array[Xfake_amoeba]	= Xdrip;
    cav.android_array[Yfake_amoeba]	= Xdrip;
    cav.android_array[Xamoeba_1]	= Xdrip;
    cav.android_array[Xamoeba_2]	= Xdrip;
    cav.android_array[Xamoeba_3]	= Xdrip;
    cav.android_array[Xamoeba_4]	= Xdrip;
    cav.android_array[Xamoeba_5]	= Xdrip;
    cav.android_array[Xamoeba_6]	= Xdrip;
    cav.android_array[Xamoeba_7]	= Xdrip;
    cav.android_array[Xamoeba_8]	= Xdrip;
  }
}

void convert_em_level(unsigned char *src, int file_version)
{
  static int eater_offset[8] =
  {
    2048, 2057, 2066, 2075,
    2112, 2121, 2130, 2139
  };
  int i, x, y, temp;

  /* common to all emc caves */

  cav.time_seconds = GET_BE16(src[2110]);
  if (cav.time_seconds > 9999)
    cav.time_seconds = 9999;

  cav.required_initial = src[2095];

  /* scores */

  cav.emerald_score	= src[2084];
  cav.diamond_score	= src[2085];
  cav.alien_score	= src[2086];
  cav.tank_score	= src[2087];
  cav.bug_score		= src[2088];
  cav.eater_score	= src[2089];
  cav.nut_score		= src[2090];
  cav.dynamite_score	= src[2091];
  cav.key_score		= src[2092];
  cav.exit_score	= src[2093] * 8 / 5;

  cav.lenses_score	= src[2151];
  cav.magnify_score	= src[2152];
  cav.slurp_score	= src[2153];

  /* times */

  cav.android_move_time  = GET_BE16(src[2164]);
  cav.android_clone_time = GET_BE16(src[2166]);
  cav.ball_time		 = GET_BE16(src[2160]);

  cav.lenses_time	= GET_BE16(src[2154]);
  cav.magnify_time	= GET_BE16(src[2156]);
  cav.wheel_time	= GET_BE16(src[2104]);

  temp = GET_BE16(src[2100]) * 28;
  if (temp > 9999)
    temp = 9999;
  cav.amoeba_time = temp;

  cav.wonderwall_time_initial = GET_BE16(src[2102]);

  cav.wind_cnt_initial = src[2149] & 15 ? cav.wind_time : 0;
  temp = src[2149];
  cav.wind_direction_initial = (temp & 8 ? 0 :
				temp & 1 ? 1 :
				temp & 2 ? 2 :
				temp & 4 ? 3 : 0);
  /* global flags */

  cav.ball_random	 = src[2162] & 1   ? 1 : 0;
  cav.ball_state_initial = src[2162] & 128 ? 1 : 0;

  for (temp = 1; temp < 2047; temp++)
  {
    switch (src[temp])
    {
      case 36:					/* wonderwall */
	cav.wonderwall_state_initial = 1;
	cav.wonderwall_time_initial = 9999;
	break;

      case 40:					/* wheel */
	cav.wheel_x_initial = temp & 63;
	cav.wheel_y_initial = temp >> 6;
	cav.wheel_cnt_initial = cav.wheel_time;
	break;

      case 163:					/* fake blank */
	cav.lenses_cnt_initial = 9999;
	break;

      case 164:					/* fake grass */
	cav.magnify_cnt_initial = 9999;
	break;
    }
  }

  /* android */

  temp = GET_BE16(src[2168]);

  init_android_clone_table(temp);

  /* eaters */

  for (i = 0; i < 8; i++)
    for (x = 0; x < 9; x++)
      cav.eater_array[i][x] = map_emc[src[eater_offset[i] + x]];

  /* ball */

  temp = map_emc[src[2159]];

  for (y = 0; y < 8; y++)
  {
    if (src[2162] & 1)
    {
      for (x = 0; x < 8; x++)
	cav.ball_array[y][x] = temp;
    }
    else
    {
      cav.ball_array[y][1] = (src[2163] & 1)  ? temp : Xblank; /* north */
      cav.ball_array[y][6] = (src[2163] & 2)  ? temp : Xblank; /* south */
      cav.ball_array[y][3] = (src[2163] & 4)  ? temp : Xblank; /* west */
      cav.ball_array[y][4] = (src[2163] & 8)  ? temp : Xblank; /* east */
      cav.ball_array[y][7] = (src[2163] & 16) ? temp : Xblank; /* southeast */
      cav.ball_array[y][5] = (src[2163] & 32) ? temp : Xblank; /* southwest */
      cav.ball_array[y][2] = (src[2163] & 64) ? temp : Xblank; /* northeast */
      cav.ball_array[y][0] = (src[2163] & 128)? temp : Xblank; /* northwest */
    }
  }

  /* players */

  for (i = 0; i < 2; i++)
  {
    temp = GET_BE16(src[2096 + i * 2]);

    cav.player_x[i] = (temp & 63);
    cav.player_y[i] = (temp >> 6 & 31);
  }

  /* cave */

  /* first fill the complete playfield with the default border element */
  for (y = 0; y < CAVE_HEIGHT; y++)
    for (x = 0; x < CAVE_WIDTH; x++)
      cav.cave_raw[x][y] = Zborder;

  /* then copy the real level contents from level file into the playfield */
  temp = 0;
  for (y = 0; y < cav.height; y++)
    for (x = 0; x < cav.width; x++)
      cav.cave_raw[x][y] = map_emc[src[temp++]];

  /* at last, set the two players at their positions in the playfield */
  /* (native EM[C] levels always have exactly two players in a level) */
  for (i = 0; i < 2; i++)
    cav.cave_raw[cav.player_x[i]][cav.player_y[i]] = Zplayer;

  native_em_level.file_version = file_version;
}


/* convert all emerald mine caves to emc version 6 cave format.
 *
 * caves are filtered to get rid of invalid or unsupported tiles.
 *
 * although the result is a somewhat clean cave, it is meant only
 * to give a common structure for the binary cave format. it is not
 * for archiving purposes (it is better to keep the raw cave as-is)
 * and it is not meant for serializing (the intermediate structure
 * is better defined).
 *
 * acid top is added to acid bottom in both the cave and the eaters.
 * fake acid (only in v4) does not exist because it adds nothing to
 * the game, and is different even in different versions of the emc
 * player.
 *
 * v4/v5 time is converted to 10x old time (as it should be).
 * the reason the kingsoft player used 5x time was copy protection.
 *
 * note: emc v6 converter has an error in converting v4 eaters to the
 * wrong bug(24 instead of 20) and tank(12 instead of 8).
 */

/* changes for game engine integration in Rocks'n'Diamonds:
 *
 * cave versions below V5 had no grass, but only sand/dirt
 * - object code 130 (V6 grass) is changed to 189 (V6 dirt)
 * - object codes are changed in both cave and eater arrays
 * - only graphical change, as both objects behave the same
 */

static const unsigned char map_v6[256] =
{
  /* filter for v6 */

  0,0,2,2,         4,4,118,118,     8,9,10,11,       12,13,14,15,
  16,16,18,18,     20,21,22,23,     24,25,26,27,     28,28,118,28,
  0,16,2,18,       36,37,37,37,     40,41,42,43,     44,45,128,128,
  128,148,148,     148,45,45,45,    148,0,57,58,     59,60,61,62,63,

  64,65,66,67,     68,69,69,71,     72,73,74,75,     118,75,75,75,
  75,75,75,75,     75,153,153,153,  153,153,153,153, 153,153,153,153,
  153,153,153,99,  100,68,68,68,    68,68,68,68,     68,118,118,118,
  118,118,114,115, 131,118,118,119, 120,121,122,118, 118,118,118,118,

  128,129,130,131, 132,133,134,135, 136,137,138,139, 140,141,142,143,
  144,145,146,147, 148,149,150,151, 152,153,154,155, 156,157,158,159,
  160,161,162,163, 164,165,165,118, 168,169,170,171, 172,173,174,175,
  176,177,178,179, 180,181,182,183, 184,185,186,187, 188,189,68,191,

  192,193,194,195, 196,197,198,199, 200,201,202,203, 204,205,206,207,
  208,209,210,211, 212,213,214,215, 216,217,218,219, 220,221,222,223,
  224,225,226,227, 228,229,230,231, 232,233,234,235, 236,237,238,239,
  240,241,242,243, 244,245,153,153, 153,153,153,153, 153,153,153,153
};

static const unsigned char map_v5[256] =
{
  /* filter for v5 */

  0,0,2,2,         4,4,118,118,     8,9,10,11,       12,13,14,15,
  16,16,18,18,     20,21,22,23,     24,25,26,27,     28,28,118,28,
  0,16,2,18,       36,37,37,37,     147,41,42,43,    44,45,128,128,
  128,148,148,148, 45,45,45,148,    0,57,58,59,      60,61,62,63,

  64,65,66,67,     68,153,153,153,  153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,68,68,68,    68,68,68,68,     68,118,118,118,
  118,118,114,115, 131,118,118,119, 120,121,122,118, 118,118,118,118,

  128,129,130,131, 132,133,134,135, 136,137,138,139, 140,141,142,143,
  144,145,146,147, 148,149,150,151, 152,153,154,155, 156,157,158,159,
  160,153,153,153, 153,153,153,118, 168,169,170,171, 172,173,174,175,
  176,177,178,179, 180,181,182,183, 184,185,186,187, 188,189,68,153,

  153,153,153,153, 153,153,153,153, 200,201,202,203, 204,205,206,207,
  208,209,210,211, 212,213,214,215, 216,217,218,219, 220,221,222,223,
  224,225,226,227, 228,229,230,231, 232,233,234,235, 236,237,238,239,
  240,241,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153
};

static const unsigned char map_v4[256] =
{
  /* filter for v4 */

  0,0,2,2,         4,4,118,118,     8,9,10,11,       12,13,14,15,
  16,16,18,18,     20,21,22,23,     24,25,26,27,     28,28,118,28,
  0,16,2,18,       36,37,37,37,     147,41,42,43,    44,45,128,128,
  128,148,148,148, 45,45,45,148,    0,153,153,59,    60,61,62,63,

  64,65,66,153,    153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,118,114,115, 131,118,118,119, 120,121,122,118, 118,118,118,118,

  128,129,189,131, 132,133,134,135, 136,137,138,139, 140,141,142,143,
  144,145,146,147, 148,149,150,151, 152,68,154,155,  156,157,158,160,
  160,160,160,160, 160,160,160,160, 160,160,160,160, 160,160,160,175,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,68,153,

  153,153,153,153, 153,153,153,153, 200,201,202,203, 204,205,206,207,
  208,209,210,211, 212,213,214,215, 216,217,218,219, 220,221,222,223,
  224,225,226,227, 228,229,230,231, 232,233,234,235, 236,237,238,239,
  240,241,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153
};

static const unsigned char map_v4_eater[32] =
{
  /* filter for v4 eater */

  128,18,2,0,      4,8,16,20,       28,37,41,45,     189,129,131,132,
  133,134,135,136, 146,147,175,65,  66,64,2,18,      128,128,128,128
};

static boolean filename_has_v1_format(char *filename)
{
  char *basename = getBaseNamePtr(filename);

  return (strlen(basename) == 3 &&
	  basename[0] == 'a' &&
	  basename[1] >= 'a' && basename[1] <= 'k' &&
	  basename[2] >= '0' && basename[2] <= '9');
}

int cleanup_em_level(unsigned char *src, int length, char *filename)
{
  int file_version = FILE_VERSION_EM_UNKNOWN;
  int i;

  if (length >= 2172 &&
      src[2106] == 255 &&		/* version id: */
      src[2107] == 54 &&		/* '6' */
      src[2108] == 48 &&		/* '0' */
      src[2109] == 48)			/* '0' */
  {
    /* ---------- this cave has V6 file format ---------- */
    file_version = FILE_VERSION_EM_V6;

    /* remap elements to internal EMC level format */
    for (i = 0; i < 2048; i++)
      src[i] = map_v6[src[i]];
    for (i = 2048; i < 2084; i++)
      src[i] = map_v6[src[i]];
    for (i = 2112; i < 2148; i++)
      src[i] = map_v6[src[i]];
  }
  else if (length >= 2110 &&
	   src[2106] == 255 &&		/* version id: */
	   src[2107] == 53 &&		/* '5' */
	   src[2108] == 48 &&		/* '0' */
	   src[2109] == 48)		/* '0' */
  {
    /* ---------- this cave has V5 file format ---------- */
    file_version = FILE_VERSION_EM_V5;

    /* remap elements to internal EMC level format */
    for (i = 0; i < 2048; i++)
      src[i] = map_v5[src[i]];
    for (i = 2048; i < 2084; i++)
      src[i] = map_v5[src[i]];
    for (i = 2112; i < 2148; i++)
      src[i] = src[i - 64];
  }
  else if (length >= 2106 &&
	   (src[1983] == 27 ||		/* encrypted (only EM I/II/III) */
	    src[1983] == 116 ||		/* unencrypted (usual case) */
	    src[1983] == 131))		/* unencrypted (rare case) */
  {
    /* ---------- this cave has V1, V2 or V3 file format ---------- */

    boolean fix_copyright = FALSE;

    /*
      byte at position 1983 (0x07bf) is used as "magic byte":
      - 27  (0x1b)	=> encrypted level (V3 only / Kingsoft original games)
      - 116 (0x74)	=> unencrypted level (byte is corrected to 131 (0x83))
      - 131 (0x83)	=> unencrypted level (happens only in very rare cases)
    */

    if (src[1983] == 27)	/* (0x1b) -- after decryption: 116 (0x74) */
    {
      /* this is original (encrypted) Emerald Mine I, II or III level file */

      int first_byte = src[0];
      unsigned char code0 = 0x65;
      unsigned char code1 = 0x11;

      /* decode encrypted level data */
      for (i = 0; i < 2106; i++)
      {
	src[i] ^= code0;
	src[i] -= code1;

	code0 = (code0 + 7) & 0xff;
      }

      src[1] = 131;		/* needed for all Emerald Mine levels */

      /* first byte is either 0xf1 (EM I and III) or 0xf5 (EM II) */
      if (first_byte == 0xf5)
      {
	src[0] = 131;		/* only needed for Emerald Mine II levels */

	fix_copyright = TRUE;
      }

      /* ---------- this cave has V3 file format ---------- */
      file_version = FILE_VERSION_EM_V3;
    }
    else if (filename_has_v1_format(filename))
    {
      /* ---------- this cave has V1 file format ---------- */
      file_version = FILE_VERSION_EM_V1;
    }
    else
    {
      /* ---------- this cave has V2 file format ---------- */
      file_version = FILE_VERSION_EM_V2;
    }

    /* remap elements to internal EMC level format */
    for (i = 0; i < 2048; i++)
      src[i] = map_v4[src[i]];
    for (i = 2048; i < 2084; i++)
      src[i] = map_v4_eater[src[i] >= 28 ? 0 : src[i]];
    for (i = 2112; i < 2148; i++)
      src[i] = src[i - 64];

    if (fix_copyright)		/* fix "(c)" sign in Emerald Mine II levels */
    {
      for (i = 0; i < 2048; i++)
	if (src[i] == 241)
	  src[i] = 254;		/* replace 'Xdecor_1' with 'Xalpha_copyr' */
    }
  }
  else
  {
    /* ---------- this cave has unknown file format ---------- */

    /* if file has length of old-style level file, print (wrong) magic byte */
    if (length < 2110)
      Error(ERR_WARN, "unknown magic byte 0x%02x at position 0x%04x",
	    src[1983], 1983);

    return FILE_VERSION_EM_UNKNOWN;
  }

  if (file_version < FILE_VERSION_EM_V6)
  {
    /* id */
    src[2106] = 255;		/* version id: */
    src[2107] = 54;		/* '6' */
    src[2108] = 48;		/* '0' */
    src[2109] = 48;		/* '0' */

    /* time */
    i = src[2094] * 10;
    /* stored level time of levels for the V2 player was changed to 50% of the
       time for the V1 player (original V3 levels already considered this) */
    if (file_version != FILE_VERSION_EM_V1 &&
	file_version != FILE_VERSION_EM_V3)
      i /= 2;
    src[2110] = i >> 8;
    src[2111] = i;

    for (i = 2148; i < 2172; i++)
      src[i] = 0;

    /* ball data */
    src[2159] = 128;
  }

  /* ---------- at this stage, the cave data always has V6 format ---------- */

  /* fix wheel */
  for (i = 0; i < 2048; i++)
    if (src[i] == 40)
      break;
  for (i++; i < 2048; i++)
    if (src[i] == 40)
      src[i] = 147;

  /* fix acid */
  for (i = 64; i < 2048; i++)
    if (src[i] == 63)		/* replace element above 'Xacid_s' ... */
      src[i - 64] = 101;	/* ... with 'Xacid_1' */

  /* fix acid with no base beneath it (see below for details (*)) */
  for (i = 64; i < 2048 - 1; i++)
  {
    if (file_version <= FILE_VERSION_EM_V2 &&
	src[i - 64] == 101 && src[i] != 63)	/* acid without base */
    {
      if (src[i - 1] == 101 ||			/* remove acid over acid row */
	  src[i + 1] == 101)
	src[i - 64] = 6;	/* replace element above with 'Xblank' */
      else
	src[i - 64] = 255;	/* replace element above with 'Xfake_acid_1' */
    }
  }

  /* fix acid in eater 1 */
  for (i = 2051; i < 2057; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 2 */
  for (i = 2060; i < 2066; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 3 */
  for (i = 2069; i < 2075; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 4 */
  for (i = 2078; i < 2084; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 5 */
  for (i = 2115; i < 2121; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 6 */
  for (i = 2124; i < 2130; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 7 */
  for (i = 2133; i < 2139; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 8 */
  for (i = 2142; i < 2148; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* old style time */
  src[2094] = 0;

  /* player 1 pos */
  src[2096] &= 7;
  src[GET_BE16(src[2096])] = 128;

  /* player 2 pos */
  src[2098] &= 7;
  src[GET_BE16(src[2098])] = 128;

  /* amoeba speed */
  if (GET_BE16(src[2100]) > 9999)
    PUT_BE16(src[2100], 9999);

  /* time wonderwall */
  if (GET_BE16(src[2102]) > 9999)
    PUT_BE16(src[2102], 9999);

  /* time */
  if (GET_BE16(src[2110]) > 9999)
    PUT_BE16(src[2110], 9999);

  /* wind direction */
  i = src[2149];
  i &= 15;
  i &= -i;
  src[2149] = i;

  /* time lenses */
  if (GET_BE16(src[2154]) > 9999)
    PUT_BE16(src[2154], 9999);

  /* time magnify */
  if (GET_BE16(src[2156]) > 9999)
    PUT_BE16(src[2156], 9999);

  /* ball object */
  src[2158] = 0;
  src[2159] = map_v6[src[2159]];

  /* ball pause */
  if (GET_BE16(src[2160]) > 9999)
    PUT_BE16(src[2160], 9999);

  /* ball data */
  src[2162] &= 129;
  if (src[2162] & 1)
    src[2163] = 0;

  /* android move pause */
  if (GET_BE16(src[2164]) > 9999)
    PUT_BE16(src[2164], 9999);

  /* android clone pause */
  if (GET_BE16(src[2166]) > 9999)
    PUT_BE16(src[2166], 9999);

  /* android data */
  src[2168] &= 31;

  /* size of v6 cave */
  length = 2172;

  if (options.debug)
    Error(ERR_DEBUG, "EM level file version: %d", file_version);

  return file_version;
}

/*
2000-08-20 09:41:18
David Tritscher

structure of emerald mine level disk files
----------------------------------------------------------------------

if(len >= 2172 && (buf[2106] == 255 && buf[2107] == 54 && buf[2108] == 48 && buf[2109] == 48)) // v6
if(len >= 2110 && (buf[2106] == 255 && buf[2107] == 53 && buf[2108] == 48 && buf[2109] == 48)) // v5
if(len >= 2106 && (buf[1983] == 116 || buf[2047] == 116)) // v4
if(len >= 2106 && (buf[1983] == 27 || buf[2047] == 219)) // v3

buf[0]=241;buf[1]=248;for(i=0,j=101;i<2106;i++,j+=7)buf[i]=(buf[i]^j)-17; // decrypt

number of movements (calls to logic) = time * 50 / 8

{} reserved (but some broken levels use them)

----------------------------------------------------------------------

version 6

0: level
2048: eater exp 1
2057: eater exp 2
2066: eater exp 3
2075: eater exp 4
2084: emerald value
2085: diamond value
2086: alien value
2087: tank value
2088: bug value
2089: eater value
2090: nut value
2091: dynamite value
2092: key value
2093: bonus
2094
2095: emeralds needed
2096: player 1 pos
2098: player 2 pos
2100: ameuba speed
2102: wonderwall time
2104: wheel time
2106: ID (0xff363030)
2110: time
2112: eater exp 5
2121: eater exp 6
2130: eater exp 7
2139: eater exp 8
2148: flags bit#7=NOI #6=RIS
2149: wind direction bit#0=right #1=down #2=left #3=up
2150: cave number bit#7=teamwork
2151: lenses value
2152: magnify value
2153: spring value
2154: lenses time
2156: magnify time
2158
2159: ball object
2160: ball speed
2162: ball info bit#15=switch state #8=random
; bit#0=N #1=S #2=W #3=E #4=SE #5=SW #6=NE #7=NW
2164: android move speed
2166: android clone speed
2168: android data
; bit#0=emerald #1=diamond #2=stone #3=bomb #4=nut #5=tank #6=eater
; #7=bug #8=alien #9=spring #10=balloon #11=ameuba #12=dynamite
2170
2172: SIZE

0: stone
1: stone {stone_fall}
2: diamond
3: diamond {diamond_fall}
4: alien
5: alien {alien_pause}
6: pause {boom_1}
7: pause {boom_2}
8: tank_n_1
9: tank_e_1
10: tank_s_1
11: tank_w_1
12: tank_n_2
13: tank_e_2
14: tank_s_2
15: tank_w_2
16: bomb
17: bomb {bomb_fall}
18: emerald
19: emerald {emerald_fall}
20: bug_n_1
21: bug_e_1
22: bug_s_1
23: bug_w_1
24: bug_n_2
25: bug_e_2
26: bug_s_2
27: bug_w_2
28: drip
29: drip {drip_fall}
30: blank {drip_stretchB}
31: drip {drip_stretch}
32: stone {stone_pause}
33: bomb {bomb_pause}
34: diamond {diamond_pause}
35: emerald {emerald_pause}
36: wonderwall {wonderwallB}
37: nut
38: nut {nut_fall}
39: nut {nut_pause}
40: wheel {wheelB}
41: eater_n
42: eater_s
43: eater_w
44: eater_e
45: sand_stone
46: blank {sand_stonein_2}
47: blank {sand_stonein_3}
48: blank {sand_stonein_4}
49: sand {sand_stonesand_2}
50: sand {sand_stonesand_3}
51: sand {sand_stonesand_4}
52: sand_stone {sand_sandstone_2}
53: sand_stone {sand_sandstone_3}
54: sand_stone {sand_sandstone_4}
55: sand {sand_stonesand_4}
56: stone {sand_stoneout_2}
57: slide_ew
58: slide_ns
59: dynamite_1
60: dynamite_2
61: dynamite_3
62: dynamite_4
63: acid_s
64: exit_1
65: exit_2
66: exit_3
67: balloon
68: plant
69: spring
70: spring {spring_fall}
71: spring {spring_w}
72: spring {spring_e}
73: ball_1
74: ball_2
75: android
76: pause {}
77: android {android_n_1}
78: android {android_n_2}
79: android {android_s_1}
80: android {android_s_2}
81: android {android_e_1}
82: android {android_e_2}
83: android {android_w_1}
84: android {android_w_2}
85: fake_blank {}
86: fake_blank {}
87: fake_blank {}
88: fake_blank {}
89: fake_blank {}
90: fake_blank {}
91: fake_blank {}
92: fake_blank {}
93: fake_blank {}
94: fake_blank {}
95: fake_blank {}
96: fake_blank {}
97: fake_blank {}
98: fake_blank {}
99: spring {push_spring_w}
100: spring {push_spring_e}
101: plant {acid_1}
102: plant {acid_2}
103: plant {acid_3}
104: plant {acid_4}
105: plant {acid_5}
106: plant {acid_6}
107: plant {acid_7}
108: plant {acid_8}
109: pause {grass_wB}
110: pause {grass_eB}
111: pause {grass_nB}
112: pause {grass_sB}
113: pause {dynamite_blank}
114: nut {push_nut_w}
115: nut {push_nut_e}
116: steel_2 {end of level}
117: pause {}
118: pause {emerald_blank}
119: bomb {push_bomb_w}
120: bomb {push_bomb_e}
121: stone {push_stone_w}
122: stone {push_stone_e}
123: pause {diamond_blank}
124: pause {dirt_wB}
125: pause {dirt_eB}
126: pause {dirt_nB}
127: pause {dirt_sB}

128: blank
129: roundwall_2
130: grass
131: steel_2
132: wall_2
133: key_1
134: key_2
135: key_3
136: key_4
137: door_1
138: door_2
139: door_3
140: door_4
141: dripper
142: fake_door_1
143: fake_door_2
144: fake_door_3
145: fake_door_4
146: wonderwall
147: wheel
148: sand
149: acid_nw
150: acid_ne
151: acid_sw
152: acid_se
153: fake_blank
154: ameuba_1
155: ameuba_2
156: ameuba_3
157: ameuba_4
158: exit
159: alpha_arrow_w
160: fake_grass
161: lenses
162: magnify
163: fake_blank {fake_blankB}
164: fake_grass {fake_grassB}
165: switch
166: switch {switchB}
167: blank {}
168: decor_8
169: decor_9
170: decor_10
171: decor_5
172: alpha_comma
173: alpha_apost
174: alpha_minus
175: dynamite
176: steel_3
177: decor_6
178: decor_7
179: steel_1
180: roundwall_1
181: decor_2
182: decor_4
183: decor_3
184: wind_any
185: wind_e
186: wind_s
187: wind_w
188: wind_n
189: dirt
190: plant {}
191: key_5
192: key_6
193: key_7
194: key_8
195: door_5
196: door_6
197: door_7
198: door_8
199: bumper
200: alpha_a
201: alpha_b
202: alpha_c
203: alpha_d
204: alpha_e
205: alpha_f
206: alpha_g
207: alpha_h
208: alpha_i
209: alpha_j
210: alpha_k
211: alpha_l
212: alpha_m
213: alpha_n
214: alpha_o
215: alpha_p
216: alpha_q
217: alpha_r
218: alpha_s
219: alpha_t
220: alpha_u
221: alpha_v
222: alpha_w
223: alpha_x
224: alpha_y
225: alpha_z
226: alpha_0
227: alpha_1
228: alpha_2
229: alpha_3
230: alpha_4
231: alpha_5
232: alpha_6
233: alpha_7
234: alpha_8
235: alpha_9
236: alpha_perio
237: alpha_excla
238: alpha_colon
239: alpha_quest
240: alpha_arrow_e
241: decor_1
242: fake_door_5
243: fake_door_6
244: fake_door_7
245: fake_door_8
246: fake_blank {}
247: fake_blank {}
248: fake_blank {}
249: fake_blank {}
250: fake_blank {}
251: fake_blank {}
252: fake_blank {}
253: fake_blank {}
254: fake_blank {}
255: fake_blank {}

----------------------------------------------------------------------

version 5

0: level
2048: eater exp 1
2057: eater exp 2
2066: eater exp 3
2075: eater exp 4
2084: emerald value
2085: diamond value
2086: alien value
2087: tank value
2088: bug value
2089: eater value
2090: nut value
2091: dynamite value
2092: key value
2093: bonus
2094: time
2095: emeralds needed
2096: player 1 pos
2098: player 2 pos
2100: ameuba speed
2102: wonderwall time
2104: wheel time
2106: ID (0xff353030)
2110: SIZE

0: stone
1: stone {stone_fall}
2: diamond
3: diamond {diamond_fall}
4: alien
5: alien {alien_pause}
6: pause {boom_1}
7: pause {boom_2}
8: tank_n_1
9: tank_e_1
10: tank_s_1
11: tank_w_1
12: tank_n_2
13: tank_e_2
14: tank_s_2
15: tank_w_2
16: bomb
17: bomb {bomb_fall}
18: emerald
19: emerald {emerald_fall}
20: bug_n_1
21: bug_e_1
22: bug_s_1
23: bug_w_1
24: bug_n_2
25: bug_e_2
26: bug_s_2
27: bug_w_2
28: drip
29: drip {drip_fall}
30: blank {drip_stretchB}
31: drip {drip_stretch}
32: stone {stone_pause}
33: bomb {bomb_pause}
34: diamond {diamond_pause}
35: emerald {emerald_pause}
36: wonderwall {wonderwallB}
37: nut
38: nut {nut_fall}
39: nut {nut_pause}
40: wheel {wheelB}
41: eater_n
42: eater_s
43: eater_w
44: eater_e
45: sand_stone
46: blank {sand_stonein_2}
47: blank {sand_stonein_3}
48: blank {sand_stonein_4}
49: sand {sand_stonesand_2}
50: sand {sand_stonesand_3}
51: sand {sand_stonesand_4}
52: sand_stone {sand_sandstone_2}
53: sand_stone {sand_sandstone_3}
54: sand_stone {sand_sandstone_4}
55: sand {sand_stonesand_4}
56: stone {sand_stoneout_2}
57: slide_ew
58: slide_ns
59: dynamite_1
60: dynamite_2
61: dynamite_3
62: dynamite_4
63: acid_s
64: exit_1
65: exit_2
66: exit_3
67: balloon
68: plant
69: fake_blank {}
70: fake_blank {}
71: fake_blank {}
72: fake_blank {}
73: fake_blank {}
74: fake_blank {}
75: fake_blank {}
76: fake_blank {}
77: fake_blank {}
78: fake_blank {}
79: fake_blank {}
80: fake_blank {}
81: fake_blank {}
82: fake_blank {}
83: fake_blank {}
84: fake_blank {}
85: fake_blank {}
86: fake_blank {}
87: fake_blank {}
88: fake_blank {}
89: fake_blank {}
90: fake_blank {}
91: fake_blank {}
92: fake_blank {}
93: fake_blank {}
94: fake_blank {}
95: fake_blank {}
96: fake_blank {}
97: fake_blank {}
98: fake_blank {}
99: fake_blank {}
100: fake_blank {}
101: plant {acid_1}
102: plant {acid_2}
103: plant {acid_3}
104: plant {acid_4}
105: plant {acid_5}
106: plant {acid_6}
107: plant {acid_7}
108: plant {acid_8}
109: pause {grass_wB}
110: pause {grass_eB}
111: pause {grass_nB}
112: pause {grass_sB}
113: pause {dynamite_blank}
114: nut {push_nut_w}
115: nut {push_nut_e}
116: steel_2 {end of level}
117: dynamite_4 {boom_2}
118: pause {emerald_blank}
119: bomb {push_bomb_w}
120: bomb {push_bomb_e}
121: stone {push_stone_w}
122: stone {push_stone_e}
123: pause {diamond_blank}
124: pause {dirt_wB}
125: pause {dirt_eB}
126: pause {dirt_nB}
127: pause {dirt_sB}

128: blank
129: roundwall_2
130: grass
131: steel_2
132: wall_2
133: key_1
134: key_2
135: key_3
136: key_4
137: door_1
138: door_2
139: door_3
140: door_4
141: dripper
142: fake_door_1
143: fake_door_2
144: fake_door_3
145: fake_door_4
146: wonderwall
147: wheel
148: sand
149: acid_nw
150: acid_ne
151: acid_sw
152: acid_se
153: fake_blank
154: ameuba_1
155: ameuba_2
156: ameuba_3
157: ameuba_4
158: exit
159: alpha_arrow_w
160: fake_grass
161: fake_blank {}
162: fake_blank {}
163: fake_blank {}
164: fake_blank {}
165: fake_blank {}
166: fake_blank {}
167: blank {}
168: decor_8
169: decor_9
170: decor_10
171: decor_5
172: alpha_comma
173: alpha_apost
174: alpha_minus
175: dynamite
176: steel_3
177: decor_6
178: decor_7
179: steel_1
180: roundwall_1
181: decor_2
182: decor_4
183: decor_3
184: wind_any
185: wind_e
186: wind_s
187: wind_w
188: wind_n
189: dirt
190: plant {}
191: fake_blank {}
192: fake_blank {}
193: fake_blank {}
194: fake_blank {}
195: fake_blank {}
196: fake_blank {}
197: fake_blank {}
198: fake_blank {}
199: fake_blank {}
200: alpha_a
201: alpha_b
202: alpha_c
203: alpha_d
204: alpha_e
205: alpha_f
206: alpha_g
207: alpha_h
208: alpha_i
209: alpha_j
210: alpha_k
211: alpha_l
212: alpha_m
213: alpha_n
214: alpha_o
215: alpha_p
216: alpha_q
217: alpha_r
218: alpha_s
219: alpha_t
220: alpha_u
221: alpha_v
222: alpha_w
223: alpha_x
224: alpha_y
225: alpha_z
226: alpha_0
227: alpha_1
228: alpha_2
229: alpha_3
230: alpha_4
231: alpha_5
232: alpha_6
233: alpha_7
234: alpha_8
235: alpha_9
236: alpha_perio
237: alpha_excla
238: alpha_colon
239: alpha_quest
240: alpha_arrow_e
241: decor_1
242: fake_blank {}
243: fake_blank {}
244: fake_blank {}
245: fake_blank {}
246: fake_blank {}
247: fake_blank {}
248: fake_blank {}
249: fake_blank {}
250: fake_blank {}
251: fake_blank {}
252: fake_blank {}
253: fake_blank {}
254: fake_blank {}
255: fake_blank {}

----------------------------------------------------------------------

version 4

0: level
2048: eater exp 1
2057: eater exp 2
2066: eater exp 3
2075: eater exp 4
2084: emerald value
2085: diamond value
2086: alien value
2087: tank value
2088: bug value
2089: eater value
2090: nut value
2091: dynamite value
2092: key value
2093: bonus
2094: time
2095: emeralds needed
2096: player 1 pos
2098: player 2 pos
2100: ameuba speed
2102: wonderwall time
2104: wheel time
2106: SIZE

0: blank
1: emerald
2: diamond
3: stone
4: alien
5: tank_n_1
6: bomb
7: bug_n_1
8: drip
9: nut
10: eater_n
11: sand_stone
12: grass
13: roundwall_1
14: steel_1
15: wall_1
16: key_1
17: key_2
18: key_3
19: key_4
20: wonderwall
21: wheel
22: dynamite
23: exit_2 {}
24: exit_3 {}
25: exit_1 {}
26: diamond {diamond_shine}
27: emerald {emerald_shine}

0: stone
1: stone {stone_fall}
2: diamond
3: diamond {diamond_fall}
4: alien
5: alien {alien_pause}
6: pause {boom_1}
7: pause {boom_2}
8: tank_n_1
9: tank_e_1
10: tank_s_1
11: tank_w_1
12: tank_n_2
13: tank_e_2
14: tank_s_2
15: tank_w_2
16: bomb
17: bomb {bomb_fall}
18: emerald
19: emerald {emerald_fall}
20: bug_n_1
21: bug_e_1
22: bug_s_1
23: bug_w_1
24: bug_n_2
25: bug_e_2
26: bug_s_2
27: bug_w_2
28: drip
29: drip {drip_fall}
30: blank {drip_stretchB}
31: drip {drip_stretch}
32: stone {stone_pause}
33: bomb {bomb_pause}
34: diamond {diamond_pause}
35: emerald {emerald_pause}
36: wonderwall {wonderwallB}
37: nut
38: nut {nut_fall}
39: nut {nut_pause}
40: wheel {wheelB}
41: eater_n
42: eater_s
43: eater_w
44: eater_e
45: sand_stone
46: blank {sand_stonein_2}
47: blank {sand_stonein_3}
48: blank {sand_stonein_4}
49: sand {sand_stonesand_2}
50: sand {sand_stonesand_3}
51: sand {sand_stonesand_4}
52: sand_stone {sand_sandstone_2}
53: sand_stone {sand_sandstone_3}
54: sand_stone {sand_sandstone_4}
55: sand {sand_stonesand_4}
56: stone {sand_stoneout_2}
57: fake_blank {}
58: fake_blank {}
59: dynamite_1
60: dynamite_2
61: dynamite_3
62: dynamite_4
63: acid_s
64: exit_1
65: exit_2
66: exit_3
67: fake_blank {}
68: fake_blank {}
69: fake_blank {}
70: fake_blank {}
71: fake_blank {}
72: fake_blank {}
73: fake_blank {}
74: fake_blank {}
75: fake_blank {}
76: fake_blank {}
77: fake_blank {}
78: fake_blank
79: fake_blank {}
80: fake_blank {}
81: fake_blank {}
82: fake_blank {}
83: fake_blank {}
84: fake_blank {}
85: fake_blank {}
86: fake_blank {}
87: fake_blank {}
88: fake_blank {}
89: fake_blank {}
90: fake_blank {}
91: fake_blank {}
92: fake_blank {}
93: fake_blank {}
94: fake_blank {}
95: fake_blank {}
96: fake_blank {}
97: fake_blank {}
98: fake_blank {}
99: fake_blank {}
100: fake_blank {}
101: fake_blank {}
102: fake_blank {}
103: fake_blank {}
104: fake_blank {}
105: fake_blank {}
106: fake_blank {}
107: fake_blank {}
108: fake_blank {}
109: fake_blank {}
110: fake_blank {}
111: fake_blank {}
112: fake_blank {}
113: pause {dynamite_blank}
114: nut {push_nut_w}
115: nut {push_nut_e}
116: steel_1 {end of level}
117: dynamite_4 {boom_2}
118: pause {emerald_blank}
119: bomb {push_bomb_w}
120: bomb {push_bomb_e}
121: stone {push_stone_w}
122: stone {push_stone_e}
123: pause {diamond_blank}
124: pause {dirt_wB}
125: pause {dirt_eB}
126: pause {dirt_nB}
127: pause {dirt_sB}

128: blank
129: roundwall_1
130: grass
131: steel_1
132: wall_1
133: key_1
134: key_2
135: key_3
136: key_4
137: door_1
138: door_2
139: door_3
140: door_4
141: dripper {}
142: fake_door_1
143: fake_door_2
144: fake_door_3
145: fake_door_4
146: wonderwall
147: wheel
148: sand
149: acid_nw
150: acid_ne
151: acid_sw
152: acid_se
153: plant {acid_1}
154: ameuba_5
155: ameuba_6
156: ameuba_7
157: ameuba_8
158: exit
159: fake_grass {dirt}
160: fake_grass {dirt}
161: fake_grass {dirt}
162: fake_grass {dirt}
163: fake_grass {dirt}
164: fake_grass {dirt}
165: fake_grass {dirt}
166: fake_grass {dirt}
167: fake_grass {dirt}
168: fake_grass {dirt}
169: fake_grass {dirt}
170: fake_grass {dirt}
171: fake_grass {dirt}
172: fake_grass {dirt}
173: fake_grass {dirt}
174: fake_grass {dirt}
175: dynamite
176: fake_blank {}
177: fake_blank {}
178: fake_blank {}
179: fake_blank {}
180: fake_blank {}
181: fake_blank {}
182: fake_blank {}
183: fake_blank {}
184: fake_blank {}
185: fake_blank {}
186: fake_blank {}
187: fake_blank {}
188: fake_blank {}
189: fake_blank {}
190: plant {}
191: fake_blank {}
192: fake_blank {}
193: fake_blank {}
194: fake_blank {}
195: fake_blank {}
196: fake_blank {}
197: fake_blank {}
198: fake_blank {}
199: fake_blank {}
200: alpha_a
201: alpha_b
202: alpha_c
203: alpha_d
204: alpha_e
205: alpha_f
206: alpha_g
207: alpha_h
208: alpha_i
209: alpha_j
210: alpha_k
211: alpha_l
212: alpha_m
213: alpha_n
214: alpha_o
215: alpha_p
216: alpha_q
217: alpha_r
218: alpha_s
219: alpha_t
220: alpha_u
221: alpha_v
222: alpha_w
223: alpha_x
224: alpha_y
225: alpha_z
226: alpha_0
227: alpha_1
228: alpha_2
229: alpha_3
230: alpha_4
231: alpha_5
232: alpha_6
233: alpha_7
234: alpha_8
235: alpha_9
236: alpha_perio
237: alpha_excla
238: alpha_colon
239: alpha_quest
240: alpha_arrow_e {}
241: decor_1 {alpha_copyr}
242: fake_blank {}
243: fake_blank {}
244: fake_blank {}
245: fake_blank {}
246: fake_blank {}
247: fake_blank {}
248: fake_blank {}
249: fake_blank {}
250: fake_blank {}
251: fake_blank {}
252: fake_blank {}
253: fake_blank {}
254: fake_blank {}
255: fake_blank {}

----------------------------------------------------------------------
*/
