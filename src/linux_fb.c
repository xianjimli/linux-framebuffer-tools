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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "linux_fb.h"

#define fb_width(fb) ((fb)->w)
#define fb_height(fb) ((fb)->h)
#define fb_size(fb) ((fb)->h * (fb)->line_length)

fb_info_t* linux_fb_open(const char* filename) {
  uint32_t size = 0;
  fb_info_t* fb = NULL;
  struct fb_fix_screeninfo fix;
  struct fb_var_screeninfo var;
  return_value_if_fail(filename != NULL, NULL);

  fb = (fb_info_t*)calloc(1, sizeof(fb_info_t));
  return_value_if_fail(fb != NULL, NULL);

  fb->fd = open(filename, O_RDWR);
  if (fb->fd < 0) {
    log_debug("open %s failed(%d)\n", filename, errno);
    free(fb);
    return NULL;
  }

  if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fix) < 0) goto fail;
  if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &var) < 0) goto fail;

  var.xoffset = 0;
  var.yoffset = 0;
  ioctl(fb->fd, FBIOPAN_DISPLAY, &(var));

  log_debug("fb_info_t: %s\n", filename);
  log_debug("fb_info_t: xres=%d yres=%d bits_per_pixel=%d mem_size=%d\n", var.xres, var.yres,
            var.bits_per_pixel, fb_size(fb));
  log_debug("fb_info_t: red(%d %d) green(%d %d) blue(%d %d)\n", var.red.offset, var.red.length,
            var.green.offset, var.green.length, var.blue.offset, var.blue.length);

  fb->w = var.xres;
  fb->h = var.yres;
  fb->bpp = var.bits_per_pixel / 8;
  fb->line_length = fix.line_length;

  size = fb_size(fb);
  fb->data = (uint8_t*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);

  if (fb->data == MAP_FAILED) {
    log_debug("map framebuffer failed.\n");
    goto fail;
  }

  log_debug("line_length=%d mem_size=%d\n", fix.line_length, fb_size(fb));
  log_debug("xres_virtual =%d yres_virtual=%d xpanstep=%d ywrapstep=%d\n", var.xres_virtual,
            var.yres_virtual, fix.xpanstep, fix.ywrapstep);

  return fb;
fail:
  log_debug("%s is not a framebuffer.\n", filename);
  close(fb->fd);
  free(fb);

  return NULL;
}

static uint8_t* linux_fb_get_addr(fb_info_t* fb, uint32_t x, uint32_t y) {
  return_value_if_fail(fb != NULL && fb->data != NULL && x < fb->w && y < fb->h, NULL);

  return fb->data + y * fb->line_length + x * fb->bpp;
}

static ret_t linux_fb_get(fb_info_t* fb, uint32_t x, uint32_t y, rgba_t* rgba) {
  uint8_t* addr = linux_fb_get_addr(fb, x, y);
  return_value_if_fail(addr != NULL && rgba != NULL, RET_BAD_PARAMS);

  if (fb->bpp == 2) {
    pixel_rgb565_t* p = (pixel_rgb565_t*)addr;
    rgba_t pixel = pixel_rgb565_to_rgba((*p));
    *rgba = pixel;
  } else if (fb->bpp == 4) {
    pixel_bgra8888_t* p = (pixel_bgra8888_t*)addr;
    rgba_t pixel = pixel_bgra8888_to_rgba((*p));
    *rgba = pixel;
  }

  return RET_OK;
}

static ret_t linux_fb_set(fb_info_t* fb, uint32_t x, uint32_t y, const rgba_t* rgba) {
  uint8_t* addr = linux_fb_get_addr(fb, x, y);
  return_value_if_fail(addr != NULL && rgba != NULL, RET_BAD_PARAMS);

  if (fb->bpp == 2) {
    pixel_rgb565_t* p = (pixel_rgb565_t*)addr;
    pixel_rgb565_t pixel = pixel_rgb565_from_rgb(rgba->r, rgba->g, rgba->b);
    *p = pixel;
  } else if (fb->bpp == 4) {
    pixel_bgra8888_t* p = (pixel_bgra8888_t*)addr;
    pixel_bgra8888_t pixel = pixel_bgra8888_from_rgb(rgba->r, rgba->g, rgba->b);
    *p = pixel;
  }

  return RET_OK;
}

