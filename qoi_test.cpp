/* A pointer to a qoi_desc struct has to be supplied to all of qoi's functions.
It describes either the input format (for qoi_write and qoi_encode1), or is
filled with the description read from the file header (for qoi_read and
qoi_decode1).

The colorspace in this qoi_desc is an enum where
	0 = sRGB, i.e. gamma scaled RGB channels and a linear alpha channel
	1 = all channels are linear
You may use the constants QOI_SRGB or QOI_LINEAR. The colorspace is purely
informative. It will be saved to the file header, but does not affect
how chunks are en-/decoded. */

enum DebugCounter {
    DEBUG_COUNTER_DRIVE_READ,
    DEBUG_COUNTER_DRIVE_WRITE,

    DEBUG_COUNTER_SIZE,
};

#ifdef DEBUG
#undef DEBUG
#endif

#define QOI_SRGB   0
#define QOI_LINEAR 1

typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned char channels;
	unsigned char colorspace;
} qoi_desc;

/* -----------------------------------------------------------------------------
Implementation */

#include <stdlib.h>
#include <string.h>

#ifndef QOI_MALLOC
	#define QOI_MALLOC(sz) malloc(sz)
	#define QOI_FREE(p)    free(p)
#endif
#ifndef QOI_ZEROARR
	#define QOI_ZEROARR(a) memset((a),0,sizeof(a))
#endif

#define QOI_OP_LUMA5551 0x00 /* 0xxxxxxx */
#define QOI_OP_LUMA2221 0x80 /* 10xxxxxx */
#define QOI_OP_LUMA7771 0xc0 /* 110xxxxx */
#define QOI_OP_RUN1     0xe0 /* 111xxxxx */
#define QOI_OP_RGB1     0xfe /* 11111110 */
#define QOI_OP_RGBA1    0xff /* 11111111 */

#define QOI_MASK_11     0x80 /* 10000000 */
#define QOI_MASK_21     0xc0 /* 11000000 */
#define QOI_MASK_31     0xe0 /* 11100000 */

