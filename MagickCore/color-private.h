/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private image color methods.
*/
#ifndef MAGICKCORE_COLOR_PRIVATE_H
#define MAGICKCORE_COLOR_PRIVATE_H

#include "MagickCore/image.h"
#include "MagickCore/image-private.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern MagickPrivate MagickBooleanType
  ColorComponentGenesis(void),
  IsEquivalentAlpha(const Image *,const PixelInfo *,const PixelInfo *),
  IsEquivalentIntensity(const Image *,const PixelInfo *,const PixelInfo *);

extern MagickPrivate void
  ColorComponentTerminus(void);

static inline MagickBooleanType GetColorRange(const char *color,
  PixelInfo *start,PixelInfo *stop,ExceptionInfo *exception)
{
  char
    start_color[MagickPathExtent] = "white",
    stop_color[MagickPathExtent] = "black";

  MagickBooleanType
    status;

  if (*color != '\0')
    {
      char
        *p;

      (void) CopyMagickString(start_color,color,MagickPathExtent);
      for (p=start_color; (*p != '-') && (*p != '\0'); p++)
        if (*p == '(')
          {
            for (p++; (*p != ')') && (*p != '\0'); p++);
            if (*p == '\0')
              break;
          }
      if (*p == '-')
        (void) CopyMagickString(stop_color,p+1,MagickPathExtent);
      *p='\0';
    }
  status=QueryColorCompliance(start_color,AllCompliance,start,exception);
  if (status == MagickFalse)
    return(status);
  return(QueryColorCompliance(stop_color,AllCompliance,stop,exception));
}

static inline double GetFuzzyColorDistance(const Image *p,const Image *q)
{
  double
    fuzz;

  fuzz=(double) MagickMax(MagickMax(p->fuzz,q->fuzz),(MagickRealType)
    MagickSQ1_2);
  return(fuzz*fuzz);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
