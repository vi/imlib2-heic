/* File: loader_bpg.c
   Time-stamp: <2012-12-09 21:19:30 gawen>

   Copyright (c) 2011 David Hauweele <david@hauweele.net>
   All rights reserved.
   
   Modified by Vitaly "_Vi" Shukela to use bpg instead of webp.

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
#include "libbpg.h"

char load(ImlibImage * im, ImlibProgressFunction progress,
          char progress_granularity, char immediate_load)
{
  
  int w,h;
  uint8_t *buf = NULL;
  int buf_len, buf_len_max;
  FILE *f = NULL;
  BPGImageInfo img_info_s, *img_info = &img_info_s;
  BPGDecoderContext *img = NULL;
  uint8_t *bgra = NULL;
  int decoded_image_used = 0;
  int y=0;
  
  if(im->data)
    return 0;


  f = fopen(im->real_file, "rb");
  if (!f) {
        goto EXIT;
  }

  {
    fseek(f, 0, SEEK_END);
    buf_len_max = ftell(f);
    fseek(f, 0, SEEK_SET);
  }
  if (buf_len_max < 1) {
     buf_len_max = BPG_DECODER_INFO_BUF_SIZE;
  }

  buf = malloc(buf_len_max);
  if (!buf) {
		goto EXIT;
  }
  buf_len = fread(buf, 1, buf_len_max, f);

  fclose(f); f = NULL;

  
  img = bpg_decoder_open();

  if (bpg_decoder_decode(img, buf, buf_len) < 0) {
		goto EXIT;
  }
  free(buf); buf = NULL;

  
  bpg_decoder_get_info(img, img_info);
    
  w = img_info->width;
  h = img_info->height;

  bgra = malloc(4 * w * h);
  if (!bgra) goto EXIT;

  bpg_decoder_start(img, BPG_OUTPUT_FORMAT_RGBA32);
  for (y = 0; y < h; ++y) {
     bpg_decoder_get_line(img, bgra + 4*w*y);

	 int x;
	 for (x=0; x < w; ++x) {
		#define SWAP(a,b) { unsigned char tmp; tmp=a; a=b; b=tmp; }
		SWAP(bgra[4*w*y + 4*x + 0], bgra[4*w*y + 4*x + 2]);
		#undef SWAP
	 }
  }

  bpg_decoder_close(img); img = NULL;
  
  
  if(!im->loader && !im->data) {
    im->w = w;
    im->h = h;

    if(!IMAGE_DIMENSIONS_OK(w, h))
      goto EXIT;

    SET_FLAGS(im->flags, F_HAS_ALPHA);
    
    im->format = strdup("bpg");
  }

  if((!im->data && im->loader) || immediate_load || progress) {
    im->data = (DATA32*)bgra;
    decoded_image_used = 1;
    if(progress)
      progress(im, 100, 0, 0, w, h);
    return 1;
  }

  

EXIT:
  if (f) fclose(f);
  if (buf) free(buf);
  if (img) bpg_decoder_close(img);
  if ((!decoded_image_used) && bgra) free(bgra);

  return 0;
}

char save(ImlibImage *im, ImlibProgressFunction progress,
          char progress_granularity)
{
  return 0;
}

void formats(ImlibLoader *l)
{
  int i;
  char *list_formats[] = { "bpg" };

  l->num_formats = (sizeof(list_formats) / sizeof(char *));
  l->formats     = malloc(sizeof(char *) * l->num_formats);
  for(i = 0 ; i < l->num_formats ; i++)
    l->formats[i] = strdup(list_formats[i]);
}
