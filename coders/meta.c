/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        M   M  EEEEE  TTTTT   AAA                            %
%                        MM MM  E        T    A   A                           %
%                        M M M  EEE      T    AAAAA                           %
%                        M   M  E        T    A   A                           %
%                        M   M  EEEEE    T    A   A                           %
%                                                                             %
%                                                                             %
%                    Read/Write Embedded Image Profiles.                      %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                 July 2001                                   %
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
%
*/

/*
  Include declarations.
*/
#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/channel.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/profile.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteMETAImage(const ImageInfo *,Image *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M E T A                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMETA() returns MagickTrue if the image format type, identified by the
%  magick string, is META.
%
%  The format of the IsMETA method is:
%
%      MagickBooleanType IsMETA(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
%
*/
#ifdef IMPLEMENT_IS_FUNCTION
static MagickBooleanType IsMETA(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"8BIM",4) == 0)
    return(MagickTrue);
  if (LocaleNCompare((char *) magick,"APP1",4) == 0)
    return(MagickTrue);
  if (LocaleNCompare((char *) magick,"\034\002",2) == 0)
    return(MagickTrue);
  return(MagickFalse);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M E T A I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMETAImage() reads a META image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMETAImage method is:
%
%      Image *ReadMETAImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  Decompression code contributed by Kyle Shorter.
%
%  A description of each parameter follows:
%
%    o image: Method ReadMETAImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static const struct
{
  const unsigned char
    len;

  const char
    code[7],
    val;
} html_codes[] = {
#ifdef HANDLE_GT_LT
  { 4,"&lt;",'<' },
  { 4,"&gt;",'>' },
#endif
  { 5,"&amp;",'&' },
  { 6,"&quot;",'"' },
  { 6,"&apos;",'\''}
};

static int stringnicmp(const char *p,const char *q,size_t n)
{
  register ssize_t
    i,
    j;

  if (p == q)
    return(0);
  if (p == (char *) NULL)
    return(-1);
  if (q == (char *) NULL)
    return(1);
  while ((*p != '\0') && (*q != '\0'))
  {
    if ((*p == '\0') || (*q == '\0'))
      break;
    i=(*p);
    if (islower(i))
      i=LocaleUppercase(i);
    j=(*q);
    if (islower(j))
      j=LocaleUppercase(j);
    if (i != j)
      break;
    n--;
    if (n == 0)
      break;
    p++;
    q++;
  }
  return(LocaleUppercase((int) *p)-LocaleUppercase((int) *q));
}

static size_t convertHTMLcodes(char *s)
{
  int
    value;

  register size_t
    i;

  size_t
    length;

  length=0;
  for (i=0; (i < 7U) && (s[i] != '\0'); i++)
    if (s[i] == ';')
      {
        length=i+1;
        break;
      }
  if ((length == 0) || (s == (char *) NULL) || (*s == '\0'))
    return(0);
  if ((length > 3) && (s[1] == '#') && (sscanf(s,"&#%d;",&value) == 1))
    {
      size_t
        o;

      o=3;
      while (s[o] != ';')
      {
        o++;
        if (o > 5)
          break;
      }
      if (o < 6)
        (void) memmove(s+1,s+1+o,strlen(s+1+o)+1);
      *s=value;
      return(o);
    }
  for (i=0; i < (ssize_t) (sizeof(html_codes)/sizeof(html_codes[0])); i++)
  {
    if (html_codes[i].len <= (ssize_t) length)
      if (stringnicmp(s,html_codes[i].code,(size_t) (html_codes[i].len)) == 0)
        {
          (void) memmove(s+1,s+html_codes[i].len,strlen(s+html_codes[i].len)+1);
          *s=html_codes[i].val;
          return(html_codes[i].len-1);
        }
  }
  return(0);
}

static char *super_fgets(char **b, int *blen, Image *file)
{
  int
    c,
    len;

  unsigned char
    *p,
    *q;

  len=*blen;
  p=(unsigned char *) (*b);
  for (q=p; ; q++)
  {
    c=ReadBlobByte(file);
    if (c == EOF || c == '\n')
      break;
    if ((q-p+1) >= (int) len)
      {
        int
          tlen;

        tlen=q-p;
        len<<=1;
        p=(unsigned char *) ResizeQuantumMemory(p,(size_t) len+2UL,sizeof(*p));
        *b=(char *) p;
        if (p == (unsigned char *) NULL)
          break;
        q=p+tlen;
      }
    *q=(unsigned char) c;
  }
  *blen=0;
  if (p != (unsigned char *) NULL)
    {
      int
        tlen;

      tlen=q-p;
      if (tlen == 0)
        return (char *) NULL;
      p[tlen] = '\0';
      *blen=++tlen;
    }
  return((char *) p);
}

#define IPTC_ID 1028
#define THUMBNAIL_ID 1033

