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
%                                   Cristy                                    %
%                                March 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#include "magick/color-private.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/thread-private.h"
#include "magick/string-private.h"
#include "magick/utility-private.h"

/*
  Define declarations.
*/
#define UndefinedCompressionQuality  0UL

/*
  Constant declaration.
*/
static const char
  MogrifyBackgroundColor[] = "#fff",  /* white */
  MogrifyBorderColor[] = "#dfdfdf",  /* sRGB gray */
  MogrifyMatteColor[] = "#bdbdbd";  /* slightly darker gray */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     M a g i c k C o m m a n d G e n e s i s                                 %
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
%        MagickCommand command,int argc,char **argv,char **metadata,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o command: Choose from ConvertImageCommand, IdentifyImageCommand,
%      MogrifyImageCommand, CompositeImageCommand, CompareImageCommand,
%      ConjureImageCommand, StreamImageCommand, ImportImageCommand,
%      DisplayImageCommand, or AnimateImageCommand.
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%    o metadata: any metadata is returned here.
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
    serial;

  MagickBooleanType
    concurrent,
    regard_warnings,
    status;

  register ssize_t
    i;

  size_t
    iterations,
    number_threads;

  ssize_t
    n;

  (void) setlocale(LC_ALL,"");
  (void) setlocale(LC_NUMERIC,"C");
  concurrent=MagickFalse;
  duration=(-1.0);
  iterations=1;
  status=MagickFalse;
  regard_warnings=MagickFalse;
  for (i=1; i < (ssize_t) (argc-1); i++)
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
    if (LocaleCompare("distribute-cache",option+1) == 0)
      {
        DistributePixelCacheServer(StringToInteger(argv[++i]),exception);
        exit(0);
      }
    if (LocaleCompare("duration",option+1) == 0)
      duration=StringToDouble(argv[++i],(char **) NULL);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=MagickTrue;
  }
  if (iterations == 1)
    {
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
            *metadata=DestroyString(*metadata);
          }
      return(status);
    }
  number_threads=GetOpenMPMaximumThreads();
  serial=0.0;
  for (n=1; n <= (ssize_t) number_threads; n++)
  {
    double
      e,
      parallel,
      user_time;

    TimerInfo
      *timer;

    SetOpenMPMaximumThreads((int) n);
    timer=AcquireTimerInfo();
    if (concurrent == MagickFalse)
      {
        for (i=0; i < (ssize_t) iterations; i++)
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
        for (i=0; i < (ssize_t) iterations; i++)
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
           # pragma omp critical (MagickCore_CommandGenesis)
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
                *metadata=DestroyString(*metadata);
              }
          }
        }
      }
    user_time=GetUserTime(timer);
    parallel=GetElapsedTime(timer);
    e=1.0;
    if (n == 1)
      serial=parallel;
    else
      e=((1.0/(1.0/((serial/(serial+parallel))+(1.0-(serial/(serial+parallel)))/
        (double) n)))-(1.0/(double) n))/(1.0-1.0/(double) n);
    (void) FormatLocaleFile(stderr,
      "Performance[%.20g]: %.20gi %0.3fips %0.3fe %0.3fu %lu:%02lu.%03lu\n",
      (double) n,(double) iterations,(double) iterations/parallel,e,user_time,
      (unsigned long) (parallel/60.0),(unsigned long) floor(fmod(parallel,
      60.0)),(unsigned long) (1000.0*(parallel-floor(parallel))+0.5));
    timer=DestroyTimerInfo(timer);
  }
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
%  MogrifyImage() applies simple single image processing options to a single
%  image that may be part of a large list, but also handles any 'region'
%  image handling.
%
%  The image in the list may be modified in three different ways...
%
%    * directly modified (EG: -negate, -gamma, -level, -annotate, -draw),
%    * replaced by a new image (EG: -spread, -resize, -rotate, -morphology)
%    * replace by a list of images (only the -separate option!)
%
%  In each case the result is returned into the list, and a pointer to the
%  modified image (last image added if replaced by a list of images) is
%  returned.
%
%  ASIDE: The -crop is present but restricted to non-tile single image crops
%
%  This means if all the images are being processed (such as by
%  MogrifyImages(), next image to be processed will be as per the pointer
%  (*image)->next.  Also the image list may grow as a result of some specific
%  operations but as images are never merged or deleted, it will never shrink
%  in length.  Typically the list will remain the same length.
%
%  WARNING: As the image pointed to may be replaced, the first image in the
%  list may also change.  GetFirstImageInList() should be used by caller if
%  they wish return the Image pointer to the first image in list.
%
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

  /*
    Read an image into a image cache if not already present.  Return the image
    that is in the cache under that filename.
  */
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

static inline MagickBooleanType IsPathWritable(const char *path)
{
  if (IsPathAccessible(path) == MagickFalse)
    return(MagickFalse);
  if (access_utf8(path,W_OK) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

static inline ssize_t MagickMax(const ssize_t x,const ssize_t y)
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

  wand_unreferenced(client_data);

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

  Image
    *sparse_image;

  MagickPixelPacket
    color;

  MagickBooleanType
    error;

  register size_t
    x;

  size_t
    number_arguments,
    number_colors;

  /*
    SparseColorOption() parses the complex -sparse-color argument into an an
    array of floating point values then calls SparseColorImage().  Argument is
    a complex mix of floating-point pixel coodinates, and color specifications
    (or direct floating point numbers).  The number of floats needed to
    represent a color varies depending on the current channel setting.
  */
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
    sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
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
    sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
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
        /* Colors given as a set of floating point values - experimental */
        /* NB: token contains the first floating point value to use! */
        if ( channels & RedChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & GreenChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & BlueChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & IndexChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
          token[0] = ','; /* used this token - get another */
        }
        if ( channels & OpacityChannel ) {
          while ( token[0] == ',' ) GetMagickToken(p,&p,token);
          if ( token[0] == '\0' || isalpha((int)token[0]) || token[0] == '#' )
            break;
          sparse_arguments[x++]=StringToDouble(token,(char **) NULL);
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

  ImageInfo
    *mogrify_info;

  MagickStatusType
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

  register ssize_t
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
  mogrify_info=CloneImageInfo(image_info);
  draw_info=CloneDrawInfo(mogrify_info,(DrawInfo *) NULL);
  quantize_info=AcquireQuantizeInfo(mogrify_info);
  SetGeometryInfo(&geometry_info);
  GetMagickPixelPacket(*image,&fill);
  SetMagickPixelPacket(*image,&(*image)->background_color,(IndexPacket *) NULL,
    &fill);
  channel=mogrify_info->channel;
  format=GetImageOption(mogrify_info,"format");
  SetGeometry(*image,&region_geometry);
  region_image=NewImageList();
  /*
    Transmogrify the image.
  */
  for (i=0; i < (ssize_t) argc; i++)
  {
    Image
      *mogrify_image;

    ssize_t
      count;

    option=argv[i];
    if (IsCommandOption(option) == MagickFalse)
      continue;
    count=MagickMax(ParseCommandOption(MagickCommandOptions,MagickFalse,option),
      0L);
    if ((i+count) >= (ssize_t) argc)
      break;
    status=MogrifyImageInfo(mogrify_info,(int) count+1,argv+i,exception);
    mogrify_image=(Image *)NULL;
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("adaptive-blur",option+1) == 0)
          {
            /*
              Adaptive blur image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=AdaptiveBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("adaptive-resize",option+1) == 0)
          {
            /*
              Adaptive resize image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=AdaptiveResizeImage(*image,geometry.width,
              geometry.height,exception);
            break;
          }
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            /*
              Adaptive sharpen image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=AdaptiveSharpenImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,exception);
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

            (void) SyncImageSettings(mogrify_info,*image);
            alpha_type=(AlphaChannelType) ParseCommandOption(MagickAlphaOptions,
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
            (void) SyncImageSettings(mogrify_info,*image);
            SetGeometryInfo(&geometry_info);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            text=InterpretImageProperties(mogrify_info,*image,argv[i+2]);
            InheritException(exception,&(*image)->exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
            (void) AutoGammaImageChannel(*image,channel);
            break;
          }
        if (LocaleCompare("auto-level",option+1) == 0)
          {
            /*
              Perfectly Normalize (max/min stretch) the image
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) AutoLevelImageChannel(*image,channel);
            break;
          }
        if (LocaleCompare("auto-orient",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=AutoOrientImage(*image,(*image)->orientation,
              exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
            (void) BlackThresholdImageChannel(*image,channel,argv[i+1],
              exception);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("blue-shift",option+1) == 0)
          {
            /*
              Blue shift image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            geometry_info.rho=1.5;
            if (*option == '-')
              flags=ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=BlueShiftImage(*image,geometry_info.rho,exception);
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            /*
              Gaussian blur image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=BlurImageChannel(*image,channel,geometry_info.rho,
              geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            /*
              Surround image with a border of solid color.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=BorderImage(*image,&geometry,exception);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) QueryColorDatabase(MogrifyBorderColor,
                  &draw_info->border_color,exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
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
            (void) SyncImageSettings(mogrify_info,*image);
            color_correction_collection=FileToString(argv[i+1],~0UL,exception);
            if (color_correction_collection == (char *) NULL)
              break;
            (void) ColorDecisionListImage(*image,color_correction_collection);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            if (*option == '+')
              channel=DefaultChannels;
            else
              channel=(ChannelType) ParseChannelOption(argv[i+1]);
            break;
          }
        if (LocaleCompare("charcoal",option+1) == 0)
          {
            /*
              Charcoal image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=CharcoalImage(*image,geometry_info.rho,
              geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("chop",option+1) == 0)
          {
            /*
              Chop the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=ChopImage(*image,&geometry,exception);
            break;
          }
        if (LocaleCompare("clamp",option+1) == 0)
          {
            /*
              Clamp image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ClampImageChannel(*image,channel);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("clip",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
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
            CacheView
              *mask_view;

            Image
              *mask_image;

            register PixelPacket
              *restrict q;

            register ssize_t
              x;

            ssize_t
              y;

            (void) SyncImageSettings(mogrify_info,*image);
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
              FUTURE: This Should Be a SetImageAlphaChannel() call, Or two.
            */
            mask_image=GetImageCache(mogrify_info,argv[i+1],exception);
            if (mask_image == (Image *) NULL)
              break;
            if (SetImageStorageClass(mask_image,DirectClass) == MagickFalse)
              return(MagickFalse);
            mask_view=AcquireAuthenticCacheView(mask_image,exception);
            for (y=0; y < (ssize_t) mask_image->rows; y++)
            {
              q=GetCacheViewAuthenticPixels(mask_view,0,y,mask_image->columns,1,
                exception);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (ssize_t) mask_image->columns; x++)
              {
                if (mask_image->matte == MagickFalse)
                  SetPixelOpacity(q,ClampToQuantum(GetPixelIntensity(mask_image,q)));
                SetPixelRed(q,GetPixelOpacity(q));
                SetPixelGreen(q,GetPixelOpacity(q));
                SetPixelBlue(q,GetPixelOpacity(q));
                q++;
              }
              if (SyncCacheViewAuthenticPixels(mask_view,exception) == MagickFalse)
                break;
            }
            mask_view=DestroyCacheView(mask_view);
            mask_image->matte=MagickTrue;
            (void) SetImageClipMask(*image,mask_image);
            mask_image=DestroyImage(mask_image);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("clip-path",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ClipImagePath(*image,argv[i+1],*option == '-' ? MagickTrue :
              MagickFalse);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("colorize",option+1) == 0)
          {
            /*
              Colorize the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=ColorizeImage(*image,argv[i+1],draw_info->fill,
              exception);
            break;
          }
        if (LocaleCompare("color-matrix",option+1) == 0)
          {
            KernelInfo
              *kernel;

            (void) SyncImageSettings(mogrify_info,*image);
            kernel=AcquireKernelInfo(argv[i+1]);
            if (kernel == (KernelInfo *) NULL)
              break;
            mogrify_image=ColorMatrixImage(*image,kernel,exception);
            kernel=DestroyKernelInfo(kernel);
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            /*
              Reduce the number of colors in the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
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

            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              {
                (void) TransformImageColorspace(*image,sRGBColorspace);
                InheritException(exception,&(*image)->exception);
                break;
              }
            colorspace=(ColorspaceType) ParseCommandOption(
              MagickColorspaceOptions,MagickFalse,argv[i+1]);
            (void) TransformImageColorspace(*image,colorspace);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("contrast",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
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
            (void) SyncImageSettings(mogrify_info,*image);
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

            KernelInfo
              *kernel;

            register ssize_t
              j;

            (void) SyncImageSettings(mogrify_info,*image);
            kernel=AcquireKernelInfo(argv[i+1]);
            if (kernel == (KernelInfo *) NULL)
              break;
            gamma=0.0;
            for (j=0; j < (ssize_t) (kernel->width*kernel->height); j++)
              gamma+=kernel->values[j];
            gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
            for (j=0; j < (ssize_t) (kernel->width*kernel->height); j++)
              kernel->values[j]*=gamma;
            mogrify_image=FilterImageChannel(*image,channel,kernel,exception);
            kernel=DestroyKernelInfo(kernel);
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            /*
              Crop a image to a smaller size
            */
            (void) SyncImageSettings(mogrify_info,*image);
#if 0
            flags=ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            if (((geometry.width != 0) || (geometry.height != 0)) &&
                ((flags & XValue) == 0) && ((flags & YValue) == 0))
              break;
#endif
#if 0
            mogrify_image=CloneImage(*image,0,0,MagickTrue,&(*image)->exception);
            mogrify_image->next = mogrify_image->previous = (Image *)NULL;
            (void) TransformImage(&mogrify_image,argv[i+1],(char *) NULL);
            InheritException(exception,&mogrify_image->exception);
#else
            mogrify_image=CropImageToTiles(*image,argv[i+1],exception);
#endif
            break;
          }
        if (LocaleCompare("cycle",option+1) == 0)
          {
            /*
              Cycle an image colormap.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) CycleColormapImage(*image,(ssize_t) StringToLong(argv[i+1]));
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
            (void) SyncImageSettings(mogrify_info,*image);
            passkey=FileToStringInfo(argv[i+1],~0UL,exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
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

            /*
              Straighten the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              threshold=40.0*QuantumRange/100.0;
            else
              threshold=StringToDoubleInterval(argv[i+1],(double) QuantumRange+
                1.0);
            mogrify_image=DeskewImage(*image,threshold,exception);
            break;
          }
        if (LocaleCompare("despeckle",option+1) == 0)
          {
            /*
              Reduce the speckles within an image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=DespeckleImage(*image,exception);
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

            register ssize_t
              x;

            size_t
              number_arguments;

            /*
              Distort image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            method=(DistortImageMethod) ParseCommandOption(MagickDistortOptions,
              MagickFalse,argv[i+1]);
            if ( method == ResizeDistortion )
              {
                 /* Special Case - Argument is actually a resize geometry!
                 ** Convert that to an appropriate distortion argument array.
                 */
                 double
                   resize_args[2];
                 (void) ParseRegionGeometry(*image,argv[i+2],&geometry,
                      exception);
                 resize_args[0]=(double)geometry.width;
                 resize_args[1]=(double)geometry.height;
                 mogrify_image=DistortImage(*image,method,(size_t)2,
                      resize_args,MagickTrue,exception);
                 break;
              }
            args=InterpretImageProperties(mogrify_info,*image,argv[i+2]);
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
              arguments[x]=StringToDouble(token,(char **) NULL);
            }
            args=DestroyString(args);
            mogrify_image=DistortImage(*image,method,number_arguments,arguments,
              (*option == '+') ? MagickTrue : MagickFalse,exception);
            arguments=(double *) RelinquishMagickMemory(arguments);
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
            quantize_info->dither_method=(DitherMethod) ParseCommandOption(
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
            (void) SyncImageSettings(mogrify_info,*image);
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
            /*
              Enhance edges in the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=EdgeImage(*image,geometry_info.rho,exception);
            break;
          }
        if (LocaleCompare("emboss",option+1) == 0)
          {
            /*
              Gaussian embossen image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=EmbossImage(*image,geometry_info.rho,
              geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("encipher",option+1) == 0)
          {
            StringInfo
              *passkey;

            /*
              Encipher pixels.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            passkey=FileToStringInfo(argv[i+1],~0UL,exception);
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
            /*
              Enhance image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=EnhanceImage(*image,exception);
            break;
          }
        if (LocaleCompare("equalize",option+1) == 0)
          {
            /*
              Equalize image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
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

            (void) SyncImageSettings(mogrify_info,*image);
            op=(MagickEvaluateOperator) ParseCommandOption(
              MagickEvaluateOptions,MagickFalse,argv[i+1]);
            constant=StringToDoubleInterval(argv[i+2],(double) QuantumRange+
              1.0);
            (void) EvaluateImageChannel(*image,channel,op,constant,exception);
            break;
          }
        if (LocaleCompare("extent",option+1) == 0)
          {
            /*
              Set the image extent.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            if (geometry.width == 0)
              geometry.width=(*image)->columns;
            if (geometry.height == 0)
              geometry.height=(*image)->rows;
            mogrify_image=ExtentImage(*image,&geometry,exception);
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
              draw_info->fill_pattern=GetImageCache(mogrify_info,argv[i+1],
                exception);
            break;
          }
        if (LocaleCompare("flip",option+1) == 0)
          {
            /*
              Flip image scanlines.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=FlipImage(*image,exception);
            break;
          }
        if (LocaleCompare("floodfill",option+1) == 0)
          {
            MagickPixelPacket
              target;

            /*
              Floodfill image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            (void) QueryMagickColor(argv[i+2],&target,exception);
            (void) FloodfillPaintImage(*image,channel,draw_info,&target,
              geometry.x,geometry.y,*option == '-' ? MagickFalse : MagickTrue);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("flop",option+1) == 0)
          {
            /*
              Flop image scanlines.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=FlopImage(*image,exception);
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

            /*
              Surround image with an ornamental border.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            frame_info.width=geometry.width;
            frame_info.height=geometry.height;
            frame_info.outer_bevel=geometry.x;
            frame_info.inner_bevel=geometry.y;
            frame_info.x=(ssize_t) frame_info.width;
            frame_info.y=(ssize_t) frame_info.height;
            frame_info.width=(*image)->columns+2*frame_info.width;
            frame_info.height=(*image)->rows+2*frame_info.height;
            mogrify_image=FrameImage(*image,&frame_info,exception);
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

            register ssize_t
              x;

            size_t
              number_parameters;

            /*
              Function Modify Image Values
            */
            (void) SyncImageSettings(mogrify_info,*image);
            function=(MagickFunction) ParseCommandOption(MagickFunctionOptions,
              MagickFalse,argv[i+1]);
            arguments=InterpretImageProperties(mogrify_info,*image,argv[i+2]);
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
              parameters[x]=StringToDouble(token,(char **) NULL);
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
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              (*image)->gamma=StringToDouble(argv[i+1],(char **) NULL);
            else
              {
                if (strchr(argv[i+1],',') != (char *) NULL)
                  (void) GammaImage(*image,argv[i+1]);
                else
                  (void) GammaImageChannel(*image,channel,
                    StringToDouble(argv[i+1],(char **) NULL));
                InheritException(exception,&(*image)->exception);
              }
            break;
          }
        if ((LocaleCompare("gaussian-blur",option+1) == 0) ||
            (LocaleCompare("gaussian",option+1) == 0))
          {
            /*
              Gaussian blur image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=GaussianBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
              /*
                Record Image offset, Resize last image.
              */
            (void) SyncImageSettings(mogrify_info,*image);
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
              mogrify_image=ResizeImage(*image,geometry.width,geometry.height,
                (*image)->filter,(*image)->blur,exception);
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            if (*option == '+')
              {
                draw_info->gravity=UndefinedGravity;
                break;
              }
            draw_info->gravity=(GravityType) ParseCommandOption(
              MagickGravityOptions,MagickFalse,argv[i+1]);
            break;
          }
        if (LocaleCompare("grayscale",option+1) == 0)
          {
            PixelIntensityMethod
              method;

            (void) SyncImagesSettings(mogrify_info,*image);
            method=(PixelIntensityMethod) ParseCommandOption(
              MagickPixelIntensityOptions,MagickFalse,argv[i+1]);
            (void) GrayscaleImage(*image,method);
            InheritException(exception,&(*image)->exception);
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

            (void) SyncImageSettings(mogrify_info,*image);
            if (format == (char *) NULL)
              {
                (void) IdentifyImage(*image,stdout,mogrify_info->verbose);
                InheritException(exception,&(*image)->exception);
                break;
              }
            text=InterpretImageProperties(mogrify_info,*image,format);
            InheritException(exception,&(*image)->exception);
            if (text == (char *) NULL)
              break;
            (void) fputs(text,stdout);
            text=DestroyString(text);
            break;
          }
        if (LocaleCompare("implode",option+1) == 0)
          {
            /*
              Implode image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=ImplodeImage(*image,geometry_info.rho,exception);
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
        if (LocaleCompare("interpolative-resize",option+1) == 0)
          {
            /*
              Resize image using 'point sampled' interpolation
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=InterpolativeResizeImage(*image,geometry.width,
              geometry.height,(*image)->interpolate,exception);
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
            /*
              Local adaptive threshold image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & PercentValue) != 0)
              geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
            mogrify_image=AdaptiveThresholdImage(*image,(size_t)
              geometry_info.rho,(size_t) geometry_info.sigma,(ssize_t)
              geometry_info.xi,exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
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
                if ((isalpha((int) *token) == 0) && ((*token == '#') == 0))
                  GetMagickToken(p,&p,token); /* Get white point color. */
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

            (void) SyncImageSettings(mogrify_info,*image);
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
            draw_info->stroke_width=StringToDouble(argv[i+1],(char **) NULL);
            break;
          }
        if (LocaleCompare("liquid-rescale",option+1) == 0)
          {
            /*
              Liquid rescale image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            if ((flags & XValue) == 0)
              geometry.x=1;
            if ((flags & YValue) == 0)
              geometry.y=0;
            mogrify_image=LiquidRescaleImage(*image,geometry.width,
              geometry.height,1.0*geometry.x,1.0*geometry.y,exception);
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
        if (LocaleCompare("magnify",option+1) == 0)
          {
            /*
              Double image size.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=MagnifyImage(*image,exception);
            break;
          }
        if (LocaleCompare("map",option+1) == 0)
          {
            Image
              *remap_image;

            /*
              Transform image colors to match this set of colors.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              break;
            remap_image=GetImageCache(mogrify_info,argv[i+1],exception);
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

            (void) SyncImageSettings(mogrify_info,*image);
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
            mask=GetImageCache(mogrify_info,argv[i+1],exception);
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
            /*
              Median filter image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=StatisticImageChannel(*image,channel,MedianStatistic,
              (size_t) geometry_info.rho,(size_t) geometry_info.rho,exception);
            break;
          }
        if (LocaleCompare("mode",option+1) == 0)
          {
            /*
              Mode image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=StatisticImageChannel(*image,channel,ModeStatistic,
              (size_t) geometry_info.rho,(size_t) geometry_info.rho,exception);
            break;
          }
        if (LocaleCompare("modulate",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ModulateImage(*image,argv[i+1]);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("moments",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageArtifact(*image,"identify:moments");
                break;
              }
            (void) SetImageArtifact(*image,"identify:moments",argv[i+1]);
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageProgressMonitor(*image,
                  (MagickProgressMonitor) NULL,(void *) NULL);
                break;
              }
            (void) SetImageProgressMonitor(*image,MonitorProgress,
              (void *) NULL);
            break;
          }
        if (LocaleCompare("monochrome",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            (void) SetImageType(*image,BilevelType);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("morphology",option+1) == 0)
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
            (void) SyncImageSettings(mogrify_info,*image);
            p=argv[i+1];
            GetMagickToken(p,&p,token);
            method=(MorphologyMethod) ParseCommandOption(
              MagickMorphologyOptions,MagickFalse,token);
            iterations=1L;
            GetMagickToken(p,&p,token);
            if ((*p == ':') || (*p == ','))
              GetMagickToken(p,&p,token);
            if ((*p != '\0'))
              iterations=(ssize_t) StringToLong(p);
            kernel=AcquireKernelInfo(argv[i+2]);
            if (kernel == (KernelInfo *) NULL)
              {
                (void) ThrowMagickException(exception,GetMagickModule(),
                  OptionError,"UnabletoParseKernel","morphology");
                status=MagickFalse;
                break;
              }
            mogrify_image=MorphologyImageChannel(*image,channel,method,
              iterations,kernel,exception);
            kernel=DestroyKernelInfo(kernel);
            break;
          }
        if (LocaleCompare("motion-blur",option+1) == 0)
          {
            /*
              Motion blur image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=MotionBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,geometry_info.xi,exception);
            break;
          }
        break;
      }
      case 'n':
      {
        if (LocaleCompare("negate",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            (void) NegateImageChannel(*image,channel,*option == '+' ?
              MagickTrue : MagickFalse);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("noise",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '-')
              {
                flags=ParseGeometry(argv[i+1],&geometry_info);
                if ((flags & SigmaValue) == 0)
                  geometry_info.sigma=geometry_info.rho;
                mogrify_image=StatisticImageChannel(*image,channel,
                  NonpeakStatistic,(size_t) geometry_info.rho,(size_t)
                  geometry_info.sigma,exception);
              }
            else
              {
                NoiseType
                  noise;

                noise=(NoiseType) ParseCommandOption(MagickNoiseOptions,
                  MagickFalse,argv[i+1]);
                mogrify_image=AddNoiseImageChannel(*image,channel,noise,
                  exception);
              }
            break;
          }
        if (LocaleCompare("normalize",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
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

            (void) SyncImageSettings(mogrify_info,*image);
            (void) QueryMagickColor(argv[i+1],&target,exception);
            (void) OpaquePaintImageChannel(*image,channel,&target,&fill,
              *option == '-' ? MagickFalse : MagickTrue);
            break;
          }
        if (LocaleCompare("ordered-dither",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
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
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=OilPaintImage(*image,geometry_info.rho,exception);
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
        if (LocaleCompare("perceptible",option+1) == 0)
          {
            /*
              Perceptible image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) PerceptibleImageChannel(*image,channel,StringToDouble(
              argv[i+1],(char **) NULL));
            InheritException(exception,&(*image)->exception);
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

            RandomInfo
              *random_info;

            /*
              Simulate a Polaroid picture.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            random_info=AcquireRandomInfo();
            angle=22.5*(GetPseudoRandomValue(random_info)-0.5);
            random_info=DestroyRandomInfo(random_info);
            if (*option == '-')
              {
                SetGeometryInfo(&geometry_info);
                flags=ParseGeometry(argv[i+1],&geometry_info);
                angle=geometry_info.rho;
              }
            mogrify_image=PolaroidImage(*image,draw_info,angle,exception);
            break;
          }
        if (LocaleCompare("posterize",option+1) == 0)
          {
            /*
              Posterize image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) PosterizeImage(*image,StringToUnsignedLong(argv[i+1]),
              quantize_info->dither);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("preview",option+1) == 0)
          {
            PreviewType
              preview_type;

            /*
              Preview image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              preview_type=UndefinedPreview;
            else
              preview_type=(PreviewType) ParseCommandOption(
                MagickPreviewOptions,MagickFalse,argv[i+1]);
            mogrify_image=PreviewImage(*image,preview_type,exception);
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

            (void) SyncImageSettings(mogrify_info,*image);
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
            profile_info=CloneImageInfo(mogrify_info);
            profile=GetImageProfile(*image,"iptc");
            if (profile != (StringInfo *) NULL)
              profile_info->profile=(void *) CloneStringInfo(profile);
            profile_image=GetImageCache(profile_info,argv[i+1],exception);
            profile_info=DestroyImageInfo(profile_info);
            if (profile_image == (Image *) NULL)
              {
                StringInfo
                  *profile;

                profile_info=CloneImageInfo(mogrify_info);
                (void) CopyMagickString(profile_info->filename,argv[i+1],
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
        if (LocaleCompare("quantize",option+1) == 0)
          {
            if (*option == '+')
              {
                quantize_info->colorspace=UndefinedColorspace;
                break;
              }
            quantize_info->colorspace=(ColorspaceType) ParseCommandOption(
              MagickColorspaceOptions,MagickFalse,argv[i+1]);
            break;
          }
        break;
      }
      case 'r':
      {
        if (LocaleCompare("radial-blur",option+1) == 0)
          {
            /*
              Radial blur image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=RadialBlurImageChannel(*image,channel,
              StringToDouble(argv[i+1],(char **) NULL),exception);
            break;
          }
        if (LocaleCompare("raise",option+1) == 0)
          {
            /*
              Surround image with a raise of solid color.
            */
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
            (void) RandomThresholdImageChannel(*image,channel,argv[i+1],
              exception);
            break;
          }
        if (LocaleCompare("recolor",option+1) == 0)
          {
            KernelInfo
              *kernel;

            (void) SyncImageSettings(mogrify_info,*image);
            kernel=AcquireKernelInfo(argv[i+1]);
            if (kernel == (KernelInfo *) NULL)
              break;
            mogrify_image=ColorMatrixImage(*image,kernel,exception);
            kernel=DestroyKernelInfo(kernel);
            break;
          }
        if (LocaleCompare("region",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
            if (region_image != (Image *) NULL)
              {
                /*
                  Composite region.
                */
                (void) CompositeImage(region_image,region_image->matte !=
                     MagickFalse ? CopyCompositeOp : OverCompositeOp,*image,
                     region_geometry.x,region_geometry.y);
                InheritException(exception,&region_image->exception);
                *image=DestroyImage(*image);
                *image=region_image;
                region_image = (Image *) NULL;
              }
            if (*option == '+')
              break;
            /*
              Apply transformations to a selected region of the image.
            */
            (void) ParseGravityGeometry(*image,argv[i+1],&region_geometry,
              exception);
            mogrify_image=CropImage(*image,&region_geometry,exception);
            if (mogrify_image == (Image *) NULL)
              break;
            region_image=(*image);
            *image=mogrify_image;
            mogrify_image=(Image *) NULL;
            break;
          }
        if (LocaleCompare("render",option+1) == 0)
          {
            (void) SyncImageSettings(mogrify_info,*image);
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
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              break;
            remap_image=GetImageCache(mogrify_info,argv[i+1],exception);
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
            /*
              Resample image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            mogrify_image=ResampleImage(*image,geometry_info.rho,
              geometry_info.sigma,(*image)->filter,(*image)->blur,exception);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            /*
              Resize image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=ResizeImage(*image,geometry.width,geometry.height,
              (*image)->filter,(*image)->blur,exception);
            break;
          }
        if (LocaleCompare("roll",option+1) == 0)
          {
            /*
              Roll image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=RollImage(*image,geometry.x,geometry.y,exception);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            char
              *geometry;

            /*
              Check for conditional image rotation.
            */
            (void) SyncImageSettings(mogrify_info,*image);
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
            mogrify_image=RotateImage(*image,geometry_info.rho,exception);
            break;
          }
        break;
      }
      case 's':
      {
        if (LocaleCompare("sample",option+1) == 0)
          {
            /*
              Sample image with pixel replication.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=SampleImage(*image,geometry.width,geometry.height,
              exception);
            break;
          }
        if (LocaleCompare("scale",option+1) == 0)
          {
            /*
              Resize image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=ScaleImage(*image,geometry.width,geometry.height,
              exception);
            break;
          }
        if (LocaleCompare("selective-blur",option+1) == 0)
          {
            /*
              Selectively blur pixels within a contrast threshold.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & PercentValue) != 0)
              geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
            mogrify_image=SelectiveBlurImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,geometry_info.xi,exception);
            break;
          }
        if (LocaleCompare("separate",option+1) == 0)
          {
            /*
              Break channels into separate images.
              WARNING: This can generate multiple images!
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=SeparateImages(*image,channel,exception);
            break;
          }
        if (LocaleCompare("sepia-tone",option+1) == 0)
          {
            double
              threshold;

            /*
              Sepia-tone image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            threshold=StringToDoubleInterval(argv[i+1],(double) QuantumRange+
              1.0);
            mogrify_image=SepiaToneImage(*image,threshold,exception);
            break;
          }
        if (LocaleCompare("segment",option+1) == 0)
          {
            /*
              Segment image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            (void) SegmentImage(*image,(*image)->colorspace,
              mogrify_info->verbose,geometry_info.rho,geometry_info.sigma);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            char
              *value;

            /*
              Set image option.
            */
            if (*option == '+')
              {
                if (LocaleNCompare(argv[i+1],"registry:",9) == 0)
                  (void) DeleteImageRegistry(argv[i+1]+9);
                else
                  if (LocaleNCompare(argv[i+1],"option:",7) == 0)
                    {
                      (void) DeleteImageOption(mogrify_info,argv[i+1]+7);
                      (void) DeleteImageArtifact(*image,argv[i+1]+7);
                    }
                  else
                    (void) DeleteImageProperty(*image,argv[i+1]);
                break;
              }
            value=InterpretImageProperties(mogrify_info,*image,argv[i+2]);
            if (value == (char *) NULL)
              break;
            if (LocaleNCompare(argv[i+1],"registry:",9) == 0)
              (void) SetImageRegistry(StringRegistryType,argv[i+1]+9,value,
                exception);
            else
              if (LocaleNCompare(argv[i+1],"option:",7) == 0)
                {
                  (void) SetImageOption(image_info,argv[i+1]+7,value);
                  (void) SetImageOption(mogrify_info,argv[i+1]+7,value);
                  (void) SetImageArtifact(*image,argv[i+1]+7,value);
                }
              else
                (void) SetImageProperty(*image,argv[i+1],value);
            value=DestroyString(value);
            break;
          }
        if (LocaleCompare("shade",option+1) == 0)
          {
            /*
              Shade image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=ShadeImage(*image,(*option == '-') ? MagickTrue :
              MagickFalse,geometry_info.rho,geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("shadow",option+1) == 0)
          {
            /*
              Shadow image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & XiValue) == 0)
              geometry_info.xi=4.0;
            if ((flags & PsiValue) == 0)
              geometry_info.psi=4.0;
            mogrify_image=ShadowImage(*image,geometry_info.rho,
              geometry_info.sigma,(ssize_t) ceil(geometry_info.xi-0.5),(ssize_t)
              ceil(geometry_info.psi-0.5),exception);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            /*
              Sharpen image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=SharpenImageChannel(*image,channel,geometry_info.rho,
              geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("shave",option+1) == 0)
          {
            /*
              Shave the image edges.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParsePageGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=ShaveImage(*image,&geometry,exception);
            break;
          }
        if (LocaleCompare("shear",option+1) == 0)
          {
            /*
              Shear image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=geometry_info.rho;
            mogrify_image=ShearImage(*image,geometry_info.rho,
              geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
          {
            /*
              Sigmoidal non-linearity contrast control.
            */
            (void) SyncImageSettings(mogrify_info,*image);
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
            /*
              Sketch image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=SketchImage(*image,geometry_info.rho,
              geometry_info.sigma,geometry_info.xi,exception);
            break;
          }
        if (LocaleCompare("solarize",option+1) == 0)
          {
            double
              threshold;

            (void) SyncImageSettings(mogrify_info,*image);
            threshold=StringToDoubleInterval(argv[i+1],(double) QuantumRange+
              1.0);
            (void) SolarizeImageChannel(*image,channel,threshold,exception);
            break;
          }
        if (LocaleCompare("sparse-color",option+1) == 0)
          {
            SparseColorMethod
              method;

            char
              *arguments;

            /*
              Sparse Color Interpolated Gradient
            */
            (void) SyncImageSettings(mogrify_info,*image);
            method=(SparseColorMethod) ParseCommandOption(
              MagickSparseColorOptions,MagickFalse,argv[i+1]);
            arguments=InterpretImageProperties(mogrify_info,*image,argv[i+2]);
            InheritException(exception,&(*image)->exception);
            if (arguments == (char *) NULL)
              break;
            mogrify_image=SparseColorOption(*image,channel,method,arguments,
              option[0] == '+' ? MagickTrue : MagickFalse,exception);
            arguments=DestroyString(arguments);
            break;
          }
        if (LocaleCompare("splice",option+1) == 0)
          {
            /*
              Splice a solid color into the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGravityGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=SpliceImage(*image,&geometry,exception);
            break;
          }
        if (LocaleCompare("spread",option+1) == 0)
          {
            /*
              Spread an image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=SpreadImage(*image,geometry_info.rho,exception);
            break;
          }
        if (LocaleCompare("statistic",option+1) == 0)
          {
            StatisticType
              type;

            (void) SyncImageSettings(mogrify_info,*image);
            type=(StatisticType) ParseCommandOption(MagickStatisticOptions,
              MagickFalse,argv[i+1]);
            (void) ParseGeometry(argv[i+2],&geometry_info);
            mogrify_image=StatisticImageChannel(*image,channel,type,(size_t)
              geometry_info.rho,(size_t) geometry_info.sigma,exception);
            break;
          }
        if (LocaleCompare("stretch",option+1) == 0)
          {
            if (*option == '+')
              {
                draw_info->stretch=UndefinedStretch;
                break;
              }
            draw_info->stretch=(StretchType) ParseCommandOption(
              MagickStretchOptions,MagickFalse,argv[i+1]);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          {
            /*
              Strip image of profiles and comments.
            */
            (void) SyncImageSettings(mogrify_info,*image);
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
              draw_info->stroke_pattern=GetImageCache(mogrify_info,argv[i+1],
                exception);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            draw_info->stroke_width=StringToDouble(argv[i+1],(char **) NULL);
            break;
          }
        if (LocaleCompare("style",option+1) == 0)
          {
            if (*option == '+')
              {
                draw_info->style=UndefinedStyle;
                break;
              }
            draw_info->style=(StyleType) ParseCommandOption(MagickStyleOptions,
              MagickFalse,argv[i+1]);
            break;
          }
        if (LocaleCompare("swirl",option+1) == 0)
          {
            /*
              Swirl image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseGeometry(argv[i+1],&geometry_info);
            mogrify_image=SwirlImage(*image,geometry_info.rho,exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              threshold=(double) QuantumRange/2;
            else
              threshold=StringToDoubleInterval(argv[i+1],(double) QuantumRange+
                1.0);
            (void) BilevelImageChannel(*image,channel,threshold);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            /*
              Thumbnail image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            (void) ParseRegionGeometry(*image,argv[i+1],&geometry,exception);
            mogrify_image=ThumbnailImage(*image,geometry.width,geometry.height,
              exception);
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
            draw_info->fill_pattern=GetImageCache(mogrify_info,argv[i+1],
              exception);
            break;
          }
        if (LocaleCompare("tint",option+1) == 0)
          {
            /*
              Tint the image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=TintImage(*image,argv[i+1],draw_info->fill,exception);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          {
            /*
              Affine transform image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=AffineTransformImage(*image,&draw_info->affine,
              exception);
            break;
          }
        if (LocaleCompare("transparent",option+1) == 0)
          {
            MagickPixelPacket
              target;

            (void) SyncImageSettings(mogrify_info,*image);
            (void) QueryMagickColor(argv[i+1],&target,exception);
            (void) TransparentPaintImage(*image,&target,(Quantum)
              TransparentOpacity,*option == '-' ? MagickFalse : MagickTrue);
            InheritException(exception,&(*image)->exception);
            break;
          }
        if (LocaleCompare("transpose",option+1) == 0)
          {
            /*
              Transpose image scanlines.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=TransposeImage(*image,exception);
            break;
          }
        if (LocaleCompare("transverse",option+1) == 0)
          {
            /*
              Transverse image scanlines.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=TransverseImage(*image,exception);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            quantize_info->tree_depth=StringToUnsignedLong(argv[i+1]);
            break;
          }
        if (LocaleCompare("trim",option+1) == 0)
          {
            /*
              Trim image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=TrimImage(*image,exception);
            break;
          }
        if (LocaleCompare("type",option+1) == 0)
          {
            ImageType
              type;

            (void) SyncImageSettings(mogrify_info,*image);
            if (*option == '+')
              type=UndefinedType;
            else
              type=(ImageType) ParseCommandOption(MagickTypeOptions,MagickFalse,
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
                (void) DeleteImageArtifact(*image,"identify:unique-colors");
                break;
              }
            (void) SetImageArtifact(*image,"identify:unique-colors","true");
            (void) SetImageArtifact(*image,"verbose","true");
            break;
          }
        if (LocaleCompare("unique-colors",option+1) == 0)
          {
            /*
              Unique image colors.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            mogrify_image=UniqueImageColors(*image,exception);
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            /*
              Unsharp mask image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & XiValue) == 0)
              geometry_info.xi=1.0;
            if ((flags & PsiValue) == 0)
              geometry_info.psi=0.05;
            mogrify_image=UnsharpMaskImageChannel(*image,channel,
              geometry_info.rho,geometry_info.sigma,geometry_info.xi,
              geometry_info.psi,exception);
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
            /*
              Vignette image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            if ((flags & XiValue) == 0)
              geometry_info.xi=0.1*(*image)->columns;
            if ((flags & PsiValue) == 0)
              geometry_info.psi=0.1*(*image)->rows;
            if ((flags & PercentValue) != 0)
              {
                geometry_info.xi*=(double) (*image)->columns/100.0;
                geometry_info.psi*=(double) (*image)->rows/100.0;
              }
            mogrify_image=VignetteImage(*image,geometry_info.rho,
              geometry_info.sigma,(ssize_t) ceil(geometry_info.xi-0.5),(ssize_t)
              ceil(geometry_info.psi-0.5),exception);
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
              ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i+1]));
            break;
          }
        break;
      }
      case 'w':
      {
        if (LocaleCompare("wave",option+1) == 0)
          {
            /*
              Wave image.
            */
            (void) SyncImageSettings(mogrify_info,*image);
            flags=ParseGeometry(argv[i+1],&geometry_info);
            if ((flags & SigmaValue) == 0)
              geometry_info.sigma=1.0;
            mogrify_image=WaveImage(*image,geometry_info.rho,
              geometry_info.sigma,exception);
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
            (void) SyncImageSettings(mogrify_info,*image);
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
    /*
       Replace current image with any image that was generated
    */
    if (mogrify_image != (Image *) NULL)
      ReplaceImageInListReturnLast(image,mogrify_image);
    i+=count;
  }
  if (region_image != (Image *) NULL)
    {
      /*
        Composite transformed region onto image.
      */
      (void) SyncImageSettings(mogrify_info,*image);
      (void) CompositeImage(region_image,region_image->matte != MagickFalse ?
        CopyCompositeOp : OverCompositeOp,*image,region_geometry.x,
        region_geometry.y);
      InheritException(exception,&region_image->exception);
      *image=DestroyImage(*image);
      *image=region_image;
      region_image = (Image *) NULL;
    }
  /*
    Free resources.
  */
  quantize_info=DestroyQuantizeInfo(quantize_info);
  draw_info=DestroyDrawInfo(draw_info);
  mogrify_info=DestroyImageInfo(mogrify_info);
  status=(MagickStatusType) ((*image)->exception.severity ==
    UndefinedException ? 1 : 0);
  return(status == 0 ? MagickFalse : MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+    M o g r i f y I m a g e C o m m a n d                                    %
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
      "-distribute-cache port",
      "                     distributed pixel cache spanning one or more servers",
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
      "-clamp               keep pixel values in range (0-QuantumRange)",
      "-clip                clip along the first path from the 8BIM profile",
      "-clip-mask filename  associate a clip mask with the image",
      "-clip-path id        clip along a named path from the 8BIM profile",
      "-colorize value      colorize the image with the fill color",
      "-color-matrix matrix apply color correction to the image",
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
      "-features distance   analyze image features (e.g. contrast, correlation)",
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
      "-grayscale method    convert image to grayscale",
      "-help                print program options",
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
      "-magnify             double the size of the image with pixel art scaling",
      "-median geometry     apply a median filter to the image",
      "-mode geometry       make each pixel the 'predominant color' of the",
      "                     neighborhood",
      "-modulate value      vary the brightness, saturation, and hue",
      "-monochrome          transform image to black and white",
      "-morphology method kernel",
      "                     apply a morphology method to the image",
      "-motion-blur geometry",
      "                     simulate motion blur",
      "-negate              replace every pixel with its complementary color ",
      "-noise geometry      add or reduce noise in an image",
      "-normalize           transform image to span the full range of colors",
      "-opaque color        change this color to the fill color",
      "-ordered-dither NxN",
      "                     add a noise pattern to the image with specific",
      "                     amplitudes",
      "-paint radius        simulate an oil painting",
      "-perceptible epsilon",
      "                     pixel value less than |epsilon| become epsilon or",
      "                     -epsilon",
      "-polaroid angle      simulate a Polaroid picture",
      "-posterize levels    reduce the image to a limited number of color levels",
      "-profile filename    add, delete, or apply an image profile",
      "-quantize colorspace reduce colors in this colorspace",
      "-radial-blur angle   radial blur the image",
      "-raise value         lighten/darken image edges to create a 3-D effect",
      "-random-threshold low,high",
      "                     random threshold the image",
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
      "                     increase the contrast without saturating highlights or",
      "                     shadows",
      "-sketch geometry     simulate a pencil sketch",
      "-solarize threshold  negate all pixels above the threshold level",
      "-sparse-color method args",
      "                     fill in a image based on a few color points",
      "-splice geometry     splice the background color into the image",
      "-spread radius       displace image pixels by a random amount",
      "-statistic type radius",
      "                     replace each pixel with corresponding statistic from the neighborhood",
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
      "-affinity filename   transform image colors to match this set of colors",
      "-append              append an image sequence",
      "-clut                apply a color lookup table to the image",
      "-coalesce            merge a sequence of images",
      "-combine             combine a sequence of images",
      "-compare             mathematically and visually annotate the difference between an image and its reconstruction",
      "-complex operator    perform complex mathematics on an image sequence",
      "-composite           composite image",
      "-crop geometry       cut out a rectangular region of the image",
      "-deconstruct         break down an image sequence into constituent parts",
      "-evaluate-sequence operator",
      "                     evaluate an arithmetic, relational, or logical expression",
      "-flatten             flatten a sequence of images",
      "-fx expression       apply mathematical expression to an image channel(s)",
      "-hald-clut           apply a Hald color lookup table to the image",
      "-layers method       optimize, merge, or compare image layers",
      "-morph value         morph an image sequence",
      "-mosaic              create a mosaic from an image sequence",
      "-poly terms          build a polynomial from the image sequence and the corresponding",
      "                     terms (coefficients and degree pairs).",
      "-print string        interpret string and print to console",
      "-process arguments   process the image with a custom image filter",
      "-separate            separate an image channel into a grayscale image",
      "-smush geometry      smush an image sequence together",
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
      "-cdl filename        color correct with a color decision list",
      "-channel type        apply option to select image channels",
      "-colors value        preferred number of colors in the image",
      "-colorspace type     alternate image colorspace",
      "-comment string      annotate image with comment",
      "-compose operator    set image composite operator",
      "-compress type       type of pixel compression when writing the image",
      "-decipher filename   convert cipher pixels to plain pixels",
      "-define format:option",
      "                     define one or more image format options",
      "-delay value         display the next image after pausing",
      "-density geometry    horizontal and vertical density of the image",
      "-depth value         image depth",
      "-direction type      render text right-to-left or left-to-right",
      "-display server      get image or font from this X server",
      "-dispose method      layer disposal method",
      "-dither method       apply error diffusion to image",
      "-encipher filename   convert plain pixels to cipher pixels",
      "-encoding type       text encoding type",
      "-endian type         endianness (MSB or LSB) of the image",
      "-family name         render text with this font family",
      "-features distance   analyze image features (e.g. contrast, correlation)",
      "-fill color          color to use when filling a graphic primitive",
      "-filter type         use this filter when resizing an image",
      "-flatten             flatten a sequence of images",
      "-font name           render text with this font",
      "-format \"string\"     output formatted image characteristics",
      "-function name       apply a function to the image",
      "-fuzz distance       colors within this distance are considered equal",
      "-gravity type        horizontal and vertical text placement",
      "-green-primary point chromaticity green primary point",
      "-intensity method    method to generate intensity value from pixel",
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
      "-matte               store matte channel if the image has one",
      "-mattecolor color    frame color",
      "-monitor             monitor progress",
      "-morphology method kernel",
      "                     apply a morphology method to the image",
      "-orient type         image orientation",
      "-page geometry       size and location of an image canvas (setting)",
      "-path path           write images to this path on disk",
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
      "-synchronize         synchronize image to storage device",
      "-taint               declare the image as modified",
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
      "-delete indexes      delete the image from the image sequence",
      "-duplicate count,indexes",
      "                     duplicate an image one or more times",
      "-insert index        insert last image into the image sequence",
      "-reverse             reverse image sequence",
      "-swap indexes        swap two images in the image sequence",
      (char *) NULL
    };

  const char
    **p;

  ListMagickVersion(stdout);
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
  for (i=0; i < (ssize_t) argc; i++) \
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

  MagickBooleanType
    global_colormap;

  MagickBooleanType
    fire,
    pend,
    respect_parenthesis;

  MagickStatusType
    status;

  register ssize_t
    i;

  ssize_t
    j,
    k;

  wand_unreferenced(metadata);

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
          ListMagickVersion(stdout);
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
  respect_parenthesis=MagickFalse;
  status=MagickTrue;
  /*
    Parse command line.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowMogrifyException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=1; i < (ssize_t) argc; i++)
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
    if (IsCommandOption(option) == MagickFalse)
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
        if ((LocaleCompare(filename,"--") == 0) && (i < (ssize_t) (argc-1)))
          filename=argv[++i];
        (void) SetImageOption(image_info,"filename",filename);
        (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
        images=ReadImages(image_info,exception);
        status&=(images != (Image *) NULL) &&
          (exception->severity < ErrorException);
        if (images == (Image *) NULL)
          continue;
        if (format != (char *) NULL)
          (void) CopyMagickString(images->filename,images->magick_filename,
            MaxTextExtent);
        if (path != (char *) NULL)
          {
            GetPathComponent(option,TailPath,filename);
            (void) FormatLocaleString(images->filename,MaxTextExtent,"%s%c%s",
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
            register ssize_t
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
                (rename_utf8(image->filename,backup_filename) != 0))
              *backup_filename='\0';
          }
        /*
          Write transmogrified image to disk.
        */
        image_info->synchronize=MagickTrue;
        status&=WriteImages(image_info,image,image->filename,exception);
        if ((status == MagickFalse) && (*backup_filename != '\0'))
          (void) remove_utf8(backup_filename);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-resize",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("alpha",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickAlphaOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("bias",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blue-shift",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("box",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("brightness-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            ssize_t
              channel;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("charcoal",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("color-matrix",option+1) == 0)
          {
            KernelInfo
              *kernel_info;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i]);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            kernel_info=DestroyKernelInfo(kernel_info);
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("colorspace",option+1) == 0)
          {
            ssize_t
              colorspace;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
              argv[i]);
            if (colorspace < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("combine",option+1) == 0)
          {
            if (*option == '-')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("compare",option+1) == 0)
          break;
        if (LocaleCompare("complex",option+1) == 0)
          {
            ssize_t
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickComplexOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedComplexOperator",
                argv[i]);
            break;
          }
        if (LocaleCompare("composite",option+1) == 0)
          break;
        if (LocaleCompare("compress",option+1) == 0)
          {
            ssize_t
              compress;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            compress=ParseCommandOption(MagickCompressOptions,MagickFalse,
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("convolve",option+1) == 0)
          {
            KernelInfo
              *kernel_info;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i]);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            kernel_info=DestroyKernelInfo(kernel_info);
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("deconstruct",option+1) == 0)
          break;
        if (LocaleCompare("debug",option+1) == 0)
          {
            ssize_t
              event;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("delete",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("despeckle",option+1) == 0)
          break;
        if (LocaleCompare("dft",option+1) == 0)
          break;
        if (LocaleCompare("direction",option+1) == 0)
          {
            ssize_t
              direction;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            direction=ParseCommandOption(MagickDirectionOptions,MagickFalse,
              argv[i]);
            if (direction < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedDirectionType",
                argv[i]);
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("dispose",option+1) == 0)
          {
            ssize_t
              dispose;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,
              argv[i]);
            if (dispose < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("distort",option+1) == 0)
          {
            ssize_t
              op;

            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickDistortOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedDistortMethod",
                argv[i]);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            ssize_t
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickDitherOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("duplicate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("endian",option+1) == 0)
          {
            ssize_t
              endian;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            endian=ParseCommandOption(MagickEndianOptions,MagickFalse,argv[i]);
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
            ssize_t
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickEvaluateOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedEvaluateOperator",
                argv[i]);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("evaluate-sequence",option+1) == 0)
          {
            ssize_t
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickEvaluateOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("features",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("fill",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("filter",option+1) == 0)
          {
            ssize_t
              filter;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            filter=ParseCommandOption(MagickFilterOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("function",option+1) == 0)
          {
            ssize_t
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickFunctionOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedFunction",argv[i]);
             i++;
             if (i == (ssize_t) (argc-1))
               ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("fuzz",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if ((LocaleCompare("gaussian-blur",option+1) == 0) ||
            (LocaleCompare("gaussian",option+1) == 0))
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            ssize_t
              gravity;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            break;
          }
        if (LocaleCompare("grayscale",option+1) == 0)
          {
            ssize_t
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickPixelIntensityOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedIntensityMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("intensity",option+1) == 0)
          {
            ssize_t
              intensity;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            intensity=ParseCommandOption(MagickPixelIntensityOptions,
              MagickFalse,argv[i]);
            if (intensity < 0)
              ThrowMogrifyException(OptionError,
                "UnrecognizedPixelIntensityMethod",argv[i]);
            break;
          }
        if (LocaleCompare("intent",option+1) == 0)
          {
            ssize_t
              intent;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            intent=ParseCommandOption(MagickIntentOptions,MagickFalse,argv[i]);
            if (intent < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedIntentType",
                argv[i]);
            break;
          }
        if (LocaleCompare("interlace",option+1) == 0)
          {
            ssize_t
              interlace;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
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
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("interpolate",option+1) == 0)
          {
            ssize_t
              interpolate;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("lat",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
          }
        if (LocaleCompare("layers",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickLayerOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedLayerMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("level",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("level-colors",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("linewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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

            ssize_t
              resource;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("liquid-rescale",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("list",option+1) == 0)
          {
            ssize_t
              list;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedListType",argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            return(status != 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("loop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMogrifyException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("magnify",option+1) == 0)
          break;
        if (LocaleCompare("map",option+1) == 0)
          {
            global_colormap=(*option == '+') ? MagickTrue : MagickFalse;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("metric",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickMetricOptions,MagickTrue,argv[i]);
            if (type < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedMetricType",
                argv[i]);
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("mode",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("morphology",option+1) == 0)
          {
            char
              token[MaxTextExtent];

            KernelInfo
              *kernel_info;

            ssize_t
              op;

            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            GetMagickToken(argv[i],NULL,token);
            op=ParseCommandOption(MagickMorphologyOptions,MagickFalse,token);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedMorphologyMethod",
                token);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i]);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            kernel_info=DestroyKernelInfo(kernel_info);
            break;
          }
        if (LocaleCompare("mosaic",option+1) == 0)
          break;
        if (LocaleCompare("motion-blur",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                ssize_t
                  noise;

                noise=ParseCommandOption(MagickNoiseOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("ordered-dither",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("orient",option+1) == 0)
          {
            ssize_t
              orientation;

            orientation=UndefinedOrientation;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            orientation=ParseCommandOption(MagickOrientationOptions,MagickFalse,
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("paint",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            (void) CloneString(&path,argv[i]);
            break;
          }
        if (LocaleCompare("perceptible",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("poly",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("quantize",option+1) == 0)
          {
            ssize_t
              colorspace;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("raise",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("recolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
          }
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("region",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("remap",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("render",option+1) == 0)
          break;
        if (LocaleCompare("repage",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parenthesis=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("reverse",option+1) == 0)
          break;
        if (LocaleCompare("roll",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("selective-blur",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("shade",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shear",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("smush",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            i++;
            break;
          }
        if (LocaleCompare("solarize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sparse-color",option+1) == 0)
          {
            ssize_t
              op;

            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickSparseColorOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedSparseColorMethod",
                argv[i]);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("splice",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("spread",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("statistic",option+1) == 0)
          {
            ssize_t
              op;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickStatisticOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedStatisticType",
                argv[i]);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("stretch",option+1) == 0)
          {
            ssize_t
              stretch;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            stretch=ParseCommandOption(MagickStretchOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("style",option+1) == 0)
          {
            ssize_t
              style;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            style=ParseCommandOption(MagickStyleOptions,MagickFalse,argv[i]);
            if (style < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedStyleType",
                argv[i]);
            break;
          }
        if (LocaleCompare("swap",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("swirl",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("synchronize",option+1) == 0)
          break;
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("tile-offset",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("transparent",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("trim",option+1) == 0)
          break;
        if (LocaleCompare("type",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickTypeOptions,MagickFalse,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("unique-colors",option+1) == 0)
          break;
        if (LocaleCompare("units",option+1) == 0)
          {
            ssize_t
              units;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            units=ParseCommandOption(MagickResolutionOptions,MagickFalse,
              argv[i]);
            if (units < 0)
              ThrowMogrifyException(OptionError,"UnrecognizedUnitsType",
                argv[i]);
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
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
            ListMagickVersion(stdout);
            break;
          }
        if (LocaleCompare("view",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("vignette",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("virtual-pixel",option+1) == 0)
          {
            ssize_t
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
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
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) (argc-1))
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
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
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMogrifyInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowMogrifyException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i != (ssize_t) argc)
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

  ssize_t
    count;

  register ssize_t
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
  for (i=0; i < (ssize_t) argc; i++)
  {
    option=argv[i];
    if (IsCommandOption(option) == MagickFalse)
      continue;
    count=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
    count=MagickMax(count,0L);
    if ((i+count) >= (ssize_t) argc)
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
                (void) QueryColorDatabase(MogrifyBackgroundColor,
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
                (void) QueryColorDatabase(MogrifyBorderColor,
                  &image_info->border_color,exception);
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
              limit=(MagickSizeType) SiPrefixToDoubleInterval(argv[i+1],100.0);
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
            image_info->colorspace=(ColorspaceType) ParseCommandOption(
              MagickColorspaceOptions,MagickFalse,argv[i+1]);
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
            image_info->compression=(CompressionType) ParseCommandOption(
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
        if (LocaleCompare("direction",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
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
                (void) SetImageOption(image_info,option+1,"none");
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
            image_info->endian=(EndianType) ParseCommandOption(
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
              if (strchr("Agkrz@[#",*(q+1)) != (char *) NULL)
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
            image_info->fuzz=StringToDoubleInterval(argv[i+1],(double)
              QuantumRange+1.0);
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
        if (LocaleCompare("intensity",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) SetImageOption(image_info,option+1,"undefined");
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            break;
          }
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
            image_info->interlace=(InterlaceType) ParseCommandOption(
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
            type=(ResourceType) ParseCommandOption(MagickResourceOptions,
              MagickFalse,argv[i+1]);
            limit=MagickResourceInfinity;
            if (LocaleCompare("unlimited",argv[i+2]) != 0)
              limit=(MagickSizeType) SiPrefixToDoubleInterval(argv[i+2],100.0);
            (void) SetMagickResourceLimit(type,limit);
            break;
          }
        if (LocaleCompare("list",option+1) == 0)
          {
            ssize_t
              list;

            /*
              Display configuration list.
            */
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i+1]);
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
                (void) QueryColorDatabase(MogrifyMatteColor,
                  &image_info->matte_color,exception);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
            (void) QueryColorDatabase(argv[i+1],&image_info->matte_color,
              exception);
            break;
          }
        if (LocaleCompare("metric",option+1) == 0)
          {
            if (*option == '+')
              {
                (void) DeleteImageOption(image_info,option+1);
                break;
              }
            (void) SetImageOption(image_info,option+1,argv[i+1]);
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
            image_info->orientation=(OrientationType) ParseCommandOption(
              MagickOrientationOptions,MagickFalse,argv[i+1]);
            (void) SetImageOption(image_info,option+1,argv[i+1]);
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
            (void) FormatLocaleString(page,MaxTextExtent,"%lux%lu",
              (unsigned long) geometry.width,(unsigned long) geometry.height);
            if (((flags & XValue) != 0) || ((flags & YValue) != 0))
              (void) FormatLocaleString(page,MaxTextExtent,"%lux%lu%+ld%+ld",
                (unsigned long) geometry.width,(unsigned long) geometry.height,
                (long) geometry.x,(long) geometry.y);
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
            image_info->preview_type=(PreviewType) ParseCommandOption(
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
            size_t
              seed;

            if (*option == '+')
              {
                seed=(size_t) time((time_t *) NULL);
                SetRandomSecretKey(seed);
                break;
              }
            seed=StringToUnsignedLong(argv[i+1]);
            SetRandomSecretKey(seed);
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
        if (LocaleCompare("synchronize",option+1) == 0)
          {
            if (*option == '+')
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
            image_info->type=(ImageType) ParseCommandOption(MagickTypeOptions,
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
            image_info->units=(ResolutionType) ParseCommandOption(
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
              ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
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
%    o images: pointer to pointer of the first image in image list.
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

  ImageInfo
    *mogrify_info;

  MagickStatusType
    status;

  QuantizeInfo
    *quantize_info;

  register ssize_t
    i;

  ssize_t
    count,
    index;

  /*
    Apply options to the image list.
  */
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
  mogrify_info=CloneImageInfo(image_info);
  quantize_info=AcquireQuantizeInfo(mogrify_info);
  channel=mogrify_info->channel;
  status=MagickTrue;
  for (i=0; i < (ssize_t) argc; i++)
  {
    if (*images == (Image *) NULL)
      break;
    option=argv[i];
    if (IsCommandOption(option) == MagickFalse)
      continue;
    count=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
    count=MagickMax(count,0L);
    if ((i+count) >= (ssize_t) argc)
      break;
    status=MogrifyImageInfo(mogrify_info,(int) count+1,argv+i,exception);
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("affinity",option+1) == 0)
          {
            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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
            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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
        if (LocaleCompare("compare",option+1) == 0)
          {
            const char
              *option;

            double
              distortion;

            Image
              *difference_image,
              *image,
              *reconstruct_image;

            MetricType
              metric;

            /*
              Mathematically and visually annotate the difference between an
              image and its reconstruction.
            */
            (void) SyncImagesSettings(mogrify_info,*images);
            image=RemoveFirstImageFromList(images);
            reconstruct_image=RemoveFirstImageFromList(images);
            if (reconstruct_image == (Image *) NULL)
              {
                status=MagickFalse;
                break;
              }
            metric=UndefinedMetric;
            option=GetImageOption(image_info,"metric");
            if (option != (const char *) NULL)
              metric=(MetricType) ParseCommandOption(MagickMetricOptions,
                MagickFalse,option);
            difference_image=CompareImageChannels(image,reconstruct_image,
              channel,metric,&distortion,exception);
            if (difference_image == (Image *) NULL)
              break;
            if (*images != (Image *) NULL)
              *images=DestroyImage(*images);
            *images=difference_image;
            break;
          }
        if (LocaleCompare("complex",option+1) == 0)
          {
            ComplexOperator
              op;

            Image
              *complex_images;

            (void) SyncImageSettings(mogrify_info,*images);
            op=(ComplexOperator) ParseCommandOption(MagickComplexOptions,
              MagickFalse,argv[i+1]);
            complex_images=ComplexImages(*images,op,exception);
            if (complex_images == (Image *) NULL)
              {
                status=MagickFalse;
                break;
              }
            *images=DestroyImageList(*images);
            *images=complex_images;
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

            (void) SyncImagesSettings(mogrify_info,*images);
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
                    /* POSIBLE ERROR; what if image->mask already set */
                    image->mask=mask_image;
                    (void) NegateImage(image->mask,MagickFalse);
                  }
              }
            (void) CompositeImageChannel(image,channel,image->compose,
              composite_image,geometry.x,geometry.y);
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
        if (LocaleCompare("deconstruct",option+1) == 0)
          {
            Image
              *deconstruct_image;

            (void) SyncImagesSettings(mogrify_info,*images);
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
            quantize_info->dither_method=(DitherMethod) ParseCommandOption(
              MagickDitherOptions,MagickFalse,argv[i+1]);
            break;
          }
        if (LocaleCompare("duplicate",option+1) == 0)
          {
            Image
              *duplicate_images;

            if (*option == '+')
              duplicate_images=DuplicateImages(*images,1,"-1",exception);
            else
              {
                const char
                  *p;

                size_t
                  number_duplicates;

                number_duplicates=(size_t) StringToLong(argv[i+1]);
                p=strchr(argv[i+1],',');
                if (p == (const char *) NULL)
                  duplicate_images=DuplicateImages(*images,number_duplicates,
                    "-1",exception);
                else
                  duplicate_images=DuplicateImages(*images,number_duplicates,p,
                    exception);
              }
            AppendImageToList(images, duplicate_images);
            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImageSettings(mogrify_info,*images);
            op=(MagickEvaluateOperator) ParseCommandOption(
              MagickEvaluateOptions,MagickFalse,argv[i+1]);
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
            (void) SyncImageSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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
            (void) SyncImagesSettings(mogrify_info,*images);
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
              index=(ssize_t) StringToLong(argv[i+1]);
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
              if (index == (ssize_t) GetImageListLength(*images))
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

            (void) SyncImagesSettings(mogrify_info,*images);
            layers=(Image *) NULL;
            method=(ImageLayerMethod) ParseCommandOption(MagickLayerOptions,
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
                option=GetImageOption(mogrify_info,"compose");
                if (option != (const char *) NULL)
                  compose=(CompositeOperator) ParseCommandOption(
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
            (void) SyncImagesSettings(mogrify_info,*images);
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
            (void) SyncImagesSettings(mogrify_info,*images);
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
            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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

            (void) SyncImagesSettings(mogrify_info,*images);
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
        if (LocaleCompare("poly",option+1) == 0)
          {
            char
              *args,
              token[MaxTextExtent];

            const char
              *p;

            double
              *arguments;

            Image
              *polynomial_image;

            register ssize_t
              x;

            size_t
              number_arguments;

            /*
              Polynomial image.
            */
            (void) SyncImageSettings(mogrify_info,*images);
            args=InterpretImageProperties(mogrify_info,*images,argv[i+1]);
            InheritException(exception,&(*images)->exception);
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
                "MemoryAllocationFailed",(*images)->filename);
            (void) ResetMagickMemory(arguments,0,number_arguments*
              sizeof(*arguments));
            p=(char *) args;
            for (x=0; (x < (ssize_t) number_arguments) && (*p != '\0'); x++)
            {
              GetMagickToken(p,&p,token);
              if (*token == ',')
                GetMagickToken(p,&p,token);
              arguments[x]=StringToDouble(token,(char **) NULL);
            }
            args=DestroyString(args);
            polynomial_image=PolynomialImageChannel(*images,channel,
              number_arguments >> 1,arguments,exception);
            arguments=(double *) RelinquishMagickMemory(arguments);
            if (polynomial_image == (Image *) NULL)
              {
                status=MagickFalse;
                break;
              }
            *images=DestroyImageList(*images);
            *images=polynomial_image;
            break;
          }
        if (LocaleCompare("print",option+1) == 0)
          {
            char
              *string;

            (void) SyncImagesSettings(mogrify_info,*images);
            string=InterpretImageProperties(mogrify_info,*images,argv[i+1]);
            if (string == (char *) NULL)
              break;
            InheritException(exception,&(*images)->exception);
            (void) FormatLocaleFile(stdout,"%s",string);
            string=DestroyString(string);
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            char
              **arguments;

            int
              j,
              number_arguments;

            (void) SyncImagesSettings(mogrify_info,*images);
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
                if (~length >= (MaxTextExtent-1))
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
        if (LocaleCompare("smush",option+1) == 0)
          {
            Image
              *smush_image;

            ssize_t
              offset;

            (void) SyncImagesSettings(mogrify_info,*images);
            offset=(ssize_t) StringToLong(argv[i+1]);
            smush_image=SmushImages(*images,*option == '-' ? MagickTrue :
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
        if (LocaleCompare("swap",option+1) == 0)
          {
            Image
              *p,
              *q,
              *swap;

            ssize_t
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
        if (LocaleCompare("write",option+1) == 0)
          {
            char
              key[MaxTextExtent];

            Image
              *write_images;

            ImageInfo
              *write_info;

            (void) SyncImagesSettings(mogrify_info,*images);
            (void) FormatLocaleString(key,MaxTextExtent,"cache:%s",argv[i+1]);
            (void) DeleteImageRegistry(key);
            write_images=(*images);
            if (*option == '+')
              write_images=CloneImageList(*images,exception);
            write_info=CloneImageInfo(mogrify_info);
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
  mogrify_info=DestroyImageInfo(mogrify_info);
  status&=MogrifyImageInfo(image_info,argc,argv,exception);
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
%    o images: pointer to a pointer of the first image in image list.
%
%    o exception: return any errors or warnings in this structure.
%
*/
WandExport MagickBooleanType MogrifyImages(ImageInfo *image_info,
  const MagickBooleanType post,const int argc,const char **argv,
  Image **images,ExceptionInfo *exception)
{
#define MogrifyImageTag  "Mogrify/Image"

  MagickStatusType
    status;

  MagickBooleanType
    proceed;

  size_t
    n;

  register ssize_t
    i;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (images == (Image **) NULL)
    return(MogrifyImage(image_info,argc,argv,images,exception));
  assert((*images)->previous == (Image *) NULL);
  assert((*images)->signature == MagickSignature);
  if ((*images)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      (*images)->filename);
  if ((argc <= 0) || (*argv == (char *) NULL))
    return(MagickTrue);
  (void) SetImageInfoProgressMonitor(image_info,(MagickProgressMonitor) NULL,
    (void *) NULL);
  status=0;

#if 0
  (void) FormatLocaleFile(stderr, "mogrify start %s %d (%s)\n",argv[0],argc,
    post?"post":"pre");
#endif

  /*
    Pre-process multi-image sequence operators
  */
  if (post == MagickFalse)
    status&=MogrifyImageList(image_info,argc,argv,images,exception);
  /*
    For each image, process simple single image operators
  */
  i=0;
  n=GetImageListLength(*images);
  for (;;)
  {
#if 0
  (void) FormatLocaleFile(stderr,"mogrify %ld of %ld\n",(long)
    GetImageIndexInList(*images),(long)GetImageListLength(*images));
#endif
    status&=MogrifyImage(image_info,argc,argv,images,exception);
    proceed=SetImageProgress(*images,MogrifyImageTag,(MagickOffsetType) i, n);
    if (proceed == MagickFalse)
      break;
    if ( (*images)->next == (Image *) NULL )
      break;
    *images=(*images)->next;
    i++;
  }
  assert( *images != (Image *) NULL );
#if 0
  (void) FormatLocaleFile(stderr,"mogrify end %ld of %ld\n",(long)
    GetImageIndexInList(*images),(long)GetImageListLength(*images));
#endif

  /*
    Post-process, multi-image sequence operators
  */
  *images=GetFirstImageInList(*images);
  if (post != MagickFalse)
    status&=MogrifyImageList(image_info,argc,argv,images,exception);
  return(status != 0 ? MagickTrue : MagickFalse);
}
