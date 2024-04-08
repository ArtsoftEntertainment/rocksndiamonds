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

#ifndef BD_RANDOM_H
#define BD_RANDOM_H


typedef struct _GdRand GdRand;

/* GdRand - a good and fast random number generator: Mersenne Twister
 * see http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html for more info.
 * The range functions return a value in the interval [begin, end).
 * int          -> [0..2^32-1]
 * int_range    -> [begin..end-1]
 * double       -> [0..1)
 * double_range -> [begin..end)
 */

GdRand      *gd_rand_new_with_seed(unsigned int seed);
GdRand      *gd_rand_new_with_seed_array(const unsigned int *seed, unsigned int seed_length);
GdRand      *gd_rand_new(void);
void         gd_rand_free(GdRand *rand);
GdRand      *gd_rand_copy(GdRand *rand);
void         gd_rand_set_seed(GdRand *rand, unsigned int seed);
void         gd_rand_set_seed_array(GdRand *rand_, const unsigned int *seed, unsigned int seed_length);
#define      gd_rand_boolean(rand_) ((gd_rand_int (rand_) & (1 << 15)) != 0)
unsigned int gd_rand_int(GdRand *rand_);
int          gd_rand_int_range(GdRand *rand_, int begin, int end);
double       gd_rand_double(GdRand *rand_);
double       gd_rand_double_range(GdRand *rand_, double begin, double end);

void         gd_random_set_seed(unsigned int seed);
#define      gd_random_boolean() ((gd_random_int () & (1 << 15)) != 0)
unsigned int gd_random_int(void);
int          gd_random_int_range(int begin, int end);

double       gd_random_double(void);
double       gd_random_double_range(double  begin, double  end);

#endif	// BD_RANDOM_H