static ssize_t parse8BIM(Image *ifile, Image *ofile)
{
  char
    brkused,
    quoted,
    *line,
    *token,
    *newstr,
    *name;

  int
    state,
    next;

  unsigned char
    dataset;

  unsigned int
    recnum;

  int
    inputlen = MagickPathExtent;

  MagickOffsetType
    savedpos,
    currentpos;

  ssize_t
    savedolen = 0L,
    outputlen = 0L;

  TokenInfo
    *token_info;

  dataset = 0;
  recnum = 0;
  line = (char *) AcquireQuantumMemory((size_t) inputlen,sizeof(*line));
  if (line == (char *) NULL)
    return(-1);
  newstr = name = token = (char *) NULL;
  savedpos = 0;
  token_info=AcquireTokenInfo();
  while (super_fgets(&line,&inputlen,ifile)!=NULL)
  {
    state=0;
    next=0;

    token=(char *) AcquireQuantumMemory((size_t) inputlen,sizeof(*token));
    if (token == (char *) NULL)
      break;
    newstr=(char *) AcquireQuantumMemory((size_t) inputlen,sizeof(*newstr));
    if (newstr == (char *) NULL)
      break;
    while (Tokenizer(token_info,0,token,(size_t) inputlen,line,"","=","\"",0,
           &brkused,&next,&quoted)==0)
    {
      if (state == 0)
        {
          int
            state,
            next;

          char
            brkused,
            quoted;

          state=0;
          next=0;
          while (Tokenizer(token_info,0,newstr,(size_t) inputlen,token,"","#",
            "", 0,&brkused,&next,&quoted)==0)
          {
            switch (state)
            {
              case 0:
                if (strcmp(newstr,"8BIM")==0)
                  dataset = 255;
                else
                  dataset = (unsigned char) StringToLong(newstr);
                break;
              case 1:
                recnum = (unsigned int) StringToUnsignedLong(newstr);
                break;
              case 2:
                name=(char *) AcquireQuantumMemory(strlen(newstr)+MagickPathExtent,
                  sizeof(*name));
                if (name)
                  (void) strcpy(name,newstr);
                break;
            }
            state++;
          }
        }
      else
        if (state == 1)
          {
            int
              next;

            ssize_t
              len;

            char
              brkused,
              quoted;

            next=0;
            len = (ssize_t) strlen(token);
            while (Tokenizer(token_info,0,newstr,(size_t) inputlen,token,"","&",
              "",0,&brkused,&next,&quoted)==0)
            {
              if (brkused && next > 0)
                {
                  size_t
                    codes_length;

                  char
                    *s = &token[next-1];

                  codes_length=convertHTMLcodes(s);
                  if ((ssize_t) codes_length > len)
                    len=0;
                  else
                    len-=codes_length;
                }
            }

            if (dataset == 255)
              {
                unsigned char
                  nlen = 0;

                int
                  i;

                if (savedolen > 0)
                  {
                    MagickOffsetType
                      offset;

                    ssize_t diff = outputlen - savedolen;
                    currentpos = TellBlob(ofile);
                    if (currentpos < 0)
                      {
                        line=DestroyString(line);
                        return(-1);
                      }
                    offset=SeekBlob(ofile,savedpos,SEEK_SET);
                    if (offset < 0)
                      {
                        line=DestroyString(line);
                        return(-1);
                      }
                    (void) WriteBlobMSBLong(ofile,(unsigned int) diff);
                    offset=SeekBlob(ofile,currentpos,SEEK_SET);
                    if (offset < 0)
                      {
                        line=DestroyString(line);
                        return(-1);
                      }
                    savedolen = 0L;
                  }
                if (outputlen & 1)
                  {
                    (void) WriteBlobByte(ofile,0x00);
                    outputlen++;
                  }
                (void) WriteBlobString(ofile,"8BIM");
                (void) WriteBlobMSBShort(ofile,(unsigned short) recnum);
                outputlen += 6;
                if (name)
                  nlen = (unsigned char) strlen(name);
                (void) WriteBlobByte(ofile,nlen);
                outputlen++;
                for (i=0; i<nlen; i++)
                  (void) WriteBlobByte(ofile,(unsigned char) name[i]);
                outputlen += nlen;
                if ((nlen & 0x01) == 0)
                  {
                    (void) WriteBlobByte(ofile,0x00);
                    outputlen++;
                  }
                if (recnum != IPTC_ID)
                  {
                    (void) WriteBlobMSBLong(ofile, (unsigned int) len);
                    outputlen += 4;

                    next=0;
                    outputlen += len;
                    while (len-- > 0)
                      (void) WriteBlobByte(ofile,(unsigned char) token[next++]);

                    if (outputlen & 1)
                      {
                        (void) WriteBlobByte(ofile,0x00);
                        outputlen++;
                      }
                  }
                else
                  {
                    /* patch in a fake length for now and fix it later */
                    savedpos = TellBlob(ofile);
                    if (savedpos < 0)
                      return(-1);
                    (void) WriteBlobMSBLong(ofile,0xFFFFFFFFU);
                    outputlen += 4;
                    savedolen = outputlen;
                  }
              }
            else
              {
                if (len <= 0x7FFF)
                  {
                    (void) WriteBlobByte(ofile,0x1c);
                    (void) WriteBlobByte(ofile,(unsigned char) dataset);
                    (void) WriteBlobByte(ofile,(unsigned char) (recnum & 0xff));
                    (void) WriteBlobMSBShort(ofile,(unsigned short) len);
                    outputlen += 5;
                    next=0;
                    outputlen += len;
                    while (len-- > 0)
                      (void) WriteBlobByte(ofile,(unsigned char) token[next++]);
                  }
              }
          }
      state++;
    }
    if (token != (char *) NULL)
      token=DestroyString(token);
    if (newstr != (char *) NULL)
      newstr=DestroyString(newstr);
    if (name != (char *) NULL)
      name=DestroyString(name);
  }
  token_info=DestroyTokenInfo(token_info);
  if (token != (char *) NULL)
    token=DestroyString(token);
  if (newstr != (char *) NULL)
    newstr=DestroyString(newstr);
  if (name != (char *) NULL)
    name=DestroyString(name);
  line=DestroyString(line);
  if (savedolen > 0)
    {
      MagickOffsetType
        offset;

      ssize_t diff = outputlen - savedolen;

      currentpos = TellBlob(ofile);
      if (currentpos < 0)
        return(-1);
      offset=SeekBlob(ofile,savedpos,SEEK_SET);
      if (offset < 0)
        return(-1);
      (void) WriteBlobMSBLong(ofile,(unsigned int) diff);
      offset=SeekBlob(ofile,currentpos,SEEK_SET);
      if (offset < 0)
        return(-1);
      savedolen = 0L;
    }
  return outputlen;
}

static char *super_fgets_w(char **b, int *blen, Image *file)
{
  int
    c,
    len;

  unsigned char
    *p,
    *q;

  len=*blen;
  p=(unsigned char *) (*b);
  for (q=p; ; q++)
  {
    c=ReadBlobLSBSignedShort(file);
    if ((c == -1) || (c == '\n'))
      break;
   if (EOFBlob(file))
      break;
   if ((q-p+1) >= (int) len)
      {
        int
          tlen;

        tlen=q-p;
        len<<=1;
        p=(unsigned char *) ResizeQuantumMemory(p,(size_t) (len+2),sizeof(*p));
        *b=(char *) p;
        if (p == (unsigned char *) NULL)
          break;
        q=p+tlen;
      }
    *q=(unsigned char) c;
  }
  *blen=0;
  if ((*b) != (char *) NULL)
    {
      int
        tlen;

      tlen=q-p;
      if (tlen == 0)
        return (char *) NULL;
      p[tlen] = '\0';
      *blen=++tlen;
    }
  return((char *) p);
}

