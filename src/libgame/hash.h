// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    https://www.artsoft.org/
// ----------------------------------------------------------------------------
// hash.h
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

#ifndef HASH_H
#define HASH_H


/* Example of use:
 *
 *      struct hashtable  *h;
 *      struct some_key   *k;
 *      struct some_value *v;
 *
 *      static unsigned int         hash_from_key_fn( void *k );
 *      static int                  keys_equal_fn ( void *key1, void *key2 );
 *
 *      h = create_hashtable(16, 0.75, hash_from_key_fn, keys_equal_fn, free, free);
 *      k = (struct some_key *)     malloc(sizeof(struct some_key));
 *      v = (struct some_value *)   malloc(sizeof(struct some_value));
 *
 *      (initialise k and v to suitable values)
 * 
 *      if (! hashtable_insert(h,k,v) )
 *      {     exit(-1);               }
 *
 *      if (NULL == (found = hashtable_search(h,k) ))
 *      {    printf("not found!");                  }
 *
 *      if (NULL == (found = hashtable_remove(h,k) ))
 *      {    printf("Not found\n");                 }
 *
 */

/* Macros may be used to define type-safe(r) hashtable access functions, with
 * methods specialized to take known key and value types as parameters.
 * 
 * Example:
 *
 * Insert this at the start of your file:
 *
 * DEFINE_HASHTABLE_INSERT(insert_some, struct some_key, struct some_value);
 * DEFINE_HASHTABLE_SEARCH(search_some, struct some_key, struct some_value);
 * DEFINE_HASHTABLE_REMOVE(remove_some, struct some_key, struct some_value);
 *
 * This defines the functions 'insert_some', 'search_some' and 'remove_some'.
 * These operate just like hashtable_insert etc., with the same parameters,
 * but their function signatures have 'struct some_key *' rather than
 * 'void *', and hence can generate compile time errors if your program is
 * supplying incorrect data as a key (and similarly for value).
 *
 * Note that the hash and key equality functions passed to create_hashtable
 * still take 'void *' parameters instead of 'some key *'. This shouldn't be
 * a difficult issue as they're only defined and passed once, and the other
 * functions will ensure that only valid keys are supplied to them.
 *
 * The cost for this checking is increased code size and runtime overhead
 * - if performance is important, it may be worth switching back to the
 * unsafe methods once your program has been debugged with the safe methods.
 * This just requires switching to some simple alternative defines - eg:
 * #define insert_some hashtable_insert
 *
 */


/*****************************************************************************/
struct entry
{
  void *k, *v;
  unsigned int h;
  struct entry *next;
};

struct hashtable
{
  unsigned int tablelength;
  struct entry **table;
  unsigned int entrycount;
  unsigned int loadlimit;
  unsigned int (*hashfn) (void *k);
  int (*eqfn) (void *k1, void *k2);
  void (*freekfn) (void *k);
  void (*freevfn) (void *v);
};

/*****************************************************************************/
struct hashtable_itr
{
  struct hashtable *h;
  struct entry *e;
  unsigned int index;
};

typedef struct hashtable HashTable;


/*****************************************************************************
 * create_hashtable_ext
   
 * @name                    create_hashtable
 * @param   minsize         minimum initial size of hashtable
 * @param   maxloadfactor   maximum ratio entries / tablesize
 * @param   hashfunction    function for hashing keys
 * @param   key_eq_fn       function for determining key equality
 * @param   key_free_fn     function for freeing keys
 * @param   value_free_fn   function for freeing values
 * @return                  newly created hashtable or NULL on failure
 */

struct hashtable *
create_hashtable_ext(unsigned int minsize, float maxloadfactor,
                     unsigned int (*hashfunction) (void*),
                     int (*key_eq_fn) (void*, void*),
                     void (*key_free_fn) (void*),
                     void (*value_free_fn) (void*));

/* wrapper function using reasonable default values for some parameters */
struct hashtable *
create_hashtable(unsigned int (*hashfunction) (void*),
                 int (*key_eq_fn) (void*, void*),
                 void (*key_free_fn) (void*),
                 void (*value_free_fn) (void*));

/*****************************************************************************
 * hashtable_insert
   
 * @name        hashtable_insert
 * @param   h   the hashtable to insert into
 * @param   k   the key   - will be freed on removal if free function defined
 * @param   v   the value - will be freed on removal if free function defined
 * @return      non-zero for successful insertion
 *
 * This function will cause the table to expand if the insertion would take
 * the ratio of entries to table size over the maximum load factor.
 *
 * This function does not check for repeated insertions with a duplicate key.
 * The value returned when using a duplicate key is undefined -- when
 * the hashtable changes size, the order of retrieval of duplicate key
 * entries is reversed.
 * If in doubt, remove before insert.
 */

int 
hashtable_insert(struct hashtable *h, void *k, void *v);

