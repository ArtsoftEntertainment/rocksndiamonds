/*
 * Copyright (c) 2007, 2008, 2009, Czirkos Zoltan <cirix@fw.hu>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <glib.h>

#include "main_bd.h"


/*
  The C64 sound chip (the SID) had 3 channels. Boulder Dash used all 3 of them.

  Different channels were used for different sounds.

  Channel 1: other small sounds, ie. diamonds falling, boulders rolling.

  Channel 2: Walking, diamond collecting and explosion; also time running out
             sound.

  Channel 3: amoeba sound, magic wall sound, cave cover & uncover sound, and
             the crack sound (gate open)

  Sounds have precedence over each other. Ie. the crack sound is given
  precedence over other sounds (amoeba, for example.)
  Different channels also behave differently. Channel 2 sounds are not stopped,
  ie. walking can not be heard, until the explosion sound is finished playing
  completely.

  Explosions are always restarted, though. This is controlled by the array
  defined in cave.c.

  Channel 1 sounds are always stopped, if a new sound is requested.

  Here we implement this a bit differently. We use samples, instead of
  synthesizing the sounds. By stopping samples, sometimes small clicks
  generate. Therefore we do not stop them, rather fade them out quickly.
  (The SID had filters, which stopped these small clicks.)

  Also, channel 1 and 2 should be stopped very often. So I decided to use two
  SDL_Mixer channels to emulate one C64 channel; and they are used alternating.
  SDL channel 4 is the "backup" channel 1, channel 5 is the backup channel 2.
  Other channels have the same indexes.
*/


#define MAX_CHANNELS		5

typedef enum _sound_flag
{
  GD_SP_LOOPED  = 1 << 0,
  GD_SP_FORCE   = 1 << 1,  /* force restart, regardless of precedence level */
} GdSoundFlag;

typedef struct _sound_property
{
  GdSound sound;
  int channel;        /* channel this sound is played on. */
  int precedence;     /* greater numbers will have precedence. */
  int flags;
} SoundProperty;