static ssize_t parse8BIMW(Image *ifile, Image *ofile)
{
  char
    brkused,
    quoted,
    *line,
    *token,
    *newstr,
    *name;

  int
    state,
    next;

  unsigned char
    dataset;

  unsigned int
    recnum;

  int
    inputlen = MagickPathExtent;

  ssize_t
    savedolen = 0L,
    outputlen = 0L;

  MagickOffsetType
    savedpos,
    currentpos;

  TokenInfo
    *token_info;

  dataset = 0;
  recnum = 0;
  line=(char *) AcquireQuantumMemory((size_t) inputlen,sizeof(*line));
  if (line == (char *) NULL)
    return(-1);
  newstr = name = token = (char *) NULL;
  savedpos = 0;
  token_info=AcquireTokenInfo();
  while (super_fgets_w(&line,&inputlen,ifile) != NULL)
  {
    state=0;
    next=0;

    token=(char *) AcquireQuantumMemory((size_t) inputlen,sizeof(*token));
    if (token == (char *) NULL)
      break;
    newstr=(char *) AcquireQuantumMemory((size_t) inputlen,sizeof(*newstr));
    if (newstr == (char *) NULL)
      break;
    while (Tokenizer(token_info,0,token,(size_t) inputlen,line,"","=","\"",0,
      &brkused,&next,&quoted)==0)
    {
      if (state == 0)
        {
          int
            state,
            next;

          char
            brkused,
            quoted;

          state=0;
          next=0;
          while (Tokenizer(token_info,0,newstr,(size_t) inputlen,token,"","#",
            "",0,&brkused,&next,&quoted)==0)
          {
            switch (state)
            {
              case 0:
                if (strcmp(newstr,"8BIM")==0)
                  dataset = 255;
                else
                  dataset = (unsigned char) StringToLong(newstr);
                break;
              case 1:
                recnum=(unsigned int) StringToUnsignedLong(newstr);
                break;
              case 2:
                name=(char *) AcquireQuantumMemory(strlen(newstr)+MagickPathExtent,
                  sizeof(*name));
                if (name)
                  (void) CopyMagickString(name,newstr,strlen(newstr)+MagickPathExtent);
                break;
            }
            state++;
          }
        }
      else
        if (state == 1)
          {
            int
              next;

            ssize_t
              len;

            char
              brkused,
              quoted;

            next=0;
            len = (ssize_t) strlen(token);
            while (Tokenizer(token_info,0,newstr,(size_t) inputlen,token,"","&",
              "",0,&brkused,&next,&quoted)==0)
            {
              if (brkused && next > 0)
                {
                  size_t
                    codes_length;

                  char
                    *s = &token[next-1];

                  codes_length=convertHTMLcodes(s);
                  if ((ssize_t) codes_length > len)
                    len=0;
                  else
                    len-=codes_length;
                }
            }

            if (dataset == 255)
              {
                unsigned char
                  nlen = 0;

                int
                  i;

                if (savedolen > 0)
                  {
                    MagickOffsetType
                      offset;

                    ssize_t diff = outputlen - savedolen;
                    currentpos = TellBlob(ofile);
                    if (currentpos < 0)
                      return(-1);
                    offset=SeekBlob(ofile,savedpos,SEEK_SET);
                    if (offset < 0)
                      return(-1);
                    (void) WriteBlobMSBLong(ofile,(unsigned int) diff);
                    offset=SeekBlob(ofile,currentpos,SEEK_SET);
                    if (offset < 0)
                      return(-1);
                    savedolen = 0L;
                  }
                if (outputlen & 1)
                  {
                    (void) WriteBlobByte(ofile,0x00);
                    outputlen++;
                  }
                (void) WriteBlobString(ofile,"8BIM");
                (void) WriteBlobMSBShort(ofile,(unsigned short) recnum);
                outputlen += 6;
                if (name)
                  nlen = (unsigned char) strlen(name);
                (void) WriteBlobByte(ofile,(unsigned char) nlen);
                outputlen++;
                for (i=0; i<nlen; i++)
                  (void) WriteBlobByte(ofile,(unsigned char) name[i]);
                outputlen += nlen;
                if ((nlen & 0x01) == 0)
                  {
                    (void) WriteBlobByte(ofile,0x00);
                    outputlen++;
                  }
                if (recnum != IPTC_ID)
                  {
                    (void) WriteBlobMSBLong(ofile,(unsigned int) len);
                    outputlen += 4;

                    next=0;
                    outputlen += len;
                    while (len--)
                      (void) WriteBlobByte(ofile,(unsigned char) token[next++]);

                    if (outputlen & 1)
                      {
                        (void) WriteBlobByte(ofile,0x00);
                        outputlen++;
                      }
                  }
                else
                  {
                    /* patch in a fake length for now and fix it later */
                    savedpos = TellBlob(ofile);
                    if (savedpos < 0)
                      return(-1);
                    (void) WriteBlobMSBLong(ofile,0xFFFFFFFFU);
                    outputlen += 4;
                    savedolen = outputlen;
                  }
              }
            else
              {
                if (len <= 0x7FFF)
                  {
                    (void) WriteBlobByte(ofile,0x1c);
                    (void) WriteBlobByte(ofile,dataset);
                    (void) WriteBlobByte(ofile,(unsigned char) (recnum & 0xff));
                    (void) WriteBlobMSBShort(ofile,(unsigned short) len);
                    outputlen += 5;
                    next=0;
                    outputlen += len;
                    while (len--)
                      (void) WriteBlobByte(ofile,(unsigned char) token[next++]);
                  }
              }
          }
      state++;
    }
    if (token != (char *) NULL)
      token=DestroyString(token);
    if (newstr != (char *) NULL)
      newstr=DestroyString(newstr);
    if (name != (char *) NULL)
      name=DestroyString(name);
  }
  token_info=DestroyTokenInfo(token_info);
  if (token != (char *) NULL)
    token=DestroyString(token);
  if (newstr != (char *) NULL)
    newstr=DestroyString(newstr);
  if (name != (char *) NULL)
    name=DestroyString(name);
  line=DestroyString(line);
  if (savedolen > 0)
    {
      MagickOffsetType
        offset;

      ssize_t diff = outputlen - savedolen;

      currentpos = TellBlob(ofile);
      if (currentpos < 0)
        return(-1);
      offset=SeekBlob(ofile,savedpos,SEEK_SET);
      if (offset < 0)
        return(-1);
      (void) WriteBlobMSBLong(ofile,(unsigned int) diff);
      offset=SeekBlob(ofile,currentpos,SEEK_SET);
      if (offset < 0)
        return(-1);
      savedolen = 0L;
    }
  return(outputlen);
}

/* some defines for the different JPEG block types */
#define M_SOF0  0xC0            /* Start Of Frame N */
#define M_SOF1  0xC1            /* N indicates which compression process */
#define M_SOF2  0xC2            /* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5            /* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8
#define M_EOI   0xD9            /* End Of Image (end of datastream) */
#define M_SOS   0xDA            /* Start Of Scan (begins compressed data) */
#define M_APP0  0xe0
#define M_APP1  0xe1
#define M_APP2  0xe2
#define M_APP3  0xe3
#define M_APP4  0xe4
#define M_APP5  0xe5
#define M_APP6  0xe6
#define M_APP7  0xe7
#define M_APP8  0xe8
#define M_APP9  0xe9
#define M_APP10 0xea
#define M_APP11 0xeb
#define M_APP12 0xec
#define M_APP13 0xed
#define M_APP14 0xee
#define M_APP15 0xef

static int jpeg_transfer_1(Image *ifile, Image *ofile)
{
  int c;

  c = ReadBlobByte(ifile);
  if (c == EOF)
    return EOF;
  (void) WriteBlobByte(ofile,(unsigned char) c);
  return c;
}

#if defined(future)
static int jpeg_skip_1(Image *ifile)
{
  int c;

  c = ReadBlobByte(ifile);
  if (c == EOF)
    return EOF;
  return c;
}
#endif

static int jpeg_read_remaining(Image *ifile, Image *ofile)
{
   int c;

  while ((c = jpeg_transfer_1(ifile, ofile)) != EOF)
    continue;
  return M_EOI;
}

static int jpeg_skip_variable(Image *ifile, Image *ofile)
{
  unsigned int  length;
  int c1,c2;

  if ((c1 = jpeg_transfer_1(ifile, ofile)) == EOF)
    return M_EOI;
  if ((c2 = jpeg_transfer_1(ifile, ofile)) == EOF)
    return M_EOI;

  length = (((unsigned int) c1) << 8) + ((unsigned int) c2);
  length -= 2;

  while (length--)
    if (jpeg_transfer_1(ifile, ofile) == EOF)
      return M_EOI;

  return 0;
}

