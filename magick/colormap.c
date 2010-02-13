/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%            CCCC   OOO   L       OOO   RRRR   M   M   AAA   PPPP             %
%           C      O   O  L      O   O  R   R  MM MM  A   A  P   P            %
%           C      O   O  L      O   O  RRRR   M M M  AAAAA  PPPP             %
%           C      O   O  L      O   O  R R    M   M  A   A  P                %
%            CCCC   OOO   LLLLL   OOO   R  R   M   M  A   A  P                %
%                                                                             %
%                                                                             %
%                        MagickCore Colormap Methods                          %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  We use linked-lists because splay-trees do not currently support duplicate
%  key / value pairs (.e.g X11 green compliance and SVG green compliance).
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/cache-view.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/client.h"
#include "magick/configure.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-private.h"
#include "magick/quantize.h"
#include "magick/quantum.h"
#include "magick/semaphore.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/utility.h"
#include "magick/xml-tree.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C y c l e C o l o r m a p I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CycleColormap() displaces an image's colormap by a given number of
%  positions.  If you cycle the colormap a number of times you can produce
%  a psychodelic effect.
%
%  The format of the CycleColormapImage method is:
%
%      MagickBooleanType CycleColormapImage(Image *image,const long displace)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o displace:  displace the colormap this amount.
%
*/
MagickExport MagickBooleanType CycleColormapImage(Image *image,
  const long displace)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    y;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == DirectClass)
    (void) SetImageType(image,PaletteType);
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status)
#endif
  for (y=0; y < (long) image->rows; y++)
  {
    long
      index;

    register IndexPacket
      *restrict indexes;

    register long
      x;

    register PixelPacket
      *restrict q;

    if (status == MagickFalse)
      continue;
    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      index=(long) (indexes[x]+displace) % image->colors;
      if (index < 0)
        index+=image->colors;
      indexes[x]=(IndexPacket) index;
      q->red=image->colormap[index].red;
      q->green=image->colormap[index].green;
      q->blue=image->colormap[index].blue;
      q++;
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheView(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S o r t C o l o r m a p B y I n t e n s i t y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SortColormapByIntensity() sorts the colormap of a PseudoClass image by
%  decreasing color intensity.
%
%  The format of the SortColormapByIntensity method is:
%
%      MagickBooleanType SortColormapByIntensity(Image *image)
%
%  A description of each parameter follows:
%
%    o image: A pointer to an Image structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int IntensityCompare(const void *x,const void *y)
{
  const PixelPacket
    *color_1,
    *color_2;

  int
    intensity;

  color_1=(const PixelPacket *) x;
  color_2=(const PixelPacket *) y;
  intensity=(int) PixelIntensityToQuantum(color_2)-
    (int) PixelIntensityToQuantum(color_1);
  return(intensity);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport MagickBooleanType SortColormapByIntensity(Image *image)
{
  CacheView
    *image_view;

  ExceptionInfo
    *exception;

  long
    y;

  MagickBooleanType
    status;

  register long
    i;

  unsigned short
    *pixels;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->storage_class != PseudoClass)
    return(MagickTrue);
  /*
    Allocate memory for pixel indexes.
  */
  pixels=(unsigned short *) AcquireQuantumMemory((size_t) image->colors,
    sizeof(*pixels));
  if (pixels == (unsigned short *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Assign index values to colormap entries.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (i=0; i < (long) image->colors; i++)
    image->colormap[i].opacity=(IndexPacket) i;
  /*
    Sort image colormap by decreasing color popularity.
  */
  qsort((void *) image->colormap,(size_t) image->colors,
    sizeof(*image->colormap),IntensityCompare);
  /*
    Update image colormap indexes to sorted colormap order.
  */
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(dynamic,4) shared(status)
#endif
  for (i=0; i < (long) image->colors; i++)
    pixels[(long) image->colormap[i].opacity]=(unsigned short) i;
  status=MagickTrue;
  exception=(&image->exception);
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    IndexPacket
      index;

    register long
      x;

    register IndexPacket
      *restrict indexes;

    register PixelPacket
      *restrict q;

    q=GetCacheViewAuthenticPixels(image_view,0,y,image->columns,1,exception);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewAuthenticIndexQueue(image_view);
    for (x=0; x < (long) image->columns; x++)
    {
      index=(IndexPacket) pixels[(long) indexes[x]];
      indexes[x]=index;
      *q++=image->colormap[(long) index];
    }
    if (SyncCacheViewAuthenticPixels(image_view,exception) == MagickFalse)
      status=MagickFalse;
    if (status == MagickFalse)
      break;
  }
  image_view=DestroyCacheView(image_view);
  pixels=(unsigned short *) RelinquishMagickMemory(pixels);
  return(status);
}
