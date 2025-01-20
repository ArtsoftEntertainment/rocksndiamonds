// ============================================================================
// list.c
// ============================================================================

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

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#include <stdlib.h>
#include <stddef.h>

#include "list.h"


/**
 * List:
 * @data: holds the element's data, which can be a pointer to any kind
 *        of data, or any integer value using the
 *        [Type Conversion Macros][glib-Type-Conversion-Macros]
 * @next: contains the link to the next element in the list
 * @prev: contains the link to the previous element in the list
 *
 * The #List struct is used for each element in a doubly-linked list.
 **/

/**
 * list_previous:
 * @list: an element in a #List
 *
 * A convenience macro to get the previous element in a #List.
 * Note that it is considered perfectly acceptable to access
 * @list->prev directly.
 *
 * Returns: the previous element, or %NULL if there are no previous
 *          elements
 **/

/**
 * list_next:
 * @list: an element in a #List
 *
 * A convenience macro to get the next element in a #List.
 * Note that it is considered perfectly acceptable to access
 * @list->next directly.
 *
 * Returns: the next element, or %NULL if there are no more elements
 **/

/**
 * list_alloc:
 *
 * Allocates space for one #List element. It is called by
 * list_append(), list_prepend(), list_insert() and
 * list_insert_sorted() and so is rarely used on its own.
 *
 * Returns: a pointer to the newly-allocated #List element
 **/
List *
list_alloc (void)
{
  return checked_calloc(sizeof(List));
}

/**
 * list_free:
 * @list: the first link of a #List
 *
 * Frees all of the memory used by a #List.
 * The freed elements are returned to the slice allocator.
 *
 * If list elements contain dynamically-allocated memory, you should
 * either use list_free_full() or free them manually first.
 *
 * It can be combined with steal_pointer() to ensure the list head pointer
 * is not left dangling:
 * |[<!-- language="C" -->
 * List *list_of_borrowed_things = …;  /<!-- -->* (transfer container) *<!-- -->/
 * list_free (steal_pointer (&list_of_borrowed_things));
 * ]|
 */
void
list_free (List *list)
{
  void *slice = list;
  size_t next_offset = offsetof(List, next);

  while (slice)
  {
    void *current = slice;
    slice = *(void**) (current + next_offset);
    checked_free(current);
  }
}

/**
 * list_free_1:
 * @list: a #List element
 *
 * Frees one #List element, but does not update links from the next and
 * previous elements in the list, so you should not call this function on an
 * element that is currently part of a list.
 *
 * It is usually used after list_remove_link().
 */
/**
 * list_free1:
 *
 * Another name for list_free_1().
 **/
void
list_free_1 (List *list)
{
  checked_free(list);
}

/**
 * list_append:
 * @list: a pointer to a #List
 * @data: the data for the new element
 *
 * Adds a new element on to the end of the list.
 *
 * Note that the return value is the new start of the list,
 * if @list was empty; make sure you store the new value.
 *
 * list_append() has to traverse the entire list to find the end,
 * which is inefficient when adding multiple elements. A common idiom
 * to avoid the inefficiency is to use list_prepend() and reverse
 * the list with list_reverse() when all elements have been added.
 *
 * |[<!-- language="C" -->
 * // Notice that these are initialized to the empty list.
 * List *strinlist = NULL, *number_list = NULL;
 *
 * // This is a list of strings.
 * strinlist = list_append (strinlist, "first");
 * strinlist = list_append (strinlist, "second");
 *
 * // This is a list of integers.
 * number_list = list_append (number_list, INT_TO_PTR (27));
 * number_list = list_append (number_list, INT_TO_PTR (14));
 * ]|
 *
 * Returns: either @list or the new start of the #List if @list was %NULL
 */
List *
list_append (List *list, void *data)
{
  List *new_list;
  List *last;

  new_list = checked_malloc(sizeof(List));
  new_list->data = data;
  new_list->next = NULL;

  if (list)
  {
    last = list_last(list);
    last->next = new_list;
    new_list->prev = last;

    return list;
  }
  else
  {
    new_list->prev = NULL;

    return new_list;
  }
}