static int jpeg_skip_variable2(Image *ifile, Image *ofile)
{
  unsigned int  length;
  int c1,c2;

  (void) ofile;
  if ((c1 = ReadBlobByte(ifile)) == EOF) return M_EOI;
  if ((c2 = ReadBlobByte(ifile)) == EOF) return M_EOI;

  length = (((unsigned int) c1) << 8) + ((unsigned int) c2);
  length -= 2;

  while (length--)
    if (ReadBlobByte(ifile) == EOF)
      return M_EOI;

  return 0;
}

static int jpeg_nextmarker(Image *ifile, Image *ofile)
{
  int c;

  /* transfer anything until we hit 0xff */
  do
  {
    c = ReadBlobByte(ifile);
    if (c == EOF)
      return M_EOI; /* we hit EOF */
    else
      if (c != 0xff)
        (void) WriteBlobByte(ofile,(unsigned char) c);
  } while (c != 0xff);

  /* get marker byte, swallowing possible padding */
  do
  {
    c = ReadBlobByte(ifile);
    if (c == EOF)
      return M_EOI; /* we hit EOF */
  } while (c == 0xff);

  return c;
}

#if defined(future)
static int jpeg_skip_till_marker(Image *ifile, int marker)
{
  int c, i;

  do
  {
    /* skip anything until we hit 0xff */
    i = 0;
    do
    {
      c = ReadBlobByte(ifile);
      i++;
      if (c == EOF)
        return M_EOI; /* we hit EOF */
    } while (c != 0xff);

    /* get marker byte, swallowing possible padding */
    do
    {
      c = ReadBlobByte(ifile);
      if (c == EOF)
        return M_EOI; /* we hit EOF */
    } while (c == 0xff);
  } while (c != marker);
  return c;
}
#endif

/* Embed binary IPTC data into a JPEG image. */
static int jpeg_embed(Image *ifile, Image *ofile, Image *iptc)
{
  unsigned int marker;
  unsigned int done = 0;
  unsigned int len;
  int inx;

  if (jpeg_transfer_1(ifile, ofile) != 0xFF)
    return 0;
  if (jpeg_transfer_1(ifile, ofile) != M_SOI)
    return 0;

  while (done == MagickFalse)
  {
    marker=(unsigned int) jpeg_nextmarker(ifile, ofile);
    if (marker == M_EOI)
      { /* EOF */
        break;
      }
    else
      {
        if (marker != M_APP13)
          {
            (void) WriteBlobByte(ofile,0xff);
            (void) WriteBlobByte(ofile,(unsigned char) marker);
          }
      }

    switch (marker)
    {
      case M_APP13:
        /* we are going to write a new APP13 marker, so don't output the old one */
        jpeg_skip_variable2(ifile, ofile);
        break;

      case M_APP0:
        /* APP0 is in each and every JPEG, so when we hit APP0 we insert our new APP13! */
        jpeg_skip_variable(ifile, ofile);

        if (iptc != (Image *) NULL)
          {
            char
              psheader[] = "\xFF\xED\0\0Photoshop 3.0\0" "8BIM\x04\x04\0\0\0\0";

            len=(unsigned int) GetBlobSize(iptc);
            if (len & 1)
              len++; /* make the length even */
            psheader[2]=(char) ((len+16)>>8);
            psheader[3]=(char) ((len+16)&0xff);
            for (inx = 0; inx < 18; inx++)
              (void) WriteBlobByte(ofile,(unsigned char) psheader[inx]);
            jpeg_read_remaining(iptc, ofile);
            len=(unsigned int) GetBlobSize(iptc);
            if (len & 1)
              (void) WriteBlobByte(ofile,0);
          }
        break;

      case M_SOS:
        /* we hit data, no more marker-inserting can be done! */
        jpeg_read_remaining(ifile, ofile);
        done = 1;
        break;

      default:
        jpeg_skip_variable(ifile, ofile);
        break;
    }
  }
  return 1;
}

/* handle stripping the APP13 data out of a JPEG */
#if defined(future)
static void jpeg_strip(Image *ifile, Image *ofile)
{
  unsigned int marker;

  marker = jpeg_skip_till_marker(ifile, M_SOI);
  if (marker == M_SOI)
  {
    (void) WriteBlobByte(ofile,0xff);
    (void) WriteBlobByte(ofile,M_SOI);
    jpeg_read_remaining(ifile, ofile);
  }
}

/* Extract any APP13 binary data into a file. */
static int jpeg_extract(Image *ifile, Image *ofile)
{
  unsigned int marker;
  unsigned int done = 0;

  if (jpeg_skip_1(ifile) != 0xff)
    return 0;
  if (jpeg_skip_1(ifile) != M_SOI)
    return 0;

  while (done == MagickFalse)
  {
    marker = jpeg_skip_till_marker(ifile, M_APP13);
    if (marker == M_APP13)
      {
        marker = jpeg_nextmarker(ifile, ofile);
        break;
      }
  }
  return 1;
}
#endif

static inline void CopyBlob(Image *source,Image *destination)
{
  ssize_t
    i;

  unsigned char
    *buffer;

  ssize_t
    count,
    length;

  buffer=(unsigned char *) AcquireQuantumMemory(MagickMaxBufferExtent,
    sizeof(*buffer));
  if (buffer != (unsigned char *) NULL)
    {
      i=0;
      while ((length=ReadBlob(source,MagickMaxBufferExtent,buffer)) != 0)
      {
        count=0;
        for (i=0; i < (ssize_t) length; i+=count)
        {
          count=WriteBlob(destination,(size_t) (length-i),buffer+i);
          if (count <= 0)
            break;
        }
        if (i < (ssize_t) length)
          break;
      }
      buffer=(unsigned char *) RelinquishMagickMemory(buffer);
    }
}