static SoundProperty sound_flags[] =
{
  { 0, GD_S_NONE,		0, 0	},

  /* channel 1 sounds. */
  /* CHANNEL 1 SOUNDS ARE ALWAYS RESTARTED, so no need for GD_SP_FORCE flag. */
  { GD_S_STONE,			1, 10	},
  { GD_S_NUT,			1, 8	}, /* nut falling is relatively silent, so low precedence. */
  { GD_S_NUT_CRACK,		1, 12	}, /* higher precedence than a stone bouncing. */
  { GD_S_DIRT_BALL,		1, 8	}, /* sligthly lower precedence, as stones and diamonds should be "louder" */
  { GD_S_NITRO,			1, 10	},
  { GD_S_FALLING_WALL,		1, 10	},
  { GD_S_EXPANDING_WALL,	1, 10	},
  { GD_S_WALL_REAPPEAR,		1, 9	},
  { GD_S_DIAMOND_RANDOM,	1, 10	},
  { GD_S_DIAMOND_1,		1, 10	},
  { GD_S_DIAMOND_2,		1, 10	},
  { GD_S_DIAMOND_3,		1, 10	},
  { GD_S_DIAMOND_4,		1, 10	},
  { GD_S_DIAMOND_5,		1, 10	},
  { GD_S_DIAMOND_6,		1, 10	},
  { GD_S_DIAMOND_7,		1, 10	},
  { GD_S_DIAMOND_8,		1, 10	},
  { GD_S_DIAMOND_COLLECT,	1, 100	}, /* diamond collect sound has precedence over everything. */

  /* collect sounds have higher precedence than falling sounds and the like. */
  { GD_S_SKELETON_COLLECT,	1, 100	},
  { GD_S_PNEUMATIC_COLLECT,	1, 50	},
  { GD_S_BOMB_COLLECT,		1, 50	},
  { GD_S_CLOCK_COLLECT,		1, 50	},
  { GD_S_SWEET_COLLECT,		1, 50	},
  { GD_S_KEY_COLLECT,		1, 50	},
  { GD_S_DIAMOND_KEY_COLLECT,	1, 50	},
  { GD_S_SLIME,			1, 5	}, /* slime has lower precedence than diamond and stone falling sounds. */
  { GD_S_LAVA,			1, 5	}, /* lava has low precedence, too. */
  { GD_S_REPLICATOR,		1, 5	},
  { GD_S_ACID_SPREAD,		1, 3	}, /* same for acid, even lower. */
  { GD_S_BLADDER_MOVE,		1, 5	}, /* same for bladder. */
  { GD_S_BLADDER_CONVERT,	1, 8	},
  { GD_S_BLADDER_SPENDER,	1, 8	},
  { GD_S_BITER_EAT,		1, 3	}, /* very low precedence. biters tend to produce too much sound. */

  /* channel2 sounds. */
  { GD_S_DOOR_OPEN,		2, 10	},
  { GD_S_WALK_EARTH,		2, 10	},
  { GD_S_WALK_EMPTY,		2, 10	},
  { GD_S_STIRRING,		2, 10	},
  { GD_S_BOX_PUSH,		2, 10	},
  { GD_S_TELEPORTER,		2, 10	},
  { GD_S_TIMEOUT_10,		2, 20	}, /* timeout sounds have increasing precedence so they are always started */
  { GD_S_TIMEOUT_9,		2, 21	}, /* timeout sounds are examples which do not need "force restart" flag. */
  { GD_S_TIMEOUT_8,		2, 22	},
  { GD_S_TIMEOUT_7,		2, 23	},
  { GD_S_TIMEOUT_6,		2, 24	},
  { GD_S_TIMEOUT_5,		2, 25	},
  { GD_S_TIMEOUT_4,		2, 26	},
  { GD_S_TIMEOUT_3,		2, 27	},
  { GD_S_TIMEOUT_2,		2, 28	},
  { GD_S_TIMEOUT_1,		2, 29	},
  { GD_S_TIMEOUT_0,		2, 150, GD_SP_FORCE	},
  { GD_S_EXPLOSION,		2, 100, GD_SP_FORCE	},
  { GD_S_BOMB_EXPLOSION,	2, 100, GD_SP_FORCE	},
  { GD_S_GHOST_EXPLOSION,	2, 100, GD_SP_FORCE	},
  { GD_S_VOODOO_EXPLOSION,	2, 100, GD_SP_FORCE	},
  { GD_S_NITRO_EXPLOSION,	2, 100, GD_SP_FORCE	},
  { GD_S_BOMB_PLACE,		2, 10	},
  { GD_S_FINISHED,		2, 15,  GD_SP_FORCE | GD_SP_LOOPED	}, /* precedence larger than normal, but smaller than timeout sounds */
  { GD_S_SWITCH_BITER,		2, 10	},
  { GD_S_SWITCH_CREATURES,	2, 10	},
  { GD_S_SWITCH_GRAVITY,	2, 10	},
  { GD_S_SWITCH_EXPANDING,	2, 10	},
  { GD_S_SWITCH_CONVEYOR,	2, 10	},
  { GD_S_SWITCH_REPLICATOR,	2, 10	},

  /* channel 3 sounds. */
  { GD_S_AMOEBA,		3, 30,  GD_SP_LOOPED	},
  { GD_S_AMOEBA_MAGIC,		3, 40,  GD_SP_LOOPED	},
  { GD_S_MAGIC_WALL,		3, 35,  GD_SP_LOOPED	},
  { GD_S_COVER,			3, 100, GD_SP_LOOPED	},
  { GD_S_PNEUMATIC_HAMMER,	3, 50,  GD_SP_LOOPED	},
  { GD_S_WATER,			3, 20,  GD_SP_LOOPED	},
  { GD_S_CRACK,			3, 150	},
  { GD_S_GRAVITY_CHANGE,	3, 60	},

  /* other sounds */
  /* the bonus life sound has nothing to do with the cave. */
  /* playing on channel 4. */
  { GD_S_BONUS_LIFE,		4, 0	},
};

