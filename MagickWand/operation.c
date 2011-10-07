/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%             OOO  PPPP  EEEE RRRR   AA  TTTTTT III  OOO  N   N               %
%            O   O P   P E    R   R A  A   TT    I  O   O NN  N               %
%            O   O PPPP  EEE  RRRR  AAAA   TT    I  O   O N N N               %
%            O   O P     E    R R   A  A   TT    I  O   O N  NN               %
%             OOO  P     EEEE R  RR A  A   TT   III  OOO  N   N               %
%                                                                             %
%                                                                             %
%                         MagickWand Module Methods                           %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               September 2011                                %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
% Apply the given options (settings, and simple, or sequence operations) to
% the given image(s) according to the current "image_info" and "draw_info"
% settings.
%
% The final goal is to allow the execution in a strict one option at a time
% manner that is needed for 'pipelining and file scripting' of options in
% IMv7.
%
% Anthony Thyssen, Sept 2011
*/
#if 0

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/mogrify-private.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/string-private.h"

/*
  Define declarations.
*/
#define UndefinedCompressionQuality  0UL
/*
  Constant declaration. (temporary exports)
*/
static const char
  BackgroundColor[] = "#fff",  /* white */
  BorderColor[] = "#dfdfdf",  /* gray */
  MatteColor[] = "#bdbdbd";  /* gray */

