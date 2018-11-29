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
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
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
  char
    *cube_buffer,
   token[MagickPathExtent],
   value[MagickPathExtent];

  Image
    *image;

  MagickBooleanType
    status;

  register char
    *p;

  size_t
    length;

  ssize_t
    blue_rows,
    green_columns,
    red_columns;

  ssize_t
    blue;

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
  length=MagickPathExtent;
  cube_buffer=(char *) AcquireQuantumMemory((size_t) length,
    sizeof(*cube_buffer));
  if (cube_buffer == (char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  red_columns=0;
  green_columns=0;
  blue_rows=0;
  *cube_buffer='\0';
  p=cube_buffer;
  while (ReadBlobString(image,p) != (char *) NULL)
  {
    const char
      *q;

    if ((*p == '#') && ((p == cube_buffer) || (*(p-1) == '\n')))
      continue;
    q=p;
    GetNextToken(q,&q,MagickPathExtent,token);
    GetNextToken(q,&q,MagickPathExtent,value);
    if (LocaleCompare(token,"LUT_1D_SIZE") == 0)
      {
        red_columns=(ssize_t) StringToLong(value);
        if (red_columns > 65535)
          {
            cube_buffer=DestroyString(cube_buffer);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
        green_columns=1;
        blue_rows=1;
      }
    if (LocaleCompare(token,"LUT_3D_SIZE") == 0)
      {
        red_columns=(ssize_t) StringToLong(value);
        if (red_columns > 256)
          {
            cube_buffer=DestroyString(cube_buffer);
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          }
        green_columns=red_columns;
        blue_rows=red_columns;
      }
    if (LocaleCompare(token,"TITLE ") == 0)
      (void) SetImageProperty(image,"title",value,exception);
    if (('+' < *p) && (*p < ':'))
      break;
  }
  status=MagickTrue;
  image->columns=(size_t) (red_columns*green_columns);
  image->rows=(size_t) (blue_rows);
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      cube_buffer=DestroyString(cube_buffer);
      return(DestroyImageList(image));
    }
  for (blue=0; blue < blue_rows; blue++)
  {
    ssize_t
      green,
      red;

    register Quantum
      *magick_restrict q;

    if (status == MagickFalse)
      continue;
    q=QueueAuthenticPixels(image,0,blue,(size_t) red_columns*green_columns,1,
      exception);
    if (q == (Quantum *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    for (green=0; green < green_columns; green++)
    {
      for (red=0; red < red_columns; red++)
      {
        char
          *p;

        p=cube_buffer;
        SetPixelRed(image,ClampToQuantum(QuantumRange*StringToDouble(p,&p)),
          q);
        SetPixelGreen(image,ClampToQuantum(QuantumRange*StringToDouble(p,&p)),
          q);
        SetPixelBlue(image,ClampToQuantum(QuantumRange*StringToDouble(p,&p)),
          q);
        SetPixelAlpha(image,OpaqueAlpha,q);
        q+=GetPixelChannels(image);
        while (ReadBlobString(image,cube_buffer) != (char *) NULL)
          if (('+' < *cube_buffer) && (*cube_buffer < ':'))
            break;
      }
    }
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      status=MagickFalse;
  }
  cube_buffer=DestroyString(cube_buffer);
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