struct GdSoundInfo
{
  int x, y;
  int element;
  int sound;
};

static GdSound snd_playing[MAX_CHANNELS];
struct GdSoundInfo sound_info_to_play[MAX_CHANNELS];
struct GdSoundInfo sound_info_playing[MAX_CHANNELS];

static boolean gd_sound_is_looped(GdSound sound)
{
  return (sound_flags[sound].flags & GD_SP_LOOPED) != 0;
}

static boolean gd_sound_force_start(GdSound sound)
{
  return (sound_flags[sound].flags & GD_SP_FORCE) != 0;
}

static int gd_sound_get_channel(GdSound sound)
{
  return sound_flags[sound].channel;
}

static int gd_sound_get_precedence(GdSound sound)
{
  return sound_flags[sound].precedence;
}

static GdSound sound_playing(int channel)
{
  struct GdSoundInfo *si = &sound_info_playing[channel];

  // check if sound has finished playing
  if (snd_playing[channel] != GD_S_NONE)
    if (!isSoundPlaying_BD(si->element, si->sound))
      snd_playing[channel] = GD_S_NONE;

  return snd_playing[channel];
}

static void halt_channel(int channel)
{
  struct GdSoundInfo *si = &sound_info_playing[channel];

#if 0
  if (isSoundPlaying_BD(si->element, si->sound))
    printf("::: stop sound %d\n", si->sound);
#endif

  if (isSoundPlaying_BD(si->element, si->sound))
    StopSound_BD(si->element, si->sound);

  snd_playing[channel] = GD_S_NONE;
}

static void play_channel_at_position(int channel)
{
  struct GdSoundInfo *si = &sound_info_to_play[channel];

  PlayLevelSound_BD(si->x, si->y, si->element, si->sound);

  sound_info_playing[channel] = *si;
}

static void play_sound(int channel, GdSound sound)
{
  /* channel 1 and channel 4 are used alternating */
  /* channel 2 and channel 5 are used alternating */
  static const GdSound diamond_sounds[] =
  {
    GD_S_DIAMOND_1,
    GD_S_DIAMOND_2,
    GD_S_DIAMOND_3,
    GD_S_DIAMOND_4,
    GD_S_DIAMOND_5,
    GD_S_DIAMOND_6,
    GD_S_DIAMOND_7,
    GD_S_DIAMOND_8,
  };

  if (sound == GD_S_NONE)
    return;

  /* change diamond falling random to a selected diamond falling sound. */
  if (sound == GD_S_DIAMOND_RANDOM)
    sound = diamond_sounds[g_random_int_range(0, G_N_ELEMENTS(diamond_sounds))];

  /* channel 1 may have been changed to channel 4 above. */

  if (!gd_sound_is_looped(sound))
    halt_channel(channel);

  play_channel_at_position(channel);

  snd_playing[channel] = sound;
}

void gd_sound_init(void)
{
  int i;

  for (i = 0; i < MAX_CHANNELS; i++)
    snd_playing[i] = GD_S_NONE;
}

void gd_sound_off(void)
{
  int i;

  /* stop all sounds. */
  for (i = 0; i < G_N_ELEMENTS(snd_playing); i++)
    halt_channel(i);
}

void gd_sound_play_bonus_life(void)
{
  // required to set extended sound information for native sound engine
  gd_sound_play(NULL, GD_S_BONUS_LIFE, O_NONE, -1, -1);

  // now play the sound directly (on non-standard sound channel)
  play_sound(gd_sound_get_channel(GD_S_BONUS_LIFE), GD_S_BONUS_LIFE);
}

