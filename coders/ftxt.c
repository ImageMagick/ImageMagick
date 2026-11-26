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
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/license/                                         %
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

#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/channel.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/constitute.h"
#include "MagickCore/draw.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "coders/ftxt.h"

/*
  Define declarations.
*/
#define chEsc '\\'
#define dfltChSep ","
#define dfltFmt "\\x,\\y:\\c\\n"

/*
  Enumerated declarations.
*/
typedef enum
  {
    vtAny,
    vtQuant,
    vtPercent,
    vtProp,
    vtIntHex,
    vtFltHex
  } ValueTypeT;

/*
  Forward declaration.
*/
static MagickBooleanType
  WriteFTXTImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s F T X T                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsFTXT() returns MagickTrue if the image format type, identified by the
%  magick string, is formatted text.
%
%  The format of the IsFTXT method is:
%
%      MagickBooleanType IsFTXT(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsFTXT(const unsigned char *magick,const size_t length)
{
  if (length < 7)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"id=ftxt",7) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F T X T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadFTXTImage() reads an formatted text image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadFTXTImage method is:
%
%      Image *ReadFTXTImage(image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static int ReadChar(Image *image, int *chPushed)
{
  int
    ch;

  if (*chPushed)
    {
      ch=*chPushed;
      *chPushed=0;
    }
  else
    ch=ReadBlobByte(image);
  return(ch);
}

static ssize_t ReadInt(Image * image,MagickBooleanType *eofInp,int *chPushed,
  MagickBooleanType *err)
{
  char
    buffer[MagickPathExtent];

  char
    *p,
    *tail;

  int
    chIn;

  long
    val;

  p=buffer;
  chIn=ReadChar(image,chPushed);
  if (chIn == EOF)
    *eofInp = MagickTrue;
  while (isdigit(chIn))
  {
    *p=(char) chIn;
    p++;
    if (p-buffer >= MagickPathExtent-1)
      {
        *eofInp=MagickTrue;
        *err=MagickTrue;
        break;
      }
    chIn=ReadChar(image,chPushed);
  }
  if (p == buffer)
    {
      /* No digits: clean EOF is fine, anything else is corrupt input. */
      if (chIn != EOF)
        *err=MagickTrue;
      *eofInp=MagickTrue;
      return(0);
    }
  if (*eofInp)
    {
      *chPushed='\0';
      return(0);
    }
  *p='\0';
  *chPushed=chIn;
  errno=0;
  val=strtol(buffer,&tail,10);
  if (errno || *tail)
    {
      *eofInp=MagickTrue;
      *err=MagickTrue;
    }
  if (val < 0)
    *err=MagickTrue;
  return((ssize_t) val);
}

static long double BufToFlt(char * buffer,char ** tail,ValueTypeT expectType,
  MagickBooleanType *err)
{
  long double
    val;

  *err=MagickFalse;
  val=0;
  if (*buffer == '#')
    {
      char
        *p;

      /* read hex integer */
      p=buffer+1;
      while (*p)
      {
        short
          v;

        if (*p >= '0' && *p <= '9')
          v=*p-'0';
        else if (*p >= 'a' && *p <= 'f')
          v=*p-'a'+10;
        else if (*p >= 'A' && *p <= 'F')
          v=*p-'A'+10;
        else
          break;
        val=val*16+v;
        p++;
      }
      *tail=p;
      if (p == buffer+1)
        *err=MagickTrue;
      if ((expectType != vtAny) && (expectType != vtIntHex))
        *err=MagickTrue;
    }
  else if ((*buffer == '0') && (*(buffer + 1) == 'x'))
    {
      /* read hex floating-point */
      val=strtold(buffer,tail);
      if ((expectType != vtAny) && (expectType != vtFltHex))
        *err=MagickTrue;
    }
  else
    {
      /* Read decimal floating-point (possibly a percent). */
      errno=0;
      val=strtold(buffer,tail);
      if (errno || (*tail == buffer))
        *err=MagickTrue;
      if (**tail=='%')
        {
          (*tail)++;
          val*=(long double) QuantumRange/100.0L;
          if ((expectType != vtAny) && (expectType != vtPercent))
            *err=MagickTrue;
        }
      else if (expectType == vtPercent)
        *err=MagickTrue;
    }
  return(val);
}

static void SkipUntil(Image * image,int UntilChar,MagickBooleanType *eofInp,
  int *chPushed)
{
  int
    chIn;

  chIn=ReadChar(image,chPushed);
  if (chIn == EOF)
    {
      *eofInp=MagickTrue;
      *chPushed='\0';
      return;
    }
  while ((chIn != UntilChar) && (chIn != EOF))
    chIn=ReadChar(image,chPushed);
  if (chIn == EOF)
    {
      *eofInp=MagickTrue;
      *chPushed='\0';
      return;
    }
  *chPushed=chIn;
}

static void ReadUntil(Image * image,int UntilChar,MagickBooleanType *eofInp,
  int *chPushed,char *buf,int bufSize)
{
  int
    chIn,
    i;

  i=0;
  for (;;)
    {
      chIn=ReadChar(image,chPushed);
      if (chIn == EOF)
        {
          if (i==0)
            *eofInp=MagickTrue;
          break;
        }
      if (chIn == UntilChar)
        break;
      if (i >= bufSize)
        {
          *eofInp=MagickTrue;
          break;
        }
      buf[i++]=(char) chIn;
    }
  if (*eofInp)
    *chPushed='\0';
  else
    *chPushed=chIn;
  buf[i]='\0';
  if ((UntilChar == '\n') && (i > 0) && (buf[i-1] == '\r'))
    buf[i-1]='\0';
}

static Image *ReadFTXTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    chSep,
    *ppf,
    procFmt[MagickPathExtent];

  const char
    *pf,
    *sChSep,
    *sFmt,
    *sNumMeta;

  Image
    *image;

  int
    chIn,
    chPushed,
    nExpCh,
    numMeta;

  ssize_t
    i;

  long double
    chVals[MaxPixelChannels];

  MagickBooleanType
    eofInp,
    firstX,
    firstY,
    hasAlpha,
    intErr,
    nChErr,
    status,
    typeErr,
    valErr;

  MagickOffsetType
    offsetStart;

  PixelInfo
    mppBlack;

  Quantum
    *q;

  ssize_t
    maxX,
    maxY,
    nPix,
    x,
    y;

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
  SetImageColorspace(image,RGBColorspace,exception);
  SetImageColorspace(image,image_info->colorspace,exception);
  sFmt=GetImageArtifact(image,"ftxt:format");
  if (sFmt == (const char *) NULL)
    sFmt=dfltFmt;
  sChSep=GetImageArtifact(image,"ftxt:chsep");
  if (sChSep == (const char *) NULL)
    sChSep=dfltChSep;
  if ((sChSep[0] == chEsc) && ((sChSep[1] == 'n' || sChSep[1] == 'N')))
    chSep='\n';
  else
    chSep=sChSep[0];
  hasAlpha=IsStringTrue(GetImageArtifact(image,"ftxt:hasalpha"));
  numMeta=0;
  sNumMeta=GetImageArtifact(image,"ftxt:nummeta");
  if (sNumMeta != (const char *) NULL)
    numMeta=atoi(sNumMeta);
  if ((numMeta < 0) || (numMeta > MaxPixelChannels))
    ThrowReaderException(OptionError,"InvalidMetaChannelCount");
  if (hasAlpha)
    {
      if (SetImageAlphaChannel(image,OpaqueAlphaChannel,exception) == MagickFalse)
        ThrowReaderException(OptionError,"SetImageAlphaChannelFailure");
    }
  if (numMeta)
    {
      if (SetPixelMetaChannels (image, (size_t) numMeta, exception) == MagickFalse)
        ThrowReaderException(OptionError,"SetPixelMetaChannelsFailure");
    }
  /* make image zero (if RGB channels, transparent black). */
  GetPixelInfo(image,&mppBlack);
  if (hasAlpha)
    mppBlack.alpha=TransparentAlpha;
  SetImageColor(image,&mppBlack,exception);
  pf=sFmt;
  ppf=procFmt;
  i=0;
  while (*pf)
  {
    if (*pf == chEsc)
      {
        pf++;
        if (*pf == '\0')
          ThrowReaderException(OptionError,"TrailingEscapeInFormat");
        switch (*pf) {
          case chEsc:
            if ((i+=2) >= MagickPathExtent)
              ThrowReaderException(DelegateFatalError,"ppf bust");
            *ppf=chEsc;
            ppf++;
            *ppf=chEsc;
            ppf++;
            break;
          case 'n':
            if (++i >= MagickPathExtent)
              ThrowReaderException(DelegateFatalError,"ppf bust");
            *ppf='\n';
            ppf++;
            break;
          case 'j':
            if (*(pf+1)=='\0')
              ThrowReaderException(DelegateFatalError,"EscapeJproblem");
            magick_fallthrough;
          default:
            if ((i+=2) >= MagickPathExtent)
              ThrowReaderException(DelegateFatalError,"ppf bust");
            *ppf=chEsc;
            ppf++;
            *ppf=*pf;
            ppf++;
            break;
        }
      }
    else
      {
        /* Not escape */
        if (++i >= MagickPathExtent)
          ThrowReaderException (DelegateFatalError,"ppf bust");
        *ppf=*pf;
        ppf++;
      }
    pf++;
  }
  *ppf='\0';
  if (ppf == procFmt)
    ThrowReaderException(OptionError,"EmptyFormat");
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  /* How many channel values can we expect? */
  nExpCh=0;
  for (i=0; i < (ssize_t) GetPixelChannels (image); i++)
  {
    PixelChannel
      channel;

    PixelTrait
      traits;

    channel=GetPixelChannelChannel(image,i);
    traits=GetPixelChannelTraits(image,channel);
    if ((traits & UpdatePixelTrait) != UpdatePixelTrait)
      continue;
    nExpCh++;
  }
  for (i=0; i < MaxPixelChannels; i++)
    chVals[i] = 0;
  eofInp=MagickFalse;
  chPushed=0;
  x=0;
  y=0;
  maxX=-1;
  maxY=-1;
  nPix=0;
  firstX=MagickTrue;
  firstY=MagickTrue;
  intErr=MagickFalse;
  typeErr=MagickFalse;
  nChErr=MagickFalse;
  valErr=MagickFalse;
  while (!eofInp)
  {
    ValueTypeT
      expectType;

    expectType=vtAny;
    offsetStart=TellBlob(image);
    ppf=procFmt;
    while (*ppf && eofInp == MagickFalse)
    {
      if (*ppf == chEsc)
        {
          ppf++;
          switch (*ppf)
          {
            case 'x':
            {
              x=ReadInt(image,&eofInp,&chPushed,&intErr);
              if (intErr != MagickFalse)
                eofInp=MagickTrue;
              if (eofInp != MagickFalse)
                break;
              if (firstX != MagickFalse)
                {
                  firstX=MagickFalse;
                  maxX=x;
                }
              else if (maxX < x)
                maxX=x;
              break;
            }
            case 'y':
            {
              y=ReadInt(image,&eofInp,&chPushed,&intErr);
              if (intErr != MagickFalse)
                eofInp=MagickTrue;
              if (eofInp != MagickFalse)
                break;
              if (firstY != MagickFalse)
                {
                  firstY=MagickFalse;
                  maxY=y;
                }
              else if (maxY < y)
                maxY=y;
              break;
            }
            case 'c':
            case 'v':
            case 'p':
            case 'o':
            case 'h':
            case 'f':
            {
              char
                *pt,
                *tail;

              int
                untilChar;

              long double
                val;

              if (*ppf == 'c')
                expectType=vtAny;
              else if (*ppf == 'v')
                expectType=vtQuant;
              else if (*ppf=='p')
                expectType=vtPercent;
              else if (*ppf=='o')
                expectType=vtProp;
              else if (*ppf=='h')
                expectType=vtIntHex;
              else if (*ppf=='f')
                expectType=vtFltHex;
              /*
                Read chars until next char in format,
                then parse that string into chVals[],
                then write that into image.
              */
              untilChar=*(ppf+1);
              ReadUntil(image,untilChar,&eofInp,&chPushed,buffer,
                MagickPathExtent-1);
              if (eofInp != MagickFalse)
                break;
              pt=buffer;
              i=0;
              for (;;)
              {
                /* Loop through input channels. */
                val=BufToFlt(pt,&tail,expectType,&valErr);
                if (expectType == vtProp)
                  val *= QuantumRange;
                if (valErr != MagickFalse)
                  {
                    typeErr=MagickTrue;
                    break;
                  }
                if (i < MaxPixelChannels)
                  chVals[i]=val;
                if ((*tail == '\r') && (chSep == '\n') && (*(tail + 1) == '\n'))
                  tail++;
                if (*tail == chSep)
                  pt=tail+1;
                else
                  {
                    while (isspace((int) ((unsigned char) *tail)))
                      tail++;
                    if (*tail != '\0')
                      typeErr=MagickTrue;
                    break;
                  }
                i++;
              }
              if (i+1 != nExpCh)
                nChErr=MagickTrue;
              if (x < (ssize_t) image->columns && y < (ssize_t) image->rows)
                {
                  ssize_t
                    k;

                  q=GetAuthenticPixels(image,x,y,1,1,exception);
                  if (q == (Quantum *) NULL)
                    {
                      status=MagickFalse;
                      eofInp=MagickTrue;
                      break;
                    }
                  k=0;
                  for (i=0; (i < (ssize_t) GetPixelChannels(image)) &&
                            (k < nExpCh); i++)
                  {
                    PixelChannel
                      channel;

                    PixelTrait
                      traits;

                    channel=GetPixelChannelChannel(image,i);
                    traits=GetPixelChannelTraits(image,channel);
                    if ((traits & UpdatePixelTrait) != UpdatePixelTrait)
                      continue;
                    q[i]=ClampToQuantum((MagickRealType) chVals[k]);
                    k++;
                  }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    {
                      status=MagickFalse;
                      eofInp=MagickTrue;
                      break;
                    }
                }
              break;
            }
            case chEsc:
            {
              /* Literal backslash. */
              chIn=ReadChar(image,&chPushed);
              if (chIn == EOF)
                ThrowReaderException(CorruptImageError,"EOFduringFormat");
              if (chIn != chEsc)
                ThrowReaderException(CorruptImageError,"UnexpectedInputChar");
              break;
            }
            case 'j':
            {
              /* Skip chars until we find char after *ppf. */
              SkipUntil(image,*(ppf+1),&eofInp,&chPushed);
              break;
            }
            case 'H':
            case 's':
            {
              int
                untilChar;

              PixelInfo
                pixelinf;

              untilChar=*(ppf+1);
              ReadUntil(image,untilChar,&eofInp,&chPushed,buffer,
                MagickPathExtent-1);
              if (eofInp != MagickFalse)
                break;
              if (*buffer == 0)
                ThrowReaderException(CorruptImageError,
                  "No input for escape 'H' or 's'.");
              if (QueryColorCompliance(buffer,AllCompliance,&pixelinf,
                    exception) == MagickFalse)
                break;
              if (x < (ssize_t) image->columns && y < (ssize_t) image->rows)
                {
                  q=GetAuthenticPixels(image,x,y,1,1,exception);
                  if (q == (Quantum *) NULL)
                    {
                      status=MagickFalse;
                      eofInp=MagickTrue;
                      break;
                    }
                  SetPixelViaPixelInfo(image,&pixelinf,q);
                  if (SyncAuthenticPixels(image,exception) == MagickFalse)
                    {
                      status=MagickFalse;
                      eofInp=MagickTrue;
                      break;
                    }
                }
              break;
            }
            default:
              break;
          }
        }
      else
        {
          /* Not escape */
          chIn=ReadChar(image,&chPushed);
          if (chIn == EOF)
            {
              if (ppf != procFmt)
                ThrowReaderException(CorruptImageError,"EOFduringFormat");
              eofInp=MagickTrue;
            }
          else
            {
              if ((chIn == '\r') && (*ppf == '\n'))
                {
                  chIn=ReadChar(image,&chPushed);
                  if (chIn != '\n')
                    ThrowReaderException(CorruptImageError,"BackslashRbad");
                }
              if (chIn != *ppf)
                ThrowReaderException(CorruptImageError,"UnexpectedInputChar");
            }
        }
      ppf++;
    }
    if ((eofInp == MagickFalse) && (TellBlob(image) == offsetStart))
      ThrowReaderException(CorruptImageError,"FormatConsumesNoInput");
    if (eofInp == MagickFalse)
      {
        nPix++;
        if (maxX < x)
          maxX=x;
        if (maxY < y)
          maxY=y;
        if ((firstX != MagickFalse) && (firstY != MagickFalse))
          {
            x++;
            if (x >= (ssize_t) image->columns)
              {
                x=0;
                y++;
              }
          }
      }
  }
  if (intErr != MagickFalse)
    ThrowReaderException(CorruptImageError,"ParseIntegerError");
  if (typeErr != MagickFalse)
    ThrowReaderException(CorruptImageError,"TypeError");
  if (chPushed != 0)
    ThrowReaderException(CorruptImageError,"UnusedPushedChar");
  if ((maxX < 0) && (maxY < 0))
    ThrowReaderException(CorruptImageError,"UnexpectedEof");
  if (nChErr != MagickFalse)
    ThrowReaderException(CorruptImageError,"NumChannelsError");
  if (nPix > (ssize_t) (image->columns * image->rows))
    ThrowMagickException(exception,GetMagickModule(),CorruptImageWarning,
      "TooManyPixels","`%s'",image_info->filename);
  else if ((maxX >= (ssize_t) image->columns) ||
           (maxY >= (ssize_t) image->rows))
    ThrowMagickException(exception,GetMagickModule(),CorruptImageWarning,
      "ImageBoundsExceeded","`%s'",image_info->filename);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r F T X T I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterFTXTImage() adds properties for the FTXT image format to
%  the list of supported formats. The properties include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterFTXTImage method is:
%
%      size_t RegisterFTXTImage(void)
%
*/
ModuleExport size_t RegisterFTXTImage(void)
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r F T X T I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterFTXTImage() removes format registrations made by the
%  FTXT module from the list of supported formats.
%
%  The format of the UnregisterFTXTImage method is:
%
%      UnregisterFTXTImage(void)
%
*/
ModuleExport void UnregisterFTXTImage(void)
{
  (void) UnregisterMagickInfo("FTXT");
}

static MagickBooleanType WriteFTXTString(Image *image,const char *text,
  unsigned char *pixels,size_t *extent)
{
  size_t
    count,
    length;

  count=strlen(text);
  while (count != 0)
  {
    length=MagickMin(count,MagickPathExtent-*extent);
    (void) memcpy(pixels+*extent,text,length);
    *extent+=length;
    text+=length;
    count-=length;
    if (*extent == MagickPathExtent)
      {
        if (WriteBlob(image,*extent,pixels) != (ssize_t) *extent)
          return(MagickFalse);
        *extent=0;
      }
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e F T X T I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteFTXTImage() writes an image in the formatted text image format.
%
%  The format of the WriteFTXTImage method is:
%
%      MagickBooleanType WriteFTXTImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteFTXTImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  char
    buffer[MagickPathExtent],
    chSep,
    sSuff[2];

  const char
    *sChSep,
    *sFmt;

  const Quantum
    *p;

  int
    precision;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  PixelInfo
    pixel;

  size_t
    extent;

  unsigned char
    *pixels;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  scene=0;
  precision=GetMagickPrecision();
  sFmt=GetImageArtifact(image,"ftxt:format");
  if (sFmt == (const char *) NULL)
    sFmt=dfltFmt;
  sChSep=GetImageArtifact(image,"ftxt:chsep");
  if (sChSep == (const char *) NULL)
    sChSep=dfltChSep;
  if ((sChSep[0]==chEsc) && ((sChSep[1] == 'n') || (sChSep[1] == 'N')))
    chSep='\n';
  else
    chSep=sChSep[0];
  sSuff[0]='\0';
  sSuff[1]='\0';
  pixels=(unsigned char *) AcquireQuantumMemory(MagickPathExtent,
    sizeof(*pixels));
  if (pixels == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  extent=0;

  do
  {
    ssize_t
      x,
      y;

    GetPixelInfo(image,&pixel);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      if (status == MagickFalse)
        break;
      p=GetVirtualPixels(image,0,y,image->columns,1,exception);
      if (p == (const Quantum *) NULL)
        {
          status=MagickFalse;
          break;
        }
      for (x=0; x < (ssize_t) image->columns; x++)
      {
        const char
          *pFmt;

        long double
          valMult;

        MagickBooleanType
          fltHexFmt,
          hexFmt;

        if (status == MagickFalse)
          break;
        valMult=1.0;
        hexFmt=MagickFalse;
        fltHexFmt=MagickFalse;
        pFmt=sFmt;
        while (*pFmt != '\0')
        {
          if (status == MagickFalse)
            break;
          if (*pFmt == chEsc)
          {
            pFmt++;
            switch (*pFmt)
            {
              case 'x':
              {
                FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) x);
                status=WriteFTXTString(image,buffer,pixels,&extent);
                break;
              }
              case 'y':
              {
                FormatLocaleString(buffer,MagickPathExtent,"%.20g",(double) y);
                status=WriteFTXTString(image,buffer,pixels,&extent);
                break;
              }
              case 'n':
              {
                status=WriteFTXTString(image,"\n",pixels,&extent);
                break;
              }
              case chEsc:
              {
                status=WriteFTXTString(image,"\\",pixels,&extent);
                break;
              }
              case 'c':
              case 'v':
              case 'p':
              case 'o':
              case 'h':
              case 'f':
              {
                char
                  sSep[2];

                ssize_t
                  i;

                hexFmt=MagickFalse;
                fltHexFmt=MagickFalse;
                sSuff[0]='\0';
                if (*pFmt == 'c')
                  valMult=1.0;
                else if (*pFmt == 'v')
                  valMult=1.0;
                else if (*pFmt == 'p')
                  {
                    valMult=100*QuantumScale;
                    sSuff[0]='%';
                  }
                else if (*pFmt == 'o')
                  valMult=QuantumScale;
                else if (*pFmt == 'h')
                  {
                    valMult=1.0;
                    hexFmt=MagickTrue;
                  }
                else if (*pFmt == 'f')
                  {
                    valMult=1.0;
                    fltHexFmt=MagickTrue;
                  }
                /* Output all "-channel" channels. */
                sSep[0]=sSep[1]='\0';
                for (i=0; i < (ssize_t) GetPixelChannels (image); i++)
                {
                  PixelChannel
                    channel;

                  PixelTrait
                    traits;

                  if (status == MagickFalse)
                    break;
                  channel=GetPixelChannelChannel(image,i);
                  traits=GetPixelChannelTraits(image,channel);
                  if ((traits & UpdatePixelTrait) != UpdatePixelTrait)
                    continue;
                  if (hexFmt)
                    {
                      long double
                        hv;

                      hv=((long double) p[i])+0.5L;
                      if (hv < 0.0L)
                        hv=0.0L;
                      FormatLocaleString(buffer,MagickPathExtent,"%s#%llx",sSep,
                        (unsigned long long) hv);
                    }
                  else if (fltHexFmt)
                    FormatLocaleString(buffer,MagickPathExtent,"%s%a",sSep,
                      (double) p[i]);
                  else
                    FormatLocaleString(buffer,MagickPathExtent,"%s%.*g%s",sSep,
                      precision,(double) (p[i]*valMult),sSuff);
                  status=WriteFTXTString(image,buffer,pixels,&extent);
                  sSep[0]=chSep;
                }
                break;
              }
              case 'j':
              {
                /* Output nothing. */
                break;
              }
              case 's':
              {
                GetPixelInfoPixel(image,p,&pixel);
                GetColorTuple(&pixel,MagickFalse,buffer);
                status=WriteFTXTString(image,buffer,pixels,&extent);
                break;
              }
              case 'H':
              {
                GetPixelInfoPixel(image,p,&pixel);
                /*
                  For reading, QueryColorCompliance misreads 64 bit/channel
                  hex colours, so when writing we ensure it is at most 32 bits.
                */
                if (pixel.depth > 32)
                  pixel.depth=32;
                GetColorTuple(&pixel,MagickTrue,buffer);
                status=WriteFTXTString(image,buffer,pixels,&extent);
                break;
              }
              default:
                break;
            }
          }
          else
            {
              /* Not an escape char. */
              buffer[0]=*pFmt;
              buffer[1]='\0';
              status=WriteFTXTString(image,buffer,pixels,&extent);
            }
          if (*pFmt)
            pFmt++;
        }
        p+=(ptrdiff_t) GetPixelChannels(image);
      }
      if ((status != MagickFalse) && (extent != 0))
        {
          if (WriteBlob(image,extent,pixels) != (ssize_t) extent)
            status=MagickFalse;
          extent=0;
        }
      if ((status != MagickFalse) && (image->previous == (Image *) NULL) &&
          (image->progress_monitor != (MagickProgressMonitor) NULL) &&
          (QuantumTick(y,image->rows) != MagickFalse))
        {
          status=image->progress_monitor(SaveImageTag,y,image->rows,
            image->client_data);
          if (status == MagickFalse)
            break;
        }
    }
    if (status == MagickFalse)
      break;
    if (GetNextImageInList(image) == (Image*)NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
    scene++;
  } while (image_info->adjoin != MagickFalse);
  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}
