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

#ifndef BD_BDCFF_H
#define BD_BDCFF_H

typedef struct _gd_ptr_array
{
  void **data;
  unsigned int size;
  unsigned int size_initial;
  unsigned int size_allocated;
} GdPtrArray;

GdPtrArray *gd_ptr_array_sized_new(unsigned int size);
GdPtrArray *gd_ptr_array_new(void);
void gd_ptr_array_add(GdPtrArray *array, void *data);
boolean gd_ptr_array_remove(GdPtrArray *array, void *data);
void gd_ptr_array_free(GdPtrArray *array, boolean free_data);
#define gd_ptr_array_index(array, index) ((array)->data)[index]

boolean gd_caveset_load_from_bdcff(const char *contents);
GdPtrArray *gd_caveset_save_to_bdcff(void);

#endif	// BD_BDCFF_H