/**
 * list_prepend:
 * @list: a pointer to a #List, this must point to the top of the list
 * @data: the data for the new element
 *
 * Prepends a new element on to the start of the list.
 *
 * Note that the return value is the new start of the list,
 * which will have changed, so make sure you store the new value.
 *
 * |[<!-- language="C" -->
 * // Notice that it is initialized to the empty list.
 * List *list = NULL;
 *
 * list = list_prepend (list, "last");
 * list = list_prepend (list, "first");
 * ]|
 *
 * Do not use this function to prepend a new element to a different
 * element than the start of the list. Use list_insert_before() instead.
 *
 * Returns: a pointer to the newly prepended element, which is the new
 *     start of the #List
 */
List *
list_prepend (List *list, void *data)
{
  List *new_list;

  new_list = checked_malloc(sizeof(List));
  new_list->data = data;
  new_list->next = list;

  if (list)
  {
    new_list->prev = list->prev;
    if (list->prev)
      list->prev->next = new_list;
    list->prev = new_list;
  }
  else
  {
    new_list->prev = NULL;
  }

  return new_list;
}

/**
 * list_insert:
 * @list: a pointer to a #List, this must point to the top of the list
 * @data: the data for the new element
 * @position: the position to insert the element. If this is
 *     negative, or is larger than the number of elements in the
 *     list, the new element is added on to the end of the list.
 *
 * Inserts a new element into the list at the given position.
 *
 * Returns: the (possibly changed) start of the #List
 */
List *
list_insert (List *list, void *data, int position)
{
  List *new_list;
  List *tmp_list;

  if (position < 0)
    return list_append(list, data);
  else if (position == 0)
    return list_prepend(list, data);

  tmp_list = list_nth(list, position);
  if (!tmp_list)
    return list_append(list, data);

  new_list = checked_malloc(sizeof(List));
  new_list->data = data;
  new_list->prev = tmp_list->prev;
  tmp_list->prev->next = new_list;
  new_list->next = tmp_list;
  tmp_list->prev = new_list;

  return list;
}

static inline List *
_list_remove_link (List *list, List *link)
{
  if (link == NULL)
    return list;

  if (link->prev)
  {
    if (link->prev->next == link)
      link->prev->next = link->next;
    else
      Warn("corrupted double-linked list detected");
  }

  if (link->next)
  {
    if (link->next->prev == link)
      link->next->prev = link->prev;
    else
      Warn("corrupted double-linked list detected");
  }

  if (link == list)
    list = list->next;

  link->next = NULL;
  link->prev = NULL;

  return list;
}

/**
 * list_remove:
 * @list: a #List, this must point to the top of the list
 * @data: the data of the element to remove
 *
 * Removes an element from a #List.
 * If two elements contain the same data, only the first is removed.
 * If none of the elements contain the data, the #List is unchanged.
 *
 * Returns: the (possibly changed) start of the #List
 */
List *
list_remove (List *list, const void *data)
{
  List *tmp;

  tmp = list;
  while (tmp)
  {
    if (tmp->data != data)
    {
      tmp = tmp->next;
    }
    else
    {
      list = _list_remove_link (list, tmp);
      free (tmp);

      break;
    }
  }

  return list;
}

/**
 * list_remove_all:
 * @list: a #List, this must point to the top of the list
 * @data: data to remove
 *
 * Removes all list nodes with data equal to @data.
 * Returns the new head of the list. Contrast with
 * list_remove() which removes only the first node
 * matching the given data.
 *
 * Returns: the (possibly changed) start of the #List
 */
List *
list_remove_all (List *list, const void *data)
{
  List *tmp = list;

  while (tmp)
  {
    if (tmp->data != data)
    {
      tmp = tmp->next;
    }
    else
    {
      List *next = tmp->next;

      if (tmp->prev)
	tmp->prev->next = next;
      else
	list = next;

      if (next)
	next->prev = tmp->prev;

      free (tmp);
      tmp = next;
    }
  }

  return list;
}

/**
 * list_remove_link:
 * @list: a #List, this must point to the top of the list
 * @llink: an element in the #List
 *
 * Removes an element from a #List, without freeing the element.
 * The removed element's prev and next links are set to %NULL, so
 * that it becomes a self-contained list with one element.
 *
 * This function is for example used to move an element in the list
 * (see the example for list_concat()) or to remove an element in
 * the list before freeing its data:
 * |[<!-- language="C" -->
 * list = list_remove_link (list, llink);
 * free_some_data_that_may_access_the_list_again (llink->data);
 * list_free (llink);
 * ]|
 *
 * Returns: the (possibly changed) start of the #List
 */
