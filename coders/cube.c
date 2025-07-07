/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         CCCC  U   U  BBBB   EEEEE                           %
%                        C      U   U  B   B  E                               %
%                        C      U   U  BBBB   EEE                             %
%                        C      U   U  B   B  E                               %
%                         CCCC   UUU   BBBB   EEEEE                           %
%                                                                             %
%                                                                             %
%                           Cube LUT Image Format                             %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2018                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% See Cube LUT specification 1.0 @
% https://wwwimages2.adobe.com/content/dam/acom/en/products/speedgrade/cc/pdfs/cube-lut-specification-1.0.pdf
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/token.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C U B E I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCUBEImage() creates a Cube color lookup table image and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadCUBEImage method is:
%
%      Image *ReadCUBEImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadCUBEImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
#define FlattenCube(level,b,g,r)  \
  ((ssize_t) ((b)*(level)*(level)+(g)*(level)+(r)))

  typedef struct _CubePixel
  {
    float
      r,
      g,
      b;
  } CubePixel;

  char
    *buffer,
    token[MagickPathExtent],
    value[MagickPathExtent];

  CubePixel
    *cube;

  Image
    *image;

  MagickBooleanType
    status;

  MemoryInfo
    *cube_info;

  char
    *p;

  size_t
    cube_level,
    hald_level;

  ssize_t
    b,
    i,
    n;

  /*
    Read CUBE color lookup table.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  cube_level=0;
  cube_info=(MemoryInfo *) NULL;
  cube=(CubePixel *) NULL;
  n=0;
  buffer=AcquireString("");
  *buffer='\0';
  p=buffer;
  while (ReadBlobString(image,p) != (char *) NULL)
  {
    const char
      *q;

    q=p;
    (void) GetNextToken(q,&q,MagickPathExtent,token);
    if ((*token == '#') || (*token == '\0'))
      continue;
    if (((LocaleCompare(token,"LUT_1D_SIZE") == 0) ||
         (LocaleCompare(token,"LUT_3D_SIZE") == 0)) &&
        (cube_info == (MemoryInfo *) NULL))
      {
        (void) GetNextToken(q,&q,MagickPathExtent,value);
        cube_level=(size_t) StringToLong(value);
        if (LocaleCompare(token,"LUT_1D_SIZE") == 0)
          cube_level=(size_t) ceil(pow((double) cube_level,1.0/3.0));
        if ((cube_level < 2) || (cube_level > 256))
          {
            buffer=DestroyString(buffer);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
        cube_info=AcquireVirtualMemory(cube_level*cube_level,cube_level*
          sizeof(*cube));
        if (cube_info == (MemoryInfo *) NULL)
          {
            buffer=DestroyString(buffer);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        cube=(CubePixel *) GetVirtualMemoryBlob(cube_info);
        (void) memset(cube,0,cube_level*cube_level*cube_level*sizeof(*cube));
      }
    else
      if (LocaleCompare(token,"TITLE ") == 0)
        {
          (void) GetNextToken(q,&q,MagickPathExtent,value);
          (void) SetImageProperty(image,"title",value,exception);
        }
      else
        if (cube_level != 0)
          {
            char
              *r;

            if (n >= (ssize_t) (cube_level*cube_level*cube_level))
              break;
            r=buffer;
            cube[n].r=StringToFloat(r,&r);
            cube[n].g=StringToFloat(r,&r);
            cube[n].b=StringToFloat(r,&r);
            n++;
          }
        else
          if (('+' < *buffer) && (*buffer < ':'))
            break;
  }
  buffer=DestroyString(buffer);
  if (cube_level == 0)
    {
      if (cube_info != (MemoryInfo *) NULL)
        cube_info=RelinquishVirtualMemory(cube_info);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  /*
    Convert CUBE image to HALD.
  */
  status=MagickTrue;
  hald_level=image_info->scene;
  if ((hald_level < 2) || (hald_level > 256))
    hald_level=8;
  image->columns=(size_t) (hald_level*hald_level*hald_level);
  image->rows=(size_t) (hald_level*hald_level*hald_level);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      cube_info=RelinquishVirtualMemory(cube_info);
      return(DestroyImageList(image));
    }
  for (b=0; b < (ssize_t) (hald_level*hald_level); b++)
  {
    ssize_t
      g;

    if (status == MagickFalse)
      continue;
    for (g=0; g < (ssize_t) (hald_level*hald_level); g++)
    {
      Quantum
        *magick_restrict q;

      ssize_t
        r;

      if (status == MagickFalse)
        continue;
      q=QueueAuthenticPixels(image,(g % (ssize_t) hald_level)*((ssize_t)
        hald_level*(ssize_t) hald_level),(b*(ssize_t) hald_level)+((g/(ssize_t)
        hald_level) % ((ssize_t) hald_level*(ssize_t) hald_level)),
        hald_level*hald_level,1,exception);
      if (q == (Quantum *) NULL)
        {
          status=MagickFalse;
          continue;
        }
      for (r=0; r < (ssize_t) (hald_level*hald_level); r++)
      {
        CubePixel
          index,
          next,
          offset,
          scale;

        offset.r=(float) ((MagickSafeReciprocal(((double) hald_level*hald_level)-1.0)*
          r)*(cube_level-1.0));
        index.r=floorf(offset.r);
        scale.r=offset.r-index.r;
        next.r=index.r+1;
        if ((size_t) index.r == (cube_level-1))
          next.r=index.r;
        offset.g=(float) ((MagickSafeReciprocal(((double) hald_level*hald_level)-1.0)*
          g)*(cube_level-1.0));
        index.g=floorf(offset.g);
        scale.g=offset.g-index.g;
        next.g=index.g+1;
        if ((size_t) index.g == (cube_level-1))
          next.g=index.g;
        offset.b=(float) ((MagickSafeReciprocal(((double) hald_level*hald_level)-1.0)*
          b)*(cube_level-1.0));
        index.b=floorf(offset.b);
        scale.b=offset.b-index.b;
        next.b=index.b+1;
        if ((size_t) index.b == (cube_level-1))
          next.b=index.b;
        SetPixelRed(image,ClampToQuantum(QuantumRange*(
          cube[FlattenCube(cube_level,index.b,index.g,index.r)].r+scale.r*(
          cube[FlattenCube(cube_level,index.b,index.g,next.r)].r-
          cube[FlattenCube(cube_level,index.b,index.g,index.r)].r))),q);
        SetPixelGreen(image,ClampToQuantum(QuantumRange*(
          cube[FlattenCube(cube_level,index.b,index.g,index.r)].g+scale.g*(
          cube[FlattenCube(cube_level,index.b,next.g,index.r)].g-
          cube[FlattenCube(cube_level,index.b,index.g,index.r)].g))),q);
        SetPixelBlue(image,ClampToQuantum(QuantumRange*(
          cube[FlattenCube(cube_level,index.b,index.g,index.r)].b+scale.b*(
          cube[FlattenCube(cube_level,next.b,index.g,index.r)].b-
          cube[FlattenCube(cube_level,index.b,index.g,index.r)].b))),q);
        q+=(ptrdiff_t) GetPixelChannels(image);
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        status=MagickFalse;
    }
  }
  cube_info=RelinquishVirtualMemory(cube_info);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  if (image_info->scene != 0)
    for (i=0; i < (ssize_t) image_info->scene; i++)
      AppendImageToList(&image,CloneImage(image,0,0,MagickTrue,exception));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H A L D I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCUBEImage() adds attributes for the Hald color lookup table image
%  format to the list of supported formats.  The attributes include the image
%  format tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob, whether
%  the format supports native in-memory I/O, and a brief description of the
%  format.
%
%  The format of the RegisterCUBEImage method is:
%
%      size_t RegisterCUBEImage(void)
%
*/
ModuleExport size_t RegisterCUBEImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("CUBE","CUBE","Cube LUT");
  entry->decoder=(DecodeImageHandler *) ReadCUBEImage;
  entry->flags^=CoderAdjoinFlag;
  entry->format_type=ImplicitFormatType;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H A L D I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCUBEImage() removes format registrations made by the
%  CUBE module from the list of supported formats.
%
%  The format of the UnregisterCUBEImage method is:
%
%      UnregisterCUBEImage(void)
%
*/
ModuleExport void UnregisterCUBEImage(void)
{
  (void) UnregisterMagickInfo("CUBE");
}
