/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              M   M   OOO   GGGGG  RRRR   IIIII  FFFFF  Y   Y                %
%              MM MM  O   O  G      R   R    I    F       Y Y                 %
%              M M M  O   O  G GGG  RRRR     I    FFF      Y                  %
%              M   M  O   O  G   G  R R      I    F        Y                  %
%              M   M   OOO   GGGG   R  R   IIIII  F        Y                  %
%                                                                             %
%                                                                             %
%                         MagickWand Module Methods                           %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                March 2000                                   %
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
%  Use the mogrify program to resize an image, blur, crop, despeckle, dither,
%  draw on, flip, join, re-sample, and much more. This tool is similiar to
%  convert except that the original image file is overwritten (unless you
%  change the file suffix with the -format option) with any changes you
%  request.
%
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/mogrify-private.h"
#include "magick/monitor-private.h"
#include "magick/thread-private.h"
#include "magick/string-private.h"

/*
  Define declarations.
*/
#define UndefinedCompressionQuality  0UL

/*
  Constant declaration.
*/
static const char
  BackgroundColor[] = "#fff",  /* white */
  BorderColor[] = "#dfdfdf",  /* gray */
  MatteColor[] = "#bdbdbd";  /* gray */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     M a g i c k C o m m a n d G e n e s i s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCommandGenesis() applies image processing options to an image as
