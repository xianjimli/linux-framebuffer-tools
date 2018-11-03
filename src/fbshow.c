/**
 * File:   fbshow.c
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  snap framebuffer to png file
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

#include "linux_fb.h"

int main(int argc, char* argv[]) {
  fb_info_t* fb = NULL;
  const char* filename = NULL;
  const char* fbfilename = NULL;

  if (argc != 3) {
    printf("\nUsage: %s outputfilename [framebuffer dev]\n", argv[0]);
    printf("Example: %s data/rgba.png /dev/fb0\n", argv[0]);
    printf("-----------------------------------------\n");

    return 0;
  }

  filename = argv[1];
  fbfilename = argv[2];

  fb = linux_fb_open(fbfilename);
  if (fb != NULL) {
    bitmap_t* b = bitmap_load(filename);
    if (b != NULL) {
      linux_fb_from_bitmap(fb, b);
      bitmap_destroy(b);
    }
    linux_fb_close(fb);
  }

  return 0;
}
