// Copyright 2015 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Utilities for animated images

#include "anim_util.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include "core/logger.h"
static const int kNumChannels = 4;

// -----------------------------------------------------------------------------
// Common utilities.

static int CheckSizeForOverflow(uint64_t size) {
  return (size == (size_t)size);
}

static int AllocateFrames(AnimatedImage* const image, uint32_t num_frames) {
  uint32_t i;
  uint8_t* mem = NULL;
  DecodedFrame* frames = NULL;
  const uint64_t rgba_size =
      (uint64_t)image->canvas_width * kNumChannels * image->canvas_height;
  const uint64_t total_size = (uint64_t)num_frames * rgba_size * sizeof(*mem);
  const uint64_t total_frame_size = (uint64_t)num_frames * sizeof(*frames);
  if (!CheckSizeForOverflow(total_size) ||
      !CheckSizeForOverflow(total_frame_size)) {
    return 0;
  }
  mem = (uint8_t*)WebPMalloc((size_t)total_size);
  frames = (DecodedFrame*)WebPMalloc((size_t)total_frame_size);

  if (mem == NULL || frames == NULL) {
    WebPFree(mem);
    WebPFree(frames);
    return 0;
  }
  WebPFree(image->raw_mem);
  image->num_frames = num_frames;
  image->frames = frames;
  for (i = 0; i < num_frames; ++i) {
    frames[i].rgba = mem + i * rgba_size;
    frames[i].duration = 0;
    frames[i].is_key_frame = 0;
  }
  image->raw_mem = mem;
  return 1;
}

void ClearAnimatedImage(AnimatedImage* const image) {
  if (image != NULL) {
    WebPFree(image->raw_mem);
    WebPFree(image->frames);
    image->num_frames = 0;
    image->frames = NULL;
    image->raw_mem = NULL;
  }
}

// Canonicalize all transparent pixels to transparent black to aid comparison.
static void CleanupTransparentPixels(uint32_t* rgba,
                                     uint32_t width, uint32_t height) {
  const uint32_t* const rgba_end = rgba + width * height;
  while (rgba < rgba_end) {
    const uint8_t alpha = (*rgba >> 24) & 0xff;
    if (alpha == 0) {
      *rgba = 0;
    }
    ++rgba;
  }
}

// -----------------------------------------------------------------------------
// WebP Decoding.

// Returns true if this is a valid WebP bitstream.
static int IsWebP(const WebPData* const webp_data) {
  return (WebPGetInfo(webp_data->bytes, webp_data->size, NULL, NULL) != 0);
}

// Read animated WebP bitstream 'webp_data' into 'AnimatedImage' struct.
static int ReadAnimatedWebP(const char *task_id,
                            const WebPData* const webp_data,
                            AnimatedImage* const image) {
  int ok = 0;
  uint32_t frame_index = 0;
  int prev_frame_timestamp = 0;
  WebPAnimDecoder* dec;
  WebPAnimInfo anim_info;

  memset(image, 0, sizeof(*image));

  dec = WebPAnimDecoderNew(webp_data, NULL);
  if (dec == NULL) {
    FUNLOG(Error, "Error parsing image: %s", task_id);
    goto End;
  }

  if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
    FUNLOG(Error, "Error getting global info about the animation");
    goto End;
  }

  // Animation properties.
  image->canvas_width = anim_info.canvas_width;
  image->canvas_height = anim_info.canvas_height;
  image->loop_count = anim_info.loop_count;
  image->bgcolor = anim_info.bgcolor;

  // Allocate frames.
  if (!AllocateFrames(image, anim_info.frame_count)) return 0;

  // Decode frames.
  while (WebPAnimDecoderHasMoreFrames(dec)) {
    DecodedFrame* curr_frame;
    uint8_t* curr_rgba;
    uint8_t* frame_rgba;
    int timestamp;

    if (!WebPAnimDecoderGetNext(dec, &frame_rgba, &timestamp)) {
      FUNLOG(Error, "Error decoding frame, %s index %u", task_id, frame_index);
      goto End;
    }
    assert(frame_index < anim_info.frame_count);
    curr_frame = &image->frames[frame_index];
    curr_rgba = curr_frame->rgba;
    curr_frame->duration = timestamp - prev_frame_timestamp;
    curr_frame->is_key_frame = 0;  // Unused.
    memcpy(curr_rgba, frame_rgba,
           image->canvas_width * kNumChannels * image->canvas_height);

    // Needed only because we may want to compare with GIF later.
    CleanupTransparentPixels((uint32_t*)curr_rgba,
                             image->canvas_width, image->canvas_height);

    ++frame_index;
    prev_frame_timestamp = timestamp;
  }
  ok = 1;
  if (ok) image->format = ANIM_WEBP;

 End:
  WebPAnimDecoderDelete(dec);
  return ok;
}

int ReadAnimatedImage(const char *task_id, const WebPData *webp_data, AnimatedImage* const image) {
  int ok = 0;
  if (IsWebP(webp_data)) {
    ok = ReadAnimatedWebP(task_id, webp_data, image);
  } else {
    FUNLOG(Error, "%s, Supported WebP only", task_id);
    ok = 0;
  }
  if (!ok) ClearAnimatedImage(image);
  return ok;
}

static void Accumulate(double v1, double v2, double* const max_diff,
                       double* const sse) {
  const double diff = fabs(v1 - v2);
  if (diff > *max_diff) *max_diff = diff;
  *sse += diff * diff;
}

void GetDiffAndPSNR(const uint8_t rgba1[], const uint8_t rgba2[],
                    uint32_t width, uint32_t height, int premultiply,
                    int* const max_diff, double* const psnr) {
  const uint32_t stride = width * kNumChannels;
  const int kAlphaChannel = kNumChannels - 1;
  double f_max_diff = 0.;
  double sse = 0.;
  uint32_t x, y;
  for (y = 0; y < height; ++y) {
    for (x = 0; x < stride; x += kNumChannels) {
      int k;
      const size_t offset = (size_t)y * stride + x;
      const int alpha1 = rgba1[offset + kAlphaChannel];
      const int alpha2 = rgba2[offset + kAlphaChannel];
      Accumulate(alpha1, alpha2, &f_max_diff, &sse);
      if (!premultiply) {
        for (k = 0; k < kAlphaChannel; ++k) {
          Accumulate(rgba1[offset + k], rgba2[offset + k], &f_max_diff, &sse);
        }
      } else {
        // premultiply R/G/B channels with alpha value
        for (k = 0; k < kAlphaChannel; ++k) {
          Accumulate(rgba1[offset + k] * alpha1 / 255.,
                     rgba2[offset + k] * alpha2 / 255.,
                     &f_max_diff, &sse);
        }
      }
    }
  }
  *max_diff = (int)f_max_diff;
  if (*max_diff == 0) {
    *psnr = 99.;  // PSNR when images are identical.
  } else {
    sse /= stride * height;
    *psnr = 4.3429448 * log(255. * 255. / sse);
  }
}

void GetAnimatedImageVersions(int* const decoder_version,
                              int* const demux_version) {
  *decoder_version = WebPGetDecoderVersion();
  *demux_version = WebPGetDemuxVersion();
}
