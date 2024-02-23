// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// hash.c
// ============================================================================

/*
 * Copyright (C) 2002 Christopher Clark <firstname.lastname@cl.cam.ac.uk>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software and its documentation and acknowledgment shall be
 * given in the documentation and software packages that this Software was
 * used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hash.h"


/*****************************************************************************/
struct hashtable *
create_hashtable_ext(unsigned int minsize, float maxloadfactor,
		     unsigned int (*hashf) (void*),
		     int (*eqf) (void*, void*),
                     void (*freekfn) (void*),
                     void (*freevfn) (void*))
{
  struct hashtable *h;
  unsigned int i, size = 1u;

  /* Check requested hashtable isn't too large */
  if (minsize > (1u << 31))
    return NULL;

  /* Enforce size as power of 2 */
  while (size < minsize)
    size <<= 1;

  h = (struct hashtable *)malloc(sizeof(struct hashtable));

  if (h == NULL)
    return NULL;

  h->table = (struct entry **)malloc(sizeof(struct entry*) * size);

  if (h->table == NULL)
  {
    free(h);

    return NULL;
  }

  for (i = 0; i < size; i++)
    h->table[i] = NULL;

  h->tablelength  = size;
  h->entrycount   = 0;
  h->hashfn       = hashf;
  h->eqfn         = eqf;
  h->loadlimit    = (unsigned int) ((float)size * maxloadfactor);
  h->freekfn      = freekfn;
  h->freevfn      = freevfn;

  return h;
}

struct hashtable *
create_hashtable(unsigned int (*hashf) (void*),
                 int (*eqf) (void*, void*),
                 void (*freekfn) (void*),
                 void (*freevfn) (void*))
{
  return create_hashtable_ext(16, 0.75, hashf, eqf, freekfn, freevfn);
}

/*****************************************************************************/
static unsigned int
hash(struct hashtable *h, void *k)
{
  /* Aim to protect against poor hash functions by adding logic here
   * - logic taken from java 1.4 hashtable source */

  unsigned int i = h->hashfn(k);

  i += ~(i << 9);
  i ^=  ((i >> 14) | (i << 18)); /* >>> */
  i +=  (i << 4);
  i ^=  ((i >> 10) | (i << 22)); /* >>> */

  return i;
}

/*****************************************************************************/
static unsigned int
indexFor(unsigned int tablelength, unsigned int hashvalue)
{
  /* Only works if tablelength == 2^N */
  return (hashvalue & (tablelength - 1u));
}

/*****************************************************************************/
static int
hashtable_expand(struct hashtable *h)
{
  /* Double the size of the table to accomodate more entries */
  struct entry **newtable;
  struct entry *e;
  struct entry **pE;
  unsigned int newsize, i, index;

  /* Check we're not hitting max capacity */
  if (0 == (newsize = (h->tablelength << 1)))
    return 0;

  newtable = (struct entry **)malloc(sizeof(struct entry*) * newsize);

  if (newtable != NULL)
  {
    memset(newtable, 0, newsize * sizeof(struct entry *));

    /* This algorithm is not 'stable'. ie. it reverses the list
     * when it transfers entries between the tables */
    for (i = 0; i < h->tablelength; i++)
    {
      while ((e = h->table[i]) != NULL)
      {
	h->table[i] = e->next;
	index = indexFor(newsize, e->h);
	e->next = newtable[index];
	newtable[index] = e;
      }
    }

    free(h->table);
    h->table = newtable;
  }
  else		/* Plan B: realloc instead */
  {
    newtable = (struct entry **)
      realloc(h->table, newsize * sizeof(struct entry *));

    if (newtable == NULL)
      return 0;

    h->table = newtable;

    for (i = h->tablelength; i < newsize; i++)
      newtable[i] = NULL;

    for (i = 0; i < h->tablelength; i++)
    {
      for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE)
      {
	index = indexFor(newsize, e->h);

	if (index == i)
	{
	  pE = &(e->next);
	}
	else
	{
	  *pE = e->next;
	  e->next = newtable[index];
	  newtable[index] = e;
	}
      }
    }
  }

  h->tablelength = newsize;
  h->loadlimit <<= 1;

  return -1;
}

