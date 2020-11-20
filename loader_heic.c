/* File: loader_heic.c
   Time-stamp: <2012-12-09 21:19:30 gawen>

   Copyright (c) 2011 David Hauweele <david@hauweele.net>
   All rights reserved.
   
   Modified by Vitaly "_Vi" Shukela to use heic instead of webp.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. Neither the name of the University nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE. */

#define _BSD_SOURCE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include <Imlib2.h>

#include "imlib2_common.h"
#include "loader.h"

#include <inttypes.h>
#include "libheif/heif.h"

char load(ImlibImage * im, ImlibProgressFunction progress,
          char progress_granularity, char immediate_load)
{
  
  int w,h;
  int x=0,y=0;
  int alpha;

  char retcode = 0;
  struct heif_context *ctx = heif_context_alloc();
  struct heif_error ret;
  struct heif_image_handle *imh = NULL;
  struct heif_image *img = NULL;
  
  if(im->data)
    return 0;


  ret =  heif_context_read_from_file(ctx, im->real_file, NULL);

  if (ret.code != heif_error_Ok) {
		goto EXIT;
  }

  ret = heif_context_get_primary_image_handle(ctx, &imh);
  if (ret.code != heif_error_Ok) {
		goto EXIT;
  }
  if (!imh) {
		goto EXIT;
  }

  w = heif_image_handle_get_width(imh);
  h = heif_image_handle_get_height(imh);
  alpha = heif_image_handle_has_alpha_channel(imh);
  
  im->w = w;
  im->h = h;

  if(!IMAGE_DIMENSIONS_OK(w, h))
      goto EXIT;
  
  if(progress) {
      progress(im, 0, 0, 0, w, h);
  }

  if (!immediate_load) {
      retcode = 1;
      goto EXIT;
  }

  ret = heif_decode_image(imh, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, NULL);
  if (!imh) {
		goto EXIT;
  }
  
  /*struct heif_error heif_decode_image(const struct heif_image_handle* in_handle,
                                    struct heif_image** out_img,
                                    enum heif_colorspace colorspace,
                                    enum heif_chroma chroma,
                                    const struct heif_decoding_options* options);*/


  int stride = 0;
  uint8_t *plane  = heif_image_get_plane(img, heif_channel_interleaved, &stride);

  if (!plane) {
		goto EXIT;
  }

  uint8_t *bgra;
  bgra = (uint8_t*)malloc(4 * w * h);
  if (!bgra) goto EXIT;

  int channel = alpha ? 4 : 3;

  for (y = 0; y < h; ++y) {
    for (x = 0; x < w; ++x) {
      bgra[4 * (y * w + x) + 0] =         plane[y * stride + channel * x + 2];
      bgra[4 * (y * w + x) + 1] =         plane[y * stride + channel * x + 1];
      bgra[4 * (y * w + x) + 2] =         plane[y * stride + channel * x + 0];
      bgra[4 * (y * w + x) + 3] = alpha ? plane[y * stride + channel * x + 3] : 255;
    }
  }

  im->data = (DATA32*)bgra;
  if(progress)
      progress(im, 100, 0, 0, w, h);

  SET_FLAGS(im->flags, F_HAS_ALPHA);
    
  im->format = strdup("heif");
  retcode = 1;

EXIT:
  if (ctx) heif_context_free(ctx);
  if (imh) heif_image_handle_release(imh);
  if (img) heif_image_release(img);

  return retcode;
}

char save(ImlibImage *im, ImlibProgressFunction progress,
          char progress_granularity)
{
  return 0;
}

void formats(ImlibLoader *l)
{
  int i;
  char *list_formats[] = { "heic", "heif" };

  l->num_formats = (sizeof(list_formats) / sizeof(char *));
  l->formats     = malloc(sizeof(char *) * l->num_formats);
  for(i = 0 ; i < l->num_formats ; i++)
    l->formats[i] = strdup(list_formats[i]);
}
