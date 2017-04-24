/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%          OOO   PPPP   EEEE  RRRR    AA   TTTTT  III   OOO   N   N           %
%         O   O  P   P  E     R   R  A  A    T     I   O   O  NN  N           %
%         O   O  PPPP   EEE   RRRR   AAAA    T     I   O   O  N N N           %
%         O   O  P      E     R R    A  A    T     I   O   O  N  NN           %
%          OOO   P      EEEE  R  RR  A  A    T    III   OOO   N   N           %
%                                                                             %
%                                                                             %
%                         CLI Magick Option Methods                           %
%                                                                             %
%                              Dragon Computing                               %
%                              Anthony Thyssen                                %
%                               September 2011                                %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
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
% Apply the given options (settings, and simple, or sequence operations) to
% the given image(s) according to the current "image_info", "draw_info", and
% "quantize_info" settings, stored in a special CLI Image Wand.
%
% The final goal is to allow the execution in a strict one option at a time
% manner that is needed for 'pipelining and file scripting' of options in
% IMv7.
%
% Anthony Thyssen, September 2011
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/mogrify.h"
#include "MagickWand/operation.h"
#include "MagickWand/wand.h"
#include "MagickWand/wandcli.h"
#include "MagickWand/wandcli-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-private.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"

/*
  Constant declaration.
*/
static const char
  MogrifyAlphaColor[] = "#bdbdbd",  /* slightly darker gray */
  MogrifyBackgroundColor[] = "#fff",  /* white */
  MogrifyBorderColor[] = "#dfdfdf";  /* sRGB gray */

/*
  Define declarations.
*/
#define USE_WAND_METHODS  1
#define MAX_STACK_DEPTH  32
#define UNDEFINED_COMPRESSION_QUALITY  0UL

/* FUTURE: why is this default so specific? */
#define DEFAULT_DISSIMILARITY_THRESHOLD "0.31830988618379067154"

/* For Debugging Geometry Input */
#define ReportGeometry(flags,info) \
  (void) FormatLocaleFile(stderr, "Geometry = 0x%04X : %lg x %lg %+lg %+lg\n", \
       flags, info.rho, info.sigma, info.xi, info.psi )

/*
** Function to report on the progress of image operations
*/
static MagickBooleanType MonitorProgress(const char *text,
  const MagickOffsetType offset,const MagickSizeType extent,
  void *wand_unused(client_data))
{
  char
    message[MagickPathExtent],
    tag[MagickPathExtent];

  const char
    *locale_message;

  register char
    *p;

  magick_unreferenced(client_data);

  if ((extent <= 1) || (offset < 0) || (offset >= (MagickOffsetType) extent))
    return(MagickTrue);
  if ((offset != (MagickOffsetType) (extent-1)) && ((offset % 50) != 0))
    return(MagickTrue);
  (void) CopyMagickString(tag,text,MagickPathExtent);
  p=strrchr(tag,'/');
  if (p != (char *) NULL)
    *p='\0';
  (void) FormatLocaleString(message,MagickPathExtent,"Monitor/%s",tag);
  locale_message=GetLocaleMessage(message);
  if (locale_message == message)
    locale_message=tag;
  if (p == (char *) NULL)
    (void) FormatLocaleFile(stderr,"%s: %ld of %lu, %02ld%% complete\r",
      locale_message,(long) offset,(unsigned long) extent,(long)
      (100L*offset/(extent-1)));
  else
    (void) FormatLocaleFile(stderr,"%s[%s]: %ld of %lu, %02ld%% complete\r",
      locale_message,p+1,(long) offset,(unsigned long) extent,(long)
      (100L*offset/(extent-1)));
  if (offset == (MagickOffsetType) (extent-1))
    (void) FormatLocaleFile(stderr,"\n");
  (void) fflush(stderr);
  return(MagickTrue);
}

/*
** GetImageCache() will read an image into a image cache if not already
** present then return the image that is in the cache under that filename.
*/
static inline Image *GetImageCache(const ImageInfo *image_info,const char *path,
  ExceptionInfo *exception)
{
  char
    key[MagickPathExtent];

  ExceptionInfo
    *sans_exception;

  Image
    *image;

  ImageInfo
    *read_info;

  (void) FormatLocaleString(key,MagickPathExtent,"cache:%s",path);
  sans_exception=AcquireExceptionInfo();
  image=(Image *) GetImageRegistry(ImageRegistryType,key,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (image != (Image *) NULL)
    return(image);
  read_info=CloneImageInfo(image_info);
  if (path != (const char *) NULL)
    (void) CopyMagickString(read_info->filename,path,MagickPathExtent);
  image=ReadImage(read_info,exception);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    (void) SetImageRegistry(ImageRegistryType,key,image,exception);
  return(image);
}

/*
  SparseColorOption() parse the complex -sparse-color argument into an
  an array of floating point values than call SparseColorImage().
  Argument is a complex mix of floating-point pixel coodinates, and color
  specifications (or direct floating point numbers).  The number of floats
  needed to represent a color varies depending on the current channel
  setting.

  This really should be in MagickCore, so that other API's can make use of it.
*/
static Image *SparseColorOption(const Image *image,
  const SparseColorMethod method,const char *arguments,ExceptionInfo *exception)
{
  char
    token[MagickPathExtent];

  const char
    *p;

  double
    *sparse_arguments;

  Image
    *sparse_image;

  PixelInfo
    color;

  MagickBooleanType
    error;

  register size_t
    x;

  size_t
    number_arguments,
    number_colors;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  /*
    Limit channels according to image
    add up number of values needed per color.
  */
  number_colors=0;
  if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
    number_colors++;
  if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
    number_colors++;
  if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
    number_colors++;
  if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
      (image->colorspace == CMYKColorspace))
    number_colors++;
  if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
      image->alpha_trait != UndefinedPixelTrait)
    number_colors++;

  /*
    Read string, to determine number of arguments needed,
  */
  p=arguments;
  x=0;
  while( *p != '\0' )
  {
    GetNextToken(p,&p,MagickPathExtent,token);
    if ( token[0] == ',' ) continue;
    if ( isalpha((int) token[0]) || token[0] == '#' )
      x += number_colors;  /* color argument found */
    else
      x++;   /* floating point argument */
  }
  /* control points and color values */
  if ((x % (2+number_colors)) != 0)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidArgument","'%s': %s", "sparse-color",
        "Invalid number of Arguments");
      return( (Image *) NULL);
    }
  error=MagickFalse;
  number_arguments=x;

  /* Allocate and fill in the floating point arguments */
  sparse_arguments=(double *) AcquireQuantumMemory(number_arguments,
    sizeof(*sparse_arguments));
  if (sparse_arguments == (double *) NULL) {
    (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
      "MemoryAllocationFailed","%s","SparseColorOption");
    return( (Image *) NULL);
  }
  (void) ResetMagickMemory(sparse_arguments,0,number_arguments*
    sizeof(*sparse_arguments));
  p=arguments;
  x=0;
  while( *p != '\0' && x < number_arguments ) {
    /* X coordinate */
    token[0]=','; while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
    if ( token[0] == '\0' ) break;
    if ( isalpha((int) token[0]) || token[0] == '#' ) {
      (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError, "InvalidArgument", "'%s': %s", "sparse-color",
            "Color found, instead of X-coord");
      error=MagickTrue;
      break;
    }
    sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
    /* Y coordinate */
    token[0]=','; while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
    if ( token[0] == '\0' ) break;
    if ( isalpha((int) token[0]) || token[0] == '#' ) {
      (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError, "InvalidArgument", "'%s': %s", "sparse-color",
            "Color found, instead of Y-coord");
      error=MagickTrue;
      break;
    }
    sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
    /* color name or function given in string argument */
    token[0]=','; while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
    if ( token[0] == '\0' ) break;
    if ( isalpha((int) token[0]) || token[0] == '#' ) {
      /* Color string given */
      (void) QueryColorCompliance(token,AllCompliance,&color,
                exception);
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        sparse_arguments[x++] = QuantumScale*color.red;
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        sparse_arguments[x++] = QuantumScale*color.green;
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        sparse_arguments[x++] = QuantumScale*color.blue;
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        sparse_arguments[x++] = QuantumScale*color.black;
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          image->alpha_trait != UndefinedPixelTrait)
        sparse_arguments[x++] = QuantumScale*color.alpha;
    }
    else {
      /* Colors given as a set of floating point values - experimental */
      /* NB: token contains the first floating point value to use! */
      if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
        {
        while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
        if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
          break;
        sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
        token[0] = ','; /* used this token - get another */
      }
      if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
        {
        while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
        if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
          break;
        sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
        token[0] = ','; /* used this token - get another */
      }
      if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
        {
        while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
        if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
          break;
        sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
        token[0] = ','; /* used this token - get another */
      }
      if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
        while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
        if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
          break;
        sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
        token[0] = ','; /* used this token - get another */
      }
      if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
          image->alpha_trait != UndefinedPixelTrait)
        {
        while ( token[0] == ',' ) GetNextToken(p,&p,MagickPathExtent,token);
        if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
          break;
        sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
        token[0] = ','; /* used this token - get another */
      }
    }
  }
  if (error != MagickFalse)
    {
      sparse_arguments=(double *) RelinquishMagickMemory(sparse_arguments);
      return((Image *) NULL);
    }
  if (number_arguments != x)
    {
      sparse_arguments=(double *) RelinquishMagickMemory(sparse_arguments);
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "InvalidArgument","'%s': %s","sparse-color","Argument Parsing Error");
      return((Image *) NULL);
    }
  /* Call the Sparse Color Interpolation function with the parsed arguments */
  sparse_image=SparseColorImage(image,method,number_arguments,sparse_arguments,
    exception);
  sparse_arguments=(double *) RelinquishMagickMemory(sparse_arguments);
  return( sparse_image );
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C L I S e t t i n g O p t i o n I n f o                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLISettingOptionInfo() applies a single settings option into a CLI wand
%  holding the image_info, draw_info, quantize_info structures that will be
%  used when processing the images.
%
%  These options do no require images to be present in the CLI wand for them
%  to be able to be set, in which case they will generally be applied to image
%  that are read in later
%
%  Options handled by this function are listed in CommandOptions[] of
%  "option.c" that is one of "SettingOptionFlags" option flags.
%
%  The format of the CLISettingOptionInfo method is:
%
%    void CLISettingOptionInfo(MagickCLI *cli_wand,
%               const char *option, const char *arg1, const char *arg2)
%
%  A description of each parameter follows:
%
%    o cli_wand: structure holding settings to be applied
%
%    o option: The option string to be set
%
%    o arg1, arg2: optional argument strings to the operation
%        arg2 is currently only used by "-limit"
%
*/
WandPrivate void CLISettingOptionInfo(MagickCLI *cli_wand,
     const char *option,const char *arg1n, const char *arg2n)
{
  ssize_t
    parse;     /* option argument parsing (string to value table lookup) */

  const char    /* percent escaped versions of the args */
    *arg1,
    *arg2;

#define _image_info       (cli_wand->wand.image_info)
#define _image            (cli_wand->wand.images)
#define _exception        (cli_wand->wand.exception)
#define _draw_info        (cli_wand->draw_info)
#define _quantize_info    (cli_wand->quantize_info)
#define IfSetOption       (*option=='-')
#define ArgBoolean        IfSetOption ? MagickTrue : MagickFalse
#define ArgBooleanNot     IfSetOption ? MagickFalse : MagickTrue
#define ArgBooleanString  (IfSetOption?"true":"false")
#define ArgOption(def)    (IfSetOption?arg1:(const char *)(def))

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  assert(cli_wand->wand.signature == MagickWandSignature);

  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
         "- Setting Option: %s \"%s\" \"%s\"", option,arg1n,arg2n);

  arg1 = arg1n,
  arg2 = arg2n;

#if 1
#define _process_flags    (cli_wand->process_flags)
#define _option_type      ((CommandOptionFlags) cli_wand->command->flags)
  /* Interpret Percent Escapes in Arguments - using first image */
  if ( (((_process_flags & ProcessInterpretProperities) != 0 )
        || ((_option_type & AlwaysInterpretArgsFlag) != 0)
       )  && ((_option_type & NeverInterpretArgsFlag) == 0) ) {
    /* Interpret Percent escapes in argument 1 */
    if (arg1n != (char *) NULL) {
      arg1=InterpretImageProperties(_image_info,_image,arg1n,_exception);
      if (arg1 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg1=arg1n;  /* use the given argument as is */
      }
    }
    if (arg2n != (char *) NULL) {
      arg2=InterpretImageProperties(_image_info,_image,arg2n,_exception);
      if (arg2 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg2=arg2n;  /* use the given argument as is */
      }
    }
  }
