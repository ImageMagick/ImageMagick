/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%           V   V   AAA   L      IIIII  DDDD    AAA   TTTTT  EEEEE            %
%           V   V  A   A  L        I    D   D  A   A    T    E                %
%           V   V  AAAAA  L        I    D   D  AAAAA    T    EEE              %
%            V V   A   A  L        I    D   D  A   A    T    E                %
%             V    A   A  LLLLL  IIIII  DDDD   A   A    T    EEEEE            %
%                                                                             %
%                                                                             %
%                        ImageMagick Validation Suite                         %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2001                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
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
%  see the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <locale.h>
#include "wand/MagickWand.h"
#include "magick/colorspace-private.h"
#include "magick/resource_.h"
#include "magick/string-private.h"
#include "validate.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o m p a r e C o m m a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateCompareCommand() validates the ImageMagick compare command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateCompareCommand method is:
%
%      size_t ValidateCompareCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateCompareCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MaxTextExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate compare command line program:\n");
  for (i=0; compare_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      compare_options[i]);
    (void) FormatLocaleString(command,MaxTextExtent,"%s %s %s %s",
      compare_options[i],reference_filename,reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    status=CompareImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status != MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o m p o s i t e C o m m a n d                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateCompositeCommand() validates the ImageMagick composite command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateCompositeCommand method is:
%
%      size_t ValidateCompositeCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateCompositeCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MaxTextExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate composite command line program:\n");
  for (i=0; composite_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      composite_options[i]);
    (void) FormatLocaleString(command,MaxTextExtent,"%s %s %s %s",
      reference_filename,composite_options[i],reference_filename,
      output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    status=CompositeImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status != MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e C o n v e r t C o m m a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateConvertCommand() validates the ImageMagick convert command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateConvertCommand method is:
%
%      size_t ValidateConvertCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateConvertCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MaxTextExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate convert command line program:\n");
  for (i=0; convert_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) test++,
      convert_options[i]);
    (void) FormatLocaleString(command,MaxTextExtent,"%s %s %s %s",
      reference_filename,convert_options[i],reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    status=ConvertImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status != MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I d e n t i f y C o m m a n d                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateIdentifyCommand() validates the ImageMagick identify command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateIdentifyCommand method is:
%
%      size_t ValidateIdentifyCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateIdentifyCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MaxTextExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  (void) output_filename;
  test=0;
  (void) FormatLocaleFile(stdout,"validate identify command line program:\n");
  for (i=0; identify_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) test++,
      identify_options[i]);
    (void) FormatLocaleString(command,MaxTextExtent,"%s %s",
      identify_options[i],reference_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    status=IdentifyImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status != MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
          GetMagickModule());
        (*fail)++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I m a g e F o r m a t s I n M e m o r y                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateImageFormatsInMemory() validates the ImageMagick image formats in
%  memory and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateImageFormatsInMemory method is:
%
%      size_t ValidateImageFormatsInMemory(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/

/*
  Enable this to count remaining $TMPDIR/magick-* files.  Note that the count
  includes any files left over from other runs.
*/
#undef MagickCountTempFiles

static size_t ValidateImageFormatsInMemory(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
#ifdef MagickCountTempFiles
    SystemCommand[MaxTextExtent],
    path[MaxTextExtent],
#endif
    size[MaxTextExtent];

  const MagickInfo
    *magick_info;

  double
    distortion,
    fuzz;

  Image
    *difference_image,
    *ping_image,
    *reference_image,
    *reconstruct_image;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    length;

  unsigned char
    *blob;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate image formats in memory:\n");

#ifdef MagickCountTempFiles
  (void)GetPathTemplate(path);
  /* Remove file template except for the leading "magick-" */
  path[strlen(path)-17]='\0';
  (void) FormatLocaleFile(stdout," tmp path is '%s*'\n",path);
#endif

  for (i=0; reference_formats[i].magick != (char *) NULL; i++)
  {
    magick_info=GetMagickInfo(reference_formats[i].magick,exception);
    if ((magick_info == (const MagickInfo *) NULL) ||
        (magick_info->decoder == (DecodeImageHandler *) NULL) ||
        (magick_info->encoder == (EncodeImageHandler *) NULL))
      continue;
    for (j=0; reference_types[j].type != UndefinedType; j++)
    {
      /*
        Generate reference image.
      */
      CatchException(exception);
      (void) FormatLocaleFile(stdout,"  test %.20g: %s/%s/%s/%.20g-bits",
        (double) (test++),reference_formats[i].magick,CommandOptionToMnemonic(
        MagickCompressOptions,reference_formats[i].compression),
        CommandOptionToMnemonic(MagickTypeOptions,reference_types[j].type),
        (double) reference_types[j].depth);
      (void) CopyMagickString(image_info->filename,reference_filename,
        MaxTextExtent);
      reference_image=ReadImage(image_info,exception);
      if (reference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(size,MaxTextExtent,"%.20gx%.20g",
        (double) reference_image->columns,(double) reference_image->rows);
      (void) CloneString(&image_info->size,size);
      image_info->depth=reference_types[j].depth;
      (void) FormatLocaleString(reference_image->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      status=SetImageType(reference_image,reference_types[j].type);
      InheritException(exception,&reference_image->exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      status=SetImageDepth(reference_image,reference_types[j].depth);
      InheritException(exception,&reference_image->exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      reference_image->compression=reference_formats[i].compression;
      status=WriteImage(image_info,reference_image);
      InheritException(exception,&reference_image->exception);
      reference_image=DestroyImage(reference_image);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      /*
        Ping reference image.
      */
      (void) FormatLocaleString(image_info->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      ping_image=PingImage(image_info,exception);
      if (ping_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      ping_image=DestroyImage(ping_image);
      /*
        Read reference image.
      */
      (void) FormatLocaleString(image_info->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reference_image=ReadImage(image_info,exception);
      if (reference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(reference_image->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      (void) CopyMagickString(image_info->magick,reference_formats[i].magick,
        MaxTextExtent);
      reference_image->depth=reference_types[j].depth;
      reference_image->compression=reference_formats[i].compression;
      length=8192;
      blob=ImageToBlob(image_info,reference_image,&length,exception);
      if (blob == (unsigned char *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Ping reference blob.
      */
      ping_image=PingBlob(image_info,blob,length,exception);
      if (ping_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          blob=(unsigned char *) RelinquishMagickMemory(blob);
          continue;
        }
      ping_image=DestroyImage(ping_image);
      /*
        Read reconstruct image.
      */
      (void) FormatLocaleString(image_info->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reconstruct_image=BlobToImage(image_info,blob,length,exception);
      blob=(unsigned char *) RelinquishMagickMemory(blob);
      if (reconstruct_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Compare reference to reconstruct image.
      */
      fuzz=0.0;
      if (reference_formats[i].fuzz != 0.0)
        fuzz=reference_formats[i].fuzz;
#if defined(MAGICKCORE_HDRI_SUPPORT)
      fuzz+=0.003;
#endif
      if (IssRGBColorspace(reference_image->colorspace) == MagickFalse)
        fuzz+=0.3;
      fuzz+=MagickEpsilon;
      difference_image=CompareImageChannels(reference_image,reconstruct_image,
        CompositeChannels,RootMeanSquaredErrorMetric,&distortion,exception);
      reconstruct_image=DestroyImage(reconstruct_image);
      reference_image=DestroyImage(reference_image);
      if (difference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      difference_image=DestroyImage(difference_image);
      if ((QuantumScale*distortion) > fuzz)
        {
          (void) FormatLocaleFile(stdout,"... fail (with distortion %g).\n",
            QuantumScale*distortion);
          (*fail)++;
          continue;
        }

#ifdef MagickCountTempFiles
      (void) FormatLocaleFile(stdout,"... pass, ");
      (void) fflush(stdout);
      SystemCommand[0]='\0';
      (void)strncat(SystemCommand,"echo `ls ",9);
      (void)strncat(SystemCommand,path,MaxTextExtent-31);
      (void)strncat(SystemCommand,"* | wc -w` tmp files.",20);
      (void)system(SystemCommand);
      (void) fflush(stdout);
#else
      (void) FormatLocaleFile(stdout,"... pass\n");
#endif
    }
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I m a g e F o r m a t s O n D i s k                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateImageFormatsOnDisk() validates the ImageMagick image formats on disk
%  and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateImageFormatsOnDisk method is:
%
%      size_t ValidateImageFormatsOnDisk(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateImageFormatsOnDisk(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    size[MaxTextExtent];

  const MagickInfo
    *magick_info;

  double
    distortion,
    fuzz;

  Image
    *difference_image,
    *reference_image,
    *reconstruct_image;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate image formats on disk:\n");
  for (i=0; reference_formats[i].magick != (char *) NULL; i++)
  {
    magick_info=GetMagickInfo(reference_formats[i].magick,exception);
    if ((magick_info == (const MagickInfo *) NULL) ||
        (magick_info->decoder == (DecodeImageHandler *) NULL) ||
        (magick_info->encoder == (EncodeImageHandler *) NULL))
      continue;
    for (j=0; reference_types[j].type != UndefinedType; j++)
    {
      /*
        Generate reference image.
      */
      CatchException(exception);
      (void) FormatLocaleFile(stdout,"  test %.20g: %s/%s/%s/%.20g-bits",
        (double) (test++),reference_formats[i].magick,CommandOptionToMnemonic(
        MagickCompressOptions,reference_formats[i].compression),
        CommandOptionToMnemonic(MagickTypeOptions,reference_types[j].type),
        (double) reference_types[j].depth);
      (void) CopyMagickString(image_info->filename,reference_filename,
        MaxTextExtent);
      reference_image=ReadImage(image_info,exception);
      if (reference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(size,MaxTextExtent,"%.20gx%.20g",
        (double) reference_image->columns,(double) reference_image->rows);
      (void) CloneString(&image_info->size,size);
      image_info->depth=reference_types[j].depth;
      (void) FormatLocaleString(reference_image->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      status=SetImageType(reference_image,reference_types[j].type);
      InheritException(exception,&reference_image->exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      status=SetImageDepth(reference_image,reference_types[j].depth);
      InheritException(exception,&reference_image->exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      reference_image->compression=reference_formats[i].compression;
      status=WriteImage(image_info,reference_image);
      InheritException(exception,&reference_image->exception);
      reference_image=DestroyImage(reference_image);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      /*
        Read reference image.
      */
      (void) FormatLocaleString(image_info->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reference_image=ReadImage(image_info,exception);
      if (reference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      /*
        Write reference image.
      */
      (void) FormatLocaleString(reference_image->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reference_image->depth=reference_types[j].depth;
      reference_image->compression=reference_formats[i].compression;
      status=WriteImage(image_info,reference_image);
      InheritException(exception,&reference_image->exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Read reconstruct image.
      */
      (void) FormatLocaleString(image_info->filename,MaxTextExtent,"%s:%s",
        reference_formats[i].magick,output_filename);
      reconstruct_image=ReadImage(image_info,exception);
      if (reconstruct_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Compare reference to reconstruct image.
      */
      fuzz=0.0;
      if (reference_formats[i].fuzz != 0.0)
        fuzz=reference_formats[i].fuzz;
#if defined(MAGICKCORE_HDRI_SUPPORT)
      fuzz+=0.003;
#endif
      if (IssRGBColorspace(reference_image->colorspace) == MagickFalse)
        fuzz+=0.3;
      fuzz+=MagickEpsilon;
      difference_image=CompareImageChannels(reference_image,reconstruct_image,
        CompositeChannels,RootMeanSquaredErrorMetric,&distortion,exception);
      reconstruct_image=DestroyImage(reconstruct_image);
      reference_image=DestroyImage(reference_image);
      if (difference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      difference_image=DestroyImage(difference_image);
      if ((QuantumScale*distortion) > fuzz)
        {
          (void) FormatLocaleFile(stdout,"... fail (with distortion %g).\n",
            QuantumScale*distortion);
          (*fail)++;
          continue;
        }
      (void) FormatLocaleFile(stdout,"... pass.\n");
    }
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e I m p o r t E x p o r t P i x e l s                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateImportExportPixels() validates the pixel import and export methods.
%  It returns the number of validation tests that passed and failed.
%
%  The format of the ValidateImportExportPixels method is:
%
%      size_t ValidateImportExportPixels(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateImportExportPixels(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  double
    distortion;

  Image
    *difference_image,
    *reference_image,
    *reconstruct_image;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    length;

  unsigned char
    *pixels;

  size_t
    test;

  (void) output_filename;
  test=0;
  (void) FormatLocaleFile(stdout,
    "validate the import and export of image pixels:\n");
  for (i=0; reference_map[i] != (char *) NULL; i++)
  {
    for (j=0; reference_storage[j].type != UndefinedPixel; j++)
    {
      /*
        Generate reference image.
      */
      CatchException(exception);
      (void) FormatLocaleFile(stdout,"  test %.20g: %s/%s",(double) (test++),
        reference_map[i],CommandOptionToMnemonic(MagickStorageOptions,
        reference_storage[j].type));
      (void) CopyMagickString(image_info->filename,reference_filename,
        MaxTextExtent);
      reference_image=ReadImage(image_info,exception);
      if (reference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      if (LocaleNCompare(reference_map[i],"cmy",3) == 0)
        (void) TransformImageColorspace(reference_image,CMYKColorspace);
      length=strlen(reference_map[i])*reference_image->columns*
        reference_image->rows*reference_storage[j].quantum;
      pixels=(unsigned char *) AcquireQuantumMemory(length,sizeof(*pixels));
      if (pixels == (unsigned char *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      (void) ResetMagickMemory(pixels,0,length*sizeof(*pixels));
      status=ExportImagePixels(reference_image,0,0,reference_image->columns,
        reference_image->rows,reference_map[i],reference_storage[j].type,pixels,
        exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          reference_image=DestroyImage(reference_image);
          continue;
        }
      (void) SetImageBackgroundColor(reference_image);
      status=ImportImagePixels(reference_image,0,0,reference_image->columns,
        reference_image->rows,reference_map[i],reference_storage[j].type,
        pixels);
      InheritException(exception,&reference_image->exception);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
           pixels=(unsigned char *) RelinquishMagickMemory(pixels);
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Read reconstruct image.
      */
      reconstruct_image=AcquireImage(image_info);
      (void) SetImageExtent(reconstruct_image,reference_image->columns,
        reference_image->rows);
      (void) SetImageColorspace(reconstruct_image,reference_image->colorspace);
      (void) SetImageBackgroundColor(reconstruct_image);
      status=ImportImagePixels(reconstruct_image,0,0,reconstruct_image->columns,
        reconstruct_image->rows,reference_map[i],reference_storage[j].type,
        pixels);
      InheritException(exception,&reconstruct_image->exception);
      pixels=(unsigned char *) RelinquishMagickMemory(pixels);
      if (status == MagickFalse)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          reference_image=DestroyImage(reference_image);
          continue;
        }
      /*
        Compare reference to reconstruct image.
      */
      difference_image=CompareImageChannels(reference_image,reconstruct_image,
        CompositeChannels,RootMeanSquaredErrorMetric,&distortion,exception);
      reconstruct_image=DestroyImage(reconstruct_image);
      reference_image=DestroyImage(reference_image);
      if (difference_image == (Image *) NULL)
        {
          (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
          (*fail)++;
          continue;
        }
      difference_image=DestroyImage(difference_image);
      if ((QuantumScale*distortion) > 0.0)
        {
          (void) FormatLocaleFile(stdout,"... fail (with distortion %g).\n",
            QuantumScale*distortion);
          (*fail)++;
          continue;
        }
      (void) FormatLocaleFile(stdout,"... pass.\n");
    }
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e M o n t a g e C o m m a n d                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateMontageCommand() validates the ImageMagick montage command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateMontageCommand method is:
%
%      size_t ValidateMontageCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateMontageCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MaxTextExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate montage command line program:\n");
  for (i=0; montage_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      montage_options[i]);
    (void) FormatLocaleString(command,MaxTextExtent,"%s %s %s %s",
      reference_filename,montage_options[i],reference_filename,
      output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
        (*fail)++;
        continue;
      }
    status=MontageImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status != MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
        (*fail)++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   V a l i d a t e S t r e a m C o m m a n d                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ValidateStreamCommand() validates the ImageMagick stream command line
%  program and returns the number of validation tests that passed and failed.
%
%  The format of the ValidateStreamCommand method is:
%
%      size_t ValidateStreamCommand(ImageInfo *image_info,
%        const char *reference_filename,const char *output_filename,
%        size_t *fail,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o reference_filename: the reference image filename.
%
%    o output_filename: the output image filename.
%
%    o fail: return the number of validation tests that pass.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static size_t ValidateStreamCommand(ImageInfo *image_info,
  const char *reference_filename,const char *output_filename,size_t *fail,
  ExceptionInfo *exception)
{
  char
    **arguments,
    command[MaxTextExtent];

  int
    number_arguments;

  MagickBooleanType
    status;

  register ssize_t
    i,
    j;

  size_t
    test;

  test=0;
  (void) FormatLocaleFile(stdout,"validate stream command line program:\n");
  for (i=0; stream_options[i] != (char *) NULL; i++)
  {
    CatchException(exception);
    (void) FormatLocaleFile(stdout,"  test %.20g: %s",(double) (test++),
      stream_options[i]);
    (void) FormatLocaleString(command,MaxTextExtent,"%s %s %s",
      stream_options[i],reference_filename,output_filename);
    arguments=StringToArgv(command,&number_arguments);
    if (arguments == (char **) NULL)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
        (*fail)++;
        continue;
      }
    status=StreamImageCommand(image_info,number_arguments,arguments,
      (char **) NULL,exception);
    for (j=0; j < (ssize_t) number_arguments; j++)
      arguments[j]=DestroyString(arguments[j]);
    arguments=(char **) RelinquishMagickMemory(arguments);
    if (status != MagickFalse)
      {
        (void) FormatLocaleFile(stdout,"... fail @ %s/%s/%lu.\n",
            GetMagickModule());
        (*fail)++;
        continue;
      }
    (void) FormatLocaleFile(stdout,"... pass.\n");
  }
  (void) FormatLocaleFile(stdout,
    "  summary: %.20g subtests; %.20g passed; %.20g failed.\n",(double) test,
    (double) (test-(*fail)),(double) *fail);
  return(test);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M a i n                                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

static MagickBooleanType ValidateUsage(void)
{
  const char
    **p;

  static const char
    *miscellaneous[]=
    {
      "-debug events        display copious debugging information",
      "-help                print program options",
      "-log format          format of debugging information",
      "-validate type       validation type",
      "-version             print version information",
      (char *) NULL
    },
    *settings[]=
    {
      "-regard-warnings     pay attention to warning messages",
      "-verbose             print detailed information about the image",
      (char *) NULL
    };

  (void) printf("Version: %s\n",GetMagickVersion((size_t *) NULL));
  (void) printf("Copyright: %s\n\n",GetMagickCopyright());
  (void) printf("Features: %s\n",GetMagickFeatures());
  (void) printf("Usage: %s [options ...] reference-file\n",GetClientName());
  (void) printf("\nValidate Settings:\n");
  for (p=settings; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nMiscellaneous Options:\n");
  for (p=miscellaneous; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  return(MagickTrue);
}

int main(int argc,char **argv)
{
#define DestroyValidate() \
{ \
  image_info=DestroyImageInfo(image_info); \
  exception=DestroyExceptionInfo(exception); \
}
#define ThrowValidateException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  CatchException(exception); \
  DestroyValidate(); \
  return(MagickFalse); \
}

  char
    output_filename[MaxTextExtent],
    reference_filename[MaxTextExtent],
    *option;

  double
    elapsed_time,
    user_time;

  ExceptionInfo
    *exception;

  Image
    *reference_image;

  ImageInfo
    *image_info;

  MagickBooleanType
    regard_warnings,
    status;

  MagickSizeType
    memory_resource,
    map_resource;

  register ssize_t
    i;

  TimerInfo
    *timer;

  size_t
    fail,
    iterations,
    tests;

  ValidateType
    type;

  /*
    Validate the ImageMagick image processing suite.
  */
  MagickCoreGenesis(*argv,MagickTrue);
  (void) setlocale(LC_ALL,"");
  (void) setlocale(LC_NUMERIC,"C");
  iterations=1;
  status=MagickFalse;
  type=AllValidate;
  regard_warnings=MagickFalse;
  (void) regard_warnings;
  exception=AcquireExceptionInfo();
  image_info=AcquireImageInfo();
  (void) CopyMagickString(image_info->filename,ReferenceFilename,MaxTextExtent);
  for (i=1; i < (ssize_t) argc; i++)
  {
    option=argv[i];
    if (IsCommandOption(option) == MagickFalse)
      {
        (void) CopyMagickString(image_info->filename,option,MaxTextExtent);
        continue;
      }
    switch (*(option+1))
    {
      case 'b':
      {
        if (LocaleCompare("bench",option+1) == 0)
          {
            iterations=StringToUnsignedLong(argv[++i]);
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'd':
      {
        if (LocaleCompare("debug",option+1) == 0)
          {
            (void) SetLogEventMask(argv[++i]);
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if (LocaleCompare("help",option+1) == 0)
          {
            (void) ValidateUsage();
            return(0);
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option != '+')
              (void) SetLogFormat(argv[i+1]);
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("regard-warnings",option+1) == 0)
          {
            regard_warnings=MagickTrue;
            break;
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      case 'v':
      {
        if (LocaleCompare("validate",option+1) == 0)
          {
            ssize_t
              validate;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowValidateException(OptionError,"MissingArgument",option);
            validate=ParseCommandOption(MagickValidateOptions,MagickFalse,
              argv[i]);
            if (validate < 0)
              ThrowValidateException(OptionError,"UnrecognizedValidateType",
                argv[i]);
            type=(ValidateType) validate;
            break;
          }
        if ((LocaleCompare("version",option+1) == 0) ||
            (LocaleCompare("-version",option+1) == 0))
          {
            (void) FormatLocaleFile(stdout,"Version: %s\n",
              GetMagickVersion((size_t *) NULL));
            (void) FormatLocaleFile(stdout,"Copyright: %s\n\n",
              GetMagickCopyright());
            (void) FormatLocaleFile(stdout,"Features: %s\n\n",
              GetMagickFeatures());
            return(0);
          }
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
      }
      default:
        ThrowValidateException(OptionError,"UnrecognizedOption",option)
    }
  }
  timer=(TimerInfo *) NULL;
  if (iterations > 1)
    timer=AcquireTimerInfo();
  reference_image=ReadImage(image_info,exception);
  tests=0;
  fail=0;
  if (reference_image == (Image *) NULL)
    fail++;
  else
    {
      if (LocaleCompare(image_info->filename,ReferenceFilename) == 0)
        (void) CopyMagickString(reference_image->magick,ReferenceImageFormat,
          MaxTextExtent);
      (void) AcquireUniqueFilename(reference_filename);
      (void) AcquireUniqueFilename(output_filename);
      (void) CopyMagickString(reference_image->filename,reference_filename,
        MaxTextExtent);
      status=WriteImage(image_info,reference_image);
      InheritException(exception,&reference_image->exception);
      reference_image=DestroyImage(reference_image);
      if (status == MagickFalse)
        fail++;
      else
        {
          (void) FormatLocaleFile(stdout,"Version: %s\n",
            GetMagickVersion((size_t *) NULL));
          (void) FormatLocaleFile(stdout,"Copyright: %s\n\n",
            GetMagickCopyright());
          (void) FormatLocaleFile(stdout,
            "ImageMagick Validation Suite (%s)\n\n",CommandOptionToMnemonic(
            MagickValidateOptions,(ssize_t) type));
          if ((type & CompareValidate) != 0)
            tests+=ValidateCompareCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & CompositeValidate) != 0)
            tests+=ValidateCompositeCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & ConvertValidate) != 0)
            tests+=ValidateConvertCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & FormatsInMemoryValidate) != 0)
            {
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory] ");
              tests+=ValidateImageFormatsInMemory(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory-mapped] ");
              memory_resource=SetMagickResourceLimit(MemoryResource,0);
              tests+=ValidateImageFormatsInMemory(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: disk] ");
              map_resource=SetMagickResourceLimit(MapResource,0);
              tests+=ValidateImageFormatsInMemory(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) SetMagickResourceLimit(MemoryResource,memory_resource);
              (void) SetMagickResourceLimit(MapResource,map_resource);
            }
          if ((type & FormatsOnDiskValidate) != 0)
            {
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory] ");
              tests+=ValidateImageFormatsOnDisk(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: memory-mapped] ");
              memory_resource=SetMagickResourceLimit(MemoryResource,0);
              tests+=ValidateImageFormatsOnDisk(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) FormatLocaleFile(stdout,"[pixel-cache: disk] ");
              map_resource=SetMagickResourceLimit(MapResource,0);
              tests+=ValidateImageFormatsOnDisk(image_info,reference_filename,
                output_filename,&fail,exception);
              (void) SetMagickResourceLimit(MemoryResource,memory_resource);
              (void) SetMagickResourceLimit(MapResource,map_resource);
            }
          if ((type & IdentifyValidate) != 0)
            tests+=ValidateIdentifyCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & ImportExportValidate) != 0)
            tests+=ValidateImportExportPixels(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & MontageValidate) != 0)
            tests+=ValidateMontageCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          if ((type & StreamValidate) != 0)
            tests+=ValidateStreamCommand(image_info,reference_filename,
              output_filename,&fail,exception);
          (void) FormatLocaleFile(stdout,
            "validation suite: %.20g tests; %.20g passed; %.20g failed.\n",
            (double) tests,(double) (tests-fail),(double) fail);
        }
      (void) RelinquishUniqueFileResource(output_filename);
      (void) RelinquishUniqueFileResource(reference_filename);
    }
  if (exception->severity != UndefinedException)
    CatchException(exception);
  if (iterations > 1)
    {
      elapsed_time=GetElapsedTime(timer);
      user_time=GetUserTime(timer);
      (void) FormatLocaleFile(stderr,
        "Performance: %.20gi %gips %0.3fu %ld:%02ld.%03ld\n",(double)
        iterations,1.0*iterations/elapsed_time,user_time,(long)
        (elapsed_time/60.0),(long) ceil(fmod(elapsed_time,60.0)),
        (long) (1000.0*(elapsed_time-floor(elapsed_time))));
      timer=DestroyTimerInfo(timer);
    }
  DestroyValidate();
  MagickCoreTerminus();
  return(fail == 0 ? 0 : 1);
}
