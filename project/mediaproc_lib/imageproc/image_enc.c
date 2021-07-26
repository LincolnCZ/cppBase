// Copyright 2016 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Save image

#include "image_enc.h"

#include <assert.h>
#include <string.h>
#include <png.h>
#include <setjmp.h>   // note: this must be included *after* png.h

//------------------------------------------------------------------------------
// PNG
static void PNGAPI PNGErrorFunction(png_structp png, png_const_charp dummy) {
  (void)dummy;  // remove variable-unused warning
  longjmp(png_jmpbuf(png), 1);
}

int WebPWritePNG(FILE* out_file, const WebPDecBuffer* const buffer) {
  volatile png_structp png;
  volatile png_infop info;

  if (out_file == NULL || buffer == NULL) return 0;

  png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                NULL, PNGErrorFunction, NULL);
  if (png == NULL) {
    return 0;
  }
  info = png_create_info_struct(png);
  if (info == NULL) {
    png_destroy_write_struct((png_structpp)&png, NULL);
    return 0;
  }
  if (setjmp(png_jmpbuf(png))) {
    png_destroy_write_struct((png_structpp)&png, (png_infopp)&info);
    return 0;
  }
  png_init_io(png, out_file);
  {
    const uint32_t width = buffer->width;
    const uint32_t height = buffer->height;
    png_bytep row = buffer->u.RGBA.rgba;
    const int stride = buffer->u.RGBA.stride;
    const int has_alpha = WebPIsAlphaMode(buffer->colorspace);
    uint32_t y;

    png_set_IHDR(png, info, width, height, 8,
                 has_alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);
    for (y = 0; y < height; ++y) {
      png_write_rows(png, &row, 1);
      row += stride;
    }
  }
  png_write_end(png, info);
  png_destroy_write_struct((png_structpp)&png, (png_infopp)&info);
  return 1;
}