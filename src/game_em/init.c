/* 2000-01-06 06:43:39
 *
 * set everything up and close everything down
 */

#include "main_em.h"


Bitmap *screenBitmap;

struct GlobalInfo_EM global_em_info;
struct GameInfo_EM game_em;

static boolean native_sound_active[SOUND_MAX];
static int native_sound_element[SOUND_MAX];


void InitGfxBuffers_EM(void)
{
  ReCreateBitmap(&screenBitmap, MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY);

  global_em_info.screenbuffer = screenBitmap;
}

void game_init_random(void)
{
  game_em.random = 1684108901;	/* what a nice seed */
}

void game_init_cave_buffers(void)
{
  int x, y;

  for (x = 0; x < CAVE_BUFFER_WIDTH; x++)
  {
    for (y = 0; y < CAVE_BUFFER_HEIGHT; y++)
    {
      lev.cavebuf[x][y] = Zborder;
      lev.nextbuf[x][y] = Zborder;
      lev.drawbuf[x][y] = Zborder;
      lev.boombuf[x][y] = Xblank;
    }

    lev.cavecol[x] = lev.cavebuf[x];
    lev.nextcol[x] = lev.nextbuf[x];
    lev.drawcol[x] = lev.drawbuf[x];
    lev.boomcol[x] = lev.boombuf[x];
  }

  lev.cave = lev.cavecol;
  lev.next = lev.nextcol;
  lev.draw = lev.drawcol;
  lev.boom = lev.boomcol;
}

void em_open_all(void)
{
  InitGraphicInfo_EM();

  game_init_random();
  game_init_cave_buffers();
}

void em_close_all(void)
{
}

void init_native_sounds(void)
{
  int i;

  for (i = 0; i < SOUND_MAX; i++)
  {
    native_sound_active[i] = FALSE;
    native_sound_element[i] = -1;
  }
}

void play_native_sounds(void)
{
  int i;

  for (i = 0; i < SOUND_MAX; i++)
    if (native_sound_active[i])
      PlayLevelSound_EM(-1, -1, native_sound_element[i], i);
}

static void mark_native_sound_active(int x, int y, int sample, int element)
{
  int left = game_em.screen_x / TILEX;
  int top  = game_em.screen_y / TILEY;

  // do not mark sound for fields that are outside the visible screen area
  // (SCR_FIELDX/Y + 1 required, because even viewport has two half tiles)
  if (x < left || x >= left + SCR_FIELDX + 1 ||
      y < top  || y >= top  + SCR_FIELDY + 1)
    return;

  native_sound_active[sample] = TRUE;
  native_sound_element[sample] = element;
}

void play_element_sound(int x, int y, int sample, int element)
{
  if (game.use_native_emc_sound_engine)
    mark_native_sound_active(x, y, sample, element);
  else
    PlayLevelSound_EM(CAVE_POS_X(x), CAVE_POS_Y(y), element, sample);
}

void play_sound(int x, int y, int sample)
{
  play_element_sound(x, y, sample, -1);
}

unsigned int InitEngineRandom_EM(int seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    int simple_rnd = GetSimpleRandom(1000);
    int i;

    for (i = 0; i < simple_rnd || game_em.random == NEW_RANDOMIZE; i++)
      game_em.random = game_em.random * 129 + 1;

    seed = game_em.random;
  }

  game_em.random = seed;

  return (unsigned int)seed;
}
