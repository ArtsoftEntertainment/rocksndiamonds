/* 2000-04-19T13:26:05Z
 *
 * construct some tables to be included directly in emerald mine source.
 * i made this because dynamically building the tables every time sucks and i
 * need to be able to easily modify tile definitions.
 *
 * this is key data which almost everything depends on.
 *
 * this is supposed to be fairly easy to read and modify. the tab values
 * are still hard coded constants but that should be less of a problem to
 * modify.
 */

#include "main_em.h"


/* ------------------------------------------------------------------------- */

/* 0=stop 1=blank */
int tile_blank[] =
{
  Xblank, 1,
  Xsplash_e, 1,
  Xsplash_w, 1,
  Xfake_acid_1, 1,
  Xfake_acid_2, 1,
  Xfake_acid_3, 1,
  Xfake_acid_4, 1,
  Xfake_acid_5, 1,
  Xfake_acid_6, 1,
  Xfake_acid_7, 1,
  Xfake_acid_8, 1,
  TILE_MAX
};

/* 0=stop 1=acid */
int tile_acid[] =
{
  Xblank, 1,
  Xsplash_e, 1,
  Xsplash_w, 1,
  Xfake_acid_1, 1,
  Xfake_acid_2, 1,
  Xfake_acid_3, 1,
  Xfake_acid_4, 1,
  Xfake_acid_5, 1,
  Xfake_acid_6, 1,
  Xfake_acid_7, 1,
  Xfake_acid_8, 1,
  Xacid_1, 1,
  Xacid_2, 1,
  Xacid_3, 1,
  Xacid_4, 1,
  Xacid_5, 1,
  Xacid_6, 1,
  Xacid_7, 1,
  Xacid_8, 1,
  TILE_MAX
};

/* 0=stop 1=fake_acid */
int tile_fake_acid[] =
{
  Xfake_acid_1, 1,
  Xfake_acid_2, 1,
  Xfake_acid_3, 1,
  Xfake_acid_4, 1,
  Xfake_acid_5, 1,
  Xfake_acid_6, 1,
  Xfake_acid_7, 1,
  Xfake_acid_8, 1,
  TILE_MAX
};

/* 0=stop 1=amoeba */
int tile_amoeba[] =
{
  Xfake_amoeba, 1,
  Xfake_amoebaB, 1,
  Xamoeba_1, 1,
  Xamoeba_2, 1,
  Xamoeba_3, 1,
  Xamoeba_4, 1,
  Xamoeba_5, 1,
  Xamoeba_6, 1,
  Xamoeba_7, 1,
  Xamoeba_8, 1,
  TILE_MAX
};

/* 0=stop 1=move */
int tile_android_move[] =
{
  Xblank, 1,
  Xsplash_e, 1,
  Xsplash_w, 1,
  Xfake_acid_1, 1,
  Xfake_acid_2, 1,
  Xfake_acid_3, 1,
  Xfake_acid_4, 1,
  Xfake_acid_5, 1,
  Xfake_acid_6, 1,
  Xfake_acid_7, 1,
  Xfake_acid_8, 1,
  Xplant, 1,
  TILE_MAX
};


/* ------------------------------------------------------------------------- */

