/*
  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image compression/decompression methods.
*/
#ifndef MAGICKCORE_COMPRESS_H
#define MAGICKCORE_COMPRESS_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedCompression,
  B44ACompression,
  B44Compression,
  BZipCompression,
  DXT1Compression,
  DXT3Compression,
  DXT5Compression,
  FaxCompression,
  Group4Compression,
  JBIG1Compression,        /* ISO/IEC std 11544 / ITU-T rec T.82 */
  JBIG2Compression,        /* ISO/IEC std 14492 / ITU-T rec T.88 */
  JPEG2000Compression,     /* ISO/IEC std 15444-1 */
  JPEGCompression,
  LosslessJPEGCompression,
  LZMACompression,         /* Lempel-Ziv-Markov chain algorithm */
  LZWCompression,
  NoCompression,
  PizCompression,
  Pxr24Compression,
  RLECompression,
  ZipCompression,
  ZipSCompression
} CompressionType;

typedef struct _Ascii85Info
  Ascii85Info;

extern MagickExport MagickBooleanType
  HuffmanDecodeImage(Image *,ExceptionInfo *),
  HuffmanEncodeImage(const ImageInfo *,Image *,Image *,ExceptionInfo *),
  LZWEncodeImage(Image *,const size_t,unsigned char *magick_restrict,
    ExceptionInfo *),
  PackbitsEncodeImage(Image *,const size_t,unsigned char *magick_restrict,
    ExceptionInfo *),
  ZLIBEncodeImage(Image *,const size_t,unsigned char *magick_restrict,
    ExceptionInfo *);

extern MagickExport void
  Ascii85Encode(Image *,const unsigned char),
  Ascii85Flush(Image *),
  Ascii85Initialize(Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
