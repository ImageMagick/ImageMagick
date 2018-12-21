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
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
#define MapHALD(level,b,g,r)  ((ssize_t) ((b)*(level)*(level)+(g)*(level)+(r)))

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
    *cube,
    *hald;

  Image
    *image;

  MagickBooleanType
    status;

  MemoryInfo
    *cube_info,
    *hald_info;

  register char
    *p;

  size_t
    cube_level,
    hald_level;

  ssize_t
    b,
    i,
    n;

  /*
    Create CUBE color lookup table image.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  buffer=AcquireString("");
  cube_level=0;
  cube_info=(MemoryInfo *) NULL;
  cube=(CubePixel *) NULL;
  n=0;
  *buffer='\0';
  p=buffer;
  while (ReadBlobString(image,p) != (char *) NULL)
  {
    const char
      *q;

    q=p;
    GetNextToken(q,&q,MagickPathExtent,token);
    if ((*token == '#') || (*token == '\0'))
      continue;
    GetNextToken(q,&q,MagickPathExtent,value);
    if ((LocaleCompare(token,"LUT_1D_SIZE") == 0) ||
        (LocaleCompare(token,"LUT_3D_SIZE") == 0))
      {
        cube_level=(size_t) StringToLong(value);
        if ((cube_level < 2) || (cube_level > 65536))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        if (cube_info != (MemoryInfo *) NULL)
          cube_info=RelinquishVirtualMemory(cube_info);
        cube_info=AcquireVirtualMemory(cube_level*cube_level,cube_level*
          sizeof(*cube));
        if (cube_info == (MemoryInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        cube=(CubePixel *) GetVirtualMemoryBlob(cube_info);
        (void) memset(cube,0,cube_level*cube_level*cube_level*sizeof(*cube));
      }
    else
      if (LocaleCompare(token,"TITLE ") == 0)
        (void) SetImageProperty(image,"title",value,exception);
      else
        if (cube_level != 0)
          {
            char
              *q;

            q=buffer;
            cube[n].r=StringToDouble(q,&q);
            cube[n].g=StringToDouble(q,&q);
            cube[n].b=StringToDouble(q,&q);
            n++;
          }
        else
          if (('+' < *buffer) && (*buffer < ':'))
            break;
  }
  buffer=DestroyString(buffer);
  if (cube_level == 0)
    {
      cube_info=RelinquishVirtualMemory(cube_info);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  /*
    Create HALD image, interopolate from CUBE LUT.
  */
  hald_level=image_info->scene;
  if ((hald_level < 2) || (hald_level > 256))
    hald_level=8;
  hald_info=AcquireVirtualMemory(hald_level*hald_level*hald_level,
    hald_level*hald_level*hald_level*sizeof(*hald));
  if (hald_info == (MemoryInfo *) NULL)
    {
      cube_info=RelinquishVirtualMemory(cube_info);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  hald=(CubePixel *) GetVirtualMemoryBlob(hald_info);
  (void) memset(hald,0,hald_level*hald_level*hald_level*sizeof(*hald));
  n=0;
  for (b=0; b < (ssize_t) (hald_level*hald_level); b++)
  {
    register ssize_t
      g;

    for (g=0; g < (ssize_t) (hald_level*hald_level); g++)
    {
      register ssize_t
        r;

      for (r=0; r < (ssize_t) (hald_level*hald_level); r++)
      {
        CubePixel
          index,
          next,
          offset,
          scale;

        offset.r=(PerceptibleReciprocal((double) (hald_level*hald_level)-1.0)*
          r)*(cube_level-1.0);
        index.r=floor(offset.r);
        scale.r=offset.r-index.r;
        next.r=index.r+1;
        if ((size_t) index.r == (cube_level-1))
          next.r=index.r;
        offset.g=(PerceptibleReciprocal(((double) hald_level*hald_level)-1.0)*
          g)*(cube_level-1.0);
        index.g=floor(offset.g);
        scale.g=offset.g-index.g;
        next.g=index.g+1;
        if ((size_t) index.g == (cube_level-1))
          next.g=index.g;
        offset.b=(PerceptibleReciprocal(((double) hald_level*hald_level)-1.0)*
          b)*(cube_level-1.0);
        index.b=floor(offset.b);
        scale.b=offset.b-index.b;
        next.b=index.b+1;
        if ((size_t) index.b == (cube_level-1))
          next.b=index.b;
        hald[n].r=cube[MapHALD(cube_level,index.b,index.g,index.r)].r+scale.r*(
          cube[MapHALD(cube_level,index.b,index.g,next.r)].r-
          cube[MapHALD(cube_level,index.b,index.g,index.r)].r);
        hald[n].g=cube[MapHALD(cube_level,index.b,index.g,index.r)].g+scale.g*(
          cube[MapHALD(cube_level,index.b,next.g,index.r)].g-
          cube[MapHALD(cube_level,index.b,index.g,index.r)].g);
        hald[n].b=cube[MapHALD(cube_level,index.b,index.g,index.r)].b+scale.b*(
          cube[MapHALD(cube_level,next.b,index.g,index.r)].b-
          cube[MapHALD(cube_level,index.b,index.g,index.r)].b);
        n++;
      }
    }
  }
  cube_info=RelinquishVirtualMemory(cube_info);
  /*
    Write HALD image.
  */
  status=MagickTrue;
  image->columns=(size_t) (hald_level*hald_level*hald_level);
  image->rows=(size_t) (hald_level*hald_level*hald_level);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      hald_info=RelinquishVirtualMemory(hald_info);
      return(DestroyImageList(image));
    }
  n=0;
  for (b=0; b < (ssize_t) (hald_level*hald_level); b++)
  {
    register ssize_t
      g;

    if (status == MagickFalse)
      continue;
    for (g=0; g < (ssize_t) (hald_level*hald_level); g++)
    {
      register ssize_t
        r;

      for (r=0; r < (ssize_t) (hald_level*hald_level); r++)
      {
        register Quantum
          *magick_restrict q;

        ssize_t
          x,
          y;

        x=(g % hald_level)*(hald_level*hald_level)+r;
        y=(b*hald_level)+((g/hald_level) % (hald_level*hald_level));
        q=QueueAuthenticPixels(image,x,y,1,1,exception);
        if (q == (Quantum *) NULL)
          {
            status=MagickFalse;
            continue;
          }
        SetPixelRed(image,ClampToQuantum(QuantumRange*hald[n].r),q);
        SetPixelGreen(image,ClampToQuantum(QuantumRange*hald[n].g),q);
        SetPixelBlue(image,ClampToQuantum(QuantumRange*hald[n].b),q);
        SetPixelAlpha(image,OpaqueAlpha,q);
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          status=MagickFalse;
        n++;
      }
    }
  }
  hald_info=RelinquishVirtualMemory(hald_info);
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

  entry=AcquireMagickInfo("CUBE","CUBE",
    "Cube color lookup table image");
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
