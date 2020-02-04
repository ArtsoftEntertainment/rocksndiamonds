/* 2000-08-10T18:03:54Z
 *
 * open X11 display and sound
 */

#include "main_em.h"


Bitmap *screenBitmap;

struct GlobalInfo_EM global_em_info;
struct GameInfo_EM game_em;

void InitGfxBuffers_EM(void)
{
  ReCreateBitmap(&screenBitmap, MAX_BUF_XSIZE * TILEX, MAX_BUF_YSIZE * TILEY);

  global_em_info.screenbuffer = screenBitmap;
}

void em_open_all(void)
{
  /* pre-calculate some data */
  tab_generate();

  /* initialize graphics */
  InitGraphicInfo_EM();

  game_init_random();
  game_init_cave_buffers();
}

void em_close_all(void)
{
}

/* ---------------------------------------------------------------------- */

extern int screen_x;
extern int screen_y;

void play_element_sound(int x, int y, int sample, int element)
{
  PlayLevelSound_EM(x, y, element, sample);
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

    for (i = 0; i < simple_rnd || RandomEM == NEW_RANDOMIZE; i++)
      RandomEM = RandomEM * 129 + 1;

    seed = RandomEM;
  }

  RandomEM = seed;

  return (unsigned int) seed;
}
