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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image-private.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteUILImage(const ImageInfo *,Image *);

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
%      unsigned long RegisterUILImage(void)
%
*/
ModuleExport unsigned long RegisterUILImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("UIL");
  entry->encoder=(EncodeImageHandler *) WriteUILImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("X-Motif UIL table");
  entry->module=ConstantString("UIL");
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
%      MagickBooleanType WriteUILImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/
static MagickBooleanType WriteUILImage(const ImageInfo *image_info,Image *image)
{
#define MaxCixels  92

  char
    basename[MaxTextExtent],
    buffer[MaxTextExtent],
    name[MaxTextExtent],
    symbol[MaxTextExtent];

  ExceptionInfo
    *exception;

  int
    j;

  long
    k,
    y;

  MagickBooleanType
    status,
    transparent;

  MagickPixelPacket
    pixel;

  MagickSizeType
    number_pixels;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  static const char
    Cixel[MaxCixels+1] = " .XoO+@#$%&*=-;:>,<1234567890qwertyuipasdfghjk"
                         "lzxcvbnmMNBVCZASDFGHJKLPIUYTREWQ!~^/()_`'][{}|";

  unsigned long
    characters_per_pixel,
    colors;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(status);
  if (image->colorspace != RGBColorspace)
    (void) TransformImageColorspace(image,RGBColorspace);
  exception=(&image->exception);
  transparent=MagickFalse;
  i=0;
  p=(const PixelPacket *) NULL;
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
      if (image->matte != MagickFalse)
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
          for (y=0; y < (long) image->rows; y++)
          {
            p=GetVirtualPixels(image,0,y,image->columns,1,exception);
            if (p == (const PixelPacket *) NULL)
              break;
            for (x=0; x < (long) image->columns; x++)
            {
              matte_image[i]=(unsigned char)
              (p->opacity == (Quantum) TransparentOpacity ? 1 : 0);
              if (matte_image[i] != 0)
                transparent=MagickTrue;
              i++;
              p++;
            }
          }
        }
      (void) SetImageType(image,PaletteType);
      colors=image->colors;
      if (transparent != MagickFalse)
        {
          register IndexPacket
            *indexes;

          register PixelPacket
            *q;

          colors++;
          for (y=0; y < (long) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            indexes=GetAuthenticIndexQueue(image);
            for (x=0; x < (long) image->columns; x++)
            {
              if (matte_image[i] != 0)
                indexes[x]=(IndexPacket) image->colors;
              p++;
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
  for (k=MaxCixels; (long) colors > k; k*=MaxCixels)
    characters_per_pixel++;
  /*
    UIL header.
  */
  (void) WriteBlobString(image,"/* UIL */\n");
  GetPathComponent(image->filename,BasePath,basename);
  (void) FormatMagickString(buffer,MaxTextExtent,
    "value\n  %s_ct : color_table(\n",basename);
  (void) WriteBlobString(image,buffer);
  GetMagickPixelPacket(image,&pixel);
  for (i=0; i < (long) colors; i++)
  {
    /*
      Define UIL color.
    */
    SetMagickPixelPacket(image,image->colormap+i,(IndexPacket *) NULL,&pixel);
    pixel.colorspace=RGBColorspace;
    pixel.depth=8;
    pixel.opacity=(MagickRealType) OpaqueOpacity;
    GetColorTuple(&pixel,MagickTrue,name);
    if (transparent != MagickFalse)
      if (i == (long) (colors-1))
        (void) CopyMagickString(name,"None",MaxTextExtent);
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
    if (LocaleCompare(name,"None") == 0)
      (void) FormatMagickString(buffer,MaxTextExtent,
        "    background color = '%s'",symbol);
    else
      (void) FormatMagickString(buffer,MaxTextExtent,
        "    color('%s',%s) = '%s'",name,
        PixelIntensityToQuantum(image->colormap+i) <
        ((Quantum) QuantumRange/2) ? "background" : "foreground",symbol);
    (void) WriteBlobString(image,buffer);
    (void) FormatMagickString(buffer,MaxTextExtent,"%s",
      (i == (long) (colors-1) ? ");\n" : ",\n"));
    (void) WriteBlobString(image,buffer);
  }
  /*
    Define UIL pixels.
  */
  GetPathComponent(image->filename,BasePath,basename);
  (void) FormatMagickString(buffer,MaxTextExtent,
    "  %s_icon : icon(color_table = %s_ct,\n",basename,basename);
  (void) WriteBlobString(image,buffer);
  for (y=0; y < (long) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,&image->exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetVirtualIndexQueue(image);
    (void) WriteBlobString(image,"    \"");
    for (x=0; x < (long) image->columns; x++)
    {
      k=((long) indexes[x] % MaxCixels);
      symbol[0]=Cixel[k];
      for (j=1; j < (int) characters_per_pixel; j++)
      {
        k=(((int) indexes[x]-k)/MaxCixels) % MaxCixels;
        symbol[j]=Cixel[k];
      }
      symbol[j]='\0';
      (void) CopyMagickString(buffer,symbol,MaxTextExtent);
      (void) WriteBlobString(image,buffer);
      p++;
    }
    (void) FormatMagickString(buffer,MaxTextExtent,"\"%s\n",
      (y == (long) (image->rows-1) ? ");" : ","));
    (void) WriteBlobString(image,buffer);
    status=SetImageProgress(image,SaveImageTag,y,image->rows);
    if (status == MagickFalse)
      break;
  }
  (void) CloseBlob(image);
  return(MagickTrue);
}