%  prescribed by command line options.
%
%  The format of the MagickCommandGenesis method is:
%
%      MagickBooleanType MagickCommandGenesis(ImageInfo *image_info,
%        MagickCommand command,const int argc,const char **argv,Image **image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o command: the magick command.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType MagickCommandGenesis(ImageInfo *image_info,
  MagickCommand command,int argc,char **argv,char **metadata,
  ExceptionInfo *exception)
{
  char
    *option;

  double
    duration,
    elapsed_time,
    user_time;

  MagickBooleanType
    concurrent,
    regard_warnings,
    status;

  register long
    i;

  TimerInfo
    *timer;

  unsigned long
    iterations;

  concurrent=MagickFalse;
  duration=(-1.0);
  iterations=1;
  status=MagickFalse;
  regard_warnings=MagickFalse;
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if ((strlen(option) == 1) || ((*option != '-') && (*option != '+')))
      continue;
    if (LocaleCompare("bench",option+1) == 0)
      iterations=StringToUnsignedLong(argv[++i]);
    if (LocaleCompare("concurrent",option+1) == 0)
      concurrent=MagickTrue;
    if (LocaleCompare("debug",option+1) == 0)
      (void) SetLogEventMask(argv[++i]);
    if (LocaleCompare("duration",option+1) == 0)
      duration=StringToDouble(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=MagickTrue;
  }
  timer=AcquireTimerInfo();
  if (concurrent == MagickFalse)
    {
      for (i=0; i < (long) iterations; i++)
      {
        if (status != MagickFalse)
          continue;
        if (duration > 0)
          {
            if (GetElapsedTime(timer) > duration)
              continue;
            (void) ContinueTimer(timer);
          }
        status=command(image_info,argc,argv,metadata,exception);
        if (exception->severity != UndefinedException)
          {
            if ((exception->severity > ErrorException) ||
                (regard_warnings != MagickFalse))
              status=MagickTrue;
            CatchException(exception);
          }
        if ((metadata != (char **) NULL) && (*metadata != (char *) NULL))
          {
            (void) fputs(*metadata,stdout);
            (void) fputc('\n',stdout);
            *metadata=DestroyString(*metadata);
          }
      }
    }
  else
    {
      SetOpenMPNested(1);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  # pragma omp parallel for shared(status)
#endif
      for (i=0; i < (long) iterations; i++)
      {
        if (status != MagickFalse)
          continue;
        if (duration > 0)
          {
            if (GetElapsedTime(timer) > duration)
              continue;
            (void) ContinueTimer(timer);
          }
        status=command(image_info,argc,argv,metadata,exception);
#if defined(MAGICKCORE_OPENMP_SUPPORT)
  # pragma omp critical (MagickCore_Launch_Command)
#endif
        {
          if (exception->severity != UndefinedException)
            {
              if ((exception->severity > ErrorException) ||
                  (regard_warnings != MagickFalse))
                status=MagickTrue;
              CatchException(exception);
            }
          if ((metadata != (char **) NULL) && (*metadata != (char *) NULL))
            {
              (void) fputs(*metadata,stdout);
              (void) fputc('\n',stdout);
              *metadata=DestroyString(*metadata);
            }
        }
      }
    }
  if (iterations > 1)
    {
      elapsed_time=GetElapsedTime(timer);
      user_time=GetUserTime(timer);
      (void) fprintf(stderr,
        "Performance: %lui %gips %0.3fu %ld:%02ld.%03ld\n",
        iterations,1.0*iterations/elapsed_time,user_time,(long)
        (elapsed_time/60.0),(long) floor(fmod(elapsed_time,60.0)),
        (long) (1000.0*(elapsed_time-floor(elapsed_time))));
    }
  timer=DestroyTimerInfo(timer);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     M o g r i f y I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MogrifyImage() applies image processing options to an image as prescribed
%  by command line options.
%
%  The format of the MogrifyImage method is:
%
%      MagickBooleanType MogrifyImage(ImageInfo *image_info,const int argc,
%        const char **argv,Image **image)
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
%    o image: the image.
%
%    o exception: return any errors or warnings in this structure.
%
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

  (void) FormatMagickString(key,MaxTextExtent,"cache:%s",path);
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

static int IsPathDirectory(const char *path)
{
  MagickBooleanType
    status;

  struct stat
    attributes;

  if ((path == (const char *) NULL) || (*path == '\0'))
    return(MagickFalse);
  status=GetPathAttributes(path,&attributes);
  if (status == MagickFalse)
    return(-1);
  if (S_ISDIR(attributes.st_mode) == 0)
    return(0);
  return(1);
}

static MagickBooleanType IsPathWritable(const char *path)
{
  if (IsPathAccessible(path) == MagickFalse)
    return(MagickFalse);
  if (access(path,W_OK) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

static inline long MagickMax(const long x,const long y)
{
  if (x > y)
    return(x);
  return(y);
}

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
  (void) FormatMagickString(message,MaxTextExtent,"Monitor/%s",tag);
  locale_message=GetLocaleMessage(message);
  if (locale_message == message)
    locale_message=tag;
  if (p == (char *) NULL)
    (void) fprintf(stderr,"%s: %ld of %lu, %02ld%% complete\r",locale_message,
      (long) offset,(unsigned long) extent,(long) (100L*offset/(extent-1)));
  else
    (void) fprintf(stderr,"%s[%s]: %ld of %lu, %02ld%% complete\r",
      locale_message,p+1,(long) offset,(unsigned long) extent,(long)
      (100L*offset/(extent-1)));
  if (offset == (MagickOffsetType) (extent-1))
    (void) fprintf(stderr,"\n");
  (void) fflush(stderr);
  return(MagickTrue);
}

static Image *SparseColorOption(const Image *image,const ChannelType channel,
  const SparseColorMethod method,const char *arguments,
  const MagickBooleanType color_from_image,ExceptionInfo *exception)
{
  ChannelType
    channels;

  char
    token[MaxTextExtent];

  const char
    *p;

  double
    *sparse_arguments;

  register unsigned long
    x;

  unsigned long
    number_arguments;

  unsigned long
    number_colors;

  Image
    *sparse_image;

  MagickPixelPacket
    color;

  MagickBooleanType
    error;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  /*
    Limit channels according to image - and add up number of color channel.
  */
  channels=channel;
  if (image->colorspace != CMYKColorspace)
    channels=(ChannelType) (channels & ~IndexChannel);  /* no index channel */
  if (image->matte == MagickFalse)
    channels=(ChannelType) (channels & ~OpacityChannel);  /* no alpha channel */
  number_colors=0;
  if ((channels & RedChannel) != 0)
    number_colors++;
  if ((channels & GreenChannel) != 0)
    number_colors++;
  if ((channels & BlueChannel) != 0)
    number_colors++;
  if ((channels & IndexChannel) != 0)
    number_colors++;
  if ((channels & OpacityChannel) != 0)
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
    sparse_arguments[x++]=StringToDouble(token);
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
    sparse_arguments[x++]=StringToDouble(token);
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
        (void) QueryMagickColor(token,&color,exception);
        if ( channels & RedChannel )
          sparse_arguments[x++] = QuantumScale*color.red;
        if ( channels & GreenChannel )
          sparse_arguments[x++] = QuantumScale*color.green;
        if ( channels & BlueChannel )
          sparse_arguments[x++] = QuantumScale*color.blue;
        if ( channels & IndexChannel )
          sparse_arguments[x++] = QuantumScale*color.index;
        if ( channels & OpacityChannel )
          sparse_arguments[x++] = QuantumScale*color.opacity;
      }
      else {
#if 0
        /* the color name/function/value was not found - error */
        break;
#else
        /* Colors given as a set of floating point values - experimental */
        /* NB: token contains the first floating point value to use! */
        if ( channels & RedChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & GreenChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & BlueChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & IndexChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & OpacityChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token);
          token[0] = ','; /* used this token - get another */
        }
#endif
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
  sparse_image=SparseColorImage(image,channels,method,number_arguments,
    sparse_arguments,exception);
  sparse_arguments=(double *) RelinquishMagickMemory(sparse_arguments);
  return( sparse_image );
}

WandExport MagickBooleanType MogrifyImage(ImageInfo *image_info,const int argc,
  const char **argv,Image **image,ExceptionInfo *exception)
{
  ChannelType
    channel;

  const char
    *format,
    *option;

  DrawInfo
    *draw_info;

  GeometryInfo
    geometry_info;

  Image
    *region_image;

  long
    count;

  MagickBooleanType
    status;

  MagickPixelPacket
    fill;

  MagickStatusType
    flags;

  QuantizeInfo
    *quantize_info;

  RectangleInfo
    geometry,
    region_geometry;

  register long
    i;

  /*
    Initialize method variables.
  */
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
  GetMagickPixelPacket(*image,&fill);
  SetMagickPixelPacket(*image,&(*image)->background_color,(IndexPacket *) NULL,
    &fill);
  channel=image_info->channel;
  format=GetImageOption(image_info,"format");
  SetGeometry(*image,&region_geometry);
  region_image=NewImageList();
  /*
    Transmogrify the image.
  */
  for (i=0; i < (long) argc; i++)
  {
    option=argv[i];
    if (IsMagickOption(option) == MagickFalse)
      continue;
    count=MagickMax(ParseMagickOption(MagickCommandOptions,MagickFalse,option),
      0L);
    if ((i+count) >= argc)
      break;
    status=MogrifyImageInfo(image_info,count+1,argv+i,exception);
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("adaptive-blur",option+1) == 0)
          {
            Image
              *blur_image;

            /*
              Adaptive blur image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            blur_image=AdaptiveBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,exception);
            if (blur_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=blur_image;
            break;
          }
        if (LocaleCompare("adaptive-resize",option+1) == 0)
          {
            Image
              *resize_image;

            /*
              Adaptive resize image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            resize_image=AdaptiveResizeImage(*image,geometry.width,
              geometry.height,exception);
            if (resize_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=resize_image;
            break;
          }
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            Image
              *sharp_image;

            /*
              Adaptive sharpen image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            sharp_image=AdaptiveSharpenImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,exception);
            if (sharp_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=sharp_image;
            break;
          }
        if (LocaleCompare("affine",option+1) == 0)
          {
            /*
              Affine matrix.
            */
            if (*option == '+')
              {
                GetAffineMatrix(&draw_info->affine);
                break;
              }
            (void) ParseAffineGeometry(argv[i+1],&draw_info->affine,exception);
            break;
          }
        if (LocaleCompare("alpha",option+1) == 0)
          {
            AlphaChannelType
              alpha_type;

            (void) SyncImageSettings(image_info,*image);
            alpha_type=(AlphaChannelType) ParseMagickOption(MagickAlphaOptions,
              MagickFalse,argv[i+1]);
            (void) SetImageAlphaChannel(*image,alpha_type);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("annotate",option+1) == 0)
          {
            char
              *text,
              geometry[MaxTextExtent];

            /*
              Annotate image.
            */
            (void) SyncImageSettings(image_info,*image);
            SetGeometryInfo(&geometry_info);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            text=InterpretImageProperties(image_info,*image,argv[i+2]);
            InheritException(exception,&(*image)->exception);
            if (text == (char *) NULL)
              break;
            (void) CloneString(&draw_info->text,text);
            text=DestroyString(text);
            (void) FormatMagickString(geometry,MaxTextExtent,"%+f%+f",
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
            (void) AnnotateImage(*image,draw_info);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("antialias",option+1) == 0)
          {
            draw_info->stroke_antialias=(*option == '-') ? MagickTrue :
              MagickFalse;
            draw_info->text_antialias=(*option == '-') ? MagickTrue :
              MagickFalse;
            break;
          }
        if (LocaleCompare("auto-gamma",option+1) == 0)
          {
            /*
              Auto Adjust Gamma of image based on its mean
            */
            (void) SyncImageSettings(image_info,*image);
            (void) AutoGammaImageChannel(*image,channel);
            break;
          }
        if (LocaleCompare("auto-level",option+1) == 0)
          {
            /*
              Perfectly Normalize (max/min stretch) the image
            */
            (void) SyncImageSettings(image_info,*image);
            (void) AutoLevelImageChannel(*image,channel);
            break;
          }
        if (LocaleCompare("auto-orient",option+1) == 0)
          {
            Image
              *orient_image;

            (void) SyncImageSettings(image_info,*image);
            orient_image=NewImageList();
            switch ((*image)->orientation)
            {
              case TopRightOrientation:
              {
                orient_image=FlopImage(*image,exception);
                break;
              }
              case BottomRightOrientation:
              {
                orient_image=RotateImage(*image,180.0,exception);
                break;
              }
              case BottomLeftOrientation:
              {
                orient_image=FlipImage(*image,exception);
                break;
              }
              case LeftTopOrientation:
              {
                orient_image=TransposeImage(*image,exception);
                break;
              }
              case RightTopOrientation:
              {
                orient_image=RotateImage(*image,90.0,exception);
                break;
              }
              case RightBottomOrientation:
              {
                orient_image=TransverseImage(*image,exception);
                break;
              }
              case LeftBottomOrientation:
              {
                orient_image=RotateImage(*image,270.0,exception);
                break;
              }
              default:
                break;
            }
            if (orient_image == (Image *) NULL)
              break;
            orient_image->orientation=TopLeftOrientation;
            *image=DestroyImage(*image);
            *image=orient_image;
            break;
          }
        break;
      }
      case 'b':
      {
        if (LocaleCompare("black-threshold",option+1) == 0)
          {
            /*
              Black threshold image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) BlackThresholdImageChannel(*image,channel,argv[i+1],
              exception);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("blue-shift",option+1) == 0)
          {
            Image
              *shift_image;

            /*
              Blue shift image.
            */
            (void) SyncImageSettings(image_info,*image);
            geometry_info.rho=1.5;
            if (*option == '-')
              flags=ParseGeometry(argv[i+1],&geometry_info);
            shift_image=BlueShiftImage(*image,geometry_info.rho,exception);
            if (shift_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=shift_image;
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            Image
              *blur_image;

            /*
              Gaussian blur image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            blur_image=BlurImageChannel(*image,channel,geometry_info.rho,
              geometry_info.sigma,exception);
            if (blur_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=blur_image;
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            Image
              *border_image;

            /*
              Surround image with a border of solid color.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            if ((flags & SigmaValue) == 0)
              geometry.height=geometry.width;
            border_image=BorderImage(*image,&geometry,exception);
            if (border_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=border_image;
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) QueryColorDatabase(BorderColor,&draw_info->border_color,
                  exception);
                break;
              }
            (void) QueryColorDatabase(argv[i+1],&draw_info->border_color,
              exception);
            break;
          }
        if (LocaleCompare("box",option+1) == 0)
          {
            (void) QueryColorDatabase(argv[i+1],&draw_info->undercolor,
              exception);
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

            /*
              Brightness / contrast image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            brightness=geometry_info.rho;
            contrast=0.0;
            if ((flags & SigmaValue) != 0)
              contrast=geometry_info.sigma;
            (void) BrightnessContrastImageChannel(*image,channel,brightness,
              contrast);
            InheritException(exception,&(*image)->exception);
            break;
          }
        break;
      }
      case 'c':
      {
        if (LocaleCompare("cdl",option+1) == 0)
          {
            char
              *color_correction_collection;

            /*
              Color correct with a color decision list.
            */
            (void) SyncImageSettings(image_info,*image);
            color_correction_collection=FileToString(argv[i+1],~0,exception);
            if (color_correction_collection == (char *) NULL)
              break;
            (void) ColorDecisionListImage(*image,color_correction_collection);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            if (*option == '+')
              {
                channel=DefaultChannels;
                break;
              }
            channel=(ChannelType) ParseChannelOption(argv[i+1]);
            break;
          }
        if (LocaleCompare("charcoal",option+1) == 0)
          {
            Image
              *charcoal_image;

            /*
              Charcoal image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            charcoal_image=CharcoalImage(*image,geometry_info.rho,
              geometry_info.sigma,exception);
            if (charcoal_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=charcoal_image;
            break;
          }
        if (LocaleCompare("chop",option+1) == 0)
          {
            Image
              *chop_image;

            /*
              Chop the image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            chop_image=ChopImage(*image,&geometry,exception);
            if (chop_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=chop_image;
            break;
          }
        if (LocaleCompare("clamp",option+1) == 0)
          {
            /*
              Clamp image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ClampImageChannel(*image,channel);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("clip",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              {
                (void) SetImageClipMask(*image,(Image *) NULL);
                InheritException(exception,&(*image)->exception);
                break;
              }
            (void) ClipImage(*image);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("clip-mask",option+1) == 0)
          {
            Image
              *mask;

            long
              y;

            register long
              x;

            register PixelPacket
              *restrict q;

            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              {
                /*
                  Remove a mask.
                */
                (void) SetImageMask(*image,(Image *) NULL);
                InheritException(exception,&(*image)->exception);
                break;
              }
            /*
              Set the image mask.
            */
            mask=GetImageCache(image_info,argv[i+1],exception);
            if (mask == (Image *) NULL)
              break;
            for (y=0; y < (long) mask->rows; y++)
            {
              q=GetAuthenticPixels(mask,0,y,mask->columns,1,exception);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (long) mask->columns; x++)
              {
                if (mask->matte == MagickFalse)
                  q->opacity=PixelIntensityToQuantum(q);
                q->red=q->opacity;
                q->green=q->opacity;
                q->blue=q->opacity;
                q++;
              }
              if (SyncAuthenticPixels(mask,exception) == MagickFalse)
                break;
            }
            if (SetImageStorageClass(mask,DirectClass) == MagickFalse)
              return(MagickFalse);
            mask->matte=MagickTrue;
            (void) SetImageClipMask(*image,mask);
            mask=DestroyImage(mask);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("clip-path",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) ClipImagePath(*image,argv[i+1],*option == '-' ? MagickTrue :
              MagickFalse);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("colorize",option+1) == 0)
          {
            Image
              *colorize_image;

            /*
              Colorize the image.
            */
            (void) SyncImageSettings(image_info,*image);
            colorize_image=ColorizeImage(*image,argv[i+1],draw_info->fill,
              exception);
            if (colorize_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=colorize_image;
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            /*
              Reduce the number of colors in the image.
            */
            (void) SyncImageSettings(image_info,*image);
            quantize_info->number_colors=StringToUnsignedLong(argv[i+1]);
            if (quantize_info->number_colors == 0)
              break;
            if (((*image)->storage_class == DirectClass) ||
                (*image)->colors > quantize_info->number_colors)
              (void) QuantizeImage(quantize_info,*image);
            else
              (void) CompressImageColormap(*image);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("colorspace",option+1) == 0)
          {
            ColorspaceType
              colorspace;

            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              {
                (void) TransformImageColorspace(*image,RGBColorspace);
                InheritException(exception,&(*image)->exception);
                break;
              }
            colorspace=(ColorspaceType) ParseMagickOption(
              MagickColorspaceOptions,MagickFalse,argv[i+1]);
            (void) TransformImageColorspace(*image,colorspace);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("contrast",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) ContrastImage(*image,(*option == '-') ? MagickTrue :
              MagickFalse);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("contrast-stretch",option+1) == 0)
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
            flags=ParseGeometry(argv[i+1],&geometry_info);
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
            (void) ContrastStretchImageChannel(*image,channel,black_point,
              white_point);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("convolve",option+1) == 0)
          {
            double
              gamma;

            Image
              *convolve_image;

            KernelInfo
              *kernel;

            register long
              j;

            (void) SyncImageSettings(image_info,*image);
            kernel=AcquireKernelInfo(argv[i+1]);
            if (kernel == (KernelInfo *) NULL)
              break;
            gamma=0.0;
            for (j=0; j < (long) (kernel->width*kernel->height); j++)
              gamma+=kernel->values[j];
            gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
            for (j=0; j < (long) (kernel->width*kernel->height); j++)
              kernel->values[j]*=gamma;
            convolve_image=FilterImageChannel(*image,channel,kernel,exception);
            kernel=DestroyKernelInfo(kernel);
            if (convolve_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=convolve_image;
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            if (((geometry.width != 0) || (geometry.height != 0)) &&
                ((flags & XValue) == 0) && ((flags & YValue) == 0))
              break;
            (void) TransformImage(image,argv[i+1],(char *) NULL);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("cycle",option+1) == 0)
          {
            /*
              Cycle an image colormap.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) CycleColormapImage(*image,StringToLong(argv[i+1]));
            InheritException(exception,&(*image)->exception);
            break;
          }
        break;
      }
      case 'd':
      {
        if (LocaleCompare("decipher",option+1) == 0)
          {
            StringInfo
              *passkey;

            /*
              Decipher pixels.
            */
            (void) SyncImageSettings(image_info,*image);
            passkey=FileToStringInfo(argv[i+1],~0,exception);
            if (passkey != (StringInfo *) NULL)
              {
                (void) PasskeyDecipherImage(*image,passkey,exception);
                passkey=DestroyStringInfo(passkey);
              }
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            /*
              Set image density.
            */
            (void) CloneString(&draw_info->density,argv[i+1]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              {
                (void) SetImageDepth(*image,MAGICKCORE_QUANTUM_DEPTH);
                break;
              }
            (void) SetImageDepth(*image,StringToUnsignedLong(argv[i+1]));
            break;
          }
        if (LocaleCompare("deskew",option+1) == 0)
          {
            double
              threshold;

            Image
              *deskew_image;

            /*
              Straighten the image.
            */
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              threshold=40.0*QuantumRange/100.0;
            else
              threshold=SiPrefixToDouble(argv[i+1],QuantumRange);
            deskew_image=DeskewImage(*image,threshold,exception);
            if (deskew_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=deskew_image;
            break;
          }
        if (LocaleCompare("despeckle",option+1) == 0)
          {
            Image
              *despeckle_image;

            /*
              Reduce the speckles within an image.
            */
            (void) SyncImageSettings(image_info,*image);
            despeckle_image=DespeckleImage(*image,exception);
            if (despeckle_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=despeckle_image;
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            (void) CloneString(&draw_info->server_name,argv[i+1]);
            break;
          }
        if (LocaleCompare("distort",option+1) == 0)
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

            Image
              *distort_image;

            register long
              x;

            unsigned long
              number_arguments;

            /*
              Distort image.
            */
            (void) SyncImageSettings(image_info,*image);
            method=(DistortImageMethod) ParseMagickOption(MagickDistortOptions,
              MagickFalse,argv[i+1]);
            args=InterpretImageProperties(image_info,*image,argv[i+2]);
            InheritException(exception,&(*image)->exception);
            if (args == (char *) NULL)
              break;
            p=(char *) args;
            for (x=0; *p != '\0'; x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
            }
            number_arguments=(unsigned long) x;
            arguments=(double *) AcquireQuantumMemory(number_arguments,
              sizeof(*arguments));
            if (arguments == (double *) NULL)
              ThrowWandFatalException(ResourceLimitFatalError,
                "MemoryAllocationFailed",(*image)->filename);
            (void) ResetMagickMemory(arguments,0,number_arguments*
              sizeof(*arguments));
            p=(char *) args;
            for (x=0; (x < (long) number_arguments) && (*p != '\0'); x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              arguments[x]=StringToDouble(token);
            }
            args=DestroyString(args);
            distort_image=DistortImage(*image,method,number_arguments,arguments,
              (*option == '+') ? MagickTrue : MagickFalse,exception);
            arguments=(double *) RelinquishMagickMemory(arguments);
            if (distort_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=distort_image;
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            if (*option == '+')
              {
                quantize_info->dither=MagickFalse;
                break;
              }
            quantize_info->dither=MagickTrue;
            quantize_info->dither_method=(DitherMethod) ParseMagickOption(
              MagickDitherOptions,MagickFalse,argv[i+1]);
            if (quantize_info->dither_method == NoDitherMethod)
              quantize_info->dither=MagickFalse;
            break;
          }
        if (LocaleCompare("draw",option+1) == 0)
          {
            /*
              Draw image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) CloneString(&draw_info->primitive,argv[i+1]);
            (void) DrawImage(*image,draw_info);
            InheritException(exception,&(*image)->exception);
            break;
          }
        break;
      }
      case 'e':
      {
        if (LocaleCompare("edge",option+1) == 0)
          {
            Image
              *edge_image;

            /*
              Enhance edges in the image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            edge_image=EdgeImage(*image,geometry_info.rho,exception);
            if (edge_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=edge_image;
            break;
          }
        if (LocaleCompare("emboss",option+1) == 0)
          {
            Image
              *emboss_image;

            /*
              Gaussian embossen image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            emboss_image=EmbossImage(*image,geometry_info.rho,
              geometry_info.sigma,exception);
            if (emboss_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=emboss_image;
            break;
          }
        if (LocaleCompare("encipher",option+1) == 0)
          {
            StringInfo
              *passkey;

            /*
              Encipher pixels.
            */
            (void) SyncImageSettings(image_info,*image);
            passkey=FileToStringInfo(argv[i+1],~0,exception);
            if (passkey != (StringInfo *) NULL)
              {
                (void) PasskeyEncipherImage(*image,passkey,exception);
                passkey=DestroyStringInfo(passkey);
              }
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            (void) CloneString(&draw_info->encoding,argv[i+1]);
            break;
          }
        if (LocaleCompare("enhance",option+1) == 0)
          {
            Image
              *enhance_image;

            /*
              Enhance image.
            */
            (void) SyncImageSettings(image_info,*image);
            enhance_image=EnhanceImage(*image,exception);
            if (enhance_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=enhance_image;
            break;
          }
        if (LocaleCompare("equalize",option+1) == 0)
          {
            /*
              Equalize image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) EqualizeImageChannel(*image,channel);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("evaluate",option+1) == 0)
          {
            double
              constant;

            MagickEvaluateOperator
              op;

            (void) SyncImageSettings(image_info,*image);
            op=(MagickEvaluateOperator) ParseMagickOption(MagickEvaluateOptions,
              MagickFalse,argv[i+1]);
            constant=SiPrefixToDouble(argv[i+2],QuantumRange);
            (void) EvaluateImageChannel(*image,channel,op,constant,exception);
            break;
          }
        if (LocaleCompare("extent",option+1) == 0)
          {
            Image
              *extent_image;

            /*
              Set the image extent.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            if (geometry.width == 0)
              geometry.width=(*image)->columns;
            if (geometry.height == 0)
              geometry.height=(*image)->rows;
            geometry.x=(-geometry.x);
            geometry.y=(-geometry.y);
            extent_image=ExtentImage(*image,&geometry,exception);
            if (extent_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=extent_image;
            break;
          }
        break;
      }
      case 'f':
      {
        if (LocaleCompare("family",option+1) == 0)
          {
            if (*option == '+')
              {
                if (draw_info->family != (char *) NULL)
                  draw_info->family=DestroyString(draw_info->family);
                break;
              }
            (void) CloneString(&draw_info->family,argv[i+1]);
            break;
          }
        if (LocaleCompare("features",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageArtifact(*image,"identify:features");
                break;
              }
            (void) SetImageArtifact(*image,"identify:features",argv[i+1]);
            break;
          }
        if (LocaleCompare("fill",option+1) == 0)
          {
            ExceptionInfo
              *sans;

            GetMagickPixelPacket(*image,&fill);
            if (*option == '+')
              {
                (void) QueryMagickColor("none",&fill,exception);
                (void) QueryColorDatabase("none",&draw_info->fill,exception);
                if (draw_info->fill_pattern != (Image *) NULL)
                  draw_info->fill_pattern=DestroyImage(draw_info->fill_pattern);
                break;
              }
            sans=AcquireExceptionInfo();
            (void) QueryMagickColor(argv[i+1],&fill,sans);
            status=QueryColorDatabase(argv[i+1],&draw_info->fill,sans);
            sans=DestroyExceptionInfo(sans);
            if (status == MagickFalse)
              draw_info->fill_pattern=GetImageCache(image_info,argv[i+1],
                exception);
            break;
          }
        if (LocaleCompare("flip",option+1) == 0)
          {
            Image
              *flip_image;

            /*
              Flip image scanlines.
            */
            (void) SyncImageSettings(image_info,*image);
            flip_image=FlipImage(*image,exception);
            if (flip_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=flip_image;
            break;
          }
        if (LocaleCompare("flop",option+1) == 0)
          {
            Image
              *flop_image;

            /*
              Flop image scanlines.
            */
            (void) SyncImageSettings(image_info,*image);
            flop_image=FlopImage(*image,exception);
            if (flop_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=flop_image;
            break;
          }
        if (LocaleCompare("floodfill",option+1) == 0)
          {
            MagickPixelPacket
              target;

            /*
              Floodfill image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            (void) QueryMagickColor(argv[i+2],&target,exception);
            (void) FloodfillPaintImage(*image,channel,draw_info,&target,
              geometry.x,geometry.y,*option == '-' ? MagickFalse : MagickTrue);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              {
                if (draw_info->font != (char *) NULL)
                  draw_info->font=DestroyString(draw_info->font);
                break;
              }
            (void) CloneString(&draw_info->font,argv[i+1]);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            format=argv[i+1];
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            FrameInfo
              frame_info;

            Image
              *frame_image;

            /*
              Surround image with an ornamental border.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            frame_info.width=geometry.width;
            frame_info.height=geometry.height;
            if ((flags & HeightValue) == 0)
              frame_info.height=geometry.width;
            frame_info.outer_bevel=geometry.x;
            frame_info.inner_bevel=geometry.y;
            frame_info.x=(long) frame_info.width;
            frame_info.y=(long) frame_info.height;
            frame_info.width=(*image)->columns+2*frame_info.width;
            frame_info.height=(*image)->rows+2*frame_info.height;
            frame_image=FrameImage(*image,&frame_info,exception);
            if (frame_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=frame_image;
            break;
          }
        if (LocaleCompare("function",option+1) == 0)
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

            register long
              x;

            unsigned long
              number_parameters;

            /*
              Function Modify Image Values
            */
            (void) SyncImageSettings(image_info,*image);
            function=(MagickFunction) ParseMagickOption(MagickFunctionOptions,
              MagickFalse,argv[i+1]);
            arguments=InterpretImageProperties(image_info,*image,argv[i+2]);
            InheritException(exception,&(*image)->exception);
            if (arguments == (char *) NULL)
              break;
            p=(char *) arguments;
            for (x=0; *p != '\0'; x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
            }
            number_parameters=(unsigned long) x;
            parameters=(double *) AcquireQuantumMemory(number_parameters,
              sizeof(*parameters));
            if (parameters == (double *) NULL)
              ThrowWandFatalException(ResourceLimitFatalError,
                "MemoryAllocationFailed",(*image)->filename);
            (void) ResetMagickMemory(parameters,0,number_parameters*
              sizeof(*parameters));
            p=(char *) arguments;
            for (x=0; (x < (long) number_parameters) && (*p != '\0'); x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              parameters[x]=StringToDouble(token);
            }
            arguments=DestroyString(arguments);
            (void) FunctionImageChannel(*image,channel,function,
              number_parameters,parameters,exception);
            parameters=(double *) RelinquishMagickMemory(parameters);
            break;
          }
        break;
      }
      case 'g':
      {
        if (LocaleCompare("gamma",option+1) == 0)
          {
            /*
              Gamma image.
            */
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              (*image)->gamma=StringToDouble(argv[i+1]);
            else
              {
                if (strchr(argv[i+1],',') != (char *) NULL)
                  (void) GammaImage(*image,argv[i+1]);
                else
                  (void) GammaImageChannel(*image,channel,
                    StringToDouble(argv[i+1]));
                InheritException(exception,&(*image)->exception);
              }
            break;
          }
        if ((LocaleCompare("gaussian-blur",option+1) == 0) ||
            (LocaleCompare("gaussian",option+1) == 0))
          {
            Image
              *gaussian_image;

            /*
              Gaussian blur image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            gaussian_image=GaussianBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,exception);
            if (gaussian_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=gaussian_image;
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              {
                if ((*image)->geometry != (char *) NULL)
                  (*image)->geometry=DestroyString((*image)->geometry);
                break;
              }
            flags=ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            if (((flags & XValue) != 0) || ((flags & YValue) != 0))
              (void) CloneString(&(*image)->geometry,argv[i+1]);
            else
              {
                Image
                  *zoom_image;

                /*
                  Resize image.
                */
                zoom_image=ZoomImage(*image,geometry.width,geometry.height,
                  exception);
                if (zoom_image == (Image *) NULL)
                  break;
                *image=DestroyImage(*image);
                *image=zoom_image;
              }
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            if (*option == '+')
              {
                draw_info->gravity=UndefinedGravity;
                break;
              }
            draw_info->gravity=(GravityType) ParseMagickOption(
              MagickGravityOptions,MagickFalse,argv[i+1]);
            break;
          }
        break;
      }
      case 'h':
      {
        if (LocaleCompare("highlight-color",option+1) == 0)
          {
            (void) SetImageArtifact(*image,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'i':
      {
        if (LocaleCompare("identify",option+1) == 0)
          {
            char
              *text;

            (void) SyncImageSettings(image_info,*image);
            if (format == (char *) NULL)
              {
                (void) IdentifyImage(*image,stdout,image_info->verbose);
                InheritException(exception,&(*image)->exception);
                break;
              }
            text=InterpretImageProperties(image_info,*image,format);
            InheritException(exception,&(*image)->exception);
            if (text == (char *) NULL)
              break;
            (void) fputs(text,stdout);
            (void) fputc('\n',stdout);
            text=DestroyString(text);
            break;
          }
        if (LocaleCompare("implode",option+1) == 0)
          {
            Image
              *implode_image;

            /*
              Implode image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            implode_image=ImplodeImage(*image,geometry_info.rho,exception);
            if (implode_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=implode_image;
            break;
          }
        if (LocaleCompare("interline-spacing",option+1) == 0)
          {
            if (*option == '+')
              (void) ParseGeometry("0",&geometry_info);
            else
              (void) ParseGeometry(argv[i+1],&geometry_info);
            draw_info->interline_spacing=geometry_info.rho;
            break;
          }
        if (LocaleCompare("interword-spacing",option+1) == 0)
          {
            if (*option == '+')
              (void) ParseGeometry("0",&geometry_info);
            else
              (void) ParseGeometry(argv[i+1],&geometry_info);
            draw_info->interword_spacing=geometry_info.rho;
            break;
          }
        break;
      }
      case 'k':
      {
        if (LocaleCompare("kerning",option+1) == 0)
          {
            if (*option == '+')
              (void) ParseGeometry("0",&geometry_info);
            else
              (void) ParseGeometry(argv[i+1],&geometry_info);
            draw_info->kerning=geometry_info.rho;
            break;
          }
        break;
      }
      case 'l':
      {
        if (LocaleCompare("lat",option+1) == 0)
          {
            Image
              *threshold_image;

            /*
              Local adaptive threshold image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & PercentValue) != 0)
              geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
            threshold_image=AdaptiveThresholdImage(*image,(unsigned long)
              geometry_info.rho,(unsigned long) geometry_info.sigma,
              (long) geometry_info.xi,exception);
            if (threshold_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=threshold_image;
            break;
          }
        if (LocaleCompare("level",option+1) == 0)
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
            flags=ParseGeometry(argv[i+1],&geometry_info);
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
            if ((*option == '+') || ((flags & AspectValue) != 0))
              (void) LevelizeImageChannel(*image,channel,black_point,
                white_point,gamma);
            else
              (void) LevelImageChannel(*image,channel,black_point,white_point,
                gamma);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("level-colors",option+1) == 0)
          {
            char
              token[MaxTextExtent];

            const char
              *p;

            MagickPixelPacket
              black_point,
              white_point;

            p=(const char *) argv[i+1];
            GetMagickToken(p,&p,token);  /* get black point color */
            if ((isalpha((int) *token) != 0) || ((*token == '#') != 0))
              (void) QueryMagickColor(token,&black_point,exception);
            else
              (void) QueryMagickColor("#000000",&black_point,exception);
            if (isalpha((int) token[0]) || (token[0] == '#'))
              GetMagickToken(p,&p,token);
            if (*token == '\0')
              white_point=black_point; /* set everything to that color */
            else
              {
                /*
                  Get white point color.
                */
                if ((isalpha((int) *token) == 0) && ((*token == '#') == 0))
                  GetMagickToken(p,&p,token);
                if ((isalpha((int) *token) != 0) || ((*token == '#') != 0))
                  (void) QueryMagickColor(token,&white_point,exception);
                else
                  (void) QueryMagickColor("#ffffff",&white_point,exception);
              }
            (void) LevelColorsImageChannel(*image,channel,&black_point,
              &white_point,*option == '+' ? MagickTrue : MagickFalse);
            break;
          }
        if (LocaleCompare("linear-stretch",option+1) == 0)
          {
            double
              black_point,
              white_point;

            MagickStatusType
              flags;

            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
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
            (void) LinearStretchImage(*image,black_point,white_point);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("linewidth",option+1) == 0)
          {
            draw_info->stroke_width=StringToDouble(argv[i+1]);
            break;
          }
        if (LocaleCompare("liquid-rescale",option+1) == 0)
          {
            Image
              *resize_image;

            /*
              Liquid rescale image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            if ((flags & XValue) == 0)
              geometry.x=1;
            if ((flags & YValue) == 0)
              geometry.y=0;
            resize_image=LiquidRescaleImage(*image,geometry.width,
              geometry.height,1.0*geometry.x,1.0*geometry.y,exception);
            if (resize_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=resize_image;
            break;
          }
        if (LocaleCompare("lowlight-color",option+1) == 0)
          {
            (void) SetImageArtifact(*image,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'm':
      {
        if (LocaleCompare("map",option+1) == 0)
          {
            Image
              *remap_image;

            /*
              Transform image colors to match this set of colors.
            */
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              break;
            remap_image=GetImageCache(image_info,argv[i+1],exception);
            if (remap_image == (Image *) NULL)
              break;
            (void) RemapImage(quantize_info,*image,remap_image);
            InheritException(exception,&(*image)->exception);
            remap_image=DestroyImage(remap_image);
            break;
          }
        if (LocaleCompare("mask",option+1) == 0)
          {
            Image
              *mask;

            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              {
                /*
                  Remove a mask.
                */
                (void) SetImageMask(*image,(Image *) NULL);
                InheritException(exception,&(*image)->exception);
                break;
              }
            /*
              Set the image mask.
            */
            mask=GetImageCache(image_info,argv[i+1],exception);
            if (mask == (Image *) NULL)
              break;
            (void) SetImageMask(*image,mask);
            mask=DestroyImage(mask);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("matte",option+1) == 0)
          {
            (void) SetImageAlphaChannel(*image,(*option == '-') ?
              SetAlphaChannel : DeactivateAlphaChannel );
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("median",option+1) == 0)
          {
            Image
              *median_image;

            /*
              Median filter image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            median_image=MedianFilterImage(*image,geometry_info.rho,exception);
            if (median_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=median_image;
            break;
          }
        if (LocaleCompare("modulate",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) ModulateImage(*image,argv[i+1]);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          {
            (void) SetImageProgressMonitor(*image,MonitorProgress,
              (void *) NULL);
            break;
          }
        if (LocaleCompare("monochrome",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) SetImageType(*image,BilevelType);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("morphology",option+1) == 0)
          {
            MorphologyMethod
              method;

            KernelInfo
              *kernel;

            char
              token[MaxTextExtent];

            const char
              *p;

            long
              iterations;

            Image
              *morphology_image;
            /*
              Morphological Image Operation
            */
            (void) SyncImageSettings(image_info,*image);
            p=argv[i+1];
            GetMagickToken(p,&p,token);
            method=(MorphologyMethod) ParseMagickOption(MagickMorphologyOptions,
                      MagickFalse,token);
            iterations=1L;
            GetMagickToken(p,&p,token);
            if ((*p == ':') || (*p == ','))
              GetMagickToken(p,&p,token);
            if ((*p != '\0'))
              iterations=StringToLong(p);
            kernel=AcquireKernelInfo(argv[i+2]);
            if (kernel == (KernelInfo *) NULL)
              ThrowWandFatalException(ResourceLimitFatalError,
                "MemoryAllocationFailed",(*image)->filename);
            if ( GetImageArtifact(*image,"showkernel") != (const char *) NULL)
              ShowKernelInfo(kernel);  /* display the kernel to stderr */
            morphology_image=MorphologyImageChannel(*image,channel,method,
              iterations,kernel,exception);
            kernel=DestroyKernelInfo(kernel);
            if (morphology_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=morphology_image;
            break;
          }
        if (LocaleCompare("motion-blur",option+1) == 0)
          {
            Image
              *blur_image;

            /*
              Motion blur image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            blur_image=MotionBlurImageChannel(*image,channel,geometry_info.rho,
              geometry_info.sigma,geometry_info.xi,exception);
            if (blur_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=blur_image;
            break;
          }
        break;
      }
      case 'n':
      {
        if (LocaleCompare("negate",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) NegateImageChannel(*image,channel,*option == '+' ?
              MagickTrue : MagickFalse);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("noise",option+1) == 0)
          {
            Image
              *noisy_image;

            (void) SyncImageSettings(image_info,*image);
            if (*option == '-')
              {
                (void) ParseGeometry(argv[i+1],&geometry_info);
                noisy_image=ReduceNoiseImage(*image,geometry_info.rho,
                  exception);
              }
            else
              {
                NoiseType
                  noise;

                noise=(NoiseType) ParseMagickOption(MagickNoiseOptions,
                  MagickFalse,argv[i+1]);
                noisy_image=AddNoiseImageChannel(*image,channel,noise,
                  exception);
              }
            if (noisy_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=noisy_image;
            break;
          }
        if (LocaleCompare("normalize",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) NormalizeImageChannel(*image,channel);
            InheritException(exception,&(*image)->exception);
            break;
          }
        break;
      }
      case 'o':
      {
        if (LocaleCompare("opaque",option+1) == 0)
          {
            MagickPixelPacket
              target;

            (void) SyncImageSettings(image_info,*image);
            (void) QueryMagickColor(argv[i+1],&target,exception);
            (void) OpaquePaintImageChannel(*image,channel,&target,&fill,
              *option == '-' ? MagickFalse : MagickTrue);
            break;
          }
        if (LocaleCompare("ordered-dither",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            (void) OrderedPosterizeImageChannel(*image,channel,argv[i+1],
              exception);
            break;
          }
        break;
      }
      case 'p':
      {
        if (LocaleCompare("paint",option+1) == 0)
          {
            Image
              *paint_image;

            /*
              Oil paint image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            paint_image=OilPaintImage(*image,geometry_info.rho,exception);
            if (paint_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=paint_image;
            break;
          }
        if (LocaleCompare("pen",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) QueryColorDatabase("none",&draw_info->fill,exception);
                break;
              }
            (void) QueryColorDatabase(argv[i+1],&draw_info->fill,exception);
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              (void) ParseGeometry("12",&geometry_info);
            else
              (void) ParseGeometry(argv[i+1],&geometry_info);
            draw_info->pointsize=geometry_info.rho;
            break;
          }
        if (LocaleCompare("polaroid",option+1) == 0)
          {
            double
              angle;

            Image
              *polaroid_image;

            RandomInfo
              *random_info;

            /*
              Simulate a Polaroid picture.
            */
            (void) SyncImageSettings(image_info,*image);
            random_info=AcquireRandomInfo();
            angle=22.5*(GetPseudoRandomValue(random_info)-0.5);
            random_info=DestroyRandomInfo(random_info);
            if (*option == '-')
              {
                SetGeometryInfo(&geometry_info);
                flags=ParseGeometry(argv[i+1],&geometry_info);
                angle=geometry_info.rho;
              }
            polaroid_image=PolaroidImage(*image,draw_info,angle,exception);
            if (polaroid_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=polaroid_image;
            break;
          }
        if (LocaleCompare("posterize",option+1) == 0)
          {
            /*
              Posterize image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) PosterizeImage(*image,StringToUnsignedLong(argv[i+1]),
              quantize_info->dither);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("preview",option+1) == 0)
          {
            Image
              *preview_image;

            PreviewType
              preview_type;

            /*
              Preview image.
            */
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              preview_type=UndefinedPreview;
            else
              preview_type=(PreviewType) ParseMagickOption(MagickPreviewOptions,
                MagickFalse,argv[i+1]);
            preview_image=PreviewImage(*image,preview_type,exception);
            if (preview_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=preview_image;
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
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
            if (*option == '+')
              {
                /*
                  Remove a profile from the image.
                */
                (void) ProfileImage(*image,argv[i+1],(const unsigned char *)
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
            profile_image=GetImageCache(profile_info,argv[i+1],exception);
            profile_info=DestroyImageInfo(profile_info);
            if (profile_image == (Image *) NULL)
              {
                char
                  name[MaxTextExtent],
                  filename[MaxTextExtent];

                register char
                  *p;

                StringInfo
                  *profile;

                (void) CopyMagickString(filename,argv[i+1],MaxTextExtent);
                (void) CopyMagickString(name,argv[i+1],MaxTextExtent);
                for (p=filename; *p != '\0'; p++)
                  if ((*p == ':') && (IsPathDirectory(argv[i+1]) < 0) &&
                      (IsPathAccessible(argv[i+1]) == MagickFalse))
                    {
                      register char
                        *q;

                      /*
                        Look for profile name (e.g. name:profile).
                      */
                      (void) CopyMagickString(name,filename,(size_t)
                        (p-filename+1));
                      for (q=filename; *q != '\0'; q++)
                        *q=(*++p);
                      break;
                    }
                profile=FileToStringInfo(filename,~0UL,exception);
                if (profile != (StringInfo *) NULL)
                  {
                    (void) ProfileImage(*image,name,GetStringInfoDatum(profile),
                      (unsigned long) GetStringInfoLength(profile),MagickFalse);
                    profile=DestroyStringInfo(profile);
                  }
                break;
              }
            ResetImageProfileIterator(profile_image);
            name=GetNextImageProfile(profile_image);
            while (name != (const char *) NULL)
            {
              profile=GetImageProfile(profile_image,name);
              if (profile != (StringInfo *) NULL)
                (void) ProfileImage(*image,name,GetStringInfoDatum(profile),
                  (unsigned long) GetStringInfoLength(profile),MagickFalse);
              name=GetNextImageProfile(profile_image);
            }
            profile_image=DestroyImage(profile_image);
            break;
          }
        break;
      }
      case 'q':
      {
        if (LocaleCompare("quantize",option+1) == 0)
          {
            if (*option == '+')
              {
                quantize_info->colorspace=UndefinedColorspace;
                break;
              }
            quantize_info->colorspace=(ColorspaceType) ParseMagickOption(
              MagickColorspaceOptions,MagickFalse,argv[i+1]);
            break;
          }
        break;
      }
      case 'r':
      {
        if (LocaleCompare("radial-blur",option+1) == 0)
          {
            Image
              *blur_image;

            /*
              Radial blur image.
            */
            (void) SyncImageSettings(image_info,*image);
            blur_image=RadialBlurImageChannel(*image,channel,
              StringToDouble(argv[i+1]),exception);
            if (blur_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=blur_image;
            break;
          }
        if (LocaleCompare("raise",option+1) == 0)
          {
            /*
              Surround image with a raise of solid color.
            */
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            if ((flags & SigmaValue) == 0)
              geometry.height=geometry.width;
            (void) RaiseImage(*image,&geometry,*option == '-' ? MagickTrue :
              MagickFalse);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("random-threshold",option+1) == 0)
          {
            /*
              Threshold image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) RandomThresholdImageChannel(*image,channel,argv[i+1],
              exception);
            break;
          }
        if (LocaleCompare("recolor",option+1) == 0)
          {
            char
              token[MaxTextExtent];

            const char
              *p;

            double
              *color_matrix;

            Image
              *recolor_image;

            register long
              x;

            unsigned long
              order;

            /*
              Transform color image.
            */
            (void) SyncImageSettings(image_info,*image);
            p=argv[i+1];
            for (x=0; *p != '\0'; x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
            }
            order=(unsigned long) sqrt((double) x+1.0);
            color_matrix=(double *) AcquireQuantumMemory(order,order*
              sizeof(*color_matrix));
            if (color_matrix == (double *) NULL)
              ThrowWandFatalException(ResourceLimitFatalError,
                "MemoryAllocationFailed",(*image)->filename);
            p=argv[i+1];
            for (x=0; (x < (long) (order*order)) && (*p != '\0'); x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              color_matrix[x]=StringToDouble(token);
            }
            for ( ; x < (long) (order*order); x++)
              color_matrix[x]=0.0;
            recolor_image=RecolorImage(*image,order,color_matrix,exception);
            color_matrix=(double *) RelinquishMagickMemory(color_matrix);
            if (recolor_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=recolor_image;
            break;
          }
        if (LocaleCompare("region",option+1) == 0)
          {
            Image
              *crop_image;

            (void) SyncImageSettings(image_info,*image);
            if (region_image != (Image *) NULL)
              {
                /*
                  Composite region.
                */
                (void) CompositeImage(region_image,(*image)->matte !=
                  MagickFalse ? OverCompositeOp : CopyCompositeOp,*image,
                  region_geometry.x,region_geometry.y);
                InheritException(exception,&region_image->exception);
                *image=DestroyImage(*image);
                *image=region_image;
              }
            if (*option == '+')
              {
                if (region_image != (Image *) NULL)
                  region_image=DestroyImage(region_image);
                break;
              }
            /*
              Apply transformations to a selected region of the image.
            */
            (void) ParseGravityGeometry(*image,argv[i+1],&region_geometry,
              exception);
            crop_image=CropImage(*image,&region_geometry,exception);
            if (crop_image == (Image *) NULL)
              break;
            region_image=(*image);
            *image=crop_image;
            break;
          }
        if (LocaleCompare("render",option+1) == 0)
          {
            (void) SyncImageSettings(image_info,*image);
            draw_info->render=(*option == '+') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("remap",option+1) == 0)
          {
            Image
              *remap_image;

            /*
              Transform image colors to match this set of colors.
            */
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              break;
            remap_image=GetImageCache(image_info,argv[i+1],exception);
            if (remap_image == (Image *) NULL)
              break;
            (void) RemapImage(quantize_info,*image,remap_image);
            InheritException(exception,&(*image)->exception);
            remap_image=DestroyImage(remap_image);
            break;
          }
        if (LocaleCompare("repage",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) ParseAbsoluteGeometry("0x0+0+0",&(*image)->page);
                break;
              }
            (void) ResetImagePage(*image,argv[i+1]);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("resample",option+1) == 0)
          {
            Image
              *resample_image;

            /*
              Resample image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            resample_image=ResampleImage(*image,geometry_info.rho,
              geometry_info.sigma,(*image)->filter,(*image)->blur,exception);
            if (resample_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=resample_image;
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            Image
              *resize_image;

            /*
              Resize image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            resize_image=ResizeImage(*image,geometry.width,geometry.height,
              (*image)->filter,(*image)->blur,exception);
            if (resize_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=resize_image;
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parenthesis=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("roll",option+1) == 0)
          {
            Image
              *roll_image;

            /*
              Roll image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            roll_image=RollImage(*image,geometry.x,geometry.y,exception);
            if (roll_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=roll_image;
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            char
              *geometry;

            Image
              *rotate_image;

            /*
              Check for conditional image rotation.
            */
            (void) SyncImageSettings(image_info,*image);
            if (strchr(argv[i+1],'>') != (char *) NULL)
              if ((*image)->columns <= (*image)->rows)
                break;
            if (strchr(argv[i+1],'<') != (char *) NULL)
              if ((*image)->columns >= (*image)->rows)
                break;
            /*
              Rotate image.
            */
            geometry=ConstantString(argv[i+1]);
            (void) SubstituteString(&geometry,">","");
            (void) SubstituteString(&geometry,"<","");
            (void) ParseGeometry(geometry,&geometry_info);
            geometry=DestroyString(geometry);
            rotate_image=RotateImage(*image,geometry_info.rho,exception);
            if (rotate_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=rotate_image;
            break;
          }
        break;
      }
      case 's':
      {
        if (LocaleCompare("sample",option+1) == 0)
          {
            Image
              *sample_image;

            /*
              Sample image with pixel replication.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            sample_image=SampleImage(*image,geometry.width,geometry.height,
              exception);
            if (sample_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=sample_image;
            break;
          }
        if (LocaleCompare("scale",option+1) == 0)
          {
            Image
              *scale_image;

            /*
              Resize image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            scale_image=ScaleImage(*image,geometry.width,geometry.height,
              exception);
            if (scale_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=scale_image;
            break;
          }
        if (LocaleCompare("selective-blur",option+1) == 0)
          {
            Image
              *blur_image;

            /*
              Selectively blur pixels within a contrast threshold.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & PercentValue) != 0)
              geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
            blur_image=SelectiveBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,geometry_info.xi,exception);
            if (blur_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=blur_image;
            break;
          }
        if (LocaleCompare("separate",option+1) == 0)
          {
            Image
              *separate_images;

            /*
              Break channels into separate images.
            */
            (void) SyncImageSettings(image_info,*image);
            separate_images=SeparateImages(*image,channel,exception);
            if (separate_images == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=separate_images;
            break;
          }
        if (LocaleCompare("sepia-tone",option+1) == 0)
          {
            double
              threshold;

            Image
              *sepia_image;

            /*
              Sepia-tone image.
            */
            (void) SyncImageSettings(image_info,*image);
            threshold=SiPrefixToDouble(argv[i+1],QuantumRange);
            sepia_image=SepiaToneImage(*image,threshold,exception);
            if (sepia_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=sepia_image;
            break;
          }
        if (LocaleCompare("segment",option+1) == 0)
          {
            /*
              Segment image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            (void) SegmentImage(*image,(*image)->colorspace,image_info->verbose,
              geometry_info.rho,geometry_info.sigma);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            /*
              Set image option.
            */
            if (LocaleNCompare(argv[i+1],"registry:",9) == 0)
              (void) DeleteImageRegistry(argv[i+1]+9);
            else
              if (LocaleNCompare(argv[i+1],"option:",7) == 0)
                (void) DeleteImageOption(image_info,argv[i+1]+7);
              else
                (void) DeleteImageProperty(*image,argv[i+1]);
            if (*option == '-')
              {
                char
                  *value;

                value=InterpretImageProperties(image_info,*image,argv[i+2]);
                if (value != (char *) NULL)
                  {
                    if (LocaleNCompare(argv[i+1],"registry:",9) == 0)
                      (void) SetImageRegistry(StringRegistryType,argv[i+1]+9,
                        value,exception);
                    else
                      if (LocaleNCompare(argv[i+1],"option:",7) == 0)
                        {
                          (void) SetImageOption(image_info,argv[i+1]+7,value);
                          (void) SetImageArtifact(*image,argv[i+1]+7,value);
                        }
                      else
                        (void) SetImageProperty(*image,argv[i+1],value);
                    value=DestroyString(value);
                  }
              }
            break;
          }
        if (LocaleCompare("shade",option+1) == 0)
          {
            Image
              *shade_image;

            /*
              Shade image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            shade_image=ShadeImage(*image,(*option == '-') ? MagickTrue :
              MagickFalse,geometry_info.rho,geometry_info.sigma,exception);
            if (shade_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=shade_image;
            break;
          }
        if (LocaleCompare("shadow",option+1) == 0)
          {
            Image
              *shadow_image;

            /*
              Shadow image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & XiValue) == 0)
              geometry_info.xi=4.0;
            if ((flags & PsiValue) == 0)
              geometry_info.psi=4.0;
            shadow_image=ShadowImage(*image,geometry_info.rho,
              geometry_info.sigma,(long) (geometry_info.xi+0.5),(long)
              (geometry_info.psi+0.5),exception);
            if (shadow_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=shadow_image;
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            Image
              *sharp_image;

            /*
              Sharpen image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            sharp_image=SharpenImageChannel(*image,channel,geometry_info.rho,
              geometry_info.sigma,exception);
            if (sharp_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=sharp_image;
            break;
          }
        if (LocaleCompare("shave",option+1) == 0)
          {
            Image
              *shave_image;

            /*
              Shave the image edges.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            shave_image=ShaveImage(*image,&geometry,exception);
            if (shave_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=shave_image;
            break;
          }
        if (LocaleCompare("shear",option+1) == 0)
          {
            Image
              *shear_image;

            /*
              Shear image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            shear_image=ShearImage(*image,geometry_info.rho,geometry_info.sigma,
              exception);
            if (shear_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=shear_image;
            break;
          }
        if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
          {
            /*
              Sigmoidal non-linearity contrast control.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=(double) QuantumRange/2.0;
            if ((flags & PercentValue) != 0)
              geometry_info.sigma=(double) QuantumRange*geometry_info.sigma/
                100.0;
            (void) SigmoidalContrastImageChannel(*image,channel,
              (*option == '-') ? MagickTrue : MagickFalse,geometry_info.rho,
              geometry_info.sigma);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("sketch",option+1) == 0)
          {
            Image
              *sketch_image;

            /*
              Sketch image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            sketch_image=SketchImage(*image,geometry_info.rho,
              geometry_info.sigma,geometry_info.xi,exception);
            if (sketch_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=sketch_image;
            break;
          }
        if (LocaleCompare("solarize",option+1) == 0)
          {
            double
              threshold;

            (void) SyncImageSettings(image_info,*image);
            threshold=SiPrefixToDouble(argv[i+1],QuantumRange);
            (void) SolarizeImage(*image,threshold);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("sparse-color",option+1) == 0)
          {
            Image
              *sparse_image;

            SparseColorMethod
              method;

            char
              *arguments;

            /*
              Sparse Color Interpolated Gradient
            */
            (void) SyncImageSettings(image_info,*image);
            method=(SparseColorMethod) ParseMagickOption(
              MagickSparseColorOptions,MagickFalse,argv[i+1]);
            arguments=InterpretImageProperties(image_info,*image,argv[i+2]);
            InheritException(exception,&(*image)->exception);
            if (arguments == (char *) NULL)
              break;
            sparse_image=SparseColorOption(*image,channel,method,arguments,
              option[0] == '+' ? MagickTrue : MagickFalse,exception);
            arguments=DestroyString(arguments);
            if (sparse_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=sparse_image;
            break;
          }
        if (LocaleCompare("splice",option+1) == 0)
          {
            Image
              *splice_image;

            /*
              Splice a solid color into the image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            splice_image=SpliceImage(*image,&geometry,exception);
            if (splice_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=splice_image;
            break;
          }
        if (LocaleCompare("spread",option+1) == 0)
          {
            Image
              *spread_image;

            /*
              Spread an image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            spread_image=SpreadImage(*image,geometry_info.rho,exception);
            if (spread_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=spread_image;
            break;
          }
        if (LocaleCompare("stretch",option+1) == 0)
          {
            if (*option == '+')
              {
                draw_info->stretch=UndefinedStretch;
                break;
              }
            draw_info->stretch=(StretchType) ParseMagickOption(
              MagickStretchOptions,MagickFalse,argv[i+1]);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          {
            /*
              Strip image of profiles and comments.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) StripImage(*image);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("stroke",option+1) == 0)
          {
            ExceptionInfo
              *sans;

            if (*option == '+')
              {
                (void) QueryColorDatabase("none",&draw_info->stroke,exception);
                if (draw_info->stroke_pattern != (Image *) NULL)
                  draw_info->stroke_pattern=DestroyImage(
                    draw_info->stroke_pattern);
                break;
              }
            sans=AcquireExceptionInfo();
            status=QueryColorDatabase(argv[i+1],&draw_info->stroke,sans);
            sans=DestroyExceptionInfo(sans);
            if (status == MagickFalse)
              draw_info->stroke_pattern=GetImageCache(image_info,argv[i+1],
                exception);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            draw_info->stroke_width=StringToDouble(argv[i+1]);
            break;
          }
        if (LocaleCompare("style",option+1) == 0)
          {
            if (*option == '+')
              {
                draw_info->style=UndefinedStyle;
                break;
              }
            draw_info->style=(StyleType) ParseMagickOption(MagickStyleOptions,
              MagickFalse,argv[i+1]);
            break;
          }
        if (LocaleCompare("swirl",option+1) == 0)
          {
            Image
              *swirl_image;

            /*
              Swirl image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            swirl_image=SwirlImage(*image,geometry_info.rho,exception);
            if (swirl_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=swirl_image;
            break;
          }
        break;
      }
      case 't':
      {
        if (LocaleCompare("threshold",option+1) == 0)
          {
            double
              threshold;

            /*
              Threshold image.
            */
            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              threshold=(double) QuantumRange/2.5;
            else
              threshold=SiPrefixToDouble(argv[i+1],QuantumRange);
            (void) BilevelImageChannel(*image,channel,threshold);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            Image
              *thumbnail_image;

            /*
              Thumbnail image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            thumbnail_image=ThumbnailImage(*image,geometry.width,
              geometry.height,exception);
            if (thumbnail_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=thumbnail_image;
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            if (*option == '+')
              {
                if (draw_info->fill_pattern != (Image *) NULL)
                  draw_info->fill_pattern=DestroyImage(draw_info->fill_pattern);
                break;
              }
            draw_info->fill_pattern=GetImageCache(image_info,argv[i+1],
              exception);
            break;
          }
        if (LocaleCompare("tint",option+1) == 0)
          {
            Image
              *tint_image;

            /*
              Tint the image.
            */
            (void) SyncImageSettings(image_info,*image);
            tint_image=TintImage(*image,argv[i+1],draw_info->fill,exception);
            if (tint_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=tint_image;
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          {
            Image
              *transform_image;

            /*
              Affine transform image.
            */
            (void) SyncImageSettings(image_info,*image);
            transform_image=AffineTransformImage(*image,&draw_info->affine,
              exception);
            if (transform_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=transform_image;
            break;
          }
        if (LocaleCompare("transparent",option+1) == 0)
          {
            MagickPixelPacket
              target;

            (void) SyncImageSettings(image_info,*image);
            (void) QueryMagickColor(argv[i+1],&target,exception);
            (void) TransparentPaintImage(*image,&target,(Quantum)
              TransparentOpacity,*option == '-' ? MagickFalse : MagickTrue);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("transpose",option+1) == 0)
          {
            Image
              *transpose_image;

            /*
              Transpose image scanlines.
            */
            (void) SyncImageSettings(image_info,*image);
            transpose_image=TransposeImage(*image,exception);
            if (transpose_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=transpose_image;
            break;
          }
        if (LocaleCompare("transverse",option+1) == 0)
          {
            Image
              *transverse_image;

            /*
              Transverse image scanlines.
            */
            (void) SyncImageSettings(image_info,*image);
            transverse_image=TransverseImage(*image,exception);
            if (transverse_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=transverse_image;
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            quantize_info->tree_depth=StringToUnsignedLong(argv[i+1]);
            break;
          }
        if (LocaleCompare("trim",option+1) == 0)
          {
            Image
              *trim_image;

            /*
              Trim image.
            */
            (void) SyncImageSettings(image_info,*image);
            trim_image=TrimImage(*image,exception);
            if (trim_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=trim_image;
            break;
          }
        if (LocaleCompare("type",option+1) == 0)
          {
            ImageType
              type;

            (void) SyncImageSettings(image_info,*image);
            if (*option == '+')
              type=UndefinedType;
            else
              type=(ImageType) ParseMagickOption(MagickTypeOptions,MagickFalse,
                argv[i+1]);
            (*image)->type=UndefinedType;
            (void) SetImageType(*image,type);
            InheritException(exception,&(*image)->exception);
            break;
          }
        break;
      }
      case 'u':
      {
        if (LocaleCompare("undercolor",option+1) == 0)
          {
            (void) QueryColorDatabase(argv[i+1],&draw_info->undercolor,
              exception);
            break;
          }
        if (LocaleCompare("unique",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageArtifact(*image,"identify:unique");
                break;
              }
            (void) SetImageArtifact(*image,"identify:unique","true");
            break;
          }
        if (LocaleCompare("unique-colors",option+1) == 0)
          {
            Image
              *unique_image;

            /*
              Unique image colors.
            */
            (void) SyncImageSettings(image_info,*image);
            unique_image=UniqueImageColors(*image,exception);
            if (unique_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=unique_image;
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            Image
              *unsharp_image;

            /*
              Unsharp mask image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & XiValue) == 0)
              geometry_info.xi=1.0;
            if ((flags & PsiValue) == 0)
              geometry_info.psi=0.05;
            unsharp_image=UnsharpMaskImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,geometry_info.xi,
              geometry_info.psi,exception);
            if (unsharp_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=unsharp_image;
            break;
          }
        break;
      }
      case 'v':
      {
        if (LocaleCompare("verbose",option+1) == 0)
          {
            (void) SetImageArtifact(*image,option+1,
              *option == '+' ? "false" : "true");
            break;
          }
        if (LocaleCompare("vignette",option+1) == 0)
          {
            Image
              *vignette_image;

            /*
              Vignette image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & XiValue) == 0)
              geometry_info.xi=0.1*(*image)->columns;
            if ((flags & PsiValue) == 0)
              geometry_info.psi=0.1*(*image)->rows;
            vignette_image=VignetteImage(*image,geometry_info.rho,
              geometry_info.sigma,(long) (geometry_info.xi+0.5),(long)
              (geometry_info.psi+0.5),exception);
            if (vignette_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=vignette_image;
            break;
          }
        if (LocaleCompare("virtual-pixel",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageVirtualPixelMethod(*image,
                  UndefinedVirtualPixelMethod);
                break;
              }
            (void) SetImageVirtualPixelMethod(*image,(VirtualPixelMethod)
              ParseMagickOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i+1]));
            break;
          }
        break;
      }
      case 'w':
      {
        if (LocaleCompare("wave",option+1) == 0)
          {
            Image
              *wave_image;

            /*
              Wave image.
            */
            (void) SyncImageSettings(image_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            wave_image=WaveImage(*image,geometry_info.rho,geometry_info.sigma,
              exception);
            if (wave_image == (Image *) NULL)
              break;
            *image=DestroyImage(*image);
            *image=wave_image;
            break;
          }
        if (LocaleCompare("weight",option+1) == 0)
          {
            draw_info->weight=StringToUnsignedLong(argv[i+1]);
            if (LocaleCompare(argv[i+1],"all") == 0)
              draw_info->weight=0;
            if (LocaleCompare(argv[i+1],"bold") == 0)
              draw_info->weight=700;
            if (LocaleCompare(argv[i+1],"bolder") == 0)
              if (draw_info->weight <= 800)
                draw_info->weight+=100;
            if (LocaleCompare(argv[i+1],"lighter") == 0)
              if (draw_info->weight >= 100)
                draw_info->weight-=100;
            if (LocaleCompare(argv[i+1],"normal") == 0)
              draw_info->weight=400;
            break;
          }
        if (LocaleCompare("white-threshold",option+1) == 0)
          {
            /*
              White threshold image.
            */
            (void) SyncImageSettings(image_info,*image);
            (void) WhiteThresholdImageChannel(*image,channel,argv[i+1],
              exception);
            InheritException(exception,&(*image)->exception);
            break;
          }
        break;
      }
      default:
        break;
    }
    i+=count;
  }
  if (region_image != (Image *) NULL)
    {
      /*
        Composite transformed region onto image.
      */
      (void) SyncImageSettings(image_info,*image);
      (void) CompositeImage(region_image,(*image)->matte != MagickFalse ?
        OverCompositeOp : CopyCompositeOp,*image,region_geometry.x,
        region_geometry.y);
      InheritException(exception,&region_image->exception);
      *image=DestroyImage(*image);
      *image=region_image;
    }
  /*
    Free resources.
  */
  quantize_info=DestroyQuantizeInfo(quantize_info);
  draw_info=DestroyDrawInfo(draw_info);
  status=(*image)->exception.severity == UndefinedException ?
    MagickTrue : MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    M o g r i f y I m a g e C o m m a n d                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MogrifyImageCommand() transforms an image or a sequence of images. These
%  transforms include image scaling, image rotation, color reduction, and
%  others. The transmogrified image overwrites the original image.
%
%  The format of the MogrifyImageCommand method is:
%
%      MagickBooleanType MogrifyImageCommand(ImageInfo *image_info,int argc,
%        const char **argv,char **metadata,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o metadata: any metadata is returned here.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType MogrifyUsage(void)
{
  static const char
    *miscellaneous[]=
    {
      "-debug events        display copious debugging information",
      "-help                print program options",
      "-list type           print a list of supported option arguments",
      "-log format          format of debugging information",
      "-version             print version information",
      (char *) NULL
    },
    *operators[]=
    {
      "-adaptive-blur geometry",
      "                     adaptively blur pixels; decrease effect near edges",
      "-adaptive-resize geometry",
      "                     adaptively resize image using 'mesh' interpolation",
      "-adaptive-sharpen geometry",
      "                     adaptively sharpen pixels; increase effect near edges",
      "-alpha option        on, activate, off, deactivate, set, opaque, copy",
      "                     transparent, extract, background, or shape",
      "-annotate geometry text",
      "                     annotate the image with text",
      "-auto-gamma          automagically adjust gamma level of image",
      "-auto-level          automagically adjust color levels of image",
      "-auto-orient         automagically orient (rotate) image",
      "-bench iterations    measure performance",
      "-black-threshold value",
      "                     force all pixels below the threshold into black",
      "-blue-shift          simulate a scene at nighttime in the moonlight",
      "-blur geometry       reduce image noise and reduce detail levels",
      "-border geometry     surround image with a border of color",
      "-bordercolor color   border color",
      "-brightness-contrast geometry",
      "                     improve brightness / contrast of the image",
      "-cdl filename        color correct with a color decision list",
      "-charcoal radius     simulate a charcoal drawing",
      "-chop geometry       remove pixels from the image interior",
      "-clamp               restrict pixel range from 0 to the quantum depth",
      "-clip                clip along the first path from the 8BIM profile",
      "-clip-mask filename  associate a clip mask with the image",
      "-clip-path id        clip along a named path from the 8BIM profile",
      "-colorize value      colorize the image with the fill color",
      "-contrast            enhance or reduce the image contrast",
      "-contrast-stretch geometry",
      "                     improve contrast by `stretching' the intensity range",
      "-convolve coefficients",
      "                     apply a convolution kernel to the image",
      "-cycle amount        cycle the image colormap",
      "-decipher filename   convert cipher pixels to plain pixels",
      "-deskew threshold    straighten an image",
      "-despeckle           reduce the speckles within an image",
      "-distort method args",
      "                     distort images according to given method ad args",
      "-draw string         annotate the image with a graphic primitive",
      "-edge radius         apply a filter to detect edges in the image",
      "-encipher filename   convert plain pixels to cipher pixels",
      "-emboss radius       emboss an image",
      "-enhance             apply a digital filter to enhance a noisy image",
      "-equalize            perform histogram equalization to an image",
      "-evaluate operator value",
      "                     evaluate an arithmetic, relational, or logical expression",
      "-extent geometry     set the image size",
      "-extract geometry    extract area from image",
      "-fft                 implements the discrete Fourier transform (DFT)",
      "-flip                flip image vertically",
      "-floodfill geometry color",
      "                     floodfill the image with color",
      "-flop                flop image horizontally",
      "-frame geometry      surround image with an ornamental border",
      "-function name parameters",
      "                     apply function over image values",
      "-gamma value         level of gamma correction",
      "-gaussian-blur geometry",
      "                     reduce image noise and reduce detail levels",
      "-geometry geometry   preferred size or location of the image",
      "-identify            identify the format and characteristics of the image",
      "-ift                 implements the inverse discrete Fourier transform (DFT)",
      "-implode amount      implode image pixels about the center",
      "-lat geometry        local adaptive thresholding",
      "-layers method       optimize, merge,  or compare image layers",
      "-level value         adjust the level of image contrast",
      "-level-colors color,color",
      "                     level image with the given colors",
      "-linear-stretch geometry",
      "                     improve contrast by `stretching with saturation'",
      "-liquid-rescale geometry",
      "                     rescale image with seam-carving",
      "-median radius       apply a median filter to the image",
      "-modulate value      vary the brightness, saturation, and hue",
      "-monochrome          transform image to black and white",
      "-morphology method kernel",
      "                     apply a morphology method to the image",
      "-motion-blur geometry",
      "                     simulate motion blur",
      "-negate              replace every pixel with its complementary color ",
      "-noise radius        add or reduce noise in an image",
      "-normalize           transform image to span the full range of colors",
      "-opaque color        change this color to the fill color",
      "-ordered-dither NxN",
      "                     add a noise pattern to the image with specific",
      "                     amplitudes",
      "-paint radius        simulate an oil painting",
      "-polaroid angle      simulate a Polaroid picture",
      "-posterize levels    reduce the image to a limited number of color levels",
      "-print string        interpret string and print to console",
      "-profile filename    add, delete, or apply an image profile",
      "-quantize colorspace reduce colors in this colorspace",
      "-radial-blur angle   radial blur the image",
      "-raise value         lighten/darken image edges to create a 3-D effect",
      "-random-threshold low,high",
      "                     random threshold the image",
      "-recolor matrix      translate, scale, shear, or rotate image colors",
      "-region geometry     apply options to a portion of the image",
      "-render              render vector graphics",
      "-repage geometry     size and location of an image canvas",
      "-resample geometry   change the resolution of an image",
      "-resize geometry     resize the image",
      "-roll geometry       roll an image vertically or horizontally",
      "-rotate degrees      apply Paeth rotation to the image",
      "-sample geometry     scale image with pixel sampling",
      "-scale geometry      scale the image",
      "-segment values      segment an image",
      "-selective-blur geometry",
      "                     selectively blur pixels within a contrast threshold",
      "-sepia-tone threshold",
      "                     simulate a sepia-toned photo",
      "-set property value  set an image property",
      "-shade degrees       shade the image using a distant light source",
      "-shadow geometry     simulate an image shadow",
      "-sharpen geometry    sharpen the image",
      "-shave geometry      shave pixels from the image edges",
      "-shear geometry      slide one edge of the image along the X or Y axis",
      "-sigmoidal-contrast geometry",
      "                     increase the contrast without saturating highlights or shadows",
      "-sketch geometry     simulate a pencil sketch",
      "-solarize threshold  negate all pixels above the threshold level",
      "-sparse-color method args",
      "                     fill in a image based on a few color points",
      "-splice geometry     splice the background color into the image",
      "-spread radius       displace image pixels by a random amount",
      "-strip               strip image of all profiles and comments",
      "-swirl degrees       swirl image pixels about the center",
      "-threshold value     threshold the image",
      "-thumbnail geometry  create a thumbnail of the image",
      "-tile filename       tile image when filling a graphic primitive",
      "-tint value          tint the image with the fill color",
      "-transform           affine transform image",
      "-transparent color   make this color transparent within the image",
      "-transpose           flip image vertically and rotate 90 degrees",
      "-transverse          flop image horizontally and rotate 270 degrees",
      "-trim                trim image edges",
      "-type type           image type",
      "-unique-colors       discard all but one of any pixel color",
      "-unsharp geometry    sharpen the image",
      "-vignette geometry   soften the edges of the image in vignette style",
      "-wave geometry       alter an image along a sine wave",
      "-white-threshold value",
      "                     force all pixels above the threshold into white",
      (char *) NULL
    },
    *sequence_operators[]=
    {
      "-append              append an image sequence",
      "-clut                apply a color lookup table to the image",
      "-coalesce            merge a sequence of images",
      "-combine             combine a sequence of images",
      "-composite           composite image",
      "-crop geometry       cut out a rectangular region of the image",
      "-deconstruct         break down an image sequence into constituent parts",
      "-evaluate-sequence operator",
      "                     evaluate an arithmetic, relational, or logical expression",
      "-flatten             flatten a sequence of images",
      "-fx expression       apply mathematical expression to an image channel(s)",
      "-hald-clut           apply a Hald color lookup table to the image",
      "-morph value         morph an image sequence",
      "-mosaic              create a mosaic from an image sequence",
      "-process arguments   process the image with a custom image filter",
      "-reverse             reverse image sequence",
      "-separate            separate an image channel into a grayscale image",
      "-write filename      write images to this file",
      (char *) NULL
    },
    *settings[]=
    {
      "-adjoin              join images into a single multi-image file",
      "-affine matrix       affine transform matrix",
      "-alpha option        activate, deactivate, reset, or set the alpha channel",
      "-antialias           remove pixel-aliasing",
      "-authenticate password",
      "                     decipher image with this password",
      "-attenuate value     lessen (or intensify) when adding noise to an image",
      "-background color    background color",
      "-bias value          add bias when convolving an image",
      "-black-point-compensation",
      "                     use black point compensation",
      "-blue-primary point  chromaticity blue primary point",
      "-bordercolor color   border color",
      "-caption string      assign a caption to an image",
      "-channel type        apply option to select image channels",
      "-colors value        preferred number of colors in the image",
      "-colorspace type     alternate image colorspace",
      "-comment string      annotate image with comment",
      "-compose operator    set image composite operator",
      "-compress type       type of pixel compression when writing the image",
      "-define format:option",
      "                     define one or more image format options",
      "-delay value         display the next image after pausing",
      "-density geometry    horizontal and vertical density of the image",
      "-depth value         image depth",
      "-display server      get image or font from this X server",
      "-dispose method      layer disposal method",
      "-dither method       apply error diffusion to image",
      "-encoding type       text encoding type",
      "-endian type         endianness (MSB or LSB) of the image",
      "-family name         render text with this font family",
      "-fill color          color to use when filling a graphic primitive",
      "-filter type         use this filter when resizing an image",
      "-font name           render text with this font",
      "-format \"string\"     output formatted image characteristics",
      "-fuzz distance       colors within this distance are considered equal",
      "-gravity type        horizontal and vertical text placement",
      "-green-primary point chromaticity green primary point",
      "-intent type         type of rendering intent when managing the image color",
      "-interlace type      type of image interlacing scheme",
      "-interline-spacing value",
      "                     set the space between two text lines",
      "-interpolate method  pixel color interpolation method",
      "-interword-spacing value",
      "                     set the space between two words",
      "-kerning value       set the space between two letters",
      "-label string        assign a label to an image",
      "-limit type value    pixel cache resource limit",
      "-loop iterations     add Netscape loop extension to your GIF animation",
      "-mask filename       associate a mask with the image",
      "-mattecolor color    frame color",
      "-monitor             monitor progress",
      "-orient type         image orientation",
      "-page geometry       size and location of an image canvas (setting)",
      "-ping                efficiently determine image attributes",
      "-pointsize value     font point size",
      "-precision value     maximum number of significant digits to print",
      "-preview type        image preview type",
      "-quality value       JPEG/MIFF/PNG compression level",
      "-quiet               suppress all warning messages",
      "-red-primary point   chromaticity red primary point",
      "-regard-warnings     pay attention to warning messages",
      "-remap filename      transform image colors to match this set of colors",
      "-respect-parentheses settings remain in effect until parenthesis boundary",
      "-sampling-factor geometry",
      "                     horizontal and vertical sampling factor",
      "-scene value         image scene number",
      "-seed value          seed a new sequence of pseudo-random numbers",
      "-size geometry       width and height of image",
      "-stretch type        render text with this font stretch",
      "-stroke color        graphic primitive stroke color",
      "-strokewidth value   graphic primitive stroke width",
      "-style type          render text with this font style",
      "-taint               image as ineligible for bi-modal delegate",
      "-texture filename    name of texture to tile onto the image background",
      "-tile-offset geometry",
      "                     tile offset",
      "-treedepth value     color tree depth",
      "-transparent-color color",
      "                     transparent color",
      "-undercolor color    annotation bounding box color",
      "-units type          the units of image resolution",
      "-verbose             print detailed information about the image",
      "-view                FlashPix viewing transforms",
      "-virtual-pixel method",
      "                     virtual pixel access method",
      "-weight type         render text with this font weight",
      "-white-point point   chromaticity white point",
      (char *) NULL
    },
    *stack_operators[]=
    {
      "-clone index         clone an image",
      "-delete index        delete the image from the image sequence",
      "-insert index        insert last image into the image sequence",
      "-swap indexes        swap two images in the image sequence",
      (char *) NULL
    };

  const char
    **p;

  (void) printf("Version: %s\n",GetMagickVersion((unsigned long *) NULL));
  (void) printf("Copyright: %s\n",GetMagickCopyright());
  (void) printf("Features: %s\n\n",GetMagickFeatures());
  (void) printf("Usage: %s [options ...] file [ [options ...] file ...]\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  for (p=settings; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nImage Operators:\n");
  for (p=operators; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nImage Sequence Operators:\n");
  for (p=sequence_operators; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nImage Stack Operators:\n");
  for (p=stack_operators; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nMiscellaneous Options:\n");
  for (p=miscellaneous; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf(
    "\nBy default, the image format of `file' is determined by its magic\n");
  (void) printf(
    "number.  To specify a particular image format, precede the filename\n");
  (void) printf(
    "with an image format name and a colon (i.e. ps:image) or specify the\n");
  (void) printf(
    "image type as the filename suffix (i.e. image.ps).  Specify 'file' as\n");
  (void) printf("'-' for standard input or output.\n");
  return(MagickFalse);
}

WandExport MagickBooleanType MogrifyImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **wand_unused(metadata),ExceptionInfo *exception)
{
#define DestroyMogrify() \
{ \
  if (format != (char *) NULL) \
    format=DestroyString(format); \
  if (path != (char *) NULL) \
    path=DestroyString(path); \
  DestroyImageStack(); \
  for (i=0; i < (long) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowMogrifyException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  DestroyMogrify(); \
  return(MagickFalse); \
}
#define ThrowMogrifyInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","`%s': %s",argument,option); \
  DestroyMogrify(); \
  return(MagickFalse); \
}

  char
    *format,
    *option,
    *path;

  Image
    *image;

  ImageStack
    image_stack[MaxImageStackDepth+1];

  long
    j,
    k;

  register long
    i;

  MagickBooleanType
    global_colormap;

  MagickBooleanType
    fire,
    pend;

  MagickStatusType
    status;

  /*
    Set defaults.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(exception != (ExceptionInfo *) NULL);
  if (argc == 2)
    {
      option=argv[1];
      if ((LocaleCompare("version",option+1) == 0) ||
          (LocaleCompare("-version",option+1) == 0))
        {
          (void) fprintf(stdout,"Version: %s\n",
            GetMagickVersion((unsigned long *) NULL));
          (void) fprintf(stdout,"Copyright: %s\n",GetMagickCopyright());
          (void) fprintf(stdout,"Features: %s\n\n",GetMagickFeatures());
          return(MagickFalse);
        }
    }
  if (argc < 2)
    return(MogrifyUsage());
  format=(char *) NULL;
  path=(char *) NULL;
  global_colormap=MagickFalse;
  k=0;
  j=1;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  status=MagickTrue;
  /*
    Parse command line.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowMogrifyException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=1; i < (long) argc; i++)
  {
    option=argv[i];
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowMogrifyException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowMogrifyException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsMagickOption(option) == MagickFalse)
      {
        char
          backup_filename[MaxTextExtent],
          *filename;

        Image
          *images;

        /*
          Option is a file name: begin by reading image from specified file.
        */
        FireImageStack(MagickFalse,MagickFalse,pend);
        filename=argv[i];
        if ((LocaleCompare(filename,"--") == 0) && (i < (argc-1)))
          filename=argv[++i];
        (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
        images=ReadImages(image_info,exception);
        status&=(images != (Image *) NULL) &&
          (exception->severity < ErrorException);
        if (images == (Image *) NULL)
          continue;
        if (path != (char *) NULL)
          {
            GetPathComponent(option,TailPath,filename);
            (void) FormatMagickString(images->filename,MaxTextExtent,"%s%c%s",
              path,*DirectorySeparator,filename);
          }
        if (format != (char *) NULL)
          AppendImageFormat(format,images->filename);
        AppendImageStack(images);
        FinalizeImageSettings(image_info,image,MagickFalse);
        if (global_colormap != MagickFalse)
          {
            QuantizeInfo
              *quantize_info;

            quantize_info=AcquireQuantizeInfo(image_info);
            (void) RemapImages(quantize_info,images,(Image *) NULL);
            quantize_info=DestroyQuantizeInfo(quantize_info);
          }
        *backup_filename='\0';
        if ((LocaleCompare(image->filename,"-") != 0) &&
            (IsPathWritable(image->filename) != MagickFalse))
          {
            register long
              i;

            /*
              Rename image file as backup.
            */
            (void) CopyMagickString(backup_filename,image->filename,
              MaxTextExtent);
            for (i=0; i < 6; i++)
            {
              (void) ConcatenateMagickString(backup_filename,"~",MaxTextExtent);
              if (IsPathAccessible(backup_filename) == MagickFalse)
                break;
            }
            if ((IsPathAccessible(backup_filename) != MagickFalse) ||
                (rename(image->filename,backup_filename) != 0))
              *backup_filename='\0';
          }
        /*
          Write transmogrified image to disk.
        */
        image_info->synchronize=MagickTrue;
        status&=WriteImages(image_info,image,image->filename,exception);
        if ((status == MagickFalse) && (*backup_filename != '\0'))
          (void) remove(backup_filename);
        RemoveAllImageStack();
        continue;
      }
    pend=image != (Image *) NULL ? MagickTrue : MagickFalse;
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("adaptive-blur",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-resize",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("affine",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("alpha",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickAlphaOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedAlphaChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("annotate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            i++;
            break;
          }
        if (LocaleCompare("antialias",option+1) == 0)
          break;
        if (LocaleCompare("append",option+1) == 0)
          break;
        if (LocaleCompare("attenuate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("auto-gamma",option+1) == 0)
          break;
        if (LocaleCompare("auto-level",option+1) == 0)
          break;
        if (LocaleCompare("auto-orient",option+1) == 0)
          break;
        if (LocaleCompare("average",option+1) == 0)
          break;
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'b':
      {
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("bias",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("black-point-compensation",option+1) == 0)
          break;
        if (LocaleCompare("black-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blue-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blue-shift",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("box",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("brightness-contrast",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("caption",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            long
              channel;

            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("cdl",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("charcoal",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("chop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("clamp",option+1) == 0)
          break;
        if (LocaleCompare("clip",option+1) == 0)
          break;
        if (LocaleCompare("clip-mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("clut",option+1) == 0)
          break;
        if (LocaleCompare("coalesce",option+1) == 0)
          break;
        if (LocaleCompare("colorize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("colorspace",option+1) == 0)
          {
            long
              colorspace;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            colorspace=ParseMagickOption(MagickColorspaceOptions,MagickFalse,
              argv[i]);
            if (colorspace < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("combine",option+1) == 0)
          break;
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("composite",option+1) == 0)
          break;
        if (LocaleCompare("compress",option+1) == 0)
          {
            long
              compress;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            compress=ParseMagickOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedImageCompression",
                argv[i]);
            break;
          }
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        if (LocaleCompare("contrast",option+1) == 0)
          break;
        if (LocaleCompare("contrast-stretch",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("convolve",option+1) == 0)
          {
            char
              token[MaxTextExtent];

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
#if 1
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
#else
            /* Allow the use of built-in kernels like 'gaussian'
             * These may not work for kernels with 'nan' values, like 'diamond'
             */
            GetMagickToken(argv[i],NULL,token);
            if ( isalpha((int)token[0]) )
              {
                long
                op;

                op=ParseMagickOption(MagickKernelOptions,MagickFalse,token);
                if (op < 0)
                  ThrowMogrifyException(OptionError,"UnrecognizedKernelType",
                       token);
              }
            /* geometry current returns invalid if 'nan' values are used */
            else if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
#endif
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("cycle",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'd':
      {
        if (LocaleCompare("decipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("deconstruct",option+1) == 0)
          break;
        if (LocaleCompare("debug",option+1) == 0)
          {
            long
              event;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            event=ParseMagickOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowMogrifyException(OptionError,"NoSuchOption",argv[i]);
                break;
              }
            break;
          }
        if (LocaleCompare("delay",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("deskew",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("despeckle",option+1) == 0)
          break;
        if (LocaleCompare("dft",option+1) == 0)
          break;
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("dispose",option+1) == 0)
          {
            long
              dispose;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            dispose=ParseMagickOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("distort",option+1) == 0)
          {
            long
              op;

            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseMagickOption(MagickDistortOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedDistortMethod",
                argv[i]);
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            long
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            method=ParseMagickOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("draw",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'e':
      {
        if (LocaleCompare("edge",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("emboss",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("encipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("endian",option+1) == 0)
          {
            long
              endian;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            endian=ParseMagickOption(MagickEndianOptions,MagickFalse,argv[i]);
            if (endian < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedEndianType",
                argv[i]);
            break;
          }
        if (LocaleCompare("enhance",option+1) == 0)
          break;
        if (LocaleCompare("equalize",option+1) == 0)
          break;
        if (LocaleCompare("evaluate",option+1) == 0)
          {
            long
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseMagickOption(MagickEvaluateOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedEvaluateOperator",
                argv[i]);
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("evaluate-sequence",option+1) == 0)
          {
            long
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseMagickOption(MagickEvaluateOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedEvaluateOperator",
                argv[i]);
            break;
          }
        if (LocaleCompare("extent",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("extract",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'f':
      {
        if (LocaleCompare("family",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("fill",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("filter",option+1) == 0)
          {
            long
              filter;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            filter=ParseMagickOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedImageFilter",
                argv[i]);
            break;
          }
        if (LocaleCompare("flatten",option+1) == 0)
          break;
        if (LocaleCompare("flip",option+1) == 0)
          break;
        if (LocaleCompare("flop",option+1) == 0)
          break;
        if (LocaleCompare("floodfill",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MaxTextExtent);
            (void) CloneString(&format,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            (void) CloneString(&format,argv[i]);
            (void) CopyMagickString(image_info->filename,format,MaxTextExtent);
            (void) ConcatenateMagickString(image_info->filename,":",
              MaxTextExtent);
            (void) SetImageInfo(image_info,0,exception);
            if (*image_info->magick == '\0')
              ThrowMogrifyException(OptionError,"UnrecognizedImageFormat",
                format);
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("function",option+1) == 0)
          {
            long
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseMagickOption(MagickFunctionOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedFunction",argv[i]);
             i++;
             if (i == (long) (argc-1))
               ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("fuzz",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("fx",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'g':
      {
        if (LocaleCompare("gamma",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if ((LocaleCompare("gaussian-blur",option+1) == 0) ||
            (LocaleCompare("gaussian",option+1) == 0))
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            long
              gravity;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            gravity=ParseMagickOption(MagickGravityOptions,MagickFalse,argv[i]);
            if (gravity < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if (LocaleCompare("hald-clut",option+1) == 0)
          break;
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          return(MogrifyUsage());
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'i':
      {
        if (LocaleCompare("identify",option+1) == 0)
          break;
        if (LocaleCompare("idft",option+1) == 0)
          break;
        if (LocaleCompare("implode",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("intent",option+1) == 0)
          {
            long
              intent;

            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            intent=ParseMagickOption(MagickIntentOptions,MagickFalse,argv[i]);
            if (intent < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedIntentType",
                argv[i]);
            break;
          }
        if (LocaleCompare("interlace",option+1) == 0)
          {
            long
              interlace;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            interlace=ParseMagickOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedInterlaceType",
                argv[i]);
            break;
          }
        if (LocaleCompare("interline-spacing",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("interpolate",option+1) == 0)
          {
            long
              interpolate;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            interpolate=ParseMagickOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("interword-spacing",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'k':
      {
        if (LocaleCompare("kerning",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("lat",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
          }
        if (LocaleCompare("layers",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickLayerOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedLayerMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("level",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("level-colors",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("linewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("limit",option+1) == 0)
          {
            char
              *p;

            double
              value;

            long
              resource;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            resource=ParseMagickOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            value=strtod(argv[i],&p);
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("liquid-rescale",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("list",option+1) == 0)
          {
            long
              list;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            list=ParseMagickOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedListType",argv[i]);
            (void) MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            return(MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (long) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("loop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("map",option+1) == 0)
          {
            global_colormap=(*option == '+') ? MagickTrue : MagickFalse;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("matte",option+1) == 0)
          break;
        if (LocaleCompare("mattecolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("maximum",option+1) == 0)
          break;
        if (LocaleCompare("minimum",option+1) == 0)
          break;
        if (LocaleCompare("modulate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("median",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          break;
        if (LocaleCompare("monochrome",option+1) == 0)
          break;
        if (LocaleCompare("morph",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("morphology",option+1) == 0)
          {
            long
              op;

            char
              token[MaxTextExtent];

            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            GetMagickToken(argv[i],NULL,token);
            op=ParseMagickOption(MagickMorphologyOptions,MagickFalse,token);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedMorphologyMethod",
                   token);
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            GetMagickToken(argv[i],NULL,token);
            if ( isalpha((int)token[0]) )
              {
                op=ParseMagickOption(MagickKernelOptions,MagickFalse,token);
                if (op < 0)
                  ThrowMogrifyException(OptionError,"UnrecognizedKernelType",
                       token);
              }
#if 0
  /* DO NOT ENABLE, geometry can not handle user defined kernels
   * which include 'nan' values, though '-' are acceptable.
   */
            else if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
#endif
            break;
          }
        if (LocaleCompare("mosaic",option+1) == 0)
          break;
        if (LocaleCompare("motion-blur",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'n':
      {
        if (LocaleCompare("negate",option+1) == 0)
          break;
        if (LocaleCompare("noise",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                long
                  noise;

                noise=ParseMagickOption(MagickNoiseOptions,MagickFalse,argv[i]);
                if (noise < 0)
                  ThrowMogrifyException(OptionError,"UnrecognizedNoiseType",
                    argv[i]);
                break;
              }
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("noop",option+1) == 0)
          break;
        if (LocaleCompare("normalize",option+1) == 0)
          break;
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'o':
      {
        if (LocaleCompare("opaque",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("ordered-dither",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("orient",option+1) == 0)
          {
            long
              orientation;

            orientation=UndefinedOrientation;
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            orientation=ParseMagickOption(MagickOrientationOptions,MagickFalse,
              argv[i]);
            if (orientation < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedImageOrientation",
                argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("paint",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("path",option+1) == 0)
          {
            (void) CloneString(&path,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            (void) CloneString(&path,argv[i]);
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("polaroid",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("posterize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("precision",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("print",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("quantize",option+1) == 0)
          {
            long
              colorspace;

            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            colorspace=ParseMagickOption(MagickColorspaceOptions,MagickFalse,
              argv[i]);
            if (colorspace < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("radial-blur",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("raise",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("random-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("red-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
          }
        if (LocaleCompare("region",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("render",option+1) == 0)
          break;
        if (LocaleCompare("repage",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("reverse",option+1) == 0)
          break;
        if (LocaleCompare("roll",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 's':
      {
        if (LocaleCompare("sample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scale",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scene",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("segment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("selective-blur",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("separate",option+1) == 0)
          break;
        if (LocaleCompare("sepia-tone",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("shade",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shadow",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shave",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shear",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
          {
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sketch",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("solarize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sparse-color",option+1) == 0)
          {
            long
              op;

            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseMagickOption(MagickSparseColorOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedSparseColorMethod",
                argv[i]);
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("spread",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("stretch",option+1) == 0)
          {
            long
              stretch;

            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            stretch=ParseMagickOption(MagickStretchOptions,MagickFalse,argv[i]);
            if (stretch < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedStyleType",
                argv[i]);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          break;
        if (LocaleCompare("stroke",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("style",option+1) == 0)
          {
            long
              style;

            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            style=ParseMagickOption(MagickStyleOptions,MagickFalse,argv[i]);
            if (style < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedStyleType",
                argv[i]);
            break;
          }
        if (LocaleCompare("swirl",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 't':
      {
        if (LocaleCompare("taint",option+1) == 0)
          break;
        if (LocaleCompare("texture",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("tile-offset",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tint",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          break;
        if (LocaleCompare("transpose",option+1) == 0)
          break;
        if (LocaleCompare("transverse",option+1) == 0)
          break;
        if (LocaleCompare("threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("transparent",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("trim",option+1) == 0)
          break;
        if (LocaleCompare("type",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickTypeOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedImageType",
                argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'u':
      {
        if (LocaleCompare("undercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("unique-colors",option+1) == 0)
          break;
        if (LocaleCompare("units",option+1) == 0)
          {
            long
              units;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            units=ParseMagickOption(MagickResolutionOptions,MagickFalse,
              argv[i]);
            if (units < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedUnitsType",
                argv[i]);
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'v':
      {
        if (LocaleCompare("verbose",option+1) == 0)
          {
            image_info->verbose=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if ((LocaleCompare("version",option+1) == 0) ||
            (LocaleCompare("-version",option+1) == 0))
          {
            (void) fprintf(stdout,"Version: %s\n",
              GetMagickVersion((unsigned long *) NULL));
            (void) fprintf(stdout,"Copyright: %s\n",GetMagickCopyright());
            (void) fprintf(stdout,"Features: %s\n\n",GetMagickFeatures());
            break;
          }
        if (LocaleCompare("view",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("vignette",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("virtual-pixel",option+1) == 0)
          {
            long
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            method=ParseMagickOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowMogrifyException(OptionError,
                "UnrecognizedVirtualPixelMethod",argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'w':
      {
        if (LocaleCompare("wave",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("weight",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("white-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            i++;
            if (i == (long) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case '?':
        break;
      default:
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
    }
    fire=ParseMagickOption(MagickImageListOptions,MagickFalse,option+1) < 0 ?
      MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowMogrifyException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i != argc)
    ThrowMogrifyException(OptionError,"MissingAnImageFilename",argv[i]);
  DestroyMogrify();
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     M o g r i f y I m a g e I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MogrifyImageInfo() applies image processing settings to the image as
%  prescribed by command line options.
%
%  The format of the MogrifyImageInfo method is:
%
%      MagickBooleanType MogrifyImageInfo(ImageInfo *image_info,const int argc,
%        const char **argv,ExceptionInfo *exception)
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
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType MogrifyImageInfo(ImageInfo *image_info,
  const int argc,const char **argv,ExceptionInfo *exception)
{
  const char
    *option;

  GeometryInfo
    geometry_info;

  long
    count;

  register long
    i;

  /*
    Initialize method variables.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (argc < 0)
    return(MagickTrue);
  /*
    Set the image settings.
  */
  for (i=0; i < (long) argc; i++)
  {
    option=argv[i];
    if (IsMagickOption(option) == MagickFalse)
      continue;
    count=MagickMax(ParseMagickOption(MagickCommandOptions,MagickFalse,option),
      0L);
    if ((i+count) >= argc)
      break;
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("adjoin",option+1) == 0)
          {
            image_info->adjoin=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("antialias",option+1) == 0)
          {
            image_info->antialias=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("attenuate",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              (void) CloneString(&image_info->authenticate,(char *) NULL);
            else
              (void) CloneString(&image_info->authenticate,argv[i+1]);
            break;
          }
        break;
      }
      case 'b':
      {
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                (void) QueryColorDatabase(BackgroundColor,
                  &image_info->background_color,exception);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            (void) QueryColorDatabase(argv[i+1],&image_info->background_color,
              exception);
            break;
          }
        if (LocaleCompare("bias",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0.0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("black-point-compensation",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"false");
                break;
              }
            (void) SetImageOption(image_info,option+1,"true");
            break;
          }
        if (LocaleCompare("blue-primary",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0.0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                (void) QueryColorDatabase(BorderColor,&image_info->border_color,
                  exception);
                break;
              }
            (void) QueryColorDatabase(argv[i+1],&image_info->border_color,
              exception);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("box",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,"undercolor","none");
                break;
              }
            (void) SetImageOption(image_info,"undercolor",argv[i+1]);
            break;
          }
        break;
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            MagickSizeType
              limit;

            limit=MagickResourceInfinity;
            if (LocaleCompare("unlimited",argv[i+1]) != 0)
              limit=(MagickSizeType) SiPrefixToDouble(argv[i+1],100.0);
            (void) SetMagickResourceLimit(MemoryResource,limit);
            (void) SetMagickResourceLimit(MapResource,2*limit);
            break;
          }
        if (LocaleCompare("caption",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->channel=DefaultChannels;
                break;
              }
            image_info->channel=(ChannelType) ParseChannelOption(argv[i+1]);
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            image_info->colors=StringToUnsignedLong(argv[i+1]);
            break;
          }
        if (LocaleCompare("colorspace",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->colorspace=UndefinedColorspace;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->colorspace=(ColorspaceType) ParseMagickOption(
              MagickColorspaceOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("compress",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->compression=UndefinedCompression;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->compression=(CompressionType) ParseMagickOption(
              MagickCompressOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("compose",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("compress",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->compression=UndefinedCompression;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->compression=(CompressionType) ParseMagickOption(
              MagickCompressOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'd':
      {
        if (LocaleCompare("debug",option+1) == 0)
          {
            if (*option == '+')
              (void) SetLogEventMask("none");
            else
              (void) SetLogEventMask(argv[i+1]);
            image_info->debug=IsEventLogging();
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            if (*option == '+')
              {
                if (LocaleNCompare(argv[i+1],"registry:",9) == 0)
                  (void) DeleteImageRegistry(argv[i+1]+9);
                else
                  (void) DeleteImageOption(image_info,argv[i+1]);
                break;
              }
            if (LocaleNCompare(argv[i+1],"registry:",9) == 0)
              {
                (void) DefineImageRegistry(StringRegistryType,argv[i+1]+9,
                  exception);
                break;
              }
            (void) DefineImageOption(image_info,argv[i+1]);
            break;
          }
        if (LocaleCompare("delay",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            /*
              Set image density.
            */
            if (*option == '+')
              {
                if (image_info->density != (char *) NULL)
                  image_info->density=DestroyString(image_info->density);
                (void) SetImageOption(image_info,option+1,"72");
                break;
              }
            (void) CloneString(&image_info->density,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->depth=MAGICKCORE_QUANTUM_DEPTH;
                break;
              }
            image_info->depth=StringToUnsignedLong(argv[i+1]);
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              {
                if (image_info->server_name != (char *) NULL)
                  image_info->server_name=DestroyString(
                    image_info->server_name);
                break;
              }
            (void) CloneString(&image_info->server_name,argv[i+1]);
            break;
          }
        if (LocaleCompare("dispose",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->dither=MagickFalse;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            image_info->dither=MagickTrue;
            break;
          }
        break;
      }
      case 'e':
      {
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("endian",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->endian=UndefinedEndian;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->endian=(EndianType) ParseMagickOption(
              MagickEndianOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("extract",option+1) == 0)
          {
            /*
              Set image extract geometry.
            */
            if (*option == '+')
              {
                if (image_info->extract != (char *) NULL)
                  image_info->extract=DestroyString(image_info->extract);
                break;
              }
            (void) CloneString(&image_info->extract,argv[i+1]);
            break;
          }
        break;
      }
      case 'f':
      {
        if (LocaleCompare("fill",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"none");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("filter",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              {
                if (image_info->font != (char *) NULL)
                  image_info->font=DestroyString(image_info->font);
                break;
              }
            (void) CloneString(&image_info->font,argv[i+1]);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            register const char
              *q;

            for (q=strchr(argv[i+1],'%'); q != (char *) NULL; q=strchr(q+1,'%'))
              if (strchr("gkrz@[#",*(q+1)) != (char *) NULL)
                image_info->ping=MagickFalse;
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("fuzz",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->fuzz=0.0;
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            image_info->fuzz=SiPrefixToDouble(argv[i+1],(double) QuantumRange+
              1.0);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'g':
      {
        if (LocaleCompare("gravity",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0.0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'i':
      {
        if (LocaleCompare("intent",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("interlace",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->interlace=UndefinedInterlace;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->interlace=(InterlaceType) ParseMagickOption(
              MagickInterlaceOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("interline-spacing",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("interpolate",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("interword-spacing",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'k':
      {
        if (LocaleCompare("kerning",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("limit",option+1) == 0)
          {
            MagickSizeType
              limit;

            ResourceType
              type;

            if (*option == '+')
              break;
            type=(ResourceType) ParseMagickOption(MagickResourceOptions,
              MagickFalse,argv[i+1]);
            limit=MagickResourceInfinity;
            if (LocaleCompare("unlimited",argv[i+2]) != 0)
              limit=(MagickSizeType) SiPrefixToDouble(argv[i+2],100.0);
            (void) SetMagickResourceLimit(type,limit);
            break;
          }
        if (LocaleCompare("list",option+1) == 0)
          {
            long
              list;

            /*
              Display configuration list.
            */
            list=ParseMagickOption(MagickListOptions,MagickFalse,argv[i+1]);
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
                (void) ListMagickOptions((FILE *) NULL,(MagickOption) list,
                  exception);
                break;
              }
            }
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            (void) SetLogFormat(argv[i+1]);
            break;
          }
        if (LocaleCompare("loop",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'm':
      {
        if (LocaleCompare("matte",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"false");
                break;
              }
            (void) SetImageOption(image_info,option+1,"true");
            break;
          }
        if (LocaleCompare("mattecolor",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,argv[i+1]);
                (void) QueryColorDatabase(MatteColor,&image_info->matte_color,
                  exception);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            (void) QueryColorDatabase(argv[i+1],&image_info->matte_color,
              exception);
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          {
            (void) SetImageInfoProgressMonitor(image_info,MonitorProgress,
              (void *) NULL);
            break;
          }
        if (LocaleCompare("monochrome",option+1) == 0)
          {
            image_info->monochrome=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        break;
      }
      case 'o':
      {
        if (LocaleCompare("orient",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->orientation=UndefinedOrientation;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->orientation=(OrientationType) ParseMagickOption(
              MagickOrientationOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,"undefined");
            break;
          }
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
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

            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                (void) CloneString(&image_info->page,(char *) NULL);
                break;
              }
            (void) ResetMagickMemory(&geometry,0,sizeof(geometry));
            image_option=GetImageOption(image_info,"page");
            if (image_option != (const char *) NULL)
              flags=ParseAbsoluteGeometry(image_option,&geometry);
            canonical_page=GetPageGeometry(argv[i+1]);
            flags=ParseAbsoluteGeometry(canonical_page,&geometry);
            canonical_page=DestroyString(canonical_page);
            (void) FormatMagickString(page,MaxTextExtent,"%lux%lu",
              geometry.width,geometry.height);
            if (((flags & XValue) != 0) || ((flags & YValue) != 0))
              (void) FormatMagickString(page,MaxTextExtent,"%lux%lu%+ld%+ld",
                geometry.width,geometry.height,geometry.x,geometry.y);
            (void) SetImageOption(image_info,option+1,page);
            (void) CloneString(&image_info->page,page);
            break;
          }
        if (LocaleCompare("pen",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"none");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("ping",option+1) == 0)
          {
            image_info->ping=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              geometry_info.rho=0.0;
            else
              (void) ParseGeometry(argv[i+1],&geometry_info);
            image_info->pointsize=geometry_info.rho;
            break;
          }
        if (LocaleCompare("precision",option+1) == 0)
          {
            (void) SetMagickPrecision(StringToInteger(argv[i+1]));
            break;
          }
        if (LocaleCompare("preview",option+1) == 0)
          {
            /*
              Preview image.
            */
            if (*option == '+')
              {
                image_info->preview_type=UndefinedPreview;
                break;
              }
            image_info->preview_type=(PreviewType) ParseMagickOption(
              MagickPreviewOptions,MagickFalse,argv[i+1]);
            break;
          }
        break;
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            /*
              Set image compression quality.
            */
            if (*option == '+')
              {
                image_info->quality=UndefinedCompressionQuality;
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            image_info->quality=StringToUnsignedLong(argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          {
            static WarningHandler
              warning_handler = (WarningHandler) NULL;

            if (*option == '+')
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
        if (LocaleCompare("red-primary",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0.0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 's':
      {
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            /*
              Set image sampling factor.
            */
            if (*option == '+')
              {
                if (image_info->sampling_factor != (char *) NULL)
                  image_info->sampling_factor=DestroyString(
                    image_info->sampling_factor);
                break;
              }
            (void) CloneString(&image_info->sampling_factor,argv[i+1]);
            break;
          }
        if (LocaleCompare("scene",option+1) == 0)
          {
            /*
              Set image scene.
            */
            if (*option == '+')
              {
                image_info->scene=0;
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            image_info->scene=StringToUnsignedLong(argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            unsigned long
              seed;

            if (*option == '+')
              {
                seed=(unsigned long) time((time_t *) NULL);
                SeedPseudoRandomGenerator(seed);
                break;
              }
            seed=StringToUnsignedLong(argv[i+1]);
            SeedPseudoRandomGenerator(seed);
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              {
                if (image_info->size != (char *) NULL)
                  image_info->size=DestroyString(image_info->size);
                break;
              }
            (void) CloneString(&image_info->size,argv[i+1]);
            break;
          }
        if (LocaleCompare("stroke",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"none");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 't':
      {
        if (LocaleCompare("taint",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"false");
                break;
              }
            (void) SetImageOption(image_info,option+1,"true");
            break;
          }
        if (LocaleCompare("texture",option+1) == 0)
          {
            if (*option == '+')
              {
                if (image_info->texture != (char *) NULL)
                  image_info->texture=DestroyString(image_info->texture);
                break;
              }
            (void) CloneString(&image_info->texture,argv[i+1]);
            break;
          }
        if (LocaleCompare("tile-offset",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) QueryColorDatabase("none",&image_info->transparent_color,                  exception);
                (void) SetImageOption(image_info,option+1,"none");
                break;
              }
            (void) QueryColorDatabase(argv[i+1],&image_info->transparent_color,
              exception);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("type",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->type=UndefinedType;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->type=(ImageType) ParseMagickOption(MagickTypeOptions,
              MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'u':
      {
        if (LocaleCompare("undercolor",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        if (LocaleCompare("units",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->units=UndefinedResolution;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->units=(ResolutionType) ParseMagickOption(
              MagickResolutionOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'v':
      {
        if (LocaleCompare("verbose",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->verbose=MagickFalse;
                break;
              }
            image_info->verbose=MagickTrue;
            image_info->ping=MagickFalse;
            break;
          }
        if (LocaleCompare("view",option+1) == 0)
          {
            if (*option == '+')
              {
                if (image_info->view != (char *) NULL)
                  image_info->view=DestroyString(image_info->view);
                break;
              }
            (void) CloneString(&image_info->view,argv[i+1]);
            break;
          }
        if (LocaleCompare("virtual-pixel",option+1) == 0)
          {
            if (*option == '+')
              {
                image_info->virtual_pixel_method=UndefinedVirtualPixelMethod;
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            image_info->virtual_pixel_method=(VirtualPixelMethod)
              ParseMagickOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      case 'w':
      {
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"0.0");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
        break;
      }
      default:
        break;
    }
    i+=count;
  }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     M o g r i f y I m a g e L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MogrifyImageList() applies any command line options that might affect the
%  entire image list (e.g. -append, -coalesce, etc.).
%
%  The format of the MogrifyImage method is:
%
%      MagickBooleanType MogrifyImageList(ImageInfo *image_info,const int argc,
%        const char **argv,Image **images,ExceptionInfo *exception)
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
%    o images: the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType MogrifyImageList(ImageInfo *image_info,
  const int argc,const char **argv,Image **images,ExceptionInfo *exception)
{
  ChannelType
    channel;

  const char
    *option;

  long
    count,
    index;

  MagickStatusType
    status;

  QuantizeInfo
    *quantize_info;

  register long
    i;

  /*
    Apply options to the image list.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(images != (Image **) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  if ((argc <= 0) || (*argv == (char *) NULL))
    return(MagickTrue);
  quantize_info=AcquireQuantizeInfo(image_info);
  channel=image_info->channel;
  status=MagickTrue;
  for (i=0; i < (long) argc; i++)
  {
    if (*images == (Image *) NULL)
      break;
    option=argv[i];
    if (IsMagickOption(option) == MagickFalse)
      continue;
    count=MagickMax(ParseMagickOption(MagickCommandOptions,MagickFalse,option),
      0L);
    if ((i+count) >= argc)
      break;
    status=MogrifyImageInfo(image_info,count+1,argv+i,exception);
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("affinity",option+1) == 0)
          {
            (void) SyncImagesSettings(image_info,*images);
            if (*option == '+')
              {
                (void) RemapImages(quantize_info,*images,(Image *) NULL);
                InheritException(exception,&(*images)->exception);
                break;
              }
            i++;
            break;
          }
        if (LocaleCompare("append",option+1) == 0)
          {
            Image
              *append_image;

            (void) SyncImagesSettings(image_info,*images);
            append_image=AppendImages(*images,*option == '-' ? MagickTrue :
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
        if (LocaleCompare("average",option+1) == 0)
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
        if (LocaleCompare("channel",option+1) == 0)
          {
            if (*option == '+')
              {
                channel=DefaultChannels;
                break;
              }
            channel=(ChannelType) ParseChannelOption(argv[i+1]);
            break;
          }
        if (LocaleCompare("clut",option+1) == 0)
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
            (void) ClutImageChannel(image,channel,clut_image);
            clut_image=DestroyImage(clut_image);
            InheritException(exception,&image->exception);
            *images=DestroyImageList(*images);
            *images=image;
            break;
          }
        if (LocaleCompare("coalesce",option+1) == 0)
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
        if (LocaleCompare("combine",option+1) == 0)
          {
            Image
              *combine_image;

            (void) SyncImagesSettings(image_info,*images);
            combine_image=CombineImages(*images,channel,exception);
            if (combine_image == (Image *) NULL)
              {
                status=MagickFalse;
                break;
              }
            *images=DestroyImageList(*images);
            *images=combine_image;
            break;
          }
        if (LocaleCompare("composite",option+1) == 0)
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
                    */
                    image->mask=mask_image;
                    (void) NegateImage(image->mask,MagickFalse);
                  }
              }
            (void) CompositeImageChannel(image,channel,image->compose,
              composite_image,geometry.x,geometry.y);
            if (image->mask != (Image *) NULL)
              image->mask=DestroyImage(image->mask);
            composite_image=DestroyImage(composite_image);
            InheritException(exception,&image->exception);
            *images=DestroyImageList(*images);
            *images=image;
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            MagickStatusType
              flags;

            RectangleInfo
              geometry;

            (void) SyncImagesSettings(image_info,*images);
            flags=ParseGravityGeometry(*images,argv[i+1],&geometry,exception);
            if (((geometry.width == 0) && (geometry.height == 0)) ||
                ((flags & XValue) != 0) || ((flags & YValue) != 0))
              break;
            (void) TransformImages(images,argv[i+1],(char *) NULL);
            InheritException(exception,&(*images)->exception);
            break;
          }
        break;
      }
      case 'd':
      {
        if (LocaleCompare("deconstruct",option+1) == 0)
          {
            Image
              *deconstruct_image;

            (void) SyncImagesSettings(image_info,*images);
            deconstruct_image=DeconstructImages(*images,exception);
            if (deconstruct_image == (Image *) NULL)
              {
                status=MagickFalse;
                break;
              }
            *images=DestroyImageList(*images);
            *images=deconstruct_image;
            break;
          }
        if (LocaleCompare("delete",option+1) == 0)
          {
            if (*option == '+')
              DeleteImages(images,"-1",exception);
            else
              DeleteImages(images,argv[i+1],exception);
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            if (*option == '+')
              {
                quantize_info->dither=MagickFalse;
                break;
              }
            quantize_info->dither=MagickTrue;
            quantize_info->dither_method=(DitherMethod) ParseMagickOption(
              MagickDitherOptions,MagickFalse,argv[i+1]);
            break;
          }
        break;
      }
      case 'e':
      {
        if (LocaleCompare("evaluate-sequence",option+1) == 0)
          {
            Image
              *evaluate_image;

            MagickEvaluateOperator
              op;

            (void) SyncImageSettings(image_info,*images);
            op=(MagickEvaluateOperator) ParseMagickOption(MagickEvaluateOptions,
              MagickFalse,argv[i+1]);
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
        if (LocaleCompare("fft",option+1) == 0)
          {
            Image
              *fourier_image;

            /*
              Implements the discrete Fourier transform (DFT).
            */
            (void) SyncImageSettings(image_info,*images);
            fourier_image=ForwardFourierTransformImage(*images,*option == '-' ?
              MagickTrue : MagickFalse,exception);
            if (fourier_image == (Image *) NULL)
              break;
            *images=DestroyImage(*images);
            *images=fourier_image;
            break;
          }
        if (LocaleCompare("flatten",option+1) == 0)
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
        if (LocaleCompare("fx",option+1) == 0)
          {
            Image
              *fx_image;

            (void) SyncImagesSettings(image_info,*images);
            fx_image=FxImageChannel(*images,channel,argv[i+1],exception);
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
        if (LocaleCompare("hald-clut",option+1) == 0)
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
            (void) HaldClutImageChannel(image,channel,hald_image);
            hald_image=DestroyImage(hald_image);
            InheritException(exception,&image->exception);
            if (*images != (Image *) NULL)
              *images=DestroyImageList(*images);
            *images=image;
            break;
          }
        break;
      }
      case 'i':
      {
        if (LocaleCompare("ift",option+1) == 0)
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
              phase_image,*option == '-' ? MagickTrue : MagickFalse,exception);
            if (fourier_image == (Image *) NULL)
              break;
            if (*images != (Image *) NULL)
              *images=DestroyImage(*images);
            *images=fourier_image;
            break;
          }
        if (LocaleCompare("insert",option+1) == 0)
          {
            Image
              *p,
              *q;

            index=0;
            if (*option != '+')
              index=StringToLong(argv[i+1]);
            p=RemoveLastImageFromList(images);
            if (p == (Image *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionError,"NoSuchImage","`%s'",argv[i+1]);
                status=MagickFalse;
                break;
              }
            q=p;
            if (index == 0)
              PrependImageToList(images,q);
            else
              if (index == (long) GetImageListLength(*images))
                AppendImageToList(images,q);
              else
                {
                   q=GetImageFromList(*images,index-1);
                   if (q == (Image *) NULL)
                     {
                       (void) ThrowMagickException(exception,GetMagickModule(),
                         OptionError,"NoSuchImage","`%s'",argv[i+1]);
                       status=MagickFalse;
                       break;
                     }
                  InsertImageInList(&q,p);
                }
            *images=GetFirstImageInList(q);
            break;
          }
        break;
      }
      case 'l':
      {
        if (LocaleCompare("layers",option+1) == 0)
          {
            Image
              *layers;

            ImageLayerMethod
              method;

            (void) SyncImagesSettings(image_info,*images);
            layers=(Image *) NULL;
            method=(ImageLayerMethod) ParseMagickOption(MagickLayerOptions,
              MagickFalse,argv[i+1]);
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
                layers=CompareImageLayers(*images,method,exception);
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
                InheritException(exception,&layers->exception);
                *images=DestroyImageList(*images);
                *images=layers;
                layers=OptimizeImageLayers(*images,exception);
                if (layers == (Image *) NULL)
                  {
                    status=MagickFalse;
                    break;
                  }
                InheritException(exception,&layers->exception);
                *images=DestroyImageList(*images);
                *images=layers;
                layers=(Image *) NULL;
                OptimizeImageTransparency(*images,exception);
                InheritException(exception,&(*images)->exception);
                (void) RemapImages(quantize_info,*images,(Image *) NULL);
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
                option=GetImageOption(image_info,"compose");
                if (option != (const char *) NULL)
                  compose=(CompositeOperator) ParseMagickOption(
                    MagickComposeOptions,MagickFalse,option);
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
        if (LocaleCompare("map",option+1) == 0)
          {
            (void) SyncImagesSettings(image_info,*images);
            if (*option == '+')
              {
                (void) RemapImages(quantize_info,*images,(Image *) NULL);
                InheritException(exception,&(*images)->exception);
                break;
              }
            i++;
            break;
          }
        if (LocaleCompare("maximum",option+1) == 0)
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
        if (LocaleCompare("minimum",option+1) == 0)
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
        if (LocaleCompare("morph",option+1) == 0)
          {
            Image
              *morph_image;

            (void) SyncImagesSettings(image_info,*images);
            morph_image=MorphImages(*images,StringToUnsignedLong(argv[i+1]),
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
        if (LocaleCompare("mosaic",option+1) == 0)
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
        if (LocaleCompare("print",option+1) == 0)
          {
            char
              *string;

            (void) SyncImagesSettings(image_info,*images);
            string=InterpretImageProperties(image_info,*images,argv[i+1]);
            if (string == (char *) NULL)
              break;
            InheritException(exception,&(*images)->exception);
            (void) fprintf(stdout,"%s",string);
            string=DestroyString(string);
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            char
              **arguments;

            int
              j,
              number_arguments;

            (void) SyncImagesSettings(image_info,*images);
            arguments=StringToArgv(argv[i+1],&number_arguments);
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
                length=strlen(argv[i+1]);
                token=(char *) NULL;
                if (~length >= MaxTextExtent)
                  token=(char *) AcquireQuantumMemory(length+MaxTextExtent,
                    sizeof(*token));
                if (token == (char *) NULL)
                  break;
                next=0;
                arguments=argv[i+1];
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
        if (LocaleCompare("reverse",option+1) == 0)
          {
            ReverseImageList(images);
            InheritException(exception,&(*images)->exception);
            break;
          }
        break;
      }
      case 's':
      {
        if (LocaleCompare("swap",option+1) == 0)
          {
            Image
              *p,
              *q,
              *swap;

            long
              swap_index;

            index=(-1);
            swap_index=(-2);
            if (*option != '+')
              {
                GeometryInfo
                  geometry_info;

                MagickStatusType
                  flags;

                swap_index=(-1);
                flags=ParseGeometry(argv[i+1],&geometry_info);
                index=(long) geometry_info.rho;
                if ((flags & SigmaValue) != 0)
                  swap_index=(long) geometry_info.sigma;
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
        if (LocaleCompare("write",option+1) == 0)
          {
            Image
              *write_images;

            ImageInfo
              *write_info;

            (void) SyncImagesSettings(image_info,*images);
            write_images=(*images);
            if (*option == '+')
              write_images=CloneImageList(*images,exception);
            write_info=CloneImageInfo(image_info);
            status&=WriteImages(write_info,write_images,argv[i+1],exception);
            write_info=DestroyImageInfo(write_info);
            if (*option == '+')
              write_images=DestroyImageList(write_images);
            break;
          }
        break;
      }
      default:
        break;
    }
    i+=count;
  }
  quantize_info=DestroyQuantizeInfo(quantize_info);
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     M o g r i f y I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MogrifyImages() applies image processing options to a sequence of images as
%  prescribed by command line options.
%
%  The format of the MogrifyImage method is:
%
%      MagickBooleanType MogrifyImages(ImageInfo *image_info,
%        const MagickBooleanType post,const int argc,const char **argv,
%        Image **images,Exceptioninfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o post: If true, post process image list operators otherwise pre-process.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o images: the images.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType MogrifyImages(ImageInfo *image_info,
  const MagickBooleanType post,const int argc,const char **argv,
  Image **images,ExceptionInfo *exception)
{
#define MogrifyImageTag  "Mogrify/Image"

  Image
    *image,
    *mogrify_images;

  MagickBooleanType
    proceed;

  MagickSizeType
    number_images;

  MagickStatusType
    status;

  register long
    i;

  /*
    Apply options to individual images in the list.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (images == (Image **) NULL)
    return(MogrifyImage(image_info,argc,argv,images,exception));
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  if ((argc <= 0) || (*argv == (char *) NULL))
    return(MagickTrue);
  (void) SetImageInfoProgressMonitor(image_info,(MagickProgressMonitor) NULL,
    (void *) NULL);
  mogrify_images=NewImageList();
  number_images=GetImageListLength(*images);
  status=0;
  if (post == MagickFalse)
    status&=MogrifyImageList(image_info,argc,argv,images,exception);
  for (i=0; i < (long) number_images; i++)
  {
    image=RemoveFirstImageFromList(images);
    if (image == (Image *) NULL)
      continue;
    status&=MogrifyImage(image_info,argc,argv,&image,exception);
    AppendImageToList(&mogrify_images,image);
    proceed=SetImageProgress(image,MogrifyImageTag,(MagickOffsetType) i,
      number_images);
    if (proceed == MagickFalse)
      break;
  }
  if (post != MagickFalse)
    status&=MogrifyImageList(image_info,argc,argv,&mogrify_images,exception);
  *images=mogrify_images;
  return(status != 0 ? MagickTrue : MagickFalse);
}
