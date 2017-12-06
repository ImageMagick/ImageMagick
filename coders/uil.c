/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            U   U  IIIII  L                                  %
%                            U   U    I    L                                  %
%                            U   U    I    L                                  %
%                            U   U    I    L                                  %
%                             UUU   IIIII  LLLLL                              %
%                                                                             %
%                                                                             %
%                          Write X-Motif UIL Table.                           %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
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
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteUILImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r U I L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterUILImage() adds attributes for the UIL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterUILImage method is:
%
%      size_t RegisterUILImage(void)
%
*/
ModuleExport size_t RegisterUILImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("UIL","UIL","X-Motif UIL table");
  entry->encoder=(EncodeImageHandler *) WriteUILImage;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r U I L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterUILImage() removes format registrations made by the
%  UIL module from the list of supported formats.
%
%  The format of the UnregisterUILImage method is:
%
%      UnregisterUILImage(void)
%
*/
ModuleExport void UnregisterUILImage(void)
{
  (void) UnregisterMagickInfo("UIL");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e U I L I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure WriteUILImage() writes an image to a file in the X-Motif UIL table
%  format.
%
%  The format of the WriteUILImage method is:
%
%      MagickBooleanType WriteUILImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteUILImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
#define MaxCixels  92

  char
    basename[MagickPathExtent],
    buffer[MagickPathExtent],
    name[MagickPathExtent],
    *symbol;

  int
    j;

  MagickBooleanType
    status,
    transparent;

  MagickSizeType
    number_pixels;

  PixelInfo
    pixel;

  register const Quantum
    *p;

  register ssize_t
    i,
    x;

  size_t
    characters_per_pixel,
    colors;

  ssize_t
    k,
    y;

  static const char
    Cixel[MaxCixels+1] = " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjk"
                         "lzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  transparent=MagickFalse;
  i=0;
  p=(const Quantum *) NULL;
  if (image->storage_class == PseudoClass)
    colors=image->colors;
  else
    {
      unsigned char
        *matte_image;

      /*
        Convert DirectClass to PseudoClass image.
      */
      matte_image=(unsigned char *) NULL;
      if (image->alpha_trait != UndefinedPixelTrait)
        {
          /*
            Map all the transparent pixels.
          */
          number_pixels=(MagickSizeType) image->columns*image->rows;
          if (number_pixels != ((MagickSizeType) (size_t) number_pixels))
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          matte_image=(unsigned char *) AcquireQuantumMemory(image->columns,
            image->rows*sizeof(*matte_image));
          if (matte_image == (unsigned char *) NULL)
            ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              matte_image[i]=(unsigned char) (GetPixelAlpha(image,p) ==
                (Quantum) TransparentAlpha ? 1 : 0);
              if (matte_image[i] != 0)
                transparent=MagickTrue;
              i++;
              p+=GetPixelChannels(image);
            }
          }
        }
      (void) SetImageType(image,PaletteType,exception);
      colors=image->colors;
      if (transparent != MagickFalse)
        {
          register Quantum
            *q;

          i=0;
          colors++;
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (matte_image[i] != 0)
                SetPixelIndex(image,(Quantum) image->colors,q);
              i++;
              q+=GetPixelChannels(image);
            }
          }
        }
      if (matte_image != (unsigned char *) NULL)
        matte_image=(unsigned char *) RelinquishMagickMemory(matte_image);
    }
  /*
    Compute the character per pixel.
  */
  characters_per_pixel=1;
  for (k=MaxCixels; (ssize_t) colors > k; k*=MaxCixels)
    characters_per_pixel++;
  /*
    UIL header.
  */
  symbol=AcquireString("");
  (void) WriteBlobString(image,"/* UIL */\n");
  GetPathComponent(image->filename,BasePath,basename);
  (void) FormatLocaleString(buffer,MagickPathExtent,
    "value\n  %s_ct : color_table(\n",basename);
  (void) WriteBlobString(image,buffer);
  GetPixelInfo(image,&pixel);
  for (i=0; i < (ssize_t) colors; i++)
  {
    /*
      Define UIL color.
    */
    pixel=image->colormap[i];
    pixel.colorspace=sRGBColorspace;
    pixel.depth=8;
    pixel.alpha=(double) OpaqueAlpha;
    GetColorTuple(&pixel,MagickTrue,name);
    if (transparent != MagickFalse)
      if (i == (ssize_t) (colors-1))
        (void) CopyMagickString(name,"None",MagickPathExtent);
    /*
      Write UIL color.
    */
    k=i % MaxCixels;
    symbol[0]=Cixel[k];
    for (j=1; j < (int) characters_per_pixel; j++)
    {
      k=((i-k)/MaxCixels) % MaxCixels;
      symbol[j]=Cixel[k];
    }
    symbol[j]='\0';
    (void) SubstituteString(&symbol,"'","''");
    if (LocaleCompare(name,"None") == 0)
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "    background color = '%s'",symbol);
    else
      (void) FormatLocaleString(buffer,MagickPathExtent,
        "    color('%s',%s) = '%s'",name,
        GetPixelInfoIntensity(image,image->colormap+i) <
        (QuantumRange/2.0) ? "background" : "foreground",symbol);
    (void) WriteBlobString(image,buffer);
    (void) FormatLocaleString(buffer,MagickPathExtent,"%s",
      (i == (ssize_t) (colors-1) ? ");\n" : ",\n"));
    (void) WriteBlobString(image,buffer);
  }
  /*
    Define UIL pixels.
  */
  GetPathComponent(image->filename,BasePath,basename);
  (void) FormatLocaleString(buffer,MagickPathExtent,
    "  %s_icon : icon(color_table = %s_ct,\n",basename,basename);
  (void) WriteBlobString(image,buffer);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    (void) WriteBlobString(image,"    \"");
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      k=((ssize_t) GetPixelIndex(image,p) % MaxCixels);
      symbol[0]=Cixel[k];
      for (j=1; j < (int) characters_per_pixel; j++)
      {
        k=(((int) GetPixelIndex(image,p)-k)/MaxCixels) %
          MaxCixels;
        symbol[j]=Cixel[k];
      }
      symbol[j]='\0';
      (void) CopyMagickString(buffer,symbol,MagickPathExtent);
      (void) WriteBlobString(image,buffer);
      p+=GetPixelChannels(image);
    }
    (void) FormatLocaleString(buffer,MagickPathExtent,"\"%s\n",
      (y == (ssize_t) (image->rows-1) ? ");" : ","));
    (void) WriteBlobString(image,buffer);
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  symbol=DestroyString(symbol);
  (void) CloseBlob(image);
  return(MagickTrue);
}
