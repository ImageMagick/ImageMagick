/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%           PPPP    RRRR    EEEEE  PPPP   RRRR   EEEEE  SSSSS  SSSSS          %
%           P   P   R   R   E      P   P  R   R  E      SS     SS             %
%           PPPP    RRRR    EEE    PPPP   RRRR   EEE     SSS    SSS           %
%           P       R R     E      P      R R    E         SS     SS          %
%           P       R  R    EEEEE  P      R  R   EEEEE  SSSSS  SSSSS          %
%                                                                             %
%                                                                             %
%                         MagickCore Prepress Methods                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                October 2001                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/cache-view.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/hashmap.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/memory_.h"
#include "magick/pixel-accessor.h"
#include "magick/prepress.h"
#include "magick/registry.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"
#include "magick/thread-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e T o t a l I n k D e n s i t y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageTotalInkDensity() returns the total ink density for a CMYK image.
%  Total Ink Density (TID) is determined by adding the CMYK values in the
%  darkest shadow area in an image.
%
%  The format of the GetImageTotalInkDensity method is:
%
%      double GetImageTotalInkDensity(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport double GetImageTotalInkDensity(Image *image)
{
  CacheView
    *image_view;

  double
    total_ink_density;

  ExceptionInfo
    *exception;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->colorspace != CMYKColorspace)
    {
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        ImageError,"ColorSeparatedImageRequired","`%s'",image->filename);
      return(0.0);
    }
  status=MagickTrue;
  total_ink_density=0.0;
  exception=(&image->exception);
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static,4) shared(status) \
    dynamic_number_threads(image,image->columns,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    double
      density;

    register const IndexPacket
      *indexes;

    register const PixelPacket
      *p;

    register ssize_t
      x;

    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewVirtualIndexQueue(image_view);
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      density=(double) GetPixelRed(p)+GetPixelGreen(p)+
        GetPixelBlue(p)+GetPixelIndex(indexes+x);
      if (density > total_ink_density)
#if defined(MAGICKCORE_OPENMP_SUPPORT)
        #pragma omp critical (MagickCore_GetImageTotalInkDensity)
#endif
        {
          if (density > total_ink_density)
            total_ink_density=density;
        }
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    total_ink_density=0.0;
  return(total_ink_density);
}
