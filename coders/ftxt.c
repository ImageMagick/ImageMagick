/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        FFFFF  TTTTT  X   X  TTTTT                           %
%                        F        T     X X     T                             %
%                        FFF      T      X      T                             %
%                        F        T     X X     T                             %
%                        F        T    X   X    T                             %
%                                                                             %
%                 Read and Write Pixels as Formatted Text                     %
%                                                                             %
%                               Software Design                               %
%                             snibgo (Alan Gibson)                            %
%                                 October 2021                                %
%                                                                             %
%                                                                             %
%                                                                             %
%  Copyright 1999-2022 ImageMagick Studio LLC, a non-profit organization      %
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
%
*/

#include <MagickCore/studio.h>
#include <MagickCore/artifact.h>
#include <MagickCore/attribute.h>
#include <MagickCore/blob.h>
#include "MagickCore/blob-private.h"
#include <MagickCore/cache.h>
#include <MagickCore/channel.h>
#include <MagickCore/colorspace.h>
#include <MagickCore/exception.h>
#include "MagickCore/exception-private.h"
#include <MagickCore/image.h>
#include "MagickCore/image-private.h"
#include <MagickCore/list.h>
#include <MagickCore/magick.h>
#include <MagickCore/memory_.h>
#include <MagickCore/module.h>
#include <MagickCore/monitor.h>
#include "MagickCore/monitor-private.h"
#include <MagickCore/pixel-accessor.h>
#include "MagickCore/quantum-private.h"
#include <MagickCore/string_.h>
/*
  Forward declaration.
*/
static MagickBooleanType
  WriteFTXTImage(const ImageInfo *,Image *,ExceptionInfo *);


typedef enum {
  vtAny,
  vtQuant,
  vtPercent,
  vtProp,
  vtIntHex,
  vtFltHex
} ValueTypeT;

#define chEsc '\\'

#define dfltChSep ","

#define dfltFmt "\\x,\\y:\\c\\n"


static MagickBooleanType IsFTXT(const unsigned char *magick,const size_t length)
{
  if (length < 10)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"id=ftxt",10) == 0)
    return(MagickTrue);
  return(MagickFalse);
}



static int ReadChar (Image * image, int * chPushed)
{
  int ch;
  if (*chPushed) {
    ch = *chPushed;
    *chPushed = 0;
  } else {
    ch = ReadBlobByte (image);
  }
  return ch;
}

static int ReadInt (Image * image, MagickBooleanType *eofInp, int * chPushed, MagickBooleanType *err)
{
  char
    buffer[MaxTextExtent];

  char * p = buffer;

  int chIn = ReadChar (image, chPushed);
  if (chIn == EOF) *eofInp = MagickTrue;

  while (isdigit (chIn)) {
    *p = chIn;
    p++;
    if (p-buffer >= MaxTextExtent) {
      fprintf (stderr, "ReadInt too long\n");
      *eofInp = MagickTrue;
      continue;
    }
    chIn = ReadChar (image, chPushed);
  }
  if (p==buffer) {
    *eofInp = MagickTrue;
    return 0;
  }
  if (*eofInp) {
    *chPushed = '\0';
    return 0;
  }
  *p = '\0';
  *chPushed = chIn;

  char * tail;
  errno = 0;
  int val = strtol (buffer, &tail, 10);
  if (errno || *tail) {
    if (errno) fprintf (stderr, "ReadInt errno=%i: %s\n", errno, strerror (errno));
    if (*tail) fprintf (stderr, "ReadInt: unused input [%s]\n", tail);
    *eofInp = MagickTrue;
    *err = MagickTrue;
  }

  if (val < 0) {
    fprintf (stderr, "Negative integer [%i] not permitted\n", val);
    *err = MagickTrue;
  }

  return (val);
}