/*
** Function to report on the progress of image operations
*/
static MagickBooleanType MonitorProgress(const char *text,
  const MagickOffsetType offset,const MagickSizeType extent,
  void *wand_unused(client_data))
{
  char
    message[MaxTextExtent],
    tag[MaxTextExtent];

  const char
    *locale_message;

  register char
    *p;

  if (extent < 2)
    return(MagickTrue);
  (void) CopyMagickMemory(tag,text,MaxTextExtent);
  p=strrchr(tag,'/');
  if (p != (char *) NULL)
    *p='\0';
  (void) FormatLocaleString(message,MaxTextExtent,"Monitor/%s",tag);
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
    key[MaxTextExtent];

  ExceptionInfo
    *sans_exception;

  Image
    *image;

  ImageInfo
    *read_info;

  (void) FormatLocaleString(key,MaxTextExtent,"cache:%s",path);
  sans_exception=AcquireExceptionInfo();
  image=(Image *) GetImageRegistry(ImageRegistryType,key,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if (image != (Image *) NULL)
    return(image);
  read_info=CloneImageInfo(image_info);
  (void) CopyMagickString(read_info->filename,path,MaxTextExtent);
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
  needed to represent a color varies depending on teh current channel
  setting.
*/
static Image *SparseColorOption(const Image *image,
  const SparseColorMethod method,const char *arguments,
  const MagickBooleanType color_from_image,ExceptionInfo *exception)
{
  char
    token[MaxTextExtent];

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
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  /*
    Limit channels according to image - and add up number of color channel.
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
      (image->matte != MagickFalse))
    number_colors++;

  /*
    Read string, to determine number of arguments needed,
  */
  p=arguments;
  x=0;
  while( *p != '\0' )
  {
    GetMagickToken(p,&p,token);
    if ( token[0] == ',' ) continue;
    if ( isalpha((int) token[0]) || token[0] == '#' ) {
      if ( color_from_image ) {
        (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError, "InvalidArgument", "`%s': %s", "sparse-color",
            "Color arg given, when colors are coming from image");
        return( (Image *)NULL);
      }
      x += number_colors;  /* color argument */
    }
    else {
      x++;   /* floating point argument */
    }
  }
  error=MagickTrue;
  if ( color_from_image ) {
    /* just the control points are being given */
    error = ( x % 2 != 0 ) ? MagickTrue : MagickFalse;
    number_arguments=(x/2)*(2+number_colors);
  }
  else {
    /* control points and color values */
    error = ( x % (2+number_colors) != 0 ) ? MagickTrue : MagickFalse;
    number_arguments=x;
  }
  if ( error ) {
    (void) ThrowMagickException(exception,GetMagickModule(),
               OptionError, "InvalidArgument", "`%s': %s", "sparse-color",
               "Invalid number of Arguments");
    return( (Image *)NULL);
  }

  /* Allocate and fill in the floating point arguments */
  sparse_arguments=(double *) AcquireQuantumMemory(number_arguments,
    sizeof(*sparse_arguments));
  if (sparse_arguments == (double *) NULL) {
    (void) ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
      "MemoryAllocationFailed","%s","SparseColorOption");
    return( (Image *)NULL);
  }
  (void) ResetMagickMemory(sparse_arguments,0,number_arguments*
    sizeof(*sparse_arguments));
  p=arguments;
  x=0;
  while( *p != '\0' && x < number_arguments ) {
    /* X coordinate */
    token[0]=','; while ( token[0] == ',' ) GetMagickToken(p,&p,token);
    if ( token[0] == '\0' ) break;
    if ( isalpha((int) token[0]) || token[0] == '#' ) {
      (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError, "InvalidArgument", "`%s': %s", "sparse-color",
            "Color found, instead of X-coord");
      error = MagickTrue;
      break;
    }
    sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
    /* Y coordinate */
    token[0]=','; while ( token[0] == ',' ) GetMagickToken(p,&p,token);
    if ( token[0] == '\0' ) break;
    if ( isalpha((int) token[0]) || token[0] == '#' ) {
      (void) ThrowMagickException(exception,GetMagickModule(),
            OptionError, "InvalidArgument", "`%s': %s", "sparse-color",
            "Color found, instead of Y-coord");
      error = MagickTrue;
      break;
    }
    sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
    /* color values for this control point */
#if 0
    if ( (color_from_image ) {
      /* get color from image */
      /* HOW??? */
    }
    else
#endif
    {
      /* color name or function given in string argument */
      token[0]=','; while ( token[0] == ',' ) GetMagickToken(p,&p,token);
      if ( token[0] == '\0' ) break;
      if ( isalpha((int) token[0]) || token[0] == '#' ) {
        /* Color string given */
        (void) QueryMagickColorCompliance(token,AllCompliance,&color,
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
            (image->matte != MagickFalse))
          sparse_arguments[x++] = QuantumScale*color.alpha;
      }
      else {
        /* Colors given as a set of floating point values - experimental */
        /* NB: token contains the first floating point value to use! */
        if ((GetPixelRedTraits(image) & UpdatePixelTrait) != 0)
          {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if ((GetPixelGreenTraits(image) & UpdatePixelTrait) != 0)
          {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if ((GetPixelBlueTraits(image) & UpdatePixelTrait) != 0)
          {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if (((GetPixelBlackTraits(image) & UpdatePixelTrait) != 0) &&
            (image->colorspace == CMYKColorspace))
          {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if (((GetPixelAlphaTraits(image) & UpdatePixelTrait) != 0) &&
            (image->matte != MagickFalse))
          {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=InterpretLocaleValue(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
      }
    }
  }
  if ( number_arguments != x && !error ) {
    (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
      "InvalidArgument","`%s': %s","sparse-color","Argument Parsing Error");
    sparse_arguments=(double *) RelinquishMagickMemory(sparse_arguments);
    return( (Image *)NULL);
  }
  if ( error )
    return( (Image *)NULL);

  /* Call the Interpolation function with the parsed arguments */
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
+   A p p l y S e t t i n g O p t i o n                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ApplySettingOption() saves the given single settings option into a CLI wand
%  holding the image_info, draw_info, quantize_info structures that is later
%  used for reading, processing, and writing images.
%
%  No image in the wand is actually modified (setting options only)
%
%  The format of the ApplySettingOption method is:
%
%    MagickBooleanType ApplySettingOption(MagickWand *wand,
%        const int argc, const char **argv,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o wand: structure holding settings to be applied
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType ApplySettingsOption(ImageInfo *image_info,
  const int argc,const char **argv,ExceptionInfo *exception)
{
  GeometryInfo
    geometry_info;

  ImageInfo
    *image_info;

  DrawInfo
    *draw_info;

  const char
    *option;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  assert(wand->draw_info != (DrawInfo *) NULL); /* ensure it is a CLI wand */
  assert(wand->quantize_info == (QuantizeInfo *) NULL);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (argc < 0)
    return(MagickTrue);

  option=argv[0]+1;
  image_info=wand->image_info;
  draw_info=wand->_info;

#define DeleteOption (const char*)NULL
#define IfSetOption ((*argv[0])=='-')

  switch (*option)
  {
    case 'a':
    {
      if (LocaleCompare("adjoin",option) == 0)
        {
          image_info->adjoin = IfSetOption ? MagickTrue : MagickFalse;
          break;
        }
      if (LocaleCompare("affine",option) == 0)
        {
          if (IfSetOption)
            (void) ParseAffineGeometry(argv[1],draw_info->affine,
               exception);
          else
            GetAffineMatrix(draw_info->affine);
          break;
        }
      if (LocaleCompare("antialias",option) == 0)
        {
          image_info->antialias =
          draw_info->stroke_antialias =
          draw_info->text_antialias =
               IfSetOption ? MagickTrue : MagickFalse;
          break;
        }
      if (LocaleCompare("attenuate",option) == 0)
        {
          (void) SetImageOption(image_info,option,
               IfSetOption ? argv[1] : DeleteOption);
          break;
        }
      if (LocaleCompare("authenticate",option) == 0)
        {
          (void) SetImageOption(image_info,option,
               IfSetOption ? argv[1] : DeleteOption);
          break;
        }
      break;
    }
    case 'b':
    {
      if (LocaleCompare("background",option) == 0)
        {
          /* FUTURE: both image_info attribute & ImageOption in use!
             Note that +background, means fall-back to image attribute
             so ImageOption is deleted, not set to a default.
          */
          if (IfSetOption)
            {
              (void) DeleteImageOption(image_info,option);
              (void) QueryColorCompliance(BackgroundColor,AllCompliance,
                    image_info->background_color,exception);
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          (void) QueryColorCompliance(argv[1],AllCompliance,
              image_info->background_color,exception);
          break;
        }
      if (LocaleCompare("bias",option) == 0)
        {
          /* FUTURE: bias OBSOLETED, replaced by "convolve:bias"
             as it is actually rarely used except in direct convolve
             Usage outside direct convolve is actally non-sensible!
          */
          (void) SetImageOption(image_info,option,
               IfSetOption ? argv[1] : "0");
          break;
        }
      if (LocaleCompare("black-point-compensation",option) == 0)
        {
          (void) SetImageOption(image_info,option,
               IfSetOption ? "true" : "false" );
          break;
        }
      if (LocaleCompare("blue-primary",option) == 0)
        {
          (void) SetImageOption(image_info,option,
               IfSetOption ? argv[1] : "0" );
          break;
        }
      if (LocaleCompare("bordercolor",option) == 0)
        {
          /* FUTURE: both image_info attribute & ImageOption in use! */
          if (IfSetOption)
            {
              (void) SetImageOption(image_info,option,argv[1]);
              (void) QueryColorCompliance(argv[1],AllCompliece,
                  &image_info->border_color,exception);
              (void) QueryColorCompliance(argv[1],AllCompliance,
                  &draw_info->border_color,exception);
              break;
            }
          (void) DeleteImageOption(image_info,option);
          (void) QueryColorCompliance(BorderColor,AllCompliance,
            &image_info->border_color,exception);
          (void) QueryColorCompliance(BorderColor,AllCompliance,
            &draw_info->border_color,exception);
          break;
        }
      if (LocaleCompare("box",option) == 0)
        {
          const char
            *value = IfSetOption ? argv[1] : "none";
          (void) SetImageOption(image_info,option,value);
          (void) QueryColorCompliance(value,AllCompliance,
               &draw_info->undercolor,exception);
          break;
        }
      break;
    }
    case 'c':
    {
      if (LocaleCompare("cache",option) == 0)
        {
          MagickSizeType
            limit;

          limit=MagickResourceInfinity;
          if (LocaleCompare("unlimited",argv[1]) != 0)
            limit=(MagickSizeType) SiPrefixToDouble(argv[1],100.0);
          (void) SetMagickResourceLimit(MemoryResource,limit);
          (void) SetMagickResourceLimit(MapResource,2*limit);
          break;
        }
      if (LocaleCompare("caption",option) == 0)
        {
          (void) SetImageOption(image_info,option,
               IfSetOption ? argv[1] : DeleteOption);
          break;
        }
      if (LocaleCompare("channel",option) == 0)
        {
          image_info->channel=(ChannelType) (
               IfSetOption ? ParseChannelOption(argv[1]) : DefaultChannels );
          /* This is also a SimpleImageOperator */
          break;
        }
      if (LocaleCompare("colorspace",option) == 0)
        {
          /* This is also a SimpleImageOperator */
          if (IfSetOption)
            {
              image_info->colorspace=(ColorspaceType) ParseCommandOption(
                MagickColorspaceOptions,MagickFalse,argv[1]);
              break;
            }
          image_info->colorspace=UndefinedColorspace;
          (void) SetImageOption(image_info,option,"undefined");
          break;
        }
      if (LocaleCompare("comment",option) == 0)
        {
          (void) SetImageOption(image_info,option,
               IfSetOption ? argv[1] : DeleteOption);
          break;
        }
      if (LocaleCompare("compose",option) == 0)
        {
          /* FUTURE: What should be used?  image_info  or ImageOption ???
             The former is more efficent, the later cristy prefers!
          */
          const char
            *value;

          value = IfSetOption ? argv[1] : "undefined";
          (void) SetImageOption(image_info,option,value);
          image_info->compose=(CompositeOperator) ParseCommandOption(
               MagickComposeOptions,MagickFalse,value);
          break;
        }
      if (LocaleCompare("compress",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->compression=UndefinedCompression;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->compression=(CompressionType) ParseCommandOption(
            MagickCompressOptions,MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'd':
    {
      if (LocaleCompare("debug",option) == 0)
        {
          if (*argv[0] == '+')
            (void) SetLogEventMask("none");
          else
            (void) SetLogEventMask(argv[1]);
          image_info->debug=IsEventLogging();
          break;
        }
      if (LocaleCompare("define",option) == 0)
        {
          if (*argv[0] == '+')
            {
              if (LocaleNCompare(argv[1],"registry:",9) == 0)
                (void) DeleteImageRegistry(argv[1]+9);
              else
                (void) DeleteImageOption(image_info,argv[1]);
              break;
            }
          if (LocaleNCompare(argv[1],"registry:",9) == 0)
            {
              (void) DefineImageRegistry(StringRegistryType,argv[1]+9,
                exception);
              break;
            }
          (void) DefineImageOption(image_info,argv[1]);
          break;
        }
      if (LocaleCompare("delay",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("density",option) == 0)
        {
          /*
            Set image density.
          */
          if (*argv[0] == '+')
            {
              if (image_info->density != (char *) NULL)
                image_info->density=DestroyString(image_info->density);
              (void) SetImageOption(image_info,option,"72");
              break;
            }
          (void) CloneString(&image_info->density,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("depth",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->depth=MAGICKCORE_QUANTUM_DEPTH;
              break;
            }
          image_info->depth=StringToUnsignedLong(argv[1]);
          break;
        }
      if (LocaleCompare("direction",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("display",option) == 0)
        {
          if (*argv[0] == '+')
            {
              if (image_info->server_name != (char *) NULL)
                image_info->server_name=DestroyString(
                  image_info->server_name);
              break;
            }
          (void) CloneString(&image_info->server_name,argv[1]);
          break;
        }
      if (LocaleCompare("dispose",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("dither",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->dither=MagickFalse;
              (void) SetImageOption(image_info,option,"none");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          image_info->dither=MagickTrue;
          break;
        }
      break;
    }
    case 'e':
    {
      if (LocaleCompare("encoding",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("endian",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->endian=UndefinedEndian;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->endian=(EndianType) ParseCommandOption(
            MagickEndianOptions,MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("extract",option) == 0)
        {
          /*
            Set image extract geometry.
          */
          if (*argv[0] == '+')
            {
              if (image_info->extract != (char *) NULL)
                image_info->extract=DestroyString(image_info->extract);
              break;
            }
          (void) CloneString(&image_info->extract,argv[1]);
          break;
        }
      break;
    }
    case 'f':
    {
      if (LocaleCompare("fill",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"none");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("filter",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("font",option) == 0)
        {
          if (*argv[0] == '+')
            {
              if (image_info->font != (char *) NULL)
                image_info->font=DestroyString(image_info->font);
              break;
            }
          (void) CloneString(&image_info->font,argv[1]);
          break;
        }
      if (LocaleCompare("format",option) == 0)
        {
          register const char
            *q;

          for (q=strchr(argv[1],'%'); q != (char *) NULL; q=strchr(q+1,'%'))
            if (strchr("Agkrz@[#",*(q+1)) != (char *) NULL)
              image_info->ping=MagickFalse;
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("fuzz",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->fuzz=0.0;
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          image_info->fuzz=SiPrefixToDouble(argv[1],(double) QuantumRange+
            1.0);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'g':
    {
      if (LocaleCompare("gravity",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("green-primary",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0.0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'i':
    {
      if (LocaleCompare("intent",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("interlace",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->interlace=UndefinedInterlace;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->interlace=(InterlaceType) ParseCommandOption(
            MagickInterlaceOptions,MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("interline-spacing",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("interpolate",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("interword-spacing",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'k':
    {
      if (LocaleCompare("kerning",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'l':
    {
      if (LocaleCompare("label",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) DeleteImageOption(image_info,option);
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("limit",option) == 0)
        {
          MagickSizeType
            limit;

          ResourceType
            type;

          if (*argv[0] == '+')
            break;
          type=(ResourceType) ParseCommandOption(MagickResourceOptions,
            MagickFalse,argv[1]);
          limit=MagickResourceInfinity;
          if (LocaleCompare("unlimited",argv[2]) != 0)
            limit=(MagickSizeType) SiPrefixToDouble(argv[2],100.0);
          (void) SetMagickResourceLimit(type,limit);
          break;
        }
      if (LocaleCompare("list",option) == 0)
        {
          ssize_t
            list;

          /*
            Display configuration list.
          */
          list=ParseCommandOption(MagickListOptions,MagickFalse,argv[1]);
          switch (list)
          {
            case MagickCoderOptions:
            {
              (void) ListCoderInfo((FILE *) NULL,exception);
              break;
            }
            case MagickColorOptions:
            {
              (void) ListColorInfo((FILE *) NULL,exception);
              break;
            }
            case MagickConfigureOptions:
            {
              (void) ListConfigureInfo((FILE *) NULL,exception);
              break;
            }
            case MagickDelegateOptions:
            {
              (void) ListDelegateInfo((FILE *) NULL,exception);
              break;
            }
            case MagickFontOptions:
            {
              (void) ListTypeInfo((FILE *) NULL,exception);
              break;
            }
            case MagickFormatOptions:
            {
              (void) ListMagickInfo((FILE *) NULL,exception);
              break;
            }
            case MagickLocaleOptions:
            {
              (void) ListLocaleInfo((FILE *) NULL,exception);
              break;
            }
            case MagickLogOptions:
            {
              (void) ListLogInfo((FILE *) NULL,exception);
              break;
            }
            case MagickMagicOptions:
            {
              (void) ListMagicInfo((FILE *) NULL,exception);
              break;
            }
            case MagickMimeOptions:
            {
              (void) ListMimeInfo((FILE *) NULL,exception);
              break;
            }
            case MagickModuleOptions:
            {
              (void) ListModuleInfo((FILE *) NULL,exception);
              break;
            }
            case MagickPolicyOptions:
            {
              (void) ListPolicyInfo((FILE *) NULL,exception);
              break;
            }
            case MagickResourceOptions:
            {
              (void) ListMagickResourceInfo((FILE *) NULL,exception);
              break;
            }
            case MagickThresholdOptions:
            {
              (void) ListThresholdMaps((FILE *) NULL,exception);
              break;
            }
            default:
            {
              (void) ListCommandOptions((FILE *) NULL,(CommandOption) list,
                exception);
              break;
            }
          }
          break;
        }
      if (LocaleCompare("log",option) == 0)
        {
          if (*argv[0] == '+')
            break;
          (void) SetLogFormat(argv[1]);
          break;
        }
      if (LocaleCompare("loop",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'm':
    {
      if (LocaleCompare("matte",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"false");
              break;
            }
          (void) SetImageOption(image_info,option,"true");
          break;
        }
      if (LocaleCompare("mattecolor",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,argv[1]);
              (void) QueryColorCompliance(MatteColor,AllCompliance,
                &image_info->matte_color,exception);
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          (void) QueryColorCompliance(argv[1],AllCompliance,&image_info->matte_color,
            exception);
          break;
        }
      if (LocaleCompare("monitor",option) == 0)
        {
          (void) SetImageInfoProgressMonitor(image_info,MonitorProgress,
            (void *) NULL);
          break;
        }
      if (LocaleCompare("monochrome",option) == 0)
        {
          image_info->monochrome=(*argv[0] == '-') ? MagickTrue : MagickFalse;
          break;
        }
      break;
    }
    case 'o':
    {
      if (LocaleCompare("orient",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->orientation=UndefinedOrientation;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->orientation=(OrientationType) ParseCommandOption(
            MagickOrientationOptions,MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
    }
    case 'p':
    {
      if (LocaleCompare("page",option) == 0)
        {
          char
            *canonical_page,
            page[MaxTextExtent];

          const char
            *image_option;

          MagickStatusType
            flags;

          RectangleInfo
            geometry;

          if (*argv[0] == '+')
            {
              (void) DeleteImageOption(image_info,option);
              (void) CloneString(&image_info->page,(char *) NULL);
              break;
            }
          (void) ResetMagickMemory(&geometry,0,sizeof(geometry));
          image_option=GetImageOption(image_info,"page");
          if (image_option != (const char *) NULL)
            flags=ParseAbsoluteGeometry(image_option,&geometry);
          canonical_page=GetPageGeometry(argv[1]);
          flags=ParseAbsoluteGeometry(canonical_page,&geometry);
          canonical_page=DestroyString(canonical_page);
          (void) FormatLocaleString(page,MaxTextExtent,"%lux%lu",
            (unsigned long) geometry.width,(unsigned long) geometry.height);
          if (((flags & XValue) != 0) || ((flags & YValue) != 0))
            (void) FormatLocaleString(page,MaxTextExtent,"%lux%lu%+ld%+ld",
              (unsigned long) geometry.width,(unsigned long) geometry.height,
              (long) geometry.x,(long) geometry.y);
          (void) SetImageOption(image_info,option,page);
          (void) CloneString(&image_info->page,page);
          break;
        }
      if (LocaleCompare("pen",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"none");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("ping",option) == 0)
        {
          image_info->ping=(*argv[0] == '-') ? MagickTrue : MagickFalse;
          break;
        }
      if (LocaleCompare("pointsize",option) == 0)
        {
          if (*argv[0] == '+')
            geometry_info.rho=0.0;
          else
            (void) ParseGeometry(argv[1],&geometry_info);
          image_info->pointsize=geometry_info.rho;
          break;
        }
      if (LocaleCompare("precision",option) == 0)
        {
          (void) SetMagickPrecision(StringToInteger(argv[1]));
          break;
        }
      if (LocaleCompare("preview",option) == 0)
        {
          /*
            Preview image.
          */
          if (*argv[0] == '+')
            {
              image_info->preview_type=UndefinedPreview;
              break;
            }
          image_info->preview_type=(PreviewType) ParseCommandOption(
            MagickPreviewOptions,MagickFalse,argv[1]);
          break;
        }
      break;
    }
    case 'q':
    {
      if (LocaleCompare("quality",option) == 0)
        {
          /*
            Set image compression quality.
          */
          if (*argv[0] == '+')
            {
              image_info->quality=UndefinedCompressionQuality;
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          image_info->quality=StringToUnsignedLong(argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("quiet",option) == 0)
        {
          static WarningHandler
            warning_handler = (WarningHandler) NULL;

          if (*argv[0] == '+')
            {
              /*
                Restore error or warning messages.
              */
              warning_handler=SetWarningHandler(warning_handler);
              break;
            }
          /*
            Suppress error or warning messages.
          */
          warning_handler=SetWarningHandler((WarningHandler) NULL);
          break;
        }
      break;
    }
    case 'r':
    {
      if (LocaleCompare("red-primary",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0.0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 's':
    {
      if (LocaleCompare("sampling-factor",option) == 0)
        {
          /*
            Set image sampling factor.
          */
          if (*argv[0] == '+')
            {
              if (image_info->sampling_factor != (char *) NULL)
                image_info->sampling_factor=DestroyString(
                  image_info->sampling_factor);
              break;
            }
          (void) CloneString(&image_info->sampling_factor,argv[1]);
          break;
        }
      if (LocaleCompare("scene",option) == 0)
        {
          /*
            Set image scene.
          */
          if (*argv[0] == '+')
            {
              image_info->scene=0;
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          image_info->scene=StringToUnsignedLong(argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("seed",option) == 0)
        {
          size_t
            seed;

          if (*argv[0] == '+')
            {
              seed=(size_t) time((time_t *) NULL);
              SeedPseudoRandomGenerator(seed);
              break;
            }
          seed=StringToUnsignedLong(argv[1]);
          SeedPseudoRandomGenerator(seed);
          break;
        }
      if (LocaleCompare("size",option) == 0)
        {
          /* FUTURE: convert to ImageOption
             Look at special handling for "size" in SetImageOption()
           */
          if (*argv[0] == '+')
            {
              if (image_info->size != (char *) NULL)
                image_info->size=DestroyString(image_info->size);
              break;
            }
          (void) CloneString(&image_info->size,argv[1]);
          break;
        }
      if (LocaleCompare("stroke",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"none");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("strokewidth",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("synchronize",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->synchronize=MagickFalse;
              break;
            }
          image_info->synchronize=MagickTrue;
          break;
        }
      break;
    }
    case 't':
    {
      if (LocaleCompare("taint",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"false");
              break;
            }
          (void) SetImageOption(image_info,option,"true");
          break;
        }
      if (LocaleCompare("texture",option) == 0)
        {
          if (*argv[0] == '+')
            {
              if (image_info->texture != (char *) NULL)
                image_info->texture=DestroyString(image_info->texture);
              break;
            }
          (void) CloneString(&image_info->texture,argv[1]);
          break;
        }
      if (LocaleCompare("tile-offset",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("transparent-color",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) QueryColorCompliance("none",AllCompliance,
                  &image_info->transparent_color,exception);
              (void) SetImageOption(image_info,option,"none");
              break;
            }
              (void) QueryColorCompliance("none",AllCompliance,
                  &image_info->transparent_color,exception);
            exception);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("type",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->type=UndefinedType;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->type=(ImageType) ParseCommandOption(MagickTypeOptions,
            MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'u':
    {
      if (LocaleCompare("undercolor",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) DeleteImageOption(image_info,option);
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      if (LocaleCompare("units",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->units=UndefinedResolution;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->units=(ResolutionType) ParseCommandOption(
            MagickResolutionOptions,MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'v':
    {
      if (LocaleCompare("verbose",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->verbose=MagickFalse;
              break;
            }
          image_info->verbose=MagickTrue;
          image_info->ping=MagickFalse;
          break;
        }
      if (LocaleCompare("view",option) == 0)
        {
          if (*argv[0] == '+')
            {
              if (image_info->view != (char *) NULL)
                image_info->view=DestroyString(image_info->view);
              break;
            }
          (void) CloneString(&image_info->view,argv[1]);
          break;
        }
      if (LocaleCompare("virtual-pixel",option) == 0)
        {
          if (*argv[0] == '+')
            {
              image_info->virtual_pixel_method=UndefinedVirtualPixelMethod;
              (void) SetImageOption(image_info,option,"undefined");
              break;
            }
          image_info->virtual_pixel_method=(VirtualPixelMethod)
            ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,argv[1]);
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    case 'w':
    {
      if (LocaleCompare("white-point",option) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageOption(image_info,option,"0.0");
              break;
            }
          (void) SetImageOption(image_info,option,argv[1]);
          break;
        }
      break;
    }
    default:
      break;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     A p p l y I m a g e O p e r a t o r                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ApplyImageOperator() apply one simple image operation to just the current
%  image.
%
%  The image in the list may be modified in three different ways...
%
%    * directly modified (EG: -negate, -gamma, -level, -annotate, -draw),
%    * replaced by a new image (EG: -spread, -resize, -rotate, -morphology)
%    * replace by a list of images (-separate and -crop only!)
%
%  In each case the result is returned into the list, and the pointer to the
%  modified image (last image added if replaced by a list of images) is
%  returned.  As the image pointed to may be replaced, the first image in the
%  list may also change.  GetFirstImageInList() should be used by caller if
%  they wish return the Image pointer to the first image in list.
%
%  The format of the ApplyImageOperator method is:
%
%    MagickBooleanType ApplyImageOperator(MagickWand *wand,
%        const int argc,const char **argv)
%
%  A description of each parameter follows:
%
%    o wand: The CLI wand holding all the settings and pointer to image
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ApplyImageOperator(MagickWand *wand,
     const int wand_unused(argc), const char **argv, ExceptionInfo *exception)
{
  Image *
    new_image;

  ChannelType
    channel;

  const char
    *format;

  DrawInfo
    *draw_info;

  GeometryInfo
    geometry_info;

  RectangleInfo
    geometry;

  MagickStatusType
    status;

  PixelInfo
    fill;

  MagickStatusType
    flags;

  QuantizeInfo
    *quantize_info;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  if (argc < 0)
    return(MagickTrue);
  draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  quantize_info=AcquireQuantizeInfo(image_info);
  SetGeometryInfo(&geometry_info);
  GetPixelInfo(*image,&fill);
  SetPixelInfoPacket(*image,&(*image)->background_color,&fill);
  channel=image_info->channel;
  format=GetImageOption(image_info,"format");

  new_image = (Image *)NULL;

  switch (*(argv[0]+1))
  {
    case 'a':
    {
      if (LocaleCompare("adaptive-blur",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.0;
          new_image=AdaptiveBlurImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("adaptive-resize",argv[0]+1) == 0)
        {
          /* FUTURE: this is really a "interpolate-resize" operator
             "adaptive-resize" uses a fixed "Mesh" interpolation
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=AdaptiveResizeImage(*image,geometry.width,
            geometry.height,interpolate_method,exception);
          break;
        }
      if (LocaleCompare("adaptive-sharpen",argv[0]+1) == 0)
        {
          /*
            Adaptive sharpen image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.0;
          new_image=AdaptiveSharpenImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("alpha",argv[0]+1) == 0)
        {
          AlphaChannelType
            alpha_type;

          (void) SyncImageSettings(image_info,*image);
          alpha_type=(AlphaChannelType) ParseCommandOption(MagickAlphaOptions,
            MagickFalse,argv[1]);
          (void) SetImageAlphaChannel(*image,alpha_type,exception);
          break;
        }
      if (LocaleCompare("annotate",argv[0]+1) == 0)
        {
          char
            *text,
            geometry[MaxTextExtent];

          (void) SyncImageSettings(image_info,*image);
          SetGeometryInfo(&geometry_info);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          text=InterpretImageProperties(image_info,*image,argv[2],
            exception);
          if (text == (char *) NULL)
            break;
          (void) CloneString(&draw_info->text,text);
          text=DestroyString(text);
          (void) FormatLocaleString(geometry,MaxTextExtent,"%+f%+f",
            geometry_info.xi,geometry_info.psi);
          (void) CloneString(&draw_info->geometry,geometry);
          draw_info->affine.sx=cos(DegreesToRadians(
            fmod(geometry_info.rho,360.0)));
          draw_info->affine.rx=sin(DegreesToRadians(
            fmod(geometry_info.rho,360.0)));
          draw_info->affine.ry=(-sin(DegreesToRadians(
            fmod(geometry_info.sigma,360.0))));
          draw_info->affine.sy=cos(DegreesToRadians(
            fmod(geometry_info.sigma,360.0)));
          (void) AnnotateImage(*image,draw_info,exception);
          break;
        }
      if (LocaleCompare("auto-gamma",argv[0]+1) == 0)
        {
          /*
            Auto Adjust Gamma of image based on its mean
          */
          (void) SyncImageSettings(image_info,*image);
          (void) AutoGammaImage(*image,exception);
          break;
        }
      if (LocaleCompare("auto-level",argv[0]+1) == 0)
        {
          /*
            Perfectly Normalize (max/min stretch) the image
          */
          (void) SyncImageSettings(image_info,*image);
          (void) AutoLevelImage(*image,exception);
          break;
        }
      if (LocaleCompare("auto-orient",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          switch ((*image)->orientation)
          {
            case TopRightOrientation:
            {
              new_image=FlopImage(*image,exception);
              break;
            }
            case BottomRightOrientation:
            {
              new_image=RotateImage(*image,180.0,exception);
              break;
            }
            case BottomLeftOrientation:
            {
              new_image=FlipImage(*image,exception);
              break;
            }
            case LeftTopOrientation:
            {
              new_image=TransposeImage(*image,exception);
              break;
            }
            case RightTopOrientation:
            {
              new_image=RotateImage(*image,90.0,exception);
              break;
            }
            case RightBottomOrientation:
            {
              new_image=TransverseImage(*image,exception);
              break;
            }
            case LeftBottomOrientation:
            {
              new_image=RotateImage(*image,270.0,exception);
              break;
            }
            default:
              break;
          }
          if (new_image != (Image *) NULL)
            new_image->orientation=TopLeftOrientation;
          break;
        }
      break;
    }
    case 'b':
    {
      if (LocaleCompare("black-threshold",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) BlackThresholdImage(*image,argv[1],exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("blue-shift",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          geometry_info.rho=1.5;
          if (*argv[0] == '-')
            flags=ParseGeometry(argv[1],&geometry_info);
          new_image=BlueShiftImage(*image,geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("blur",argv[0]+1) == 0)
        {
          /* FUTURE: use of "bias" in a blur is non-sensible */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.0;
          new_image=BlurImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("border",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
          if ((flags & SigmaValue) == 0)
            geometry.height=geometry.width;
          new_image=BorderImage(*image,&geometry,compose,exception);
          break;
        }
      if (LocaleCompare("brightness-contrast",argv[0]+1) == 0)
        {
          double
            brightness,
            contrast;

          GeometryInfo
            geometry_info;

          MagickStatusType
            flags;

          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          brightness=geometry_info.rho;
          contrast=0.0;
          if ((flags & SigmaValue) != 0)
            contrast=geometry_info.sigma;
          (void) BrightnessContrastImage(*image,brightness,contrast,
            exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      break;
    }
    case 'c':
    {
      if (LocaleCompare("cdl",argv[0]+1) == 0)
        {
          char
            *color_correction_collection;

          /*
            Color correct with a color decision list.
          */
          (void) SyncImageSettings(image_info,*image);
          color_correction_collection=FileToString(argv[1],~0,exception);
          if (color_correction_collection == (char *) NULL)
            break;
          (void) ColorDecisionListImage(*image,color_correction_collection,
            exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("channel",argv[0]+1) == 0)
        {
          /* The "channel" setting has already been set */
          SetPixelChannelMap(*image,image_info->channel);
          break;
        }
      if (LocaleCompare("charcoal",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=1.0;
          new_image=CharcoalImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("chop",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) ParseGravityGeometry(*image,argv[1],&geometry,exception);
          new_image=ChopImage(*image,&geometry,exception);
          break;
        }
      if (LocaleCompare("clamp",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) ClampImage(*image);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("clip",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              (void) SetImageClipMask(*image,(Image *) NULL,exception);
              break;
            }
          (void) ClipImage(*image,exception);
          break;
        }
      if (LocaleCompare("clip-mask",argv[0]+1) == 0)
        {
          CacheView
            *mask_view;

          Image
            *mask_image;

          register Quantum
            *restrict q;

          register ssize_t
            x;

          ssize_t
            y;

          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              /* Remove the write mask */
              (void) SetImageMask(*image,(Image *) NULL,exception);
              break;
            }
          mask_image=GetImageCache(image_info,argv[1],exception);
          if (mask_image == (Image *) NULL)
            break;
          if (SetImageStorageClass(mask_image,DirectClass,exception) == MagickFalse)
            return(MagickFalse);
          /* create a write mask from clip-mask image */
          /* FUTURE: use Alpha operations instead */
          mask_view=AcquireCacheView(mask_image);
          for (y=0; y < (ssize_t) mask_image->rows; y++)
          {
            q=GetCacheViewAuthenticPixels(mask_view,0,y,mask_image->columns,1,
              exception);
            if (q == (Quantum *) NULL)
              break;
            for (x=0; x < (ssize_t) mask_image->columns; x++)
            {
              if (mask_image->matte == MagickFalse)
                SetPixelAlpha(mask_image,GetPixelIntensity(mask_image,q),q);
              SetPixelRed(mask_image,GetPixelAlpha(mask_image,q),q);
              SetPixelGreen(mask_image,GetPixelAlpha(mask_image,q),q);
              SetPixelBlue(mask_image,GetPixelAlpha(mask_image,q),q);
              q+=GetPixelChannels(mask_image);
            }
            if (SyncCacheViewAuthenticPixels(mask_view,exception) == MagickFalse)
              break;
          }
          /* set the write mask */
          mask_view=DestroyCacheView(mask_view);
          mask_image->matte=MagickTrue;
          (void) SetImageClipMask(*image,mask_image,exception);
          mask_image=DestroyImage(mask_image);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("clip-path",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) ClipImagePath(*image,argv[1],*argv[0] == '-' ? MagickTrue :
            MagickFalse,exception);
          break;
        }
      if (LocaleCompare("colorize",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          new_image=ColorizeImage(*image,argv[1],draw_info->fill,
            exception);
          break;
        }
      if (LocaleCompare("color-matrix",argv[0]+1) == 0)
        {
          KernelInfo
            *kernel;

          (void) SyncImageSettings(image_info,*image);
          kernel=AcquireKernelInfo(argv[1]);
          if (kernel == (KernelInfo *) NULL)
            break;
          new_image=ColorMatrixImage(*image,kernel,exception);
          kernel=DestroyKernelInfo(kernel);
          break;
        }
      if (LocaleCompare("colors",argv[0]+1) == 0)
        {
          /* Reduce the number of colors in the image.  */
          (void) SyncImageSettings(image_info,*image);
          quantize_info->number_colors=StringToUnsignedLong(argv[1]);
          if (quantize_info->number_colors == 0)
            break;
          if (((*image)->storage_class == DirectClass) ||
              (*image)->colors > quantize_info->number_colors)
            (void) QuantizeImage(quantize_info,*image,exception);
          else
            (void) CompressImageColormap(*image,exception);
          break;
        }
      if (LocaleCompare("colorspace",argv[0]+1) == 0)
        {
          ColorspaceType
            colorspace;

          /* FUTURE: default colorspace should be sRGB! unless linear is set */
          /* This is a Image Setting, which has already been applied */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              (void) TransformImageColorspace(*image,RGBColorspace);
              InheritException(exception,&(*image)->exception);
              break;
            }
          /* colorspace=(ColorspaceType) ParseCommandOption(
              MagickColorspaceOptions,MagickFalse,argv[1]); */
          colorspace=image_info->colorspace;
          (void) TransformImageColorspace(*image,colorspace);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("contrast",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) ContrastImage(*image,(*argv[0] == '-') ? MagickTrue :
            MagickFalse,exception);
          break;
        }
      if (LocaleCompare("contrast-stretch",argv[0]+1) == 0)
        {
          double
            black_point,
            white_point;

          MagickStatusType
            flags;

          /*
            Contrast stretch image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          black_point=geometry_info.rho;
          white_point=(flags & SigmaValue) != 0 ? geometry_info.sigma :
            black_point;
          if ((flags & PercentValue) != 0)
            {
              black_point*=(double) (*image)->columns*(*image)->rows/100.0;
              white_point*=(double) (*image)->columns*(*image)->rows/100.0;
            }
          white_point=(MagickRealType) (*image)->columns*(*image)->rows-
            white_point;
          (void) ContrastStretchImage(*image,black_point,white_point,
            exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("convolve",argv[0]+1) == 0)
        {
          KernelInfo
            *kernel_info;

          (void) SyncImageSettings(image_info,*image);
          kernel_info=AcquireKernelInfo(argv[1]);
          if (kernel_info == (KernelInfo *) NULL)
            break;
          kernel_info->bias=(*image)->bias;
          new_image=ConvolveImage(*image,kernel_info,exception);
          kernel_info=DestroyKernelInfo(kernel_info);
          break;
        }
      if (LocaleCompare("crop",argv[0]+1) == 0)
        {
          /*
            Crop a image to a smaller size
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=CropImageToTiles(*image,argv[1],exception);
          break;
        }
      if (LocaleCompare("cycle",argv[0]+1) == 0)
        {
          /*
            Cycle an image colormap.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) CycleColormapImage(*image,(ssize_t) StringToLong(argv[1]),
            exception);
          break;
        }
      break;
    }
    case 'd':
    {
      if (LocaleCompare("decipher",argv[0]+1) == 0)
        {
          StringInfo
            *passkey;

          /*
            Decipher pixels.
          */
          (void) SyncImageSettings(image_info,*image);
          passkey=FileToStringInfo(argv[1],~0,exception);
          if (passkey != (StringInfo *) NULL)
            {
              (void) PasskeyDecipherImage(*image,passkey,exception);
              passkey=DestroyStringInfo(passkey);
            }
          break;
        }
      if (LocaleCompare("density",argv[0]+1) == 0)
        {
          /*
            Set image density.
          */
          (void) CloneString(&draw_info->density,argv[1]);
          break;
        }
      if (LocaleCompare("depth",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              (void) SetImageDepth(*image,MAGICKCORE_QUANTUM_DEPTH);
              break;
            }
          (void) SetImageDepth(*image,StringToUnsignedLong(argv[1]));
          break;
        }
      if (LocaleCompare("deskew",argv[0]+1) == 0)
        {
          double
            threshold;

          /*
            Straighten the image.
          */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            threshold=40.0*QuantumRange/100.0;
          else
            threshold=SiPrefixToDouble(argv[1],QuantumRange);
          new_image=DeskewImage(*image,threshold,exception);
          break;
        }
      if (LocaleCompare("despeckle",argv[0]+1) == 0)
        {
          /*
            Reduce the speckles within an image.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=DespeckleImage(*image,exception);
          break;
        }
      if (LocaleCompare("display",argv[0]+1) == 0)
        {
          (void) CloneString(&draw_info->server_name,argv[1]);
          break;
        }
      if (LocaleCompare("distort",argv[0]+1) == 0)
        {
          char
            *args,
            token[MaxTextExtent];

          const char
            *p;

          DistortImageMethod
            method;

          double
            *arguments;

          register ssize_t
            x;

          size_t
            number_arguments;

          /*
            Distort image.
          */
          (void) SyncImageSettings(image_info,*image);
          method=(DistortImageMethod) ParseCommandOption(MagickDistortOptions,
            MagickFalse,argv[1]);
          if ( method == ResizeDistortion )
            {
               /* Special Case - Argument is actually a resize geometry!
               ** Convert that to an appropriate distortion argument array.
               */
               double
                 resize_args[2];
               (void) ParseRegionGeometry(*image,argv[2],&geometry,
                    exception);
               resize_args[0]=(double)geometry.width;
               resize_args[1]=(double)geometry.height;
               new_image=DistortImage(*image,method,(size_t)2,
                    resize_args,MagickTrue,exception);
               break;
            }
          args=InterpretImageProperties(image_info,*image,argv[2],
            exception);
          if (args == (char *) NULL)
            break;
          p=(char *) args;
          for (x=0; *p != '\0'; x++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
          }
          number_arguments=(size_t) x;
          arguments=(double *) AcquireQuantumMemory(number_arguments,
            sizeof(*arguments));
          if (arguments == (double *) NULL)
            ThrowWandFatalException(ResourceLimitFatalError,
              "MemoryAllocationFailed",(*image)->filename);
          (void) ResetMagickMemory(arguments,0,number_arguments*
            sizeof(*arguments));
          p=(char *) args;
          for (x=0; (x < (ssize_t) number_arguments) && (*p != '\0'); x++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
            arguments[x]=InterpretLocaleValue(token,(char **) NULL);
          }
          args=DestroyString(args);
          new_image=DistortImage(*image,method,number_arguments,arguments,
            (*argv[0] == '+') ? MagickTrue : MagickFalse,exception);
          arguments=(double *) RelinquishMagickMemory(arguments);
          break;
        }
      if (LocaleCompare("dither",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              quantize_info->dither=MagickFalse;
              break;
            }
          quantize_info->dither=MagickTrue;
          quantize_info->dither_method=(DitherMethod) ParseCommandOption(
            MagickDitherOptions,MagickFalse,argv[1]);
          if (quantize_info->dither_method == NoDitherMethod)
            quantize_info->dither=MagickFalse;
          break;
        }
      if (LocaleCompare("draw",argv[0]+1) == 0)
        {
          /*
            Draw image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) CloneString(&draw_info->primitive,argv[1]);
          (void) DrawImage(*image,draw_info,exception);
          break;
        }
      break;
    }
    case 'e':
    {
      if (LocaleCompare("edge",argv[0]+1) == 0)
        {
          /*
            Enhance edges in the image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=EdgeImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("emboss",argv[0]+1) == 0)
        {
          /*
            Gaussian embossen image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=EmbossImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("encipher",argv[0]+1) == 0)
        {
          StringInfo
            *passkey;

          /*
            Encipher pixels.
          */
          (void) SyncImageSettings(image_info,*image);
          passkey=FileToStringInfo(argv[1],~0,exception);
          if (passkey != (StringInfo *) NULL)
            {
              (void) PasskeyEncipherImage(*image,passkey,exception);
              passkey=DestroyStringInfo(passkey);
            }
          break;
        }
      if (LocaleCompare("encoding",argv[0]+1) == 0)
        {
          (void) CloneString(&draw_info->encoding,argv[1]);
          break;
        }
      if (LocaleCompare("enhance",argv[0]+1) == 0)
        {
          /*
            Enhance image.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=EnhanceImage(*image,exception);
          break;
        }
      if (LocaleCompare("equalize",argv[0]+1) == 0)
        {
          /*
            Equalize image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) EqualizeImage(*image,exception);
          break;
        }
      if (LocaleCompare("evaluate",argv[0]+1) == 0)
        {
          double
            constant;

          MagickEvaluateOperator
            op;

          (void) SyncImageSettings(image_info,*image);
          op=(MagickEvaluateOperator) ParseCommandOption(
            MagickEvaluateOptions,MagickFalse,argv[1]);
          constant=SiPrefixToDouble(argv[2],QuantumRange);
          (void) EvaluateImage(*image,op,constant,exception);
          break;
        }
      if (LocaleCompare("extent",argv[0]+1) == 0)
        {
          /*
            Set the image extent.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGravityGeometry(*image,argv[1],&geometry,exception);
          if (geometry.width == 0)
            geometry.width=(*image)->columns;
          if (geometry.height == 0)
            geometry.height=(*image)->rows;
          new_image=ExtentImage(*image,&geometry,exception);
          break;
        }
      break;
    }
    case 'f':
    {
      if (LocaleCompare("family",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              if (draw_info->family != (char *) NULL)
                draw_info->family=DestroyString(draw_info->family);
              break;
            }
          (void) CloneString(&draw_info->family,argv[1]);
          break;
        }
      if (LocaleCompare("features",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) DeleteImageArtifact(*image,"identify:features");
              break;
            }
          (void) SetImageArtifact(*image,"identify:features",argv[1]);
          break;
        }
      if (LocaleCompare("fill",argv[0]+1) == 0)
        {
          ExceptionInfo
            *sans;

          GetPixelInfo(*image,&fill);
          if (*argv[0] == '+')
            {
              (void) QueryMagickColorCompliance("none",AllCompliance,&fill,
                 exception);
              (void) QueryColorCompliance("none",AllCompliance,&draw_info->fill,
                exception);
              if (draw_info->fill_pattern != (Image *) NULL)
                draw_info->fill_pattern=DestroyImage(draw_info->fill_pattern);
              break;
            }
          sans=AcquireExceptionInfo();
          (void) QueryMagickColorCompliance(argv[1],AllCompliance,&fill,sans);
          status=QueryColorCompliance(argv[1],AllCompliance,&draw_info->fill,sans);
          sans=DestroyExceptionInfo(sans);
          if (status == MagickFalse)
            draw_info->fill_pattern=GetImageCache(image_info,argv[1],
              exception);
          break;
        }
      if (LocaleCompare("flip",argv[0]+1) == 0)
        {
          /*
            Flip image scanlines.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=FlipImage(*image,exception);
          break;
        }
      if (LocaleCompare("flop",argv[0]+1) == 0)
        {
          /*
            Flop image scanlines.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=FlopImage(*image,exception);
          break;
        }
      if (LocaleCompare("floodfill",argv[0]+1) == 0)
        {
          PixelInfo
            target;

          /*
            Floodfill image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParsePageGeometry(*image,argv[1],&geometry,exception);
          (void) QueryMagickColorCompliance(argv[2],AllCompliance,&target,
                        exception);
          (void) FloodfillPaintImage(*image,draw_info,&target,geometry.x,
            geometry.y,*argv[0] == '-' ? MagickFalse : MagickTrue,exception);
          break;
        }
      if (LocaleCompare("font",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              if (draw_info->font != (char *) NULL)
                draw_info->font=DestroyString(draw_info->font);
              break;
            }
          (void) CloneString(&draw_info->font,argv[1]);
          break;
        }
      if (LocaleCompare("format",argv[0]+1) == 0)
        {
          format=argv[1];
          break;
        }
      if (LocaleCompare("frame",argv[0]+1) == 0)
        {
          FrameInfo
            frame_info;

          /*
            Surround image with an ornamental border.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
          frame_info.width=geometry.width;
          frame_info.height=geometry.height;
          if ((flags & HeightValue) == 0)
            frame_info.height=geometry.width;
          frame_info.outer_bevel=geometry.x;
          frame_info.inner_bevel=geometry.y;
          frame_info.x=(ssize_t) frame_info.width;
          frame_info.y=(ssize_t) frame_info.height;
          frame_info.width=(*image)->columns+2*frame_info.width;
          frame_info.height=(*image)->rows+2*frame_info.height;
          new_image=FrameImage(*image,&frame_info,compose,exception);
          break;
        }
      if (LocaleCompare("function",argv[0]+1) == 0)
        {
          char
            *arguments,
            token[MaxTextExtent];

          const char
            *p;

          double
            *parameters;

          MagickFunction
            function;

          register ssize_t
            x;

          size_t
            number_parameters;

          /*
            Function Modify Image Values
          */
          (void) SyncImageSettings(image_info,*image);
          function=(MagickFunction) ParseCommandOption(MagickFunctionOptions,
            MagickFalse,argv[1]);
          arguments=InterpretImageProperties(image_info,*image,argv[2],
            exception);
          if (arguments == (char *) NULL)
            break;
          p=(char *) arguments;
          for (x=0; *p != '\0'; x++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
          }
          number_parameters=(size_t) x;
          parameters=(double *) AcquireQuantumMemory(number_parameters,
            sizeof(*parameters));
          if (parameters == (double *) NULL)
            ThrowWandFatalException(ResourceLimitFatalError,
              "MemoryAllocationFailed",(*image)->filename);
          (void) ResetMagickMemory(parameters,0,number_parameters*
            sizeof(*parameters));
          p=(char *) arguments;
          for (x=0; (x < (ssize_t) number_parameters) && (*p != '\0'); x++)
          {
            GetMagickToken(p,&p,token);
            if (*token == ',')
              GetMagickToken(p,&p,token);
            parameters[x]=InterpretLocaleValue(token,(char **) NULL);
          }
          arguments=DestroyString(arguments);
          (void) FunctionImage(*image,function,number_parameters,parameters,
            exception);
          parameters=(double *) RelinquishMagickMemory(parameters);
          break;
        }
      break;
    }
    case 'g':
    {
      if (LocaleCompare("gamma",argv[0]+1) == 0)
        {
          /*
            Gamma image.
          */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            (*image)->gamma=InterpretLocaleValue(argv[1],(char **) NULL);
          else
            (void) GammaImage(*image,InterpretLocaleValue(argv[1],
              (char **) NULL),exception);
          break;
        }
      if ((LocaleCompare("gaussian-blur",argv[0]+1) == 0) ||
          (LocaleCompare("gaussian",argv[0]+1) == 0))
        {
          /*
            Gaussian blur image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.0;
          new_image=GaussianBlurImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("geometry",argv[0]+1) == 0)
        {
            /*
              Record Image offset, Resize last image.
            */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              if ((*image)->geometry != (char *) NULL)
                (*image)->geometry=DestroyString((*image)->geometry);
              break;
            }
          flags=ParseRegionGeometry(*image,argv[1],&geometry,exception);
          if (((flags & XValue) != 0) || ((flags & YValue) != 0))
            (void) CloneString(&(*image)->geometry,argv[1]);
          else
            new_image=ResizeImage(*image,geometry.width,geometry.height,
              (*image)->filter,(*image)->blur,exception);
          break;
        }
      if (LocaleCompare("gravity",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              draw_info->gravity=UndefinedGravity;
              break;
            }
          draw_info->gravity=(GravityType) ParseCommandOption(
            MagickGravityOptions,MagickFalse,argv[1]);
          break;
        }
      break;
    }
    case 'h':
    {
      if (LocaleCompare("highlight-color",argv[0]+1) == 0)
        {
          (void) SetImageArtifact(*image,argv[0]+1,argv[1]);
          break;
        }
      break;
    }
    case 'i':
    {
      if (LocaleCompare("identify",argv[0]+1) == 0)
        {
          char
            *text;

          (void) SyncImageSettings(image_info,*image);
          if (format == (char *) NULL)
            {
              (void) IdentifyImage(*image,stdout,image_info->verbose,
                exception);
              break;
            }
          text=InterpretImageProperties(image_info,*image,format,
            exception);
          if (text == (char *) NULL)
            break;
          (void) fputs(text,stdout);
          (void) fputc('\n',stdout);
          text=DestroyString(text);
          break;
        }
      if (LocaleCompare("implode",argv[0]+1) == 0)
        {
          /*
            Implode image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=ImplodeImage(*image,geometry_info.rho,
            interpolate_method,exception);
          break;
        }
      if (LocaleCompare("interline-spacing",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            (void) ParseGeometry("0",&geometry_info);
          else
            (void) ParseGeometry(argv[1],&geometry_info);
          draw_info->interline_spacing=geometry_info.rho;
          break;
        }
      if (LocaleCompare("interpolate",argv[0]+1) == 0)
        {
          interpolate_method=(PixelInterpolateMethod) ParseCommandOption(
            MagickInterpolateOptions,MagickFalse,argv[1]);
          break;
        }
      if (LocaleCompare("interword-spacing",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            (void) ParseGeometry("0",&geometry_info);
          else
            (void) ParseGeometry(argv[1],&geometry_info);
          draw_info->interword_spacing=geometry_info.rho;
          break;
        }
      break;
    }
    case 'k':
    {
      if (LocaleCompare("kerning",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            (void) ParseGeometry("0",&geometry_info);
          else
            (void) ParseGeometry(argv[1],&geometry_info);
          draw_info->kerning=geometry_info.rho;
          break;
        }
      break;
    }
    case 'l':
    {
      if (LocaleCompare("lat",argv[0]+1) == 0)
        {
          /*
            Local adaptive threshold image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=AdaptiveThresholdImage(*image,(size_t)
            geometry_info.rho,(size_t) geometry_info.sigma,(double)
            geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("level",argv[0]+1) == 0)
        {
          MagickRealType
            black_point,
            gamma,
            white_point;

          MagickStatusType
            flags;

          /*
            Parse levels.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          black_point=geometry_info.rho;
          white_point=(MagickRealType) QuantumRange;
          if ((flags & SigmaValue) != 0)
            white_point=geometry_info.sigma;
          gamma=1.0;
          if ((flags & XiValue) != 0)
            gamma=geometry_info.xi;
          if ((flags & PercentValue) != 0)
            {
              black_point*=(MagickRealType) (QuantumRange/100.0);
              white_point*=(MagickRealType) (QuantumRange/100.0);
            }
          if ((flags & SigmaValue) == 0)
            white_point=(MagickRealType) QuantumRange-black_point;
          if ((*argv[0] == '+') || ((flags & AspectValue) != 0))
            (void) LevelizeImage(*image,black_point,white_point,gamma,
              exception);
          else
            (void) LevelImage(*image,black_point,white_point,gamma,
              exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("level-colors",argv[0]+1) == 0)
        {
          char
            token[MaxTextExtent];

          const char
            *p;

          PixelInfo
            black_point,
            white_point;

          p=(const char *) argv[1];
          GetMagickToken(p,&p,token);  /* get black point color */
          if ((isalpha((int) *token) != 0) || ((*token == '#') != 0))
            (void) QueryMagickColorCompliance(token,AllCompliance,
                      &black_point,exception);
          else
            (void) QueryMagickColorCompliance("#000000",AllCompliance,
                      &black_point,exception);
          if (isalpha((int) token[0]) || (token[0] == '#'))
            GetMagickToken(p,&p,token);
          if (*token == '\0')
            white_point=black_point; /* set everything to that color */
          else
            {
              if ((isalpha((int) *token) == 0) && ((*token == '#') == 0))
                GetMagickToken(p,&p,token); /* Get white point color. */
              if ((isalpha((int) *token) != 0) || ((*token == '#') != 0))
                (void) QueryMagickColorCompliance(token,AllCompliance,
                           &white_point,exception);
              else
                (void) QueryMagickColorCompliance("#ffffff",AllCompliance,
                           &white_point,exception);
            }
          (void) LevelImageColors(*image,&black_point,&white_point,
            *argv[0] == '+' ? MagickTrue : MagickFalse,exception);
          break;
        }
      if (LocaleCompare("linear-stretch",argv[0]+1) == 0)
        {
          double
            black_point,
            white_point;

          MagickStatusType
            flags;

          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          black_point=geometry_info.rho;
          white_point=(MagickRealType) (*image)->columns*(*image)->rows;
          if ((flags & SigmaValue) != 0)
            white_point=geometry_info.sigma;
          if ((flags & PercentValue) != 0)
            {
              black_point*=(double) (*image)->columns*(*image)->rows/100.0;
              white_point*=(double) (*image)->columns*(*image)->rows/100.0;
            }
          if ((flags & SigmaValue) == 0)
            white_point=(MagickRealType) (*image)->columns*(*image)->rows-
              black_point;
          (void) LinearStretchImage(*image,black_point,white_point,exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("linewidth",argv[0]+1) == 0)
        {
          draw_info->stroke_width=InterpretLocaleValue(argv[1],
            (char **) NULL);
          break;
        }
      if (LocaleCompare("liquid-rescale",argv[0]+1) == 0)
        {
          /*
            Liquid rescale image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseRegionGeometry(*image,argv[1],&geometry,exception);
          if ((flags & XValue) == 0)
            geometry.x=1;
          if ((flags & YValue) == 0)
            geometry.y=0;
          new_image=LiquidRescaleImage(*image,geometry.width,
            geometry.height,1.0*geometry.x,1.0*geometry.y,exception);
          break;
        }
      if (LocaleCompare("lowlight-color",argv[0]+1) == 0)
        {
          (void) SetImageArtifact(*image,argv[0]+1,argv[1]);
          break;
        }
      break;
    }
    case 'm':
    {
      if (LocaleCompare("map",argv[0]+1) == 0)
        {
          Image
            *remap_image;

          /*
            Transform image colors to match this set of colors.
          */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            break;
          remap_image=GetImageCache(image_info,argv[1],exception);
          if (remap_image == (Image *) NULL)
            break;
          (void) RemapImage(quantize_info,*image,remap_image,exception);
          remap_image=DestroyImage(remap_image);
          break;
        }
      if (LocaleCompare("mask",argv[0]+1) == 0)
        {
          Image
            *mask;

          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              /*
                Remove a mask.
              */
              (void) SetImageMask(*image,(Image *) NULL,exception);
              break;
            }
          /*
            Set the image mask.
          */
          mask=GetImageCache(image_info,argv[1],exception);
          if (mask == (Image *) NULL)
            break;
          (void) SetImageMask(*image,mask,exception);
          mask=DestroyImage(mask);
          break;
        }
      if (LocaleCompare("matte",argv[0]+1) == 0)
        {
          (void) SetImageAlphaChannel(*image,(*argv[0] == '-') ?
            SetAlphaChannel : DeactivateAlphaChannel,exception);
          break;
        }
      if (LocaleCompare("median",argv[0]+1) == 0)
        {
          /*
            Median filter image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=StatisticImage(*image,MedianStatistic,(size_t)
            geometry_info.rho,(size_t) geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("mode",argv[0]+1) == 0)
        {
          /*
            Mode image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=StatisticImage(*image,ModeStatistic,(size_t)
            geometry_info.rho,(size_t) geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("modulate",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) ModulateImage(*image,argv[1],exception);
          break;
        }
      if (LocaleCompare("monitor",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageProgressMonitor(*image,
                (MagickProgressMonitor) NULL,(void *) NULL);
              break;
            }
          (void) SetImageProgressMonitor(*image,MonitorProgress,
            (void *) NULL);
          break;
        }
      if (LocaleCompare("monochrome",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) SetImageType(*image,BilevelType,exception);
          break;
        }
      if (LocaleCompare("morphology",argv[0]+1) == 0)
        {
          char
            token[MaxTextExtent];

          const char
            *p;

          KernelInfo
            *kernel;

          MorphologyMethod
            method;

          ssize_t
            iterations;

          /*
            Morphological Image Operation
          */
          (void) SyncImageSettings(image_info,*image);
          p=argv[1];
          GetMagickToken(p,&p,token);
          method=(MorphologyMethod) ParseCommandOption(
            MagickMorphologyOptions,MagickFalse,token);
          iterations=1L;
          GetMagickToken(p,&p,token);
          if ((*p == ':') || (*p == ','))
            GetMagickToken(p,&p,token);
          if ((*p != '\0'))
            iterations=(ssize_t) StringToLong(p);
          kernel=AcquireKernelInfo(argv[2]);
          if (kernel == (KernelInfo *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"UnabletoParseKernel","morphology");
              status=MagickFalse;
              break;
            }
          new_image=MorphologyImage(*image,method,iterations,kernel,
            exception);
          kernel=DestroyKernelInfo(kernel);
          break;
        }
      if (LocaleCompare("motion-blur",argv[0]+1) == 0)
        {
          /*
            Motion blur image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=MotionBlurImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,geometry_info.psi,
            exception);
          break;
        }
      break;
    }
    case 'n':
    {
      if (LocaleCompare("negate",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) NegateImage(*image,*argv[0] == '+' ? MagickTrue :
            MagickFalse,exception);
          break;
        }
      if (LocaleCompare("noise",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '-')
            {
              flags=ParseGeometry(argv[1],&geometry_info);
              if ((flags & SigmaValue) == 0)
                geometry_info.sigma=geometry_info.rho;
              new_image=StatisticImage(*image,NonpeakStatistic,(size_t)
                geometry_info.rho,(size_t) geometry_info.sigma,exception);
            }
          else
            {
              NoiseType
                noise;

              noise=(NoiseType) ParseCommandOption(MagickNoiseOptions,
                MagickFalse,argv[1]);
              new_image=AddNoiseImage(*image,noise,exception);
            }
          break;
        }
      if (LocaleCompare("normalize",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) NormalizeImage(*image,exception);
          break;
        }
      break;
    }
    case 'o':
    {
      if (LocaleCompare("opaque",argv[0]+1) == 0)
        {
          PixelInfo
            target;

          (void) SyncImageSettings(image_info,*image);
          (void) QueryMagickColorCompliance(argv[1],AllCompliance,&target,
                       exception);
          (void) OpaquePaintImage(*image,&target,&fill,*argv[0] == '-' ?
            MagickFalse : MagickTrue,exception);
          break;
        }
      if (LocaleCompare("ordered-dither",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) OrderedPosterizeImage(*image,argv[1],exception);
          break;
        }
      break;
    }
    case 'p':
    {
      if (LocaleCompare("paint",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=OilPaintImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("pen",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) QueryColorCompliance("none",AllCompliance,&draw_info->fill,
                 exception);
              break;
            }
          (void) QueryColorCompliance(argv[1],AllCompliance,&draw_info->fill,
                 exception);
          break;
        }
      if (LocaleCompare("pointsize",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            (void) ParseGeometry("12",&geometry_info);
          else
            (void) ParseGeometry(argv[1],&geometry_info);
          draw_info->pointsize=geometry_info.rho;
          break;
        }
      if (LocaleCompare("polaroid",argv[0]+1) == 0)
        {
          double
            angle;

          RandomInfo
            *random_info;

          /*
            Simulate a Polaroid picture.
          */
          (void) SyncImageSettings(image_info,*image);
          random_info=AcquireRandomInfo();
          angle=22.5*(GetPseudoRandomValue(random_info)-0.5);
          random_info=DestroyRandomInfo(random_info);
          if (*argv[0] == '-')
            {
              SetGeometryInfo(&geometry_info);
              flags=ParseGeometry(argv[1],&geometry_info);
              angle=geometry_info.rho;
            }
          new_image=PolaroidImage(*image,draw_info,angle,
            interpolate_method,exception);
          break;
        }
      if (LocaleCompare("posterize",argv[0]+1) == 0)
        {
          /*
            Posterize image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) PosterizeImage(*image,StringToUnsignedLong(argv[1]),
            quantize_info->dither,exception);
          break;
        }
      if (LocaleCompare("preview",argv[0]+1) == 0)
        {
          PreviewType
            preview_type;

          /*
            Preview image.
          */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            preview_type=UndefinedPreview;
          else
            preview_type=(PreviewType) ParseCommandOption(
              MagickPreviewOptions,MagickFalse,argv[1]);
          new_image=PreviewImage(*image,preview_type,exception);
          break;
        }
      if (LocaleCompare("profile",argv[0]+1) == 0)
        {
          const char
            *name;

          const StringInfo
            *profile;

          Image
            *profile_image;

          ImageInfo
            *profile_info;

          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            {
              /*
                Remove a profile from the image.
              */
              (void) ProfileImage(*image,argv[1],(const unsigned char *)
                NULL,0,MagickTrue);
              InheritException(exception,&(*image)->exception);
              break;
            }
          /*
            Associate a profile with the image.
          */
          profile_info=CloneImageInfo(image_info);
          profile=GetImageProfile(*image,"iptc");
          if (profile != (StringInfo *) NULL)
            profile_info->profile=(void *) CloneStringInfo(profile);
          profile_image=GetImageCache(profile_info,argv[1],exception);
          profile_info=DestroyImageInfo(profile_info);
          if (profile_image == (Image *) NULL)
            {
              StringInfo
                *profile;

              profile_info=CloneImageInfo(image_info);
              (void) CopyMagickString(profile_info->filename,argv[1],
                MaxTextExtent);
              profile=FileToStringInfo(profile_info->filename,~0UL,exception);
              if (profile != (StringInfo *) NULL)
                {
                  (void) ProfileImage(*image,profile_info->magick,
                    GetStringInfoDatum(profile),(size_t)
                    GetStringInfoLength(profile),MagickFalse);
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
              (void) ProfileImage(*image,name,GetStringInfoDatum(profile),
                (size_t) GetStringInfoLength(profile),MagickFalse);
            name=GetNextImageProfile(profile_image);
          }
          profile_image=DestroyImage(profile_image);
          break;
        }
      break;
    }
    case 'q':
    {
      if (LocaleCompare("quantize",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              quantize_info->colorspace=UndefinedColorspace;
              break;
            }
          quantize_info->colorspace=(ColorspaceType) ParseCommandOption(
            MagickColorspaceOptions,MagickFalse,argv[1]);
          break;
        }
      break;
    }
    case 'r':
    {
      if (LocaleCompare("radial-blur",argv[0]+1) == 0)
        {
          /*
            Radial blur image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          new_image=RadialBlurImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("raise",argv[0]+1) == 0)
        {
          /*
            Surround image with a raise of solid color.
          */
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
          if ((flags & SigmaValue) == 0)
            geometry.height=geometry.width;
          (void) RaiseImage(*image,&geometry,*argv[0] == '-' ? MagickTrue :
            MagickFalse,exception);
          break;
        }
      if (LocaleCompare("random-threshold",argv[0]+1) == 0)
        {
          /*
            Threshold image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) RandomThresholdImage(*image,argv[1],exception);
          break;
        }
      if (LocaleCompare("recolor",argv[0]+1) == 0)
        {
          KernelInfo
            *kernel;

          (void) SyncImageSettings(image_info,*image);
          kernel=AcquireKernelInfo(argv[1]);
          if (kernel == (KernelInfo *) NULL)
            break;
          new_image=ColorMatrixImage(*image,kernel,exception);
          kernel=DestroyKernelInfo(kernel);
          break;
        }
      if (LocaleCompare("render",argv[0]+1) == 0)
        {
          (void) SyncImageSettings(image_info,*image);
          draw_info->render=(*argv[0] == '+') ? MagickTrue : MagickFalse;
          break;
        }
      if (LocaleCompare("remap",argv[0]+1) == 0)
        {
          Image
            *remap_image;

          /*
            Transform image colors to match this set of colors.
          */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            break;
          remap_image=GetImageCache(image_info,argv[1],exception);
          if (remap_image == (Image *) NULL)
            break;
          (void) RemapImage(quantize_info,*image,remap_image,exception);
          remap_image=DestroyImage(remap_image);
          break;
        }
      if (LocaleCompare("repage",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) ParseAbsoluteGeometry("0x0+0+0",&(*image)->page);
              break;
            }
          (void) ResetImagePage(*image,argv[1]);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("resample",argv[0]+1) == 0)
        {
          /*
            Resample image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=ResampleImage(*image,geometry_info.rho,
            geometry_info.sigma,(*image)->filter,(*image)->blur,exception);
          break;
        }
      if (LocaleCompare("resize",argv[0]+1) == 0)
        {
          /*
            Resize image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=ResizeImage(*image,geometry.width,geometry.height,
            (*image)->filter,(*image)->blur,exception);
          break;
        }
      if (LocaleCompare("roll",argv[0]+1) == 0)
        {
          /*
            Roll image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParsePageGeometry(*image,argv[1],&geometry,exception);
          new_image=RollImage(*image,geometry.x,geometry.y,exception);
          break;
        }
      if (LocaleCompare("rotate",argv[0]+1) == 0)
        {
          char
            *geometry;

          /*
            Check for conditional image rotation.
          */
          (void) SyncImageSettings(image_info,*image);
          if (strchr(argv[1],'>') != (char *) NULL)
            if ((*image)->columns <= (*image)->rows)
              break;
          if (strchr(argv[1],'<') != (char *) NULL)
            if ((*image)->columns >= (*image)->rows)
              break;
          /*
            Rotate image.
          */
          geometry=ConstantString(argv[1]);
          (void) SubstituteString(&geometry,">","");
          (void) SubstituteString(&geometry,"<","");
          (void) ParseGeometry(geometry,&geometry_info);
          geometry=DestroyString(geometry);
          new_image=RotateImage(*image,geometry_info.rho,exception);
          break;
        }
      break;
    }
    case 's':
    {
      if (LocaleCompare("sample",argv[0]+1) == 0)
        {
          /*
            Sample image with pixel replication.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=SampleImage(*image,geometry.width,geometry.height,
            exception);
          break;
        }
      if (LocaleCompare("scale",argv[0]+1) == 0)
        {
          /*
            Resize image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=ScaleImage(*image,geometry.width,geometry.height,
            exception);
          break;
        }
      if (LocaleCompare("selective-blur",argv[0]+1) == 0)
        {
          /*
            Selectively blur pixels within a contrast threshold.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=SelectiveBlurImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,geometry_info.psi,exception);
          break;
        }
      if (LocaleCompare("separate",argv[0]+1) == 0)
        {
          /*
            Break channels into separate images.
            WARNING: This can generate multiple images!
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=SeparateImages(*image,exception);
          break;
        }
      if (LocaleCompare("sepia-tone",argv[0]+1) == 0)
        {
          double
            threshold;

          /*
            Sepia-tone image.
          */
          (void) SyncImageSettings(image_info,*image);
          threshold=SiPrefixToDouble(argv[1],QuantumRange);
          new_image=SepiaToneImage(*image,threshold,exception);
          break;
        }
      if (LocaleCompare("segment",argv[0]+1) == 0)
        {
          /*
            Segment image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          (void) SegmentImage(*image,(*image)->colorspace,
            image_info->verbose,geometry_info.rho,geometry_info.sigma,
            exception);
          break;
        }
      if (LocaleCompare("set",argv[0]+1) == 0)
        {
          char
            *value;

          if (*argv[0] == '+')
            {
              if (LocaleNCompare(argv[1],"registry:",9) == 0)
                (void) DeleteImageRegistry(argv[1]+9);
              else
                if (LocaleNCompare(argv[1],"argv[0]:",7) == 0)
                  {
                    (void) DeleteImageOption(image_info,argv[1]+7);
                    (void) DeleteImageArtifact(*image,argv[1]+7);
                  }
                else
                  (void) DeleteImageProperty(*image,argv[1]);
              break;
            }
          value=InterpretImageProperties(image_info,*image,argv[2],
            exception);
          if (value == (char *) NULL)
            break;
          if (LocaleNCompare(argv[1],"registry:",9) == 0)
            (void) SetImageRegistry(StringRegistryType,argv[1]+9,value,
              exception);
          else
            if (LocaleNCompare(argv[1],"option:",7) == 0)
              {
                (void) SetImageOption(image_info,argv[1]+7,value);
                (void) SetImageArtifact(*image,argv[1]+7,value);
              }
            else
              (void) SetImageProperty(*image,argv[1],value);
          value=DestroyString(value);
          break;
        }
      if (LocaleCompare("shade",argv[0]+1) == 0)
        {
          /*
            Shade image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=ShadeImage(*image,(*argv[0] == '-') ? MagickTrue :
            MagickFalse,geometry_info.rho,geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("shadow",argv[0]+1) == 0)
        {
          /*
            Shadow image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=4.0;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=4.0;
          new_image=ShadowImage(*image,geometry_info.rho,
            geometry_info.sigma,(ssize_t) ceil(geometry_info.xi-0.5),(ssize_t)
            ceil(geometry_info.psi-0.5),exception);
          break;
        }
      if (LocaleCompare("sharpen",argv[0]+1) == 0)
        {
          /*
            Sharpen image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.0;
          new_image=SharpenImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("shave",argv[0]+1) == 0)
        {
          /*
            Shave the image edges.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
          new_image=ShaveImage(*image,&geometry,exception);
          break;
        }
      if (LocaleCompare("shear",argv[0]+1) == 0)
        {
          /*
            Shear image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=ShearImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("sigmoidal-contrast",argv[0]+1) == 0)
        {
          /*
            Sigmoidal non-linearity contrast control.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=(double) QuantumRange/2.0;
          if ((flags & PercentValue) != 0)
            geometry_info.sigma=(double) QuantumRange*geometry_info.sigma/
              100.0;
          (void) SigmoidalContrastImage(*image,(*argv[0] == '-') ?
            MagickTrue : MagickFalse,geometry_info.rho,geometry_info.sigma,
            exception);
          break;
        }
      if (LocaleCompare("sketch",argv[0]+1) == 0)
        {
          /*
            Sketch image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=SketchImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,geometry_info.psi,exception);
          break;
        }
      if (LocaleCompare("solarize",argv[0]+1) == 0)
        {
          double
            threshold;

          (void) SyncImageSettings(image_info,*image);
          threshold=SiPrefixToDouble(argv[1],QuantumRange);
          (void) SolarizeImage(*image,threshold,exception);
          break;
        }
      if (LocaleCompare("sparse-color",argv[0]+1) == 0)
        {
          SparseColorMethod
            method;

          char
            *arguments;

          /*
            Sparse Color Interpolated Gradient
          */
          (void) SyncImageSettings(image_info,*image);
          method=(SparseColorMethod) ParseCommandOption(
            MagickSparseColorOptions,MagickFalse,argv[1]);
          arguments=InterpretImageProperties(image_info,*image,argv[2],
            exception);
          if (arguments == (char *) NULL)
            break;
          new_image=SparseColorOption(*image,method,arguments,
            argv[0][0] == '+' ? MagickTrue : MagickFalse,exception);
          arguments=DestroyString(arguments);
          break;
        }
      if (LocaleCompare("splice",argv[0]+1) == 0)
        {
          /*
            Splice a solid color into the image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseGravityGeometry(*image,argv[1],&geometry,exception);
          new_image=SpliceImage(*image,&geometry,exception);
          break;
        }
      if (LocaleCompare("spread",argv[0]+1) == 0)
        {
          /*
            Spread an image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=SpreadImage(*image,geometry_info.rho,
            interpolate_method,exception);
          break;
        }
      if (LocaleCompare("statistic",argv[0]+1) == 0)
        {
          StatisticType
            type;

          (void) SyncImageSettings(image_info,*image);
          type=(StatisticType) ParseCommandOption(MagickStatisticOptions,
            MagickFalse,argv[1]);
          (void) ParseGeometry(argv[2],&geometry_info);
          new_image=StatisticImage(*image,type,(size_t) geometry_info.rho,
            (size_t) geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("stretch",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              draw_info->stretch=UndefinedStretch;
              break;
            }
          draw_info->stretch=(StretchType) ParseCommandOption(
            MagickStretchOptions,MagickFalse,argv[1]);
          break;
        }
      if (LocaleCompare("strip",argv[0]+1) == 0)
        {
          /*
            Strip image of profiles and comments.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) StripImage(*image);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("stroke",argv[0]+1) == 0)
        {
          ExceptionInfo
            *sans;

          if (*argv[0] == '+')
            {
              (void) QueryColorCompliance("none",AllCompliance,&draw_info->stroke,
                exception);
              if (draw_info->stroke_pattern != (Image *) NULL)
                draw_info->stroke_pattern=DestroyImage(
                  draw_info->stroke_pattern);
              break;
            }
          sans=AcquireExceptionInfo();
          status=QueryColorCompliance(argv[1],AllCompliance,&draw_info->stroke,sans);
          sans=DestroyExceptionInfo(sans);
          if (status == MagickFalse)
            draw_info->stroke_pattern=GetImageCache(image_info,argv[1],
              exception);
          break;
        }
      if (LocaleCompare("strokewidth",argv[0]+1) == 0)
        {
          draw_info->stroke_width=InterpretLocaleValue(argv[1],
            (char **) NULL);
          break;
        }
      if (LocaleCompare("style",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              draw_info->style=UndefinedStyle;
              break;
            }
          draw_info->style=(StyleType) ParseCommandOption(MagickStyleOptions,
            MagickFalse,argv[1]);
          break;
        }
      if (LocaleCompare("swirl",argv[0]+1) == 0)
        {
          /*
            Swirl image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=SwirlImage(*image,geometry_info.rho,
            interpolate_method,exception);
          break;
        }
      break;
    }
    case 't':
    {
      if (LocaleCompare("threshold",argv[0]+1) == 0)
        {
          double
            threshold;

          /*
            Threshold image.
          */
          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            threshold=(double) QuantumRange/2;
          else
            threshold=SiPrefixToDouble(argv[1],QuantumRange);
          (void) BilevelImage(*image,threshold);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("thumbnail",argv[0]+1) == 0)
        {
          /*
            Thumbnail image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=ThumbnailImage(*image,geometry.width,geometry.height,
            exception);
          break;
        }
      if (LocaleCompare("tile",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              if (draw_info->fill_pattern != (Image *) NULL)
                draw_info->fill_pattern=DestroyImage(draw_info->fill_pattern);
              break;
            }
          draw_info->fill_pattern=GetImageCache(image_info,argv[1],
            exception);
          break;
        }
      if (LocaleCompare("tint",argv[0]+1) == 0)
        {
          /*
            Tint the image.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=TintImage(*image,argv[1],&fill,exception);
          break;
        }
      if (LocaleCompare("transform",argv[0]+1) == 0)
        {
          /*
            Affine transform image.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=AffineTransformImage(*image,&draw_info->affine,
            exception);
          break;
        }
      if (LocaleCompare("transparent",argv[0]+1) == 0)
        {
          PixelInfo
            target;

          (void) SyncImageSettings(image_info,*image);
          (void) QueryMagickColorCompliance(argv[1],AllCompliance,&target,
                       exception);
          (void) TransparentPaintImage(*image,&target,(Quantum)
            TransparentAlpha,*argv[0] == '-' ? MagickFalse : MagickTrue,
            &(*image)->exception);
          break;
        }
      if (LocaleCompare("transpose",argv[0]+1) == 0)
        {
          /*
            Transpose image scanlines.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=TransposeImage(*image,exception);
          break;
        }
      if (LocaleCompare("transverse",argv[0]+1) == 0)
        {
          /*
            Transverse image scanlines.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=TransverseImage(*image,exception);
          break;
        }
      if (LocaleCompare("treedepth",argv[0]+1) == 0)
        {
          quantize_info->tree_depth=StringToUnsignedLong(argv[1]);
          break;
        }
      if (LocaleCompare("trim",argv[0]+1) == 0)
        {
          /*
            Trim image.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=TrimImage(*image,exception);
          break;
        }
      if (LocaleCompare("type",argv[0]+1) == 0)
        {
          ImageType
            type;

          (void) SyncImageSettings(image_info,*image);
          if (*argv[0] == '+')
            type=UndefinedType;
          else
            type=(ImageType) ParseCommandOption(MagickTypeOptions,MagickFalse,
              argv[1]);
          (*image)->type=UndefinedType;
          (void) SetImageType(*image,type,exception);
          break;
        }
      break;
    }
    case 'u':
    {
      if (LocaleCompare("undercolor",argv[0]+1) == 0)
        {
          (void) QueryColorCompliance(argv[1],AllCompliance,&draw_info->undercolor,
            exception);
          break;
        }
      if (LocaleCompare("unique",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) DeleteImageArtifact(*image,"identify:unique-colors");
              break;
            }
          (void) SetImageArtifact(*image,"identify:unique-colors","true");
          (void) SetImageArtifact(*image,"verbose","true");
          break;
        }
      if (LocaleCompare("unique-colors",argv[0]+1) == 0)
        {
          /*
            Unique image colors.
          */
          (void) SyncImageSettings(image_info,*image);
          new_image=UniqueImageColors(*image,exception);
          break;
        }
      if (LocaleCompare("unsharp",argv[0]+1) == 0)
        {
          /*
            Unsharp mask image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=1.0;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=0.05;
          new_image=UnsharpMaskImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,geometry_info.psi,exception);
          break;
        }
      break;
    }
    case 'v':
    {
      if (LocaleCompare("verbose",argv[0]+1) == 0)
        {
          (void) SetImageArtifact(*image,argv[0]+1,
            *argv[0] == '+' ? "false" : "true");
          break;
        }
      if (LocaleCompare("vignette",argv[0]+1) == 0)
        {
          /*
            Vignette image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=0.1*(*image)->columns;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=0.1*(*image)->rows;
          new_image=VignetteImage(*image,geometry_info.rho,
            geometry_info.sigma,(ssize_t) ceil(geometry_info.xi-0.5),(ssize_t)
            ceil(geometry_info.psi-0.5),exception);
          break;
        }
      if (LocaleCompare("virtual-pixel",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              (void) SetImageVirtualPixelMethod(*image,
                UndefinedVirtualPixelMethod);
              break;
            }
          (void) SetImageVirtualPixelMethod(*image,(VirtualPixelMethod)
            ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
            argv[1]));
          break;
        }
      break;
    }
    case 'w':
    {
      if (LocaleCompare("wave",argv[0]+1) == 0)
        {
          /*
            Wave image.
          */
          (void) SyncImageSettings(image_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=WaveImage(*image,geometry_info.rho,
            geometry_info.sigma,interpolate_method,exception);
          break;
        }
      if (LocaleCompare("weight",argv[0]+1) == 0)
        {
          draw_info->weight=StringToUnsignedLong(argv[1]);
          if (LocaleCompare(argv[1],"all") == 0)
            draw_info->weight=0;
          if (LocaleCompare(argv[1],"bold") == 0)
            draw_info->weight=700;
          if (LocaleCompare(argv[1],"bolder") == 0)
            if (draw_info->weight <= 800)
              draw_info->weight+=100;
          if (LocaleCompare(argv[1],"lighter") == 0)
            if (draw_info->weight >= 100)
              draw_info->weight-=100;
          if (LocaleCompare(argv[1],"normal") == 0)
            draw_info->weight=400;
          break;
        }
      if (LocaleCompare("white-threshold",argv[0]+1) == 0)
        {
          /*
            White threshold image.
          */
          (void) SyncImageSettings(image_info,*image);
          (void) WhiteThresholdImage(*image,argv[1],exception);
          InheritException(exception,&(*image)->exception);
          break;
        }
      break;
    }
    default:
      break;
  }
  /*
     Replace current image with any image that was generated
  */
  if (new_image != (Image *) NULL)
    ReplaceImageInListReturnLast(image,new_image);

  /*
    Free resources.
  */
  quantize_info=DestroyQuantizeInfo(quantize_info);
  draw_info=DestroyDrawInfo(draw_info);
  status=(MagickStatusType) ((*image)->exception.severity ==
    UndefinedException ? 1 : 0);
  return(status == 0 ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     S e q u e n c e O p e r a t i o n I m a g e s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SequenceOperationImages() applies a single operation that apply to the
%  entire image list (e.g. -append, -layers, -coalesce, etc.).
%
%  The format of the MogrifyImage method is:
%
%    MagickBooleanType SequenceOperationImages(ImageInfo *image_info,
%        const int argc, const char **argv,Image **images,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o images: pointer to pointer of the first image in image list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType SequenceOperationImages(ImageInfo *image_info,
  const int argc,const char **argv,Image **images,ExceptionInfo *exception)
{

  MagickStatusType
    status;

  QuantizeInfo
    *quantize_info;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(images != (Image **) NULL);
  assert((*images)->previous == (Image *) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  if ((argc <= 0) || (*argv == (char *) NULL))
    return(MagickTrue);
  status=MagickTrue;

  switch (*(argv[0]+1))
  {
    case 'a':
    {
      if (LocaleCompare("affinity",argv[0]+1) == 0)
        {
          (void) SyncImagesSettings(image_info,*images);
          if (*argv[0] == '+')
            {
              (void) RemapImages(quantize_info,*images,(Image *) NULL,
                exception);
              break;
            }
          break;
        }
      if (LocaleCompare("append",argv[0]+1) == 0)
        {
          Image
            *append_image;

          (void) SyncImagesSettings(image_info,*images);
          append_image=AppendImages(*images,*argv[0] == '-' ? MagickTrue :
            MagickFalse,exception);
          if (append_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=append_image;
          break;
        }
      if (LocaleCompare("average",argv[0]+1) == 0)
        {
          Image
            *average_image;

          /*
            Average an image sequence (deprecated).
          */
          (void) SyncImagesSettings(image_info,*images);
          average_image=EvaluateImages(*images,MeanEvaluateOperator,
            exception);
          if (average_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=average_image;
          break;
        }
      break;
    }
    case 'c':
    {
      if (LocaleCompare("channel",argv[0]+1) == 0)
        {
          ChannelType
            channel;

          if (*argv[0] == '+')
            {
              channel=DefaultChannels;
              break;
            }
          channel=(ChannelType) ParseChannelOption(argv[1]);
          SetPixelChannelMap(*images,channel);
          break;
        }
      if (LocaleCompare("clut",argv[0]+1) == 0)
        {
          Image
            *clut_image,
            *image;

          (void) SyncImagesSettings(image_info,*images);
          image=RemoveFirstImageFromList(images);
          clut_image=RemoveFirstImageFromList(images);
          if (clut_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          (void) ClutImage(image,clut_image,interpolate_method,exception);
          clut_image=DestroyImage(clut_image);
          *images=DestroyImageList(*images);
          *images=image;
          break;
        }
      if (LocaleCompare("coalesce",argv[0]+1) == 0)
        {
          Image
            *coalesce_image;

          (void) SyncImagesSettings(image_info,*images);
          coalesce_image=CoalesceImages(*images,exception);
          if (coalesce_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=coalesce_image;
          break;
        }
      if (LocaleCompare("combine",argv[0]+1) == 0)
        {
          Image
            *combine_image;

          (void) SyncImagesSettings(image_info,*images);
          combine_image=CombineImages(*images,exception);
          if (combine_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=combine_image;
          break;
        }
      if (LocaleCompare("composite",argv[0]+1) == 0)
        {
          Image
            *mask_image,
            *composite_image,
            *image;

          RectangleInfo
            geometry;

          (void) SyncImagesSettings(image_info,*images);
          image=RemoveFirstImageFromList(images);
          composite_image=RemoveFirstImageFromList(images);
          if (composite_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          (void) TransformImage(&composite_image,(char *) NULL,
            composite_image->geometry);
          SetGeometry(composite_image,&geometry);
          (void) ParseAbsoluteGeometry(composite_image->geometry,&geometry);
          GravityAdjustGeometry(image->columns,image->rows,image->gravity,
            &geometry);
          mask_image=RemoveFirstImageFromList(images);
          if (mask_image != (Image *) NULL)
            {
              if ((image->compose == DisplaceCompositeOp) ||
                  (image->compose == DistortCompositeOp))
                {
                  /*
                    Merge Y displacement into X displacement image.
                  */
                  (void) CompositeImage(composite_image,CopyGreenCompositeOp,
                    mask_image,0,0);
                  mask_image=DestroyImage(mask_image);
                }
              else
                {
                  /*
                    Set a blending mask for the composition.
                    Posible error, what if image->mask already set.
                  */
                  image->mask=mask_image;
                  (void) NegateImage(image->mask,MagickFalse,exception);
                }
            }
          (void) CompositeImage(image,image->compose,composite_image,
            geometry.x,geometry.y);
          if (mask_image != (Image *) NULL)
            mask_image=image->mask=DestroyImage(image->mask);
          composite_image=DestroyImage(composite_image);
          InheritException(exception,&image->exception);
          *images=DestroyImageList(*images);
          *images=image;
          break;
        }
      break;
    }
    case 'd':
    {
      if (LocaleCompare("deconstruct",argv[0]+1) == 0)
        {
          Image
            *deconstruct_image;

          (void) SyncImagesSettings(image_info,*images);
          deconstruct_image=CompareImagesLayers(*images,CompareAnyLayer,
            exception);
          if (deconstruct_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=deconstruct_image;
          break;
        }
      if (LocaleCompare("delete",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            DeleteImages(images,"-1",exception);
          else
            DeleteImages(images,argv[1],exception);
          break;
        }
      if (LocaleCompare("dither",argv[0]+1) == 0)
        {
          if (*argv[0] == '+')
            {
              quantize_info->dither=MagickFalse;
              break;
            }
          quantize_info->dither=MagickTrue;
          quantize_info->dither_method=(DitherMethod) ParseCommandOption(
            MagickDitherOptions,MagickFalse,argv[1]);
          break;
        }
      if (LocaleCompare("duplicate",argv[0]+1) == 0)
        {
          Image
            *duplicate_images;

          if (*argv[0] == '+')
            duplicate_images=DuplicateImages(*images,1,"-1",exception);
          else
            {
              const char
                *p;

              size_t
                number_duplicates;

              number_duplicates=(size_t) StringToLong(argv[1]);
              p=strchr(argv[1],',');
              if (p == (const char *) NULL)
                duplicate_images=DuplicateImages(*images,number_duplicates,
                  "-1",exception);
              else
                duplicate_images=DuplicateImages(*images,number_duplicates,p,
                  exception);
            }
          AppendImageToList(images, duplicate_images);
          (void) SyncImagesSettings(image_info,*images);
          break;
        }
      break;
    }
    case 'e':
    {
      if (LocaleCompare("evaluate-sequence",argv[0]+1) == 0)
        {
          Image
            *evaluate_image;

          MagickEvaluateOperator
            op;

          (void) SyncImageSettings(image_info,*images);
          op=(MagickEvaluateOperator) ParseCommandOption(
            MagickEvaluateOptions,MagickFalse,argv[1]);
          evaluate_image=EvaluateImages(*images,op,exception);
          if (evaluate_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=evaluate_image;
          break;
        }
      break;
    }
    case 'f':
    {
      if (LocaleCompare("fft",argv[0]+1) == 0)
        {
          Image
            *fourier_image;

          /*
            Implements the discrete Fourier transform (DFT).
          */
          (void) SyncImageSettings(image_info,*images);
          fourier_image=ForwardFourierTransformImage(*images,*argv[0] == '-' ?
            MagickTrue : MagickFalse,exception);
          if (fourier_image == (Image *) NULL)
            break;
          *images=DestroyImage(*images);
          *images=fourier_image;
          break;
        }
      if (LocaleCompare("flatten",argv[0]+1) == 0)
        {
          Image
            *flatten_image;

          (void) SyncImagesSettings(image_info,*images);
          flatten_image=MergeImageLayers(*images,FlattenLayer,exception);
          if (flatten_image == (Image *) NULL)
            break;
          *images=DestroyImageList(*images);
          *images=flatten_image;
          break;
        }
      if (LocaleCompare("fx",argv[0]+1) == 0)
        {
          Image
            *fx_image;

          (void) SyncImagesSettings(image_info,*images);
          fx_image=FxImage(*images,argv[1],exception);
          if (fx_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=fx_image;
          break;
        }
      break;
    }
    case 'h':
    {
      if (LocaleCompare("hald-clut",argv[0]+1) == 0)
        {
          Image
            *hald_image,
            *image;

          (void) SyncImagesSettings(image_info,*images);
          image=RemoveFirstImageFromList(images);
          hald_image=RemoveFirstImageFromList(images);
          if (hald_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          (void) HaldClutImage(image,hald_image,exception);
          hald_image=DestroyImage(hald_image);
          if (*images != (Image *) NULL)
            *images=DestroyImageList(*images);
          *images=image;
          break;
        }
      break;
    }
    case 'i':
    {
      if (LocaleCompare("ift",argv[0]+1) == 0)
        {
          Image
            *fourier_image,
            *magnitude_image,
            *phase_image;

          /*
            Implements the inverse fourier discrete Fourier transform (DFT).
          */
          (void) SyncImagesSettings(image_info,*images);
          magnitude_image=RemoveFirstImageFromList(images);
          phase_image=RemoveFirstImageFromList(images);
          if (phase_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          fourier_image=InverseFourierTransformImage(magnitude_image,
            phase_image,*argv[0] == '-' ? MagickTrue : MagickFalse,exception);
          if (fourier_image == (Image *) NULL)
            break;
          if (*images != (Image *) NULL)
            *images=DestroyImage(*images);
          *images=fourier_image;
          break;
        }
      if (LocaleCompare("insert",argv[0]+1) == 0)
        {
          Image
            *p,
            *q;

          index=0;
          if (*argv[0] != '+')
            index=(ssize_t) StringToLong(argv[1]);
          p=RemoveLastImageFromList(images);
          if (p == (Image *) NULL)
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"NoSuchImage","`%s'",argv[1]);
              status=MagickFalse;
              break;
            }
          q=p;
          if (index == 0)
            PrependImageToList(images,q);
          else
            if (index == (ssize_t) GetImageListLength(*images))
              AppendImageToList(images,q);
            else
              {
                 q=GetImageFromList(*images,index-1);
                 if (q == (Image *) NULL)
                   {
                     (void) ThrowMagickException(exception,GetMagickModule(),
                       OptionError,"NoSuchImage","`%s'",argv[1]);
                     status=MagickFalse;
                     break;
                   }
                InsertImageInList(&q,p);
              }
          *images=GetFirstImageInList(q);
          break;
        }
      if (LocaleCompare("interpolate",argv[0]+1) == 0)
        {
          interpolate_method=(PixelInterpolateMethod) ParseCommandOption(
            MagickInterpolateOptions,MagickFalse,argv[1]);
          break;
        }
      break;
    }
    case 'l':
    {
      if (LocaleCompare("layers",argv[0]+1) == 0)
        {
          Image
            *layers;

          ImageLayerMethod
            method;

          (void) SyncImagesSettings(image_info,*images);
          layers=(Image *) NULL;
          method=(ImageLayerMethod) ParseCommandOption(MagickLayerOptions,
            MagickFalse,argv[1]);
          switch (method)
          {
            case CoalesceLayer:
            {
              layers=CoalesceImages(*images,exception);
              break;
            }
            case CompareAnyLayer:
            case CompareClearLayer:
            case CompareOverlayLayer:
            default:
            {
              layers=CompareImagesLayers(*images,method,exception);
              break;
            }
            case MergeLayer:
            case FlattenLayer:
            case MosaicLayer:
            case TrimBoundsLayer:
            {
              layers=MergeImageLayers(*images,method,exception);
              break;
            }
            case DisposeLayer:
            {
              layers=DisposeImages(*images,exception);
              break;
            }
            case OptimizeImageLayer:
            {
              layers=OptimizeImageLayers(*images,exception);
              break;
            }
            case OptimizePlusLayer:
            {
              layers=OptimizePlusImageLayers(*images,exception);
              break;
            }
            case OptimizeTransLayer:
            {
              OptimizeImageTransparency(*images,exception);
              break;
            }
            case RemoveDupsLayer:
            {
              RemoveDuplicateLayers(images,exception);
              break;
            }
            case RemoveZeroLayer:
            {
              RemoveZeroDelayLayers(images,exception);
              break;
            }
            case OptimizeLayer:
            {
              /*
                General Purpose, GIF Animation Optimizer.
              */
              layers=CoalesceImages(*images,exception);
              if (layers == (Image *) NULL)
                {
                  status=MagickFalse;
                  break;
                }
              *images=DestroyImageList(*images);
              *images=layers;
              layers=OptimizeImageLayers(*images,exception);
              if (layers == (Image *) NULL)
                {
                  status=MagickFalse;
                  break;
                }
              *images=DestroyImageList(*images);
              *images=layers;
              layers=(Image *) NULL;
              OptimizeImageTransparency(*images,exception);
              (void) RemapImages(quantize_info,*images,(Image *) NULL,
                exception);
              break;
            }
            case CompositeLayer:
            {
              CompositeOperator
                compose;

              Image
                *source;

              RectangleInfo
                geometry;

              /*
                Split image sequence at the first 'NULL:' image.
              */
              source=(*images);
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
                    {
                      /*
                        Separate the two lists, junk the null: image.
                      */
                      source=SplitImageList(source->previous);
                      DeleteImageFromList(&source);
                    }
                }
              if (source == (Image *) NULL)
                {
                  (void) ThrowMagickException(exception,GetMagickModule(),
                    OptionError,"MissingNullSeparator","layers Composite");
                  status=MagickFalse;
                  break;
                }
              /*
                Adjust offset with gravity and virtual canvas.
              */
              SetGeometry(*images,&geometry);
              (void) ParseAbsoluteGeometry((*images)->geometry,&geometry);
              geometry.width=source->page.width != 0 ?
                source->page.width : source->columns;
              geometry.height=source->page.height != 0 ?
               source->page.height : source->rows;
              GravityAdjustGeometry((*images)->page.width != 0 ?
                (*images)->page.width : (*images)->columns,
                (*images)->page.height != 0 ? (*images)->page.height :
                (*images)->rows,(*images)->gravity,&geometry);
              compose=OverCompositeOp;
              argv[0]=GetImageOption(image_info,"compose");
              if (argv[0] != (const char *) NULL)
                compose=(CompositeOperator) ParseCommandOption(
                  MagickComposeOptions,MagickFalse,argv[0]);
              CompositeLayers(*images,compose,source,geometry.x,geometry.y,
                exception);
              source=DestroyImageList(source);
              break;
            }
          }
          if (layers == (Image *) NULL)
            break;
          InheritException(exception,&layers->exception);
          *images=DestroyImageList(*images);
          *images=layers;
          break;
        }
      break;
    }
    case 'm':
    {
      if (LocaleCompare("map",argv[0]+1) == 0)
        {
          (void) SyncImagesSettings(image_info,*images);
          if (*argv[0] == '+')
            {
              (void) RemapImages(quantize_info,*images,(Image *) NULL,
                exception);
              break;
            }
          break;
        }
      if (LocaleCompare("maximum",argv[0]+1) == 0)
        {
          Image
            *maximum_image;

          /*
            Maximum image sequence (deprecated).
          */
          (void) SyncImagesSettings(image_info,*images);
          maximum_image=EvaluateImages(*images,MaxEvaluateOperator,exception);
          if (maximum_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=maximum_image;
          break;
        }
      if (LocaleCompare("minimum",argv[0]+1) == 0)
        {
          Image
            *minimum_image;

          /*
            Minimum image sequence (deprecated).
          */
          (void) SyncImagesSettings(image_info,*images);
          minimum_image=EvaluateImages(*images,MinEvaluateOperator,exception);
          if (minimum_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=minimum_image;
          break;
        }
      if (LocaleCompare("morph",argv[0]+1) == 0)
        {
          Image
            *morph_image;

          (void) SyncImagesSettings(image_info,*images);
          morph_image=MorphImages(*images,StringToUnsignedLong(argv[1]),
            exception);
          if (morph_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=morph_image;
          break;
        }
      if (LocaleCompare("mosaic",argv[0]+1) == 0)
        {
          Image
            *mosaic_image;

          (void) SyncImagesSettings(image_info,*images);
          mosaic_image=MergeImageLayers(*images,MosaicLayer,exception);
          if (mosaic_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=mosaic_image;
          break;
        }
      break;
    }
    case 'p':
    {
      if (LocaleCompare("print",argv[0]+1) == 0)
        {
          char
            *string;

          (void) SyncImagesSettings(image_info,*images);
          string=InterpretImageProperties(image_info,*images,argv[1],
            exception);
          if (string == (char *) NULL)
            break;
          (void) FormatLocaleFile(stdout,"%s",string);
          string=DestroyString(string);
        }
      if (LocaleCompare("process",argv[0]+1) == 0)
        {
          char
            **arguments;

          int
            j,
            number_arguments;

          (void) SyncImagesSettings(image_info,*images);
          arguments=StringToArgv(argv[1],&number_arguments);
          if (arguments == (char **) NULL)
            break;
          if ((argc > 1) && (strchr(arguments[1],'=') != (char *) NULL))
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
                Support old style syntax, filter="-option arg".
              */
              length=strlen(argv[1]);
              token=(char *) NULL;
              if (~length >= (MaxTextExtent-1))
                token=(char *) AcquireQuantumMemory(length+MaxTextExtent,
                  sizeof(*token));
              if (token == (char *) NULL)
                break;
              next=0;
              arguments=argv[1];
              token_info=AcquireTokenInfo();
              status=Tokenizer(token_info,0,token,length,arguments,"","=",
                "\"",'\0',&breaker,&next,&quote);
              token_info=DestroyTokenInfo(token_info);
              if (status == 0)
                {
                  const char
                    *argv;

                  argv=(&(arguments[next]));
                  (void) InvokeDynamicImageFilter(token,&(*images),1,&argv,
                    exception);
                }
              token=DestroyString(token);
              break;
            }
          (void) SubstituteString(&arguments[1],"-","");
          (void) InvokeDynamicImageFilter(arguments[1],&(*images),
            number_arguments-2,(const char **) arguments+2,exception);
          for (j=0; j < number_arguments; j++)
            arguments[j]=DestroyString(arguments[j]);
          arguments=(char **) RelinquishMagickMemory(arguments);
          break;
        }
      break;
    }
    case 'r':
    {
      if (LocaleCompare("reverse",argv[0]+1) == 0)
        {
          ReverseImageList(images);
          InheritException(exception,&(*images)->exception);
          break;
        }
      break;
    }
    case 's':
    {
      if (LocaleCompare("smush",argv[0]+1) == 0)
        {
          Image
            *smush_image;

          ssize_t
            offset;

          (void) SyncImagesSettings(image_info,*images);
          offset=(ssize_t) StringToLong(argv[1]);
          smush_image=SmushImages(*images,*argv[0] == '-' ? MagickTrue :
            MagickFalse,offset,exception);
          if (smush_image == (Image *) NULL)
            {
              status=MagickFalse;
              break;
            }
          *images=DestroyImageList(*images);
          *images=smush_image;
          break;
        }
      if (LocaleCompare("swap",argv[0]+1) == 0)
        {
          Image
            *p,
            *q,
            *swap;

          ssize_t
            swap_index;

          index=(-1);
          swap_index=(-2);
          if (*argv[0] != '+')
            {
              GeometryInfo
                geometry_info;

              MagickStatusType
                flags;

              swap_index=(-1);
              flags=ParseGeometry(argv[1],&geometry_info);
              index=(ssize_t) geometry_info.rho;
              if ((flags & SigmaValue) != 0)
                swap_index=(ssize_t) geometry_info.sigma;
            }
          p=GetImageFromList(*images,index);
          q=GetImageFromList(*images,swap_index);
          if ((p == (Image *) NULL) || (q == (Image *) NULL))
            {
              (void) ThrowMagickException(exception,GetMagickModule(),
                OptionError,"NoSuchImage","`%s'",(*images)->filename);
              status=MagickFalse;
              break;
            }
          if (p == q)
            break;
          swap=CloneImage(p,0,0,MagickTrue,exception);
          ReplaceImageInList(&p,CloneImage(q,0,0,MagickTrue,exception));
          ReplaceImageInList(&q,swap);
          *images=GetFirstImageInList(q);
          break;
        }
      break;
    }
    case 'w':
    {
      if (LocaleCompare("write",argv[0]+1) == 0)
        {
          char
            key[MaxTextExtent];

          Image
            *write_images;

          ImageInfo
            *write_info;

          (void) SyncImagesSettings(image_info,*images);
          (void) FormatLocaleString(key,MaxTextExtent,"cache:%s",argv[1]);
          (void) DeleteImageRegistry(key);
          write_images=(*images);
          if (*argv[0] == '+')
            write_images=CloneImageList(*images,exception);
          write_info=CloneImageInfo(image_info);
          status&=WriteImages(write_info,write_images,argv[1],exception);
          write_info=DestroyImageInfo(write_info);
          if (*argv[0] == '+')
            write_images=DestroyImageList(write_images);
          break;
        }
      break;
    }
    default:
      break;
  }
  quantize_info=DestroyQuantizeInfo(quantize_info);

  status=(MagickStatusType) ((*image)->exception.severity ==
    UndefinedException ? 1 : 0);
  return(status != 0 ? MagickTrue : MagickFalse);
}
#endif