static Image *ReadMETAImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *buff,
    *image;

  MagickBooleanType
    status;

  StringInfo
    *profile;

  size_t
    length;

  void
    *blob;

  /*
    Open file containing binary metadata
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
  image->columns=1;
  image->rows=1;
  if (SetImageBackgroundColor(image,exception) == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  length=1;
  if (LocaleNCompare(image_info->magick,"8BIM",4) == 0)
    {
      /*
        Read 8BIM binary metadata.
      */
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      blob=(unsigned char *) AcquireQuantumMemory(length,sizeof(unsigned char));
      if (blob == (unsigned char *) NULL)
        {
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) memset(blob,0,length);
      AttachBlob(buff->blob,blob,length);
      if (LocaleCompare(image_info->magick,"8BIMTEXT") == 0)
        {
          length=(size_t) parse8BIM(image, buff);
          if (length == 0)
            {
              blob=DetachBlob(buff->blob);
              blob=(unsigned char *) RelinquishMagickMemory(blob);
              buff=DestroyImage(buff);
              ThrowReaderException(CorruptImageError,"CorruptImage");
            }
          if (length & 1)
            (void) WriteBlobByte(buff,0x0);
        }
      else if (LocaleCompare(image_info->magick,"8BIMWTEXT") == 0)
        {
          length=(size_t) parse8BIMW(image, buff);
          if (length == 0)
            {
              blob=DetachBlob(buff->blob);
              blob=(unsigned char *) RelinquishMagickMemory(blob);
              buff=DestroyImage(buff);
              ThrowReaderException(CorruptImageError,"CorruptImage");
            }
          if (length & 1)
            (void) WriteBlobByte(buff,0x0);
        }
      else
        CopyBlob(image,buff);
      profile=BlobToStringInfo(GetBlobStreamData(buff),(size_t)
        GetBlobSize(buff));
      if (profile == (StringInfo *) NULL)
        {
          blob=DetachBlob(buff->blob);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      status=SetImageProfile(image,"8bim",profile,exception);
      profile=DestroyStringInfo(profile);
      blob=DetachBlob(buff->blob);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      buff=DestroyImage(buff);
      if (status == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if (LocaleNCompare(image_info->magick,"APP1",4) == 0)
    {
      char
        name[MagickPathExtent];

      (void) FormatLocaleString(name,MagickPathExtent,"APP%d",1);
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      blob=(unsigned char *) AcquireQuantumMemory(length,sizeof(unsigned char));
      if (blob == (unsigned char *) NULL)
        {
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      AttachBlob(buff->blob,blob,length);
      if (LocaleCompare(image_info->magick,"APP1JPEG") == 0)
        {
          Image
            *iptc;

          int
            result;

          if (image_info->profile == (void *) NULL)
            {
              blob=DetachBlob(buff->blob);
              blob=(unsigned char *) RelinquishMagickMemory(blob);
              buff=DestroyImage(buff);
              ThrowReaderException(CoderError,"NoIPTCProfileAvailable");
            }
          profile=CloneStringInfo((StringInfo *) image_info->profile);
          iptc=AcquireImage((ImageInfo *) NULL,exception);
          if (iptc == (Image *) NULL)
            {
              blob=DetachBlob(buff->blob);
              blob=(unsigned char *) RelinquishMagickMemory(blob);
              buff=DestroyImage(buff);
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            }
          AttachBlob(iptc->blob,GetStringInfoDatum(profile),
            GetStringInfoLength(profile));
          result=jpeg_embed(image,buff,iptc);
          blob=DetachBlob(iptc->blob);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          iptc=DestroyImage(iptc);
          if (result == 0)
            {
              buff=DestroyImage(buff);
              ThrowReaderException(CoderError,"JPEGEmbeddingFailed");
            }
        }
      else
        CopyBlob(image,buff);
      profile=BlobToStringInfo(GetBlobStreamData(buff),(size_t)
        GetBlobSize(buff));
      if (profile == (StringInfo *) NULL)
        {
          blob=DetachBlob(buff->blob);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      status=SetImageProfile(image,name,profile,exception);
      profile=DestroyStringInfo(profile);
      blob=DetachBlob(buff->blob);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      buff=DestroyImage(buff);
      if (status == MagickFalse)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  if ((LocaleCompare(image_info->magick,"ICC") == 0) ||
      (LocaleCompare(image_info->magick,"ICM") == 0))
    {
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      blob=(unsigned char *) AcquireQuantumMemory(length,sizeof(unsigned char));
      if (blob == (unsigned char *) NULL)
        {
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      AttachBlob(buff->blob,blob,length);
      CopyBlob(image,buff);
      profile=BlobToStringInfo(GetBlobStreamData(buff),(size_t)
        GetBlobSize(buff));
      if (profile == (StringInfo *) NULL)
        {
          blob=DetachBlob(buff->blob);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) SetImageProfile(image,"icc",profile,exception);
      profile=DestroyStringInfo(profile);
      blob=DetachBlob(buff->blob);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      buff=DestroyImage(buff);
    }
  if (LocaleCompare(image_info->magick,"IPTC") == 0)
    {
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      blob=(unsigned char *) AcquireQuantumMemory(length,sizeof(unsigned char));
      if (blob == (unsigned char *) NULL)
        {
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      AttachBlob(buff->blob,blob,length);
      CopyBlob(image,buff);
      profile=BlobToStringInfo(GetBlobStreamData(buff),(size_t)
        GetBlobSize(buff));
      if (profile == (StringInfo *) NULL)
        {
          blob=DetachBlob(buff->blob);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) SetImageProfile(image,"iptc",profile,exception);
      profile=DestroyStringInfo(profile);
      blob=DetachBlob(buff->blob);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      buff=DestroyImage(buff);
    }
  if (LocaleCompare(image_info->magick,"XMP") == 0)
    {
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      blob=(unsigned char *) AcquireQuantumMemory(length,sizeof(unsigned char));
      if (blob == (unsigned char *) NULL)
        {
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      AttachBlob(buff->blob,blob,length);
      CopyBlob(image,buff);
      profile=BlobToStringInfo(GetBlobStreamData(buff),(size_t)
        GetBlobSize(buff));
      if (profile == (StringInfo *) NULL)
        {
          blob=DetachBlob(buff->blob);
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          buff=DestroyImage(buff);
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        }
      (void) SetImageProfile(image,"xmp",profile,exception);
      profile=DestroyStringInfo(profile);
      blob=DetachBlob(buff->blob);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      buff=DestroyImage(buff);
    }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M E T A I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterMETAImage() adds attributes for the META image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMETAImage method is:
%
%      size_t RegisterMETAImage(void)
%
*/
ModuleExport size_t RegisterMETAImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("META","8BIM","Photoshop resource format");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","8BIMTEXT","Photoshop resource text format");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","8BIMWTEXT",
    "Photoshop resource wide text format");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","APP1","Raw application information");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","APP1JPEG","Raw JPEG binary data");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","EXIF","Exif digital camera binary data");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","XMP","Adobe XML metadata");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","ICM","ICC Color Profile");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","ICC","ICC Color Profile");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","IPTC","IPTC Newsphoto");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","IPTCTEXT","IPTC Newsphoto text format");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("META","IPTCWTEXT","IPTC Newsphoto text format");
  entry->decoder=(DecodeImageHandler *) ReadMETAImage;
  entry->encoder=(EncodeImageHandler *) WriteMETAImage;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderStealthFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M E T A I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterMETAImage() removes format registrations made by the
%  META module from the list of supported formats.
%
%  The format of the UnregisterMETAImage method is:
%
%      UnregisterMETAImage(void)
%
*/
ModuleExport void UnregisterMETAImage(void)
{
  (void) UnregisterMagickInfo("8BIM");
  (void) UnregisterMagickInfo("8BIMTEXT");
  (void) UnregisterMagickInfo("8BIMWTEXT");
  (void) UnregisterMagickInfo("EXIF");
  (void) UnregisterMagickInfo("APP1");
  (void) UnregisterMagickInfo("APP1JPEG");
  (void) UnregisterMagickInfo("ICCTEXT");
  (void) UnregisterMagickInfo("ICM");
  (void) UnregisterMagickInfo("ICC");
  (void) UnregisterMagickInfo("IPTC");
  (void) UnregisterMagickInfo("IPTCTEXT");
  (void) UnregisterMagickInfo("IPTCWTEXT");
  (void) UnregisterMagickInfo("XMP");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M E T A I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteMETAImage() writes a META image to a file.
%
%  The format of the WriteMETAImage method is:
%
%      MagickBooleanType WriteMETAImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  Compression code contributed by Kyle Shorter.
%
%  A description of each parameter follows:
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: A pointer to a Image structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static size_t GetIPTCStream(unsigned char **info,size_t length)
{
  int
    c;

  register ssize_t
    i;

  register unsigned char
    *p;

  size_t
    extent,
    info_length;

  unsigned int
    marker;

  size_t
    tag_length;

  p=(*info);
  extent=length;
  if ((*p == 0x1c) && (*(p+1) == 0x02))
    return(length);
  /*
    Extract IPTC from 8BIM resource block.
  */
  while (extent >= 12)
  {
    if (strncmp((const char *) p,"8BIM",4))
      break;
    p+=4;
    extent-=4;
    marker=(unsigned int) (*p) << 8 | *(p+1);
    p+=2;
    extent-=2;
    c=*p++;
    extent--;
    c|=0x01;
    if ((size_t) c >= extent)
      break;
    p+=c;
    extent-=c;
    if (extent < 4)
      break;
    tag_length=(((size_t) *p) << 24) | (((size_t) *(p+1)) << 16) |
      (((size_t) *(p+2)) << 8) | ((size_t) *(p+3));
    p+=4;
    extent-=4;
    if (tag_length > extent)
      break;
    if (marker == IPTC_ID)
      {
        *info=p;
        return(tag_length);
      }
    if ((tag_length & 0x01) != 0)
      tag_length++;
    p+=tag_length;
    extent-=tag_length;
  }
  /*
    Find the beginning of the IPTC info.
  */
  p=(*info);
  tag_length=0;
iptc_find:
  info_length=0;
  marker=MagickFalse;
  while (length != 0)
  {
    c=(*p++);
    length--;
    if (length == 0)
      break;
    if (c == 0x1c)
      {
        p--;
        *info=p; /* let the caller know were it is */
        break;
      }
  }
  /*
    Determine the length of the IPTC info.
  */
  while (length != 0)
  {
    c=(*p++);
    length--;
    if (length == 0)
      break;
    if (c == 0x1c)
      marker=MagickTrue;
    else
      if (marker)
        break;
      else
        continue;
    info_length++;
    /*
      Found the 0x1c tag; skip the dataset and record number tags.
    */
    c=(*p++); /* should be 2 */
    length--;
    if (length == 0)
      break;
    if ((info_length == 1) && (c != 2))
      goto iptc_find;
    info_length++;
    c=(*p++); /* should be 0 */
    length--;
    if (length == 0)
      break;
    if ((info_length == 2) && (c != 0))
      goto iptc_find;
    info_length++;
    /*
      Decode the length of the block that follows - ssize_t or short format.
    */
    c=(*p++);
    length--;
    if (length == 0)
      break;
    info_length++;
    if ((c & 0x80) != 0)
      {
        /*
          Long format.
        */
        tag_length=0;
        for (i=0; i < 4; i++)
        {
          tag_length<<=8;
          tag_length|=(*p++);
          length--;
          if (length == 0)
            break;
          info_length++;
        }
      }
    else
      {
        /*
          Short format.
        */
        tag_length=((long) c) << 8;
        c=(*p++);
        length--;
        if (length == 0)
          break;
        info_length++;
        tag_length|=(long) c;
      }
    if (tag_length > (length+1))
      break;
    p+=tag_length;
    length-=tag_length;
    if (length == 0)
      break;
    info_length+=tag_length;
  }
  return(info_length);
}

static void formatString(Image *ofile, const char *s, int len)
{
  char
    temp[MagickPathExtent];

  (void) WriteBlobByte(ofile,'"');
  for (; len > 0; len--, s++) {
    int c = (*s) & 255;
    switch (c) {
    case '&':
      (void) WriteBlobString(ofile,"&amp;");
      break;
#ifdef HANDLE_GT_LT
    case '<':
      (void) WriteBlobString(ofile,"&lt;");
      break;
    case '>':
      (void) WriteBlobString(ofile,"&gt;");
      break;
#endif
    case '"':
      (void) WriteBlobString(ofile,"&quot;");
      break;
    default:
      if (isprint(c))
        (void) WriteBlobByte(ofile,(unsigned char) *s);
      else
        {
          (void) FormatLocaleString(temp,MagickPathExtent,"&#%d;", c & 255);
          (void) WriteBlobString(ofile,temp);
        }
      break;
    }
  }
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
  (void) WriteBlobString(ofile,"\"\r\n");
#else
#if defined(macintosh)
  (void) WriteBlobString(ofile,"\"\r");
#else
  (void) WriteBlobString(ofile,"\"\n");
#endif
#endif
}

typedef struct _tag_spec
{
  const short
    id;

  const char
    *name;
} tag_spec;

static const tag_spec tags[] = {
  { 5, "Image Name" },
  { 7, "Edit Status" },
  { 10, "Priority" },
  { 15, "Category" },
  { 20, "Supplemental Category" },
  { 22, "Fixture Identifier" },
  { 25, "Keyword" },
  { 30, "Release Date" },
  { 35, "Release Time" },
  { 40, "Special Instructions" },
  { 45, "Reference Service" },
  { 47, "Reference Date" },
  { 50, "Reference Number" },
  { 55, "Created Date" },
  { 60, "Created Time" },
  { 65, "Originating Program" },
  { 70, "Program Version" },
  { 75, "Object Cycle" },
  { 80, "Byline" },
  { 85, "Byline Title" },
  { 90, "City" },
  { 92, "Sub-Location" },
  { 95, "Province State" },
  { 100, "Country Code" },
  { 101, "Country" },
  { 103, "Original Transmission Reference" },
  { 105, "Headline" },
  { 110, "Credit" },
  { 115, "Source" },
  { 116, "Copyright String" },
  { 120, "Caption" },
  { 121, "Image Orientation" },
  { 122, "Caption Writer" },
  { 131, "Local Caption" },
  { 200, "Custom Field 1" },
  { 201, "Custom Field 2" },
  { 202, "Custom Field 3" },
  { 203, "Custom Field 4" },
  { 204, "Custom Field 5" },
  { 205, "Custom Field 6" },
  { 206, "Custom Field 7" },
  { 207, "Custom Field 8" },
  { 208, "Custom Field 9" },
  { 209, "Custom Field 10" },
  { 210, "Custom Field 11" },
  { 211, "Custom Field 12" },
  { 212, "Custom Field 13" },
  { 213, "Custom Field 14" },
  { 214, "Custom Field 15" },
  { 215, "Custom Field 16" },
  { 216, "Custom Field 17" },
  { 217, "Custom Field 18" },
  { 218, "Custom Field 19" },
  { 219, "Custom Field 20" }
};

static int formatIPTC(Image *ifile, Image *ofile)
{
  char
    temp[MagickPathExtent];

  unsigned int
    foundiptc,
    tagsfound;

  unsigned char
    recnum,
    dataset;

  unsigned char
    *readable,
    *str;

  ssize_t
    tagindx,
    taglen;

  int
    i,
    tagcount = (int) (sizeof(tags) / sizeof(tags[0]));

  int
    c;

  foundiptc = 0; /* found the IPTC-Header */
  tagsfound = 0; /* number of tags found */

  c = ReadBlobByte(ifile);
  while (c != EOF)
  {
    if (c == 0x1c)
      foundiptc = 1;
    else
      {
        if (foundiptc)
          return(-1);
        else
          {
            c=0;
            continue;
          }
      }

    /* we found the 0x1c tag and now grab the dataset and record number tags */
    c = ReadBlobByte(ifile);
    if (c == EOF)
      return(-1);
    dataset = (unsigned char) c;
    c = ReadBlobByte(ifile);
    if (c == EOF)
      return(-1);
    recnum = (unsigned char) c;
    /* try to match this record to one of the ones in our named table */
    for (i=0; i< tagcount; i++)
    {
      if (tags[i].id == (short) recnum)
          break;
    }
    if (i < tagcount)
      readable = (unsigned char *) tags[i].name;
    else
      readable = (unsigned char *) "";
    /*
      We decode the length of the block that follows - ssize_t or short fmt.
    */
    c=ReadBlobByte(ifile);
    if (c == EOF)
      return(-1);
    if (c & (unsigned char) 0x80)
      return(0);
    else
      {
        int
          c0;

        c0=ReadBlobByte(ifile);
        if (c0 == EOF)
          return(-1);
        taglen = (c << 8) | c0;
      }
    if (taglen < 0)
      return(-1);
    /* make a buffer to hold the tag datand snag it from the input stream */
    str=(unsigned char *) AcquireQuantumMemory((size_t) (taglen+
      MagickPathExtent),sizeof(*str));
    if (str == (unsigned char *) NULL)
      return(0);
    for (tagindx=0; tagindx<taglen; tagindx++)
    {
      c=ReadBlobByte(ifile);
      if (c == EOF)
        {
          str=(unsigned char *) RelinquishMagickMemory(str);
          return(-1);
        }
      str[tagindx] = (unsigned char) c;
    }
    str[taglen] = 0;

    /* now finish up by formatting this binary data into ASCII equivalent */
    if (strlen((char *)readable) > 0)
      (void) FormatLocaleString(temp,MagickPathExtent,"%d#%d#%s=",
        (unsigned int) dataset, (unsigned int) recnum, readable);
    else
      (void) FormatLocaleString(temp,MagickPathExtent,"%d#%d=",
        (unsigned int) dataset,(unsigned int) recnum);
    (void) WriteBlobString(ofile,temp);
    formatString( ofile, (char *)str, taglen );
    str=(unsigned char *) RelinquishMagickMemory(str);

    tagsfound++;

    c=ReadBlobByte(ifile);
  }
  return((int) tagsfound);
}

static int readWordFromBuffer(char **s, ssize_t *len)
{
  unsigned char
    buffer[2];

  int
    i,
    c;

  for (i=0; i<2; i++)
  {
    c = *(*s)++; (*len)--;
    if (*len < 0) return -1;
    buffer[i] = (unsigned char) c;
  }
  return (((int) buffer[ 0 ]) <<  8) |
         (((int) buffer[ 1 ]));
}

static int formatIPTCfromBuffer(Image *ofile, char *s, ssize_t len)
{
  char
    temp[MagickPathExtent];

  unsigned int
    foundiptc,
    tagsfound;

  unsigned char
    recnum,
    dataset;

  unsigned char
    *readable,
    *str;

  ssize_t
    tagindx,
    taglen;

  int
    i,
    tagcount = (int) (sizeof(tags) / sizeof(tags[0]));

  int
    c;

  foundiptc = 0; /* found the IPTC-Header */
  tagsfound = 0; /* number of tags found */

  while (len > 0)
  {
    c = *s++; len--;
    if (c == 0x1c)
      foundiptc = 1;
    else
      {
        if (foundiptc)
          return -1;
        else
          continue;
      }
    /*
      We found the 0x1c tag and now grab the dataset and record number tags.
    */
    c = *s++; len--;
    if (len < 0) return -1;
    dataset = (unsigned char) c;
    c = *s++; len--;
    if (len < 0) return -1;
    recnum = (unsigned char) c;
    /* try to match this record to one of the ones in our named table */
    for (i=0; i< tagcount; i++)
      if (tags[i].id == (short) recnum)
        break;
    if (i < tagcount)
      readable=(unsigned char *) tags[i].name;
    else
      readable=(unsigned char *) "";
    /*
      We decode the length of the block that follows - ssize_t or short fmt.
    */
    c=(*s++);
    len--;
    if (len < 0)
      return(-1);
    if (c & (unsigned char) 0x80)
      return(0);
    else
      {
        s--;
        len++;
        taglen=readWordFromBuffer(&s, &len);
      }
    if (taglen < 0)
      return(-1);
    if (taglen > 65535)
      return(-1);
    /* make a buffer to hold the tag datand snag it from the input stream */
    str=(unsigned char *) AcquireQuantumMemory((size_t) (taglen+
      MagickPathExtent),sizeof(*str));
    if (str == (unsigned char *) NULL)
      {
        (void) printf("MemoryAllocationFailed");
        return 0;
      }
    for (tagindx=0; tagindx<taglen; tagindx++)
    {
      c = *s++; len--;
      if (len < 0)
        {
          str=(unsigned char *) RelinquishMagickMemory(str);
          return(-1);
        }
      str[tagindx]=(unsigned char) c;
    }
    str[taglen]=0;

    /* now finish up by formatting this binary data into ASCII equivalent */
    if (strlen((char *)readable) > 0)
      (void) FormatLocaleString(temp,MagickPathExtent,"%d#%d#%s=",
        (unsigned int) dataset,(unsigned int) recnum, readable);
    else
      (void) FormatLocaleString(temp,MagickPathExtent,"%d#%d=",
        (unsigned int) dataset,(unsigned int) recnum);
    (void) WriteBlobString(ofile,temp);
    formatString( ofile, (char *)str, taglen );
    str=(unsigned char *) RelinquishMagickMemory(str);

    tagsfound++;
  }
  return ((int) tagsfound);
}

static int format8BIM(Image *ifile, Image *ofile)
{
  char
    temp[MagickPathExtent];

  unsigned int
    foundOSType;

  int
    ID,
    resCount,
    i,
    c;

  ssize_t
    count;

  unsigned char
    *PString,
    *str;

  resCount=0;
  foundOSType=0; /* found the OSType */
  (void) foundOSType;
  c=ReadBlobByte(ifile);
  while (c != EOF)
  {
    if (c == '8')
      {
        unsigned char
          buffer[5];

        buffer[0]=(unsigned char) c;
        for (i=1; i<4; i++)
        {
          c=ReadBlobByte(ifile);
          if (c == EOF)
            return(-1);
          buffer[i] = (unsigned char) c;
        }
        buffer[4]=0;
        if (strcmp((const char *)buffer, "8BIM") == 0)
          foundOSType=1;
        else
          continue;
      }
    else
      {
        c=ReadBlobByte(ifile);
        continue;
      }
    /*
      We found the OSType (8BIM) and now grab the ID, PString, and Size fields.
    */
    ID=ReadBlobMSBSignedShort(ifile);
    if (ID < 0)
      return(-1);
    {
      unsigned char
        plen;

      c=ReadBlobByte(ifile);
      if (c == EOF)
        return(-1);
      plen = (unsigned char) c;
      PString=(unsigned char *) AcquireQuantumMemory((size_t) (plen+
        MagickPathExtent),sizeof(*PString));
      if (PString == (unsigned char *) NULL)
        return 0;
      for (i=0; i<plen; i++)
      {
        c=ReadBlobByte(ifile);
        if (c == EOF)
          {
            PString=(unsigned char *) RelinquishMagickMemory(PString);
            return -1;
          }
        PString[i] = (unsigned char) c;
      }
      PString[ plen ] = 0;
      if ((plen & 0x01) == 0)
      {
        c=ReadBlobByte(ifile);
        if (c == EOF)
          {
            PString=(unsigned char *) RelinquishMagickMemory(PString);
            return -1;
          }
      }
    }
    count=(ssize_t) ReadBlobMSBSignedLong(ifile);
    if ((count < 0) || (count > (ssize_t) GetBlobSize(ifile)))
      {
        PString=(unsigned char *) RelinquishMagickMemory(PString);
        return -1;
      }
    /* make a buffer to hold the data and snag it from the input stream */
    str=(unsigned char *) AcquireQuantumMemory((size_t) count+1,sizeof(*str));
    if (str == (unsigned char *) NULL)
      {
        PString=(unsigned char *) RelinquishMagickMemory(PString);
        return 0;
      }
    for (i=0; i < (ssize_t) count; i++)
    {
      c=ReadBlobByte(ifile);
      if (c == EOF)
        {
          str=(unsigned char *) RelinquishMagickMemory(str);
          PString=(unsigned char *) RelinquishMagickMemory(PString);
          return -1;
        }
      str[i]=(unsigned char) c;
    }

    /* we currently skip thumbnails, since it does not make
     * any sense preserving them in a real world application
     */
    if (ID != THUMBNAIL_ID)
      {
        /* now finish up by formatting this binary data into
         * ASCII equivalent
         */
        if (strlen((const char *)PString) > 0)
          (void) FormatLocaleString(temp,MagickPathExtent,"8BIM#%d#%s=",ID,
            PString);
        else
          (void) FormatLocaleString(temp,MagickPathExtent,"8BIM#%d=",ID);
        (void) WriteBlobString(ofile,temp);
        if (ID == IPTC_ID)
          {
            formatString(ofile, "IPTC", 4);
            formatIPTCfromBuffer(ofile, (char *)str, (ssize_t) count);
          }
        else
          formatString(ofile, (char *)str, (ssize_t) count);
      }
    str=(unsigned char *) RelinquishMagickMemory(str);
    PString=(unsigned char *) RelinquishMagickMemory(PString);
    resCount++;
    c=ReadBlobByte(ifile);
  }
  return resCount;
}

static MagickBooleanType WriteMETAImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  const StringInfo
    *profile;

  MagickBooleanType
    status;

  size_t
    length;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=0;
  if (LocaleCompare(image_info->magick,"8BIM") == 0)
    {
      /*
        Write 8BIM image.
      */
      profile=GetImageProfile(image,"8bim");
      if (profile == (StringInfo *) NULL)
        ThrowWriterException(CoderError,"No8BIMDataIsAvailable");
      assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
      (void) WriteBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  if (LocaleCompare(image_info->magick,"iptc") == 0)
    {
      size_t
        length;

      unsigned char
        *info;

      profile=GetImageProfile(image,"iptc");
      if (profile == (StringInfo *) NULL)
        profile=GetImageProfile(image,"8bim");
      if (profile == (StringInfo *) NULL)
        ThrowWriterException(CoderError,"No8BIMDataIsAvailable");
      assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      info=GetStringInfoDatum(profile);
      length=GetStringInfoLength(profile);
      length=GetIPTCStream(&info,length);
      if (length == 0)
        ThrowWriterException(CoderError,"NoIPTCProfileAvailable");
      (void) WriteBlob(image,length,info);
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  if (LocaleCompare(image_info->magick,"8BIMTEXT") == 0)
    {
      Image
        *buff;

      profile=GetImageProfile(image,"8bim");
      if (profile == (StringInfo *) NULL)
        ThrowWriterException(CoderError,"No8BIMDataIsAvailable");
      assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      AttachBlob(buff->blob,GetStringInfoDatum(profile),
        GetStringInfoLength(profile));
      format8BIM(buff,image);
      (void) DetachBlob(buff->blob);
      buff=DestroyImage(buff);
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  if (LocaleCompare(image_info->magick,"8BIMWTEXT") == 0)
    return(MagickFalse);
  if (LocaleCompare(image_info->magick,"IPTCTEXT") == 0)
    {
      Image
        *buff;

      unsigned char
        *info;

      profile=GetImageProfile(image,"8bim");
      if (profile == (StringInfo *) NULL)
        ThrowWriterException(CoderError,"No8BIMDataIsAvailable");
      info=GetStringInfoDatum(profile);
      length=GetStringInfoLength(profile);
      length=GetIPTCStream(&info,length);
      if (length == 0)
        ThrowWriterException(CoderError,"NoIPTCProfileAvailable");
      assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
      buff=AcquireImage((ImageInfo *) NULL,exception);
      if (buff == (Image *) NULL)
        ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
      AttachBlob(buff->blob,info,length);
      formatIPTC(buff,image);
      (void) DetachBlob(buff->blob);
      buff=DestroyImage(buff);
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  if (LocaleCompare(image_info->magick,"IPTCWTEXT") == 0)
    return(MagickFalse);
  if ((LocaleCompare(image_info->magick,"APP1") == 0) ||
      (LocaleCompare(image_info->magick,"EXIF") == 0) ||
      (LocaleCompare(image_info->magick,"XMP") == 0))
    {
      /*
        (void) Write APP1 image.
      */
      profile=GetImageProfile(image,image_info->magick);
      if (profile == (StringInfo *) NULL)
        ThrowWriterException(CoderError,"NoAPP1DataIsAvailable");
      assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
      (void) WriteBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  if ((LocaleCompare(image_info->magick,"ICC") == 0) ||
      (LocaleCompare(image_info->magick,"ICM") == 0))
    {
      /*
        Write ICM image.
      */
      profile=GetImageProfile(image,"icc");
      if (profile == (StringInfo *) NULL)
        ThrowWriterException(CoderError,"NoColorProfileIsAvailable");
      assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
      if (status == MagickFalse)
        return(status);
      (void) WriteBlob(image,GetStringInfoLength(profile),
        GetStringInfoDatum(profile));
      (void) CloseBlob(image);
      return(MagickTrue);
    }
  return(MagickFalse);
}
