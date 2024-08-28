/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand mogrify command-line private methods.
*/
#ifndef MAGICKWAND_MOGRIFY_PRIVATE_H
#define MAGICKWAND_MOGRIFY_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define AppendImageStack(images) \
{ \
  (void) SyncImagesSettings(image_info,images,exception); \
  AppendImageToList(&image_stack[k].image,images); \
  image=image_stack[k].image; \
}
#define DestroyImageStack() \
{ \
  while (k > 0) \
    PopImageStack(); \
  image_stack[k].image=DestroyImageList(image_stack[k].image); \
  image_stack[k].image_info=DestroyImageInfo(image_stack[k].image_info); \
  image_info=image_stack[MaxImageStackDepth].image_info; \
}
#define FinalizeImageSettings(image_info,image,advance) \
{ \
  FireImageStack(MagickTrue,advance,MagickTrue); \
  if (image != (Image *) NULL) \
    (void) SyncImagesSettings(image_info,image,exception); \
}
#define FireImageStack(postfix,advance,fire) \
  if ((j <= i) && (i < (ssize_t) argc)) \
    { \
DisableMSCWarning(4127) \
      if (image_stack[k].image == (Image *) NULL) \
        status&=(MagickStatusType) MogrifyImageInfo(image_stack[k].image_info, \
          (int) (i-j+1),(const char **) (argv+j),exception); \
      else \
        if ((fire) != MagickFalse) \
          { \
            status&=(MagickStatusType) MogrifyImages(image_stack[k].image_info,\
              postfix,(int) (i-j+1),(const char **) (argv+j), \
              &image_stack[k].image,exception); \
            image=image_stack[k].image; \
            if ((advance) != MagickFalse) \
              j=i+1; \
            pend=MagickFalse; \
          } \
RestoreMSCWarning \
    }
#define MaxImageStackDepth  128
#define NewImageStack() \
{ \
  image_stack[MaxImageStackDepth].image_info=image_info; \
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
  if (respect_parentheses == MagickFalse) \
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
#define QuantumTick(i,span) ((MagickBooleanType) ((((i) & ((i)-1)) == 0) || \
   (((i) & 0xfff) == 0) || \
   ((MagickOffsetType) (i) == ((MagickOffsetType) (span)-1))))
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

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
