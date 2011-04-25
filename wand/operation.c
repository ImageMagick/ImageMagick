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
%                                April 2011                                   %
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
  Constant declaration. (temporary exports)
*/
MagickExport const char
  BackgroundColor[] = "#fff",  /* white */
  BorderColor[] = "#dfdfdf",  /* gray */
  MatteColor[] = "#bdbdbd";  /* gray */

/*
** Function to report on the progress of image operations
*/
MagickExport MagickBooleanType MonitorProgress(const char *text,
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

/*
** SparseColorOption() parse the complex -sparse-color argument into an
** an array of floating point values than call SparseColorImage().
** Argument is a complex mix of floating-point pixel coodinates, and color
** specifications (or direct floating point numbers).  The number of floats
** needed to represent a color varies depending on teh current channel
** setting.
*/
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

  register size_t
    x;

  size_t
    number_arguments;

  size_t
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


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     S i m p l e O p e r a t i o n I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SimpleOperationImage() apply one simple operation on one image.
%  The image however may be part of a longer list of images.
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
%  The format of the SimpleOperationImage method is:
%
%    MagickBooleanType SimpleOperationImage(ImageInfo *image_info,
%        const int argc,const char **argv,Image **image)
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

MagickExport MagickBooleanType SimpleOperationImage(ImageInfo *image_info,
     const int wand_unused(argc), const char **argv,Image **image,
     ExceptionInfo *exception)
{
  Image *
    new_image;

  ChannelType
    channel;

  const char
    *format,
    *option;

  DrawInfo
    *draw_info;

  GeometryInfo
    geometry_info;

  RectangleInfo
    geometry;

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

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);

  /* why clone? -- Can't we just use the info struct more directly? */
  mogrify_info=CloneImageInfo(image_info);
  draw_info=CloneDrawInfo(mogrify_info,(DrawInfo *) NULL);
  quantize_info=AcquireQuantizeInfo(mogrify_info);
  SetGeometryInfo(&geometry_info);
  GetMagickPixelPacket(*image,&fill);
  SetMagickPixelPacket(*image,&(*image)->background_color,(IndexPacket *) NULL,
    &fill);
  channel=mogrify_info->channel;
  format=GetImageOption(mogrify_info,"format");

  option=argv[0];
  new_image = (Image *)NULL;

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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=AdaptiveBlurImageChannel(*image,channel,
            geometry_info.rho,geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("adaptive-resize",option+1) == 0)
        {
          /*
            Adaptive resize image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=AdaptiveResizeImage(*image,geometry.width,
            geometry.height,exception);
          break;
        }
      if (LocaleCompare("adaptive-sharpen",option+1) == 0)
        {
          /*
            Adaptive sharpen image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=AdaptiveSharpenImageChannel(*image,channel,
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
          (void) ParseAffineGeometry(argv[1],&draw_info->affine,exception);
          break;
        }
      if (LocaleCompare("alpha",option+1) == 0)
        {
          AlphaChannelType
            alpha_type;

          (void) SyncImageSettings(mogrify_info,*image);
          alpha_type=(AlphaChannelType) ParseCommandOption(MagickAlphaOptions,
            MagickFalse,argv[1]);
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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          text=InterpretImageProperties(mogrify_info,*image,argv[2]);
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
      if (LocaleCompare("black-threshold",option+1) == 0)
        {
          /*
            Black threshold image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) BlackThresholdImageChannel(*image,channel,argv[1],
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
            flags=ParseGeometry(argv[1],&geometry_info);
          new_image=BlueShiftImage(*image,geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("blur",option+1) == 0)
        {
          /*
            Gaussian blur image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=BlurImageChannel(*image,channel,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("border",option+1) == 0)
        {
          /*
            Surround image with a border of solid color.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
          if ((flags & SigmaValue) == 0)
            geometry.height=geometry.width;
          new_image=BorderImage(*image,&geometry,exception);
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
          (void) QueryColorDatabase(argv[1],&draw_info->border_color,
            exception);
          break;
        }
      if (LocaleCompare("box",option+1) == 0)
        {
          (void) QueryColorDatabase(argv[1],&draw_info->undercolor,
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
          flags=ParseGeometry(argv[1],&geometry_info);
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
          color_correction_collection=FileToString(argv[1],~0,exception);
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
              channel=(ChannelType) ParseChannelOption(argv[1]);
          break;
        }
      if (LocaleCompare("charcoal",option+1) == 0)
        {
          /*
            Charcoal image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=CharcoalImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("chop",option+1) == 0)
        {
          /*
            Chop the image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseGravityGeometry(*image,argv[1],&geometry,exception);
          new_image=ChopImage(*image,&geometry,exception);
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
          mask_image=GetImageCache(mogrify_info,argv[1],exception);
          if (mask_image == (Image *) NULL)
            break;
          if (SetImageStorageClass(mask_image,DirectClass) == MagickFalse)
            return(MagickFalse);
          mask_view=AcquireCacheView(mask_image);
          for (y=0; y < (ssize_t) mask_image->rows; y++)
          {
            q=GetCacheViewAuthenticPixels(mask_view,0,y,mask_image->columns,1,
              exception);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (ssize_t) mask_image->columns; x++)
            {
              if (mask_image->matte == MagickFalse)
                q->opacity=PixelIntensityToQuantum(q);
              q->red=q->opacity;
              q->green=q->opacity;
              q->blue=q->opacity;
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
          (void) ClipImagePath(*image,argv[1],*option == '-' ? MagickTrue :
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
          new_image=ColorizeImage(*image,argv[1],draw_info->fill,
            exception);
          break;
        }
      if (LocaleCompare("color-matrix",option+1) == 0)
        {
          KernelInfo
            *kernel;

          (void) SyncImageSettings(mogrify_info,*image);
          kernel=AcquireKernelInfo(argv[1]);
          if (kernel == (KernelInfo *) NULL)
            break;
          new_image=ColorMatrixImage(*image,kernel,exception);
          kernel=DestroyKernelInfo(kernel);
          break;
        }
      if (LocaleCompare("colors",option+1) == 0)
        {
          /*
            Reduce the number of colors in the image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          quantize_info->number_colors=StringToUnsignedLong(argv[1]);
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
              (void) TransformImageColorspace(*image,RGBColorspace);
              InheritException(exception,&(*image)->exception);
              break;
            }
          colorspace=(ColorspaceType) ParseCommandOption(
            MagickColorspaceOptions,MagickFalse,argv[1]);
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
          kernel=AcquireKernelInfo(argv[1]);
          if (kernel == (KernelInfo *) NULL)
            break;
          gamma=0.0;
          for (j=0; j < (ssize_t) (kernel->width*kernel->height); j++)
            gamma+=kernel->values[j];
          gamma=1.0/(fabs((double) gamma) <= MagickEpsilon ? 1.0 : gamma);
          for (j=0; j < (ssize_t) (kernel->width*kernel->height); j++)
            kernel->values[j]*=gamma;
          new_image=FilterImageChannel(*image,channel,kernel,exception);
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
          flags=ParseGravityGeometry(*image,argv[1],&geometry,exception);
          if (((geometry.width != 0) || (geometry.height != 0)) &&
              ((flags & XValue) == 0) && ((flags & YValue) == 0))
            break;
#endif
#if 0
          new_image=CloneImage(*image,0,0,MagickTrue,&(*image)->exception);
          new_image->next = new_image->previous = (Image *)NULL;
          (void) TransformImage(&new_image,argv[1],(char *) NULL);
          InheritException(exception,&new_image->exception);
#else
          new_image=CropImageToTiles(*image,argv[1],exception);
#endif
          break;
        }
      if (LocaleCompare("cycle",option+1) == 0)
        {
          /*
            Cycle an image colormap.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) CycleColormapImage(*image,(ssize_t) StringToLong(argv[1]));
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
          passkey=FileToStringInfo(argv[1],~0,exception);
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
          (void) CloneString(&draw_info->density,argv[1]);
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
          (void) SetImageDepth(*image,StringToUnsignedLong(argv[1]));
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
            threshold=SiPrefixToDouble(argv[1],QuantumRange);
          new_image=DeskewImage(*image,threshold,exception);
          break;
        }
      if (LocaleCompare("despeckle",option+1) == 0)
        {
          /*
            Reduce the speckles within an image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=DespeckleImage(*image,exception);
          break;
        }
      if (LocaleCompare("display",option+1) == 0)
        {
          (void) CloneString(&draw_info->server_name,argv[1]);
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
          args=InterpretImageProperties(mogrify_info,*image,argv[2]);
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
            arguments[x]=StringToDouble(token);
          }
          args=DestroyString(args);
          new_image=DistortImage(*image,method,number_arguments,arguments,
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
            MagickDitherOptions,MagickFalse,argv[1]);
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
          (void) CloneString(&draw_info->primitive,argv[1]);
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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=EdgeImage(*image,geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("emboss",option+1) == 0)
        {
          /*
            Gaussian embossen image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=EmbossImage(*image,geometry_info.rho,
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
          passkey=FileToStringInfo(argv[1],~0,exception);
          if (passkey != (StringInfo *) NULL)
            {
              (void) PasskeyEncipherImage(*image,passkey,exception);
              passkey=DestroyStringInfo(passkey);
            }
          break;
        }
      if (LocaleCompare("encoding",option+1) == 0)
        {
          (void) CloneString(&draw_info->encoding,argv[1]);
          break;
        }
      if (LocaleCompare("enhance",option+1) == 0)
        {
          /*
            Enhance image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=EnhanceImage(*image,exception);
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
          op=(MagickEvaluateOperator) ParseCommandOption(MagickEvaluateOptions,
            MagickFalse,argv[1]);
          constant=SiPrefixToDouble(argv[2],QuantumRange);
          (void) EvaluateImageChannel(*image,channel,op,constant,exception);
          break;
        }
      if (LocaleCompare("extent",option+1) == 0)
        {
          /*
            Set the image extent.
          */
          (void) SyncImageSettings(mogrify_info,*image);
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
      if (LocaleCompare("family",option+1) == 0)
        {
          if (*option == '+')
            {
              if (draw_info->family != (char *) NULL)
                draw_info->family=DestroyString(draw_info->family);
              break;
            }
          (void) CloneString(&draw_info->family,argv[1]);
          break;
        }
      if (LocaleCompare("features",option+1) == 0)
        {
          if (*option == '+')
            {
              (void) DeleteImageArtifact(*image,"identify:features");
              break;
            }
          (void) SetImageArtifact(*image,"identify:features",argv[1]);
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
          (void) QueryMagickColor(argv[1],&fill,sans);
          status=QueryColorDatabase(argv[1],&draw_info->fill,sans);
          sans=DestroyExceptionInfo(sans);
          if (status == MagickFalse)
            draw_info->fill_pattern=GetImageCache(mogrify_info,argv[1],
              exception);
          break;
        }
      if (LocaleCompare("flip",option+1) == 0)
        {
          /*
            Flip image scanlines.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=FlipImage(*image,exception);
          break;
        }
      if (LocaleCompare("flop",option+1) == 0)
        {
          /*
            Flop image scanlines.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=FlopImage(*image,exception);
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
          (void) ParsePageGeometry(*image,argv[1],&geometry,exception);
          (void) QueryMagickColor(argv[2],&target,exception);
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
          (void) CloneString(&draw_info->font,argv[1]);
          break;
        }
      if (LocaleCompare("format",option+1) == 0)
        {
          format=argv[1];
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
          new_image=FrameImage(*image,&frame_info,exception);
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
            MagickFalse,argv[1]);
          arguments=InterpretImageProperties(mogrify_info,*image,argv[2]);
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
          (void) SyncImageSettings(mogrify_info,*image);
          if (*option == '+')
            (*image)->gamma=StringToDouble(argv[1]);
          else
            {
              if (strchr(argv[1],',') != (char *) NULL)
                (void) GammaImage(*image,argv[1]);
              else
                (void) GammaImageChannel(*image,channel,
                  StringToDouble(argv[1]));
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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=GaussianBlurImageChannel(*image,channel,
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
          flags=ParseRegionGeometry(*image,argv[1],&geometry,exception);
          if (((flags & XValue) != 0) || ((flags & YValue) != 0))
            (void) CloneString(&(*image)->geometry,argv[1]);
          else
            new_image=ResizeImage(*image,geometry.width,geometry.height,
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
            MagickGravityOptions,MagickFalse,argv[1]);
          break;
        }
      break;
    }
    case 'h':
    {
      if (LocaleCompare("highlight-color",option+1) == 0)
        {
          (void) SetImageArtifact(*image,option+1,argv[1]);
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
          (void) fputc('\n',stdout);
          text=DestroyString(text);
          break;
        }
      if (LocaleCompare("implode",option+1) == 0)
        {
          /*
            Implode image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=ImplodeImage(*image,geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("interline-spacing",option+1) == 0)
        {
          if (*option == '+')
            (void) ParseGeometry("0",&geometry_info);
          else
            (void) ParseGeometry(argv[1],&geometry_info);
          draw_info->interline_spacing=geometry_info.rho;
          break;
        }
      if (LocaleCompare("interword-spacing",option+1) == 0)
        {
          if (*option == '+')
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
      if (LocaleCompare("kerning",option+1) == 0)
        {
          if (*option == '+')
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
      if (LocaleCompare("lat",option+1) == 0)
        {
          /*
            Local adaptive threshold image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=AdaptiveThresholdImage(*image,(size_t)
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

          p=(const char *) argv[1];
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
          (void) LinearStretchImage(*image,black_point,white_point);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("linewidth",option+1) == 0)
        {
          draw_info->stroke_width=StringToDouble(argv[1]);
          break;
        }
      if (LocaleCompare("liquid-rescale",option+1) == 0)
        {
          /*
            Liquid rescale image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseRegionGeometry(*image,argv[1],&geometry,exception);
          if ((flags & XValue) == 0)
            geometry.x=1;
          if ((flags & YValue) == 0)
            geometry.y=0;
          new_image=LiquidRescaleImage(*image,geometry.width,
            geometry.height,1.0*geometry.x,1.0*geometry.y,exception);
          break;
        }
      if (LocaleCompare("lowlight-color",option+1) == 0)
        {
          (void) SetImageArtifact(*image,option+1,argv[1]);
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
          (void) SyncImageSettings(mogrify_info,*image);
          if (*option == '+')
            break;
          remap_image=GetImageCache(mogrify_info,argv[1],exception);
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
          mask=GetImageCache(mogrify_info,argv[1],exception);
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
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=StatisticImageChannel(*image,channel,MedianStatistic,
            (size_t) geometry_info.rho,(size_t) geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("mode",option+1) == 0)
        {
          /*
            Mode image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=StatisticImageChannel(*image,channel,ModeStatistic,
            (size_t) geometry_info.rho,(size_t) geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("modulate",option+1) == 0)
        {
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ModulateImage(*image,argv[1]);
          InheritException(exception,&(*image)->exception);
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
          p=argv[1];
          GetMagickToken(p,&p,token);
          method=(MorphologyMethod) ParseCommandOption(MagickMorphologyOptions,
            MagickFalse,token);
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
          new_image=MorphologyImageChannel(*image,channel,method,
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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=MotionBlurImageChannel(*image,channel,
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
              (void) ParseGeometry(argv[1],&geometry_info);
              new_image=StatisticImageChannel(*image,channel,
                NonpeakStatistic,(size_t) geometry_info.rho,(size_t)
                geometry_info.rho,exception);
            }
          else
            {
              NoiseType
                noise;

              noise=(NoiseType) ParseCommandOption(MagickNoiseOptions,
                MagickFalse,argv[1]);
              new_image=AddNoiseImageChannel(*image,channel,noise,
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
          (void) QueryMagickColor(argv[1],&target,exception);
          (void) OpaquePaintImageChannel(*image,channel,&target,&fill,
            *option == '-' ? MagickFalse : MagickTrue);
          break;
        }
      if (LocaleCompare("ordered-dither",option+1) == 0)
        {
          (void) SyncImageSettings(mogrify_info,*image);
          (void) OrderedPosterizeImageChannel(*image,channel,argv[1],
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
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
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
          (void) QueryColorDatabase(argv[1],&draw_info->fill,exception);
          break;
        }
      if (LocaleCompare("pointsize",option+1) == 0)
        {
          if (*option == '+')
            (void) ParseGeometry("12",&geometry_info);
          else
            (void) ParseGeometry(argv[1],&geometry_info);
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
              flags=ParseGeometry(argv[1],&geometry_info);
              angle=geometry_info.rho;
            }
          new_image=PolaroidImage(*image,draw_info,angle,exception);
          break;
        }
      if (LocaleCompare("posterize",option+1) == 0)
        {
          /*
            Posterize image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) PosterizeImage(*image,StringToUnsignedLong(argv[1]),
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
            preview_type=(PreviewType) ParseCommandOption(MagickPreviewOptions,
              MagickFalse,argv[1]);
          new_image=PreviewImage(*image,preview_type,exception);
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
              (void) ProfileImage(*image,argv[1],(const unsigned char *)
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
          profile_image=GetImageCache(profile_info,argv[1],exception);
          profile_info=DestroyImageInfo(profile_info);
          if (profile_image == (Image *) NULL)
            {
              StringInfo
                *profile;

              profile_info=CloneImageInfo(mogrify_info);
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
      if (LocaleCompare("quantize",option+1) == 0)
        {
          if (*option == '+')
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
      if (LocaleCompare("radial-blur",option+1) == 0)
        {
          /*
            Radial blur image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=RadialBlurImageChannel(*image,channel,
            StringToDouble(argv[1]),exception);
          break;
        }
      if (LocaleCompare("raise",option+1) == 0)
        {
          /*
            Surround image with a raise of solid color.
          */
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
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
          (void) SyncImageSettings(mogrify_info,*image);
          (void) RandomThresholdImageChannel(*image,channel,argv[1],
            exception);
          break;
        }
      if (LocaleCompare("recolor",option+1) == 0)
        {
          KernelInfo
            *kernel;

          (void) SyncImageSettings(mogrify_info,*image);
          kernel=AcquireKernelInfo(argv[1]);
          if (kernel == (KernelInfo *) NULL)
            break;
          new_image=ColorMatrixImage(*image,kernel,exception);
          kernel=DestroyKernelInfo(kernel);
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
          remap_image=GetImageCache(mogrify_info,argv[1],exception);
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
          (void) ResetImagePage(*image,argv[1]);
          InheritException(exception,&(*image)->exception);
          break;
        }
      if (LocaleCompare("resample",option+1) == 0)
        {
          /*
            Resample image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=ResampleImage(*image,geometry_info.rho,
            geometry_info.sigma,(*image)->filter,(*image)->blur,exception);
          break;
        }
      if (LocaleCompare("resize",option+1) == 0)
        {
          /*
            Resize image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=ResizeImage(*image,geometry.width,geometry.height,
            (*image)->filter,(*image)->blur,exception);
          break;
        }
      if (LocaleCompare("roll",option+1) == 0)
        {
          /*
            Roll image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParsePageGeometry(*image,argv[1],&geometry,exception);
          new_image=RollImage(*image,geometry.x,geometry.y,exception);
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
      if (LocaleCompare("sample",option+1) == 0)
        {
          /*
            Sample image with pixel replication.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=SampleImage(*image,geometry.width,geometry.height,
            exception);
          break;
        }
      if (LocaleCompare("scale",option+1) == 0)
        {
          /*
            Resize image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=ScaleImage(*image,geometry.width,geometry.height,
            exception);
          break;
        }
      if (LocaleCompare("selective-blur",option+1) == 0)
        {
          /*
            Selectively blur pixels within a contrast threshold.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & PercentValue) != 0)
            geometry_info.xi=(double) QuantumRange*geometry_info.xi/100.0;
          new_image=SelectiveBlurImageChannel(*image,channel,
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
          new_image=SeparateImages(*image,channel,exception);
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
          threshold=SiPrefixToDouble(argv[1],QuantumRange);
          new_image=SepiaToneImage(*image,threshold,exception);
          break;
        }
      if (LocaleCompare("segment",option+1) == 0)
        {
          /*
            Segment image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
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
              if (LocaleNCompare(argv[1],"registry:",9) == 0)
                (void) DeleteImageRegistry(argv[1]+9);
              else
                if (LocaleNCompare(argv[1],"option:",7) == 0)
                  {
                    (void) DeleteImageOption(mogrify_info,argv[1]+7);
                    (void) DeleteImageArtifact(*image,argv[1]+7);
                  }
                else
                  (void) DeleteImageProperty(*image,argv[1]);
              break;
            }
          value=InterpretImageProperties(mogrify_info,*image,argv[2]);
          if (value == (char *) NULL)
            break;
          if (LocaleNCompare(argv[1],"registry:",9) == 0)
            (void) SetImageRegistry(StringRegistryType,argv[1]+9,value,
              exception);
          else
            if (LocaleNCompare(argv[1],"option:",7) == 0)
              {
                (void) SetImageOption(image_info,argv[1]+7,value);
                (void) SetImageOption(mogrify_info,argv[1]+7,value);
                (void) SetImageArtifact(*image,argv[1]+7,value);
              }
            else
              (void) SetImageProperty(*image,argv[1],value);
          value=DestroyString(value);
          break;
        }
      if (LocaleCompare("shade",option+1) == 0)
        {
          /*
            Shade image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=ShadeImage(*image,(*option == '-') ? MagickTrue :
            MagickFalse,geometry_info.rho,geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("shadow",option+1) == 0)
        {
          /*
            Shadow image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
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
      if (LocaleCompare("sharpen",option+1) == 0)
        {
          /*
            Sharpen image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=SharpenImageChannel(*image,channel,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("shave",option+1) == 0)
        {
          /*
            Shave the image edges.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParsePageGeometry(*image,argv[1],&geometry,exception);
          new_image=ShaveImage(*image,&geometry,exception);
          break;
        }
      if (LocaleCompare("shear",option+1) == 0)
        {
          /*
            Shear image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=geometry_info.rho;
          new_image=ShearImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
        {
          /*
            Sigmoidal non-linearity contrast control.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=SketchImage(*image,geometry_info.rho,
            geometry_info.sigma,geometry_info.xi,exception);
          break;
        }
      if (LocaleCompare("solarize",option+1) == 0)
        {
          double
            threshold;

          (void) SyncImageSettings(mogrify_info,*image);
          threshold=SiPrefixToDouble(argv[1],QuantumRange);
          (void) SolarizeImage(*image,threshold);
          InheritException(exception,&(*image)->exception);
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
            MagickSparseColorOptions,MagickFalse,argv[1]);
          arguments=InterpretImageProperties(mogrify_info,*image,argv[2]);
          InheritException(exception,&(*image)->exception);
          if (arguments == (char *) NULL)
            break;
          new_image=SparseColorOption(*image,channel,method,arguments,
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
          (void) ParseGravityGeometry(*image,argv[1],&geometry,exception);
          new_image=SpliceImage(*image,&geometry,exception);
          break;
        }
      if (LocaleCompare("spread",option+1) == 0)
        {
          /*
            Spread an image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=SpreadImage(*image,geometry_info.rho,exception);
          break;
        }
      if (LocaleCompare("statistic",option+1) == 0)
        {
          StatisticType
            type;

          (void) SyncImageSettings(mogrify_info,*image);
          type=(StatisticType) ParseCommandOption(MagickStatisticOptions,
            MagickFalse,argv[1]);
          (void) ParseGeometry(argv[2],&geometry_info);
          new_image=StatisticImageChannel(*image,channel,type,(size_t)
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
            MagickStretchOptions,MagickFalse,argv[1]);
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
          status=QueryColorDatabase(argv[1],&draw_info->stroke,sans);
          sans=DestroyExceptionInfo(sans);
          if (status == MagickFalse)
            draw_info->stroke_pattern=GetImageCache(mogrify_info,argv[1],
              exception);
          break;
        }
      if (LocaleCompare("strokewidth",option+1) == 0)
        {
          draw_info->stroke_width=StringToDouble(argv[1]);
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
            MagickFalse,argv[1]);
          break;
        }
      if (LocaleCompare("swirl",option+1) == 0)
        {
          /*
            Swirl image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) ParseGeometry(argv[1],&geometry_info);
          new_image=SwirlImage(*image,geometry_info.rho,exception);
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
            threshold=(double) QuantumRange/2.5;
          else
            threshold=SiPrefixToDouble(argv[1],QuantumRange);
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
          (void) ParseRegionGeometry(*image,argv[1],&geometry,exception);
          new_image=ThumbnailImage(*image,geometry.width,geometry.height,
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
          draw_info->fill_pattern=GetImageCache(mogrify_info,argv[1],
            exception);
          break;
        }
      if (LocaleCompare("tint",option+1) == 0)
        {
          /*
            Tint the image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=TintImage(*image,argv[1],draw_info->fill,exception);
          break;
        }
      if (LocaleCompare("transform",option+1) == 0)
        {
          /*
            Affine transform image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=AffineTransformImage(*image,&draw_info->affine,
            exception);
          break;
        }
      if (LocaleCompare("transparent",option+1) == 0)
        {
          MagickPixelPacket
            target;

          (void) SyncImageSettings(mogrify_info,*image);
          (void) QueryMagickColor(argv[1],&target,exception);
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
          new_image=TransposeImage(*image,exception);
          break;
        }
      if (LocaleCompare("transverse",option+1) == 0)
        {
          /*
            Transverse image scanlines.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=TransverseImage(*image,exception);
          break;
        }
      if (LocaleCompare("treedepth",option+1) == 0)
        {
          quantize_info->tree_depth=StringToUnsignedLong(argv[1]);
          break;
        }
      if (LocaleCompare("trim",option+1) == 0)
        {
          /*
            Trim image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          new_image=TrimImage(*image,exception);
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
              argv[1]);
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
          (void) QueryColorDatabase(argv[1],&draw_info->undercolor,
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
          new_image=UniqueImageColors(*image,exception);
          break;
        }
      if (LocaleCompare("unsharp",option+1) == 0)
        {
          /*
            Unsharp mask image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          if ((flags & XiValue) == 0)
            geometry_info.xi=1.0;
          if ((flags & PsiValue) == 0)
            geometry_info.psi=0.05;
          new_image=UnsharpMaskImageChannel(*image,channel,
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
            argv[1]));
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
          flags=ParseGeometry(argv[1],&geometry_info);
          if ((flags & SigmaValue) == 0)
            geometry_info.sigma=1.0;
          new_image=WaveImage(*image,geometry_info.rho,
            geometry_info.sigma,exception);
          break;
        }
      if (LocaleCompare("weight",option+1) == 0)
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
      if (LocaleCompare("white-threshold",option+1) == 0)
        {
          /*
            White threshold image.
          */
          (void) SyncImageSettings(mogrify_info,*image);
          (void) WhiteThresholdImageChannel(*image,channel,argv[1],
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
  if (new_image != (Image *) NULL)
    ReplaceImageInListReturnLast(image,new_image);

  /*
    Free resources.
  */
  quantize_info=DestroyQuantizeInfo(quantize_info);
  draw_info=DestroyDrawInfo(draw_info);
  mogrify_info=DestroyImageInfo(mogrify_info);
  status=(*image)->exception.severity == UndefinedException ? 1 : 0;

  return(status);
}

