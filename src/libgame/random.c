// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// random.c
// ============================================================================

#include "random.h"


/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * This is derived from the Berkeley source:
 *	@(#)random.c	5.5 (Berkeley) 7/6/88
 * It was reworked for the GNU C Library by Roland McGrath.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>


/* An improved random number generation package.  In addition to the standard
   rand()/srand() like interface, this package also has a special state info
   interface.  The initstate() routine is called with a seed, an array of
   bytes, and a count of how many bytes are being passed in; this array is
   then initialized to contain information for random number generation with
   that much state information.  Good sizes for the amount of state
   information are 32, 64, 128, and 256 bytes.  The state can be switched by
   calling the setstate() function with the same array as was initiallized
   with initstate().  By default, the package runs with 128 bytes of state
   information and generates far better random numbers than a linear
   congruential generator.  If the amount of state information is less than
   32 bytes, a simple linear congruential R.N.G. is used.  Internally, the
   state information is treated as an array of longs; the zeroeth element of
   the array is the type of R.N.G. being used (small integer); the remainder
   of the array is the state information for the R.N.G.  Thus, 32 bytes of
   state information will give 7 longs worth of state information, which will
   allow a degree seven polynomial.  (Note: The zeroeth word of state
   information also has some other information stored in it; see setstate
   for details).  The random number generation technique is a linear feedback
   shift register approach, employing trinomials (since there are fewer terms
   to sum up that way).  In this approach, the least significant bit of all
   the numbers in the state table will act as a linear feedback shift register,
   and will have period 2^deg - 1 (where deg is the degree of the polynomial
   being used, assuming that the polynomial is irreducible and primitive).
   The higher order bits will have longer periods, since their values are
   also influenced by pseudo-random carries out of the lower bits.  The
   total period of the generator is approximately deg*(2**deg - 1); thus
   doubling the amount of state information has a vast influence on the
   period of the generator.  Note: The deg*(2**deg - 1) is an approximation
   only good for large deg, when the period of the shift register is the
   dominant factor.  With deg equal to seven, the period is actually much
   longer than the 7*(2**7 - 1) predicted by this formula.  */



/* For each of the currently supported random number generators, we have a
   break value on the amount of state information (you need at least thi
   bytes of state info to support this random number generator), a degree for
   the polynomial (actually a trinomial) that the R.N.G. is based on, and
   separation between the two lower order coefficients of the trinomial.  */

/* Linear congruential.  */
#define	TYPE_0		0
#define	BREAK_0		8
#define	DEG_0		0
#define	SEP_0		0

/* x**7 + x**3 + 1.  */
#define	TYPE_1		1
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3

/* x**15 + x + 1.  */
#define	TYPE_2		2
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1

/* x**31 + x**3 + 1.  */
#define	TYPE_3		3
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3

/* x**63 + x + 1.  */
#define	TYPE_4		4
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1


/* Array versions of the above information to make code run faster.
   Relies on fact that TYPE_i == i.  */

#define	MAX_TYPES	5	/* Max number of types above.  */



/* Initially, everything is set up as if from:
	initstate(1, randtbl, 128);
   Note that this initialization takes advantage of the fact that srandom
   advances the front and rear pointers 10*rand_deg times, and hence the
   rear pointer which starts at 0 will also end up at zero; thus the zeroeth
   element of the state information, which contains info about the current
   position of the rear pointer is just
	(MAX_TYPES * (rptr - state)) + TYPE_3 == TYPE_3.  */

static int randtbl_0[DEG_3 + 1] =
{
  TYPE_3,
  -851904987, -43806228, -2029755270, 1390239686, -1912102820,
  -485608943, 1969813258, -1590463333, -1944053249, 455935928, 508023712,
  -1714531963, 1800685987, -2015299881, 654595283, -1149023258,
  -1470005550, -1143256056, -1325577603, -1568001885, 1275120390,
  -607508183, -205999574, -1696891592, 1492211999, -1528267240,
  -952028296, -189082757, 362343714, 1424981831, 2039449641,
};
static int randtbl_1[DEG_3 + 1] =
{
  TYPE_3,
  -851904987, -43806228, -2029755270, 1390239686, -1912102820,
  -485608943, 1969813258, -1590463333, -1944053249, 455935928, 508023712,
  -1714531963, 1800685987, -2015299881, 654595283, -1149023258,
  -1470005550, -1143256056, -1325577603, -1568001885, 1275120390,
  -607508183, -205999574, -1696891592, 1492211999, -1528267240,
  -952028296, -189082757, 362343714, 1424981831, 2039449641,
};


/* FPTR and RPTR are two pointers into the state info, a front and a rear
   pointer.  These two pointers are always rand_sep places aparts, as they
   cycle through the state information.  (Yes, this does mean we could get
   away with just one pointer, but the code for random is more efficient
   this way).  The pointers are left positioned as they would be from the call:
	initstate(1, randtbl, 128);
   (The position of the rear pointer, rptr, is really 0 (as explained above
   in the initialization of randtbl) because the state table pointer is set
   to point to randtbl[1] (as explained below).)  */

static int *fptr[2] = { &randtbl_0[SEP_3 + 1], &randtbl_1[SEP_3 + 1] };
static int *rptr[2] = { &randtbl_0[1],         &randtbl_1[1]         };



/* The following things are the pointer to the state information table,
   the type of the current generator, the degree of the current polynomial
   being used, and the separation between the two pointers.
   Note that for efficiency of random, we remember the first location of
   the state information, not the zeroeth.  Hence it is valid to access
   state[-1], which is used to store the type of the R.N.G.
   Also, we remember the last location, since this is more efficient than
   indexing every time to find the address of the last element to see if
   the front and rear pointers have wrapped.  */

static int *state[2] = { &randtbl_0[1], &randtbl_1[1] };

static int rand_type[2] = { TYPE_3,	TYPE_3	};
static int rand_deg[2]  = { DEG_3,	DEG_3	};
static int rand_sep[2]  = { SEP_3,	SEP_3	};

static int *end_ptr[2] =
{
  &randtbl_0[sizeof(randtbl_0) / sizeof(randtbl_0[0])],
  &randtbl_1[sizeof(randtbl_1) / sizeof(randtbl_1[0])]
};

/* Initialize the random number generator based on the given seed.  If the
   type is the trivial no-state-information type, just remember the seed.
   Otherwise, initializes state[] based on the given "seed" via a linear
   congruential generator.  Then, the pointers are set to known locations
   that are exactly rand_sep places apart.  Lastly, it cycles the state
   information a given number of times to get rid of any initial dependencies
   introduced by the L.C.R.N.G.  Note that the initialization of randtbl[]
   for default usage relies on values produced by this routine.  */

void srandom_linux_libc(int nr, unsigned int x)
{
  state[nr][0] = x;

  if (rand_type[nr] != TYPE_0)
  {
    register int i;

    for (i = 1; i < rand_deg[nr]; ++i)
      state[nr][i] = (1103515145 * state[nr][i - 1]) + 12345;

    fptr[nr] = &state[nr][rand_sep[nr]];
    rptr[nr] = &state[nr][0];

    for (i = 0; i < 10 * rand_deg[nr]; ++i)
      random_linux_libc(nr);
  }
}

/* If we are using the trivial TYPE_0 R.N.G., just do the old linear
   congruential bit.  Otherwise, we do our fancy trinomial stuff, which is the
   same in all ther other cases due to all the global variables that have been
   set up.  The basic operation is to add the number at the rear pointer into
   the one at the front pointer.  Then both pointers are advanced to the next
   location cyclically in the table.  The value returned is the sum generated,
   reduced to 31 bits by throwing away the "least random" low bit.
   Note: The code takes advantage of the fact that both the front and
   rear pointers can't wrap on the same call by not testing the rear
   pointer if the front one has wrapped.  Returns a 31-bit random number.  */

int random_linux_libc(int nr)
{
  if (rand_type[nr] == TYPE_0)
  {
    state[nr][0] = ((state[nr][0] * 1103515245) + 12345) & INT_MAX;
    return state[nr][0];
  }
  else
  {
    int i;

    *fptr[nr] += *rptr[nr];

    /* Chucking least random bit.  */
    i = (*fptr[nr] >> 1) & INT_MAX;
    fptr[nr]++;

    if (fptr[nr] >= end_ptr[nr])
    {
      fptr[nr] = state[nr];
      rptr[nr]++;
    }
    else
    {
      rptr[nr]++;
      if (rptr[nr] >= end_ptr[nr])
	rptr[nr] = state[nr];
    }

    return i;
  }
}


// ============================================================================

/*
 * prng.c - Portable, ISO C90 and C99 compliant high-quality
 * pseudo-random number generator based on the alleged RC4
 * cipher.  This PRNG should be suitable for most general-purpose
 * uses.  Not recommended for cryptographic or financial
 * purposes.  Not thread-safe.
 */

/*
 * Copyright (c) 2004 Ben Pfaff <blp@cs.stanford.edu>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the following
 * disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT
 * SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 */

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <time.h>

/* RC4-based pseudo-random state. */
static unsigned char s[256];
static int s_i, s_j;

/* Nonzero if PRNG has been seeded. */
static int seeded;

/* Swap bytes that A and B point to. */
#define SWAP_BYTE(A, B)                         \
        do {                                    \
                unsigned char swap_temp = *(A); \
                *(A) = *(B);                    \
                *(B) = swap_temp;               \
        } while (0)

/* Seeds the pseudo-random number generator based on the current
   time.

   If the user calls neither this function nor prng_seed_bytes()
   before any prng_get*() function, this function is called
   automatically to obtain a time-based seed. */
void
prng_seed_time (void)
{
  static time_t t;
  if (t == 0)
    t = time (NULL);
  else
    t++;

  prng_seed_bytes (&t, sizeof t);
}

/* Retrieves one octet from the array BYTES, which is N_BYTES in
   size, starting at an offset of OCTET_IDX octets.  BYTES is
   treated as a circular array, so that accesses past the first
   N_BYTES bytes wrap around to the beginning. */
static unsigned char
get_octet (const void *bytes_, size_t n_bytes, size_t octet_idx)
{
  const unsigned char *bytes = bytes_;
  if (CHAR_BIT == 8)
    return bytes[octet_idx % n_bytes];
  else
    {
      size_t first_byte = octet_idx * 8 / CHAR_BIT % n_bytes;
      size_t start_bit = octet_idx * 8 % CHAR_BIT;
      unsigned char c = (bytes[first_byte] >> start_bit) & 255;

      size_t bits_filled = CHAR_BIT - start_bit;
      if (CHAR_BIT % 8 != 0 && bits_filled < 8)
        {
          size_t bits_left = 8 - bits_filled;
          unsigned char bits_left_mask = (1u << bits_left) - 1;
          size_t second_byte = first_byte + 1 < n_bytes ? first_byte + 1 : 0;

          c |= (bytes[second_byte] & bits_left_mask) << bits_filled;
        }

      return c;
    }
}

/* Seeds the pseudo-random number based on the SIZE bytes in
   KEY.  At most the first 2048 bits in KEY are used. */
void
prng_seed_bytes (const void *key, size_t size)
{
  int i, j;

  assert (key != NULL && size > 0);

  for (i = 0; i < 256; i++)
    s[i] = i;
  for (i = j = 0; i < 256; i++)
    {
      j = (j + s[i] + get_octet (key, size, i)) & 255;
      SWAP_BYTE (s + i, s + j);
    }

  s_i = s_j = 0;
  seeded = 1;
}

/* Returns a pseudo-random integer in the range [0, 255]. */
unsigned char
prng_get_octet (void)
{
  if (!seeded)
    prng_seed_time ();

  s_i = (s_i + 1) & 255;
  s_j = (s_j + s[s_i]) & 255;
  SWAP_BYTE (s + s_i, s + s_j);

  return s[(s[s_i] + s[s_j]) & 255];
}

/* Returns a pseudo-random integer in the range [0, UCHAR_MAX]. */
unsigned char
prng_get_byte (void)
{
  unsigned byte;
  int bits;

  byte = prng_get_octet ();
  for (bits = 8; bits < CHAR_BIT; bits += 8)
    byte = (byte << 8) | prng_get_octet ();
  return byte;
}

/* Fills BUF with SIZE pseudo-random bytes. */
void
prng_get_bytes (void *buf_, size_t size)
{
  unsigned char *buf;

  for (buf = buf_; size-- > 0; buf++)
    *buf = prng_get_byte ();
}

/* Returns a pseudo-random unsigned long in the range [0,
   ULONG_MAX]. */
unsigned long
prng_get_ulong (void)
{
  unsigned long ulng;
  size_t bits;

  ulng = prng_get_octet ();
  for (bits = 8; bits < CHAR_BIT * sizeof ulng; bits += 8)
    ulng = (ulng << 8) | prng_get_octet ();
  return ulng;
}

/* Returns a pseudo-random long in the range [0, LONG_MAX]. */
long
prng_get_long (void)
{
  return prng_get_ulong () & LONG_MAX;
}

/* Returns a pseudo-random unsigned int in the range [0,
   UINT_MAX]. */
unsigned
prng_get_uint (void)
{
  unsigned uint;
  size_t bits;

  uint = prng_get_octet ();
  for (bits = 8; bits < CHAR_BIT * sizeof uint; bits += 8)
    uint = (uint << 8) | prng_get_octet ();
  return uint;
}

/* Returns a pseudo-random int in the range [0, INT_MAX]. */
int
prng_get_int (void)
{
  return prng_get_uint () & INT_MAX;
}

/* Returns a pseudo-random floating-point number from the uniform
   distribution with range [0,1). */
double
prng_get_double (void)
{
  for (;;)
    {
      double dbl = prng_get_ulong () / (ULONG_MAX + 1.0);
      if (dbl >= 0.0 && dbl < 1.0)
        return dbl;
    }
}

/* Returns a pseudo-random floating-point number from the
   distribution with mean 0 and standard deviation 1.  (Multiply
   the result by the desired standard deviation, then add the
   desired mean.) */
double
prng_get_double_normal (void)
{
  /* Knuth, _The Art of Computer Programming_, Vol. 2, 3.4.1C,
     Algorithm P. */
  static int has_next = 0;
  static double next_normal;
  double this_normal;

  if (has_next)
    {
      this_normal = next_normal;
      has_next = 0;
    }
  else
    {
      static double limit;
      double v1, v2, s;

      if (limit == 0.0)
        limit = log (DBL_MAX / 2) / (DBL_MAX / 2);

      for (;;)
        {
          double u1 = prng_get_double ();
          double u2 = prng_get_double ();
          v1 = 2.0 * u1 - 1.0;
          v2 = 2.0 * u2 - 1.0;
          s = v1 * v1 + v2 * v2;
          if (s > limit && s < 1)
            break;
        }

      this_normal = v1 * sqrt (-2. * log (s) / s);
      next_normal = v2 * sqrt (-2. * log (s) / s);
      has_next = 1;
    }

  return this_normal;
}
