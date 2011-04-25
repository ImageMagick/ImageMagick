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
#include "wand/operation.h"
#include "magick/monitor-private.h"
#include "magick/thread-private.h"
#include "magick/string-private.h"

/*
  Define declarations.
*/
#define UndefinedCompressionQuality  0UL

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
    elapsed_time,
    user_time;

  MagickBooleanType
    concurrent,
    regard_warnings,
    status;

  register ssize_t
    i;

  TimerInfo
    *timer;

  size_t
    iterations;

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
    if (LocaleCompare("duration",option+1) == 0)
      duration=StringToDouble(argv[++i]);
    if (LocaleCompare("regard-warnings",option+1) == 0)
      regard_warnings=MagickTrue;
  }
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
        "Performance: %.20gi %gips %0.3fu %.20g:%02g.%03g\n",(double)
        iterations,1.0*iterations/elapsed_time,user_time,(double)
        (elapsed_time/60.0),floor(fmod(elapsed_time,60.0)),(double)
        (1000.0*(elapsed_time-floor(elapsed_time))));
      (void) fflush(stderr);
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
%  MogrifyImage() applies simple single image processing options to a single
%  image, but also hanles any 'region' image handling.
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

static MagickBooleanType IsPathWritable(const char *path)
{
  if (IsPathAccessible(path) == MagickFalse)
    return(MagickFalse);
  if (access(path,W_OK) != 0)
    return(MagickFalse);
  return(MagickTrue);
}

static inline ssize_t MagickMax(const ssize_t x,const ssize_t y)
{
  if (x > y)
    return(x);
  return(y);
}

WandExport MagickBooleanType MogrifyImage(ImageInfo *image_info,const int argc,
  const char **argv,Image **image,ExceptionInfo *exception)
{
  Image
    *region_image;

  MagickStatusType
    status;

  RectangleInfo
    region_geometry;

  ImageInfo
    *mogrify_info;

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
  SetGeometry(*image,&region_geometry);
  region_image=NewImageList();

  /*
    Transmogrify the image.
  */
  for (i=0; i < (ssize_t) argc; i++)
  {
    ssize_t
      count;

    if (IsCommandOption(argv[i]) == MagickFalse)
      continue;
    count=ParseCommandOption(MagickCommandOptions,MagickFalse,argv[i]);
    count=MagickMax(count,0L);
    if ((i+count) >= (ssize_t) argc)
      break;
    status=MogrifyImageInfo(mogrify_info,(int) count+1,argv+i,exception);

    if (LocaleCompare("region",argv[i]+1) == 0)
      {

        /*
          Merge any existing region image back into saved original image
        */
        (void) SyncImageSettings(mogrify_info,*image);
        if (region_image != (Image *) NULL)
          {
            (void) CompositeImage(region_image,region_image->matte !=
                  MagickFalse ? CopyCompositeOp : OverCompositeOp,*image,
                  region_geometry.x,region_geometry.y);
            InheritException(exception,&region_image->exception);
            *image=DestroyImage(*image);
            *image=region_image;
            region_image = (Image *) NULL;
          }

        /*
          Set up a new region image
        */
        if (*(argv[i]) != '+')
          {
            Image
              *crop_image;

            (void) ParseGravityGeometry(*image,argv[i+1],&region_geometry,
              exception);
            crop_image=CropImage(*image,&region_geometry,exception);
            if (crop_image != (Image *) NULL)
              {
                region_image=(*image);
                *image=crop_image;
              }
          }
      }
    else
      status=SimpleOperationImage(mogrify_info,count,argv+i,image,exception);

    i+=count;
  }

  /*
    Merge any existing region image back into saved original image
  */
  if (region_image != (Image *) NULL)
    {
      (void) SyncImageSettings(mogrify_info,*image);
      (void) CompositeImage(region_image,region_image->matte !=
           MagickFalse ? CopyCompositeOp : OverCompositeOp,*image,
           region_geometry.x,region_geometry.y);
      InheritException(exception,&region_image->exception);
      *image=DestroyImage(*image);
      *image=region_image;
      region_image = (Image *) NULL;
    }
  /*
    Free resources.
  */
  mogrify_info=DestroyImageInfo(mogrify_info);
  status=(*image)->exception.severity == UndefinedException ? 1 : 0;
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
      "-median geometry     apply a median filter to the image",
      "-mode geometry       make each pixel the 'predominate color' of the neighborhood",
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
      "                     increase the contrast without saturating highlights or shadows",
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
      "-direction type      render text right-to-left or left-to-right",
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

  (void) printf("Version: %s\n",GetMagickVersion((size_t *) NULL));
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
            GetMagickVersion((size_t *) NULL));
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
          break;
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowMogrifyException(OptionError,"MissingArgument",option);
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
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,argv[i]);
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
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,argv[i]);
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
            value=strtod(argv[i],&p);
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
            (void) fprintf(stdout,"Version: %s\n",
              GetMagickVersion((size_t *) NULL));
            (void) fprintf(stdout,"Copyright: %s\n",GetMagickCopyright());
            (void) fprintf(stdout,"Features: %s\n\n",GetMagickFeatures());
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
            image_info->colorspace=(ColorspaceType) ParseCommandOption(
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
            image_info->compression=(CompressionType) ParseCommandOption(
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
              limit=(MagickSizeType) SiPrefixToDouble(argv[i+2],100.0);
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
            (void) FormatMagickString(page,MaxTextExtent,"%lux%lu",
              (unsigned long) geometry.width,(unsigned long) geometry.height);
            if (((flags & XValue) != 0) || ((flags & YValue) != 0))
              (void) FormatMagickString(page,MaxTextExtent,"%lux%lu%+ld%+ld",
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
#if 0
This has been merged completely into MogrifyImage()
        if (LocaleCompare("crop",option+1) == 0)
          {
            MagickStatusType
              flags;

            RectangleInfo
              geometry;

            /*
              Crop Image.
            */
            (void) SyncImagesSettings(mogrify_info,*images);
            flags=ParseGravityGeometry(*images,argv[i+1],&geometry,exception);
            if (((geometry.width == 0) && (geometry.height == 0)) ||
                ((flags & XValue) != 0) || ((flags & YValue) != 0))
              break;
            (void) TransformImages(images,argv[i+1],(char *) NULL);
            InheritException(exception,&(*images)->exception);
            break;
          }
#endif
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
            op=(MagickEvaluateOperator) ParseCommandOption(MagickEvaluateOptions,
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
        if (LocaleCompare("print",option+1) == 0)
          {
            char
              *string;

            (void) SyncImagesSettings(mogrify_info,*images);
            string=InterpretImageProperties(mogrify_info,*images,argv[i+1]);
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
            (void) FormatMagickString(key,MaxTextExtent,"cache:%s",argv[i+1]);
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
fprintf(stderr, "mogrify start %s %d (%s)\n",argv[0],argc,post?"post":"pre");
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
fprintf(stderr, "mogrify %ld of %ld\n",
  (long)GetImageIndexInList(*images),(long)GetImageListLength(*images));
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
fprintf(stderr, "mogrify end %ld of %ld\n",
  (long)GetImageIndexInList(*images),(long)GetImageListLength(*images));
#endif

  /*
    Post-process, multi-image sequence operators
  */
  *images=GetFirstImageInList(*images);
  if (post != MagickFalse)
    status&=MogrifyImageList(image_info,argc,argv,images,exception);
  return(status != 0 ? MagickTrue : MagickFalse);
}
