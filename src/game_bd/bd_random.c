/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/* Originally developed and coded by Makoto Matsumoto and Takuji
 * Nishimura.  Please mail <matumoto@math.keio.ac.jp>, if you're using
 * code from this file in your own programs or libraries.
 * Further information on the Mersenne Twister can be found at
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 * This code was adapted to glib by Sebastian Wilhelmi.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#if defined(PLATFORM_WINDOWS)
#include <process.h>	/* for getpid() */
#endif

#include "main_bd.h"


/**
 * GdRand:
 *
 * The GdRand struct is an opaque data structure. It should only be
 * accessed through the gd_rand_* functions.
 **/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A   0x9908b0df /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

struct _GdRand
{
  unsigned int mt[N]; /* the array for the state vector  */
  unsigned int mti;
};

/**
 * gd_rand_new_with_seed: (constructor)
 * @seed: a value to initialize the random number generator
 *
 * Creates a new random number generator initialized with @seed.
 *
 * Returns: (transfer full): the new #GdRand
 **/
GdRand *
gd_rand_new_with_seed (unsigned int seed)
{
  GdRand *rand = checked_calloc(sizeof(GdRand));
  gd_rand_set_seed (rand, seed);
  return rand;
}

/**
 * gd_rand_new_with_seed_array: (constructor)
 * @seed: an array of seeds to initialize the random number generator
 * @seed_length: an array of seeds to initialize the random number
 *     generator
 *
 * Creates a new random number generator initialized with @seed.
 *
 * Returns: (transfer full): the new #GdRand
 *
 * Since: 2.4
 */
GdRand *
gd_rand_new_with_seed_array (const unsigned int *seed, unsigned int seed_length)
{
  GdRand *rand = checked_calloc(sizeof(GdRand));
  gd_rand_set_seed_array (rand, seed, seed_length);

  return rand;
}

/**
 * gd_rand_new: (constructor)
 *
 * Creates a new random number generator initialized with a seed taken
 * either from `/dev/urandom` (if existing) or from the current time
 * (as a fallback).
 *
 * On Windows, the seed is taken from rand_s().
 *
 * Returns: (transfer full): the new #GdRand
 */
GdRand *
gd_rand_new (void)
{
  unsigned int seed[4];

#if defined(PLATFORM_UNIX)
  static boolean dev_urandom_exists = TRUE;

  if (dev_urandom_exists)
  {
    FILE *dev_urandom;

    do
    {
      dev_urandom = fopen ("/dev/urandom", "rbe");
    }
    while (dev_urandom == NULL && errno == EINTR);

    if (dev_urandom)
    {
      int r;

      setvbuf (dev_urandom, NULL, _IONBF, 0);

      do
      {
        errno = 0;
        r = fread (seed, sizeof (seed), 1, dev_urandom);
      }
      while (errno == EINTR);

      if (r != 1)
        dev_urandom_exists = FALSE;

      fclose (dev_urandom);
    }
    else
    {
      dev_urandom_exists = FALSE;
    }
  }

  if (!dev_urandom_exists)
  {
    struct timespec now;

    clock_gettime(CLOCK_REALTIME, &now);

    seed[0] = now.tv_sec;
    seed[1] = now.tv_nsec;
    seed[2] = getpid ();
    seed[3] = getppid ();
  }
#else /* PLATFORM_WINDOWS */
  /* rand_s() is only available since Visual Studio 2005 and
   * MinGW-w64 has a wrapper that will emulate rand_s() if it's not in msvcrt
   */
#if (defined(_MSC_VER) && _MSC_VER >= 1400) || defined(__MINGW64_VERSION_MAJOR)
  size_t i;

  for (i = 0; i < ARRAY_SIZE(seed); i++)
    rand_s (&seed[i]);
#else
#warning Using insecure seed for random number generation because of missing rand_s() in Windows XP
  GTimeVal now;

  gd_get_current_time (&now);

  seed[0] = now.tv_sec;
  seed[1] = now.tv_usec;
  seed[2] = getpid ();
  seed[3] = 0;
#endif

#endif

  return gd_rand_new_with_seed_array (seed, 4);
}

/**
 * gd_rand_free:
 * @rand_: a #GdRand
 *
 * Frees the memory allocated for the #GdRand.
 */
void
gd_rand_free (GdRand *rand)
{
  if (rand != NULL)
    checked_free(rand);
}

/**
 * gd_rand_copy:
 * @rand_: a #GdRand
 *
 * Copies a #GdRand into a new one with the same exact state as before.
 * This way you can take a snapshot of the random number generator for
 * replaying later.
 *
 * Returns: (transfer full): the new #GdRand
 *
 * Since: 2.4
 */
GdRand *
gd_rand_copy (GdRand *rand)
{
  GdRand *new_rand;

  if (rand == NULL)
    return NULL;

  new_rand = checked_calloc(sizeof(GdRand));
  memcpy(new_rand, rand, sizeof(GdRand));

  return new_rand;
}

/**
 * gd_rand_set_seed:
 * @rand_: a #GdRand
 * @seed: a value to reinitialize the random number generator
 *
 * Sets the seed for the random number generator #GdRand to @seed.
 */
