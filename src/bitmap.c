/**
 * File:   bitmap.c
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

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "bitmap.h"
#include "stb_image.h"
#include "stb_image_write.h"

bitmap_t* bitmap_create(uint32_t w, uint32_t h) {
  uint32_t size = 0;
  bitmap_t* b = (bitmap_t*)calloc(1, sizeof(bitmap_t));
  return_value_if_fail(b != NULL, NULL);

  b->w = w;
  b->h = h;
  size = w * h * sizeof(rgba_t);

  b->data = (rgba_t*)malloc(size);
  memset(b->data, 0xff, size);

  return b;
}

ret_t bitmap_save(bitmap_t* b, const char* filename) {
  return_value_if_fail(b != NULL && b->data != NULL && filename != NULL, RET_BAD_PARAMS);

  stbi_write_png(filename, b->w, b->h, 4, b->data, 0);

  return RET_OK;
}

ret_t bitmap_init_from_data(bitmap_t* b, uint32_t w, uint32_t h, const uint8_t* data,
                            uint32_t comp) {
  uint32_t i = 0;

  if (comp == 4) {
    uint32_t size = w * h * 4;
    memcpy((uint8_t*)(b->data), data, size);
  } else {
    uint32_t n = w * h;
    const uint8_t* s = data;
    uint8_t* d = (uint8_t*)(b->data);

    for (i = 0; i < n; i++) {
      *d++ = *s++;
      *d++ = *s++;
      *d++ = *s++;
      *d++ = 0xff;
    }
  }

  return RET_OK;
}

bitmap_t* bitmap_load(const char* filename) {
  int w = 0;
  int h = 0;
  int n = 0;
  bitmap_t* b = NULL;
  uint8_t* data = NULL;
  return_value_if_fail(filename != NULL, NULL);
  data = stbi_load(filename, &w, &h, &n, 4);
  return_value_if_fail(data != NULL, NULL);

  b = bitmap_create(w, h);
  if (b != NULL) {
    bitmap_init_from_data(b, w, h, data, n);
  }
  stbi_image_free((uint8_t*)(data));

  return b;
}

ret_t bitmap_get(bitmap_t* b, uint32_t x, uint32_t y, rgba_t* rgba) {
  return_value_if_fail(b != NULL && b->data != NULL && rgba != NULL, RET_BAD_PARAMS);
  return_value_if_fail(x < b->w && y < b->h, RET_BAD_PARAMS);

  *rgba = b->data[b->w * y + x];

  return RET_OK;
}

ret_t bitmap_set(bitmap_t* b, uint32_t x, uint32_t y, const rgba_t* rgba) {
  return_value_if_fail(b != NULL && b->data != NULL && rgba != NULL, RET_BAD_PARAMS);
  return_value_if_fail(x < b->w && y < b->h, RET_BAD_PARAMS);

  b->data[b->w * y + x] = *rgba;

  return RET_OK;
}

ret_t bitmap_destroy(bitmap_t* b) {
  return_value_if_fail(b != NULL, RET_BAD_PARAMS);

  free(b->data);
  free(b);

  return RET_OK;
}
