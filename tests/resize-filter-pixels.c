/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                          Resize Filter Pixel Tests                          %
%                                                                             %
%                                                                             %
%  Copyright 1999 ImageMagick Studio LLC, a non-profit organization           %
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

/*
  Compare every Lanczos sample with the stored Q16 non-HDRI reference.

  Each filter tap is pixel += weight * sample in double, then ClampToQuantum
  adds one half and truncates. The tiled kernels use fused multiply-adds on
  CPUs with AVX2 and FMA, and a compiler may contract the scalar loops the
  same way, as GCC and Clang do on arm64. A fused and a separate multiply
  plus add can fall on opposite sides of the half-quantum boundary, so a
  filtered sample may differ by one quantum. The 20x15 to 8x4 case leaves
  fewer columns than one 16-column tile, so its vertical pass runs only the
  scalar loop. The 40x30 cases run the four-channel kernel (CMYK), the
  stride 3 and 4 gathers (red-only and cyan-only), and the generic gather
  (gray). A copied channel is never filtered and must match exactly: the
  alpha of the copy case, the write mask of the mask case, and the channels
  outside the channel mask of the red-only and cyan-only cases.

  tests/validate.c cannot express this contract. It round-trips image
  formats and compares them with root-mean-square error, accepting a
  normalized error of 0.003. At Q16 that is an RMSE of about 197 quanta,
  so both a one-quantum boundary and a missed exact sample stay inside
  the fuzz. The program clears MAGICK_CONFIGURE_PATH before initialization
  so host policy and logging files cannot change the oracle. Other quantum
  depths and HDRI builds exit 77 so the test harness can skip them.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MagickCore/MagickCore.h>

static Image *AcquireCanvas(const size_t columns,const size_t rows,
  ExceptionInfo *exception)
{
  Image
    *image;

  ImageInfo
    *image_info;

  image_info=AcquireImageInfo();
  image=AcquireImage(image_info,exception);
  image_info=DestroyImageInfo(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageExtent(image,columns,rows,exception) == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  return(image);
}

static MagickBooleanType FillPattern(Image *image,ExceptionInfo *exception)
{
  CacheView
    *view;

  ssize_t
    y;

  size_t
    nch;

  nch=GetPixelChannels(image);
  view=AcquireAuthenticCacheView(image,exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    Quantum
      *q;

    ssize_t
      x;

    q=QueueCacheViewAuthenticPixels(view,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      {
        view=DestroyCacheView(view);
        return(MagickFalse);
      }
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      ssize_t
        c;

      for (c=0; c < (ssize_t) nch; c++)
        q[x*(ssize_t) nch+c]=(Quantum) ((x*1103+y*917+c*257) % 65536);
    }
    if (SyncCacheViewAuthenticPixels(view,exception) == MagickFalse)
      {
        view=DestroyCacheView(view);
        return(MagickFalse);
      }
  }
  view=DestroyCacheView(view);
  return(MagickTrue);
}

static MagickBooleanType DumpResize(FILE *out,const char *name,Image *image,
  const size_t columns,const size_t rows,ExceptionInfo *exception)
{
  CacheView
    *view;

  Image
    *resized;

  ssize_t
    y;

  size_t
    nch;

  resized=ResizeImage(image,columns,rows,LanczosFilter,exception);
  if (resized == (Image *) NULL)
    {
      (void) fprintf(stderr,"resize failed: %s\n",name);
      return(MagickFalse);
    }
  nch=GetPixelChannels(resized);
  (void) fprintf(out,"CASE %s %lu %lu %lu\n",name,
    (unsigned long) resized->columns,(unsigned long) resized->rows,
    (unsigned long) nch);
  view=AcquireVirtualCacheView(resized,exception);
  for (y=0; y < (ssize_t) resized->rows; y++)
  {
    const Quantum
      *p;

    ssize_t
      x;

    p=GetCacheViewVirtualPixels(view,0,y,resized->columns,1,exception);
    if (p == (const Quantum *) NULL)
      {
        view=DestroyCacheView(view);
        resized=DestroyImage(resized);
        return(MagickFalse);
      }
    for (x=0; x < (ssize_t) resized->columns; x++)
    {
      ssize_t
        c;

      for (c=0; c < (ssize_t) nch; c++)
      {
        if ((x != 0) || (c != 0))
          (void) fputc(' ',out);
        (void) fprintf(out,"%u",(unsigned int) p[x*(ssize_t) nch+c]);
      }
    }
    (void) fputc('\n',out);
  }
  view=DestroyCacheView(view);
  resized=DestroyImage(resized);
  return(MagickTrue);
}

typedef struct _CopiedChannel
{
  const char
    *name;

  size_t
    channel;
} CopiedChannel;

static const CopiedChannel
  CopiedChannels[] =
  {
    { "rgba-copy-alpha", 3 },
    { "rgb-write-mask", 3 },
    { "rgb-red-only", 1 },
    { "rgb-red-only", 2 },
    { "cmyk-cyan-only", 1 },
    { "cmyk-cyan-only", 2 },
    { "cmyk-cyan-only", 3 }
  };

static int CaseTolerance(const char *name,const size_t channel)
{
  size_t
    i;

  for (i=0; i < (sizeof(CopiedChannels)/sizeof(*CopiedChannels)); i++)
    if ((strcmp(name,CopiedChannels[i].name) == 0) &&
        (channel == CopiedChannels[i].channel))
      return(0);
  return(1);
}

static const char *NextToken(char **cursor)
{
  char
    *start;

  while ((**cursor == ' ') || (**cursor == '\t') || (**cursor == '\r') ||
         (**cursor == '\n'))
    (*cursor)++;
  if (**cursor == '\0')
    return((const char *) NULL);
  start=*cursor;
  while ((**cursor != '\0') && (**cursor != ' ') && (**cursor != '\t') &&
         (**cursor != '\r') && (**cursor != '\n'))
    (*cursor)++;
  if (**cursor != '\0')
    {
      **cursor='\0';
      (*cursor)++;
    }
  return(start);
}

static MagickBooleanType ReadDumpLine(FILE *file,char *line,const size_t length)
{
  if (fgets(line,(int) length,file) == (char *) NULL)
    return(MagickFalse);
  if ((strchr(line,'\n') == (char *) NULL) && (feof(file) == 0))
    {
      (void) fprintf(stderr,"dump line exceeds %lu bytes\n",
        (unsigned long) length);
      return(MagickFalse);
    }
  return(MagickTrue);
}

static MagickBooleanType CompareDump(FILE *actual,FILE *reference,int *max_diff)
{
  char
    actual_line[16384],
    name[128],
    reference_line[16384];

  int
    tolerance;

  size_t
    channels,
    row,
    sample;

  *max_diff=0;
  tolerance=0;
  channels=0;
  row=0;
  sample=0;
  name[0]='\0';
  while (ReadDumpLine(actual,actual_line,sizeof(actual_line)) != MagickFalse)
  {
    char
      *actual_cursor,
      *reference_cursor;

    if (ReadDumpLine(reference,reference_line,
        sizeof(reference_line)) == MagickFalse)
      {
        (void) fprintf(stderr,"actual dump is longer than the reference\n");
        return(MagickFalse);
      }
    if (strncmp(actual_line,"CASE ",5) == 0)
      {
        unsigned long
          case_channels,
          case_columns,
          case_rows;

        if (strcmp(actual_line,reference_line) != 0)
          {
            (void) fprintf(stderr,"case header mismatch:\n%s%s",actual_line,
              reference_line);
            return(MagickFalse);
          }
        if (sscanf(actual_line,"CASE %127s %lu %lu %lu",name,&case_columns,
              &case_rows,&case_channels) != 4)
          {
            (void) fprintf(stderr,"malformed case header: %s",actual_line);
            return(MagickFalse);
          }
        channels=(size_t) case_channels;
        row=0;
        continue;
      }
    actual_cursor=actual_line;
    reference_cursor=reference_line;
    sample=0;
    for ( ; ; )
    {
      char
        *end;

      const char
        *actual_token,
        *reference_token;

      long
        actual_value,
        reference_value;

      int
        difference;

      size_t
        channel,
        column;

      actual_token=NextToken(&actual_cursor);
      reference_token=NextToken(&reference_cursor);
      if ((actual_token == (const char *) NULL) &&
          (reference_token == (const char *) NULL))
        break;
      if ((actual_token == (const char *) NULL) ||
          (reference_token == (const char *) NULL))
        {
          (void) fprintf(stderr,"%s row %lu has a different sample count\n",
            name,(unsigned long) row);
          return(MagickFalse);
        }
      actual_value=strtol(actual_token,&end,10);
      if (*end != '\0')
        {
          (void) fprintf(stderr,"%s row %lu has a non-numeric sample\n",name,
            (unsigned long) row);
          return(MagickFalse);
        }
      reference_value=strtol(reference_token,&end,10);
      if (*end != '\0')
        {
          (void) fprintf(stderr,
            "reference %s row %lu has a non-numeric sample\n",name,
            (unsigned long) row);
          return(MagickFalse);
        }
      column=0;
      channel=sample;
      if (channels != 0)
        {
          column=sample/channels;
          channel=sample % channels;
        }
      tolerance=CaseTolerance(name,channel);
      difference=(int) labs(actual_value-reference_value);
      if (difference > *max_diff)
        *max_diff=difference;
      if (difference > tolerance)
        {
          (void) fprintf(stderr,"%s pixel %lu,%lu channel %lu differs by %d "
            "(actual %ld, reference %ld, tolerance %d)\n",
            name,(unsigned long) column,(unsigned long) row,
            (unsigned long) channel,difference,actual_value,reference_value,
            tolerance);
          return(MagickFalse);
        }
      sample++;
    }
    row++;
  }
  if (ReadDumpLine(reference,reference_line,sizeof(reference_line)) !=
      MagickFalse)
    {
      (void) fprintf(stderr,"reference dump is longer than the actual dump\n");
      return(MagickFalse);
    }
  return(MagickTrue);
}

int main(int argc,char **argv)
{
  ExceptionInfo
    *exception;

  FILE
    *out;

  Image
    *image,
    *mask;

  MagickBooleanType
    status;

  if ((argc != 2) && (argc != 3))
    {
      (void) fprintf(stderr,"usage: %s dump.txt [reference.txt]\n",argv[0]);
      return(1);
    }
#if MAGICKCORE_QUANTUM_DEPTH != 16 || defined(MAGICKCORE_HDRI_SUPPORT)
  (void) fprintf(stderr,"skip: Q16 non-HDRI build required\n");
  return(77);
#endif
#if defined(_WIN32)
  (void) _putenv("MAGICK_CONFIGURE_PATH=");
#else
  (void) unsetenv("MAGICK_CONFIGURE_PATH");
#endif
  MagickCoreGenesis(argv[0],MagickTrue);
  (void) SetMagickResourceLimit(ThreadResource,1);
  exception=AcquireExceptionInfo();
  out=fopen(argv[1],"w");
  if (out == (FILE *) NULL)
    return(1);
  status=MagickTrue;

  image=AcquireCanvas(64,48,exception);
  status&=FillPattern(image,exception);
  status&=DumpResize(out,"rgb-down-64x48",image,16,12,exception);
  image=DestroyImage(image);

  image=AcquireCanvas(20,15,exception);
  status&=FillPattern(image,exception);
  status&=DumpResize(out,"rgb-up-20x15",image,50,38,exception);
  /*
    Shrink width more than height so the vertical pass sees 8 columns.
    The horizontal pass runs first and uses the 4-row kernel.
  */
  status&=DumpResize(out,"rgb-remainder-20x15",image,8,4,exception);
  image=DestroyImage(image);

  image=AcquireCanvas(18,14,exception);
  status&=SetImageAlphaChannel(image,ActivateAlphaChannel,exception);
  status&=FillPattern(image,exception);
  status&=DumpResize(out,"rgba-18x14",image,9,7,exception);
  (void) SetPixelChannelTraits(image,AlphaPixelChannel,CopyPixelTrait);
  status&=DumpResize(out,"rgba-copy-alpha",image,9,7,exception);
  (void) SetPixelChannelTraits(image,AlphaPixelChannel,
    (PixelTrait) (UpdatePixelTrait | BlendPixelTrait));
  image->alpha_trait=BlendPixelTrait;
  status&=DumpResize(out,"rgba-blend-alpha",image,9,7,exception);
  image=DestroyImage(image);

  image=AcquireCanvas(17,13,exception);
  status&=SetImageColorspace(image,GRAYColorspace,exception);
  status&=FillPattern(image,exception);
  status&=DumpResize(out,"gray-17x13",image,8,6,exception);
  image=DestroyImage(image);

  /*
    The 40x30 to 20x15 cases filter the 40 source columns vertically first:
    two full 16-column tiles and an 8-column remainder.
  */
  image=AcquireCanvas(40,30,exception);
  status&=SetImageColorspace(image,GRAYColorspace,exception);
  status&=FillPattern(image,exception);
  status&=DumpResize(out,"gray-40x30",image,20,15,exception);
  image=DestroyImage(image);

  image=AcquireCanvas(40,30,exception);
  status&=FillPattern(image,exception);
  (void) SetImageChannelMask(image,RedChannel);
  status&=DumpResize(out,"rgb-red-only",image,20,15,exception);
  image=DestroyImage(image);

  image=AcquireCanvas(40,30,exception);
  status&=SetImageColorspace(image,CMYKColorspace,exception);
  status&=FillPattern(image,exception);
  status&=DumpResize(out,"cmyk-40x30",image,20,15,exception);
  (void) SetImageChannelMask(image,CyanChannel);
  status&=DumpResize(out,"cmyk-cyan-only",image,20,15,exception);
  image=DestroyImage(image);

  image=AcquireCanvas(20,15,exception);
  status&=FillPattern(image,exception);
  mask=AcquireCanvas(20,15,exception);
  status&=SetImageColorspace(mask,GRAYColorspace,exception);
  status&=FillPattern(mask,exception);
  status&=SetImageMask(image,WritePixelMask,mask,exception);
  mask=DestroyImage(mask);
  status&=DumpResize(out,"rgb-write-mask",image,10,8,exception);
  image=DestroyImage(image);

  (void) fclose(out);
  if (exception->severity >= ErrorException)
    CatchException(exception);
  exception=DestroyExceptionInfo(exception);
  MagickCoreTerminus();
  if (status == MagickFalse)
    return(1);
  if (argc == 3)
    {
      FILE
        *actual,
        *reference;

      int
        max_diff;

      actual=fopen(argv[1],"r");
      reference=fopen(argv[2],"r");
      if ((actual == (FILE *) NULL) || (reference == (FILE *) NULL))
        {
          (void) fprintf(stderr,"unable to open dump or reference\n");
          if (actual != (FILE *) NULL)
            (void) fclose(actual);
          if (reference != (FILE *) NULL)
            (void) fclose(reference);
          return(1);
        }
      status=CompareDump(actual,reference,&max_diff);
      (void) fclose(actual);
      (void) fclose(reference);
      (void) fprintf(stderr,"max_quantum_diff=%d\n",max_diff);
      if (status == MagickFalse)
        return(1);
    }
  return(0);
}
