/**
 * File:   linux_fb.h
 * Author: Li XianJing <xianjimli@hotmail.com>
 * Brief:  linux_framebuffer
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

#include <stdio.h>
#include "pixel.h"
#include "bitmap.h"

#ifndef LINUX_FB_H
#define LINUX_FB_H

typedef struct _fb_info_t {
  int fd;
  uint8_t* data;

  uint32_t w;
  uint32_t h;
  uint32_t bpp;
  uint32_t line_length;
} fb_info_t;

fb_info_t* linux_fb_open(const char* filename);

ret_t linux_fb_to_bitmap(fb_info_t* fb, bitmap_t* b);
ret_t linux_fb_from_bitmap(fb_info_t* fb, bitmap_t* b);

ret_t linux_fb_vline(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t h, const rgba_t* rgba);
ret_t linux_fb_hline(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t w, const rgba_t* rgba);
ret_t linux_fb_fill_rect(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                         const rgba_t* rgba);
ret_t linux_fb_stroke_rect(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                           const rgba_t* rgba);

ret_t linux_fb_close(fb_info_t* fb);

#endif /*LINUX_FB_H*/
