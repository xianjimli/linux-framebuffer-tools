/**
 * File:   bitmap.h
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  bitmap
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-11-03 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "types_def.h"

#ifndef BITMAP_H
#define BITMAP_H

BEGIN_C_DECLS

typedef struct _bitmap_t {
  uint32_t w;
  uint32_t h;
  rgba_t* data;
} bitmap_t;

bitmap_t* bitmap_create(uint32_t w, uint32_t h);
bitmap_t* bitmap_load(const char* filename);

ret_t bitmap_save(bitmap_t* b, const char* filename);

ret_t bitmap_get(bitmap_t* b, uint32_t x, uint32_t y, rgba_t* rgba);
ret_t bitmap_set(bitmap_t* b, uint32_t x, uint32_t y, const rgba_t* rgba);

ret_t bitmap_destroy(bitmap_t* b);

END_C_DECLS

#endif /*BITMAP_H*/