static long double BufToFlt (char * buffer, char ** tail, ValueTypeT expectType, MagickBooleanType *err)
{
  *err = MagickFalse;
  long double val = 0;

  if (*buffer == '#') {
    // read hex integer
    char * p = buffer + 1;
    while (*p) {
      short v;
      if (*p >= '0' && *p <= '9') v = *p - '0';
      else if (*p >= 'a' && *p <= 'f') v = *p - 'a' + 10;
      else if (*p >= 'A' && *p <= 'F') v = *p - 'A' + 10;
      else break;
      val = val * 16 + v;
      p++;
    }
    *tail = p;
    if (expectType != vtAny && expectType != vtIntHex) *err = MagickTrue;
  } else if (*buffer == '0' && *(buffer+1)=='x') {
    // read hex floating-point
    errno = 0;
    val = strtold (buffer, tail);
    if (errno) fprintf (stderr, "ReadFlt HexFlt errno=%i: %s\n", errno, strerror (errno));
    if (expectType != vtAny && expectType != vtFltHex) *err = MagickTrue;
  } else {
    // Read decimal floating-point (possibly a percent).
    errno = 0;
    val = strtold (buffer, tail);
    if (errno) {
      fprintf (stderr, "ReadFlt flt errno=%i: %s\n", errno, strerror (errno));
      *err = MagickTrue;
    }
    if (**tail=='%') {
      (*tail)++;
      val *= QuantumRange / 100.0;
      if (expectType != vtAny && expectType != vtPercent) *err = MagickTrue;
    } else {
      if (expectType == vtPercent) *err = MagickTrue;
    }
  }

  return (val);
}

static void SkipUntil (Image * image, int UntilChar, MagickBooleanType *eofInp, int * chPushed)
{
  int chIn = ReadChar (image, chPushed);
  if (chIn == EOF) {
    *eofInp = MagickTrue;
    *chPushed = '\0';
    return;
  }

  while (chIn != UntilChar && chIn != EOF) {
    chIn = ReadChar (image, chPushed);
  }
  if (chIn == EOF) {
    *eofInp = MagickTrue;
    *chPushed = '\0';
    return;
  }
  *chPushed = chIn;
}

static void ReadUntil (Image * image, int UntilChar, MagickBooleanType *eofInp, int * chPushed,
  char * buf, int BufSiz)
{
  int chIn;
  int i=0;

  for (;;) {
    chIn = ReadChar (image, chPushed);
    if (chIn == EOF) {
      if (i==0) *eofInp = MagickTrue;
      break;
    }
    if (chIn == UntilChar) break;
    if (i >= BufSiz) {
      fprintf (stderr, "ReadUntil: BufSiz busted\n");
      *eofInp = MagickTrue;
      break;
    }
    buf[i++] = chIn;
  }

  if (*eofInp) {
    *chPushed = '\0';
  }
  else *chPushed = chIn;
  buf[i] = '\0';
  if (UntilChar=='\n' && i>0 && buf[i-1]=='\r') buf[i-1]='\0';
}

