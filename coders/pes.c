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
%                                John Cristy                                  %
%                                 July 2009                                   %
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
%  The PES coders is derived from Robert Heel's PHP script (see
%  http://bobosch.dyndns.org/embroidery/showFile.php?pes.php) and pesconvert
%  (see http://torvalds-family.blogspot.com/2010/01/embroidery-gaah.html).
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/client.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/decorate.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/montage.h"
#include "magick/resize.h"
#include "magick/shear.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#include "magick/utility.h"

/*
  PES Colors.
*/

typedef struct _PESColorInfo
{
  unsigned char
    red,
    green,
    blue;
} PESColorInfo;

static const PESColorInfo
  PESColor[256] =
  {
    {   0,   0,   0 },
    {  14,  31, 124 },
    {  10,  85, 163 },
    {  48, 135, 119 },
    {  75, 107, 175 },
    { 237,  23,  31 },
    { 209,  92,   0 },
    { 145,  54, 151 },
    { 228, 154, 203 },
    { 145,  95, 172 },
    { 157, 214, 125 },
    { 232, 169,   0 },
    { 254, 186,  53 },
    { 255, 255,   0 },
    { 112, 188,  31 },
    { 192, 148,   0 },
    { 168, 168, 168 },
    { 123, 111,   0 },
    { 255, 255, 179 },
    {  79,  85,  86 },
    {   0,   0,   0 },
    {  11,  61, 145 },
    { 119,   1, 118 },
    {  41,  49,  51 },
    {  42,  19,   1 },
    { 246,  74, 138 },
    { 178, 118,  36 },
    { 252, 187, 196 },
    { 254,  55,  15 },
    { 240, 240, 240 },
    { 106,  28, 138 },
    { 168, 221, 196 },
    {  37, 132, 187 },
    { 254, 179,  67 },
    { 255, 240, 141 },
    { 208, 166,  96 },
    { 209,  84,   0 },
    { 102, 186,  73 },
    {  19,  74,  70 },
    { 135, 135, 135 },
    { 216, 202, 198 },
    {  67,  86,   7 },
    { 254, 227, 197 },
    { 249, 147, 188 },
    {   0,  56,  34 },
    { 178, 175, 212 },
    { 104, 106, 176 },
    { 239, 227, 185 },
    { 247,  56, 102 },
    { 181,  76, 100 },
    {  19,  43,  26 },
    { 199,   1,  85 },
    { 254, 158,  50 },
    { 168, 222, 235 },
    {   0, 103,  26 },
    {  78,  41, 144 },
    {  47, 126,  32 },
    { 253, 217, 222 },
    { 255, 217,  17 },
    {   9,  91, 166 },
    { 240, 249, 112 },
    { 227, 243,  91 },
    { 255, 200, 100 },
    { 255, 200, 150 },
    { 255, 200, 200 },
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
  Image
    *image;

  MagickBooleanType
    status;

  PESColorInfo
    colors[256];

  register long
    i;

  ssize_t
    count;

  size_t
    length;

  unsigned char
    magick[4],
    version[4];

  unsigned long
    number_colors;    

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
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
  length=(size_t) ReadBlobLSBLong(image);
  for (i=0; i < 37; i++)
    if (ReadBlobByte(image) == EOF)
      break;
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
  /*
    Get PES colors.
  */
  number_colors=ReadBlobByte(image)+1;
  for (i=0; i < (long) number_colors; i++)
    colors[i]=PESColor[ReadBlobByte(image)];
  for (i=0; i < (532-number_colors-49); i++)
    if (ReadBlobByte(image) == EOF)
      break;
  if (EOFBlob(image) != MagickFalse)
    ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
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
%      unsigned long RegisterPESImage(void)
%
*/
ModuleExport unsigned long RegisterPESImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PES");
  entry->decoder=(DecodeImageHandler *) ReadPESImage;
  entry->magick=(IsImageFormatHandler *) IsPES;
  entry->description=ConstantString("Brother PES");
  entry->module=ConstantString("PES");
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