List *
list_remove_link (List *list, List *llink)
{
  return _list_remove_link(list, llink);
}

/**
 * list_delete_link:
 * @list: a #List, this must point to the top of the list
 * @link_: node to delete from @list
 *
 * Removes the node link_ from the list and frees it.
 * Compare this to list_remove_link() which removes the node
 * without freeing it.
 *
 * Returns: the (possibly changed) start of the #List
 */
List *
list_delete_link (List *list, List *link_)
{
  list = _list_remove_link(list, link_);
  checked_free(link_);

  return list;
}

/**
 * list_copy:
 * @list: a #List, this must point to the top of the list
 *
 * Copies a #List.
 *
 * Note that this is a "shallow" copy. If the list elements
 * consist of pointers to data, the pointers are copied but
 * the actual data is not. See list_copy_deep() if you need
 * to copy the data as well.
 *
 * Returns: the start of the new list that holds the same data as @list
 */
List *
list_copy (List *list)
{
  return list_copy_deep(list, NULL, NULL);
}

/**
 * g_list_copy_deep:
 * @list: a #List, this must point to the top of the list
 * @func: (scope call): a copy function used to copy every element in the list
 * @user_data: user data passed to the copy function @func, or %NULL
 *
 * Makes a full (deep) copy of a #List.
 *
 * In contrast with g_list_copy(), this function uses @func to make
 * a copy of each list element, in addition to copying the list
 * container itself.
 *
 * @func, as a #GCopyFunc, takes two arguments, the data to be copied
 * and a @user_data pointer. On common processor architectures, it's safe to
 * pass %NULL as @user_data if the copy function takes only one argument. You
 * may get compiler warnings from this though if compiling with GCC’s
 * `-Wcast-function-type` warning.
 *
 * For instance, if @list holds a list of GObjects, you can do:
 * |[<!-- language="C" -->
 * another_list = g_list_copy_deep (list, (GCopyFunc) g_object_ref, NULL);
 * ]|
 *
 * And, to entirely free the new list, you could do:
 * |[<!-- language="C" -->
 * g_list_free_full (another_list, g_object_unref);
 * ]|
 *
 * Returns: the start of the new list that holds a full copy of @list,
 *     use g_list_free_full() to free it
 *
 * Since: 2.34
 */
List *
list_copy_deep (List *list, list_copy_fn func, void *user_data)
{
  List *new_list = NULL;

  if (list)
  {
    List *last;

    new_list = checked_malloc(sizeof(List));

    if (func)
      new_list->data = func (list->data, user_data);
    else
      new_list->data = list->data;

    new_list->prev = NULL;
    last = new_list;
    list = list->next;

    while (list)
    {
      last->next = checked_malloc(sizeof(List));
      last->next->prev = last;
      last = last->next;

      if (func)
	last->data = func (list->data, user_data);
      else
	last->data = list->data;

      list = list->next;
    }

    last->next = NULL;
  }

  return new_list;
}

/**
 * list_reverse:
 * @list: a #List, this must point to the top of the list
 *
 * Reverses a #List.
 * It simply switches the next and prev pointers of each element.
 *
 * Returns: the start of the reversed #List
 */
List *
list_reverse (List *list)
{
  List *last;

  last = NULL;

  while (list)
  {
    last = list;
    list = last->next;
    last->next = last->prev;
    last->prev = list;
  }

  return last;
}

/**
 * list_nth:
 * @list: a #List, this must point to the top of the list
 * @n: the position of the element, counting from 0
 *
 * Gets the element at the given position in a #List.
 *
 * This iterates over the list until it reaches the @n-th position. If you
 * intend to iterate over every element, it is better to use a for-loop as
 * described in the #List introduction.
 *
 * Returns: the element, or %NULL if the position is off
 *     the end of the #List
 */
List *
list_nth (List *list, unsigned int  n)
{
  while ((n-- > 0) && list)
    list = list->next;

  return list;
}

/**
 * list_nth_prev:
 * @list: a #List
 * @n: the position of the element, counting from 0
 *
 * Gets the element @n places before @list.
 *
 * Returns: the element, or %NULL if the position is
 *     off the end of the #List
 */
