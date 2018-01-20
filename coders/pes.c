/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   EEEEE  SSSSS                              %
%                            P   P  E      SS                                 %
%                            PPPP   EEE     SSS                               %
%                            P      E         SS                              %
%                            P      EEEEE  SSSSS                              %
%                                                                             %
%                                                                             %
%                     Read/Write Brother PES Image Format                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 2009                                   %
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
%  The PES format was derived from Robert Heel's PHP script (see
%  http://bobosch.dyndns.org/embroidery/showFile.php?pes.php) and pesconvert
%  (see http://torvalds-family.blogspot.com/2010/01/embroidery-gaah.html).
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/property.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/client.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/decorate.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/gem.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/montage.h"
#include "MagickCore/resize.h"
#include "MagickCore/shear.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/resource_.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"

/*
  Typedef declarations.
*/
typedef struct _PESColorInfo
{
  const unsigned char
    red,
    green,
    blue,
    alpha;
} PESColorInfo;

typedef struct _PESBlockInfo
{
  const PESColorInfo
    *color;

  ssize_t
    offset;
} PESBlockInfo;

/*
  PES Colors.
*/
static const PESColorInfo
  PESColor[256] =
  {
    {   0,   0,   0, 1 },
    {  14,  31, 124, 1 },
    {  10,  85, 163, 1 },
    {  48, 135, 119, 1 },
    {  75, 107, 175, 1 },
    { 237,  23,  31, 1 },
    { 209,  92,   0, 1 },
    { 145,  54, 151, 1 },
    { 228, 154, 203, 1 },
    { 145,  95, 172, 1 },
    { 157, 214, 125, 1 },
    { 232, 169,   0, 1 },
    { 254, 186,  53, 1 },
    { 255, 255,   0, 1 },
    { 112, 188,  31, 1 },
    { 192, 148,   0, 1 },
    { 168, 168, 168, 1 },
    { 123, 111,   0, 1 },
    { 255, 255, 179, 1 },
    {  79,  85,  86, 1 },
    {   0,   0,   0, 1 },
    {  11,  61, 145, 1 },
    { 119,   1, 118, 1 },
    {  41,  49,  51, 1 },
    {  42,  19,   1, 1 },
    { 246,  74, 138, 1 },
    { 178, 118,  36, 1 },
    { 252, 187, 196, 1 },
    { 254,  55,  15, 1 },
    { 240, 240, 240, 1 },
    { 106,  28, 138, 1 },
    { 168, 221, 196, 1 },
    {  37, 132, 187, 1 },
    { 254, 179,  67, 1 },
    { 255, 240, 141, 1 },
    { 208, 166,  96, 1 },
    { 209,  84,   0, 1 },
    { 102, 186,  73, 1 },
    {  19,  74,  70, 1 },
    { 135, 135, 135, 1 },
    { 216, 202, 198, 1 },
    {  67,  86,   7, 1 },
    { 254, 227, 197, 1 },
    { 249, 147, 188, 1 },
    {   0,  56,  34, 1 },
    { 178, 175, 212, 1 },
    { 104, 106, 176, 1 },
    { 239, 227, 185, 1 },
    { 247,  56, 102, 1 },
    { 181,  76, 100, 1 },
    {  19,  43,  26, 1 },
    { 199,   1,  85, 1 },
    { 254, 158,  50, 1 },
    { 168, 222, 235, 1 },
    {   0, 103,  26, 1 },
    {  78,  41, 144, 1 },
    {  47, 126,  32, 1 },
    { 253, 217, 222, 1 },
    { 255, 217,  17, 1 },
    {   9,  91, 166, 1 },
    { 240, 249, 112, 1 },
    { 227, 243,  91, 1 },
    { 255, 200, 100, 1 },
    { 255, 200, 150, 1 },
    { 255, 200, 200, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 },
    {   0,   0,   0, 1 }
  };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P E S                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsPES() returns MagickTrue if the image format type, identified by the
%  magick string, is PES.
%
%  The format of the IsPES method is:
%
%      MagickBooleanType IsPES(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsPES(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((const char *) magick,"#PES",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P E S I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadPESImage() reads a Brother PES image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadPESImage method is:
%
%      image=ReadPESImage(image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadPESImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    filename[MagickPathExtent];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *read_info;

  int
    delta_x,
    delta_y,
    j,
    unique_file,
    x,
    y;

  MagickBooleanType
    status;

  PESBlockInfo
    blocks[256];

  PointInfo
    *stitches;

  SegmentInfo
    bounds;

  register ssize_t
    i;

  size_t
    number_blocks,
    number_colors,
    number_stitches;

  ssize_t
    count,
    offset;

  unsigned char
    magick[4],
    version[4];

  /*
    Open image file.
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
  /*
    Verify PES identifier.
  */
  count=ReadBlob(image,4,magick);
  if ((count != 4) || (LocaleNCompare((char *) magick,"#PES",4) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,4,version);
  offset=ReadBlobLSBSignedLong(image);
  if (DiscardBlobBytes(image,offset+36) == MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  /*
    Get PES colors.
  */
  number_colors=(size_t) ReadBlobByte(image)+1;
  for (i=0; i < (ssize_t) number_colors; i++)
  {
    j=ReadBlobByte(image);
    blocks[i].color=PESColor+(j < 0 ? 0 : j);
    blocks[i].offset=0;
  }
  for ( ; i < 256L; i++)
  {
    blocks[i].offset=0;
    blocks[i].color=PESColor;
  }
  if (DiscardBlobBytes(image,532L-number_colors-21) == MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  /*
    Stitch away.
  */
  number_stitches=64;
  stitches=(PointInfo *) AcquireQuantumMemory(number_stitches,
    sizeof(*stitches));
  if (stitches == (PointInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  bounds.x1=65535.0;
  bounds.y1=65535.0;
  bounds.x2=(-65535.0);
  bounds.y2=(-65535.0);
  i=0;
  j=0;
  delta_x=0;
  delta_y=0;
  while (EOFBlob(image) == MagickFalse)
  {
    x=ReadBlobByte(image);
    y=ReadBlobByte(image);
    if ((x == 0xff) && (y == 0))
      break;
    if ((x == 254) && (y == 176))
      {
        /*
          Start a new stitch block.
        */
        j++;
        blocks[j].offset=(ssize_t) i;
        if (j >= 255)
          {
            stitches=(PointInfo *) RelinquishMagickMemory(stitches);
            ThrowReaderException(ResourceLimitError,"CorruptImage");
          }
        (void) ReadBlobByte(image);
        continue;
      }
    if ((x & 0x80) == 0)
      {
        /*
          Normal stitch.
        */
        if ((x & 0x40) != 0)
          x-=0x80;
      }
    else
      {
        /*
          Jump stitch.
        */
        x=((x & 0x0f) << 8)+y;
        if ((x & 0x800) != 0)
          x-=0x1000;
        y=ReadBlobByte(image);
      }
    if ((y & 0x80) == 0)
      {
        /*
          Normal stitch.
        */
        if ((y & 0x40) != 0)
          y-=0x80;
      }
    else
      {
        /*
          Jump stitch.
        */
        y=((y & 0x0f) << 8)+ReadBlobByte(image);
        if ((y & 0x800) != 0)
          y-=0x1000;
      }
    /*
      Note stitch (x,y).
    */
    x+=delta_x;
    y+=delta_y;
    delta_x=x;
    delta_y=y;
    stitches[i].x=(double) x;
    stitches[i].y=(double) y;
    if ((double) x < bounds.x1)
      bounds.x1=(double) x;
    if ((double) x > bounds.x2)
      bounds.x2=(double) x;
    if ((double) y < bounds.y1)
      bounds.y1=(double) y;
    if ((double) y > bounds.y2)
      bounds.y2=(double) y;
    i++;
    if (i >= (ssize_t) number_stitches)
      {
        /*
          Make room for more stitches.
        */
        number_stitches<<=1;
        stitches=(PointInfo *)  ResizeQuantumMemory(stitches,(size_t)
          number_stitches,sizeof(*stitches));
        if (stitches == (PointInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
     }
  }
  j++;
  blocks[j].offset=(ssize_t) i;
  number_blocks=(size_t) j;
  image->columns=bounds.x2-bounds.x1;
  image->rows=bounds.y2-bounds.y1;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      stitches=(PointInfo *) RelinquishMagickMemory(stitches);
      return(DestroyImageList(image));
    }
  /*
    Write stitches as SVG file.
  */
  file=(FILE *) NULL;
  unique_file=AcquireUniqueFileResource(filename);
  if (unique_file != -1)
    file=fdopen(unique_file,"wb");
  if ((unique_file == -1) || (file == (FILE *) NULL))
    ThrowImageException(FileOpenError,"UnableToCreateTemporaryFile");
  (void) FormatLocaleFile(file,"<?xml version=\"1.0\"?>\n");
  (void) FormatLocaleFile(file,"<svg xmlns=\"http://www.w3.org/2000/svg\" "
    "xlink=\"http://www.w3.org/1999/xlink\" "
    "ev=\"http://www.w3.org/2001/xml-events\" version=\"1.1\" "
    "baseProfile=\"full\" width=\"%g\" height=\"%g\">\n",(double)
    image->columns,(double) image->rows);
  for (i=0; i < (ssize_t) number_blocks; i++)
  {
    offset=blocks[i].offset;
    (void) FormatLocaleFile(file,"  <path stroke=\"#%02x%02x%02x\" "
      "fill=\"none\" d=\"M %g %g",blocks[i].color->red,blocks[i].color->green,
      blocks[i].color->blue,stitches[offset].x-bounds.x1,
      stitches[offset].y-bounds.y1);
    for (j=1; j < (ssize_t) (blocks[i+1].offset-offset); j++)
      (void) FormatLocaleFile(file," L %g %g",stitches[offset+j].x-bounds.x1,
        stitches[offset+j].y-bounds.y1);
    (void) FormatLocaleFile(file,"\"/>\n");
  }
  (void) FormatLocaleFile(file,"</svg>\n");
  (void) fclose(file);
  stitches=(PointInfo *) RelinquishMagickMemory(stitches);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  /*
    Read SVG file.
  */
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"svg:%s",
    filename);
  image=ReadImage(read_info,exception);
  if (image != (Image *) NULL)
    {
      (void) CopyMagickString(image->filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick_filename,image_info->filename,
        MagickPathExtent);
      (void) CopyMagickString(image->magick,"PES",MagickPathExtent);
    }
  read_info=DestroyImageInfo(read_info);
  (void) RelinquishUniqueFileResource(filename);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r P E S I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterPESImage() adds attributes for the PES image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterPESImage method is:
%
%      size_t RegisterPESImage(void)
%
*/
ModuleExport size_t RegisterPESImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("PES","PES","Embrid Embroidery Format");
  entry->decoder=(DecodeImageHandler *) ReadPESImage;
  entry->magick=(IsImageFormatHandler *) IsPES;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r P E S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterPESImage() removes format registrations made by the
%  PES module from the list of supported formats.
%
%  The format of the UnregisterPESImage method is:
%
%      UnregisterPESImage(void)
%
*/
ModuleExport void UnregisterPESImage(void)
{
  (void) UnregisterMagickInfo("PES");
}