static Image *ReadFTXTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    buffer[MaxTextExtent];

  Image
    *image;

  MagickBooleanType
    status;

  Quantum
    *q;

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

  MagickBooleanType verbose = image_info->verbose;

  if (verbose) {
    fprintf (stderr, "Image colorspace: %i\n", image->colorspace);
    fprintf (stderr, "Image_info colorspace: %i\n", image_info->colorspace);
  }

  SetImageColorspace (image, RGBColorspace, exception);
  SetImageColorspace (image, image_info->colorspace, exception);

  if (verbose) {
    fprintf (stderr, "Image colorspace: %i\n", image->colorspace);
    fprintf (stderr, "num channels: %lu\n", GetPixelChannels (image));
    fprintf (stderr, "Image depth: %lu\n", image->depth);
    fprintf (stderr, "Image_info depth: %lu\n", image_info->depth);
  }

  const char * sFmt = GetImageArtifact (image, "ftxt:format");
  if (sFmt == NULL) sFmt = dfltFmt;

  const char * sChSep = GetImageArtifact (image, "ftxt:chsep");
  if (sChSep == NULL) sChSep = dfltChSep;

  char chSep;
  if (sChSep[0]==chEsc && (sChSep[1] == 'n' || sChSep[1] == 'N')) chSep = '\n';
  else chSep = sChSep[0];

  MagickBooleanType hasAlpha = IsStringTrue (GetImageArtifact (image, "ftxt:hasalpha"));

  int numMeta = 0;
  const char * sNumMeta = GetImageArtifact (image, "ftxt:nummeta");
  if (sNumMeta != NULL) numMeta = atoi(sNumMeta);

  if (hasAlpha) {
    if (!SetImageAlphaChannel (image, OpaqueAlphaChannel, exception))
      ThrowReaderException(OptionError,"SetImageAlphaChannelFailure");
  }

  if (numMeta) {
    if (!SetPixelMetaChannels (image, numMeta, exception))
      ThrowReaderException(OptionError,"SetPixelMetaChannelsFailure");
  }

  if (verbose) {
    fprintf (stderr, "numMeta %i\n", numMeta);
    fprintf (stderr, "num channels: %lu\n", GetPixelChannels (image));
  }

  // make image zero (if RGB channels, transparent black).
  PixelInfo mppBlack;
  GetPixelInfo (image, &mppBlack);
  if (hasAlpha) mppBlack.alpha = TransparentAlpha;
  SetImageColor (image, &mppBlack, exception);

  char
    procFmt[MaxTextExtent];

  const char * pf = sFmt;
  char * ppf = procFmt;

  int i = 0;
  while (*pf) {
    if (*pf == chEsc) {
      pf++;
      switch (*pf) {
        case chEsc:
          if (++i >= MaxTextExtent) ThrowReaderException (DelegateFatalError, "ppf bust");
          *ppf = chEsc;
          ppf++;
          break;
        case 'n':
          if (++i >= MaxTextExtent) ThrowReaderException (DelegateFatalError, "ppf bust");
          *ppf = '\n';
          ppf++;
          break;
        case 'j':
          if (*(pf+1)=='\0') ThrowReaderException (DelegateFatalError, "EscapeJproblem");
          // Drop through...
        default:
          if ((i+=2) >= MaxTextExtent) ThrowReaderException (DelegateFatalError, "ppf bust");
          *ppf = chEsc;
          ppf++;
          *ppf = *pf;
          ppf++;
          break;
      }
    } else {
      // Not escape
      if (++i >= MaxTextExtent) ThrowReaderException (DelegateFatalError, "ppf bust");
      *ppf = *pf;
      ppf++;
    }
    pf++;
  }
  *ppf = '\0';

  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");

  if (verbose) fprintf (stderr, "size %lix%li\n", image->columns, image->rows);

  // How many channel values can we expect?
  int nExpCh = 0;
  for (i=0; i < (ssize_t) GetPixelChannels (image); i++) {
    PixelChannel channel = GetPixelChannelChannel (image, i);
    PixelTrait traits = GetPixelChannelTraits (image, channel);
    if (verbose) fprintf (stderr, "i=%i ch=%i traits=%i\n", i, channel, traits);
    if ((traits & UpdatePixelTrait) != UpdatePixelTrait) continue;
    nExpCh++;
  }
  if (verbose)
    fprintf (stderr, "nExpCh=%i: Expect %i channel values%s.\n",
             nExpCh, nExpCh,
             (hasAlpha) ? " (including alpha)" : ""
            );

  long double chVals[MaxPixelChannels];
  for (i=0; i < MaxPixelChannels; i++) chVals[i] = 0;

  MagickBooleanType eofInp = MagickFalse;

  int chIn;
  int chPushed = 0;

  ssize_t x=0, y=0, MaxX=-1, MaxY=-1;
  ssize_t nPix = 0;

  MagickBooleanType
    firstX = MagickTrue,
    firstY = MagickTrue,
    IntErr = MagickFalse,
    TypeErr = MagickFalse,
    nChErr = MagickFalse;

  while (!eofInp) {
    ValueTypeT expectType = vtAny;
    char * ppf = procFmt;
    while (*ppf && !eofInp) {
      if (*ppf == chEsc) {
        ppf++;
        switch (*ppf) {
          case 'x': {
            x = ReadInt (image, &eofInp, &chPushed, &IntErr);
            if (IntErr || eofInp) continue;
            if (firstX) {
              firstX = MagickFalse;
              MaxX = x;
            } else if (MaxX < x) MaxX = x;
            break;
          }
          case 'y': {
            y = ReadInt (image, &eofInp, &chPushed, &IntErr);
            if (IntErr || eofInp) continue;
            if (firstY) {
              firstY = MagickFalse;
              MaxY = y;
            } else if (MaxY < y) MaxY = y;
            break;
          }

          case 'c':
          case 'v':
          case 'p':
          case 'o':
          case 'h':
          case 'f':
          {
            if (*ppf=='c') {
              expectType = vtAny;
            } else if (*ppf=='v') {
              expectType = vtQuant;
            } else if (*ppf=='p') {
              expectType = vtPercent;
            } else if (*ppf=='o') {
              expectType = vtProp;
            } else if (*ppf=='h') {
              expectType = vtIntHex;
            } else if (*ppf=='f') {
              expectType = vtFltHex;
            }
            /* Read chars until next char in format,
               then parse that string into chVals[],
               then write that into image.
            */
            int UntilChar = *(ppf+1);
            ReadUntil (image, UntilChar, &eofInp, &chPushed, buffer, MaxTextExtent-1);
            if (eofInp) break;

            char * tail;
            char * pt = buffer;
            long double val;
            i = 0;
            for (;;) { // Loop through input channels.
              val = BufToFlt (pt, &tail, expectType, &TypeErr);
              if (expectType == vtProp) val *= QuantumRange;
              if (TypeErr) {
                fprintf (stderr, "Type error: (%li,%li) expected %i at %s i=%i val=%Lg tail=[%s]\n",
                         x, y, (int)expectType, pt, i, val, tail);
                break;
              }

              if (i < MaxPixelChannels) chVals[i] = val;

              if (*tail=='\r' && chSep=='\n' && *(tail+1)=='\n') tail++;

              if (*tail == chSep) {
                pt = tail+1;
              } else {
                break;
              }
              i++;
            }

            if (i+1 != nExpCh) {
              nChErr = MagickTrue;
              fprintf (stderr, "NumChannelsError (%li,%li): i=%i but nExpCh=%i\n", x, y, i, nExpCh);
            }

            if (x < (ssize_t) image->columns && y < (ssize_t) image->rows) {
              q = QueueAuthenticPixels (image, x,y, 1,1, exception);
              if (!q) break;
              for (i=0; i< nExpCh; i++) {
                q[i] = chVals[i];
              }
              if (!SyncAuthenticPixels (image,exception)) break;
            }

            break;
          }
          case 'j':
            // Skip chars until we find char after *ppf.
            SkipUntil (image, *(ppf+1), &eofInp, &chPushed);
            break;
          case 'H':
          case 's': {
            int UntilChar = *(ppf+1);
            ReadUntil (image, UntilChar, &eofInp, &chPushed, buffer, MaxTextExtent-1);
            if (eofInp) break;
            if (!*buffer) {
              fprintf (stderr, "buffer empty (%li,%li)\n", x, y);
              ThrowReaderException(CorruptImageError,"No input for escape 'H' or 's'.");
            }
            PixelInfo pixelinf;
            if (!QueryColorCompliance(buffer, AllCompliance, &pixelinf, exception)) {
              fprintf (stderr, "QueryColorCompliance failed (%li,%li) [%s]\n", x, y, buffer);
              break;
            }

            if (x < (ssize_t) image->columns && y < (ssize_t) image->rows) {
              q = QueueAuthenticPixels (image, x,y, 1,1, exception);
              if (!q) break;

              SetPixelViaPixelInfo(image, &pixelinf, q);

              if (!SyncAuthenticPixels (image,exception)) break;
            }

            break;
          }
          default:
            fprintf (stderr, "Unknown escape '%c'\n", *ppf);
            break;
        }
      } else {
        // Not escape
        chIn = ReadChar (image, &chPushed);
        if (chIn == EOF) {
          if (ppf != procFmt) {
            fprintf (stderr, "(%li,%li) Expect [%c] %i but found EOF.\n", x, y, *ppf, (int)*ppf);
            ThrowReaderException(CorruptImageError,"EOFduringFormat");
          }
          eofInp = MagickTrue;
        } else {
          if (chIn == '\r' && *ppf == '\n') {
            chIn = ReadChar (image, &chPushed);
            if (chIn != '\n') {
              fprintf (stderr, "(%li,%li) \\r not followed by \\n.\n", x, y);
              ThrowReaderException(CorruptImageError,"BackslashRbad");
            }
          }

          if (chIn != *ppf) {
            fprintf (stderr, "(%li,%li) Error at character '%c'. Expected '%c'.\n", x, y, chIn, *ppf);
            ThrowReaderException(CorruptImageError,"UnexpectedInputChar");
          }
        }
      }
      ppf++;
    }
    if (!eofInp) {
      nPix++;

      if (MaxX < x) MaxX = x;
      if (MaxY < y) MaxY = y;

      if (firstX && firstY) {
        x++;
        if (x >= (ssize_t) image->columns) {
          x = 0;
          y++;
        }
      }
    }
  }
  if (IntErr)
    ThrowReaderException(CorruptImageError,"ParseIntegerError");

  if (TypeErr)
    ThrowReaderException(CorruptImageError,"TypeError");

  if (chPushed) {
    fprintf (stderr, "Unused pushed char [%c] %i\n", chPushed, (int)chPushed);
    ThrowReaderException(CorruptImageError,"UnusedPushedChar");
  }

  if (MaxX < 0 && MaxY < 0) {
    fprintf (stderr, "Unexpected EOF: no pixels were read\n");
    ThrowReaderException(CorruptImageError,"UnexpectedEof");
  }

  if (nChErr) {
    ThrowReaderException(CorruptImageError,"NumChannelsError");
  }

  if (verbose) fprintf (stderr, "nPix=%li MaxX=%li MaxY=%li\n", nPix, MaxX, MaxY);

  if (nPix > (ssize_t) (image->columns * image->rows)) {
    fprintf (stderr, "Too many pixels were read\n");
    fprintf (stderr, "nPix=%li MaxX=%li MaxY=%li\n", nPix, MaxX, MaxY);
    ThrowMagickException(exception, GetMagickModule(), CorruptImageWarning,"TooManyPixels", "`%s'",image_info->filename);
  } else if (MaxX >= (ssize_t) image->columns || MaxY >= (ssize_t) image->rows) {
    fprintf (stderr, "Image bounds exceeded\n");
    fprintf (stderr, "nPix=%li MaxX=%li MaxY=%li\n", nPix, MaxX, MaxY);
    ThrowMagickException(exception, GetMagickModule(), CorruptImageWarning,"ImageBoundsExceeded", "`%s'",image_info->filename);
  }

  return(GetFirstImageInList(image));

}