/* explosions: special format */
/* everything is initially filled with Xboom_1 */
int tile_explode[] =
{
  Zborder,
  Znormal,
  Zdynamite,
  Xboom_bug,
  Xboom_bomb,
  Xboom_android,
  Xandroid,
  Xandroid_1_n,
  Xandroid_2_n,
  Xandroid_1_e,
  Xandroid_2_e,
  Xandroid_1_s,
  Xandroid_2_s,
  Xandroid_1_w,
  Xandroid_2_w,
  Xacid_ne,
  Xacid_nw,
  Xacid_s,
  Xacid_se,
  Xacid_sw,
  Xacid_1,
  Xacid_2,
  Xacid_3,
  Xacid_4,
  Xacid_5,
  Xacid_6,
  Xacid_7,
  Xacid_8,
  Xdoor_1,
  Xdoor_2,
  Xdoor_3,
  Xdoor_4,
  Xdoor_5,
  Xdoor_6,
  Xdoor_7,
  Xdoor_8,
  Xplant,
  Yplant,
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
  TILE_MAX, 			/* up till here are indestructable */

  Xbug_1_n, Xboom_bug,
  Xbug_1_e, Xboom_bug,
  Xbug_1_s, Xboom_bug,
  Xbug_1_w, Xboom_bug,
  Xbug_2_n, Xboom_bug,
  Xbug_2_e, Xboom_bug,
  Xbug_2_s, Xboom_bug,
  Xbug_2_w, Xboom_bug,
  Xbomb, Xboom_bomb,
  Xbomb_pause, Xboom_bomb,
  Xbomb_fall, Xboom_bomb,
  TILE_MAX, 			/* up till here are special explosions */

  Xandroid, Xboom_android,
  Xandroid_1_n, Xboom_android,
  Xandroid_2_n, Xboom_android,
  Xandroid_1_e, Xboom_android,
  Xandroid_2_e, Xboom_android,
  Xandroid_1_s, Xboom_android,
  Xandroid_2_s, Xboom_android,
  Xandroid_1_w, Xboom_android,
  Xandroid_2_w, Xboom_android,
  TILE_MAX			/* up until here are dynamite explosions */
};


/* ------------------------------------------------------------------------- */


/* 0=stop 1=blank */
unsigned char tab_blank[TILE_MAX];

/* 0=stop 1=acid */
unsigned char tab_acid[TILE_MAX];

/* 0=stop 1=fake_acid */
unsigned char tab_fake_acid[TILE_MAX];

/* 0=stop 1=amoeba */
unsigned char tab_amoeba[TILE_MAX];

/* 0=stop 1=move */
unsigned char tab_android_move[TILE_MAX];

/* normal explosion */
unsigned short tab_explode_normal[TILE_MAX];

/* dynamite explosion */
unsigned short tab_explode_dynamite[TILE_MAX];

/* map tiles and frames to graphic info */
struct GraphicInfo_EM graphic_info_em_object[TILE_MAX][8];

/* map player number, frames and action to graphic info */
struct GraphicInfo_EM graphic_info_em_player[MAX_PLAYERS][PLY_MAX][8];

static void create_tab(int *invert, unsigned char *array)
{
  int i;
  int buffer[TILE_MAX];

  for (i = 0; i < TILE_MAX; i++)
    buffer[i] = 0;

  for (;invert[0] < TILE_MAX; invert += 2)
    buffer[invert[0]] = invert[1];

  for (i = 0; i < TILE_MAX; i++)
    array[i] = buffer[i];
}

static void create_explode(void)
{
  int i;
  int *tile = tile_explode;
  int buffer[TILE_MAX];

  for (i = 0; i < TILE_MAX; i++)
    buffer[i] = Xboom_1;
  while ((i = *tile++) < TILE_MAX)
    buffer[i] = i;			/* these tiles are indestructable */
  while ((i = *tile++) < TILE_MAX)
    buffer[i] = *tile++;		/* these tiles are special */

  for (i = 0; i < TILE_MAX; i++)
    tab_explode_normal[i] = buffer[i];

  while ((i = *tile++) < TILE_MAX)
    buffer[i] = *tile++;		/* these tiles for dynamite */

  for (i = 0; i < TILE_MAX; i++)
    tab_explode_dynamite[i] = buffer[i];
}

void tab_generate(void)
{
  create_tab(tile_blank, tab_blank);
  create_tab(tile_acid, tab_acid);
  create_tab(tile_fake_acid, tab_fake_acid);
  create_tab(tile_amoeba, tab_amoeba);
  create_tab(tile_android_move, tab_android_move);
  create_explode();
}

void tab_generate_graphics_info_em(void)
{
  InitGraphicInfo_EM();
}