#define DEFINE_HASHTABLE_INSERT(fnname, keytype, valuetype) \
static int fnname (struct hashtable *h, keytype *k, valuetype *v) \
{ \
  return hashtable_insert(h, k, v); \
}

/*****************************************************************************
 * hashtable_change
   
 * @name        hashtable_change
 * @param   h   the hashtable to search
 * @param   k   the key of the entry to change
 * @param   v   the new value
 * @return      non-zero for successful change
 */

int 
hashtable_change(struct hashtable *h, void *k, void *v);

#define DEFINE_HASHTABLE_CHANGE(fnname, keytype, valuetype) \
static int fnname (struct hashtable *h, keytype *k, valuetype *v) \
{ \
  return hashtable_change(h, k, v); \
}

/*****************************************************************************
 * hashtable_exists
   
 * @name        hashtable_exists
 * @param   h   the hashtable to search
 * @param   k   the key to search for
 * @return      non-zero if key exists, else zero
 */

int
hashtable_exists(struct hashtable *h, void *k);

#define DEFINE_HASHTABLE_EXISTS(fnname, keytype, valuetype) \
static int fnname (struct hashtable *h, keytype *k) \
{ \
  return hashtable_exists(h, k); \
}

/*****************************************************************************
 * hashtable_search
   
 * @name        hashtable_search
 * @param   h   the hashtable to search
 * @param   k   the key to search for
 * @return      the value associated with the key, or NULL if none found
 */

void *
hashtable_search(struct hashtable *h, void *k);

#define DEFINE_HASHTABLE_SEARCH(fnname, keytype, valuetype) \
static valuetype * fnname (struct hashtable *h, keytype *k) \
{ \
  return (valuetype *) (hashtable_search(h, k)); \
}

/*****************************************************************************
 * hashtable_remove
   
 * @name        hashtable_remove
 * @param   h   the hashtable to remove the item from
 * @param   k   the key to search for
 * @return      the value associated with the key, or NULL if none found
 */

void * /* returns value */
hashtable_remove(struct hashtable *h, void *k);

#define DEFINE_HASHTABLE_REMOVE(fnname, keytype, valuetype) \
static valuetype * fnname (struct hashtable *h, keytype *k) \
{ \
  return (valuetype *) (hashtable_remove(h, k)); \
}


/*****************************************************************************
 * hashtable_count
   
 * @name        hashtable_count
 * @return      the number of items stored in the hashtable
 */
unsigned int
hashtable_count(struct hashtable *h);


/*****************************************************************************
 * hashtable_destroy
   
 * @name        hashtable_destroy
 */

void
hashtable_destroy(struct hashtable *h);


/*****************************************************************************/
/* hashtable_iterator
 */

struct hashtable_itr *
hashtable_iterator(struct hashtable *h);

/*****************************************************************************/
/* key - return the key of the (key, value) pair at the current position */

void *
hashtable_iterator_key(struct hashtable_itr *i);

/*****************************************************************************/
/* value - return the value of the (key, value) pair at the current position */

void *
hashtable_iterator_value(struct hashtable_itr *i);

/*****************************************************************************/
/* advance - advance the iterator to the next element
 *           returns zero if advanced to end of table */

int
hashtable_iterator_advance(struct hashtable_itr *itr);


/*****************************************************************************/
/* hashtable_fn - prototype of function to call for hashtable entry

 * @name        hashtable_fn
 * @param   k   the key of the current hash entry
 * @param   v   the value of the current hash entry
 * @param   u   additional user data
 */

typedef void (*hashtable_fn) (void *k, void *v, void *u);

/*****************************************************************************/
/* hashtable_foreach - call function for all hashtable entries

 * @name        hashtable_foreach
 * @param   h   the hashtable to iterate through
 * @param   fn  the function to call for each entry
 */

void
hashtable_foreach(struct hashtable *h, hashtable_fn fn, void *userdata);

/*****************************************************************************/
/* hashtable_remove_fn - prototype of function to call for hashtable entry

 * @name        hashtable_remove_fn
 * @param   k   the key of the current hash entry
 * @param   v   the value of the current hash entry
 * @param   u   additional user data
 * @return      non-zero if entry should be removed, else zero
 */

typedef int (*hashtable_remove_fn) (void *k, void *v, void *u);

/*****************************************************************************/
/* hashtable_foreach_remove - call function for all hashtable entries and remove them,
 *                            if function returned 1
 *                            returns the number of removed entries

 * @name        hashtable_foreach_remove
 * @param   h   the hashtable to iterate through
 * @param   fn  the function to call for each entry
 * @return      the number of removed entries
 */

unsigned int
hashtable_foreach_remove(struct hashtable *h, hashtable_remove_fn fn, void *userdata);

/*****************************************************************************/
/* hashtable_remove_all - remove_all hashtable entries

 * @name        hashtable_remove
 * @param   h   the hashtable to remove all entries from
 * @return      the number of removed entries
 */

unsigned int
hashtable_remove_all(struct hashtable *h);

#endif
