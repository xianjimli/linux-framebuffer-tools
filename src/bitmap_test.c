/**
 * File:   bitmap_test.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  bitmap_test.c
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

#include "bitmap.h"

static bool_t bitmap_copy(bitmap_t* dst, bitmap_t* src) {
  rgba_t rgba1;
  rgba_t rgba2;
  uint32_t x = 0;
  uint32_t y = 0;

  assert(dst->w == src->w);
  assert(src->h == src->h);

  for (y = 0; y < dst->h; y++) {
    for (x = 0; x < dst->w; x++) {
      assert(bitmap_get(src, x, y, &rgba1) == RET_OK);
      assert(bitmap_set(dst, x, y, &rgba1) == RET_OK);
      assert(bitmap_get(dst, x, y, &rgba2) == RET_OK);
      assert(rgba1.r == rgba2.r);
      assert(rgba1.g == rgba2.g);
      assert(rgba1.b == rgba2.b);
      assert(rgba1.a == rgba2.a);
    }
  }

  return TRUE;
}

static bool_t bitmap_cmp(bitmap_t* b1, bitmap_t* b2) {
  rgba_t rgba1;
  rgba_t rgba2;
  uint32_t x = 0;
  uint32_t y = 0;

  assert(b1->w == b2->w);
  assert(b2->h == b2->h);

  for (y = 0; y < b1->h; y++) {
    for (x = 0; x < b1->w; x++) {
      assert(bitmap_get(b1, x, y, &rgba1) == RET_OK);
      assert(bitmap_get(b2, x, y, &rgba2) == RET_OK);
      assert(rgba1.r == rgba2.r);
      assert(rgba1.g == rgba2.g);
      assert(rgba1.b == rgba2.b);
      assert(rgba1.a == rgba2.a);
    }
  }

  return TRUE;
}

static void test_load_save(const char* filename) {
  bitmap_t* b1 = NULL;
  bitmap_t* b2 = bitmap_load("rgba.png");
  assert(b2->w == 30);
  assert(b2->h == 30);
  assert(bitmap_save(b2, "test.png") == RET_OK);

  b1 = bitmap_load("test.png");
  assert(bitmap_cmp(b1, b2));

  bitmap_copy(b1, b2);
  bitmap_destroy(b1);
  bitmap_destroy(b2);
}

int main(int argc, char* argv[]) {
  test_load_save("rgba.png");
  return 0;
}