#undef _process_flags
#undef _option_type
#endif

  switch (*(option+1))
  {
    case 'a':
    {
      if (LocaleCompare("adjoin",option+1) == 0)
        {
          _image_info->adjoin = ArgBoolean;
          break;
        }
      if (LocaleCompare("affine",option+1) == 0)
        {
          CLIWandWarnReplaced("-draw 'affine ...'");
          if (IfSetOption)
            (void) ParseAffineGeometry(arg1,&_draw_info->affine,_exception);
          else
            GetAffineMatrix(&_draw_info->affine);
          break;
        }
      if (LocaleCompare("antialias",option+1) == 0)
        {
          _image_info->antialias =
            _draw_info->stroke_antialias =
              _draw_info->text_antialias = ArgBoolean;
          break;
        }
      if (LocaleCompare("attenuate",option+1) == 0)
        {
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption("1.0"));
          break;
        }
      if (LocaleCompare("authenticate",option+1) == 0)
        {
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'b':
    {
      if (LocaleCompare("background",option+1) == 0)
        {
          /* FUTURE: both _image_info attribute & ImageOption in use!
             _image_info only used directly for generating new images.
             SyncImageSettings() used to set per-image attribute.

             FUTURE: if _image_info->background_color is not set then
             we should fall back to per-image background_color

             At this time -background will 'wipe out' the per-image
             background color!

             Better error handling of QueryColorCompliance() needed.
          */
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          (void) QueryColorCompliance(ArgOption(MogrifyBackgroundColor),AllCompliance,
             &_image_info->background_color,_exception);
          break;
        }
      if (LocaleCompare("bias",option+1) == 0)
        {
          /* FUTURE: bias OBSOLETED, replaced by Artifact "convolve:bias"
             as it is actually rarely used except in direct convolve operations
             Usage outside a direct convolve operation is actally non-sensible!

             SyncImageSettings() used to set per-image attribute.
          */
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,"convolve:bias",ArgOption(NULL));
          break;
        }
      if (LocaleCompare("black-point-compensation",option+1) == 0)
        {
          /* Used as a image chromaticity setting
             SyncImageSettings() used to set per-image attribute.
          */
          (void) SetImageOption(_image_info,option+1,ArgBooleanString);
          break;
        }
      if (LocaleCompare("blue-primary",option+1) == 0)
        {
          /* Image chromaticity X,Y  NB: Y=X if Y not defined
             Used by many coders including PNG
             SyncImageSettings() used to set per-image attribute.
          */
          arg1=ArgOption("0.0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("bordercolor",option+1) == 0)
        {
          /* FUTURE: both _image_info attribute & ImageOption in use!
             SyncImageSettings() used to set per-image attribute.
             Better error checking of QueryColorCompliance().
          */
          if (IfSetOption)
            {
              (void) SetImageOption(_image_info,option+1,arg1);
              (void) QueryColorCompliance(arg1,AllCompliance,
                  &_image_info->border_color,_exception);
              (void) QueryColorCompliance(arg1,AllCompliance,
                  &_draw_info->border_color,_exception);
              break;
            }
          (void) DeleteImageOption(_image_info,option+1);
          (void) QueryColorCompliance(MogrifyBorderColor,AllCompliance,
            &_image_info->border_color,_exception);
          (void) QueryColorCompliance(MogrifyBorderColor,AllCompliance,
            &_draw_info->border_color,_exception);
          break;
        }
      if (LocaleCompare("box",option+1) == 0)
        {
          CLIWandWarnReplaced("-undercolor");
          CLISettingOptionInfo(cli_wand,"-undercolor",arg1, arg2);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'c':
    {
      if (LocaleCompare("cache",option+1) == 0)
        {
          MagickSizeType
            limit;

          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          limit=MagickResourceInfinity;
          if (LocaleCompare("unlimited",arg1) != 0)
            limit=(MagickSizeType) SiPrefixToDoubleInterval(arg1,100.0);
          (void) SetMagickResourceLimit(MemoryResource,limit);
          (void) SetMagickResourceLimit(MapResource,2*limit);
          break;
        }
      if (LocaleCompare("caption",option+1) == 0)
        {
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("colorspace",option+1) == 0)
        {
          /* Setting used for new images via AquireImage()
             But also used as a SimpleImageOperator
             Undefined colorspace means don't modify images on
             read or as a operation */
          parse=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
             ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedColorspace",option,
              arg1);
          _image_info->colorspace=(ColorspaceType) parse;
          break;
        }
      if (LocaleCompare("comment",option+1) == 0)
        {
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("compose",option+1) == 0)
        {
          /* FUTURE: _image_info should be used,
             SyncImageSettings() used to set per-image attribute. - REMOVE

             This setting should NOT be used to set image 'compose'
             "-layer" operators shoud use _image_info if defined otherwise
             they should use a per-image compose setting.
          */
          parse = ParseCommandOption(MagickComposeOptions,MagickFalse,
                          ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedComposeOperator",
                                      option,arg1);
          _image_info->compose=(CompositeOperator) parse;
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("compress",option+1) == 0)
        {
          /* FUTURE: What should be used?  _image_info  or ImageOption ???
             The former is more efficent, but Crisy prefers the latter!
             SyncImageSettings() used to set per-image attribute.

             The coders appears to use _image_info, not Image_Option
             however the image attribute (for save) is set from the
             ImageOption!

             Note that "undefined" is a different setting to "none".
          */
          parse = ParseCommandOption(MagickCompressOptions,MagickFalse,
                     ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedImageCompression",
                                      option,arg1);
          _image_info->compression=(CompressionType) parse;
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'd':
    {
      if (LocaleCompare("debug",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          arg1=ArgOption("none");
          parse = ParseCommandOption(MagickLogEventOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedEventType",
                                      option,arg1);
          (void) SetLogEventMask(arg1);
          _image_info->debug=IsEventLogging();   /* extract logging*/
          cli_wand->wand.debug=IsEventLogging();
          break;
        }
      if (LocaleCompare("define",option+1) == 0)
        {
          if (LocaleNCompare(arg1,"registry:",9) == 0)
            {
              if (IfSetOption)
                (void) DefineImageRegistry(StringRegistryType,arg1+9,_exception);
              else
                (void) DeleteImageRegistry(arg1+9);
              break;
            }
          /* DefineImageOption() equals SetImageOption() but with '=' */
          if (IfSetOption)
            (void) DefineImageOption(_image_info,arg1);
          else if (DeleteImageOption(_image_info,arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"NoSuchOption",option,arg1);
          break;
        }
      if (LocaleCompare("delay",option+1) == 0)
        {
          /* Only used for new images via AcquireImage()
             FUTURE: Option should also be used for "-morph" (color morphing)
          */
          arg1=ArgOption("0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("density",option+1) == 0)
        {
          /* FUTURE: strings used in _image_info attr and _draw_info!
             Basically as density can be in a XxY form!

             SyncImageSettings() used to set per-image attribute.
          */
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          (void) CloneString(&_image_info->density,ArgOption(NULL));
          (void) CloneString(&_draw_info->density,_image_info->density);
          break;
        }
      if (LocaleCompare("depth",option+1) == 0)
        {
          /* This is also a SimpleImageOperator! for 8->16 vaule trunc !!!!
             SyncImageSettings() used to set per-image attribute.
          */
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          _image_info->depth=IfSetOption?StringToUnsignedLong(arg1)
                                       :MAGICKCORE_QUANTUM_DEPTH;
          break;
        }
      if (LocaleCompare("direction",option+1) == 0)
        {
          /* Image Option is only used to set _draw_info */
          arg1=ArgOption("undefined");
          parse = ParseCommandOption(MagickDirectionOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedDirectionType",
                                      option,arg1);
          _draw_info->direction=(DirectionType) parse;
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("display",option+1) == 0)
        {
          (void) CloneString(&_image_info->server_name,ArgOption(NULL));
          (void) CloneString(&_draw_info->server_name,_image_info->server_name);
          break;
        }
      if (LocaleCompare("dispose",option+1) == 0)
        {
          /* only used in setting new images */
          arg1=ArgOption("undefined");
          parse = ParseCommandOption(MagickDisposeOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedDisposeMethod",
                                      option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption("undefined"));
          break;
        }
      if (LocaleCompare("dissimilarity-threshold",option+1) == 0)
        {
          /* FUTURE: this is only used by CompareImages() which is used
             only by the "compare" CLI program at this time.  */
          arg1=ArgOption(DEFAULT_DISSIMILARITY_THRESHOLD);
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("dither",option+1) == 0)
        {
          /* _image_info attr (on/off), _quantize_info attr (on/off)
             but also ImageInfo and _quantize_info method!
             FUTURE: merge the duality of the dithering options
          */
          _image_info->dither = ArgBoolean;
          (void) SetImageOption(_image_info,option+1,ArgOption("none"));
          _quantize_info->dither_method=(DitherMethod) ParseCommandOption(
             MagickDitherOptions,MagickFalse,ArgOption("none"));
          if (_quantize_info->dither_method == NoDitherMethod)
            _image_info->dither = MagickFalse;
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'e':
    {
      if (LocaleCompare("encoding",option+1) == 0)
        {
          (void) CloneString(&_draw_info->encoding,ArgOption("undefined"));
          (void) SetImageOption(_image_info,option+1,_draw_info->encoding);
          break;
        }
      if (LocaleCompare("endian",option+1) == 0)
        {
          /* Both _image_info attr and ImageInfo */
          arg1 = ArgOption("undefined");
          parse = ParseCommandOption(MagickEndianOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedEndianType",
                                      option,arg1);
          /* FUTURE: check alloc/free of endian string!  - remove? */
          _image_info->endian=(EndianType) (*arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("extract",option+1) == 0)
        {
          (void) CloneString(&_image_info->extract,ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'f':
    {
      if (LocaleCompare("family",option+1) == 0)
        {
          (void) CloneString(&_draw_info->family,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("features",option+1) == 0)
        {
          (void) SetImageOption(_image_info,"identify:features",
            ArgBooleanString);
          if (IfSetOption)
            (void) SetImageArtifact(_image,"verbose","true");
          break;
        }
      if (LocaleCompare("fill",option+1) == 0)
        {
          /* Set "fill" OR "fill-pattern" in _draw_info
             The original fill color is preserved if a fill-pattern is given.
             That way it does not effect other operations that directly using
             the fill color and, can be retored using "+tile".
          */
          MagickBooleanType
            status;

          ExceptionInfo
            *sans;

          PixelInfo
            color;

          arg1 = ArgOption("none");  /* +fill turns it off! */
          (void) SetImageOption(_image_info,option+1,arg1);
          if (_draw_info->fill_pattern != (Image *) NULL)
            _draw_info->fill_pattern=DestroyImage(_draw_info->fill_pattern);

          /* is it a color or a image? -- ignore exceptions */
          sans=AcquireExceptionInfo();
          status=QueryColorCompliance(arg1,AllCompliance,&color,sans);
          sans=DestroyExceptionInfo(sans);

          if (status == MagickFalse)
            _draw_info->fill_pattern=GetImageCache(_image_info,arg1,_exception);
          else
            _draw_info->fill=color;
          break;
        }
      if (LocaleCompare("filter",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          arg1 = ArgOption("undefined");
          parse = ParseCommandOption(MagickFilterOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedImageFilter",
                                      option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("font",option+1) == 0)
        {
          (void) CloneString(&_draw_info->font,ArgOption(NULL));
          (void) CloneString(&_image_info->font,_draw_info->font);
          break;
        }
      if (LocaleCompare("format",option+1) == 0)
        {
          /* FUTURE: why the ping test, you could set ping after this! */
          /*
          register const char
            *q;

          for (q=strchr(arg1,'%'); q != (char *) NULL; q=strchr(q+1,'%'))
            if (strchr("Agkrz@[#",*(q+1)) != (char *) NULL)
              _image_info->ping=MagickFalse;
          */
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("fuzz",option+1) == 0)
        {
          /* Option used to set image fuzz! unless blank canvas (from color)
             Image attribute used for color compare operations
             SyncImageSettings() used to set per-image attribute.

             FUTURE: Can't find anything else using _image_info->fuzz directly!
                     convert structure attribute to 'option' string
          */
          arg1=ArgOption("0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          _image_info->fuzz=StringToDoubleInterval(arg1,(double)
                QuantumRange+1.0);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'g':
    {
      if (LocaleCompare("gravity",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          arg1 = ArgOption("none");
          parse = ParseCommandOption(MagickGravityOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedGravityType",
                                      option,arg1);
          _draw_info->gravity=(GravityType) parse;
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("green-primary",option+1) == 0)
        {
          /* Image chromaticity X,Y  NB: Y=X if Y not defined
             SyncImageSettings() used to set per-image attribute.
             Used directly by many coders
          */
          arg1=ArgOption("0.0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'h':
    {
      if (LocaleCompare("highlight-color",option+1) == 0)
        {
          /* FUTURE: this is only used by CompareImages() which is used
             only by the "compare" CLI program at this time.  */
          (void) SetImageOption(_image_info,"compare:highlight-color",
            ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'i':
    {
      if (LocaleCompare("intensity",option+1) == 0)
        {
          arg1 = ArgOption("undefined");
          parse = ParseCommandOption(MagickPixelIntensityOptions,MagickFalse,
            arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedIntensityType",
              option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("intent",option+1) == 0)
        {
          /* Only used by coders: MIFF, MPC, BMP, PNG
             and for image profile call to AcquireTransformThreadSet()
             SyncImageSettings() used to set per-image attribute.
          */
          arg1 = ArgOption("undefined");
          parse = ParseCommandOption(MagickIntentOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedIntentType",
                                      option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("interlace",option+1) == 0)
        {
          /* _image_info is directly used by coders (so why an image setting?)
             SyncImageSettings() used to set per-image attribute.
          */
          arg1 = ArgOption("undefined");
          parse = ParseCommandOption(MagickInterlaceOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedInterlaceType",
                                      option,arg1);
          _image_info->interlace=(InterlaceType) parse;
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("interline-spacing",option+1) == 0)
        {
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1, ArgOption(NULL));
          _draw_info->interline_spacing=StringToDouble(ArgOption("0"),
               (char **) NULL);
          break;
        }
      if (LocaleCompare("interpolate",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          arg1 = ArgOption("undefined");
          parse = ParseCommandOption(MagickInterpolateOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedInterpolateMethod",
                                      option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("interword-spacing",option+1) == 0)
        {
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1, ArgOption(NULL));
          _draw_info->interword_spacing=StringToDouble(ArgOption("0"),(char **) NULL);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'k':
    {
      if (LocaleCompare("kerning",option+1) == 0)
        {
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          _draw_info->kerning=StringToDouble(ArgOption("0"),(char **) NULL);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'l':
    {
      if (LocaleCompare("label",option+1) == 0)
        {
          /* only used for new images - not in SyncImageOptions() */
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("limit",option+1) == 0)
        {
          MagickSizeType
            limit;

          limit=MagickResourceInfinity;
          parse= ParseCommandOption(MagickResourceOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedResourceType",
                option,arg1);
          if (LocaleCompare("unlimited",arg2) != 0)
            limit=(MagickSizeType) SiPrefixToDoubleInterval(arg2,100.0);
          (void) SetMagickResourceLimit((ResourceType)parse,limit);
          break;
        }
      if (LocaleCompare("log",option+1) == 0)
        {
          if (IfSetOption) {
            if ((strchr(arg1,'%') == (char *) NULL))
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            (void) SetLogFormat(arg1);
          }
          break;
        }
      if (LocaleCompare("lowlight-color",option+1) == 0)
        {
          /* FUTURE: this is only used by CompareImages() which is used
             only by the "compare" CLI program at this time.  */
          (void) SetImageOption(_image_info,"compare:lowlight-color",
            ArgOption(NULL));
          break;
        }
      if (LocaleCompare("loop",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          arg1=ArgOption("0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'm':
    {
      if (LocaleCompare("mattecolor",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          (void) QueryColorCompliance(ArgOption(MogrifyAlphaColor),
            AllCompliance,&_image_info->matte_color,_exception);
          break;
        }
      if (LocaleCompare("metric",option+1) == 0)
        {
          /* FUTURE: this is only used by CompareImages() which is used
             only by the "compare" CLI program at this time.  */
          parse=ParseCommandOption(MagickMetricOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedMetricType",
                option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("moments",option+1) == 0)
        {
          (void) SetImageOption(_image_info,"identify:moments",
            ArgBooleanString);
          if (IfSetOption)
            (void) SetImageArtifact(_image,"verbose","true");
          break;
        }
      if (LocaleCompare("monitor",option+1) == 0)
        {
          (void) SetImageInfoProgressMonitor(_image_info, IfSetOption?
                MonitorProgress: (MagickProgressMonitor) NULL, (void *) NULL);
          break;
        }
      if (LocaleCompare("monochrome",option+1) == 0)
        {
          /* Setting (used by some input coders!) -- why?
             Warning: This is also Special '-type' SimpleOperator
          */
          _image_info->monochrome= ArgBoolean;
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'o':
    {
      if (LocaleCompare("orient",option+1) == 0)
        {
          /* Is not used when defining for new images.
             This makes it more of a 'operation' than a setting
             FUTURE: make set meta-data operator instead.
             SyncImageSettings() used to set per-image attribute.
          */
          parse=ParseCommandOption(MagickOrientationOptions,MagickFalse,
               ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedImageOrientation",
                                      option,arg1);
          _image_info->orientation=(OrientationType)parse;
          (void) SetImageOption(_image_info,option+1, ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'p':
    {
      if (LocaleCompare("page",option+1) == 0)
        {
          /* Only used for new images and image generators.
             SyncImageSettings() used to set per-image attribute. ?????
             That last is WRONG!!!!
             FUTURE: adjust named 'page' sizes according density
          */
          char
            *canonical_page,
            page[MagickPathExtent];

          const char
            *image_option;

          MagickStatusType
            flags;

          RectangleInfo
            geometry;

          if (!IfSetOption)
            {
              (void) DeleteImageOption(_image_info,option+1);
              (void) CloneString(&_image_info->page,(char *) NULL);
              break;
            }
          (void) ResetMagickMemory(&geometry,0,sizeof(geometry));
          image_option=GetImageOption(_image_info,"page");
          if (image_option != (const char *) NULL)
            flags=ParseAbsoluteGeometry(image_option,&geometry);
          canonical_page=GetPageGeometry(arg1);
          flags=ParseAbsoluteGeometry(canonical_page,&geometry);
          canonical_page=DestroyString(canonical_page);
          (void) FormatLocaleString(page,MagickPathExtent,"%lux%lu",
            (unsigned long) geometry.width,(unsigned long) geometry.height);
          if (((flags & XValue) != 0) || ((flags & YValue) != 0))
            (void) FormatLocaleString(page,MagickPathExtent,"%lux%lu%+ld%+ld",
              (unsigned long) geometry.width,(unsigned long) geometry.height,
              (long) geometry.x,(long) geometry.y);
          (void) SetImageOption(_image_info,option+1,page);
          (void) CloneString(&_image_info->page,page);
          break;
        }
      if (LocaleCompare("ping",option+1) == 0)
        {
          _image_info->ping = ArgBoolean;
          break;
        }
      if (LocaleCompare("pointsize",option+1) == 0)
        {
          if (IfSetOption) {
            if (IsGeometry(arg1) == MagickFalse)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            _image_info->pointsize =
            _draw_info->pointsize =
              StringToDouble(arg1,(char **) NULL);
          }
          else {
            _image_info->pointsize=0.0; /* unset pointsize */
            _draw_info->pointsize=12.0;
          }
          break;
        }
      if (LocaleCompare("precision",option+1) == 0)
        {
          arg1=ArgOption("-1");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetMagickPrecision(StringToInteger(arg1));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'q':
    {
      if (LocaleCompare("quality",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          _image_info->quality= IfSetOption ? StringToUnsignedLong(arg1)
                                            : UNDEFINED_COMPRESSION_QUALITY;
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("quantize",option+1) == 0)
        {
          /* Just a set direct in _quantize_info */
          arg1=ArgOption("undefined");
          parse=ParseCommandOption(MagickColorspaceOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedColorspace",
                 option,arg1);
          _quantize_info->colorspace=(ColorspaceType)parse;
          break;
        }
      if (LocaleCompare("quiet",option+1) == 0)
        {
          /* FUTURE: if two -quiet is performed you can not do +quiet!
             This needs to be checked over thoughly.
          */
          static WarningHandler
            warning_handler = (WarningHandler) NULL;

          WarningHandler
            tmp = SetWarningHandler((WarningHandler) NULL);

          if ( tmp != (WarningHandler) NULL)
            warning_handler = tmp; /* remember the old handler */
          if (!IfSetOption)        /* set the old handler */
            warning_handler=SetWarningHandler(warning_handler);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'r':
    {
      if (LocaleCompare("red-primary",option+1) == 0)
        {
          /* Image chromaticity X,Y  NB: Y=X if Y not defined
             Used by many coders
             SyncImageSettings() used to set per-image attribute.
          */
          arg1=ArgOption("0.0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("regard-warnings",option+1) == 0)
        /* FUTURE: to be replaced by a 'fatal-level' type setting */
        break;
      if (LocaleCompare("render",option+1) == 0)
        {
          /* _draw_info only setting */
          _draw_info->render= ArgBooleanNot;
          break;
        }
      if (LocaleCompare("respect-parenthesis",option+1) == 0)
        {
          /* link image and setting stacks - option is itself saved on stack! */
          (void) SetImageOption(_image_info,option+1,ArgBooleanString);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 's':
    {
      if (LocaleCompare("sampling-factor",option+1) == 0)
        {
          /* FUTURE: should be converted to jpeg:sampling_factor */
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) CloneString(&_image_info->sampling_factor,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("scene",option+1) == 0)
        {
          /* SyncImageSettings() used to set this as a per-image attribute.
             What ??? Why ????
          */
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          _image_info->scene=StringToUnsignedLong(ArgOption("0"));
          break;
        }
      if (LocaleCompare("seed",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          SetRandomSecretKey(
               IfSetOption ? (unsigned long) StringToUnsignedLong(arg1)
                           : (unsigned long) time((time_t *) NULL) );
          break;
        }
      if (LocaleCompare("size",option+1) == 0)
        {
          /* FUTURE: string in _image_info -- convert to Option ???
             Look at the special handling for "size" in SetImageOption()
           */
          (void) CloneString(&_image_info->size,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("stretch",option+1) == 0)
        {
          arg1=ArgOption("undefined");
          parse = ParseCommandOption(MagickStretchOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedStretchType",
                 option,arg1);
          _draw_info->stretch=(StretchType) parse;
          break;
        }
      if (LocaleCompare("stroke",option+1) == 0)
        {
          /* set stroke color OR stroke-pattern
             UPDATE: ensure stroke color is not destroyed is a pattern
             is given. Just in case the color is also used for other purposes.
           */
          MagickBooleanType
            status;

          ExceptionInfo
            *sans;

          PixelInfo
            color;

          arg1 = ArgOption("none");  /* +fill turns it off! */
          (void) SetImageOption(_image_info,option+1,arg1);
          if (_draw_info->stroke_pattern != (Image *) NULL)
            _draw_info->stroke_pattern=DestroyImage(_draw_info->stroke_pattern);

          /* is it a color or a image? -- ignore exceptions */
          sans=AcquireExceptionInfo();
          status=QueryColorCompliance(arg1,AllCompliance,&color,sans);
          sans=DestroyExceptionInfo(sans);

          if (status == MagickFalse)
            _draw_info->stroke_pattern=GetImageCache(_image_info,arg1,_exception);
          else
            _draw_info->stroke=color;
          break;
        }
      if (LocaleCompare("strokewidth",option+1) == 0)
        {
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          _draw_info->stroke_width=StringToDouble(ArgOption("1.0"),
               (char **) NULL);
          break;
        }
      if (LocaleCompare("style",option+1) == 0)
        {
          arg1=ArgOption("undefined");
          parse = ParseCommandOption(MagickStyleOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedStyleType",
                 option,arg1);
          _draw_info->style=(StyleType) parse;
          break;
        }
#if 0
      if (LocaleCompare("subimage-search",option+1) == 0)
        {
        /* FUTURE: this is only used by CompareImages() which is used
            only by the "compare" CLI program at this time.  */
          (void) SetImageOption(_image_info,option+1,ArgBooleanString);
          break;
        }
#endif
      if (LocaleCompare("synchronize",option+1) == 0)
        {
          /* FUTURE: syncronize to storage - but what does that mean? */
          _image_info->synchronize = ArgBoolean;
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 't':
    {
      if (LocaleCompare("taint",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          (void) SetImageOption(_image_info,option+1,ArgBooleanString);
          break;
        }
      if (LocaleCompare("texture",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          /* FUTURE: move _image_info string to option splay-tree
             Other than "montage" what uses "texture" ????
          */
          (void) CloneString(&_image_info->texture,ArgOption(NULL));
          break;
        }
      if (LocaleCompare("tile",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          _draw_info->fill_pattern=IfSetOption
                                 ?GetImageCache(_image_info,arg1,_exception)
                                 :DestroyImage(_draw_info->fill_pattern);
          break;
        }
      if (LocaleCompare("tile-offset",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. ??? */
          arg1=ArgOption("0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("transparent-color",option+1) == 0)
        {
          /* FUTURE: both _image_info attribute & ImageOption in use!
             _image_info only used for generating new images.
             SyncImageSettings() used to set per-image attribute.

             Note that +transparent-color, means fall-back to image
             attribute so ImageOption is deleted, not set to a default.
          */
          if (IfSetOption && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          (void) QueryColorCompliance(ArgOption("none"),AllCompliance,
              &_image_info->transparent_color,_exception);
          break;
        }
      if (LocaleCompare("treedepth",option+1) == 0)
        {
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          _quantize_info->tree_depth=StringToUnsignedLong(ArgOption("0"));
          break;
        }
      if (LocaleCompare("type",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute. */
          parse=ParseCommandOption(MagickTypeOptions,MagickFalse,
               ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedImageType",
                 option,arg1);
          _image_info->type=(ImageType) parse;
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'u':
    {
      if (LocaleCompare("undercolor",option+1) == 0)
        {
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          (void) QueryColorCompliance(ArgOption("none"),AllCompliance,
               &_draw_info->undercolor,_exception);
          break;
        }
      if (LocaleCompare("units",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute.
             Should this effect _draw_info X and Y resolution?
             FUTURE: this probably should be part of the density setting
          */
          parse=ParseCommandOption(MagickResolutionOptions,MagickFalse,
               ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedUnitsType",
                 option,arg1);
          _image_info->units=(ResolutionType) parse;
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'v':
    {
      if (LocaleCompare("verbose",option+1) == 0)
        {
          /* FUTURE: Remember all options become image artifacts
             _image_info->verbose is only used by coders.
          */
          (void) SetImageOption(_image_info,option+1,ArgBooleanString);
          _image_info->verbose= ArgBoolean;
          _image_info->ping=MagickFalse; /* verbose can't be a ping */
          break;
        }
      if (LocaleCompare("virtual-pixel",option+1) == 0)
        {
          /* SyncImageSettings() used to set per-image attribute.
             This is VERY deep in the image caching structure.
          */
          parse=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
               ArgOption("undefined"));
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedVirtualPixelMethod",
                 option,arg1);
          (void) SetImageOption(_image_info,option+1,ArgOption(NULL));
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'w':
    {
      if (LocaleCompare("weight",option+1) == 0)
        {
          ssize_t
            weight;

          weight=ParseCommandOption(MagickWeightOptions,MagickFalse,arg1);
          if (weight == -1)
            weight=(ssize_t) StringToUnsignedLong(arg1);
          _draw_info->weight=(size_t) weight;
          break;
        }
      if (LocaleCompare("white-point",option+1) == 0)
        {
          /* Used as a image chromaticity setting
             SyncImageSettings() used to set per-image attribute.
          */
          arg1=ArgOption("0.0");
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    default:
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
  }

  /* clean up percent escape interpreted strings */
  if ((arg1 && arg1n) && (arg1 != arg1n ))
    arg1=DestroyString((char *) arg1);
  if ((arg2 && arg2n) && (arg2 != arg2n ))
    arg2=DestroyString((char *) arg2);

#undef _image_info
#undef _exception
#undef _draw_info
#undef _quantize_info
#undef IfSetOption
#undef ArgBoolean
#undef ArgBooleanNot
#undef ArgBooleanString
#undef ArgOption

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C L I S i m p l e O p e r a t o r I m a g e s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLISimpleOperatorImages() applys one simple image operation given to all
%  the images in the CLI wand, using any per-image or global settings that was
%  previously saved in the CLI wand.
%
%  It is assumed that any such settings are up-to-date.
%
%  The format of the WandSimpleOperatorImages method is:
%
%    MagickBooleanType CLISimpleOperatorImages(MagickCLI *cli_wand,const char *option,
%      const char *arg1, const char *arg2,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cli_wand: structure holding settings and images to be operated on
%
%    o option:  The option string for the operation
%
%    o arg1, arg2: optional argument strings to the operation
%
*/

/*
  CLISimpleOperatorImage() is an Internal subrountine to apply one simple
  image operation to the current image pointed to by the CLI wand.

  The image in the list may be modified in three different ways...
    * directly modified (EG: -negate, -gamma, -level, -annotate, -draw),
    * replaced by a new image (EG: -spread, -resize, -rotate, -morphology)
    * one image replace by a list of images (-separate and -crop only!)

  In each case the result replaces the single original image in the list, as
  well as the pointer to the modified image (last image added if replaced by a
  list of images) is returned.

  As the image pointed to may be replaced, the first image in the list may
  also change.  GetFirstImageInList() should be used by caller if they wish
  return the Image pointer to the first image in list.
*/
static MagickBooleanType CLISimpleOperatorImage(MagickCLI *cli_wand,
  const char *option, const char *arg1n, const char *arg2n,
  ExceptionInfo *exception)
{
  Image *
    new_image;

  GeometryInfo
    geometry_info;

  RectangleInfo
    geometry;

  MagickStatusType
    flags;

  ssize_t
    parse;

  const char    /* percent escaped versions of the args */
    *arg1,
    *arg2;

#define _image_info       (cli_wand->wand.image_info)
#define _image            (cli_wand->wand.images)
#define _exception        (cli_wand->wand.exception)
#define _draw_info        (cli_wand->draw_info)
#define _quantize_info    (cli_wand->quantize_info)
#define _process_flags    (cli_wand->process_flags)
#define _option_type      ((CommandOptionFlags) cli_wand->command->flags)
#define IfNormalOp        (*option=='-')
#define IfPlusOp          (*option!='-')
#define IsNormalOp        IfNormalOp ? MagickTrue : MagickFalse
#define IsPlusOp          IfNormalOp ? MagickFalse : MagickTrue

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  assert(cli_wand->wand.signature == MagickWandSignature);
  assert(_image != (Image *) NULL);             /* an image must be present */
  if (cli_wand->wand.debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",cli_wand->wand.name);

  arg1 = arg1n,
  arg2 = arg2n;

  /* Interpret Percent Escapes in Arguments - using first image */
  if ( (((_process_flags & ProcessInterpretProperities) != 0 )
        || ((_option_type & AlwaysInterpretArgsFlag) != 0)
       )  && ((_option_type & NeverInterpretArgsFlag) == 0) ) {
    /* Interpret Percent escapes in argument 1 */
    if (arg1n != (char *) NULL) {
      arg1=InterpretImageProperties(_image_info,_image,arg1n,_exception);
      if (arg1 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg1=arg1n;  /* use the given argument as is */
      }
    }
    if (arg2n != (char *) NULL) {
      arg2=InterpretImageProperties(_image_info,_image,arg2n,_exception);
      if (arg2 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg2=arg2n;  /* use the given argument as is */
      }
    }
  }
#undef _process_flags
#undef _option_type

#if 0
  (void) FormatLocaleFile(stderr,
    "CLISimpleOperatorImage: \"%s\" \"%s\" \"%s\"\n",option,arg1,arg2);
#endif

  new_image = (Image *) NULL; /* the replacement image, if not null at end */
  SetGeometryInfo(&geometry_info);

  switch (*(option+1))
  {
    case 'a':
    {
      if (LocaleCompare("adaptive-blur",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=AdaptiveBlurImage(_image,geometry_info.rho,
            geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("adaptive-resize",option+1) == 0)
        {
          /* FUTURE: Roll into a resize special operator */
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseRegionGeometry(_image,arg1,&geometry,_exception);
          new_image=AdaptiveResizeImage(_image,geometry.width,geometry.height,
            _exception);
          break;
        }
      if (LocaleCompare("adaptive-sharpen",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=AdaptiveSharpenImage(_image,geometry_info.rho,
            geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("alpha",option+1) == 0)
        {
          parse=ParseCommandOption(MagickAlphaChannelOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedAlphaChannelOption",
              option,arg1);
          (void) SetImageAlphaChannel(_image,(AlphaChannelOption) parse,
            _exception);
          break;
        }
      if (LocaleCompare("annotate",option+1) == 0)
        {
          char
            geometry[MagickPathExtent];

          SetGeometryInfo(&geometry_info);
          flags=ParseGeometry(arg1,&geometry_info);
          if (flags == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          (void) CloneString(&_draw_info->text,arg2);
          (void) FormatLocaleString(geometry,MagickPathExtent,"%+f%+f",
            geometry_info.xi,geometry_info.psi);
          (void) CloneString(&_draw_info->geometry,geometry);
          _draw_info->affine.sx=cos(DegreesToRadians(
            fmod(geometry_info.rho,360.0)));
          _draw_info->affine.rx=sin(DegreesToRadians(
            fmod(geometry_info.rho,360.0)));
          _draw_info->affine.ry=(-sin(DegreesToRadians(
            fmod(geometry_info.sigma,360.0))));
          _draw_info->affine.sy=cos(DegreesToRadians(
            fmod(geometry_info.sigma,360.0)));
          (void) AnnotateImage(_image,_draw_info,_exception);
          GetAffineMatrix(&_draw_info->affine);
          break;
        }
      if (LocaleCompare("auto-gamma",option+1) == 0)
        {
          (void) AutoGammaImage(_image,_exception);
          break;
        }
      if (LocaleCompare("auto-level",option+1) == 0)
        {
          (void) AutoLevelImage(_image,_exception);
          break;
        }
      if (LocaleCompare("auto-orient",option+1) == 0)
        {
          new_image=AutoOrientImage(_image,_image->orientation,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'b':
    {
      if (LocaleCompare("black-threshold",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) BlackThresholdImage(_image,arg1,_exception);
          break;
        }
      if (LocaleCompare("blue-shift",option+1) == 0)
        {
          geometry_info.rho=1.5;
          if (IfNormalOp) {
            flags=ParseGeometry(arg1,&geometry_info);
            if ((flags & RhoValue) == 0)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          }
          new_image=BlueShiftImage(_image,geometry_info.rho,_exception);
          break;
        }
      if (LocaleCompare("blur",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=BlurImage(_image,geometry_info.rho,geometry_info.sigma,
           _exception);
          break;
        }
      if (LocaleCompare("border",option+1) == 0)
        {
          CompositeOperator
            compose;

          const char*
            value;

          flags=ParsePageGeometry(_image,arg1,&geometry,_exception);
          if ((flags & (WidthValue | HeightValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          compose=OverCompositeOp;
          value=GetImageOption(_image_info,"compose");
          if (value != (const char *) NULL)
            compose=(CompositeOperator) ParseCommandOption(MagickComposeOptions,
              MagickFalse,value);
          new_image=BorderImage(_image,&geometry,compose,_exception);
          break;
        }
      if (LocaleCompare("brightness-contrast",option+1) == 0)
        {
          double
            brightness,
            contrast;

          GeometryInfo
            geometry_info;

          MagickStatusType
            flags;

          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          brightness=geometry_info.rho;
          contrast=0.0;
          if ((flags & SigmaValue) != 0)
            contrast=geometry_info.sigma;
          (void) BrightnessContrastImage(_image,brightness,contrast,
            _exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'c':
    {
      if (LocaleCompare("canny",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=10;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=30;
          if ((flags & PercentValue) != 0)
            {
              geometry_info.xi/=100.0;
              geometry_info.psi/=100.0;
            }
          new_image=CannyEdgeImage(_image,geometry_info.rho,geometry_info.sigma,
            geometry_info.xi,geometry_info.psi,_exception);
          break;
        }
      if (LocaleCompare("cdl",option+1) == 0)
        {
          char
            *color_correction_collection; /* Note: arguments do not have percent escapes expanded */

          /*
            Color correct with a color decision list.
          */
          color_correction_collection=FileToString(arg1,~0UL,_exception);
          if (color_correction_collection == (char *) NULL)
            break;
          (void) ColorDecisionListImage(_image,color_correction_collection,
            _exception);
          break;
        }
      if (LocaleCompare("channel",option+1) == 0)
        {
          if (IfPlusOp)
            {
              (void) SetPixelChannelMask(_image,DefaultChannels);
              break;
            }
          parse=ParseChannelOption(arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedChannelType",option,
              arg1);
          (void) SetPixelChannelMask(_image,(ChannelType) parse);
          break;
        }
      if (LocaleCompare("charcoal",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=1.0;
          new_image=CharcoalImage(_image,geometry_info.rho,geometry_info.sigma,
            _exception);
          break;
        }
      if (LocaleCompare("chop",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseGravityGeometry(_image,arg1,&geometry,_exception);
          new_image=ChopImage(_image,&geometry,_exception);
          break;
        }
      if (LocaleCompare("clamp",option+1) == 0)
        {
          (void) ClampImage(_image,_exception);
          break;
        }
      if (LocaleCompare("clip",option+1) == 0)
        {
          if (IfNormalOp)
            (void) ClipImage(_image,_exception);
          else /* "+mask" remove the write mask */
            (void) SetImageMask(_image,ReadPixelMask,(Image *) NULL,_exception);
          break;
        }
      if (LocaleCompare("clip-mask",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          CacheView
            *mask_view;

          Image
            *mask_image;

          register Quantum
            *magick_restrict q;

          register ssize_t
            x;

          ssize_t
            y;

          if (IfPlusOp) {
            /* use "+clip-mask" Remove the write mask for -clip-path */
            (void) SetImageMask(_image,ReadPixelMask,(Image *) NULL,_exception);
            break;
          }
          mask_image=GetImageCache(_image_info,arg1,_exception);
          if (mask_image == (Image *) NULL)
            break;
          if (SetImageStorageClass(mask_image,DirectClass,_exception) == MagickFalse)
            break;
          /* Create a write mask from cli_wand mask image */
          /* FUTURE: use Alpha operations instead and create a Grey Image */
          mask_view=AcquireAuthenticCacheView(mask_image,_exception);
          for (y=0; y < (ssize_t) mask_image->rows; y++)
          {
            q=GetCacheViewAuthenticPixels(mask_view,0,y,mask_image->columns,1,
              _exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) mask_image->columns; x++)
            {
              if (mask_image->alpha_trait == UndefinedPixelTrait)
                SetPixelAlpha(mask_image,(Quantum)
                  GetPixelIntensity(mask_image,q),q);
              SetPixelGray(mask_image,GetPixelAlpha(mask_image,q),q);
              q+=GetPixelChannels(mask_image);
            }
            if (SyncCacheViewAuthenticPixels(mask_view,_exception) == MagickFalse)
              break;
          }
          /* clean up and set the write mask */
          mask_view=DestroyCacheView(mask_view);
          mask_image->alpha_trait=BlendPixelTrait;
          (void) SetImageColorspace(_image,GRAYColorspace,_exception);
          (void) SetImageMask(_image,ReadPixelMask,mask_image,_exception);
          mask_image=DestroyImage(mask_image);
          break;
        }
      if (LocaleCompare("clip-path",option+1) == 0)
        {
          (void) ClipImagePath(_image,arg1,IsNormalOp,_exception);
          /* Note: Use "+clip-mask" remove the write mask added */
          break;
        }
      if (LocaleCompare("colorize",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=ColorizeImage(_image,arg1,&_draw_info->fill,_exception);
          break;
        }
      if (LocaleCompare("color-matrix",option+1) == 0)
        {
          KernelInfo
            *kernel;

          kernel=AcquireKernelInfo(arg1,exception);
          if (kernel == (KernelInfo *) NULL)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=ColorMatrixImage(_image,kernel,_exception);
          kernel=DestroyKernelInfo(kernel);
          break;
        }
      if (LocaleCompare("colors",option+1) == 0)
        {
          /* Reduce the number of colors in the image.
             FUTURE: also provide 'plus version with image 'color counts'
          */
          _quantize_info->number_colors=StringToUnsignedLong(arg1);
          if (_quantize_info->number_colors == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((_image->storage_class == DirectClass) ||
              _image->colors > _quantize_info->number_colors)
            (void) QuantizeImage(_quantize_info,_image,_exception);
          else
            (void) CompressImageColormap(_image,_exception);
          break;
        }
      if (LocaleCompare("colorspace",option+1) == 0)
        {
          /* WARNING: this is both a image_info setting (already done)
                      and a operator to change image colorspace.

             FUTURE: default colorspace should be sRGB!
             Unless some type of 'linear colorspace' mode is set.

             Note that +colorspace sets "undefined" or no effect on
             new images, but forces images already in memory back to RGB!
             That seems to be a little strange!
          */
          (void) TransformImageColorspace(_image,
                    IfNormalOp ? _image_info->colorspace : sRGBColorspace,
                    _exception);
          break;
        }
      if (LocaleCompare("connected-components",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=ConnectedComponentsImage(_image,(size_t)
            StringToInteger(arg1),(CCObjectInfo **) NULL,_exception);
          break;
        }
      if (LocaleCompare("contrast",option+1) == 0)
        {
          CLIWandWarnReplaced(IfNormalOp?"-level":"+level");
          (void) ContrastImage(_image,IsNormalOp,_exception);
          break;
        }
      if (LocaleCompare("contrast-stretch",option+1) == 0)
        {
          double
            black_point,
            white_point;

          MagickStatusType
            flags;

          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          black_point=geometry_info.rho;
          white_point=(flags & SigmaValue) != 0 ? geometry_info.sigma :
            black_point;
          if ((flags & PercentValue) != 0) {
              black_point*=(double) _image->columns*_image->rows/100.0;
              white_point*=(double) _image->columns*_image->rows/100.0;
            }
          white_point=(double) _image->columns*_image->rows-white_point;
          (void) ContrastStretchImage(_image,black_point,white_point,
            _exception);
          break;
        }
      if (LocaleCompare("convolve",option+1) == 0)
        {
          double
            gamma;

          KernelInfo
            *kernel_info;

          register ssize_t
            j;

          kernel_info=AcquireKernelInfo(arg1,exception);
          if (kernel_info == (KernelInfo *) NULL)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          gamma=0.0;
          for (j=0; j < (ssize_t) (kernel_info->width*kernel_info->height); j++)
            gamma+=kernel_info->values[j];
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          for (j=0; j < (ssize_t) (kernel_info->width*kernel_info->height); j++)
            kernel_info->values[j]*=gamma;
          new_image=MorphologyImage(_image,CorrelateMorphology,1,kernel_info,
            _exception);
          kernel_info=DestroyKernelInfo(kernel_info);
          break;
        }
      if (LocaleCompare("crop",option+1) == 0)
        {
          /* WARNING: This can generate multiple images! */
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=CropImageToTiles(_image,arg1,_exception);
          break;
        }
      if (LocaleCompare("cycle",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) CycleColormapImage(_image,(ssize_t) StringToLong(arg1),
            _exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'd':
    {
      if (LocaleCompare("decipher",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          StringInfo
            *passkey;

          passkey=FileToStringInfo(arg1,~0UL,_exception);
          if (passkey == (StringInfo *) NULL)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);

          (void) PasskeyDecipherImage(_image,passkey,_exception);
          passkey=DestroyStringInfo(passkey);
          break;
        }
      if (LocaleCompare("depth",option+1) == 0)
        {
          /* The _image_info->depth setting has already been set
             We just need to apply it to all images in current sequence

             WARNING: Depth from 8 to 16 causes 'quantum rounding to images!
             That is it really is an operation, not a setting! Arrgghhh

             FUTURE: this should not be an operator!!!
          */
          (void) SetImageDepth(_image,_image_info->depth,_exception);
          break;
        }
      if (LocaleCompare("deskew",option+1) == 0)
        {
          double
            threshold;

          if (IfNormalOp) {
            if (IsGeometry(arg1) == MagickFalse)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            threshold=StringToDoubleInterval(arg1,(double) QuantumRange+1.0);
          }
          else
            threshold=40.0*QuantumRange/100.0;
          new_image=DeskewImage(_image,threshold,_exception);
          break;
        }
      if (LocaleCompare("despeckle",option+1) == 0)
        {
          new_image=DespeckleImage(_image,_exception);
          break;
        }
      if (LocaleCompare("distort",option+1) == 0)
        {
          double
            *args;

          ssize_t
            count;

          parse = ParseCommandOption(MagickDistortOptions,MagickFalse,arg1);
          if ( parse < 0 )
             CLIWandExceptArgBreak(OptionError,"UnrecognizedDistortMethod",
                                      option,arg1);
          if ((DistortMethod) parse == ResizeDistortion)
            {
               double
                 resize_args[2];
               /* Special Case - Argument is actually a resize geometry!
               ** Convert that to an appropriate distortion argument array.
               ** FUTURE: make a separate special resize operator
                    Roll into a resize special operator */
               if (IsGeometry(arg2) == MagickFalse)
                 CLIWandExceptArgBreak(OptionError,"InvalidGeometry",
                                           option,arg2);
               (void) ParseRegionGeometry(_image,arg2,&geometry,_exception);
               resize_args[0]=(double) geometry.width;
               resize_args[1]=(double) geometry.height;
               new_image=DistortImage(_image,(DistortMethod) parse,
                    (size_t)2,resize_args,MagickTrue,_exception);
               break;
            }
          /* convert argument string into an array of doubles */
          args = StringToArrayOfDoubles(arg2,&count,_exception);
          if (args == (double *) NULL )
            CLIWandExceptArgBreak(OptionError,"InvalidNumberList",option,arg2);

          new_image=DistortImage(_image,(DistortMethod) parse,(size_t)
             count,args,IsPlusOp,_exception);
          args=(double *) RelinquishMagickMemory(args);
          break;
        }
      if (LocaleCompare("draw",option+1) == 0)
        {
          (void) CloneString(&_draw_info->primitive,arg1);
          (void) DrawImage(_image,_draw_info,_exception);
          (void) CloneString(&_draw_info->primitive,(char *) NULL);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'e':
    {
      if (LocaleCompare("edge",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=EdgeImage(_image,geometry_info.rho,_exception);
          break;
        }
      if (LocaleCompare("emboss",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=EmbossImage(_image,geometry_info.rho,
            geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("encipher",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          StringInfo
            *passkey;

          passkey=FileToStringInfo(arg1,~0UL,_exception);
          if (passkey != (StringInfo *) NULL)
            {
              (void) PasskeyEncipherImage(_image,passkey,_exception);
              passkey=DestroyStringInfo(passkey);
            }
          break;
        }
      if (LocaleCompare("enhance",option+1) == 0)
        {
          new_image=EnhanceImage(_image,_exception);
          break;
        }
      if (LocaleCompare("equalize",option+1) == 0)
        {
          (void) EqualizeImage(_image,_exception);
          break;
        }
      if (LocaleCompare("evaluate",option+1) == 0)
        {
          double
            constant;

          parse = ParseCommandOption(MagickEvaluateOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedEvaluateOperator",
                 option,arg1);
          if (IsGeometry(arg2) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg2);
          constant=StringToDoubleInterval(arg2,(double) QuantumRange+1.0);
          (void) EvaluateImage(_image,(MagickEvaluateOperator)parse,constant,
               _exception);
          break;
        }
      if (LocaleCompare("extent",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParseGravityGeometry(_image,arg1,&geometry,_exception);
          if (geometry.width == 0)
            geometry.width=_image->columns;
          if (geometry.height == 0)
            geometry.height=_image->rows;
          new_image=ExtentImage(_image,&geometry,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'f':
    {
      if (LocaleCompare("flip",option+1) == 0)
        {
          new_image=FlipImage(_image,_exception);
          break;
        }
      if (LocaleCompare("flop",option+1) == 0)
        {
          new_image=FlopImage(_image,_exception);
          break;
        }
      if (LocaleCompare("floodfill",option+1) == 0)
        {
          PixelInfo
            target;

          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParsePageGeometry(_image,arg1,&geometry,_exception);
          (void) QueryColorCompliance(arg2,AllCompliance,&target,_exception);
          (void) FloodfillPaintImage(_image,_draw_info,&target,geometry.x,
            geometry.y,IsPlusOp,_exception);
          break;
        }
      if (LocaleCompare("frame",option+1) == 0)
        {
          FrameInfo
            frame_info;

          CompositeOperator
            compose;

          const char*
            value;

          value=GetImageOption(_image_info,"compose");
            compose=OverCompositeOp;  /* use Over not _image->compose */
          if (value != (const char *) NULL)
            compose=(CompositeOperator) ParseCommandOption(MagickComposeOptions,
              MagickFalse,value);
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParsePageGeometry(_image,arg1,&geometry,_exception);
          frame_info.width=geometry.width;
          frame_info.height=geometry.height;
          frame_info.outer_bevel=geometry.x;
          frame_info.inner_bevel=geometry.y;
          frame_info.x=(ssize_t) frame_info.width;
          frame_info.y=(ssize_t) frame_info.height;
          frame_info.width=_image->columns+2*frame_info.width;
          frame_info.height=_image->rows+2*frame_info.height;
          new_image=FrameImage(_image,&frame_info,compose,_exception);
          break;
        }
      if (LocaleCompare("function",option+1) == 0)
        {
          double
            *args;

          ssize_t
            count;

          parse=ParseCommandOption(MagickFunctionOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedFunction",
                 option,arg1);
          /* convert argument string into an array of doubles */
          args = StringToArrayOfDoubles(arg2,&count,_exception);
          if (args == (double *) NULL )
            CLIWandExceptArgBreak(OptionError,"InvalidNumberList",option,arg2);

          (void) FunctionImage(_image,(MagickFunction)parse,(size_t) count,args,
               _exception);
          args=(double *) RelinquishMagickMemory(args);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'g':
    {
      if (LocaleCompare("gamma",option+1) == 0)
        {
          double
            constant;

          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          constant=StringToDouble(arg1,(char **) NULL);
#if 0
          /* Using Gamma, via a cache */
          if (IfPlusOp)
            constant=PerceptibleReciprocal(constant);
          (void) GammaImage(_image,constant,_exception);
#else
          /* Using Evaluate POW, direct update of values - more accurite */
          if (IfNormalOp)
            constant=PerceptibleReciprocal(constant);
          (void) EvaluateImage(_image,PowEvaluateOperator,constant,_exception);
          _image->gamma*=StringToDouble(arg1,(char **) NULL);
#endif
          /* Set gamma setting -- Old meaning of "+gamma"
           * _image->gamma=StringToDouble(arg1,(char **) NULL);
           */
          break;
        }
      if (LocaleCompare("gaussian-blur",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=GaussianBlurImage(_image,geometry_info.rho,
            geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("gaussian",option+1) == 0)
        {
          CLIWandWarnReplaced("-gaussian-blur");
          (void) CLISimpleOperatorImage(cli_wand,"-gaussian-blur",arg1,NULL,exception);
        }
      if (LocaleCompare("geometry",option+1) == 0)
        {
          /*
            Record Image offset for composition. (A Setting)
            Resize last _image. (ListOperator)  -- DEPRECIATE
            FUTURE: Why if no 'offset' does this resize ALL images?
            Also why is the setting recorded in the IMAGE non-sense!
          */
          if (IfPlusOp)
            { /* remove the previous composition geometry offset! */
              if (_image->geometry != (char *) NULL)
                _image->geometry=DestroyString(_image->geometry);
              break;
            }
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParseRegionGeometry(_image,arg1,&geometry,_exception);
          if (((flags & XValue) != 0) || ((flags & YValue) != 0))
            (void) CloneString(&_image->geometry,arg1);
          else
            new_image=ResizeImage(_image,geometry.width,geometry.height,
              _image->filter,_exception);
          break;
        }
      if (LocaleCompare("grayscale",option+1) == 0)
        {
          parse=ParseCommandOption(MagickPixelIntensityOptions,
            MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedIntensityMethod",
              option,arg1);
          (void) GrayscaleImage(_image,(PixelIntensityMethod) parse,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'h':
    {
      if (LocaleCompare("hough-lines",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          if ((flags & XiValue) == 0)
            geometry_info.xi=40;
          new_image=HoughLineImage(_image,(size_t) geometry_info.rho,
            (size_t) geometry_info.sigma,(size_t) geometry_info.xi,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'i':
    {
      if (LocaleCompare("identify",option+1) == 0)
        {
          const char
            *format,
            *text;

          format=GetImageOption(_image_info,"format");
          if (format == (char *) NULL)
            {
              (void) IdentifyImage(_image,stdout,_image_info->verbose,
                _exception);
              break;
            }
          text=InterpretImageProperties(_image_info,_image,format,_exception);
          if (text == (char *) NULL)
            CLIWandExceptionBreak(OptionWarning,"InterpretPropertyFailure",
              option);
          (void) fputs(text,stdout);
          text=DestroyString((char *)text);
          break;
        }
      if (LocaleCompare("implode",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=ImplodeImage(_image,geometry_info.rho,_image->interpolate,
               _exception);
          break;
        }
      if (LocaleCompare("interpolative-resize",option+1) == 0)
        {
          /* FUTURE: New to IMv7
               Roll into a resize special operator */
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseRegionGeometry(_image,arg1,&geometry,_exception);
          new_image=InterpolativeResizeImage(_image,geometry.width,
               geometry.height,_image->interpolate,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'k':
    {
      if (LocaleCompare("kuwahara",option+1) == 0)
        {
          /*
            Edge preserving blur.
          */
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho-0.5;
          new_image=KuwaharaImage(_image,geometry_info.rho,geometry_info.sigma,
           _exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'l':
    {
      if (LocaleCompare("lat",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=AdaptiveThresholdImage(_image,(size_t) geometry_info.rho,
               (size_t) geometry_info.sigma,(double) geometry_info.xi,
               _exception);
          break;
        }
      if (LocaleCompare("level",option+1) == 0)
        {
          double
            black_point,
            gamma,
            white_point;

          MagickStatusType
            flags;

          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          black_point=geometry_info.rho;
          white_point=(double) QuantumRange;
          if ((flags & SigmaValue) != 0)
            white_point=geometry_info.sigma;
          gamma=1.0;
          if ((flags & XiValue) != 0)
            gamma=geometry_info.xi;
          if ((flags & PercentValue) != 0)
            {
              black_point*=(double) (QuantumRange/100.0);
              white_point*=(double) (QuantumRange/100.0);
            }
          if ((flags & SigmaValue) == 0)
            white_point=(double) QuantumRange-black_point;
          if (IfPlusOp || ((flags & AspectValue) != 0))
            (void) LevelizeImage(_image,black_point,white_point,gamma,_exception);
          else
            (void) LevelImage(_image,black_point,white_point,gamma,_exception);
          break;
        }
      if (LocaleCompare("level-colors",option+1) == 0)
        {
          char
            token[MagickPathExtent];

          const char
            *p;

          PixelInfo
            black_point,
            white_point;

          p=(const char *) arg1;
          GetNextToken(p,&p,MagickPathExtent,token);  /* get black point color */
          if ((isalpha((int) *token) != 0) || ((*token == '#') != 0))
            (void) QueryColorCompliance(token,AllCompliance,
                      &black_point,_exception);
          else
            (void) QueryColorCompliance("#000000",AllCompliance,
                      &black_point,_exception);
          if (isalpha((int) token[0]) || (token[0] == '#'))
            GetNextToken(p,&p,MagickPathExtent,token);
          if (*token == '\0')
            white_point=black_point; /* set everything to that color */
          else
            {
              if ((isalpha((int) *token) == 0) && ((*token == '#') == 0))
                GetNextToken(p,&p,MagickPathExtent,token); /* Get white point color. */
              if ((isalpha((int) *token) != 0) || ((*token == '#') != 0))
                (void) QueryColorCompliance(token,AllCompliance,
                           &white_point,_exception);
              else
                (void) QueryColorCompliance("#ffffff",AllCompliance,
                           &white_point,_exception);
            }
          (void) LevelImageColors(_image,&black_point,&white_point,
                     IsPlusOp,_exception);
          break;
        }
      if (LocaleCompare("linear-stretch",option+1) == 0)
        {
          double
            black_point,
            white_point;

          MagickStatusType
            flags;

          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          black_point=geometry_info.rho;
          white_point=(double) _image->columns*_image->rows;
          if ((flags & SigmaValue) != 0)
            white_point=geometry_info.sigma;
          if ((flags & PercentValue) != 0)
            {
              black_point*=(double) _image->columns*_image->rows/100.0;
              white_point*=(double) _image->columns*_image->rows/100.0;
            }
          if ((flags & SigmaValue) == 0)
            white_point=(double) _image->columns*_image->rows-
              black_point;
          (void) LinearStretchImage(_image,black_point,white_point,_exception);
          break;
        }
      if (LocaleCompare("liquid-rescale",option+1) == 0)
        {
          /* FUTURE: Roll into a resize special operator */
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParseRegionGeometry(_image,arg1,&geometry,_exception);
          if ((flags & XValue) == 0)
            geometry.x=1;
          if ((flags & YValue) == 0)
            geometry.y=0;
          new_image=LiquidRescaleImage(_image,geometry.width,
            geometry.height,1.0*geometry.x,1.0*geometry.y,_exception);
          break;
        }
      if (LocaleCompare("local-contrast",option+1) == 0)
        {
          MagickStatusType
            flags;

          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            geometry_info.rho=10;
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=12.5;
          new_image=LocalContrastImage(_image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'm':
    {
      if (LocaleCompare("magnify",option+1) == 0)
        {
          new_image=MagnifyImage(_image,_exception);
          break;
        }
      if (LocaleCompare("map",option+1) == 0)
        {
          CLIWandWarnReplaced("-remap");
          (void) CLISimpleOperatorImage(cli_wand,"-remap",NULL,NULL,exception);
          break;
        }
      if (LocaleCompare("mask",option+1) == 0)
        {
          Image
            *mask;

          if (IfPlusOp)
            {
              /*
                Remove a mask.
              */
              (void) SetImageMask(_image,WritePixelMask,(Image *) NULL,
                _exception);
              break;
            }
          /*
            Set the image mask.
          */
          mask=GetImageCache(_image_info,arg1,_exception);
          if (mask == (Image *) NULL)
            break;
          (void) NegateImage(mask,MagickFalse,exception);
          (void) SetImageMask(_image,WritePixelMask,mask,_exception);
          mask=DestroyImage(mask);
          break;
        }
      if (LocaleCompare("matte",option+1) == 0)
        {
          CLIWandWarnReplaced(IfNormalOp?"-alpha Set":"-alpha Off");
          (void) SetImageAlphaChannel(_image,IfNormalOp ? SetAlphaChannel :
            DeactivateAlphaChannel, _exception);
          break;
        }
      if (LocaleCompare("mean-shift",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.10*QuantumRange;
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=MeanShiftImage(_image,(size_t) geometry_info.rho,
            (size_t) geometry_info.sigma,geometry_info.xi,_exception);
          break;
        }
      if (LocaleCompare("median",option+1) == 0)
        {
          CLIWandWarnReplaced("-statistic Median");
          (void) CLISimpleOperatorImage(cli_wand,"-statistic","Median",arg1,exception);
          break;
        }
      if (LocaleCompare("mode",option+1) == 0)
        {
          /* FUTURE: note this is also a special "montage" option */
          CLIWandWarnReplaced("-statistic Mode");
          (void) CLISimpleOperatorImage(cli_wand,"-statistic","Mode",arg1,exception);
          break;
        }
      if (LocaleCompare("modulate",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ModulateImage(_image,arg1,_exception);
          break;
        }
      if (LocaleCompare("monitor",option+1) == 0)
        {
          (void) SetImageProgressMonitor(_image, IfNormalOp ? MonitorProgress :
                (MagickProgressMonitor) NULL,(void *) NULL);
          break;
        }
      if (LocaleCompare("monochrome",option+1) == 0)
        {
          (void) SetImageType(_image,BilevelType,_exception);
          break;
        }
      if (LocaleCompare("morphology",option+1) == 0)
        {
          char
            token[MagickPathExtent];

          const char
            *p;

          KernelInfo
            *kernel;

          ssize_t
            iterations;

          p=arg1;
          GetNextToken(p,&p,MagickPathExtent,token);
          parse=ParseCommandOption(MagickMorphologyOptions,MagickFalse,token);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedFunction",option,
              arg1);
          iterations=1L;
          GetNextToken(p,&p,MagickPathExtent,token);
          if ((*p == ':') || (*p == ','))
            GetNextToken(p,&p,MagickPathExtent,token);
          if ((*p != '\0'))
            iterations=(ssize_t) StringToLong(p);
          kernel=AcquireKernelInfo(arg2,exception);
          if (kernel == (KernelInfo *) NULL)
            CLIWandExceptArgBreak(OptionError,"UnabletoParseKernel",option,arg2);
          new_image=MorphologyImage(_image,(MorphologyMethod)parse,iterations,
            kernel,_exception);
          kernel=DestroyKernelInfo(kernel);
          break;
        }
      if (LocaleCompare("motion-blur",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=MotionBlurImage(_image,geometry_info.rho,geometry_info.sigma,
            geometry_info.xi,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'n':
    {
      if (LocaleCompare("negate",option+1) == 0)
        {
          (void) NegateImage(_image, IsPlusOp, _exception);
          break;
        }
      if (LocaleCompare("noise",option+1) == 0)
        {
          double
            attenuate;

          const char*
            value;

          if (IfNormalOp)
            {
              CLIWandWarnReplaced("-statistic NonPeak");
              (void) CLISimpleOperatorImage(cli_wand,"-statistic","NonPeak",arg1,exception);
              break;
            }
          parse=ParseCommandOption(MagickNoiseOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedNoiseType",
                option,arg1);
          attenuate=1.0;
          value=GetImageOption(_image_info,"attenuate");
          if  (value != (const char *) NULL)
            attenuate=StringToDouble(value,(char **) NULL);
          new_image=AddNoiseImage(_image,(NoiseType)parse,attenuate,
               _exception);
          break;
        }
      if (LocaleCompare("normalize",option+1) == 0)
        {
          (void) NormalizeImage(_image,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'o':
    {
      if (LocaleCompare("opaque",option+1) == 0)
        {
          PixelInfo
            target;

          (void) QueryColorCompliance(arg1,AllCompliance,&target,_exception);
          (void) OpaquePaintImage(_image,&target,&_draw_info->fill,IsPlusOp,
               _exception);
          break;
        }
      if (LocaleCompare("ordered-dither",option+1) == 0)
        {
          (void) OrderedDitherImage(_image,arg1,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'p':
    {
      if (LocaleCompare("paint",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=OilPaintImage(_image,geometry_info.rho,geometry_info.sigma,
               _exception);
          break;
        }
      if (LocaleCompare("perceptible",option+1) == 0)
        {
          (void) PerceptibleImage(_image,StringToDouble(arg1,(char **) NULL),
            _exception);
          break;
        }
      if (LocaleCompare("polaroid",option+1) == 0)
        {
          const char
            *caption;

          double
            angle;

          if (IfPlusOp) {
            RandomInfo
              *random_info;

            random_info=AcquireRandomInfo();
            angle=22.5*(GetPseudoRandomValue(random_info)-0.5);
            random_info=DestroyRandomInfo(random_info);
          }
          else {
            flags=ParseGeometry(arg1,&geometry_info);
            if ((flags & RhoValue) == 0)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            angle=geometry_info.rho;
          }
          caption=GetImageProperty(_image,"caption",_exception);
          new_image=PolaroidImage(_image,_draw_info,caption,angle,
            _image->interpolate,_exception);
          break;
        }
      if (LocaleCompare("posterize",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) PosterizeImage(_image,(size_t) geometry_info.rho,
            _quantize_info->dither_method,_exception);
          break;
        }
      if (LocaleCompare("preview",option+1) == 0)
        {
          /* FUTURE: should be a 'Genesis' option?
             Option however is also in WandSettingOptionInfo()
             Why???
          */
          parse=ParseCommandOption(MagickPreviewOptions, MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedPreviewType",
                option,arg1);
          new_image=PreviewImage(_image,(PreviewType)parse,_exception);
          break;
        }
      if (LocaleCompare("profile",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          const char
            *name;

          const StringInfo
            *profile;

          Image
            *profile_image;

          ImageInfo
            *profile_info;

          if (IfPlusOp)
            { /* Remove a profile from the _image.  */
              (void) ProfileImage(_image,arg1,(const unsigned char *)
                NULL,0,_exception);
              break;
            }
          /* Associate a profile with the _image.  */
          profile_info=CloneImageInfo(_image_info);
          profile=GetImageProfile(_image,"iptc");
          if (profile != (StringInfo *) NULL)
            profile_info->profile=(void *) CloneStringInfo(profile);
          profile_image=GetImageCache(profile_info,arg1,_exception);
          profile_info=DestroyImageInfo(profile_info);
          if (profile_image == (Image *) NULL)
            {
              StringInfo
                *profile;

              profile_info=CloneImageInfo(_image_info);
              (void) CopyMagickString(profile_info->filename,arg1,
                MagickPathExtent);
              profile=FileToStringInfo(profile_info->filename,~0UL,_exception);
              if (profile != (StringInfo *) NULL)
                {
                  (void) ProfileImage(_image,profile_info->magick,
                    GetStringInfoDatum(profile),(size_t)
                    GetStringInfoLength(profile),_exception);
                  profile=DestroyStringInfo(profile);
                }
              profile_info=DestroyImageInfo(profile_info);
              break;
            }
          ResetImageProfileIterator(profile_image);
          name=GetNextImageProfile(profile_image);
          while (name != (const char *) NULL)
          {
            profile=GetImageProfile(profile_image,name);
            if (profile != (StringInfo *) NULL)
              (void) ProfileImage(_image,name,GetStringInfoDatum(profile),
                (size_t) GetStringInfoLength(profile),_exception);
            name=GetNextImageProfile(profile_image);
          }
          profile_image=DestroyImage(profile_image);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'r':
    {
      if (LocaleCompare("raise",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParsePageGeometry(_image,arg1,&geometry,_exception);
          (void) RaiseImage(_image,&geometry,IsNormalOp,_exception);
          break;
        }
      if (LocaleCompare("random-threshold",option+1) == 0)
        {
          double
            min_threshold,
            max_threshold;

          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          min_threshold=0.0;
          max_threshold=(double) QuantumRange;
          flags=ParseGeometry(arg1,&geometry_info);
          min_threshold=geometry_info.rho;
          max_threshold=geometry_info.sigma;
          if ((flags & SigmaValue) == 0)
            max_threshold=min_threshold;
          if (strchr(arg1,'%') != (char *) NULL)
            {
              max_threshold*=(double) (0.01*QuantumRange);
              min_threshold*=(double) (0.01*QuantumRange);
            }
          (void) RandomThresholdImage(_image,min_threshold,max_threshold,
            _exception);
          break;
        }
      if (LocaleCompare("read-mask",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          Image
            *mask;

          if (IfPlusOp)
            { /* Remove a mask. */
              (void) SetImageMask(_image,ReadPixelMask,(Image *) NULL,
                _exception);
              break;
            }
          /* Set the image mask. */
          mask=GetImageCache(_image_info,arg1,_exception);
          if (mask == (Image *) NULL)
            break;
          (void) SetImageMask(_image,ReadPixelMask,mask,_exception);
          mask=DestroyImage(mask);
          break;
        }
      if (LocaleCompare("recolor",option+1) == 0)
        {
          CLIWandWarnReplaced("-color-matrix");
          (void) CLISimpleOperatorImage(cli_wand,"-color-matrix",arg1,NULL,
            exception);
        }
      if (LocaleCompare("region",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if (*option == '+')
            {
              (void) SetImageRegionMask(_image,ReadPixelMask,
                (const RectangleInfo *) NULL,_exception);
              break;
            }
          (void) ParseGravityGeometry(_image,arg1,&geometry,_exception);
          (void) SetImageRegionMask(_image,ReadPixelMask,&geometry,_exception);
          break;
        }
      if (LocaleCompare("remap",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          Image
            *remap_image;

          remap_image=GetImageCache(_image_info,arg1,_exception);
          if (remap_image == (Image *) NULL)
            break;
          (void) RemapImage(_quantize_info,_image,remap_image,_exception);
          remap_image=DestroyImage(remap_image);
          break;
        }
      if (LocaleCompare("repage",option+1) == 0)
        {
          if (IfNormalOp)
            {
              if (IsGeometry(arg1) == MagickFalse)
                CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,
                  arg1);
              (void) ResetImagePage(_image,arg1);
            }
          else
            (void) ParseAbsoluteGeometry("0x0+0+0",&_image->page);
          break;
        }
      if (LocaleCompare("resample",option+1) == 0)
        {
          /* FUTURE: Roll into a resize special operation */
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=ResampleImage(_image,geometry_info.rho,
            geometry_info.sigma,_image->filter,_exception);
          break;
        }
      if (LocaleCompare("resize",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseRegionGeometry(_image,arg1,&geometry,_exception);
          new_image=ResizeImage(_image,geometry.width,geometry.height,
            _image->filter,_exception);
          break;
        }
      if (LocaleCompare("roll",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParsePageGeometry(_image,arg1,&geometry,_exception);
          new_image=RollImage(_image,geometry.x,geometry.y,_exception);
          break;
        }
      if (LocaleCompare("rotate",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & GreaterValue) != 0 && (_image->columns <= _image->rows))
            break;
          if ((flags & LessValue) != 0 && (_image->columns >= _image->rows))
            break;
          new_image=RotateImage(_image,geometry_info.rho,_exception);
          break;
        }
      if (LocaleCompare("rotational-blur",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=RotationalBlurImage(_image,geometry_info.rho,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 's':
    {
      if (LocaleCompare("sample",option+1) == 0)
        {
          /* FUTURE: Roll into a resize special operator */
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseRegionGeometry(_image,arg1,&geometry,_exception);
          new_image=SampleImage(_image,geometry.width,geometry.height,
            _exception);
          break;
        }
      if (LocaleCompare("scale",option+1) == 0)
        {
          /* FUTURE: Roll into a resize special operator */
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseRegionGeometry(_image,arg1,&geometry,_exception);
          new_image=ScaleImage(_image,geometry.width,geometry.height,
            _exception);
          break;
        }
      if (LocaleCompare("segment",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          (void) SegmentImage(_image,_image->colorspace,
            _image_info->verbose,geometry_info.rho,geometry_info.sigma,
            _exception);
          break;
        }
      if (LocaleCompare("selective-blur",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=SelectiveBlurImage(_image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,_exception);
          break;
        }
      if (LocaleCompare("separate",option+1) == 0)
        {
          /* WARNING: This can generate multiple images! */
          /* FUTURE - this may be replaced by a "-channel" method */
          new_image=SeparateImages(_image,_exception);
          break;
        }
      if (LocaleCompare("sepia-tone",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=SepiaToneImage(_image,StringToDoubleInterval(arg1,
                 (double) QuantumRange+1.0),_exception);
          break;
        }
      if (LocaleCompare("shade",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if (((flags & RhoValue) == 0) || ((flags & SigmaValue) == 0))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=ShadeImage(_image,IsNormalOp,geometry_info.rho,
               geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("shadow",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=4.0;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=4.0;
          new_image=ShadowImage(_image,geometry_info.rho,geometry_info.sigma,
            (ssize_t) ceil(geometry_info.xi-0.5),(ssize_t)
            ceil(geometry_info.psi-0.5),_exception);
          break;
        }
      if (LocaleCompare("sharpen",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.0;
          new_image=SharpenImage(_image,geometry_info.rho,geometry_info.sigma,
           _exception);
          break;
        }
      if (LocaleCompare("shave",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParsePageGeometry(_image,arg1,&geometry,_exception);
          new_image=ShaveImage(_image,&geometry,_exception);
          break;
        }
      if (LocaleCompare("shear",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=ShearImage(_image,geometry_info.rho,geometry_info.sigma,
            _exception);
          break;
        }
      if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=(double) QuantumRange/2.0;
          if ((flags & PercentValue) != 0)
            geometry_info.sigma=(double) QuantumRange*geometry_info.sigma/
              100.0;
          (void) SigmoidalContrastImage(_image,IsNormalOp,geometry_info.rho,
               geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("sketch",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=SketchImage(_image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,_exception);
          break;
        }
      if (LocaleCompare("solarize",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) SolarizeImage(_image,StringToDoubleInterval(arg1,(double)
                 QuantumRange+1.0),_exception);
          break;
        }
      if (LocaleCompare("sparse-color",option+1) == 0)
        {
          parse= ParseCommandOption(MagickSparseColorOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedSparseColorMethod",
                option,arg1);
          new_image=SparseColorOption(_image,(SparseColorMethod)parse,arg2,
               _exception);
          break;
        }
      if (LocaleCompare("splice",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          flags=ParseGravityGeometry(_image,arg1,&geometry,_exception);
          new_image=SpliceImage(_image,&geometry,_exception);
          break;
        }
      if (LocaleCompare("spread",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg2);
          new_image=SpreadImage(_image,_image->interpolate,geometry_info.rho,
           _exception);
          break;
        }
      if (LocaleCompare("statistic",option+1) == 0)
        {
          parse=ParseCommandOption(MagickStatisticOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedStatisticType",
                 option,arg1);
          flags=ParseGeometry(arg2,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg2);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=StatisticImage(_image,(StatisticType)parse,
               (size_t) geometry_info.rho,(size_t) geometry_info.sigma,
               _exception);
          break;
        }
      if (LocaleCompare("strip",option+1) == 0)
        {
          (void) StripImage(_image,_exception);
          break;
        }
      if (LocaleCompare("swirl",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=SwirlImage(_image,geometry_info.rho,
            _image->interpolate,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 't':
    {
      if (LocaleCompare("threshold",option+1) == 0)
        {
          double
            threshold;

          threshold=(double) QuantumRange/2;
          if (IfNormalOp) {
            if (IsGeometry(arg1) == MagickFalse)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            threshold=StringToDoubleInterval(arg1,(double) QuantumRange+1.0);
          }
          (void) BilevelImage(_image,threshold,_exception);
          break;
        }
      if (LocaleCompare("thumbnail",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) ParseRegionGeometry(_image,arg1,&geometry,_exception);
          new_image=ThumbnailImage(_image,geometry.width,geometry.height,
            _exception);
          break;
        }
      if (LocaleCompare("tint",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          new_image=TintImage(_image,arg1,&_draw_info->fill,_exception);
          break;
        }
      if (LocaleCompare("transform",option+1) == 0)
        {
          CLIWandWarnReplaced("+distort AffineProjection");
          new_image=AffineTransformImage(_image,&_draw_info->affine,_exception);
          break;
        }
      if (LocaleCompare("transparent",option+1) == 0)
        {
          PixelInfo
            target;

          (void) QueryColorCompliance(arg1,AllCompliance,&target,_exception);
          (void) TransparentPaintImage(_image,&target,(Quantum)
            TransparentAlpha,IsPlusOp,_exception);
          break;
        }
      if (LocaleCompare("transpose",option+1) == 0)
        {
          new_image=TransposeImage(_image,_exception);
          break;
        }
      if (LocaleCompare("transverse",option+1) == 0)
        {
          new_image=TransverseImage(_image,_exception);
          break;
        }
      if (LocaleCompare("trim",option+1) == 0)
        {
          new_image=TrimImage(_image,_exception);
          break;
        }
      if (LocaleCompare("type",option+1) == 0)
        {
          /* Note that "type" setting should have already been defined */
          (void) SetImageType(_image,_image_info->type,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'u':
    {
      if (LocaleCompare("unique",option+1) == 0)
        {
          /* FUTURE: move to SyncImageSettings() and AcqireImage()???
             Option is not documented, bt appears to be for "identify".
             We may need a identify specific verbose!
          */
          if (IsPlusOp) {
              (void) DeleteImageArtifact(_image,"identify:unique-colors");
              break;
            }
          (void) SetImageArtifact(_image,"identify:unique-colors","true");
          (void) SetImageArtifact(_image,"verbose","true");
          break;
        }
      if (LocaleCompare("unique-colors",option+1) == 0)
        {
          new_image=UniqueImageColors(_image,_exception);
          break;
        }
      if (LocaleCompare("unsharp",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=1.0;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=0.05;
          new_image=UnsharpMaskImage(_image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,geometry_info.psi,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'v':
    {
      if (LocaleCompare("verbose",option+1) == 0)
        {
          /* FUTURE: move to SyncImageSettings() and AcquireImage()???
             three places!   ImageArtifact   ImageOption  _image_info->verbose
             Some how new images also get this artifact!
          */
          (void) SetImageArtifact(_image,option+1,
                           IfNormalOp ? "true" : "false" );
          break;
        }
      if (LocaleCompare("vignette",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.1*_image->columns;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=0.1*_image->rows;
          if ((flags & PercentValue) != 0)
            {
              geometry_info.xi*=(double) _image->columns/100.0;
              geometry_info.psi*=(double) _image->rows/100.0;
            }
          new_image=VignetteImage(_image,geometry_info.rho,geometry_info.sigma,
            (ssize_t) ceil(geometry_info.xi-0.5),(ssize_t)
            ceil(geometry_info.psi-0.5),_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'w':
    {
      if (LocaleCompare("wave",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & (RhoValue|SigmaValue)) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=WaveImage(_image,geometry_info.rho,geometry_info.sigma,
            _image->interpolate,_exception);
          break;
        }
      if (LocaleCompare("wavelet-denoise",option+1) == 0)
        {
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          if ((flags & PercentValue) != 0)
            {
              geometry_info.rho=QuantumRange*geometry_info.rho/100.0;
              geometry_info.sigma=QuantumRange*geometry_info.sigma/100.0;
            }
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=0.0;
          new_image=WaveletDenoiseImage(_image,geometry_info.rho,
            geometry_info.sigma,_exception);
          break;
        }
      if (LocaleCompare("white-threshold",option+1) == 0)
        {
          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          (void) WhiteThresholdImage(_image,arg1,_exception);
          break;
        }
      if (LocaleCompare("write-mask",option+1) == 0)
        {
          /* Note: arguments do not have percent escapes expanded */
          Image
            *mask;

          if (IfPlusOp)
            { /* Remove a mask. */
              (void) SetImageMask(_image,WritePixelMask,(Image *) NULL,
                _exception);
              break;
            }
          /* Set the image mask. */
          mask=GetImageCache(_image_info,arg1,_exception);
          if (mask == (Image *) NULL)
            break;
          (void) SetImageMask(_image,WritePixelMask,mask,_exception);
          mask=DestroyImage(mask);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    default:
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
  }
  /* clean up percent escape interpreted strings */
  if (arg1 != arg1n )
    arg1=DestroyString((char *)arg1);
  if (arg2 != arg2n )
    arg2=DestroyString((char *)arg2);

  /* Replace current image with any image that was generated
     and set image point to last image (so image->next is correct) */
  if (new_image != (Image *) NULL)
    ReplaceImageInListReturnLast(&_image,new_image);

  return(MagickTrue);
#undef _image_info
#undef _draw_info
#undef _quantize_info
#undef _image
#undef _exception
#undef IfNormalOp
#undef IfPlusOp
#undef IsNormalOp
#undef IsPlusOp
}

WandPrivate MagickBooleanType CLISimpleOperatorImages(MagickCLI *cli_wand,
  const char *option,const char *arg1,const char *arg2,ExceptionInfo *exception)
{
#if !USE_WAND_METHODS
  size_t
    n,
    i;
#endif

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  assert(cli_wand->wand.signature == MagickWandSignature);
  assert(cli_wand->wand.images != (Image *) NULL); /* images must be present */

  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
         "- Simple Operator: %s \"%s\" \"%s\"", option,arg1,arg2);

#if !USE_WAND_METHODS
  /* FUTURE add appropriate tracing */
  i=0;
  n=GetImageListLength(cli_wand->wand.images);
  cli_wand->wand.images=GetFirstImageInList(cli_wand->wand.images);
  while (1) {
    i++;
    CLISimpleOperatorImage(cli_wand, option, arg1, arg2,exception);
    if ( cli_wand->wand.images->next == (Image *) NULL )
      break;
    cli_wand->wand.images=cli_wand->wand.images->next;
  }
  assert( i == n );
  cli_wand->wand.images=GetFirstImageInList(cli_wand->wand.images);
#else
  MagickResetIterator(&cli_wand->wand);
  while (MagickNextImage(&cli_wand->wand) != MagickFalse)
    (void) CLISimpleOperatorImage(cli_wand, option, arg1, arg2,exception);
  MagickResetIterator(&cli_wand->wand);
#endif
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     C L I L i s t O p e r a t o r I m a g e s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLIListOperatorImages() applies a single operation that is apply to the
%  entire image list as a whole. The result is often a complete replacment
%  of the image list with a completely new list, or with just a single image
%  result.
%
%  The format of the MogrifyImage method is:
%
%    MagickBooleanType CLIListOperatorImages(MagickCLI *cli_wand,
%      const char *option,const char *arg1,const char *arg2)
%
%  A description of each parameter follows:
%
%    o cli_wand: structure holding settings to be applied
%
%    o option:  The option string for the operation
%
%    o arg1, arg2: optional argument strings to the operation
%        arg2 is currently not used
%
*/
WandPrivate MagickBooleanType CLIListOperatorImages(MagickCLI *cli_wand,
  const char *option,const char *arg1n,const char *arg2n)
{
  const char    /* percent escaped versions of the args */
    *arg1,
    *arg2;

  Image
    *new_images;

  MagickStatusType
    status;

  ssize_t
    parse;

#define _image_info     (cli_wand->wand.image_info)
#define _images         (cli_wand->wand.images)
#define _exception      (cli_wand->wand.exception)
#define _draw_info      (cli_wand->draw_info)
#define _quantize_info  (cli_wand->quantize_info)
#define _process_flags  (cli_wand->process_flags)
#define _option_type    ((CommandOptionFlags) cli_wand->command->flags)
#define IfNormalOp      (*option=='-')
#define IfPlusOp        (*option!='-')
#define IsNormalOp      IfNormalOp ? MagickTrue : MagickFalse

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  assert(cli_wand->wand.signature == MagickWandSignature);
  assert(_images != (Image *) NULL);             /* _images must be present */

  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
       "- List Operator: %s \"%s\" \"%s\"", option,
       arg1n == (const char *) NULL ? "null" : arg1n,
       arg2n == (const char *) NULL ? "null" : arg2n);

  arg1 = arg1n;
  arg2 = arg2n;

  /* Interpret Percent Escapes in Arguments - using first image */
  if ( (((_process_flags & ProcessInterpretProperities) != 0 )
        || ((_option_type & AlwaysInterpretArgsFlag) != 0)
       )  && ((_option_type & NeverInterpretArgsFlag) == 0) ) {
    /* Interpret Percent escapes in argument 1 */
    if (arg1n != (char *) NULL) {
      arg1=InterpretImageProperties(_image_info,_images,arg1n,_exception);
      if (arg1 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg1=arg1n;  /* use the given argument as is */
      }
    }
    if (arg2n != (char *) NULL) {
      arg2=InterpretImageProperties(_image_info,_images,arg2n,_exception);
      if (arg2 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg2=arg2n;  /* use the given argument as is */
      }
    }
  }
#undef _process_flags
#undef _option_type

  status=MagickTrue;
  new_images=NewImageList();

  switch (*(option+1))
  {
    case 'a':
    {
      if (LocaleCompare("append",option+1) == 0)
        {
          new_images=AppendImages(_images,IsNormalOp,_exception);
          break;
        }
      if (LocaleCompare("average",option+1) == 0)
        {
          CLIWandWarnReplaced("-evaluate-sequence Mean");
          (void) CLIListOperatorImages(cli_wand,"-evaluate-sequence","Mean",
            NULL);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'c':
    {
      if (LocaleCompare("channel-fx",option+1) == 0)
        {
          new_images=ChannelFxImage(_images,arg1,_exception);
          break;
        }
      if (LocaleCompare("clut",option+1) == 0)
        {
          Image
            *clut_image;

          /* FUTURE - make this a compose option, and thus can be used
             with layers compose or even compose last image over all other
             _images.
          */
          new_images=RemoveFirstImageFromList(&_images);
          clut_image=RemoveLastImageFromList(&_images);
          /* FUTURE - produce Exception, rather than silent fail */
          if (clut_image == (Image *) NULL)
            break;
          (void) ClutImage(new_images,clut_image,new_images->interpolate,
            _exception);
          clut_image=DestroyImage(clut_image);
          break;
        }
      if (LocaleCompare("coalesce",option+1) == 0)
        {
          new_images=CoalesceImages(_images,_exception);
          break;
        }
      if (LocaleCompare("combine",option+1) == 0)
        {
          parse=(ssize_t) _images->colorspace;
          if ( IfPlusOp )
            parse=ParseCommandOption(MagickColorspaceOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedColorspace",option,
              arg1);
          new_images=CombineImages(_images,(ColorspaceType) parse,_exception);
          break;
        }
      if (LocaleCompare("compare",option+1) == 0)
        {
          double
            distortion;

          Image
            *image,
            *reconstruct_image;

          MetricType
            metric;

          /*
            Mathematically and visually annotate the difference between an
            image and its reconstruction.
          */
          image=RemoveFirstImageFromList(&_images);
          reconstruct_image=RemoveFirstImageFromList(&_images);
          /* FUTURE - produce Exception, rather than silent fail */
          if (reconstruct_image == (Image *) NULL)
            break;
          metric=UndefinedErrorMetric;
          option=GetImageOption(_image_info,"metric");
          if (option != (const char *) NULL)
            metric=(MetricType) ParseCommandOption(MagickMetricOptions,
              MagickFalse,option);
          new_images=CompareImages(image,reconstruct_image,metric,&distortion,
            _exception);
          (void) distortion;
          reconstruct_image=DestroyImage(reconstruct_image);
          image=DestroyImage(image);
          break;
        }
      if (LocaleCompare("complex",option+1) == 0)
        {
          parse=ParseCommandOption(MagickComplexOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedEvaluateOperator",
              option,arg1);
          new_images=ComplexImages(_images,(ComplexOperator) parse,_exception);
          break;
        }
      if (LocaleCompare("composite",option+1) == 0)
        {
          CompositeOperator
            compose;

          const char*
            value;

          MagickBooleanType
            clip_to_self;

          Image
            *mask_image,
            *source_image;

          RectangleInfo
            geometry;

          /* Compose value from "-compose" option only */
          value=GetImageOption(_image_info,"compose");
          if (value == (const char *) NULL)
            compose=OverCompositeOp;  /* use Over not source_image->compose */
          else
            compose=(CompositeOperator) ParseCommandOption(MagickComposeOptions,
              MagickFalse,value);

          /* Get "clip-to-self" expert setting (false is normal) */
          value=GetImageOption(_image_info,"compose:clip-to-self");
          if (value == (const char *) NULL)
            clip_to_self=MagickTrue;
          else
            clip_to_self=IsStringTrue(GetImageOption(_image_info,
              "compose:clip-to-self")); /* if this is true */
          value=GetImageOption(_image_info,"compose:outside-overlay");
          if (value != (const char *) NULL) {   /* or this false */
            /* FUTURE: depreciate warning for "compose:outside-overlay"*/
            clip_to_self=IsStringFalse(value);
          }

          new_images=RemoveFirstImageFromList(&_images);
          source_image=RemoveFirstImageFromList(&_images);
          if (source_image == (Image *) NULL)
            break; /* FUTURE - produce Exception, rather than silent fail */

          /* FUTURE - this should not be here! - should be part of -geometry */
          if (source_image->geometry != (char *) NULL)
            {
              RectangleInfo
                resize_geometry;

              (void) ParseRegionGeometry(source_image,source_image->geometry,
                &resize_geometry,_exception);
              if ((source_image->columns != resize_geometry.width) ||
                  (source_image->rows != resize_geometry.height))
                {
                  Image
                    *resize_image;

                  resize_image=ResizeImage(source_image,resize_geometry.width,
                    resize_geometry.height,source_image->filter,_exception);
                  if (resize_image != (Image *) NULL)
                    {
                      source_image=DestroyImage(source_image);
                      source_image=resize_image;
                    }
                }
            }
          SetGeometry(source_image,&geometry);
          (void) ParseAbsoluteGeometry(source_image->geometry,&geometry);
          GravityAdjustGeometry(new_images->columns,new_images->rows,
            new_images->gravity, &geometry);
          mask_image=RemoveFirstImageFromList(&_images);
          if (mask_image == (Image *) NULL)
            status&=CompositeImage(new_images,source_image,compose,clip_to_self,
              geometry.x,geometry.y,_exception);
          else
            {
              if ((compose == DisplaceCompositeOp) ||
                  (compose == DistortCompositeOp))
                {
                  status&=CompositeImage(source_image,mask_image,
                    CopyGreenCompositeOp,MagickTrue,0,0,_exception);
                  status&=CompositeImage(new_images,source_image,compose,
                    clip_to_self,geometry.x,geometry.y,_exception);
                }
              else
                {
                  Image
                    *clone_image;

                  clone_image=CloneImage(new_images,0,0,MagickTrue,_exception);
                  if (clone_image == (Image *) NULL)
                    break;
                  status&=CompositeImage(new_images,source_image,compose,
                    clip_to_self,geometry.x,geometry.y,_exception);
                  status&=CompositeImage(new_images,mask_image,
                    CopyAlphaCompositeOp,MagickTrue,0,0,_exception);
                  status&=CompositeImage(clone_image,new_images,OverCompositeOp,
                    clip_to_self,0,0,_exception);
                  new_images=DestroyImage(new_images);
                  new_images=clone_image;
                }
              mask_image=DestroyImage(mask_image);
            }
          source_image=DestroyImage(source_image);
          break;
        }
        if (LocaleCompare("copy",option+1) == 0)
          {
            Image
              *source_image;

            OffsetInfo
              offset;

            RectangleInfo
              geometry;

            /*
              Copy image pixels.
            */
            if (IsGeometry(arg1) == MagickFalse)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            if (IsGeometry(arg2) == MagickFalse)
              CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
            (void) ParsePageGeometry(_images,arg2,&geometry,_exception);
            offset.x=geometry.x;
            offset.y=geometry.y;
            source_image=_images;
            if (source_image->next != (Image *) NULL)
              source_image=source_image->next;
            (void) ParsePageGeometry(source_image,arg1,&geometry,_exception);
            (void) CopyImagePixels(_images,source_image,&geometry,&offset,
              _exception);
            break;
          }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'd':
    {
      if (LocaleCompare("deconstruct",option+1) == 0)
        {
          CLIWandWarnReplaced("-layer CompareAny");
          (void) CLIListOperatorImages(cli_wand,"-layer","CompareAny",NULL);
          break;
        }
      if (LocaleCompare("delete",option+1) == 0)
        {
          if (IfNormalOp)
            DeleteImages(&_images,arg1,_exception);
          else
            DeleteImages(&_images,"-1",_exception);
          break;
        }
      if (LocaleCompare("duplicate",option+1) == 0)
        {
          if (IfNormalOp)
            {
              const char
                *p;

              size_t
                number_duplicates;

              if (IsGeometry(arg1) == MagickFalse)
                CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,
                      arg1);
              number_duplicates=(size_t) StringToLong(arg1);
              p=strchr(arg1,',');
              if (p == (const char *) NULL)
                new_images=DuplicateImages(_images,number_duplicates,"-1",
                  _exception);
              else
                new_images=DuplicateImages(_images,number_duplicates,p,
                  _exception);
            }
          else
            new_images=DuplicateImages(_images,1,"-1",_exception);
          AppendImageToList(&_images, new_images);
          new_images=(Image *) NULL;
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'e':
    {
      if (LocaleCompare("evaluate-sequence",option+1) == 0)
        {
          parse=ParseCommandOption(MagickEvaluateOptions,MagickFalse,arg1);
          if (parse < 0)
            CLIWandExceptArgBreak(OptionError,"UnrecognizedEvaluateOperator",
              option,arg1);
          new_images=EvaluateImages(_images,(MagickEvaluateOperator)parse,
            _exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'f':
    {
      if (LocaleCompare("fft",option+1) == 0)
        {
          new_images=ForwardFourierTransformImage(_images,IsNormalOp,
           _exception);
          break;
        }
      if (LocaleCompare("flatten",option+1) == 0)
        {
          /* REDIRECTED to use -layers flatten instead */
          (void) CLIListOperatorImages(cli_wand,"-layers",option+1,NULL);
          break;
        }
      if (LocaleCompare("fx",option+1) == 0)
        {
          new_images=FxImage(_images,arg1,_exception);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'h':
    {
      if (LocaleCompare("hald-clut",option+1) == 0)
        {
          /* FUTURE - make this a compose option (and thus layers compose )
             or perhaps compose last image over all other _images.
          */
          Image
            *hald_image;

          new_images=RemoveFirstImageFromList(&_images);
          hald_image=RemoveLastImageFromList(&_images);
          if (hald_image == (Image *) NULL)
            break;
          (void) HaldClutImage(new_images,hald_image,_exception);
          hald_image=DestroyImage(hald_image);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'i':
    {
      if (LocaleCompare("ift",option+1) == 0)
        {
          Image
            *magnitude_image,
            *phase_image;

           magnitude_image=RemoveFirstImageFromList(&_images);
           phase_image=RemoveFirstImageFromList(&_images);
          /* FUTURE - produce Exception, rather than silent fail */
           if (phase_image == (Image *) NULL)
             break;
           new_images=InverseFourierTransformImage(magnitude_image,phase_image,
             IsNormalOp,_exception);
           magnitude_image=DestroyImage(magnitude_image);
           phase_image=DestroyImage(phase_image);
          break;
        }
      if (LocaleCompare("insert",option+1) == 0)
        {
          Image
            *insert_image,
            *index_image;

          ssize_t
            index;

          if (IfNormalOp && (IsGeometry(arg1) == MagickFalse))
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          index=0;
          insert_image=RemoveLastImageFromList(&_images);
          if (IfNormalOp)
            index=(ssize_t) StringToLong(arg1);
          index_image=insert_image;
          if (index == 0)
            PrependImageToList(&_images,insert_image);
          else if (index == (ssize_t) GetImageListLength(_images))
            AppendImageToList(&_images,insert_image);
          else
            {
               index_image=GetImageFromList(_images,index-1);
               if (index_image == (Image *) NULL)
                 CLIWandExceptArgBreak(OptionError,"NoSuchImage",option,arg1);
              InsertImageInList(&index_image,insert_image);
            }
          _images=GetFirstImageInList(index_image);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'l':
    {
      if (LocaleCompare("layers",option+1) == 0)
        {
          parse=ParseCommandOption(MagickLayerOptions,MagickFalse,arg1);
          if ( parse < 0 )
            CLIWandExceptArgBreak(OptionError,"UnrecognizedLayerMethod",
                 option,arg1);
          switch ((LayerMethod) parse)
          {
            case CoalesceLayer:
            {
              new_images=CoalesceImages(_images,_exception);
              break;
            }
            case CompareAnyLayer:
            case CompareClearLayer:
            case CompareOverlayLayer:
            default:
            {
              new_images=CompareImagesLayers(_images,(LayerMethod) parse,
                   _exception);
              break;
            }
            case MergeLayer:
            case FlattenLayer:
            case MosaicLayer:
            case TrimBoundsLayer:
            {
              new_images=MergeImageLayers(_images,(LayerMethod) parse,
                _exception);
              break;
            }
            case DisposeLayer:
            {
              new_images=DisposeImages(_images,_exception);
              break;
            }
            case OptimizeImageLayer:
            {
              new_images=OptimizeImageLayers(_images,_exception);
              break;
            }
            case OptimizePlusLayer:
            {
              new_images=OptimizePlusImageLayers(_images,_exception);
              break;
            }
            case OptimizeTransLayer:
            {
              OptimizeImageTransparency(_images,_exception);
              break;
            }
            case RemoveDupsLayer:
            {
              RemoveDuplicateLayers(&_images,_exception);
              break;
            }
            case RemoveZeroLayer:
            {
              RemoveZeroDelayLayers(&_images,_exception);
              break;
            }
            case OptimizeLayer:
            { /* General Purpose, GIF Animation Optimizer.  */
              new_images=CoalesceImages(_images,_exception);
              if (new_images == (Image *) NULL)
                break;
              _images=DestroyImageList(_images);
              _images=OptimizeImageLayers(new_images,_exception);
              if (_images == (Image *) NULL)
                break;
              new_images=DestroyImageList(new_images);
              OptimizeImageTransparency(_images,_exception);
              (void) RemapImages(_quantize_info,_images,(Image *) NULL,
                _exception);
              break;
            }
            case CompositeLayer:
            {
              Image
                *source;

              RectangleInfo
                geometry;

              CompositeOperator
                compose;

              const char*
                value;

              value=GetImageOption(_image_info,"compose");
              compose=OverCompositeOp;  /* Default to Over */
              if (value != (const char *) NULL)
                compose=(CompositeOperator) ParseCommandOption(
                      MagickComposeOptions,MagickFalse,value);

              /* Split image sequence at the first 'NULL:' image. */
              source=_images;
              while (source != (Image *) NULL)
              {
                source=GetNextImageInList(source);
                if ((source != (Image *) NULL) &&
                    (LocaleCompare(source->magick,"NULL") == 0))
                  break;
              }
              if (source != (Image *) NULL)
                {
                  if ((GetPreviousImageInList(source) == (Image *) NULL) ||
                      (GetNextImageInList(source) == (Image *) NULL))
                    source=(Image *) NULL;
                  else
                    { /* Separate the two lists, junk the null: image.  */
                      source=SplitImageList(source->previous);
                      DeleteImageFromList(&source);
                    }
                }
              if (source == (Image *) NULL)
                {
                  (void) ThrowMagickException(_exception,GetMagickModule(),
                    OptionError,"MissingNullSeparator","layers Composite");
                  break;
                }
              /* Adjust offset with gravity and virtual canvas.  */
              SetGeometry(_images,&geometry);
              (void) ParseAbsoluteGeometry(_images->geometry,&geometry);
              geometry.width=source->page.width != 0 ?
                source->page.width : source->columns;
              geometry.height=source->page.height != 0 ?
               source->page.height : source->rows;
              GravityAdjustGeometry(_images->page.width != 0 ?
                _images->page.width : _images->columns,
                _images->page.height != 0 ? _images->page.height :
                _images->rows,_images->gravity,&geometry);

              /* Compose the two image sequences together */
              CompositeLayers(_images,compose,source,geometry.x,geometry.y,
                _exception);
              source=DestroyImageList(source);
              break;
            }
          }
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'm':
    {
      if (LocaleCompare("map",option+1) == 0)
        {
          CLIWandWarnReplaced("+remap");
          (void) RemapImages(_quantize_info,_images,(Image *) NULL,_exception);
          break;
        }
      if (LocaleCompare("metric",option+1) == 0)
        {
          (void) SetImageOption(_image_info,option+1,arg1);
          break;
        }
      if (LocaleCompare("morph",option+1) == 0)
        {
          Image
            *morph_image;

          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          morph_image=MorphImages(_images,StringToUnsignedLong(arg1),
            _exception);
          if (morph_image == (Image *) NULL)
            break;
          _images=DestroyImageList(_images);
          _images=morph_image;
          break;
        }
      if (LocaleCompare("mosaic",option+1) == 0)
        {
          /* REDIRECTED to use -layers mosaic instead */
          (void) CLIListOperatorImages(cli_wand,"-layers",option+1,NULL);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'p':
    {
      if (LocaleCompare("poly",option+1) == 0)
        {
          double
            *args;

          ssize_t
            count;

          /* convert argument string into an array of doubles */
          args = StringToArrayOfDoubles(arg1,&count,_exception);
          if (args == (double *) NULL )
            CLIWandExceptArgBreak(OptionError,"InvalidNumberList",option,arg1);
          new_images=PolynomialImage(_images,(size_t) (count >> 1),args,
           _exception);
          args=(double *) RelinquishMagickMemory(args);
          break;
        }
      if (LocaleCompare("process",option+1) == 0)
        {
          /* FUTURE: better parsing using ScriptToken() from string ??? */
          char
            **arguments;

          int
            j,
            number_arguments;

          arguments=StringToArgv(arg1,&number_arguments);
          if (arguments == (char **) NULL)
            break;
          if (strchr(arguments[1],'=') != (char *) NULL)
            {
              char
                breaker,
                quote,
                *token;

              const char
                *arguments;

              int
                next,
                status;

              size_t
                length;

              TokenInfo
                *token_info;

              /*
                Support old style syntax, filter="-option arg1".
              */
              assert(arg1 != (const char *) NULL);
              length=strlen(arg1);
              token=(char *) NULL;
              if (~length >= (MagickPathExtent-1))
                token=(char *) AcquireQuantumMemory(length+MagickPathExtent,
                  sizeof(*token));
              if (token == (char *) NULL)
                break;
              next=0;
              arguments=arg1;
              token_info=AcquireTokenInfo();
              status=Tokenizer(token_info,0,token,length,arguments,"","=",
                "\"",'\0',&breaker,&next,&quote);
              token_info=DestroyTokenInfo(token_info);
              if (status == 0)
                {
                  const char
                    *argv;

                  argv=(&(arguments[next]));
                  (void) InvokeDynamicImageFilter(token,&_images,1,&argv,
                    _exception);
                }
              token=DestroyString(token);
              break;
            }
          (void) SubstituteString(&arguments[1],"-","");
          (void) InvokeDynamicImageFilter(arguments[1],&_images,
            number_arguments-2,(const char **) arguments+2,_exception);
          for (j=0; j < number_arguments; j++)
            arguments[j]=DestroyString(arguments[j]);
          arguments=(char **) RelinquishMagickMemory(arguments);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 'r':
    {
      if (LocaleCompare("remap",option+1) == 0)
        {
          (void) RemapImages(_quantize_info,_images,(Image *) NULL,_exception);
          break;
        }
      if (LocaleCompare("reverse",option+1) == 0)
        {
          ReverseImageList(&_images);
          break;
        }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    case 's':
    {
      if (LocaleCompare("smush",option+1) == 0)
        {
          /* FUTURE: this option needs more work to make better */
          ssize_t
            offset;

          if (IsGeometry(arg1) == MagickFalse)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          offset=(ssize_t) StringToLong(arg1);
          new_images=SmushImages(_images,IsNormalOp,offset,_exception);
          break;
        }
      if (LocaleCompare("subimage",option+1) == 0)
        {
          Image
            *base_image,
            *compare_image;

          const char
            *value;

          MetricType
            metric;

          double
            similarity;

          RectangleInfo
            offset;

          base_image=GetImageFromList(_images,0);
          compare_image=GetImageFromList(_images,1);

          /* Comparision Metric */
          metric=UndefinedErrorMetric;
          value=GetImageOption(_image_info,"metric");
          if (value != (const char *) NULL)
            metric=(MetricType) ParseCommandOption(MagickMetricOptions,
              MagickFalse,value);

          new_images=SimilarityImage(base_image,compare_image,metric,0.0,
            &offset,&similarity,_exception);

          if (new_images != (Image *) NULL)
            {
              char
                result[MagickPathExtent];

              (void) FormatLocaleString(result,MagickPathExtent,"%lf",
                similarity);
              (void) SetImageProperty(new_images,"subimage:similarity",result,
                _exception);
              (void) FormatLocaleString(result,MagickPathExtent,"%+ld",(long)
                offset.x);
              (void) SetImageProperty(new_images,"subimage:x",result,
                _exception);
              (void) FormatLocaleString(result,MagickPathExtent,"%+ld",(long)
                offset.y);
              (void) SetImageProperty(new_images,"subimage:y",result,
                _exception);
              (void) FormatLocaleString(result,MagickPathExtent,
                "%lux%lu%+ld%+ld",(unsigned long) offset.width,(unsigned long)
                offset.height,(long) offset.x,(long) offset.y);
              (void) SetImageProperty(new_images,"subimage:offset",result,
                _exception);
            }
          break;
        }
      if (LocaleCompare("swap",option+1) == 0)
        {
        Image
          *p,
          *q,
          *swap;

        ssize_t
          index,
          swap_index;

        index=(-1);
        swap_index=(-2);
        if (IfNormalOp) {
          GeometryInfo
            geometry_info;

          MagickStatusType
            flags;

          swap_index=(-1);
          flags=ParseGeometry(arg1,&geometry_info);
          if ((flags & RhoValue) == 0)
            CLIWandExceptArgBreak(OptionError,"InvalidArgument",option,arg1);
          index=(ssize_t) geometry_info.rho;
          if ((flags & SigmaValue) != 0)
            swap_index=(ssize_t) geometry_info.sigma;
        }
        p=GetImageFromList(_images,index);
        q=GetImageFromList(_images,swap_index);
        if ((p == (Image *) NULL) || (q == (Image *) NULL)) {
          if (IfNormalOp)
            CLIWandExceptArgBreak(OptionError,"InvalidImageIndex",option,arg1)
          else
            CLIWandExceptionBreak(OptionError,"TwoOrMoreImagesRequired",option);
        }
        if (p == q)
          CLIWandExceptArgBreak(OptionError,"InvalidImageIndex",option,arg1);
        swap=CloneImage(p,0,0,MagickTrue,_exception);
        ReplaceImageInList(&p,CloneImage(q,0,0,MagickTrue,_exception));
        ReplaceImageInList(&q,swap);
        _images=GetFirstImageInList(q);
        break;
      }
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
    }
    default:
      CLIWandExceptionBreak(OptionError,"UnrecognizedOption",option);
  }

  /* clean up percent escape interpreted strings */
  if (arg1 != arg1n )
    arg1=DestroyString((char *)arg1);
  if (arg2 != arg2n )
    arg2=DestroyString((char *)arg2);

  /* if new image list generated, replace existing image list */
  if (new_images == (Image *) NULL)
    return(status == 0 ? MagickFalse : MagickTrue);
  _images=DestroyImageList(_images);
  _images=GetFirstImageInList(new_images);
  return(status == 0 ? MagickFalse : MagickTrue);

#undef _image_info
#undef _images
#undef _exception
#undef _draw_info
#undef _quantize_info
#undef IfNormalOp
#undef IfPlusOp
#undef IsNormalOp
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C L I N o I m a g e O p e r a t i o n s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLINoImageOperator() Applies operations that may not actually need images
%  in an image list.
%
%  The classic operators of this type is "-read", which actually creates
%  images even when no images are present.  Or image stack operators, which
%  can be applied (push or pop) to an empty image list.
%
%  Note that these operators may involve other special 'option' prefix
%  characters other  than '-' or '+', namely parenthesis and braces.
%
%  The format of the CLINoImageOption method is:
%
%      void CLINoImageOption(MagickCLI *cli_wand,const char *option,
%           const char *arg1, const char *arg2)
%
%  A description of each parameter follows:
%
%    o cli_wand: the main CLI Wand to use. (sometimes not required)
%
%    o option: The special option (with any switch char) to process
%
%    o arg1 & arg2: Argument for option, if required
%                   Currently arg2 is not used.
%
*/
WandPrivate void CLINoImageOperator(MagickCLI *cli_wand,
  const char *option,const char *arg1n,const char *arg2n)
{
  const char    /* percent escaped versions of the args */
    *arg1,
    *arg2;

#define _image_info     (cli_wand->wand.image_info)
#define _images         (cli_wand->wand.images)
#define _exception      (cli_wand->wand.exception)
#define _process_flags  (cli_wand->process_flags)
#define _option_type    ((CommandOptionFlags) cli_wand->command->flags)
#define IfNormalOp      (*option=='-')
#define IfPlusOp        (*option!='-')

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  assert(cli_wand->wand.signature == MagickWandSignature);

  if (cli_wand->wand.debug != MagickFalse)
    (void) CLILogEvent(cli_wand,CommandEvent,GetMagickModule(),
      "- NoImage Operator: %s \"%s\" \"%s\"", option,
      arg1n != (char *) NULL ? arg1n : "",
      arg2n != (char *) NULL ? arg2n : "");

  arg1 = arg1n;
  arg2 = arg2n;

  /* Interpret Percent Escapes in Arguments - using first image */
  if ( (((_process_flags & ProcessInterpretProperities) != 0 )
        || ((_option_type & AlwaysInterpretArgsFlag) != 0)
       )  && ((_option_type & NeverInterpretArgsFlag) == 0) ) {
    /* Interpret Percent escapes in argument 1 */
    if (arg1n != (char *) NULL) {
      arg1=InterpretImageProperties(_image_info,_images,arg1n,_exception);
      if (arg1 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg1=arg1n;  /* use the given argument as is */
      }
    }
    if (arg2n != (char *) NULL) {
      arg2=InterpretImageProperties(_image_info,_images,arg2n,_exception);
      if (arg2 == (char *) NULL) {
        CLIWandException(OptionWarning,"InterpretPropertyFailure",option);
        arg2=arg2n;  /* use the given argument as is */
      }
    }
  }
#undef _process_flags
#undef _option_type

  do {  /* break to exit code */
    /*
      No-op options  (ignore these)
    */
    if (LocaleCompare("noop",option+1) == 0)   /* zero argument */
      break;
    if (LocaleCompare("sans",option+1) == 0)   /* one argument */
      break;
    if (LocaleCompare("sans0",option+1) == 0)  /* zero argument */
      break;
    if (LocaleCompare("sans1",option+1) == 0)  /* one argument */
      break;
    if (LocaleCompare("sans2",option+1) == 0)  /* two arguments */
      break;
    /*
      Image Reading
    */
    if ( ( LocaleCompare("read",option+1) == 0 ) ||
      ( LocaleCompare("--",option) == 0 ) ) {
      /* Do Glob filename Expansion for 'arg1' then read all images.
      *
      * Expansion handles '@', '~', '*', and '?' meta-characters while ignoring
      * (but attaching to the filenames in the generated argument list) any
      * [...] read modifiers that may be present.
      *
      * For example: It will expand '*.gif[20x20]' into a list such as
      * 'abc.gif[20x20]',  'foobar.gif[20x20]',  'xyzzy.gif[20x20]'
      *
      * NOTE: In IMv6 this was done globally across all images. This
      * meant you could include IM options in '@filename' lists, but you
      * could not include comments.   Doing it only for image read makes
      * it far more secure.
      *
      * Note: arguments do not have percent escapes expanded for security
      * reasons.
      */
      int      argc;
      char     **argv;
      ssize_t  i;

      argc = 1;
      argv = (char **) &arg1;

      /* Expand 'glob' expressions in the given filename.
        Expansion handles any 'coder:' prefix, or read modifiers attached
        to the filename, including them in the resulting expanded list.
      */
      if (ExpandFilenames(&argc,&argv) == MagickFalse)
        CLIWandExceptArgBreak(ResourceLimitError,"MemoryAllocationFailed",
            option,GetExceptionMessage(errno));

      /* loop over expanded filename list, and read then all in */
      for (i=0; i < (ssize_t) argc; i++) {
        Image *
          new_images;
        if (_image_info->ping != MagickFalse)
          new_images=PingImages(_image_info,argv[i],_exception);
        else
          new_images=ReadImages(_image_info,argv[i],_exception);
        AppendImageToList(&_images, new_images);
        argv[i]=DestroyString(argv[i]);
      }
      argv=(char **) RelinquishMagickMemory(argv);
      break;
    }
    /*
      Image Writing
      Note: Writing a empty image list is valid in specific cases
    */
    if (LocaleCompare("write",option+1) == 0) {
      /* Note: arguments do not have percent escapes expanded */
      char
        key[MagickPathExtent];

      Image
        *write_images;

      ImageInfo
        *write_info;

      /* Need images, unless a "null:" output coder is used */
      if ( _images == (Image *) NULL ) {
        if ( LocaleCompare(arg1,"null:") == 0 )
          break;
        CLIWandExceptArgBreak(OptionError,"NoImagesForWrite",option,arg1);
      }

      (void) FormatLocaleString(key,MagickPathExtent,"cache:%s",arg1);
      (void) DeleteImageRegistry(key);
      write_images=_images;
      if (IfPlusOp)
        write_images=CloneImageList(_images,_exception);
      write_info=CloneImageInfo(_image_info);
      (void) WriteImages(write_info,write_images,arg1,_exception);
      write_info=DestroyImageInfo(write_info);
      if (IfPlusOp)
        write_images=DestroyImageList(write_images);
      break;
    }
    /*
      Parenthesis and Brace operations
    */
    if (LocaleCompare("(",option) == 0) {
      /* stack 'push' images */
      Stack
        *node;

      size_t
        size;

      size=0;
      node=cli_wand->image_list_stack;
      for ( ; node != (Stack *) NULL; node=node->next)
        size++;
      if ( size >= MAX_STACK_DEPTH )
        CLIWandExceptionBreak(OptionError,"ParenthesisNestedTooDeeply",option);
      node=(Stack *) AcquireMagickMemory(sizeof(*node));
      if (node == (Stack *) NULL)
        CLIWandExceptionBreak(ResourceLimitFatalError,
            "MemoryAllocationFailed",option);
      node->data = (void *)cli_wand->wand.images;
      node->next = cli_wand->image_list_stack;
      cli_wand->image_list_stack = node;
      cli_wand->wand.images = NewImageList();

      /* handle respect-parenthesis */
      if (IsStringTrue(GetImageOption(cli_wand->wand.image_info,
                    "respect-parenthesis")) != MagickFalse)
        option="{"; /* fall-thru so as to push image settings too */
      else
        break;
      /* fall thru to operation */
    }
    if (LocaleCompare("{",option) == 0) {
      /* stack 'push' of image_info settings */
      Stack
        *node;

      size_t
        size;

      size=0;
      node=cli_wand->image_info_stack;
      for ( ; node != (Stack *) NULL; node=node->next)
        size++;
      if ( size >= MAX_STACK_DEPTH )
        CLIWandExceptionBreak(OptionError,"CurlyBracesNestedTooDeeply",option);
      node=(Stack *) AcquireMagickMemory(sizeof(*node));
      if (node == (Stack *) NULL)
        CLIWandExceptionBreak(ResourceLimitFatalError,
            "MemoryAllocationFailed",option);

      node->data = (void *)cli_wand->wand.image_info;
      node->next = cli_wand->image_info_stack;

      cli_wand->image_info_stack = node;
      cli_wand->wand.image_info = CloneImageInfo(cli_wand->wand.image_info);
      if (cli_wand->wand.image_info == (ImageInfo *) NULL) {
        CLIWandException(ResourceLimitFatalError,"MemoryAllocationFailed",
            option);
        cli_wand->wand.image_info = (ImageInfo *)node->data;
        node = (Stack *)RelinquishMagickMemory(node);
        break;
      }

      break;
    }
    if (LocaleCompare(")",option) == 0) {
      /* pop images from stack */
      Stack
        *node;

      node = (Stack *)cli_wand->image_list_stack;
      if ( node == (Stack *) NULL)
        CLIWandExceptionBreak(OptionError,"UnbalancedParenthesis",option);
      cli_wand->image_list_stack = node->next;

      AppendImageToList((Image **)&node->data,cli_wand->wand.images);
      cli_wand->wand.images= (Image *)node->data;
      node = (Stack *)RelinquishMagickMemory(node);

      /* handle respect-parenthesis - of the previous 'pushed' settings */
      node = cli_wand->image_info_stack;
      if ( node != (Stack *) NULL)
        {
          if (IsStringTrue(GetImageOption(
                cli_wand->wand.image_info,"respect-parenthesis")) != MagickFalse)
            option="}"; /* fall-thru so as to pop image settings too */
          else
            break;
        }
      else
        break;
      /* fall thru to next if */
    }
    if (LocaleCompare("}",option) == 0) {
      /* pop image_info settings from stack */
      Stack
        *node;

      node = (Stack *)cli_wand->image_info_stack;
      if ( node == (Stack *) NULL)
        CLIWandExceptionBreak(OptionError,"UnbalancedCurlyBraces",option);
      cli_wand->image_info_stack = node->next;

      (void) DestroyImageInfo(cli_wand->wand.image_info);
      cli_wand->wand.image_info = (ImageInfo *)node->data;
      node = (Stack *)RelinquishMagickMemory(node);

      GetDrawInfo(cli_wand->wand.image_info, cli_wand->draw_info);
      cli_wand->quantize_info=DestroyQuantizeInfo(cli_wand->quantize_info);
      cli_wand->quantize_info=AcquireQuantizeInfo(cli_wand->wand.image_info);

      break;
    }
      if (LocaleCompare("print",option+1) == 0)
        {
          (void) FormatLocaleFile(stdout,"%s",arg1);
          break;
        }
    if (LocaleCompare("set",option+1) == 0)
      {
        /* Settings are applied to each image in memory in turn (if any).
           While a option: only need to be applied once globally.

           NOTE: rguments have not been automatically percent expaneded
        */

        /* escape the 'key' once only, using first image. */
        arg1=InterpretImageProperties(_image_info,_images,arg1n,_exception);
        if (arg1 == (char *) NULL)
          CLIWandExceptionBreak(OptionWarning,"InterpretPropertyFailure",
                option);

        if (LocaleNCompare(arg1,"registry:",9) == 0)
          {
            if (IfPlusOp)
              {
                (void) DeleteImageRegistry(arg1+9);
                arg1=DestroyString((char *)arg1);
                break;
              }
            arg2=InterpretImageProperties(_image_info,_images,arg2n,_exception);
            if (arg2 == (char *) NULL) {
              arg1=DestroyString((char *)arg1);
              CLIWandExceptionBreak(OptionWarning,"InterpretPropertyFailure",
                    option);
            }
            (void) SetImageRegistry(StringRegistryType,arg1+9,arg2,_exception);
            arg1=DestroyString((char *)arg1);
            arg2=DestroyString((char *)arg2);
            break;
          }
        if (LocaleNCompare(arg1,"option:",7) == 0)
          {
            /* delete equivelent artifact from all images (if any) */
            if (_images != (Image *) NULL)
              {
                MagickResetIterator(&cli_wand->wand);
                while (MagickNextImage(&cli_wand->wand) != MagickFalse)
                  (void) DeleteImageArtifact(_images,arg1+7);
                MagickResetIterator(&cli_wand->wand);
              }
            /* now set/delete the global option as needed */
            /* FUTURE: make escapes in a global 'option:' delayed */
            arg2=(char *) NULL;
            if (IfNormalOp)
              {
                arg2=InterpretImageProperties(_image_info,_images,arg2n,_exception);
                if (arg2 == (char *) NULL)
                  CLIWandExceptionBreak(OptionWarning,
                       "InterpretPropertyFailure",option);
              }
            (void) SetImageOption(_image_info,arg1+7,arg2);
            arg1=DestroyString((char *)arg1);
            arg2=DestroyString((char *)arg2);
            break;
          }
        /* Set Artifacts/Properties/Attributes all images (required) */
        if ( _images == (Image *) NULL )
          CLIWandExceptArgBreak(OptionWarning,"NoImageForProperty",option,arg1);

        MagickResetIterator(&cli_wand->wand);
        while (MagickNextImage(&cli_wand->wand) != MagickFalse)
          {
            arg2=(char *) NULL;
            if (IfNormalOp)
              {
                arg2=InterpretImageProperties(_image_info,_images,arg2n,_exception);
                if (arg2 == (char *) NULL)
                  CLIWandExceptionBreak(OptionWarning,
                       "InterpretPropertyFailure",option);
              }
            if (LocaleNCompare(arg1,"artifact:",9) == 0)
              (void) SetImageArtifact(_images,arg1+9,arg2);
            else if (LocaleNCompare(arg1,"property:",9) == 0)
              (void) SetImageProperty(_images,arg1+9,arg2,_exception);
            else
              (void) SetImageProperty(_images,arg1,arg2,_exception);
            arg2=DestroyString((char *)arg2);
          }
        MagickResetIterator(&cli_wand->wand);
        arg1=DestroyString((char *)arg1);
        break;
     }
    if (LocaleCompare("clone",option+1) == 0) {
        Image
          *new_images;

        if (*option == '+')
          arg1=AcquireString("-1");
        if (IsSceneGeometry(arg1,MagickFalse) == MagickFalse)
          CLIWandExceptionBreak(OptionError,"InvalidArgument",option);
        if ( cli_wand->image_list_stack == (Stack *) NULL)
          CLIWandExceptionBreak(OptionError,"UnableToCloneImage",option);
        new_images = (Image *)cli_wand->image_list_stack->data;
        if (new_images == (Image *) NULL)
          CLIWandExceptionBreak(OptionError,"UnableToCloneImage",option);
        new_images=CloneImages(new_images,arg1,_exception);
        if (new_images == (Image *) NULL)
          CLIWandExceptionBreak(OptionError,"NoSuchImage",option);
        AppendImageToList(&_images,new_images);
        break;
      }
    /*
       Informational Operations.

       Note that these do not require either a cli-wand or images!
       Though currently a cli-wand much be provided regardless.
    */
    if (LocaleCompare("version",option+1) == 0)
      {
        ListMagickVersion(stdout);
        break;
      }
    if (LocaleCompare("list",option+1) == 0) {
      /*
         FUTURE: This 'switch' should really be part of MagickCore
      */
      ssize_t
        list;

      list=ParseCommandOption(MagickListOptions,MagickFalse,arg1);
      if ( list < 0 ) {
        CLIWandExceptionArg(OptionError,"UnrecognizedListType",option,arg1);
        break;
      }
      switch (list)
      {
        case MagickCoderOptions:
        {
          (void) ListCoderInfo((FILE *) NULL,_exception);
          break;
        }
        case MagickColorOptions:
        {
          (void) ListColorInfo((FILE *) NULL,_exception);
          break;
        }
        case MagickConfigureOptions:
        {
          (void) ListConfigureInfo((FILE *) NULL,_exception);
          break;
        }
        case MagickDelegateOptions:
        {
          (void) ListDelegateInfo((FILE *) NULL,_exception);
          break;
        }
        case MagickFontOptions:
        {
          (void) ListTypeInfo((FILE *) NULL,_exception);
          break;
        }
        case MagickFormatOptions:
          (void) ListMagickInfo((FILE *) NULL,_exception);
          break;
        case MagickLocaleOptions:
          (void) ListLocaleInfo((FILE *) NULL,_exception);
          break;
        case MagickLogOptions:
          (void) ListLogInfo((FILE *) NULL,_exception);
          break;
        case MagickMagicOptions:
          (void) ListMagicInfo((FILE *) NULL,_exception);
          break;
        case MagickMimeOptions:
          (void) ListMimeInfo((FILE *) NULL,_exception);
          break;
        case MagickModuleOptions:
          (void) ListModuleInfo((FILE *) NULL,_exception);
          break;
        case MagickPolicyOptions:
          (void) ListPolicyInfo((FILE *) NULL,_exception);
          break;
        case MagickResourceOptions:
          (void) ListMagickResourceInfo((FILE *) NULL,_exception);
          break;
        case MagickThresholdOptions:
          (void) ListThresholdMaps((FILE *) NULL,_exception);
          break;
        default:
          (void) ListCommandOptions((FILE *) NULL,(CommandOption) list,
            _exception);
          break;
      }
      break;
    }

    CLIWandException(OptionError,"UnrecognizedOption",option);

DisableMSCWarning(4127)
  } while (0);  /* break to exit code. */
RestoreMSCWarning

  /* clean up percent escape interpreted strings */
  if (arg1 != arg1n )
    arg1=DestroyString((char *)arg1);
  if (arg2 != arg2n )
    arg2=DestroyString((char *)arg2);

#undef _image_info
#undef _images
#undef _exception
#undef IfNormalOp
#undef IfPlusOp
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C L I O p t i o n                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CLIOption() Processes the given option using the given CLI Magick Wand.
%  The option arguments can be variable in number, though at this time no more
%  that two is actually used by any option (this may change). Excess options
%  are simply ignored.
%
%  If the cli_wand->command pointer is non-null, then it is assumed that the
%  option has already been search for up from the CommandOptions[] table in
%  "MagickCore/options.c" using  GetCommandOptionInfo().  If not set this
%  routine will do the lookup instead. The pointer is reset afterward.
%
%  This action allows the caller to lookup and pre-handle any 'special'
%  options, (such as implicit reads) before calling this general option
%  handler to deal with 'standard' command line options.
%
%  The format of the CLIOption method is:
%
%       void CLIOption(MagickCLI *cli_wand,const char *option, ...)
%
%  A description of each parameter follows:
%
%     o cli_wand: the main CLI Wand to use.
%
%     o option: The special option (with any switch char) to process
%
%     o args: any required arguments for an option (variable number)
%
%  Example Usage...
%
%    CLIoption(cli_wand,"-read","rose:");
%    CLIoption(cli_wand,"-virtual-pixel","transparent");
%    CLIoption(cli_wand,"-distort","SRT:","30");
%    CLIoption(cli_wand,"-write","rotated_rose.png");
%
*/
WandExport void CLIOption(MagickCLI *cli_wand,const char *option,...)
{
  const char    /* extracted option args from args */
    *arg1,
    *arg2;

  CommandOptionFlags
    option_type;

  assert(cli_wand != (MagickCLI *) NULL);
  assert(cli_wand->signature == MagickWandSignature);
  assert(cli_wand->wand.signature == MagickWandSignature);

  do { /* Break Code Block for error handling */

    /* get information about option */
    if ( cli_wand->command == (const OptionInfo *) NULL )
      cli_wand->command = GetCommandOptionInfo(option);
#if 0
      (void) FormatLocaleFile(stderr, "CLIOption \"%s\" matched \"%s\"\n",
            option, cli_wand->command->mnemonic );
#endif
    option_type=(CommandOptionFlags) cli_wand->command->flags;

    if ( option_type == UndefinedOptionFlag )
      CLIWandExceptionReturn(OptionFatalError,"UnrecognizedOption",option);

    assert( LocaleCompare(cli_wand->command->mnemonic,option) == 0 );

    /* deprecated options */
    if ( (option_type & DeprecateOptionFlag) != 0 )
      CLIWandExceptionBreak(OptionError,"DeprecatedOptionNoCode",option);

    /* options that this module does not handle */
    if ((option_type & (SpecialOptionFlag|GenesisOptionFlag)) != 0 )
      CLIWandExceptionBreak(OptionFatalError,"InvalidUseOfOption",option);

    /* Get argument strings from VarArgs
      How can you determine if enough arguments was supplied?
      What happens if not enough arguments were supplied?
    */
    { size_t
        count = (size_t) cli_wand->command->type;

      va_list
        operands;

      va_start(operands,option);

      arg1=arg2=NULL;
      if ( count >= 1 )
        arg1=(const char *) va_arg(operands, const char *);
      if ( count >= 2 )
        arg2=(const char *) va_arg(operands, const char *);

      va_end(operands);
#if 0
      (void) FormatLocaleFile(stderr,
        "CLIOption: \"%s\"  Count: %ld  Flags: %04x  Args: \"%s\" \"%s\"\n",
            option,(long) count,option_type,arg1,arg2);
#endif
    }

    /*
      Call the appropriate option handler
    */

    /* FUTURE: this is temporary - get 'settings' to handle distribution of
      settings to images attributes,proprieties,artifacts */
    if ( cli_wand->wand.images != (Image *) NULL )
      (void) SyncImagesSettings(cli_wand->wand.image_info,cli_wand->wand.images,
        cli_wand->wand.exception);

    if ( (option_type & SettingOptionFlags) != 0 ) {
      CLISettingOptionInfo(cli_wand, option, arg1, arg2);
      // FUTURE: Sync Specific Settings into Image Properities (not global)
    }

    /* Operators that do not need images - read, write, stack, clone */
    if ((option_type & NoImageOperatorFlag) != 0)
      CLINoImageOperator(cli_wand, option, arg1, arg2);

    /* FUTURE: The not a setting part below is a temporary hack due to
    * some options being both a Setting and a Simple operator.
    * Specifically -monitor, -depth, and  -colorspace */
    if ( cli_wand->wand.images == (Image *) NULL )
      if ( ((option_type & (SimpleOperatorFlag|ListOperatorFlag)) != 0 ) &&
          ((option_type & SettingOptionFlags) == 0 ))  /* temp hack */
        CLIWandExceptionBreak(OptionError,"NoImagesFound",option);

    /* Operators which loop of individual images, simply */
    if ( (option_type & SimpleOperatorFlag) != 0 &&
         cli_wand->wand.images != (Image *) NULL) /* temp hack */
      {
        ExceptionInfo *exception=AcquireExceptionInfo();
        (void) CLISimpleOperatorImages(cli_wand, option, arg1, arg2,exception);
        exception=DestroyExceptionInfo(exception);
      }

    /* Operators that work on the image list as a whole */
    if ( (option_type & ListOperatorFlag) != 0 )
      (void) CLIListOperatorImages(cli_wand, option, arg1, arg2);

DisableMSCWarning(4127)
  } while (0);  /* end Break code block */
RestoreMSCWarning

  cli_wand->command = (const OptionInfo *) NULL; /* prevent re-use later */
}
