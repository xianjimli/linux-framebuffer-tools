/**
 * File:   gsnap.c
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
    printf("Example: %s fb.png /dev/fb0\n", argv[0]);
    printf("-----------------------------------------\n");

    return 0;
  }

  filename = argv[1];
  fbfilename = argv[2];

  fb = linux_fb_open(fbfilename);
  if (fb != NULL) {
    bitmap_t* b = bitmap_create(fb->w, fb->h);
    if (b != NULL) {
      rgba_t bg = {0, 0xff, 0, 0xff};
      rgba_t fg = {0xff, 0, 0, 0xff};

      linux_fb_fill_rect(fb, 0, 0, fb->w, fb->h, &bg);
      linux_fb_stroke_rect(fb, 0, 0, fb->w, fb->h, &fg);
      linux_fb_stroke_rect(fb, 1, 1, fb->w - 2, fb->h - 2, &fg);

      linux_fb_to_bitmap(fb, b);
      bitmap_save(b, filename);
      bitmap_destroy(b);
    }
    linux_fb_close(fb);
  }

  return 0;
}