void
gd_rand_set_seed (GdRand *rand, unsigned int seed)
{
  if (rand == NULL)
    return;

  /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
  /* In the previous version (see above), MSBs of the    */
  /* seed affect only MSBs of the array mt[].            */

  rand->mt[0] = seed;
  for (rand->mti = 1; rand->mti < N; rand->mti++)
    rand->mt[rand->mti] = 1812433253UL *
      (rand->mt[rand->mti - 1] ^ (rand->mt[rand->mti - 1] >> 30)) + rand->mti;
}

/**
 * gd_rand_set_seed_array:
 * @rand_: a #GdRand
 * @seed: array to initialize with
 * @seed_length: length of array
 *
 * Initializes the random number generator by an array of longs.
 * Array can be of arbitrary size, though only the first 624 values
 * are taken.  This function is useful if you have many low entropy
 * seeds, or if you require more then 32 bits of actual entropy for
 * your application.
 *
 * Since: 2.4
 */
void
gd_rand_set_seed_array (GdRand *rand, const unsigned int *seed, unsigned int seed_length)
{
  unsigned int i, j, k;

  if (rand == NULL || seed_length < 1)
    return;

  gd_rand_set_seed (rand, 19650218UL);

  i = 1;
  j = 0;
  k = (N > seed_length ? N : seed_length);

  for (; k; k--)
  {
    rand->mt[i] = ((rand->mt[i] ^ ((rand->mt[i - 1] ^ (rand->mt[i - 1] >> 30)) * 1664525UL))
		   + seed[j] + j);	/* non linear */
    rand->mt[i] &= 0xffffffffUL;	/* for WORDSIZE > 32 machines */
    i++;
    j++;

    if (i >= N)
    {
      rand->mt[0] = rand->mt[N - 1];
      i = 1;
    }

    if (j >= seed_length)
      j = 0;
  }

  for (k = N - 1; k; k--)
  {
    rand->mt[i] = ((rand->mt[i] ^ ((rand->mt[i - 1] ^ (rand->mt[i - 1] >> 30)) * 1566083941UL))
		   - i);		/* non linear */
    rand->mt[i] &= 0xffffffffUL;	/* for WORDSIZE > 32 machines */
    i++;

    if (i >= N)
    {
      rand->mt[0] = rand->mt[N - 1];
      i = 1;
    }
  }

  rand->mt[0] = 0x80000000UL;		/* MSB is 1; assuring non-zero initial array */
}

/**
 * gd_rand_boolean:
 * @rand_: a #GdRand
 *
 * Returns a random #boolean from @rand_.
 * This corresponds to an unbiased coin toss.
 *
 * Returns: a random #boolean
 */
/**
 * gd_rand_int:
 * @rand_: a #GdRand
 *
 * Returns the next random unsigned int from @rand_ equally distributed over
 * the range [0..2^32-1].
 *
 * Returns: a random number
 */
unsigned int
gd_rand_int (GdRand *rand)
{
  unsigned int y;
  static const unsigned int mag01[2] = { 0x0, MATRIX_A };
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (rand == NULL)
    return 0;

  if (rand->mti >= N)
  {
    /* generate N words at one time */
    int kk;

    for (kk = 0; kk < N - M; kk++)
    {
      y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
      rand->mt[kk] = rand->mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
    }

    for (; kk < N - 1; kk++)
    {
      y = (rand->mt[kk] & UPPER_MASK) | (rand->mt[kk + 1] & LOWER_MASK);
      rand->mt[kk] = rand->mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }

    y = (rand->mt[N - 1] & UPPER_MASK) | (rand->mt[0] & LOWER_MASK);
    rand->mt[N - 1] = rand->mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

    rand->mti = 0;
  }

  y = rand->mt[rand->mti++];
  y ^= TEMPERING_SHIFT_U(y);
  y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L(y);

  return y;
}

/**
 * gd_rand_int_range:
 * @rand_: a #GdRand
 * @begin: lower closed bound of the interval
 * @end: upper open bound of the interval
 *
 * Returns the next random #int from @rand_ equally distributed over
 * the range [@begin..@end-1].
 *
 * Returns: a random number
 */
int
gd_rand_int_range (GdRand *rand, int begin, int end)
{
  unsigned int dist = end - begin;
  unsigned int random = 0;

  if (rand == NULL || end <= begin)
    return begin;

  if (dist == 0)
    return begin;

  /* maxvalue is set to the predecessor of the greatest
   * multiple of dist less or equal 2^32.
   */
  unsigned int maxvalue;
  if (dist <= 0x80000000u) /* 2^31 */
  {
    /* maxvalue = 2^32 - 1 - (2^32 % dist) */
    unsigned int leftover = (0x80000000u % dist) * 2;
    if (leftover >= dist) leftover -= dist;
    maxvalue = 0xffffffffu - leftover;
  }
  else
  {
    maxvalue = dist - 1;
  }

  do
    random = gd_rand_int (rand);
  while (random > maxvalue);

  random %= dist;

  return begin + random;
}

static GdRand *
get_global_random (void)
{
  static GdRand *global_random;

  /* called while locked */
  if (!global_random)
    global_random = gd_rand_new();

  return global_random;
}

/**
 * gd_random_int_range:
 * @begin: lower closed bound of the interval
 * @end: upper open bound of the interval
 *
 * Returns a random #int equally distributed over the range
 * [@begin..@end-1].
 *
 * Returns: a random number
 */
int
gd_random_int_range (int begin, int end)
{
  int result;

  result = gd_rand_int_range (get_global_random(), begin, end);

  return result;
}