static void play_sounds(GdSound sound1, GdSound sound2, GdSound sound3)
{
  /* CHANNEL 1 is for small sounds */
  if (sound1 != GD_S_NONE)
  {
    /* start new sound if higher or same precedence than the one currently playing */
    if (gd_sound_get_precedence(sound1) >= gd_sound_get_precedence(sound_playing(1)))
      play_sound(1, sound1);
  }
  else
  {
    /* only interrupt looped sounds. non-looped sounds will go away automatically. */
    if (gd_sound_is_looped(sound_playing(1)))
      halt_channel(1);
  }

  /* CHANNEL 2 is for walking, explosions */
  /* if no sound requested, do nothing. */
  if (sound2 != GD_S_NONE)
  {
    boolean play = FALSE;

    /* always start if not currently playing a sound. */
    if (sound_playing(2) == GD_S_NONE ||
	gd_sound_force_start(sound2) ||
	gd_sound_get_precedence(sound2) > gd_sound_get_precedence(sound_playing(2)))
      play = TRUE;

    /* if figured out to play: do it. */
    /* (if the requested sound is looped, this is required to continue playing it) */
    if (play)
      play_sound(2, sound2);
  }
  else
  {
    /* only interrupt looped sounds. non-looped sounds will go away automatically. */
    if (gd_sound_is_looped(sound_playing(2)))
      halt_channel(2);
  }

  /* CHANNEL 3 is for crack sound, amoeba and magic wall. */
  if (sound3 != GD_S_NONE)
  {
    boolean play = TRUE;

    /* if requests a non-looped sound, play that immediately.
       that can be a crack sound, gravity change, new life, ... */
    /* do not interrupt the previous sound, if it is non-looped.
       later calls of this function will probably contain the same sound3,
       and then it will be set. */
    if (!gd_sound_is_looped(sound_playing(3)) &&
	gd_sound_is_looped(sound3) &&
	sound_playing(3) != GD_S_NONE)
      play = FALSE;

    /* if figured out to play: do it. */
    if (play)
      play_sound(3, sound3);
  }
  else
  {
    /* sound3 = none, so interrupt sound requested. */
    /* only interrupt looped sounds. non-looped sounds will go away automatically. */
    if (gd_sound_is_looped(sound_playing(3)))
      halt_channel(3);
  }
}

void gd_sound_play_cave(GdCave *cave)
{
  play_sounds(cave->sound1, cave->sound2, cave->sound3);
}

static void gd_sound_info_to_play(int channel, int x, int y, int element, int sound)
{
  struct GdSoundInfo *si = &sound_info_to_play[channel];

  si->x = x;
  si->y = y;
  si->element = element;
  si->sound = sound;
}

/* plays sound in a cave */
void gd_sound_play(GdCave *cave, GdSound sound, GdElement element, int x, int y)
{
  if (sound == GD_S_NONE)
    return;

  // do not play sounds when fast-forwarding until player hatched
  if (setup.bd_skip_hatching && !game_bd.game->cave->hatched &&
      game_bd.game->state_counter == GAME_INT_CAVE_RUNNING)
    return;

  // if no player position specified, use middle of the screen position
  if (x == -1 && y == -1)
  {
    x = get_play_area_w() / 2 + get_scroll_x();
    y = get_play_area_h() / 2 + get_scroll_y();
  }

  if (!game.use_native_bd_sound_engine)
  {
    // when not using native sound engine, just play the sound
    PlayLevelSound_BD(x, y, element, sound);

    return;
  }

  GdSound *s = &sound;		// use reliable default value
  int channel = gd_sound_get_channel(sound);

  switch (channel)
  {
    case 1: s = &cave->sound1; break;
    case 2: s = &cave->sound2; break;
    case 3: s = &cave->sound3; break;
    default: break;
  }

  if (gd_sound_get_precedence(sound) >= gd_sound_get_precedence(*s))
  {
    // set sound
    *s = sound;

    // set extended sound information for native sound engine
    gd_sound_info_to_play(channel, x, y, element, sound);
  }
}
