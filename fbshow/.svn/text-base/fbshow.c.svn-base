/*
 * File:    fbshow.c
 * Author:  Li XianJing <xianjimli@hotmail.com>
 * Brief:   show an image on framebuffer device.
 *
 * Copyright (c) 2011  Li XianJing <xianjimli@hotmail.com>
 *
 */

/*
 * History:
 * ================================================================
 * 2011-04-11 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <png.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <jpeglib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/kd.h>

typedef struct _Color
{
	unsigned char b;
	unsigned char r;
	unsigned char g;
	unsigned char a;
}Color;

typedef struct _Image
{
	size_t w;
	size_t h;
	Color* bits;
}Image;

struct _FBInfo;
typedef struct _FBInfo FBInfo;
typedef int (*WritePixel)(FBInfo* fb, unsigned char* dst, Color* color);

struct _FBInfo
{
	int fd;
	WritePixel write;
	unsigned char *bits;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
};

#define fb_width(fb)  ((fb)->vi.xres)
#define fb_height(fb)  ((fb)->vi.yres)
#define fb_bpp(fb)     ((fb)->vi.bits_per_pixel>>3)
#define fb_size(fb)    ((fb)->vi.xres * (fb)->vi.yres * fb_bpp(fb))
#define fb_line_length(fb) ((fb)->fi.line_length)
#define min(a, b)      ((a) < (b)) ? (a) : (b)

static int fb_write_rgb565(FBInfo* fb, unsigned char* dst, Color* c)
{
	unsigned short pixel = 0;
	unsigned int color = *(unsigned int*)c;
    pixel = (color & 0xf80000) >> 8 | ((color & 0xfc00) >> 5) | ((color & 0xf8) >> 3);
	*(unsigned short*)dst = pixel;

	return 0;
}

static int fb_write_rgb24(FBInfo* fb, unsigned char* dst, Color* color)
{
	dst[0] = color->r;
	dst[1] = color->g;
	dst[2] = color->b;

	return 0;
}

static int fb_write_argb32(FBInfo* fb, unsigned char* dst, Color* color)
{
	dst[0] = color->a;
	dst[1] = color->r;
	dst[2] = color->g;
	dst[3] = color->b;

	return 0;
}

static int fb_write_none(FBInfo* fb, unsigned char* dst, Color* color)
{
	return 0;
}

static void set_pixel_writer(FBInfo* fb)
{
	if(fb_bpp(fb) == 2)
	{
		fb->write = fb_write_rgb565;
	}
	else if(fb_bpp(fb) == 3)
	{
		fb->write = fb_write_rgb24;
	}
	else if(fb_bpp(fb) == 4)
	{
		fb->write = fb_write_argb32;
	}
	else
	{
		fb->write = fb_write_none;
		printf("%s: not supported format.\n", __func__);
	}

	return;
}

static int fb_open(FBInfo* fb, const char* fbfilename)
{
	fb->fd = open(fbfilename, O_RDWR);

	if (fb->fd < 0)
	{
		fprintf(stderr, "can't open %s\n", fbfilename);

		return -1;
	}

	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0)
		goto fail;

	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0)
		goto fail;

	fb->bits = mmap(0, fb_size(fb), PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);

	if (fb->bits == MAP_FAILED)
		goto fail;

	printf("---------------%s---------------\n  width : %8d\n  height: %8d\n  bpp   : %8d\n  line  : %8d\n  r(%2d, %2d)\n  g(%2d, %2d)\n  b(%2d, %2d)\n",
		fbfilename, fb_width(fb), fb_height(fb), fb_bpp(fb), 
		fb_line_length(fb),
		fb->vi.red.offset, fb->vi.red.length,
		fb->vi.green.offset, fb->vi.green.length,
		fb->vi.blue.offset, fb->vi.blue.length);
	printf("-----------------------------------------------\n");

	set_pixel_writer(fb);

	return 0;

fail:
	printf("%s is not a framebuffer.\n", fbfilename);
	close(fb->fd);

	return -1;
}

static void fb_close(FBInfo* fb)
{
    fb->vi.yoffset = 1;
    ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
    fb->vi.yoffset = 0;
    ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
	munmap(fb->bits, fb_size(fb));

	close(fb->fd);

	return;
}

static Image* image_create(int w, int h)
{
	Image* image = malloc(sizeof(Image));
	image->w = w;
	image->h = h;
	image->bits = malloc(w * h * sizeof(Color));

	return image;
}

static Color* image_bits(Image* image)
{
	return image->bits;
}

static void image_destroy(Image* image)
{
	free(image->bits);
	free(image);

	return;
}

struct my_error_mgr
{
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

static Image* image_load_png(const char* filename)
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	int passes_nr = 0;

	FILE *fp = NULL;

	Color* dst = NULL;
	unsigned char* src = NULL;
	Image* image = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep * row_pointers = NULL;

	if ((fp = fopen (filename, "rb")) == NULL)
	{
		printf("%s: open %s failed.\n", __func__, filename);
		return NULL;
	}

	if((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
	{
		fclose(fp);
		return NULL;
	}
	
	if((info_ptr = png_create_info_struct(png_ptr)) == NULL)
	{
		fclose(fp);
		return NULL;
	}

#ifdef PNG_SETJMP_SUPPORTED
	setjmp(png_jmpbuf(png_ptr));
#endif

	png_init_io(png_ptr, fp);

	memset(info_ptr, 0x00, sizeof(*info_ptr));
	png_read_info(png_ptr, info_ptr);

	w = info_ptr->width;
	h = info_ptr->height;

	passes_nr = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

#ifdef PNG_SETJMP_SUPPORTED
	setjmp(png_jmpbuf(png_ptr));
#endif

	row_pointers = (png_bytep*) calloc(1, h * sizeof(png_bytep));
	for (y=0; y< h; y++)
	{
		row_pointers[y] = (png_byte*) calloc(1, info_ptr->rowbytes);
	}
	png_read_image(png_ptr, row_pointers);

	image = image_create(w, h);
	dst = image_bits(image);
	if (info_ptr->color_type == PNG_COLOR_TYPE_RGBA)
	{
		for(y = 0; y < h; y++)
		{
			src = row_pointers[y];
			for(x = 0; x < w; x++)
			{
				if(src[3])
				{
					dst->r = src[0];
					dst->g = src[1];
					dst->b = src[2];
				}
				dst->a = src[3];
				src +=4;
				dst++;
			}
		}
	}
	else if(info_ptr->color_type == PNG_COLOR_TYPE_RGB)
	{
		if(0 == info_ptr->num_trans)
		{
			for(y = 0; y < h; y++)
			{
				src = row_pointers[y];
				for(x = 0; x < w; x++)
				{
					dst->r = src[0];
					dst->g = src[1];
					dst->b = src[2];
					dst->a = 0xff;
					src += 3;
					dst++;
				}
			}
		}
		else 
		{
#if PNG_LIBPNG_VER > 10399
			png_byte red = png_ptr->trans_color.red & 0xff;
			png_byte green = png_ptr->trans_color.green & 0xff;
			png_byte blue = png_ptr->trans_color.blue & 0xff;
#else
			png_byte red = png_ptr->trans_values.red & 0xff;
			png_byte green = png_ptr->trans_values.green & 0xff;
			png_byte blue = png_ptr->trans_values.blue & 0xff;
#endif
			for(y = 0; y < h; y++)
			{
				src = row_pointers[y];
				for(x = 0; x < w; x++)
				{
					if(src[0] == red && src[1] == green && src[2] == blue)
					{
						dst->a = 0;
					}
					else
					{
						dst->a = 0xff;
					}
					dst->r = src[0];
					dst->g = src[1];
					dst->b = src[2];
					src += 3;
					dst++;
				}
			}
		}
	}
	else
	{
		assert(!"not supported.");
	}

	for(y = 0; y < h; y++)
	{
		free(row_pointers[y]);
	}
	free(row_pointers);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL); 

	fclose(fp);

	return image;
}

typedef struct my_error_mgr *my_error_ptr;

METHODDEF (void)
my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	(*cinfo->err->output_message) (cinfo);

	longjmp (myerr->setjmp_buffer, 1);
}

static void put_scanline_to_image (Image* image, char *scanline, int y, int width)
{
	int i = 0;
	Color* dst = NULL;
	char *pixels = scanline;

	dst = image_bits(image) + y * width;

	while (i < width)
	{
		dst->r = pixels[0];
		dst->g = pixels[1];
		dst->b = pixels[2];

		i++;
		dst++;
		pixels += 3;
	}

	return;
}
static Image* image_load_jpg(const char* filename)
{
	int i = 0;
	FILE *infile;
	JSAMPARRAY buffer;	
	int row_stride;	
	Image* image = NULL;
	struct my_error_mgr jerr;
	struct jpeg_decompress_struct cinfo;
	
	if ((infile = fopen (filename, "rb")) == NULL)
	{
		printf("can't open %s\n", filename);
		return NULL;
	}

	cinfo.err = jpeg_std_error (&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	
	if (setjmp (jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress (&cinfo);
		fclose (infile);
		return NULL;
	}
	jpeg_create_decompress (&cinfo);

	jpeg_stdio_src (&cinfo, infile);

	(void) jpeg_read_header (&cinfo, TRUE);

	(void) jpeg_start_decompress (&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);
	
	if((image = image_create(cinfo.output_width, cinfo.output_height)) == NULL)
	{
		jpeg_destroy_decompress (&cinfo);
		fclose (infile);

		return NULL;
	}

	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void) jpeg_read_scanlines (&cinfo, buffer, 1);

		put_scanline_to_image (image, (char*)buffer[0], i, cinfo.output_width);
		i++;
	}

	(void) jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);

	fclose (infile);

	return image;

	return image;
}

static Image* image_load(const char* filename)
{
	Image* image = NULL;
	
	if(strstr(filename, ".png") != NULL)
	{
		image = image_load_png(filename);
	}
	else
	{
		image = image_load_jpg(filename);
	}

	return image;
}

static void image_show(Image* image, FBInfo* fb)
{
	size_t x = 0;
	size_t y = 0;
	size_t w = min(image->w, fb_width(fb));
	size_t h = min(image->h, fb_height(fb));
	size_t line_size = fb_line_length(fb);
	unsigned char* dst = NULL;
	Color* src = image->bits;

	for(y = 0; y < h; y++)
	{
		dst = fb->bits + y * line_size;
		for(x = 0; x < w; x++, src++, dst+=fb_bpp(fb))
		{
			fb->write(fb, dst, src);	
		}
	}

	return;
}

int main(int argc, char* argv[])
{
	FBInfo fb;
	const char* filename   = NULL;
	const char* fbfilename = NULL;

	if(argc != 3)
	{
		printf("\nUsage: %s [jpeg|png file] [framebuffer dev]\n", argv[0]);
		printf("Example: %s fb.jpg /dev/fb0\n", argv[0]);
		printf("-----------------------------------------\n");
		printf("Author: Li XianJing <xianjimli@hotmail.com>\n\n");

		return 0;
	}

	filename   = argv[1];
	fbfilename = argv[2];

	memset(&fb, 0x00, sizeof(fb));
	Image* image = image_load(filename);
	if (fb_open(&fb, fbfilename) == 0)
	{
		if(image != NULL)
		{
			image_show(image, &fb);
		}
		fb_close(&fb);
	}
	printf("Author: Li XianJing <xianjimli@hotmail.com>\n\n");

	return 0;
}

