/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        DDDD   EEEEE  PPPP   RRRR   EEEEE   CCCC   AAA   TTTTT  EEEEE        %
%        D   D  E      P   P  R   R  E      C      A   A    T    E            %
%        D   D  EEE    PPPPP  RRRR   EEE    C      AAAAA    T    EEE          %
%        D   D  E      P      R R    E      C      A   A    T    E            %
%        DDDD   EEEEE  P      R  R   EEEEE   CCCC  A   A    T    EEEEE        %
%                                                                             %
%                                                                             %
%                       MagickWand Deprecated Methods                         %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                October 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
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
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/mogrify-private.h"
#include "MagickWand/wand.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/string-private.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/utility-private.h"

#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n v e r t I m a g e C o m m a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConvertImageCommand() reads one or more images, applies one or more image
%  processing operations, and writes out the image in the same or differing
%  format.
%
%  The format of the ConvertImageCommand method is:
%
%      MagickBooleanType ConvertImageCommand(ImageInfo *image_info,int argc,
%        char **argv,char **metadata,ExceptionInfo *exception)
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

static MagickBooleanType ConcatenateImages(int argc,char **argv,
  ExceptionInfo *exception)
{
  FILE
    *input,
    *output;

  int
    c;

  MagickBooleanType
    status;

  ssize_t
    i;

  /*
    Open output file.
  */
  output=fopen_utf8(argv[argc-1],"wb");
  if (output == (FILE *) NULL)
    {
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",
        argv[argc-1]);
      return(MagickFalse);
    }
  status=MagickTrue;
  for (i=2; i < (ssize_t) (argc-1); i++)
  {
    input=fopen_utf8(argv[i],"rb");
    if (input == (FILE *) NULL)
      {
        ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[i]);
        continue;
      }
    for (c=fgetc(input); c != EOF; c=fgetc(input))
      if (fputc((char) c,output) != c)
        status=MagickFalse;
    (void) fclose(input);
    (void) remove_utf8(argv[i]);
  }
  (void) fclose(output);
  return(status);
}