/*****************************************************************************/
unsigned int
hashtable_count(struct hashtable *h)
{
  return h->entrycount;
}

/*****************************************************************************/
int
hashtable_insert(struct hashtable *h, void *k, void *v)
{
  /* This method allows duplicate keys - but they shouldn't be used */
  unsigned int index;
  struct entry *e;

  if (++(h->entrycount) > h->loadlimit)
  {
    /* Ignore the return value. If expand fails, we should
     * still try cramming just this value into the existing table
     * -- we may not have memory for a larger table, but one more
     * element may be ok. Next time we insert, we'll try expanding again.*/

    hashtable_expand(h);
  }

  e = (struct entry *)malloc(sizeof(struct entry));

  if (e == NULL)
  {
    --(h->entrycount);

    return 0;
  }

  e->h = hash(h, k);
  index = indexFor(h->tablelength, e->h);
  e->k = k;
  e->v = v;
  e->next = h->table[index];
  h->table[index] = e;

  return -1;
}

/*****************************************************************************/
int
hashtable_change(struct hashtable *h, void *k, void *v)
{
  struct entry *e;
  unsigned int hashvalue, index;

  hashvalue = hash(h, k);
  index = indexFor(h->tablelength, hashvalue);
  e = h->table[index];

  while (e != NULL)
  {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (h->eqfn(k, e->k)))
    {
      if (h->freevfn != NULL)
	h->freevfn(e->v);
      e->v = v;

      return -1;
    }

    e = e->next;
  }

  return 0;
}

/*****************************************************************************/
int /* checks if key exists */
hashtable_exists(struct hashtable *h, void *k)
{
  struct entry *e;
  unsigned int hashvalue, index;

  hashvalue = hash(h, k);
  index = indexFor(h->tablelength, hashvalue);
  e = h->table[index];

  while (e != NULL)
  {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (h->eqfn(k, e->k)))
      return 1;

    e = e->next;
  }

  return 0;
}

/*****************************************************************************/
void * /* returns value associated with key */
hashtable_search(struct hashtable *h, void *k)
{
  struct entry *e;
  unsigned int hashvalue, index;

  hashvalue = hash(h, k);
  index = indexFor(h->tablelength, hashvalue);
  e = h->table[index];

  while (e != NULL)
  {
    /* Check hash value to short circuit heavier comparison */
    if ((hashvalue == e->h) && (h->eqfn(k, e->k)))
      return e->v;

    e = e->next;
  }

  return NULL;
}

/*****************************************************************************/
void * /* returns value associated with key */
hashtable_remove(struct hashtable *h, void *k)
{
  /* TODO: consider compacting the table when the load factor drops enough,
   *       or provide a 'compact' method. */

  struct entry *e;
  struct entry **pE;
  void *v;
  unsigned int index = indexFor(h->tablelength, hash(h, k));

  pE = &(h->table[index]);
  e = *pE;

  while (e != NULL)
  {
    if (h->eqfn(k, e->k))
    {
      *pE = e->next;
      h->entrycount--;
      v = NULL;
      if (h->freekfn != NULL)
	h->freekfn(e->k);
      if (h->freevfn != NULL)
	h->freevfn(e->v);
      else
	v = e->v;
      free(e);

      return v;
    }

    pE = &(e->next);
    e = e->next;
  }

  return NULL;
}

/*****************************************************************************/
/* destroy */
void
hashtable_destroy(struct hashtable *h)
{
  unsigned int i;
  struct entry *e, *f;
  struct entry **table = h->table;

  for (i = 0; i < h->tablelength; i++)
  {
    e = table[i];

    while (e != NULL)
    {
      f = e;
      e = e->next;
      if (h->freekfn != NULL)
	h->freekfn(f->k);
      if (h->freevfn != NULL)
	h->freevfn(f->v);

      free(f);
    }
  }

  free(h->table);
  free(h);
}

/*****************************************************************************/
/* hashtable_iterator    - iterator constructor */

