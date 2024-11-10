/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            X   X  W   W  DDDD                               %
%                             X X   W   W  D   D                              %
%                              X    W   W  D   D                              %
%                             X X   W W W  D   D                              %
%                            X   X   W W   DDDD                               %
%                                                                             %
%                                                                             %
%                Read/Write X Windows System Window Dump Format               %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
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
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#if defined(MAGICKCORE_X11_DELEGATE)
#include "MagickCore/xwindow-private.h"
#if !defined(vms)
#include <X11/XWDFile.h>
#else
#include "XWDFile.h"
#endif
#endif

/*
  Forward declarations.
*/
#if defined(MAGICKCORE_X11_DELEGATE)
static MagickBooleanType
  WriteXWDImage(const ImageInfo *,Image *,ExceptionInfo *);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s X W D                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsXWD() returns MagickTrue if the image format type, identified by the
%  magick string, is XWD.
%
%  The format of the IsXWD method is:
%
%      MagickBooleanType IsXWD(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsXWD(const unsigned char *magick,const size_t length)
{
  if (length < 8)
    return(MagickFalse);
  if (memcmp(magick+1,"\000\000",2) == 0)
    {
      if (memcmp(magick+4,"\007\000\000",3) == 0)
        return(MagickTrue);
      if (memcmp(magick+5,"\000\000\007",3) == 0)
        return(MagickTrue);
    }
  return(MagickFalse);
}

#if defined(MAGICKCORE_X11_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X W D I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadXWDImage() reads an X Window System window dump image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadXWDImage method is:
%
%      Image *ReadXWDImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static Image *ReadXWDImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define CheckOverflowException(length,width,height) \
  (((height) != 0) && ((length)/((size_t) height) != ((size_t) width)))

  char
    *comment;

  Image
    *image;

  int
    x_status;

  MagickBooleanType
    authentic_colormap;

  MagickStatusType
    status;

  Quantum
    index;

  ssize_t
    x;

  Quantum
    *q;

  ssize_t
    i;

  size_t
    pixel;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned long
    lsb_first;

  XColor
    *colors;

  XImage
    *ximage;

  XWDFileHeader
    header;

  /*
    Open image file.
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
  /*
     Read in header information.
  */
  count=ReadBlob(image,sz_XWDheader,(unsigned char *) &header);
  if (count != sz_XWDheader)
    ThrowReaderException(CorruptImageError,"UnableToReadImageHeader");
  /*
    Ensure the header byte-order is most-significant byte first.
  */
  lsb_first=1;
  if ((int) (*(char *) &lsb_first) != 0)
    MSBOrderLong((unsigned char *) &header,sz_XWDheader);
  /*
    Check to see if the dump file is in the proper format.
  */
  if (header.file_version != XWD_FILE_VERSION)
    ThrowReaderException(CorruptImageError,"FileFormatVersionMismatch");
  if (header.header_size < sz_XWDheader)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (header.xoffset >= header.pixmap_width)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  switch (header.visual_class)
  {
    case StaticGray:
    case GrayScale:
    {
      if (header.bits_per_pixel != 1)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      break;
    }
    case StaticColor:
    case PseudoColor:
    {
      if ((header.bits_per_pixel < 1) || (header.bits_per_pixel > 15) ||
          (header.colormap_entries == 0))
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      break;
    }
    case TrueColor:
    case DirectColor:
    {
      if ((header.bits_per_pixel != 16) && (header.bits_per_pixel != 24) &&
          (header.bits_per_pixel != 32))
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      break;
    }
    default:
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  switch (header.pixmap_format)
  {
    case XYBitmap:
    {
      if (header.pixmap_depth != 1)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      break;
    }
    case XYPixmap:
    case ZPixmap:
    {
      if ((header.pixmap_depth < 1) || (header.pixmap_depth > 32))
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      switch (header.bitmap_pad)
      {
        case 8:
        case 16:
        case 32:
          break;
        default:
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
      break;
    }
    default:
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  switch (header.bitmap_unit)
  {
    case 8:
    case 16:
    case 32:
      break;
    default:
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  switch (header.byte_order)
  {
    case LSBFirst:
    case MSBFirst:
      break;
    default:
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  switch (header.bitmap_bit_order)
  {
    case LSBFirst:
    case MSBFirst:
      break;
    default:
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  }
  if (((header.bitmap_pad % 8) != 0) || (header.bitmap_pad > 32))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  length=(size_t) (header.header_size-sz_XWDheader);
  comment=(char *) AcquireQuantumMemory(length+1,sizeof(*comment));
  if (comment == (char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  count=ReadBlob(image,length,(unsigned char *) comment);
  comment[length]='\0';
  (void) SetImageProperty(image,"comment",comment,exception);
  comment=DestroyString(comment);
  if (count != (ssize_t) length)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  /*
    Initialize the X image.
  */
  ximage=(XImage *) AcquireMagickMemory(sizeof(*ximage));
  if (ximage == (XImage *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  ximage->depth=(int) header.pixmap_depth;
  ximage->format=(int) header.pixmap_format;
  ximage->xoffset=(int) header.xoffset;
  ximage->data=(char *) NULL;
  ximage->width=(int) header.pixmap_width;
  ximage->height=(int) header.pixmap_height;
  ximage->bitmap_pad=(int) header.bitmap_pad;
  ximage->bytes_per_line=(int) header.bytes_per_line;
  ximage->byte_order=(int) header.byte_order;
  ximage->bitmap_unit=(int) header.bitmap_unit;
  ximage->bitmap_bit_order=(int) header.bitmap_bit_order;
  ximage->bits_per_pixel=(int) header.bits_per_pixel;
  ximage->red_mask=header.red_mask;
  ximage->green_mask=header.green_mask;
  ximage->blue_mask=header.blue_mask;
  if ((ximage->depth < 0) || (ximage->format < 0) || (ximage->xoffset < 0) ||
      (ximage->width < 0) || (ximage->height < 0) || (ximage->bitmap_pad < 0) ||
      (ximage->bytes_per_line < 0) || (ximage->byte_order < 0) ||
      (ximage->bitmap_unit < 0) || (ximage->bitmap_bit_order < 0) ||
      (ximage->bits_per_pixel < 0))
    {
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  if ((ximage->width > 65535) || (ximage->height > 65535))
    {
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  if ((ximage->bits_per_pixel > 32) || (ximage->bitmap_unit > 32))
    {
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  x_status=XInitImage(ximage);
  if (x_status == 0)
    {
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    }
  /*
    Read colormap.
  */
  authentic_colormap=MagickFalse;
  colors=(XColor *) NULL;
  if (header.ncolors != 0)
    {
      XWDColor
        color;

      length=(size_t) header.ncolors;
      if (length > ((~0UL)/sizeof(*colors)))
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      colors=(XColor *) AcquireQuantumMemory(length,sizeof(*colors));
      if (colors == (XColor *) NULL)
        {
          ximage=(XImage *) RelinquishMagickMemory(ximage);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      for (i=0; i < (ssize_t) header.ncolors; i++)
      {
        count=ReadBlob(image,sz_XWDColor,(unsigned char *) &color);
        if (count != sz_XWDColor)
          {
            colors=(XColor *) RelinquishMagickMemory(colors);
            ximage=(XImage *) RelinquishMagickMemory(ximage);
            ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
          }
        colors[i].pixel=color.pixel;
        colors[i].red=color.red;
        colors[i].green=color.green;
        colors[i].blue=color.blue;
        colors[i].flags=(char) color.flags;
        if (color.flags != 0)
          authentic_colormap=MagickTrue;
      }
      /*
        Ensure the header byte-order is most-significant byte first.
      */
      lsb_first=1;
      if ((int) (*(char *) &lsb_first) != 0)
        for (i=0; i < (ssize_t) header.ncolors; i++)
        {
          MSBOrderLong((unsigned char *) &colors[i].pixel,
            sizeof(colors[i].pixel));
          MSBOrderShort((unsigned char *) &colors[i].red,3*
            sizeof(colors[i].red));
        }
    }
  /*
    Allocate the pixel buffer.
  */
  length=(size_t) (ximage->bytes_per_line*ximage->height);
  if (CheckOverflowException(length,ximage->bytes_per_line,ximage->height))
    {
      if (header.ncolors != 0)
        colors=(XColor *) RelinquishMagickMemory(colors);
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  if (ximage->format != ZPixmap)
    {
      size_t
        extent;

      extent=length;
      length*=(size_t) ximage->depth;
      if (CheckOverflowException(length,extent,ximage->depth))
        {
          if (header.ncolors != 0)
            colors=(XColor *) RelinquishMagickMemory(colors);
          ximage=(XImage *) RelinquishMagickMemory(ximage);
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        }
    }
  ximage->data=(char *) AcquireQuantumMemory(length,sizeof(*ximage->data));
  if (ximage->data == (char *) NULL)
    {
      if (header.ncolors != 0)
        colors=(XColor *) RelinquishMagickMemory(colors);
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  count=ReadBlob(image,length,(unsigned char *) ximage->data);
  if (count != (ssize_t) length)
    {
      if (header.ncolors != 0)
        colors=(XColor *) RelinquishMagickMemory(colors);
      ximage->data=DestroyString(ximage->data);
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    }
  /*
    Convert image to MIFF format.
  */
  image->columns=(size_t) ximage->width;
  image->rows=(size_t) ximage->height;
  image->depth=8;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      if (header.ncolors != 0)
        colors=(XColor *) RelinquishMagickMemory(colors);
      ximage->data=DestroyString(ximage->data);
      ximage=(XImage *) RelinquishMagickMemory(ximage);
      return(DestroyImageList(image));
    }
  if ((header.ncolors == 0U) || (ximage->red_mask != 0) ||
      (ximage->green_mask != 0) || (ximage->blue_mask != 0))
    image->storage_class=DirectClass;
  else
    image->storage_class=PseudoClass;
  image->colors=header.ncolors;
  if (image_info->ping == MagickFalse)
    switch (image->storage_class)
    {
      case DirectClass:
      default:
      {
        size_t
          color;

        size_t
          blue_mask,
          blue_shift,
          green_mask,
          green_shift,
          red_mask,
          red_shift;

        /*
          Determine shift and mask for red, green, and blue.
        */
        red_mask=ximage->red_mask;
        red_shift=0;
        while ((red_mask != 0) && ((red_mask & 0x01) == 0))
        {
          red_mask>>=1;
          red_shift++;
        }
        green_mask=ximage->green_mask;
        green_shift=0;
        while ((green_mask != 0) && ((green_mask & 0x01) == 0))
        {
          green_mask>>=1;
          green_shift++;
        }
        blue_mask=ximage->blue_mask;
        blue_shift=0;
        while ((blue_mask != 0) && ((blue_mask & 0x01) == 0))
        {
          blue_mask>>=1;
          blue_shift++;
        }
        /*
          Convert X image to DirectClass packets.
        */
        if ((image->colors != 0) && (authentic_colormap != MagickFalse))
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              pixel=XGetPixel(ximage,(int) x,(int) y);
              index=(Quantum) ConstrainColormapIndex(image,(ssize_t) ((pixel >>
                red_shift) & red_mask),exception);
              SetPixelRed(image,ScaleShortToQuantum(
                colors[(ssize_t) index].red),q);
              index=(Quantum) ConstrainColormapIndex(image,(ssize_t) ((pixel >>
                green_shift) & green_mask),exception);
              SetPixelGreen(image,ScaleShortToQuantum(
                colors[(ssize_t) index].green),q);
              index=(Quantum) ConstrainColormapIndex(image,(ssize_t) ((pixel >>
                blue_shift) & blue_mask),exception);
              SetPixelBlue(image,ScaleShortToQuantum(
                colors[(ssize_t) index].blue),q);
              q+=(ptrdiff_t) GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
              image->rows);
            if (status == MagickFalse)
              break;
          }
        else
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              pixel=XGetPixel(ximage,(int) x,(int) y);
              color=(pixel >> red_shift) & red_mask;
              if (red_mask != 0)
                color=(color*65535UL)/red_mask;
              SetPixelRed(image,ScaleShortToQuantum((unsigned short) color),q);
              color=(pixel >> green_shift) & green_mask;
              if (green_mask != 0)
                color=(color*65535UL)/green_mask;
              SetPixelGreen(image,ScaleShortToQuantum((unsigned short) color),
                q);
              color=(pixel >> blue_shift) & blue_mask;
              if (blue_mask != 0)
                color=(color*65535UL)/blue_mask;
              SetPixelBlue(image,ScaleShortToQuantum((unsigned short) color),q);
              q+=(ptrdiff_t) GetPixelChannels(image);
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
              image->rows);
            if (status == MagickFalse)
              break;
          }
        break;
      }
      case PseudoClass:
      {
        /*
          Convert X image to PseudoClass packets.
        */
        if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
          {
            if (header.ncolors != 0)
              colors=(XColor *) RelinquishMagickMemory(colors);
            ximage->data=DestroyString(ximage->data);
            ximage=(XImage *) RelinquishMagickMemory(ximage);
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          }
        for (i=0; i < (ssize_t) image->colors; i++)
        {
          image->colormap[i].red=(MagickRealType) ScaleShortToQuantum(
            colors[i].red);
          image->colormap[i].green=(MagickRealType) ScaleShortToQuantum(
            colors[i].green);
          image->colormap[i].blue=(MagickRealType) ScaleShortToQuantum(
            colors[i].blue);
        }
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
            break;
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            index=(Quantum) ConstrainColormapIndex(image,(ssize_t)
              XGetPixel(ximage,(int) x,(int) y),exception);
            SetPixelIndex(image,index,q);
            SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=(ptrdiff_t) GetPixelChannels(image);
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
        break;
      }
    }
  /*
    Free image and colormap.
  */
  if (header.ncolors != 0)
    colors=(XColor *) RelinquishMagickMemory(colors);
  ximage->data=DestroyString(ximage->data);
  ximage=(XImage *) RelinquishMagickMemory(ximage);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r X W D I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterXWDImage() adds properties for the XWD image format to
%  the list of supported formats.  The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterXWDImage method is:
%
%      size_t RegisterXWDImage(void)
%
*/
ModuleExport size_t RegisterXWDImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("XWD","XWD","X Windows system window dump (color)");
#if defined(MAGICKCORE_X11_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadXWDImage;
  entry->encoder=(EncodeImageHandler *) WriteXWDImage;
#endif
  entry->magick=(IsImageFormatHandler *) IsXWD;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r X W D I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterXWDImage() removes format registrations made by the
%  XWD module from the list of supported formats.
%
%  The format of the UnregisterXWDImage method is:
%
%      UnregisterXWDImage(void)
%
*/
ModuleExport void UnregisterXWDImage(void)
{
  (void) UnregisterMagickInfo("XWD");
}

#if defined(MAGICKCORE_X11_DELEGATE)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X W D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteXWDImage() writes an image to a file in X window dump
%  rasterfile format.
%
%  The format of the WriteXWDImage method is:
%
%      MagickBooleanType WriteXWDImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteXWDImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  const char
    *value;

  MagickBooleanType
    status;

  const Quantum
    *p;

  ssize_t
    x;

  unsigned char
    *q;

  size_t
    bits_per_pixel,
    bytes_per_line,
    length,
    scanline_pad;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  unsigned long
    lsb_first;

  XWDFileHeader
    xwd_info;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if ((image->columns != (CARD32) image->columns) ||
      (image->rows != (CARD32) image->rows))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  if ((image->storage_class == PseudoClass) && (image->colors > 256))
    (void) SetImageType(image,TrueColorType,exception);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  /*
    Initialize XWD file header.
  */
  (void) memset(&xwd_info,0,sizeof(xwd_info));
  xwd_info.header_size=(CARD32) sz_XWDheader;
  value=GetImageProperty(image,"comment",exception);
  if (value != (const char *) NULL)
    xwd_info.header_size+=(CARD32) strlen(value);
  xwd_info.header_size++;
  xwd_info.file_version=(CARD32) XWD_FILE_VERSION;
  xwd_info.pixmap_format=(CARD32) ZPixmap;
  xwd_info.pixmap_depth=(CARD32) (image->storage_class == DirectClass ? 24 : 8);
  xwd_info.pixmap_width=(CARD32) image->columns;
  xwd_info.pixmap_height=(CARD32) image->rows;
  xwd_info.xoffset=(CARD32) 0;
  xwd_info.byte_order=(CARD32) MSBFirst;
  xwd_info.bitmap_unit=(CARD32) (image->storage_class == DirectClass ? 32 : 8);
  xwd_info.bitmap_bit_order=(CARD32) MSBFirst;
  xwd_info.bitmap_pad=(CARD32) (image->storage_class == DirectClass ? 32 : 8);
  bits_per_pixel=(size_t) (image->storage_class == DirectClass ? 24 : 8);
  xwd_info.bits_per_pixel=(CARD32) bits_per_pixel;
  bytes_per_line=(CARD32) ((((xwd_info.bits_per_pixel*
    xwd_info.pixmap_width)+((xwd_info.bitmap_pad)-1))/
    (xwd_info.bitmap_pad))*((xwd_info.bitmap_pad) >> 3));
  xwd_info.bytes_per_line=(CARD32) bytes_per_line;
  xwd_info.visual_class=(CARD32)
    (image->storage_class == DirectClass ? DirectColor : PseudoColor);
  xwd_info.red_mask=(CARD32)
    (image->storage_class == DirectClass ? 0xff0000 : 0);
  xwd_info.green_mask=(CARD32)
    (image->storage_class == DirectClass ? 0xff00 : 0);
  xwd_info.blue_mask=(CARD32) (image->storage_class == DirectClass ? 0xff : 0);
  xwd_info.bits_per_rgb=(CARD32) (image->storage_class == DirectClass ? 24 : 8);
  xwd_info.colormap_entries=(CARD32)
    (image->storage_class == DirectClass ? 256 : image->colors);
  xwd_info.ncolors=(unsigned int)
    (image->storage_class == DirectClass ? 0 : image->colors);
  xwd_info.window_width=(CARD32) image->columns;
  xwd_info.window_height=(CARD32) image->rows;
  xwd_info.window_x=0;
  xwd_info.window_y=0;
  xwd_info.window_bdrwidth=(CARD32) 0;
  /*
    Write XWD header.
  */
  lsb_first=1;
  if ((int) (*(char *) &lsb_first) != 0)
    MSBOrderLong((unsigned char *) &xwd_info,sizeof(xwd_info));
  (void) WriteBlob(image,sz_XWDheader,(unsigned char *) &xwd_info);
  if (value != (const char *) NULL)
    (void) WriteBlob(image,strlen(value),(unsigned char *) value);
  (void) WriteBlob(image,1,(const unsigned char *) "\0");
  if (image->storage_class == PseudoClass)
    {
      ssize_t
        i;

      XColor
        *colors;

      XWDColor
        color;

      /*
        Dump colormap to file.
      */
      (void) memset(&color,0,sizeof(color));
      colors=(XColor *) AcquireQuantumMemory((size_t) image->colors,
        sizeof(*colors));
      if (colors == (XColor *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        colors[i].pixel=(unsigned long) i;
        colors[i].red=ScaleQuantumToShort(ClampToQuantum(
          image->colormap[i].red));
        colors[i].green=ScaleQuantumToShort(ClampToQuantum(
          image->colormap[i].green));
        colors[i].blue=ScaleQuantumToShort(ClampToQuantum(
          image->colormap[i].blue));
        colors[i].flags=(char) (DoRed | DoGreen | DoBlue);
        colors[i].pad='\0';
        if ((int) (*(char *) &lsb_first) != 0)
          {
            MSBOrderLong((unsigned char *) &colors[i].pixel,
              sizeof(colors[i].pixel));
            MSBOrderShort((unsigned char *) &colors[i].red,3*
              sizeof(colors[i].red));
          }
      }
      for (i=0; i < (ssize_t) image->colors; i++)
      {
        color.pixel=(CARD32) colors[i].pixel;
        color.red=colors[i].red;
        color.green=colors[i].green;
        color.blue=colors[i].blue;
        color.flags=(CARD8) colors[i].flags;
        count=WriteBlob(image,sz_XWDColor,(unsigned char *) &color);
        if (count != (ssize_t) sz_XWDColor)
          break;
      }
      colors=(XColor *) RelinquishMagickMemory(colors);
    }
  /*
    Allocate memory for pixels.
  */
  length=3*bytes_per_line;
  if (image->storage_class == PseudoClass)
    length=bytes_per_line;
  pixels=(unsigned char *) AcquireQuantumMemory(length,sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  (void) memset(pixels,0,length);
  /*
    Convert MIFF to XWD raster pixels.
  */
  scanline_pad=(bytes_per_line-((image->columns*bits_per_pixel) >> 3));
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    q=pixels;
    if (image->storage_class == PseudoClass)
      {
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          *q++=(unsigned char) ((ssize_t) GetPixelIndex(image,p));
          p+=(ptrdiff_t) GetPixelChannels(image);
        }
      }
    else
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        *q++=ScaleQuantumToChar(GetPixelRed(image,p));
        *q++=ScaleQuantumToChar(GetPixelGreen(image,p));
        *q++=ScaleQuantumToChar(GetPixelBlue(image,p));
        p+=(ptrdiff_t) GetPixelChannels(image);
      }
    for (x=0; x < (ssize_t) scanline_pad; x++)
      *q++='\0';
    length=(size_t) (q-pixels);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
      break;
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  (void) CloseBlob(image);
  return(y < (ssize_t) image->rows ? MagickFalse :  MagickTrue);
}
#endif