List *
list_nth_prev (List *list, unsigned int n)
{
  while ((n-- > 0) && list)
    list = list->prev;

  return list;
}

/**
 * list_nth_data:
 * @list: a #List, this must point to the top of the list
 * @n: the position of the element
 *
 * Gets the data of the element at the given position.
 *
 * This iterates over the list until it reaches the @n-th position. If you
 * intend to iterate over every element, it is better to use a for-loop as
 * described in the #List introduction.
 *
 * Returns: the element's data, or %NULL if the position
 *     is off the end of the #List
 */
void *
list_nth_data (List *list, unsigned int n)
{
  while ((n-- > 0) && list)
    list = list->next;

  return list ? list->data : NULL;
}

/**
 * list_position:
 * @list: a #List, this must point to the top of the list
 * @llink: an element in the #List
 *
 * Gets the position of the given element
 * in the #List (starting from 0).
 *
 * Returns: the position of the element in the #List,
 *     or -1 if the element is not found
 */
int
list_position (List *list, List *llink)
{
  int i;

  i = 0;
  while (list)
  {
    if (list == llink)
      return i;

    i++;
    list = list->next;
  }

  return -1;
}

/**
 * list_index:
 * @list: a #List, this must point to the top of the list
 * @data: the data to find
 *
 * Gets the position of the element containing
 * the given data (starting from 0).
 *
 * Returns: the index of the element containing the data,
 *     or -1 if the data is not found
 */
int
list_index (List *list, const void *data)
{
  int i;

  i = 0;
  while (list)
  {
    if (list->data == data)
      return i;

    i++;
    list = list->next;
  }

  return -1;
}

/**
 * list_last:
 * @list: any #List element
 *
 * Gets the last element in a #List.
 *
 * Returns: the last element in the #List,
 *     or %NULL if the #List has no elements
 */
List *
list_last (List *list)
{
  if (list)
  {
    while (list->next)
      list = list->next;
  }

  return list;
}

/**
 * list_first:
 * @list: any #List element
 *
 * Gets the first element in a #List.
 *
 * Returns: the first element in the #List,
 *     or %NULL if the #List has no elements
 */
List *
list_first (List *list)
{
  if (list)
  {
    while (list->prev)
      list = list->prev;
  }

  return list;
}

/**
 * list_length:
 * @list: a #List, this must point to the top of the list
 *
 * Gets the number of elements in a #List.
 *
 * This function iterates over the whole list to count its elements.
 * Use a #GQueue instead of a List if you regularly need the number
 * of items. To check whether the list is non-empty, it is faster to check
 * @list against %NULL.
 *
 * Returns: the number of elements in the #List
 */
unsigned int
list_length (List *list)
{
  unsigned int length;

  length = 0;
  while (list)
  {
    length++;
    list = list->next;
  }

  return length;
}

/**
 * list_foreach_fn_1:
 * @list: a #List, this must point to the top of the list
 * @func: (scope call): the function to call with each element's data
 *
 * Calls a function for each element of a #List.
 *
 * It is safe for @func to remove the element from @list, but it must
 * not modify any part of the list after that element.
 */
/**
 * list_fn_1:
 * @data: the element's data
 *
 * Specifies the type of functions passed to list_foreach_fn_1().
 */
void
list_foreach_fn_1 (List *list, list_fn_1 func)
{
  while (list)
  {
    List *next = list->next;

    (*func) (list->data);
    list = next;
  }
}

/**
 * list_foreach_fn_2:
 * @list: a #List, this must point to the top of the list
 * @func: (scope call): the function to call with each element's data
 * @user_data: user data to pass to the function
 *
 * Calls a function for each element of a #List.
 *
 * It is safe for @func to remove the element from @list, but it must
 * not modify any part of the list after that element.
 */
/**
 * list_fn_2:
 * @data: the element's data
 * @user_data: user data passed to list_foreach_fn_2()
 *
 * Specifies the type of functions passed to list_foreach_fn_2().
 */
void
list_foreach_fn_2 (List *list, list_fn_2 func, void *user_data)
{
  while (list)
  {
    List *next = list->next;

    (*func) (list->data, user_data);
    list = next;
  }
}