static MagickBooleanType ConvertUsage(void)
{
  static const char
    channel_operators[] =
      "  -channel-fx expression\n"
      "                       exchange, extract, or transfer one or more image channels\n"
      "  -separate            separate an image channel into a grayscale image",
    miscellaneous[] =
      "  -debug events        display copious debugging information\n"
      "  -distribute-cache port\n"
      "                       distributed pixel cache spanning one or more servers\n"
      "  -help                print program options\n"
      "  -list type           print a list of supported option arguments\n"
      "  -log format          format of debugging information\n"
      "  -version             print version information",
    operators[] =
      "  -adaptive-blur geometry\n"
      "                       adaptively blur pixels; decrease effect near edges\n"
      "  -adaptive-resize geometry\n"
      "                       adaptively resize image using 'mesh' interpolation\n"
      "  -adaptive-sharpen geometry\n"
      "                       adaptively sharpen pixels; increase effect near edges\n"
      "  -alpha option        on, activate, off, deactivate, set, opaque, copy\n"
      "                       transparent, extract, background, or shape\n"
      "  -annotate geometry text\n"
      "                       annotate the image with text\n"
      "  -auto-gamma          automagically adjust gamma level of image\n"
      "  -auto-level          automagically adjust color levels of image\n"
      "  -auto-orient         automagically orient (rotate) image\n"
      "  -auto-threshold method\n"
      "                       automatically perform image thresholding\n"
      "  -bench iterations    measure performance\n"
      "  -bilateral-blur geometry\n"
      "                       non-linear, edge-preserving, and noise-reducing smoothing filter\n"
      "  -black-threshold value\n"
      "                       force all pixels below the threshold into black\n"
      "  -blue-shift factor   simulate a scene at nighttime in the moonlight\n"
      "  -blur geometry       reduce image noise and reduce detail levels\n"
      "  -border geometry     surround image with a border of color\n"
      "  -bordercolor color   border color\n"
      "  -brightness-contrast geometry\n"
      "                       improve brightness / contrast of the image\n"
      "  -canny geometry      detect edges in the image\n"
      "  -cdl filename        color correct with a color decision list\n"
      "  -channel mask        set the image channel mask\n"
      "  -charcoal radius     simulate a charcoal drawing\n"
      "  -chop geometry       remove pixels from the image interior\n"
      "  -clahe geometry      contrast limited adaptive histogram equalization\n"
      "  -clamp               keep pixel values in range (0-QuantumRange)\n"
      "  -colorize value      colorize the image with the fill color\n"
      "  -color-matrix matrix apply color correction to the image\n"
      "  -colors value        preferred number of colors in the image\n"
      "  -connected-components connectivity\n"
      "                       connected-components uniquely labeled\n"
      "  -contrast            enhance or reduce the image contrast\n"
      "  -contrast-stretch geometry\n"
      "                       improve contrast by 'stretching' the intensity range\n"
      "  -convolve coefficients\n"
      "                       apply a convolution kernel to the image\n"
      "  -cycle amount        cycle the image colormap\n"
      "  -decipher filename   convert cipher pixels to plain pixels\n"
      "  -deskew threshold    straighten an image\n"
      "  -despeckle           reduce the speckles within an image\n"
      "  -distort method args\n"
      "                       distort images according to given method and args\n"
      "  -draw string         annotate the image with a graphic primitive\n"
      "  -edge radius         apply a filter to detect edges in the image\n"
      "  -encipher filename   convert plain pixels to cipher pixels\n"
      "  -emboss radius       emboss an image\n"
      "  -enhance             apply a digital filter to enhance a noisy image\n"
      "  -equalize            perform histogram equalization to an image\n"
      "  -evaluate operator value\n"
      "                       evaluate an arithmetic, relational, or logical expression\n"
      "  -extent geometry     set the image size\n"
      "  -extract geometry    extract area from image\n"
      "  -fft                 implements the discrete Fourier transform (DFT)\n"
      "  -flip                flip image vertically\n"
      "  -floodfill geometry color\n"
      "                       floodfill the image with color\n"
      "  -flop                flop image horizontally\n"
      "  -frame geometry      surround image with an ornamental border\n"
      "  -function name parameters\n"
      "                       apply function over image values\n"
      "  -gamma value         level of gamma correction\n"
      "  -gaussian-blur geometry\n"
      "                       reduce image noise and reduce detail levels\n"
      "  -geometry geometry   preferred size or location of the image\n"
      "  -grayscale method    convert image to grayscale\n"
      "  -hough-lines geometry\n"
      "                       identify lines in the image\n"
      "  -identify            identify the format and characteristics of the image\n"
      "  -ift                 implements the inverse discrete Fourier transform (DFT)\n"
      "  -implode amount      implode image pixels about the center\n"
      "  -integral            calculate the sum of values (pixel values) in the image\n"
      "  -interpolative-resize geometry\n"
      "                       resize image using interpolation\n"
      "  -kmeans geometry     K means color reduction\n"
      "  -kuwahara geometry   edge preserving noise reduction filter\n"
      "  -lat geometry        local adaptive thresholding\n"
      "  -level value         adjust the level of image contrast\n"
      "  -level-colors color,color\n"
      "                       level image with the given colors\n"
      "  -linear-stretch geometry\n"
      "                       improve contrast by 'stretching with saturation'\n"
      "  -liquid-rescale geometry\n"
      "                       rescale image with seam-carving\n"
      "  -local-contrast geometry\n"
      "                       enhance local contrast\n"
      "  -mean-shift geometry delineate arbitrarily shaped clusters in the image\n"
      "  -median geometry     apply a median filter to the image\n"
      "  -mode geometry       make each pixel the 'predominant color' of the\n"
      "                       neighborhood\n"
      "  -modulate value      vary the brightness, saturation, and hue\n"
      "  -monochrome          transform image to black and white\n"
      "  -morphology method kernel\n"
      "                       apply a morphology method to the image\n"
      "  -motion-blur geometry\n"
      "                       simulate motion blur\n"
      "  -negate              replace every pixel with its complementary color \n"
      "  -noise geometry      add or reduce noise in an image\n"
      "  -normalize           transform image to span the full range of colors\n"
      "  -opaque color        change this color to the fill color\n"
      "  -ordered-dither NxN\n"
      "                       add a noise pattern to the image with specific\n"
      "                       amplitudes\n"
      "  -paint radius        simulate an oil painting\n"
      "  -perceptible epsilon\n"
      "                       pixel value less than |epsilon| become epsilon or\n"
      "                       -epsilon\n"
      "  -polaroid angle      simulate a Polaroid picture\n"
      "  -posterize levels    reduce the image to a limited number of color levels\n"
      "  -profile filename    add, delete, or apply an image profile\n"
      "  -quantize colorspace reduce colors in this colorspace\n"
      "  -raise value         lighten/darken image edges to create a 3-D effect\n"
      "  -random-threshold low,high\n"
      "                       random threshold the image\n"
      "  -range-threshold values\n"
      "                       perform either hard or soft thresholding within some range of values in an image\n"
      "  -region geometry     apply options to a portion of the image\n"
      "  -render              render vector graphics\n"
      "  -resample geometry   change the resolution of an image\n"
      "  -reshape geometry    reshape the image\n"
      "  -resize geometry     resize the image\n"
      "  -roll geometry       roll an image vertically or horizontally\n"
      "  -rotate degrees      apply Paeth rotation to the image\n"
      "  -rotational-blur angle\n"
      "                       rotational blur the image\n"
      "  -sample geometry     scale image with pixel sampling\n"
      "  -scale geometry      scale the image\n"
      "  -segment values      segment an image\n"
      "  -selective-blur geometry\n"
      "                       selectively blur pixels within a contrast threshold\n"
      "  -sepia-tone threshold\n"
      "                       simulate a sepia-toned photo\n"
      "  -set property value  set an image property\n"
      "  -shade degrees       shade the image using a distant light source\n"
      "  -shadow geometry     simulate an image shadow\n"
      "  -sharpen geometry    sharpen the image\n"
      "  -shave geometry      shave pixels from the image edges\n"
      "  -shear geometry      slide one edge of the image along the X or Y axis\n"
      "  -sigmoidal-contrast geometry\n"
      "                       increase the contrast without saturating highlights or\n"
      "                       shadows\n"
      "  -sketch geometry     simulate a pencil sketch\n"
      "  -solarize threshold  negate all pixels above the threshold level\n"
      "  -sort-pixels         sort each scanline in ascending order of intensity\n"
      "  -sparse-color method args\n"
      "                       fill in a image based on a few color points\n"
      "  -splice geometry     splice the background color into the image\n"
      "  -spread radius       displace image pixels by a random amount\n"
      "  -statistic type geometry\n"
      "                       replace each pixel with corresponding statistic from the\n"
      "                       neighborhood\n"
      "  -strip               strip image of all profiles and comments\n"
      "  -swirl degrees       swirl image pixels about the center\n"
      "  -threshold value     threshold the image\n"
      "  -thumbnail geometry  create a thumbnail of the image\n"
      "  -tile filename       tile image when filling a graphic primitive\n"
      "  -tint value          tint the image with the fill color\n"
      "  -transform           affine transform image\n"
      "  -transparent color   make this color transparent within the image\n"
      "  -transpose           flip image vertically and rotate 90 degrees\n"
      "  -transverse          flop image horizontally and rotate 270 degrees\n"
      "  -trim                trim image edges\n"
      "  -type type           image type\n"
      "  -unique-colors       discard all but one of any pixel color\n"
      "  -unsharp geometry    sharpen the image\n"
      "  -vignette geometry   soften the edges of the image in vignette style\n"
      "  -wave geometry       alter an image along a sine wave\n"
      "  -wavelet-denoise threshold\n"
      "                       removes noise from the image using a wavelet transform\n"
      "  -white-balance       automagically adjust white balance of image\n"
      "  -white-threshold value\n"
      "                       force all pixels above the threshold into white",
    sequence_operators[] =
      "  -append              append an image sequence\n"
      "  -clut                apply a color lookup table to the image\n"
      "  -coalesce            merge a sequence of images\n"
      "  -combine             combine a sequence of images\n"
      "  -compare             mathematically and visually annotate the difference between an image and its reconstruction\n"
      "  -complex operator    perform complex mathematics on an image sequence\n"
      "  -composite           composite image\n"
      "  -copy geometry offset\n"
      "                       copy pixels from one area of an image to another\n"
      "  -crop geometry       cut out a rectangular region of the image\n"
      "  -deconstruct         break down an image sequence into constituent parts\n"
      "  -evaluate-sequence operator\n"
      "                       evaluate an arithmetic, relational, or logical expression\n"
      "  -flatten             flatten a sequence of images\n"
      "  -fx expression       apply mathematical expression to an image channel(s)\n"
      "  -hald-clut           apply a Hald color lookup table to the image\n"
      "  -layers method       optimize, merge, or compare image layers\n"
      "  -morph value         morph an image sequence\n"
      "  -mosaic              create a mosaic from an image sequence\n"
      "  -poly terms          build a polynomial from the image sequence and the corresponding\n"
      "                       terms (coefficients and degree pairs).\n"
      "  -print string        interpret string and print to console\n"
      "  -process arguments   process the image with a custom image filter\n"
      "  -smush geometry      smush an image sequence together\n"
      "  -write filename      write images to this file",
    settings[] =
      "  -adjoin              join images into a single multi-image file\n"
      "  -affine matrix       affine transform matrix\n"
      "  -alpha option        activate, deactivate, reset, or set the alpha channel\n"
      "  -antialias           remove pixel-aliasing\n"
      "  -authenticate password\n"
      "                       decipher image with this password\n"
      "  -attenuate value     lessen (or intensify) when adding noise to an image\n"
      "  -background color    background color\n"
      "  -bias value          add bias when convolving an image\n"
      "  -black-point-compensation\n"
      "                       use black point compensation\n"
      "  -blue-primary point  chromaticity blue primary point\n"
      "  -bordercolor color   border color\n"
      "  -caption string      assign a caption to an image\n"
      "  -clip                clip along the first path from the 8BIM profile\n"
      "  -clip-mask filename  associate a clip mask with the image\n"
      "  -clip-path id        clip along a named path from the 8BIM profile\n"
      "  -colorspace type     alternate image colorspace\n"
      "  -comment string      annotate image with comment\n"
      "  -compose operator    set image composite operator\n"
      "  -compress type       type of pixel compression when writing the image\n"
      "  -define format:option\n"
      "                       define one or more image format options\n"
      "  -delay value         display the next image after pausing\n"
      "  -density geometry    horizontal and vertical density of the image\n"
      "  -depth value         image depth\n"
      "  -direction type      render text right-to-left or left-to-right\n"
      "  -display server      get image or font from this X server\n"
      "  -dispose method      layer disposal method\n"
      "  -dither method       apply error diffusion to image\n"
      "  -encoding type       text encoding type\n"
      "  -endian type         endianness (MSB or LSB) of the image\n"
      "  -family name         render text with this font family\n"
      "  -features distance   analyze image features (e.g. contrast, correlation)\n"
      "  -fill color          color to use when filling a graphic primitive\n"
      "  -filter type         use this filter when resizing an image\n"
      "  -font name           render text with this font\n"
      "  -format \"string\"     output formatted image characteristics\n"
      "  -fuzz distance       colors within this distance are considered equal\n"
      "  -gravity type        horizontal and vertical text placement\n"
      "  -green-primary point chromaticity green primary point\n"
      "  -illuminant type     reference illuminant\n"
      "  -intensity method    method to generate an intensity value from a pixel\n"
      "  -intent type         type of rendering intent when managing the image color\n"
      "  -interlace type      type of image interlacing scheme\n"
      "  -interline-spacing value\n"
      "                       set the space between two text lines\n"
      "  -interpolate method  pixel color interpolation method\n"
      "  -interword-spacing value\n"
      "                       set the space between two words\n"
      "  -kerning value       set the space between two letters\n"
      "  -label string        assign a label to an image\n"
      "  -limit type value    pixel cache resource limit\n"
      "  -loop iterations     add Netscape loop extension to your GIF animation\n"
      "  -matte               store matte channel if the image has one\n"
      "  -mattecolor color    frame color\n"
      "  -moments             report image moments\n"
      "  -monitor             monitor progress\n"
      "  -orient type         image orientation\n"
      "  -page geometry       size and location of an image canvas (setting)\n"
      "  -ping                efficiently determine image attributes\n"
      "  -pointsize value     font point size\n"
      "  -precision value     maximum number of significant digits to print\n"
      "  -preview type        image preview type\n"
      "  -quality value       JPEG/MIFF/PNG compression level\n"
      "  -quiet               suppress all warning messages\n"
      "  -read-mask filename  associate a read mask with the image\n"
      "  -red-primary point   chromaticity red primary point\n"
      "  -regard-warnings     pay attention to warning messages\n"
      "  -remap filename      transform image colors to match this set of colors\n"
      "  -repage geometry     size and location of an image canvas\n"
      "  -respect-parentheses settings remain in effect until parenthesis boundary\n"
      "  -sampling-factor geometry\n"
      "                       horizontal and vertical sampling factor\n"
      "  -scene value         image scene number\n"
      "  -seed value          seed a new sequence of pseudo-random numbers\n"
      "  -size geometry       width and height of image\n"
      "  -stretch type        render text with this font stretch\n"
      "  -stroke color        graphic primitive stroke color\n"
      "  -strokewidth value   graphic primitive stroke width\n"
      "  -style type          render text with this font style\n"
      "  -support factor      resize support: > 1.0 is blurry, < 1.0 is sharp\n"
      "  -synchronize         synchronize image to storage device\n"
      "  -taint               declare the image as modified\n"
      "  -texture filename    name of texture to tile onto the image background\n"
      "  -tile-offset geometry\n"
      "                       tile offset\n"
      "  -treedepth value     color tree depth\n"
      "  -transparent-color color\n"
      "                       transparent color\n"
      "  -undercolor color    annotation bounding box color\n"
      "  -units type          the units of image resolution\n"
      "  -verbose             print detailed information about the image\n"
      "  -view                FlashPix viewing transforms\n"
      "  -virtual-pixel method\n"
      "                       virtual pixel access method\n"
      "  -weight type         render text with this font weight\n"
      "  -white-point point   chromaticity white point\n"
      "  -write-mask filename associate a write mask with the image"
      "  -word-break type     sets whether line breaks appear wherever the text would otherwise overflow",
    stack_operators[] =
      "  -clone indexes       clone an image\n"
      "  -delete indexes      delete the image from the image sequence\n"
      "  -duplicate count,indexes\n"
      "                       duplicate an image one or more times\n"
      "  -insert index        insert last image into the image sequence\n"
      "  -reverse             reverse image sequence\n"
      "  -swap indexes        swap two images in the image sequence";

  ListMagickVersion(stdout);
  (void) printf("Usage: %s [options ...] file [ [options ...] "
    "file ...] [options ...] file\n",GetClientName());
  (void) printf("\nImage Settings:\n");
  (void) puts(settings);
  (void) printf("\nImage Operators:\n");
  (void) puts(operators);
  (void) printf("\nImage Channel Operators:\n");
  (void) puts(channel_operators);
  (void) printf("\nImage Sequence Operators:\n");
  (void) puts(sequence_operators);
  (void) printf("\nImage Stack Operators:\n");
  (void) puts(stack_operators);
  (void) printf("\nMiscellaneous Options:\n");
  (void) puts(miscellaneous);
  (void) printf(
    "\nBy default, the image format of 'file' is determined by its magic\n");
  (void) printf(
    "number.  To specify a particular image format, precede the filename\n");
  (void) printf(
    "with an image format name and a colon (i.e. ps:image) or specify the\n");
  (void) printf(
    "image type as the filename suffix (i.e. image.ps).  Specify 'file' as\n");
  (void) printf("'-' for standard input or output.\n");
  return(MagickTrue);
}

