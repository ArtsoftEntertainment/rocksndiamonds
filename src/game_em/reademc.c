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

static const short map_emc[256] =
{
  Cstone,		Cstone,		Cdiamond,	Cdiamond,
  Calien,		Calien,		Cblank,		Cblank,
  Ctank_1_n,		Ctank_1_e,	Ctank_1_s,	Ctank_1_w,
  Ctank_2_n,		Ctank_2_e,	Ctank_2_s,	Ctank_2_w,

  Cbomb,		Cbomb,		Cemerald,	Cemerald,
  Cbug_1_n,		Cbug_1_e,	Cbug_1_s,	Cbug_1_w,
  Cbug_2_n,		Cbug_2_e,	Cbug_2_s,	Cbug_2_w,
  Cdrip,		Cdrip,		Cdrip,		Cdrip,

  Cstone,		Cbomb,		Cdiamond,	Cemerald,
  Cwonderwall,		Cnut,		Cnut,		Cnut,
  Cwheel,		Ceater_n,	Ceater_s,	Ceater_w,
  Ceater_e,		Csand_stone,	Cblank,		Cblank,

  Cblank,		Csand,		Csand,		Csand,
  Csand_stone,		Csand_stone,	Csand_stone,	Csand,
  Cstone,		Cslide_ew,	Cslide_ns,	Cdynamite_1,
  Cdynamite_2,		Cdynamite_3,	Cdynamite_4,	Cacid_s,

  Cexit_1,		Cexit_2,	Cexit_3,	Cballoon,
  Cplant,		Cspring,	Cspring,	Cpush_spring_w,
  Cpush_spring_e,	Cball_1,	Cball_2,	Candroid,
  Cblank,		Candroid,	Candroid,	Candroid,

  Candroid,		Candroid,	Candroid,	Candroid,
  Candroid,		Cblank,		Cblank,		Cblank,
  Cblank,		Cblank,		Cblank,		Cblank,
  Cblank,		Cblank,		Cblank,		Cblank,

  Cblank,		Cblank,		Cblank,		Cpush_spring_w,
  Cpush_spring_e,	Cacid_1,	Cacid_2,	Cacid_3,
  Cacid_4,		Cacid_5,	Cacid_6,	Cacid_7,
  Cacid_8,		Cblank,		Cblank,		Cblank,

  Cblank,		Cblank,		Cpush_nut_w,	Cpush_nut_e,
  Csteel_1,		Cblank,		Cblank,		Cpush_bomb_w,
  Cpush_bomb_e,		Cpush_stone_w,	Cpush_stone_e,	Cblank,
  Cblank,		Cblank,		Cblank,		Cblank,

  Cblank,		Croundwall_1,	Cgrass,		Csteel_1,
  Cwall_1,		Ckey_1,		Ckey_2,		Ckey_3,
  Ckey_4,		Cdoor_1,	Cdoor_2,	Cdoor_3,
  Cdoor_4,		Cfake_amoeba,	Cfake_door_1,	Cfake_door_2,

  Cfake_door_3,		Cfake_door_4,	Cwonderwall,	Cwheel,
  Csand,		Cacid_nw,	Cacid_ne,	Cacid_sw,
  Cacid_se,		Cfake_blank,	Camoeba_1,	Camoeba_2,
  Camoeba_3,		Camoeba_4,	Cexit,		Calpha_arrow_w,

  Cfake_grass,		Clenses,	Cmagnify,	Cfake_blank,
  Cfake_grass,		Cswitch,	Cswitch,	Cblank,
  Cdecor_8,		Cdecor_9,	Cdecor_10,	Cdecor_5,
  Calpha_comma,		Calpha_apost,	Calpha_minus,	Cdynamite,

  Csteel_3,		Cdecor_6,	Cdecor_7,	Csteel_2,
  Croundwall_2,		Cdecor_2,	Cdecor_4,	Cdecor_3,
  Cwind_any,		Cwind_e,	Cwind_s,	Cwind_w,
  Cwind_n,		Cdirt,		Cplant,		Ckey_5,

  Ckey_6,		Ckey_7,		Ckey_8,		Cdoor_5,
  Cdoor_6,		Cdoor_7,	Cdoor_8,	Cbumper,
  Calpha_a,		Calpha_b,	Calpha_c,	Calpha_d,
  Calpha_e,		Calpha_f,	Calpha_g,	Calpha_h,

  Calpha_i,		Calpha_j,	Calpha_k,	Calpha_l,
  Calpha_m,		Calpha_n,	Calpha_o,	Calpha_p,
  Calpha_q,		Calpha_r,	Calpha_s,	Calpha_t,
  Calpha_u,		Calpha_v,	Calpha_w,	Calpha_x,

  Calpha_y,		Calpha_z,	Calpha_0,	Calpha_1,
  Calpha_2,		Calpha_3,	Calpha_4,	Calpha_5,
  Calpha_6,		Calpha_7,	Calpha_8,	Calpha_9,
  Calpha_perio,		Calpha_excla,	Calpha_colon,	Calpha_quest,

  Calpha_arrow_e,	Cdecor_1,	Cfake_door_5,	Cfake_door_6,
  Cfake_door_7,		Cfake_door_8,	Cblank,		Cblank,
  Cblank,		Cblank,		Cblank,		Cblank,
  Cblank,		Cblank,		Calpha_copyr,	Cfake_acid_1
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

  for (i = 0; i < GAME_TILE_MAX; i++)
    cav.android_array[i] = Cblank;

  if (android_eater)
  {
    cav.android_array[Xeater_n]		= Ceater_n;
    cav.android_array[Yeater_nB]	= Ceater_n;

    cav.android_array[Xeater_e]		= Ceater_e;
    cav.android_array[Yeater_eB]	= Ceater_e;

    cav.android_array[Xeater_s]		= Ceater_s;
    cav.android_array[Yeater_sB]	= Ceater_s;

    cav.android_array[Xeater_w]		= Ceater_w;
    cav.android_array[Yeater_wB]	= Ceater_w;
  }

  if (android_alien)
  {
    cav.android_array[Xalien]		= Calien;
    cav.android_array[Xalien_pause]	= Calien;
    cav.android_array[Yalien_nB]	= Calien;
    cav.android_array[Yalien_eB]	= Calien;
    cav.android_array[Yalien_sB]	= Calien;
    cav.android_array[Yalien_wB]	= Calien;
  }

  if (android_bug)
  {
    cav.android_array[Xbug_1_n]		= Cbug_2_n;
    cav.android_array[Xbug_2_n]		= Cbug_2_n;
    cav.android_array[Ybug_nB]		= Cbug_2_n;
    cav.android_array[Ybug_n_e]		= Cbug_2_n;
    cav.android_array[Ybug_n_w]		= Cbug_2_n;

    cav.android_array[Xbug_1_e]		= Cbug_2_e;
    cav.android_array[Xbug_2_e]		= Cbug_2_e;
    cav.android_array[Ybug_eB]		= Cbug_2_e;
    cav.android_array[Ybug_e_s]		= Cbug_2_e;
    cav.android_array[Ybug_e_n]		= Cbug_2_e;

    cav.android_array[Xbug_1_s]		= Cbug_2_s;
    cav.android_array[Xbug_2_s]		= Cbug_2_s;
    cav.android_array[Ybug_sB]		= Cbug_2_s;
    cav.android_array[Ybug_s_w]		= Cbug_2_s;
    cav.android_array[Ybug_s_e]		= Cbug_2_s;

    cav.android_array[Xbug_1_w]		= Cbug_2_w;
    cav.android_array[Xbug_2_w]		= Cbug_2_w;
    cav.android_array[Ybug_wB]		= Cbug_2_w;
    cav.android_array[Ybug_w_n]		= Cbug_2_w;
    cav.android_array[Ybug_w_s]		= Cbug_2_w;
  }

  if (android_tank)
  {
    cav.android_array[Xtank_1_n]	= Ctank_1_n;
    cav.android_array[Xtank_2_n]	= Ctank_1_n;
    cav.android_array[Ytank_nB]		= Ctank_1_n;
    cav.android_array[Ytank_n_e]	= Ctank_1_n;
    cav.android_array[Ytank_n_w]	= Ctank_1_n;

    cav.android_array[Xtank_1_e]	= Ctank_1_e;
    cav.android_array[Xtank_2_e]	= Ctank_1_e;
    cav.android_array[Ytank_eB]		= Ctank_1_e;
    cav.android_array[Ytank_e_s]	= Ctank_1_e;
    cav.android_array[Ytank_e_n]	= Ctank_1_e;

    cav.android_array[Xtank_1_s]	= Ctank_1_s;
    cav.android_array[Xtank_2_s]	= Ctank_1_s;
    cav.android_array[Ytank_sB]		= Ctank_1_s;
    cav.android_array[Ytank_s_w]	= Ctank_1_s;
    cav.android_array[Ytank_s_e]	= Ctank_1_s;

    cav.android_array[Xtank_1_w]	= Ctank_1_w;
    cav.android_array[Xtank_2_w]	= Ctank_1_w;
    cav.android_array[Ytank_wB]		= Ctank_1_w;
    cav.android_array[Ytank_w_n]	= Ctank_1_w;
    cav.android_array[Ytank_w_s]	= Ctank_1_w;
  }

  if (android_emerald)
  {
    cav.android_array[Xemerald]		= Cemerald;
    cav.android_array[Xemerald_pause]	= Cemerald;
    cav.android_array[Xemerald_fall]	= Cemerald;
    cav.android_array[Yemerald_sB]	= Cemerald;
    cav.android_array[Yemerald_eB]	= Cemerald;
    cav.android_array[Yemerald_wB]	= Cemerald;
  }

  if (android_diamond)
  {
    cav.android_array[Xdiamond]		= Cdiamond;
    cav.android_array[Xdiamond_pause]	= Cdiamond;
    cav.android_array[Xdiamond_fall]	= Cdiamond;
    cav.android_array[Ydiamond_sB]	= Cdiamond;
    cav.android_array[Ydiamond_eB]	= Cdiamond;
    cav.android_array[Ydiamond_wB]	= Cdiamond;
  }

  if (android_stone)
  {
    cav.android_array[Xstone]		= Cstone;
    cav.android_array[Xstone_pause]	= Cstone;
    cav.android_array[Xstone_fall]	= Cstone;
    cav.android_array[Ystone_sB]	= Cstone;
    cav.android_array[Ystone_eB]	= Cstone;
    cav.android_array[Ystone_wB]	= Cstone;
  }

  if (android_bomb)
  {
    cav.android_array[Xbomb]		= Cbomb;
    cav.android_array[Xbomb_pause]	= Cbomb;
    cav.android_array[Xbomb_fall]	= Cbomb;
    cav.android_array[Ybomb_sB]		= Cbomb;
    cav.android_array[Ybomb_eB]		= Cbomb;
    cav.android_array[Ybomb_wB]		= Cbomb;
  }

  if (android_nut)
  {
    cav.android_array[Xnut]		= Cnut;
    cav.android_array[Xnut_pause]	= Cnut;
    cav.android_array[Xnut_fall]	= Cnut;
    cav.android_array[Ynut_sB]		= Cnut;
    cav.android_array[Ynut_eB]		= Cnut;
    cav.android_array[Ynut_wB]		= Cnut;
  }

  if (android_spring)
  {
    cav.android_array[Xspring]		= Cspring;
    cav.android_array[Xspring_pause]	= Cspring;
    cav.android_array[Xspring_fall]	= Cspring;
    cav.android_array[Xspring_e]	= Cspring;
    cav.android_array[Xspring_w]	= Cspring;
    cav.android_array[Yspring_sB]	= Cspring;
    cav.android_array[Yspring_eB]	= Cspring;
    cav.android_array[Yspring_wB]	= Cspring;
    cav.android_array[Yspring_alien_eB]	= Cspring;
    cav.android_array[Yspring_alien_wB]	= Cspring;
  }

  if (android_dynamite)
  {
    cav.android_array[Xdynamite]	= Cdynamite;
  }

  if (android_balloon)
  {
    cav.android_array[Xballoon]		= Cballoon;
    cav.android_array[Yballoon_nB]	= Cballoon;
    cav.android_array[Yballoon_eB]	= Cballoon;
    cav.android_array[Yballoon_sB]	= Cballoon;
    cav.android_array[Yballoon_wB]	= Cballoon;
  }

  if (android_amoeba)
  {
    cav.android_array[Xfake_amoeba]	= Cdrip;
    cav.android_array[Yfake_amoeba]	= Cdrip;
    cav.android_array[Xamoeba_1]	= Cdrip;
    cav.android_array[Xamoeba_2]	= Cdrip;
    cav.android_array[Xamoeba_3]	= Cdrip;
    cav.android_array[Xamoeba_4]	= Cdrip;
    cav.android_array[Xamoeba_5]	= Cdrip;
    cav.android_array[Xamoeba_6]	= Cdrip;
    cav.android_array[Xamoeba_7]	= Cdrip;
    cav.android_array[Xamoeba_8]	= Cdrip;
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

  cav.time_seconds = MIN(GET_BE16(src[2110]), 9999);
  cav.gems_needed = src[2095];

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

  cav.android_move_time  = MIN(GET_BE16(src[2164]), 9999);
  cav.android_clone_time = MIN(GET_BE16(src[2166]), 9999);
  cav.ball_time		 = MIN(GET_BE16(src[2160]), 9999);

  cav.lenses_time	= MIN(GET_BE16(src[2154]), 9999);
  cav.magnify_time	= MIN(GET_BE16(src[2156]), 9999);
  cav.wheel_time	= MIN(GET_BE16(src[2104]), 9999);

  cav.amoeba_time	= MIN(GET_BE16(src[2100]) * 28, 9999);
  cav.wonderwall_time	= MIN(GET_BE16(src[2102]), 9999);

  cav.wind_cnt = src[2149] & 15 ? cav.wind_time : 0;
  temp = src[2149];
  cav.wind_direction = (temp & 8 ? 0 :
			temp & 1 ? 1 :
			temp & 2 ? 2 :
			temp & 4 ? 3 : 0);

  /* global flags */

  cav.ball_random = src[2162] & 1   ? 1 : 0;
  cav.ball_state  = src[2162] & 128 ? 1 : 0;

  for (temp = 1; temp < 2047; temp++)
  {
    switch (src[temp])
    {
      case 36:					/* wonderwall */
	cav.wonderwall_state = 1;
	cav.wonderwall_time = 9999;
	break;

      case 40:					/* wheel */
	cav.wheel_x = temp & 63;
	cav.wheel_y = temp >> 6;
	cav.wheel_cnt = cav.wheel_time;
	break;

      case 163:					/* fake blank */
	cav.lenses_cnt = 9999;
	break;

      case 164:					/* fake grass */
	cav.magnify_cnt = 9999;
	break;
    }
  }

  /* android */

  init_android_clone_table(GET_BE16(src[2168]));

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
      cav.ball_array[y][1] = (src[2163] & 1)  ? temp : Cblank; /* north */
      cav.ball_array[y][6] = (src[2163] & 2)  ? temp : Cblank; /* south */
      cav.ball_array[y][3] = (src[2163] & 4)  ? temp : Cblank; /* west */
      cav.ball_array[y][4] = (src[2163] & 8)  ? temp : Cblank; /* east */
      cav.ball_array[y][7] = (src[2163] & 16) ? temp : Cblank; /* southeast */
      cav.ball_array[y][5] = (src[2163] & 32) ? temp : Cblank; /* southwest */
      cav.ball_array[y][2] = (src[2163] & 64) ? temp : Cblank; /* northeast */
      cav.ball_array[y][0] = (src[2163] & 128)? temp : Cblank; /* northwest */
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

  /* first fill the complete playfield with the empty space element */
  for (y = 0; y < CAVE_HEIGHT; y++)
    for (x = 0; x < CAVE_WIDTH; x++)
      cav.cave[x][y] = Cblank;

  /* then copy the real level contents from level file into the playfield */
  temp = 0;
  for (y = 0; y < cav.height; y++)
    for (x = 0; x < cav.width; x++)
      cav.cave[x][y] = map_emc[src[temp++]];

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
	  src[i] = 254;		/* replace 'Cdecor_1' with 'Calpha_copyr' */
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
    if (src[i] == 63)		/* replace element above 'Cacid_s' ... */
      src[i - 64] = 101;	/* ... with 'Cacid_1' */

  /* fix acid with no base beneath it (see below for details (*)) */
  for (i = 64; i < 2048 - 1; i++)
  {
    if (file_version <= FILE_VERSION_EM_V2 &&
	src[i - 64] == 101 && src[i] != 63)	/* acid without base */
    {
      if (src[i - 1] == 101 ||			/* remove acid over acid row */
	  src[i + 1] == 101)
	src[i - 64] = 6;	/* replace element above with 'Cblank' */
      else
	src[i - 64] = 255;	/* replace element above with 'Cfake_acid_1' */
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

  /* wind direction */
  i = src[2149];
  i &= 15;
  i &= -i;
  src[2149] = i;

  /* ball object */
  src[2158] = 0;
  src[2159] = map_v6[src[2159]];

  /* ball data */
  src[2162] &= 129;
  if (src[2162] & 1)
    src[2163] = 0;

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
