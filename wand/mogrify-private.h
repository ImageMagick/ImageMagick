/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand mogrify command-line private methods.
*/
#ifndef _MAGICKWAND_MOGRIFY_PRIVATE_H
#define _MAGICKWAND_MOGRIFY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define AppendImageStack(images) \
{ \
  (void) SyncImagesSettings(image_info,images); \
  AppendImageToList(&image_stack[k].image,images); \
  image=image_stack[k].image; \
}
#define DegreesToRadians(x)  (MagickPI*(x)/180.0)
#define DestroyImageStack() \
{ \
  while (k > 0) \
    PopImageStack(); \
  image_stack[k].image=DestroyImageList(image_stack[k].image); \
  image_stack[k].image_info=DestroyImageInfo(image_stack[k].image_info); \
}
#define FinalizeImageSettings(image_info,image,advance) \
{ \
  FireImageStack(MagickTrue,advance,MagickTrue); \
  if (image != (Image *) NULL) \
    { \
      InheritException(exception,&(image)->exception); \
      (void) SyncImagesSettings(image_info,image); \
    } \
}
#define FireImageStack(postfix,advance,fire) \
  if ((j <= i) && (i < (ssize_t) argc)) \
    { \
      if (image_stack[k].image == (Image *) NULL) \
        status&=MogrifyImageInfo(image_stack[k].image_info,(int) (i-j+1), \
          (const char **) (argv+j),exception); \
      else \
        if ((fire) != MagickFalse) \
          { \
            status&=MogrifyImages(image_stack[k].image_info,postfix,(int) \
              (i-j+1),(const char **) (argv+j),&image_stack[k].image, \
              exception); \
            image=image_stack[k].image; \
            if ((advance) != MagickFalse) \
              j=i+1; \
            pend=MagickFalse; \
          } \
    }
#define MagickPI  3.14159265358979323846264338327950288419716939937510
#define MaxImageStackDepth  32
#define NewImageStack() \
{ \
  image_stack[0].image_info=CloneImageInfo(image_info); \
  image_stack[0].image=NewImageList(); \
  image_info=image_stack[0].image_info; \
  image=image_stack[0].image; \
}
#define PushImageStack() \
{ \
  k++; \
  image_stack[k].image_info=CloneImageInfo(image_stack[k-1].image_info); \
  image_stack[k].image=NewImageList(); \
  image_info=image_stack[k].image_info; \
  image=image_stack[k].image; \
}
#define PopImageStack() \
{ \
  if (respect_parenthesis == MagickFalse) \
    { \
      image_stack[k-1].image_info=DestroyImageInfo(image_stack[k-1].image_info); \
      image_stack[k-1].image_info=CloneImageInfo(image_stack[k].image_info); \
    } \
  image_stack[k].image_info=DestroyImageInfo(image_stack[k].image_info); \
  AppendImageToList(&image_stack[k-1].image,image_stack[k].image); \
  k--; \
  image_info=image_stack[k].image_info; \
  image=image_stack[k].image; \
}
#define QuantumScale  ((MagickRealType) 1.0/(MagickRealType) QuantumRange)
#define QuantumTick(i,span) ((MagickBooleanType) ((((i) & ((i)-1)) == 0) || \
   (((i) & 0xfff) == 0) || \
   ((MagickOffsetType) (i) == ((MagickOffsetType) (span)-1))))
#define RadiansToDegrees(x) (180.0*(x)/MagickPI)
#define RemoveImageStack(images) \
{ \
  images=RemoveFirstImageFromList(&image_stack[k].image); \
  image=image_stack[k].image; \
}
#define RemoveAllImageStack() \
{ \
  if (image_stack[k].image != (Image *) NULL) \
    image_stack[k].image=DestroyImageList(image_stack[k].image); \
}
#define SetImageStack(image) \
{ \
  image_stack[k].image=(image); \
}

typedef struct _ImageStack
{
  ImageInfo
    *image_info;

  Image
    *image;
} ImageStack;

static MagickBooleanType
  respect_parenthesis = MagickFalse;

static inline MagickRealType MagickPixelIntensity(
  const MagickPixelPacket *pixel)
{
  MagickRealType
    intensity;

  intensity=0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue;
  return(intensity);
}

static inline Quantum MagickPixelIntensityToQuantum(
  const MagickPixelPacket *pixel)
{
  MagickRealType
    intensity;

  intensity=0.299*pixel->red+0.587*pixel->green+0.114*pixel->blue;
  return((Quantum) (intensity+0.5));
}

static inline MagickRealType PixelIntensity(const PixelPacket *pixel)
{
  MagickRealType
    intensity;

  intensity=(MagickRealType) (0.299*pixel->red+0.587*pixel->green+
    0.114*pixel->blue);
  return(intensity);
}

static inline Quantum PixelIntensityToQuantum(const PixelPacket *pixel)
{
  MagickRealType
    intensity;

  intensity=(MagickRealType) (0.299*pixel->red+0.587*pixel->green+
    0.114*pixel->blue);
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  return((Quantum) (intensity+0.5));
#else
  return((Quantum) intensity);
#endif
}

static inline void SetMagickPixelPacket(const Image *image,
  const PixelPacket *color,const IndexPacket *index,MagickPixelPacket *pixel)
{
  pixel->red=(MagickRealType) color->red;
  pixel->green=(MagickRealType) color->green;
  pixel->blue=(MagickRealType) color->blue;
  if (image->matte != MagickFalse)
    pixel->opacity=(MagickRealType) color->opacity;
  if (((image->colorspace == CMYKColorspace) ||
       (image->storage_class == PseudoClass)) &&
      (index != (const IndexPacket *) NULL))
    pixel->index=(MagickRealType) *index;
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
