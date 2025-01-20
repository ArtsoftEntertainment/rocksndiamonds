// ============================================================================
// list.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
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

#ifndef LIST_H
#define LIST_H

#include "misc.h"


typedef struct _List List;

struct _List
{
  void *data;
  List *next;
  List *prev;
};


typedef void (*list_fn_1) (void *data);
typedef void (*list_fn_2) (void *data, void *userdata);
typedef void *(*list_copy_fn) (const void *data, void *userdata);

/* Doubly linked lists */
List *list_alloc(void);
void  list_free(List *list);
void  list_free_1(List *list);

List *list_append(List *list, void *data);
List *list_prepend(List *list, void *data);
List *list_insert(List *list, void *data, int position);
List *list_remove(List *list, const void *data);
List *list_remove_all(List *list, const void *data);
List *list_remove_link(List *list, List *llink);
List *list_delete_link(List *list, List *link_);
List *list_reverse(List *list);
List *list_copy(List *list);
List *list_copy_deep(List *list, list_copy_fn func, void *user_data);
List *list_nth(List *list, unsigned int n);
List *list_nth_prev(List *list, unsigned int n);
int   list_position(List *list, List *llink);
int   list_index(List *list, const void *data);
List *list_last(List *list);
List *list_first(List *list);
unsigned int list_length(List *list);
void  list_foreach_fn_1(List *list, list_fn_1 func);
void  list_foreach_fn_2(List *list, list_fn_2 func, void *user_data);
void *list_nth_data(List *list, unsigned int n);

#define list_previous(list)	((list) ? (((List *)(list))->prev) : NULL)
#define list_next(list)		((list) ? (((List *)(list))->next) : NULL)

#endif