WandExport MagickBooleanType ConvertImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
#define NotInitialized  (unsigned int) (~0)
#define DestroyConvert() \
{ \
  DestroyImageStack(); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowConvertException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  DestroyConvert(); \
  return(MagickFalse); \
}
#define ThrowConvertInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","'%s': %s",option,argument); \
  DestroyConvert(); \
  return(MagickFalse); \
}

  char
    *filename,
    *option;

  const char
    *format;

  Image
    *image = (Image *) NULL;

  ImageStack
    image_stack[MaxImageStackDepth+1];

  MagickBooleanType
    fire,
    pend,
    respect_parentheses;

  MagickStatusType
    status;

  ssize_t
    i;

  ssize_t
    j,
    k;

  /*
    Set defaults.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) FormatLocaleFile(stderr,"WARNING: %s\n",
        "The convert command is deprecated in IMv7, use \"magick\" instead of \"convert\" or \"magick convert\"\n");
  if (argc == 2)
    {
      option=argv[1];
      if ((LocaleCompare("help",option+1) == 0) ||
          (LocaleCompare("-help",option+1) == 0))
        return(ConvertUsage());
      if ((LocaleCompare("version",option+1) == 0) ||
          (LocaleCompare("-version",option+1) == 0))
        {
          ListMagickVersion(stdout);
          return(MagickTrue);
        }
    }
  if (argc < 3)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "MissingArgument","%s","");
      (void) ConvertUsage();
      return(MagickFalse);
    }
  filename=(char *) NULL;
  format="%w,%h,%m";
  j=1;
  k=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  respect_parentheses=MagickFalse;
  status=MagickTrue;
  /*
    Parse command-line arguments.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowConvertException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  if ((argc > 2) && (LocaleCompare("-concatenate",argv[1]) == 0))
    return(ConcatenateImages(argc,argv,exception));
  for (i=1; i < (ssize_t) (argc-1); i++)
  {
    option=argv[i];
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickTrue,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowConvertException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickTrue,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowConvertException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsCommandOption(option) == MagickFalse)
      {
        Image
          *images;

        /*
          Read input image.
        */
        FireImageStack(MagickTrue,MagickTrue,pend);
        filename=argv[i];
        if ((LocaleCompare(filename,"--") == 0) && (i < (ssize_t) (argc-1)))
          filename=argv[++i];
        if (image_info->ping != MagickFalse)
          images=PingImages(image_info,filename,exception);
        else
          images=ReadImages(image_info,filename,exception);
        status&=(MagickStatusType) (images != (Image *) NULL) &&
          (exception->severity < ErrorException);
        if (images == (Image *) NULL)
          continue;
        AppendImageStack(images);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-resize",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adjoin",option+1) == 0)
          break;
        if (LocaleCompare("affine",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickAlphaChannelOptions,MagickFalse,
              argv[i]);
            if (type < 0)
              ThrowConvertException(OptionError,
                "UnrecognizedAlphaChannelOption",argv[i]);
            break;
          }
        if (LocaleCompare("annotate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("auto-gamma",option+1) == 0)
          break;
        if (LocaleCompare("auto-level",option+1) == 0)
          break;
        if (LocaleCompare("auto-orient",option+1) == 0)
          break;
        if (LocaleCompare("auto-threshold",option+1) == 0)
          {
            ssize_t
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickAutoThresholdOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowConvertException(OptionError,"UnrecognizedThresholdMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("average",option+1) == 0)
          break;
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'b':
      {
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("bench",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("bias",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("bilateral-blur",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blue-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blue-shift",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("box",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("brightness-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("canny",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("caption",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("cdl",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            ssize_t
              channel;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowConvertException(OptionError,"UnrecognizedChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("channel-fx",option+1) == 0)
          {
            ssize_t
              channel;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            channel=ParsePixelChannelOption(argv[i]);
            if (channel < 0)
              ThrowConvertException(OptionError,"UnrecognizedChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("charcoal",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("chop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("clahe",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("clip-path",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("clone",option+1) == 0)
          {
            Image
              *clone_images,
              *clone_list;

            clone_list=CloneImageList(image,exception);
            if (k != 0)
              clone_list=CloneImageList(image_stack[k-1].image,exception);
            if (clone_list == (Image *) NULL)
              ThrowConvertException(ImageError,"ImageSequenceRequired",option);
            FireImageStack(MagickTrue,MagickTrue,MagickTrue);
            if (*option == '+')
              clone_images=CloneImages(clone_list,"-1",exception);
            else
              {
                i++;
                if (i == (ssize_t) argc)
                  ThrowConvertException(OptionError,"MissingArgument",option);
                if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
                  ThrowConvertInvalidArgumentException(option,argv[i]);
                clone_images=CloneImages(clone_list,argv[i],exception);
              }
            if (clone_images == (Image *) NULL)
              ThrowConvertException(OptionError,"NoSuchImage",option);
            AppendImageStack(clone_images);
            clone_list=DestroyImageList(clone_list);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("color-matrix",option+1) == 0)
          {
            KernelInfo
              *kernel_info;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i],exception);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            kernel_info=DestroyKernelInfo(kernel_info);
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) ||
                (IsGeometry(argv[i]) == MagickFalse))
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowConvertException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("color-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickComplexOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedComplexOperator",
                argv[i]);
            break;
          }
        if (LocaleCompare("compose",option+1) == 0)
          {
            ssize_t
              compose;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            compose=ParseCommandOption(MagickComposeOptions,MagickFalse,
              argv[i]);
            if (compose < 0)
              ThrowConvertException(OptionError,"UnrecognizedComposeOperator",
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            compress=ParseCommandOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowConvertException(OptionError,"UnrecognizedImageCompression",
                argv[i]);
            break;
          }
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        if (LocaleCompare("connected-components",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("contrast",option+1) == 0)
          break;
        if (LocaleCompare("contrast-stretch",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i],exception);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            kernel_info=DestroyKernelInfo(kernel_info);
            break;
          }
        if (LocaleCompare("copy",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("cycle",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'd':
      {
        if (LocaleCompare("decipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowConvertException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowConvertException(OptionError,"NoSuchOption",argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("delete",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("deskew",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("despeckle",option+1) == 0)
          break;
        if (LocaleCompare("direction",option+1) == 0)
          {
            ssize_t
              direction;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            direction=ParseCommandOption(MagickDirectionOptions,MagickFalse,
              argv[i]);
            if (direction < 0)
              ThrowConvertException(OptionError,"UnrecognizedDirectionType",
                argv[i]);
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowConvertException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("distort",option+1) == 0)
          {
            ssize_t
              op;

            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickDistortOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedDistortMethod",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowConvertException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("draw",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("duplicate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'e':
      {
        if (LocaleCompare("edge",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("emboss",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("encipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            endian=ParseCommandOption(MagickEndianOptions,MagickFalse,
              argv[i]);
            if (endian < 0)
              ThrowConvertException(OptionError,"UnrecognizedEndianType",
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickEvaluateOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedEvaluateOperator",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickEvaluateOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedEvaluateOperator",
                argv[i]);
            break;
          }
        if (LocaleCompare("extent",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("extract",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'f':
      {
        if (LocaleCompare("family",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("features",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("fft",option+1) == 0)
          break;
        if (LocaleCompare("fill",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            filter=ParseCommandOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowConvertException(OptionError,"UnrecognizedImageFilter",
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            format=argv[i];
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickFunctionOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedFunction",argv[i]);
             i++;
             if (i == (ssize_t) argc)
               ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("fuzz",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("fx",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'g':
      {
        if (LocaleCompare("gamma",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if ((LocaleCompare("gaussian-blur",option+1) == 0) ||
            (LocaleCompare("gaussian",option+1) == 0))
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowConvertException(OptionError,"UnrecognizedGravityType",
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
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickPixelIntensityOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowConvertException(OptionError,"UnrecognizedIntensityMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if (LocaleCompare("hald-clut",option+1) == 0)
          break;
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          {
            DestroyConvert();
            return(ConvertUsage());
          }
        if (LocaleCompare("hough-lines",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'i':
      {
        if (LocaleCompare("identify",option+1) == 0)
          break;
        if (LocaleCompare("ift",option+1) == 0)
          break;
        if (LocaleCompare("illuminant",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickIlluminantOptions,MagickFalse,
              argv[i]);
            if (type < 0)
              ThrowConvertException(OptionError,"UnrecognizedIlluminantMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("implode",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("insert",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("integral",option+1) == 0)
          break;
        if (LocaleCompare("intensity",option+1) == 0)
          {
            ssize_t
              intensity;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            intensity=ParseCommandOption(MagickPixelIntensityOptions,
              MagickFalse,argv[i]);
            if (intensity < 0)
              ThrowConvertException(OptionError,"UnrecognizedIntensityMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("intent",option+1) == 0)
          {
            ssize_t
              intent;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            intent=ParseCommandOption(MagickIntentOptions,MagickFalse,argv[i]);
            if (intent < 0)
              ThrowConvertException(OptionError,"UnrecognizedIntentType",argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowConvertException(OptionError,"UnrecognizedInterlaceType",
                argv[i]);
            break;
          }
        if (LocaleCompare("interline-spacing",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowConvertException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("interword-spacing",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'k':
      {
        if (LocaleCompare("kerning",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("kmeans",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("kuwahara",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("lat",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("layers",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickLayerOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowConvertException(OptionError,"UnrecognizedLayerMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("level",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("level-colors",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowConvertException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("linear-stretch",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("liquid-rescale",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowConvertException(OptionError,"UnrecognizedListType",argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyConvert();
            return(status == 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("local-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t)argc)
              ThrowConvertException(OptionError, "MissingArgument", option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("loop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("magnify",option+1) == 0)
          break;
        if (LocaleCompare("map",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("matte",option+1) == 0)
          break;
        if (LocaleCompare("mattecolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t)argc)
              ThrowConvertException(OptionError, "MissingArgument", option);
            break;
          }
        if (LocaleCompare("maximum",option+1) == 0)
          break;
        if (LocaleCompare("mean-shift",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("median",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickMetricOptions,MagickTrue,argv[i]);
            if (type < 0)
              ThrowConvertException(OptionError,"UnrecognizedMetricType",
                argv[i]);
            break;
          }
        if (LocaleCompare("minimum",option+1) == 0)
          break;
        if (LocaleCompare("mode",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("modulate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("moments",option+1) == 0)
          break;
        if (LocaleCompare("monitor",option+1) == 0)
          break;
        if (LocaleCompare("monochrome",option+1) == 0)
          break;
        if (LocaleCompare("morph",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("morphology",option+1) == 0)
          {
            char
              token[MagickPathExtent];

            KernelInfo
              *kernel_info;

            ssize_t
              op;

            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            (void) GetNextToken(argv[i],(const char **) NULL,MagickPathExtent,token);
            op=ParseCommandOption(MagickMorphologyOptions,MagickFalse,token);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedMorphologyMethod",
                token);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i],exception);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'n':
      {
        if (LocaleCompare("negate",option+1) == 0)
          break;
        if (LocaleCompare("noise",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                ssize_t
                  noise;

                noise=ParseCommandOption(MagickNoiseOptions,MagickFalse,
                  argv[i]);
                if (noise < 0)
                  ThrowConvertException(OptionError,"UnrecognizedNoiseType",
                    argv[i]);
                break;
              }
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("noop",option+1) == 0)
          break;
        if (LocaleCompare("normalize",option+1) == 0)
          break;
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'o':
      {
        if (LocaleCompare("opaque",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("ordered-dither",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("orient",option+1) == 0)
          {
            ssize_t
              orientation;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            orientation=ParseCommandOption(MagickOrientationOptions,
              MagickFalse,argv[i]);
            if (orientation < 0)
              ThrowConvertException(OptionError,"UnrecognizedImageOrientation",
                argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",argv[i])
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("paint",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("perceptible",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("ping",option+1) == 0)
          break;
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("polaroid",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("poly",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("posterize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("precision",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("preview",option+1) == 0)
          {
            ssize_t
              preview;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            preview=ParseCommandOption(MagickPreviewOptions,MagickFalse,
              argv[i]);
            if (preview < 0)
              ThrowConvertException(OptionError,"UnrecognizedPreviewType",
                argv[i]);
            break;
          }
        if (LocaleCompare("print",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("quantize",option+1) == 0)
          {
            ssize_t
              colorspace;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowConvertException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("rotational-blur",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("raise",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("random-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("range-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("read-mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("red-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("region",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("remap",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("reshape",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parentheses=(*option == '-') ? MagickTrue : MagickFalse;
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 's':
      {
        if (LocaleCompare("sample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scale",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scene",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("segment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("selective-blur",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("shade",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shadow",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shave",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shear",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sketch",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("smush",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("solarize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sort-pixels",option+1) == 0)
          break;
        if (LocaleCompare("sparse-color",option+1) == 0)
          {
            ssize_t
              op;

            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickSparseColorOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedSparseColorMethod",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("splice",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("spread",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) ||
                (IsGeometry(argv[i]) == MagickFalse))
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            op=ParseCommandOption(MagickStatisticOptions,MagickFalse,argv[i]);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedStatisticType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("stretch",option+1) == 0)
          {
            ssize_t
              stretch;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            stretch=ParseCommandOption(MagickStretchOptions,MagickFalse,
              argv[i]);
            if (stretch < 0)
              ThrowConvertException(OptionError,"UnrecognizedStyleType",
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("style",option+1) == 0)
          {
            ssize_t
              style;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            style=ParseCommandOption(MagickStyleOptions,MagickFalse,argv[i]);
            if (style < 0)
              ThrowConvertException(OptionError,"UnrecognizedStyleType",
                argv[i]);
            break;
          }
        if (LocaleCompare("support",option+1) == 0)
          {
            i++;  /* deprecated */
            break;
          }
        if (LocaleCompare("swap",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("swirl",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("synchronize",option+1) == 0)
          break;
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("tile-offset",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tint",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          break;
        if (LocaleCompare("transparent",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("transpose",option+1) == 0)
          break;
        if (LocaleCompare("transverse",option+1) == 0)
          break;
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickTypeOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowConvertException(OptionError,"UnrecognizedImageType",
                argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'u':
      {
        if (LocaleCompare("undercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            units=ParseCommandOption(MagickResolutionOptions,MagickFalse,
              argv[i]);
            if (units < 0)
              ThrowConvertException(OptionError,"UnrecognizedUnitsType",
                argv[i]);
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'v':
      {
        if (LocaleCompare("verbose",option+1) == 0)
          break;
        if ((LocaleCompare("version",option+1) == 0) ||
            (LocaleCompare("-version",option+1) == 0))
          {
            ListMagickVersion(stdout);
            break;
          }
        if (LocaleCompare("vignette",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowConvertException(OptionError,
                "UnrecognizedVirtualPixelMethod",argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'w':
      {
        if (LocaleCompare("wave",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("wavelet-denoise",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("weight",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("white-balance",option+1) == 0)
          break;
        if (LocaleCompare("white-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("word-break",option+1) == 0)
          {
            ssize_t
              word_break;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            word_break=ParseCommandOption(MagickWordBreakOptions,MagickFalse,
              argv[i]);
            if (word_break < 0)
              ThrowConvertException(OptionError,"UnrecognizedArgument",argv[i]);
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("write-mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case '?':
        break;
      default:
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
    }
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickTrue,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowConvertException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i-- != (ssize_t) (argc-1))
    ThrowConvertException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  FinalizeImageSettings(image_info,image,MagickTrue);
  if (image == (Image *) NULL)
    ThrowConvertException(OptionError,"NoImagesDefined",argv[argc-1]);
  if (IsCommandOption(argv[argc-1]))
    ThrowConvertException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  if (LocaleCompare(" ",argv[argc-1]) == 0)  /* common line continuation error */
    ThrowConvertException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  status&=(MagickStatusType) WriteImages(image_info,image,argv[argc-1],
    exception);
  if (metadata != (char **) NULL)
    {
      char
        *text;

      text=InterpretImageProperties(image_info,image,format,exception);
      if (text == (char *) NULL)
        ThrowConvertException(ResourceLimitError,"MemoryAllocationFailed",
          GetExceptionMessage(errno));
      (void) ConcatenateString(&(*metadata),text);
      text=DestroyString(text);
    }
  DestroyConvert();
  return(status != 0 ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k G e t I m a g e A l p h a C o l o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickGetImageAlphaColor() returns the image alpha color.
%
%  The format of the MagickGetImageAlphaColor method is:
%
%      MagickBooleanType MagickGetImageAlphaColor(MagickWand *wand,
%        PixelWand *alpha_color)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o alpha_color: return the alpha color.
%
*/
WandExport MagickBooleanType MagickGetImageAlphaColor(MagickWand *wand,
  PixelWand *alpha_color)
{
  assert(wand != (MagickWand *)NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *)NULL)
    ThrowWandException(WandError, "ContainsNoImages", wand->name);
  PixelSetPixelColor(alpha_color,&wand->images->matte_color);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g i c k S e t I m a g e A l p h a C o l o r                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickSetImageAlphaColor() sets the image alpha color.
%
%  The format of the MagickSetImageAlphaColor method is:
%
%      MagickBooleanType MagickSetImageAlphaColor(MagickWand *wand,
%        const PixelWand *matte)
%
%  A description of each parameter follows:
%
%    o wand: the magick wand.
%
%    o matte: the alpha pixel wand.
%
*/
WandExport MagickBooleanType MagickSetImageAlphaColor(MagickWand *wand,
  const PixelWand *alpha)
{
  assert(wand != (MagickWand *)NULL);
  assert(wand->signature == MagickWandSignature);
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);
  if (wand->images == (Image *)NULL)
    ThrowWandException(WandError,"ContainsNoImages",wand->name);
  PixelGetQuantumPacket(alpha,&wand->images->matte_color);
  return(MagickTrue);
}
#endif