ret_t linux_fb_to_bitmap(fb_info_t* fb, bitmap_t* b) {
  uint32_t x = 0;
  uint32_t y = 0;
  return_value_if_fail(fb != NULL && b != NULL, RET_BAD_PARAMS);
  return_value_if_fail(fb->w == b->w && fb->h == b->h, RET_BAD_PARAMS);

  for (y = 0; y < fb->h; y++) {
    for (x = 0; x < fb->w; x++) {
      rgba_t rgba;

      linux_fb_get(fb, x, y, &rgba);
      bitmap_set(b, x, y, &rgba);
    }
  }

  return RET_OK;
}

ret_t linux_fb_from_bitmap(fb_info_t* fb, bitmap_t* b) {
  uint32_t x = 0;
  uint32_t y = 0;
  return_value_if_fail(fb != NULL && b != NULL, RET_BAD_PARAMS);

  for (y = 0; y < fb->h && y < b->h; y++) {
    for (x = 0; x < fb->w && x < b->w; x++) {
      rgba_t rgba;

      bitmap_get(b, x, y, &rgba);
      linux_fb_set(fb, x, y, &rgba);
    }
  }

  return RET_OK;
}

ret_t linux_fb_vline(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t h, const rgba_t* rgba) {
  uint32_t i = 0;
  return_value_if_fail(fb != NULL && x < fb->w && (y + h - 1) < fb->h, RET_BAD_PARAMS);

  for (i = 0; i < h; i++) {
    linux_fb_set(fb, x, y + i, rgba);
  }

  return RET_OK;
}

ret_t linux_fb_hline(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t w, const rgba_t* rgba) {
  uint32_t i = 0;
  return_value_if_fail(fb != NULL && (x + w - 1) < fb->w && y < fb->h, RET_BAD_PARAMS);

  for (i = 0; i < w; i++) {
    linux_fb_set(fb, x + i, y, rgba);
  }

  return RET_OK;
}

ret_t linux_fb_fill_rect(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                         const rgba_t* rgba) {
  uint32_t i = 0;
  return_value_if_fail(fb != NULL && rgba != NULL, RET_BAD_PARAMS);
  return_value_if_fail((x + w - 1) < fb->w, RET_BAD_PARAMS);
  return_value_if_fail((y + h - 1) < fb->h, RET_BAD_PARAMS);

  for (i = 0; i < h; i++) {
    linux_fb_hline(fb, x, y + i, w, rgba);
  }

  return RET_OK;
}

ret_t linux_fb_stroke_rect(fb_info_t* fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                           const rgba_t* rgba) {
  return_value_if_fail(fb != NULL && rgba != NULL, RET_BAD_PARAMS);
  return_value_if_fail((x + w - 1) < fb->w, RET_BAD_PARAMS);
  return_value_if_fail((y + h - 1) < fb->h, RET_BAD_PARAMS);

  linux_fb_hline(fb, x, y, w, rgba);
  linux_fb_hline(fb, x, y + h - 1, w, rgba);
  linux_fb_vline(fb, x, y, h, rgba);
  linux_fb_vline(fb, x + w - 1, y, h, rgba);

  return RET_OK;
}

ret_t linux_fb_close(fb_info_t* fb) {
  return_value_if_fail(fb != NULL, RET_BAD_PARAMS);

  munmap(fb->data, fb_size(fb));
  close(fb->fd);
  memset(fb, 0x00, sizeof(fb_info_t));
  free(fb);

  return RET_OK;
}