struct hashtable_itr *
hashtable_iterator(struct hashtable *h)
{
  unsigned int i, tablelength;
  struct hashtable_itr *itr = (struct hashtable_itr *)
    malloc(sizeof(struct hashtable_itr));

  if (itr == NULL)
    return NULL;

  itr->h = h;
  itr->e = NULL;
  tablelength = h->tablelength;
  itr->index = tablelength;

  if (0 == h->entrycount)
    return itr;

  for (i = 0; i < tablelength; i++)
  {
    if (h->table[i] != NULL)
    {
      itr->e = h->table[i];
      itr->index = i;

      break;
    }
  }

  return itr;
}

/*****************************************************************************/
/* key - return the key of the (key, value) pair at the current position */

void *
hashtable_iterator_key(struct hashtable_itr *i)
{
  return i->e->k;
}

/*****************************************************************************/
/* value - return the value of the (key, value) pair at the current position */

void *
hashtable_iterator_value(struct hashtable_itr *i)
{
  return i->e->v;
}

/*****************************************************************************/
/* advance - advance the iterator to the next element
 *           returns zero if advanced to end of table */

int
hashtable_iterator_advance(struct hashtable_itr *itr)
{
  unsigned int j, tablelength;
  struct entry **table;
  struct entry *next;

  if (itr->e == NULL)
    return 0; /* stupidity check */

  next = itr->e->next;
  if (next != NULL)
  {
    itr->e = next;

    return -1;
  }

  tablelength = itr->h->tablelength;
  if (tablelength <= (j = ++(itr->index)))
  {
    itr->e = NULL;

    return 0;
  }

  table = itr->h->table;
  while ((next = table[j]) == NULL)
  {
    if (++j >= tablelength)
    {
      itr->index = tablelength;

      return 0;
    }
  }

  itr->index = j;
  itr->e = next;

  return -1;
}

/*****************************************************************************/
/* call function for all hashtable entries */
void
hashtable_foreach(struct hashtable *h, hashtable_fn fn, void *userdata)
{
  if (h == NULL)
    return;

  if (hashtable_count(h) == 0)
    return;

  struct hashtable_itr *itr = hashtable_iterator(h);

  do
  {
    fn(hashtable_iterator_key(itr), hashtable_iterator_value(itr), userdata);
  }
  while (hashtable_iterator_advance(itr));

  free(itr);
}

/*****************************************************************************/
/* call function for all hashtable entries and remove them, if function returned 1 */
unsigned int
hashtable_foreach_remove(struct hashtable *h, hashtable_remove_fn fn, void *userdata)
{
  if (h == NULL)
    return 0;

  if (hashtable_count(h) == 0)
    return 0;

  struct hashtable *remove = create_hashtable(h->hashfn, h->eqfn, NULL, NULL);
  struct hashtable_itr *itr = hashtable_iterator(h);

  do
  {
    if (fn(hashtable_iterator_key(itr), hashtable_iterator_value(itr), userdata))
      hashtable_insert(remove, hashtable_iterator_key(itr), "1");
  }
  while (hashtable_iterator_advance(itr));

  free(itr);

  struct hashtable_itr *itr_remove = hashtable_iterator(remove);
  unsigned int num_removed = 0;

  do
  {
    hashtable_remove(h, hashtable_iterator_key(itr_remove));
    num_removed++;
  }
  while (hashtable_iterator_advance(itr_remove));

  free(itr_remove);

  hashtable_destroy(remove);

  return num_removed;
}

/*****************************************************************************/
/* remove_all hashtable entries */
unsigned int
hashtable_remove_all(struct hashtable *h)
{
  /* TODO: this function should directly remove all hashtable entries */

  if (h == NULL)
    return 0;

  if (hashtable_count(h) == 0)
    return 0;

  struct hashtable *remove = create_hashtable(h->hashfn, h->eqfn, NULL, NULL);
  struct hashtable_itr *itr = hashtable_iterator(h);

  do
  {
    hashtable_insert(remove, hashtable_iterator_key(itr), "1");
  }
  while (hashtable_iterator_advance(itr));

  free(itr);

  struct hashtable_itr *itr_remove = hashtable_iterator(remove);
  unsigned int num_removed = 0;

  do
  {
    hashtable_remove(h, hashtable_iterator_key(itr_remove));
    num_removed++;
  }
  while (hashtable_iterator_advance(itr_remove));

  free(itr_remove);

  hashtable_destroy(remove);

  return num_removed;
}
