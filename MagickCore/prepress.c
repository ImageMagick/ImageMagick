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
%                                   Cristy                                    %
%                                October 2001                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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
#include "MagickCore/studio.h"
#include "MagickCore/cache-view.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/prepress.h"
#include "MagickCore/thread-private.h"

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
%      double GetImageTotalInkDensity(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport double GetImageTotalInkDensity(Image *image,ExceptionInfo *exception)
{
  CacheView
    *image_view;

  double
    total_ink_density;

  MagickBooleanType
    status;

  ssize_t
    y;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->colorspace != CMYKColorspace)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),ImageError,
        "ColorSeparatedImageRequired","`%s'",image->filename);
      return(0.0);
    }
  status=MagickTrue;
  total_ink_density=0.0;
  image_view=AcquireVirtualCacheView(image,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  #pragma omp parallel for schedule(static) shared(status) \
    reduction(max:total_ink_density) \
    magick_number_threads(image,image,image->rows,1)
#endif
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *p;

    double
      row_max_density;

    ssize_t
      x;

    if (status == MagickFalse)
      continue;
    p=GetCacheViewVirtualPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    row_max_density=0.0;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      double
        density;

      density=(double) GetPixelCyan(image,p)+(double) GetPixelMagenta(image,p)+
        (double) GetPixelYellow(image,p)+(double) GetPixelBlack(image,p);
      if (density > row_max_density)
        row_max_density=density;
      p+=(ptrdiff_t) GetPixelChannels(image);
    }
    if (row_max_density > total_ink_density)
      total_ink_density=row_max_density;
  }
  image_view=DestroyCacheView(image_view);
  if (status == MagickFalse)
    total_ink_density=0.0;
  return(total_ink_density);
}
