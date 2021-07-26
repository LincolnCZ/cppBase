// Copyright 2016 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  All-in-one library to save PNG/JPEG/WebP/TIFF/WIC images.
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_IMAGEIO_IMAGE_ENC_H_
#define WEBP_IMAGEIO_IMAGE_ENC_H_

#include <stdio.h>

#include <webp/types.h>
#include <webp/decode.h>

#ifdef __cplusplus
extern "C" {
#endif

// Output types
typedef enum {
  PNG = 0,
  PAM,
  PPM,
  PGM,
  BMP,
  TIFF,
  RAW_YUV,
  ALPHA_PLANE_ONLY,  // this is for experimenting only
  // forced colorspace output (for testing, mostly)
  RGB, RGBA, BGR, BGRA, ARGB,
  RGBA_4444, RGB_565,
  rgbA, bgrA, Argb, rgbA_4444,
  YUV, YUVA
} WebPOutputFileFormat;

// Save to PNG.
#define WEBP_HAVE_PNG 1
int WebPWritePNG(FILE* out_file, const WebPDecBuffer* const buffer);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  // WEBP_IMAGEIO_IMAGE_ENC_H_