ModuleExport unsigned long RegisterFTXTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("FTXT","FTXT","Formatted text image");
  entry->decoder=(DecodeImageHandler *) ReadFTXTImage;
  entry->encoder=(EncodeImageHandler *) WriteFTXTImage;
  entry->magick=(IsImageFormatHandler *) IsFTXT;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}



ModuleExport void UnregisterFTXTImage(void)
{
  (void) UnregisterMagickInfo("FTXT");
}



static MagickBooleanType WriteFTXTImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  char
    buffer[MaxTextExtent];

  long
    x,
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  const Quantum
    *p;

  PixelInfo
    pixel;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;

  MagickBooleanType verbose = image_info->verbose;

  int precision = GetMagickPrecision();
  const char * sFmt;

  sFmt = GetImageArtifact (image, "ftxt:format");
  if (sFmt == NULL) sFmt = dfltFmt;

  const char * sChSep = GetImageArtifact (image, "ftxt:chsep");
  if (sChSep == NULL) sChSep = dfltChSep;

  char chSep;
  if (sChSep[0]==chEsc && (sChSep[1] == 'n' || sChSep[1] == 'N')) chSep = '\n';
  else chSep = sChSep[0];

  int depth = image->depth;

  char sSuff[2];
  sSuff[0] = '\0';
  sSuff[1] = '\0';

  if (verbose) {
    fprintf (stderr, "Image colorspace: %i\n", image->colorspace);
    fprintf (stderr, "GetPixelChannels(): %lu\n", GetPixelChannels (image));
    fprintf (stderr, "Image depth: %i\n", depth);
    fprintf (stderr, "Image_info depth: %lu\n", image_info->depth);
    fprintf (stderr, "Image Q depth: %lu\n", GetImageQuantumDepth(image, MagickFalse));
  }

  do
  {
    GetPixelInfo(image,&pixel);

    for (y=0; y < (long) image->rows; y++)
    {
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (!p) break;
      for (x=0; x < (long) image->columns; x++)
      {
        long double ValMult=1.0;
        MagickBooleanType HexFmt = MagickFalse;
        MagickBooleanType FltHexFmt = MagickFalse;
        const char * pFmt = sFmt;
        while (*pFmt) {
          if (*pFmt == chEsc) {
            pFmt++;
            switch (*pFmt) {
              case 'x':
                FormatLocaleString (buffer, MaxTextExtent, "%li", x);
                WriteBlobString (image, buffer);
                break;
              case 'y':
                FormatLocaleString (buffer, MaxTextExtent, "%li", y);
                WriteBlobString (image, buffer);
                break;
              case 'n':
                WriteBlobString (image, "\n");
                break;
              case chEsc:
                FormatLocaleString (buffer, MaxTextExtent, "%c%c", chEsc, chEsc);
                WriteBlobString (image, buffer);
                break;
              case 'c':
              case 'v':
              case 'p':
              case 'o':
              case 'h':
              case 'f':
              {
                HexFmt = MagickFalse;
                if (*pFmt=='c') {
                  ValMult = 1.0;
                } else if (*pFmt=='v') {
                  ValMult = 1.0;
                } else if (*pFmt=='p') {
                  ValMult = 100 * QuantumScale;
                  sSuff[0] = '%';
                } else if (*pFmt=='o') {
                  ValMult = QuantumScale;
                } else if (*pFmt=='h') {
                  ValMult = 1.0;
                  HexFmt = MagickTrue;
                } else if (*pFmt=='f') {
                  ValMult = 1.0;
                  FltHexFmt = MagickTrue;
                }
                // Output all "-channel" channels.
                ssize_t i;
                char sSep[2];
                sSep[0] = sSep[1] = '\0';
                for (i=0; i < (ssize_t) GetPixelChannels (image); i++) {
                  PixelChannel channel = GetPixelChannelChannel (image, i);
                  PixelTrait traits = GetPixelChannelTraits (image, channel);
                  if (verbose) fprintf (stderr, "i=%li ch=%i tr=%i\n", i, channel, traits);
                  if ((traits & UpdatePixelTrait) != UpdatePixelTrait) continue;
                  if (HexFmt) {
                    FormatLocaleString (buffer,MaxTextExtent,"%s#%Lx", sSep, (signed long long)(((long double)p[i])+0.5));
                  } else if (FltHexFmt) {
                    FormatLocaleString (buffer,MaxTextExtent,"%s%a", sSep, p[i]);
                  } else {
                    FormatLocaleString (buffer,MaxTextExtent,"%s%.*Lg%s", sSep, precision, p[i]*ValMult, sSuff);
                  }
                  WriteBlobString (image, buffer);
                  sSep[0] = chSep;
                }
                break;
              }
              case 'j':
                // Output nothing.
                break;
              case 's':
                GetPixelInfoPixel (image, p, &pixel);
                GetColorTuple (&pixel, MagickFalse, buffer);
                WriteBlobString (image, buffer);
                break;
              case 'H':
                GetPixelInfoPixel (image, p, &pixel);
                // For reading, QueryColorCompliance misreads 64 bit/channel hex colours,
                // so when writing we ensure it is at most 32 bits.
                if (pixel.depth > 32) pixel.depth = 32;
                GetColorTuple (&pixel, MagickTrue, buffer);
                WriteBlobString (image, buffer);
                break;
              default:
                break;
            }
          } else {
            // Not an escape char.
            buffer[0] = *pFmt;
            buffer[1] = '\0';
            (void) WriteBlobString(image,buffer);
          }
          pFmt++;
        }
        p += GetPixelChannels (image);
      }
      if (image->previous == (Image *) NULL)
        if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
            (QuantumTick(y,image->rows) != MagickFalse))
          {
            status=image->progress_monitor(SaveImageTag,y,image->rows,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
    }
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
    scene++;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}