#define QOI_COLOR_HASH1(C) (C.rgba.r*3 + C.rgba.g*5 + C.rgba.b*7 + C.rgba.a*11)
#define QOI_MAGIC1 \
	(((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | \
	 ((unsigned int)'i') <<  8 | ((unsigned int)'f'))
#define QOI_HEADER_SIZE1 14

/* 2GB is the max file size that this implementation can safely handle. We guard
against anything larger than that, assuming the worst case with 5 bytes per
pixel, rounded down to a nice clean value. 400 million pixels ought to be
enough for anybody. */
#define QOI_PIXELS_MAX1 ((unsigned int)400000000)

typedef union {
	struct { unsigned char r, g, b, a; } rgba;
	unsigned int v;
} qoi_rgba_t;


#define RGB_ENC_SCALAR do{\
	signed char vr = px.rgba.r - px_prev.rgba.r;\
	signed char vg = px.rgba.g - px_prev.rgba.g;\
	signed char vb = px.rgba.b - px_prev.rgba.b;\
	signed char vg_r = vr - vg;\
	signed char vg_b = vb - vg;\
	unsigned char ar = (vg_r<0)?(-vg_r)-1:vg_r;\
	unsigned char ag = (vg<0)?(-vg)-1:vg;\
	unsigned char ab = (vg_b<0)?(-vg_b)-1:vg_b;\
	unsigned char argb = ar|ag|ab;\
	switch(optable1[argb]){\
		case 0:\
			bytes[p++] = QOI_OP_LUMA2221 | ((vg_r + 2) << 4) | ((vg_b + 2) << 2) | (vg + 2);\
			break;\
		case 1:\
			bytes[p++] = QOI_OP_LUMA5551    | ((vg_b   + 16) << 2) | ((vg_r + 16)>>3);\
			bytes[p++] = (((vg_r + 16) & 7) << 5) | (vg +  16);\
			break;\
		case 2:\
			bytes[p++] = QOI_OP_LUMA7771     | ((vg_b + 64)>>2);\
			bytes[p++] = (((vg_b+64)&3)<<6) | ((vg_r + 64)>>1);\
			bytes[p++] = (((vg_r+64)&1)<<7) | (vg+64);\
			break;\
		case 3:\
			bytes[p++] = QOI_OP_RGB1;\
			bytes[p++] = px.rgba.r;\
			bytes[p++] = px.rgba.g;\
			bytes[p++] = px.rgba.b;\
			break;\
	}\
}while(0)

static const unsigned char qoi_padding[8] = {0,0,0,0,0,0,0,1};

static void qoi_write_32(unsigned char *bytes, int *p, unsigned int v) {
	*((unsigned int*) &bytes[*p]) = v;
	(*p) += 4;
}

static unsigned int qoi_read_32(const unsigned char *bytes, int *p) {
	(*p) += 4;

	return *((unsigned int*) &bytes[(*p) - 4]);
}

const unsigned char optable1[128]={0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};

void *qoi_encode1(void *data, const qoi_desc *desc, int *out_len) {
	int i, max_size, p, run;
	int px_len, px_end, px_pos, channels;
	unsigned char *bytes;
	const unsigned char *pixels;
	qoi_rgba_t px, px_prev;

	max_size =
		desc->width * desc->height * (desc->channels + 1) +
		QOI_HEADER_SIZE1 + sizeof(qoi_padding);

	p = 0;
	bytes = (unsigned char *) QOI_MALLOC(max_size);
	if (!bytes) {
		return NULL;
	}

	qoi_write_32(bytes, &p, QOI_MAGIC1);
	qoi_write_32(bytes, &p, desc->width);
	qoi_write_32(bytes, &p, desc->height);
	bytes[p++] = desc->channels;
	bytes[p++] = desc->colorspace;

	pixels = (const unsigned char *)data;

	run = 0;
	px_prev.rgba.r = 0;
	px_prev.rgba.g = 0;
	px_prev.rgba.b = 0;
	px_prev.rgba.a = 255;
	px = px_prev;

	px_len = desc->width * desc->height * desc->channels;
	px_end = px_len - desc->channels;
	channels = desc->channels;

	if (channels == 4) {
		for (px_pos = 0; px_pos < px_len; px_pos += 4) {
			px.rgba.r = pixels[px_pos + 0];
			px.rgba.g = pixels[px_pos + 1];
			px.rgba.b = pixels[px_pos + 2];
			px.rgba.a = pixels[px_pos + 3];

			while(px.v == px_prev.v) {
				++run;
				if(px_pos == px_end) {
					bytes[p++] = (unsigned char) (QOI_OP_RUN1 | (run - 1));
					goto DONE;
				}
				else if (run == 30) {
					bytes[p++] = (unsigned char) (QOI_OP_RUN1 | (run - 1));
					run = 0;
				}
				px_pos+=4;
				px.rgba.r = pixels[px_pos + 0];
				px.rgba.g = pixels[px_pos + 1];
				px.rgba.b = pixels[px_pos + 2];
				px.rgba.a = pixels[px_pos + 3];
			}
			if (run) {
				bytes[p++] = (unsigned char) (QOI_OP_RUN1 | (run - 1));
				run = 0;
			}

			if(px.rgba.a!=px_prev.rgba.a){
				bytes[p++] = QOI_OP_RGBA1;
				bytes[p++] = px.rgba.a;
			}

			RGB_ENC_SCALAR;
			px_prev = px;
		}
	}
	else {
		for (px_pos = 0; px_pos < px_len; px_pos += 3) {
			px.rgba.r = pixels[px_pos + 0];
			px.rgba.g = pixels[px_pos + 1];
			px.rgba.b = pixels[px_pos + 2];

			while(px.v == px_prev.v) {
				++run;
				if(px_pos == px_end) {
					bytes[p++] = (unsigned char) (QOI_OP_RUN1 | (run - 1));
					goto DONE;
				}
				else if (run == 30) {
					bytes[p++] = (unsigned char) (QOI_OP_RUN1 | (run - 1));
					run = 0;
				}
				px_pos+=3;
				px.rgba.r = pixels[px_pos + 0];
				px.rgba.g = pixels[px_pos + 1];
				px.rgba.b = pixels[px_pos + 2];
			}
			if (run) {
				bytes[p++] = (unsigned char) (QOI_OP_RUN1 | (run - 1));
				run = 0;
			}

			RGB_ENC_SCALAR;
			px_prev = px;
		}
	}
	DONE:

	for (i = 0; i < (int)sizeof(qoi_padding); i++) {
		bytes[p++] = qoi_padding[i];
	}

	*out_len = p;
	return bytes;
}

void *qoi_decode1(const void *data, int size, qoi_desc *desc, int channels) {
	const unsigned char *bytes;
	unsigned int header_magic;
	unsigned char *pixels;
	qoi_rgba_t px;
	int px_len, chunks_len, px_pos;
	int p = 0, run = 0;

	if (
		data == NULL || desc == NULL ||
		(channels != 0 && channels != 3 && channels != 4) ||
		size < QOI_HEADER_SIZE1 + (int)sizeof(qoi_padding)
	) {
		return NULL;
	}

	bytes = (const unsigned char *)data;

	header_magic = qoi_read_32(bytes, &p);
	desc->width = qoi_read_32(bytes, &p);
	desc->height = qoi_read_32(bytes, &p);
	desc->channels = bytes[p++];
	desc->colorspace = bytes[p++];

	if (channels == 0) {
		channels = desc->channels;
	}

	px_len = desc->width * desc->height * channels;
	pixels = (unsigned char *) QOI_MALLOC(px_len);
	if (!pixels) {
		return NULL;
	}

	px.rgba.r = 0;
	px.rgba.g = 0;
	px.rgba.b = 0;
	px.rgba.a = 255;

	chunks_len = size - (int)sizeof(qoi_padding);
	for (px_pos = 0; px_pos < px_len; px_pos += channels) {
		if (run > 0) {
			run--;
		}
		else {
			OP_RGBA_GOTO:
			int b1 = bytes[p++];
			if (b1 == QOI_OP_RGB1) {
				px.rgba.r = bytes[p++];
				px.rgba.g = bytes[p++];
				px.rgba.b = bytes[p++];
			}
			else if (b1 == QOI_OP_RGBA1) {
				px.rgba.a = bytes[p++];
				goto OP_RGBA_GOTO;
			}
			else if ((b1 & QOI_MASK_21) == QOI_OP_LUMA2221) {
				int vg = (b1 & 3) - 2;
				px.rgba.r += (unsigned char) (vg - 2 + ((b1 >> 4) & 3));
				px.rgba.g += (unsigned char) (vg);
				px.rgba.b += (unsigned char) (vg - 2 + ((b1 >> 2) & 3));
			}
			else if ((b1 & QOI_MASK_11) == QOI_OP_LUMA5551) {
				int b2 = bytes[p++];
				int vg = (b2 & 31) - 16;
				px.rgba.r += (unsigned char) (vg - 16 + (((b1&3)<<3) | (b2>>5)));
				px.rgba.g += (unsigned char) (vg);
				px.rgba.b += (unsigned char) (vg - 16 +  ((b1 >>2)&31));
			}
			else if ((b1 & QOI_MASK_31) == QOI_OP_LUMA7771) {
				int b2 = bytes[p++];
				int b3 = bytes[p++];
				int vg = (b3 & 0x7f) - 64;
				px.rgba.r += (unsigned char) (vg - 64 + ((b2 & 0x3f)<<1) + (b3>>7));
				px.rgba.g += (unsigned char) (vg);
				px.rgba.b += (unsigned char) (vg - 64 + ((b1 & 0x1f)<<2) + (b2>>6));
			}
			else if ((b1 & QOI_MASK_31) == QOI_OP_RUN1) {
				run = (b1 & 0x1f);
			}
		}
		pixels[px_pos + 0] = px.rgba.r;
		pixels[px_pos + 1] = px.rgba.g;
		pixels[px_pos + 2] = px.rgba.b;

		if (channels == 4) {
			pixels[px_pos + 3] = px.rgba.a;
		}
	}

	return pixels;
}

#include <stdio.h>

int qoi_write(const char *filename, void *data, const qoi_desc *desc) {
	FILE *f = fopen(filename, "wb");
	int size, err;
	void *encoded;

	if (!f) {
		return 0;
	}

	encoded = qoi_encode1(data, desc, &size);
	if (!encoded) {
		fclose(f);
		return 0;
	}

	fflush(f);
	err = ferror(f);
	fclose(f);

	QOI_FREE(encoded);
	return err ? 0 : size;
}

void *qoi_read(const char *filename, qoi_desc *desc, int channels) {
	FILE *f = fopen(filename, "rb");
	int size, bytes_read;
	void *pixels, *data;

	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	if (size <= 0 || fseek(f, 0, SEEK_SET) != 0) {
		fclose(f);
		return NULL;
	}

	data = QOI_MALLOC(size);
	if (!data) {
		fclose(f);
		return NULL;
	}

	bytes_read = (int) fread(data, 1, size, f);
	fclose(f);
	pixels = (bytes_read != size) ? NULL : qoi_decode1(data, bytes_read, desc, channels);
	QOI_FREE(data);
	return pixels;
}

#define STB_IMAGE_IMPLEMENTATION
#include "../GameEngine/image/stb_image.h"
#include "../GameEngine/image/qoi.h"

int main(int argc, char* argv[])
{
    int w2,h2;
    unsigned char* data2 = stbi_load("C:/Users/spl1nes/Documents/git/TalesOfSouls/GameAssets/images/font_atlas1.png", &w2, &h2, 0, 4);

    int out_len;
    qoi_desc desc = {};
	desc.width = w2;
	desc.height = h2;
	desc.channels = 4;
    unsigned char * data = (unsigned char *) qoi_encode1(data2, &desc, &out_len);

	void* data_normal = qoi_decode1(data, out_len, &desc, 4);

	int a = memcmp(data2, (char *) data_normal, 512*512);

	Image image = {};
	image.width = w2;
	image.height = h2;
	image.pixel_count = w2 * h2;
	image.image_settings |= 4;
	image.pixels = (unsigned char*) data2;

	unsigned char * data3 = (unsigned char *) malloc(1024 * 1000 * 10);
	qoi_encode(&image, (unsigned char *) data3);

	image.pixels = (unsigned char*) malloc(1024 * 1000 *10);
	qoi_decode((unsigned char*) data3, &image);

	int t = memcmp(data + 14, data3 + 9, out_len - 14 - 8);

	for (int i = 0 ; i < 512*512; ++i) {
		if (image.pixels[i] != data2[i]) {
			int qwer = 1;
		}
	}
	int b = memcmp(data2, image.pixels, 512*512);

    int c = 1;
}